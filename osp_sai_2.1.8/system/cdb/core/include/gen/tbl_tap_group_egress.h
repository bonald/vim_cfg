
#ifndef __TBL_TAP_GROUP_EGRESS_H__
#define __TBL_TAP_GROUP_EGRESS_H__

int32
tbl_tap_group_egress_add_tap_group_egress_sync(tbl_tap_group_egress_t *p_tap_group_egress, uint32 sync);

int32
tbl_tap_group_egress_del_tap_group_egress_sync(tbl_tap_group_egress_key_t *p_tap_group_egress_key, uint32 sync);

int32
tbl_tap_group_egress_set_tap_group_egress_field_sync(tbl_tap_group_egress_t *p_tap_group_egress, tbl_tap_group_egress_field_id_t field_id, uint32 sync);

int32
tbl_tap_group_egress_add_tap_group_egress(tbl_tap_group_egress_t *p_tap_group_egress);

int32
tbl_tap_group_egress_del_tap_group_egress(tbl_tap_group_egress_key_t *p_tap_group_egress_key);

int32
tbl_tap_group_egress_set_tap_group_egress_field(tbl_tap_group_egress_t *p_tap_group_egress, tbl_tap_group_egress_field_id_t field_id);

tbl_tap_group_egress_t*
tbl_tap_group_egress_get_tap_group_egress(tbl_tap_group_egress_key_t *p_tap_group_egress_key);

char*
tbl_tap_group_egress_key_val2str(tbl_tap_group_egress_t *p_tap_group_egress, char *str, uint32 str_len);

int32
tbl_tap_group_egress_key_str2val(char *str, tbl_tap_group_egress_t *p_tap_group_egress);

int32
tbl_tap_group_egress_dump_one(tbl_tap_group_egress_t *p_tap_group_egress, tbl_iter_args_t *pargs);

char*
tbl_tap_group_egress_key_name_dump(tbl_tap_group_egress_t *p_tap_group_egress, char* str);

char*
tbl_tap_group_egress_key_value_dump(tbl_tap_group_egress_t *p_tap_group_egress, char* str);

char*
tbl_tap_group_egress_field_name_dump(tbl_tap_group_egress_t *p_tap_group_egress, int32 field_id, char* str);

char*
tbl_tap_group_egress_field_value_dump(tbl_tap_group_egress_t *p_tap_group_egress, int32 field_id, char* str);

char**
tbl_tap_group_egress_table_dump(tbl_tap_group_egress_t *p_tap_group_egress, char** str);

int32
tbl_tap_group_egress_table_parser(char** array, char* key_value, tbl_tap_group_egress_t *p_tap_group_egress);

int32
tbl_tap_group_egress_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_tap_group_egress_master_t*
tbl_tap_group_egress_get_master();

tbl_tap_group_egress_master_t*
tbl_tap_group_egress_init_tap_group_egress();

#endif /* !__TBL_TAP_GROUP_EGRESS_H__ */

