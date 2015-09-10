
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>

#include "pktintf.h"
#include "dgramsck.h"
#include "m4bus.h"


/* 
todo: 
  增加定时器, 超时没有收到响应, 
  则自动删除, 并且调用回调函数表示超时. 
  超时函数给 ubus 返回错误码.
*/

typedef struct _tag_m4bus_node
{
    struct uloop_timeout  timer;

    /**/
    uint8_t  addr;
    uint16_t  cmd;
    
    /**/
    m4bus_cbk_func func;
    intptr_t arg;

    /**/
    intptr_t  ctx;
    
} m4bus_node_t;



typedef struct _tag_m4bus_context
{
    /**/
    intptr_t  dgram;
    pkt_intf_t * pintf;

    /**/
    m4bus_node_t * pnode;
    
} m4bus_context_t;



void  m4bus_inter_timer_callbk( struct uloop_timeout * ptmr )
{
    m4bus_node_t * pnode;
    m4bus_context_t * pctx;

    /**/
    pnode = (m4bus_node_t *)ptmr;
    pctx = (m4bus_context_t *)(pnode->ctx);

    /**/
    pnode->func( pnode->arg, 0xFFFF, 0, NULL );
    
    /**/
    free( pnode );
    pctx->pnode = NULL;
    
    return;
    
}


void  m4bus_inter_callbk( intptr_t arg, int tlen, void * pdat )
{
    m4bus_context_t * pctx;
    m4bus_node_t * pnode;
    m4bus_rsp_t * prsp;

    /**/
    pctx = (m4bus_context_t *)arg;

    /**/
    if ( tlen < sizeof(m4bus_rsp_t) )
    {
        printf( "m4bus cbk, tlen too small \n" );
        return;
    }

    /**/
    prsp = (m4bus_rsp_t *)pdat;

    /**/
    pnode = pctx->pnode;
    if ( NULL == pnode )
    {
        printf( "m4bus cbk, node NULL \n" );
        return;
    }

    /**/
    if ( (prsp->addr != pnode->addr) || (prsp->cmd != pnode->cmd) )
    {
        printf( "m4bus cbk, not match\n" );
        return;
    }

    /**/
    uloop_timeout_cancel( &(pnode->timer) );
    
    /**/
    if ( NULL != pnode->func )
    {
        pnode->func( pnode->arg, prsp->uret, tlen-sizeof(m4bus_rsp_t), prsp->tary );
    }
    
    /**/
    pctx->pnode = NULL;
    free( pnode );    
    return;
    
}


int  m4bus_send_req( intptr_t ctx, m4bus_req_t * preq, m4bus_cbk_func func, intptr_t arg, int tms )
{
    int  iret;
    m4bus_context_t * pctx;
    m4bus_node_t * pnode;

    /**/
    pctx = (m4bus_context_t *)ctx;

    /**/
    if ( NULL != pctx->pnode )
    {
        return 1;
    }
    
    /**/
    pnode = (m4bus_node_t *)malloc( sizeof(m4bus_node_t) );
    if ( NULL == pnode )
    {
        return 2;
    }

    /**/
    pnode->addr = preq->addr;
    pnode->cmd = preq->cmd;
    pnode->func = func;
    pnode->arg = arg;
    pnode->ctx = ctx;
    
    /**/
    pnode->timer.pending = 0;
    pnode->timer.cb = m4bus_inter_timer_callbk;
    uloop_timeout_set( &(pnode->timer), tms );
    
    /**/
    iret = pctx->pintf->pkti_send( pctx->dgram, sizeof(m4bus_req_t) + preq->tlen, preq );
    if ( 0 != iret )
    {
        uloop_timeout_cancel( &(pnode->timer) );
        free( pnode );
        return 4;
    }
    
    /**/
    pctx->pnode = pnode;
    return 0;
    
}


int  m4bus_get_fd( intptr_t ctx, int * pfd )
{
    m4bus_context_t * pctx;

    /**/
    pctx = (m4bus_context_t *)ctx;

    /**/
    return pctx->pintf->pkti_get_fd( pctx->dgram, pfd );
}


int  m4bus_try_run( intptr_t ctx )
{
    m4bus_context_t * pctx;

    /**/
    pctx = (m4bus_context_t *)ctx;

    /**/
    return pctx->pintf->pkti_try_run( pctx->dgram );
}


int  m4bus_init( int fd, intptr_t * pret )
{
    int  iret;
    m4bus_context_t * pctx;

    /**/
    pctx = (m4bus_context_t *)malloc( sizeof(m4bus_context_t) );
    if ( NULL == pctx )
    {
        return 1;
    }
    
    /**/
    iret = dgram_sock_init( fd, &(pctx->dgram), &(pctx->pintf) );
    if ( 0 != iret )
    {
        free( pctx );
        return 2;
    }

    /**/
    pctx->pnode = NULL;
    
    /**/
    pctx->pintf->pkti_set_callbk( pctx->dgram, m4bus_inter_callbk, (intptr_t)pctx );
    
    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}


