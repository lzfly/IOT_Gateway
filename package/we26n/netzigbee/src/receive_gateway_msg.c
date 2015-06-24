
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


int  sendMsgToWeb(void * data, int len)
{
    int  iret;
    uint32_t  id;
    struct ubus_context *ctx;
	static struct blob_buf b;
	
	blob_buf_init( &b, 0 );
	blobmsg_add_u32( &b, "args",  1234 );
	blobmsg_add_u32( &b, "argv",  5678 );
	
    /**/
	ctx = ubus_connect( NULL );
	if ( NULL == ctx) 
	{
	    fprintf(stderr, "Failed to connect to ubus\n");
	    return -1;
	}

    /**/
	if ( ubus_lookup_id(ctx, "we26n.flowmeter", &id) ) {
		fprintf(stderr, "Failed to look up flowmeter object\n");
		return;
	}
	
	/**/
	ubus_invoke( ctx, id, "info", b.head, test_data_cback, 0, 3000);

	
    /**/
	ubus_free(ctx);
	return 0;
	
}



extern int receiveMsg(int fd,char* resp_body, int *resp_length);

static int searchDeviceSocket=0;
int receiveDeviceMsg()
{

	int cmd_length=1;
	w26n_byte msg[cmd_length];

	char buffer1[512];
	char *buffer;
	int index = 0;

	int resp_length=0;
	msg[SRPC_CMD_ID_POS]=RPCS_GET_DEVICES;
    printf("[receiveDeviceMsg] start"); 
	while(1)
	{
			receiveMsg(g_monitor_socket,buffer1,&resp_length);
			printf("[receiveDeviceMsg]resp_length=%d\r\n",resp_length);

			index = 0;
            while(resp_length > index){
            	buffer = buffer1 + index;
				w26n_byte resptype = buffer[0];
				printf("[receiveDeviceMsg]resptype=%d\r\n",resptype);
				printf("[receiveDeviceMsg]index=%d\r\n",index);
				if(resptype == RPCS_GET_DEVICES_RSP)
			   {

					w26n_byte resplen = buffer[1];
					printf("[receiveDeviceMsg]resplen=%d\r\n",resplen);
					index = index + resplen + 2;
                    
					if(g_devices_count >= MAX_DEVICES)
					    continue;
					
					g_devices[g_devices_count].shortaddr = (buffer[2]&0xFF)|(buffer[3]<<8);
					printf("buffer[2] = 0x%x", buffer[2]&0xFF);
					printf("buffer[3] = 0x%x", buffer[3]);
					printf("[receiveDeviceMsg]shortaddr=%d\r\n",g_devices[g_devices_count].shortaddr);

					g_devices[g_devices_count].endpoint = buffer[4];
					printf("[receiveDeviceMsg]endpoint=%d\r\n",g_devices[g_devices_count].endpoint);
                    
					int i;
					int olddevice = 0;
                    for(i = 0; i < g_devices_count; i++)
					{
						printf("[receiveDeviceMsg] i  endpoint=%d\r\n",g_devices[i].endpoint);

						if(g_devices[g_devices_count].endpoint == g_devices[i].endpoint)
						{
							printf("[receiveDeviceMsg] find old device\r\n");
							olddevice = 1;
							break;
						}
					}
					if(olddevice)
						continue;

                    printf("[receiveDeviceMsg] find new device\r\n");

					g_devices[g_devices_count].profileId = buffer[5]|(buffer[6]<<8);
					printf("buffer[5] = 0x%x", buffer[5]);
					printf("buffer[6] = 0x%x", buffer[6]);

					printf("[receiveDeviceMsg]profileId=%d\r\n",g_devices[g_devices_count].profileId);

					g_devices[g_devices_count].deviceId = buffer[7]|(buffer[8]<<8);
		            printf("buffer[7] = 0x%x", buffer[7]);
		            printf("buffer[8] = 0x%x", buffer[8]);

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
					printf("[receiveDeviceMsg]resplen=%d\r\n",resplen);
					index = index + resplen + 2;

					int shortaddr = buffer[2]&(buffer[3]<<8);
					printf("[receiveDeviceMsg]shortaddr=%d\r\n",shortaddr);

					w26n_byte endpoint = buffer[4];
					printf("[receiveDeviceMsg]endpoint=%d\r\n",endpoint);

					w26n_byte status = buffer[5];
					printf("[receiveDeviceMsg]status=%d\r\n",status);


				}
				else if(resptype == RPCS_DEVICE_REPORT)
				{
					printf("[receiveDeviceMsg]resptype=%d\r\n",resptype);


					w26n_byte resplen = buffer[1];
					printf("[receiveDeviceMsg]resplen=%d\r\n",resplen);
					index = index + resplen + 2;

					int shortaddr = buffer[2]&(buffer[3]<<8);
					printf("[receiveDeviceMsg]shortaddr=%d\r\n",shortaddr);

					w26n_byte endpoint = buffer[4];
					printf("[receiveDeviceMsg]endpoint=%d\r\n",endpoint);

					int clusterId = buffer[5]&(buffer[6]<<8);
					printf("[receiveDeviceMsg]clusterId=%d\r\n",clusterId);


					w26n_byte num = buffer[7];
					printf("[receiveDeviceMsg]num=%d\r\n",num);

					int attr = buffer[8]&(buffer[9]<<8);
					printf("[receiveDeviceMsg]attr=%d\r\n",attr);

					w26n_byte type = buffer[10];
					printf("[receiveDeviceMsg]type=%d\r\n",type);

					w26n_byte value = buffer[11];
					printf("[receiveDeviceMsg] value=%d\r\n",value);

				}
				else{
					w26n_byte resplen = buffer[1];
					printf("[receiveDeviceMsg]resplen=%d\r\n",resplen);
					index = index + resplen + 2;

				}

         }
		sleep(10);
	}

}




