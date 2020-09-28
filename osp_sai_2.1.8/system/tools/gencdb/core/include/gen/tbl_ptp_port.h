
#ifndef __TBL_PTP_PORT_H__
#define __TBL_PTP_PORT_H__

int32
tbl_ptp_port_add_ptp_port_sync(tbl_ptp_port_t *p_port, uint32 sync);

int32
tbl_ptp_port_del_ptp_port_sync(tbl_ptp_port_key_t *p_port_key, uint32 sync);

int32
tbl_ptp_port_set_ptp_port_field_sync(tbl_ptp_port_t *p_port, tbl_ptp_port_field_id_t field_id, uint32 sync);

int32
tbl_ptp_port_add_ptp_port(tbl_ptp_port_t *p_port);

int32
tbl_ptp_port_del_ptp_port(tbl_ptp_port_key_t *p_port_key);

int32
tbl_ptp_port_set_ptp_port_field(tbl_ptp_port_t *p_port, tbl_ptp_port_field_id_t field_id);

tbl_ptp_port_t*
tbl_ptp_port_get_ptp_port(tbl_ptp_port_key_t *p_port_key);

char*
tbl_ptp_port_key_val2str(tbl_ptp_port_t *p_port, char *str, uint32 str_len);

int32
tbl_ptp_port_key_str2val(char *str, tbl_ptp_port_t *p_port);

int32
tbl_ptp_port_dump_one(tbl_ptp_port_t *p_port, tbl_iter_args_t *pargs);

char*
tbl_ptp_port_key_name_dump(tbl_ptp_port_t *p_port, char* str);

char*
tbl_ptp_port_key_value_dump(tbl_ptp_port_t *p_port, char* str);

char*
tbl_ptp_port_field_name_dump(tbl_ptp_port_t *p_port, int32 field_id, char* str);

char*
tbl_ptp_port_field_value_dump(tbl_ptp_port_t *p_port, int32 field_id, char* str);

char**
tbl_ptp_port_table_dump(tbl_ptp_port_t *p_port, char** str);

int32
tbl_ptp_port_table_parser(char** array, char* key_value, tbl_ptp_port_t *p_port);

int32
tbl_ptp_port_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ptp_port_master_t*
tbl_ptp_port_get_master();

tbl_ptp_port_master_t*
tbl_ptp_port_init_ptp_port();

tbl_ptp_port_t*
tbl_ptp_port_get_ptp_port_by_name(char* ifname);
#endif /* !__TBL_PTP_PORT_H__ */

