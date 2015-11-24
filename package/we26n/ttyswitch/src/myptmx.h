
#ifndef  __NEW_PTMX_H_
#define  __NEW_PTMX_H_


typedef void (*ptmx_cbk_func)( intptr_t arg, int tlen, void * pdat );

int  ptmx_init( struct event_base * pevbase, int tno, intptr_t * pret );
int  ptmx_send( intptr_t ctx, int tlen, void * pdat );
int  ptmx_set_callbk( intptr_t ctx, ptmx_cbk_func func, intptr_t arg );
int  ptmx_pktmode( intptr_t ctx, int mode );
int  ptmx_getspeed( intptr_t ctx, uint32_t * pspeed );


#endif


