
#ifndef __TBL_QUAGGA_DEBUG_H__
#define __TBL_QUAGGA_DEBUG_H__

int32
tbl_quagga_debug_set_quagga_debug_field_sync(tbl_quagga_debug_t *p_dbg, tbl_quagga_debug_field_id_t field_id, uint32 sync);

int32
tbl_quagga_debug_set_quagga_debug_field(tbl_quagga_debug_t *p_dbg, tbl_quagga_debug_field_id_t field_id);

tbl_quagga_debug_t*
tbl_quagga_debug_get_quagga_debug();

char*
tbl_quagga_debug_key_val2str(tbl_quagga_debug_t *p_dbg, char *str, uint32 str_len);

int32
tbl_quagga_debug_key_str2val(char *str, tbl_quagga_debug_t *p_dbg);

int32
tbl_quagga_debug_dump_one(tbl_quagga_debug_t *p_dbg, tbl_iter_args_t *pargs);

char*
tbl_quagga_debug_key_name_dump(tbl_quagga_debug_t *p_dbg, char* str);

char*
tbl_quagga_debug_key_value_dump(tbl_quagga_debug_t *p_dbg, char* str);

char*
tbl_quagga_debug_field_name_dump(tbl_quagga_debug_t *p_dbg, int32 field_id, char* str);

char*
tbl_quagga_debug_field_value_dump(tbl_quagga_debug_t *p_dbg, int32 field_id, char* str);

char**
tbl_quagga_debug_table_dump(tbl_quagga_debug_t *p_dbg, char** str);

int32
tbl_quagga_debug_table_parser(char** array, char* key_value, tbl_quagga_debug_t *p_dbg);

int32
tbl_quagga_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_quagga_debug_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_quagga_debug_t*
tbl_quagga_debug_init_quagga_debug();

#endif /* !__TBL_QUAGGA_DEBUG_H__ */

