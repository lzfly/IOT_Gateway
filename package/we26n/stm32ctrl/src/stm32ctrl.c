

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

#include "myuart.h"


intptr_t  g_uartctx;


enum {
    RESET_TARGET,
    __RESET_MAX
};


static const struct blobmsg_policy  reset_policy[] = {
    [RESET_TARGET] = { .name = "target", .type = BLOBMSG_TYPE_STRING },
};



static int stm32_reset( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{

    struct blob_attr * tb[__RESET_MAX];
    const char * ptr;
    static struct blob_buf b;
    uint8_t  tary[4];
    
    /**/
    blobmsg_parse( reset_policy, ARRAY_SIZE(reset_policy), tb, blob_data(msg), blob_len(msg));

    if ( tb[RESET_TARGET] )
    {
        ptr = blobmsg_data( tb[RESET_TARGET] );
        printf( "target = %s\n", ptr );
		
		if ( 0 == strcmp(ptr, "stm32" ) )
		{
            tary[0] = 0xAA;
            tary[1] = 0x0;
            
            myuart_send( g_uartctx, 2, tary );
		}
        else if ( 0 == strcmp(ptr, "zigbee" ) )
        {
            tary[0] = 0xAA;
            tary[1] = 0x3;
            
            myuart_send( g_uartctx, 2, tary );
        }
        else if ( 0 == strcmp(ptr, "ble" ) )
        {
            tary[0] = 0xAA;
            tary[1] = 0x2;
            
            myuart_send( g_uartctx, 2, tary );
        }
        else if ( 0 == strcmp(ptr, "rf433" ) )
        {
            tary[0] = 0xAA;
            tary[1] = 0x1;
            
            myuart_send( g_uartctx, 2, tary );
        }
        else
        {
        
        }
        
        /* send reply */
        blob_buf_init( &b, 0 );
        blobmsg_add_string( &b, "return",  "ok" );
        
    }
    else
    {
        /* send reply */
        blob_buf_init( &b, 0 );
        blobmsg_add_string( &b, "return",  "args fail" );
    }

    /**/
    ubus_send_reply( ctx, req, b.head );
    return UBUS_STATUS_OK;
    
}


static int stm32_testbb( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{
    static struct blob_buf  b;
    uint8_t  tary[4];

    /**/
    blob_buf_init( &b, 0 );
    blobmsg_add_string( &b, "return",  "ok" );

    tary[0] = 0xBB;
    tary[1] = 0x0;
    
    myuart_send( g_uartctx, 2, tary );

    /**/
    ubus_send_reply( ctx, req, b.head );
    return UBUS_STATUS_OK;
    
}


static int stm32_getver( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{
    static struct blob_buf  b;
    
    /**/
    blob_buf_init( &b, 0 );
    blobmsg_add_string( &b, "return",  "ok" );

    /**/
    ubus_send_reply( ctx, req, b.head );
    return UBUS_STATUS_OK;
}


static const struct ubus_method stm32_methods[] = {
    UBUS_METHOD( "reset",  stm32_reset, reset_policy ),
    UBUS_METHOD_NOARG( "testbb",  stm32_testbb ),
    UBUS_METHOD_NOARG( "getver",  stm32_getver )
};


static struct ubus_object_type system_object_type =
    UBUS_OBJECT_TYPE("stm32ctrl_iface", stm32_methods);

static struct ubus_object stm32_object = {
    .name = "we26n_stm32ctrl",
    .type = &system_object_type,
    .methods = stm32_methods,
    .n_methods = ARRAY_SIZE(stm32_methods),
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
    ret = ubus_add_object( ctx, &stm32_object );
    if (ret)
    {
        fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
        return 2;
    }
    
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


int  test_prepare_uart( void )
{
    int  iret;
    int  ufd;
    intptr_t  uctx;    
    struct uloop_fd * pufd;

    /**/
    iret = myuart_init( 10, &uctx );
    if ( 0 != iret )
    {
        return 2;
    }

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

    /**/
    g_uartctx = uctx;
    return 0;
    
}


int  main( void )
{
    int  iret;

    /**/
    syslog( LOG_CRIT, "begin stm32ctrl ..." );

    
    /**/
    uloop_init();
    signal( SIGPIPE, SIG_IGN );

    /**/
    iret = test_prepare_uart();
    if ( 0 != iret )
    {
        printf( "prepare uart fail, ret = %d\n", iret );
        uloop_done();
        return 3;
    }


    iret = test_server_add();
    if ( 0 != iret )
    {
        printf( "server add fail,ret = %d\n", iret );
        uloop_done();
        return 2;
    }
    
    /**/
    uloop_run();
    uloop_done();
    
    return 0;
    
}



