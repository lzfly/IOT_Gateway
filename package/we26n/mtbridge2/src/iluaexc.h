
#ifndef  __ILUA_EXC_H_
#define  __ILUA_EXC_H_



int  ilua_init( uv_loop_t * ploop );

int  ilua_run_once( char * code );
int  ilua_add_instance( char * key, char * code );
int  ilua_del_instance( char * key );


#endif

 
