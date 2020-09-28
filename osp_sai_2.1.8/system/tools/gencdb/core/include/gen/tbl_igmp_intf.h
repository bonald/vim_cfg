
#ifndef __TBL_IGMP_INTF_H__
#define __TBL_IGMP_INTF_H__

int32
tbl_igmp_intf_add_igmp_intf_sync(tbl_igmp_intf_t *p_igmp_intf, uint32 sync);

int32
tbl_igmp_intf_del_igmp_intf_sync(tbl_igmp_intf_key_t *p_igmp_intf_key, uint32 sync);

int32
tbl_igmp_intf_set_igmp_intf_field_sync(tbl_igmp_intf_t *p_igmp_intf, tbl_igmp_intf_field_id_t field_id, uint32 sync);

int32
tbl_igmp_intf_add_igmp_intf(tbl_igmp_intf_t *p_igmp_intf);

int32
tbl_igmp_intf_del_igmp_intf(tbl_igmp_intf_key_t *p_igmp_intf_key);

int32
tbl_igmp_intf_set_igmp_intf_field(tbl_igmp_intf_t *p_igmp_intf, tbl_igmp_intf_field_id_t field_id);

tbl_igmp_intf_t*
tbl_igmp_intf_get_igmp_intf(tbl_igmp_intf_key_t *p_igmp_intf_key);

char*
tbl_igmp_intf_key_val2str(tbl_igmp_intf_t *p_igmp_intf, char *str, uint32 str_len);

int32
tbl_igmp_intf_key_str2val(char *str, tbl_igmp_intf_t *p_igmp_intf);

int32
tbl_igmp_intf_dump_one(tbl_igmp_intf_t *p_igmp_intf, tbl_iter_args_t *pargs);

char*
tbl_igmp_intf_key_name_dump(tbl_igmp_intf_t *p_igmp_intf, char* str);

char*
tbl_igmp_intf_key_value_dump(tbl_igmp_intf_t *p_igmp_intf, char* str);

char*
tbl_igmp_intf_field_name_dump(tbl_igmp_intf_t *p_igmp_intf, int32 field_id, char* str);

char*
tbl_igmp_intf_field_value_dump(tbl_igmp_intf_t *p_igmp_intf, int32 field_id, char* str);

char**
tbl_igmp_intf_table_dump(tbl_igmp_intf_t *p_igmp_intf, char** str);

int32
tbl_igmp_intf_table_parser(char** array, char* key_value, tbl_igmp_intf_t *p_igmp_intf);

int32
tbl_igmp_intf_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_igmp_intf_master_t*
tbl_igmp_intf_get_master();

tbl_igmp_intf_master_t*
tbl_igmp_intf_init_igmp_intf();

#endif /* !__TBL_IGMP_INTF_H__ */

