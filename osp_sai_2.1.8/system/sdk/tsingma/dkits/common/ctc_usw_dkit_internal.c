#include "sal.h"
#include "dal.h"
#include "ctc_cli.h"
#include "drv_api.h"
#include "ctc_dkit.h"
#include "ctc_dkit_common.h"
#include "usw/include/drv_chip_ctrl.h"
#include "usw/include/drv_common.h"
#include "ctc_usw_dkit.h"
#include "ctc_usw_dkit_drv.h"
#include "ctc_usw_dkit_internal.h"
#include "ctc_usw_dkit_interface.h"
#include "ctc_usw_dkit_misc.h"
#include "ctc_usw_dkit_monitor.h"

extern ctc_dkit_chip_api_t* g_dkit_chip_api[CTC_DKITS_MAX_LOCAL_CHIP_NUM];
extern int32
ctc_dt2_dkit_interface_show_pcs_status(uint8 lchip, uint16 mac_id);
extern int32
ctc_tm_dkit_interface_show_pcs_status(uint8 lchip, uint16 mac_id);
#define DKIT_STATS_MAC_BASED_STATS_XQMAC_RAM_DEPTH     144u
#define DKIT_STATS_XQMAC_SUBMAC_MAX                        4u
#define DKIT_STATS_XQMAC_RAM_MAX                        24u
#define DKIT_STATS_CGMAC_RAM_MAX                         4u


#define SLICE_ID(mac_id)   ((mac_id <= 55) ? 0 : 1)
#define PCS_OFFSET(mac_id) (((SLICE_ID(mac_id) ? (mac_id - 64) : mac_id) >= 48) ? 8 : 0)
/* slice0: 0~47, slice1:48~95 */
#define PCS_ID(mac_id)  (SLICE_ID(mac_id) ? ((mac_id - 16) - (PCS_OFFSET(mac_id))) : (mac_id - (PCS_OFFSET(mac_id))))

#define DKIT_STATS_IS_XQMAC_STATS(mac_ram_type) \
          ((DKIT_STATS_XQMAC_STATS_RAM0  == mac_ram_type)  || (DKIT_STATS_XQMAC_STATS_RAM1  == mac_ram_type)\
        || (DKIT_STATS_XQMAC_STATS_RAM2  == mac_ram_type)  || (DKIT_STATS_XQMAC_STATS_RAM3  == mac_ram_type)\
        || (DKIT_STATS_XQMAC_STATS_RAM4  == mac_ram_type)  || (DKIT_STATS_XQMAC_STATS_RAM5  == mac_ram_type)\
        || (DKIT_STATS_XQMAC_STATS_RAM6  == mac_ram_type)  || (DKIT_STATS_XQMAC_STATS_RAM7  == mac_ram_type)\
        || (DKIT_STATS_XQMAC_STATS_RAM8  == mac_ram_type)  || (DKIT_STATS_XQMAC_STATS_RAM9  == mac_ram_type)\
        || (DKIT_STATS_XQMAC_STATS_RAM10 == mac_ram_type)  || (DKIT_STATS_XQMAC_STATS_RAM11 == mac_ram_type)\
        || (DKIT_STATS_XQMAC_STATS_RAM12 == mac_ram_type)  || (DKIT_STATS_XQMAC_STATS_RAM13 == mac_ram_type)\
        || (DKIT_STATS_XQMAC_STATS_RAM14 == mac_ram_type)  || (DKIT_STATS_XQMAC_STATS_RAM15 == mac_ram_type)\
        || (DKIT_STATS_XQMAC_STATS_RAM16 == mac_ram_type)  || (DKIT_STATS_XQMAC_STATS_RAM17 == mac_ram_type)\
        || (DKIT_STATS_XQMAC_STATS_RAM18 == mac_ram_type)  || (DKIT_STATS_XQMAC_STATS_RAM19 == mac_ram_type)\
        || (DKIT_STATS_XQMAC_STATS_RAM20 == mac_ram_type)  || (DKIT_STATS_XQMAC_STATS_RAM21 == mac_ram_type)\
        || (DKIT_STATS_XQMAC_STATS_RAM22 == mac_ram_type)  || (DKIT_STATS_XQMAC_STATS_RAM23 == mac_ram_type))

#define DKIT_STATS_IS_CGMAC_STATS(mac_ram_type)\
        ((DKIT_STATS_CGMAC_STATS_RAM0 == mac_ram_type) || (DKIT_STATS_CGMAC_STATS_RAM1 == mac_ram_type)\
        || (DKIT_STATS_CGMAC_STATS_RAM2 == mac_ram_type) || (DKIT_STATS_CGMAC_STATS_RAM3 == mac_ram_type))

#define DKIT_XQMAC_PER_SLICE_NUM      12
#define DKIT_SUBMAC_PER_XQMAC_NUM      4
#define DKIT_SGMAC_PER_SLICE_NUM      (DKIT_XQMAC_PER_SLICE_NUM * DKIT_SUBMAC_PER_XQMAC_NUM)
#define DKIT_CGMAC_PER_SLICE_NUM       2


/* descriptor extended property structure */
struct entry_desc_s
{
    char desc[64];
};
typedef struct entry_desc_s entry_desc_t;

enum dkits_pcie_ctl_e
{
    DKIT_PCIE_CFG_R,
    DKIT_PCIE_CFG_W,
    DKIT_PCIE_IO_R,
    DKIT_PCIE_IO_W,
    DKIT_PCIE_MEM_R,
    DKIT_PCIE_MEM_W,
    DKIT_PCIE_MAX
};
typedef enum dkits_pcie_ctl_e dkits_pcie_ctl_t;

extern dal_op_t g_dal_op;

enum dkit_stats_mac_ram_e
{
    DKIT_STATS_XQMAC_STATS_RAM0,
    DKIT_STATS_XQMAC_STATS_RAM1,
    DKIT_STATS_XQMAC_STATS_RAM2,
    DKIT_STATS_XQMAC_STATS_RAM3,
    DKIT_STATS_XQMAC_STATS_RAM4,
    DKIT_STATS_XQMAC_STATS_RAM5,
    DKIT_STATS_XQMAC_STATS_RAM6,
    DKIT_STATS_XQMAC_STATS_RAM7,
    DKIT_STATS_XQMAC_STATS_RAM8,
    DKIT_STATS_XQMAC_STATS_RAM9,
    DKIT_STATS_XQMAC_STATS_RAM10,
    DKIT_STATS_XQMAC_STATS_RAM11,
    DKIT_STATS_XQMAC_STATS_RAM12,
    DKIT_STATS_XQMAC_STATS_RAM13,
    DKIT_STATS_XQMAC_STATS_RAM14,
    DKIT_STATS_XQMAC_STATS_RAM15,
    DKIT_STATS_XQMAC_STATS_RAM16,
    DKIT_STATS_XQMAC_STATS_RAM17,
    DKIT_STATS_XQMAC_STATS_RAM18,
    DKIT_STATS_XQMAC_STATS_RAM19,
    DKIT_STATS_XQMAC_STATS_RAM20,
    DKIT_STATS_XQMAC_STATS_RAM21,
    DKIT_STATS_XQMAC_STATS_RAM22,
    DKIT_STATS_XQMAC_STATS_RAM23,

    DKIT_STATS_CGMAC_STATS_RAM0,
    DKIT_STATS_CGMAC_STATS_RAM1,
    DKIT_STATS_CGMAC_STATS_RAM2,
    DKIT_STATS_CGMAC_STATS_RAM3,

    DKIT_STATS_MAC_STATS_RAM_MAX
};
typedef enum dkit_stats_mac_ram_e dkit_stats_mac_ram_t;

/*mac statistics type*/
enum dkit_stats_mac_rec_stats_type_e
{
    DKIT_STATS_MAC_RCV_GOOD_UCAST,
    DKIT_STATS_MAC_RCV_GOOD_MCAST,
    DKIT_STATS_MAC_RCV_GOOD_BCAST,
    DKIT_STATS_MAC_RCV_GOOD_NORMAL_PAUSE,
    DKIT_STATS_MAC_RCV_GOOD_PFC_PAUSE,
    DKIT_STATS_MAC_RCV_GOOD_CONTROL,
    DKIT_STATS_MAC_RCV_FCS_ERROR,
    DKIT_STATS_MAC_RCV_MAC_OVERRUN,
    DKIT_STATS_MAC_RCV_GOOD_63B,
    DKIT_STATS_MAC_RCV_BAD_63B,
    DKIT_STATS_MAC_RCV_GOOD_1519B,
    DKIT_STATS_MAC_RCV_BAD_1519B,
    DKIT_STATS_MAC_RCV_GOOD_JUMBO,
    DKIT_STATS_MAC_RCV_BAD_JUMBO,
    DKIT_STATS_MAC_RCV_64B,
    DKIT_STATS_MAC_RCV_127B,
    DKIT_STATS_MAC_RCV_255B,
    DKIT_STATS_MAC_RCV_511B,
    DKIT_STATS_MAC_RCV_1023B,
    DKIT_STATS_MAC_RCV_1518B,

    DKIT_STATS_MAC_RCV_MAX,
    DKIT_STATS_MAC_RCV_NUM = DKIT_STATS_MAC_RCV_MAX
};
typedef enum sys_stats_mac_rec_stats_type_e sys_stats_mac_rec_stats_type_t;

enum dkit_stats_mac_snd_stats_type_e
{
    DKIT_STATS_MAC_SEND_UCAST = 0x14,
    DKIT_STATS_MAC_SEND_MCAST,
    DKIT_STATS_MAC_SEND_BCAST,
    DKIT_STATS_MAC_SEND_PAUSE,
    DKIT_STATS_MAC_SEND_CONTROL,
    DKIT_STATS_MAC_SEND_FCS_ERROR,
    DKIT_STATS_MAC_SEND_MAC_UNDERRUN,
    DKIT_STATS_MAC_SEND_63B,
    DKIT_STATS_MAC_SEND_64B,
    DKIT_STATS_MAC_SEND_127B,
    DKIT_STATS_MAC_SEND_255B,
    DKIT_STATS_MAC_SEND_511B,
    DKIT_STATS_MAC_SEND_1023B,
    DKIT_STATS_MAC_SEND_1518B,
    DKIT_STATS_MAC_SEND_1519B,
    DKIT_STATS_MAC_SEND_JUMBO,

    DKIT_STATS_MAC_SEND_MAX,
    DKIT_STATS_MAC_STATS_TYPE_NUM = DKIT_STATS_MAC_SEND_MAX,
    DKIT_STATS_MAC_SEND_NUM = 16
};
typedef enum dkit_stats_mac_snd_stats_type_e dkit_stats_mac_snd_stats_type_t;

/* the value defined should be consistent with ctc_stats.h */
enum dkit_stats_map_mac_rx_type_e
{
    DKIT_STATS_MAP_MAC_RX_GOOD_UC_PKT = 0,
    DKIT_STATS_MAP_MAC_RX_GOOD_UC_BYTE = 1,
    DKIT_STATS_MAP_MAC_RX_GOOD_MC_PKT = 2,
    DKIT_STATS_MAP_MAC_RX_GOOD_MC_BYTE = 3,
    DKIT_STATS_MAP_MAC_RX_GOOD_BC_PKT = 4,
    DKIT_STATS_MAP_MAC_RX_GOOD_BC_BYTE = 5,
    DKIT_STATS_MAP_MAC_RX_GOOD_NORMAL_PAUSE_PKT = 8,
    DKIT_STATS_MAP_MAC_RX_GOOD_NORMAL_PAUSE_BYTE = 9,
    DKIT_STATS_MAP_MAC_RX_GOOD_PFC_PAUSE_PKT = 10,
    DKIT_STATS_MAP_MAC_RX_GOOD_PFC_PAUSE_BYTE = 11,
    DKIT_STATS_MAP_MAC_RX_GOOD_CONTROL_PKT = 12,
    DKIT_STATS_MAP_MAC_RX_GOOD_CONTROL_BYTE = 13,
    DKIT_STATS_MAP_MAC_RX_FCS_ERROR_PKT = 18,
    DKIT_STATS_MAP_MAC_RX_FCS_ERROR_BYTE = 19,
    DKIT_STATS_MAP_MAC_RX_MAC_OVERRUN_PKT = 22,
    DKIT_STATS_MAP_MAC_RX_MAC_OVERRUN_BYTE = 23,
    DKIT_STATS_MAP_MAC_RX_GOOD_63B_PKT = 30,
    DKIT_STATS_MAP_MAC_RX_GOOD_63B_BYTE = 31,
    DKIT_STATS_MAP_MAC_RX_BAD_63B_PKT = 32,
    DKIT_STATS_MAP_MAC_RX_BAD_63B_BYTE = 33,
    DKIT_STATS_MAP_MAC_RX_GOOD_1519B_PKT = 34,
    DKIT_STATS_MAP_MAC_RX_GOOD_1519B_BYTE = 35,
    DKIT_STATS_MAP_MAC_RX_BAD_1519B_PKT = 36,
    DKIT_STATS_MAP_MAC_RX_BAD_1519B_BYTE = 37,
    DKIT_STATS_MAP_MAC_RX_GOOD_JUMBO_PKT = 38,
    DKIT_STATS_MAP_MAC_RX_GOOD_JUMBO_BYTE = 39,
    DKIT_STATS_MAP_MAC_RX_BAD_JUMBO_PKT = 40,
    DKIT_STATS_MAP_MAC_RX_BAD_JUMBO_BYTE = 41,
    DKIT_STATS_MAP_MAC_RX_64B_PKT = 42,
    DKIT_STATS_MAP_MAC_RX_64B_BYTE = 43,
    DKIT_STATS_MAP_MAC_RX_127B_PKT = 44,
    DKIT_STATS_MAP_MAC_RX_127B_BYTE = 45,
    DKIT_STATS_MAP_MAC_RX_255B_PKT = 46,
    DKIT_STATS_MAP_MAC_RX_255B_BYTE = 47,
    DKIT_STATS_MAP_MAC_RX_511B_PKT = 48,
    DKIT_STATS_MAP_MAC_RX_511B_BYTE = 49,
    DKIT_STATS_MAP_MAC_RX_1023B_PKT = 50,
    DKIT_STATS_MAP_MAC_RX_1023B_BYTE = 51,
    DKIT_STATS_MAP_MAC_RX_1518B_PKT = 52,
    DKIT_STATS_MAP_MAC_RX_1518B_BYTE = 53,

    DKIT_STATS_MAP_MAC_RX_TYPE_NUM = 40
};
typedef enum dkit_stats_map_mac_rx_type_e dkit_stats_map_mac_rx_type_t;

/* the value defined should be consistent with ctc_stats.h */
enum dkit_stats_map_mac_tx_type_e
{
    DKIT_STATS_MAP_MAC_TX_GOOD_UC_PKT = 0,
    DKIT_STATS_MAP_MAC_TX_GOOD_UC_BYTE = 1,
    DKIT_STATS_MAP_MAC_TX_GOOD_MC_PKT = 2,
    DKIT_STATS_MAP_MAC_TX_GOOD_MC_BYTE = 3,
    DKIT_STATS_MAP_MAC_TX_GOOD_BC_PKT = 4,
    DKIT_STATS_MAP_MAC_TX_GOOD_BC_BYTE = 5,
    DKIT_STATS_MAP_MAC_TX_GOOD_PAUSE_PKT = 6,
    DKIT_STATS_MAP_MAC_TX_GOOD_PAUSE_BYTE = 7,
    DKIT_STATS_MAP_MAC_TX_GOOD_CONTROL_PKT = 8,
    DKIT_STATS_MAP_MAC_TX_GOOD_CONTROL_BYTE = 9,
    DKIT_STATS_MAP_MAC_TX_FCS_ERROR_PKT = 32,
    DKIT_STATS_MAP_MAC_TX_FCS_ERROR_BYTE = 33,
    DKIT_STATS_MAP_MAC_TX_MAC_UNDERRUN_PKT = 30,
    DKIT_STATS_MAP_MAC_TX_MAC_UNDERRUN_BYTE = 31,
    DKIT_STATS_MAP_MAC_TX_63B_PKT = 12,
    DKIT_STATS_MAP_MAC_TX_63B_BYTE = 13,
    DKIT_STATS_MAP_MAC_TX_64B_PKT = 14,
    DKIT_STATS_MAP_MAC_TX_64B_BYTE = 15,
    DKIT_STATS_MAP_MAC_TX_127B_PKT = 16,
    DKIT_STATS_MAP_MAC_TX_127B_BYTE = 17,
    DKIT_STATS_MAP_MAC_TX_225B_PKT = 18,
    DKIT_STATS_MAP_MAC_TX_225B_BYTE = 19,
    DKIT_STATS_MAP_MAC_TX_511B_PKT = 20,
    DKIT_STATS_MAP_MAC_TX_511B_BYTE = 21,
    DKIT_STATS_MAP_MAC_TX_1023B_PKT = 22,
    DKIT_STATS_MAP_MAC_TX_1023B_BYTE = 23,
    DKIT_STATS_MAP_MAC_TX_1518B_PKT = 24,
    DKIT_STATS_MAP_MAC_TX_1518B_BYTE = 25,
    DKIT_STATS_MAP_MAC_TX_1519B_PKT = 26,
    DKIT_STATS_MAP_MAC_TX_1519B_BYTE = 27,
    DKIT_STATS_MAP_MAC_TX_JUMBO_PKT = 28,
    DKIT_STATS_MAP_MAC_TX_JUMBO_BYTE = 29,

    DKIT_STATS_MAP_MAC_TX_TYPE_NUM
};
typedef enum dkit_stats_map_mac_tx_type_e dkit_stats_map_mac_tx_type_t;

enum dkit_stats_mac_cnt_type_e
{
    DKIT_STATS_MAC_CNT_TYPE_PKT,
    DKIT_STATS_MAC_CNT_TYPE_BYTE,
    DKIT_STATS_MAC_CNT_TYPE_NUM
};
typedef enum dkit_stats_mac_cnt_type_e dkit_stats_mac_cnt_type_t;

union dkit_macstats_u
{
    uint8 xgmac_stats;
    uint8 cgmac_stats;
};
typedef union dkit_macstats_u dkit_macstats_t;

struct dkit_mac_stats_rx_s
{
    uint64 mac_stats_rx_bytes[DKIT_STATS_MAC_RCV_MAX];
    uint64 mac_stats_rx_pkts[DKIT_STATS_MAC_RCV_MAX];
};
typedef struct dkit_mac_stats_rx_s dkit_mac_stats_rx_t;

struct dkit_mac_stats_tx_s
{
    uint64 mac_stats_tx_bytes[DKIT_STATS_MAC_SEND_MAX];
    uint64 mac_stats_tx_pkts[DKIT_STATS_MAC_SEND_MAX];
};
typedef struct dkit_mac_stats_tx_s dkit_mac_stats_tx_t;

struct dkit_xqmac_stats_s
{
    dkit_mac_stats_rx_t mac_stats_rx[DKIT_STATS_XQMAC_SUBMAC_MAX];
    dkit_mac_stats_tx_t mac_stats_tx[DKIT_STATS_XQMAC_SUBMAC_MAX];
};
typedef struct dkit_xqmac_stats_s dkit_xqmac_stats_t;

struct dkit_cgmac_stats_s
{
    dkit_mac_stats_rx_t mac_stats_rx;
    dkit_mac_stats_tx_t mac_stats_tx;
};
typedef struct dkit_cgmac_stats_s dkit_cgmac_stats_t;

struct dkit_stats_mac_rec_s
{
    uint64 good_ucast_pkts;              /**< [HB.GB.GG] length equal to 64B to 1518B(no vlan) or 1522B(vlan),total number of unicast packets received without error */
    uint64 good_ucast_bytes;             /**< [HB.GB.GG] length equal to 64B to 1518B(no vlan) or 1522B(vlan),total bytes of unicast packets received without error */
    uint64 good_mcast_pkts;              /**< [HB.GB.GG] length equal to 64B to 1518B(no vlan) or 1522B(vlan),total number of multicast packets received without error */
    uint64 good_mcast_bytes;             /**< [HB.GB.GG] length equal to 64B to 1518B(no vlan) or 1522B(vlan),total bytes of multicast packets received without error */
    uint64 good_bcast_pkts;              /**< [HB.GB.GG] length equal to 64B to 1518B(no vlan) or 1522B(vlan),total number of broadcast packets received without error */
    uint64 good_bcast_bytes;             /**< [HB.GB.GG] length equal to 64B to 1518B(no vlan) or 1522B(vlan),total bytes of broadcast packets received without error */
    uint64 good_pause_pkts;              /**< [HB] total number of pause packets without error */
    uint64 good_pause_bytes;             /**< [HB] total bytes of pause packets without error */
    uint64 good_normal_pause_pkts;       /**< [GB.GG] total number of nomal pause packets without error */
    uint64 good_normal_pause_bytes;      /**< [GB.GG] total bytes of nomal pause packets without error */
    uint64 good_pfc_pause_pkts;          /**< [GB.GG] total number of pfc pause packets without error */
    uint64 good_pfc_pause_bytes;         /**< [GB.GG] total bytes of pfc pause packets without error */
    uint64 good_control_pkts;            /**< [HB.GB.GG] total number of control packets without error excluding the pause packet */
    uint64 good_control_bytes;           /**< [HB.GB.GG] total bytes of control packets without error excluding the pause packet */
    uint64 jabber_pkts;                  /**< [HB] no mac overrun,length greater than MTU,total number of packets with fcs error or alignment error */
    uint64 jabber_bytes;                 /**< [HB] no mac overrun,length greater than MTU,total bytes of packets with fcs error or alignment error */
    uint64 collision_pkts;               /**< [HB] no mac overrun,length less than 64B,total number of packets with fcs error or alignment error */
    uint64 collision_bytes;              /**< [HB] no mac overrun,length less than 64B,total bytes of packets with fcs error or alignment error */
    uint64 fcs_error_pkts;               /**< [HB.GB.GG] no mac overrun,length equal to 64B to MTU,total number of packets with fcs error */
    uint64 fcs_error_bytes;              /**< [HB.GB.GG] no mac overrun,length equal to 64B to MTU,total bytes of packets with fcs error */
    uint64 alignment_error_pkts;         /**< [HB] no mac overrun,length equal to 64B to MTU,total number of packets with alignment error */
    uint64 alignment_error_bytes;        /**< [HB] no mac overrun,length equal to 64B to MTU,total bytes of packets with alignment error */
    uint64 mac_overrun_pkts;             /**< [HB.GB.GG] total number of packets received with mac overrun condition */
    uint64 mac_overrun_bytes;            /**< [HB.GB.GG] total bytes of packets received with mac overrun condition */
    uint64 good_oversize_pkts;           /**< [HB] no mac overrun,length greater than 1518B(no vlan) or 1522B(vlan),total number of packets received without fcs error and alignment error */
    uint64 good_oversize_bytes;          /**< [HB] no mac overrun,length greater than 1518B(no vlan) or 1522B(vlan),total bytes of packets received without fcs error and alignment error */
    uint64 good_undersize_pkts;          /**< [HB] no mac overrun,length less than 64B,total number of packets received without fcs error and alignment error */
    uint64 good_undersize_bytes;         /**< [HB] no mac overrun,length less than 64B,total bytes of packets received without fcs error and alignment error */
    uint64 gmac_good_oam_pkts;           /**< [HB] total number of packets without error */
    uint64 gmac_good_oam_bytes;          /**< [HB] total bytes of packets without error */
    uint64 good_63_pkts;                 /**< [HB.GB.GG] total number of good packets received with length less than 64B without fcs error and late collision */
    uint64 good_63_bytes;                /**< [HB.GB.GG] total bytes of good packets received with length less than 64B without fcs error and late collision */
    uint64 bad_63_pkts;                  /**< [HB.GB.GG] total number of errored packets received with length less than 64B including fcs error and late collision */
    uint64 bad_63_bytes;                 /**< [HB.GB.GG] total bytes of errored packets received with length less than 64B including fcs error and late collision */
    uint64 good_1519_pkts;               /**< [HB.GB.GG] total number of good packets received with length equal to 1519B to MTU including fcs error and late collision */
    uint64 good_1519_bytes;              /**< [HB.GB.GG] total bytes of good packets received with length equal to 1519B to MTU including fcs error and late collision */
    uint64 bad_1519_pkts;                /**< [HB.GB.GG] total number of errored packets received with length equal to 1519B to MTU including fcs error */
    uint64 bad_1519_bytes;               /**< [HB.GB.GG] total bytes of errored packets received with length equal to 1519B to MTU including fcs error */
    uint64 good_jumbo_pkts;              /**< [HB.GB.GG] total number of good packets received with length greater than MTU including fcs error */
    uint64 good_jumbo_bytes;             /**< [HB.GB.GG] total bytes of good packets received with length greater than MTU including fcs error */
    uint64 bad_jumbo_pkts;               /**< [HB.GB.GG] total number of errored packets received with length greater than MTU including fcs error */
    uint64 bad_jumbo_bytes;              /**< [HB.GB.GG] total bytes of errored packets received with length greater than MTU including fcs error */
    uint64 pkts_64;                      /**< [HB.GB.GG] total number of packets received with length equal to 64B including fcs error and late collision */
    uint64 bytes_64;                     /**< [HB.GB.GG] total bytes of packets received with length equal to 64B including fcs error and late collision */
    uint64 pkts_65_to_127;               /**< [HB.GB.GG] total number of packets received with length equal to 65B to 127B including fcs error and late collision */
    uint64 bytes_65_to_127;              /**< [HB.GB.GG] total bytes of packets received with length equal to 65B to 127B including fcs error and late collision */
    uint64 pkts_128_to_255;              /**< [HB.GB.GG] total number of packets received with length equal to 128B to 255B including fcs error and late collision */
    uint64 bytes_128_to_255;             /**< [HB.GB.GG] total bytes of packets received with length equal to 128B to 255B including fcs error and late collision */
    uint64 pkts_256_to_511;              /**< [HB.GB.GG] total number of packets received with length equal to 256B to 511B including fcs error and late collision */
    uint64 bytes_256_to_511;             /**< [HB.GB.GG] total bytes of packets received with length equal to 256B to 511B including fcs error and late collision */
    uint64 pkts_512_to_1023;             /**< [HB.GB.GG] total number of packets received with length equal to 512B to 1023B including fcs error and late collision */
    uint64 bytes_512_to_1023;            /**< [HB.GB.GG] total bytes of packets received with length equal to 512B to 1023B including fcs error and late collision */
    uint64 pkts_1024_to_1518;            /**< [HB.GB.GG] total number of packets received with length equal to 1024B to 1518B including fcs error and late collision */
    uint64 bytes_1024_to_1518;           /**< [HB.GB.GG] total bytes of packets received with length equal to 1024B to 1518B including fcs error and late collision */
};
typedef struct dkit_stats_mac_rec_s dkit_stats_mac_rec_t;

struct dkit_stats_mac_snd_s
{
    uint64 good_ucast_pkts;              /**< [HB.GB.GG] total number of unicast packets transmitted without error */
    uint64 good_ucast_bytes;             /**< [HB.GB.GG] total bytes of unicast packets transmitted without error */
    uint64 good_mcast_pkts;              /**< [HB.GB.GG] total number of multicast packets transmitted without error */
    uint64 good_mcast_bytes;             /**< [HB.GB.GG] total bytes of multicast packets transmitted without error */
    uint64 good_bcast_pkts;              /**< [HB.GB.GG] total number of broadcast packets transmitted without error */
    uint64 good_bcast_bytes;             /**< [HB.GB.GG] total bytes of broadcast packets transmitted without error */
    uint64 good_pause_pkts;              /**< [HB.GB.GG] total number of pause packets transmitted without error */
    uint64 good_pause_bytes;             /**< [HB.GB.GG] total bytes of pause packets transmitted without error */
    uint64 good_control_pkts;            /**< [HB.GB.GG] total number of pause packets transmitted without error excluding the pause packet */
    uint64 good_control_bytes;           /**< [HB.GB.GG] total bytes of pause packets transmitted without error excluding the pause packet */
    uint64 good_oam_pkts;                /**< [HB] total number of oam packets transmitted without error */
    uint64 good_oam_bytes;               /**< [HB] total bytes of oam packets transmitted without error */
    uint64 pkts_63;                      /**< [HB.GB.GG] total number of packets transmitted with length less than 64B including fcs error and late collision */
    uint64 bytes_63;                     /**< [HB.GB.GG] total bytes of packets transmitted with length less than 64B including fcs error and late collision */
    uint64 pkts_64;                      /**< [HB.GB.GG] total number of packets transmitted with length equal to 64B including fcs error and late collision */
    uint64 bytes_64;                     /**< [HB.GB.GG] total bytes of packets transmitted with length equal to 64B including fcs error and late collision */
    uint64 pkts_65_to_127;               /**< [HB.GB.GG] total number of packets transmitted with length equal to 65B to 127B including fcs error and late collision */
    uint64 bytes_65_to_127;              /**< [HB.GB.GG] total bytes of packets transmitted with length equal to 65B to 127B including fcs error and late collision */
    uint64 pkts_128_to_255;              /**< [HB.GB.GG] total number of packets transmitted with length equal to 128B to 255B including fcs error and late collision */
    uint64 bytes_128_to_255;             /**< [HB.GB.GG] total bytes of packets transmitted with length equal to 128B to 255B including fcs error and late collision */
    uint64 pkts_256_to_511;              /**< [HB.GB.GG] total number of packets transmitted with length equal to 256B to 511B including fcs error and late collision */
    uint64 bytes_256_to_511;             /**< [HB.GB.GG] total bytes of packets transmitted with length equal to 256B to 511B including fcs error and late collision */
    uint64 pkts_512_to_1023;             /**< [HB.GB.GG] total number of packets transmitted with length equal to 512B to 1023B including fcs error and late collision */
    uint64 bytes_512_to_1023;            /**< [HB.GB.GG] total bytes of packets transmitted with length equal to 512B to 1023B including fcs error and late collision */
    uint64 pkts_1024_to_1518;            /**< [HB.GB.GG] total number of packets transmitted with length equal to 1024B to 1518B including fcs error and late collision */
    uint64 bytes_1024_to_1518;           /**< [HB.GB.GG] total bytes of packets transmitted with length equal to 1024B to 1518B including fcs error and late collision */
    uint64 pkts_1519;                    /**< [HB.GB.GG] total number of packets transmitted with length equal to 1519B to MTU including fcs error and late collision */
    uint64 bytes_1519;                   /**< [HB.GB.GG] total bytes of packets transmitted with length equal to 1519B to MTU including fcs error and late collision */
    uint64 jumbo_pkts;                   /**< [HB.GB.GG] total number of packets transmitted with length greater than MTU including fcs error and late collision */
    uint64 jumbo_bytes;                  /**< [HB.GB.GG] total bytes of packets transmitted with length greater than MTU including fcs error and late collision */
    uint64 mac_underrun_pkts;            /**< [HB.GB.GG] total number of packets transmitted with mac underrun condition */
    uint64 mac_underrun_bytes;           /**< [HB.GB.GG] total bytes of packets transmitted with mac underrun condition */
    uint64 fcs_error_pkts;               /**< [HB.GB.GG] total number of packets transmitted with fcs error */
    uint64 fcs_error_bytes;              /**< [HB.GB.GG] total bytes of packets transmitted with fcs error */
};
typedef struct dkit_stats_mac_snd_s dkit_stats_mac_snd_t;

struct dkit_mac_stats_detail_s
{
    union
    {
        dkit_stats_mac_rec_t rx_stats;
        dkit_stats_mac_snd_t tx_stats;
    }stats;
};
typedef struct dkit_mac_stats_detail_s dkit_mac_stats_detail_t;

struct dkit_mac_stats_s
{
    uint8 stats_mode;  /**< [GB.GG] get stats mode, 0: plus stats, 1: detail stats */
    uint8 rsv[3];
    dkit_mac_stats_detail_t stats_detail; /**< [HB.GB.GG] centec asic detail stats */
};
typedef struct dkit_mac_stats_s dkit_mac_stats_t;

struct dkit_stats_basic_s
{
    uint64 packet_count;                  /**< [HB.GB.GG] total number of packets */
    uint64 byte_count;                    /**< [HB.GB.GG] total bytes of paclets */
};
typedef struct dkit_stats_basic_s dkit_stats_basic_t;

struct dkit_stats_master_s
{
    dkit_xqmac_stats_t xqmac_stats_table[DKIT_STATS_XQMAC_RAM_MAX];
    dkit_cgmac_stats_t cgmac_stats_table[DKIT_STATS_CGMAC_RAM_MAX];
};
typedef struct dkit_stats_master_s dkit_stats_master_t;

enum dkit_lkup_request_stats_e
{
    DKIT_LKUP_STATS_IPEHDRADJ = 0,
    DKIT_LKUP_STATS_IPEINTFMAP,
    DKIT_LKUP_STATS_IPELKUPMGR,
    DKIT_LKUP_STATS_IPEPKTPROC,
    DKIT_LKUP_STATS_IPEFORWARD,
    DKIT_LKUP_STATS_EPEHDRADJ,
    DKIT_LKUP_STATS_EPENEXTHOP,
    DKIT_LKUP_STATS_EPEHDRPROC,
    DKIT_LKUP_STATS_EPEHDREDIT,
    DKIT_LKUP_STATS_EPEACLOAM0,
    DKIT_LKUP_STATS_EPEACLOAM1,
    DKIT_LKUP_STATS_EPEACLOAM2,
    DKIT_LKUP_STATS_MAX
};
 typedef enum dkit_lkup_request_stats_e dkit_lkup_request_stats_t;

#define ____1_COMMON____

#if 0
STATIC int32
_ctc_usw_dkit_stats_get_mac_en(uint8 lchip, uint32 mac_id, uint32* p_enable)
{
    ctc_dkit_interface_status_t status;

    sal_memset(&status, 0, sizeof(ctc_dkit_interface_status_t));

    ctc_usw_dkit_interface_get_mac_status(lchip, &status);

    if (mac_id >= CTC_DKIT_INTERFACE_MAC_NUM)
    {
        *p_enable = 0;
    }
    else
    {
        *p_enable = status.mac_en[mac_id];
    }

    return CLI_SUCCESS;
}
#endif

#define ____2_MAC_STATS____
dkit_stats_master_t* p_dkit_stats_master = NULL;


STATIC int32
_ctc_usw_dkit_internal_mac_stats_reset(uint8 lchip)
{
#ifdef NEVER
    uint8 i = 0;
    int32  ret = 0;
    uint32 cmd = 0;
    XqmacStatsCfg0_m xqmac_stats_cfg;
    CgmacStatsCfg0_m cgmac_stats_cfg;

    CTC_DKIT_LCHIP_CHECK(lchip);

    for (i = 0; i < 24; i++)
    {
        sal_memset(&xqmac_stats_cfg, 0, sizeof(XqmacStatsCfg0_m));

        cmd = DRV_IOR(XqmacStatsCfg0_t + i, DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &xqmac_stats_cfg);
        if (ret < 0)
        {
            CTC_DKIT_PRINT("%s %d, %s[%u].\n", __FUNCTION__, __LINE__, TABLE_NAME(lchip, XqmacStatsCfg0_t + i), 0);
            return CLI_ERROR;
        }

        SetXqmacStatsCfg0(V, clearOnRead_f, &xqmac_stats_cfg, 1);
        SetXqmacStatsCfg0(V, incrHold_f, &xqmac_stats_cfg, 0);
        SetXqmacStatsCfg0(V, incrSaturate_f, &xqmac_stats_cfg, 0);
        SetXqmacStatsCfg0(V, packetLenMtu1_f, &xqmac_stats_cfg, 0x5EE);
        SetXqmacStatsCfg0(V, packetLenMtu2_f, &xqmac_stats_cfg, 0x600);

        cmd = DRV_IOW(XqmacStatsCfg0_t + i, DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &xqmac_stats_cfg);
        if (ret < 0)
        {
            CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
            return CLI_ERROR;
        }
    }

    for (i = 0; i < 4; i++)
    {
        sal_memset(&cgmac_stats_cfg, 0, sizeof(CgmacStatsCfg0_m));

        cmd = DRV_IOR(CgmacStatsCfg0_t + i, DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &cgmac_stats_cfg);
        if (ret < 0)
        {
            CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
            return CLI_ERROR;
        }

        SetCgmacStatsCfg0(V, clearOnRead_f, &cgmac_stats_cfg, 1);
        SetCgmacStatsCfg0(V, incrHold_f, &cgmac_stats_cfg, 0);
        SetCgmacStatsCfg0(V, incrSaturate_f, &cgmac_stats_cfg, 0);
        SetCgmacStatsCfg0(V, packetLenMtu1_f, &cgmac_stats_cfg, 0x5EE);
        SetCgmacStatsCfg0(V, packetLenMtu2_f, &cgmac_stats_cfg, 0x600);

        cmd = DRV_IOW(CgmacStatsCfg0_t + i, DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &cgmac_stats_cfg);
        if (ret < 0)
        {
            CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
            return CLI_ERROR;
        }
    }

    return CLI_SUCCESS;
#endif /* never */
    return CLI_SUCCESS;
}
#if 0
STATIC void
_ctc_usw_dkit_internal_mac_stats_get_ram(uint8 lchip, uint32 mac_id, uint8* p_mac_ram_type, ctc_dkit_interface_mac_mode_t* p_mac_type)
{
    ctc_dkit_interface_status_t status;
    char* mac_type[] = {"sgmii", "xfi", "xaui", "xlg", "cg"};

    sal_memset(&status, 0, sizeof(ctc_dkit_interface_status_t));
    ctc_usw_dkit_interface_get_mac_status(lchip, &status);
    *p_mac_type = status.mac_mode[mac_id];

    CTC_DKIT_PRINT_DEBUG("%s %d, MacId:%u, MacType:%s\n", __FUNCTION__, __LINE__, mac_id, mac_type[*p_mac_type]);
    if ((CTC_DKIT_INTERFACE_SGMII == *p_mac_type) || (CTC_DKIT_INTERFACE_XFI == *p_mac_type))
    {
        *p_mac_ram_type = DKIT_STATS_XQMAC_STATS_RAM0 + (SLICE_ID(mac_id) * DKIT_XQMAC_PER_SLICE_NUM) \
                          + ((SLICE_ID(mac_id) ? (PCS_ID(mac_id) - 48) : PCS_ID(mac_id)) / DKIT_SUBMAC_PER_XQMAC_NUM);
    }
    else if ((CTC_DKIT_INTERFACE_XAUI == *p_mac_type) || (CTC_DKIT_INTERFACE_XLG == *p_mac_type))
    {
        *p_mac_ram_type = DKIT_STATS_XQMAC_STATS_RAM0 + (SLICE_ID(mac_id) * DKIT_XQMAC_PER_SLICE_NUM) \
                          + (SLICE_ID(mac_id) ? (PCS_ID(mac_id) - 48) : PCS_ID(mac_id));
    }

    else if (CTC_DKIT_INTERFACE_CG == *p_mac_type)
    {
        *p_mac_ram_type = DKIT_STATS_CGMAC_STATS_RAM0 + (SLICE_ID(mac_id) * DKIT_CGMAC_PER_SLICE_NUM) \
                          + (SLICE_ID(mac_id));
    }

    return;
}

STATIC void
_ctc_usw_dkit_internal_mac_stats_get_data(bool is_xqmac, void* p_stats_ram, dkit_stats_basic_t* p_basic_stats)
{
    #if 0
    uint64 tmp = 0;
    XqmacStatsRam0_m* p_xqmac_stats_ram = NULL;
    CgmacStatsRam0_m* p_cgmac_stats_ram = NULL;

    /*judge xgmac or cgmac through mac ram type*/
    if (TRUE == is_xqmac)
    {
        p_xqmac_stats_ram = (XqmacStatsRam0_m*)p_stats_ram;

        tmp = GetXqmacStatsRam0(V, frameCntDataHi_f, p_xqmac_stats_ram);
        tmp <<= 32;
        tmp |= GetXqmacStatsRam0(V, frameCntDataLo_f, p_xqmac_stats_ram);

        p_basic_stats->packet_count = tmp;

        tmp = GetXqmacStatsRam0(V, byteCntDataHi_f, p_xqmac_stats_ram);
        tmp <<= 32;
        tmp |= GetXqmacStatsRam0(V, byteCntDataLo_f, p_xqmac_stats_ram);

        p_basic_stats->byte_count = tmp;
    }
    else
    {
        p_cgmac_stats_ram = (CgmacStatsRam0_m*)p_stats_ram;

        tmp = GetCgmacStatsRam0(V, frameCntDataHi_f, p_cgmac_stats_ram);
        tmp <<= 32;
        tmp |= GetCgmacStatsRam0(V, frameCntDataLo_f, p_cgmac_stats_ram);

        p_basic_stats->packet_count = tmp;

        tmp = GetCgmacStatsRam0(V, byteCntDataHi_f, p_cgmac_stats_ram);
        tmp <<= 32;
        tmp |= GetCgmacStatsRam0(V, byteCntDataLo_f, p_cgmac_stats_ram);

        p_basic_stats->byte_count = tmp;
    }
    #endif
    return;
}

STATIC void
_ctc_usw_dkit_internal_mac_stats_mem_mapping(uint8 sys_type, dkit_stats_mac_cnt_type_t cnt_type, uint8* p_offset)
{
    /* The table mapper offset value against ctc_stats_mac_rec_t variable address based on stats_type and direction. */
    uint8 rx_mac_stats[DKIT_STATS_MAC_RCV_NUM][DKIT_STATS_MAC_CNT_TYPE_NUM] =
    {
        {DKIT_STATS_MAP_MAC_RX_GOOD_UC_PKT,           DKIT_STATS_MAP_MAC_RX_GOOD_UC_BYTE},
        {DKIT_STATS_MAP_MAC_RX_GOOD_MC_PKT,           DKIT_STATS_MAP_MAC_RX_GOOD_MC_BYTE},
        {DKIT_STATS_MAP_MAC_RX_GOOD_BC_PKT,           DKIT_STATS_MAP_MAC_RX_GOOD_BC_BYTE},
        {DKIT_STATS_MAP_MAC_RX_GOOD_NORMAL_PAUSE_PKT, DKIT_STATS_MAP_MAC_RX_GOOD_NORMAL_PAUSE_BYTE},
        {DKIT_STATS_MAP_MAC_RX_GOOD_PFC_PAUSE_PKT,    DKIT_STATS_MAP_MAC_RX_GOOD_PFC_PAUSE_BYTE},
        {DKIT_STATS_MAP_MAC_RX_GOOD_CONTROL_PKT,      DKIT_STATS_MAP_MAC_RX_GOOD_CONTROL_BYTE},
        {DKIT_STATS_MAP_MAC_RX_FCS_ERROR_PKT,         DKIT_STATS_MAP_MAC_RX_FCS_ERROR_BYTE},
        {DKIT_STATS_MAP_MAC_RX_MAC_OVERRUN_PKT,       DKIT_STATS_MAP_MAC_RX_MAC_OVERRUN_BYTE},
        {DKIT_STATS_MAP_MAC_RX_GOOD_63B_PKT,          DKIT_STATS_MAP_MAC_RX_GOOD_63B_BYTE},
        {DKIT_STATS_MAP_MAC_RX_BAD_63B_PKT,           DKIT_STATS_MAP_MAC_RX_BAD_63B_BYTE},
        {DKIT_STATS_MAP_MAC_RX_GOOD_1519B_PKT,        DKIT_STATS_MAP_MAC_RX_GOOD_1519B_BYTE},
        {DKIT_STATS_MAP_MAC_RX_BAD_1519B_PKT,         DKIT_STATS_MAP_MAC_RX_BAD_1519B_BYTE},
        {DKIT_STATS_MAP_MAC_RX_GOOD_JUMBO_PKT,        DKIT_STATS_MAP_MAC_RX_GOOD_JUMBO_BYTE},
        {DKIT_STATS_MAP_MAC_RX_BAD_JUMBO_PKT,         DKIT_STATS_MAP_MAC_RX_BAD_JUMBO_BYTE},
        {DKIT_STATS_MAP_MAC_RX_64B_PKT,               DKIT_STATS_MAP_MAC_RX_64B_BYTE},
        {DKIT_STATS_MAP_MAC_RX_127B_PKT,              DKIT_STATS_MAP_MAC_RX_127B_BYTE},
        {DKIT_STATS_MAP_MAC_RX_255B_PKT,              DKIT_STATS_MAP_MAC_RX_255B_BYTE},
        {DKIT_STATS_MAP_MAC_RX_511B_PKT,              DKIT_STATS_MAP_MAC_RX_511B_BYTE},
        {DKIT_STATS_MAP_MAC_RX_1023B_PKT,             DKIT_STATS_MAP_MAC_RX_1023B_BYTE},
        {DKIT_STATS_MAP_MAC_RX_1518B_PKT,             DKIT_STATS_MAP_MAC_RX_1518B_BYTE}
    };

    /* The table mapper offset value against ctc_stats_mac_snd_t variable address based on stats_type and direction. */
    uint8 tx_mac_stats[DKIT_STATS_MAC_SEND_NUM][DKIT_STATS_MAC_CNT_TYPE_NUM] =
    {
        {DKIT_STATS_MAP_MAC_TX_GOOD_UC_PKT,      DKIT_STATS_MAP_MAC_TX_GOOD_UC_BYTE},
        {DKIT_STATS_MAP_MAC_TX_GOOD_MC_PKT,      DKIT_STATS_MAP_MAC_TX_GOOD_MC_BYTE},
        {DKIT_STATS_MAP_MAC_TX_GOOD_BC_PKT,      DKIT_STATS_MAP_MAC_TX_GOOD_BC_BYTE},
        {DKIT_STATS_MAP_MAC_TX_GOOD_PAUSE_PKT,   DKIT_STATS_MAP_MAC_TX_GOOD_PAUSE_BYTE},
        {DKIT_STATS_MAP_MAC_TX_GOOD_CONTROL_PKT, DKIT_STATS_MAP_MAC_TX_GOOD_CONTROL_BYTE},
        {DKIT_STATS_MAP_MAC_TX_FCS_ERROR_PKT,    DKIT_STATS_MAP_MAC_TX_FCS_ERROR_BYTE},
        {DKIT_STATS_MAP_MAC_TX_MAC_UNDERRUN_PKT, DKIT_STATS_MAP_MAC_TX_MAC_UNDERRUN_BYTE},
        {DKIT_STATS_MAP_MAC_TX_63B_PKT,          DKIT_STATS_MAP_MAC_TX_63B_BYTE},
        {DKIT_STATS_MAP_MAC_TX_64B_PKT,          DKIT_STATS_MAP_MAC_TX_64B_BYTE},
        {DKIT_STATS_MAP_MAC_TX_127B_PKT,         DKIT_STATS_MAP_MAC_TX_127B_BYTE},
        {DKIT_STATS_MAP_MAC_TX_225B_PKT,         DKIT_STATS_MAP_MAC_TX_225B_BYTE},
        {DKIT_STATS_MAP_MAC_TX_511B_PKT,         DKIT_STATS_MAP_MAC_TX_511B_BYTE},
        {DKIT_STATS_MAP_MAC_TX_1023B_PKT,        DKIT_STATS_MAP_MAC_TX_1023B_BYTE},
        {DKIT_STATS_MAP_MAC_TX_1518B_PKT,        DKIT_STATS_MAP_MAC_TX_1518B_BYTE},
        {DKIT_STATS_MAP_MAC_TX_1519B_PKT,        DKIT_STATS_MAP_MAC_TX_1519B_BYTE},
        {DKIT_STATS_MAP_MAC_TX_JUMBO_PKT,        DKIT_STATS_MAP_MAC_TX_JUMBO_BYTE}
    };

    if (sys_type < DKIT_STATS_MAC_RCV_MAX)
    {
        *p_offset = rx_mac_stats[sys_type][cnt_type];
    }
    else
    {
        *p_offset = tx_mac_stats[sys_type - DKIT_STATS_MAC_SEND_UCAST][cnt_type];
    }

    return;
}

STATIC int32
_ctc_usw_dkit_internal_mac_stats_get_mem(uint8 mac_ram_type, uint8 stats_type, uint32 mac_id, void** pp_mac_stats)
{
    if (DKIT_STATS_IS_XQMAC_STATS(stats_type))
    {
        if (stats_type < DKIT_STATS_MAC_RCV_MAX)
        {
            *pp_mac_stats = &p_dkit_stats_master->xqmac_stats_table[mac_ram_type].mac_stats_rx[PCS_ID(mac_id) % 4];
        }
        else
        {
            *pp_mac_stats = &p_dkit_stats_master->xqmac_stats_table[mac_ram_type].mac_stats_tx[PCS_ID(mac_id) % 4];
        }
    }
    else if (DKIT_STATS_IS_CGMAC_STATS(stats_type))
    {
        if (stats_type < DKIT_STATS_MAC_RCV_MAX)
        {
            *pp_mac_stats = &p_dkit_stats_master->cgmac_stats_table[mac_ram_type - DKIT_STATS_XQMAC_RAM_MAX].mac_stats_rx;
        }
        else
        {
            *pp_mac_stats = &p_dkit_stats_master->cgmac_stats_table[mac_ram_type - DKIT_STATS_XQMAC_RAM_MAX].mac_stats_tx;
        }
    }
    else
    {
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

STATIC void
_ctc_usw_dkit_internal_mac_stats_write_mem(uint8 stats_type, void* p_mac_stats, dkit_stats_basic_t* p_stats_basic)
{
    dkit_mac_stats_rx_t* p_mac_stats_rx = NULL;
    dkit_mac_stats_tx_t* p_mac_stats_tx = NULL;

    if (stats_type < DKIT_STATS_MAC_RCV_MAX)
    {
        p_mac_stats_rx = (dkit_mac_stats_rx_t*)p_mac_stats;

        p_mac_stats_rx->mac_stats_rx_pkts[stats_type] = p_stats_basic->packet_count;
        p_mac_stats_rx->mac_stats_rx_bytes[stats_type] = p_stats_basic->byte_count;
    }
    else
    {
        p_mac_stats_tx = (dkit_mac_stats_tx_t*)p_mac_stats;
        p_mac_stats_tx->mac_stats_tx_pkts[stats_type - DKIT_STATS_MAC_SEND_UCAST] = p_stats_basic->packet_count;
        p_mac_stats_tx->mac_stats_tx_bytes[stats_type - DKIT_STATS_MAC_SEND_UCAST] = p_stats_basic->byte_count;
    }

    return;
}

STATIC void
_ctc_usw_dkit_internal_mac_stats_read_item(uint8 sys_type, void* p_ctc_stats, dkit_stats_basic_t* p_stats_basic)
{
    uint8  offset = 0;
    uint8  cnt_type = 0;
    uint64* p_val[DKIT_STATS_MAC_CNT_TYPE_NUM] = {&p_stats_basic->packet_count, &p_stats_basic->byte_count};

    for (cnt_type = 0; cnt_type < DKIT_STATS_MAC_CNT_TYPE_NUM; cnt_type++)
    {
        _ctc_usw_dkit_internal_mac_stats_mem_mapping(sys_type, cnt_type, &offset);
        ((uint64*)p_ctc_stats)[offset] = *(p_val[cnt_type]);
    }

    return;
}

STATIC void
_ctc_usw_dkit_internal_mac_stats_read_mem(uint8 stats_type, void* p_ctc_stats, void* p_mac_stats)
{
    dkit_stats_basic_t stats_basic;

    sal_memset(&stats_basic, 0, sizeof(dkit_stats_basic_t));
    if (stats_type < DKIT_STATS_MAC_RCV_MAX)
    {
        stats_basic.packet_count = ((dkit_mac_stats_rx_t*)p_mac_stats)->mac_stats_rx_pkts[stats_type];
        stats_basic.byte_count = ((dkit_mac_stats_rx_t*)p_mac_stats)->mac_stats_rx_bytes[stats_type];
    }
    else
    {
        stats_basic.packet_count = ((dkit_mac_stats_tx_t*)p_mac_stats)->mac_stats_tx_pkts[stats_type - DKIT_STATS_MAC_RCV_MAX];
        stats_basic.byte_count = ((dkit_mac_stats_tx_t*)p_mac_stats)->mac_stats_tx_bytes[stats_type - DKIT_STATS_MAC_RCV_MAX];
    }
    _ctc_usw_dkit_internal_mac_stats_read_item(stats_type, p_ctc_stats, &stats_basic);

    return;
}

STATIC int32
_ctc_usw_dkit_internal_mac_stats_get_tbl(uint8 lchip, uint32 mac_id, uint32* p_tbl_id, uint8* p_tbl_base, uint8* p_mac_ram_type)
{
    #if 0
    int32  tbl_step = 0;
    ctc_dkit_interface_mac_mode_t mac_type;

    /*get mac index, channel of lport from function to justify mac ram*/
    _ctc_usw_dkit_internal_mac_stats_get_ram(lchip, mac_id, p_mac_ram_type, &mac_type);

    if (DKIT_STATS_IS_XQMAC_STATS(*p_mac_ram_type))
    {
        tbl_step = XqmacStatsRam1_t - XqmacStatsRam0_t;
        *p_tbl_id = XqmacStatsRam0_t + (*p_mac_ram_type - DKIT_STATS_XQMAC_STATS_RAM0) * tbl_step;
    }
    else if (DKIT_STATS_IS_CGMAC_STATS(*p_mac_ram_type))
    {
        tbl_step = CgmacStatsRam1_t - CgmacStatsRam0_t;
        *p_tbl_id = CgmacStatsRam0_t + (*p_mac_ram_type - DKIT_STATS_CGMAC_STATS_RAM0) * tbl_step;
    }
    else
    {
        return CLI_ERROR;
    }

    if ((CTC_DKIT_INTERFACE_SGMII == mac_type) || (CTC_DKIT_INTERFACE_XFI == mac_type))
    {
        *p_tbl_base = (PCS_ID(mac_id) & 0x3) * (DKIT_STATS_MAC_BASED_STATS_XQMAC_RAM_DEPTH / 4);
    }
    else
    {
        *p_tbl_base = 0;
    }
    #endif
    return CLI_SUCCESS;
}
#endif
int32
ctc_usw_dkit_internal_mac_stats_get_rx(uint8 lchip, uint32 mac_id, dkit_mac_stats_t* p_stats)
{
#ifdef NEVER
    bool   is_xqmac = FALSE;
    void*  p_mac_stats = NULL;
    uint8  stats_type = 0;
    uint8  tbl_base = 0;
    int32  ret = CLI_SUCCESS;
    uint32 cmdr = 0;
    uint32 cmdw = 0;
    uint32 tbl_id = 0;
    uint32 is_enable = 0;
    uint8  mac_ram_type = 0;
    dkit_macstats_t mac_stats;
    dkit_stats_basic_t stats_basic;
    XqmacStatsRam0_m xqmac_stats_ram;
    drv_work_platform_type_t platform_type = MAX_WORK_PLATFORM;
    dkit_stats_mac_rec_t* stats_mac_rec = NULL;
    dkit_stats_mac_rec_t* p_stats_mac_rec = NULL;
    ctc_dkit_interface_status_t* status = NULL;

    DKITS_PTR_VALID_CHECK(p_stats);
    CTC_DKIT_LCHIP_CHECK(lchip);

    status = (ctc_dkit_interface_status_t*)sal_malloc(sizeof(ctc_dkit_interface_status_t));
    if(NULL == status)
    {
        return CLI_ERROR;
    }
    stats_mac_rec = (dkit_stats_mac_rec_t*)sal_malloc(sizeof(dkit_stats_mac_rec_t));
    if(NULL == stats_mac_rec)
    {
        return CLI_ERROR;
    }

    sal_memset(status, 0, sizeof(ctc_dkit_interface_status_t));
    ctc_usw_dkit_interface_get_mac_status(lchip, status);

    if (0 == status->valid[mac_id])
    {
        CTC_DKIT_PRINT("%s %d, invalid macid:%u\n", __FUNCTION__, __LINE__, mac_id);
        ret = CLI_ERROR;
        goto error;
    }

    sal_memset(&mac_stats, 0, sizeof(dkit_macstats_t));
    sal_memset(stats_mac_rec, 0, sizeof(dkit_stats_mac_rec_t));
    sal_memset(&xqmac_stats_ram, 0, sizeof(XqmacStatsRam0_m));

    sal_memset(&p_stats->stats_detail, 0, sizeof(dkit_mac_stats_detail_t));
    p_stats_mac_rec = &p_stats->stats_detail.stats.rx_stats;

    ret = _ctc_usw_dkit_internal_mac_stats_get_tbl(lchip, mac_id, &tbl_id, &tbl_base, &mac_ram_type);
    if (ret < 0)
    {
        CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
        ret = CLI_ERROR;
        goto error;
    }
    cmdr = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    cmdw = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);

    is_xqmac = ((tbl_id >= XqmacStatsRam0_t) && (tbl_id <= XqmacStatsRam23_t)) ? 1 : 0;

    ret = drv_get_platform_type(0, &platform_type);
    if (ret < 0)
    {
        CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
        ret = CLI_ERROR;
        goto error;
    }

    for (stats_type = DKIT_STATS_MAC_RCV_GOOD_UCAST; stats_type < DKIT_STATS_MAC_RCV_MAX; stats_type++)
    {
        sal_memset(&mac_stats, 0, sizeof(dkit_macstats_t));
        sal_memset(&stats_basic, 0, sizeof(dkit_stats_basic_t));
        /* check mac is disable */
        ret = _ctc_usw_dkit_stats_get_mac_en(lchip,mac_id, &is_enable);
        if (ret < 0)
        {
            CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
            ret = CLI_ERROR;
            goto error;
        }

        if (is_enable)
        {
             /*CTC_DKIT_PRINT("Read mac stats tbl %s[%d] \n", TABLE_NAME(lchip, tbl_id), stats_type + tbl_base);*/
            ret = DRV_IOCTL(lchip, stats_type + tbl_base, cmdr, &mac_stats);
            CTC_DKIT_PRINT_DEBUG("%s %d, %s[%u]\n", __FUNCTION__, __LINE__, TABLE_NAME(lchip, tbl_id), stats_type + tbl_base);

            if (ret < 0)
            {
                CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
                ret = CLI_ERROR;
                goto error;
            }
        }

        /* for uml, add this code to support clear after read */
        if (SW_SIM_PLATFORM == platform_type)
        {
             /*CTC_DKIT_PRINT("Clear %s[%d] by software in uml\n", TABLE_NAME(lchip, tbl_id), stats_type + tbl_base);*/
            ret = DRV_IOCTL(lchip, stats_type + tbl_base, cmdw, &xqmac_stats_ram);
            if (ret < 0)
            {
                CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
                ret = CLI_ERROR;
                goto error;
            }
        }

        _ctc_usw_dkit_internal_mac_stats_get_data(is_xqmac, &mac_stats, &stats_basic);
        ret = _ctc_usw_dkit_internal_mac_stats_get_mem(mac_ram_type, stats_type, mac_id, &p_mac_stats);
        if (ret < 0)
        {
            CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
            ret = CLI_ERROR;
            goto error;
        }
        _ctc_usw_dkit_internal_mac_stats_write_mem(stats_type, p_mac_stats, &stats_basic);
        _ctc_usw_dkit_internal_mac_stats_read_mem(stats_type, stats_mac_rec, p_mac_stats);
    }

    sal_memcpy(p_stats_mac_rec, stats_mac_rec, sizeof(dkit_stats_mac_rec_t));

error:
    if(NULL != status)
    {
        sal_free(status);
    }
    if(NULL != stats_mac_rec)
    {
        sal_free(stats_mac_rec);
    }

    return ret;
#endif /* never */
    return CLI_SUCCESS;
}

int32
ctc_usw_dkit_internal_mac_stats_get_tx(uint8 lchip, uint32 mac_id, dkit_mac_stats_t* p_stats)
{
#ifdef NEVER
    bool   is_xqmac = FALSE;
    void*  p_mac_stats = NULL;
    uint8  stats_type = 0;
    uint8  tbl_base = 0;
    int32  ret = CLI_SUCCESS;
    uint32 cmdr = 0;
    uint32 cmdw = 0;
    uint32 tbl_id = 0;
    uint32 is_enable = 0;
    uint8  mac_ram_type = 0;
    dkit_macstats_t mac_stats;
    dkit_stats_basic_t stats_basic;
    XqmacStatsRam0_m xqmac_stats_ram;
    drv_work_platform_type_t platform_type = MAX_WORK_PLATFORM;
    dkit_stats_mac_snd_t* stats_mac_snd = NULL;
    dkit_stats_mac_snd_t* p_stats_mac_snd = NULL;
    ctc_dkit_interface_status_t* status = NULL;

    DKITS_PTR_VALID_CHECK(p_stats);
    CTC_DKIT_LCHIP_CHECK(lchip);

    status = (ctc_dkit_interface_status_t*)sal_malloc(sizeof(ctc_dkit_interface_status_t));
    if(NULL == status)
    {
        return CLI_ERROR;
    }
    stats_mac_snd = (dkit_stats_mac_snd_t*)sal_malloc(sizeof(dkit_stats_mac_snd_t));
    if(NULL == stats_mac_snd)
    {
        return CLI_ERROR;
    }

    sal_memset(&status, 0, sizeof(ctc_dkit_interface_status_t));
    ctc_usw_dkit_interface_get_mac_status(lchip, status);

    if (0 == status->valid[mac_id])
    {
        CTC_DKIT_PRINT("%s %d, invalid macid:%u\n", __FUNCTION__, __LINE__, mac_id);
        ret = CLI_ERROR;
        goto error;
    }

    sal_memset(&mac_stats, 0, sizeof(dkit_macstats_t));
    sal_memset(stats_mac_snd, 0, sizeof(dkit_stats_mac_snd_t));
    sal_memset(&xqmac_stats_ram, 0, sizeof(XqmacStatsRam0_m));

    sal_memset(&p_stats->stats_detail, 0, sizeof(dkit_mac_stats_detail_t));
    p_stats_mac_snd = &p_stats->stats_detail.stats.tx_stats;

    ret = _ctc_usw_dkit_internal_mac_stats_get_tbl(lchip, mac_id, &tbl_id, &tbl_base, &mac_ram_type);
    if (ret < 0)
    {
        CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
        ret = CLI_ERROR;
        goto error;
    }

    cmdr = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    cmdw = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);

    is_xqmac = ((tbl_id >= XqmacStatsRam0_t) && (tbl_id <= XqmacStatsRam23_t)) ? 1 : 0;
    ret = drv_get_platform_type(&platform_type);
    if (ret < 0)
    {
        CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
        ret = CLI_ERROR;
        goto error;
    }

    for (stats_type = DKIT_STATS_MAC_SEND_UCAST; stats_type < DKIT_STATS_MAC_SEND_MAX; stats_type++)
    {
        sal_memset(&mac_stats, 0, sizeof(dkit_macstats_t));
        sal_memset(&stats_basic, 0, sizeof(dkit_stats_basic_t));

        /* check mac is disable */
        ret = _ctc_usw_dkit_stats_get_mac_en(lchip, mac_id, &is_enable);
        if (ret < 0)
        {
            CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
            ret = CLI_ERROR;
            goto error;
        }

        if (is_enable)
        {
             /*CTC_DKIT_PRINT("read mac stats tbl %s[%d] \n", TABLE_NAME(lchip, tbl_id), stats_type + tbl_base);*/
            ret = DRV_IOCTL(lchip, stats_type + tbl_base, cmdr, &mac_stats);
            CTC_DKIT_PRINT_DEBUG("%s %d, %s[%u]\n", __FUNCTION__, __LINE__, TABLE_NAME(lchip, tbl_id), stats_type + tbl_base);
            if (ret < 0)
            {
                CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
                ret = CLI_ERROR;
                goto error;
            }
        }

        /* for uml, add this code to support clear after read */
        if (SW_SIM_PLATFORM == platform_type)
        {
            ret = DRV_IOCTL(lchip, stats_type + tbl_base, cmdw, &xqmac_stats_ram);
             /*CTC_DKIT_PRINT("clear %s[%d] by software in uml\n", TABLE_NAME(lchip, tbl_id),  stats_type + tbl_base);*/
        }

        _ctc_usw_dkit_internal_mac_stats_get_data(is_xqmac, &mac_stats, &stats_basic);
        ret = _ctc_usw_dkit_internal_mac_stats_get_mem(mac_ram_type, stats_type, mac_id, &p_mac_stats);
        if (ret < 0)
        {
            CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
            ret = CLI_ERROR;
            goto error;
        }
        _ctc_usw_dkit_internal_mac_stats_write_mem(stats_type, p_mac_stats, &stats_basic);
        _ctc_usw_dkit_internal_mac_stats_read_mem(stats_type, stats_mac_snd, p_mac_stats);
    }

    sal_memcpy(p_stats_mac_snd, stats_mac_snd, sizeof(dkit_stats_mac_snd_t));

error:
    if(NULL != status)
    {
        sal_free(status);
    }
    if(NULL != stats_mac_snd)
    {
        sal_free(stats_mac_snd);
    }

    return ret;
#endif /* never */
    return CLI_SUCCESS;
}
#if 1
STATIC int32
ctc_usw_dkit_stats_init(void)
{
    p_dkit_stats_master = (dkit_stats_master_t*)sal_malloc(sizeof(dkit_stats_master_t));
    if (NULL == p_dkit_stats_master)
    {
        return CLI_ERROR;
    }
    sal_memset(p_dkit_stats_master, 0, sizeof(dkit_stats_master_t));

    return CLI_SUCCESS;
}
#endif
#define ____3_MISC____

extern int32
_ctc_usw_dkit_memory_show_ram_by_data_tbl_id(uint8 lchip,  uint32 tbl_id, uint32 index, uint32* data_entry, char* regex,uint8 acc_io);

STATIC int32
_ctc_usw_dkit_internal_bus_process(ctc_cmd_element_t* self, ctc_vti_t* vty, int argc, char** argv)
{

    uint32 lchip = 0;
    uint8 arg_id = 0;
    char *bus_name = NULL;
    tbls_id_t tmp_tableid = 0;

    if (argc >=1 )
    {
        bus_name = argv[0];
    }
    else
    {
        CTC_DKIT_PRINT("input bus name");
        return DRV_E_NONE;
    }

    arg_id = ctc_cli_get_prefix_item(&argv[0], argc, "lchip", sal_strlen("lchip"));
    if (0xFF != arg_id)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[arg_id + 1]);
    }
    DRV_INIT_CHECK(lchip);
    lchip = g_ctcs_api_en ? g_api_lchip : lchip;
    for (tmp_tableid = 0; tmp_tableid < MaxTblId_t; tmp_tableid++)
    {
        if (0 == TABLE_BUS(lchip, tmp_tableid))
        {
            continue;
        }
        if (0 == sal_strcasecmp(bus_name, TABLE_NAME(lchip, tmp_tableid)))
        {
            CTC_DKIT_PRINT("%s\n", TABLE_NAME(lchip, tmp_tableid));
            CTC_DKIT_PRINT("\n");
        }
    }

    return DRV_E_NONE;

}

STATIC int32
_ctc_usw_dkit_internal_bus_init(uint8 lchip, int (* func) (ctc_cmd_element_t*, ctc_vti_t*, int, char**), uint8 cli_tree_mode, uint8 install)
{

#define MAX_BUS_CLI_LEN         (8024*10)
#define MAX_BUS_CLI_TIPS_LEN    256
    ctc_cmd_element_t *cli_cmd = NULL;
    uint32 j = 0;
    char *cmd_str = NULL;
    char **cli_help = NULL;
    int32 cnt = 0, tip_node_num = 0;
    char bus_name[MAX_BUS_CLI_TIPS_LEN] = {0};
    char *name = NULL;
    uint32 bus_cnt = 0;
    uint32 tmp_cnt = 0;
    tbls_id_t tmp_tableid = 0;


    cmd_str = sal_malloc(MAX_BUS_CLI_LEN);
    if (NULL == cmd_str)
    {
        CTC_DKIT_PRINT("Dkits bus init no memory\n");
        return DRV_E_NO_MEMORY;
    }
    sal_memset(cmd_str, 0, MAX_BUS_CLI_LEN);

    sal_sprintf(cmd_str,"%s", "show bus (");

    for (tmp_tableid = 0; tmp_tableid < MaxTblId_t; tmp_tableid++)
    {
        if (0 == TABLE_BUS(lchip, tmp_tableid))
        {
            continue;
        }
        bus_cnt++;
    }
    tip_node_num = 5 + bus_cnt;
    cli_help = sal_malloc(sizeof(char*)*tip_node_num);
    for (cnt= 0; cnt < tip_node_num; cnt++)
    {
        cli_help[cnt] = sal_malloc(MAX_BUS_CLI_TIPS_LEN);
        sal_memset(cli_help[cnt], 0, MAX_BUS_CLI_TIPS_LEN);
    }
    cli_help[tip_node_num -1] = NULL;

    sal_sprintf(cli_help[0], CTC_CLI_SHOW_STR);
    sal_sprintf(cli_help[1], "show bus");
    sal_sprintf(cli_help[tip_node_num - 3], CTC_DKITS_CHIP_DESC);
    sal_sprintf(cli_help[tip_node_num - 2], CTC_DKITS_CHIP_ID_DESC);

    for (tmp_tableid = 0; tmp_tableid < MaxTblId_t; tmp_tableid++)
    {
        if (0 == TABLE_BUS(lchip, tmp_tableid))
        {
            continue;
        }
        tmp_cnt++;
        name = TABLE_NAME(lchip, tmp_tableid);

        sal_memset(bus_name, 0, MAX_BUS_CLI_TIPS_LEN);
        for (j = 0; j < sal_strlen(name); j++)
        {
            bus_name[j] = sal_tolower(name[j]);
        }
        if (tmp_cnt == bus_cnt)
        {
            sal_sprintf(cmd_str + sal_strlen(cmd_str), " %s ) (lchip CHIP_ID|) ", bus_name);
        }
        else
        {
            sal_sprintf(cmd_str + sal_strlen(cmd_str), " %s |", bus_name);
        }
        sal_sprintf(cli_help[(tmp_cnt - 1) + 2], "show %s bus info", bus_name);
    }

    cli_cmd = sal_malloc(sizeof(ctc_cmd_element_t));
    sal_memset(cli_cmd, 0 ,sizeof(ctc_cmd_element_t));

    cli_cmd->func   = func;
    cli_cmd->string = cmd_str;
    cli_cmd->doc    = cli_help;

    if (install)
    {
        install_element(cli_tree_mode, cli_cmd);
    }
    else
    {
        uninstall_element(cli_tree_mode, cli_cmd);
        sal_free(cli_cmd);
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_internal_pcie_read(uint8 lchip, dkits_pcie_ctl_t pcie_ctl, uint32 addr)
{
    uint32 value = 0;
    int32 ret = CLI_SUCCESS;
    CTC_DKIT_LCHIP_CHECK(lchip);
    switch (pcie_ctl)
    {
        case DKIT_PCIE_CFG_R:
            if (g_dal_op.pci_conf_read)
            {
                ret = g_dal_op.pci_conf_read(lchip, addr, &value);
                if (ret >= 0)
                {
                    CTC_DKIT_PRINT("Read pcie config address 0x%08x, value = 0x%08x\n", addr, value);
                }
            }
            else
            {
                CTC_DKIT_PRINT("g_dal_op.pci_conf_read is not registered!!!\n");
            }
            break;
        case DKIT_PCIE_MEM_R:
            if (g_dal_op.pci_read)
            {
                ret = g_dal_op.pci_read(lchip, addr, &value);
                if (ret >= 0)
                {
                    CTC_DKIT_PRINT("Read pcie memory address 0x%08x, value = 0x%08x\n", addr, value);
                }
            }
            else
            {
                CTC_DKIT_PRINT("g_dal_op.pci_read is not registered!!!\n");
            }
            break;
        default:
            break;
    }

    return ret;
}

STATIC int32
_ctc_usw_dkit_internal_pcie_write(uint8 lchip, dkits_pcie_ctl_t pcie_ctl, uint32 addr, uint32 value)
{
    int32 ret = CLI_SUCCESS;
    CTC_DKIT_LCHIP_CHECK(lchip);
    switch (pcie_ctl)
    {
        case DKIT_PCIE_CFG_W:
            if (g_dal_op.pci_conf_write)
            {
                ret = g_dal_op.pci_conf_write(lchip, addr, value);
                if (ret >= 0)
                {
                    CTC_DKIT_PRINT("Write pcie config address 0x%08x, value = 0x%08x\n", addr, value);
                }
            }
            else
            {
                CTC_DKIT_PRINT("g_dal_op.pci_conf_write is not registered!!!\n");
            }
            break;
        case DKIT_PCIE_MEM_W:
            if (g_dal_op.pci_write)
            {
                ret = g_dal_op.pci_write(lchip, addr, value);
                if (ret >= 0)
                {
                    CTC_DKIT_PRINT("Write pcie memory address 0x%08x, value = 0x%08x\n", addr, value);
                }
            }
            else
            {
                CTC_DKIT_PRINT("g_dal_op.pci_write is not registered!!!\n");
            }
            break;
        default:
            break;
    }
    return ret;
}

STATIC int32
_ctc_usw_dkit_internal_module_process(ctc_cmd_element_t *self, ctc_vti_t *vty, int argc, char **argv)
{

    uint32 lchip = 0;
    uint8 arg_id = 0;
    char *module_name = NULL;
    char *master_name = NULL;
    uint8  dump_all = 0;
    sal_file_t p_wf = NULL;
    char cfg_file[256] = {0};
    int32 ret = CLI_SUCCESS;
    tbls_id_t tmp_tableid = 0;

    master_name = argv[0];
    module_name = argv[1];
    CTC_DKIT_LCHIP_CHECK(lchip);
    DRV_INIT_CHECK(lchip);

    arg_id = ctc_cli_get_prefix_item(&argv[0], argc, "lchip", sal_strlen("lchip"));
    if (0xFF != arg_id)
    {
        CTC_CLI_GET_UINT32("lchip", lchip, argv[arg_id + 1]);
    }

    arg_id = ctc_cli_get_prefix_item(&argv[0], argc, "output-file", sal_strlen("output-file"));
    if (0xFF != arg_id)
    {
        sal_memcpy(cfg_file, argv[arg_id + 1], sal_strlen(argv[arg_id + 1]));
        sal_sprintf(cfg_file, "%s%s", argv[arg_id + 1], DUMP_DECODC_TXT_FILE_POSTFIX);
        CTC_DKIT_PRINT("file name: %s\n",cfg_file);
        p_wf = sal_fopen(cfg_file, "wb+");
        if (NULL == p_wf)
        {
            CTC_DKIT_PRINT(" Open file: %s failed!\n\n", cfg_file);
            return CLI_ERROR;
        }
    }

    /*get module id*/
    if ((0 == sal_strcmp("all", module_name)) && (0 == sal_strcmp("all", master_name)))
    {
        dump_all = 1;
    }

    if(dump_all)
    {
        for (tmp_tableid = 0; tmp_tableid < MaxTblId_t; tmp_tableid++)
        {
            if (0 == TABLE_STATS(lchip, tmp_tableid))
            {
                continue;
            }
            CTC_DKITS_PRINT_FILE(p_wf, "%s\n", TABLE_NAME(lchip, tmp_tableid));
        }
    }
    else
    {
        for (tmp_tableid = 0; tmp_tableid < MaxTblId_t; tmp_tableid++)
        {
            if (0 == TABLE_STATS(lchip, tmp_tableid))
            {
                continue;
            }
            if (0 == sal_strcasecmp(module_name, MODULE_NAME(lchip, tmp_tableid)))
            {
                CTC_DKITS_PRINT_FILE(p_wf, "%s\n", TABLE_NAME(lchip, tmp_tableid));
            }
        }
    }
    CTC_DKITS_PRINT_FILE(p_wf, "\n");
    if (NULL != p_wf)
    {
        sal_fclose(p_wf);
    }
    return ret;
}

STATIC int32
_ctc_usw_dkit_internal_module_init(int (*func) (ctc_cmd_element_t *, ctc_vti_t *, int, char **), uint8 cli_tree_mode)
{


#define MAX_CLI_LEN         128
#define MAX_CLI_TIPS_LEN    64

    ctc_cmd_element_t *cli_cmd = NULL;
    int32 loop = 0, loop_mod = 0;
    char *cmd_str = NULL;
    char **cli_help = NULL;
    int cnt = 0, tip_node_num = 0;
    char module_name[MAX_CLI_TIPS_LEN] = {0};
    dkit_modules_t modules_list[MaxModId_m] =
    {
        {"all", "all"},
        {"ipe", "IpeFwd"}, {"ipe", "IpeHdrAdj"}, {"ipe", "IpeIntfMap"}, {"ipe", "IpeLkupMgr"}, {"ipe", "IpePktProc"}, {"ipe", "NetRx"},
        {"epe", "EpeAclOam"}, {"epe", "EpeHdrAdj"}, {"epe", "EpeHdrEdit"}, {"epe", "EpeHdrProc"}, {"epe", "EpeNextHop"}, {"epe", "NetTx"},
        {"bsr", "BufRetrv"}, {"bsr", "BufStore"}, {"bsr", "MetFifo"}, {"bsr", "QMgrEnq"}, {"bsr", "QMgrMsgStore"},
        {"oam", "OamAutoGenPkt"}, {"oam", "OamFwd"}, {"oam", "OamParser"}, {"oam", "OamProc"},
        {"rlm", "RlmAdCtl"}, {"rlm", "RlmCsCtl"}, {"rlm", "RlmHsCtl"}, {"rlm", "RlmNetCtl"},
        {"share", "FibAcc"}, {"share", "FibEngine"}, {"share", "FlowAcc"}, {"share", "FlowHash"}, {"share", "FlowTcam"},
        {"share", "UserIdHash"}, {"share", "Parser"}, {"share", "EgrOamHash"},
        {"mac", "Hss15GUnitWrapper"}, {"mac", "Hss28GUnitWrapper"}, {"mac", "I2CMaster"}, {"mac", "Mdio"},
        {"misc", "PbCtl"}, {"misc", "PcieIntf"}, {"misc", "RefTrigger"}, {"misc", "SharedPcs"}, {"misc", "SupClockTree"},
        {"misc", "SupCtl"}, {"misc", "SupMacro"}, {"misc", "TsEngine"}, {"misc", "TsEngineRef"}, {"misc", "Dlb"},
        {"misc", "DmaCtl"}, {"misc", "DsAging"}, {"misc", "Efd"}, {"misc", "GlobalStats"}, {"misc", "OobFc"},
        {NULL, NULL},
    };

    while(modules_list[loop].master_name && modules_list[loop].module_name)
    {
        cmd_str = sal_malloc(MAX_CLI_LEN);
        if (NULL == cmd_str)
        {
            goto error_return;
        }
        sal_memset(cmd_str, 0, MAX_CLI_LEN);

        sal_memset(&module_name, 0, MAX_CLI_TIPS_LEN);
        for (loop_mod = 0; loop_mod < sal_strlen(modules_list[loop].module_name); loop_mod++)
        {
            module_name[loop_mod] = sal_tolower(modules_list[loop].module_name[loop_mod]);
        }

        sal_sprintf(cmd_str, "show statistic (%s) (%s)  (output-file FILE_NAME|) (lchip CHIP_ID|)", modules_list[loop].master_name, module_name);
        tip_node_num = 11;

        cli_help = sal_malloc(sizeof(char*)*tip_node_num);
        if (NULL == cli_help)
        {
            goto error_return;
        }
        for (cnt= 0; cnt < tip_node_num - 1; cnt++)
        {
            cli_help[cnt] = sal_malloc(MAX_CLI_TIPS_LEN);
            if (NULL == cli_help[cnt])
            {
                goto error_return;
            }
            sal_memset(cli_help[cnt], 0, MAX_CLI_TIPS_LEN);
        }
        cli_help[tip_node_num -1] = NULL;

        sal_sprintf(cli_help[0], "show %s cmd", module_name);
        sal_sprintf(cli_help[1], "module statistic");
        sal_sprintf(cli_help[2], "%s status", modules_list[loop].master_name);
        sal_sprintf(cli_help[3], "%s debug info", module_name);
        sal_sprintf(cli_help[4], "module index");
        sal_sprintf(cli_help[5], "module index");
        sal_sprintf(cli_help[6], "Centec file");
        sal_sprintf(cli_help[7], "Centec file");
        sal_sprintf(cli_help[8], CTC_DKITS_CHIP_DESC);
        sal_sprintf(cli_help[9], CTC_DKITS_CHIP_ID_DESC);

        cli_cmd = sal_malloc(sizeof(ctc_cmd_element_t));
        if (NULL == cli_cmd)
        {
            goto error_return;
        }
        sal_memset(cli_cmd, 0 ,sizeof(ctc_cmd_element_t));
        cli_cmd->func   = func;
        cli_cmd->string = cmd_str;
        cli_cmd->doc    = cli_help;
        install_element(cli_tree_mode, cli_cmd);

        loop++;
    }

    return CLI_SUCCESS;

error_return:

    for (cnt= 0; (cnt < tip_node_num) && cli_help; cnt++)
    {
        if (cli_help[cnt])
        {
            sal_free(cli_help[cnt]);
            cli_help[cnt] = NULL;
        }
    }
    if (cli_help)
    {
        sal_free(cli_help);
        cli_help = NULL;
    }
    if (cmd_str)
    {
        sal_free(cmd_str);
        cmd_str = NULL;
    }
    return CLI_ERROR;
}


STATIC int32
_ctc_usw_dkit_internal_mac_soft_rst(uint8  lchip , uint32 mac_id, uint32 enable)
{
#ifdef NEVER
    uint8  i = 0;
    int32  ret = CLI_SUCCESS;
    uint32 cmd = 0;
    SharedPcsSoftRst0_m shared_pcs_soft_rst;
    CgPcsSoftRst0_m cg_pcs_soft_rst;
    ctc_dkit_interface_status_t status;

    uint32 soft_rst_rx_field[4] = {SharedPcsSoftRst0_softRstPcsRx0_f,
                                   SharedPcsSoftRst0_softRstPcsRx1_f,
                                   SharedPcsSoftRst0_softRstPcsRx2_f,
                                   SharedPcsSoftRst0_softRstPcsRx3_f};
    uint32 soft_rst_tx_field[4] = {SharedPcsSoftRst0_softRstPcsTx0_f,
                                   SharedPcsSoftRst0_softRstPcsTx1_f,
                                   SharedPcsSoftRst0_softRstPcsTx2_f,
                                   SharedPcsSoftRst0_softRstPcsTx3_f};

    sal_memset(&status, 0, sizeof(ctc_dkit_interface_status_t));
    ctc_usw_dkit_interface_get_mac_status(lchip, &status);

    if (0 == status.valid[mac_id])
    {
        CTC_DKIT_PRINT("%s %d, invalid macid:%u\n", __FUNCTION__, __LINE__, mac_id);
        return CLI_SUCCESS;
    }

    if (CTC_DKIT_INTERFACE_CG != status.mac_mode[mac_id])
    {
        sal_memset(&shared_pcs_soft_rst, 0, sizeof(SharedPcsSoftRst0_m));
        cmd = DRV_IOR(SharedPcsSoftRst0_t + (PCS_ID(mac_id) >> 2), DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &shared_pcs_soft_rst);
        if (ret < 0)
        {
            CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
            return CLI_ERROR;
        }

        if ((CTC_DKIT_INTERFACE_SGMII == status.mac_mode[mac_id]) || (CTC_DKIT_INTERFACE_XFI == status.mac_mode[mac_id]))
        {
            DRV_SET_FIELD_V(SharedPcsSoftRst0_t, soft_rst_rx_field[PCS_ID(mac_id) & 0x3], &shared_pcs_soft_rst, enable);
            DRV_SET_FIELD_V(SharedPcsSoftRst0_t, soft_rst_tx_field[PCS_ID(mac_id) & 0x3], &shared_pcs_soft_rst, enable);
        }
        else if ((CTC_DKIT_INTERFACE_XAUI == status.mac_mode[mac_id]) || (CTC_DKIT_INTERFACE_XLG == status.mac_mode[mac_id]))
        {
            for (i = 0; i < 4; i++)
            {
                DRV_SET_FIELD_V(SharedPcsSoftRst0_t, soft_rst_rx_field[(PCS_ID(mac_id) & 0x3) + i], &shared_pcs_soft_rst, enable);
                DRV_SET_FIELD_V(SharedPcsSoftRst0_t, soft_rst_tx_field[(PCS_ID(mac_id) & 0x3) + i], &shared_pcs_soft_rst, enable);
            }
            SetSharedPcsSoftRst0(V, softRstXlgRx_f,  &shared_pcs_soft_rst, enable);
            SetSharedPcsSoftRst0(V, softRstXlgTx_f,  &shared_pcs_soft_rst, enable);
        }

        cmd = DRV_IOW(SharedPcsSoftRst0_t + (PCS_ID(mac_id) >> 2), DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(lchip, 0, cmd, &shared_pcs_soft_rst);
        if (ret < 0)
        {
            CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
            return CLI_ERROR;
        }
    }
    else
    {
        sal_memset(&cg_pcs_soft_rst, 0, sizeof(CgPcsSoftRst0_m));
        SetCgPcsSoftRst0(V, softRstPcsRx_f, &cg_pcs_soft_rst, enable);
        SetCgPcsSoftRst0(V, softRstPcsTx_f, &cg_pcs_soft_rst, enable);

        if (36 == PCS_ID(mac_id))
        {
            cmd = DRV_IOW(CgPcsSoftRst0_t, DRV_ENTRY_FLAG);
        }
        else if (44 == PCS_ID(mac_id))
        {
            cmd = DRV_IOW(CgPcsSoftRst1_t, DRV_ENTRY_FLAG);
        }
        else if (84 == PCS_ID(mac_id))
        {
            cmd = DRV_IOW(CgPcsSoftRst2_t, DRV_ENTRY_FLAG);
        }
        else if (92 == PCS_ID(mac_id))
        {
            cmd = DRV_IOW(CgPcsSoftRst3_t, DRV_ENTRY_FLAG);
        }
        ret = DRV_IOCTL(lchip, 0, cmd, &cg_pcs_soft_rst);
        if (ret < 0)
        {
            CTC_DKIT_PRINT("%s %d\n", __FUNCTION__, __LINE__);
            return CLI_ERROR;
        }
    }

    return CLI_SUCCESS;
#endif /* never */
    return CLI_SUCCESS;
}


STATIC int32
_ctc_usw_dkit_internal_show_lkup_req_table(uint8 lchip, uint32 tbl_id)
{

    uint32* data_entry = NULL;
    uint32* value = NULL;
    uint32 cmd = 0;
    tables_info_t* tbl_ptr = NULL;
    fields_t* fld_ptr = NULL;
    int32 fld_idx = 0;
    char str[35] = {0};
    char format[10] = {0};

    data_entry = (uint32*)sal_malloc(MAX_ENTRY_WORD * sizeof(uint32));
    if(NULL == data_entry)
    {
        return CLI_ERROR;
    }
    value = (uint32*)sal_malloc(MAX_ENTRY_WORD * sizeof(uint32));
    if(NULL == value)
    {
        sal_free(data_entry);
        return CLI_ERROR;
    }

    sal_memset(data_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));
    sal_memset(value, 0, MAX_ENTRY_WORD * sizeof(uint32));

    tbl_ptr = TABLE_INFO_PTR(lchip, tbl_id);
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, data_entry);

    CTC_DKIT_PRINT("%s:\n", TABLE_NAME(lchip, tbl_id));
    CTC_DKIT_PRINT("------------------------------------\n");
    for (fld_idx = 0; fld_idx < tbl_ptr->field_num; fld_idx++)
    {
        sal_memset(value, 0 , MAX_ENTRY_WORD * sizeof(uint32));
        fld_ptr = &(tbl_ptr->ptr_fields[fld_idx]);

        drv_get_field(lchip, tbl_id, fld_idx, data_entry, value);
        CTC_DKIT_PRINT("%-13s%-40s\n",  CTC_DKITS_HEX_FORMAT(str, format, value[0], 8), fld_ptr->ptr_field_name);
    }
    CTC_DKIT_PRINT("------------------------------------\n");

    if(NULL != data_entry)
    {
        sal_free(data_entry);
    }
    if(NULL != value)
    {
        sal_free(value);
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_internal_show_lkup_req(uint8 lchip, uint32 module_bitmap)
{

    uint32 lkup_stats_tbl[DKIT_LKUP_STATS_MAX]=
    {
        IpeHdrAdjDebugStats_t,
        IpeIntfMapDebugStats_t,
        IpeLkupMgrDebugStats_t,
        IpePktProcDebugStats_t,
        IpeFwdDebugStats_t,
        EpeHdrAdjDebugStats_t,
        EpeNextHopDebugStats_t,
        EpeHdrProcDebugStats_t,
        EpeHdrEditDebugStats_t,
        EpeAclQosDebugStats_t,
        EpeClaDebugStats_t,
        EpeOamDebugStats_t
    };
    uint8 i = 0;
    CTC_DKIT_LCHIP_CHECK(lchip);
    for (i = 0; i < DKIT_LKUP_STATS_MAX; i++)
    {
        if (DKITS_IS_BIT_SET(module_bitmap, i) || (0 == module_bitmap))
        {
            _ctc_usw_dkit_internal_show_lkup_req_table(lchip, lkup_stats_tbl[i]);
        }
    }

    return CLI_SUCCESS;
}

#define ____4_CLI____

CTC_CLI(cli_usw_dkit_serdes_read,
        cli_usw_dkit_serdes_read_cmd,
        "serdes SERDES_ID read (link-tx|link-rx|plla|pllb|aec|aet|common-pll|link-training|bist|firmware|all) ADDR_OFFSET (lchip CHIP_ID|)",
        "Serdes",
        "Serdes ID",
        "Read address",
        "Tx address",
        "Rx address",
        "PLLA address",
        "PLLB address",
        "AEC address",
        "AET address",
        "Common and PLL address",
        "Link trainging address",
        "Bist address",
        "Firmware address",
        "All",
        "Address offset",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_wr;
    uint32 index = 0;
    uint8 lchip = 0;

    sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));

    CTC_CLI_GET_UINT32_RANGE("serdes-id", ctc_dkit_serdes_wr.serdes_id, argv[0], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT32_RANGE("addr-offset", ctc_dkit_serdes_wr.addr_offset, argv[2], 0, CTC_MAX_UINT32_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("link-tx");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
    }
    index = CTC_CLI_GET_ARGC_INDEX("link-rx");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_RX;
    }
    index = CTC_CLI_GET_ARGC_INDEX("plla");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_PLLA;
    }
    index = CTC_CLI_GET_ARGC_INDEX("pllb");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_PLLB;
    }
    index = CTC_CLI_GET_ARGC_INDEX("aec");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_AEC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("aet");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_AET;
    }
    index = CTC_CLI_GET_ARGC_INDEX("common-pll");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_COMMON_PLL;
    }
    index = CTC_CLI_GET_ARGC_INDEX("link-training");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_LINK_TRAINING;
    }
    index = CTC_CLI_GET_ARGC_INDEX("bist");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_BIST;
    }
    index = CTC_CLI_GET_ARGC_INDEX("firmware");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_FW;
    }
    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_ALL;
    }
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ctc_dkit_serdes_wr.lchip = lchip;
    if ((CTC_DKIT_SERDES_AEC == ctc_dkit_serdes_wr.type)
        || (CTC_DKIT_SERDES_AET == ctc_dkit_serdes_wr.type))
    {
        ctc_usw_dkit_misc_read_serdes_aec_aet(&ctc_dkit_serdes_wr);
    }
    else
    {
        ctc_usw_dkit_misc_read_serdes(&ctc_dkit_serdes_wr);
    }


    CTC_DKIT_PRINT("Read data:0x%04x\n", ctc_dkit_serdes_wr.data);

    return CLI_SUCCESS;
}


CTC_CLI(cli_usw_dkit_serdes_write,
        cli_usw_dkit_serdes_write_cmd,
        "serdes SERDES_ID write (link-tx|link-rx|plla|pllb|aec|aet|common-pll|link-training|bist|firmware|all) ADDR_OFFSET VALUE (lchip CHIP_ID|)",
        "Serdes",
        "Serdes ID",
        "Write address",
        "Tx address",
        "Rx address",
        "PLLA address",
        "PLLB address",
        "AEC address",
        "AET address",
        "Common and PLL address",
        "Link trainging address",
        "Bist address",
        "Firmware address",
        "All",
        "Address offset",
        "Value",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    ctc_dkit_serdes_wr_t ctc_dkit_serdes_wr;
    uint32 index = 0;
    uint8 lchip = 0;

    sal_memset(&ctc_dkit_serdes_wr, 0, sizeof(ctc_dkit_serdes_wr_t));

    CTC_CLI_GET_UINT32_RANGE("serdes-id", ctc_dkit_serdes_wr.serdes_id, argv[0], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT32_RANGE("addr-offset", ctc_dkit_serdes_wr.addr_offset, argv[2], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT16_RANGE("value", ctc_dkit_serdes_wr.data, argv[3], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("link-tx");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_TX;
    }
    index = CTC_CLI_GET_ARGC_INDEX("link-rx");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_RX;
    }
    index = CTC_CLI_GET_ARGC_INDEX("plla");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_PLLA;
    }
    index = CTC_CLI_GET_ARGC_INDEX("pllb");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_PLLB;
    }
    index = CTC_CLI_GET_ARGC_INDEX("aec");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_AEC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("aet");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_AET;
    }
    index = CTC_CLI_GET_ARGC_INDEX("common-pll");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_COMMON_PLL;
    }
    index = CTC_CLI_GET_ARGC_INDEX("link-training");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_LINK_TRAINING;
    }
    index = CTC_CLI_GET_ARGC_INDEX("bist");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_BIST;
    }
    index = CTC_CLI_GET_ARGC_INDEX("firmware");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_FW;
    }
    index = CTC_CLI_GET_ARGC_INDEX("all");
    if (index != 0xFF)
    {
        ctc_dkit_serdes_wr.type = CTC_DKIT_SERDES_ALL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ctc_dkit_serdes_wr.lchip = lchip;
    if ((CTC_DKIT_SERDES_AEC == ctc_dkit_serdes_wr.type)
        || (CTC_DKIT_SERDES_AET == ctc_dkit_serdes_wr.type))
    {
       ctc_usw_dkit_misc_write_serdes_aec_aet(&ctc_dkit_serdes_wr);
    }
    else
    {
        ctc_usw_dkit_misc_write_serdes(&ctc_dkit_serdes_wr);
    }

    return CLI_SUCCESS;
}


CTC_CLI(cli_usw_dkit_serdes_reset,
        cli_usw_dkit_serdes_reset_cmd,
        "serdes SERDES_ID reset (lchip CHIP_ID|)",
        "Serdes",
        "Serdes ID",
        "Reset",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
#if 1
    uint8 serdes_id = 0;
    uint8 lchip = 0;
    uint8 index = 0;
    CTC_CLI_GET_UINT8_RANGE("serdes-id", serdes_id, argv[0], 0, CTC_MAX_UINT8_VALUE);
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
#endif
    ctc_usw_dkit_misc_serdes_resert(lchip, serdes_id);

    return CLI_SUCCESS;
}

CTC_CLI(cli_usw_dkit_pcie_read,
        cli_usw_dkit_pcie_read_cmd,
        "pcie read (config|memory) ADDR (lchip CHIP_ID|)",
        "Pcie",
        "Read pcie address",
        "Read pcie cfg address",
        "Read pcie memory address",
        "Address",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint32 index = 0;
    uint32 addr = 0;
    uint32 pcie_ct_type = DKIT_PCIE_MAX;
    uint8 lchip = 0;

    CTC_CLI_GET_UINT32_RANGE("address", addr, argv[1], 0, CTC_MAX_UINT32_VALUE);
    index = CTC_CLI_GET_ARGC_INDEX("config");
    if (index != 0xFF)
    {
        pcie_ct_type = DKIT_PCIE_CFG_R;
    }
    else
    {
        pcie_ct_type = DKIT_PCIE_MEM_R;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    _ctc_usw_dkit_internal_pcie_read(lchip, pcie_ct_type, addr);
    return CLI_SUCCESS;
}

CTC_CLI(cli_usw_dkit_pcie_write,
        cli_usw_dkit_pcie_write_cmd,
        "pcie write (config|memory) ADDR VALUE (lchip CHIP_ID|)",
        "Pcie",
        "Write pcie address",
        "Write pcie cfg address",
        "Write pcie memory address",
        "Address",
        "Value",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint32 index = 0;
    uint32 addr = 0;
    uint32 value = 0;
    uint32 pcie_ct_type = DKIT_PCIE_MAX;
    uint8 lchip = 0;

    CTC_CLI_GET_UINT32_RANGE("address", addr, argv[1], 0, CTC_MAX_UINT32_VALUE);
    CTC_CLI_GET_UINT32_RANGE("value", value, argv[2], 0, CTC_MAX_UINT32_VALUE);
    index = CTC_CLI_GET_ARGC_INDEX("config");
    if (index != 0xFF)
    {
        pcie_ct_type = DKIT_PCIE_CFG_W;
    }
    else
    {
        pcie_ct_type = DKIT_PCIE_MEM_W;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    _ctc_usw_dkit_internal_pcie_write(lchip, pcie_ct_type, addr, value);
    return CLI_SUCCESS;
}

CTC_CLI(cli_usw_dkit_mac_en,
        cli_usw_dkit_mac_en_cmd,
        "mac MAC_ID (enable | disable) (lchip CHIP_ID|)",
        "Mac",
        "Mac id",
        "Enable",
        "Disable",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint8  index = 0, enable = 0;
    uint32 mac_id = 0;
    uint8 lchip = 0;

    CTC_CLI_GET_UINT32("mac-id", mac_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    return _ctc_usw_dkit_internal_mac_soft_rst(lchip, mac_id, enable);
}

CTC_CLI(cli_usw_dkit_show_mac_stats,
        cli_usw_dkit_show_mac_stats_cmd,
        "show stats mac START_MAC_ID END_MAC_ID ((rx | tx) |) (clear|) (lchip CHIP_ID|)",
        "Show",
        "Stats",
        "Mac",
        "Start mac id",
        "End mac id",
        "Recevie",
        "Transmit",
        "Clear after read",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    int32  ret = CLI_SUCCESS;
    uint8  index = 0;
    uint8  clear_index = 0;
    uint32 macid = 0;
    uint32 start_macid = 0;
    uint32 end_macid = 0;
    dkit_mac_stats_t stats;
    uint8 lchip = 0;

    CTC_CLI_GET_UINT32("mac-id", start_macid, argv[0]);
    CTC_CLI_GET_UINT32("mac-id", end_macid, argv[1]);

    if ((start_macid > end_macid)
       || (start_macid >= CTC_DKIT_INTERFACE_MAC_NUM)
       || (end_macid >= CTC_DKIT_INTERFACE_MAC_NUM))
    {
        CTC_DKIT_PRINT("Invalid macid!\n");
        return CLI_SUCCESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    for (macid = start_macid; macid <= end_macid; macid++)
    {
        sal_memset(&stats, 0, sizeof(dkit_mac_stats_t));
        index = CTC_CLI_GET_ARGC_INDEX("rx");
        if ((0xFF != index) || (2 == argc) || ((3 == argc) && (0xFF != clear_index)))
        {
            ret = ctc_usw_dkit_internal_mac_stats_get_rx(lchip, macid, &stats);
            if (CLI_SUCCESS != ret)
            {
                CTC_DKIT_PRINT("Get mac[%u] rx stats error!\n", macid);
            }
            else
            {
                CTC_DKIT_PRINT("\n");
                CTC_DKIT_PRINT(" Mac[%u]ReceiveStats:\n", macid);
                CTC_DKIT_PRINT(" --------------------------------------------------------------\n");
                CTC_DKIT_PRINT(" %-16s%-23s%s\n", "FrameType", "PacketCnt", "ByteCnt");
                CTC_DKIT_PRINT(" --------------------------------------------------------------\n");
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "GoodUcast",       stats.stats_detail.stats.rx_stats.good_ucast_pkts, stats.stats_detail.stats.rx_stats.good_ucast_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "GoodMcast",       stats.stats_detail.stats.rx_stats.good_mcast_pkts, stats.stats_detail.stats.rx_stats.good_mcast_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "GoodBcast",       stats.stats_detail.stats.rx_stats.good_bcast_pkts, stats.stats_detail.stats.rx_stats.good_bcast_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "GoodNormalPause", stats.stats_detail.stats.rx_stats.good_normal_pause_pkts, stats.stats_detail.stats.rx_stats.good_normal_pause_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "GoodPfcPause",    stats.stats_detail.stats.rx_stats.good_pfc_pause_pkts, stats.stats_detail.stats.rx_stats.good_pfc_pause_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "GoodControl",     stats.stats_detail.stats.rx_stats.good_control_pkts, stats.stats_detail.stats.rx_stats.good_control_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "FcsError",        stats.stats_detail.stats.rx_stats.fcs_error_pkts, stats.stats_detail.stats.rx_stats.fcs_error_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "MacOverrun",      stats.stats_detail.stats.rx_stats.mac_overrun_pkts, stats.stats_detail.stats.rx_stats.mac_overrun_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "Good63B",         stats.stats_detail.stats.rx_stats.good_63_pkts, stats.stats_detail.stats.rx_stats.good_63_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "Bad63B",          stats.stats_detail.stats.rx_stats.bad_63_pkts, stats.stats_detail.stats.rx_stats.bad_63_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "64B",             stats.stats_detail.stats.rx_stats.pkts_64, stats.stats_detail.stats.rx_stats.bytes_64);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "65B~127B",        stats.stats_detail.stats.rx_stats.pkts_65_to_127, stats.stats_detail.stats.rx_stats.bytes_65_to_127);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "128B~255B",       stats.stats_detail.stats.rx_stats.pkts_128_to_255, stats.stats_detail.stats.rx_stats.bytes_128_to_255);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "256B~511B",       stats.stats_detail.stats.rx_stats.pkts_256_to_511, stats.stats_detail.stats.rx_stats.bytes_256_to_511);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "512B~1023B",      stats.stats_detail.stats.rx_stats.pkts_512_to_1023, stats.stats_detail.stats.rx_stats.bytes_512_to_1023);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "1024B~1518B",     stats.stats_detail.stats.rx_stats.pkts_1024_to_1518, stats.stats_detail.stats.rx_stats.bytes_1024_to_1518);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "Good1519B",       stats.stats_detail.stats.rx_stats.good_1519_pkts, stats.stats_detail.stats.rx_stats.good_1519_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "Bad1519B",        stats.stats_detail.stats.rx_stats.bad_1519_pkts, stats.stats_detail.stats.rx_stats.bad_1519_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "GoodJumbo",       stats.stats_detail.stats.rx_stats.good_jumbo_pkts, stats.stats_detail.stats.rx_stats.good_jumbo_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "BadJumbo",        stats.stats_detail.stats.rx_stats.bad_jumbo_pkts, stats.stats_detail.stats.rx_stats.bad_jumbo_bytes);
                CTC_DKIT_PRINT("\n");
            }
        }

        index = CTC_CLI_GET_ARGC_INDEX("tx");
        if ((0xFF != index) || (2 == argc) || ((3 == argc) && (0xFF != clear_index)))
        {
            ret = ctc_usw_dkit_internal_mac_stats_get_tx(lchip, macid, &stats);
            if (CLI_SUCCESS != ret)
            {
                CTC_DKIT_PRINT("Get mac[%u] tx stats error!\n", macid);
            }
            else
            {
                CTC_DKIT_PRINT("\n");
                CTC_DKIT_PRINT(" Mac[%u]TransmitStats:\n", macid);
                CTC_DKIT_PRINT(" --------------------------------------------------------------\n");
                CTC_DKIT_PRINT(" %-16s%-23s%s\n", "FrameType", "PacketCnt", "ByteCnt");
                CTC_DKIT_PRINT(" --------------------------------------------------------------\n");
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "GoodUcast",       stats.stats_detail.stats.tx_stats.good_ucast_pkts, stats.stats_detail.stats.tx_stats.good_ucast_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "GoodMcast",       stats.stats_detail.stats.tx_stats.good_mcast_pkts, stats.stats_detail.stats.tx_stats.good_mcast_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "GoodBcast",       stats.stats_detail.stats.tx_stats.good_bcast_pkts, stats.stats_detail.stats.tx_stats.good_bcast_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "GoodPause",       stats.stats_detail.stats.tx_stats.good_pause_pkts, stats.stats_detail.stats.tx_stats.good_pause_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "GoodControl",     stats.stats_detail.stats.tx_stats.good_control_pkts, stats.stats_detail.stats.tx_stats.good_control_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "63B",             stats.stats_detail.stats.tx_stats.pkts_63, stats.stats_detail.stats.tx_stats.bytes_63);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "64B",             stats.stats_detail.stats.tx_stats.pkts_64, stats.stats_detail.stats.tx_stats.bytes_64);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "65B~127B",        stats.stats_detail.stats.tx_stats.pkts_65_to_127, stats.stats_detail.stats.tx_stats.bytes_65_to_127);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "128B~255B",       stats.stats_detail.stats.tx_stats.pkts_128_to_255, stats.stats_detail.stats.tx_stats.bytes_128_to_255);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "256B~511B",       stats.stats_detail.stats.tx_stats.pkts_256_to_511, stats.stats_detail.stats.tx_stats.bytes_256_to_511);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "512B~1023B",      stats.stats_detail.stats.tx_stats.pkts_512_to_1023, stats.stats_detail.stats.tx_stats.bytes_512_to_1023);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "1024B~1518B",     stats.stats_detail.stats.tx_stats.pkts_1024_to_1518, stats.stats_detail.stats.tx_stats.bytes_1024_to_1518);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "1519B",           stats.stats_detail.stats.tx_stats.pkts_1519, stats.stats_detail.stats.tx_stats.bytes_1519);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "Jumbo",           stats.stats_detail.stats.tx_stats.jumbo_pkts, stats.stats_detail.stats.tx_stats.jumbo_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "MacUnderrun",     stats.stats_detail.stats.tx_stats.mac_underrun_pkts, stats.stats_detail.stats.tx_stats.mac_underrun_bytes);
                CTC_DKIT_PRINT(" %-16s%-23"PRIu64"%"PRIu64"\n", "FcsError",        stats.stats_detail.stats.tx_stats.fcs_error_pkts, stats.stats_detail.stats.tx_stats.fcs_error_bytes);
                CTC_DKIT_PRINT("\n");
            }
        }
    }

    clear_index = CTC_CLI_GET_ARGC_INDEX("clear");
    if (0xFF != clear_index)
    {
        ret = _ctc_usw_dkit_internal_mac_stats_reset(lchip);
        if (0 != ret)
        {
            return CLI_ERROR;
        }
    }

    return CLI_SUCCESS;
}

CTC_CLI(cli_usw_dkit_show_lookup_request,
        cli_usw_dkit_show_lookup_request_cmd,
        "show lkup-req ( {ipe-hdr-adj | ipe-intf-map | ipe-lkup-mgr | ipe-pkt-proc | ipe-forward | epe-hdr-adj | epe-next-hop | \
        epe-hdr-proc | epe-acl-oam | epe-hdr-edit} | ) (lchip CHIP_ID|)",
        "Show ",
        "Show lookup request stats",
        "ipehdradj",
        "ipeintfmap",
        "ipelkupmgr",
        "ipepktproc",
        "ipeforward",
        "epehdradj",
        "epenexthop",
        "epehdrproc",
        "epeacloam",
        "epehdredit",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
     uint32 module_bitmap = 0;
     uint8  index = 0;
     uint8 lchip = 0;

     index = CTC_CLI_GET_ARGC_INDEX("ipe-hdr-adj");
     if (0xFF != index)
     {
         DKITS_BIT_SET(module_bitmap, DKIT_LKUP_STATS_IPEHDRADJ);
     }
     index = CTC_CLI_GET_ARGC_INDEX("ipe-intf-map");
     if (0xFF != index)
     {
         DKITS_BIT_SET(module_bitmap, DKIT_LKUP_STATS_IPEINTFMAP);
     }
     index = CTC_CLI_GET_ARGC_INDEX("ipe-lkup-mgr");
     if (0xFF != index)
     {
         DKITS_BIT_SET(module_bitmap, DKIT_LKUP_STATS_IPELKUPMGR);
     }
     index = CTC_CLI_GET_ARGC_INDEX("ipe-pkt-proc");
     if (0xFF != index)
     {
         DKITS_BIT_SET(module_bitmap, DKIT_LKUP_STATS_IPEPKTPROC);
     }
     index = CTC_CLI_GET_ARGC_INDEX("ipe-forward");
     if (0xFF != index)
     {
         DKITS_BIT_SET(module_bitmap, DKIT_LKUP_STATS_IPEFORWARD);
     }
     index = CTC_CLI_GET_ARGC_INDEX("epe-hdr-adj");
     if (0xFF != index)
     {
         DKITS_BIT_SET(module_bitmap, DKIT_LKUP_STATS_EPEHDRADJ);
     }
     index = CTC_CLI_GET_ARGC_INDEX("epe-next-hop");
     if (0xFF != index)
     {
         DKITS_BIT_SET(module_bitmap, DKIT_LKUP_STATS_EPENEXTHOP);
     }
     index = CTC_CLI_GET_ARGC_INDEX("epe-hdr-proc");
     if (0xFF != index)
     {
         DKITS_BIT_SET(module_bitmap, DKIT_LKUP_STATS_EPEHDRPROC);
     }
     index = CTC_CLI_GET_ARGC_INDEX("epe-hdr-edit");
     if (0xFF != index)
     {
         DKITS_BIT_SET(module_bitmap, DKIT_LKUP_STATS_EPEHDREDIT);
     }
     index = CTC_CLI_GET_ARGC_INDEX("epe-acl-oam");
     if (0xFF != index)
     {
         DKITS_BIT_SET(module_bitmap, DKIT_LKUP_STATS_EPEACLOAM0);
         DKITS_BIT_SET(module_bitmap, DKIT_LKUP_STATS_EPEACLOAM1);
         DKITS_BIT_SET(module_bitmap, DKIT_LKUP_STATS_EPEACLOAM2);
     }

     index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    _ctc_usw_dkit_internal_show_lkup_req(lchip, module_bitmap);

    return CLI_SUCCESS;
}

CTC_CLI(cli_usw_dkit_serdes_dump,
        cli_usw_dkit_serdes_dump_cmd,
        "serdes SERDES_ID dump (tx | rx | common-pll | aec-aet |detail|)(file FILENAME | ) (lchip CHIP_ID|)",
        "Serdes",
        "Serdes ID",
        "Dump serdes register",
        "TX register",
        "RX register",
        "Common and PLL register",
        "AEC and AET register",
        "Detail register include bathtub and isi",
        "Output file name",
        "file name",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint16 serdes_id = 0;
    uint8  index = 0;
    uint32 type = CTC_DKIT_SERDES_ALL;
    uint8 lchip = 0;
    char* p_file_name = NULL;

    CTC_CLI_GET_UINT16_RANGE("serdes-id", serdes_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("tx");
    if (0xFF != index)
    {
        type = CTC_DKIT_SERDES_TX;
    }
    index = CTC_CLI_GET_ARGC_INDEX("rx");
    if (0xFF != index)
    {
        type = CTC_DKIT_SERDES_RX;
    }
    index = CTC_CLI_GET_ARGC_INDEX("common-pll");
    if (0xFF != index)
    {
        type = CTC_DKIT_SERDES_COMMON_PLL;
    }
    index = CTC_CLI_GET_ARGC_INDEX("aec-aet");
    if (0xFF != index)
    {
        type = CTC_DKIT_SERDES_AEC_AET;
    }
    index = CTC_CLI_GET_ARGC_INDEX("detail");
    if (0xFF != index)
    {
        type = CTC_DKIT_SERDES_DETAIL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("file");
    if (0xFF != index)
    {
        p_file_name = argv[index + 1];
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    ctc_usw_dkit_misc_serdes_status(lchip, serdes_id, type, p_file_name);

    return CLI_SUCCESS;
}

CTC_CLI(cli_usw_dkit_access_mode,
        cli_usw_dkit_access_mode_cmd,
        "access-mode (pcie|i2c) (lchip CHIP_ID|)",
        "Set access mode cmd",
        "PCI mode",
        "I2C mode",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint32 index = 0;
    uint8 lchip = 0;
    drv_access_type_t access_type = DRV_MAX_ACCESS_TYPE;

    if (0 == sal_strncmp(argv[0], "pcie", sal_strlen("pcie")))
    {
        access_type = DRV_PCI_ACCESS;
    }
    else if (0 == sal_strncmp(argv[0], "i2c", sal_strlen("i2c")))
    {
        access_type = DRV_I2C_ACCESS;
    }

   index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    CTC_DKIT_LCHIP_CHECK(lchip);
    drv_set_access_type(lchip, access_type);
    return CLI_SUCCESS;
}
CTC_CLI(cli_usw_dkit_show_pcs_status,
        cli_usw_dkit_show_pcs_status_cmd,
        "show pcs status mac MAC (lchip CHIP_ID|)",
        "Show",
        "Pcs status",
        "Pcs status",
        "Mac id",
        "Mac id",
        CTC_DKITS_CHIP_DESC,
        CTC_DKITS_CHIP_ID_DESC)
{
    uint16 mac_id = 0;
    uint32 index = 0;
    uint8 lchip = 0;
    uint32 ret = 0;
    CTC_CLI_GET_UINT16_RANGE("mac-id", mac_id, argv[0], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_INTEGER("lchip", lchip, argv[index + 1]);
    }
    lchip = g_ctcs_api_en? g_api_lchip : lchip;
    CTC_DKIT_LCHIP_CHECK(lchip);
    if(g_dkit_chip_api[lchip]->dkits_show_pcs_status)
    {
        ret = g_dkit_chip_api[lchip]->dkits_show_pcs_status(lchip, mac_id);
    }
    return ret;
}
int32
ctc_usw_dkit_show_pcs_register(uint8 lchip)
{
#ifdef DUET2
    if (DRV_IS_DUET2(lchip))
    {
        g_dkit_chip_api[lchip]->dkits_show_pcs_status = ctc_dt2_dkit_interface_show_pcs_status;
    }
#endif
#ifdef TSINGMA
    if (DRV_IS_TSINGMA(lchip))
    {
        g_dkit_chip_api[lchip]->dkits_show_pcs_status = ctc_tm_dkit_interface_show_pcs_status;
    }
#endif
    return 0;

}

int32
ctc_usw_dkit_internal_cli_init(uint8 lchip, uint8 cli_tree_mode)
{
    CTC_DKIT_PRINT_DEBUG("%s %d\n", __FUNCTION__, __LINE__);
    ctc_usw_dkit_stats_init();
    ctc_usw_dkit_show_pcs_register(lchip);
    _ctc_usw_dkit_internal_module_init(_ctc_usw_dkit_internal_module_process,  cli_tree_mode);
    _ctc_usw_dkit_internal_bus_init(lchip, _ctc_usw_dkit_internal_bus_process, cli_tree_mode, TRUE);
    install_element(cli_tree_mode, &cli_usw_dkit_serdes_read_cmd);
    install_element(cli_tree_mode, &cli_usw_dkit_serdes_write_cmd);
    install_element(cli_tree_mode, &cli_usw_dkit_serdes_reset_cmd);
    install_element(cli_tree_mode, &cli_usw_dkit_serdes_dump_cmd);
    install_element(cli_tree_mode, &cli_usw_dkit_pcie_read_cmd);
    install_element(cli_tree_mode, &cli_usw_dkit_pcie_write_cmd);
    install_element(cli_tree_mode, &cli_usw_dkit_show_pcs_status_cmd);
     /*install_element(cli_tree_mode, &cli_usw_dkit_mac_en_cmd);*/
     /*install_element(cli_tree_mode, &cli_usw_dkit_show_mac_stats_cmd);*/

    install_element(cli_tree_mode, &cli_usw_dkit_show_lookup_request_cmd);
    install_element(cli_tree_mode, &cli_usw_dkit_access_mode_cmd);

    return CLI_SUCCESS;
}


int32
ctc_usw_dkit_internal_cli_deinit(uint8 lchip, uint8 cli_tree_mode)
{
    CTC_DKIT_PRINT_DEBUG("%s %d\n", __FUNCTION__, __LINE__);
    #if 0
    ctc_usw_dkit_stats_init();
    _ctc_usw_dkit_internal_module_init(_ctc_usw_dkit_internal_module_process,  cli_tree_mode);
    uninstall_element(cli_tree_mode, &cli_dkit_serdes_read_cmd);
    uninstall_element(cli_tree_mode, &cli_dkit_serdes_write_cmd);
    uninstall_element(cli_tree_mode, &cli_dkit_serdes_reset_cmd);
    uninstall_element(cli_tree_mode, &cli_dkit_serdes_dump_cmd);
    #endif
    _ctc_usw_dkit_internal_bus_init(lchip, _ctc_usw_dkit_internal_bus_process, cli_tree_mode, FALSE);
    uninstall_element(cli_tree_mode, &cli_usw_dkit_pcie_read_cmd);
    uninstall_element(cli_tree_mode, &cli_usw_dkit_pcie_write_cmd);
    uninstall_element(cli_tree_mode, &cli_usw_dkit_show_pcs_status_cmd);
     /*uninstall_element(cli_tree_mode, &cli_usw_dkit_mac_en_cmd);*/
     /*uninstall_element(cli_tree_mode, &cli_usw_dkit_show_mac_stats_cmd);*/

    uninstall_element(cli_tree_mode, &cli_usw_dkit_show_lookup_request_cmd);

    return CLI_SUCCESS;
}
