
#ifdef TSINGMA

#include "ctc_error.h"
#include "sys_usw_mchip.h"
#include "sys_usw_ipuc.h"
#include "sys_tsingma_ipuc_tcam.h"
#include "sys_tsingma_nalpm.h"
#include "sys_tsingma_mac.h"
#include "sys_tsingma_datapath.h"
#include "sys_tsingma_mcu.h"
#include "sys_usw_ftm.h"
#include "sys_tsingma_peri.h"
extern int32 sys_tsingma_capability_init(uint8 lchip);
extern int32 sys_tsingma_hash_set_config(uint8 lchip, void* p_hash_cfg);
extern int32 sys_tsingma_hash_get_config(uint8 lchip, void* p_hash_cfg);
extern int32 sys_tsingma_hash_set_offset(uint8 lchip, void* p_hash_offset);
extern int32 sys_tsingma_hash_get_offset(uint8 lchip, void* p_hash_offset);

extern int32
sys_tsingma_parser_set_hash_field(uint8 lchip,
                                  ctc_parser_hash_ctl_t* p_hash_ctl,
                                  sys_parser_hash_usage_t hash_usage);
extern int32
sys_tsingma_parser_get_hash_field(uint8 lchip,
                                  ctc_parser_hash_ctl_t* p_hash_ctl,
                                  sys_parser_hash_usage_t hash_usage);


extern int32
sys_tsingma_parser_hash_init(uint8 lchip);


extern int32
sys_tsingma_ipuc_tcam_show_status(uint8 lchip);


extern int32
sys_tsingma_ftm_mapping_drv_hash_poly_type(uint8 lchip, uint8 sram_type, uint32 type, uint32* p_poly);
extern int32
sys_tsingma_ftm_get_current_hash_poly_type(uint8 lchip, uint32 mem_id, uint8 sram_type, uint32 *poly_type);
extern int32
sys_tsingma_ftm_get_hash_poly_cap(uint8 lchip, uint8 sram_type, ctc_ftm_hash_poly_t* hash_poly);
extern int32
sys_tsingma_nalpm_show_sram_usage(uint8 lchip);

/**DIAG START*/
extern int32
sys_tsingma_diag_get_pkt_trace(uint8 lchip, void* p_value);
extern int32
sys_tsingma_diag_get_drop_info(uint8 lchip, void* p_value);
extern int32
sys_tsingma_diag_set_dbg_pkt(uint8 lchip, void* p_value);
extern int32
sys_tsingma_diag_set_dbg_session(uint8 lchip, void* p_value);
extern int32
sys_tsingma_mac_link_up_event(uint8 lchip, uint32 gport);
/**DIAG END*/

sys_usw_mchip_api_t sys_tsingma_mchip_api = {
    sys_tsingma_capability_init,
/*##ipuc##*/
    sys_tsingma_ipuc_tcam_init,
    sys_tsingma_ipuc_tcam_deinit,
    sys_tsingma_ipuc_tcam_get_blockid,
    sys_tsingma_ipuc_tcam_write_key,
    sys_tsingma_ipuc_tcam_write_ad,
    sys_tsingma_ipuc_tcam_move,
    sys_tsingma_ipuc_tcam_show_key,
    sys_tsingma_ipuc_tcam_show_status,
    sys_tsingma_nalpm_show_sram_usage,
    sys_tsingma_nalpm_init,
    sys_tsingma_nalpm_deinit,
    sys_tsingma_nalpm_add,
    sys_tsingma_nalpm_del,
    sys_tsingma_nalpm_update,
    NULL,
    sys_tsingma_nalpm_show_route_info,
    NULL,
    NULL,
    sys_tsingma_nalpm_wb_get_info,
    sys_tsingma_nalpm_dump_db,
    sys_tsingma_nalpm_merge,
    sys_tsingma_nalpm_set_fragment_status,
    sys_tsingma_nalpm_get_fragment_status,
    sys_tsingma_nalpm_get_fragment_auto_enable,
    _sys_tsingma_nalpm_get_real_tcam_index,
    NULL,
    sys_tsingma_parser_set_hash_field,
    sys_tsingma_parser_get_hash_field,
    sys_tsingma_parser_hash_init,
    sys_tsingma_hash_set_config,
    sys_tsingma_hash_get_config,
    sys_tsingma_hash_set_offset,
    sys_tsingma_hash_get_offset,

    /* DMPS */
    sys_tsingma_mac_set_property,
    sys_tsingma_mac_get_property,
    sys_tsingma_mac_set_interface_mode,
    sys_tsingma_mac_get_link_up,
    sys_tsingma_mac_get_capability,
    sys_tsingma_serdes_set_property,
    sys_tsingma_serdes_get_property,
    sys_tsingma_serdes_set_mode,
    sys_tsingma_serdes_set_link_training_en,
    sys_tsingma_serdes_get_link_training_status,
    sys_tsingma_datapath_init,
    sys_tsingma_mac_init,
    sys_tsingma_mcu_show_debug_info,
    sys_tsingma_mac_self_checking,
    sys_tsingma_mac_link_up_event,

    /*ftm*/
    sys_tsingma_ftm_mapping_drv_hash_poly_type,
    sys_tsingma_ftm_get_current_hash_poly_type,
    sys_tsingma_ftm_get_hash_poly_cap,
    /*peri*/
    sys_tsingma_peri_init,
    sys_tsingma_peri_mdio_init,
    sys_tsingma_peri_set_phy_scan_cfg,
    sys_tsingma_peri_set_phy_scan_para,
    sys_tsingma_peri_get_phy_scan_para,
    sys_tsingma_peri_read_phy_reg,
    sys_tsingma_peri_write_phy_reg,
    sys_tsingma_peri_set_mdio_clock,
    sys_tsingma_peri_get_mdio_clock,
    sys_tsingma_peri_set_mac_led_mode,
    sys_tsingma_peri_set_mac_led_mapping,
    sys_tsingma_peri_set_mac_led_en,
    sys_tsingma_peri_get_mac_led_en,
    sys_tsingma_peri_set_gpio_mode,
    sys_tsingma_peri_set_gpio_output,
    sys_tsingma_peri_get_gpio_input,
    sys_tsingma_peri_phy_link_change_isr,
    sys_tsingma_peri_get_chip_sensor,

    /*diag*/
    sys_tsingma_diag_get_pkt_trace,
    sys_tsingma_diag_get_drop_info,
    sys_tsingma_diag_set_dbg_pkt,
    sys_tsingma_diag_set_dbg_session,
};

int32
sys_tsingma_mchip_init(uint8 lchip)
{

    p_sys_mchip_master[lchip]->p_capability = (uint32*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(uint32)*SYS_CAP_MAX);
    if (NULL == p_sys_mchip_master[lchip]->p_capability)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_sys_mchip_master[lchip]->p_capability, 0, sizeof(uint32)*SYS_CAP_MAX);

    p_sys_mchip_master[lchip]->p_mchip_api = &sys_tsingma_mchip_api;

    return CTC_E_NONE;
}

int32
sys_tsingma_mchip_deinit(uint8 lchip)
{
    CTC_PTR_VALID_CHECK(p_sys_mchip_master[lchip]);

    if (p_sys_mchip_master[lchip]->p_capability)
    {
        mem_free(p_sys_mchip_master[lchip]->p_capability);
    }

    return CTC_E_NONE;
}

#endif

