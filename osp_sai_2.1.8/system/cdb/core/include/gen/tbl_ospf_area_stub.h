
#ifndef __TBL_OSPF_AREA_STUB_H__
#define __TBL_OSPF_AREA_STUB_H__

int32
tbl_ospf_area_stub_add_ospf_area_stub_sync(tbl_ospf_area_stub_t *p_ospf_area_stub, uint32 sync);

int32
tbl_ospf_area_stub_del_ospf_area_stub_sync(tbl_ospf_area_stub_key_t *p_ospf_area_stub_key, uint32 sync);

int32
tbl_ospf_area_stub_set_ospf_area_stub_field_sync(tbl_ospf_area_stub_t *p_ospf_area_stub, tbl_ospf_area_stub_field_id_t field_id, uint32 sync);

int32
tbl_ospf_area_stub_add_ospf_area_stub(tbl_ospf_area_stub_t *p_ospf_area_stub);

int32
tbl_ospf_area_stub_del_ospf_area_stub(tbl_ospf_area_stub_key_t *p_ospf_area_stub_key);

int32
tbl_ospf_area_stub_set_ospf_area_stub_field(tbl_ospf_area_stub_t *p_ospf_area_stub, tbl_ospf_area_stub_field_id_t field_id);

tbl_ospf_area_stub_t*
tbl_ospf_area_stub_get_ospf_area_stub(tbl_ospf_area_stub_key_t *p_ospf_area_stub_key);

char*
tbl_ospf_area_stub_key_val2str(tbl_ospf_area_stub_t *p_ospf_area_stub, char *str, uint32 str_len);

int32
tbl_ospf_area_stub_key_str2val(char *str, tbl_ospf_area_stub_t *p_ospf_area_stub);

int32
tbl_ospf_area_stub_dump_one(tbl_ospf_area_stub_t *p_ospf_area_stub, tbl_iter_args_t *pargs);

char*
tbl_ospf_area_stub_key_name_dump(tbl_ospf_area_stub_t *p_ospf_area_stub, char* str);

char*
tbl_ospf_area_stub_key_value_dump(tbl_ospf_area_stub_t *p_ospf_area_stub, char* str);

char*
tbl_ospf_area_stub_field_name_dump(tbl_ospf_area_stub_t *p_ospf_area_stub, int32 field_id, char* str);

char*
tbl_ospf_area_stub_field_value_dump(tbl_ospf_area_stub_t *p_ospf_area_stub, int32 field_id, char* str);

char**
tbl_ospf_area_stub_table_dump(tbl_ospf_area_stub_t *p_ospf_area_stub, char** str);

int32
tbl_ospf_area_stub_table_parser(char** array, char* key_value, tbl_ospf_area_stub_t *p_ospf_area_stub);

int32
tbl_ospf_area_stub_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ospf_area_stub_master_t*
tbl_ospf_area_stub_get_master();

tbl_ospf_area_stub_master_t*
tbl_ospf_area_stub_init_ospf_area_stub();

#endif /* !__TBL_OSPF_AREA_STUB_H__ */

