
#ifndef  __UBUS_SRV_H_
#define  __UBUS_SRV_H_


/**/
#define  SRV_CMD_GETSTATE       0xFF01
#define  SRV_CMD_CONNECT        0xFF02
#define  SRV_CMD_DISCONN        0xFF03

/**/
#define  SRV_ACK_DATA        0xFF10


/**/
int  ubussrv_start( int fd );


#endif


