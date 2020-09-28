
#ifndef __TBL_ERPS_RING_H__
#define __TBL_ERPS_RING_H__

int32
tbl_erps_ring_add_erps_ring_sync(tbl_erps_ring_t *p_erps_ring, uint32 sync);

int32
tbl_erps_ring_del_erps_ring_sync(tbl_erps_ring_key_t *p_erps_ring_key, uint32 sync);

int32
tbl_erps_ring_set_erps_ring_field_sync(tbl_erps_ring_t *p_erps_ring, tbl_erps_ring_field_id_t field_id, uint32 sync);

int32
tbl_erps_ring_add_erps_ring(tbl_erps_ring_t *p_erps_ring);

int32
tbl_erps_ring_del_erps_ring(tbl_erps_ring_key_t *p_erps_ring_key);

int32
tbl_erps_ring_set_erps_ring_field(tbl_erps_ring_t *p_erps_ring, tbl_erps_ring_field_id_t field_id);

tbl_erps_ring_t*
tbl_erps_ring_get_erps_ring(tbl_erps_ring_key_t *p_erps_ring_key);

char*
tbl_erps_ring_key_val2str(tbl_erps_ring_t *p_erps_ring, char *str, uint32 str_len);

int32
tbl_erps_ring_key_str2val(char *str, tbl_erps_ring_t *p_erps_ring);

int32
tbl_erps_ring_dump_one(tbl_erps_ring_t *p_erps_ring, tbl_iter_args_t *pargs);

char*
tbl_erps_ring_key_name_dump(tbl_erps_ring_t *p_erps_ring, char* str);

char*
tbl_erps_ring_key_value_dump(tbl_erps_ring_t *p_erps_ring, char* str);

char*
tbl_erps_ring_field_name_dump(tbl_erps_ring_t *p_erps_ring, int32 field_id, char* str);

char*
tbl_erps_ring_field_value_dump(tbl_erps_ring_t *p_erps_ring, int32 field_id, char* str);

char**
tbl_erps_ring_table_dump(tbl_erps_ring_t *p_erps_ring, char** str);

int32
tbl_erps_ring_table_parser(char** array, char* key_value, tbl_erps_ring_t *p_erps_ring);

int32
tbl_erps_ring_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_erps_ring_master_t*
tbl_erps_ring_get_master();

tbl_erps_ring_master_t*
tbl_erps_ring_init_erps_ring();

#endif /* !__TBL_ERPS_RING_H__ */

