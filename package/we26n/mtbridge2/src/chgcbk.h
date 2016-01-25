
#ifndef  __CHG_CBK_H__
#define  __CHG_CBK_H__


#define  CCBK_T_REPORT  1
#define  CCBK_T_ACTION  2
#define  CCBK_T_BOTH    (CCBK_T_REPORT|CCBK_T_ACTION)


typedef void (* field_change_cbf)( intptr_t ctx, intptr_t tdat );

int  ccbk_init( intptr_t fccbk, intptr_t * pret );
int  ccbk_fini( intptr_t ctx );
int  ccbk_addtail( intptr_t ctx, field_change_cbf pfunc, intptr_t argkey, int type );
int  ccbk_remove( intptr_t ctx, intptr_t argkey, int type );
int  ccbk_invoke( intptr_t ctx, int type, intptr_t tdat );


#endif

