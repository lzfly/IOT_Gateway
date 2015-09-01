
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


void  dump_hex( const unsigned char * ptr, size_t  len );


#define  ST_UNKNOW          0           /* 启动开始阶段, 不知道模块状态. */
#define  ST_ERROR           1           

#define  ST_UNPAIR          2           /* 没有配对过, 没有对端连接. */
#define  ST_PAIRING         3           /* 正在配对进行中. */
#define  ST_PAIRED          4           /* 已经配对过, 可以尝试通讯. */



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
    int state;
    int ecode;
    uint8_t  saddr[6];
    uint8_t  taddr[6];

    /**/
    intptr_t  dtask;
    
} mthrd_context_t;



typedef struct _tag_mtask_start_pair
{
    mthrd_context_t * pctx;
    uint8_t  taddr[6];
    int  result;
    
} mtask_start_pair_t;



int  mtask_set_target_host( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    mtask_start_pair_t * pair;

printf( "cmd = %X : ", cmd );
dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pair );
    if ( cmd == ACK_GET_DHOST )
    {
        if ( tlen >= 6 )
        {
            memcpy( pair->pctx->taddr, pair->taddr, 6 );
            pair->pctx->state = ST_PAIRED;
            pair->pctx->ecode = 0;
            return 0;
        }
    }
    
    /**/
    if ( cmd == ACK_TMR_OUT )
    {
        pair->pctx->state = ST_ERROR;
        pair->pctx->ecode = 11;
        return 0;
    }

    return 1;
    
}


int  mtask_get_target_addr( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    mtask_start_pair_t * pair;

printf( "cmd = %X : ", cmd );
dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pair );

    /**/
    if ( cmd == ACK_GET_DADR )
    {
        if ( tlen >= 6 )
        {
            memcpy( pair->taddr, pdat, 6 );
            task_reactive( tctx, CMD_GET_DHOST, 6, pair->taddr, mtask_set_target_host, 6000 );    
            return 1;
        }
    }
    
    /**/
    if ( cmd == ACK_TMR_OUT )
    {
        pair->pctx->state = ST_ERROR;
        pair->pctx->ecode = 10;
        return 0;
    }

    return 1;
    
}


int  mtask_delay( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    if ( cmd == ACK_TMR_OUT )
    {
        task_reactive( tctx, CMD_GET_DADR, 0, NULL, mtask_get_target_addr, 6000 );    
    }

    return 1;
    
}



int  mtask_stop_pair( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    mtask_start_pair_t * pair;
    
printf( "cmd = %X : ", cmd );
dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pair );

    if ( cmd == ACK_IO_PAIR )
    {
        if ( tlen >= 1 )
        {
            if ( pdat[0] == 0 )
            {
                if ( pair->result == 0 )
                {
                    task_reactive( tctx, CMD_NULL, 0, NULL, mtask_delay, 1200 );
                    return 1;
                }
                else
                {
                    pair->pctx->state = ST_UNPAIR;
                    pair->pctx->ecode = 0;
                    return 0;
                }
            }
        }
    }

    if ( cmd == ACK_TMR_OUT )
    {
        pair->pctx->state = ST_ERROR;
        pair->pctx->ecode = 9;
        return 0;    
    }

    return 1;
    
}



int  mtask_start_pair( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    mtask_start_pair_t * pair;
    uint8_t  tary[4];
    
printf( "cmd = %X : ", cmd );
dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pair );

    /**/
    if ( cmd == EVT_IO_PAIR )
    {
        if ( tlen >= 1 )
        {
            /* pair success */
            if ( pdat[0] == 0x0 )
            {
                pair->result = 0;
                tary[0] = 0;
                task_reactive( tctx, CMD_IO_PAIR, 1, tary, mtask_stop_pair, 3000 );
                return 1;
            }
        }
    }

    if ( cmd == ACK_TMR_OUT )
    {
        pair->result = 1;
        tary[0] = 0;
        task_reactive( tctx, CMD_IO_PAIR, 1, tary, mtask_stop_pair, 3000 );
        return 1;
    }
    
    /**/
    return 1;
    
}



int  mtask_auto_chn( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    mtask_start_pair_t * pair;
    uint8_t  tary[4];
    
printf( "cmd = %X : ", cmd );
dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pair );

    /**/
    if ( cmd == ACK_AUTO_CHN )
    {
        if ( tlen < 1 )
        {
            return 1;
        }

        if ( pdat[0] != 0 )
        {
            return 1;
        }

        printf( "auto chn ok\n" );
    }

    if ( cmd == EVT_AUTO_CHN )
    {
        if ( tlen < 1 )
        {
            return 1;
        }

        if ( pdat[0] != 0 )
        {
            return 1;
        }

        /**/
        tary[0] = 1;
        task_reactive( tctx, CMD_IO_PAIR, 1, tary, mtask_start_pair, 30000 );
        return 1;
    }

    if ( cmd == ACK_TMR_OUT )
    {
        pair->pctx->state = ST_ERROR;
        pair->pctx->ecode = 8;
        return 0;
    }

    return 1;
    
}


int  mtask_stop_io_pair( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    mtask_start_pair_t * pair;
    uint8_t  tary[4];
    
printf( "cmd = %X : ", cmd );
dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pair );

    /**/
    if ( cmd == ACK_IO_PAIR )
    {
        if ( tlen >= 1 )
        {
            if ( pdat[0] == 0 )
            {
                task_reactive( tctx, CMD_AUTO_CHN, 0, NULL, mtask_auto_chn, 10000 );
            }
            else
            {
                tary[0] = 0;
                task_reactive( tctx, CMD_IO_PAIR, 1, tary, mtask_stop_io_pair, 1000 );
            }
        }
    }

    /**/
    if ( cmd == ACK_TMR_OUT )
    {
        pair->pctx->state = ST_ERROR;
        pair->pctx->ecode = 7;
        return 0;
    }
    
    return 1;
    
}


int  mtask_get_io_pair( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    mtask_start_pair_t * pair;
    uint8_t  tary[4];
    
printf( "cmd = %X : ", cmd );
dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pair );

    /**/
    if ( cmd == ACK_IO_PAIR )
    {
        if ( tlen >= 1 )
        {
            if ( pdat[0] == 0 )
            {
                task_reactive( tctx, CMD_AUTO_CHN, 0, NULL, mtask_auto_chn, 10000 );
            }
            else
            {
                tary[0] = 0;
                task_reactive( tctx, CMD_IO_PAIR, 1, tary, mtask_stop_io_pair, 1000 );
            }
        }
    }

    /**/
    if ( cmd == ACK_TMR_OUT )
    {
        pair->pctx->state = ST_ERROR;
        pair->pctx->ecode = 6;
        return 0;
    }

    return 1;
    
}


int  mtask_set_node_type( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    mtask_start_pair_t * pair;
    uint8_t  tary[4];
    
printf( "cmd = %X : ", cmd );
dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pair );
    
    /**/
    if ( cmd == ACK_NODE_TYP )
    {
        if ( tlen >= 1 )
        {
            /* 3: 主节点(配对). */
            if ( pdat[0] == 3 )
            {
                task_reactive( tctx, CMD_IO_PAIR, 0, NULL, mtask_get_io_pair, 1000 );
            }
            else
            {
                tary[0] = 3;
                task_reactive( tctx, CMD_NODE_TYP, 1, tary, mtask_set_node_type, 2000 );
            }
            
            return 1;
        }
    }

    /**/
    if ( cmd == ACK_TMR_OUT )
    {
        pair->pctx->state = ST_ERROR;
        pair->pctx->ecode = 5;
        return 0;
    }
    
    return 1;
    
}


int  mtask_get_node_type( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    mtask_start_pair_t * pair;
    uint8_t  tary[4];
    
printf( "cmd = %X : ", cmd );
dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pair );

    /**/
    if ( cmd == ACK_NODE_TYP )
    {
        if ( tlen >= 1 )
        {
            /* 3: 主节点(配对). */
            if ( pdat[0] == 3 )
            {
                task_reactive( tctx, CMD_IO_PAIR, 0, NULL, mtask_get_io_pair, 1000 );
            }
            else
            {
                tary[0] = 3;
                task_reactive( tctx, CMD_NODE_TYP, 1, tary, mtask_set_node_type, 2000 );
            }

            return 1;
        }
    }
    
    if ( cmd == ACK_TMR_OUT )
    {
        pair->pctx->state = ST_ERROR;
        pair->pctx->ecode = 4;
        return 0;
    }
    
    return 1;
    
}


int  mthrd_task_start_pair( mthrd_context_t * pctx )
{
    int  iret;
    intptr_t  tctx;
    mtask_start_pair_t * pair;
    
    /**/
    iret = task_init( pctx->pevbase, pctx->uctx, sizeof(mtask_start_pair_t), &tctx );
    if ( 0 != iret )
    {
        return 3;
    }

    /**/
    task_getptr( tctx, (void **)&pair );
    pair->pctx = pctx;
    
    /**/
    task_active( tctx, CMD_NODE_TYP, 0, NULL, mtask_get_node_type, 1000 );
    return 0;

}



typedef struct _tag_mtask_start_check
{
    mthrd_context_t * pctx;
    uint8_t  saddr[6];
    uint8_t  taddr[6];
    uint8_t  thaddr[6];
    
} mtask_start_check_t;



int  mtask_get_dest_host( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    mtask_start_check_t * pchk;

printf( "cmd = %X : ", cmd );
dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pchk );

    /**/
    if ( cmd == ACK_GET_DHOST )
    {
        if ( tlen >= 6 );
        {
            memcpy( pchk->thaddr, pdat, 6 );
            memcpy( pchk->pctx->saddr, pchk->saddr, 6 );
            memcpy( pchk->pctx->taddr, pchk->taddr, 6 );

            if ( 0 == memcmp( pchk->taddr, pchk->thaddr, 6) )
            {
                pchk->pctx->state = ST_PAIRED;
                pchk->pctx->ecode = 0;
            }
            else
            {
                pchk->pctx->state = ST_UNPAIR;
                pchk->pctx->ecode = 0;
            }
            
            return 0;
        }
    }
    
    /**/
    if ( cmd == ACK_TMR_OUT )
    {
        pchk->pctx->state = ST_ERROR;
        pchk->pctx->ecode = 2;
        return 0;
    }

    return 1;

}



int  mtask_get_dest_addr( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    mtask_start_check_t * pchk;

printf( "cmd = %X : ", cmd );
dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pchk );

    /**/
    if ( cmd == ACK_GET_DADR )
    {
        if ( tlen >= 6 );
        {
            memcpy( pchk->taddr, pdat, 6 );

            /**/
            task_reactive( tctx, CMD_GET_DHOST, 0, NULL, mtask_get_dest_host, 1000 );
        }
    }

    /**/
    if ( cmd == ACK_TMR_OUT )
    {
        pchk->pctx->state = ST_ERROR;
        pchk->pctx->ecode = 2;
        return 0;
    }

    return 1;

}


int  mtask_get_addr( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    mtask_start_check_t * pchk;

printf( "cmd = %X : ", cmd );
dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pchk );

    /**/
    if ( cmd == ACK_GET_ADR )
    {
        if ( tlen >= 6 );
        {
            memcpy( pchk->saddr, pdat, 6 );

            /**/
            task_reactive( tctx, CMD_GET_DADR, 0, NULL, mtask_get_dest_addr, 1000 );
        }
    }
    
    /**/
    if ( cmd == ACK_TMR_OUT )
    {
        pchk->pctx->state = ST_ERROR;
        pchk->pctx->ecode = 1;
        return 0;
    }
    
    return 1;
    
}


int  mtask_wait_ready( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat );


int  mtask_wait_reboot( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    if ( cmd == ACK_REBOOT )
    {
        task_active( tctx, CMD_NULL, 0, NULL, mtask_wait_ready, 6000 );
    }

    return 1;
}


int  mtask_wait_ready( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    uint8_t  tary[64];

    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

#if 0
    if ( cmd == EVT_LDR_INFO )
    {
        tary[0] = 0;
        task_reactive( tctx, CMD_REBOOT, 1, tary, mtask_wait_reboot, 1000 );
    }
#endif

    if ( cmd == ACK_TMR_OUT )
    {
        task_reactive( tctx, CMD_GET_ADR, 0, NULL, mtask_get_addr, 1000 );
    }
    
    return 1;
}


int  mthrd_task_start_check( mthrd_context_t * pctx )
{
    int  iret;
    intptr_t  tctx;
    mtask_start_check_t * pchk;
    
    /**/
    iret = task_init( pctx->pevbase, pctx->uctx, sizeof(mtask_start_check_t), &tctx );
    if ( 0 != iret )
    {
        return 3;
    }

    /**/
    task_getptr( tctx, (void **)&pchk );
    pchk->pctx = pctx;
    
    /**/
    task_active( tctx, CMD_NULL, 0, NULL, mtask_wait_ready, 20000 );
    return 0;
    
}


typedef struct _tag_mtask_data_request
{
    mthrd_context_t * pctx;
    m4bus_req_t  treq;
    int  stage;
    uint8_t  tary[6+6];
    
} mtask_data_request_t;

void  mthrd_dgram_reply_array( mthrd_context_t * pctx, m4bus_req_t * preq, int tlen, uint8_t * pdat );
void  mthrd_dgram_reply_code( mthrd_context_t * pctx, m4bus_req_t * preq, uint16_t code );

int  mtask_data_trans( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    mtask_data_request_t * pdrq;

printf( "cmd = %X : ", cmd );
dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pdrq );

    /**/
    if ( cmd == ACK_SEND_DAT )
    {
        if ( tlen >= 1 )
        {
            if ( pdat[0] == 0 )
            {
                pdrq->stage = 1;
            }
            else
            {
                /* retry? */
                mthrd_dgram_reply_code( pdrq->pctx, &(pdrq->treq), 0xff01 );                
                return 0;
            }
        }
    }

    /**/
    if ( cmd == EVT_SEND_DAT )
    {
        if ( tlen >= 1 )
        {
            if ( pdat[0] == 0 )
            {
                pdrq->stage = 2;
                return 0;
            }
            else
            {
                /* retry? */
                mthrd_dgram_reply_code( pdrq->pctx, &(pdrq->treq), 0xff02 );
                return 0;
            }
        }
    }

#if 0
    m4bus_rsp_t * prsp;

    /**/
    if ( cmd == ACK_RECV_DAT )
    {
        if ( tlen < 6 )
        {
            return 1;
        }

        /**/
        if ( 0 != memcmp( pdat, pdrq->pctx->taddr, 6) )
        {
            return 2;
        }

        /**/
        if ( (tlen - 6) < sizeof(m4bus_rsp_t) )
        {
            return 3;
        }

        /**/
        prsp = (m4bus_rsp_t *)(pdat + 6);
        
        if ( (prsp->bus != pdrq->treq.bus) || (prsp->addr != pdrq->treq.addr) || (prsp->reg != pdrq->treq.reg) )
        {
            return 4;
        }

        /**/
        if ( 0 != prsp->uret )
        {
            mthrd_dgram_reply_code( pdrq->pctx, &(pdrq->treq), prsp->uret );
        }
        else
        {
            mthrd_dgram_reply_array( pdrq->pctx, &(pdrq->treq), tlen-6-sizeof(m4bus_rsp_t), prsp->tary );
        }

        return 0;
        
    }
#endif

    /**/
    if ( cmd == ACK_TMR_OUT )
    {
        printf( "data, trans, timeout \n");
        mthrd_dgram_reply_code( pdrq->pctx, &(pdrq->treq), 0xff03 );
        return 0;
    }

    return 1;
    
}


int  mthrd_task_data_request( mthrd_context_t * pctx, m4bus_req_t * preq )
{
    int  iret;
    intptr_t  tctx;
    mtask_data_request_t * pdrq;

    /**/
    iret = task_init( pctx->pevbase, pctx->uctx, sizeof(mtask_data_request_t), &tctx );
    if ( 0 != iret )
    {
        return 3;
    }

    /**/
    task_getptr( tctx, (void **)&pdrq );
    pdrq->pctx = pctx;
    pdrq->stage = 0;
    memcpy( &(pdrq->treq), preq, sizeof(m4bus_req_t) );

    /**/
    pdrq->tary[0] = pdrq->tary[1] = pdrq->tary[2] = 0xEE;
    pdrq->tary[3] = pdrq->tary[4] = pdrq->tary[5] = 0xEE;
    
    pdrq->tary[6] = preq->bus;
    pdrq->tary[7] = preq->addr;
    pdrq->tary[8] = (uint8_t)(preq->reg & 0xFF);
    pdrq->tary[9] = (uint8_t)(preq->reg >> 8);
    pdrq->tary[10] = (uint8_t)(preq->num & 0xFF);
    pdrq->tary[11] = (uint8_t)(preq->num >> 8);
    
    /**/
    task_active( tctx, CMD_SEND_DAT, 12, pdrq->tary, mtask_data_trans, 3000 );
    return 0;
    
}

#if 0
#endif

void  mthrd_dgram_reply_array( mthrd_context_t * pctx, m4bus_req_t * preq, int tlen, uint8_t * pdat )
{
    m4bus_rsp_t * prsp;

    /**/
    prsp = alloca( sizeof(m4bus_rsp_t) + tlen );

    /**/
    prsp->bus = preq->bus;
    prsp->addr = preq->addr;
    prsp->reg = preq->reg;
    prsp->uret = 0;
    memcpy( prsp->tary, pdat, tlen );
    
    /**/
    pctx->pintf->pkti_send( pctx->dgctx, sizeof(m4bus_rsp_t) + tlen, prsp );
    return;
}


void  mthrd_dgram_reply_state( mthrd_context_t * pctx, m4bus_req_t * preq )
{
    m4bus_rsp_t * prsp;

    /**/
    prsp = alloca( sizeof(m4bus_rsp_t) + 8 );

    /**/
    prsp->bus = preq->bus;
    prsp->addr = preq->addr;
    prsp->reg = preq->reg;
    prsp->uret = 0;
    prsp->tary[0] = (uint8_t)pctx->state;
    prsp->tary[1] = (uint8_t)pctx->ecode;

    if ( ST_PAIRED == pctx->state )
    {
        memcpy( &(prsp->tary[2]), pctx->taddr, 6 );
        /**/
        pctx->pintf->pkti_send( pctx->dgctx, sizeof(m4bus_rsp_t) + 8, prsp );
        
    }
    else
    {
        /**/
        pctx->pintf->pkti_send( pctx->dgctx, sizeof(m4bus_rsp_t) + 2, prsp );
    }
    
    return;
    
}


void  mthrd_dgram_reply_code( mthrd_context_t * pctx, m4bus_req_t * preq, uint16_t code )
{
    m4bus_rsp_t  trsp;

    trsp.bus = preq->bus;
    trsp.addr = preq->addr;
    trsp.reg = preq->reg;
    trsp.uret = code;

    /**/
    pctx->pintf->pkti_send( pctx->dgctx, sizeof(m4bus_rsp_t), &trsp );
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
    if ( (preq->bus == 0) && (preq->addr == 0) )
    {
        switch( preq->reg )
        {
        case 0:
            /**/
            mthrd_dgram_reply_state( pctx, preq );
            break;

        case 1:
            /**/
            mthrd_dgram_reply_code( pctx, preq, 0 );
            mthrd_task_start_pair( pctx );
            break;
        }
    }
    else
    {
        if ( ST_PAIRED != pctx->state )
        {
            mthrd_dgram_reply_code( pctx, preq, 0x1 );
        }
        else if ( 0 != pctx->dtask )
        {
            mthrd_dgram_reply_code( pctx, preq, 0x2 );
        }
        else
        {
            /**/
            mthrd_task_data_request( pctx, preq );
        }
    }
    
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


void  mthrd_inter_udata_cbk( intptr_t arg, int tlen, void * pdat )
{
    mthrd_context_t * pctx;
    uint8_t * pu8;
    
    /**/
    pctx = (mthrd_context_t *)arg;

    /**/
    if ( ST_PAIRED != pctx->state )
    {
        /* todo: log this event.. */
        return;
    }

    if ( tlen <= 6 )
    {
        return;
    }
    
    /* 比较地址是否是配对时的目的端. */
    if ( 0 != memcmp( pctx->taddr, pdat, 6) )
    {
        return;
    }

    /**/
    pu8 = (uint8_t *)pdat;
    pu8 = pu8 + 6;
    tlen = tlen - 6;

    /**/
    printf( "recv uart data, %d\n", tlen );
    pctx->pintf->pkti_send( pctx->dgctx, tlen, pu8 );
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
    iret = nuart_init( pctx->pevbase, 11, &(pctx->uctx) );
    if ( 0 != iret )
    {
        printf( "uart init ret = %d\n", iret );
        return 3;
    }

    /* data call back.. */
    nuart_set_data_callbk( pctx->uctx, mthrd_inter_udata_cbk, (intptr_t)pctx );
    
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
    pctx->state = ST_UNKNOW;
    pctx->ecode = 0;
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
    int  iret;
    mthrd_context_t * pctx;

    /**/
    pctx = (mthrd_context_t *)ctx;

    /**/
    iret = mthrd_task_start_check( pctx );
    if ( 0 != iret )
    {
        return 1;
    }
    
    // 开始主程序的循环
    event_base_dispatch( pctx->pevbase );
    return 0;
}



