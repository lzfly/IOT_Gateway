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

#include<syslog.h>

#include <uci.h>
#include <stdint.h>
#include <math.h>

#define N 128

static char time_p[8],time_w[8],time_h[8];
int timep=0,timew=0,timeh=0; 


static char powerid[32],waterid[32],heatid[32];
static struct uci_context * uci_ctx;
static struct uci_package * uci_jianyoucfg;

static struct uci_context * uci_ctx_s;
static struct uci_package * uci_ennconfig;

/*
double pow(double num,int n)
{
    double powint = 1;
    int i;
    for(i = 1;i <= n;i++)
    powint*=num;
    return powint;
}*/

int write_to_ini(char *powerid,char *waterid,char *heatid,double ddd,double ccc,double ccc_h)
{
    char *path="/tmp/devices_7.ini";
    FILE *fp;
    char devicesstr[2048];
    devicesstr[0] = 0;
    
    sprintf(&devicesstr[0], "[");
	sprintf(&devicesstr[strlen(devicesstr)], "{");
	sprintf(&devicesstr[strlen(devicesstr)], "\"deviceid\":\"wifi_meter_%s\",",powerid);
	sprintf(&devicesstr[strlen(devicesstr)], "\"status\":\"5\",");
	sprintf(&devicesstr[strlen(devicesstr)], "\"devicetype\":\"0007\",");
	sprintf(&devicesstr[strlen(devicesstr)], "\"data\":\"%f\"",ddd);	
	sprintf(&devicesstr[strlen(devicesstr)], "},");
	sprintf(&devicesstr[strlen(devicesstr)], "{");
	sprintf(&devicesstr[strlen(devicesstr)], "\"deviceid\":\"wifi_meter_%s\",",waterid);
	sprintf(&devicesstr[strlen(devicesstr)], "\"status\":\"5\",");
	sprintf(&devicesstr[strlen(devicesstr)], "\"devicetype\":\"0008\",");
	sprintf(&devicesstr[strlen(devicesstr)], "\"data\":\"%f\"",ccc);	
	sprintf(&devicesstr[strlen(devicesstr)], "},");
	sprintf(&devicesstr[strlen(devicesstr)], "{");
	sprintf(&devicesstr[strlen(devicesstr)], "\"deviceid\":\"wifi_meter_%s\",",heatid);
	sprintf(&devicesstr[strlen(devicesstr)], "\"status\":\"5\",");
	sprintf(&devicesstr[strlen(devicesstr)], "\"devicetype\":\"0021\",");
	sprintf(&devicesstr[strlen(devicesstr)], "\"data\":\"%f\"",ccc_h);	
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
	return 0;
		
}


int get_Deviceid_config(char *jyconfig)
{
      int rtn = -1;

    if (!uci_ctx)
    {
        uci_ctx = uci_alloc_context();
    }
    else
    {
        uci_jianyoucfg = uci_lookup_package(uci_ctx, jyconfig);
        if (uci_jianyoucfg)
            uci_unload(uci_ctx, uci_jianyoucfg);
    }

    if (uci_load(uci_ctx, jyconfig, &uci_jianyoucfg))
    {
        printf("uci load jianyou config fail\n");
    }else
	{
	    char *value_p = NULL;
	    char *value_w = NULL;
	    char *value_h = NULL;
            struct uci_element *e   = NULL;
            printf("uci load jianyou config success\n");


            /* scan jianyou config ! */
            uci_foreach_element(&uci_jianyoucfg->sections, e)
            {
                struct uci_section *s = uci_to_section(e);
                if(0 == strcmp(s->type, "deviceid"))
                {
                    printf("%s(), type: %s\n", __FUNCTION__, s->type);

                    value_p = uci_lookup_option_string(uci_ctx, s, "powermeter");
                   
                    if(value_p)
                    {
                            strcpy(powerid, value_p);
                            printf("%s(), powerid: %s\n", __FUNCTION__, powerid);
                            rtn = 0;
                     }
                     else{
                            printf("%s(), gas_meter_id not found\n", __FUNCTION__);
                     }
                      value_w = uci_lookup_option_string(uci_ctx, s, "watermeter"); 
                   
                      if(value_w)
                    {
                            strcpy(waterid, value_w);
                            printf("%s(), waterid: %s\n", __FUNCTION__, waterid);
                            rtn = 0;
                     }
                     else{
                            printf("%s(), water_meter_id not found\n", __FUNCTION__);
                     }
                      value_h = uci_lookup_option_string(uci_ctx, s, "heatmeter"); 
                       if(value_h)
                    {
                            strcpy(heatid, value_h);
                            printf("%s(), heatid: %s\n", __FUNCTION__, heatid);
                            rtn = 0;
                     }
                     else{
                            printf("%s(), heat_meter_id not found\n", __FUNCTION__);
                  }
                     break;
            }

          }
             
   }
   return rtn;
}


int get_SleepTime_config(char *ennconfig)
{
    int rtn = -1;

    if (!uci_ctx_s)
    {
        uci_ctx_s = uci_alloc_context();
    }
    else
    {
        uci_ennconfig = uci_lookup_package(uci_ctx_s, ennconfig);
        if (uci_ennconfig)
            uci_unload(uci_ctx_s, uci_ennconfig);
    }

    if (uci_load(uci_ctx_s, ennconfig, &uci_ennconfig))
    {
        printf("uci load ENN config fail\n");
    }else
	{
	    char *value_sp = NULL;
	    char *value_sw = NULL;
	    char *value_sh = NULL;
            struct uci_element *e_s   = NULL;
            printf("uci load ENN config success\n");


            /* scan ENN config ! */
            uci_foreach_element(&uci_ennconfig->sections, e_s)
            {
                struct uci_section *s_s = uci_to_section(e_s);
                if(0 == strcmp(s_s->type, "470m"))
                {
                    printf("%s(), type: %s\n", __FUNCTION__, s_s->type);

                    value_sp = uci_lookup_option_string(uci_ctx_s, s_s, "power_interval");
                   
                    if(value_sp)
                    {
                            strcpy(time_p, value_sp);
                            printf("%s(), power meter sleep: %s\n", __FUNCTION__, time_p);
                            timep=strtoul(time_p,NULL,10);
                            printf("%d\n",timep);
                            rtn = 0;
                     }
                     else{
                            printf("%s(), sleep time not found\n", __FUNCTION__);
                     }
                      value_sw = uci_lookup_option_string(uci_ctx_s, s_s, "water_interval"); 
                   
                      if(value_sw)
                    {
                            strcpy(time_w, value_sw);
                            printf("%s(), water sleep time: %s\n", __FUNCTION__, time_w);
                            timew=strtoul(time_w,NULL,10);
                            printf("%d\n",timew);
                            rtn = 0;
                     }
                     else{
                            printf("%s(), water meter sleep time not found\n", __FUNCTION__);
                     }
                      value_sh = uci_lookup_option_string(uci_ctx_s, s_s, "heat_interval"); 
                       if(value_sh)
                    {
                            strcpy(time_h, value_sh);
                            printf("%s(), heat sleep time: %s\n", __FUNCTION__, time_h);
                            timeh=strtoul(time_h,NULL,10);
                            printf("%d\n",timeh);
                            rtn = 0;
                     }
                     else{
                            printf("%s(), heat meter sleep time not found\n", __FUNCTION__);
                  }
                     break;
            }

          }
             
   }

   return rtn;
}

int get_sleep_time()
{
    int get_SleepTime;
   get_SleepTime = get_SleepTime_config("ennconfig");
    if(get_SleepTime == 0)
    {
        return;
    }
    printf("read config ennconfig, get_SleepTime = %d\n ", get_SleepTime );
    printf("require config file ennconfig fail now require ennconfig_ever\n");
    
    get_SleepTime =  get_SleepTime_config("ennconfig_ever");
    if(get_SleepTime == 0)
    {
        return 0;
    } 
     printf("read config ennconfig_ever fail, get_SleepTime = %d\n ", get_SleepTime );    
	return -100;
}

int get_device_id()
{
  int get_meterid;
    get_meterid = get_Deviceid_config("jyconfig");
    if(get_meterid == 0)
    {
        return;
    }

     printf("read config jyconfig_ever fail, get_meterid = %d\n ", get_meterid );    
	return -100;
}

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
uint8_t auchCRCHi[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

uint8_t auchCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};
uint16_t  modbus_crc16( uint8_t * puchMsg, int tlen )
{
    uint8_t  uchCRCHi=0xFF;  /* žßCRC×ÖœÚ³õÊŒ»¯ */ 
    uint8_t  uchCRCLo=0xFF;  /* µÍCRC ×ÖœÚ³õÊŒ»¯ */ 
    uint32_t  uIndex;        /* CRCÑ­»·ÖÐµÄË÷Òý */ 
    
    while ( tlen--)
    {
        uIndex = uchCRCHi^*puchMsg++;
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
        uchCRCLo = auchCRCLo[uIndex];
    }

    /**/
    uIndex = uchCRCHi;
    uIndex = uIndex << 8;
    uIndex = uIndex | uchCRCLo;
    return (uint16_t)uIndex;
}

#define MAXINTERFACES   16
char g_localMAC[16];
struct sockaddr_in g_localAddr;

//char *powerid="1507080600179";
//char *waterid="TS0004482";
//char *heatid="50001482";
	
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


int get_By_multiplication_factor(int connectfd,char *buff_heat_take[N])
{
        char buf_by[N],buff_by[N];
        uint8_t *puc_by;
        int m = 0,m_b=0;
        uint16_t crcvalue_b,crcvalue_b1;
        int by_multiplication_factor;
        m=send(connectfd,buff_heat_take,8,0);
		if(m<0)
		{
	        return -2;
	     }
	        syslog(LOG_CRIT,"[heat_meter]heat meter by multiplication factor send ok");
	        printf("heat meter by multiplication factor send ok\n");
		int len_b=0,count_b=0;
	    	while(count_b < 5 && len_b < 7)
	    	{
			m_b=recv(connectfd,buf_by,7,0);
			count_b++;
			if(m_b<=0)
			    continue;
            memcpy(buff_by+len_b, buf_by, m_b);
			len_b = len_b + m_b;
			//printf("\nlen_h = %d,count_h = %d\n")
                        sleep(1);
			
		}
		if(len_b != 7)
		{
			syslog(LOG_CRIT,"[heat_meter]heat meter by multiplication factor read not 7");
			printf("heat meter by multiplication factor read not 7\n");
			return -3;
		}
		printf("heat meter by multiplication factor recv ok\n");
		syslog(LOG_CRIT,"[heat_meter]heat meter by multiplication factor recv ok");
		printf("\n*****heat meter*******\n");
		dump_hex(buff_by,len_b);


                crcvalue_b = modbus_crc16( buff_by, len_b - 2 );
                printf("crcvalue_b = %x\n", crcvalue_b);
                crcvalue_b1 = ((buff_by[len_b - 2]&0xff) << 8) | (buff_by[len_b - 1] & 0xff);
                printf("crcvalue_b1 = %x\n", crcvalue_b1);
                if(crcvalue_b == crcvalue_b1)
                {
		            puc_by = (uint8_t *)buff_by;
		            by_multiplication_factor=(puc_by[3] << 8) | (puc_by[4]);
		            printf("by multiplication factor = %d\n",by_multiplication_factor);
		            return by_multiplication_factor;	            
                }
				else
				{
				    syslog(LOG_CRIT,"[heat_meter] crc by multiplication factor error");
				    return -1;
				}

    
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
	sprintf(deviceidstr, "wifi_meter_%s", ieeestr);
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
	float bbb,bbb_h;
	int32_t aaa_w;
	double ddd,ccc,ccc_h;
	uint8_t * puc_power,* puc_water,*puc_heat;
	double aaa,d,aaa_h;
	int connectfd;
	uint16_t crcvalue, crcvalue1;
	
    	char buf_power[N],buf_water[N],buf_heat[N];
     	char buff_power[N] = {0x01,0X03,0X00,0X14,0X00,0X02,0X84,0X0F};
	 //char buff_water[N]={0x02,0X03,0X00,0X1C,0X00,0X04,0X85,0XFC};
	 //char buff_heat[N]={0x03,0X03,0X00,0X78,0X00,0X02,0X45,0XF0};
	char buff_water[N]={0x02,0X03,0X00,0X1E,0X00,0X04,0X24,0X3C};
	char buff_heat[N]={0x03,0X03,0X00,0X1C,0X00,0X04,0X84,0X2D};
	char buff_heat_take[N]={0x03,0x03,0x05,0x9F,0x00,0x01,0xB5,0x0A};
	
        printf( "enn_meter_thread 89789789 init\n");  
 	
	connectfd = arg;
	
	struct timeval timeout = {3,0};  
        setsockopt(connectfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(struct timeval));
	
	printf("connectfd = %d\n",connectfd);
	printf("out of while\n");
	write_to_ini(powerid,waterid,heatid,0.000000,0.000000,0.000000);
	while(1)
	{      
		printf("in while \n");
                do{
                    i = recv(connectfd,buf_power,18,0);
                }while(i > 0);
		//电表
		sleep(2);
		printf("sleep ok\n");
		j=send(connectfd,buff_power,8,0);
		if(j<0)
	            break;
	        syslog(LOG_CRIT,"[power_meter]power meter  send ok");
	        printf("power meter send ok\n");
		int len_p=0,count_p=0;
                char buff2[32];

	    	while(count_p < 5 && len_p < 9)
	    	{
			i=recv(connectfd,buff2,9,0);
			count_p++;
			if(i<=0)
			    continue;

                        memcpy(buf_power+len_p, buff2, i);
			len_p = len_p + i;
                        sleep(1);
			
			
		}
		if(len_p != 9)
		{
			syslog(LOG_CRIT,"[power_meter]power meter read not 9");
			printf("power meter read not 9\n");
			break;
		}
		  printf("power meter recv ok\n");
		  syslog(LOG_CRIT,"[power_meter]power meter  recv ok");
		printf("count_p=%d\n",count_p);
		dump_hex(buf_power,len_p);

                

                crcvalue = modbus_crc16( buf_power, len_p - 2 );
                printf("crcvalue = %x\n", crcvalue);
                crcvalue1 = ((buf_power[len_p - 2]&0xff) << 8) | (buf_power[len_p - 1] & 0xff);
                printf("crcvalue1 = %x\n", crcvalue1);
                if(crcvalue == crcvalue1)
                {
		    puc_power = (uint8_t *)buf_power;
		    aaa = modbus_conv_long(&(puc_power[3]));
		    //ddd = aaa*250*60;
		    //ddd = ddd/18000000;
                    ddd = aaa/(18000000/(250*60));

		    printf("The power meter reading is :%lf\n",ddd);
                    syslog(LOG_CRIT,"[power_meter]power meter send value=%f", ddd);
		    sendMsgToWeb(powerid,POWER,ddd);
                }
				else{
				    syslog(LOG_CRIT,"[power_meter] crc error");
				}
                    
                

		sleep(timep*20);
		printf("power sleep time:%d\n",timep);
		printf("sleep ok\n");
		//水表
		
		p=send(connectfd,buff_water,8,0);
		if(p<0)
	            break;
	        syslog(LOG_CRIT,"[water_meter]water meter  send ok");
	        printf("water meter send ok\n");
		int len_w=0,count_w=0;
	    	while(count_w < 5 && len_w < 13)
	    	{
			q=recv(connectfd,buff2,13,0);
			count_w++;
			if(q<=0)
			    continue;

            memcpy(buf_water+len_w, buff2, q);
			len_w = len_w + q;
            sleep(1);
			
		}
		if(len_w != 13)
		{
			syslog(LOG_CRIT,"[water_meter]water meter read not 13");
			printf("water meter read not 13\n");
			break;
		}
		printf("water meter recv ok\n");
		printf("count_w=%d\n",count_w);
		dump_hex(buf_water,len_w);
		syslog(LOG_CRIT,"[water_meter]water meter  recv ok");


                crcvalue = modbus_crc16( buf_water, len_w - 2 );
                printf("crcvalue = %x\n", crcvalue);
                crcvalue1 = ((buf_water[len_w - 2]&0xff) << 8) | (buf_water[len_w - 1] & 0xff);
                printf("crcvalue1 = %x\n", crcvalue1);
                if(crcvalue == crcvalue1)
                {
		            puc_water = (uint8_t *)buf_water;
		            aaa_w = modbus_conv_longm(&(puc_water[3]));
		            bbb = modbus_conv_real4(&(puc_water[7]));
		            ccc = bbb;
		            ccc = ccc + aaa_w; 
		            printf("The water meter reading is :%f\n",ccc);
                            syslog(LOG_CRIT,"[water_meter]water meter send value=%f", ccc);
		            sendMsgToWeb(waterid,WATER,ccc);
                }
				else
				{
				    syslog(LOG_CRIT,"[water_meter] crc error");
				}


		sleep(timew*20);
		printf("water sleep time:%d\n",timew);
		
		//热表
		m=send(connectfd,buff_heat,8,0);
		if(m<0)
	            break;
	        syslog(LOG_CRIT,"[heat_meter]heat meter  send ok");
	        printf("heat meter send ok\n");
		int len_h=0,count_h=0;
	    	while(count_h < 5 && len_h < 13)
	    	{
			n=recv(connectfd,buff2,13,0);
			count_h++;
			if(n<=0)
			    continue;
                        memcpy(buf_heat+len_h, buff2, n);
			len_h = len_h + n;
			//printf("\nlen_h = %d,count_h = %d\n")
                        sleep(1);
			
		}
		if(len_h != 13)
		{
			syslog(LOG_CRIT,"[heat_meter]heat meter read not 13");
			printf("heat meter read not 13\n");
			break;
		}
		printf("heat meter recv ok\n");
		syslog(LOG_CRIT,"[heat_meter]heat meter  recv ok");
		printf("\n*****heat meter*******\n");
		dump_hex(buf_heat,len_h);


                crcvalue = modbus_crc16( buf_heat, len_h - 2 );
                printf("crcvalue = %x\n", crcvalue);
                crcvalue1 = ((buf_heat[len_h - 2]&0xff) << 8) | (buf_heat[len_h - 1] & 0xff);
                printf("crcvalue1 = %x\n", crcvalue1);
                if(crcvalue == crcvalue1)
                {
		            puc_heat = (uint8_t *)buf_heat;
		            aaa_h = modbus_conv_longm(&(puc_heat[3]));
		            bbb_h = modbus_conv_real4(&(puc_heat[7]));
		            printf("\n************\n");
		            printf("aaa_h = %ld\n",aaa_h);
		            printf("\n************\n");
		            
		            
		            int by_take = 0;
		            by_take = get_By_multiplication_factor(connectfd,buff_heat_take); 
		            printf("\n********in heat meter recv*********\n");
		            printf("By multiplication factor is:%d\n",by_take);
		            
		            
		            ccc_h = bbb_h;
		            ccc_h = ccc_h + aaa_h;
		             printf("The heat meter before count reading is :%f\n",ccc_h);
		            ccc_h = (pow(10,by_take-4))*ccc_h;
		            printf("The heat meter after count reading is :%f\n",ccc_h);
		          // printf("The heat meter reading is :%f\n",pow(10,by_take));
                            syslog(LOG_CRIT,"[heat_meter]heat meter send value=%f", ccc_h);
		            sendMsgToWeb(heatid,HEAT,ccc_h);
                }
				else
				{
				    syslog(LOG_CRIT,"[heat_meter] crc error");
				}



		sleep(timeh*20);
		printf("heat sleep time:%d\n",timeh);
		
		write_to_ini(powerid,waterid,heatid,ddd,ccc,ccc_h);
		
		
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

    get_sleep_time();
    get_device_id();

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
