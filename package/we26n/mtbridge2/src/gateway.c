

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "chgcbk.h"
#include "atthead.h"
#include "gateway.h"


typedef struct _tag_tdata_context
{
    intptr_t  sctx;
    uint16_t  uuid;
    intptr_t  ccbk;    
    int  type;
    union {
        uint32_t  ui32;
        uint64_t  ui64;
        float  ft32;
        double  ft64;
    } value;
    
} tdata_context_t;


/* north ->> south */
int  tdata_action( intptr_t ctx, double value )
{
    tdata_context_t * pctx;
    
    /**/
    pctx = (tdata_context_t *)ctx;
    
    /**/
    pctx->value.ft64 = value;

    /**/
    ccbk_invoke( pctx->ccbk, CCBK_T_ACTION, ctx );
    printf( "tdata, action, %f\n", value );
    
    return 0;
}


/* south ->> north */
int  tdata_report( intptr_t ctx, double value )
{
    tdata_context_t * pctx;
    
    /**/
    pctx = (tdata_context_t *)ctx;
    
    /**/
    pctx->value.ft64 = value;

    /**/
    ccbk_invoke( pctx->ccbk, CCBK_T_REPORT, ctx );
    return 0;
}


int  tdata_subscribe( intptr_t ctx, field_change_cbf pfunc, intptr_t argkey, int type )
{
    tdata_context_t * pctx;
    
    /**/
    pctx = (tdata_context_t *)ctx;
    
    /**/
    ccbk_addtail( pctx->ccbk, pfunc, argkey, type );
    
    /* call back once */
    
    /**/
    return 0;
}


int  tdata_unsubscribe( intptr_t ctx, intptr_t argkey )
{
    tdata_context_t * pctx;
    
    /**/
    pctx = (tdata_context_t *)ctx;

    /**/
    ccbk_remove( pctx->ccbk, argkey );
    return 0;
}


int  tdata_get_double( intptr_t ctx, double * pret )
{
    tdata_context_t * pctx;
    
    /**/
    pctx = (tdata_context_t *)ctx;

    /**/
    *pret = pctx->value.ft64;
    return 0;
}


int  tdata_get_uuid( intptr_t ctx, uint16_t * pret )
{
    tdata_context_t * pctx;
    
    /**/
    pctx = (tdata_context_t *)ctx;

    /**/
    *pret = pctx->uuid;
    return 0;
}


int  tdata_get_father( intptr_t ctx, intptr_t * pret )
{
    tdata_context_t * pctx;
    
    /**/
    pctx = (tdata_context_t *)ctx;

    /**/
    *pret = pctx->sctx;
    return 0;
}



int  tdata_init( uint16_t uuid, int type, intptr_t sctx, intptr_t fccbk, intptr_t * pret )
{
    int  iret;
    intptr_t  ccbk;
    tdata_context_t * pctx;
    
    /**/
    iret = ccbk_init( fccbk, &ccbk );
    if ( 0 != iret )
    {
        return 1;
    }

    /**/
    pctx = (tdata_context_t *)malloc( sizeof(tdata_context_t) );
    if ( NULL == pctx )
    {
        return 2;
    }
    
    /**/
    memset( pctx, 0, sizeof(tdata_context_t) );
    pctx->sctx = sctx;
    pctx->ccbk = ccbk;
    pctx->uuid = uuid;
    pctx->type = type;
    
    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}



typedef struct _service_context
{
    char * sid;
    
    /**/
    intptr_t  dctx;
    intptr_t  ccbk;
    
    /**/
    intptr_t  ahead;
    intptr_t  drows[2];
    
} service_context_t;


int  srvs_search_tdata( intptr_t ctx, uint16_t uuid, intptr_t * pret )
{
    int  iret;
    int  tidx;
    service_context_t * pctx;

    /**/
    pctx = (service_context_t *)ctx;

    /**/
    iret = ahd_get_byuuid( pctx->ahead, uuid, &tidx );
    if ( 0 != iret )
    {
        return 1;
    }
    
    /**/
    *pret = pctx->drows[tidx];
    return 0;
    
}



int  srvs_get_father( intptr_t ctx, intptr_t * pret )
{
    service_context_t * pctx;
    
    /**/
    pctx = (service_context_t *)ctx;

    /**/
    *pret = pctx->dctx;
    return 0;
}


int  srvs_subscribe( intptr_t sctx, field_change_cbf pfunc, intptr_t argkey, int type )
{
    service_context_t * pctx;

    /**/
    pctx = (service_context_t *)sctx;

    /**/
    return ccbk_addtail( pctx->ccbk, pfunc, argkey, type );
}


int  srvs_unsubscribe( intptr_t sctx, intptr_t argkey )
{
    service_context_t * pctx;

    /**/
    pctx = (service_context_t *)sctx;

    return ccbk_remove( pctx->ccbk, argkey );
}


int  srvs_init( const char * sid, intptr_t ahd, intptr_t dctx, intptr_t fccbk, intptr_t * pret )
{
    int  i;
    int  iret;
    int  anum;
    int  type;
    uint16_t  uuid;
    intptr_t  ccbk;
    service_context_t * pctx;
    
    /**/
    iret = ccbk_init( fccbk, &ccbk );
    if ( 0 != iret )
    {
        return 1;
    }
    
    /**/
    iret = ahd_get_maxnum( ahd, &anum );
    if ( 0 != iret )
    {
        ccbk_fini( ccbk );
        return 2;
    }
    
    /**/
    pctx = (service_context_t *)malloc( sizeof(service_context_t) + (anum * sizeof(intptr_t)) );
    if ( NULL == pctx )
    {
        ccbk_fini( ccbk );    
        return 3;
    }
    
    /**/
    pctx->sid = strdup( sid );
    pctx->ccbk = ccbk;
    pctx->ahead = ahd;
    pctx->dctx = dctx;

    /**/
    for ( i=0; i<anum; i++ )
    {
        iret = ahd_get_byindex( ahd, i, &uuid, &type );
        if ( 0 != iret )
        {
            return 4;
        }
                
        iret = tdata_init( uuid, type, (intptr_t)pctx, pctx->ccbk, &(pctx->drows[i]) );
        if ( 0 != iret )
        {
            return 5;
        }
    }
    
    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}



typedef struct _tag_device_context
{
    char * did;

    /**/
    intptr_t  ccbk;
    
    /**/
    intptr_t  defsrv;
    
} device_context_t;



int  devs_add_service( intptr_t devctx, const char * sid, intptr_t ahd, intptr_t * pret )
{
    int  iret;
    device_context_t * pctx;

    /**/
    pctx = (device_context_t *)devctx;

    /**/
    if ( 0 != strcmp( sid, "default") )
    {
        return 1;
    }

    /**/
    if ( pctx->defsrv != 0 )
    {
        return 2;
    }
    
    /**/
    iret = srvs_init( sid, ahd, devctx, pctx->ccbk, &(pctx->defsrv) );
    if ( 0 != iret )
    {
        printf( "srvs init ret =%d\n", iret );
        return 3;
    }
    
    /**/
    *pret = pctx->defsrv;
    return 0;

    
}


int  devs_search_service( intptr_t devctx, const char * sid, intptr_t * pret )
{
    device_context_t * pctx;

    /**/
    pctx = (device_context_t *)devctx;

    /**/
    if ( 0 != strcmp( sid, "default") )
    {
        return 1;
    }

    /**/
    *pret = pctx->defsrv;
    return 0;
}


int  devs_get_name( intptr_t devctx, const char ** pret )
{
    device_context_t * pctx;

    /**/
    pctx = (device_context_t *)devctx;

    /**/
    *pret = pctx->did;
    return  0;
}


int  devs_subscribe( intptr_t dctx, field_change_cbf pfunc, intptr_t argkey, int type )
{
    device_context_t * pctx;

    /**/
    pctx = (device_context_t *)dctx;

    /**/
    return ccbk_addtail( pctx->ccbk, pfunc, argkey, type );
}


int  devs_unsubscribe( intptr_t dctx, intptr_t argkey )
{
    device_context_t * pctx;

    /**/
    pctx = (device_context_t *)dctx;

    return ccbk_remove( pctx->ccbk, argkey );
}



int  devs_init( const char * did, intptr_t fccbk, intptr_t * pret )
{
    int  iret;
    device_context_t * pctx;
    
    /**/
    pctx = (device_context_t *)malloc( sizeof(device_context_t) );
    if ( NULL == pctx )
    {
        return 1;
    }

    /**/
    pctx->did = strdup( did );

    /**/
    iret = ccbk_init( fccbk, &(pctx->ccbk) );
    if ( 0 != iret )
    {
        return 2;
    }

    /**/
    pctx->defsrv = 0;
    
    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}



typedef struct _tag_device_map
{
    char * did;
    intptr_t  dctx;
    
} device_map_t;


#define  GTW_DEV_MAX        40

static device_map_t  dev_map[GTW_DEV_MAX];
static intptr_t  gtw_ccbk;



#if 0

/* https://jansson.readthedocs.org */
#include <jansson.h>

/*
    {
        name : "xxxx",
        services : [ 
            { name : "xxxx", type : uuid },  
            { name : "xxxx", attrs : [
                    {name:"xxx", uuid : xxx, type:xxx}, 
                ] 
            }
        ]
    }
*/

int  gtw_inter_add_atthead( json_t * atthd, intptr_t * pahd )
{
    int  iret;
    int  num;
    int  i;
    json_t * attr;
    json_t * name;
    json_t * uuid;
    

    /**/
    if ( !json_is_array( atthd ) )
    {
        return 5;
    }

    num = json_array_size( atthd );
    printf( "array num : %d\n", num );

    /**/
    for ( i=0; i<num; i++ )
    {
        attr = json_array_get( atthd, i );
        if ( NULL == service )
        {
            return 2;
        }

        if ( !json_is_object( attr ) )
        {
            json_decref(attr);
            return 3;
        }

        /**/
        name = json_object_get( service, "name" );
        if ( NULL == name )
        {
            json_decref(attr);
            return 4;
        }
        
        /**/
        if ( !json_is_string( name ) )
        {
            json_decref(name);
            json_decref(attr);            
            return 5;
        }

        /**/
        uuid = json_object_get( service, "uuid" );
        if ( NULL == uuid )
        {
            json_decref(attr);
            return 4;
        }
        
        /**/
        if ( !json_is_integer( uuid ) )
        {
            json_decref(uuid);
            json_decref(attr);            
            return 5;
        }        

        
    }
    

    return 0;
    
}


int  gtw_inter_add_services( intptr_t  dctx, json_t * srvs )
{
    int  iret;
    int  num;
    int  i;
    json_t * service;    
    json_t * atthd;
    json_t * sname;
    intptr_t  ahdctx;
    intptr_t  srvctx;

    /**/
    if ( !json_is_array( srvs ) )
    {
        return 1;
    }

    /**/
    num = json_array_size( srvs );
    printf( "array num : %d\n", num );

    /**/
    for ( i=0; i<num; i++ )
    {
        service = json_array_get( srvs, i );
        if ( NULL == service )
        {
            return 2;
        }

        if ( !json_is_object( service ) )
        {
            json_decref(service);
            return 3;
        }

        /**/
        sname = json_object_get( service, "name" );
        if ( NULL == sname )
        {
            json_decref(service);
            return 4;
        }
        
        /**/
        if ( !json_is_string( sname ) )
        {
            json_decref(sname);
            json_decref(service);            
            return 5;
        }

        /**/
        atthd = json_object_get( service, "attrs" );
        if ( NULL == atthd )
        {
            json_decref(service);
            return 6;
        }
        
        /**/
        iret = gtw_inter_add_atthead( atthd, &ahdctx );
        if ( 0 != iret )
        {
            json_decref( atthd );
            return 9;
        }

        json_decref( atthd );

        /**/
        printf( "sname : %s\n", json_string_value(sname) );
        /**/
        iret = devs_add_service( dctx, json_string_value(sname), ahdctx, &srvctx );
        if ( 0 != iret )
        {
            return 10;
        }

        /**/
        json_decref(sname);        
        json_decref(service);
        
    }
    
    /**/
    return 0;
    
}


int  gtw_inter_add_device( json_t * root, intptr_t * pdev )
{
    int  iret;
    json_t * dname;
    json_t * srvs;
    intptr_t  dctx;

    if ( !json_is_object( root ) )
    {
        json_decref(root);
        return 2;
    }

    /**/
    dname = json_object_get( root, "name" );
    if ( NULL == dname )
    {
        return 3;
    }
    
    /**/
    if ( !json_is_string( dname ) )
    {
        json_decref(dname);
        return 4;
    }

    /**/
    printf( "did = %s\n", json_string_value(dname) );

    /**/
    iret = devs_init( json_string_value(dname), 0, &dctx );
    if ( 0 != iret )
    {
        return 5;
    }

    /**/
    json_decref(dname);

    /**/
    srvs = json_object_get( root, "services" );
    if ( NULL == srvs )
    {
        return 6;
    }
    
    /**/
    iret = gtw_inter_add_services( dctx, srvs );
    if ( 0 != iret )
    {
        printf( "inter add services : %d\n", iret );
        json_decref(srvs);    
        return 8;
    }

    json_decref(srvs);
    *pdev = dctx;
    return 0;

}



int  gtw_add_device( char * json, intptr_t * pret )
{
    int  iret;
    json_t * root;
    json_error_t error;
    intptr_t  dctx;
    
    
    /**/
    root = json_loads( json, 0, &error);
    if ( NULL == root )
    {
        printf( "add device error: on line %d: %s\n", error.line, error.text );
        return 1;
    }


    

    /**/
    iret = gtw_inter_add_device( root, &dctx );
    if ( 0 != iret )
    {
        printf( "inter add device = %d\n", iret );
        json_decref(root);
        return 4;
    }
    
    /**/
    json_decref(root);
    return 0;
    
}

#endif




int  gtw_search_device( const char * did, intptr_t * pret )
{
    int  i;

    /**/
    for( i=0; i<GTW_DEV_MAX; i++ )
    {
        if ( NULL == dev_map[i].did )
        {
            continue;
        }

        /**/
        if ( 0 == strcmp( dev_map[i].did, did) )
        {
            break;
        }
    }

    if ( i<GTW_DEV_MAX )
    {
        *pret = dev_map[i].dctx;
        return 0;
    }
    
    return 1;
    
}


int  gtw_search_tdata( const char * did, uint16_t uuid, intptr_t * pret )
{
    int  iret;
    intptr_t  dctx;
    intptr_t  sctx;
    intptr_t  tctx;
    
    /**/
    iret = gtw_search_device( did, &dctx );
    if ( 0 != iret )
    {
        return 1;
    }

    /**/
    iret = devs_search_service( dctx, "default", &sctx );
    if ( 0 != iret )
    {
        return 2;
    }

    /**/
    iret = srvs_search_tdata( sctx, uuid, &tctx );
    if ( 0 != iret )
    {
        return 3;
    }

    /**/
    *pret = tctx;
    return 0;
    
}


int  gtw_add_device( const char * did, intptr_t ahd, intptr_t * pret )
{
    int  i;
    int  iret;
    intptr_t  dctx;
    intptr_t  sctx;

    /**/
    iret = gtw_search_device( did, &dctx );
    if ( 0 == iret )
    {
        return 1;
    }

    /**/
    for ( i=0; i<GTW_DEV_MAX; i++ )
    {
        if ( NULL == dev_map[i].did )
        {
            break;
        }
    }

    if ( i>=GTW_DEV_MAX )
    {
        return 2;
    }
    
    /**/
    iret = devs_init( did, gtw_ccbk, &dctx );
    if ( 0 != iret )
    {
        return 3;
    }

    /**/
    iret = devs_add_service( dctx, "default", ahd, &sctx );
    if ( 0 != iret )
    {
        printf( "devs_add service, ret = %d\n", iret );
        return 4;
    }

    /**/
    dev_map[i].did = strdup( did );
    dev_map[i].dctx = dctx;

    /**/
    *pret = dctx;
    return 0;
    
}


int  gtw_subscribe( field_change_cbf pfunc, intptr_t argkey, int type )
{
    return ccbk_addtail( gtw_ccbk, pfunc, argkey, type );
}


int  gtw_unsubscribe( intptr_t argkey )
{
    return ccbk_remove( gtw_ccbk, argkey );
}



int  gtw_init( uv_loop_t * ploop )
{
    int  i;
    int  iret;

#if 0
    iret = ccqueue_init( ploop );
	if ( 0 != iret )
	{
		return 5;
	}
#endif

    /**/
    for( i=0; i<GTW_DEV_MAX; i++ )
    {
        dev_map[i].did = NULL;
        dev_map[i].dctx = 0;
    }
    
    iret = ccbk_init( 0, &gtw_ccbk );
    if ( 0 != iret )
    {
        return 1;
    }
    
    return 0;
    
}


