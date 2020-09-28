
#ifndef __TBL_PVLAN_H__
#define __TBL_PVLAN_H__

#include "gen/ds_pvlan_community.h"

int32
tbl_pvlan_add_pvlan_sync(tbl_pvlan_t *p_pvlan, uint32 sync);

int32
tbl_pvlan_del_pvlan_sync(tbl_pvlan_key_t *p_pvlan_key, uint32 sync);

int32
tbl_pvlan_set_pvlan_field_sync(tbl_pvlan_t *p_pvlan, tbl_pvlan_field_id_t field_id, uint32 sync);

int32
tbl_pvlan_add_pvlan(tbl_pvlan_t *p_pvlan);

int32
tbl_pvlan_del_pvlan(tbl_pvlan_key_t *p_pvlan_key);

int32
tbl_pvlan_set_pvlan_field(tbl_pvlan_t *p_pvlan, tbl_pvlan_field_id_t field_id);

tbl_pvlan_t*
tbl_pvlan_get_pvlan(tbl_pvlan_key_t *p_pvlan_key);

char*
tbl_pvlan_key_val2str(tbl_pvlan_t *p_pvlan, char *str, uint32 str_len);

int32
tbl_pvlan_key_str2val(char *str, tbl_pvlan_t *p_pvlan);

int32
tbl_pvlan_dump_one(tbl_pvlan_t *p_pvlan, tbl_iter_args_t *pargs);

char*
tbl_pvlan_key_name_dump(tbl_pvlan_t *p_pvlan, char* str);

char*
tbl_pvlan_key_value_dump(tbl_pvlan_t *p_pvlan, char* str);

char*
tbl_pvlan_field_name_dump(tbl_pvlan_t *p_pvlan, int32 field_id, char* str);

char*
tbl_pvlan_field_value_dump(tbl_pvlan_t *p_pvlan, int32 field_id, char* str);

char**
tbl_pvlan_table_dump(tbl_pvlan_t *p_pvlan, char** str);

int32
tbl_pvlan_table_parser(char** array, char* key_value, tbl_pvlan_t *p_pvlan);

int32
tbl_pvlan_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_pvlan_master_t*
tbl_pvlan_get_master();

tbl_pvlan_master_t*
tbl_pvlan_init_pvlan();

int32
tbl_pvlan_set_isloate_ports_sync(tbl_pvlan_t *p_pvlan, uint32 add, void *p_object, uint32 sync);

int32
tbl_pvlan_set_isloate_ports(tbl_pvlan_t *p_pvlan, uint32 add, void *p_object);

int32
tbl_pvlan_set_promiscuous_ports_sync(tbl_pvlan_t *p_pvlan, uint32 add, void *p_object, uint32 sync);

int32
tbl_pvlan_set_promiscuous_ports(tbl_pvlan_t *p_pvlan, uint32 add, void *p_object);

#endif /* !__TBL_PVLAN_H__ */

