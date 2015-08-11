
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

#include "myuart.h"


typedef struct _tag_uart_ctx
{
	int  sno;
	int  fd;

	uartcb_func  func;
	intptr_t  arg;

	/**/
	int  padcnt;
	uint8_t  tpad[8];
	
} uart_ctx_t;


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



int  myuart_init( int sno, intptr_t * pret )
{
	int  iret;
	int  fd;
	struct termios  settings;
	char tstr[64];
	uart_ctx_t * pctx;


	/**/
	sprintf( tstr, "/dev/ttyS%d", sno );
    
	/**/
	fd = open( tstr, O_RDWR | O_NOCTTY | O_NONBLOCK );
	if ( -1 == fd )
	{
		return 1;
	}
	
	/**/
	memset( &settings, 0, sizeof(settings));

	tcgetattr(fd, &settings );
	settings.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
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
	pctx = (uart_ctx_t *)malloc( sizeof(uart_ctx_t) );
	if ( NULL == pctx )
	{
		close( fd );
		return 3;
	}

	/**/
	pctx->sno = sno;
	pctx->fd = fd;
	pctx->func = NULL;
	pctx->padcnt = 0;

	/**/
	*pret = (intptr_t)pctx;
	return 0;
	
}


/* 清空 input buffer.  */
int  myuart_clear( intptr_t ctx )
{
	uart_ctx_t * pctx;

	/**/
	pctx = (uart_ctx_t *)ctx;

	/**/
	tcflush( pctx->fd, TCIFLUSH );
	return 0;
}



int  myuart_send( intptr_t ctx, int tlen, uint8_t * pdat )
{
	uart_ctx_t * pctx;

	/**/
	pctx = (uart_ctx_t *)ctx;
    
    /**/
	write( pctx->fd, pdat, tlen );
	return 0;
}


int  myuart_set_callback( intptr_t ctx, uartcb_func func, intptr_t arg )
{
	uart_ctx_t * pctx;

	/**/
	pctx = (uart_ctx_t *)ctx;

	/**/
	pctx->arg = arg;
	pctx->func = func;
	
	return 0;
}




/*
 * 0, 没有数据了, poll and tray again.
 * 1, 返回 0 长度字节.
 * 2, 其他错误, 比如 usb 串口拔出.
 */

int  myuart_run( intptr_t ctx )
{
	int  iret;
	uart_ctx_t * pctx;
	uint8_t  tbuf[64];
	
	/**/
	pctx = (uart_ctx_t *)ctx;

	/**/
	while (1)
	{
		iret = read( pctx->fd, &(tbuf[0]), 64 );

	    if ( iret < 0 )
	    {
	        if ( (errno == EAGAIN) || (errno == EWOULDBLOCK) )
	        {
	            return 0;
	        }
	        else
	        {
	            return 2;
	        }
	    }

	    if ( iret == 0 )
	    {
	    	return 1;
	    }

#if 1
        dump_hex( tbuf, iret );
#endif

        /**/
        if ( NULL != pctx->func )
        {
		    pctx->func( pctx->arg, iret, &(tbuf[0]) );
		}
	    
	}

}



int  myuart_get_fd( intptr_t ctx, int * pfd )
{
	uart_ctx_t * pctx;

	
	/**/
	pctx = (uart_ctx_t *)ctx;

	/**/
	*pfd = pctx->fd;
	return 0;
}


