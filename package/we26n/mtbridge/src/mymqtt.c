
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>
#include <unistd.h>
#include <syslog.h>


#include <mosquitto.h>
#include "myqueue.h"
#include "mymqtt.h"


typedef struct _tag_mmqt_context
{
    struct mosquitto * mosq;
    intptr_t qctx;
    char  topic[20];

    /**/
    char  user[40];
    
} mmqt_context_t;



int  recongnize_dir( char * path, int max, char * ptr, char ** ppnext )
{
    int  i;
    
    if ( path[0] != '/' )
    {
        return 1;
    }

    /**/
    for ( i=1;; i++ )
    {
        if ( i >= max )
        {
            return 2;
        }
        
        if ( (path[i] == '/') || path[i] == '\0' )
        {
            break;
        }

        ptr[i-1] = path[i];
    }

    /**/
    ptr[i-1] = 0;
    *ppnext = &(path[i]);
    return 0;
    
}


void  my_message_inner_togateway( mmqt_context_t * pctx, char * pnext, const struct mosquitto_message * message )
{
    int  iret;
    mmqt_msg_t  tmsg;
    char  tstr[50];

    /**/
    tmsg.target = TARGET_TOGTW;
    
    /**/
    iret = recongnize_dir( pnext, 50, tstr, &pnext );
    if ( 0 != iret )
    {
        /*  */
        printf( "recongnize 3rd gtwid fail = %d\n", iret );
        return;
    }

    if ( 0 != strcmp( pctx->topic, tstr ) )
    {
        return;
    }

    /**/
    if ( message->payloadlen > GTW_MSG_MAX_LEN )
    {
        printf( "gateway msg too long\n" );
        return;
    }

    memcpy( tmsg.payload.togtw.msge, message->payload, message->payloadlen );
    tmsg.payload.togtw.msge[message->payloadlen] = '\0';

    /**/
    msq_enqueue( pctx->qctx, sizeof(tmsg), &tmsg );
    write( ((int *)pctx->mosq)[1], tstr, 1 );
    return;
    
}


void  my_message_inner_todevice( mmqt_context_t * pctx, char * pnext, const struct mosquitto_message * message )
{
    int  iret;
    mmqt_msg_t  tmsg;
    char  tstr[50];

    /**/
    tmsg.target = TARGET_TODEV;
    
    /**/
    iret = recongnize_dir( pnext, 50, tstr, &pnext );
    if ( 0 != iret )
    {
        /*  */
        printf( "recongnize 3rd devsn fail = %d\n", iret );
        return;
    }

    if ( strlen(tstr) > DEVID_MAX_LEN )
    {
        printf( "devsn too long\n" );        
        return;
    }

    /**/
    strcpy( tmsg.payload.todev.devid, tstr );

    /**/
    iret = recongnize_dir( pnext, 50, tstr, &pnext );
    if ( 0 != iret )
    {
        /*  */
        printf( "recongnize 4th attid fail = %d\n", iret );
        return;
    }

    if ( *pnext != '\0' )
    {
        /* */
        printf( "attid not tail\n" );
        return;
    }

    if ( strlen(tstr) > ATTID_MAX_LEN )
    {
        printf( "attid too long\n" );
        return;
    }

    /**/
    strcpy( tmsg.payload.todev.attid, tstr );

    /**/
    if ( message->payloadlen > VALUE_MAX_LEN )
    {
        printf( "value too long\n" );
        return;
    }

    memcpy( tmsg.payload.todev.value, message->payload, message->payloadlen );
    tmsg.payload.todev.value[message->payloadlen] = '\0';

    /**/
    msq_enqueue( pctx->qctx, sizeof(tmsg), &tmsg );
    write( ((int *)pctx->mosq)[1], tstr, 1 );
    return;
    
}


void  my_message_callback(struct mosquitto * mosq, void * obj, const struct mosquitto_message * message )
{
    int  iret;
    mmqt_context_t * pctx;
    char  tstr[50];
    char * pnext;
    
    /**/
    pctx = (mmqt_context_t *)obj;
    
    /**/
  	if ( message->payloadlen <= 0 )
  	{
      	return;
  	}
    
    /**/
    iret = recongnize_dir( message->topic, 50, tstr, &pnext );
    if ( 0 != iret )
    {
        /*  */
        printf( "recongnize 1st path fail = %d\n", iret );
        return;
    }
    
    if ( 0 != strcmp( "v1", tstr ) )
    {
        printf( "recg, 1st not v1\n" );
        return;
    }

    /**/
    iret = recongnize_dir( pnext, 50, tstr, &pnext );
    if ( 0 != iret )
    {
        /*  */
        printf( "recongnize 2nd path fail = %d\n", iret );
        return;
    }

    /**/
    if ( 0 == strcmp( "to_device", tstr ) )
    {
        my_message_inner_todevice( pctx, pnext, message );    
    }
    else if ( 0 == strcmp( "to_gateway", tstr ) )
    {
        my_message_inner_togateway( pctx, pnext, message );
    }
    

    return;
  	
}



void  my_connect_callback(struct mosquitto * mosq, void * obj, int result )
{


    /**/
	if( 0 == result )
	{
		mosquitto_subscribe( mosq, NULL, "/v1/to_gateway/#", 0 );
		mosquitto_subscribe( mosq, NULL, "/v1/to_device/#", 0 );
	}
	else
	{
	    fprintf(stderr, "%s\n", mosquitto_connack_string(result));
	}

#if 0
    mmqt_context_t * pctx;

    /**/
    pctx = (mmqt_context_t *)obj;

    mmqt_msg_t  tmsg;

    /**/
    tmsg.action = MT_ACT_NTC;
    tmsg.object = MT_OBJ_SYS;
    strcpy( tmsg.msg, "CON" );

    /**/
    msq_enqueue( pctx->qctx, sizeof(mmqt_msg_t), &tmsg );
    write( ((int *)pctx->mosq)[1], &tmsg, 1 );    
#endif

    /**/
    printf( "connect ok, %d\n", result );
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


int  mmqt_set_user( intptr_t ctx, char * user )
{
    mmqt_context_t * pctx;
    
    /**/
    pctx = (mmqt_context_t *)ctx;

    /**/
    strcpy( pctx->user, user );
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


/* publish to user */
int  mmqt_notice( intptr_t ctx, char * mod, char * msg )
{
    int  iret;
    int  tlen;    
    mmqt_context_t * pctx;
    char * ptr;
    
    /**/
    pctx = (mmqt_context_t *)ctx;

    /**/
    tlen = strlen(mod) + strlen(msg) + 10;
    ptr = (char *)alloca( tlen );
    if ( NULL == ptr )
    {
        return 1;
    }

    /**/
    tlen = sprintf( ptr, "C:N|M:%s|%s", mod, msg );
    
    /* qos = 0, retain = 0. */
    iret = mosquitto_publish( pctx->mosq, NULL, pctx->user, tlen, ptr, 0, 0 );
    if ( 0 != iret )
    {
        return 2;
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
    fout = popen( "eth_mac r wifi", "r" );
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
        syslog( LOG_CRIT, "init, malloc, fail" );
        return 1;
    }

    /**/
    sprintf( pctx->topic, "we26n_" );
    iret = mmqt_get_macaddr( &(pctx->topic[6]) );
    if ( 0 != iret )
    {
        syslog( LOG_CRIT, "init, getmac, ret %d", iret );
        return 2;
    }

    sprintf( pctx->user, "%s_user", pctx->topic );

    printf( "inner mmqt, topic, %s\n", pctx->topic );
    printf( "inner mmqt, user, %s\n", pctx->user );
    
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

retry:    
    /**/
    iret = mosquitto_connect_bind_async(mosq, ipdr, port, 60, NULL );
    if ( 0 != iret )
    {
        if ( iret == MOSQ_ERR_ERRNO )
        {
            printf( "retry connect\n" );
            sleep(1);
            goto retry;
        }
        else
        {
            syslog( LOG_CRIT, "init, bind_async, ret %d", iret );
            mosquitto_destroy( mosq );
            mosquitto_lib_cleanup();
            return 3;
        }
    }


    /**/
    pctx->mosq = mosq;
    pctx->qctx = qctx;
    
    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}



