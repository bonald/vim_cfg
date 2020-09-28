/**
 @file ctc_stats.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-12-15

 @version v2.0

   This file contains all stats related data structure, enum, macro and proto.
*/

#ifndef _CTC_STATS_H
#define _CTC_STATS_H
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
 @defgroup stats STATS
 @{
*/

/**
 @brief  Define stats type
*/
enum ctc_stats_type_e
{
    CTC_STATS_TYPE_FWD,                                 /**< [GB.GG.D2.TM] fwd stats*/
    CTC_STATS_TYPE_GMAC,                                /**< [GB.GG.D2.TM] gmac stats*/
    CTC_STATS_TYPE_SGMAC,                               /**< [GB.GG.D2.TM] sgmac stats*/
    CTC_STATS_TYPE_XQMAC,                               /**< [GG] xqmac stats*/
    CTC_STATS_TYPE_CGMAC,                               /**< [GG] cgmac stats*/
    CTC_STATS_TYPE_CPUMAC,                              /**< [GB] cpu mac stats*/

    CTC_STATS_TYPE_MAX
};
typedef enum ctc_stats_type_e ctc_stats_type_t;

/**
 @brief  Define MAC base stats property type
*/
enum  ctc_mac_stats_prop_type_e
{
    CTC_STATS_PACKET_LENGTH_MTU1,                  /**< [GB.GG.D2.TM] MTU1 packet length, packet length larger than this consider as oversized packet. Default is 1518B*/
    CTC_STATS_PACKET_LENGTH_MTU2,                  /**< [GB.GG.D2.TM] MTU2 packet length, packet length larger than this consider as jumbo packet. Default is 1536B*/

    CTC_STATS_MAC_STATS_PROP_MAX
};
typedef enum ctc_mac_stats_prop_type_e ctc_mac_stats_prop_type_t;

/**
 @brief  Define MAC base stats direction type
*/
enum  ctc_mac_stats_dir_e
{
    CTC_STATS_MAC_STATS_RX,                  /**< [GB.GG.D2.TM] Mac stats RX direction*/
    CTC_STATS_MAC_STATS_TX,                  /**< [GB.GG.D2.TM] Mac stats TX direction*/

    CTC_STATS_MAC_STATS_MAX
};
typedef enum ctc_mac_stats_dir_e ctc_mac_stats_dir_t;

/**
 @brief  Define MAC base stats direction type
*/
enum  ctc_stats_property_type_e
{
    CTC_STATS_PROPERTY_SATURATE,                    /**< [GB.GG.D2.TM] deal with general options saturate enable*/
    CTC_STATS_PROPERTY_HOLD,                        /**< [GB.GG.D2.TM] deal with general options hold enable*/
    CTC_STATS_PROPERTY_PKT_CNT_THREASHOLD,          /**< [GB] deal with general options pkt cnt threshold*/
    CTC_STATS_PROPERTY_BYTE_CNT_THREASHOLD,         /**< [GB] deal with general options byte cnt threshold*/
    CTC_STATS_PROPERTY_FIFO_DEPTH_THREASHOLD,       /**< [GB] deal with general options fifo depth threshold*/
    CTC_STATS_PROPERTY_CLASSIFY_FLOW_STATS_RAM,     /**< [D2.TM] configure flow stats ram distribution */

    CTC_STATS_PROPERTY_MAX
};
typedef enum ctc_stats_property_type_e ctc_stats_property_type_t;

/**
 @brief  Define stats color
*/
enum ctc_stats_color_e
{
    CTC_STATS_GREEN,                    /**< [GB] green color packet stats*/
    CTC_STATS_NOT_GREEN,                /**< [GB] not green color packet stats*/

    CTC_STATS_COLOR_MAX
};
typedef enum ctc_stats_color_e ctc_stats_color_t;

/**
 @brief  Define stats forward flag
*/
enum ctc_stats_fwd_stats_bitmap_e
{
    CTC_STATS_QUEUE_DEQ_STATS       = 0x0001,            /**< [GB] dequeue stats*/
    CTC_STATS_QUEUE_DROP_STATS      = 0x0002,            /**< [GB] drop queue stats. For GB, dequeue stats must be enabled*/
    CTC_STATS_PORT_POLICER_STATS    = 0x0004,            /**< [HB] port policer stats */
    CTC_STATS_SERVICE_POLICER_STATS = 0x0008,            /**< [HB] service policer stats */
    CTC_STATS_FLOW_POLICER_STATS    = 0x0010,            /**< [GB] flow policer stats*/
    CTC_STATS_FLOW_STATS            = 0x0020,            /**< [HB.GB] flow stats */
    CTC_STATS_VLAN_STATS            = 0x0040,            /**< [GB] vlan stats*/
    CTC_STATS_VRF_STATS             = 0x0080,            /**< [GB] vrf stats*/
    CTC_STATS_ECMP_STATS            = 0x0100,            /**< [GG.D2.TM] Ecmp group stats*/
    CTC_STATS_PORT_STATS            = 0x0200             /**< [D2.TM] Port stats*/
};
typedef enum ctc_stats_fwd_stats_bitmap_e ctc_stats_fwd_stats_bitmap_t;

/**
 @brief  Define stats discard flag
*/
enum ctc_stats_discard_e
{
    CTC_STATS_RANDOM_LOG_DISCARD_STATS      = 0x01,         /**< [GB.GG.D2.TM] random log stats discard pkt*/
    CTC_STATS_FLOW_DISCARD_STATS            = 0x02,         /**< [GB.GG.D2.TM] flow stats discard pkt*/

    CTC_STATS_MAX
};
typedef enum ctc_stats_discard_e ctc_stats_discard_t;

/**
 @brief Define stats mode
 */
enum ctc_stats_mode_e
{
    CTC_STATS_MODE_PLUS,          /**< [GB.GG.D2.TM] plus stats info correspond to plus hw stats info*/
    CTC_STATS_MODE_DETAIL,        /**< [GB.GG.D2.TM] detail stats info based on hw stats info*/
    CTC_STATS_MODE_NUM
};
typedef enum ctc_stats_mode_e ctc_stats_mode_t;

/**
 @brief  Define stats phb
*/
struct ctc_stats_phb_s
{
    ctc_stats_color_t color;        /**< [GB] phb color*/
    uint8 cos;                      /**< [GB] cos mapped from priority*/
    uint8 is_all_phb;               /**< [GB] If set this field, it means all phb stats. Then the color and cos fields are unuseful*/
    uint8 rsv[2];
};
typedef struct ctc_stats_phb_s ctc_stats_phb_t;

/**
 @brief  Define mac receive statistics storage structor
*/
struct ctc_stats_mac_rec_s
{
    uint64 good_ucast_pkts;              /**< [GB.GG.D2.TM] length equal to 64B to 1518B(no vlan) or 1522B(vlan),total number of unicast packets received without error*/
    uint64 good_ucast_bytes;             /**< [GB.GG.D2.TM] length equal to 64B to 1518B(no vlan) or 1522B(vlan),total bytes of unicast packets received without error*/
    uint64 good_mcast_pkts;              /**< [GB.GG.D2.TM] length equal to 64B to 1518B(no vlan) or 1522B(vlan),total number of multicast packets received without error*/
    uint64 good_mcast_bytes;             /**< [GB.GG.D2.TM] length equal to 64B to 1518B(no vlan) or 1522B(vlan),total bytes of multicast packets received without error*/
    uint64 good_bcast_pkts;              /**< [GB.GG.D2.TM] length equal to 64B to 1518B(no vlan) or 1522B(vlan),total number of broadcast packets received without error*/
    uint64 good_bcast_bytes;             /**< [GB.GG.D2.TM] length equal to 64B to 1518B(no vlan) or 1522B(vlan),total bytes of broadcast packets received without error*/
    uint64 good_pause_pkts;              /**< [D2.TM] total number of pause packets without error*/
    uint64 good_pause_bytes;             /**< [D2.TM] total bytes of pause packets without error*/
    uint64 good_normal_pause_pkts;       /**< [GB.GG.D2.TM] total number of nomal pause packets without error*/
    uint64 good_normal_pause_bytes;      /**< [GB.GG.D2.TM] total bytes of nomal pause packets without error*/
    uint64 good_pfc_pause_pkts;          /**< [GB.GG.D2.TM] total number of pfc pause packets without error*/
    uint64 good_pfc_pause_bytes;         /**< [GB.GG.D2.TM] total bytes of pfc pause packets without error*/
    uint64 good_control_pkts;            /**< [GB.GG.D2.TM] total number of control packets without error excluding the pause packet*/
    uint64 good_control_bytes;           /**< [GB.GG.D2.TM] total bytes of control packets without error excluding the pause packet*/
    uint64 jabber_pkts;                  /**< [HB] no mac overrun,length greater than MTU,total number of packets with fcs error or alignment error*/
    uint64 jabber_bytes;                 /**< [HB] no mac overrun,length greater than MTU,total bytes of packets with fcs error or alignment error*/
    uint64 collision_pkts;               /**< [HB] no mac overrun,length less than 64B,total number of packets with fcs error or alignment error*/
    uint64 collision_bytes;              /**< [HB] no mac overrun,length less than 64B,total bytes of packets with fcs error or alignment error*/
    uint64 fcs_error_pkts;               /**< [GB.GG.D2.TM] no mac overrun,length equal to 64B to MTU,total number of packets with fcs error*/
    uint64 fcs_error_bytes;              /**< [GB.GG.D2.TM] no mac overrun,length equal to 64B to MTU,total bytes of packets with fcs error*/
    uint64 alignment_error_pkts;         /**< [HB] no mac overrun,length equal to 64B to MTU,total number of packets with alignment error*/
    uint64 alignment_error_bytes;        /**< [HB] no mac overrun,length equal to 64B to MTU,total bytes of packets with alignment error*/
    uint64 mac_overrun_pkts;             /**< [GB.GG.D2.TM] total number of packets received with mac overrun condition*/
    uint64 mac_overrun_bytes;            /**< [GB.GG.D2.TM] total bytes of packets received with mac overrun condition*/
    uint64 good_oversize_pkts;           /**< [D2.TM] no mac overrun,length greater than 1518B(no vlan) or 1522B(vlan),total number of packets received without fcs error and alignment error*/
    uint64 good_oversize_bytes;          /**< [D2.TM] no mac overrun,length greater than 1518B(no vlan) or 1522B(vlan),total bytes of packets received without fcs error and alignment error*/
    uint64 good_undersize_pkts;          /**< [D2.TM] no mac overrun,length less than 64B,total number of packets received without fcs error and alignment error*/
    uint64 good_undersize_bytes;         /**< [D2.TM] no mac overrun,length less than 64B,total bytes of packets received without fcs error and alignment error*/
    uint64 gmac_good_oam_pkts;           /**< [HB] total number of packets without error*/
    uint64 gmac_good_oam_bytes;          /**< [HB] total bytes of packets without error*/
    uint64 good_63_pkts;                 /**< [GB.GG.D2.TM] total number of good packets received with length less than 64B without fcs error and late collision*/
    uint64 good_63_bytes;                /**< [GB.GG.D2.TM] total bytes of good packets received with length less than 64B without fcs error and late collision*/
    uint64 bad_63_pkts;                  /**< [GB.GG.D2.TM] total number of errored packets received with length less than 64B including fcs error and late collision*/
    uint64 bad_63_bytes;                 /**< [GB.GG.D2.TM] total bytes of errored packets received with length less than 64B including fcs error and late collision*/
    uint64 good_1519_pkts;               /**< [GB.GG.D2.TM] total number of good packets received with length equal to 1519B to MTU including fcs error and late collision*/
    uint64 good_1519_bytes;              /**< [GB.GG.D2.TM] total bytes of good packets received with length equal to 1519B to MTU including fcs error and late collision*/
    uint64 bad_1519_pkts;                /**< [GB.GG.D2.TM] total number of errored packets received with length equal to 1519B to MTU including fcs error*/
    uint64 bad_1519_bytes;               /**< [GB.GG.D2.TM] total bytes of errored packets received with length equal to 1519B to MTU including fcs error*/
    uint64 good_jumbo_pkts;              /**< [GB.GG.D2.TM] total number of good packets received with length greater than MTU including fcs error*/
    uint64 good_jumbo_bytes;             /**< [GB.GG.D2.TM] total bytes of good packets received with length greater than MTU including fcs error*/
    uint64 bad_jumbo_pkts;               /**< [GB.GG.D2.TM] total number of errored packets received with length greater than MTU including fcs error*/
    uint64 bad_jumbo_bytes;              /**< [GB.GG.D2.TM] total bytes of errored packets received with length greater than MTU including fcs error*/
    uint64 pkts_64;                      /**< [GB.GG.D2.TM] total number of packets received with length equal to 64B including fcs error and late collision*/
    uint64 bytes_64;                     /**< [GB.GG.D2.TM] total bytes of packets received with length equal to 64B including fcs error and late collision*/
    uint64 pkts_65_to_127;               /**< [GB.GG.D2.TM] total number of packets received with length equal to 65B to 127B including fcs error and late collision*/
    uint64 bytes_65_to_127;              /**< [GB.GG.D2.TM] total bytes of packets received with length equal to 65B to 127B including fcs error and late collision*/
    uint64 pkts_128_to_255;              /**< [GB.GG.D2.TM] total number of packets received with length equal to 128B to 255B including fcs error and late collision*/
    uint64 bytes_128_to_255;             /**< [GB.GG.D2.TM] total bytes of packets received with length equal to 128B to 255B including fcs error and late collision*/
    uint64 pkts_256_to_511;              /**< [GB.GG.D2.TM] total number of packets received with length equal to 256B to 511B including fcs error and late collision*/
    uint64 bytes_256_to_511;             /**< [GB.GG.D2.TM] total bytes of packets received with length equal to 256B to 511B including fcs error and late collision*/
    uint64 pkts_512_to_1023;             /**< [GB.GG.D2.TM] total number of packets received with length equal to 512B to 1023B including fcs error and late collision*/
    uint64 bytes_512_to_1023;            /**< [GB.GG.D2.TM] total bytes of packets received with length equal to 512B to 1023B including fcs error and late collision*/
    uint64 pkts_1024_to_1518;            /**< [GB.GG.D2.TM] total number of packets received with length equal to 1024B to 1518B including fcs error and late collision*/
    uint64 bytes_1024_to_1518;           /**< [GB.GG.D2.TM] total bytes of packets received with length equal to 1024B to 1518B including fcs error and late collision*/
    uint64 pkts_1519_to_2047;            /**< [D2.TM] total number of packets received with length equal to 1519B to 2047B including fcs error and late collision*/
    uint64 bytes_1519_to_2047;           /**< [D2.TM] total bytes of packets received with length equal to 1519B to 2047B including fcs error and late collision*/
};
typedef struct ctc_stats_mac_rec_s ctc_stats_mac_rec_t;

/**
 @brief  Define mac transmit statistics storage structor
*/
struct ctc_stats_mac_snd_s
{
    uint64 good_ucast_pkts;              /**< [GB.GG.D2.TM] total number of unicast packets transmitted without error*/
    uint64 good_ucast_bytes;             /**< [GB.GG.D2.TM] total bytes of unicast packets transmitted without error*/
    uint64 good_mcast_pkts;              /**< [GB.GG.D2.TM] total number of multicast packets transmitted without error*/
    uint64 good_mcast_bytes;             /**< [GB.GG.D2.TM] total bytes of multicast packets transmitted without error*/
    uint64 good_bcast_pkts;              /**< [GB.GG.D2.TM] total number of broadcast packets transmitted without error*/
    uint64 good_bcast_bytes;             /**< [GB.GG.D2.TM] total bytes of broadcast packets transmitted without error*/
    uint64 good_pause_pkts;              /**< [GB.GG.D2.TM] total number of pause packets transmitted without error*/
    uint64 good_pause_bytes;             /**< [GB.GG.D2.TM] total bytes of pause packets transmitted without error*/
    uint64 good_control_pkts;            /**< [GB.GG.D2.TM] total number of pause packets transmitted without error excluding the pause packet*/
    uint64 good_control_bytes;           /**< [GB.GG.D2.TM] total bytes of pause packets transmitted without error excluding the pause packet*/
    uint64 good_oam_pkts;                /**< [HB] total number of oam packets transmitted without error*/
    uint64 good_oam_bytes;               /**< [HB] total bytes of oam packets transmitted without error*/
    uint64 pkts_63;                      /**< [GB.GG.D2.TM] total number of packets transmitted with length less than 64B including fcs error and late collision*/
    uint64 bytes_63;                     /**< [GB.GG.D2.TM] total bytes of packets transmitted with length less than 64B including fcs error and late collision*/
    uint64 pkts_64;                      /**< [GB.GG.D2.TM] total number of packets transmitted with length equal to 64B including fcs error and late collision*/
    uint64 bytes_64;                     /**< [GB.GG.D2.TM] total bytes of packets transmitted with length equal to 64B including fcs error and late collision*/
    uint64 pkts_65_to_127;               /**< [GB.GG.D2.TM] total number of packets transmitted with length equal to 65B to 127B including fcs error and late collision*/
    uint64 bytes_65_to_127;              /**< [GB.GG.D2.TM] total bytes of packets transmitted with length equal to 65B to 127B including fcs error and late collision*/
    uint64 pkts_128_to_255;              /**< [GB.GG.D2.TM] total number of packets transmitted with length equal to 128B to 255B including fcs error and late collision*/
    uint64 bytes_128_to_255;             /**< [GB.GG.D2.TM] total bytes of packets transmitted with length equal to 128B to 255B including fcs error and late collision*/
    uint64 pkts_256_to_511;              /**< [GB.GG.D2.TM] total number of packets transmitted with length equal to 256B to 511B including fcs error and late collision*/
    uint64 bytes_256_to_511;             /**< [GB.GG.D2.TM] total bytes of packets transmitted with length equal to 256B to 511B including fcs error and late collision*/
    uint64 pkts_512_to_1023;             /**< [GB.GG.D2.TM] total number of packets transmitted with length equal to 512B to 1023B including fcs error and late collision*/
    uint64 bytes_512_to_1023;            /**< [GB.GG.D2.TM] total bytes of packets transmitted with length equal to 512B to 1023B including fcs error and late collision*/
    uint64 pkts_1024_to_1518;            /**< [GB.GG.D2.TM] total number of packets transmitted with length equal to 1024B to 1518B including fcs error and late collision*/
    uint64 bytes_1024_to_1518;           /**< [GB.GG.D2.TM] total bytes of packets transmitted with length equal to 1024B to 1518B including fcs error and late collision*/
    uint64 pkts_1519;                    /**< [GB.GG.] total number of packets transmitted with length equal to 1519B to MTU including fcs error and late collision*/
    uint64 bytes_1519;                   /**< [GB.GG] total bytes of packets transmitted with length equal to 1519B to MTU including fcs error and late collision*/
    uint64 jumbo_pkts;                   /**< [GB.GG.D2.TM] total number of packets transmitted with length greater than MTU including fcs error and late collision*/
    uint64 jumbo_bytes;                  /**< [GB.GG.D2.TM] total bytes of packets transmitted with length greater than MTU including fcs error and late collision*/
    uint64 mac_underrun_pkts;            /**< [GB.GG.D2.TM] total number of packets transmitted with mac underrun condition*/
    uint64 mac_underrun_bytes;           /**< [GB.GG.D2.TM] total bytes of packets transmitted with mac underrun condition*/
    uint64 fcs_error_pkts;               /**< [GB.GG.D2.TM] total number of packets transmitted with fcs error*/
    uint64 fcs_error_bytes;              /**< [GB.GG.D2.TM] total bytes of packets transmitted with fcs error*/
    uint64 pkts_1519_to_2047;            /**< [D2.TM] total number of packets transmitted with length equal to 1519B to 2047B including fcs error and late collision*/
    uint64 bytes_1519_to_2047;           /**< [D2.TM] total bytes of packets transmitted with length equal to 1519B to 2047B including fcs error and late collision*/
};
typedef struct ctc_stats_mac_snd_s ctc_stats_mac_snd_t;

/**
 @brief  Define cpu mac statistics storage structor
*/
struct ctc_stats_cpu_mac_s
{
    uint64 cpu_mac_rx_good_pkts;         /**< [GB.GG] the frames counter of the mac receive good packets*/
    uint64 cpu_mac_rx_good_bytes;        /**< [GB.GG] the bytes counter of the mac receive good packets*/
    uint64 cpu_mac_rx_bad_pkts;          /**< [GB.GG] the frames counter of the mac receive bad packets*/
    uint64 cpu_mac_rx_bad_bytes;         /**< [GB.GG] the bytes counter of the mac receive bad packets*/
    uint64 cpu_mac_rx_fragment_pkts;     /**< [GB.GG] the frames counter of the mac receive fragment packets*/
    uint64 cpu_mac_rx_fcs_error_pkts;    /**< [GB.GG] the frames counter of the mac receive fcs error packets*/
    uint64 cpu_mac_rx_overrun_pkts;      /**< [GB.GG] the frames counter of the mac receive overrun packets*/
    uint64 cpu_mac_tx_total_pkts;        /**< [GB.GG] the frames counter of the mac transmitted total packets*/
    uint64 cpu_mac_tx_total_bytes;       /**< [GB.GG] the bytes counter of the mac transmitted total packets*/
    uint64 cpu_mac_tx_fcs_error_pkts;    /**< [GB.GG] the frames counter of the mac transmitted fcs error packets*/
    uint64 cpu_mac_tx_underrun_pkts;     /**< [GB.GG] the frames counter of the mac transmitted underrun packets*/
};
typedef struct ctc_stats_cpu_mac_s ctc_stats_cpu_mac_t;

/**
 @brief  Define mac statistics receive plus storage structor
*/
struct ctc_stats_mac_rec_plus_s
{
    uint64 all_octets;      /**< [GB.GG.D2.TM] total bytes with all receive bytes*/
    uint64 all_pkts;        /**< [GB.GG.D2.TM] total packets with all receive pkts*/
    uint64 ucast_pkts;      /**< [GB.GG.D2.TM] total packets with unicast pkts*/
    uint64 mcast_pkts;      /**< [GB.GG.D2.TM] total packets with multicast pkts*/
    uint64 bcast_pkts;      /**< [GB.GG.D2.TM] total packets with broadcast pkts*/
    uint64 runts_pkts;      /**< [GB.GG.D2.TM] total packets with undersize pkts*/
    uint64 giants_pkts;     /**< [GB.GG.D2.TM] total packets with oversize pkts*/
    uint64 crc_pkts;        /**< [GB.GG.D2.TM] total packets with fcs erorr pkts*/
    uint64 overrun_pkts;    /**< [GB.GG.D2.TM] total packets with overrun pkts*/
    uint64 pause_pkts;      /**< [GB.GG.D2.TM] total packets with pause pkts*/
    uint64 fragments_pkts;  /**< [GB.GG.D2.TM] total packets with fragment pkts*/
    uint64 jabber_pkts;     /**< [GB.GG.D2.TM] total packets with jabber pkts*/
    uint64 jumbo_events;    /**< [GB.GG.D2.TM] total packets with jumbo pkts*/
    uint64 drop_events;     /**< [GB.GG.D2.TM] total packets with overrun pkts*/
    uint64 error_pkts;      /**< [GB.GG.D2.TM] total packets with fcs erorr, overrun pkts*/
};
typedef struct ctc_stats_mac_rec_plus_s ctc_stats_mac_rec_plus_t;

/**
 @brief  Define mac statistics transmit plus storage structor
*/
struct ctc_stats_mac_snd_plus_s
{
    uint64 all_octets;      /**< [GB.GG.D2.TM] total bytes with all transmit bytes*/
    uint64 all_pkts;        /**< [GB.GG.D2.TM] total packets with all transmit pkts*/
    uint64 ucast_pkts;      /**< [GB.GG.D2.TM] total packets with unicast pkts*/
    uint64 mcast_pkts;      /**< [GB.GG.D2.TM] total packets with multicast pkts*/
    uint64 bcast_pkts;      /**< [GB.GG.D2.TM] total packets with broadcast pkts*/
    uint64 underruns_pkts;  /**< [GB.GG.D2.TM] total packets with underrun pkts*/
    uint64 jumbo_events;    /**< [GB.GG.D2.TM] total packets with jumbo pkts*/
    uint64 error_pkts;      /**< [GB.GG.D2.TM] total packets with fcs erorr pkts*/
};
typedef struct ctc_stats_mac_snd_plus_s ctc_stats_mac_snd_plus_t;


/**
 @brief  Define basic statistics storage structor
*/
struct ctc_stats_basic_s
{
    uint64 packet_count;                  /**< [GB.GG.D2.TM] total number of packets*/
    uint64 byte_count;                    /**< [GB.GG.D2.TM] total bytes of packets*/
};
typedef struct ctc_stats_basic_s ctc_stats_basic_t;

/**
 @brief  Define Mac base stats property storage structor
*/
struct ctc_mac_stats_property_s
{
    union
    {
        uint16 length;       /**< [GB.GG.D2.TM] length of mtu packet*/
    } data;
    uint16 rsv0;
};
typedef struct ctc_mac_stats_property_s ctc_mac_stats_property_t;

/**
 @brief  Define Mac base detail stats property storage structor
*/
struct ctc_mac_stats_detail_s
{
    union
    {
        ctc_stats_mac_rec_t rx_stats;       /**< [GB.GG.D2.TM] mac receive statistics storage*/
        ctc_stats_mac_snd_t tx_stats;       /**< [GB.GG.D2.TM] mac send statistics storage*/
    } stats;
};
typedef struct ctc_mac_stats_detail_s ctc_mac_stats_detail_t;

/**
 @brief  Define Mac base plus stats property storage structor
*/
struct ctc_mac_stats_plus_s
{
    union
    {
        ctc_stats_mac_rec_plus_t rx_stats_plus;       /**< [GB.GG.D2.TM] mac receive statistics storage*/
        ctc_stats_mac_snd_plus_t tx_stats_plus;       /**< [GB.GG.D2.TM] mac send statistics storage*/
    } stats;
};
typedef struct ctc_mac_stats_plus_s ctc_mac_stats_plus_t;

/**
 @brief  Define Mac base stats property storage structor
*/
struct ctc_mac_stats_s
{
    uint8 stats_mode;  /**< [GB.GG.D2.TM] get stats mode, refer to ctc_stats_mode_t*/
    uint8 rsv[3];
    struct
    {
        ctc_mac_stats_plus_t stats_plus;     /**< [GB.GG.D2.TM] aggreate stats for system*/
        ctc_mac_stats_detail_t stats_detail; /**< [GB.GG.D2.TM] centec asic detail stats*/
    } u;
};
typedef struct ctc_mac_stats_s ctc_mac_stats_t;

/**
 @brief  Define Dma sync mac stats
*/
struct ctc_stats_mac_stats_sync_s
{
    uint8  valid_cnt;                                  /**< [GB.GG] num of phy port enable*/
    uint32 gport[CTC_MAX_PHY_PORT];                    /**< [GB.GG] gport id*/
    ctc_mac_stats_t stats[CTC_MAX_PHY_PORT];           /**< [GB.GG] mac stats on this gport*/
};
typedef struct ctc_stats_mac_stats_sync_s ctc_stats_mac_stats_sync_t;

typedef int32 (* ctc_stats_sync_fn_t)  (ctc_stats_mac_stats_sync_t*, void *userdata);

/**
 @brief  Define Mac base stats property storage structor
*/
struct ctc_cpu_mac_stats_s
{
    ctc_stats_cpu_mac_t cpu_mac_stats;     /**< [GB.GG] cpu mac stats for asic*/
    uint8 rsv0;
};
typedef struct ctc_cpu_mac_stats_s ctc_cpu_mac_stats_t;


/**
 @brief  Define Global Fwarding stats property storage structor
*/
struct ctc_stats_property_s
{
    union
    {
        bool enable;                 /**< [GB.GG.D2.TM] saturate enable, hold enable, clear after read value*/
        uint16 threshold_16byte;     /**< [GB] packet count threshold or byte count threshold value*/
        uint8 threshold_8byte;       /**< [GB] fifo depth threshold value*/
    } data;
};
typedef struct ctc_stats_property_s ctc_stats_property_t;

/**
 @brief  Define stats_id structor create
*/
enum ctc_stats_statsid_type_e
{
     CTC_STATS_STATSID_TYPE_VLAN,            /**< [GB.D2.TM] vlan stats, support ingress and egress*/
     CTC_STATS_STATSID_TYPE_VRF,             /**< [GB.GG.D2.TM] vrf stats*/
     CTC_STATS_STATSID_TYPE_ACL,             /**< [GB.GG.D2.TM] acl stats*/
     CTC_STATS_STATSID_TYPE_IPMC,            /**< [GB.GG.D2.TM] ipmc stats*/
     CTC_STATS_STATSID_TYPE_IP = CTC_STATS_STATSID_TYPE_IPMC,              /**< [D2.TM] ip stats*/
     CTC_STATS_STATSID_TYPE_MPLS,            /**< [GB.GG.D2.TM] mpls ilm stats, in ctc_stats_global_cfg_t.flow_pool_bmp means LSP*/
     CTC_STATS_STATSID_TYPE_MPLS_PW,         /**< [D2.TM] mpls PW type*/
     CTC_STATS_STATSID_TYPE_TUNNEL,          /**< [GB.GG.D2.TM] tunnel stats*/
     CTC_STATS_STATSID_TYPE_SCL,             /**< [GB.GG.D2.TM] scl stats*/
     CTC_STATS_STATSID_TYPE_NEXTHOP,         /**< [GB.GG.D2.TM] nexthop stats*/
     CTC_STATS_STATSID_TYPE_NEXTHOP_MPLS_PW, /**< [GG.D2.TM] nexthop mpls pw stats*/
     CTC_STATS_STATSID_TYPE_NEXTHOP_MPLS_LSP,/**< [GG.D2.TM] nexthop mpls lsp stats*/
     CTC_STATS_STATSID_TYPE_NEXTHOP_MCAST,   /**< [GG.D2.TM] mcast stats*/
     CTC_STATS_STATSID_TYPE_L3IF,            /**< [GB.GG.D2.TM] l3if stats. for gb, only support ingress subif*/
     CTC_STATS_STATSID_TYPE_FID,             /**< [GG.D2.TM] fid stats, invalid in ctc_stats_global_cfg_t.flow_pool_bmp*/
     CTC_STATS_STATSID_TYPE_ECMP,            /**< [GB.GG.D2.TM] ecmp group stats*/
     CTC_STATS_STATSID_TYPE_SDC,             /**< [D2.TM] sdc stats, invalid in ctc_stats_global_cfg_t.flow_pool_bmp*/
     CTC_STATS_STATSID_TYPE_MAC,             /**< [D2.TM] mac stats*/
     CTC_STATS_STATSID_TYPE_FLOW_HASH,       /**< [D2.TM] flow hash stats*/
     CTC_STATS_STATSID_TYPE_PORT,           /**< [D2.TM] only valid in ctc_stats_global_cfg_t.flow_ram_bmp*/
     CTC_STATS_STATSID_TYPE_POLICER0,       /**< [D2.TM] policer0 stats*/
     CTC_STATS_STATSID_TYPE_POLICER1,       /**< [D2.TM] policer1 stats*/

     CTC_STATS_STATSID_TYPE_MAX
};
typedef enum ctc_stats_statsid_type_e ctc_stats_statsid_type_t;

/**
 @brief  Define Global Fwarding stats property storage structor
*/
struct ctc_stats_property_param_s
{
    ctc_stats_property_type_t prop_type;    /**< [GB.GG.D2.TM] stats property type*/
    ctc_stats_type_t          stats_type;   /**< [GB.GG.D2.TM] stats type*/
    uint16  flow_ram_bmp[CTC_STATS_STATSID_TYPE_MAX][CTC_BOTH_DIRECTION];
                                            /**< [D2.TM] global shared stats ram bitmap, for ctc_stats_statsid_type_t except CTC_STATS_STATSID_TYPE_ACL;
                                                bit 0~3 : means  pool 0~3;
                                                to CTC_STATS_STATSID_TYPE_ACL, it means acl lookup 4~7 , valid when CTC_STATS_PROPERTY_CLASSIFY_FLOW_STATS_RAM*/
    uint16  acl_ram_bmp[CTC_MAX_ACL_LKUP_NUM][CTC_BOTH_DIRECTION];/**<[D2.TM] assign ram per acl lookup priority,
                                                                    valid when CTC_STATS_PROPERTY_CLASSIFY_FLOW_STATS_RAM*/

};
typedef struct ctc_stats_property_param_s ctc_stats_property_param_t;


#define CTC_STATS_MAX_STATSID 0xFFFFFFFF          /**< [GB.GG.D2.TM] valid statsid max value*/

struct ctc_stats_statsid_s
{
     ctc_stats_statsid_type_t type;           /**< [GB.GG.D2.TM] type for statsid*/
     ctc_direction_t dir;                     /**< [GB.GG.D2.TM] direction*/
     uint32      stats_id;                    /**< [GB.GG.D2.TM] statsid*/
     uint8       color_aware;                 /**< [D2.TM] for ACL stats, will do 3 stats(0:green 1:yellow 2:red) when color-aware enabled*/
     union
     {
          uint16 vlan_id;                     /**< [GB.D2.TM] vlan id*/
          uint16 vrf_id;                      /**< [GB] vrf id*/
          uint8  acl_priority;                /**< [GG.D2.TM] priority*/
          uint8  is_vc_label;                 /**< [GG.D2.TM] Indicate vc label*/
     }statsid;
};
typedef struct ctc_stats_statsid_s ctc_stats_statsid_t;

/**
 @brief  Define stats mode
*/
enum ctc_stats_flow_mode_e
{
     CTC_STATS_MODE_USER,                    /**< [GB.GG.D2.TM] Stats mode user config stats id*/
     CTC_STATS_MODE_DEFINE,                  /**< [GB.GG.D2.TM] Stats mode sdk config stats id*/
     CTC_STATS_FLOW_MODE_MAX
};
typedef enum ctc_stats_flow_mode_e ctc_stats_flow_mode_t;

/**
 @brief  Define stats mode
*/
enum ctc_stats_query_mode_e
{
     CTC_STATS_QUERY_MODE_IO,                /**< [GB.GG.D2.TM] Mac stats mode will reply instant mac stats*/
     CTC_STATS_QUERY_MODE_POLL,              /**< [GB.GG.D2.TM] Mac stats mode will reply poll mac stats, with some error*/
     CTC_STATS_QUERY_MODE_MAX
};
typedef enum ctc_stats_query_mode_e ctc_stats_query_mode_t;

/**
 @brief Global stats config info.
*/
struct ctc_stats_global_cfg_s
{
    uint8  stats_mode;              /**< [GB.GG.D2.TM] Referce to ctc_stats_flow_mode_t*/
    uint16 query_mode;              /**< [GB.GG.D2.TM] Mac stats used mode, refer to ctc_stats_query_mode_t*/
    uint16 stats_bitmap;            /**< [GB.GG.D2.TM] Supported stats bitmap, refer to ctc_stats_fwd_stats_bitmap_t*/
    uint16 policer_stats_num;       /**< [GB] policer number which supported stats, it should be alignment with 64*/
    uint32 mac_timer;               /**< [D2.TM] mac stats sync timer in seconds between 30 and 600, only valid in polling query mode*/
};
typedef struct ctc_stats_global_cfg_s ctc_stats_global_cfg_t;

/**@} end of @defgroup stats STATS  */

#ifdef __cplusplus
}
#endif

#endif

