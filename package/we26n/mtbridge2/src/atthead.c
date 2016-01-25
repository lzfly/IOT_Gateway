
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "atthead.h"


typedef struct _tag_ahd_context
{
    int  num;
    ahd_field_t  field[0];
    
} ahd_context_t;


/*
 uuid : 
 name : zero end string.
 type : uint32, uint64, float32, float64, 
other : read, write
*/

int  ahd_get_maxnum( intptr_t ctx, int * pnum )
{
    ahd_context_t * pctx;

    /**/
    pctx = (ahd_context_t *)ctx;

    /**/
    *pnum = pctx->num;
    return 0;
}


int  ahd_get_byindex( intptr_t ctx, int idx, uint16_t * puuid, int * ptype )
{
    ahd_context_t * pctx;

    /**/
    pctx = (ahd_context_t *)ctx;

    /**/
    if ( idx >= pctx->num )
    {
        return 1;
    }

    /**/
    *puuid = pctx->field[idx].uuid;
    *ptype = pctx->field[idx].type;
    return 0;
}


int  ahd_get_byuuid( intptr_t ctx, uint16_t uuid, int * pidx )
{
    int  i;
    ahd_context_t * pctx;

    /**/
    pctx = (ahd_context_t *)ctx;

    // printf( "by uuid, %x, num = %d\n", ctx, pctx->num );
    // printf( "field[0].uuid, %x\n", pctx->field[0].uuid );
    /**/
    for ( i=0; i<pctx->num; i++ )
    {
        if ( pctx->field[i].uuid == uuid )
        {
            *pidx = i;
            return 0;
        }
    }
    
    return 1;
    
}


int  ahd_init( int num, ahd_field_t * pfield, intptr_t * pret )
{
    int  i;
    ahd_context_t * pctx;

    /**/
    if ( num <= 0 )
    {
        return 1;
    }
    
    /**/
    pctx = (ahd_context_t *)malloc( sizeof(ahd_context_t) + (num * sizeof(ahd_field_t)) );
    if ( NULL == pctx )
    {
        return 2;
    }

    /**/
    pctx->num = num;
    for ( i=0; i<num; i++ )
    {
        strcpy( pctx->field[i].name, pfield[i].name );
        pctx->field[i].uuid = pfield[i].uuid;
        pctx->field[i].type = pfield[i].type;
        pctx->field[i].other = pfield[i].other;
    }

    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}


