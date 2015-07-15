

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

#include <syslog.h>

#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>

#include "enn_device_type.h"
#include "enn_device_attr.h"

#include "modbus.h"
#include "myuart.h"


typedef struct  powermeter_info
{
    int  state;
    
    /**/
    uint32_t  scount;
    uint32_t  rcount;

    /**/
    double  value;
    
} powermeter_info_t;


typedef struct  flowmeter_info
{
    int  state;
    char  devsn[10];

    /**/
    uint32_t  scount;
    uint32_t  rcount;

    /**/
    double  value;
    
} flowmeter_info_t;


/**/
powermeter_info_t * gp_powermeter_info;
flowmeter_info_t * gp_flowmeter_info;



enum {
    CTRLCMD_GATEWAY,
    CTRLCMD_DEVICEID,
    CTRLCMD_ATTR,
    CTRLCMD_DATA,
    __CTRLCMD_MAX
};


static const struct blobmsg_policy  ctrlcmd_policy[] = {
    [CTRLCMD_GATEWAY] = { .name = "gatewayid", .type = BLOBMSG_TYPE_STRING },
    [CTRLCMD_DEVICEID] = { .name = "deviceid", .type = BLOBMSG_TYPE_STRING },
    [CTRLCMD_ATTR] = { .name = "attr", .type = BLOBMSG_TYPE_STRING },
    [CTRLCMD_DATA] = { .name = "data", .type = BLOBMSG_TYPE_STRING },   
};



static int flow_ctrlcmd( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{

    struct blob_attr * tb[__CTRLCMD_MAX];
    const char * ptr;
    static struct blob_buf b;

    /**/
    blobmsg_parse( ctrlcmd_policy, ARRAY_SIZE(ctrlcmd_policy), tb, blob_data(msg), blob_len(msg));

    if ( tb[CTRLCMD_GATEWAY] )
    {
        ptr = blobmsg_data( tb[CTRLCMD_GATEWAY] );
        printf( "gate = %s\n", ptr );
    }

    if ( tb[CTRLCMD_DEVICEID] )
    {
        ptr = blobmsg_data( tb[CTRLCMD_DEVICEID] );
        printf( "device = %s\n", ptr );
    }

    if ( tb[CTRLCMD_ATTR] )
    {
        ptr = blobmsg_data( tb[CTRLCMD_ATTR] );
        printf( "attr = %s\n", ptr );
    }

    if ( tb[CTRLCMD_DATA] )
    {
        ptr = blobmsg_data( tb[CTRLCMD_DATA] );
        printf( "data = %s\n", ptr );
    }
    

    /* send reply */
    blob_buf_init( &b, 0 );
    blobmsg_add_string( &b, "return",  "ok" );
    
    /**/
    ubus_send_reply( ctx, req, b.head );
    return UBUS_STATUS_OK;
    
}



enum {
    GETDEVS_DEVTYPE,
    __GETDEVS_MAX
};


static const struct blobmsg_policy  getdevs_policy[] = {
    [GETDEVS_DEVTYPE] = { .name = "devicetype", .type = BLOBMSG_TYPE_STRING },
};


static int flow_getdevs( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{

    struct blob_attr * tb[__GETDEVS_MAX];
    const char * ptr;
    static struct blob_buf  b;
    char  tstr[100];
    uint32_t  temp;
    void * tkie;    

    /**/
    blobmsg_parse( getdevs_policy, ARRAY_SIZE(getdevs_policy), tb, blob_data(msg), blob_len(msg));

    if ( NULL == tb[GETDEVS_DEVTYPE] )
    {
        /* send reply */
        blob_buf_init( &b, 0 );
        blobmsg_add_string( &b, "return",  "fail1" );
        
        /**/
        ubus_send_reply( ctx, req, b.head );
    }

    /**/
    ptr = blobmsg_data( tb[GETDEVS_DEVTYPE] );
    temp = strtoul( ptr, NULL, 10 );
    sprintf( tstr, "%04d", temp );

    /**/
    blob_buf_init( &b, 0 );

    /**/
    if ( 0 == strcmp(tstr, ENN_DEVICE_TYPE_WATERMETER) )
    {
        tkie = blobmsg_open_table( &b, NULL );

        sprintf( tstr, "rf433_enn_%s", gp_flowmeter_info->devsn );        
        blobmsg_add_string( &b, "deviceid",  tstr );
        blobmsg_add_string( &b, "devicetype", ENN_DEVICE_TYPE_WATERMETER );
        sprintf( tstr, "%d", ENN_DEVICE_ATTR_POWERMETER_VALUE );
        blobmsg_add_string( &b, "attr", tstr );
        sprintf( tstr, "%f", gp_flowmeter_info->value );
        blobmsg_add_string( &b, "data", tstr );
        
        blobmsg_close_table( &b, tkie );
       
    }
    else if ( 0 == strcmp(tstr, ENN_DEVICE_TYPE_POWERMETER) )
    {
        tkie = blobmsg_open_table( &b, NULL );

        sprintf( tstr, "rf433_enn_%s", "12345678" );        
        blobmsg_add_string( &b, "deviceid",  tstr );
        blobmsg_add_string( &b, "devicetype", ENN_DEVICE_TYPE_POWERMETER );
        sprintf( tstr, "%d", ENN_DEVICE_ATTR_POWERMETER_VALUE );
        blobmsg_add_string( &b, "attr", tstr );
        sprintf( tstr, "%f", gp_powermeter_info->value );
        blobmsg_add_string( &b, "data", tstr );
        
        blobmsg_close_table( &b, tkie );

    }
    else
    {
        blobmsg_add_string( &b, "return",  "fail1" );    
    }
    
    /**/
    ubus_send_reply( ctx, req, b.head );
    return UBUS_STATUS_OK;
    
}



static const struct ubus_method flow_methods[] = {
    UBUS_METHOD( "ctrlcmd",  flow_ctrlcmd, ctrlcmd_policy ),
    UBUS_METHOD( "getdevicescmd",  flow_getdevs, getdevs_policy ),    
};


static struct ubus_object_type system_object_type =
    UBUS_OBJECT_TYPE("flowmeter_iface", flow_methods);

static struct ubus_object flow_object = {
    .name = "we26n_flowmeter",
    .type = &system_object_type,
    .methods = flow_methods,
    .n_methods = ARRAY_SIZE(flow_methods),
};




int  test_server_add( void )
{
    int ret;
    struct ubus_context * ctx;

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


char  g_mac_addr[200];

// FILE *popen(const char *command, const char *type);
// char *fgets(char *s, int size, FILE *stream);


int  test_get_macaddr( void )
{
    FILE * fout;
    char  tbuf[100];
    char * ptr;
    char * dst;
    
    /**/
    fout = popen( "eth_mac r lan", "r" );
    if ( fout == NULL )
    {
        return 1;
    }

    /**/
    ptr = fgets( tbuf, 90, fout );
    if ( NULL == ptr )
    {
        return 2;
    }

    /**/
    pclose( fout );

    /**/
    dst = g_mac_addr;
    ptr = tbuf;
    while( '\0' != *(ptr) )
    {
        if ( *(ptr) != ':' )
        {
            *dst++ = *ptr++;
        }
        else
        {
            ptr++;
        }
    }

    /**/
    if ( *(dst-1) == 0x0a )
    {
        *(dst-1) = '\0';
    }
    
    *dst = '\0';
    return 0;
    
}




int  test_ubus_01_send_report( powermeter_info_t * pinfo, double value )
{
    int  iret;
    uint32_t id;
    char  tstr[200];
    struct ubus_context * ctx;
    struct blob_buf b = { };

    /**/
    ctx = ubus_connect( NULL );
    if ( NULL == ctx )
    {
        return 1;
    }

    /**/
    iret = ubus_lookup_id( ctx, "jianyou", &id );
    if ( 0 != iret )
    {
        ubus_free(ctx);
        return 2;
    }

    /**/
    blob_buf_init( &b, 0 );
    sprintf( tstr, "we26n_%s", g_mac_addr );
    blobmsg_add_string( &b, "gatewayid", tstr );
    sprintf( tstr, "rf433_enn_%s", "12345678" );
    blobmsg_add_string( &b, "deviceid", tstr );
    blobmsg_add_string( &b, "devicetype", ENN_DEVICE_TYPE_POWERMETER );
    sprintf( tstr, "%d", ENN_DEVICE_ATTR_POWERMETER_VALUE );
    blobmsg_add_string( &b, "attr", tstr );
    sprintf( tstr, "%f", value );
    blobmsg_add_string( &b, "data", tstr );
    
    ubus_invoke(ctx, id, "report", b.head, NULL, NULL, 200);

    /**/
    blob_buf_free(&b);
    ubus_free(ctx);
    return 0;
    
}


int  test_modbus_01_data_cbk( intptr_t arg, int tlen, void * pdat )
{
    int32_t  aaa;
    double  ddd;
    uint8_t * puc;
    powermeter_info_t * pinfo;

    /**/
    pinfo = (powermeter_info_t *)arg;


    if ( tlen == 7 )
    {
        puc = (uint8_t *)pdat;

        aaa = modbus_conv_long( &(puc[3]) );
        ddd = aaa;
        ddd = aaa * 250 * 60;
        ddd = ddd / 18000000;
        printf( "%f\n", ddd );

        /**/
        pinfo->value = ddd;

        /**/
        test_ubus_01_send_report( pinfo, ddd );
    }
    
    // test_modbus_cbk( arg, tlen, pdat );
    return 0;
    
}



void  test_modbus_01_timer_cbk( struct uloop_timeout * ptmr )
{
    intptr_t  mctx;
    powermeter_info_t * pinfo;

    /**/
    mctx = *((intptr_t *)(ptmr+1));
    pinfo = (powermeter_info_t *)(((intptr_t *)(ptmr + 1)) + 1);

    /**/
    modbus_send_req( mctx, 1, 0x14, 0x2, test_modbus_01_data_cbk, (intptr_t)pinfo );

    /**/
    uloop_timeout_set( ptmr, 30000 );
    return;
}



int  test_ubus_02_send_report( flowmeter_info_t * pinfo, double value )
{
    int  iret;
    uint32_t id;
    char  tstr[200];
    struct ubus_context * ctx;
    struct blob_buf b = { };

    /**/
    ctx = ubus_connect( NULL );
    if ( NULL == ctx )
    {
        return 1;
    }

    /**/
    iret = ubus_lookup_id( ctx, "jianyou", &id );
    if ( 0 != iret )
    {
        ubus_free(ctx);
        return 2;
    }

    /**/
    blob_buf_init( &b, 0 );
    sprintf( tstr, "we26n_%s", g_mac_addr );
    blobmsg_add_string( &b, "gatewayid", tstr );
    sprintf( tstr, "rf433_enn_%s", pinfo->devsn );
    blobmsg_add_string( &b, "deviceid", tstr );
    blobmsg_add_string( &b, "devicetype", ENN_DEVICE_TYPE_WATERMETER );
    sprintf( tstr, "%d", ENN_DEVICE_ATTR_WATERMETER_VALUE );
    blobmsg_add_string( &b, "attr", tstr );
    sprintf( tstr, "%f", value );
    blobmsg_add_string( &b, "data", tstr );
    
    ubus_invoke(ctx, id, "report", b.head, NULL, NULL, 200);

    /**/
    blob_buf_free(&b);
    ubus_free(ctx);
    return 0;
    
}


int  test_modbus_02_data_cbk( intptr_t arg, int tlen, void * pdat )
{
    int32_t  aaa;
    float  bbb;
    double  ccc;
    uint8_t * puc;
    flowmeter_info_t * pinfo;

    /**/
    pinfo = (flowmeter_info_t *)arg;


    if ( tlen == 11 )
    {
        puc = (uint8_t *)pdat;

        aaa = modbus_conv_longm( &(puc[3]) );
        bbb = modbus_conv_real4( &(puc[7]) );
        ccc = bbb;
        ccc = ccc + aaa;
        printf( "%f\n", ccc );

        /**/
        pinfo->value = ccc;

        /**/
        test_ubus_02_send_report( pinfo, ccc );
    }
    
    // test_modbus_cbk( arg, tlen, pdat );
    return 0;
    
}


int  test_modbus_02_sn_cbk( intptr_t arg, int tlen, void * pdat )
{
    uint8_t * puc;
    flowmeter_info_t * pinfo;

    /**/
    pinfo = (flowmeter_info_t *)arg;
    puc = (uint8_t *)pdat;


    // test_modbus_cbk( arg, tlen, pdat );

    /**/
    if ( tlen == 7 )
    {    
        /**/
        if ( (puc[0] == 2) && (puc[1] == 3) && (puc[2] == 4) )
        {
            /**/
            sprintf( pinfo->devsn, "%02x%02x%02x%02x", puc[4], puc[3], puc[6], puc[5] );
            pinfo->state = 1;
        }
    }
    
    return 0;
    
}


void  test_modbus_02_timer_cbk( struct uloop_timeout * ptmr )
{
    intptr_t  mctx;
    flowmeter_info_t * pinfo;

    /**/
    mctx = *((intptr_t *)(ptmr+1));
    pinfo = (flowmeter_info_t *)(((intptr_t *)(ptmr + 1)) + 1);

    /**/
    switch ( pinfo->state )
    {
    case 0:
        modbus_send_req( mctx, 2, 1528, 2, test_modbus_02_sn_cbk, (intptr_t)pinfo );
        uloop_timeout_set( ptmr, 4000 );        
        break;

    case 1:
        modbus_send_req( mctx, 2, 144, 0x4, test_modbus_02_data_cbk, (intptr_t)pinfo );
        uloop_timeout_set( ptmr, 30000 );        
        break;
    }

    /**/    

    return;
    
}


int  test_prepare_timer( intptr_t mctx )
{
    struct uloop_timeout * ptmr;
    flowmeter_info_t * pinfo2;
    powermeter_info_t * pinfo1;
    
    /**/
    ptmr = (struct uloop_timeout *)malloc( sizeof(struct uloop_timeout) + sizeof(intptr_t) + sizeof(flowmeter_info_t) );
    if ( NULL == ptmr ) 
    {
        return 1;
    }

    /* flow meter */
    memset( ptmr, 0, sizeof(struct uloop_timeout) );
    ptmr->cb = test_modbus_02_timer_cbk;

    /**/
    *((intptr_t *)(ptmr + 1)) = mctx;
    pinfo2 = (flowmeter_info_t *)(((intptr_t *)(ptmr + 1)) + 1);
    pinfo2->state = 0;
    pinfo2->value = 0;
    sprintf( pinfo2->devsn, "%02x%02x%02x%02x", 6,7,8,9 );    
    gp_flowmeter_info = pinfo2;
    uloop_timeout_set( ptmr, 2000 );
    
    /* power meter */
    ptmr = (struct uloop_timeout *)malloc( sizeof(struct uloop_timeout) + sizeof(intptr_t) + sizeof(powermeter_info_t) );
    if ( NULL == ptmr ) 
    {
        return 1;
    }

    /**/
    memset( ptmr, 0, sizeof(struct uloop_timeout) );
    ptmr->cb = test_modbus_01_timer_cbk;

    /**/
    *((intptr_t *)(ptmr + 1)) = mctx;
    pinfo1 = (powermeter_info_t *)(((intptr_t *)(ptmr + 1)) + 1);
    pinfo1->state = 0;
    pinfo1->value = 0;
    gp_powermeter_info = pinfo1;
    uloop_timeout_set( ptmr, 4000 );    
    return 0;
    
}


void  test_uart_cbk(struct uloop_fd * pufd, unsigned int events)
{
    intptr_t  uctx;

    /**/
    uctx = *((intptr_t *)(pufd + 1));

    /* pufd->eof == true */
    /**/
    myuart_run( uctx );
    return;
}


int  test_prepare_ufd( intptr_t uctx )
{
    int  iret;
    int  ufd;
    struct uloop_fd * pufd;


    /**/
    myuart_get_fd( uctx, &ufd );
    
    /**/
    pufd = (struct uloop_fd *)malloc( sizeof(struct uloop_fd) + sizeof(intptr_t) );
    if ( NULL == pufd )
    {
        return 3;
    }
    
    memset( pufd, 0, sizeof(struct uloop_fd) );
    pufd->fd = ufd;
    pufd->cb = test_uart_cbk;
    *((intptr_t *)(pufd + 1)) = uctx;

    /**/
    iret = uloop_fd_add( pufd, ULOOP_READ | ULOOP_BLOCKING );
    if ( iret < 0 )
    {
        return 4;
    }

    return 0;

}


int  test_prepare_modbus( void )
{
    int  iret;
    intptr_t  uctx;
    intptr_t  mctx;

    /**/
    iret = modbus_init( 100, &mctx );
    if ( 0 != iret )
    {
        return 1;
    }

    /**/
    iret = myuart_init( 0, &uctx );
    if ( 0 != iret )
    {
        return 2;
    }

    /**/
    modbus_set_uartctx( mctx, uctx );
    myuart_set_callback( uctx, (uartcb_func)modbus_recv_decode, mctx );

    /**/
    iret = test_prepare_ufd( uctx );
    if ( 0 != iret )
    {
        return 3;
    }
    
    /**/
    iret = test_prepare_timer( mctx );
    if ( 0 != iret )
    {
        return 4;
    }
    
    return 0;
    
}




int  main( void )
{
    int  iret;

    /**/
    syslog( LOG_CRIT, "begin flowmeter ..." );


    iret = test_get_macaddr();
    if ( 0 != iret )
    {
        syslog( LOG_CRIT, "get mac, ret = %d", iret );
        return 1;
    }

    
#if 0
    /**/
    iret = prepare_threads();
    if ( 0 != iret )
    {
        fprintf( stderr, "Failed to prepare thread\n" );
        return 1;
    }

#else


    /**/
    uloop_init();
    signal(SIGPIPE, SIG_IGN);   

    iret = test_server_add();
    if ( 0 != iret )
    {
        printf( "server add fail,ret = %d\n", iret );
        uloop_done();
        return 2;
    }

    /**/
    iret = test_prepare_modbus();
    if ( 0 != iret )
    {
        printf( "prepare modbus fail, ret = %d\n", iret );
        uloop_done();
        return 3;
    }

    /**/
    uloop_run();
    uloop_done();
    
#endif
    
    return 0;
    
}



