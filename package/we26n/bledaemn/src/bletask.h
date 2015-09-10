
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


/* ˫������, �������Ӧ���� CMD, ����. */

#define  CMD_GET_STATE      0xE1                    /* CYBLE_STATE_T */

#define  CMD_STRT_SCAN      0xA5                    /* ��Ӧ, 1 byte, 0 �ɹ�, ������ʾʧ��. */
#define  CMD_STOP_SCAN      0x5A                    /* ��Ӧ, 1 byte, 0 �ɹ�, ������ʾʧ��. */

#define  CMD_CONNECT        0x3C                    /* ��Ӧ, 1 byte, 0 �ɹ�, ������ʾʧ��. */
#define  CMD_DISCONNT       0xC3                    /* ��Ӧ, 1 byte, 0 �ɹ�, ������ʾʧ��. */

#define  CMD_ENA_NOTY       0xD2                    /* ����, 2 byte, attr desc handle; ��Ӧ, 1byte , 0 �ɹ�, ������ʾʧ��. */
#define  CMD_WRT_NRSP       0x2D                    /* ����, 2 + var byte, attr handle + var byte; ��Ӧ, 1byte , 0 �ɹ�, ������ʾʧ��. */


/* ���� event �ϱ�. */
#define  CMD_EVT_PEER       0x87                    /* SCANING ״̬��, �ϱ�ɨ�赽�� peer ��ַ. */
#define  CMD_EVT_NOTY       0x78                    /* CONNECT ״̬��, �ϱ��� notify ������Ϣ. */
#define  CMD_EVT_DISC       0x69                    /* CONNECT ״̬��, �ϱ� peer �����Ͽ�����. */


/************************************************************************************************/




typedef int  (*task_cbk_func)( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat );


/**/
int  task_init( struct event_base * pevbase, intptr_t uartctx, int usize, intptr_t * pret );
int  task_inherit( intptr_t oldctx, int usize, intptr_t * pret );
int  task_getptr( intptr_t tctx, void ** pptr );
int  task_active( intptr_t tctx, uint8_t cmd, uint8_t tlen, uint8_t * pdat, task_cbk_func func, uint32_t tms );
int  task_reactive( intptr_t tctx, uint8_t cmd, uint8_t tlen, uint8_t * pdat, task_cbk_func func, uint32_t tms );


#endif

