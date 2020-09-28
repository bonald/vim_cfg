#if (FEATURE_MODE == 0)
/**
 @file ctc_dot1ae.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2010-2-4

 @version v2.0

   This file contains all dot1ae related data structure, enum, macro and proto.
*/

#ifndef _CTC_DOT1AE_H_
#define _CTC_DOT1AE_H_
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
/**
 @defgroup dot1ae DOT1AE
 @{
*/
#define CTC_DOT1AE_KEY_LEN             32           /**< [D2.TM] 802.1ae key length*/
#define CTC_DOT1AE_SALT_LEN            12           /**< [TM] 802.1ae SALT length*/

/**
 @brief Dot1ae encryption offset
*/
enum ctc_dot1ae_encrypt_offset_e
{
    CTC_DOT1AE_ENCRYPTION_OFFSET_0,   /**< [D2.TM] Encrypt all user data*/
    CTC_DOT1AE_ENCRYPTION_OFFSET_30,   /**< [D2.TM] Encrypt user data from offset 30*/
    CTC_DOT1AE_ENCRYPTION_OFFSET_50,   /**< [D2.TM] Encrypt user data from offset 50*/
    CTC_DOT1AE_ENCRYPTION_OFFSET_NUM
};
typedef enum ctc_dot1ae_encrypt_offset_e ctc_dot1ae_encrypt_offset_t;

/**
 @brief Dot1ae secure channel direction
*/
enum ctc_dot1ae_sc_dir_e
{
    CTC_DOT1AE_SC_DIR_TX,     /**< [D2.TM] TX SC */
    CTC_DOT1AE_SC_DIR_RX     /**< [D2.TM] RX SC */
};
typedef enum ctc_dot1ae_sc_dir_e ctc_dot1ae_sc_dir_t;

/**
 @brief Dot1ae secure association flag
*/
enum ctc_dot1ae_sa_flag_e
{
    CTC_DOT1AE_SA_FLAG_NEXT_PN            = 0x00000001,      /**< [D2.TM]If set,indicate it is valid */
    CTC_DOT1AE_SA_FLAG_KEY                = 0x00000002,      /**< [D2.TM]If set,indicate it is valid */
    CTC_DOT1AE_SA_FLAG_ENCRYPTION         = 0x00000004,      /**< [D2.TM]If set,indicate  encryption option */
    CTC_DOT1AE_SA_FLAG_ICV_ERROR_ACTION   = 0x00000008,      /**< [D2]If set,indicate it is valid */
    CTC_DOT1AE_SA_FLAG_VALIDATEFRAMES     = 0x00000010       /**< [TM]If set, validateframes is valid */

};
typedef enum ctc_dot1ae_sa_flag_e ctc_dot1ae_sa_flag_t;

/**
 @brief Dot1ae cipher suite
*/
enum ctc_dot1ae_cipher_suite_e
{
    CTC_DOT1AE_CIPHER_SUITE_GCM_AES_128,
    CTC_DOT1AE_CIPHER_SUITE_GCM_AES_256,
    CTC_DOT1AE_CIPHER_SUITE_GCM_AES_XPN_128,
    CTC_DOT1AE_CIPHER_SUITE_GCM_AES_XPN_256,
    CTC_DOT1AE_CIPHER_SUITE_MAX
};
typedef enum ctc_dot1ae_cipher_suite_e ctc_dot1ae_cipher_suite_t;

/**
 @brief define validateframes mode
*/
enum ctc_dot1ae_validateframes_e
{
    CTC_DOT1AE_VALIDFRAMES_DISABLE,         /**< [TM] validateframes disable */
    CTC_DOT1AE_VALIDFRAMES_CHECK,           /**< [TM] validateframes check */
    CTC_DOT1AE_VALIDFRAMES_STRICT,          /**< [TM] validateframes strict */
    CTC_DOT1AE_VALIDFRAMES_MAX
};
typedef enum ctc_dot1ae_validateframes_e ctc_dot1ae_validateframes_t;

/**
 @brief Dot1ae secure association
*/
struct ctc_dot1ae_sa_s
{
    uint32 upd_flag;                 /**< [D2.TM] [rx/tx]update flag ctc_dot1ae_sa_flag_t*/
    uint8  an;                       /**< [D2.TM] [rx/tx] Association Number*/
    uint64 next_pn;                  /**< [D2.TM] [rx/tx] Packet Number, D2 max value is 0xffffffff*/
    uint64 lowest_pn;                /**< [D2.TM] [rx] [read only] Packet Number*/
    uint8  in_use;                   /**< [D2.TM] [rx/tx][read only] if set,indicate the an is in use.*/
    uint8  key[CTC_DOT1AE_KEY_LEN];  /**< [D2.TM] [rx/tx] key, 128 bits or 256 bits*/
    uint8  no_encryption;            /**< [D2.TM][rx/tx] if set,indicate packet is unencrypted*/
    uint16 confid_offset;            /**< [D2.TM][rx/tx] Encrypted packet confidentiality offset ,refer to ctc_dot1ae_encrypt_offset_t*/
    uint8  icv_error_action;         /**< [D2] [rx] icv check fail action, ctc_exception_type_t, valid when  CTC_DOT1AE_SA_FLAG_ICV_ERROR_ACTION is set*/
    uint8  cipher_suite;                /**< [TM] refer to ctc_dot1ae_cipher_suite_t valid when  icv_check_disable is 0*/
    uint8  salt[CTC_DOT1AE_SALT_LEN];   /**< [TM] [rx/tx] Salt for GCM_AES_XPN_128 and  GCM_AES_XPN_256, valid when  CTC_DOT1AE_SA_FLAG_KEY is set */
    uint32 ssci;                        /**< [TM] [rx/tx] Short SCI for GCM_AES_XPN_128 and  GCM_AES_XPN_256, valid when  CTC_DOT1AE_SA_FLAG_KEY is set*/
    uint8 validateframes;           /**< [TM] [rx] icv check mode, DISABLE, CHECK or STRICT*/
};
typedef struct ctc_dot1ae_sa_s ctc_dot1ae_sa_t;

/**
 @brief Dot1ae secure channel property
*/
enum ctc_dot1ae_sc_property_e
{
    CTC_DOT1AE_SC_PROP_AN_EN,               /**< [D2.TM][tx/rx] Enable AN bitmap to channel  */
    CTC_DOT1AE_SC_PROP_NEXT_AN,             /**< [D2.TM][tx] Enable NEXT AN channel (0~3) */
    CTC_DOT1AE_SC_PROP_INCLUDE_SCI,         /**< [D2.TM] [tx] Transmit SCI in MacSec Tag*/
    CTC_DOT1AE_SC_PROP_SCI,                 /**< [D2.TM] [tx] Secure Channel Identifier,64 bit.*/
    CTC_DOT1AE_SC_PROP_REPLAY_WINDOW_SIZE,  /**< [D2.TM] [rx] Replay protect windows size*/
    CTC_DOT1AE_SC_PROP_REPLAY_PROTECT_EN,      /**< [D2.TM] [rx] Replay protect enable or disable */
    CTC_DOT1AE_SC_PROP_SA_CFG,              /**< [D2.TM] [tx/rx] configure SA Property,ext_data: ctc_dot1ae_sa_t*/

};
typedef enum ctc_dot1ae_sc_property_e ctc_dot1ae_sc_property_t;

/**
 @brief Dot1ae secure channel
*/
struct ctc_dot1ae_sc_s
{
   uint32   chan_id;                /**< [D2.TM] bind Dot1AE SC ID to port,value = 0 is unbind*/
   uint8    dir;                    /**< [D2.TM] tx/rx*/
   uint8    property;               /**< [D2.TM] ctc_dot1ae_sc_property_t */
   uint64   data;                   /**< [D2.TM] Key Field data (uint64). */
   void*  ext_data;                 /**< [D2.TM] Key Field extend data (void*). */


};
typedef struct ctc_dot1ae_sc_s ctc_dot1ae_sc_t;

/**
 @brief mirrored packet type on dot1ae port
*/
enum ctc_dot1ae_port_mirror_mode_e
{
    CTC_DOT1AE_PORT_MIRRORED_MODE_PLAIN_TEXT = 0,   /**< [D2.TM] mirror plain pkt on dot1ae enabled port.This is default mode */
    CTC_DOT1AE_PORT_MIRRORED_MODE_ENCRYPTED = 1     /**< [D2.TM] mirror encrypted pkt on dot1ae enabled port*/
};
typedef enum ctc_dot1ae_port_mirror_mode_e ctc_dot1ae_port_mirror_mode_t;

/**
 @brief Secuirty data structure for Dot1AE global
*/
struct ctc_dot1ae_glb_cfg_s
{
    uint64 tx_pn_thrd;         /**< [D2.TM] Dot1AE Encrypt PN Threshold, D2 max value is 0xffffffff*/
    uint64 rx_pn_thrd;         /**< [D2.TM] Dot1AE Decrypt PN Threshold, D2 max value is 0xffffffff*/
    uint8  dot1ae_port_mirror_mode;      /**< [D2.TM] The Dot1AE port enable mirroring or random-log,
                                            which data packets (plaintext or ciphertext) will be mirrored. refer to ctc_dot1ae_port_mirror_mode_t */
    uint8  tx_pn_overflow_action; /**< [TM] Dot1ae pn overflow action, refer to ctc_exception_type_t.*/
    uint8  rsv[2];
};
typedef struct ctc_dot1ae_glb_cfg_s ctc_dot1ae_glb_cfg_t;

struct ctc_dot1ae_an_stats_s
{
    uint64 out_pkts_protected;      /**< [TM] protected packet count*/
    uint64 out_pkts_encrypted;      /**< [TM] encrypted packet count*/
    uint64 in_pkts_ok;              /**< [TM] valid packet count*/
    uint64 in_pkts_unchecked;       /**< [TM] unchecked packet count*/
    uint64 in_pkts_delayed;         /**< [TM] delayed packet count*/
    uint64 in_pkts_invalid;         /**< [TM] invalid packet count*/
    uint64 in_pkts_not_valid;       /**< [TM] not valid packet count*/
    uint64 in_pkts_late;            /**< [TM] late packet count*/
    uint64 in_pkts_not_using_sa;    /**< [TM] not using sa packet count*/
    uint64 in_pkts_unused_sa;       /**< [TM] unused sa packet count*/
}
;
typedef struct ctc_dot1ae_an_stats_s ctc_dot1ae_an_stats_t;


/**
 @brief  Define dot1ae stats
*/
struct ctc_dot1ae_stats_s
{
    ctc_dot1ae_an_stats_t an_stats[4];      /**< [TM] Dot1AE stats per channel*/
    uint64 in_pkts_no_sci;                  /**< [TM] Dot1AE global stats*/
    uint64 in_pkts_unknown_sci;             /**< [TM] Dot1AE global stats*/
};
typedef struct ctc_dot1ae_stats_s  ctc_dot1ae_stats_t;



/**@} end of @defgroup  dot1ae DOT1AE */

#ifdef __cplusplus
}
#endif

#endif

#endif

