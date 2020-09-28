
#ifndef __TBL_OSPF_AREA_VLINK_H__
#define __TBL_OSPF_AREA_VLINK_H__

int32
tbl_ospf_area_vlink_add_ospf_area_vlink_sync(tbl_ospf_area_vlink_t *p_ospf_area_vlink, uint32 sync);

int32
tbl_ospf_area_vlink_del_ospf_area_vlink_sync(tbl_ospf_area_vlink_key_t *p_ospf_area_vlink_key, uint32 sync);

int32
tbl_ospf_area_vlink_set_ospf_area_vlink_field_sync(tbl_ospf_area_vlink_t *p_ospf_area_vlink, tbl_ospf_area_vlink_field_id_t field_id, uint32 sync);

int32
tbl_ospf_area_vlink_add_ospf_area_vlink(tbl_ospf_area_vlink_t *p_ospf_area_vlink);

int32
tbl_ospf_area_vlink_del_ospf_area_vlink(tbl_ospf_area_vlink_key_t *p_ospf_area_vlink_key);

int32
tbl_ospf_area_vlink_set_ospf_area_vlink_field(tbl_ospf_area_vlink_t *p_ospf_area_vlink, tbl_ospf_area_vlink_field_id_t field_id);

tbl_ospf_area_vlink_t*
tbl_ospf_area_vlink_get_ospf_area_vlink(tbl_ospf_area_vlink_key_t *p_ospf_area_vlink_key);

char*
tbl_ospf_area_vlink_key_val2str(tbl_ospf_area_vlink_t *p_ospf_area_vlink, char *str, uint32 str_len);

int32
tbl_ospf_area_vlink_key_str2val(char *str, tbl_ospf_area_vlink_t *p_ospf_area_vlink);

int32
tbl_ospf_area_vlink_dump_one(tbl_ospf_area_vlink_t *p_ospf_area_vlink, tbl_iter_args_t *pargs);

char*
tbl_ospf_area_vlink_key_name_dump(tbl_ospf_area_vlink_t *p_ospf_area_vlink, char* str);

char*
tbl_ospf_area_vlink_key_value_dump(tbl_ospf_area_vlink_t *p_ospf_area_vlink, char* str);

char*
tbl_ospf_area_vlink_field_name_dump(tbl_ospf_area_vlink_t *p_ospf_area_vlink, int32 field_id, char* str);

char*
tbl_ospf_area_vlink_field_value_dump(tbl_ospf_area_vlink_t *p_ospf_area_vlink, int32 field_id, char* str);

char**
tbl_ospf_area_vlink_table_dump(tbl_ospf_area_vlink_t *p_ospf_area_vlink, char** str);

int32
tbl_ospf_area_vlink_table_parser(char** array, char* key_value, tbl_ospf_area_vlink_t *p_ospf_area_vlink);

int32
tbl_ospf_area_vlink_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ospf_area_vlink_master_t*
tbl_ospf_area_vlink_get_master();

tbl_ospf_area_vlink_master_t*
tbl_ospf_area_vlink_init_ospf_area_vlink();

#endif /* !__TBL_OSPF_AREA_VLINK_H__ */

