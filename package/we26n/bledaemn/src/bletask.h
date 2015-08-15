
#ifndef __BLE_TASK_H_
#define __BLE_TASK_H_


typedef int  (*task_cbk_func)( intptr_t tctx, uint8_t cmd, int tlen, void * pdat );


/**/
int  task_init( struct event_base * pevbase, intptr_t uartctx, int usize, intptr_t * pret );
int  task_inherit( intptr_t oldctx, int usize, intptr_t * pret );
int  task_getptr( intptr_t tctx, void ** pptr );
int  task_active( intptr_t tctx, uint8_t cmd, uint8_t tlen, uint8_t * pdat, task_cbk_func func, uint32_t tms );


#endif

