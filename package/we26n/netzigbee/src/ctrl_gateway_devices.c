
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/time.h>

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>

#include "we26n_type.h"
#include "gateway.h"
#include "fbee_protocol.h"

extern int sendCommand(int fd,w26n_byte* cmd, int cmd_length);


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


static int zigbee_ctrlcmd( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{

	struct blob_attr * tb[__CTRLCMD_MAX];
    const char * ptr;
    static struct blob_buf b;

	printf("[zigbee_ctrlcmd]start\r\n");
	
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
	
	/**/
	
	
    return UBUS_STATUS_OK;
    
}


static const struct ubus_method zigbee_methods[] = {
	UBUS_METHOD( "ctrlcmd",  zigbee_ctrlcmd, ctrlcmd_policy ),
};


static struct ubus_object_type system_object_type =
	UBUS_OBJECT_TYPE("zigbee_iface", zigbee_methods);

static struct ubus_object zigbee_object = {
	.name = "we26n_zigbee_febee",
	.type = &system_object_type,
	.methods = zigbee_methods,
	.n_methods = ARRAY_SIZE(zigbee_methods),
};


int  server_add( void )
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
	ret = ubus_add_object( ctx, &zigbee_object );
	if (ret)
	{
		fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
		return 2;
    }
    printf("[server_add] ok\r\n");
    return 0;
    
}


int ctrlDevice()
{
    struct ubus_context *ctx;
	
    printf("[ctrlDevice]start\r\n");
		
    /**/
	uloop_init();

	signal(SIGPIPE, SIG_IGN);	
    
    /**/
	server_add();
	uloop_run();

	uloop_done();

}

int sendDeviceState(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint, w26n_uint8 state)
{

	int cmd_length=15;
	w26n_byte msg[cmd_length];

	char buffer[512];
	int resp_length=0;
	msg[0] = RPCS_SET_DEV_STATE;
	msg[1] = 0x0D;
	msg[2] = addrmode;
	msg[3] = shortaddr&0xFF;
	msg[4] = (shortaddr&0xFF00)>>8;
	msg[11] = endPoint;
	msg[3] = state;

	sendCommand(g_monitor_socket,msg,cmd_length);

	return 0;

}


