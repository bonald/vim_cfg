
#ifndef __TBL_NAT_RTIF_INSIDE_H__
#define __TBL_NAT_RTIF_INSIDE_H__

int32
tbl_nat_rtif_inside_add_nat_rtif_inside_sync(tbl_nat_rtif_inside_t *p_nat_rtif_inside, uint32 sync);

int32
tbl_nat_rtif_inside_del_nat_rtif_inside_sync(tbl_nat_rtif_inside_key_t *p_nat_rtif_inside_key, uint32 sync);

int32
tbl_nat_rtif_inside_set_nat_rtif_inside_field_sync(tbl_nat_rtif_inside_t *p_nat_rtif_inside, tbl_nat_rtif_inside_field_id_t field_id, uint32 sync);

int32
tbl_nat_rtif_inside_add_nat_rtif_inside(tbl_nat_rtif_inside_t *p_nat_rtif_inside);

int32
tbl_nat_rtif_inside_del_nat_rtif_inside(tbl_nat_rtif_inside_key_t *p_nat_rtif_inside_key);

int32
tbl_nat_rtif_inside_set_nat_rtif_inside_field(tbl_nat_rtif_inside_t *p_nat_rtif_inside, tbl_nat_rtif_inside_field_id_t field_id);

tbl_nat_rtif_inside_t*
tbl_nat_rtif_inside_get_nat_rtif_inside(tbl_nat_rtif_inside_key_t *p_nat_rtif_inside_key);

char*
tbl_nat_rtif_inside_key_val2str(tbl_nat_rtif_inside_t *p_nat_rtif_inside, char *str, uint32 str_len);

int32
tbl_nat_rtif_inside_key_str2val(char *str, tbl_nat_rtif_inside_t *p_nat_rtif_inside);

int32
tbl_nat_rtif_inside_dump_one(tbl_nat_rtif_inside_t *p_nat_rtif_inside, tbl_iter_args_t *pargs);

char*
tbl_nat_rtif_inside_key_name_dump(tbl_nat_rtif_inside_t *p_nat_rtif_inside, char* str);

char*
tbl_nat_rtif_inside_key_value_dump(tbl_nat_rtif_inside_t *p_nat_rtif_inside, char* str);

char*
tbl_nat_rtif_inside_field_name_dump(tbl_nat_rtif_inside_t *p_nat_rtif_inside, int32 field_id, char* str);

char*
tbl_nat_rtif_inside_field_value_dump(tbl_nat_rtif_inside_t *p_nat_rtif_inside, int32 field_id, char* str);

char**
tbl_nat_rtif_inside_table_dump(tbl_nat_rtif_inside_t *p_nat_rtif_inside, char** str);

int32
tbl_nat_rtif_inside_table_parser(char** array, char* key_value, tbl_nat_rtif_inside_t *p_nat_rtif_inside);

int32
tbl_nat_rtif_inside_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_nat_rtif_inside_master_t*
tbl_nat_rtif_inside_get_master();

tbl_nat_rtif_inside_master_t*
tbl_nat_rtif_inside_init_nat_rtif_inside();

#endif /* !__TBL_NAT_RTIF_INSIDE_H__ */

