
#ifndef  __ILUA_EXC_H_
#define  __ILUA_EXC_H_



#define  WAIT_FREE      0
#define  WAIT_SUBS      1

typedef void (* wait_fini_cbk_f)( intptr_t wctx );

int  wait_init( int size, wait_fini_cbk_f  func, intptr_t * pret );
int  wait_fini( intptr_t ctx );
int  wait_getptr( intptr_t ctx, void ** pret );


/**/
int  ilua_init( uv_loop_t * ploop );

int  ilua_run_once( char * code );
int  ilua_add_instance( char * key, char * code );
int  ilua_del_instance( char * key );


#endif

 
