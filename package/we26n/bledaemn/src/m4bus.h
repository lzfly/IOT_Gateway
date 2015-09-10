
#ifndef  __MFOUR_BUS_H_
#define  __MFOUR_BUS_H_


typedef struct _tag_m4bus_rsp
{
    uint8_t  addr;
    uint16_t  cmd;
    uint16_t  uret;
    uint8_t  tary[0];
    
} m4bus_rsp_t;



typedef struct _tag_m4bus_req
{
    uint8_t  addr;
    uint16_t  cmd;
    int  tlen;
    uint8_t  tary[0];
    
} m4bus_req_t;


typedef void (* m4bus_cbk_func)( intptr_t arg, uint16_t uret, int tlen, uint8_t * pdat );

int  m4bus_send_req( intptr_t ctx, m4bus_req_t * preq, m4bus_cbk_func func, intptr_t arg, int tms );
int  m4bus_get_fd( intptr_t ctx, int * pfd );
int  m4bus_try_run( intptr_t ctx );
int  m4bus_init( int fd, intptr_t * pret );


#endif

