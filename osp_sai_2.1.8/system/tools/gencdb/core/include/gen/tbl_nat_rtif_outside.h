
#ifndef __TBL_NAT_RTIF_OUTSIDE_H__
#define __TBL_NAT_RTIF_OUTSIDE_H__

int32
tbl_nat_rtif_outside_add_nat_rtif_outside_sync(tbl_nat_rtif_outside_t *p_nat_rtif_outside, uint32 sync);

int32
tbl_nat_rtif_outside_del_nat_rtif_outside_sync(tbl_nat_rtif_outside_key_t *p_nat_rtif_outside_key, uint32 sync);

int32
tbl_nat_rtif_outside_set_nat_rtif_outside_field_sync(tbl_nat_rtif_outside_t *p_nat_rtif_outside, tbl_nat_rtif_outside_field_id_t field_id, uint32 sync);

int32
tbl_nat_rtif_outside_add_nat_rtif_outside(tbl_nat_rtif_outside_t *p_nat_rtif_outside);

int32
tbl_nat_rtif_outside_del_nat_rtif_outside(tbl_nat_rtif_outside_key_t *p_nat_rtif_outside_key);

int32
tbl_nat_rtif_outside_set_nat_rtif_outside_field(tbl_nat_rtif_outside_t *p_nat_rtif_outside, tbl_nat_rtif_outside_field_id_t field_id);

tbl_nat_rtif_outside_t*
tbl_nat_rtif_outside_get_nat_rtif_outside(tbl_nat_rtif_outside_key_t *p_nat_rtif_outside_key);

char*
tbl_nat_rtif_outside_key_val2str(tbl_nat_rtif_outside_t *p_nat_rtif_outside, char *str, uint32 str_len);

int32
tbl_nat_rtif_outside_key_str2val(char *str, tbl_nat_rtif_outside_t *p_nat_rtif_outside);

int32
tbl_nat_rtif_outside_dump_one(tbl_nat_rtif_outside_t *p_nat_rtif_outside, tbl_iter_args_t *pargs);

char*
tbl_nat_rtif_outside_key_name_dump(tbl_nat_rtif_outside_t *p_nat_rtif_outside, char* str);

char*
tbl_nat_rtif_outside_key_value_dump(tbl_nat_rtif_outside_t *p_nat_rtif_outside, char* str);

char*
tbl_nat_rtif_outside_field_name_dump(tbl_nat_rtif_outside_t *p_nat_rtif_outside, int32 field_id, char* str);

char*
tbl_nat_rtif_outside_field_value_dump(tbl_nat_rtif_outside_t *p_nat_rtif_outside, int32 field_id, char* str);

char**
tbl_nat_rtif_outside_table_dump(tbl_nat_rtif_outside_t *p_nat_rtif_outside, char** str);

int32
tbl_nat_rtif_outside_table_parser(char** array, char* key_value, tbl_nat_rtif_outside_t *p_nat_rtif_outside);

int32
tbl_nat_rtif_outside_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_nat_rtif_outside_master_t*
tbl_nat_rtif_outside_get_master();

tbl_nat_rtif_outside_master_t*
tbl_nat_rtif_outside_init_nat_rtif_outside();

#endif /* !__TBL_NAT_RTIF_OUTSIDE_H__ */

