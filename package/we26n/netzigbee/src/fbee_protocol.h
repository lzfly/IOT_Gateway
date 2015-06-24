#ifndef FBEEPROTOCOL_H
#define FBEEPROTOCOL_H

#include <stddef.h>

#define SRPC_CMD_ID_POS  0
#define SRPC_CMD_LEN_POS 1
#define HEART_BEAT  1

/*CONTROL COMMAND TO GATEWAY*/
#define RPCS_GET_DEVICES              0x81
#define RPCS_SET_DEV_STATE            0x82
#define RPCS_SET_DEV_LEVEL            0x83
#define RPCS_SET_DEV_COLOR            0x84
#define RPCS_GET_DEV_STATE            0x85
#define RPCS_GET_DEV_LEVEL            0x86
#define RPCS_GET_DEV_HUE              0x87
#define RPCS_GET_DEV_SAT              0x88
#define RPCS_BIND_DEVICES             0x89
#define RPCS_GET_THERM_READING        0x8a
#define RPCS_GET_POWER_READING        0x8b
#define RPCS_DISCOVER_DEVICES         0x8c
#define RPCS_SEND_ZCL                 0x8d
#define RPCS_GET_GROUPS               0x8e
#define RPCS_ADD_GROUP                0x8f
#define RPCS_GET_SCENES               0x90
#define RPCS_ADD_SCENE                0x91
#define RPCS_RECALL_SCENE             0x92
#define RPCS_IDENTIFY_DEVICE          0x93
#define RPCS_CHANGE_DEVICE_NAME       0x94
#define RPCS_DELETE_DEVICE            0x95
#define RPCS_UN_BIND                  0x96
#define RPCS_DELETE_DEVICE_FROM_GROUP 0x97
#define RPCS_GET_GROUP_MEMBERS        0x98
#define RPCS_GET_TIME_TASK            0x99
#define RPCS_ADD_TIME_TASK            0x9A
#define RPCS_DELETE_TIME_TASK         0x9B
#define RPCS_MODIFY_TIME_TASK         0x9C
#define RPCS_GET_GATEDETAIL           0x9D
#define RPCS_SET_DEVICE_REPORT_INT    0x9E
#define RPCS_ALLOW_ADDDEVICES         0x9F
#define RPCS_CTRL_QUERY               0xA0
#define RPCS_FACTORY_GATEWAY          0xA1
#define RPCS_GET_RSSI                 0xA2
#define RPCS_ADD_TASK                 0xA3
#define RPCS_DELETE_TASK              0xA4
#define RPCS_VIEW_TASK                0xA5
#define RPCS_GET_TASK                 0xA6
#define RPCS_IRDA                     0xA7
#define RPCS_SET_COLORTMP             0xA8
#define RPCS_GET_COLORTMP             0xA9
#define RPCS_CHANGE_GATEWAY_CHAR      0xAB

/*MESSAGE FROM GATEWAY*/
#define RPCS_GET_DEVICES_RSP              0x01

#define RPCS_GET_DEV_STATE_RSP            0x07
#define RPCS_GET_DEV_LEVEL_RSP            0x08
#define RPCS_GET_DEV_HUE_RSP              0x09
#define RPCS_GET_DEV_SAT_RSP              0x0A
#define RPCS_ADD_GROUP_RSP                0x0B
#define RPCS_GET_GROUP_RSP                0x0C
#define RPCS_ADD_SCENE_RSP                0x0D
#define RPCS_GET_SCENE_RSP                0x0E
#define RPCS_DELETE_DEVICE_FROM_GROUP_RSP 0x0F

#define RPCS_GET_TIME_TASK_RSP            0x11
#define RPCS_ADD_TIME_TASK_RSP            0x12
#define RPCS_DELETE_TIME_TASK_RSP         0x13
#define RPCS_MODIFY_TIME_TASK_RSP         0x14
#define RPCS_GET_GATEDETAIL_RSP           0x15

#define RPCS_ADD_TASK_RSP                 0x22
#define RPCS_DELETE_TASK_RSP              0x23
#define RPCS_VIEW_TASK_RSP                0x24
#define RPCS_GET_TASK_RSP                 0x25 

#define RPCS_GET_COLORTMP_RSP             0x27

#define RPCS_COMMON_RSP                   0x29

#define RPCS_DEVICE_REPORT                0x70

#endif
