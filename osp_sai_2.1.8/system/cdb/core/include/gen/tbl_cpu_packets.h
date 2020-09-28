
#ifndef __TBL_CPU_PACKETS_H__
#define __TBL_CPU_PACKETS_H__

int32
tbl_cpu_packets_set_cpu_packets_field_sync(tbl_cpu_packets_t *p_cpu_packets, tbl_cpu_packets_field_id_t field_id, uint32 sync);

int32
tbl_cpu_packets_set_cpu_packets_field(tbl_cpu_packets_t *p_cpu_packets, tbl_cpu_packets_field_id_t field_id);

tbl_cpu_packets_t*
tbl_cpu_packets_get_cpu_packets();

char*
tbl_cpu_packets_key_val2str(tbl_cpu_packets_t *p_cpu_packets, char *str, uint32 str_len);

int32
tbl_cpu_packets_key_str2val(char *str, tbl_cpu_packets_t *p_cpu_packets);

int32
tbl_cpu_packets_dump_one(tbl_cpu_packets_t *p_cpu_packets, tbl_iter_args_t *pargs);

char*
tbl_cpu_packets_key_name_dump(tbl_cpu_packets_t *p_cpu_packets, char* str);

char*
tbl_cpu_packets_key_value_dump(tbl_cpu_packets_t *p_cpu_packets, char* str);

char*
tbl_cpu_packets_field_name_dump(tbl_cpu_packets_t *p_cpu_packets, int32 field_id, char* str);

char*
tbl_cpu_packets_field_value_dump(tbl_cpu_packets_t *p_cpu_packets, int32 field_id, char* str);

char**
tbl_cpu_packets_table_dump(tbl_cpu_packets_t *p_cpu_packets, char** str);

int32
tbl_cpu_packets_table_parser(char** array, char* key_value, tbl_cpu_packets_t *p_cpu_packets);

int32
tbl_cpu_packets_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_cpu_packets_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_cpu_packets_t*
tbl_cpu_packets_init_cpu_packets();

#endif /* !__TBL_CPU_PACKETS_H__ */

