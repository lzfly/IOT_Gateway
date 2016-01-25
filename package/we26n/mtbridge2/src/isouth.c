
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <alloca.h>

#include <libubox/list.h>
#include <libubox/blob.h>
#include <libubox/blobmsg.h>

#include <uv.h>

#include "tubus.h"

#include "chgcbk.h"
#include "atthead.h"
#include "gateway.h"
#include "iluaexc.h"


/**/
typedef struct  _tag_isou_context
{
    intptr_t  ubus;
    
} isou_context_t;


isou_context_t  isou_ctx;

/**/
int  isou_add_device( char * did, intptr_t ahd, intptr_t * pret )
{
    return gtw_add_device( did, ahd, pret );
}


int  isou_report_attr( char * did, uint16_t uuid, double value )
{
    int  iret;
    intptr_t  tdat;

    /**/
    iret = gtw_search_tdata( did, uuid, &tdat );
    if ( 0 != iret )
    {
        return 1;
    }

    /**/
    iret = tdata_report( tdat, value );
    if ( 0 != iret )
    {
        return 2;
    }
    
    return 0;
    
}


typedef struct _tag_invoke_args
{
    intptr_t  ubus;
    uint32_t  obj;
    uint16_t  uuid;
    double  value;
    char  name[2];
    
} invoke_args_t;


void isou_zigbee_invoke_cb( intptr_t arg, int iret, struct blob_attr * attr )
{
    invoke_args_t * parg;

    /**/
    parg = (invoke_args_t *)arg;

    printf( "zigbee invoke cb, %d\n", iret );

    /**/
    free( parg );
    return;
}


void  isou_zigbee_lookup_cb( intptr_t arg, uint32_t tid )
{
    int  iret;
    invoke_args_t * parg;
	struct blob_buf b = { 0 };
    char  tstr[200];
    
    /**/
    printf( "zigbee lookup cb, %u\n", tid );
    
    /**/
    parg = (invoke_args_t *)arg;
    parg->obj = tid;

    /**/
    blob_buf_init( &b, 0 );
	blobmsg_add_string( &b, "gatewayid", "xxxx" ); // not need
	blobmsg_add_string( &b, "deviceid", parg->name );
	blobmsg_add_string( &b, "devicetype", "xxxx" );// not need
	sprintf( tstr, "%d", parg->uuid );
	blobmsg_add_string( &b, "attr", tstr );
	sprintf( tstr, "%f", parg->value);	
	blobmsg_add_string( &b, "data", tstr );
    
    /**/
    iret = tubus_invoke( parg->ubus, tid, "ctrlcmd", b.head, isou_zigbee_invoke_cb, arg );
    if ( 0 != iret )
    {
        free( parg );
    }

printf( "isou, invoke, %d\n", iret );

    blob_buf_free( &b );
    return;
    
}


void  isou_field_change_cbk( intptr_t ctx, intptr_t tdat )
{
    int  iret;
    intptr_t  sctx;
    intptr_t  dctx;
    uint16_t  uuid;
    const char * name;
    double  value;
    invoke_args_t * parg;
    isou_context_t * pctx;

    /**/
    printf( "isou action begin \n" );

    /**/
    pctx = (isou_context_t *)ctx;
    
    /**/
    iret = tdata_get_father( tdat, &sctx );
    if ( 0 != iret )
    {
        return;
    }
    
    iret = srvs_get_father( sctx, &dctx );
    if ( 0 != iret )
    {
        return;
    }

    /**/
    tdata_get_double( tdat, &value );    
    tdata_get_uuid( tdat, &uuid );
    devs_get_name( dctx, &name );

    /**/
    parg = (invoke_args_t *)malloc( sizeof(invoke_args_t) + strlen(name) );
    if ( NULL == parg )
    {
        return;
    }

    /**/
    parg->ubus = pctx->ubus;
    parg->obj = 0;
    parg->uuid = uuid;
    parg->value = value;
    strcpy( parg->name, name );
    
    /**/
    iret = tubus_lookup( pctx->ubus, "we26n_zigbee_febee", isou_zigbee_lookup_cb, (intptr_t)parg );
    if ( iret != 0 )
    {
        printf( "field change cb, lookup ret = %d\n", iret );
        free( parg );
        return;
    }

    /**/
    printf( "isou action cbk : %s : %u \n", name, uuid  );
    return;
    
}


struct blobmsg_policy  isou_adddev_plcy[] = 
{
    { "devid", BLOBMSG_TYPE_STRING },
    { "attrs", BLOBMSG_TYPE_ARRAY },
};


int  isou_adddev_handler( intptr_t reply, const char * method, struct blob_attr * args )
{
    int  i;
    int  iret;
    struct blob_attr * tb[ARRAY_SIZE(isou_adddev_plcy)];
    char * did;
    int  num;
    ahd_field_t * pfield;
	struct blob_attr *cur;
    int  rem;
    intptr_t  actx;
    intptr_t  dctx;
    
    /**/
    printf("add dev handler begin\n");
    
    /**/
    iret = blobmsg_parse( isou_adddev_plcy, ARRAY_SIZE(isou_adddev_plcy), &(tb[0]), blob_data(args), blob_len(args) );
    if ( 0 != iret )
    {
        return 0;
    }

    /**/
    if ( tb[0] == NULL )
    {
        return 0;
    }
    did = (char *)blobmsg_data( tb[0] );

    /**/
    if ( tb[1] == NULL )
    {
        return 0;
    }
    num = blobmsg_check_array( tb[1], BLOBMSG_TYPE_INT32 );
    if ( num <= 0 )
    {
        printf("add dev handler array, %d\n", num );
        return 0;
    }

    /**/
    pfield = (ahd_field_t *)alloca( sizeof(ahd_field_t) * num );
    i = 0;
    blobmsg_for_each_attr( cur, tb[1], rem )
    {
        strcpy( pfield[i].name, "none" );
        pfield[i].uuid = (uint16_t)blobmsg_get_u32(cur);
        pfield[i].type = 0;
        pfield[i].other = 0;
        i += 1;
    }

    /**/
    iret = ahd_init( num, pfield, &actx );
    if ( 0 != iret )
    {
        printf( "isou adddev, ahd init, %d\n", iret );
        return 0;
    }

    /**/
    iret = gtw_add_device( did, actx, &dctx );
    if ( 0 != iret )
    {
        printf( "isou adddev, add dev, %d\n", iret );
        return 0;
    }
    
    /**/    
    printf( "isou adddev handler, end, %s, %d\n", did, num );
    return 0;
    
}



struct blobmsg_policy  isou_report_plcy[] = 
{
    { "devid", BLOBMSG_TYPE_STRING },
    { "uuid", BLOBMSG_TYPE_INT32 },
    { "value", BLOBMSG_TYPE_STRING }
};


int  isou_report_handler( intptr_t reply, const char * method, struct blob_attr * args )
{
    int  iret;
    struct blob_attr * tb[ARRAY_SIZE(isou_report_plcy)];
    char * did;
    uint16_t  uuid;
    double  value;
    
    /**/
    iret = blobmsg_parse( isou_report_plcy, ARRAY_SIZE(isou_report_plcy), &(tb[0]), blob_data(args), blob_len(args) );
    if ( 0 != iret )
    {
        return 0;
    }

    /**/
    if ( tb[0] == NULL )
    {
        return 0;
    }
    did = (char *)blobmsg_data( tb[0] );

    /**/
    if ( tb[1] == NULL )
    {
        return 0;
    }
    uuid = (uint16_t)blobmsg_get_u32( tb[1] );

    /**/
    if ( tb[2] == NULL )
    {
        return 0;
    }
    value = strtod( (char *)blobmsg_data( tb[2] ), NULL );

    /**/
    isou_report_attr( did, uuid, value );
    
    /**/    
    printf( "isou report handler, %s, %d, %f::%s\n", did, uuid, value, (char *)blobmsg_data( tb[2] ) );
    return 0;
    
}


struct blobmsg_policy  isou_add_ifttt[] = 
{
    { "name", BLOBMSG_TYPE_STRING },
    { "code", BLOBMSG_TYPE_STRING }
};


int  isou_add_ifttt_handler( intptr_t reply, const char * method, struct blob_attr * args )
{
    int  iret;
    struct blob_attr * tb[ARRAY_SIZE(isou_add_ifttt)];
    char * name;
    char * code;    
        
    /**/
    iret = blobmsg_parse( isou_add_ifttt, ARRAY_SIZE(isou_add_ifttt), &(tb[0]), blob_data(args), blob_len(args) );
    if ( 0 != iret )
    {
        return 0;
    }

    /**/
    if ( tb[0] == NULL )
    {
        return 0;
    }
    name = (char *)blobmsg_data( tb[0] );

    /**/
    if ( tb[1] == NULL )
    {
        return 0;
    }
    code = blobmsg_data( tb[1] );
    
    /**/
    ilua_add_instance( name, code );
    
    /**/    
    printf( "isou add ifttt handler, end\n");
    return 0;
    
}


struct blobmsg_policy  isou_del_ifttt[] = 
{
    { "name", BLOBMSG_TYPE_STRING }
};


int  isou_del_ifttt_handler( intptr_t reply, const char * method, struct blob_attr * args )
{
    int  iret;
    struct blob_attr * tb[ARRAY_SIZE(isou_del_ifttt)];
    char * name;
    
    /**/
    iret = blobmsg_parse( isou_del_ifttt, ARRAY_SIZE(isou_del_ifttt), &(tb[0]), blob_data(args), blob_len(args) );
    if ( 0 != iret )
    {
        return 0;
    }

    /**/
    if ( tb[0] == NULL )
    {
        return 0;
    }
    name = (char *)blobmsg_data( tb[0] );

    /**/
    ilua_del_instance( name );
    
    /**/    
    printf( "isou del ifttt handler, end\n");
    return 0;
    
}



struct ubus_method  isou_ubus_method[] = 
{
    { "add_device", isou_adddev_handler, isou_adddev_plcy, ARRAY_SIZE(isou_adddev_plcy) },
    { "report", isou_report_handler, isou_report_plcy, ARRAY_SIZE(isou_report_plcy) },
    { "add_ifttt", isou_add_ifttt_handler, isou_add_ifttt, ARRAY_SIZE(isou_add_ifttt) },
    { "del_ifttt", isou_del_ifttt_handler, isou_del_ifttt, ARRAY_SIZE(isou_del_ifttt) }
};


struct ubus_object_type isou_ubus_obj = 
{
    "we26n_mtbridge", 0,
    isou_ubus_method, ARRAY_SIZE(isou_ubus_method)
};



int  isou_init( uv_loop_t * ploop )
{
    int  iret;
    
    /**/
    iret = tubus_init( ploop, &(isou_ctx.ubus) );
    if ( 0 != iret )
    {
        return 1;
    }
    
    /**/
    iret = tubus_addobj( isou_ctx.ubus, &isou_ubus_obj );
    if ( 0 != iret )
    {
        return 2;
    }

    /**/
    iret = gtw_subscribe( isou_field_change_cbk, (intptr_t)&isou_ctx, CCBK_T_ACTION );
    if ( 0 != iret )
    {
        return 3;
    }
    
    return 0;
    
}




