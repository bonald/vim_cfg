
#ifndef __TBL_STATIC_ROUTE_CFG_H__
#define __TBL_STATIC_ROUTE_CFG_H__

int32
tbl_static_route_cfg_add_static_route_cfg_sync(tbl_static_route_cfg_t *p_rst_cfg, uint32 sync);

int32
tbl_static_route_cfg_del_static_route_cfg_sync(tbl_static_route_cfg_key_t *p_rst_cfg_key, uint32 sync);

int32
tbl_static_route_cfg_set_static_route_cfg_field_sync(tbl_static_route_cfg_t *p_rst_cfg, tbl_static_route_cfg_field_id_t field_id, uint32 sync);

int32
tbl_static_route_cfg_add_static_route_cfg(tbl_static_route_cfg_t *p_rst_cfg);

int32
tbl_static_route_cfg_del_static_route_cfg(tbl_static_route_cfg_key_t *p_rst_cfg_key);

int32
tbl_static_route_cfg_set_static_route_cfg_field(tbl_static_route_cfg_t *p_rst_cfg, tbl_static_route_cfg_field_id_t field_id);

tbl_static_route_cfg_t*
tbl_static_route_cfg_get_static_route_cfg(tbl_static_route_cfg_key_t *p_rst_cfg_key);

char*
tbl_static_route_cfg_key_val2str(tbl_static_route_cfg_t *p_rst_cfg, char *str, uint32 str_len);

int32
tbl_static_route_cfg_key_str2val(char *str, tbl_static_route_cfg_t *p_rst_cfg);

int32
tbl_static_route_cfg_dump_one(tbl_static_route_cfg_t *p_rst_cfg, tbl_iter_args_t *pargs);

char*
tbl_static_route_cfg_key_name_dump(tbl_static_route_cfg_t *p_rst_cfg, char* str);

char*
tbl_static_route_cfg_key_value_dump(tbl_static_route_cfg_t *p_rst_cfg, char* str);

char*
tbl_static_route_cfg_field_name_dump(tbl_static_route_cfg_t *p_rst_cfg, int32 field_id, char* str);

char*
tbl_static_route_cfg_field_value_dump(tbl_static_route_cfg_t *p_rst_cfg, int32 field_id, char* str);

char**
tbl_static_route_cfg_table_dump(tbl_static_route_cfg_t *p_rst_cfg, char** str);

int32
tbl_static_route_cfg_table_parser(char** array, char* key_value, tbl_static_route_cfg_t *p_rst_cfg);

int32
tbl_static_route_cfg_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_static_route_cfg_master_t*
tbl_static_route_cfg_get_master();

tbl_static_route_cfg_master_t*
tbl_static_route_cfg_init_static_route_cfg();

#endif /* !__TBL_STATIC_ROUTE_CFG_H__ */

