/**
 @file ctc_aps.h

 @date 2010-3-10

 @version v2.0

 The file define all struct used in aps APIs.
*/

#ifndef _CTC_APS_H
#define _CTC_APS_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

/****************************************************************
*
* Defines and Macros
*
****************************************************************/

/**
 @addtogroup aps APS
 @{
*/
enum ctc_aps_flag_e
{
    CTC_APS_FLAG_IS_L2             = 0x00000001, /**< [GB.GG.D2.TM] Used for indicate l2 aps protection */
    CTC_APS_FLAG_IS_MPLS           = 0x00000002, /**< [GB.GG.D2.TM] Used for indicate mpls aps protection */
    MAX_CTC_APS_FLAG
};
typedef enum ctc_aps_flag_e ctc_aps_flag_t;

/**
 @brief Define APS type
*/
enum ctc_aps_type_e
{
    CTC_APS_DISABLE,   /**< [GB.GG.D2.TM] APS disable */
    CTC_APS_SELECT=2,     /**< [GB.D2.TM] APS selector */
    CTC_APS_BRIDGE=3,      /**< [GB.GG.D2.TM] APS bridge */
    CTC_APS_TYPE_MAX
};
typedef enum ctc_aps_type_e ctc_aps_type_t;


/**
 @brief Define MPLS APS Next Bridge group
*/
struct ctc_aps_next_aps_s
{
    uint16 w_group_id;                  /**< [GB.GG.D2.TM] Next working aps bridge group id */
    uint16 p_group_id;                  /**< [GB.GG.D2.TM] Next protection aps bridge group id */
};
typedef struct ctc_aps_next_aps_s ctc_aps_next_aps_t;

/**
 @brief Define APS Bridge group,For Mpls APS, APS Group ID and mpls Nexthop or mpls tunnel
   must be 1:1 relations,the gport and l3if_id comes from Mpls tunnel.
   so working_gport/protection_gport and w_l3if_id/p_l3if_id is invalid in Mpls APS.

*/
struct ctc_aps_bridge_group_s
{
    uint32 working_gport;               /**< [GB.GG.D2.TM] APS bridge working path for mpls aps indicate l3if id */
    uint32 protection_gport;            /**< [GB.GG.D2.TM] APS bridge protection path for mpls aps indicate l3if id */

    uint16 w_l3if_id;                   /**< [GB.GG.D2.TM] Working l3if id */
    uint16 p_l3if_id;                   /**< [GB.GG.D2.TM] Protection l3if id */

    ctc_aps_next_aps_t next_aps;        /**< [GB.GG.D2.TM] Next aps group */

    uint8 next_w_aps_en;                 /**< [GB.GG.D2.TM] Use next_w_aps neglect working_gport */
    uint8 next_p_aps_en;                 /**< [GB.GG.D2.TM] Use next_w_aps neglect protection_gport */
    uint8 protect_en;                    /**< [GB.GG.D2.TM] For bridge switch */
    uint8 aps_flag;                    /**< [GB.GG.D2.TM] APS flag,ctc_aps_flag_t */

    uint8 physical_isolated;          /**< [GB.GG.D2.TM] Indicate port aps use physical isolated path */

    uint8 raps_en;                       /**< [GB.GG.D2.TM] Indicate raps whether enable */
    uint16 raps_group_id;               /**< [GB.GG.D2.TM] Raps mcast group id */

    uint8 aps_failover_en;              /**< [GB.GG.D2.TM] Indicate link down use hw based port level aps */
    uint8 rsv[3];
};
typedef struct ctc_aps_bridge_group_s ctc_aps_bridge_group_t;

/**
 @brief Define R-APS member
*/
struct ctc_raps_member_s
{
    uint16 group_id;        /**< [GB.GG.D2.TM] Mcast group id */
    uint32 mem_port;        /**< [GB.GG.D2.TM] Member port if member is local member  gport:gchip(8bit) +local phy port(8bit);
                                            else if member is remote chip entry,gport: gchip(local) + remote gchip id(8bit)*/
    bool   remote_chip;     /**< [GB.GG.D2.TM] If set,member is remote chip entry */
    bool   remove_flag;     /**< [GB.GG.D2.TM] If set,remove member, else add member */

};
typedef struct ctc_raps_member_s ctc_raps_member_t;

/**@}*/ /*end of @addtogroup APS*/

#ifdef __cplusplus
}
#endif

#endif

