
#ifndef __TBL_CFM_MD_H__
#define __TBL_CFM_MD_H__

#include "gen/ds_cfm_error.h"

int32
tbl_cfm_md_add_cfm_md_sync(tbl_cfm_md_t *p_md, uint32 sync);

int32
tbl_cfm_md_del_cfm_md_sync(tbl_cfm_md_key_t *p_md_key, uint32 sync);

int32
tbl_cfm_md_set_cfm_md_field_sync(tbl_cfm_md_t *p_md, tbl_cfm_md_field_id_t field_id, uint32 sync);

int32
tbl_cfm_md_add_cfm_md(tbl_cfm_md_t *p_md);

int32
tbl_cfm_md_del_cfm_md(tbl_cfm_md_key_t *p_md_key);

int32
tbl_cfm_md_set_cfm_md_field(tbl_cfm_md_t *p_md, tbl_cfm_md_field_id_t field_id);

tbl_cfm_md_t*
tbl_cfm_md_get_cfm_md(tbl_cfm_md_key_t *p_md_key);

char*
tbl_cfm_md_key_val2str(tbl_cfm_md_t *p_md, char *str, uint32 str_len);

int32
tbl_cfm_md_key_str2val(char *str, tbl_cfm_md_t *p_md);

int32
tbl_cfm_md_dump_one(tbl_cfm_md_t *p_md, tbl_iter_args_t *pargs);

char*
tbl_cfm_md_key_name_dump(tbl_cfm_md_t *p_md, char* str);

char*
tbl_cfm_md_key_value_dump(tbl_cfm_md_t *p_md, char* str);

char*
tbl_cfm_md_field_name_dump(tbl_cfm_md_t *p_md, int32 field_id, char* str);

char*
tbl_cfm_md_field_value_dump(tbl_cfm_md_t *p_md, int32 field_id, char* str);

char**
tbl_cfm_md_table_dump(tbl_cfm_md_t *p_md, char** str);

int32
tbl_cfm_md_table_parser(char** array, char* key_value, tbl_cfm_md_t *p_md);

int32
tbl_cfm_md_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_cfm_md_master_t*
tbl_cfm_md_get_master();

tbl_cfm_md_master_t*
tbl_cfm_md_init_cfm_md();

#endif /* !__TBL_CFM_MD_H__ */

