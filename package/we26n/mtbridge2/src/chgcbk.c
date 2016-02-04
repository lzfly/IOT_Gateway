
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <uv.h>
#include "dlist.h"
#include "chgcbk.h"


typedef struct _tag_field_chg_cbk_info
{
    /**/
    struct list_head  node;
    struct list_head  tttt;
    
    /**/
    int  type;
    intptr_t  argkey;
    field_change_cbf  pfunc;
    
} field_chg_cbk_info_t;


typedef struct _tag_chgcbk_context
{
    intptr_t  fccbk;
    struct list_head  head;
    
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
    field_chg_cbk_info_t * ptr;    
    struct list_head  head;

    /**/
    INIT_LIST_HEAD( &head );
    
    /**/
    pctx = (chgcbk_context_t *)ctx;

    /**/
    list_for_each_entry( pcur, &(pctx->head), node )
    {
        /**/
        if ( type == (pcur->type & type) )
        {
            list_add( &(pcur->tttt), &head );
        }
    }
    
    /**/
    list_for_each_entry_safe( pcur, ptr, &head, tttt )
    {
        /**/
        if ( CCBK_T_ONCE == (pcur->type & CCBK_T_ONCE) )
        {
            list_del( &(pcur->node) );
        }

        /**/
        pcur->pfunc( pcur->argkey, tdat );

        if ( CCBK_T_ONCE == (pcur->type & CCBK_T_ONCE) )
        {
            free( pcur );
        }
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
    field_chg_cbk_info_t * pcur;

    /**/
    pctx = (chgcbk_context_t *)ctx;

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
    list_add( &(pcur->node), &(pctx->head) );
    
    /**/
    return 0;
    
}


int  ccbk_remove( intptr_t ctx, intptr_t argkey )
{
    chgcbk_context_t * pctx;
    field_chg_cbk_info_t * pcur;
    field_chg_cbk_info_t * ptr;

    /**/
    pctx = (chgcbk_context_t *)ctx;

    /**/
    list_for_each_entry_safe( pcur, ptr, &(pctx->head), node )
    {
        if ( pcur->argkey == argkey )
        {
            list_del( &(pcur->node) );
            free( pcur );
            break;
        }
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
    INIT_LIST_HEAD( &(pctx->head) );
        
    /**/
    *pret = (intptr_t)pctx;
    return 0;
}


int  ccbk_fini( intptr_t ctx )
{
    chgcbk_context_t * pctx;
    field_chg_cbk_info_t * pcur;
    field_chg_cbk_info_t * ptr;
    
    /**/
    pctx = (chgcbk_context_t *)ctx;

    /**/
    list_for_each_entry_safe( pcur, ptr, &(pctx->head), node )
    {
        list_del( &(pcur->node) );
        free( pcur );
    }
    
    /**/
    free( pctx );
    return 0;
}


