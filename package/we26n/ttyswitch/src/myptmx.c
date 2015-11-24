
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

#include "myqueue.h"
#include "myptmx.h"

typedef struct  _tag_ptmx_context
{
    int  mfd;

    /**/
    struct event_base * pevbase;
    struct event * prdevt;
    struct event * pwtevt;
    
    /**/
    ptmx_cbk_func  func;
    intptr_t  arg;

    /**/
    intptr_t  qctx;
    uint8_t * sptr;
    int  slen;
    
} ptmx_context_t;


void  ptmx_read_cbk( evutil_socket_t ufd, short event, void * parg )
{
    int  iret;
    ptmx_context_t * pctx;
    uint8_t  tbuf[64];

    /**/
    pctx = (ptmx_context_t *)parg;


    /**/
    while(1)
    {
		iret = read( pctx->mfd, &(tbuf[0]), 64 );	    
		if ( iret < 0 )
		{	        
		    if ( (errno == EAGAIN) || (errno == EWOULDBLOCK) )	        
		    {	            
    	    	return;
		    }
		    else	        
	    	{

                {
                    int  flags;
                    ioctl( pctx->mfd, TIOCGPTN, &flags );
                }
	    	

                printf( "slave close?? \n" );
#if 0
while(1)	    	    
{
    sleep(10);
}
#endif

		        return;
		    }
		}
        
		if ( iret == 0 )
		{
		    return;
		}

		/**/        
		if ( NULL != pctx->func )
		{
		    pctx->func( pctx->arg, iret, &(tbuf[0]) );		
		}
		
    }
    
    return;
    
}


int  ptmx_trysend( ptmx_context_t * pctx, int tlen, uint8_t * pdat )
{
    int  iret;
    
    /**/
    iret = write( pctx->mfd, pdat, tlen );
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
        printf( "write ptmx error :\n" );
        return -1;
    }

}


void  ptmx_write_cbk( evutil_socket_t ufd, short event, void * parg )
{
    int  iret;
    ptmx_context_t * pctx;

    /**/
    pctx = (ptmx_context_t *)parg;

    
    /**/
    while ( 1 )
    {
        iret = ptmx_trysend( pctx, pctx->slen, pctx->sptr );

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


int  ptmx_init( struct event_base * pevbase, int tno, intptr_t * pret )
{
    int  iret;
    int  mfd;
    char  tbuf[100];
    char  cmdn[200];
    ptmx_context_t * pctx;
    struct termios  settings;
    unsigned int fl;
    
    /**/
    mfd = posix_openpt( O_RDWR | O_NOCTTY );
    if ( mfd < 0 )
    {
        return 1;
    }

    /**/
    memset( &settings, 0, sizeof(settings));

    tcgetattr( mfd, &settings );
    settings.c_cflag = B38400 | CS8 | CLOCAL | CREAD;
    settings.c_iflag = 0;
    settings.c_oflag = 0;
    settings.c_lflag = 0;
    settings.c_cc[VMIN] = 1;      /* block untill n bytes are received */
    settings.c_cc[VTIME] = 0; 
    
    iret = tcsetattr( mfd, TCSANOW, &settings );
    if ( -1 == iret )
    {
        close( mfd );
        return 2;
    }

    fl = fcntl( mfd, F_GETFL, 0);
    fl |= O_NONBLOCK;
    fcntl( mfd, F_SETFL, fl);
    
    /**/
    iret = ptsname_r( mfd, tbuf, 90 );
    if ( 0 != iret )
    {
        close( mfd );
        return 3;
    }

    /**/
    printf( "name = %s\n", tbuf );
    sprintf( cmdn, "rm -f /dev/ttyS%d", tno );
    system( cmdn );

    sprintf( cmdn, "ln -s %s /dev/ttyS%d", tbuf, tno );
    system( cmdn );

    /**/
    pctx = (ptmx_context_t *)malloc( sizeof(ptmx_context_t) );
    if ( NULL == pctx )
    {
        close( mfd );
        return 4;
    }

    /**/
    pctx->mfd = mfd;
    pctx->pevbase = pevbase;
    pctx->func = NULL;
    pctx->arg = 0;
    pctx->sptr = NULL;
    pctx->slen = 0;

    /**/
    iret = msq_init( &(pctx->qctx) );
    if ( 0 != iret )
    {
        close( mfd );
        free( pctx );
        return 4;
    }

    /**/
    pctx->prdevt = event_new( pevbase, mfd, EV_READ|EV_PERSIST, ptmx_read_cbk, (void*)pctx );
    if ( pctx->prdevt == NULL )
    {
        free( pctx );
        close( mfd );
        return 4;
    }

    /**/
    pctx->pwtevt = event_new( pevbase, mfd, EV_WRITE|EV_PERSIST, ptmx_write_cbk, (void*)pctx );
    if ( pctx->prdevt == NULL )
    {
        free( pctx );
        close( mfd );
        return 4;
    }

    /**/
    event_add( pctx->prdevt, NULL );

    
    /**/
    iret = grantpt( mfd );
    if ( 0 != iret )
    {
        return 2;
    }

    /**/
    iret = unlockpt( mfd );
    if ( 0 != iret )
    {
        return 3;
    }

    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}




int  ptmx_send( intptr_t ctx, int tlen, void * pdat )
{
    int  iret;
    uint8_t * pu8;
    ptmx_context_t * pctx;

    /**/
    pctx = (ptmx_context_t *)ctx;
    pu8 = (uint8_t *)pdat;

    /**/
    if ( tlen <= 0 )
    {
        return 1;
    }
    
    /**/
    if ( pctx->sptr == NULL )
    {
        /* try send */
        iret = ptmx_trysend( pctx, tlen, pu8 );
        if ( iret < 0 )
        {
            /* fail */
            return 2;
        }
        
        if ( iret == tlen )
        {
            return 0;
        }
        
        /**/
        event_add( pctx->pwtevt, NULL );        
        msq_enqueue( pctx->qctx, tlen-iret, pu8+iret );
        msq_dequeue( pctx->qctx, (void **)&(pctx->sptr), &(pctx->slen) );

    }
    else
    {
        msq_enqueue( pctx->qctx, tlen, pdat );
    }
    
    /**/
    return 0;
    
}


int  ptmx_set_callbk( intptr_t ctx, ptmx_cbk_func func, intptr_t arg )
{
    ptmx_context_t * pctx;

    /**/
    pctx = (ptmx_context_t *)ctx;
    
    /**/
    pctx->func = func;
    pctx->arg = arg;
    return 0;
}



int  ptmx_pktmode( intptr_t ctx, int mode )
{
    ptmx_context_t * pctx;
    
    
    /**/
    pctx = (ptmx_context_t *)ctx;

    /**/
    mode = mode != 0? 1: 0;
    
    /**/
    ioctl( pctx->mfd, TIOCPKT, &mode );
    return 0;

}



static const uint32_t baud_table[31] = {
	0, 50, 75, 110, 134, 150, 200, 300, 
	600, 1200, 1800, 2400, 4800, 9600, 19200, 38400, 
	57600, 115200, 230400, 460800,	500000, 576000, 921600, 1000000, 
	1152000, 1500000, 2000000,	2500000, 3000000, 3500000, 4000000
};


int  ptmx_getspeed( intptr_t ctx, uint32_t * pspeed )
{
    ptmx_context_t * pctx;
	struct termios  settings;
	uint32_t  cbaud;
	
    
    /**/
    pctx = (ptmx_context_t *)ctx;
    
    memset( &settings, 0, sizeof(settings));

    tcgetattr( pctx->mfd, &settings );

	cbaud = settings.c_cflag & CBAUD;

	if ( 0 != (cbaud & CBAUDEX) ) 
	{
		cbaud &= ~CBAUDEX;

		if (cbaud < 1 || (cbaud + 15) > 31)
			cbaud = 0;
		else
			cbaud += 15;
	}

    /**/
	*pspeed = baud_table[cbaud];
	return 0;
	
}




