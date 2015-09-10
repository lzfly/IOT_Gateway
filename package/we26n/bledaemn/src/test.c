

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

#include <event2/event.h>

#include "newuart.h"
#include "bletask.h"





void  dump_hex( const unsigned char * ptr, size_t  len )
{
    int  i;
    int  nn;
    int  len2 = len;

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



uint16_t  crc16_reflect( uint16_t undata )
{
	int  i;
	uint16_t  unreflect = 0;
	
	/**/
	for ( i=0; i<16; i++ )
	{
		if ( undata & 0x0001 )
		{
			unreflect |= ( 0x1 << (15-i) );
		}
		
		undata = undata >> 1;
	}
	
	return unreflect;
	
}


uint16_t  crc16_ccitt( int tlen, uint8_t * pdata )
{
	uint16_t  uncrc = 0xFFFF;
	int  i;
	int  j;
	
	for ( i=0; i<tlen; i++ )
	{
		uncrc ^= crc16_reflect( (uint16_t)(*pdata++) );
		
		for ( j=0; j<16; j++ )
		{
			if ( uncrc & 0x8000 )
				uncrc = (uncrc << 1) ^ 0x1021;
			else
				uncrc = uncrc << 1;
		}
	}
	
	return uncrc;
	
}


int  meter_enc_get_addr( uint8_t * pdat )
{
    pdat[0] = 0xFE;
    pdat[1] = 0x00;
    pdat[2] = 0x22;
    pdat[3] = 0xFD;
    pdat[4] = 0x00;
    pdat[5] = 0x21;
    pdat[6] = 0x87;
    pdat[7] = 0xF5;
    return 0;
}


int  meter_enc_get_data( uint8_t addr, uint8_t * pdat )
{
    uint16_t  tcrc;
    
    pdat[0] = 0xFE;
    pdat[1] = addr;
    pdat[2] = 0x22;
    pdat[3] = 0xEC;
    pdat[4] = 0x00;

    tcrc = crc16_ccitt( 5, pdat );
    pdat[5] = (tcrc >> 8) & 0xFF;
    pdat[6] = tcrc & 0xFF;
    pdat[7] = 0xF5;
    return 0;
}


int  meter_check_crc( int tlen, uint8_t * pdat )
{
    uint16_t  tcrc;
    uint16_t  temp;
    
    if ( tlen <= 3 )
    {
        return 1;
    }

    /**/
    if ( 0xC5 != pdat[tlen-1] )
    {
        return 2;
    }
    
    /**/
    tcrc = crc16_ccitt( tlen-3, pdat );
    temp = pdat[tlen-3];
    temp = (temp << 8) | pdat[tlen-2];

    /**/
    if ( tcrc != temp )
    {
        return 3;
    }

    /**/
    return 0;
    
}


int  meter_decode_addr( int tlen, uint8_t * pdat, uint8_t * paddr )
{
    int  iret;
    
    /**/
    iret = meter_check_crc( tlen, pdat );
    if ( 0 != iret )
    {
        return 1;
    }
    
    *paddr = pdat[5];
    return 0;
}




int32_t  modbus_conv_long( uint8_t * puc )
{
    int32_t  temp;

    /**/
    temp = puc[3];
    temp = temp << 8;
    temp = temp | puc[2];
    temp = temp << 8;
    temp = temp | puc[1];
    temp = temp << 8;
    temp = temp | puc[0];

    /**/
    return temp;
}


float  modbus_conv_real4( uint8_t * puc )
{
    volatile float  temp;
    uint32_t * pu32;

    /**/
    pu32 = (uint32_t *)&temp;

    /**/
    *pu32 = puc[3];
    *pu32 <<= 8;
    *pu32 |= puc[2];
    *pu32 <<= 8;
    *pu32 |= puc[1];
    *pu32 <<= 8;
    *pu32 |= puc[0];

    /**/
    return temp;
}



int  g_offs = 0;
uint8_t  g_tary[128];


int  meter_decode_data( int tlen, uint8_t * pdat, double * pdbb )
{
    float  ftemp;
    uint32_t  itemp;
    
    memcpy( g_tary+g_offs, pdat, tlen );
    g_offs += tlen;
    
    /**/
    printf( "decode , tlen = %d \n", g_offs );
    
    /**/
    if ( g_offs < 31 )
    {
        return 1;
    }

    /**/
    printf( "decode = %d : \n", g_offs );
    dump_hex( g_tary, g_offs );

    /**/
    ftemp = modbus_conv_real4( &(g_tary[7]) );
    printf( "shunshi = %f \n", ftemp );

    itemp = modbus_conv_long( &(g_tary[11]) );
    ftemp = modbus_conv_real4( &(g_tary[15]) );
    ftemp = ftemp + itemp;
    printf( "zongliang = %f \n", ftemp );
    *pdbb = ftemp;
    
    ftemp = modbus_conv_real4( &(g_tary[19]) );
    printf( "wendu = %f \n", ftemp );
    
    return 0;
    
}


int  test_task_disconnect_finish( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );
    return 0;
}


int  test_task_write_noresp_data( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    int  iret;
    uint8_t * ptr;
    intptr_t nctx;
    double  temp;
    
    /**/
    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    ptr = (uint8_t *)pdat;
    if ( (cmd == CMD_EVT_NOTY) && (ptr[0] == 0x32) && (ptr[1] == 0x00) )
    {
        iret = meter_decode_data( tlen-2, &(ptr[2]), &temp );
        if ( 0 != iret )
        {
            return 2;
        }

        /**/
        printf( "meter data = %f \n", temp );

        /**/
        task_inherit( tctx, 0, &nctx );
        task_active( nctx, CMD_DISCONNT, 0, NULL, test_task_disconnect_finish, 2000 );
        return 0;
    }
    
    if ( (cmd == CMD_WRT_NRSP) && (ptr[0] == 0x80) )
    {
        task_inherit( tctx, 0, &nctx );
        task_active( nctx, CMD_DISCONNT, 0, NULL, test_task_disconnect_finish, 2000 );
        return 0;
    }
    
    return 1;
    
}


int  test_task_write_noresp_addr( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    int  iret;
    intptr_t nctx;
    uint8_t * ptr;
    uint8_t * uptr;
    uint8_t  addr;

    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    if ( tlen < 2)
    {
        return 1;
    }

    ptr = (uint8_t *)pdat;
    if ( (cmd == CMD_EVT_NOTY) && (ptr[0] == 0x32) && (ptr[1] == 0x00) )
    {
        
        /**/
        iret = meter_decode_addr( tlen-2, &(ptr[2]), &addr );
        if ( 0 != iret )
        {
            return 2;
        }

        /**/
        task_inherit( tctx, 10, &nctx );
        task_getptr( nctx, (void **)&uptr );
        uptr[0] = 0x32;
        uptr[1] = 0x00;
        
        meter_enc_get_data( addr, &(uptr[2]) );
        task_active( nctx, CMD_WRT_NRSP, 10, uptr, test_task_write_noresp_data, 1000 );
        return 0;
        
    }

    return 2;
    
}



int  test_task_enable_notify( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    intptr_t nctx;
    uint8_t * ptr;
    uint8_t * uptr;

    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    if ( tlen <= 0)
    {
        return 1;
    }

    ptr = (uint8_t *)pdat;
    if ( (cmd == CMD_ENA_NOTY) && (ptr[0] == 0) )
    {
        task_inherit( tctx, 10, &nctx );
        task_getptr( nctx, (void **)&uptr );
        uptr[0] = 0x32;
        uptr[1] = 0x00;
        
        meter_enc_get_addr( &(uptr[2]) );
        task_active( nctx, CMD_WRT_NRSP, 10, uptr, test_task_write_noresp_addr, 1000 );
        return 0;
    }

    return 2;
    
}


int  test_task_connect( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    intptr_t nctx;
    uint8_t * ptr;
    uint8_t * uptr;

    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    if ( tlen <= 0)
    {
        return 1;
    }

    ptr = (uint8_t *)pdat;
    if ( (cmd == CMD_CONNECT) && (ptr[0] == 0) )
    {
        task_inherit( tctx, 2, &nctx );
        task_getptr( nctx, (void **)&uptr );
        uptr[0] = 0x33;
        uptr[1] = 0x00;
        task_active( nctx, CMD_ENA_NOTY, 2, uptr, test_task_enable_notify, 1000 );
        return 0;
    }

    return 2;
    
}


int  test_task_stop_scan_to_connect( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    intptr_t nctx;
    uint8_t * ptr;
    uint8_t * uptr;

    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    if ( tlen <= 0)
    {
        return 1;
    }

    ptr = (uint8_t *)pdat;
    if ( (cmd == CMD_STOP_SCAN) && (ptr[0] == 0) )
    {
        task_inherit( tctx, 6, &nctx );
        task_getptr( tctx, (void **)&ptr );
        task_getptr( nctx, (void **)&uptr );
        memcpy( uptr, ptr, 6 );
        task_active( nctx, CMD_CONNECT, 6, uptr, test_task_connect, 30000 );
        return 0;
    }

    return 2;

}


int  test_task_start_scan( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    intptr_t nctx;
    uint8_t * ptr;
    uint8_t * uptr;

    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    /**/
    ptr = (uint8_t *)pdat;    
    if ( cmd == CMD_EVT_PEER )
    {
        if ( tlen >= 6 )
        {
            if ( (ptr[0] == 0xaf) && (ptr[1] == 0x3b) )
            {
                task_inherit( tctx, 6, &nctx );
                task_getptr( nctx, (void **)&uptr );
                memcpy( uptr, ptr, 6 );
                task_active( nctx, CMD_STOP_SCAN, 0, NULL, test_task_stop_scan_to_connect, 1000 );
                
                return 0;
            }
        }
    }
    
    return 1;
    
}



int  test_task_checkstate( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat );

int  test_task_stop_connect( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    intptr_t nctx;
    uint8_t * ptr;

    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    if ( tlen <= 0)
    {
        return 1;
    }

    ptr = (uint8_t *)pdat;
    if ( (cmd == CMD_DISCONNT) && (ptr[0] == 0) )
    {
        task_inherit( tctx, 0, &nctx );
        task_active( nctx, CMD_GET_STATE, 0, NULL, test_task_checkstate, 1000 );
        return 0;
    }

    return 2;
    
}


int  test_task_stop_scan( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    intptr_t nctx;
    uint8_t * ptr;

    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    if ( tlen <= 0)
    {
        return 1;
    }

    ptr = (uint8_t *)pdat;
    if ( (cmd == CMD_STOP_SCAN) && (ptr[0] == 0) )
    {
        task_inherit( tctx, 0, &nctx );
        task_active( nctx, CMD_GET_STATE, 0, NULL, test_task_checkstate, 1000 );
        return 0;
    }

    return 2;

}


int  test_task_checkstate( intptr_t tctx, uint8_t cmd, int tlen, uint8_t * pdat )
{
    intptr_t nctx;
    uint8_t * ptr;
    
    printf( "cmd = %X : ", cmd );
    dump_hex( pdat, tlen );

    /**/
    ptr = (uint8_t *)pdat;
    if ( tlen <= 0)
    {
        return 1;
    }

    /**/
    if ( ptr[0] == CYBLE_STATE_DISCONNECTED )
    {
        task_inherit( tctx, 0, &nctx );
        task_active( nctx, CMD_STRT_SCAN, 0, NULL, test_task_start_scan, 30000 );
    }
    else if (ptr[0] == CYBLE_STATE_CONNECTED )
    {
        task_inherit( tctx, 0, &nctx );
        task_active( nctx, CMD_DISCONNT, 0, NULL, test_task_stop_connect, 1000 );
    }
    else if (ptr[0] == CYBLE_STATE_SCANNING )
    {
        task_inherit( tctx, 0, &nctx );
        task_active( nctx, CMD_STOP_SCAN, 0, NULL, test_task_stop_scan, 1000 );
    }
    else
    {
        /* error */
    }
    
    return 0;
    
}




int  test_init( struct event_base * pevbase )
{
    int  iret;
    intptr_t  uctx;
    intptr_t  tctx;

    
    /**/
    iret = nuart_init( pevbase, 12, &uctx );
    if ( 0 != iret )
    {
        printf( "uart init ret = %d\n", iret );
        return 1;
    }

    
    /**/
    iret = task_init( pevbase, uctx, 0, &tctx );
    if ( 0 != iret )
    {
        return 2;
    }

    task_active( tctx, CMD_GET_STATE, 0, NULL, test_task_checkstate, 1000 );
    
    return 0;
    
}




int  test( void )
{
    int  iret;
    struct event_base * pevbase;

    /**/
    pevbase = event_base_new();
    if (NULL == pevbase)
    {
        return 1;
    }

    /**/
    iret = test_init( pevbase );
    if ( 0 != iret )
    {
        return 2;
    }
    
    // 开始主程序的循环
    event_base_dispatch( pevbase );
    return 0;
    
}


#if 0
int  main( void )
{
    int  iret;

    iret = test();

    printf( "test return iret = %d\n", iret );
    return 0;
}

#else

#include "mainthrd.h"
#include "ubussrv.h"


int  main( int argc, char * argv[] )
{
    int  iret;
    int  sfd;
    intptr_t  mnctx;

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

#endif


