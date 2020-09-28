
#ifndef __TBL_BOOTIMAGE_H__
#define __TBL_BOOTIMAGE_H__

int32
tbl_bootimage_set_bootimage_field_sync(tbl_bootimage_t *p_boot, tbl_bootimage_field_id_t field_id, uint32 sync);

int32
tbl_bootimage_set_bootimage_field(tbl_bootimage_t *p_boot, tbl_bootimage_field_id_t field_id);

tbl_bootimage_t*
tbl_bootimage_get_bootimage();

char*
tbl_bootimage_key_val2str(tbl_bootimage_t *p_boot, char *str, uint32 str_len);

int32
tbl_bootimage_key_str2val(char *str, tbl_bootimage_t *p_boot);

int32
tbl_bootimage_dump_one(tbl_bootimage_t *p_boot, tbl_iter_args_t *pargs);

char*
tbl_bootimage_key_name_dump(tbl_bootimage_t *p_boot, char* str);

char*
tbl_bootimage_key_value_dump(tbl_bootimage_t *p_boot, char* str);

char*
tbl_bootimage_field_name_dump(tbl_bootimage_t *p_boot, int32 field_id, char* str);

char*
tbl_bootimage_field_value_dump(tbl_bootimage_t *p_boot, int32 field_id, char* str);

char**
tbl_bootimage_table_dump(tbl_bootimage_t *p_boot, char** str);

int32
tbl_bootimage_table_parser(char** array, char* key_value, tbl_bootimage_t *p_boot);

int32
tbl_bootimage_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_bootimage_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_bootimage_t*
tbl_bootimage_init_bootimage();

#endif /* !__TBL_BOOTIMAGE_H__ */

