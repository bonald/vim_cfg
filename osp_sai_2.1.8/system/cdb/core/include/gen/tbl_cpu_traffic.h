
#ifndef __TBL_CPU_TRAFFIC_H__
#define __TBL_CPU_TRAFFIC_H__

int32
tbl_cpu_traffic_add_cpu_traffic_sync(tbl_cpu_traffic_t *p_cpu_traffic, uint32 sync);

int32
tbl_cpu_traffic_del_cpu_traffic_sync(tbl_cpu_traffic_key_t *p_cpu_traffic_key, uint32 sync);

int32
tbl_cpu_traffic_set_cpu_traffic_field_sync(tbl_cpu_traffic_t *p_cpu_traffic, tbl_cpu_traffic_field_id_t field_id, uint32 sync);

int32
tbl_cpu_traffic_add_cpu_traffic(tbl_cpu_traffic_t *p_cpu_traffic);

int32
tbl_cpu_traffic_del_cpu_traffic(tbl_cpu_traffic_key_t *p_cpu_traffic_key);

int32
tbl_cpu_traffic_set_cpu_traffic_field(tbl_cpu_traffic_t *p_cpu_traffic, tbl_cpu_traffic_field_id_t field_id);

tbl_cpu_traffic_t*
tbl_cpu_traffic_get_cpu_traffic(tbl_cpu_traffic_key_t *p_cpu_traffic_key);

char*
tbl_cpu_traffic_key_val2str(tbl_cpu_traffic_t *p_cpu_traffic, char *str, uint32 str_len);

int32
tbl_cpu_traffic_key_str2val(char *str, tbl_cpu_traffic_t *p_cpu_traffic);

int32
tbl_cpu_traffic_dump_one(tbl_cpu_traffic_t *p_cpu_traffic, tbl_iter_args_t *pargs);

char*
tbl_cpu_traffic_key_name_dump(tbl_cpu_traffic_t *p_cpu_traffic, char* str);

char*
tbl_cpu_traffic_key_value_dump(tbl_cpu_traffic_t *p_cpu_traffic, char* str);

char*
tbl_cpu_traffic_field_name_dump(tbl_cpu_traffic_t *p_cpu_traffic, int32 field_id, char* str);

char*
tbl_cpu_traffic_field_value_dump(tbl_cpu_traffic_t *p_cpu_traffic, int32 field_id, char* str);

char**
tbl_cpu_traffic_table_dump(tbl_cpu_traffic_t *p_cpu_traffic, char** str);

int32
tbl_cpu_traffic_table_parser(char** array, char* key_value, tbl_cpu_traffic_t *p_cpu_traffic);

int32
tbl_cpu_traffic_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_cpu_traffic_master_t*
tbl_cpu_traffic_get_master();

tbl_cpu_traffic_master_t*
tbl_cpu_traffic_init_cpu_traffic();

#endif /* !__TBL_CPU_TRAFFIC_H__ */

