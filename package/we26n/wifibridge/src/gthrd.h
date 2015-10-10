
#ifndef  __G_THRD_H__
#define  __G_THRD_H__


int  gthrd_init( intptr_t * pret );
int  gthrd_start( intptr_t ctx );
int  gthrd_getfd( intptr_t ctx, int * pfd );

int  gthrd_notify_mid( intptr_t ctx, uint64_t mid );
int  gthrd_notify_sock( intptr_t ctx, int sock );
int  gthrd_notify_ever( intptr_t ctx );


#endif


