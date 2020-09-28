/****************************************************************************
 *file ctc_error.h

 *author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 *date 2009-11-26

 *version v2.0

 The file define  CENTEC SDK  error code
 ****************************************************************************/

#ifndef _CTC_ERROR_H
#define _CTC_ERROR_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/../../sal/include/sal_log.h"
#include "common/../../sal/include/sal_types.h"
#include "common/include/ctc_debug.h"
#include "common/include/ctc_warmboot.h"
/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/**
 @defgroup error ERROR
 @{
*/

/**
 @brief  define error type
*/
enum ctc_err_e
{
    CTC_E_NONE = 0,                /**< NO error */
    CTC_E_INVALID_PTR = -32000,    /**< Invalid pointer */
    CTC_E_INVALID_PORT,            /**< Invalid port */
    CTC_E_INVALID_PARAM,           /**< Invalid parameter */
    CTC_E_BADID,                   /**< Invalid identifier */
    CTC_E_INVALID_CHIP_ID,         /**< Invalid chip */
    CTC_E_INVALID_CONFIG,          /**< Invalid configuration */
    CTC_E_EXIST,                   /**< Entry exists */
    CTC_E_NOT_EXIST,               /**< Entry not found */
    CTC_E_NOT_READY,               /**< Not ready to configuration */
    CTC_E_IN_USE,                  /**< Already in used */
    CTC_E_NOT_SUPPORT,             /**< API or some feature is not supported */
    CTC_E_NO_RESOURCE,             /**< No resource in ASIC */
    CTC_E_PROFILE_NO_RESOURCE,     /**< Profile no resource in ASIC */
    CTC_E_NO_MEMORY,               /**< No memory */
    CTC_E_HASH_CONFLICT,           /**< Hash Conflict */
    CTC_E_NOT_INIT,                /**< Feature not initialized */
    CTC_E_INIT_FAIL,               /**< Feature initialize fail */
    CTC_E_DMA,                     /**< DMA error */
    CTC_E_HW_TIME_OUT,             /**< HW operation timed out */
    CTC_E_HW_BUSY,                 /**< HW busy */
    CTC_E_HW_INVALID_INDEX,        /**< Exceed max table index */
    CTC_E_HW_NOT_LOCK,             /**< HW not lock */
    CTC_E_HW_FAIL,                 /**< HW operation fail */
    CTC_E_VERSION_MISMATCH,        /**< Version mismatch*/
    CTC_E_PARAM_CONFLICT,          /**< Parameter Conflict*/
    CTC_E_TOO_MANY_FRAGMENT,     /**< Too many fragment, need Defragmentation*/
    CTC_E_WB_BUSY,               /**< Warmboot busy */
};
typedef enum ctc_err_e ctc_err_t;

enum ctc_err_old_e
{
    /**< generic */
    CTC_E_ENTRY_EXIST = -30000,    /**< Entry already exist */
    CTC_E_ENTRY_NOT_EXIST,         /**< Entry not exist */
    CTC_E_EXCEED_MAX_SIZE,         /**< Exceed max value */
    CTC_E_EXCEED_MIN_SIZE,         /**< Under min value */
    CTC_E_INVALID_DIR,             /**< Invalid direction */
    CTC_E_CANT_CREATE_AVL,         /**< Cannot create avl tree */
    CTC_E_UNEXPECT,                /**< Unexpect value/result */
    CTC_E_LESS_ZERO,               /**< Less zero */
    CTC_E_MEMBER_EXIST,            /**< Member already exist */
    CTC_E_MEMBER_NOT_EXIST,        /**< Member not exist */
    CTC_E_INVALID_EXP_VALUE,       /**< Invalid exp value */
    CTC_E_INVALID_MPLS_LABEL_VALUE, /**< Invalid mpls label value */
    CTC_E_INVALID_REG,             /**< Invalid register id */
    CTC_E_INVALID_TBL,             /**< Invalid table id */
    CTC_E_GLOBAL_CONFIG_CONFLICT,  /**< Global config conflict*/
    CTC_E_FEATURE_NOT_SUPPORT,     /**< Feature not supported */
    CTC_E_INVALID_TTL,             /**< Invalid TTL */
    CTC_E_INVALID_MTU_SIZE,        /**< Invalid mtu size */
    CTC_E_SPOOL_ADD_UPDATE_FAILED, /**< Share pool add or update fail */
    CTC_E_SPOOL_REMOVE_FAILED,     /**< Share pool remove fail*/
    CTC_E_RPF_SPOOL_ADD_VECTOR_FAILED,    /**< Share pool add vector failed */
    CTC_E_RPF_SPOOL_REMOVE_VECTOR_FAILED, /**< Share pool remove vector failed */
    CTC_E_NO_ROOM_TO_MOVE_ENTRY,          /**< No room to move entry */

    CTC_E_STP_INVALID_STP_ID,       /**< Invalid stp id */
    CTC_E_EXCEED_STP_MAX_LPORT,     /**< Exceed max stp lport */
    CTC_E_INVALID_COS,              /**< Invalid cos */
    CTC_E_INVALID_FRAMESIZE_INDEX,  /**< Invalid frame size index */
    CTC_E_INVALID_MIN_FRAMESIZE,    /**< Invalid min frame size */
    CTC_E_INVALID_MAX_FRAMESIZE,    /**< Invalid max frame size */
    CTC_E_MAC_NOT_USED,             /**< MAC is not used */
    CTC_E_TRAININT_FAIL,            /**< Training fail */

    /**< MUTEX */
    CTC_E_FAIL_CREATE_MUTEX = -29900,   /**< Create mutex fail*/
    CTC_E_MUTEX_BUSY,                   /**< Mutex busy */

    /**< Chip,port */
    CTC_E_INVALID_CHIP_NUM = -29800,        /**< Invalid chip num */
    CTC_E_INVALID_GLOBAL_CHIPID,            /**< Invalid global chip id */
    CTC_E_INVALID_LOCAL_CHIPID,             /**< Invalid Local chip id */
    CTC_E_CHIP_IS_LOCAL,                    /**< Chip is local! */
    CTC_E_CHIP_IS_REMOTE,                   /**< Chip is not local! */
    CTC_E_INVALID_GLOBAL_PORT,              /**< Invalid global port */
    CTC_E_INVALID_LOGIC_PORT,               /**< Invalid logic port*/
    CTC_E_MEMBER_PORT_EXIST,                /**< Port is exist */
    CTC_E_MEMBER_PORT_NOT_EXIST,            /**< Port is not exist */
    CTC_E_LOCAL_PORT_NOT_EXIST,             /**< Local phyport is not exist */
    CTC_E_ALREADY_PHY_IF_EXIST,             /**< Physical interface  already exists the port */
    CTC_E_ALREADY_SUB_IF_EXIST,             /**< Sub interface  already exists the port */
    CTC_E_PORT_HAS_OTHER_FEATURE,           /**< Has other feature on this port */
    CTC_E_PORT_FEATURE_MISMATCH,            /**< Feature on port mismatch */
    CTC_E_PORT_PVLAN_TYPE_NOT_SUPPORT,      /**< port for this pvlan type can not configure isolated id */
    CTC_E_PORT_HAS_OTHER_RESTRICTION,       /**< port has another restriction feature */
    CTC_E_INVALID_LOCAL_PORT,               /**< Invalid localphy port */
    CTC_E_INVALID_PORT_MAC_TYPE,            /**< Invalid port mac type */
    CTC_E_INVALID_PORT_SPEED_MODE,          /**< Invalid port speed mode */
    CTC_E_INVALID_PREAMBLE,                 /**< Invalid preamble */
    CTC_E_INVALID_TX_THRESHOLD,             /**< Invalid transmit threshold */
    CTC_E_CANT_CHANGE_SERDES_MODE,          /**< Serdes mode can't be changed */
    CTC_E_SERDES_STATUS_NOT_READY,          /**< Serdes Status is not ready  */
    CTC_E_SERDES_MODE_NOT_SUPPORT,          /**< Serdes mode is not support  */
    CTC_E_SERDES_PLL_NOT_SUPPORT,           /**< Serdes pll core divide is not support  */
    CTC_E_SERDES_EYE_TEST_NOT_DONE,         /**< Eye test not done  */
    CTC_E_CHIP_TIME_OUT,                    /**< Time out */
    CTC_E_CHIP_SCAN_PHY_REG_FULL,           /**< phy opt register full*/
    CTC_E_CHIP_DATAPATH_NOT_MATCH,          /**< datapath not match*/
    CTC_E_PORT_INVALID_PORT_MAC,            /**< Invalid Port MAC, high 32 bits must same*/
    CTC_E_PORT_INVALID_ACL_LABEL,           /**< Invalid Port ACL label*/
    CTC_E_PORT_PROP_COLLISION,              /**< Port property collision */
    CTC_E_PORT_ACL_EN_FIRST,                /**< Enable ACL on port first */
    CTC_E_CHIP_SWITCH_FAILED,               /**< dynamic switch failed */
    CTC_E_PORT_MAC_IS_DISABLE,              /**< mac is not enable */
    CTC_E_PORT_MAC_PCS_ERR,                 /**< Pcs is error */
    CTC_E_INVALID_DEVICE_ID,                /**< Invalid device id */

    /**< Vlan */
    CTC_E_VLAN_EXCEED_MAX_VLANCTL = -29700,    /**< [Vlan] Exceed max vlantag control type */
    CTC_E_VLAN_EXCEED_MAX_PHY_PORT,            /**< [Vlan] Exceed max phy port */
    CTC_E_VLAN_INVALID_VLAN_ID,                /**< [Vlan] Invalid vlan id */
    CTC_E_VLAN_INVALID_VLAN_PTR,               /**< [Vlan] Invalid vlan ptr */
    CTC_E_VLAN_ERROR_MODE,                     /**< [Vlan] API not support in this mode */
    CTC_E_VLAN_INVALID_VLAN_ID_OR_VLAN_PTR,    /**< [Vlan] Invalid vlan id or vlan ptr */
    CTC_E_VLAN_INVALID_FID_ID,                 /**< [Vlan] Invalid fid id */
    CTC_E_VLAN_INVALID_VRFID,                  /**< [Vlan] Invalid vrfid */
    CTC_E_VLAN_INVALID_PROFILE_ID,             /**< [Vlan] Invalid profile id */
    CTC_E_VLAN_EXIST,                          /**< [Vlan] Vlan already create  */
    CTC_E_VLAN_NOT_CREATE,                     /**< [Vlan] Vlan not create */
    CTC_E_VLAN_FILTER_PORT,                    /**< [Vlan] Invalid vlan filter port*/
    CTC_E_VLAN_CLASS_INVALID_TYPE,             /**< [Vlan] Vlan class invalid type */
    CTC_E_VLAN_CLASS_NOT_ENABLE,               /**< [Vlan] Vlan class not enable */
    CTC_E_VLAN_MAPPING_NOT_ENABLE,             /**< [Vlan] Vlan mapping not enable */
    CTC_E_VLAN_MAPPING_INVALID_SERVICE_ID,     /**< [Vlan] Vlan mapping invalid service id */
    CTC_E_VLAN_MAPPING_RANGE_TYPE_NOT_MATCH,   /**< [Vlan] Vlan mapping range type not match */
    CTC_E_VLAN_MAPPING_RANGE_GROUP_NOT_MATCH,  /**< [Vlan] Vlan mapping range group not match */
    CTC_E_VLAN_MAPPING_GET_VLAN_RANGE_FAILED,  /**< [Vlan] Vlan mapping get range failed*/
    CTC_E_VLAN_MAPPING_VRANGE_FULL,            /**< [Vlan] Vlan mapping vlan range full*/
    CTC_E_VLAN_MAPPING_VRANGE_OVERLAPPED,      /**< [Vlan] Vlan mapping vlan range overlapped*/
    CTC_E_VLAN_MAPPING_TAG_APPEND,             /**< [Vlan] Vlan mapping tag_edit append wrong*/
    CTC_E_VLAN_MAPPING_TAG_SWAP,               /**< [Vlan] Vlan mapping tag_edit swap wrong*/
    CTC_E_VLAN_MAPPING_TAG_OP_NOT_SUPPORT,     /**< [Vlan] Vlan mapping tag operation not support */
    CTC_E_VLAN_RANGE_NOT_EXIST,                /**< [Vlan] Vlan range not exist*/
    CTC_E_VLAN_RANGE_EXIST,                    /**< [Vlan] Vlan range already exist*/
    CTC_E_VLAN_RANGE_END_GREATER_START,        /**< [Vlan] End vlan id should greater than start vlan id */
    CTC_E_VLAN_CLASS_PROTOCOL_VLAN_FULL,       /**< [Vlan] Protocol vlan table is full */
    CTC_E_VLAN_ACL_EN_FIRST,                   /**< [Vlan] Enable ACL on vlan first */

    /**< Linkagg */
    CTC_E_INVALID_TID = -29600,                  /**< [Linkagg] Tid is invalid */
    CTC_E_LINKAGG_NOT_EXIST,                     /**< [Linkagg] Entry is not exist */
    CTC_E_LINKAGG_HAS_EXIST,                     /**< [Linkagg] Entry already exist */
    CTC_E_LINKAGG_DYNAMIC_BALANCE_NOT_SUPPORT,   /**< [Linkagg] Dynamic balance don't support */
    CTC_E_LINKAGG_INVALID_LINKAGG_MODE,          /**< [Linkagg] Linkagg mode is invalid */

    /**< mem manager */
    CTC_E_CREATE_MEM_CACHE_FAIL = -29500,   /**< Create mem cache fail */

    /**< usrid */
    CTC_E_USRID_INVALID_KEY = -29400,  /**< [Userid] Invalid usrid key */
    CTC_E_USRID_INVALID_LABEL,         /**< [Userid] Invalid usrid label */
    CTC_E_USRID_INVALID_TYPE,          /**< [Userid] Invalid usrid type */
    CTC_E_USRID_DISABLE,               /**< [Userid] Userid disable */
    CTC_E_USRID_ALREADY_ENABLE,        /**< [Userid] Userid already enable */
    CTC_E_USRID_NO_KEY,                /**< [Userid] Userid no key */

    /**< scl*/
    CTC_E_SCL_INVALID_KEY = -29380,          /**< [SCL] Invalid scl key */
    CTC_E_SCL_INVALID_LABEL,                 /**< [SCL] Invalid scl label */
    CTC_E_SCL_INVALID_TYPE,                  /**< [SCL] Invalid scl type */
    CTC_E_SCL_DISABLE,                       /**< [SCL] SCL disable */
    CTC_E_SCL_ALREADY_ENABLE,                /**< [SCL] SCL already enable */
    CTC_E_SCL_NO_KEY,                        /**< [SCL] SCL no key */
    CTC_E_SCL_INVALID_DEFAULT_PORT,          /**< [SCL] Invalid default port*/
    CTC_E_SCL_HASH_CONFLICT,                 /**< [SCL] Hash conflict*/
    CTC_E_SCL_HASH_INSERT_FAILED,            /**< [SCL] Hash insert fail*/
    CTC_E_SCL_HASH_CONVERT_FAILED,           /**< [SCL] Hash covert fail*/
    CTC_E_SCL_BUILD_AD_INDEX_FAILED,         /**< [SCL] Build ad index fail*/
    CTC_E_SCL_NEED_DEBUG,                    /**< [SCL] Need debug*/
    CTC_E_SCL_GET_KEY_FAILED,                /**< [SCL] Get key fail*/
    CTC_E_SCL_KEY_AD_EXIST,                  /**< [SCL] Key ad already exist*/
    CTC_E_SCL_AD_NO_MEMORY,                  /**< [SCL] Ad no memory*/
    CTC_E_SCL_BUILD_VLAN_ACTION_INDEX_FAILED, /**< [SCL] Build vlan action index fail*/
    CTC_E_SCL_UNSUPPORT,                     /**< [SCL] No support*/
    CTC_E_SCL_CANNOT_OVERWRITE,              /**< [SCL] Cannot overwrite*/
    CTC_E_SCL_LKP_FAILED,                    /**< [SCL] Lookup fail*/
    CTC_E_SCL_VLAN_ACTION_INVALID,           /**< [SCL] Invalid vlan action*/
    CTC_E_SCL_STATIC_ENTRY,                  /**< [SCL] Static entry*/
    CTC_E_SCL_UPDATE_FAILED,                 /**< [SCL] Update entry failed*/
    CTC_E_SCL_INIT,                          /**< [SCL] Init failed */
    CTC_E_SCL_GROUP_PRIORITY,                /**< [SCL] Invalid group priority */
    CTC_E_SCL_GROUP_TYPE,                    /**< [SCL] Invalid group type */
    CTC_E_SCL_GROUP_ID,                      /**< [SCL] Invalid group id */
    CTC_E_SCL_GROUP_ID_RSVED,                /**< [SCL] Group id reserved */
    CTC_E_SCL_GROUP_UNEXIST,                 /**< [SCL] Group not exist */
    CTC_E_SCL_GROUP_EXIST,                   /**< [SCL] Group exist */
    CTC_E_SCL_GROUP_NOT_EMPTY,               /**< [SCL] Group not empty */
    CTC_E_SCL_INVALID_ACTION,                 /**< [SCL] Invalid action */
    CTC_E_SCL_INVALID_KEY_TYPE,               /**< [SCL] Invalid key type */
    CTC_E_SCL_INVALID_LABEL_TYPE,             /**< [SCL] Invalid label type */
    CTC_E_SCL_LABEL_NOT_EXIST,                /**< [SCL] Label not exist */
    CTC_E_SCL_HAVE_ENABLED,                   /**< [SCL] Have enabled */
    CTC_E_SCL_NOT_ENABLED,                    /**< [SCL] Not enabled */
    CTC_E_SCL_LABEL_IN_USE,                   /**< [SCL] Label in use */
    CTC_E_SCL_FLAG_COLLISION,                 /**< [SCL] Flag collision */
    CTC_E_SCL_L4_PORT_RANGE_EXHAUSTED,        /**< [SCL] L4 port range exhausted */
    CTC_E_SCL_TCP_FLAG_EXHAUSTED,             /**< [SCL] Tcp flags exhausted */
    CTC_E_SCL_GET_NODES_FAILED ,              /**< [SCL] Get nodes failed */
    CTC_E_SCL_PORT_BITMAP_OVERFLOW,           /**< [SCL] Port bitmap overflow */
    CTC_E_SCL_GROUP_INFO,                     /**< [SCL] Group info error */
    CTC_E_SCL_GET_BLOCK_INDEX_FAILED,         /**< [SCL] Get block index failed */
    CTC_E_SCL_ENTRY_INSTALLED,                /**< [SCL] Entry installed */
    CTC_E_SCL_GET_STATS_FAILED,               /**< [SCL] Get stats failed */
    CTC_E_SCL_GROUP_NOT_INSTALLED,            /**< [SCL] Group not installed */
    CTC_E_SCL_SERVICE_ID,                     /**< [SCL] Invalid service id */
    CTC_E_SCL_GLOBAL_CFG_ERROR,               /**< [SCL] Global config error */
    CTC_E_SCL_STATS_NOT_SUPPORT,              /**< [SCL] Stats not support on this priority */
    CTC_E_SCL_ADD_TCAM_ENTRY_TO_WRONG_GROUP,  /**< [SCL] Add tcam entry to wrong group */
    CTC_E_SCL_ADD_HASH_ENTRY_TO_WRONG_GROUP,  /**< [SCL] Add hash entry to wrong group */
    CTC_E_SCL_HASH_ENTRY_NO_PRIORITY,         /**< [SCL] Hash entry has no priority */
    CTC_E_SCL_HASH_ENTRY_UNSUPPORT_COPY,      /**< [SCL] Hash acl entry not support copy */
    CTC_E_SCL_VLAN_EDIT_INVALID,              /**< [SCL] SCL vlan edit action invalid */
    CTC_E_SCL_KEY_ACTION_TYPE_NOT_MATCH,      /**< [SCL] SCL Key type Action type not match */


    /**< fdb */
    CTC_E_FDB_WRONG_MAC_ADDR = -29300,     /**< [FDB] Wrong mac address */
    CTC_E_FDB_MAC_FILTERING_ENTRY_EXIST,   /**< [FDB] Mac filtering entry is already exist */
    CTC_E_FDB_ENTRY_FULL,                  /**< [FDB] Entries have been full */
    CTC_E_FDB_L2MCAST_MEMBER_INVALID,      /**< [FDB] L2mcast member is invalid */
    CTC_E_FDB_L2MCAST_ADD_MEMBER_FAILED,   /**< [FDB] L2mcast add member failed */
    CTC_E_FDB_MCAST_ENTRY_EXIST,           /**< [FDB] Multicast entry with same mac is already exists */
    CTC_E_FDB_DEFAULT_ENTRY_NOT_EXIST,     /**< [FDB] Default entry does not exist */
    CTC_E_FDB_OPERATION_PAUSE,             /**< [FDB] Need to next flush */
    CTC_E_FDB_HASH_INSERT_FAILED,          /**< [FDB] Hash insert failed */
    CTC_E_FDB_HASH_REMOVE_FAILED,          /**< [FDB] Hash remove failed */
    CTC_E_FDB_AD_INDEX_NOT_EXIST,          /**< [FDB] Ad index not exist */
    CTC_E_FDB_DEFAULT_ENTRY_NOT_ALLOWED,   /**< [FDB] Default entry not allowed */
    CTC_E_FDB_INVALID_FDB_TYPE,            /**< [FDB] Invalid fdb type */
    CTC_E_FDB_NOT_LOCAL_MEMBER,            /**< [FDB] Member port is not local member */
    CTC_E_FDB_KEY_ALREADY_EXIST,           /**< [FDB] Key already exist */
    CTC_E_FDB_HASH_CONFLICT,               /**< [FDB] Hash confict */
    CTC_E_FDB_SOURCE_PORT_TYPE_NOT_MATCH,  /**< [FDB] Source port type not match */
    CTC_E_FDB_NO_RESOURCE,                 /**< [FDB] No resource */
    CTC_E_FDB_INVALID_FID,                 /**< [FDB] Invalid FID */
    CTC_E_FDB_NO_SW_TABLE_NO_QUARY,        /**< [FDB] Not support this function when no software table */
    CTC_E_FDB_ONLY_SW_LEARN,               /**< [FDB] FDB in software aging mode, you can change it in chip_profile.cfg */
    CTC_E_FDB_LOGIC_USE_HW_LEARN_DISABLE,  /**< [FDB] FDB logic-port use hw-learning disable, you can change it in chip_profile.cfg */
    CTC_E_FDB_NO_SW_TABLE_NO_FLUSH_BY_SW,  /**< [FDB] Not support software flush when no software table */
    CTC_E_FDB_ONLY_HW_LEARN,               /**< [FDB] Not support Hw learning to Software learning Switch*/
    CTC_E_FDB_FIB_DUMP_FAIL,               /**< [FDB] FIB Dump failed */
    CTC_E_FDB_SECURITY_VIOLATION,          /**< [FDB] Security violation */

    /**< Nexthop module */
    CTC_E_NH_NOT_INIT = -29200,                    /**< [Nexthop] Nexhexthop not init */
    CTC_E_NH_INVALID_NHID,                         /**< [Nexthop] Nexthop ID is invalid value */
    CTC_E_NH_NOT_EXIST,                            /**< [Nexthop] Nexthop not exist */
    CTC_E_NH_EXIST,                                /**< [Nexthop] Nexthop  exist */
    CTC_E_NH_INVALID_DSEDIT_PTR,                   /**< [Nexthop] L2DsEditPtr is invalid value*/
    CTC_E_NH_INVALID_NH_TYPE,                      /**< [Nexthop] Nexthop type is invalid */
    CTC_E_NH_INVALID_NH_SUB_TYPE,                  /**< [Nexthop] Nexthop sub type is invalid */
    CTC_E_NH_INVALID_VLAN_EDIT_TYPE,               /**< [Nexthop] Egress Vlan Edit type is invalid */
    CTC_E_NH_INVALID_DSNH_TYPE,                    /**< [Nexthop] DsNexthop table type is invalid */
    CTC_E_NH_VLAN_EDIT_CONFLICT,                   /**< [Nexthop] Configure confilct, if cvlan(svlan) edit type is insert-vlan, svlan(cvlan) edit type should not be edit-none or edit-replace */
    CTC_E_NH_STATS_NOT_EXIST,                      /**< [Nexthop] Nexthop stats not exist */
    CTC_E_NH_STATS_EXIST,                          /**< [Nexthop] Nexthop stats exist */
    CTC_E_NH_INVALID_MARTINI_SEQ_TYPE = -29150,    /**< [Nexthop] Invalid martini seq type */
    CTC_E_NH_SHOULD_USE_DSNH8W,                    /**< [Nexthop] Should use DsNexthop8W table */
    CTC_E_NH_SHOULD_USE_DSNH4W,                    /**< [Nexthop] Should use DsNexthop4W table */
    CTC_E_NH_NHID_NOT_MATCH_NHTYPE,                /**< [Nexthop] Nexthop type doesn't match */
    CTC_E_NH_GLB_SRAM_IS_INUSE,                    /**< [Nexthop] Global DsNexthop offset or GroupId is used in other nexthop, can't re-use it */
    CTC_E_NH_GLB_SRAM_ISNOT_INUSE,                 /**< [Nexthop] Global DsNexthop offset or GroupId isn't used,  can't free it */
    CTC_E_NH_GLB_SRAM_INDEX_EXCEED,                /**< [Nexthop] Global DsNexthop offset or GroupId is out of range of offset value*/
    CTC_E_NH_NO_LABEL,                             /**< [Nexthop] Need at least one mpls label*/
    CTC_E_NH_ISNT_UNROV,                           /**< [Nexthop] Current nexthop isn't unresolved nexthop */
    CTC_E_NH_IS_UNROV,                             /**< [Nexthop] Current nexthop is unresolved nexthop */
    CTC_E_NH_EXCEED_MAX_DSNH_OFFSET,               /**< [Nexthop] Exceed the max number of dsnh offset */
    CTC_E_NH_EXCEED_MAX_LOGICAL_REPLI_CNT,         /**< [Nexthop] Exceed the max logical replicate number */
    CTC_E_NH_MEMBER_IS_UPMEP,                      /**< [Nexthop] Member is upmep */
    CTC_E_NH_EGS_EDIT_CONFLICT_VLAN_AND_MACDA_EDIT, /**< [Nexthop] Vlan edit and change MAC DA edit exist conflict */
    CTC_E_NH_EXIST_VC_LABEL,                       /**< [Nexthop] Other nexthop are using already*/
    CTC_E_NH_NOT_EXIST_TUNNEL_LABEL,               /**< [Nexthop] Tunnel label not exist*/
    CTC_E_NH_NHOFFSET_EXHAUST,                     /**< [Nexthop] Nexthop offset is exhausted*/
    CTC_E_NH_TOO_MANY_LABELS,                      /**< [Nexthop] Nexthop offset is exhausted*/
    CTC_E_NH_EXCEED_MAX_ECMP_NUM,                  /**< [Nexthop] Exceed the max number of ecmp */
    CTC_E_NH_ECMP_MEM_NOS_EXIST,                   /**< [Nexthop] ECMP member not exist! */
    CTC_E_NH_TUNNEL_GRE_KEY_IS_TOO_BIG,            /**< [Nexthop] GRE key is too big, it is only 16 bit when enable mtu check */
    CTC_E_NH_ECMP_IN_USE,                          /**< [Nexthop] ECMP already in use, max path can not be reset */
    CTC_E_NH_HR_NH_IN_USE,                         /**< [Nexthop] Host nexthop already in use. */
    CTC_E_NH_CROSS_CHIP_RSPAN_NH_IN_USE,           /**< [Nexthop] CrossChip RSPAN nexthop already in use. */
    CTC_E_NH_MCAST_MIRROR_NH_IN_USE,               /**< [Nexthop] Mcast mirror already in use. */
    CTC_E_NH_EXCEED_MCAST_PHY_REP_NUM,             /**< [Nexthop] Exceed the max physical replicate number . */
    CTC_E_NH_LABEL_IS_MISMATCH_WITH_STATS,         /**< [Nexthop] When support two tunnel label, can not support stats */
    CTC_E_NH_SUPPORT_PW_APS_UPDATE_BY_INT_ALLOC_DSNH, /**< [Nexthop] Only sdk allocate dsnh offset support update form normal pw to aps pw,or reverse*/
    CTC_E_NH_INVALID_TPID_INDEX,                   /**< [Nexthop] Nexthop svlan tpid index is invalid*/


    /**< l3 interface */
    CTC_E_L3IF_VMAC_NOT_EXIST = -29100,    /**< [L3if] Vmac not exist! */
    CTC_E_L3IF_VMAC_INVALID_PREFIX_TYPE,   /**< [L3if] The prefix_type for vmac is invalid! */
    CTC_E_L3IF_VMAC_ENTRY_EXCEED_MAX_SIZE, /**< [L3if] Vmac entry exceed the max size! */
    CTC_E_L3IF_ROUTED_EXCEED_IF_SIZE,      /**< [L3if] Routed port(Sub interface) entry exceed the max size! */
    CTC_E_L3IF_INVALID_IF_TYPE,            /**< [L3if] Invalid  L3 interface type */
    CTC_E_L3IF_INVALID_IF_ID,              /**< [L3if] Invalid interface ID! */
    CTC_E_L3IF_NOT_EXIST,                  /**< [L3if] Not exist! */
    CTC_E_L3IF_EXIST,                      /**< [L3if] Already exist! */
    CTC_E_L3IF_MISMATCH,                   /**< [L3if] Information is mismatch */
    CTC_E_L3IF_NO_ALLOCED_IPUCSA,          /**< [L3if] No alloced ipucsa */
    CTC_E_L3IF_INVALID_PBR_LABEL,          /**< [L3if] Invalid L3 interface pbr label! */
    CTC_E_L3IF_INVALID_RPF_CHECK_TYPE,     /**< [L3if] Invalid RPF check type! */
    CTC_E_L3IF_INVALID_RPF_STRICK_TYPE,    /**< [L3if] Invalid RPF strick type! */
    CTC_E_L3IF_INVALID_ACL_LABEL,          /**< [L3if] Invalid acl vlan label! */
    CTC_E_L3IF_VRF_STATS_NOT_EXIST,        /**< [L3if] VRF stats not exist */
    CTC_E_L3IF_ROUTER_MAC_EXHAUSTED,       /**< [L3if] Router mac exhausted */

    /**< acl/qos */
    CTC_E_ACLQOS_ENTRY_NOT_EXIST = -29000,       /**< [AclQos] Entry not exist */
    CTC_E_ACLQOS_ENTRY_EXIST,                    /**< [AclQos] Entry exist */
    CTC_E_ACLQOS_INVALID_ACTION,                 /**< [AclQos] Invalid action */
    CTC_E_ACLQOS_INVALID_KEY_TYPE,               /**< [AclQos] Invalid key type */
    CTC_E_ACLQOS_INVALID_LABEL_TYPE,             /**< [AclQos] Invalid label type */
    CTC_E_ACLQOS_LABEL_NOT_EXIST,                /**< [AclQos] Label not exist */
    CTC_E_ACLQOS_HAVE_ENABLED,                   /**< [AclQos] Have enabled */
    CTC_E_ACLQOS_NOT_ENABLED,                    /**< [AclQos] Not enabled */
    CTC_E_ACLQOS_LABEL_IN_USE,                   /**< [AclQos] Label in use */
    CTC_E_QOS_POLICER_CREATED,                   /**< [Qos] Policer created */
    CTC_E_QOS_POLICER_NOT_EXIST,                 /**< [Qos] Policer not exist */
    CTC_E_QOS_POLICER_NOT_BIND,                  /**< [Qos] Policer not_bind */
    CTC_E_QOS_POLICER_IN_USE,                    /**< [Qos] Policer in_use */
    CTC_E_QOS_POLICER_CIR_GREATER_THAN_PIR,      /**< [Qos] Policer cir greater than pir */
    CTC_E_QOS_POLICER_CBS_GREATER_THAN_PBS,      /**< [Qos] Policer cbs greater than pbs */
    CTC_E_QOS_POLICER_SERVICE_POLICER_NOT_ENABLE, /**< [Qos] Policer service policer not enable */
    CTC_E_QOS_POLICER_STATS_NOT_ENABLE,          /**< [Qos] Policer stats not enable */
    CTC_E_QOS_QUEUE_STATS_NOT_ENABLE,            /**< [Qos] Queue stats not enable */
    CTC_E_QOS_NO_INT_POLICER_ENTRY,              /**< [Qos] No available internal policer entries */
    CTC_E_QOS_NO_EXT_POLICER_ENTRY,              /**< [Qos] No available external policer entries */
    CTC_E_QOS_NO_INT_POLICER_PROFILE_ENTRY,      /**< [Qos] No available internal policer profile entries */
    CTC_E_ACLQOS_DIFFERENT_DIR,                  /**< [AclQos] Different direction */
    CTC_E_ACLQOS_DIFFERENT_TYPE,                 /**< [AclQos] Different type */
    CTC_E_ACLQOS_DIFFERENT_CHIP,                 /**< [AclQos] Different chip */
    CTC_E_ACL_PBR_ECMP_CMP_FAILED,               /**< [Acl] PBR ECMP entry compare failed, can not do ecmp */
    CTC_E_ACL_PBR_DO_ECMP_FAILED,                /**< [Acl] Pbr do ecmp failed */
    CTC_E_ACL_PBR_ENTRY_NO_NXTTHOP,              /**< [Acl] Pbr permit entry, must add fwd-to-nhid action */
    CTC_E_QOS_THIS_PORT_NOT_SUPPORT_POLICER,     /**< [Qos] This port not support policer */
    CTC_E_QOS_POLICER_NOT_SUPPORT_STATS,         /**< [Qos] This policer not support stats */
    CTC_E_ACL_INIT,                              /**< [Acl] Init failed */
    CTC_E_ACL_GROUP_PRIORITY,                    /**< [Acl] Invalid group priority */
    CTC_E_ACL_GROUP_TYPE,                        /**< [Acl] Invalid group type */
    CTC_E_ACL_GROUP_ID,                          /**< [Acl] Invalid group id */
    CTC_E_ACL_GROUP_ID_RSVED,                    /**< [Acl] Group id reserved */
    CTC_E_ACL_ENTRY_EXIST,                       /**< [Acl] Entry exist */
    CTC_E_ACL_ENTRY_UNEXIST,                     /**< [Acl] Entry not exist */
    CTC_E_ACL_GROUP_UNEXIST,                     /**< [Acl] Group not exist */
    CTC_E_ACL_GROUP_EXIST,                       /**< [Acl] Group exist */
    CTC_E_ACL_GROUP_NOT_EMPTY,                   /**< [Acl] Group not empty */
    CTC_E_ACL_INVALID_ACTION,                 /**< [Acl] Invalid action */
    CTC_E_ACL_INVALID_KEY_TYPE,               /**< [Acl] Invalid key type */
    CTC_E_ACL_INVALID_LABEL_TYPE,             /**< [Acl] Invalid label type */
    CTC_E_ACL_LABEL_NOT_EXIST,                /**< [Acl] Label not exist */
    CTC_E_ACL_HAVE_ENABLED,                   /**< [Acl] Have enabled */
    CTC_E_ACL_NOT_ENABLED,                    /**< [Acl] Not enabled */
    CTC_E_ACL_LABEL_IN_USE,                   /**< [Acl] Label in use */
    CTC_E_ACL_FLAG_COLLISION,                 /**< [Acl] Flag collision */
    CTC_E_ACL_L4_PORT_RANGE_EXHAUSTED,        /**< [Acl] L4 port range exhausted */
    CTC_E_ACL_TCP_FLAG_EXHAUSTED,             /**< [Acl] Tcp flags exhausted */
    CTC_E_ACL_GET_NODES_FAILED ,              /**< [Acl] Get nodes failed */
    CTC_E_ACL_PORT_BITMAP_OVERFLOW,           /**< [Acl] Port bitmap overflow */
    CTC_E_ACL_GROUP_INFO,                     /**< [Acl] Group info error */
    CTC_E_ACL_ENTRY_INSTALLED,                /**< [Acl] Entry installed */
    CTC_E_ACL_GET_STATS_FAILED,               /**< [Acl] Get stats failed */
    CTC_E_ACL_GROUP_NOT_INSTALLED,            /**< [Acl] Group not installed */
    CTC_E_ACL_SERVICE_ID,                     /**< [Acl] Invalid service id */
    CTC_E_ACL_GLOBAL_CFG_ERROR,               /**< [Acl] Global config error */
    CTC_E_ACL_STATS_NOT_SUPPORT,              /**< [Acl] Stats not support on this priority */
    CTC_E_ACL_ADD_TCAM_ENTRY_TO_WRONG_GROUP,  /**< [Acl] Add tcam entry to wrong group */
    CTC_E_ACL_ADD_HASH_ENTRY_TO_WRONG_GROUP,  /**< [Acl] Add hash entry to wrong group */
    CTC_E_ACL_HASH_ENTRY_NO_PRIORITY,         /**< [Acl] Hash entry has no priority */
    CTC_E_ACL_HASH_ENTRY_UNSUPPORT_COPY,      /**< [Acl] Hash acl entry not support copy */
    CTC_E_ACL_VLAN_EDIT_INVALID,              /**< [Acl] Acl vlan edit action invalid */
    CTC_E_ACL_INVALID_EGRESS_ACTION,          /**< [Acl] Acl invalid egress action */
    CTC_E_ACL_NO_ROOM_TO_MOVE_ENTRY,          /**< [Acl] Acl no room to move entry */
    CTC_E_ACL_HASH_CONFLICT,                  /**< [Acl] Hash conflict*/
    CTC_E_ACL_IPV4_NO_LAYER3_TYPE,            /**< [Acl] IPV4 key must configure layer3 type*/

    /**< stats */
    CTC_E_STATS_MTU2_LESS_MTU1 = -28900,      /**< [Stats] Mtu2 pkt length should greater than mtu1 pkt length */
    CTC_E_STATS_MTU1_GREATER_MTU2,            /**< [Stats] Mtu1 pkt length should less than mtu2 pkt length */
    CTC_E_STATS_PORT_STATS_NO_TYPE,           /**< [Stats] Port stats option type is none */
    CTC_E_STATS_PHB_STATS_INVALID,            /**< [Stats] Do phb/per-port-phb stats in qos module */
    CTC_E_STATS_PORT_NOT_MAP_TO_MAC,          /**< [Stats] Port not map to mac */
    CTC_E_STATS_PORT_NOT_ENABLE,              /**< [Stats] Port not enable */
    CTC_E_STATS_MAC_STATS_MODE_ASIC,          /**< [Stats] Mac stats information from asic */
    CTC_E_STATS_TABLE_NUM_NO_EXPECT,          /**< [Stats] Stats table entry get from ftm not expect */
    CTC_E_STATS_VLAN_STATS_CONFLICT_WITH_PHB, /**< [Stats] Vlan stats is conflict with phb */
    CTC_E_STATS_NOT_ENABLE,                   /**< [Stats] Stats is not enabled */
    CTC_E_STATS_DMA_ENABLE_BUT_SW_DISABLE,    /**< [Stats] When dma sync is enabled, sw mode must be enabled also */
    CTC_E_STATS_STATSID_EXIST,                /**< [Stats] Statsid have been alloced */
    CTC_E_STATS_STATSID_TYPE_MISMATCH,        /**< [Stats] Stats_id type is mismatch */
    CTC_E_STATS_STATSID_ALREADY_IN_USE,       /**< [Stats] Statsid have been used */

    /**< ipuc */
    CTC_E_IPUC_VERSION_DISABLE = -28800,    /**< [IPUC] SDK is not assigned resource for this version */
    CTC_E_IPUC_INVALID_VRF,                 /**< [IPUC] Invalid vrfid */
    CTC_E_IPUC_INVALID_L3IF,                /**< [IPUC] L3if is not needed */
    CTC_E_IPUC_NEED_L3IF,                   /**< [IPUC] Need l3if */
    CTC_E_IPUC_TUNNEL_INVALID,              /**< [IPUC] SDK is not assigned resource for tunnel of this IP version */
    CTC_E_IPUC_RPF_NOT_SUPPORT_THIS_MASK_LEN,        /**< [IPUC] IPv6 rpf check not support mask length 128 */
    CTC_E_IPUC_NAT_NOT_SUPPORT_THIS_MASK_LEN,        /**< [IPUC] NAPT not support this mask length. for IPV4, only support mask length 32 */
    CTC_E_IPUC_INVALID_TTL_THRESHOLD,       /**< [IPUC] Invalid TTL threshold */
    CTC_E_IPUC_TUNNEL_RPF_INFO_MISMATCH,    /**< [IPUC] IP-tunnel RPF info is mismatch with former config */
    CTC_E_IPUC_ADD_FAILED,                  /**< [IPUC] Add ipuc route failed */
    CTC_E_IPUC_TUNNEL_PAYLOAD_TYPE_MISMATCH, /**< [IPUC] IP-tunnel payload type is mismatch with grekey */
    CTC_E_IPUC_HASH_CONFLICT,               /**< [IPUC] Hash confict */
    CTC_E_IPUC_INVALID_ROUTE_FLAG,          /**< [IPUC] route flag is confict */

    /**< ipmc */
    CTC_E_IPMC_GROUP_NOT_EXIST = -28700, /**< [IPMC] This group does NOT exist */
    CTC_E_IPMC_GROUP_HAS_EXISTED,       /**< [IPMC] This group has already existed */
    CTC_E_IPMC_INVALID_MASK_LEN,        /**< [IPMC] Mask length error */
    CTC_E_IPMC_BAD_L3IF_TYPE,           /**< [IPMC] Unknown l3if type */
    CTC_E_IPMC_NOT_MCAST_ADDRESS,       /**< [IPMC] Group address is NOT multicast IP address */
    CTC_E_IPMC_EXCEED_GLOBAL_CHIP,      /**< [IPMC] Target chip exceeds global chip id */
    CTC_E_IPMC_ADD_MEMBER_FAILED,       /**< [IPMC] Add member failed */
    CTC_E_IPMC_GROUP_IS_NOT_IP_MAC,     /**< [IPMC] The group is not a IP based l2 mcast group */
    CTC_E_IPMC_GROUP_IS_IP_MAC,         /**< [IPMC] The group is a IP based l2 mcast group */
    CTC_E_IPMC_INVALID_VRF,             /**< [IPMC] Invalid vrfid */
    CTC_E_IPMC_GROUP_IS_NOT_INIT,       /**< [IPMC] The group is not init */
    CTC_E_IPMC_VERSION_DISABLE,         /**< [IPMC] SDK is not assigned resource for this version */
    CTC_E_IPMC_RPF_CHECK_DISABLE,       /**< [IPMC] Rpf check disable */
    CTC_E_IPMC_OFFSET_ALLOC_ERROR,      /**< [IPMC] Offset alloc error */
    CTC_E_IPMC_STATS_NOT_EXIST,         /**< [IPMC] Stats not exist */
    CTC_E_IPMC_HASH_CONFLICT,           /**< [IPMC] Ipmc hash conflict */

    /**< pdu */
    CTC_E_PDU_INVALID_ACTION_INDEX = -28600, /**< [PDU] Invalid action index */
    CTC_E_PDU_INVALID_INDEX,                 /**< [PDU] Invalid index */
    CTC_E_PDU_INVALID_IPDA,                  /**< [PDU] Invalid ipda */
    CTC_E_PDU_INVALID_MACDA,                 /**< [PDU] Invalid macda */
    CTC_E_PDU_INVALID_MACDA_MASK,            /**< [PDU] Invalid macda mask */

    /**< queue */
    CTC_E_QUEUE_DROP_PROF_NOT_EXIST = -28500,   /**< [Queue] Drop profile don't exist */
    CTC_E_QUEUE_SHAPE_PROF_NOT_EXIST,           /**< [Queue] Shape profile don't exist */
    CTC_E_QUEUE_CHANNEL_SHAPE_PROF_EXIST,       /**< [Queue] Channel shape profile exist */
    CTC_E_QUEUE_CHANNEL_SHAPE_PROF_NOT_EXIST,   /**< [Queue] Channel shape profile not exist */
    CTC_E_QUEUE_NOT_ENOUGH,                     /**< [Queue] Queue not enough */
    CTC_E_QUEUE_SERVICE_QUEUE_NOT_INITIALIZED,  /**< [Queue] Service queue not initialized */
    CTC_E_QUEUE_SERVICE_QUEUE_INITIALIZED,      /**< [Queue] Service queue initialized */
    CTC_E_QUEUE_SERVICE_EXCEED_MAX_COUNT_LIMIT, /**< [Queue] The number of services reaches the maximum limit */
    CTC_E_QUEUE_SERVICE_NOT_EXIST,              /**< [Queue] Service don't exist */
    CTC_E_QUEUE_SERVICE_REGISTERED,             /**< [Queue] Service already registered */
    CTC_E_QUEUE_SERVICE_EXIST,                  /**< [Queue] Service already exist */
    CTC_E_QUEUE_SERVICE_QUEUE_NO_HASH_ENTRY,    /**< [Queue] Service queue have no hash entry */
    CTC_E_QUEUE_SERVICE_GROUP_NOT_EXIST,        /**< [Queue] Service group don't exist */
    CTC_E_QUEUE_INTERNAL_PORT_IN_USE,           /**< [Queue] Internal port in use */
    CTC_E_QUEUE_NO_FREE_INTERNAL_PORT,          /**< [Queue] Internal port no free count */
    CTC_E_QUEUE_DEPTH_NOT_EMPTY,                /**< [Queue] Depth is not empty */
    CTC_E_QUEUE_WDRR_WEIGHT_TOO_BIG,            /**< [Queue] Wdrr weight is too big */
    CTC_E_QUEUE_INVALID_CONFIG,                 /**< [Queue] Config is invalid  */
    CTC_E_QUEUE_CPU_REASON_NOT_CREATE,           /**<[CPU]  CPU Reason not create */

    /**< aps */
    CTC_E_APS_GROUP_EXIST = -28400, /**< [Aps] Group exist */
    CTC_E_APS_INTERFACE_ERROR,    /**< [Aps] L3 if and port not match */
    CTC_E_APS_INVALID_GROUP_ID,     /**< [Aps] Invalid group id */
    CTC_E_APS_INVALID_TYPE,         /**< [Aps] Invalid select type */
    CTC_E_APS_GROUP_NOT_EXIST,      /**< [Aps] Group don't exist */
    CTC_E_APS_GROUP_NEXT_APS_EXIST,      /**< [Aps] Next aps group exist */
    CTC_E_APS_MPLS_TYPE_L3IF_NOT_EXIST,      /**< [Aps] Mpls aps must configure l3if id */
    CTC_E_APS_RAPS_GROUP_NOT_EXIST,      /**< [Aps] Raps group don't exist */
    CTC_E_APS_DONT_SUPPORT_2_LEVEL_HW_BASED_APS,      /**< [Aps] Aps only support hw-based aps for 1 level */
    CTC_E_APS_DONT_SUPPORT_HW_BASED_APS_FOR_LINKAGG,      /**< [Aps] Aps only support hw-based aps for phyport */

    /**< vpls */
    CTC_E_VPLS_VSI_NOT_CREATE = -28300,  /**< [Vpls] Instance is not created */
    CTC_E_VPLS_INVALID_VPLSPORT_ID,      /**< [Vpls] Invalid logic port id */

    /**< asic */
    CTC_E_HW_OP_FAIL = -28000,          /**< [ASIC] Hardware operation failed */
    CTC_E_DRV_FAIL,                     /**< [ASIC] Driver init failed */
    CTC_E_CHECK_HSS_READY_FAIL,         /**< [ASIC] Hss4G Macro change failed >*/

    /**< OAM */
    CTC_E_OAM_NOT_INIT = -27900,                   /**< [OAM] Module not initialized */
    CTC_E_OAM_MAID_LENGTH_INVALID,                 /**< [OAM] Maid length invalid */
    CTC_E_OAM_MAID_ENTRY_EXIST,                    /**< [OAM] Maid entry exist */
    CTC_E_OAM_MAID_ENTRY_NOT_FOUND,                /**< [OAM] Maid entry not found */
    CTC_E_OAM_MAID_ENTRY_REF_BY_MEP,               /**< [OAM] Maid entry used by local mep, can not remove */
    CTC_E_OAM_CHAN_ENTRY_EXIST,                    /**< [OAM] MEP or MIP lookup key exists */
    CTC_E_OAM_CHAN_ENTRY_NOT_FOUND,                /**< [OAM] MEP or MIP lookup key not found */
    CTC_E_OAM_CHAN_MD_LEVEL_EXIST,                 /**< [OAM] MD level already config other*/
    CTC_E_OAM_CHAN_MD_LEVEL_NOT_EXIST,             /**< [OAM] MD level not exist*/
    CTC_E_OAM_CHAN_ENTRY_NUM_EXCEED,               /**< [OAM] Lookup key entry full in EVC*/
    CTC_E_OAM_LM_NUM_RXCEED,                       /**< [OAM] LM entry full in EVC*/
    CTC_E_OAM_CHAN_LMEP_EXIST,                     /**< [OAM] Local mep already exists*/
    CTC_E_OAM_CHAN_LMEP_NOT_FOUND,                 /**< [OAM] Local mep not found */
    CTC_E_OAM_CHAN_NOT_UP_DIRECTION,               /**< [OAM] Lookup channel isn't up mep channel */
    CTC_E_OAM_CHAN_NOT_DOWN_DIRECTION,             /**< [OAM] Lookup channel isn't down mep channel */
    CTC_E_OAM_TX_GPORT_AND_MASTER_GCHIP_NOT_MATCH, /**< [OAM] Linkagg's master gchip does not match with tx_ccm_gport's chip */
    CTC_E_OAM_TX_GPORT_AND_CHAN_GPORT_NOT_MATCH,   /**< [OAM] Ccm gport does not match with channel gport */
    CTC_E_OAM_RMEP_EXIST,                          /**< [OAM] Remote mep already exists */
    CTC_E_OAM_RMEP_NOT_FOUND,                      /**< [OAM] Remote mep is not found */
    CTC_E_OAM_MEP_LM_NOT_EN,                       /**< [OAM] LM not enable in MEP*/
    CTC_E_OAM_INVALID_MD_LEVEL,                    /**< [OAM] Md level is invalid*/
    CTC_E_OAM_INVALID_MEP_ID,                      /**< [OAM] Mep id is invalid*/
    CTC_E_OAM_INVALID_MEP_CCM_INTERVAL,            /**< [OAM] Mep ccm interval is invalid */
    CTC_E_OAM_INVALID_MEP_TPID_INDEX,              /**< [OAM] Mep tpid index is invalid*/
    CTC_E_OAM_INVALID_MEP_COS,                     /**< [OAM] Mep cos is invalid*/
    CTC_E_OAM_INVALID_CFG_IN_MEP_TYPE,             /**< [OAM] Config is invalid in this mep type*/
    CTC_E_OAM_INVALID_CFG_LM,                      /**< [OAM] Config is invalid lm is Enable*/
    CTC_E_OAM_INVALID_LM_TYPE,                     /**< [OAM] Lm type is invalid*/
    CTC_E_OAM_INVALID_LM_COS_TYPE,                 /**< [OAM] Lm cos type is invalid*/
    CTC_E_OAM_INVALID_CSF_TYPE,                    /**< [OAM] CSF type is invalid*/
    CTC_E_OAM_NH_OFFSET_EXIST,                     /**< [OAM] Nexthop offset for down mep already exists */
    CTC_E_OAM_NH_OFFSET_NOT_FOUND,                 /**< [OAM] Nexthop offset for down mep is not found */
    CTC_E_OAM_NH_OFFSET_ADD_VECTOR_FAIL,           /**< [OAM] Add next hop offset for down mep fail */
    CTC_E_OAM_NH_OFFSET_IN_USE,                    /**< [OAM] Nexthop offset for down mep is in use */
    CTC_E_OAM_RMEP_D_LOC_PRESENT,                  /**< [OAM] Some rmep still has dloc defect */
    CTC_E_OAM_ITU_DEFECT_CLEAR_MODE,               /**< [OAM] Clear the defect fail in itu defect clear mode */
    CTC_E_OAM_MEP_INDEX_VECTOR_ADD_FAIL,           /**< [OAM] Add mep to index vector fail */
    CTC_E_OAM_INVALID_OPERATION_ON_CPU_MEP,        /**< [OAM] Invalid operation on mep configured for cpu */
    CTC_E_OAM_INVALID_OPERATION_ON_P2P_MEP,        /**< [OAM] Invalid operation on p2p mep configured*/
    CTC_E_OAM_DRIVER_FAIL,                         /**< [OAM] Driver fail */
    CTC_E_OAM_EXCEED_MAX_LOOP_BACK_PORT_NUMBER,    /**< [OAM] Loop back port number exceed max value */
    CTC_E_OAM_INVALID_GLOBAL_PARAM_TYPE,           /**< [OAM] Global param type is invalid  */
    CTC_E_OAM_INDEX_EXIST,                         /**< [OAM] Index already exist */
    CTC_E_OAM_INVALID_MA_INDEX,                    /**< [OAM] Ma index is invalid */
    CTC_E_OAM_INVALID_MEP_INDEX,                   /**< [OAM] Oam mep index is invalid  */
    CTC_E_OAM_INVALID_OAM_TYPE,                    /**< [OAM] Oam type is invalid  */
    CTC_E_OAM_EXCEED_MAX_LEVEL_NUM_PER_CHAN,       /**< [OAM] Exceed max level num per channel*/
    CTC_E_OAM_BFD_INVALID_STATE,                   /**< [OAM] BFD state is invalid*/
    CTC_E_OAM_BFD_INVALID_DIAG,                    /**< [OAM] BFD diag is invalid*/
    CTC_E_OAM_BFD_INVALID_INTERVAL,                /**< [OAM] BFD interval is invalid*/
    CTC_E_OAM_BFD_INVALID_DETECT_MULT,             /**< [OAM] BFD detect mult is invalid*/
    CTC_E_OAM_BFD_INVALID_UDP_PORT,                /**< [OAM] BFD Source UDP port is invalid*/
    CTC_E_OAM_TRPT_NOT_INIT,                       /**< [OAM] Throughput Module not initialized */
    CTC_E_OAM_TRPT_INTERVAL_CONFLICT,              /**< [OAM] Throughput rate conflict */
    CTC_E_OAM_TRPT_SESSION_EN,                     /**< [OAM] The session is ongoing */
    CTC_E_OAM_TRPT_SESSION_ALREADY_EN,                     /**< [OAM] The session is ongoing */
    CTC_E_OAM_TRPT_SESSION_NOT_CFG,                     /**< [OAM] The session is not config */
    CTC_E_OAM_TRPT_PERIOD_INVALID,                       /**< [OAM] Throughput send packet period is invalid */

    /**< PTP */
    CTC_E_PTP_NOT_INIT = -27800,                   /**< [PTP] Module not initialized */
    CTC_E_PTP_EXCEED_MAX_FRACTIONAL_VALUE,         /**< [PTP] Exceed max ns or fractional ns value, must less than 10^9 */
    CTC_E_PTP_EXCEED_MAX_FFO_VALUE,                /**< [PTP] Invalid sync interface param */
    CTC_E_PTP_INVALID_SYNC_INTF_PARAM_VALUE,       /**< [PTP] Invalid sync interface param value */
    CTC_E_PTP_TS_NOT_READY,                     /**< [PTP] Time stamp is not ready */
    CTC_E_PTP_TX_TS_ROLL_OVER_FAILURE,             /**< [PTP] TX time stamp roll over failure */
    CTC_E_PTP_TS_GET_BUFF_FAIL,                 /**< [PTP] Fail to get ts from buffer */
    CTC_E_PTP_TS_ADD_BUFF_FAIL,                     /**< [PTP] Fail to add ts to buffer */
    CTC_E_PTP_RX_TS_INFORMATION_NOT_READY,         /**< [PTP] RX TS from sync interface or tod interface is not ready */
    CTC_E_PTP_EXCEED_MAX_DRIFT_VALUE,               /**< [PTP] Exceed max drift value */
    CTC_E_PTP_TAI_LESS_THAN_GPS,                    /**< [PTP] TAI second is less than gps second */
    CTC_E_PTP_CAN_NOT_SET_TOW,                      /**< [PTP] ToW can not be set at this time */
    CTC_E_PTP_SYNC_SIGNAL_FREQUENCY_OUT_OF_RANGE, /**< [PTP] Signal frequency is out of range*/
    CTC_E_PTP_SYNC_SIGNAL_NOT_VALID,                /**< [PTP] Invalid signal frequency*/
    CTC_E_PTP_SYNC_SIGNAL_DUTY_OUT_OF_RANGE,       /**< [PTP] Signal duty is out of range*/
    CTC_E_PTP_INVALID_SYNC_INTF_ACCURACY_VALUE,    /**< [PTP] Invalid sync interface accuracy value*/
    CTC_E_PTP_INVALID_SYNC_INTF_EPOCH_VALUE,        /**< [PTP] Invalid sync interface epoch value*/
    CTC_E_PTP_INVALID_TOD_INTF_PARAM_VALUE,       /**< [PTP] Invalid tod interface param value*/
    CTC_E_PTP_INVALID_TOD_INTF_EPOCH_VALUE,        /**< [PTP] Invalid tod interface epoch value*/
    CTC_E_PTP_INVALID_TOD_INTF_PPS_STATUS_VALUE,  /**< [PTP] Invalid tod interface 1PPS status value*/
    CTC_E_PTP_INVALID_TOD_INTF_CLOCK_SRC_VALUE,  /**< [PTP] Invalid tod interface clock source value*/
    CTC_E_PTP_INVALID_CAPTURED_TS_SEQ_ID,        /**< [PTP] Invalid captured ts sequece id*/
    CTC_E_PTP_INVALID_CAPTURED_TS_SOURCE,        /**< [PTP] Invalid captured ts source*/
    CTC_E_PTP_INVALID_CAPTURED_TYPE,             /**< [PTP] Invalid captured ts type*/

    /**< learning and aging */
    CTC_E_LEARNING_AND_AGING_INVALID_AGING_THRESHOLD = -27700,  /**< [LEARNING_AGING] Invalid aging threshold */
    CTC_E_LEARNING_AND_AGING_INVALID_LEARNING_THRESHOLD,        /**< [LEARNING_AGING] Invalid learning threshold */
    CTC_E_AGING_INVALID_INTERVAL,                               /**< [LEARNING_AGING] Invalid aging interval */
    CTC_E_INVALID_MAC_FILTER_SIZE,                              /**< [LEARNING_AGING] Invalid mac filter size */

    /**< memory allcation */
    CTC_E_ALLOCATION_INVALID_ENTRY_SIZE = -27600,   /**< [FTM] Entry size is invalid,must be multiple of 256 (80bit) */
    CTC_E_ALLOCATION_INVALID_KEY_SIZE,              /**< [FTM] Key size is invalid  */
    CTC_E_ALLOCATION_EXCEED_INT_TCAM_PHYSIZE,       /**< [FTM] Internal tcam physize exceed */
    CTC_E_ALLOCATION_EXCEED_EXT_TCAM_PHYSIZE,       /**< [FTM] External tcam physize exceed */
    CTC_E_ALLOCATION_EXCEED_HASH_PHYSIZE,           /**< [FTM] Hash physize exceed max*/
    CTC_E_ALLOCATION_INVALID_HASH_PHYSIZE,          /**< [FTM] Hash physize is invalid */
    CTC_E_ALLOCATION_EXCEEED_SRAM_PHYSIZE,          /**< [FTM] Sram physize exceed max*/
    CTC_E_ALLOCATION_EXCEED_MAX_OAM,                /**< [FTM] Oam num exceed max */
    CTC_E_HASH_MEM_INIT_TIMEOUT,                    /**< [FTM] Hash ram init timeout */

    /**< mirror */
    CTC_E_MIRROR_EXCEED_SESSION = -27500,           /**< [Mirror] Exceed mirror session num */
    CTC_E_MIRROR_INVALID_MIRROR_INFO_TYPE,          /**< [Mirror] Invalid mirror info type */
    CTC_E_MIRROR_INVALID_MIRROR_LOG_ID,             /**< [Mirror] Invalid mirror log id */

    /**< mpls */
    CTC_E_MPLS_ENTRY_NOT_SUPPORT_STATS = -27400,    /**< [MPLS] Entry not support statistics */
    CTC_E_MPLS_ENTRY_STATS_EXIST,                   /**< [MPLS] Statistics entry exist */
    CTC_E_MPLS_ENTRY_STATS_NOT_EXIST,               /**< [MPLS] Statistics entry not exist */
    CTC_E_MPLS_NO_RESOURCE,                      /**< [MPLS] No resource */
    CTC_E_MPLS_SPACE_ERROR,                  /**< [MPLS] Space don't enable */
    CTC_E_MPLS_LABEL_ERROR,                  /**< [MPLS] Label  don't correct */
    CTC_E_MPLS_LABEL_TYPE_ERROR,                  /**< [MPLS] Label Type don't correct */
    CTC_E_MPLS_VRF_ID_ERROR,                  /**< [MPLS] VRF ID don't correct */
    CTC_E_MPLS_SPACE_NO_RESOURCE,       /**< [MPLS] User allocate num larger than sdk allocate, please modify chip_profile.cfg */
    CTC_E_MPLS_LABEL_RANGE_ERROR,       /**< [MPLS] User allocate label range error */

    /**< dma */
    CTC_E_DMA_TX_FAILED = -27200,                   /**< [DMA] Packet DMA transmit failed */
    CTC_E_DMA_TABLE_WRITE_FAILED,                   /**< [DMA] Table DMA write failed */
    CTC_E_DMA_TABLE_READ_FAILED,                   /**< [DMA] Table DMA read failed */
    CTC_E_DMA_DESC_INVALID,                         /**< [DMA] Invalid descriptor */
    CTC_E_DMA_INVALID_PKT_LEN,                      /**< [DMA] Invalid length of packet */
    CTC_E_DMA_NOT_INIT,                      /**< [DMA] DMA Module is not init */
    CTC_E_DMA_INVALID_SUB_TYPE,                      /**< [DMA] Invalid DMA sub isr type */
    CTC_E_DMA_INVALID_CHAN_ID,                      /**< [DMA] Invalid DMA channel ID */
    CTC_E_DMA_EXCEED_MAX_DESC_NUM,                      /**< [DMA] Exceed Max Desc num */
    CTC_E_DMA_DESC_NOT_DONE,                         /**<[DMA] Desc not done */

    /**< parser */
    CTC_E_PARSER_INVALID_L3_TYPE = -27100,           /**< [Parser] Layer3 type is invalid */

    /**< stacking */
    CTC_E_STK_NOT_INIT = -27000,                   /**< [STACKING] Module not initialized */
    CTC_E_STK_INVALID_TRUNK_ID,                    /**< [STACKING] Trunk id is invalid */
    CTC_E_STK_TRUNK_NOT_EXIST,                     /**< [STACKING] Trunk not exist */
    CTC_E_STK_TRUNK_HAS_EXIST,                     /**< [STACKING] Trunk have exist */
    CTC_E_STK_TRUNK_EXCEED_MAX_MEMBER_PORT,        /**< [STACKING] Trunk exceed max member port*/
    CTC_E_STK_TRUNK_MEMBER_PORT_NOT_EXIST,         /**< [STACKING] Trunk member port not exist*/
    CTC_E_STK_TRUNK_MEMBER_PORT_EXIST,             /**< [STACKING] Trunk member port exist*/
    CTC_E_STK_TRUNK_MEMBER_PORT_INVALID,           /**< [STACKING] Trunk member port not valid*/
    CTC_E_STK_TRUNK_HDR_DSCP_NOT_VALID,            /**< [STACKING] Trunk header dscp not valid*/
    CTC_E_STK_TRUNK_HDR_COS_NOT_VALID,             /**< [STACKING] Trunk header cos not valid*/
    CTC_E_STK_KEEPLIVE_TWO_MEMBER_AND_ONE_CPU,     /**< [STACKING] KeepLive only support two member trunk and one cpu port*/

    /**< SYNCE */
    CTC_E_SYNCE_NOT_INIT = -26900,                 /**< [SYNCE] Module not initialized */
    CTC_E_SYNCE_CLOCK_ID_EXCEED_MAX_VALUE,         /**< [SYNCE] Clock ID exceed max value */
    CTC_E_SYNCE_DIVIDER_EXCEED_MAX_VALUE,          /**< [SYNCE] Divider exceed max value */
    CTC_E_SYNCE_INVALID_RECOVERED_PORT,            /**< [SYNCE] Recovered clock port ID must be integer multiples of 4 */
    CTC_E_SYNCE_NO_SERDES_INFO,                    /**< [SYNCE] Can not get serdes info */

    /**< interrupt */
    CTC_E_INTR_NOT_INIT = -26800,                  /**< [INTERRUPT] Module not initialized */
    CTC_E_INTR_INVALID_TYPE,                       /**< [INTERRUPT] sup-level type is invalid */
    CTC_E_INTR_INVALID_SUB_TYPE,                   /**< [INTERRUPT] sub-level type is invalid */
    CTC_E_INTR_NOT_SUPPORT,                        /**< [INTERRUPT] operation does not support */
    CTC_E_INTR_INVALID_PARAM,                      /**< [INTERRUPT] parameter is invalid */

    /**< storm ctl */
    CTC_E_STMCTL_INVALID_UC_MODE = -26700,         /**< [STMCTL] Invalid unicast storm ctl mode */
    CTC_E_STMCTL_INVALID_MC_MODE,                  /**< [STMCTL] Invalid mcast storm ctl mode */
    CTC_E_STMCTL_INVALID_THRESHOLD,                /**< [STMCTL] Invalid storm ctl threshold */
    CTC_E_STMCTL_INVALID_PKT_TYPE,                 /**< [STMCTL] Invalid storm ctl packet type */

    /**< bpe */
    CTC_E_BPE_NOT_INIT = -26600,                   /**< [BPE] Module not initialized */
    CTC_E_BPE_INVALID_PORT_BASE,                   /**< [BPE] Invalid BPE port base */
    CTC_E_BPE_INVALID_VLAN_BASE,                   /**< [BPE] Invalid BPE vlan base */
    CTC_E_BPE_INVALID_BPE_CAPABLE_GPORT,           /**< [BPE] Invalid BPE capable gport */
    CTC_E_BPE_INVLAID_BPE_MODE,                    /**< [BPE] Invalid BPE mode */

    /**< overlay tunnel */
    CTC_E_OVERLAY_TUNNEL_INVALID_TYPE = -26500,    /**< [OVERLAY_TUNNEL] The tunnel type is not correct*/
    CTC_E_OVERLAY_TUNNEL_INVALID_VN_ID,            /**< [OVERLAY_TUNNEL] This Virtual subnet ID is out of range or not assigned*/
    CTC_E_OVERLAY_TUNNEL_INVALID_FID,              /**< [OVERLAY_TUNNEL] This Virtual FID is out of range*/
    CTC_E_OVERLAY_TUNNEL_VN_FID_NOT_EXIST,         /**< [OVERLAY_TUNNEL] This Virtual subnet's FID has not assigned*/
    CTC_E_OVERLAY_TUNNEL_NO_DA_INDEX_RESOURCE,     /**< [OVERLAY_TUNNEL] The register to store IP DA index is full*/

    /**< datapath */
    CTC_E_DATAPATH_HSS_NOT_READY = -26400,                   /**< [DATAPATH] Hss can not ready */
    CTC_E_DATAPATH_CLKTREE_CFG_ERROR,                   /**< [DATAPATH] Clktree cfg error */
    CTC_E_DATAPATH_INVALID_PORT_TYPE,                   /**< [DATAPATH] invalid port type */
    CTC_E_DATAPATH_INVALID_CORE_FRE,                   /**< [DATAPATH] invalid core frequency */
    CTC_E_DATAPATH_PLL_NOT_LOCK,                         /**< [DATAPATH] pll cannot lock */
    CTC_E_DATAPATH_PLL_EXIST_LANE,                         /**< [DATAPATH] pll cannot change */
    CTC_E_DATAPATH_MAC_ENABLE,                         /**< [DATAPATH] mac is enable, cannot switch or 3ap training */
    CTC_E_DATAPATH_SWITCH_FAIL,                         /**< [DATAPATH] dynamic switch fail */

    CTC_E_ERROE_CODE_END = -10000,                  /**< End error code*/
};
typedef enum ctc_err_old_e ctc_err_old_t;


#define CTC_ERROR_RETURN(op) \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            if (g_error_on) \
            sal_printf("Error Happened!! Fun:%s()  Line:%d ret:%d\n",__FUNCTION__,__LINE__, rv); \
            return (rv); \
        } \
    }
#define CTC_ERROR_GOTO(op, ret, label) \
    { \
        ret = (op); \
        if (ret < 0) \
        { \
            if (g_error_on) \
            sal_printf("Error Happened!! Fun:%s()  Line:%d\n",__FUNCTION__,__LINE__); \
            goto label; \
        } \
    }
#define CTC_ERROR_RETURN_WITH_UNLOCK(op, lock) \
    { \
        int32 rv; \
        if ((rv = (op)) < 0) \
        { \
            if (g_error_on) \
            sal_printf("Error Happened!! Fun:%s()  Line:%d\n",__FUNCTION__,__LINE__); \
            sal_mutex_unlock(lock); \
            return (rv); \
        }; \
    }
#define CTC_API_ERROR_RETURN(api, lchip, ...) \
{ \
    int32 ret = CTC_E_NOT_SUPPORT; \
    if (lchip >= CTC_MAX_LOCAL_CHIP_NUM) \
    { \
        return CTC_E_INVALID_CHIP_ID; \
    } \
    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) != CTC_WB_STATUS_DONE)) \
    { \
        return CTC_E_WB_BUSY; \
    } \
    ret = api? api(lchip, ##__VA_ARGS__) : CTC_E_NOT_SUPPORT; \
    ret = ( ret < 0 )? ctc_error_code_mapping(ret) : ret; \
    return ret;\
}

#define CTC_API_INIT_ERROR_RETURN(api, lchip, ...) \
{ \
    int32 ret = CTC_E_NOT_SUPPORT; \
    if (lchip >= CTC_MAX_LOCAL_CHIP_NUM) \
    { \
        return CTC_E_INVALID_CHIP_ID; \
    } \
    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_SYNC)) \
    { \
        return CTC_E_WB_BUSY; \
    } \
    ret = api? api(lchip, ##__VA_ARGS__) : CTC_E_NOT_SUPPORT; \
    ret = ( ret < 0 )? ctc_error_code_mapping(ret) : ret; \
    return ret;\
}

extern const char*
ctc_get_error_desc(int32 error_code);

extern int32
ctc_error_code_mapping(int32 error_code);

extern int32
ctc_set_error_code_mapping_en(uint8 enable);

/**@} end of @defgroup  error ERROR */

#ifdef __cplusplus
}
#endif

#endif

