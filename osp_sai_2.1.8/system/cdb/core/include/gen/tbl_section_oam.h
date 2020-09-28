
#ifndef __TBL_SECTION_OAM_H__
#define __TBL_SECTION_OAM_H__

int32
tbl_section_oam_add_section_oam_sync(tbl_section_oam_t *p_section_oam, uint32 sync);

int32
tbl_section_oam_del_section_oam_sync(tbl_section_oam_key_t *p_section_oam_key, uint32 sync);

int32
tbl_section_oam_set_section_oam_field_sync(tbl_section_oam_t *p_section_oam, tbl_section_oam_field_id_t field_id, uint32 sync);

int32
tbl_section_oam_add_section_oam(tbl_section_oam_t *p_section_oam);

int32
tbl_section_oam_del_section_oam(tbl_section_oam_key_t *p_section_oam_key);

int32
tbl_section_oam_set_section_oam_field(tbl_section_oam_t *p_section_oam, tbl_section_oam_field_id_t field_id);

tbl_section_oam_t*
tbl_section_oam_get_section_oam(tbl_section_oam_key_t *p_section_oam_key);

char*
tbl_section_oam_key_val2str(tbl_section_oam_t *p_section_oam, char *str, uint32 str_len);

int32
tbl_section_oam_key_str2val(char *str, tbl_section_oam_t *p_section_oam);

int32
tbl_section_oam_dump_one(tbl_section_oam_t *p_section_oam, tbl_iter_args_t *pargs);

char*
tbl_section_oam_key_name_dump(tbl_section_oam_t *p_section_oam, char* str);

char*
tbl_section_oam_key_value_dump(tbl_section_oam_t *p_section_oam, char* str);

char*
tbl_section_oam_field_name_dump(tbl_section_oam_t *p_section_oam, int32 field_id, char* str);

char*
tbl_section_oam_field_value_dump(tbl_section_oam_t *p_section_oam, int32 field_id, char* str);

char**
tbl_section_oam_table_dump(tbl_section_oam_t *p_section_oam, char** str);

int32
tbl_section_oam_table_parser(char** array, char* key_value, tbl_section_oam_t *p_section_oam);

int32
tbl_section_oam_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_section_oam_master_t*
tbl_section_oam_get_master();

tbl_section_oam_master_t*
tbl_section_oam_init_section_oam();

#endif /* !__TBL_SECTION_OAM_H__ */

