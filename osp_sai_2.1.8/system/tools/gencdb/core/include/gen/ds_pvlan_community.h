
#ifndef __DS_PVLAN_COMMUNITY_H__
#define __DS_PVLAN_COMMUNITY_H__

int32
ds_pvlan_community_add_pvlan_community_sync(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, uint32 sync);

int32
ds_pvlan_community_del_pvlan_community_sync(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, uint32 sync);

int32
ds_pvlan_community_set_pvlan_community_field_sync(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, ds_pvlan_community_field_id_t field_id, uint32 sync);

int32
ds_pvlan_community_add_pvlan_community(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community);

int32
ds_pvlan_community_del_pvlan_community(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community);

int32
ds_pvlan_community_set_pvlan_community_field(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, ds_pvlan_community_field_id_t field_id);

ds_pvlan_community_t*
ds_pvlan_community_get_pvlan_community(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community);

int32
ds_pvlan_community_key_str2val(char* tbl_key_value, char* ds_key_value, tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community);

char*
ds_pvlan_community_key_name_dump(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, char* str);

char*
ds_pvlan_community_key_value_dump(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, char* str);

char*
ds_pvlan_community_field_name_dump(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, int32 field_id, char* str);

char*
ds_pvlan_community_field_value_dump(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, int32 field_id, char* str);

char**
ds_pvlan_community_table_dump(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, char** str);

int32
ds_pvlan_community_field_value_parser(char* str, int32 field_id, tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community);

int32
ds_pvlan_community_table_parser(char** array, char* tbl_key_value, char* ds_key_value, tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community);

int32
ds_pvlan_community_dump_one(ds_pvlan_community_t *p_community, tbl_iter_args_t *pargs);

int32
ds_pvlan_community_set_ports_sync(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, uint32 add, void *p_object, uint32 sync);

int32
ds_pvlan_community_set_ports(tbl_pvlan_t *p_pvlan, ds_pvlan_community_t *p_community, uint32 add, void *p_object);

#endif /* !__DS_PVLAN_COMMUNITY_H__ */

