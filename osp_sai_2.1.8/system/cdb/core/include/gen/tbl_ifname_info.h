
#ifndef __TBL_IFNAME_INFO_H__
#define __TBL_IFNAME_INFO_H__

int32
tbl_ifname_info_set_ifname_info_field_sync(tbl_ifname_info_t *p_ifname_info, tbl_ifname_info_field_id_t field_id, uint32 sync);

int32
tbl_ifname_info_set_ifname_info_field(tbl_ifname_info_t *p_ifname_info, tbl_ifname_info_field_id_t field_id);

tbl_ifname_info_t*
tbl_ifname_info_get_ifname_info();

char*
tbl_ifname_info_key_val2str(tbl_ifname_info_t *p_ifname_info, char *str, uint32 str_len);

int32
tbl_ifname_info_key_str2val(char *str, tbl_ifname_info_t *p_ifname_info);

int32
tbl_ifname_info_dump_one(tbl_ifname_info_t *p_ifname_info, tbl_iter_args_t *pargs);

char*
tbl_ifname_info_key_name_dump(tbl_ifname_info_t *p_ifname_info, char* str);

char*
tbl_ifname_info_key_value_dump(tbl_ifname_info_t *p_ifname_info, char* str);

char*
tbl_ifname_info_field_name_dump(tbl_ifname_info_t *p_ifname_info, int32 field_id, char* str);

char*
tbl_ifname_info_field_value_dump(tbl_ifname_info_t *p_ifname_info, int32 field_id, char* str);

char**
tbl_ifname_info_table_dump(tbl_ifname_info_t *p_ifname_info, char** str);

int32
tbl_ifname_info_table_parser(char** array, char* key_value, tbl_ifname_info_t *p_ifname_info);

int32
tbl_ifname_info_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_ifname_info_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ifname_info_t*
tbl_ifname_info_init_ifname_info();

#endif /* !__TBL_IFNAME_INFO_H__ */

