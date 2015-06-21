

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

static struct blob_buf b;

static int flow_info( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{
    /**/
	blob_buf_init( &b, 0 );
	blobmsg_add_u32( &b, "args",  1234 );
	blobmsg_add_u32( &b, "argv",  5678 );

    /**/
    ubus_send_reply(ctx, req, b.head);
    return UBUS_STATUS_OK;
}


static const struct ubus_method flow_methods[] = {
	UBUS_METHOD_NOARG("info",  flow_info ),
};


static struct ubus_object_type system_object_type =
	UBUS_OBJECT_TYPE("flowmeter_iface", flow_methods);

static struct ubus_object flow_object = {
	.name = "we26n.flowmeter",
	.type = &system_object_type,
	.methods = flow_methods,
	.n_methods = ARRAY_SIZE(flow_methods),
};



void server_main( struct ubus_context *ctx )
{
	int ret;

	ret = ubus_add_object(ctx, &flow_object);
	if (ret)
		fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
    
	uloop_run();
}


int  main( void )
{
    struct ubus_context *ctx;

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
	return 0;
	
}


