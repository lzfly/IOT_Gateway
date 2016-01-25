#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>


#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <uv.h>

#include <libubox/list.h>
#include <libubox/blob.h>
#include <libubox/blobmsg.h>
#include <ubusmsg.h>


#include "mmqtt.h"

#include "chgcbk.h"
#include "gateway.h"
#include "iluaexc.h"
#include "isouth.h"
#include "inorth.h"


void  dump_hex( size_t len, uint8_t * ptr )
{
    int  i;
    int  nn;
    int  len2 = len;

    if (len <= 0)
    {
        puts("");
        fflush(stdout);
        return;
    }
    
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




int  test( int argc, char * argv[] )
{
    int  iret;
    uv_loop_t  loop;
    
    /**/
	iret = uv_loop_init( &loop );
	if ( 0 != iret )
	{
		return 3;
	}

    /**/
    iret = gtw_init( &loop );
    if ( 0 != iret )
    {
        printf( "gateway init fail, %d\n", iret );
        return 4;
    }

    iret = ilua_init( &loop );
    if ( 0 != iret )
    {
        return 5;
    }
    /**/
    iret = isou_init( &loop );
    if ( 0 != iret )
    {
        return 6;
    }

    iret = inor_init( &loop );
    if ( 0 != iret )
    {
        return 7;
    }
            
    /**/
    uv_run( &loop, UV_RUN_DEFAULT );
    uv_loop_close( &loop );
	return 0;
    
}



int  main( int argc, char * argv[] )
{
    int  iret;

    iret = test( argc, argv );
    printf( "test fail, end, %d\n", iret );
    return 0;
}

