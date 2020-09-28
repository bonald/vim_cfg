
#ifndef __TBL_TAP_GROUP_INGRESS_H__
#define __TBL_TAP_GROUP_INGRESS_H__

int32
tbl_tap_group_ingress_add_tap_group_ingress_sync(tbl_tap_group_ingress_t *p_tap_group_ingress, uint32 sync);

int32
tbl_tap_group_ingress_del_tap_group_ingress_sync(tbl_tap_group_ingress_key_t *p_tap_group_ingress_key, uint32 sync);

int32
tbl_tap_group_ingress_set_tap_group_ingress_field_sync(tbl_tap_group_ingress_t *p_tap_group_ingress, tbl_tap_group_ingress_field_id_t field_id, uint32 sync);

int32
tbl_tap_group_ingress_add_tap_group_ingress(tbl_tap_group_ingress_t *p_tap_group_ingress);

int32
tbl_tap_group_ingress_del_tap_group_ingress(tbl_tap_group_ingress_key_t *p_tap_group_ingress_key);

int32
tbl_tap_group_ingress_set_tap_group_ingress_field(tbl_tap_group_ingress_t *p_tap_group_ingress, tbl_tap_group_ingress_field_id_t field_id);

tbl_tap_group_ingress_t*
tbl_tap_group_ingress_get_tap_group_ingress(tbl_tap_group_ingress_key_t *p_tap_group_ingress_key);

char*
tbl_tap_group_ingress_key_val2str(tbl_tap_group_ingress_t *p_tap_group_ingress, char *str, uint32 str_len);

int32
tbl_tap_group_ingress_key_str2val(char *str, tbl_tap_group_ingress_t *p_tap_group_ingress);

int32
tbl_tap_group_ingress_dump_one(tbl_tap_group_ingress_t *p_tap_group_ingress, tbl_iter_args_t *pargs);

char*
tbl_tap_group_ingress_key_name_dump(tbl_tap_group_ingress_t *p_tap_group_ingress, char* str);

char*
tbl_tap_group_ingress_key_value_dump(tbl_tap_group_ingress_t *p_tap_group_ingress, char* str);

char*
tbl_tap_group_ingress_field_name_dump(tbl_tap_group_ingress_t *p_tap_group_ingress, int32 field_id, char* str);

char*
tbl_tap_group_ingress_field_value_dump(tbl_tap_group_ingress_t *p_tap_group_ingress, int32 field_id, char* str);

char**
tbl_tap_group_ingress_table_dump(tbl_tap_group_ingress_t *p_tap_group_ingress, char** str);

int32
tbl_tap_group_ingress_table_parser(char** array, char* key_value, tbl_tap_group_ingress_t *p_tap_group_ingress);

int32
tbl_tap_group_ingress_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_tap_group_ingress_master_t*
tbl_tap_group_ingress_get_master();

tbl_tap_group_ingress_master_t*
tbl_tap_group_ingress_init_tap_group_ingress();

#endif /* !__TBL_TAP_GROUP_INGRESS_H__ */

