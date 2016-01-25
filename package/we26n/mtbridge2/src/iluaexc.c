#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <uv.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>


/**/
typedef struct  _tag_ilua_context
{
    uv_loop_t * ploop;
    lua_State * L;
    
} ilua_context_t;


ilua_context_t  ilua_ctx;



/**/
int  ilua_add_instance( char * key, char * code )
{
    int  iret;
    lua_State * L;

    /**/
    lua_getglobal( ilua_ctx.L, "ifttt_threads" );
    L = lua_newthread( ilua_ctx.L );
    lua_setfield( ilua_ctx.L, -2, key );
    lua_pop( ilua_ctx.L, 1 );
    
    /**/
    iret = luaL_loadstring(L, code );
    if ( 0 != iret )
    {
        const char * str = lua_tostring(L, -1);
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
    iret = lua_resume( L, 0 );
    if ( LUA_YIELD == iret )
    {
        return 0;
    }

    /**/
    if ( 0 != iret )
    {
        const char * str = lua_tostring(L, -1);
        printf( "lua pcall error:\n %s\n", str );
        return 4;
    }
    
    return 0;
    
}


int  ilua_del_instance( char * key )
{
    return 0;
}

extern int luaopen_dataclib( lua_State * L );

int  ilua_run_once( char * code )
{
    int  iret;
    lua_State * L;
    
    /**/
    L = lua_newthread( ilua_ctx.L );
    lua_setglobal( ilua_ctx.L, "once_thread" );
    
    /**/
    iret = luaL_loadstring(L, code );
    if ( 0 != iret )
    {
        const char * str = lua_tostring(L, -1);
        printf( "lua load string error:\n %s\n", str );
        return 4;
    }
    
    iret = lua_pcall( L, 0, LUA_MULTRET, 0 );
    if ( 0 != iret )
    {
        const char * str = lua_tostring(L, -1);
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



