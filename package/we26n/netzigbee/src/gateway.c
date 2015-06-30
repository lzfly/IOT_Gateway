#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "we26n_type.h"
#include "utils.h"
#include "gateway.h"
#include <sys/socket.h>
#include <netinet/in.h>

struct GatewayBean g_Gate;
struct deviceBean g_devices[MAX_DEVICES];
int g_devices_count = 0;
struct sockaddr_in g_localAddr;

//
//w26n_bool void we26n_read_gateIP(char *ip)
//{
//	strcpy(g_pGate->IP,ip);
//}


