
#ifndef __TBL_IGSP_INTF_H__
#define __TBL_IGSP_INTF_H__

#include "gen/ds_mrouter_port.h"
#include "gen/ds_query.h"
#include "gen/ds_join_group.h"

int32
tbl_igsp_intf_add_igsp_intf_sync(tbl_igsp_intf_t *p_if, uint32 sync);

int32
tbl_igsp_intf_del_igsp_intf_sync(tbl_igsp_intf_key_t *p_if_key, uint32 sync);

int32
tbl_igsp_intf_set_igsp_intf_field_sync(tbl_igsp_intf_t *p_if, tbl_igsp_intf_field_id_t field_id, uint32 sync);

int32
tbl_igsp_intf_add_igsp_intf(tbl_igsp_intf_t *p_if);

int32
tbl_igsp_intf_del_igsp_intf(tbl_igsp_intf_key_t *p_if_key);

int32
tbl_igsp_intf_set_igsp_intf_field(tbl_igsp_intf_t *p_if, tbl_igsp_intf_field_id_t field_id);

tbl_igsp_intf_t*
tbl_igsp_intf_get_igsp_intf(tbl_igsp_intf_key_t *p_if_key);

char*
tbl_igsp_intf_key_val2str(tbl_igsp_intf_t *p_if, char *str, uint32 str_len);

int32
tbl_igsp_intf_key_str2val(char *str, tbl_igsp_intf_t *p_if);

int32
tbl_igsp_intf_dump_one(tbl_igsp_intf_t *p_if, tbl_iter_args_t *pargs);

char*
tbl_igsp_intf_key_name_dump(tbl_igsp_intf_t *p_if, char* str);

char*
tbl_igsp_intf_key_value_dump(tbl_igsp_intf_t *p_if, char* str);

char*
tbl_igsp_intf_field_name_dump(tbl_igsp_intf_t *p_if, int32 field_id, char* str);

char*
tbl_igsp_intf_field_value_dump(tbl_igsp_intf_t *p_if, int32 field_id, char* str);

char**
tbl_igsp_intf_table_dump(tbl_igsp_intf_t *p_if, char** str);

int32
tbl_igsp_intf_table_parser(char** array, char* key_value, tbl_igsp_intf_t *p_if);

int32
tbl_igsp_intf_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_igsp_intf_master_t*
tbl_igsp_intf_get_master();

tbl_igsp_intf_master_t*
tbl_igsp_intf_init_igsp_intf();

#endif /* !__TBL_IGSP_INTF_H__ */

