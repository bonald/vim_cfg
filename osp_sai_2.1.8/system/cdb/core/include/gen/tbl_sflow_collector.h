
#ifndef __TBL_SFLOW_COLLECTOR_H__
#define __TBL_SFLOW_COLLECTOR_H__

int32
tbl_sflow_collector_add_sflow_collector_sync(tbl_sflow_collector_t *p_sflow_collector, uint32 sync);

int32
tbl_sflow_collector_del_sflow_collector_sync(tbl_sflow_collector_key_t *p_sflow_collector_key, uint32 sync);

int32
tbl_sflow_collector_set_sflow_collector_field_sync(tbl_sflow_collector_t *p_sflow_collector, tbl_sflow_collector_field_id_t field_id, uint32 sync);

int32
tbl_sflow_collector_add_sflow_collector(tbl_sflow_collector_t *p_sflow_collector);

int32
tbl_sflow_collector_del_sflow_collector(tbl_sflow_collector_key_t *p_sflow_collector_key);

int32
tbl_sflow_collector_set_sflow_collector_field(tbl_sflow_collector_t *p_sflow_collector, tbl_sflow_collector_field_id_t field_id);

tbl_sflow_collector_t*
tbl_sflow_collector_get_sflow_collector(tbl_sflow_collector_key_t *p_sflow_collector_key);

char*
tbl_sflow_collector_key_val2str(tbl_sflow_collector_t *p_sflow_collector, char *str, uint32 str_len);

int32
tbl_sflow_collector_key_str2val(char *str, tbl_sflow_collector_t *p_sflow_collector);

int32
tbl_sflow_collector_dump_one(tbl_sflow_collector_t *p_sflow_collector, tbl_iter_args_t *pargs);

char*
tbl_sflow_collector_key_name_dump(tbl_sflow_collector_t *p_sflow_collector, char* str);

char*
tbl_sflow_collector_key_value_dump(tbl_sflow_collector_t *p_sflow_collector, char* str);

char*
tbl_sflow_collector_field_name_dump(tbl_sflow_collector_t *p_sflow_collector, int32 field_id, char* str);

char*
tbl_sflow_collector_field_value_dump(tbl_sflow_collector_t *p_sflow_collector, int32 field_id, char* str);

char**
tbl_sflow_collector_table_dump(tbl_sflow_collector_t *p_sflow_collector, char** str);

int32
tbl_sflow_collector_table_parser(char** array, char* key_value, tbl_sflow_collector_t *p_sflow_collector);

int32
tbl_sflow_collector_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_sflow_collector_master_t*
tbl_sflow_collector_get_master();

tbl_sflow_collector_master_t*
tbl_sflow_collector_init_sflow_collector();

#endif /* !__TBL_SFLOW_COLLECTOR_H__ */

