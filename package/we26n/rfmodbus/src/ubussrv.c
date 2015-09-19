
#include <syslog.h>
#include <pthread.h>

#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>

#include "m4bus.h"
#include "ubussrv.h"


struct ubus_context * ctx;
static intptr_t g_m4bus_ctx;



enum {
    RDREG_BUS,
    RDREG_ADDR,
    RDREG_REG,
    RDREG_NUM,
    __RDREG_MAX 
};

static const struct blobmsg_policy  readreg_policy[] = {
    [RDREG_BUS] = { .name = "bus", .type = BLOBMSG_TYPE_INT32 },
    [RDREG_ADDR] = { .name = "addr", .type = BLOBMSG_TYPE_INT32 },
    [RDREG_REG] = { .name = "reg", .type = BLOBMSG_TYPE_INT32 },
    [RDREG_NUM] = { .name = "num", .type = BLOBMSG_TYPE_INT32 },   
};


void  rfmodbus_readreg_m4bus_cbk( intptr_t arg, uint16_t uret, int tlen, uint8_t * pdat )
{
    struct ubus_request_data * nreq;
    static struct blob_buf  b;
    
    /**/
    nreq = (struct ubus_request_data *)arg;
    
    /**/
    blob_buf_init( &b, 0 );
    blobmsg_add_u32( &b, "result", (uint32_t)uret );

    if ( tlen > 0 )
    {
        pdat[tlen] = 0x0;
        blobmsg_add_field( &b, BLOBMSG_TYPE_STRING, "data", pdat, tlen+1 );
    }

    /**/
    ubus_send_reply( ctx, nreq, b.head );
    ubus_complete_deferred_request( ctx, nreq, 0 );
    free( nreq );
    return;
    
}



static int rfmodbus_readreg( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{
    struct blob_attr * tb[__RDREG_MAX];
    struct ubus_request_data * nreq;
    m4bus_req_t  treq;

    /**/
    blobmsg_parse( readreg_policy, ARRAY_SIZE(readreg_policy), tb, blob_data(msg), blob_len(msg));
    
    if ( tb[RDREG_BUS] )
    {
        treq.bus = (uint8_t)blobmsg_get_u32( tb[RDREG_BUS] );
    }
    
    if ( tb[RDREG_ADDR] )
    {
        treq.addr = (uint8_t)blobmsg_get_u32( tb[RDREG_ADDR] );
    }
    
    if ( tb[RDREG_REG] )
    {
        treq.reg = (uint16_t)blobmsg_get_u32( tb[RDREG_REG] );
    }
    
    if ( tb[RDREG_NUM] )
    {
        treq.num = (uint16_t)blobmsg_get_u32( tb[RDREG_NUM] );
    }
    
    /**/
    nreq = (struct ubus_request_data *)malloc( sizeof(struct ubus_request_data) );
    ubus_defer_request( ctx, req, nreq);

    /**/
    m4bus_send_req( g_m4bus_ctx, &treq, rfmodbus_readreg_m4bus_cbk, (intptr_t)nreq, 500 );
    return UBUS_STATUS_OK;
    
}




void  rfmodbus_startpair_m4bus_cbk( intptr_t arg, uint16_t uret, int tlen, uint8_t * pdat )
{
    struct ubus_request_data * nreq;
    static struct blob_buf  b;

    /**/
    nreq = (struct ubus_request_data *)arg;

    /**/
    blob_buf_init( &b, 0 );
    blobmsg_add_u32( &b, "result", (uint32_t)uret );
    
    ubus_send_reply( ctx, nreq, b.head );
    ubus_complete_deferred_request( ctx, nreq, 0 );
    free( nreq );
    return;
}


static int rfmodbus_startpair( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{
    struct ubus_request_data * nreq;
    m4bus_req_t  treq;

    /**/
    nreq = (struct ubus_request_data *)malloc( sizeof(struct ubus_request_data) );
    ubus_defer_request( ctx, req, nreq);

    /**/
    treq.bus = 0;
    treq.addr = 0;
    treq.reg = 1;
    treq.num = 0;

    /**/
    // printf( "in rfmodbus_startpair\n" );
    m4bus_send_req( g_m4bus_ctx, &treq, rfmodbus_startpair_m4bus_cbk, (intptr_t)nreq, 300 );
    return UBUS_STATUS_OK;
    
}


void  rfmodbus_getstate_m4bus_cbk( intptr_t arg, uint16_t uret, int tlen, uint8_t * pdat )
{
    struct ubus_request_data * nreq;
    static struct blob_buf  b;

    /**/
    nreq = (struct ubus_request_data *)arg;

    /**/
    if ( uret != 0 )
    {
        /**/
        blob_buf_init( &b, 0 );
        blobmsg_add_u32( &b, "result", (uint32_t)uret );
    }
    else
    {
        blob_buf_init( &b, 0 );
        blobmsg_add_u32( &b, "result", 0 );
        blobmsg_add_u32( &b, "state", (uint32_t)pdat[0] );
        blobmsg_add_u32( &b, "ecode", (uint32_t)pdat[1] );

        /* paired */
        if ( pdat[0] == 4 )
        {
            pdat[8] = 0x0;
            blobmsg_add_field( &b, BLOBMSG_TYPE_STRING, "addr", &(pdat[2]), 7 );
        }
    }
    
    ubus_send_reply( ctx, nreq, b.head );
    ubus_complete_deferred_request( ctx, nreq, 0 );
    free( nreq );    
    return;
    
}


static int rfmodbus_getstate( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{
    int  iret;
    struct ubus_request_data * nreq;
    m4bus_req_t  treq;

    /**/
    nreq = (struct ubus_request_data *)malloc( sizeof(struct ubus_request_data) );
    ubus_defer_request( ctx, req, nreq);

    /**/
    treq.bus = 0;
    treq.addr = 0;
    treq.reg = 0;
    treq.num = 0;

    /**/
    // printf( "in rfmodbus_getstate\n" );
    iret = m4bus_send_req( g_m4bus_ctx, &treq, rfmodbus_getstate_m4bus_cbk, (intptr_t)nreq, 300 );
    if ( 0 != iret )
    {
        printf( "in rfmodbus_getstate, send ret = %d\n", iret );
    }
    
    return UBUS_STATUS_OK;
    
}



static const struct ubus_method rfmodbus_methods[] = {
    UBUS_METHOD_NOARG( "getstate", rfmodbus_getstate ),
    UBUS_METHOD_NOARG( "startpair", rfmodbus_startpair ),
    UBUS_METHOD( "readreg", rfmodbus_readreg, readreg_policy ),    
};


static struct ubus_object_type system_object_type =
    UBUS_OBJECT_TYPE("flowmeter_iface", rfmodbus_methods);

static struct ubus_object flow_object = {
    .name = "we26n_rfmodbus",
    .type = &system_object_type,
    .methods = rfmodbus_methods,
    .n_methods = ARRAY_SIZE(rfmodbus_methods),
};


void  test_uart_cbk(struct uloop_fd * pufd, unsigned int events)
{
    intptr_t  uctx;

    /**/
    uctx = *((intptr_t *)(pufd + 1));

    /* pufd->eof == true */
    /**/
    m4bus_try_run( uctx );
    return;
}


int  ubussrv_m4bus_add( int fd )
{
    int  iret;
    int  tfd;
    struct uloop_fd * pufd;

    /**/
    iret = m4bus_init( fd, &g_m4bus_ctx );
    if ( 0 != iret )
    {
        return 1;
    }

    /**/
    m4bus_get_fd( g_m4bus_ctx, &tfd );
    
    /**/
    pufd = (struct uloop_fd *)malloc( sizeof(struct uloop_fd) + sizeof(intptr_t) );
    if ( NULL == pufd )
    {
        return 3;
    }
    
    memset( pufd, 0, sizeof(struct uloop_fd) );
    pufd->fd = tfd;
    pufd->cb = test_uart_cbk;
    *((intptr_t *)(pufd + 1)) = g_m4bus_ctx;

    /**/
    iret = uloop_fd_add( pufd, ULOOP_READ | ULOOP_BLOCKING );
    if ( iret < 0 )
    {
        return 4;
    }
    
    return 0;
    
}


int  ubussrv_server_add( void )
{
    int ret;

    /**/
    ctx = ubus_connect( NULL );
    if ( NULL == ctx) 
    {
        return 1;
    }

    /**/
    ubus_add_uloop( ctx );

    /**/
    ret = ubus_add_object( ctx, &flow_object );
    if (ret)
    {
        fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
        return 2;
    }
    return 0;
    
}


void * ubussrv_thread( void * arg )
{
    int  iret;
    
    /**/
    uloop_init();
    signal(SIGPIPE, SIG_IGN);   

    /**/
    iret = ubussrv_m4bus_add( (int)(intptr_t)arg );
    if ( 0 != iret )
    {
        /*  */
        printf( "ubussrv_m4bus_add fail, ret= %d\n", iret );
        return NULL;
    }

    
    /**/
    iret = ubussrv_server_add();
    if ( 0 != iret )
    {
        /*  */
        printf( "ubussrv_server_add fail, ret= %d\n", iret );
        return NULL;
    }

    /**/
    uloop_run();
    uloop_done();

    /**/
    return NULL;
    
}


int  ubussrv_start( int fd )
{
    int  iret;
    pthread_t  pid;
    
    /**/
    iret = pthread_create( &pid, NULL, ubussrv_thread, (void *)(intptr_t)fd );
    if ( 0 != iret )
    {
        /*  */
        printf( "pthread_create fail, ret= %d\n", iret );
        return 1;
    }
    
	return 0;
}


