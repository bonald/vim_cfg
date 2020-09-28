
#ifndef __TBL_VLAN_H__
#define __TBL_VLAN_H__

int32
tbl_vlan_add_vlan_sync(tbl_vlan_t *p_vlan, uint32 sync);

int32
tbl_vlan_del_vlan_sync(tbl_vlan_key_t *p_vlan_key, uint32 sync);

int32
tbl_vlan_set_vlan_field_sync(tbl_vlan_t *p_vlan, tbl_vlan_field_id_t field_id, uint32 sync);

int32
tbl_vlan_add_vlan(tbl_vlan_t *p_vlan);

int32
tbl_vlan_del_vlan(tbl_vlan_key_t *p_vlan_key);

int32
tbl_vlan_set_vlan_field(tbl_vlan_t *p_vlan, tbl_vlan_field_id_t field_id);

tbl_vlan_t*
tbl_vlan_get_vlan(tbl_vlan_key_t *p_vlan_key);

char*
tbl_vlan_key_val2str(tbl_vlan_t *p_vlan, char *str, uint32 str_len);

int32
tbl_vlan_key_str2val(char *str, tbl_vlan_t *p_vlan);

int32
tbl_vlan_dump_one(tbl_vlan_t *p_vlan, tbl_iter_args_t *pargs);

char*
tbl_vlan_key_name_dump(tbl_vlan_t *p_vlan, char* str);

char*
tbl_vlan_key_value_dump(tbl_vlan_t *p_vlan, char* str);

char*
tbl_vlan_field_name_dump(tbl_vlan_t *p_vlan, int32 field_id, char* str);

char*
tbl_vlan_field_value_dump(tbl_vlan_t *p_vlan, int32 field_id, char* str);

char**
tbl_vlan_table_dump(tbl_vlan_t *p_vlan, char** str);

int32
tbl_vlan_table_parser(char** array, char* key_value, tbl_vlan_t *p_vlan);

int32
tbl_vlan_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_vlan_master_t*
tbl_vlan_get_master();

tbl_vlan_master_t*
tbl_vlan_init_vlan();

#endif /* !__TBL_VLAN_H__ */

