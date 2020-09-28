
#ifndef __TBL_ROUTE_IF_H__
#define __TBL_ROUTE_IF_H__

#include "gen/ds_connected.h"
#include "gen/ds_connected_v6.h"
#include "gen/ds_ospf_auth.h"
#include "gen/ds_nd_prefix.h"

int32
tbl_route_if_add_route_if_sync(tbl_route_if_t *p_rtif, uint32 sync);

int32
tbl_route_if_del_route_if_sync(tbl_route_if_key_t *p_rtif_key, uint32 sync);

int32
tbl_route_if_set_route_if_field_sync(tbl_route_if_t *p_rtif, tbl_route_if_field_id_t field_id, uint32 sync);

int32
tbl_route_if_add_route_if(tbl_route_if_t *p_rtif);

int32
tbl_route_if_del_route_if(tbl_route_if_key_t *p_rtif_key);

int32
tbl_route_if_set_route_if_field(tbl_route_if_t *p_rtif, tbl_route_if_field_id_t field_id);

tbl_route_if_t*
tbl_route_if_get_route_if(tbl_route_if_key_t *p_rtif_key);

char*
tbl_route_if_key_val2str(tbl_route_if_t *p_rtif, char *str, uint32 str_len);

int32
tbl_route_if_key_str2val(char *str, tbl_route_if_t *p_rtif);

int32
tbl_route_if_dump_one(tbl_route_if_t *p_rtif, tbl_iter_args_t *pargs);

char*
tbl_route_if_key_name_dump(tbl_route_if_t *p_rtif, char* str);

char*
tbl_route_if_key_value_dump(tbl_route_if_t *p_rtif, char* str);

char*
tbl_route_if_field_name_dump(tbl_route_if_t *p_rtif, int32 field_id, char* str);

char*
tbl_route_if_field_value_dump(tbl_route_if_t *p_rtif, int32 field_id, char* str);

char**
tbl_route_if_table_dump(tbl_route_if_t *p_rtif, char** str);

int32
tbl_route_if_table_parser(char** array, char* key_value, tbl_route_if_t *p_rtif);

int32
tbl_route_if_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_route_if_master_t*
tbl_route_if_get_master();

tbl_route_if_master_t*
tbl_route_if_init_route_if();

tbl_route_if_t*
tbl_route_if_get_route_if_by_name(char* ifname);
#endif /* !__TBL_ROUTE_IF_H__ */

