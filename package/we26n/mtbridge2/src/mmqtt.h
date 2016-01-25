
#ifndef  __MMQTT_H_
#define  __MMQTT_H_


typedef void (* mqtt_conn_cbf)( intptr_t arg, int status );
typedef void (* mqtt_message_cbf)( intptr_t arg, const char * topic, int tlen, uint8_t * pdat );


int  mqtt_set_conn_callbk( intptr_t ctx, mqtt_conn_cbf func, intptr_t arg );
int  mqtt_set_mesage_callbk( intptr_t ctx, mqtt_message_cbf func, intptr_t arg );


int  mqtt_init( uv_loop_t * ploop, intptr_t * pret );
int  mqtt_start( intptr_t ctx );



#endif

