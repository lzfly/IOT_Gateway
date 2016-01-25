

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <uv.h>

#include <libubox/list.h>
#include <libubox/blob.h>
#include <libubox/blobmsg.h>
#include <ubusmsg.h>

#include "tubus.h"


extern  void  dump_hex( size_t len, uint8_t * ptr );


typedef struct _tag_msg_hdr
{
    struct ubus_msghdr hdr;
    struct blob_attr data;
} msg_hdr_t;



typedef  void  (* recv_proc_func)( intptr_t arg, struct ubus_msghdr * phdr, struct blob_attr * attr );

typedef struct _tag_recv_blk
{
    int  asize;
    int  size;
    int  offs;

    /**/
    recv_proc_func  func;
    intptr_t  arg;
    
    /**/
    uint8_t * data;
    
} recv_blk_t;



int  recv_init( int size, recv_proc_func func, intptr_t arg, intptr_t * pret )
{
    recv_blk_t * pctx;

    /**/
    pctx = (recv_blk_t *)malloc( sizeof(recv_blk_t) );
    if ( NULL == pctx )
    {
        return 1;
    }

    /**/
    pctx->data = (uint8_t *)malloc( size );
    if ( NULL == pctx->data )
    {
        free( pctx );
        return 2;
    }
    
    /**/
    pctx->asize = size;
    pctx->size = size;
    pctx->offs = 0;
    pctx->func = func;
    pctx->arg = arg;

    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}


int  recv_fini( intptr_t ctx )
{
    recv_blk_t * pctx;

    /**/
    pctx = (recv_blk_t *)ctx;
    free( pctx );
    return 0;
}



/* return eated data length  */
int  recv_recv( intptr_t ctx, uint8_t * ptr, int tlen )
{
    recv_blk_t * pctx;
    msg_hdr_t * phdr;
    int  temp;
    
    /**/
    pctx = (recv_blk_t *)ctx;
    
    if ( pctx->offs < sizeof(msg_hdr_t) )
    {
        if ( (pctx->offs + tlen) < sizeof(msg_hdr_t) )
        {
            memcpy( (pctx->data + pctx->offs), ptr, tlen );
            pctx->offs += tlen;
            return tlen;
        }
        
        /**/
        tlen = sizeof(msg_hdr_t) - pctx->offs;
        memcpy( (pctx->data + pctx->offs), ptr, tlen );
        pctx->offs = sizeof(msg_hdr_t);

        /**/
        phdr = (msg_hdr_t *)(pctx->data);
        temp = blob_len( &(phdr->data) );
        if ( temp <= 0 )
        {
            pctx->func( pctx->arg, &(phdr->hdr), &(phdr->data) );
            pctx->offs = 0;
            return tlen;
        }

        /* next stage size */
        pctx->size = sizeof(msg_hdr_t) + temp;
        if ( pctx->asize < pctx->size )
        {
            /* realloc */
        }
        
        return tlen;
        
    }
    else 
    {
        /**/
        if ( (pctx->offs + tlen) < pctx->size )
        {
            memcpy( (pctx->data + pctx->offs), ptr, tlen );
            pctx->offs += tlen;
            return tlen;
        }
        
        tlen = pctx->size - pctx->offs;    
        memcpy( (pctx->data + pctx->offs), ptr, tlen );
        pctx->offs = pctx->size;
        phdr = (msg_hdr_t *)(pctx->data);
        pctx->func( pctx->arg, &(phdr->hdr), &(phdr->data) );
        pctx->offs = 0;
        return tlen;
    }
    
}



#define  UBUS_UNIX_SOCKET    "/var/run/ubus.sock"


typedef struct _tag_tubus_context
{
    /**/
    uv_loop_t * ploop;

    /**/
    int  fd;
    int  status;
    uv_tcp_t  tcp;
    uv_timer_t  timer;
    uint32_t  local_id;
    
    /**/
    intptr_t  rcv_hdr;
    intptr_t  rcv_dat;
    
    /**/
    uint16_t  request_seq;
    struct list_head  request_list;
    
    /**/
    struct ubus_object_type * obj;
    
} tubus_context_t;


typedef  void  (* req_ack_func)( intptr_t arg, int status, struct blob_attr * attr );

typedef struct _tag_tubus_request
{
    /**/
    struct list_head  node;
    
    /**/
    struct ubus_msghdr hdr;    
    
    /**/
    struct blob_buf buf;

    /**/
    req_ack_func  func;
    intptr_t  arg;

    /**/
    intptr_t  arg1;
    intptr_t  arg2;
    
    /**/
    uv_write_t  write;
    uv_timer_t  timer;

    /* reply */
    int  rets;
    struct blob_attr * reta;
    
} tubus_request_t;


typedef struct _tag_tubus_reply
{
    /**/
    struct list_head  node;
    
    /**/
    struct ubus_msghdr hdr;
    struct blob_buf  stabuf;
    
    /**/
    int  noret;
    uint32_t  objid;
    const struct ubus_method * method;
    struct blob_attr * args;

    /**/
    struct ubus_msghdr dathdr;
    struct blob_buf  datbuf;
    
    /**/
    uv_write_t  datwrite;    
    uv_write_t  stawrite;

    
} tubus_reply_t;



static void  tubus_req_write_cb( uv_write_t* req, int status )
{
return;
}


/**/
int  tubus_req_alloc( uint32_t peer, tubus_request_t ** pret )
{
    tubus_request_t * preq;
    
    /**/
    preq = (tubus_request_t *)malloc( sizeof(tubus_request_t) );
    if ( NULL == preq )
    {
        return 1;
    }

    /**/
    memset( preq, 0, sizeof(tubus_request_t) );
    preq->hdr.version = 0;
    preq->hdr.peer = peer;

    /**/
    *pret = preq;
    return 0;
    
}


int  tubus_req_free( tubus_request_t * preq )
{
    blob_buf_free( &(preq->buf) );
    if ( NULL != preq->reta )
    {
        free( preq->reta );
    }

    /**/
    free( preq );
    return 0;
}


int  tubus_req_setcbk( tubus_request_t * preq, int timeout, req_ack_func func, intptr_t arg )
{
    preq->func = func;
    preq->arg = arg;
    return 0;
}


int  tubus_req_setarg( tubus_request_t * preq, intptr_t arg1, intptr_t arg2 )
{
    preq->arg1 = arg1;
    preq->arg2 = arg2;
    return 0;
}


int  tubus_req_getarg( tubus_request_t * preq, intptr_t * parg1, intptr_t * parg2 )
{
    *parg1 = preq->arg1;
    *parg2 = preq->arg2;
    return 0;
}


int  tubus_req_lookup( tubus_context_t * pubus, tubus_request_t * preq, const char * path )
{
    uv_buf_t  iov[2];

    /**/
    preq->hdr.seq = pubus->request_seq++;
    preq->hdr.type = (uint8_t)UBUS_MSG_LOOKUP;

printf( "send lookup, %u\n", preq->hdr.seq );

    /**/
    blob_buf_init( &(preq->buf), 0 );    
    blob_put_string( &(preq->buf), UBUS_ATTR_OBJPATH, path );

    /**/
    iov[0].base = (char *)&(preq->hdr);
    iov[0].len = sizeof(preq->hdr);
    iov[1].base = (char *)preq->buf.head;
    iov[1].len = blob_raw_len(preq->buf.head);
    
    /**/
    uv_write( &(preq->write), (uv_stream_t*)&(pubus->tcp), iov, 2, tubus_req_write_cb );
    preq->write.data = (void*)preq;

    /**/
    list_add( &(preq->node), &(pubus->request_list) );
    return 0;
    
}


int  tubus_req_invoke( tubus_context_t * pubus, tubus_request_t * preq, uint32_t obj, const char * method, struct blob_attr * attr )
{
    uv_buf_t  iov[2];
    
    /**/
    preq->hdr.seq = pubus->request_seq++;
    preq->hdr.type = (uint8_t)UBUS_MSG_INVOKE;

    /**/
    blob_buf_init( &(preq->buf), 0 );    
	blob_put_int32( &(preq->buf), UBUS_ATTR_OBJID, obj );
	blob_put_string( &(preq->buf), UBUS_ATTR_METHOD, method );
	if ( NULL != attr )
	{
		blob_put( &(preq->buf), UBUS_ATTR_DATA, blob_data(attr), blob_len(attr) );
    }

    /**/
    iov[0].base = (char *)&(preq->hdr);
    iov[0].len = sizeof(preq->hdr);
    iov[1].base = (char *)preq->buf.head;
    iov[1].len = blob_raw_len(preq->buf.head);
    
    /**/
    uv_write( &(preq->write), (uv_stream_t*)&(pubus->tcp), iov, 2, tubus_req_write_cb );
    preq->write.data = (void*)preq;

    /**/
    list_add( &(preq->node), &(pubus->request_list) );
    return 0;
    
}


static  void  tubus_req_push_method( struct blob_buf * pbuf, const struct ubus_method * pmtd )
{
	void * mtbl;
	int i;

	mtbl = blobmsg_open_table( pbuf, pmtd->name);
	for (i = 0; i < pmtd->n_policy; i++) {
		blobmsg_add_u32( pbuf, pmtd->policy[i].name, pmtd->policy[i].type);
	}
	blobmsg_close_table( pbuf, mtbl);

    return;
}


int  tubus_req_addobj( tubus_context_t * pubus, tubus_request_t * preq, struct ubus_object_type * obj )
{
    uv_buf_t  iov[2];
    void *s;
    int i;
    
    /**/
    preq->hdr.seq = pubus->request_seq++;
    preq->hdr.type = (uint8_t)UBUS_MSG_ADD_OBJECT;

    /**/
    blob_buf_init( &(preq->buf), 0 );    
    blob_put_string( &(preq->buf), UBUS_ATTR_OBJPATH, obj->name );

    /**/
    s = blob_nest_start( &(preq->buf), UBUS_ATTR_SIGNATURE );
    for ( i = 0; i < obj->n_methods; i++ )
        tubus_req_push_method( &(preq->buf), &(obj->methods[i]) );
    blob_nest_end( &(preq->buf), s);

    /**/
    iov[0].base = (char *)&(preq->hdr);
    iov[0].len = sizeof(preq->hdr);
    iov[1].base = (char *)preq->buf.head;
    iov[1].len = blob_raw_len(preq->buf.head);
    
    /**/
    uv_write( &(preq->write), (uv_stream_t*)&(pubus->tcp), iov, 2, tubus_req_write_cb );
    preq->write.data = (void*)preq;

    /**/
    list_add( &(preq->node), &(pubus->request_list) );
    return 0;
    
}


int  tubus_req_search( struct list_head * plist, struct ubus_msghdr * phdr, tubus_request_t ** pret )
{
    tubus_request_t * preq;

    /**/
    list_for_each_entry( preq, plist, node )
    {
        if ( preq->hdr.seq != phdr->seq )
            continue;

        if ( preq->hdr.peer != phdr->peer )
            continue;
            
        /**/
        *pret = preq;
        return 0;
    }

    /**/
    return 1;
}


static void  tubus_reply_data_write_cb( uv_write_t* req, int status )
{
return;
}


static void  tubus_reply_status_write_cb( uv_write_t* req, int status )
{
return;
}


static const struct blob_attr_info ubus_policy[UBUS_ATTR_MAX] = {
	[UBUS_ATTR_STATUS] = { .type = BLOB_ATTR_INT32 },
	[UBUS_ATTR_OBJID] = { .type = BLOB_ATTR_INT32 },
	[UBUS_ATTR_OBJPATH] = { .type = BLOB_ATTR_STRING },
	[UBUS_ATTR_METHOD] = { .type = BLOB_ATTR_STRING },
	[UBUS_ATTR_ACTIVE] = { .type = BLOB_ATTR_INT8 },
	[UBUS_ATTR_NO_REPLY] = { .type = BLOB_ATTR_INT8 },
	[UBUS_ATTR_SUBSCRIBERS] = { .type = BLOB_ATTR_NESTED },
};



int  tubus_reply_search( tubus_context_t * pctx, struct ubus_msghdr * phdr, struct blob_attr * attr, tubus_reply_t ** pret )
{
    tubus_reply_t * reply;
    struct blob_attr *attrbuf[UBUS_ATTR_MAX];
	uint32_t  objid;
	char * method;
    int  i;
    
    /**/
    if ( NULL == pctx->obj )
    {
        return 1;
    }
    
    /**/
    blob_parse( attr, attrbuf, ubus_policy, UBUS_ATTR_MAX );

    /**/
    if ( NULL == attrbuf[UBUS_ATTR_OBJID] )
    {
        return 2;
    }

    /**/
    objid = blob_get_u32(attrbuf[UBUS_ATTR_OBJID]);
    if ( objid != pctx->obj->id )
    {
        return 3;
    }

    /**/
    if ( NULL == attrbuf[UBUS_ATTR_METHOD] )
    {
        return 4;
    }

    method = (char *)blob_data( attrbuf[UBUS_ATTR_METHOD] );
    
    /**/
    for ( i=0; i<pctx->obj->n_methods; i++ )
    {
        if ( 0 == strcmp( pctx->obj->methods[i].name, method ) )
        {
            break;
        }
    }

    if ( i >= pctx->obj->n_methods )
    {
        return 5;
    }

    /**/
    reply = (tubus_reply_t *)malloc( sizeof(tubus_reply_t) );
    if ( NULL == reply )
    {
        return 6;
    }

    /**/
    memset( reply, 0, sizeof(tubus_reply_t) );
    reply->hdr = *phdr;
    reply->objid = objid;
    reply->method = &(pctx->obj->methods[i]);
    reply->noret = 0;
    reply->args = attrbuf[UBUS_ATTR_DATA];

    /**/
    if ( NULL != attrbuf[UBUS_ATTR_NO_REPLY] )
    {
        reply->noret = blob_get_int8( attrbuf[UBUS_ATTR_NO_REPLY] );
    }
    
    /**/
    *pret = reply;
    return 0;
    
}


void  tubus_reply_proc( tubus_context_t * pctx, tubus_reply_t * reply )
{
    int  iret;
    uv_buf_t  iov[2];

    /**/
    iret = reply->method->handler( (intptr_t)reply, reply->method->name, reply->args );

    /**/
    if ( 0 != reply->noret )
    {
        return;
    }

    /**/
    reply->hdr.type = UBUS_MSG_STATUS;
    blob_buf_init( &(reply->stabuf), 0 );
	blob_put_int32(&(reply->stabuf), UBUS_ATTR_STATUS, iret );
	blob_put_int32(&(reply->stabuf), UBUS_ATTR_OBJID, reply->objid );

    /**/
    iov[0].base = (char *)&(reply->hdr);
    iov[0].len = sizeof(reply->hdr);
    iov[1].base = (char *)reply->stabuf.head;
    iov[1].len = blob_raw_len(reply->stabuf.head);
    
    /**/
    uv_write( &(reply->stawrite), (uv_stream_t*)&(pctx->tcp), iov, 2, tubus_reply_status_write_cb );
    reply->stawrite.data = (void*)reply;
    return;
    
}


void  tubus_recv_msg_status( tubus_context_t * pctx, tubus_request_t * preq, struct blob_attr * attr )
{
    int  offs;
    int  total;
    struct blob_attr * node;
    int ret = UBUS_STATUS_INVALID_ARGUMENT;

    /**/
    offs = 0;
    total = blob_len( attr );
    node = (struct blob_attr *)blob_data( attr );

    /**/
    while ( offs < total )
    {
        if ( UBUS_ATTR_STATUS == blob_id( node ) )
        {
            ret = blob_get_u32( node );
            break;
        }

        /**/
        offs += blob_pad_len( node );
        node = blob_next( node );
    }

    /**/
    preq->rets = ret;
    preq->func( preq->arg, preq->rets, preq->reta );

    /* delete node from list */
    list_del( &(preq->node) );
    tubus_req_free( preq );
    return;
    
}



void  tubus_recv_message( intptr_t arg, struct ubus_msghdr * phdr, struct blob_attr * attr )
{
    int  iret;
    tubus_context_t * pctx;
    tubus_request_t * preq;
    tubus_reply_t * prpy;

    /**/
    pctx = (tubus_context_t *)arg;

#if 0
    /**/
    printf( "\nheader: " );
    dump_hex( sizeof(*phdr), (uint8_t *)phdr );

    printf( "\ndata: " );
    dump_hex( sizeof(*attr), (uint8_t *)attr );
#endif

    /**/
    switch( phdr->type )
    {
    case UBUS_MSG_HELLO:
        pctx->local_id = phdr->peer;
        pctx->status = 2;
        break;

    case UBUS_MSG_STATUS:
        iret = tubus_req_search( &(pctx->request_list), phdr, &preq );
        if ( 0 != iret )
        {
            break;
        }

        tubus_recv_msg_status( pctx, preq, attr );
        break;
        
    case UBUS_MSG_DATA:
        iret = tubus_req_search( &(pctx->request_list), phdr, &preq );
        if ( 0 != iret )
        {
            break;
        }
        
        preq->reta = blob_memdup( attr );
        break;
    
    case UBUS_MSG_INVOKE:
        iret = tubus_reply_search( pctx, phdr, attr, &prpy );
        if ( 0 != iret )
        {
            break;
        }
        tubus_reply_proc( pctx, prpy );
        break;
        
    default:
        break;
    }

    return;
    
}


void  tubus_read_cb( uv_stream_t * stream, ssize_t nread, const uv_buf_t * buf )
{
    tubus_context_t * pctx;
    uint8_t * ptr;
    int  tlen;
    int  part;
    
    /**/
    pctx = (tubus_context_t *)(stream->data);

    if ( nread < 0 )
    {
        /* fail to close */
        printf( "read cb fail %d\n", nread );
        return;
    }

    if ( nread == 0 )
    {
        return;
    }

    /**/
    ptr = (uint8_t *)(buf->base);
    tlen = nread;

    while ( tlen > 0 )
    {
        part = recv_recv( pctx->rcv_hdr, ptr, tlen );
        
        tlen -= part;
        ptr += part;
    }

    /**/
    free( buf->base );
    return;
    
}


void  tubus_read_alloc_cb( uv_handle_t* handle, size_t suggested_size, uv_buf_t * buf )
{
    char * ptr;

    /**/
    ptr = (char *)malloc( 4096 );
    if ( NULL == ptr )
    {
        buf->base = NULL;
        buf->len = 0;
        return;
    }
    
    /**/
    buf->base = ptr;
    buf->len = 4096;
    return;
}


int  tubus_connect( tubus_context_t * pctx )
{
    int  iret;
	int  fd;
	struct sockaddr_un  addr;
	
    /**/
	fd = socket( AF_UNIX, SOCK_STREAM, 0 );
	if ( fd < 0 )
	{
	    return 1;
	}

    /**/
    memset( &addr, 0, sizeof(addr) );
    addr.sun_family = AF_UNIX;
	strcpy( addr.sun_path, UBUS_UNIX_SOCKET );
	
    iret = connect( fd, (struct sockaddr*)&addr, sizeof(addr) );
    if ( 0 != iret )
    {
        printf( "conn, %s\n", strerror(errno) );
        return 2;
    }

    /**/
    iret = uv_tcp_open( &(pctx->tcp), fd );
    if ( 0 != iret )
    {
        return 3;
    }

    /**/
    uv_read_start( (uv_stream_t *)&(pctx->tcp), tubus_read_alloc_cb, tubus_read_cb );
    pctx->tcp.data = (void *)pctx;
    
    /**/
    pctx->fd = fd;
    pctx->status = 1;
    return 0;
    
}

#if 0
int test_info_handler( intptr_t reply, const char * method, struct blob_attr * args )
{
    printf( "tubus info handler\n");
    return 0;
}


struct blobmsg_policy  plcy[2] = 
{
    { "arg1", BLOBMSG_TYPE_INT32 },
    { "arg2", BLOBMSG_TYPE_STRING }
};


struct ubus_method  tmtd = 
{
    "info", test_info_handler, plcy, 2
};

struct ubus_object_type tobj = 
{
    "tubus", 0,
    &tmtd,  1
};
#endif


void  tubus_lookup_cbk( intptr_t ctx, int status, struct blob_attr * attr )
{
    tubus_request_t * preq;
    int  offs;
    int  total;
    struct blob_attr * node;
    uint32_t ret = 0;
    ubus_lookup_cbk_f func;
    intptr_t arg;

    /**/
    preq = (tubus_request_t *)ctx;
    tubus_req_getarg( preq, (intptr_t *)&func, &arg );


    /**/
    if ( status != UBUS_STATUS_OK )
    {
        if ( NULL != func )
        {
            func( arg, 0 );
        }

        return;
    }
    
    /**/
    offs = 0;
    total = blob_len( attr );
    node = (struct blob_attr *)blob_data( attr );

    /**/
    while ( offs < total )
    {
        if ( UBUS_ATTR_OBJID == blob_id( node ) )
        {
            ret = blob_get_u32( node );
            break;
        }

        /**/
        offs += blob_pad_len( node );
        node = blob_next( node );
    }

    /**/
    if ( NULL != func )
    {
        func( arg, ret );
    }
    
    return;
    
}


void  tubus_invoke_cbk( intptr_t ctx, int status, struct blob_attr * attr )
{
    tubus_request_t * preq;
    ubus_invoke_cbk_f func;
    intptr_t arg;
    int  offs;
    int  total;
    struct blob_attr * node;

    /**/
    preq = (tubus_request_t *)ctx;
    tubus_req_getarg( preq, (intptr_t *)&func, &arg );
    if ( func == NULL )
    {
        return;
    }

printf( "internal invoke cbk, %d\n", status );

    /**/
    if ( status != UBUS_STATUS_OK )
    {
        if ( NULL != func )
        {
            func( arg, status, NULL );
        }
        return;
    }
    
    /**/
    offs = 0;
    total = blob_len( attr );
    node = (struct blob_attr *)blob_data( attr );
    
    /**/
    while ( offs < total )
    {
        if ( UBUS_ATTR_DATA == blob_id( node ) )
        {
            break;
        }
    
        /**/
        offs += blob_pad_len( node );
        node = blob_next( node );
    }

    if ( offs >= total )
    {
        node = NULL;
    }



    /**/
    if ( NULL != func )
    {
        func( arg, status, node );
    }
    
    return;
    
}


void  tubus_addobj_cbk( intptr_t arg, int status, struct blob_attr * attr )
{
    tubus_context_t * pctx;
    int  offs;
    int  total;
    struct blob_attr * node;
    uint32_t ret = 0;

    /**/
    pctx = (tubus_context_t *)arg;
    
    /**/
    offs = 0;
    total = blob_len( attr );
    node = (struct blob_attr *)blob_data( attr );

    /**/
    while ( offs < total )
    {
        if ( UBUS_ATTR_OBJID == blob_id( node ) )
        {
            ret = blob_get_u32( node );
            break;
        }

        /**/
        offs += blob_pad_len( node );
        node = blob_next( node );
    }

    /**/
    pctx->obj->id = ret;
    printf( "addobj cbk, %d, %u\n", status, ret );
    return;
    
}


int  tubus_lookup( intptr_t ctx, const char * path, ubus_lookup_cbk_f func, intptr_t arg )
{
    int  iret;
    tubus_context_t * pctx;
    tubus_request_t * preq;
    
    /**/
    pctx = (tubus_context_t *)ctx;
    
    /**/
    iret = tubus_req_alloc( 0, &preq );
    if ( 0 != iret )
    {
        return 1;
    }

    /**/
    tubus_req_setarg( preq, (intptr_t)func, arg );
    iret = tubus_req_setcbk( preq, 200, tubus_lookup_cbk, (intptr_t)preq );
    if ( 0 != iret )
    {
        return 2;
    }

    /**/
    iret = tubus_req_lookup( pctx, preq, path );
    if ( 0 != iret )
    {
        return 3;
    }

    return 0;
    
}



int  tubus_invoke( intptr_t ctx, uint32_t obj, const char * method, struct blob_attr * attr, ubus_invoke_cbk_f func, intptr_t arg )
{
    int  iret;
    tubus_context_t * pctx;
    tubus_request_t * preq;
    
    /**/
    pctx = (tubus_context_t *)ctx;
    
    /**/
    iret = tubus_req_alloc( obj, &preq );
    if ( 0 != iret )
    {
        return 1;
    }

    /**/
    tubus_req_setarg( preq, (intptr_t)func, arg );
    iret = tubus_req_setcbk( preq, 200, tubus_invoke_cbk, (intptr_t)preq );
    if ( 0 != iret )
    {
        return 2;
    }

    /**/
    iret = tubus_req_invoke( pctx, preq, obj, method, attr );
    if ( 0 != iret )
    {
        return 3;
    }

    return 0;
    
}



int  tubus_addobj( intptr_t ctx, struct ubus_object_type * obj )
{
    int  iret;
    tubus_context_t * pctx;
    tubus_request_t * preq;
    
    /**/
    pctx = (tubus_context_t *)ctx;
    
    /**/
    iret = tubus_req_alloc( 0, &preq );
    if ( 0 != iret )
    {
        return 1;
    }

    /**/
    iret = tubus_req_setcbk( preq, 200, tubus_addobj_cbk, (intptr_t)pctx );
    if ( 0 != iret )
    {
        return 2;
    }

    /**/
    iret = tubus_req_addobj( pctx, preq, obj );
    if ( 0 != iret )
    {
        return 3;
    }

    /**/
    obj->id = 0;
    pctx->obj = obj;
    return 0;
    
}



int  tubus_reply( intptr_t rpy, struct blob_attr * attr )
{
    tubus_reply_t * prpy;

    /**/
    prpy = (tubus_reply_t *)rpy;
    
    /**/
    return 0;
}



int  tubus_init( uv_loop_t * ploop, intptr_t * pret )
{
    int  iret;
    tubus_context_t * pctx;

    /**/
    pctx = (tubus_context_t *)malloc( sizeof(tubus_context_t) );
    if ( NULL == pctx )
    {
        return 1;
    }

    /**/
    pctx->ploop = ploop;
    pctx->status = 0;

    /**/
    recv_init( 4096, tubus_recv_message, (intptr_t)pctx, &(pctx->rcv_hdr) );

    /**/
    pctx->request_seq = 1;
    INIT_LIST_HEAD( &(pctx->request_list) );

    /**/
    pctx->obj = NULL;
    
    /**/
    iret = uv_tcp_init( pctx->ploop, &(pctx->tcp) );
    if ( 0 != iret )
    {
        free( pctx );
        return 2;
    }


    /**/
    iret = tubus_connect( pctx );
    if ( 0 != iret )
    {
        return 3;
    }

    /**/
    *pret = (intptr_t)pctx;
    return 0;
    
}



