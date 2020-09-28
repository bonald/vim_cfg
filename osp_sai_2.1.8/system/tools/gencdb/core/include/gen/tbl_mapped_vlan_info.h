
#ifndef __TBL_MAPPED_VLAN_INFO_H__
#define __TBL_MAPPED_VLAN_INFO_H__

int32
tbl_mapped_vlan_info_add_mapped_vlan_info_sync(tbl_mapped_vlan_info_t *p_mapped_vlan_info, uint32 sync);

int32
tbl_mapped_vlan_info_del_mapped_vlan_info_sync(tbl_mapped_vlan_info_key_t *p_mapped_vlan_info_key, uint32 sync);

int32
tbl_mapped_vlan_info_set_mapped_vlan_info_field_sync(tbl_mapped_vlan_info_t *p_mapped_vlan_info, tbl_mapped_vlan_info_field_id_t field_id, uint32 sync);

int32
tbl_mapped_vlan_info_add_mapped_vlan_info(tbl_mapped_vlan_info_t *p_mapped_vlan_info);

int32
tbl_mapped_vlan_info_del_mapped_vlan_info(tbl_mapped_vlan_info_key_t *p_mapped_vlan_info_key);

int32
tbl_mapped_vlan_info_set_mapped_vlan_info_field(tbl_mapped_vlan_info_t *p_mapped_vlan_info, tbl_mapped_vlan_info_field_id_t field_id);

tbl_mapped_vlan_info_t*
tbl_mapped_vlan_info_get_mapped_vlan_info(tbl_mapped_vlan_info_key_t *p_mapped_vlan_info_key);

char*
tbl_mapped_vlan_info_key_val2str(tbl_mapped_vlan_info_t *p_mapped_vlan_info, char *str, uint32 str_len);

int32
tbl_mapped_vlan_info_key_str2val(char *str, tbl_mapped_vlan_info_t *p_mapped_vlan_info);

int32
tbl_mapped_vlan_info_dump_one(tbl_mapped_vlan_info_t *p_mapped_vlan_info, tbl_iter_args_t *pargs);

char*
tbl_mapped_vlan_info_key_name_dump(tbl_mapped_vlan_info_t *p_mapped_vlan_info, char* str);

char*
tbl_mapped_vlan_info_key_value_dump(tbl_mapped_vlan_info_t *p_mapped_vlan_info, char* str);

char*
tbl_mapped_vlan_info_field_name_dump(tbl_mapped_vlan_info_t *p_mapped_vlan_info, int32 field_id, char* str);

char*
tbl_mapped_vlan_info_field_value_dump(tbl_mapped_vlan_info_t *p_mapped_vlan_info, int32 field_id, char* str);

char**
tbl_mapped_vlan_info_table_dump(tbl_mapped_vlan_info_t *p_mapped_vlan_info, char** str);

int32
tbl_mapped_vlan_info_table_parser(char** array, char* key_value, tbl_mapped_vlan_info_t *p_mapped_vlan_info);

int32
tbl_mapped_vlan_info_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_mapped_vlan_info_master_t*
tbl_mapped_vlan_info_get_master();

tbl_mapped_vlan_info_master_t*
tbl_mapped_vlan_info_init_mapped_vlan_info();

#endif /* !__TBL_MAPPED_VLAN_INFO_H__ */
