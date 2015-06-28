
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


void  test_data_cback(struct ubus_request *req, int type, struct blob_attr *msg)
{
	static const struct blobmsg_policy policy[2] = { { "args", BLOBMSG_TYPE_INT32 }, { "argv", BLOBMSG_TYPE_INT32 } };
	struct blob_attr * cur[2];
	uint32_t  a,b;
	
	/**/
	if ( type == UBUS_MSG_DATA )
	{
		/* req.priv */
		printf( "data cback, %d\n", type );
		blobmsg_parse( &policy, 2, &cur, blob_data(msg), blob_len(msg) );
		
		/**/
		a = blobmsg_get_u32( cur[0] );
		b = blobmsg_get_u32( cur[1] );
		
		printf( "a = %d, b = %d\n", a, b );
		
	}

	return;
}


int  sendMsgToWeb(w26n_uint16 deviceId, w26n_uint16 shortaddr, w26n_uint8 endpoint, w26n_uint8 atrr, w26n_uint32 data)
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

    printf("[sendMsgToWeb] start--1\r\n");

    /**/
	if ( ubus_lookup_id(ctx, "jianyou", &id) ) {
		fprintf(stderr, "Failed to look up jianyou object\n");
		return;
	}
	printf("[sendMsgToWeb] start--2\r\n");

	blob_buf_init( &b, 0 );
	blobmsg_add_string( &b, "gatewayid", "we26n_78A351111384" );
	
	char deviceidstr[64];
	sprintf(deviceidstr, "zigbee_fbee_%d_%d", shortaddr, endpoint);
	blobmsg_add_string( &b, "deviceid", deviceidstr);
	
	char devicetypestr[8];
	char deviceattrstr[16];
	switch(deviceId)
	{
	 case FB_DEVICE_TYPE_GAS:
	      sprintf(devicetypestr, "%s", ENN_DEVICE_TYPE_GAS);
		  sprintf(devicetypestr, "%d", ENN_DEVICE_ATTR_GAS_ALERT);
	     break;
	 case FB_DEVICE_TYPE_TEMP_HUM:
	 case FB_DEVICE_TYPE_TEMP_HUM_2:
	     sprintf(devicetypestr, "%s", ENN_DEVICE_TYPE_TEMP_HUM);
		 if(atrr)
		     sprintf(devicetypestr, "%d", ENN_DEVICE_ATTR_TEMP_VALUE);
		 else
		     sprintf(devicetypestr, "%d", ENN_DEVICE_ATTR_HUM_VALUE);
	     break;
	 default:
	     break;
	}
	blobmsg_add_string( &b, "devicetype", devicetypestr);
	
	blobmsg_add_string( &b, "attr", devicetypestr );
	
	char devicedatastr[64];
	sprintf(devicedatastr, "%d", data);
	blobmsg_add_string( &b, "data", devicedatastr);	

	printf("[sendMsgToWeb]ubus_invoke\r\n");
	/**/
	ubus_invoke( ctx, id, "report", b.head, test_data_cback, 0, 3000);

	
    /**/
	ubus_free(ctx);
	return 0;
	
}

int receiveDeviceMsg(char *buf, int len)
{
	char *buffer;
	int index = 0;

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
					printf("[receiveDeviceMsg]shortaddr=%d\r\n",g_devices[g_devices_count].shortaddr);

					g_devices[g_devices_count].endpoint = buffer[4];
					printf("[receiveDeviceMsg]endpoint=%d\r\n",g_devices[g_devices_count].endpoint);
                    
					int i;
					int olddevice = 0;
                    for(i = 0; i < g_devices_count; i++)
					{
						printf("[receiveDeviceMsg] i  endpoint=%d\r\n",g_devices[i].endpoint);

						if(g_devices[g_devices_count].endpoint == g_devices[i].endpoint && g_devices[g_devices_count].shortaddr == g_devices[i].shortaddr)
						{
							//printf("[receiveDeviceMsg] find old device\r\n");
							olddevice = 1;
							break;
						}
					}
					if(olddevice)
						continue;

                    printf("[receiveDeviceMsg] find new device\r\n");

					g_devices[g_devices_count].profileId = (buffer[5]&0xFF)|((buffer[6]&0xFF)<<8);
					//printf("buffer[5] = 0x%x", buffer[5]);
					//printf("buffer[6] = 0x%x", buffer[6]);

					printf("[receiveDeviceMsg]profileId=%d\r\n",g_devices[g_devices_count].profileId);

					g_devices[g_devices_count].deviceId = (buffer[7]&0xFF)|((buffer[8]&0xFF)<<8);
		            //printf("buffer[7] = 0x%x", buffer[7]);
		            //printf("buffer[8] = 0x%x", buffer[8]);

					printf("[receiveDeviceMsg]deviceId=%d\r\n",g_devices[g_devices_count].deviceId);

					g_devices[g_devices_count].namelen = buffer[10];
					printf("[receiveDeviceMsg]namelen=%d\r\n",g_devices[g_devices_count].namelen);

					memcpy(g_devices[g_devices_count].name, &buffer[11], g_devices[g_devices_count].namelen);
					g_devices[g_devices_count].name[g_devices[g_devices_count].namelen] = 0;
					printf("[receiveDeviceMsg]name=%s\r\n",g_devices[g_devices_count].name);

					g_devices[g_devices_count].status = buffer[11 + g_devices[g_devices_count].namelen];
					printf("[receiveDeviceMsg]status=%d\r\n",g_devices[g_devices_count].status);

					w26n_byte IEEE[8];
					memcpy(&IEEE[0], &buffer[12 + g_devices[g_devices_count].namelen], 8);
					memcpy(&g_devices[g_devices_count].IEEE[0], &buffer[12 + g_devices[g_devices_count].namelen], 8);
					printf("[receiveDeviceMsg]IEEE=%d\r\n",g_devices[g_devices_count].IEEE);

					g_devices[g_devices_count].SNlen = buffer[20 + g_devices[g_devices_count].namelen];
					printf("[receiveDeviceMsg]SNlen=%d\r\n",g_devices[g_devices_count].SNlen);



					memcpy(g_devices[g_devices_count].SN, &buffer[21 + g_devices[g_devices_count].namelen], g_devices[g_devices_count].SNlen);
					g_devices[g_devices_count].SN[g_devices[g_devices_count].SNlen] = 0;
                   
					printf("[receiveDeviceMsg]SN=%s\r\n",g_devices[g_devices_count].SN);
					
					g_devices_count++;
			   }

				else if(resptype == RPCS_GET_DEV_STATE_RSP)
				{
					printf("[receiveDeviceMsg]resptype=%d\r\n",resptype);


					w26n_byte resplen = buffer[1];
					//printf("[receiveDeviceMsg]resplen=%d\r\n",resplen);
					index = index + resplen + 2;

					int shortaddr = (buffer[2]&0xFF )|((buffer[3]&0xFF)<<8);
					//printf("[receiveDeviceMsg]shortaddr=%d\r\n",shortaddr);

					w26n_byte endpoint = buffer[4];
					//printf("[receiveDeviceMsg]endpoint=%d\r\n",endpoint);

					w26n_byte status = buffer[5];
					//printf("[receiveDeviceMsg]status=%d\r\n",status);


				}
				else if(resptype == RPCS_DEVICE_REPORT)
				{
					printf("[receiveDeviceMsg]report resptype=%d\r\n",resptype);

                    printf("[receiveDeviceMsg] RPCS_DEVICE_REPORT \r\n");


					w26n_byte resplen = buffer[1];
					printf("[receiveDeviceMsg]report resplen=%d\r\n",resplen);
					index = index + resplen + 2;

                    //printf("buffer[2]= 0x%x", buffer[2]);
					//printf("buffer[3]= 0x%x", buffer[3]);


					int shortaddr = (buffer[2]&0xFF)|((buffer[3]&0xFF)<<8);
					printf("[receiveDeviceMsg]report shortaddr=%d\r\n",shortaddr);

					w26n_byte endpoint = buffer[4];
					printf("[receiveDeviceMsg]report endpoint=%d\r\n",endpoint);

					int clusterId = (buffer[5]&0xFF)|((buffer[6]&0xFF)<<8);
					printf("[receiveDeviceMsg]report clusterId=%d\r\n",clusterId);
                    

					int i, index;
                    for(i = 0; i < g_devices_count; i++)
					{
						printf("[receiveDeviceMsg] report i  endpoint=%d\r\n",g_devices[i].endpoint);
						 printf("[receiveDeviceMsg] report i  shortaddr=%d\r\n",g_devices[i].shortaddr);

						if(shortaddr == g_devices[i].shortaddr && endpoint == g_devices[i].endpoint)
						{
							printf("[receiveDeviceMsg] find device\r\n");
							index = i;
							break;
						}
					}
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
						
						sendMsgToWeb(g_devices[index].deviceId, g_devices[index].shortaddr, g_devices[index].endpoint, 0, value);
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
						printf("[receiveDeviceMsg]attr=%d\r\n",attr);

						w26n_byte type1 = buffer[15];
						printf("[receiveDeviceMsg]type=%d\r\n",type);

						w26n_byte value1 = buffer[16];
						printf("[receiveDeviceMsg] value=%d\r\n",value);
						
						sendMsgToWeb(g_devices[index].deviceId, g_devices[index].shortaddr, g_devices[index].endpoint, 1, value);
						sendMsgToWeb(g_devices[index].deviceId, g_devices[index].shortaddr, g_devices[index].endpoint, 0, value1);
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




