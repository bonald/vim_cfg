
#ifndef __TBL_VRRP_VIP_H__
#define __TBL_VRRP_VIP_H__

int32
tbl_vrrp_vip_add_vrrp_vip_sync(tbl_vrrp_vip_t *p_vrrp_vip, uint32 sync);

int32
tbl_vrrp_vip_del_vrrp_vip_sync(tbl_vrrp_vip_key_t *p_vrrp_vip_key, uint32 sync);

int32
tbl_vrrp_vip_set_vrrp_vip_field_sync(tbl_vrrp_vip_t *p_vrrp_vip, tbl_vrrp_vip_field_id_t field_id, uint32 sync);

int32
tbl_vrrp_vip_add_vrrp_vip(tbl_vrrp_vip_t *p_vrrp_vip);

int32
tbl_vrrp_vip_del_vrrp_vip(tbl_vrrp_vip_key_t *p_vrrp_vip_key);

int32
tbl_vrrp_vip_set_vrrp_vip_field(tbl_vrrp_vip_t *p_vrrp_vip, tbl_vrrp_vip_field_id_t field_id);

tbl_vrrp_vip_t*
tbl_vrrp_vip_get_vrrp_vip(tbl_vrrp_vip_key_t *p_vrrp_vip_key);

char*
tbl_vrrp_vip_key_val2str(tbl_vrrp_vip_t *p_vrrp_vip, char *str, uint32 str_len);

int32
tbl_vrrp_vip_key_str2val(char *str, tbl_vrrp_vip_t *p_vrrp_vip);

int32
tbl_vrrp_vip_dump_one(tbl_vrrp_vip_t *p_vrrp_vip, tbl_iter_args_t *pargs);

char*
tbl_vrrp_vip_key_name_dump(tbl_vrrp_vip_t *p_vrrp_vip, char* str);

char*
tbl_vrrp_vip_key_value_dump(tbl_vrrp_vip_t *p_vrrp_vip, char* str);

char*
tbl_vrrp_vip_field_name_dump(tbl_vrrp_vip_t *p_vrrp_vip, int32 field_id, char* str);

char*
tbl_vrrp_vip_field_value_dump(tbl_vrrp_vip_t *p_vrrp_vip, int32 field_id, char* str);

char**
tbl_vrrp_vip_table_dump(tbl_vrrp_vip_t *p_vrrp_vip, char** str);

int32
tbl_vrrp_vip_table_parser(char** array, char* key_value, tbl_vrrp_vip_t *p_vrrp_vip);

int32
tbl_vrrp_vip_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_vrrp_vip_master_t*
tbl_vrrp_vip_get_master();

tbl_vrrp_vip_master_t*
tbl_vrrp_vip_init_vrrp_vip();

#endif /* !__TBL_VRRP_VIP_H__ */

