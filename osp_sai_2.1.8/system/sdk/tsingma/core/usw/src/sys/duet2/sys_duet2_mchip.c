
#ifdef DUET2

#include "ctc_error.h"
#include "sys_usw_mchip.h"
#include "sys_usw_ipuc.h"
#include "sys_duet2_ipuc_tcam.h"
#include "sys_duet2_calpm.h"
#include "sys_usw_mac.h"
#include "sys_usw_datapath.h"
#include "sys_usw_ftm.h"
#include "sys_duet2_mac.h"
#include "sys_duet2_datapath.h"
#include "sys_duet2_mcu.h"
#include "sys_duet2_peri.h"

extern int32 sys_duet2_capability_init(uint8 lchip);
extern int32 sys_duet2_chip_set_mem_bist(uint8 lchip, void* p_val);

extern int32
sys_usw_parser_set_hash_field(uint8 lchip,
                              ctc_parser_ecmp_hash_ctl_t* p_hash_ctl,
                              sys_parser_hash_usage_t hash_usage);
extern int32
sys_usw_parser_get_hash_field(uint8 lchip,
                              ctc_parser_hash_ctl_t* p_hash_ctl,
                              sys_parser_hash_usage_t hash_usage);

extern int32
sys_usw_parser_hash_init(uint8 lchip);

extern int32
sys_usw_ftm_mapping_drv_hash_poly_type(uint8 lchip, uint8 sram_type, uint32 type, uint32* p_poly);
extern int32
sys_usw_ftm_get_current_hash_poly_type(uint8 lchip, uint32 mem_id, uint8 sram_type, uint32 *poly_type);
extern int32
sys_usw_ftm_get_hash_poly_cap(uint8 lchip, uint8 sram_type, ctc_ftm_hash_poly_t* hash_poly);
extern int32
sys_usw_mac_link_up_event(uint8 lchip, uint32 gport);

sys_usw_mchip_api_t sys_duet2_mchip_api = {
    sys_duet2_capability_init,
/*##ipuc##*/
    sys_duet2_ipuc_tcam_init,
    sys_duet2_ipuc_tcam_deinit,
    sys_duet2_ipuc_tcam_get_blockid,
    sys_duet2_ipuc_tcam_write_key,
    sys_duet2_ipuc_tcam_write_ad,
    sys_duet2_ipuc_tcam_move,
    sys_duet2_ipuc_tcam_show_key,
    sys_duet2_ipuc_tcam_show_status,
    NULL,
    sys_duet2_calpm_init,
    sys_duet2_calpm_deinit,
    sys_duet2_calpm_add,
    sys_duet2_calpm_del,
    sys_duet2_calpm_update,
    sys_duet2_calpm_arrange_fragment,
    sys_duet2_calpm_show_calpm_key,
    sys_duet2_calpm_show_status,
    sys_duet2_calpm_mapping_wb_master,
    sys_duet2_calpm_get_wb_info,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    /*####*/
    sys_duet2_chip_set_mem_bist,
    /*parser hash*/
    sys_usw_parser_set_hash_field,
    sys_usw_parser_get_hash_field,
    sys_usw_parser_hash_init,
    NULL,
    NULL,
    NULL,
    NULL,

    /* DMPS */
    sys_duet2_mac_set_property,
    sys_duet2_mac_get_property,
    sys_duet2_mac_set_interface_mode,
    sys_duet2_mac_get_link_up,
    sys_duet2_mac_get_capability,
    sys_duet2_serdes_set_property,
    sys_duet2_serdes_get_property,
    sys_duet2_serdes_set_mode,
    sys_duet2_serdes_set_link_training_en,
    sys_duet2_serdes_get_link_training_status,
    sys_duet2_datapath_init,
    sys_duet2_mac_init,
    sys_duet2_mcu_show_debug_info,
    NULL,
    sys_usw_mac_link_up_event,

    /*ftm*/
    sys_usw_ftm_mapping_drv_hash_poly_type,
    sys_usw_ftm_get_current_hash_poly_type,
    sys_usw_ftm_get_hash_poly_cap,

    /*peri*/
    sys_duet2_peri_init,
    sys_duet2_peri_mdio_init,
    sys_duet2_peri_set_phy_scan_cfg,
    sys_duet2_peri_set_phy_scan_para,
    sys_duet2_peri_get_phy_scan_para,
    sys_duet2_peri_read_phy_reg,
    sys_duet2_peri_write_phy_reg,
    sys_duet2_peri_set_mdio_clock,
    sys_duet2_peri_get_mdio_clock,
    sys_duet2_peri_set_mac_led_mode,
    sys_duet2_peri_set_mac_led_mapping,
    sys_duet2_peri_set_mac_led_en,
    sys_duet2_peri_get_mac_led_en,
    sys_duet2_peri_set_gpio_mode,
    sys_duet2_peri_set_gpio_output,
    sys_duet2_peri_get_gpio_input,
    sys_duet2_peri_phy_link_change_isr,
    sys_duet2_peri_get_chip_sensor,

    /*diag*/
    NULL,
    NULL,
    NULL,
    NULL,
};


int32
sys_duet2_mchip_init(uint8 lchip)
{
    p_sys_mchip_master[lchip]->p_capability = (uint32*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(uint32)*SYS_CAP_MAX);
    if (NULL == p_sys_mchip_master[lchip]->p_capability)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_sys_mchip_master[lchip]->p_capability, 0, sizeof(uint32)*SYS_CAP_MAX);

    p_sys_mchip_master[lchip]->p_mchip_api = &sys_duet2_mchip_api;

    return CTC_E_NONE;
}

int32
sys_duet2_mchip_deinit(uint8 lchip)
{
    CTC_PTR_VALID_CHECK(p_sys_mchip_master[lchip]);

    if (p_sys_mchip_master[lchip]->p_capability)
    {
        mem_free(p_sys_mchip_master[lchip]->p_capability);
    }

    return CTC_E_NONE;
}

#endif

