
#ifndef __TBL_VLAN_MAPPING_TABLE_H__
#define __TBL_VLAN_MAPPING_TABLE_H__

int32
tbl_vlan_mapping_table_add_vlan_mapping_table_sync(tbl_vlan_mapping_table_t *p_vlan_mapping_table, uint32 sync);

int32
tbl_vlan_mapping_table_del_vlan_mapping_table_sync(tbl_vlan_mapping_table_key_t *p_vlan_mapping_table_key, uint32 sync);

int32
tbl_vlan_mapping_table_set_vlan_mapping_table_field_sync(tbl_vlan_mapping_table_t *p_vlan_mapping_table, tbl_vlan_mapping_table_field_id_t field_id, uint32 sync);

int32
tbl_vlan_mapping_table_add_vlan_mapping_table(tbl_vlan_mapping_table_t *p_vlan_mapping_table);

int32
tbl_vlan_mapping_table_del_vlan_mapping_table(tbl_vlan_mapping_table_key_t *p_vlan_mapping_table_key);

int32
tbl_vlan_mapping_table_set_vlan_mapping_table_field(tbl_vlan_mapping_table_t *p_vlan_mapping_table, tbl_vlan_mapping_table_field_id_t field_id);

tbl_vlan_mapping_table_t*
tbl_vlan_mapping_table_get_vlan_mapping_table(tbl_vlan_mapping_table_key_t *p_vlan_mapping_table_key);

char*
tbl_vlan_mapping_table_key_val2str(tbl_vlan_mapping_table_t *p_vlan_mapping_table, char *str, uint32 str_len);

int32
tbl_vlan_mapping_table_key_str2val(char *str, tbl_vlan_mapping_table_t *p_vlan_mapping_table);

int32
tbl_vlan_mapping_table_dump_one(tbl_vlan_mapping_table_t *p_vlan_mapping_table, tbl_iter_args_t *pargs);

char*
tbl_vlan_mapping_table_key_name_dump(tbl_vlan_mapping_table_t *p_vlan_mapping_table, char* str);

char*
tbl_vlan_mapping_table_key_value_dump(tbl_vlan_mapping_table_t *p_vlan_mapping_table, char* str);

char*
tbl_vlan_mapping_table_field_name_dump(tbl_vlan_mapping_table_t *p_vlan_mapping_table, int32 field_id, char* str);

char*
tbl_vlan_mapping_table_field_value_dump(tbl_vlan_mapping_table_t *p_vlan_mapping_table, int32 field_id, char* str);

char**
tbl_vlan_mapping_table_table_dump(tbl_vlan_mapping_table_t *p_vlan_mapping_table, char** str);

int32
tbl_vlan_mapping_table_table_parser(char** array, char* key_value, tbl_vlan_mapping_table_t *p_vlan_mapping_table);

int32
tbl_vlan_mapping_table_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_vlan_mapping_table_master_t*
tbl_vlan_mapping_table_get_master();

tbl_vlan_mapping_table_master_t*
tbl_vlan_mapping_table_init_vlan_mapping_table();

#endif /* !__TBL_VLAN_MAPPING_TABLE_H__ */

