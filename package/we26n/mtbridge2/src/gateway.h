
#ifndef  __T_GATEWAY_H__
#define  __T_GATEWAY_H__

/**/
int  tdata_init( uint16_t uuid, int type, intptr_t sctx, intptr_t fccbk, intptr_t * pret );
int  tdata_subscribe( intptr_t ctx, field_change_cbf pfunc, intptr_t argkey, int type );
int  tdata_unsubscribe( intptr_t ctx, intptr_t argkey );

int  tdata_action( intptr_t ctx, double value );
int  tdata_report( intptr_t ctx, double value );
int  tdata_get_double( intptr_t ctx, double * pret );
int  tdata_get_uuid( intptr_t ctx, uint16_t * pret );
int  tdata_get_father( intptr_t ctx, intptr_t * pret );



/**/
int  srvs_search_tdata( intptr_t ctx, uint16_t uuid, intptr_t * pret );
int  srvs_get_father( intptr_t ctx, intptr_t * pret );
int  srvs_subscribe( intptr_t sctx, field_change_cbf pfunc, intptr_t argkey, int type );
int  srvs_unsubscribe( intptr_t sctx, intptr_t argkey );
int  srvs_init( const char * sid, intptr_t ahd, intptr_t dctx, intptr_t fccbk, intptr_t * pret );



/**/
int  devs_add_service( intptr_t devctx, const char * sid, intptr_t ahd, intptr_t * pret );
int  devs_search_service( intptr_t devctx, const char * sid, intptr_t * pret );
int  devs_get_name( intptr_t devctx, const char ** pret );
int  devs_subscribe( intptr_t dctx, field_change_cbf pfunc, intptr_t argkey, int type );
int  devs_unsubscribe( intptr_t dctx, intptr_t argkey );
int  devs_init( const char * did, intptr_t fccbk, intptr_t * pret );


/**/
int  gtw_search_device( const char * did, intptr_t * pret );
int  gtw_search_tdata( const char * did, uint16_t uuid, intptr_t * pret );

int  gtw_add_device( const char * did, intptr_t ahd, intptr_t * pret );
int  gtw_subscribe( field_change_cbf pfunc, intptr_t argkey, int type );
int  gtw_unsubscribe( intptr_t argkey );
int  gtw_init( uv_loop_t * ploop );



#endif


