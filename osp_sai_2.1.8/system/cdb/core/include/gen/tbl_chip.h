
#ifndef __TBL_CHIP_H__
#define __TBL_CHIP_H__

int32
tbl_chip_set_chip_field_sync(tbl_chip_t *p_chip, tbl_chip_field_id_t field_id, uint32 sync);

int32
tbl_chip_set_chip_field(tbl_chip_t *p_chip, tbl_chip_field_id_t field_id);

tbl_chip_t*
tbl_chip_get_chip();

char*
tbl_chip_key_val2str(tbl_chip_t *p_chip, char *str, uint32 str_len);

int32
tbl_chip_key_str2val(char *str, tbl_chip_t *p_chip);

int32
tbl_chip_dump_one(tbl_chip_t *p_chip, tbl_iter_args_t *pargs);

char*
tbl_chip_key_name_dump(tbl_chip_t *p_chip, char* str);

char*
tbl_chip_key_value_dump(tbl_chip_t *p_chip, char* str);

char*
tbl_chip_field_name_dump(tbl_chip_t *p_chip, int32 field_id, char* str);

char*
tbl_chip_field_value_dump(tbl_chip_t *p_chip, int32 field_id, char* str);

char**
tbl_chip_table_dump(tbl_chip_t *p_chip, char** str);

int32
tbl_chip_table_parser(char** array, char* key_value, tbl_chip_t *p_chip);

int32
tbl_chip_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_chip_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_chip_t*
tbl_chip_init_chip();

uint32
tbl_chip_get_type();

#endif /* !__TBL_CHIP_H__ */

