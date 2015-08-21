#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <net/if.h>
#include <net/if_arp.h> 
#include <sys/ioctl.h> 

#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>

#include <pthread.h> 

#include "enn_device_type.h"
#include "enn_device_attr.h"

#include <uci.h>
#include<syslog.h>

#define N 128

#define MAXINTERFACES   16
char g_localMAC[16];
struct sockaddr_in g_localAddr;

//char *ieeestr="A01511504001758";
static char gas_meter_id[32];
static struct uci_context * uci_ctx;
static struct uci_package * uci_jianyoucfg;

static struct uci_context * uci_ctx_s;
static struct uci_package * uci_ennconfig;

static char time_s[8];
 int times = 0;

uint8_t sub(char *buff_sub,int len_sub)
{
	int i_sub = 1;
	int tmp = 0;
	uint8_t ret_sub; 
	for(i_sub = 1;i_sub < len_sub;i_sub ++)
	{
		tmp = tmp + buff_sub[i_sub];
	}
	printf("tmp = %hhx\n",tmp);
	ret_sub = (uint8_t )(tmp & 0xff);
	return ret_sub;
}
	
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


void  dump_hex( const unsigned char * ptr, size_t  len )
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

int  sendMsgToWeb(char *meterid,unsigned short int attr,long double data)
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

    /**/
	if ( ubus_lookup_id(ctx, "jianyou", &id) ) {
		fprintf(stderr, "Failed to look up jianyou object\n");
		return;
	}

	blob_buf_init( &b, 0 );
	char gatewayidstr[32];
	sprintf(gatewayidstr, "we26n_%s", g_localMAC);
	blobmsg_add_string( &b, "gatewayid", gatewayidstr );

        printf("[sendMsgToWeb] start--3\r\n");
	
	char deviceidstr[64];
	sprintf(deviceidstr, "wifi_gas_%s", meterid);
        printf("[sendMsgToWeb] start--%s\r\n", deviceidstr);
	blobmsg_add_string( &b, "deviceid", deviceidstr);
	
	char devicetypestr[8];
	char deviceattrstr[16];
	char devicedatastr[64];
	
	
	sprintf(devicetypestr, "%s", ENN_DEVICE_TYPE_GAS_METER);
	sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_GASMETER_VALUE);
	 printf("1111111111\n");
	sprintf(devicedatastr, "%lf", data);
	
	blobmsg_add_string( &b, "devicetype", devicetypestr);
	
	blobmsg_add_string( &b, "attr", deviceattrstr );
	

	
	blobmsg_add_string( &b, "data", devicedatastr);	

	printf("[sendMsgToWeb]ubus_invoke data = %s\r\n", devicedatastr);
	/**/
	ubus_invoke( ctx, id, "report", b.head, test_data_cback, 0, 3000);

	
    /**/
	ubus_free(ctx);
	return 0;
	
}


void* gas_meter_thread( void *arg )  
{  
    extern int errno;
    char devicesstr[2048];
    devicesstr[0] = 0;
    char *path="/tmp/devices_6.ini";
    FILE *fp;
    char buff_path[N];
    int i,j,p,q,k,l;
    int m=0,n=0,t=0,f=0;
    long double d;
    int connectfd;
    uint8_t crc,crc_o;	

    char buf[N],buf_f[N],buf_ff[N];
    
    
    
  	//读取配置文件数据上报时间	
	if (!uci_ctx_s)
    {
        uci_ctx_s = uci_alloc_context();
    }
    else
    {
        uci_ennconfig = uci_lookup_package(uci_ctx_s, "ennconfig");
        if (uci_ennconfig)
            uci_unload(uci_ctx_s, uci_ennconfig);
    }

    if (uci_load(uci_ctx_s, "ennconfig", &uci_ennconfig))
    {
        printf("uci load ENN config fail\n");
    }else
	{
	    char *value_s = NULL;
            struct uci_element *e_s   = NULL;
            printf("uci load ENN config success\n");


            /* scan enn config ! */
            uci_foreach_element(&uci_ennconfig->sections, e_s)
            {
                struct uci_section *s_s = uci_to_section(e_s);
                if(0 == strcmp(s_s->type, "wifi"))
                {
                    printf("%s(), type: %s\n", __FUNCTION__, s_s->type);

                    value_s = uci_lookup_option_string(uci_ctx_s, s_s, "gas_interval");
                    if(value_s){
                            strcpy(time_s, value_s);
                            printf("%s(), sleep time: %s\n", __FUNCTION__, time_s);
                             times=strtoul(time_s,NULL,10);
                             printf("%d\n",times);
                        }else{
                            printf("%s(), sleep time_id not found\n", __FUNCTION__);
                     }
                     break;
                 }

             }
            
             
    }

    
    
    
    	char *id;
     	char str[32];
        //id = gas_meter_id;
        //printf("ga_sid = %s\n",id);
        //id = id +3;
        printf("ddd=%s\n",&gas_meter_id[3]);
        uint64_t d_ata;
        if((d_ata = strtoull(&gas_meter_id[3], NULL, 10))<0)
        {
        	printf("errno=%d\n",errno);
        }
	printf("d_ata = %llu \n",d_ata);
	//printf("%hhx-%hhx-%hhx-%hhx\r\n", (uint8_t)(d_ata & 0xff), (uint8_t)((d_ata >> 8) & 0xff), (uint8_t)((d_ata >> 16) & 0xff), (uint8_t)((d_ata >> 24) & 0xff) );
	 
    
   // char buff[N]={0x3C,0X00,0X01,0X00,0XDE,0XAA,0X03,0X18,0X22,0X00,0X08,0X03,0XEA,0XAE,0X01,0X6A};
    char buf_wake[N]={0X3C,0X00,0X01,0X00,0X00,0X00,0X00,0X00,0XF9,0X00,0X08,0X08,0X01,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X85,0X4F};
    char buf_read[N]={0x3C,0X00,0X01,0X00,0XDE,0XAA,0X03,0X18,0X22,0X00,0X08,0X03,0XEA,0XAE,0X01,0X6A};
    buf_read[4] = (uint8_t)(d_ata & 0xff);
    buf_read[5] = (uint8_t)(d_ata >>8 & 0xff);
    buf_read[6] = (uint8_t)(d_ata >>16 & 0xff);
    buf_read[7] = (uint8_t)(d_ata >>24 & 0xff);
    crc = sub(buf_read,15);
    printf("CRC = %x\n",crc);
    printf("%hhx-%hhx-%hhx-%hhx\r\n",buf_read[4],buf_read[5],buf_read[6],buf_read[7]);
    buf_read[15] = crc;
    char buf_open[N]={0x3C,0xFF,0x01,0x00,0xDE,0xAA,0x03,0x18,0x31,0x00,0x08,0x06,0x0E,0x00,0x02,0x00,0x00,0x00,0xF2};
    buf_open[4] = buf_read[4];
    buf_open[5] = buf_read[5];
    buf_open[6] = buf_read[6];
    buf_open[7] = buf_read[7];
    printf("%hhx-%hhx-%hhx-%hhx\r\n",buf_open[4],buf_open[5],buf_open[6],buf_open[7]);
    buf_open[18] = 0; 
    crc_o = sub(buf_open,18);
    printf("buf[18]= %x\n",crc_o);
    char buf_sleep[21]={0X3C,0X00,0X01,0X00,0X00,0X00,0X00,0X00,0XFD,0X00,0X08,0X08,0X00,0X00,0X00,0X00,0X05,0X00,0X00,0X00,0X13};
    printf( "gas_meter_thread init\n");  
 
	connectfd = arg;
	struct timeval timeout = {3,0};  
        setsockopt(connectfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(struct timeval));
	
	while(1)
	{      
			sleep(2);
			printf("\n******gas_meter awake ********\n");
			
			j=send(connectfd,buf_wake,22,0);
			if(j<0)
			continue;
			syslog(LOG_CRIT,"[gas_meter]gas meter awake send ok");
		 	i=recv(connectfd,buf,18,0);
			if(i<=0)
			continue;
			syslog(LOG_CRIT,"[gas_meter]gas meter awake recv ok");
			dump_hex( buf, i );
			sleep(6);
    			
			printf("\n******gas_meter send cmd********\n");
			p=send(connectfd,buf_read,16,0);
			if(p<0)
			continue;
			syslog(LOG_CRIT,"[gas_meter]gas meter read meter send ok");
			dump_hex( buf_read, 16 );
			//sleep(2);
			int len_g=0,count_g=0;
			//char buff2[41];
			printf("\n******gas_meter read  data before********\n");
	    		while(count_g < 5 && len_g < 41)
	    		{
				q=recv(connectfd,buf_f,41,0);
				printf("q=%d\n", q);
				count_g++;
				if(q<0)
				    continue;
				//memcpy(buf_f+len_g, buff2, q);
				len_g = len_g + q;
				
				//sleep(1);
			
			}
			syslog(LOG_CRIT,"[gas_meter]gas meter read meter recv ok");
			printf("count_g=%d, len_g=%d\n", count_g,len_g);
			dump_hex( buf_f, len_g );
			printf("\n******gas_meter read  data after********\n");
			if(len_g != 41 && len_g != 59){
			    printf("read not 41 and 59\n");
			    continue;
			    }
			
			
			/*k = buf_f[18];
			k = (k * 256) + buf_f[17];
			k = (k * 256) + buf_f[16];
			k = (k * 256) + buf_f[15];
			*/
			k = buf_f[18];
			k = buf_f[17] | (k << 8);
			k = buf_f[16] | (k << 8);
			k = buf_f[15] | (k << 8);
			
			printf("kx=%2x\nkd=%d\n",k,k);
			/*
			k = *(int *)&(buf_f[15]);
			printf("kx=%2x\nkd=%d\n",k,k);*/
			d=(float)k/10;
			if(30 == f)
			{
				sprintf(&devicesstr[0], "[");
				sprintf(&devicesstr[strlen(devicesstr)], "{");
				sprintf(&devicesstr[strlen(devicesstr)], "\"deviceid\":\"gas_meter_%s\",",gas_meter_id);
				sprintf(&devicesstr[strlen(devicesstr)], "\"status\":\"5\",");
				sprintf(&devicesstr[strlen(devicesstr)], "\"devicetype\":\"ENN_DEVICE_TYPE_GAS_METER\",");
				sprintf(&devicesstr[strlen(devicesstr)], "\"data\":\"%f\"",d);	
				sprintf(&devicesstr[strlen(devicesstr)], "},");
				sprintf(&devicesstr[strlen(devicesstr)-1], "]");
				printf("devicesstr= %s\n",devicesstr);
				if((fp=fopen(path,"w+")) == NULL)
				{
					printf("fail to open\n");
					return -1;
				}
			
				fwrite(devicesstr,1,strlen(devicesstr),fp);
				fclose(fp);
				f=0;
			}
			sendMsgToWeb(gas_meter_id,ENN_DEVICE_ATTR_GASMETER_VALUE,d);
			//dump_hex( buf_f, q );
			
			j=send(connectfd,buf_sleep,21,0);
			if(j<0)
			continue;
			syslog(LOG_CRIT,"[gas_meter]gas meter sleep send ok");
			printf("\n******gas_meter sleep ********\n");
     			sleep(times*60);
     			printf("\nsleep=%d\n",times);
        		t++;
        		f++;
	}
	close(connectfd);
	
} 


int main(int argc,char *argv[])
{

    int sockfd,connectfd;
	pthread_t th;  
	int ret;  
	int *thread_ret = NULL;  
		
    struct sockaddr_in server_addr,client_addr;


    strcpy(gas_meter_id, "A01511504001758");
	
    if (!uci_ctx)
    {
        uci_ctx = uci_alloc_context();
    }
    else
    {
        uci_jianyoucfg = uci_lookup_package(uci_ctx, "jyconfig");
        if (uci_jianyoucfg)
            uci_unload(uci_ctx, uci_jianyoucfg);
    }

    if (uci_load(uci_ctx, "jyconfig", &uci_jianyoucfg))
    {
        printf("uci load jianyou config fail\n");
    }else
	{
	    char *value = NULL;
            struct uci_element *e   = NULL;
            printf("uci load jianyou config success\n");


            /* scan jianyou config ! */
            uci_foreach_element(&uci_jianyoucfg->sections, e)
            {
                struct uci_section *s = uci_to_section(e);
                if(0 == strcmp(s->type, "deviceid"))
                {
                    printf("%s(), type: %s\n", __FUNCTION__, s->type);

                    value = uci_lookup_option_string(uci_ctx, s, "gasmeter");
                    if(value){
                            strcpy(gas_meter_id, value);
                            printf("%s(), gas_meter_id: %s\n", __FUNCTION__, gas_meter_id);
                        }else{
                            printf("%s(), gas_meter_id not found\n", __FUNCTION__);
                     }
                     break;
                 }

             }
             
    }


	
	
   getLocalIPandMAC ();
   if( (sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("socket");
    }
    printf("sockfd =%d\n",sockfd);
    memset(&server_addr,0,sizeof(server_addr));
    memset(&client_addr,0,sizeof(client_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9000);
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    if(bind (sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0)
    {
        perror("bind");
    }
    listen(sockfd,5);

    socklen_t addrlen = sizeof(client_addr);
    while(1)
    {
        if((connectfd = accept(sockfd,(struct sockaddr*)&client_addr,&addrlen)) < 0)
        {
            perror("accept error");
        }
        printf("new socket connectfd = %d\n",connectfd);
        printf("client IP and Port %s:%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
		
		ret = pthread_create( &th, NULL, gas_meter_thread, connectfd );  
		if( ret != 0 ){  
			printf( "Create thread error!\n");  
			continue;  
		}  
		printf( "This is the main process.\n" );  
		
    }
    close(sockfd);

    return 0;
}
