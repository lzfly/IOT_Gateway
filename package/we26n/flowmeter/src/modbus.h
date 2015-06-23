
#ifndef  __MOD_BUS_H__
#define  __MOD_BUS_H__

typedef int (* modbus_cb_func)( intptr_t arg, int tlen, void * pdat );

/**/
int  modbus_init( int tsize, intptr_t * pret );
int  modbus_fini( intptr_t  ctx );

int  modbus_set_uartctx( intptr_t ctx, intptr_t uctx );
int  modbus_recv_decode( intptr_t ctx, int tlen, uint8_t * pdat );

/**/
int  modbus_send_req( intptr_t ctx, uint8_t addr, uint16_t reg, uint16_t num, modbus_cb_func func, intptr_t arg );


#endif


