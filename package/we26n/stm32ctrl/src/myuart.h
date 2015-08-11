
#ifndef  _MY_UART_H_
#define  _MY_UART_H_


typedef int (* uartcb_func)( intptr_t arg, int tlen, void * pdat );


int  myuart_init( int sno, intptr_t * pret );
int  myuart_send( intptr_t ctx, int tlen, uint8_t * pdat );
int  myuart_clear( intptr_t ctx );

int  myuart_set_callback( intptr_t ctx, uartcb_func func, intptr_t arg );
int  myuart_run( intptr_t ctx );

int  myuart_get_fd( intptr_t ctx, int * pfd );


#endif


