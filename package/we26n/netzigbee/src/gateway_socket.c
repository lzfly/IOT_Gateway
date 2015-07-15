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

#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h> 
#include <sys/ioctl.h> 

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include "gateway.h"
#include "we26n_type.h"
#include "fbee_protocol.h"
#include "gateway_socket.h"

#define FEIBEE_REQUEST_PREFIX_LENGTH 7

int g_monitor_socket;

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
		printf("bt1[%d] = 0x%x\r\n", FEIBEE_REQUEST_PREFIX_LENGTH+i, cmd[i]);
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


		w26n_byte msg[128];
		int msg_length=0;
		getSendSrpc(cmd,cmd_length,msg,&msg_length);


		if(-1 == write(fd,msg,msg_length))
		{
			printf("[sendCommand]write fail!\r\n");
                        sendCmdFailCount++;
			return 2;
		}

		printf("[sendCommand]write ok!\r\n");

		return 0;

}

int connectGateway()
{
	int sin_size;
	struct sockaddr_in s_add,c_add;
	unsigned short portnum=8001;
	printf("into method sendCommand!\r\n");
	g_monitor_socket=socket(AF_INET,SOCK_STREAM,0);
	if(-1==g_monitor_socket)
	{
		printf("[connectGateway] socket create failed\r\n");
		return -1;
	}
	printf("[connectGateway]socket create ok\r\n");

	bzero(&s_add,sizeof(s_add));
	s_add.sin_family=AF_INET;
	s_add.sin_addr.s_addr=inet_addr(g_Gate.IP);
	s_add.sin_port=htons(portnum);

	if(-1==connect(g_monitor_socket,(struct sockaddr*)(&s_add),sizeof(struct sockaddr)))
	{
		printf("[connectGateway]connect fail!");
		return -1;
	}
	printf("[connectGateway]connect ok! \r\n");
	
	return 0;

}

int disConnectGateway()
{

	close(g_monitor_socket);

	printf("[disConnectGateway]disconnect ok! \r\n");
	
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


#define MAXINTERFACES   16
int getLocalIPandMAC ()
{
   register int fd, intrface, retn = 0;
   struct ifreq buf[MAXINTERFACES];
   struct arpreq arp;
   struct ifconf ifc;


printf ("getLocalIPandMAC\n");


if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0)
{
  ifc.ifc_len = sizeof buf;
  ifc.ifc_buf = (caddr_t) buf;
  if (!ioctl (fd, SIOCGIFCONF, (char *) &ifc))
  {
   //获取接口信息
   intrface = ifc.ifc_len / sizeof (struct ifreq);
    printf("interface num is intrface=%d\n\n\n",intrface);
   //根据借口信息循环获取设备IP和MAC地址
   while (intrface-- > 0)
   {
    //获取设备名称
    printf ("net device %s\n", buf[intrface].ifr_name);
	
	if(0 != strncmp(buf[intrface].ifr_name, "br-lan", strlen("br-lan")))
	    continue;


   //判断网卡状态
            if (buf[intrface].ifr_flags & IFF_UP)
   {
                printf("the interface status is UP" );
            }
            else
   {
                printf("the interface status is DOWN" );
            }
   //获取当前网卡的IP地址
            if (!(ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface])))
            {
                 printf ("IP address is:" );
                 printf("%08x", ((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr);
                 g_localAddr.sin_addr = ((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr;				 
                 printf("" );
                   //puts (buf[intrface].ifr_addr.sa_data);
            }
            else
           {
               char str[256];
               sprintf (str, "cpm: ioctl device %s", buf[intrface].ifr_name);
               perror (str);
           }
/* this section can't get Hardware Address,I don't know whether the reason is module driver*/

            if (!(ioctl (fd, SIOCGIFHWADDR, (char *) &buf[intrface])))
            {
                 printf ("HW address is:" );
                 printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                                (unsigned char)buf[intrface].ifr_hwaddr.sa_data[0],
                                (unsigned char)buf[intrface].ifr_hwaddr.sa_data[1],
                                (unsigned char)buf[intrface].ifr_hwaddr.sa_data[2],
                                (unsigned char)buf[intrface].ifr_hwaddr.sa_data[3],
                                (unsigned char)buf[intrface].ifr_hwaddr.sa_data[4],
                                (unsigned char)buf[intrface].ifr_hwaddr.sa_data[5]);
				 sprintf(g_localMAC, "%02X%02X%02X%02X%02X%02X",
                                (unsigned char)buf[intrface].ifr_hwaddr.sa_data[0],
                                (unsigned char)buf[intrface].ifr_hwaddr.sa_data[1],
                                (unsigned char)buf[intrface].ifr_hwaddr.sa_data[2],
                                (unsigned char)buf[intrface].ifr_hwaddr.sa_data[3],
                                (unsigned char)buf[intrface].ifr_hwaddr.sa_data[4],
                                (unsigned char)buf[intrface].ifr_hwaddr.sa_data[5]);
                 printf("g_localMAC=%s\n", g_localMAC);
                 printf("" );
             }

            else
            {
               char str[256];
               sprintf (str, "cpm: ioctl device %s", buf[intrface].ifr_name);
               printf (str);
           }
        } //while
      } else
         printf ("cpm: ioctl" );
   } else
      printf ("cpm: socket" );
    close (fd);
    return retn;
} 
