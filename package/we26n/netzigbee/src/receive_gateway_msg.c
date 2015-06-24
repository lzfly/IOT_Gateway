
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

#include "we26n_type.h"
#include "gateway.h"
#include "fbee_protocol.h"

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

	while(1)
	{
			receiveMsg(g_monitor_socket,buffer1,&resp_length);
			printf("[startSearchDevice]resp_length=%d\r\n",resp_length);

			index = 0;
            while(resp_length > index){
            	buffer = buffer1 + index;
				w26n_byte resptype = buffer[0];
				printf("[startSearchDevice]resptype=%d\r\n",resptype);
				printf("[startSearchDevice]index=%d\r\n",index);
				if(resptype == 1)
			   {

					w26n_byte resplen = buffer[1];
					printf("[startSearchDevice]resplen=%d\r\n",resplen);
					index = index + resplen + 2;

					int shortaddr = buffer[2]&(buffer[3]<<8);
					printf("[startSearchDevice]shortaddr=%d\r\n",shortaddr);

					w26n_byte endpoint = buffer[4];
					printf("[startSearchDevice]endpoint=%d\r\n",endpoint);

					int profileId = buffer[5]&(buffer[6]<<8);
					printf("[startSearchDevice]profileId=%d\r\n",profileId);

					int deviceId = buffer[7]&(buffer[8]<<8);
					printf("[startSearchDevice]deviceId=%d\r\n",deviceId);

					w26n_byte namelen = buffer[10];
					printf("[startSearchDevice]namelen=%d\r\n",namelen);

					char name[101];
					memcpy(name, &buffer[11], namelen);
					name[namelen] = 0;
					printf("[startSearchDevice]name=%s\r\n",name);

					w26n_byte status = buffer[11 + namelen];
					printf("[startSearchDevice]status=%d\r\n",status);

					w26n_byte IEEE[8];
					memcpy(&IEEE[0], &buffer[12 + namelen], 8);
					printf("[startSearchDevice]IEEE=%d\r\n",IEEE);

					w26n_byte SNlen = buffer[20 + namelen];
					printf("[startSearchDevice]SNlen=%d\r\n",SNlen);

					char SN[101];

					memcpy(SN, &buffer[21 + namelen], SNlen);
					SN[SNlen] = 0;

					printf("[startSearchDevice]SN=%s\r\n",SN);
			   }

				else if(resptype == 7)
				{
					printf("[startSearchDevice]resptype=%d\r\n",resptype);


					w26n_byte resplen = buffer[1];
					printf("[startSearchDevice]resplen=%d\r\n",resplen);
					index = index + resplen + 2;

					int shortaddr = buffer[2]&(buffer[3]<<8);
					printf("[startSearchDevice]shortaddr=%d\r\n",shortaddr);

					w26n_byte endpoint = buffer[4];
					printf("[startSearchDevice]endpoint=%d\r\n",endpoint);

					w26n_byte status = buffer[5];
					printf("[startSearchDevice]status=%d\r\n",status);


				}
				else if(resptype == 0x70)
				{
					printf("[startSearchDevice]resptype=%d\r\n",resptype);


					w26n_byte resplen = buffer[1];
					printf("[startSearchDevice]resplen=%d\r\n",resplen);
					index = index + resplen + 2;

					int shortaddr = buffer[2]&(buffer[3]<<8);
					printf("[startSearchDevice]shortaddr=%d\r\n",shortaddr);

					w26n_byte endpoint = buffer[4];
					printf("[startSearchDevice]endpoint=%d\r\n",endpoint);

					int clusterId = buffer[5]&(buffer[6]<<8);
					printf("[startSearchDevice]clusterId=%d\r\n",clusterId);


					w26n_byte num = buffer[7];
					printf("[startSearchDevice]num=%d\r\n",num);

					int attr = buffer[8]&(buffer[9]<<8);
					printf("[startSearchDevice]attr=%d\r\n",attr);

					w26n_byte type = buffer[10];
					printf("[startSearchDevice]type=%d\r\n",type);

					w26n_byte value = buffer[11];
					printf("[startSearchDevice value=%d\r\n",value);

				}
				else{
					w26n_byte resplen = buffer[1];
					printf("[startSearchDevice]resplen=%d\r\n",resplen);
					index = index + resplen + 2;

				}

         }
		sleep(10);
	}

}




