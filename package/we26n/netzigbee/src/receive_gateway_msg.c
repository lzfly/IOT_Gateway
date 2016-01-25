
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
#include "enn_device_type.h"
#include "enn_device_attr.h"
#include "receive_gateway_msg.h"

static int alertcount = 0;
static int humcount = 0;

void  dump_hex( const char * ptr, size_t  len )
{
    int  i;
    int  nn;
    int  len2 = len;
    printf( "len = %d\n", len);
    nn = 0;
    while ( (len2 - nn) >= 16 )
    {
        for ( i=0; i<16; i++ )
        {
            printf( "%02x ", ptr[nn + i] );
        }

        printf("  |  ");

        for ( i=0; i<16; i++ )
        {
            int  c = ptr[nn + i];

            if ( (c < 32) || (c > 127) )
                c = '.';

            printf("%c", c);
        }

        nn += 16;
        printf("\n");

    }

    if ( len2 > nn )
    {
        for ( i = 0; i < (len2-nn); i++ )
            printf("%02x ", ptr[nn + i]);
        printf("  >  ");

        for ( i = 0; i < (len2-nn); i++ )
        {
            int  c = ptr[nn + i];
            if (c < 32 || c > 127)
                c = '.';
            printf("%c", c);
        }

        printf("\n");
    }

    fflush(stdout);

}


void  test_data_cback(struct ubus_request *req, int type, struct blob_attr *msg)
{
	static const struct blobmsg_policy policy[2] = { { "args", BLOBMSG_TYPE_INT32 }, { "argv", BLOBMSG_TYPE_INT32 } };
	//struct blob_attr * cur[2];
	//uint32_t  a,b;
	
	/**/
	if ( type == UBUS_MSG_DATA )
	{
		/* req.priv */
		printf( "data cback, %d\n", type );
//		blobmsg_parse( &policy, 2, &cur, blob_data(msg), blob_len(msg) );
		
		/**/
//		a = blobmsg_get_u32( cur[0] );
//		b = blobmsg_get_u32( cur[1] );
		
//		printf( "a = %d, b = %d\n", a, b );
		
	}

	return;
}


int  sendMsgToWeb(w26n_uint16 deviceId, w26n_char *ieeestr, w26n_uint8 endpoint, w26n_uint16 attr, w26n_uint32 data)
{
    int  iret;
    uint32_t  id;
    struct ubus_context *ctx;
	static struct blob_buf b;
 
    printf("[sendMsgToWeb] start\r\n");


    /**/
	ctx = ubus_connect( NULL );
	if ( NULL == ctx) 
	{
	    fprintf(stderr, "Failed to connect to ubus\n");
	    return -1;
	}

    //printf("[sendMsgToWeb] start--1\r\n");

    /**/
	if ( ubus_lookup_id(ctx, "webbridge_device", &id) ) {
		fprintf(stderr, "Failed to look up webbridge_device object\n");
		return -1;
	}
	//printf("[sendMsgToWeb] start--2\r\n");

	blob_buf_init( &b, 0 );
	char gatewayidstr[32];
	sprintf(gatewayidstr, "we26n_%s", g_localMAC);
	blobmsg_add_string( &b, "gatewayid", gatewayidstr );

    //printf("[sendMsgToWeb] start--3\r\n");
	
	char deviceidstr[64];
	sprintf(deviceidstr, "zigbee_fbee_%s_%d", ieeestr, endpoint);
    //printf("[sendMsgToWeb] start--%s\r\n", deviceidstr);
	blobmsg_add_string( &b, "deviceid", deviceidstr);
	
	char devicetypestr[16];
	char deviceattrstr[16];
	char devicedatastr[64];
	
	switch(deviceId)
	{
	 case FB_DEVICE_TYPE_GAS:
	      sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_GAS);
		  sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_GAS_ALERT);
		  sprintf(devicedatastr, "%d", data);
	     break;
	 case FB_DEVICE_TYPE_MAGNETIC_DOOR:
	      sprintf(devicetypestr,"%d",ENN_DEVICE_TYPE_MAGNETIC_DOOR);
	      sprintf(deviceattrstr,"%d",ENN_DEVICE_ATTR_MAGNETIC_DOOR_ALERT);
	      sprintf(devicedatastr, "%d", data);
	     break;
	 case FB_DEVICE_TYPE_BODY_INFRARED:
	      sprintf(devicetypestr,"%d",ENN_DEVICE_TYPE_BODY_INFRARED);
	      sprintf(deviceattrstr,"%d",ENN_DEVICE_ATTR_BODY_INFRARED);
	      sprintf(devicedatastr, "%d", data);
	     break;
	 case FB_DEVICE_TYPE_TEMP_HUM:
	 case FB_DEVICE_TYPE_TEMP_HUM_2:
	     sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_TEMP_HUM);
		 if(attr == ENN_DEVICE_ATTR_TEMP_VALUE)
		 {
		     double data1 = data;
			 data1 = data1/100;
			 sprintf(devicedatastr, "%2.2f", data1);
			 
		     sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_TEMP_VALUE);
		 }
		 else{
		     sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_HUM_VALUE);
			 sprintf(devicedatastr, "%d", data);
		}
	     break;
		 
	 case FB_DEVICE_TYPE_LEVEL_CONTROL_SWITCH:
	      sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_ON_OFF_THREE);
		  sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_ON_OFF_THREE_STATE);
		  sprintf(devicedatastr, "%d", data);
	     break;
	 case FB_DEVICE_TYPE_POWER_OUTLET:
	      sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_POWER_OUTLET);
		  sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_POWER_OUTLET);
		  sprintf(devicedatastr, "%d", data);
	     break;
	 case FB_DEVICE_TYPE_WINDOWS:
	      sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_WINDOWS);
		  sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_WINDOWS_VALUE);
		  sprintf(devicedatastr, "%d", data);
	     break;
	 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP:
	 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP_2:
	     sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_COLOR_TEMP_LAMP);
		 if(attr == ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_STATE){
		     sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_STATE);
			 sprintf(devicedatastr, "%d", data);
		}
		 else if(attr == ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_BRIGHTNESS_VALUE){
		     sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_BRIGHTNESS_VALUE);
			 data = (data)*100/255 ;
			 sprintf(devicedatastr, "%d", data);
		}
		 else{
		     sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_COLOR_TEMP_VALUE);
			 if(data < 2700)
			     data =2700;
			 if(data > 6500)
			     data =6500;
			 data = (data - 2700)*100/(6500-2700) + 1 ;
			 sprintf(devicedatastr, "%d", data);
		}
	     break;
		 
	 default:
	     sprintf(devicedatastr, "%d", data);
	     break;
	}
	blobmsg_add_string( &b, "devicetype", devicetypestr);
	
	blobmsg_add_string( &b, "deviceattr", deviceattrstr );
	

	
	blobmsg_add_string( &b, "data", devicedatastr);	

	printf("[sendMsgToWeb]ubus_invoke data = %s\r\n", devicedatastr);
	/**/
	ubus_invoke( ctx, id, "device_attr_report", b.head, test_data_cback, 0, 3000);

	
    /**/
	ubus_free(ctx);
	return 0;
	
}

int  addDeviceToWeb(w26n_char * gatewayId, w26n_char * deviceId, w26n_char * devicetype)
{
    uint32_t  id;
    struct ubus_context *ctx;
	static struct blob_buf b;
 
    printf("[addDeviceToWeb] start\r\n");

    /**/
	ctx = ubus_connect( NULL );
	if ( NULL == ctx) 
	{
	    fprintf(stderr, "Failed to connect to ubus\n");
	    return -1;
	}
 printf("[addDeviceToWeb] start-1\r\n");
    /**/
	if ( ubus_lookup_id(ctx, "webbridge_device", &id) ) {
		fprintf(stderr, "Failed to look up webbridge_device object\n");
		return -1;
	}
	
    blob_buf_init( &b, 0 );
	 printf("[addDeviceToWeb] start-%s %s %s\r\n", gatewayId,deviceId,devicetype);
	
	blobmsg_add_string( &b, "gatewayid", gatewayId );

	blobmsg_add_string( &b, "deviceid", deviceId);
	
	blobmsg_add_string( &b, "devicetype", devicetype);
	
	 printf("[addDeviceToWeb] start-3\r\n");
	/**/
	ubus_invoke( ctx, id, "add_device", b.head, test_data_cback, 0, 3000);

	
    /**/
	ubus_free(ctx);
	return 0;
	
}

int  addDeviceAttrToWeb(w26n_char * gatewayId, w26n_char * deviceId, w26n_char * devicetype,   w26n_char *  deviceattr, w26n_char * attrpermission)
{
    uint32_t  id;
    struct ubus_context *ctx;
	static struct blob_buf b;
 
    printf("[addDeviceAttrToWeb] start\r\n");

    /**/
	ctx = ubus_connect( NULL );
	if ( NULL == ctx) 
	{
	    fprintf(stderr, "Failed to connect to ubus\n");
	    return -1;
	}

    /**/
	if ( ubus_lookup_id(ctx, "webbridge_device", &id) ) {
		fprintf(stderr, "Failed to look up webbridge_device object\n");
		return -1;
	}
    blob_buf_init( &b, 0 );
	blobmsg_add_string( &b, "gatewayid", gatewayId );

	blobmsg_add_string( &b, "deviceid", deviceId);
	
	blobmsg_add_string( &b, "devicetype", devicetype);
	
	blobmsg_add_string( &b, "deviceattr", deviceattr );
	
	blobmsg_add_string( &b, "attrpermission", attrpermission );
	
	
	/**/
	ubus_invoke( ctx, id, "add_device_attr", b.head, test_data_cback, 0, 3000);

	
    /**/
	ubus_free(ctx);
	return 0;
	
}

int  addDevice(w26n_uint16 deviceId, w26n_char *ieeestr, w26n_uint8 endpoint)
{
    uint32_t  id;
    struct ubus_context *ctx;
	static struct blob_buf b;
 
    printf("[addDevice] start\r\n");

	char gatewayidstr[32];
	sprintf(gatewayidstr, "we26n_%s", g_localMAC);
	
	char deviceidstr[64];
	sprintf(deviceidstr, "zigbee_fbee_%s_%d", ieeestr, endpoint);

	
	char devicetypestr[16];
	char deviceattrstr[16];

	
	switch(deviceId)
	{
	 case FB_DEVICE_TYPE_GAS:
	      sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_GAS);
		  addDeviceToWeb(gatewayidstr, deviceidstr, devicetypestr);
		  sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_GAS_ALERT);
		  addDeviceAttrToWeb(gatewayidstr, deviceidstr, devicetypestr, deviceattrstr, "I");
		  
	     break;
	 case FB_DEVICE_TYPE_MAGNETIC_DOOR:
	      sprintf(devicetypestr,"%d",ENN_DEVICE_TYPE_MAGNETIC_DOOR);
		  addDeviceToWeb(gatewayidstr, deviceidstr, devicetypestr);
	      sprintf(deviceattrstr,"%d",ENN_DEVICE_ATTR_MAGNETIC_DOOR_ALERT);
	      addDeviceAttrToWeb(gatewayidstr, deviceidstr, devicetypestr, deviceattrstr, "I");
	     break;
	 case FB_DEVICE_TYPE_BODY_INFRARED:
	      sprintf(devicetypestr,"%d",ENN_DEVICE_TYPE_BODY_INFRARED);
		  addDeviceToWeb(gatewayidstr, deviceidstr, devicetypestr);
	      sprintf(deviceattrstr,"%d",ENN_DEVICE_ATTR_BODY_INFRARED);
	      addDeviceAttrToWeb(gatewayidstr, deviceidstr, devicetypestr, deviceattrstr, "I");
	     break;
	 case FB_DEVICE_TYPE_TEMP_HUM:
	 case FB_DEVICE_TYPE_TEMP_HUM_2:
	     sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_TEMP_HUM);
                 addDeviceToWeb(gatewayidstr, deviceidstr, devicetypestr);
		 sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_TEMP_VALUE);
         addDeviceAttrToWeb(gatewayidstr, deviceidstr, devicetypestr, deviceattrstr, "RI");
		 sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_HUM_VALUE);
         addDeviceAttrToWeb(gatewayidstr, deviceidstr, devicetypestr, deviceattrstr, "RI");
	     break;
		 
	 case FB_DEVICE_TYPE_LEVEL_CONTROL_SWITCH:
	      sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_ON_OFF_THREE);
		  addDeviceToWeb(gatewayidstr, deviceidstr, devicetypestr);
		  sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_ON_OFF_THREE_STATE);
		  addDeviceAttrToWeb(gatewayidstr, deviceidstr, devicetypestr, deviceattrstr, "RW");
	     break;
	 case FB_DEVICE_TYPE_POWER_OUTLET:
	      sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_POWER_OUTLET);
		  addDeviceToWeb(gatewayidstr, deviceidstr, devicetypestr);
		  sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_POWER_OUTLET);
		  addDeviceAttrToWeb(gatewayidstr, deviceidstr, devicetypestr, deviceattrstr, "RW");
	     break;
	 case FB_DEVICE_TYPE_WINDOWS:
	      sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_WINDOWS);
		  addDeviceToWeb(gatewayidstr, deviceidstr, devicetypestr);
		  sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_WINDOWS_VALUE);
		  addDeviceAttrToWeb(gatewayidstr, deviceidstr, devicetypestr, deviceattrstr, "RW");
	     break;
	 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP:
	 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP_2:
	     sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_COLOR_TEMP_LAMP);
		 addDeviceToWeb(gatewayidstr, deviceidstr, devicetypestr);

		 sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_STATE);
	     addDeviceAttrToWeb(gatewayidstr, deviceidstr, devicetypestr, deviceattrstr, "RW");
		 sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_BRIGHTNESS_VALUE);
		 addDeviceAttrToWeb(gatewayidstr, deviceidstr, devicetypestr, deviceattrstr, "RW");
		 sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_COLOR_TEMP_VALUE);
         addDeviceAttrToWeb(gatewayidstr, deviceidstr, devicetypestr, deviceattrstr, "RW");
	     break;
		 
	 default:
	     break;
	}
	return 0;
	
}


int  sendMsgToMQTT(w26n_uint16 deviceId, w26n_char *ieeestr, w26n_uint8 endpoint, w26n_uint16 attr, w26n_uint32 data)
{
    int  iret;
    uint32_t  id;
    struct ubus_context *ctx;
	static struct blob_buf b;
 
    printf("[sendMsgToMQTT] start\r\n");


    /**/
	ctx = ubus_connect( NULL );
	if ( NULL == ctx) 
	{
	    fprintf(stderr, "Failed to connect to ubus\n");
	    return -1;
	}

    //printf("[sendMsgToWeb] start--1\r\n");

    /**/
	if ( ubus_lookup_id(ctx, "we26n_mtbridge", &id) ) {
		fprintf(stderr, "Failed to look up we26n_mtbridge object\n");
		return;
	}
	//printf("[sendMsgToWeb] start--2\r\n");

	blob_buf_init( &b, 0 );
	char gatewayidstr[32];
	sprintf(gatewayidstr, "we26n_%s", g_localMAC);
	blobmsg_add_string( &b, "gatewayid", gatewayidstr );

    //printf("[sendMsgToWeb] start--3\r\n");
	
	char deviceidstr[64];
	sprintf(deviceidstr, "zigbee_fbee_%s_%d", ieeestr, endpoint);
    //printf("[sendMsgToWeb] start--%s\r\n", deviceidstr);
	blobmsg_add_string( &b, "devid", deviceidstr);
	
	char devicetypestr[16];
	uint32_t  uuid = 0;
	char devicedatastr[64];
	
	switch(deviceId)
	{
	 case FB_DEVICE_TYPE_GAS:
	      sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_GAS);
		  uuid = ENN_DEVICE_ATTR_GAS_ALERT;
		  sprintf(devicedatastr, "%d", data);
	     break;
	 case FB_DEVICE_TYPE_MAGNETIC_DOOR:
	      sprintf(devicetypestr,"%d",ENN_DEVICE_TYPE_MAGNETIC_DOOR);
	      uuid = ENN_DEVICE_ATTR_MAGNETIC_DOOR_ALERT;
	      sprintf(devicedatastr, "%d", data);
	     break;
	 case FB_DEVICE_TYPE_BODY_INFRARED:
	      sprintf(devicetypestr,"%d",ENN_DEVICE_TYPE_BODY_INFRARED);
	      uuid = ENN_DEVICE_ATTR_BODY_INFRARED;
	      sprintf(devicedatastr, "%d", data);
	     break;
	 case FB_DEVICE_TYPE_TEMP_HUM:
	 case FB_DEVICE_TYPE_TEMP_HUM_2:
	     sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_TEMP_HUM);
		 if(attr == ENN_DEVICE_ATTR_TEMP_VALUE)
		 {
		     double data1 = data;
			 data1 = data1/100;
			 sprintf(devicedatastr, "%2.2f", data1);
			 
		     uuid = ENN_DEVICE_ATTR_TEMP_VALUE;
		 }
		 else{
		     uuid = ENN_DEVICE_ATTR_HUM_VALUE;
			 sprintf(devicedatastr, "%d", data);
		}
	     break;
		 
	 case FB_DEVICE_TYPE_LEVEL_CONTROL_SWITCH:
	      sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_ON_OFF_THREE);
		  uuid = ENN_DEVICE_ATTR_ON_OFF_THREE_STATE;
		  sprintf(devicedatastr, "%d", data);
	     break;
	 case FB_DEVICE_TYPE_POWER_OUTLET:
	      sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_POWER_OUTLET);
		  uuid = ENN_DEVICE_ATTR_POWER_OUTLET;
		  sprintf(devicedatastr, "%d", data);
	     break;
	 case FB_DEVICE_TYPE_WINDOWS:
	      sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_WINDOWS);
		  uuid = ENN_DEVICE_ATTR_WINDOWS_VALUE;
		  sprintf(devicedatastr, "%d", data);
	     break;
	 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP:
	 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP_2:
	     sprintf(devicetypestr, "%d", ENN_DEVICE_TYPE_COLOR_TEMP_LAMP);
		 if(attr == ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_STATE){
		     uuid = ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_STATE;
			 sprintf(devicedatastr, "%d", data);
		 }
		 else if(attr == ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_BRIGHTNESS_VALUE){
		     uuid = ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_BRIGHTNESS_VALUE;
			 data = (data)*100/255 ;
			 sprintf(devicedatastr, "%d", data);
		 }
		 else{
		     uuid = ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_COLOR_TEMP_VALUE;
			 if(data < 2700)
			     data =2700;
			 if(data > 6500)
			     data =6500;
			 data = (data - 2700)*100/(6500-2700) + 1 ;
			 sprintf(devicedatastr, "%d", data);
		}
	     break;
		 
	 default:
	     sprintf(devicedatastr, "%d", data);
	     break;
	}
	blobmsg_add_string( &b, "devicetype", devicetypestr);
	
	blobmsg_add_u32( &b, "uuid", uuid );
	
	blobmsg_add_string( &b, "value", devicedatastr);	

	printf("[sendMsgToMQTT]ubus_invoke data = %s\r\n", devicedatastr);
	/**/
	ubus_invoke( ctx, id, "report", b.head, test_data_cback, 0, 3000);
	
    /**/
	ubus_free(ctx);
	return 0;
	
}

int  addDeviceToMQTT(w26n_uint16 deviceId, w26n_char *ieeestr, w26n_uint8 endpoint)
{
    int  iret;
    uint32_t  id;
    struct ubus_context *ctx;
	static struct blob_buf b;
	void *e;
 
    printf("[addDeviceToMQTT] start\r\n");


    /**/
	ctx = ubus_connect( NULL );
	if ( NULL == ctx) 
	{
	    fprintf(stderr, "Failed to connect to ubus\n");
	    return -1;
	}

    //printf("[sendMsgToWeb] start--1\r\n");

    /**/
	if ( ubus_lookup_id(ctx, "we26n_mtbridge", &id) ) {
		fprintf(stderr, "Failed to look up we26n_mtbridge object\n");
		return;
	}
	//printf("[sendMsgToWeb] start--2\r\n");

	blob_buf_init( &b, 0 );

    //printf("[sendMsgToWeb] start--3\r\n");
	
	char deviceidstr[64];
	sprintf(deviceidstr, "zigbee_fbee_%s_%d", ieeestr, endpoint);
    //printf("[sendMsgToWeb] start--%s\r\n", deviceidstr);
	blobmsg_add_string( &b, "devid", deviceidstr);
	
	char devicetypestr[16];
	char deviceattrstr[16];
	
	switch(deviceId)
	{
	 case FB_DEVICE_TYPE_GAS:
		  e = blobmsg_open_array(&b, "attrs");
		  blobmsg_add_u32(&b, NULL, ENN_DEVICE_ATTR_GAS_ALERT);
	      blobmsg_close_array(&b, e);
	     break;
	 case FB_DEVICE_TYPE_MAGNETIC_DOOR:
		  e = blobmsg_open_array(&b, "attrs");
		  blobmsg_add_u32(&b, NULL, ENN_DEVICE_ATTR_MAGNETIC_DOOR_ALERT);
	      blobmsg_close_array(&b, e);
	     break;
	 case FB_DEVICE_TYPE_BODY_INFRARED:
		  e = blobmsg_open_array(&b, "attrs");
		  blobmsg_add_u32(&b, NULL, ENN_DEVICE_ATTR_BODY_INFRARED);
	      blobmsg_close_array(&b, e);
	     break;
	 case FB_DEVICE_TYPE_TEMP_HUM:
	 case FB_DEVICE_TYPE_TEMP_HUM_2:
		  e = blobmsg_open_array(&b, "attrs");
		  blobmsg_add_u32(&b, NULL, ENN_DEVICE_ATTR_TEMP_VALUE);
		  blobmsg_add_u32(&b, NULL, ENN_DEVICE_ATTR_HUM_VALUE);
	      blobmsg_close_array(&b, e);
	     break;
		 
	 case FB_DEVICE_TYPE_LEVEL_CONTROL_SWITCH:
		  e = blobmsg_open_array(&b, "attrs");
		  blobmsg_add_u32(&b, NULL, ENN_DEVICE_ATTR_ON_OFF_THREE_STATE);
	      blobmsg_close_array(&b, e);
	     break;
	 case FB_DEVICE_TYPE_POWER_OUTLET:
		  e = blobmsg_open_array(&b, "attrs");
		  blobmsg_add_u32(&b, NULL, ENN_DEVICE_ATTR_POWER_OUTLET);
	      blobmsg_close_array(&b, e);
	     break;
	 case FB_DEVICE_TYPE_WINDOWS:
		  e = blobmsg_open_array(&b, "attrs");
		  blobmsg_add_u32(&b, NULL, ENN_DEVICE_ATTR_WINDOWS_VALUE);
	      blobmsg_close_array(&b, e);
	     break;
	 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP:
	 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP_2:
		  e = blobmsg_open_array(&b, "attrs");
		  blobmsg_add_u32(&b, NULL, ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_STATE);
		  blobmsg_add_u32(&b, NULL, ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_BRIGHTNESS_VALUE);
		  blobmsg_add_u32(&b, NULL, ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_COLOR_TEMP_VALUE);
	      blobmsg_close_array(&b, e);
	     break;
		 
	 default:
	     break;
	}

	/**/
	ubus_invoke( ctx, id, "add_device", b.head, test_data_cback, 0, 3000);

    /**/
	ubus_free(ctx);
	return 0;
	
}


int receiveDeviceMsg(char *buf, int len)
{
	char *buffer;
	int index = 0;
	int iii=0;
	int resp_length=0;
	
	resp_length = len;
    printf("[receiveDeviceMsg] start"); 
	//while(1)
	{
			printf("[receiveDeviceMsg]resp_length=%d\r\n",resp_length);

			index = 0;
            while(resp_length > index){
            	buffer = buf + index;
				w26n_byte resptype = buffer[0];
				printf("[receiveDeviceMsg]resptype=%d\r\n",resptype);
				//printf("[receiveDeviceMsg]index=%d\r\n",index);
				if(resptype == RPCS_GET_DEVICES_RSP)
			   {

					w26n_byte resplen = buffer[1];
					//printf("[receiveDeviceMsg]resplen=%d\r\n",resplen);
					index = index + resplen + 2;
                    
					if(g_devices_count >= MAX_DEVICES)
					    continue;
					
					g_devices[g_devices_count].shortaddr = (buffer[2]&0xFF)|((buffer[3]&0xFF)<<8);
					//printf("buffer[2] = 0x%x", buffer[2]&0xFF);
					//printf("buffer[3] = 0x%x", buffer[3]);
					//printf("[receiveDeviceMsg]shortaddr=%d\r\n",g_devices[g_devices_count].shortaddr);

					g_devices[g_devices_count].endpoint = buffer[4];
					printf("[receiveDeviceMsg]endpoint=%d\r\n",g_devices[g_devices_count].endpoint);
                    

					g_devices[g_devices_count].profileId = (buffer[5]&0xFF)|((buffer[6]&0xFF)<<8);
					//printf("buffer[5] = 0x%x", buffer[5]);
					//printf("buffer[6] = 0x%x", buffer[6]);

					//printf("[receiveDeviceMsg]profileId=%d\r\n",g_devices[g_devices_count].profileId);

					g_devices[g_devices_count].deviceId = (buffer[7]&0xFF)|((buffer[8]&0xFF)<<8);
		            //printf("buffer[7] = 0x%x", buffer[7]);
		            //printf("buffer[8] = 0x%x", buffer[8]);

					printf("[receiveDeviceMsg]deviceId=%d\r\n",g_devices[g_devices_count].deviceId);

					g_devices[g_devices_count].namelen = buffer[10];
					//printf("[receiveDeviceMsg]namelen=%d\r\n",g_devices[g_devices_count].namelen);

					memcpy(g_devices[g_devices_count].name, &buffer[11], g_devices[g_devices_count].namelen);
					g_devices[g_devices_count].name[g_devices[g_devices_count].namelen] = 0;
					//printf("[receiveDeviceMsg]name=%s\r\n",g_devices[g_devices_count].name);

					g_devices[g_devices_count].status = buffer[11 + g_devices[g_devices_count].namelen];
					//printf("[receiveDeviceMsg]status=%d\r\n",g_devices[g_devices_count].status);

					w26n_byte IEEE[8];
					memcpy(&IEEE[0], &buffer[12 + g_devices[g_devices_count].namelen], 8);
					memcpy(&g_devices[g_devices_count].IEEE[0], &buffer[12 + g_devices[g_devices_count].namelen], 8);
                                        sprintf(g_devices[g_devices_count].ieeestr,"%02x%02x%02x%02x%02x%02x%02x%02x", IEEE[0], IEEE[1], IEEE[2], IEEE[3], IEEE[4], IEEE[5], IEEE[6], IEEE[7]);

				/*	printf("[receiveDeviceMsg]ieeestr=%s\n",g_devices[g_devices_count].ieeestr);
					for(iii=0;iii<=7;iii++)
					{
						printf("\n");
						printf("%02x",IEEE[iii]);
						printf("\n");
					}*/

                                        printf("[receiveDeviceMsg]IEEE=%s\r\n",g_devices[g_devices_count].ieeestr);

					g_devices[g_devices_count].SNlen = buffer[20 + g_devices[g_devices_count].namelen];
                                        if(g_devices[g_devices_count].SNlen > 100)
                                            g_devices[g_devices_count].SNlen = 100;

					//printf("[receiveDeviceMsg]SNlen=%d\r\n",g_devices[g_devices_count].SNlen);



					memcpy(g_devices[g_devices_count].SN, &buffer[21 + g_devices[g_devices_count].namelen], g_devices[g_devices_count].SNlen);
					g_devices[g_devices_count].SN[g_devices[g_devices_count].SNlen] = 0;
                   
					//printf("[receiveDeviceMsg]SN=%s\r\n",g_devices[g_devices_count].SN);
					
					
					int i;
					int olddevice = 0;
                    for(i = 0; i < g_devices_count; i++)
					{
						//printf("[receiveDeviceMsg] i  endpoint=%d\r\n",g_devices[i].endpoint);

						if(g_devices[g_devices_count].endpoint == g_devices[i].endpoint && (strcmp(g_devices[i].ieeestr,  g_devices[g_devices_count].ieeestr) == 0))
						{
							printf("[receiveDeviceMsg] find old device\r\n");
							g_devices[i].shortaddr = g_devices[g_devices_count].shortaddr;
							g_devices[i].status = g_devices[g_devices_count].status;
							olddevice = 1;
						    if(g_openStatus[i] != 5 && g_devices[g_devices_count].status == 0)
							{
							//	g_openStatus[i] = 5;

								printf("status=%d", g_openStatus[i]);
							    //sendMsgToWeb(g_devices[i].deviceId, g_devices[i].ieeestr, g_devices[i].endpoint, 0, g_openStatus[i]);
							}
							else if(g_openStatus[i] == 5 && g_devices[g_devices_count].status != 0)
							{
							    if(g_devices[i].deviceId == FB_DEVICE_TYPE_LEVEL_CONTROL_SWITCH)
					                g_openStatus[i] = 0;
								else
								    g_openStatus[i] = 1;
								printf("status=%d", g_openStatus[i]);

							    sendMsgToWeb(g_devices[i].deviceId, g_devices[i].ieeestr, g_devices[i].endpoint, 0, g_openStatus[i] );
							}

							break;
						}
					}
					if(olddevice)
						continue;
					
				    printf("[receiveDeviceMsg] find new device\r\n");
					
					addDevice(g_devices[g_devices_count].deviceId, g_devices[g_devices_count].ieeestr, g_devices[g_devices_count].endpoint);
					addDeviceToMQTT(g_devices[g_devices_count].deviceId, g_devices[g_devices_count].ieeestr, g_devices[g_devices_count].endpoint);
					
                    if(g_devices[g_devices_count].status != 0)
					{
                        if(g_devices[g_devices_count].deviceId == FB_DEVICE_TYPE_LEVEL_CONTROL_SWITCH)
						    g_openStatus[g_devices_count] = 0;
					    else
						    g_openStatus[g_devices_count] = 1;
					    printf("status=%d", g_openStatus[g_devices_count]);
					    sendMsgToWeb(g_devices[g_devices_count].deviceId, g_devices[g_devices_count].ieeestr, g_devices[g_devices_count].endpoint, 0, g_openStatus[g_devices_count] );
                    }
					else{
						 g_openStatus[g_devices_count] = 5;
			             printf("status=%d", g_openStatus[g_devices_count]);
					     //sendMsgToWeb(g_devices[g_devices_count].deviceId, g_devices[g_devices_count].ieeestr, g_devices[g_devices_count].endpoint, 0, g_openStatus[g_devices_count]);
					}
					g_devices_count++;
			   }

				else if(resptype == RPCS_GET_DEV_STATE_RSP)
				{
					printf("[receiveDeviceMsg]RPCS_GET_DEV_STATE_RSP\r\n");


					w26n_byte resplen = buffer[1];
					//printf("[receiveDeviceMsg]resplen=%d\r\n",resplen);
					index = index + resplen + 2;

					int shortaddr = (buffer[2]&0xFF )|((buffer[3]&0xFF)<<8);
					//printf("[receiveDeviceMsg]shortaddr=%d\r\n",shortaddr);

					w26n_byte endpoint = buffer[4];
					//printf("[receiveDeviceMsg]endpoint=%d\r\n",endpoint);

					int i, index = -1;
                    for(i = 0; i < g_devices_count; i++)
					{
						//printf("[receiveDeviceMsg] report i  endpoint=%d\r\n",g_devices[i].endpoint);
						 //printf("[receiveDeviceMsg] report i  shortaddr=%d\r\n",g_devices[i].shortaddr);

						if(shortaddr == g_devices[i].shortaddr && endpoint == g_devices[i].endpoint)
						{
							printf("[receiveDeviceMsg]report find device\r\n");
							index = i;
							break;
						}
					}	
					if(index == -1)
						continue;
				
					w26n_byte status = buffer[5];
					printf("[receiveDeviceMsg]status=%d g_openStatus[index]=%d\r\n",status, g_openStatus[index]);
					if(g_openStatus[index] == status)
					    continue;
					g_openStatus[index] = status;
					
					switch(g_devices[index].deviceId)
					{
					 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP:
					 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP_2:
                         sendMsgToWeb(g_devices[index].deviceId, g_devices[index].ieeestr, g_devices[index].endpoint, ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_STATE, status);
						 break;
					 default:
					     sendMsgToWeb(g_devices[index].deviceId, g_devices[index].ieeestr, g_devices[index].endpoint, 0, status);
						 break;
					}
					
				}
				else if(resptype == RPCS_GET_DEV_LEVEL_RSP)
				{
					//printf("[receiveDeviceMsg]resptype=%d\r\n",resptype);
					 printf("[receiveDeviceMsg]RPCS_GET_DEV_LEVEL_RSP\r\n");


					w26n_byte resplen = buffer[1];
					//printf("[receiveDeviceMsg]resplen=%d\r\n",resplen);
					index = index + resplen + 2;

					int shortaddr = (buffer[2]&0xFF )|((buffer[3]&0xFF)<<8);
					//printf("[receiveDeviceMsg]shortaddr=%d\r\n",shortaddr);

					w26n_byte endpoint = buffer[4];
					//printf("[receiveDeviceMsg]endpoint=%d\r\n",endpoint);

					int i, index = -1;
                    for(i = 0; i < g_devices_count; i++)
					{
						//printf("[receiveDeviceMsg] report i  endpoint=%d\r\n",g_devices[i].endpoint);
						 //printf("[receiveDeviceMsg] report i  shortaddr=%d\r\n",g_devices[i].shortaddr);

						if(shortaddr == g_devices[i].shortaddr && endpoint == g_devices[i].endpoint)
						{
							printf("[receiveDeviceMsg]report find device\r\n");
							index = i;
							break;
						}
					}	
				    if(index == -1)
						continue;

					w26n_byte level = buffer[5];
					printf("[receiveDeviceMsg]level=%d g_level[index]=%d\r\n",level, g_level[index]);
					if(g_level[index] == level)
					    continue;
					g_level[index] = level;
					switch(g_devices[index].deviceId)
					{
					 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP:
					 case FB_DEVICE_TYPE_COLOR_TEMP_LAMP_2:
                         sendMsgToWeb(g_devices[index].deviceId, g_devices[index].ieeestr, g_devices[index].endpoint, ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_BRIGHTNESS_VALUE, level);
						 break;
					 default:
					     sendMsgToWeb(g_devices[index].deviceId, g_devices[index].ieeestr, g_devices[index].endpoint, 0, level);
						 break;
					}
				}
				else if(resptype == RPCS_GET_COLORTMP_RSP)
				{
					//printf("[receiveDeviceMsg]resptype=%d\r\n",resptype);
					 printf("[receiveDeviceMsg]RPCS_GET_DEV_COLORTMP_RSP\r\n");



					w26n_byte resplen = buffer[1];
					//printf("[receiveDeviceMsg]resplen=%d\r\n",resplen);
					index = index + resplen + 2;

					int shortaddr = (buffer[2]&0xFF )|((buffer[3]&0xFF)<<8);
					//printf("[receiveDeviceMsg]shortaddr=%d\r\n",shortaddr);

					w26n_byte endpoint = buffer[4];
					//printf("[receiveDeviceMsg]endpoint=%d\r\n",endpoint);

					int i, index = -1;
                    for(i = 0; i < g_devices_count; i++)
					{
						//printf("[receiveDeviceMsg] report i  endpoint=%d\r\n",g_devices[i].endpoint);
						 //printf("[receiveDeviceMsg] report i  shortaddr=%d\r\n",g_devices[i].shortaddr);

						if(shortaddr == g_devices[i].shortaddr && endpoint == g_devices[i].endpoint)
						{
							printf("[receiveDeviceMsg]report find device\r\n");
							index = i;
							break;
						}
					}	
				     if(index == -1)
						 continue;
					 printf("[receiveDeviceMsg]buffer[5]=%x\r\n",buffer[5]);
					 printf("[receiveDeviceMsg]buffer[6]=%x\r\n",buffer[6]);

					 w26n_uint16 colortmp = ((1000000/((buffer[5]&0xFF)|((buffer[6]&0xFF)<<8)))/100)*100;
					 printf("[receiveDeviceMsg]colortmp=%d\r\n",colortmp);
					 
					printf("[receiveDeviceMsg]colortmp=%d g_colorTmp[index]=%d\r\n",colortmp, g_colorTmp[index]);
					if(g_colorTmp[index] == colortmp)
					    continue;
					g_colorTmp[index] = colortmp;
					 
                     sendMsgToWeb(g_devices[index].deviceId, g_devices[index].ieeestr, g_devices[index].endpoint, ENN_DEVICE_ATTR_COLOR_TEMP_LAMP_COLOR_TEMP_VALUE, colortmp);
				}
				else if(resptype == RPCS_DEVICE_REPORT)
				{
					//printf("[receiveDeviceMsg]report resptype=%d\r\n",resptype);

                                        printf("[receiveDeviceMsg] RPCS_DEVICE_REPORT \r\n");


					w26n_byte resplen = buffer[1];
					//printf("[receiveDeviceMsg]report resplen=%d\r\n",resplen);
					index = index + resplen + 2;

                    //printf("buffer[2]= 0x%x", buffer[2]);
					//printf("buffer[3]= 0x%x", buffer[3]);


					int shortaddr = (buffer[2]&0xFF)|((buffer[3]&0xFF)<<8);
					printf("[receiveDeviceMsg]report shortaddr=%d\r\n",shortaddr);

					w26n_byte endpoint = buffer[4];
					printf("[receiveDeviceMsg]report endpoint=%d\r\n",endpoint);

					int clusterId = (buffer[5]&0xFF)|((buffer[6]&0xFF)<<8);
					printf("[receiveDeviceMsg]report clusterId=%d\r\n",clusterId);
                    

					int i, index = -1;
                                        for(i = 0; i < g_devices_count; i++)
					{
						//printf("[receiveDeviceMsg] report i  endpoint=%d\r\n",g_devices[i].endpoint);
						 //printf("[receiveDeviceMsg] report i  shortaddr=%d\r\n",g_devices[i].shortaddr);

						if(shortaddr == g_devices[i].shortaddr && endpoint == g_devices[i].endpoint)
						{
							printf("[receiveDeviceMsg]report find device\r\n");
							index = i;
							break;
						}
					}
                                        if(index == -1)
						continue;


					if(g_devices[index].deviceId == FB_DEVICE_TYPE_GAS)
					{
					
					    printf("[receiveDeviceMsg]FB_DEVICE_TYPE_GAS\r\n");
						w26n_byte num = buffer[7];
						printf("[receiveDeviceMsg]num=%d\r\n",num);

						int attr = (buffer[8]&0xFF)|((buffer[9]&0xFF)<<8);
						printf("[receiveDeviceMsg]attr=%d\r\n",attr);

						w26n_byte type = buffer[10];
						printf("[receiveDeviceMsg]type=%d\r\n",type);

						w26n_byte value = buffer[11];
						printf("[receiveDeviceMsg] value=%d\r\n",value);
						
						sendMsgToWeb(g_devices[index].deviceId, g_devices[index].ieeestr, g_devices[index].endpoint, 0, value);
						sendMsgToMQTT(g_devices[index].deviceId, g_devices[index].ieeestr, g_devices[index].endpoint, 0, value);
					}
					
					else if(g_devices[index].deviceId == FB_DEVICE_TYPE_MAGNETIC_DOOR)
					{

						printf("[receiveDeviceMsg]FB_DEVICE_TYPE_MAGNETIC_DOOR\r\n");
						w26n_byte num = buffer[7];
						printf("[receiveDeviceMsg]num=%d\r\n",num);

						int attr = (buffer[8]&0xFF)|((buffer[9]&0xFF)<<8);
						printf("[receiveDeviceMsg]attr=%d\r\n",attr);

						w26n_byte type = buffer[10];
						printf("[receiveDeviceMsg]type=%d\r\n",type);

						w26n_byte value = buffer[11];
						printf("[receiveDeviceMsg] value=%d\r\n",value);
						
						sendMsgToWeb(g_devices[index].deviceId, g_devices[index].ieeestr, g_devices[index].endpoint, ENN_DEVICE_ATTR_MAGNETIC_DOOR_ALERT, value);
						sendMsgToMQTT(g_devices[index].deviceId, g_devices[index].ieeestr, g_devices[index].endpoint, ENN_DEVICE_ATTR_MAGNETIC_DOOR_ALERT, value);
					}
					else if(g_devices[index].deviceId == FB_DEVICE_TYPE_BODY_INFRARED)
					{

						printf("[receiveDeviceMsg]FB_DEVICE_TYPE_BODY_INFRARED\r\n");
						w26n_byte num = buffer[7];
						printf("[receiveDeviceMsg]num=%d\r\n",num);

						int attr = (buffer[8]&0xFF)|((buffer[9]&0xFF)<<8);
						printf("[receiveDeviceMsg]attr=%d\r\n",attr);

						w26n_byte type = buffer[10];
						printf("[receiveDeviceMsg]type=%d\r\n",type);

						w26n_byte value = buffer[11];
						printf("[receiveDeviceMsg] value=%d\r\n",value);
						if(value == 1 || g_openStatus[index] == 1 )
                                                {
						    sendMsgToWeb(g_devices[index].deviceId, g_devices[index].ieeestr, g_devices[index].endpoint, ENN_DEVICE_ATTR_BODY_INFRARED, value);
						    sendMsgToMQTT(g_devices[index].deviceId, g_devices[index].ieeestr, g_devices[index].endpoint, ENN_DEVICE_ATTR_BODY_INFRARED, value);
                                                }
                                                g_openStatus[index] = value; 
					}

					else if(g_devices[index].deviceId == FB_DEVICE_TYPE_TEMP_HUM || g_devices[index].deviceId == FB_DEVICE_TYPE_TEMP_HUM_2)
					{
					    printf("[receiveDeviceMsg]FB_DEVICE_TYPE_TEMP_HUM\r\n");
						w26n_byte num = buffer[7];
						printf("[receiveDeviceMsg]num=%d\r\n",num);

						w26n_uint16 attr = (buffer[8]&0xFF)|((buffer[9]&0xFF)<<8);
						printf("[receiveDeviceMsg]attr=%d\r\n",attr);

						w26n_byte type = buffer[10];
						printf("[receiveDeviceMsg]type=%d\r\n",type);

						w26n_uint16 value = (buffer[11]&0xFF)|((buffer[12]&0xFF)<<8);
						printf("[receiveDeviceMsg] value=%d\r\n",value);

						w26n_uint16 attr1 = (buffer[13]&0xFF)|((buffer[14]&&0xFF)<<8);
						printf("[receiveDeviceMsg]attr1=%d\r\n",attr);

						w26n_byte type1 = buffer[15];
						printf("[receiveDeviceMsg]type1=%d\r\n",type1);

						w26n_byte value1 = buffer[16];
						printf("[receiveDeviceMsg] value1=%d\r\n",value1);
						
						printf("\n**********************\n");
						dump_hex( buffer, 17 );
						printf("\n**********************\n");
                        	                if(type == 41 && num == 1)
                        	                {		
                        	                   if(humcount == 0 && value == 1)
                        	                   {
                        	                       humcount = 1;
                        	                       printf("in 41\n");
                        	                       continue;
                        	                   }
                        	                    printf("after 41\n");	
						    sendMsgToWeb(g_devices[index].deviceId, g_devices[index].ieeestr, g_devices[index].endpoint, ENN_DEVICE_ATTR_TEMP_VALUE, value);
						}
						else if(type == 33 && num ==1)
						{   
						    if(humcount == 0 && value == 1)
                        	                   { 
                                                       printf("in 33\n");
                        	                       humcount = 1;
                        	                       continue;
                        	                   }	
                        	                   printf("in 33\n");
					           sendMsgToWeb(g_devices[index].deviceId, g_devices[index].ieeestr, g_devices[index].endpoint, ENN_DEVICE_ATTR_HUM_VALUE, value/100);
					         }
						        /*
						        else if(num == 2 && type != 32)
						        {
						             sendMsgToWeb(g_devices[index].deviceId, g_devices[index].ieeestr, g_devices[index].endpoint, ENN_DEVICE_ATTR_TEMP_VALUE, value);
						              sendMsgToWeb(g_devices[index].deviceId, g_devices[index].ieeestr, g_devices[index].endpoint, ENN_DEVICE_ATTR_HUM_VALUE, value1);
						        }
						        */
						 
					}
					else{

						w26n_byte num = buffer[7];
						printf("[receiveDeviceMsg]num=%d\r\n",num);

						int attr = (buffer[8]&0xFF)&((buffer[9]&0xFF)<<8);
						printf("[receiveDeviceMsg]attr=%d\r\n",attr);

						w26n_byte type = buffer[10];
						printf("[receiveDeviceMsg]type=%d\r\n",type);

						w26n_byte value = buffer[11];
						printf("[receiveDeviceMsg] value=%d\r\n",value);
					}


				}
				else if(resptype == RPCS_GET_GATEDETAIL_RSP){
					int resplen = buffer[1];
					char respversion[8];
					
					printf("[receiveDeviceMsg]resplen=%d\r\n",resplen);
					index = index + resplen + 2;
					
					memcpy(respversion, &buffer[2], 5);
					respversion[5] = 0;

					printf("[getGateDetailInfo]version=%s\r\n",respversion);

					strcpy(g_Gate.version,respversion);

					int respsnid;
					memcpy(&respsnid, &buffer[7], 4);

					printf("[getGateDetailInfo]snid=0x%x\r\n",respsnid);

					char respname[21];
					memcpy(respname, &buffer[11], 20);
					respname[20] = 0;

					printf("[getGateDetailInfo]name=%s\r\n",respname);

					strcpy(g_Gate.username,respname);

					char resppass[21];
					memcpy(resppass, &buffer[31], 20);
					resppass[20] = 0;

					printf("[getGateDetailInfo]pass=%s\r\n",resppass);

					strcpy(g_Gate.password,resppass);
				}
				else{
					w26n_byte resplen = buffer[1];
					printf("[receiveDeviceMsg]resplen=%d\r\n",resplen);
					index = index + resplen + 2;

				}

         }
	}

}




