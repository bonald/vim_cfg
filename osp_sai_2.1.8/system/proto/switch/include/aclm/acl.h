#ifndef __ACL_H__
#define __ACL_H__

#define TIME_RANGE_STATUS_NONE      0
#define TIME_RANGE_STATUS_NOT_START 1
#define TIME_RANGE_STATUS_START     2
#define TIME_RANGE_STATUS_END       3

#define foreach_acl_config_start(KEY_NAME)                                              \
{                                                                                       \
    tbl_acl_config_master_t *p_acl_config_master = tbl_acl_config_get_master();         \
    ctclib_slistnode_t      *listnode = NULL;                                           \
    tbl_acl_config_t        *KEY_NAME = NULL;                                           \
                                                                                        \
    CTCLIB_SLIST_LOOP(p_acl_config_master->acl_config_list, KEY_NAME, listnode)         \
    {                                                                                   

#define foreach_acl_config_end                                                          \
    }                                                                                   \
} 

#define foreach_ace_config_start(KEY_NAME)                                              \
{                                                                                       \
    tbl_ace_config_master_t *p_ace_master = tbl_ace_config_get_master();                \
    ctclib_slistnode_t      *listnode = NULL;                                           \
    tbl_ace_config_t        *KEY_NAME = NULL;                                           \
                                                                                        \
    CTCLIB_SLIST_LOOP(p_ace_master->ace_config_list, KEY_NAME, listnode)                \
    {                                                                                   

#define foreach_ace_config_end                                                                 \
    }                                                                                   \
} 

#define MAKE_ACL_ID(seq_no, direct, ifindex, class_priority, group_priority, type) ((uint64)((((seq_no) << 16)&0xffff0000) | ((direct)<<15) | ((ifindex)&0x7fff)) + (((class_priority&0x3ff)<< 38) | (((group_priority&0x3f) << 32)) | ((type&0x3f)<<48)))
#define MAKE_ACL_PRIORITY(seq_no, class_priority, group_priority) (((seq_no)&0xffff) | ((class_priority&0x3ff)<< 22) | (((group_priority&0x3f) << 16)))

#define MAKE_TAP_ACL_PRIORITY(seq_no, class_priority, group_priority)((class_priority & 0x3ff)<< 22) | (((seq_no) & 0xffff) << 6) | (group_priority & 0x3f)

extern int32 acl_show_acl(tbl_acl_config_t *p_db_acl_config, uint32 type, cfg_cmd_para_t *para);

extern int32 time_range_show_info(tbl_time_range_t *p_db_time_range, cfg_cmd_para_t *para);

extern void acl_time_range_invalid(void *p_data);

extern void acl_time_range_enable_absolute(void *p_data);

extern void acl_time_range_enable_periodic(void *p_data);

extern int32 acl_cmd_process_time_range(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32 acl_attach_interface(char *acl_name, char *interface_name, uint32 type, bool direct_in);

extern int32 acl_detach_interface(char *acl_name, char *interface_name, uint32 type, bool direct_in);

extern int32 acl_config_add_ace(char *acl_name, uint32 *p_seq_no, bool deny, char **ace_para,char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32 acl_config_add_ace_tap(char *acl_name, uint32 *p_seq_no, bool deny, char **ace_para,char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32 acl_config_del_ace(char *acl_name, uint32 seqno,char **argv, int32 argc, uint32 snmp_set);

extern int32 acl_config_set_remark(char *acl_name, char **argv, int32 argc, char *remark);

extern int32 acl_config_add_acl(char *acl_name, char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32 acl_config_del_acl(char *acl_name, char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32 timerange_config_check_tr_exist(char *tr_name);

extern int32 acl_worm_filter_add(char *acl_worm_filter_name, uint32 istcp, uint32 dstport, uint32 isrange);

extern int32 acl_worm_filter_del(char *acl_worm_filter_name);


extern int32 ace_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32 acl_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32 acl_worm_filter_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32 acl_entry_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32 acl_udf_group_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32 acl_api_add_udf_entry(tbl_acl_udf_entry_t *p_udf_entry, cfg_cmd_para_t *para);

extern int32 acl_api_set_udf_offset(tbl_acl_udf_entry_t *udf_entry, tbl_acl_udf_entry_t *p_udf_entry, cfg_cmd_para_t *para);

extern int32 acl_api_set_udf_match(tbl_acl_udf_entry_t *p_udf_entry, char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32 acl_udf_entry_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32 acl_cmd_show_acl_udf(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32 acl_cmd_show_acl(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32 acl_cmd_show_timerange_info(char **argv, int32 argc, cfg_cmd_para_t *para);

extern int32 acl_start(switch_master_t *master);

extern int32
acl_prefix_check(char *acl_name, prefix_t *dest, prefix_t *src);

uint32
acl_stm_is_ipv6_profile();

int32
acl_entry_get_count_1(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs);

int32
acl_entry_get_count(tbl_acl_entry_t *p_acl_entry, tbl_iter_args_t *pargs);

int32
acl_entry_get_entry_dir_count_1(acl_entry_count_t *entry_count);

int32
acl_entry_get_entry_dir_count(uint32* cnt_ingress, uint32* cnt_egress, uint32* cnt_worm_filter);

#endif

