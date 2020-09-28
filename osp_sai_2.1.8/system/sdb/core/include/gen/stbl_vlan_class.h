
#ifndef __STBL_VLAN_CLASS_H__
#define __STBL_VLAN_CLASS_H__

int32
stbl_vlan_class_add_vlan_class_sync(stbl_vlan_class_t *p_vclass, uint32 sync);

int32
stbl_vlan_class_del_vlan_class_sync(stbl_vlan_class_key_t *p_vclass_key, uint32 sync);

int32
stbl_vlan_class_set_vlan_class_field_sync(stbl_vlan_class_t *p_vclass, stbl_vlan_class_field_id_t field_id, uint32 sync);

int32
stbl_vlan_class_add_vlan_class(stbl_vlan_class_t *p_vclass);

int32
stbl_vlan_class_del_vlan_class(stbl_vlan_class_key_t *p_vclass_key);

int32
stbl_vlan_class_set_vlan_class_field(stbl_vlan_class_t *p_vclass, stbl_vlan_class_field_id_t field_id);

stbl_vlan_class_t*
stbl_vlan_class_get_vlan_class(stbl_vlan_class_key_t *p_vclass_key);

char*
stbl_vlan_class_key_val2str(stbl_vlan_class_t *p_vclass, char *str, uint32 str_len);

int32
stbl_vlan_class_key_str2val(char *str, stbl_vlan_class_t *p_vclass);

int32
stbl_vlan_class_dump_one(stbl_vlan_class_t *p_vclass, stbl_iter_args_t *pargs);

int32
stbl_vlan_class_iterate(STBL_ITER_CB_FUNC fn, stbl_iter_args_t *pargs);

stbl_vlan_class_master_t*
stbl_vlan_class_get_master();

stbl_vlan_class_master_t*
stbl_vlan_class_init_vlan_class();

#endif /* !__STBL_VLAN_CLASS_H__ */

