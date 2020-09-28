
#ifndef __STBL_QOS_MPLS_DOMAIN_H__
#define __STBL_QOS_MPLS_DOMAIN_H__

int32
stbl_qos_mpls_domain_add_qos_mpls_domain_sync(stbl_qos_mpls_domain_t *p_qos_mpls_domain, uint32 sync);

int32
stbl_qos_mpls_domain_del_qos_mpls_domain_sync(stbl_qos_mpls_domain_key_t *p_qos_mpls_domain_key, uint32 sync);

int32
stbl_qos_mpls_domain_set_qos_mpls_domain_field_sync(stbl_qos_mpls_domain_t *p_qos_mpls_domain, stbl_qos_mpls_domain_field_id_t field_id, uint32 sync);

int32
stbl_qos_mpls_domain_add_qos_mpls_domain(stbl_qos_mpls_domain_t *p_qos_mpls_domain);

int32
stbl_qos_mpls_domain_del_qos_mpls_domain(stbl_qos_mpls_domain_key_t *p_qos_mpls_domain_key);

int32
stbl_qos_mpls_domain_set_qos_mpls_domain_field(stbl_qos_mpls_domain_t *p_qos_mpls_domain, stbl_qos_mpls_domain_field_id_t field_id);

stbl_qos_mpls_domain_t*
stbl_qos_mpls_domain_get_qos_mpls_domain(stbl_qos_mpls_domain_key_t *p_qos_mpls_domain_key);

char*
stbl_qos_mpls_domain_key_val2str(stbl_qos_mpls_domain_t *p_qos_mpls_domain, char *str, uint32 str_len);

int32
stbl_qos_mpls_domain_key_str2val(char *str, stbl_qos_mpls_domain_t *p_qos_mpls_domain);

int32
stbl_qos_mpls_domain_dump_one(stbl_qos_mpls_domain_t *p_qos_mpls_domain, stbl_iter_args_t *pargs);

int32
stbl_qos_mpls_domain_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_qos_mpls_domain_master_t*
stbl_qos_mpls_domain_get_master();

stbl_qos_mpls_domain_master_t*
stbl_qos_mpls_domain_init_qos_mpls_domain();

#endif /* !__STBL_QOS_MPLS_DOMAIN_H__ */

