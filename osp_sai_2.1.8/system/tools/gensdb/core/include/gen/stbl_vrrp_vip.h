
#ifndef __STBL_VRRP_VIP_H__
#define __STBL_VRRP_VIP_H__

int32
stbl_vrrp_vip_add_vrrp_vip_sync(stbl_vrrp_vip_t *p_vip, uint32 sync);

int32
stbl_vrrp_vip_del_vrrp_vip_sync(stbl_vrrp_vip_key_t *p_vip_key, uint32 sync);

int32
stbl_vrrp_vip_set_vrrp_vip_field_sync(stbl_vrrp_vip_t *p_vip, stbl_vrrp_vip_field_id_t field_id, uint32 sync);

int32
stbl_vrrp_vip_add_vrrp_vip(stbl_vrrp_vip_t *p_vip);

int32
stbl_vrrp_vip_del_vrrp_vip(stbl_vrrp_vip_key_t *p_vip_key);

int32
stbl_vrrp_vip_set_vrrp_vip_field(stbl_vrrp_vip_t *p_vip, stbl_vrrp_vip_field_id_t field_id);

stbl_vrrp_vip_t*
stbl_vrrp_vip_get_vrrp_vip(stbl_vrrp_vip_key_t *p_vip_key);

char*
stbl_vrrp_vip_key_val2str(stbl_vrrp_vip_t *p_vip, char *str, uint32 str_len);

int32
stbl_vrrp_vip_key_str2val(char *str, stbl_vrrp_vip_t *p_vip);

int32
stbl_vrrp_vip_dump_one(stbl_vrrp_vip_t *p_vip, stbl_iter_args_t *pargs);

int32
stbl_vrrp_vip_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_vrrp_vip_master_t*
stbl_vrrp_vip_get_master();

stbl_vrrp_vip_master_t*
stbl_vrrp_vip_init_vrrp_vip();

#endif /* !__STBL_VRRP_VIP_H__ */

