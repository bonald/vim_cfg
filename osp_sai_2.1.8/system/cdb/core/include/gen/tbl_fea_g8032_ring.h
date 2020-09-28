
#ifndef __TBL_FEA_G8032_RING_H__
#define __TBL_FEA_G8032_RING_H__

int32
tbl_fea_g8032_ring_add_fea_g8032_ring_sync(tbl_fea_g8032_ring_t *p_fea_g8032_ring, uint32 sync);

int32
tbl_fea_g8032_ring_del_fea_g8032_ring_sync(tbl_fea_g8032_ring_key_t *p_fea_g8032_ring_key, uint32 sync);

int32
tbl_fea_g8032_ring_set_fea_g8032_ring_field_sync(tbl_fea_g8032_ring_t *p_fea_g8032_ring, tbl_fea_g8032_ring_field_id_t field_id, uint32 sync);

int32
tbl_fea_g8032_ring_add_fea_g8032_ring(tbl_fea_g8032_ring_t *p_fea_g8032_ring);

int32
tbl_fea_g8032_ring_del_fea_g8032_ring(tbl_fea_g8032_ring_key_t *p_fea_g8032_ring_key);

int32
tbl_fea_g8032_ring_set_fea_g8032_ring_field(tbl_fea_g8032_ring_t *p_fea_g8032_ring, tbl_fea_g8032_ring_field_id_t field_id);

tbl_fea_g8032_ring_t*
tbl_fea_g8032_ring_get_fea_g8032_ring(tbl_fea_g8032_ring_key_t *p_fea_g8032_ring_key);

char*
tbl_fea_g8032_ring_key_val2str(tbl_fea_g8032_ring_t *p_fea_g8032_ring, char *str, uint32 str_len);

int32
tbl_fea_g8032_ring_key_str2val(char *str, tbl_fea_g8032_ring_t *p_fea_g8032_ring);

int32
tbl_fea_g8032_ring_dump_one(tbl_fea_g8032_ring_t *p_fea_g8032_ring, tbl_iter_args_t *pargs);

char*
tbl_fea_g8032_ring_key_name_dump(tbl_fea_g8032_ring_t *p_fea_g8032_ring, char* str);

char*
tbl_fea_g8032_ring_key_value_dump(tbl_fea_g8032_ring_t *p_fea_g8032_ring, char* str);

char*
tbl_fea_g8032_ring_field_name_dump(tbl_fea_g8032_ring_t *p_fea_g8032_ring, int32 field_id, char* str);

char*
tbl_fea_g8032_ring_field_value_dump(tbl_fea_g8032_ring_t *p_fea_g8032_ring, int32 field_id, char* str);

char**
tbl_fea_g8032_ring_table_dump(tbl_fea_g8032_ring_t *p_fea_g8032_ring, char** str);

int32
tbl_fea_g8032_ring_table_parser(char** array, char* key_value, tbl_fea_g8032_ring_t *p_fea_g8032_ring);

int32
tbl_fea_g8032_ring_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fea_g8032_ring_master_t*
tbl_fea_g8032_ring_get_master();

tbl_fea_g8032_ring_master_t*
tbl_fea_g8032_ring_init_fea_g8032_ring();

#endif /* !__TBL_FEA_G8032_RING_H__ */

