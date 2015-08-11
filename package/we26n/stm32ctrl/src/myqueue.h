
#ifndef  _MY_SQUEUE_H_
#define  _MY_SQUEUE_H_




int  msq_init( intptr_t * pret );
int  msq_fini( intptr_t ctx );

int  msq_getefd( intptr_t ctx, int * pret );

int  msq_enqueue( intptr_t ctx, int tlen, void * pdat );
int  msq_dequeue( intptr_t ctx, void ** ppdat );




#endif
