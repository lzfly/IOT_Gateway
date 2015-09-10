
#include <stdio.h>
#include <stdint.h>

#include <syslog.h>
#include <pthread.h>

#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>

#include "m4bus.h"
#include "ubussrv.h"


struct ubus_context * ctx;
intptr_t  g_m4bus_ctx;

enum {
    EXCHG_ADDR,
    EXCHG_CMD,
    EXCHG_DATA,
    __EXCHG_MAX 
};

static const struct blobmsg_policy  exchange_policy[] = {
    [EXCHG_ADDR] = { .name = "addr", .type = BLOBMSG_TYPE_INT32 },
    [EXCHG_CMD] = { .name = "cmd", .type = BLOBMSG_TYPE_INT32 },
    [EXCHG_DATA] = { .name = "data", .type = BLOBMSG_TYPE_STRING },
};


enum {
    CONN_MAC,
    __CONN_MAX 
};

static const struct blobmsg_policy  connect_policy[] = {
    [CONN_MAC] = { .name = "mac", .type = BLOBMSG_TYPE_STRING },
};



void  bledaemn_return_error( struct ubus_context *ctx, struct ubus_request_data *req, uint16_t uret )
{
    static struct blob_buf  b;
    
    /**/
    blob_buf_init( &b, 0 );
    blobmsg_add_u32( &b, "result", (uint32_t)uret );

    /**/
    ubus_send_reply( ctx, req, b.head );
    return;
}


void  bledaemn_exchange_m4bus_cbk( intptr_t arg, uint16_t uret, int tlen, uint8_t * pdat )
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

    printf( "read reg, %d\n", tlen );
    
    /**/
    ubus_send_reply( ctx, nreq, b.head );
    ubus_complete_deferred_request( ctx, nreq, 0 );
    free( nreq );
    return;
    
}



static int bledaemn_exchange( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{
    int  iret;
    struct blob_attr * tb[__EXCHG_MAX];
    struct ubus_request_data * nreq;
    uint8_t  addr;
    uint16_t  cmd;
    uint8_t * ptr;
    int  tlen = 0;
    m4bus_req_t * mreq;
    
    /**/
    blobmsg_parse( exchange_policy, ARRAY_SIZE(exchange_policy), tb, blob_data(msg), blob_len(msg));
    
    if ( tb[EXCHG_ADDR] )
    {
        addr = (uint8_t)blobmsg_get_u32( tb[EXCHG_ADDR] );
    }
    else
    {
        bledaemn_return_error( ctx, req, 0xff01 );
        return UBUS_STATUS_OK;
    }
    
    if ( tb[EXCHG_CMD] )
    {
        cmd = (uint16_t)blobmsg_get_u32( tb[EXCHG_CMD] );
    }
    else
    {
        bledaemn_return_error( ctx, req, 0xff02 );
        return UBUS_STATUS_OK;
    }
    
    if ( tb[EXCHG_DATA] )
    {
        /* tlen include tail zero. */
        tlen = blobmsg_len( tb[EXCHG_DATA] );
        ptr = (uint8_t *)blobmsg_data( tb[EXCHG_DATA] );

        if ( tlen <= 1 )
        {
            bledaemn_return_error( ctx, req, 0xff03 );
            return UBUS_STATUS_OK;
        }

        tlen = tlen - 1;
    }

    /**/
    mreq = (m4bus_req_t *)alloca( sizeof(m4bus_req_t) + tlen );
    mreq->addr = addr;
    mreq->cmd = cmd;
    mreq->tlen = tlen;
    if ( tlen > 0 )
    {
        memcpy( mreq->tary, ptr, tlen );
    }
    
    /**/
    nreq = (struct ubus_request_data *)malloc( sizeof(struct ubus_request_data) );

    /**/
    printf( "in bledaemn_exchange \n" );
    iret = m4bus_send_req( g_m4bus_ctx, mreq, bledaemn_exchange_m4bus_cbk, (intptr_t)nreq, 5000 );
    if ( 0 != iret )
    {
        free( nreq );
        bledaemn_return_error( ctx, req, 0xff04 );
        return UBUS_STATUS_OK;    
    }

    /**/
    ubus_defer_request( ctx, req, nreq );    
    return UBUS_STATUS_OK;
    
}




void  bledaemn_connect_m4bus_cbk( intptr_t arg, uint16_t uret, int tlen, uint8_t * pdat )
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


static int bledaemn_connect( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{
    int  iret;
    struct blob_attr * tb[__CONN_MAX];
    struct ubus_request_data * nreq;
    m4bus_req_t * mreq;
    uint8_t * ptr;
    int  tlen = 0;


    /**/
    blobmsg_parse( connect_policy, ARRAY_SIZE(connect_policy), tb, blob_data(msg), blob_len(msg));
    
    if ( tb[CONN_MAC] )
    {
        tlen = blobmsg_len( tb[CONN_MAC] );
        ptr = (uint8_t *)blobmsg_data( tb[CONN_MAC] );

        if ( tlen <= 6 )
        {
            bledaemn_return_error( ctx, req, 0xff01 );
            return UBUS_STATUS_OK;
        }
    }
    else
    {
        bledaemn_return_error( ctx, req, 0xff02 );
        return UBUS_STATUS_OK;
    }

    /**/
    mreq = (m4bus_req_t *)alloca( sizeof(m4bus_req_t) + tlen );
    mreq->addr = 0;
    mreq->cmd = SRV_CMD_CONNECT;
    mreq->tlen = 6;
    memcpy( mreq->tary, ptr, 6 );

    /**/
    nreq = (struct ubus_request_data *)malloc( sizeof(struct ubus_request_data) );

    /**/
    printf( "in bledaemn_connect \n" );
    iret = m4bus_send_req( g_m4bus_ctx, mreq, bledaemn_connect_m4bus_cbk, (intptr_t)nreq, 5000 );
    if ( 0 != iret )
    {
        free( nreq );
        bledaemn_return_error( ctx, req, 0xff04 );
        return UBUS_STATUS_OK;
    }
    
    ubus_defer_request( ctx, req, nreq);
    return UBUS_STATUS_OK;
    
}


void  bledaemn_disconnect_m4bus_cbk( intptr_t arg, uint16_t uret, int tlen, uint8_t * pdat )
{
    struct ubus_request_data * nreq;
    static struct blob_buf  b;

    /**/
    nreq = (struct ubus_request_data *)arg;

    /**/
    blob_buf_init( &b, 0 );
    blobmsg_add_u32( &b, "result", (uint32_t)uret );

    /**/
    ubus_send_reply( ctx, nreq, b.head );
    ubus_complete_deferred_request( ctx, nreq, 0 );
    free( nreq );    
    return;
}


static int bledaemn_disconnect( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{
    int  iret;
    struct ubus_request_data * nreq;
    m4bus_req_t  treq;

    /**/
    treq.addr = 0;
    treq.cmd = SRV_CMD_DISCONN;
    treq.tlen = 0;

    /**/
    nreq = (struct ubus_request_data *)malloc( sizeof(struct ubus_request_data) );
    
    /**/
    // printf( "in rfmodbus_getstate\n" );
    iret = m4bus_send_req( g_m4bus_ctx, &treq, bledaemn_disconnect_m4bus_cbk, (intptr_t)nreq, 2000 );
    if ( 0 != iret )
    {
        free( nreq );
        bledaemn_return_error( ctx, req, 0xff01 );
        return UBUS_STATUS_OK;        
    }

    ubus_defer_request( ctx, req, nreq);    
    return UBUS_STATUS_OK;
    
}



void  bledaemn_getstate_m4bus_cbk( intptr_t arg, uint16_t uret, int tlen, uint8_t * pdat )
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
    }
    
    ubus_send_reply( ctx, nreq, b.head );
    ubus_complete_deferred_request( ctx, nreq, 0 );
    free( nreq );    
    return;
    
}


static int bledaemn_getstate( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{
    int  iret;
    struct ubus_request_data * nreq;
    m4bus_req_t  treq;

    /**/
    treq.addr = 0;
    treq.cmd = SRV_CMD_GETSTATE;
    treq.tlen = 0;
    
    /**/
    nreq = (struct ubus_request_data *)malloc( sizeof(struct ubus_request_data) );

    /**/
    // printf( "in rfmodbus_getstate\n" );
    iret = m4bus_send_req( g_m4bus_ctx, &treq, bledaemn_getstate_m4bus_cbk, (intptr_t)nreq, 2000 );
    if ( 0 != iret )
    {
        printf( "in rfmodbus_getstate, send ret = %d\n", iret );
        free( nreq );
        bledaemn_return_error( ctx, req, 0xff01 );
        return UBUS_STATUS_OK;
    }

    /**/
    ubus_defer_request( ctx, req, nreq);
    return UBUS_STATUS_OK;
    
}


static const struct ubus_method bledaemn_methods[] = {
    UBUS_METHOD_NOARG( "getstate", bledaemn_getstate ),
    UBUS_METHOD( "connect", bledaemn_connect, connect_policy ),    
    UBUS_METHOD_NOARG( "disconnect", bledaemn_disconnect ),
    UBUS_METHOD( "exchage", bledaemn_exchange, exchange_policy ),    
};


static struct ubus_object_type system_object_type =
    UBUS_OBJECT_TYPE("bledaemn_iface", bledaemn_methods);

static struct ubus_object flow_object = {
    .name = "we26n_bledaemn",
    .type = &system_object_type,
    .methods = bledaemn_methods,
    .n_methods = ARRAY_SIZE(bledaemn_methods),
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



