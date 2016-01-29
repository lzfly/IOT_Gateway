
#ifndef  __ISOUTH_H_
#define  __ISOUTH_H_


int  isou_report_attr( char * did, uint16_t uuid, void * pval );
int  isou_add_device( char * did, intptr_t ahd, intptr_t * pret );
int  isou_gateway_notify_linkdevice( void );

int  isou_init( uv_loop_t * ploop );


#endif

