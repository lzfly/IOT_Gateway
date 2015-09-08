
#ifndef __BLE_TASK_H_
#define __BLE_TASK_H_

/**/

#define CMD_NULL        0x0000
#define ACK_TMR_OUT     0x0000


#define CMD_GET_VER     0x4001
#define ACK_GET_VER     0x8001

#define CMD_GET_ADR     0x4002
#define ACK_GET_ADR     0x8002

#define CMD_GET_DADR    0x4003
#define ACK_GET_DADR    0x8003

#define CMD_GET_DHOST   0x4017
#define ACK_GET_DHOST   0x8017


#define CMD_REBOOT      0x4013
#define ACK_REBOOT      0x8013

#define CMD_AUTO_CHN    0x4014
#define ACK_AUTO_CHN    0x8014
#define EVT_AUTO_CHN    0x8015

#define CMD_NODE_TYP    0x400E
#define ACK_NODE_TYP    0x800E

#define CMD_IO_PAIR     0x4022
#define ACK_IO_PAIR     0x8022
#define EVT_IO_PAIR     0x8019


#define CMD_SEND_DAT    0x400A
#define ACK_SEND_DAT    0x800A
#define EVT_SEND_DAT    0x8010


#define ACK_RECV_DAT    0x800B


/* upgrade */
#define EVT_LDR_INFO    0x00E1

#define CMD_TRANS       0x40E2
#define ACK_TRANS       0x80E2
#define ACK_TRANS_CRC   0x00E3

#define CMD_UPGRADE     0x40E4
#define ACK_UPGRADE     0x80E4



typedef int  (*task_cbk_func)( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat );


/**/
int  task_init( struct event_base * pevbase, intptr_t uartctx, int usize, intptr_t * pret );
int  task_inherit( intptr_t oldctx, int usize, intptr_t * pret );
int  task_getptr( intptr_t tctx, void ** pptr );
int  task_active( intptr_t tctx, uint16_t cmd, uint8_t tlen, uint8_t * pdat, task_cbk_func func, uint32_t tms );
int  task_reactive( intptr_t tctx, uint16_t cmd, uint8_t tlen, uint8_t * pdat, task_cbk_func func, uint32_t tms );


#endif

