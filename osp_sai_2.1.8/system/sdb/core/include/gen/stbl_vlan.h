
#ifndef __STBL_VLAN_H__
#define __STBL_VLAN_H__

int32
stbl_vlan_add_vlan_sync(stbl_vlan_t *p_vlan, uint32 sync);

int32
stbl_vlan_del_vlan_sync(stbl_vlan_key_t *p_vlan_key, uint32 sync);

int32
stbl_vlan_set_vlan_field_sync(stbl_vlan_t *p_vlan, stbl_vlan_field_id_t field_id, uint32 sync);

int32
stbl_vlan_add_vlan(stbl_vlan_t *p_vlan);

int32
stbl_vlan_del_vlan(stbl_vlan_key_t *p_vlan_key);

int32
stbl_vlan_set_vlan_field(stbl_vlan_t *p_vlan, stbl_vlan_field_id_t field_id);

stbl_vlan_t*
stbl_vlan_get_vlan(stbl_vlan_key_t *p_vlan_key);

char*
stbl_vlan_key_val2str(stbl_vlan_t *p_vlan, char *str, uint32 str_len);

int32
stbl_vlan_key_str2val(char *str, stbl_vlan_t *p_vlan);

int32
stbl_vlan_dump_one(stbl_vlan_t *p_vlan, stbl_iter_args_t *pargs);

int32
stbl_vlan_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_vlan_master_t*
stbl_vlan_get_master();

stbl_vlan_master_t*
stbl_vlan_init_vlan();

#endif /* !__STBL_VLAN_H__ */

