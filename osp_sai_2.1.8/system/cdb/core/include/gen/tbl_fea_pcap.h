
#ifndef __TBL_FEA_PCAP_H__
#define __TBL_FEA_PCAP_H__

int32
tbl_fea_pcap_set_fea_pcap_field_sync(tbl_fea_pcap_t *p_pcap, tbl_fea_pcap_field_id_t field_id, uint32 sync);

int32
tbl_fea_pcap_set_fea_pcap_field(tbl_fea_pcap_t *p_pcap, tbl_fea_pcap_field_id_t field_id);

tbl_fea_pcap_t*
tbl_fea_pcap_get_fea_pcap();

char*
tbl_fea_pcap_key_val2str(tbl_fea_pcap_t *p_pcap, char *str, uint32 str_len);

int32
tbl_fea_pcap_key_str2val(char *str, tbl_fea_pcap_t *p_pcap);

int32
tbl_fea_pcap_dump_one(tbl_fea_pcap_t *p_pcap, tbl_iter_args_t *pargs);

char*
tbl_fea_pcap_key_name_dump(tbl_fea_pcap_t *p_pcap, char* str);

char*
tbl_fea_pcap_key_value_dump(tbl_fea_pcap_t *p_pcap, char* str);

char*
tbl_fea_pcap_field_name_dump(tbl_fea_pcap_t *p_pcap, int32 field_id, char* str);

char*
tbl_fea_pcap_field_value_dump(tbl_fea_pcap_t *p_pcap, int32 field_id, char* str);

char**
tbl_fea_pcap_table_dump(tbl_fea_pcap_t *p_pcap, char** str);

int32
tbl_fea_pcap_table_parser(char** array, char* key_value, tbl_fea_pcap_t *p_pcap);

int32
tbl_fea_pcap_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

int32
tbl_fea_pcap_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_fea_pcap_t*
tbl_fea_pcap_init_fea_pcap();

#endif /* !__TBL_FEA_PCAP_H__ */

