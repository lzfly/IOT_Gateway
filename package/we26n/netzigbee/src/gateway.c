#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "we26n_type.h"
#include "utils.h"
#include "gateway.h"


struct GatewayBean g_Gate;
struct deviceBean g_devices[MAX_DEVICES];
int g_devices_count = 0;
int g_monitor_socket;

//
//w26n_bool void we26n_read_gateIP(char *ip)
//{
//	strcpy(g_pGate->IP,ip);
//}


