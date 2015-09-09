
#ifndef  __MY_MQTT_H__
#define  __MY_MQTT_H__


/**/
#define  MT_ACT_GET     1
#define  MT_ACT_SET     2
#define  MT_ACT_NTC     3


#define  MT_OBJ_GATE    1
#define  MT_OBJ_470     2
#define  MT_OBJ_BLE     3
#define  MT_OBJ_ZIG     4
#define  MT_OBJ_WIFI    5



/**/
typedef struct _tag_mmqt_msg
{
    uint8_t action;
    uint8_t object;
    char  msg[2];

} mmqt_msg_t;




/**/
int  mmqt_init( intptr_t qctx, char * ipdr, int port, intptr_t * pret );

int  mmqt_set_user( intptr_t ctx, char * user );
int  mmqt_get_fd( intptr_t ctx, int * pfd );
int  mmqt_start_run( intptr_t ctx );

int  mmqt_publish( intptr_t ctx, char * topic, char * msg );
int  mmqt_notice( intptr_t ctx, char * mod, char * msg );


#endif


