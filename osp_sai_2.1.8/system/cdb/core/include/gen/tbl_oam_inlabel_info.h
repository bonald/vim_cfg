
#ifndef __TBL_OAM_INLABEL_INFO_H__
#define __TBL_OAM_INLABEL_INFO_H__

int32
tbl_oam_inlabel_info_add_oam_inlabel_info_sync(tbl_oam_inlabel_info_t *p_oam_inlabel_info, uint32 sync);

int32
tbl_oam_inlabel_info_del_oam_inlabel_info_sync(tbl_oam_inlabel_info_key_t *p_oam_inlabel_info_key, uint32 sync);

int32
tbl_oam_inlabel_info_set_oam_inlabel_info_field_sync(tbl_oam_inlabel_info_t *p_oam_inlabel_info, tbl_oam_inlabel_info_field_id_t field_id, uint32 sync);

int32
tbl_oam_inlabel_info_add_oam_inlabel_info(tbl_oam_inlabel_info_t *p_oam_inlabel_info);

int32
tbl_oam_inlabel_info_del_oam_inlabel_info(tbl_oam_inlabel_info_key_t *p_oam_inlabel_info_key);

int32
tbl_oam_inlabel_info_set_oam_inlabel_info_field(tbl_oam_inlabel_info_t *p_oam_inlabel_info, tbl_oam_inlabel_info_field_id_t field_id);

tbl_oam_inlabel_info_t*
tbl_oam_inlabel_info_get_oam_inlabel_info(tbl_oam_inlabel_info_key_t *p_oam_inlabel_info_key);

char*
tbl_oam_inlabel_info_key_val2str(tbl_oam_inlabel_info_t *p_oam_inlabel_info, char *str, uint32 str_len);

int32
tbl_oam_inlabel_info_key_str2val(char *str, tbl_oam_inlabel_info_t *p_oam_inlabel_info);

int32
tbl_oam_inlabel_info_dump_one(tbl_oam_inlabel_info_t *p_oam_inlabel_info, tbl_iter_args_t *pargs);

char*
tbl_oam_inlabel_info_key_name_dump(tbl_oam_inlabel_info_t *p_oam_inlabel_info, char* str);

char*
tbl_oam_inlabel_info_key_value_dump(tbl_oam_inlabel_info_t *p_oam_inlabel_info, char* str);

char*
tbl_oam_inlabel_info_field_name_dump(tbl_oam_inlabel_info_t *p_oam_inlabel_info, int32 field_id, char* str);

char*
tbl_oam_inlabel_info_field_value_dump(tbl_oam_inlabel_info_t *p_oam_inlabel_info, int32 field_id, char* str);

char**
tbl_oam_inlabel_info_table_dump(tbl_oam_inlabel_info_t *p_oam_inlabel_info, char** str);

int32
tbl_oam_inlabel_info_table_parser(char** array, char* key_value, tbl_oam_inlabel_info_t *p_oam_inlabel_info);

int32
tbl_oam_inlabel_info_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_oam_inlabel_info_master_t*
tbl_oam_inlabel_info_get_master();

tbl_oam_inlabel_info_master_t*
tbl_oam_inlabel_info_init_oam_inlabel_info();

#endif /* !__TBL_OAM_INLABEL_INFO_H__ */

