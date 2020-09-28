
#ifndef __TBL_OAM_SESSION_UPLOAD_H__
#define __TBL_OAM_SESSION_UPLOAD_H__

int32
tbl_oam_session_upload_add_oam_session_upload_sync(tbl_oam_session_upload_t *p_oam_session_upload, uint32 sync);

int32
tbl_oam_session_upload_del_oam_session_upload_sync(tbl_oam_session_upload_key_t *p_oam_session_upload_key, uint32 sync);

int32
tbl_oam_session_upload_set_oam_session_upload_field_sync(tbl_oam_session_upload_t *p_oam_session_upload, tbl_oam_session_upload_field_id_t field_id, uint32 sync);

int32
tbl_oam_session_upload_add_oam_session_upload(tbl_oam_session_upload_t *p_oam_session_upload);

int32
tbl_oam_session_upload_del_oam_session_upload(tbl_oam_session_upload_key_t *p_oam_session_upload_key);

int32
tbl_oam_session_upload_set_oam_session_upload_field(tbl_oam_session_upload_t *p_oam_session_upload, tbl_oam_session_upload_field_id_t field_id);

tbl_oam_session_upload_t*
tbl_oam_session_upload_get_oam_session_upload(tbl_oam_session_upload_key_t *p_oam_session_upload_key);

char*
tbl_oam_session_upload_key_val2str(tbl_oam_session_upload_t *p_oam_session_upload, char *str, uint32 str_len);

int32
tbl_oam_session_upload_key_str2val(char *str, tbl_oam_session_upload_t *p_oam_session_upload);

int32
tbl_oam_session_upload_dump_one(tbl_oam_session_upload_t *p_oam_session_upload, tbl_iter_args_t *pargs);

char*
tbl_oam_session_upload_key_name_dump(tbl_oam_session_upload_t *p_oam_session_upload, char* str);

char*
tbl_oam_session_upload_key_value_dump(tbl_oam_session_upload_t *p_oam_session_upload, char* str);

char*
tbl_oam_session_upload_field_name_dump(tbl_oam_session_upload_t *p_oam_session_upload, int32 field_id, char* str);

char*
tbl_oam_session_upload_field_value_dump(tbl_oam_session_upload_t *p_oam_session_upload, int32 field_id, char* str);

char**
tbl_oam_session_upload_table_dump(tbl_oam_session_upload_t *p_oam_session_upload, char** str);

int32
tbl_oam_session_upload_table_parser(char** array, char* key_value, tbl_oam_session_upload_t *p_oam_session_upload);

int32
tbl_oam_session_upload_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_oam_session_upload_master_t*
tbl_oam_session_upload_get_master();

tbl_oam_session_upload_master_t*
tbl_oam_session_upload_init_oam_session_upload();

#endif /* !__TBL_OAM_SESSION_UPLOAD_H__ */

