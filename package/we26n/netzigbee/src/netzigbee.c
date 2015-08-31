

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>
#include "we26n_type.h"

#include "gateway.h"
#include <uci.h>
#include <stdint.h>

static struct uci_context * uci_ctx;
static struct uci_package * uci_ennconfig;
static struct uci_context * uci_ctx_i;
static struct uci_package * uci_zigbeeid;
char ReportTime_tem[8];
char ReportTime_hum[8];


 int ReadZigbeeId()
   {
    int i = 0;
    int j = 0;
    if (!uci_ctx_i)
    {
        uci_ctx_i = uci_alloc_context();
    }
    else
    {
        uci_zigbeeid = uci_lookup_package(uci_ctx_i, "zigbeeid");
        if (uci_zigbeeid)
            uci_unload(uci_ctx_i, uci_zigbeeid);
    }

    if (uci_load(uci_ctx_i, "zigbeeid", &uci_zigbeeid))
    {
        printf("uci load zigbeeid config fail\n");
    }else
	{
	    char *value_i = NULL;
            struct uci_element *e_i   = NULL;
            printf("uci load zigbeeid config success\n");


            /* scan zigbeeid config ! */
            uci_foreach_element(&uci_zigbeeid->sections, e_i)
            {
            	printf("uci_foreach_element\n");
                struct uci_section *s_i = uci_to_section(e_i);
               	struct uci_option * o = uci_lookup_option(uci_ctx_i,s_i,"id");
               	if((NULL != o)&&(UCI_TYPE_LIST == o->type))
               	{
               		struct uci_element *e_ii = NULL;
               		uci_foreach_element(&o->v.list,e_ii)
               		{
               			//printf("id = %s\n",e_ii->name);
               			i += sprintf(&ZigbeeId[i],"%s ", e_ii -> name);
               			
                         	printf("i = %d\n",i);	
               		}
               		
               		
               			printf("id = %s\n",&ZigbeeId[0]);
               			printf("len = %d\n",strlen(ZigbeeId));
               			
               	}

             }
             
    }

	return 0;
}





int GetReportTime()
{

 if (!uci_ctx)
    {
        uci_ctx = uci_alloc_context();
    }
    else
    {
        uci_ennconfig = uci_lookup_package(uci_ctx, "ennconfig");
        if (uci_ennconfig)
            uci_unload(uci_ctx, uci_ennconfig);
    }

    if (uci_load(uci_ctx, "ennconfig", &uci_ennconfig))
    {
        printf("uci load ENN config fail\n");
    }else
	{
	    char *value_tem = NULL;
	    char *value_hum = NULL;
            struct uci_element *e   = NULL;
            printf("uci load ennconfig success\n");


            /* scan ENN config ! */
            uci_foreach_element(&uci_ennconfig->sections, e)
            {
                struct uci_section *s = uci_to_section(e);
                if(0 == strcmp(s->type, "zigbee"))
                {
                    printf("%s(), type: %s\n", __FUNCTION__, s->type);

                    value_tem = uci_lookup_option_string(uci_ctx, s, "temp_interval");
                   
                    if(value_tem)
                    {
                            strcpy(ReportTime_tem, value_tem);
                            printf("%s(), tem report time: %s\n", __FUNCTION__, ReportTime_tem);
                            g_ReportTime.tem_time=strtoul(ReportTime_tem,NULL,10);
                            printf("report time:%d\n",g_ReportTime.tem_time);
                     }
                     else{
                            printf("%s(), tem report time not found\n", __FUNCTION__);
                     }
                      value_hum = uci_lookup_option_string(uci_ctx, s, "hum_interval"); 
                   
                      if(value_hum)
                    {
                            strcpy(ReportTime_hum, value_hum);
                            printf("%s(), hum report time: %s\n", __FUNCTION__, ReportTime_hum);
                            g_ReportTime.hum_time=strtoul(ReportTime_hum,NULL,10);
                            printf("hum report time:%d\n",g_ReportTime.hum_time);
                     }
                     else{
                            printf("%s(), hum report time not found\n", __FUNCTION__);
                     }
                   /*   value_h = uci_lookup_option_string(uci_ctx, s, "heatmeter"); 
                       if(value_h)
                    {
                            strcpy(heatid, value_h);
                            printf("%s(), heatid: %s\n", __FUNCTION__, heatid);
                     }
                     else{
                            printf("%s(), heat_meter_id not found\n", __FUNCTION__);
                  }*/
                     break;
            }

          }
             
   }
	return 0;


}

/*static struct blob_buf b;

static int zigbee_info( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{
   
	blob_buf_init( &b, 0 );
	blobmsg_add_u32( &b, "args",  1234 );
	blobmsg_add_u32( &b, "argv",  5678 );

    
    ubus_send_reply(ctx, req, b.head);
    return UBUS_STATUS_OK;
}


static const struct ubus_method zigbee_methods[] = {
	UBUS_METHOD_NOARG("info",  zigbee_info ),
};


static struct ubus_object_type zigbee_object_type =
	UBUS_OBJECT_TYPE("netzigbee_iface", zigbee_methods);

static struct ubus_object zigbee_object = {
	.name = "we26n.netzigbee",
	.type = &zigbee_object_type,
	.methods = zigbee_methods,
	.n_methods = ARRAY_SIZE(zigbee_methods),
};



void server_main( struct ubus_context *ctx )
{
	int ret;

	ret = ubus_add_object(ctx, &zigbee_object);
	if (ret)
		fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
    
	uloop_run();
}*/


extern void *  beginSearch();
//extern void *  receiveDeviceMsg();
//extern void *  ctrlDevice();

int  prepare_threads( void )
{
    int  iret;
    pthread_t  aux_thrd;

    
    /**/
    iret = pthread_create( &aux_thrd, NULL, beginSearch, NULL );
    if ( 0 != iret )
    {
        printf( "aux pthread create fail, %d", iret );
        return -1;
    }

    return 0;
}


int  main( void )
{
    int  iret;
    struct ubus_context *ctx;
    GetReportTime();
    ReadZigbeeId();
    /**/
    iret = prepare_threads();
    if ( 0 != iret )
    {
        fprintf( stderr, "Failed to prepare thread\n" );
        return 1;
    }


	while(1)
		sleep(1000);

	return 0;
	
}



