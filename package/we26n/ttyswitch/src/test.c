

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


typedef struct _tag_test_context
{
    intptr_t  uctx;
    intptr_t  mctx0;    
    intptr_t  mctx1;
    intptr_t  mctx2;
    intptr_t  mctx3;
    
} test_context_t;


void  test_nuart_callbk( intptr_t arg, int tidx, int tlen, void * pdat )
{
    test_context_t * pctx;

    /**/
    pctx = (test_context_t *)arg;

    /**/
    printf( "uart recv : %d : ", tidx );
    dump_hex( pdat, tlen );
    
    switch ( tidx )
    {
    case 0:
        ptmx_send( pctx->mctx0, tlen, pdat );
        break;
    case 1:
        ptmx_send( pctx->mctx1, tlen, pdat );
        break;
    case 2:
        ptmx_send( pctx->mctx2, tlen, pdat );
        break;
    case 3:
        ptmx_send( pctx->mctx3, tlen, pdat );
        break;
        
    default:
        break;
    }

    return;
    
}


void  test_ptmx0_callbk( intptr_t arg, int tlen, void * pdat )
{
    test_context_t * pctx;

    /**/
    pctx = (test_context_t *)arg;

    
    /**/
    printf( "ptmx 0 recv : " );
    dump_hex( pdat, tlen );

    /**/
    nuart_send( pctx->uctx, 0, tlen, pdat );
    return;
}


void  test_ptmx1_callbk( intptr_t arg, int tlen, void * pdat )
{
    test_context_t * pctx;

    /**/
    pctx = (test_context_t *)arg;

    
    /**/
    printf( "ptmx 1 recv : " );
    dump_hex( pdat, tlen );

    /**/
    nuart_send( pctx->uctx, 1, tlen, pdat );
    return;
}


void  test_ptmx2_callbk( intptr_t arg, int tlen, void * pdat )
{
    test_context_t * pctx;

    /**/
    pctx = (test_context_t *)arg;

    
    /**/
    printf( "ptmx 2 recv : " );
    dump_hex( pdat, tlen );

    /**/
    nuart_send( pctx->uctx, 2, tlen, pdat );
    return;
}


void  test_ptmx3_callbk( intptr_t arg, int tlen, void * pdat )
{
    test_context_t * pctx;

    /**/
    pctx = (test_context_t *)arg;

    
    /**/
    printf( "ptmx 3 recv : " );
    dump_hex( pdat, tlen );

    /**/
    nuart_send( pctx->uctx, 3, tlen, pdat );
    return;
}


int  test_init( struct event_base * pevbase )
{
    int  iret;
    test_context_t * pctx;

    /**/
    pctx = (test_context_t *)malloc( sizeof(test_context_t) );
    if (NULL == pctx )
    {
        return 1;
    }
    
    /**/
    iret = nuart_init( pevbase, 0, &(pctx->uctx) );
    if ( 0 != iret )
    {
        printf( "uart init ret = %d\n", iret );
        return 2;
    }

    /**/
    iret = ptmx_init( pevbase, 10, &(pctx->mctx0) );
    if ( 0 != iret )
    {
        printf( "ptmx0 init ret = %d\n", iret );
        return 3;
    }

    /**/
    iret = ptmx_init( pevbase, 11, &(pctx->mctx1) );
    if ( 0 != iret )
    {
        printf( "ptmx1 init ret = %d\n", iret );
        return 3;
    }

    /**/
    iret = ptmx_init( pevbase, 12, &(pctx->mctx2) );
    if ( 0 != iret )
    {
        printf( "ptmx2 init ret = %d\n", iret );
        return 3;
    }

    /**/
    iret = ptmx_init( pevbase, 13, &(pctx->mctx3) );
    if ( 0 != iret )
    {
        printf( "ptmx3 init ret = %d\n", iret );
        return 3;
    }
    
    /**/
    nuart_set_callbk( pctx->uctx, test_nuart_callbk, (intptr_t)pctx );
    ptmx_set_callbk( pctx->mctx0, test_ptmx0_callbk, (intptr_t)pctx );
    ptmx_set_callbk( pctx->mctx1, test_ptmx1_callbk, (intptr_t)pctx );
    ptmx_set_callbk( pctx->mctx2, test_ptmx2_callbk, (intptr_t)pctx );
    ptmx_set_callbk( pctx->mctx3, test_ptmx3_callbk, (intptr_t)pctx );    

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


