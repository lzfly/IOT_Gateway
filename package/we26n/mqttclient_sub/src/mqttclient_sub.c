/*
Copyright (c) 2009-2014 Roger Light <roger@atchoo.org>

All rights reserved. This program and the accompanying materials
are made available under the terms of the Eclipse Public License v1.0
and Eclipse Distribution License v1.0 which accompany this distribution.
 
The Eclipse Public License is available at
   http://www.eclipse.org/legal/epl-v10.html
and the Eclipse Distribution License is available at
  http://www.eclipse.org/org/documents/edl-v10.php.
 
Contributors:
   Roger Light - initial implementation and documentation.
*/

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#include <winsock2.h>
#define snprintf sprintf_s
#endif

#include <mosquitto.h>
#include "client_shared.h"

#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <net/if.h>
#include <net/if_arp.h> 
#include <sys/ioctl.h> 

#include <uci.h>

#define MAXINTERFACES   16
char g_localMAC[16];
struct sockaddr_in g_localAddr;

static struct uci_context * uci_ctx;
static struct uci_package * uci_jianyoucfg;

#define MAXTOPIC 128
char mqtt_server[MAXTOPIC];
char mqtt_port[MAXTOPIC];
char mqtt_topic[MAXTOPIC];

void  test_data_cback(struct ubus_request *req, int type, struct blob_attr *msg)
{
	//static const struct blobmsg_policy policy[2] = { { "args", BLOBMSG_TYPE_INT32 }, { "argv", BLOBMSG_TYPE_INT32 } };
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


int  send_set_msg_to_zigbee(char *deviceid, char *attr, char *data)
{
    int  iret;
    uint32_t  id;
    struct ubus_context *ctx;
	static struct blob_buf b;

    printf("[send_set_msg_to_zigbee] start\r\n");

    /**/
	ctx = ubus_connect( NULL );
	if ( NULL == ctx) 
	{
	    fprintf(stderr, "Failed to connect to ubus\n");
	    return -1;
	}

    /**/
	if ( ubus_lookup_id(ctx, "we26n_zigbee_febee", &id) ) {
		fprintf(stderr, "Failed to look up we26n_zigbee_febee object\n");
		return;
	}
	
	blob_buf_init( &b, 0 );

	blobmsg_add_string( &b, "gatewayid", "xxxx" ); // not need

	blobmsg_add_string( &b, "deviceid", deviceid);

	blobmsg_add_string( &b, "devicetype", "xxxx");// not need
	
	blobmsg_add_string( &b, "attr", attr );
	
	blobmsg_add_string( &b, "data", data);	

	printf("[send_set_msg_to_zigbee]ubus_invoke data = %s\r\n", data);
	/**/
	ubus_invoke( ctx, id, "ctrlcmd", b.head, test_data_cback, 0, 3000);

    /**/
	ubus_free(ctx);
	return 0;
	
}


int  send_get_msg_to_zigbee(char *deviceid, char *attr)
{
    int  iret;
    uint32_t  id;
    struct ubus_context *ctx;
	static struct blob_buf b;

    printf("[send_get_msg_to_zigbee] start\r\n");

    /**/
	ctx = ubus_connect( NULL );
	if ( NULL == ctx) 
	{
	    fprintf(stderr, "Failed to connect to ubus\n");
	    return -1;
	}

    /**/
	if ( ubus_lookup_id(ctx, "we26n_zigbee_febee", &id) ) {
		fprintf(stderr, "Failed to look up we26n_zigbee_febee object\n");
		return;
	}
	
	blob_buf_init( &b, 0 );

	blobmsg_add_string( &b, "gatewayid", "xxxx" ); // not need

	blobmsg_add_string( &b, "deviceid", deviceid);

	blobmsg_add_string( &b, "devicetype", "xxxx");// not need
	
	blobmsg_add_string( &b, "attr", attr );

	printf("[send_get_msg_to_zigbee]ubus_invoke attr = %s\r\n", attr);
	/**/
	ubus_invoke( ctx, id, "getstatecmd", b.head, test_data_cback, 0, 3000);

    /**/
	ubus_free(ctx);
	return 0;
	
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
	
	        if(0 != strncmp(buf[intrface].ifr_name, "ra0", strlen("ra0")))
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

int get_mqttclient_config()
{
    int rtn = 0;
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
		return -1;
    }
	else
	{
	        char *value = NULL;
            struct uci_element *e   = NULL;
			int rtn;
            printf("uci load jianyou config success\n");


            /* scan jianyou config ! */
            uci_foreach_element(&uci_jianyoucfg->sections, e)
            {
                struct uci_section *s = uci_to_section(e);

                if(0 == strcmp(s->type, "mqttclient"))
                {
                    printf("%s(), type: %s\n", __FUNCTION__, s->type);

                    value = uci_lookup_option_string(uci_ctx, s, "server");
                    if(value){
                            sprintf(mqtt_server, "%s", value);
                            printf("%s(), mqtt_server: %s\n", __FUNCTION__, mqtt_server);
                            rtn = 0;
                        }else{
                            printf("%s(), mqtt_server not found\n", __FUNCTION__);
							rtn = -1;
							goto exit;
                     }
					 
					value = uci_lookup_option_string(uci_ctx, s, "port");
                    if(value){
                            sprintf(mqtt_port, "%s", value);
                             printf("%s(), mqtt_port: %s\n", __FUNCTION__, mqtt_port);
                        }else{
                            printf("%s(), mqtt_port not found\n", __FUNCTION__);
							rtn = -1;
							goto exit;
                     }
					 
					//value = uci_lookup_option_string(uci_ctx, s, "topic");
                    if(getLocalIPandMAC() == 0){

							sprintf(mqtt_topic, "we26n_%s", g_localMAC);
                            printf("%s(), mqtt_topic: %s\n", __FUNCTION__, mqtt_topic);

                        }else{
                            printf("%s(), mqtt_topic not found\n", __FUNCTION__);
							rtn = -1;
							goto exit;
                     }
					 
                     break;
                 }

             }
             
    }
exit:
    uci_unload(uci_ctx, uci_jianyoucfg);
    return rtn;
}


bool process_messages = true;
int msg_count = 0;

void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	struct mosq_config *cfg;
	int i;
	bool res;

	if(process_messages == false) return;

	assert(obj);
	cfg = (struct mosq_config *)obj;

	if(message->retain && cfg->no_retain) return;
	if(cfg->filter_outs){
		for(i=0; i<cfg->filter_out_count; i++){
			mosquitto_topic_matches_sub(cfg->filter_outs[i], message->topic, &res);
			if(res) return;
		}
	}

	if(cfg->verbose){
		if(message->payloadlen){
			printf("%s ", message->topic);
			fwrite(message->payload, 1, message->payloadlen, stdout);
			if(cfg->eol){
				printf("\n");
			}
		}else{
			if(cfg->eol){
				printf("%s (null)\n", message->topic);
			}
		}
		fflush(stdout);
	}else{
		if(message->payloadlen){
			fwrite(message->payload, 1, message->payloadlen, stdout);
			if(cfg->eol){
				printf("\n");
			}
			fflush(stdout);
			
			
		    char jsondata[256];
			printf("payloadlen=%d\n", message->payloadlen);
			if(message->payloadlen < 255)
			{
			    memcpy(jsondata,message->payload, message->payloadlen);
			    jsondata[message->payloadlen] = 0;
				printf("jsondata=%s\n", jsondata);
				
				char *haystack=jsondata;
				char *haystack1;
				char *needle="|";
				char* buf = strstr( haystack, needle);
				if( buf != NULL )
					buf[0]='\0';
				if(strlen(haystack) != 0 )
				{
					printf( "CMD=%s\n ", haystack);
					
				    char *needle1=":";
					char* buf1 = strstr( haystack, needle1);
					if( buf1 != NULL )
					    buf1[0]='\0';
					if(strlen(haystack) != 0  && strcmp(haystack, "C") == 0) //CMD 
					{
					    if(buf1 == NULL)
						{
							printf("mqtt message find not S/G/N \n");
							goto exit;
						}
						haystack1 = buf1 + strlen(needle1);
					    if(strlen(haystack1) != 0  && strcmp(haystack1, "S") == 0) // SET
						{
						    if( buf == NULL )
							{
								printf("mqtt message find not M \n");
								goto exit;
							}
						    haystack = buf + strlen(needle);
							if(strlen(haystack) == 0)
							{
								printf("mqtt message find not M \n");
								goto exit;
							}
					        /* Get next token: */
				            buf = strstr( haystack, needle);
							if( buf != NULL )
							    buf[0]='\0';
							if(strlen(haystack) != 0)
							{

					             printf( "MODULE=%s\n ", haystack);
					             buf1 = strstr( haystack, needle1);
								 if( buf1 != NULL )
					                buf1[0]='\0';
								 if(strlen(haystack) != 0  && strcmp(haystack, "M") == 0) //MODULE
								 {
								 	if(buf1 == NULL)
						           {
							           printf("mqtt message find not Z/W/4/B/G \n");
							           goto exit;
						            }
									haystack1 = buf1 + strlen(needle1);
								     if(strlen(haystack1) != 0  && strcmp(haystack1, "Z") == 0) // ZIGBEE
									 {
									    char deviceid[65];
										char attr[33];
										char data[33];
									    if( buf == NULL )
							           {
								           printf("mqtt message find not DEVICEID \n");
								           goto exit;
							            }
									    haystack = buf + strlen(needle);
										if(strlen(haystack) == 0)
										{
											printf("mqtt message find DEVICEID\n");
											goto exit;
										}
					                    /* Get next token: */
				                        buf = strstr( haystack, needle);
										if(buf != NULL)
										    buf[0]='\0';
										if(strlen(haystack) != 0)
										{
					                        printf( "DEVICEID=%s\n ", haystack);
											if(strlen(haystack) > 64)
											{
												printf("DEVICEID is too large\n");
											    goto exit;
											}    
											strcpy(deviceid, haystack);
										}
										else
										{
										    printf("mqtt message find DEVICEID\n");
											goto exit;
										}
										
									    if( buf == NULL )
							           {
								           printf("mqtt message find not ATTR \n");
								           goto exit;
							            }
										
									    haystack = buf + strlen(needle);
										
										/* Get next token: */
				                        buf = strstr( haystack, needle);
										if(buf != NULL)
										    buf[0]='\0';
										if(strlen(haystack) != 0)
										{
					                        printf( "ATTR=%s\n ", haystack);
											if(strlen(haystack) > 32)
											{
												printf("ATTR is too large\n");
											    goto exit;
											}    
											strcpy(attr, haystack);
										}
										else
										{
										    printf("mqtt message find ATTR\n");
											goto exit;
										}
										
										if( buf == NULL )
							           {
								           printf("mqtt message find not DATA \n");
								           goto exit;
							            }
																			 
									    buf = buf +1;
										if(strlen(buf) > 0)
										{
					                        printf( "DATA=%s\n ", buf);
											if(strlen(buf) > 32)
											{
												printf("DATA is too large\n");
											    goto exit;
											}    
											strcpy(data, buf);
										}
										else
										{
										    printf("mqtt message find DATA\n");
											goto exit;
										}
										
										send_set_msg_to_zigbee(deviceid, attr, data);
										
									 }
									 else if(strcmp(&buf1[1], "B") == 0) // BLE
									 {}
									 else if(strcmp(&buf1[1], "4") == 0) // 470M
									 {}
									 else if(strcmp(&buf1[1], "W") == 0) // WIFI
									 {}
									 else if(strcmp(&buf1[1], "G") == 0) // GATEWAY
									 {}
									 else
									 {
									     printf("mqtt message find not Z/W/4/B/G \n");
										 goto exit;
									 }
								 }
								 else
								 {
								     printf("mqtt message find not M \n");
									 goto exit;
								 }
								 
							}
							else
							{
							    printf("mqtt message find not M \n");
								goto exit;
							}
						}
						else if(strcmp(&buf1[1], "G") == 0)
						{
						    if( buf == NULL )
							{
								printf("mqtt message find not M \n");
								goto exit;
							}
						    haystack = buf + strlen(needle);
							if(strlen(haystack) == 0)
							{
								printf("mqtt message find not M \n");
								goto exit;
							}
					        /* Get next token: */
				            buf = strstr( haystack, needle);
							if( buf != NULL )
							    buf[0]='\0';
							if(strlen(haystack) != 0)
							{

					             printf( "MODULE=%s\n ", haystack);
					             buf1 = strstr( haystack, needle1);
								 if( buf1 != NULL )
					                buf1[0]='\0';
								 if(strlen(haystack) != 0  && strcmp(haystack, "M") == 0) //MODULE
								 {
								 	if(buf1 == NULL)
						           {
							           printf("mqtt message find not Z/W/4/B/G \n");
							           goto exit;
						            }
									haystack1 = buf1 + strlen(needle1);
								     if(strlen(haystack1) != 0  && strcmp(haystack1, "Z") == 0) // ZIGBEE
									 {
									 	char deviceid[65];
										char attr[33];
										
									    if( buf == NULL )
							           {
								           printf("mqtt message find not DEVICEID \n");
								           goto exit;
							            }
									    haystack = buf + strlen(needle);
										if(strlen(haystack) == 0)
										{
											printf("mqtt message find DEVICEID\n");
											goto exit;
										}
					                    /* Get next token: */
				                        buf = strstr( haystack, needle);
										if(buf != NULL)
										    buf[0]='\0';
										if(strlen(haystack) != 0)
										{
					                        printf( "DEVICEID=%s\n ", haystack);
											if(strlen(haystack) > 64)
											{
												printf("DEVICEID is too large\n");
											    goto exit;
											}    
											strcpy(deviceid, haystack);
										}
										else
										{
										    printf("mqtt message find DEVICEID\n");
											goto exit;
										}
													
										if( buf == NULL )
							           {
								           printf("mqtt message find not ATTR \n");
								           goto exit;
							            }
																			 
									    buf = buf +1;
										if(strlen(buf) > 0)
										{
					                        printf( "ATTR=%s\n ", buf);
											if(strlen(buf) > 32)
											{
												printf("ATTR is too large\n");
											    goto exit;
											}    
											strcpy(attr, buf);
										}
										else
										{
										    printf("mqtt message find ATTR\n");
											goto exit;
										}
										
										send_get_msg_to_zigbee(deviceid, attr);
										
									 }
									 else if(strcmp(&buf1[1], "B") == 0) // BLE
									 {}
									 else if(strcmp(&buf1[1], "4") == 0) // 470M
									 {}
									 else if(strcmp(&buf1[1], "W") == 0) // WIFI
									 {}
									 else if(strcmp(&buf1[1], "G") == 0) // GATEWAY
									 {}
									 else
									 {
									     printf("mqtt message find not Z/W/4/B/G \n");
										 goto exit;
									 }
								 }
								 else
								 {
								     printf("mqtt message find not M \n");
									 goto exit;
								 }
								 
							}
							else
							{
							    printf("mqtt message find not M \n");
								goto exit;
							}
						}
						else if(strcmp(&buf1[1], "N") == 0)
						{
						
						}
						else
						{
						   printf("mqtt message find not S/G/N \n");
						   goto exit;
						}
					
					}
					else{
					    printf("mqtt message find not C \n");
						goto exit;
					}

				}
				else
				{
				    printf("mqtt message find not C \n");
					goto exit;
				}
				
			}
			else
			{
				printf("mqtt message length is too large\n");
				goto exit;
			}
			
		}
	}
	
exit:
	if(cfg->msg_count>0){
		msg_count++;
		if(cfg->msg_count == msg_count){
			process_messages = false;
			mosquitto_disconnect(mosq);
		}
	}
}

void my_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
	int i;
	struct mosq_config *cfg;

	assert(obj);
	cfg = (struct mosq_config *)obj;
    printf("mosquitto_sub  connect result=%d\n", result);
	if(!result){
		for(i=0; i<cfg->topic_count; i++){
			mosquitto_subscribe(mosq, NULL, cfg->topics[i], cfg->qos);
		}
	}else{
		if(result && !cfg->quiet){
			fprintf(stderr, "%s\n", mosquitto_connack_string(result));
		}
	}
}

void my_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	int i;
	struct mosq_config *cfg;
    printf("mosquitto_sub  my_subscribe_callback\n");
	assert(obj);
	cfg = (struct mosq_config *)obj;

	if(!cfg->quiet) printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(i=1; i<qos_count; i++){
		if(!cfg->quiet) printf(", %d", granted_qos[i]);
	}
	if(!cfg->quiet) printf("\n");
}

void my_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
	printf("%s\n", str);
}

void print_usage(void)
{
	int major, minor, revision;

	mosquitto_lib_version(&major, &minor, &revision);
	printf("mosquitto_sub is a simple mqtt client that will subscribe to a single topic and print all messages it receives.\n");
	printf("mosquitto_sub version %s running on libmosquitto %d.%d.%d.\n\n", "1.4.2", major, minor, revision);
	printf("Usage: mosquitto_sub [-c] [-h host] [-k keepalive] [-p port] [-q qos] [-R] -t topic ...\n");
	printf("                     [-C msg_count] [-T filter_out]\n");
#ifdef WITH_SRV
	printf("                     [-A bind_address] [-S]\n");
#else
	printf("                     [-A bind_address]\n");
#endif
	printf("                     [-i id] [-I id_prefix]\n");
	printf("                     [-d] [-N] [--quiet] [-v]\n");
	printf("                     [-u username [-P password]]\n");
	printf("                     [--will-topic [--will-payload payload] [--will-qos qos] [--will-retain]]\n");
#ifdef WITH_TLS
	printf("                     [{--cafile file | --capath dir} [--cert file] [--key file]\n");
	printf("                      [--ciphers ciphers] [--insecure]]\n");
#ifdef WITH_TLS_PSK
	printf("                     [--psk hex-key --psk-identity identity [--ciphers ciphers]]\n");
#endif
#endif
#ifdef WITH_SOCKS
	printf("                     [--proxy socks-url]\n");
#endif
	printf("       mosquitto_sub --help\n\n");
	printf(" -A : bind the outgoing socket to this host/ip address. Use to control which interface\n");
	printf("      the client communicates over.\n");
	printf(" -c : disable 'clean session' (store subscription and pending messages when client disconnects).\n");
	printf(" -C : disconnect and exit after receiving the 'msg_count' messages.\n");
	printf(" -d : enable debug messages.\n");
	printf(" -h : mqtt host to connect to. Defaults to localhost.\n");
	printf(" -i : id to use for this client. Defaults to mosquitto_sub_ appended with the process id.\n");
	printf(" -I : define the client id as id_prefix appended with the process id. Useful for when the\n");
	printf("      broker is using the clientid_prefixes option.\n");
	printf(" -k : keep alive in seconds for this client. Defaults to 60.\n");
	printf(" -N : do not add an end of line character when printing the payload.\n");
	printf(" -p : network port to connect to. Defaults to 1883.\n");
	printf(" -P : provide a password (requires MQTT 3.1 broker)\n");
	printf(" -q : quality of service level to use for the subscription. Defaults to 0.\n");
	printf(" -R : do not print stale messages (those with retain set).\n");
#ifdef WITH_SRV
	printf(" -S : use SRV lookups to determine which host to connect to.\n");
#endif
	printf(" -t : mqtt topic to subscribe to. May be repeated multiple times.\n");
	printf(" -T : topic string to filter out of results. May be repeated.\n");
	printf(" -u : provide a username (requires MQTT 3.1 broker)\n");
	printf(" -v : print published messages verbosely.\n");
	printf(" -V : specify the version of the MQTT protocol to use when connecting.\n");
	printf("      Can be mqttv31 or mqttv311. Defaults to mqttv31.\n");
	printf(" --help : display this message.\n");
	printf(" --quiet : don't print error messages.\n");
	printf(" --will-payload : payload for the client Will, which is sent by the broker in case of\n");
	printf("                  unexpected disconnection. If not given and will-topic is set, a zero\n");
	printf("                  length message will be sent.\n");
	printf(" --will-qos : QoS level for the client Will.\n");
	printf(" --will-retain : if given, make the client Will retained.\n");
	printf(" --will-topic : the topic on which to publish the client Will.\n");
#ifdef WITH_TLS
	printf(" --cafile : path to a file containing trusted CA certificates to enable encrypted\n");
	printf("            certificate based communication.\n");
	printf(" --capath : path to a directory containing trusted CA certificates to enable encrypted\n");
	printf("            communication.\n");
	printf(" --cert : client certificate for authentication, if required by server.\n");
	printf(" --key : client private key for authentication, if required by server.\n");
	printf(" --ciphers : openssl compatible list of TLS ciphers to support.\n");
	printf(" --tls-version : TLS protocol version, can be one of tlsv1.2 tlsv1.1 or tlsv1.\n");
	printf("                 Defaults to tlsv1.2 if available.\n");
	printf(" --insecure : do not check that the server certificate hostname matches the remote\n");
	printf("              hostname. Using this option means that you cannot be sure that the\n");
	printf("              remote host is the server you wish to connect to and so is insecure.\n");
	printf("              Do not use this option in a production environment.\n");
#ifdef WITH_TLS_PSK
	printf(" --psk : pre-shared-key in hexadecimal (no leading 0x) to enable TLS-PSK mode.\n");
	printf(" --psk-identity : client identity string for TLS-PSK mode.\n");
#endif
#endif
#ifdef WITH_SOCKS
	printf(" --proxy : SOCKS5 proxy URL of the form:\n");
	printf("           socks5h://[username[:password]@]hostname[:port]\n");
	printf("           Only \"none\" and \"username\" authentication is supported.\n");
#endif
	printf("\nSee http://mosquitto.org/ for more information.\n\n");
}

int main(int argc, char *argv[])
{
	struct mosq_config cfg;
	struct mosquitto *mosq = NULL;
	int rc;
	
    rc = get_mqttclient_config();
	if(rc){
		printf("mqttclient_sub read config fail\n");
		return 1;
	}
	
	int argc1 = 7;
	char *argv1[7];
	argv1[0] = "mqttclient_sub";
	argv1[1] = "-h";
	argv1[2] = mqtt_server;
	argv1[3] = "-p";
	argv1[4] = mqtt_port;
	argv1[5] = "-t";
	argv1[6] = mqtt_topic;
	
	//rc = client_config_load(&cfg, CLIENT_SUB, argc, argv);
	rc = client_config_load(&cfg, CLIENT_SUB, argc1, argv1);
	if(rc){
		client_config_cleanup(&cfg);
		if(rc == 2){
			/* --help */
			print_usage();
		}else{
			fprintf(stderr, "\nUse 'mosquitto_sub --help' to see usage.\n");
		}
		return 1;
	}
	
	rc = get_mqttclient_config(&cfg);
	if(rc){
		client_config_cleanup(&cfg);
		return 1;
	}
	
	mosquitto_lib_init();

	while(1){
	    printf("mosquitto_sub  while again\n");
		
		if(client_id_generate(&cfg, "mosqsub")){
			return 1;
		}

		mosq = mosquitto_new(cfg.id, cfg.clean_session, &cfg);
		if(!mosq){
			switch(errno){
				case ENOMEM:
					if(!cfg.quiet) fprintf(stderr, "Error: Out of memory.\n");
					break;
				case EINVAL:
					if(!cfg.quiet) fprintf(stderr, "Error: Invalid id and/or clean_session.\n");
					break;
			}
			mosquitto_lib_cleanup();
			return 1;
		}
		if(client_opts_set(mosq, &cfg)){
			return 1;
		}
		if(cfg.debug){
			mosquitto_log_callback_set(mosq, my_log_callback);
			mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
		}
		mosquitto_connect_callback_set(mosq, my_connect_callback);
		mosquitto_message_callback_set(mosq, my_message_callback);

		while(1){
		    printf("mosquitto_sub  connect again\n");
			rc = client_connect(mosq, &cfg);
			if(rc == 0) 
			     break;
			sleep(10);
		}
		printf("mosquitto_sub  connect success\n");

		rc = mosquitto_loop_forever(mosq, -1, 1);

		printf("mosquitto_sub  exit loop\n");
		
		mosquitto_destroy(mosq);
	
	}
	
	mosquitto_lib_cleanup();

	if(cfg.msg_count>0 && rc == MOSQ_ERR_NO_CONN){
		rc = 0;
	}
	if(rc){
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
	}
	return rc;
}

