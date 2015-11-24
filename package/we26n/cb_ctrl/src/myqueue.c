
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>

#include <sys/eventfd.h>

#include "myqueue.h"


static inline void MemoryBarrier( void )
{

#if 0
  unsigned long Barrier = 0;
  __asm__ __volatile__( "xchgl %%eax,%0 " :"=r" (Barrier) );        /* x86 */
  __asm__ __volatile__( "dmb" );        /* arm */
#else
  __asm__ __volatile__( "NOP" );        /* mips single core */
#endif

}

typedef struct _tag_sq_node {
    struct _tag_sq_node * next;
    int  tlen;
    uint8_t  dat[0];
} sq_node_t;


typedef struct _tag_sq_context
{
    sq_node_t * head;
    sq_node_t * tail;
    
} sq_context_t;


int  msq_enqueue( intptr_t ctx, int tlen, void * pdat )
{
    sq_context_t * pctx;
    sq_node_t * pnd;
    
    if ( 0 == ctx )
    {
        return 1;
    }
    pctx = (sq_context_t *)ctx;

	//printf( "queue, enqueue, %x\r\n", tlen );
	
    /**/
    pnd = malloc( sizeof(sq_node_t)+tlen );
    if (NULL == pnd)
    {
        return 2;
    }
    
    pnd->next = NULL;
    pnd->tlen = tlen;
    memcpy( pnd->dat, pdat, tlen );
    
    /**/
    MemoryBarrier();
    
    pctx->tail->next = pnd;
    pctx->tail = pnd;
    
    /**/
    return 0;
}


int  msq_dequeue( intptr_t ctx, void ** ppdat, int * plen )
{
    sq_context_t * pctx;
    sq_node_t * pnd;
    
    if ( 0 == ctx )
    {
        return 1;
    }
    
    pctx = (sq_context_t *)ctx;
    
    if ( NULL == pctx->head )
    {
        return 3;
    }
    
    if ( NULL == pctx->head->next )
    {
        return 2;
    }
    
    pnd = pctx->head;
    pctx->head = pnd->next;
    
    free(pnd);
    
    *ppdat = (void *)(pctx->head->dat);
    *plen = pctx->head->tlen;
    return 0;
}



int  msq_init( intptr_t * pret )
{
    sq_context_t * pctx;
    sq_node_t * pnd;
    
    pctx = malloc( sizeof(sq_context_t) );
    if ( NULL == pctx )
    {
        return 1;
    }
    
    pnd = malloc( sizeof(sq_node_t) );
    if ( NULL == pnd )
    {
        free( pctx );
        return 2;
    }
    
    pnd->next = NULL;
    pctx->head = pnd;
    pctx->tail = pnd;
    
    *pret = (intptr_t)pctx;
    return 0;
    
}


int  msq_fini( intptr_t ctx )
{
	int  iret;
	int  temp;
    sq_context_t * pctx;
    void * pdat;
    
    if ( 0 == ctx )
    {
        return 1;
    }
    
    pctx = (sq_context_t *)ctx;

    /**/
    while ( 1 )
    {
    	iret = msq_dequeue( ctx, &pdat, &temp );
    	if ( iret != 0 )
    	{
    		break;
    	}
    }

	/**/
	free( pctx->head );
	free( pctx );
	return 0;
	
}



