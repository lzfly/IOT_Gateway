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
#include "gatewayBean.h"
#include "we26n_type.h"
#include "FbeeControlCommand.h"

#define FEIBEE_REQUEST_PREFIX_LENGTH 7

//int client_monitor()
//{
//	int cfd;
//	int recbytes;
//	int sin_size;
//	char buffer[1024]={0};
//	struct sockaddr_in s_add,c_add;
//	unsigned short portnum=8001;
//	printf("Hello,welcome to client!\r\n");
//	cfd=socket(AF_INET,SOCK_STREAM,0);
//	if(-1==cfd)
//	{
//		printf("socket create failed\r\n");
//		return -1;
//	}
//	printf("socket create ok\r\n");
//
//	bzero(&s_add,sizeof(s_add));
//	s_add.sin_family=AF_INET;
//	s_add.sin_addr.s_addr=inet_addr(g_Gate.IP);
//	s_add.sin_port=htons(portnum);
//
//	if(-1==connect(cfd,(struct sockaddr*)(&s_add),sizeof(struct sockaddr)))
//	{
//		printf("connect fail!");
//		return -1;
//	}
//	printf("connect ok\r\n");
//
//	recbytes=read(cfd,buffer,1024);
//	if(-1==recbytes)
//	{
//		printf("read data fail!");
//		return -1;
//	}
//	buffer[recbytes]='\0';
//
//	printf("%s\r\n",buffer);
//
//	getchar();
//	close(cfd);
//	return 0;
//
//}

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

    for(i=0;i<cmd_length;i++)
    {
    	bt1[FEIBEE_REQUEST_PREFIX_LENGTH+i]=cmd[i];
    }
    memcpy(msg, bt1, FEIBEE_REQUEST_PREFIX_LENGTH+cmd_length);
    *msg_length=FEIBEE_REQUEST_PREFIX_LENGTH+cmd_length;
}

int sendCommand(int fd,w26n_byte* cmd, int cmd_length,char* resp_body, int *resp_length)
{

		char buffer[1024]={0};
		int recbytes;
//		int fd;
//
//		int sin_size;
//		struct sockaddr_in s_add,c_add;
//		unsigned short portnum=8001;
//		printf("into method sendCommand!\r\n");
//		fd=socket(AF_INET,SOCK_STREAM,0);
//		if(-1==fd)
//		{
//			printf("[server_monitor] socket create failed\r\n");
//			return -1;
//		}
//		printf("[server_monitor]socket create ok\r\n");
//
//		bzero(&s_add,sizeof(s_add));
//		s_add.sin_family=AF_INET;
//		s_add.sin_addr.s_addr=inet_addr(g_Gate.IP);
//		s_add.sin_port=htons(portnum);
//
//		if(-1==connect(fd,(struct sockaddr*)(&s_add),sizeof(struct sockaddr)))
//		{
//			printf("[server_monitor]connect fail!");
//			return -1;
//		}
//		printf("[server_monitor]connect ok! \r\n");



//		w26n_byte msg1[8];
//		msg1[2] = 0xff&g_Gate.snid;
//		msg1[3] = (0xff00&g_Gate.snid)>>8;
//		msg1[4] = (0xff0000&g_Gate.snid)>>16;
//		msg1[5] = (0xff000000&g_Gate.snid)>>24;
//        msg1[6] = 0xfe;
//        msg1[7] = RPCS_GET_GATEDETAIL;
//        msg1[0] = 0x8;
//        msg1[1] = 0;


		w26n_byte msg[32];
		int msg_length=0;
		getSendSrpc(cmd,cmd_length,msg,&msg_length);

		if(-1 == write(fd,msg,8))
		{
			printf("[sendCommand]write fail!\r\n");
			return -1;
		}

		printf("[sendCommand]write ok!\r\n");

		recbytes=read(fd,buffer,1024);
		if(-1==recbytes)
		{
			printf("[sendCommand]read data fail!");
			return -1;
		}

		memcpy(resp_body,buffer,recbytes);

		*resp_length=recbytes;
	//	buffer[recbytes]='\0';

		printf("[sendCommand]recbytes=%d\r\n",recbytes);

		//close(cfd);
		return 0;

}

//
//int server_monitor()
//{
//	int nfp;
//	struct sockaddr_in s_add,c_add;
//	int sin_size;
//	unsigned short portnum=8001;
//
//	printf("Hello,welcome to my server !\r\n");
//	g_monitor_socket = socket(AF_INET, SOCK_STREAM, 0);
//	if(-1 == g_monitor_socket)
//	{
//	    printf("socket fail ! \r\n");
//	    return -1;
//	}
//	printf("socket ok !\r\n");
//
//
//	bzero(&s_add,sizeof(struct sockaddr_in));
//	s_add.sin_family=AF_INET;
//	s_add.sin_addr.s_addr=htonl(INADDR_ANY);
//	s_add.sin_port=htons(portnum);
//
//	if(-1 == bind(g_monitor_socket,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
//	{
//	    printf("bind fail !\r\n");
//	    return -1;
//	}
//	printf("bind ok !\r\n");
//
//	if(-1 == listen(g_monitor_socket,5))
//	{
//	    printf("listen fail !\r\n");
//	    return -1;
//	}
//	printf("listen ok\r\n");
//
//	while(1)
//	{
//		sin_size = sizeof(struct sockaddr_in);
//
//		nfp = accept(g_monitor_socket, (struct sockaddr *)(&c_add), &sin_size);
//		if(-1 == nfp)
//		{
//			printf("accept fail !\r\n");
//			return -1;
//		}
//		printf("accept ok!\r\nServer start get connect from %#x : %#x\r\n",ntohl(c_add.sin_addr.s_addr),ntohs(c_add.sin_port));
//
//
////		if(-1 == write(nfp,"hello,welcome to my server \r\n",32))
////		{
////			printf("write fail!\r\n");
////			return -1;
////		}
////		printf("write ok!\r\n");
////		close(nfp);
//	}
//	//close(sfp);
//	return 0;
//}


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


		return 0;
}

