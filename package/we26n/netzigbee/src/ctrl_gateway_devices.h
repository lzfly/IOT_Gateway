
#ifndef CTRLGATEWAYDEVICES_H_
#define CTRLGATEWAYDEVICES_H_

extern int sendDeviceState(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint, w26n_uint8 state);
extern int sendDeviceLevel(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint, w26n_uint8 level);
extern int sendDeviceColorTemp(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint, w26n_uint16 colorTemp);
#endif /* CTRLGATEWAYDEVICES_H_ */
