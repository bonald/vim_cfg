#ifndef __CFM_API_H__
#define __CFM_API_H__

int32
cfm_md_check_valid(char **argv, int32 argc, cfg_cmd_para_t *para, uint16 *p_level, uint8 *p_name_type, tbl_cfm_md_t   *p_md);

int32
cfm_ma_check_valid(char **argv, int32 argc, cfg_cmd_para_t *para, tbl_cfm_md_t  *p_md, uint16 *p_vlan, uint8 *p_name_type, uint8 *p_index);

int32
cfm_lmep_check_valid(char **argv, int32 argc, cfg_cmd_para_t *para, uint16 *p_vlan_id, uint8 *p_interval, uint8  *p_direction, uint16  *p_mep_id);

int32
cfm_rmep_check_valid(char **argv, int32 argc, cfg_cmd_para_t *para, uint16 *p_vlan_id, uint16  *p_rmep_id, uint8 *mac);

char * cfm_get_cc_interval_desc(uint8 interval);

char *cfm_format_mac_address (uint8 *mac_addr);

int32
cfm_set_service_cci_enable(tbl_cfm_ma_t   *p_ma, uint8   cci_enabled);

int32
cfm_set_service_ccm_vlan_cos(tbl_cfm_ma_t   *p_ma, uint8   ccm_vlan_cos);

int32
cfm_api_delete_meps_for_ma(tbl_cfm_ma_t   *p_ma);

int32
cfm_api_delete_ma_for_md(tbl_cfm_md_t   *p_md);

int32
cfm_api_clear_cfm_errors_for_md(tbl_cfm_md_t   *p_md);

int32
cfm_api_delete_ma_for_vlan(vid_t   vid);

int32
cfm_api_get_cfm_state_for_g8032(tbl_cfm_lmep_t        *p_lmep);

#endif /* !__CFMD_H__ */
