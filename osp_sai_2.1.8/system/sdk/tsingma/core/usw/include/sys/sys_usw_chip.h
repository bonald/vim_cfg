/**
 @file sys_usw_chip.h

 @date 2009-10-19

 @version v2.0

 The file contains all chip related APIs of sys layer
*/

#ifndef _SYS_USW_CHIP_H
#define _SYS_USW_CHIP_H
#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_const.h"
#include "ctc_chip.h"
#include "ctc_port.h"
#include "ctc_debug.h"
#include "ctc_register.h"
#include "sys_usw_chip_global.h"
#include "sys_usw_common.h"

/****************************************************************
*
* Defines and Macros
*
****************************************************************/
#define SYS_CHIP_HSS12G_MACRO_NUM        4
#define STS_CHIP_HSS12G_LINK_NUM         8
#define SYS_HSS12G_PORT_EN_REG1          6
#define SYS_HSS12G_PORT_EN_REG2          7
#define SYS_HSS12G_PORT_RESET_REG1       8
#define SYS_HSS12G_PORT_RESET_REG2       9
#define SYS_HSS12G_PLL_COMMON_HIGH_ADDR  16
#define SYS_CHIP_CPU_MAC_ETHER_TYPE      0x5a5a
#define SYS_CHIP_SWITH_THREADHOLD        20

#define SYS_DRV_LOCAL_PORT_LENGTH           9           /**< Local port id length(unit: bit) */
#define SYS_DRV_LOCAL_PORT_MASK             0x1FF        /**< Local port mask */

#define SYS_DRV_GPORT_TO_GCHIP(gport)        (((gport) >> SYS_DRV_LOCAL_PORT_LENGTH) & CTC_GCHIP_MASK)
#define SYS_DRV_GPORT_TO_LPORT(gport)        ((gport)& SYS_DRV_LOCAL_PORT_MASK)
#define SYS_DRV_IS_LINKAGG_PORT(gport)       ((SYS_DRV_GPORT_TO_GCHIP(gport)) == CTC_LINKAGG_CHIPID)

#define SYS_MAP_DRV_LPORT_TO_SYS_LPORT(lport) (lport)

#define SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport)  (lport)

#define SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport) SYS_MAP_SYS_LPORT_TO_DRV_LPORT(CTC_MAP_GPORT_TO_LPORT((gport)))

#define SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport) CTC_MAP_LPORT_TO_GPORT(gchip, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(lport))

#define SYS_MAP_DRV_LPORT_TO_SLICE(lport) (((lport) < MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM)) ? 0 : 1)

#define SYS_MAP_CHANID_TO_SLICE(chan) ((chan >= 64) ? 1 : 0)

#define SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport) \
{\
    uint8 gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport);\
    SYS_GLOBAL_CHIPID_CHECK(gchip);\
    if (FALSE == sys_usw_chip_is_local(lchip, gchip)) \
        return CTC_E_INVALID_CHIP_ID;\
    lport = CTC_MAP_GPORT_TO_LPORT(gport);\
    if ((lport >= SYS_USW_MAX_PORT_NUM_PER_CHIP) || CTC_IS_CPU_PORT(gport))\
        return CTC_E_INVALID_PORT;\
     lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport);\
}

#define SYS_GCHIP_IS_REMOTE(lchip, gchip) ((0x1F != gchip)&&(!sys_usw_chip_is_local(lchip, gchip)))

#define SYS_MAP_CTC_GPORT_TO_DRV_GPORT(gport) ((CTC_MAP_GPORT_TO_GCHIP(gport) << SYS_DRV_LOCAL_PORT_LENGTH)\
                                              | (CTC_IS_LINKAGG_PORT(gport) ? CTC_MAP_GPORT_TO_LPORT(gport) : SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport)))

#define SYS_MAP_CTC_GPORT_TO_GCHIP(gport)     (((gport) >> CTC_LOCAL_PORT_LENGTH) & CTC_GCHIP_MASK)

#define SYS_MAP_DRV_GPORT_TO_CTC_GPORT(gport)   (CTC_MAP_LPORT_TO_GPORT(SYS_DRV_GPORT_TO_GCHIP(gport)\
        , SYS_DRV_IS_LINKAGG_PORT(gport)? (SYS_DRV_GPORT_TO_LPORT(gport)):(SYS_MAP_DRV_LPORT_TO_SYS_LPORT(SYS_DRV_GPORT_TO_LPORT(gport)))))

#define SYS_MAX_PHY_PORT_CHECK(lport) \
{ \
    if (((lport) & 0xff) >= 64) \
    {  \
        return CTC_E_INVALID_PORT;  \
    } \
}

/*Below Define is for CTC APIs dispatch*/
/*---------------------------------------------------------------*/
extern uint8 g_ctcs_api_en;

#define SYS_MAP_GCHIP_TO_LCHIP(gchip, lchip) \
    do{\
        if (gchip > CTC_MAX_GCHIP_CHIP_ID || (gchip == 0x1F))\
        {\
            return CTC_E_INVALID_CHIP_ID;\
        }\
        if(!g_ctcs_api_en)\
        {\
            if(CTC_E_NONE != sys_usw_get_local_chip_id(gchip, &lchip))\
            {\
                return CTC_E_INVALID_CHIP_ID;\
            }\
        }\
    }while(0);

#define SYS_USW_MAX_LOCAL_CHIP_NUM               30

#define SYS_USW_MAX_LINKAGG_GROUP_NUM            MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM)
#define SYS_USW_MAX_PORT_NUM_PER_CHIP            256
#define SYS_USW_MAX_ECPN                         64
#define SYS_USW_MAX_IP_RPF_IF                    16
#define SYS_USW_MIN_CTC_L3IF_ID                  1
#define SYS_USW_MAX_APS_GROUP_NUM                2048
#define SYS_USW_MAX_DEFAULT_ECMP_NUM             16
#define SYS_USW_MAX_ACL_PORT_BITMAP_NUM          128
#define SYS_USW_MAX_ACL_PORT_CLASSID             255
#define SYS_USW_MIN_ACL_PORT_CLASSID             1
#define SYS_USW_MAX_SCL_PORT_CLASSID             255
#define SYS_USW_MIN_SCL_PORT_CLASSID             1
#define SYS_USW_MAX_ACL_VLAN_CLASSID             255
#define SYS_USW_MIN_ACL_VLAN_CLASSID             1
#define SYS_USW_PKT_CPUMAC_LEN                   18
#define SYS_USW_PKT_HEADER_LEN                   40
#define SYS_USW_LPORT_CPU                        0x10000000
#define SYS_USW_MAX_CPU_MACDA_NUM                4
#define SYS_USW_MAX_LINKAGG_ID                   (MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM)-1)
#define SYS_USW_MAX_CPU_REASON_GROUP_NUM         16
#define SYS_USW_MAX_QNUM_PER_GROUP               8
#define SYS_USW_MAX_INTR_GROUP                   6
#define SYS_USW_LINKAGGID_MASK                   (MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM)-1)
#define SYS_USW_MAX_IPUC_IP_TUNNEL_IF_NUM        4

#define SYS_USW_IPMC_RP_INTF                     16
#define SYS_USW_CHIP_FFE_PARAM_NUM               5
#define SYS_USW_MAX_GCHIP_CHIP_ID                0x7F

#define SYS_USW_MLAG_ISOLATION_GROUP             31
#define SYS_USW_MAX_RESERVED_PORT                32
#define SYS_USW_MAX_NORMAL_PANNEL_PORT           96
#define SYS_USW_MAX_EXTEND_PANNEL_PORT           384
#define SYS_USW_NORMAL_PANNEL_PORT_BASE0         0
#define SYS_USW_NORMAL_PANNEL_PORT_BASE1         64
#define SYS_USW_EXTEND_PANNEL_PORT_BASE0         128
#define SYS_USW_EXTEND_PANNEL_PORT_BASE1         320

#define SYS_ACL_VLAN_CLASS_ID_CHECK(id) \
    if (((id) < SYS_USW_MIN_ACL_VLAN_CLASSID) || ((id) > SYS_USW_MAX_ACL_VLAN_CLASSID)) \
    { \
        return CTC_E_INVALID_PARAM; \
    }

#define SYS_ACL_PORT_CLASS_ID_CHECK(id) \
    if (((id) < SYS_USW_MIN_ACL_PORT_CLASSID) || ((id) > SYS_USW_MAX_ACL_PORT_CLASSID)) \
    { \
        return CTC_E_INVALID_PARAM; \
    }

#define SYS_SCL_PORT_CLASS_ID_CHECK(id) \
    if (((id) < SYS_USW_MIN_SCL_PORT_CLASSID) || ((id) > SYS_USW_MAX_SCL_PORT_CLASSID)) \
    { \
        return CTC_E_INVALID_PARAM; \
    }

#define SYS_LOCAL_CHIPID_CHECK(lchip)                                      \
    do {                                                                   \
        uint8 local_chip_num = sys_usw_get_local_chip_num(lchip);        \
        if (lchip >= local_chip_num){ return CTC_E_INVALID_CHIP_ID; } \
    } while (0);


#define SYS_CHIP_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(chip, chip, CHIP_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0)

/*#define SYS_USW_CHIP_CLOCK 625*/
#define CHIP_LOCK(lchip) \
    if (p_usw_chip_master[lchip]->p_chip_mutex) sal_mutex_lock(p_usw_chip_master[lchip]->p_chip_mutex)

#define CHIP_UNLOCK(lchip) \
    if (p_usw_chip_master[lchip]->p_chip_mutex) sal_mutex_unlock(p_usw_chip_master[lchip]->p_chip_mutex)

#define SYS_LPORT_IS_CPU_ETH(lchip, lport) sys_usw_chip_is_eth_cpu_port(lchip, lport)


#define SYS_HSS12G_ADDRESS_CONVERT(addr_h, addr_l)   (((addr_h) << 6) | (addr_l))

#define SYS_XQMAC_PER_SLICE_NUM      12
#define SYS_SUBMAC_PER_XQMAC_NUM      4
#define SYS_SGMAC_PER_SLICE_NUM       (SYS_XQMAC_PER_SLICE_NUM * SYS_SUBMAC_PER_XQMAC_NUM)
#define SYS_CGMAC_PER_SLICE_NUM       2

typedef int32 (* CTC_DUMP_MASTER_FUNC )(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param);

struct sys_chip_cut_through_speed_s
{
  uint8 port_speed;
  uint8 enable;
};
typedef struct sys_chip_cut_through_speed_s sys_chip_cut_through_speed_t;

struct sys_chip_master_s
{
    sal_mutex_t* p_chip_mutex;
    uint8   resv;
    uint8   rchain_en;
    uint8   rchain_gchip;
    uint8   g_chip_id;
    mac_addr_t cpu_mac_sa;
    mac_addr_t cpu_mac_da[SYS_USW_MAX_CPU_MACDA_NUM];
    sys_chip_cut_through_speed_t cut_through_speed[CTC_PORT_SPEED_MAX];

    uint8   cut_through_en;

    uint8   tpid;
    uint8   cpu_eth_en;
    uint16  cpu_channel;
    uint16  vlan_id;
    uint64  alpm_affinity_mask;                  /**< [TM] Set affinity mask for alpm task */
    uint64  normal_affinity_mask;                /**< [TM] Set affinity mask for normal task such as DMA,Stats... */
    CTC_DUMP_MASTER_FUNC dump_cb[CTC_FEATURE_MAX];
};
typedef struct sys_chip_master_s sys_chip_master_t;

enum sys_chip_device_id_type_e
{
    SYS_CHIP_DEVICE_ID_GG_CTC8096,
    SYS_CHIP_DEVICE_ID_USW_CTC7148,
    SYS_CHIP_DEVICE_ID_USW_CTC7132,
    SYS_CHIP_DEVICE_ID_INVALID,
};
typedef enum sys_chip_device_id_type_e sys_chip_device_id_type_t;

/****************************************************************************
 *
* Function
*
*****************************************************************************/
extern int32
sys_usw_chip_init(uint8 lchip, uint8 lchip_num);

extern uint8
sys_usw_get_local_chip_num();

extern int32
sys_usw_set_gchip_id(uint8 lchip, uint8 gchip_id);

extern bool
sys_usw_chip_is_local(uint8 lchip, uint8 gchip_id);

extern int32
sys_usw_get_gchip_id(uint8 lchip, uint8* gchip_id);

extern int32
sys_usw_set_chip_global_cfg(uint8 lchip, ctc_chip_global_cfg_t* chip_cfg);

extern int32
sys_usw_get_chip_clock(uint8 lchip, uint16* freq);

extern int32
sys_usw_get_chip_cpumac(uint8 lchip, uint8* mac_sa, uint8* mac_da);

extern int32
sys_usw_get_chip_cpu_eth_en(uint8 lchip, uint8 *enable, uint8* cpu_eth_chan);

extern int32
sys_usw_chip_set_eth_cpu_cfg(uint8 lchip);

extern bool
sys_usw_chip_is_eth_cpu_port(uint8 lchip, uint16 lport);

extern int32
sys_usw_chip_set_access_type(uint8 lchip, ctc_chip_access_type_t access_type);

extern int32
sys_usw_chip_get_access_type(uint8 lchip, ctc_chip_access_type_t* p_access_type);

/* return value units: MHz*/
extern uint16
sys_usw_get_core_freq(uint8 lchip, uint8 type);

extern uint8
sys_usw_chip_get_cut_through_en(uint8 lchip);

extern uint8
sys_usw_chip_get_cut_through_speed(uint8 lchip, uint32 gport);

extern int32
sys_usw_chip_ecc_recover_init(uint8 lchip);

extern int32
sys_usw_chip_show_ecc_recover_status(uint8 lchip);

extern int32
sys_usw_chip_get_device_info(uint8 lchip, ctc_chip_device_info_t* p_device_info);

extern int32
sys_usw_chip_get_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value);

/**
 Configure the tcam scan burst interval time and burst entry num.
 When burst_interval = 0, means scan all tcam without interval, otherwise burst_entry_num must > 0.
 */
extern int32
sys_usw_chip_set_tcam_scan_cfg(uint8 lchip, uint32 burst_entry_num, uint32 burst_interval);

extern int32
sys_usw_get_local_chip_id(uint8 gchip_id, uint8* lchip_id);

extern int32
sys_usw_chip_wb_sync(uint8 lchip);

extern int32
sys_usw_chip_deinit(uint8 lchip);

extern int32
sys_usw_chip_get_ecc_info(uint8 lchip , uint8* p_tcam_scan_en, uint8* p_ecc_recover_en);

extern uint64
sys_usw_chip_get_affinity(uint8 lchip, uint8 is_alpm);

extern int32
sys_usw_chip_get_reset_hw_en(uint8 lchip);

extern uint8
sys_usw_chip_get_rchain_en();

extern uint8
sys_usw_chip_get_rchain_gchip();
#ifdef __cplusplus
}
#endif

#endif

