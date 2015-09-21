
#ifndef  __MAIN_THRD_H_
#define  __MAIN_THRD_H_



int  mthrd_init( uint8_t * macbin, intptr_t * pret );
int  mthrd_get_sfd( intptr_t ctx, int * psfd );
int  mthrd_run( intptr_t ctx );


#endif


