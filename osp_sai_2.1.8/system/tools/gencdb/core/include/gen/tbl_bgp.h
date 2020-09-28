
#ifndef __TBL_BGP_H__
#define __TBL_BGP_H__

int32
tbl_bgp_add_bgp_sync(tbl_bgp_t *p_bgp, uint32 sync);

int32
tbl_bgp_del_bgp_sync(tbl_bgp_key_t *p_bgp_key, uint32 sync);

int32
tbl_bgp_set_bgp_field_sync(tbl_bgp_t *p_bgp, tbl_bgp_field_id_t field_id, uint32 sync);

int32
tbl_bgp_add_bgp(tbl_bgp_t *p_bgp);

int32
tbl_bgp_del_bgp(tbl_bgp_key_t *p_bgp_key);

int32
tbl_bgp_set_bgp_field(tbl_bgp_t *p_bgp, tbl_bgp_field_id_t field_id);

tbl_bgp_t*
tbl_bgp_get_bgp(tbl_bgp_key_t *p_bgp_key);

char*
tbl_bgp_key_val2str(tbl_bgp_t *p_bgp, char *str, uint32 str_len);

int32
tbl_bgp_key_str2val(char *str, tbl_bgp_t *p_bgp);

int32
tbl_bgp_dump_one(tbl_bgp_t *p_bgp, tbl_iter_args_t *pargs);

char*
tbl_bgp_key_name_dump(tbl_bgp_t *p_bgp, char* str);

char*
tbl_bgp_key_value_dump(tbl_bgp_t *p_bgp, char* str);

char*
tbl_bgp_field_name_dump(tbl_bgp_t *p_bgp, int32 field_id, char* str);

char*
tbl_bgp_field_value_dump(tbl_bgp_t *p_bgp, int32 field_id, char* str);

char**
tbl_bgp_table_dump(tbl_bgp_t *p_bgp, char** str);

int32
tbl_bgp_table_parser(char** array, char* key_value, tbl_bgp_t *p_bgp);

int32
tbl_bgp_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_bgp_master_t*
tbl_bgp_get_master();

tbl_bgp_master_t*
tbl_bgp_init_bgp();

#endif /* !__TBL_BGP_H__ */

