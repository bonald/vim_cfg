
#ifndef __TBL_CPU_TRAFFIC_GROUP_H__
#define __TBL_CPU_TRAFFIC_GROUP_H__

int32
tbl_cpu_traffic_group_add_cpu_traffic_group_sync(tbl_cpu_traffic_group_t *p_cpu_traffic_group, uint32 sync);

int32
tbl_cpu_traffic_group_del_cpu_traffic_group_sync(tbl_cpu_traffic_group_key_t *p_cpu_traffic_group_key, uint32 sync);

int32
tbl_cpu_traffic_group_set_cpu_traffic_group_field_sync(tbl_cpu_traffic_group_t *p_cpu_traffic_group, tbl_cpu_traffic_group_field_id_t field_id, uint32 sync);

int32
tbl_cpu_traffic_group_add_cpu_traffic_group(tbl_cpu_traffic_group_t *p_cpu_traffic_group);

int32
tbl_cpu_traffic_group_del_cpu_traffic_group(tbl_cpu_traffic_group_key_t *p_cpu_traffic_group_key);

int32
tbl_cpu_traffic_group_set_cpu_traffic_group_field(tbl_cpu_traffic_group_t *p_cpu_traffic_group, tbl_cpu_traffic_group_field_id_t field_id);

tbl_cpu_traffic_group_t*
tbl_cpu_traffic_group_get_cpu_traffic_group(tbl_cpu_traffic_group_key_t *p_cpu_traffic_group_key);

char*
tbl_cpu_traffic_group_key_val2str(tbl_cpu_traffic_group_t *p_cpu_traffic_group, char *str, uint32 str_len);

int32
tbl_cpu_traffic_group_key_str2val(char *str, tbl_cpu_traffic_group_t *p_cpu_traffic_group);

int32
tbl_cpu_traffic_group_dump_one(tbl_cpu_traffic_group_t *p_cpu_traffic_group, tbl_iter_args_t *pargs);

char*
tbl_cpu_traffic_group_key_name_dump(tbl_cpu_traffic_group_t *p_cpu_traffic_group, char* str);

char*
tbl_cpu_traffic_group_key_value_dump(tbl_cpu_traffic_group_t *p_cpu_traffic_group, char* str);

char*
tbl_cpu_traffic_group_field_name_dump(tbl_cpu_traffic_group_t *p_cpu_traffic_group, int32 field_id, char* str);

char*
tbl_cpu_traffic_group_field_value_dump(tbl_cpu_traffic_group_t *p_cpu_traffic_group, int32 field_id, char* str);

char**
tbl_cpu_traffic_group_table_dump(tbl_cpu_traffic_group_t *p_cpu_traffic_group, char** str);

int32
tbl_cpu_traffic_group_table_parser(char** array, char* key_value, tbl_cpu_traffic_group_t *p_cpu_traffic_group);

int32
tbl_cpu_traffic_group_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_cpu_traffic_group_master_t*
tbl_cpu_traffic_group_get_master();

tbl_cpu_traffic_group_master_t*
tbl_cpu_traffic_group_init_cpu_traffic_group();

#endif /* !__TBL_CPU_TRAFFIC_GROUP_H__ */

