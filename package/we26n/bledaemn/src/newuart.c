
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

void  dump_hex( const unsigned char * ptr, size_t  len );

typedef struct _tag_code_context
{
    /**/
    int  dec_flag;
    int  dec_offs;
    uint8_t  dec_pad[128];

    /**/
    uint8_t  enc_pad[128];

    /**/
    nuart_cbk_func  func;
    intptr_t  arg;

} code_context_t;


int  code_init( intptr_t * pret )
{
    code_context_t * pctx;

    /**/
    pctx = (code_context_t *)malloc( sizeof(code_context_t) );
    if ( NULL == pctx )
    {
        return 1;
    }

    /**/
    pctx->dec_flag = 0;
    pctx->dec_offs = 0;
    pctx->func = NULL;
    pctx->arg = 0;    
    *pret = (intptr_t)pctx;
    return 0;
}



int  code_encode( intptr_t ctx, uint8_t cmd, int tlen, uint8_t * pdat, uint8_t ** ppad, int * pret )
{
    int  i;
    uint8_t * xpad;    
    code_context_t * pctx;

    /**/
    if ( tlen > 64 )
    {
        return 1;
    }

    /**/
    pctx = (code_context_t *)ctx;
    xpad = pctx->enc_pad;
    
    /**/
    xpad[0] = 0xAA;
    xpad[1] = cmd;
    xpad[2] = (uint8_t)tlen;
    
    for ( i=0; i<tlen; i++ )
    {
        xpad[3+i] = pdat[i];
    }
    
    /**/
    *ppad = xpad;
    *pret = 3+tlen;
    return 0;
    
}


int  code_dec_step( code_context_t * pctx, uint8_t tdat )
{
    uint8_t  temp;
    
    switch ( pctx->dec_flag )
    {
    case 0:
        if ( tdat == 0xAA )
        {
            pctx->dec_flag = 1;
            pctx->dec_offs = 1;
            pctx->dec_pad[0] = 0xAA;
        }
        break;

    case 1:
        if ( (((tdat >> 4) ^ tdat) & 0xF) == 0xF )
        {
            pctx->dec_flag = 2;
            pctx->dec_offs = 2;
            pctx->dec_pad[1] = tdat;
        }
        else
        {
            pctx->dec_flag = 0;
        }
        break;

    case 2:
        if ( tdat == 0 )
        {
            if ( NULL != pctx->func )
            {
                pctx->func( pctx->arg, pctx->dec_pad[1], 0, &(pctx->dec_pad[3]) );
            }

            pctx->dec_flag = 0;
        }
        else if ( tdat < 64 )
        {
            pctx->dec_flag = 3;
            pctx->dec_offs = 3;
            pctx->dec_pad[2] = tdat;
        }
        else if ( tdat == 0xAA )
        {
            pctx->dec_flag = 1;
            pctx->dec_offs = 1;
            pctx->dec_pad[0] = 0xAA;
        }
        else
        {
            pctx->dec_flag = 0;        
        }
        
        break;

    case 3:
        pctx->dec_pad[pctx->dec_offs] = tdat;
        pctx->dec_offs += 1;

        /**/
        temp = pctx->dec_pad[2] + 3;

        if (pctx->dec_offs < temp )
        {
            break;
        }
        
        /**/
        if ( NULL != pctx->func )
        {
            pctx->func( pctx->arg, pctx->dec_pad[1], (temp -3), &(pctx->dec_pad[3]) );
        }

        pctx->dec_flag = 0;
        pctx->dec_offs = 0;        
        break;
        
    }

    return 0;
    
}


int  code_decode( intptr_t ctx, int tlen, uint8_t * pdat )
{
    int  i;
    code_context_t * pctx;

    /**/
    pctx = (code_context_t *)ctx;

    /**/
    for ( i=0; i<tlen; i++ )
    {
        code_dec_step( pctx, pdat[i] );
    }
    
    return 0;
}


int  code_set_callbk( intptr_t ctx, nuart_cbk_func func, intptr_t arg )
{
    code_context_t * pctx;

    /**/
    pctx = (code_context_t *)ctx;

    /**/
    if ( func == NULL )
    {
        if ( arg != pctx->arg )
        {
            return 0;
        }
    }

    pctx->func = func;
    pctx->arg = arg;
    return 0;
    
}


typedef struct  _tag_nuart_context
{
    int  ufd;

    /**/
    struct event_base * pevbase;
    struct event * prdevt;
    struct event * pwtevt;

    /**/
    intptr_t  cctx;
    
    /**/
    intptr_t  qctx;
    uint8_t * sptr;
    int  slen;
    
} nuart_context_t;



void  nuart_read_cbk( evutil_socket_t ufd, short event, void * parg )
{
    int  iret;
    nuart_context_t * pctx;
    uint8_t  tbuf[64];

    /**/
    pctx = (nuart_context_t *)parg;

    // printf( "nuart read event %d\n", event  );

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
        code_decode( pctx->cctx, iret, &(tbuf[0]) );
        
    }
    
    return;
    
}


int  nuart_trysend( nuart_context_t * pctx, int tlen, uint8_t * pdat )
{
    int  iret;
    
    /**/
    // printf( "write tlen = %d\n", tlen );
    
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
    sprintf( tstr, "/dev/ttyUSB%d", tno );
    printf( "%s\n", tstr );
    
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

    iret = code_init( &(pctx->cctx) );
    if ( 0 != iret )
    {
        return 6;
    }
    
    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}




int  nuart_send( intptr_t ctx, uint8_t cmd, int tlen, void * pdat )
{
    int  iret;
    int  elen;
    uint8_t * petr;
    nuart_context_t * pctx;

    /**/
    pctx = (nuart_context_t *)ctx;


    if ( tlen > 80 )
    {
        printf( "nuart_send, too large\n" );
        return 3;
    }

    /**/
    iret = code_encode( pctx->cctx, cmd, tlen, (uint8_t *)pdat, &petr, &elen );
    if ( 0 != iret )
    {
        return 4;
    }
    
    dump_hex( petr, elen );

    /**/
    if ( pctx->sptr == NULL )
    {
        /* try send */
        iret = nuart_trysend( pctx, elen, petr );
        if ( iret < 0 )
        {
            /* fail */
            return 5;
        }
        
        if ( iret == elen )
        {
            return 0;
        }
        
        /**/
        msq_enqueue( pctx->qctx, elen-iret, petr+iret );
        event_add( pctx->pwtevt, NULL );
    }
    else
    {
        msq_enqueue( pctx->qctx, elen, petr );
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
    return code_set_callbk( pctx->cctx, func, arg );
}




