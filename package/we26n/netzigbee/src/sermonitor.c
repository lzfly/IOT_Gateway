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
#include "gateway.h"
#include "we26n_type.h"
#include "fbee_protocol.h"

#define FEIBEE_REQUEST_PREFIX_LENGTH 7


static void getSendSrpc(w26n_byte* cmd,int cmd_length,w26n_byte* msg,int *msg_length)
{
	w26n_byte bt1[FEIBEE_REQUEST_PREFIX_LENGTH+cmd_length];
	int i=0;
	bt1[0]=(FEIBEE_REQUEST_PREFIX_LENGTH+cmd_length)&0xff;
	bt1[1]=((FEIBEE_REQUEST_PREFIX_LENGTH+cmd_length)&0xff00)/0x100;

    bt1[2]=	0xff&g_Gate.snid;
    bt1[3] = (0xff00&g_Gate.snid)>>8;
    bt1[4] = (0xff0000&g_Gate.snid)>>16;
    bt1[5] = (0xff000000&g_Gate.snid)>>24;

	bt1[6] = 0xfe;
    printf("bit[0] = 0x%x", bt1[0]);
	printf("bit[1] = 0x%x", bt1[1]);
	printf("bit[2] = 0x%x", bt1[2]);
	printf("bit[3] = 0x%x", bt1[3]);
	printf("bit[4] = 0x%x", bt1[4]);
	printf("bit[5] = 0x%x", bt1[5]);
	printf("bit[6] = 0x%x", bt1[6]);

    for(i=0;i<cmd_length;i++)
    {
    	bt1[FEIBEE_REQUEST_PREFIX_LENGTH+i]=cmd[i];
		printf("bt1[%d] = 0x%x", FEIBEE_REQUEST_PREFIX_LENGTH+i, cmd[i]);
    }
    memcpy(msg, bt1, FEIBEE_REQUEST_PREFIX_LENGTH+cmd_length);
    *msg_length=FEIBEE_REQUEST_PREFIX_LENGTH+cmd_length;
}

int sendCommandRevMsg(int fd,w26n_byte* cmd, int cmd_length,char* resp_body, int *resp_length)
{

		char buffer[1024]={0};
		int recbytes;



		w26n_byte msg[32];
		int msg_length=0;
		getSendSrpc(cmd,cmd_length,msg,&msg_length);

		if(-1 == write(fd,msg,8))
		{
			printf("[sendCommandRevMsg]write fail!\r\n");
			return -1;
		}

		printf("[sendCommandRevMsg]write ok!\r\n");

		recbytes=read(fd,buffer,1024);
		if(-1==recbytes)
		{
			printf("[sendCommandRevMsg]read data fail!");
			return -1;
		}

		memcpy(resp_body,buffer,recbytes);

		*resp_length=recbytes;
	//	buffer[recbytes]='\0';

		printf("[sendCommandRevMsg]recbytes=%d\r\n",recbytes);

		//close(cfd);
		return 0;

}


int sendCommand(int fd,w26n_byte* cmd, int cmd_length)
{

		char buffer[1024]={0};
		int recbytes;



		w26n_byte msg[128];
		int msg_length=0;
		getSendSrpc(cmd,cmd_length,msg,&msg_length);

        printf("msg[0] = 0x%x", msg[0]);
		printf("msg[0] = 0x%x", msg[0]);
		printf("msg[0] = 0x%x", msg[0]);
		printf("msg[0] = 0x%x", msg[0]);
		printf("msg[0] = 0x%x", msg[0]);
		printf("msg[0] = 0x%x", msg[0]);
		printf("msg[0] = 0x%x", msg[0]);


		if(-1 == write(fd,msg,msg_length))
		{
			printf("[sendCommand]write fail!\r\n");
			return -1;
		}

		printf("[sendCommand]write ok!\r\n");

		return 0;

}

int receiveMsg(int fd,char* resp_body, int *resp_length)
{

		char buffer[1024]={0};
		int recbytes;


		recbytes=read(fd,buffer,1024);
		if(-1==recbytes)
		{
			printf("[reveiveMsg]read data fail!");
			return -1;
		}

		memcpy(resp_body,buffer,recbytes);

		*resp_length=recbytes;
	//	buffer[recbytes]='\0';

		printf("[reveiveMsg]recbytes=%d\r\n",recbytes);

		//close(cfd);
		return 0;

}


extern void *  receiveDeviceMsg();
extern void *  ctrlDevice();


int server_monitor()
{

	int sin_size;
	struct sockaddr_in s_add,c_add;
	unsigned short portnum=8001;
	printf("into method sendCommand!\r\n");
	g_monitor_socket=socket(AF_INET,SOCK_STREAM,0);
	if(-1==g_monitor_socket)
	{
		printf("[server_monitor] socket create failed\r\n");
		return -1;
	}
	printf("[server_monitor]socket create ok\r\n");

	bzero(&s_add,sizeof(s_add));
	s_add.sin_family=AF_INET;
	s_add.sin_addr.s_addr=inet_addr(g_Gate.IP);
	s_add.sin_port=htons(portnum);

	if(-1==connect(g_monitor_socket,(struct sockaddr*)(&s_add),sizeof(struct sockaddr)))
	{
		printf("[server_monitor]connect fail!");
		return -1;
	}
	printf("[server_monitor]connect ok! \r\n");

    int  iret;
	pthread_t  aux_thrd;


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

		return 0;
}

