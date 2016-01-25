
#ifndef  __TUBUS_H_
#define  __TUBUS_H_


typedef int (*ubus_handler_f)( intptr_t reply, const char * method, struct blob_attr * args );

struct ubus_method {
	const char *name;
	ubus_handler_f handler;

	const struct blobmsg_policy *policy;
	int n_policy;
};


struct ubus_object_type {
	const char * name;
	uint32_t id;

	const struct ubus_method *methods;
	int n_methods;
};


typedef void (*ubus_lookup_cbk_f)( intptr_t arg, uint32_t tid );
typedef void (*ubus_invoke_cbk_f)( intptr_t arg, int iret, struct blob_attr * attr );
typedef void (*ubus_addobj_cbk_f)( intptr_t arg, uint32_t oid );

/**/
int  tubus_init( uv_loop_t * ploop, intptr_t * pret );
int  tubus_lookup( intptr_t ctx, const char * path, ubus_lookup_cbk_f func, intptr_t arg );
int  tubus_invoke( intptr_t ctx, uint32_t obj, const char * method, struct blob_attr * attr, ubus_invoke_cbk_f func, intptr_t arg );
int  tubus_addobj( intptr_t ctx, struct ubus_object_type * obj );


#endif

