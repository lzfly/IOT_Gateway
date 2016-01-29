

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include <uv.h>

#include "mmqtt.h"

#include "chgcbk.h"
#include "gateway.h"
#include "isouth.h"
#include "inorth.h"


extern  void  dump_hex( size_t len, uint8_t * ptr );

/**/
typedef struct  _tag_inor_context
{
    uv_loop_t * ploop;
    intptr_t  mqtt;
    char  gtwid[40];
    
} inor_context_t;


inor_context_t  inor_ctx;


/**/
void  inor_field_change_cbk( intptr_t ctx, intptr_t tdat )
{
    int  iret;
    inor_context_t * pctx;
    intptr_t  sctx;
    intptr_t  dctx;
    uint16_t  uuid;
    const char * name;
    double  value;
    char * topic;
    char * pload;

    /**/
    printf( "inor field change cbk, begin\n" );
    
    /**/
    pctx = (inor_context_t *)ctx;
    
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

    /* /v1/from_device/name/attr */
    topic = alloca( 20 + strlen(name) + 10 );
    sprintf( topic, "/v1/from_device/%s/%u", name, uuid );

    pload = alloca( 40 );
    sprintf( pload, "%f", value );
    
    /**/
    iret = mqtt_publish( pctx->mqtt, topic, pload );
    if ( 0 != iret )
    {
        printf( "inor, mqtt_pulish, ret = %d\n", iret );
        return;
    }

    /**/
    return;
    
}



/**/
int  recongnize_dir( const char * path, int max, char * ptr, const char ** ppnext )
{
    int  i;

    if ( path[0] != '/' )
    {
        return 1;
    }

    /**/
    for ( i=1;; i++ )
    {
        if ( i >= max )
        {
            return 2;
        }

        if ( (path[i] == '/') || path[i] == '\0' )
        {
            break;
        }

        ptr[i-1] = path[i];
    }

    /**/
    ptr[i-1] = 0;
    *ppnext = &(path[i]);
    return 0;

}


int  inor_inner_to_device( char * did, char * attr, int tlen, uint8_t * pdat )
{
    int  iret;
    long int  temp;
    uint16_t  uuid;
    double  value;
    intptr_t  tdata;
    
    /**/
    temp = strtol( attr, NULL, 10 );
    if ( (temp <= 0) || (temp>=0xffff) )
    {
        return 1;
    }

    /**/
    uuid = (uint16_t)temp;

    /* mpacket alloc more 4 bytes, so this may write tail zero. */
    pdat[tlen] = 0;
    value = strtod( (char *)pdat, NULL );
    
    /**/
    printf( "did : %s, %u, %f\n", did, uuid, value );
    dump_hex( tlen, pdat );

    /**/
    iret = gtw_search_tdata( did, uuid, &tdata );
    if ( iret != 0 )
    {
        return 2;
    }

    /**/
    tdata_action( tdata, value );
    return 0;
    
}



int  inor_inner_to_gateway( inor_context_t * pctx, const char * gtwid, int tlen, uint8_t * pdat )
{
    if ( 0 != strcmp( gtwid, pctx->gtwid ) )
    {
        return 1;
    }

    /* mpacket alloc more 4 bytes, so this may write tail zero. */
    pdat[tlen] = 0;
    if ( 0 == strcmp( (char *)pdat, "linkdevice" ) )
    {
        printf( "link device\n" );
        isou_gateway_notify_linkdevice();
    }
    
    return 0;
}


/**/
void  inor_mqtt_conn_cb( intptr_t arg, int status )
{
    printf( "inor, conn, status = %d\n", status );
    return;
}

void  inor_mqtt_message_cb( intptr_t arg, const char * topic, int tlen, uint8_t * pdat )
{
    int  iret;
    inor_context_t * pctx;
    char  tstr[50];
    char  attr[10];
    const char * pnext;

    /**/
    if ( tlen <= 0 )
    {
        /* logmsg */
        return;
    }

    /**/
    pctx = (inor_context_t *)arg;
    
    /**/
    iret = recongnize_dir( topic, 50, tstr, &pnext );
    if ( 0 != iret )
    {
        /**/
        return;
    }
    
    if ( 0 != strcmp( "v1", tstr ) )
    {
        printf( "recg, 1st not v1\n" );
        return;
    }
    
    /**/
    iret = recongnize_dir( pnext, 50, tstr, &pnext );
    if ( 0 != iret )
    {
        /*  */
        return;
    }
    
    /**/
    if ( 0 == strcmp( "to_device", tstr ) )
    {
        iret = recongnize_dir( pnext, 50, tstr, &pnext );
        if ( 0 != iret )
        {
            return;
        }
        
        iret = recongnize_dir( pnext, 10, attr, &pnext );
        if ( 0 != iret )
        {
            return;
        }

        /**/
        iret = inor_inner_to_device( tstr, attr, tlen, pdat );
        if ( 0 != iret )
        {
            return;
        }
        
    }
    else if ( 0 == strcmp( "to_gateway", tstr ) )
    {
        iret = recongnize_dir( pnext, 50, tstr, &pnext );
        if ( 0 != iret )
        {
            return;
        }

        /**/
        iret = inor_inner_to_gateway( pctx, tstr, tlen, pdat );
        if ( 0 != iret )
        {
            return;
        }
    }
    
    /**/
#if 0
    printf( "inor, messag, %s\n", topic );
    dump_hex( tlen, pdat );
#endif

    return;
    
}



static int  inor_get_macaddr( char * pmac )
{
    FILE * fout;
    char  tbuf[100];
    char * ptr;
    char * dst;
    
    /**/
    fout = popen( "eth_mac r wifi", "r" );
    if ( fout == NULL )
    {
        return 1;
    }

    /**/
    ptr = fgets( tbuf, 90, fout );
    if ( NULL == ptr )
    {
        return 2;
    }

    /**/
    pclose( fout );

    /**/
    strcpy( pmac, "we26n_" );
    dst = &(pmac[strlen(pmac)]);
    ptr = tbuf;
    while( '\0' != *(ptr) )
    {
        if ( *(ptr) != ':' )
        {
            *dst++ = *ptr++;
        }
        else
        {
            ptr++;
        }
    }

    /**/
    if ( *(dst-1) == 0x0a )
    {
        *(dst-1) = '\0';
    }
    
    *dst = '\0';
    return 0;
    
}



int  inor_init( uv_loop_t * ploop )
{
    int  iret;

    /**/
    inor_ctx.ploop = ploop;

    /**/
    iret = inor_get_macaddr( inor_ctx.gtwid );
    if ( 0 != iret )
    {
        return 7;
    }

    printf( "inor, getmac, %s\n", inor_ctx.gtwid );
    
    /**/
    iret = mqtt_init( ploop, &(inor_ctx.mqtt) );
    if ( 0 != iret )
    {
        printf( "mqtt init , ret = %d\n", iret );
        return 8;
    }

    /**/
    mqtt_set_conn_callbk( inor_ctx.mqtt, inor_mqtt_conn_cb, (intptr_t)&inor_ctx );
    mqtt_set_mesage_callbk( inor_ctx.mqtt, inor_mqtt_message_cb, (intptr_t)&inor_ctx );

    /**/
    iret = mqtt_start( inor_ctx.mqtt );
    if ( 0 != iret )
    {
        return 9;
    }


    /**/
    iret = gtw_subscribe( inor_field_change_cbk, (intptr_t)&inor_ctx, CCBK_T_REPORT );
    if ( 0 != iret )
    {
        return 3;
    }

    return 0;
    
}



