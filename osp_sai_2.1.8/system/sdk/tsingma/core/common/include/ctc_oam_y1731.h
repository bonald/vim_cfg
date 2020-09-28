/**
 @file ctc_oam_y1731.h

 @date 2010-1-19

 @version v2.0

 This file contains Ethernet OAM(80.21.ag/y1731) ,MPLS TP OAM(Y.1731) OAM related data structure, enum, macro.

*/

#ifndef _CTC_OAM_Y1731_H
#define _CTC_OAM_Y1731_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal_types.h"
#include "common/include/ctc_const.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/**
 @defgroup y1731_oam Y.1731_OAM
 @{
*/
#define CTC_OAM_ETH_MAX_SENDER_ID_LEN        15     /**< The max length of SENDER ID */

/**
 @brief  Define CFM OAM port status
*/
enum ctc_oam_eth_port_status_s
{
    CTC_OAM_ETH_PS_BLOCKED=1,         /**< [GB.GG.D2.TM] Ordinary data can't pass the port mep resides*/
    CTC_OAM_ETH_PS_UP                 /**< [GB.GG.D2.TM] Ordinary data can pass the port mep resides*/
};
typedef enum ctc_oam_eth_port_status_s ctc_oam_eth_port_status_t;

/**
 @brief  Define CFM OAM interface status
*/
enum ctc_oam_eth_interface_status_s
{
    CTC_OAM_ETH_INTF_IS_UP = 1,                  /**< [GB.GG.D2.TM] Interface is ready to pass pkt*/
    CTC_OAM_ETH_INTF_IS_DOWN,                    /**< [GB.GG.D2.TM] Interface can't pass pkt*/
    CTC_OAM_ETH_INTF_IS_TESTING,                 /**< [GB.GG.D2.TM] Interface in some test mode*/
    CTC_OAM_ETH_INTF_IS_UNKNOWN,                 /**< [GB.GG.D2.TM] Interface status can't be determined*/
    CTC_OAM_ETH_INTF_IS_DORMANT,                 /**< [GB.GG.D2.TM] Interface in a pending state*/
    CTC_OAM_ETH_INTF_IS_NOT_PRESENT,             /**< [GB.GG.D2.TM] Some componet is missing*/
    CTC_OAM_ETH_INTF_IS_LOWER_LAYER_DOWN         /**< [GB.GG.D2.TM] Interface is down due to lower layer stauts*/
};
typedef enum ctc_oam_eth_interface_status_s ctc_oam_eth_interface_status_t;

/**
 @brief   Define the sender id of CCM in 802.1ag mode
 */
struct ctc_oam_eth_senderid_s
{
    uint8 sender_id[CTC_OAM_ETH_MAX_SENDER_ID_LEN]; /**< [GB.GG.D2.TM] Sender id  */
    uint8 sender_id_len;                            /**< [GB.GG.D2.TM] The length of sender id */
};
typedef struct ctc_oam_eth_senderid_s ctc_oam_eth_senderid_t;

/**
 @brief   Define Ethernet OAM local mep flag
*/
enum ctc_oam_y1731_lmep_flag_e
{
    CTC_OAM_Y1731_LMEP_FLAG_NONE            = 0x00000000,  /**< [GB.GG.D2.TM] None*/
    CTC_OAM_Y1731_LMEP_FLAG_MEP_EN          = 0x00000001,  /**< [GB.GG.D2.TM] If set, indicate local mep is active */
    CTC_OAM_Y1731_LMEP_FLAG_DM_EN           = 0x00000002,  /**< [GB.GG.D2.TM] If set, indicate local mep dm is enable */
    CTC_OAM_Y1731_LMEP_FLAG_LM_EN           = 0x00000004,  /**< [GB.GG.D2.TM] If set, indicate local mep enable LM*/
    CTC_OAM_Y1731_LMEP_FLAG_TX_CSF_EN       = 0x00000008,  /**< [GB.GG.D2.TM] If set, indicate local mep enable TX CSF*/
    CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU      = 0x00000010,  /**< [GB.GG.D2.TM] If set, indicate local mep is proc by software not by asic */
    CTC_OAM_Y1731_LMEP_FLAG_APS_EN          = 0x00000020,  /**< [GB.GG] If set, indicate local mep aps message is enable */
    CTC_OAM_Y1731_LMEP_FLAG_CCM_SEQ_NUM_EN  = 0x00000040,  /**< [GB.GG.D2.TM] [1ag] If set, indicate local mep ccm sequence num is enable */
    CTC_OAM_Y1731_LMEP_FLAG_TX_IF_STATUS    = 0x00000080,  /**< [GB.GG.D2.TM] [1ag] If set, indicate local mep ccm tx with interface status TLV*/
    CTC_OAM_Y1731_LMEP_FLAG_TX_PORT_STATUS  = 0x00000100,  /**< [GB.GG.D2.TM] [1ag] If set, indicate local mep ccm tx with port status TLV*/
    CTC_OAM_Y1731_LMEP_FLAG_TX_SEND_ID      = 0x00000200,  /**< [GB.GG.D2.TM] [1ag] If set, indicate local mep ccm tx with sender id TLV*/
    CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE        = 0x00000800,  /**< [GB.GG.D2.TM] [1ag/Y1731] If set, indicate local mep is p2p mep */
    CTC_OAM_Y1731_LMEP_FLAG_SHARE_MAC       = 0x00001000,  /**< [GB.GG.D2.TM] [1ag/Y1731] If set, indicate local mep share mac mode*/
    CTC_OAM_Y1731_LMEP_FLAG_WITHOUT_GAL     = 0x00002000,  /**< [GB.GG.D2.TM] [TP Y1731] If set, indicate local mep pw without gal */
    CTC_OAM_Y1731_LMEP_FLAG_PROTECTION_PATH = 0x00004000,  /**< [GB.GG.D2.TM] [TP Y1731] If set, indicate local mep config on protection path, else working path*/
    CTC_OAM_Y1731_LMEP_FLAG_TWAMP_EN        = 0x00008000,  /**< [D2.TM] If set, TWAMP sender/receiver proc by asic*/
    CTC_OAM_Y1731_LMEP_FLAG_TWAMP_REFLECT_EN = 0x00010000,  /**< [D2.TM] If set, indicate TWAMP reflector enable*/
    CTC_OAM_Y1731_LMEP_FLAG_ALL             = 0x000FFFFF   /**< [GB.GG] All*/
};
typedef enum ctc_oam_y1731_lmep_flag_e ctc_oam_y1731_lmep_flag_t;

/**
 @brief   Define Ethernet OAM  local mep  update type
*/
enum ctc_oam_y1731_lmep_update_type_e
{
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NONE,              /**< [GB.GG.D2.TM] None */
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_EN,            /**< [GB.GG.D2.TM] Mep enable */
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_EN,            /**< [GB.GG.D2.TM] CCM enable  */
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_DM_EN,             /**< [GB.GG.D2.TM] DM enable */
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_COS_EXP,        /**< [GB.GG.D2.TM] Local mep transmit cos value*/
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_RDI,               /**< [GB.GG.D2.TM] Update local rdi value */
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LMEP_SF_STATE,     /**< [GB.GG] Update local signal fail state value, update_value: signal fail state*/
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_SEQ_NUM_EN,    /**< [GB.GG.D2.TM] [1ag] If set, tx ccm with sequence number */
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_EN,         /**< [GB.GG.D2.TM] If set, the mep is used as server mep, and enable tx CSF PDU (must disable CCM tx function) */
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_TYPE,       /**< [GB.GG.D2.TM] Local mep CSF tx type, refer to ctc_oam_csf_type_t*/
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_EN,             /**< [GB.GG.D2.TM] LM enable */
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_TYPE,           /**< [GB.GG.D2.TM] LM type,  refer to ctc_oam_lm_type_t*/
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS_TYPE,       /**< [GB.GG.D2.TM] LM cos type, refer to ctc_oam_lm_cos_type_t*/
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LM_COS,            /**< [GB.GG.D2.TM] Special lm cos value*/
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_PORT_STATUS,       /**< [GB.GG.D2.TM] [1ag] refer to ctc_oam_eth_port_status_t*/
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_IF_STATUS,         /**< [GB.GG.D2.TM] [1ag] refer to ctc_oam_eth_interface_status_t*/
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_USE_UC_DA,  /**< [GB.GG.D2.TM] [1ag/Y1731] If set, CSF tx use unicast mac da*/
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_P2P_TX_USE_UC_DA,  /**< [GB.GG.D2.TM] [1ag/Y1731] If set, P2P mep tx use unicast mac da*/
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NHOP,              /**< [GB.GG.D2.TM] [TP Y1731] If set, will use NextHop to edit PDU*/
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEL,               /**< [GB.GG.D2.TM] [1ag/Y1731] MD level*/
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TRPT,              /**<[GB.GG.D2.TM] Update local mep throughput state, 0xff means disable */
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LOCK,              /**< [GG.D2.TM] [TP Y1731] If set, enable MPLS-TP lock */
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MASTER_GCHIP,      /**< [GB.GG.D2.TM] Update mep master gchip ID */
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NPM,               /**< [D2.TM] Associate local mep with NPM session, 0xff means disable */
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TTL,               /**< [GG.D2.TM] [TP Y1731] MEP TTL */
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MAID,              /**< [D2.TM] MAID, refer to ctc_oam_maid_t*/
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_INTERVAL,      /**< [D2.TM] CCM interval */
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_ID,            /**< [D2.TM] Mep id */
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LABEL,             /**< [D2.TM] Update MPLS-TP OAM lookup key, associated in Label*/
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_STATS_EN,      /**< [D2.TM] Rx:1.equal/low ccm stats 2.error ccm stats; Tx: tx ccm stats*/
    CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MAX
};
typedef enum ctc_oam_y1731_lmep_update_type_e ctc_oam_y1731_lmep_update_type_t;

/**
 @brief   Define Ethernet OAM  local mep
*/
struct ctc_oam_y1731_lmep_s
{
    uint32 flag;                 /**< [GB.GG.D2.TM] Local mep flag, refer to ctc_oam_y1731_lmep_flag_t */

    uint8  master_gchip;         /**< [GB.GG.D2.TM] MEP's master gchip ID, the MEP status is maintained on the master chip */
    uint8  ccm_interval;         /**< [GB.GG.D2.TM] CCM interval */
    uint16 mep_id;               /**< [GB.GG.D2.TM] MEP id */

    uint8  lm_type;              /**< [GB.GG.D2.TM] LM type, refer to ctc_oam_lm_type_t   */
    uint8  lm_cos_type;          /**< [GB.GG.D2.TM] LM cos type, refer to ctc_oam_lm_cos_type_t*/
    uint8  lm_cos;               /**< [GB.GG.D2.TM] LM cos, only used for lm cos type CTC_OAM_LM_COS_TYPE_SPECIFIED_COS */
    uint8  tx_csf_type;          /**< [GB.GG.D2.TM] Tx csf type, refer to ctc_oam_csf_type_t */

    uint8  tx_cos_exp;           /**< [GB.GG.D2.TM] Local mep tx cos */
    uint8  tpid_index;           /**< [GB.GG.D2.TM] [1ag/Y1731] Ether type index for vlan tpid */

    uint8  mpls_ttl;             /**< [GB.GG.D2.TM] [TP Y1731] Local mep tx mpls ttl*/
    uint8  rsv;

    uint32 nhid;                 /**< [GB.GG.D2.TM] [TP Y1731] NextHop id, */
};
typedef struct ctc_oam_y1731_lmep_s ctc_oam_y1731_lmep_t;

/**
 @brief   Define Ethernet OAM remote mep flag
*/
enum ctc_oam_y1731_rmep_flag_e
{
    CTC_OAM_Y1731_RMEP_FLAG_NONE              = 0x00000000, /**< [GB.GG.D2.TM] None*/
    CTC_OAM_Y1731_RMEP_FLAG_MEP_EN            = 0x00000001, /**< [GB.GG.D2.TM] If set, indicate Remote mep is active */
    CTC_OAM_Y1731_RMEP_FLAG_CSF_EN            = 0x00000002, /**< [GB.GG.D2.TM] If set, indicate Remote mep CSF rx enable */
    CTC_OAM_Y1731_RMEP_FLAG_CCM_SEQ_NUM_EN    = 0x00000004, /**< [GB.GG.D2.TM] [1ag] If set, indicate Remote mep sequence num check enable */
    CTC_OAM_Y1731_RMEP_FLAG_MAC_UPDATE_EN     = 0x00000008, /**< [GB.GG.D2.TM] [1ag] If set, indicate Remote mep source mac learn enable */
    CTC_OAM_Y1731_RMEP_FLAG_ALL               = 0x0000000F  /**< [GB.GG.D2.TM] All*/
};
typedef enum ctc_oam_y1731_rmep_flag_e ctc_oam_y1731_rmep_flag_t;

/**
 @brief   Define Ethernet OAM remote mep update type
*/
enum ctc_oam_y1731_rmep_update_type_e
{
    CTC_OAM_Y1731_RMEP_UPDATE_TYPE_NONE,              /**< [GB.GG.D2.TM] None */
    CTC_OAM_Y1731_RMEP_UPDATE_TYPE_MEP_EN,            /**< [GB.GG.D2.TM] Mep enable*/
    CTC_OAM_Y1731_RMEP_UPDATE_TYPE_CSF_EN,            /**< [GB.GG.D2.TM] Csf rx enable*/
    CTC_OAM_Y1731_RMEP_UPDATE_TYPE_DCSF,              /**< [GB.GG.D2.TM] dcsf(CTC_OAM_DEFECT_CSF) bit set by software*/
    CTC_OAM_Y1731_RMEP_UPDATE_TYPE_RMEP_SF_STATE,     /**< [GB.GG] Update remote signal fail state value*/
    CTC_OAM_Y1731_RMEP_UPDATE_TYPE_SEQ_FAIL_CLEAR,    /**< [GB.GG.D2.TM] [1ag] Reset remote mep's sequence num fail counter*/
    CTC_OAM_Y1731_RMEP_UPDATE_TYPE_CCM_SEQ_NUM_EN,    /**< [GB.GG.D2.TM] [1ag] CCM sequence number check enable*/
    CTC_OAM_Y1731_RMEP_UPDATE_TYPE_RMAC_MIS,          /**< [GB.GG.D2.TM] [1ag] rmepmacmismatch(CTC_OAM_DEFECT_SRC_MAC_MISMATCH) bit set by software*/
    CTC_OAM_Y1731_RMEP_UPDATE_TYPE_PORT_INTF_MIS,     /**< [GB.GG.D2.TM] [1ag] macStatusChange(CTC_OAM_DEFECT_MAC_STATUS_CHANGE) bit set by software*/
    CTC_OAM_Y1731_RMEP_UPDATE_TYPE_HW_APS,            /**< [D2.TM] Associate Mep with hw APS configure info, refer to ctc_oam_hw_aps_t */
    CTC_OAM_Y1731_RMEP_UPDATE_TYPE_HW_APS_EN,         /**< [D2.TM] If enabled, the chip will automatically switch when SF is detected on the path*/
    CTC_OAM_Y1731_RMEP_UPDATE_TYPE_RMEP_MACSA,        /**< [D2.TM] Update Remote mep mac sa(only used for ethoam)*/
    CTC_OAM_Y1731_RMEP_UPDATE_TYPE_MAX
};
typedef enum ctc_oam_y1731_rmep_update_type_e ctc_oam_y1731_rmep_update_type_t;

/**
 @brief   Define Ethernet OAM remote mep data structure
*/
struct ctc_oam_y1731_rmep_s
{
    uint32 flag;           /**< [GB.GG.D2.TM] Refer to ctc_oam_y1731_rmep_flag_t*/

    uint16     rmep_id;    /**< [GB.GG.D2.TM] Remote mep mep id*/
    mac_addr_t rmep_mac;   /**< [GB.GG.D2.TM] [1ag/Y1731] Remote mep mac address*/
};
typedef struct ctc_oam_y1731_rmep_s ctc_oam_y1731_rmep_t;

/**
 @brief   Define Ethernet OAM property configure type
*/
enum ctc_oam_y1731_cfg_type_e
{
    CTC_OAM_Y1731_CFG_TYPE_BIG_CCM_INTERVAL_TO_CPU, /**< [GB.GG.D2.TM] If receieved CCM intervel is greater than configured value, will send to CPU*/
    CTC_OAM_Y1731_CFG_TYPE_LBM_PROC_IN_ASIC,       /**< [GB.GG.D2.TM] LBM process by ASIC*/
    CTC_OAM_Y1731_CFG_TYPE_LMM_PROC_IN_ASIC,       /**< [GG.D2.TM] LMM process by ASIC*/
    CTC_OAM_Y1731_CFG_TYPE_DM_PROC_IN_ASIC,        /**< [GG.D2.TM] DM process by ASIC*/
    CTC_OAM_Y1731_CFG_TYPE_SLM_PROC_IN_ASIC,       /**< [GG.D2.TM] SLM process by ASIC*/

    CTC_OAM_Y1731_CFG_TYPE_PORT_OAM_EN,            /**< [GB.GG.D2.TM] [1ag/Y1731] Port oam en, key:(gport, dir)  */
    CTC_OAM_Y1731_CFG_TYPE_PORT_TUNNEL_EN,         /**< [GB.GG.D2.TM] [1ag/Y1731] Port tunnel en, key:(gport) */
    CTC_OAM_Y1731_CFG_TYPE_ALL_TO_CPU,             /**< [GB.GG.D2.TM] [1ag/Y1731] All packet proc by cpu*/
    CTC_OAM_Y1731_CFG_TYPE_TX_VLAN_TPID,           /**< [GB.GG.D2.TM] [1ag/Y1731] TX vlan tpid, value[31:16] tpid_index, value[15:0] vlan_tpid */
    CTC_OAM_Y1731_CFG_TYPE_RX_VLAN_TPID,           /**< [GB.GG.D2.TM] [1ag/Y1731] RX vlan tpid, value[31:16] tpid_index, value[15:0] vlan_tpid */
    CTC_OAM_Y1731_CFG_TYPE_ADD_EDGE_PORT,          /**< [GB.GG.D2.TM] [1ag/Y1731] Add edge to mep, key:(gport)*/
    CTC_OAM_Y1731_CFG_TYPE_REMOVE_EDGE_PORT,       /**< [GB.GG.D2.TM] [1ag/Y1731] Remove edge from mep, key:(gport)*/
    CTC_OAM_Y1731_CFG_TYPE_SENDER_ID,              /**< [GB.GG.D2.TM] [1ag/Y1731] Sender id, refer to ctc_oam_eth_senderid_t*/
    CTC_OAM_Y1731_CFG_TYPE_BRIDGE_MAC,             /**< [GB.GG.D2.TM] [1ag/Y1731] Bridge mac for share mac mode, refert to mac_addr_t*/
    CTC_OAM_Y1731_CFG_TYPE_LBR_SA_USE_LBM_DA,      /**< [GB.GG.D2.TM] [1ag/Y1731] Reply LBR/SLR/DMR macsa use LBM/SLM/DMM macda when LBM/SLM/DMM is unicast macda*/
    CTC_OAM_Y1731_CFG_TYPE_LBR_SA_SHARE_MAC,       /**< [GB.GG.D2.TM] [1ag/Y1731] Reply LBR/SLR/DMR use share mac mode*/
    CTC_OAM_Y1731_CFG_TYPE_PORT_LM_EN,             /**< [GB.GG.D2.TM] [1ag/Y1731] Port lm enable*/

    CTC_OAM_Y1731_CFG_TYPE_L2VPN_OAM_MODE,         /**< [GG] [1ag/Y1731] L2VPN oam mode, value[0: Port + Oam ID; 1: Port + Vlan] */
    CTC_OAM_Y1731_CFG_TYPE_TP_ACH_CHAN_TYPE,       /**< [GB.GG.D2.TM] [1ag/Y1731] TP Y1731 ACH channel type*/
    CTC_OAM_Y1731_CFG_TYPE_CFM_PORT_MIP_EN,        /**< [D2.TM] Port mip en, key:(gport, dir)*/
    CTC_OAM_Y1731_CFG_TYPE_IF_STATUS,              /**< [GB.GG.D2.TM] Interface status, refer to ctc_oam_eth_interface_status_t*/
    CTC_OAM_Y1731_CFG_TYPE_RDI_MODE,               /**< [D2.TM] RDIMode 0: Auto Generate RDI by asic; mode 1(only for P2P mode):Auto Genrate & Clear RDI by asic*/
    CTC_OAM_Y1731_CFG_TYPE_POAM_LM_STATS_EN,       /**< [D2] Eth-LM count proactive OAM frames transmitted or received by the MEP at the
                                                        MEP's MEG level(e.g., those for ETH-CC/ETH-APS and ETH-CSF) */
    CTC_OAM_Y1731_CFG_TYPE_MAX
};
typedef enum ctc_oam_y1731_cfg_type_e ctc_oam_y1731_cfg_type_t;

/**
 @brief   Define Ethernet OAM property data structure
*/
struct ctc_oam_y1731_prop_s
{
    uint8    cfg_type;  /**< [GB.GG.D2.TM] Config type, refer to ctc_oam_y1731_cfg_type_t*/
    uint8    dir;       /**< [GB.GG.D2.TM] [1ag/Y1731] Direction, refer to ctc_direction_t, set depend on cfg_type*/
    uint16   rsv0;

    uint32   gport;     /**< [GB.GG.D2.TM] [1ag/Y1731] Global port value£¬only used for port property*/

    uint32   value;     /**< [GB.GG.D2.TM] Configure value, set depend on cfg_type*/
    void*    p_value;   /**< [GB.GG.D2.TM] Configure value, set depend on cfg_type*/
};
typedef struct ctc_oam_y1731_prop_s ctc_oam_y1731_prop_t;

/*
 @brief   Define Ether & MPLS-TP Y.1731 Local MEP data structure get from both SDK database and ASIC dataset
*/
struct ctc_oam_y1731_lmep_info_s
{
    uint16  mep_id;        /**< [GB.GG.D2.TM] Local mep id*/
    uint8   ccm_interval;  /**< [GB.GG.D2.TM] CCM interval*/
    uint8   ccm_enable;    /**< [GB.GG.D2.TM] CCM enable*/

    uint8   dm_enable;     /**< [GG.D2.TM] DM enable*/
    uint8   active;        /**< [GB.GG.D2.TM] Active flag in local mep*/
    uint16  vlan_id;       /**< [GB.GG.D2.TM] [1ag/Y1731] Vlan id for chan, the vlan_id may be greater than 4095, please refer ctc_vlanptr_mode_t*/

    uint8   present_rdi;   /**< [GB.GG.D2.TM] Present rdi value*/
    uint8   d_unexp_mep;   /**< [GB.GG.D2.TM] Unexp mep defect*/
    uint8   d_mismerge;    /**< [GB.GG.D2.TM] Mismerge defect*/
    uint8   d_meg_lvl;     /**< [GB.GG.D2.TM] Cross-connect defect, low ccm*/

    uint8   level;         /**< [GB.GG.D2.TM] [1ag/Y1731] MD level*/
    uint8   is_up_mep;     /**< [GB.GG.D2.TM] [1ag/Y1731]Is upmep or down mep*/
    uint8   seq_num_en;    /**< [D2.TM] [1ag/Y1731] Sequnce number enable*/
    uint8   port_status_en;   /**< [D2.TM] Whether ccm tx with port status is enable*/

    uint8   if_status_en;     /**< [D2.TM] Whether ccm tx with interface status is enable*/
    uint8   sender_id_en;     /**< [D2.TM] Whether ccm tx with sender id is enable*/
    uint8   rsv1[2];
    uint32  ccm_tx_pkts;       /**< [D2.TM] CCM stats for TX pkts counter, only used for P2P mode*/
};
typedef struct ctc_oam_y1731_lmep_info_s ctc_oam_y1731_lmep_info_t;

/*
 @brief   Define Ether & MPLS-TP Y.1731 Remote MEP data structure get from both SDK database and ASIC dataset
*/
struct ctc_oam_y1731_rmep_info_s
{
    uint16  rmep_id;                    /**< [GB.GG.D2.TM] Remote mep id*/
    uint16  lmep_index;                 /**< [GB.GG.D2.TM] Local mep table index*/

    uint8   first_pkt_rx;               /**< [GB.GG.D2.TM] First packet receive flag*/
    uint8   active;                     /**< [GB.GG.D2.TM] Active falg in Remote mep*/
    uint8   last_rdi;                   /**< [GB.GG.D2.TM] Last rdi value*/
    uint8   d_loc;                      /**< [GB.GG.D2.TM] Dloc defect*/

    uint8   d_unexp_period;             /**< [GB.GG.D2.TM] Unexpect ccm period defect*/
    uint8   csf_en;                     /**< [GB.GG.D2.TM] CSF enable*/
    uint8   d_csf;                      /**< [GB.GG.D2.TM] CSF defect*/
    uint8   rx_csf_type;                /**< [GB.GG.D2.TM] Rx CSF type from remote*/

    uint8   is_p2p_mode;                /**< [GB.GG.D2.TM] [1ag/Y1731] P2P mode falg in Remote mep*/
    uint8   last_port_status;           /**< [D2.TM] [1ag] Last port status*/
    uint8   last_intf_status;           /**< [D2.TM] [1ag] Last interface status*/
    uint8   last_seq_chk_en;            /**< [D2.TM] [1ag] Last sequence number check enable*/
    uint32  seq_fail_count;             /**< [D2.TM] [1ag] Sequence number check fail counter(only valid if last_seq_chk_en)*/

    uint8   ma_sa_mismatch;             /**< [GB.GG.D2.TM] [1ag] Indicate configured rmep src mac and rx src mac is mismatch*/
    uint8   mac_status_change;          /**< [GB.GG.D2.TM] [1ag] Indicate rx mac status(port status or interface status) changed*/
    uint8   mac_sa[CTC_ETH_ADDR_LEN];   /**< [GB.GG.D2.TM] [1ag/Y1731] Mac sa value of remote mep*/
    uint32  dloc_time;                  /**< [GB.GG.D2.TM] The time value of detecting dLoc, uint:ms*/
    uint8   path_active;                /**< [D2.TM] Is the path in active or standby */
    uint8   protection_path;            /**< [D2.TM] Is the path in working path or protection path */
    uint8   path_fail;                  /**< [D2.TM] Signal fail status, if set, indicate the path detect SF */
    uint8   hw_aps_en;                  /**< [D2.TM] Is the chip enable automatical switching when SF is detected on the path */
    uint16  hw_aps_group_id;            /**< [D2.TM] APS Group ID, If 0, MEP is not associated with hw APS */
    uint8   rsv[2];
    uint32  ccm_rx_pkts;                 /**< [D2.TM] RX equal/low ccm counter, only used for P2P mode*/
    uint32  err_ccm_rx_pkts;             /**< [D2.TM] RX error ccm(unknown OpCode/slow OAM/macda check fail) counter, only used for P2P mode*/
};
typedef struct ctc_oam_y1731_rmep_info_s ctc_oam_y1731_rmep_info_t;

/**@} end of @defgroup  y1731_oam Y.1731_OAM */

#ifdef __cplusplus
}
#endif

#endif

