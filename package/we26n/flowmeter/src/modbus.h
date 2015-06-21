
#ifndef  __MOD_BUS_H__
#define  __MOD_BUS_H__

typedef int (* modbus_cb_func)( intptr_t arg, int tlen, void * pdat );

/**/
int  modbus_init( int tsize, intptr_t * pret );
int  modbus_fini( intptr_t  ctx );

int  modbus_set_callback( intptr_t ctx, modbus_cb_func func, intptr_t arg );
int  modbus_recv_dec( intptr_t ctx, int tlen, uint8_t * pdat );

/**/
int  modbus_rreg_enc( uint16_t addr, uint16_t num, int tmax, uint8_t * pdat, int * pret );


#endif


