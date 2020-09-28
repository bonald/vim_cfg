
#ifndef __TBL_DHCSRVGRP_H__
#define __TBL_DHCSRVGRP_H__

int32
tbl_dhcsrvgrp_add_dhcsrvgrp_sync(tbl_dhcsrvgrp_t *p_dhcsrvgrp, uint32 sync);

int32
tbl_dhcsrvgrp_del_dhcsrvgrp_sync(tbl_dhcsrvgrp_key_t *p_dhcsrvgrp_key, uint32 sync);

int32
tbl_dhcsrvgrp_set_dhcsrvgrp_field_sync(tbl_dhcsrvgrp_t *p_dhcsrvgrp, tbl_dhcsrvgrp_field_id_t field_id, uint32 sync);

int32
tbl_dhcsrvgrp_add_dhcsrvgrp(tbl_dhcsrvgrp_t *p_dhcsrvgrp);

int32
tbl_dhcsrvgrp_del_dhcsrvgrp(tbl_dhcsrvgrp_key_t *p_dhcsrvgrp_key);

int32
tbl_dhcsrvgrp_set_dhcsrvgrp_field(tbl_dhcsrvgrp_t *p_dhcsrvgrp, tbl_dhcsrvgrp_field_id_t field_id);

tbl_dhcsrvgrp_t*
tbl_dhcsrvgrp_get_dhcsrvgrp(tbl_dhcsrvgrp_key_t *p_dhcsrvgrp_key);

char*
tbl_dhcsrvgrp_key_val2str(tbl_dhcsrvgrp_t *p_dhcsrvgrp, char *str, uint32 str_len);

int32
tbl_dhcsrvgrp_key_str2val(char *str, tbl_dhcsrvgrp_t *p_dhcsrvgrp);

int32
tbl_dhcsrvgrp_dump_one(tbl_dhcsrvgrp_t *p_dhcsrvgrp, tbl_iter_args_t *pargs);

char*
tbl_dhcsrvgrp_key_name_dump(tbl_dhcsrvgrp_t *p_dhcsrvgrp, char* str);

char*
tbl_dhcsrvgrp_key_value_dump(tbl_dhcsrvgrp_t *p_dhcsrvgrp, char* str);

char*
tbl_dhcsrvgrp_field_name_dump(tbl_dhcsrvgrp_t *p_dhcsrvgrp, int32 field_id, char* str);

char*
tbl_dhcsrvgrp_field_value_dump(tbl_dhcsrvgrp_t *p_dhcsrvgrp, int32 field_id, char* str);

char**
tbl_dhcsrvgrp_table_dump(tbl_dhcsrvgrp_t *p_dhcsrvgrp, char** str);

int32
tbl_dhcsrvgrp_table_parser(char** array, char* key_value, tbl_dhcsrvgrp_t *p_dhcsrvgrp);

int32
tbl_dhcsrvgrp_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_dhcsrvgrp_master_t*
tbl_dhcsrvgrp_get_master();

tbl_dhcsrvgrp_master_t*
tbl_dhcsrvgrp_init_dhcsrvgrp();

#endif /* !__TBL_DHCSRVGRP_H__ */

