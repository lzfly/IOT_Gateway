#ifndef FBEECONTROLCOMMAND_H
#define FBEECONTROLCOMMAND_H

#include <stddef.h>
#include "we26n_type.h"

  extern int SRPC_CMD_ID_POS;
  extern int SRPC_CMD_LEN_POS ;
  extern w26n_byte HEART_BEAT;
//SRPC CMD ID's
//define the outgoing RPSC command ID's
  extern w26n_byte RPCS_NEW_ZLL_DEVICE ;
  extern w26n_byte RPCS_DEV_ANNCE;
  extern w26n_byte RPCS_SIMPLE_DESC;
  extern w26n_byte RPCS_TEMP_READING;
  extern w26n_byte RPCS_POWER_READING;
  extern w26n_byte RPCS_PING;
  extern w26n_byte RPCS_GET_DEV_STATE_RSP;
  extern w26n_byte RPCS_GET_DEV_LEVEL_RSP;
  extern w26n_byte RPCS_GET_DEV_HUE_RSP ;
  extern w26n_byte RPCS_GET_DEV_SAT_RSP;
  extern w26n_byte RPCS_ADD_GROUP_RSP;
  extern w26n_byte RPCS_GET_GROUP_RSP;
  extern w26n_byte RPCS_ADD_SCENE_RSP;
  extern w26n_byte RPCS_GET_SCENE_RSP;
  extern w26n_byte RPCS_GET_GATEDETAIL_RSP;



  extern w26n_byte RPCS_GET_DEV_SP ;
  extern w26n_byte RPCS_CLOSE;
  extern w26n_byte RPCS_GET_DEVICES;
  extern w26n_byte RPCS_SET_DEV_STATE;
  extern w26n_byte RPCS_SET_DEV_LEVEL;
  extern w26n_byte RPCS_SET_DEV_COLOR;
  extern w26n_byte RPCS_GET_DEV_STATE;
  extern w26n_byte RPCS_GET_DEV_LEVEL;
  extern w26n_byte RPCS_GET_DEV_HUE;
  extern w26n_byte RPCS_GET_DEV_SAT;
  extern w26n_byte RPCS_BIND_DEVICES;
  extern w26n_byte RPCS_GET_THERM_READING;
  extern w26n_byte RPCS_GET_POWER_READING;
  extern w26n_byte RPCS_DISCOVER_DEVICES;
  extern w26n_byte RPCS_SEND_ZCL;
  extern w26n_byte RPCS_GET_GROUPS;
  extern w26n_byte RPCS_ADD_GROUP;
  extern w26n_byte RPCS_GET_SCENES;
  extern w26n_byte RPCS_STORE_SCENE;
  extern w26n_byte RPCS_RECALL_SCENE;
  extern w26n_byte RPCS_IDENTIFY_DEVICE;
  extern w26n_byte RPCS_CHANGE_DEVICE_NAME;
  extern w26n_byte RPCS_DELETE_DEVICE;
  extern w26n_byte RPCS_GET_GATEDETAIL;
  extern w26n_byte RPCS_ALLOW_ADDDEVICES;
  extern w26n_byte RPCS_FACTORY_GATEWAY;
#endif
