
#include <stdio.h>
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

#include "myqueue.h"
#include "newuart.h"


/*
 *
 * Code generated by universal_crc by Danjel McGougan
 *
 * CRC parameters used:
 *   bits:       8
 *   poly:       0x31
 *   init:       0x00
 *   xor:        0x00
 *   reverse:    false
 *   non-direct: false
 *
 * CRC of the string "123456789" is 0xa2
 */

const uint8_t crc_table[256] = {
	0x00, 0x31, 0x62, 0x53, 0xc4, 0xf5, 0xa6, 0x97,
	0xb9, 0x88, 0xdb, 0xea, 0x7d, 0x4c, 0x1f, 0x2e,
	0x43, 0x72, 0x21, 0x10, 0x87, 0xb6, 0xe5, 0xd4,
	0xfa, 0xcb, 0x98, 0xa9, 0x3e, 0x0f, 0x5c, 0x6d,
	0x86, 0xb7, 0xe4, 0xd5, 0x42, 0x73, 0x20, 0x11,
	0x3f, 0x0e, 0x5d, 0x6c, 0xfb, 0xca, 0x99, 0xa8,
	0xc5, 0xf4, 0xa7, 0x96, 0x01, 0x30, 0x63, 0x52,
	0x7c, 0x4d, 0x1e, 0x2f, 0xb8, 0x89, 0xda, 0xeb,
	0x3d, 0x0c, 0x5f, 0x6e, 0xf9, 0xc8, 0x9b, 0xaa,
	0x84, 0xb5, 0xe6, 0xd7, 0x40, 0x71, 0x22, 0x13,
	0x7e, 0x4f, 0x1c, 0x2d, 0xba, 0x8b, 0xd8, 0xe9,
	0xc7, 0xf6, 0xa5, 0x94, 0x03, 0x32, 0x61, 0x50,
	0xbb, 0x8a, 0xd9, 0xe8, 0x7f, 0x4e, 0x1d, 0x2c,
	0x02, 0x33, 0x60, 0x51, 0xc6, 0xf7, 0xa4, 0x95,
	0xf8, 0xc9, 0x9a, 0xab, 0x3c, 0x0d, 0x5e, 0x6f,
	0x41, 0x70, 0x23, 0x12, 0x85, 0xb4, 0xe7, 0xd6,
	0x7a, 0x4b, 0x18, 0x29, 0xbe, 0x8f, 0xdc, 0xed,
	0xc3, 0xf2, 0xa1, 0x90, 0x07, 0x36, 0x65, 0x54,
	0x39, 0x08, 0x5b, 0x6a, 0xfd, 0xcc, 0x9f, 0xae,
	0x80, 0xb1, 0xe2, 0xd3, 0x44, 0x75, 0x26, 0x17,
	0xfc, 0xcd, 0x9e, 0xaf, 0x38, 0x09, 0x5a, 0x6b,
	0x45, 0x74, 0x27, 0x16, 0x81, 0xb0, 0xe3, 0xd2,
	0xbf, 0x8e, 0xdd, 0xec, 0x7b, 0x4a, 0x19, 0x28,
	0x06, 0x37, 0x64, 0x55, 0xc2, 0xf3, 0xa0, 0x91,
	0x47, 0x76, 0x25, 0x14, 0x83, 0xb2, 0xe1, 0xd0,
	0xfe, 0xcf, 0x9c, 0xad, 0x3a, 0x0b, 0x58, 0x69,
	0x04, 0x35, 0x66, 0x57, 0xc0, 0xf1, 0xa2, 0x93,
	0xbd, 0x8c, 0xdf, 0xee, 0x79, 0x48, 0x1b, 0x2a,
	0xc1, 0xf0, 0xa3, 0x92, 0x05, 0x34, 0x67, 0x56,
	0x78, 0x49, 0x1a, 0x2b, 0xbc, 0x8d, 0xde, 0xef,
	0x82, 0xb3, 0xe0, 0xd1, 0x46, 0x77, 0x24, 0x15,
	0x3b, 0x0a, 0x59, 0x68, 0xff, 0xce, 0x9d, 0xac
};


static inline uint8_t crc_next(uint8_t crc, uint8_t data)
{
	return crc_table[crc ^ data];
}


uint8_t crc_calc( size_t len, const uint8_t *data)
{
	uint8_t crc = 0;

	if (len) do {
		crc = crc_next(crc, *data++);
	} while (--len);

	return crc;
}


uint8_t crc_step( uint8_t tcc, uint8_t tdat )
{
	return crc_next( tcc, tdat );
}



typedef struct  _tag_nuart_context
{
    int  ufd;

    /**/
    struct event_base * pevbase;
    struct event * prdevt;
    struct event * pwtevt;
    
    /**/
    nuart_cbk_func  func;
    intptr_t  arg;

    /**/
    intptr_t  qctx;
    uint8_t * sptr;
    int  slen;
    uint8_t  tbuf[150];

    int  rdflag;
    int  rdlen;
    uint8_t  rbuf[150];
    uint8_t  rcrc;
    
} nuart_context_t;


static inline int  tt_decode( nuart_context_t * pctx, uint8_t tdat, int * plen )
{

    /* 原始开始, 或者前者是超长数据错误, 必须找到 0x7E 才能开工. */
    if ( pctx->rdflag == 0 )
    {
        if ( tdat == 0x7E )
        {
            pctx->rdflag = 2;
            pctx->rdlen = 0;
            pctx->rcrc = 0;
        }
        
        return 1;
    }
    
    
    /* 刚刚处理了一个正常的 0x7E 结尾, 
     * 虽然期望着一个 0x7E 做为良好的开始, 
     * 但是如果没有发现 0x7E, 其他数据也认为是数据开始了.
     */
    if ( pctx->rdflag == 1 )
    {
        if ( tdat == 0x7E )
        {
            pctx->rdflag = 2;
            pctx->rdlen = 0;
            pctx->rcrc = 0;
        }
        else if ( tdat == 0x7D )
        {
            pctx->rdflag = 3;
            pctx->rdlen = 0;
            pctx->rcrc = 0;            
        }
        else
        {
            pctx->rdflag = 2;
            pctx->rbuf[0] = tdat;
            pctx->rdlen = 1;
            pctx->rcrc = crc_step( 0, tdat );
        }

        return 2;
        
    }
    
    
    if ( pctx->rdflag == 2 )
    {
        if ( tdat == 0x7E )
        {
            pctx->rdflag = 1;

            /* end */
            if ( pctx->rdlen < 3 )
            {
                return 13;
            }

            if ( pctx->rcrc != 0 )
            {
                return 14;
            }

            /**/
            *plen = pctx->rdlen;
            return 0;

        }

        /* 是否已经是超长错误. */
        if ( pctx->rdlen >= 64 )
        {
            pctx->rdflag = 1;
            return 15;
        }
        
        if ( tdat != 0x7D )
        {
            pctx->rbuf[pctx->rdlen] = tdat;
            pctx->rdlen += 1;
            pctx->rcrc = crc_step( pctx->rcrc, tdat );
        }
        else
        {
            pctx->rdflag = 3;
        }

        return 5;
        
    }
    
        
    if ( pctx->rdflag == 3 )
    {

        /* escape */
        if ( tdat == 0x5E )
        {
            pctx->rbuf[pctx->rdlen] = 0x7E;
            pctx->rdlen += 1;           
            pctx->rdflag = 2;
            pctx->rcrc = crc_step( pctx->rcrc, 0x7E );
            return 6;
        }

        /**/
        if ( tdat == 0x5D )
        {
            pctx->rbuf[pctx->rdlen] = 0x7D;
            pctx->rdlen += 1;
            pctx->rdflag = 2;
            pctx->rcrc = crc_step( pctx->rcrc, 0x7D );            
            return 7;
        }

        if ( tdat == 0x7E )
        {
            pctx->rdflag = 1;
            return 18;
        }
        else
        {
            pctx->rdflag = 0;
            return 19;
        }
        
    }
    
    /* 不应该走到这里 */
    pctx->rdflag = 0;
    return 11;
    
}




int  nuart_decode( nuart_context_t * pctx, int tlen, uint8_t * pdat )
{
    int  i;
    int  iret;
    int  rlen;
    
    /**/
    for ( i=0; i<tlen; i++ )
    {
        iret = tt_decode( pctx, pdat[i], &rlen );
        if ( 0 == iret )
        {
            if ( NULL != pctx->func )
            {
                pctx->func( pctx->arg, pctx->rbuf[rlen-2], rlen-2, pctx->rbuf );
            }
        }
        else
        {
            /**/
            // printf( "encode ret = %02x %d\n", pdat[i], iret );
        }
    }

    return 0;
    
}


void  nuart_read_cbk( evutil_socket_t ufd, short event, void * parg )
{
    int  iret;
    nuart_context_t * pctx;
    uint8_t  tbuf[64];

    /**/
    pctx = (nuart_context_t *)parg;

    //printf( "nuart read event %d\n", event  );

    /**/
    while(1)
    {
		iret = read( pctx->ufd, &(tbuf[0]), 64 );	    
		if ( iret < 0 )
		{	        
		    if ( (errno == EAGAIN) || (errno == EWOULDBLOCK) )	        
		    {	            
    	    	return;
		    }
		    else	        
	    	{
		        return;	       
		    }
		}
        
		if ( iret == 0 )
		{
		    return;
		}
        
		/**/
		nuart_decode( pctx, iret, &(tbuf[0]) );
		
    }
    
    return;
    
}


int  nuart_trysend( nuart_context_t * pctx, int tlen, uint8_t * pdat )
{
    int  iret;
    
    /**/
    iret = write( pctx->ufd, pdat, tlen );
    if ( iret >= 0 )
    {
        return iret;
    }

    /**/
    if ( (errno == EAGAIN) || (errno == EWOULDBLOCK) )
    {
        return 0;
    }
    else
    {
        /**/
        printf( "write uart error :\n" );
        return -1;
    }

}


void  nuart_write_cbk( evutil_socket_t ufd, short event, void * parg )
{
    int  iret;
    nuart_context_t * pctx;

    /**/
    pctx = (nuart_context_t *)parg;

    /**/
    if ( NULL == pctx->sptr )
    {
        iret = msq_dequeue( pctx->qctx, (void **)&(pctx->sptr), &(pctx->slen) );
        if ( 0 != iret )
        {
            pctx->sptr = NULL;        
            event_del( pctx->pwtevt );
            return;
        }
    }
    
    /**/
    while ( 1 )
    {
        iret = nuart_trysend( pctx, pctx->slen, pctx->sptr );

        if ( iret < 0 )
        {
            /* fail */
            event_del( pctx->pwtevt );
            return;
        }
        
        /**/
        if ( iret < pctx->slen )
        {
            pctx->slen -= iret;
            pctx->sptr += iret;
            return;
        }
        
        /**/
        iret = msq_dequeue( pctx->qctx, (void **)&(pctx->sptr), &(pctx->slen) );
        if ( 0 != iret )
        {
            pctx->sptr = NULL;
            event_del( pctx->pwtevt );
            return;
        }
        
    }
    
    return;
    
}


int  nuart_init( struct event_base * pevbase, int tno, intptr_t * pret )
{
    int  iret;
    int  fd;
    struct termios  settings;
    char tstr[64];
    nuart_context_t * pctx;

    /**/
    sprintf( tstr, "/dev/ttyS%d", tno );

    /**/
    fd = open( tstr, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if ( -1 == fd )
    {
        return 1;
    }
    
    /**/
    memset( &settings, 0, sizeof(settings));

    tcgetattr(fd, &settings );
    settings.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    settings.c_iflag = 0;
    settings.c_oflag = 0;
    settings.c_lflag = 0;
    settings.c_cc[VMIN] = 1;      /* block untill n bytes are received */
    settings.c_cc[VTIME] = 0; 
    
    iret = tcsetattr( fd, TCSANOW, &settings );
    if ( -1 == iret )
    {
        close( fd );
        return 2;
    }

    /**/
    pctx = (nuart_context_t *)malloc( sizeof(nuart_context_t ) );
    if ( NULL == pctx )
    {
        close( fd );
        return 3;
    }

    /**/
    pctx->ufd = fd;
    pctx->pevbase = pevbase;
    pctx->func = NULL;
    pctx->arg = 0;

    /**/
    pctx->prdevt = event_new( pevbase, fd, EV_READ|EV_PERSIST, nuart_read_cbk, (void*)pctx );
    if ( pctx->prdevt == NULL )
    {
        free( pctx );
        close( fd );
        return 4;
    }

    /**/
    pctx->pwtevt = event_new( pevbase, fd, EV_WRITE|EV_PERSIST, nuart_write_cbk, (void*)pctx );
    if ( pctx->prdevt == NULL )
    {
        free( pctx );
        close( fd );
        return 4;
    }

    /**/
    event_add( pctx->prdevt, NULL );

    /**/
    pctx->sptr = NULL;
    pctx->slen = 0;
    iret = msq_init( &(pctx->qctx) );
    if ( 0 != iret )
    {
        free( pctx );
        close( fd );
        return 5;    
    }
    
    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}


/**/
#if 1

static inline int  nuart_encode( uint8_t * pbuf, int tidx, int tlen, uint8_t * pdat )
{
    int  k;
    int  i;
    uint8_t crc;
    uint8_t  tstr[4];
    
    /* 先计算 crc8 */
    tidx = tidx & 0x3;
    crc = crc_calc( tlen, pdat );
    crc = crc_step( crc, (uint8_t)tidx );

    /**/
    tstr[0] = (uint8_t)tidx;
    tstr[1] = (uint8_t)crc;
    
    /**/
    pbuf[0] = 0x7e;
    k = 1;
    for ( i=0; i<tlen; i++ )
    {
        if ( pdat[i] == 0x7E )
        {
            pbuf[k] = 0x7D;
            pbuf[k+1] = 0x5E;
            k += 2;
        }
        else if ( pdat[i] == 0x7D )
        {
            pbuf[k] = 0x7D;
            pbuf[k+1] = 0x5D;
            k += 2;
        }
        else
        {
            pbuf[k] = pdat[i];
            k += 1;
        }
    }

    /**/
    for ( i=0; i<2; i++ )
    {
        if ( tstr[i] == 0x7E )
        {
            pbuf[k] = 0x7D;
            pbuf[k+1] = 0x5E;
            k += 2;
        }
        else if ( tstr[i] == 0x7D )
        {
            pbuf[k] = 0x7D;
            pbuf[k+1] = 0x5D;
            k += 2;
        }
        else
        {
            pbuf[k] = tstr[i];
            k += 1;
        }
    }
    
    /**/
    pbuf[k] = 0x7e;
    k += 1;
    return k;
    
}

#else

int  nuart_encode( uint8_t * pbuf, int tidx, int tlen, void * pdat )
{
    memcpy( pbuf, pdat, tlen );
    return tlen;
}

#endif



int  nuart_send( intptr_t ctx, int tidx, int tlen, void * pdat )
{
    int  iret;
    int  elen;
    nuart_context_t * pctx;

    /**/
    pctx = (nuart_context_t *)ctx;

    /**/
    if ( tidx > 3 )
    {
        printf( "nuart send , idx fail = %d\n", tidx);
        return 1;
    }
    
    /**/
    if ( tlen <= 0 )
    {
        return 2;
    }

    if ( tlen > 64 )
    {
        printf( "nuart_send, too large\n" );
        return 3;
    }

    /**/
    elen = nuart_encode( pctx->tbuf, tidx, tlen, (uint8_t *)pdat );
    
    /**/
    if ( pctx->sptr == NULL )
    {
        /* try send */
        iret = nuart_trysend( pctx, elen, pctx->tbuf );
        if ( iret < 0 )
        {
            /* fail */
            return 3;
        }
        
        if ( iret == elen )
        {
            return 0;
        }
        
        /**/
        msq_enqueue( pctx->qctx, elen-iret, pctx->tbuf+iret );
        event_add( pctx->pwtevt, NULL );
    }
    else
    {
        msq_enqueue( pctx->qctx, elen, pctx->tbuf );
    }
    
    /**/
    return 0;
    
}


int  nuart_set_callbk( intptr_t ctx, nuart_cbk_func func, intptr_t arg )
{
    nuart_context_t * pctx;

    /**/
    pctx = (nuart_context_t *)ctx;
    
    /**/
    pctx->func = func;
    pctx->arg = arg;
    return 0;
}




