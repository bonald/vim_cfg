
#ifndef __TBL_QOS_GLOBAL_H__
#define __TBL_QOS_GLOBAL_H__

int32
tbl_qos_global_set_qos_global_field_sync(tbl_qos_global_t *p_qos_glb, tbl_qos_global_field_id_t field_id, uint32 sync);

int32
tbl_qos_global_set_qos_global_field(tbl_qos_global_t *p_qos_glb, tbl_qos_global_field_id_t field_id);

tbl_qos_global_t*
tbl_qos_global_get_qos_global();

char*
tbl_qos_global_key_val2str(tbl_qos_global_t *p_qos_glb, char *str, uint32 str_len);

int32
tbl_qos_global_key_str2val(char *str, tbl_qos_global_t *p_qos_glb);

int32
tbl_qos_global_dump_one(tbl_qos_global_t *p_qos_glb, tbl_iter_args_t *pargs);

char*
tbl_qos_global_key_name_dump(tbl_qos_global_t *p_qos_glb, char* str);

char*
tbl_qos_global_key_value_dump(tbl_qos_global_t *p_qos_glb, char* str);

char*
tbl_qos_global_field_name_dump(tbl_qos_global_t *p_qos_glb, int32 field_id, char* str);

char*
tbl_qos_global_field_value_dump(tbl_qos_global_t *p_qos_glb, int32 field_id, char* str);

char**
tbl_qos_global_table_dump(tbl_qos_global_t *p_qos_glb, char** str);

int32
tbl_qos_global_table_parser(char** array, char* key_value, tbl_qos_global_t *p_qos_glb);

int32
tbl_qos_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_qos_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_qos_global_t*
tbl_qos_global_init_qos_global();

bool
tbl_qos_global_get_phb_enable();

bool
tbl_qos_global_get_qos_enable();

bool
tbl_qos_global_get_port_policer_first_enable();

bool
tbl_qos_global_get_policer_stats_enable();

int32
tbl_qos_global_get_cur_cpu_rate();

int32
tbl_qos_global_get_def_cpu_rate();

#endif /* !__TBL_QOS_GLOBAL_H__ */

