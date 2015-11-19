
#ifndef  __G_THRD_H__
#define  __G_THRD_H__


int  gthrd_init( intptr_t * pret );
int  gthrd_start( intptr_t ctx );
int  gthrd_getfd( intptr_t ctx, int * pfd );

/* debug */
int  gthrd_getstat( intptr_t ctx, char * pstr );

/**/
int  gthrd_notify_sock( intptr_t ctx, int sock );
int  gthrd_set_target_tem( intptr_t ctx, uint32_t mid );
int  gthrd_get_ctrl_status( intptr_t ctx );
int  gthrd_get_tem( intptr_t ctx );

int  gthrd_set_rem_tem( intptr_t ctx, uint32_t  rem_tem );
int  gthrd_set_ctrl_mode( intptr_t ctx, uint32_t  ctrl_mode );



int  get_cb_id(void);
char cb_id[200];


typedef struct _tag_tem_msg
{
    int  attr;
    double data;
} tem_t;


#endif


