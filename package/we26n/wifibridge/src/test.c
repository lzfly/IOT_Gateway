
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#include <syslog.h>

#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubox/usock.h>
#include <libubus.h>
#include <uci.h>

#include "enn_device_type.h"
#include "enn_device_attr.h"

#include "gthrd.h"


char  g_mac_addr[200];
intptr_t  g_gthrd = 0;
char  g_gas_id[200];

/**/
int  g_intver = 1;
struct uloop_timeout * g_ptmr = NULL;
int  g_state = 0;

/**/
double  g_threshold = 5.0;
int  g_warning = 0;


/**/
int  g_con_count = 0;
char  g_con_ipstr[20] = { 0 };


int  test_get_macaddr( void )
{
    FILE * fout;
    char  tbuf[100];
    char * ptr;
    char * dst;
    
    /**/
    fout = popen( "eth_mac r wifi", "r" );
    if ( fout == NULL )
    {
        return 1;
    }

    /**/
    ptr = fgets( tbuf, 90, fout );
    if ( NULL == ptr )
    {
        return 2;
    }

    /**/
    pclose( fout );

    /**/
    dst = g_mac_addr;
    ptr = tbuf;
    while( '\0' != *(ptr) )
    {
        if ( *(ptr) != ':' )
        {
            *dst++ = *ptr++;
        }
        else
        {
            ptr++;
        }
    }

    /**/
    if ( *(dst-1) == 0x0a )
    {
        *(dst-1) = '\0';
    }
    
    *dst = '\0';
    return 0;
    
}


int  get_gasmeterid_config( char * jyconfig, char * mstr )
{
    int  iret;
    struct uci_context * uci_ctx;
    struct uci_package * uci_pkg;
    struct uci_element * uci_ele;
    struct uci_section * uci_sec; 
    const char * value_s = NULL;

    uci_ctx = uci_alloc_context();
    if(!uci_ctx)
        return  1;

    iret = uci_load( uci_ctx, jyconfig, &uci_pkg );
    if ( 0 != iret )
    {
        printf("uci load jianyou config fail\n");
        uci_free_context( uci_ctx );
        return  2;
    }


    /* scan jianyou config ! */
    uci_foreach_element(&uci_pkg->sections, uci_ele)
    {
        uci_sec = uci_to_section(uci_ele);
        if( 0 == strcmp(uci_sec->type, "deviceid") )
        {
            value_s = uci_lookup_option_string( uci_ctx, uci_sec, "gasmeter");
            if ( NULL == value_s )
            {
                uci_free_context( uci_ctx );
                return 1;
            }

            /**/
            strcpy( mstr, value_s );
            uci_free_context( uci_ctx );            
            return 0;
        }
    }

    uci_free_context( uci_ctx );
    return 4;
    
}



int  get_gas_meter_id( void )
{
    int  iret;
    
    iret = get_gasmeterid_config( "jyconfig", g_gas_id );
    if ( iret != 0 )
    {
        strcpy( g_gas_id, "A01000000000000" );
    }
    
    /**/
    return 0;
}



int  get_gas_report_time_config( char * ennconfig, int * pret )
{
    int  iret;
    struct uci_context * uci_ctx;
    struct uci_package * uci_pkg;
    struct uci_element * uci_ele;
    struct uci_section * uci_sec; 
    const char * value_s = NULL;

    /* 读取配置文件数据上报时间 */
    uci_ctx = uci_alloc_context();

    iret = uci_load( uci_ctx, ennconfig, &uci_pkg );
    if ( iret != 0 )
    {
        printf("uci load ENN config fail\n");
        uci_free_context( uci_ctx );        
        return 1;
    }

    /* scan enn config ! */
    uci_foreach_element(&uci_pkg->sections, uci_ele)
    {
        uci_sec = uci_to_section(uci_ele);
        
        if( 0 == strcmp(uci_sec->type, "wifi") )
        {

            value_s = uci_lookup_option_string(uci_ctx, uci_sec, "gas_interval");

            if (value_s)
            {
                *pret = strtoul( value_s, NULL, 10 );
                uci_free_context( uci_ctx );                
                return 0;
            }
            else
            {
                uci_free_context( uci_ctx );            
                return 1;
            }
        }
    }

    uci_free_context( uci_ctx );
    return 2;
    
}



int  get_gas_report_time( int * pret )
{
    int  iret;

    /**/
    iret = get_gas_report_time_config( "ennconfig", pret );
    if ( iret == 0)
    {
        return 0;
    }

    iret =  get_gas_report_time_config( "ennconfig_ever", pret );
    if ( iret == 0 )
    {
        return 0;
    }
    
    printf("read config ennconfig_ever fail, get_report_time = %d\n ", *pret );
    return 1;
    
}


int  get_gas_battery_threshold_config( char * ennconfig, double * pret )
{
    int  iret;
    struct uci_context * uci_ctx;
    struct uci_package * uci_pkg;
    struct uci_element * uci_ele;
    struct uci_section * uci_sec; 
    const char * value_s = NULL;
    char * endptr;
    
    /* 读取配置文件数据上报时间 */
    uci_ctx = uci_alloc_context();

    iret = uci_load( uci_ctx, ennconfig, &uci_pkg );
    if ( iret != 0 )
    {
        printf("uci load ENN config fail\n");
        uci_free_context( uci_ctx );        
        return 1;
    }

    /* scan enn config ! */
    uci_foreach_element(&uci_pkg->sections, uci_ele)
    {
        uci_sec = uci_to_section(uci_ele);
        
        if( 0 == strcmp(uci_sec->type, "wifi") )
        {

            value_s = uci_lookup_option_string(uci_ctx, uci_sec, "gas_bat_threshold");

            if (value_s)
            {
                *pret = strtod( value_s, &endptr );
                uci_free_context( uci_ctx );

                if ( endptr == value_s )
                {
                    return 3;
                }

                return 0;
            }
            else
            {
                uci_free_context( uci_ctx );            
                return 1;
            }
        }
    }

    uci_free_context( uci_ctx );
    return 2;
    
}


int  get_gas_battery_threshold( double * pret )
{
    int  iret;

    /**/
    iret = get_gas_battery_threshold_config( "ennconfig", pret );
    if ( iret == 0)
    {
        return 0;
    }

    iret =  get_gas_battery_threshold_config( "ennconfig_ever", pret );
    if ( iret == 0 )
    {
        return 0;
    }
    
    printf("read config ennconfig_ever fail, get_gas_battery_threshold = %f\n ", *pret );
    return 1;
    
}


int  test_write_to_ini( char * gas_meter_id, double data )
{
    char devicesstr[2048];
    char *path="/tmp/devices_6.ini";
    FILE *fp;
    devicesstr[0] = 0;

    sprintf(&devicesstr[0], "[");
    sprintf(&devicesstr[strlen(devicesstr)], "{");
    sprintf(&devicesstr[strlen(devicesstr)], "\"deviceid\":\"wifi_gas_%s\",",gas_meter_id);
    sprintf(&devicesstr[strlen(devicesstr)], "\"status\":\"5\",");
    sprintf(&devicesstr[strlen(devicesstr)], "\"devicetype\":\"0020\",");
    sprintf(&devicesstr[strlen(devicesstr)], "\"data\":\"%f\"",data);
    sprintf(&devicesstr[strlen(devicesstr)], "},");
    sprintf(&devicesstr[strlen(devicesstr)-1], "]");

    printf("devicesstr= %s\n",devicesstr);
    
    if((fp=fopen(path,"w+")) == NULL)
    {
        printf("fail to open\n");
        return -1;
    }

    fwrite(devicesstr,1,strlen(devicesstr),fp);
    fclose(fp);
    
    printf("[gas_meter]write to ini file ok\n");
    syslog(LOG_CRIT,"[gas_meter]write to ini file ok");
    return 0;
    
}


void  test_data_cback(struct ubus_request *req, int type, struct blob_attr *msg)
{
    /**/
    if ( type == UBUS_MSG_DATA )
    {
        printf( "data cback, %d\n", type );
    }

    return;
}


int  sendMsgToWeb( char * meterid, unsigned short int attr, double data )
{
    uint32_t  id;
    struct ubus_context *ctx;
    static struct blob_buf b;

    printf("[sendMsgToWeb] start\r\n");

    /**/
    ctx = ubus_connect( NULL );
    if ( NULL == ctx)
    {
        fprintf(stderr, "Failed to connect to ubus\n");
        return 1;
    }

    /**/
    if ( ubus_lookup_id(ctx, "jianyou", &id) )
    {
        fprintf(stderr, "Failed to look up jianyou object\n");
        return 2;
    }

    blob_buf_init( &b, 0 );
    char gatewayidstr[32];
    sprintf(gatewayidstr, "we26n_xxxxx" );
    blobmsg_add_string( &b, "gatewayid", gatewayidstr );

    char deviceidstr[64];
    sprintf(deviceidstr, "wifi_gas_%s", meterid);
    printf("[sendMsgToWeb] start--%s\r\n", deviceidstr);
    blobmsg_add_string( &b, "deviceid", deviceidstr);

    char devicetypestr[8];
    char deviceattrstr[16];
    char devicedatastr[64];

    sprintf(devicetypestr, "%s", ENN_DEVICE_TYPE_GAS_METER);
    sprintf(deviceattrstr, "%d", ENN_DEVICE_ATTR_GASMETER_VALUE);
    sprintf(devicedatastr, "%lf", data);

    blobmsg_add_string( &b, "devicetype", devicetypestr);
    blobmsg_add_string( &b, "attr", deviceattrstr );
    blobmsg_add_string( &b, "data", devicedatastr);

    /**/
    printf("[sendMsgToWeb]ubus_invoke data = %s\r\n", devicedatastr);
    ubus_invoke( ctx, id, "report", b.head, test_data_cback, 0, 3000);
    ubus_free(ctx);
    return 0;
    
}




static int wifib_notify( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{

    static struct blob_buf b;
    uint64_t  mid;
    
    /**/
    get_gas_meter_id();
    mid = strtoull( &g_gas_id[3], NULL, 10 );
    gthrd_notify_mid( g_gthrd, mid );
    system( "rm /tmp/devices_6.ini" );
    g_state = 0;
    uloop_timeout_set( g_ptmr, 500 );

    /**/
    printf( "wifib_notify, %llu\n", mid );
    
    /* send reply */
    blob_buf_init( &b, 0 );
    blobmsg_add_string( &b, "return",  "ok" );
    
    /**/
    ubus_send_reply( ctx, req, b.head );
    return UBUS_STATUS_OK;
    
}


static int wifib_getstat( struct ubus_context *ctx, struct ubus_object *obj,
                struct ubus_request_data *req, const char *method,
                struct blob_attr *msg )
{

    static struct blob_buf b;
    char  tstr[1024];
    
    /**/
    printf( "wifib_getstat" );
    
    /* send reply */
    blob_buf_init( &b, 0 );
    blobmsg_add_string( &b, "return",  "ok" );

    /**/
    sprintf( tstr, "mid=%s, interval=%d, state=%d, accept=%d, ipadr=%s", g_gas_id, g_intver, g_state, g_con_count, g_con_ipstr );
    blobmsg_add_string( &b, "global",  tstr );

    /**/
    gthrd_getstat( g_gthrd, tstr );
    blobmsg_add_string( &b, "gthrd",  tstr );
    
    /**/
    ubus_send_reply( ctx, req, b.head );
    return UBUS_STATUS_OK;
    
}


static const struct ubus_method wifib_methods[] = {
    UBUS_METHOD_NOARG( "notify",  wifib_notify ),
    UBUS_METHOD_NOARG( "getstat",  wifib_getstat ),
};


static struct ubus_object_type system_object_type =
    UBUS_OBJECT_TYPE("wifib_iface", wifib_methods);


static struct ubus_object wifib_object = {
    .name = "we26n_wifibridge",
    .type = &system_object_type,
    .methods = wifib_methods,
    .n_methods = ARRAY_SIZE(wifib_methods),
};




int  test_server_add( void )
{
    int ret;
    struct ubus_context * ctx;

    /**/
    ctx = ubus_connect( NULL );
    if ( NULL == ctx) 
    {
        return 1;
    }

    /**/
    ubus_add_uloop( ctx );

    /**/
    ret = ubus_add_object( ctx, &wifib_object );
    if (ret)
    {
        fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
        return 2;
    }
    
    return 0;
    
}



void  test_accept_cbk( struct uloop_fd * pufd, unsigned int events )
{
    int  sock;
    struct sockaddr_in  client_addr;
    socklen_t  addrlen;
    
    /**/
    addrlen = sizeof(client_addr);
    sock = accept( pufd->fd, (struct sockaddr*)&client_addr, &addrlen );
    if ( sock < 0 )
    {
        return;
    }
    
    /**/
    printf( "accept, %d \n", sock );
    printf( "client IP and Port %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port) );
    syslog( LOG_CRIT, "client IP and Port %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port) );

    /**/
#if 0
    test_write_to_ini( g_gas_id, 0.0 );
#endif
    
    sprintf( g_con_ipstr, "%s", inet_ntoa(client_addr.sin_addr) );
    g_con_count += 1;
    
    gthrd_notify_sock( g_gthrd, sock );
    
    /**/
    return;
    
}



int  test_listen_add( void )
{
    int  iret;
    int  sock;
    int  type;
    struct uloop_fd * pufd;

    /**/
    type = USOCK_TCP | USOCK_SERVER | USOCK_NONBLOCK | USOCK_IPV4ONLY;

    sock = usock( type, "0.0.0.0", "9000" );
    if ( sock <= 0 )
    {
        return 1;
    }

    /**/
    pufd = (struct uloop_fd *)malloc( sizeof(struct uloop_fd) );
    if ( NULL == pufd )
    {
        return 2;
    }

    /**/
    memset( pufd, 0, sizeof(struct uloop_fd) );
    pufd->fd = sock;
    pufd->cb = test_accept_cbk;
    
    /**/
    iret = uloop_fd_add( pufd, ULOOP_READ | ULOOP_BLOCKING );
    if ( iret < 0 )
    {
        return 3;
    }

    return 0;
    
}

extern void  dump_hex( const unsigned char * ptr, size_t len );

void  test_recv_cbk( struct uloop_fd * pufd, unsigned int events )
{
    int  iret;
    uint8_t  tary[2048];
    double  data;
    double  batty;
    
    /**/
    while (1)
    {
        iret = recv( pufd->fd, tary, 2048, 0 );

        /**/
        if ( iret < 0 )
        {
            if ( (errno == EAGAIN) || (errno == EWOULDBLOCK) )
            {
                break;
            }
            else if ( errno == EINTR )
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if ( iret == 0 )
        {
            break;
        }

        /**/
        printf( "uloop recv %d\n", iret );        
        dump_hex( &(tary[0]), iret );

        /* report to web */
        g_state = 1;
        memcpy( &data, tary, sizeof(data) );
        memcpy( &batty, tary + sizeof(data), sizeof(batty) );

        /**/
        test_write_to_ini( g_gas_id, data );
        sendMsgToWeb( g_gas_id, ENN_DEVICE_ATTR_GASMETER_VALUE, data );

        /**/
        syslog( LOG_CRIT, "battery, %f", batty );
        if ( batty < g_threshold )
        {
            sendMsgToWeb( g_gas_id, ENN_DEVICE_ATTR_GAS_ALERT, 1 );        
        }
        else
        {
            sendMsgToWeb( g_gas_id, ENN_DEVICE_ATTR_GAS_ALERT, 0 );        
        }
        
    }
    
    return;
    
}


int  test_gthrd_start( void )
{
    int  iret;
    int  tfd;
    intptr_t  tctx;
    struct uloop_fd * pufd;
    uint64_t  mid;
    
    /**/
    iret = gthrd_init( &tctx );
    if ( 0 != iret )
    {
        return 1;
    }

    gthrd_getfd( tctx, &tfd );

    /**/
    pufd = (struct uloop_fd *)malloc( sizeof(struct uloop_fd) );
    if ( NULL == pufd )
    {
        return 2;
    }

    /**/
    memset( pufd, 0, sizeof(struct uloop_fd) );
    pufd->fd = tfd;
    pufd->cb = test_recv_cbk;
    
    /**/
    iret = uloop_fd_add( pufd, ULOOP_READ | ULOOP_BLOCKING );
    if ( iret < 0 )
    {
        return 3;
    }
    
    /**/
    iret = gthrd_start( tctx );
    if ( 0 != iret )
    {
        return 2;
    }

    /**/
    mid = strtoull( &g_gas_id[3], NULL, 10 );
    gthrd_notify_mid( tctx, mid );
    printf( "start, mid, %llu\n", mid );
    
    /**/
    g_gthrd = tctx;
    return 0;
    
}


void  test_timer_cbk( struct uloop_timeout * ptmr )
{
    /**/
    gthrd_notify_ever( g_gthrd );

    if ( 0 == g_state )
    {
        uloop_timeout_set( ptmr, 15000 );
    }
    else
    {
        uloop_timeout_set( ptmr, g_intver * 60000 );    
    }
    
    return;
}



int  test_timer_start( void )
{
    struct uloop_timeout * ptmr;
    
    /**/
    ptmr = (struct uloop_timeout *)malloc( sizeof(struct uloop_timeout) );
    if ( NULL == ptmr )
    {
        return 1;
    }

    /* flow meter */
    memset( ptmr, 0, sizeof(struct uloop_timeout) );
    ptmr->cb = test_timer_cbk;
    uloop_timeout_set( ptmr, 15000 );

    /**/
    g_ptmr = ptmr;
    return 0;
}




int  main( void )
{
    int  iret;
    int  temp;
    double  thres;
    
    /**/
    openlog( "wifibridge", 0, 0 );    
    syslog( LOG_CRIT, "begin wifibridge ..." );

    /**/
    get_gas_meter_id();
    iret = get_gas_report_time( &temp );
    if ( 0 != iret )
    {
        temp = 1;
    }
    g_intver = temp;
    syslog( LOG_CRIT, "interval, %d", g_intver );
    

    /**/
    iret = get_gas_battery_threshold( &thres );
    if ( 0 != iret )
    {
        thres = 5.0;
    }
    
    g_threshold = thres;
    syslog( LOG_CRIT, "battery thres, %f", g_threshold );
    
    
    /**/
    iret = test_get_macaddr();
    if ( 0 != iret )
    {
        syslog( LOG_CRIT, "get mac, ret = %d", iret );
        return 1;
    }

    /**/
    uloop_init();
    signal(SIGPIPE, SIG_IGN);   

    /**/
    iret = test_gthrd_start();
    if ( 0 != iret )
    {
        syslog( LOG_CRIT, "gthrd start, ret = %d", iret );
        return 2;
    }
    

    iret = test_server_add();
    if ( 0 != iret )
    {
        printf( "server add fail,ret = %d\n", iret );
        uloop_done();
        return 2;
    }

    /**/
    iret = test_listen_add();
    if ( 0 != iret )
    {
        printf( "prepare modbus fail, ret = %d\n", iret );
        uloop_done();
        return 3;
    }

    /**/
    iret = test_timer_start();
    if ( 0 != iret )
    {
        printf( "timer start fail, ret = %d\n", iret );
        uloop_done();
        return 4;
    }
    
    /**/
    uloop_run();
    uloop_done();
    
    return 0;
    
}



