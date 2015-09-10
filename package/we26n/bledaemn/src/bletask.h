
#ifndef __BLE_TASK_H_
#define __BLE_TASK_H_

/**/
#define CMD_NULL        0x0000
#define ACK_TMR_OUT     0x0000


/************************************************************************************************/

#define  CYBLE_STATE_STOPPED            0               /* BLE is turned off */
#define  CYBLE_STATE_INITIALIZING       1               /* Initializing state */
#define  CYBLE_STATE_CONNECTED          2               /* Peer device is connected */
#define  CYBLE_STATE_SCANNING           3               /* Scanning process */
#define  CYBLE_STATE_CONNECTING         4               /* Connecting */
#define  CYBLE_STATE_DISCONNECTED       5               /* Essentially idle state */


/* 双向命令, 命令和响应共用 CMD, 如下. */

#define  CMD_GET_STATE      0xE1                    /* CYBLE_STATE_T */

#define  CMD_STRT_SCAN      0xA5                    /* 响应, 1 byte, 0 成功, 其他表示失败. */
#define  CMD_STOP_SCAN      0x5A                    /* 响应, 1 byte, 0 成功, 其他表示失败. */

#define  CMD_CONNECT        0x3C                    /* 响应, 1 byte, 0 成功, 其他表示失败. */
#define  CMD_DISCONNT       0xC3                    /* 响应, 1 byte, 0 成功, 其他表示失败. */

#define  CMD_ENA_NOTY       0xD2                    /* 命令, 2 byte, attr desc handle; 响应, 1byte , 0 成功, 其他表示失败. */
#define  CMD_WRT_NRSP       0x2D                    /* 命令, 2 + var byte, attr handle + var byte; 响应, 1byte , 0 成功, 其他表示失败. */


/* 单向 event 上报. */
#define  CMD_EVT_PEER       0x87                    /* SCANING 状态下, 上报扫描到的 peer 地址. */
#define  CMD_EVT_NOTY       0x78                    /* CONNECT 状态下, 上报的 notify 数据信息. */
#define  CMD_EVT_DISC       0x69                    /* CONNECT 状态下, 上报 peer 主动断开连接. */


/************************************************************************************************/




typedef int  (*task_cbk_func)( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat );


/**/
int  task_init( struct event_base * pevbase, intptr_t uartctx, int usize, intptr_t * pret );
int  task_inherit( intptr_t oldctx, int usize, intptr_t * pret );
int  task_getptr( intptr_t tctx, void ** pptr );
int  task_active( intptr_t tctx, uint8_t cmd, uint8_t tlen, uint8_t * pdat, task_cbk_func func, uint32_t tms );
int  task_reactive( intptr_t tctx, uint8_t cmd, uint8_t tlen, uint8_t * pdat, task_cbk_func func, uint32_t tms );


#endif

