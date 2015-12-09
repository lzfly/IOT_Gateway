
#ifndef  __MY_MQTT_H__
#define  __MY_MQTT_H__


#define   TARGET_TOGTW      0x11
#define   TARGET_TODEV      0x44


#define   DEVID_MAX_LEN     49
#define   ATTID_MAX_LEN      9
#define   VALUE_MAX_LEN     19

#define   GTW_MSG_MAX_LEN   80


/**/
typedef struct _tag_mmqt_msg
{
    int  target;

    union {
        struct _tag_todev
        {
            char  devid[DEVID_MAX_LEN+1];
            char  attid[ATTID_MAX_LEN+1];
            char  value[VALUE_MAX_LEN+1];
        } todev;

        struct _tag_togtw
        {
            char  msge[GTW_MSG_MAX_LEN+1];
        } togtw;
        
    } payload;
    
} mmqt_msg_t;




/**/
int  mmqt_init( intptr_t qctx, char * ipdr, int port, intptr_t * pret );

int  mmqt_set_user( intptr_t ctx, char * user );
int  mmqt_get_fd( intptr_t ctx, int * pfd );
int  mmqt_start_run( intptr_t ctx );

int  mmqt_publish( intptr_t ctx, char * topic, char * msg );
int  mmqt_notice( intptr_t ctx, char * mod, char * msg );


#endif


