
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
int  modbus_proc_timeout( intptr_t ctx );


/**/
int32_t  modbus_conv_long( uint8_t * puc );
float  modbus_conv_real4( uint8_t * puc );
int32_t  modbus_conv_longm( uint8_t * puc );

#endif


