
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
#include <string.h>

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
#include "gateway_socket.h"
#include "ctrl_gateway_devices.h"
#include "enn_device_type.h"
#include "enn_device_attr.h"


struct uloop_fd ufd;

enum {
    CTRLCMD_GATEWAY,
    CTRLCMD_DEVICEID,
    CTRLCMD_ATTR,
    CTRLCMD_DATA,
	CTRLCMD_DEVICETYPE,
    __CTRLCMD_MAX
};


static const struct blobmsg_policy  ctrlcmd_policy[] = {
	[CTRLCMD_GATEWAY] = { .name = "gatewayid", .type = BLOBMSG_TYPE_STRING },
	[CTRLCMD_DEVICEID] = { .name = "deviceid", .type = BLOBMSG_TYPE_STRING },
	[CTRLCMD_ATTR] = { .name = "attr", .type = BLOBMSG_TYPE_STRING },
	[CTRLCMD_DATA] = { .name = "data", .type = BLOBMSG_TYPE_STRING },	
};

static const struct blobmsg_policy  getstatecmd_policy[] = {
	[CTRLCMD_GATEWAY] = { .name = "gatewayid", .type = BLOBMSG_TYPE_STRING },
	[CTRLCMD_DEVICEID] = { .name = "deviceid", .type = BLOBMSG_TYPE_STRING },
	[CTRLCMD_ATTR] = { .name = "attr", .type = BLOBMSG_TYPE_STRING },	
};

static const struct blobmsg_policy  getdevicescmd_policy[] = {
	[CTRLCMD_DEVICETYPE] = { .name = "devicetype", .type = BLOBMSG_TYPE_STRING },
};


static int zigbee_ctrlcmd( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{

	struct blob_attr * tb[__CTRLCMD_MAX];
    const char * gatewayidstr;
	const char * deviceIdstr;
	const char * attrstr;
	const char * datastr;
	//w26n_uint16 shortaddr ;
        char ieeestr[32];
	w26n_uint8 endpoint;
	w26n_uint32 attr;
	w26n_uint32 data;
    static struct blob_buf b;

	printf("[zigbee_ctrlcmd]start\r\n");
	
    /**/
	blobmsg_parse( ctrlcmd_policy, ARRAY_SIZE(ctrlcmd_policy), tb, blob_data(msg), blob_len(msg));

	if ( tb[CTRLCMD_GATEWAY] )
	{
		gatewayidstr = blobmsg_data( tb[CTRLCMD_GATEWAY] );
		printf( "gate = %s\n", gatewayidstr );
	}

	if ( tb[CTRLCMD_DEVICEID] )
	{
		deviceIdstr = blobmsg_data( tb[CTRLCMD_DEVICEID] );
		printf( "device = %s\n", deviceIdstr );
	}

	if ( tb[CTRLCMD_ATTR] )
	{
		attrstr = blobmsg_data( tb[CTRLCMD_ATTR] );
		printf( "attr = %s\n", attrstr );
	}

	if ( tb[CTRLCMD_DATA] )
	{
		datastr = blobmsg_data( tb[CTRLCMD_DATA] );
		printf( "data = %s\n", datastr );
	}
	if(0==strcmp(deviceIdstr,"zigbee_fbee_entrynet_ffffffffffffffff_99")&&0==strcmp(attrstr,"9999"))
	{
		sendEntryNet();
		goto done;;
	}
	
    {
	   char *ptr,*ptr1, c = '_';
	   //char ieeestr[32];
	   char endpiontstr[32];
	   ptr = strchr(deviceIdstr, c);
	   printf( "deviceid parse 1\n");
       if(ptr == NULL)
	   {
	       printf( "deviceid fromat is error\n");
	       goto done;
	    }
		   
	   ptr = strchr(ptr + 1, c);
	   printf( "deviceid parse 2\n");
       if(ptr == NULL)
	   {
	       printf( "deviceid fromat is error\n");
	       goto done;
	    }
		   
	   ptr1 = strchr(ptr + 1, c);
	   printf( "deviceid parse 3\n");
       if(ptr1 == NULL)
	   {
	       printf( "deviceid fromat is error\n");
	       goto done;
	    }
		   
       printf( "deviceid parse ptr=0x%x ptr1=0x%x\n", (int)ptr,(int)ptr1);


	   if(ptr == 0 || ptr1 == 0)
	   {
		   printf( "deviceid parse error\n");

           goto done;
	   }
	   int i = 0;

	   printf( "deviceid parse ptr=0x%x ptr1=0x%x\n", (int)ptr,(int)ptr1);

	   printf( "deviceid parse count=%d\n", ptr1-ptr-1);

	   while(i < ptr1 - ptr - 1)
	   {
	      ieeestr[i] = ptr[i+1];
		  i++;
	   }
	   ieeestr[ptr1 - ptr - 1] = 0;
	   
	   strcpy(endpiontstr, ptr1 + 1);
	   endpiontstr[strlen(ptr1) - 1] = 0;
	   
	   printf( "ieeestr = %s\n", ieeestr );
	   printf( "endpiontstr = %s\n", endpiontstr );
	   
	   //shortaddr = strtoul(shortaddrstr, NULL, 10);
	   endpoint = strtoul(endpiontstr, NULL, 10);
	   
	   printf( "ieeestr = %s ieeelen=%d\n", ieeestr, strlen(ieeestr) );
	   printf( "endpoint = %d\n", endpoint );
	   
	   attr = strtoul(attrstr, NULL, 10);
	   printf( "attr = %d\n", attr );
	   
	   data = strtoul(datastr, NULL, 10);
	   printf( "data = %d\n", data );
	   

	}
	{
        int i;
		for(i = 0; i < g_devices_count; i++)
		{
                        if((strstr(ZigbeeId,g_devices[i].ieeestr) == NULL) && ZIGBEE_ENABLE)
                        {
                        	printf("deviceid  not in deviceid config file can not be controlled\n");
                        	printf("deviceid = %s\n",g_devices[i].ieeestr);
                        	printf("ZigbeeId = %s\n",ZigbeeId);
                        	continue;
                        }       
			printf("[ctrlcmd] ieeestr=%s ieeelen=%d endpoint=%d\r\n", g_devices[i].ieeestr,strlen(g_devices[i].ieeestr), g_devices[i].endpoint);
			if(g_devices[i].endpoint == endpoint && (strcmp(g_devices[i].ieeestr,  ieeestr) == 0))
			{
				printf("device SN = %s", g_devices[i].SN);
				printf("device ieeestr = %s", g_devices[i].ieeestr);
				
				switch(g_devices[i].deviceId)
				{
				 case FB_DEVICE_TYPE_LEVEL_CONTROL_SWITCH:
                     sendDeviceState(0x2, g_devices[i].shortaddr, g_devices[i].endpoint, data);
					 break;
				 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP:
				 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP_2:
					 if(attr == ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_STATE)
					 {
					     sendDeviceState(0x2, g_devices[i].shortaddr, g_devices[i].endpoint, data);
						
					 }
					 else if(attr == ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_BRIGHTNESS_VALUE)
					 {
					 	 if(data < 1)
                             data = 1;
          				 if(data > 100)
                             data = 100;   
							 
					     data = data * 255 / 100;
						 sendDeviceLevel(0x2, g_devices[i].shortaddr, g_devices[i].endpoint, data);
					 }
					 else if(attr == ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_COLOR_TEMP_VALUE)
					 {  
					     if(data < 1)
                             data = 1;
          				 if(data > 100)
                             data = 100;               							 
					     data = 2700 + (data - 1) * (6500 - 2700) / 100;
						 sendDeviceColorTemp(0x2, g_devices[i].shortaddr, g_devices[i].endpoint, data);
					 }
					 else
					 {
					     printf("[startSearchDevice] error attr\r\n");
					}
					 break;
				 case FB_DEVICE_TYPE_WINDOWS:
				     sendDeviceState(0x2, g_devices[i].shortaddr, g_devices[i].endpoint, data);
                                     g_openStatus[i] = data;
					 break;
				 case FB_DEVICE_TYPE_POWER_OUTLET:
				     sendDeviceState(0x2, g_devices[i].shortaddr, g_devices[i].endpoint, data);
				         break; 
				default:
					 break;
				}
                
				printf("[startSearchDevice] sendDeviceState=%d\r\n", data);

			}
		}
	}

done:

    /* send reply */
	blob_buf_init( &b, 0 );
	blobmsg_add_string( &b, "return",  "ok777" );
    
    /**/
    ubus_send_reply( ctx, req, b.head );
	
	/**/
	
	
    return UBUS_STATUS_OK;
    
}

static int zigbee_getstatecmd( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{

	struct blob_attr * tb[__CTRLCMD_MAX];
    const char * gatewayidstr;
	const char * deviceIdstr;
	const char * attrstr;

	//w26n_uint16 shortaddr ;
        char ieeestr[32];
	w26n_uint8 endpoint;
	w26n_uint32 attr;

    static struct blob_buf b;

	printf("[zigbee_getstatecmd]start\r\n");
	
    /**/
	blobmsg_parse( ctrlcmd_policy, ARRAY_SIZE(ctrlcmd_policy), tb, blob_data(msg), blob_len(msg));

	if ( tb[CTRLCMD_GATEWAY] )
	{
		gatewayidstr = blobmsg_data( tb[CTRLCMD_GATEWAY] );
		printf( "gate = %s\n", gatewayidstr );
	}

	if ( tb[CTRLCMD_DEVICEID] )
	{
		deviceIdstr = blobmsg_data( tb[CTRLCMD_DEVICEID] );
		printf( "device = %s\n", deviceIdstr );
	}

	if ( tb[CTRLCMD_ATTR] )
	{
		attrstr = blobmsg_data( tb[CTRLCMD_ATTR] );
		printf( "attr = %s\n", attrstr );
	}


	
	
    {
	   char *ptr,*ptr1, c = '_';
	   //char ieeestr[32];
	   char endpiontstr[32];
	   ptr = strchr(deviceIdstr, c);
	   printf( "deviceid parse 1\n");
	   if(ptr == NULL)
	   {
	       printf( "deviceid fromat is error\n");
	       goto done;
	    }

	   ptr = strchr(ptr + 1, c);
	   printf( "deviceid parse 2\n");
       if(ptr == NULL)
	   {
	       printf( "deviceid fromat is error\n");
	       goto done;
	    }

	   ptr1 = strchr(ptr + 1, c);
	   printf( "deviceid parse 3\n");
       if(ptr1 == NULL)
	   {
	       printf( "deviceid fromat is error\n");
	       goto done;
	    }

       printf( "deviceid parse ptr=0x%x ptr1=0x%x\n", (int)ptr,(int)ptr1);


	   if(ptr == 0 || ptr1 == 0)
	   {
		   printf( "deviceid parse error\n");

           goto done;
	   }
	   int i = 0;

	   printf( "deviceid parse ptr=0x%x ptr1=0x%x\n", (int)ptr,(int)ptr1);

	   printf( "deviceid parse count=%d\n", ptr1-ptr-1);

	   while(i < ptr1 - ptr - 1)
	   {
	      ieeestr[i] = ptr[i+1];
		  i++;
	   }
	   ieeestr[ptr1 - ptr - 1] = 0;
	   
	   strcpy(endpiontstr, ptr1 + 1);
	   endpiontstr[strlen(ptr1) - 1] = 0;
	   
	   printf( "ieeestr = %s\n", ieeestr );
	   printf( "endpiontstr = %s\n", endpiontstr );
	   
	   //shortaddr = strtoul(shortaddrstr, NULL, 10);
	   endpoint = strtoul(endpiontstr, NULL, 10);
	   
	   printf( "ieeestr = %s\n", ieeestr );
	   printf( "endpoint = %d\n", endpoint );
	   
	   attr = strtoul(attrstr, NULL, 10);
	   printf( "attr = %d\n", attr );
	   

	}
	{
        int i;
		for(i = 0; i < g_devices_count; i++)
		{
	                printf("[startSearchDevice] ieeestr=%d endpoint=%d\r\n", g_devices[i].ieeestr, g_devices[i].endpoint);

			if(g_devices[i].endpoint == endpoint && (strcmp(g_devices[i].ieeestr,  ieeestr) == 0))
			{
				printf("device SN = %s", g_devices[i].SN);
				printf("device ieeestr = 0x%x", g_devices[i].ieeestr);
				
				switch(g_devices[i].deviceId)
				{
				 case FB_DEVICE_TYPE_LEVEL_CONTROL_SWITCH:
                    			 getDeviceState(0x2, g_devices[i].shortaddr, g_devices[i].endpoint);
					 break;
				 case FB_DEVICE_TYPE_BODY_INFRARED:
					getDeviceState(0x2, g_devices[i].shortaddr, g_devices[i].endpoint);
					 break;
				 case FB_DEVICE_TYPE_MAGNETIC_DOOR:
					getDeviceState(0x2, g_devices[i].shortaddr, g_devices[i].endpoint);
					 break;
				 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP:
				 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP_2:
					 if(attr == ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_STATE)
					 {
					     getDeviceState(0x2, g_devices[i].shortaddr, g_devices[i].endpoint);
						
					 }
					 else if(attr == ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_BRIGHTNESS_VALUE)
					 {
						 getDeviceLevel(0x2, g_devices[i].shortaddr, g_devices[i].endpoint);
					 }
					 else if(attr == ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_COLOR_TEMP_VALUE)
					 {
						 getDeviceColorTemp(0x2, g_devices[i].shortaddr, g_devices[i].endpoint);
					 }
					 else
					 {
					     printf("[startSearchDevice] error attr\r\n");
					}
					 break;
				 case FB_DEVICE_TYPE_WINDOWS:
				     //getDeviceState(0x2, g_devices[i].shortaddr, g_devices[i].endpoint);
					 break;
				 default:
					 break;
				}
                
			}
		}
	}

done:

    /* send reply */
	blob_buf_init( &b, 0 );
	blobmsg_add_string( &b, "return",  "ok" );
    
    /**/
    ubus_send_reply( ctx, req, b.head );
	
	/**/
	
	
    return UBUS_STATUS_OK;
    
}


static int zigbee_getdevicescmd( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{
	struct blob_attr * tb[__CTRLCMD_MAX];
	const char * devicetypestr;
	w26n_uint32 devicetype;


    static struct blob_buf b;

	printf("[zigbee_getdevicescmd]start\r\n");
	
    /**/
	blobmsg_parse( getdevicescmd_policy, ARRAY_SIZE(getdevicescmd_policy), tb, blob_data(msg), blob_len(msg));


	if ( tb[CTRLCMD_DEVICETYPE] )
	{
		devicetypestr = blobmsg_data( tb[CTRLCMD_DEVICETYPE] );
		printf( "devicetypestr = %s\n", devicetypestr );
	}
	printf("[zigbee_getdevicescmd]blobmsg_parse 2222 devicetypestr = %s\r\n",devicetypestr);
    devicetype = strtoul(devicetypestr, NULL, 10);
    //printf("[zigbee_getdevicescmd]blobmsg_parse 333333333\r\n");
        int i;
		int add = 0;
		char devicesstr[2048];
		devicesstr[0] = 0;
		sprintf(&devicesstr[strlen(devicesstr)], "[");
		for(i = 0; i < g_devices_count; i++)
		{
		    add = 0;
			switch(g_devices[i].deviceId)
			{
				case FB_DEVICE_TYPE_COLOR_TEMP_LAMP:
				case FB_DEVICE_TYPE_COLOR_TEMP_LAMP_2:
				case FB_DEVICE_TYPE_PM:
				case FB_DEVICE_TYPE_SMOGE:
                    if(devicetype == 1)
					{
					    add = 1;
					}
					break;
				case FB_DEVICE_TYPE_LEVEL_CONTROL_SWITCH:
				case FB_DEVICE_TYPE_POWER_OUTLET:
                    if(devicetype == 4)
					{
					    add = 1;
					}
					break;
				case FB_DEVICE_TYPE_WINDOWS:
                    if(devicetype == 2)
					{
					    add = 1;
					}
					break;
				case FB_DEVICE_TYPE_TEMP_HUM:
				case FB_DEVICE_TYPE_TEMP_HUM_2:
                    if(devicetype == 3)
					{
					    add = 1;
					}
					break;
				case FB_DEVICE_TYPE_BODY_INFRARED:
				case FB_DEVICE_TYPE_MAGNETIC_DOOR:
				case FB_DEVICE_TYPE_GAS:
                    if(devicetype == 5)
					{
					    add = 1;
					}
					break;
				default:
					break;
			}
			
			if(add){
			    sprintf(&devicesstr[strlen(devicesstr)], "{");
				sprintf(&devicesstr[strlen(devicesstr)], "\"deviceid\":\"zigbee_fbee_%s_%d\",", g_devices[i].ieeestr, g_devices[i].endpoint);
				sprintf(&devicesstr[strlen(devicesstr)], "\"status\":\"%d\",", g_openStatus[i]);

			switch(g_devices[i].deviceId)
			{
			 case FB_DEVICE_TYPE_GAS:
				 sprintf(&devicesstr[strlen(devicesstr)], "\"devicetype\":\"%s\"", ENN_DEVICE_TYPE_GAS);
				 break;
			 case FB_DEVICE_TYPE_MAGNETIC_DOOR:
				 sprintf(&devicesstr[strlen(devicesstr)], "\"devicetype\":\"%s\"", ENN_DEVICE_TYPE_MAGNETIC_DOOR);
				 break;
			 case FB_DEVICE_TYPE_BODY_INFRARED:
				  sprintf(&devicesstr[strlen(devicesstr)], "\"devicetype\":\"%s\"", ENN_DEVICE_TYPE_BODY_INFRARED);
				 break;
			 case FB_DEVICE_TYPE_TEMP_HUM:
			 case FB_DEVICE_TYPE_TEMP_HUM_2:
				 sprintf(&devicesstr[strlen(devicesstr)], "\"devicetype\":\"%s\"", ENN_DEVICE_TYPE_TEMP_HUM);
				 break;
				 
			 case FB_DEVICE_TYPE_LEVEL_CONTROL_SWITCH:
				  sprintf(&devicesstr[strlen(devicesstr)], "\"devicetype\":\"%s\"", ENN_DEVICE_TYPE_ON_OFF_THREE);
				 break;
			 case FB_DEVICE_TYPE_POWER_OUTLET:
				  sprintf(&devicesstr[strlen(devicesstr)], "\"devicetype\":\"%s\"", ENN_DEVICE_TYPE_POWER_OUTLET);
				 break;
			 case FB_DEVICE_TYPE_WINDOWS:
				  sprintf(&devicesstr[strlen(devicesstr)], "\"devicetype\":\"%s\"", ENN_DEVICE_TYPE_WINDOWS);
				 break;
			 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP:
			 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP_2:
				 sprintf(&devicesstr[strlen(devicesstr)], "\"devicetype\":\"%s\"", ENN_DEVICE_TYPE_COLOR_TEMP_LAMP);
				 break;
				 
			 default:
				 sprintf(&devicesstr[strlen(devicesstr)], "\"devicetype\":\"%s\"", "xxxx");
				 break;
			}
                               
                                
				
				sprintf(&devicesstr[strlen(devicesstr)], "},");
				printf("[zigbee_getdevicescmd]i=%d,total_count=%d,deviceid=zigbee_fbee_%s_%d\r\n",i,g_devices_count,g_devices[i].ieeestr,g_devices[i].endpoint);
				
//				if(i<g_devices_count-1)
//				{
//					sprintf(&devicesstr[strlen(devicesstr)], "},");
//					printf("[zigbee_getdevicescmd]i=%d,total_count=%d,deviceid=zigbee_fbee_%s_%d\r\n",i,g_devices_count,g_devices[i].ieeestr,g_devices[i].endpoint);
//				}
//				else
//				{
//					sprintf(&devicesstr[strlen(devicesstr)], "}");
//					printf("[zigbee_getdevicescmd]i=%d,total_count=%d,deviceid=zigbee_fbee_%s_%d\r\n",i,g_devices_count,g_devices[i].ieeestr,g_devices[i].endpoint);
//				}
	        }
			
        }
		//remove last ,

		//printf("[zigbee_getdevicescmd]blobmsg_parse 444444444444\r\n");
	    sprintf(&devicesstr[strlen(devicesstr)-1], "]");
	
	
    /* send reply */
	blob_buf_init( &b, 0 );
	
	//blobmsg_add_string( &b, "devices",  devicesstr );
	blobmsg_add_string( &b, devicetypestr,  devicesstr );
    
    /**/
    ubus_send_reply( ctx, req, b.head );
	
	/**/
	printf("[zigbee_getdevicescmd]ubus_status_ok\n");
    return UBUS_STATUS_OK;
    
}


static const struct ubus_method zigbee_methods[] = {
	UBUS_METHOD( "ctrlcmd",  zigbee_ctrlcmd, ctrlcmd_policy ),
	UBUS_METHOD( "getstatecmd",  zigbee_getstatecmd, getstatecmd_policy ),
	UBUS_METHOD( "getdevicescmd",  zigbee_getdevicescmd, getdevicescmd_policy ),
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


extern int receiveDeviceMsg(char *buf, int len);
static void
zigbee_proc_poll_fd(struct uloop_fd *fd, unsigned int events)
{
        char buf[1024];

        while (1) {
                int b = read(fd->fd, buf, sizeof(buf));
                if (b < 0) {
                        if (errno == EINTR)
                                continue;

                        if (errno == EAGAIN)
                                return;

                        break;
                }
                
                if (!b)
                        break;
				
				receiveDeviceMsg(buf, b);
        }
}

int socket_rev_add()
{
	int iret;
	struct uloop_fd *pufd;
	
    pufd = &ufd;
    
    memset( pufd, 0, sizeof(struct uloop_fd) );
    pufd->fd = g_monitor_socket;
    pufd->cb = zigbee_proc_poll_fd;

    /**/
    iret = uloop_fd_add( pufd, ULOOP_READ);
	
	printf("[socket_rev_add] ok\r\n");
	return 0;
}

int gatewaySevice()
{
    struct ubus_context *ctx;
	
    printf("[gatewaySevice]start\r\n");
		
    /**/
	uloop_init();

	signal(SIGPIPE, SIG_IGN);	
    
    /**/
	server_add();
	
	socket_rev_add();
	
	
	uloop_run();
    printf("[gatewaySevice]exit run\r\n");

	uloop_done();
	printf("[gatewaySevice] exit gateway thread\r\n");

	return 0;

}
int startGatewayService()
{

    int  iret;
	pthread_t  aux_thrd;


    printf( "[startGatewayService]start");
		   
	iret = pthread_create( &aux_thrd, NULL, gatewaySevice, NULL );
	if ( 0 != iret )
	{
		printf( "gatewaySevice pthread create fail, %d", iret );
		return -1;
	}

	return 0;
}

int restartGatewayService()
{
	   socket_rev_add();
}
int endGatewayService()
{
   uloop_fd_delete(&ufd);
   printf( "[endGatewayService]end");
}

int sendDeviceState(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint, w26n_uint8 state)
{

	int cmd_length=15;
	w26n_byte msg[cmd_length];

	
	printf( "[sendDeviceState]start");
	
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

	sendCommand(g_monitor_socket,msg,cmd_length);

	return 0;

}

int sendDeviceLevel(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint, w26n_uint8 level)
{

	int cmd_length=17;
	w26n_byte msg[cmd_length];
	
	printf( "[sendDeviceLevel]start");
	
	msg[0] = RPCS_SET_DEV_LEVEL;
	msg[1] = 0x0F;
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
	msg[14] = level;
	msg[15] = 0x0;
	msg[16] = 0x0;

	sendCommand(g_monitor_socket,msg,cmd_length);

	return 0;

}

int sendDeviceColorTemp(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint, w26n_uint16 colorTemp)
{

	int cmd_length=18;
	w26n_byte msg[cmd_length];
	
	printf( "[sendDeviceColorTemp]start");
	
	msg[0] = RPCS_SET_COLORTMP;
	msg[1] = 0x10;
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
	msg[14] = colorTemp&0xFF;
	msg[15] = (colorTemp&0xFF00)>>8;
	msg[16] = 0x0;
	msg[17] = 0x0;

	sendCommand(g_monitor_socket,msg,cmd_length);

	return 0;

}

int getDeviceState(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint)
{

	int cmd_length=14;
	w26n_byte msg[cmd_length];

	
	printf( "[getDeviceState]start");
	
	msg[0] = RPCS_GET_DEV_STATE;
	msg[1] = 0x0C;
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

	sendCommand(g_monitor_socket,msg,cmd_length);

	return 0;

}

int getDeviceLevel(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint)
{

	int cmd_length=14;
	w26n_byte msg[cmd_length];
	
	printf( "[sendDeviceLevel]start");
	
	msg[0] = RPCS_GET_DEV_LEVEL;
	msg[1] = 0x0C;
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

	sendCommand(g_monitor_socket,msg,cmd_length);

	return 0;

}

int getDeviceColorTemp(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint)
{

	int cmd_length=14;
	w26n_byte msg[cmd_length];
	
	printf( "[sendDeviceColorTemp]start");
	
	msg[0] = RPCS_GET_COLORTMP;
	msg[1] = 0x0C;
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


	sendCommand(g_monitor_socket,msg,cmd_length);

	return 0;

}


int sendTmpHumAlertInterval(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint, w26n_uint16 alertInterval)
{

	int cmd_length=21;
	w26n_byte msg[cmd_length];
	
	printf( "[sendtmpHumAlertInterval]start");
	
	msg[0] = RPCS_SET_DEVICE_REPORT_INT;
	msg[1] = 0x13;
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
	msg[14] = 0x01;
	msg[15] = 0x02;
	msg[16] = 0x00;
	msg[17] = 0x00;
	msg[18] = 0x21;
	msg[19] = alertInterval&0xFF;
	msg[20] = (alertInterval&0xFF00)>>8;

	sendCommand(g_monitor_socket,msg,cmd_length);

	return 0;

}


int sendEntryNet()
{

        int cmd_length=1;
        w26n_byte msg[cmd_length];

        printf( "[sendEntryNet]start");

        msg[0] = RPCS_ALLOW_ADDDEVICES;

        sendCommand(g_monitor_socket,msg,cmd_length);

        return 0;

}

