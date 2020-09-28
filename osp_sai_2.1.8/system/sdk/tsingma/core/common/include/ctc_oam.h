/**
 @file ctc_oam.h

 @date 2010-1-19

 @version v2.0

 This file contains Ethernet OAM(80.21.ag/y1731) ,MPLS TP OAM(Y.1731), BFD (IP/MPLS/MPLS-TP) OAM related data structure, enum, macro.

*/

#ifndef _CTC_OAM_H
#define _CTC_OAM_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_const.h"
#include "common/include/ctc_linklist.h"
#include "common/include/ctc_oam_y1731.h"
#include "common/include/ctc_oam_bfd.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/**
 @defgroup oam OAM
 @{
*/
#define CTC_OAM_MAX_ERROR_CACHE_NUM             16      /**< [GB.GG.D2.TM]The max num of error cache */
#define CTC_OAM_EVENT_MAX_ENTRY_NUM             512     /**< [GB.GG.D2.TM]The max num of oam event for isr call back */
#define CTC_OAM_MAX_MAID_LEN                    48      /**< [GB.GG.D2.TM]The max length of MAID or MEGID */
#define CTC_OAM_STATS_COS_NUM                   8        /**< [GB.GG.D2.TM]The max num of COS based stats */
#define CTC_OAM_TRPT_SESSION_NUM                4       /**< [GB.GG]The max num of Throughput session,D2 NPM max session num is 8 */

/**
 @brief  Define oam exceptions
*/
enum ctc_oam_exceptions_e
{
    CTC_OAM_EXCP_INVALID_OAM_PDU                = 0,  /**< [GB.GG.D2.TM] Exception[0]: Individual OAM PDU */
    CTC_OAM_EXCP_SOME_RDI_DEFECT                = 1,  /**< [GB] Exception[1]: Some RDI defect */
    CTC_OAM_EXCP_ALL_DEFECT                     = 1,  /**< [GG.D2.TM] Exception[1]: All defect to CPU */
    CTC_OAM_EXCP_SOME_MAC_STATUS_DEFECT         = 2,  /**< [GB] Exception[2]: Some MAC status defect */
    CTC_OAM_EXCP_LBM                            = 2,  /**< [GG.D2.TM] Exception[2]: Ether LBM/LBR and MPLS-TP LBR to CPU */
    CTC_OAM_EXCP_HIGH_VER_OAM_TO_CPU            = 3,  /**< [GB.GG.D2.TM] Exception[3]: Higher version OAM to CPU */
    CTC_OAM_EXCP_RMEP_NOT_FOUND                 = 4,  /**< [GB] Exception[4]: RMEP not found */
    CTC_OAM_EXCP_TP_LBM                         = 4,  /**< [GG.D2.TM] Exception[4]: MPLS-TP LBM to CPU */
    CTC_OAM_EXCP_XCON_CCM_DEFECT                = 5,  /**< [GB] Exception[5]: cross-connect CCM defect */
    CTC_OAM_EXCP_TP_CSF                         = 5,  /**< [GG.D2.TM] Exception[5]: MPLS-TP CSF to CPU */
    CTC_OAM_EXCP_CCM_SEQ_NUM_ERROR              = 6,  /**< [GB] Exception[6]: CCM sequence number error */
    CTC_OAM_EXCP_ETH_SC_TO_CPU                  = 6,  /**< [D2.TM] Exception[6]: ETH SC PDU to CPU */
    CTC_OAM_EXCP_ETH_LL_TO_CPU                  = 7,  /**< [D2.TM] Exception[7]: ETH LL PDU to CPU */
    CTC_OAM_EXCP_CTRL_REG_CFG_ERROR             = 7,  /**< [GB] Exception[7]: Ds or Control Register configure error */
    CTC_OAM_EXCP_OPTION_CCM_TLV                 = 8,  /**< [GB.GG.D2.TM] Exception[8]: CCM has optional TLV */
    CTC_OAM_EXCP_SLOW_OAM_TO_CPU                = 9,  /**< [GB] Exception[9]: Slow OAM to CPU */
    CTC_OAM_EXCP_TP_FM                          = 9,  /**< [GG.D2.TM] Exception[9]: MPLS-TP FM to CPU */
    CTC_OAM_EXCP_ETH_TST_TO_CPU                 = 10, /**< [GB.GG.D2.TM] Exception[10]: Test PDU to CPU */
    CTC_OAM_EXCP_SM                             = 11, /**< [GG.D2.TM] Exception[11]: Ether SLM/SLR to CPU */
    CTC_OAM_EXCP_INVALID_BFD_MPLS_DM_DLM_PDU    = 11, /**< [GB] Exception[11]: Invalid BFD/MPLS-TP DM DLM PDU */
    CTC_OAM_EXCP_BIG_INTERVAL_OR_SW_MEP_TO_CPU  = 12, /**< [GB.GG.D2.TM] Exception[12]: Big CCM/BFD interval to CPU or MEP configured in software */
    CTC_OAM_EXCP_SMAC_MISMATCH                  = 13, /**< [GB] Exception[13]: Source MAC mismatch */
    CTC_OAM_EXCP_TP_BFD_CV                      = 13, /**< [GG.D2.TM] Exception[13]: TP BFD CV PDU to CPU */
    CTC_OAM_EXCP_MPLS_TP_DLM_TO_CPU             = 14, /**< [GB.GG.D2.TM] Exception[14]: MPLS-TP DLM to CPU */
    CTC_OAM_EXCP_MPLS_TP_DM_DLMDM_TO_CPU        = 15, /**< [GB.GG.D2.TM] Exception[15]: MPLS-TP DM/DLMDM to CPU */
    CTC_OAM_EXCP_APS_PDU_TO_CPU                 = 16, /**< [GB.GG.D2.TM] Exception[16]: APS PDU to CPU */
    CTC_OAM_EXCP_CCM_INTERVAL_MISMATCH          = 17, /**< [GB] Exception[17]: CCM interval mismatch */
    CTC_OAM_EXCP_UNKNOWN_PDU                    = 17, /**< [GG.D2.TM] Exception[17]: Unknown PDU to CPU */
    CTC_OAM_EXCP_DM_TO_CPU                      = 18, /**< [GB.GG.D2.TM] Exception[18]: DM to CPU */
    CTC_OAM_EXCP_LEARNING_CCM_TO_CPU            = 19, /**< [GG.D2.TM] Exception[19]: Learning CCM to CPU */
    CTC_OAM_EXCP_CSF_TO_CPU                     = 20, /**< [GB.GG.D2.TM] Exception[20]: Y1731 CSF PDU to CPU */
    CTC_OAM_EXCP_BFD_DISC_MISMATCH              = 21, /**< [GG.D2.TM] Exception[21]: BFD discreaminator mismatch to CPU */
    CTC_OAM_EXCP_MIP_RECEIVE_NON_PROCESS_PDU    = 21, /**< [GB] Exception[21]: MIP receive non process PDU */
    CTC_OAM_EXCP_MCC_PDU_TO_CPU                 = 22, /**< [GB.GG.D2.TM] Exception[22]: Y1731 MCC PDU to CPU */
    CTC_OAM_EXCP_PBT_MM_DEFECT_OAM_PDU          = 23, /**< [GB.GG.D2.TM] Exception[23]: PBT mmDefect OAM PDU to CPU */
    CTC_OAM_EXCP_EQUAL_LTM_LTR_TO_CPU           = 24, /**< [GB.GG.D2.TM] Exception[24]: Equal LTM/LTR to CPU */
    CTC_OAM_EXCP_EQUAL_LBR                      = 25, /**< [GB] Exception[25]: LBR to CPU */
    CTC_OAM_EXCP_ALL_TO_CPU                     = 25, /**< [D2.TM] Exception[25]: ALL OAM PDU to CPU */
    CTC_OAM_EXCP_LBM_MAC_DA_MEP_ID_CHECK_FAIL   = 26, /**< [GB] Exception[26]: LBM MAC DA for Ethernet or MEP ID for MPLS-TP check fail */
    CTC_OAM_EXCP_MACDA_CHK_FAIL                 = 26, /**< [D2.TM] Exception[26]: Ethernet OAM MAC DA check fail */
    CTC_OAM_EXCP_LM_TO_CPU                      = 27, /**< [GB.GG.D2.TM] Exception[27]: LM to CPU */
    CTC_OAM_EXCP_LEARNING_BFD_TO_CPU            = 28, /**< [GB.GG.D2.TM] Exception[28]: Learning BFD to CPU */
    CTC_OAM_EXCP_TWAMP_TO_CPU                   = 29, /**< [D2.TM] Exception[29]: TWAMP PDU to CPU */
    CTC_OAM_EXCP_BFD_TIMER_NEGOTIATION          = 30, /**< [GB.GG.D2.TM] Exception[30]: BFD timer negotiation */
    CTC_OAM_EXCP_SCC_PDU_TO_CPU                 = 31, /**< [GB.GG.D2.TM] Exception[31]: Y1731 SCC PDU to CPU */
    CTC_OAM_EXCP_TP_MCC = CTC_OAM_EXCP_MCC_PDU_TO_CPU,  /**< [GG.D2.TM] Exception[22]: Y1731 MCC PDU to CPU */
    CTC_OAM_EXCP_TP_SCC = CTC_OAM_EXCP_SCC_PDU_TO_CPU,  /**< [GG.D2.TM] Exception[31]: Y1731 SCC PDU to CPU */
};
typedef enum ctc_oam_exceptions_e ctc_oam_exceptions_t;

/**
 @brief  Define defects causing aps signal fail message
*/

enum ctc_oam_trig_aps_msg_flag_e
{
    CTC_OAM_TRIG_APS_MSG_FLAG_NONE                = 0x00000000, /**< [GB] None */
    CTC_OAM_TRIG_APS_MSG_FLAG_INTF_STATE          = 0x00000001, /**< [GB] Interface status */
    CTC_OAM_TRIG_APS_MSG_FLAG_PORT_STATE          = 0x00000002, /**< [GB] Port status */
    CTC_OAM_TRIG_APS_MSG_FLAG_RDI                 = 0x00000004, /**< [GB] Defect RDI */
    CTC_OAM_TRIG_APS_MSG_FLAG_D_LOC               = 0x00000008, /**< [GB] Defect loss of connectivity */
    CTC_OAM_TRIG_APS_MSG_FLAG_D_UNEXP_PERIOD      = 0x00000010, /**< [GB] Defect unexpected period */
    CTC_OAM_TRIG_APS_MSG_FLAG_D_MEG_LVL           = 0x00000020, /**< [GB] Defect unexpected MEG level */
    CTC_OAM_TRIG_APS_MSG_FLAG_D_MISMERGE          = 0x00000040, /**< [GB] Defect mismerge */
    CTC_OAM_TRIG_APS_MSG_FLAG_D_UNEXP_MEP         = 0x00000080, /**< [GB] Defect unexpected MEP */

    CTC_OAM_TRIG_APS_MSG_FLAG_BFD_UP_TO_DOWN      = 0x00000100, /**< [GB] BFD state up to down */
    CTC_OAM_TRIG_APS_MSG_FLAG_ALL                 = 0x000001FF  /**< [GB] All */

};
typedef enum ctc_oam_trig_aps_msg_flag_e ctc_oam_trig_aps_msg_flag_t;

/**
 @brief   Define Mep Type
*/
enum ctc_oam_mep_type_e
{
    /*Y.1731*/
    CTC_OAM_MEP_TYPE_ETH_1AG,           /**< [GB.GG.D2.TM] Ethernet 802.1ag  mep */
    CTC_OAM_MEP_TYPE_ETH_Y1731,         /**< [GB.GG.D2.TM] Ethernet Y1731    mep */
    CTC_OAM_MEP_TYPE_MPLS_TP_Y1731,     /**< [GB.GG.D2.TM] MPLS-TP Y.1731    mep */
    /*BFD*/
    CTC_OAM_MEP_TYPE_IP_BFD,            /**< [GB.GG.D2.TM] Ipv4/ipv6 bfd mep */
    CTC_OAM_MEP_TYPE_MPLS_BFD,          /**< [GB.GG.D2.TM] MPLS BFD mep */
    CTC_OAM_MEP_TYPE_MPLS_TP_BFD,       /**< [GB.GG.D2.TM] MPLS-TP BFD mep */
    CTC_OAM_MEP_TYPE_MICRO_BFD,         /**< [D2.TM] Micro-BFD mep */

    CTC_OAM_MEP_TYPE_MAX
};
typedef enum ctc_oam_mep_type_e ctc_oam_mep_type_t;

/**
 @brief   Define oam property Type
*/
enum ctc_oam_property_type_e
{
    CTC_OAM_PROPERTY_TYPE_COMMON,       /**< [GB.GG.D2.TM] Common OAM property */
    CTC_OAM_PROPERTY_TYPE_Y1731,        /**< [GB.GG.D2.TM] Y.1731 OAM property */
    CTC_OAM_PROPERTY_TYPE_BFD,          /**< [GB.GG.D2.TM] BFD OAM property */
    CTC_OAM_PROPERTY_TYPE_MAX
};
typedef enum ctc_oam_property_type_e ctc_oam_property_type_t;

/**
 @brief   Define LM Type
*/
enum ctc_oam_lm_type_e
{
    CTC_OAM_LM_TYPE_NONE,           /**< [GB.GG.D2.TM] No Lm enable */
    CTC_OAM_LM_TYPE_DUAL,           /**< [GB.GG.D2.TM] Dual-ended Lm */
    CTC_OAM_LM_TYPE_SINGLE,         /**< [GB.GG.D2.TM] Single-ended Lm */
    CTC_OAM_LM_TYPE_MAX
};
typedef enum ctc_oam_lm_type_e ctc_oam_lm_type_t;

/**
 @brief   Define LM cos Type
*/
enum ctc_oam_lm_cos_type_e
{
    CTC_OAM_LM_COS_TYPE_ALL_COS,            /**< [GB.GG.D2.TM] All cos data counter together */
    CTC_OAM_LM_COS_TYPE_SPECIFIED_COS,      /**< [GB.GG.D2.TM] Only count specified cos data */
    CTC_OAM_LM_COS_TYPE_PER_COS,            /**< [GB.GG.D2.TM] Per cos date to count */
    CTC_OAM_LM_COS_TYPE_MAX
};
typedef enum ctc_oam_lm_cos_type_e ctc_oam_lm_cos_type_t;

/**
 @brief  Define CSF Type
*/
enum ctc_oam_csf_type_e
{
    CTC_OAM_CSF_TYPE_LOS,        /**< [GB.GG.D2.TM] Client loss of signal */
    CTC_OAM_CSF_TYPE_FDI,        /**< [GB.GG.D2.TM] Client forward defect indication */
    CTC_OAM_CSF_TYPE_RDI,        /**< [GB.GG.D2.TM] Client remote defect indication */
    CTC_OAM_CSF_TYPE_DCI,        /**< [GB.GG.D2.TM] Client defect clear indication */
    CTC_OAM_CSF_TYPE_MAX
};
typedef enum ctc_oam_csf_type_e ctc_oam_csf_type_t;

/**
 @brief   Define the defect cache info
 */
struct ctc_oam_defect_info_s
{
    uint8  valid_cache_num;                                 /**< [GB.GG.D2.TM] Indicated valid cache number */
    uint8  rsv0[3];
    uint16 mep_index_base[CTC_OAM_MAX_ERROR_CACHE_NUM];     /**< [GB.GG.D2.TM] MEP index base in error cache */
    uint32 mep_index_bitmap[CTC_OAM_MAX_ERROR_CACHE_NUM];   /**< [GB.GG.D2.TM] MEP bitmap from start MEP index base */
};
typedef struct ctc_oam_defect_info_s ctc_oam_defect_info_t;


/**
 @brief Define OAM defect type for RDI & CPU
*/
enum ctc_oam_defect_s
{
    /*Y.1731*/
    CTC_OAM_DEFECT_NONE                         = 0x00000, /**< [GB.GG.D2.TM] None */
    CTC_OAM_DEFECT_MAC_STATUS_CHANGE            = 0x00001, /**< [GB.GG.D2.TM] Some mac status defect */
    CTC_OAM_DEFECT_SRC_MAC_MISMATCH             = 0x00002, /**< [GB.GG.D2.TM] Remote mep mac not match defect */
    CTC_OAM_DEFECT_MISMERGE                     = 0x00004, /**< [GB.GG.D2.TM] Mismerge ccm defect */
    CTC_OAM_DEFECT_UNEXPECTED_LEVEL             = 0x00008, /**< [GB.GG.D2.TM] Level cross connect ccm defect */
    CTC_OAM_DEFECT_UNEXPECTED_MEP               = 0x00010, /**< [GB.GG.D2.TM] Unexpected MEP defect */
    CTC_OAM_DEFECT_UNEXPECTED_PERIOD            = 0x00020, /**< [GB.GG.D2.TM] Unexpected period defect */
    CTC_OAM_DEFECT_DLOC                         = 0x00040, /**< [GB.GG.D2.TM] LOC defect */
    CTC_OAM_DEFECT_CSF                          = 0x00080, /**< [GB.GG.D2.TM] CSF defect */

    CTC_OAM_DEFECT_EVENT_RX_FIRST_PKT           = 0x00100, /**< [GB.GG.D2.TM] Rx first CCM Packet defect, no trigger rdi */
    CTC_OAM_DEFECT_EVENT_RDI_RX                 = 0x00200, /**< [GB.GG.D2.TM] RDI rx defect, no trigger rdi */
    CTC_OAM_DEFECT_EVENT_RDI_TX                 = 0x00400, /**< [GB.GG.D2.TM] RDI tx defect, no trigger rdi */
    /*BFD*/
    CTC_OAM_DEFECT_EVENT_BFD_DOWN               = 0x00800, /**< [GB.GG.D2.TM] BFD down defect, no trigger rdi */
    CTC_OAM_DEFECT_EVENT_BFD_INIT               = 0x01000, /**< [GB.GG.D2.TM] BFD init defect, no trigger rdi */
    CTC_OAM_DEFECT_EVENT_BFD_UP                 = 0x02000, /**< [GB.GG.D2.TM] BFD up defect, no trigger rdi */
    CTC_OAM_DEFECT_EVENT_BFD_MIS_CONNECT        = 0x04000, /**< [GB.GG.D2.TM] BFD mis connect defect, no trigger rdi */

    CTC_OAM_DEFECT_SF                           = 0x08000, /**< [D2.TM] working/protection path generate signal fail, mapping from other defect
                                                               status using CTC_OAM_COM_PRO_TYPE_DEFECT_TO_SF */
    CTC_OAM_DEFECT_EVENT_BFD_TIMER_NEG_SUCCESS  = 0x10000, /**< [TM] BFD timer negotiation success defect, no trigger rdi */
    CTC_OAM_DEFECT_ALL                          = 0xFFFFF  /**< [GB.GG.D2.TM] All */
};
typedef enum ctc_oam_defect_s ctc_oam_defect_t;

/**
 @brief  Define OAM maid length mode
*/
enum ctc_oam_maid_len_format_s
{
    CTC_OAM_MAID_LEN_16BYTES = 1,   /**< [GB.GG.D2.TM] Length of MAID is 16 Bytes */
    CTC_OAM_MAID_LEN_32BYTES,       /**< [GB.GG.D2.TM] Length of MAID is 32 Bytes */
    CTC_OAM_MAID_LEN_48BYTES        /**< [GB.GG.D2.TM] Length of MAID is 48 Bytes */
};
typedef enum ctc_oam_maid_len_format_s ctc_oam_maid_len_format_t;

/**********************************************************************************************
------------------------- Ether OAM (support IEEE 802.1ag & Y.1731 OAM)-----------------------
**********************************************************************************************/
/**
 @defgroup eth_oam ETH_OAM
 @{
*/

/**
 @brief  Define Ethernet OAM key
*/
struct ctc_oam_eth_key_s
{
    uint16    vlan_id;          /**< [GB.GG.D2.TM] Ethernet OAM channel associated Vlan ID(if set 0, means invalid),
                                      the vlan_id may be greater than 4095, refer ctc_vlanptr_mode_t */
    uint16    cvlan_id;         /**< [D2.TM] Ethernet OAM channel associated cvlan id (if set 0, means invalid)*/
    uint8     md_level;         /**< [GB.GG.D2.TM] Ethernet OAM MD level */
    uint32    gport;            /**< [GB.GG.D2.TM] Ethernet OAM channel associated global port id,if MEP configured on linkagg,the gport should global linkagg ID */

    uint16    l2vpn_oam_id;     /**< [GG.D2.TM] VPWS OAM id or VPLS fid or QINQ fid, used in Port + Fid mode */
    uint8     rsv0[2];
};
typedef struct ctc_oam_eth_key_s ctc_oam_eth_key_t;


/**@}end of @defgroup  eth_oam ETH_OAM */

/**********************************************************************************************
----------------------------- MPLS-TP Y.1731 OAM ---------------------------
**********************************************************************************************/
/**
 @defgroup mpls_tp_Y.1731_oam MPLS_TP_Y.1731_OAM
 @{
*/
/**
 @brief  Define MPLS-TP Y.1731 & BFD OAM key
*/
struct ctc_oam_tp_key_s
{
    uint32 label;               /**< [GB.GG.D2.TM] MPLS-TP OAM lookup key, associated in Label */
    uint32 gport_or_l3if_id;    /**< [GB.GG.D2.TM] MPLS-TP Section OAM lookup key, refer to ctc_oam_global_t.tp_section_oam_based_l3if */
    uint8  resv1[3];
    uint8  mpls_spaceid;        /**< [GB.GG.D2.TM] MPLS Label space ID */
};
typedef struct ctc_oam_tp_key_s ctc_oam_tp_key_t;

/**@}end of @defgroup  mpls_tp_Y.1731_oam MPLS_TP_Y.1731_OAM */

/**
 @brief  Define IP/MPLS BFD OAM key
*/
struct ctc_oam_bfd_key_s
{
    uint32  discr;              /**< [GB.GG.D2.TM] IP/MPLS BFD OAM lookup key, discriminator */
};
typedef struct ctc_oam_bfd_key_s ctc_oam_bfd_key_t;

/*
 @brief   Define oam global data structure
*/
struct ctc_oam_global_s
{
    uint8      maid_len_format;                 /**< [GB.GG.D2.TM] MAID length format, refer to ctc_oam_maid_len_format_t */
    uint16     tp_y1731_ach_chan_type;          /**< [GB.GG.D2.TM] TP Y.1731 ACH channel type, default 0x8902*/

    uint16     tp_csf_ach_chan_type;            /**< [GB.GG.D2.TM] ACH channel type for CSF */
    uint8      tp_csf_fdi;                      /**< [GB.GG.D2.TM] FDI field of CSF */
    uint8      tp_csf_rdi;                      /**< [GB.GG.D2.TM] RDI field of CSF */

    uint8      tp_csf_los;                      /**< [GB.GG.D2.TM] LOS field of CSF */
    uint8      tp_csf_clear;                    /**< [GB.GG.D2.TM] CLEAR field of CSF */
    uint8      mep_index_alloc_by_sdk;          /**< [GB.GG.D2.TM] MEP resource allocated by SDK or system */
    uint8      tp_section_oam_based_l3if;       /**< [GB.GG.D2.TM] If set, TP section OAM lookup key based on l3 interface ID, otherwise based on port */

    uint8      tp_bfd_333ms;                    /**< [GB.GG] TP BFD OAM use Y.1731 interval mode, default to 0 means 1ms interval mode */
    uint16     mep_1ms_num;                     /**< [GB] OAM session range for 1ms interval OAM, such as IP BFD, MPLS BFD, mep means session */
    uint8      mpls_pw_vccv_with_ip_en;         /**< [GB] If set, will support MPLS PW VCCV with IP BFD */
};
typedef struct ctc_oam_global_s ctc_oam_global_t;

enum ctc_oam_com_property_cfg_type_e
{
    CTC_OAM_COM_PRO_TYPE_DEFECT_TO_CPU,         /**< [GB.GG.D2.TM] If corresponding defect set, the defect will be reported to CPU, refer to ctc_oam_defect_t */
    CTC_OAM_COM_PRO_TYPE_DEFECT_TO_RDI,         /**< [GB.GG.D2.TM] If corresponding defect set, the defect will trigger RDI , refer to ctc_oam_defect_t */
    CTC_OAM_COM_PRO_TYPE_EXCEPTION_TO_CPU,      /**< [GB.GG.D2.TM] Indicate corresponding OAM exception to CPU, refer to ctc_oam_exceptions_t */
    CTC_OAM_COM_PRO_TYPE_TRIG_APS_MSG,          /**< [GB] Indicated to set APS message trigger condition, ctc_oam_trig_aps_msg_flag_t */
    CTC_OAM_COM_PRO_TYPE_TIMER_DISABLE,         /**< [GB.GG.D2.TM]Stop OAM TIMER and close tx/rx state machine */
    CTC_OAM_COM_PRO_TYPE_MASTER_GCHIP,          /**< [GB.GG.D2.TM] Update ALL existing MEPs' master gchip ID */
    CTC_OAM_COM_PRO_TYPE_DEFECT_TO_SF,          /**< [D2.TM] Mapping some defect to SF, refer to ctc_oam_defect_t */
    CTC_OAM_COM_PRO_TYPE_HW_APS_MODE            /**< [D2.TM] When transport path (working path/protection path) detect SF, If transport path enable hardware based aps:
                                              mode 0 -- the chip only automatically switch once, and then turn off the automatic switching function.
                                              mode 1 -- the chip will automatically switch (the chip can sel-repair).*/
};
typedef enum ctc_oam_com_property_cfg_type_e ctc_oam_com_property_cfg_type_t;

/*
 @brief   Define oam com property data structure
*/
struct ctc_oam_com_property_s
{
    uint32           cfg_type;          /**< [GB.GG.D2.TM] Global common property, refer to ctc_oam_com_property_cfg_type_t */
    uint32           value;             /**< [GB.GG.D2.TM] According cfg type to use */
};
typedef struct ctc_oam_com_property_s ctc_oam_com_property_t;

/*
 @brief   Define oam property data structure
*/
struct ctc_oam_property_s
{
    uint8 oam_pro_type;                         /**< [GB.GG.D2.TM] OAM property type, refer to ctc_oam_property_type_t */
    union
    {
        ctc_oam_com_property_t  common;         /**< [GB.GG.D2.TM] Common oam property */
        ctc_oam_y1731_prop_t    y1731;          /**< [GB.GG.D2.TM] Ether oam & MPLS-TP Y.1731 property */
        ctc_oam_bfd_prop_t      bfd;            /**< [GB.GG.D2.TM] IP/MPLS/MPLS-TP BFD property */
    } u;
};
typedef struct ctc_oam_property_s ctc_oam_property_t;

/*
 @brief   Define oam key flags
*/
enum ctc_oam_key_flag_e
{
    CTC_OAM_KEY_FLAG_LINK_SECTION_OAM   = 1U << 0,   /**< [GB.GG.D2.TM] Eth link or MPLS-TP Section OAM */
    CTC_OAM_KEY_FLAG_VPLS               = 1U << 1,   /**< [GB.GG.D2.TM] If set, indicate VPLS MEP, otherwise VPWS MEP*/
    CTC_OAM_KEY_FLAG_UP_MEP             = 1U << 2,   /**< [GB.GG.D2.TM] Indicate UP MEP */
    CTC_OAM_KEY_FLAG_L2VPN              = 1U << 3,   /**< [GG.D2.TM] Indicate L2VPN MEP */
    CTC_OAM_KEY_FLAG_BASE_FID           = CTC_OAM_KEY_FLAG_L2VPN,   /**< [D2.TM] Indicate fid based MEP */
    CTC_OAM_KEY_FLAG_CVLAN_DOMAIN       = 1U << 4,   /**< [D2.TM] Indicate mep in cvlan domain */
    CTC_OAM_KEY_FLAG_SBFD_REFLECTOR     = 1U << 5,   /**< [TM] Indicate key for sbfd reflector */

    CTC_OAM_KEY_FLAG_MAX
};
typedef enum ctc_oam_key_flag_e ctc_oam_key_flag_t;

/*
 @brief   Define oam key data structure
*/
struct ctc_oam_key_s
{
    ctc_oam_mep_type_t mep_type;  /**< [GB.GG.D2.TM] Mep type, refer to ctc_oam_mep_type_t */
    uint32             flag;      /**< [GB.GG.D2.TM] OAM key flag, refer to ctc_oam_key_flag_t */
    union
    {
        ctc_oam_eth_key_t eth;    /**< [GB.GG.D2.TM] Ether OAM key */
        ctc_oam_tp_key_t  tp;     /**< [GB.GG.D2.TM] MPLS-TP Y.1731 & BFD OAM key */
        ctc_oam_bfd_key_t bfd;    /**< [GB.GG.D2.TM] MPLS/IP BFD OAM key */
    } u;
};
typedef struct ctc_oam_key_s ctc_oam_key_t;

/*
 @brief   Define oam maid data structure
*/
struct ctc_oam_maid_s
{
    ctc_oam_mep_type_t  mep_type;                       /**< [GB.GG.D2.TM] Mep type, refer to ctc_oam_mep_type_t */

    uint8               maid[CTC_OAM_MAX_MAID_LEN];     /**< [GB.GG.D2.TM] MAID or MEGID  value */
    uint8               maid_len;                       /**< [GB.GG.D2.TM] MAID or MEGID  length */
    uint8               rsv0[3];
};
typedef struct ctc_oam_maid_s ctc_oam_maid_t;


/*
 @brief   Define oam local mep data structure
*/
struct ctc_oam_lmep_s
{
    ctc_oam_key_t  key;                         /**< [GB.GG.D2.TM] OAM lookup key */
    ctc_oam_maid_t maid;                        /**< [GB.GG.D2.TM] OAM maid */
    union
    {
        ctc_oam_y1731_lmep_t     y1731_lmep;    /**< [GB.GG.D2.TM] Ether OAM & TP Y.1731 local mep */
        ctc_oam_bfd_lmep_t       bfd_lmep;      /**< [GB.GG.D2.TM] BFD OAM local mep */
    } u;
    uint32 lmep_index;                          /**< [GB.GG.D2.TM] Local mep index allocated by system */
};
typedef struct ctc_oam_lmep_s ctc_oam_lmep_t;

/*
 @brief   Define oam remote mep data structure
*/
struct ctc_oam_rmep_s
{
    ctc_oam_key_t key;                              /**< [GB.GG.D2.TM] OAM lookup key */
    union
    {
        ctc_oam_y1731_rmep_t      y1731_rmep;       /**< [GB.GG.D2.TM] Ether OAM & MPLS-TP Y.1731 remote mep */
        ctc_oam_bfd_rmep_t        bfd_rmep;         /**< [GB.GG.D2.TM] BFD OAM remote mep */
    } u;
    uint32 rmep_index;                              /**< [GB.GG.D2.TM] Remote mep index allocated by system, only used for P2MP mode*/
};
typedef struct ctc_oam_rmep_s ctc_oam_rmep_t;

/*
 @brief   Define oam mip data structure
*/
struct ctc_oam_mip_s
{
    ctc_oam_key_t key;                  /**< [GB.GG.D2.TM] OAM lookup key */
    uint8  master_gchip;                /**< [GB.GG.D2.TM] MEP's master gchip ID, the MEP status is maintained on the master chip */
};
typedef struct ctc_oam_mip_s ctc_oam_mip_t;

struct ctc_oam_update_s
{
    ctc_oam_key_t key;                  /**< [GB.GG.D2.TM] OAM lookup key */
    uint32  update_type;                /**< [GB.GG.D2.TM] According mep type to update ctc_oam_y1731_lmep_update_type_t,
                                                                            ctc_oam_y1731_rmep_update_type_t,
                                                                            ctc_oam_bfd_lmep_update_type_t,
                                                                            ctc_oam_bfd_rmep_update_type_t */
    uint32  update_value;               /**< [GB.GG.D2.TM] Update value */
    uint8   is_local;                   /**< [GB.GG.D2.TM] Indicate update local or remote mep */
    uint8   resv0;
    uint16  rmep_id;                    /**< [GB.GG.D2.TM] Remote mep mep id, for update remote mep only */
    void*   p_update_value;             /**< [GB.GG.D2.TM] Update values */
};
typedef struct ctc_oam_update_s ctc_oam_update_t;

/*
 @brief   Define Local MEP data structure get from both SDK database and ASIC dataset
*/
union ctc_oam_lmep_info_u
{
    ctc_oam_y1731_lmep_info_t       y1731_lmep;     /**< [GB.GG.D2.TM] Local Eth MEP & MPLS-TP Y.1731 MEP from database */
    ctc_oam_bfd_lmep_info_t         bfd_lmep;       /**< [GB.GG.D2.TM] Local BFD MEP from database */
};
typedef union ctc_oam_lmep_info_u ctc_oam_lmep_info_t;

/*
 @brief   Define Remote MEP data structure get from both SDK database and ASIC dataset
*/
union ctc_oam_rmep_info_u
{
    ctc_oam_y1731_rmep_info_t       y1731_rmep;     /**< [GB.GG.D2.TM] Remote ETH MEP & MPLS-TP Y.1731 MEP from database */
    ctc_oam_bfd_rmep_info_t         bfd_rmep;       /**< [GB.GG.D2.TM] Remote BFD MEP from database */
};
typedef union ctc_oam_rmep_info_u ctc_oam_rmep_info_t;

/*
 @brief   Define MEP data structure get from both SDK database and ASIC dataset
*/
struct ctc_oam_mep_info_s
{
    uint16    mep_index;                /**< [GB.GG.D2.TM] The index of MEP, input parameter */
    uint8     is_rmep;                  /**< [GB.GG.D2.TM] Indicate index of MEP is Remote MEP */
    uint8     resv[1];
    ctc_oam_mep_type_t   mep_type;      /**< [GB.GG.D2.TM] Mep type, refer to ctc_oam_mep_type_t */
    ctc_oam_lmep_info_t  lmep;          /**< [GB.GG.D2.TM] Local MEP from database,*/
    ctc_oam_rmep_info_t  rmep;          /**< [GB.GG.D2.TM] Remote MEP from database (P2MP can only get lmep info)*/
};
typedef struct ctc_oam_mep_info_s ctc_oam_mep_info_t;


/*
 @brief   Define MEP data structure get from both SDK database and ASIC dataset
*/
struct ctc_oam_mep_info_with_key_s
{
    ctc_oam_key_t  key;                 /**< [GB.GG.D2.TM] OAM lookup key information */
    uint16 rmep_id;                     /**< [GB.GG.D2.TM] OAM remote mep id */
    ctc_oam_lmep_info_t  lmep;          /**< [GB.GG.D2.TM] Local MEP from database */
    ctc_oam_rmep_info_t  rmep;          /**< [GB.GG.D2.TM] Remote MEP from database (P2MP can only get lmep info) */
};
typedef struct ctc_oam_mep_info_with_key_s ctc_oam_mep_info_with_key_t;

/*
 @brief   Define ccm lm counter structure
*/
struct ctc_oam_lm_info_s
{
    uint32 tx_fcf;          /**< [GB.GG.D2.TM] Value of the local counter TxFCl at the time of transmission of the CCM frame */
    uint32 rx_fcb;          /**< [GB.GG.D2.TM] Value of the local counter RxFCl at the time of reception of the last CCM
                                  frame from the peer MEP */
    uint32 tx_fcb;          /**< [GB.GG.D2.TM] Value of TxFCf in the last received CCM frame from the peer MEP */
    uint32 rx_fcl;          /**< [GB.GG.D2.TM] Value of local counter RxFCl at time this CCM frame was received */
};
typedef struct ctc_oam_lm_info_s ctc_oam_lm_info_t;

/*
 @brief   Define Autogen stats structure
*/
struct ctc_oam_trpt_stats_s
{
    uint64 tx_pkt;         /**< [GB.GG.D2.TM] Throughput packet cnt stats in Tx Dir */
    uint64 tx_oct;         /**< [GB.GG.D2.TM] Throughput packet octets stats in Tx Dir */
    uint64 rx_pkt;         /**< [GB.GG.D2.TM] Throughput packet cnt stats in Rx Dir */
    uint64 rx_oct;         /**< [GB.GG.D2.TM] Throughput packet octets stats in Rx Dir */
    uint64 tx_fcf;         /**< [GG.D2.TM] Tx fcf in SLM */
    uint64 tx_fcb;         /**< [GG.D2.TM] Tx fcb in SLM */
    uint64 rx_fcl;         /**< [GG.D2.TM] Rx fcl in SLM */
};
typedef struct ctc_oam_trpt_stats_s ctc_oam_trpt_stats_t;

/*
 @brief   Define MEP stats structure get from ASIC dataset
*/
struct ctc_oam_stats_info_s
{
    ctc_oam_key_t  key;                                     /**< [GB.GG.D2.TM] OAM lookup key */
    ctc_oam_lm_info_t lm_info[CTC_OAM_STATS_COS_NUM];       /**< [GB.GG.D2.TM] LM counter in ccm */
    uint8 lm_type;                                          /**< [GB.GG.D2.TM] MEP LM type, refer to ctc_oam_lm_type_t */
    uint8 lm_cos_type;                                      /**< [GB.GG.D2.TM] MEP LM cos type, refer to ctc_oam_lm_cos_type_t */
    uint8 resv[2];
};
typedef struct ctc_oam_stats_info_s ctc_oam_stats_info_t;

/*
 @brief   Define OAM defect information entry
*/
struct ctc_oam_event_entry_s
{
    ctc_oam_mep_type_t mep_type;            /**< [GB.GG.D2.TM] Mep type , refer to ctc_oam_mep_type_t */
    uint8   maid[CTC_OAM_MAX_MAID_LEN];     /**< [GB.GG.D2.TM] MAID or MEGID  value */
    uint16  lmep_id;                        /**< [GB.GG.D2.TM] Local MEP ID */
    uint16  rmep_id;                        /**< [GB.GG.D2.TM] Remote MEP ID, valid only is_remote set */
    ctc_oam_key_t  oam_key;                 /**< [GB.GG.D2.TM] OAM lookup key information */

    uint16  lmep_index;                     /**< [GB.GG.D2.TM] Local MEP Index */
    uint16  rmep_index;                     /**< [GB.GG.D2.TM] Remote MEP Index, valid only is_remote set */
    uint8   is_remote;                      /**< [GB.GG.D2.TM] Indicated event is triggered by local or remote MEP */
    uint8   resv[3];
    uint32  event_bmp;                      /**< [GB.GG.D2.TM] Defect bitmap ,refer to ctc_oam_defect_t */
    uint8   local_state;                    /**< [GB.GG.D2.TM] BFD session local mep state*/
    uint8   local_diag;                     /**< [GB.GG.D2.TM] BFD session local mep diagnostic code*/
    uint8   remote_state;                   /**< [GB.GG.D2.TM] BFD session remote mep state*/
    uint8   remote_diag;                    /**< [GB.GG.D2.TM] BFD session remote mep diagnostic code*/
};
typedef struct ctc_oam_event_entry_s ctc_oam_event_entry_t;

/*
 @brief   Define OAM defect event information for interrput call back function
*/
struct ctc_oam_event_s
{
    uint16 valid_entry_num;                                             /**< [GB.GG.D2.TM] Indicated valid event entry num */
    uint8  resv0[2];
    ctc_oam_event_entry_t oam_event_entry[CTC_OAM_EVENT_MAX_ENTRY_NUM]; /**< [GB.GG.D2.TM] Event entry ,refer to ctc_oam_event_entry_t */
};
typedef struct ctc_oam_event_s ctc_oam_event_t;

/*
 @brief   Define OAM throughput pattern type
*/
enum ctc_oam_trpt_pattern_type_e
{
    CTC_OAM_PATTERN_REPEAT_TYPE,       /**< [GB.GG.D2.TM] Repeat mode */
    CTC_OAM_PATTERN_RANDOM_TYPE,       /**< [GB.GG.D2.TM] Random mode */
    CTC_OAM_PATTERN_INC_BYTE_TYPE,     /**< [GB.GG.D2.TM] Increase by byte mode */
    CTC_OAM_PATTERN_DEC_BYTE_TYPE,     /**< [GB.GG.D2.TM] Decrease by byte mode */
    CTC_OAM_PATTERN_INC_WORD_TYPE,     /**< [GB.GG.D2.TM] Increase by word mode */
    CTC_OAM_PATTERN_DEC_WORD_TYPE,     /**< [GB.GG.D2.TM] Decrease by word mode */

    CTC_OAM_PATTERN_MAX_TYPE
};
typedef enum ctc_oam_trpt_pattern_type_e ctc_oam_trpt_pattern_type_t;

/*
 @brief   Define OAM throughput tx mode type
*/
enum ctc_oam_trpt_tx_type_e
{
    CTC_OAM_TX_TYPE_PACKET = 0,      /**< [GG.D2.TM] Stop packets transmission after the number of packet transmission */
    CTC_OAM_TX_TYPE_CONTINUOUS,      /**< [GG.D2.TM] Transmit continuously */
    CTC_OAM_TX_TYPE_PERIOD,          /**< [GG.D2.TM] Stop packets transmission after a period of packet transmission */

    CTC_OAM_TX_TYPE_MAX
};
typedef enum ctc_oam_trpt_tx_type_e ctc_oam_trpt_tx_type_t;

/*
 @brief   Define OAM throughput config structure
*/
struct ctc_oam_trpt_s
{
    uint8 session_id;             /**< [GB.GG.D2.TM] Throughput session id 0~3 */
    uint8  tx_mode;               /**< [GB.GG.D2.TM] Tx mode, refer to ctc_oam_trpt_tx_type_t */
    uint8 resv[2];

    uint16 vlan_id;               /**< [GB.GG.D2.TM] Vlan id */
    uint32 gport;                 /**< [GB.GG.D2.TM] Throughput test port, invalid when nhid is non-zero,
                                  D2 when nhid is non-zero, means mcast dest-group-id*/

    uint32 nhid;                  /**< [GB.GG.D2.TM] nexthop ID, 0 means invalid */

    ctc_oam_trpt_pattern_type_t pattern_type;    /**< [GB.GG.D2.TM] Pattern type */
    uint32 repeat_pattern;        /**< [GB.GG.D2.TM] Repeat pattern used for CTC_PATTERN_REPEAT_TYPE */
    uint32 rate;                  /**< [GB.GG.D2.TM] Line speed(Kbps) */

    void*   pkt_header;           /**< [GB.GG.D2.TM] User-defined data */

    uint16  size;                 /**< [GB.GG.D2.TM] Total packet size, include header_len and crc, must >= 64 && >= header_len + 4 bytes */
    uint16  header_len;           /**< [GB.GG.D2.TM] User-defined data length, not include crc */

    uint32  tx_period;            /**< [GG.D2.TM] Used for send pkts when in tx CTC_OAM_TX_TYPE_PERIOD mode, unit: 1s */

    uint8 tx_seq_en;              /**< [GB.GG.D2.TM] Enable insert sequence number */
    uint8 seq_num_offset;         /**< [GB.GG.D2.TM] Insert sequence number offset in user-defined packet
                             when send SLM packet, the offset is the TxFcf offset in SLM packet, and tx_seq_en must be set to 1*/
    uint32  packet_num;           /**< [GB.GG.D2.TM] Used for send pkts when in tx CTC_OAM_TX_TYPE_PACKET mode */
    ctc_direction_t direction;    /**< [GB.GG.D2.TM] direction of trpt, ingress or egress */
    /*SLM/SLR*/
    uint8 is_slm;                             /**< [GG] 1: send SLM packet */
    uint8 first_pkt_clear_en;     /**< [GG] SLM stats value will be cleared when tx/rx the first packet */
};
typedef struct ctc_oam_trpt_s ctc_oam_trpt_t;

struct ctc_oam_hw_aps_s
{
    uint16 aps_group_id;          /**< [D2.TM] APS Group ID, 0 is used for MEP clear association with hw APS */
    uint8  protection_path;       /**< [D2.TM] Is the path in working path or protection path */
    uint8  rsv;
};
typedef struct ctc_oam_hw_aps_s ctc_oam_hw_aps_t;

/**@} end of @defgroup  oam OAM */

#ifdef __cplusplus
}
#endif

#endif

