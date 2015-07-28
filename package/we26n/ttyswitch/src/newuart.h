
#ifndef  __NEW_UART_H_
#define  __NEW_UART_H_


typedef void (*nuart_cbk_func)( intptr_t arg, int tidx, int tlen, void * pdat );

int  nuart_init( struct event_base * pevbase, int tno, intptr_t * pret );
int  nuart_send( intptr_t ctx, int tidx, int tlen, void * pdat );
int  nuart_set_callbk( intptr_t ctx, nuart_cbk_func func, intptr_t arg );


#endif


