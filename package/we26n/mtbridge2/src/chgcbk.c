
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <uv.h>
#include "chgcbk.h"


typedef struct _tag_field_chg_cbk_info
{
    /**/
    struct _tag_field_chg_cbk_info * next;

    /**/
    int  type;
    intptr_t  argkey;
    field_change_cbf  pfunc;
    
} field_chg_cbk_info_t;


typedef struct _tag_chgcbk_context
{
    intptr_t  fccbk;
    field_chg_cbk_info_t * chain;
    
} chgcbk_context_t;



#if 0


/**/
typedef struct _tag_ccqueue_context
{
    struct list_head  queue_list;
    uv_check_t  check;
    
} ccqueue_context_t;


static  ccqueue_context_t * g_ccqueue_ctx;

/**/
static  void  ccqueue_check_cb( uv_check_t * handle )
{
    ccqueue_context_t * pctx;
    field_chg_cbk_info_t * preq;
    field_chg_cbk_info_t * pnxt;

    printf( "ccqueue check cb\n" );

    /**/
    pctx = (ccqueue_context_t *)(handle->data);
    
    /**/
    list_for_each_entry_safe( preq, pnxt, &(pctx->queue_list), node )
    {
        list_del( &(preq->node) );
        preq->pfunc( preq->argkey, preq->tdat );
    }
    
    return;
    
}


int  ccqueue_init( uv_loop_t * ploop )
{
    ccqueue_context_t * pctx;

    /**/
    pctx = (ccqueue_context_t *)malloc( sizeof(ccqueue_context_t) );
    if ( NULL == pctx )
    {
        return 1;
    }

    /**/
    INIT_LIST_HEAD( &(pctx->queue_list) );
    uv_check_init( ploop, &(pctx->check) );
    uv_check_start( &(pctx->check), ccqueue_check_cb );
    pctx->check.data = (void *)pctx;
    
    /**/
    g_ccqueue_ctx = pctx;
    return 0;
    
}

#endif


int  ccbk_invoke( intptr_t ctx, int type, intptr_t tdat )
{
    chgcbk_context_t * pctx;
    field_chg_cbk_info_t * pcur;
    field_chg_cbk_info_t * pnext;

    /**/
    pctx = (chgcbk_context_t *)ctx;

    /**/
    pcur = pctx->chain;
    while ( NULL != pcur )
    {
        pnext = pcur->next;

        /**/
        if ( pcur->type == type )
        {
            // pcur->tdat = tdat;
            // list_add( &(pcur->node), &(g_ccqueue_ctx->queue_list) );
            pcur->pfunc( pcur->argkey, tdat );
        }

        /**/
        pcur = pnext;        
    }

    /**/
    if ( 0 != pctx->fccbk )
    {
        ccbk_invoke( pctx->fccbk, type, tdat );
    }

    return 0;
    
}



int  ccbk_addtail( intptr_t ctx, field_change_cbf pfunc, intptr_t argkey, int type )
{
    chgcbk_context_t * pctx;
    field_chg_cbk_info_t ** ppcbk;
    field_chg_cbk_info_t * pcur;
        
    /**/
    pctx = (chgcbk_context_t *)ctx;

    /**/
    ppcbk = &(pctx->chain);
    pcur = pctx->chain;
    
    while( pcur != NULL )
    {
        ppcbk = &(pcur->next);
        pcur = pcur->next;
    }

    /**/
    pcur = (field_chg_cbk_info_t *)malloc( sizeof(field_chg_cbk_info_t) );
    if ( NULL == pcur )
    {
        return 1;
    }

    /**/
    pcur->type = type;
    pcur->argkey = argkey;
    pcur->pfunc = pfunc;
    pcur->next = NULL;
    
    /**/
    *ppcbk = pcur;
    return 0;
    
}


int  ccbk_remove( intptr_t ctx, intptr_t argkey, int type )
{
    chgcbk_context_t * pctx;
    field_chg_cbk_info_t ** ppre;
    field_chg_cbk_info_t * pcur;    

    /**/
    pctx = (chgcbk_context_t *)ctx;

    /**/
    ppre = &(pctx->chain);
    pcur = pctx->chain;
    while ( NULL != pcur )
    {
        if ( (pcur->argkey == argkey) && (pcur->type == type) )
        {
            *ppre = pcur->next;
            free( pcur );
            break;
        }
        
        /**/
        ppre = &(pcur->next);
        pcur = pcur->next;
    }

    /**/
    return 0;
    
}


int  ccbk_init( intptr_t fccbk, intptr_t * pret )
{
    chgcbk_context_t * pctx;
    
    /**/
    pctx = (chgcbk_context_t *)malloc( sizeof(chgcbk_context_t) );
    if ( NULL == pctx )
    {
        return 1;
    }
    
    /**/
    pctx->fccbk = fccbk;
    pctx->chain = NULL;
        
    /**/
    *pret = (intptr_t)pctx;
    return 0;
}


int  ccbk_fini( intptr_t ctx )
{
    chgcbk_context_t * pctx;
    field_chg_cbk_info_t * pnext;
    field_chg_cbk_info_t * ptr;
        
    /**/
    pctx = (chgcbk_context_t *)ctx;

    /**/
    ptr = pctx->chain;
    while ( NULL != ptr )
    {
        pnext = ptr->next;
        free( ptr );
        ptr = pnext;
    }
    
    /**/
    free( pctx );
    return 0;
}


