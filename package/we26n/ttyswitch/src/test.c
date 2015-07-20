

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

#include "newuart.h"
#include "myptmx.h"


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



void  test_nuart_callbk( intptr_t arg, int tidx, int tlen, void * pdat )
{
    printf( "uart recv : %d : ", tidx );
    dump_hex( pdat, tlen );
    ptmx_send( arg, tlen, pdat );
    return;
}

void  test_ptmx0_callbk( intptr_t arg, int tlen, void * pdat )
{
    printf( "ptmx recv : " );
    dump_hex( pdat, tlen );
    nuart_send( arg, 2, tlen, pdat );
    return;
}


int  test_init( struct event_base * pevbase )
{
    int  iret;
    intptr_t  uctx;
    intptr_t  mctx0;

    /**/
    iret = nuart_init( pevbase, 0, &uctx );
    if ( 0 != iret )
    {
        printf( "uart init ret = %d\n", iret );
        return 1;
    }

    /**/
    iret = ptmx_init( pevbase, 10, &mctx0 );
    if ( 0 != iret )
    {
        printf( "ptmx0 init ret = %d\n", iret );
        return 1;
    }

    
    /**/
    nuart_set_callbk( uctx, test_nuart_callbk, mctx0 );
    ptmx_set_callbk( mctx0, test_ptmx0_callbk, uctx );
    
    return 0;
    
}




int  test( void )
{
    int  iret;
    struct event_base * pevbase;

    /**/
    pevbase = event_base_new();
    if (NULL == pevbase)
    {
        return 1;
    }

    /**/
    iret = test_init( pevbase );
    if ( 0 != iret )
    {
        return 2;
    }
    
    // 开始主程序的循环
    event_base_dispatch( pevbase );
    return 0;
    
}



int  main( void )
{
    int  iret;

    iret = test();

    printf( "test return iret = %d\n", iret );
    return 0;
}


