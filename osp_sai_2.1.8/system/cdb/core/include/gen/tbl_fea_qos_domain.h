
#ifndef __TBL_FEA_QOS_DOMAIN_H__
#define __TBL_FEA_QOS_DOMAIN_H__

int32
tbl_fea_qos_domain_add_fea_qos_domain_sync(tbl_fea_qos_domain_t *p_fea_qos_domain, uint32 sync);

int32
tbl_fea_qos_domain_del_fea_qos_domain_sync(tbl_fea_qos_domain_key_t *p_fea_qos_domain_key, uint32 sync);

int32
tbl_fea_qos_domain_set_fea_qos_domain_field_sync(tbl_fea_qos_domain_t *p_fea_qos_domain, tbl_fea_qos_domain_field_id_t field_id, uint32 sync);

int32
tbl_fea_qos_domain_add_fea_qos_domain(tbl_fea_qos_domain_t *p_fea_qos_domain);

int32
tbl_fea_qos_domain_del_fea_qos_domain(tbl_fea_qos_domain_key_t *p_fea_qos_domain_key);

int32
tbl_fea_qos_domain_set_fea_qos_domain_field(tbl_fea_qos_domain_t *p_fea_qos_domain, tbl_fea_qos_domain_field_id_t field_id);

tbl_fea_qos_domain_t*
tbl_fea_qos_domain_get_fea_qos_domain(tbl_fea_qos_domain_key_t *p_fea_qos_domain_key);

char*
tbl_fea_qos_domain_key_val2str(tbl_fea_qos_domain_t *p_fea_qos_domain, char *str, uint32 str_len);

int32
tbl_fea_qos_domain_key_str2val(char *str, tbl_fea_qos_domain_t *p_fea_qos_domain);

int32
tbl_fea_qos_domain_dump_one(tbl_fea_qos_domain_t *p_fea_qos_domain, tbl_iter_args_t *pargs);

char*
tbl_fea_qos_domain_key_name_dump(tbl_fea_qos_domain_t *p_fea_qos_domain, char* str);

char*
tbl_fea_qos_domain_key_value_dump(tbl_fea_qos_domain_t *p_fea_qos_domain, char* str);

char*
tbl_fea_qos_domain_field_name_dump(tbl_fea_qos_domain_t *p_fea_qos_domain, int32 field_id, char* str);

char*
tbl_fea_qos_domain_field_value_dump(tbl_fea_qos_domain_t *p_fea_qos_domain, int32 field_id, char* str);

char**
tbl_fea_qos_domain_table_dump(tbl_fea_qos_domain_t *p_fea_qos_domain, char** str);

int32
tbl_fea_qos_domain_table_parser(char** array, char* key_value, tbl_fea_qos_domain_t *p_fea_qos_domain);

int32
tbl_fea_qos_domain_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fea_qos_domain_master_t*
tbl_fea_qos_domain_get_master();

tbl_fea_qos_domain_master_t*
tbl_fea_qos_domain_init_fea_qos_domain();

#endif /* !__TBL_FEA_QOS_DOMAIN_H__ */

