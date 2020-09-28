/**
 @file ctc_oam_bfd.h

 @date 2012-01-07

 @version v2.0

 This file contains BFD OAM(MPLS TP/MPLS/IP)related data structure, enum, macro.

*/

#ifndef _CTC_OAM_BFD_H
#define _CTC_OAM_BFD_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_const.h"
/****************************************************************
*
* Defines and Macros
*
****************************************************************/

#define CTC_OAM_BFD_CV_MEP_ID_MAX_LEN  36   /**< The max length of source MEP ID TLV */
#define CTC_OAM_BFD_TIMER_NEG_NUM  31   /**< The number of timer negotiation array */

/**
 @defgroup bfd_oam BFD_OAM
 @{
*/

/**
 @brief  Define BFD OAM state
*/
enum ctc_oam_bfd_state_e
{
    CTC_OAM_BFD_STATE_ADMIN_DOWN    = 0,    /**< [GB.GG.D2.TM] BFD session state admin down */
    CTC_OAM_BFD_STATE_DOWN          = 1,    /**< [GB.GG.D2.TM] BFD session state down */
    CTC_OAM_BFD_STATE_INIT          = 2,    /**< [GB.GG.D2.TM] BFD session state init */
    CTC_OAM_BFD_STATE_UP            = 3     /**< [GB.GG.D2.TM] BFD session state up */
};
typedef enum ctc_oam_bfd_state_e ctc_oam_bfd_state_t;

/**
 @brief  Define BFD OAM diag
*/
enum ctc_oam_bfd_diag_e
{
    CTC_OAM_BFD_DIAG_NONE                           = 0,    /**< [GB.GG.D2] BFD No Diagnostic */
    CTC_OAM_BFD_DIAG_TIME_EXPIRED                   = 1,    /**< [GB.GG.D2] BFD Control Detection Time Expired */
    CTC_OAM_BFD_DIAG_ECHO_FAIL                      = 2,    /**< [GB.GG.D2] BFD Echo Function Failed */
    CTC_OAM_BFD_DIAG_NEIGHBOR_DOWN                  = 3,    /**< [GB.GG.D2] BFD Neighbor Signaled Session Down */
    CTC_OAM_BFD_DIAG_FORWARDING_RESET               = 4,    /**< [GB.GG.D2] BFD Forwarding Plane Reset */
    CTC_OAM_BFD_DIAG_PATH_DOWN                      = 5,    /**< [GB.GG.D2] BFD Path Down */
    CTC_OAM_BFD_DIAG_CONCATENTED_PATH_DOWN          = 6,    /**< [GB.GG.D2] BFD Concatenated Path Down */
    CTC_OAM_BFD_DIAG_ADMINISTRATIVELY_DOWN          = 7,    /**< [GB.GG.D2] BFD Administratively Down */
    CTC_OAM_BFD_DIAG_REVERSE_CONCATENTED_PATH_DOWN  = 8,    /**< [GB.GG.D2] BFD Reverse Concatenated Path Down */
    CTC_OAM_BFD_DIAG_MIS_CONNECT                    = 9,    /**< [GB.GG.D2] BFD mis-connectivity defect */
    CTC_OAM_BFD_DIAG_MAX                            = 31
};
typedef enum ctc_oam_bfd_diag_e ctc_oam_bfd_diag_t;

/**
 @brief  Define BFD OAM timer negotiation param
*/
struct ctc_oam_bfd_timer_s
{
    uint16 min_interval; /**< [GB.GG.D2.TM] BFD min tx/rx interval */
    uint8  detect_mult;  /**< [GB.GG.D2.TM] BFD detection time multiplier */
    uint8  resv[1];
};
typedef struct ctc_oam_bfd_timer_s ctc_oam_bfd_timer_t;

/**
 @brief   Define BFD OAM local mep flag
*/
enum ctc_oam_bfd_lmep_flag_e
{
    CTC_OAM_BFD_LMEP_FLAG_NONE            = 0x00000000,  /**< [GB.GG.D2.TM] None */
    CTC_OAM_BFD_LMEP_FLAG_MEP_EN          = 0x00000001,  /**< [GB.GG.D2.TM] If set, indicate local mep is active */
    CTC_OAM_BFD_LMEP_FLAG_LM_EN           = 0x00000002,  /**< [GB.GG.D2.TM] [TP BFD] If set, indicate local mep enable LM */
    CTC_OAM_BFD_LMEP_FLAG_CSF_EN          = 0x00000004,  /**< [GB.GG.D2.TM] [TP BFD] If set, indicate CSF is enable */
    CTC_OAM_BFD_LMEP_FLAG_TX_CSF_EN       = 0x00000008,  /**< [GB.GG.D2.TM] [TP BFD] If set, indicate local mep enable TX CSF */
    CTC_OAM_BFD_LMEP_FLAG_MEP_ON_CPU      = 0x00000010,  /**< [GB.GG.D2.TM] If set, indicate local mep is proc by software not by asic */
    CTC_OAM_BFD_LMEP_FLAG_APS_EN          = 0x00000020,  /**< [GB] If set, indicate local mep aps message is enable */
    CTC_OAM_BFD_LMEP_FLAG_IP_SINGLE_HOP   = 0x00000040,  /**< [GB.GG.D2.TM] [IP BFD] If set, indicate local mep is single hop */
    CTC_OAM_BFD_LMEP_FLAG_WITHOUT_GAL     = 0x00000080,  /**< [GB.GG.D2.TM] [TP BFD] If set, indicate local mep is without GAL */
    CTC_OAM_BFD_LMEP_FLAG_TP_WITH_MEP_ID  = 0x00000100,  /**< [GB.GG.D2.TM] [TP BFD] If set, indicate local mep with source MEP ID tlv */
    CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV    = 0x00000200,  /**< [GB.GG.D2.TM] [TP BFD] If set, indicate local mep mpls PW MEP with no IP */
    CTC_OAM_BFD_LMEP_FLAG_IPV4_ENCAP      = 0x00000400,  /**< [GG.D2.TM] If set, indicate local mep with ipv4 address */
    CTC_OAM_BFD_LMEP_FLAG_IPV6_ENCAP            = 0x00000800,  /**< [GG.D2.TM] If set, indicate local mep with ipv6 address */
    CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV_IPV4    = 0x00001000,  /**< [GB.GG.D2.TM] If set, indicate local mep mpls PW MEP */
    CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV_IPV6    = 0x00002000,  /**< [GG.D2.TM] If set, indicate local mep mpls PW MEP */
    CTC_OAM_BFD_LMEP_FLAG_PROTECTION_PATH = 0x00004000,  /**< [GB.GG.D2.TM] [TP BFD] If set, indicate local mep config in aps protection path, else working path */
    CTC_OAM_BFD_LMEP_FLAG_SBFD_INITIATOR  = 0x00008000,   /**< [D2.TM] [SBFD] If set, indicate local mep is SBFDInitiator */
    CTC_OAM_BFD_LMEP_FLAG_SBFD_REFLECTOR  = 0x00010000,   /**< [D2.TM] [SBFD] If set, indicate local mep is SBFDReflector */
    CTC_OAM_BFD_LMEP_FLAG_BINDING_EN      = 0x00020000,   /**< [TM] If set, IPBFD/MicroBFD/SBFDInitiator check rx IPSA, LSP/VCCV BFD check rx mpls-in-label*/
    CTC_OAM_BFD_LMEP_FLAG_ALL             = 0x0001FFFF   /**< [GB.GG.D2.TM] All */
};
typedef enum ctc_oam_bfd_lmep_flag_e ctc_oam_bfd_lmep_flag_t;

/**
 @brief   Define BFD OAM  local mep  update type
*/
enum ctc_oam_bfd_lmep_update_type_e
{
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_NONE,                      /**< Update local mep none */
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_MEP_EN,                    /**< [GB.GG.D2.TM] mep enable*/
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_CC_EN,                     /**< [GB.GG.D2.TM] BFD(MPLS-TP cc) tx enable*/
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_CV_EN,                     /**< [GB.GG.D2.TM] MPLS-TP cv enable*/
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_COS_EXP,                /**< [GB.GG.D2.TM] transmit tos/exp/dscp value */
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_LMEP_SF_STATE,             /**< [GB.GG] Local signal fail state value */
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_CSF_EN,                    /**< [GB.GG.D2.TM] MPLS-TP BFD local mep process csf enable*/
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_CSF_EN,                 /**< [GB.GG.D2.TM] MPLS-TP BFD CSF tx enable */
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_CSF_TYPE,               /**< [GB.GG.D2.TM] CSF tx type , refer to ctc_oam_csf_type_t*/
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_DCSF,                      /**< [GB.GG.D2.TM] dcsf(CTC_OAM_DEFECT_CSF) bit set by software*/
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_LM_EN,                     /**< [GB.GG.D2.TM] LM enable */
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_LM_COS_TYPE,               /**< [GB.GG.D2.TM] LM cos type, refer to ctc_oam_lm_cos_type_t */
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_LM_COS,                    /**< [GB.GG.D2.TM] Special lm cos value */
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_TIMER,                  /**< [GB.GG.D2.TM] Update desired min tx interval
                                                                 & detection interval multiplier, p_update_value refer to ctc_oam_bfd_timer_t */
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_ACTUAL_TX_TIMER,           /**< [GB.GG.D2.TM] acutal tx interval*/
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_DIAG,                      /**< [GB.GG.D2.TM] local diagnostic code */
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_STATE,                     /**< [GB.GG.D2.TM] local  state */
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_DISCR,                     /**< [GB.GG.D2.TM] My discriminator */
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_NHOP,                      /**< [GB.GG.D2.TM] If set, will use NextHop to edit PDU */
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_LOCK,                      /**< [GG.D2.TM] [TP BFD] If set, enable MPLS-TP BFD lock */
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_MASTER_GCHIP,              /**< [GB.GG.D2.TM] MEP's master gchip ID */
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_TTL,                       /**< [GG.D2.TM] MEP's ttl, used for MPLS BFD and MPLS-TP BFD*/
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_RX_TIMER,               /**< [GB.GG.D2.TM] Update desired min tx interval and detection interval multiplier use ctc_oam_bfd_timer[0],
                                                                update required min rx interval and remote detection interval multiplier use ctc_oam_bfd_timer[1],
                                                                p_update_value refer to ctc_oam_bfd_timer_t*/
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_LABEL,                     /**< [D2.TM] Update Update MPLS-TP OAM lookup key, associated in Label*/
    CTC_OAM_BFD_LMEP_UPDATE_TYPE_MAX
};
typedef enum ctc_oam_bfd_lmep_update_type_e ctc_oam_bfd_lmep_update_type_t;

/**
 @brief   Define BFD OAM local mep
*/
struct ctc_oam_bfd_lmep_s
{
    uint32 flag;                    /**< [GB.GG.D2.TM] Refer to ctc_oam_bfd_lmep_flag_t */

    uint8  master_gchip;            /**< [GB.GG.D2.TM] MEP's master gchip ID, the MEP status is maintained on the master chip */
    uint8  local_diag;              /**< [GB.GG.D2.TM] Local diagnostic code, refer to ctc_oam_bfd_diag_t */
    uint8  local_state;             /**< [GB.GG.D2.TM] Local session state, refer to ctc_oam_bfd_state_t */
    uint8  local_detect_mult;       /**< [GB.GG.D2.TM] Local detection interval multiplier */

    uint32 local_discr;             /**< [GB.GG.D2.TM] My discriminator */

    uint16 desired_min_tx_interval; /**< [GB.GG.D2.TM] Desired local min tx interval */
    uint8  resv;
    uint8  mpls_spaceid;            /**< [GB.GG.D2.TM] MPLS Label space ID */

    uint8  mep_id[CTC_OAM_BFD_CV_MEP_ID_MAX_LEN];   /**< [GB.GG.D2.TM] Local mep MPLS-TP CC/CV TLV and source MEP ID */
    uint8  mep_id_len;                              /**< [GB.GG.D2.TM] Length of MPLS-TP CC/CV TLV and source MEP ID */
    uint8  lm_cos_type;             /**< [GB.GG.D2.TM] LM cos type, refer to ctc_oam_lm_cos_type_t */
    uint8  lm_cos;                  /**< [GB.GG.D2.TM] LM cos for lm cos type CTC_OAM_LM_COS_TYPE_SPECIFIED_COS */
    uint8  tx_csf_type;             /**< [GB.GG.D2.TM] Tx csf type, refer to ctc_oam_csf_type_t */

    uint8  tx_cos_exp;              /**< [GB.GG.D2.TM] Tx cos value*/
    uint8  ttl;                     /**< [GB.GG.D2.TM] Mpls ttl or ip ttl */
    uint16 bfd_src_port;            /**< [GB.GG.D2.TM] Source UDP port*/

    uint32 nhid;                    /**< [GB.GG.D2.TM] NextHop id */
    uint32 mpls_in_label;           /**< [TM] Only for VCCV/LSP BFD binding check*/

    ip_addr_t   ip4_sa;             /**< [GB.GG.D2.TM] IPv4 source address */
    ip_addr_t   ip4_da;             /**< [GG.D2.TM] IPv4 destination address */
    ipv6_addr_t ipv6_sa;            /**< [GG.D2.TM] IPv6 source address */
    ipv6_addr_t ipv6_da;            /**< [GG.D2.TM] IPv6 destination address */

};
typedef struct ctc_oam_bfd_lmep_s ctc_oam_bfd_lmep_t;

/**
 @brief   Define BFD OAM remote mep flag
*/
enum ctc_oam_bfd_rmep_flag_e
{
    CTC_OAM_BFD_RMEP_FLAG_NONE              = 0x00000000, /**< [GB.GG.D2.TM] None */
    CTC_OAM_BFD_RMEP_FLAG_MEP_EN            = 0x00000001, /**< [GB.GG.D2.TM] If set, indicate Remote mep is active */
    CTC_OAM_BFD_RMEP_FLAG_TP_WITH_MEP_ID    = 0x00000002, /**< [GG.D2.TM] If set, represent Remote mep id */
    CTC_OAM_BFD_RMEP_FLAG_ALL               = 0x00000003  /**< [GB.GG.D2.TM] All */
};
typedef enum ctc_oam_bfd_rmep_flag_e ctc_oam_bfd_rmep_flag_t;

/**
 @brief   Define BFD OAM remote mep update type
*/
enum ctc_oam_bfd_rmep_update_type_e
{
    CTC_OAM_BFD_RMEP_UPDATE_TYPE_NONE,              /**< [GB.GG.D2.TM] None */
    CTC_OAM_BFD_RMEP_UPDATE_TYPE_MEP_EN,            /**< [GB.GG.D2.TM] Remote mep enable*/
    CTC_OAM_BFD_RMEP_UPDATE_TYPE_RMEP_SF_STATE,     /**< [GB.GG] Remote mep signal fail state value */
    CTC_OAM_BFD_RMEP_UPDATE_TYPE_RX_TIMER,          /**< [GB.GG.D2.TM] Update required min rx interval
                                                                 &can not update remote detection interval multiplier, p_update_value refer to ctc_oam_bfd_timer_t */
    CTC_OAM_BFD_RMEP_UPDATE_TYPE_ACTUAL_RX_TIMER,   /**< [GB.GG.D2.TM] Acutal rx interval*/
    CTC_OAM_BFD_RMEP_UPDATE_TYPE_DIAG,              /**< [GB.GG.D2.TM] Remote diagnostic code, refer to ctc_oam_bfd_diag_t */
    CTC_OAM_BFD_RMEP_UPDATE_TYPE_STATE,             /**< [GB.GG.D2.TM] Remote session state, refer to ctc_oam_bfd_state_t */
    CTC_OAM_BFD_RMEP_UPDATE_TYPE_DISCR,             /**< [GB.GG.D2.TM] Remote mep discriminator */
    CTC_OAM_BFD_RMEP_UPDATE_TYPE_LEARN_EN,          /**< [D2.TM] Update remote mep learn enable*/
    CTC_OAM_BFD_RMEP_UPDATE_TYPE_HW_APS,            /**< [D2.TM] Associate Mep with hw APS configure info, refer to ctc_oam_hw_aps_t */
    CTC_OAM_BFD_RMEP_UPDATE_TYPE_HW_APS_EN,         /**< [D2.TM] If enabled, the chip will automatically switch when SF is detected on the path*/
    CTC_OAM_BFD_RMEP_UPDATE_TYPE_MAX
};
typedef enum ctc_oam_bfd_rmep_update_type_e ctc_oam_bfd_rmep_update_type_t;

/**
 @brief   Define BFD OAM remote mep
*/
struct ctc_oam_bfd_rmep_s
{
    uint32 flag;                        /**< [GB.GG.D2.TM] Refer to ctc_oam_bfd_rmep_flag_t */
    uint32 remote_discr;                /**< [GB.GG.D2.TM] Your discriminator */

    uint16 required_min_rx_interval;    /**< [GB.GG.D2.TM] Required remote min rx interval */
    uint8  remote_diag;                 /**< [GB.GG.D2.TM] Remote diagnostic code, refer to ctc_oam_bfd_diag_t */
    uint8  remote_state;                /**< [GB.GG.D2.TM] Remote session state, refer to ctc_oam_bfd_state_t */

    uint8  remote_detect_mult;          /**< [GB.GG.D2.TM] Remote detection interval multiplier */
    uint8  resv[3];

    uint8  mep_id[CTC_OAM_BFD_CV_MEP_ID_MAX_LEN];   /**< [GG.D2.TM] Remote mep MPLS-TP CC/CV TLV and source MEP ID */
    uint8  mep_id_len;                              /**< [GG.D2.TM] Length of MPLS-TP CC/CV TLV and source MEP ID */
};
typedef struct ctc_oam_bfd_rmep_s ctc_oam_bfd_rmep_t;



/**
 @brief   Define BFD timer negotiation capability array
*/
struct ctc_oam_bfd_timer_neg_s
{
    uint32 interval[CTC_OAM_BFD_TIMER_NEG_NUM];    /**< [GG.D2.TM] Interval array, unit: msec */
    uint8  interval_num;                           /**< [GG.D2.TM] Valid number of the array, include rx and tx */
    uint8  is_rx;                                  /**< [GG.D2.TM] If set, means config rx interval, else tx.
                                                                For goldengate, the rx/tx interval is a pair at the same array position*/
};
typedef struct ctc_oam_bfd_timer_neg_s ctc_oam_bfd_timer_neg_t;

/**
 @brief   Define BFD property configure type
*/
enum ctc_oam_bfd_cfg_type_e
{
    CTC_OAM_BFD_CFG_TYPE_SLOW_INTERVAL, /**< [GG.D2.TM] BFD actual tx/rx interval when establish session, unit: msec */
    CTC_OAM_BFD_CFG_TYPE_TIMER_NEG_INTERVAL, /**< [GG.D2.TM] BFD tx/rx timer negotiation capability array for hardware negotiate, refer to ctc_oam_bfd_timer_neg_t */
    CTC_OAM_BFD_CFG_TYPE_MAX
};
typedef enum ctc_oam_bfd_cfg_type_e ctc_oam_bfd_cfg_type_t;


/**
 @brief   Define BFD OAM property data structure
*/
struct ctc_oam_bfd_prop_s
{
    uint8    cfg_type;  /**< [GG.D2.TM] Config type, refer to ctc_oam_bfd_cfg_type_t */
    uint8    rsv[3];

    uint32   value;         /**< [GG.D2.TM] Configure value, set depend on cfg_type */
    void*    p_value;   /**< [GG.D2.TM] Configure value, set depend on cfg_type*/
};
typedef struct ctc_oam_bfd_prop_s  ctc_oam_bfd_prop_t;

/*
 @brief   Define BFD Local MEP data structure get from both SDK database and ASIC dataset
*/
struct ctc_oam_bfd_lmep_info_s
{
    uint8   mep_en;                     /**< [GB.GG.D2.TM] Active flag in local mep */
    uint8   cc_enable;                  /**< [GB.GG.D2.TM] BFD enable */
    uint8   cv_enable;                  /**< [GB.GG.D2.TM] BFD CV enable */
    uint8   resv;

    uint8   loacl_state;                /**< [GB.GG.D2.TM] Local mep state */
    uint8   local_diag;                 /**< [GB.GG.D2.TM] Local mep diagnostic code */
    uint8   local_detect_mult;          /**< [GB.GG.D2.TM] Local mep detection interval multiplier */

    uint32  local_discr;                /**< [GB.GG.D2.TM] Local mep discriminator */

    uint16  actual_tx_interval;         /**< [GB.GG.D2.TM] Local actual tx interval */
    uint16  desired_min_tx_interval;    /**< [GB.GG.D2.TM] Local desired min tx interval */

    uint8   mep_id[CTC_OAM_BFD_CV_MEP_ID_MAX_LEN];  /**< [GB.GG.D2.TM] Source mep id */

    uint8   single_hop;                 /**< [GB.GG.D2.TM] BFD session is single hop */
    uint8   csf_en;                     /**< [GB.GG.D2.TM] CSF enable */
    uint8   d_csf;                      /**< [GB.GG.D2.TM] CSF defect */
    uint8   rx_csf_type;                /**< [GB.GG.D2.TM] Rx CSF type from remote */
};
typedef struct ctc_oam_bfd_lmep_info_s ctc_oam_bfd_lmep_info_t;

/*
 @brief   Define BFD Remote MEP data structure get from both SDK database and ASIC dataset
*/
struct ctc_oam_bfd_rmep_info_s
{
    uint16  lmep_index;                 /**< [GB.GG.D2.TM] Local mep table index */
    uint8   first_pkt_rx;               /**< [GB.GG.D2.TM] First packet receive flag */
    uint8   mep_en;                     /**< [GB.GG.D2.TM] Active falg in Remote mep */

    uint8   mis_connect;                /**< [GB.GG.D2.TM] Mis-connect defect */
    uint8   remote_state;               /**< [GB.GG.D2.TM] Remote mep state */
    uint8   remote_diag;                /**< [GB.GG.D2.TM] Remote mep diagnostic code */
    uint8   remote_detect_mult;         /**< [GB.GG.D2.TM] Remote mep detection interval multiplier */

    uint32  remote_discr;               /**< [GB.GG.D2.TM] Remote mep discriminator */

    uint16  actual_rx_interval;         /**< [GB.GG.D2.TM] Actual local rx interval */
    uint16  required_min_rx_interval;   /**< [GB.GG.D2.TM] Required min rx interval */
    uint32  dloc_time;                  /**< [GB.GG.D2.TM] The time value of detecting Up to Down event, uint:ms */
    uint8   path_active;                /**< [D2.TM] Is the path in active or standby */
    uint8   protection_path;            /**< [D2.TM] Is the path in working path or protection path */
    uint8   path_fail;                  /**< [D2.TM] Signal fail status, if set, indicate the path detect SF */
    uint8   hw_aps_en;                  /**< [D2.TM] Is the chip enable automatical switching when SF is detected on the path */
    uint16  hw_aps_group_id;            /**< [D2.TM] APS Group ID, If 0, MEP is not associated with hw APS*/
    uint8   rsv[2];
};
typedef struct ctc_oam_bfd_rmep_info_s ctc_oam_bfd_rmep_info_t;

/**@} end of @defgroup bfd_oam BFD_OAM */

#ifdef __cplusplus
}
#endif

#endif

