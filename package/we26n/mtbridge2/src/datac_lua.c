
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <alloca.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <uv.h>

#include "chgcbk.h"
#include "atthead.h"
#include "gateway.h"


typedef struct _tag_waitchg_context
{
    /**/
    lua_State * corte;
    
    /**/
    uint16_t uuid;
    char  did[2];
    
} waitchg_context_t;


int  waitchg_fini( intptr_t wctx )
{
    waitchg_context_t * pctx;

    /**/
    pctx = (waitchg_context_t *)wctx;
    free( pctx );
    return 0;
}


static void waitchg_change_cbk( intptr_t wctx, intptr_t tdat )
{
    waitchg_context_t * pctx;
    double  value;
    
    /**/
    pctx = (waitchg_context_t *)wctx;

    /**/
    tdata_unsubscribe( tdat, wctx, CCBK_T_BOTH );
    tdata_get_double( tdat, &value );
    
    /**/
    lua_pushnumber( pctx->corte, value );
    lua_resume( pctx->corte, 1 );
    
    /**/
    waitchg_fini( wctx );
    return;
}


int  waitchg_subs( intptr_t wctx, lua_State * L, int timeout )
{
    int  iret;
    intptr_t  tdat;
    waitchg_context_t * pctx;

    /**/
    pctx = (waitchg_context_t *)wctx;

    /**/
    pctx->corte = L;

    /**/
    iret = gtw_search_tdata( pctx->did, pctx->uuid, &tdat );
    if ( 0 != iret )
    {
        return luaL_error( L, "gtw search data ret =%d", iret );
    }

    /**/
    iret = tdata_subscribe( tdat, waitchg_change_cbk, wctx, CCBK_T_BOTH );
    if ( 0 != iret )
    {
        return 1;
    }
    
    return 0;
    
}


int  waitchg_init( const char * did, uint16_t uuid, intptr_t * pret )
{
    waitchg_context_t * pctx;

    /**/
    pctx = (waitchg_context_t *)malloc( sizeof(waitchg_context_t) + strlen(did) );
    if ( NULL == pctx )
    {
        return 1;
    }

    /**/
    pctx->uuid = uuid;
    strcpy( pctx->did, did );

    /**/
    *pret = (intptr_t)pctx;
    return 0;
}





#define MODNAME		"datac"


/*[{ name, uuid, type }, ... ]*/

static int helper_convert_field( lua_State * L, ahd_field_t * pfield )
{
    int  top;
    int  len;
    top = lua_gettop( L );

    /**/
    lua_getfield( L, top, "name" );
    if ( LUA_TSTRING != lua_type(L,-1) )
    {
        return 1;
    }

    len = lua_objlen( L, -1 );
    if ( len > 30 )
    {
        return 2;
    }

    strcpy( pfield->name, lua_tostring(L,-1) );
    lua_pop( L, 1 );

    /**/
    lua_getfield( L, top, "uuid" );
    if ( LUA_TNUMBER != lua_type(L,-1) )
    {
        return 3;
    }

    pfield->uuid = (uint16_t)lua_tointeger( L, -1 );
    lua_pop( L, 1 );
    
    /**/
    lua_getfield( L, top, "type" );
    if ( LUA_TNUMBER != lua_type(L,-1) )
    {
        return 3;
    }

    pfield->type = lua_tointeger( L, -1 );
    lua_pop( L, 1 );
    
    return 0;
    
}


static int datac_ahd_alloc( lua_State * L )
{
    int  iret;
    int  i;
    int  top;
    intptr_t  ahd;
    int  num;
    ahd_field_t * pfield;

    top = lua_gettop(L);
    if ( top != 1 )
    {
        return luaL_error( L, "arg num must is 1" );
    }
    
    /**/
    if ( LUA_TTABLE != lua_type(L,1) )
    {
        return luaL_error( L, "arg1 type must is table(array)" );
    }
    
    num = lua_objlen( L, 1 );
    if ( num <= 0 )
    {
        return luaL_error( L, "arg1 objlen must bigger than 0" );
    }

    /**/
    pfield = (ahd_field_t *)alloca( sizeof(ahd_field_t) * num );
    
    /**/
    for ( i=0; i<num; i++ )
    {
        lua_pushinteger( L, (lua_Integer)(i+1) );
        lua_gettable( L, 1 );
        iret = helper_convert_field( L, &(pfield[i]) );
        lua_pop( L, 1 );

        /**/
        if ( 0 != iret )
        {
            return luaL_error( L, "convert field %d, ret is %d", i, iret );
        }
    }
    
    /**/
    iret = ahd_init( num, pfield, &ahd );
    if ( 0 != iret )
    {
        return luaL_error( L, "ahd init ret is %d", iret );
    }
    
    /**/
    printf( "datac ahd alloc, %x\n", ahd);
    lua_pushlightuserdata( L, (void *)ahd );
    return 1;
    
}


/* name(string), ahd(light userdata)  */
static int datac_gtw_adddev( lua_State * L )
{
    int  iret;
    int  top;
    intptr_t  dctx;
    
    top = lua_gettop( L );
    if ( top != 2 )
    {
        return luaL_error( L, "arg num must is 2" );
    }
    
    /**/
    if ( LUA_TSTRING != lua_type( L, 1 ) )
    {
        return luaL_error( L, "arg1 type must is string" );
    }
    
    /**/
    if ( LUA_TLIGHTUSERDATA != lua_type( L, 2 ) )
    {
        return luaL_error( L, "arg2 type must is light userdata" );
    }

    /**/
    printf( "datac gtw adddev, %x\n", (intptr_t)lua_touserdata( L, 2 ) );
    
    /**/
    iret = gtw_add_device( lua_tostring( L, 1 ), (intptr_t)lua_touserdata( L, 2 ), &dctx );
    if ( 0 != iret )
    {
        return luaL_error( L, "gtw add device, ret = %d", iret );
    }

    /**/
    lua_pushlightuserdata( L, (void *)dctx );    
    return 1;
    
}


/* did(string), uuid(number)  */
static int  datac_gtw_waitchg( lua_State * L )
{
    int  iret;
    int  top;
    const char * did;
    uint16_t  uuid;
    intptr_t  wctx;
    
    /**/
    top = lua_gettop( L );
    if ( top != 2 )
    {
        return luaL_error( L, "arg num must is 2" );
    }
    
    /**/
    if ( LUA_TSTRING != lua_type( L, 1 ) )
    {
        return luaL_error( L, "arg1 type must is string" );
    }
    
    /**/
    if ( LUA_TNUMBER != lua_type( L, 2 ) )
    {
        return luaL_error( L, "arg2 type must is number" );
    }

    /**/
    did = lua_tostring( L, 1 );
    uuid = (uint16_t)lua_tointeger( L, 2 );

    /**/
    iret = waitchg_init( did, uuid, &wctx );
    if ( 0 != iret )
    {
        return luaL_error( L, "waitchg init ret =%d", iret );    
    }

    iret = waitchg_subs( wctx, L, 0 );
    if ( 0 != iret )
    {
        waitchg_fini( wctx );
        return luaL_error( L, "waitchg subs ret =%d", iret );    
    }

    printf( "datac waitchg, end\n" );
    
    /**/
    return lua_yield( L, 0 );
    
}


/* did(string), uuid(number)  */
static int  datac_gtw_getattr( lua_State * L )
{
    int  iret;
    int  top;
    const char * did;
    uint16_t  uuid;
    double  value;
    intptr_t  tdat;
    
    /**/
    top = lua_gettop( L );
    if ( top != 2 )
    {
        return luaL_error( L, "arg num must is 2" );
    }
    
    /**/
    if ( LUA_TSTRING != lua_type( L, 1 ) )
    {
        return luaL_error( L, "arg1 type must is string" );
    }
    
    /**/
    if ( LUA_TNUMBER != lua_type( L, 2 ) )
    {
        return luaL_error( L, "arg2 type must is number" );
    }

    /**/
    did = lua_tostring( L, 1 );
    uuid = (uint16_t)lua_tointeger( L, 2 );

    /**/
    iret = gtw_search_tdata( did, uuid, &tdat );
    if ( 0 != iret )
    {
        return luaL_error( L, "gtw search data ret =%d", iret );
    }

    /**/
    iret = tdata_get_double( tdat, &value );
    if ( 0 != iret )
    {
        return luaL_error( L, "tdata get double ret =%d", iret );
    }

    /**/
    lua_pushnumber( L, value );
    return 1;
    
}


/* did(string), uuid(number)  */
static int  datac_gtw_setattr( lua_State * L )
{
    int  iret;
    int  top;
    const char * did;
    uint16_t  uuid;
    double  value;    
    intptr_t  tdat;
    
    /**/
    top = lua_gettop( L );
    if ( top != 3 )
    {
        return luaL_error( L, "arg num must is 3" );
    }
    
    /**/
    if ( LUA_TSTRING != lua_type( L, 1 ) )
    {
        return luaL_error( L, "arg1 type must is string" );
    }
    
    /**/
    if ( LUA_TNUMBER != lua_type( L, 2 ) )
    {
        return luaL_error( L, "arg2 type must is number" );
    }

    /**/
    if ( LUA_TNUMBER != lua_type( L, 3 ) )
    {
        return luaL_error( L, "arg3 type must is number" );
    }
    

    /**/
    did = lua_tostring( L, 1 );
    uuid = (uint16_t)lua_tointeger( L, 2 );
    value = lua_tonumber( L, 3 );
    
    /**/
    iret = gtw_search_tdata( did, uuid, &tdat );
    if ( 0 != iret )
    {
        return luaL_error( L, "gtw search data ret =%d", iret );
    }

    /**/
    tdata_action( tdat, value );
    return 0;
    
}


void  datac_timer_close_cb( uv_handle_t* handle )
{
    free( handle );
    return;
}


void  datac_timer_cb( uv_timer_t * timer )
{
    lua_State * L;

    /**/
    L = (lua_State *)timer->data;
    uv_close( (uv_handle_t *)timer, datac_timer_close_cb );
    
    /**/
    lua_resume( L, 0 );
    return;
}


static int datac_delay( lua_State * L )
{
    uv_loop_t * loop;
    uv_timer_t * timer;
    int  top;
    int32_t  mills;
    
    /**/
    top = lua_gettop( L );
    if ( top != 1 )
    {
        return luaL_error( L, "arg num must is 1" );
    }
    
    /**/
    if ( LUA_TNUMBER != lua_type( L, 1 ) )
    {
        return luaL_error( L, "arg2 type must is number" );
    }

    mills = (int32_t)lua_tointeger( L, 1 );
    if ( mills <= 0 )
    {
        return 0;
    }
    
    /**/
    lua_getglobal( L, "auvLoop" );
    loop = (uv_loop_t *)lua_topointer( L, -1 );
    lua_pop( L, 1 );

    /**/
    timer = (uv_timer_t *)malloc( sizeof(uv_timer_t) );
    uv_timer_init( loop, timer );
    uv_timer_start( timer, datac_timer_cb, (uint64_t)mills, 0 );
    timer->data = (void *)L;

    /**/
    return lua_yield( L, 0 );
    
}



static const luaL_Reg datac_funcs[] = 
{
    { "ahd_alloc", datac_ahd_alloc },
    
    { "gtw_adddev", datac_gtw_adddev },

    { "wait_change", datac_gtw_waitchg },
    
    { "get_attr", datac_gtw_getattr },
    
    { "set_attr", datac_gtw_setattr },

    { "delay", datac_delay },
    
    { NULL, NULL }
};



int luaopen_dataclib( lua_State * L )
{
    /**/
    luaL_register( L, MODNAME, datac_funcs );

	/* set some enum defines */	
	lua_pushinteger(L, AHD_UINT32 );
	lua_setfield(L, -2, "UINT32" );

	lua_pushinteger(L, AHD_UINT64 );
	lua_setfield(L, -2, "UINT64" );

	lua_pushinteger(L, AHD_FLOAT );
	lua_setfield(L, -2, "FLOAT" );	

	lua_pushinteger(L, AHD_DOUBLE );
	lua_setfield(L, -2, "DOUBLE");

    /**/
    return 1;
    
}




