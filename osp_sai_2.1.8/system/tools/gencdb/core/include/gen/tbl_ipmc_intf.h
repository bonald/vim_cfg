
#ifndef __TBL_IPMC_INTF_H__
#define __TBL_IPMC_INTF_H__

int32
tbl_ipmc_intf_add_ipmc_intf_sync(tbl_ipmc_intf_t *p_pim_intf, uint32 sync);

int32
tbl_ipmc_intf_del_ipmc_intf_sync(tbl_ipmc_intf_key_t *p_pim_intf_key, uint32 sync);

int32
tbl_ipmc_intf_set_ipmc_intf_field_sync(tbl_ipmc_intf_t *p_pim_intf, tbl_ipmc_intf_field_id_t field_id, uint32 sync);

int32
tbl_ipmc_intf_add_ipmc_intf(tbl_ipmc_intf_t *p_pim_intf);

int32
tbl_ipmc_intf_del_ipmc_intf(tbl_ipmc_intf_key_t *p_pim_intf_key);

int32
tbl_ipmc_intf_set_ipmc_intf_field(tbl_ipmc_intf_t *p_pim_intf, tbl_ipmc_intf_field_id_t field_id);

tbl_ipmc_intf_t*
tbl_ipmc_intf_get_ipmc_intf(tbl_ipmc_intf_key_t *p_pim_intf_key);

char*
tbl_ipmc_intf_key_val2str(tbl_ipmc_intf_t *p_pim_intf, char *str, uint32 str_len);

int32
tbl_ipmc_intf_key_str2val(char *str, tbl_ipmc_intf_t *p_pim_intf);

int32
tbl_ipmc_intf_dump_one(tbl_ipmc_intf_t *p_pim_intf, tbl_iter_args_t *pargs);

char*
tbl_ipmc_intf_key_name_dump(tbl_ipmc_intf_t *p_pim_intf, char* str);

char*
tbl_ipmc_intf_key_value_dump(tbl_ipmc_intf_t *p_pim_intf, char* str);

char*
tbl_ipmc_intf_field_name_dump(tbl_ipmc_intf_t *p_pim_intf, int32 field_id, char* str);

char*
tbl_ipmc_intf_field_value_dump(tbl_ipmc_intf_t *p_pim_intf, int32 field_id, char* str);

char**
tbl_ipmc_intf_table_dump(tbl_ipmc_intf_t *p_pim_intf, char** str);

int32
tbl_ipmc_intf_table_parser(char** array, char* key_value, tbl_ipmc_intf_t *p_pim_intf);

int32
tbl_ipmc_intf_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_ipmc_intf_master_t*
tbl_ipmc_intf_get_master();

tbl_ipmc_intf_master_t*
tbl_ipmc_intf_init_ipmc_intf();

#endif /* !__TBL_IPMC_INTF_H__ */

