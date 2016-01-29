

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <alloca.h>

#include "uv.h"
#include "mpacket.h"
#include "mmqtt.h"

   
#define  STA_NOTHING            0
#define  STA_TCP_TRING          1
#define  STA_CONN_SEND          8
#define  STA_CONN_OK            10
#define  STA_DOWN_ING           11


extern  void  dump_hex( size_t len, uint8_t * ptr );


typedef struct _tag_mqtt_context
{
    /**/
    char * name;
	uv_loop_t * ploop;

    /* call back function */
    mqtt_conn_cbf  connfunc;
    intptr_t  connarg;
    
    mqtt_message_cbf  mesgfunc;
    intptr_t  mesgarg;
    
    /* 0: */
    uint16_t  tid;
    int  status;
    uv_timer_t  ttimer;
	uv_tcp_t  tcp;
	uv_connect_t  connect;
	uv_shutdown_t  shutdown;

    /**/
    uv_write_t  sendconn;
    uv_write_t  sendping;

	/* recv packet */
	int  rcvstep;
	intptr_t  rcvpkt;
	int  rcvlength;
	uint8_t  rcvheader;
	
} mqtt_context_t;

void  mqtt_tcp_shutdown_cb( uv_shutdown_t * req, int status )
{
	mqtt_context_t * pctx;

    /**/
    pctx = (mqtt_context_t *)(req->data);

    printf( "tcp shutdown cb, %d\n", status );

    /**/
    if ( NULL != pctx->connfunc )
    {
        pctx->connfunc( pctx->connarg, 0 );
    }
    
    /**/
    pctx->status = STA_NOTHING;
    return;
}


int  mqtt_shut_down( mqtt_context_t * pctx )
{
    uv_shutdown( &(pctx->shutdown), (uv_stream_t *)&(pctx->tcp), mqtt_tcp_shutdown_cb );
    pctx->shutdown.data = (void *)pctx;
    pctx->status = STA_DOWN_ING;
    return 0;
}


void  mqtt_send_subs_cb( uv_write_t * req, int status )
{
    intptr_t  mpkt;

    /**/
    printf( "mqtt_send_subs_cb, s = %d\n", status );

    /**/
    mpkt = (intptr_t)req->data;
    mpkt_free( mpkt );

    /**/
    free( req );
    return;
}


int  mqtt_send_subscribe( mqtt_context_t * pctx, char * topic )
{
    int  iret;
    uv_write_t * wreq;
    intptr_t  mpkt;
    uv_buf_t * pbufs;
    int  bfnum;
    int  temp;
    
    /**/
    temp = 2 + 2 + strlen(topic) + 1;
    iret = mpkt_alloc( MQTT_SUBSCRIBE | (1<<1), temp, &mpkt );
    if ( 0 != iret )
    {
        return 1;
    }

    /**/
    pctx->tid += 1;
    mpkt_subs_set_pkt_id( mpkt, pctx->tid );
    mpkt_subs_add_topic( mpkt, topic );
    
    /**/
    iret = mpkt_iovec( mpkt, &pbufs, &bfnum );
    if ( 0 != iret )
    {
        mpkt_free( mpkt );
        return 2;
    }

    wreq = (uv_write_t *)malloc( sizeof(uv_write_t) );
    if ( wreq == NULL )
    {
        mpkt_free( mpkt );
    }
    
    /**/
    printf( "\nheader: ");
    dump_hex( pbufs[0].len, (uint8_t *)pbufs[0].base );

    printf( "\npayload: ");
    dump_hex( pbufs[1].len, (uint8_t *)pbufs[1].base );

    /**/
    uv_write( wreq, (uv_stream_t*)&(pctx->tcp), pbufs, bfnum, mqtt_send_subs_cb );
    wreq->data = (void *)mpkt;
    return 0;
    
}



static void  mqtt_read_proc( mqtt_context_t * pctx )
{
    int  iret;
    uint8_t  temp;
    uint8_t * ptr;
    int  tlen;
    char * topic;
    
    /**/
    temp = (pctx->rcvheader & 0xF0);

    switch( temp )
    {
    case MQTT_CONNACK:
        /**/
        iret = mpkt_conack_get_retcode( pctx->rcvpkt, &temp );
        if ( 0 != iret )
        {
            /* fail to close */
            mqtt_shut_down( pctx );
            break;
        }

        /**/
        if ( temp != 0 )
        {
            /* fail to close */
            mqtt_shut_down( pctx );
            break;
        }

        /**/
        printf( "recv conn_ack ok\n" );
        pctx->status = STA_CONN_OK;
        uv_timer_set_repeat( &(pctx->ttimer), 20000 );

        /**/
        if ( NULL != pctx->connfunc )
        {
            pctx->connfunc( pctx->connarg, 0 );
        }
        
        /* send to subscribe fix topic */
        mqtt_send_subscribe( pctx, "/v1/to_gateway/#" );        
        mqtt_send_subscribe( pctx, "/v1/to_device/#" );
        break;

    case MQTT_SUBACK:
        printf( "recv subs_ack ok\n" );    
        break;

    case MQTT_PINGRESP:
        printf( "recv ping_ack ok\n" );    
        break;

    case MQTT_PUBLISH:
        printf( "recv publish\n" );
        iret = mpkt_publ_get_topic( pctx->rcvpkt, &ptr, &tlen );
        if ( 0 != iret )
        {
            break;
        }
        
        topic = alloca( tlen + 1 );
        memcpy( topic, ptr, tlen );
        topic[tlen] = 0;

        iret = mpkt_publ_get_message( pctx->rcvpkt, &ptr, &tlen );
        if ( 0 != iret )
        {
            break;
        }
        
        if ( NULL != pctx->mesgfunc )
        {
            pctx->mesgfunc( pctx->mesgarg, topic, tlen, ptr );
        }
        
        break;
        
    default:
        break;
        
    }

    
    return;
    
}


static inline void  mqtt_read_input( mqtt_context_t * pctx, uint8_t data )
{
    int  iret;
    
    /**/
    if ( pctx->rcvstep < 0 )
    {
        /**/
        mpkt_input( pctx->rcvpkt, data );
        pctx->rcvstep += 1;

        /**/
        if ( pctx->rcvstep == 0 )
        {
            /* call back. */
            mqtt_read_proc( pctx );
            //printf( "recv packet end\n" );
            mpkt_free( pctx->rcvpkt );
        }
    }
    else
    {
        /**/
        if ( pctx->rcvstep == 0 )
        {
            pctx->rcvheader = data;
            pctx->rcvlength = 0;
            pctx->rcvstep = 1;
        }
        else
        {
            if ( 0 != (data & 0x80) )
            {
                if ( pctx->rcvstep >= 4 )
                {
                    /* fail, close  */
                    mqtt_shut_down( pctx );
                    //printf( "fail, recv size too long\n" );
                    return;
                }
                
                data = data & 0x7F;
                pctx->rcvlength += (int)data << (7 * (pctx->rcvstep -1));
            }
            else            
            {
                pctx->rcvlength += (int)data << (7 * (pctx->rcvstep -1));
                
                /**/
                iret = mpkt_alloc( pctx->rcvheader, pctx->rcvlength, &(pctx->rcvpkt) );
                if ( 0 != iret )
                {
                    /* fail, close  */
                    mqtt_shut_down( pctx );
                    //printf( "fail, recv alloc packet fail\n" );
                    return;
                }

                if ( pctx->rcvlength == 0 )
                {
                    mqtt_read_proc( pctx );
                    //printf( "recv zero len packet end\n" );
                    mpkt_free( pctx->rcvpkt );
                    pctx->rcvstep = 0;
                }
                else
                {
                    /**/
                    pctx->rcvstep = 0 - pctx->rcvlength;
                }
            }
            
        }
        
    }
    
    return;
    
}


void  mqtt_read_cb( uv_stream_t * stream, ssize_t nread, const uv_buf_t* buf )
{
    int  i;
    mqtt_context_t * pctx;

    /**/
    pctx = (mqtt_context_t *)(stream->data);

    /**/
    if ( nread < 0 )
    {
        printf( "read cbk, nread = %zd\n", nread );
        mqtt_shut_down( pctx );
        return;
    }

    if ( nread == 0 )
    {
        return;
    }

#if 0
    /**/
    dump_hex( nread, (uint8_t *)buf->base );
#endif

    /**/
    for ( i=0; i<nread; i++ )
    {
        mqtt_read_input( pctx, (uint8_t)(buf->base[i]) );
    }
    
    /**/
    free( buf->base );
    return;
    
}



void  mqtt_read_alloc_cb( uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf )
{
    char * ptr;

    /**/
    // printf( "suggested_size %zu\n", suggested_size );
    ptr = (char *)malloc( 4096 );
    if ( NULL == ptr )
    {
        buf->base = NULL;
        buf->len = 0;
        return;
    }

    /**/
    buf->base = ptr;
    buf->len = 4096;
    return;
}



void  mqtt_send_publ_cb( uv_write_t * req, int status )
{
    intptr_t  mpkt;

    /**/
    mpkt = (intptr_t)req->data;
    mpkt_free( mpkt );

    /**/
    free( req );
    return;
}


int  mqtt_send_publish( mqtt_context_t * pctx, char * topic, char * mesge )
{
    int  iret;
    uv_write_t * wreq;
    intptr_t  mpkt;
    uv_buf_t * pbufs;
    int  bfnum;
    int  temp;
    
    /**/
    temp = 2 + strlen(topic) + strlen(mesge);
    iret = mpkt_alloc( MQTT_PUBLISH, temp, &mpkt );
    if ( 0 != iret )
    {
        return 1;
    }

    /**/
    mpkt_publ_set_topic( mpkt, topic );
    iret = mpkt_publ_set_message( mpkt, mesge );
    printf( "set message ret = %d\n", iret );
    
    /**/
    iret = mpkt_iovec( mpkt, &pbufs, &bfnum );
    if ( 0 != iret )
    {
        mpkt_free( mpkt );
        return 2;
    }

    wreq = (uv_write_t *)malloc( sizeof(uv_write_t) );
    if ( wreq == NULL )
    {
        mpkt_free( mpkt );
    }

#if 0    
    /**/
    printf( "\n, header: ");
    dump_hex( pbufs[0].len, (uint8_t *)pbufs[0].base );

    printf( "\n, payload: ");
    dump_hex( pbufs[1].len, (uint8_t *)pbufs[1].base );
#endif

    /**/
    uv_write( wreq, (uv_stream_t*)&(pctx->tcp), pbufs, bfnum, mqtt_send_publ_cb );
    wreq->data = (void *)mpkt;
    return 0;
    
}


void  mqtt_send_conn_cb( uv_write_t * req, int status )
{
    intptr_t  mpkt;

    /**/
    printf( "mqtt_send_conn_cb, s = %d\n", status );

    /**/
    mpkt = (intptr_t)req->data;
    mpkt_free( mpkt );
    return;
}


int  mqtt_send_connect( mqtt_context_t * pctx )
{
    int  iret;
    intptr_t  mpkt;
    uv_buf_t * pbufs;
    int  bfnum;
    
    /**/
    iret = mpkt_conn_alloc( 40, &mpkt );
    if ( 0 != iret )
    {
        return 1;
    }
    
    /**/
    mpkt_conn_set_protocol_level( mpkt, 4 );
    mpkt_conn_set_clean_session( mpkt, 1 );
    mpkt_conn_set_client_ident( mpkt, pctx->name );

printf( "connect.client.ident: %s\n", pctx->name );

    /**/
    iret = mpkt_iovec( mpkt, &pbufs, &bfnum );
    if ( 0 != iret )
    {
        mpkt_free( mpkt );
        return 2;
    }

#if 0
    /**/
    printf( "\n, header: ");
    dump_hex( pbufs[0].len, (uint8_t *)pbufs[0].base );

    printf( "\n, payload: ");
    dump_hex( pbufs[1].len, (uint8_t *)pbufs[1].base );
#endif

    /**/
    uv_write( &(pctx->sendconn), (uv_stream_t*)&(pctx->tcp), pbufs, bfnum, mqtt_send_conn_cb );
    pctx->sendconn.data = (void *)mpkt;
    pctx->status = STA_CONN_SEND;
    return 0;
    
}





void  mqtt_tcp_connect_cb( uv_connect_t * req, int status )
{
    int  iret;
	mqtt_context_t * pctx;

    /**/
    pctx = (mqtt_context_t *)(req->data);

    printf( "connect cb, status = %d\n", status );
    if ( status != 0 )
    {
        pctx->status = STA_NOTHING;
        return;
    }

    /**/
    iret = mqtt_send_connect( pctx );
    if ( 0 != iret )
    {
        mqtt_shut_down( pctx );
        return;
    }

    /**/
    uv_read_start( (uv_stream_t*)&(pctx->tcp), mqtt_read_alloc_cb, mqtt_read_cb );
    return;
    
}


void  mqtt_send_ping_cb( uv_write_t * req, int status )
{
    intptr_t  mpkt;

    /**/
    //printf( "mqtt_send_ping_cb, s = %d\n", status );

    /**/
    mpkt = (intptr_t)req->data;
    mpkt_free( mpkt );
    return;
}


int  mqtt_send_ping( mqtt_context_t * pctx )
{
    int  iret;
    intptr_t  mpkt;
    uv_buf_t * pbufs;
    int  bfnum;
    
    /**/
    iret = mpkt_alloc( MQTT_PINGREQ, 0, &mpkt );
    if ( 0 != iret )
    {
        return 1;
    }
    
    /**/
    iret = mpkt_iovec( mpkt, &pbufs, &bfnum );
    if ( 0 != iret )
    {
        mpkt_free( mpkt );
        return 2;
    }

#if 0
    /**/
    printf( "\nPING header: ");
    dump_hex( pbufs[0].len, (uint8_t *)pbufs[0].base );
#endif

    /**/
    uv_write( &(pctx->sendping), (uv_stream_t*)&(pctx->tcp), pbufs, bfnum, mqtt_send_ping_cb );
    pctx->sendping.data = (void *)mpkt;
    return 0;
    
}



void  mqtt_timer_cb( uv_timer_t * handle )
{
    int  iret;
	mqtt_context_t * pctx;
    struct sockaddr_in  addr;

    /**/
    pctx = (mqtt_context_t *)(handle->data);

    // printf( "timer once. \n");
    
    /**/
    if ( pctx->status == STA_NOTHING )
    {
        printf( "try connect \n");

        /**/
        uv_tcp_init( pctx->ploop, &(pctx->tcp) );
        pctx->tcp.data = (void *)pctx;
        
        /**/
        uv_ip4_addr( "123.57.206.2", 8020, &addr );
        iret = uv_tcp_connect( &(pctx->connect), &(pctx->tcp), (const struct sockaddr *)&addr, mqtt_tcp_connect_cb );
        if ( 0 != iret )
        {
            printf( "connect fail = %d\n", iret );
        }
        pctx->connect.data = (void *)pctx;
        pctx->status = STA_TCP_TRING;
    }

    /**/
    if ( pctx->status == STA_CONN_OK )
    {
        mqtt_send_ping( pctx );
    }
    
    return;
    
}



int  mqtt_start( intptr_t ctx )
{
	mqtt_context_t * pctx;

    /**/
    pctx = (mqtt_context_t *)ctx;
    
    /**/
    uv_timer_start( &(pctx->ttimer), mqtt_timer_cb, 2000, 2000 );
    pctx->ttimer.data = (void *)pctx;

    /**/
    return 0;
}


int  mqtt_set_conn_callbk( intptr_t ctx, mqtt_conn_cbf func, intptr_t arg )
{
	mqtt_context_t * pctx;

    /**/
    pctx = (mqtt_context_t *)ctx;

    /**/
    pctx->connfunc = func;
    pctx->connarg = arg;
    
    /**/
    return 0;
}


int  mqtt_set_mesage_callbk( intptr_t ctx, mqtt_message_cbf func, intptr_t arg )
{
	mqtt_context_t * pctx;

    /**/
    pctx = (mqtt_context_t *)ctx;

    /**/
    pctx = (mqtt_context_t *)ctx;

    /**/
    pctx->mesgfunc = func;
    pctx->mesgarg = arg;

    /**/
    return 0;
}


int  mqtt_init( char * name, uv_loop_t * ploop, intptr_t * pret )
{
	int  iret;
	mqtt_context_t * pctx;
    
	/**/
	pctx = (mqtt_context_t *)malloc( sizeof(mqtt_context_t) );
	if ( NULL == pctx )
	{
		return 1;
	}

	/**/
	pctx->name = strdup(name);
	pctx->tid = 11;
	pctx->status = STA_NOTHING;
    pctx->ploop	= ploop;
    
	/**/
	iret = uv_timer_init( pctx->ploop, &(pctx->ttimer) );
	if ( 0 != iret )
	{
	    return 5;
	}

    /**/
    pctx->rcvstep = 0;
    pctx->rcvpkt = 0;
    pctx->rcvlength = 0;
    
	/**/
	*pret = (intptr_t)pctx;
	return 0;
	
}



int  mqtt_publish( intptr_t ctx, char * topic, char * mesge )
{
	mqtt_context_t * pctx;

    /**/
    pctx = (mqtt_context_t *)ctx;

    /**/
    if ( pctx->status != STA_CONN_OK )
    {
        return 1;
    }

    /**/
    return mqtt_send_publish( pctx, topic, mesge );
}



