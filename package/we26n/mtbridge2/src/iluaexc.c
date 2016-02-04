#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <uv.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/**/
#include "iluaexc.h"


typedef struct  _tag_mempool_context
{
    int  msiz;
    int  tnum;
    uint8_t * ptr;
    uint8_t  bitmap[2];
    
} mempool_context_t;


intptr_t  memp_ctx = 0;


int  mempool_init( int pow, int num, intptr_t * pret )
{
    int  size;
    uint8_t * ptr;
    mempool_context_t * pctx;

    /**/
    size = 1<<pow;
    size = size * num;

    /**/
    ptr = (uint8_t *)malloc( size );
    if ( NULL == ptr )
    {
        return 1;
    }

    /**/
    pctx = (mempool_context_t *)malloc( sizeof(mempool_context_t) + ( num / 8 ) );
    if ( NULL == pctx )
    {
        return 2;
    }

    /**/
    pctx->msiz = 1<<pow;
    pctx->tnum = num;
    pctx->ptr = ptr;
    memset( pctx->bitmap, 0xff, (num/8) + 1 );

    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}


void * mempool_alloc( intptr_t ctx, int size )
{
    int  count;
    int  i, j;
    mempool_context_t * pctx;
    
    /**/
    pctx = (mempool_context_t *)ctx;

    /**/
    if ( size > pctx->msiz )
    {
        printf( "req too large size = %d\n", size );
        return NULL;
    }
    
    /**/
    count = (pctx->tnum / 8);
    
    /**/
    for ( i=0; i<count; i++ )
    {
        if ( pctx->bitmap[i] != 0 )
        {
            break;
        }
    }

    /**/
    if ( i >= count )
    {
        printf( "req too many. leak = %d\n", size );    
        return NULL;
    }
    
    /**/
    for ( j=0; j<8; j++ )
    {
        if ( (1<<j) & pctx->bitmap[i] )
        {
            break;
        }
    }

    /**/
    pctx->bitmap[i] ^= 1<<j;
    return pctx->ptr + (pctx->msiz * ((i<<3) + j));
    
}


void  mempool_free( intptr_t ctx, void * ptr )
{
    mempool_context_t * pctx;
    uint8_t * dest;
    int  count;
    int  i, j;
    
    /**/
    pctx = (mempool_context_t *)ctx;

    /**/
    dest = (uint8_t *)ptr;
    if ( dest < pctx->ptr )
    {
        printf( "mempool free, ptr out of <<< \n" );
        return;
    }

    /**/
    count = (dest - pctx->ptr) / pctx->msiz;
    if ( count >= pctx->tnum )
    {
        printf( "mempool free, ptr out of >>> \n" );
        return;
    }

    /**/
    i = count >> 3;
    j = count & 0x7;

    /**/
    if ( (pctx->bitmap[i] & (1<<j)) != 0 )
    {
        printf( "mempool free, ptr retry free \n" );
        return;
    }

    /**/
    pctx->bitmap[i] |= 1<<j;
    return;
    
}



/**/
typedef struct  _tag_ilua_context
{
    uv_loop_t * ploop;
    lua_State * L;
    
} ilua_context_t;


ilua_context_t  ilua_ctx;




typedef struct _tag_wait_context
{
    uint32_t  magic;
    wait_fini_cbk_f  fini_func;
    uint8_t  dat[2];
    
} wait_context_t;




/**/
int  wait_init( int size, wait_fini_cbk_f  func, intptr_t * pret )
{
    wait_context_t * pctx;

    /**/
    pctx = (wait_context_t *)mempool_alloc( memp_ctx, sizeof(wait_context_t) + size );
    if ( NULL == pctx )
    {
        return 1;
    }

    /**/
    pctx->magic = 0x112233FF;
    pctx->fini_func = func;

    /**/
    printf( "wait init, %p\n", pctx );
    *pret = (intptr_t)pctx;
    return 0;
}


int  wait_fini( intptr_t ctx )
{
    wait_context_t * pctx;

    /**/
    pctx = (wait_context_t *)ctx;

    if ( pctx->magic != 0x112233FF )
    {
        printf( "wait fini, %p, %x \n", pctx, pctx->magic );
    }
    
    /**/
    pctx->fini_func( ctx );
    pctx->magic = 0;
    
    /**/
    printf( "wait fini, %p\n", pctx );
    mempool_free( memp_ctx, pctx );
    return 0;
    
}


int  wait_getptr( intptr_t ctx, void ** pret )
{
    wait_context_t * pctx;

    /**/
    pctx = (wait_context_t *)ctx;
    if ( pctx->magic != 0x112233FF )
    {
        printf( "wait getptr, %p, %x \n", pctx, pctx->magic );
    }

    /**/
    *pret =  (void *)(pctx->dat);
    return 0;
}


int  ilua_del_instance( const char * key )
{
    lua_State * corte;
    intptr_t  wctx;
    char  tstr[16];
    
    /**/
    lua_getglobal( ilua_ctx.L, "ifttt_threads" );
    lua_getfield( ilua_ctx.L, -1, key );

    /**/
    if ( lua_isnil( ilua_ctx.L, -1 ) )
    {
        lua_pop( ilua_ctx.L, 2 );
        return 1;
    }

    /**/
    if ( 0 == lua_isthread( ilua_ctx.L, -1 ) )
    {
        lua_pop( ilua_ctx.L, 2 );
        return 2;
    }

    /**/
    corte = lua_tothread( ilua_ctx.L, -1 );
    lua_pop( ilua_ctx.L, 1 );

    /**/
    printf( "del ifttt, %s, %p\n", key, corte );
    
    /**/
    sprintf( tstr, "%p", corte );
    lua_getglobal( corte, "ifttt_waits" );
    lua_getfield( corte, -1, tstr );
            
    if ( lua_islightuserdata( corte, -1 ) )
    {
        wctx = (intptr_t)lua_touserdata( corte, -1 );
        lua_pop( corte, 1 );
        
        wait_fini( wctx );

        /**/
        lua_pop( corte, 1 );

    }
    else
    {
        printf( "del ifttt, curWait, %s\n", luaL_typename( corte, -1) );
        lua_pop( corte, 2 );
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
        ilua_del_instance( key );
        lua_pop( ilua_ctx.L, 1 );        
    }

    lua_pop( ilua_ctx.L, 1 );

    /**/
//    printf( "before, gc, count = %d\n ", lua_gc( ilua_ctx.L, LUA_GCCOUNT, 0 ) );
    lua_gc( ilua_ctx.L, LUA_GCCOLLECT, 0 );
//    printf( "after, gc, count = %d\n ", lua_gc( ilua_ctx.L, LUA_GCCOUNT, 0 ) );    
    return 0;
    
}


/**/
int  ilua_add_instance( const char * key, char * code )
{
    int  iret;
    lua_State * corte;

    /**/
    // ilua_del_instance( key );
    
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

printf( "add instance, %s, %p\n", key, corte );

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
        printf( "corte lua_resume error:\n %s\n", str );
        // ilua_del_instance( key );
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
    int  iret;
    lua_State * L;

    /**/
    iret = mempool_init( 8, 8, &memp_ctx );
    if ( 0 != iret )
    {
        printf( "mempool, init fail = %d\n", iret );
        return 1;
    }
    
    /**/
    L = luaL_newstate();
    luaL_openlibs( L );

    /**/
    lua_pushlightuserdata( L, (void *)ploop );
    lua_setglobal( L, "auvLoop" );

    /**/
    lua_newtable( L );
    lua_setglobal( L, "ifttt_threads" );

    lua_newtable( L );
    lua_setglobal( L, "ifttt_waits" );
    
    /**/
    luaopen_dataclib( L );
    lua_setglobal( L, "datac" );

    /**/
    ilua_ctx.ploop = ploop;
    ilua_ctx.L = L;
    return 0;
}






