
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

#include <event2/event.h>

#include "newuart.h"
#include "bletask.h"




typedef struct _tag_task_context
{
    struct event_base * pevbase;
    intptr_t  uartctx;

    /* timer */
    struct timeval tv;  
    struct event * evt;
    
    /**/
    uint16_t  cmd;

    /* cback func */
    task_cbk_func  func;
    
    /* user ptr */
    int  tsize;
    uint8_t  tptr[4];
    
} task_context_t;


/**/
int  task_fini( intptr_t tctx )
{
    task_context_t * pctx;

    /**/
    pctx = (task_context_t *)tctx;

    /* timer */
    evtimer_del( pctx->evt );
    event_free( pctx->evt );
    
    /**/
    nuart_set_callbk( pctx->uartctx, NULL, tctx );

    /**/
    free( pctx );
    return 0;
}


/**/
static void  task_timer_cbk( evutil_socket_t ufd, short event, void * parg )
{
    int  iret;
    task_context_t * pctx;
    
    /**/
    pctx = (task_context_t *)parg;

    /**/
    iret = 0;
    if ( NULL != pctx->func )
    {
        iret = pctx->func( (intptr_t)pctx, ACK_TMR_OUT, 0, NULL );
    }

    if ( iret == 0 )
    {
        /**/
        task_fini( (intptr_t)pctx );
    }
    
    return;
    
}


static void  task_uart_cbk( intptr_t arg, uint16_t cmd, int tlen, void * pdat )
{
    int  iret;
    task_context_t * pctx;
    
    /**/
    pctx = (task_context_t *)arg;

    /**/
    if ( NULL == pctx->func )
    {
        /* error */
        printf( "error error, func is NULL \n" );
        task_fini( (intptr_t)pctx );        
        return;
    }

    /* 0 表示结束, 其他表示继续等待回调 或者 超时. */
    iret = pctx->func( (intptr_t)pctx, cmd, tlen, (uint8_t *)pdat );
    if ( 0 == iret )
    {
        task_fini( (intptr_t)pctx );        
    }
    
    return;
    
}


/**/
int  task_init( struct event_base * pevbase, intptr_t uartctx, int usize, intptr_t * pret )
{
    task_context_t * pctx;

    /**/
    pctx = (task_context_t *)malloc( sizeof(task_context_t) + usize );
    if ( NULL == pctx )
    {
        return 1;
    }

    /**/
    memset( pctx, 0, sizeof(task_context_t) + usize ) ;
    
    /**/
    pctx->pevbase = pevbase;
    pctx->uartctx = uartctx;
    pctx->tsize = usize;

    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}


int  task_inherit( intptr_t oldctx, int usize, intptr_t * pret )
{
    int  iret;
    task_context_t * pctx;
    task_context_t * npctx;

    /**/
    pctx = (task_context_t *)oldctx;

    /**/
    iret = task_init( pctx->pevbase, pctx->uartctx, usize, pret );
    if ( 0 != iret )
    {
        return iret;
    }

    /**/
    if ( (usize > 0) && (pctx->tsize > 0) )
    {
        npctx = (task_context_t *)(*pret);
        
        if ( pctx->tsize < usize )
        {
            usize = pctx->tsize;
        }
        
        memcpy( npctx->tptr, pctx->tptr, usize );
    }

    /**/
    return 0;
    
}


int  task_getptr( intptr_t tctx, void ** pptr )
{
    task_context_t * pctx;

    /**/
    pctx = (task_context_t *)tctx;

    /**/
    *pptr = (void *)(pctx->tptr);
    return 0;
}


int  task_active( intptr_t tctx, uint16_t cmd, uint8_t tlen, uint8_t * pdat, task_cbk_func func, uint32_t tms )
{
    task_context_t * pctx;

    /**/
    pctx = (task_context_t *)tctx;

    /**/
    pctx->cmd = cmd;
    pctx->func = func;

    /* timer */
    pctx->tv.tv_sec = tms / 1000;
    pctx->tv.tv_usec = tms % 1000;
    pctx->tv.tv_usec *= 1000;
    pctx->evt = evtimer_new( pctx->pevbase, task_timer_cbk, (void *)pctx );
    evtimer_add( pctx->evt, &(pctx->tv) );

    /**/
    nuart_set_callbk( pctx->uartctx, task_uart_cbk, tctx );
    if ( cmd != 0 )
    {
        nuart_send( pctx->uartctx, cmd, tlen, pdat );
    }
    
    return 0;
    
}


int  task_reactive( intptr_t tctx, uint16_t cmd, uint8_t tlen, uint8_t * pdat, task_cbk_func func, uint32_t tms )
{
    task_context_t * pctx;

    /**/
    pctx = (task_context_t *)tctx;

    /**/
    pctx->cmd = cmd;
    pctx->func = func;

    /* timer */
    pctx->tv.tv_sec = tms / 1000;
    pctx->tv.tv_usec = tms % 1000;
    pctx->tv.tv_usec *= 1000;
    evtimer_del( pctx->evt );
    evtimer_add( pctx->evt, &(pctx->tv) );

    /**/
    nuart_set_callbk( pctx->uartctx, task_uart_cbk, tctx );
    if ( cmd != 0 )
    {
        nuart_send( pctx->uartctx, cmd, tlen, pdat );
    }
    return 0;
    
}




