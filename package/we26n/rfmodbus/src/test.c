

#include <stdio.h>

#define __USE_XOPEN

#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include <sys/socket.h>

#include <event2/event.h>

#include "newuart.h"
#include "bletask.h"
#include "pktintf.h"
#include "dgramsck.h"
#include "ubussrv.h"







void  dump_hex( const unsigned char * ptr, size_t  len )
{
    int  i;
    int  nn;
    int  len2 = len;

    if (len <= 0)
    {
        puts("");
        fflush(stdout);
        return;
    }
    
    nn = 0;
    while ( (len2 - nn) >= 16 )
    {
        for ( i=0; i<16; i++ )
        {
            printf( "%02x ", ptr[nn + i] );
        }
        
        printf("  |  ");

        for ( i=0; i<16; i++ )
        {
            int  c = ptr[nn + i];

            if ( (c < 32) || (c > 127) )
                c = '.';
                
            printf("%c", c);
        }

        nn += 16;
        printf("\n");
        
    }

    if ( len2 > nn )
    {
        for ( i = 0; i < (len2-nn); i++ )
            printf("%02x ", ptr[nn + i]);
        printf("  >  ");

        for ( i = 0; i < (len2-nn); i++ )
        {
            int  c = ptr[nn + i];
            if (c < 32 || c > 127)
                c = '.';
            printf("%c", c);
        }

        printf("\n");        
    }

    fflush(stdout);
    
}


typedef struct _tag_upgrade_info
{
    int  tlen;
    uint8_t * pblk;

    /**/
    int  tcrc;
    int  size;
    int  blks;
    int  offs;
    
} upgrade_info_t;


void  make_trans_data( upgrade_info_t * pinfo, uint8_t * pdat, int * plen )
{
    uint16_t  total;
    uint16_t  sernm;
    uint16_t  temp;
    int  toffs;
    
    /**/
    total = (uint16_t)(pinfo->blks);
    sernm = (uint16_t)(pinfo->offs + 1);
    
    /**/
    pdat[0] = 0x01;
    pdat[2] = (uint8_t)total;
    pdat[3] = (uint8_t)(total >> 8);
    pdat[4] = (uint8_t)sernm;
    pdat[5] = (uint8_t)(sernm >> 8);

    /**/
    if ( sernm == 1 )
    {
        pdat[1] = 0x00;
        pdat[6] = 0x14;
        pdat[7] = 0x00;

        memcpy( &(pdat[8]), pinfo->pblk, 20 );
        *plen = 28;
    }
    else if ( sernm == total )
    {
        temp = (pinfo->tlen - 20) % pinfo->size;
        pdat[1] = 0x01;
        pdat[6] = (uint8_t)temp;
        pdat[7] = (uint8_t)(temp >> 8);

        memcpy( &(pdat[8]), pinfo->pblk + (pinfo->tlen - temp), temp );
        *plen = 8 + temp;
    }
    else
    {
        temp = pinfo->size;
        
        pdat[1] = 0x00;
        pdat[6] = (uint8_t)temp;
        pdat[7] = (uint8_t)(temp >> 8);

        toffs = temp;
        toffs = toffs * (pinfo->offs - 1);
        toffs = toffs + 20;
        memcpy( &(pdat[8]), pinfo->pblk + toffs, temp );
        *plen = 8 + temp;
    }

    return;
    
}


int  test_task_reboot_final( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );
    return 1;
}


int  test_temp_final( intptr_t tctx )
{
    uint8_t  tary[2];
    intptr_t nctx;

    /**/
    task_inherit( tctx, 0, &nctx );
    tary[0] = 0;
    task_active( nctx, CMD_REBOOT, 1, tary, test_task_reboot_final, 2000 );
    return 0;
}


int  test_task_upgrade_file( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    printf( "cmd = %X : %d\n", cmd, tlen );
    dump_hex( pdat, tlen );
    return 1;
}


int  test_task_transfer_file( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    upgrade_info_t * pinfo;
    uint8_t  tary[64];
    intptr_t nctx;
    int  temp;
    
    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    /**/
    task_getptr( tctx, (void **)&pinfo );

    /**/
    if ( cmd == ACK_TRANS )
    {
        if ( tlen < 3 )
        {
            return test_temp_final( tctx );
        }

        /**/
        temp = pdat[2];
        temp = (temp << 8) | pdat[1];
        
        if ( (pdat[0] == 0) && (temp == (pinfo->offs+1)) )
        {
            pinfo->offs += 1;
            make_trans_data( pinfo, tary, &temp );            
            task_reactive( tctx, CMD_TRANS, temp, tary, test_task_transfer_file, 1000 );
            return 1;
        }
    }

    if ( cmd == ACK_TRANS_CRC )
    {
        if ( tlen < 1 )
        {
            return test_temp_final( tctx );
        }

        temp = pdat[1];
        temp = (temp << 8) | pdat[0];

        if ( temp == pinfo->tcrc )
        {
            task_inherit( tctx, 0, &nctx );
            task_active( nctx, CMD_UPGRADE, 0, NULL, test_task_upgrade_file, 2000 );
            return 0;
        }
    }

    if ( cmd == ACK_TMR_OUT )
    {
        return test_temp_final( tctx );
    }


    return 1;
    
}


int  test_task_reboot_to_loader( intptr_t tctx, uint16_t cmd, int tlen, uint8_t * pdat )
{
    int  iret;
    uint8_t  tary[64];
    upgrade_info_t * pinfo;

    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );
    
    if ( cmd == ACK_REBOOT )
    {
        if ( tlen < 1 )
        {
            return test_temp_final( tctx );
        }

        if ( pdat[0] != 0 )
        {
            return test_temp_final( tctx );
        }
    }

    if ( cmd == EVT_LDR_INFO )
    {
        if ( tlen < 5 )
        {
            return test_temp_final( tctx );
        }

        /**/
        task_getptr( tctx, (void **)&pinfo );
        make_trans_data( pinfo, tary, &iret );
        task_reactive( tctx, CMD_TRANS, iret, tary, test_task_transfer_file, 2000 );
        return 1;
    }

    if ( cmd == ACK_TMR_OUT )
    {
        /* time out */
        tary[0] = 0x1E;
        task_reactive( tctx, CMD_REBOOT, 1, tary, test_task_reboot_to_loader, 2000 );        
        return 1;
    }
    
    return 1;
    
}



intptr_t  g_dgram;
pkt_intf_t * g_pintf;

void  test_dgram_cbk( intptr_t arg, int tlen, void * pdat )
{
    g_pintf->pkti_send( g_dgram, tlen, pdat );
    return;
}


void  test_dgram_evt( evutil_socket_t ufd, short event, void * parg )
{
    intptr_t  ctx;

    /**/
    ctx = (intptr_t)parg;
    
    /**/
    g_pintf->pkti_try_run( ctx );
    return;
}



int  test_open_file( char * fname, int * ptlen, uint8_t ** ppblk )
{
    FILE * fp;
    uint32_t  tlen;
    uint8_t * pblk;
    uint32_t  temp;
    
    /**/
    fp = fopen( fname, "rb" );
    if ( NULL == fp )
    {
        return 1;
    }

    /**/
    fseek( fp, 0, SEEK_END );
    tlen = (uint32_t)ftell( fp );

    /**/
    pblk = (uint8_t *)malloc( tlen );
    if ( NULL == pblk )
    {
        fclose( fp );
        return 2;
    }

    /**/
    fseek( fp, 0, SEEK_SET );
    temp = (uint32_t)fread( pblk, 1, tlen, fp );
    if ( temp != tlen )
    {
        fclose( fp );
        return 3;
    }

    /**/
    fclose( fp );
    *ptlen = (int)tlen;
    *ppblk = pblk;
    return 0;
    
}


int  test_test_upgrade( int argc, char * argv[] )
{
    int  iret;
    int  tlen;
    uint8_t * pblk;
    intptr_t  uctx;    
    struct event_base * pevbase;
    intptr_t  tctx;
    upgrade_info_t  tinfo;
    uint32_t  temp;
    void * ptr;

    /**/
    if ( argc != 2 )
    {
        printf( "usage:\n");
        printf( "%s  <file_name> \n", argv[0] );
        return 1;
    }
    
    /**/
    iret = test_open_file( argv[1], &tlen, &pblk );
    if ( 0 != iret )
    {
        printf( "open file fail : %s.\n", argv[1] );
        return 2;
    }

    printf( "upgrade file size : %d \n", tlen );

    /**/
    pevbase = event_base_new();
    if (NULL == pevbase)
    {
        return 3;
    }
    
    /**/
    iret = nuart_init( pevbase, 11, &uctx );
    if ( 0 != iret )
    {
        printf( "uart init ret = %d\n", iret );
        return 4;
    }
    
    /**/
    if ( tlen < 20 )
    {
        return 5;
    }

    /**/
    temp = pblk[8];
    temp = (temp << 8) | pblk[7];
    temp = (temp << 8) | pblk[6];
    temp = (temp << 8) | pblk[5];

    if ( tlen != (temp + 20) )
    {
        return 6;
    }
    
    /**/
    tinfo.tlen = tlen;
    tinfo.pblk = pblk;
    tinfo.tcrc = pblk[10];
    tinfo.tcrc = (tinfo.tcrc << 8) | pblk[9];
    tinfo.size = 50;
    
    temp = (tlen - 20) / 50;
    if ( ((tlen - 20) % 50) > 0 )
    {
        temp += 1;
    }
        
    tinfo.blks = temp + 1;
    tinfo.offs = 0;
    
    /**/
    iret = task_init( pevbase, uctx, sizeof(upgrade_info_t), &tctx );
    if ( 0 != iret )
    {
        return 7;
    }
    
    /**/
    task_getptr( tctx, &ptr );
    memcpy( ptr, &tinfo, sizeof(upgrade_info_t) );
    
    /**/
    task_active( tctx, CMD_NULL, 0, NULL, test_task_reboot_to_loader, 6000 );

    // 开始主程序的循环
    event_base_dispatch( pevbase );
    
    return 0;
    
}



#include "mainthrd.h"

int  main( int argc, char * argv[] )
{
    int  iret;
    int  sfd;
    intptr_t  mnctx;

    if ( argc != 1 )
    {
        test_test_upgrade( argc, argv );
        return 1;
    }

    /**/
    iret = mthrd_init( &mnctx );
    if ( 0 != iret )
    {
        printf( "mthrd_init ret = %d\n", iret );
        return 1;
    }

    /**/
    mthrd_get_sfd( mnctx, &sfd );
    iret = ubussrv_start( sfd );
    if ( 0 != iret )
    {
        printf( "ubussrv_start iret = %d\n", iret );
        return 2;
    }
    
    /**/
    iret = mthrd_run( mnctx );
    if ( 0 != iret )
    {
        printf( "mthrd_run iret = %d\n", iret );
        return 3;
    }
    
    return 0;
    
}


