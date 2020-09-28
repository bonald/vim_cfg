
#ifndef __TBL_SNMP_COMMUNITY_H__
#define __TBL_SNMP_COMMUNITY_H__

int32
tbl_snmp_community_add_snmp_community_sync(tbl_snmp_community_t *p_snmp_community, uint32 sync);

int32
tbl_snmp_community_del_snmp_community_sync(tbl_snmp_community_key_t *p_snmp_community_key, uint32 sync);

int32
tbl_snmp_community_set_snmp_community_field_sync(tbl_snmp_community_t *p_snmp_community, tbl_snmp_community_field_id_t field_id, uint32 sync);

int32
tbl_snmp_community_add_snmp_community(tbl_snmp_community_t *p_snmp_community);

int32
tbl_snmp_community_del_snmp_community(tbl_snmp_community_key_t *p_snmp_community_key);

int32
tbl_snmp_community_set_snmp_community_field(tbl_snmp_community_t *p_snmp_community, tbl_snmp_community_field_id_t field_id);

tbl_snmp_community_t*
tbl_snmp_community_get_snmp_community(tbl_snmp_community_key_t *p_snmp_community_key);

char*
tbl_snmp_community_key_val2str(tbl_snmp_community_t *p_snmp_community, char *str, uint32 str_len);

int32
tbl_snmp_community_key_str2val(char *str, tbl_snmp_community_t *p_snmp_community);

int32
tbl_snmp_community_dump_one(tbl_snmp_community_t *p_snmp_community, tbl_iter_args_t *pargs);

char*
tbl_snmp_community_key_name_dump(tbl_snmp_community_t *p_snmp_community, char* str);

char*
tbl_snmp_community_key_value_dump(tbl_snmp_community_t *p_snmp_community, char* str);

char*
tbl_snmp_community_field_name_dump(tbl_snmp_community_t *p_snmp_community, int32 field_id, char* str);

char*
tbl_snmp_community_field_value_dump(tbl_snmp_community_t *p_snmp_community, int32 field_id, char* str);

char**
tbl_snmp_community_table_dump(tbl_snmp_community_t *p_snmp_community, char** str);

int32
tbl_snmp_community_table_parser(char** array, char* key_value, tbl_snmp_community_t *p_snmp_community);

int32
tbl_snmp_community_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs);

tbl_snmp_community_master_t*
tbl_snmp_community_get_master();

tbl_snmp_community_master_t*
tbl_snmp_community_init_snmp_community();

#endif /* !__TBL_SNMP_COMMUNITY_H__ */

