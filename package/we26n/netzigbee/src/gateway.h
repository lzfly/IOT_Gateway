#ifndef GATEWAYBEAN_H_
#define GATEWAYBEAN_H_
#include "we26n_type.h"
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_DEVICES 64

struct GatewayBean {

	char IP[16];
	char SN[16];
	char version[32];
	int snid;
	char username[64];
	char password[64];

};

struct deviceBean{
	w26n_uint16 shortaddr ;
	w26n_uint8 endpoint;					
	w26n_uint16 profileId ;
	w26n_uint16 deviceId;
	w26n_uint8 namelen;
	w26n_char name[101];				
	w26n_uint8 status;;
	w26n_uint8 IEEE[8];
	w26n_uint8 SNlen ;
	w26n_char SN[101];
        w26n_char ieeestr[20];
	
};
extern struct GatewayBean g_Gate;
extern struct deviceBean g_devices[MAX_DEVICES];
extern int g_devices_count;
extern struct sockaddr_in g_localAddr;
extern char g_localMAC[16];
extern w26n_uint8  g_openStatus[MAX_DEVICES];
extern w26n_uint8  g_level[MAX_DEVICES];
extern w26n_uint16  g_colorTmp[MAX_DEVICES];

#endif /* SEARCHGATEWAY_H_ */
