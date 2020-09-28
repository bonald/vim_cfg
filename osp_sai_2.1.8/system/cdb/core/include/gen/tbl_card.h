
#ifndef __TBL_CARD_H__
#define __TBL_CARD_H__

int32
tbl_card_add_card_sync(tbl_card_t *p_card, uint32 sync);

int32
tbl_card_del_card_sync(tbl_card_key_t *p_card_key, uint32 sync);

int32
tbl_card_set_card_field_sync(tbl_card_t *p_card, tbl_card_field_id_t field_id, uint32 sync);

int32
tbl_card_add_card(tbl_card_t *p_card);

int32
tbl_card_del_card(tbl_card_key_t *p_card_key);

int32
tbl_card_set_card_field(tbl_card_t *p_card, tbl_card_field_id_t field_id);

tbl_card_t*
tbl_card_get_card(tbl_card_key_t *p_card_key);

char*
tbl_card_key_val2str(tbl_card_t *p_card, char *str, uint32 str_len);

int32
tbl_card_key_str2val(char *str, tbl_card_t *p_card);

int32
tbl_card_dump_one(tbl_card_t *p_card, tbl_iter_args_t *pargs);

char*
tbl_card_key_name_dump(tbl_card_t *p_card, char* str);

char*
tbl_card_key_value_dump(tbl_card_t *p_card, char* str);

char*
tbl_card_field_name_dump(tbl_card_t *p_card, int32 field_id, char* str);

char*
tbl_card_field_value_dump(tbl_card_t *p_card, int32 field_id, char* str);

char**
tbl_card_table_dump(tbl_card_t *p_card, char** str);

int32
tbl_card_table_parser(char** array, char* key_value, tbl_card_t *p_card);

int32
tbl_card_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_card_master_t*
tbl_card_get_master();

tbl_card_master_t*
tbl_card_init_card();

tbl_card_t*
tbl_card_get_default_card();

#endif /* !__TBL_CARD_H__ */

