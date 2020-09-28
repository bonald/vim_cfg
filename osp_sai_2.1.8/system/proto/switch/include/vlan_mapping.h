#ifndef __VLAN_MAPPING_H__
#define __VLAN_MAPPING_H__

typedef int32 (*VLAN_MAPPING_TABLE_ITER_APPLIED_PORTS_FUNC)(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs);

#define CFG_VLAN_MAPPING_ENTRY_EXIST_RET(VID) \
do \
{ \
    if(GLB_VLAN_NONE == VID) \
    { \
        CFG_CONFLICT_RET("VLAN mapping entry exist for untagged raw vlan"); \
    } \
    else if(GLB_VLAN_ALL == VID) \
    { \
        CFG_CONFLICT_RET("VLAN mapping entry exist for out-of-range raw vlan"); \
    } \
    else \
    { \
        CFG_CONFLICT_RET("VLAN mapping entry exist for raw vlan %d", VID); \
    } \
} \
while (0)

#define CFG_VLAN_MAPPING_ENTRY_NOTEXIST_RET(VID) \
do \
{ \
    if(GLB_VLAN_NONE == VID) \
    { \
        CFG_CONFLICT_RET("VLAN mapping entry not exist for untagged raw vlan"); \
    } \
    else if(GLB_VLAN_ALL == VID) \
    { \
        CFG_CONFLICT_RET("VLAN mapping entry not exist for out-of-range raw vlan"); \
    } \
    else \
    { \
        CFG_CONFLICT_RET("VLAN mapping entry not exist for raw vlan %d", VID); \
    } \
} \
while (0)

#define VLANMAP_TABLE_SHOW_ALL_TITLE  "Table Name       EVC Name         Mapped VLAN  Raw VLAN\n"
#define VLANMAP_TABLE_SHOW_ALL_TITLE_FTM   \
    "----------------+----------------+------------+------------\n"
#define VLANMAP_TABLE_SHOW_EVC_MAPPED_VLAN_FMT  "%-16s %-12d "
#define VLANMAP_TABLE_SHOW_APPLIED_IF_HDR_STR  "Table Name       Interface Name   Type\n"
#define VLANMAP_TABLE_SHOW_APPLIED_IF_HDR_FMT  "----------------+----------------+----------------\n"

#define VLANMAP_TABLE_SHOW_TABLE_NAME_FTM(FIRST, FP, VMT) \
do \
{ \
    if(FIRST) \
    { \
        sal_fprintf(FP, "%-16s ", VMT->key.name); \
    } \
    else \
    { \
        sal_fprintf(FP, "%-16s ", ""); \
    } \
} \
while (0)

#define VLANMAP_TABLE_SHOW_RAW_VID_FTM(FP, VID) \
do \
{ \
    if(GLB_VLAN_NONE == VID) \
    { \
        sal_fprintf(FP, "untagged\n"); \
    } \
    else if(GLB_VLAN_ALL == VID) \
    { \
        sal_fprintf(FP, "out-of-range\n"); \
    } \
    else \
    { \
        sal_fprintf(FP, "%d\n", VID); \
    } \
} \
while (0)


int32
vlan_mapping_start(switch_master_t *master);

int32
vlan_mapping_stop(switch_master_t *master);

int32
vlanmap_api_check_mapped_vlan_exist(char *vm_table, uint16 vid);

int32
vlanmap_cmd_process_vlanmap_table(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
vlanmap_cmd_process_vlanmap_entry(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
vlanmap_cmd_process_egress_vlanmap_entry(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
vlanmap_cmd_process_vlan_group(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
vlanmap_cmd_process_evc_entry(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
vlanmap_api_process_pdu(tbl_interface_t *p_db_if, vid_t *vid);

int32
vlanmap_api_iterate_applied_ports(tbl_vlan_mapping_table_t *p_db_vmt, 
       VLAN_MAPPING_TABLE_ITER_APPLIED_PORTS_FUNC fn, tbl_iter_args_t *pargs);

int32
vlanmap_api_if_enable_vlantrans(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs);

int32
vlanmap_api_if_delete_vmt(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs);

int32
vlanmap_api_isin_vlan_group(uint32 group_id, uint32 vid);

int32
vlanmap_api_get_valid_group(tbl_vlan_group_t *p_db_vlan_group);

int32
vlanmap_api_mod_applied_entry_one(tbl_interface_t *p_db_if, tbl_iter_args_t *pargs);

int32
vlanmap_api_mod_applied_entry(tbl_vlan_mapping_table_t *p_db_vmt, uint32 entry_num, uint32 is_add);

int32
vlanmap_api_vmt_check_vlantrans(tbl_vlan_mapping_table_t *p_db_vmt, cfg_cmd_para_t *para);

int32
vlanmap_api_mapped_double_check(tbl_vlan_mapping_table_t *p_db_vmt, tbl_vlan_mapping_entry_t *p_vm_entry, 
        tbl_vlan_group_t *p_db_vlan_group, uint32 group_svid, cfg_cmd_para_t *para);

int32
vlanmap_api_mapped_evc_check(tbl_vlan_mapping_table_t *p_db_vmt, 
        uint16 raw_svid, cfg_cmd_para_t *para);

int32
vlanmap_api_vmt_add_vm_entry(tbl_vlan_mapping_table_t *p_db_vmt, 
           tbl_vlan_mapping_entry_t *p_vm_entry, cfg_cmd_para_t *para);

int32
vlanmap_api_vmt_del_vm_entry(tbl_vlan_mapping_table_t *p_db_vmt, 
           tbl_vlan_mapping_entry_t *p_vm_entry, cfg_cmd_para_t *para);

int32
vlanmap_api_vmt_add_egress_vm_entry(tbl_vlan_mapping_table_t *p_db_vmt, 
           tbl_egress_mapping_entry_t *p_vm_entry, cfg_cmd_para_t *para);

int32
vlanmap_api_vmt_del_egress_vm_entry(tbl_vlan_mapping_table_t *p_db_vmt, 
           tbl_egress_mapping_entry_t *p_vm_entry, cfg_cmd_para_t *para);
                                    
int32
vlanmap_api_vmt_add_vlan_group(tbl_vlan_mapping_table_t *p_db_vmt, 
           tbl_vlan_mapping_table_t *p_vm_table, cfg_cmd_para_t *para);

int32
vlanmap_api_vmt_del_vlan_group(tbl_vlan_mapping_table_t *p_db_vmt, cfg_cmd_para_t *para);

int32
vlanmap_api_del_vm_table(tbl_vlan_mapping_table_t *p_db_vmt, cfg_cmd_para_t *para);

int32
vlanmap_api_add_vlan_group(tbl_vlan_group_key_t *p_vlan_group_key, char *vlan_list, cfg_cmd_para_t *para);

char *
vlanmap_api_show_vlan_group(uint32 group_id, char *group_str);

int32
vlanmap_api_show_vmt(tbl_vlan_mapping_table_t *p_db_vmt, char *evc_name, uint32 raw_vid, 
        FILE *fp, cfg_cmd_para_t *para);

int32
vlanmap_api_show_vmt_applied_if(tbl_vlan_mapping_table_t *p_db_vmt, FILE *fp, cfg_cmd_para_t *para);

int32
vlanmap_api_show_evc_one(tbl_evc_entry_t *p_db_evc, FILE *fp, cfg_cmd_para_t *para);

int32
vlanmap_cmd_show_vmt(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
vlanmap_cmd_show_evc(char **argv, int32 argc, cfg_cmd_para_t *para);

#endif /* __VLAN_MAPPING_H__ */

