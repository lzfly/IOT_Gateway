
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


int  test_aux_loop( void * pctx )
{
    while(1)
    {
        sleep( 2 );
        printf( "in aux loop\n" );
    }
}


void *  test_aux_thread( void * arg )
{
    int  iret;

    /**/
    iret =  test_aux_loop( arg );
    return NULL;
}


