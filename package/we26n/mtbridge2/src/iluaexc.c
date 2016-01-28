#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <uv.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/**/
#include "iluaexc.h"

/**/
typedef struct  _tag_ilua_context
{
    uv_loop_t * ploop;
    lua_State * L;
    
} ilua_context_t;


ilua_context_t  ilua_ctx;




typedef struct _tag_wait_context
{
    wait_fini_cbk_f  fini_func;
    uint8_t  dat[2];
    
} wait_context_t;




/**/
int  wait_init( int size, wait_fini_cbk_f  func, intptr_t * pret )
{
    wait_context_t * pctx;

    /**/
    pctx = (wait_context_t *)malloc( sizeof(wait_context_t) + size );
    if ( NULL == pctx )
    {
        return 1;
    }

    /**/
    pctx->fini_func = func;

    /**/
    *pret = (intptr_t)pctx;
    return 0;
}


int  wait_fini( intptr_t ctx )
{
    wait_context_t * pctx;

    /**/
    pctx = (wait_context_t *)ctx;

    /**/
    pctx->fini_func( ctx );

    /**/
    free( pctx );
    return 0;
}


int  wait_getptr( intptr_t ctx, void ** pret )
{
    wait_context_t * pctx;

    /**/
    pctx = (wait_context_t *)ctx;

    /**/
    *pret =  (void *)(pctx->dat);
    return 0;
}


int  ilua_del_instance( const char * key )
{
    lua_State * corte;
    intptr_t  wctx;
    
    /**/
    lua_getglobal( ilua_ctx.L, "ifttt_threads" );
    lua_getfield( ilua_ctx.L, -1, key );

    /**/
    if ( lua_isnil( ilua_ctx.L, -1 ) )
    {
        return 1;
    }

    /**/
    if ( 0 == lua_isthread( ilua_ctx.L, -1 ) )
    {
        return 2;
    }

    corte = lua_tothread( ilua_ctx.L, -1 );
    lua_pop( ilua_ctx.L, 1 );
    
    /**/
    lua_getglobal( corte, "curWait" );
    if ( lua_islightuserdata( corte, -1) )
    {
        wctx = (intptr_t)lua_touserdata( corte, -1 );
        wait_fini( wctx );
    }

    /**/
    lua_pushnil( ilua_ctx.L );
    lua_setfield( ilua_ctx.L, -2, key );
    lua_pop( ilua_ctx.L, 1 );    
    return 0;
    
}



int  ilua_clr_instance( void )
{
    const char * key;
    
    /**/
    lua_getglobal( ilua_ctx.L, "ifttt_threads" );
        
    while ( 1 )
    {
        lua_pushnil( ilua_ctx.L );
        if ( 0 == lua_next( ilua_ctx.L, -2 ) )
        {
            break;
        }

        /**/
        lua_pop( ilua_ctx.L, 1 );
        key = lua_tostring( ilua_ctx.L, -1 );
        printf( "del ifttt, %s\n", key );
        ilua_del_instance( key );
        lua_pop( ilua_ctx.L, 1 );        
    }

    return 0;
    
}


/**/
int  ilua_add_instance( const char * key, char * code )
{
    int  iret;
    lua_State * corte;

    /**/
    ilua_del_instance( key );
    
    /**/
    lua_getglobal( ilua_ctx.L, "ifttt_threads" );
    corte = lua_newthread( ilua_ctx.L );
    lua_setfield( ilua_ctx.L, -2, key );
    lua_pop( ilua_ctx.L, 1 );
    
    /**/
    iret = luaL_loadstring(corte, code );
    if ( 0 != iret )
    {
        const char * str = lua_tostring(corte, -1);
        printf( "lua load string error:\n %s\n", str );
        return 4;
    }

    /*
    --- thread status; 0 is OK 
    #define LUA_YIELD       1
    #define LUA_ERRRUN      2
    #define LUA_ERRSYNTAX   3
    #define LUA_ERRMEM      4
    #define LUA_ERRERR      5
    */
    iret = lua_resume( corte, 0 );
    if ( LUA_YIELD == iret )
    {
        return 0;
    }

    /**/
    if ( 0 != iret )
    {
        const char * str = lua_tostring( corte, -1 );
        printf( "lua pcall error:\n %s\n", str );
        return 4;
    }
    
    return 0;
    
}




extern int luaopen_dataclib( lua_State * L );

int  ilua_run_once( char * code )
{
    int  iret;
    lua_State * corte;
    
    /**/
    corte = lua_newthread( ilua_ctx.L );
    lua_setglobal( ilua_ctx.L, "once_thread" );
    
    /**/
    iret = luaL_loadstring(corte, code );
    if ( 0 != iret )
    {
        const char * str = lua_tostring( corte, -1);
        printf( "lua load string error:\n %s\n", str );
        return 4;
    }
    
    iret = lua_pcall( corte, 0, LUA_MULTRET, 0 );
    if ( 0 != iret )
    {
        const char * str = lua_tostring( corte, -1);
        printf( "lua pcall error:\n %s\n", str );
        return 4;
    }

    return 0;
    
}


/**/
int  ilua_init( uv_loop_t * ploop )
{
    lua_State * L;

    /**/
    L = luaL_newstate();
    luaL_openlibs( L );

    /**/
    lua_pushlightuserdata( L, (void *)ploop );
    lua_setglobal( L, "auvLoop" );

    /**/
    lua_newtable( L );
    lua_setglobal( L, "ifttt_threads" );
    
    /**/
    luaopen_dataclib( L );
    lua_setglobal( L, "datac" );

    /**/
    ilua_ctx.ploop = ploop;
    ilua_ctx.L = L;
    return 0;
}






