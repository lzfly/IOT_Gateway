#ifndef GATEWAYSOCKET_H
#define GATEWAYSOCKET_H
extern int g_monitor_socket;
extern int connectGateway();
extern int disConnectGateway();
extern int sendCommand(int fd,w26n_byte* cmd, int cmd_length);
extern int receiveMsg(int fd,char* resp_body, int *resp_length);
#endif



