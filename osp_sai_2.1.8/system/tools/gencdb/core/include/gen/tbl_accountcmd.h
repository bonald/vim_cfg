
#ifndef __TBL_ACCOUNTCMD_H__
#define __TBL_ACCOUNTCMD_H__

int32
tbl_accountcmd_add_accountcmd_sync(tbl_accountcmd_t *p_accountcmd, uint32 sync);

int32
tbl_accountcmd_del_accountcmd_sync(tbl_accountcmd_key_t *p_accountcmd_key, uint32 sync);

int32
tbl_accountcmd_set_accountcmd_field_sync(tbl_accountcmd_t *p_accountcmd, tbl_accountcmd_field_id_t field_id, uint32 sync);

int32
tbl_accountcmd_add_accountcmd(tbl_accountcmd_t *p_accountcmd);

int32
tbl_accountcmd_del_accountcmd(tbl_accountcmd_key_t *p_accountcmd_key);

int32
tbl_accountcmd_set_accountcmd_field(tbl_accountcmd_t *p_accountcmd, tbl_accountcmd_field_id_t field_id);

tbl_accountcmd_t*
tbl_accountcmd_get_accountcmd(tbl_accountcmd_key_t *p_accountcmd_key);

char*
tbl_accountcmd_key_val2str(tbl_accountcmd_t *p_accountcmd, char *str, uint32 str_len);

int32
tbl_accountcmd_key_str2val(char *str, tbl_accountcmd_t *p_accountcmd);

int32
tbl_accountcmd_dump_one(tbl_accountcmd_t *p_accountcmd, tbl_iter_args_t *pargs);

char*
tbl_accountcmd_key_name_dump(tbl_accountcmd_t *p_accountcmd, char* str);

char*
tbl_accountcmd_key_value_dump(tbl_accountcmd_t *p_accountcmd, char* str);

char*
tbl_accountcmd_field_name_dump(tbl_accountcmd_t *p_accountcmd, int32 field_id, char* str);

char*
tbl_accountcmd_field_value_dump(tbl_accountcmd_t *p_accountcmd, int32 field_id, char* str);

char**
tbl_accountcmd_table_dump(tbl_accountcmd_t *p_accountcmd, char** str);

int32
tbl_accountcmd_table_parser(char** array, char* key_value, tbl_accountcmd_t *p_accountcmd);

int32
tbl_accountcmd_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_accountcmd_master_t*
tbl_accountcmd_get_master();

tbl_accountcmd_master_t*
tbl_accountcmd_init_accountcmd();

#endif /* !__TBL_ACCOUNTCMD_H__ */

