
#ifndef  __MQT_PACKET_H__
#define  __MQT_PACKET_H__

/* Message types */
#define MQTT_CONNECT        0x10
#define MQTT_CONNACK        0x20
#define MQTT_PUBLISH        0x30
#define MQTT_PUBACK         0x40
#define MQTT_PUBREC         0x50
#define MQTT_PUBREL         0x60
#define MQTT_PUBCOMP        0x70
#define MQTT_SUBSCRIBE      0x80
#define MQTT_SUBACK         0x90
#define MQTT_UNSUBSCRIBE    0xA0
#define MQTT_UNSUBACK       0xB0
#define MQTT_PINGREQ        0xC0
#define MQTT_PINGRESP       0xD0
#define MQTT_DISCONNECT     0xE0

#define MQTT_CONNACK_ACCEPTED                       0
#define MQTT_CONNACK_REFUSED_PROTOCOL_VERSION       1
#define MQTT_CONNACK_REFUSED_IDENTIFIER_REJECTED    2
#define MQTT_CONNACK_REFUSED_SERVER_UNAVAILABLE     3
#define MQTT_CONNACK_REFUSED_BAD_USERNAME_PASSWORD  4
#define MQTT_CONNACK_REFUSED_NOT_AUTHORIZED         5


/**/
int  mpkt_conn_alloc( int vsize, intptr_t * pret );
int  mpkt_conn_set_protocol_level( intptr_t ctx, uint8_t level );
int  mpkt_conn_set_clean_session( intptr_t ctx, int flag );
int  mpkt_conn_set_client_ident( intptr_t ctx, char * client );
int  mpkt_conn_add_will_message( intptr_t ctx, char * topic, char * msage );
int  mpkt_conn_add_name_passwd( intptr_t ctx, char * name, char * passwd );

/**/
int  mpkt_conack_get_retcode( intptr_t ctx, uint8_t * pret );

/**/
int  mpkt_publ_set_topic( intptr_t ctx, char * topic );
int  mpkt_publ_set_message( intptr_t ctx, char * mesge );
int  mpkt_publ_get_topic( intptr_t ctx, uint8_t ** ptr, int * plen );
int  mpkt_publ_get_message( intptr_t ctx, uint8_t ** ptr, int * plen );

/**/
int  mpkt_subs_set_pkt_id( intptr_t ctx, uint16_t idt );
int  mpkt_subs_add_topic( intptr_t ctx, char * topic );

/**/
int  mpkt_alloc( uint8_t header, int rsize, intptr_t * pret );
int  mpkt_input( intptr_t ctx, uint8_t );
int  mpkt_free( intptr_t ctx );
int  mpkt_iovec( intptr_t ctx, uv_buf_t ** pbufs, int * pnum );


#endif

