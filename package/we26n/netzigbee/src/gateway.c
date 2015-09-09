#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "we26n_type.h"
#include "utils.h"
#include "gateway.h"
#include <sys/socket.h>
#include <netinet/in.h>

struct ZigbeeReportTime g_ReportTime;
char ZigbeeId[2048]={0};
struct GatewayBean g_Gate;
struct deviceBean g_devices[MAX_DEVICES];
int g_devices_count = 0;
struct sockaddr_in g_localAddr;
char  g_localMAC[16];
w26n_uint8  g_openStatus[MAX_DEVICES];
w26n_uint8  g_level[MAX_DEVICES];
w26n_uint16  g_colorTmp[MAX_DEVICES];
int sendCmdFailCount = 0;
//
//w26n_bool void we26n_read_gateIP(char *ip)
//{
//	strcpy(g_pGate->IP,ip);
//}


