
#ifndef __TBL_OSPF_AREA_RANGE_H__
#define __TBL_OSPF_AREA_RANGE_H__

int32
tbl_ospf_area_range_add_ospf_area_range_sync(tbl_ospf_area_range_t *p_ospf_area_range, uint32 sync);

int32
tbl_ospf_area_range_del_ospf_area_range_sync(tbl_ospf_area_range_key_t *p_ospf_area_range_key, uint32 sync);

int32
tbl_ospf_area_range_set_ospf_area_range_field_sync(tbl_ospf_area_range_t *p_ospf_area_range, tbl_ospf_area_range_field_id_t field_id, uint32 sync);

int32
tbl_ospf_area_range_add_ospf_area_range(tbl_ospf_area_range_t *p_ospf_area_range);

int32
tbl_ospf_area_range_del_ospf_area_range(tbl_ospf_area_range_key_t *p_ospf_area_range_key);

int32
tbl_ospf_area_range_set_ospf_area_range_field(tbl_ospf_area_range_t *p_ospf_area_range, tbl_ospf_area_range_field_id_t field_id);

tbl_ospf_area_range_t*
tbl_ospf_area_range_get_ospf_area_range(tbl_ospf_area_range_key_t *p_ospf_area_range_key);

char*
tbl_ospf_area_range_key_val2str(tbl_ospf_area_range_t *p_ospf_area_range, char *str, uint32 str_len);

int32
tbl_ospf_area_range_key_str2val(char *str, tbl_ospf_area_range_t *p_ospf_area_range);

int32
tbl_ospf_area_range_dump_one(tbl_ospf_area_range_t *p_ospf_area_range, tbl_iter_args_t *pargs);

char*
tbl_ospf_area_range_key_name_dump(tbl_ospf_area_range_t *p_ospf_area_range, char* str);

char*
tbl_ospf_area_range_key_value_dump(tbl_ospf_area_range_t *p_ospf_area_range, char* str);

char*
tbl_ospf_area_range_field_name_dump(tbl_ospf_area_range_t *p_ospf_area_range, int32 field_id, char* str);

char*
tbl_ospf_area_range_field_value_dump(tbl_ospf_area_range_t *p_ospf_area_range, int32 field_id, char* str);

char**
tbl_ospf_area_range_table_dump(tbl_ospf_area_range_t *p_ospf_area_range, char** str);

int32
tbl_ospf_area_range_table_parser(char** array, char* key_value, tbl_ospf_area_range_t *p_ospf_area_range);

int32
tbl_ospf_area_range_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ospf_area_range_master_t*
tbl_ospf_area_range_get_master();

tbl_ospf_area_range_master_t*
tbl_ospf_area_range_init_ospf_area_range();

#endif /* !__TBL_OSPF_AREA_RANGE_H__ */

