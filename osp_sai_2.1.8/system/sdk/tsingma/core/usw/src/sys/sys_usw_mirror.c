/**
 @file sys_usw_mirror.c

 @date 2009-10-21

 @version v2.0
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_error.h"
#include "ctc_common.h"
#include "ctc_const.h"
#include "ctc_packet.h"

#include "sys_usw_chip.h"
#include "sys_usw_mirror.h"
#include "sys_usw_port.h"
#include "sys_usw_internal_port.h"
#include "sys_usw_vlan.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_nexthop.h"
#include "sys_usw_common.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_register.h"
#include "sys_usw_wb_common.h"

#include "drv_api.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

#define MAX_SYS_MIRROR_SESSION_TYPE (MAX_CTC_MIRROR_SESSION_TYPE + 8)
#define SYS_MIRROR_INVALID_ILOOP_PORT  0xFFFF

#define SYS_MIRROR_INIT_CHECK() \
    do { \
        LCHIP_CHECK(lchip); \
        if (p_usw_mirror_master[lchip] == NULL){ \
            SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
			return CTC_E_NOT_INIT;\
 } \
    } while (0)

#define MIRROR_SET_MAC(d, s)       sal_memcpy(d, s, sizeof(mac_addr_t))
#define MIRROR_SET_HW_MAC(d, s)    SYS_USW_SET_HW_MAC(d, s)

/* mirror's session-id correspond to per acl lookup's priority-id, and mirror's priority-id correspond to acl lookup index */
#define SYS_MIRROR_ACL_LOG_EXCEPTION_IDX(dir, session_id, priority_id)\
        ((CTC_INGRESS == dir)\
        ? (MCHIP_CAP(SYS_CAP_MIRROR_INGRESS_ACL_LOG_INDEX_BASE) + (priority_id * MAX_CTC_MIRROR_SESSION_ID) + session_id)\
        : (MCHIP_CAP(SYS_CAP_MIRROR_EGRESS_ACL_LOG_INDEX_BASE) + (priority_id * MAX_CTC_MIRROR_SESSION_ID) + session_id))

#define SYS_MIRROR_TYPE_MAPPING(p_mirror, mirror_type) \
{\
    mirror_type = p_mirror->type;\
    if (p_mirror->type == CTC_MIRROR_ACLLOG_SESSION)\
    {  mirror_type = p_mirror->type + p_mirror->acl_priority; }\
    else if (p_mirror->type > CTC_MIRROR_ACLLOG_SESSION)\
    {  mirror_type = p_mirror->type + MCHIP_CAP(SYS_CAP_MIRROR_ACL_ID) - 1; }\
}
struct sys_mirror_master_s
{
    uint32 mirror_nh[MAX_CTC_MIRROR_SESSION_ID][MAX_SYS_MIRROR_SESSION_TYPE][CTC_BOTH_DIRECTION];
    uint16 iloop_port[MAX_CTC_MIRROR_SESSION_ID][MAX_SYS_MIRROR_SESSION_TYPE][CTC_BOTH_DIRECTION];
};
typedef struct sys_mirror_master_s sys_mirror_master_t;

sys_mirror_master_t* p_usw_mirror_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

/****************************************************************************
 *
* Function
*
*****************************************************************************/

STATIC bool
_sys_usw_mirror_check_internal_nexthop(uint8 lchip, uint32 nh_id)
{
    uint32 max_nh = 0;

    sys_usw_nh_get_max_external_nhid(lchip, &max_nh);

    if (nh_id >= max_nh)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

STATIC uint8
_sys_usw_mirror_get_iloop_port(uint8 lchip, uint32 nh_id, uint16* p_iloop_port)
{
    uint8 i = 0;
    uint8 j = 0;
    uint8 k = 0;
    uint8 iloop_port_num = 0;

    *p_iloop_port = 0;
    for (i = 0; i < MAX_CTC_MIRROR_SESSION_ID; i++)
    {
        for (j = 0; j < MAX_SYS_MIRROR_SESSION_TYPE; j++)
        {
            for (k = 0; k < CTC_BOTH_DIRECTION; k++)
            {
                if (nh_id == p_usw_mirror_master[lchip]->mirror_nh[i][j][k])
                {
                    iloop_port_num ++;
                    *p_iloop_port = p_usw_mirror_master[lchip]->iloop_port[i][j][k];
                }
            }
        }
    }

    return iloop_port_num;
}

STATIC int32
_sys_usw_mirror_set_internal_port(uint8 lchip , uint8 lport, uint8 type, uint32 dsfwd_offset, bool is_add)
{
    DsPhyPortExt_m phy_port_ext;
    DsPhyPort_m phy_port;
    DsSrcPort_m src_port;
    uint32 cmd = 0;
    uint32 value = 0;

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "%s config of internal port:%d\n",is_add?"add":"clear",lport);

    cmd = DRV_IOR(DsSrcPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &src_port));
    cmd = DRV_IOR(DsPhyPortExt_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port_ext));
    cmd = DRV_IOR(DsPhyPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port));

    value = is_add? 1 : 0;
    SetDsSrcPort(V, routeDisable_f, &src_port, value);
    SetDsSrcPort(V, addDefaultVlanDisable_f, &src_port, value);
    SetDsPhyPort(V, packetTypeValid_f, &phy_port, value);
    value = is_add? 0 : 1;
    SetDsPhyPortExt(V, defaultVlanId_f, &phy_port_ext, value);
    value = is_add? 7 : 0;
    SetDsPhyPort(V, packetType_f, &phy_port, value);
    if (type == 0)/*iloop*/
    {
        value = is_add? 1 : 0;
        SetDsSrcPort(V, stpDisable_f, &src_port, value);
    }
    else if(type == 1)/*mcast*/
    {
        value = is_add? 1 : 0;
        SetDsSrcPort(V, portCrossConnect_f, &src_port, value);
        SetDsSrcPort(V, bridgeEn_f, &src_port, !value);
        value = is_add? dsfwd_offset : 0;
        SetDsPhyPortExt(V, u1_g2_dsFwdPtr_f, &phy_port_ext,value);
    }

    cmd = DRV_IOW(DsSrcPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &src_port));
    cmd = DRV_IOW(DsPhyPortExt_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port_ext));
    cmd = DRV_IOW(DsPhyPort_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &phy_port));

    return CTC_E_NONE;
}

/*
 @brief This function is to write chip to set the destination port
*/
STATIC int32
_sys_usw_mirror_set_iloop_port(uint8 lchip, uint32 nh_id, uint32* dsnh_offset, uint32* gport, ctc_mirror_dest_t* p_mirror, bool is_add)
{
    sys_nh_info_com_t* p_nhinfo = NULL;
    uint8  gchip = 0;
    uint16 iloop_port = 0;
    uint32 old_nh_id = 0;
    uint8 new_iloop_port_num = 0;
    uint8 old_iloop_port_num = 0;
    uint16 new_iloop_port = 0;
    uint16 old_iloop_port = 0;
    uint8 type = 0;
    uint8 is_free = 0;
    uint8 is_alloc = 0;
    int32 ret = CTC_E_NONE;
    ctc_internal_port_assign_para_t port_assign;

    SYS_MIRROR_TYPE_MAPPING(p_mirror, type);
    ret = (sys_usw_nh_get_nhinfo_by_nhid(lchip, nh_id, &p_nhinfo));
    if ((CTC_E_NOT_EXIST == ret)
        &&
        (SYS_MIRROR_INVALID_ILOOP_PORT != p_usw_mirror_master[lchip]->iloop_port[p_mirror->session_id][type][p_mirror->dir]))
    {
        old_iloop_port_num = _sys_usw_mirror_get_iloop_port(lchip, nh_id, &iloop_port);
        if (1 == old_iloop_port_num)
        {
            is_free = 1;
            p_usw_mirror_master[lchip]->iloop_port[p_mirror->session_id][type][p_mirror->dir] = SYS_MIRROR_INVALID_ILOOP_PORT;
        }
    }

    if (p_nhinfo && (p_nhinfo->hdr.nh_entry_type != SYS_NH_TYPE_ILOOP) && (p_nhinfo->hdr.nh_entry_type != SYS_NH_TYPE_MCAST))
    {
        return CTC_E_NONE;
    }

    /*iloop*/
    if (p_nhinfo && (p_nhinfo->hdr.nh_entry_type == SYS_NH_TYPE_ILOOP))
    {
        if (CTC_IS_BIT_SET(p_nhinfo->hdr.dsfwd_info.dsnh_offset, 11)
            && ( 7 == ((p_nhinfo->hdr.dsfwd_info.dsnh_offset >> 12) & 0x7)))
        {
            iloop_port = (p_nhinfo->hdr.dsfwd_info.dsnh_offset & 0x7F) | (((p_nhinfo->hdr.dsfwd_info.dsnh_offset >> 16)&0x1) << 7);
        }
        else
        {
            return CTC_E_NONE;
        }

        _sys_usw_mirror_set_internal_port(lchip, iloop_port, 0, 0, is_add);
        return CTC_E_NONE;
    }
    /*mcast*/
    else if(p_nhinfo)
    {
        iloop_port = p_usw_mirror_master[lchip]->iloop_port[p_mirror->session_id][type][p_mirror->dir];
        old_nh_id  = p_usw_mirror_master[lchip]->mirror_nh[p_mirror->session_id][type][p_mirror->dir];
        new_iloop_port_num = _sys_usw_mirror_get_iloop_port(lchip, nh_id, &new_iloop_port);
        old_iloop_port_num = _sys_usw_mirror_get_iloop_port(lchip, old_nh_id, &old_iloop_port);
        if ((is_add)&&(old_nh_id != nh_id))
        {
            /*del and release old master iloop port,only do when this iloop only used for 1 time and nh is multicast*/
            if ((1 == old_iloop_port_num) && (iloop_port != SYS_MIRROR_INVALID_ILOOP_PORT))
            {
                is_free = 1;
                p_usw_mirror_master[lchip]->iloop_port[p_mirror->session_id][type][p_mirror->dir] = SYS_MIRROR_INVALID_ILOOP_PORT;
            }

            if(0 == new_iloop_port_num)
            {
                is_alloc = 1;
            }
            else
            {
                p_usw_mirror_master[lchip]->iloop_port[p_mirror->session_id][type][p_mirror->dir] = new_iloop_port;
            }
        }
        else if(!is_add)
        {
            if((1 == old_iloop_port_num) && (iloop_port != SYS_MIRROR_INVALID_ILOOP_PORT))
            {
                is_free = 1;
            }
            p_usw_mirror_master[lchip]->iloop_port[p_mirror->session_id][type][p_mirror->dir] = SYS_MIRROR_INVALID_ILOOP_PORT;
        }
    }

    sys_usw_get_gchip_id(lchip, &gchip);
    sal_memset(&port_assign, 0, sizeof(port_assign));
    port_assign.type = CTC_INTERNAL_PORT_TYPE_ILOOP;
    port_assign.gchip = gchip;
    if (is_free)
    {
        _sys_usw_mirror_set_internal_port(lchip, iloop_port, 1, 0, 0);
        port_assign.inter_port = iloop_port;
        CTC_ERROR_RETURN(sys_usw_internal_port_release(lchip, &port_assign));
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "release internal port:%d\n", iloop_port);
    }

    if (is_alloc)
    {
        CTC_ERROR_RETURN(sys_usw_internal_port_allocate(lchip, &port_assign));
        iloop_port = port_assign.inter_port;
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "alloc internal port:%d\n", iloop_port);
        _sys_usw_mirror_set_internal_port(lchip, iloop_port, 1, p_nhinfo->hdr.dsfwd_info.dsfwd_offset, 1);
        p_usw_mirror_master[lchip]->iloop_port[p_mirror->session_id][type][p_mirror->dir] = iloop_port;

    }

    if (is_add)
    {
        iloop_port = p_usw_mirror_master[lchip]->iloop_port[p_mirror->session_id][type][p_mirror->dir];
        *gport  = CTC_MAP_LPORT_TO_GPORT(gchip, SYS_RSV_PORT_ILOOP_ID);
        *dsnh_offset = SYS_NH_ENCODE_ILOOP_DSNH(iloop_port, 0, 0, 0, 0);
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mirror_set_dest_write_chip(uint8 lchip, uint32 excp_index, uint32 dest_gport, uint32 nh_ptr, uint32 trun_len, uint8 is_rspan)
{
    uint8  gchip_id = 0;
    uint16 lport = 0;
    uint32 destmap = 0;
    uint32 cmd = 0;
    uint8  profile_id = 0;
    uint8  sub_queue_id = 0;

    DsMetFifoExcp_m met_excp;
    DsBufRetrvExcp_m bufrev_exp;

    sal_memset(&met_excp, 0, sizeof(DsMetFifoExcp_m));
    sal_memset(&bufrev_exp, 0, sizeof(DsBufRetrvExcp_m));

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if ((0 != trun_len) && CTC_IS_CPU_PORT(dest_gport))
    {
        return CTC_E_INVALID_CONFIG;
    }

    cmd = DRV_IOR(DsMetFifoExcp_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd, &met_excp));
    cmd = DRV_IOR(DsBufRetrvExcp_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index , cmd, &bufrev_exp));

    if(0 != (profile_id = GetDsBufRetrvExcp(V, truncateLenProfId_f, &bufrev_exp)))
    {
        CTC_ERROR_RETURN(sys_usw_register_remove_truncation_profile(lchip, 1, profile_id));
        SetDsBufRetrvExcp(V, truncateLenProfId_f, &bufrev_exp, 0);
    }
    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(dest_gport);
    if (CTC_IS_LINKAGG_PORT(dest_gport))
    {
        destmap = SYS_ENCODE_MIRROR_DESTMAP(CTC_LINKAGG_CHIPID, CTC_GPORT_LINKAGG_ID(dest_gport));

        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "excp_index = %d, dest_gport = %d, nh_ptr = %d\n", excp_index, dest_gport, nh_ptr&0x7FFFFFFF);

        SetDsMetFifoExcp(V, destMap_f, &met_excp, destmap);
        SetDsMetFifoExcp(V, nextHopExt_f, &met_excp, CTC_IS_BIT_SET(nh_ptr, 31));
        SetDsMetFifoExcp(V, isSpanPkt_f, &met_excp, 1);

        SetDsBufRetrvExcp(V, nextHopPtr_f, &bufrev_exp, nh_ptr);
        SetDsBufRetrvExcp(V, resetPacketHeaderU1_f, &bufrev_exp, 1);
        SetDsBufRetrvExcp(V, isSpanPkt_f, &bufrev_exp, 1);
		SetDsBufRetrvExcp(V, resetPacketHeaderToNormal_f, &bufrev_exp, 1);

        if (0 != trun_len)
        {
            CTC_ERROR_RETURN(sys_usw_register_add_truncation_profile(lchip, trun_len, 0, &profile_id));
            SetDsBufRetrvExcp(V, truncateLenProfId_f, &bufrev_exp, profile_id);
        }
    }
    else
    {
        gchip_id = SYS_MAP_CTC_GPORT_TO_GCHIP(dest_gport);

        if (SYS_RSV_PORT_DROP_ID == lport)
        {
            sys_usw_get_gchip_id(lchip, &gchip_id);
            destmap = SYS_ENCODE_DESTMAP( gchip_id, SYS_RSV_PORT_DROP_ID);
        }
        else if (CTC_IS_CPU_PORT(dest_gport))
        {
            CTC_ERROR_RETURN(sys_usw_get_sub_queue_id_by_cpu_reason(lchip, CTC_PKT_CPU_REASON_MIRRORED_TO_CPU, &sub_queue_id));
            destmap = SYS_ENCODE_EXCP_DESTMAP(gchip_id, sub_queue_id);
            if(!is_rspan)
            {
                nh_ptr = CTC_PKT_CPU_REASON_BUILD_NHPTR(CTC_PKT_CPU_REASON_MIRRORED_TO_CPU, 0);
            }
        }
        else
        {
            destmap = SYS_ENCODE_MIRROR_DESTMAP(gchip_id, lport);
        }

        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "excp_index = %d, dest_gport = %d, nh_ptr = %d\n", excp_index, dest_gport, nh_ptr&0x7FFFFFFF);

        SetDsMetFifoExcp(V, destMap_f, &met_excp, destmap);
        SetDsMetFifoExcp(V, nextHopExt_f, &met_excp, CTC_IS_BIT_SET(nh_ptr, 31));
        SetDsMetFifoExcp(V, isSpanPkt_f, &met_excp, 1);

        SetDsBufRetrvExcp(V, nextHopPtr_f, &bufrev_exp, nh_ptr&0x7FFFFFFF);
        SetDsBufRetrvExcp(V, resetPacketHeaderU1_f, &bufrev_exp, 1);
        SetDsBufRetrvExcp(V, isSpanPkt_f, &bufrev_exp, 1);
		SetDsBufRetrvExcp(V, resetPacketHeaderToNormal_f, &bufrev_exp, 1);

        /*To CPU need to forceEgressEdit and not support truncation */
        if(CTC_IS_CPU_PORT(dest_gport))
        {
            SetDsBufRetrvExcp(V, forceEgressEdit_f, &bufrev_exp, 1);
        }
        else if (0 != trun_len)
        {
            CTC_ERROR_RETURN(sys_usw_register_add_truncation_profile(lchip, trun_len, 0, &profile_id));
            SetDsBufRetrvExcp(V, truncateLenProfId_f, &bufrev_exp, profile_id);
        }
    }

    /*write DsBufRetrvExcp_t table, 0 is slice 0, 1 is slice 1*/
    cmd = DRV_IOW(DsBufRetrvExcp_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd, &bufrev_exp));

	/*Must write DsMetFifoExcp_t table AFTER write BufRetrvExcp*/
    cmd = DRV_IOW(DsMetFifoExcp_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd, &met_excp));
    return CTC_E_NONE;
}

int32
sys_usw_mirror_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param)
{
    uint32 loop1 = 0;
    uint32 loop2 = 0;
    uint32 loop3 = 0;
    uint32 valid_count = 0;
    SYS_MIRROR_INIT_CHECK();

    SYS_DUMP_DB_LOG(p_f, "%s\n", "# Mirror");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");

    SYS_DUMP_DB_LOG(p_f, "%-30s:","mirror_nh");
    for (loop1 = 0; loop1 < MAX_CTC_MIRROR_SESSION_ID; loop1++)
    {
        for (loop2 = 0; loop2 < MAX_SYS_MIRROR_SESSION_TYPE; loop2++)
        {
            for (loop3 = 0; loop3 < CTC_BOTH_DIRECTION; loop3++)
            {
                if (0 == p_usw_mirror_master[lchip]->mirror_nh[loop1][loop2][loop3])
                {
                    continue;
                }
                SYS_DUMP_DB_LOG(p_f, "[%u-%u-%u:%u]", loop1, loop2, loop3, p_usw_mirror_master[lchip]->mirror_nh[loop1][loop2][loop3]);
                if (valid_count % 4 == 3)
                {
                    SYS_DUMP_DB_LOG(p_f, "\n%31s", " ");
                }
                valid_count ++;
            }
        }
    }
    SYS_DUMP_DB_LOG(p_f, "\n");

    SYS_DUMP_DB_LOG(p_f, "%-30s:","iloop_port");
    valid_count = 0;
    for (loop1 = 0; loop1 < MAX_CTC_MIRROR_SESSION_ID; loop1++)
    {
        for (loop2 = 0; loop2 < MAX_SYS_MIRROR_SESSION_TYPE; loop2++)
        {
            for (loop3 = 0; loop3 < CTC_BOTH_DIRECTION; loop3++)
            {
                if (0xFFFF == p_usw_mirror_master[lchip]->iloop_port[loop1][loop2][loop3])
                {
                    continue;
                }
                SYS_DUMP_DB_LOG(p_f, "[%u-%u-%u:%u]", loop1, loop2, loop3, p_usw_mirror_master[lchip]->iloop_port[loop1][loop2][loop3]);
                if (valid_count % 4 == 3)
                {
                    SYS_DUMP_DB_LOG(p_f, "\n%31s", " ");
                }
                valid_count ++;
            }
        }
    }
    SYS_DUMP_DB_LOG(p_f, "\n");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");

    return CTC_E_NONE;
}

int32
sys_usw_mirror_wb_sync(uint8 lchip)
{
    uint8 loop1 = 0;
    uint8 loop2 = 0;
    uint8 loop3 = 0;
    uint16 max_entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_wb_mirror_dest_t* p_wb_mirror_dest = NULL;
    sys_wb_mirror_master_t* p_wb_mirror_master = NULL;
    uint8 work_status = 0;

	sys_usw_ftm_get_working_status(lchip, &work_status);

    /*sync up mirror dest*/
    wb_data.buffer = mem_malloc(MEM_MIRROR_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    p_wb_mirror_master = (sys_wb_mirror_master_t*)wb_data.buffer;
    p_wb_mirror_master->lchip = lchip;
    p_wb_mirror_master->version = SYS_WB_VERSION_MIRROR;

    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_mirror_master_t, CTC_FEATURE_MIRROR, SYS_WB_APPID_MIRROR_SUBID_MASTER);
    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_mirror_dest_t, CTC_FEATURE_MIRROR, SYS_WB_APPID_MIRROR_SUBID_DEST);
    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data.key_len + wb_data.data_len);

    for (loop1 = 0; loop1 < MAX_CTC_MIRROR_SESSION_ID; loop1++)
    {
        for (loop2 = 0; loop2 < MAX_SYS_MIRROR_SESSION_TYPE; loop2++)
        {
            for (loop3 = 0; loop3 < CTC_BOTH_DIRECTION; loop3++)
            {
                /*delete rspan mirror*/
                if(work_status == CTC_FTM_MEM_CHANGE_RECOVER && p_usw_mirror_master[lchip]->mirror_nh[loop1][loop2][loop3])
                {
                    ctc_mirror_dest_t mirror;
                    sal_memset(&mirror, 0, sizeof(mirror));
                    mirror.session_id = loop1;
                    if(loop2 < CTC_MIRROR_ACLLOG_SESSION)
                    {
                        mirror.type = loop2;
                    }
                    else if(loop2 < (CTC_MIRROR_CPU_SESSION+MCHIP_CAP(SYS_CAP_MIRROR_ACL_ID) - 1))
                    {
                        mirror.type = CTC_MIRROR_ACLLOG_SESSION;
                        mirror.acl_priority = loop2 - CTC_MIRROR_ACLLOG_SESSION;
                    }
                    else
                    {
                        mirror.type = loop2 - (MCHIP_CAP(SYS_CAP_MIRROR_ACL_ID) - 1);
                    }

                    mirror.dir = loop3;
                    sys_usw_mirror_unset_dest(lchip, &mirror);
                    continue;
                }
                p_wb_mirror_dest = (sys_wb_mirror_dest_t *)wb_data.buffer + wb_data.valid_cnt;
                p_wb_mirror_dest->session_id = loop1;
                p_wb_mirror_dest->type = loop2;
                p_wb_mirror_dest->dir = loop3;
                p_wb_mirror_dest->nh_id = p_usw_mirror_master[lchip]->mirror_nh[loop1][loop2][loop3];
                p_wb_mirror_dest->iloop_port = p_usw_mirror_master[lchip]->iloop_port[loop1][loop2][loop3];
                if (++wb_data.valid_cnt == max_entry_cnt)
                {
                    CTC_ERROR_RETURN(ctc_wb_add_entry(&wb_data));
                    wb_data.valid_cnt = 0;
                }
            }
        }
    }

    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(&wb_data));
    }

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32
sys_usw_mirror_wb_restore(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t wb_query;
    sys_wb_mirror_dest_t* p_wb_mirror_dest;
    sys_wb_mirror_master_t wb_mirror_master;
    uint16 entry_cnt = 0;

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));

    wb_query.buffer = mem_malloc(MEM_MIRROR_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    /*restore master*/
    /* set default value to new added fields, default value may not be zeros */
    sal_memset(&wb_mirror_master, 0, sizeof(sys_wb_mirror_master_t));
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_mirror_master_t, CTC_FEATURE_MIRROR, SYS_WB_APPID_MIRROR_SUBID_MASTER);
    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);
    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "query mirror master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_INVALID_CONFIG;
        goto done;
    }
    sal_memcpy((uint8*)&wb_mirror_master, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
    if CTC_WB_VERSION_CHECK(SYS_WB_VERSION_MIRROR, wb_mirror_master.version)
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    /*restore mirror dest*/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_mirror_dest_t, CTC_FEATURE_MIRROR, SYS_WB_APPID_MIRROR_SUBID_DEST);

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        p_wb_mirror_dest = (sys_wb_mirror_dest_t*)wb_query.buffer + entry_cnt++;/*after reading entry from sqlite, wb_query.buffer is still the first address of the space*/
        p_usw_mirror_master[lchip]->mirror_nh[p_wb_mirror_dest->session_id][p_wb_mirror_dest->type][p_wb_mirror_dest->dir] = p_wb_mirror_dest->nh_id;
        p_usw_mirror_master[lchip]->iloop_port[p_wb_mirror_dest->session_id][p_wb_mirror_dest->type][p_wb_mirror_dest->dir] = p_wb_mirror_dest->iloop_port;
    CTC_WB_QUERY_ENTRY_END((&wb_query));
done:
    if (wb_query.key)
    {
        mem_free(wb_query.key);
    }

    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }
    return ret;
}

/*
 @brief This function is to initialize the mirror module
*/
int32
sys_usw_mirror_init(uint8 lchip)
{
    uint8  session_id = 0;
    uint8  gchip_id = 0;
    uint32 cmd1 = 0;
    uint32 cmd2 = 0;
    uint32 excp_index = 0;
    uint32 nh_ptr = 0;
    uint32 cmd = 0;
    uint32 dest_map = 0;
    mac_addr_t mac_addr;
    DsMetFifoExcp_m met_excp;
    DsBufRetrvExcp_m bufrev_exp;
    EpeMirrorEscapeCam_m mirror_escape;
    BufferStoreCtl_m buffer_store_ctl;
  /*   BufStoreChanIdCfg_m buf_store_chan_id_cfg;*/
    EpeNextHopCtl_m ctl;

    if (p_usw_mirror_master[lchip])
    {
        return CTC_E_NONE;
    }

    sal_memset(&met_excp, 0, sizeof(DsMetFifoExcp_m));
    sal_memset(&bufrev_exp, 0, sizeof(DsBufRetrvExcp_m));
    cmd1 = DRV_IOW(DsMetFifoExcp_t, DRV_ENTRY_FLAG);
    cmd2 = DRV_IOW(DsBufRetrvExcp_t, DRV_ENTRY_FLAG);

    CTC_ERROR_RETURN(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_MIRROR_NH, &nh_ptr));

    sys_usw_get_gchip_id(lchip, &gchip_id);
    dest_map = SYS_ENCODE_DESTMAP( gchip_id, SYS_RSV_PORT_DROP_ID);

    SetDsMetFifoExcp(V, destMap_f, &met_excp, dest_map);
    SetDsBufRetrvExcp(V, nextHopPtr_f, &bufrev_exp, nh_ptr);
    SetDsBufRetrvExcp(V, resetPacketOffset_f, &bufrev_exp, 1);

    for (session_id = 0; session_id < MAX_CTC_MIRROR_SESSION_ID; session_id++)
    {
        /*port session ingress*/
        excp_index = MCHIP_CAP(SYS_CAP_MIRROR_INGRESS_L2_SPAN_INDEX_BASE) + session_id;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd1, &met_excp));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd2, &bufrev_exp));

        /*port session egress*/
        excp_index = MCHIP_CAP(SYS_CAP_MIRROR_EGRESS_L2_SPAN_INDEX_BASE) + session_id;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd1, &met_excp));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd2, &bufrev_exp));

        /*vlan session ingress*/
        excp_index = MCHIP_CAP(SYS_CAP_MIRROR_INGRESS_L3_SPAN_INDEX_BASE) + session_id;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd1, &met_excp));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd2, &bufrev_exp));

        /*vlan session egress*/
        excp_index = MCHIP_CAP(SYS_CAP_MIRROR_EGRESS_L3_SPAN_INDEX_BASE) + session_id;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd1, &met_excp));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd2, &bufrev_exp));

        /*acl session ingress*/
        excp_index = MCHIP_CAP(SYS_CAP_MIRROR_INGRESS_ACL_LOG_INDEX_BASE) + session_id;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd1, &met_excp));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd2, &bufrev_exp));

        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 4, cmd1, &met_excp));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 4, cmd2, &bufrev_exp));

        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 8, cmd1, &met_excp));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 8, cmd2, &bufrev_exp));

        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 12, cmd1, &met_excp));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 12, cmd2, &bufrev_exp));

        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 16, cmd1, &met_excp));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 16, cmd2, &bufrev_exp));

        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 20, cmd1, &met_excp));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 20, cmd2, &bufrev_exp));

        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 24, cmd1, &met_excp));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 24, cmd2, &bufrev_exp));

        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 28, cmd1, &met_excp));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 28, cmd2, &bufrev_exp));

        /*acl session egress*/
        excp_index = MCHIP_CAP(SYS_CAP_MIRROR_EGRESS_ACL_LOG_INDEX_BASE) + session_id;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd1, &met_excp));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd2, &bufrev_exp));

        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 4, cmd1, &met_excp));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 4, cmd2, &bufrev_exp));

        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 8, cmd1, &met_excp));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + 8, cmd2, &bufrev_exp));
    }


    /*Rx cpu session*/
    excp_index = MCHIP_CAP(SYS_CAP_MIRROR_CPU_RX_SPAN_INDEX);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd1, &met_excp));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd2, &bufrev_exp));

    /*Tx cpu session*/
    excp_index = MCHIP_CAP(SYS_CAP_MIRROR_CPU_TX_SPAN_INDEX);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd1, &met_excp));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd2, &bufrev_exp));

    p_usw_mirror_master[lchip] = (sys_mirror_master_t*)mem_malloc(MEM_MIRROR_MODULE, sizeof(sys_mirror_master_t));
    if (NULL == p_usw_mirror_master[lchip])
    {
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }

    sal_memset(p_usw_mirror_master[lchip], 0, sizeof(sys_mirror_master_t));
    sal_memset(&mirror_escape, 0, sizeof(EpeMirrorEscapeCam_m));
    sal_memset(p_usw_mirror_master[lchip]->iloop_port, 0xFF, (sizeof(uint16)*MAX_CTC_MIRROR_SESSION_ID*MAX_SYS_MIRROR_SESSION_TYPE*CTC_BOTH_DIRECTION));
    sal_memset(&mac_addr, 0xFF, sizeof(mac_addr_t));
    SetEpeMirrorEscapeCam(A, array_0_macDaMask_f, &mirror_escape, &mac_addr);
    SetEpeMirrorEscapeCam(A, array_1_macDaMask_f, &mirror_escape, &mac_addr);
    SetEpeMirrorEscapeCam(V, array_0_valid_f, &mirror_escape, 0);
    SetEpeMirrorEscapeCam(V, array_1_valid_f, &mirror_escape, 0);

    cmd = DRV_IOW(EpeMirrorEscapeCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mirror_escape));

    sal_memset(&buffer_store_ctl, 0, sizeof(BufferStoreCtl_m));
    cmd = DRV_IOR(BufferStoreCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buffer_store_ctl));

    SetBufferStoreCtl(V, exceptionVectorIngressBitmap_f, &buffer_store_ctl, 0x4A801); /*100 1010 1000 0000 0001*/
    SetBufferStoreCtl(V, exceptionVectorEgressBitmap_f, &buffer_store_ctl, 0xD41); /*1101 0100 0001*/
    SetBufferStoreCtl(V, cpuRxQueueModeEn_f, &buffer_store_ctl, 1);

    cmd = DRV_IOW(BufferStoreCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buffer_store_ctl));

    cmd = DRV_IOR(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &ctl));
    SetEpeNextHopCtl(V, symmetricalHashCtl_0_hashEn_f, &ctl, 1);
    SetEpeNextHopCtl(V, symmetricalHashCtl_1_hashEn_f, &ctl, 1);
    cmd = DRV_IOW(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &ctl));

    CTC_ERROR_RETURN(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_MIRROR, sys_usw_mirror_wb_sync));

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_mirror_wb_restore(lchip));
    }
    CTC_ERROR_RETURN(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_MIRROR, sys_usw_mirror_dump_db));

    return CTC_E_NONE;
}


/*
 @brief This function is to initialize the mirror module
*/
int32
sys_usw_mirror_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);

    if (!p_usw_mirror_master[lchip])
    {
        return CTC_E_NONE;
    }

    mem_free(p_usw_mirror_master[lchip]);
    return 0;
}

/*
 @brief This function is to set port able to mirror
*/
int32
sys_usw_mirror_set_port_en(uint8 lchip, uint32 gport, ctc_direction_t dir, bool enable, uint8 session_id)
{
    uint16 lport = 0;
    uint32 cmd = 0;
    BufferStoreCtl_m buffer_store_ctl;

    SYS_MIRROR_INIT_CHECK();

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (CTC_IS_CPU_PORT(gport))
    {
        /*SYSTEM MODIFY by yejl for D2 and Tsingma, 2019-01-21*/
        //if (session_id >= 1)
        //{
            //return CTC_E_INVALID_PARAM;
        //}
        /*ended*/
        sal_memset(&buffer_store_ctl, 0, sizeof(BufferStoreCtl_m));

        cmd = DRV_IOR(BufferStoreCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buffer_store_ctl));

        if ((dir == CTC_INGRESS) || (dir == CTC_BOTH_DIRECTION))
        {
            SetBufferStoreCtl(V, cpuRxExceptionEn0_f, &buffer_store_ctl, ((enable == TRUE)? 1 : 0));
            SetBufferStoreCtl(V, cpuRxExceptionEn1_f, &buffer_store_ctl, ((enable == TRUE)? 1 : 0));
        }
        if ((dir == CTC_EGRESS) || (dir == CTC_BOTH_DIRECTION))
        {
            SetBufferStoreCtl(V, cpuTxExceptionEn_f, &buffer_store_ctl, ((enable == TRUE)? 1 : 0));
        }

        cmd = DRV_IOW(BufferStoreCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buffer_store_ctl));
        return CTC_E_NONE;
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    if (TRUE == enable)
    {
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport = 0x%X, direction = %d, span_id = %d\n", gport, dir, session_id);

        if (session_id >= MAX_CTC_MIRROR_SESSION_ID)
        {
            return CTC_E_INVALID_PARAM;
        }

        CTC_ERROR_RETURN(sys_usw_port_set_internal_direction_property(lchip, gport, SYS_PORT_DIR_PROP_L2_SPAN_ID, dir, session_id));
        CTC_ERROR_RETURN(sys_usw_port_set_internal_direction_property(lchip, gport, SYS_PORT_DIR_PROP_L2_SPAN_EN, dir, TRUE));
    }
    else if (FALSE == enable)
    {
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport = 0x%X, direction = %d\n", gport, dir);

        CTC_ERROR_RETURN(sys_usw_port_set_internal_direction_property(lchip, gport, SYS_PORT_DIR_PROP_L2_SPAN_EN, dir, FALSE));
    }

    return CTC_E_NONE;
}

/*
 @brief This function is to get the information of port mirror
*/
int32
sys_usw_mirror_get_port_info(uint8 lchip, uint32 gport, ctc_direction_t dir, bool* enable, uint8* session_id)
{
    uint32 value = 0;
    uint32 value_enable = 0;

    SYS_MIRROR_INIT_CHECK();

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport = %d, direction = %d\n", gport, dir);

    CTC_PTR_VALID_CHECK(enable);
    CTC_PTR_VALID_CHECK(session_id);

    CTC_ERROR_RETURN(sys_usw_port_get_internal_direction_property(lchip, gport, SYS_PORT_DIR_PROP_L2_SPAN_EN, dir, &value_enable));
    CTC_ERROR_RETURN(sys_usw_port_get_internal_direction_property(lchip, gport, SYS_PORT_DIR_PROP_L2_SPAN_ID, dir, &value));
    *session_id = value;
    *enable = value_enable ? TRUE : FALSE;

    return CTC_E_NONE;
}

/*
 @brief This function is to set vlan able to mirror
*/
int32
sys_usw_mirror_set_vlan_en(uint8 lchip, uint16 vlan_id, ctc_direction_t dir, bool enable, uint8 session_id)
{

    int32 ret = 0;

    SYS_MIRROR_INIT_CHECK();

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if (TRUE == enable)
    {
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "vlan_id = %d, direction = %d, span_id = %d\n", vlan_id, dir, session_id);

        CTC_VLAN_RANGE_CHECK(vlan_id);

        if (session_id >= MAX_CTC_MIRROR_SESSION_ID)
        {
            return CTC_E_INVALID_PARAM;
        }

        if ((dir == CTC_INGRESS) || (dir == CTC_BOTH_DIRECTION))
        {
            ret =  sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_INGRESS_VLAN_SPAN_ID, session_id);
            ret = ret ? ret : sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_INGRESS_VLAN_SPAN_EN, 1);
        }

        if ((dir == CTC_EGRESS) || (dir == CTC_BOTH_DIRECTION))
        {
            ret = ret ? ret : sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_EGRESS_VLAN_SPAN_ID, session_id);
            ret = ret ? ret : sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_EGRESS_VLAN_SPAN_EN, 1);
        }
    }
    else if (FALSE == enable)
    {
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "vlan_id = %d, direction = %d\n", vlan_id, dir);

        CTC_VLAN_RANGE_CHECK(vlan_id);

        if ((dir == CTC_INGRESS) || (dir == CTC_BOTH_DIRECTION))
        {
            ret = sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_INGRESS_VLAN_SPAN_EN, 0);
        }

        if ((dir == CTC_EGRESS) || (dir == CTC_BOTH_DIRECTION))
        {
            ret = ret ? ret : sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_EGRESS_VLAN_SPAN_EN, 0);
        }
    }

    return ret;

}

/*
 @brief This function is to get the information of vlan mirror
*/
int32
sys_usw_mirror_get_vlan_info(uint8 lchip, uint16 vlan_id, ctc_direction_t dir, bool* enable, uint8* session_id)
{
    int32           ret = 0;
    uint32          value = 0;

    SYS_MIRROR_INIT_CHECK();

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "vlan_id = %d, direction = %d\n", vlan_id, dir);

    CTC_VLAN_RANGE_CHECK(vlan_id);
    CTC_PTR_VALID_CHECK(enable);
    CTC_PTR_VALID_CHECK(session_id);

    if (dir == CTC_INGRESS)
    {
        ret =  sys_usw_vlan_get_internal_property(lchip, vlan_id, SYS_VLAN_PROP_INGRESS_VLAN_SPAN_ID, &value);
        *session_id = value;
        ret = ret ? ret : sys_usw_vlan_get_internal_property(lchip, vlan_id, SYS_VLAN_PROP_INGRESS_VLAN_SPAN_EN, &value);
        *enable = value;
    }
    else if (dir == CTC_EGRESS)
    {
        ret =  sys_usw_vlan_get_internal_property(lchip, vlan_id, SYS_VLAN_PROP_EGRESS_VLAN_SPAN_ID, &value);
        *session_id = value;
        ret = ret ? ret : sys_usw_vlan_get_internal_property(lchip, vlan_id, SYS_VLAN_PROP_EGRESS_VLAN_SPAN_EN, &value);
        *enable = value;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return ret;

}

STATIC int32
_sys_usw_mirror_set_excp_index_and_dest_and_nhptr(uint8 lchip, ctc_mirror_dest_t* mirror, uint32 dest_gport, uint32 nh_ptr)
{
    uint32 excp_index = 0;

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "direction = %d, dest_gport = %d, session_id = %d, nh_ptr = %d\n", mirror->dir, dest_gport, mirror->session_id, nh_ptr);

    CTC_MAX_VALUE_CHECK(mirror->dir, CTC_EGRESS);
    CTC_MAX_VALUE_CHECK(mirror->session_id, MAX_CTC_MIRROR_SESSION_ID - 1);

    if (mirror->type == CTC_MIRROR_L2SPAN_SESSION)    /*port mirror*/
    {
        if (CTC_INGRESS == mirror->dir)
        {
            excp_index = MCHIP_CAP(SYS_CAP_MIRROR_INGRESS_L2_SPAN_INDEX_BASE) + mirror->session_id;
        }
        else if (CTC_EGRESS == mirror->dir)
        {
            excp_index = MCHIP_CAP(SYS_CAP_MIRROR_EGRESS_L2_SPAN_INDEX_BASE) + mirror->session_id;
        }

    }
    else if (mirror->type == CTC_MIRROR_L3SPAN_SESSION)   /*vlan mirror*/
    {
        if (CTC_INGRESS == mirror->dir)
        {
            excp_index = MCHIP_CAP(SYS_CAP_MIRROR_INGRESS_L3_SPAN_INDEX_BASE) + mirror->session_id;
        }
        else if (CTC_EGRESS == mirror->dir)
        {
            excp_index = MCHIP_CAP(SYS_CAP_MIRROR_EGRESS_L3_SPAN_INDEX_BASE) + mirror->session_id;
        }
    }
    else if (mirror->type == CTC_MIRROR_ACLLOG_SESSION)  /*acllog0 mirror set dest*/
    {
        excp_index = SYS_MIRROR_ACL_LOG_EXCEPTION_IDX(mirror->dir, mirror->session_id, mirror->acl_priority);
    }
    else if (mirror->type == CTC_MIRROR_CPU_SESSION)
    {
        /*SYSTEM MODIFY by yejl for D2 and Tsingma, 2019-01-21*/
        //CTC_MAX_VALUE_CHECK(mirror->session_id,0);
        /*endif*/

        if (CTC_INGRESS == mirror->dir)
        {
            excp_index = MCHIP_CAP(SYS_CAP_MIRROR_CPU_RX_SPAN_INDEX);
        }
        else if (CTC_EGRESS == mirror->dir)
        {
            excp_index = MCHIP_CAP(SYS_CAP_MIRROR_CPU_TX_SPAN_INDEX);
        }

    }
    else if(mirror->type == CTC_MIRROR_IPFIX_LOG_SESSION)
    {
        CTC_MAX_VALUE_CHECK(mirror->session_id,0);

        if (CTC_INGRESS == mirror->dir)
        {
            excp_index = MCHIP_CAP(SYS_CAP_MIRROR_INGRESS_IPFIX_LOG_INDEX);
        }
        else if (CTC_EGRESS == mirror->dir)
        {
            excp_index = MCHIP_CAP(SYS_CAP_MIRROR_EGRESS_IPFIX_LOG_INDEX);
        }
    }

    CTC_ERROR_RETURN(_sys_usw_mirror_set_dest_write_chip(lchip, excp_index, dest_gport, nh_ptr, mirror->truncated_len, mirror->is_rspan));
    return CTC_E_NONE;
}

/*
 @brief This function is to set remote mirror destination port
*/
STATIC int32
_sys_usw_mirror_rspan_set_dest(uint8 lchip, ctc_mirror_dest_t* mirror)
{
    uint32 dsnh_offset = 0;
    uint8 type = 0;
    uint32 temp_gport = 0;
    uint32 del_nh_id = 0;
    uint32 nh_id = CTC_MAX_UINT32_VALUE;
    ctc_rspan_nexthop_param_t  temp_p_nh_param;

    sal_memset(&temp_p_nh_param, 0, sizeof(ctc_rspan_nexthop_param_t));

    SYS_MIRROR_INIT_CHECK();
    CTC_PTR_VALID_CHECK(mirror);

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "dest_gport = %d, direction = %d, session_id = %d\n",
                       mirror->dest_gport, mirror->dir, mirror->session_id);

    SYS_GLOBAL_PORT_CHECK(mirror->dest_gport);

    if (1 == mirror->vlan_valid)
    {
        CTC_VLAN_RANGE_CHECK(mirror->rspan.vlan_id);
        if (!sys_usw_chip_is_local(lchip, CTC_MAP_GPORT_TO_GCHIP(mirror->dest_gport)) && !CTC_IS_LINKAGG_PORT(mirror->dest_gport))
        {
            temp_p_nh_param.remote_chip = TRUE;
        }

        temp_p_nh_param.rspan_vid = mirror->rspan.vlan_id;
        CTC_ERROR_RETURN(sys_usw_rspan_nh_create(lchip, &nh_id, 0, &temp_p_nh_param));
    }
    else
    {
        nh_id = mirror->rspan.nh_id;
    }

    CTC_ERROR_RETURN(sys_usw_nh_get_mirror_info_by_nhid(lchip, nh_id, &dsnh_offset, &temp_gport, TRUE));

    if (temp_gport == CTC_MAX_UINT16_VALUE)
    {
        temp_gport = mirror->dest_gport;
    }
    /*for iloop nh or mcast nh*/
    CTC_ERROR_RETURN(_sys_usw_mirror_set_iloop_port(lchip,  nh_id, &dsnh_offset, &temp_gport, mirror, TRUE));

    CTC_ERROR_RETURN(_sys_usw_mirror_set_excp_index_and_dest_and_nhptr(lchip, mirror, temp_gport, dsnh_offset));

    /*if nexthop id is internal , need to delete*/
    SYS_MIRROR_TYPE_MAPPING(mirror,type);
    del_nh_id = p_usw_mirror_master[lchip]->mirror_nh[mirror->session_id][type][mirror->dir];
    if((del_nh_id) && _sys_usw_mirror_check_internal_nexthop(lchip, del_nh_id))
    {
        CTC_ERROR_RETURN(sys_usw_rspan_nh_delete(lchip,del_nh_id));
    }

    p_usw_mirror_master[lchip]->mirror_nh[mirror->session_id][type][mirror->dir] = nh_id;

    return CTC_E_NONE;

}

/*
 @brief This function is to set local mirror destination port
*/
int32
sys_usw_mirror_set_dest(uint8 lchip, ctc_mirror_dest_t* p_mirror)
{
    uint32 nh_ptr = 0;
    uint32 nh_id = 0;
    uint8 dir[3] = {0xff, 0xff, 0xff};
    uint8 index = 0;
    uint8 type = 0;
    uint8 service_queue_egress_en = 0;

    SYS_MIRROR_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_mirror);
    CTC_MAX_VALUE_CHECK(p_mirror->truncated_len,MCHIP_CAP(SYS_CAP_PKT_TRUNCATED_LEN));
    CTC_ERROR_RETURN(sys_usw_global_get_logic_destport_en(lchip,&service_queue_egress_en));
    if(service_queue_egress_en && p_mirror->truncated_len)
    {
        return CTC_E_NOT_SUPPORT;
    }

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "dest_gport = %d, direction = %d, session_id = %d\n",
                       p_mirror->dest_gport, p_mirror->dir, p_mirror->session_id);

    if ((p_mirror->type >= MAX_CTC_MIRROR_SESSION_TYPE) || (p_mirror->session_id >= MAX_CTC_MIRROR_SESSION_ID)
        || (p_mirror->dir > CTC_BOTH_DIRECTION))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_mirror->type == CTC_MIRROR_ACLLOG_SESSION)
    {
        if (p_mirror->acl_priority >= MCHIP_CAP(SYS_CAP_MIRROR_ACL_ID))
        {
            return CTC_E_INVALID_PARAM;
        }
        if (( p_mirror->acl_priority >= 3) && ((CTC_BOTH_DIRECTION == p_mirror->dir) || (CTC_EGRESS == p_mirror->dir)))
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    SYS_MIRROR_TYPE_MAPPING(p_mirror,type);

    /*add new mirror session*/
    if (p_mirror->is_rspan)
    {
        if (p_mirror->dir != CTC_BOTH_DIRECTION)
        {
            CTC_ERROR_RETURN(_sys_usw_mirror_rspan_set_dest(lchip, p_mirror));
        }
        else if (p_mirror->dir == CTC_BOTH_DIRECTION)
        {
            p_mirror->dir = CTC_INGRESS;
            CTC_ERROR_RETURN(_sys_usw_mirror_rspan_set_dest(lchip, p_mirror));
            p_mirror->dir = CTC_EGRESS;
            CTC_ERROR_RETURN(_sys_usw_mirror_rspan_set_dest(lchip, p_mirror));
            p_mirror->dir = CTC_BOTH_DIRECTION;
        }
    }
    else
    {
        SYS_GLOBAL_PORT_CHECK(p_mirror->dest_gport);

        /*here used to remove rspan nexthop information*/
        if (p_mirror->dir != CTC_BOTH_DIRECTION)
        {
            dir[p_mirror->dir] = p_mirror->dir;
        }
        else
        {
            dir[0] = CTC_INGRESS;
            dir[1] = CTC_EGRESS;
        }

        for (index = CTC_INGRESS; index < CTC_BOTH_DIRECTION; index++)
        {
            if (dir[index] == 0xff)
            {
                continue;
            }

            if(p_mirror->dest_gport == SYS_RSV_PORT_DROP_ID)
            {
                nh_id = p_usw_mirror_master[lchip]->mirror_nh[p_mirror->session_id][type][dir[index]];
                if ((nh_id) && _sys_usw_mirror_check_internal_nexthop(lchip, nh_id))
                {
                    CTC_ERROR_RETURN(sys_usw_rspan_nh_delete(lchip, nh_id));
                }
                else if (nh_id)
                {
                    p_mirror->dir = index;
                    _sys_usw_mirror_set_iloop_port(lchip, nh_id, NULL, NULL, p_mirror, 0);
                }
                p_usw_mirror_master[lchip]->mirror_nh[p_mirror->session_id][type][dir[index]] = 0;
            }
        }
        if((dir[0] == CTC_INGRESS)&&(dir[1] == CTC_EGRESS))
        {
            p_mirror->dir = CTC_BOTH_DIRECTION;
        }

        CTC_ERROR_RETURN(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_MIRROR_NH, &nh_ptr));

        if (p_mirror->dir == CTC_INGRESS)
        {
            CTC_ERROR_RETURN(_sys_usw_mirror_set_excp_index_and_dest_and_nhptr(lchip, p_mirror, p_mirror->dest_gport, nh_ptr));
        }
        else if (p_mirror->dir == CTC_BOTH_DIRECTION)
        {
            p_mirror->dir = CTC_INGRESS;
            CTC_ERROR_RETURN(_sys_usw_mirror_set_excp_index_and_dest_and_nhptr(lchip, p_mirror, p_mirror->dest_gport, nh_ptr));
            p_mirror->dir = CTC_BOTH_DIRECTION;
        }

        if (p_mirror->dir == CTC_EGRESS)
        {
            CTC_ERROR_RETURN(_sys_usw_mirror_set_excp_index_and_dest_and_nhptr(lchip, p_mirror, p_mirror->dest_gport, nh_ptr));
        }
        else if (p_mirror->dir == CTC_BOTH_DIRECTION)
        {
            p_mirror->dir = CTC_EGRESS;
            CTC_ERROR_RETURN(_sys_usw_mirror_set_excp_index_and_dest_and_nhptr(lchip, p_mirror, p_mirror->dest_gport, nh_ptr));
            p_mirror->dir = CTC_BOTH_DIRECTION;
        }

    }

    return CTC_E_NONE;
}

int32
sys_usw_mirror_rspan_escape_en(uint8 lchip, bool enable)
{
    uint32 cmd;
    uint32 field_val;

    SYS_MIRROR_INIT_CHECK();

    field_val = (TRUE == enable) ? 1 : 0;

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_mirrorEscapeCamEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    return CTC_E_NONE;
}

int32
sys_usw_mirror_rspan_escape_mac(uint8 lchip, ctc_mirror_rspan_escape_t* p_escape)
{
    uint32 cmd = 0;
    hw_mac_addr_t mac_addr;
    EpeMirrorEscapeCam_m mirror_escape;

    SYS_MIRROR_INIT_CHECK();

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    MIRROR_SET_HW_MAC(mac_addr, p_escape->mac0);
    SetEpeMirrorEscapeCam(A, array_0_macDaValue_f, &mirror_escape, &mac_addr);
    MIRROR_SET_HW_MAC(mac_addr, p_escape->mac_mask0);
    SetEpeMirrorEscapeCam(A, array_0_macDaMask_f, &mirror_escape, &mac_addr);

    MIRROR_SET_HW_MAC(mac_addr, p_escape->mac1);
    SetEpeMirrorEscapeCam(A, array_1_macDaValue_f, &mirror_escape, &mac_addr);
    MIRROR_SET_HW_MAC(mac_addr, p_escape->mac_mask1);
    SetEpeMirrorEscapeCam(A, array_1_macDaMask_f, &mirror_escape, &mac_addr);

    SetEpeMirrorEscapeCam(V, array_0_valid_f, &mirror_escape, 1);
    SetEpeMirrorEscapeCam(V, array_1_valid_f, &mirror_escape, 1);

    cmd = DRV_IOW(EpeMirrorEscapeCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mirror_escape));

    return CTC_E_NONE;
}

/*
 @brief This function is to remove mirror destination port
*/
int32
sys_usw_mirror_unset_dest(uint8 lchip, ctc_mirror_dest_t* p_mirror)
{
    SYS_MIRROR_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_mirror);

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "session_id = %d, direction = %d, session type = %d\n",
                       p_mirror->session_id, p_mirror->dir, p_mirror->type);
    p_mirror->dest_gport = SYS_RSV_PORT_DROP_ID;
    p_mirror->is_rspan = 0;
    CTC_ERROR_RETURN(sys_usw_mirror_set_dest(lchip, p_mirror));

    return CTC_E_NONE;
}

/*
  @brief This functions is used to set packet enable or not to log if the packet is discarded on EPE process.
*/
int32
sys_usw_mirror_set_mirror_discard(uint8 lchip, ctc_direction_t dir, uint16 discard_flag, bool enable)
{
    uint32 cmd;
    uint32 logon_discard;
    uint16 discard_flag_temp = 0;

    SYS_MIRROR_INIT_CHECK();
    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "discard_flag = %d, direction = %d, enable = %d\n", discard_flag, dir, enable);
    SYS_MIRROR_DISCARD_CHECK(discard_flag);

    if (discard_flag & CTC_MIRROR_L2SPAN_DISCARD)
    {
        CTC_SET_FLAG(discard_flag_temp, SYS_MIRROR_L2SPAN_DISCARD);
    }

    if (discard_flag & CTC_MIRROR_L3SPAN_DISCARD)
    {
        CTC_SET_FLAG(discard_flag_temp, SYS_MIRROR_L3SPAN_DISCARD);
    }

    if ((CTC_INGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
    {
        if (discard_flag & CTC_MIRROR_ACLLOG_PRI_DISCARD)
        {
            CTC_SET_FLAG(discard_flag_temp, SYS_MIRROR_INGRESS_ACLLOG_PRI_DISCARD);
        }

        if (discard_flag & CTC_MIRROR_IPFIX_DISCARD)
        {
            CTC_SET_FLAG(discard_flag_temp, SYS_MIRROR_INGRESS_IPFIX_MIRROR_DISCARD);
        }

        cmd = DRV_IOR(IpeFwdCtl_t, IpeFwdCtl_logOnDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &logon_discard));

        if (TRUE == enable)
        {
            CTC_SET_FLAG(logon_discard, discard_flag_temp);
        }
        else
        {
            CTC_UNSET_FLAG(logon_discard, discard_flag_temp);
        }

        cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_logOnDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &logon_discard));
        cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_logOnDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &logon_discard));
    }

    if ((CTC_EGRESS == dir) || (CTC_BOTH_DIRECTION == dir))
    {
        if (discard_flag & CTC_MIRROR_ACLLOG_PRI_DISCARD)
        {
            CTC_SET_FLAG(discard_flag_temp, SYS_MIRROR_EGRESS_ACLLOG_PRI_DISCARD);
        }

        if (discard_flag & CTC_MIRROR_IPFIX_DISCARD)
        {
            CTC_SET_FLAG(discard_flag_temp, SYS_MIRROR_EGRESS_IPFIX_MIRROR_DISCARD);
        }

        cmd = DRV_IOR(EpeHeaderEditCtl_t, EpeHeaderEditCtl_logOnDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &logon_discard));

        if (TRUE == enable)
        {
            CTC_SET_FLAG(logon_discard, discard_flag_temp);
        }
        else
        {
            CTC_UNSET_FLAG(logon_discard, discard_flag_temp);
        }

        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_logOnDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &logon_discard));
    }

    return CTC_E_NONE;
}

int32
sys_usw_mirror_get_mirror_discard(uint8 lchip, ctc_direction_t dir, ctc_mirror_discard_t discard_flag, bool* p_enable)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint16 discard_flag_temp = 0;

    SYS_MIRROR_INIT_CHECK();
    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "discard_flag = %d, direction = %d\n", discard_flag, dir);

    CTC_PTR_VALID_CHECK(p_enable);
    SYS_MIRROR_DISCARD_CHECK(discard_flag);

    switch (discard_flag)
    {
    case CTC_MIRROR_L2SPAN_DISCARD:
    case CTC_MIRROR_L3SPAN_DISCARD:
    case CTC_MIRROR_ACLLOG_PRI_DISCARD:
    case CTC_MIRROR_IPFIX_DISCARD:
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    if (discard_flag & CTC_MIRROR_L2SPAN_DISCARD)
    {
        CTC_SET_FLAG(discard_flag_temp, SYS_MIRROR_L2SPAN_DISCARD);
    }

    if (discard_flag & CTC_MIRROR_L3SPAN_DISCARD)
    {
        CTC_SET_FLAG(discard_flag_temp, SYS_MIRROR_L3SPAN_DISCARD);
    }

    switch (dir)
    {
    case CTC_INGRESS:

       if (discard_flag & CTC_MIRROR_ACLLOG_PRI_DISCARD)
       {
            CTC_SET_FLAG(discard_flag_temp, SYS_MIRROR_INGRESS_ACLLOG_PRI_DISCARD);
       }
       if (discard_flag & CTC_MIRROR_IPFIX_DISCARD)
       {
            CTC_SET_FLAG(discard_flag_temp, SYS_MIRROR_INGRESS_IPFIX_MIRROR_DISCARD);
       }

        cmd = DRV_IOR(IpeFwdCtl_t, IpeFwdCtl_logOnDiscard_f);
        break;

    case CTC_EGRESS:

        if (discard_flag & CTC_MIRROR_ACLLOG_PRI_DISCARD)
        {
            CTC_SET_FLAG(discard_flag_temp, SYS_MIRROR_EGRESS_ACLLOG_PRI_DISCARD);
        }
        if (discard_flag & CTC_MIRROR_IPFIX_DISCARD)
        {
            CTC_SET_FLAG(discard_flag_temp, SYS_MIRROR_EGRESS_IPFIX_MIRROR_DISCARD);
        }


        cmd = DRV_IOR(EpeHeaderEditCtl_t, EpeHeaderEditCtl_logOnDiscard_f);
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    *p_enable = CTC_FLAG_ISSET(field_val, discard_flag_temp) ? TRUE : FALSE;

    return CTC_E_NONE;
}

int32
sys_usw_mirror_set_erspan_psc(uint8 lchip, ctc_mirror_erspan_psc_t* psc)
{
    uint32 value = 0;
    EpeNextHopCtl_m ctl;
    uint32 cmd = 0;

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_MIRROR_INIT_CHECK();
    CTC_PTR_VALID_CHECK(psc);

    cmd = DRV_IOR(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd , &ctl));

    if(CTC_FLAG_ISSET(psc->type, CTC_MIRROR_ERSPAN_PSC_TYPE_IPV4))
    {
        value = CTC_FLAG_ISSET(psc->ipv4_flag, CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_L4_PORT);
        SetEpeNextHopCtl(V,symmetricalHashCtl_0_l4PortEn_f, &ctl, value);

        value = CTC_FLAG_ISSET(psc->ipv4_flag, CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_IP_ADDR);
        SetEpeNextHopCtl(V,symmetricalHashCtl_0_addrEn_f, &ctl, value);

        value = CTC_FLAG_ISSET(psc->ipv4_flag, CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_IS_TCP);
        SetEpeNextHopCtl(V,symmetricalHashCtl_0_tcpEn_f, &ctl, value);

        value = CTC_FLAG_ISSET(psc->ipv4_flag, CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_IS_UDP);
        SetEpeNextHopCtl(V,symmetricalHashCtl_0_udpEn_f, &ctl, value);

        value = CTC_FLAG_ISSET(psc->ipv4_flag, CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_IS_DCCP);
        SetEpeNextHopCtl(V,symmetricalHashCtl_0_dccpEn_f, &ctl, value);

        value = CTC_FLAG_ISSET(psc->ipv4_flag, CTC_MIRROR_ERSPAN_PSC_IPV4_FLAG_IS_SCTP);
        SetEpeNextHopCtl(V,symmetricalHashCtl_0_sctpEn_f, &ctl, value);
    }

    if(CTC_FLAG_ISSET(psc->type, CTC_MIRROR_ERSPAN_PSC_TYPE_IPV6))
    {
        value = CTC_FLAG_ISSET(psc->ipv6_flag, CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_L4_PORT);
        SetEpeNextHopCtl(V,symmetricalHashCtl_1_l4PortEn_f, &ctl, value);

        value = CTC_FLAG_ISSET(psc->ipv6_flag, CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_IP_ADDR);
        SetEpeNextHopCtl(V,symmetricalHashCtl_1_addrEn_f, &ctl, value);

        value = CTC_FLAG_ISSET(psc->ipv6_flag, CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_IS_TCP);
        SetEpeNextHopCtl(V,symmetricalHashCtl_1_tcpEn_f, &ctl, value);

        value = CTC_FLAG_ISSET(psc->ipv6_flag, CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_IS_UDP);
        SetEpeNextHopCtl(V,symmetricalHashCtl_1_udpEn_f, &ctl, value);

        value = CTC_FLAG_ISSET(psc->ipv6_flag, CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_IS_DCCP);
        SetEpeNextHopCtl(V,symmetricalHashCtl_1_dccpEn_f, &ctl, value);

        value = CTC_FLAG_ISSET(psc->ipv6_flag, CTC_MIRROR_ERSPAN_PSC_IPV6_FLAG_IS_SCTP);
        SetEpeNextHopCtl(V,symmetricalHashCtl_1_sctpEn_f, &ctl, value);
    }

    cmd = DRV_IOW(EpeNextHopCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mirror_show_port_info(uint8 lchip, uint32 gport)
{
    uint16 drv_gport = 0;
    uint16 lport = 0;
    uint16 dst_gport = 0;
    uint32 value = 0;
    uint32 nh_ptr = 0;
    uint32 excp_index = 0;
    uint32 cmd = 0;
    uint32 session_id = 0;
    uint32 enable = FALSE;
    uint8  dest_is_cpu = 0;
    ctc_direction_t dir;
    DsMetFifoExcp_m met_excp;
    DsBufRetrvExcp_m bufrev_exp;
    BufferStoreCtl_m buffer_store_ctl;

    sal_memset(&met_excp, 0, sizeof(DsMetFifoExcp_m));
    sal_memset(&bufrev_exp, 0, sizeof(DsBufRetrvExcp_m));

    SYS_MIRROR_INIT_CHECK();

    if (CTC_IS_CPU_PORT(gport))
    {
        dir = CTC_INGRESS;
        cmd = DRV_IOR(BufferStoreCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buffer_store_ctl));
        GetBufferStoreCtl(A, cpuRxExceptionEn1_f, &buffer_store_ctl, &enable);

        excp_index = MCHIP_CAP(SYS_CAP_MIRROR_CPU_RX_SPAN_INDEX);
        cmd = DRV_IOR(DsMetFifoExcp_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd, &met_excp));
        GetDsMetFifoExcp(A, destMap_f, &met_excp, &value);
        drv_gport = SYS_USW_DESTMAP_TO_DRV_GPORT(value);
        dst_gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(drv_gport);

        lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(dst_gport);
        cmd = DRV_IOR(DsBufRetrvExcp_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + (SYS_MAP_DRV_LPORT_TO_SLICE(lport) * MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM)), cmd, &bufrev_exp));
        GetDsBufRetrvExcp(A, nextHopPtr_f, &bufrev_exp, &value);
        nh_ptr = value;

        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Mirror Source Port=0x%.4x \n", gport);

        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "===================================== \n");

        if (enable == TRUE)
        {
            SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Ingress Mirror :cpu mirror don't care session id \n");
            SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Mirror DestGport: 0x%.4x  NexthopPtr: 0x%.4x\n", dst_gport, nh_ptr);
        }
        else
        {
            SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Ingress Mirror is disable\n");
        }

        enable = FALSE;
        dir = CTC_EGRESS;
        cmd = DRV_IOR(BufferStoreCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buffer_store_ctl));
        GetBufferStoreCtl(A, cpuTxExceptionEn_f, &buffer_store_ctl, &enable);

        excp_index = MCHIP_CAP(SYS_CAP_MIRROR_CPU_TX_SPAN_INDEX);
        cmd = DRV_IOR(DsMetFifoExcp_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd, &met_excp));
        GetDsMetFifoExcp(A, destMap_f, &met_excp, &value);
        drv_gport = SYS_USW_DESTMAP_TO_DRV_GPORT(value);
        dst_gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(drv_gport);

        lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(dst_gport);
        cmd = DRV_IOR(DsBufRetrvExcp_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + (SYS_MAP_DRV_LPORT_TO_SLICE(lport) * MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM)), cmd, &bufrev_exp));
        GetDsBufRetrvExcp(A, nextHopPtr_f, &bufrev_exp, &value);
        nh_ptr = value;

        if (enable == TRUE)
        {
            SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Egress Mirror :cpu mirror don't care session id \n");
            SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Mirror DestGport: 0x%.4x  NexthopPtr: 0x%.4x\n", dst_gport, nh_ptr);
        }
        else
        {
            SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Egress Mirror is disable\n");
        }

        return CTC_E_NONE;
    }


    dir = CTC_INGRESS;
    CTC_ERROR_RETURN(sys_usw_port_get_internal_direction_property(lchip, gport, SYS_PORT_DIR_PROP_L2_SPAN_EN, dir, &enable));
    CTC_ERROR_RETURN(sys_usw_port_get_internal_direction_property(lchip, gport, SYS_PORT_DIR_PROP_L2_SPAN_ID, dir, &session_id));

    excp_index = MCHIP_CAP(SYS_CAP_MIRROR_INGRESS_L2_SPAN_INDEX_BASE) + session_id;
    cmd = DRV_IOR(DsMetFifoExcp_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd, &met_excp));
    GetDsMetFifoExcp(A, destMap_f, &met_excp, &value);
    if (SYS_DESTMAP_IS_CPU(value))
    {
        dest_is_cpu = 1;
    }
    drv_gport = SYS_USW_DESTMAP_TO_DRV_GPORT(value);
    dst_gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(drv_gport);

    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(dst_gport);
    cmd = DRV_IOR(DsBufRetrvExcp_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + (SYS_MAP_DRV_LPORT_TO_SLICE(lport) * MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM)), cmd, &bufrev_exp));
    GetDsBufRetrvExcp(A, nextHopPtr_f, &bufrev_exp, &value);
    nh_ptr = value;

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Mirror Source Port=0x%.4x \n", gport);

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "===================================== \n");

    if (enable == TRUE)
    {
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Ingress Mirror :session Id: %d \n", session_id);
        if (dest_is_cpu)
        {
            SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Mirror Dest: %.9s ,  CPU Reason: 0x%.4x\n", "CPU", nh_ptr>>4);
        }
        else
        {
            SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Mirror DestGport: 0x%.4x  NexthopPtr: 0x%.4x\n", dst_gport, nh_ptr);
        }
    }
    else
    {
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Ingress Mirror is disable\n");
    }

    enable = FALSE;
    dir = CTC_EGRESS;
    CTC_ERROR_RETURN(sys_usw_port_get_internal_direction_property(lchip, gport, SYS_PORT_DIR_PROP_L2_SPAN_EN, dir, &enable));
    CTC_ERROR_RETURN(sys_usw_port_get_internal_direction_property(lchip, gport, SYS_PORT_DIR_PROP_L2_SPAN_ID, dir, &session_id));

    excp_index = MCHIP_CAP(SYS_CAP_MIRROR_EGRESS_L2_SPAN_INDEX_BASE) + session_id;
    cmd = DRV_IOR(DsMetFifoExcp_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd, &met_excp));
    GetDsMetFifoExcp(A, destMap_f, &met_excp, &value);
    if (SYS_DESTMAP_IS_CPU(value))
    {
        dest_is_cpu = 1;
    }
    drv_gport = SYS_USW_DESTMAP_TO_DRV_GPORT(value);
    dst_gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(drv_gport);

    cmd = DRV_IOR(DsBufRetrvExcp_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + (SYS_MAP_DRV_LPORT_TO_SLICE(lport) * MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM)), cmd, &bufrev_exp));
    GetDsBufRetrvExcp(A, nextHopPtr_f, &bufrev_exp, &value);
    nh_ptr = value;

    if (enable == TRUE)
    {
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Egress Mirror :session Id: %d \n", session_id);
        if (dest_is_cpu)
        {
            SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Mirror Dest: %.9s , CPU Reason: 0x%.4x\n", "CPU", nh_ptr>>4);
        }
        else
        {
            SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Mirror DestGport: 0x%.4x  NexthopPtr: 0x%.4x\n", dst_gport, nh_ptr);
        }
    }
    else
    {
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Egress Mirror is disable\n");
    }

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "===================================== \n");

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mirror_show_vlan_info(uint8 lchip, uint16 vlan_id)
{
    uint16 drv_gport = 0;
    uint16 dst_gport = 0;
    uint16 lport = 0;
    uint32 nh_ptr = 0;
    uint32 excp_index = 0;
    uint32 cmd = 0;
    uint32 session_id = 0;
    uint32 enable = FALSE;
    uint32 value = 0;
    uint8  dest_is_cpu = 0;
    DsMetFifoExcp_m met_excp;
    DsBufRetrvExcp_m bufrev_exp;

    sal_memset(&met_excp, 0, sizeof(DsMetFifoExcp_m));
    sal_memset(&bufrev_exp, 0, sizeof(DsBufRetrvExcp_m));

    SYS_MIRROR_INIT_CHECK();
    CTC_VLAN_RANGE_CHECK(vlan_id);

    CTC_ERROR_RETURN(sys_usw_vlan_get_internal_property(lchip, vlan_id, SYS_VLAN_PROP_INGRESS_VLAN_SPAN_ID, &session_id));
    CTC_ERROR_RETURN(sys_usw_vlan_get_internal_property(lchip, vlan_id, SYS_VLAN_PROP_INGRESS_VLAN_SPAN_EN, &enable));

    excp_index = MCHIP_CAP(SYS_CAP_MIRROR_INGRESS_L3_SPAN_INDEX_BASE) + session_id;
    cmd = DRV_IOR(DsMetFifoExcp_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd, &met_excp));
    GetDsMetFifoExcp(A, destMap_f, &met_excp, &value);
    drv_gport = SYS_USW_DESTMAP_TO_DRV_GPORT(value);
    dst_gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(drv_gport);

    lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(dst_gport);
    cmd = DRV_IOR(DsBufRetrvExcp_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + (SYS_MAP_DRV_LPORT_TO_SLICE(lport) * MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM)), cmd, &bufrev_exp));
    GetDsBufRetrvExcp(A, nextHopPtr_f, &bufrev_exp, &value);

    nh_ptr = value;

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Mirror Source Vlan=%d \n", vlan_id);
    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "===================================== \n");

    if (enable == TRUE)
    {
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Ingress Mirror :session Id: %d \n", session_id);
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Mirror DestGport: 0x%.4x  NexthopPtr: 0x%.4x\n", dst_gport, nh_ptr);
    }
    else
    {
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Ingress Mirror is disable\n");
    }

    enable = FALSE;
    CTC_ERROR_RETURN(sys_usw_vlan_get_internal_property(lchip, vlan_id, SYS_VLAN_PROP_EGRESS_VLAN_SPAN_ID, &session_id));
    CTC_ERROR_RETURN(sys_usw_vlan_get_internal_property(lchip, vlan_id, SYS_VLAN_PROP_EGRESS_VLAN_SPAN_EN, &enable));

    excp_index = MCHIP_CAP(SYS_CAP_MIRROR_EGRESS_L3_SPAN_INDEX_BASE) + session_id;
    cmd = DRV_IOR(DsMetFifoExcp_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd, &met_excp));
    GetDsMetFifoExcp(A, destMap_f, &met_excp, &value);
    if (SYS_DESTMAP_IS_CPU(value))
    {
        dest_is_cpu = 1;
    }
    drv_gport = SYS_USW_DESTMAP_TO_DRV_GPORT(value);
    dst_gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(drv_gport);

    cmd = DRV_IOR(DsBufRetrvExcp_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index + (SYS_MAP_DRV_LPORT_TO_SLICE(lport) * MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM)), cmd, &bufrev_exp));
    GetDsBufRetrvExcp(A, nextHopPtr_f, &bufrev_exp, &value);
    nh_ptr = value;

    if (enable == TRUE)
    {
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Egress Mirror :session Id: %d \n", session_id);
        if (dest_is_cpu)
        {
            SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Mirror Dest: %.9s , CPU Reason: %.0x4x\n", "CPU", nh_ptr>>4);
        }
        else
        {
            SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Mirror DestGport: 0x%.4x  NexthopPtr: 0x%.4x\n", dst_gport, nh_ptr);
        }
    }
    else
    {
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Egress Mirror is disable\n");
    }

    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "===================================== \n");

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mirror_show_acl_info(uint8 lchip, uint16 log_id)
{
    uint8  priority_id = 0;
    uint8  max_priority_id = 0;
    uint16 drv_gport = 0;
    uint16 dst_gport = 0;
    uint32 nh_ptr = 0;
    uint32 excp_index = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 dest_map = 0;
    uint32 direction = 0;
    DsMetFifoExcp_m met_excp;
    DsBufRetrvExcp_m bufrev_exp;
    char* desc[10] = {"Ingress", "Egress"};
    uint32 acl_log_base[2] = {MCHIP_CAP(SYS_CAP_MIRROR_INGRESS_ACL_LOG_INDEX_BASE), MCHIP_CAP(SYS_CAP_MIRROR_EGRESS_ACL_LOG_INDEX_BASE)};

    sal_memset(&met_excp, 0, sizeof(DsMetFifoExcp_m));
    sal_memset(&bufrev_exp, 0, sizeof(DsBufRetrvExcp_m));

    SYS_MIRROR_INIT_CHECK();

    if (log_id >= MCHIP_CAP(SYS_CAP_MIRROR_ACL_LOG_ID))
    {
        SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Mirror] Invalid mirror log id \n");
			return CTC_E_BADID;

    }

    for (direction = 0; direction < 2; direction++)
    {
        if(direction == 0)
        {
            max_priority_id = MCHIP_CAP(SYS_CAP_MIRROR_INGRESS_ACL_LOG_PRIORITY);
        }
        else
        {
            max_priority_id = MCHIP_CAP(SYS_CAP_MIRROR_EGRESS_ACL_LOG_PRIORITY);
        }

        for (priority_id = 0; priority_id < max_priority_id; priority_id++)
        {
            excp_index = acl_log_base[direction] + (MCHIP_CAP(SYS_CAP_MIRROR_ACL_LOG_ID) * priority_id) + log_id;
            cmd = DRV_IOR(DsMetFifoExcp_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd, &met_excp));
            GetDsMetFifoExcp(A, destMap_f, &met_excp, &dest_map);
            drv_gport = SYS_USW_DESTMAP_TO_DRV_GPORT(dest_map);
            dst_gport = SYS_MAP_DRV_GPORT_TO_CTC_GPORT(drv_gport);

            cmd = DRV_IOR(DsBufRetrvExcp_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, excp_index, cmd, &bufrev_exp));
            GetDsBufRetrvExcp(A, nextHopPtr_f, &bufrev_exp, &value);
            nh_ptr = value;

            SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "========================================= \n");
            SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s Mirror Acl Log Id: %d\n", desc[direction], log_id);

            if (SYS_RSV_PORT_DROP_ID != (dest_map & 0xFF))
            {
                SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Mirror Priority Id: %d DestGport: 0x%.4x  NexthopPtr: 0x%.4x\n", priority_id, dst_gport, nh_ptr);
            }
            else
            {
                SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s Mirror Priority Id: %d is disable\n", desc[direction], priority_id);
            }
        }
    }
    SYS_MIRROR_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "========================================= \n");

    return CTC_E_NONE;
}

int32
sys_usw_mirror_show_info(uint8 lchip, ctc_mirror_info_type_t type, uint32 value)
{
    SYS_MIRROR_INIT_CHECK();

    if (CTC_MIRROR_INFO_PORT == type)
    {
        SYS_MAP_GPORT_TO_LCHIP(value, lchip);
        CTC_ERROR_RETURN(_sys_usw_mirror_show_port_info(lchip, value));
    }
    else if (CTC_MIRROR_INFO_VLAN == type)
    {
        CTC_ERROR_RETURN(_sys_usw_mirror_show_vlan_info(lchip, value));
    }
    else if (CTC_MIRROR_INFO_ACL == type)
    {
        CTC_ERROR_RETURN(_sys_usw_mirror_show_acl_info(lchip, value));
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

