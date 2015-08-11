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
#include "type.h"

#include <stdint.h>

#define N 128

int32_t  modbus_conv_longm( uint8_t * puc )
{
    int32_t  temp;

    /**/
    temp = puc[2];
    temp = temp << 8;
    temp = temp | puc[3];
    temp = temp << 8;
    temp = temp | puc[0];
    temp = temp << 8;
    temp = temp | puc[1];

    /**/
    return temp;
}


float  modbus_conv_real4( uint8_t * puc )
{
    volatile float  temp;
    uint32_t * pu32;

    /**/
    pu32 = (uint32_t *)&temp;

    /**/
    *pu32 = puc[2];
    *pu32 <<= 8;
    *pu32 |= puc[3];
    *pu32 <<= 8;
    *pu32 |= puc[0];
    *pu32 <<= 8;
    *pu32 |= puc[1];

    /**/
    return temp;
}

int32_t  modbus_conv_long( uint8_t * puc )
{
    int32_t  temp;

    /**/
    temp = puc[0];
    temp = temp << 8;
    temp = temp | puc[1];
    temp = temp << 8;
    temp = temp | puc[2];
    temp = temp << 8;
    temp = temp | puc[3];

    /**/
    return temp;
}

#define MAXINTERFACES   16
char g_localMAC[16];
struct sockaddr_in g_localAddr;

char *powerid="1507080600179";
char *waterid="TS0004482";
char *heatid="50001482";
	
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

int  sendMsgToWeb(char *ieeestr,unsigned short int type,double data)
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
	char gatewayidstr[32];
	sprintf(gatewayidstr, "we26n_%s", g_localMAC);
	blobmsg_add_string( &b, "gatewayid", gatewayidstr );

        printf("[sendMsgToWeb] start--3\r\n");
		
	char deviceidstr[64];
	sprintf(deviceidstr, "wifi_meters_%s", ieeestr);
        printf("[sendMsgToWeb] start--%s\r\n", deviceidstr);
	blobmsg_add_string( &b, "deviceid", deviceidstr);
	
	char devicetypestr[8];
	char deviceattrstr[16];
	char devicedatastr[64];
	
	switch(type)
	{
	case POWER:
		sprintf(devicetypestr, "%s", ENN_DEVICE_TYPE_POWERMETER);
		sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_POWERMETER_VALUE);
		sprintf(devicedatastr, "%lf", data);
		break;	
	case WATER:
		sprintf(devicetypestr, "%s", ENN_DEVICE_TYPE_WATERMETER);
		sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_WATERMETER_VALUE);
		sprintf(devicedatastr, "%lf", data);
		break;	
	case HEAT:
		sprintf(devicetypestr, "%s", ENN_DEVICE_TYPE_HEAT_METER);
		sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_HEATMETER_VALUE);
		sprintf(devicedatastr, "%lf", data);
		break;	
	default:
		sprintf(devicedatastr, "%lf", data);
		break;	
	}
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


void* enn_meter_thread( void *arg )  
{  
	int i,j,p,q,k;
	int m=0,n=0,t=0;
	float bbb;
	int32_t aaa,aaa_w;
	double ddd,ccc;
	uint8_t * puc_power,* puc_water,*puc_heat;
	double d,aaa_h;
	int connectfd;
	
    	char buf_power[N],buf_water[N],buf_heat[N];
     	char buff_power[N] = {0x01,0X03,0X00,0X14,0X00,0X02,0X84,0X0F};
	 char buff_water[N]={0x02,0X03,0X00,0X90,0X00,0X04,0X44,0X17};
	 char buff_heat[N]={0x02,0X03,0X00,0X78,0X00,0X02,0X44,0X21};

	
	
    printf( "enn_meter_thread init\n");  
 
	connectfd = arg;

	
	while(1)
	{      
		//电表
		sleep(2);
		j=send(connectfd,buff_power,8,0);
		if(j<0)
	        break;
		int len_p=0,count_p=0;
	    	while(count_p < 5 && len_p < 9)
	    	{
			i=recv(connectfd,buf_power,9,0);
			if(i<=0)
			    break;
			len_p = len_p + i;
			count_p++;
			
		}
		printf("count_p=%d\n",count_p);
		dump_hex(buf_power,len_p);
		puc_power = (uint8_t *)buf_power;
		aaa = modbus_conv_long(&(puc_power[3]));
		ddd = aaa*250*60;
		ddd = ddd/18000000;
		printf("The power meter reading is :%f\n",ddd);
		sendMsgToWeb(powerid,POWER,ddd);
		sleep(10);
		printf("sleep ok\n");
		//水表
		p=send(connectfd,buff_water,8,0);
		if(p<0)
	        break;
	        
		int len_w=0,count_w=0;
	    	while(count_w < 5 && len_w < 13)
	    	{
			q=recv(connectfd,buf_water,13,0);
			if(q<=0)
			    break;
			len_w = len_w + q;
			count_w++;
		}
		printf("count_w=%d\n",count_w);
		dump_hex(buf_water,len_w);
		puc_water = (uint8_t *)buf_water;
		aaa_w = modbus_conv_longm(&(puc_water[3]));
		bbb = modbus_conv_real4(&(puc_water[7]));
		ccc = bbb;
		ccc = ccc + aaa_w; 
		printf("The water meter reading is :%f\n",ccc);
		sendMsgToWeb(waterid,WATER,ccc);
		sleep(10);
		
		//热表
		m=send(connectfd,buff_heat,8,0);
		if(m<0)
	        break;
	        
		int len_h=0,count_h=0;
	    	while(count_h < 5 && len_h < 9)
	    	{
			n=recv(connectfd,buf_heat,9,0);
			if(n<=0)
			    break;
			len_h = len_h + n;
			count_h++;
		}
		dump_hex(buf_heat,len_h);
		puc_heat = (uint8_t *)buf_heat;
		aaa_h = modbus_conv_real4(&(puc_heat[3]));
		
		printf("The heat meter reading is :%f\n",aaa_h);
		sendMsgToWeb(heatid,HEAT,aaa_h);
		sleep(10);
		
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

   getLocalIPandMAC ();
   if( (sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("socket");
    }
    printf("sockfd =%d\n",sockfd);
    memset(&server_addr,0,sizeof(server_addr));
    memset(&client_addr,0,sizeof(client_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9001);
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
		
		ret = pthread_create( &th, NULL, enn_meter_thread, connectfd );  
		if( ret != 0 ){  
			printf( "Create thread error!\n");  
			continue;  
		}  
		printf( "This is the main process.\n" );  
		
    }
    close(sockfd);

    return 0;
}
