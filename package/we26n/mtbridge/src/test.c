
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <syslog.h>
#include <pthread.h>

#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>


#include "myqueue.h"
#include "mymqtt.h"



static intptr_t qctx;
static intptr_t mtctx;


enum {
    PUB_TOPIC,
    PUB_MESG,
    __PUB_MAX 
};

static const struct blobmsg_policy  public_policy[] = {
    [PUB_TOPIC] = { .name = "topic", .type = BLOBMSG_TYPE_STRING },
    [PUB_MESG] = { .name = "message", .type = BLOBMSG_TYPE_STRING },
};


static int mtbridge_public( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{
    struct blob_attr * tb[__PUB_MAX];
    char * topic = NULL;
    char * mssge = NULL;
    
    /**/
    blobmsg_parse( public_policy, ARRAY_SIZE(public_policy), tb, blob_data(msg), blob_len(msg));
    
    if ( tb[PUB_TOPIC] )
    {
        topic = blobmsg_data( tb[PUB_TOPIC] );
    }
    
    if ( tb[PUB_MESG] )
    {
        mssge = blobmsg_data( tb[PUB_MESG] );
    }

    /**/
    printf( "ubus, %s, %s\n", topic, mssge );
    
    /**/
    mmqt_publish( mtctx, topic, mssge );
    return UBUS_STATUS_OK;

}


static const struct ubus_method mtbridge_methods[] = {
    UBUS_METHOD( "publish", mtbridge_public, public_policy ),    
};


static struct ubus_object_type system_object_type =
    UBUS_OBJECT_TYPE("mtbridge_iface", mtbridge_methods);


static struct ubus_object mtbridge_object = {
    .name = "we26n_mtbridge",
    .type = &system_object_type,
    .methods = mtbridge_methods,
    .n_methods = ARRAY_SIZE(mtbridge_methods),
};



int  ubussrv_server_add( void )
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
    ret = ubus_add_object( ctx, &mtbridge_object );
    if (ret)
    {
        fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
        return 2;
    }
    
    return 0;
    
}


void  test_uloopfd_cbk(struct uloop_fd * pufd, unsigned int events)
{
    int  iret;
    int  tfd;
    intptr_t  qctx;
    uint8_t tary[4];
    int  tlen;
    mmqt_msg_t * pmsg;

    /**/
    printf( "ufd cbk\n" );
    
    /**/
    qctx = *((intptr_t *)(pufd + 1));
    tfd = (int)*((intptr_t *)(pufd + 1) + 1);

    /**/
    read( tfd, tary, 4 );

    /**/
    while(1)
    {
        iret = msq_dequeue( qctx, (void **)&pmsg, &tlen );
        if ( 0 != iret )
        {
            return;
        }

        /**/
        printf( "msq : %d,%d : %s\n", pmsg->action, pmsg->object, pmsg->msg );
    }

    /**/
    return;
    
}



int  test_uloopfd_add( int tfd, intptr_t qctx )
{
    int  iret;
    struct uloop_fd * pufd;

    /**/
    pufd = (struct uloop_fd *)malloc( sizeof(struct uloop_fd) + sizeof(intptr_t) + sizeof(intptr_t) );
    if ( NULL == pufd )
    {
        return 2;
    }

    /**/
    memset( pufd, 0, sizeof(struct uloop_fd) );
    pufd->fd = tfd;
    pufd->cb = test_uloopfd_cbk;
    *((intptr_t *)(pufd + 1)) = qctx;
    *((intptr_t *)(pufd + 1) + 1) = tfd;
    
    /**/    
    iret = uloop_fd_add( pufd, ULOOP_READ | ULOOP_BLOCKING );
    if ( iret < 0 )
    {
        return 4;
    }

    /**/
    return 0;
    
}


/*
cfg->port = 1883;
cfg->max_inflight = 20;
cfg->keepalive = 60;
cfg->clean_session = true;
cfg->eol = true;
cfg->protocol_version = MQTT_PROTOCOL_V31;
*/


int  test( void )
{
    int  iret;
    int  tfd;
    
    /**/
    iret = msq_init( &qctx );
    if ( 0 != iret )
    {
        return 1;
    }

    /**/
    iret = mmqt_init( qctx, "10.4.44.210", 8020, &mtctx );
    if ( 0 != iret )
    {
        return 2;
    }

    /**/
    mmqt_get_fd( mtctx, &tfd );
    
    /**/
    iret = mmqt_start_run( mtctx );
    if ( 0 != iret )
    {
        return 3;
    }


    /**/
    uloop_init();
    signal(SIGPIPE, SIG_IGN);

    /**/
    iret = ubussrv_server_add();
    if ( 0 != iret )
    {
        /*  */
        printf( "ubussrv_server_add fail, ret= %d\n", iret );
        return 4;
    }

    /**/
    iret = test_uloopfd_add( tfd, qctx );
    if ( 0 != iret )
    {
        return 5;
    }
    
    /**/
    uloop_run();
    uloop_done();
    
    /**/
    return 0;
    
}


int  main( void )
{
    int  iret;

    iret = test();
    printf( "test ret %d\n", iret );
    return 0;
}


