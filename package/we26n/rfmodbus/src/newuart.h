
#ifndef  __NEW_UART_H_
#define  __NEW_UART_H_


typedef void (*nuart_cbk_func)( intptr_t arg, uint16_t cmd, int tlen, void * pdat );
typedef void (*nuart_data_cbk_func)( intptr_t arg, int tlen, void * pdat );


int  nuart_init( struct event_base * pevbase, int tno, intptr_t * pret );
int  nuart_send( intptr_t ctx, uint16_t cmd, int tlen, void * pdat );
int  nuart_set_callbk( intptr_t ctx, nuart_cbk_func func, intptr_t arg );

/**/
int  nuart_set_data_callbk( intptr_t ctx, nuart_data_cbk_func func, intptr_t arg );



#endif


