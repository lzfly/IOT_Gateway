
#ifndef  __ATT_HEAD_H__
#define  __ATT_HEAD_H__


#define  AHD_UINT32         1
#define  AHD_UINT64         2
#define  AHD_FLOAT          3
#define  AHD_DOUBLE         4

typedef struct
{
    char name[32];
    uint16_t  uuid;
    int  type;
    int  other;
    
} ahd_field_t;


int  ahd_get_maxnum( intptr_t ctx, int * pnum );
int  ahd_get_byindex( intptr_t ctx, int idx, uint16_t * puuid, int * ptype );
int  ahd_get_byuuid( intptr_t ctx, uint16_t uuid, int * pidx );
int  ahd_init( int num, ahd_field_t * pfield, intptr_t * pret );


#endif

