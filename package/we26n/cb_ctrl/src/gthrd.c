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

#include "myqueue.h"
#include "gthrd.h"
#include "type.h"


#define  SET_REQ_LEN   7
#define  READ_REQ_LEN   6
#define  SLEP_REQ_LEN   21
#define  SET_TIM_LEN   8
#define  GET_CTRL_STAT 6
#define SET_REM_LEN 7
#define SET_CTR_MOD 7

#define  WAKE_RSP_LEN   18
#define  READ_RSP_LEN   13
#define  SET_RSP_LEN 14
#define  SET_TIM_RSP 14

char cb_id[200];

int  temp_write_to_ini( char * path, uint8_t data ,char *name)
{
    char devicesstr[2048];
    FILE *fp;
    
    devicesstr[0] = 0;
	//get_cb_id();
	printf("cb id =%s\n",cb_id);
    sprintf(&devicesstr[0], "[");
    sprintf(&devicesstr[strlen(devicesstr)], "{");
    sprintf(&devicesstr[strlen(devicesstr)], "\"%s\":\"%d\",",name,data);
	if( 0 == strcmp( name, "Remote") )
	{
		sprintf(&devicesstr[strlen(devicesstr)], "\"deviceid\":\"CB_controller_%s\",",cb_id);
		sprintf(&devicesstr[strlen(devicesstr)], "\"devicetype\":\"%s\",",CB_TYPE);
		sprintf(&devicesstr[strlen(devicesstr)], "\"status\":\"8\"");
	}
	
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
    
    printf("write to ini file ok\n");
    syslog(LOG_CRIT,"write to ini file ok");
    return 0;
    
}


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




int send_tem_to_uloop(int sock,  tem_t tem)
{
	send(sock, &tem, sizeof(tem), 0);
	return 0;
}



int  util_check_readtem_resp( uint8_t * resp, int tlen )
{

    /**/
    if ( resp[4] != 0xEE )
    {
        syslog( LOG_CRIT,"check wake, head byte error" );
        return 1;
    }
    
    return 0;
    
}

int util_check_settem_resp(uint8_t * resp, int tlen)
{
    if(resp[5] != 0xEE)
    {
        syslog( LOG_CRIT,"check set, tail byte error" );
        return 1;
    }
    
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
    int  dbg_close_count;
    
    /* socket pair */
    int  sv[2];
    intptr_t  mqctx;
    struct event * prdevt;

    /* task */
    task_cbfunc  tfunc;
    struct event * pevtmr;
    struct evbuffer * pbuf;

    /**/
    uint32_t  target_tem;
	uint32_t  rem_tem;
	uint32_t  ctrl_mode;
    
};


typedef struct _tag_notify_msg
{
    int  type;
	uint32_t min;
	uint32_t hour;
    union 
    {
        int  sock;
        uint32_t  target_tem;
        uint32_t  rem_tem;
        uint32_t  ctrl_mode;
        
    } value;
    
} notify_msg_t;


void  gthrd_inter_check_new( gthrd_context_t * pctx );

int  gth_task_init( gthrd_context_t * pctx, task_cbfunc func )
{
    /**/
	printf("gth_task_init\n");
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

    /**/
    gthrd_inter_check_new( pctx );
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


void  gth_task_timer_cbk( evutil_socket_t fd, short events, void * arg )
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


void  gth_task_data_cbk( struct bufferevent * bev, void * arg )
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
		dump_hex( tbuf, tsize );

        /**/
        if ( NULL != pctx->tfunc )
        {
            pctx->tfunc( pctx, tsize, tbuf );
        }
    }
    
    /**/
    return;
    
}



void  task_set_tem( gthrd_context_t * pctx, int tlen, uint8_t * pdat )
{
    int  iret;
    uint8_t * resp;
   
    /**/
    if ( tlen == 0 )
    {
        gth_task_fini( pctx );
        syslog( LOG_CRIT, "wakeup, time out" );
        return;
    } 

    /**/
    evbuffer_add( pctx->pbuf, pdat, tlen );
    iret = evbuffer_get_length( pctx->pbuf );
    if ( iret < SET_RSP_LEN )
    {
        return;
    }

    /* check wake resp */
    resp = evbuffer_pullup( pctx->pbuf, SET_RSP_LEN );
  
  
   iret = util_check_settem_resp( resp, SET_RSP_LEN );
    if ( 0 != iret )
    {
       gth_task_fini( pctx );
        syslog( LOG_CRIT, "set tem fail" );
        return;
    }
	
	
	
    syslog( LOG_CRIT, "set tem, success" );
	printf("set tem\n");
    gth_task_fini( pctx );
    return;
    
}



void  task_set_ctrl_mode( gthrd_context_t * pctx, int tlen, uint8_t * pdat )
{
    int  iret;
    uint8_t * resp;
    
   
    /**/
    if ( tlen == 0 )
    {
        gth_task_fini( pctx );
        syslog( LOG_CRIT, "set  ctrl mode, time out" );
        return;
    } 

    /**/
    evbuffer_add( pctx->pbuf, pdat, tlen );
    iret = evbuffer_get_length( pctx->pbuf );
    if ( iret < SET_RSP_LEN )
    {
        return;
    }

    /* check wake resp */
    resp = evbuffer_pullup( pctx->pbuf, SET_RSP_LEN );
  
  
   iret = util_check_settem_resp( resp, SET_RSP_LEN );
    if ( 0 != iret )
    {
       gth_task_fini( pctx );
        syslog( LOG_CRIT, "set ctrl mode fail" );
        return;
    }
	
	
	
    syslog( LOG_CRIT, "set tem, success" );
	printf("set tem\n");
    gth_task_fini( pctx );
    return;
    
}


void  task_set_rem_tem( gthrd_context_t * pctx, int tlen, uint8_t * pdat )
{
    int  iret;
    uint8_t * resp;
   
    /**/
    if ( tlen == 0 )
    {
        gth_task_fini( pctx );
        syslog( LOG_CRIT, "set rem tem, time out" );
        return;
    } 

    /**/
    evbuffer_add( pctx->pbuf, pdat, tlen );
    iret = evbuffer_get_length( pctx->pbuf );
    if ( iret < SET_RSP_LEN )
    {
        return;
    }

    /* check wake resp */
    resp = evbuffer_pullup( pctx->pbuf, SET_RSP_LEN );
  
  
    iret = util_check_settem_resp( resp, SET_RSP_LEN );
    if ( 0 != iret )
    {
       gth_task_fini( pctx );
        syslog( LOG_CRIT, "set  rem  tem fail" );
        return;
    }
	
    syslog( LOG_CRIT, "set tem, success" );
	printf("set tem\n");
    gth_task_fini( pctx );
    return;
    
}


void  task_set_time( gthrd_context_t * pctx, int tlen, uint8_t * pdat )
{
    int  iret;
    uint8_t * resp;
   
    /**/
    if ( tlen == 0 )
    {
        gth_task_fini( pctx );
        syslog( LOG_CRIT, "set time, time out" );
        return;
    } 

    /**/
    evbuffer_add( pctx->pbuf, pdat, tlen );
    iret = evbuffer_get_length( pctx->pbuf );
    if ( iret < SET_TIM_RSP )
    {
        return;
    }
    /* check wake resp */
    resp = evbuffer_pullup( pctx->pbuf, SET_TIM_RSP );
  
   iret = util_check_settem_resp( resp, SET_TIM_RSP );
    if ( 0 != iret )
    {
       gth_task_fini( pctx );
        syslog( LOG_CRIT, "set tem fail" );
        return;
    }
	
	
    syslog( LOG_CRIT, "set time, success" );
	printf("set time\n");
    gth_task_fini( pctx );
    return;
    
}


void  task_get_stat( gthrd_context_t * pctx, int tlen, uint8_t * pdat )
{
    int  iret;
    uint8_t * resp;
	tem_t tem;
   
    /**/
    if ( tlen == 0 )
    {
        gth_task_fini( pctx );
        syslog( LOG_CRIT, "get status, time out" );
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
 
  
   iret = util_check_readtem_resp( resp, READ_RSP_LEN );
    if ( 0 != iret )
    {
        return;
    }
	printf("status = %d\n",resp[3]);
	tem.attr = CB_ATTR_WORK_STAT;
	tem.data = resp[3];
	send_tem_to_uloop(pctx->sv[0],tem);
	
    syslog( LOG_CRIT, "wakeup, success" );
	printf("wakeup\n");
    return;
    
}


void  task_read_rem_tem( gthrd_context_t * pctx, int tlen, uint8_t * pdat )
{
    int  iret;
    uint8_t * resp;
	tem_t tem;
   
    /**/
    if ( tlen == 0 )
    {
        gth_task_fini( pctx );
        syslog( LOG_CRIT, "wakeup, time out" );
		

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
 
  
   iret = util_check_readtem_resp( resp, READ_RSP_LEN );
    if ( 0 != iret )
    {
        return;
    }
	printf("Remote = %d\n",resp[3]);

	tem.attr = CB_ATTR_REM;
	tem.data = resp[3];
	send_tem_to_uloop(pctx->sv[0],tem);
	
    temp_write_to_ini("/etc/CB/Remote.ini",resp[3],"Remote");
    
    /**/
    syslog( LOG_CRIT, "wakeup, success" );
	printf("wakeup\n");
    return;
    
}


/**/
void  task_read_tem( gthrd_context_t * pctx, int tlen, uint8_t * pdat )
{
    uint8_t buf_rem[READ_REQ_LEN]= {0XFF,0XAA,0X00,0X32,0X01,0XEE};
    int  iret;
    uint8_t * resp;
	tem_t   tem;
   
    /**/
    if ( tlen == 0 )
    {
        gth_task_fini( pctx );
        syslog( LOG_CRIT, "wakeup, time out" );
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
   
  
    iret = util_check_readtem_resp( resp, READ_RSP_LEN );
    if ( 0 != iret )
    {
        return;
    }
	printf("temp = %d\n",resp[3]);
			tem.attr = CB_ATTR_SHORT;
			tem.data = resp[3];
			send_tem_to_uloop(pctx->sv[0],tem);
   			temp_write_to_ini("/etc/CB/Short-range.ini",resp[3],"Short-range");
    /**/
    syslog( LOG_CRIT, "wakeup, success" );
	printf("wakeup\n");


    iret = evbuffer_get_length( pctx->pbuf );
    evbuffer_drain( pctx->pbuf, iret + 1 );
    
    gth_task_init( pctx, task_read_rem_tem );
    gth_task_active( pctx, READ_REQ_LEN, buf_rem, 6000 );
    return;
    
}


int  gthrd_spawn_get_tem(gthrd_context_t * pctx)
{
    uint8_t buf_read[READ_REQ_LEN] = {0xFF,0XAA,0X01,0X00,0X01,0XEE};
    
	/**/
	if ( NULL != pctx->tfunc )
	{
	    syslog( LOG_CRIT, "spawn task, but func is not null" );
	    return 1;
	}
    /**/
    evbuffer_drain( pctx->pbuf, 999999 );    
    gth_task_init( pctx, task_read_tem );
    gth_task_active( pctx, READ_REQ_LEN, buf_read, 6000 );
    return 0;

}



int  gthrd_spawn_get_ctrl_status(gthrd_context_t * pctx)
{
    uint8_t buf_stat[GET_CTRL_STAT] ={0XFF,0XAA,0X01,0X03,0X01,0XEE};

	/**/
	if ( NULL != pctx->tfunc )
	{
	    syslog( LOG_CRIT, "spawn task, but func is not null" );
	    return 1;
	}
    /**/
    evbuffer_drain( pctx->pbuf, 999999 );    
    gth_task_init( pctx, task_get_stat );
    gth_task_active( pctx, GET_CTRL_STAT, buf_stat, 6000 );
    return 0;

}


int  gthrd_spawn_set_target_tem( gthrd_context_t * pctx, uint32_t target_tem )
{
    uint8_t buf_set[SET_REQ_LEN] = {0xFF,0xBB,0x00,0x01,0x01,0x28,0xEE};

    /**/
    buf_set[5] = (uint8_t)( target_tem & 0xFF);
		
	/**/
	if ( NULL != pctx->tfunc )
	{
	    syslog( LOG_CRIT, "spawn task, but func is not null" );
	    return 1;
	}
    /**/
    evbuffer_drain( pctx->pbuf, 999999 );    
    gth_task_init( pctx, task_set_tem );
    gth_task_active( pctx, SET_REQ_LEN, buf_set, 6000 );
    return 0;
}


int  gthrd_spawn_set_rem_tem( gthrd_context_t * pctx, uint32_t rem_tem )
{
    uint8_t buf_set_rem[SET_REM_LEN] = {0xFF,0xBB,0x00,0x32,0x01,0x28,0xEE};

    /**/
    buf_set_rem[5] = (uint8_t)(rem_tem & 0xFF);

	/**/
	if ( NULL != pctx->tfunc )
	{
	    syslog( LOG_CRIT, "spawn task, but func is not null" );
	    return 1;
	}
    /**/
    evbuffer_drain( pctx->pbuf, 999999 );    
    gth_task_init( pctx, task_set_rem_tem );
    gth_task_active( pctx, SET_REM_LEN, buf_set_rem, 6000 );
    return 0;
}


int  gthrd_spawn_set_ctrl_mode( gthrd_context_t * pctx, uint32_t ctrl_mode )
{
    uint8_t buf_ctrl_mode[SET_CTR_MOD] = {0xFF,0xBB,0x00,0x33,0x01,0x00,0xEE};

    /**/
    buf_ctrl_mode[5] = (uint8_t)(ctrl_mode & 0xFF);
    
	/**/
	if ( NULL != pctx->tfunc )
	{
	    syslog( LOG_CRIT, "set ctrl  mode task, but func is not null" );
	    return 1;
	}
    /**/
    evbuffer_drain( pctx->pbuf, 999999 );    
    gth_task_init( pctx, task_set_ctrl_mode );
    gth_task_active( pctx, SET_CTR_MOD, buf_ctrl_mode, 6000 );
    return 0;
}


int  gthrd_spawn_set_time( gthrd_context_t * pctx, uint32_t hour, uint32_t min )
{
    uint8_t buf_time[SET_TIM_LEN] = {0xFF,0xBB,0x00,0x02,0x02,0x00,0x00,0xEE};

    /**/
    buf_time[6] = (uint8_t)(hour & 0xFF);
    buf_time[5] = (uint8_t)(min & 0xFF);

    /**/
    printf( "case hour =%d\n min = %d\n", buf_time[5], buf_time[6] );	
    
	/**/
	if ( NULL != pctx->tfunc )
	{
	    syslog( LOG_CRIT, "spawn task, but func is not null" );
	    return 1;
	}
    /**/
    evbuffer_drain( pctx->pbuf, 999999 );    
    gth_task_init( pctx, task_set_time );
    gth_task_active( pctx, SET_TIM_LEN, buf_time, 6000 );
    return 0;
    
}


int  gthrd_inter_delete_evbuf( gthrd_context_t * pctx )
{
    int  sock;

    if ( pctx->pevbuf == NULL )
    {
        return 1;
    }
    
    /**/
    sock = (int)bufferevent_getfd( pctx->pevbuf );
    bufferevent_free( pctx->pevbuf );
    pctx->pevbuf = NULL;

    /**/
    close( sock );
    pctx->dbg_close_count += 1;
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
    if ( NULL == pevbuf )
    {
        return 1;
    }

    bufferevent_setcb( pevbuf, gth_task_data_cbk, NULL, gthrd_inter_event_cbk, (void *)pctx );
    bufferevent_enable( pevbuf, EV_READ | EV_WRITE );
    
    /**/
    pctx->pevbuf = pevbuf;
    return 0;
}


void  gthrd_inter_check_new( gthrd_context_t * pctx )
{
    int  iret;
    notify_msg_t * pmsg;
    int  tlen;
    
    /**/
    if ( NULL != pctx->tfunc )
    {
        return;
    }

    /**/
    iret = msq_dequeue( pctx->mqctx, (void **)&pmsg, &tlen );
    if ( 0 != iret )
    {
        return;
    }

    /* new accept sock. */
    if ( pmsg->type == 0x22222222 )
    {
        if ( NULL != pctx->pevbuf )
        {
            gth_task_fini( pctx );
            gthrd_inter_delete_evbuf( pctx );
        }
        
        /**/
        gthrd_inter_create_evbuf( pctx, pmsg->value.sock );

        /**/
        iret = msq_dequeue( pctx->mqctx, (void **)&pmsg, &tlen );
        if ( 0 != iret )
        {
            return;
        }     
    }
    
    /**/
    if ( NULL == pctx->pevbuf )
    {
        return;
    }
    
    /**/
    switch ( pmsg->type )
    {
    case 0x11111111:
        pctx->target_tem = pmsg->value.target_tem;
		gthrd_spawn_set_target_tem( pctx, pmsg->value.target_tem );
        break;
        
	case 0x44444444:
	    /* read short and re*/
		gthrd_spawn_get_tem( pctx );
        break;
        
	case 0x66666666:
		gthrd_spawn_set_time( pctx, pmsg->hour, pmsg->min );
        break;
        
	case 0x77777777:
		gthrd_spawn_get_ctrl_status( pctx );
        break;

	case 0x88888888:
      	pctx->rem_tem = pmsg->value.rem_tem;
		gthrd_spawn_set_rem_tem( pctx, pmsg->value.rem_tem );
        break;
    
    case 0x99999999:
        pctx->ctrl_mode= pmsg->value.ctrl_mode;
		gthrd_spawn_set_ctrl_mode( pctx, pmsg->value.ctrl_mode );
        break;
        
    default:
        break;
    }

    return;
    
}


void  gthrd_inter_dgram_evt( evutil_socket_t ufd, short event, void * parg )
{
    int  iret;
    gthrd_context_t * pctx;
    uint8_t  tary[200];
    
    /**/
    while (1)
    {
        iret = recv( ufd, tary, 200, 0 );

        /**/
        if ( iret < 0 )
        {
            if ( (errno == EAGAIN) || (errno == EWOULDBLOCK) )
            {
                break;
            }
            else if ( errno == EINTR )
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if ( iret == 0 )
        {
            break;
        }

        /**/
        
    }
    
    /**/
    pctx = (gthrd_context_t *)parg;
    gthrd_inter_check_new( pctx );
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

 
    /**/
    pctx = (gthrd_context_t *)malloc( sizeof(gthrd_context_t) );
    if ( NULL == pctx )
    {
        return 1;
    }

    /**/
    pctx->pevbuf = NULL;
    pctx->tfunc = NULL;
    pctx->dbg_close_count = 0;
    pctx->target_tem = 0;    
    
    /**/
    pctx->pevbase = event_base_new();
    if ( NULL == pctx->pevbase )
    {
        return 2;
    }

    /**/
    pctx->pevtmr = evtimer_new( pctx->pevbase, gth_task_timer_cbk, (void *)pctx );
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
    pctx->prdevt = event_new( pctx->pevbase, pctx->sv[0], EV_READ|EV_PERSIST, gthrd_inter_dgram_evt, (void *)pctx );
    if ( pctx->prdevt == NULL )
    {
        return 7;
    }

    /**/
    iret = msq_init( &(pctx->mqctx) );
    if ( 0 != iret )
    {
        return 8;
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


int  gthrd_getstat( intptr_t ctx, char * pstr )
{
    gthrd_context_t * pctx;
    int  offset;
    
    /**/
    pctx = (gthrd_context_t *)ctx;

    /**/
    offset = sprintf( pstr, "target_tem=%d,", pctx->target_tem );
    if ( pctx->pevbuf == NULL )
    {
        offset += sprintf( pstr+offset, "sock not connected," );
    }
    else
    {
        offset += sprintf( pstr+offset, "sock is connected," );
    }
    
    offset += sprintf( pstr+offset, "close count=%d,", pctx->dbg_close_count );
    return 0;
    
}



int  gthrd_get_tem( intptr_t ctx )
{
	
	gthrd_context_t * pctx;
    notify_msg_t  msg;
    
    /**/
    pctx = (gthrd_context_t *)ctx;

    /**/
    msg.type = 0x44444444;

    /**/
    msq_enqueue( pctx->mqctx, sizeof(msg), &msg );
    send( pctx->sv[1], &msg, 4, 0 );
    return 0;
}



int  gthrd_get_ctrl_status( intptr_t ctx )
{
	gthrd_context_t * pctx;
    notify_msg_t  msg;
    
    /**/
    pctx = (gthrd_context_t *)ctx;

    /**/
    msg.type = 0x77777777;

    /**/
    msq_enqueue( pctx->mqctx, sizeof(msg), &msg );
    send( pctx->sv[1], &msg, 4, 0 );
    return 0;
}


/* notify meter id change. */
int  gthrd_set_target_tem( intptr_t ctx, uint32_t value )
{
    gthrd_context_t * pctx;
    notify_msg_t  msg;
    
    /**/
    pctx = (gthrd_context_t *)ctx;

    /**/
    msg.type = 0x11111111;
    msg.value.target_tem = value;

    /**/
    msq_enqueue( pctx->mqctx, sizeof(msg), &msg );
    send( pctx->sv[1], &msg, 4, 0 );
    return 0;
}


int  gthrd_set_rem_tem( intptr_t ctx, uint32_t  rem_tem )
{
    gthrd_context_t * pctx;
    notify_msg_t  msg;
    
    /**/
    pctx = (gthrd_context_t *)ctx;

    /**/
    msg.type = 0x88888888;
    msg.value.rem_tem = rem_tem;

    /**/
    msq_enqueue( pctx->mqctx, sizeof(msg), &msg );
    send( pctx->sv[1], &msg, 4, 0 );
    return 0;
}


int  gthrd_set_ctrl_mode( intptr_t ctx, uint32_t  ctrl_mode )
{
    gthrd_context_t * pctx;
    notify_msg_t  msg;
    
    /**/
    pctx = (gthrd_context_t *)ctx;

    /**/
    msg.type = 0x99999999;
    msg.value.ctrl_mode= ctrl_mode;

    /**/
    msq_enqueue( pctx->mqctx, sizeof(msg), &msg );
    send( pctx->sv[1], &msg, 4, 0 );
    return 0;
}


int  gthrd_set_time( intptr_t ctx, uint32_t hour, uint32_t min )
{
    gthrd_context_t * pctx;
    notify_msg_t  msg;
    
    /**/
    pctx = (gthrd_context_t *)ctx;

    /**/
    msg.type = 0x66666666;
    msg.hour = hour;
	msg.min = min;

    /**/
    msq_enqueue( pctx->mqctx, sizeof(msg), &msg );
    send( pctx->sv[1], &msg, 4, 0 );
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

    printf("\n******sockfd = %d**********\n", sock );
    /**/
    msq_enqueue( pctx->mqctx, sizeof(msg), &msg );
    send( pctx->sv[1], &msg, 4, 0 );
    return 0;

}







