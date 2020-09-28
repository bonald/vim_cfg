
#ifndef __TBL_QOS_POLICER_RES_H__
#define __TBL_QOS_POLICER_RES_H__

int32
tbl_qos_policer_res_add_qos_policer_res_sync(tbl_qos_policer_res_t *p_qos_policer_res, uint32 sync);

int32
tbl_qos_policer_res_del_qos_policer_res_sync(tbl_qos_policer_res_key_t *p_qos_policer_res_key, uint32 sync);

int32
tbl_qos_policer_res_set_qos_policer_res_field_sync(tbl_qos_policer_res_t *p_qos_policer_res, tbl_qos_policer_res_field_id_t field_id, uint32 sync);

int32
tbl_qos_policer_res_add_qos_policer_res(tbl_qos_policer_res_t *p_qos_policer_res);

int32
tbl_qos_policer_res_del_qos_policer_res(tbl_qos_policer_res_key_t *p_qos_policer_res_key);

int32
tbl_qos_policer_res_set_qos_policer_res_field(tbl_qos_policer_res_t *p_qos_policer_res, tbl_qos_policer_res_field_id_t field_id);

tbl_qos_policer_res_t*
tbl_qos_policer_res_get_qos_policer_res(tbl_qos_policer_res_key_t *p_qos_policer_res_key);

char*
tbl_qos_policer_res_key_val2str(tbl_qos_policer_res_t *p_qos_policer_res, char *str, uint32 str_len);

int32
tbl_qos_policer_res_key_str2val(char *str, tbl_qos_policer_res_t *p_qos_policer_res);

int32
tbl_qos_policer_res_dump_one(tbl_qos_policer_res_t *p_qos_policer_res, tbl_iter_args_t *pargs);

char*
tbl_qos_policer_res_key_name_dump(tbl_qos_policer_res_t *p_qos_policer_res, char* str);

char*
tbl_qos_policer_res_key_value_dump(tbl_qos_policer_res_t *p_qos_policer_res, char* str);

char*
tbl_qos_policer_res_field_name_dump(tbl_qos_policer_res_t *p_qos_policer_res, int32 field_id, char* str);

char*
tbl_qos_policer_res_field_value_dump(tbl_qos_policer_res_t *p_qos_policer_res, int32 field_id, char* str);

char**
tbl_qos_policer_res_table_dump(tbl_qos_policer_res_t *p_qos_policer_res, char** str);

int32
tbl_qos_policer_res_table_parser(char** array, char* key_value, tbl_qos_policer_res_t *p_qos_policer_res);

int32
tbl_qos_policer_res_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_qos_policer_res_master_t*
tbl_qos_policer_res_get_master();

tbl_qos_policer_res_master_t*
tbl_qos_policer_res_init_qos_policer_res();

#endif /* !__TBL_QOS_POLICER_RES_H__ */

