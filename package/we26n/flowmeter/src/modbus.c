

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <sys/time.h>

#if 0
#define container_of(ptr, type, member) \
    (type *)((char *)(ptr) - (char *) &((type *)0)->member)
#endif

#include <libubox/list.h>

#include "myuart.h"
#include "modbus.h"


typedef struct _tag_modbus_req
{
    struct list_head  node;

    /**/
    uint64_t  usec_send;

    /**/
    uint8_t  addr;
    uint16_t  reg;
    uint16_t  num;

    /**/
    modbus_cb_func  func;
    intptr_t  arg;
        
} modbus_req_t;


/**/
typedef struct _tag_modbus_context
{   
    /**/
    struct list_head  reqlist;
    
    /* uart handler */
    intptr_t  uctx;

    /**/
    int  tsize;
    int  toffs;
    uint8_t  tbuf[4];
    
} modbus_context_t;



uint64_t  tu_get_usec( void )
{	
    uint64_t  temp;	
    struct timeval  mtv;
    
    gettimeofday( &mtv, NULL );	
    temp = mtv.tv_sec;	
    temp = temp * 1000 * 1000;	
    temp = temp + mtv.tv_usec;
    
    /**/
    return  temp;
}



uint8_t auchCRCHi[] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

uint8_t auchCRCLo[] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};


uint16_t  modbus_crc16( uint8_t * puchMsg, int tlen )
{
    uint8_t  uchCRCHi=0xFF;  /* 高CRC字节初始化 */ 
    uint8_t  uchCRCLo=0xFF;  /* 低CRC 字节初始化 */ 
    uint32_t  uIndex;        /* CRC循环中的索引 */ 
    
    while ( tlen--)
    {
        uIndex = uchCRCHi^*puchMsg++;
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
        uchCRCLo = auchCRCLo[uIndex];
    }

    /**/
    uIndex = uchCRCHi;
    uIndex = uIndex << 8;
    uIndex = uIndex | uchCRCLo;
    return (uint16_t)uIndex;
}


int  modbus_set_uartctx( intptr_t ctx, intptr_t uctx )
{
    modbus_context_t * pctx;

    /**/
    pctx = (modbus_context_t *)ctx;

    /**/
    pctx->uctx = uctx;
    return 0;
}


static int  modbus_internal_send( modbus_context_t * pctx, modbus_req_t * preq )
{
    uint16_t  temp;
    uint8_t  pdat[8];
    
    /**/
    pdat[0] = preq->addr;
    pdat[1] = 0x3;

    pdat[2] = (uint8_t)(preq->reg >> 8);
    pdat[3] = (uint8_t)(preq->reg & 0xFF);

    pdat[4] = (uint8_t)(preq->num >> 8);
    pdat[5] = (uint8_t)(preq->num & 0xFF);

    /**/
    temp = modbus_crc16( pdat, 6 );
    pdat[6] = (uint8_t)(temp >> 8);
    pdat[7] = (uint8_t)(temp & 0xFF);    

    /**/
    myuart_send( pctx->uctx, 8, pdat );
    preq->usec_send = tu_get_usec();
    
    return 0;
    
}




static int  modbus_internal_trynew( modbus_context_t * pctx )
{
    modbus_req_t * preq;    
    
    /**/
    if ( list_empty( &(pctx->reqlist) ) )
    {
        return 0;
    }
    
    /**/
    preq = list_first_entry( &(pctx->reqlist), modbus_req_t, node );
    printf( "try new %p \n", preq );
    modbus_internal_send( pctx, preq );
    pctx->toffs = 0;
    return 0;
}


static int  modbus_internal_timeout( modbus_context_t * pctx )
{
    uint64_t  temp;
    modbus_req_t * preq;
    
    /**/
    if ( list_empty( &(pctx->reqlist) ) )
    {
        return 0;
    }
    
    /**/
    temp = tu_get_usec();
    preq = list_first_entry( &(pctx->reqlist), modbus_req_t, node );
    
    /* wait 10ms, timeout  */
    if ( (temp - preq->usec_send) > 100000 )
    {
        printf( "timeout delete, addr = %d \n", preq->addr );

        /**/
        list_del( &(preq->node) );
        free( preq );
        
        /**/
        modbus_internal_trynew( pctx );
    }
    
    /**/
    return 0;
    
}


int  modbus_send_req( intptr_t ctx, uint8_t addr, uint16_t reg, uint16_t num, modbus_cb_func func, intptr_t arg )
{
    modbus_req_t * preq;
    modbus_context_t * pctx;

    /**/
    pctx = (modbus_context_t *)ctx;

    /**/
    if ( 0 == pctx->uctx )
    {
        return 1;
    }
    
    /**/
    preq = (modbus_req_t *)malloc( sizeof(modbus_req_t) );
    if ( NULL == preq )
    {
        return 3;
    }

    /**/
    preq->addr = addr;
    preq->reg = reg;
    preq->num = num;
    preq->func = func;
    preq->arg = arg;
    
    /* empty -> first entry */
    if ( list_empty( &(pctx->reqlist) ) )
    {
        modbus_internal_send( pctx, preq );
        pctx->toffs = 0;

        /**/
        printf( "add head to addr %d\n", preq->addr );
        list_add_tail( &(preq->node), &(pctx->reqlist) );

    }
    else
    {
        /**/
        printf( "add tail to addr %d\n", preq->addr );
        list_add_tail( &(preq->node), &(pctx->reqlist) );

        /**/
        modbus_internal_timeout( pctx );
    }

    /**/
    
    return 0;
    
}



extern  void  dump_hex( const unsigned char * ptr, size_t  len );

int  modbus_recv_decode( intptr_t ctx, int tlen, uint8_t * pdat )
{
    uint32_t  temp;
    uint32_t  crcs;
    uint8_t  dat;
    int  count;
    int  tsize;
    modbus_req_t * preq;    
    modbus_context_t * pctx;

    /**/
    pctx = (modbus_context_t *)ctx;

    /**/
    if ( list_empty( &(pctx->reqlist) ) )
    {
        return 0;
    }
    
    /**/
    preq = list_first_entry( &(pctx->reqlist), modbus_req_t, node );

#if 1

    {
        printf( "recv from addr %d :", preq->addr );
        dump_hex( pdat, tlen );
    }
    
#endif

    /**/
    memcpy( &(pctx->tbuf[pctx->toffs]), pdat, tlen );
    tsize = pctx->toffs + tlen;

    /**/
    while( pctx->toffs < tsize )
    {
        /**/
        dat = pctx->tbuf[pctx->toffs];

        /**/
        if ( pctx->toffs == 0 )
        {
            /* address */
            if ( dat == preq->addr )
            {
                pctx->toffs += 1;
                continue;
            }

            goto next_shift;
        }

        /**/
        if ( pctx->toffs == 1 )
        {
            /* function, fix 3 */
            if ( dat == 3 )
            {
                pctx->toffs += 1;
                continue;
            }

            goto next_shift;
        }


        if ( pctx->toffs == 2 )
        {
            /* byte count */
            if ( (dat < 2) && (dat > (pctx->tsize - 5)) )
            {
                /* too long not fit in buffer, notify to user? */            
                goto next_shift;
            }
            
            pctx->toffs += 1;
            continue;
        }

        /**/
        count = pctx->tbuf[2];
        count += 4;
        if ( pctx->toffs < count )
        {
             pctx->toffs += 1;
             continue;
        }
        
        /* check crc */
        temp = modbus_crc16( &(pctx->tbuf[0]), (pctx->toffs - 1) );
        
        /**/
        crcs = pctx->tbuf[pctx->toffs-1];
        crcs = crcs << 8;
        crcs = crcs | pctx->tbuf[pctx->toffs];

#if 0
        /**/
        dump_hex( &(pctx->tbuf[0]), (pctx->toffs + 1) );
        printf( "crc = %08x, crc = %08x\n", temp, crcs );
#endif
        
        /**/
        if ( crcs == temp )
        {
            /**/
            preq->func( preq->arg, (pctx->toffs - 1), &(pctx->tbuf[0]) );
            
            /**/
            list_del( &(preq->node) );
            free( preq );

            /**/
            modbus_internal_trynew( pctx );
            return 0;
        }
        
next_shift:
        if ( tsize <= 1 ) 
        {
            break;
        }

        
        memmove( &(pctx->tbuf[0]), &(pctx->tbuf[1]), tsize-1 );
        pctx->toffs = 0;
        tsize = tsize - 1;
        
    }
    
    /**/
    modbus_internal_timeout( pctx );
    return 0;
    
}



int  modbus_proc_timeout( intptr_t ctx )
{
    modbus_context_t * pctx;

    /**/
    pctx = (modbus_context_t *)ctx;

    /**/
    modbus_internal_timeout( pctx );
    return 0;
}



int  modbus_init( int tsize, intptr_t * pret )
{
    modbus_context_t * pctx;

    /**/
    pctx = (modbus_context_t *)malloc( sizeof(modbus_context_t) + tsize );
    if ( NULL == pctx )
    {
        return 1;
    }

    /**/
    memset( pctx, 0, sizeof(modbus_context_t) + tsize );
    pctx->tsize = tsize;
    pctx->toffs = 0;
    
    /**/
    pctx->uctx = 0;

    /**/
    INIT_LIST_HEAD( &(pctx->reqlist) );
    
    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}


int  modbus_fini( intptr_t  ctx )
{
    modbus_context_t * pctx;

    /**/
    pctx = (modbus_context_t *)ctx;

    /**/
    free( pctx );
    return 0;
}





int32_t  modbus_conv_long( uint8_t * puc )
{
    int32_t  temp;

    /**/
    temp = puc[2];
    temp = temp << 8;
    temp = temp | puc[3];
    temp = temp << 8;
    temp = temp | puc[0];
    temp = temp << 8;
    temp = temp | puc[1];

    /**/
    return temp;
}


float  modbus_conv_real4( uint8_t * puc )
{
    volatile float  temp;
    uint32_t * pu32;

    /**/
    pu32 = (uint32_t *)&temp;

    /**/
    *pu32 = puc[2];
    *pu32 <<= 8;
    *pu32 |= puc[3];
    *pu32 <<= 8;
    *pu32 |= puc[0];
    *pu32 <<= 8;
    *pu32 |= puc[1];

    /**/
    return temp;
}



