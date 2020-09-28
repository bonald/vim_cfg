
#ifndef __TBL_VRRP_VMAC_H__
#define __TBL_VRRP_VMAC_H__

int32
tbl_vrrp_vmac_add_vrrp_vmac_sync(tbl_vrrp_vmac_t *p_vrrp_vmac, uint32 sync);

int32
tbl_vrrp_vmac_del_vrrp_vmac_sync(tbl_vrrp_vmac_key_t *p_vrrp_vmac_key, uint32 sync);

int32
tbl_vrrp_vmac_set_vrrp_vmac_field_sync(tbl_vrrp_vmac_t *p_vrrp_vmac, tbl_vrrp_vmac_field_id_t field_id, uint32 sync);

int32
tbl_vrrp_vmac_add_vrrp_vmac(tbl_vrrp_vmac_t *p_vrrp_vmac);

int32
tbl_vrrp_vmac_del_vrrp_vmac(tbl_vrrp_vmac_key_t *p_vrrp_vmac_key);

int32
tbl_vrrp_vmac_set_vrrp_vmac_field(tbl_vrrp_vmac_t *p_vrrp_vmac, tbl_vrrp_vmac_field_id_t field_id);

tbl_vrrp_vmac_t*
tbl_vrrp_vmac_get_vrrp_vmac(tbl_vrrp_vmac_key_t *p_vrrp_vmac_key);

char*
tbl_vrrp_vmac_key_val2str(tbl_vrrp_vmac_t *p_vrrp_vmac, char *str, uint32 str_len);

int32
tbl_vrrp_vmac_key_str2val(char *str, tbl_vrrp_vmac_t *p_vrrp_vmac);

int32
tbl_vrrp_vmac_dump_one(tbl_vrrp_vmac_t *p_vrrp_vmac, tbl_iter_args_t *pargs);

char*
tbl_vrrp_vmac_key_name_dump(tbl_vrrp_vmac_t *p_vrrp_vmac, char* str);

char*
tbl_vrrp_vmac_key_value_dump(tbl_vrrp_vmac_t *p_vrrp_vmac, char* str);

char*
tbl_vrrp_vmac_field_name_dump(tbl_vrrp_vmac_t *p_vrrp_vmac, int32 field_id, char* str);

char*
tbl_vrrp_vmac_field_value_dump(tbl_vrrp_vmac_t *p_vrrp_vmac, int32 field_id, char* str);

char**
tbl_vrrp_vmac_table_dump(tbl_vrrp_vmac_t *p_vrrp_vmac, char** str);

int32
tbl_vrrp_vmac_table_parser(char** array, char* key_value, tbl_vrrp_vmac_t *p_vrrp_vmac);

int32
tbl_vrrp_vmac_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_vrrp_vmac_master_t*
tbl_vrrp_vmac_get_master();

tbl_vrrp_vmac_master_t*
tbl_vrrp_vmac_init_vrrp_vmac();

#endif /* !__TBL_VRRP_VMAC_H__ */

