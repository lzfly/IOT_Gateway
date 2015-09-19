
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include "pktintf.h"
#include "dgramsck.h"


typedef struct _tag_dgram_sock_context
{
    pkt_obj_t  obj;
    pkt_intf_t  intf;
    
} dgram_sock_context_t;

extern void  dump_hex( const unsigned char * ptr, size_t  len );

int  dgram_sock_send( intptr_t obj, int tlen, void * pdat )
{
    pkt_obj_t * pctx;

    /**/
    pctx = (pkt_obj_t *)obj;

    /**/
    send( pctx->fd, pdat, tlen, 0 );
    // printf( "dgram send %d\n", tlen );
    // dump_hex( pdat, tlen );
    return 0;
}


int  dgram_sock_set_callbk( intptr_t obj, pkti_cbk_func func, intptr_t arg )
{
    pkt_obj_t * pctx;

    /**/
    pctx = (pkt_obj_t *)obj;

    /**/
    pctx->func = func;
    pctx->arg = arg;
    return 0;
}


int  dgram_sock_get_fd( intptr_t obj, int * pfd )
{
    pkt_obj_t * pctx;

    /**/
    pctx = (pkt_obj_t *)obj;

    /**/
    *pfd = pctx->fd;
    return 0;
}


int  dgram_sock_try_run( intptr_t obj )
{
    pkt_obj_t * pctx;
    int  iret;
    uint8_t  tary[2048];
    
    /**/
    pctx = (pkt_obj_t *)obj;
    

    while (1)
    {
        iret = recv( pctx->fd, tary, 2048, 0 );

        /**/
	    if ( iret < 0 )
	    {
	        if ( (errno == EAGAIN) || (errno == EWOULDBLOCK) )
	        {
	            return 0;
	        }
	        else if ( errno == EINTR )
	        {
	            continue;
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

#if 0
printf( "dgram recv %d\n", iret );
dump_hex( &(tary[0]), iret );
#endif

        /**/
        if ( NULL != pctx->func )
        {
		    pctx->func( pctx->arg, iret, &(tary[0]) );
		}        
    }
    
    return 0;
    
}


int  dgram_sock_fini( intptr_t obj )
{
    pkt_obj_t * pctx;

    /**/
    pctx = (pkt_obj_t *)obj;
    free( pctx );
    return 0;
}


static pkt_intf_t  dgram_sock_intf = 
{
    dgram_sock_send,
    dgram_sock_set_callbk,
    dgram_sock_get_fd,
    dgram_sock_try_run,
    dgram_sock_fini
};


int  dgram_sock_init( int fd, intptr_t * pobj, pkt_intf_t ** pintf )
{
    pkt_obj_t * pctx;

    /**/
    pctx = (pkt_obj_t *)malloc( sizeof(pkt_obj_t) );
    if ( NULL == pctx)
    {
        return 1;
    }


    /**/
    pctx->fd = fd;
    pctx->func = NULL;
    pctx->arg = 0;

    /**/
    *pobj = (intptr_t)pctx;
    *pintf = &(dgram_sock_intf);
    
    /**/
    return 0;
    
}


