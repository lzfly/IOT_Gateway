
#ifndef  __G_THRD_H__
#define  __G_THRD_H__


int  gthrd_init( intptr_t * pret );
int  gthrd_start( intptr_t ctx );
int  gthrd_getfd( intptr_t ctx, int * pfd );

/* debug */
int  gthrd_getstat( intptr_t ctx, char * pstr );

int  gthrd_notify_mid( intptr_t ctx, uint32_t mid );
int  gthrd_notify_sock( intptr_t ctx, int sock );
int  gthrd_notify_ever( intptr_t ctx );
int  get_cb_id(void);
char cb_id[200];


typedef struct _tag_tem_msg
{
    int  attr;
    double data;
} tem_t;


#endif


