
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/time.h>

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "we26n_type.h"
#include "gateway.h"
#include "fbee_protocol.h"

extern int sendCommand(int fd,w26n_byte* cmd, int cmd_length,char* resp_body, int *resp_length);

int ctrlDevice()
{}

int sendDeviceCmd()
{

	int cmd_length=1;
	w26n_byte msg[cmd_length];

	char buffer[512];
	int resp_length=0;
	msg[SRPC_CMD_ID_POS]=RPCS_GET_GATEDETAIL;

	sendCommand(g_monitor_socket,msg,cmd_length,buffer,&resp_length);

	return 0;

}


