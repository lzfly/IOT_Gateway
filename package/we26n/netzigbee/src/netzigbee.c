

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>
#include "we26n_type.h"

/*static struct blob_buf b;

static int zigbee_info( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{
   
	blob_buf_init( &b, 0 );
	blobmsg_add_u32( &b, "args",  1234 );
	blobmsg_add_u32( &b, "argv",  5678 );

    
    ubus_send_reply(ctx, req, b.head);
    return UBUS_STATUS_OK;
}


static const struct ubus_method zigbee_methods[] = {
	UBUS_METHOD_NOARG("info",  zigbee_info ),
};


static struct ubus_object_type zigbee_object_type =
	UBUS_OBJECT_TYPE("netzigbee_iface", zigbee_methods);

static struct ubus_object zigbee_object = {
	.name = "we26n.netzigbee",
	.type = &zigbee_object_type,
	.methods = zigbee_methods,
	.n_methods = ARRAY_SIZE(zigbee_methods),
};



void server_main( struct ubus_context *ctx )
{
	int ret;

	ret = ubus_add_object(ctx, &zigbee_object);
	if (ret)
		fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
    
	uloop_run();
}*/


extern void *  beginSearch();
//extern void *  receiveDeviceMsg();
//extern void *  ctrlDevice();

int  prepare_threads( void )
{
    int  iret;
    pthread_t  aux_thrd;

    
    /**/
    iret = pthread_create( &aux_thrd, NULL, beginSearch, NULL );
    if ( 0 != iret )
    {
        printf( "aux pthread create fail, %d", iret );
        return -1;
    }

/*
	iret = pthread_create( &aux_thrd, NULL, receiveDeviceMsg, NULL );
	if ( 0 != iret )
	{
		printf( "receiveDeviceMsg pthread create fail, %d", iret );
		return -1;
	}
	iret = pthread_create( &aux_thrd, NULL, ctrlDevice, NULL );
	if ( 0 != iret )
	{
		printf( "ctrlDevice pthread create fail, %d", iret );
		return -1;
	}
*/	
    return 0;
}


int  main( void )
{
    int  iret;
    struct ubus_context *ctx;

    /**/
    iret = prepare_threads();
    if ( 0 != iret )
    {
        fprintf( stderr, "Failed to prepare thread\n" );
        return 1;
    }


	while(1)
		sleep(1000);
#if 0
    /**/
	uloop_init();

	signal(SIGPIPE, SIG_IGN);	
    
    /**/
	ctx = ubus_connect( NULL );
	if ( NULL == ctx) 
	{
	    fprintf(stderr, "Failed to connect to ubus\n");
	    return -1;
	}

    /**/
	ubus_add_uloop( ctx );
	server_main( ctx );

    /**/
	ubus_free(ctx);
	uloop_done();
#endif

	return 0;
	
}



