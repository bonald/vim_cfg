
#ifndef __TBL_LSP_PE_H__
#define __TBL_LSP_PE_H__

int32
tbl_lsp_pe_add_lsp_pe_sync(tbl_lsp_pe_t *p_lsp_pe, uint32 sync);

int32
tbl_lsp_pe_del_lsp_pe_sync(tbl_lsp_pe_key_t *p_lsp_pe_key, uint32 sync);

int32
tbl_lsp_pe_set_lsp_pe_field_sync(tbl_lsp_pe_t *p_lsp_pe, tbl_lsp_pe_field_id_t field_id, uint32 sync);

int32
tbl_lsp_pe_add_lsp_pe(tbl_lsp_pe_t *p_lsp_pe);

int32
tbl_lsp_pe_del_lsp_pe(tbl_lsp_pe_key_t *p_lsp_pe_key);

int32
tbl_lsp_pe_set_lsp_pe_field(tbl_lsp_pe_t *p_lsp_pe, tbl_lsp_pe_field_id_t field_id);

tbl_lsp_pe_t*
tbl_lsp_pe_get_lsp_pe(tbl_lsp_pe_key_t *p_lsp_pe_key);

char*
tbl_lsp_pe_key_val2str(tbl_lsp_pe_t *p_lsp_pe, char *str, uint32 str_len);

int32
tbl_lsp_pe_key_str2val(char *str, tbl_lsp_pe_t *p_lsp_pe);

int32
tbl_lsp_pe_dump_one(tbl_lsp_pe_t *p_lsp_pe, tbl_iter_args_t *pargs);

char*
tbl_lsp_pe_key_name_dump(tbl_lsp_pe_t *p_lsp_pe, char* str);

char*
tbl_lsp_pe_key_value_dump(tbl_lsp_pe_t *p_lsp_pe, char* str);

char*
tbl_lsp_pe_field_name_dump(tbl_lsp_pe_t *p_lsp_pe, int32 field_id, char* str);

char*
tbl_lsp_pe_field_value_dump(tbl_lsp_pe_t *p_lsp_pe, int32 field_id, char* str);

char**
tbl_lsp_pe_table_dump(tbl_lsp_pe_t *p_lsp_pe, char** str);

int32
tbl_lsp_pe_table_parser(char** array, char* key_value, tbl_lsp_pe_t *p_lsp_pe);

int32
tbl_lsp_pe_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_lsp_pe_master_t*
tbl_lsp_pe_get_master();

tbl_lsp_pe_master_t*
tbl_lsp_pe_init_lsp_pe();

#endif /* !__TBL_LSP_PE_H__ */

