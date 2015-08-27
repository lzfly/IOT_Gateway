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


#include <errno.h>
#include <fcntl.h>
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


#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>

#include <uci.h>

#define MAX_MESSAGE 129

static struct uci_context * uci_ctx;
static struct uci_package * uci_jianyoucfg;

#define MAXTOPIC 128
char mqtt_server[MAXTOPIC];
char mqtt_port[MAXTOPIC];
char mqtt_topic[MAXTOPIC];

enum {
    ALERT_MODULE,
    ALERT_NOTICE,
    __CTRLCMD_MAX
};


static const struct blobmsg_policy  alertnotice_policy[] = {
    [ALERT_MODULE] = { .name = "alertmodule", .type = BLOBMSG_TYPE_STRING },
	[ALERT_NOTICE] = { .name = "alertnotice", .type = BLOBMSG_TYPE_STRING },
};

static int mqttclient_pub_alertnotice( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{

	struct blob_attr * tb[__CTRLCMD_MAX];
    const char * alertmodule;
	const char * alertnotice;
    static struct blob_buf b;

	printf("[mqttclient_pub_alertnotice]start\r\n");
	
    /**/
	blobmsg_parse( alertnotice_policy, ARRAY_SIZE(alertnotice_policy), tb, blob_data(msg), blob_len(msg));

	if ( tb[ALERT_MODULE] )
	{
		alertmodule = blobmsg_data( tb[ALERT_MODULE] );
		printf( "alertmodule = %s\n", alertmodule );
	}

	if ( tb[ALERT_NOTICE] )
	{
		alertnotice = blobmsg_data( tb[ALERT_NOTICE] );
		printf( "alertnotice = %s\n", alertnotice );
	}
    char message[MAX_MESSAGE + 1];
	if(strlen(alertmodule) > 1)
	{
	    printf("module length error");
	}
	if(strlen(alertnotice) > 110)
	{
	    printf("notice length error");
	}
	
	sprintf(message, "C:N|M:%s|%s", alertmodule, alertnotice);
	printf("mqtt notice = %s", message);
	

	char mqttpubcmd[180];
	//sprintf(mqttpubcmd, "mosquitto_pub -h 101.200.1.101 -p 8020 -t sensor  -m  \"%s\"", message);
	sprintf(mqttpubcmd, "mosquitto_pub -h %s -p %s -t %s  -m  \"%s\"", mqtt_server, mqtt_port, mqtt_topic, message);
	system(mqttpubcmd);
	
done:

    /* send reply */
	blob_buf_init( &b, 0 );
	blobmsg_add_string( &b, "return",  "ok" );
    
    /**/
    ubus_send_reply( ctx, req, b.head );
	
	/**/
	
	
    return UBUS_STATUS_OK;
    
}


static const struct ubus_method mqttclient_pub_methods[] = {
	UBUS_METHOD( "alertnotice",  mqttclient_pub_alertnotice, alertnotice_policy ),
};


static struct ubus_object_type system_object_type =
	UBUS_OBJECT_TYPE("mqttclient_pub_iface", mqttclient_pub_methods);

static struct ubus_object mqttclient_pub_object = {
	.name = "we26n_mqttclient_pub",
	.type = &system_object_type,
	.methods = mqttclient_pub_methods,
	.n_methods = ARRAY_SIZE(mqttclient_pub_methods),
};


int  ubus_server_init( void )
{
    struct ubus_context *ctx;
	int ret;
	
    printf("[ubus_server_init]start\r\n");
		
    /**/
	uloop_init();

	signal(SIGPIPE, SIG_IGN);	
    
    /**/
	ctx = ubus_connect( NULL );
	if ( NULL == ctx) 
	{
	    return 1;
	}

    /**/
	ubus_add_uloop( ctx );

    /**/
	ret = ubus_add_object( ctx, &mqttclient_pub_object );
	if (ret)
	{
		fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
		return 2;
    }
    printf("[ubus_server_init] ok\r\n");
	
	uloop_run();
    printf("[ubus_server_init]exit run\r\n");

	uloop_done();
	
    return 0;
    
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
					 
					value = uci_lookup_option_string(uci_ctx, s, "userid");
                    if(value){
							sprintf(mqtt_topic, "%s", value);
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

int main(int argc, char *argv[])
{
    get_mqttclient_config();
	ubus_server_init();
	return 0;
}
