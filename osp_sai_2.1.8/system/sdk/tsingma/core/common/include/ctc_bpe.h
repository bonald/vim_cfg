/**
 @file ctc_bpe.h

 @author Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2013-05-28

 @version v2.0

This file contains all bpe related data structure, enum, macro and proto.

*/
#ifndef _CTC_BPE_H
#define _CTC_BPE_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "common/include/ctc_const.h"

/**
 @defgroup bpe BPE
 @{
*/
/****************************************************************
*
* Defines and Macros
*
****************************************************************/
/**
 @brief  Define the mode of bpe group
*/

#define CTC_BPE_ETAG_TPID  0x893F
#define CTC_BPE_EVB_TPID   0x8100

enum ctc_bpe_intlk_mode_e
{
    CTC_BPE_INTLK_PKT_MODE = 0,    /**< [GB] Interlaken use pkt mode*/
    CTC_BPE_INTLK_SEGMENT_MODE,    /**< [GB] Interlaken use segment mode*/
    CTC_BPE_INTLK_MAX_MODE         /**< [GB] Max interlaken mode*/
};
typedef enum ctc_bpe_intlk_mode_e  ctc_bpe_intlk_mode_t;


/**
 @brief bpe global config information
*/
struct ctc_bpe_global_cfg_s
{
    uint8   intlk_mode;       /**< [GB] ctc_bpe_intlk_mode_t*/
    uint8   is_port_extender; /**< [GG.D2.TM] If set,indicate it is a 802.1 BR PE*/
    uint16  max_uc_ecid_id;   /**< [GG.D2.TM] PE support the max number of ucast's ECID*/

    uint16  max_mc_ecid_id;   /**< [GG.D2.TM] PE support the max number of mcast's ECID*/
    uint16  port_base;        /**< [GG.D2.TM] EVB, 802.2BR and mux/demux's port base*/

};
typedef struct ctc_bpe_global_cfg_s  ctc_bpe_global_cfg_t;

/**
 @brief bpe port extender type
*/
enum ctc_bpe_externder_mode_e
{
    CTC_BPE_MUX_DEMUX_MODE,     /**< [GB.GG.D2.TM] Mux/Demux */
    CTC_BPE_8021QBG_M_VEPA,     /**< [GG.D2.TM] Multi-channel VEPA */
    CTC_BPE_8021BR_CB_CASCADE,  /**< [GG.D2.TM] Cascade port of CB */
    CTC_BPE_8021BR_PE_CASCADE,  /**< [GG.D2.TM] Cascade port of PE */
    CTC_BPE_8021BR_EXTENDED,    /**< [GG.D2.TM] Extended port of PE */
    CTC_BPE_8021BR_UPSTREAM,    /**< [GG.D2.TM] Upstream port of PE */
    CTC_BPE_MAX_EXTERNDER_MODE
};
typedef enum ctc_bpe_externder_mode_e  ctc_bpe_externder_mode_t;

/**
 @brief bpe port mux/demux config information
*/
struct ctc_bpe_extender_s
{
    uint8  mode;            /**< [GB.GG.D2.TM] ctc_bpe_externder_mode_t */
    uint8  port_num;        /**< [GB.GG.D2.TM] Port extender number on the physical port */
    uint16 vlan_base;       /**< [GB.GG.D2.TM] Vlan base in mux/demux ,E-VID base in 802.1qbg, E-CID base in 802.1br */

    uint16 port_base;       /**< [GB.GG.D2.TM] The lport base */
    uint8  enable;          /**< [GB.GG.D2.TM] Enable */
    uint8  rsv;
};
typedef struct ctc_bpe_extender_s ctc_bpe_extender_t;



enum ctc_bpe_forward_flag_e
{
    CTC_BPE_FORWARD_MCAST                 = 0x00000001,          /**< [D2.TM] BPE forward to mcast group*/
    CTC_BPE_FORWARD_COPY_TO_CPU           = 0x00000002,          /**< [D2.TM] BPE forward with copy to cpu*/
    CTC_BPE_FORWARD_REDIRECT_TO_CPU       = 0x00000004,          /**< [D2.TM] BPE forward with redirect to cpu*/
    CTC_BPE_FORWARD_EXTEND_PORT           = 0x00000008,          /**< [D2.TM] BPE forward to extend port*/
    CTC_BPE_FORWARD_MAX
};
typedef enum ctc_bpe_forward_flag_e  ctc_bpe_forward_flag_t;


/* Extender forwarding for 802.1BR PE device */
struct ctc_bpe_8021br_forward_s
{
    uint32 flag;                        /**< [D2.TM] Refer to ctc_bpe_forward_flag_t */
    uint16 name_space;                  /**< [D2.TM] Extender Namespace. */
    uint16 ecid;                        /**< [D2.TM] Extended destination E-CID. */
    uint32 dest_port;                   /**< [D2.TM] Destination Gport which used in downstream or upstream */
    uint32 mc_grp_id;                   /**< [D2.TM] Destination Multicast Group which used in downstream*/
};
typedef struct ctc_bpe_8021br_forward_s ctc_bpe_8021br_forward_t;

/**
@} end of @defgroup   bpe BPE */

#ifdef __cplusplus
}
#endif

#endif
