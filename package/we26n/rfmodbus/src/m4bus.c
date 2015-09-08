
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
    uint8_t  bus;    
    uint8_t  addr;
    uint16_t  reg;
    uint16_t  num;

    /**/
    m4bus_cbk_func func;
    intptr_t arg;

    /**/
    intptr_t  ctx;
    struct _tag_m4bus_node * next;
    
} m4bus_node_t;



typedef struct _tag_m4bus_context
{
    /**/
    intptr_t  dgram;
    pkt_intf_t * pintf;

    /**/
    m4bus_node_t * phead[4];
    m4bus_node_t * ptail[4];
    
} m4bus_context_t;


int  m4bus_inter_send( m4bus_context_t * pctx, int idx, m4bus_node_t * pnode );


void  m4bus_inter_timer_callbk( struct uloop_timeout * ptmr )
{
    int  idx;
    m4bus_node_t * pnode;
    m4bus_context_t * pctx;

    /**/
    pnode = (m4bus_node_t *)ptmr;
    pctx = (m4bus_context_t *)(pnode->ctx);

    /**/
    pnode->func( pnode->arg, 0xFFFF, 0, NULL );
    
    /**/
    idx = pnode->bus;
    pctx->phead[idx] = pnode->next;
    free( pnode );
    
    /**/
    pnode = pctx->phead[idx];
    if ( NULL != pnode )
    {
        /**/
        pnode->timer.pending = 0;
        pnode->timer.cb = m4bus_inter_timer_callbk;
        uloop_timeout_set( &(pnode->timer), 200 );
        
        /**/
        m4bus_inter_send( pctx, idx, pnode );
    }
    
    return;
    
}


void  m4bus_inter_callbk( intptr_t arg, int tlen, void * pdat )
{
    int  idx;
    m4bus_context_t * pctx;
    m4bus_node_t * pnode;
    m4bus_rsp_t * prsp;

    /**/
    pctx = (m4bus_context_t *)arg;

    /**/
    prsp = (m4bus_rsp_t *)pdat;

    /**/
    idx = prsp->bus;
    if ( (idx < 0) || (idx > 3) )
    {
        return;
    }
    
    pnode = pctx->phead[idx];
    if ( NULL == pnode )
    {
        return;
    }

    /**/
    if ( (prsp->addr == pnode->addr) && (prsp->reg == pnode->reg) )
    {
        /**/
        uloop_timeout_cancel( &(pnode->timer) );
        
        /**/
        pnode->func( pnode->arg, prsp->uret, tlen-sizeof(m4bus_rsp_t), prsp->tary );
        
        /**/
        pctx->phead[idx] = pnode->next;
        free( pnode );

        /**/
        pnode = pctx->phead[idx];
        if ( NULL != pnode )
        {
            /**/
            pnode->timer.pending = 0;
            pnode->timer.cb = m4bus_inter_timer_callbk;
            uloop_timeout_set( &(pnode->timer), 200 );
                
            m4bus_inter_send( pctx, idx, pnode );
        }
        
    }

    return;
    
}




int  m4bus_inter_send( m4bus_context_t * pctx, int idx, m4bus_node_t * pnode )
{
    int  iret;
    m4bus_req_t  treq;

    /**/
    treq.bus = idx;
    treq.addr = pnode->addr;
    treq.reg = pnode->reg;
    treq.num = pnode->num;

    /**/
    iret = pctx->pintf->pkti_send( pctx->dgram, sizeof(m4bus_req_t), &treq );
    if ( 0 != iret )
    {
        return 1;
    }

    return 0;
    
}


int  m4bus_send_req( intptr_t ctx, m4bus_req_t * preq, m4bus_cbk_func func, intptr_t arg )
{
    int  iret;
    int  idx;
    m4bus_context_t * pctx;
    m4bus_node_t * pnode;

    /**/
    pctx = (m4bus_context_t *)ctx;

    /**/
    idx = preq->bus;
    if ( (idx < 0) || (idx > 3) )
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
    pnode->bus = preq->bus;    
    pnode->addr = preq->addr;
    pnode->reg = preq->reg;
    pnode->num = preq->num;
    pnode->func = func;
    pnode->arg = arg;
    pnode->ctx = ctx;
    pnode->next = NULL;
    
    /**/
    if ( NULL != pctx->phead[idx] )
    {
        pctx->ptail[idx]->next = pnode;
        pctx->ptail[idx] = pnode;
        printf( "head no null, enqueue\n");
        return 0;
    }

    /**/
    pctx->phead[idx] = pnode;
    pctx->ptail[idx] = pnode;

    /**/
    pnode->timer.pending = 0;
    pnode->timer.cb = m4bus_inter_timer_callbk;
    uloop_timeout_set( &(pnode->timer), 200 );

    /**/
    iret = m4bus_inter_send( pctx, idx, pnode );
    if ( 0 != iret )
    {
        return 4;
    }
    
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
    pctx->phead[0] = pctx->ptail[0] = NULL;
    pctx->phead[1] = pctx->ptail[1] = NULL;
    pctx->phead[2] = pctx->ptail[2] = NULL;
    pctx->phead[3] = pctx->ptail[3] = NULL;

    /**/
    pctx->pintf->pkti_set_callbk( pctx->dgram, m4bus_inter_callbk, (intptr_t)pctx );
    
    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}


