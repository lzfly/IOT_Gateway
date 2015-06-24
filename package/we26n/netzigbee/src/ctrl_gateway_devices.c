
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


static struct blob_buf b;

static int zigbee_info( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{

	static const struct blobmsg_policy policy[2] = { { "args", BLOBMSG_TYPE_INT32 }, { "argv", BLOBMSG_TYPE_INT32 } };
	struct blob_attr * cur[2];
	uint32_t  x,y;
	static struct blob_buf b;
	
	/**/
	
	{
		/* req.priv */
	
		blobmsg_parse( &policy, 2, &cur, blob_data(msg), blob_len(msg) );
		
		/**/
		x = blobmsg_get_u32( cur[0] );
		y = blobmsg_get_u32( cur[1] );
		
		printf( "x = %d, y = %d\n", x, y );
		
	}

    /**/
	blob_buf_init( &b, 0 );
	blobmsg_add_u32( &b, "args",  1234 );
	blobmsg_add_u32( &b, "argv",  5678 );

    /**/
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
}



int ctrlDevice()
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
    msg[5] = 0x0;
	msg[6] = 0x0;
	msg[7] = 0x0;
	msg[8] = 0x0;
	msg[9] = 0x0;
	msg[10] = 0x0;
	msg[11] = endPoint;
	msg[12] = 0x0;
	msg[13] = 0x0;
	msg[14] = state;

	printf("luz--msg0=0x%x\r\n", msg[0]);
    printf("luz--msg1=0x%x\r\n", msg[1]);
	printf("luz--msg2=0x%x\r\n", msg[2]);
	printf("luz--msg3=0x%x\r\n", msg[3]);
	printf("luz--msg4=0x%x\r\n", msg[4]);
	printf("luz--msg11=0x%x\r\n", msg[11]);
	printf("luz--msg14=0x%x\r\n", msg[14]);
	sendCommand(g_monitor_socket,msg,cmd_length);

	return 0;

}


