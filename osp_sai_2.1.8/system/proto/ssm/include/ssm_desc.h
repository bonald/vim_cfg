#ifndef __SSM_DESC_H__
#define __SSM_DESC_H__

char *
ssm_desc_clockid(char *clock_id_str, ssm_clock_id_t clockid);

char *
ssm_desc_enable(uint32 value);

char *
ssm_desc_quality(uint32 value, uint8 desc_mode);

char *
ssm_desc_select_interface(char *ifname_ext, uint32 is_next);

char *
ssm_desc_device_status(tbl_ssm_global_t* p_ssm_glb);

char *
ssm_desc_role(uint32 value);

char *
ssm_desc_dpll_selected_if_status(tbl_ssm_global_t* p_ssm_glb);

char *
ssm_desc_dpll_status(uint32 value);

char *
ssm_desc_if_pdu(uint32 value);

#endif /* !__SSM_DESC_H__ */
