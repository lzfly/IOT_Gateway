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

#include "enn_device_type.h"
#include "enn_device_attr.h"

#define N 128

#define MAXINTERFACES   16
char g_localMAC[16];
struct sockaddr_in g_localAddr;
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

int  sendMsgToWeb(char *ieeestr,unsigned short int attr,long double data)
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
	sprintf(deviceidstr, "wifi_gas_%s", ieeestr);
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

int main(int argc,char *argv[])
{
    char *ieeestr="A01511504001758";
    int sockfd,connectfd;
    int i,j,p,q,k;
    int m=0,n=0,t=0;
    long double d;
    struct sockaddr_in server_addr,client_addr;
    char buf[N],buf_f[N],buf_ff[N];
   // char buff[N]={0x3C,0X00,0X01,0X00,0XDE,0XAA,0X03,0X18,0X22,0X00,0X08,0X03,0XEA,0XAE,0X01,0X6A};
    char buf_wake[N]={0X3C,0X00,0X01,0X00,0X00,0X00,0X00,0X00,0XF9,0X00,0X08,0X08,0X01,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X85,0X4F};
    char buf_read[N]={0x3C,0X00,0X01,0X00,0XDE,0XAA,0X03,0X18,0X22,0X00,0X08,0X03,0XEA,0XAE,0X01,0X6A};
    char buf_open[N]={0x3C,0xFF,0x01,0x00,0xDE,0xAA,0x03,0x18,0x31,0x00,0x08,0x06,0x0E,0x00,0x02,0x00,0x00,0x00,0xF2};
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

        if(  (connectfd = accept(sockfd,(struct sockaddr*)&client_addr,&addrlen)) < 0)
        {
            perror("accept");
        }
        printf("connectfd = %d\n",connectfd);
        printf("client IP and Port %s:%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
	while(1)
	
	{      
		sleep(2);
		printf("AAAAAAA");
		if(0==m)
		{
			j=send(connectfd,buf_wake,22,0);
			printf("BBBBBBB");
			if(j<0)
			break;
			m=1;
		 	i=recv(connectfd,buf,18,0);
			if(i<=0)
			break;
			dump_hex( buf, i );
			sleep(6);
    		}
			if(20==t)
			{
				printf("11111111111111111111111111111");
				send(connectfd,buf_open,19,0);
				recv(connectfd,buf_ff,N,0);
			}
			
			p=send(connectfd,buf_read,16,0);
			printf("CCCCCCCC");
			if(p<0)
			break;
		 	q=recv(connectfd,buf_f,N,0);
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
			if(q<=0)
			break;
			d=(float)k/10;
			sendMsgToWeb(ieeestr,ENN_DEVICE_ATTR_GASMETER_VALUE,d);
			dump_hex( buf_f, q );
     			sleep(60);
        		t++;
	}
	

       // close(connectfd);
    }
    close(sockfd);

    return 0;
}
