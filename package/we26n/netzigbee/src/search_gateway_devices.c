
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
#include "search_gateway_devices.h"
#include "gateway.h"
#include "fbee_protocol.h"
#include "we26n_type.h"
#include "gateway_socket.h"
#include "ctrl_gateway_devices.h"

extern int startSearchDevice();

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

int waitMessage(int socket_descriptor)
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

    	strcpy(g_Gate.IP,gateway_IP);
    	strcpy(g_Gate.SN,gateway_SN);
    	g_Gate.snid=strtoul(gateway_SN, NULL, 16);

    	printf("IP:%s\n",g_Gate.IP);
		printf("SN:%s\n",g_Gate.SN);
		printf("SN=0X%x\n", g_Gate.snid);
		
		return 0;
    }
	else{
	    return 2;
	}
}




int beginSearch()
{
	int sock=-1;
	sleep(10);


    getLocalIPandMAC();

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
	//addrto.sin_addr.s_addr=htonl( 0xc0a800ff );//192.168.1.255
	addrto.sin_addr.s_addr = g_localAddr.sin_addr.s_addr | 0xFF000000;
	

	addrto.sin_port=htons(PORT);
	int nlen=sizeof(addrto);

research:	
	printf("[beginSearch]begin to search gateway\n"); 
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
    
	int iret;
	iret = waitMessage(sock);
	if(iret == 0)
	{ 
	    printf("[beginSearch]find gateway\n");
	    close(sock);
		connectGateway();
		startGatewayService();
		sleep(2);
		getGateDetailInfo(); 
	    sleep(5);
	    startSearchDevice();
	}
	else
	{
	    printf("[beginSearch] no find gateway\n");
	    sleep(40);
		goto research;
	}
	
	return 0;
}

//get gate info
int getGateDetailInfo()
{
	int cmd_length=1;
	w26n_byte msg[cmd_length];

	msg[SRPC_CMD_ID_POS]=RPCS_GET_GATEDETAIL;

	sendCommand(g_monitor_socket,msg,cmd_length);

	return 0;
}

static int sendFailCount = 0;

#define SEARCHDEVICEMAX  5
#define GETDEVICESTATEMAX 20
#define ENTRYNETMAX 6
static int searchDeviceCount = 0;
static int getDeviceStateCount = 0;
static int entryNetCount = 0;
//static int alertint = 0;
int startSearchDevice()
{

	int cmd_length=1;
	w26n_byte msg[cmd_length];

	int index = 0;
	int iret;

	msg[SRPC_CMD_ID_POS]=RPCS_GET_DEVICES;
    printf("[startSearchDevice]\r\n");

	while(1)
	{
		printf("[startSearchDevice] g_devices_count=%d\r\n", g_devices_count);
                if(searchDeviceCount >= SEARCHDEVICEMAX){
		    iret = sendCommand(g_monitor_socket,msg,cmd_length);
		    if(iret)
		    {  
			sendFailCount++;
			if(sendFailCount > 3)
			{
			    printf("[startSearchDevice]send command fail 3 times\n"); 
				printf("[startSearchDevice]restart Gateway Service\n");
			    disConnectGateway();
				endGatewayService();
				sleep(2);
				connectGateway();
				restartGatewayService();
				sendFailCount = 0;
				
			}
		    } 
		    searchDeviceCount = 0;
		    sleep(20);
               }
               searchDeviceCount++;

            if(getDeviceStateCount >= GETDEVICESTATEMAX){		
                int i;
		index++;
		for(i = 0; i < g_devices_count; i++)
		{
		    //printf("getDeviceState");
			printf("[startSearchDevice] endpoint=%d\r\n", g_devices[i].endpoint);
			printf("device SN = %s\n", g_devices[i].SN);
		    printf("device shortaddr = %d\n", g_devices[i].shortaddr);
                    
                    printf("device ieee = %s\n", g_devices[i].ieeestr);
                    printf("openstatus=%d", g_openStatus[i]);
                    if(g_openStatus[i] == 5)
			    continue;
                    printf("getDeviceState------1\n");

			switch(g_devices[i].deviceId)
			{
				case FB_DEVICE_TYPE_COLOR_TEMP_LAMP:
				case FB_DEVICE_TYPE_COLOR_TEMP_LAMP_2:
                    getDeviceState(0x2, g_devices[i].shortaddr, g_devices[i].endpoint);
					getDeviceLevel(0x2, g_devices[i].shortaddr, g_devices[i].endpoint);
					getDeviceColorTemp(0x2, g_devices[i].shortaddr, g_devices[i].endpoint);
					break;
				case FB_DEVICE_TYPE_LEVEL_CONTROL_SWITCH:
				case FB_DEVICE_TYPE_WINDOWS:
					getDeviceState(0x2, g_devices[i].shortaddr, g_devices[i].endpoint);
					break;
				case FB_DEVICE_TYPE_TEMP_HUM:
				case FB_DEVICE_TYPE_TEMP_HUM_2:
					//if(alertint == 0)
				    //    sendTmpHumAlertInterval(0x2, g_devices[i].shortaddr, g_devices[i].endpoint, 0x1aaa);
					//alertint = 1;
					break;
				default:
					break;
			}
			
			sleep(20);
                
		    }
                    getDeviceStateCount = 0;
                }
                getDeviceStateCount++;

            if(entryNetCount >= ENTRYNETMAX){
                printf("can enrty zigbee net\n");

                sendEntryNet();         
                entryNetCount = 0; 
            }
            entryNetCount++;     

            sleep(10);
	}

}




