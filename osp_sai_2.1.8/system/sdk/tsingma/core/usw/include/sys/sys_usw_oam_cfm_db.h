#if (FEATURE_MODE == 0)
/**
 @file sys_usw_oam_cfm_db.h

 @date 2010-1-19

 @version v2.0

  The file defines Macro, stored data structure for  Ethernet OAM module
*/
#ifndef _SYS_USW_OAM_CFM_DB_H
#define _SYS_USW_OAM_CFM_DB_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************************
 *
 * Header Files
 *
****************************************************************************/

#include "ctc_oam.h"

#include "sys_usw_oam.h"
#include "sys_usw_oam_db.h"

/****************************************************************************
*
* Defines and Macros
*
****************************************************************************/

/****************************************************************************
*
* Global and Declaration
*
*****************************************************************************/

/****************************************************************************
 *
* Function
*
****************************************************************************/
#define MAID_H ""


#define CHAN_H ""

sys_oam_chan_eth_t*
_sys_usw_cfm_chan_lkup(uint8 lchip, ctc_oam_key_t* p_key_parm);

sys_oam_chan_eth_t*
_sys_usw_cfm_chan_build_node(uint8 lchip, ctc_oam_lmep_t* p_chan_param);

int32
_sys_usw_cfm_chan_free_node(uint8 lchip, sys_oam_chan_eth_t* p_sys_chan_eth);

int32
_sys_usw_cfm_chan_build_index(uint8 lchip, sys_oam_chan_eth_t* p_sys_chan_eth);

int32
_sys_usw_cfm_chan_free_index(uint8 lchip, sys_oam_chan_eth_t* p_sys_chan_eth);

int32
_sys_usw_cfm_chan_add_to_db(uint8 lchip, sys_oam_chan_eth_t* p_sys_chan_eth);

int32
_sys_usw_cfm_chan_del_from_db(uint8 lchip, sys_oam_chan_eth_t* p_sys_chan_eth);

int32
_sys_usw_cfm_chan_add_to_asic(uint8 lchip, sys_oam_chan_eth_t* p_sys_chan_eth);
int32
_sys_usw_cfm_chan_del_from_asic(uint8 lchip, sys_oam_chan_eth_t* p_sys_chan_eth);

int32
_sys_usw_cfm_chan_update(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth, bool is_add);

int32
_sys_usw_cfm_chan_update_reomte(uint8 lchip, ctc_oam_lmep_t* p_lmep, sys_oam_chan_eth_t* p_sys_chan_eth, bool is_add);


#define LMEP_H ""

sys_oam_lmep_t*
_sys_usw_cfm_lmep_lkup(uint8 lchip, uint8 md_level, sys_oam_chan_eth_t* p_sys_chan_eth);

sys_oam_lmep_t*
_sys_usw_cfm_lmep_build_node(uint8 lchip, ctc_oam_lmep_t* p_lmep_param);

int32
_sys_usw_cfm_lmep_free_node(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth);

int32
_sys_usw_cfm_lmep_build_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth);

int32
_sys_usw_cfm_lmep_free_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth);

int32
_sys_usw_cfm_lmep_build_lm_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth);

int32
_sys_usw_cfm_lmep_free_lm_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth);

int32
_sys_usw_cfm_lmep_add_to_db(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth);

int32
_sys_usw_cfm_lmep_del_from_db(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth);

int32
_sys_usw_cfm_lmep_add_to_asic(uint8 lchip, ctc_oam_lmep_t* p_lmep_param,
                                    sys_oam_lmep_t* p_sys_lmep_eth);

int32
_sys_usw_cfm_lmep_del_from_asic(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_eth);

int32
_sys_usw_cfm_lmep_update_asic(uint8 lchip, ctc_oam_update_t* p_lmep_param,
                                    sys_oam_lmep_t* p_sys_lmep_eth);

int32
_sys_usw_cfm_update_lmep_port_status(uint8 lchip, ctc_oam_update_t* p_lmep_param);

int32
_sys_usw_cfm_update_lmep_if_status(uint8 lchip, ctc_oam_update_t* p_lmep_param);

int32
_sys_usw_cfm_update_lmep_lm(uint8 lchip, ctc_oam_update_t* p_lmep_param);

int32
_sys_usw_cfm_update_lmep_npm(uint8 lchip, ctc_oam_update_t* p_lmep_param);

int32
_sys_usw_cfm_update_master_gchip(uint8 lchip, ctc_oam_update_t* p_lmep_param);


#define RMEP_H ""

sys_oam_rmep_t*
_sys_usw_cfm_rmep_lkup(uint8 lchip, uint32 rmep_id, sys_oam_lmep_t* p_sys_lmep_eth);

sys_oam_rmep_t*
_sys_usw_cfm_rmep_build_node(uint8 lchip, ctc_oam_rmep_t* p_rmep_param);

int32
_sys_usw_cfm_rmep_free_node(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_eth);

int32
_sys_usw_cfm_rmep_build_index(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_eth, sys_oam_lmep_t* p_sys_lmep_eth);

int32
_sys_usw_cfm_rmep_free_index(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_eth);

int32
_sys_usw_cfm_rmep_add_to_db(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_eth);

int32
_sys_usw_cfm_rmep_del_from_db(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_eth);

int32
_sys_usw_cfm_rmep_add_to_asic(uint8 lchip, ctc_oam_rmep_t* p_rmep_param,
                                    sys_oam_rmep_t* p_sys_rmep_eth);

int32
_sys_usw_cfm_rmep_del_from_asic(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_eth);

int32
_sys_usw_cfm_rmep_update_asic(uint8 lchip, ctc_oam_update_t* p_rmep_param,
                                    sys_oam_rmep_t* p_sys_rmep_eth);

#define COMMON_SET_H ""

int32
_sys_usw_cfm_oam_set_port_tunnel_en(uint8 lchip, uint32 gport, uint8 enable);

int32
_sys_usw_cfm_oam_set_port_oam_en(uint8 lchip, uint32 gport, uint8 dir, uint8 enable);

int32
_sys_usw_cfm_oam_set_relay_all_to_cpu_en(uint8 lchip, uint8 enable);

int32
_sys_usw_cfm_oam_set_port_edge_en(uint8 lchip, uint32 gport, uint8 enable);

int32
_sys_usw_cfm_oam_set_tx_vlan_tpid(uint8 lchip, uint8 tpid_index, uint16 tpid);

int32
_sys_usw_cfm_oam_set_rx_vlan_tpid(uint8 lchip, uint8 tpid_index, uint16 tpid);

int32
_sys_usw_cfm_oam_set_send_id(uint8 lchip, ctc_oam_eth_senderid_t* p_sender_id);

int32
_sys_usw_cfm_oam_set_big_ccm_interval(uint8 lchip, uint8 big_interval);

int32
_sys_usw_cfm_oam_set_port_lm_en(uint8 lchip, uint32 gport, uint8 enable);

int32
_sys_usw_cfm_oam_set_bridge_mac(uint8 lchip, mac_addr_t* bridge_mac);

int32
_sys_usw_cfm_oam_set_lbm_process_by_asic(uint8 lchip, uint8 enable);

int32
_sys_usw_cfm_oam_set_lmm_process_by_asic(uint8 lchip, uint8 enable);

int32
_sys_usw_cfm_oam_set_dm_process_by_asic(uint8 lchip, uint8 enable);

int32
_sys_usw_cfm_oam_set_slm_process_by_asic(uint8 lchip, uint8 enable);

int32
_sys_usw_cfm_oam_set_lbr_sa_use_lbm_da(uint8 lchip, uint8 enable);

int32
_sys_usw_cfm_oam_set_lbr_sa_use_bridge_mac(uint8 lchip, uint8 enable);

int32
_sys_usw_cfm_set_mip_port_en(uint8 lchip, uint32 gport, uint8 level_bitmap);

int32
_sys_usw_cfm_set_if_status(uint8 lchip, uint32 gport, uint8 if_status);

int32
_sys_usw_cfm_oam_set_ach_channel_type(uint8 lchip, uint16 value);

int32
_sys_usw_cfm_oam_set_rdi_mode(uint8 lchip, uint32 rdi_mode);

int32
_sys_usw_cfm_oam_set_ccm_lm_en(uint8 lchip, uint32 enable);

#define COMMON_GET_H ""

int32
_sys_usw_cfm_oam_get_port_oam_en(uint8 lchip, uint32 gport, uint8 dir, uint8* enable);

int32
_sys_usw_cfm_oam_get_port_tunnel_en(uint8 lchip, uint32 gport, uint8* enable);

int32
_sys_usw_cfm_oam_get_big_ccm_interval(uint8 lchip, uint8* big_interval);

int32
_sys_usw_cfm_oam_get_port_lm_en(uint8 lchip, uint32 gport, uint8* enable);

int32
_sys_usw_cfm_oam_get_lbm_process_by_asic(uint8 lchip, uint8* enable);

int32
_sys_usw_cfm_oam_get_lmm_process_by_asic(uint8 lchip, uint8* enable);

int32
_sys_usw_cfm_oam_get_dm_process_by_asic(uint8 lchip, uint8* enable);

int32
_sys_usw_cfm_oam_get_slm_process_by_asic(uint8 lchip, uint8* enable);

int32
_sys_usw_cfm_oam_get_lbr_sa_use_lbm_da(uint8 lchip, uint8* enable);

int32
_sys_usw_cfm_oam_get_l2vpn_oam_mode(uint8 lchip, uint8* enable);

int32
_sys_usw_cfm_update_lmep_vlan(uint8 lchip, ctc_oam_update_t* p_lmep_param);

int32
_sys_usw_cfm_oam_get_mip_port_en(uint8 lchip, uint32 gport, uint8* level_bitmap);

int32
_sys_usw_cfm_oam_get_if_status(uint8 lchip, uint32 gport, uint8* if_status);

int32
_sys_usw_cfm_oam_get_ach_channel_type(uint8 lchip, uint16* value);

int32
_sys_usw_cfm_register_init(uint8 lchip);

int32
_sys_usw_cfm_oam_get_rdi_mode(uint8 lchip, uint32* rdi_mode);

int32
_sys_usw_cfm_oam_get_ccm_lm_en(uint8 lchip, uint32* enable);

#ifdef __cplusplus
}
#endif

#endif

#endif
