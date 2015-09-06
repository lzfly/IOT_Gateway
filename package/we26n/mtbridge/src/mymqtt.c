
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>
#include <unistd.h>


#include <mosquitto.h>
#include "myqueue.h"
#include "mymqtt.h"


typedef struct _tag_mmqt_context
{
    struct mosquitto * mosq;
    intptr_t qctx;
    char  topic[20];
    
} mmqt_context_t;



static uint8_t  recognize_action( char * ptr )
{
    if ( 0 == strcmp(ptr, "C:S") )
    {
        return MT_ACT_SET;
    }

    if ( 0 == strcmp(ptr, "C:G") )
    {
        return MT_ACT_GET;
    }

    if ( 0 == strcmp(ptr, "C:N") )
    {
        return MT_ACT_NTC;
    }
    
    return 0;
}



static uint8_t  recognize_object( char * ptr )
{
printf("action : %s\n", ptr );
    if ( 0 == strcmp(ptr, "M:Z") )
    {
        return MT_OBJ_ZIG;
    }

    if ( 0 == strcmp(ptr, "M:W") )
    {
        return MT_OBJ_WIFI;
    }

    if ( 0 == strcmp(ptr, "M:4") )
    {
        return MT_OBJ_470;
    }
    
    if ( 0 == strcmp(ptr, "M:B") )
    {
        return MT_OBJ_BLE;
    }

    if ( 0 == strcmp(ptr, "M:G") )
    {
        return MT_OBJ_GATE;
    }

    return 0;
    
}



void  my_message_callback(struct mosquitto * mosq, void * obj, const struct mosquitto_message * message )
{
    mmqt_context_t * pctx;
    mmqt_msg_t * pmsg;
    char * ptr;
    char * ped;
    uint8_t  act;
    uint8_t  ooo;
    int  tsize;
    
    /**/
    pctx = (mmqt_context_t *)obj;
    
    /**/
    if ( 0 != strcmp( message->topic, pctx->topic ) )
    {
        return;
    }

    /**/
  	if ( message->payloadlen <= 0 )
  	{
      	return;
  	}

    /**/
    printf( "inner cbk : %s\n", (char *)message->payload );
    
    /**/
    ptr = (char *)(message->payload);
    ped = strchr( ptr, '|' );
    if ( NULL == ped )
    {
        return;
    }

    /**/
    *ped = '\0';
    act = recognize_action( ptr );
    if ( 0 == act )
    {
        return;
    }

    /**/
    ptr = ped + 1;
    ped = strchr( ptr, '|' );
    if ( NULL == ped )
    {
        return;
    }

    /**/
    *ped = '\0';
    ooo = recognize_object( ptr );
    if ( 0 == ooo )
    {
        return;
    }

    /**/
    ptr = ped + 1;
    tsize = sizeof(mmqt_msg_t) + strlen(ptr);
    pmsg = alloca( tsize );
    if ( NULL == pmsg )
    {
        return;
    }

    /**/
    pmsg->action = act;
    pmsg->object = ooo;
    strcpy( pmsg->msg, ptr );

    /**/
    msq_enqueue( pctx->qctx, tsize, pmsg );
    write( ((int *)pctx->mosq)[1], &act, 1 );
    return;
  	
}



void  my_connect_callback(struct mosquitto * mosq, void * obj, int result )
{
    mmqt_context_t * pctx;

    /**/
    pctx = (mmqt_context_t *)obj;

    /**/
	if( 0 == result )
	{
		mosquitto_subscribe( mosq, NULL, pctx->topic, 0 );
	}
	else
	{
	    fprintf(stderr, "%s\n", mosquitto_connack_string(result));
	}

	return;
}


void  my_disconnect_callback(struct mosquitto * mosq, void * obj, int result )
{
    printf( "dis con, %d\n", result );
	fflush(stdout);
	return;
}



int  mmqt_start_run( intptr_t ctx )
{
    int  iret;
    mmqt_context_t * pctx;

    /**/
    pctx = (mmqt_context_t *)ctx;

    
    /**/
    iret = mosquitto_loop_start( pctx->mosq );
    if ( 0 != iret )
    {
        return 1;
    }
    
    return 0;
}


int  mmqt_get_fd( intptr_t ctx, int * pfd )
{
    mmqt_context_t * pctx;
    int  * ptr;
    
    /**/
    pctx = (mmqt_context_t *)ctx;

    /**/
    ptr = (int *)(pctx->mosq);
    *pfd = ptr[2];  /* mosq->sockpairW */
    return 0;
}


int  mmqt_publish( intptr_t ctx, char * topic, char * msg )
{
    int  iret;
    mmqt_context_t * pctx;

    /**/
    pctx = (mmqt_context_t *)ctx;

    /* qos = 0, retain = 0. */
    iret = mosquitto_publish( pctx->mosq, NULL, topic, strlen(msg), msg, 0, 0 );
    if ( 0 != iret )
    {
        return 1;
    }

    return 0;
}



static int  mmqt_get_macaddr( char * pmac )
{
    FILE * fout;
    char  tbuf[100];
    char * ptr;
    char * dst;
    
    /**/
    fout = popen( "eth_mac r lan", "r" );
    if ( fout == NULL )
    {
        return 1;
    }

    /**/
    ptr = fgets( tbuf, 90, fout );
    if ( NULL == ptr )
    {
        return 2;
    }

    /**/
    pclose( fout );

    /**/
    dst = pmac;
    ptr = tbuf;
    while( '\0' != *(ptr) )
    {
        if ( *(ptr) != ':' )
        {
            *dst++ = *ptr++;
        }
        else
        {
            ptr++;
        }
    }

    /**/
    if ( *(dst-1) == 0x0a )
    {
        *(dst-1) = '\0';
    }
    
    *dst = '\0';
    return 0;
    
}



int  mmqt_init( intptr_t qctx, char * ipdr, int port, intptr_t * pret )
{
    int  iret;
    struct mosquitto * mosq;
    int  version = MQTT_PROTOCOL_V31;
    mmqt_context_t * pctx;

    /**/
    pctx = (mmqt_context_t *)malloc( sizeof(mmqt_context_t) );
    if ( NULL == pctx )
    {
        return 1;
    }

    /**/
    sprintf( pctx->topic, "we36n_" );
    iret = mmqt_get_macaddr( &(pctx->topic[6]) );
    if ( 0 != iret )
    {
        return 3;
    }
    printf( "inner mmqt, %s\n", pctx->topic );
    
    /**/
    mosquitto_lib_init();
    
    mosq = mosquitto_new( pctx->topic, true, pctx );

    /* option */
    mosquitto_max_inflight_messages_set( mosq, 20 );
    mosquitto_opts_set( mosq, MOSQ_OPT_PROTOCOL_VERSION, &(version) );
    
    /* call back */
    mosquitto_connect_callback_set(mosq, my_connect_callback);
    mosquitto_disconnect_callback_set( mosq, my_disconnect_callback );
    mosquitto_message_callback_set(mosq, my_message_callback);
    
    /**/
    iret = mosquitto_connect_bind_async(mosq, ipdr, port, 60, NULL );
    if ( 0 != iret )
    {
        mosquitto_destroy( mosq );
        mosquitto_lib_cleanup();
        return 2;
    }


    /**/
    pctx->mosq = mosq;
    pctx->qctx = qctx;
    
    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}



