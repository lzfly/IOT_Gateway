
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

#include <event2/event.h>

#include "newuart.h"
#include "bletask.h"
#include "pktintf.h"
#include "dgramsck.h"
#include "ubussrv.h"
#include "m4bus.h"
#include "mainthrd.h"


void  dump_hex( const unsigned char * ptr, size_t len );
uint16_t  crc16_ccitt( int tlen, uint8_t * pdata );


/* pret data len : 8 + tlen */
static void  meter_encode( uint8_t addr, uint16_t cmd, int tlen, uint8_t * pdat, uint8_t * pret )
{
    int  i;
    uint8_t  tcrc;

    /**/
    pret[0] = 0x68;
    pret[1] = 0x50;
    pret[2] = (uint8_t)cmd;
    
    /**/
    if ( tlen > 0 )
    {
        memcpy( &(pret[3]), pdat, tlen );
    }
    
    /* crc */
    tcrc = 0;
    for ( i=0; i<tlen+3; i++ )
    {
        tcrc += pret[i];
    }
    
    /**/
    pret[3+tlen] = tcrc;
    pret[3+tlen+1] = 0x16;
    return;
    
}


static int  meter_check( uint8_t addr, uint16_t cmd, int tlen, uint8_t * pdat )
{

    /**/
    if ( (pdat[0] != 0x68) || (pdat[1] != 0x50) )
    {
        return 1;
    }

    /**/
    if ( pdat[tlen-1] != 0x16 )
    {
        return 2;
    }

    /**/
    return 0;
    
}



/**/
typedef struct _tag_mthrd_context
{
    struct event_base * pevbase;
    intptr_t  uctx;

    /* socket pair */
    int  sv[2];
    intptr_t  dgctx;
    pkt_intf_t * pintf;
    struct event * prdevt;

    /**/
    intptr_t  dtask;
    
} mthrd_context_t;


void  mthrd_commn_resp_code( mthrd_context_t * pctx, m4bus_req_t * preq, uint16_t uret )
{
    m4bus_rsp_t  trsp;
    
    /**/
    trsp.addr = preq->addr;
    trsp.cmd = preq->cmd;
    trsp.uret = uret;

    /**/
    pctx->pintf->pkti_send( pctx->dgctx, sizeof(m4bus_rsp_t), &trsp );
    return;
}


void  mthrd_commn_resp_arry( mthrd_context_t * pctx, m4bus_req_t * preq, int tlen, uint8_t * pdat )
{
    m4bus_rsp_t * prsp;
    
    /**/
    prsp = (m4bus_rsp_t *)alloca( sizeof(m4bus_rsp_t) + tlen );
    if ( prsp == NULL )
    {
        return;
    }

    /**/
    prsp->addr = preq->addr;
    prsp->cmd = preq->cmd;
    prsp->uret = 0;
    memcpy( prsp->tary, pdat, tlen );
    pctx->pintf->pkti_send( pctx->dgctx, sizeof(m4bus_rsp_t) + tlen, prsp );
    return;
}

typedef struct _tag_task_exchage
{
    mthrd_context_t * pctx;  
    m4bus_req_t  treq;
    int  offs;
    uint8_t  tpad[8];
    
} task_exchage_t;


int  mthrd_exch_waitnotify( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    int  iret;
    task_exchage_t * pexc;
        
    /**/
    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pexc );

    /**/
    if ( cmd == CMD_EVT_NOTY )
    {
        if ( tlen < 2 )
        {
            mthrd_commn_resp_code( pexc->pctx, &(pexc->treq), 0xEE07 );
            return 0;
        }

        if ( (pdat[0] != 0x28) || (pdat[1] != 0x00) )
        {
            mthrd_commn_resp_code( pexc->pctx, &(pexc->treq), 0xEE08 );
            return 0;        
        }

        /**/
        tlen -= 2;
        pdat += 2;

        if ( (pexc->offs + tlen) > 40 )
        {
            printf( "offs too large\n ");
            mthrd_commn_resp_code( pexc->pctx, &(pexc->treq), 0xEE09 );
            return 0;
        }

        /**/
        memcpy( pexc->tpad + pexc->offs, pdat, tlen );
        pexc->offs += tlen;

        /**/
        iret = meter_check( pexc->treq.addr, pexc->treq.cmd, pexc->offs, pexc->tpad );
        if ( 0 == iret )
        {
            /**/
            tlen = pexc->offs - 8;
            pdat = pexc->tpad + 5;
            
            /**/
            mthrd_commn_resp_arry( pexc->pctx, &(pexc->treq), tlen, pdat );
            return 0;
        }
        
    }

    /**/
    if ( cmd == ACK_TMR_OUT )
    {
        mthrd_commn_resp_code( pexc->pctx, &(pexc->treq), 0xEE0A );
        return 0;
    }
    
    return 1;
    
}


int  mthrd_exch_waitwrite( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    task_exchage_t * pexc;
        
    /**/
    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pexc );

    /**/
    if ( cmd == CMD_WRT_NRSP )
    {
        if ( tlen <= 0 )
        {
            mthrd_commn_resp_code( pexc->pctx, &(pexc->treq), 0xEE04 );
            return 0;
        }

        if ( pdat[0] != 0 )
        {
            mthrd_commn_resp_code( pexc->pctx, &(pexc->treq), 0xEE05 );
            return 0;
        }
                
        /**/
        pexc->offs = 0;
        task_reactive( tctx, CMD_NULL, 0, NULL, mthrd_exch_waitnotify, 3000 );
        return 1;
    }

    /**/
    if ( cmd == ACK_TMR_OUT )
    {
        mthrd_commn_resp_code( pexc->pctx, &(pexc->treq), 0xEE06 );
        return 0;
    }
    
    return 1;
    
}


void  mthrd_start_exchage( mthrd_context_t * pctx, m4bus_req_t * preq )
{
    int  iret;
    intptr_t  tctx;
    task_exchage_t * pexc;
    
    /**/
    iret = task_init( pctx->pevbase, pctx->uctx, sizeof(task_exchage_t) + 32, &tctx );
    if ( 0 != iret )
    {
        printf( "mthrd, getstate task_init fail, %d", iret );
        return;
    }
    
    /**/
    task_getptr( tctx,(void **)&pexc );
    pexc->pctx = pctx;
    memcpy( &(pexc->treq), preq, sizeof(m4bus_req_t) );

    /**/
    pexc->tpad[0] = 0x25;
    pexc->tpad[1] = 0x00;
    meter_encode( preq->addr, preq->cmd, preq->tlen, preq->tary, &(pexc->tpad[2]) );
    
    /**/
    task_active( tctx, CMD_WRT_NRSP, 2 + 5 + preq->tlen, pexc->tpad, mthrd_exch_waitwrite, 2000 );
    return;

}


typedef struct _tag_task_connect
{
    mthrd_context_t * pctx;  
    m4bus_req_t  treq;
    
} task_connect_t;


int  mthrd_discon_waitdiscon( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    task_connect_t * pconn;
        
    /**/
    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pconn );

    /**/
    if ( cmd == CMD_DISCONNT )
    {
        if ( tlen <= 0)
        {
            mthrd_commn_resp_code( pconn->pctx, &(pconn->treq), 0xEE04 );
            return 0;
        }

        if ( pdat[0] != 0 )
        {
            mthrd_commn_resp_code( pconn->pctx, &(pconn->treq), 0xEE05 );
            return 0;
        }

        
        /**/
        mthrd_commn_resp_code( pconn->pctx, &(pconn->treq), 0x0 );
        return 0;
        
    }

    /**/
    if ( cmd == ACK_TMR_OUT )
    {
        mthrd_commn_resp_code( pconn->pctx, &(pconn->treq), 0xEE06 );
        return 0;
    }

    return 1;
    
}



void  mthrd_start_disconnect( mthrd_context_t * pctx, m4bus_req_t * preq )
{
    int  iret;
    intptr_t  tctx;
    task_connect_t * pconn;
    
    /**/
    iret = task_init( pctx->pevbase, pctx->uctx, sizeof(task_connect_t), &tctx );
    if ( 0 != iret )
    {
        printf( "mthrd, getstate task_init fail, %d", iret );
        return;
    }

    /**/
    task_getptr( tctx,(void **)&pconn );
    pconn->pctx = pctx;
    memcpy( &(pconn->treq), preq, sizeof(m4bus_req_t) );
    
    /**/
    task_active( tctx, CMD_DISCONNT, 0, NULL, mthrd_discon_waitdiscon, 1000 );
    return;

}


int  mthrd_conn_ennotify( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    task_connect_t * pconn;
    
    /**/
    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pconn );

    /**/
    if ( cmd == CMD_ENA_NOTY )
    {
        if ( tlen <= 0 )
        {
            mthrd_commn_resp_code( pconn->pctx, &(pconn->treq), 0xEE04 );
            return 0;
        }

        if ( pdat[0] != 0 )
        {
            mthrd_commn_resp_code( pconn->pctx, &(pconn->treq), 0xEE05 );
            return 0;
        }

        /**/
        mthrd_commn_resp_code( pconn->pctx, &(pconn->treq), 0x0 );
        return 0;
    }

    /**/
    if ( cmd == ACK_TMR_OUT )
    {
        mthrd_commn_resp_code( pconn->pctx, &(pconn->treq), 0xEE06 );
        return 0;
    }

    return 1;
    
}


int  mthrd_conn_waitconn( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    task_connect_t * pconn;
    uint8_t  tary[2];
    
    /**/
    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pconn );

    /**/
    if ( cmd == CMD_CONNECT )
    {
        if ( tlen <= 0)
        {
            mthrd_commn_resp_code( pconn->pctx, &(pconn->treq), 0xEE04 );
            return 0;
        }

        if ( pdat[0] != 0 )
        {
            mthrd_commn_resp_code( pconn->pctx, &(pconn->treq), 0xEE05 );
            return 0;
        }

        /**/
        tary[0] = 0x29;
        tary[1] = 0x00;
        
        /**/
        task_reactive( tctx, CMD_ENA_NOTY, 2, tary, mthrd_conn_ennotify, 2000 );
        return 1;
        
    }

    /**/
    if ( cmd == ACK_TMR_OUT )
    {
        mthrd_commn_resp_code( pconn->pctx, &(pconn->treq), 0xEE06 );
        return 0;
    }

    return 1;
    
}


int  mthrd_conn_checkstate( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    task_connect_t * pconn;

    /**/
    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pconn );
    
    if ( cmd == CMD_GET_STATE )
    {
        /**/
        if ( tlen <= 0)
        {
            mthrd_commn_resp_code( pconn->pctx, &(pconn->treq), 0xEE01 );
            return 0;
        }

        if ( pdat[0] != CYBLE_STATE_DISCONNECTED )
        {
            mthrd_commn_resp_code( pconn->pctx, &(pconn->treq), 0xEE02 );
            return 0;
        }
        
        /**/
        task_reactive( tctx, CMD_CONNECT, 6, pconn->treq.tary, mthrd_conn_waitconn, 4000 );
        return 1;
    }
    
    if ( cmd == ACK_TMR_OUT )
    {
        mthrd_commn_resp_code( pconn->pctx, &(pconn->treq), 0xEE03 );
        return 0;
    }

    return 1;
    
}


void  mthrd_start_connect( mthrd_context_t * pctx, m4bus_req_t * preq )
{
    int  iret;
    intptr_t  tctx;
    task_connect_t * pconn;
    
    /**/
    iret = task_init( pctx->pevbase, pctx->uctx, sizeof(task_connect_t) + 6, &tctx );
    if ( 0 != iret )
    {
        printf( "mthrd, getstate task_init fail, %d", iret );
        return;
    }

    /**/
    task_getptr( tctx,(void **)&pconn );
    pconn->pctx = pctx;
    memcpy( &(pconn->treq), preq, sizeof(m4bus_req_t) + 6 );
    
    /**/
    task_active( tctx, CMD_GET_STATE, 0, NULL, mthrd_conn_checkstate, 1000 );
    return;

}


typedef struct _tag_task_getstate
{
    mthrd_context_t * pctx;  
    m4bus_req_t  treq;
    
} task_getstate_t;


int  mthrd_gst_checkstate( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    task_getstate_t * pgst;

    /**/
    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx,(void **)&pgst );
    
    if ( cmd == CMD_GET_STATE )
    {
        /**/
        if ( tlen <= 0)
        {
            mthrd_commn_resp_code( pgst->pctx, &(pgst->treq), 0xEE01 );
            return 0;
        }

        /**/
        mthrd_commn_resp_arry( pgst->pctx, &(pgst->treq), 1, pdat );
        return 0;
    }
    
    if ( cmd == ACK_TMR_OUT )
    {
        mthrd_commn_resp_code( pgst->pctx, &(pgst->treq), 0xEE02 );
        return 0;
    }

    return 1;
    
}



void  mthrd_start_getstate( mthrd_context_t * pctx, m4bus_req_t * preq )
{
    int  iret;
    intptr_t  tctx;
    task_getstate_t * pgst;
    
    /**/
    iret = task_init( pctx->pevbase, pctx->uctx, sizeof(task_getstate_t), &tctx );
    if ( 0 != iret )
    {
        printf( "mthrd, getstate task_init fail, %d", iret );
        return;
    }

    /**/
    task_getptr( tctx,(void **)&pgst );
    pgst->pctx = pctx;
    memcpy( &(pgst->treq), preq, sizeof(m4bus_req_t) );
    
    /**/
    task_active( tctx, CMD_GET_STATE, 0, NULL, mthrd_gst_checkstate, 1000 );
    return;
    
}


void  mthrd_inter_dgram_cbk( intptr_t arg, int tlen, void * pdat )
{
    mthrd_context_t * pctx;
    m4bus_req_t * preq;

    /**/
    pctx = (mthrd_context_t *)arg;
    preq = (m4bus_req_t *)pdat;

    /**/
    printf( " addr=%d, cmd=%d\n", preq->addr, preq->cmd );
    dump_hex( preq->tary, preq->tlen );

    /**/
    if ( preq->addr == 0 )
    {
        if ( preq->cmd == SRV_CMD_GETSTATE )
        {
            mthrd_start_getstate( pctx, preq );
            return;
        }

        if ( preq->cmd == SRV_CMD_CONNECT )
        {
            mthrd_start_connect( pctx, preq );
            return;
        }

        
        if ( preq->cmd == SRV_CMD_DISCONN )
        {
            mthrd_start_disconnect( pctx, preq );
            return;
        }

    }

    /**/
    mthrd_start_exchage( pctx, preq );
    return;
    
    
}



void  mthrd_inter_dgram_evt( evutil_socket_t ufd, short event, void * parg )
{
    mthrd_context_t * pctx;

    /**/
    pctx = (mthrd_context_t *)parg;
    
    /**/
    pctx->pintf->pkti_try_run( pctx->dgctx );
    return;
}

/*  */
void  mthrd_inter_udata_cbk( intptr_t arg, int tlen, void * pdat )
{
    m4bus_rsp_t * prsp;
    mthrd_context_t * pctx;

    /**/
    pctx = (mthrd_context_t *)arg;

    /**/
    if ( tlen <= 0 )
    {
        return;
    }

    /**/
    prsp = (m4bus_rsp_t *)alloca( sizeof(m4bus_rsp_t) + tlen );
    if ( prsp == NULL )
    {
        return;
    }

    /**/
    prsp->addr = 0;
    prsp->cmd = SRV_ACK_DATA;
    prsp->uret = 0;
    memcpy( prsp->tary, pdat, tlen );
    pctx->pintf->pkti_send( pctx->dgctx, sizeof(m4bus_rsp_t) + tlen, prsp );
    return;
    
}


int  mthrd_init( intptr_t * pret )
{
    int  iret;
    mthrd_context_t * pctx;

    /**/
    pctx = (mthrd_context_t *)malloc( sizeof(mthrd_context_t) );
    if ( NULL == pctx )
    {
        return 1;
    }
    
    /**/
    pctx->pevbase = event_base_new();
    if ( NULL == pctx->pevbase )
    {
        return 2;
    }

    /**/
    iret = nuart_init( pctx->pevbase, 12, &(pctx->uctx) );
    if ( 0 != iret )
    {
        printf( "uart init ret = %d\n", iret );
        return 3;
    }

    /**/
    iret = socketpair( AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK, 0, pctx->sv );
    if ( 0 != iret )
    {
        return 4;
    }
    
    /**/
    iret = dgram_sock_init( pctx->sv[0], &(pctx->dgctx), &(pctx->pintf) );
    if ( 0 != iret )
    {
        return 5;
    }

    /**/
    pctx->pintf->pkti_set_callbk( pctx->dgctx, mthrd_inter_dgram_cbk, (intptr_t)pctx );

    /**/
    pctx->prdevt = event_new( pctx->pevbase, pctx->sv[0], EV_READ|EV_PERSIST, mthrd_inter_dgram_evt, (void *)pctx );
    if ( pctx->prdevt == NULL )
    {
        return 6;
    }
    
    /**/
    event_add( pctx->prdevt, NULL );

    /**/
    pctx->dtask = 0;
    
    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}


int  mthrd_get_sfd( intptr_t ctx, int * psfd )
{
    mthrd_context_t * pctx;

    /**/
    pctx = (mthrd_context_t *)ctx;

    /**/
    *psfd = pctx->sv[1];
    return 0;
}


int  mthrd_run( intptr_t ctx )
{
    mthrd_context_t * pctx;

    /**/
    pctx = (mthrd_context_t *)ctx;

    // 开始主程序的循环
    event_base_dispatch( pctx->pevbase );
    return 0;
}



