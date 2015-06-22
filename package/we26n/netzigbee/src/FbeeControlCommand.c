#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include "we26n_type.h"

int SRPC_CMD_ID_POS = 0;
int SRPC_CMD_LEN_POS = 1;
w26n_byte HEART_BEAT = 1;
//SRPC CMD ID's
//define the outgoing RPSC command ID's
w26n_byte RPCS_NEW_ZLL_DEVICE = (w26n_byte)0x0001;
w26n_byte RPCS_DEV_ANNCE = (w26n_byte)0x0002;
w26n_byte RPCS_SIMPLE_DESC = (w26n_byte)0x0003;
w26n_byte RPCS_TEMP_READING = (w26n_byte)0x0004;
w26n_byte RPCS_POWER_READING = (w26n_byte)0x0005;
w26n_byte RPCS_PING = (w26n_byte)0x0006;
w26n_byte RPCS_GET_DEV_STATE_RSP = (w26n_byte)0x0007;
w26n_byte RPCS_GET_DEV_LEVEL_RSP = (w26n_byte)0x0008;
w26n_byte RPCS_GET_DEV_HUE_RSP = (w26n_byte)0x0009;
w26n_byte RPCS_GET_DEV_SAT_RSP = (w26n_byte)0x000a;
w26n_byte RPCS_ADD_GROUP_RSP = (w26n_byte)0x000b;
w26n_byte RPCS_GET_GROUP_RSP = (w26n_byte)0x000c;
w26n_byte RPCS_ADD_SCENE_RSP = (w26n_byte)0x000d;
w26n_byte RPCS_GET_SCENE_RSP = (w26n_byte)0x000e;
w26n_byte RPCS_GET_GATEDETAIL_RSP = (w26n_byte)0x15;



w26n_byte RPCS_GET_DEV_SP = (w26n_byte)0x70;
w26n_byte RPCS_CLOSE = (w26n_byte)0x80;
w26n_byte RPCS_GET_DEVICES = (w26n_byte)0x81;
w26n_byte RPCS_SET_DEV_STATE = (w26n_byte)0x82;
w26n_byte RPCS_SET_DEV_LEVEL = (w26n_byte)0x83;
w26n_byte RPCS_SET_DEV_COLOR = (w26n_byte)0x84;
w26n_byte RPCS_GET_DEV_STATE = (w26n_byte)0x85;
w26n_byte RPCS_GET_DEV_LEVEL = (w26n_byte)0x86;
w26n_byte RPCS_GET_DEV_HUE = (w26n_byte)0x87;
w26n_byte RPCS_GET_DEV_SAT = (w26n_byte)0x88;
w26n_byte RPCS_BIND_DEVICES = (w26n_byte)0x89;
w26n_byte RPCS_GET_THERM_READING = (w26n_byte)0x8a;
w26n_byte RPCS_GET_POWER_READING = (w26n_byte)0x8b;
w26n_byte RPCS_DISCOVER_DEVICES = (w26n_byte)0x8c;
w26n_byte RPCS_SEND_ZCL = (w26n_byte)0x8d;
w26n_byte RPCS_GET_GROUPS = (w26n_byte)0x8e;
w26n_byte RPCS_ADD_GROUP = (w26n_byte)0x8f;
w26n_byte RPCS_GET_SCENES = (w26n_byte)0x90;
w26n_byte RPCS_STORE_SCENE = (w26n_byte)0x91;
w26n_byte RPCS_RECALL_SCENE = (w26n_byte)0x92;
w26n_byte RPCS_IDENTIFY_DEVICE = (w26n_byte)0x93;
w26n_byte RPCS_CHANGE_DEVICE_NAME = (w26n_byte)0x94;

w26n_byte RPCS_DELETE_DEVICE= (w26n_byte)0x95;

w26n_byte RPCS_GET_GATEDETAIL = (w26n_byte)0x9D;
w26n_byte RPCS_ALLOW_ADDDEVICES= (w26n_byte)0x9F;

w26n_byte RPCS_FACTORY_GATEWAY= (w26n_byte)0xA1;
