#ifndef GATEWAYBEAN_H_
#define GATEWAYBEAN_H_


struct GatewayBean {

	char IP[16];
	char SN[16];
	char version[32];
	int snid;
	char username[64];
	char password[64];

};
extern struct GatewayBean g_Gate;
extern int g_monitor_socket;

#endif /* SEARCHGATEWAY_H_ */
