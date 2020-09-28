
#ifndef __TBL_LSP_P_H__
#define __TBL_LSP_P_H__

int32
tbl_lsp_p_add_lsp_p_sync(tbl_lsp_p_t *p_lsp_p, uint32 sync);

int32
tbl_lsp_p_del_lsp_p_sync(tbl_lsp_p_key_t *p_lsp_p_key, uint32 sync);

int32
tbl_lsp_p_set_lsp_p_field_sync(tbl_lsp_p_t *p_lsp_p, tbl_lsp_p_field_id_t field_id, uint32 sync);

int32
tbl_lsp_p_add_lsp_p(tbl_lsp_p_t *p_lsp_p);

int32
tbl_lsp_p_del_lsp_p(tbl_lsp_p_key_t *p_lsp_p_key);

int32
tbl_lsp_p_set_lsp_p_field(tbl_lsp_p_t *p_lsp_p, tbl_lsp_p_field_id_t field_id);

tbl_lsp_p_t*
tbl_lsp_p_get_lsp_p(tbl_lsp_p_key_t *p_lsp_p_key);

char*
tbl_lsp_p_key_val2str(tbl_lsp_p_t *p_lsp_p, char *str, uint32 str_len);

int32
tbl_lsp_p_key_str2val(char *str, tbl_lsp_p_t *p_lsp_p);

int32
tbl_lsp_p_dump_one(tbl_lsp_p_t *p_lsp_p, tbl_iter_args_t *pargs);

char*
tbl_lsp_p_key_name_dump(tbl_lsp_p_t *p_lsp_p, char* str);

char*
tbl_lsp_p_key_value_dump(tbl_lsp_p_t *p_lsp_p, char* str);

char*
tbl_lsp_p_field_name_dump(tbl_lsp_p_t *p_lsp_p, int32 field_id, char* str);

char*
tbl_lsp_p_field_value_dump(tbl_lsp_p_t *p_lsp_p, int32 field_id, char* str);

char**
tbl_lsp_p_table_dump(tbl_lsp_p_t *p_lsp_p, char** str);

int32
tbl_lsp_p_table_parser(char** array, char* key_value, tbl_lsp_p_t *p_lsp_p);

int32
tbl_lsp_p_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_lsp_p_master_t*
tbl_lsp_p_get_master();

tbl_lsp_p_master_t*
tbl_lsp_p_init_lsp_p();

#endif /* !__TBL_LSP_P_H__ */

