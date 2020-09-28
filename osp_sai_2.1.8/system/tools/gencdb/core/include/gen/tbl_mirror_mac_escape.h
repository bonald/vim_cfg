
#ifndef __TBL_MIRROR_MAC_ESCAPE_H__
#define __TBL_MIRROR_MAC_ESCAPE_H__

int32
tbl_mirror_mac_escape_add_mirror_mac_escape_sync(tbl_mirror_mac_escape_t *p_mirror_mac_escape, uint32 sync);

int32
tbl_mirror_mac_escape_del_mirror_mac_escape_sync(tbl_mirror_mac_escape_key_t *p_mirror_mac_escape_key, uint32 sync);

int32
tbl_mirror_mac_escape_set_mirror_mac_escape_field_sync(tbl_mirror_mac_escape_t *p_mirror_mac_escape, tbl_mirror_mac_escape_field_id_t field_id, uint32 sync);

int32
tbl_mirror_mac_escape_add_mirror_mac_escape(tbl_mirror_mac_escape_t *p_mirror_mac_escape);

int32
tbl_mirror_mac_escape_del_mirror_mac_escape(tbl_mirror_mac_escape_key_t *p_mirror_mac_escape_key);

int32
tbl_mirror_mac_escape_set_mirror_mac_escape_field(tbl_mirror_mac_escape_t *p_mirror_mac_escape, tbl_mirror_mac_escape_field_id_t field_id);

tbl_mirror_mac_escape_t*
tbl_mirror_mac_escape_get_mirror_mac_escape(tbl_mirror_mac_escape_key_t *p_mirror_mac_escape_key);

char*
tbl_mirror_mac_escape_key_val2str(tbl_mirror_mac_escape_t *p_mirror_mac_escape, char *str, uint32 str_len);

int32
tbl_mirror_mac_escape_key_str2val(char *str, tbl_mirror_mac_escape_t *p_mirror_mac_escape);

int32
tbl_mirror_mac_escape_dump_one(tbl_mirror_mac_escape_t *p_mirror_mac_escape, tbl_iter_args_t *pargs);

char*
tbl_mirror_mac_escape_key_name_dump(tbl_mirror_mac_escape_t *p_mirror_mac_escape, char* str);

char*
tbl_mirror_mac_escape_key_value_dump(tbl_mirror_mac_escape_t *p_mirror_mac_escape, char* str);

char*
tbl_mirror_mac_escape_field_name_dump(tbl_mirror_mac_escape_t *p_mirror_mac_escape, int32 field_id, char* str);

char*
tbl_mirror_mac_escape_field_value_dump(tbl_mirror_mac_escape_t *p_mirror_mac_escape, int32 field_id, char* str);

char**
tbl_mirror_mac_escape_table_dump(tbl_mirror_mac_escape_t *p_mirror_mac_escape, char** str);

int32
tbl_mirror_mac_escape_table_parser(char** array, char* key_value, tbl_mirror_mac_escape_t *p_mirror_mac_escape);

int32
tbl_mirror_mac_escape_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_mirror_mac_escape_master_t*
tbl_mirror_mac_escape_get_master();

tbl_mirror_mac_escape_master_t*
tbl_mirror_mac_escape_init_mirror_mac_escape();

#endif /* !__TBL_MIRROR_MAC_ESCAPE_H__ */

