/**
 @file sys_usw_port.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2011-11-22

 @version v2.0

*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_interrupt.h"
#include "ctc_warmboot.h"

#include "sys_usw_common.h"
#include "sys_usw_port.h"
#include "sys_usw_chip.h"
#include "sys_usw_l3if.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_vlan.h"
#include "sys_usw_vlan_mapping.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_internal_port.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_chip.h"
 /*#include "sys_usw_stats_api.h"*/
#include "sys_usw_l2_fdb.h"
#include "sys_usw_ftm.h"
#include "sys_usw_register.h"
 /*#include "sys_usw_interrupt.h"*/
#include "sys_usw_dmps.h"
#include "sys_usw_mac.h"
#include "sys_usw_peri.h"
#include "sys_usw_wb_common.h"

#include "drv_api.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
#define SYS_MAX_FRAME_VALUE (16 * 1024)

#define SYS_SGMAC_RESET_INT_STEP   (ResetIntRelated_ResetSgmac1_f - ResetIntRelated_ResetSgmac0_f)

#define SYS_SGMAC_EN_PCS_CLK_STEP  (ModuleGatedClkCtl_EnClkSupSgmac1Pcs_f - ModuleGatedClkCtl_EnClkSupSgmac0Pcs_f)

#define SYS_SGMAC_XGMAC_PCS_CLK_STEP  (ModuleGatedClkCtl_EnClkSupSgmac5XgmacPcs_f - ModuleGatedClkCtl_EnClkSupSgmac4XgmacPcs_f)

#define SYS_GMAC_RESET_INT_STEP  (ResetIntRelated_ResetQuadMac1Reg_f - ResetIntRelated_ResetQuadMac0Reg_f)

#define GMAC_FLOW_CTL_REG_ID_INTERVAL 20
#define SGMAC_FLOW_CTL_REG_ID_INTERVAL 42
#define XGMAC_FLOW_CTL_REG_ID_INTERVAL 36


#define VLAN_RANGE_ENTRY_NUM 64


#define SYS_MLAG_ISOLATION_RESERVE_PORT_MASK 0xffffff00


#define SYS_VLAN_RANGE_INFO_CHECK(vrange_info) \
    { \
        CTC_BOTH_DIRECTION_CHECK(vrange_info->direction); \
        if (VLAN_RANGE_ENTRY_NUM <= vrange_info->vrange_grpid) \
        {    return CTC_E_INVALID_PARAM; } \
    }

#define PORT_DB_SET(old, cur) \
    do \
    { \
        if (cur == old) \
        { \
            PORT_UNLOCK; \
            return CTC_E_NONE; \
        } \
        else \
        { \
            old = cur; \
        } \
    } while (0)

#define PORT_DB_GET(cur, old) \
    do \
    { \
        *cur = old; \
    } while (0)


#define SYS_USW_PORT_INIT_CHECK() \
    do { \
        SYS_LCHIP_CHECK_ACTIVE(lchip); \
        if (NULL == p_usw_port_master[lchip]){ \
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
			return CTC_E_NOT_INIT;\
 } \
    } while (0)

#define SYS_LINKAGG_MC_MAP_PBM_PORTID(lport, port_type, port_id)\
    do{\
      port_type = (lport >> 7)&0x1;\
      port_id   = (lport & 0x7F);\
    }while(0)

enum sys_port_egress_restriction_type_e
{
    SYS_PORT_RESTRICTION_NONE = 0,                    /**< restriction disable */
    SYS_PORT_RESTRICTION_PVLAN = 1,                   /**< private vlan enable */
    SYS_PORT_RESTRICTION_BLOCKING = 2,                /**< port blocking enable */
    SYS_PORT_RESTRICTION_ISOLATION = 4                /**< port isolation enable */
};
typedef enum sys_port_egress_restriction_type_e sys_port_egress_restriction_type_t;


enum sys_scl_key_mapping_type_e
{
    SYS_PORT_KEY_MAPPING_TYPE_DRV2CTC,
    SYS_PORT_KEY_MAPPING_TYPE_CTC2DRV,
    SYS_PORT_KEY_MAPPING_TYPE_NUM
};
typedef enum sys_scl_key_mapping_type_e sys_scl_key_mapping_type_t;

enum sys_port_preamble_type_e
{
    SYS_PORT_PREAMBLE_TYPE_8BYTE,
    SYS_PORT_PREAMBLE_TYPE_4BYTE,
    SYS_PORT_PREAMBLE_TYPE_NUM
};
typedef enum sys_port_preamble_type_e sys_port_preamble_type_t;

struct sys_scl_type_map_s
{
    uint8 hash_en;
    uint8 use_macda;
    uint8 vlan_high_priority;
    uint8 drv_type;
    uint8 ctc_type;
};
typedef struct sys_scl_type_map_s sys_scl_type_map_t;

struct sys_scl_type_para_s
{
    uint8 tunnel_en        :1;
    uint8 auto_tunnel      :1;
    uint8 nvgre_en         :1;
    uint8 vxlan_en         :1;
    uint8 tunnel_rpf_check :1;
    uint8 use_macda        :1;
    uint8 tcam_en          :1;
    uint8 vlan_high_priority              :1;
    uint8 drv_hash_type;
    uint8 ctc_hash_type;
    uint8 drv_tcam_type;
    uint8 ctc_tcam_type;
};
typedef struct sys_scl_type_para_s sys_scl_type_para_t;

typedef struct
{
    uint32 hash_type;
    uint32 tcam_type;
    uint32 tcam_en;
    uint32 vlan_high_priority;
    uint8  use_label;
    uint8  label;
    uint8  is_userid;
    uint8  use_macda;
    uint8  ipv4_tunnel_en;
    uint8  ipv4_gre_tunnel_en;
    uint8  rpf_check_en;
    uint8  auto_tunnel;
    uint8  nvgre_en;
    uint8  v6_nvgre_en;
    uint8  v4_vxlan_en;
    uint8  v6_vxlan_en;
} sys_usw_igs_port_scl_map_t;

typedef struct
{
    uint32 hash_type;
    uint8  use_label;
    uint8  label;
} sys_usw_egs_port_scl_map_t;


/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
sys_port_master_t* p_usw_port_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#define TCAML2KEY       0
#define TCAML3KEY       1
#define TCAML2L3KEY     2
#define TCAMUSERIDKEY   3
#define TCAMUDFSHORTKEY 4
#define TCAMUDFLONGKEY   5
#define TCAMDISABLE     6

/*MCHIP_CAP(SYS_CAP_SCL_LABEL_FOR_VLAN_CLASS) MCHIP_CAP(SYS_CAP_SCL_LABEL_FOR_IPSG)*/
extern int32
sys_usw_ip_source_guard_remove_default_entry(uint8 lchip, uint32 gport, uint8 scl_id);
extern int32
sys_usw_ip_source_guard_add_default_entry(uint8 lchip, uint32 gport, uint8 scl_id);

STATIC int32
_sys_usw_port_set_wlan_port_type(uint8 lchip, uint32 gport, uint32 value);
STATIC int32
_sys_usw_port_set_wlan_port_route(uint8 lchip, uint32 gport, uint32 value);

/****************************************************************************
 *
* Function
*
*****************************************************************************/

STATIC int32 _sys_usw_port_set_sd_action(uint8 lchip, ctc_direction_t dir, uint32 index, uint32 act_type)
{
    uint32 cmd = 0;
    uint32 value = 0;

    if (DRV_IS_DUET2(lchip))
    {
        switch(act_type)
        {
            case CTC_PORT_SD_ACTION_TYPE_NONE:
                value = 0;
                break;
            case CTC_PORT_SD_ACTION_TYPE_DISCARD_OAM:
                value = 5;
                break;
            case CTC_PORT_SD_ACTION_TYPE_DISCARD_DATA:
                value = 6;
                break;
            case CTC_PORT_SD_ACTION_TYPE_DISCARD_BOTH:
                value = 7;
                break;
            default :
                return CTC_E_INVALID_PARAM;
        }
        if ((CTC_INGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
        {
            cmd = DRV_IOW(DsPhyPort_t, DsPhyPort_pbbPortType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
        }
        if ((CTC_EGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
        {
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_pbbPortType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
        }
    }
    else
    {
        switch(act_type)
        {
            case CTC_PORT_SD_ACTION_TYPE_NONE:
                value = 0;
                break;
            case CTC_PORT_SD_ACTION_TYPE_DISCARD_OAM:
                value = 1;
                break;
            case CTC_PORT_SD_ACTION_TYPE_DISCARD_DATA:
                value = 2;
                break;
            case CTC_PORT_SD_ACTION_TYPE_DISCARD_BOTH:
                value = 3;
                break;
            default :
                return CTC_E_INVALID_PARAM;
        }
        if ((CTC_INGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
        {
            cmd = DRV_IOW(DsPhyPort_t, DsPhyPort_sdChkType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
        }
        if ((CTC_EGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
        {
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_sdChkType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
        }
    }
    return CTC_E_NONE;
}

STATIC int32 _sys_usw_port_get_sd_action(uint8 lchip, ctc_direction_t dir, uint32 index, uint32* act_type)
{
    uint32 cmd = 0;
    uint32 value = 0;
    if (DRV_IS_DUET2(lchip))
    {
        if (CTC_INGRESS == dir)
        {
            cmd = DRV_IOR(DsPhyPort_t, DsPhyPort_pbbPortType_f);
        }
        else if (CTC_EGRESS == dir)
        {
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_pbbPortType_f);
        }
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
        switch(value)
        {
            case 5:
                *act_type = CTC_PORT_SD_ACTION_TYPE_DISCARD_OAM;
                break;
            case 6:
                *act_type = CTC_PORT_SD_ACTION_TYPE_DISCARD_DATA;
                break;
            case 7:
                *act_type = CTC_PORT_SD_ACTION_TYPE_DISCARD_BOTH;
                break;
            default :
                *act_type = CTC_PORT_SD_ACTION_TYPE_NONE;
                break;
        }
    }
    else
    {
        if (CTC_INGRESS == dir)
        {
            cmd = DRV_IOR(DsPhyPort_t, DsPhyPort_sdChkType_f);
        }
        else if (CTC_EGRESS == dir)
        {
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_sdChkType_f);
        }
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
        switch(value)
        {
            case 1:
                *act_type = CTC_PORT_SD_ACTION_TYPE_DISCARD_OAM;
                break;
            case 2:
                *act_type = CTC_PORT_SD_ACTION_TYPE_DISCARD_DATA;
                break;
            case 3:
                *act_type = CTC_PORT_SD_ACTION_TYPE_DISCARD_BOTH;
                break;
            default :
                *act_type = CTC_PORT_SD_ACTION_TYPE_NONE;
                break;
        }
    }
    return CTC_E_NONE;
}
STATIC uint8 _sys_usw_port_is_support_glb_port(uint8 lchip)
{
    uint32 cmd = DRV_IOR(IpeHeaderAdjustCtl_t, IpeHeaderAdjustCtl_cFlexPacketTypeMode_f);
    uint32 field_val = 0;

    DRV_IOCTL(lchip, 0, cmd, &field_val);
    if(field_val)
    {
        return 1;
    }

    return 0;

}
int32
_sys_usw_port_map_qos_policy(uint8 lchip, uint32 ctc_qos_trust, uint16 lport)
{
    uint32  field = 0;
    uint32  cmd = 0;

    switch (ctc_qos_trust)
    {
        case CTC_QOS_TRUST_PORT:
            field = 0;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_trustOuterPrio_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));

            field = 1;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_trustPortPcp_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
            break;

        case CTC_QOS_TRUST_OUTER:
            field = 1;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_trustOuterPrio_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
            break;

        case CTC_QOS_TRUST_DSCP:
            field = 0;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_trustOuterPrio_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));

            field = 0;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_trustPortPcp_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));

            field = 1;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_trustDscp_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));

            break;

        case CTC_QOS_TRUST_STAG_COS:
            field = 0;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_trustOuterPrio_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_trustPortPcp_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_trustDscp_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_cosPhbUseInner_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));

            break;

        case CTC_QOS_TRUST_CTAG_COS:
            field = 0;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_trustOuterPrio_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_trustPortPcp_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_trustDscp_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
            field = 1;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_cosPhbUseInner_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));

            break;
        case CTC_QOS_TRUST_COS:
            field = 0;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_trustOuterPrio_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_trustPortPcp_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_trustDscp_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_cosPhbUseInner_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));

            break;
        case CTC_QOS_TRUST_IP:
            return CTC_E_NOT_SUPPORT;
        case CTC_QOS_TRUST_MAX:
            return CTC_E_INVALID_PARAM;

        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_port_unmap_qos_policy(uint8 lchip, uint16 lport, uint32* ctc_qos_trust)
{
    uint32  cmd = 0;
    DsSrcPort_m  ds_src_port;

    sal_memset(&ds_src_port, 0, sizeof(ds_src_port));

    cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_src_port));

    if(GetDsSrcPort(V, trustOuterPrio_f, &ds_src_port))
    {
        *ctc_qos_trust = CTC_QOS_TRUST_OUTER;
    }
    else if(GetDsSrcPort(V, trustPortPcp_f, &ds_src_port))
    {
        *ctc_qos_trust = CTC_QOS_TRUST_PORT;
    }
    else if(GetDsSrcPort(V, trustDscp_f, &ds_src_port))
    {
        *ctc_qos_trust = CTC_QOS_TRUST_DSCP;
    }
    else if(0 == GetDsSrcPort(V, cosPhbUseInner_f, &ds_src_port))
    {
        *ctc_qos_trust = CTC_QOS_TRUST_STAG_COS;
    }
    else if(1 == GetDsSrcPort(V, cosPhbUseInner_f, &ds_src_port))
    {
        *ctc_qos_trust = CTC_QOS_TRUST_CTAG_COS;
    }
    else
    {
        *ctc_qos_trust = CTC_QOS_TRUST_COS;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_port_set_port_blocking(uint8 lchip, uint32 gport, ctc_port_blocking_pkt_type_t type)
{
    uint16 lport = 0;
    uint32 value = 0;
    uint32 cmd = 0;
    DsDestPort_m ds_dest_port;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    sal_memset(&ds_dest_port, 0, sizeof(DsDestPort_m));
    cmd = DRV_IOR(DsDestPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_dest_port));

    value = CTC_FLAG_ISSET(type, CTC_PORT_BLOCKING_UNKNOW_UCAST) ? 1 : 0;
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "unknown ucast flooding = %d\n", value);
    SetDsDestPort(V, ucastFloodingDisable_f, &ds_dest_port, value);

    value = CTC_FLAG_ISSET(type, CTC_PORT_BLOCKING_UNKNOW_MCAST) ? 1 : 0;
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "unknown mcast flooding = %d\n", value);
    SetDsDestPort(V, mcastFloodingDisable_f, &ds_dest_port, value);

    value = CTC_FLAG_ISSET(type, CTC_PORT_BLOCKING_KNOW_UCAST) ? 1 : 0;
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "known ucast flooding = %d\n", value);
    SetDsDestPort(V, knownUcastFloodingDisable_f, &ds_dest_port, value);

    value = CTC_FLAG_ISSET(type, CTC_PORT_BLOCKING_KNOW_MCAST) ? 1 : 0;
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "known mcast flooding = %d\n", value);
    SetDsDestPort(V, knownMcastFloodingDisable_f, &ds_dest_port, value);

    value = CTC_FLAG_ISSET(type, CTC_PORT_BLOCKING_BCAST) ? 1 : 0;
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "bcast flooding = %d\n", value);
    SetDsDestPort(V, bcastFloodingDisable_f, &ds_dest_port, value);

    cmd = DRV_IOW(DsDestPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_dest_port));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_port_get_port_blocking(uint8 lchip, uint32 gport, uint16* p_type)
{
    uint16 lport = 0;
    uint32 value = 0;
    uint32 cmd = 0;
    DsDestPort_m ds_dest_port;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    sal_memset(&ds_dest_port, 0, sizeof(DsDestPort_m));
    cmd = DRV_IOR(DsDestPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_dest_port));

    GetDsDestPort(A, ucastFloodingDisable_f, &ds_dest_port, &value);
    if (value)
    {
        CTC_SET_FLAG(*p_type, CTC_PORT_BLOCKING_UNKNOW_UCAST);
    }

    GetDsDestPort(A, mcastFloodingDisable_f, &ds_dest_port, &value);
    if (value)
    {
        CTC_SET_FLAG(*p_type, CTC_PORT_BLOCKING_UNKNOW_MCAST);
    }

    GetDsDestPort(A, knownUcastFloodingDisable_f, &ds_dest_port, &value);
    if (value)
    {
        CTC_SET_FLAG(*p_type, CTC_PORT_BLOCKING_KNOW_UCAST);
    }

    GetDsDestPort(A, knownMcastFloodingDisable_f, &ds_dest_port, &value);
    if (value)
    {
        CTC_SET_FLAG(*p_type, CTC_PORT_BLOCKING_KNOW_MCAST);
    }

    GetDsDestPort(A, bcastFloodingDisable_f, &ds_dest_port, &value);
    if (value)
    {
        CTC_SET_FLAG(*p_type, CTC_PORT_BLOCKING_BCAST);
    }

    return CTC_E_NONE;
}

int32
sys_usw_port_dump_db(uint8 lchip, sal_file_t dump_db_fp,ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;
    uint32 lport = 0;

    SYS_PORT_INIT_CHECK();
    PORT_LOCK;

    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "# Port");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "{");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "igs-lport lbk_en    subif_en  rsv       rsv0      inter_lport global_src_port  flag   flag_ext  isolation_id   nhid\n");

    for (lport =0; lport <SYS_USW_MAX_PORT_NUM_PER_CHIP; lport++)
    {
        SYS_DUMP_DB_LOG(dump_db_fp, "%-10d%-10d%-10d%-10d%-10d%-12d%-17d%-7d%-10d%-15d%-10u\n",\
        lport,  p_usw_port_master[lchip]->igs_port_prop[lport].lbk_en,\
        p_usw_port_master[lchip]->igs_port_prop[lport].subif_en,\
        p_usw_port_master[lchip]->igs_port_prop[lport].rsv,\
        p_usw_port_master[lchip]->igs_port_prop[lport].rsv0,\
        p_usw_port_master[lchip]->igs_port_prop[lport].inter_lport,\
        p_usw_port_master[lchip]->igs_port_prop[lport].global_src_port,\
        p_usw_port_master[lchip]->igs_port_prop[lport].flag,\
        p_usw_port_master[lchip]->igs_port_prop[lport].flag_ext,\
        p_usw_port_master[lchip]->igs_port_prop[lport].isolation_id,\
        p_usw_port_master[lchip]->igs_port_prop[lport].nhid);

    }

    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "egs-lport  flag      flag_ext  isolation_id  isolation_ref_cnt\n");
    for (lport =0; lport <SYS_USW_MAX_PORT_NUM_PER_CHIP; lport++)
    {
        SYS_DUMP_DB_LOG(dump_db_fp, "%-11d%-10d%-10d%-14d%-17d\n",\
        lport, p_usw_port_master[lchip]->egs_port_prop[lport].flag,\
        p_usw_port_master[lchip]->egs_port_prop[lport].flag_ext,\
        p_usw_port_master[lchip]->egs_port_prop[lport].isolation_id,\
        p_usw_port_master[lchip]->egs_port_prop[lport].isolation_ref_cnt);
    }

    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "------------------------------------------------------\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","cur_log_index",p_usw_port_master[lchip]->cur_log_index);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","use_logic_port_check",p_usw_port_master[lchip]->use_logic_port_check);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","use_isolation_id",p_usw_port_master[lchip]->use_isolation_id);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","isolation_group_mode",p_usw_port_master[lchip]->isolation_group_mode);

    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","mac_bmp[0]",p_usw_port_master[lchip]->mac_bmp[0]);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","mac_bmp[1]",p_usw_port_master[lchip]->mac_bmp[1]);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","mac_bmp[2]",p_usw_port_master[lchip]->mac_bmp[2]);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","mac_bmp[3]",p_usw_port_master[lchip]->mac_bmp[3]);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","igs_isloation_bitmap[0]",p_usw_port_master[lchip]->igs_isloation_bitmap[0]);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","igs_isloation_bitmap[1]",p_usw_port_master[lchip]->igs_isloation_bitmap[1]);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","opf_type_port_sflow",p_usw_port_master[lchip]->opf_type_port_sflow);
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "------------------------------------------------------\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "}");
    PORT_UNLOCK;

    return ret;
}

#if 0
STATIC int32
_sys_usw_framesize_init(uint8 lchip)
{
    uint8  idx = 0;

    for (idx = 0; idx < CTC_FRAME_SIZE_MAX; idx++)
    {
        CTC_ERROR_RETURN(sys_usw_set_min_frame_size(lchip, idx, SYS_USW_MIN_FRAMESIZE_DEFAULT_VALUE));
        CTC_ERROR_RETURN(sys_usw_set_max_frame_size(lchip, idx, SYS_USW_MAX_FRAMESIZE_DEFAULT_VALUE));
    }

    return CTC_E_NONE;
}
#endif

STATIC int32
_sys_usw_port_init_opf(uint8 lchip)
{
    sys_usw_opf_t       opf;
    uint8       opf_type_port_sflow = 0;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &opf_type_port_sflow, CTC_BOTH_DIRECTION, "opf-type-port-sflow"));
    p_usw_port_master[lchip]->opf_type_port_sflow = opf_type_port_sflow;
    /*seed id ingress*/
    opf.pool_index = CTC_INGRESS;
    opf.pool_type  = opf_type_port_sflow;
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 1, 16));
    /*seed id egress*/
    opf.pool_index = CTC_EGRESS;
    opf.pool_type  = opf_type_port_sflow;
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 1, 16));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_port_set_seed_id(uint8 lchip, uint16 lport, ctc_direction_t dir, uint8 enable)
{
    sys_usw_opf_t opf;
    uint32      seed_id = 0;
    uint32      tbl_id[CTC_BOTH_DIRECTION] = {IpeRandomSeedMap_t, EpeRandomSeedMap_t};
    uint32      fld_id[CTC_BOTH_DIRECTION] = {IpeRandomSeedMap_seedId_f, EpeRandomSeedMap_seedId_f};
    uint32      cmd = 0;
    uint8       index = lport & 0x3F;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = dir;
    opf.pool_type = p_usw_port_master[lchip]->opf_type_port_sflow;
    if (enable)
    {
        int32 ret = sys_usw_opf_alloc_offset(lchip, &opf, 1, &seed_id);
        if(ret && CTC_E_NO_RESOURCE != ret)
        {
            return ret;
        }
        seed_id = (ret==CTC_E_NO_RESOURCE)?0:seed_id;
    }
    else
    {
        cmd = DRV_IOR(tbl_id[dir], fld_id[dir]);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &seed_id));
        if(seed_id)
        {
            sys_usw_opf_free_offset(lchip, &opf, 1, seed_id);
        }
        seed_id = 0;
    }

    cmd = DRV_IOW(tbl_id[dir], fld_id[dir]);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &seed_id));

    return CTC_E_NONE;
}

int32
sys_usw_port_glb_dest_port_init(uint8 lchip, uint16 glb_port_index)
{
    DsGlbDestPort_m ds_glb_dest_port;
    uint32 cmd = 0;

    sal_memset(&ds_glb_dest_port, 0, sizeof(DsGlbDestPort_m));

    SetDsGlbDestPort(V, defaultVlanId_f, &ds_glb_dest_port, 1);
    SetDsGlbDestPort(V, untagDefaultSvlan_f, &ds_glb_dest_port, 1);
    SetDsGlbDestPort(V, untagDefaultVlanId_f, &ds_glb_dest_port, 1);
    SetDsGlbDestPort(V, bridgeEn_f, &ds_glb_dest_port, 1);
    SetDsGlbDestPort(V, dot1QEn_f, &ds_glb_dest_port, CTC_DOT1Q_TYPE_BOTH);
    SetDsGlbDestPort(V, svlanTpidValid_f, &ds_glb_dest_port, 0);
    SetDsGlbDestPort(V, replaceStagTpid_f, &ds_glb_dest_port, 0);
    SetDsGlbDestPort(V, stagOperationDisable_f, &ds_glb_dest_port, 0);

    cmd = DRV_IOW(DsGlbDestPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, glb_port_index, cmd, &ds_glb_dest_port));


    return CTC_E_NONE;
}

/*NOTE: set global dest port field & clear dest port filed*/
int32 _sys_usw_port_set_glb_dest_port_by_dest_port(uint8 lchip, uint16 lport)
{
    uint32 cmd = DRV_IOR(DsDestPort_t, DRV_ENTRY_FLAG);
    DsGlbDestPort_m ds_glb_dest_port;
    DsDestPort_m ds_dest_port;

    sal_memset(&ds_dest_port, 0, sizeof(ds_dest_port));
    sal_memset(&ds_glb_dest_port, 0, sizeof(ds_glb_dest_port));

    DRV_IOCTL(lchip, lport, cmd, &ds_dest_port);

    SetDsGlbDestPort(V, bridgeEn_f, &ds_glb_dest_port, GetDsDestPort(V, bridgeEn_f, &ds_dest_port));
    SetDsDestPort(V, bridgeEn_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, cosPhbPtr_f, &ds_glb_dest_port, GetDsDestPort(V, cosPhbPtr_f, &ds_dest_port));
    SetDsDestPort(V, cosPhbPtr_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, ctagOperationDisable_f, &ds_glb_dest_port, GetDsDestPort(V, ctagOperationDisable_f, &ds_dest_port));
    SetDsDestPort(V, ctagOperationDisable_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, cvlanSpace_f, &ds_glb_dest_port, GetDsDestPort(V, cvlanSpace_f, &ds_dest_port));
    SetDsDestPort(V, cvlanSpace_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, defaultDei_f, &ds_glb_dest_port, GetDsDestPort(V, defaultDei_f, &ds_dest_port));
    SetDsDestPort(V, defaultDei_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, defaultDscp_f, &ds_glb_dest_port, GetDsDestPort(V, defaultDscp_f, &ds_dest_port));
    SetDsDestPort(V, defaultDscp_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, defaultPcp_f, &ds_glb_dest_port, GetDsDestPort(V, defaultPcp_f, &ds_dest_port));
    SetDsDestPort(V, defaultPcp_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, defaultVlanId_f, &ds_glb_dest_port, GetDsDestPort(V, defaultVlanId_f, &ds_dest_port));
    SetDsDestPort(V, defaultVlanId_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, dot1QEn_f, &ds_glb_dest_port, GetDsDestPort(V, dot1QEn_f, &ds_dest_port));
    SetDsDestPort(V, dot1QEn_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, dscpPhbPtr_f, &ds_glb_dest_port, GetDsDestPort(V, dscpPhbPtr_f, &ds_dest_port));
    SetDsDestPort(V, dscpPhbPtr_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, dscpRemarkMode_f, &ds_glb_dest_port, GetDsDestPort(V, dscpRemarkMode_f, &ds_dest_port));
    SetDsDestPort(V, dscpRemarkMode_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, priorityTagEn_f, &ds_glb_dest_port, GetDsDestPort(V, priorityTagEn_f, &ds_dest_port));
    SetDsDestPort(V, priorityTagEn_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, replaceCtagCos_f, &ds_glb_dest_port, GetDsDestPort(V, replaceCtagCos_f, &ds_dest_port));
    SetDsDestPort(V, replaceCtagCos_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, replacePortPcp_f, &ds_glb_dest_port, GetDsDestPort(V, replacePortPcp_f, &ds_dest_port));
    SetDsDestPort(V, replacePortPcp_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, replaceStagCos_f, &ds_glb_dest_port, GetDsDestPort(V, replaceStagCos_f, &ds_dest_port));
    SetDsDestPort(V, replaceStagCos_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, replaceStagTpid_f, &ds_glb_dest_port, GetDsDestPort(V, replaceStagTpid_f, &ds_dest_port));
    SetDsDestPort(V, replaceStagTpid_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, stagOperationDisable_f, &ds_glb_dest_port, GetDsDestPort(V, stagOperationDisable_f, &ds_dest_port));
    SetDsDestPort(V, stagOperationDisable_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, svlanTpidIndex_f, &ds_glb_dest_port, GetDsDestPort(V, svlanTpidIndex_f, &ds_dest_port));
    SetDsDestPort(V, svlanTpidIndex_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, svlanTpidValid_f, &ds_glb_dest_port, GetDsDestPort(V, svlanTpidValid_f, &ds_dest_port));
    SetDsDestPort(V, svlanTpidValid_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, untagDefaultSvlan_f, &ds_glb_dest_port, GetDsDestPort(V, untagDefaultSvlan_f, &ds_dest_port));
    SetDsDestPort(V, untagDefaultSvlan_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, untagDefaultVlanId_f, &ds_glb_dest_port, GetDsDestPort(V, untagDefaultVlanId_f, &ds_dest_port));
    SetDsDestPort(V, untagDefaultVlanId_f, &ds_dest_port, 0);

    SetDsGlbDestPort(V, useDecapVlanCos_f, &ds_glb_dest_port, GetDsDestPort(V, useDecapVlanCos_f, &ds_dest_port));
    SetDsDestPort(V, useDecapVlanCos_f, &ds_dest_port, 0);

    cmd = DRV_IOW(DsDestPort_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, lport, cmd, &ds_dest_port);
    cmd = DRV_IOW(DsGlbDestPort_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, lport, cmd, &ds_glb_dest_port);

    return CTC_E_NONE;
}
int32
_sys_usw_port_set_default(uint8 lchip, uint16 lport)
{
    DsSrcPort_m ds_src_port;
    DsPhyPort_m ds_phy_port;
    DsDestPort_m ds_dest_port;
    DsGlbDestPort_m ds_glb_dest_port;
    DsPhyPortExt_m ds_phy_port_ext;
    uint32 cmd = 0;
    uint32 gport = 0;
    uint32 val = 0;
    uint8  gchip = 0;
    uint8  support_glb_port = _sys_usw_port_is_support_glb_port(lchip);

    sal_memset(&ds_phy_port, 0, sizeof(DsPhyPort_m));
    sal_memset(&ds_phy_port_ext, 0, sizeof(DsPhyPortExt_m));
    sal_memset(&ds_src_port, 0, sizeof(DsSrcPort_m));
    sal_memset(&ds_dest_port, 0, sizeof(DsDestPort_m));
    sal_memset(&ds_glb_dest_port, 0, sizeof(ds_glb_dest_port));

    sys_usw_get_gchip_id(lchip, &gchip);
    gport = CTC_MAP_LPORT_TO_GPORT(gchip, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(lport));

    _sys_usw_port_set_wlan_port_route(lchip, gport, 0);
    _sys_usw_port_set_wlan_port_type(lchip, gport, CTC_PORT_WLAN_PORT_TYPE_NONE);
    _sys_usw_port_map_qos_policy(lchip, CTC_QOS_TRUST_COS, lport);

    SetDsPhyPort(V, outerVlanIsCVlan_f, &ds_phy_port, 0);
    SetDsPhyPort(V, packetTypeValid_f, &ds_phy_port, 0);
    SetDsPhyPort(V, fwdHashGenDis_f, &ds_phy_port, 0);

    SetDsPhyPortExt(V, srcOuterVlanIsSvlan_f, &ds_phy_port_ext, 1);
    SetDsPhyPortExt(V, defaultVlanId_f, &ds_phy_port_ext, 1);
    SetDsPhyPortExt(V, useDefaultVlanLookup_f, &ds_phy_port_ext, 1);
    SetDsPhyPortExt(V, exception2Discard_f, &ds_phy_port_ext, 1);
    SetDsPhyPortExt(V, exception2En_f, &ds_phy_port_ext, 1);
    SetDsPhyPortExt(V, hash1Priority_f, &ds_phy_port_ext, 0);
    SetDsPhyPortExt(V, tcam1Priority_f, &ds_phy_port_ext, 0);
    SetDsPhyPortExt(V, hash2Priority_f, &ds_phy_port_ext, 1);
    SetDsPhyPortExt(V, tcam2Priority_f, &ds_phy_port_ext, 1);
    SetDsPhyPortExt(V, hashDefaultEntryValid_f, &ds_phy_port_ext, 1);
    /* SetDsPhyPortExt(V, qosPolicy_f, &ds_phy_port_ext, 1); //default trust port*/
    SetDsPhyPortExt(V, userIdTcamCustomerIdEn_f, &ds_phy_port_ext, 1);
    if(DRV_IS_DUET2(lchip))
    {
        SetDsPhyPortExt(V, userIdTcam1En_f, &ds_phy_port_ext, 1);
        SetDsPhyPortExt(V, userIdTcam1Type_f, &ds_phy_port_ext, TCAMUSERIDKEY);
        SetDsPhyPortExt(V, userIdTcam2En_f, &ds_phy_port_ext, 1);
        SetDsPhyPortExt(V, userIdTcam2Type_f, &ds_phy_port_ext, TCAMUSERIDKEY);
    }
    SetDsSrcPort(V, receiveDisable_f, &ds_src_port, 0);
    SetDsSrcPort(V, bridgeEn_f, &ds_src_port, 1);
    /*SYSTEM MODIFY, Added by weij for update SDK, 2017-09-11*/
    /*SYSTEM MODIFIED by xgu for bug 28025, DsSrcPort_portSecurityEn_f will be always enabled for srcMismatch discard, 2014-10-29*/
    SetDsSrcPort(V, portSecurityEn_f, &ds_src_port, 1);
    SetDsSrcPort(V, routeDisable_f, &ds_src_port, 0);
    /* depend on vlan ingress filter */
    SetDsSrcPort(V, ingressFilteringMode_f, &ds_src_port, SYS_PORT_VLAN_FILTER_MODE_VLAN);
    SetDsSrcPort(V, fastLearningEn_f, &ds_src_port, 1);
    SetDsSrcPort(V, discardAllowLearning_f, &ds_src_port, 0);
    SetDsSrcPort(V, forceSecondParser_f, &ds_src_port, 0);
    SetDsSrcPort(V, trustPortPcp_f, &ds_src_port, 0);
    SetDsSrcPort(V, isAcOamUseDataVlan_f, &ds_src_port, 1);
    SetDsSrcPort(V, portBasedHashProfile0Valid_f, &ds_src_port, 1);
    SetDsSrcPort(V, portBasedHashProfile1Valid_f, &ds_src_port, 0);
    SetDsSrcPort(V, learningDisableMode_f, &ds_src_port, 1);
    SetDsDestPort(V, transmitDisable_f, &ds_dest_port, 0);
    SetDsDestPort(V, stpCheckEn_f, &ds_dest_port, 1);
    SetDsDestPort(V, sourcePortToHeader_f, &ds_dest_port, 0);/*SYSTEM MODIFIED by yejl, 2019-04-02*/
    if(support_glb_port)
    {
        SetDsGlbDestPort(V, cvlanSpace_f, &ds_glb_dest_port, 0);
    }
    else
    {
        SetDsDestPort(V, cvlanSpace_f, &ds_dest_port, 0);
    }
    SetDsDestPort(V, vlanHash1DefaultEntryValid_f, &ds_dest_port, 1);
    SetDsDestPort(V, vlanHash2DefaultEntryValid_f, &ds_dest_port, 1);
    if(support_glb_port)
    {
        SetDsGlbDestPort(V, defaultVlanId_f, &ds_glb_dest_port, 1);
    }
    else
    {
        SetDsDestPort(V, defaultVlanId_f, &ds_dest_port, 1);
    }
    if(support_glb_port)
    {
        SetDsGlbDestPort(V, untagDefaultSvlan_f, &ds_glb_dest_port, 1);
    }
    else
    {
        SetDsDestPort(V, untagDefaultSvlan_f, &ds_dest_port, 1);
    }
    if(support_glb_port)
    {
        SetDsGlbDestPort(V, untagDefaultVlanId_f, &ds_glb_dest_port, 1);
    }
    else
    {
        SetDsDestPort(V, untagDefaultVlanId_f, &ds_dest_port, 1);
    }
    if(support_glb_port)
    {
        SetDsGlbDestPort(V, bridgeEn_f, &ds_glb_dest_port, 1);
    }
    else
    {
        SetDsDestPort(V, bridgeEn_f, &ds_dest_port, 1);
    }
    if(support_glb_port)
    {
        SetDsGlbDestPort(V, dot1QEn_f, &ds_glb_dest_port, CTC_DOT1Q_TYPE_BOTH);
    }
    else
    {
        SetDsDestPort(V, dot1QEn_f, &ds_dest_port, CTC_DOT1Q_TYPE_BOTH);
    }
    if(support_glb_port)
    {
        SetDsGlbDestPort(V, svlanTpidValid_f, &ds_glb_dest_port, 0);
    }
    else
    {
        SetDsDestPort(V, svlanTpidValid_f, &ds_dest_port, 0);
    }
    if(support_glb_port)
    {
        SetDsGlbDestPort(V, replaceStagTpid_f, &ds_glb_dest_port, 0);
    }
    else
    {
        SetDsDestPort(V, replaceStagTpid_f, &ds_dest_port, 0);
    }
    if(support_glb_port)
    {
        SetDsGlbDestPort(V, stagOperationDisable_f, &ds_glb_dest_port, 0);
    }
    else
    {
        SetDsDestPort(V, stagOperationDisable_f, &ds_dest_port, 0);
    }
    SetDsDestPort(V, isAcOamUseDataVlan_f, &ds_dest_port, 1);
    /*default acl is use gport*/
    SetDsSrcPort(V, gAcl_0_aclUseGlobalPortType_f, &ds_src_port, 1);
    SetDsSrcPort(V, gAcl_1_aclUseGlobalPortType_f, &ds_src_port, 1);
    SetDsSrcPort(V, gAcl_2_aclUseGlobalPortType_f, &ds_src_port, 1);
    SetDsSrcPort(V, gAcl_3_aclUseGlobalPortType_f, &ds_src_port, 1);
    SetDsSrcPort(V, gAcl_4_aclUseGlobalPortType_f, &ds_src_port, 1);
    SetDsSrcPort(V, gAcl_5_aclUseGlobalPortType_f, &ds_src_port, 1);
    SetDsSrcPort(V, gAcl_6_aclUseGlobalPortType_f, &ds_src_port, 1);
    SetDsSrcPort(V, gAcl_7_aclUseGlobalPortType_f, &ds_src_port, 1);

    SetDsDestPort(V, gAcl_0_aclUseGlobalPortType_f, &ds_dest_port, 1);
    SetDsDestPort(V, gAcl_1_aclUseGlobalPortType_f, &ds_dest_port, 1);
    SetDsDestPort(V, gAcl_2_aclUseGlobalPortType_f, &ds_dest_port, 1);

    if (0 == ((lport >> 7) & 0x1)) /* x00xxxxxx*/
    {
        SetDsSrcPort(V, aclPortNum_f, &ds_src_port, lport);
        SetDsDestPort(V, aclPortNum_f, &ds_dest_port, lport);
    }
    cmd = DRV_IOW(DsPhyPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_phy_port));

    cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_phy_port_ext));

    cmd = DRV_IOW(DsSrcPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_src_port));

    cmd = DRV_IOW(DsDestPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_dest_port));

    cmd = DRV_IOW(DsGlbDestPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_glb_dest_port));

    CTC_ERROR_RETURN(_sys_usw_port_set_port_blocking(lchip, gport, 0));

    if ((lport & 0xFF) < 128)
    {
        val = lport & 0x7F ;
        cmd = DRV_IOW(DsPortLinkAgg_t, DsPortLinkAgg_u1_g2_sourceChannel_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &val));
    }

    sys_usw_port_set_global_port(lchip, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(lport), gport, FALSE);
    return CTC_E_NONE;
}

int32
_sys_usw_port_isolation_type_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 index = 0;
    uint32 value = 0;

    /*unknown unicast*/
    value = 0;
    index = 4;          /*L2UC*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = 8;          /*MPLS*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = 9;          /*VPLS*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = 10;          /*L3VPN*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = 11;          /*VPWS*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = 15;          /*MPLS_OTHER_VPN*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = 12;          /*TRILL_UC*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = 14;          /*FCOE*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));

    /*unknown multicast*/
    value = 1;
    index = 5;          /*L2MC*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = 13;          /*TRILL_MC*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));

    /*known unicast*/
    value = 2;
    index = (1 << 4) + 4;          /*L2UC*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = (1 << 4) + 8;          /*MPLS*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = (1 << 4) + 9;          /*VPLS*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = (1 << 4) + 10;          /*L3VPN*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = (1 << 4) + 11;          /*VPWS*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = (1 << 4) + 15;          /*MPLS_OTHER_VPN*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = (1 << 4) + 12;          /*TRILL_UC*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = (1 << 4) + 14;          /*FCOE*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));

    /*known multicast*/
    value = 3;
    index = (1 << 4) + 5;          /*L2MC*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = (1 << 4) + 13;          /*TRILL_MC*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));

    /*bcast*/
    value = 4;
    index = 6;          /*BC*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));

    /*route*/
    value = 5;
    index = 2;          /*IPUC*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = (1 << 4) + 2;          /*IPUC*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = 3;          /*IPMC*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
    index = (1 << 4) + 3;          /*IPMC*/
    cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));

    return CTC_E_NONE;
}

int32
_sys_usw_port_isolation_init(uint8 lchip, ctc_port_global_cfg_t* p_port_global_cfg)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 value2[2] = {0};
    uint32 value3[8] = {0};

    p_usw_port_master[lchip]->isolation_group_mode = p_port_global_cfg->isolation_group_mode;
    value = p_port_global_cfg->isolation_group_mode;
    cmd = DRV_IOW(QWritePortIsolateCtl_t, QWritePortIsolateCtl_portIsolateMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    value = 1;
    cmd = DRV_IOW(QWritePortIsolateCtl_t, QWritePortIsolateCtl_portIsolateFwdTypeEnableBitmap_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(MetFifoIsolateCtl_t, MetFifoIsolateCtl_peerLink1Valid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(MetFifoIsolateCtl_t, MetFifoIsolateCtl_isolationBcastOptimization1_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    cmd = DRV_IOW(MetFifoIsolateCtl_t, MetFifoIsolateCtl_isolateFwdTypeEnableBitmap1_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    sal_memset(value2, 0xFF, sizeof(value2));
    cmd = DRV_IOW(MetFifoIsolateCtl_t, MetFifoIsolateCtl_lagNonBlockBitmap1_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value2));

    sal_memset(value3, 0xFF, sizeof(value3));
    cmd = DRV_IOW(MetFifoIsolateCtl_t, MetFifoIsolateCtl_portNonBlockBitmap1_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value3));

    value = 0x1c;
    cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_bridgeOperationMapping_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    p_usw_port_master[lchip]->use_isolation_id =  p_port_global_cfg->use_isolation_id;

    _sys_usw_port_isolation_type_init(lchip);
    return CTC_E_NONE;

}

/**
 @brief initialize the port module
*/
int32
sys_usw_port_init(uint8 lchip, ctc_port_global_cfg_t* p_port_global_cfg)
{
    int32  ret = CTC_E_NONE;
    uint32 index = 0;
    uint32 cmd = 0;
    MetFifoIsolateCtl_m   met_fifo_ctl;
    uint32  field_v[8] = {0};
    uint8 i = 0;

    if (p_usw_port_master[lchip] != NULL)
    {
        return CTC_E_NONE;
    }


    /*alloc&init DB and mutex*/
    p_usw_port_master[lchip] = (sys_port_master_t*)mem_malloc(MEM_PORT_MODULE, sizeof(sys_port_master_t));

    if (NULL == p_usw_port_master[lchip])
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    sal_memset(p_usw_port_master[lchip], 0, sizeof(sys_port_master_t));

    CTC_ERROR_GOTO(_sys_usw_port_init_opf(lchip), ret, rollback_0);

    ret = sal_mutex_create(&(p_usw_port_master[lchip]->p_port_mutex));

    if (ret || !(p_usw_port_master[lchip]->p_port_mutex))
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Create mutex fail\n");
        goto rollback_1;
    }

    p_usw_port_master[lchip]->igs_port_prop = (sys_igs_port_prop_t*)mem_malloc(MEM_PORT_MODULE, SYS_USW_MAX_PORT_NUM_PER_CHIP * sizeof(sys_igs_port_prop_t));

    if (NULL == p_usw_port_master[lchip]->igs_port_prop)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        goto rollback_2;
    }
    sal_memset(p_usw_port_master[lchip]->igs_port_prop, 0, sizeof(sys_igs_port_prop_t) * SYS_USW_MAX_PORT_NUM_PER_CHIP);

    p_usw_port_master[lchip]->egs_port_prop = (sys_egs_port_prop_t*)mem_malloc(MEM_PORT_MODULE, SYS_USW_MAX_PORT_NUM_PER_CHIP * sizeof(sys_egs_port_prop_t));

    if (NULL == p_usw_port_master[lchip]->egs_port_prop)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        goto rollback_3;
    }
    sal_memset(p_usw_port_master[lchip]->egs_port_prop, 0, sizeof(sys_egs_port_prop_t) * SYS_USW_MAX_PORT_NUM_PER_CHIP);

    p_usw_port_master[lchip]->network_port = (uint16*)mem_malloc(MEM_PORT_MODULE, SYS_USW_MAX_PORT_NUM_PER_CHIP * sizeof(uint16));

    if (NULL == p_usw_port_master[lchip]->network_port)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        goto rollback_4;
    }
    sal_memset(p_usw_port_master[lchip]->network_port, 0xFF, sizeof(uint16) * SYS_USW_MAX_PORT_NUM_PER_CHIP);

    /*init asic table*/
    p_usw_port_master[lchip]->use_logic_port_check = (p_port_global_cfg->default_logic_port_en)? TRUE : FALSE;

    if (CTC_WB_STATUS(lchip) != CTC_WB_STATUS_RELOADING)
    {
        for (index = 0; index < SYS_USW_MAX_PORT_NUM_PER_CHIP; index++)
        {
            CTC_ERROR_GOTO(_sys_usw_port_set_default(lchip, index), ret, rollback_5);
        }
    }

    /*init isolation mode*/
    _sys_usw_port_isolation_init(lchip, p_port_global_cfg);

    /* init port bitmap*/
    cmd = DRV_IOR(MetFifoIsolateCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &met_fifo_ctl), ret, rollback_5);
    SetMetFifoIsolateCtl(V, bitmap0AsLinkStatus_f, &met_fifo_ctl, 1);
    SetMetFifoIsolateCtl(V, isolationBcastOptimization0_f, &met_fifo_ctl, 1);
    sal_memset(field_v, 0xFF, sizeof(field_v));
    SetMetFifoIsolateCtl(A, portNonBlockBitmap0_f, &met_fifo_ctl, field_v);
    cmd = DRV_IOW(MetFifoIsolateCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &met_fifo_ctl), ret, rollback_5);

    /*warm boot*/
    CTC_ERROR_GOTO(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_PORT, sys_usw_port_wb_sync), ret, rollback_5);
    /*dump db*/
    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_PORT, sys_usw_port_dump_db), ret, rollback_6);

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_GOTO(sys_usw_port_wb_restore(lchip), ret, rollback_6);
    }
    if(sys_usw_chip_get_rchain_en() && lchip)
    {
        uint32 gport = 0;
        uint8 gchip = 0;
        sys_usw_get_gchip_id(lchip, &gchip);
        for (i = 0; i < 128; i++)
        {
            gport = CTC_MAP_LPORT_TO_GPORT(gchip, (100 + i));
            sys_usw_port_set_phy_if_en(lchip, gport, 1);
        }
    }
    return CTC_E_NONE;

rollback_6:
    sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_PORT, NULL);
rollback_5:
    mem_free(p_usw_port_master[lchip]->network_port);
rollback_4:
    mem_free(p_usw_port_master[lchip]->egs_port_prop);
rollback_3:
    mem_free(p_usw_port_master[lchip]->igs_port_prop);
rollback_2:
    sal_mutex_destroy(p_usw_port_master[lchip]->p_port_mutex);
rollback_1:
    sys_usw_opf_free(lchip, p_usw_port_master[lchip]->opf_type_port_sflow, CTC_INGRESS);
    sys_usw_opf_free(lchip, p_usw_port_master[lchip]->opf_type_port_sflow, CTC_EGRESS);
    sys_usw_opf_deinit(lchip, p_usw_port_master[lchip]->opf_type_port_sflow);
rollback_0:
    mem_free(p_usw_port_master[lchip]);
    return ret;
}

/**
 @brief set the port default configure
*/
int32
sys_usw_port_set_default_cfg(uint8 lchip, uint32 gport)
{

    int32 ret = CTC_E_NONE;
    uint32 lport = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set gport:0x%04X default configure!\n", gport);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    /*do write table*/
    PORT_LOCK;
    _sys_usw_port_set_default(lchip, lport);
    PORT_UNLOCK;

    ret = sys_usw_brguc_nh_create(lchip, gport, CTC_NH_PARAM_BRGUC_SUB_TYPE_ALL);
    if (CTC_E_EXIST != ret && CTC_E_NONE != ret)
    {
        return ret;
    }

    return CTC_E_NONE;
}

int32
sys_usw_port_update_mc_linkagg(uint8 lchip, uint8 tid, uint16 lport, bool is_add)
{
    uint8 mc_port_id = 0;
    uint8 mc_port_type = 0;
    uint32 cmd = 0;
    uint16 bitmap_index = 0;
    uint16 port_index = 0;
    uint32 bmp[4] = {0};
    DsMetLinkAggregatePort_m ds_met_link_aggregate_port;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "tid = %d, lport=%d, is_add=%d\n", tid, lport, is_add);

    SYS_LINKAGG_MC_MAP_PBM_PORTID(lport, mc_port_type, mc_port_id);

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "mc_port_type = %d, mc_port_id=%d\n", mc_port_type, mc_port_id);

    bitmap_index = (tid << 1) + mc_port_type;
    cmd = DRV_IOR(DsMetLinkAggregateBitmap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, bitmap_index, cmd, bmp));

    if (is_add)
    {
        CTC_BMP_SET(bmp, mc_port_id);
    }
    else
    {
        CTC_BMP_UNSET(bmp, mc_port_id);
    }

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "DsMetLinkAggregateBitmap_t index = %d, bmp:0x%x:%x:%x:%x\n",
                        bitmap_index, bmp[3], bmp[2], bmp[1], bmp[0]);


    port_index = mc_port_id + (mc_port_type<<7);


    cmd = DRV_IOR(DsMetLinkAggregatePort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_index, cmd, &ds_met_link_aggregate_port));

    if (is_add)
    {
        cmd = DRV_IOW(DsMetLinkAggregateBitmap_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, bitmap_index, cmd, bmp));

        DRV_SET_FIELD_V(lchip, DsMetLinkAggregatePort_t,
                            DsMetLinkAggregatePort_linkAggregationEn_f,
                            &ds_met_link_aggregate_port,
                            TRUE);

        DRV_SET_FIELD_V(lchip, DsMetLinkAggregatePort_t,
                            DsMetLinkAggregatePort_linkAggregationId_f,
                            &ds_met_link_aggregate_port,
                            tid);

        cmd = DRV_IOW(DsMetLinkAggregatePort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_index, cmd, &ds_met_link_aggregate_port));

    }
    else
    {
        DRV_SET_FIELD_V(lchip, DsMetLinkAggregatePort_t,
                            DsMetLinkAggregatePort_linkAggregationEn_f,
                            &ds_met_link_aggregate_port,
                            FALSE);

        DRV_SET_FIELD_V(lchip, DsMetLinkAggregatePort_t,
                            DsMetLinkAggregatePort_linkAggregationId_f,
                            &ds_met_link_aggregate_port,
                            0);

        cmd = DRV_IOW(DsMetLinkAggregatePort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, port_index, cmd, &ds_met_link_aggregate_port));

        cmd = DRV_IOW(DsMetLinkAggregateBitmap_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, bitmap_index, cmd, bmp));
    }

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "DsMetLinkAggregatePort_t index = %d\n", port_index);

    return CTC_E_NONE;
}
/**
 @brief set the port global_src_port and global_dest_port in system, the lport means ctc/sys level lport
*/
int32
sys_usw_port_set_global_port(uint8 lchip, uint16 lport, uint32 gport, bool update_mc_linkagg)
{
    int32  ret = CTC_E_NONE;
    uint32 field_value = 0;
    uint32 cmd = 0;
    uint16 old_gport = 0;
    uint16 agg_gport = 0;
    uint32 agg_port_cnt = 0;
    uint16 tid = 0;
    uint8 gchip_id = 0;
    uint32 real_gport = 0;
    ctc_port_scl_property_t scl_property;

    sal_memset(&scl_property, 0, sizeof(scl_property));

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, \
                     "Set Global port, lchip=%u, lport=%u, gport=0x%04X\n", lchip, lport, gport);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    SYS_GLOBAL_PORT_CHECK(gport);

    if(!CTC_IS_LINKAGG_PORT(gport))
    {
        sys_usw_port_get_global_port(lchip, lport, &agg_gport);
        if(CTC_IS_LINKAGG_PORT(agg_gport))
        {
            tid = agg_gport & CTC_LOCAL_PORT_MASK;
            cmd = DRV_IOR(DsLinkAggregateGroup_t, DsLinkAggregateGroup_linkAggMemNum_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, tid, cmd, &agg_port_cnt));
        }
    }

    if (SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) >= SYS_USW_MAX_PORT_NUM_PER_CHIP)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
			return CTC_E_INVALID_PORT;

    }

    field_value = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(gport);

    /*do write table*/
    cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_globalSrcPort_f);
    ret = DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport), cmd, &field_value);

    cmd = DRV_IOW(DsDestPort_t, DsDestPort_globalDestPort_f);
    ret = ret ? ret : DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport), cmd, &field_value);

    if (p_usw_port_master[lchip]->use_logic_port_check)
    {
        field_value = 1;
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_useDefaultLogicSrcPort_f);
        ret = DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport), cmd, &field_value);

        /*for learning use logic port
          Logic port:
          0-(MAX_PORT_NUM_PER_CHIP-1)
              -----------> local phy port
          MAX_PORT_NUM_PER_CHIP - (MAX_PORT_NUM_PER_CHIP+CTC_MAX_LINKAGG_GROUP_NUM-1)
              ---------> linkagg 0-(CTC_MAX_LINKAGG_GROUP_NUM-1), base:MAX_PORT_NUM_PER_CHIP
          MAX_PORT_NUM_PER_CHIP+CTC_MAX_LINKAGG_GROUP_NUM -
              -------> global logic port for userid and vpls learing, base:MAX_PORT_NUM_PER_CHIP+CTC_MAX_LINKAGG_GROUP_NUM
        */
        if (CTC_IS_LINKAGG_PORT(gport))
        {
            field_value = CTC_GPORT_LINKAGG_ID(gport) + SYS_USW_MAX_PORT_NUM_PER_CHIP - 1;
        }
        else
        {
            field_value  = lport;
        }

        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_defaultLogicSrcPort_f);
        ret = DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport), cmd, &field_value);

        field_value  = lport;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_logicDestPort_f);
        ret = ret ? ret : DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport), cmd, &field_value);
    }

    CTC_ERROR_RETURN(ret);

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "igs_port_prop[%d].global_src_port(drv_port) = 0x%x.\n", lport, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(gport));

    old_gport = p_usw_port_master[lchip]->igs_port_prop[lport].global_src_port;
    p_usw_port_master[lchip]->igs_port_prop[lport].global_src_port = gport;

    if (CTC_IS_LINKAGG_PORT(gport) && update_mc_linkagg)
    {
        /*update mcast linkagg bitmap*/
        CTC_ERROR_RETURN(sys_usw_port_update_mc_linkagg(lchip, CTC_GPORT_LINKAGG_ID(gport), SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport), TRUE));
    }
    else if (CTC_IS_LINKAGG_PORT(old_gport) && update_mc_linkagg)
    {
        /*update mcast linkagg bitmap*/
        CTC_ERROR_RETURN(sys_usw_port_update_mc_linkagg(lchip, CTC_GPORT_LINKAGG_ID(old_gport), SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport), FALSE));
    }

    /* check if need to add ipsg default entry */
    if(CTC_IS_LINKAGG_PORT(gport) && (!DRV_IS_DUET2(lchip)))
    {
        scl_property.direction = CTC_INGRESS;
        for (scl_property.scl_id = 0; scl_property.scl_id <= 1; scl_property.scl_id++)
        {
            sys_usw_get_gchip_id(lchip, &gchip_id);
            real_gport = CTC_MAP_LPORT_TO_GPORT(gchip_id, lport);
            sys_usw_port_get_scl_property(lchip, real_gport, &scl_property);
            if (((CTC_PORT_IGS_SCL_HASH_TYPE_PORT_MAC_SA == scl_property.hash_type)
                || (CTC_PORT_IGS_SCL_HASH_TYPE_PORT_IP_SA == scl_property.hash_type))
                && (MCHIP_CAP(SYS_CAP_SCL_LABEL_FOR_IPSG) == scl_property.class_id))
            {
                sys_usw_port_get_global_port(lchip, lport, &agg_gport);
                ret = sys_usw_ip_source_guard_add_default_entry(lchip, agg_gport, scl_property.scl_id);
                if (CTC_E_ENTRY_EXIST != ret && CTC_E_NONE != ret)
                {
                    return ret;
                }
            }
        }
    }
    else if((!DRV_IS_DUET2(lchip)))
    {
    /* check if need to remove ipsg default entry */

        if (0 == agg_port_cnt)
        {
            scl_property.direction = CTC_INGRESS;
            for (scl_property.scl_id = 0; scl_property.scl_id <= 1; scl_property.scl_id++)
            {
                agg_gport = CTC_MAP_TID_TO_GPORT(tid);
                ret = sys_usw_ip_source_guard_remove_default_entry(lchip, agg_gport, scl_property.scl_id);
                if (CTC_E_ENTRY_NOT_EXIST != ret && CTC_E_NONE != ret)
                {
                    return ret;
                }
            }
        }
    }

    return CTC_E_NONE;
}

/**
 @brief get the port global_src_port and global_dest_port in system. Src and dest are equal.
*/
int32
sys_usw_port_get_global_port(uint8 lchip, uint16 lport, uint16* p_gport)
{
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, \
                     "Get Global port, lchip=%d, lport=%d\n", lchip, lport);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_gport);

    if (SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) >= SYS_USW_MAX_PORT_NUM_PER_CHIP)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
		return CTC_E_INVALID_PORT;
    }

    *p_gport = p_usw_port_master[lchip]->igs_port_prop[lport].global_src_port;

    return CTC_E_NONE;
}

int32
sys_usw_port_set_mcast_member_down(uint8 lchip, uint32 gport, uint8 value)
{
    uint16 lport                   = 0;
    uint32 cmd                     = 0;
    MetFifoIsolateCtl_m     met_fifo_ctl;
    uint32 bitmap[8] = {0};

    CTC_MAX_VALUE_CHECK(value, 1);

    sal_memset(&met_fifo_ctl, 0, sizeof(met_fifo_ctl));
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    cmd = DRV_IOR(MetFifoIsolateCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &met_fifo_ctl));

    GetMetFifoIsolateCtl(A, portNonBlockBitmap0_f, &met_fifo_ctl, bitmap);
    if(value)
    {
        CTC_BMP_UNSET(bitmap, lport);
    }
    else
    {
        CTC_BMP_SET(bitmap, lport);
    }

    SetMetFifoIsolateCtl(A, portNonBlockBitmap0_f, &met_fifo_ctl, bitmap);

    cmd = DRV_IOW(MetFifoIsolateCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &met_fifo_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_port_set_port_en(uint8 lchip, uint32 gport, bool enable)
{
    uint16 lport = 0;
    int32  ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 value = 0;
    uint16 chan_id = 0;

     /*ctc_chip_device_info_t dev_info;*/
     /*sal_memset(&dev_info, 0, sizeof(ctc_chip_device_info_t));*/

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set gport:0x%04X, port enable:%d!\n", gport, enable);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
#if 0 /* SYSTEM MODIFIED by wus for sdk bug 105157 system bug 47782(E) and 54179(V) 2019-10-30 */
    value = (TRUE == enable) ? 0 : 1;
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_receiveDisable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));

    value = (TRUE == enable) ? 0 : 1;
    cmd = DRV_IOW(DsDestPort_t, DsDestPort_transmitDisable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
#endif    

    /* clear link status to up*/
    if (TRUE == enable)
    {
        chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
        if (0xFF != chan_id)
        {
            value = 0;
            cmd = DRV_IOW(LagEngineLinkState_t, LagEngineLinkState_linkState_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &value));
            cmd = DRV_IOW(DlbChanStateCtl_t, DlbChanStateCtl_linkState_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &value));
        }
    }
#if 0
    /* check chip version */
    sys_usw_chip_get_device_info(lchip, &dev_info);

    if (dev_info.version_id <= 1)
    {
        if (p_usw_port_master[lchip]->igs_port_prop[lport].link_intr_en)
        {
            _sys_usw_port_set_link_intr(lchip, gport, enable);
        }
    }
#endif

    ret = sys_usw_brguc_nh_create(lchip, gport, CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC);
    if (CTC_E_EXIST != ret && CTC_E_NONE != ret)
    {
        return ret;
    }
    CTC_ERROR_RETURN(sys_usw_port_set_mcast_member_down(lchip, gport, (TRUE == enable) ? 0 : 1));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_port_get_port_en(uint8 lchip, uint32 gport, uint32* p_enable)
{
    uint16 lport = 0;
    uint32 cmd = 0;
    uint32 receive = 0;
    uint32 transmit = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get gport:0x%04X, port enable!\n", gport);

    CTC_PTR_VALID_CHECK(p_enable);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_receiveDisable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &receive));

    cmd = DRV_IOR(DsDestPort_t, DsDestPort_transmitDisable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &transmit));

    if ((0 == receive) && (0 == transmit))
    {
        *p_enable = TRUE;
    }
    else
    {
        *p_enable = FALSE;
    }

    return CTC_E_NONE;
}



STATIC int32
_sys_usw_port_set_cut_through_en(uint8 lchip, uint32 gport,uint32 enable)
{
    uint16 lport = 0;
    uint32 cmd = 0;
    uint32 speed = 0;
	uint32 field_val = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set gport:0x%04X!\n", gport);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    speed = enable? sys_usw_chip_get_cut_through_speed(lchip, gport) :0;

    field_val = (speed&0x7);
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_speed_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));

    field_val = ((speed>>3)&0x1);
	cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_gCtSrcportSpeed_0_speedHigherBit_f + lport);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    cmd = DRV_IOW(DsDestPortSpeed_t, DsDestPortSpeed_speed_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &speed));

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_port_get_cut_through_en(uint8 lchip, uint32 gport,uint32 *enable)
{
    uint16 lport = 0;
    uint32 cmd = 0;
    uint32 speed = 0;
     SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    cmd = DRV_IOR(DsDestPortSpeed_t, DsDestPortSpeed_speed_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &speed));

    *enable = (speed != 0);

    return CTC_E_NONE;
}

/**
 @brief decide the port whether is routed port
*/
STATIC int32
_sys_usw_port_set_routed_port(uint8 lchip, uint32 gport, uint32 is_routed)
{

    int32 ret = CTC_E_NONE;

    uint16 lport = 0;
    uint32 cmd = 0;
    uint32 field_value = (TRUE == is_routed) ? 1 : 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set gport:0x%04X, routed port:%d!\n", gport, is_routed);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    /*do write table*/
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_routedPort_f);
    ret = DRV_IOCTL(lchip, lport, cmd, &field_value);

    cmd = DRV_IOW(DsDestPort_t, DsDestPort_routedPort_f);
    ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

    return ret;
}

/**
 @brief get the port whether is routed port
*/
STATIC int32
_sys_usw_port_get_routed_port(uint8 lchip, uint32 gport, bool* p_is_routed)
{
    int32 ret = CTC_E_NONE;

    uint16 lport = 0;
    uint32 cmd = 0;
    uint32 value1 = 0;
    uint32 value2 = 0;

    CTC_PTR_VALID_CHECK(p_is_routed);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get gport:0x%04X!\n", gport);

    /*do write table*/
    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_routedPort_f);
    ret = DRV_IOCTL(lchip, lport, cmd, &value1);

    cmd = DRV_IOR(DsDestPort_t, DsDestPort_routedPort_f);
    ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &value2);

    if (value1 && value2)
    {
        *p_is_routed = TRUE;
    }
    else
    {
        *p_is_routed = FALSE;
    }

    return ret;
}

/**
 @brief bind phy_if to port
*/
int32
sys_usw_port_set_phy_if_en(uint8 lchip, uint32 gport, bool enable)
{
    int32  ret = CTC_E_NONE;

    uint16 lport = 0;
    uint32 field_value = 0;
    uint32 cmd = 0;
    uint32 is_phy = FALSE;
    uint16 global_src_port = 0;
    sys_l3if_prop_t l3if_prop;
    uint8  is_support_glb_port = _sys_usw_port_is_support_glb_port(lchip);

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set gport:0x%04X, phy_if enable:%d\n", gport, enable);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    SYS_GLOBAL_PORT_CHECK(gport);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    PORT_LOCK;

    global_src_port = p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(gport)].global_src_port;
    sal_memset(&l3if_prop, 0, sizeof(sys_l3if_prop_t));

    l3if_prop.gport = global_src_port;
    l3if_prop.l3if_type = CTC_L3IF_TYPE_PHY_IF;
    l3if_prop.vaild     = 1;

    is_phy = sys_usw_l3if_is_port_phy_if(lchip, global_src_port);

    if ((TRUE == enable) && (FALSE == is_phy))
    {
        CTC_ERROR_RETURN_WITH_UNLOCK(CTC_E_NOT_EXIST, p_usw_port_master[lchip]->p_port_mutex);
    }

#if 0 /*allow destroy interfaced first, then disable in port*/
    if ((FALSE == enable) && (TRUE == is_phy))
    {
        CTC_ERROR_RETURN_WITH_UNLOCK(CTC_E_EXIST, p_usw_port_master[lchip]->p_port_mutex);
    }

#endif

    if (TRUE == enable)
    {
        ret = _sys_usw_port_set_routed_port(lchip, gport, TRUE);
        ret = ret ? ret : sys_usw_l3if_get_l3if_info(lchip, 0, &l3if_prop);

/*SYSTEM MODIFIED for hybrid530 porting*/
#ifndef OFPRODUCT
        field_value = 0;
        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_dot1QEn_f) : DRV_IOW(DsDestPort_t, DsDestPort_dot1QEn_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);
#endif

        field_value = l3if_prop.l3if_id;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_interfaceId_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

/*SYSTEM MODIFIED for hybrid530 porting*/
#ifndef OFPRODUCT
        field_value = 1;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_addDefaultVlanDisable_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);
#endif

        field_value = 1;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_stpDisable_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

        field_value = 0;
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_defaultVlanId_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

        field_value = 0;
        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_defaultVlanId_f) : DRV_IOW(DsDestPort_t, DsDestPort_defaultVlanId_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

        /* enable phy if should disable bridge */
        field_value = 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_bridgeEn_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_bridgeEn_f) : DRV_IOW(DsDestPort_t, DsDestPort_bridgeEn_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

        field_value = l3if_prop.l3if_id;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_portMappedInterfaceId_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);


        CTC_ERROR_RETURN_WITH_UNLOCK(ret, p_usw_port_master[lchip]->p_port_mutex);
    }
    else
    {
        ret = _sys_usw_port_set_routed_port(lchip, gport, FALSE);

/*SYSTEM MODIFIED by wangl 2018-5-31*/
#ifndef OFPRODUCT
        field_value = CTC_DOT1Q_TYPE_BOTH;
        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_dot1QEn_f) : DRV_IOW(DsDestPort_t, DsDestPort_dot1QEn_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);
#endif

        field_value = 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_interfaceId_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

/*SYSTEM MODIFIED for hybrid530 porting*/
#ifndef OFPRODUCT
        field_value = 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_addDefaultVlanDisable_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);
#endif

        field_value = 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_stpDisable_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

        field_value = 1;
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_defaultVlanId_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

        field_value = 1;

        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_defaultVlanId_f) : DRV_IOW(DsDestPort_t, DsDestPort_defaultVlanId_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

        /* disable phy if should enable bridge */
        field_value = 1;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_bridgeEn_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_bridgeEn_f) : DRV_IOW(DsDestPort_t, DsDestPort_bridgeEn_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

        field_value = 0;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_portMappedInterfaceId_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

        CTC_ERROR_RETURN_WITH_UNLOCK(ret, p_usw_port_master[lchip]->p_port_mutex);
    }

    PORT_UNLOCK;

    return CTC_E_NONE;
}

/**
 @brief get phy_if whether enable on port
*/

int32
sys_usw_port_get_phy_if_en(uint8 lchip, uint32 gport, uint16* p_l3if_id, bool* p_enable)
{
    bool   is_routed = FALSE;

    int32  ret = CTC_E_NONE;
    uint16 lport = 0;
    uint32 value = 0;
    uint32 cmd = 0;
    uint32 is_phy = 0;
    DsSrcPort_m ds_src_port;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get gport:0x%04X, phyical interface!\n", gport);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_enable);
    CTC_PTR_VALID_CHECK(p_l3if_id);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    is_phy = sys_usw_l3if_is_port_phy_if(lchip, gport);
    CTC_ERROR_RETURN(_sys_usw_port_get_routed_port(lchip, gport, &is_routed));

    if ((TRUE == is_routed) && (TRUE == is_phy))
    {
        *p_enable = TRUE;
    }
    else
    {
        *p_enable = FALSE;
    }

    sal_memset(&ds_src_port, 0, sizeof(DsSrcPort_m));
    cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_src_port));

    GetDsSrcPort(A, interfaceId_f, &ds_src_port, &value);
    *p_l3if_id = value;

    return ret;
}

int32
sys_usw_port_set_vlan_range(uint8 lchip, uint32 gport, ctc_vlan_range_info_t* p_vrange_info, bool enable)
{
    int32 ret = CTC_E_NONE;

    uint16 lport = 0;
    uint32 cmd = 0;
    uint32 field_val = 0;
    bool is_svlan = FALSE;
    uint8 direcion = p_vrange_info->direction;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set gport:0x%04X, direction:%d, vrange_grpid:%d\n",
                     gport, p_vrange_info->direction, p_vrange_info->vrange_grpid);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_vrange_info);
    SYS_VLAN_RANGE_INFO_CHECK(p_vrange_info);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    if(enable)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_get_vlan_range_type(lchip, p_vrange_info, &is_svlan));
    }

    if (CTC_INGRESS == direcion)
    {
        if (!enable)
        {
            field_val = 0;
            cmd = DRV_IOW(DsPhyPort_t, DsPhyPort_vlanRangeProfileEn_f);
            ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_val);
        }
        else
        {
            if (is_svlan)
            {
                field_val = 1;
            }
            else
            {
                field_val = 0;
            }

            cmd = DRV_IOW(DsPhyPort_t, DsPhyPort_vlanRangeType_f);
            ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_val);

            field_val = 1;
            cmd = DRV_IOW(DsPhyPort_t, DsPhyPort_vlanRangeProfileEn_f);
            ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_val);

            field_val = p_vrange_info->vrange_grpid;
            cmd = DRV_IOW(DsPhyPort_t, DsPhyPort_vlanRangeProfile_f);
            ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_val);
        }
    }
    else
    {
        if (!enable)
        {
            field_val = 0;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_vlanRangeProfileEn_f);
            ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_val);
        }
        else
        {
            if (is_svlan)
            {
                field_val = 1;
            }
            else
            {
                field_val = 0;
            }

            cmd = DRV_IOW(DsDestPort_t, DsDestPort_vlanRangeType_f);
            ret =  DRV_IOCTL(lchip, lport, cmd, &field_val);

            field_val = 1;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_vlanRangeProfileEn_f);
            ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_val);

            field_val = p_vrange_info->vrange_grpid;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_vlanRangeProfile_f);
            ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_val);
        }
    }

    return ret;
}

int32
sys_usw_port_get_vlan_range(uint8 lchip, uint32 gport, ctc_vlan_range_info_t* p_vrange_info, bool* p_enable)
{
    int32 ret = CTC_E_NONE;

    uint16 lport = 0;
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint8 direction = p_vrange_info->direction;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get gport:0x%04X, vlan range!\n", gport);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_enable);
    CTC_PTR_VALID_CHECK(p_vrange_info);
    CTC_BOTH_DIRECTION_CHECK(direction);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    if (CTC_INGRESS == direction)
    {
        cmd = DRV_IOR(DsPhyPort_t, DsPhyPort_vlanRangeProfileEn_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_val);
        if (0 == field_val)
        {
            *p_enable = FALSE;
        }
        else
        {
            cmd = DRV_IOR(DsPhyPort_t, DsPhyPort_vlanRangeProfile_f);
            ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_val);
            p_vrange_info->vrange_grpid = field_val;
            *p_enable = TRUE;

        }
    }
    else
    {
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_vlanRangeProfileEn_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_val);
        if (0 == field_val)
        {
            *p_enable = FALSE;
        }
        else
        {
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_vlanRangeProfile_f);
            ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_val);
            p_vrange_info->vrange_grpid = field_val;
            *p_enable = TRUE;
        }
    }

    return ret;
}

STATIC int32
_sys_usw_port_set_scl_key_type_ingress(uint8 lchip, uint16 lport, uint8 scl_id, ctc_port_scl_key_type_t type)
{
    DsPhyPortExt_m ds;
    uint32 cmd = 0;

    sys_usw_igs_port_scl_map_t scl_map[CTC_SCL_KEY_TYPE_MAX] =
    {
        /* hash_type                       , tcam_type   ,en,high,ul,lab,ad,da,v4,gre,rpf,auto,nvgre,v6_nvgre,v4_vxlan,v6_vxlan*/
        {USERIDPORTHASHTYPE_DISABLE        , TCAMDISABLE , 0, 0 , 0 , 0 , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_DISABLE                */
        {USERIDPORTHASHTYPE_PORT           , TCAML2KEY   , 1, 0 , 0 , 0 , 1, 0, 0, 0, 0, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_VLAN_MAPPING_PORT      */
        {USERIDPORTHASHTYPE_CVLANPORT      , TCAML2KEY   , 1, 0 , 0 , 0 , 1, 0, 0, 0, 0, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_VLAN_MAPPING_CVID      */
        {USERIDPORTHASHTYPE_SVLANPORT      , TCAML2KEY   , 1, 0 , 0 , 0 , 1, 0, 0, 0, 0, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_VLAN_MAPPING_SVID      */
        {USERIDPORTHASHTYPE_CVLANCOSPORT   , TCAML2KEY   , 1, 0 , 0 , 0 , 1, 0, 0, 0, 0, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_VLAN_MAPPING_CVID_CCOS */
        {USERIDPORTHASHTYPE_SVLANCOSPORT   , TCAML2KEY   , 1, 0 , 0 , 0 , 1, 0, 0, 0, 0, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_VLAN_MAPPING_SVID_SCOS */
        {USERIDPORTHASHTYPE_DOUBLEVLANPORT , TCAML2KEY   , 1, 0 , 0 , 0 , 1, 0, 0, 0, 0, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_VLAN_MAPPING_DVID      */
        {USERIDPORTHASHTYPE_MAC            , TCAML2KEY   , 1, 1 , 1 , 62, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_VLAN_CLASS_MAC_SA      */
        {USERIDPORTHASHTYPE_MAC            , TCAML2KEY   , 1, 1 , 1 , 62, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_VLAN_CLASS_MAC_DA      */
        {USERIDPORTHASHTYPE_IPSA           , TCAML3KEY   , 1, 1 , 1 , 62, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_VLAN_CLASS_IPV4        */
        {USERIDPORTHASHTYPE_IPSA           , TCAML3KEY   , 1, 1 , 1 , 62, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_VLAN_CLASS_IPV6        */
        {USERIDPORTHASHTYPE_MACPORT        , TCAML2KEY   , 1, 1 , 0 , 63, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_IPSG_PORT_MAC          */
        {USERIDPORTHASHTYPE_IPSAPORT       , TCAML3KEY   , 1, 1 , 0 , 63, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_IPSG_PORT_IP           */
        {USERIDPORTHASHTYPE_IPSA           , TCAML3KEY   , 1, 1 , 0 , 63, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_IPSG_IPV6              */
        {USERIDPORTHASHTYPE_TUNNEL         , TCAML3KEY   , 1, 0 , 0 , 0 , 0, 0, 1, 1, 0, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_IPV4_TUNNEL            */
        {USERIDPORTHASHTYPE_TUNNEL         , TCAML3KEY   , 1, 0 , 0 , 0 , 0, 0, 1, 1, 0, 1, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_IPV4_TUNNEL_AUTO       */
        {USERIDPORTHASHTYPE_TUNNEL         , TCAML3KEY   , 1, 0 , 0 , 0 , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_IPV6_TUNNEL            */
        {USERIDPORTHASHTYPE_TUNNEL         , TCAML3KEY   , 1, 0 , 0 , 0 , 0, 0, 1, 1, 1, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_IPV4_TUNNEL_WITH_RPF   */
        {USERIDPORTHASHTYPE_TUNNEL         , TCAML3KEY   , 1, 0 , 0 , 0 , 0, 0, 0, 0, 1, 0, 0, 0, 0, 0} , /*CTC_SCL_KEY_TYPE_IPV6_TUNNEL_WITH_RPF   */
        {USERIDPORTHASHTYPE_TUNNEL         , TCAML3KEY   , 1, 0 , 0 , 0 , 0, 0, 0, 1, 0, 0, 1, 1, 0, 0} , /*CTC_SCL_KET_TYPE_NVGRE                  */
        {USERIDPORTHASHTYPE_TUNNEL         , TCAML3KEY   , 1, 0 , 0 , 0 , 0, 0, 0, 0, 0, 0, 0, 0, 1, 1} , /*CTC_SCL_KET_TYPE_VXLAN                  */
    };

    cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport), cmd, &ds));
    if (0 == scl_id)
    {
        p_usw_port_master[lchip]->igs_port_prop[lport].flag = type;
    }
    else if (1 == scl_id)
    {
        p_usw_port_master[lchip]->igs_port_prop[lport].flag_ext = type;
    }

    {
        if (CTC_SCL_KEY_TYPE_IPV4_TUNNEL_WITH_RPF == type)
        {
            if (0 == scl_id)
            {
                return CTC_E_INVALID_PARAM;
            }
        }

        if (CTC_SCL_KEY_TYPE_IPV6_TUNNEL_WITH_RPF == type)
        {
            if (0 == scl_id)
            {
                return CTC_E_INVALID_PARAM;
            }
        }
    }

    if (0 == scl_id)
    {
        SetDsPhyPortExt(V, hash1UseDa_f, &ds, scl_map[type].use_macda);
        SetDsPhyPortExt(V, userIdPortHash1Type_f, &ds, scl_map[type].hash_type);
        SetDsPhyPortExt(V, userIdTcam1En_f, &ds, scl_map[type].tcam_en);
        SetDsPhyPortExt(V, userIdTcam1Type_f, &ds, scl_map[type].tcam_type);
        SetDsPhyPortExt(V, hashLookup1UseLabel_f , &ds, scl_map[type].use_label);
        SetDsPhyPortExt(V, userIdLabel1_f, &ds, scl_map[type].label);

        SetDsPhyPortExt(V, ipv4TunnelHashEn1_f, &ds, scl_map[type].ipv4_tunnel_en);
        SetDsPhyPortExt(V, ipv4GreTunnelHashEn1_f, &ds, scl_map[type].ipv4_gre_tunnel_en);
        SetDsPhyPortExt(V, autoTunnelEn_f, &ds, scl_map[type].auto_tunnel);
        SetDsPhyPortExt(V, autoTunnelForHash1_f, &ds, scl_map[type].auto_tunnel);
        SetDsPhyPortExt(V, nvgreEnable_f, &ds, scl_map[type].nvgre_en);
        SetDsPhyPortExt(V, ipv4VxlanTunnelHashEn1_f, &ds, scl_map[type].v4_vxlan_en);
        SetDsPhyPortExt(V, ipv6NvgreTunnelHashEn1_f, &ds, scl_map[type].v6_nvgre_en);
        SetDsPhyPortExt(V, ipv6VxlanTunnelHashEn1_f, &ds, scl_map[type].v6_vxlan_en);

        SetDsPhyPortExt(V, tcam1IsUserId_f, &ds, scl_map[type].is_userid);
        SetDsPhyPortExt(V, tcam1IsSclFlow_f, &ds, 0);
        SetDsPhyPortExt(V, sclFlowHashEn_f , &ds, 0);
    }
    else if (1 == scl_id)
    {
        SetDsPhyPortExt(V, hash2UseDa_f, &ds, scl_map[type].use_macda);
        SetDsPhyPortExt(V, userIdPortHash2Type_f, &ds, scl_map[type].hash_type);
        SetDsPhyPortExt(V, userIdTcam2En_f, &ds, scl_map[type].tcam_en);
        SetDsPhyPortExt(V, userIdTcam2Type_f, &ds, scl_map[type].tcam_type);
        SetDsPhyPortExt(V, hashLookup2UseLabel_f , &ds, scl_map[type].use_label);
        SetDsPhyPortExt(V, userIdLabel2_f, &ds, scl_map[type].label);

        SetDsPhyPortExt(V, ipv4TunnelHashEn2_f, &ds, scl_map[type].ipv4_tunnel_en);
        SetDsPhyPortExt(V, ipv4GreTunnelHashEn2_f, &ds, scl_map[type].ipv4_gre_tunnel_en);
        SetDsPhyPortExt(V, autoTunnelEn_f, &ds, scl_map[type].auto_tunnel);
        SetDsPhyPortExt(V, tunnelRpfCheck_f, &ds, scl_map[type].rpf_check_en);
        SetDsPhyPortExt(V, autoTunnelForHash1_f, &ds, !(scl_map[type].auto_tunnel));
        SetDsPhyPortExt(V, nvgreEnable_f, &ds, scl_map[type].nvgre_en);
        SetDsPhyPortExt(V, ipv4VxlanTunnelHashEn2_f, &ds, scl_map[type].v4_vxlan_en);
        SetDsPhyPortExt(V, ipv6NvgreTunnelHashEn2_f, &ds, scl_map[type].v6_nvgre_en);
        SetDsPhyPortExt(V, ipv6VxlanTunnelHashEn2_f, &ds, scl_map[type].v6_vxlan_en);

        SetDsPhyPortExt(V, tcam2IsUserId_f, &ds, scl_map[type].is_userid);
        SetDsPhyPortExt(V, tcam2IsSclFlow_f, &ds, 0);
    }

    cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport), cmd, &ds));
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_ingressTagHighPriority_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport), cmd, &scl_map[type].vlan_high_priority));

    return CTC_E_NONE;
}
/**
 @brief bind phy_if to port
*/
int32
sys_usw_port_set_sub_if_en(uint8 lchip, uint32 gport, bool enable)
{
    int32 ret = CTC_E_NONE;

    uint16 lport = 0;
    uint32 field_value = 0;
    uint32 cmd = 0;
    uint32 is_sub = FALSE;
    uint16 global_src_port = 0;
    sys_l3if_prop_t l3if_prop;
    uint8  is_support_glb_port = _sys_usw_port_is_support_glb_port(lchip);
    /*Sanity check*/
    SYS_USW_PORT_INIT_CHECK();

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set gport:0x%04X, phy_if enable:%d\n", gport, enable);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    PORT_LOCK;

    global_src_port = p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(gport)].global_src_port;
    sal_memset(&l3if_prop, 0, sizeof(sys_l3if_prop_t));
    l3if_prop.gport     = global_src_port;
    l3if_prop.l3if_type = CTC_L3IF_TYPE_SUB_IF;
    l3if_prop.vaild     = 1;

    is_sub = sys_usw_l3if_is_port_sub_if(lchip, global_src_port);

    if ((TRUE == enable) && (FALSE == is_sub))
    {
        CTC_ERROR_RETURN_WITH_UNLOCK(CTC_E_NOT_EXIST, p_usw_port_master[lchip]->p_port_mutex);
    }

    if (TRUE == enable)
    {
        CTC_ERROR_RETURN_WITH_UNLOCK(
        _sys_usw_port_set_scl_key_type_ingress(lchip,
                                                     CTC_MAP_GPORT_TO_LPORT(gport),
                                                     0,
                                                     CTC_SCL_KEY_TYPE_VLAN_MAPPING_SVID),
                                                     p_usw_port_master[lchip]->p_port_mutex);

        ret = ret ? ret : _sys_usw_port_set_routed_port(lchip, gport, TRUE);


        /* enable sub if should disable bridge */
        field_value = 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_bridgeEn_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_bridgeEn_f) : DRV_IOW(DsDestPort_t, DsDestPort_bridgeEn_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);
        CTC_ERROR_RETURN_WITH_UNLOCK(ret, p_usw_port_master[lchip]->p_port_mutex);

        p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(gport)].subif_en = TRUE;


    }
    else if (p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(gport)].subif_en)
    {
        CTC_ERROR_RETURN_WITH_UNLOCK(
        _sys_usw_port_set_scl_key_type_ingress(lchip,
                                                     CTC_MAP_GPORT_TO_LPORT(gport),
                                                     0,
                                                     CTC_SCL_KEY_TYPE_DISABLE),
                                                     p_usw_port_master[lchip]->p_port_mutex);

        ret = ret ? ret : _sys_usw_port_set_routed_port(lchip, gport, FALSE);

        /* enable sub if should disable bridge */
        field_value = 1;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_bridgeEn_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_bridgeEn_f) : DRV_IOW(DsDestPort_t, DsDestPort_bridgeEn_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field_value);

        CTC_ERROR_RETURN_WITH_UNLOCK(ret, p_usw_port_master[lchip]->p_port_mutex);

        p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(gport)].subif_en = FALSE;
    }

    PORT_UNLOCK;

    return CTC_E_NONE;
}

/**
 @brief get phy_if whether enable on port
*/
int32
sys_usw_port_get_sub_if_en(uint8 lchip, uint32 gport,  bool* p_enable)
{
    uint16 lport = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_enable);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    PORT_LOCK;
    *p_enable = p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(gport)].subif_en;
    PORT_UNLOCK;
    return CTC_E_NONE;
}
#if 0
STATIC int32
_sys_usw_port_set_pause_speed_mode(uint8 lchip, uint8 slice_id, uint8 mac_id, ctc_port_speed_t speed_mode)
{
    #ifdef NEVER
    uint32 cmd  = 0;
    uint32 tb_id = 0;
    NetRxPauseTimerMem0_m rx_pause;
    uint32 mode = 0;

    tb_id = NetRxPauseTimerMem0_t + slice_id;

    cmd = DRV_IOR(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, mac_id, cmd, &rx_pause));

    switch (speed_mode)
    {
        case CTC_PORT_SPEED_10M:
        case CTC_PORT_SPEED_100M:
        case CTC_PORT_SPEED_1G:
            mode = 0x3;
            break;
        case CTC_PORT_SPEED_2G5:
            mode = 0x4;
            break;
        case CTC_PORT_SPEED_10G:
            mode = 0x0;
            break;
        case CTC_PORT_SPEED_20G:
        case CTC_PORT_SPEED_40G:
            mode = 0x1;
            break;
        case CTC_PORT_SPEED_100G:
            mode = 0x2;
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    SetNetRxPauseTimerMem0(V, speedMode_f, &rx_pause, mode);

    cmd = DRV_IOW(tb_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, mac_id, cmd, &rx_pause));
    #endif
    return CTC_E_NONE;
}
#endif


int32
_sys_usw_port_maping_to_local_phy_port(uint8 lchip, uint32 gport, uint16 local_phy_port)
{
    uint16 lport = 0;
    uint32 cmd = 0;
    uint32 field_value = local_phy_port;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    cmd = DRV_IOW(IpeHeaderAdjustPhyPortMap_t, IpeHeaderAdjustPhyPortMap_localPhyPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_value));

    return CTC_E_NONE;
}

int32
sys_usw_port_set_lbk_port_property(uint8 lchip, ctc_port_lbk_param_t* p_port_lbk, uint16 inter_lport, uint16 dsfwd_offset)
{


    uint8  enable = p_port_lbk->lbk_enable ? 1 : 0;
    uint16 src_lport = 0;
    uint16 inter_dlport = 0;
    uint32 src_gport = 0;
    uint32 cmd = 0;
    DsPhyPortExt_m inter_phy_port_ext;
    DsSrcPort_m inter_src_port;
    DsDestPort_m inter_dest_port;

    CTC_PTR_VALID_CHECK(p_port_lbk);

    src_gport = p_port_lbk->src_gport;
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(src_gport, lchip, src_lport);
    inter_dlport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(inter_lport);

    sal_memset(&inter_src_port,     0, sizeof(DsSrcPort_m));
    sal_memset(&inter_dest_port,    0, sizeof(DsDestPort_m));
    sal_memset(&inter_phy_port_ext, 0, sizeof(DsPhyPortExt_m));

    cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_dlport, cmd, &inter_src_port));

    SetDsSrcPort(V, portCrossConnect_f, &inter_src_port, (enable ? 1 : 0));
    SetDsSrcPort(V, receiveDisable_f, &inter_src_port, 0);
    SetDsSrcPort(V, stpDisable_f, &inter_src_port, (enable ? 1 : 0));
    SetDsSrcPort(V, addDefaultVlanDisable_f, &inter_src_port, (enable ? 1 : 0));
    SetDsSrcPort(V, routeDisable_f, &inter_src_port, (enable ? 1 : 0));
    SetDsSrcPort(V, learningDisable_f, &inter_src_port, (enable ? 1 : 0));
    SetDsSrcPort(V, ingressFilteringMode_f, &inter_src_port, SYS_PORT_VLAN_FILTER_MODE_VLAN);

    /*l2pdu same to src port*/
    cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_dlport, cmd, &inter_phy_port_ext));

    cmd = DRV_IOR(DsDestPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_dlport, cmd, &inter_dest_port));

    SetDsPhyPortExt(V, u1_g2_dsFwdPtr_f, &inter_phy_port_ext, dsfwd_offset);

    if (enable)
    {
        SetDsSrcPort(V, bridgeEn_f, &inter_src_port, 0);
        SetDsPhyPortExt(V, defaultVlanId_f, &inter_phy_port_ext, 0);
        SetDsDestPort(V, stpCheckEn_f, &inter_dest_port, 0);
        if ((p_port_lbk->src_gport == p_port_lbk->dst_gport) && p_port_lbk->lbk_mode)
        {
            SetDsPhyPortExt(V, efmLoopbackEn_f, &inter_phy_port_ext, 1);
            SetDsDestPort(V, discardNon8023Oam_f, &inter_dest_port, 1);
        }
    }
    else
    {
        SetDsSrcPort(V, bridgeEn_f, &inter_src_port, 1);
        SetDsPhyPortExt(V, exception2En_f, &inter_phy_port_ext, 0);
        SetDsPhyPortExt(V, exception2Discard_f, &inter_phy_port_ext, 0);
        SetDsPhyPortExt(V, efmLoopbackEn_f, &inter_phy_port_ext, 0);
        SetDsPhyPortExt(V, defaultVlanId_f, &inter_phy_port_ext, 1);
        SetDsDestPort(V, discardNon8023Oam_f, &inter_dest_port, 0);
        SetDsDestPort(V, reflectiveBridgeEn_f, &inter_dest_port, (enable ? 1 : 0));
        SetDsDestPort(V, egressFilteringMode_f, &inter_dest_port, SYS_PORT_VLAN_FILTER_MODE_VLAN);
        SetDsDestPort(V, stpCheckEn_f, &inter_dest_port, 1);
    }

    cmd = DRV_IOW(DsSrcPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_dlport, cmd, &inter_src_port));

    cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_dlport, cmd, &inter_phy_port_ext));

    cmd = DRV_IOW(DsDestPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_dlport, cmd, &inter_dest_port));

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_port_set_cross_connect(uint8 lchip, uint32 cc_gport, uint32 nhid)
{
    uint16  lport = 0;
    uint32  cmd = 0;
    uint32  field_val = 0;
    bool enable = FALSE;
    uint32 dsfwd_offset = 0;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(cc_gport, lchip, lport);

    /* when efm loopback enabled on src port, cc should not be enabled.*/
    if (p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(cc_gport)].lbk_en)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Has other feature on this port \n");
			return CTC_E_INVALID_CONFIG;

    }


    if (SYS_NH_INVALID_NHID != nhid)
    {
        CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, nhid, &dsfwd_offset, 0));
        enable = TRUE;
    }

    field_val = enable ? 1 : 0;
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_portCrossConnect_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));

    field_val = enable ? 0 : 1;
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_bridgeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));


    field_val = enable ? 1 : 0;
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_routedPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));

    field_val = enable ? 1 : 0;
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_addDefaultVlanDisable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));

    field_val = enable ? 1 : 0;
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_learningDisable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));

    field_val = dsfwd_offset;
    cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_u1_g2_dsFwdPtr_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));

    p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(cc_gport)].nhid = nhid;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_port_get_cross_connect(uint8 lchip, uint32 cc_gport, uint32* p_value)
{
    uint16 lport = 0;
    uint32 cmd = 0;
    uint32 bridge_en = 0;
    uint32 port_cross_connect = 0;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(cc_gport, lchip, lport);

    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_bridgeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &bridge_en));

    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_portCrossConnect_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &port_cross_connect));

    if ((0 == bridge_en) && (1 == port_cross_connect))
    {
        *p_value = p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(cc_gport)].nhid;
    }
    else
    {
        *p_value = SYS_NH_INVALID_NHID;
    }

    return CTC_E_NONE;
}

/* efm and crossconnect loopback */
STATIC int32
_sys_usw_port_set_loopback(uint8 lchip, ctc_port_lbk_param_t* p_port_lbk)
{
    uint16  sys_internal_gport = 0;
    uint16  sys_internal_lport = 0;
    uint16  drv_internal_lport = 0;
    uint16  drv_src_lport = 0;
    uint32  sys_src_gport   = 0;
    uint32  sys_dst_gport   = 0;
    uint32  dsfwd_offset = 0;
    uint32  nhid        = 0;
    uint8   gchip       = 0;
    uint8   chan        = 0;

    int32  ret         = CTC_E_NONE;
    ctc_misc_nh_param_t nh_param;
    ctc_internal_port_assign_para_t port_assign;

#define RET_PROCESS_WITH_ERROR(func) ret = ret ? ret : (func)

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_port_lbk);
    SYS_GLOBAL_PORT_CHECK(p_port_lbk->src_gport);
    SYS_GLOBAL_PORT_CHECK(p_port_lbk->dst_gport);

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sys_src_gport = p_port_lbk->src_gport;
    sys_dst_gport = p_port_lbk->dst_gport;

    /*not support linkagg*/
    if (CTC_IS_LINKAGG_PORT(sys_src_gport))
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(sys_src_gport, lchip, drv_src_lport);

    gchip = CTC_MAP_GPORT_TO_GCHIP(sys_src_gport);
    SYS_GLOBAL_CHIPID_CHECK(gchip);
    sal_memset(&nh_param,   0, sizeof(ctc_misc_nh_param_t));
    sal_memset(&port_assign,    0, sizeof(ctc_internal_port_assign_para_t));


    CTC_ERROR_RETURN(_sys_usw_port_get_cross_connect(lchip, sys_src_gport, &nhid));

    /* cc is enabled on src port */
    if (SYS_NH_INVALID_NHID != nhid)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Has other feature on this port \n");
			return CTC_E_INVALID_CONFIG;

    }

    nhid = 0;

    if (p_port_lbk->lbk_enable)
    {
        if (p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(sys_src_gport)].lbk_en)
        {
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
			return CTC_E_EXIST;

        }

        /* allocate internal port */
        port_assign.type        = CTC_INTERNAL_PORT_TYPE_FWD;
        port_assign.gchip       = gchip;
        if (CTC_PORT_LBK_MODE_EFM == p_port_lbk->lbk_mode)
        {
            port_assign.fwd_gport       = sys_src_gport;
        }
        else if(CTC_PORT_LBK_MODE_CC == p_port_lbk->lbk_mode)
        {
            port_assign.fwd_gport       = sys_dst_gport;
        }

        CTC_ERROR_RETURN(sys_usw_internal_port_allocate(lchip, &port_assign));

        sys_internal_lport = SYS_MAP_DRV_LPORT_TO_SYS_LPORT(SYS_MAP_CTC_GPORT_TO_DRV_LPORT(port_assign.inter_port)
                             + (SYS_MAP_DRV_LPORT_TO_SLICE(drv_src_lport) * 256));

        sys_internal_gport = CTC_MAP_LPORT_TO_GPORT(gchip, sys_internal_lport);

        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "source gport:0x%04X, dest gport:0x%04X, internal lport = %u!\n",\
                                               sys_src_gport, sys_dst_gport, sys_internal_lport);

        /* map network port to internal port */
        drv_internal_lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(sys_internal_lport);
        RET_PROCESS_WITH_ERROR(_sys_usw_port_maping_to_local_phy_port(lchip, sys_src_gport, drv_internal_lport));

        if (CTC_PORT_LBK_MODE_EFM == p_port_lbk->lbk_mode)
        {
            /*efm loopback*/
            uint8 chan = SYS_GET_CHANNEL_ID(lchip, sys_src_gport);
            RET_PROCESS_WITH_ERROR(sys_usw_add_port_to_channel(lchip, drv_internal_lport, chan));
            RET_PROCESS_WITH_ERROR(sys_usw_add_port_to_channel(lchip, drv_src_lport, MCHIP_CAP(SYS_CAP_CHANID_DROP)));
            /*get reflect nexthop*/
            RET_PROCESS_WITH_ERROR(sys_usw_nh_get_reflective_dsfwd_offset(lchip, &dsfwd_offset));
        }
        else if(CTC_PORT_LBK_MODE_CC == p_port_lbk->lbk_mode)
        {
            /*
            if (src_gport == dst_gport)
            {
                RET_PROCESS_WITH_ERROR(sys_usw_add_port_to_channel(lchip, inter_gport, src_lport));
                RET_PROCESS_WITH_ERROR(sys_usw_add_port_to_channel(lchip, src_gport, MCHIP_CAP(SYS_CAP_CHANID_DROP)));
            }
            */

            if (CTC_PORT_LBK_TYPE_SWAP_MAC == p_port_lbk->lbk_type)
            {
                nh_param.type           = CTC_MISC_NH_TYPE_FLEX_EDIT_HDR;
                nh_param.gport          = sys_dst_gport;
                nh_param.dsnh_offset    = 0;

                nh_param.misc_param.l2edit.is_reflective    = 0;
                nh_param.misc_param.l2edit.flag             |= CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR;
                nh_param.misc_param.l2edit.flag             |= CTC_MISC_NH_FLEX_EDIT_SWAP_MACDA;

                sys_usw_nh_add_misc(lchip, &nhid, &nh_param, TRUE);
                sys_usw_nh_get_dsfwd_offset(lchip, nhid, &dsfwd_offset, 0);

            }
            else if (CTC_PORT_LBK_TYPE_BYPASS == p_port_lbk->lbk_type)
            {
                uint32 bypass_nhid    = 0;
                sys_usw_l2_get_ucast_nh(lchip, sys_dst_gport, CTC_NH_PARAM_BRGUC_SUB_TYPE_BYPASS, &bypass_nhid);

                sys_usw_nh_get_dsfwd_offset(lchip, bypass_nhid, &dsfwd_offset, 0);

            }
        }

        /*set lbk port property*/
        ret = sys_usw_port_set_lbk_port_property(lchip, p_port_lbk, sys_internal_lport, dsfwd_offset);

        if (CTC_E_NONE != ret)
        {
            if (CTC_PORT_LBK_MODE_EFM == p_port_lbk->lbk_mode)
            {
                /*resverv channel*/
                chan = SYS_GET_CHANNEL_ID(lchip, sys_src_gport);
                RET_PROCESS_WITH_ERROR(sys_usw_remove_port_from_channel(lchip, drv_internal_lport, chan));
                RET_PROCESS_WITH_ERROR(sys_usw_remove_port_from_channel(lchip, drv_src_lport, MCHIP_CAP(SYS_CAP_CHANID_DROP)));
                RET_PROCESS_WITH_ERROR(sys_usw_add_port_to_channel(lchip, drv_src_lport, chan));
            }
            else if(CTC_PORT_LBK_MODE_CC == p_port_lbk->lbk_mode)
            { /*release nexthop*/
                if (nhid)
                {
                    sys_usw_nh_remove_misc(lchip, nhid);
                }
            }

            /* Release internal port */
            port_assign.type        = CTC_INTERNAL_PORT_TYPE_FWD;
            port_assign.gchip       = gchip;
            sys_internal_lport = SYS_MAP_DRV_LPORT_TO_SYS_LPORT((SYS_MAP_CTC_GPORT_TO_DRV_LPORT(port_assign.inter_port) & 0xFF));
            port_assign.inter_port  = sys_internal_lport;
            if (CTC_PORT_LBK_MODE_EFM == p_port_lbk->lbk_mode)
            {
                port_assign.fwd_gport       = sys_src_gport;
            }
            else if(CTC_PORT_LBK_MODE_CC == p_port_lbk->lbk_mode)
            {
                port_assign.fwd_gport       = sys_dst_gport;
            }
            CTC_ERROR_RETURN(sys_usw_internal_port_release(lchip, &port_assign));

            return ret;
        }

        p_port_lbk->lbk_gport = sys_internal_gport;

        /*save the port*/
        PORT_LOCK;
        p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(sys_src_gport)].inter_lport = sys_internal_lport;
        p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(sys_src_gport)].lbk_en   = 1;
        p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(sys_src_gport)].nhid     = nhid;
        PORT_UNLOCK;
    }
    else
    {
        if (!p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(sys_src_gport)].lbk_en)
        {
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
			return CTC_E_NOT_EXIST;

        }

        PORT_LOCK;
        p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(sys_src_gport)].lbk_en = 0;
        sys_internal_lport = p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(sys_src_gport)].inter_lport;
        nhid        = p_usw_port_master[lchip]->igs_port_prop[CTC_MAP_GPORT_TO_LPORT(sys_src_gport)].nhid;
        PORT_UNLOCK;

        drv_internal_lport = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(sys_internal_lport);
        CTC_ERROR_RETURN(_sys_usw_port_maping_to_local_phy_port(lchip, sys_src_gport, drv_src_lport));
        sys_internal_gport = CTC_MAP_LPORT_TO_GPORT(gchip, sys_internal_lport);
        CTC_ERROR_RETURN(sys_usw_port_set_lbk_port_property(lchip, p_port_lbk, sys_internal_lport, 0));

        /* valid internal port not suppose to be 0 */
        if (sys_internal_lport)
        {
            sys_internal_gport = CTC_MAP_LPORT_TO_GPORT(gchip, sys_internal_lport);

            if (CTC_PORT_LBK_MODE_EFM == p_port_lbk->lbk_mode)
            {
                chan = SYS_GET_CHANNEL_ID(lchip, sys_src_gport);
                CTC_ERROR_RETURN(sys_usw_remove_port_from_channel(lchip, drv_internal_lport, chan));
                CTC_ERROR_RETURN(sys_usw_remove_port_from_channel(lchip, drv_internal_lport, MCHIP_CAP(SYS_CAP_CHANID_DROP)));
                CTC_ERROR_RETURN(sys_usw_add_port_to_channel(lchip, drv_src_lport, chan));
            }
            else if(CTC_PORT_LBK_MODE_CC == p_port_lbk->lbk_mode)
            {
                /*
                if (src_gport == dst_gport)
                {
                    CTC_ERROR_RETURN(sys_usw_remove_port_from_channel(lchip, inter_gport, src_lport));
                    CTC_ERROR_RETURN(sys_usw_remove_port_from_channel(lchip, src_gport, MCHIP_CAP(SYS_CAP_CHANID_DROP)));
                    CTC_ERROR_RETURN(sys_usw_add_port_to_channel(lchip, src_gport, src_lport));
                }
                */
                /*release nexthop*/
                if (nhid)
                {
                    sys_usw_nh_remove_misc(lchip, nhid);
                }
            }

            /* Release internal port */
            port_assign.type        = CTC_INTERNAL_PORT_TYPE_FWD;
            port_assign.gchip       = gchip;
            sys_internal_lport = SYS_MAP_DRV_LPORT_TO_SYS_LPORT((SYS_MAP_SYS_LPORT_TO_DRV_LPORT(sys_internal_lport) & 0xFF));
            port_assign.inter_port  = sys_internal_lport;
            if (CTC_PORT_LBK_MODE_EFM == p_port_lbk->lbk_mode)
            {
                port_assign.fwd_gport       = sys_src_gport;
            }
            else if(CTC_PORT_LBK_MODE_CC == p_port_lbk->lbk_mode)
            {
                port_assign.fwd_gport       = sys_dst_gport;
            }
            CTC_ERROR_RETURN(sys_usw_internal_port_release(lchip, &port_assign));
        }
    }

    return CTC_E_NONE;
}

/* Port support 3 loopback modes, efm loopback crossconnect loopback and TAP. Note, when TAP, chip divide mac into groups, */
/* if use one mac as TAP dest, all mac included in the group which has the TAP dest won't do normal forwarding. */
/* Src_mac and dst_mac should have the same speed mode, src_mac and dest_mac should one-to-one mapping. */
/* Meanwhile TAP src_mac dst_mac should both in the same slice. */
/* Slice0:                                              Slice1:                                                       */
/* | GROUP |  Txqm0  |  Txqm1  |  Cxqm0  |  Cxqm1  |    | GROUP |  Txqm0  |  Txqm1  |  Cxqm0  |  Cxqm1  |    */
/* |-------|---------|---------|---------|---------|    |-------|---------|---------|---------|---------|    */
/* |  MAC  |   0~15  |  16~31  |  32~39  |  48~55  |    |  MAC  |   0~15  |  16~31  |  32~39  |  48~55  |    */

int32
sys_usw_port_set_loopback(uint8 lchip, ctc_port_lbk_param_t* p_port_lbk)
{
    /* sanity check */
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_port_lbk);

    /* debug */
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,"src_gport%d, dst_gport:%d, lbk_enable:%d, lbk_mode:%d\n",p_port_lbk->src_gport,
                            p_port_lbk->dst_gport, p_port_lbk->lbk_enable, p_port_lbk->lbk_mode);

    /* set loopback by lbk_mode */
    if(CTC_PORT_LBK_MODE_TAP != p_port_lbk->lbk_mode)
    {
        CTC_ERROR_RETURN(_sys_usw_port_set_loopback(lchip, p_port_lbk));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_port_set_default_vlan(uint8 lchip, uint32 gport, uint32 value)
{
    uint16  lport = 0;
    uint32  cmd = 0;
    uint32  default_vlan_id = value;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set gport:0x%04X, default vlan:%d \n", gport, value);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_defaultVlanId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &default_vlan_id));

    cmd = _sys_usw_port_is_support_glb_port(lchip) ? \
        DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_defaultVlanId_f) : DRV_IOW(DsDestPort_t, DsDestPort_defaultVlanId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &default_vlan_id));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_port_set_mux_type(uint8 lchip, uint16 gport, uint32 value)
{
    uint16 chan_id = 0;
    uint32 cmd  = 0;
    uint8 mux_type = 0;
    uint32 field_val = 0;

    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (chan_id == 0xff)
    {
        return CTC_E_INVALID_PARAM;
    }

    switch (value)
    {
    case CTC_PORT_MUX_TYPE_NONE:
        mux_type = 0;
        break;
    case CTC_PORT_MUX_TYPE_CLOUD_L2_HDR:
        mux_type = (DRV_IS_DUET2(lchip))?7:8;
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOW(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f);
    field_val = mux_type;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_port_get_mux_type(uint8 lchip, uint16 gport, uint32* p_value)
{
    uint16 chan_id = 0;
    uint32 cmd  = 0;
    uint32 field_val = 0;

    CTC_PTR_VALID_CHECK(p_value);
    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (chan_id == 0xff)
    {
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOR(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &field_val));
    if (!field_val)
    {
        *p_value = CTC_PORT_MUX_TYPE_NONE;
    }
    else
    {
        if (((field_val == 7) && DRV_IS_DUET2(lchip)) || ((field_val == 8) && !DRV_IS_DUET2(lchip)))
        {
            *p_value = CTC_PORT_MUX_TYPE_CLOUD_L2_HDR;
        }
    }

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_port_set_phy_init(uint8 lchip, uint16 gport, uint32 value)
{
    uint16  lport = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set gport:0x%04X phy init\n", gport);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    SYS_MAX_PHY_PORT_CHECK(lport);

    CTC_ERROR_RETURN(sys_usw_peri_set_phy_prop(lchip, lport, CTC_PORT_PROP_PHY_INIT, value));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_port_set_raw_packet_type(uint8 lchip, uint32 gport, uint32 value)
{
    uint16 lport = 0;
    uint32 cmd = 0;
    DsPhyPort_m ds_phy_port;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set gport:0x%04X, packet type:%d \n", gport, value);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    CTC_MAX_VALUE_CHECK(value, CTC_PORT_RAW_PKT_IPV6);

    sal_memset(&ds_phy_port, 0, sizeof(DsPhyPort_m));

    cmd = DRV_IOR(DsPhyPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_phy_port));

    if (CTC_PORT_RAW_PKT_NONE == value)
    {
        SetDsPhyPort(V, packetTypeValid_f, &ds_phy_port, 0);
    }
    else
    {
        SetDsPhyPort(V, packetTypeValid_f, &ds_phy_port, 1);

        switch (value)
        {
        case CTC_PORT_RAW_PKT_ETHERNET:
            SetDsPhyPort(V, packetType_f, &ds_phy_port, 0);
            break;

        case CTC_PORT_RAW_PKT_IPV4:
            SetDsPhyPort(V, packetType_f, &ds_phy_port, 1);
            break;

        case CTC_PORT_RAW_PKT_IPV6:
            SetDsPhyPort(V, packetType_f, &ds_phy_port, 3);
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }
    }

    cmd = DRV_IOW(DsPhyPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_phy_port));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_port_get_raw_packet_type(uint8 lchip, uint32 gport, uint32* p_value)
{
    uint16  lport = 0;
    uint32  cmd = 0;
    uint32  packet_type_valid = 0;
    uint32  packet_type = 0;
    DsPhyPort_m ds_phy_port;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*Sanity check*/
    CTC_PTR_VALID_CHECK(p_value);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    sal_memset(&ds_phy_port, 0, sizeof(DsPhyPort_m));
    cmd = DRV_IOR(DsPhyPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_phy_port));

    CTC_ERROR_RETURN(GetDsPhyPort(A, packetTypeValid_f, &ds_phy_port, &packet_type_valid));
    CTC_ERROR_RETURN(GetDsPhyPort(A, packetType_f, &ds_phy_port, &packet_type));

    if (0 == packet_type_valid)
    {
        *p_value = CTC_PORT_RAW_PKT_NONE;
    }
    else
    {
        switch (packet_type)
        {
        case 0:
            *p_value = CTC_PORT_RAW_PKT_ETHERNET;
            break;

        case 1:
            *p_value = CTC_PORT_RAW_PKT_IPV4;
            break;

        case 3:
            *p_value = CTC_PORT_RAW_PKT_IPV6;
            break;
        }
    }

    return CTC_E_NONE;
}

#if 0
int32
_sys_usw_port_set_rx_pause_type(uint8 lchip, uint32 gport, uint32 pasue_type)
{
#ifdef NEVER

    uint16 lport = 0;
    int32  step = 0;
    uint32 cmd = 0;
    uint32 tbl_net_rx_pause_ctl[2] = {NetRxPauseCtl0_t, NetRxPauseCtl1_t};
    NetRxPauseCtl0_m net_rx_pause_ctl;
    sys_datapath_lport_attr_t* p_port_cap = NULL;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, CTC_MAP_GPORT_TO_LPORT(gport), &p_port_cap));
    if (p_port_cap->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " MAC is not used \n");
			return CTC_E_INVALID_CONFIG;

    }

    PORT_LOCK;
    sal_memset(&net_rx_pause_ctl, 0, sizeof(NetRxPauseCtl0_m));
    cmd = DRV_IOR(tbl_net_rx_pause_ctl[p_port_cap->slice_id], DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_UNLOCK((DRV_IOCTL(lchip, 0, cmd, &net_rx_pause_ctl)), p_usw_port_master[lchip]->p_port_mutex);

    step = NetRxPauseCtl0_cfgNormPauseEn1_f - NetRxPauseCtl0_cfgNormPauseEn0_f;
    if (pasue_type & SYS_PORT_PAUSE_FRAME_TYPE_NORMAL)
    {
        SetNetRxPauseCtl0(V, cfgNormPauseEn0_f + (step * ((p_port_cap->mac_id) >> 4)), &net_rx_pause_ctl, 1);
    }

    step = NetRxPauseCtl0_cfgPfcPauseEn1_f - NetRxPauseCtl0_cfgPfcPauseEn0_f;
    if (pasue_type & SYS_PORT_PAUSE_FRAME_TYPE_PFC)
    {
        SetNetRxPauseCtl0(V, cfgPfcPauseEn0_f + (step * ((p_port_cap->mac_id) >> 4)), &net_rx_pause_ctl, 1);
    }

    cmd = DRV_IOW(tbl_net_rx_pause_ctl[p_port_cap->slice_id], DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_UNLOCK((DRV_IOCTL(lchip, 0, cmd, &net_rx_pause_ctl)), p_usw_port_master[lchip]->p_port_mutex);
    PORT_UNLOCK;

    return CTC_E_NONE;
#endif /* never */
    return CTC_E_NONE;
}
#endif
int32
_sys_usw_port_get_aps_failover_en(uint8 lchip, uint32 gport, uint32* enable)
{
    DsApsChannelMap_m ds_aps_channel_map;
    uint8 channel_id = 0;
    uint32 cmd =0;

    /*uint8 lport = 0;*/

    SYS_USW_PORT_INIT_CHECK();

    /*lport = CTC_MAP_GPORT_TO_LPORT(gport);*/

    /*gport get channel id.*/
    channel_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (0xFF == channel_id)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
			return CTC_E_INVALID_PORT;

    }

    cmd = DRV_IOR(DsApsChannelMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &ds_aps_channel_map));

    *enable = GetDsApsChannelMap(V, linkChangeEn_f, &ds_aps_channel_map);

    return CTC_E_NONE;
}

int32
_sys_usw_port_get_linkagg_failover_en(uint8 lchip, uint32 gport, uint32* enable)
{
    DsLinkAggregateChannel_m linkagg_channel;
    uint8 channel_id = 0;
    uint32 cmd =0;

    SYS_USW_PORT_INIT_CHECK();

    /*lport = CTC_MAP_GPORT_TO_LPORT(gport);*/

    /*gport get channel id.*/
    channel_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (0xFF == channel_id)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
			return CTC_E_INVALID_PORT;

    }

    /*get ds_aps_channel_map.link_change_en for scio scan down notify aps model.*/
    cmd = DRV_IOR(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &linkagg_channel));

    *enable =  GetDsLinkAggregateChannel(V, linkChangeEn_f, &linkagg_channel);

    return CTC_E_NONE;
}

int32
_sys_usw_port_set_linkagg_failover_en(uint8 lchip, uint32 gport, bool enable)
{
    uint32 channel_id = 0;
    uint32 cmd =0;
    DsLinkAggregateChannel_m linkagg_channel;

    SYS_USW_PORT_INIT_CHECK();

    sal_memset(&linkagg_channel, 0, sizeof(DsLinkAggregateChannel_m));

    /*lport = CTC_MAP_GPORT_TO_LPORT(gport);*/

    /*gport get channel id.*/
    channel_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (0xFF == channel_id)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
			return CTC_E_INVALID_PORT;

    }

    /*set linkagg_channel.link_change_en  for scio scan down notify linkagg model.*/
    cmd = DRV_IOR(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &linkagg_channel));
    SetDsLinkAggregateChannel(V, groupEn_f, &linkagg_channel, 1);
    SetDsLinkAggregateChannel(V, linkChangeEn_f, &linkagg_channel, enable);
    cmd = DRV_IOW(DsLinkAggregateChannel_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &linkagg_channel));

    return CTC_E_NONE;
}

int32
_sys_usw_port_set_aps_failover_en(uint8 lchip, uint32 gport, bool enable)
{
    DsApsChannelMap_m ds_aps_channel_map;
    uint32 channel_id = 0;
    uint32 cmd =0;

    uint16 lport = 0;

    SYS_USW_PORT_INIT_CHECK();

    /*lport = CTC_MAP_GPORT_TO_LPORT(gport);*/

     /*linkagg port check return.*/
    if(CTC_IS_LINKAGG_PORT(gport))
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Aps] Aps only support hw-based aps for phyport \n");
			return CTC_E_NOT_SUPPORT;

    }

    /*gport get channel id.*/
    channel_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (0xFF == channel_id)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
			return CTC_E_INVALID_PORT;

    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    /*Set ds_aps_channel_map.link_change_en for scio scan down notify aps model.*/
    cmd = DRV_IOR(DsApsChannelMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &ds_aps_channel_map));
    SetDsApsChannelMap(V, linkChangeEn_f, &ds_aps_channel_map, enable);
    SetDsApsChannelMap(V, localPhyPort_f, &ds_aps_channel_map, lport);
    cmd = DRV_IOW(DsApsChannelMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, channel_id, cmd, &ds_aps_channel_map));


    return CTC_E_NONE;
}

int32
_sys_usw_port_get_aps_select_grp_id(uint8 lchip, uint32 gport, uint32* grp_id)
{
    uint32 cmd = 0;
    uint32 value = 0;

    uint16 lport = 0;
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_apsSelectValid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
    if (0 == value)
    {
        *grp_id = 0xFFFFFFFF;
    }
    else
    {
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_apsSelectGroupId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, grp_id));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_port_get_wlan_port_type(uint8 lchip, uint32 gport, uint32* p_value)
{
    uint16 lport     = 0;
    uint16 tmp_lport = 0;
    uint32 cmd       = 0;
    uint32 field = 0;

    SYS_PORT_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    *p_value = CTC_PORT_WLAN_PORT_TYPE_NONE;
    cmd = DRV_IOR(IpeE2iLoopCtl_t, IpeE2iLoopCtl_portArray_0_loopbackPort_f+lport);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));
    tmp_lport = field;
    if (0 != tmp_lport)
    {
        *p_value = CTC_PORT_WLAN_PORT_TYPE_DECAP_WITHOUT_DECRYPT;
        cmd = DRV_IOR(IpeE2iLoopCtl_t, IpeE2iLoopCtl_portArray_0_loopbackPort_f+tmp_lport+1);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));
        if (field == tmp_lport)
        {
            *p_value = CTC_PORT_WLAN_PORT_TYPE_DECAP_WITH_DECRYPT;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_port_set_wlan_port_type(uint8 lchip, uint32 gport, uint32 value)
{
    uint16 lport     = 0;
    uint32 cmd       = 0;
    uint32 field = 0;
    uint16 inter_lport = 0;
    uint8 gchip_id = 0;
    uint32 excp2_en = 0;
    uint32 sub_index = 0;
    uint32 wlan_type = 0;
    DsPhyPortExt_m phy_port_ext;
    DsSrcPort_m src_port;
    ctc_internal_port_assign_para_t port_assign;

    SYS_PORT_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    CTC_ERROR_RETURN(_sys_usw_port_get_wlan_port_type(lchip, gport, &wlan_type));
    if ((wlan_type != CTC_PORT_WLAN_PORT_TYPE_NONE) && (value != CTC_PORT_WLAN_PORT_TYPE_NONE))
    {
        return CTC_E_IN_USE;
    }
    sal_memset(&port_assign, 0, sizeof(ctc_internal_port_assign_para_t));

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip_id));

    cmd = DRV_IOR(IpeBpduEscapeCtl_t, IpeBpduEscapeCtl_eapolExceptionSubIndex_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sub_index));

    switch (value)
    {
        case CTC_PORT_WLAN_PORT_TYPE_DECAP_WITH_DECRYPT:
            port_assign.type = CTC_INTERNAL_PORT_TYPE_WLAN;
            port_assign.gchip = gchip_id;
            port_assign.fwd_gport = gport;
            CTC_ERROR_RETURN(sys_usw_internal_port_allocate(lchip, &port_assign));
            inter_lport = port_assign.inter_port;
            field = inter_lport;
            cmd = DRV_IOW(IpeE2iLoopCtl_t, IpeE2iLoopCtl_portArray_0_loopbackPort_f+lport);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));
            cmd = DRV_IOW(IpeE2iLoopCtl_t, IpeE2iLoopCtl_portArray_0_loopbackPort_f+inter_lport+1);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

            cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port_ext));
            field = 1;
            SetDsPhyPortExt(V, capwapHashEn1_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, capwapHashEn2_f, &phy_port_ext, field);
            field = 0x1d;
            SetDsPhyPortExt(V, userIdPortHash1Type_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, userIdPortHash2Type_f, &phy_port_ext, field);
            field = 1;
            SetDsPhyPortExt(V, userIdTcam1En_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, userIdTcam2En_f, &phy_port_ext, field);
            field = 3;
            SetDsPhyPortExt(V, userIdTcam1Type_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, userIdTcam2Type_f, &phy_port_ext, field);
            field = 0;
            SetDsPhyPortExt(V, tcam1IsSclFlow_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam2IsSclFlow_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam1IsUserId_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam2IsUserId_f, &phy_port_ext, field);
            cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port_ext));

            sal_memset(&phy_port_ext, 0, sizeof(phy_port_ext));
            cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_lport+1, cmd, &phy_port_ext));
            field = 1;
            SetDsPhyPortExt(V, capwapHashEn1_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, capwapHashEn2_f, &phy_port_ext, field);
            field = 0x1d;
            SetDsPhyPortExt(V, userIdPortHash1Type_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, userIdPortHash2Type_f, &phy_port_ext, field);
            field = 1;
            SetDsPhyPortExt(V, userIdTcam1En_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, userIdTcam2En_f, &phy_port_ext, field);
            field = 3;
            SetDsPhyPortExt(V, userIdTcam1Type_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, userIdTcam2Type_f, &phy_port_ext, field);
            field = 0;
            SetDsPhyPortExt(V, tcam1IsSclFlow_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam2IsSclFlow_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam1IsUserId_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam2IsUserId_f, &phy_port_ext, field);
            cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_lport+1, cmd, &phy_port_ext));

            sal_memset(&phy_port_ext, 0, sizeof(phy_port_ext));
            cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_lport, cmd, &phy_port_ext));

            excp2_en = GetDsPhyPortExt(V, exception2En_f, &phy_port_ext);
            CTC_SET_FLAG(excp2_en, (1 << sub_index));
            SetDsPhyPortExt(V, exception2En_f, &phy_port_ext, excp2_en);

            field = 1;
            SetDsPhyPortExt(V, tcamUseLogicPort_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam1UseLogicPort_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam2UseLogicPort_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam3UseLogicPort_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam4UseLogicPort_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, userId1UseLogicPort_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, userId2UseLogicPort_f, &phy_port_ext, field);
            field = 1;
            SetDsPhyPortExt(V, userIdTcam1En_f, &phy_port_ext, field);
            field = 3;
            SetDsPhyPortExt(V, userIdTcam1Type_f, &phy_port_ext, field);
            field = 0;
            SetDsPhyPortExt(V, tcam1IsSclFlow_f, &phy_port_ext, field);
            field = 1;
            SetDsPhyPortExt(V, tcam1IsUserId_f, &phy_port_ext, field);
            field = 1;
            SetDsPhyPortExt(V, userIdTcam2En_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam2IsUserId_f, &phy_port_ext, field);
            field = 0;
            SetDsPhyPortExt(V, userIdTcam2Type_f, &phy_port_ext, field);
            field = 0;
            SetDsPhyPortExt(V, useDefaultLogicSrcPort_f, &phy_port_ext, field);
            cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_lport, cmd, &phy_port_ext));

            sal_memset(&src_port, 0, sizeof(src_port));
            cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_lport+1, cmd, &src_port));
            SetDsSrcPort(V, interfaceId_f, &src_port, MCHIP_CAP(SYS_CAP_L3IF_RSV_L3IF_ID));
            SetDsSrcPort(V, routedPort_f, &src_port, 1);
            SetDsSrcPort(V, addDefaultVlanDisable_f, &src_port, 1);
            cmd = DRV_IOW(DsSrcPort_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_lport+1, cmd, &src_port));

            sys_usw_port_set_property(lchip, CTC_MAP_LPORT_TO_GPORT(gchip_id, inter_lport), CTC_PORT_PROP_PORT_EN,1);
            sys_usw_port_set_property(lchip, CTC_MAP_LPORT_TO_GPORT(gchip_id, inter_lport+1), CTC_PORT_PROP_PORT_EN,1);
            p_usw_port_master[lchip]->network_port[inter_lport] = lport;
            p_usw_port_master[lchip]->network_port[inter_lport+1] = lport;
            break;

        case CTC_PORT_WLAN_PORT_TYPE_DECAP_WITHOUT_DECRYPT:
            port_assign.type = CTC_INTERNAL_PORT_TYPE_FWD;
            port_assign.gchip = gchip_id;
            port_assign.fwd_gport = gport;
            CTC_ERROR_RETURN(sys_usw_internal_port_allocate(lchip, &port_assign));
            inter_lport = port_assign.inter_port;
            field = inter_lport;
            cmd = DRV_IOW(IpeE2iLoopCtl_t, IpeE2iLoopCtl_portArray_0_loopbackPort_f+lport);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

            cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port_ext));
            field = 1;
            SetDsPhyPortExt(V, capwapHashEn1_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, capwapHashEn2_f, &phy_port_ext, field);
            field = 0x1d;
            SetDsPhyPortExt(V, userIdPortHash1Type_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, userIdPortHash2Type_f, &phy_port_ext, field);
            field = 1;
            SetDsPhyPortExt(V, userIdTcam1En_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, userIdTcam2En_f, &phy_port_ext, field);
            field = 3;
            SetDsPhyPortExt(V, userIdTcam1Type_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, userIdTcam2Type_f, &phy_port_ext, field);
            field = 0;
            SetDsPhyPortExt(V, tcam1IsSclFlow_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam2IsSclFlow_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam1IsUserId_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam2IsUserId_f, &phy_port_ext, field);
            cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port_ext));

            sal_memset(&phy_port_ext, 0, sizeof(phy_port_ext));
            cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_lport, cmd, &phy_port_ext));

            excp2_en = GetDsPhyPortExt(V, exception2En_f, &phy_port_ext);
            CTC_SET_FLAG(excp2_en, (1 << sub_index));
            SetDsPhyPortExt(V, exception2En_f, &phy_port_ext, excp2_en);

            field = 1;
            SetDsPhyPortExt(V, tcamUseLogicPort_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam1UseLogicPort_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam2UseLogicPort_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam3UseLogicPort_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam4UseLogicPort_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, userId1UseLogicPort_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, userId2UseLogicPort_f, &phy_port_ext, field);
            field = 1;
            SetDsPhyPortExt(V, userIdTcam1En_f, &phy_port_ext, field);
            field = 3;
            SetDsPhyPortExt(V, userIdTcam1Type_f, &phy_port_ext, field);
            field = 0;
            SetDsPhyPortExt(V, tcam1IsSclFlow_f, &phy_port_ext, field);
            field = 1;
            SetDsPhyPortExt(V, tcam1IsUserId_f, &phy_port_ext, field);
            field = 1;
            SetDsPhyPortExt(V, userIdTcam2En_f, &phy_port_ext, field);
            SetDsPhyPortExt(V, tcam2IsUserId_f, &phy_port_ext, field);
            field = (!DRV_IS_DUET2(lchip)) ? 3 : 0;
            SetDsPhyPortExt(V, userIdTcam2Type_f, &phy_port_ext, field);
            field = 0;
            SetDsPhyPortExt(V, useDefaultLogicSrcPort_f, &phy_port_ext, field);
            cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_lport, cmd, &phy_port_ext));

            sys_usw_port_set_property(lchip, CTC_MAP_LPORT_TO_GPORT(gchip_id, inter_lport), CTC_PORT_PROP_PORT_EN,1);
            p_usw_port_master[lchip]->network_port[inter_lport] = lport;

            break;

        case CTC_PORT_WLAN_PORT_TYPE_NONE:
            port_assign.gchip = gchip_id;
            port_assign.fwd_gport = gport;
            cmd = DRV_IOR(IpeE2iLoopCtl_t, IpeE2iLoopCtl_portArray_0_loopbackPort_f+lport);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));
            inter_lport = field;
            if (0 != inter_lport)
            {
                port_assign.type = CTC_INTERNAL_PORT_TYPE_FWD;
                port_assign.inter_port = inter_lport;
                cmd = DRV_IOR(IpeE2iLoopCtl_t, IpeE2iLoopCtl_portArray_0_loopbackPort_f+inter_lport+1);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));
                if (field == inter_lport)
                {
                    port_assign.type = CTC_INTERNAL_PORT_TYPE_WLAN;
                    field = 0;
                    cmd = DRV_IOW(IpeE2iLoopCtl_t, IpeE2iLoopCtl_portArray_0_loopbackPort_f+lport);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));
                    cmd = DRV_IOW(IpeE2iLoopCtl_t, IpeE2iLoopCtl_portArray_0_loopbackPort_f+inter_lport+1);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

                    sal_memset(&phy_port_ext, 0, sizeof(phy_port_ext));
                    cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port_ext));
                    field = 0;
                    SetDsPhyPortExt(V, capwapHashEn1_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, capwapHashEn2_f, &phy_port_ext, field);
                    field = 0;
                    SetDsPhyPortExt(V, userIdPortHash1Type_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userIdPortHash2Type_f, &phy_port_ext, field);
                    field = 0;
                    SetDsPhyPortExt(V, userIdTcam1En_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userIdTcam2En_f, &phy_port_ext, field);
                    field = 0;
                    SetDsPhyPortExt(V, userIdTcam1Type_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userIdTcam2Type_f, &phy_port_ext, field);
                    cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port_ext));

                    sal_memset(&phy_port_ext, 0, sizeof(phy_port_ext));
                    cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_lport+1, cmd, &phy_port_ext));
                    field = 0;
                    SetDsPhyPortExt(V, capwapHashEn1_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, capwapHashEn2_f, &phy_port_ext, field);
                    field = 0;
                    SetDsPhyPortExt(V, userIdPortHash1Type_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userIdPortHash2Type_f, &phy_port_ext, field);
                    field = 0;
                    SetDsPhyPortExt(V, userIdTcam1En_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userIdTcam2En_f, &phy_port_ext, field);
                    field = 0;
                    SetDsPhyPortExt(V, userIdTcam1Type_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userIdTcam2Type_f, &phy_port_ext, field);
                    cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_lport+1, cmd, &phy_port_ext));

                    sal_memset(&phy_port_ext, 0, sizeof(phy_port_ext));
                    cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_lport, cmd, &phy_port_ext));

                    excp2_en = GetDsPhyPortExt(V, exception2En_f, &phy_port_ext);
                    CTC_UNSET_FLAG(excp2_en, (1 << sub_index));
                    SetDsPhyPortExt(V, exception2En_f, &phy_port_ext, excp2_en);

                    field = 0;
                    SetDsPhyPortExt(V, tcamUseLogicPort_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, tcam1UseLogicPort_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, tcam2UseLogicPort_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, tcam3UseLogicPort_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, tcam4UseLogicPort_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userId1UseLogicPort_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userId2UseLogicPort_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userIdTcam1En_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userIdTcam1Type_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, tcam1IsSclFlow_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, tcam1IsUserId_f, &phy_port_ext, field);
                    field = 1;
                    SetDsPhyPortExt(V, useDefaultLogicSrcPort_f, &phy_port_ext, field);
                    cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_lport, cmd, &phy_port_ext));

                    sal_memset(&src_port, 0, sizeof(src_port));
                    cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_lport+1, cmd, &src_port));
                    SetDsSrcPort(V, interfaceId_f, &src_port, 0);
                    SetDsSrcPort(V, routedPort_f, &src_port, 0);
                    SetDsSrcPort(V, addDefaultVlanDisable_f, &src_port, 0);
                    cmd = DRV_IOW(DsSrcPort_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_lport+1, cmd, &src_port));

                    p_usw_port_master[lchip]->network_port[inter_lport] = SYS_INVALID_LOCAL_PHY_PORT;
                    p_usw_port_master[lchip]->network_port[inter_lport+1] = SYS_INVALID_LOCAL_PHY_PORT;
                }
                else
                {
                    field = 0;
                    cmd = DRV_IOW(IpeE2iLoopCtl_t, IpeE2iLoopCtl_portArray_0_loopbackPort_f+lport);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

                    sal_memset(&phy_port_ext, 0, sizeof(phy_port_ext));
                    cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port_ext));
                    field = 0;
                    SetDsPhyPortExt(V, capwapHashEn1_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, capwapHashEn2_f, &phy_port_ext, field);
                    field = 0;
                    SetDsPhyPortExt(V, userIdPortHash1Type_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userIdPortHash2Type_f, &phy_port_ext, field);
                    field = 0;
                    SetDsPhyPortExt(V, userIdTcam1En_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userIdTcam2En_f, &phy_port_ext, field);
                    field = 0;
                    SetDsPhyPortExt(V, userIdTcam1Type_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userIdTcam2Type_f, &phy_port_ext, field);
                    cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port_ext));

                    sal_memset(&phy_port_ext, 0, sizeof(phy_port_ext));
                    cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_lport, cmd, &phy_port_ext));
                    field = 0;
                    SetDsPhyPortExt(V, tcamUseLogicPort_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, tcam1UseLogicPort_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, tcam2UseLogicPort_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, tcam3UseLogicPort_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, tcam4UseLogicPort_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userId1UseLogicPort_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userId2UseLogicPort_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userIdTcam1En_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, userIdTcam1Type_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, tcam1IsSclFlow_f, &phy_port_ext, field);
                    SetDsPhyPortExt(V, tcam1IsUserId_f, &phy_port_ext, field);

                    excp2_en = GetDsPhyPortExt(V, exception2En_f, &phy_port_ext);
                    CTC_UNSET_FLAG(excp2_en, (1 << sub_index));
                    SetDsPhyPortExt(V, exception2En_f, &phy_port_ext, excp2_en);

                    cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, inter_lport, cmd, &phy_port_ext));

                    p_usw_port_master[lchip]->network_port[inter_lport] = SYS_INVALID_LOCAL_PHY_PORT;
                }
                CTC_ERROR_RETURN(sys_usw_internal_port_release (lchip, &port_assign));
            }
            break;
        default:
            return CTC_E_INVALID_PARAM;
            break;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_port_set_wlan_port_route(uint8 lchip, uint32 gport, uint32 value)
{
    uint16 lport     = 0;
    uint16 tmp_lport     = 0;
    uint32 cmd       = 0;
    uint32 field = 0;
    DsSrcPort_m src_port;
    DsPhyPortExt_m phy_port_ext;
    DsPhyPort_m phy_port;

    SYS_PORT_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    cmd = DRV_IOR(IpeE2iLoopCtl_t, IpeE2iLoopCtl_portArray_0_loopbackPort_f+lport);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));
    tmp_lport = field;
    if (0 == tmp_lport)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (value)
    {
        sal_memset(&src_port, 0, sizeof(src_port));
        cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_lport, cmd, &src_port));
        SetDsSrcPort(V, interfaceId_f, &src_port, MCHIP_CAP(SYS_CAP_L3IF_RSV_L3IF_ID));
        SetDsSrcPort(V, routedPort_f, &src_port, 1);
        SetDsSrcPort(V, addDefaultVlanDisable_f, &src_port, 1);
        cmd = DRV_IOW(DsSrcPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_lport, cmd, &src_port));

        sal_memset(&phy_port_ext, 0, sizeof(phy_port_ext));
        cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_lport, cmd, &phy_port_ext));
        SetDsPhyPortExt(V, defaultVlanId_f, &phy_port_ext, 0);
        cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_lport, cmd, &phy_port_ext));

        sal_memset(&phy_port, 0, sizeof(phy_port));
        SetDsPhyPort(V, fwdHashGenDis_f, &phy_port, 1);
        cmd = DRV_IOW(DsPhyPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_lport, cmd, &phy_port));
    }
    else
    {
        sal_memset(&src_port, 0, sizeof(src_port));
        cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_lport, cmd, &src_port));
        SetDsSrcPort(V, interfaceId_f, &src_port, 0);
        SetDsSrcPort(V, routedPort_f, &src_port, 0);
        SetDsSrcPort(V, addDefaultVlanDisable_f, &src_port, 0);
        cmd = DRV_IOW(DsSrcPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_lport, cmd, &src_port));

        sal_memset(&phy_port_ext, 0, sizeof(phy_port_ext));
        cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_lport, cmd, &phy_port_ext));
        SetDsPhyPortExt(V, defaultVlanId_f, &phy_port_ext, 1);
        cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_lport, cmd, &phy_port_ext));

        sal_memset(&phy_port, 0, sizeof(phy_port));
        SetDsPhyPort(V, fwdHashGenDis_f, &phy_port, 0);
        cmd = DRV_IOW(DsPhyPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_lport, cmd, &phy_port));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_port_get_wlan_port_route(uint8 lchip, uint32 gport, uint32* p_value)
{
    uint16 lport     = 0;
    uint16 tmp_lport     = 0;
    uint16 l3if_id     = 0;
    uint32 cmd       = 0;
    uint32 field = 0;
    DsSrcPort_m src_port;

    SYS_PORT_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    cmd = DRV_IOR(IpeE2iLoopCtl_t, IpeE2iLoopCtl_portArray_0_loopbackPort_f+lport);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));
    tmp_lport = field;
    *p_value = 0;
    sal_memset(&src_port, 0, sizeof(src_port));
    cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_lport, cmd, &src_port));
    l3if_id =  GetDsSrcPort(V, interfaceId_f, &src_port);
    if (MCHIP_CAP(SYS_CAP_L3IF_RSV_L3IF_ID) == l3if_id)
    {
        *p_value = 1;
    }

    return CTC_E_NONE;
}
/*MUST return none when not support global port property*/
STATIC int32
_sys_usw_port_set_global_property(uint8 lchip, uint32 gport, uint32 port_prop, uint32 value)
{
    int32   ret = CTC_E_NONE;
    uint32  cmd = 0;
    uint16  index = 0;
    uint32  temp_value = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set global port property, gport:0x%04X, property:%d, value:%d\n", \
                     gport, port_prop, value);

    if(!_sys_usw_port_is_support_glb_port(lchip) || !p_usw_port_master[lchip]->rchip_gport_idx_cb)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "the callback function for mapping gport to global dest index is not registered!\n");
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NONE;

    }

    CTC_ERROR_RETURN(p_usw_port_master[lchip]->rchip_gport_idx_cb(lchip, gport, &index));

    temp_value = value;
    switch (port_prop)
    {
        case CTC_PORT_PROP_BRIDGE_EN:
            temp_value = value ? 1 : 0;
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_bridgeEn_f);
            break;
        case CTC_PORT_DIR_PROP_QOS_COS_DOMAIN:
            CTC_MAX_VALUE_CHECK(value, 7);
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_cosPhbPtr_f);
            break;
        case CTC_PORT_PROP_VLAN_DOMAIN:
            CTC_MAX_VALUE_CHECK(value, CTC_DOT1Q_TYPE_BOTH);
            temp_value = (CTC_DOT1Q_TYPE_CVLAN == value) ? 1 : 0;
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_cvlanSpace_f);
            break;
        case CTC_PORT_PROP_DEFAULT_DEI:
            CTC_MAX_VALUE_CHECK(value, 1);
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_defaultDei_f);
            break;
        case CTC_PORT_PROP_DEFAULT_DSCP:
            if (value > CTC_MAX_QOS_DSCP_VALUE)
            {
                return CTC_E_INVALID_PARAM;
            }
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_defaultDscp_f);
            break;
        case CTC_PORT_PROP_REPLACE_PCP_DEI:
            temp_value = !!value;
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_replacePortPcp_f);
            break;
        case CTC_PORT_PROP_DEFAULT_PCP:
            CTC_MAX_VALUE_CHECK(value, 7);
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_defaultPcp_f);
            break;
        case CTC_PORT_PROP_DEFAULT_VLAN:
            CTC_VLAN_RANGE_CHECK(value);
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_defaultVlanId_f);
            break;
        case CTC_PORT_PROP_DOT1Q_TYPE:
            CTC_MAX_VALUE_CHECK(temp_value, CTC_DOT1Q_TYPE_BOTH);
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_dot1QEn_f);
            break;
        case CTC_PORT_DIR_PROP_QOS_DOMAIN:
            CTC_MAX_VALUE_CHECK(temp_value, 7);
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_cosPhbPtr_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &temp_value));
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_dscpPhbPtr_f);
            break;
        case CTC_PORT_DIR_PROP_QOS_DSCP_DOMAIN:
            CTC_MAX_VALUE_CHECK(value, 15);
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_dscpPhbPtr_f);
            break;
        case CTC_PORT_PROP_DSCP_SELECT_MODE:
            CTC_MAX_VALUE_CHECK(value, MAX_CTC_DSCP_SELECT_MODE - 1);
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_dscpRemarkMode_f);
            break;

        case CTC_PORT_PROP_PRIORITY_TAG_EN:
            temp_value = !!value;
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_priorityTagEn_f);
            break;

        case CTC_PORT_PROP_REPLACE_CTAG_COS:
            temp_value = !!value;
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_replaceCtagCos_f);
            break;

        case CTC_PORT_PROP_REPLACE_DSCP_EN:
            temp_value = (value) ? 2 : 0;
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_dscpRemarkMode_f);
            break;
        case CTC_PORT_PROP_REPLACE_STAG_COS:
            temp_value = !!value;
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_replaceStagCos_f);
            break;

        case CTC_PORT_PROP_REPLACE_STAG_TPID:
            temp_value = !!value;
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_replaceStagTpid_f);
            break;

        case CTC_PORT_DIR_PROP_STAG_TPID_INDEX:
            CTC_MAX_VALUE_CHECK(value, 3);
            cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_svlanTpidIndex_f);
            break;

        case CTC_PORT_PROP_UNTAG_PVID_TYPE:
            switch(value)
            {
                case CTC_PORT_UNTAG_PVID_TYPE_NONE:
                    temp_value = 0;
                    cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_untagDefaultVlanId_f);
                    ret = DRV_IOCTL(lchip, index, cmd, &temp_value);
                    break;

                case CTC_PORT_UNTAG_PVID_TYPE_SVLAN:

                    temp_value = 1;
                    cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_untagDefaultVlanId_f);
                    ret = DRV_IOCTL(lchip, index, cmd, &temp_value);
                    temp_value = 1;
                    cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_untagDefaultSvlan_f);
                    ret = ret ? ret : DRV_IOCTL(lchip, index, cmd, &temp_value);
                    break;

                case CTC_PORT_UNTAG_PVID_TYPE_CVLAN:

                    temp_value = 1;
                    cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_untagDefaultVlanId_f);
                    ret = DRV_IOCTL(lchip, index, cmd, &temp_value);
                    temp_value = 0;
                    cmd = DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_untagDefaultSvlan_f);
                    ret = ret ? ret : DRV_IOCTL(lchip, index, cmd, &temp_value);
                    break;

                default:
                    return CTC_E_INVALID_PARAM;
                    break;
            }

            return ret;

        default:
            return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &temp_value));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_port_get_global_property(uint8 lchip, uint32 gport, uint32 port_prop, uint32* p_value)
{
    uint32 cmd = 0;
    uint16 index = 0;
    uint32 temp_value = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get global port property, gport:0x%04X, property:%d!\n", gport, port_prop);

    if(!_sys_usw_port_is_support_glb_port(lchip) || !p_usw_port_master[lchip]->rchip_gport_idx_cb)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "the callback function for mapping gport to global dest index is not registered!\n");
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

    }

    CTC_ERROR_RETURN(p_usw_port_master[lchip]->rchip_gport_idx_cb(lchip, gport, &index));

    switch (port_prop)
    {
        case CTC_PORT_PROP_BRIDGE_EN:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_bridgeEn_f);
            break;
        case CTC_PORT_DIR_PROP_QOS_COS_DOMAIN:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_cosPhbPtr_f);
            break;
        case CTC_PORT_DIR_PROP_QOS_DOMAIN:
            {
                uint32 value1 = 0;
                uint32 value2 = 0;

                cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_cosPhbPtr_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value1));
                cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_dscpPhbPtr_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value2));
                if(value1 == value2)
                {
                    *p_value = value1;
                }
                else
                {
                    *p_value = 0;
                }
                cmd = 0 ;
                break;
            }
        case CTC_PORT_DIR_PROP_QOS_DSCP_DOMAIN:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_dscpPhbPtr_f);
            break;
        case CTC_PORT_PROP_VLAN_DOMAIN:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_cvlanSpace_f);
            break;
        case CTC_PORT_PROP_DEFAULT_DEI:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_defaultDei_f);
            break;
        case CTC_PORT_PROP_DEFAULT_DSCP:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_defaultDscp_f);
            break;
        case CTC_PORT_PROP_REPLACE_DSCP_EN:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_dscpRemarkMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &temp_value));
            if(temp_value == 2)
            {
                *p_value = 1;
            }
            else
            {
                *p_value = 0;
            }
            cmd = 0;
            break;
        case CTC_PORT_PROP_REPLACE_PCP_DEI:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_replacePortPcp_f);
            break;
        case CTC_PORT_PROP_DEFAULT_PCP:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_defaultPcp_f);
            break;
        case CTC_PORT_PROP_DEFAULT_VLAN:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_defaultVlanId_f);
            break;
        case CTC_PORT_PROP_DOT1Q_TYPE:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_dot1QEn_f);
            break;

        case CTC_PORT_PROP_DSCP_SELECT_MODE:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_dscpRemarkMode_f);
            break;

        case CTC_PORT_PROP_PRIORITY_TAG_EN:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_priorityTagEn_f);
            break;

        case CTC_PORT_PROP_REPLACE_CTAG_COS:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_replaceCtagCos_f);
            break;


        case CTC_PORT_PROP_REPLACE_STAG_COS:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_replaceStagCos_f);
            break;

        case CTC_PORT_PROP_REPLACE_STAG_TPID:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_replaceStagTpid_f);
            break;

        case CTC_PORT_DIR_PROP_STAG_TPID_INDEX:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_svlanTpidIndex_f);
            break;

        case CTC_PORT_PROP_UNTAG_PVID_TYPE:
            cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_untagDefaultVlanId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &temp_value));
            if (!temp_value) /* none */
            {
                *p_value = CTC_PORT_UNTAG_PVID_TYPE_NONE;
            }
            else /* s or c */
            {
                cmd = DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_untagDefaultSvlan_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &temp_value));

                if (temp_value) /* s */
                {
                    *p_value = CTC_PORT_UNTAG_PVID_TYPE_SVLAN;
                }
                else
                {
                    *p_value = CTC_PORT_UNTAG_PVID_TYPE_CVLAN;
                }
            }
            return CTC_E_NONE;
        default:
            return CTC_E_NOT_SUPPORT;
    }

    if(cmd)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, p_value));
    }
    return CTC_E_NONE;
}

int32
sys_usw_port_get_local_phy_port(uint8 lchip, uint32 gport, uint32* p_value)
{
    uint16 lport     = 0;
    uint16 local_phy_port = SYS_INVALID_LOCAL_PHY_PORT;
    uint8  gchip_id = 0;

    SYS_PORT_INIT_CHECK();
    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);

    gchip_id = CTC_MAP_GPORT_TO_GCHIP(gport);
    PORT_LOCK;
    if(sys_usw_chip_is_local(lchip, gchip_id) && !CTC_IS_LINKAGG_PORT(gport))
    {
        local_phy_port = p_usw_port_master[lchip]->network_port[lport];
    }
    PORT_UNLOCK;
    *p_value = local_phy_port;

    return CTC_E_NONE;
}

int32
sys_usw_port_lport_convert(uint8 lchip, uint16 internal_lport, uint16* p_value)
{
    uint16 loop = 0;

    SYS_PORT_INIT_CHECK();
    PORT_LOCK;
    for (loop = 0; loop < MCHIP_CAP(SYS_CAP_SPEC_MAX_PHY_PORT_NUM); loop++)
    {
        if (0 == p_usw_port_master[lchip]->igs_port_prop[loop].lbk_en)
        {
            continue;
        }
        if (p_usw_port_master[lchip]->igs_port_prop[loop].inter_lport == internal_lport)
        {
            *p_value = loop;
            break;
        }
    }
    PORT_UNLOCK;

    return CTC_E_NONE;
}


int32
sys_usw_port_set_station_move(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32 value)
{
    uint32  cmd = 0;
    int32   ret = 0;
    uint32  field = 0;
    uint8   gchip = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sys_usw_get_gchip_id(lchip, &gchip);

    if (port_prop == CTC_PORT_PROP_STATION_MOVE_PRIORITY)
    {
        CTC_MAX_VALUE_CHECK(value, 1);
        field = value;
        CTC_ERROR_RETURN(sys_usw_l2_set_station_move(lchip, gport, SYS_L2_STATION_MOVE_OP_PRIORITY, field));
    }
    else if(port_prop == CTC_PORT_PROP_STATION_MOVE_ACTION)
    {
        CTC_MAX_VALUE_CHECK(value, CTC_PORT_STATION_MOVE_ACTION_TYPE_MAX-1);
        field = (value == CTC_PORT_STATION_MOVE_ACTION_TYPE_DISCARD_TOCPU) ? 1 : 0;
        if (field && (gchip != CTC_MAP_GPORT_TO_GCHIP(gport)))
        {
            return CTC_E_NOT_SUPPORT;
        }

        CTC_ERROR_RETURN(sys_usw_l2_set_station_move(lchip, gport, SYS_L2_STATION_MOVE_OP_ACTION, value));

        if (gchip == CTC_MAP_GPORT_TO_GCHIP(gport))
        {
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_srcMismatchExceptionEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, CTC_MAP_GPORT_TO_LPORT(gport), cmd, &field));
        }
    }

    return ret;
}


/*SYSTEM MODIFIED by huwx for copp iloop port 2018-06-07*/
int32
sys_usw_internal_port_set_routed_port(uint8 lchip, uint16 gport, uint32 is_routed)
{    
    int32 ret = CTC_E_NONE;    
    uint32 value = 0;    
    uint16 lport = 0;    
    uint32 cmd = 0;   
    
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);    
    value = is_routed?1:0;    
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_routedPort_f);   
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));  
    
    return ret;
}

/**
@brief   Config port's properties
*/
int32
sys_usw_port_set_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32 value)
{
    int32   ret = CTC_E_NONE;

    uint16  lport = 0;
    uint32  field = 0;
    uint32  cmd = 0;
    uint32  entry_num = 0;
    uint32  port_type = 0;
    uint32  port_mac_id = 0;
    uint32  port_pmac_id = 0;
    uint32  port_chan_id = 0;
    uint32  port_pchan_id = 0;
    uint32  step = 0;
    uint32  index = 0;
    uint32  srcGuaranteeEn[2] = {0};
    uint8   is_support_glb_port = _sys_usw_port_is_support_glb_port(lchip);
	uint8   gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport);


    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set port property, gport:0x%04X, property:%d, value:%d\n", \
                     gport, port_prop, value);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
	SYS_GLOBAL_CHIPID_CHECK(gchip);
	if (FALSE == sys_usw_chip_is_local(lchip, gchip) && port_prop != CTC_PORT_PROP_XPIPE_EN)
	{
		return _sys_usw_port_set_global_property(lchip, gport, port_prop, value);
	}

	lport = CTC_MAP_GPORT_TO_LPORT(gport);
	if(lport >= MCHIP_CAP(SYS_CAP_PORT_NUM_PER_CHIP))
	{
		return CTC_E_INVALID_PORT;
	}

    switch (port_prop)
    {
    case CTC_PORT_PROP_WLAN_DECAP_WITH_RID:
        field = value ? 1 : 0;
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_radioMacType_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;
    case CTC_PORT_PROP_DISCARD_UNENCRYPT_PKT:
        field = value ? 1 : 0;
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_discardPlainTextPacket_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;
    case CTC_PORT_PROP_RAW_PKT_TYPE:
        ret = _sys_usw_port_set_raw_packet_type(lchip, gport, value);
        break;

    case CTC_PORT_PROP_VLAN_DOMAIN:
        field = (CTC_PORT_VLAN_DOMAIN_CVLAN == value) ? 1 : 0;

        cmd = DRV_IOW(DsPhyPort_t, DsPhyPort_outerVlanIsCVlan_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_cvlanSpace_f):DRV_IOW(DsDestPort_t, DsDestPort_cvlanSpace_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field);
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_flowL2KeyUseCvlan_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field);
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_flowL2KeyUseCvlan_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field);
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_sclFlowL2KeyUseCvlan_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_PORT_EN:
        if (value)
        {
            if(MCHIP_API(lchip)->mac_link_up_event)
            {
                MCHIP_API(lchip)->mac_link_up_event(lchip, gport);
            }
        }
        else
        {
            sys_usw_mac_link_down_event(lchip, gport);
        }
        ret = _sys_usw_port_set_port_en(lchip, gport, value);
        break;

    case CTC_PORT_PROP_TRANSMIT_EN:     /**< set port whether the tranmist is enable */
        field = (value) ? 0 : 1;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_transmitDisable_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
		/* SYSTEM MODIFIED by wulx for bug 45496, sdk bug 105105, 2018-08-04 */
        //CTC_ERROR_RETURN(sys_usw_port_set_mcast_member_down(lchip, gport, (value) ? 0 : 1));
        break;

    case CTC_PORT_PROP_RECEIVE_EN:      /**< set port whether the receive is enable */
        field = (value) ? 0 : 1;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_receiveDisable_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_BRIDGE_EN:       /**< set port whehter layer2 bridge function is enable */
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_bridgeEn_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        field = (value) ? 1 : 0;

        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_bridgeEn_f):DRV_IOW(DsDestPort_t, DsDestPort_bridgeEn_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_DEFAULT_VLAN:     /**< set default vlan id of packet which receive from this port */
        /* SYSTEM MODIFYED BY huwx for copp iloop internal port set vlan 0 2018-06-01 */
        //CTC_VLAN_RANGE_CHECK(value);
        ret = _sys_usw_port_set_default_vlan(lchip, gport, value);
        break;

    case CTC_PORT_PROP_UNTAG_PVID_TYPE:
        switch(value)
        {
        case CTC_PORT_UNTAG_PVID_TYPE_NONE:
            field = 0;
            cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_untagDefaultVlanId_f):DRV_IOW(DsDestPort_t, DsDestPort_untagDefaultVlanId_f);
            ret = DRV_IOCTL(lchip, lport, cmd, &field);
            break;

        case CTC_PORT_UNTAG_PVID_TYPE_SVLAN:
            field = 1;
            cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_untagDefaultVlanId_f):DRV_IOW(DsDestPort_t, DsDestPort_untagDefaultVlanId_f);
            ret = DRV_IOCTL(lchip, lport, cmd, &field);
            field = 1;
            cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_untagDefaultSvlan_f):DRV_IOW(DsDestPort_t, DsDestPort_untagDefaultSvlan_f);
            ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field);
            break;

        case CTC_PORT_UNTAG_PVID_TYPE_CVLAN:
            field = 1;
            cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_untagDefaultVlanId_f):DRV_IOW(DsDestPort_t, DsDestPort_untagDefaultVlanId_f);
            ret = DRV_IOCTL(lchip, lport, cmd, &field);
            field = 0;
            cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_untagDefaultSvlan_f):DRV_IOW(DsDestPort_t, DsDestPort_untagDefaultSvlan_f);
            ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field);
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }
        break;

    case CTC_PORT_PROP_VLAN_CTL:     /**< set port's vlan tag control mode */
        if (value >= MAX_CTC_VLANTAG_CTL)
        {
            return CTC_E_INVALID_PARAM;
        }
        field = value;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_vlanTagCtl_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_CROSS_CONNECT_EN:     /**< Set port cross connect */
        ret = _sys_usw_port_set_cross_connect(lchip, gport, value);
        break;

    case CTC_PORT_PROP_LEARNING_EN:     /**< Set learning enable/disable on port */
        field = (value) ? 0 : 1;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_learningDisable_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_LEARN_DIS_MODE:
        if (!DRV_FLD_IS_EXISIT(DsSrcPort_t, DsSrcPort_learningDisableMode_f))
        {
            return CTC_E_NOT_SUPPORT;
        }
        field = value? 1: 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_learningDisableMode_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_FORCE_MPLS_EN:
        if (!DRV_FLD_IS_EXISIT(DsSrcPort_t, DsSrcPort_openflowMplsEn_f))
        {
            return CTC_E_NOT_SUPPORT;
        }
        field = value? 1: 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_openflowMplsEn_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_FORCE_TUNNEL_EN:
        if (!DRV_FLD_IS_EXISIT(DsSrcPort_t, DsSrcPort_openflowTunnelEn_f))
        {
            return CTC_E_NOT_SUPPORT;
        }
        field = value? 1: 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_openflowTunnelEn_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_DOT1Q_TYPE:     /**< Set port dot1q type */
        CTC_MAX_VALUE_CHECK(value, CTC_DOT1Q_TYPE_BOTH);
        field = value;
        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_dot1QEn_f): DRV_IOW(DsDestPort_t, DsDestPort_dot1QEn_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_USE_OUTER_TTL:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_useOuterTtl_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_PROTOCOL_VLAN_EN:     /**< set protocol vlan enable on port */
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_protocolVlanEn_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_SRC_DISCARD_EN:     /**< set port whether the src_discard is enable */
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsPhyPort_t, DsPhyPort_srcDiscard_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_PORT_CHECK_EN:     /**< set port whether the src port match check is enable */
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_portCheckEn_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_HW_LEARN_EN:     /**< Hardware learning enable*/
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_fastLearningEn_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_SCL_IPV6_LOOKUP_EN:
        ret = CTC_E_NOT_SUPPORT;
        break;

    case CTC_PORT_PROP_SCL_IPV4_LOOKUP_EN:
        ret = CTC_E_NOT_SUPPORT;
        break;

    case CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_useDefaultVlanLookup_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_SCL_FORCE_IPV6_TO_MAC:
        ret = CTC_E_NOT_SUPPORT;
        break;

    case CTC_PORT_PROP_SCL_FORCE_IPV4_TO_MAC:
        ret = CTC_E_NOT_SUPPORT;
        break;

    case CTC_PORT_PROP_TRILL_EN:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_trillEn_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;
    case CTC_PORT_PROP_TRILL_MCAST_DECAP_EN:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_trillMcastLoopback_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_DISCARD_NON_TRIL_PKT:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_discardNonTrill_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_DISCARD_TRIL_PKT:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_discardTrill_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_REFLECTIVE_BRIDGE_EN:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_reflectiveBridgeEn_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_FCOE_EN:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_fcoeEn_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_RPF_EN:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_portBasedRpfEn_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;
    case CTC_PORT_PROP_FCMAP:
        CTC_VALUE_RANGE_CHECK(value, 0x0EFC00, 0x0EFCFF);
        field = value & 0xFF;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_fcoeOuiIndex_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_REPLACE_STAG_COS:
        field = (value) ? 1 : 0;
        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_replaceStagCos_f):DRV_IOW(DsDestPort_t, DsDestPort_replaceStagCos_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_REPLACE_STAG_TPID:
        field = (value) ? 1 : 0;
        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_replaceStagTpid_f):DRV_IOW(DsDestPort_t, DsDestPort_replaceStagTpid_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_REPLACE_CTAG_COS:
        field = (value) ? 1 : 0;
        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_replaceCtagCos_f):DRV_IOW(DsDestPort_t, DsDestPort_replaceCtagCos_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_REPLACE_DSCP_EN:
        field = (value) ? 2 : 0;
        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_dscpRemarkMode_f):DRV_IOW(DsDestPort_t, DsDestPort_dscpRemarkMode_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_L3PDU_ARP_ACTION:
        if (value >= CTC_PORT_ARP_ACTION_TYPE_MAX)
        {
            return CTC_E_INVALID_PARAM;
        }
        field = value;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_arpExceptionType_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_L3PDU_DHCP_ACTION:
        if (value >= CTC_PORT_DHCP_ACTION_TYPE_MAX)
        {
            return CTC_E_INVALID_PARAM;
        }
        field = value;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_dhcpV4ExceptionType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
        field = value;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_dhcpV6ExceptionType_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_TUNNEL_RPF_CHECK:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_tunnelRpfCheck_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_PTP_EN:
        if (value > MCHIP_CAP(SYS_CAP_PTP_MAX_PTP_ID))
        {
            return CTC_E_INVALID_PARAM;
        }
        field = value;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_ptpIndex_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_RPF_TYPE:
        if (value >= CTC_PORT_RPF_TYPE_MAX)
        {
            return CTC_E_INVALID_PARAM;
        }
        field = value;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_rpfType_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_IS_LEAF:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_isLeaf_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_PKT_TAG_HIGH_PRIORITY:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_ingressTagHighPriority_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_ROUTE_EN:
        field = (value) ? 0 : 1;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_routeDisable_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_PRIORITY_TAG_EN:
        field = (value) ? 1 : 0;
        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_priorityTagEn_f):DRV_IOW(DsDestPort_t, DsDestPort_priorityTagEn_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_IPG:
        if (value >= CTC_IPG_SIZE_MAX)
        {
            return CTC_E_INVALID_PARAM;
        }
        field = value;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_ipgIndex_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_ipgIndex_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &value);
        break;

    case CTC_PORT_PROP_DEFAULT_PCP:
        CTC_MAX_VALUE_CHECK(value, 7);

        field = value;
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_defaultPcp_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        field = value;
        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_defaultPcp_f):DRV_IOW(DsDestPort_t, DsDestPort_defaultPcp_f);
        ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_DEFAULT_DEI:
        CTC_MAX_VALUE_CHECK(value, 1);
        field = value;
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_defaultDei_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);

        field = value;
        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_defaultDei_f):DRV_IOW(DsDestPort_t, DsDestPort_defaultDei_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_NVGRE_MCAST_NO_DECAP:
        CTC_MAX_VALUE_CHECK(value, 1);
        field = value;
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_nvgreMcastLoopbackMode_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_VXLAN_MCAST_NO_DECAP:
        CTC_MAX_VALUE_CHECK(value, 1);
        field = value;
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_vxlanMcastLoopbackMode_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_QOS_POLICY:
        if (value >= CTC_QOS_TRUST_MAX)
        {
            return CTC_E_INVALID_PARAM;
        }
        ret  = _sys_usw_port_map_qos_policy(lchip, value, lport);

        break;

    case CTC_PORT_PROP_SCL_HASH_FIELD_SEL_ID:
        CTC_MAX_VALUE_CHECK(value, 3);
        field = value;
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_sclFlowHashFieldSel_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_SNOOPING_PARSER:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_forceSecondParser_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_FLOW_LKUP_BY_OUTER_HEAD:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_aclQosUseOuterInfo_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_IPFIX_LKUP_BY_OUTER_HEAD:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_ipfixAndMicroflowUseOuterInfo_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_AWARE_TUNNEL_INFO_EN:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_aclKeyMergeInnerAndOuterHdr_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_IPFIX_AWARE_TUNNEL_INFO_EN:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_ipfixAndMicroflowMergeHdr_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_NVGRE_ENABLE:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_nvgreEnable_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_METADATA_OVERWRITE_PORT:
        field = (value) ? 3 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_0_aclUseGlobalPortType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_1_aclUseGlobalPortType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_2_aclUseGlobalPortType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_3_aclUseGlobalPortType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
        break;

    case CTC_PORT_PROP_METADATA_OVERWRITE_UDF:
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

        break;

    case CTC_PORT_PROP_SRC_MISMATCH_EXCEPTION_EN:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_srcMismatchExceptionEn_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_STATION_MOVE_PRIORITY:
        CTC_MAX_VALUE_CHECK(value, 1);
        field = value;
        ret = sys_usw_l2_set_station_move(lchip, gport, SYS_L2_STATION_MOVE_OP_PRIORITY, field);
        break;

    case CTC_PORT_PROP_STATION_MOVE_ACTION:
        CTC_MAX_VALUE_CHECK(value, CTC_PORT_STATION_MOVE_ACTION_TYPE_MAX-1);
        CTC_ERROR_RETURN(sys_usw_l2_set_station_move(lchip, gport, SYS_L2_STATION_MOVE_OP_ACTION, value));
        field = (value == CTC_PORT_STATION_MOVE_ACTION_TYPE_DISCARD_TOCPU) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_srcMismatchExceptionEn_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_EFD_EN:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(IpePortEfdCtl_t, IpePortEfdCtl_array_0_efdEnable_f+(lport&0xFF));
        ret = DRV_IOCTL(lchip, (lport>>8), cmd, &field);
        break;

    case CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_addDefaultVlanDisable_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_LOGIC_PORT:
        if (!value)
        {
            field  = 0;
            cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_useDefaultLogicSrcPort_f);
            ret = DRV_IOCTL(lchip, lport, cmd, &field);

            field = 0;
            cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_defaultLogicSrcPort_f);
            ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field);

            field  = 0;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_logicDestPort_f);
            ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field);
        }
        else
        {
            CTC_MAX_VALUE_CHECK(value, (DRV_IS_DUET2(lchip)? 0x3FFF: 0xFFFF));
            field  = 1;
            cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_useDefaultLogicSrcPort_f);
            ret = DRV_IOCTL(lchip, lport, cmd, &field);

            field = value;
            cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_defaultLogicSrcPort_f);
            ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field);

            field  = value;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_logicDestPort_f);
            ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field);
        }

        break;

    case CTC_PORT_PROP_GPORT:

        ret = sys_usw_port_set_global_port(lchip, lport, value, FALSE);
        break;

    case CTC_PORT_PROP_LOGIC_PORT_CHECK_EN:

        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_logicPortCheckEn_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;

    case CTC_PORT_PROP_APS_FAILOVER_EN:
        value = (value) ? 1 : 0;
        ret = _sys_usw_port_set_aps_failover_en(lchip, gport, value);
        break;

    case CTC_PORT_PROP_LINKAGG_FAILOVER_EN:
        value = (value) ? 1 : 0;
        ret = _sys_usw_port_set_linkagg_failover_en(lchip, gport, value);
        break;

    case CTC_PORT_PROP_APS_SELECT_GRP_ID:
        if (0xFFFFFFFF == value)
        {
            field = 0;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_apsSelectValid_f);
            ret = DRV_IOCTL(lchip, lport, cmd, &field);
        }
        else
        {
            sys_usw_ftm_query_table_entry_num(lchip, DsApsBridge_t, &entry_num);

            if (value >= entry_num)
            {
                ret = CTC_E_BADID;
            }
            else
            {
                field = value;
                cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_apsSelectGroupId_f);
                ret = DRV_IOCTL(lchip, lport, cmd, &field);
                field = 1;
                cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_apsSelectValid_f);
                ret = ret ? ret : DRV_IOCTL(lchip, lport, cmd, &field);
            }
        }
        break;

    case CTC_PORT_PROP_APS_SELECT_WORKING:
        field = (value) ? 0 : 1;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_apsSelectProtectingPath_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;
    case CTC_PORT_PROP_CID:
        SYS_USW_CID_CHECK(lchip,value);
        field = (value)?1:0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_categoryId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_categoryIdValid_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;
    case CTC_PORT_PROP_TX_CID_HDR_EN:
        field = (value)?1:0;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_withCidHeader_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
        break;
    case CTC_PORT_PROP_WLAN_PORT_TYPE:
        ret = _sys_usw_port_set_wlan_port_type(lchip, gport, value);
        break;
    case CTC_PORT_PROP_WLAN_PORT_ROUTE_EN:
        ret = _sys_usw_port_set_wlan_port_route(lchip, gport, value);
        break;
    case CTC_PORT_PROP_DSCP_SELECT_MODE:
        CTC_MAX_VALUE_CHECK(value, MAX_CTC_DSCP_SELECT_MODE - 1);
        field = value;
        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_dscpRemarkMode_f):DRV_IOW(DsDestPort_t, DsDestPort_dscpRemarkMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
        break;
    case CTC_PORT_PROP_DEFAULT_DSCP:
        if (value > CTC_MAX_QOS_DSCP_VALUE)
        {
            return CTC_E_INVALID_PARAM;
        }
        field = value;
        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_defaultDscp_f):DRV_IOW(DsDestPort_t, DsDestPort_defaultDscp_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
        break;

    case CTC_PORT_PROP_REPLACE_PCP_DEI:
        field = value ? 1 : 0;
        cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_replacePortPcp_f):DRV_IOW(DsDestPort_t, DsDestPort_replacePortPcp_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
        break;
    case CTC_PORT_PROP_CUT_THROUGH_EN:
        ret = _sys_usw_port_set_cut_through_en(lchip,gport,value);
        break;
    case CTC_PORT_PROP_SD_ACTION:
        ret = _sys_usw_port_set_sd_action(lchip, CTC_BOTH_DIRECTION, lport, value);
        break;

    case CTC_PORT_PROP_LOOP_WITH_SRC_PORT:
        field = (value) ? 1 : 0;
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_sourcePortToHeader_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &field);
        break;
    case CTC_PORT_PROP_LB_HASH_ECMP_PROFILE:
        CTC_MAX_VALUE_CHECK(value, 63);
        field =  value?1:0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_portBasedHashProfile0Valid_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_portBasedHashProfileId0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
        break;
	case CTC_PORT_PROP_LB_HASH_LAG_PROFILE:
		CTC_MAX_VALUE_CHECK(value, 63);
        field = value?1:0;
		cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_portBasedHashProfile1Valid_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field));
		cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_portBasedHashProfileId1_f);
		CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
        break;
    case CTC_PORT_PROP_XPIPE_EN:
        CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_PORT_TYPE, (void *)&port_type));
        if (SYS_DMPS_NETWORK_PORT != port_type)
        {
            return CTC_E_INVALID_CONFIG;
        }
        ret = sys_usw_dmps_set_port_property(lchip, gport, SYS_DMPS_PORT_PROP_XPIPE_ENABLE, (void *)&value);
        if(ret)
        {
            return ret;
        }
        ret = sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_MAC_ID, (void *)&port_mac_id);
        if(ret)
        {
            return ret;
        }
        cmd = DRV_IOR(NetRxCtl_t, NetRxCtl_portSplitEn_f);
        DRV_IOCTL(lchip, 0, cmd, &field);
        /* index = {macid[5:3] macid[1:0]} */
        index = ((port_mac_id & 0x38)>>1) | (port_mac_id & 0x3);
        if (value)
        {
            CTC_BIT_SET(field, index);
        }
        else
        {
            CTC_BIT_UNSET(field, index);
        }
        cmd = DRV_IOW(NetRxCtl_t, NetRxCtl_portSplitEn_f);
        DRV_IOCTL(lchip, 0, cmd, &field);
        cmd = DRV_IOW(NetTxPortSplitEnCtl_t, NetTxPortSplitEnCtl_portSplitEn_f);
        DRV_IOCTL(lchip, 0, cmd, &field);
        cmd = DRV_IOW(QWriteGuaranteeCtl_t, QWriteGuaranteeCtl_glbGuaranteeEn_f);
        value = field?1:0;
        DRV_IOCTL(lchip, 0, cmd, &value);
        /* set srcGuaranteeEn and destGuaranteeChannel */
        cmd = DRV_IOR(QWriteGuaranteeCtl_t, QWriteGuaranteeCtl_srcGuaranteeEn_f);
        DRV_IOCTL(lchip, 0, cmd, srcGuaranteeEn);
        if(port_mac_id<4)
        {
            port_pmac_id = port_mac_id + 4;
            /* get emac channel id from mac id*/
            cmd = DRV_IOR(NetRxChannelMap_t, NetRxChannelMap_cfgPortToChanMapping_f);
            DRV_IOCTL(lchip, port_mac_id, cmd, &port_chan_id);
            /* get pmac channel id from mac id*/
            DRV_IOCTL(lchip, port_pmac_id, cmd, &port_pchan_id);
            CTC_BIT_UNSET(srcGuaranteeEn[port_chan_id>31?1:0], port_chan_id>31?port_chan_id-32:port_chan_id);
            CTC_BIT_SET(srcGuaranteeEn[port_pchan_id>31?1:0], port_pchan_id>31?port_pchan_id-32:port_pchan_id);
        }
        else
        {
            port_pmac_id = port_mac_id - 4;
            /* get emac channel id from mac id*/
            cmd = DRV_IOR(NetRxChannelMap_t, NetRxChannelMap_cfgPortToChanMapping_f);
            DRV_IOCTL(lchip, port_mac_id, cmd, &port_chan_id);
            /* get pmac channel id from mac id*/
            DRV_IOCTL(lchip, port_pmac_id, cmd, &port_pchan_id);
            CTC_BIT_SET(srcGuaranteeEn[port_pchan_id>31?1:0], port_pchan_id>31?port_pchan_id-32:port_pchan_id);
            CTC_BIT_UNSET(srcGuaranteeEn[port_chan_id>31?1:0], port_chan_id>31?port_chan_id-32:port_chan_id);
        }
        cmd = DRV_IOW(QWriteGuaranteeCtl_t, QWriteGuaranteeCtl_srcGuaranteeEn_f);
        DRV_IOCTL(lchip, 0, cmd, srcGuaranteeEn);
        step = QWriteGuaranteeCtl_array_1_destGuaranteeChannel_f - QWriteGuaranteeCtl_array_0_destGuaranteeChannel_f;
        cmd = DRV_IOW(QWriteGuaranteeCtl_t, QWriteGuaranteeCtl_array_0_destGuaranteeChannel_f+step*port_chan_id);
        DRV_IOCTL(lchip, 0, cmd, &port_pchan_id);
        break;
    case CTC_PORT_PROP_INBAND_CPU_TRAFFIC_EN:
        field = (value) ? 1 : 0;
        if (REGISTER_API(lchip)->inband_port_en)
        {
            CTC_ERROR_RETURN(REGISTER_API(lchip)->inband_port_en(lchip, gport, field));
        }
        break;
    case CTC_PORT_PROP_MUX_TYPE:
        ret = _sys_usw_port_set_mux_type(lchip, gport, value);
        break;
    case CTC_PORT_PROP_PHY_INIT:
        ret = _sys_usw_port_set_phy_init(lchip, gport, value);
        break;
    case CTC_PORT_PROP_PHY_EN:
        {
            uint32 phy_id = 0;
            sys_usw_peri_get_phy_id(lchip, lport, &phy_id);
            if ((CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport)) && (phy_id != CTC_CHIP_PHY_NULL_PHY_ID))
            {
                ret = sys_usw_peri_set_phy_prop(lchip, lport, CTC_PORT_PROP_PHY_EN, value);
            }
            else
            {
                ret = sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_MAC_EN, value);
            }
        }
        break;
    case CTC_PORT_PROP_PHY_DUPLEX:
    case CTC_PORT_PROP_PHY_MEDIUM:
    case CTC_PORT_PROP_PHY_LOOPBACK:
        if (CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport))
        {
            ret = sys_usw_peri_set_phy_prop(lchip, lport, port_prop, value);
        }
        else
        {
            return CTC_E_NOT_INIT;
        }
        break;
    default:
        if ((port_prop >= CTC_PORT_PROP_PHY_CUSTOM_BASE) && (port_prop <= CTC_PORT_PROP_PHY_CUSTOM_MAX_TYPE))
        {
            if (CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport))
            {
                ret = sys_usw_peri_set_phy_prop(lchip, lport, port_prop, value);
            }
            else
            {
                return CTC_E_NOT_INIT;
            }
        }
        else
        {
            return CTC_E_NOT_SUPPORT;
        }
    }

    if (ret != CTC_E_NONE)
    {
        return ret;
    }

    return CTC_E_NONE;
}

/**
@brief    Get port's properties according to gport id
*/
int32
sys_usw_port_get_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32* p_value)
{
    uint16 value16 = 0;
    uint8  inverse_value = 0;
    int32  ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint16 lport = 0;
    uint32 index = 0;
    uint32 value = 0;
    uint32  port_type = 0;
    uint32  port_mac_id = 0;
    uint8   is_support_glb_port = _sys_usw_port_is_support_glb_port(lchip);
	uint8   gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport);

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get port property, gport:0x%04X, property:%d!\n", gport, port_prop);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_value);
    SYS_GLOBAL_CHIPID_CHECK(gchip);

	if (FALSE == sys_usw_chip_is_local(lchip, gchip))
	{
		return _sys_usw_port_get_global_property(lchip, gport, port_prop, p_value);
	}

	lport = CTC_MAP_GPORT_TO_LPORT(gport);
	if(lport >= MCHIP_CAP(SYS_CAP_PORT_NUM_PER_CHIP))
	{
		return CTC_E_INVALID_PORT;
	}
    /* zero value */
    *p_value = 0;
    index = lport;

    switch (port_prop)
    {
    case CTC_PORT_PROP_WLAN_DECAP_WITH_RID:
        cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_radioMacType_f);
        break;
    case CTC_PORT_PROP_DISCARD_UNENCRYPT_PKT:
        cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_discardPlainTextPacket_f);
        break;
    case CTC_PORT_PROP_RAW_PKT_TYPE:
        ret = _sys_usw_port_get_raw_packet_type(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_VLAN_DOMAIN:
        cmd = DRV_IOR(DsPhyPort_t, DsPhyPort_outerVlanIsCVlan_f);
        break;

    case CTC_PORT_PROP_PORT_EN:
        ret = _sys_usw_port_get_port_en(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_TRANSMIT_EN:      /**< get port whether the tranmist is enable */
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_transmitDisable_f);
        inverse_value = 1;
        break;

    case CTC_PORT_PROP_RECEIVE_EN:       /**< get port whether the receive is enable */
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_receiveDisable_f);
        inverse_value = 1;
        break;

    case CTC_PORT_PROP_BRIDGE_EN:        /**< get port whehter layer2 bridge function is enable */
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_bridgeEn_f);
        break;

    case CTC_PORT_PROP_DEFAULT_VLAN:     /**< Get default vlan id on the Port */
        cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_defaultVlanId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, p_value));

        return CTC_E_NONE;
        break;

    case CTC_PORT_PROP_UNTAG_PVID_TYPE:
        {
            uint32 r_value = 0;
            cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_untagDefaultVlanId_f):DRV_IOR(DsDestPort_t, DsDestPort_untagDefaultVlanId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &r_value));
            if (!r_value) /* none */
            {
                *p_value = CTC_PORT_UNTAG_PVID_TYPE_NONE;
            }
            else /* s or c */
            {
                cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_untagDefaultSvlan_f):DRV_IOR(DsDestPort_t, DsDestPort_untagDefaultSvlan_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &r_value));

                if (r_value) /* s */
                {
                    *p_value = CTC_PORT_UNTAG_PVID_TYPE_SVLAN;
                }
                else
                {
                    *p_value = CTC_PORT_UNTAG_PVID_TYPE_CVLAN;
                }
            }

            return CTC_E_NONE;

        }
        break;

    case CTC_PORT_PROP_VLAN_CTL:     /**< Get port's vlan tag control mode */
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_vlanTagCtl_f);
        break;

    case CTC_PORT_PROP_CROSS_CONNECT_EN:     /**< Get port cross connect */
        ret = _sys_usw_port_get_cross_connect(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_LEARNING_EN:     /**< Get learning enable/disable on port */
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_learningDisable_f);
        inverse_value = 1;
        break;

    case CTC_PORT_PROP_LEARN_DIS_MODE :
        if (!DRV_FLD_IS_EXISIT(DsSrcPort_t, DsSrcPort_learningDisableMode_f))
        {
            return CTC_E_NOT_SUPPORT;
        }
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_learningDisableMode_f);
        break;
    case CTC_PORT_PROP_FORCE_MPLS_EN:
        if (!DRV_FLD_IS_EXISIT(DsSrcPort_t, DsSrcPort_openflowMplsEn_f))
        {
            return CTC_E_NOT_SUPPORT;
        }
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_openflowMplsEn_f);
        break;

    case CTC_PORT_PROP_FORCE_TUNNEL_EN:
        if (!DRV_FLD_IS_EXISIT(DsSrcPort_t, DsSrcPort_openflowTunnelEn_f))
        {
            return CTC_E_NOT_SUPPORT;
        }
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_openflowTunnelEn_f);
        break;

    case CTC_PORT_PROP_DOT1Q_TYPE:     /**< Get port dot1q type */
        cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_dot1QEn_f):DRV_IOR(DsDestPort_t, DsDestPort_dot1QEn_f);
        break;

    case CTC_PORT_PROP_USE_OUTER_TTL:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_useOuterTtl_f);
        break;

    case CTC_PORT_PROP_PROTOCOL_VLAN_EN:     /**< Get protocol vlan enable on port */
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_protocolVlanEn_f);
        break;

    case CTC_PORT_PROP_SRC_DISCARD_EN:     /**< set port whether the src_discard is enable */
        cmd = DRV_IOR(DsPhyPort_t, DsPhyPort_srcDiscard_f);
        break;

    case CTC_PORT_PROP_PORT_CHECK_EN:     /**< set port whether the src port match check is enable */
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_portCheckEn_f);
        break;

    /*new properties for GB*/
    case CTC_PORT_PROP_HW_LEARN_EN:     /**< hardware learning enable */
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_fastLearningEn_f);
        break;

    case CTC_PORT_PROP_SCL_IPV6_LOOKUP_EN:
        ret = CTC_E_NOT_SUPPORT;
        break;

    case CTC_PORT_PROP_SCL_IPV4_LOOKUP_EN:
        ret = CTC_E_NOT_SUPPORT;
        break;

    case CTC_PORT_PROP_SCL_USE_DEFAULT_LOOKUP:
        cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_useDefaultVlanLookup_f);
        break;

    case CTC_PORT_PROP_SCL_FORCE_IPV6_TO_MAC:
        ret = CTC_E_NOT_SUPPORT;
        break;

    case CTC_PORT_PROP_SCL_FORCE_IPV4_TO_MAC:
        ret = CTC_E_NOT_SUPPORT;
        break;

    case CTC_PORT_PROP_TRILL_EN:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_trillEn_f);
        break;
    case CTC_PORT_PROP_TRILL_MCAST_DECAP_EN:
        cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_trillMcastLoopback_f);
        break;

    case CTC_PORT_PROP_DISCARD_NON_TRIL_PKT:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_discardNonTrill_f);
        break;

    case CTC_PORT_PROP_DISCARD_TRIL_PKT:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_discardTrill_f);
        break;

    case CTC_PORT_PROP_REFLECTIVE_BRIDGE_EN:
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_reflectiveBridgeEn_f);
        break;

    case CTC_PORT_PROP_FCOE_EN:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_fcoeEn_f);
        break;

    case CTC_PORT_PROP_RPF_EN:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_portBasedRpfEn_f);
        break;

    case CTC_PORT_PROP_FCMAP:
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_fcoeOuiIndex_f);
        break;

    case CTC_PORT_PROP_REPLACE_STAG_COS:
        cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_replaceStagCos_f):DRV_IOR(DsDestPort_t, DsDestPort_replaceStagCos_f);
        break;

    case CTC_PORT_PROP_REPLACE_CTAG_COS:
        cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_replaceCtagCos_f):DRV_IOR(DsDestPort_t, DsDestPort_replaceCtagCos_f);
        break;

    case CTC_PORT_PROP_REPLACE_STAG_TPID:
        cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_replaceStagTpid_f):DRV_IOR(DsDestPort_t, DsDestPort_replaceStagTpid_f);
        break;

    case CTC_PORT_PROP_REPLACE_DSCP_EN:
        cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_dscpRemarkMode_f):DRV_IOR(DsDestPort_t, DsDestPort_dscpRemarkMode_f);
        DRV_IOCTL(lchip, lport, cmd, &value);
        if(value == 2)
        {
            *p_value = 1;
        }
        cmd = 0;
        break;

    case CTC_PORT_PROP_L3PDU_ARP_ACTION:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_arpExceptionType_f);
        break;

    case CTC_PORT_PROP_L3PDU_DHCP_ACTION:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_dhcpV4ExceptionType_f);
        break;

    case CTC_PORT_PROP_TUNNEL_RPF_CHECK:
        cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_tunnelRpfCheck_f);
        break;

    case CTC_PORT_PROP_PTP_EN:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_ptpIndex_f);
        break;

    case CTC_PORT_PROP_RPF_TYPE:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_rpfType_f);
        break;

    case CTC_PORT_PROP_IS_LEAF:
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_isLeaf_f);
        break;

    case CTC_PORT_PROP_PKT_TAG_HIGH_PRIORITY:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_ingressTagHighPriority_f);
        break;

    case CTC_PORT_PROP_ROUTE_EN:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_routeDisable_f);
        inverse_value = 1;
        break;

    case CTC_PORT_PROP_PRIORITY_TAG_EN:
        cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_priorityTagEn_f):DRV_IOR(DsDestPort_t, DsDestPort_priorityTagEn_f);
        break;

    case CTC_PORT_PROP_IPG:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_ipgIndex_f);
        break;

    case CTC_PORT_PROP_DEFAULT_PCP:
        cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_defaultPcp_f):DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_defaultPcp_f);
        break;

    case CTC_PORT_PROP_DEFAULT_DEI:
        cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_defaultDei_f):DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_defaultDei_f);
        break;

    case CTC_PORT_PROP_NVGRE_MCAST_NO_DECAP:
        cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_nvgreMcastLoopbackMode_f);
        break;

    case CTC_PORT_PROP_VXLAN_MCAST_NO_DECAP:
        cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_vxlanMcastLoopbackMode_f);
        break;

    case CTC_PORT_PROP_QOS_POLICY:
        CTC_ERROR_RETURN(_sys_usw_port_unmap_qos_policy(lchip, lport, p_value));
        cmd = 0;
        break;

    case CTC_PORT_PROP_SCL_HASH_FIELD_SEL_ID:
        cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_sclFlowHashFieldSel_f);
        break;

    case CTC_PORT_PROP_SNOOPING_PARSER:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_forceSecondParser_f);
        break;

    case CTC_PORT_PROP_FLOW_LKUP_BY_OUTER_HEAD:
        cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_aclQosUseOuterInfo_f);
        break;
    case CTC_PORT_PROP_IPFIX_LKUP_BY_OUTER_HEAD:
        cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_ipfixAndMicroflowUseOuterInfo_f);
        break;
    case CTC_PORT_PROP_AWARE_TUNNEL_INFO_EN:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_aclKeyMergeInnerAndOuterHdr_f);
        break;

    case CTC_PORT_PROP_IPFIX_AWARE_TUNNEL_INFO_EN:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_ipfixAndMicroflowMergeHdr_f);
        break;

    case CTC_PORT_PROP_NVGRE_ENABLE:
        cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_nvgreEnable_f);
        break;
	case CTC_PORT_PROP_LOGIC_PORT_CHECK_EN:
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_logicPortCheckEn_f);
        break;
    case CTC_PORT_PROP_METADATA_OVERWRITE_PORT:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_gAcl_0_aclUseGlobalPortType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip,lport, cmd, p_value));
        if(*p_value == 3)
        {
            *p_value = 1;
        }
        else
        {
            *p_value = 0;
        }
        cmd = 0;
        break;

    case CTC_PORT_PROP_SRC_MISMATCH_EXCEPTION_EN:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_srcMismatchExceptionEn_f);
        break;

    case CTC_PORT_PROP_STATION_MOVE_PRIORITY:
        ret = sys_usw_l2_get_station_move(lchip, gport, SYS_L2_STATION_MOVE_OP_PRIORITY, p_value);
        break;

    case CTC_PORT_PROP_STATION_MOVE_ACTION:
        CTC_ERROR_RETURN(sys_usw_l2_get_station_move(lchip, gport, SYS_L2_STATION_MOVE_OP_ACTION, &value));

        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_srcMismatchExceptionEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, p_value));

        if (value == CTC_PORT_STATION_MOVE_ACTION_TYPE_FWD)
        {
            *p_value = CTC_PORT_STATION_MOVE_ACTION_TYPE_FWD;
        }
        else if (value == CTC_PORT_STATION_MOVE_ACTION_TYPE_DISCARD)
        {
            *p_value = *p_value ? CTC_PORT_STATION_MOVE_ACTION_TYPE_DISCARD_TOCPU : CTC_PORT_STATION_MOVE_ACTION_TYPE_DISCARD;
        }

        return CTC_E_NONE;
    case CTC_PORT_PROP_EFD_EN:
        cmd = DRV_IOR(IpePortEfdCtl_t, IpePortEfdCtl_array_0_efdEnable_f+(lport&0xFF));
        ret = DRV_IOCTL(lchip, (lport>>8), cmd, p_value);
        cmd = 0;
        break;

    case CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_addDefaultVlanDisable_f);
        break;

    case CTC_PORT_PROP_LOGIC_PORT:
         cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_defaultLogicSrcPort_f);
         break;

     case CTC_PORT_PROP_GPORT:
        ret = sys_usw_port_get_global_port(lchip, lport, &value16);
        if (CTC_E_NONE == ret)
        {
            *p_value = value16;
        }
        break;

    case CTC_PORT_PROP_APS_FAILOVER_EN:
        ret = _sys_usw_port_get_aps_failover_en(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_LINKAGG_FAILOVER_EN:
        ret = _sys_usw_port_get_linkagg_failover_en(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_APS_SELECT_GRP_ID:
        ret = _sys_usw_port_get_aps_select_grp_id(lchip, gport, p_value);
        break;

    case CTC_PORT_PROP_APS_SELECT_WORKING:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_apsSelectProtectingPath_f);
        inverse_value = 1;
        break;
    case CTC_PORT_PROP_CID:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_categoryId_f);
        break;
    case CTC_PORT_PROP_TX_CID_HDR_EN:
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_withCidHeader_f);
        break;
    case CTC_PORT_PROP_WLAN_PORT_TYPE:
        ret = _sys_usw_port_get_wlan_port_type(lchip, gport, p_value);
        break;
    case CTC_PORT_PROP_WLAN_PORT_ROUTE_EN:
        ret = _sys_usw_port_get_wlan_port_route(lchip, gport, p_value);
        break;
    case CTC_PORT_PROP_DSCP_SELECT_MODE:
        cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_dscpRemarkMode_f):DRV_IOR(DsDestPort_t, DsDestPort_dscpRemarkMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, p_value));
        cmd = 0;
        break;
    case CTC_PORT_PROP_DEFAULT_DSCP:
        cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_defaultDscp_f):DRV_IOR(DsDestPort_t, DsDestPort_defaultDscp_f);
        break;
    case CTC_PORT_PROP_REPLACE_PCP_DEI:
        cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_replacePortPcp_f):DRV_IOR(DsDestPort_t, DsDestPort_replacePortPcp_f);
        break;
    case CTC_PORT_PROP_CUT_THROUGH_EN:
        _sys_usw_port_get_cut_through_en(lchip,gport,p_value);
        cmd = 0;
        break;

    case CTC_PORT_PROP_SD_ACTION:
        ret = _sys_usw_port_get_sd_action(lchip, CTC_INGRESS, index, p_value);
        cmd = 0;
        break;

    case CTC_PORT_PROP_LOOP_WITH_SRC_PORT:
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_sourcePortToHeader_f);
        break;

    case CTC_PORT_PROP_LB_HASH_ECMP_PROFILE:
        if (!DRV_FLD_IS_EXISIT(DsSrcPort_t, DsSrcPort_portBasedHashProfileId0_f))
        {
            return CTC_E_NOT_SUPPORT;
        }
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_portBasedHashProfileId0_f);
        break;

    case CTC_PORT_PROP_LB_HASH_LAG_PROFILE:
        if (!DRV_FLD_IS_EXISIT(DsSrcPort_t, DsSrcPort_portBasedHashProfileId1_f))
        {
            return CTC_E_NOT_SUPPORT;
        }
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_portBasedHashProfileId1_f);
        break;
    case CTC_PORT_PROP_XPIPE_EN:
        {
            uint16 pmac_channel[24] = {4,5,6,7,16,17,18,19,8,9,10,11,24,25,26,27,40,41,42,43,56,57,58,59};
            uint16 emac_channel[24] = {0,1,2,3,20,21,22,23,12,13,14,15,28,29,30,31,44,45,46,47,60,61,62,63};
            for(index = 0; index < 24; index ++)
            {
                if(pmac_channel[index] == lport)
                {
                    gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, emac_channel[index]);
                    break;
                }
            }
            CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_PORT_TYPE, (void *)&port_type));
            if (SYS_DMPS_NETWORK_PORT != port_type)
            {
                return CTC_E_INVALID_CONFIG;
            }
            CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_MAC_ID, (void *)&port_mac_id));
            cmd = DRV_IOR(NetRxCtl_t, NetRxCtl_portSplitEn_f);
            DRV_IOCTL(lchip, 0, cmd, &value);
            /* index = macid[5:3] macid[1:0] */
            index = ((port_mac_id & 0x38) >> 1) | (port_mac_id & 0x3);
            *p_value = CTC_IS_BIT_SET(value, index);
            return CTC_E_NONE;
        }

    case CTC_PORT_PROP_MUX_TYPE:
        ret = _sys_usw_port_get_mux_type(lchip, gport, p_value);
        break;
    case CTC_PORT_PROP_PHY_INIT:
        ret = sys_usw_peri_get_phy_register_exist(lchip, lport);
        if (CTC_E_NOT_INIT == ret)
        {
            *p_value = FALSE;
            ret = CTC_E_NONE;
        }
        else
        {
            *p_value = ret?FALSE:TRUE;
        }
        break;
    case CTC_PORT_PROP_PHY_EN:
        if (CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport))
        {
            ret = sys_usw_peri_get_phy_prop(lchip, lport, CTC_PORT_PROP_PHY_EN, p_value);
        }
        else
        {
            ret = sys_usw_mac_get_mac_en(lchip, gport, p_value);
        }
        break;
    case CTC_PORT_PROP_PHY_DUPLEX:
    case CTC_PORT_PROP_PHY_MEDIUM:
    case CTC_PORT_PROP_PHY_LOOPBACK:
        if (CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport))
        {
            ret = sys_usw_peri_get_phy_prop(lchip, lport, port_prop, p_value);
        }
        break;
    default:
        if ((port_prop >= CTC_PORT_PROP_PHY_CUSTOM_BASE) && (port_prop <= CTC_PORT_PROP_PHY_CUSTOM_MAX_TYPE))
        {
            if (CTC_E_NONE == sys_usw_peri_get_phy_register_exist(lchip, lport))
            {
                ret = sys_usw_peri_get_phy_prop(lchip, lport, port_prop, p_value);
            }
        }
        else
        {
            return CTC_E_NOT_SUPPORT;
        }
    }

    if (ret != CTC_E_NONE)
    {
        return ret;
    }

    if (cmd == 0)
    {
        /*when cmd == 0, get value from  software memory*/
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, p_value));

    *p_value = inverse_value ? (!(*p_value)) : (*p_value);

    if (CTC_PORT_PROP_SD_ACTION == port_prop)
    {
        switch(*p_value)
        {
            case 5:
                *p_value = CTC_PORT_SD_ACTION_TYPE_DISCARD_OAM;
                break;
            case 6:
                *p_value = CTC_PORT_SD_ACTION_TYPE_DISCARD_DATA;
                break;
            case 7:
                *p_value = CTC_PORT_SD_ACTION_TYPE_DISCARD_BOTH;
                break;
            default:
                *p_value = CTC_PORT_SD_ACTION_TYPE_NONE;
                break;
        }
    }

    return CTC_E_NONE;
}


int32
_sys_usw_port_set_mlag_isolation(uint8 lchip, ctc_port_isolation_t* p_port_isolation)
{
    uint16 lport = 0;
    uint32 drv_port = 0;
    uint32 cmd = 0;
    uint8 loop = 0;
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!CTC_IS_LINKAGG_PORT(p_port_isolation->gport))
    {
         SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_port_isolation->gport, lchip, lport);
    }
    else if (MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM) <= (p_port_isolation->gport & 0xFF))
    {
         SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
         return CTC_E_INVALID_PARAM;
    }
    /*mlag port isolation*/
    if (!(p_port_isolation->is_linkagg))
    {

         drv_port = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port_isolation->gport);
         cmd = DRV_IOW(MetFifoIsolateCtl_t, MetFifoIsolateCtl_peerLinkSourcePort1_f);
         CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &drv_port));
         for (loop = 0; loop < 8; loop++)
         {
             p_port_isolation->pbm[loop] = ~(p_port_isolation->pbm[loop]);
         }
         p_port_isolation->pbm[7] = (p_port_isolation->pbm[7] | SYS_MLAG_ISOLATION_RESERVE_PORT_MASK);
         cmd = DRV_IOW(MetFifoIsolateCtl_t, MetFifoIsolateCtl_portNonBlockBitmap1_f);
         CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &p_port_isolation->pbm));

    }
    /*mlag linkagg isolation*/
    else
    {
        drv_port = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_port_isolation->gport);
        cmd = DRV_IOW(MetFifoIsolateCtl_t, MetFifoIsolateCtl_peerLinkSourcePort1_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &drv_port));
        for (loop = 0; loop < 2; loop++)
        {
           p_port_isolation->pbm[loop] = ~(p_port_isolation->pbm[loop]);
        }
         cmd = DRV_IOW(MetFifoIsolateCtl_t, MetFifoIsolateCtl_lagNonBlockBitmap1_f);
         CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &p_port_isolation->pbm));
    }
    return CTC_E_NONE;
}


int32
_sys_usw_port_get_mlag_isolation(uint8 lchip, ctc_port_isolation_t* p_port_isolation)
{
    uint32 drv_port = 0;
    uint32 cmd = 0;
    uint8 loop = 0;
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOR(MetFifoIsolateCtl_t, MetFifoIsolateCtl_peerLinkSourcePort1_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &drv_port));
    p_port_isolation->gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(drv_port);

    if (!(p_port_isolation->is_linkagg))
    {
        cmd = DRV_IOR(MetFifoIsolateCtl_t, MetFifoIsolateCtl_portNonBlockBitmap1_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &p_port_isolation->pbm));
        for (loop = 0; loop < 8; loop++)
        {
            p_port_isolation->pbm[loop] = ~(p_port_isolation->pbm[loop]);
        }
    }
    else
    {
        cmd = DRV_IOR(MetFifoIsolateCtl_t, MetFifoIsolateCtl_lagNonBlockBitmap1_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &p_port_isolation->pbm));
        for (loop = 0; loop < 2; loop++)
        {
            p_port_isolation->pbm[loop] = ~(p_port_isolation->pbm[loop]);
        }
    }
    return CTC_E_NONE;
}

int32
_sys_usw_port_check_restriction_mode(uint8 lchip, uint32 gport, ctc_port_restriction_t* p_restriction)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint16 lport = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set port property, gport:0x%04X\n", gport);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_restriction);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    p_restriction->mode = CTC_PORT_RESTRICTION_DISABLE;

     if (CTC_INGRESS == p_restriction->dir)
    {
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_srcPortIsolateId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
        value = (value & 0x1F) | ((value >> 1) & 0x20);
    }
    else if (CTC_EGRESS == p_restriction->dir)
    {
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_destPortIsolateId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    if (0 != value)
    {
        if (0x30 == (value & 0x38))
        {
            p_restriction->mode = CTC_PORT_RESTRICTION_PVLAN;
            p_restriction->type = CTC_PORT_PVLAN_PROMISCUOUS;
        }
        else if  (0x38 == (value & 0x38))
        {
            p_restriction->mode = CTC_PORT_RESTRICTION_PVLAN;
            p_restriction->type = CTC_PORT_PVLAN_ISOLATED;
        }
        else if (0x20 == (value & 0x30))
        {
            p_restriction->mode = CTC_PORT_RESTRICTION_PVLAN;
            p_restriction->type = CTC_PORT_PVLAN_COMMUNITY;
            p_restriction->isolated_id = value & (~((uint32)0x30));
        }
        else
        {
            p_restriction->mode = CTC_PORT_RESTRICTION_PORT_ISOLATION;

            if (CTC_INGRESS == p_restriction->dir)
            {
                p_restriction->isolated_id = value;
            }
            else if (CTC_EGRESS == p_restriction->dir)
            {
                p_restriction->isolated_id = value;
            }
        }
    }
    return CTC_E_NONE;
}

/**
@brief    Set port's restriction
*/
int32
sys_usw_port_set_restriction(uint8 lchip, uint32 gport, ctc_port_restriction_t* p_restriction)
{
    int32  ret = CTC_E_NONE;

    uint16 lport = 0;
    uint32 value = 0;
    uint32 value_igs = 0;
    uint32 cmd = 0;
    ctc_port_restriction_t restriction;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set port property, gport:0x%04X, mode:%d, isolated_id:%d, dir:%d\n", \
                     gport, p_restriction->mode, p_restriction->isolated_id, p_restriction->dir);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_restriction);
#if 0
    if (CTC_PORT_RESTRICTION_PORT_ISOLATION == p_restriction->mode
        && SYS_USW_MLAG_ISOLATION_GROUP == p_restriction->isolate_group)
    {
        CTC_ERROR_RETURN(sys_usw_port_set_mlag_isolation(lchip, gport, p_restriction));
        return CTC_E_NONE;
    }
#endif
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    sal_memset(&restriction, 0, sizeof(ctc_port_restriction_t));

    restriction.dir = CTC_INGRESS;
    CTC_ERROR_RETURN(_sys_usw_port_check_restriction_mode(lchip, gport, &restriction));
    if (((CTC_PORT_RESTRICTION_PVLAN == restriction.mode) && (CTC_PORT_RESTRICTION_PORT_ISOLATION == p_restriction->mode))
       || ((CTC_PORT_RESTRICTION_PORT_ISOLATION == restriction.mode) && (CTC_PORT_RESTRICTION_PVLAN == p_restriction->mode)))
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " port has another restriction feature \n");
        return CTC_E_INVALID_CONFIG;
    }

    restriction.dir = CTC_EGRESS;
    CTC_ERROR_RETURN(_sys_usw_port_check_restriction_mode(lchip, gport, &restriction));
    if (((CTC_PORT_RESTRICTION_PVLAN == restriction.mode) && (CTC_PORT_RESTRICTION_PORT_ISOLATION == p_restriction->mode))
       || ((CTC_PORT_RESTRICTION_PORT_ISOLATION == restriction.mode) && (CTC_PORT_RESTRICTION_PVLAN == p_restriction->mode)))
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " port has another restriction feature \n");
        return CTC_E_INVALID_CONFIG;
    }

    switch (p_restriction->mode)
    {
    case CTC_PORT_RESTRICTION_PVLAN:

        if (CTC_PORT_PVLAN_PROMISCUOUS == p_restriction->type)
        {
            /* port_isolate_id[5:0] = 6'b110XXX */
            value_igs = 0x50;
            value = 0x30;
        }
        else if (CTC_PORT_PVLAN_ISOLATED == p_restriction->type)
        {
            /* port_isolate_id[5:0] = 6'b111XXX */
            value_igs = 0x58;
            value = 0x38;
        }
        else if (CTC_PORT_PVLAN_COMMUNITY == p_restriction->type)
        {
            value = p_restriction->isolated_id;
            CTC_MAX_VALUE_CHECK(value, MCHIP_CAP(SYS_CAP_PVLAN_COMMUNITY_ID_NUM));
            /* port_isolate_id[5:0] = 6'b10XXXX */
            value_igs = (value | 0x40);
            value = (value | 0x20);
        }
        else if (CTC_PORT_PVLAN_NONE == p_restriction->type)
        {
            value_igs = 0;
            value = 0;
        }
        else
        {
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
            return CTC_E_INVALID_PARAM;
        }

        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_srcPortIsolateId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value_igs));

        cmd = DRV_IOW(DsDestPort_t, DsDestPort_destPortIsolateId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
        break;

    case CTC_PORT_RESTRICTION_PORT_ISOLATION:
        CTC_MAX_VALUE_CHECK(p_restriction->isolated_id, (SYS_USW_ISOLATION_GROUP_NUM - 1));
        CTC_MAX_VALUE_CHECK(p_restriction->type, CTC_PORT_ISOLATION_ALL + CTC_PORT_ISOLATION_UNKNOW_UCAST\
                                                 + CTC_PORT_ISOLATION_UNKNOW_MCAST + CTC_PORT_ISOLATION_KNOW_UCAST\
                                                 + CTC_PORT_ISOLATION_KNOW_MCAST + CTC_PORT_ISOLATION_BCAST);
        if (0 == p_usw_port_master[lchip]->use_isolation_id)
        {
              SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " port has another restriction feature \n");
              return CTC_E_INVALID_CONFIG;

        }
        if ((CTC_INGRESS == p_restriction->dir) || (CTC_BOTH_DIRECTION == p_restriction->dir))
        {
            value = p_restriction->isolated_id;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_srcPortIsolateId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
        }
        if ((CTC_EGRESS == p_restriction->dir) || (CTC_BOTH_DIRECTION == p_restriction->dir))
        {
            value = p_restriction->isolated_id;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_destPortIsolateId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
        }

#if 0
        if ((0 == p_restriction->isolate_group) || (0 == p_restriction->isolated_id))
        {
            p_restriction->isolate_group = 0;
            p_restriction->isolated_id = 0;
            p_restriction->type = 0;
        }

        if ((CTC_EGRESS == p_restriction->dir) || (CTC_BOTH_DIRECTION == p_restriction->dir))
        {
            CTC_ERROR_RETURN(_sys_usw_port_set_port_isolation(lchip, p_restriction->isolate_group, p_restriction->isolated_id, p_restriction->type));
        }
#endif
        break;

    case CTC_PORT_RESTRICTION_PORT_BLOCKING:
        CTC_MAX_VALUE_CHECK(p_restriction->type, CTC_PORT_BLOCKING_UNKNOW_UCAST\
                                                 + CTC_PORT_BLOCKING_UNKNOW_MCAST + CTC_PORT_BLOCKING_KNOW_UCAST\
                                                 + CTC_PORT_BLOCKING_KNOW_MCAST + CTC_PORT_BLOCKING_BCAST);
        CTC_ERROR_RETURN(_sys_usw_port_set_port_blocking(lchip, gport, p_restriction->type));
        break;

    case CTC_PORT_RESTRICTION_DISABLE:
        value = 0;
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_srcPortIsolateId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_destPortIsolateId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));

        CTC_ERROR_RETURN(_sys_usw_port_set_port_blocking(lchip, gport, 0));
        break;

    default:
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
        return CTC_E_INVALID_PARAM;
    }

    return ret;
}

/**
@brief    Get port's restriction
*/
int32
sys_usw_port_get_restriction(uint8 lchip, uint32 gport, ctc_port_restriction_t* p_restriction)
{
    uint16 lport = 0;
    uint32 value = 0;
    uint32 cmd = 0;
    uint16 block_type = 0;
    ctc_port_restriction_t restriction;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set port property, gport:0x%04X\n", gport);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_restriction);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    sal_memset(&restriction, 0, sizeof(ctc_port_restriction_t));

    restriction.dir = CTC_INGRESS;
    CTC_ERROR_RETURN(_sys_usw_port_check_restriction_mode(lchip, gport, &restriction));
    if (((CTC_PORT_RESTRICTION_PVLAN == restriction.mode) && (CTC_PORT_RESTRICTION_PORT_ISOLATION == p_restriction->mode)))
    {
        p_restriction->isolated_id = 0;
        return CTC_E_NONE;
    }
    else if (((CTC_PORT_RESTRICTION_PORT_ISOLATION == restriction.mode) && (CTC_PORT_RESTRICTION_PVLAN == p_restriction->mode)))
    {
        p_restriction->type = CTC_PORT_PVLAN_NONE;
        return CTC_E_NONE;
    }

    restriction.dir = CTC_EGRESS;
    CTC_ERROR_RETURN(_sys_usw_port_check_restriction_mode(lchip, gport, &restriction));
    if (((CTC_PORT_RESTRICTION_PVLAN == restriction.mode) && (CTC_PORT_RESTRICTION_PORT_ISOLATION == p_restriction->mode)))
    {
        p_restriction->isolated_id = 0;
        return CTC_E_NONE;
    }
    else if (((CTC_PORT_RESTRICTION_PORT_ISOLATION == restriction.mode) && (CTC_PORT_RESTRICTION_PVLAN == p_restriction->mode)))
    {
        p_restriction->type = CTC_PORT_PVLAN_NONE;
        return CTC_E_NONE;
    }

    if (CTC_INGRESS == p_restriction->dir)
    {
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_srcPortIsolateId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
        value = (value & 0x1F) | ((value >> 1) & 0x20);
    }
    else if (CTC_EGRESS == p_restriction->dir)
    {
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_destPortIsolateId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    switch (p_restriction->mode)
    {
        case CTC_PORT_RESTRICTION_DISABLE:
            break;
        case CTC_PORT_RESTRICTION_PVLAN:
            if (0x30 == (value & 0x38))
            {
                p_restriction->type = CTC_PORT_PVLAN_PROMISCUOUS;
            }
            else if  (0x38 == (value & 0x38))
            {
                p_restriction->type = CTC_PORT_PVLAN_ISOLATED;
            }
            else if (0x20 == (value & 0x30))
            {
                p_restriction->type = CTC_PORT_PVLAN_COMMUNITY;
                p_restriction->isolated_id = value & (~((uint32)0x30));
            }
            break;
        case CTC_PORT_RESTRICTION_PORT_BLOCKING:
            CTC_ERROR_RETURN(_sys_usw_port_get_port_blocking(lchip, gport, &block_type));
            if (0 != block_type)
            {
                p_restriction->type = block_type;
            }
            break;
        case CTC_PORT_RESTRICTION_PORT_ISOLATION:
            if (0 == p_usw_port_master[lchip]->use_isolation_id)
            {
                  SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " port has another restriction feature \n");
                  return CTC_E_INVALID_CONFIG;
            }

            if (CTC_INGRESS == p_restriction->dir)
            {
                p_restriction->isolated_id = value;
            }
            else if (CTC_EGRESS == p_restriction->dir)
            {
                p_restriction->isolated_id = value;
            }
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }
    return CTC_E_NONE;
}

int32
_sys_usw_port_set_isolation_id(uint8 lchip, ctc_port_isolation_t* p_port_isolation)
{
    uint32 igs_isolation_id = 0;
    uint32 igs_isolation_id_tmp = 0;
    uint32 igs_isolation_bitmap[2];
    uint16 lport = 0;
    uint16 loop = 0;
    uint16 entry_num = 0;
    uint32 cmd = 0;
    uint8 isolation_group = 0;
    DsPortBlockMask_m ds_port_blockmask;

    /***************************************
     Ingress isolation resource
     ***************************************/
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_port_isolation->gport, lchip, lport);

    sal_memset(&ds_port_blockmask, 0, sizeof(DsPortBlockMask_m));
    sal_memcpy(igs_isolation_bitmap, p_usw_port_master[lchip]->igs_isloation_bitmap, sizeof(p_usw_port_master[lchip]->igs_isloation_bitmap));

    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_srcPortIsolateId_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &igs_isolation_id));

    switch (p_usw_port_master[lchip]->isolation_group_mode)
    {
         case CTC_ISOLATION_GROUP_MODE_64:
             entry_num = 2;
             break;
         case CTC_ISOLATION_GROUP_MODE_32:
             entry_num = 4;
             break;
         case CTC_ISOLATION_GROUP_MODE_16:
             entry_num = 8;
             break;
         default:
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
            return CTC_E_INVALID_PARAM;
     }

    if (0 == igs_isolation_id)
    {
        uint8 bit = 0;
        for (loop = 0; loop < entry_num; loop++)
        {
            if (0 != p_port_isolation->pbm[loop])
            {
                break;
            }
            if (entry_num == (loop + 1))
            {
                return CTC_E_NONE;
            }
        }
        for (bit = 1; bit < SYS_USW_ISOLATION_ID_NUM; bit++)
        {
            if (!CTC_IS_BIT_SET(igs_isolation_bitmap[bit/32], bit%32))
            {
                igs_isolation_id = bit;
                break;
            }
        }
        if (SYS_USW_ISOLATION_ID_NUM == bit)
        {
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
            return CTC_E_NO_RESOURCE;

        }

        CTC_BIT_SET(igs_isolation_bitmap[bit/32], bit%32);

        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "src-port: %d, igs_isolation_bitmap0: 0x%x,  igs_isolation_bitmap1: 0x%x,igs_isolation_id:%d, allocate\n",
                         lport, igs_isolation_bitmap[0], igs_isolation_bitmap[1],igs_isolation_id);

         cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_srcPortIsolateId_f);
         CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &igs_isolation_id));

    }
    else
    {
        for (loop = 0; loop < entry_num; loop++)
        {
             if (0 != p_port_isolation->pbm[loop])
             {
                 break;
             }
        }

        /*clear isolation, recycle resource */
        if (entry_num == loop)
        {
            CTC_BIT_UNSET(igs_isolation_bitmap[igs_isolation_id/32], igs_isolation_id%32);
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_srcPortIsolateId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &igs_isolation_id_tmp));
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "recycle the old isolation id:%d, src-port: %d, igs_isolation_bitmap0: 0x%x,igs_isolation_bitmap1: 0x%x\n",
                         igs_isolation_id, lport, igs_isolation_bitmap[0],igs_isolation_bitmap[1]);
        }else
        {
             SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "src-port: %d, igs_isolation_bitmap: 0x%x, igs_isolation_bitmap1: 0x%x,igs_isolation_id:%d, Update\n",
                         lport, igs_isolation_bitmap[0],igs_isolation_bitmap[1], igs_isolation_id);
        }
    }

    for (loop = 0; loop < entry_num; loop++)
    {
         isolation_group = igs_isolation_id * entry_num + loop;
         SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "isolation_group: %d\n", isolation_group);
         cmd = DRV_IOR(DsPortBlockMask_t, DRV_ENTRY_FLAG);
         CTC_ERROR_RETURN(DRV_IOCTL(lchip, isolation_group, cmd, &ds_port_blockmask));
         if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
            CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_UNKNOW_UCAST))
         {
            SetDsPortBlockMask(A, array_0_blockMask_f, &ds_port_blockmask, &p_port_isolation->pbm[loop]);
         }
         if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
            CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_UNKNOW_MCAST))
         {
            SetDsPortBlockMask(A, array_1_blockMask_f, &ds_port_blockmask, &p_port_isolation->pbm[loop]);
         }
         if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
            CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_KNOW_UCAST))
         {
            SetDsPortBlockMask(A, array_2_blockMask_f, &ds_port_blockmask, &p_port_isolation->pbm[loop]);
         }
         if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
            CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_KNOW_MCAST))
         {
            SetDsPortBlockMask(A, array_3_blockMask_f, &ds_port_blockmask, &p_port_isolation->pbm[loop]);
         }
         if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
            CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_BCAST))
         {
            SetDsPortBlockMask(A, array_4_blockMask_f, &ds_port_blockmask, &p_port_isolation->pbm[loop]);
         }
         cmd = DRV_IOW(DsPortBlockMask_t, DRV_ENTRY_FLAG);
         CTC_ERROR_RETURN(DRV_IOCTL(lchip, isolation_group, cmd, &ds_port_blockmask));
     }

    sal_memcpy(p_usw_port_master[lchip]->igs_isloation_bitmap, igs_isolation_bitmap, sizeof(p_usw_port_master[lchip]->igs_isloation_bitmap));

    return CTC_E_NONE;
}

int32
sys_usw_port_set_isolation_mode(uint8 lchip, uint8 choice_mode)
{
    SYS_PORT_INIT_CHECK();
    PORT_LOCK;
    if (0 == choice_mode)
    {
        p_usw_port_master[lchip]->use_isolation_id = 0;
    }
    else if (1 == choice_mode)
    {
        p_usw_port_master[lchip]->use_isolation_id = 1;
    }
    PORT_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_usw_port_get_isolation_mode(uint8 lchip, uint8* p_choice_mode)
{
    SYS_PORT_INIT_CHECK();
    PORT_LOCK;
    if (p_choice_mode)
    {
        *p_choice_mode = p_usw_port_master[lchip]->use_isolation_id;
    }
    else
    {
        PORT_UNLOCK;
        return CTC_E_INVALID_PTR;
    }
    PORT_UNLOCK;
    return CTC_E_NONE;
}

int32
sys_usw_port_set_isolation(uint8 lchip, ctc_port_isolation_t* p_port_isolation)
{
    uint32 cmd = 0;
    uint32 isolation_group = 0;
    uint32 pbm = 0;
    uint32 igs_isolation_id = 0;
    uint16 lport = 0;
    DsPortIsolation_m ds_port_isolation;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_port_isolation);

    if (p_port_isolation->mlag_en)
    {
        if (p_port_isolation->use_isolation_id)
        {
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
            return CTC_E_INVALID_PARAM;
        }
        CTC_ERROR_RETURN(_sys_usw_port_set_mlag_isolation(lchip, p_port_isolation));
        return CTC_E_NONE;
    }


    if (p_port_isolation->use_isolation_id != p_usw_port_master[lchip]->use_isolation_id)
    {

SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " port has another restriction feature \n");
          return CTC_E_INVALID_CONFIG;

    }
    if (1 == p_port_isolation->use_isolation_id)
    {
        isolation_group = p_port_isolation->gport;
        CTC_MAX_VALUE_CHECK(isolation_group, (SYS_USW_ISOLATION_GROUP_NUM - 1));
        if (0 == p_port_isolation->gport || 1 == p_port_isolation->pbm[0])
        {
            return CTC_E_INVALID_PARAM;
        }
        pbm = p_port_isolation->pbm[0];
        sal_memset(&ds_port_isolation, 0, sizeof(DsPortIsolation_m));
        cmd = DRV_IOR(DsPortIsolation_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, isolation_group, cmd, &ds_port_isolation));
        if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
            CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_UNKNOW_UCAST))
        {
            SetDsPortIsolation(A, unknownUcIsolate_f, &ds_port_isolation, &pbm);
        }
        if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
            CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_KNOW_UCAST))
        {
            SetDsPortIsolation(A, knownUcIsolate_f, &ds_port_isolation, &pbm);
        }
        if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
            CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_UNKNOW_MCAST))
        {
            SetDsPortIsolation(A, unknownMcIsolate_f, &ds_port_isolation, &pbm);
        }
        if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
            CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_KNOW_MCAST))
        {
            SetDsPortIsolation(A, knownMcIsolate_f, &ds_port_isolation, &pbm);
        }
        if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
            CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_BCAST))
        {
            SetDsPortIsolation(A, bcIsolate_f, &ds_port_isolation, &pbm);
        }
        cmd = DRV_IOW(DsPortIsolation_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, isolation_group, cmd, &ds_port_isolation));

    }
    else if (0 == p_port_isolation->use_isolation_id)
    {
        /*verifying whether the port is configurated other mode*/
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_port_isolation->gport, lchip, lport);
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_srcPortIsolateId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &igs_isolation_id));
        if (CTC_IS_BIT_SET(igs_isolation_id, 6))
        {
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " port has another restriction feature \n");
            return CTC_E_INVALID_CONFIG;
        }
        CTC_ERROR_RETURN(_sys_usw_port_set_isolation_id(lchip, p_port_isolation));
    }
    else
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
        return CTC_E_INVALID_PARAM;
    }


    return CTC_E_NONE;
}

int32
sys_usw_port_get_isolation(uint8 lchip, ctc_port_isolation_t* p_port_isolation)
{
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_port_isolation);

    if (p_port_isolation->mlag_en)
    {
        CTC_ERROR_RETURN(_sys_usw_port_get_mlag_isolation(lchip, p_port_isolation));
        return CTC_E_NONE;
    }

    if (1 == p_usw_port_master[lchip]->use_isolation_id)
    {
        uint32 cmd = 0;
        uint32 isolation_group = 0;

        isolation_group = p_port_isolation->gport;
        CTC_MAX_VALUE_CHECK(isolation_group, (SYS_USW_ISOLATION_GROUP_NUM - 1));

        if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
            CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_UNKNOW_UCAST))
        {
            cmd = DRV_IOR(DsPortIsolation_t, DsPortIsolation_unknownUcIsolate_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, isolation_group, cmd, &p_port_isolation->pbm[0]));
        }
        if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
            CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_KNOW_UCAST))
        {
            cmd = DRV_IOR(DsPortIsolation_t, DsPortIsolation_knownUcIsolate_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, isolation_group, cmd, &p_port_isolation->pbm[0]));
        }
        if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
            CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_UNKNOW_MCAST))
        {
            cmd = DRV_IOR(DsPortIsolation_t, DsPortIsolation_unknownMcIsolate_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, isolation_group, cmd, &p_port_isolation->pbm[0]));
        }
        if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
            CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_KNOW_MCAST))
        {
            cmd = DRV_IOR(DsPortIsolation_t, DsPortIsolation_knownMcIsolate_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, isolation_group, cmd, &p_port_isolation->pbm[0]));
        }
        if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
            CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_BCAST))
        {
            cmd = DRV_IOR(DsPortIsolation_t, DsPortIsolation_bcIsolate_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, isolation_group, cmd, &p_port_isolation->pbm[0]));
        }
        p_port_isolation->use_isolation_id = 1;
    }
    else if (0 == p_usw_port_master[lchip]->use_isolation_id)
    {
        uint32 cmd = 0;
        uint32 igs_isolation_id = 0;
        uint16 lport = 0;
        uint32 isolation_group_mode = 0;
        uint8 isolation_group = 0;
        uint16 loop = 0;
        uint16 entry_num = 0;

        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_port_isolation->gport, lchip, lport);

        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_srcPortIsolateId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &igs_isolation_id));

        if (CTC_IS_BIT_SET(igs_isolation_id, 6))
        {
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "This port has been configured another mode! \n");
            return CTC_E_NONE;
        }

        cmd = DRV_IOR(QWritePortIsolateCtl_t, QWritePortIsolateCtl_portIsolateMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &isolation_group_mode));

        switch (isolation_group_mode)
        {
             case CTC_ISOLATION_GROUP_MODE_64:
                 entry_num = 2;
                 break;
             case CTC_ISOLATION_GROUP_MODE_32:
                 entry_num = 4;
                 break;
             case CTC_ISOLATION_GROUP_MODE_16:
                 entry_num = 8;
                 break;
             default:
                 SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
                 return CTC_E_INVALID_PARAM;
        }

        for (loop = 0; loop < entry_num; loop++)
        {
            isolation_group = igs_isolation_id * entry_num + loop;
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "isolation_group: %d\n", isolation_group);
            if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
                CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_UNKNOW_UCAST))
            {
                cmd = DRV_IOR(DsPortBlockMask_t, DsPortBlockMask_array_0_blockMask_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, isolation_group, cmd, &p_port_isolation->pbm[loop]));
            }
            if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
                CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_UNKNOW_MCAST))
            {
                cmd = DRV_IOR(DsPortBlockMask_t, DsPortBlockMask_array_1_blockMask_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, isolation_group, cmd, &p_port_isolation->pbm[loop]));
            }
            if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
                CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_KNOW_UCAST))
            {
                cmd = DRV_IOR(DsPortBlockMask_t, DsPortBlockMask_array_2_blockMask_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, isolation_group, cmd, &p_port_isolation->pbm[loop]));
            }
            if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
                CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_KNOW_MCAST))
            {
                cmd = DRV_IOR(DsPortBlockMask_t, DsPortBlockMask_array_3_blockMask_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, isolation_group, cmd, &p_port_isolation->pbm[loop]));
            }
            if (CTC_PORT_ISOLATION_ALL == p_port_isolation->isolation_pkt_type ||
                CTC_FLAG_ISSET(p_port_isolation->isolation_pkt_type, CTC_PORT_ISOLATION_BCAST))
            {
                cmd = DRV_IOR(DsPortBlockMask_t, DsPortBlockMask_array_4_blockMask_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, isolation_group, cmd, &p_port_isolation->pbm[loop]));
            }
        }

        p_port_isolation->use_isolation_id = 0;
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_port_set_random_log_percent(uint8 lchip, uint32 gport, ctc_direction_t dir, uint32 percent)
{
    uint16 lport = 0;
    uint32 value = 0;
    uint32 cmd = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set port gport:0x%04X, direction:%d, percent:%d\n", \
                     gport, dir, percent);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    CTC_MAX_VALUE_CHECK(percent, 100);
    /* randomLogShift = 5 */
    if (100 > percent)
    {
        value = ((MCHIP_CAP(SYS_CAP_RANDOM_LOG_THRESHOD)/100)/32) * percent;
    }
    else
    {
        value = 0x7FFF;
    }

    if (CTC_INGRESS == dir)
    {
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_randomThreshold_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
    }
    else if (CTC_EGRESS == dir)
    {
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_randomThreshold_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_port_get_random_log_percent(uint8 lchip, uint32 gport, ctc_direction_t dir, uint32* p_percent)
{
    int32 ret = CTC_E_NONE;

    uint16 lport = 0;
    uint32 value = 0;
    uint32 cmd = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set port gport:0x%04X, direction:%d\n", gport, dir);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    if (CTC_INGRESS == dir)
    {
        cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_randomThreshold_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &value);
        if (CTC_E_NONE == ret)
        {
            /* randomLogShift = 5 */
            *p_percent = value  / ((MCHIP_CAP(SYS_CAP_RANDOM_LOG_THRESHOD)/100)/32);
        }
    }
    else if (CTC_EGRESS == dir)
    {
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_randomThreshold_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &value);
        if (CTC_E_NONE == ret)
        {
            /* randomLogShift = 5 */
            *p_percent = value  / ((MCHIP_CAP(SYS_CAP_RANDOM_LOG_THRESHOD)/100)/32);
        }
    }

    return ret;
}

STATIC int32
_sys_usw_port_set_random_log_rate(uint8 lchip, uint32 gport, ctc_direction_t dir, uint32 rate)
{
    int32 ret = CTC_E_NONE;

    uint16 lport = 0;
    uint32 cmd = 0;

    /*Sanity check*/
    SYS_USW_PORT_INIT_CHECK();

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set port gport:0x%04X, direction:%d, rate:%d\n", \
                     gport, dir, rate);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    CTC_MAX_VALUE_CHECK(rate, MCHIP_CAP(SYS_CAP_RANDOM_LOG_RATE) - 1);

    /* randomLogShift = 5*/
    rate = rate ;
    if (CTC_INGRESS == dir)
    {
        cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_randomThreshold_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &rate);
    }
    else if (CTC_EGRESS == dir)
    {
        cmd = DRV_IOW(DsDestPort_t, DsDestPort_randomThreshold_f);
        ret = DRV_IOCTL(lchip, lport, cmd, &rate);
    }

    return ret;
}

/**
@brief  Set port's internal properties with direction according to gport id
*/
int32
sys_usw_port_set_direction_property(uint8 lchip, uint32 gport, ctc_port_direction_property_t port_prop, ctc_direction_t dir, uint32 value)
{
    int32 ret = CTC_E_NONE;

    uint16 drv_lport = 0;
    uint32 cmd = 0;
    uint32 temp;
    uint32 egress_value = value;
    uint32 acl_lkup_type = 0;
    uint32 acl_gport_type = 0;
    uint32 dot1ae_en[8] = {0};
    QWriteDot1AeCtl_m ds_ctl;
    uint32 index = 0;
    uint8  is_support_glb_port = _sys_usw_port_is_support_glb_port(lchip);
	uint8   gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport);
    uint32  random_en = 0;
    uint32 valid = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set port property with direction, gport:0x%04X, property:%d, dir:%d,\
                                            value:%d\n", gport, port_prop, dir, value);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_MAX_VALUE_CHECK(dir, CTC_BOTH_DIRECTION);
    SYS_GLOBAL_CHIPID_CHECK(gchip);
	if (FALSE == sys_usw_chip_is_local(lchip, gchip))
	{
        if(dir != CTC_EGRESS)
        {
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Only support config global port's egress property\n");
            return CTC_E_NONE;
        }
		return _sys_usw_port_set_global_property(lchip, gport, port_prop, value);
	}
	drv_lport = CTC_MAP_GPORT_TO_LPORT(gport);
    if(CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0 == port_prop || CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_1 == port_prop ||
       CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_2 == port_prop || CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_3 == port_prop)
    {
        CTC_MAX_VALUE_CHECK(value, CTC_ACL_TCAM_LKUP_TYPE_MAX -1);
        if(CTC_ACL_TCAM_LKUP_TYPE_VLAN == value || value == CTC_ACL_TCAM_LKUP_TYPE_COPP)
        {
            return CTC_E_NOT_SUPPORT;        /* vlan not support, copp only support global configure */
        }
        acl_lkup_type = sys_usw_map_acl_tcam_lkup_type(lchip, value);
    }
    /*do write*/
    if ((CTC_INGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
    {
        switch (port_prop)
        {
        case CTC_PORT_DIR_PROP_STAG_TPID_INDEX:
            if (value > 3 && (0xff != value))
            {
                return CTC_E_INVALID_PARAM;
            }
            cmd = DRV_IOW(DsPhyPort_t, DsPhyPort_svlanTpidIndex_f);
            break;

        case CTC_PORT_DIR_PROP_VLAN_FILTER_EN:
            value = (value) ? SYS_PORT_VLAN_FILTER_MODE_ENABLE : SYS_PORT_VLAN_FILTER_MODE_DISABLE;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_ingressFilteringMode_f);
            break;

        case CTC_PORT_DIR_PROP_RANDOM_LOG_EN:
            if ((drv_lport>=SYS_PHY_PORT_NUM_PER_SLICE) && !DRV_IS_DUET2(lchip))        /*TM random log only support lport 0-63*/
            {
                return CTC_E_NOT_SUPPORT;
            }
            cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_randomLogEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &random_en));
            value = (value) ? 1 : 0;
            cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_randomLogEn_f);
            index = drv_lport;
            if ((random_en^value) && !DRV_IS_DUET2(lchip))      /*TM only change random en will call the function*/
            {
                CTC_ERROR_RETURN(_sys_usw_port_set_seed_id(lchip, drv_lport, CTC_INGRESS, value));
            }
            break;

        case CTC_PORT_DIR_PROP_RANDOM_LOG_PERCENT:
            ret = _sys_usw_port_set_random_log_percent(lchip, gport, CTC_INGRESS, value);
            break;

        case CTC_PORT_DIR_PROP_RANDOM_LOG_RATE:
            ret = _sys_usw_port_set_random_log_rate(lchip, gport, CTC_INGRESS, value);
            break;
/********** ACL property begin **********/
        case CTC_PORT_DIR_PROP_ACL_EN:

            CTC_MAX_VALUE_CHECK(value, CTC_ACL_EN_0 + CTC_ACL_EN_1 + CTC_ACL_EN_2 + CTC_ACL_EN_3
                                       );

            if (0 == value)     /* if acl dis, set acl lookup type = 0 */
            {
                cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_0_aclLookupType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));
                cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_1_aclLookupType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));
                cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_2_aclLookupType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));
                cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_3_aclLookupType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));
            }
            else
            {
                /*when acl-en,tcam lookup type is L2 and global port type is use port bitmap
                                                                           for compatible GG*/
                acl_lkup_type = SYS_ACL_TCAM_LKUP_TYPE_L2;
                acl_gport_type = 0;
                if(value & 0x1)
                {
                    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_0_aclLookupType_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_lkup_type));
                    cmd = DRV_IOW(DsSrcPort_t,DsSrcPort_gAcl_0_aclUseGlobalPortType_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip,drv_lport,cmd,&acl_gport_type));
                }
                if(value & 0x2)
                {
                    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_1_aclLookupType_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_lkup_type));
                    cmd = DRV_IOW(DsSrcPort_t,DsSrcPort_gAcl_1_aclUseGlobalPortType_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip,drv_lport,cmd,&acl_gport_type));
                }
                if(value & 0x4)
                {
                    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_2_aclLookupType_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_lkup_type));
                    cmd = DRV_IOW(DsSrcPort_t,DsSrcPort_gAcl_2_aclUseGlobalPortType_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip,drv_lport,cmd,&acl_gport_type));
                }
                if(value & 0x8)
                {
                    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_3_aclLookupType_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_lkup_type));
                    cmd = DRV_IOW(DsSrcPort_t,DsSrcPort_gAcl_3_aclUseGlobalPortType_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip,drv_lport,cmd,&acl_gport_type));
                }
            }

            cmd = 0;
            break;
        case CTC_PORT_DIR_PROP_ACL_HASH_FIELD_SEL_ID:
            CTC_MAX_VALUE_CHECK(value, 0xF);
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_aclFlowHashFieldSel_f);
            break;

        case CTC_PORT_DIR_PROP_ACL_CLASSID:
        case CTC_PORT_DIR_PROP_ACL_CLASSID_0:
            SYS_ACL_PORT_CLASS_ID_CHECK(value);

            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_0_aclLabel_f);
            break;

        case CTC_PORT_DIR_PROP_ACL_CLASSID_1:
            SYS_ACL_PORT_CLASS_ID_CHECK(value);
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_1_aclLabel_f);
            break;

        case CTC_PORT_DIR_PROP_ACL_CLASSID_2:
            SYS_ACL_PORT_CLASS_ID_CHECK(value);
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_2_aclLabel_f);
            break;

        case CTC_PORT_DIR_PROP_ACL_CLASSID_3:
            SYS_ACL_PORT_CLASS_ID_CHECK(value);
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_3_aclLabel_f);
            break;

        case CTC_PORT_DIR_PROP_ACL_USE_CLASSID:
            if(value)
            {
                acl_gport_type = DRV_FLOWPORTTYPE_GPORT;
                cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_0_aclUseGlobalPortType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_gport_type));
                cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_1_aclUseGlobalPortType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_gport_type));
                cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_2_aclUseGlobalPortType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_gport_type));
                cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_3_aclUseGlobalPortType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_gport_type));
                cmd = 0;
            }
            else
            {
                acl_gport_type = DRV_FLOWPORTTYPE_BITMAP;
                cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_0_aclUseGlobalPortType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_gport_type));
                cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_1_aclUseGlobalPortType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_gport_type));
                cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_2_aclUseGlobalPortType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_gport_type));
                cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_3_aclUseGlobalPortType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_gport_type));
                cmd = 0;
            }
            break;
        case CTC_PORT_DIR_PROP_ACL_HASH_LKUP_TYPE:
            CTC_MAX_VALUE_CHECK(value, CTC_ACL_HASH_LKUP_TYPE_MAX -1);
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_aclFlowHashType_f);
            break;
        case CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0:
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_0_aclLookupType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_lkup_type));
            cmd = 0;
            break;

        case CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_1:
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_1_aclLookupType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_lkup_type));
            cmd = 0;
            break;

        case CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_2:
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_2_aclLookupType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_lkup_type));
            cmd = 0;
            break;

        case CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_3:

            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_3_aclLookupType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_lkup_type));
            cmd = 0;
            break;
        case CTC_PORT_DIR_PROP_ACL_USE_MAPPED_VLAN:
            value = value ? 1 : 0;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_0_aclUsePIVlan_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_1_aclUsePIVlan_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_2_aclUsePIVlan_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_gAcl_3_aclUsePIVlan_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));
            cmd = 0;
            break;
        case CTC_PORT_DIR_PROP_ACL_PORT_BITMAP_ID:
            CTC_MAX_VALUE_CHECK(value, SYS_USW_MAX_ACL_PORT_BITMAP_NUM -1);
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_aclPortNum_f);
            break;
        case CTC_PORT_DIR_PROP_SERVICE_ACL_EN:
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;


/********** ACL property end **********/
        case CTC_PORT_DIR_PROP_QOS_DOMAIN:
            CTC_MAX_VALUE_CHECK(value, MCHIP_CAP(SYS_CAP_QOS_CLASS_COS_DOMAIN_MAX));
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_cosPhbPtr_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_dscpPhbPtr_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));
            break;

        case CTC_PORT_DIR_PROP_QOS_COS_DOMAIN:
            CTC_MAX_VALUE_CHECK(value, MCHIP_CAP(SYS_CAP_QOS_CLASS_COS_DOMAIN_MAX));
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_cosPhbPtr_f);
            break;

        case CTC_PORT_DIR_PROP_QOS_DSCP_DOMAIN:
            CTC_MAX_VALUE_CHECK(value, MCHIP_CAP(SYS_CAP_QOS_CLASS_DSCP_DOMAIN_MAX));
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_dscpPhbPtr_f);
            break;

        case CTC_PORT_DIR_PROP_PORT_POLICER_VALID:
            value = value ? 1 : 0;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_portPolicerEn_f);
            break;

        case CTC_PORT_DIR_PROP_DOT1AE_EN:
            {
                sys_usw_register_dot1ae_bind_sc_t bind_sc;
                sal_memset(&bind_sc, 0, sizeof(bind_sc));
                bind_sc.type = 0;
                bind_sc.gport = gport;
                bind_sc.dir = 1; /* CTC_DOT1AE_SC_DIR_RX */
                cmd = DRV_IOR(DsPhyPort_t, DsPhyPort_dot1AeSaIndex_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &temp));
                bind_sc.sc_index = temp >> 2;
                if (value)/*enable*/
                {
                    if (REGISTER_API(lchip)->dot1ae_get_bind_sec_chan)
                    {
                        CTC_ERROR_RETURN(REGISTER_API(lchip)->dot1ae_get_bind_sec_chan(lchip, &bind_sc));
                        if(0 == bind_sc.chan_id)
                        {
                            return CTC_E_NOT_READY;
                        }
                    }
                    else
                    {
                        return CTC_E_NOT_SUPPORT;
                    }
                }
                else/*disable*/
                {
                    if (REGISTER_API(lchip)->dot1ae_unbind_sec_chan)
                    {
                        CTC_ERROR_RETURN(REGISTER_API(lchip)->dot1ae_unbind_sec_chan(lchip,&bind_sc));
                    }
                    else
                    {
                        return CTC_E_NOT_SUPPORT;
                    }
                }
                value = value ? 1 : 0;
                cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_dot1AeP2pMode0En_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));
                cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_dot1AeP2pMode1En_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));
                cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_dot1AeP2pMode2En_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));

                cmd = DRV_IOW(DsPhyPort_t, DsPhyPort_dot1AeEn_f);
            }
            break;
        case CTC_PORT_DIR_PROP_DOT1AE_CHAN_ID:
            {
                sys_usw_register_dot1ae_bind_sc_t bind_sc;
                CTC_MIN_VALUE_CHECK(value,1);
                sal_memset(&bind_sc, 0, sizeof(bind_sc));
                bind_sc.type = 0;
                bind_sc.gport = gport;
                bind_sc.dir = 1; /* CTC_DOT1AE_SC_DIR_RX */
                bind_sc.chan_id = value;
                if (REGISTER_API(lchip)->dot1ae_bind_sec_chan)
                {
                    CTC_ERROR_RETURN(REGISTER_API(lchip)->dot1ae_bind_sec_chan(lchip, &bind_sc));
                }
                value = bind_sc.sc_index << 2;
                cmd = DRV_IOW(DsPhyPort_t, DsPhyPort_dot1AeSaIndex_f);
            }
            break;

        case CTC_PORT_DIR_PROP_SD_ACTION :
            ret = _sys_usw_port_set_sd_action(lchip, CTC_INGRESS, drv_lport, value);
            cmd = 0;
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }

        if (cmd)
        {
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));
        }
    }

    value = egress_value;

    if ((CTC_EGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
    {
        index = drv_lport;
        switch (port_prop)
        {
        case CTC_PORT_DIR_PROP_STAG_TPID_INDEX:
            if (value > 3 && (0xff != value))
            {
                return CTC_E_INVALID_PARAM;
            }
            valid = (0xff == value) ? 0 : 1;
            value = (0xff == value) ? 0 : value;
            cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_svlanTpidValid_f) : DRV_IOW(DsDestPort_t, DsDestPort_svlanTpidValid_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &valid));
            cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_svlanTpidIndex_f):DRV_IOW(DsDestPort_t, DsDestPort_svlanTpidIndex_f);
            break;
            
/* SYSTEM MODIFIED for tap */
#ifdef TAPPRODUCT
        case CTC_PORT_DIR_PROP_STAG_TPID_INDEX_EN:
            value = (value) ? 1 : 0;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_svlanTpidValid_f);
            break;
#endif
        case CTC_PORT_DIR_PROP_VLAN_FILTER_EN:
            value = (value) ? SYS_PORT_VLAN_FILTER_MODE_ENABLE : SYS_PORT_VLAN_FILTER_MODE_DISABLE;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_egressFilteringMode_f);
            break;

        case CTC_PORT_DIR_PROP_RANDOM_LOG_EN:
            if ((drv_lport>=SYS_PHY_PORT_NUM_PER_SLICE) && !DRV_IS_DUET2(lchip))        /*TM random log only support lport 0-63*/
            {
                return CTC_E_NOT_SUPPORT;
            }
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_randomLogEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &random_en));
            value = (value) ? 1 : 0;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_randomLogEn_f);
            if ((random_en^value) && !DRV_IS_DUET2(lchip))      /*TM only change random en will call the function*/
            {
                CTC_ERROR_RETURN(_sys_usw_port_set_seed_id(lchip, drv_lport, CTC_EGRESS, value));
            }
            break;

        case CTC_PORT_DIR_PROP_RANDOM_LOG_PERCENT:
            ret = _sys_usw_port_set_random_log_percent(lchip, gport, CTC_EGRESS, value);
            break;

        case CTC_PORT_DIR_PROP_RANDOM_LOG_RATE:
            ret = _sys_usw_port_set_random_log_rate(lchip, gport, CTC_EGRESS, value);
            break;
/********** ACL property begin **********/
        case CTC_PORT_DIR_PROP_ACL_EN:
            CTC_MAX_VALUE_CHECK(value, CTC_ACL_EN_0);

            if (0 == value)     /* if acl dis, set l2 label = 0 */
            {
                cmd = DRV_IOW(DsDestPort_t, DsDestPort_gAcl_0_aclLookupType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));
            }
            else
            {
                //acl_lkup_type = 1;
                /* SYSTEM MODIFIED for tap */
#ifdef TAPPRODUCT
                acl_lkup_type = SYS_ACL_TCAM_LKUP_TYPE_L2_L3;
#else
                acl_lkup_type = SYS_ACL_TCAM_LKUP_TYPE_L2;
#endif     
                acl_gport_type = 0;
                if(value & 0x1)
                {
                    cmd = DRV_IOW(DsDestPort_t, DsDestPort_gAcl_0_aclLookupType_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_lkup_type));
                    cmd = DRV_IOW(DsDestPort_t,DsDestPort_gAcl_0_aclUseGlobalPortType_f);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip,drv_lport,cmd,&acl_gport_type));
                }
            }
            cmd = 0;
            break;

        case CTC_PORT_DIR_PROP_ACL_CLASSID:
        case CTC_PORT_DIR_PROP_ACL_CLASSID_0:
            SYS_ACL_PORT_CLASS_ID_CHECK(value);
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_gAcl_0_aclLabel_f);
            break;
        case CTC_PORT_DIR_PROP_ACL_USE_CLASSID:
            if(value)
            {
                acl_gport_type = DRV_FLOWPORTTYPE_GPORT;
                cmd = DRV_IOW(DsDestPort_t, DsDestPort_gAcl_0_aclUseGlobalPortType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_gport_type));
                cmd = 0;
            }
            else
            {
                acl_gport_type = DRV_FLOWPORTTYPE_BITMAP;
                cmd = DRV_IOW(DsDestPort_t, DsDestPort_gAcl_0_aclUseGlobalPortType_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_gport_type));
                cmd = 0;
            }
            break;
        case CTC_PORT_DIR_PROP_SERVICE_ACL_EN:
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;


        case CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0:
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_gAcl_0_aclLookupType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &acl_lkup_type));
            cmd = 0;
            break;
        case CTC_PORT_DIR_PROP_ACL_PORT_BITMAP_ID:
            CTC_MAX_VALUE_CHECK(value, SYS_USW_MAX_ACL_PORT_BITMAP_NUM -1);
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_aclPortNum_f);
            break;

/********** ACL property end **********/
        case CTC_PORT_DIR_PROP_QOS_DOMAIN:
            CTC_MAX_VALUE_CHECK(value, MCHIP_CAP(SYS_CAP_QOS_CLASS_COS_DOMAIN_MAX));

            cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_cosPhbPtr_f):DRV_IOW(DsDestPort_t, DsDestPort_cosPhbPtr_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));
            cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_dscpPhbPtr_f):DRV_IOW(DsDestPort_t, DsDestPort_dscpPhbPtr_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &value));
            break;

        case CTC_PORT_DIR_PROP_QOS_COS_DOMAIN:
            CTC_MAX_VALUE_CHECK(value, MCHIP_CAP(SYS_CAP_QOS_CLASS_COS_DOMAIN_MAX));
            cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_cosPhbPtr_f):DRV_IOW(DsDestPort_t, DsDestPort_cosPhbPtr_f);
            break;

        case CTC_PORT_DIR_PROP_QOS_DSCP_DOMAIN:
            CTC_MAX_VALUE_CHECK(value, MCHIP_CAP(SYS_CAP_QOS_CLASS_DSCP_DOMAIN_MAX));
            cmd = is_support_glb_port ? DRV_IOW(DsGlbDestPort_t, DsGlbDestPort_dscpPhbPtr_f):DRV_IOW(DsDestPort_t, DsDestPort_dscpPhbPtr_f);
            break;

        case CTC_PORT_DIR_PROP_PORT_POLICER_VALID:
            value = value ? 1 : 0;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_portPolicerEn_f);
            break;

        case CTC_PORT_DIR_PROP_DOT1AE_EN:
            {
                sys_usw_register_dot1ae_bind_sc_t bind_sc;
                sal_memset(&bind_sc, 0, sizeof(bind_sc));
                bind_sc.type = 0;
                bind_sc.gport = gport;
                bind_sc.dir = 0; /* CTC_DOT1AE_SC_DIR_TX */
                cmd = DRV_IOR(DsDestPort_t, DsDestPort_dot1AeSaIndex_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, drv_lport, cmd, &temp));
                bind_sc.sc_index = temp >> 2;
                if (value)/*enable*/
                {
                    if (REGISTER_API(lchip)->dot1ae_get_bind_sec_chan)
                    {
                        CTC_ERROR_RETURN(REGISTER_API(lchip)->dot1ae_get_bind_sec_chan(lchip, &bind_sc));
                        if (0 == bind_sc.chan_id)
                        {
                            return CTC_E_NOT_READY;
                        }
                    }
                    else
                    {
                        return CTC_E_NOT_SUPPORT;
                    }
                }
                else/*disable*/
                {
                    if (REGISTER_API(lchip)->dot1ae_unbind_sec_chan)
                    {
                        CTC_ERROR_RETURN(REGISTER_API(lchip)->dot1ae_unbind_sec_chan(lchip, &bind_sc));
                    }
                    else
                    {
                        return CTC_E_NOT_SUPPORT;
                    }
                }
            }
            sal_memset(&ds_ctl, 0, sizeof(QWriteDot1AeCtl_m));
            cmd = DRV_IOR(QWriteDot1AeCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_ctl));
            GetQWriteDot1AeCtl(A, dot1AeEn_f, &ds_ctl, dot1ae_en);
            if(value)   /* enable */
            {
                CTC_BIT_SET(dot1ae_en[drv_lport/32], drv_lport%32);
            }
            else
            {
                CTC_BIT_UNSET(dot1ae_en[drv_lport/32], drv_lport%32);
            }
            SetQWriteDot1AeCtl(A, dot1AeEn_f, &ds_ctl, dot1ae_en);
            cmd = DRV_IOW(QWriteDot1AeCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_ctl));

            value = value ? 1 : 0;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_dot1AeEn_f);
            break;
        case CTC_PORT_DIR_PROP_DOT1AE_CHAN_ID:
            {
                sys_usw_register_dot1ae_bind_sc_t bind_sc;
                CTC_MIN_VALUE_CHECK(value,1);
                sal_memset(&bind_sc,0,sizeof(bind_sc));
                bind_sc.type = 0;
                bind_sc.gport = gport;
                bind_sc.dir = 0; /* CTC_DOT1AE_SC_DIR_TX */
                bind_sc.chan_id = value;
                if (REGISTER_API(lchip)->dot1ae_bind_sec_chan)
                {
                    CTC_ERROR_RETURN(REGISTER_API(lchip)->dot1ae_bind_sec_chan(lchip,&bind_sc));
                }
                value = bind_sc.include_sci;
                cmd = DRV_IOW(DsDestPort_t, DsDestPort_dot1AeSciEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
                value = bind_sc.sc_index << 2;
                cmd = DRV_IOW(DsDestPort_t, DsDestPort_dot1AeSaIndex_f);
            }
            break;

        case CTC_PORT_DIR_PROP_SD_ACTION :
            ret = _sys_usw_port_set_sd_action(lchip, CTC_EGRESS, index, value);
            cmd = 0;
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }

        if (cmd)
        {
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value));
        }
    }

    CTC_ERROR_RETURN(ret);

    return CTC_E_NONE;
}

/**
@brief  Get port's properties with direction according to gport id
*/
int32
sys_usw_port_get_direction_property(uint8 lchip, uint32 gport, ctc_port_direction_property_t port_prop, ctc_direction_t dir, uint32* p_value)
{
    uint16 lport = 0;
    uint32 cmd = 0;
    int32  ret = CTC_E_NONE;
    uint32 acl_en = 0;
    uint32 acl_lkup_type = 0;
    uint32 acl_gport_type;
    uint32 index = 0;
    uint8  is_support_glb_port = _sys_usw_port_is_support_glb_port(lchip);
	uint8   gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get port property with direction, gport:0x%04X, property:%d, dir:%d\n" \
                     , gport, port_prop, dir);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_value);
    CTC_MAX_VALUE_CHECK(dir, CTC_BOTH_DIRECTION - 1);
    SYS_GLOBAL_CHIPID_CHECK(gchip);
	if (FALSE == sys_usw_chip_is_local(lchip, gchip))
	{
        if(dir != CTC_EGRESS)
        {
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Only support config global port's egress property\n");
            return CTC_E_INVALID_PARAM;
        }
		return _sys_usw_port_get_global_property(lchip, gport, port_prop, p_value);
	}
	lport = CTC_MAP_GPORT_TO_LPORT(gport);

    /* zero value */
    *p_value = 0;
    index = lport;

    /*do read,only get value by CTC_INGRESS or CTC_EGRESS,no CTC_BOTH_DIRECTION*/
    if (CTC_INGRESS == dir)
    {
        switch (port_prop)
        {
        case CTC_PORT_DIR_PROP_STAG_TPID_INDEX:
            cmd = DRV_IOR(DsPhyPort_t, DsPhyPort_svlanTpidIndex_f);
            break;

        case CTC_PORT_DIR_PROP_VLAN_FILTER_EN:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_ingressFilteringMode_f);
            break;

        case CTC_PORT_DIR_PROP_RANDOM_LOG_EN:
            cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_randomLogEn_f);
            break;

        case CTC_PORT_DIR_PROP_RANDOM_LOG_PERCENT:
            ret = _sys_usw_port_get_random_log_percent(lchip, gport, dir, p_value);
            break;

        case CTC_PORT_DIR_PROP_RANDOM_LOG_RATE:
            cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_randomThreshold_f);
            /* randomLogShift = 5*/
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, p_value));
            cmd = 0;
            break;
/********** ACL property begin **********/
        case CTC_PORT_DIR_PROP_ACL_EN:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_gAcl_0_aclLookupType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &acl_en));
            if (acl_en)
            {
                CTC_SET_FLAG(*p_value, CTC_ACL_EN_0);
            }

            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_gAcl_1_aclLookupType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &acl_en));
            if (acl_en)
            {
                CTC_SET_FLAG(*p_value, CTC_ACL_EN_1);
            }

            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_gAcl_2_aclLookupType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &acl_en));
            if (acl_en)
            {
                CTC_SET_FLAG(*p_value, CTC_ACL_EN_2);
            }

            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_gAcl_3_aclLookupType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &acl_en));
            if (acl_en)
            {
                CTC_SET_FLAG(*p_value, CTC_ACL_EN_3);
            }

            cmd = 0;
            break;

        case CTC_PORT_DIR_PROP_ACL_CLASSID:
        case CTC_PORT_DIR_PROP_ACL_CLASSID_0:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_gAcl_0_aclLabel_f);
            break;

        case CTC_PORT_DIR_PROP_ACL_CLASSID_1:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_gAcl_1_aclLabel_f);
            break;

        case CTC_PORT_DIR_PROP_ACL_CLASSID_2:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_gAcl_2_aclLabel_f);
            break;

        case CTC_PORT_DIR_PROP_ACL_CLASSID_3:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_gAcl_3_aclLabel_f);
            break;

        case CTC_PORT_DIR_PROP_ACL_USE_CLASSID:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_gAcl_0_aclUseGlobalPortType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &acl_gport_type));
            if(acl_gport_type == DRV_FLOWPORTTYPE_GPORT)
            {
                *p_value = 1;
            }
            else
            {
                *p_value = 0;
            }
            cmd = 0;
            break;
        case CTC_PORT_DIR_PROP_ACL_HASH_FIELD_SEL_ID:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_aclFlowHashFieldSel_f);
            break;

        case CTC_PORT_DIR_PROP_SERVICE_ACL_EN:
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;

        case CTC_PORT_DIR_PROP_ACL_HASH_LKUP_TYPE:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_aclFlowHashType_f);
            break;

        case CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_gAcl_0_aclLookupType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &acl_lkup_type));
            *p_value = sys_usw_unmap_acl_tcam_lkup_type(lchip, acl_lkup_type);
            cmd = 0;
            break;

        case CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_1:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_gAcl_1_aclLookupType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &acl_lkup_type));
            *p_value = sys_usw_unmap_acl_tcam_lkup_type(lchip, acl_lkup_type);
            cmd = 0;
            break;

        case CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_2:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_gAcl_2_aclLookupType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &acl_lkup_type));
            *p_value = sys_usw_unmap_acl_tcam_lkup_type(lchip, acl_lkup_type);
            cmd = 0;
            break;

        case CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_3:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_gAcl_3_aclLookupType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &acl_lkup_type));
            *p_value = sys_usw_unmap_acl_tcam_lkup_type(lchip, acl_lkup_type);
            cmd = 0;
            break;

        case CTC_PORT_DIR_PROP_ACL_USE_MAPPED_VLAN:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_gAcl_0_aclUsePIVlan_f);
            break;
        case CTC_PORT_DIR_PROP_ACL_PORT_BITMAP_ID:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_aclPortNum_f);
            break;
/********** ACL property end **********/
        case CTC_PORT_DIR_PROP_QOS_DOMAIN:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_cosPhbPtr_f);
            break;

        case CTC_PORT_DIR_PROP_QOS_COS_DOMAIN:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_cosPhbPtr_f);
            break;

        case CTC_PORT_DIR_PROP_QOS_DSCP_DOMAIN:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_dscpPhbPtr_f);
            break;

        case CTC_PORT_DIR_PROP_PORT_POLICER_VALID:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_portPolicerEn_f);
            break;

        case CTC_PORT_DIR_PROP_DOT1AE_EN:
            cmd = DRV_IOR(DsPhyPort_t, DsPhyPort_dot1AeEn_f);
            break;
        case CTC_PORT_DIR_PROP_DOT1AE_CHAN_ID:
            {
                sys_usw_register_dot1ae_bind_sc_t bind_sc;
                bind_sc.chan_id = 0;
                bind_sc.type = 0;
                bind_sc.gport = gport;
                bind_sc.dir = 1; /* CTC_DOT1AE_SC_DIR_RX */

                cmd = DRV_IOR(DsPhyPort_t, DsPhyPort_dot1AeSaIndex_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, p_value));
                bind_sc.sc_index= *p_value >> 2;
                if (REGISTER_API(lchip)->dot1ae_get_bind_sec_chan)
                {
                    CTC_ERROR_RETURN(REGISTER_API(lchip)->dot1ae_get_bind_sec_chan(lchip,&bind_sc));
                }
                *p_value = bind_sc.chan_id;
                cmd = 0;
            }
            break;

        case CTC_PORT_DIR_PROP_SD_ACTION :
            ret = _sys_usw_port_get_sd_action(lchip, CTC_INGRESS, index, p_value);
            break;

        default:
            return CTC_E_INVALID_PARAM;

        }
    }
    else if (CTC_EGRESS == dir)
    {
        switch (port_prop)
        {
        case CTC_PORT_DIR_PROP_STAG_TPID_INDEX:
            cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_svlanTpidIndex_f):DRV_IOR(DsDestPort_t, DsDestPort_svlanTpidIndex_f);
            break;

        case CTC_PORT_DIR_PROP_VLAN_FILTER_EN:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_egressFilteringMode_f);
            break;

        case CTC_PORT_DIR_PROP_RANDOM_LOG_EN:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_randomLogEn_f);
            break;

        case CTC_PORT_DIR_PROP_RANDOM_LOG_PERCENT:
            ret = _sys_usw_port_get_random_log_percent(lchip, gport, dir, p_value);
            break;

        case CTC_PORT_DIR_PROP_RANDOM_LOG_RATE:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_randomThreshold_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, p_value));
            /* randomLogShift = 5 */
            *p_value = *p_value;
            cmd = 0;
            break;
/********** ACL property begin **********/
        case CTC_PORT_DIR_PROP_ACL_EN:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_gAcl_0_aclLookupType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &acl_en));
            if (acl_en)
            {
                CTC_SET_FLAG(*p_value, CTC_ACL_EN_0);
            }

            cmd = 0;
            break;

        case CTC_PORT_DIR_PROP_ACL_CLASSID:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_gAcl_0_aclLabel_f);
            break;

        case CTC_PORT_DIR_PROP_ACL_USE_CLASSID:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_gAcl_0_aclUseGlobalPortType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &acl_gport_type));
            if(acl_gport_type == DRV_FLOWPORTTYPE_GPORT)
            {
                *p_value = 1;
            }
            else
            {
                *p_value = 0;
            }
            cmd = 0;
            break;

        case CTC_PORT_DIR_PROP_SERVICE_ACL_EN:
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

            break;

        case CTC_PORT_DIR_PROP_ACL_TCAM_LKUP_TYPE_0:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_gAcl_0_aclLookupType_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &acl_lkup_type));
            *p_value = sys_usw_unmap_acl_tcam_lkup_type(lchip, acl_lkup_type);
            cmd = 0;
            break;
        case CTC_PORT_DIR_PROP_ACL_PORT_BITMAP_ID:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_aclPortNum_f);
            break;

/********** ACL property end **********/
        case CTC_PORT_DIR_PROP_QOS_DOMAIN:
            {
                uint32 value1=0;
                uint32 value2=0;

                cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_cosPhbPtr_f):DRV_IOR(DsDestPort_t, DsDestPort_cosPhbPtr_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value1));

                cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_dscpPhbPtr_f):DRV_IOR(DsDestPort_t, DsDestPort_dscpPhbPtr_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &value2));
                if(value1 == value2)
                {
                    *p_value = value1;
                }
                else
                {
                    *p_value =0;
                }
                cmd = 0;
                break;
            }

        case CTC_PORT_DIR_PROP_QOS_COS_DOMAIN:
            cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_cosPhbPtr_f):DRV_IOR(DsDestPort_t, DsDestPort_cosPhbPtr_f);
            break;

        case CTC_PORT_DIR_PROP_QOS_DSCP_DOMAIN:
            cmd = is_support_glb_port ? DRV_IOR(DsGlbDestPort_t, DsGlbDestPort_dscpPhbPtr_f):DRV_IOR(DsDestPort_t, DsDestPort_dscpPhbPtr_f);
            break;

        case CTC_PORT_DIR_PROP_PORT_POLICER_VALID:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_portPolicerEn_f);
            break;

        case CTC_PORT_DIR_PROP_DOT1AE_EN:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_dot1AeEn_f);
            break;
        case CTC_PORT_DIR_PROP_DOT1AE_CHAN_ID:
            {
                sys_usw_register_dot1ae_bind_sc_t bind_sc;
                bind_sc.chan_id = 0;
                bind_sc.type = 0;
                bind_sc.gport = gport;
                bind_sc.dir = 0; /* CTC_DOT1AE_SC_DIR_TX */
                cmd = DRV_IOR(DsDestPort_t, DsDestPort_dot1AeSaIndex_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, p_value));
                bind_sc.sc_index= *p_value >> 2;
                if (REGISTER_API(lchip)->dot1ae_get_bind_sec_chan)
                {
                    CTC_ERROR_RETURN(REGISTER_API(lchip)->dot1ae_get_bind_sec_chan(lchip,&bind_sc));
                }
                *p_value = bind_sc.chan_id;
                cmd = 0;
            }
            break;

        case CTC_PORT_DIR_PROP_SD_ACTION :
            ret = _sys_usw_port_get_sd_action(lchip, CTC_EGRESS, index, p_value);
            cmd = 0;
        break;

        default:
            return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(ret);

    if (cmd)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, p_value));
    }

    return CTC_E_NONE;
}

/**
@brief  Config port's internal properties
*/
int32
sys_usw_port_set_internal_property(uint8 lchip, uint32 gport, sys_port_internal_property_t port_prop, uint32 value)
{
    uint8  idx = 0;
    uint8  num = 1;

    uint16 lport = 0;
    uint32 cmd[2] = {0};

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set port internal property, gport:0x%04X, property:%d, value:%d\n", \
                     gport, port_prop, value);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    switch (port_prop)
    {
    case SYS_PORT_PROP_DEFAULT_PCP:
        cmd[0] = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_defaultPcp_f);
        break;

    case SYS_PORT_PROP_DEFAULT_DEI:
        cmd[0] = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_defaultDei_f);
        break;

    case SYS_PORT_PROP_OAM_TUNNEL_EN:
        value = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcPort_t, DsSrcPort_oamTunnelEn_f);
        break;

    case SYS_PORT_PROP_OAM_VALID:
        value = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcPort_t, DsSrcPort_etherOamValid_f);
        value = (value) ? 1 : 0;
        cmd[1] = DRV_IOW(DsDestPort_t, DsDestPort_etherOamValid_f);
        num++;
        break;

    case SYS_PORT_PROP_DISCARD_NONE_8023OAM_EN:
        value = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsDestPort_t, DsDestPort_discardNon8023Oam_f);
        break;

    case SYS_PORT_PROP_REPLACE_TAG_EN:
        value = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcPort_t, DsSrcPort_defaultReplaceTagEn_f);
        break;

    case SYS_PORT_PROP_EXCEPTION_EN:
        cmd[0] = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_exception2En_f);
        break;

    case SYS_PORT_PROP_EXCEPTION_DISCARD:
        cmd[0] = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_exception2Discard_f);
        break;

    case SYS_PORT_PROP_SECURITY_EXCP_EN:
        value = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcPort_t, DsSrcPort_portSecurityExceptionEn_f);
        /*SYSTEM MODIFIED by chenyq for bug 28025 on GG, bug 54152 for usw, srcMismatchExceptionEn should be enable */
        cmd[1] = DRV_IOW(DsSrcPort_t, DsSrcPort_srcMismatchExceptionEn_f);
        num++;
        break;

    case SYS_PORT_PROP_SECURITY_EN:
        /* portSecurityEn will be always enabled for srcMismatch discard */
        /*SYSTEM MODIFIED by xgu for bug 28025, DsSrcPort_portSecurityEn_f will be always enabled for srcMismatch discard, 2
014-3-31*/
        return CTC_E_NONE;
        value = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcPort_t, DsSrcPort_portSecurityEn_f);
        break;

    case SYS_PORT_PROP_MAC_SECURITY_DISCARD:
        value = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcPort_t, DsSrcPort_macSecurityDiscard_f);
        break;
    case SYS_PORT_PROP_REPLACE_DSCP_EN:
        cmd[0] = 0;
        break;
    case SYS_PORT_PROP_STMCTL_EN:
        value = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcPort_t, DsSrcPort_portStormCtlEn_f);
        break;

    case SYS_PORT_PROP_STMCTL_OFFSET:
        cmd[0] = DRV_IOW(DsSrcPort_t, DsSrcPort_portStormCtlPtr_f);
        break;

    /*new properties for GB*/
    case SYS_PORT_PROP_LINK_OAM_EN:
        value = (value) ? 1 : 0;
        cmd[0] = DRV_IOW(DsSrcPort_t, DsSrcPort_linkOamEn_f);
        break;

    case SYS_PORT_PROP_LINK_OAM_MEP_INDEX:
        cmd[0] = DRV_IOW(DsSrcPort_t, DsSrcPort_linkOamMepIndex_f);
        break;

    case SYS_PORT_PROP_DOT1AE_TX_SCI_EN:
        cmd[0] = DRV_IOW(DsDestPort_t, DsDestPort_dot1AeSciEn_f);
        break;


    case SYS_PORT_PROP_MAC_AUTH_EN:
        cmd[0] = DRV_IOW(DsSrcPort_t, DsSrcPort_portAuthEn_f);
        cmd[1] = DRV_IOW(DsSrcPort_t, DsSrcPort_portAuthExceptionEn_f);
        num++;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    for (idx = 0; idx < num; idx++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd[idx], &value));
    }

    return CTC_E_NONE;
}

/**
@brief  Get port's internal properties according to gport id
*/
int32
sys_usw_port_get_internal_property(uint8 lchip, uint32 gport, sys_port_internal_property_t port_prop, uint32* p_value)
{
    uint32 cmd = 0;

    uint16 lport = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get port internal property, gport:0x%04X, property:%d!\n", gport, port_prop);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_value);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    /* zero value */
    *p_value = 0;

    switch (port_prop)
    {
    case SYS_PORT_PROP_DEFAULT_DEI:
        cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_defaultDei_f);
        break;

    case SYS_PORT_PROP_OAM_TUNNEL_EN:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_oamTunnelEn_f);
        break;

    case SYS_PORT_PROP_OAM_VALID:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_etherOamValid_f);
        break;

    case SYS_PORT_PROP_REPLACE_TAG_EN:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_defaultReplaceTagEn_f);
        break;

    case SYS_PORT_PROP_EXCEPTION_EN:
        cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_exception2En_f);
        break;

    case SYS_PORT_PROP_EXCEPTION_DISCARD:
        cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_exception2Discard_f);
        break;

    case SYS_PORT_PROP_SECURITY_EXCP_EN:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_portSecurityExceptionEn_f);
        break;

    case SYS_PORT_PROP_SECURITY_EN:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_portSecurityEn_f);
        break;

    case SYS_PORT_PROP_MAC_SECURITY_DISCARD:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_macSecurityDiscard_f);
        break;
    case SYS_PORT_PROP_REPLACE_DSCP_EN:
        cmd = 0;
        break;
    case SYS_PORT_PROP_STMCTL_EN:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_portStormCtlEn_f);
        break;
    case SYS_PORT_PROP_STMCTL_OFFSET:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_portStormCtlPtr_f);
        break;

    /*new properties for GB*/
    case SYS_PORT_PROP_LINK_OAM_EN:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_linkOamEn_f);
        break;

    case SYS_PORT_PROP_LINK_OAM_MEP_INDEX:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_linkOamMepIndex_f);
        break;

    case SYS_PORT_PROP_DOT1AE_TX_SCI_EN:
        cmd = DRV_IOR(DsDestPort_t, DsDestPort_dot1AeSciEn_f);
        break;

    case SYS_PORT_PROP_MAC_AUTH_EN:
        cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_portAuthEn_f);
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, p_value));

    return CTC_E_NONE;
}

int32
sys_usw_port_set_ingress_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32 value)
{
    uint32 cmd = 0;
    uint32 lport = 0;
    if (CTC_PORT_PROP_BRIDGE_EN != port_prop)
    {
        CTC_ERROR_RETURN(CTC_E_NOT_SUPPORT);
    }
    lport = CTC_MAP_GPORT_TO_LPORT(gport);
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_bridgeEn_f);
    value = value ? 1 : 0;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
    return CTC_E_NONE;
}

/**
@brief  Set port's internal properties with direction according to gport id
*/
int32
sys_usw_port_set_internal_direction_property(uint8 lchip, uint32 gport, sys_port_internal_direction_property_t port_prop, ctc_direction_t dir, uint32 value)
{
    uint16 lport = 0;
    uint32 cmd = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set port internal property with direction, gport:0x%04X, property:%d, dir:%d,\
                                              value:%d\n", gport, port_prop, dir, value);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_MAX_VALUE_CHECK(dir, CTC_BOTH_DIRECTION);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    /*do write*/
    if ((CTC_INGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
    {
        switch (port_prop)
        {
        case SYS_PORT_DIR_PROP_L2_SPAN_EN:
            value = value ? 1 : 0;
            cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_l2SpanEn_f);
            break;

        case SYS_PORT_DIR_PROP_L2_SPAN_ID:
            CTC_MAX_VALUE_CHECK(value, 3);
            cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_l2SpanId_f);
            break;

        case SYS_PORT_DIR_PROP_ETHER_OAM_EDGE_PORT:
            value = value ? 1 : 0;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_etherOamEdgePort_f);
            break;

        case SYS_PORT_DIR_PROP_LINK_LM_TYPE:
            CTC_MAX_VALUE_CHECK(value, 3);
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_linkLmType_f);
            break;

        case SYS_PORT_DIR_PROP_LINK_LM_COS:
            CTC_MAX_VALUE_CHECK(value, 7);
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_linkLmCos_f);
            break;

        case SYS_PORT_DIR_PROP_LINK_LM_COS_TYPE:
            CTC_MAX_VALUE_CHECK(value, 3);
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_linkLmCosType_f);
            break;

        case SYS_PORT_DIR_PROP_LINK_LM_INDEX_BASE:
            CTC_MAX_VALUE_CHECK(value, 2047);
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_linkLmIndexBase_f);
            break;

        case SYS_PORT_DIR_PROP_ETHER_LM_VALID:
            value = value ? 1 : 0;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_etherLmValid_f);
            break;

        case SYS_PORT_DIR_PROP_MPLS_SECTION_LM_EN:
            value = value ? 1 : 0;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_mplsSectionLmEn_f);
            break;

        case SYS_PORT_DIR_PROP_DOT1AE_SA_IDX_BASE:
            CTC_MAX_VALUE_CHECK(value, 32 - 1);
            value = value << 2;
            cmd = DRV_IOW(DsPhyPort_t, DsPhyPort_dot1AeSaIndex_f);
            break;

        default:
            return CTC_E_INVALID_PARAM;

        }

        if (cmd)
        {
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
        }
    }

    if ((CTC_EGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
    {
        switch (port_prop)
        {
        case SYS_PORT_DIR_PROP_L2_SPAN_EN:
            value = value ? 1 : 0;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_l2SpanEn_f);
            break;

        case SYS_PORT_DIR_PROP_L2_SPAN_ID:
            CTC_MAX_VALUE_CHECK(value, 3);
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_l2SpanId_f);
            break;

        case SYS_PORT_DIR_PROP_ETHER_OAM_EDGE_PORT:
            value = value ? 1 : 0;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_etherOamEdgePort_f);
            break;

        case SYS_PORT_DIR_PROP_LINK_LM_TYPE:
            CTC_MAX_VALUE_CHECK(value, 3);
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_linkLmType_f);
            break;

        case SYS_PORT_DIR_PROP_LINK_LM_COS:
            CTC_MAX_VALUE_CHECK(value, 7);
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_linkLmCos_f);
            break;

        case SYS_PORT_DIR_PROP_LINK_LM_COS_TYPE:
            CTC_MAX_VALUE_CHECK(value, 3);
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_linkLmCosType_f);
            break;

        case SYS_PORT_DIR_PROP_LINK_LM_INDEX_BASE:
            CTC_MAX_VALUE_CHECK(value, 2047);
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_linkLmIndexBase_f);
            break;

        case SYS_PORT_DIR_PROP_ETHER_LM_VALID:
            value = value ? 1 : 0;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_etherLmValid_f);
            break;

        case SYS_PORT_DIR_PROP_MPLS_SECTION_LM_EN:
            value = value ? 1 : 0;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_mplsSectionLmEn_f);
            break;

        case SYS_PORT_DIR_PROP_DOT1AE_SA_IDX_BASE:
            CTC_MAX_VALUE_CHECK(value, 32 - 1);
            value = value << 2;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_dot1AeSaIndex_f);
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }

        if (cmd)
        {
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
        }
    }

    return CTC_E_NONE;
}

/**
@brief  Get port's internal properties with direction according to gport id
*/
int32
sys_usw_port_get_internal_direction_property(uint8 lchip, uint32 gport, sys_port_internal_direction_property_t port_prop, ctc_direction_t dir, uint32* p_value)
{
    uint16 lport = 0;
    uint32 cmd = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get port internal property with direction, gport:0x%04X, property:%d, dir:%d" \
                     , gport, port_prop, dir);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_value);
    CTC_MAX_VALUE_CHECK(dir, CTC_BOTH_DIRECTION - 1);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    /* zero value */
    *p_value = 0;

    /*do read,only get value by CTC_INGRESS or CTC_EGRESS,no CTC_BOTH_DIRECTION*/
    if (CTC_INGRESS == dir)
    {
        switch (port_prop)
        {
        case SYS_PORT_DIR_PROP_L2_SPAN_EN:
            cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_l2SpanEn_f);
            break;

        case SYS_PORT_DIR_PROP_L2_SPAN_ID:
            cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_l2SpanId_f);
            break;

        case SYS_PORT_DIR_PROP_ETHER_OAM_EDGE_PORT:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_etherOamEdgePort_f);
            break;

        case SYS_PORT_DIR_PROP_LINK_LM_TYPE:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_linkLmType_f);
            break;

        case SYS_PORT_DIR_PROP_LINK_LM_COS:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_linkLmCos_f);
            break;

        case SYS_PORT_DIR_PROP_LINK_LM_COS_TYPE:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_linkLmCosType_f);
            break;

        case SYS_PORT_DIR_PROP_LINK_LM_INDEX_BASE:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_linkLmIndexBase_f);
            break;

        case SYS_PORT_DIR_PROP_ETHER_LM_VALID:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_etherLmValid_f);
            break;

        case SYS_PORT_DIR_PROP_MPLS_SECTION_LM_EN:
            cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_mplsSectionLmEn_f);
            break;

        default:
            return CTC_E_INVALID_PARAM;

        }
    }
    else if (CTC_EGRESS == dir)
    {
        switch (port_prop)
        {
        case SYS_PORT_DIR_PROP_L2_SPAN_EN:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_l2SpanEn_f);
            break;

        case SYS_PORT_DIR_PROP_L2_SPAN_ID:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_l2SpanId_f);
            break;

        case SYS_PORT_DIR_PROP_ETHER_OAM_EDGE_PORT:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_etherOamEdgePort_f);
            break;

        case SYS_PORT_DIR_PROP_LINK_LM_TYPE:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_linkLmType_f);
            break;

        case SYS_PORT_DIR_PROP_LINK_LM_COS:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_linkLmCos_f);
            break;

        case SYS_PORT_DIR_PROP_LINK_LM_COS_TYPE:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_linkLmCosType_f);
            break;

        case SYS_PORT_DIR_PROP_LINK_LM_INDEX_BASE:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_linkLmIndexBase_f);
            break;

        case SYS_PORT_DIR_PROP_ETHER_LM_VALID:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_etherLmValid_f);
            break;

        case SYS_PORT_DIR_PROP_MPLS_SECTION_LM_EN:
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_mplsSectionLmEn_f);
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, p_value));

    return CTC_E_NONE;
}

/**
@brief  Set ipg size
*/
int32
sys_usw_set_ipg_size(uint8 lchip, ctc_ipg_size_t index, uint8 size)
{
    uint32 cmd_i = 0;
    uint32 cmd_e = 0;
    uint32 field_val = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ipg size index:%d ipg size:%d\n", index, size);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();

    field_val = size;

    switch (index)
    {
    case CTC_IPG_SIZE_0:
        cmd_i = DRV_IOW(IpeIpgCtl_t, IpeIpgCtl_array_0_ipg_f);
        cmd_e = DRV_IOW(EpeIpgCtl_t, EpeIpgCtl_array_0_ipg_f);
        break;

    case CTC_IPG_SIZE_1:
        cmd_i = DRV_IOW(IpeIpgCtl_t, IpeIpgCtl_array_1_ipg_f);
        cmd_e = DRV_IOW(EpeIpgCtl_t, EpeIpgCtl_array_1_ipg_f);
        break;

    case CTC_IPG_SIZE_2:
        cmd_i = DRV_IOW(IpeIpgCtl_t, IpeIpgCtl_array_2_ipg_f);
        cmd_e = DRV_IOW(EpeIpgCtl_t, EpeIpgCtl_array_2_ipg_f);
        break;

    case CTC_IPG_SIZE_3:
        cmd_i = DRV_IOW(IpeIpgCtl_t, IpeIpgCtl_array_3_ipg_f);
        cmd_e = DRV_IOW(EpeIpgCtl_t, EpeIpgCtl_array_3_ipg_f);
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd_i, &field_val));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd_e, &field_val));

    return CTC_E_NONE;
}

/**
@brief  Get ipg size
*/
int32
sys_usw_get_ipg_size(uint8 lchip, ctc_ipg_size_t index, uint8* p_size)
{
    uint32 cmd = 0;
    uint32 field_val = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ipg size index:%d\n", index);

    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_size);

    switch (index)
    {
    case CTC_IPG_SIZE_0:
        cmd = DRV_IOR(IpeIpgCtl_t, IpeIpgCtl_array_0_ipg_f);
        break;

    case CTC_IPG_SIZE_1:
        cmd = DRV_IOR(IpeIpgCtl_t, IpeIpgCtl_array_1_ipg_f);
        break;

    case CTC_IPG_SIZE_2:
        cmd = DRV_IOR(IpeIpgCtl_t, IpeIpgCtl_array_2_ipg_f);
        break;

    case CTC_IPG_SIZE_3:
        cmd = DRV_IOR(IpeIpgCtl_t, IpeIpgCtl_array_3_ipg_f);
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    *p_size = field_val;

    return CTC_E_NONE;
}

/**
@brief  Set Port MAC PostFix
*/
int32
sys_usw_port_set_port_mac_postfix(uint8 lchip, uint32 gport, ctc_port_mac_postfix_t* p_port_mac)
{

    uint16 lport     = 0;
    uint32 cmd       = 0;
    uint32 field_val = 0;
    uint32 mac_id    = 0;
    uint32 tbl_id    = 0;
    uint8  step      = 0;
    Sgmac0TxPauseCfg0_m tx_pause_cfg;
    QuadSgmacCfg0_m mac_cfg;
    DsPhyPort_m ds_phy_port;
    DsPortProperty_m ds_port_property;
    DsEgressPortMac_m ds_egress_port_mac;
    hw_mac_addr_t mac;
    hw_mac_addr_t pause_mac;
    uint16 profile_idx = 0;
    uint32 port_type = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_port_mac);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    if (CTC_PORT_MAC_PREFIX_48BIT != p_port_mac->prefix_type)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_PORT_TYPE, (void *)&port_type));
    if (SYS_DMPS_NETWORK_PORT != port_type)
    {
        return CTC_E_INVALID_CONFIG;
    }
    CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_MAC_STATS_ID, (void *)&mac_id));

    SYS_USW_SET_HW_MAC(mac, p_port_mac->port_mac);
    SYS_USW_SET_HW_MAC(pause_mac, p_port_mac->port_mac);

    sal_memset(&ds_phy_port, 0, sizeof(DsPhyPort_m));
    cmd = DRV_IOR(DsPhyPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_phy_port));
    SetDsPhyPort(A, portMac_f, &ds_phy_port,  mac);
    cmd = DRV_IOW(DsPhyPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_phy_port));

    sal_memset(&ds_port_property, 0, sizeof(ds_port_property));
    cmd = DRV_IOR(DsPortProperty_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_port_property));
    SetDsPortProperty(A, macSaByte_f, &ds_port_property,  mac);
    cmd = DRV_IOW(DsPortProperty_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_port_property));

    sal_memset(&tx_pause_cfg, 0, sizeof(Sgmac0TxPauseCfg0_m));
    sal_memset(&mac_cfg, 0, sizeof(QuadSgmacCfg0_m));

    step = Sgmac1TxPauseCfg0_t - Sgmac0TxPauseCfg0_t;
    tbl_id = Sgmac0TxPauseCfg0_t + (mac_id / 4) + ((mac_id % 4)*step);
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tx_pause_cfg));
    field_val = pause_mac[0]&0xFF;
    SetSgmac0TxPauseCfg0(V, cfgSgmac0TxPauseMacSaLo_f, &tx_pause_cfg, field_val);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tx_pause_cfg));

    pause_mac[0] = (pause_mac[1]&0xFF) << 24 | pause_mac[0] >> 8;
    pause_mac[1] = pause_mac[1] >> 8;
    tbl_id = QuadSgmacCfg0_t + (mac_id / 4);
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_cfg));
    SetQuadSgmacCfg0(A, cfgSgmacTxPauseMacSaHi_f, &mac_cfg, pause_mac);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mac_cfg));

    sal_memset(&ds_egress_port_mac, 0, sizeof(ds_egress_port_mac));
    cmd = DRV_IOW(DsEgressPortMac_t, DRV_ENTRY_FLAG);
    SetDsEgressPortMac(A, portMac_f, &ds_egress_port_mac,  mac);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_egress_port_mac));

    cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_globalSrcPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));

    /*Notice: DsPortProperty index now is profile id, not port id, temply alloc index0~63 as phy port use;
         64~127 as linkagg port use, should be coherence with oam code*/
    if (SYS_DRV_IS_LINKAGG_PORT(field_val))
    {
        profile_idx = 64 + (field_val&0x3F);
        sal_memset(&ds_port_property, 0, sizeof(ds_port_property));
        cmd = DRV_IOR(DsPortProperty_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_idx, cmd, &ds_port_property));
        SetDsPortProperty(A, macSaByte_f, &ds_port_property,  mac);
        cmd = DRV_IOW(DsPortProperty_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile_idx, cmd, &ds_port_property));
    }

    return CTC_E_NONE;
}

/**
@brief  Get Port MAC
*/
int32
sys_usw_port_get_port_mac(uint8 lchip, uint32 gport, mac_addr_t* p_port_mac)
{
    uint16 lport = 0;
    uint32 cmd  = 0;
    DsPhyPort_m ds_phy_port;
    hw_mac_addr_t hw_mac;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_port_mac);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    sal_memset(&ds_phy_port, 0, sizeof(DsPhyPort_m));
    cmd = DRV_IOR(DsPhyPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_phy_port));

    sal_memset(&hw_mac, 0, sizeof(hw_mac_addr_t));
    GetDsPhyPort(A, portMac_f, &ds_phy_port, hw_mac);

    SYS_USW_SET_USER_MAC(*p_port_mac, hw_mac);

    return CTC_E_NONE;
}

int32
sys_usw_port_set_reflective_bridge_en(uint8 lchip, uint32 gport, bool enable)
{
    uint16 lport = 0;
    uint32 cmd = 0;
    uint32 field_val = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    field_val = (TRUE == enable) ? 0 : 1;
    cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_portCheckEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));

    field_val = (TRUE == enable) ? 1 : 0;
    cmd = DRV_IOW(DsDestPort_t, DsDestPort_reflectiveBridgeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));

    return CTC_E_NONE;
}

int32
sys_usw_port_get_reflective_bridge_en(uint8 lchip, uint32 gport, bool* p_enable)
{
    uint16 lport = 0;
    uint32 cmd = 0;
    uint32 field_val1 = 0;
    uint32 field_val2 = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    /*Sanity check*/
    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_enable);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    *p_enable = FALSE;

    cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_portCheckEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val1));

    cmd = DRV_IOR(DsDestPort_t, DsDestPort_reflectiveBridgeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val2));

    *p_enable = ((!field_val1) && field_val2) ? TRUE : FALSE;

    return CTC_E_NONE;
}
STATIC void
_sys_usw_port_scl_type(uint8 lchip, uint8 dir, sys_scl_key_mapping_type_t mapping_type, sys_scl_type_para_t* p_scl_type_para)
{
    uint8 i = 0;

    sys_scl_type_map_t scl_hash_type_map[][CTC_PORT_IGS_SCL_HASH_TYPE_MAX] =
    {
        {
             {0, 0, 0, USERIDPORTHASHTYPE_DISABLE,        CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE         },
             {1, 0, 0, USERIDPORTHASHTYPE_DOUBLEVLANPORT, CTC_PORT_IGS_SCL_HASH_TYPE_PORT_2VLAN      },
             {1, 0, 0, USERIDPORTHASHTYPE_SVLANPORT,      CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN      },
             {1, 0, 0, USERIDPORTHASHTYPE_CVLANPORT,      CTC_PORT_IGS_SCL_HASH_TYPE_PORT_CVLAN      },
             {1, 0, 0, USERIDPORTHASHTYPE_SVLANCOSPORT,   CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN_COS  },
             {1, 0, 0, USERIDPORTHASHTYPE_CVLANCOSPORT,   CTC_PORT_IGS_SCL_HASH_TYPE_PORT_CVLAN_COS  },
             {1, 0, 1, USERIDPORTHASHTYPE_MAC,            CTC_PORT_IGS_SCL_HASH_TYPE_MAC_SA          },
             {1, 0, 0, USERIDPORTHASHTYPE_MACPORT,        CTC_PORT_IGS_SCL_HASH_TYPE_PORT_MAC_SA     },
             {1, 1, 1, USERIDPORTHASHTYPE_MAC,            CTC_PORT_IGS_SCL_HASH_TYPE_MAC_DA          },
             {1, 1, 0, USERIDPORTHASHTYPE_MACPORT,        CTC_PORT_IGS_SCL_HASH_TYPE_PORT_MAC_DA     },
             {1, 0, 1, USERIDPORTHASHTYPE_IPSA,           CTC_PORT_IGS_SCL_HASH_TYPE_IP_SA           },
             {1, 0, 0, USERIDPORTHASHTYPE_IPSAPORT,       CTC_PORT_IGS_SCL_HASH_TYPE_PORT_IP_SA      },
             {1, 0, 0, USERIDPORTHASHTYPE_PORT,           CTC_PORT_IGS_SCL_HASH_TYPE_PORT            },
             {1, 0, 0, USERIDPORTHASHTYPE_SCLFLOW,        CTC_PORT_IGS_SCL_HASH_TYPE_L2              },
             {1, 0, 0, USERIDPORTHASHTYPE_TUNNEL,         CTC_PORT_IGS_SCL_HASH_TYPE_TUNNEL          },
             {1, 0, 0, USERIDPORTHASHTYPE_TUNNEL,         CTC_PORT_IGS_SCL_HASH_TYPE_TUNNEL_RPF      },
             {1, 0, 0, USERIDPORTHASHTYPE_TUNNEL,         CTC_PORT_IGS_SCL_HASH_TYPE_IPV4_TUNNEL_AUTO},
             {1, 0, 0, USERIDPORTHASHTYPE_TUNNEL,         CTC_PORT_IGS_SCL_HASH_TYPE_NVGRE           },
             {1, 0, 0, USERIDPORTHASHTYPE_TUNNEL,         CTC_PORT_IGS_SCL_HASH_TYPE_VXLAN           },
             {1, 0, 0, USERIDPORTHASHTYPE_TRILL,          CTC_PORT_IGS_SCL_HASH_TYPE_TRILL           },
             {1, 0, 0, USERIDPORTHASHTYPE_SVLANDSCPPORT,  CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN_DSCP },
             {1, 0, 0, USERIDPORTHASHTYPE_SVLAN,          CTC_PORT_IGS_SCL_HASH_TYPE_SVLAN           },
             {1, 0, 0, USERIDPORTHASHTYPE_SVLANMACSA,     CTC_PORT_IGS_SCL_HASH_TYPE_SVLAN_MAC       }

        },
        {
            {0, 0, 0, EGRESSXCOAMHASHTYPE_DISABLE,       CTC_PORT_EGS_SCL_HASH_TYPE_DISABLE         },
            {0, 0, 0, EGRESSXCOAMHASHTYPE_DOUBLEVLANPORT,CTC_PORT_EGS_SCL_HASH_TYPE_PORT_2VLAN      },
            {0, 0, 0, EGRESSXCOAMHASHTYPE_SVLANPORT,     CTC_PORT_EGS_SCL_HASH_TYPE_PORT_SVLAN      },
            {0, 0, 0, EGRESSXCOAMHASHTYPE_CVLANPORT,     CTC_PORT_EGS_SCL_HASH_TYPE_PORT_CVLAN      },
            {0, 0, 0, EGRESSXCOAMHASHTYPE_SVLANCOSPORT,  CTC_PORT_EGS_SCL_HASH_TYPE_PORT_SVLAN_COS  },
            {0, 0, 0, EGRESSXCOAMHASHTYPE_CVLANCOSPORT,  CTC_PORT_EGS_SCL_HASH_TYPE_PORT_CVLAN_COS  },
            {0, 0, 0, EGRESSXCOAMHASHTYPE_PORT,          CTC_PORT_EGS_SCL_HASH_TYPE_PORT            },
            {0, 0, 0, EGRESSXCOAMHASHTYPE_PORTCROSS,     CTC_PORT_EGS_SCL_HASH_TYPE_PORT_XC         },
            {0, 0, 0, EGRESSXCOAMHASHTYPE_PORTVLANCROSS, CTC_PORT_EGS_SCL_HASH_TYPE_PORT_VLAN_XC    }
        }
    };

    sys_scl_type_map_t scl_tcam_type_map[CTC_PORT_IGS_SCL_TCAM_TYPE_MAX] =
    {
        {0, 0, 0, 0xFF,        CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE   },
        {1, 0, 1, TCAML2KEY,   CTC_PORT_IGS_SCL_TCAM_TYPE_MAC       },
        {1, 0, 1, TCAML2L3KEY, CTC_PORT_IGS_SCL_TCAM_TYPE_IP        },
        {1, 0, 1, TCAML3KEY,   CTC_PORT_IGS_SCL_TCAM_TYPE_IP_SINGLE },
        {1, 0, 0, TCAML2KEY,   CTC_PORT_IGS_SCL_TCAM_TYPE_VLAN      },
        {1, 0, 0, TCAML2L3KEY,   CTC_PORT_IGS_SCL_TCAM_TYPE_TUNNEL    },
        {1, 0, 0, TCAML2L3KEY,   CTC_PORT_IGS_SCL_TCAM_TYPE_TUNNEL_RPF},
        {1, 0, 0, TCAMUSERIDKEY,   CTC_PORT_IGS_SCL_TCAM_TYPE_RESOLVE_CONFLICT},
        {1, 0, 0, TCAMUDFSHORTKEY, CTC_PORT_IGS_SCL_TCAM_TYPE_UDF},
        {1, 0, 0, TCAMUDFLONGKEY, CTC_PORT_IGS_SCL_TCAM_TYPE_UDF_EXT}
    };

    if (SYS_PORT_KEY_MAPPING_TYPE_DRV2CTC == mapping_type)
    {
        if (CTC_INGRESS == dir)
        {
            p_scl_type_para->ctc_hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
            if (USERIDPORTHASHTYPE_DISABLE == p_scl_type_para->drv_hash_type)
            {
                p_scl_type_para->ctc_hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE;
            }
            else if (USERIDPORTHASHTYPE_TUNNEL != p_scl_type_para->drv_hash_type)
            {
                for (i = 0; i < CTC_PORT_IGS_SCL_HASH_TYPE_MAX; i++)
                {
                    if ((scl_hash_type_map[CTC_INGRESS][i].drv_type == p_scl_type_para->drv_hash_type)
                        && (USERIDPORTHASHTYPE_TRILL == p_scl_type_para->drv_hash_type))
                    {
                        p_scl_type_para->ctc_hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_TRILL;
                    }
                    else if ((scl_hash_type_map[CTC_INGRESS][i].drv_type == p_scl_type_para->drv_hash_type)
                       && (scl_hash_type_map[CTC_INGRESS][i].use_macda == p_scl_type_para->use_macda))
                    {
                        p_scl_type_para->ctc_hash_type = scl_hash_type_map[CTC_INGRESS][i].ctc_type;
                        break;
                    }
                }
            }
            else
            {
                if (p_scl_type_para->tunnel_en)
                {
                    p_scl_type_para->ctc_hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_TUNNEL;
                }

                if (p_scl_type_para->tunnel_en && p_scl_type_para->tunnel_rpf_check)
                {
                    p_scl_type_para->ctc_hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_TUNNEL_RPF;
                }

                if (p_scl_type_para->auto_tunnel)
                {
                    p_scl_type_para->ctc_hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_IPV4_TUNNEL_AUTO;
                }

                if (p_scl_type_para->nvgre_en)
                {
                    p_scl_type_para->ctc_hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_NVGRE;
                }

                if (p_scl_type_para->vxlan_en)
                {
                    p_scl_type_para->ctc_hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_VXLAN;
                }
            }

            if (p_scl_type_para->tcam_en)
            {
                for (i = 0; i < CTC_PORT_IGS_SCL_TCAM_TYPE_MAX; i++)
                {
                    if (scl_tcam_type_map[i].drv_type == p_scl_type_para->drv_tcam_type)
                    {
                        p_scl_type_para->ctc_tcam_type = scl_tcam_type_map[i].ctc_type;
                        break;
                    }
                }
            }
        }
        else if (CTC_EGRESS == dir)
        {
            for (i = 0; i < CTC_PORT_EGS_SCL_HASH_TYPE_MAX; i++)
            {
                if (scl_hash_type_map[CTC_EGRESS][i].drv_type == p_scl_type_para->drv_hash_type)
                {
                    p_scl_type_para->ctc_hash_type = scl_hash_type_map[CTC_EGRESS][i].ctc_type;
                    break;
                }
            }
            p_scl_type_para->drv_tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE;
        }
    }
    else if (SYS_PORT_KEY_MAPPING_TYPE_CTC2DRV == mapping_type)
    {
        p_scl_type_para->drv_hash_type = scl_hash_type_map[dir][p_scl_type_para->ctc_hash_type].drv_type;

        if (CTC_INGRESS == dir)
        {
            p_scl_type_para->drv_hash_type = scl_hash_type_map[dir][p_scl_type_para->ctc_hash_type].drv_type;
            p_scl_type_para->use_macda = scl_hash_type_map[dir][p_scl_type_para->ctc_hash_type].use_macda;
            p_scl_type_para->vlan_high_priority = scl_hash_type_map[dir][p_scl_type_para->ctc_hash_type].vlan_high_priority;
            if (CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE == p_scl_type_para->ctc_tcam_type)
            {
                p_scl_type_para->tcam_en = 0;
            }
            else
            {
                p_scl_type_para->tcam_en = 1;
                p_scl_type_para->drv_tcam_type = scl_tcam_type_map[p_scl_type_para->ctc_tcam_type].drv_type;
            }
        }
        else if (CTC_EGRESS == dir)
        {
            p_scl_type_para->drv_hash_type = scl_hash_type_map[dir][p_scl_type_para->ctc_hash_type].drv_type;
            p_scl_type_para->tcam_en = 0;
        }
    }
    return;
}

STATIC int32
_sys_usw_port_unmap_scl_type(uint8 lchip, uint8 dir, sys_scl_type_para_t* p_scl_type_para)
{
    if (CTC_INGRESS == dir)
    {
        CTC_MAX_VALUE_CHECK(p_scl_type_para->drv_hash_type, USERIDPORTHASHTYPE_TRILL);
        CTC_MAX_VALUE_CHECK(p_scl_type_para->drv_tcam_type, MCHIP_CAP(SYS_CAP_PORT_TCAM_TYPE_NUM)-1);
    }
    else
    {
        CTC_MAX_VALUE_CHECK(p_scl_type_para->drv_hash_type, EGRESSXCOAMHASHTYPE_RMEP);
    }

    _sys_usw_port_scl_type(lchip, dir, SYS_PORT_KEY_MAPPING_TYPE_DRV2CTC, p_scl_type_para);

    if (CTC_INGRESS == dir)
    {
        CTC_MAX_VALUE_CHECK(p_scl_type_para->ctc_hash_type, CTC_PORT_IGS_SCL_HASH_TYPE_MAX - 1);
        CTC_MAX_VALUE_CHECK(p_scl_type_para->ctc_tcam_type, CTC_PORT_IGS_SCL_TCAM_TYPE_MAX - 1);
    }
    else
    {
        CTC_MAX_VALUE_CHECK(p_scl_type_para->ctc_hash_type, CTC_PORT_EGS_SCL_HASH_TYPE_MAX - 1);
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_port_map_scl_type(uint8 lchip, uint8 dir, sys_scl_type_para_t* p_scl_type_para)
{
    if (CTC_INGRESS == dir)
    {
        CTC_MAX_VALUE_CHECK(p_scl_type_para->ctc_hash_type, CTC_PORT_IGS_SCL_HASH_TYPE_MAX - 1);
        CTC_MAX_VALUE_CHECK(p_scl_type_para->ctc_tcam_type, CTC_PORT_IGS_SCL_TCAM_TYPE_MAX - 1);
    }
    else
    {
        CTC_MAX_VALUE_CHECK(p_scl_type_para->ctc_hash_type, CTC_PORT_EGS_SCL_HASH_TYPE_MAX - 1);
    }

    _sys_usw_port_scl_type(lchip, dir, SYS_PORT_KEY_MAPPING_TYPE_CTC2DRV, p_scl_type_para);

    if (CTC_INGRESS == dir)
    {
        CTC_MAX_VALUE_CHECK(p_scl_type_para->drv_hash_type, USERIDPORTHASHTYPE_TRILL);
        CTC_MAX_VALUE_CHECK(p_scl_type_para->drv_tcam_type, MCHIP_CAP(SYS_CAP_PORT_TCAM_TYPE_NUM)-1);
    }
    else
    {
        CTC_MAX_VALUE_CHECK(p_scl_type_para->drv_hash_type, EGRESSXCOAMHASHTYPE_RMEP);
    }
    return CTC_E_NONE;
}

int32
sys_usw_port_set_logic_port_type_en(uint8 lchip, uint32 gport, uint8 enable)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint16 lport = 0;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    field_val = enable ? 1 : 0;
    cmd = DRV_IOW(DsDestPort_t, DsDestPort_logicPortType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &field_val));

    return CTC_E_NONE;
}

int32
sys_usw_port_set_scl_hash_tcam_priority(uint8 lchip, uint32 gport, uint8 scl_id, uint8 is_tcam, uint8 value)
{
    uint16 lport = 0;
    uint32 cmd = 0;
    DsPhyPortExt_m ds;

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    CTC_MAX_VALUE_CHECK(scl_id, 1);

    cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds));

    if((scl_id == 0)&&(is_tcam == 0))
    {
        SetDsPhyPortExt(V, hash1Priority_f, &ds, value);
    }
    else if((scl_id == 1)&&(is_tcam == 0))
    {
        SetDsPhyPortExt(V, hash2Priority_f, &ds, value);
    }
    else if((scl_id == 0)&&(is_tcam !=0))
    {
        SetDsPhyPortExt(V, tcam1Priority_f, &ds, value);
    }
    else
    {
        SetDsPhyPortExt(V, tcam2Priority_f, &ds, value);
    }

    cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds));

    return CTC_E_NONE;

}

int32
sys_usw_port_set_scl_property(uint8 lchip, uint32 gport, ctc_port_scl_property_t* p_prop)
{
    uint16 lport = 0;
    uint32 cmd = 0;
    uint8 ipv4_tunnel_en = 0;
    uint8 ipv4_gre_tunnel_en = 0;
    uint8 auto_tunnel = 0;
    uint8 v6_tunnel_en = 0;
    uint8 auto_v6_tunnel = 0;
    uint8 v4_vxlan_en = 0;
    uint8 v6_nvgre_en = 0;
    uint8 v6_vxlan_en = 0;
    uint8 rpf_check_en = 0;
    uint8 port_vlan_dscp_valid = 0;
    uint8 auto_tunnel_hash1 = 0;
    uint8 auto_v6_tunnel_hash0 = 0;
    uint16 agg_gport = 0;
    uint32 vlan_high_priority = 0;
    int32 ret = CTC_E_NONE;
    sys_scl_type_para_t scl_type_para;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_prop);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    CTC_MAX_VALUE_CHECK(p_prop->direction, (CTC_BOTH_DIRECTION - 1));
    if (p_prop->class_id_en && p_prop->use_logic_port_en)
    {
        return CTC_E_INVALID_PARAM;
    }

	if ((CTC_PORT_IGS_SCL_TCAM_TYPE_RESOLVE_CONFLICT == p_prop->tcam_type)
        && (p_prop->scl_id >= 2))
    {
        return CTC_E_INVALID_PARAM;
    }

    /* 1.only tsingma ingress hash support port+vlan+dscp; 2.resolve conflict only support ipv4 and ipv6 tcam key */
    port_vlan_dscp_valid = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN_DSCP == p_prop->hash_type && CTC_INGRESS == p_prop->direction && !DRV_IS_DUET2(lchip);
    port_vlan_dscp_valid = port_vlan_dscp_valid && (CTC_PORT_IGS_SCL_TCAM_TYPE_RESOLVE_CONFLICT != p_prop->tcam_type);
    port_vlan_dscp_valid = port_vlan_dscp_valid || CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN_DSCP != p_prop->hash_type;
    CTC_ERROR_RETURN(port_vlan_dscp_valid? CTC_E_NONE: CTC_E_NOT_SUPPORT);

    if (CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE != p_prop->tcam_type && CTC_EGRESS == p_prop->direction)
    {
        return CTC_E_NOT_SUPPORT;
    }

    if ((CTC_EGRESS == p_prop->direction) && ((0 != p_prop->hash_vlan_range_dis) || (0 != p_prop->tcam_vlan_range_dis)))
    {
        return CTC_E_NOT_SUPPORT;
    }

    /**< [TM] Check hash lookup level */
    if ((CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE != p_prop->hash_type) || \
        (CTC_PORT_EGS_SCL_HASH_TYPE_DISABLE != p_prop->hash_type))
    {
        CTC_MAX_VALUE_CHECK(p_prop->scl_id, 1);
    }
    else    /**< [TM] Default check lookup level */
    {
        CTC_MAX_VALUE_CHECK(p_prop->scl_id, (MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM) - 1));
    }
    CTC_MAX_VALUE_CHECK(p_prop->class_id, SYS_USW_MAX_SCL_PORT_CLASSID);

    if(p_prop->class_id_en)
    {
        CTC_MIN_VALUE_CHECK(p_prop->class_id, SYS_USW_MIN_SCL_PORT_CLASSID);
    }
    sal_memset(&scl_type_para, 0, sizeof(sys_scl_type_para_t));
    scl_type_para.ctc_hash_type = p_prop->hash_type;
    scl_type_para.ctc_tcam_type = p_prop->tcam_type;
    CTC_ERROR_RETURN(_sys_usw_port_map_scl_type(lchip, p_prop->direction, &scl_type_para));

    if (CTC_INGRESS == p_prop->direction)
    {
        DsPhyPortExt_m ds;
        cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds));

        if (0 == p_prop->scl_id)
        {
            ipv4_tunnel_en = GetDsPhyPortExt(V, ipv4TunnelHashEn1_f, &ds);
            v6_tunnel_en = GetDsPhyPortExt(V, ipv6TunnelHashEn0_f, &ds);
            ipv4_gre_tunnel_en = GetDsPhyPortExt(V, ipv4GreTunnelHashEn1_f, &ds);
            auto_tunnel = GetDsPhyPortExt(V, autoTunnelEn_f, &ds);
            auto_v6_tunnel = GetDsPhyPortExt(V, autoIpv6TunnelEn_f, &ds);
            v4_vxlan_en = GetDsPhyPortExt(V, ipv4VxlanTunnelHashEn1_f, &ds);
            v6_vxlan_en = GetDsPhyPortExt(V, ipv6VxlanTunnelHashEn1_f, &ds);
            v6_nvgre_en = GetDsPhyPortExt(V, ipv6NvgreTunnelHashEn1_f, &ds);
            auto_tunnel_hash1 = GetDsPhyPortExt(V, autoTunnelForHash1_f, &ds);
            auto_v6_tunnel_hash0 = GetDsPhyPortExt(V, autoIpv6TunnelForHash0_f, &ds);
        }
        else if(1 == p_prop->scl_id)
        {
            ipv4_tunnel_en = GetDsPhyPortExt(V, ipv4TunnelHashEn2_f, &ds);
            v6_tunnel_en = GetDsPhyPortExt(V, ipv6TunnelHashEn1_f, &ds);
            ipv4_gre_tunnel_en = GetDsPhyPortExt(V, ipv4GreTunnelHashEn2_f, &ds);
            auto_tunnel = GetDsPhyPortExt(V, autoTunnelEn_f, &ds);
            rpf_check_en = GetDsPhyPortExt(V, tunnelRpfCheck_f, &ds);
            auto_v6_tunnel = GetDsPhyPortExt(V, autoIpv6TunnelEn_f, &ds);
            v4_vxlan_en = GetDsPhyPortExt(V, ipv4VxlanTunnelHashEn2_f, &ds);
            v6_nvgre_en = GetDsPhyPortExt(V, ipv6NvgreTunnelHashEn2_f, &ds);
            v6_vxlan_en = GetDsPhyPortExt(V, ipv6VxlanTunnelHashEn2_f, &ds);
            auto_tunnel_hash1 = GetDsPhyPortExt(V, autoTunnelForHash1_f, &ds);
            auto_v6_tunnel_hash0 = GetDsPhyPortExt(V, autoIpv6TunnelForHash0_f, &ds);
        }
        if (CTC_PORT_IGS_SCL_HASH_TYPE_TUNNEL == p_prop->hash_type)
        {
            ipv4_gre_tunnel_en = 1;
            v6_nvgre_en = 1;
            ipv4_tunnel_en = 1;
            v6_tunnel_en = 1;
        }
        else if (CTC_PORT_IGS_SCL_HASH_TYPE_TUNNEL_RPF == p_prop->hash_type)
        {
            ipv4_gre_tunnel_en = 1;
            ipv4_tunnel_en = 1;
            v6_tunnel_en = 1;
            v6_nvgre_en = 1;
            rpf_check_en = 1;
        }
        else if (CTC_PORT_IGS_SCL_HASH_TYPE_IPV4_TUNNEL_AUTO == p_prop->hash_type)
        {
            ipv4_gre_tunnel_en = 1;
            v6_nvgre_en = 1;
            ipv4_tunnel_en = 1;
            v6_tunnel_en = 1;
            auto_tunnel = 1;
            auto_v6_tunnel = 1;
            auto_tunnel_hash1 = (0 == p_prop->scl_id) ? 1 :0;
            auto_v6_tunnel_hash0 = (0 == p_prop->scl_id) ? 1 :0;
        }
        else if (CTC_PORT_IGS_SCL_HASH_TYPE_NVGRE == p_prop->hash_type)
        {
            ipv4_gre_tunnel_en = 1;
            v6_nvgre_en = 1;
        }
        else if (CTC_PORT_IGS_SCL_HASH_TYPE_VXLAN == p_prop->hash_type)
        {
            v4_vxlan_en = 1;
            v6_vxlan_en = 1;
        }
        else if(CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE == p_prop->hash_type)
        {
            ipv4_tunnel_en = 0;
            ipv4_gre_tunnel_en = 0;
            if(0 == p_prop->scl_id)
            {
                auto_tunnel = auto_tunnel_hash1 ? 0 : auto_tunnel;
                auto_v6_tunnel = auto_v6_tunnel_hash0 ? 0 : auto_v6_tunnel;
            }
            if(1 == p_prop->scl_id)
            {
                auto_tunnel = (!auto_tunnel_hash1) ?  0 : auto_tunnel;
                auto_v6_tunnel = (!auto_v6_tunnel_hash0)? 0 : auto_v6_tunnel;
            }

            v6_tunnel_en = 0;
            v4_vxlan_en = 0;
            v6_nvgre_en = 0;
            v6_vxlan_en = 0;
            rpf_check_en = 0;
        }

        if (0 == p_prop->scl_id)
        {
            SetDsPhyPortExt(V, userIdPortHash1Type_f, &ds, scl_type_para.drv_hash_type);
            SetDsPhyPortExt(V, hash1UseDa_f, &ds, scl_type_para.use_macda);
            SetDsPhyPortExt(V, userIdTcam1En_f, &ds, scl_type_para.tcam_en);
            SetDsPhyPortExt(V, userIdTcam1Type_f, &ds, scl_type_para.drv_tcam_type);
            SetDsPhyPortExt(V, hashLookup1UseLabel_f , &ds, (p_prop->class_id_en ? 1 : 0));
            SetDsPhyPortExt(V, userIdLabel1_f, &ds, p_prop->class_id);
            SetDsPhyPortExt(V, userId1UseLogicPort_f , &ds, (p_prop->use_logic_port_en ? 1 : 0));
            SetDsPhyPortExt(V, tcamUseLogicPort_f , &ds, (p_prop->use_logic_port_en ? 1 : 0));

            SetDsPhyPortExt(V, ipv4TunnelHashEn1_f, &ds, ipv4_tunnel_en);
            SetDsPhyPortExt(V, ipv6TunnelHashEn0_f, &ds, v6_tunnel_en);
            SetDsPhyPortExt(V, ipv4GreTunnelHashEn1_f, &ds, ipv4_gre_tunnel_en);
            SetDsPhyPortExt(V, autoTunnelEn_f, &ds, auto_tunnel);
            SetDsPhyPortExt(V, autoTunnelForHash1_f, &ds, auto_tunnel_hash1);
            SetDsPhyPortExt(V, autoIpv6TunnelEn_f, &ds, auto_v6_tunnel);
            SetDsPhyPortExt(V, autoIpv6TunnelForHash0_f, &ds, auto_v6_tunnel_hash0);
            SetDsPhyPortExt(V, ipv4VxlanTunnelHashEn1_f, &ds, v4_vxlan_en);
            SetDsPhyPortExt(V, ipv6NvgreTunnelHashEn1_f, &ds, v6_nvgre_en);
            SetDsPhyPortExt(V, ipv6VxlanTunnelHashEn1_f, &ds, v6_vxlan_en);
            /**< [TM] Append hash and tcam vlan range enable */
            SetDsPhyPortExt(V, hash1VlanRangeEn_f, &ds, (p_prop->hash_vlan_range_dis? 0: 1));
            SetDsPhyPortExt(V, tcam1VlanRangeEn_f, &ds, (p_prop->tcam_vlan_range_dis? 0: 1));
            /**< [TM] Append tcam use logic port */
            SetDsPhyPortExt(V, tcam1UseLogicPort_f , &ds, (p_prop->use_logic_port_en ? 1 : 0));
            /**< [TM] Append hash mcast disable */
            SetDsPhyPortExt(V, mcMacDisableHash0_f , &ds, (p_prop->hash_mcast_dis ? 1 : 0));
        }
        else if (1 == p_prop->scl_id)
        {
            if(CTC_PORT_IGS_SCL_HASH_TYPE_L2 == p_prop->hash_type)
            {
                return CTC_E_INVALID_PARAM;
            }

            SetDsPhyPortExt(V, userIdPortHash2Type_f, &ds, scl_type_para.drv_hash_type);
            SetDsPhyPortExt(V, hash2UseDa_f, &ds, scl_type_para.use_macda);
            SetDsPhyPortExt(V, userIdTcam2En_f, &ds,  scl_type_para.tcam_en);
            SetDsPhyPortExt(V, userIdTcam2Type_f, &ds,  scl_type_para.drv_tcam_type);
            SetDsPhyPortExt(V, hashLookup2UseLabel_f , &ds, (p_prop->class_id_en ? 1 : 0));
            SetDsPhyPortExt(V, userIdLabel2_f, &ds, p_prop->class_id);
            SetDsPhyPortExt(V, userId2UseLogicPort_f , &ds, (p_prop->use_logic_port_en ? 1 : 0));
            SetDsPhyPortExt(V, tcamUseLogicPort_f , &ds, (p_prop->use_logic_port_en ? 1 : 0));

            SetDsPhyPortExt(V, ipv4TunnelHashEn2_f, &ds, ipv4_tunnel_en);
            SetDsPhyPortExt(V, ipv6TunnelHashEn1_f, &ds, v6_tunnel_en);
            SetDsPhyPortExt(V, ipv4GreTunnelHashEn2_f, &ds, ipv4_gre_tunnel_en);
            SetDsPhyPortExt(V, autoTunnelEn_f, &ds, auto_tunnel);
            SetDsPhyPortExt(V, tunnelRpfCheck_f, &ds, rpf_check_en);
            SetDsPhyPortExt(V, autoTunnelForHash1_f, &ds, auto_tunnel_hash1);
            SetDsPhyPortExt(V, autoIpv6TunnelEn_f, &ds, auto_v6_tunnel);
            SetDsPhyPortExt(V, autoIpv6TunnelForHash0_f, &ds, auto_v6_tunnel_hash0);
            SetDsPhyPortExt(V, ipv4VxlanTunnelHashEn2_f, &ds, v4_vxlan_en);
            SetDsPhyPortExt(V, ipv6NvgreTunnelHashEn2_f, &ds, v6_nvgre_en);
            SetDsPhyPortExt(V, ipv6VxlanTunnelHashEn2_f, &ds, v6_vxlan_en);
            /**< [TM] Append hash and tcam vlan range enable */
            SetDsPhyPortExt(V, hash2VlanRangeEn_f, &ds, !(p_prop->hash_vlan_range_dis));
            SetDsPhyPortExt(V, tcam2VlanRangeEn_f, &ds, !(p_prop->tcam_vlan_range_dis));
            /**< [TM] Append tcam use logic port */
            SetDsPhyPortExt(V, tcam2UseLogicPort_f , &ds, (p_prop->use_logic_port_en ? 1 : 0));
            /**< [TM] Append hash mcast disable */
            SetDsPhyPortExt(V, mcMacDisableHash1_f , &ds, (p_prop->hash_mcast_dis ? 1 : 0));

        }
        else if (2 == p_prop->scl_id)       /**< [TM] */
        {
            /**< [TM] Refer to scl 0/1 */
            SetDsPhyPortExt(V, userIdTcam3En_f, &ds, scl_type_para.tcam_en);
            SetDsPhyPortExt(V, userIdTcam3Type_f, &ds, scl_type_para.drv_tcam_type);
            SetDsPhyPortExt(V, userIdLabel3_f, &ds, p_prop->class_id);
            /**< [TM] Append tcam vlan range enable */
            SetDsPhyPortExt(V, tcam3VlanRangeEn_f, &ds, !(p_prop->tcam_vlan_range_dis));
            /**< [TM] Append tcam use logic port */
            SetDsPhyPortExt(V, tcam3UseLogicPort_f , &ds, (p_prop->use_logic_port_en ? 1 : 0));
        }
        else if (3 == p_prop->scl_id)       /**< [TM] */
        {
            /**< [TM] Refer to scl 0/1 */
            SetDsPhyPortExt(V, userIdTcam4En_f, &ds, scl_type_para.tcam_en);
            SetDsPhyPortExt(V, userIdTcam4Type_f, &ds, scl_type_para.drv_tcam_type);
            SetDsPhyPortExt(V, userIdLabel4_f, &ds, p_prop->class_id);
            /**< [TM] Append tcam vlan range enable */
            SetDsPhyPortExt(V, tcam4VlanRangeEn_f, &ds, !(p_prop->tcam_vlan_range_dis));
            /**< [TM] Append tcam use logic port */
            SetDsPhyPortExt(V, tcam4UseLogicPort_f , &ds, (p_prop->use_logic_port_en ? 1 : 0));
        }
        else
        {
            ;
        }

        switch (p_prop->action_type)
        {
            case CTC_PORT_SCL_ACTION_TYPE_SCL:
                if (0 == p_prop->scl_id)
                {
                    SetDsPhyPortExt(V, tcam1IsUserId_f, &ds, 1);
                    SetDsPhyPortExt(V, tcam1IsSclFlow_f, &ds, 0);
                    SetDsPhyPortExt(V, sclFlowHashEn_f, &ds, 0);
                }
                else if (1 == p_prop->scl_id)
                {
                    SetDsPhyPortExt(V, tcam2IsUserId_f, &ds, 1);
                    SetDsPhyPortExt(V, tcam2IsSclFlow_f, &ds, 0);
                    /**< [TM] */
                    SetDsPhyPortExt(V, sclFlowHash2En_f, &ds, 0);
                }
                else
                {
                    ;
                }
                break;
            case CTC_PORT_SCL_ACTION_TYPE_FLOW:
                if (0 == p_prop->scl_id)
                {
                    SetDsPhyPortExt(V, tcam1IsUserId_f, &ds, 0);
                    SetDsPhyPortExt(V, tcam1IsSclFlow_f, &ds, 1);
                    SetDsPhyPortExt(V, sclFlowHashEn_f, &ds, 1);
                }
                else if (1 == p_prop->scl_id)
                {
                    if (p_prop->hash_type != CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE)
                    {
                        return CTC_E_INVALID_PARAM;
                    }
                    SetDsPhyPortExt(V, tcam2IsUserId_f, &ds, 0);
                    SetDsPhyPortExt(V, tcam2IsSclFlow_f, &ds, 1);
                    /**< [TM] */
                    SetDsPhyPortExt(V, sclFlowHash2En_f, &ds, 1);
                }
                else
                {
                    ;
                }
                break;
            case CTC_PORT_SCL_ACTION_TYPE_TUNNEL:
                if (0 == p_prop->scl_id)
                {
                    SetDsPhyPortExt(V, tcam1IsUserId_f, &ds, 0);
                    SetDsPhyPortExt(V, tcam1IsSclFlow_f, &ds, 0);
                    SetDsPhyPortExt(V, sclFlowHashEn_f, &ds, 0);
                }
                else if (1 == p_prop->scl_id)
                {
                    SetDsPhyPortExt(V, tcam2IsUserId_f, &ds, 0);
                    SetDsPhyPortExt(V, tcam2IsSclFlow_f, &ds, 0);
                    /**< [TM] */
                    SetDsPhyPortExt(V, sclFlowHash2En_f, &ds, 0);
                }
                else
                {
                    ;
                }
                break;
            default:
                return CTC_E_INVALID_PARAM;
        }

        vlan_high_priority = scl_type_para.vlan_high_priority;
        cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds));
        cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_ingressTagHighPriority_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &vlan_high_priority));

        if(!DRV_IS_DUET2(lchip) && p_prop->scl_id < 2)
        {
            if(p_prop->class_id == MCHIP_CAP(SYS_CAP_SCL_LABEL_FOR_IPSG) && \
                (p_prop->hash_type != CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE || p_prop->tcam_type != CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE))
            {
                sys_usw_port_get_global_port(lchip, lport, &agg_gport);
                ret = sys_usw_ip_source_guard_add_default_entry(lchip, agg_gport, p_prop->scl_id);
                if (CTC_E_ENTRY_EXIST != ret && CTC_E_NONE != ret)
                {
                    return ret;
                }
            }
            else
            {
                ret = sys_usw_ip_source_guard_remove_default_entry(lchip, gport, p_prop->scl_id);
                if(CTC_E_NONE != ret && CTC_E_ENTRY_NOT_EXIST != ret)
                {
                    return ret;
                }
            }
        }
        else if (DRV_IS_DUET2(lchip) && p_prop->class_id == MCHIP_CAP(SYS_CAP_SCL_LABEL_FOR_IPSG) && \
                     (p_prop->hash_type != CTC_PORT_IGS_SCL_HASH_TYPE_DISABLE || p_prop->tcam_type != CTC_PORT_IGS_SCL_TCAM_TYPE_DISABLE))
        {
            CTC_ERROR_RETURN(sys_usw_ip_source_guard_add_default_entry(lchip, 0, p_prop->scl_id));
        }
    }
    else if (CTC_EGRESS == p_prop->direction)
    {
        DsDestPort_m ds;

        cmd = DRV_IOR(DsDestPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds));

        if (0 == p_prop->scl_id)
        {
            SetDsDestPort(V, vlanHash1Type_f, &ds, scl_type_para.drv_hash_type);
            SetDsDestPort(V, vlanHash1UseLogicPort_f, &ds, p_prop->use_logic_port_en);
            SetDsDestPort(V, vlanHash1UseLabel_f, &ds, p_prop->class_id_en);
        }
        else if (1 == p_prop->scl_id)
        {
            SetDsDestPort(V, vlanHash2Type_f, &ds, scl_type_para.drv_hash_type);
            SetDsDestPort(V, vlanHash2UseLogicPort_f, &ds, p_prop->use_logic_port_en);
            SetDsDestPort(V, vlanHash2UseLabel_f, &ds, p_prop->class_id_en);
            SetDsDestPort(V, vlanHash1DefaultEntryValid_f, &ds, (scl_type_para.drv_hash_type?0:1));
        }

        SetDsDestPort(V, vlanHashLabel_f, &ds, p_prop->class_id);
        SetDsDestPort(V, vlanHashUseLogicPort_f, &ds, p_prop->use_logic_port_en);
        SetDsDestPort(V, vlanHashUseLabel_f, &ds, p_prop->class_id_en);

        cmd = DRV_IOW(DsDestPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds));
    }

    return CTC_E_NONE;
}

int32
sys_usw_port_get_scl_property(uint8 lchip, uint32 gport, ctc_port_scl_property_t* p_prop)
{
    uint16 lport = 0;
    uint32 cmd = 0;
    uint8 tcam_is_userid = 0;
    uint8 tcam_is_scl_flow = 0;
    sys_scl_type_para_t scl_type_para;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_prop);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    CTC_MAX_VALUE_CHECK(p_prop->direction, CTC_BOTH_DIRECTION - 1);
    CTC_MAX_VALUE_CHECK(p_prop->scl_id, (MCHIP_CAP(SYS_CAP_SCL_TCAM_NUM)-1));

    if (1 < p_prop->scl_id && CTC_EGRESS == p_prop->direction)
    {
        return CTC_E_NOT_SUPPORT;
    }

    sal_memset(&scl_type_para, 0, sizeof(sys_scl_type_para_t));

    if (CTC_INGRESS == p_prop->direction)
    {
        DsPhyPortExt_m ds;

        cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds));

        if (0 == p_prop->scl_id)
        {
            scl_type_para.drv_hash_type = GetDsPhyPortExt(V, userIdPortHash1Type_f, &ds);
            scl_type_para.use_macda = GetDsPhyPortExt(V, hash1UseDa_f, &ds);
            scl_type_para.tcam_en = GetDsPhyPortExt(V, userIdTcam1En_f, &ds);
            scl_type_para.drv_tcam_type = GetDsPhyPortExt(V, userIdTcam1Type_f, &ds);
            scl_type_para.tunnel_en = GetDsPhyPortExt(V, ipv4TunnelHashEn1_f, &ds) && GetDsPhyPortExt(V, ipv4GreTunnelHashEn1_f, &ds);
            scl_type_para.auto_tunnel = GetDsPhyPortExt(V, autoTunnelEn_f, &ds);
            scl_type_para.auto_tunnel &= GetDsPhyPortExt(V, autoTunnelForHash1_f, &ds);
            /* nvgre full condition */
            scl_type_para.nvgre_en = GetDsPhyPortExt(V, nvgreEnable_f, &ds);
            scl_type_para.nvgre_en &= (GetDsPhyPortExt(V, ipv4GreTunnelHashEn1_f, &ds) || GetDsPhyPortExt(V, ipv6NvgreTunnelHashEn1_f, &ds));
            /* vxlan full condition */
            scl_type_para.vxlan_en = GetDsPhyPortExt(V, ipv4VxlanTunnelHashEn1_f, &ds) || GetDsPhyPortExt(V, ipv6VxlanTunnelHashEn1_f, &ds);
            p_prop->class_id_en = GetDsPhyPortExt(V, hashLookup1UseLabel_f , &ds);
            p_prop->class_id = GetDsPhyPortExt(V, userIdLabel1_f , &ds);
            p_prop->use_logic_port_en = GetDsPhyPortExt(V, userId1UseLogicPort_f , &ds);
            tcam_is_userid = GetDsPhyPortExt(V, tcam1IsUserId_f, &ds);
            tcam_is_scl_flow = GetDsPhyPortExt(V, tcam1IsSclFlow_f, &ds);
            p_prop->hash_vlan_range_dis = !GetDsPhyPortExt(V, hash1VlanRangeEn_f, &ds);
            p_prop->tcam_vlan_range_dis = !GetDsPhyPortExt(V, tcam1VlanRangeEn_f, &ds);
        }
        else if (1 == p_prop->scl_id)
        {
            if(CTC_PORT_IGS_SCL_HASH_TYPE_L2 == p_prop->hash_type)
            {
                return CTC_E_INVALID_PARAM;
            }

            scl_type_para.drv_hash_type = GetDsPhyPortExt(V, userIdPortHash2Type_f, &ds);
            scl_type_para.use_macda = GetDsPhyPortExt(V, hash2UseDa_f, &ds);
            scl_type_para.tcam_en = GetDsPhyPortExt(V, userIdTcam2En_f, &ds);
            scl_type_para.drv_tcam_type = GetDsPhyPortExt(V, userIdTcam2Type_f, &ds);
            scl_type_para.tunnel_en = GetDsPhyPortExt(V, ipv4TunnelHashEn2_f, &ds) && GetDsPhyPortExt(V, ipv4GreTunnelHashEn2_f, &ds);
            scl_type_para.tunnel_rpf_check = scl_type_para.tunnel_en && GetDsPhyPortExt(V, tunnelRpfCheck_f, &ds);
            scl_type_para.auto_tunnel = GetDsPhyPortExt(V, autoTunnelEn_f, &ds);
            scl_type_para.auto_tunnel &= (!GetDsPhyPortExt(V, autoTunnelForHash1_f, &ds));
            /* nvgre full condition */
            scl_type_para.nvgre_en = GetDsPhyPortExt(V, nvgreEnable_f, &ds);
            scl_type_para.nvgre_en &= (GetDsPhyPortExt(V, ipv4GreTunnelHashEn2_f, &ds) || GetDsPhyPortExt(V, ipv6NvgreTunnelHashEn2_f, &ds));
            /* vxlan full condition */
            scl_type_para.vxlan_en = GetDsPhyPortExt(V, ipv4VxlanTunnelHashEn2_f, &ds) || GetDsPhyPortExt(V, ipv6VxlanTunnelHashEn2_f, &ds);
            p_prop->class_id_en = GetDsPhyPortExt(V, hashLookup2UseLabel_f , &ds);
            p_prop->class_id = GetDsPhyPortExt(V, userIdLabel2_f , &ds);
            p_prop->use_logic_port_en = GetDsPhyPortExt(V, userId2UseLogicPort_f , &ds);
            tcam_is_userid = GetDsPhyPortExt(V, tcam2IsUserId_f, &ds);
            tcam_is_scl_flow = GetDsPhyPortExt(V, tcam2IsSclFlow_f, &ds);
            p_prop->hash_vlan_range_dis = !GetDsPhyPortExt(V, hash2VlanRangeEn_f, &ds);
            p_prop->tcam_vlan_range_dis = !GetDsPhyPortExt(V, tcam2VlanRangeEn_f, &ds);
        }
        else if (2 == p_prop->scl_id)
        {
            /**< [TM] Refer to scl 0/1 */
            scl_type_para.tcam_en = GetDsPhyPortExt(V, userIdTcam3En_f, &ds);
            scl_type_para.drv_tcam_type = GetDsPhyPortExt(V, userIdTcam3Type_f, &ds);
            p_prop->class_id = GetDsPhyPortExt(V, userIdLabel3_f, &ds);
            /**< [TM] Append tcam vlan range enable */
            p_prop->tcam_vlan_range_dis = !GetDsPhyPortExt(V, tcam3VlanRangeEn_f, &ds);
            /**< [TM] Append tcam use logic port */
            p_prop->use_logic_port_en = GetDsPhyPortExt(V, tcam3UseLogicPort_f , &ds);
        }
        else if (3 == p_prop->scl_id)
        {
            /**< [TM] Refer to scl 0/1 */
            scl_type_para.tcam_en = GetDsPhyPortExt(V, userIdTcam4En_f, &ds);
            scl_type_para.drv_tcam_type = GetDsPhyPortExt(V, userIdTcam4Type_f, &ds);
            p_prop->class_id = GetDsPhyPortExt(V, userIdLabel4_f, &ds);
            /**< [TM] Append tcam vlan range enable */
            p_prop->tcam_vlan_range_dis = !GetDsPhyPortExt(V, tcam4VlanRangeEn_f, &ds);
            /**< [TM] Append tcam use logic port */
            p_prop->use_logic_port_en = GetDsPhyPortExt(V, tcam4UseLogicPort_f , &ds);
        }
        else
        {
            ;
        }

        if (tcam_is_scl_flow)
        {
            p_prop->action_type = CTC_PORT_SCL_ACTION_TYPE_FLOW;
        }
        else if (tcam_is_userid)
        {
            p_prop->action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
        }
        else
        {
            p_prop->action_type = CTC_PORT_SCL_ACTION_TYPE_TUNNEL;
        }
    }
    else if (CTC_EGRESS == p_prop->direction)
    {
        DsDestPort_m ds;

        cmd = DRV_IOR(DsDestPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds));

        if (0 == p_prop->scl_id)
        {
            scl_type_para.drv_hash_type = GetDsDestPort(V, vlanHash1Type_f, &ds);
            p_prop->use_logic_port_en = GetDsDestPort(V, vlanHash1UseLogicPort_f, &ds);
            p_prop->class_id_en = GetDsDestPort(V, vlanHash1UseLabel_f, &ds);

        }
        else
        {
            scl_type_para.drv_hash_type = GetDsDestPort(V, vlanHash2Type_f, &ds);
            p_prop->use_logic_port_en = GetDsDestPort(V, vlanHash2UseLogicPort_f, &ds);
            p_prop->class_id_en = GetDsDestPort(V, vlanHash2UseLabel_f, &ds);

        }

        p_prop->class_id = GetDsDestPort(V, vlanHashLabel_f, &ds);
        if (DRV_FLD_IS_EXISIT(DsDestPort_t, DsDestPort_vlanHashUseLabel_f))
        {
            p_prop->class_id_en = GetDsDestPort(V, vlanHashUseLabel_f, &ds);
        }
        if (DRV_FLD_IS_EXISIT(DsDestPort_t, DsDestPort_vlanHashUseLogicPort_f))
        {
            p_prop->use_logic_port_en = GetDsDestPort(V, vlanHashUseLogicPort_f, &ds);
        }
    }

    CTC_ERROR_RETURN(_sys_usw_port_unmap_scl_type(lchip, p_prop->direction, &scl_type_para));

    p_prop->hash_type = scl_type_para.ctc_hash_type;
    p_prop->tcam_type = scl_type_para.ctc_tcam_type;

    return CTC_E_NONE;
}

int32
sys_usw_port_set_acl_property(uint8 lchip, uint32 gport, ctc_acl_property_t* p_prop)
{
    uint8  acl_priority = 0;
    uint16 lport = 0;
    uint32 cmd = 0;
    uint32 step = 0;
    uint8  sys_lkup_type = 0;
    uint8  gport_type = 0;

    DsSrcPort_m ds_src_port;
    DsDestPort_m ds_dest_port;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_prop);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    SYS_ACL_PROPERTY_CHECK(p_prop);

    if (CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP)
        + CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_METADATA)
        + CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT) > 1)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Port bitmap Metedata and Logic port can not be configured at the same time \n");
        return CTC_E_NOT_SUPPORT;
    }

    if (CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_HASH_LKUP))
    {
        cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_src_port));
        if(p_prop->acl_en)
        {
            CTC_MAX_VALUE_CHECK(p_prop->hash_field_sel_id, 0xF);
            CTC_MAX_VALUE_CHECK(p_prop->hash_lkup_type, CTC_ACL_HASH_LKUP_TYPE_MAX-1);
            SetDsSrcPort(V, aclFlowHashFieldSel_f, &ds_src_port, p_prop->hash_field_sel_id);
            SetDsSrcPort(V, aclFlowHashType_f,     &ds_src_port, p_prop->hash_lkup_type);
            SetDsSrcPort(V, flowHashUsePiVlan_f,     &ds_src_port, CTC_FLAG_ISSET(p_prop->flag,CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN)?1:0);
        }
        else
        {
            SetDsSrcPort(V, aclFlowHashFieldSel_f, &ds_src_port, 0);
            SetDsSrcPort(V, aclFlowHashType_f,     &ds_src_port, 0);
            SetDsSrcPort(V, flowHashUsePiVlan_f,   &ds_src_port, 0);
        }
        cmd = DRV_IOW(DsSrcPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip,lport,cmd,&ds_src_port));

        return CTC_E_NONE;
    }
    acl_priority = p_prop->acl_priority;
    if(CTC_ACL_TCAM_LKUP_TYPE_VLAN == p_prop->tcam_lkup_type)
    {
        return CTC_E_NOT_SUPPORT;
    }
    sys_lkup_type = sys_usw_map_acl_tcam_lkup_type(lchip, p_prop->tcam_lkup_type);
    if(CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP))
    {
        gport_type = DRV_FLOWPORTTYPE_BITMAP;
    }
    else if(CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_METADATA))
    {
        gport_type = DRV_FLOWPORTTYPE_METADATA;
    }
    else if(CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT))
    {
        gport_type = DRV_FLOWPORTTYPE_LPORT;
    }
    else
    {
        gport_type = DRV_FLOWPORTTYPE_GPORT;
    }

    if(p_prop->direction == CTC_INGRESS )
    {
        step = DsSrcPort_gAcl_1_aclLookupType_f - DsSrcPort_gAcl_0_aclLookupType_f;
        cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_src_port));

        if(0 == p_prop->acl_en)
        {
            /*if acl disable, clear associated config*/
            SetDsSrcPort(V, gAcl_0_aclLabel_f + acl_priority*step,             &ds_src_port, 0);
            SetDsSrcPort(V, gAcl_0_aclUseGlobalPortType_f + acl_priority*step, &ds_src_port, 0);
            SetDsSrcPort(V, gAcl_0_aclUsePIVlan_f + acl_priority*step,         &ds_src_port, 0);
            SetDsSrcPort(V, gAcl_0_aclLookupType_f + acl_priority*step,        &ds_src_port, 0);
            SetDsSrcPort(V, gAcl_0_aclUseCapwapInfo_f + acl_priority*step,     &ds_src_port, 0);
        }
        else
        {
            SetDsSrcPort(V, gAcl_0_aclLookupType_f + acl_priority*step,        &ds_src_port, sys_lkup_type);
            SetDsSrcPort(V, gAcl_0_aclLabel_f + acl_priority*step,             &ds_src_port, p_prop->class_id);
            SetDsSrcPort(V, gAcl_0_aclUseGlobalPortType_f + acl_priority*step, &ds_src_port, gport_type);
            SetDsSrcPort(V, gAcl_0_aclUsePIVlan_f + acl_priority*step,         &ds_src_port,
                                                CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN)?1:0);
            SetDsSrcPort(V, gAcl_0_aclUseCapwapInfo_f + acl_priority*step,     &ds_src_port,
                                                CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_WLAN)?1:0);
        }
        /*write hw*/
        cmd = DRV_IOW(DsSrcPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip,lport,cmd,&ds_src_port));
    }
    else
    {
        step = DsDestPort_gAcl_1_aclLookupType_f - DsDestPort_gAcl_0_aclLookupType_f;
        cmd = DRV_IOR(DsDestPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_dest_port));

        if(0 == p_prop->acl_en)
        {
            /*if acl disable, clear associated config*/
            SetDsDestPort(V, gAcl_0_aclLabel_f + acl_priority*step,             &ds_dest_port, 0);
            SetDsDestPort(V, gAcl_0_aclUseGlobalPortType_f + acl_priority*step, &ds_dest_port, 0);
            SetDsDestPort(V, gAcl_0_aclUsePIVlan_f + acl_priority*step,         &ds_dest_port, 0);
            SetDsDestPort(V, gAcl_0_aclLookupType_f + acl_priority*step,        &ds_dest_port, 0);
        }
        else
        {
            SetDsDestPort(V, gAcl_0_aclLookupType_f + acl_priority*step,        &ds_dest_port, sys_lkup_type);
            SetDsDestPort(V, gAcl_0_aclLabel_f + acl_priority*step,             &ds_dest_port, p_prop->class_id);
            SetDsDestPort(V, gAcl_0_aclUseGlobalPortType_f + acl_priority*step, &ds_dest_port, gport_type);
            SetDsDestPort(V, gAcl_0_aclUsePIVlan_f + acl_priority*step,         &ds_dest_port,
                                                CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN)?1:0);
        }
        /*write hw*/
        cmd = DRV_IOW(DsDestPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip,lport,cmd,&ds_dest_port));
    }

    return CTC_E_NONE;
}

int32
sys_usw_port_get_acl_property(uint8 lchip, uint32 gport, ctc_acl_property_t* p_prop)
{
    uint8  acl_priority = 0;
    uint16 lport = 0;
    uint32 cmd = 0;
    uint32 step = 0;
    uint8  gport_type = 0;
    uint8  sys_lkup_type = 0;
    uint8  use_mapped_vlan = 0;
    uint8  use_wlan = 0;

    DsSrcPort_m ds_src_port;
    DsDestPort_m ds_dest_port;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_prop);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    CTC_MAX_VALUE_CHECK(p_prop->direction, CTC_EGRESS);

    if (CTC_FLAG_ISSET(p_prop->flag, CTC_ACL_PROP_FLAG_USE_HASH_LKUP))
    {
        cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ds_src_port));

        p_prop->hash_field_sel_id = GetDsSrcPort(V, aclFlowHashFieldSel_f,&ds_src_port);
        p_prop->hash_lkup_type = GetDsSrcPort(V, aclFlowHashType_f,&ds_src_port);
        if(GetDsSrcPort(V, flowHashUsePiVlan_f,&ds_src_port))
        {
            CTC_SET_FLAG(p_prop->flag,CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN);
        }
        p_prop->acl_en = p_prop->hash_lkup_type ? 1 : 0;

        return CTC_E_NONE;
    }

    acl_priority = p_prop->acl_priority;
    if(p_prop->direction == CTC_INGRESS)
    {
        CTC_MAX_VALUE_CHECK(acl_priority, MCHIP_CAP(SYS_CAP_ACL_INGRESS_ACL_LKUP)-1);
        step = DsSrcPort_gAcl_1_aclLookupType_f - DsSrcPort_gAcl_0_aclLookupType_f;
        cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip,lport,cmd,&ds_src_port));

        sys_lkup_type = GetDsSrcPort(V,gAcl_0_aclLookupType_f + acl_priority*step,          &ds_src_port);
        gport_type    = GetDsSrcPort(V,gAcl_0_aclUseGlobalPortType_f + acl_priority*step,   &ds_src_port);
        p_prop->class_id =  GetDsSrcPort(V,gAcl_0_aclLabel_f + acl_priority*step,           &ds_src_port);
        use_mapped_vlan = GetDsSrcPort(V,gAcl_0_aclUsePIVlan_f + acl_priority*step,         &ds_src_port);
        use_wlan = GetDsSrcPort(V,gAcl_0_aclUseCapwapInfo_f + acl_priority*step,          &ds_src_port);
    }
    else
    {
        CTC_MAX_VALUE_CHECK(acl_priority, MCHIP_CAP(SYS_CAP_ACL_EGRESS_LKUP)-1);
        step = DsDestPort_gAcl_1_aclLookupType_f - DsDestPort_gAcl_0_aclLookupType_f;
        cmd = DRV_IOR(DsDestPort_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip,lport,cmd,&ds_dest_port));

        sys_lkup_type = GetDsDestPort(V,gAcl_0_aclLookupType_f + acl_priority*step,          &ds_dest_port);
        gport_type    = GetDsDestPort(V,gAcl_0_aclUseGlobalPortType_f + acl_priority*step,   &ds_dest_port);
        p_prop->class_id = GetDsDestPort(V,gAcl_0_aclLabel_f + acl_priority*step,            &ds_dest_port);
        use_mapped_vlan = GetDsDestPort(V,gAcl_0_aclUsePIVlan_f + acl_priority*step,         &ds_dest_port);
    }

    p_prop->acl_en = sys_lkup_type ? 1 : 0;
    p_prop->tcam_lkup_type  = sys_usw_unmap_acl_tcam_lkup_type(lchip, sys_lkup_type);
    if(use_mapped_vlan)
    {
        CTC_SET_FLAG(p_prop->flag, CTC_ACL_PROP_FLAG_USE_MAPPED_VLAN);
    }
    if(use_wlan)
    {
        CTC_SET_FLAG(p_prop->flag, CTC_ACL_PROP_FLAG_USE_WLAN);
    }

    switch(gport_type)
    {
        case DRV_FLOWPORTTYPE_BITMAP:
            CTC_SET_FLAG(p_prop->flag, CTC_ACL_PROP_FLAG_USE_PORT_BITMAP);
            break;
        case DRV_FLOWPORTTYPE_LPORT:
            CTC_SET_FLAG(p_prop->flag, CTC_ACL_PROP_FLAG_USE_LOGIC_PORT);
            break;
        case DRV_FLOWPORTTYPE_METADATA:
            CTC_SET_FLAG(p_prop->flag, CTC_ACL_PROP_FLAG_USE_METADATA);
            break;
        default:
            break;
    }

    return CTC_E_NONE;
}

int32
sys_usw_port_set_bpe_property(uint8 lchip, uint32 gport, ctc_port_bpe_property_t* p_prop)
{
    uint32 cmd = 0;
    uint8 chan_id = 0;
    uint16 lport = 0;
    uint32 src_mux_type = 0;
    uint32 dst_mux_type = 0;
    uint32 ecid = 0;
    uint32 name_space = 0;
    uint32 hash1_type = 0;
    uint32 hash2_type = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_prop);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    CTC_MAX_VALUE_CHECK(p_prop->name_space, (2*1024 - 1));
    CTC_MAX_VALUE_CHECK(p_prop->ecid, 4095);

    /*get channel from port */
    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (chan_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
        return CTC_E_INVALID_PORT;
    }

    switch(p_prop->extend_type)
    {
    case CTC_PORT_BPE_EXTEND_TYPE_NONE:
        src_mux_type = 0;
        dst_mux_type = 0;
        ecid = 0;
        name_space =  0;
		hash1_type = 0;
		hash2_type = 0;
        CTC_ERROR_RETURN(sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 0));
        break;
    case CTC_PORT_BPE_8021BR_PE_EXTEND:
        CTC_MIN_VALUE_CHECK(p_prop->ecid, 1);
        src_mux_type = 0;
        dst_mux_type = 0;
        ecid = p_prop->ecid;
        name_space =  0;
		hash1_type = 0;
		hash2_type = 0;
        CTC_ERROR_RETURN(sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1));

        break;
    case CTC_PORT_BPE_8021BR_PE_CASCADE:
        src_mux_type = BPE_IGS_MUXTYPE_PE_DOWNLINK_WITH_CASCADE_PORT;
        dst_mux_type = BPE_EGS_MUXTYPE_CASCADE;
		ecid = 0;
        name_space =  p_prop->name_space;
		hash1_type = USERIDPORTHASHTYPE_ECIDNAMESPACE;
		hash2_type = USERIDPORTHASHTYPE_INGECIDNAMESPACE;

        break;
    case CTC_PORT_BPE_8021BR_PE_UPSTREAM:
        src_mux_type = BPE_IGS_MUXTYPE_PE_UPLINK;
        dst_mux_type = BPE_EGS_MUXTYPE_UPSTREAM;
		ecid = 0;
        name_space =  p_prop->name_space;
		hash1_type = USERIDPORTHASHTYPE_ECIDNAMESPACE;
		hash2_type = USERIDPORTHASHTYPE_INGECIDNAMESPACE;

        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOW(IpeHeaderAdjustPhyPortMap_t, IpeHeaderAdjustPhyPortMap_ecidNameSpace_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &name_space));

    cmd = DRV_IOW(DsPortLinkAgg_t, DsPortLinkAgg_ecid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &ecid));

    cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_userIdPortHash1Type_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &hash1_type));

    cmd = DRV_IOW(DsPhyPortExt_t, DsPhyPortExt_userIdPortHash2Type_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &hash2_type));

    /* cfg mux type */
    cmd = DRV_IOW(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &src_mux_type));

    cmd = DRV_IOW(DsDestPort_t, DsDestPort_muxPortType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &dst_mux_type));


    return CTC_E_NONE;
}

int32
sys_usw_port_get_bpe_property(uint8 lchip, uint32 gport, ctc_port_bpe_property_t* p_prop)
{
    uint32 cmd = 0;
    uint8 chan_id = 0;
    uint16 lport = 0;
    uint32 src_mux_type = 0;
    uint32 ecid = 0;
    uint32 name_space = 0;

    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_PORT_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_prop);

    sal_memset(p_prop, 0, sizeof(ctc_port_bpe_property_t));

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    /*get channel from port */
    chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
    if (chan_id >= MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM))
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid localphy port \n");
        return CTC_E_INVALID_PORT;
    }


    cmd = DRV_IOR(IpeHeaderAdjustPhyPortMap_t, IpeHeaderAdjustPhyPortMap_ecidNameSpace_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &name_space));

    cmd = DRV_IOR(DsPortLinkAgg_t, DsPortLinkAgg_ecid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &ecid));

    /* cfg mux type */
    cmd = DRV_IOR(IpePhyPortMuxCtl_t, IpePhyPortMuxCtl_muxType_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &src_mux_type));

    switch(src_mux_type)
    {
    case BPE_IGS_MUXTYPE_PE_DOWNLINK_WITH_CASCADE_PORT:
        p_prop->extend_type = CTC_PORT_BPE_8021BR_PE_CASCADE;
        p_prop->name_space = name_space;
        break;

    case BPE_IGS_MUXTYPE_PE_UPLINK:
        p_prop->extend_type = CTC_PORT_BPE_8021BR_PE_UPSTREAM;
        p_prop->name_space = name_space;
        break;


    case BPE_IGS_MUXTYPE_NOMUX:
        if (ecid)
        {
            p_prop->extend_type = CTC_PORT_BPE_8021BR_PE_EXTEND;
			p_prop->ecid = ecid;
        }
        else
        {
            p_prop->extend_type = CTC_PORT_BPE_EXTEND_TYPE_NONE;
        }
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }


    return CTC_E_NONE;
}

int32
sys_usw_port_register_rchip_get_gport_idx_cb(uint8 lchip, SYS_PORT_GET_RCHIP_GPORT_IDX_CB cb)
{
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_PORT_INIT_CHECK();
    p_usw_port_master[lchip]->rchip_gport_idx_cb  = cb;

    return CTC_E_NONE;
}
int32
sys_usw_port_set_mac_auth(uint8 lchip, uint32 gport, bool enable)
{
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (DRV_IS_DUET2(lchip))
    {
        CTC_ERROR_RETURN(sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_SRC_MISMATCH_EXCEPTION_EN, enable));
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_SECURITY_EN, enable));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_MAC_AUTH_EN, enable));
    }

    return CTC_E_NONE;
}

int32
sys_usw_port_get_mac_auth(uint8 lchip, uint32 gport, bool* enable)
{
    uint32 value1 = 0;
    uint32 value2 = 0;

    CTC_PTR_VALID_CHECK(enable);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (DRV_IS_DUET2(lchip))
    {
        CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_SRC_MISMATCH_EXCEPTION_EN, &value1));
        CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_SECURITY_EN, &value2));

        *enable = ((value1 & value2) == 1) ? TRUE : FALSE;
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_MAC_AUTH_EN, &value1));
        *enable = (value1 == 1) ? TRUE : FALSE;
    }

    return CTC_E_NONE;
}

int32
sys_usw_port_get_capability(uint8 lchip, uint32 gport, ctc_port_capability_type_t type, void* p_value)
{

    CTC_PTR_VALID_CHECK(p_value);
    SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_mac_get_capability(lchip, gport, type, p_value));

    return CTC_E_NONE;
}
int32 sys_usw_port_wb_sync(uint8 lchip)
{
    uint32 loop = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_wb_port_master_t port_master;
    sys_wb_port_prop_t   port_prop;
    uint32  max_entry_cnt = 0;

    sal_memset(&port_master, 0, sizeof(sys_wb_l3if_prop_t));
    sal_memset(&port_prop, 0, sizeof(port_prop));

    wb_data.buffer = (ctc_wb_key_data_t*)mem_malloc(MEM_PORT_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_port_master_t, CTC_FEATURE_PORT, SYS_WB_APPID_PORT_SUBID_MASTER);
    /*mapping data*/
    port_master.lchip = lchip;
    port_master.use_logic_port_check = p_usw_port_master[lchip]->use_logic_port_check;
    port_master.use_isolation_id = p_usw_port_master[lchip]->use_isolation_id;
    port_master.isolation_group_mode = p_usw_port_master[lchip]->isolation_group_mode;
    port_master.igs_isloation_bitmap = p_usw_port_master[lchip]->igs_isloation_bitmap[0];
    port_master.igs_isloation_bitmap1 = p_usw_port_master[lchip]->igs_isloation_bitmap[1];
    port_master.version = SYS_WB_VERSION_PORT;
    sal_memcpy(port_master.mac_bmp, p_usw_port_master[lchip]->mac_bmp, sizeof(p_usw_port_master[lchip]->mac_bmp));

    sal_memcpy((uint8*)wb_data.buffer, (uint8*)&port_master, sizeof(port_master));
    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_port_prop_t, CTC_FEATURE_PORT, SYS_WB_APPID_PORT_SUBID_PROP);
    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH/(wb_data.key_len + wb_data.data_len);
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    do
    {
        port_prop.lchip = lchip;
        port_prop.lport = loop;
        sal_memcpy((uint8*)&port_prop.igs_port_prop, (uint8*)&p_usw_port_master[lchip]->igs_port_prop[loop], sizeof(sys_igs_port_prop_t));
        sal_memcpy((uint8*)&port_prop.egs_port_prop, (uint8*)&p_usw_port_master[lchip]->egs_port_prop[loop], sizeof(sys_egs_port_prop_t));
        port_prop.network_port = p_usw_port_master[lchip]->network_port[loop];

        sal_memcpy((uint8*)wb_data.buffer + wb_data.valid_cnt * sizeof(sys_wb_port_prop_t),  (uint8*)&port_prop, sizeof(sys_wb_port_prop_t));
        if (++wb_data.valid_cnt == max_entry_cnt)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
            wb_data.valid_cnt = 0;
        }
    }while(++loop < SYS_USW_MAX_PORT_NUM_PER_CHIP);

    if(wb_data.valid_cnt)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
    }

done:
    if (wb_data.buffer )
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32 sys_usw_port_wb_restore(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t    wb_query;

    sys_wb_port_master_t  wb_port_master;
    sys_wb_port_prop_t    wb_port_prop;

    sys_wb_port_master_t* p_wb_port_master = &wb_port_master;
    sys_wb_port_prop_t* p_wb_port_prop = &wb_port_prop;
    uint32 entry_cnt = 0;
    uint32 lport = 0;
    sys_usw_opf_t opf;
    uint32  cmd = 0;
    uint32  random_en = 0;
    uint32  port_sflow = 0;
    uint8   loop = 0;
    /*restore port property*/
    wb_query.buffer = (ctc_wb_key_data_t*)mem_malloc(MEM_PORT_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_port_master_t, CTC_FEATURE_PORT, SYS_WB_APPID_PORT_SUBID_MASTER);

    /*set default value*/
    sal_memset(&wb_port_master, 0, sizeof(wb_port_master));
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)p_wb_port_master, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_PORT, p_wb_port_master->version))
        {
            ret = CTC_E_VERSION_MISMATCH;
            goto done;
        }
        p_usw_port_master[lchip]->use_logic_port_check = p_wb_port_master->use_logic_port_check;
        p_usw_port_master[lchip]->use_isolation_id = p_wb_port_master->use_isolation_id;
        p_usw_port_master[lchip]->isolation_group_mode = p_wb_port_master->isolation_group_mode;
        p_usw_port_master[lchip]->igs_isloation_bitmap[0] = p_wb_port_master->igs_isloation_bitmap;
        p_usw_port_master[lchip]->igs_isloation_bitmap[1] = p_wb_port_master->igs_isloation_bitmap1;

    if (!DRV_IS_DUET2(lchip))
    {
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type = p_usw_port_master[lchip]->opf_type_port_sflow;
        for (loop=0; loop<SYS_PHY_PORT_NUM_PER_SLICE; loop+=1)
        {
            /*restore ingress port sflow*/
            cmd = DRV_IOR(DsPhyPortExt_t, DsPhyPortExt_randomLogEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, loop, cmd, &random_en));
            if (random_en)
            {
                opf.pool_index = CTC_INGRESS;
                cmd = DRV_IOR(IpeRandomSeedMap_t, IpeRandomSeedMap_seedId_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, loop, cmd, &port_sflow));
                if(port_sflow)
                {
                    sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, port_sflow);
                }
            }
            /*restore egress port sflow*/
            cmd = DRV_IOR(DsDestPort_t, DsDestPort_randomLogEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, loop, cmd, &random_en));
            if (random_en)
            {
                opf.pool_index = CTC_EGRESS;
                cmd = DRV_IOR(EpeRandomSeedMap_t, EpeRandomSeedMap_seedId_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, loop, cmd, &port_sflow));
                if(port_sflow)
                {
                    sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, port_sflow);
                }
            }
        }
    }

    CTC_WB_QUERY_ENTRY_END((&wb_query));

    CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_port_prop_t, CTC_FEATURE_PORT, SYS_WB_APPID_PORT_SUBID_PROP);

    /*set default value*/
    sal_memset(&wb_port_prop, 0, sizeof(wb_port_prop));

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)p_wb_port_prop, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        lport = p_wb_port_prop->lport;
        sal_memcpy((uint8*)&p_usw_port_master[lchip]->igs_port_prop[lport], (uint8*)&p_wb_port_prop->igs_port_prop, sizeof(sys_igs_port_prop_t));
        sal_memcpy((uint8*)&p_usw_port_master[lchip]->egs_port_prop[lport], (uint8*)&p_wb_port_prop->egs_port_prop, sizeof(sys_egs_port_prop_t));
        p_usw_port_master[lchip]->network_port[lport] = p_wb_port_prop->network_port;

    CTC_WB_QUERY_ENTRY_END((&wb_query));
done:
    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

   return ret;
}

int32 sys_usw_port_deinit(uint8 lchip)
{
    sys_port_master_t* master_ptr = NULL;
    LCHIP_CHECK(lchip);

    master_ptr = p_usw_port_master[lchip];

    if(!master_ptr)
    {
        return CTC_E_NONE;
    }

    sal_mutex_destroy(master_ptr->p_port_mutex);

    if(master_ptr->igs_port_prop)
    {
        mem_free(master_ptr->igs_port_prop);
    }
    if(master_ptr->egs_port_prop)
    {
        mem_free(master_ptr->egs_port_prop);
    }
    if(master_ptr->network_port)
    {
        mem_free(master_ptr->network_port);
    }

    /*opf deinit*/
    sys_usw_opf_deinit(lchip, master_ptr->opf_type_port_sflow);

    mem_free(master_ptr);
    p_usw_port_master[lchip] = NULL;

    return CTC_E_NONE;
}


