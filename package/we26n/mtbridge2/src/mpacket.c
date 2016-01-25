
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include "uv.h"
#include "mpacket.h"


typedef struct _tag_mpkt
{
    /**/
    int  size;    
    int  offs;
    uv_buf_t  iovec[2];
    
    /**/
    uint8_t  header;
    uint8_t  remain[5];

    /**/
    uint8_t  payload[4];

} mpkt_t;


int  mpkt_inner_add_u8( mpkt_t * pctx, uint8_t value )
{
    uint8_t * ptr;

    /**/
    if ( (pctx->offs + 1) > pctx->size )
    {
        return 1;
    }

    /**/
    ptr = &(pctx->payload[pctx->offs]);
    ptr[0] = value;

    /**/
    pctx->offs += 1;
    return 0;

}


int  mpkt_inner_add_u16( mpkt_t * pctx, uint16_t value )
{
    uint8_t * ptr;

    /**/
    if ( (pctx->offs + 2) > pctx->size )
    {
        return 1;
    }

    /**/
    ptr = &(pctx->payload[pctx->offs]);
    ptr[0] = (uint8_t)(value >> 8);
    ptr[1] = (uint8_t)(value & 0xFF);

    /**/
    pctx->offs += 2;
    return 0;
        
}



int  mpkt_inner_add_str( mpkt_t * pctx, char * str )
{
    uint16_t  temp;
    uint8_t * ptr;

    /**/
    temp = strlen( str );

    /**/
    if ( (pctx->offs + 2 + temp) > pctx->size )
    {
        return 1;
    }
    
    /**/
    ptr = &(pctx->payload[pctx->offs]);
    ptr[0] = (uint8_t)(temp >> 8);
    ptr[1] = (uint8_t)(temp & 0xFF);
    memcpy( &(ptr[2]), str, temp );

    /**/
    pctx->offs += temp + 2;
    return 0;
    
}


int  mpkt_inner_add_data( mpkt_t * pctx, int tlen, uint8_t * pdat )
{
    uint8_t * ptr;

    /**/
    if ( (pctx->offs + tlen) > pctx->size )
    {
        printf( "off = %d, size = %d, tlen = %d\n", pctx->offs, pctx->size, tlen );
        return 1;
    }

    /**/
    ptr = &(pctx->payload[pctx->offs]);
    memcpy( ptr, pdat, tlen );

    /**/
    pctx->offs += tlen;    
    return 0;
}


/*
payload : 10 bytes varheader, Client Identifier, Will Topic, Will Message, User Name, Password
*/
int  mpkt_conn_alloc( int vsize, intptr_t * pret )
{
    mpkt_t * pctx;

    /**/
    pctx = (mpkt_t *)malloc( sizeof(mpkt_t) + 10 + vsize );
    if ( NULL == pctx )
    {
        return 1;
    }

    
    /**/
    pctx->size = 10 + vsize;
    pctx->offs = 0;

    /**/
    pctx->header = 0x10;
    mpkt_inner_add_str( pctx, "MQTT" );
    mpkt_inner_add_u8( pctx, 4 );       /* Protocol Level */
    mpkt_inner_add_u8( pctx, 2 );       /* Connect Flags */
    mpkt_inner_add_u16( pctx, 60 );     /* Keep Alive */
    
    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}


int  mpkt_conn_set_protocol_level( intptr_t ctx, uint8_t level )
{
    mpkt_t * pctx;

    /**/
    pctx = (mpkt_t *)ctx;

    /**/
    if ( (pctx->header & 0xF0) != 0x10 )
    {
        return 1;
    }
    
    /**/
    pctx->payload[6] = level;
    return 0;
}


int  mpkt_conn_set_clean_session( intptr_t ctx, int flag )
{
    mpkt_t * pctx;

    /**/
    pctx = (mpkt_t *)ctx;

    /**/
    if ( (pctx->header & 0xF0) != 0x10 )
    {
        return 1;
    }
    
    /**/
    if ( 0 != flag )
    {
        pctx->payload[7] |= 0x2;
    }
    else
    {
        pctx->payload[7] &= (~0x2);
    }
    return 0;
    
}


int  mpkt_conn_set_client_ident( intptr_t ctx, char * client )
{
    mpkt_t * pctx;

    /**/
    pctx = (mpkt_t *)ctx;

    /**/
    if ( (pctx->header & 0xF0) != 0x10 )
    {
        return 1;
    }
    
    /**/
    if ( pctx->offs != 10 )
    {
        return 2;
    }

    /**/
    mpkt_inner_add_str( pctx, client );
    return 0;

}


int  mpkt_conn_add_will_message( intptr_t ctx, char * topic, char * msage )
{
    return 0;
}

int  mpkt_conn_add_name_passwd( intptr_t ctx, char * name, char * passwd )
{
    return 0;
}


int  mpkt_conack_get_retcode( intptr_t ctx, uint8_t * pret )
{
    mpkt_t * pctx;

    /**/
    pctx = (mpkt_t *)ctx;

    /**/
    if ( (pctx->header & 0xF0) != MQTT_CONNACK )
    {
        return 1;
    }

    /**/
    if ( pctx->offs < 2 )
    {
        return 2;
    }

    /**/
    *pret = pctx->payload[1];
    return 0;
    
}


int  mpkt_publ_set_pkt_id( intptr_t ctx, uint16_t idt )
{
    return 0;
}


int  mpkt_publ_set_topic( intptr_t ctx, char * topic )
{
    mpkt_t * pctx;

    /**/
    pctx = (mpkt_t *)ctx;

    /**/
    if ( (pctx->header & 0xF0) != MQTT_PUBLISH )
    {
        return 1;
    }

    /**/
    mpkt_inner_add_str( pctx, topic );
    return 0;
}


int  mpkt_publ_set_message( intptr_t ctx, char * mesge )
{
    int  iret;
    mpkt_t * pctx;

    /**/
    pctx = (mpkt_t *)ctx;

    /**/
    if ( (pctx->header & 0xF0) != MQTT_PUBLISH )
    {
        return 1;
    }

    /**/
    iret = mpkt_inner_add_data( pctx, strlen(mesge), (uint8_t *)mesge );
    if ( 0 != iret )
    {
        return iret + 2;
    }

    return 0;

}


int  mpkt_publ_get_topic( intptr_t ctx, uint8_t ** ptr, int * plen )
{
    mpkt_t * pctx;
    int  temp;
    
    /**/
    pctx = (mpkt_t *)ctx;

    /**/
    if ( (pctx->header & 0xF0) != MQTT_PUBLISH )
    {
        return 1;
    }

    /**/
    temp = pctx->payload[0];
    temp = (temp << 8) | pctx->payload[1];

    /**/
    if ( temp > (pctx->offs - 2) )
    {
        return 2;
    }
    
    /**/
    *ptr = &(pctx->payload[2]);
    *plen = temp;
    return 0;
    
}


int  mpkt_publ_get_message( intptr_t ctx, uint8_t ** ptr, int * plen )
{
    mpkt_t * pctx;
    int  temp;
    
    /**/
    pctx = (mpkt_t *)ctx;

    /**/
    if ( (pctx->header & 0xF0) != MQTT_PUBLISH )
    {
        return 1;
    }

    /**/
    temp = pctx->payload[0];
    temp = (temp << 8) | pctx->payload[1];

    /**/
    if ( temp > (pctx->offs - 2) )
    {
        return 2;
    }
    
    /**/
    *ptr = &(pctx->payload[temp+2]);
    *plen = pctx->offs - 2 - temp;
    return 0;
    
}


/**/
int  mpkt_subs_set_pkt_id( intptr_t ctx, uint16_t idt )
{
    int  iret;
    mpkt_t * pctx;

    /**/
    pctx = (mpkt_t *)ctx;

    /**/
    if ( (pctx->header & 0xF0) != MQTT_SUBSCRIBE )
    {
        return 1;
    }

    /**/
    iret = mpkt_inner_add_u16( pctx, idt );
    if ( 0 != iret )
    {
        return iret + 1;
    }

    return 0;

}


int  mpkt_subs_add_topic( intptr_t ctx, char * topic )
{
    int  iret;
    mpkt_t * pctx;

    /**/
    pctx = (mpkt_t *)ctx;

    /**/
    if ( (pctx->header & 0xF0) != MQTT_SUBSCRIBE )
    {
        return 1;
    }

    /**/
    iret = mpkt_inner_add_str( pctx, topic );
    if ( 0 != iret )
    {
        return iret + 1;
    }

    iret = mpkt_inner_add_u8( pctx, 0 );
    if ( 0 != iret )
    {
        return iret + 5;
    }

    return 0;
    
}



int  mpkt_alloc( uint8_t header, int rsize, intptr_t * pret )
{
    mpkt_t * pctx;

    /**/
    pctx = (mpkt_t *)malloc( sizeof(mpkt_t) + rsize );
    if ( NULL == pctx )
    {
        return 1;
    }

    /**/
    pctx->size = rsize;
    pctx->offs = 0;
    pctx->header = header;
    
    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}



int  mpkt_input( intptr_t ctx, uint8_t data )
{
    mpkt_t * pctx;

    /**/
    pctx = (mpkt_t *)ctx;

    /**/
    if ( pctx->offs >= pctx->size )
    {
        return 1;
    }

    /**/
    pctx->payload[pctx->offs] = data;
    pctx->offs += 1;
    return 0;
}


/**/
int  mpkt_free( intptr_t ctx )
{
    mpkt_t * pctx;

    /**/
    pctx = (mpkt_t *)ctx;

    /**/
    free( pctx );
    return 0;
}

int  mpkt_iovec( intptr_t ctx, uv_buf_t ** pbufs, int * pnum )
{
    int  temp;
    int  count;
    uint8_t  byte;
    mpkt_t * pctx;

    /**/
    pctx = (mpkt_t *)ctx;

    /* ping req, only 2 bytes */
    if ( pctx->offs == 0 )
    {
        /**/
        pctx->remain[0] = 0;
        
        /**/
        pctx->iovec[0].base = (char *)&(pctx->header);
        pctx->iovec[0].len = 2;

        /**/
        *pbufs = pctx->iovec;
        *pnum = 1;
        return 0;
    }
    
    /**/
    temp = pctx->offs;
    count = 0;
    
    /* calc remain. */
    do {
        byte = temp % 128;
        temp = temp / 128;

        /* If there are more digits to encode, set the top bit of this digit */
        if(temp > 0){
            byte = byte | 0x80;
        }
        pctx->remain[count] = byte;
        count ++;
        
    } while (temp > 0 && count < 4);

    /**/
    if ( count >= 4 )
    {
        return 1;
    }

    /**/
    pctx->iovec[0].base = (char *)&(pctx->header);
    pctx->iovec[0].len = 1 + count;
    pctx->iovec[1].base = (char *)pctx->payload;
    pctx->iovec[1].len = pctx->offs;

    /**/
    *pbufs = pctx->iovec;
    *pnum = 2;
    return 0;
    
}


