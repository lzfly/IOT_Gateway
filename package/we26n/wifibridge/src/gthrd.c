#include <stdio.h>
#define __USE_XOPEN
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include <sys/socket.h>
#include <alloca.h>
#include <syslog.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "pktintf.h"
#include "dgramsck.h"
#include "gthrd.h"


#define  WAKE_REQ_LEN   22
#define  READ_REQ_LEN   16
#define  SLEP_REQ_LEN   21

#define  WAKE_RSP_LEN   18
#define  READ_RSP_LEN   41


void  dump_hex( const unsigned char * ptr, size_t  len )
{
    int  i;
    int  nn;
    int  len2 = len;

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


int  util_send_to_uloop( int sock, double data )
{
    /**/
    send( sock, &data, sizeof(data), 0 );
    return 0;
}


uint8_t  util_calc_sum( uint8_t * tbuf, int tlen )
{
    int i;
    uint8_t  temp;

    /**/
    temp = 0;
    for( i = 0; i < tlen; i++ )
    {
        temp = temp + tbuf[i];
    }
    
    return temp;
}



int  util_patch_read_buf( uint8_t * rdbuf, uint64_t mid )
{

    rdbuf[4] = (uint8_t)(mid & 0xff);
    rdbuf[5] = (uint8_t)(mid >>8 & 0xff);
    rdbuf[6] = (uint8_t)(mid >>16 & 0xff);
    rdbuf[7] = (uint8_t)(mid >>24 & 0xff);
    
    rdbuf[15] = util_calc_sum( &(rdbuf[1]), 14 );

    printf( "CRC = %x\n", rdbuf[15] );
    return 0;
}


int  util_check_wake_resp( uint8_t * resp, int tlen )
{
    return 0;
}


int  util_check_read_resp( uint8_t * resp, int tlen, double * pval )
{
    uint8_t  temp;
    uint32_t  uval;
    double  data;
    
    /**/
    temp = util_calc_sum( &(resp[1]), tlen-2 );
    if ( temp != resp[tlen-1] )
    {
        syslog( LOG_CRIT,"check read, crc error" );
        return 1;
    }

    /**/
    uval = resp[18];
    uval = resp[17] | (uval << 8);
    uval = resp[16] | (uval << 8);
    uval = resp[15] | (uval << 8);
    
    /**/
    data = (double)uval;
    data = data / 10.0;

    /**/
    *pval = data;    
    return 0;
    
}




struct _tag_gthrd_context;
typedef struct _tag_gthrd_context  gthrd_context_t;


/* tlen is 0 to timeout callback. */
typedef void (* task_cbfunc)( gthrd_context_t * pctx, int tlen, uint8_t * pdat );


/**/
struct _tag_gthrd_context
{
    struct event_base * pevbase;
    struct bufferevent * pevbuf;
    
    /* socket pair */
    int  sv[2];
    intptr_t  dgctx;
    pkt_intf_t * pintf;
    struct event * prdevt;

    /* task */
    task_cbfunc  tfunc;
    struct event * pevtmr;
    struct evbuffer * pbuf;

    /**/
    uint8_t buf_wake[32];
    uint8_t buf_read[32];
    uint8_t buf_slep[32];
    
};


typedef struct _tag_notify_msg
{
    int  type;
    union 
    {
        int  sock;
        uint64_t  mid;
    } value;
    
} notify_msg_t;




int  gth_task_init( gthrd_context_t * pctx, task_cbfunc func )
{
    /**/
    evtimer_add( pctx->pevtmr, NULL );    
	pctx->tfunc = func;
	return 0;
}


int  gth_task_fini( gthrd_context_t * pctx )
{
	/**/
	if ( NULL == pctx->tfunc )
	{
	    syslog( LOG_CRIT, "task fini, but func is null" );
	    return 1;
	}

    /**/
    evtimer_add( pctx->pevtmr, NULL );
    pctx->tfunc = NULL;
	return 0;
}


int  gth_task_active( gthrd_context_t * pctx, int tlen, uint8_t * pdat, uint32_t tms )
{
    struct timeval  tv;
    
	/**/
	if ( NULL == pctx->tfunc )
	{
	    syslog( LOG_CRIT, "task active, but func is null" );
	    return 1;
	}
    
    /**/
    tv.tv_sec = tms / 1000;
    tv.tv_usec = (tms % 1000) * 1000;
	
	/**/
	evtimer_add( pctx->pevtmr, &tv );

    /**/
	if ( tlen > 0 )
	{
        bufferevent_write( pctx->pevbuf, pdat, tlen );
    }
    
	return 0;
	
}


void  gthrd_inter_timer_cbk( evutil_socket_t fd, short events, void * arg )
{
    gthrd_context_t * pctx;

    /**/
    pctx = (gthrd_context_t *)arg;
    
    /**/
	if ( NULL == pctx->tfunc )
	{
	    syslog( LOG_CRIT, "task timer, but func is null" );
	    return;
	}
    
    /**/
    pctx->tfunc( pctx, 0, NULL );
    return;
}


void  gthrd_inter_data_cbk( struct bufferevent * bev, void * arg )
{
    gthrd_context_t * pctx;
    uint8_t  tbuf[64];
    int  tsize;
    
    /**/
    pctx = (gthrd_context_t *)arg;

    /**/
    while ( 1 )
    {
        tsize = (int)bufferevent_read( pctx->pevbuf, tbuf, 64 );
        if ( tsize <= 0 )
        {
            break;
        }

        syslog( LOG_CRIT, "data cbk, %d \n", tsize );

        /**/
        if ( NULL != pctx->tfunc )
        {
            pctx->tfunc( pctx, tsize, tbuf );
        }
    }
    
    /**/
    return;
    
}


void  task_wait_sleep( gthrd_context_t * pctx, int tlen, uint8_t * pdat )
{
    /**/
    if ( tlen == 0 )
    {
        gth_task_fini( pctx );
        syslog( LOG_CRIT, "sleep, end" );
        return;
    }

    return;
}


void  task_wait_read( gthrd_context_t * pctx, int tlen, uint8_t * pdat )
{
    int  iret;
    uint8_t * resp;
    double  value;
    
    /**/
    if ( tlen == 0 )
    {
        gth_task_fini( pctx );
        syslog( LOG_CRIT, "read data, time out" );
        return;
    }

    /**/
    evbuffer_add( pctx->pbuf, pdat, tlen );
    iret = evbuffer_get_length( pctx->pbuf );
    if ( iret < READ_RSP_LEN )
    {
        return;
    }

    /* check wake resp */
    resp = evbuffer_pullup( pctx->pbuf, READ_RSP_LEN );
    iret = util_check_read_resp( resp, READ_RSP_LEN, &value );
    if ( 0 != iret )
    {
        gth_task_fini( pctx );
        syslog( LOG_CRIT, "read data, check resp, fail" );
        return;
    }

    /* send to uloop thread */
    util_send_to_uloop( pctx->sv[0], value );
    syslog( LOG_CRIT, "read data, value = %f", value );    
    
    /**/
    iret = evbuffer_get_length( pctx->pbuf );
    evbuffer_drain( pctx->pbuf, iret + 1 );
    gth_task_init( pctx, task_wait_sleep );
    gth_task_active( pctx, READ_REQ_LEN, pctx->buf_slep, 1000 );
    return;
    
}


void  task_wait_wait5( gthrd_context_t * pctx, int tlen, uint8_t * pdat )
{
    int  iret;
    
    if ( tlen == 0 )
    {
        iret = evbuffer_get_length( pctx->pbuf );
        evbuffer_drain( pctx->pbuf, iret + 1 );    
        gth_task_init( pctx, task_wait_read );
        gth_task_active( pctx, READ_REQ_LEN, pctx->buf_read, 5000 );
    }
}


/**/
void  task_wait_wakeup( gthrd_context_t * pctx, int tlen, uint8_t * pdat )
{
    int  iret;
    uint8_t * resp;
    
    /**/
    if ( tlen == 0 )
    {
        gth_task_fini( pctx );
        syslog( LOG_CRIT, "wakeup, time out" );

#if 0
    /* send to uloop thread */
    util_send_to_uloop( pctx->sv[0], 222.222 );
#endif
        return;
    }

    /**/
    evbuffer_add( pctx->pbuf, pdat, tlen );
    iret = evbuffer_get_length( pctx->pbuf );
    if ( iret < WAKE_RSP_LEN )
    {
        return;
    }

    /* check wake resp */
    resp = evbuffer_pullup( pctx->pbuf, WAKE_RSP_LEN );
    iret = util_check_wake_resp( resp, WAKE_RSP_LEN );
    if ( 0 != iret )
    {
        gth_task_fini( pctx );
        syslog( LOG_CRIT, "wakeup, check resp, fail" );
        return;
    }

    /**/
    syslog( LOG_CRIT, "wakeup, success" );

    /**/
    iret = evbuffer_get_length( pctx->pbuf );
    evbuffer_drain( pctx->pbuf, iret + 1 );
    gth_task_init( pctx, task_wait_wait5 );
    gth_task_active( pctx, 0, NULL, 5000 );
    return;
    
}




int  gthrd_inter_spawn_task( gthrd_context_t * pctx )
{
	/**/
	if ( NULL != pctx->tfunc )
	{
	    syslog( LOG_CRIT, "spawn task, but func is not null" );
	    return 1;
	}
    
    /**/
    evbuffer_drain( pctx->pbuf, 999999 );    
    gth_task_init( pctx, task_wait_wakeup );
    gth_task_active( pctx, WAKE_REQ_LEN, pctx->buf_wake, 6000 );
    return 0;
}


int  gthrd_inter_delete_evbuf( gthrd_context_t * pctx )
{
    int  sock;

    /**/
    sock = (int)bufferevent_getfd( pctx->pevbuf );
    bufferevent_free( pctx->pevbuf );
    pctx->pevbuf = NULL;

    /**/
    close( sock );
    return 0;
}




/*
#define 	BEV_EVENT_READING   0x01
 	error encountered while reading 
#define 	BEV_EVENT_WRITING   0x02
 	error encountered while writing 
#define 	BEV_EVENT_EOF   0x10
 	eof file reached 
#define 	BEV_EVENT_ERROR   0x20
 	unrecoverable error encountered 
#define 	BEV_EVENT_TIMEOUT   0x40
 	user-specified timeout reached 
#define 	BEV_EVENT_CONNECTED   0x80
 	connect operation finished. 
*/

void  gthrd_inter_event_cbk(struct bufferevent * bev, short what, void * arg )
{
    gthrd_context_t * pctx;

    /**/
    pctx = (gthrd_context_t *)arg;

    /**/
    gth_task_fini( pctx );
    
    /**/
    gthrd_inter_delete_evbuf( pctx );
    syslog( LOG_CRIT, "event, what = %d, close sock", what );
    return;
}


int  gthrd_inter_create_evbuf( gthrd_context_t * pctx, int sock )
{
    struct bufferevent * pevbuf;

    pevbuf = bufferevent_socket_new( pctx->pevbase, (evutil_socket_t)sock, BEV_OPT_DEFER_CALLBACKS );
    if ( NULL )
    {
        return 1;
    }

    bufferevent_setcb( pevbuf, gthrd_inter_data_cbk, NULL, gthrd_inter_event_cbk, (void *)pctx );
    bufferevent_enable( pevbuf, EV_READ | EV_WRITE );
    
    /**/
    pctx->pevbuf = pevbuf;
    return 0;
}



void  gthrd_inter_dgram_evt( evutil_socket_t ufd, short event, void * parg )
{
    gthrd_context_t * pctx;

    /**/
    pctx = (gthrd_context_t *)parg;
    
    /**/
    pctx->pintf->pkti_try_run( pctx->dgctx );
    return;
}


void  gthrd_inter_dgram_cbk( intptr_t arg, int tlen, void * pdat )
{
    gthrd_context_t * pctx;
    notify_msg_t * pmsg;

    /**/
    pctx = (gthrd_context_t *)arg;
    pmsg = (notify_msg_t *)pdat;

    /**/
    switch ( pmsg->type )
    {
    case 0x11111111:
        util_patch_read_buf( pctx->buf_read, pmsg->value.mid );
        break;
        
    case 0x22222222:
        if ( NULL != pctx->pevbuf )
        {
            gth_task_fini( pctx );
            gthrd_inter_delete_evbuf( pctx );
        }

        /**/
        gthrd_inter_create_evbuf( pctx, pmsg->value.sock );
        gthrd_inter_spawn_task( pctx );
        break;

    case 0x33333333:
        if ( NULL == pctx->pevbuf )
        {
            break;
        }
        
        syslog( LOG_CRIT, "gas_meter try start " );
        gthrd_inter_spawn_task( pctx );
        break;
        
    default:
        break;
    }
    
    /**/
#if 0
    dump_hex( pdat, tlen );
#endif

    return;
}


void * gthrd_inter_thread( void * arg )
{
    gthrd_context_t * pctx;

    /**/
    pctx = (gthrd_context_t *)arg;

    /**/
    event_base_dispatch( pctx->pevbase );
    return 0;
}


int  gthrd_init( intptr_t * pret )
{
    int  iret;
    gthrd_context_t * pctx;

    char buf_wake[WAKE_REQ_LEN] = {0X3C,0X00,0X01,0X00,0X00,0X00,0X00,0X00,0XF9,0X00,0X08,0X08,0X01,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X85,0X4F};
    char buf_read[READ_REQ_LEN] = {0x3C,0X00,0X01,0X00,0XDE,0XAA,0X03,0X18,0X22,0X00,0X08,0X03,0XEA,0XAE,0X01,0X6A};
    char buf_slep[SLEP_REQ_LEN]= {0X3C,0X00,0X01,0X00,0X00,0X00,0X00,0X00,0XFD,0X00,0X08,0X08,0X00,0X00,0X00,0X00,0X05,0X00,0X00,0X00,0X13};
 
    
    /**/
    pctx = (gthrd_context_t *)malloc( sizeof(gthrd_context_t) );
    if ( NULL == pctx )
    {
        return 1;
    }

    /**/
    pctx->pevbuf = NULL;
    pctx->tfunc = NULL;
    memcpy( pctx->buf_wake, buf_wake, WAKE_REQ_LEN );
    memcpy( pctx->buf_read, buf_read, READ_REQ_LEN );
    memcpy( pctx->buf_slep, buf_slep, SLEP_REQ_LEN );
    
    /**/
    pctx->pevbase = event_base_new();
    if ( NULL == pctx->pevbase )
    {
        return 2;
    }

    /**/
    pctx->pevtmr = evtimer_new( pctx->pevbase, gthrd_inter_timer_cbk, (void *)pctx );
    if ( NULL == pctx->pevtmr )
    {
        return 3;
    }

    pctx->pbuf = evbuffer_new();
    if ( NULL == pctx->pbuf )
    {
        return 4;
    }
    
    /**/
    iret = socketpair( AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK, 0, pctx->sv );
    if ( 0 != iret )
    {
        return 5;
    }
    
    /**/
    iret = dgram_sock_init( pctx->sv[0], &(pctx->dgctx), &(pctx->pintf) );
    if ( 0 != iret )
    {
        return 6;
    }

    /**/
    pctx->pintf->pkti_set_callbk( pctx->dgctx, gthrd_inter_dgram_cbk, (intptr_t)pctx );

    /**/
    pctx->prdevt = event_new( pctx->pevbase, pctx->sv[0], EV_READ|EV_PERSIST, gthrd_inter_dgram_evt, (void *)pctx );
    if ( pctx->prdevt == NULL )
    {
        return 7;
    }
    
    /**/
    event_add( pctx->prdevt, NULL );

    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}


int  gthrd_getfd( intptr_t ctx, int * pfd )
{
    gthrd_context_t * pctx;
    
    /**/
    pctx = (gthrd_context_t *)ctx;

    /**/
    *pfd = pctx->sv[1];
    return 0;
}


int  gthrd_start( intptr_t ctx )
{
    int  iret;
    pthread_t  pid;
    
    /**/
    iret = pthread_create( &pid, NULL, gthrd_inter_thread, (void *)ctx );
    if( iret != 0 )
    {
        return 1;
    }
    
    return 0;
}


/* notify meter id change. */
int  gthrd_notify_mid( intptr_t ctx, uint64_t mid )
{
    gthrd_context_t * pctx;
    notify_msg_t  msg;
    
    /**/
    pctx = (gthrd_context_t *)ctx;

    /**/
    msg.type = 0x11111111;
    msg.value.mid = mid;

    /**/
    send( pctx->sv[1], &msg, sizeof(msg), 0 );
    return 0;
}



int  gthrd_notify_sock( intptr_t ctx, int sock )
{
    gthrd_context_t * pctx;
    notify_msg_t  msg;
    
    /**/
    pctx = (gthrd_context_t *)ctx;

    /**/
    msg.type = 0x22222222;
    msg.value.sock = sock;

    
    /**/
    send( pctx->sv[1], &msg, sizeof(msg), 0 );
    return 0;

}



/* notify meter ever time change. */
int  gthrd_notify_ever( intptr_t ctx )
{
    gthrd_context_t * pctx;
    notify_msg_t  msg;
    
    /**/
    pctx = (gthrd_context_t *)ctx;
    
    /**/
    msg.type = 0x33333333;
    
    /**/
    send( pctx->sv[1], &msg, sizeof(msg), 0 );
    return 0;
}


