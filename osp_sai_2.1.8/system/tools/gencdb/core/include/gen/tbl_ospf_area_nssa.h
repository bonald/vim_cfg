
#ifndef __TBL_OSPF_AREA_NSSA_H__
#define __TBL_OSPF_AREA_NSSA_H__

int32
tbl_ospf_area_nssa_add_ospf_area_nssa_sync(tbl_ospf_area_nssa_t *p_ospf_area_nssa, uint32 sync);

int32
tbl_ospf_area_nssa_del_ospf_area_nssa_sync(tbl_ospf_area_nssa_key_t *p_ospf_area_nssa_key, uint32 sync);

int32
tbl_ospf_area_nssa_set_ospf_area_nssa_field_sync(tbl_ospf_area_nssa_t *p_ospf_area_nssa, tbl_ospf_area_nssa_field_id_t field_id, uint32 sync);

int32
tbl_ospf_area_nssa_add_ospf_area_nssa(tbl_ospf_area_nssa_t *p_ospf_area_nssa);

int32
tbl_ospf_area_nssa_del_ospf_area_nssa(tbl_ospf_area_nssa_key_t *p_ospf_area_nssa_key);

int32
tbl_ospf_area_nssa_set_ospf_area_nssa_field(tbl_ospf_area_nssa_t *p_ospf_area_nssa, tbl_ospf_area_nssa_field_id_t field_id);

tbl_ospf_area_nssa_t*
tbl_ospf_area_nssa_get_ospf_area_nssa(tbl_ospf_area_nssa_key_t *p_ospf_area_nssa_key);

char*
tbl_ospf_area_nssa_key_val2str(tbl_ospf_area_nssa_t *p_ospf_area_nssa, char *str, uint32 str_len);

int32
tbl_ospf_area_nssa_key_str2val(char *str, tbl_ospf_area_nssa_t *p_ospf_area_nssa);

int32
tbl_ospf_area_nssa_dump_one(tbl_ospf_area_nssa_t *p_ospf_area_nssa, tbl_iter_args_t *pargs);

char*
tbl_ospf_area_nssa_key_name_dump(tbl_ospf_area_nssa_t *p_ospf_area_nssa, char* str);

char*
tbl_ospf_area_nssa_key_value_dump(tbl_ospf_area_nssa_t *p_ospf_area_nssa, char* str);

char*
tbl_ospf_area_nssa_field_name_dump(tbl_ospf_area_nssa_t *p_ospf_area_nssa, int32 field_id, char* str);

char*
tbl_ospf_area_nssa_field_value_dump(tbl_ospf_area_nssa_t *p_ospf_area_nssa, int32 field_id, char* str);

char**
tbl_ospf_area_nssa_table_dump(tbl_ospf_area_nssa_t *p_ospf_area_nssa, char** str);

int32
tbl_ospf_area_nssa_table_parser(char** array, char* key_value, tbl_ospf_area_nssa_t *p_ospf_area_nssa);

int32
tbl_ospf_area_nssa_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ospf_area_nssa_master_t*
tbl_ospf_area_nssa_get_master();

tbl_ospf_area_nssa_master_t*
tbl_ospf_area_nssa_init_ospf_area_nssa();

#endif /* !__TBL_OSPF_AREA_NSSA_H__ */

