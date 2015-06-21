
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/time.h>

#include <syslog.h>


#include "modbus.h"
#include "myuart.h"


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




int  test_modbus_cbk( intptr_t arg, int tlen, void * pdat )
{
    dump_hex( pdat, tlen );
    return 0;
}


int  test_send_query( intptr_t  uctx )
{
    int  tlen;
    uint8_t  tary[16];

    /**/
    modbus_rreg_enc( 0x17, 0x4, 16, tary, &tlen );
    dump_hex( tary, tlen );
    
    /**/
    myuart_send( uctx, tlen, tary );
    return 0;
}

int  countt = 0;

int  test_aux_loop( void * pctx )
{
    int  iret;
    int  i;
    int  num;
    int  ufd;
    intptr_t  uctx;
    intptr_t  mctx;
    
	int  epfd;
	struct epoll_event  tevent;
	struct epoll_event  * pevent;

    /**/
    iret = modbus_init( 100, &mctx );
    if ( 0 != iret )
    {
        return 11;
    }

    /**/
    modbus_set_callback( mctx, test_modbus_cbk, 0 );
    
    /**/
    iret = myuart_init( 0, &uctx );
    if ( 0 != iret )
    {
        return 1;
    }

    /**/
    myuart_set_callback( uctx, (uartcb_func)modbus_recv_dec, mctx );
    myuart_get_fd( uctx, &ufd );

	/* epoll */
	epfd = epoll_create( 5 );
	if ( -1 == epfd )
	{
		return 2;
	}
	
	/* uart fd to epoll */	
	tevent.events = EPOLLIN;
	tevent.data.fd = ufd;
	iret = epoll_ctl( epfd, EPOLL_CTL_ADD, ufd, &tevent );
	if ( 0 != iret )
	{
		return 3;
	}

	/**/	
	pevent = malloc( 3 * sizeof(struct epoll_event) );
	memset( pevent, 0, 3 * sizeof(struct epoll_event) );
	
	while( 1 )
	{
		num = epoll_wait( epfd, pevent, 3, 500 );
		
		if ( num < 0 )
		{
			/* epoll error. */
			return 7;
		}
		
		if ( num == 0 )
		{
		    if ( countt < 10 )
		    {
		        test_send_query( uctx );
		        countt += 1;
		    }
		    
		    continue;
		}
		
        for ( i=0; i<num; i++ )
        {
            if ( pevent[i].data.fd == ufd )
            {
                iret = myuart_run( uctx );
                if ( 0 != iret )
                {
                    return 9;
                }
            }
        }
        
    }
    
}


void *  test_aux_thread( void * arg )
{
    int  iret;

    /**/
    openlog( "flowmeter", LOG_PID, LOG_USER);

    /**/
    iret =  test_aux_loop( arg );
    printf( "test_aux_loop exit = %d\n", iret );    
    return NULL;
}


