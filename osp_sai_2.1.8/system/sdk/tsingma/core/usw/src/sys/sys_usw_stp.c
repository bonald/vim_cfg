/**
 @file

 @date 2009-10-20

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
#include "ctc_const.h"

#include "sys_usw_chip.h"
#include "sys_usw_stp.h"
#include "sys_usw_vlan.h"
#include "sys_usw_register.h"
#include "sys_usw_wb_common.h"
#include "usw/include/drv_io.h"

/***************************************************************
*
*  Defines and Macros
*
***************************************************************/


#define SYS_STP_ID_VALID_CHECK(stp_id)          \
{                                               \
    if ((stp_id) >= p_usw_stp_master[lchip]->instance_num) \
    {\
        SYS_STP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid stp id \n");\
        return CTC_E_BADID;\
    }\
}

#define SYS_STP_DBG_OUT(level, FMT, ...) CTC_DEBUG_OUT(l2, stp, L2_STP_SYS, level, FMT, ##__VA_ARGS__)

struct sys_stp_master_s
{
    uint16 port_num; /* Stp port num in one slice */
    uint8 instance_num;
    uint8 shift_num; /* Lport shift num for DsStpState index */
};
typedef struct sys_stp_master_s sys_stp_master_t;

/****************************************************************************
Global and Declaration
*****************************************************************************/

sys_stp_master_t* p_usw_stp_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

int32 sys_usw_l2_stp_wb_restore(uint8 lchip);
/**********************************************************************************
Define API function interfaces
***********************************************************************************/

/**
 @brief   the function is to clear all instances belonged to one port
*/
int32
sys_usw_stp_clear_all_inst_state(uint8 lchip, uint32 gport)
{
    uint16 lport    = 0;
    uint16 entry_num = 0;
    uint32 index    = 0;
    uint32 cmd      = 0;
    uint32 loop     = 0;
    DsSrcStpState_m ds_src_stp_state;
    DsDestStpState_m ds_dest_stp_state;

    SYS_STP_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    SYS_STP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_STP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport:0x%04X \n", gport);

    lport = lport % MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM);

    if (lport >= p_usw_stp_master[lchip]->port_num)
    {
        return CTC_E_INVALID_PARAM;
    }


    if (DRV_IS_DUET2(lchip))
    {
        entry_num = MCHIP_CAP(SYS_CAP_STP_STATE_ENTRY_NUM) / p_usw_stp_master[lchip]->port_num;
        index = ((lport << p_usw_stp_master[lchip]->shift_num) & 0x1FF);

        sal_memset(&ds_src_stp_state, 0, sizeof(DsSrcStpState_m));

        cmd = DRV_IOW(DsSrcStpState_t, DRV_ENTRY_FLAG);
        for (loop = 0; loop < entry_num; loop++, index++)
        {
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_src_stp_state));
        }

        index = ((lport << p_usw_stp_master[lchip]->shift_num) & 0x1FF);
        sal_memset(&ds_dest_stp_state, 0, sizeof(DsDestStpState_m));

        cmd = DRV_IOW(DsDestStpState_t, DRV_ENTRY_FLAG);
        for (loop = 0; loop < entry_num; loop++, index++)
        {
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_dest_stp_state));
        }
    }
    else
    {
         uint8 stp_id = 0;
         uint32 field_offset = 0;
         uint32 field_val = 0;

        field_offset = lport & 0x1F;
         
        for (stp_id  = 0; stp_id < 128; stp_id++)
        { 
            index = (stp_id << 3) + ((lport >> 5) & 0x07);

            cmd = DRV_IOW(DsSrcStpState_t,  DsSrcStpState_array_0_stpState_f + field_offset);
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));

            cmd = DRV_IOW(DsDestStpState_t,  DsDestStpState_array_0_stpState_f + field_offset);
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));
        
        }
    }

    return CTC_E_NONE;
}

/**
 @brief the function is to set stp id (MSTI instance) for vlan
*/
int32
sys_usw_stp_set_vlan_stpid(uint8 lchip, uint16 vlan_id,  uint8 stpid)
{
    SYS_STP_INIT_CHECK();
    SYS_STP_ID_VALID_CHECK(stpid);

    SYS_STP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_STP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "vlan_id :%d  stpid:%d \n", vlan_id, stpid);

    CTC_ERROR_RETURN(sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_STP_ID, stpid));
    return CTC_E_NONE;
}

/**
 @brief the function is to set stp id (MSTI instance) for vlan
*/
int32
sys_usw_stp_get_vlan_stpid(uint8 lchip, uint16 vlan_id,  uint8* stpid)
{
    uint32 value_32 = 0;

    SYS_STP_INIT_CHECK();
    CTC_PTR_VALID_CHECK(stpid);

    SYS_STP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_STP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "vlan_id :%d\n", vlan_id);

    CTC_ERROR_RETURN(sys_usw_vlan_get_internal_property(lchip, vlan_id, SYS_VLAN_PROP_STP_ID, &value_32));
    *stpid = value_32 & 0xff;

    return CTC_E_NONE;
}

/**
 @brief    the function is to update the state
*/
int32
sys_usw_stp_set_state(uint8 lchip, uint32 gport,  uint8 stpid, uint8 state)
{
    uint16 lport        = 0;
    uint32 index        = 0;
    uint32 cmd          = 0;
    uint32 field_val    = 0;
    uint32 field_id     = 0;
    uint8 field_offset = 0;

    SYS_STP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_STP_INIT_CHECK();
    SYS_STP_ID_VALID_CHECK(stpid);
    CTC_MAX_VALUE_CHECK(state, 2);
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    SYS_STP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set gport:0x%04X, stp_id:%d, state:%d\n", gport, stpid, state);

    lport = lport % MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM);

    if (lport >= p_usw_stp_master[lchip]->port_num)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (DRV_IS_DUET2(lchip))       /**< [TM] index and field offset changed */
    {
        index = ((lport << p_usw_stp_master[lchip]->shift_num) & 0x1FF) + ((stpid >> 4) & 0x7);
        field_offset = stpid & 0xF;
    }
    else
    {
        index = (stpid << 3) + ((lport >> 5) & 0x07);
        field_offset = lport & 0x1F;
    }
    field_val = state;

    field_id = DsSrcStpState_array_0_stpState_f + field_offset;
    cmd = DRV_IOW(DsSrcStpState_t, field_id);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));

    field_id = DsDestStpState_array_0_stpState_f + field_offset;
    cmd = DRV_IOW(DsDestStpState_t, field_id);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));

    return CTC_E_NONE;
}

/**
 @brief the function is to get the instance of the port
*/
int32
sys_usw_stp_get_state(uint8 lchip, uint32 gport, uint8 stpid, uint8* state)
{
    uint16 lport        = 0;
    uint32 cmd          = 0;
    uint32 index        = 0;
    uint32 field_val    = 0;
    uint32 field_id     = 0;
    uint8 field_offset = 0;

    SYS_STP_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_STP_INIT_CHECK();
    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    SYS_STP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get gport:0x%04X, stp id:%d, state\n", gport, stpid);

    lport = lport % MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM);

    if (lport >= p_usw_stp_master[lchip]->port_num)
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_STP_ID_VALID_CHECK(stpid);
    CTC_PTR_VALID_CHECK(state);

    if (DRV_IS_DUET2(lchip))       /**< [TM] index and field offset changed */
    {
        index = ((lport << p_usw_stp_master[lchip]->shift_num) & 0x1FF) + ((stpid >> 4) & 0x7);
        field_offset = stpid & 0xF;
    }
    else
    {
        index = (stpid << 3) + ((lport >> 5) & 0x07);
        field_offset = lport & 0x1F;
    }

    field_id = DsSrcStpState_array_0_stpState_f + field_offset;
    cmd = DRV_IOR(DsSrcStpState_t, field_id);

    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field_val));

    *state = field_val & 0x3;

    return CTC_E_NONE;
}

/**
 @brief initialize the stp module
*/
int32
sys_usw_stp_init(uint8 lchip, uint8 stp_mode)
{
    uint8  gchip = 0;
    uint16 gport = 0;
    uint16 lport = 0;
    uint32 cmd = 0;
    uint32 field = 0;
    uint32 value = 0;

    p_usw_stp_master[lchip] = (sys_stp_master_t*)mem_malloc(MEM_VLAN_MODULE, sizeof(sys_stp_master_t));
    if (NULL == p_usw_stp_master[lchip])
    {
        SYS_STP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
			return CTC_E_NO_MEMORY;

    }
    sal_memset(p_usw_stp_master[lchip], 0, sizeof(sys_stp_master_t));
    if (DRV_IS_DUET2(lchip))
    {
        switch(stp_mode)
        {
            /* 32 stp instance, 256 port per slice, 256*2 port per chip */
            case CTC_STP_MODE_32:
                p_usw_stp_master[lchip]->instance_num = 32;
                p_usw_stp_master[lchip]->port_num = 256;
                p_usw_stp_master[lchip]->shift_num = 1;
                break;
            /* 64 stp instance, 128 port per slice, 128*2 port per chip */
            case CTC_STP_MODE_64:
                p_usw_stp_master[lchip]->instance_num = 64;
                p_usw_stp_master[lchip]->port_num = 128;
                p_usw_stp_master[lchip]->shift_num = 2;
                break;
            /* 128 stp instance, 64 port per slice, 64*2 port per chip */
            case CTC_STP_MODE_128:
                p_usw_stp_master[lchip]->instance_num = 128;
                p_usw_stp_master[lchip]->port_num = 64;
                p_usw_stp_master[lchip]->shift_num = 3;
                break;
            default:
                return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        p_usw_stp_master[lchip]->instance_num = 128;
        p_usw_stp_master[lchip]->port_num = 256;
    }
    sys_usw_get_gchip_id(lchip, &gchip);

    field = p_usw_stp_master[lchip]->shift_num;
    cmd = DRV_IOW(IpeDsVlanCtl_t, IpeDsVlanCtl_dsStpStateShift_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

    field = p_usw_stp_master[lchip]->shift_num;
    cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_dsStpStateShift_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

    for (lport = 0; lport < SYS_USW_MAX_PORT_NUM_PER_CHIP; lport++)
    {
        if (lport < p_usw_stp_master[lchip]->port_num)
        {
            gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
            CTC_ERROR_RETURN(sys_usw_stp_clear_all_inst_state(lchip, gport));
        }
        else
        {
            /*disable stp check*/
            value = 0;
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_stpCheckEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));

            value = 1;
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_stpDisable_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, lport, cmd, &value));
        }
    }

    if (DRV_IS_DUET2(lchip) && CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_l2_stp_wb_restore(lchip));
    }

    /* set chip_capability */
    MCHIP_CAP(SYS_CAP_SPEC_STP_INSTANCE_NUM) = p_usw_stp_master[lchip]->instance_num;

    return CTC_E_NONE;
}

int32
sys_usw_stp_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == p_usw_stp_master[lchip])
    {
        return CTC_E_NONE;
    }

    mem_free(p_usw_stp_master[lchip]);

    return CTC_E_NONE;
}

int32
sys_usw_l2_stp_wb_restore(uint8 lchip)
{
    uint32 cmd = DRV_IOR(IpeDsVlanCtl_t, IpeDsVlanCtl_dsStpStateShift_f);
    uint32 shift_v = 0;

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &shift_v));
    if(p_usw_stp_master[lchip]->shift_num != shift_v)
    {
        return CTC_E_VERSION_MISMATCH;
    }

    return CTC_E_NONE;
}
