
#ifndef __TBL_TAP_DPI_GLOBAL_H__
#define __TBL_TAP_DPI_GLOBAL_H__

int32
tbl_tap_dpi_global_set_tap_dpi_global_field_sync(tbl_tap_dpi_global_t *p_tap_dpi_global, tbl_tap_dpi_global_field_id_t field_id, uint32 sync);

int32
tbl_tap_dpi_global_set_tap_dpi_global_field(tbl_tap_dpi_global_t *p_tap_dpi_global, tbl_tap_dpi_global_field_id_t field_id);

tbl_tap_dpi_global_t*
tbl_tap_dpi_global_get_tap_dpi_global();

char*
tbl_tap_dpi_global_key_val2str(tbl_tap_dpi_global_t *p_tap_dpi_global, char *str, uint32 str_len);

int32
tbl_tap_dpi_global_key_str2val(char *str, tbl_tap_dpi_global_t *p_tap_dpi_global);

int32
tbl_tap_dpi_global_dump_one(tbl_tap_dpi_global_t *p_tap_dpi_global, tbl_iter_args_t *pargs);

char*
tbl_tap_dpi_global_key_name_dump(tbl_tap_dpi_global_t *p_tap_dpi_global, char* str);

char*
tbl_tap_dpi_global_key_value_dump(tbl_tap_dpi_global_t *p_tap_dpi_global, char* str);

char*
tbl_tap_dpi_global_field_name_dump(tbl_tap_dpi_global_t *p_tap_dpi_global, int32 field_id, char* str);

char*
tbl_tap_dpi_global_field_value_dump(tbl_tap_dpi_global_t *p_tap_dpi_global, int32 field_id, char* str);

char**
tbl_tap_dpi_global_table_dump(tbl_tap_dpi_global_t *p_tap_dpi_global, char** str);

int32
tbl_tap_dpi_global_table_parser(char** array, char* key_value, tbl_tap_dpi_global_t *p_tap_dpi_global);

int32
tbl_tap_dpi_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_tap_dpi_global_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_tap_dpi_global_t*
tbl_tap_dpi_global_init_tap_dpi_global();

#endif /* !__TBL_TAP_DPI_GLOBAL_H__ */

