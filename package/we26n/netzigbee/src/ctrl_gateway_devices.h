
#ifndef CTRLGATEWAYDEVICES_H_
#define CTRLGATEWAYDEVICES_H_

extern int sendDeviceState(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint, w26n_uint8 state);
extern int sendDeviceLevel(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint, w26n_uint8 level);
extern int sendDeviceColorTemp(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint, w26n_uint16 colorTemp);
extern int getDeviceState(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint);
extern int getDeviceLevel(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint);
extern int getDeviceColorTemp(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint);
extern int sendTmpHumAlertInterval(w26n_uint8 addrmode, w26n_uint16 shortaddr, w26n_uint8 endPoint, w26n_uint16 alertInterval);
#endif /* CTRLGATEWAYDEVICES_H_ */
