
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
#include "iluaexc.h"


typedef struct _tag_wait_field_context
{
    /**/
    lua_State * corte;

    /**/
    int  status;
    
    /**/
    uint16_t uuid;
    char  did[2];

} wait_field_context_t;



void  wait_field_fini_cb( intptr_t wctx )
{
    int  iret;
    wait_field_context_t * pctx;
    intptr_t  tdat;
    lua_State * corte;
    
    /**/
    wait_getptr( wctx, (void **)&pctx );
    
    /**/
    if ( pctx->status == WAIT_SUBS )
    {
        pctx->status = WAIT_FREE;
    
        /**/
        corte = pctx->corte;
        lua_pushnil( corte );
        lua_setglobal( corte, "curWait" );
        
        /**/
        iret = gtw_search_tdata( pctx->did, pctx->uuid, &tdat );
        if ( 0 != iret )
        {
            return;
        }
        
        /**/
        iret = tdata_unsubscribe( tdat, wctx );
        if ( 0 != iret )
        {
            return;
        }
    }
    
    /**/
    return;
    
}


static void wait_field_change_cb( intptr_t wctx, intptr_t tdat )
{
    wait_field_context_t * pctx;
    double  value;
    lua_State * corte;

    
    /**/
    wait_getptr( wctx, (void **)&pctx );

    /**/
    printf( "wait field cb cb cb, %d\n", pctx->status );

    pctx->status = WAIT_FREE;

    
    /**/
    tdata_get_double( tdat, &value );

    /**/
    corte = pctx->corte;
    lua_pushnil( corte );
    lua_setglobal( corte, "curWait" );

    /**/
    wait_fini( wctx );
    printf( "wait field cb cb 1111\n" );
    
    /**/
    lua_pushnumber( corte, value );
    lua_resume( corte, 1 );

    printf( "wait field cb cb 2222\n" );
    return;
    
}


int  wait_field_subs( const char * did, uint16_t uuid, lua_State * corte )
{
    int  iret;
    intptr_t  wctx;
    wait_field_context_t * pctx;
    intptr_t  tdat;
    
    /**/
    iret = wait_init( sizeof(wait_field_context_t) + strlen(did), wait_field_fini_cb, &wctx );
    if ( 0 != iret )
    {
        return 1;
    }
    
    /**/
    wait_getptr( wctx, (void **)&pctx );
    
    /**/
    pctx->corte = corte;
    pctx->status = WAIT_FREE;
    pctx->uuid = uuid;
    strcpy( pctx->did, did );

    /**/
    iret = gtw_search_tdata( pctx->did, pctx->uuid, &tdat );
    if ( 0 != iret )
    {
        wait_fini( wctx );
        return 2;
    }
    
    /**/
    iret = tdata_subscribe( tdat, wait_field_change_cb, wctx, (CCBK_T_BOTH | CCBK_T_ONCE) );
    if ( 0 != iret )
    {
        wait_fini( wctx );    
        return 3;
    }

    /**/
    pctx->status = WAIT_SUBS;

    /**/
    lua_pushlightuserdata( corte, (void *)wctx );
    lua_setglobal( corte, "curWait" );

    /**/
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

    printf( "datac waitchg, 1111\n" );
    
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

    printf( "datac waitchg, 2222\n" );
    
    /**/
    iret = wait_field_subs( did, uuid, L );
    if ( 0 != iret )
    {
        return luaL_error( L, "wait field subs ret =%d", iret );    
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




