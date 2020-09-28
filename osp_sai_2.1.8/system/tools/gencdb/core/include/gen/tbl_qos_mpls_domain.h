
#ifndef __TBL_QOS_MPLS_DOMAIN_H__
#define __TBL_QOS_MPLS_DOMAIN_H__

int32
tbl_qos_mpls_domain_add_qos_mpls_domain_sync(tbl_qos_mpls_domain_t *p_qos_mpls_domain, uint32 sync);

int32
tbl_qos_mpls_domain_del_qos_mpls_domain_sync(tbl_qos_mpls_domain_key_t *p_qos_mpls_domain_key, uint32 sync);

int32
tbl_qos_mpls_domain_set_qos_mpls_domain_field_sync(tbl_qos_mpls_domain_t *p_qos_mpls_domain, tbl_qos_mpls_domain_field_id_t field_id, uint32 sync);

int32
tbl_qos_mpls_domain_add_qos_mpls_domain(tbl_qos_mpls_domain_t *p_qos_mpls_domain);

int32
tbl_qos_mpls_domain_del_qos_mpls_domain(tbl_qos_mpls_domain_key_t *p_qos_mpls_domain_key);

int32
tbl_qos_mpls_domain_set_qos_mpls_domain_field(tbl_qos_mpls_domain_t *p_qos_mpls_domain, tbl_qos_mpls_domain_field_id_t field_id);

tbl_qos_mpls_domain_t*
tbl_qos_mpls_domain_get_qos_mpls_domain(tbl_qos_mpls_domain_key_t *p_qos_mpls_domain_key);

char*
tbl_qos_mpls_domain_key_val2str(tbl_qos_mpls_domain_t *p_qos_mpls_domain, char *str, uint32 str_len);

int32
tbl_qos_mpls_domain_key_str2val(char *str, tbl_qos_mpls_domain_t *p_qos_mpls_domain);

int32
tbl_qos_mpls_domain_dump_one(tbl_qos_mpls_domain_t *p_qos_mpls_domain, tbl_iter_args_t *pargs);

char*
tbl_qos_mpls_domain_key_name_dump(tbl_qos_mpls_domain_t *p_qos_mpls_domain, char* str);

char*
tbl_qos_mpls_domain_key_value_dump(tbl_qos_mpls_domain_t *p_qos_mpls_domain, char* str);

char*
tbl_qos_mpls_domain_field_name_dump(tbl_qos_mpls_domain_t *p_qos_mpls_domain, int32 field_id, char* str);

char*
tbl_qos_mpls_domain_field_value_dump(tbl_qos_mpls_domain_t *p_qos_mpls_domain, int32 field_id, char* str);

char**
tbl_qos_mpls_domain_table_dump(tbl_qos_mpls_domain_t *p_qos_mpls_domain, char** str);

int32
tbl_qos_mpls_domain_table_parser(char** array, char* key_value, tbl_qos_mpls_domain_t *p_qos_mpls_domain);

int32
tbl_qos_mpls_domain_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_qos_mpls_domain_master_t*
tbl_qos_mpls_domain_get_master();

tbl_qos_mpls_domain_master_t*
tbl_qos_mpls_domain_init_qos_mpls_domain();

#endif /* !__TBL_QOS_MPLS_DOMAIN_H__ */

