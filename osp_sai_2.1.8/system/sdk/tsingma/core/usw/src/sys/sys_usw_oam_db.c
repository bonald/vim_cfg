#if (FEATURE_MODE == 0)
#include "ctc_oam.h"
#include "ctc_error.h"
#include "ctc_common.h"
#include "ctc_linklist.h"
#include "ctc_hash.h"
#include "ctc_packet.h"
#include "ctc_l3if.h"
#include "ctc_register.h"

#include "sys_usw_oam.h"
#include "sys_usw_oam_db.h"
#include "sys_usw_oam_debug.h"
#include "sys_usw_oam_bfd_db.h"
#include "sys_usw_oam_com.h"
#include "sys_usw_oam_cfm_db.h"
#include "sys_usw_oam_tp_y1731_db.h"
#include "sys_usw_chip.h"
#include "sys_usw_opf.h"
#include "sys_usw_ftm.h"
#include "sys_usw_common.h"
#include "sys_usw_port.h"
#include "sys_usw_vlan.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_l3if.h"
#include "sys_usw_mpls.h"
#include "usw/include/drv_io.h"

#include "sys_usw_interrupt.h"
#include "sys_usw_wb_common.h"

#define CTC_OAM_Y1731_LMEP_UPDATE_TYPE_VLAN_PKT      25  /*TBD, later add into ctc_oam.h, ctc_oam_y1731_lmep_update_type_t*/

extern int32 sys_usw_global_ctl_get(uint8 lchip, ctc_global_control_type_t type, void* value);
/***************************************************************
*
*  Defines and Macros
*
***************************************************************/
struct sys_oam_id_hash_key_s
{
    uint32 oam_id;
    uint32 gport;
};
typedef struct sys_oam_id_hash_key_s sys_oam_id_hash_key_t;

struct sys_oam_maid_hash_key_s
{
    uint8   mep_type;
    uint8   resv0[3];

    uint8   maid[SYS_OAM_MAX_MAID_LEN];
};
typedef struct sys_oam_maid_hash_key_s sys_oam_maid_hash_key_t;

struct sys_eth_oam_chan_key_s
{
    uint16    vlan_id;
    uint16    gport;
    uint16    l2vpn_oam_id;
    uint8 resv0[2];
};
typedef struct sys_eth_oam_chan_key_s sys_eth_oam_chan_key_t;

struct sys_mpls_tp_chan_key_s
{
    uint32  label;
    uint16  gport_l3if_id;
    uint8   section_oam;
    uint8   spaceid;
};
typedef struct sys_mpls_tp_chan_key_s sys_mpls_tp_chan_key_t;


struct sys_bfd_chan_key_s
{
    uint32  my_discr;
    uint8   is_sbfd_reflector;
    uint8   rsv[3];
};
typedef struct sys_bfd_chan_key_s sys_bfd_chan_key_t;


struct sys_oam_chan_hash_key_s
{
    uint8   mep_type;
    union
    {
        sys_eth_oam_chan_key_t eth;    /** eth oam key*/
        sys_mpls_tp_chan_key_t mpls_tp;
        sys_bfd_chan_key_t     bfd;
    } u;
};
typedef struct sys_oam_chan_hash_key_s sys_oam_chan_hash_key_t;

struct sys_oam_defect_type_s
{
    uint8 entry_num;
    uint8 defect_type[2];
    uint8 defect_sub_type[2];
};
typedef struct sys_oam_defect_type_s sys_oam_defect_type_t;

struct sys_bfd_ip_v6_node_s
{
    uint32  count;
    ipv6_addr_t ipv6;
};
typedef struct sys_bfd_ip_v6_node_s sys_bfd_ip_v6_node_t;

#define ROT(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

#define MIX(a, b, c) \
    do \
    { \
        a -= c;  a ^= ROT(c, 4);  c += b; \
        b -= a;  b ^= ROT(a, 6);  a += c; \
        c -= b;  c ^= ROT(b, 8);  b += a; \
        a -= c;  a ^= ROT(c, 16);  c += b; \
        b -= a;  b ^= ROT(a, 19);  a += c; \
        c -= b;  c ^= ROT(b, 4);  b += a; \
    } while (0)

#define FINAL(a, b, c) \
    { \
        c ^= b; c -= ROT(b, 14); \
        a ^= c; a -= ROT(c, 11); \
        b ^= a; b -= ROT(a, 25); \
        c ^= b; c -= ROT(b, 16); \
        a ^= c; a -= ROT(c, 4);  \
        b ^= a; b -= ROT(a, 14); \
        c ^= b; c -= ROT(b, 24); \
    }

#define SYS_OAM_MAID_HASH_BLOCK_SIZE 64
#define SYS_OAM_CHAN_HASH_BLOCK_SIZE 64

extern sys_oam_master_t* g_oam_master[CTC_MAX_LOCAL_CHIP_NUM];

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/

#define HASH_KEY0 "Function Begin"

STATIC uint32
_sys_usw_oam_build_hash_key(void* p_data, uint8 length)
{
    uint32 a, b, c;
    uint32* k = (uint32*)p_data;
    uint8*  k8;

    /* Set up the internal state */
    a = b = c = 0xdeadbeef;

    /*------ all but last block: aligned reads and affect 32 bits of (a,b,c) */
    while (length > 12)
    {
        a += k[0];
        b += k[1];
        c += k[2];
        MIX(a, b, c);
        length -= 12;
        k += 3;
    }

    k8 = (uint8*)k;

    switch (length)
    {
    case 12:
        c += k[2];
        b += k[1];
        a += k[0];
        break;

    case 11:
        c += ((uint8)k8[10]) << 16;       /* fall through */

    case 10:
        c += ((uint8)k8[9]) << 8;         /* fall through */

    case 9:
        c += k8[8];                          /* fall through */

    case 8:
        b += k[1];
        a += k[0];
        break;

    case 7:
        b += ((uint8)k8[6]) << 16;        /* fall through */

    case 6:
        b += ((uint8)k8[5]) << 8;         /* fall through */

    case 5:
        b += k8[4];                          /* fall through */

    case 4:
        a += k[0];
        break;

    case 3:
        a += ((uint8)k8[2]) << 16;        /* fall through */

    case 2:
        a += ((uint8)k8[1]) << 8;         /* fall through */

    case 1:
        a += k8[0];
        break;

    case 0:
        return c;
    }

    FINAL(a, b, c);
    return c;
}

#define MISC "Function Begin"

int32
_sys_usw_oam_get_mep_entry_num(uint8 lchip)
{
    uint32 max_mep_index = 0;
    int32 ret = 0;

    ret = sys_usw_ftm_query_table_entry_num(lchip, DsEthMep_t,  &max_mep_index);
    if (ret)
    {
        max_mep_index = 0;
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"get max mep index fail!!\n");
    }
    return max_mep_index;
}

int32
_sys_usw_oam_get_mpls_entry_num(uint8 lchip)
{
    uint32 mpls_index = 0;
    int32 ret = 0;

    ret = sys_usw_ftm_query_table_entry_num(lchip, DsMpls_t,  &mpls_index);
    if (ret)
    {
        mpls_index = 0;
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"get mpls index fail!!\n");
    }

    return mpls_index;
}

uint8
_sys_usw_bfd_csf_convert(uint8 lchip, uint8 type, bool to_asic)
{
    uint8 csf_type = 0;

    if (to_asic)
    {
        if(3 == type)
        {
            csf_type = 0;
        }
        else if (0 == type)
        {
            csf_type = 7;
        }
        else
        {
            csf_type = type;
        }
    }
    else
    {
        if(0 == type)
        {
            csf_type = 3;
        }
        else if (7 == type)
        {
            csf_type = 0;
        }
        else
        {
            csf_type = type;
        }
    }

    return csf_type;
}

int32
_sys_usw_oam_get_nexthop_info(uint8 lchip, uint32 nhid, uint32 b_protection, sys_oam_nhop_info_t* p_nhop_info)
{
    sys_nh_info_dsnh_t nh_info;

    sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));


    CTC_PTR_VALID_CHECK(p_nhop_info);

    nh_info.protection_path = b_protection;
    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, nhid, &nh_info, 0));

    p_nhop_info->dest_map = nh_info.dest_map;
    p_nhop_info->nh_entry_type  = nh_info.nh_entry_type;
    p_nhop_info->nexthop_is_8w  = nh_info.nexthop_ext;
    p_nhop_info->dsnh_offset    = (nh_info.dsnh_offset & 0x1FFFF);  /*OAM must be egress edit mode, donot using bit17*/
    p_nhop_info->aps_bridge_en  = nh_info.oam_aps_en;
    p_nhop_info->have_l2edit  = nh_info.have_l2edit;

    if (p_nhop_info->aps_bridge_en)
    {
        p_nhop_info->mep_on_tunnel  = nh_info.mep_on_tunnel;
        p_nhop_info->dest_map = SYS_ENCODE_DESTMAP(SYS_DECODE_DESTMAP_GCHIP(nh_info.dest_map), nh_info.oam_aps_group_id);
    }

    return CTC_E_NONE;
}


STATIC int8
_sys_usw_oam_cal_bitmap_bit(uint8 lchip, uint32 bitmap)
{
    uint8 bit = 0;

    while (bitmap)
    {
        bit++;
        bitmap = (bitmap >> 1);
    }

    return (bit - 1);
}

int32
_sys_usw_oam_get_defect_type(uint8 lchip, uint32 defect, sys_oam_defect_type_t* defect_type)
{
    switch (defect)
    {
    case CTC_OAM_DEFECT_EVENT_RX_FIRST_PKT:
        defect_type->defect_type[0]     = 2;
        defect_type->defect_sub_type[0] = 4;
        defect_type->entry_num          = 1;
        break;

    case CTC_OAM_DEFECT_MAC_STATUS_CHANGE:
        defect_type->defect_type[0]     = 0;
        defect_type->defect_sub_type[0] = 1;
        defect_type->entry_num          = 1;
        break;

    case CTC_OAM_DEFECT_SRC_MAC_MISMATCH:
        defect_type->defect_type[0]     = 0;
        defect_type->defect_sub_type[0] = 7;
        defect_type->entry_num          = 1;
        break;

    case CTC_OAM_DEFECT_EVENT_RDI_RX:
        /*rx rdi*/
        defect_type->defect_type[0]     = 1;
        defect_type->defect_sub_type[0] = 0;
        /*rx rdi clear*/
        defect_type->defect_type[1]     = 1;
        defect_type->defect_sub_type[1] = 7;
        defect_type->entry_num          = 2;
        break;

    case CTC_OAM_DEFECT_MISMERGE:
        /*mismerge*/
        defect_type->defect_type[0]     = 5;
        defect_type->defect_sub_type[0] = 0;
        /*mismerge clear*/
        defect_type->defect_type[1]     = 5;
        defect_type->defect_sub_type[1] = 7;
        defect_type->entry_num          = 2;
        break;

    case CTC_OAM_DEFECT_UNEXPECTED_LEVEL:
        /*low level*/
        defect_type->defect_type[0]     = 5;
        defect_type->defect_sub_type[0] = 1;
        /*low level clear*/
        defect_type->defect_type[1]     = 5;
        defect_type->defect_sub_type[1] = 6;
        defect_type->entry_num          = 2;
        break;

    case CTC_OAM_DEFECT_UNEXPECTED_MEP:
        /*unexpected mep*/
        defect_type->defect_type[0]     = 4;
        defect_type->defect_sub_type[0] = 0;
        /*unexpected mep clear*/
        defect_type->defect_type[1]     = 4;
        defect_type->defect_sub_type[1] = 6;
        defect_type->entry_num          = 2;
        break;

    case CTC_OAM_DEFECT_UNEXPECTED_PERIOD:
        /*unexpected period*/
        defect_type->defect_type[0]     = 4;
        defect_type->defect_sub_type[0] = 1;
        /*unexpected period clear*/
        defect_type->defect_type[1]     = 4;
        defect_type->defect_sub_type[1] = 7;
        defect_type->entry_num          = 2;
        break;

    case CTC_OAM_DEFECT_DLOC:
        /* dloc*/
        defect_type->defect_type[0]     = 3;
        defect_type->defect_sub_type[0] = 0;
        /* dloc clear*/
        defect_type->defect_type[1]     = 3;
        defect_type->defect_sub_type[1] = 7;
        defect_type->entry_num          = 2;
        break;

    case CTC_OAM_DEFECT_CSF:
        /* dcsf*/
        defect_type->defect_type[0]     = 3;
        defect_type->defect_sub_type[0] = 1;
        /* dcsf clear*/
        defect_type->defect_type[1]     = 3;
        defect_type->defect_sub_type[1] = 2;
        defect_type->entry_num          = 2;
        break;

    case CTC_OAM_DEFECT_EVENT_BFD_INIT:
        defect_type->defect_type[0]     = 2;
        defect_type->defect_sub_type[0] = 1;
        defect_type->entry_num          = 1;
        break;

    case CTC_OAM_DEFECT_EVENT_BFD_UP:
        defect_type->defect_type[0]     = 2;
        defect_type->defect_sub_type[0] = 2;
        defect_type->entry_num          = 1;
        break;

    case CTC_OAM_DEFECT_EVENT_BFD_DOWN:
        defect_type->defect_type[0]     = 2;
        defect_type->defect_sub_type[0] = 3;
        defect_type->entry_num          = 1;
        break;

    case CTC_OAM_DEFECT_EVENT_BFD_MIS_CONNECT:
        defect_type->defect_type[0]     = 5;
        defect_type->defect_sub_type[0] = 0;
        defect_type->entry_num          = 1;
        break;

    case CTC_OAM_DEFECT_SF:
        /*SF defect*/
        defect_type->defect_type[0]     = 0;
        defect_type->defect_sub_type[0] = 2;

        /* SF clear*/
        defect_type->defect_type[1]     = 0;
        defect_type->defect_sub_type[1] = 3;
        defect_type->entry_num          = 2;
        break;
    case CTC_OAM_DEFECT_EVENT_BFD_TIMER_NEG_SUCCESS:
        defect_type->defect_type[0]     = 2;
        defect_type->defect_sub_type[0] = 5;
        defect_type->entry_num          = 1;
        break;
    default:
        defect_type->defect_type[0]     = 0;
        defect_type->defect_sub_type[0] = 0;
        defect_type->entry_num          = 0;
        break;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_oam_get_rdi_defect_type(uint8 lchip, uint32 defect, uint8* defect_type, uint8* defect_sub_type)
{
    uint8 type      = 0;
    uint8 sub_type  = 0;

    switch (defect)
    {
    case CTC_OAM_DEFECT_MAC_STATUS_CHANGE:
        type        = 0;
        sub_type    = 1;
        break;

    case CTC_OAM_DEFECT_SRC_MAC_MISMATCH:
        type        = 0;
        sub_type    = 7;
        break;

    case CTC_OAM_DEFECT_MISMERGE:
        type        = 5;
        sub_type    = 0;
        break;

    case CTC_OAM_DEFECT_UNEXPECTED_LEVEL:
        type        = 5;
        sub_type    = 1;
        break;

    case CTC_OAM_DEFECT_UNEXPECTED_MEP:
        type        = 4;
        sub_type    = 0;
        break;

    case CTC_OAM_DEFECT_UNEXPECTED_PERIOD:
        type        = 4;
        sub_type    = 1;
        break;

    case CTC_OAM_DEFECT_DLOC:
        type        = 3;
        sub_type    = 0;
        break;

    case CTC_OAM_DEFECT_CSF:
        type        = 3;
        sub_type    = 1;
        break;

    default:
        type        = 0;
        sub_type    = 0;
        break;
    }

    *defect_type        = type;
    *defect_sub_type    = sub_type;

    return CTC_E_NONE;
}

int32
sys_usw_oam_get_defect_type_config(uint8 lchip ,ctc_oam_defect_t defect, sys_oam_defect_info_t *p_defect)
{
    sys_oam_defect_type_t defect_type_event;
    uint8 defect_type_rdi       = 0;
    uint8 defect_sub_type_rdi   = 0;

    uint8 to_event  = 0;
    uint8 to_rdi    = 0;

    uint32 cmd = 0;
    uint32 rdi_index = 0;
    uint32 tmp[2] = {0};
    uint32 value = 0;
    uint32 rdi_mode = 0;
    uint32 local_sf_rdi = 0;
    uint32 remote_sf_rdi = 0;

    OamRxProcEtherCtl_m oam_rx_proc_ether_ctl;
    OamErrorDefectCtl_m  oam_error_defect_ctl;
    OamUpdateCtl_m       oam_update_ctl;

    sal_memset(&defect_type_event, 0, sizeof(sys_oam_defect_type_t));

    _sys_usw_oam_get_defect_type(lchip, defect, &defect_type_event);
    if(defect_type_event.entry_num)
    {
        sal_memset(&oam_error_defect_ctl, 0, sizeof(OamErrorDefectCtl_m));
        cmd = DRV_IOR(OamErrorDefectCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_error_defect_ctl));
        rdi_index = (defect_type_event.defect_type[0] << 3) | defect_type_event.defect_sub_type[0];
        GetOamErrorDefectCtl(A, reportDefectEn_f, &oam_error_defect_ctl, tmp);
        if (rdi_index < 32)
        {
           to_event = CTC_IS_BIT_SET(tmp[0], rdi_index);
        }
        else
        {
           to_event = CTC_IS_BIT_SET(tmp[1], (rdi_index - 32));
        }
    }

    _sys_usw_oam_get_rdi_defect_type(lchip, defect, &defect_type_rdi, &defect_sub_type_rdi);
    cmd = DRV_IOR(OamUpdateCtl_t, OamUpdateCtl_y1731RdiAutoIndicate_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rdi_mode));
    if (rdi_mode)   /*RDIMode 1*/
    {
        sal_memset(&oam_update_ctl, 0, sizeof(OamUpdateCtl_m));
        cmd = DRV_IOR(OamUpdateCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_update_ctl));
        local_sf_rdi = GetOamUpdateCtl(V, localSfFailtoRdi_f, &oam_update_ctl);
        remote_sf_rdi = GetOamUpdateCtl(V, remoteSfFailtoRdi_f, &oam_update_ctl);
        if (((CTC_OAM_DEFECT_MAC_STATUS_CHANGE == defect) && (CTC_IS_BIT_SET(remote_sf_rdi, 0) || CTC_IS_BIT_SET(remote_sf_rdi, 1)))
            || ((CTC_OAM_DEFECT_MISMERGE == defect) && CTC_IS_BIT_SET(local_sf_rdi, 1))
        || ((CTC_OAM_DEFECT_UNEXPECTED_LEVEL == defect) && CTC_IS_BIT_SET(local_sf_rdi, 0))
        || ((CTC_OAM_DEFECT_UNEXPECTED_MEP == defect) && CTC_IS_BIT_SET(local_sf_rdi, 2))
        || ((CTC_OAM_DEFECT_UNEXPECTED_PERIOD == defect) && CTC_IS_BIT_SET(remote_sf_rdi, 4))
        || ((CTC_OAM_DEFECT_DLOC == defect) && CTC_IS_BIT_SET(remote_sf_rdi, 3)))
        {
            to_rdi = 1;
        }
    }
    else if((0 != defect_type_rdi) || (0 != defect_sub_type_rdi))  /*RDIMode 0*/
    {
        sal_memset(&oam_rx_proc_ether_ctl, 0, sizeof(OamRxProcEtherCtl_m));
        cmd = DRV_IOR(OamRxProcEtherCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_rx_proc_ether_ctl));

        rdi_index = (defect_type_rdi << 3) | defect_sub_type_rdi;

        tmp[0] = GetOamRxProcEtherCtl(V, etherDefectToRdi0_f, &oam_rx_proc_ether_ctl);
        tmp[1] = GetOamRxProcEtherCtl(V, etherDefectToRdi1_f, &oam_rx_proc_ether_ctl);

        if (rdi_index < 32)
        {
            to_rdi = CTC_IS_BIT_SET(tmp[0], rdi_index);
        }
        else
        {
            to_rdi = CTC_IS_BIT_SET(tmp[1], (rdi_index - 32));
        }

        if (CTC_OAM_DEFECT_DLOC == defect)
        {
            cmd = DRV_IOR(OamUpdateCtl_t, OamUpdateCtl_genRdiByDloc_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            to_rdi = value;
        }
    }

    if (CTC_OAM_DEFECT_MAC_STATUS_CHANGE == defect)
    {
        p_defect->defect_name   = "Mac status";
    }
    else if (CTC_OAM_DEFECT_SRC_MAC_MISMATCH == defect)
    {
        p_defect->defect_name   = "Src Mac Mismatch";
    }
    else if(CTC_OAM_DEFECT_MISMERGE == defect)
    {
        p_defect->defect_name   = "MisMerge";
    }
    else if(CTC_OAM_DEFECT_UNEXPECTED_LEVEL == defect)
    {
        p_defect->defect_name   = "Unexpected level";
    }
    else if(CTC_OAM_DEFECT_UNEXPECTED_MEP == defect)
    {
        p_defect->defect_name = "Unexpected MEP";
    }
    else if(CTC_OAM_DEFECT_UNEXPECTED_PERIOD == defect)
    {
        p_defect->defect_name = "Unexpected period";
    }
    else if(CTC_OAM_DEFECT_DLOC == defect)
    {
        p_defect->defect_name = "Loc";
    }
    else if(CTC_OAM_DEFECT_CSF == defect)
    {
        p_defect->defect_name = "CSF";
    }
    else if(CTC_OAM_DEFECT_EVENT_RX_FIRST_PKT == defect)
    {
        p_defect->defect_name = "1st packet";
    }
    else if(CTC_OAM_DEFECT_EVENT_RDI_RX == defect)
    {
        p_defect->defect_name = "Rx RDI";
    }
    else if(CTC_OAM_DEFECT_EVENT_RDI_TX == defect)
    {
        p_defect->defect_name = "Tx RDI";
    }
    else if(CTC_OAM_DEFECT_EVENT_BFD_DOWN == defect)
    {
        p_defect->defect_name = "BFD Down";
    }
    else if(CTC_OAM_DEFECT_EVENT_BFD_INIT == defect)
    {
        p_defect->defect_name = "BFD Init";
    }
    else if(CTC_OAM_DEFECT_EVENT_BFD_UP == defect)
    {
        p_defect->defect_name = "BFD Up";
    }
    else if(CTC_OAM_DEFECT_EVENT_BFD_MIS_CONNECT == defect)
    {
        p_defect->defect_name = "BFD MisConnect";
    }
    p_defect->to_event      = to_event;
    p_defect->to_rdi        = to_rdi;

    return CTC_E_NONE;
}

#define OAMID "Function Begin"

sys_oam_id_t*
_sys_usw_oam_oamid_lkup(uint8 lchip, uint32 gport, uint32 oam_id)
{
    sys_oam_id_t* p_sys_oamid_db = NULL;
    sys_oam_id_t sys_oamid;
    sal_memset(&sys_oamid, 0 , sizeof(sys_oam_id_t));
    sys_oamid.oam_id= oam_id;
    sys_oamid.gport= gport;
    p_sys_oamid_db = ctc_hash_lookup(g_oam_master[lchip]->oamid_hash, &sys_oamid);
    return p_sys_oamid_db;
}

STATIC int32
_sys_usw_oam_oamid_cmp(sys_oam_id_t* p_sys_oamid,  sys_oam_id_t* p_sys_oamid_db)
{
    if ((NULL == p_sys_oamid) || (NULL == p_sys_oamid_db))
    {
        return 0;
    }
    if ((p_sys_oamid->oam_id == p_sys_oamid_db->oam_id) && (p_sys_oamid->gport == p_sys_oamid_db->gport))
    {
        return 1;
    }
    return 0;
}

STATIC uint32
_sys_usw_oam_oamid_build_key(sys_oam_id_t* p_sys_oamid)
{
    sys_oam_id_hash_key_t oamid_hash_key;
    sal_memset(&oamid_hash_key, 0, sizeof(sys_oam_id_hash_key_t));
    oamid_hash_key.oam_id = p_sys_oamid->oam_id;
    oamid_hash_key.gport = p_sys_oamid->gport;
    return _sys_usw_oam_build_hash_key(&oamid_hash_key, sizeof(sys_oam_id_hash_key_t));
}

int32
_sys_usw_oam_oamid_add_to_db(uint8 lchip, sys_oam_id_t* p_sys_oamid)
{
    if (NULL == ctc_hash_insert(g_oam_master[lchip]->oamid_hash, p_sys_oamid))
    {
        return CTC_E_NO_MEMORY;
    }
    return CTC_E_NONE;
}

int32
_sys_usw_oam_oamid_del_from_db(uint8 lchip, sys_oam_id_t* p_sys_oamid)
{
    if (NULL == ctc_hash_remove(g_oam_master[lchip]->oamid_hash, p_sys_oamid))
    {
        return CTC_E_NOT_EXIST;
    }
    return CTC_E_NONE;
}

int32
sys_usw_oam_add_oamid (uint8 lchip, uint32 gport, uint16 oamid, uint32 label, uint8 spaceid)
{
    sys_oam_id_t* p_sys_oamid_db = NULL;
    int32 ret = CTC_E_NONE;

    if (!DRV_IS_DUET2(lchip))
    {
        return CTC_E_NONE;
    }
    OAM_LOCK(lchip);
    p_sys_oamid_db = _sys_usw_oam_oamid_lkup(lchip, gport, oamid);
    if (p_sys_oamid_db)
    {
        if (((p_sys_oamid_db->label[0] == label) && (p_sys_oamid_db->space_id[0] == spaceid))
            || ((p_sys_oamid_db->label[1] == label) && (p_sys_oamid_db->space_id[1] == spaceid)))
        {
            OAM_UNLOCK(lchip);
            return CTC_E_NONE;
        }
        else if (p_sys_oamid_db->label[0] && (p_sys_oamid_db->label[1]))
        {
            OAM_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        p_sys_oamid_db = (sys_oam_id_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_id_t));
        if (NULL == p_sys_oamid_db)
        {
            OAM_UNLOCK(lchip);
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_sys_oamid_db, 0 , sizeof(sys_oam_id_t));
        p_sys_oamid_db->gport = gport;
        p_sys_oamid_db->oam_id = oamid;
        ret = _sys_usw_oam_oamid_add_to_db(lchip, p_sys_oamid_db);
        if (ret)
        {
            mem_free(p_sys_oamid_db);
            OAM_UNLOCK(lchip);
            return ret;
        }
    }
    if (!p_sys_oamid_db->label[0])
    {
        p_sys_oamid_db->label[0] = label;
        p_sys_oamid_db->space_id[0] = spaceid;
    }
    else
    {
        p_sys_oamid_db->label[1] = label;
        p_sys_oamid_db->space_id[1] = spaceid;
    }
    OAM_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_oam_remove_oamid(uint8 lchip, uint32 gport, uint16 oamid, uint32 label, uint8 spaceid)
{
    sys_oam_id_t* p_sys_oamid_db = NULL;
    int32 ret = CTC_E_NONE;

    if (!DRV_IS_DUET2(lchip))
    {
        return CTC_E_NONE;
    }
    OAM_LOCK(lchip);
    p_sys_oamid_db = _sys_usw_oam_oamid_lkup(lchip, gport, oamid);
    if (NULL == p_sys_oamid_db)
    {
        OAM_UNLOCK(lchip);
        return CTC_E_NONE;
    }
    if ((p_sys_oamid_db->label[0] == label) && (p_sys_oamid_db->space_id[0] == spaceid))
    {
        p_sys_oamid_db->label[0] = 0;
        p_sys_oamid_db->space_id[0] = 0;
    }
    else if ((p_sys_oamid_db->label[1] == label) && (p_sys_oamid_db->space_id[1] == spaceid))
    {
        p_sys_oamid_db->label[1] = 0;
        p_sys_oamid_db->space_id[1] = 0;
    }

    if ((0 == p_sys_oamid_db->label[0]) && (0 == p_sys_oamid_db->label[1]))
    {
        ret = _sys_usw_oam_oamid_del_from_db(lchip, p_sys_oamid_db);
        if (ret)
        {
            OAM_UNLOCK(lchip);
            return ret;
        }
    }
    OAM_UNLOCK(lchip);
    return CTC_E_NONE;
}

#define MAID "Function Begin"

sys_oam_maid_com_t*
_sys_usw_oam_maid_lkup(uint8 lchip, ctc_oam_maid_t* p_ctc_maid)
{
    ctc_hash_t* p_maid_hash = NULL;
    sys_oam_maid_com_t* p_sys_maid_db = NULL;
    sys_oam_maid_com_t sys_maid;

    sal_memset(&sys_maid, 0 , sizeof(sys_oam_maid_com_t));
    sys_maid.mep_type = p_ctc_maid->mep_type;
    sys_maid.maid_len = p_ctc_maid->maid_len;
    sal_memcpy(&sys_maid.maid, p_ctc_maid->maid, sys_maid.maid_len);

    p_maid_hash = g_oam_master[lchip]->maid_hash;

    p_sys_maid_db = ctc_hash_lookup(p_maid_hash, &sys_maid);

    return p_sys_maid_db;
}

STATIC int32
_sys_usw_oam_maid_cmp(sys_oam_maid_com_t* p_sys_maid,  sys_oam_maid_com_t* p_sys_maid_db)
{
    uint8 maid_len = 0;

    if ((NULL == p_sys_maid) || (NULL == p_sys_maid_db))
    {
        return 0;
    }

    maid_len = p_sys_maid->maid_len;

    if (p_sys_maid->maid_len != p_sys_maid_db->maid_len)
    {
        return 0;
    }

    if (sal_memcmp(&p_sys_maid->maid, &p_sys_maid_db->maid, maid_len))
    {
        return 0;
    }

    return 1;
}

STATIC uint32
_sys_usw_oam_maid_build_key(sys_oam_maid_com_t* p_sys_maid)
{

    sys_oam_maid_hash_key_t maid_hash_key;
    uint8   length = 0;

    sal_memset(&maid_hash_key, 0, sizeof(sys_oam_maid_hash_key_t));

    maid_hash_key.mep_type = p_sys_maid->mep_type;
    sal_memcpy(&maid_hash_key.maid, &p_sys_maid->maid, sizeof(p_sys_maid->maid));
    length = sizeof(maid_hash_key);

    return _sys_usw_oam_build_hash_key(&maid_hash_key, length);
}

sys_oam_maid_com_t*
_sys_usw_oam_maid_build_node(uint8 lchip, ctc_oam_maid_t* p_maid_param)
{
    sys_oam_maid_com_t* p_sys_maid = NULL;
    uint8 maid_entry_num = 0;
    uint8 maid_len = 0;
    uint8 mep_type = CTC_OAM_MEP_TYPE_MAX;

    mep_type = p_maid_param->mep_type;

    p_sys_maid = (sys_oam_maid_com_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_maid_com_t));
    if (NULL == p_sys_maid)
    {
        return NULL;
    }
    maid_len = p_maid_param->maid_len;

    sal_memset(p_sys_maid, 0, sizeof(sys_oam_maid_com_t));
    p_sys_maid->mep_type    = p_maid_param->mep_type;
    p_sys_maid->maid_len    = maid_len;
    sal_memcpy(p_sys_maid->maid, p_maid_param->maid, maid_len);

    if (((CTC_OAM_MEP_TYPE_ETH_1AG == mep_type)
        || (CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_type)) && (0 != g_oam_master[lchip]->maid_len_format))
    {
        switch (g_oam_master[lchip]->maid_len_format)
        {
            case CTC_OAM_MAID_LEN_16BYTES:
                maid_entry_num = 2;
                break;

            case CTC_OAM_MAID_LEN_32BYTES:
                maid_entry_num = 4;
                break;

            case CTC_OAM_MAID_LEN_48BYTES:
                maid_entry_num = 6;
                break;

            default:
                break;
        }
    }
    else
    {
        if (maid_len <= 16)
        {
            maid_entry_num = 2;
        }
        else if (maid_len <= 32)
        {
            maid_entry_num = 4;
        }
        else if (maid_len <= 48)
        {
            maid_entry_num = 6;
        }
    }
    p_sys_maid->maid_entry_num = maid_entry_num;


    return p_sys_maid;
}

int32
_sys_usw_oam_maid_free_node(uint8 lchip, sys_oam_maid_com_t* p_sys_maid)
{
    mem_free(p_sys_maid);
    p_sys_maid = NULL;

    return CTC_E_NONE;
}


int32
_sys_usw_oam_maid_build_index(uint8 lchip, sys_oam_maid_com_t* p_sys_maid)
{
    sys_usw_opf_t opf;
    uint8  maid_entry_num = 0;
    uint32 offset         = 0;

    if (g_oam_master[lchip]->no_mep_resource)
    {
        return CTC_E_NONE;
    }

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = g_oam_master[lchip]->oam_opf_type;
    opf.pool_index = SYS_OAM_OPF_TYPE_MA_NAME;

    maid_entry_num = p_sys_maid->maid_entry_num;
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, maid_entry_num, &offset));
    SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MANAME, 1, maid_entry_num);
    p_sys_maid->maid_index = offset;
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"MAID: lchip->%d, index->%d\n", lchip, offset);

    return CTC_E_NONE;
}

int32
_sys_usw_oam_maid_free_index(uint8 lchip, sys_oam_maid_com_t* p_sys_maid)
{
    sys_usw_opf_t opf;
    uint8  maid_entry_num = 0;
    uint32 offset         = 0;

    if (g_oam_master[lchip]->no_mep_resource)
    {
        return CTC_E_NONE;
    }

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_type  = g_oam_master[lchip]->oam_opf_type;
    opf.pool_index = SYS_OAM_OPF_TYPE_MA_NAME;

    maid_entry_num = p_sys_maid->maid_entry_num;
    offset = p_sys_maid->maid_index;
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, maid_entry_num, offset));
    SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MANAME, 0, maid_entry_num);

    return CTC_E_NONE;
}

int32
_sys_usw_oam_maid_add_to_db(uint8 lchip, sys_oam_maid_com_t* p_sys_maid)
{
    if (NULL == ctc_hash_insert(g_oam_master[lchip]->maid_hash, p_sys_maid))
    {
        return CTC_E_NO_MEMORY;
    }
    return CTC_E_NONE;
}

int32
_sys_usw_oam_maid_del_from_db(uint8 lchip, sys_oam_maid_com_t* p_sys_maid)
{
    if (NULL == ctc_hash_remove(g_oam_master[lchip]->maid_hash, p_sys_maid))
    {
        return CTC_E_NOT_EXIST;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_oam_maid_add_to_asic(uint8 lchip, sys_oam_maid_com_t* p_sys_maid)
{
    DsMaName_m ds_ma_name;
    uint32 cmd            = 0;
    uint32 ma_index       = 0;
    uint8  entry_index    = 0;
    uint8  maid_entry_num = 0;
    uint32 maid_field[2] = {0};

    maid_entry_num  = p_sys_maid->maid_entry_num;
    ma_index        = p_sys_maid->maid_index;

    sal_memset(&ds_ma_name, 0, sizeof(DsMaName_m));
    cmd = DRV_IOW(DsMaName_t, DRV_ENTRY_FLAG);

    for (entry_index = 0; entry_index < maid_entry_num; entry_index++)
    {
        maid_field[1] = p_sys_maid->maid[entry_index * 8 + 0] << 24 |
                        p_sys_maid->maid[entry_index * 8 + 1] << 16 |
                        p_sys_maid->maid[entry_index * 8 + 2] << 8 |
                        p_sys_maid->maid[entry_index * 8 + 3] << 0;

        maid_field[0] = p_sys_maid->maid[entry_index * 8 + 4] << 24 |
                        p_sys_maid->maid[entry_index * 8 + 5] << 16 |
                        p_sys_maid->maid[entry_index * 8 + 6] << 8 |
                        p_sys_maid->maid[entry_index * 8 + 7] << 0;

        DRV_SET_FIELD_A(lchip, DsMaName_t,DsMaName_maIdUmc_f,&ds_ma_name,maid_field);

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"-----ds_ma_name(maIdx:%d, entryIdx:%d)--------\n", ma_index, entry_index);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"ds_ma_name.ma_id_umc0:0x%x\n", maid_field[0]);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"ds_ma_name.ma_id_umc1:0x%x\n", maid_field[1]);

        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ma_index + entry_index, cmd, &ds_ma_name));

    }

    sys_usw_ma_clear_write_cache(lchip);
    return CTC_E_NONE;
}

int32
_sys_usw_oam_maid_del_from_asic(uint8 lchip, sys_oam_maid_com_t* p_sys_maid)
{
    DsMaName_m ds_ma_name;
    uint32 cmd              = 0;
    uint32 ma_index         = 0;
    uint8  entry_index      = 0;
    uint8  maid_entry_num   = 0;

    maid_entry_num  = p_sys_maid->maid_entry_num;
    ma_index        = p_sys_maid->maid_index;

    sal_memset(&ds_ma_name, 0, sizeof(DsMaName_m));
    cmd = DRV_IOW(DsMaName_t, DRV_ENTRY_FLAG);

    for (entry_index = 0; entry_index < maid_entry_num; entry_index++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ma_index + entry_index, cmd, &ds_ma_name));
    }

    sys_usw_ma_clear_write_cache(lchip);

    return CTC_E_NONE;
}


int32
_sys_usw_oam_maid_update(uint8 lchip, ctc_oam_update_t* p_update, sys_oam_lmep_t* p_sys_lmep)
{
    sys_oam_maid_com_t* p_sys_maid = NULL;
    uint32 ret = CTC_E_NONE;
    ctc_oam_maid_t* p_maid = (ctc_oam_maid_t*)(p_update->p_update_value);

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    p_sys_maid = _sys_usw_oam_maid_lkup(lchip, p_maid);
    if (NULL == p_sys_maid)
    {
        return CTC_E_NOT_EXIST;
    }
    p_sys_lmep->p_sys_maid->ref_cnt--;    /*old associated maid ref_cnt--*/
    p_sys_maid->ref_cnt++;                /*new associated maid ref_cnt++*/
    p_sys_lmep->p_sys_maid = p_sys_maid;

    return ret;
}


#define CHAN "Function Begin"

sys_oam_chan_com_t*
_sys_usw_oam_chan_lkup(uint8 lchip, sys_oam_chan_com_t* p_sys_chan)
{
    ctc_hash_t* p_chan_hash = NULL;
    sys_oam_chan_com_t* p_sys_chan_db = NULL;

    p_chan_hash = g_oam_master[lchip]->chan_hash;

    p_sys_chan_db = ctc_hash_lookup(p_chan_hash, p_sys_chan);

    return p_sys_chan_db;
}

int32
_sys_usw_oam_chan_cmp(sys_oam_chan_com_t* p_sys_chan, sys_oam_chan_com_t* p_sys_chan_db)
{
    uint8 oam_type = 0;
    uint32  ret = CTC_E_NONE;
    sys_oam_chan_bfd_t* p_chan_bfd_para = (sys_oam_chan_bfd_t*)p_sys_chan;
    sys_oam_chan_bfd_t* p_chan_bfd_db   = (sys_oam_chan_bfd_t*)p_sys_chan_db;
    sys_oam_chan_tp_t* p_chan_tp_para = (sys_oam_chan_tp_t*)p_sys_chan;
    sys_oam_chan_tp_t* p_chan_tp_db   = (sys_oam_chan_tp_t*)p_sys_chan_db;
    sys_oam_chan_eth_t* p_chan_eth_para = (sys_oam_chan_eth_t*)p_sys_chan;
    sys_oam_chan_eth_t* p_chan_eth_db   = (sys_oam_chan_eth_t*)p_sys_chan_db;
    oam_type = p_sys_chan->mep_type;
    switch(oam_type)
    {
    case CTC_OAM_MEP_TYPE_IP_BFD:
    case CTC_OAM_MEP_TYPE_MPLS_BFD:
    case CTC_OAM_MEP_TYPE_MICRO_BFD:
        if ((p_chan_bfd_para->key.com.mep_type == p_chan_bfd_db->key.com.mep_type)
            && (p_chan_bfd_para->key.my_discr == p_chan_bfd_db->key.my_discr)
            && (p_chan_bfd_para->key.is_sbfd_reflector == p_chan_bfd_db->key.is_sbfd_reflector))
        {
            ret = 1;
        }
        break;
    case CTC_OAM_MEP_TYPE_MPLS_TP_BFD:
    case CTC_OAM_MEP_TYPE_MPLS_TP_Y1731:
        if((p_chan_tp_para->key.com.mep_type == p_chan_tp_db->key.com.mep_type)
            && (p_chan_tp_para->key.section_oam == p_chan_tp_db->key.section_oam)
            && (p_chan_tp_para->key.label == p_chan_tp_db->key.label)
            && (p_chan_tp_para->key.spaceid == p_chan_tp_db->key.spaceid)
            && (p_chan_tp_para->key.gport_l3if_id == p_chan_tp_db->key.gport_l3if_id))
        {
            ret = 1;
        }
        break;
    case CTC_OAM_MEP_TYPE_ETH_1AG:
    case CTC_OAM_MEP_TYPE_ETH_Y1731:
        if((p_chan_eth_para->key.com.mep_type == p_chan_eth_db->key.com.mep_type)
            && (p_chan_eth_para->key.use_fid == p_chan_eth_db->key.use_fid)
            && (p_chan_eth_para->key.link_oam == p_chan_eth_db->key.link_oam)
            && (p_chan_eth_para->key.gport == p_chan_eth_db->key.gport)
            && (p_chan_eth_para->key.vlan_id == p_chan_eth_db->key.vlan_id)
            && (p_chan_eth_para->key.cvlan_id == p_chan_eth_db->key.cvlan_id)
            && (p_chan_eth_para->key.is_cvlan== p_chan_eth_db->key.is_cvlan)
            && (p_chan_eth_para->key.l2vpn_oam_id == p_chan_eth_db->key.l2vpn_oam_id))
        {
            ret = 1;
        }
        break;

    default:
        break;
    }
    return ret;
}

STATIC uint32
_sys_usw_oam_chan_build_key(sys_oam_chan_com_t* p_sys_chan)
{

    sys_oam_chan_hash_key_t chan_hash_key;
    uint8   length = 0;

    sal_memset(&chan_hash_key, 0, sizeof(sys_oam_chan_hash_key_t));

    chan_hash_key.mep_type = p_sys_chan->mep_type;

    if ((CTC_OAM_MEP_TYPE_ETH_1AG == chan_hash_key.mep_type)
        || (CTC_OAM_MEP_TYPE_ETH_Y1731 == chan_hash_key.mep_type))
    {
        chan_hash_key.u.eth.gport   = ((sys_oam_chan_eth_t*)p_sys_chan)->key.gport;
        chan_hash_key.u.eth.vlan_id = ((sys_oam_chan_eth_t*)p_sys_chan)->key.vlan_id;
        chan_hash_key.u.eth.l2vpn_oam_id = ((sys_oam_chan_eth_t*)p_sys_chan)->key.l2vpn_oam_id;
    }
    else if ((CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == chan_hash_key.mep_type)
             || (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == chan_hash_key.mep_type))
    {
        chan_hash_key.u.mpls_tp.gport_l3if_id   = ((sys_oam_chan_tp_t*)p_sys_chan)->key.gport_l3if_id;
        chan_hash_key.u.mpls_tp.section_oam     = ((sys_oam_chan_tp_t*)p_sys_chan)->key.section_oam;
        chan_hash_key.u.mpls_tp.label           = ((sys_oam_chan_tp_t*)p_sys_chan)->key.label;
        chan_hash_key.u.mpls_tp.spaceid         = ((sys_oam_chan_tp_t*)p_sys_chan)->key.spaceid;
    }
    else if ((CTC_OAM_MEP_TYPE_IP_BFD == chan_hash_key.mep_type)
             || (CTC_OAM_MEP_TYPE_MICRO_BFD == chan_hash_key.mep_type)
             || (CTC_OAM_MEP_TYPE_MPLS_BFD == chan_hash_key.mep_type))
    {
        chan_hash_key.u.bfd.my_discr   = ((sys_oam_chan_bfd_t*)p_sys_chan)->key.my_discr;
        chan_hash_key.u.bfd.is_sbfd_reflector = ((sys_oam_chan_bfd_t*)p_sys_chan)->key.is_sbfd_reflector;
    }

    length = sizeof(sys_oam_chan_hash_key_t);

    return _sys_usw_oam_build_hash_key(&chan_hash_key, length);

}

int32
_sys_usw_oam_lm_build_index(uint8 lchip, sys_oam_chan_com_t* p_sys_chan, uint8 is_link_sec, uint8  md_level)
{
    sys_usw_opf_t opf;
    uint32 offset           = 0;
    uint8  mep_type         = 0;
    uint8  lm_cos_type      = 0;
    uint16* lm_index_base   = 0;
    uint32  block_size      = 0;
    uint8   cnt             = 0;
    uint32  ret = CTC_E_NONE;
    uint32  cmd = 0;
    sys_oam_chan_eth_t* p_eth_chan = NULL;
    sys_oam_chan_tp_t* p_tp_chan  = NULL;
    sys_oam_lmep_t* p_sys_lmep_com = NULL;
    sys_oam_lmep_t sys_oam_lmep_eth;
    DsOamLmStats_m ds_oam_lm_stats;
    uint8 need_alloc_lm = 1;
    uint8 is_ether_service = 0;

    mep_type = p_sys_chan->mep_type;

    if ((CTC_OAM_MEP_TYPE_ETH_1AG == mep_type) || (CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_type))
    {
        p_eth_chan = (sys_oam_chan_eth_t*)p_sys_chan;

        sal_memset(&sys_oam_lmep_eth, 0, sizeof(sys_oam_lmep_t));
        sys_oam_lmep_eth.md_level = md_level;
        p_sys_lmep_com = _sys_usw_oam_lmep_lkup(lchip, p_sys_chan, &sys_oam_lmep_eth);
        if (NULL == p_sys_lmep_com)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Local mep not found \n");
            return CTC_E_NOT_EXIST;
        }
        if (is_link_sec)
        {
            lm_cos_type     = p_eth_chan->link_lm_cos_type;
            lm_index_base   = &p_sys_chan->link_lm_index_base;
        }
        else
        {
            if (p_sys_chan->lm_num >= 3)
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " LM entry full in EVC\n");
                return CTC_E_INVALID_CONFIG;
            }
            lm_cos_type     = p_sys_lmep_com->lm_cos_type;
            lm_index_base   = &p_sys_lmep_com->lm_index_base;
            is_ether_service = 1;
        }
    }
    else if ((CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == mep_type) || (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type))
    {
        p_tp_chan = (sys_oam_chan_tp_t*)p_sys_chan;

        lm_cos_type     = p_tp_chan->lm_cos_type;
        lm_index_base   = &p_sys_chan->lm_index_base;
    }
    block_size = (CTC_OAM_LM_COS_TYPE_PER_COS == lm_cos_type) ? 8 : 1;
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    if (is_ether_service)
    {
        if (!p_eth_chan->lm_index_alloced)
        {
            opf.pool_index  = SYS_OAM_OPF_TYPE_LM_PROFILE;
            opf.pool_type = g_oam_master[lchip]->oam_opf_type;

            CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, 1, &offset));
            p_sys_chan->lm_index_base = offset;
            p_eth_chan->lm_index_alloced = 1;
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"LM profile: DsEthLmProfile, index->%d\n", offset);
        }

        if (p_sys_lmep_com->lm_index_alloced)
        {
            need_alloc_lm = 0;
        }
    }

    if (need_alloc_lm)
    {
        opf.pool_index  = SYS_OAM_OPF_TYPE_LM;
        opf.pool_type = g_oam_master[lchip]->oam_opf_type;

        ret = sys_usw_opf_alloc_offset(lchip, &opf, block_size, &offset);
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LM, 1, block_size);
        if (CTC_E_NONE != ret)
        {
            if (is_ether_service && (0 == p_sys_chan->lm_num ) && p_eth_chan->lm_index_alloced)
            {
                opf.pool_index  = SYS_OAM_OPF_TYPE_LM_PROFILE;
                opf.pool_type = g_oam_master[lchip]->oam_opf_type;
                sys_usw_opf_free_offset(lchip, &opf, 1, p_sys_chan->lm_index_base );
                p_sys_chan->lm_index_base = 0;
                p_eth_chan->lm_index_alloced = 0;
            }
        }
        else
        {
            *lm_index_base = offset;
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"LM: lchip->%d, index->%d, block_size->%d\n", lchip, offset, block_size);
            for (cnt = 0; cnt < block_size; cnt++)
            {
                sal_memset(&ds_oam_lm_stats, 0, sizeof(DsOamLmStats_m));
                cmd = DRV_IOW(DsOamLmStats_t, DRV_ENTRY_FLAG);
                DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, (offset + cnt), cmd, &ds_oam_lm_stats));
            }
            if (is_ether_service)
            {
                p_sys_lmep_com->lm_index_alloced = 1;
                p_sys_chan->lm_num++;
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"LM number: %d\n", p_sys_chan->lm_num);
            }
        }
    }

    return ret;
}

int32
_sys_usw_oam_lm_free_index(uint8 lchip, sys_oam_chan_com_t* p_sys_chan, uint8 is_link_sec, uint8  md_level)
{
    sys_usw_opf_t opf;
    uint16 temp             = 0;
    uint16 *offset          = &temp;
    uint8  mep_type         = 0;
    uint8  lm_cos_type      = 0;
    uint32  block_size      = 0;
    uint8 need_free_lm_prof = 0;
    uint8 need_free_lm = 1;
    sys_oam_chan_eth_t* p_eth_chan = NULL;
    sys_oam_chan_tp_t* p_tp_chan  = NULL;
    sys_oam_lmep_t* p_sys_lmep_com = NULL;
    sys_oam_lmep_t sys_oam_lmep_eth;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    mep_type = p_sys_chan->mep_type;

    if ((CTC_OAM_MEP_TYPE_ETH_1AG == mep_type) || (CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_type))
    {
        p_eth_chan = (sys_oam_chan_eth_t*)p_sys_chan;
        sal_memset(&sys_oam_lmep_eth, 0, sizeof(sys_oam_lmep_t));
        sys_oam_lmep_eth.md_level = md_level;
        p_sys_lmep_com = _sys_usw_oam_lmep_lkup(lchip, p_sys_chan, &sys_oam_lmep_eth);
        if (NULL == p_sys_lmep_com)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Local mep not found \n");
            return CTC_E_NOT_EXIST;
        }
        if (is_link_sec)
        {
            lm_cos_type     = p_eth_chan->link_lm_cos_type;
            offset          = &p_sys_chan->link_lm_index_base;
        }
        else
        {
            lm_cos_type     = p_sys_lmep_com->lm_cos_type;
            offset          = &p_sys_lmep_com->lm_index_base;
            if (p_sys_chan->lm_num > 0)
            {
                p_sys_chan->lm_num --;
            }
            if (p_eth_chan->lm_index_alloced)
            {
                need_free_lm_prof = 1;
            }
            if (!p_sys_lmep_com->lm_index_alloced)
            {
                need_free_lm = 0;
            }
            else
            {
                p_sys_lmep_com->lm_index_alloced = 0;
            }
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"LM number: %d\n", p_sys_chan->lm_num);
        }
    }
    else if ((CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == mep_type) || (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type))
    {
        p_tp_chan = (sys_oam_chan_tp_t*)p_sys_chan;

        lm_cos_type     = p_tp_chan->lm_cos_type;
        offset          = &p_sys_chan->lm_index_base;
    }
    block_size = (CTC_OAM_LM_COS_TYPE_PER_COS == lm_cos_type) ? 8 : 1;
    if (need_free_lm)
    {
        opf.pool_type  = g_oam_master[lchip]->oam_opf_type ;
        opf.pool_index = SYS_OAM_OPF_TYPE_LM;
        CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, block_size, (*offset)));
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LM, 0, block_size);
        *offset = 0;
    }

    if (need_free_lm_prof && (0 == p_sys_chan->lm_num )) /*free DsEthLmProfile*/
    {
        opf.pool_index  = SYS_OAM_OPF_TYPE_LM_PROFILE;
        opf.pool_type = g_oam_master[lchip]->oam_opf_type ;
        CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, 1, p_sys_chan->lm_index_base));
        p_sys_chan->lm_index_base = 0;
        p_eth_chan->lm_index_alloced = 0;
    }

    return CTC_E_NONE;
}


int32
_sys_usw_oam_chan_add_to_db(uint8 lchip, sys_oam_chan_com_t* p_sys_chan)
{
    if (NULL == ctc_hash_insert(g_oam_master[lchip]->chan_hash, p_sys_chan))
    {
        return CTC_E_NO_MEMORY;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_oam_chan_del_from_db(uint8 lchip, sys_oam_chan_com_t* p_sys_chan)
{
    if (NULL == ctc_hash_remove(g_oam_master[lchip]->chan_hash, p_sys_chan))
    {
        return CTC_E_NOT_EXIST;
    }

    return CTC_E_NONE;
}

#define LMEP "Function Begin"

sys_oam_lmep_t*
_sys_usw_oam_lmep_lkup(uint8 lchip, sys_oam_chan_com_t* p_sys_chan, sys_oam_lmep_t* p_sys_lmep)
{
    sys_oam_lmep_t* p_sys_lmep_db = NULL;
    ctc_slistnode_t* ctc_slistnode = NULL;
    ctc_slist_t* p_lmep_list = NULL;
    uint8 oam_type = 0;
    uint8 cmp_result = 0;

    oam_type = p_sys_chan->mep_type;

    p_lmep_list = p_sys_chan->lmep_list;

    CTC_SLIST_LOOP(p_lmep_list, ctc_slistnode)
    {
        p_sys_lmep_db = _ctc_container_of(ctc_slistnode, sys_oam_lmep_t, head);
        if (NULL == p_sys_lmep_db)
        {
            continue;
        }
        switch(oam_type)
        {
        case CTC_OAM_MEP_TYPE_IP_BFD:
        case CTC_OAM_MEP_TYPE_MPLS_BFD:
        case CTC_OAM_MEP_TYPE_MICRO_BFD:
        case CTC_OAM_MEP_TYPE_MPLS_TP_BFD:
        case CTC_OAM_MEP_TYPE_MPLS_TP_Y1731:
            if (NULL == p_sys_lmep)
            {
                cmp_result = TRUE;
            }
            break;
        case CTC_OAM_MEP_TYPE_ETH_1AG:
        case CTC_OAM_MEP_TYPE_ETH_Y1731:
            if (p_sys_lmep && (((sys_oam_lmep_t*)p_sys_lmep)->md_level == ((sys_oam_lmep_t*)p_sys_lmep_db)->md_level))
            {
                cmp_result = TRUE;
            }
            break;

        default:
            break;
        }
        if (TRUE == cmp_result)
        {
            return p_sys_lmep_db;
        }
    }

    return NULL;
}

int32
_sys_usw_oam_lmep_build_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep)
{
    sys_usw_opf_t opf;
    uint32 offset       = 0;
    uint32 block_size   = 0;
    int32 ret           = CTC_E_NONE;


    if (p_sys_lmep->mep_on_cpu)
    {
        p_sys_lmep->lmep_index = 0x1FFF;
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"LMEP: lchip->%d, index->%d\n", lchip, offset);
        return CTC_E_NONE;
    }


    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    if (g_oam_master[lchip]->mep_index_alloc_by_sdk)
    {
        if (g_oam_master[lchip]->no_mep_resource)
        {
            return CTC_E_NONE;
        }

        if (FALSE == sys_usw_chip_is_local(lchip, p_sys_lmep->p_sys_chan->master_chipid))
        {
            return CTC_E_NONE;
        }

        opf.pool_type  = g_oam_master[lchip]->oam_opf_type;
        opf.pool_index = SYS_OAM_OPF_TYPE_MA;
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, 1, &offset));
        p_sys_lmep->ma_index = offset;

        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MA, 1, 1);

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"MA  : lchip->%d, index->%d\n", lchip, offset);

        block_size = 2;
        opf.pool_index  = SYS_OAM_OPF_TYPE_MEP_LMEP;

        if ((((CTC_OAM_MEP_TYPE_ETH_1AG == p_sys_lmep->p_sys_chan->mep_type)
            || (CTC_OAM_MEP_TYPE_ETH_Y1731 == p_sys_lmep->p_sys_chan->mep_type))
            && (!CTC_FLAG_ISSET(((sys_oam_lmep_t*)p_sys_lmep)->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE)
            || CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_Y1731_LMEP_FLAG_TWAMP_EN))))
        {
            block_size = 1;
        }

        ret = sys_usw_opf_alloc_offset(lchip, &opf, block_size, &offset);
        if(CTC_E_NONE != ret)
        {
            opf.pool_index  = SYS_OAM_OPF_TYPE_MA;
            offset         = p_sys_lmep->ma_index;
            p_sys_lmep->ma_index = 0;
            sys_usw_opf_free_offset(lchip, &opf, 1, offset);
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MA, 0, 1);
            return ret;
        }
        p_sys_lmep->lmep_index = offset;
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MEP, 1, block_size);

    }
    else
    {
        if (g_oam_master[lchip]->no_mep_resource)
        {
            p_sys_lmep->ma_index = 0;
            return CTC_E_NONE;
        }

        opf.pool_type  = g_oam_master[lchip]->oam_opf_type;
        opf.pool_index = SYS_OAM_OPF_TYPE_MA;
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, 1, &offset));
        p_sys_lmep->ma_index = offset;
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MA, 1, 1);
        offset = p_sys_lmep->lmep_index;
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MEP, 1, 1);
    }

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"LMEP: lchip->%d, index->%d\n", lchip, offset);

    return ret;
}

int32
_sys_usw_oam_lmep_free_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep)
{
    sys_usw_opf_t opf;
    uint32 offset       = 0;
    uint32 block_size   = 0;

    if (p_sys_lmep->mep_on_cpu)
    {
        p_sys_lmep->lmep_index = 0;
        return CTC_E_NONE;
    }

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    opf.pool_type = g_oam_master[lchip]->oam_opf_type;
    if (g_oam_master[lchip]->mep_index_alloc_by_sdk)
    {
        if (g_oam_master[lchip]->no_mep_resource)
        {
            return CTC_E_INVALID_CONFIG;
        }

        if (FALSE == sys_usw_chip_is_local(lchip, p_sys_lmep->p_sys_chan->master_chipid))
        {
            return CTC_E_NONE;
        }

        opf.pool_index = SYS_OAM_OPF_TYPE_MA;
        offset = p_sys_lmep->ma_index;
        CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, 1, offset));

        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MA, 0, 1);

        offset      = p_sys_lmep->lmep_index;
        block_size  = 2;
        opf.pool_index  = SYS_OAM_OPF_TYPE_MEP_LMEP;

        if ((((CTC_OAM_MEP_TYPE_ETH_1AG == p_sys_lmep->p_sys_chan->mep_type)
            || (CTC_OAM_MEP_TYPE_ETH_Y1731 == p_sys_lmep->p_sys_chan->mep_type))
        && (!CTC_FLAG_ISSET(((sys_oam_lmep_t*)p_sys_lmep)->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE)
        || CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_Y1731_LMEP_FLAG_TWAMP_EN))))
        {
            block_size = 1;
        }

        CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, block_size, offset));
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MEP, 0, block_size);
    }
    else
    {
        if (g_oam_master[lchip]->no_mep_resource)
        {
            return CTC_E_NONE;
        }
        opf.pool_index = SYS_OAM_OPF_TYPE_MA;
        offset = p_sys_lmep->ma_index;
        CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, 1, offset));
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MA, 0, 1);
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MEP, 0, 1);
    }

    return CTC_E_NONE;

}

int32
_sys_usw_oam_lmep_add_to_db(uint8 lchip, sys_oam_lmep_t* p_sys_lmep)
{
    uint32 mep_index    = 0;
    sys_oam_chan_com_t* p_sys_chan = NULL;

    mep_index = p_sys_lmep->lmep_index;
    p_sys_chan = p_sys_lmep->p_sys_chan;

    if (!g_oam_master[lchip]->mep_index_alloc_by_sdk)
    {
        if (ctc_vector_get(g_oam_master[lchip]->mep_vec, mep_index))
        {
            return CTC_E_EXIST;  /*need check when index alloced by system*/
        }
    }

    if (NULL == p_sys_chan->lmep_list)
    {
        p_sys_chan->lmep_list = ctc_slist_new();
        if (NULL == p_sys_chan->lmep_list)
        {
            return CTC_E_NO_MEMORY;
        }
    }

    ctc_slist_add_tail(p_sys_chan->lmep_list, &(p_sys_lmep->head));

    if (!p_sys_lmep->mep_on_cpu)
    {
        if (FALSE == ctc_vector_add(g_oam_master[lchip]->mep_vec, mep_index, p_sys_lmep))
        {
            return CTC_E_NO_MEMORY;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_oam_lmep_del_from_db(uint8 lchip, sys_oam_lmep_t* p_sys_lmep)
{
    uint32 mep_index = 0;
    sys_oam_chan_com_t* p_sys_chan = NULL;

    mep_index = p_sys_lmep->lmep_index;
    p_sys_chan = p_sys_lmep->p_sys_chan;

    if (NULL == p_sys_chan->lmep_list)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Local mep not found \n");
        return CTC_E_NOT_EXIST;
    }

    ctc_slist_delete_node(p_sys_chan->lmep_list, &(p_sys_lmep->head));

    if (0 == CTC_SLISTCOUNT(p_sys_chan->lmep_list))
    {
        ctc_slist_free(p_sys_chan->lmep_list);
        p_sys_chan->lmep_list = NULL;
    }


    if (!p_sys_lmep->mep_on_cpu)
    {
        ctc_vector_del(g_oam_master[lchip]->mep_vec, mep_index);
    }

    return CTC_E_NONE;
}

int32
_sys_usw_oam_lmep_add_eth_to_asic(uint8 lchip, ctc_oam_y1731_lmep_t* p_eth_lmep, sys_oam_lmep_t* p_sys_lmep)
{
    sys_oam_chan_eth_t* p_sys_chan_eth = NULL;
    sys_oam_lmep_t* p_sys_eth_lmep = NULL;
    sys_oam_maid_com_t* p_sys_eth_maid = NULL;
    uint32 cmd        = 0;
    uint32 index      = 0;
    uint32 lmep_flag  = 0;
    uint32 gport      = 0;
    uint8  dest_chip  = 0;
    uint16  dest_id  = 0;
    uint32 destmap = 0;
    uint8 maid_length_type = 0;
    uint16 src_vlan_ptr    = 0;
    DsMa_m ds_ma;
    DsEthMep_m ds_eth_mep;
    DsEthMep_m DsEthMep_mask;
    tbl_entry_t tbl_entry;
    uint32 nexthop_ptr_bypass = 0;
    uint8 is_double_vlan = 0;
    uint16 vlan_id = 0;
    uint16 pro_idx = 0;
    uint8  tpid_type = 0;
    p_sys_eth_lmep = p_sys_lmep;
    p_sys_chan_eth = (sys_oam_chan_eth_t*)p_sys_lmep->p_sys_chan;
    p_sys_eth_maid = p_sys_lmep->p_sys_maid;

    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
    {
        goto RETURN;
    }

    lmep_flag = p_sys_eth_lmep->flag;
    gport = p_sys_chan_eth->key.gport;

    if (0 == g_oam_master[lchip]->maid_len_format)
    {
        maid_length_type = (p_sys_eth_maid->maid_entry_num >> 1);
    }
    else
    {
        maid_length_type = g_oam_master[lchip]->maid_len_format;
    }

    dest_chip = CTC_MAP_GPORT_TO_GCHIP(gport);
    dest_id =    SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);
    destmap = SYS_ENCODE_DESTMAP( dest_chip, dest_id );

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        pro_idx = CTC_MAP_GPORT_TO_LPORT(gport) + 64;
    }
    else
    {
        pro_idx = dest_id;
    }

    sal_memset(&ds_ma, 0, sizeof(DsMa_m));
    sal_memset(&ds_eth_mep, 0, sizeof(DsEthMep_m));
    sal_memset(&DsEthMep_mask, 0, sizeof(DsEthMep_m));

    /* set ma table entry */
    if (((!p_sys_chan_eth->key.vlan_id) && (!p_sys_chan_eth->key.cvlan_id)) || p_sys_chan_eth->key.link_oam)
    {
        SetDsMa(V, txUntaggedOam_f      , &ds_ma ,  1);
    }
    else
    {
        SetDsMa(V, txUntaggedOam_f      , &ds_ma ,  0);
    }

    if ((0 != p_sys_chan_eth->key.vlan_id) && (0 != p_sys_chan_eth->key.cvlan_id))
    {    /*Double vlan mode*/
        SetDsMa(V, ipv6Hdr_f      , &ds_ma ,  1);
    }
    else
    {
        SetDsMa(V, ipv6Hdr_f            , &ds_ma , 0);
    }



    SetDsMa(V, outerVlanIsCVlan_f, &ds_ma , p_sys_eth_lmep->vlan_domain);

    SetDsMa(V, maIdLengthType_f     , &ds_ma , maid_length_type);

    SetDsMa(V, mplsLabelValid_f     , &ds_ma , 0);
    SetDsMa(V, rxOamType_f          , &ds_ma , 1);

    SetDsMa(V, txWithPortStatus_f   , &ds_ma , CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_TX_PORT_STATUS));
    SetDsMa(V, txWithSendId_f       , &ds_ma , CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_TX_SEND_ID));
    SetDsMa(V, mplsTtl_f            , &ds_ma , 0);
    SetDsMa(V, maNameIndex_f        , &ds_ma ,  p_sys_eth_maid->maid_index);
    SetDsMa(V, priorityIndex_f      , &ds_ma , p_eth_lmep->tx_cos_exp);
/*TBD, Confirm for spec v2.2.0*/
#if 0
    SetDsMa(V, sfFailWhileCfgType_f , &ds_ma , 0);
    SetDsMa(V, apsEn_f              , &ds_ma , CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_APS_EN));
#endif
    SetDsMa(V, maNameLen_f          , &ds_ma ,  p_sys_eth_maid->maid_len);
    SetDsMa(V, mdLvl_f              , &ds_ma ,  p_sys_eth_lmep->md_level);
    SetDsMa(V, txWithIfStatus_f     , &ds_ma , CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_TX_IF_STATUS));

    SetDsMa(V, tunnelApsEn_f        , &ds_ma , 0);

     /*-SetDsMa(V, sfState_f            , &ds_ma , 0); //TBD, Confirm for spec v2.2.0*/
    SetDsMa(V, csfInterval_f        , &ds_ma , 0);
    SetDsMa(V, protectingPath_f     , &ds_ma , 0);
    SetDsMa(V, csfTimeIdx_f         , &ds_ma , 0);
    SetDsMa(V, packetType_f         , &ds_ma , 0);
    SetDsMa(V, nextHopExt_f         , &ds_ma , 0);

    if (p_sys_chan_eth->key.link_oam)
    {
        uint32 default_vlan;
        CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_DEFAULT_VLAN, &default_vlan));
        CTC_ERROR_RETURN(sys_usw_vlan_get_vlan_ptr(lchip, default_vlan, &src_vlan_ptr));
        SetDsMa(V, linkoam_f            , &ds_ma , 1);
    }
    else
    {
        src_vlan_ptr = p_sys_chan_eth->key.vlan_id;
    }

    if (p_sys_eth_lmep->is_up) /*up mep*/
    {
        if (p_sys_chan_eth->key.use_fid)
        {
            SetDsMa(V, vrfid_f        , &ds_ma ,  p_sys_chan_eth->key.vlan_id); /*fid*/
        }
        else
        {
            SetDsMa(V, srcVlanPtr_f   , &ds_ma , 0);  /*vlan ptr*/
        }
    }
    else /*down mep*/
    {
        SetDsMa(V, srcVlanPtr_f   , &ds_ma , src_vlan_ptr); /*vlan ptr*/
        if (p_sys_eth_lmep->vlan_domain)
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_BYPASS_NH, &nexthop_ptr_bypass));
            SetDsMa(V, nextHopPtr_f   , &ds_ma , ((1 <<17) | nexthop_ptr_bypass));
        }
    }

    /*Set DsEthMep*/
    if ((p_sys_chan_eth->key.vlan_id) && (p_sys_chan_eth->key.cvlan_id))
    {
        is_double_vlan = 1;
    }
    else
    {
        vlan_id = p_sys_chan_eth->key.vlan_id?p_sys_chan_eth->key.vlan_id:p_sys_chan_eth->key.cvlan_id;
    }

    SetDsEthMep(V, destMap_f               , &ds_eth_mep , destmap);

    SetDsEthMep(V, ccmInterval_f, &ds_eth_mep ,  p_eth_lmep->ccm_interval);
    SetDsMa(V, csfInterval_f, &ds_ma , p_eth_lmep->ccm_interval);

    SetDsEthMep(V, active_f                , &ds_eth_mep , CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_EN));
    SetDsEthMep(V, isUp_f                  , &ds_eth_mep , p_sys_eth_lmep->is_up);
    SetDsEthMep(V, ethOamP2PMode_f         , &ds_eth_mep , CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE));
    SetDsEthMep(V, maIdCheckDisable_f      , &ds_eth_mep , 0);
    SetDsEthMep(V, cciEn_f                 , &ds_eth_mep , 0);
    SetDsEthMep(V, enablePm_f              , &ds_eth_mep , CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_DM_EN));
    SetDsEthMep(V, seqNumEn_f              , &ds_eth_mep , CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_CCM_SEQ_NUM_EN));
    SetDsEthMep(V, shareMacEn_f            , &ds_eth_mep , CTC_FLAG_ISSET(p_eth_lmep->flag, CTC_OAM_Y1731_LMEP_FLAG_SHARE_MAC));

    SetDsEthMep(V, mepId_f, &ds_eth_mep, p_eth_lmep->mep_id);

    switch (p_eth_lmep->tpid_index)
    {
        case CTC_PARSER_L2_TPID_SVLAN_TPID_0:
            tpid_type = 0;
            break;

        case CTC_PARSER_L2_TPID_SVLAN_TPID_1:
            tpid_type = 1;
            break;

        case CTC_PARSER_L2_TPID_SVLAN_TPID_2:
            tpid_type = 2;
            break;

        case CTC_PARSER_L2_TPID_SVLAN_TPID_3:
            tpid_type = 3;
            break;

        default:
            break;
    }
    SetDsEthMep(V, tpidType_f              , &ds_eth_mep , tpid_type);

    SetDsEthMep(V, mepPrimaryVid_f         , &ds_eth_mep , is_double_vlan?p_sys_chan_eth->key.vlan_id:vlan_id);
    SetDsEthMep(V, cvlanTag_f        , &ds_eth_mep , is_double_vlan?p_sys_chan_eth->key.cvlan_id:0);
    SetDsEthMep(V, maIndex_f               , &ds_eth_mep , p_sys_lmep->ma_index);

    SetDsEthMep(V, isRemote_f              , &ds_eth_mep , 0);
    SetDsEthMep(V, cciWhile_f              , &ds_eth_mep , 4);
    SetDsEthMep(V, dUnexpMepTimer_f        , &ds_eth_mep , 14);
    SetDsEthMep(V, dMismergeTimer_f        , &ds_eth_mep , 14);
    SetDsEthMep(V, dMegLvlTimer_f          , &ds_eth_mep , 14);
    SetDsEthMep(V, isBfd_f                 , &ds_eth_mep , 0);

    SetDsEthMep(V, mepType_f               , &ds_eth_mep , 0);
    SetDsEthMep(V, learnEn_f               , &ds_eth_mep , 1);
    SetDsEthMep(V, apsSignalFailLocal_f    , &ds_eth_mep , 0);

    SetDsEthMep(V, autoGenEn_f             , &ds_eth_mep , 0);
    SetDsEthMep(V, portStatus_f            , &ds_eth_mep , 0);
    SetDsEthMep(V, rmepLastRdi_f           , &ds_eth_mep , 0);
    SetDsEthMep(V, presentRdi_f            , &ds_eth_mep , 0);
    SetDsEthMep(V, dUnexpMep_f             , &ds_eth_mep , 0);
    SetDsEthMep(V, dMismerge_f             , &ds_eth_mep , 0);
    SetDsEthMep(V, dMegLvl_f               , &ds_eth_mep , 0);
    SetDsEthMep(V, portId_f                , &ds_eth_mep , pro_idx);
    SetDsEthMep(V, p2pUseUcDa_f            , &ds_eth_mep , 0);
    SetDsEthMep(V, autoGenPktPtr_f         , &ds_eth_mep , 0);
    SetDsEthMep(V, ccmSeqNum_f             , &ds_eth_mep , 0);
    SetDsEthMep(V, apsBridgeEn_f           , &ds_eth_mep , 0);
    if (CTC_FLAG_ISSET(p_eth_lmep->flag, CTC_OAM_Y1731_LMEP_FLAG_TWAMP_EN))
    {
        sys_oam_nhop_info_t  mpls_nhop_info;
        sal_memset(&mpls_nhop_info, 0, sizeof(sys_oam_nhop_info_t));
        if (p_eth_lmep->nhid)
        {
            CTC_ERROR_RETURN(_sys_usw_oam_get_nexthop_info(lchip, p_eth_lmep->nhid, CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_PROTECTION_PATH), &mpls_nhop_info));
            SetDsMa(V, nextHopExt_f, &ds_ma, mpls_nhop_info.nexthop_is_8w);
            SetDsMa(V, nextHopPtr_f, &ds_ma, mpls_nhop_info.dsnh_offset);
            SetDsEthMep(V, destMap_f, &ds_eth_mep, mpls_nhop_info.dest_map);
        }
        SetDsEthMep(V, mepType_f, &ds_eth_mep , 6);
        SetDsEthMep(V, learnEn_f, &ds_eth_mep , 0);
        if (!CTC_FLAG_ISSET(p_eth_lmep->flag, CTC_OAM_Y1731_LMEP_FLAG_TWAMP_REFLECT_EN))
        {
            SetDsEthMep(V, autoGenEn_f, &ds_eth_mep , 1);
            SetDsEthMep(V, autoGenPktPtr_f, &ds_eth_mep , 7);
        }
    }
    index = p_sys_lmep->ma_index;
    CTC_ERROR_RETURN(sys_usw_ma_add_to_asic(lchip, index, &ds_ma));

    index = p_sys_lmep->lmep_index;
    cmd = DRV_IOW(DsEthMep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_eth_mep;
    tbl_entry.mask_entry = (uint32*)&DsEthMep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));


RETURN:

    return CTC_E_NONE;

}

int32
_sys_usw_oam_lmep_add_tp_y1731_to_asic(uint8 lchip, ctc_oam_y1731_lmep_t* p_tp_y1731_lmep, sys_oam_lmep_t* p_sys_lmep)
{
    sys_oam_chan_tp_t* p_sys_chan_tp = NULL;
    sys_oam_lmep_t* p_sys_tp_y1731_lmep = NULL;
    sys_oam_maid_com_t* p_sys_tp_y1731_maid = NULL;
    uint32 cmd        = 0;
    uint32 index      = 0;
    uint32 lmep_flag  = 0;
    uint8 maid_length_type = 0;
    DsMa_m ds_ma;
    DsEthMep_m ds_eth_mep;
    DsEthMep_m DsEthMep_mask;
    tbl_entry_t tbl_entry;
    uint32 destmap = 0;

    p_sys_tp_y1731_lmep = (sys_oam_lmep_t*)p_sys_lmep;
    p_sys_chan_tp       = (sys_oam_chan_tp_t*)p_sys_lmep->p_sys_chan;
    p_sys_tp_y1731_maid = (sys_oam_maid_com_t*)p_sys_lmep->p_sys_maid;

    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
    {
        goto RETURN;
    }

    lmep_flag = p_sys_tp_y1731_lmep->flag;
    maid_length_type = (p_sys_tp_y1731_maid->maid_entry_num >> 1);

    sal_memset(&ds_ma, 0, sizeof(DsMa_m));
    sal_memset(&ds_eth_mep, 0, sizeof(DsEthMep_m));
    sal_memset(&DsEthMep_mask, 0, sizeof(DsEthMep_m));

    /* set ma table entry */
    SetDsMa(V, txWithPortStatus_f   , &ds_ma , 0);
    SetDsMa(V, txWithSendId_f   , &ds_ma , 0);
    SetDsMa(V, txWithIfStatus_f   , &ds_ma , 0);
     /*-SetDsMa(V, useVrfidLkup_f   , &ds_ma , 0);  //TBD, Confirm for spec v2.2.0*/
    SetDsMa(V, txUntaggedOam_f   , &ds_ma , CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_WITHOUT_GAL));


    SetDsMa(V, maIdLengthType_f   , &ds_ma , maid_length_type);

    SetDsEthMep(V, ccmInterval_f   , &ds_eth_mep , p_tp_y1731_lmep->ccm_interval);
    SetDsMa(V, csfInterval_f   , &ds_ma , p_tp_y1731_lmep->ccm_interval);

    SetDsMa(V, rxOamType_f          , &ds_ma , 8);

    SetDsMa(V, mplsTtl_f          , &ds_ma , p_tp_y1731_lmep->mpls_ttl);
    SetDsMa(V, maNameIndex_f          , &ds_ma , p_sys_tp_y1731_maid->maid_index);
    SetDsMa(V, priorityIndex_f          , &ds_ma , p_tp_y1731_lmep->tx_cos_exp);
    SetDsMa(V, maNameLen_f          , &ds_ma , p_sys_tp_y1731_maid->maid_len);
    SetDsMa(V, mdLvl_f              , &ds_ma ,  7);

    if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_WITHOUT_GAL))
    {
        SetDsMa(V, packetType_f     , &ds_ma ,  0);
    }
    else
    {
        SetDsMa(V, packetType_f     , &ds_ma ,  2);
    }

    if (p_sys_chan_tp->key.section_oam)
    {
        SetDsMa(V, linkoam_f            , &ds_ma , 1);
        SetDsMa(V, mplsLabelValid_f     , &ds_ma , 0);
    }
    else
    {
        SetDsMa(V, linkoam_f            , &ds_ma , 0);
        SetDsMa(V, mplsLabelValid_f     , &ds_ma , 0xF);
    }

    if (CTC_NH_RESERVED_NHID_FOR_DROP == p_sys_tp_y1731_lmep->nhid)
    {
        uint8  gchip_id = 0;
        SetDsMa(V, nextHopExt_f         , &ds_ma , 0);
        SetDsMa(V, nextHopPtr_f         , &ds_ma , 0);
        sys_usw_get_gchip_id(lchip, &gchip_id);
        destmap = SYS_ENCODE_DESTMAP( gchip_id, SYS_RSV_PORT_DROP_ID);
        SetDsEthMep(V, destMap_f           , &ds_eth_mep , destmap );
        SetDsEthMep(V, apsBridgeEn_f           , &ds_eth_mep , 0);
    }
    else
    {
        sys_oam_nhop_info_t  mpls_nhop_info;
        sal_memset(&mpls_nhop_info, 0, sizeof(sys_oam_nhop_info_t));
        CTC_ERROR_RETURN(_sys_usw_oam_get_nexthop_info(lchip, p_sys_tp_y1731_lmep->nhid, CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_PROTECTION_PATH), &mpls_nhop_info));
        SetDsMa(V, nextHopExt_f         , &ds_ma , mpls_nhop_info.nexthop_is_8w);
        SetDsMa(V, nextHopPtr_f         , &ds_ma , mpls_nhop_info.dsnh_offset);
        SetDsEthMep(V, destMap_f           , &ds_eth_mep , mpls_nhop_info.dest_map);
        SetDsEthMep(V, apsBridgeEn_f       , &ds_eth_mep , mpls_nhop_info.aps_bridge_en);
        SetDsMa(V, tunnelApsEn_f,           &ds_ma , mpls_nhop_info.mep_on_tunnel);
        SetDsMa(V, protectingPath_f,        &ds_ma , CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_PROTECTION_PATH));
    }

    /* set mep table entry */
    SetDsEthMep(V, active_f                , &ds_eth_mep , CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_EN));
    SetDsEthMep(V, isUp_f                  , &ds_eth_mep , 0);
    SetDsEthMep(V, ethOamP2PMode_f         , &ds_eth_mep , 1);
    SetDsEthMep(V, maIdCheckDisable_f      , &ds_eth_mep , 0);
    SetDsEthMep(V, cciEn_f                 , &ds_eth_mep , 0);
    SetDsEthMep(V, enablePm_f              , &ds_eth_mep , CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_DM_EN));

    /*wangt SetDsEthMep(V, presentTrafficCheckEn_f , &ds_eth_mep , 0);*/
    SetDsEthMep(V, seqNumEn_f              , &ds_eth_mep , 0);
    SetDsEthMep(V, shareMacEn_f            , &ds_eth_mep , 1);

    SetDsEthMep(V, mepId_f, &ds_eth_mep , p_tp_y1731_lmep->mep_id);
    SetDsEthMep(V, tpidType_f              , &ds_eth_mep , 0);
    SetDsEthMep(V, mepPrimaryVid_f         , &ds_eth_mep ,  0);
    SetDsEthMep(V, maIndex_f               , &ds_eth_mep , p_sys_lmep->ma_index);

    SetDsEthMep(V, isRemote_f              , &ds_eth_mep , 0);
    SetDsEthMep(V, cciWhile_f              , &ds_eth_mep , 4);
    SetDsEthMep(V, dUnexpMepTimer_f        , &ds_eth_mep , 14);
    SetDsEthMep(V, dMismergeTimer_f        , &ds_eth_mep , 14);
    SetDsEthMep(V, dMegLvlTimer_f          , &ds_eth_mep , 14);
    SetDsEthMep(V, isBfd_f                 , &ds_eth_mep , 0);

    SetDsEthMep(V, mepType_f               , &ds_eth_mep , 6);
    /*wangt SetDsEthMep(V, sfFailWhile_f           , &ds_eth_mep , 0);*/
    SetDsEthMep(V, learnEn_f               , &ds_eth_mep , 0);
    SetDsEthMep(V, apsSignalFailLocal_f    , &ds_eth_mep , 0);

    SetDsEthMep(V, autoGenEn_f             , &ds_eth_mep , 0);
    /*wangt SetDsEthMep(V, presentTraffic_f        , &ds_eth_mep , 0);*/
    SetDsEthMep(V, portStatus_f            , &ds_eth_mep , 0);
    SetDsEthMep(V, rmepLastRdi_f           , &ds_eth_mep , 0);
    SetDsEthMep(V, presentRdi_f            , &ds_eth_mep , 0);
    SetDsEthMep(V, dUnexpMep_f             , &ds_eth_mep , 0);
    SetDsEthMep(V, dMismerge_f             , &ds_eth_mep , 0);
    SetDsEthMep(V, dMegLvl_f               , &ds_eth_mep , 0);
    SetDsEthMep(V, portId_f                , &ds_eth_mep , 0);
    SetDsEthMep(V, p2pUseUcDa_f            , &ds_eth_mep , 0);
    SetDsEthMep(V, autoGenPktPtr_f         , &ds_eth_mep , 0);
    SetDsEthMep(V, ccmSeqNum_f             , &ds_eth_mep , 0);


    index = p_sys_lmep->ma_index;
    CTC_ERROR_RETURN(sys_usw_ma_add_to_asic(lchip, index, &ds_ma));

    index = p_sys_lmep->lmep_index;
    cmd = DRV_IOW(DsEthMep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_eth_mep;
    tbl_entry.mask_entry = (uint32*)&DsEthMep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));

RETURN:

    return CTC_E_NONE;

}


int32
_sys_usw_oam_lmep_del_from_asic(uint8 lchip, sys_oam_lmep_t* p_sys_lmep)
{
    uint32 cmd          = 0;
    uint32 index        = 0;
    DsMa_m ds_ma;
    DsEthMep_m ds_eth_mep;
    DsEthMep_m DsEthMep_mask;
    tbl_entry_t tbl_entry;

    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
    {
        goto RETURN;
    }

    sal_memset(&ds_ma, 0, sizeof(DsMa_m));
    sal_memset(&ds_eth_mep, 0, sizeof(ds_eth_mep));
    sal_memset(&DsEthMep_mask, 0, sizeof(ds_eth_mep));

    index = p_sys_lmep->ma_index;
    CTC_ERROR_RETURN(sys_usw_ma_add_to_asic(lchip, index, &ds_ma));

    index = p_sys_lmep->lmep_index;
    cmd = DRV_IOW(DsEthMep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_eth_mep;
    tbl_entry.mask_entry = (uint32*)&DsEthMep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));

RETURN:

    return CTC_E_NONE;
}



int32
_sys_usw_oam_lmep_check_rmep_dloc(uint8 lchip, sys_oam_lmep_t* p_sys_lmep)
{
    ctc_slistnode_t* ctc_slistnode = NULL;
    ctc_slist_t* p_rmep_list = NULL;
    sys_oam_rmep_t* p_sys_rmep = NULL;
    uint32 cmd = 0;
    uint32 r_index = 0;
    uint32 l_index = 0;
    OamRxProcEtherCtl_m rx_ether_ctl;
    uint32 dloc_rdi = 0;
    uint8 idx = 0;
    uint8 defect_type       = 0;
    uint8 defect_sub_type   = 0;
    uint8 rdi_index         = 0;
    uint32 defect = 0;
    uint8 defect_check[CTC_USW_OAM_DEFECT_NUM] = {0};
    DsEthMep_m    ds_eth_mep;
    DsEthRmep_m   ds_eth_rmep;
    uint32 d_mismerge = 0;
    uint32 d_meg_lvl = 0;
    uint32 d_unexp_mep = 0;
    uint32 d_loc = 0;
    uint32 d_unexp_period = 0;
    uint32 mac_status_change = 0;
    uint32 rmepmacmismatch = 0;
    uint32 ether_defect_to_rdi = 0;

    p_rmep_list = p_sys_lmep->rmep_list;
    l_index = p_sys_lmep->lmep_index;

    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
    {
        goto RETURN;
    }

    /*1.Get defect to RDI bitmap*/
    cmd = DRV_IOR(OamUpdateCtl_t, OamUpdateCtl_genRdiByDloc_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dloc_rdi));

    sal_memset(&rx_ether_ctl,   0, sizeof(OamRxProcEtherCtl_m));
    cmd = DRV_IOR(OamRxProcEtherCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_ether_ctl));

    /*2.According bitmap to get defect type*/
    for (idx = 0; idx < CTC_USW_OAM_DEFECT_NUM; idx++)
    {
        defect = (1 << idx);
        _sys_usw_oam_get_rdi_defect_type(lchip, (1 << idx), &defect_type, &defect_sub_type);

        switch (defect)
        {
        case CTC_OAM_DEFECT_DLOC:
            defect_check[idx] = dloc_rdi;
            break;

        case CTC_OAM_DEFECT_MISMERGE:
        case CTC_OAM_DEFECT_UNEXPECTED_LEVEL:
        case CTC_OAM_DEFECT_UNEXPECTED_MEP:
        case CTC_OAM_DEFECT_UNEXPECTED_PERIOD:
        case CTC_OAM_DEFECT_MAC_STATUS_CHANGE:
        case CTC_OAM_DEFECT_SRC_MAC_MISMATCH:
            {
                rdi_index = (defect_type << 3) | defect_sub_type;
                if (rdi_index < 32)
                {
                    ether_defect_to_rdi = GetOamRxProcEtherCtl(V, etherDefectToRdi0_f, &rx_ether_ctl);
                    defect_check[idx] = CTC_IS_BIT_SET(ether_defect_to_rdi,  rdi_index);
                }
                else
                {
                    ether_defect_to_rdi = GetOamRxProcEtherCtl(V, etherDefectToRdi1_f, &rx_ether_ctl);
                    defect_check[idx] = CTC_IS_BIT_SET(ether_defect_to_rdi, (rdi_index - 32));
                }
            }
            break;

        default:
            defect_check[idx] = 0;
            break;
        }
    }

    /*3. check local MEP*/
    sal_memset(&ds_eth_mep,     0, sizeof(DsEthMep_m));
    cmd = DRV_IOR(DsEthMep_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, l_index, cmd, &ds_eth_mep));

    DRV_GET_FIELD_A(lchip, DsEthMep_t, DsEthMep_dMismergeTimer_f , &ds_eth_mep, &d_mismerge);
    GetDsEthMep(A, dMegLvl_f, &ds_eth_mep, &d_meg_lvl);
    DRV_GET_FIELD_A(lchip, DsEthMep_t, DsEthMep_dUnexpMep_f , &ds_eth_mep, &d_unexp_mep);

    if ((d_mismerge & defect_check[_sys_usw_oam_cal_bitmap_bit(lchip, CTC_OAM_DEFECT_MISMERGE)])
        || (d_meg_lvl & defect_check[_sys_usw_oam_cal_bitmap_bit(lchip, CTC_OAM_DEFECT_UNEXPECTED_LEVEL)])
        || (d_unexp_mep & defect_check[_sys_usw_oam_cal_bitmap_bit(lchip, CTC_OAM_DEFECT_UNEXPECTED_MEP)]))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Check Lmep[%d]:  d_mismerge %d,  d_meg_lvl % d ,d_unexp_mep % d\n",
                         l_index, d_mismerge, d_meg_lvl, d_unexp_mep );
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Some rmep still has dloc defect \n");
        return CTC_E_INVALID_CONFIG;

    }

    /*4. check remote MEP */
    cmd = DRV_IOR(DsEthRmep_t, DRV_ENTRY_FLAG);
    CTC_SLIST_LOOP(p_rmep_list, ctc_slistnode)
    {
        p_sys_rmep = _ctc_container_of(ctc_slistnode, sys_oam_rmep_t, head);
        if (NULL != p_sys_rmep)
        {
            r_index = p_sys_rmep->rmep_index;
            sal_memset(&ds_eth_rmep, 0, sizeof(DsEthRmep_m));
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, r_index, cmd, &ds_eth_rmep));
            DRV_GET_FIELD_A(lchip, DsEthRmep_t, DsEthRmep_dLoc_f , &ds_eth_rmep, &d_loc);
            GetDsEthRmep(A, dUnexpPeriod_f, &ds_eth_rmep, &d_unexp_period);
            GetDsEthRmep(A, macStatusChange_f, &ds_eth_rmep, &mac_status_change);
            GetDsEthRmep(A, rmepmacmismatch_f,  &ds_eth_rmep, &rmepmacmismatch);

            if ((d_loc & defect_check[_sys_usw_oam_cal_bitmap_bit(lchip, CTC_OAM_DEFECT_DLOC)])
                || (d_unexp_period & defect_check[_sys_usw_oam_cal_bitmap_bit(lchip, CTC_OAM_DEFECT_UNEXPECTED_PERIOD)])
                || (mac_status_change & defect_check[_sys_usw_oam_cal_bitmap_bit(lchip, CTC_OAM_DEFECT_MAC_STATUS_CHANGE)])
                || (rmepmacmismatch & defect_check[_sys_usw_oam_cal_bitmap_bit(lchip, CTC_OAM_DEFECT_SRC_MAC_MISMATCH)]))
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Check Rmep[%d]:  d_loc %d,  d_unexp_period % d ,mac_status_change % d, rmepmacmismatch %d\n",
                                 r_index, d_loc, d_unexp_period, mac_status_change,  rmepmacmismatch);
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Some rmep still has dloc defect \n");
                return CTC_E_INVALID_CONFIG;

            }
        }
    }
RETURN:
    return CTC_E_NONE;

}

int32
_sys_usw_oam_lmep_update_eth_asic(uint8 lchip, sys_oam_lmep_t* p_sys_lmep, ctc_oam_update_t* p_lmep_param)
{
    sys_oam_lmep_t* p_sys_lmep_eth  = (sys_oam_lmep_t*)p_sys_lmep;
    uint32 cmd        = 0;
    uint32 index      = 0;
    uint32 value = 0;
    DsMa_m ds_ma;                    /*DS_MA */
    DsEthMep_m ds_eth_mep;          /* DS_ETH_MEP */
    DsEthRmep_m ds_eth_rmep;
    DsEthMep_m DsEthMep_mask;     /* DS_ETH_MEP */
    DsEthRmep_m DsEthRmep_mask;

    tbl_entry_t tbl_entry;

    sys_oam_rmep_t* p_sys_rmep = NULL;

    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
    {
        goto RETURN;
    }

    sal_memset(&ds_eth_mep, 0, sizeof(ds_eth_mep));
    sal_memset(&DsEthMep_mask, 0xFF, sizeof(ds_eth_mep));
    index = p_sys_lmep->lmep_index;
    cmd = DRV_IOR(DsEthMep_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_eth_mep));

    sal_memset(&ds_ma, 0, sizeof(DsMa_m));
    index = p_sys_lmep->ma_index;
    cmd = DRV_IOR(DsMa_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_ma));

    if (CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE)
        && ((CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_EN == p_lmep_param->update_type)
            || (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_TYPE == p_lmep_param->update_type)
            || (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_USE_UC_DA == p_lmep_param->update_type)
            || (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_STATS_EN == p_lmep_param->update_type)))
    {
        p_sys_rmep = (sys_oam_rmep_t*)CTC_SLISTHEAD(p_sys_lmep->rmep_list);
        if (NULL == p_sys_rmep)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Remote mep is not found \n");
			return CTC_E_NOT_EXIST;
        }

        index = p_sys_rmep->rmep_index;
        sal_memset(&ds_eth_rmep, 0, sizeof(DsEthRmep_m));
        sal_memset(&DsEthRmep_mask, 0xFF, sizeof(DsEthRmep_m));
        cmd = DRV_IOR(DsEthRmep_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_eth_rmep));


    }

    switch (p_lmep_param->update_type)
    {
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_EN:

        if (CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_EN))
        {
            SetDsEthMep(V, active_f                , &ds_eth_mep ,  1);
            SetDsEthMep(V, isRemote_f              , &ds_eth_mep , 0);
            SetDsEthMep(V, cciWhile_f              , &ds_eth_mep , 4);
            SetDsEthMep(V, dUnexpMepTimer_f        , &ds_eth_mep , 14);
            SetDsEthMep(V, dMismergeTimer_f        , &ds_eth_mep , 14);
            SetDsEthMep(V, dMegLvlTimer_f          , &ds_eth_mep , 14);
            SetDsEthMep(V, portStatus_f            , &ds_eth_mep , 0);
            /*wangt SetDsEthMep(V, presentTraffic_f        , &ds_eth_mep , 0);*/
            SetDsEthMep(V, presentRdi_f            , &ds_eth_mep , 0);
            SetDsEthMep(V, ccmSeqNum_f            , &ds_eth_mep , 0);
            SetDsEthMep(V, dUnexpMep_f             , &ds_eth_mep , 0);
            SetDsEthMep(V, dMismerge_f             , &ds_eth_mep , 0);
            SetDsEthMep(V, dMegLvl_f               , &ds_eth_mep , 0);
        }
        else
        {
            SetDsEthMep(V, active_f                , &ds_eth_mep ,  0);
        }
        SetDsEthMep(V, active_f                , &DsEthMep_mask ,  0);

        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_EN:
        if (CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE))
        {
            p_sys_rmep = (sys_oam_rmep_t*)CTC_SLISTHEAD(p_sys_lmep->rmep_list);
            if (p_sys_rmep)
            {
                cmd = DRV_IOR(DsEthRmep_t, DsEthRmep_isCsfEn_f);
                DRV_IOCTL(lchip, p_sys_rmep->rmep_index, cmd, &value);
                if (value)
                {
                    return CTC_E_INVALID_CONFIG;
                }
            }

        }
        SetDsEthMep(V, cciEn_f                , &ds_eth_mep ,  p_lmep_param->update_value);
        SetDsEthMep(V, cciEn_f                , &DsEthMep_mask ,  0);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_SEQ_NUM_EN:
        if (GetDsEthMep(V, ccmStatEn_f, &ds_eth_mep))
        {
            return CTC_E_INVALID_CONFIG;
        }
        SetDsEthMep(V, seqNumEn_f                , &ds_eth_mep ,  CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_CCM_SEQ_NUM_EN));
        SetDsEthMep(V, seqNumEn_f                , &DsEthMep_mask ,  0);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_RDI:
        if (0 == p_lmep_param->update_value)
        {
            CTC_ERROR_RETURN(_sys_usw_oam_lmep_check_rmep_dloc(lchip, p_sys_lmep));
        }
        SetDsEthMep(V, presentRdi_f                , &ds_eth_mep ,  p_lmep_param->update_value);
        SetDsEthMep(V, presentRdi_f                , &DsEthMep_mask ,  0);

        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_COS_EXP:
        SetDsMa(V, priorityIndex_f, &ds_ma ,  p_lmep_param->update_value);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_PORT_STATUS:
        SetDsEthMep(V, portStatus_f                , &ds_eth_mep ,  p_lmep_param->update_value);
        SetDsEthMep(V, portStatus_f                , &DsEthMep_mask ,  0);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LMEP_SF_STATE:
        return CTC_E_NOT_SUPPORT;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_EN:
        cmd = DRV_IOR(DsEthMep_t, DsEthMep_cciEn_f);
        DRV_IOCTL(lchip, p_sys_lmep->lmep_index, cmd, &value);
        if (value)
        {
            return CTC_E_INVALID_CONFIG;
        }
        SetDsEthRmep(V, firstPktRx_f, &ds_eth_rmep , 1);
        SetDsEthRmep(V, firstPktRx_f, &DsEthRmep_mask ,  0);
        SetDsEthRmep(V, dLoc_f, &ds_eth_rmep , 1);
        SetDsEthRmep(V, dLoc_f, &DsEthRmep_mask ,  0);
        SetDsEthRmep(V, isCsfEn_f, &ds_eth_rmep , 1);
        SetDsEthRmep(V, isCsfEn_f, &DsEthRmep_mask , 0);
        SetDsEthRmep(V, isCsfTxEn_f, &ds_eth_rmep ,  p_lmep_param->update_value);
        SetDsEthRmep(V, isCsfTxEn_f, &DsEthRmep_mask , 0);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_TYPE:
        SetDsEthRmep(V, csfType_f      , &ds_eth_rmep , p_lmep_param->update_value);
        SetDsEthRmep(V, csfType_f      , &DsEthRmep_mask , 0);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_USE_UC_DA:
        SetDsEthRmep(V, isCsfUseUcDa_f   , &ds_eth_rmep ,  p_lmep_param->update_value);
        SetDsEthRmep(V, isCsfUseUcDa_f   , &DsEthRmep_mask , 0);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_P2P_TX_USE_UC_DA:
        SetDsEthMep(V, p2pUseUcDa_f                , &ds_eth_mep , p_lmep_param->update_value);
        SetDsEthMep(V, p2pUseUcDa_f                , &DsEthMep_mask ,  0);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_DM_EN:
        SetDsEthMep(V, enablePm_f                  , &ds_eth_mep , p_lmep_param->update_value);
        SetDsEthMep(V, enablePm_f                  , &DsEthMep_mask ,  0);
        break;

   case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NPM:
   case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TRPT:
       SetDsEthMep(V, autoGenEn_f, &ds_eth_mep , (p_lmep_param->update_value == 0xff) ? 0 : 1);
       SetDsEthMep(V, autoGenEn_f, &DsEthMep_mask ,  0);

       SetDsEthMep(V, autoGenPktPtr_f, &ds_eth_mep , (p_lmep_param->update_value == 0xff) ? 0 : p_lmep_param->update_value);
       SetDsEthMep(V, autoGenPktPtr_f, &DsEthMep_mask , 0);
       if (CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_TWAMP_EN)
           && !CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_TWAMP_REFLECT_EN))
       {
           SetDsEthMep(V, learnEn_f, &ds_eth_mep , 1);
           SetDsEthMep(V, learnEn_f, &DsEthMep_mask ,  0);
       }
       break;
   case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_INTERVAL:
       SetDsEthMep(V, ccmInterval_f, &ds_eth_mep , p_lmep_param->update_value);
       SetDsEthMep(V, ccmInterval_f, &DsEthMep_mask ,  0);
       break;
   case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_ID:
       SetDsEthMep(V, mepId_f, &ds_eth_mep , p_lmep_param->update_value);
       SetDsEthMep(V, mepId_f, &DsEthMep_mask ,  0);
       break;
   case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MAID:
       CTC_ERROR_RETURN(_sys_usw_oam_maid_update(lchip, p_lmep_param, p_sys_lmep));
       SetDsMa(V, maIdLengthType_f, &ds_ma, (p_sys_lmep->p_sys_maid->maid_entry_num >> 1));
       SetDsMa(V, maNameIndex_f, &ds_ma, p_sys_lmep->p_sys_maid->maid_index);
       SetDsMa(V, maNameLen_f, &ds_ma, p_sys_lmep->p_sys_maid->maid_len);
       break;
   case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_STATS_EN:
       if (!GetDsEthMep(V, ethOamP2PMode_f, &ds_eth_mep)
           || GetDsEthMep(V, seqNumEn_f, &ds_eth_mep)
           || GetDsEthRmep(V, seqNumEn_f, &ds_eth_rmep))
       {
            return CTC_E_INVALID_CONFIG;
       }
       SetDsEthMep(V, ccmSeqNum_f, &ds_eth_mep, 0);
       SetDsEthMep(V, ccmSeqNum_f, &DsEthMep_mask, 0);
       SetDsEthMep(V, oamDiscardCnt_f, &ds_eth_mep, 0);
       SetDsEthMep(V, oamDiscardCnt_f, &DsEthMep_mask, 0);
       SetDsEthRmep(V, ccmSeqNum_f, &ds_eth_rmep, 0);
       SetDsEthRmep(V, ccmSeqNum_f, &DsEthRmep_mask, 0);
       SetDsEthMep(V, ccmStatEn_f, &ds_eth_mep, p_lmep_param->update_value ? 1 : 0);
       SetDsEthMep(V, ccmStatEn_f, &DsEthMep_mask, 0);
       break;
   default:
       return CTC_E_INVALID_PARAM;
    }

    index = p_sys_lmep->ma_index;
    CTC_ERROR_RETURN(sys_usw_ma_add_to_asic(lchip, index, &ds_ma));

    index = p_sys_lmep->lmep_index;
    cmd = DRV_IOW(DsEthMep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_eth_mep;
    tbl_entry.mask_entry = (uint32*)&DsEthMep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));

    if (CTC_FLAG_ISSET(p_sys_lmep_eth->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE)
        && ((CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_EN == p_lmep_param->update_type)
            || (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_TYPE == p_lmep_param->update_type)
            || (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_USE_UC_DA == p_lmep_param->update_type)
            || (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_STATS_EN == p_lmep_param->update_type)))
    {
        index = p_sys_rmep->rmep_index;
        cmd = DRV_IOW(DsEthRmep_t, DRV_ENTRY_FLAG);
        tbl_entry.data_entry = (uint32*)&ds_eth_rmep;
        tbl_entry.mask_entry = (uint32*)&DsEthRmep_mask;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));
    }

RETURN:

    return CTC_E_NONE;
}

int32
_sys_usw_oam_lmep_update_tp_y1731_asic(uint8 lchip, sys_oam_lmep_t* p_sys_lmep, ctc_oam_update_t* p_lmep_param)
{
    sys_oam_lmep_t* p_sys_lmep_tp  = (sys_oam_lmep_t*)p_sys_lmep;
    uint32 cmd        = 0;
    uint32 index      = 0;
    uint32 value = 0;
    DsMa_m ds_ma;                    /*DS_MA */
    DsEthMep_m ds_eth_mep;          /* DS_ETH_MEP */
    DsEthRmep_m ds_eth_rmep;
    DsEthMep_m DsEthMep_mask;     /* DS_ETH_MEP */
    DsEthRmep_m DsEthRmep_mask;
    tbl_entry_t tbl_entry;
    sys_oam_rmep_t* p_sys_rmep = NULL;
    sys_oam_nhop_info_t mpls_nhop_info;
    sys_nh_update_oam_info_t oam_info;

    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
    {
        goto RETURN;
    }

    sal_memset(&ds_eth_mep, 0, sizeof(DsEthMep_m));
    sal_memset(&DsEthMep_mask, 0xFF, sizeof(DsEthMep_m));
    index = p_sys_lmep->lmep_index;
    cmd = DRV_IOR(DsEthMep_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_eth_mep));

    sal_memset(&ds_ma, 0, sizeof(DsMa_m));
    index = p_sys_lmep->ma_index;
    cmd = DRV_IOR(DsMa_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_ma));

    if ((CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_EN == p_lmep_param->update_type)
        || (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_TYPE == p_lmep_param->update_type)
        || (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_STATS_EN == p_lmep_param->update_type))
    {
        p_sys_rmep = (sys_oam_rmep_t*)CTC_SLISTHEAD(p_sys_lmep->rmep_list);
        if (NULL == p_sys_rmep)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Remote mep is not found \n");
			return CTC_E_NOT_EXIST;
        }

        index = p_sys_rmep->rmep_index;
        sal_memset(&ds_eth_rmep, 0, sizeof(DsEthRmep_m));
        sal_memset(&DsEthRmep_mask, 0xFF, sizeof(DsEthRmep_m));
        cmd = DRV_IOR(DsEthRmep_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_eth_rmep));
    }

    switch (p_lmep_param->update_type)
    {
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_EN:

        if (CTC_FLAG_ISSET(p_sys_lmep_tp->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_EN))
        {
            SetDsEthMep(V, active_f                , &ds_eth_mep ,  1);
            SetDsEthMep(V, isRemote_f              , &ds_eth_mep , 0);
            SetDsEthMep(V, cciWhile_f              , &ds_eth_mep , 4);
            SetDsEthMep(V, dUnexpMepTimer_f        , &ds_eth_mep , 14);
            SetDsEthMep(V, dMismergeTimer_f        , &ds_eth_mep , 14);
            SetDsEthMep(V, dMegLvlTimer_f          , &ds_eth_mep , 14);
            SetDsEthMep(V, portStatus_f            , &ds_eth_mep , 0);
            /*wangt SetDsEthMep(V, presentTraffic_f        , &ds_eth_mep , 0);*/
            SetDsEthMep(V, presentRdi_f            , &ds_eth_mep , 0);
            SetDsEthMep(V, ccmSeqNum_f            , &ds_eth_mep , 0);
            SetDsEthMep(V, dUnexpMep_f             , &ds_eth_mep , 0);
            SetDsEthMep(V, dMismerge_f             , &ds_eth_mep , 0);
            SetDsEthMep(V, dMegLvl_f               , &ds_eth_mep , 0);
        }
        else
        {
            SetDsEthMep(V, active_f                , &ds_eth_mep ,  0);
        }
        SetDsEthMep(V, active_f                , &DsEthMep_mask ,  0);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_EN:
        p_sys_rmep = (sys_oam_rmep_t*)CTC_SLISTHEAD(p_sys_lmep->rmep_list);
        if (p_sys_rmep)
        {
            cmd = DRV_IOR(DsEthRmep_t, DsEthRmep_isCsfEn_f);
            DRV_IOCTL(lchip, p_sys_rmep->rmep_index, cmd, &value);
            if (value)
            {
                return CTC_E_INVALID_CONFIG;
            }
        }
        SetDsEthMep(V, cciEn_f                , &ds_eth_mep ,  p_lmep_param->update_value);
        SetDsEthMep(V, cciEn_f                , &DsEthMep_mask ,  0);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_RDI:
        if (0 == p_lmep_param->update_value)
        {
            CTC_ERROR_RETURN(_sys_usw_oam_lmep_check_rmep_dloc(lchip, p_sys_lmep));
        }
        SetDsEthMep(V, presentRdi_f                , &ds_eth_mep ,  p_lmep_param->update_value);
        SetDsEthMep(V, presentRdi_f                , &DsEthMep_mask ,  0);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_COS_EXP:
        SetDsMa(V, priorityIndex_f, &ds_ma, p_lmep_param->update_value);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LMEP_SF_STATE:
       /*  ds_ma.aps_signal_fail_local           = p_sys_lmep_tp->sf_state;*/
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_EN:
        cmd = DRV_IOR(DsEthMep_t, DsEthMep_cciEn_f);
        DRV_IOCTL(lchip, p_sys_lmep->lmep_index, cmd, &value);
        if (value)
        {
            return CTC_E_INVALID_CONFIG;
        }
        SetDsEthRmep(V, firstPktRx_f, &ds_eth_rmep , 1);
        SetDsEthRmep(V, firstPktRx_f, &DsEthRmep_mask ,  0);
        SetDsEthRmep(V, dLoc_f, &ds_eth_rmep , 1);
        SetDsEthRmep(V, dLoc_f, &DsEthRmep_mask ,  0);
        SetDsEthRmep(V, isCsfEn_f, &ds_eth_rmep , 1);
        SetDsEthRmep(V, isCsfEn_f, &DsEthRmep_mask , 0);
        SetDsEthRmep(V, isCsfTxEn_f, &ds_eth_rmep ,  p_lmep_param->update_value);
        SetDsEthRmep(V, isCsfTxEn_f, &DsEthRmep_mask , 0);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_TYPE:
        SetDsEthRmep(V, csfType_f      , &ds_eth_rmep , p_lmep_param->update_value);
        SetDsEthRmep(V, csfType_f      , &DsEthRmep_mask , 0);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_DM_EN:
        SetDsEthMep(V, enablePm_f                , &ds_eth_mep , p_lmep_param->update_value);
        SetDsEthMep(V, enablePm_f                , &DsEthMep_mask ,  0);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NHOP:
        sal_memset(&mpls_nhop_info, 0, sizeof(sys_oam_nhop_info_t));
        sal_memset(&oam_info, 0, sizeof(sys_nh_update_oam_info_t));

        CTC_ERROR_RETURN(_sys_usw_oam_get_nexthop_info(lchip, p_sys_lmep_tp->nhid, CTC_FLAG_ISSET(p_sys_lmep_tp->flag, CTC_OAM_Y1731_LMEP_FLAG_PROTECTION_PATH), &mpls_nhop_info));
        SetDsMa(V, nextHopExt_f         , &ds_ma , mpls_nhop_info.nexthop_is_8w);
        SetDsMa(V, nextHopPtr_f         , &ds_ma , mpls_nhop_info.dsnh_offset);
        SetDsEthMep(V, destMap_f           , &ds_eth_mep , mpls_nhop_info.dest_map);
        SetDsEthMep(V, apsBridgeEn_f       , &ds_eth_mep , mpls_nhop_info.aps_bridge_en);
        SetDsEthMep(V, destMap_f           , &DsEthMep_mask , 0 );
        SetDsEthMep(V, apsBridgeEn_f       , &DsEthMep_mask , 0 );
        SetDsMa(V, tunnelApsEn_f,           &ds_ma , mpls_nhop_info.mep_on_tunnel);
        SetDsMa(V, protectingPath_f,        &ds_ma , CTC_FLAG_ISSET(p_sys_lmep_tp->flag, CTC_OAM_Y1731_LMEP_FLAG_PROTECTION_PATH));

        oam_info.update_type = 1;
        oam_info.is_protection_path = CTC_FLAG_ISSET(p_sys_lmep_tp->flag, CTC_OAM_Y1731_LMEP_FLAG_PROTECTION_PATH);
        oam_info.dsma_en = 1;
        oam_info.ma_idx = p_sys_lmep_tp->ma_index;
        oam_info.mep_index = p_sys_lmep->lmep_index;
        oam_info.mep_type  = 1;
        sys_usw_nh_update_oam_en(lchip, p_sys_lmep_tp->nhid, &oam_info);
        break;

    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEL:
        SetDsMa(V, mdLvl_f , &ds_ma ,  p_lmep_param->update_value);
        break;
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TRPT:
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_NPM:
        SetDsEthMep(V, autoGenEn_f, &ds_eth_mep , (p_lmep_param->update_value == 0xff) ? 0 : 1);
        SetDsEthMep(V, autoGenEn_f, &DsEthMep_mask ,  0);
        SetDsEthMep(V, autoGenPktPtr_f, &ds_eth_mep , (p_lmep_param->update_value == 0xff) ? 0 : p_lmep_param->update_value);
        SetDsEthMep(V, autoGenPktPtr_f, &DsEthMep_mask , 0);
        break;
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TTL:
        SetDsMa(V, mplsTtl_f, &ds_ma, p_lmep_param->update_value);
        break;
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_INTERVAL:
        SetDsEthMep(V, ccmInterval_f, &ds_eth_mep , p_lmep_param->update_value);
        SetDsEthMep(V, ccmInterval_f, &DsEthMep_mask ,  0);
        break;
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MEP_ID:
        SetDsEthMep(V, mepId_f, &ds_eth_mep , p_lmep_param->update_value);
        SetDsEthMep(V, mepId_f, &DsEthMep_mask ,  0);
        break;
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_MAID:
        CTC_ERROR_RETURN(_sys_usw_oam_maid_update(lchip, p_lmep_param, p_sys_lmep));
        SetDsMa(V, maIdLengthType_f, &ds_ma, (p_sys_lmep->p_sys_maid->maid_entry_num >> 1));
        SetDsMa(V, maNameIndex_f, &ds_ma, p_sys_lmep->p_sys_maid->maid_index);
        SetDsMa(V, maNameLen_f, &ds_ma, p_sys_lmep->p_sys_maid->maid_len);
        break;
    case CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_STATS_EN:
        if (!GetDsEthMep(V, ethOamP2PMode_f, &ds_eth_mep)
            || GetDsEthMep(V, seqNumEn_f, &ds_eth_mep)
        || GetDsEthRmep(V, seqNumEn_f, &ds_eth_rmep))
        {
            return CTC_E_INVALID_CONFIG;
        }
        SetDsEthMep(V, ccmSeqNum_f, &ds_eth_mep, 0);
        SetDsEthMep(V, ccmSeqNum_f, &DsEthMep_mask, 0);
        SetDsEthMep(V, oamDiscardCnt_f, &ds_eth_mep, 0);
        SetDsEthMep(V, oamDiscardCnt_f, &DsEthMep_mask, 0);
        SetDsEthRmep(V, ccmSeqNum_f, &ds_eth_rmep, 0);
        SetDsEthRmep(V, ccmSeqNum_f, &DsEthRmep_mask, 0);
        SetDsEthMep(V, ccmStatEn_f, &ds_eth_mep, p_lmep_param->update_value ? 1 : 0);
        SetDsEthMep(V, ccmStatEn_f, &DsEthMep_mask, 0);
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    index = p_sys_lmep->ma_index;
    CTC_ERROR_RETURN(sys_usw_ma_add_to_asic(lchip, index, &ds_ma));

    index = p_sys_lmep->lmep_index;
    cmd = DRV_IOW(DsEthMep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_eth_mep;
    tbl_entry.mask_entry = (uint32*)&DsEthMep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));

    if ((CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_EN == p_lmep_param->update_type)
        || (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_CSF_TYPE == p_lmep_param->update_type)
        || (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_STATS_EN == p_lmep_param->update_type))
    {
        index = p_sys_rmep->rmep_index;
        cmd = DRV_IOW(DsEthRmep_t, DRV_ENTRY_FLAG);
        tbl_entry.data_entry = (uint32*)&ds_eth_rmep;
        tbl_entry.mask_entry = (uint32*)&DsEthRmep_mask;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));
    }

RETURN:

    return CTC_E_NONE;
}

int32
_sys_usw_oam_lmep_update_master_chip(uint8 lchip, sys_oam_chan_com_t* p_sys_chan, uint32 master_chipid)
{
    uint8 mep_type = p_sys_chan->mep_type;
    uint32 key_index = 0;
    uint8 link_oam = 0;
    DsEgressXcOamBfdHashKey_m ds_oam_key;

    switch (mep_type)
    {
        case CTC_OAM_MEP_TYPE_IP_BFD:
        case CTC_OAM_MEP_TYPE_MPLS_BFD:
            key_index =    ((sys_oam_chan_bfd_t*)p_sys_chan)->key.com.key_index;
            break;
        case CTC_OAM_MEP_TYPE_MPLS_TP_Y1731:
        case CTC_OAM_MEP_TYPE_MPLS_TP_BFD:
            key_index =    ((sys_oam_chan_tp_t*)p_sys_chan)->key.com.key_index;
            link_oam = ((sys_oam_chan_tp_t*)p_sys_chan)->key.section_oam;
            break;
        case CTC_OAM_MEP_TYPE_ETH_1AG:
        case CTC_OAM_MEP_TYPE_ETH_Y1731:
            key_index =    ((sys_oam_chan_eth_t*)p_sys_chan)->key.com.key_index;
            link_oam = ((sys_oam_chan_eth_t*)p_sys_chan)->key.link_oam;
            break;
        default:
            break;
    }
    sal_memset(&ds_oam_key, 0, sizeof(ds_oam_key));
    CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, DsEgressXcOamBfdHashKey_t, key_index, &ds_oam_key));

    switch (mep_type)
    {
        case CTC_OAM_MEP_TYPE_IP_BFD:
        case CTC_OAM_MEP_TYPE_MPLS_BFD:
            {
                SetDsEgressXcOamBfdHashKey(V, oamDestChipId_f, &ds_oam_key,         master_chipid);
            }
            break;
        case CTC_OAM_MEP_TYPE_MPLS_TP_Y1731:
        case CTC_OAM_MEP_TYPE_MPLS_TP_BFD:
            {
                if (link_oam)
                {
                    SetDsEgressXcOamMplsSectionHashKey(V, oamDestChipId_f, &ds_oam_key, master_chipid);
                }
                else
                {
                    SetDsEgressXcOamMplsLabelHashKey(V, oamDestChipId_f, &ds_oam_key,           master_chipid);
                }
            }
            break;
        case CTC_OAM_MEP_TYPE_ETH_1AG:
        case CTC_OAM_MEP_TYPE_ETH_Y1731:
            {
                if (!link_oam)
                {
                    SetDsEgressXcOamEthHashKey(V, oamDestChipId_f, &ds_oam_key,         master_chipid);
                }
            }
            break;
        default:
            break;
    }

    CTC_ERROR_RETURN(sys_usw_oam_key_write_io(lchip, DsEgressXcOamBfdHashKey_t, key_index, &ds_oam_key));
    p_sys_chan->master_chipid = master_chipid;
    return CTC_E_NONE;
}

int32
_sys_usw_oam_lmep_update_label(uint8 lchip, sys_oam_chan_com_t* p_sys_chan, ctc_oam_tp_key_t* tp_key_new)
{
    sys_oam_chan_tp_t* p_sys_chan_tp = NULL;

    p_sys_chan_tp = (sys_oam_chan_tp_t*)p_sys_chan;
    if ((p_sys_chan_tp->key.label == tp_key_new->label) && (p_sys_chan_tp->key.spaceid == tp_key_new->mpls_spaceid))
    {
        return CTC_E_NONE;
    }
    sys_usw_mpls_swap_oam_info(lchip,p_sys_chan_tp->key.label, p_sys_chan_tp->key.spaceid, tp_key_new->label, tp_key_new->mpls_spaceid);
    CTC_ERROR_RETURN(_sys_usw_oam_chan_del_from_db(lchip, (sys_oam_chan_com_t*)p_sys_chan_tp));
    p_sys_chan_tp->key.label = tp_key_new->label;
    p_sys_chan_tp->key.spaceid = tp_key_new->mpls_spaceid;
    CTC_ERROR_RETURN(_sys_usw_oam_chan_add_to_db(lchip, (sys_oam_chan_com_t*)p_sys_chan_tp));
    return CTC_E_NONE;
}



int32
_sys_usw_oam_lmep_update_for_traverse(sys_oam_chan_com_t* p_sys_chan, void* user_data)
{
    uint8 lchip = 0;
    uint32 master_chipid = 0;
    sys_traverse_t* traversal_data = (sys_traverse_t *)user_data;
    lchip = traversal_data->value1;
    master_chipid = traversal_data->value2;
    _sys_usw_oam_lmep_update_master_chip(lchip, p_sys_chan, master_chipid);
    return CTC_E_NONE;
}

#define RMEP "Function Begin"

sys_oam_rmep_t*
_sys_usw_oam_rmep_lkup(uint8 lchip, sys_oam_lmep_t* p_sys_lmep, sys_oam_rmep_t* p_sys_rmep)
{
    ctc_slistnode_t* ctc_slistnode = NULL;
    ctc_slist_t* p_rmep_list = NULL;
    sys_oam_rmep_t* p_sys_rmep_db = NULL;
    uint8 oam_type = 0;
    uint8 cmp_result = 0;

    oam_type = p_sys_lmep->p_sys_chan->mep_type;

    p_rmep_list = p_sys_lmep->rmep_list;

    CTC_SLIST_LOOP(p_rmep_list, ctc_slistnode)
    {
        p_sys_rmep_db = _ctc_container_of(ctc_slistnode, sys_oam_rmep_t, head);
        if (NULL == p_sys_rmep_db)
        {
            continue;
        }
        switch(oam_type)
        {
        case CTC_OAM_MEP_TYPE_IP_BFD:
        case CTC_OAM_MEP_TYPE_MPLS_BFD:
        case CTC_OAM_MEP_TYPE_MICRO_BFD:
        case CTC_OAM_MEP_TYPE_MPLS_TP_BFD:
            cmp_result = TRUE;
            break;
        case CTC_OAM_MEP_TYPE_MPLS_TP_Y1731:
        case CTC_OAM_MEP_TYPE_ETH_1AG:
        case CTC_OAM_MEP_TYPE_ETH_Y1731:
            if (p_sys_rmep->rmep_id == p_sys_rmep_db->rmep_id)
            {
                cmp_result = TRUE;
            }
            break;

        default:
            break;
        }
        if (TRUE == cmp_result)
        {
            return p_sys_rmep_db;
        }
    }

    return NULL;
}

int32
_sys_usw_oam_rmep_add_to_db(uint8 lchip, sys_oam_rmep_t* p_sys_rmep)
{
    uint32 rmep_index   = 0;
    sys_oam_lmep_t* p_sys_lmep = NULL;

    p_sys_lmep  = p_sys_rmep->p_sys_lmep;

    rmep_index  = p_sys_rmep->rmep_index;

    if (!g_oam_master[lchip]->mep_index_alloc_by_sdk)
    {
        if (ctc_vector_get(g_oam_master[lchip]->mep_vec, rmep_index))
        {
            return CTC_E_EXIST;  /*need check when index alloced by system*/
        }
    }

    if (NULL == p_sys_lmep->rmep_list)
    {
        p_sys_lmep->rmep_list = ctc_slist_new();
        if (NULL == p_sys_lmep->rmep_list)
        {
            return CTC_E_NO_MEMORY;
        }
    }

    ctc_slist_add_tail(p_sys_lmep->rmep_list, &(p_sys_rmep->head));

    if (FALSE == ctc_vector_add(g_oam_master[lchip]->mep_vec, rmep_index, p_sys_rmep))
    {
        return CTC_E_NO_MEMORY;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_oam_rmep_del_from_db(uint8 lchip, sys_oam_rmep_t* p_sys_rmep)
{
    uint32 rmep_index = 0;
    sys_oam_lmep_t* p_sys_lmep = NULL;

    p_sys_lmep  = p_sys_rmep->p_sys_lmep;
    rmep_index  = p_sys_rmep->rmep_index;

    if (NULL == p_sys_lmep->rmep_list)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Local mep not found \n");
		return CTC_E_NOT_EXIST;

    }

    ctc_slist_delete_node(p_sys_lmep->rmep_list, &(p_sys_rmep->head));

    if (0 == CTC_SLISTCOUNT(p_sys_lmep->rmep_list))
    {
        ctc_slist_free(p_sys_lmep->rmep_list);
        p_sys_lmep->rmep_list = NULL;
    }

    ctc_vector_del(g_oam_master[lchip]->mep_vec, rmep_index);

    return CTC_E_NONE;
}

int32
_sys_usw_oam_rmep_build_index(uint8 lchip, sys_oam_rmep_t* p_sys_rmep)
{
    sys_oam_lmep_t* p_sys_lmep = NULL;
    sys_usw_opf_t opf;
    uint32 offset = 0;

    p_sys_lmep = p_sys_rmep->p_sys_lmep;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = g_oam_master[lchip]->oam_opf_type;
    if (g_oam_master[lchip]->mep_index_alloc_by_sdk)
    {
        if (FALSE == sys_usw_chip_is_local(lchip, p_sys_lmep->p_sys_chan->master_chipid))
        {
            return CTC_E_NONE;
        }
        opf.pool_index  = SYS_OAM_OPF_TYPE_MEP_LMEP;
        offset = p_sys_lmep->lmep_index + 1;
        if (((CTC_OAM_MEP_TYPE_ETH_1AG == p_sys_lmep->p_sys_chan->mep_type)
            ||(CTC_OAM_MEP_TYPE_ETH_Y1731 == p_sys_lmep->p_sys_chan->mep_type))
            && (!CTC_FLAG_ISSET(((sys_oam_lmep_t*)p_sys_lmep)->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE)))
        {
            CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, 1, &offset));
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MEP, 1, 1);
        }
        p_sys_rmep->rmep_index = offset;
    }
    else
    {
        offset = p_sys_rmep->rmep_index;
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MEP, 1, 1);
    }

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"RMEP: lchip->%d, index->%d\n", lchip, offset);

    return CTC_E_NONE;
}

int32
_sys_usw_oam_rmep_free_index(uint8 lchip, sys_oam_rmep_t* p_sys_rmep)
{
    sys_oam_lmep_t* p_sys_lmep = NULL;
    sys_usw_opf_t opf;
    uint32 offset = 0;

    p_sys_lmep = p_sys_rmep->p_sys_lmep;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = g_oam_master[lchip]->oam_opf_type;
    if (g_oam_master[lchip]->mep_index_alloc_by_sdk)
    {
        if (FALSE == sys_usw_chip_is_local(lchip, p_sys_lmep->p_sys_chan->master_chipid))
        {
            return CTC_E_NONE;
        }

        offset = p_sys_rmep->rmep_index;
        opf.pool_index  = SYS_OAM_OPF_TYPE_MEP_LMEP;
        if (((CTC_OAM_MEP_TYPE_ETH_1AG == p_sys_lmep->p_sys_chan->mep_type)
            ||(CTC_OAM_MEP_TYPE_ETH_Y1731 == p_sys_lmep->p_sys_chan->mep_type))
            && (!CTC_FLAG_ISSET(((sys_oam_lmep_t*)p_sys_lmep)->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE)))
        {
            CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, 1, offset));
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MEP, 0, 1);
        }
    }
    else
    {
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MEP, 0, 1);
    }

    return CTC_E_NONE;

}

int32
_sys_usw_oam_rmep_add_eth_to_asic(uint8 lchip, ctc_oam_rmep_t* p_rmep_param, sys_oam_rmep_t* p_sys_rmep)
{
    DsEthRmep_m ds_eth_rmep;
    DsEthRmep_m DsEthRmep_mask;
    DsEthMep_m ds_eth_mep;
    DsEthMep_m DsEthMep_mask;
    sys_oam_lmep_t* p_sys_lmep = NULL;
    sys_oam_rmep_t* p_sys_eth_rmep = NULL;
    ctc_oam_y1731_rmep_t* p_eth_rmep = NULL;
    uint32 cmd          = 0;
    uint32 index        = 0;
    uint32 rmep_flag    = 0;
    uint32 lmep_flag    = 0;
    uint8  tx_csf_type  = 0;
    tbl_entry_t tbl_entry;
    hw_mac_addr_t mac_sa;

    p_sys_lmep     = p_sys_rmep->p_sys_lmep;
    p_sys_eth_rmep = (sys_oam_rmep_t*)p_sys_rmep;
    p_eth_rmep = (ctc_oam_y1731_rmep_t*)&p_rmep_param->u.y1731_rmep;

    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
    {
        goto RETURN;
    }
    rmep_flag   = p_sys_eth_rmep->flag;
    lmep_flag   = p_sys_lmep->flag;
    tx_csf_type = p_sys_lmep->tx_csf_type;

    sal_memset(&ds_eth_rmep, 0, sizeof(DsEthRmep_m));
    sal_memset(&DsEthRmep_mask, 0, sizeof(DsEthRmep_m));

     SetDsEthRmep(V, active_f               , &ds_eth_rmep , CTC_FLAG_ISSET(rmep_flag, CTC_OAM_Y1731_RMEP_FLAG_MEP_EN));
     SetDsEthRmep(V, ethOamP2PMode_f        , &ds_eth_rmep , CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE));
     SetDsEthRmep(V, seqNumEn_f             , &ds_eth_rmep , CTC_FLAG_ISSET(rmep_flag, CTC_OAM_Y1731_RMEP_FLAG_CCM_SEQ_NUM_EN));
     SetDsEthRmep(V, macAddrUpdateDisable_f , &ds_eth_rmep , !CTC_FLAG_ISSET(rmep_flag, CTC_OAM_Y1731_RMEP_FLAG_MAC_UPDATE_EN));
     SetDsEthRmep(V, apsSignalFailRemote_f  , &ds_eth_rmep , 0);


     if (g_oam_master[lchip]->timer_update_disable)
     {
         SetDsEthRmep(V, firstPktRx_f           , &ds_eth_rmep , 1);
     }
     else
     {
         SetDsEthRmep(V, firstPktRx_f           , &ds_eth_rmep , 0);
     }

    SetDsEthRmep(V, dLoc_f                 , &ds_eth_rmep , 0);
    if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE))
    {
        SetDsEthRmep(V, isCsfEn_f              , &ds_eth_rmep , CTC_FLAG_ISSET(rmep_flag, CTC_OAM_Y1731_RMEP_FLAG_CSF_EN));
        if (CTC_FLAG_ISSET(rmep_flag, CTC_OAM_Y1731_RMEP_FLAG_CSF_EN))
        {
            SetDsEthRmep(V, firstPktRx_f           , &ds_eth_rmep , 1);
            SetDsEthRmep(V, dLoc_f                 , &ds_eth_rmep , 1);
        }

        if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_TX_CSF_EN))
        {
            SetDsEthRmep(V, isCsfTxEn_f      , &ds_eth_rmep , 1);
            SetDsEthRmep(V, rxCsfType_f      , &ds_eth_rmep , tx_csf_type);
            SetDsEthRmep(V, dCsf_f           , &ds_eth_rmep , 0);
            SetDsEthRmep(V, rxCsfWhile_f     , &ds_eth_rmep , 0);
            SetDsEthRmep(V, isCsfUseUcDa_f   , &ds_eth_rmep , 0);
        }
        else
        {
            SetDsEthRmep(V, ccmSeqNum_f      , &ds_eth_rmep , 0);
        }
    }

    if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE))
    {
        SetDsEthRmep(V, mepId_f          , &ds_eth_rmep , p_sys_eth_rmep->rmep_id);
    }
    else
    {
        SetDsEthRmep(V, mepIndex_f       , &ds_eth_rmep , p_sys_rmep->p_sys_lmep->lmep_index);
    }

    SYS_USW_SET_HW_MAC(mac_sa, p_eth_rmep->rmep_mac);
    SetDsEthRmep(A, rmepMacSa_f            , &ds_eth_rmep , mac_sa);

    SetDsEthRmep(V, isRemote_f             , &ds_eth_rmep , 1);
    SetDsEthRmep(V, rmepWhile_f            , &ds_eth_rmep , 14);
    SetDsEthRmep(V, dUnexpPeriodTimer_f    , &ds_eth_rmep , 14);
    SetDsEthRmep(V, rmepLastPortStatus_f   , &ds_eth_rmep , 2);
    SetDsEthRmep(V, rmepLastIntfStatus_f   , &ds_eth_rmep , 1);
    SetDsEthRmep(V, cntShiftWhile_f        , &ds_eth_rmep , 4);

    SetDsEthRmep(V, isBfd_f                , &ds_eth_rmep , 0);
    SetDsEthRmep(V, isUp_f                 , &ds_eth_rmep , 0);
    SetDsEthRmep(V, mepType_f              , &ds_eth_rmep , 0);
    /*wangt SetDsEthRmep(V, sfFailWhile_f          , &ds_eth_rmep , 0);*/
    SetDsEthRmep(V, seqNumFailCounter_f    , &ds_eth_rmep , 0);
    SetDsEthRmep(V, rmepLastRdi_f          , &ds_eth_rmep , 0);
    SetDsEthRmep(V, dUnexpPeriod_f         , &ds_eth_rmep , 0);
    SetDsEthRmep(V, expCcmNum_f            , &ds_eth_rmep , 0);

    SetDsEthRmep(V, macStatusChange_f      , &ds_eth_rmep , 0);
    SetDsEthRmep(V, rmepmacmismatch_f      , &ds_eth_rmep , 0);
    /*wangt SetDsEthRmep(V, sfState_f              , &ds_eth_rmep , 0);*/

    index = p_sys_rmep->rmep_index;
    cmd = DRV_IOW(DsEthRmep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_eth_rmep;
    tbl_entry.mask_entry = (uint32*)&DsEthRmep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));


    sal_memset(&ds_eth_mep, 0, sizeof(DsEthMep_m));
    sal_memset(&DsEthMep_mask, 0xFF, sizeof(DsEthMep_m));
    index = p_sys_lmep->lmep_index;
    cmd = DRV_IOR(DsEthMep_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_eth_mep));
    SetDsEthMep(V, unknownCcmNum_f, &ds_eth_mep , 0);
    SetDsEthMep(V, unknownCcmNum_f, &DsEthMep_mask , 0);
    cmd = DRV_IOW(DsEthMep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_eth_mep;
    tbl_entry.mask_entry = (uint32*)&DsEthMep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));

RETURN:
    return CTC_E_NONE;
}

int32
_sys_usw_oam_rmep_add_tp_y1731_to_asic(uint8 lchip, sys_oam_rmep_t* p_sys_rmep)
{
    DsEthRmep_m ds_eth_rmep;
    DsEthRmep_m DsEthRmep_mask;
    tbl_entry_t tbl_entry;
    sys_oam_lmep_t* p_sys_lmep = NULL;
    sys_oam_rmep_t* p_sys_tp_y1731_rmep = NULL;
    uint32 cmd          = 0;
    uint32 index        = 0;
    uint32 rmep_flag    = 0;
    uint32 lmep_flag    = 0;
    uint8  tx_csf_type  = 0;

    p_sys_lmep     = p_sys_rmep->p_sys_lmep;
    p_sys_tp_y1731_rmep = (sys_oam_rmep_t*)p_sys_rmep;

    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
    {
        goto RETURN;
    }

    rmep_flag   = p_sys_tp_y1731_rmep->flag;

    lmep_flag   = p_sys_lmep->flag;
    tx_csf_type = p_sys_lmep->tx_csf_type;

    sal_memset(&ds_eth_rmep, 0, sizeof(DsEthRmep_m));
    sal_memset(&DsEthRmep_mask, 0, sizeof(DsEthRmep_m));

    SetDsEthRmep(V, active_f               , &ds_eth_rmep , CTC_FLAG_ISSET(rmep_flag, CTC_OAM_Y1731_RMEP_FLAG_MEP_EN));
    SetDsEthRmep(V, ethOamP2PMode_f        , &ds_eth_rmep , 1);
    SetDsEthRmep(V, seqNumEn_f             , &ds_eth_rmep , 0);
    SetDsEthRmep(V, macAddrUpdateDisable_f , &ds_eth_rmep , 0);
    SetDsEthRmep(V, apsSignalFailRemote_f  , &ds_eth_rmep , 0);
    SetDsEthRmep(V, mepType_f              , &ds_eth_rmep , 6);

    if (g_oam_master[lchip]->timer_update_disable)
    {
        SetDsEthRmep(V, firstPktRx_f, &ds_eth_rmep , 1);
    }
    else
    {
        SetDsEthRmep(V, firstPktRx_f, &ds_eth_rmep, 0);
    }
     SetDsEthRmep(V, dLoc_f                 , &ds_eth_rmep , 0);
    SetDsEthRmep(V, isCsfEn_f              , &ds_eth_rmep , CTC_FLAG_ISSET(rmep_flag, CTC_OAM_Y1731_RMEP_FLAG_CSF_EN));
    if (CTC_FLAG_ISSET(rmep_flag, CTC_OAM_Y1731_RMEP_FLAG_CSF_EN))
    {
        SetDsEthRmep(V, firstPktRx_f           , &ds_eth_rmep , 1);
        SetDsEthRmep(V, dLoc_f                 , &ds_eth_rmep , 1);
    }
    if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_Y1731_LMEP_FLAG_TX_CSF_EN))
    {
        SetDsEthRmep(V, isCsfTxEn_f      , &ds_eth_rmep , 1);
        SetDsEthRmep(V, rxCsfType_f      , &ds_eth_rmep , tx_csf_type);
        SetDsEthRmep(V, dCsf_f           , &ds_eth_rmep , 0);
        SetDsEthRmep(V, rxCsfWhile_f     , &ds_eth_rmep , 0);
        SetDsEthRmep(V, isCsfUseUcDa_f   , &ds_eth_rmep , 0);
    }

    SetDsEthRmep(V, mepId_f          , &ds_eth_rmep , p_sys_tp_y1731_rmep->rmep_id);

    SetDsEthRmep(V, isRemote_f             , &ds_eth_rmep , 1);
    SetDsEthRmep(V, rmepWhile_f            , &ds_eth_rmep , 14);
    SetDsEthRmep(V, dUnexpPeriodTimer_f    , &ds_eth_rmep , 14);
    SetDsEthRmep(V, rmepLastPortStatus_f   , &ds_eth_rmep , 2);
    SetDsEthRmep(V, rmepLastIntfStatus_f   , &ds_eth_rmep , 1);
    SetDsEthRmep(V, cntShiftWhile_f        , &ds_eth_rmep , 4);

    SetDsEthRmep(V, isBfd_f                , &ds_eth_rmep , 0);
    SetDsEthRmep(V, isUp_f                 , &ds_eth_rmep , 0);

    /*wangt SetDsEthRmep(V, sfFailWhile_f          , &ds_eth_rmep , 0);*/
    SetDsEthRmep(V, seqNumFailCounter_f    , &ds_eth_rmep , 0);
    SetDsEthRmep(V, rmepLastRdi_f          , &ds_eth_rmep , 0);
    SetDsEthRmep(V, dUnexpPeriod_f         , &ds_eth_rmep , 0);
    SetDsEthRmep(V, expCcmNum_f            , &ds_eth_rmep , 0);

    SetDsEthRmep(V, macStatusChange_f      , &ds_eth_rmep , 0);
    SetDsEthRmep(V, rmepmacmismatch_f      , &ds_eth_rmep , 0);
    /*wangt SetDsEthRmep(V, sfState_f              , &ds_eth_rmep , 0);*/

    index = p_sys_rmep->rmep_index;
    cmd = DRV_IOW(DsEthRmep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_eth_rmep;
    tbl_entry.mask_entry = (uint32*)&DsEthRmep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));
    RETURN:

    return CTC_E_NONE;
}


int32
_sys_usw_oam_rmep_del_from_asic(uint8 lchip, sys_oam_rmep_t* p_sys_rmep)
{
    uint32 cmd        = 0;
    uint32 index      = 0;

    DsEthRmep_m ds_eth_rmep;
    DsEthRmep_m DsEthRmep_mask;
    tbl_entry_t tbl_entry;

    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_rmep->p_sys_lmep->p_sys_chan->master_chipid))
    {
        goto RETURN;
    }

    sal_memset(&ds_eth_rmep, 0, sizeof(ds_eth_rmep));
    sal_memset(&DsEthRmep_mask, 0, sizeof(ds_eth_rmep));

    index = p_sys_rmep->rmep_index;
    cmd = DRV_IOW(DsEthRmep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_eth_rmep;
    tbl_entry.mask_entry = (uint32*)&DsEthRmep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));
RETURN:

    return CTC_E_NONE;
}

int32
_sys_usw_oam_rmep_update_eth_asic(uint8 lchip, sys_oam_rmep_t* p_sys_rmep, ctc_oam_update_t* p_rmep_param)
{
    sys_oam_rmep_t* p_sys_rmep_eth  = p_sys_rmep;
    sys_oam_lmep_t* p_sys_lmep = NULL;
    uint32 cmd        = 0;
    uint32 index      = 0;
    uint32 value = 0;
    uint32 tx_csf_en = 0;
    DsEthRmep_m ds_eth_rmep;
    DsEthRmep_m DsEthRmep_mask;
    tbl_entry_t tbl_entry;
    hw_mac_addr_t hw_mac_sa;

    p_sys_lmep = p_sys_rmep->p_sys_lmep;

    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
    {
        goto error;
    }

    index = p_sys_rmep->rmep_index;

    sal_memset(&ds_eth_rmep, 0, sizeof(ds_eth_rmep));
    sal_memset(&DsEthRmep_mask, 0xFF, sizeof(ds_eth_rmep));
    cmd = DRV_IOR(DsEthRmep_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_eth_rmep));

    switch (p_rmep_param->update_type)
    {
    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_MEP_EN:

        if (CTC_FLAG_ISSET(p_sys_rmep_eth->flag, CTC_OAM_Y1731_RMEP_FLAG_MEP_EN))
        {
            SetDsEthRmep(V, active_f             , &ds_eth_rmep , 1);
            SetDsEthRmep(V, isRemote_f             , &ds_eth_rmep , 1);
            SetDsEthRmep(V, rmepWhile_f            , &ds_eth_rmep , 14);
            SetDsEthRmep(V, dUnexpPeriodTimer_f    , &ds_eth_rmep , 14);
            SetDsEthRmep(V, rmepLastPortStatus_f   , &ds_eth_rmep , 2);
            SetDsEthRmep(V, rmepLastIntfStatus_f   , &ds_eth_rmep , 1);
            SetDsEthRmep(V, cntShiftWhile_f        , &ds_eth_rmep , 4);
            SetDsEthRmep(V, expCcmNum_f            , &ds_eth_rmep , 0);
            SetDsEthRmep(V, dLoc_f                 , &ds_eth_rmep , 0);
            SetDsEthRmep(V, dUnexpPeriod_f         , &ds_eth_rmep , 0);
            SetDsEthRmep(V, rmepLastRdi_f          , &ds_eth_rmep , 0);
            SetDsEthRmep(V, seqNumFailCounter_f    , &ds_eth_rmep , 0);
            /*wangt SetDsEthRmep(V, sfFailWhile_f          , &ds_eth_rmep , 0);*/
            SetDsEthRmep(V, macStatusChange_f      , &ds_eth_rmep , 0);
            SetDsEthRmep(V, rmepmacmismatch_f      , &ds_eth_rmep , 0);

        }
        else
        {
            SetDsEthRmep(V, active_f             , &ds_eth_rmep , 0);
        }
        SetDsEthRmep(V, active_f             , &DsEthRmep_mask , 0);
        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_CCM_SEQ_NUM_EN:
        cmd = DRV_IOR(DsEthMep_t, DsEthMep_ccmStatEn_f);
        DRV_IOCTL(lchip, p_sys_lmep->lmep_index, cmd, &value);
        if(value)
        {
            return CTC_E_INVALID_CONFIG;
        }
        SetDsEthRmep(V, seqNumEn_f             , &ds_eth_rmep , CTC_FLAG_ISSET(p_sys_rmep_eth->flag, CTC_OAM_Y1731_RMEP_FLAG_CCM_SEQ_NUM_EN));
        SetDsEthRmep(V, seqNumEn_f             , &DsEthRmep_mask , 0);
        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_SEQ_FAIL_CLEAR:
        SetDsEthRmep(V, seqNumFailCounter_f             , &ds_eth_rmep , 0);
        SetDsEthRmep(V, seqNumFailCounter_f             , &DsEthRmep_mask , 0);
        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_CSF_EN:
        cmd = DRV_IOR(DsEthMep_t, DsEthMep_cciEn_f);
        DRV_IOCTL(lchip, p_sys_lmep->lmep_index, cmd, &value);
        if (value)
        {
            return CTC_E_INVALID_CONFIG;
        }

        tx_csf_en = GetDsEthRmep(V, isCsfTxEn_f, &ds_eth_rmep);
        if ((tx_csf_en)&&(!p_rmep_param->update_value))
        {
            p_rmep_param->update_value = 1;
        }

        SetDsEthRmep(V, isCsfEn_f             , &ds_eth_rmep , p_rmep_param->update_value);
        SetDsEthRmep(V, isCsfEn_f             , &DsEthRmep_mask , 0);
        SetDsEthRmep(V, firstPktRx_f           , &ds_eth_rmep , 1);
        SetDsEthRmep(V, firstPktRx_f           , &DsEthRmep_mask ,  0);
        SetDsEthRmep(V, dLoc_f                 , &ds_eth_rmep , 1);
        SetDsEthRmep(V, dLoc_f           , &DsEthRmep_mask ,  0);
        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_DCSF:
        SetDsEthRmep(V, dCsf_f, &ds_eth_rmep, p_rmep_param->update_value);
        SetDsEthRmep(V, dCsf_f, &DsEthRmep_mask, 0);
        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_RMEP_SF_STATE:
        return CTC_E_NOT_SUPPORT;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_RMAC_MIS:
        SetDsEthRmep(V, rmepmacmismatch_f, &ds_eth_rmep, p_rmep_param->update_value);
        SetDsEthRmep(V, rmepmacmismatch_f, &DsEthRmep_mask, 0);
        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_PORT_INTF_MIS:
        SetDsEthRmep(V, macStatusChange_f, &ds_eth_rmep ,  p_rmep_param->update_value);
        SetDsEthRmep(V, macStatusChange_f, &DsEthRmep_mask ,  0);
        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_HW_APS:
        {
            ctc_oam_hw_aps_t* p_hw_aps = NULL;
            p_hw_aps = (ctc_oam_hw_aps_t*)p_rmep_param->p_update_value;
            SetDsEthRmep(V, apsGroupId_f, &ds_eth_rmep ,  p_hw_aps->aps_group_id);
            SetDsEthRmep(V, apsGroupId_f, &DsEthRmep_mask ,  0);
            SetDsEthRmep(V, protectionPath_f, &ds_eth_rmep, (p_hw_aps->protection_path) ? 1 : 0);
            SetDsEthRmep(V, protectionPath_f, &DsEthRmep_mask , 0);
        }
        break;
    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_HW_APS_EN:
        SetDsEthRmep(V, fastApsEn_f, &ds_eth_rmep , (p_rmep_param->update_value) ? 1: 0);
        SetDsEthRmep(V, fastApsEn_f, &DsEthRmep_mask , 0);
        break;
    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_RMEP_MACSA:
        SYS_USW_SET_HW_MAC(hw_mac_sa, (uint8*)(p_rmep_param->p_update_value));
        SetDsEthRmep(A, rmepMacSa_f, &ds_eth_rmep , hw_mac_sa);
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOW(DsEthRmep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_eth_rmep;
    tbl_entry.mask_entry = (uint32*)&DsEthRmep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));
error:

    return CTC_E_NONE;

}

int32
_sys_usw_oam_rmep_update_tp_y1731_asic(uint8 lchip, sys_oam_rmep_t* p_sys_rmep, ctc_oam_update_t* p_rmep_param)
{
    sys_oam_rmep_t* p_sys_rmep_tp_y1731  = p_sys_rmep;
    sys_oam_lmep_t* p_sys_lmep = NULL;
    uint32 cmd        = 0;
    uint32 index      = 0;
    uint32 value      = 0;
    uint32 tx_csf_en  = 0;
    DsEthRmep_m ds_eth_rmep;
    DsEthRmep_m DsEthRmep_mask;
    tbl_entry_t tbl_entry;

    p_sys_lmep = p_sys_rmep->p_sys_lmep;

    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
    {
        goto error;
    }

    index = p_sys_rmep->rmep_index;

    sal_memset(&ds_eth_rmep, 0, sizeof(DsEthRmep_m));
    sal_memset(&DsEthRmep_mask, 0xFF, sizeof(DsEthRmep_m));
    cmd = DRV_IOR(DsEthRmep_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_eth_rmep));

    switch (p_rmep_param->update_type)
    {
    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_MEP_EN:

        if (CTC_FLAG_ISSET(p_sys_rmep_tp_y1731->flag, CTC_OAM_Y1731_RMEP_FLAG_MEP_EN))
        {
            SetDsEthRmep(V, active_f             , &ds_eth_rmep , 1);
            SetDsEthRmep(V, rmepWhile_f            , &ds_eth_rmep , 14);
            SetDsEthRmep(V, dUnexpPeriodTimer_f    , &ds_eth_rmep , 14);
            SetDsEthRmep(V, rmepLastPortStatus_f   , &ds_eth_rmep , 2);
            SetDsEthRmep(V, rmepLastIntfStatus_f   , &ds_eth_rmep , 1);
            SetDsEthRmep(V, cntShiftWhile_f        , &ds_eth_rmep , 4);
            SetDsEthRmep(V, expCcmNum_f            , &ds_eth_rmep , 0);
            SetDsEthRmep(V, dLoc_f                 , &ds_eth_rmep , 0);
            SetDsEthRmep(V, dUnexpPeriod_f         , &ds_eth_rmep , 0);
            SetDsEthRmep(V, rmepLastRdi_f          , &ds_eth_rmep , 0);
            SetDsEthRmep(V, seqNumFailCounter_f    , &ds_eth_rmep , 0);
            /*wangt SetDsEthRmep(V, sfFailWhile_f          , &ds_eth_rmep , 0);*/
            SetDsEthRmep(V, macStatusChange_f      , &ds_eth_rmep , 0);
            SetDsEthRmep(V, rmepmacmismatch_f      , &ds_eth_rmep , 0);

        }
        else
        {
            SetDsEthRmep(V, active_f             , &ds_eth_rmep , 0);
        }
        SetDsEthRmep(V, active_f             , &DsEthRmep_mask , 0);
        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_CSF_EN:
        cmd = DRV_IOR(DsEthMep_t, DsEthMep_cciEn_f);
        DRV_IOCTL(lchip, p_sys_lmep->lmep_index, cmd, &value);
        if (value)
        {
            return CTC_E_INVALID_CONFIG;
        }

        tx_csf_en = GetDsEthRmep(V, isCsfTxEn_f, &ds_eth_rmep);
        if ((tx_csf_en)&&(!p_rmep_param->update_value))
        {
            p_rmep_param->update_value = 1;
        }

        SetDsEthRmep(V, isCsfEn_f, &ds_eth_rmep , p_rmep_param->update_value);
        SetDsEthRmep(V, isCsfEn_f, &DsEthRmep_mask , 0);
        SetDsEthRmep(V, firstPktRx_f, &ds_eth_rmep , 1);
        SetDsEthRmep(V, firstPktRx_f, &DsEthRmep_mask ,  0);
        SetDsEthRmep(V, dLoc_f, &ds_eth_rmep , 1);
        SetDsEthRmep(V, dLoc_f, &DsEthRmep_mask ,  0);
        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_DCSF:
        SetDsEthRmep(V, dCsf_f           , &ds_eth_rmep ,  p_rmep_param->update_value);
        SetDsEthRmep(V, dCsf_f           , &DsEthRmep_mask ,  0);
        break;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_RMEP_SF_STATE:
        return CTC_E_NOT_SUPPORT;

    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_HW_APS:
        {
            ctc_oam_hw_aps_t* p_hw_aps = NULL;
            p_hw_aps = (ctc_oam_hw_aps_t*)p_rmep_param->p_update_value;
            SetDsEthRmep(V, apsGroupId_f, &ds_eth_rmep ,  p_hw_aps->aps_group_id);
            SetDsEthRmep(V, apsGroupId_f, &DsEthRmep_mask ,  0);
            SetDsEthRmep(V, protectionPath_f, &ds_eth_rmep, (p_hw_aps->protection_path) ? 1 : 0);
            SetDsEthRmep(V, protectionPath_f, &DsEthRmep_mask , 0);
        }
        break;
    case CTC_OAM_Y1731_RMEP_UPDATE_TYPE_HW_APS_EN:
        SetDsEthRmep(V, fastApsEn_f, &ds_eth_rmep , p_rmep_param->update_value);
        SetDsEthRmep(V, fastApsEn_f, &DsEthRmep_mask , 0);
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOW(DsEthRmep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_eth_rmep;
    tbl_entry.mask_entry = (uint32*)&DsEthRmep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));
error:
    return CTC_E_NONE;
}

#define OAM_STATS "Function Begin"
int32
sys_usw_oam_get_session_type(uint8 lchip, sys_oam_lmep_t *p_oam_lmep, uint32 *session_type)
{
    uint32 mep_type = 0;

    sys_oam_lmep_t* p_sys_lmep_y1731  = NULL;

    mep_type = p_oam_lmep->p_sys_chan->mep_type;

    if ((CTC_OAM_MEP_TYPE_ETH_1AG == mep_type)
        || (CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_type))
    {
        p_sys_lmep_y1731 = (sys_oam_lmep_t*)p_oam_lmep;
        if (CTC_FLAG_ISSET(p_sys_lmep_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU))
        {
            *session_type = SYS_OAM_SESSION_MEP_ON_CPU;
        }
        else if(CTC_FLAG_ISSET(p_sys_lmep_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE))
        {
            *session_type = SYS_OAM_SESSION_Y1731_ETH_P2P;
        }
        else
        {
            *session_type = SYS_OAM_SESSION_Y1731_ETH_P2MP;
        }
    }
    else if (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == mep_type)
    {
        p_sys_lmep_y1731 = (sys_oam_lmep_t*)p_oam_lmep;
        if (CTC_FLAG_ISSET(p_sys_lmep_y1731->flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU))
        {
            *session_type = SYS_OAM_SESSION_MEP_ON_CPU;
        }
        else if(((sys_oam_chan_tp_t*)(p_oam_lmep->p_sys_chan))->key.section_oam)
        {
            *session_type = SYS_OAM_SESSION_Y1731_TP_SECTION;
        }
        else
        {
            *session_type = SYS_OAM_SESSION_Y1731_TP_MPLS;
        }

    }
    else if(CTC_OAM_MEP_TYPE_IP_BFD == mep_type)
    {
        if (CTC_FLAG_ISSET(p_oam_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_MEP_ON_CPU))
        {
            *session_type = SYS_OAM_SESSION_MEP_ON_CPU;
        }
        else if (CTC_FLAG_ISSET(p_oam_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_IPV4_ENCAP))
        {
            *session_type = SYS_OAM_SESSION_BFD_IPv4;
        }
        else if (CTC_FLAG_ISSET(p_oam_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_IPV6_ENCAP))
        {
            *session_type = SYS_OAM_SESSION_BFD_IPv6;
        }
    }
    else if(CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
    {
        if (CTC_FLAG_ISSET(p_oam_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_MEP_ON_CPU))
        {
            *session_type = SYS_OAM_SESSION_MEP_ON_CPU;
        }
        else if(((sys_oam_chan_tp_t*)(p_oam_lmep->p_sys_chan))->key.section_oam)
        {
            *session_type = SYS_OAM_SESSION_BFD_TP_SECTION;
        }
        else
        {
            *session_type = SYS_OAM_SESSION_BFD_TP_MPLS;
        }
    }
    else if (CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type)
    {
        if (CTC_FLAG_ISSET(p_oam_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_MEP_ON_CPU))
        {
            *session_type = SYS_OAM_SESSION_MEP_ON_CPU;
        }
        else
        {
            *session_type = SYS_OAM_SESSION_BFD_MPLS;
        }
    }
    else if (CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type)
    {
        if (CTC_FLAG_ISSET(p_oam_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_MEP_ON_CPU))
        {
            *session_type = SYS_OAM_SESSION_MEP_ON_CPU;
        }
        else
        {
            *session_type = SYS_OAM_SESSION_BFD_MICRO;
        }
    }


    return CTC_E_NONE;
}

int32
sys_usw_oam_get_session_num(uint8 lchip, uint32* used_session)
{
    uint8  type = 0;
    SYS_OAM_INIT_CHECK(lchip);
    for(type = SYS_OAM_SESSION_Y1731_ETH_P2P; type < SYS_OAM_SESSION_MAX; type++)
    {
        *used_session += g_oam_master[lchip]->oam_session_num[type];
    }
    return CTC_E_NONE;
}

int32
sys_usw_oam_ftm_cb(uint8 lchip, sys_ftm_specs_info_t* specs_info)
{
    uint32 used_size = 0;
    CTC_PTR_VALID_CHECK(specs_info);
    SYS_OAM_INIT_CHECK(lchip);

    CTC_ERROR_RETURN(sys_usw_oam_get_session_num(lchip, &used_size));
    specs_info->used_size = used_size;

    return CTC_E_NONE;
}


#define COM "Function Begin"
int32
_sys_usw_oam_update_vpws_key(uint8 lchip, uint32 gport, uint16 oamid, sys_oam_chan_tp_t* p_sys_chan_tp, bool is_add)
{
    uint32 key_index = 0;
    sys_oam_id_t* p_sys_oamid = NULL;
    DsEgressXcOamEthHashKey_m eth_oam_key;

    p_sys_oamid = _sys_usw_oam_oamid_lkup(lchip, gport, oamid);
    if (!p_sys_oamid || !p_sys_oamid->p_sys_chan_eth || !p_sys_oamid->p_sys_chan_eth->key.com.key_exit)
    {
        return CTC_E_NONE;
    }
    if (is_add)
    {
        CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, DsEgressXcOamEthHashKey_t, p_sys_oamid->p_sys_chan_eth->key.com.key_index, &eth_oam_key));
		/*need alloc additional key*/
        SetDsEgressXcOamEthHashKey(V, vlanId_f, &eth_oam_key, (p_sys_chan_tp->mep_index_in_key + 8192));
        CTC_ERROR_RETURN(sys_usw_oam_key_lookup_io(lchip, DsEgressXcOamEthHashKey_t, &key_index, &eth_oam_key));
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 1, 1);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "alloc additional DsEgressXcOamEthHashKey, key_index:%d \n", key_index);
        CTC_ERROR_RETURN(sys_usw_oam_key_write_io(lchip, DsEgressXcOamEthHashKey_t, key_index, &eth_oam_key));
        if (!p_sys_oamid->p_sys_chan_eth->tp_oam_key_valid[0])
        {
            p_sys_oamid->p_sys_chan_eth->tp_oam_key_index[0] = key_index;
            p_sys_oamid->p_sys_chan_eth->tp_oam_key_valid[0] = 1;
        }
        else
        {
            p_sys_oamid->p_sys_chan_eth->tp_oam_key_index[1] = key_index;
            p_sys_oamid->p_sys_chan_eth->tp_oam_key_valid[1] = 1;
        }
    }
    else
    {
        if (p_sys_oamid->p_sys_chan_eth->tp_oam_key_valid[0] && (p_sys_oamid->label[0] == p_sys_chan_tp->key.label))
        {
            sys_usw_oam_key_delete_io(lchip, DsEgressXcOamEthHashKey_t, p_sys_oamid->p_sys_chan_eth->tp_oam_key_index[0], &eth_oam_key);
            p_sys_oamid->p_sys_chan_eth->tp_oam_key_valid[0] = 0;
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 0, 1);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free additional DsEgressXcOamEthHashKey, key_index:%d \n", p_sys_oamid->p_sys_chan_eth->tp_oam_key_index[0]);
        }
        if (p_sys_oamid->p_sys_chan_eth->tp_oam_key_valid[1] && (p_sys_oamid->label[1] == p_sys_chan_tp->key.label))
        {
            sys_usw_oam_key_delete_io(lchip, DsEgressXcOamEthHashKey_t, p_sys_oamid->p_sys_chan_eth->tp_oam_key_index[1], &eth_oam_key);
            p_sys_oamid->p_sys_chan_eth->tp_oam_key_valid[1] = 0;
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 0, 1);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free additional DsEgressXcOamEthHashKey, key_index:%d \n", p_sys_oamid->p_sys_chan_eth->tp_oam_key_index[1]);
        }
    }
    return CTC_E_NONE;
}

int32
_sys_usw_oam_register_init(uint8 lchip, ctc_oam_global_t* p_oam_glb)
{
    CTC_PTR_VALID_CHECK(p_oam_glb);
    if (NULL == g_oam_master[lchip])
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(_sys_usw_cfm_register_init(lchip));
    CTC_ERROR_RETURN(_sys_usw_bfd_register_init(lchip, p_oam_glb));

    if (_sys_usw_oam_get_mpls_entry_num(lchip))
    {
        CTC_ERROR_RETURN(_sys_usw_tp_y1731_register_init(lchip, p_oam_glb));
    }

    return CTC_E_NONE;
}

int32
_sys_usw_bfd_update_nh_process(uint8 lchip, uint32 nhid, mac_addr_t mac)
{
    int32  ret = CTC_E_NONE;
    uint16 l3if_id = 0;
    sys_l3if_prop_t l3if_prop;
    sys_nh_info_dsnh_t nhinfo;
    ctc_ip_nh_param_t ipuc_nh_param;
    ctc_nh_nexthop_mac_param_t p_new_param;
    sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));
    sal_memset(&ipuc_nh_param, 0, sizeof(ctc_ip_nh_param_t));
    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, nhid, &nhinfo, 0));

    sal_memset(&l3if_prop, 0, sizeof(sys_l3if_prop_t));

    ret = sys_usw_nh_get_l3ifid(lchip, nhid, &l3if_id);
    if (ret < 0)
    {
        goto error;
    }

    l3if_prop.l3if_id = l3if_id;
    ret = sys_usw_l3if_get_l3if_info(lchip, 1, &l3if_prop);/*query_type = 1, get l3if info by l3if_id*/
    if (ret < 0)
    {
        goto error;
    }

    if (nhinfo.arp_id)
    {
        sal_memset(&p_new_param, 0, sizeof(ctc_nh_nexthop_mac_param_t));
        if ((l3if_prop.l3if_type == CTC_L3IF_TYPE_VLAN_IF) || (l3if_prop.l3if_type == CTC_L3IF_TYPE_SUB_IF))
        {
            CTC_SET_FLAG(p_new_param.flag, CTC_NH_NEXTHOP_MAC_VLAN_VALID);
            p_new_param.vlan_id = l3if_prop.vlan_id;
        }
        p_new_param.gport = nhinfo.gport;
        sal_memcpy(&p_new_param.mac, mac, sizeof(mac_addr_t));
        ret = sys_usw_nh_update_nexthop_mac(lchip, nhinfo.arp_id, &p_new_param);
        if (ret < 0)
        {
            goto error;
        }
    }
    else
    {
        ipuc_nh_param.oif.gport = nhinfo.gport;
        if ((l3if_prop.l3if_type == CTC_L3IF_TYPE_VLAN_IF) || (l3if_prop.l3if_type == CTC_L3IF_TYPE_SUB_IF))
        {
            ipuc_nh_param.oif.vid = l3if_prop.vlan_id;
        }

        sal_memcpy(&ipuc_nh_param.mac, mac, sizeof(mac_addr_t));
        ret = sys_usw_ipuc_nh_update(lchip, nhid, &ipuc_nh_param, SYS_NH_CHANGE_TYPE_UPDATE_FWD_ATTR);
        if (ret < 0)
        {
            goto error;
        }
    }

error:
    if (ret < 0)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "MicroBFD update nexthop fail!!\n");
    }
    return ret;
}

int32
_sys_usw_oam_defect_read_defect_status(uint8 lchip,
                                             void* p_defect)
{
    uint32 cmd = 0;
    uint32 cache_entry_valid = 0;
    uint8 cache_entry_idx  = 0;
    uint8 valid_cache_num = 0;
    uint32 defect_status_index = 0;
    uint32 defect_status = 0;
    OamErrorDefectCtl_m oam_error_defect_ctl;
    DsOamDefectStatus_m ds_oam_defect_status;
    OamDefectCache_m oam_defect_cache;
    ctc_oam_defect_info_t* p_defect_info = NULL;

    CTC_PTR_VALID_CHECK(p_defect);

    p_defect_info = (ctc_oam_defect_info_t*)p_defect;

    sal_memset(&oam_error_defect_ctl, 0, sizeof(OamErrorDefectCtl_m));
    cmd = DRV_IOR(OamErrorDefectCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_error_defect_ctl));

    DRV_GET_FIELD_A(lchip, OamErrorDefectCtl_t,OamErrorDefectCtl_cacheEntryValid_f,&oam_error_defect_ctl,&cache_entry_valid);

    if (0 == cache_entry_valid)
    {
        return CTC_E_NONE;
    }

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"cache_entry_valid= 0x%x\n", cache_entry_valid);

    for (cache_entry_idx = 0; cache_entry_idx < CTC_OAM_MAX_ERROR_CACHE_NUM; cache_entry_idx++)
    {
        if (IS_BIT_SET(cache_entry_valid, cache_entry_idx))
        {
            sal_memset(&oam_defect_cache, 0, sizeof(oam_defect_cache));
            cmd = DRV_IOR(OamDefectCache_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, cache_entry_idx, cmd, &oam_defect_cache));
            DRV_GET_FIELD_A(lchip, OamDefectCache_t,OamDefectCache_scanPtr_f,&oam_defect_cache,&defect_status_index);

            sal_memset(&ds_oam_defect_status, 0, sizeof(ds_oam_defect_status));
            cmd = DRV_IOR(DsOamDefectStatus_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, defect_status_index, cmd, &ds_oam_defect_status));

            p_defect_info->mep_index_base[valid_cache_num] = defect_status_index;
            DRV_GET_FIELD_A(lchip, DsOamDefectStatus_t,DsOamDefectStatus_defectStatus_f,&ds_oam_defect_status,&defect_status);
            p_defect_info->mep_index_bitmap[valid_cache_num++] = defect_status;
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"defect_status_index:%d, defect_status:0x%x\n", defect_status_index, defect_status);
            /*Clear after read the status */
#if (SDK_WORK_PLATFORM == 1)
            /*wangt if (!drv_io_is_asic())*/
            {
                 /*ds_oam_defect_status.defect_status = 0;*/
                DRV_SET_FIELD_V(lchip, DsOamDefectStatus_t,DsOamDefectStatus_defectStatus_f,&ds_oam_defect_status,0);
            }
#endif
            cmd = DRV_IOW(DsOamDefectStatus_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, defect_status_index, cmd, &ds_oam_defect_status));
        }
    }


#if (SDK_WORK_PLATFORM == 1)
    /*wangt if (!drv_io_is_asic())*/
    {
        cache_entry_valid = 0;
    }
#endif
    cmd = DRV_IOW(OamErrorDefectCtl_t, OamErrorDefectCtl_cacheEntryValid_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cache_entry_valid));

    p_defect_info->valid_cache_num = valid_cache_num;

    return CTC_E_NONE;
}

int32
_sys_usw_oam_defect_scan_en(uint8 lchip, bool enable)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 field_report[2] = {0};
    uint32 i = 0;
    DsOamDefectStatus_m ds_oam_defect_status;
    OamErrorDefectCtl_m oam_error_defect_ctl;

    sal_memset(&ds_oam_defect_status, 0xFF, sizeof(DsOamDefectStatus_m));
    sal_memset(&oam_error_defect_ctl, 0, sizeof(OamErrorDefectCtl_m));
#if (SDK_WORK_PLATFORM == 1)
    DRV_SET_FIELD_V(lchip, DsOamDefectStatus_t,DsOamDefectStatus_defectStatus_f,&ds_oam_defect_status,0);
#endif
    cmd = DRV_IOW(DsOamDefectStatus_t, DRV_ENTRY_FLAG);
    for (i = 0; i < 256; i++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, &ds_oam_defect_status));
    }

    field_val = 0;
    cmd = DRV_IOW(OamErrorDefectCtl_t, OamErrorDefectCtl_minPtr_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = 0xFF;
    cmd = DRV_IOW(OamErrorDefectCtl_t, OamErrorDefectCtl_maxPtr_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_report[0]=0xFFFFFFFF;
    field_report[1]=0xFFFFFFFF;

    cmd = DRV_IOR(OamErrorDefectCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_error_defect_ctl));
    DRV_SET_FIELD_A(lchip, OamErrorDefectCtl_t,OamErrorDefectCtl_reportDefectEn_f,&oam_error_defect_ctl,field_report);

    cmd = DRV_IOW(OamErrorDefectCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_error_defect_ctl));

    field_val = 4;
    cmd = DRV_IOW(OamErrorDefectCtl_t, OamErrorDefectCtl_scanDefectInterval_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));


    field_val = enable ? 1 : 0;
    cmd = DRV_IOW(OamErrorDefectCtl_t, OamErrorDefectCtl_scanEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

int32
_sys_usw_oam_get_stats_info(uint8 lchip, sys_oam_lm_com_t* p_sys_oam_lm, ctc_oam_stats_info_t* p_stat_info)
{

    DsOamLmStats_m ds_oam_lm_stats;
    uint8 cnt = 0;
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(p_sys_oam_lm);
    CTC_PTR_VALID_CHECK(p_stat_info);

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"p_sys_oam_lm->lm_cos_type:%d, lm_type:%d, lm_index:%d\n",
       p_sys_oam_lm->lm_cos_type, p_sys_oam_lm->lm_type, p_sys_oam_lm->lm_index);

    p_stat_info->lm_cos_type    = p_sys_oam_lm->lm_cos_type;
    p_stat_info->lm_type        = p_sys_oam_lm->lm_type;
    if ((CTC_OAM_LM_COS_TYPE_ALL_COS == p_sys_oam_lm->lm_cos_type)
        || (CTC_OAM_LM_COS_TYPE_SPECIFIED_COS == p_sys_oam_lm->lm_cos_type))
    {
        sal_memset(&ds_oam_lm_stats, 0, sizeof(DsOamLmStats_m));
        cmd = DRV_IOR(DsOamLmStats_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_oam_lm->lm_index, cmd, &ds_oam_lm_stats));

        p_stat_info->lm_info[0].rx_fcb = GetDsOamLmStats(V, rxFcb_r_f, &ds_oam_lm_stats);
        p_stat_info->lm_info[0].rx_fcl = GetDsOamLmStats(V, rxFcl_r_f, &ds_oam_lm_stats);
        p_stat_info->lm_info[0].tx_fcb = GetDsOamLmStats(V, txFcb_r_f, &ds_oam_lm_stats);
        p_stat_info->lm_info[0].tx_fcf = GetDsOamLmStats(V, txFcf_r_f, &ds_oam_lm_stats);
    }
    else
    {
        for (cnt = 0; cnt < CTC_OAM_STATS_COS_NUM; cnt++)
        {
            sal_memset(&ds_oam_lm_stats, 0, sizeof(DsOamLmStats_m));
            cmd = DRV_IOR(DsOamLmStats_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, (p_sys_oam_lm->lm_index + cnt), cmd, &ds_oam_lm_stats));

            p_stat_info->lm_info[cnt].rx_fcb = GetDsOamLmStats(V, rxFcb_r_f, &ds_oam_lm_stats);
            p_stat_info->lm_info[cnt].rx_fcl = GetDsOamLmStats(V, rxFcl_r_f, &ds_oam_lm_stats);
            p_stat_info->lm_info[cnt].tx_fcb = GetDsOamLmStats(V, txFcb_r_f, &ds_oam_lm_stats);
            p_stat_info->lm_info[cnt].tx_fcf = GetDsOamLmStats(V, txFcf_r_f, &ds_oam_lm_stats);
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_oam_get_defect_info(uint8 lchip, void* p_defect_info)
{
    int32 ret      = CTC_E_NONE;
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_defect_info);
    ret = _sys_usw_oam_defect_read_defect_status(lchip, p_defect_info);
    return ret;
}

int32
_sys_usw_oam_get_mep_info(uint8 lchip, ctc_oam_mep_info_t* mep_info)
{
    uint32 lmep_index   = 0;
    uint32 cmd_eth_mep  = 0;
    uint32 cmd_eth_rmep = 0;
    uint32 cmd = 0;
    ctc_oam_mep_type_t mep_type = 0;
    DsEthMep_m   ds_eth_mep;
    DsEthRmep_m  ds_eth_rmep;
    DsBfdMep_m   ds_bfd_mep;
    DsBfdRmep_m  ds_bfd_rmep;
    DsBfdSlowIntervalCfg_m ds_bfd_slow_interval_cfg;
    DsApsBridge_m ds_aps_bridge;
    DsMa_m ds_ma;
    sys_oam_lmep_t* p_sys_lmep_com  = NULL;
    sys_oam_rmep_t* p_sys_rmep_com  = NULL;
    sys_oam_rmep_t* p_sys_rmep_y1731  = NULL;
    ctc_oam_y1731_lmep_info_t*  p_lmep_y1731_info = NULL;
    ctc_oam_y1731_rmep_info_t*  p_rmep_y1731_info = NULL;
    ctc_oam_bfd_lmep_info_t*    p_lmep_bfd_info = NULL;
    ctc_oam_bfd_rmep_info_t*    p_rmep_bfd_info = NULL;
    hw_mac_addr_t             mac_sa   = { 0 };
    uint16  ma_index = 0;
    uint8 is_remote = 0;
    uint32 ccm_interval = 0;
    uint32 aps_gr_id_high = 0;
    uint32 tmp[8] = {0, 33, 100, 1000, 10000, 100000, 600000, 6000000};
    SYS_OAM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(mep_info);

    sal_memset(&ds_eth_mep, 0, sizeof(DsEthMep_m));
    sal_memset(&ds_eth_rmep, 0, sizeof(DsEthRmep_m));
    sal_memset(&ds_bfd_mep, 0, sizeof(DsBfdMep_m));
    sal_memset(&ds_bfd_rmep, 0, sizeof(DsBfdRmep_m));
    sal_memset(&ds_bfd_slow_interval_cfg, 0, sizeof(DsBfdSlowIntervalCfg_m));
    sal_memset(&ds_aps_bridge, 0, sizeof(ds_aps_bridge));
    sal_memset(&ds_ma, 0, sizeof(DsMa_m));

    cmd_eth_mep     = DRV_IOR(DsEthMep_t,   DRV_ENTRY_FLAG);
    cmd_eth_rmep    = DRV_IOR(DsEthRmep_t,  DRV_ENTRY_FLAG);
    if (0x1FFF == mep_info->mep_index)
    {
        return CTC_E_NONE;
    }
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, mep_info->mep_index, cmd_eth_rmep, &ds_eth_rmep));

    mep_info->is_rmep = GetDsEthRmep(V, isRemote_f, &ds_eth_rmep);
    if (!GetDsEthRmep(V, active_f, &ds_eth_rmep))
    {
        return CTC_E_NOT_READY;
    }

    if (mep_info->is_rmep == 1)
    {
        p_sys_rmep_com = ctc_vector_get(g_oam_master[lchip]->mep_vec, mep_info->mep_index);

        if (p_sys_rmep_com)
        {
            mep_type = p_sys_rmep_com->p_sys_lmep->p_sys_chan->mep_type;
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"mep_type:%d\n", mep_type);
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }

        is_remote = 1;

        if ((CTC_OAM_MEP_TYPE_ETH_1AG == mep_type)
            || (CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_type)
            || (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == mep_type))
        {
            p_sys_rmep_y1731 = p_sys_rmep_com;
            p_rmep_y1731_info = &(mep_info->rmep.y1731_rmep);

            p_rmep_y1731_info->rmep_id = p_sys_rmep_y1731->rmep_id;

            GetDsEthRmep(A, rmepMacSa_f, &ds_eth_rmep, mac_sa);
            SYS_USW_SET_USER_MAC(p_rmep_y1731_info->mac_sa, mac_sa);
            p_rmep_y1731_info->active                  = GetDsEthRmep(V, active_f, &ds_eth_rmep);
            p_rmep_y1731_info->is_p2p_mode      = GetDsEthRmep(V, ethOamP2PMode_f, &ds_eth_rmep);
            p_rmep_y1731_info->first_pkt_rx        = GetDsEthRmep(V, firstPktRx_f, &ds_eth_rmep);
            p_rmep_y1731_info->d_loc                   = GetDsEthRmep(V, dLoc_f, &ds_eth_rmep);
            p_rmep_y1731_info->d_unexp_period   = GetDsEthRmep(V, dUnexpPeriod_f , &ds_eth_rmep);
            p_rmep_y1731_info->ma_sa_mismatch   = GetDsEthRmep(V, rmepmacmismatch_f , &ds_eth_rmep);
            p_rmep_y1731_info->mac_status_change = GetDsEthRmep(V, macStatusChange_f , &ds_eth_rmep);
            p_rmep_y1731_info->last_rdi         = GetDsEthRmep(V, rmepLastRdi_f , &ds_eth_rmep);
            p_rmep_y1731_info->lmep_index = p_rmep_y1731_info->is_p2p_mode ? (mep_info->mep_index - 1) :
                                                                       GetDsEthRmep(V, mepIndex_f  , &ds_eth_rmep);
            cmd = DRV_IOR(DsEthMep_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_rmep_y1731_info->lmep_index, cmd, &ds_eth_mep));
            ccm_interval = GetDsEthMep(V, ccmInterval_f  , &ds_eth_mep);
            if (ccm_interval < 8)
            {
                p_rmep_y1731_info->dloc_time = (p_rmep_y1731_info->d_loc) ? 0 :
                (GetDsEthRmep(V, rmepWhile_f , &ds_eth_rmep) * tmp[ccm_interval] / 40);
            }
            if (GetDsEthMep(V, ccmStatEn_f, &ds_eth_mep) && p_rmep_y1731_info->is_p2p_mode)
            {
                p_rmep_y1731_info->ccm_rx_pkts = GetDsEthRmep(V, ccmSeqNum_f , &ds_eth_rmep);
                p_rmep_y1731_info->err_ccm_rx_pkts = GetDsEthMep(V, oamDiscardCnt_f, &ds_eth_mep);
            }
            p_rmep_y1731_info->hw_aps_en= GetDsEthRmep(V, fastApsEn_f, &ds_eth_rmep);
            p_rmep_y1731_info->protection_path = GetDsEthRmep(V, protectionPath_f, &ds_eth_rmep);
            p_rmep_y1731_info->hw_aps_group_id = GetDsEthRmep(V, apsGroupId_f, &ds_eth_rmep);
            if (p_rmep_y1731_info->hw_aps_group_id)
            {
                cmd = DRV_IOR(DsApsBridge_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_rmep_y1731_info->hw_aps_group_id, cmd, &ds_aps_bridge));

                p_rmep_y1731_info->path_fail = (p_rmep_y1731_info->protection_path) ? GetDsApsBridge(V, protectionFail_f, &ds_aps_bridge) :
                                                GetDsApsBridge(V, workingFail_f, &ds_aps_bridge);
                if ((p_rmep_y1731_info->protection_path && GetDsApsBridge(V, protectingEn_f, &ds_aps_bridge))
                    || (!p_rmep_y1731_info->protection_path && !GetDsApsBridge(V, protectingEn_f, &ds_aps_bridge)))
                {
                    p_rmep_y1731_info->path_active = 1;
                }
            }
            p_rmep_y1731_info->last_intf_status = GetDsEthRmep(V, rmepLastIntfStatus_f, &ds_eth_rmep);
            p_rmep_y1731_info->last_port_status = GetDsEthRmep(V, rmepLastPortStatus_f, &ds_eth_rmep);
            p_rmep_y1731_info->last_seq_chk_en = GetDsEthRmep(V, seqNumEn_f, &ds_eth_rmep);
            p_rmep_y1731_info->seq_fail_count = GetDsEthRmep(V, seqNumFailCounter_f, &ds_eth_rmep);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"p_rmep_y1731_info->first_pkt_rx  :%d\n", p_rmep_y1731_info->first_pkt_rx );
            if (p_rmep_y1731_info->is_p2p_mode && GetDsEthRmep(V, isCsfEn_f, &ds_eth_rmep))
            {
                p_rmep_y1731_info->csf_en       = 1;
                p_rmep_y1731_info->d_csf        = GetDsEthRmep(V, dCsf_f,  &ds_eth_rmep);
                p_rmep_y1731_info->rx_csf_type  = GetDsEthRmep(V, rxCsfType_f, &ds_eth_rmep);
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO," p_rmep_y1731_info->d_csf  :%d\n",  p_rmep_y1731_info->d_csf);
            }

            lmep_index = p_rmep_y1731_info->lmep_index;
            if (lmep_index == mep_info->mep_index)
            {
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Invalid mep index\n");
                return CTC_E_NONE;

            }
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"lmep_index :%d, p_rmep_y1731_info->is_p2p_mode:%d\n",
                       lmep_index, p_rmep_y1731_info->is_p2p_mode );

        }
        else if ((CTC_OAM_MEP_TYPE_IP_BFD == mep_type)
                    || (CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type)
                    || (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
                    || (CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type))
        {
            sal_memcpy(&ds_bfd_rmep, &ds_eth_rmep, sizeof(DsEthMep_m));

            p_rmep_bfd_info = &(mep_info->rmep.bfd_rmep);
            p_rmep_bfd_info->actual_rx_interval = GetDsBfdRmep(V, actualRxInterval_f, &ds_bfd_rmep);
            p_rmep_bfd_info->first_pkt_rx = GetDsBfdRmep(V, firstPktRx_f, &ds_bfd_rmep);
            p_rmep_bfd_info->mep_en = GetDsBfdRmep(V, active_f, &ds_bfd_rmep);
            p_rmep_bfd_info->required_min_rx_interval = GetDsBfdRmep(V, requiredMinRxInterval_f, &ds_bfd_rmep);

            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"p_rmep_bfd_info->first_pkt_rx  :%d, rmepIndex:%d\n",
                       p_rmep_bfd_info->first_pkt_rx , mep_info->mep_index);

            if (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
            {
                uint32 ip_sa = GetDsBfdRmep(V, ipSa_f, &ds_bfd_rmep);
                p_rmep_bfd_info->mis_connect        = CTC_IS_BIT_SET(ip_sa, 0);
            }

            p_rmep_bfd_info->remote_detect_mult = GetDsBfdRmep(V, defectMult_f, &ds_bfd_rmep);
            p_rmep_bfd_info->remote_diag = GetDsBfdRmep(V, remoteDiag_f, &ds_bfd_rmep);
            p_rmep_bfd_info->remote_discr = GetDsBfdRmep(V, remoteDisc_f, &ds_bfd_rmep);
            p_rmep_bfd_info->remote_state = GetDsBfdRmep(V, remoteStat_f, &ds_bfd_rmep);

            p_rmep_bfd_info->lmep_index  = (mep_info->mep_index - 1);
            p_rmep_bfd_info->dloc_time = GetDsBfdRmep(V, defectWhile_f, &ds_bfd_rmep);
            /*If actual_rx_interval is 4 and BFD timer is 1ms mode, will have deviation*/
            if ((4 == p_rmep_bfd_info->actual_rx_interval)
                && (p_rmep_bfd_info->dloc_time > (GetDsBfdRmep(V, defectWhile_f, &ds_bfd_rmep) * 33 / 40)))
            {
                p_rmep_bfd_info->dloc_time = GetDsBfdRmep(V, defectWhile_f, &ds_bfd_rmep) * 33 / 40;
            }
            p_rmep_bfd_info->hw_aps_en = GetDsBfdRmep(V, fastApsEn_f, &ds_bfd_rmep);
            p_rmep_bfd_info->protection_path = GetDsBfdRmep(V, protectionPath_f, &ds_bfd_rmep);
            cmd = DRV_IOR(DsBfdMep_t, DsBfdMep_apsGroupId_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_rmep_bfd_info->lmep_index, cmd, &aps_gr_id_high));
            p_rmep_bfd_info->hw_aps_group_id = (aps_gr_id_high << 6) + GetDsBfdRmep(V, apsGroupId_f, &ds_bfd_rmep);
            if (p_rmep_bfd_info->hw_aps_group_id)
            {
                cmd = DRV_IOR(DsApsBridge_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_rmep_bfd_info->hw_aps_group_id, cmd, &ds_aps_bridge));
                p_rmep_bfd_info->path_fail = (p_rmep_bfd_info->protection_path) ? GetDsApsBridge(V, protectionFail_f, &ds_aps_bridge) :
                                                GetDsApsBridge(V, workingFail_f, &ds_aps_bridge);
                if ((p_rmep_bfd_info->protection_path && GetDsApsBridge(V, protectingEn_f, &ds_aps_bridge))
                    || (!p_rmep_bfd_info->protection_path && !GetDsApsBridge(V, protectingEn_f, &ds_aps_bridge)))
                {
                    p_rmep_bfd_info->path_active = 1;
                }
            }

            lmep_index = p_rmep_bfd_info->lmep_index;

        }
    }
    else
    {
        lmep_index = mep_info->mep_index;
    }

    if(is_remote)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lmep_index, cmd_eth_mep, &ds_eth_mep));
    }
    else
    {
        sal_memcpy(&ds_eth_mep, &ds_eth_rmep, sizeof(DsEthRmep_m));
    }

#if (SDK_WORK_PLATFORM == 1)
    if (!GetDsEthMep(V, active_f, &ds_eth_mep))
    {
        return CTC_E_NONE;
    }
#endif
    p_sys_lmep_com = (sys_oam_lmep_t*)ctc_vector_get(g_oam_master[lchip]->mep_vec, lmep_index);
    if (p_sys_lmep_com)
    {
        if (p_sys_lmep_com->p_sys_chan)
        {
            mep_type = p_sys_lmep_com->p_sys_chan->mep_type;
        }
        mep_info->mep_type = mep_type;
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"mep_info->mep_type:%d \n",     mep_info->mep_type );
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((CTC_OAM_MEP_TYPE_ETH_1AG == mep_type)
        || (CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_type)
        || (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == mep_type))
    {
         /* labter complete for gg eth-oam*/
        p_lmep_y1731_info = &(mep_info->lmep.y1731_lmep);

        p_lmep_y1731_info->mep_id         = GetDsEthMep(V, mepId_f, &ds_eth_mep);
        p_lmep_y1731_info->ccm_interval   = GetDsEthMep(V, ccmInterval_f, &ds_eth_mep);
        p_lmep_y1731_info->active         = GetDsEthMep(V, active_f, &ds_eth_mep);
        p_lmep_y1731_info->d_unexp_mep    = GetDsEthMep(V, dUnexpMep_f, &ds_eth_mep);
        p_lmep_y1731_info->d_mismerge     = GetDsEthMep(V, dMismerge_f, &ds_eth_mep);
        p_lmep_y1731_info->d_meg_lvl      = GetDsEthMep(V, dMegLvl_f, &ds_eth_mep);
        p_lmep_y1731_info->is_up_mep      = GetDsEthMep(V, isUp_f, &ds_eth_mep);
        p_lmep_y1731_info->vlan_id        = GetDsEthMep(V, mepPrimaryVid_f, &ds_eth_mep);
        p_lmep_y1731_info->present_rdi    = GetDsEthMep(V, presentRdi_f, &ds_eth_mep);
        p_lmep_y1731_info->ccm_enable     = GetDsEthMep(V, cciEn_f, &ds_eth_mep);
        p_lmep_y1731_info->dm_enable      = GetDsEthMep(V, enablePm_f, &ds_eth_mep);
        p_lmep_y1731_info->seq_num_en     = GetDsEthMep(V, seqNumEn_f, &ds_eth_mep);
        ma_index = GetDsEthMep(V, maIndex_f, &ds_eth_mep);
        cmd = DRV_IOR(DsMa_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, ma_index, cmd, &ds_ma);
        p_lmep_y1731_info->level = GetDsMa(V, mdLvl_f, &ds_ma);
        p_lmep_y1731_info->if_status_en = GetDsMa(V, txWithIfStatus_f, &ds_ma);
        p_lmep_y1731_info->port_status_en = GetDsMa(V, txWithPortStatus_f, &ds_ma);
        p_lmep_y1731_info->sender_id_en = GetDsMa(V, txWithSendId_f, &ds_ma);
        if (GetDsEthMep(V, ccmStatEn_f, &ds_eth_mep) && GetDsEthMep(V, ethOamP2PMode_f, &ds_eth_mep))
        {
            p_lmep_y1731_info->ccm_tx_pkts = GetDsEthMep(V, ccmSeqNum_f, &ds_eth_mep);
        }

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"p_lmep_y1731_info->d_meg_lvl  :%d\n", p_lmep_y1731_info->d_meg_lvl );

    }
    else if ((CTC_OAM_MEP_TYPE_IP_BFD == mep_type)
                    || (CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type)
                    || (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
                    || (CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type))
    {
        p_lmep_bfd_info = &(mep_info->lmep.bfd_lmep);

        sal_memcpy(&ds_bfd_mep, &ds_eth_mep, sizeof(DsEthRmep_m));

        p_lmep_bfd_info->mep_en =  GetDsBfdMep(V, active_f, &ds_bfd_mep);
        p_lmep_bfd_info->cc_enable = GetDsBfdMep(V, cciEn_f, &ds_bfd_mep);
        p_lmep_bfd_info->loacl_state = GetDsBfdMep(V, localStat_f, &ds_bfd_mep);
        p_lmep_bfd_info->local_diag = GetDsBfdMep(V, localDiag_f, &ds_bfd_mep);
        p_lmep_bfd_info->local_discr = p_sys_lmep_com->local_discr;    /*get from db, can not get from asic*/
        p_lmep_bfd_info->actual_tx_interval = GetDsBfdMep(V, actualMinTxInterval_f, &ds_bfd_mep);
        p_lmep_bfd_info->desired_min_tx_interval = GetDsBfdMep(V, desiredMinTxInterval_f, &ds_bfd_mep);

        cmd = DRV_IOR(DsBfdRmep_t,  DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, lmep_index + 1, cmd, &ds_bfd_rmep));
        p_lmep_bfd_info->local_detect_mult = GetDsBfdRmep(V, localDefectMult_f, &ds_bfd_rmep);
        p_lmep_bfd_info->single_hop = !(GetDsBfdRmep(V, isMultiHop_f, &ds_bfd_rmep));
        if (p_lmep_bfd_info->loacl_state != CTC_OAM_BFD_STATE_UP)
        {
            cmd = DRV_IOR(DsBfdSlowIntervalCfg_t,  DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_bfd_slow_interval_cfg));
            p_lmep_bfd_info->actual_tx_interval = GetDsBfdSlowIntervalCfg(V, txSlowInterval_f, &ds_bfd_slow_interval_cfg);
            if (p_rmep_bfd_info)
            {
                p_rmep_bfd_info->actual_rx_interval = GetDsBfdSlowIntervalCfg(V, rxSlowInterval_f, &ds_bfd_slow_interval_cfg);
            }
        }


        if (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
        {
            if (GetDsBfdRmep(V, active_f, &ds_bfd_rmep))
            {
                uint32 ip_sa = GetDsBfdRmep(V, ipSa_f, &ds_bfd_rmep);
                mep_info->rmep.bfd_rmep.mis_connect = CTC_IS_BIT_SET(ip_sa, 0);
            }
            if (GetDsBfdRmep(V, isCsfEn_f, &ds_bfd_rmep))
            {
                p_lmep_bfd_info->csf_en       = 1;
                p_lmep_bfd_info->d_csf        = GetDsBfdMep(V, dCsf_f,  &ds_bfd_mep);
                p_lmep_bfd_info->rx_csf_type  = GetDsBfdMep(V, rxCsfType_f, &ds_bfd_mep);
                SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO," p_lmep_bfd_info->mep_en :%d,  p_lmep_bfd_info->d_csf  :%d, p_lmep_bfd_info->rx_csf_type  :%d\n",
                                 p_lmep_bfd_info->mep_en,
                                 p_lmep_bfd_info->d_csf,
                                 p_lmep_bfd_info->rx_csf_type);
            }
            p_lmep_bfd_info->cv_enable = GetDsBfdRmep(V, isCvEn_f, &ds_bfd_rmep);
            if(p_lmep_bfd_info->cv_enable)
            {
               sal_memcpy(&p_lmep_bfd_info->mep_id, &p_sys_lmep_com->p_sys_maid->maid, p_sys_lmep_com->p_sys_maid->maid_len);
            }
        }

    }

    return CTC_E_NONE;
}

int32
_sys_usw_oam_build_defect_event(uint8 lchip, ctc_oam_mep_info_t* p_mep_info, ctc_oam_event_t* p_event)
{
    uint16  entry_idx   = 0;
    uint16  mep_index   = 0;
    uint32  cmd = 0;
    uint32* bitmap     = NULL;

    uint8 d_meg_lvl     = 0;
    uint8 d_mismerge    = 0;
    uint8 d_unexp_mep   = 0;
    uint8 present_rdi   = 0;

    uint8 d_loc             = 0;
    uint8 first_pkt_rx      = 0;
    uint8 d_unexp_period    = 0;
    uint8 mac_status_change = 0;
    uint8 ma_sa_mismatch    = 0;
    uint8 last_rdi          = 0;
    uint8 d_csf             = 0;
    uint8 mis_connect       = 0;
    uint8 state             = CTC_OAM_BFD_STATE_ADMIN_DOWN;
    uint8 remote_state      = 0xFF;
    tbl_entry_t  tbl_entry;
    DsBfdRmep_m ds_bfd_rmep;
    DsBfdRmep_m ds_bfd_rmep_mask;
    mac_addr_t   macda   = {1, 0, 0x5E, 0x90, 0, 1};
    sys_oam_lmep_t* p_sys_lmep_com  = NULL;
    sys_oam_rmep_t* p_sys_rmep_com  = NULL;
    uint16 index = 0;
    uint8  d_sf = 0;
    uint32 sf_value = 0;

    entry_idx = p_event->valid_entry_num;
    mep_index = p_mep_info->mep_index;

    p_event->oam_event_entry[entry_idx].mep_type    = p_mep_info->mep_type;
    p_event->oam_event_entry[entry_idx].is_remote   = p_mep_info->is_rmep;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"p_mep_info->is_rmep    :%d, type:%d, active:%d\n",
        p_mep_info->is_rmep, p_mep_info->mep_type, p_mep_info->rmep.y1731_rmep.active);

    /*common*/
    if (p_mep_info->is_rmep)
    {
        p_sys_rmep_com = ctc_vector_get(g_oam_master[lchip]->mep_vec, mep_index );
        if ((NULL == p_sys_rmep_com)
            || ((0 == p_mep_info->rmep.y1731_rmep.active) && ((CTC_OAM_MEP_TYPE_ETH_1AG == p_mep_info->mep_type)
                                                              || (CTC_OAM_MEP_TYPE_ETH_Y1731 == p_mep_info->mep_type)
                                                              || (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == p_mep_info->mep_type)))
            ||((0 == p_mep_info->rmep.bfd_rmep.mep_en) && ((CTC_OAM_MEP_TYPE_IP_BFD == p_mep_info->mep_type)
                                                              || (CTC_OAM_MEP_TYPE_MPLS_BFD == p_mep_info->mep_type)
                                                              || (CTC_OAM_MEP_TYPE_MICRO_BFD == p_mep_info->mep_type)
                                                              || (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == p_mep_info->mep_type))))
        {
            return CTC_E_NONE;
        }



        p_sys_lmep_com = p_sys_rmep_com->p_sys_lmep;
        if ((NULL == p_sys_lmep_com)
            || ((0 == p_mep_info->lmep.y1731_lmep.active) && ((CTC_OAM_MEP_TYPE_ETH_1AG == p_mep_info->mep_type)
                                                              || (CTC_OAM_MEP_TYPE_ETH_Y1731 == p_mep_info->mep_type)
                                                              || (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == p_mep_info->mep_type)))
            || ((0 == p_mep_info->lmep.bfd_lmep.mep_en) && ((CTC_OAM_MEP_TYPE_IP_BFD == p_mep_info->mep_type)
                                                              || (CTC_OAM_MEP_TYPE_MPLS_BFD == p_mep_info->mep_type)
                                                              || (CTC_OAM_MEP_TYPE_MICRO_BFD == p_mep_info->mep_type)
                                                              || (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == p_mep_info->mep_type))))
        {
            return CTC_E_NONE;
        }

        p_event->oam_event_entry[entry_idx].lmep_index = p_sys_lmep_com->lmep_index;
        p_event->oam_event_entry[entry_idx].rmep_index = p_sys_rmep_com->rmep_index;

    }
    else
    {
        p_sys_lmep_com = ctc_vector_get(g_oam_master[lchip]->mep_vec, mep_index);
        if ((NULL == p_sys_lmep_com)
            || ((0 == p_mep_info->lmep.y1731_lmep.active) && ((CTC_OAM_MEP_TYPE_ETH_1AG == p_mep_info->mep_type)
                                                              || (CTC_OAM_MEP_TYPE_ETH_Y1731 == p_mep_info->mep_type)
                                                              || (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == p_mep_info->mep_type)))
            || ((0 == p_mep_info->lmep.bfd_lmep.mep_en) && ((CTC_OAM_MEP_TYPE_IP_BFD == p_mep_info->mep_type)
                                                              || (CTC_OAM_MEP_TYPE_MPLS_BFD == p_mep_info->mep_type)
                                                              || (CTC_OAM_MEP_TYPE_MICRO_BFD == p_mep_info->mep_type)
                                                              || (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == p_mep_info->mep_type))))
        {
            return CTC_E_NONE;
        }

        p_event->oam_event_entry[entry_idx].lmep_index = p_sys_lmep_com->lmep_index;
    }

    /*mep type*/
    if (p_sys_lmep_com->p_sys_maid)
    {
        sal_memcpy(p_event->oam_event_entry[entry_idx].maid, p_sys_lmep_com->p_sys_maid->maid, p_sys_lmep_com->p_sys_maid->maid_len);
    }

    /*get key*/
    if (p_sys_lmep_com->p_sys_chan)
    {
        if ((CTC_OAM_MEP_TYPE_ETH_1AG == p_mep_info->mep_type)
            ||(CTC_OAM_MEP_TYPE_ETH_Y1731 == p_mep_info->mep_type))
        {
            p_event->oam_event_entry[entry_idx].oam_key.u.eth.vlan_id   = p_mep_info->lmep.y1731_lmep.vlan_id;
            p_event->oam_event_entry[entry_idx].oam_key.u.eth.md_level  = p_mep_info->lmep.y1731_lmep.level;
            p_event->oam_event_entry[entry_idx].oam_key.u.eth.gport     = ((sys_oam_chan_eth_t*)p_sys_lmep_com->p_sys_chan)->key.gport;
        }
        else if ((CTC_OAM_MEP_TYPE_IP_BFD == p_mep_info->mep_type)
                || (CTC_OAM_MEP_TYPE_MICRO_BFD == p_mep_info->mep_type)
                || (CTC_OAM_MEP_TYPE_MPLS_BFD == p_mep_info->mep_type))
        {
            p_event->oam_event_entry[entry_idx].oam_key.u.bfd.discr = p_mep_info->lmep.bfd_lmep.local_discr;
        }
        else if ((CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == p_mep_info->mep_type)
                || (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == p_mep_info->mep_type))
        {
            if(((sys_oam_chan_tp_t*)p_sys_lmep_com->p_sys_chan)->key.section_oam)
            {
                CTC_SET_FLAG(p_event->oam_event_entry[entry_idx].oam_key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
            }
            else
            {
                CTC_UNSET_FLAG(p_event->oam_event_entry[entry_idx].oam_key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
            }
            p_event->oam_event_entry[entry_idx].oam_key.u.tp.label  = ((sys_oam_chan_tp_t*)p_sys_lmep_com->p_sys_chan)->key.label;
            p_event->oam_event_entry[entry_idx].oam_key.u.tp.mpls_spaceid    = ((sys_oam_chan_tp_t*)p_sys_lmep_com->p_sys_chan)->key.spaceid;
            p_event->oam_event_entry[entry_idx].oam_key.u.tp.gport_or_l3if_id = ((sys_oam_chan_tp_t*)p_sys_lmep_com->p_sys_chan)->key.gport_l3if_id;
        }
    }


    if ((CTC_OAM_MEP_TYPE_ETH_1AG == p_mep_info->mep_type)
        || (CTC_OAM_MEP_TYPE_ETH_Y1731 == p_mep_info->mep_type)
        || (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == p_mep_info->mep_type))
    {
        p_event->oam_event_entry[entry_idx].lmep_id = p_mep_info->lmep.y1731_lmep.mep_id;
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"p_mep_info->is_rme    :%d\n", p_mep_info->is_rmep);
        if (p_mep_info->is_rmep)
        {
            p_event->oam_event_entry[entry_idx].rmep_id = p_mep_info->rmep.y1731_rmep.rmep_id;

            d_loc             = p_mep_info->rmep.y1731_rmep.d_loc;
            first_pkt_rx      = p_mep_info->rmep.y1731_rmep.first_pkt_rx;
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"first_pkt_rx    :%d\n", first_pkt_rx);
            d_unexp_period    = p_mep_info->rmep.y1731_rmep.d_unexp_period;
            mac_status_change = p_mep_info->rmep.y1731_rmep.mac_status_change;
            ma_sa_mismatch    = p_mep_info->rmep.y1731_rmep.ma_sa_mismatch;
            last_rdi          = p_mep_info->rmep.y1731_rmep.last_rdi;
            d_csf             = p_mep_info->rmep.y1731_rmep.d_csf;

        }

        d_meg_lvl     = p_mep_info->lmep.y1731_lmep.d_meg_lvl;
        d_mismerge    = p_mep_info->lmep.y1731_lmep.d_mismerge;
        d_unexp_mep   = p_mep_info->lmep.y1731_lmep.d_unexp_mep;
        present_rdi   = p_mep_info->lmep.y1731_lmep.present_rdi;

        /*process for sf */
        for (index = 0; index < CTC_USW_OAM_DEFECT_NUM; index++)
        {
            if (!CTC_IS_BIT_SET(g_oam_master[lchip]->sf_defect_bitmap, index))
            {
                continue;
            }

            sf_value = (1 << index);

            /*Ethoam rmep*/
            if (sf_value == CTC_OAM_DEFECT_UNEXPECTED_PERIOD)
            {
                d_sf |= d_unexp_period;
            }
            else if (sf_value == CTC_OAM_DEFECT_DLOC)
            {
                d_sf |= d_loc;
            }
            else if (sf_value == CTC_OAM_DEFECT_EVENT_RDI_RX)
            {
                d_sf |= last_rdi;
            }
            else if (sf_value == CTC_OAM_DEFECT_MAC_STATUS_CHANGE)
            {
                d_sf |= mac_status_change;
            }
            /*Ethoam lmep*/
            else if (sf_value == CTC_OAM_DEFECT_UNEXPECTED_MEP)
            {
                d_sf |= d_unexp_mep;
            }
            else if (sf_value == CTC_OAM_DEFECT_MISMERGE)
            {
                d_sf |= d_mismerge;
            }
            else if (sf_value == CTC_OAM_DEFECT_UNEXPECTED_LEVEL)
            {
                d_sf |= d_meg_lvl;
            }
        }
    }
    else if ((CTC_OAM_MEP_TYPE_IP_BFD == p_mep_info->mep_type)
            || (CTC_OAM_MEP_TYPE_MPLS_BFD == p_mep_info->mep_type)
            || (CTC_OAM_MEP_TYPE_MICRO_BFD == p_mep_info->mep_type)
            || (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == p_mep_info->mep_type))
    {
        if (p_mep_info->is_rmep)
        {
            first_pkt_rx    = p_mep_info->rmep.bfd_rmep.first_pkt_rx;
            remote_state    = p_mep_info->rmep.bfd_rmep.remote_state;
            p_event->oam_event_entry[entry_idx].remote_diag     = p_mep_info->rmep.bfd_rmep.remote_diag;
            p_event->oam_event_entry[entry_idx].remote_state    = remote_state;

        }
        d_csf           = p_mep_info->lmep.bfd_lmep.d_csf;
        mis_connect     = p_mep_info->rmep.bfd_rmep.mis_connect;
        state           = p_mep_info->lmep.bfd_lmep.loacl_state;
        p_event->oam_event_entry[entry_idx].local_diag  = p_mep_info->lmep.bfd_lmep.local_diag;
        p_event->oam_event_entry[entry_idx].local_state = state;

        /*process for sf */
        for (index = 0; index < CTC_USW_OAM_DEFECT_NUM; index++)
        {
            if (!CTC_IS_BIT_SET(g_oam_master[lchip]->sf_defect_bitmap, index))
            {
                continue;
            }

            sf_value = (1 << index);

            /*bfd rmep*/
            if (sf_value == CTC_OAM_DEFECT_EVENT_BFD_MIS_CONNECT)
            {
                d_sf |= mis_connect;
            }
            else if (sf_value == CTC_OAM_DEFECT_EVENT_BFD_DOWN)
            {
                d_sf |= (CTC_OAM_BFD_STATE_DOWN == state)?1:0;
            }
        }
    }

    /*event*/
    bitmap = &p_event->oam_event_entry[entry_idx].event_bmp;
    d_meg_lvl ? (CTC_SET_FLAG((*bitmap), CTC_OAM_DEFECT_UNEXPECTED_LEVEL)) : (CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_UNEXPECTED_LEVEL));
    d_mismerge ? (CTC_SET_FLAG((*bitmap), CTC_OAM_DEFECT_MISMERGE)) : (CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_MISMERGE));
    d_unexp_mep ? (CTC_SET_FLAG((*bitmap), CTC_OAM_DEFECT_UNEXPECTED_MEP)) : (CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_UNEXPECTED_MEP));
    present_rdi ? (CTC_SET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_RDI_TX)) : (CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_RDI_TX));


    if (CTC_OAM_BFD_STATE_DOWN == state)
    {
        CTC_SET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_BFD_DOWN);
        CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_BFD_INIT);
        CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_BFD_UP);

        if ((CTC_OAM_MEP_TYPE_MICRO_BFD == p_mep_info->mep_type))
        {
            if (p_mep_info->is_rmep)
            {
                mep_index--;
            }
            sal_memset(&ds_bfd_rmep, 0, sizeof(ds_bfd_rmep));
            sal_memset(&ds_bfd_rmep_mask, 0xFF, sizeof(ds_bfd_rmep_mask));
            cmd  = DRV_IOR(DsBfdRmep_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, mep_index + 1, cmd, &ds_bfd_rmep));

            SetDsBfdRmep(V, learnEn_f, &ds_bfd_rmep, 1);
            SetDsBfdRmep(V, learnEn_f, &ds_bfd_rmep_mask, 0);

            cmd = DRV_IOW(DsBfdRmep_t, DRV_ENTRY_FLAG);
            tbl_entry.data_entry = (uint32*)&ds_bfd_rmep;
            tbl_entry.mask_entry = (uint32*)&ds_bfd_rmep_mask;
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, mep_index + 1, cmd, &tbl_entry));

            /*update MicroBFD MACDA to "01005e900001"*/
            CTC_ERROR_RETURN(_sys_usw_bfd_update_nh_process(lchip, p_sys_lmep_com->nhid, macda));
        }
    }
    else if (CTC_OAM_BFD_STATE_INIT == state)
    {
        CTC_SET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_BFD_INIT);
        CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_BFD_DOWN);
        CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_BFD_UP);
    }
    else if (CTC_OAM_BFD_STATE_UP == state)
    {
        CTC_SET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_BFD_UP);
        CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_BFD_INIT);
        CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_BFD_DOWN);
    }

    if (p_mep_info->is_rmep)
    {
        d_loc ? (CTC_SET_FLAG((*bitmap), CTC_OAM_DEFECT_DLOC)) : (CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_DLOC));
        first_pkt_rx ? (CTC_SET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_RX_FIRST_PKT)) : (CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_RX_FIRST_PKT));
        d_unexp_period ? (CTC_SET_FLAG((*bitmap), CTC_OAM_DEFECT_UNEXPECTED_PERIOD)) : (CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_UNEXPECTED_PERIOD));
        mac_status_change ? (CTC_SET_FLAG((*bitmap), CTC_OAM_DEFECT_MAC_STATUS_CHANGE)) : (CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_MAC_STATUS_CHANGE));
        ma_sa_mismatch ? (CTC_SET_FLAG((*bitmap), CTC_OAM_DEFECT_SRC_MAC_MISMATCH)) : (CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_SRC_MAC_MISMATCH));
        last_rdi ? (CTC_SET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_RDI_RX)) : (CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_RDI_RX));

    }
    d_csf ? (CTC_SET_FLAG((*bitmap), CTC_OAM_DEFECT_CSF)) : (CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_CSF));
    mis_connect ? (CTC_SET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_BFD_MIS_CONNECT)) : (CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_EVENT_BFD_MIS_CONNECT));
    d_sf?(CTC_SET_FLAG((*bitmap), CTC_OAM_DEFECT_SF)):(CTC_UNSET_FLAG((*bitmap), CTC_OAM_DEFECT_SF));
    p_event->valid_entry_num++;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"bitmap    :0x%x\n", *bitmap);

    return CTC_E_NONE;
}

int32
_sys_usw_oam_set_exception_en(uint8 lchip, uint32 exception_en)
{
    uint32 cmd = 0;

    uint32 cpu_excp[2]  = {0};
    OamHeaderEditCtl_m   oam_header_edit_ctl;

    sal_memset(&oam_header_edit_ctl, 0, sizeof(OamHeaderEditCtl_m));

    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_INVALID_OAM_PDU))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_INVALID);
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_BFD_INVALID);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_HIGH_VER_OAM_TO_CPU))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_HIGH_VER);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_OPTION_CCM_TLV))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_TLV);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_APS_PDU_TO_CPU))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_APS);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_DM_TO_CPU))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_DM);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_PBT_MM_DEFECT_OAM_PDU))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_PBX_OAM);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_LM_TO_CPU))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_LM);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_ETH_TST_TO_CPU))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_TST);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_BIG_INTERVAL_OR_SW_MEP_TO_CPU))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_BIG_CCM);
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_BIG_BFD);
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_LEARN_CCM);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_MPLS_TP_DLM_TO_CPU))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_TP_DLM);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_MPLS_TP_DM_DLMDM_TO_CPU))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_TP_DM);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_CSF_TO_CPU))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_CSF);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_MCC_PDU_TO_CPU))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_MCC);
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_TP_MCC);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_EQUAL_LTM_LTR_TO_CPU))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_LT);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_LEARNING_BFD_TO_CPU))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_BFD_LEARN);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_BFD_TIMER_NEGOTIATION))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_BFD_TIMER_NEG);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_SCC_PDU_TO_CPU))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_SCC);
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_TP_SCC);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_ALL_DEFECT))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_DEFECT);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_LBM))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_LB);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_TP_LBM))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_TP_LB);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_TP_CSF))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_TP_CSF);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_TP_FM))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_TP_FM);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_SM))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_SM);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_UNKNOWN_PDU))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_OTHER);
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_BFD_OTHER);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_BFD_DISC_MISMATCH))
    {
        CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_BFD_DISC_MISMATCH);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_TP_BFD_CV))
    {
        CTC_BIT_SET(cpu_excp[1], SYS_OAM_EXCP_TP_CV - 32);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_TWAMP_TO_CPU))
    {
        CTC_BIT_SET(cpu_excp[1], SYS_OAM_EXCP_TWAMP - 32);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_ETH_SC_TO_CPU))
    {
        CTC_BIT_SET(cpu_excp[1], SYS_OAM_EXCP_ETH_SC - 32);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_ETH_LL_TO_CPU))
    {
        CTC_BIT_SET(cpu_excp[1], SYS_OAM_EXCP_ETH_LL - 32);
    }
    if (CTC_IS_BIT_SET(exception_en, CTC_OAM_EXCP_MACDA_CHK_FAIL))
    {
        CTC_BIT_SET(cpu_excp[1], SYS_OAM_EXCP_MAC_FAIL - 32);
    }


    cmd = DRV_IOR(OamHeaderEditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_header_edit_ctl));

    DRV_SET_FIELD_A(lchip, OamHeaderEditCtl_t, OamHeaderEditCtl_cpuExceptionEn_f, &oam_header_edit_ctl, cpu_excp);
    cmd = DRV_IOW(OamHeaderEditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_header_edit_ctl));

    return CTC_E_NONE;
}

int32
_sys_usw_oam_set_common_property(uint8 lchip, void* p_oam_property)
{

    uint32 cfg_type = 0;
    uint32 value = 0;
    uint32 cmd = 0;
    uint8  rdi_index        = 0;
    uint8  defect_type      = 0;
    uint8  defect_sub_type  = 0;
    uint8  idx              = 0;
    uint32 tmp[4] = {0};
    uint32 dloc_rdi         = 0;
    uint8 local_sf_rdi = 0;
    uint8 remote_sf_rdi = 0;

    ctc_oam_com_property_t* p_common_property = NULL;
    OamRxProcEtherCtl_m     oam_rx_proc_ether_ctl;
    OamErrorDefectCtl_m     oam_error_defect_ctl;
    OamUpdateApsCtl_m       oam_update_aps_ctl;
    OamUpdateCtl_m          oam_update_ctl;

    p_common_property = &(((ctc_oam_property_t*)p_oam_property)->u.common);
    cfg_type    = p_common_property->cfg_type;
    value   = p_common_property->value;

    if (CTC_OAM_COM_PRO_TYPE_TRIG_APS_MSG == cfg_type)
    {
        return CTC_E_NOT_SUPPORT;
    }
    else if (CTC_OAM_COM_PRO_TYPE_DEFECT_TO_RDI == cfg_type)
    {
        sal_memset(&oam_update_ctl, 0, sizeof(OamUpdateCtl_m));
        sal_memset(&oam_rx_proc_ether_ctl, 0, sizeof(OamRxProcEtherCtl_m));

        cmd = DRV_IOR(OamRxProcEtherCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_rx_proc_ether_ctl));
        cmd = DRV_IOR(OamUpdateCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_update_ctl));
        for (idx = 0; idx < CTC_USW_OAM_DEFECT_NUM; idx++)
        {
            if (!CTC_IS_BIT_SET(value, idx))
            {
                continue;
            }
            /*RDIMode 0*/
            _sys_usw_oam_get_rdi_defect_type(lchip, (1 << idx), &defect_type, &defect_sub_type);
            rdi_index = (defect_type << 3) | defect_sub_type;
            if (rdi_index < 32)
            {
                CTC_BIT_SET(tmp[0], rdi_index);
            }
            else
            {
                CTC_BIT_SET(tmp[1], (rdi_index - 32));
            }

            /*RDIMode 1*/
            switch (1 << idx)
            {
                case CTC_OAM_DEFECT_MAC_STATUS_CHANGE:
                    CTC_BIT_SET(remote_sf_rdi, 0);
                    CTC_BIT_SET(remote_sf_rdi, 1);
                    break;
                case CTC_OAM_DEFECT_MISMERGE:
                    CTC_BIT_SET(local_sf_rdi, 1);
                    break;
                case CTC_OAM_DEFECT_UNEXPECTED_LEVEL:
                    CTC_BIT_SET(local_sf_rdi, 0);
                    break;
                case CTC_OAM_DEFECT_UNEXPECTED_MEP:
                    CTC_BIT_SET(local_sf_rdi, 2);
                    break;
                case CTC_OAM_DEFECT_UNEXPECTED_PERIOD:
                    CTC_BIT_SET(remote_sf_rdi, 4);
                    break;
                case CTC_OAM_DEFECT_DLOC:
                    CTC_BIT_SET(remote_sf_rdi, 3);
                    break;
                default:
                    break;
            }

        }
        if (!GetOamUpdateCtl(V, y1731RdiAutoIndicate_f, &oam_update_ctl))    /*RDIMode 0*/
        {
            SetOamRxProcEtherCtl(V, etherDefectToRdi0_f, &oam_rx_proc_ether_ctl, tmp[0]);
            SetOamRxProcEtherCtl(V, etherDefectToRdi1_f, &oam_rx_proc_ether_ctl, tmp[1]);
            cmd = DRV_IOW(OamRxProcEtherCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_rx_proc_ether_ctl));
            g_oam_master[lchip]->defect_to_rdi_bitmap0 = tmp[0];
            g_oam_master[lchip]->defect_to_rdi_bitmap1 = tmp[1];
        }
        else  /*RDIMode 1*/
        {
            SetOamUpdateCtl(V, localSfFailtoRdi_f, &oam_update_ctl, local_sf_rdi);
            SetOamUpdateCtl(V, remoteSfFailtoRdi_f, &oam_update_ctl, remote_sf_rdi);
            cmd = DRV_IOW(OamUpdateCtl_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_update_ctl));
        }
        /* dloc trigger rdi*/
        if (CTC_FLAG_ISSET(value, CTC_OAM_DEFECT_DLOC))
        {
            cmd = DRV_IOW(OamUpdateCtl_t, OamUpdateCtl_genRdiByDloc_f);
            dloc_rdi = 1;
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dloc_rdi));
        }
        else
        {
            cmd = DRV_IOW(OamUpdateCtl_t, OamUpdateCtl_genRdiByDloc_f);
            dloc_rdi = 0;
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &dloc_rdi));
        }
    }
    else if (CTC_OAM_COM_PRO_TYPE_EXCEPTION_TO_CPU == cfg_type)
    {
        CTC_ERROR_RETURN(_sys_usw_oam_set_exception_en(lchip, value));
    }
    else if (CTC_OAM_COM_PRO_TYPE_DEFECT_TO_CPU == cfg_type)
    {
        sys_oam_defect_type_t defect_info;
        uint8 cnt = 0;
        sal_memset(&oam_error_defect_ctl, 0, sizeof(OamErrorDefectCtl_m));
        cmd = DRV_IOR(OamErrorDefectCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_error_defect_ctl));
        for (idx = 0; idx < CTC_USW_OAM_DEFECT_NUM; idx++)
        {
            if (!CTC_IS_BIT_SET(value, idx))
            {
                continue;
            }
            sal_memset(&defect_info, 0, sizeof(sys_oam_defect_type_t));
            _sys_usw_oam_get_defect_type(lchip, (1 << idx), &defect_info);

            for (cnt = 0; cnt < defect_info.entry_num; cnt++)
            {
                rdi_index = (defect_info.defect_type[cnt] << 3) | defect_info.defect_sub_type[cnt];

                if (rdi_index < 32)
                {
                    CTC_BIT_SET(tmp[0], rdi_index);
                }
                else
                {
                    CTC_BIT_SET(tmp[1], (rdi_index - 32));
                }
            }
        }
        SetOamErrorDefectCtl(A, reportDefectEn_f, &oam_error_defect_ctl, tmp);
        cmd = DRV_IOW(OamErrorDefectCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_error_defect_ctl));

    }
    else if (CTC_OAM_COM_PRO_TYPE_DEFECT_TO_SF == cfg_type)
    {
        uint32 defect_val = 0;

        for (idx = 0; idx < CTC_USW_OAM_DEFECT_NUM; idx++)
        {
            if (!CTC_IS_BIT_SET(value, idx))
            {
                CTC_BIT_UNSET(g_oam_master[lchip]->sf_defect_bitmap, idx);
                continue;
            }

            CTC_BIT_SET(g_oam_master[lchip]->sf_defect_bitmap, idx);
            defect_val = (1 << idx);

            /*Ethoam rmep*/
            if (defect_val == CTC_OAM_DEFECT_UNEXPECTED_PERIOD)
            {
                CTC_BIT_SET(tmp[0], 4);
            }
            else if (defect_val == CTC_OAM_DEFECT_DLOC)
            {
                CTC_BIT_SET(tmp[0], 3);
            }
            else if (defect_val == CTC_OAM_DEFECT_EVENT_RDI_RX)
            {
                CTC_BIT_SET(tmp[0], 2);
            }
            else if (defect_val == CTC_OAM_DEFECT_MAC_STATUS_CHANGE)
            {
                CTC_BIT_SET(tmp[0], 1);
                CTC_BIT_SET(tmp[0], 0);
            }
            /*Ethoam lmep*/
            else if (defect_val == CTC_OAM_DEFECT_UNEXPECTED_MEP)
            {
                CTC_BIT_SET(tmp[1], 2);
            }
            else if (defect_val == CTC_OAM_DEFECT_MISMERGE)
            {
                CTC_BIT_SET(tmp[1], 1);
            }
            else if (defect_val == CTC_OAM_DEFECT_UNEXPECTED_LEVEL)
            {
                CTC_BIT_SET(tmp[1], 0);
            }
            /*bfd rmep*/
            else if (defect_val == CTC_OAM_DEFECT_EVENT_BFD_MIS_CONNECT)
            {
                CTC_BIT_SET(tmp[2], 1);
            }
            else if (defect_val == CTC_OAM_DEFECT_EVENT_BFD_DOWN)
            {
                CTC_BIT_SET(tmp[2], 0);
            }
        }
        cmd = DRV_IOR(OamUpdateApsCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_update_aps_ctl));
        SetOamUpdateApsCtl(V, ethFastApsRemote_f, &oam_update_aps_ctl, tmp[0]);
        SetOamUpdateApsCtl(V, ethFastApsLocal_f, &oam_update_aps_ctl, tmp[1]);
        SetOamUpdateApsCtl(V, bfdFastApsRemote_f, &oam_update_aps_ctl, tmp[2]);
        cmd = DRV_IOW(OamUpdateApsCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_update_aps_ctl));

    }
    else if (CTC_OAM_COM_PRO_TYPE_HW_APS_MODE == cfg_type)
    {
        cmd = DRV_IOR(OamUpdateApsCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_update_aps_ctl));
        SetOamUpdateApsCtl(V, selfRepair_f, &oam_update_aps_ctl, value?1:0);
        cmd = DRV_IOW(OamUpdateApsCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_update_aps_ctl));
    }
    else if (CTC_OAM_COM_PRO_TYPE_TIMER_DISABLE == cfg_type)
    {
        value = (value >= 1)? 0 : 1;
        cmd = DRV_IOW(OamUpdateCtl_t, OamUpdateCtl_updEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOW(OamUpdateCtl_t, OamUpdateCtl_bfdUpdEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        g_oam_master[lchip]->timer_update_disable = !value;
    }
    else if (CTC_OAM_COM_PRO_TYPE_MASTER_GCHIP == cfg_type)
    {
        sys_traverse_t user_data;
        SYS_GLOBAL_CHIPID_CHECK(value);
        sal_memset(&user_data, 0 ,sizeof(sys_traverse_t));
        user_data.value1 = lchip;
        user_data.value2 = value;
        CTC_ERROR_RETURN(ctc_hash_traverse(g_oam_master[lchip]->chan_hash, (hash_traversal_fn)_sys_usw_oam_lmep_update_for_traverse, (void *)&user_data));
    }

    return CTC_E_NONE;
}


int32
_sys_usw_oam_com_reg_init(uint8 lchip)
{
    DsPriorityMap_m ds_priority_map;
    uint32 cmd = 0;
    uint8 priority_index = 0;
    uint32 cpu_excp[2]  = {0};
    uint16 oam_up_phy_num = 0;
    uint16 upd_interval = 0;
    uint16 ccm_max_ptr = 0;
    uint32 ccm_min_ptr = 0;
    uint16 oam_bfd_phy_num      = 0;
    uint16 bfd_upd_interval     = 0;
    uint16 bfd_min_ptr          = 0;
    uint16 bfd_max_ptr          = 0;
    uint32 quater_cci2_interval = 0;
    uint32 tmp_field = 0;
    uint32 freq         = 0;
    DsOamExcp_m ds_oam_excp;

    OamUpdateCtl_m        oam_update_ctl;
    OamEtherCciCtl_m     oam_ether_cci_ctl;
    OamUpdateApsCtl_m    oam_update_aps_ctl;
    OamHeaderEditCtl_m   oam_header_edit_ctl;


    if (g_oam_master[lchip]->no_mep_resource)
    {
        return CTC_E_NONE;
    }

    sal_memset(&oam_update_aps_ctl, 0, sizeof(OamUpdateApsCtl_m));
    sal_memset(&ds_priority_map, 0, sizeof(DsPriorityMap_m));
    sal_memset(&oam_header_edit_ctl, 0, sizeof(OamHeaderEditCtl_m));

    sal_memset(&ds_oam_excp, 0, sizeof(DsOamExcp_m));

    cmd = DRV_IOW(DsPriorityMap_t, DRV_ENTRY_FLAG);
    for (priority_index = 0; priority_index < 8; priority_index++)
    {
        SetDsPriorityMap(V, color_f, &ds_priority_map, 3);
        SetDsPriorityMap(V, cos_f, &ds_priority_map, priority_index);
        SetDsPriorityMap(V, _priority_f, &ds_priority_map, (priority_index * 2));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, priority_index, cmd, &ds_priority_map));
    }

     /*CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_INVALID);*/
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_LB);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_LT);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_LM);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_DM);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_TST);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_APS);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_SCC);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_MCC);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_CSF);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_BIG_CCM);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_LEARN_CCM);
     /*CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_DEFECT);*/
     /*CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_PBX_OAM);*/
     /*CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_HIGH_VER);*/
     /*CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_TLV);*/
     CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_OTHER);
     /*CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_BFD_INVALID);*/
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_BFD_LEARN);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_BIG_BFD);
     /*CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_BFD_TIMER_NEG);*/
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_TP_SCC);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_TP_MCC);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_TP_CSF);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_TP_LB);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_TP_DLM);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_TP_DM);
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_TP_FM);
     /*CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_BFD_OTHER);*/
     /*CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_BFD_DISC_MISMATCH);*/
    CTC_BIT_SET(cpu_excp[0], SYS_OAM_EXCP_ETH_SM);
    CTC_BIT_SET(cpu_excp[1], SYS_OAM_EXCP_TP_CV-32);
    CTC_BIT_SET(cpu_excp[1], SYS_OAM_EXCP_TWAMP-32);
    CTC_BIT_SET(cpu_excp[1], SYS_OAM_EXCP_ETH_SC-32);
    CTC_BIT_SET(cpu_excp[1], SYS_OAM_EXCP_ETH_LL-32);
    CTC_BIT_SET(cpu_excp[1], SYS_OAM_EXCP_MAC_FAIL-32);

    cmd = DRV_IOR(OamHeaderEditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_header_edit_ctl));
    DRV_SET_FIELD_A(lchip, OamHeaderEditCtl_t,OamHeaderEditCtl_cpuExceptionEn_f,&oam_header_edit_ctl,cpu_excp);
    SetOamHeaderEditCtl(V, logicSrcPort64kMode_f, &oam_header_edit_ctl, 0);

    cmd = DRV_IOW(OamHeaderEditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_header_edit_ctl));

    /*OamEngine Update*/
    freq = sys_usw_get_core_freq(lchip, 0);
    sal_memset(&oam_update_ctl, 0, sizeof(OamUpdateCtl_m));
    cmd = DRV_IOR(OamUpdateCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_update_ctl));
    /*CCM Timer*/

    ccm_min_ptr = 0;
    oam_up_phy_num       = TABLE_MAX_INDEX(lchip, DsEthMep_t) ;
    upd_interval = (freq/10 * 8333)/oam_up_phy_num - 1;
    ccm_max_ptr = (freq/10 * 8333)/(upd_interval + 1) + ccm_min_ptr - 1;

    SetOamUpdateCtl(V, updEn_f, &oam_update_ctl, 0);
    SetOamUpdateCtl(V, ccmMinPtr_f, &oam_update_ctl, ccm_min_ptr);
    SetOamUpdateCtl(V, oamUpPhyNum_f, &oam_update_ctl, oam_up_phy_num);
    SetOamUpdateCtl(V, updInterval_f, &oam_update_ctl, upd_interval);
    SetOamUpdateCtl(V, ccmMaxPtr_f, &oam_update_ctl, ccm_max_ptr);

    /*BFD Timer*/

    bfd_min_ptr = 0;
    oam_bfd_phy_num       = TABLE_MAX_INDEX(lchip, DsEthMep_t) ;
    bfd_upd_interval = (freq *1000) / oam_bfd_phy_num - 1;
    bfd_max_ptr = (freq *1000) / (bfd_upd_interval + 1) + bfd_min_ptr - 1;

    SetOamUpdateCtl(V, bfdUpdEn_f, &oam_update_ctl, 0);
    SetOamUpdateCtl(V, bfdMinptr_f, &oam_update_ctl, bfd_min_ptr);
    SetOamUpdateCtl(V, oamBfdPhyNum_f, &oam_update_ctl, oam_bfd_phy_num);
    SetOamUpdateCtl(V, bfdUpdInterval_f, &oam_update_ctl, bfd_upd_interval);
    SetOamUpdateCtl(V, bfdMaxPtr_f, &oam_update_ctl, bfd_max_ptr);

    cmd = DRV_IOW(OamUpdateCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_update_ctl));

    sal_memset(&oam_ether_cci_ctl, 0, sizeof(OamEtherCciCtl_m));

    quater_cci2_interval = (upd_interval + 1) * 3 * (ccm_max_ptr - ccm_min_ptr + 1);

    SetOamEtherCciCtl(V, quaterCci2Interval_f, &oam_ether_cci_ctl, quater_cci2_interval);
    SetOamEtherCciCtl(V, relativeCci3Interval_f, &oam_ether_cci_ctl, 9);
    SetOamEtherCciCtl(V, relativeCci4Interval_f, &oam_ether_cci_ctl, 9);
    SetOamEtherCciCtl(V, relativeCci5Interval_f, &oam_ether_cci_ctl, 9);
    SetOamEtherCciCtl(V, relativeCci6Interval_f, &oam_ether_cci_ctl, 5);
    SetOamEtherCciCtl(V, relativeCci7Interval_f, &oam_ether_cci_ctl, 9);
    cmd = DRV_IOW(OamEtherCciCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_ether_cci_ctl));

    /*enable update*/
    tmp_field = 1;
    cmd = DRV_IOW(OamUpdateCtl_t, OamUpdateCtl_updEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp_field));

    tmp_field = 1;
    cmd = DRV_IOW(OamUpdateCtl_t, OamUpdateCtl_bfdUpdEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp_field));


    cmd = DRV_IOR(OamUpdateApsCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_update_aps_ctl));
    if (DRV_IS_DUET2(lchip))
    {
        SetOamUpdateApsCtl(V, ethFastApsRemote_f, &oam_update_aps_ctl, 8);
        SetOamUpdateApsCtl(V, ethFastApsLocal_f, &oam_update_aps_ctl, 0);
        SetOamUpdateApsCtl(V, bfdFastApsRemote_f, &oam_update_aps_ctl, 1);
    }
    else
    {
        SetOamUpdateApsCtl(V, ethFastApsRemote_f, &oam_update_aps_ctl, 31);
        SetOamUpdateApsCtl(V, ethFastApsLocal_f, &oam_update_aps_ctl, 7);
        SetOamUpdateApsCtl(V, bfdFastApsRemote_f, &oam_update_aps_ctl, 3);
    }
    cmd = DRV_IOW(OamUpdateApsCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_update_aps_ctl));

    g_oam_master[lchip]->sf_defect_bitmap = CTC_OAM_DEFECT_ALL;
    return CTC_E_NONE;
}


int32
_sys_usw_oam_db_init(uint8 lchip, ctc_oam_global_t* p_com_glb)
{
    uint32  fun_num            = 0;
    uint32 chan_entry_num     = 2048;
    uint32 mep_entry_num      = 0;
    uint32 ma_entry_num       = 0;
    uint32 ma_name_entry_num  = 0;
    uint32 lm_entry_num       = 0;
    uint32 lm_profile_entry_num       = 0;
    uint8  oam_type           = 0;
    int32 ret = CTC_E_NONE;
    sys_usw_opf_t opf   = {0};


    LCHIP_CHECK(lchip);

    /* check init */
    if (g_oam_master[lchip])
    {
        return CTC_E_NONE;
    }

    /*************************************************/
    /*               init global param               */
    /*************************************************/
    g_oam_master[lchip] = (sys_oam_master_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_master_t));
    if (NULL == g_oam_master[lchip])
    {
        goto err;
    }
    sal_memset(g_oam_master[lchip], 0 ,sizeof(sys_oam_master_t));

    /*************************************************/
    /*          init opf for related tables          */
    /*************************************************/
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsEthMep_t,  &mep_entry_num), ret, err);
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsMa_t,  &ma_entry_num), ret, err);
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsMaName_t,  &ma_name_entry_num), ret, err);
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsSrcEthLmProfile_t,  &lm_profile_entry_num), ret, err);
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsOamLmStats_t,  &lm_entry_num), ret, err);

    if ((0 == mep_entry_num) || (0 == ma_entry_num) || (0 == ma_name_entry_num)
        || (0 == lm_profile_entry_num) || (0 == lm_entry_num))
    {
        /*no need to alloc MEP resource on line card in Fabric system*/
        g_oam_master[lchip]->no_mep_resource = 1;
    }
    else
    {
        g_oam_master[lchip]->no_mep_resource = 0;
        CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &g_oam_master[lchip]->oam_opf_type,  SYS_OAM_OPF_TYPE_MAX, "opf-oam"), ret, err);
        g_oam_master[lchip]->rsv_maname_idx = ma_name_entry_num-1;
        opf.pool_type = g_oam_master[lchip]->oam_opf_type;

        opf.pool_index = SYS_OAM_OPF_TYPE_MEP_LMEP;
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 2, mep_entry_num - 2), ret, err);

        opf.pool_index = SYS_OAM_OPF_TYPE_MA;
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 0, ma_entry_num), ret, err);

        opf.pool_index = SYS_OAM_OPF_TYPE_MA_NAME;
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 0, ma_name_entry_num-1), ret, err);

        opf.pool_index = SYS_OAM_OPF_TYPE_LM_PROFILE;
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 0, lm_profile_entry_num), ret, err);

        opf.pool_index = SYS_OAM_OPF_TYPE_LM;
        CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 0, lm_entry_num), ret, err);
    }

    /*IPv6 address */
    g_oam_master[lchip]->ipv6_bmp = mem_malloc(MEM_OAM_MODULE,(MCHIP_CAP(SYS_CAP_OAM_BFD_IPV6_MAX_IPSA_NUM) * 1) * sizeof(sys_bfd_ip_v6_node_t));
    if (NULL == g_oam_master[lchip]->ipv6_bmp )
    {
        goto err;
    }
    sal_memset(g_oam_master[lchip]->ipv6_bmp, 0,(MCHIP_CAP(SYS_CAP_OAM_BFD_IPV6_MAX_IPSA_NUM) * 1) * sizeof(sys_bfd_ip_v6_node_t));

    fun_num = CTC_OAM_MEP_TYPE_MAX * SYS_OAM_MODULE_MAX * SYS_OAM_OP_MAX;
    g_oam_master[lchip]->p_fun_table = (SYS_OAM_CALL_FUNC_P*)
        mem_malloc(MEM_OAM_MODULE, fun_num * sizeof(SYS_OAM_CALL_FUNC_P));
    if (NULL == g_oam_master[lchip]->p_fun_table)
    {
        goto err;
    }

    sal_memset(g_oam_master[lchip]->p_fun_table, 0, fun_num * sizeof(SYS_OAM_CALL_FUNC_P));


    fun_num = CTC_OAM_MEP_TYPE_MAX * SYS_OAM_CHECK_MAX;
    g_oam_master[lchip]->p_check_fun_table = (SYS_OAM_CHECK_CALL_FUNC_P*)
        mem_malloc(MEM_OAM_MODULE, fun_num * sizeof(SYS_OAM_CHECK_CALL_FUNC_P));
    if (NULL == g_oam_master[lchip]->p_check_fun_table)
    {
        goto err;
    }

    sal_memset(g_oam_master[lchip]->p_check_fun_table, 0, fun_num * sizeof(SYS_OAM_CHECK_CALL_FUNC_P));

    /* (ma_name_entry_num/2)/SYS_OAM_MAID_HASH_BLOCK_SIZE, 2 DsMaName per mep*/
    if (!g_oam_master[lchip]->no_mep_resource)
    {
        g_oam_master[lchip]->maid_hash = ctc_hash_create((ma_name_entry_num / (SYS_OAM_MAID_HASH_BLOCK_SIZE * 2)),
                                                         SYS_OAM_MAID_HASH_BLOCK_SIZE,
                                                         (hash_key_fn)_sys_usw_oam_maid_build_key,
                                                         (hash_cmp_fn)_sys_usw_oam_maid_cmp);

        if (NULL == g_oam_master[lchip]->maid_hash)
        {
            goto err;
        }
    }
    g_oam_master[lchip]->chan_hash = ctc_hash_create((chan_entry_num / SYS_OAM_CHAN_HASH_BLOCK_SIZE),
                                              SYS_OAM_CHAN_HASH_BLOCK_SIZE,
                                              (hash_key_fn)_sys_usw_oam_chan_build_key,
                                              (hash_cmp_fn)_sys_usw_oam_chan_cmp);
    if (NULL == g_oam_master[lchip]->chan_hash)
    {
        goto err;
    }
    g_oam_master[lchip]->oamid_hash = ctc_hash_create((chan_entry_num / SYS_OAM_CHAN_HASH_BLOCK_SIZE),
                                              SYS_OAM_CHAN_HASH_BLOCK_SIZE,
                                              (hash_key_fn)_sys_usw_oam_oamid_build_key,
                                              (hash_cmp_fn)_sys_usw_oam_oamid_cmp);
    if (NULL == g_oam_master[lchip]->oamid_hash)
    {
        goto err;
    }
#define SYS_OAM_MEP_VEC_BLOCK_NUM 32
    if (!g_oam_master[lchip]->no_mep_resource)
    {
        g_oam_master[lchip]->mep_vec =
        ctc_vector_init(SYS_OAM_MEP_VEC_BLOCK_NUM, mep_entry_num / SYS_OAM_MEP_VEC_BLOCK_NUM);
        if (NULL == g_oam_master[lchip]->mep_vec)
        {
            goto err;
        }
    }
    if (CTC_E_NONE != sal_mutex_create(&(g_oam_master[lchip]->oam_mutex)))
    {
        goto err;
    }

    g_oam_master[lchip]->maid_len_format = p_com_glb->maid_len_format;
    g_oam_master[lchip]->mep_index_alloc_by_sdk = p_com_glb->mep_index_alloc_by_sdk;
    g_oam_master[lchip]->tp_section_oam_based_l3if = p_com_glb->tp_section_oam_based_l3if;

    /*temp for testing !!!!!!*/
    CTC_ERROR_GOTO(_sys_usw_oam_defect_scan_en(lchip, TRUE), ret, err);
    CTC_ERROR_GOTO(_sys_usw_oam_com_reg_init(lchip), ret, err);


    oam_type = CTC_OAM_PROPERTY_TYPE_COMMON;

    SYS_OAM_FUNC_TABLE(lchip, oam_type, SYS_OAM_GLOB, SYS_OAM_CONFIG) = _sys_usw_oam_set_common_property;

    sys_usw_ftm_register_callback(lchip, CTC_FTM_SPEC_OAM, sys_usw_oam_ftm_cb);

    SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MEP, 1, 2);

    MCHIP_CAP(SYS_CAP_SPEC_OAM_SESSION_NUM) = mep_entry_num / 2;
    goto success;

    /*************************************************/
    /*               Error rollback                  */
    /*************************************************/

err:

    if (NULL != g_oam_master[lchip]->oam_mutex)
    {
        sal_mutex_destroy(g_oam_master[lchip]->oam_mutex);
        g_oam_master[lchip]->oam_mutex = NULL;
    }

    if (NULL != g_oam_master[lchip]->mep_vec)
    {
        ctc_vector_release(g_oam_master[lchip]->mep_vec);
    }

    if (NULL != g_oam_master[lchip]->chan_hash)
    {
        ctc_hash_free(g_oam_master[lchip]->chan_hash);
    }

    if (NULL != g_oam_master[lchip]->maid_hash)
    {
        ctc_hash_free(g_oam_master[lchip]->maid_hash);
    }

    if (NULL != g_oam_master[lchip]->oamid_hash)
    {
        ctc_hash_free(g_oam_master[lchip]->oamid_hash);
    }

    if (NULL != g_oam_master[lchip]->p_fun_table)
    {
        mem_free(g_oam_master[lchip]->p_fun_table);
    }

    if(0 != g_oam_master[lchip]->oam_opf_type)
    {
        sys_usw_opf_deinit(lchip, g_oam_master[lchip]->oam_opf_type);
    }
    if (NULL != g_oam_master[lchip])
    {
        mem_free(g_oam_master[lchip]);
    }

    return CTC_E_NO_MEMORY;

success:
    return CTC_E_NONE;

}

STATIC int32
_sys_usw_oam_db_free_node_data(void* node_data, void* user_data)
{
    mem_free(node_data);
    return CTC_E_NONE;
}

int32
_sys_usw_oam_db_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == g_oam_master[lchip])
    {
        return CTC_E_NONE;
    }

    /*free chan*/
    ctc_hash_traverse(g_oam_master[lchip]->chan_hash, (hash_traversal_fn)_sys_usw_oam_db_free_node_data, NULL);
    ctc_hash_free(g_oam_master[lchip]->chan_hash);

    /*free mep vector*/
    ctc_vector_traverse(g_oam_master[lchip]->mep_vec, (vector_traversal_fn)_sys_usw_oam_db_free_node_data, NULL);
    ctc_vector_release(g_oam_master[lchip]->mep_vec);

    /*free maid*/
    ctc_hash_traverse(g_oam_master[lchip]->maid_hash, (hash_traversal_fn)_sys_usw_oam_db_free_node_data, NULL);
    ctc_hash_free(g_oam_master[lchip]->maid_hash);

    ctc_hash_traverse(g_oam_master[lchip]->oamid_hash, (hash_traversal_fn)_sys_usw_oam_db_free_node_data, NULL);
    ctc_hash_free(g_oam_master[lchip]->oamid_hash);

    mem_free(g_oam_master[lchip]->p_check_fun_table);
    mem_free(g_oam_master[lchip]->p_fun_table);
    mem_free(g_oam_master[lchip]->ipv6_bmp);
    mem_free(g_oam_master[lchip]->mep_defect_bitmap);

    sys_usw_opf_deinit(lchip, g_oam_master[lchip]->oam_opf_type);


    sal_mutex_destroy(g_oam_master[lchip]->oam_mutex);
    mem_free(g_oam_master[lchip]);

    return CTC_E_NONE;
}




#define WB_OAM "Function Begin"

int32
_sys_usw_oam_wb_mapping_rmep(uint8 lchip, sys_wb_oam_chan_t *p_wb_oam, sys_oam_rmep_t *p_oam_rmep, uint8 sync)
{
    if (sync)
    {
        p_wb_oam->rmep_flag = p_oam_rmep->flag;
        p_wb_oam->rmep_index = p_oam_rmep->rmep_index;
        p_wb_oam->rmep_hash_key_index = p_oam_rmep->key_index;
        p_wb_oam->rmep_md_level = p_oam_rmep->md_level;
        p_wb_oam->rmep_id = p_oam_rmep->rmep_id;
    }
    else
    {
        p_oam_rmep->flag = p_wb_oam->rmep_flag;
        p_oam_rmep->rmep_index = p_wb_oam->rmep_index;
        p_oam_rmep->key_index = p_wb_oam->rmep_hash_key_index;
        p_oam_rmep->md_level = p_wb_oam->rmep_md_level;
        p_oam_rmep->rmep_id = p_wb_oam->rmep_id;
    }
    return CTC_E_NONE;
}

int32
_sys_usw_oam_wb_mapping_maid(uint8 lchip, sys_wb_oam_chan_t *p_wb_oam, sys_oam_maid_com_t *p_sys_maid, uint8 sync, uint8 mep_type, uint16 ma_index)
{
    uint32 cmd = 0;
    uint32 maid_field[2] = {0};
    uint8  entry_index    = 0;
    DsMa_m ds_ma;
    DsMaName_m ds_ma_name;

    if (sync)
    {
        p_wb_oam->maid_entry_num = p_sys_maid->maid_entry_num;
        p_wb_oam->ref_cnt = p_sys_maid->ref_cnt;
    }
    else
    {
        sal_memset(&ds_ma, 0, sizeof(DsMa_m));
        sal_memset(&ds_ma_name, 0, sizeof(DsMaName_m));
        cmd = DRV_IOR(DsMaName_t, DRV_ENTRY_FLAG);

        p_sys_maid->ref_cnt = p_wb_oam->ref_cnt;

        cmd = DRV_IOR(DsMa_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, ma_index, cmd, &ds_ma));
        p_sys_maid->mep_type = mep_type;
        p_sys_maid->maid_len = GetDsMa(V, maNameLen_f,&ds_ma);
        p_sys_maid->maid_index = GetDsMa(V, maNameIndex_f,&ds_ma);
        p_sys_maid->maid_entry_num = (GetDsMa(V, maIdLengthType_f,&ds_ma) << 1);
        cmd = DRV_IOR(DsMaName_t, DRV_ENTRY_FLAG);
        for (entry_index = 0; entry_index < p_sys_maid->maid_entry_num; entry_index++)
        {
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_sys_maid->maid_index + entry_index, cmd, &ds_ma_name));
            DRV_GET_FIELD_A(lchip, DsMaName_t, DsMaName_maIdUmc_f, &ds_ma_name, maid_field);

            p_sys_maid->maid[entry_index * 8 + 0]  = (maid_field[1] >> 24) & 0xFF;
            p_sys_maid->maid[entry_index * 8 + 1]  = (maid_field[1] >> 16) & 0xFF;
            p_sys_maid->maid[entry_index * 8 + 2]  = (maid_field[1] >> 8) & 0xFF;
            p_sys_maid->maid[entry_index * 8 + 3]  = (maid_field[1] >> 0) & 0xFF;

            p_sys_maid->maid[entry_index * 8 + 4]  = (maid_field[0] >> 24) & 0xFF;
            p_sys_maid->maid[entry_index * 8 + 5]  = (maid_field[0] >> 16) & 0xFF;
            p_sys_maid->maid[entry_index * 8 + 6]  = (maid_field[0] >> 8) & 0xFF;
            p_sys_maid->maid[entry_index * 8 + 7]  = (maid_field[0] >> 0) & 0xFF;

            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "-----ds_ma_name(maIdx:%d, entryIdx:%d)--------\n", p_sys_maid->maid_index, entry_index);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ds_ma_name.ma_id_umc0:0x%x\n", maid_field[0]);
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ds_ma_name.ma_id_umc1:0x%x\n", maid_field[1]);
        }


    }


    return CTC_E_NONE;

}


int32
_sys_usw_oam_wb_mapping_lmep(uint8 lchip, sys_wb_oam_chan_t *p_wb_oam, sys_oam_lmep_t *p_oam_lmep, uint8 sync, uint8 mep_type)
{
    uint32 cmd = 0;
    uint32 index = 0;
    uint8 hashType = 0;
    uint8 block_size = 0;
    uint32 offset = 0;
    DsBfdMep_m  ds_bfd_mep;
    DsEthMep_m  ds_eth_mep;
    DsMa_m ds_ma;
    DsEgressXcOamMplsLabelHashKey_m ds_mpls_oam_key;
    DsEgressXcOamMplsSectionHashKey_m ds_section_oam_key;
    DsEgressXcOamEthHashKey_m eth_oam_key;


    if (sync)
    {
        p_wb_oam->lmep_flag = p_oam_lmep->flag;
        p_wb_oam->nhid = p_oam_lmep->nhid;
        p_wb_oam->local_discr = p_oam_lmep->local_discr;
        p_wb_oam->lmep_index = p_oam_lmep->lmep_index;
        p_wb_oam->lmep_lm_index_alloced = p_oam_lmep->lm_index_alloced;   /*used for ether*/
        p_wb_oam->lock_en = p_oam_lmep->lock_en;
        p_wb_oam->md_level = p_oam_lmep->md_level;
    }
    else
    {
        sys_usw_opf_t opf;
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

        /*restore from wb data*/
        p_oam_lmep->flag = p_wb_oam->lmep_flag;
        p_oam_lmep->nhid = p_wb_oam->nhid;
        p_oam_lmep->local_discr = p_wb_oam->local_discr;
        p_oam_lmep->spaceid = p_wb_oam->spaceid;
        p_oam_lmep->mpls_in_label = p_wb_oam->label;
        p_oam_lmep->lmep_index = p_wb_oam->lmep_index;
        p_oam_lmep->lm_index_alloced = p_wb_oam->lmep_lm_index_alloced;
        p_oam_lmep->lock_en = p_wb_oam->lock_en;
        p_oam_lmep->md_level = p_wb_oam->md_level;

        /*restore from asic*/
        sal_memset(&ds_bfd_mep, 0, sizeof(DsBfdMep_m));
        sal_memset(&ds_eth_mep, 0, sizeof(DsEthMep_m));
        sal_memset(&ds_ma, 0, sizeof(DsMa_m));
        sal_memset(&ds_mpls_oam_key, 0, sizeof(DsEgressXcOamMplsLabelHashKey_m));
        sal_memset(&ds_section_oam_key, 0, sizeof(DsEgressXcOamMplsSectionHashKey_m));
        sal_memset(&eth_oam_key, 0, sizeof(DsEgressXcOamEthHashKey_m));
        index = p_wb_oam->hash_key_index;
        CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, DsEgressXcOamMplsLabelHashKey_t, index, &ds_mpls_oam_key));
        hashType = GetDsEgressXcOamMplsLabelHashKey(V, hashType_f, &ds_mpls_oam_key);
        if(EGRESSXCOAMHASHTYPE_MPLSLABEL == hashType)
        {
            CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, DsEgressXcOamMplsLabelHashKey_t, index, &ds_mpls_oam_key));
            p_oam_lmep->lm_cos = GetDsEgressXcOamMplsLabelHashKey(V, lmCos_f, &ds_mpls_oam_key);
            p_oam_lmep->lm_cos_type = GetDsEgressXcOamMplsLabelHashKey(V, lmCosType_f, &ds_mpls_oam_key);
            p_oam_lmep->lm_index_base = GetDsEgressXcOamMplsLabelHashKey(V, lmIndexBase_f, &ds_mpls_oam_key);
        }
        else if(EGRESSXCOAMHASHTYPE_MPLSSECTION == hashType)
        {
            CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, DsEgressXcOamMplsSectionHashKey_t, index, &ds_section_oam_key));
            p_oam_lmep->lm_cos = GetDsEgressXcOamMplsSectionHashKey(V, lmCos_f, &ds_section_oam_key);
            p_oam_lmep->lm_cos_type = GetDsEgressXcOamMplsSectionHashKey(V, lmCosType_f, &ds_section_oam_key);
            p_oam_lmep->lm_index_base = GetDsEgressXcOamMplsSectionHashKey(V, lmIndexBase_f, &ds_section_oam_key) ;
        }


        /*restore from OAM table*/
        if(0x1FFF  != p_oam_lmep->lmep_index)
        {
            cmd = DRV_IOR(DsBfdMep_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_oam_lmep->lmep_index, cmd, &ds_bfd_mep));
            cmd = DRV_IOR(DsEthMep_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_oam_lmep->lmep_index, cmd, &ds_eth_mep));
        }
        if ((CTC_OAM_MEP_TYPE_ETH_1AG == mep_type) || (CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_type) || (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == mep_type))

        {
            p_oam_lmep->ma_index = GetDsEthMep(V, maIndex_f, &ds_eth_mep);
            p_oam_lmep->is_up = GetDsEthMep(V, isUp_f, &ds_eth_mep);
        }
        else
        {
            p_oam_lmep->ma_index = GetDsBfdMep(V, maIndex_f, &ds_bfd_mep);
        }

        cmd = DRV_IOR(DsMa_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_oam_lmep->ma_index, cmd, &ds_ma));
        p_oam_lmep->vlan_domain = GetDsMa(V, outerVlanIsCVlan_f,&ds_ma);

        if ((CTC_OAM_MEP_TYPE_ETH_1AG == mep_type) || (CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_type))
        {
            sys_oam_chan_eth_t* p_oam_chan_eth = NULL;
            p_oam_chan_eth = (sys_oam_chan_eth_t*)p_oam_lmep->p_sys_chan;
            p_oam_lmep->lm_cos = p_oam_chan_eth->lm_cos[p_oam_lmep->md_level];
            p_oam_lmep->lm_cos_type = p_oam_chan_eth->lm_cos_type[p_oam_lmep->md_level];
            p_oam_lmep->lm_index_base = p_oam_chan_eth->lm_index[p_oam_lmep->md_level];
            if(p_oam_chan_eth->key.link_oam)
            {
                SET_BIT((p_oam_chan_eth->down_mep_bitmap), p_oam_lmep->md_level);
            }
            /*Linkoam LM OPF restore*/
            if ((p_wb_oam->link_oam) && CTC_FLAG_ISSET(p_oam_lmep->flag, CTC_OAM_Y1731_LMEP_FLAG_LM_EN))
            {
                block_size = (CTC_OAM_LM_COS_TYPE_PER_COS == p_oam_chan_eth->link_lm_cos_type) ? 8 : 1;
                offset = p_oam_chan_eth->com.link_lm_index_base;
                opf.pool_index  = SYS_OAM_OPF_TYPE_LM;
                opf.pool_type = g_oam_master[lchip]->oam_opf_type;
                sys_usw_opf_alloc_offset_from_position(lchip, &opf, block_size, offset);
                p_oam_chan_eth->link_lm_index_alloced = TRUE;
            }
        }

        p_oam_lmep->mep_on_cpu = (0x1FFF == p_oam_lmep->lmep_index) ? 1 : 0;

        /*LM OPF restore*/
        if ((CTC_OAM_LM_COS_TYPE_ALL_COS == p_oam_lmep->lm_cos_type) || (CTC_OAM_LM_COS_TYPE_SPECIFIED_COS == p_oam_lmep->lm_cos_type))
        {
            block_size = 1;
        }
        else if (CTC_OAM_LM_COS_TYPE_PER_COS == p_oam_lmep->lm_cos_type)
        {
            block_size = 8;
        }
        if (p_oam_lmep->lm_index_alloced)
        {
            offset = p_oam_lmep->lm_index_base;
            opf.pool_index  = SYS_OAM_OPF_TYPE_LM;
            opf.pool_type = g_oam_master[lchip]->oam_opf_type;
            sys_usw_opf_alloc_offset_from_position(lchip, &opf, block_size, offset);
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_oam_wb_mapping_chan_eth(uint8 lchip, sys_wb_oam_chan_t *p_wb_oam, sys_oam_chan_eth_t *p_oam_chan_eth, uint8 sync)
{
    uint32 index = 0;
    uint32 cmd = 0;
    uint8 level = 0;
    uint8 count = 0;
    uint32 mep_index[4] = {0};
    uint16 lm_index[4] = {0};
    uint8  lm_cos[4] = {0};
    uint8  lm_cos_type[4] = {0};
    uint32 mode = 0;
    if (sync)
    {
        p_wb_oam->hash_key_index = p_oam_chan_eth->key.com.key_index;
        p_wb_oam->mep_type = p_oam_chan_eth->com.mep_type;
        p_wb_oam->lm_num = p_oam_chan_eth->com.lm_num;
        p_wb_oam->link_oam = p_oam_chan_eth->key.link_oam;
        p_wb_oam->is_vpws = p_oam_chan_eth->key.is_vpws;
        p_wb_oam->gport = p_oam_chan_eth->key.gport;
        p_wb_oam->eth_lm_index_alloced = p_oam_chan_eth->lm_index_alloced;
        p_wb_oam->vlan_id = p_oam_chan_eth->key.vlan_id;
        p_wb_oam->mip_bitmap = p_oam_chan_eth->mip_bitmap;
        p_wb_oam->tp_oam_key_index[0] = p_oam_chan_eth->tp_oam_key_index[0];
        p_wb_oam->tp_oam_key_index[1] = p_oam_chan_eth->tp_oam_key_index[1];
        p_wb_oam->tp_oam_key_valid[0] = p_oam_chan_eth->tp_oam_key_valid[0];
        p_wb_oam->tp_oam_key_valid[1] = p_oam_chan_eth->tp_oam_key_valid[1];
    }
    else
    {
        sys_usw_opf_t opf;
        DsEgressXcOamEthHashKey_m eth_oam_key;
        DsSrcPort_m ds_src_port;
        DsEthLmProfile_m  ds_eth_lm_profile;
        sal_memset(&eth_oam_key, 0, sizeof(DsEgressXcOamEthHashKey_m));
        sal_memset(&ds_src_port, 0, sizeof(DsSrcPort_m));
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));


        /*restore from wb data*/
        p_oam_chan_eth->com.mep_type = p_wb_oam->mep_type;
        p_oam_chan_eth->com.lm_num = p_wb_oam->lm_num;
        p_oam_chan_eth->key.com.key_index = p_wb_oam->hash_key_index;
        p_oam_chan_eth->key.com.mep_type =  p_wb_oam->mep_type;
        p_oam_chan_eth->key.com.key_alloc = SYS_OAM_KEY_HASH;
        p_oam_chan_eth->key.com.key_exit = 1;
        p_oam_chan_eth->key.is_vpws = p_wb_oam->is_vpws;
        p_oam_chan_eth->key.link_oam = p_wb_oam->link_oam;
        p_oam_chan_eth->key.gport = p_wb_oam->gport;
        p_oam_chan_eth->lm_index_alloced = p_wb_oam->eth_lm_index_alloced;
        p_oam_chan_eth->key.vlan_id = p_wb_oam->vlan_id;
        p_oam_chan_eth->mip_bitmap = p_wb_oam->mip_bitmap;
        p_oam_chan_eth->tp_oam_key_index[0] = p_wb_oam->tp_oam_key_index[0];
        p_oam_chan_eth->tp_oam_key_index[1] = p_wb_oam->tp_oam_key_index[1];
        p_oam_chan_eth->tp_oam_key_valid[0] = p_wb_oam->tp_oam_key_valid[0];
        p_oam_chan_eth->tp_oam_key_valid[1] = p_wb_oam->tp_oam_key_valid[1];
        /*restore from asic*/
        index = p_wb_oam->hash_key_index;
        CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, DsEgressXcOamEthHashKey_t, index, &eth_oam_key));
        p_oam_chan_eth->key.cvlan_id = GetDsEgressXcOamEthHashKey(V, cvlanId_f, &eth_oam_key);
        p_oam_chan_eth->key.use_fid = GetDsEgressXcOamEthHashKey(V, isFid_f, &eth_oam_key);
        p_oam_chan_eth->key.is_cvlan = GetDsEgressXcOamEthHashKey(V, isCvlan_f, &eth_oam_key);
        p_oam_chan_eth->key.l2vpn_oam_id = p_oam_chan_eth->key.use_fid ? GetDsEgressXcOamEthHashKey(V, vlanId_f, &eth_oam_key):0;
        if(p_oam_chan_eth->key.is_vpws && (p_oam_chan_eth->key.l2vpn_oam_id > (8*1024)))
        {
            p_oam_chan_eth->key.l2vpn_oam_id -= 8*1024;
            sys_usw_global_ctl_get(lchip, CTC_GLOBAL_TPOAM_VPWSOAM_COEXIST, &mode);
            p_oam_chan_eth->key.l2vpn_oam_id = (mode && DRV_IS_DUET2(lchip) && (p_oam_chan_eth->key.l2vpn_oam_id > 2048))
                              ? (p_oam_chan_eth->key.l2vpn_oam_id - 2048) : p_oam_chan_eth->key.l2vpn_oam_id;
        }
        p_oam_chan_eth->up_mep_bitmap = GetDsEgressXcOamEthHashKey(V, mepUpBitmap_f, &eth_oam_key);
        p_oam_chan_eth->down_mep_bitmap = GetDsEgressXcOamEthHashKey(V, mepDownBitmap_f, &eth_oam_key);
        p_oam_chan_eth->lm_bitmap = GetDsEgressXcOamEthHashKey(V, lmBitmap_f, &eth_oam_key);
        p_oam_chan_eth->lm_type = GetDsEgressXcOamEthHashKey(V, lmType_f, &eth_oam_key);
        p_oam_chan_eth->com.lm_index_base = GetDsEgressXcOamEthHashKey(V, lmProfileIndex_f,&eth_oam_key);

        if (p_oam_chan_eth->lm_index_alloced)  /*restore opf index for DsSrcEthLmProfile*/
        {
            opf.pool_index  = SYS_OAM_OPF_TYPE_LM_PROFILE;
            opf.pool_type = g_oam_master[lchip]->oam_opf_type;
            sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_oam_chan_eth->com.lm_index_base);
        }

        if (p_oam_chan_eth->lm_bitmap)
        {
            sal_memset(&ds_eth_lm_profile, 0, sizeof(DsEthLmProfile_m));
            cmd = DRV_IOR(DsSrcEthLmProfile_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_oam_chan_eth->com.lm_index_base, cmd, &ds_eth_lm_profile));
            lm_cos[0] = GetDsEthLmProfile(V, array_0_lmCos_f, &ds_eth_lm_profile);
            lm_cos[1] = GetDsEthLmProfile(V, array_1_lmCos_f, &ds_eth_lm_profile);
            lm_cos[2] = GetDsEthLmProfile(V, array_2_lmCos_f, &ds_eth_lm_profile);
            lm_cos[3] = GetDsEthLmProfile(V, array_3_lmCos_f, &ds_eth_lm_profile);

            lm_index[0] = GetDsEthLmProfile(V, array_0_lmIndexBase_f, &ds_eth_lm_profile);
            lm_index[1] = GetDsEthLmProfile(V, array_1_lmIndexBase_f, &ds_eth_lm_profile);
            lm_index[2] = GetDsEthLmProfile(V, array_2_lmIndexBase_f, &ds_eth_lm_profile);
            lm_index[3] = GetDsEthLmProfile(V, array_3_lmIndexBase_f, &ds_eth_lm_profile);

            lm_cos_type[0] = GetDsEgressXcOamEthHashKey(V, array_0_lmCosType_f, &eth_oam_key);
            lm_cos_type[1] = GetDsEgressXcOamEthHashKey(V, array_1_lmCosType_f, &eth_oam_key);
            lm_cos_type[2] = GetDsEgressXcOamEthHashKey(V, array_2_lmCosType_f, &eth_oam_key);
            lm_cos_type[3] = GetDsEgressXcOamEthHashKey(V, array_3_lmCosType_f, &eth_oam_key);

        }

        mep_index[0] = GetDsEgressXcOamEthHashKey(V, array_0_mepIndex_f, &eth_oam_key);
        mep_index[1] = GetDsEgressXcOamEthHashKey(V, array_1_mepIndex_f, &eth_oam_key);
        mep_index[2] = GetDsEgressXcOamEthHashKey(V, array_2_mepIndex_f, &eth_oam_key);
        mep_index[3] = GetDsEgressXcOamEthHashKey(V, array_3_mepIndex_f, &eth_oam_key);

        for (level = 0; level <= SYS_OAM_MAX_MD_LEVEL; level++)
        {
            if ((IS_BIT_SET(p_oam_chan_eth->down_mep_bitmap, level)) ||
                (IS_BIT_SET(p_oam_chan_eth->up_mep_bitmap, level)))
            {
                p_oam_chan_eth->mep_index[level] = mep_index[count];
                p_oam_chan_eth->lm_cos_type[level] = lm_cos_type[count];
                p_oam_chan_eth->lm_index[level] = lm_index[count];
                p_oam_chan_eth->lm_cos[level] = lm_cos[count];
                count++;
                if (4 == count)/*per chan max have 4 lmep*/
                {
                    break;
                }
            }
        }

        if (!CTC_IS_LINKAGG_PORT(p_oam_chan_eth->key.gport))
        {
            cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, CTC_MAP_GPORT_TO_LPORT(p_oam_chan_eth->key.gport), cmd, &ds_src_port));
            p_oam_chan_eth->link_lm_type = GetDsSrcPort(V, linkLmType_f, &ds_src_port);
            p_oam_chan_eth->link_lm_cos_type = GetDsSrcPort(V, linkLmCosType_f, &ds_src_port);
            p_oam_chan_eth->link_lm_cos = GetDsSrcPort(V, linkLmCos_f, &ds_src_port);
            p_oam_chan_eth->com.link_lm_index_base = GetDsSrcPort(V, linkLmIndexBase_f, &ds_src_port);
        }
    }

    return CTC_E_NONE;
}


int32
_sys_usw_oam_wb_mapping_chan_tp(uint8 lchip, sys_wb_oam_chan_t *p_wb_oam, sys_oam_chan_tp_t *p_oam_chan_tp, uint8 sync)
{
    uint32 index = 0;
    uint8 hashType = 0;
    uint8 block_size = 0;
    uint32 offset = 0;
    DsEgressXcOamMplsLabelHashKey_m ds_mpls_oam_key;
    DsEgressXcOamMplsSectionHashKey_m ds_section_oam_key;
    sal_memset(&ds_mpls_oam_key, 0, sizeof(DsEgressXcOamMplsLabelHashKey_m));
    sal_memset(&ds_section_oam_key, 0, sizeof(DsEgressXcOamMplsSectionHashKey_m));
    if (sync)
    {
        p_wb_oam->hash_key_index = p_oam_chan_tp->key.com.key_index;
        p_wb_oam->mep_type = p_oam_chan_tp->com.mep_type;
        p_wb_oam->label = p_oam_chan_tp->key.label;
        p_wb_oam->spaceid = p_oam_chan_tp->key.spaceid;
	    p_wb_oam->tp_lm_index_alloced = p_oam_chan_tp->lm_index_alloced;
    }
    else
    {
        sys_usw_opf_t opf;
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

        /*restore from wb data*/
        p_oam_chan_tp->key.com.key_exit = 1;
        p_oam_chan_tp->key.com.key_index = p_wb_oam->hash_key_index;
        p_oam_chan_tp->com.mep_type = p_wb_oam->mep_type;
        p_oam_chan_tp->key.label =  p_wb_oam->label;
        p_oam_chan_tp->key.spaceid = p_wb_oam->spaceid;
        p_oam_chan_tp->key.com.mep_type =  p_wb_oam->mep_type;
        p_oam_chan_tp->key.com.key_alloc = SYS_OAM_KEY_HASH;
	    p_oam_chan_tp->lm_index_alloced = p_wb_oam->tp_lm_index_alloced;

        /*restore from asic*/
        index = p_wb_oam->hash_key_index;
        CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, DsEgressXcOamMplsLabelHashKey_t, index, &ds_mpls_oam_key));
        hashType = GetDsEgressXcOamMplsLabelHashKey(V, hashType_f, &ds_mpls_oam_key);
        if(EGRESSXCOAMHASHTYPE_MPLSLABEL == hashType)
        {
            CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, DsEgressXcOamMplsLabelHashKey_t, index, &ds_mpls_oam_key));
            p_oam_chan_tp->mep_index = GetDsEgressXcOamMplsLabelHashKey(V, mepIndex_f, &ds_mpls_oam_key);
            p_oam_chan_tp->com.lm_index_base = GetDsEgressXcOamMplsLabelHashKey(V, lmIndexBase_f, &ds_mpls_oam_key);
            p_oam_chan_tp->com.master_chipid = GetDsEgressXcOamMplsLabelHashKey(V, oamDestChipId_f, &ds_mpls_oam_key);
            p_oam_chan_tp->mep_index_in_key =  GetDsEgressXcOamMplsLabelHashKey(V, mplsOamIndex_f, &ds_mpls_oam_key);
            p_oam_chan_tp->lm_cos_type = GetDsEgressXcOamMplsLabelHashKey(V, lmCosType_f, &ds_mpls_oam_key);
            p_oam_chan_tp->lm_cos = GetDsEgressXcOamMplsLabelHashKey(V, lmCos_f, &ds_mpls_oam_key);
            p_oam_chan_tp->lm_type = GetDsEgressXcOamMplsLabelHashKey(V, lmType_f, &ds_mpls_oam_key);
            p_oam_chan_tp->key.section_oam = 0;
        }
        else if(EGRESSXCOAMHASHTYPE_MPLSSECTION == hashType)/*section_oam*/
        {
            CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, DsEgressXcOamMplsSectionHashKey_t, index, &ds_section_oam_key));
            p_oam_chan_tp->mep_index = GetDsEgressXcOamMplsSectionHashKey(V, mepIndex_f, &ds_section_oam_key);
            p_oam_chan_tp->com.lm_index_base = GetDsEgressXcOamMplsSectionHashKey(V, lmIndexBase_f, &ds_section_oam_key) ;
            p_oam_chan_tp->com.master_chipid = GetDsEgressXcOamMplsSectionHashKey(V, oamDestChipId_f, &ds_section_oam_key);
            p_oam_chan_tp->key.gport_l3if_id = GetDsEgressXcOamMplsSectionHashKey(V, interfaceId_f, &ds_section_oam_key);
            p_oam_chan_tp->lm_cos = GetDsEgressXcOamMplsSectionHashKey(V, lmCos_f, &ds_section_oam_key);
            p_oam_chan_tp->lm_cos_type = GetDsEgressXcOamMplsSectionHashKey(V, lmCosType_f, &ds_section_oam_key);
            p_oam_chan_tp->lm_cos = GetDsEgressXcOamMplsSectionHashKey(V, lmCos_f, &ds_section_oam_key);
            p_oam_chan_tp->lm_type = GetDsEgressXcOamMplsSectionHashKey(V, lmType_f, &ds_section_oam_key);
            p_oam_chan_tp->key.section_oam = 1;
        }

        p_oam_chan_tp->mep_on_cpu = (0x1FFF == p_oam_chan_tp->mep_index) ? 1: 0;

         /*LM OPF restore*/
        if ((CTC_OAM_LM_COS_TYPE_ALL_COS == p_oam_chan_tp->lm_cos_type) || (CTC_OAM_LM_COS_TYPE_SPECIFIED_COS == p_oam_chan_tp->lm_cos_type))
        {
            block_size = 1;
        }
        else if (CTC_OAM_LM_COS_TYPE_PER_COS == p_oam_chan_tp->lm_cos_type)
        {
            block_size = 8;
        }
        if (p_oam_chan_tp->lm_index_alloced)
        {
            offset = p_oam_chan_tp->com.lm_index_base;
            opf.pool_index  = SYS_OAM_OPF_TYPE_LM;
            opf.pool_type = g_oam_master[lchip]->oam_opf_type;
            sys_usw_opf_alloc_offset_from_position(lchip, &opf, block_size, offset);
        }



    }

    return CTC_E_NONE;
}

int32
_sys_usw_oam_wb_mapping_chan(uint8 lchip, sys_wb_oam_chan_t *p_wb_oam, sys_oam_chan_bfd_t *p_oam_bfd_chan, uint8 sync)
{
    uint32 index = 0;
    DsEgressXcOamBfdHashKey_m ds_bfd_oam_key;


    if (sync)
    {
        p_wb_oam->hash_key_index = p_oam_bfd_chan->key.com.key_index;
        p_wb_oam->mep_type = p_oam_bfd_chan->com.mep_type;
    }
    else
    {
        sal_memset(&ds_bfd_oam_key, 0, sizeof(DsEgressXcOamBfdHashKey_m));
        p_oam_bfd_chan->key.com.mep_type =  p_wb_oam->mep_type;
        p_oam_bfd_chan->com.mep_type = p_wb_oam->mep_type;
        p_oam_bfd_chan->key.com.key_index = p_wb_oam->hash_key_index;
        index = p_oam_bfd_chan->key.com.key_index;
        CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, DsEgressXcOamBfdHashKey_t, index, &ds_bfd_oam_key));

        p_oam_bfd_chan->mep_index = GetDsEgressXcOamBfdHashKey(V, mepIndex_f, &ds_bfd_oam_key);
        p_oam_bfd_chan->key.my_discr = GetDsEgressXcOamBfdHashKey(V, myDiscriminator_f, &ds_bfd_oam_key);
        p_oam_bfd_chan->com.master_chipid = GetDsEgressXcOamBfdHashKey(V, oamDestChipId_f, &ds_bfd_oam_key);
        p_oam_bfd_chan->key.com.key_alloc = SYS_OAM_KEY_HASH;
        p_oam_bfd_chan->key.is_sbfd_reflector = GetDsEgressXcOamBfdHashKey(V, isSbfdReflector_f, &ds_bfd_oam_key);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_oam_wb_sync_misc(uint8 lchip, sys_wb_oam_chan_t *p_wb_oam, sys_oam_chan_com_t *p_oam_chan_com, sys_oam_lmep_t *p_sys_lmep)
{
    uint8 mep_type = 0;
    sys_oam_chan_bfd_t *p_oam_chan_bfd = NULL;
    sys_oam_chan_tp_t *p_oam_chan_tp = NULL;
    sys_oam_chan_eth_t *p_oam_chan_eth = NULL;

    mep_type = p_oam_chan_com->mep_type;
    if ( (CTC_OAM_MEP_TYPE_ETH_1AG == mep_type) || (CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_type))
    {
        p_oam_chan_eth = (sys_oam_chan_eth_t*)p_oam_chan_com;
        CTC_ERROR_RETURN(_sys_usw_oam_wb_mapping_chan_eth(lchip, p_wb_oam, p_oam_chan_eth, 1));
    }
    else if ((CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)|| (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == mep_type))
    {
        p_oam_chan_tp = (sys_oam_chan_tp_t*)p_oam_chan_com;
        CTC_ERROR_RETURN(_sys_usw_oam_wb_mapping_chan_tp(lchip, p_wb_oam, p_oam_chan_tp, 1));
    }
    else
    {
        p_oam_chan_bfd = (sys_oam_chan_bfd_t*)p_oam_chan_com;
        CTC_ERROR_RETURN(_sys_usw_oam_wb_mapping_chan(lchip, p_wb_oam, p_oam_chan_bfd, 1));
    }

    CTC_ERROR_RETURN(_sys_usw_oam_wb_mapping_lmep(lchip, p_wb_oam, p_sys_lmep, 1, mep_type));
    /*lmep with maid*/
    if (NULL != p_sys_lmep->p_sys_maid)
    {
        p_wb_oam->with_maid = 1;
        CTC_ERROR_RETURN(_sys_usw_oam_wb_mapping_maid(lchip, p_wb_oam, p_sys_lmep->p_sys_maid, 1, mep_type, p_sys_lmep->ma_index));
    }
    return CTC_E_NONE;
}
int32
_sys_usw_oam_wb_sync_func(sys_oam_chan_com_t *p_oam_chan_com, void* user_data)
{
    uint16 max_entry_cnt = 0;
    sys_wb_oam_chan_t* p_wb_oam = NULL;
    sys_oam_lmep_t* p_sys_lmep = NULL;
    sys_oam_rmep_t* p_sys_rmep = NULL;
    ctc_slistnode_t* ctc_slistnode = NULL;
    ctc_slistnode_t* ctc_slistnode_temp = NULL;
    ctc_slist_t* p_rmep_list = NULL;
    uint8 lchip = 0;

    sys_traverse_t *traversal_data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)traversal_data->data;
    lchip = (uint8)traversal_data->value1;

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);
    p_wb_oam = (sys_wb_oam_chan_t *)wb_data->buffer + wb_data->valid_cnt;
    sal_memset(p_wb_oam, 0, sizeof(sys_wb_oam_chan_t));

    /*start sync lmep db*/
    CTC_SLIST_LOOP(p_oam_chan_com->lmep_list, ctc_slistnode)
    {
        p_wb_oam = (sys_wb_oam_chan_t *)wb_data->buffer + wb_data->valid_cnt;

        p_sys_lmep = (sys_oam_lmep_t*)_ctc_container_of(ctc_slistnode, sys_oam_lmep_t, head);
        if (NULL == p_sys_lmep)
        {
            continue;
        }

        /*lmep without rmep [chan + lmep] for one record*/
        if (0 == CTC_SLISTCOUNT(p_sys_lmep->rmep_list))
        {
            /*without rmep, [chan + lmep] for one record*/
            CTC_ERROR_RETURN(_sys_usw_oam_wb_sync_misc(lchip, p_wb_oam, p_oam_chan_com, p_sys_lmep));
            p_wb_oam->lmep_without_rmep = 1;
            if (++wb_data->valid_cnt == max_entry_cnt)
            {
                CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
                wb_data->valid_cnt = 0;
            }
            continue;
        }

        p_rmep_list = p_sys_lmep->rmep_list;
        CTC_SLIST_LOOP(p_rmep_list, ctc_slistnode_temp)
        {
            p_wb_oam = (sys_wb_oam_chan_t *)wb_data->buffer + wb_data->valid_cnt;

            p_sys_rmep = (sys_oam_rmep_t *)_ctc_container_of(ctc_slistnode_temp, sys_oam_rmep_t, head);
            if (NULL == p_sys_rmep)
            {
                continue;
            }
            /*[chan + lmep + rmep] for one record*/
            CTC_ERROR_RETURN(_sys_usw_oam_wb_sync_misc(lchip, p_wb_oam, p_oam_chan_com, p_sys_lmep));
            CTC_ERROR_RETURN(_sys_usw_oam_wb_mapping_rmep(lchip, p_wb_oam, p_sys_rmep, 1));

            if (++wb_data->valid_cnt == max_entry_cnt)
            {
                CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
                wb_data->valid_cnt = 0;
            }
        }

    }

    return CTC_E_NONE;
}


int32
_sys_usw_oam_wb_mapping_maid_other(uint8 lchip, sys_wb_oam_maid_t *p_wb_maid, sys_oam_maid_com_t *p_sys_maid, uint8 sync)
{

    if (sync)
    {
        p_wb_maid->mep_type = p_sys_maid->mep_type;
        p_wb_maid->maid_len = p_sys_maid->maid_len;
        p_wb_maid->maid_entry_num = p_sys_maid->maid_entry_num;
        p_wb_maid->ref_cnt = p_sys_maid->ref_cnt;
        p_wb_maid->maid_index = p_sys_maid->maid_index;
        sal_memcpy(p_wb_maid->maid, p_sys_maid->maid, sizeof(uint8)*SYS_OAM_MAX_MAID_LEN);
    }
    else
    {
        p_sys_maid->mep_type = p_wb_maid->mep_type;
        p_sys_maid->maid_len = p_wb_maid->maid_len;
        p_sys_maid->maid_entry_num = p_wb_maid->maid_entry_num;
        p_sys_maid->ref_cnt = p_wb_maid->ref_cnt;
        p_sys_maid->maid_index = p_wb_maid->maid_index;
        sal_memcpy(p_sys_maid->maid, p_wb_maid->maid, sizeof(uint8)*SYS_OAM_MAX_MAID_LEN);
    }

    return CTC_E_NONE;

}
int32
_sys_usw_oam_wb_sync_maid(sys_oam_maid_com_t* p_oam_maid_com, void* user_data)
{
    uint16 max_entry_cnt = 0;
    sys_wb_oam_maid_t* p_wb_maid = NULL;
    uint8 lchip = 0;

    sys_traverse_t *traversal_data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)traversal_data->data;
    lchip = (uint8)traversal_data->value1;

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);
    p_wb_maid = (sys_wb_oam_maid_t *)wb_data->buffer + wb_data->valid_cnt;
    sal_memset(p_wb_maid, 0, sizeof(sys_wb_oam_maid_t));


    /*if ref_cnt >0, sync with lmep*/
    if (p_oam_maid_com->ref_cnt)
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(_sys_usw_oam_wb_mapping_maid_other(lchip, p_wb_maid, p_oam_maid_com, 1));

    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_oam_wb_mapping_oamid(uint8 lchip, sys_wb_oam_oamid_t *p_wb_oamid, sys_oam_id_t *p_sys_oamid, uint8 sync)
{
    if (sync)
    {
        p_wb_oamid->gport = p_sys_oamid->gport;
        p_wb_oamid->oam_id = p_sys_oamid->oam_id;
        p_wb_oamid->p_sys_chan_eth = (void*)p_sys_oamid->p_sys_chan_eth;
        p_wb_oamid->label[0] = p_sys_oamid->label[0];
        p_wb_oamid->space_id[0] = p_sys_oamid->space_id[0];
        p_wb_oamid->label[1] = p_sys_oamid->label[0];
        p_wb_oamid->space_id[1] = p_sys_oamid->space_id[0];

    }
    else
    {
        p_sys_oamid->gport = p_wb_oamid->gport;
        p_sys_oamid->oam_id = p_wb_oamid->oam_id;
        p_sys_oamid->p_sys_chan_eth = (sys_oam_chan_eth_t*)p_wb_oamid->p_sys_chan_eth;
        p_sys_oamid->label[0] = p_wb_oamid->label[0];
        p_sys_oamid->space_id[0] = p_wb_oamid->space_id[0];
        p_sys_oamid->label[1] = p_wb_oamid->label[0];
        p_sys_oamid->space_id[1] = p_wb_oamid->space_id[0];

    }
    return CTC_E_NONE;
}

int32
_sys_usw_oam_wb_sync_oamid(sys_oam_id_t* p_oam_id, void* user_data)
{
    uint16 max_entry_cnt = 0;
    sys_wb_oam_oamid_t* p_wb_oamid = NULL;
    uint8 lchip = 0;

    sys_traverse_t *traversal_data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)traversal_data->data;
    lchip = (uint8)traversal_data->value1;
    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);
    p_wb_oamid = (sys_wb_oam_oamid_t *)wb_data->buffer + wb_data->valid_cnt;
    sal_memset(p_wb_oamid, 0, sizeof(sys_wb_oam_oamid_t));
    CTC_ERROR_RETURN(_sys_usw_oam_wb_mapping_oamid(lchip, p_wb_oamid, p_oam_id, 1));
    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32
sys_usw_oam_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_traverse_t user_data;
    sys_wb_oam_master_t  *p_wb_oam_master = NULL;
    uint8 work_status = 0;

    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	return CTC_E_NONE;
    }
    wb_data.buffer = mem_malloc(MEM_OAM_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    /*syncup oam master*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_oam_master_t, CTC_FEATURE_OAM, SYS_WB_APPID_OAM_SUBID_MASTER);
    p_wb_oam_master = (sys_wb_oam_master_t  *)wb_data.buffer;
    sal_memset(wb_data.buffer, 0, sizeof(sys_wb_oam_master_t));

    p_wb_oam_master->version = SYS_WB_VERSION_OAM;
    p_wb_oam_master->maid_len_format = g_oam_master[lchip]->maid_len_format;
    p_wb_oam_master->mep_index_alloc_by_sdk = g_oam_master[lchip]->mep_index_alloc_by_sdk;
    p_wb_oam_master->tp_section_oam_based_l3if = g_oam_master[lchip]->tp_section_oam_based_l3if;
    p_wb_oam_master->oam_reource_lm = g_oam_master[lchip]->oam_reource_num[SYS_OAM_TBL_LM];
    p_wb_oam_master->oam_reource_bfdv6addr = g_oam_master[lchip]->oam_reource_num[SYS_OAM_TBL_BFDV6ADDR];
    p_wb_oam_master->oam_reource_mep = g_oam_master[lchip]->oam_reource_num[SYS_OAM_TBL_MEP];
    p_wb_oam_master->oam_reource_key = g_oam_master[lchip]->oam_reource_num[SYS_OAM_TBL_LOOKUP_KEY];
    p_wb_oam_master->defect_to_rdi_bitmap0 = g_oam_master[lchip]->defect_to_rdi_bitmap0;
    p_wb_oam_master->defect_to_rdi_bitmap1 = g_oam_master[lchip]->defect_to_rdi_bitmap1;
    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);


    /*syncup chan+lmep+(rmep|)+(maid|) for 1 entry*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_oam_chan_t, CTC_FEATURE_OAM, SYS_WB_APPID_OAM_SUBID_CHAN);

    user_data.data = &wb_data;
    user_data.value1 = lchip;
    CTC_ERROR_GOTO(ctc_hash_traverse(g_oam_master[lchip]->chan_hash, (hash_traversal_fn) _sys_usw_oam_wb_sync_func, (void *)&user_data), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

    if(!g_oam_master[lchip]->no_mep_resource)
    {
        /*syncup maid, which has no relationship with mep, (ref_cnt=0) */
        CTC_WB_INIT_DATA_T((&wb_data),sys_wb_oam_maid_t, CTC_FEATURE_OAM, SYS_WB_APPID_OAM_SUBID_MAID);

        user_data.data = &wb_data;
        user_data.value1 = lchip;
        CTC_ERROR_GOTO(ctc_hash_traverse(g_oam_master[lchip]->maid_hash, (hash_traversal_fn) _sys_usw_oam_wb_sync_maid, (void *)&user_data), ret, done);
        if (wb_data.valid_cnt > 0)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
            wb_data.valid_cnt = 0;
        }
    }

    /*syncup oamid*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_oam_oamid_t, CTC_FEATURE_OAM, SYS_WB_APPID_OAM_SUBID_OAMID);

    user_data.data = &wb_data;
    user_data.value1 = lchip;
    CTC_ERROR_GOTO(ctc_hash_traverse(g_oam_master[lchip]->oamid_hash, (hash_traversal_fn) _sys_usw_oam_wb_sync_oamid, (void *)&user_data), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32
_sys_usw_oam_wb_restore_maid_other(uint8 lchip, ctc_wb_query_t* wb_query)
{
    uint16 entry_cnt = 0;
    int32  ret = CTC_E_NONE;
    sys_wb_oam_maid_t    wb_oam_maid;
    sys_oam_maid_com_t*  p_sys_maid_tmp = NULL;
    sys_oam_maid_com_t*  p_sys_maid = NULL;
    sys_usw_opf_t opf;


    sal_memset(&wb_oam_maid, 0, sizeof(sys_wb_oam_maid_t));
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = g_oam_master[lchip]->oam_opf_type;
    p_sys_maid_tmp = (sys_oam_maid_com_t*)mem_malloc(MEM_OAM_MODULE,  sizeof(sys_oam_maid_com_t));
    if (NULL == p_sys_maid_tmp)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(p_sys_maid_tmp, 0, sizeof(sys_oam_maid_com_t));
    CTC_WB_INIT_QUERY_T(wb_query, sys_wb_oam_maid_t, CTC_FEATURE_OAM, SYS_WB_APPID_OAM_SUBID_MAID);
    /*loop begin, restore SDK OAM MAID from WB entry*/
    CTC_WB_QUERY_ENTRY_BEGIN(wb_query);
    sal_memcpy((uint8 *)&wb_oam_maid, (uint8 *)wb_query->buffer + entry_cnt * (wb_query->key_len + wb_query->data_len), (wb_query->key_len + wb_query->data_len));
    entry_cnt++;

    ret = _sys_usw_oam_wb_mapping_maid_other(lchip, &wb_oam_maid, p_sys_maid_tmp, 0);
    if (ret)
    {
        continue;
    }

    /*maid: lookup, build node and add to DB*/
    if (NULL == ctc_hash_lookup(g_oam_master[lchip]->maid_hash, p_sys_maid_tmp))
    {
        p_sys_maid = (sys_oam_maid_com_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_maid_com_t));
        if (NULL == p_sys_maid)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_sys_maid, 0, sizeof(sys_oam_maid_com_t));
        sal_memcpy(p_sys_maid, p_sys_maid_tmp, sizeof(sys_oam_maid_com_t));

        if (NULL == ctc_hash_insert(g_oam_master[lchip]->maid_hash, p_sys_maid))
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        opf.pool_index = SYS_OAM_OPF_TYPE_MA_NAME;
        sys_usw_opf_alloc_offset_from_position(lchip, &opf, p_sys_maid->maid_entry_num, p_sys_maid->maid_index);
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MANAME, 1, p_sys_maid->maid_entry_num);
    }

    CTC_WB_QUERY_ENTRY_END(wb_query);

done:

    if (p_sys_maid_tmp)
    {
        mem_free(p_sys_maid_tmp);
    }

    return ret;

}

int32
_sys_usw_oam_wb_restore_oamid(uint8 lchip, ctc_wb_query_t* wb_query)
{
    uint16 entry_cnt = 0;
    int32  ret = CTC_E_NONE;
    sys_wb_oam_oamid_t   wb_oam_id;
    sys_oam_id_t*  p_sys_oamid_tmp = NULL;
    sys_oam_id_t*  p_sys_oamid = NULL;

    sal_memset(&wb_oam_id, 0, sizeof(sys_wb_oam_oamid_t));
    p_sys_oamid_tmp = (sys_oam_id_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_id_t));
    if (NULL == p_sys_oamid_tmp)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(p_sys_oamid_tmp, 0, sizeof(sys_oam_id_t));
    CTC_WB_INIT_QUERY_T(wb_query, sys_wb_oam_oamid_t, CTC_FEATURE_OAM, SYS_WB_APPID_OAM_SUBID_OAMID);
    CTC_WB_QUERY_ENTRY_BEGIN(wb_query);
    sal_memcpy((uint8 *)&wb_oam_id, (uint8 *)wb_query->buffer + entry_cnt * (wb_query->key_len + wb_query->data_len), (wb_query->key_len + wb_query->data_len));
    entry_cnt++;
    ret = _sys_usw_oam_wb_mapping_oamid(lchip, &wb_oam_id, p_sys_oamid_tmp, 0);
    if (ret)
    {
        continue;
    }
    if (NULL == ctc_hash_lookup(g_oam_master[lchip]->oamid_hash, p_sys_oamid_tmp))
    {
        p_sys_oamid = (sys_oam_id_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_id_t));
        if (NULL == p_sys_oamid)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_sys_oamid, 0, sizeof(sys_oam_id_t));
        sal_memcpy(p_sys_oamid, p_sys_oamid_tmp, sizeof(sys_oam_id_t));

        if (NULL == ctc_hash_insert(g_oam_master[lchip]->oamid_hash, p_sys_oamid))
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
    }
    CTC_WB_QUERY_ENTRY_END(wb_query);

done:

    if (p_sys_oamid_tmp)
    {
        mem_free(p_sys_oamid_tmp);
    }

    return ret;

}


int32
sys_usw_oam_wb_restore(uint8 lchip)
{
    uint16 entry_cnt = 0;
    uint8  mep_type = 0;
    uint32 session_type = 0;
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 value = 0;

    sys_wb_oam_master_t  wb_oam_master;
    sys_wb_oam_chan_t    wb_oam_chan;
    sys_oam_lmep_t*      p_sys_lmep_tmp = NULL;
    sys_oam_maid_com_t*  p_sys_maid_tmp = NULL;
    sys_oam_rmep_t*      p_sys_rmep_tmp = NULL;
    sys_oam_chan_bfd_t*  p_chan_bfd_tmp = NULL;
    sys_oam_chan_tp_t*   p_chan_tp_tmp = NULL;
    sys_oam_chan_eth_t*  p_chan_eth_tmp = NULL;
    sys_oam_chan_com_t*  p_oam_chan_com = NULL;
    sys_oam_chan_eth_t*  p_sys_eth = NULL;
    sys_oam_chan_tp_t*   p_sys_tp = NULL;
    sys_oam_chan_bfd_t*  p_sys_bfd = NULL;
    sys_oam_lmep_t*      p_sys_lmep = NULL;
    sys_oam_maid_com_t*  p_sys_maid = NULL;
    sys_oam_rmep_t*      p_sys_rmep = NULL;
    sys_oam_id_t*        p_sys_oamid = NULL;
    uint8                work_status = 0;
    sys_usw_opf_t opf;
    ctc_wb_query_t wb_query;
    sal_memset(&wb_oam_chan, 0, sizeof(sys_wb_oam_chan_t));
    sal_memset(&wb_oam_master, 0, sizeof(sys_wb_oam_master_t));
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type  = g_oam_master[lchip]->oam_opf_type;

    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	return CTC_E_NONE;
    }

    sal_memset(&wb_query, 0, sizeof(wb_query));
    wb_query.buffer = mem_malloc(MEM_OAM_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    /**************************restore oam master start*****************/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_oam_master_t, CTC_FEATURE_OAM, SYS_WB_APPID_OAM_SUBID_MASTER);
    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);

    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "query oam master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_INVALID_CONFIG;
        goto done;
    }

    /*version and compatibility check*/
    sal_memcpy((uint8 *)&wb_oam_master, (uint8 *)wb_query.buffer, wb_query.key_len + wb_query.data_len);
    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_OAM, wb_oam_master.version))
    {
        CTC_ERROR_GOTO(CTC_E_VERSION_MISMATCH, ret, done);
    }

    g_oam_master[lchip]->oam_reource_num[SYS_OAM_TBL_LM]= wb_oam_master.oam_reource_lm;
    g_oam_master[lchip]->oam_reource_num[SYS_OAM_TBL_BFDV6ADDR]= wb_oam_master.oam_reource_bfdv6addr;
    g_oam_master[lchip]->oam_reource_num[SYS_OAM_TBL_MEP]= wb_oam_master.oam_reource_mep;
    g_oam_master[lchip]->oam_reource_num[SYS_OAM_TBL_LOOKUP_KEY]= wb_oam_master.oam_reource_key;
    g_oam_master[lchip]->defect_to_rdi_bitmap0 = wb_oam_master.defect_to_rdi_bitmap0;
    g_oam_master[lchip]->defect_to_rdi_bitmap1 = wb_oam_master.defect_to_rdi_bitmap1;
    /*check between different SDK versions*/
    if ((g_oam_master[lchip]->maid_len_format > wb_oam_master.maid_len_format)
        || (g_oam_master[lchip]->mep_index_alloc_by_sdk != wb_oam_master.mep_index_alloc_by_sdk)
        || (g_oam_master[lchip]->tp_section_oam_based_l3if != wb_oam_master.tp_section_oam_based_l3if))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    cmd = DRV_IOR(OamUpdateCtl_t, OamUpdateCtl_updEn_f);
    DRV_IOCTL(lchip, 0, cmd, &value);
    g_oam_master[lchip]->timer_update_disable = !value;

    /**************************restore oamid start**************************/
    ret = _sys_usw_oam_wb_restore_oamid(lchip, &wb_query);
    if (ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "restore oamid error!!\n");
        goto done;
    }

    /**************************restore oam db start**************************/
    /*temp memory pool for oam lookup, need free when WB restore done*/
    p_chan_eth_tmp = (sys_oam_chan_eth_t*)mem_malloc(MEM_OAM_MODULE,  sizeof(sys_oam_chan_eth_t));
    if (NULL == p_chan_eth_tmp)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(p_chan_eth_tmp, 0, sizeof(sys_oam_chan_eth_t));

    p_chan_tp_tmp = (sys_oam_chan_tp_t*)mem_malloc(MEM_OAM_MODULE,  sizeof(sys_oam_chan_tp_t));
    if (NULL == p_chan_tp_tmp)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(p_chan_tp_tmp, 0, sizeof(sys_oam_chan_tp_t));

    p_chan_bfd_tmp = (sys_oam_chan_bfd_t*)mem_malloc(MEM_OAM_MODULE,  sizeof(sys_oam_chan_bfd_t));
    if (NULL == p_chan_bfd_tmp)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(p_chan_bfd_tmp, 0, sizeof(sys_oam_chan_bfd_t));

    p_sys_lmep_tmp = (sys_oam_lmep_t*)mem_malloc(MEM_OAM_MODULE,  sizeof(sys_oam_lmep_t));
    if (NULL == p_sys_lmep_tmp)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(p_sys_lmep_tmp, 0, sizeof(sys_oam_lmep_t));

    p_sys_maid_tmp = (sys_oam_maid_com_t*)mem_malloc(MEM_OAM_MODULE,  sizeof(sys_oam_maid_com_t));
    if (NULL == p_sys_maid_tmp)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(p_sys_maid_tmp, 0, sizeof(sys_oam_maid_com_t));

    p_sys_rmep_tmp = mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_rmep_t));
    if (NULL == p_sys_rmep_tmp)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(p_sys_rmep_tmp, 0, sizeof(sys_oam_rmep_t));

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_oam_chan_t, CTC_FEATURE_OAM, SYS_WB_APPID_OAM_SUBID_CHAN);
    /*loop begin, restore SDK OAM DB from WB entry*/
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
    sal_memcpy((uint8 *)&wb_oam_chan, (uint8 *)wb_query.buffer + entry_cnt * (wb_query.key_len + wb_query.data_len), (wb_query.key_len + wb_query.data_len));
    entry_cnt++;

    mep_type = wb_oam_chan.mep_type;
    if ( (CTC_OAM_MEP_TYPE_ETH_1AG == mep_type) || (CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_type))
    {
        sal_memset(p_chan_eth_tmp, 0, sizeof(sys_oam_chan_eth_t));
        ret = _sys_usw_oam_wb_mapping_chan_eth(lchip, &wb_oam_chan, p_chan_eth_tmp, 0);
        if (ret)
        {
            continue;
        }

        /*chan: lookup, build node and add to DB*/
        if (NULL == _sys_usw_oam_chan_lkup(lchip, &(p_chan_eth_tmp->com)))
        {
            p_sys_eth = (sys_oam_chan_eth_t*)mem_malloc(MEM_OAM_MODULE,  sizeof(sys_oam_chan_eth_t));
            if (NULL == p_sys_eth)
            {
                ret = CTC_E_NO_MEMORY;
                goto done;
            }
            sal_memset(p_sys_eth, 0, sizeof(sys_oam_chan_eth_t));
            sal_memcpy(p_sys_eth, p_chan_eth_tmp, sizeof(sys_oam_chan_eth_t));

            p_oam_chan_com = &(p_sys_eth->com);
            if (NULL == ctc_hash_insert(g_oam_master[lchip]->chan_hash, p_oam_chan_com))
            {
                ret = CTC_E_NO_MEMORY;
                goto done;
            }
            if (p_sys_eth->key.is_vpws && p_sys_eth->key.use_fid && !p_sys_eth->key.link_oam)
            {
                p_sys_oamid = _sys_usw_oam_oamid_lkup(lchip, p_sys_eth->key.gport, p_sys_eth->key.l2vpn_oam_id);
                if (p_sys_oamid)
                {
                    p_sys_oamid->p_sys_chan_eth = p_sys_eth;
                }
            }
        }

    }
    else if ((CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)|| (CTC_OAM_MEP_TYPE_MPLS_TP_Y1731 == mep_type))
    {
        sal_memset(p_chan_tp_tmp, 0, sizeof(sys_oam_chan_tp_t));
        ret = _sys_usw_oam_wb_mapping_chan_tp(lchip, &wb_oam_chan, p_chan_tp_tmp, 0);
        if (ret)
        {
            continue;
        }

        /*chan: lookup, build node and add to DB*/
        if (NULL ==  _sys_usw_oam_chan_lkup(lchip, &(p_chan_tp_tmp->com)))
        {
            p_sys_tp = (sys_oam_chan_tp_t*)mem_malloc(MEM_OAM_MODULE,  sizeof(sys_oam_chan_tp_t));
            if (NULL == p_sys_tp)
            {
                ret = CTC_E_NO_MEMORY;
                goto done;
            }
            sal_memset(p_sys_tp, 0, sizeof(sys_oam_chan_tp_t));
            sal_memcpy(p_sys_tp, p_chan_tp_tmp, sizeof(sys_oam_chan_tp_t));

            p_oam_chan_com = &(p_sys_tp->com);
            if (NULL == ctc_hash_insert(g_oam_master[lchip]->chan_hash, p_oam_chan_com))
            {
                ret = CTC_E_NO_MEMORY;
                goto done;
            }
        }
    }
    else
    {
        sal_memset(p_chan_bfd_tmp, 0, sizeof(sys_oam_chan_bfd_t));
        ret = _sys_usw_oam_wb_mapping_chan(lchip, &wb_oam_chan, p_chan_bfd_tmp, 0);
        if (ret)
        {
            continue;
        }

        /*chan: lookup, build node and add to DB*/
        if (NULL == _sys_usw_oam_chan_lkup(lchip, &(p_chan_bfd_tmp->com)))
        {
            p_sys_bfd = (sys_oam_chan_bfd_t*)mem_malloc(MEM_OAM_MODULE,  sizeof(sys_oam_chan_bfd_t));
            if (NULL == p_sys_bfd)
            {
                ret = CTC_E_NO_MEMORY;
                goto done;
            }
            sal_memset(p_sys_bfd, 0, sizeof(sys_oam_chan_bfd_t));
            sal_memcpy(p_sys_bfd, p_chan_bfd_tmp, sizeof(sys_oam_chan_bfd_t));

            p_oam_chan_com = &(p_sys_bfd->com);
            if (NULL == ctc_hash_insert(g_oam_master[lchip]->chan_hash, p_oam_chan_com))
            {
                ret = CTC_E_NO_MEMORY;
                goto done;
            }
        }
    }


    /*restore lmep start*/
    sal_memset(p_sys_lmep_tmp, 0, sizeof(sys_oam_lmep_t));
    p_sys_lmep_tmp->p_sys_chan = p_oam_chan_com;  /*restore pointer releationship*/
    ret = _sys_usw_oam_wb_mapping_lmep(lchip, &wb_oam_chan, p_sys_lmep_tmp, 0, mep_type);
    if (ret)
    {
        continue;
    }

   /*lmep: lookup, build node and add to DB*/
    if (NULL == ctc_vector_get(g_oam_master[lchip]->mep_vec, p_sys_lmep_tmp->lmep_index))
    {
        p_sys_lmep = (sys_oam_lmep_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_lmep_t));
        if (NULL == p_sys_lmep)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_sys_lmep, 0, sizeof(sys_oam_lmep_t));
        sal_memcpy(p_sys_lmep, p_sys_lmep_tmp, sizeof(sys_oam_lmep_t));

        if (NULL == p_oam_chan_com->lmep_list)
        {
            p_oam_chan_com->lmep_list = ctc_slist_new();
            if (NULL == p_oam_chan_com->lmep_list)
            {
                ret = CTC_E_NO_MEMORY;
                goto done;
            }
        }
        ctc_slist_add_tail(p_oam_chan_com->lmep_list, &(p_sys_lmep->head));

        if (!p_sys_lmep->mep_on_cpu)
        {
            if (FALSE == ctc_vector_add(g_oam_master[lchip]->mep_vec, p_sys_lmep->lmep_index, p_sys_lmep))
            {
                ret =  CTC_E_NO_MEMORY;
                goto done;
            }

            opf.pool_index = SYS_OAM_OPF_TYPE_MEP_LMEP;
            if ( ((CTC_OAM_MEP_TYPE_ETH_1AG == mep_type) || (CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_type))
                &&  (!CTC_FLAG_ISSET((p_sys_lmep)->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE)))
            {/*P2MP mode*/
                sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_sys_lmep->lmep_index);
            }
            else/*P2P*/
            {
                sys_usw_opf_alloc_offset_from_position(lchip, &opf, 2, p_sys_lmep->lmep_index);
            }


            opf.pool_index = SYS_OAM_OPF_TYPE_MA;
            sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_sys_lmep->ma_index);
            SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MA, 1, 1);

        }

        if (wb_oam_chan.with_maid && !p_sys_lmep->mep_on_cpu)
        {
            ret = _sys_usw_oam_wb_mapping_maid(lchip, &wb_oam_chan, p_sys_maid_tmp, 0, mep_type, p_sys_lmep->ma_index);
            if (ret)
            {
                continue;
            }
            p_sys_maid = ctc_hash_lookup(g_oam_master[lchip]->maid_hash, p_sys_maid_tmp);
            if (NULL == p_sys_maid)
            {
                p_sys_maid = (sys_oam_maid_com_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_maid_com_t));
                if (NULL == p_sys_maid)
                {
                    ret = CTC_E_NO_MEMORY;
                    goto done;
                }
                sal_memset(p_sys_maid, 0, sizeof(sys_oam_maid_com_t));
                sal_memcpy(p_sys_maid, p_sys_maid_tmp, sizeof(sys_oam_maid_com_t));

                if (NULL == ctc_hash_insert(g_oam_master[lchip]->maid_hash, p_sys_maid))
                {
                    ret = CTC_E_NO_MEMORY;
                    goto done;
                }
                opf.pool_index = SYS_OAM_OPF_TYPE_MA_NAME;

                sys_usw_opf_alloc_offset_from_position(lchip, &opf, p_sys_maid->maid_entry_num, p_sys_maid->maid_index);
                SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_MANAME, 1, p_sys_maid->maid_entry_num);

            }
            p_sys_lmep->p_sys_maid = p_sys_maid;

        }

        sys_usw_oam_get_session_type(lchip, (sys_oam_lmep_t*)p_sys_lmep, &session_type);
        SYS_OAM_SESSION_CNT(lchip, session_type, 1);
        /*chan+lmep for one record, without rmep*/
        if (wb_oam_chan.lmep_without_rmep)
        {
            continue;
        }

    }

    /*restore rmep start*/
    sal_memset(p_sys_rmep_tmp, 0, sizeof(sys_oam_rmep_t));
    p_sys_rmep_tmp->p_sys_lmep = p_sys_lmep;  /*restore pointer releationship*/
    if (wb_oam_chan.with_maid)
    {
        p_sys_rmep_tmp->p_sys_maid = p_sys_lmep->p_sys_maid;
    }
    ret = _sys_usw_oam_wb_mapping_rmep(lchip, &wb_oam_chan, p_sys_rmep_tmp, 0);
    if (ret)
    {
        continue;
    }

    /*rmep: lookup, build node and add to DB*/
    if (NULL == ctc_vector_get(g_oam_master[lchip]->mep_vec, p_sys_rmep_tmp->rmep_index))
    {
        p_sys_rmep = (sys_oam_rmep_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_rmep_t));
        if (NULL == p_sys_rmep)
        {
            ret = CTC_E_NO_MEMORY;
            goto done;
        }
        sal_memset(p_sys_rmep, 0, sizeof(sys_oam_rmep_t));
        sal_memcpy(p_sys_rmep, p_sys_rmep_tmp, sizeof(sys_oam_rmep_t));

        if (NULL == p_sys_lmep->rmep_list)
        {
            p_sys_lmep->rmep_list = ctc_slist_new();
            if (NULL == p_sys_lmep->rmep_list)
            {
                ret =  CTC_E_NO_MEMORY;
                goto done;
            }
        }
        ctc_slist_add_tail(p_sys_lmep->rmep_list, &(p_sys_rmep->head));

        if (FALSE == ctc_vector_add(g_oam_master[lchip]->mep_vec, p_sys_rmep->rmep_index, p_sys_rmep))
        {
            ret =  CTC_E_NO_MEMORY;
            goto done;
        }

        if ( ((CTC_OAM_MEP_TYPE_ETH_1AG == mep_type) || (CTC_OAM_MEP_TYPE_ETH_Y1731 == mep_type))
            && (!CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_Y1731_LMEP_FLAG_P2P_MODE)))
        {/*P2MP mode*/
            opf.pool_index  = SYS_OAM_OPF_TYPE_MEP_LMEP;
            sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_sys_rmep->rmep_index);
        }

    }

    CTC_WB_QUERY_ENTRY_END((&wb_query));


    /**************************restore maid start**************************/
    ret = _sys_usw_oam_wb_restore_maid_other(lchip, &wb_query);
    if (ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "restore maid error!!\n");
        goto done;
    }

done:
    if (wb_query.key)
    {
        mem_free(wb_query.key);
    }
    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }
    if (p_chan_eth_tmp)
    {
        mem_free(p_chan_eth_tmp);
    }
    if (p_chan_tp_tmp)
    {
        mem_free(p_chan_tp_tmp);
    }
    if (p_chan_bfd_tmp)
    {
        mem_free(p_chan_bfd_tmp);
    }
    if (p_sys_lmep_tmp)
    {
        mem_free(p_sys_lmep_tmp);
    }
    if (p_sys_maid_tmp)
    {
        mem_free(p_sys_maid_tmp);
    }
    if (p_sys_rmep_tmp)
    {
        mem_free(p_sys_rmep_tmp);
    }

    return ret;
}

#define DUMP_DB "Function Begin"
int32
sys_usw_oam_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;
    uint32 loop = 0;
    uint32 valid_count = 0;
    SYS_OAM_INIT_CHECK(lchip);
    OAM_LOCK(lchip);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "# OAM");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","maid_len_format", g_oam_master[lchip]->maid_len_format);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%s\n","mep_index_alloc_by_sdk", g_oam_master[lchip]->mep_index_alloc_by_sdk ? "Y" : "N");
    SYS_DUMP_DB_LOG(p_f, "%-30s:%s\n","tp_section_oam_based_l3if", g_oam_master[lchip]->tp_section_oam_based_l3if ? "Y" : "N");
    SYS_DUMP_DB_LOG(p_f, "%-30s:%s\n","timer_update_disable", g_oam_master[lchip]->timer_update_disable ? "Y" : "N");
    SYS_DUMP_DB_LOG(p_f, "%-30s:%s\n","no_mep_resource", g_oam_master[lchip]->no_mep_resource ? "Y" : "N");
    SYS_DUMP_DB_LOG(p_f, "%-30s:0x%x\n","defect_to_rdi_bitmap0", g_oam_master[lchip]->defect_to_rdi_bitmap0);
    SYS_DUMP_DB_LOG(p_f, "%-30s:0x%x\n","defect_to_rdi_bitmap1", g_oam_master[lchip]->defect_to_rdi_bitmap1);
    SYS_DUMP_DB_LOG(p_f, "%-30s:0x%x\n","sf_defect_bitmap", g_oam_master[lchip]->sf_defect_bitmap);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n","rsv_maname_idx", g_oam_master[lchip]->rsv_maname_idx);

    SYS_DUMP_DB_LOG(p_f, "%-30s:","oam_reource_num");
    for(loop = 0; loop < SYS_OAM_TBL_MAX; loop ++)
    {
        if (0 == g_oam_master[lchip]->oam_reource_num[loop])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "[%u:%u]", loop, g_oam_master[lchip]->oam_reource_num[loop]);
        if (valid_count % 4 == 3)
        {
            SYS_DUMP_DB_LOG(p_f, "\n%31s", " ");
        }
        valid_count ++;
    }
    SYS_DUMP_DB_LOG(p_f, "\n");
    SYS_DUMP_DB_LOG(p_f, "%-30s:","oam_session_num");
    valid_count = 0;
    for(loop = 0; loop < SYS_OAM_SESSION_MAX; loop ++)
    {
        if (0 == g_oam_master[lchip]->oam_session_num[loop])
        {
            continue;
        }
        SYS_DUMP_DB_LOG(p_f, "[%u:%u]", loop, g_oam_master[lchip]->oam_session_num[loop]);
        if (valid_count % 4 == 3)
        {
            SYS_DUMP_DB_LOG(p_f, "\n%31s", " ");
        }
        valid_count ++;
    }
    SYS_DUMP_DB_LOG(p_f, "\n");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "\n");
    sys_usw_opf_fprint_alloc_used_info(lchip, g_oam_master[lchip]->oam_opf_type, p_f);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");
    OAM_UNLOCK(lchip);

    return ret;
}

#endif

