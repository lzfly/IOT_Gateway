
#ifndef  __PKT_INTF_H_
#define  __PKT_INTF_H_



typedef void (* pkti_cbk_func)( intptr_t arg, int tlen, void * pdat );


typedef struct
{
    /**/
    int  fd;
    pkti_cbk_func func;
    intptr_t  arg;
    
} pkt_obj_t;


/**/
typedef int (* pkti_send_func)( intptr_t obj, int tlen, void * pdat );
typedef int (* pkti_set_callbk_func)( intptr_t obj, pkti_cbk_func func, intptr_t arg );
typedef int (* pkti_get_fd_func)( intptr_t obj, int * pfd );
typedef int (* pkti_try_run)( intptr_t obj );
typedef int (* pkti_fini_func)( intptr_t obj );


/**/
typedef struct
{
    pkti_send_func pkti_send;
    pkti_set_callbk_func  pkti_set_callbk;
    pkti_get_fd_func  pkti_get_fd;
    pkti_try_run  pkti_try_run;
    pkti_fini_func  pkti_fini;
    
} pkt_intf_t;



#endif


