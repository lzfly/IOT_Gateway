
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
#include "searchgway.h"

static int PORT=9090;
static char* SEARCH_GATEWAY_COMMAND="GETIP\r\n";

static int MaxWaitCount=10;

static char gateway_IP[16];
static char gateway_SN[16];

static void write_gateway_config_file(char *buf)
{
	FILE *file;

	file=fopen(GATEWAY_CONF_FILE,"w+");
	if(file==NULL)
	{
		printf("Load gateway config file failed!");
		return ;
	}

	fprintf(file,"%s",buf);
	fclose(file);

}


int gateway_conf_init()
{

	 FILE *fp;
	 int len=0;
	 char buf[512];

	 char* z_IP_Type="IP:";
	 char* z_SN_Type="SN:";

 	 FILE *file;

	 if((fp = fopen(GATEWAY_CONF_FILE,"r")) == NULL)
	 {
		printf("Failed to read config file [%s]!",GATEWAY_CONF_FILE);
		return -1;
	 }

	 while(fgets(buf,64,fp) != NULL)
	 {
		 len = strlen(buf);
		 buf[len-1] = '\0';

		 if(strstr(buf,z_IP_Type))
		 {
		    strncpy(gateway_IP,buf+strlen(z_IP_Type),strlen(buf)-strlen(z_IP_Type));
		 }
		 else if(strstr(buf,z_SN_Type))
		 {
		     strncpy(gateway_SN,buf+strlen(z_SN_Type),strlen(buf)-strlen(z_SN_Type));
		 }
		 else
		     break;
	 }
	 return 0;
}

void waitMessage(int socket_descriptor)
{
    int sin_len;
    char message[256];
    int iWaitFlag=0;

    struct sockaddr_in sin;

    printf("Waiting for data from sender \n");

    bzero(&sin,sizeof(sin));
    sin.sin_family=AF_INET;
    sin.sin_addr.s_addr=htonl(INADDR_ANY);
    sin_len=sizeof(sin);

    bind(socket_descriptor,(struct sockaddr *)&sin,sizeof(sin));

    int iFind=0;

    while(iWaitFlag<MaxWaitCount)
    {
        recvfrom(socket_descriptor,message,sizeof(message),0,(struct sockaddr *)&sin,&sin_len);

        if(message!=NULL)
        {
        	if(strstr(message,"IP:"))
        	{
        		printf("Found IP gateway:\n%s\n",message);
        		write_gateway_config_file(message);
        		iFind=1;
        		break;
        	}
        }
        sleep(1);
        iWaitFlag++;
    }

    if(iFind==1)
    {
    	//load gateway config
    	gateway_conf_init();
    	printf("IP:%s\n",gateway_IP);
    	printf("SN:%s\n",gateway_SN);
    	int ttt;
    	ttt=strtoul(gateway_SN, NULL, 16);
    	printf("SN=0X%x\n", ttt);
    }

    close(socket_descriptor);
}


int beginSearch()
{
	int sock=-1;
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		return -1;
	}
	const int opt = 1;
	int nb = 0;
	nb = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
	if(nb == -1)
	{
		return -1;
	}

	struct sockaddr_in addrto;
	bzero(&addrto, sizeof(struct sockaddr_in));
	addrto.sin_family=AF_INET;
	//addrto.sin_addr.s_addr=htonl(INADDR_BROADCAST);
	addrto.sin_addr.s_addr=htonl( 0xc0a801ff );//192.168.1.255

	addrto.sin_port=htons(PORT);
	int nlen=sizeof(addrto);

	int ret=sendto(sock, SEARCH_GATEWAY_COMMAND, strlen(SEARCH_GATEWAY_COMMAND), 0, (struct sockaddr *)&addrto, nlen);

	if(ret<0)
	{
		printf("send error ret=%d\n",ret);
		char *mesg=strerror(errno);
		printf("Mesg error:%s\n",mesg);
		return -1;
	}
	else
	{
		printf("send ok\n");
	}

	waitMessage(sock);

	return 0;
}


