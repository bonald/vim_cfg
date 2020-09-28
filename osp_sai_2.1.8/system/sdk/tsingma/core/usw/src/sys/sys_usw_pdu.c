/**
 @file sys_usw_pdu.c

 @date 2009-12-30

 @version v2.0

---file comments----
*/

#include "sal.h"
#include "ctc_error.h"
#include "ctc_vector.h"
#include "sys_usw_pdu.h"
#include "sys_usw_chip.h"
#include "sys_usw_port.h"
#include "sys_usw_common.h"
#include "sys_usw_l3if.h"

#include "drv_api.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/

#define PDU_SET_MAC(d, s)       sal_memcpy(d, s, sizeof(mac_addr_t))
#define PDU_SET_HW_MAC(d, s)    SYS_USW_SET_HW_MAC(d, s)

#define IS_MAC_ALL_ZERO(mac)      (!mac[0] && !mac[1] && !mac[2] && !mac[3] && !mac[4] && !mac[5])
#define IS_MAC_LOW24BIT_ZERO(mac) (!mac[3] && !mac[4] && !mac[5])


/***************************************************************
*
*  Functions
*
***************************************************************/
STATIC int32
_sys_usw_l2pdu_classify_l2pdu_by_l2hdr_proto(uint8 lchip, uint8 index, ctc_pdu_l2pdu_key_t* p_entry)
{
    int32  step = 0;
    uint32 cmd = 0;
    uint32 field = 0;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ethertype:0x%x\n", p_entry->l2hdr_proto);

    field = p_entry->l2hdr_proto;
    if (index >= MCHIP_CAP(SYS_CAP_L2PDU_BASED_L2HDR_PTL_ENTRY))
    {
        return CTC_E_INVALID_PARAM;
    }

    step = IpeBpduProtocolEscapeCam3_array_1_cam3EntryValid_f - IpeBpduProtocolEscapeCam3_array_0_cam3EntryValid_f;
    cmd = DRV_IOW(IpeBpduProtocolEscapeCam3_t, IpeBpduProtocolEscapeCam3_array_0_cam3EtherType_f + (index * step));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_get_classified_key_by_l2hdr_proto(uint8 lchip, uint8 index, ctc_pdu_l2pdu_key_t* p_entry)
{
    int32  step = 0;
    uint32 cmd = 0;
    uint32 field = 0;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    if (index >= MCHIP_CAP(SYS_CAP_L2PDU_BASED_L2HDR_PTL_ENTRY))
    {
        return CTC_E_INVALID_PARAM;
    }

    step = IpeBpduProtocolEscapeCam3_array_1_cam3EntryValid_f - IpeBpduProtocolEscapeCam3_array_0_cam3EntryValid_f;
    cmd = DRV_IOR(IpeBpduProtocolEscapeCam3_t, IpeBpduProtocolEscapeCam3_array_0_cam3EtherType_f + (index * step));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

    p_entry->l2hdr_proto = field;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_classify_l2pdu_by_macda(uint8 lchip, uint8 index, ctc_pdu_l2pdu_key_t* p_entry)
{
    uint32 cmd = 0;
    int32  index_step = IpeBpduProtocolEscapeCam_array_1_cam1MacDaValue_f
                        - IpeBpduProtocolEscapeCam_array_0_cam1MacDaValue_f;
    uint32 mac_da_value_field_id = IpeBpduProtocolEscapeCam_array_0_cam1MacDaValue_f;
    uint32 mac_da_mask_field_id = IpeBpduProtocolEscapeCam_array_0_cam1MacDaMask_f;
    IpeBpduProtocolEscapeCam_m cam;
    hw_mac_addr_t mac_addr;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "macda:%x %x %x %x %x %x\n", p_entry->l2pdu_by_mac.mac[0], p_entry->l2pdu_by_mac.mac[1], \
                      p_entry->l2pdu_by_mac.mac[2], p_entry->l2pdu_by_mac.mac[3], p_entry->l2pdu_by_mac.mac[4], p_entry->l2pdu_by_mac.mac[5]);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "mask: %x %x %x %x %x %x\n", p_entry->l2pdu_by_mac.mac_mask[0], p_entry->l2pdu_by_mac.mac_mask[1], \
                      p_entry->l2pdu_by_mac.mac_mask[2], p_entry->l2pdu_by_mac.mac_mask[3], p_entry->l2pdu_by_mac.mac_mask[4], p_entry->l2pdu_by_mac.mac_mask[5]);

    if (index >= MCHIP_CAP(SYS_CAP_L2PDU_BASED_MACDA_ENTRY))
    {
        return CTC_E_INVALID_PARAM;
    }

    if ((p_entry->l2pdu_by_mac.mac_mask[0] == 0) && (p_entry->l2pdu_by_mac.mac_mask[1] == 0)
        && (p_entry->l2pdu_by_mac.mac_mask[2] == 0) && (p_entry->l2pdu_by_mac.mac_mask[3] == 0)
        && (p_entry->l2pdu_by_mac.mac_mask[4] == 0) && (p_entry->l2pdu_by_mac.mac_mask[5] == 0))
    {
        sal_memset(p_entry->l2pdu_by_mac.mac_mask, 0xFF, sizeof(mac_addr_t));
    }

    cmd = DRV_IOR(IpeBpduProtocolEscapeCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    mac_da_value_field_id += index_step * index;
    mac_da_mask_field_id += index_step * index;

    PDU_SET_HW_MAC(mac_addr, p_entry->l2pdu_by_mac.mac);
    DRV_SET_FIELD_A(lchip, IpeBpduProtocolEscapeCam_t, mac_da_value_field_id, &cam, (uint32*)&mac_addr);
    PDU_SET_HW_MAC(mac_addr, p_entry->l2pdu_by_mac.mac_mask);
    DRV_SET_FIELD_A(lchip, IpeBpduProtocolEscapeCam_t, mac_da_mask_field_id, &cam, (uint32*)&mac_addr);

    cmd = DRV_IOW(IpeBpduProtocolEscapeCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_get_classified_key_by_macda(uint8 lchip, uint8 index, ctc_pdu_l2pdu_key_t* p_entry)
{
    uint32 cmd = 0;
    int32  index_step = IpeBpduProtocolEscapeCam_array_1_cam1MacDaValue_f
                        - IpeBpduProtocolEscapeCam_array_0_cam1MacDaValue_f;
    uint32 mac_da_value_field_id = IpeBpduProtocolEscapeCam_array_0_cam1MacDaValue_f;
    uint32 mac_da_mask_field_id = IpeBpduProtocolEscapeCam_array_0_cam1MacDaMask_f;
    IpeBpduProtocolEscapeCam_m cam;
    hw_mac_addr_t mac_addr;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    if (index >= MCHIP_CAP(SYS_CAP_L2PDU_BASED_MACDA_ENTRY))
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&cam, 0, sizeof(IpeBpduProtocolEscapeCam_m));

    cmd = DRV_IOR(IpeBpduProtocolEscapeCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    mac_da_value_field_id += index_step * index;
    mac_da_mask_field_id += index_step * index;

    DRV_GET_FIELD_A(lchip, IpeBpduProtocolEscapeCam_t, mac_da_value_field_id, &cam, (uint32*)&mac_addr);
    SYS_USW_SET_USER_MAC(p_entry->l2pdu_by_mac.mac, mac_addr);

    DRV_GET_FIELD_A(lchip, IpeBpduProtocolEscapeCam_t, mac_da_mask_field_id, &cam, (uint32*)&mac_addr);
    SYS_USW_SET_USER_MAC(p_entry->l2pdu_by_mac.mac_mask, mac_addr);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_classify_l2pdu_by_macda_low24(uint8 lchip, uint8 index, ctc_pdu_l2pdu_key_t* entry)
{
    int32  cam2_index_step = IpeBpduProtocolEscapeCam2_array_1_cam2MacDaValue_f
                             - IpeBpduProtocolEscapeCam2_array_0_cam2MacDaValue_f;
    int32  cam4_index_step = IpeBpduProtocolEscapeCam4_array_1_cam4MacDa_f
                             - IpeBpduProtocolEscapeCam4_array_0_cam4MacDa_f;
    uint32 cmd = 0;
    uint32 cam2_mac_da_mask = 0;
    uint32 cam2_mac_da_value = 0;
    uint32 cam4_mac_da_value = 0;
    uint32 cam2_mac_da_mask_field_id = IpeBpduProtocolEscapeCam2_array_0_cam2MacDaMask_f;
    uint32 cam2_mac_da_value_field_id = IpeBpduProtocolEscapeCam2_array_0_cam2MacDaValue_f;
    uint32 cam4_mac_da_value_field_id = IpeBpduProtocolEscapeCam4_array_0_cam4MacDa_f;
    IpeBpduProtocolEscapeCam2_m cam2;
    IpeBpduProtocolEscapeCam4_m cam4;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "macda:%x %x %x %x %x %x\n", entry->l2pdu_by_mac.mac[0], entry->l2pdu_by_mac.mac[1], \
                      entry->l2pdu_by_mac.mac[2], entry->l2pdu_by_mac.mac[3], entry->l2pdu_by_mac.mac[4], entry->l2pdu_by_mac.mac[5]);

    if (index >= MCHIP_CAP(SYS_CAP_L2PDU_BASED_MACDA_LOW24_ENTRY))
    {
        return CTC_E_INVALID_PARAM;
    }

    cam2_mac_da_mask_field_id += cam2_index_step * index;
    cam2_mac_da_value_field_id += cam2_index_step * index;
    cam4_mac_da_value_field_id += cam4_index_step * (index - 2);

    if ((entry->l2pdu_by_mac.mac[0] != 0x01) || (entry->l2pdu_by_mac.mac[1] != 0x80)
        || (entry->l2pdu_by_mac.mac[2] != 0xc2))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (index < 2)
    {
        if ((entry->l2pdu_by_mac.mac_mask[0] != 0xff) || (entry->l2pdu_by_mac.mac_mask[1] != 0xff)
           || (entry->l2pdu_by_mac.mac_mask[2] != 0xff))
        {
            if ((entry->l2pdu_by_mac.mac_mask[0] == 0) && (entry->l2pdu_by_mac.mac_mask[1] == 0)
                && (entry->l2pdu_by_mac.mac_mask[2] == 0) && (entry->l2pdu_by_mac.mac_mask[3] == 0)
                && (entry->l2pdu_by_mac.mac_mask[4] == 0) && (entry->l2pdu_by_mac.mac_mask[5] == 0))
            {
                cam2_mac_da_mask = 0xffffff;
            }
            else
            {
                return CTC_E_INVALID_PARAM;
            }
        }

        if (0xffffff != cam2_mac_da_mask)
        {
            cam2_mac_da_mask = entry->l2pdu_by_mac.mac_mask[3] << 16
                             | entry->l2pdu_by_mac.mac_mask[4] << 8 | entry->l2pdu_by_mac.mac_mask[5];
            cam2_mac_da_mask &= 0xffffff;
        }

        cam2_mac_da_value =  entry->l2pdu_by_mac.mac[3] << 16
                            | entry->l2pdu_by_mac.mac[4] << 8 | entry->l2pdu_by_mac.mac[5];
        cam2_mac_da_value &= 0xffffff;

        sal_memset(&cam2, 0, sizeof(IpeBpduProtocolEscapeCam2_m));
        cmd = DRV_IOR(IpeBpduProtocolEscapeCam2_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam2));

        DRV_SET_FIELD_V(lchip, IpeBpduProtocolEscapeCam2_t, cam2_mac_da_value_field_id, &cam2, cam2_mac_da_value);
        DRV_SET_FIELD_V(lchip, IpeBpduProtocolEscapeCam2_t, cam2_mac_da_mask_field_id, &cam2, cam2_mac_da_mask);

        cmd = DRV_IOW(IpeBpduProtocolEscapeCam2_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam2));
    }
    else
    {
          /*only suport all 0 and all 1*/
          if ((entry->l2pdu_by_mac.mac_mask[0] == 0) && (entry->l2pdu_by_mac.mac_mask[1] == 0)
                && (entry->l2pdu_by_mac.mac_mask[2] == 0) && (entry->l2pdu_by_mac.mac_mask[3] == 0)
                && (entry->l2pdu_by_mac.mac_mask[4] == 0) && (entry->l2pdu_by_mac.mac_mask[5] == 0))
          {
               /*as 0xffffff*/
          }
         else  if ((entry->l2pdu_by_mac.mac_mask[0] != 0xFF) || (entry->l2pdu_by_mac.mac_mask[1] != 0xFF)
                || (entry->l2pdu_by_mac.mac_mask[2] != 0xFF) || (entry->l2pdu_by_mac.mac_mask[3] != 0xFF)
                || (entry->l2pdu_by_mac.mac_mask[4] != 0xFF) || (entry->l2pdu_by_mac.mac_mask[5] != 0xFF))

         {
            return CTC_E_INVALID_PARAM;
         }

        cam4_mac_da_value =  entry->l2pdu_by_mac.mac[3] << 16
            | entry->l2pdu_by_mac.mac[4] << 8 | entry->l2pdu_by_mac.mac[5];
        cam4_mac_da_value &= 0xffffff;

        sal_memset(&cam4, 0, sizeof(IpeBpduProtocolEscapeCam4_m));
        cmd = DRV_IOR(IpeBpduProtocolEscapeCam4_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam4));

        DRV_SET_FIELD_V(lchip, IpeBpduProtocolEscapeCam4_t, cam4_mac_da_value_field_id, &cam4, cam4_mac_da_value);

        cmd = DRV_IOW(IpeBpduProtocolEscapeCam4_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam4));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_get_classified_key_by_macda_low24(uint8 lchip, uint8 index, ctc_pdu_l2pdu_key_t* p_entry)
{
    int32  step1 = IpeBpduProtocolEscapeCam2_array_1_cam2MacDaValue_f
                   - IpeBpduProtocolEscapeCam2_array_0_cam2MacDaValue_f;
    int32  step2 = IpeBpduProtocolEscapeCam4_array_1_cam4MacDa_f
                   - IpeBpduProtocolEscapeCam4_array_0_cam4MacDa_f;
    uint32 cmd = 0;
    uint32 cam_mac_da_value[2] = {0};
    uint32 cam_mac_da_mask[2] = {0};
    uint32 cam2_mac_da_mask_field_id = IpeBpduProtocolEscapeCam2_array_0_cam2MacDaMask_f;
    uint32 cam2_mac_da_value_field_id = IpeBpduProtocolEscapeCam2_array_0_cam2MacDaValue_f;
    uint32 cam4_mac_da_value_field_id = IpeBpduProtocolEscapeCam4_array_0_cam4MacDa_f;

    IpeBpduProtocolEscapeCam2_m cam2;
    IpeBpduProtocolEscapeCam4_m cam4;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    if (index >= MCHIP_CAP(SYS_CAP_L2PDU_BASED_MACDA_LOW24_ENTRY))
    {
        return CTC_E_INVALID_PARAM;
    }

    cam2_mac_da_mask_field_id += step1 * index;
    cam2_mac_da_value_field_id += step1 * index;
    cam4_mac_da_value_field_id += step2 * (index - 2);

    if (index < 2)
    {
        sal_memset(&cam2, 0, sizeof(IpeBpduProtocolEscapeCam2_m));
        cmd = DRV_IOR(IpeBpduProtocolEscapeCam2_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam2));

        cam_mac_da_value[0] = 0xc2 << 24;
        cam_mac_da_value[0] |= DRV_GET_FIELD_V(lchip, IpeBpduProtocolEscapeCam2_t, cam2_mac_da_value_field_id, &cam2);
        cam_mac_da_value[1] = 0x0180;
        SYS_USW_SET_USER_MAC(p_entry->l2pdu_by_mac.mac, cam_mac_da_value);

        cam_mac_da_mask[0] = 0xFF << 24;
        cam_mac_da_mask[0] |= DRV_GET_FIELD_V(lchip, IpeBpduProtocolEscapeCam2_t, cam2_mac_da_mask_field_id, &cam2);
        cam_mac_da_value[1] = 0xFFFF;
        SYS_USW_SET_USER_MAC(p_entry->l2pdu_by_mac.mac_mask, cam_mac_da_mask);
    }
    else
    {
        sal_memset(&cam4, 0, sizeof(IpeBpduProtocolEscapeCam4_m));
        cmd = DRV_IOR(IpeBpduProtocolEscapeCam4_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam4));

        cam_mac_da_value[0] = 0xc2 << 24;
        cam_mac_da_value[0] |= DRV_GET_FIELD_V(lchip, IpeBpduProtocolEscapeCam4_t, cam4_mac_da_value_field_id, &cam4);
        cam_mac_da_value[1] = 0x0180;
        SYS_USW_SET_USER_MAC(p_entry->l2pdu_by_mac.mac, cam_mac_da_value);

        cam_mac_da_mask[0] = 0xFFFFFFFF;
        cam_mac_da_mask[1] = 0xFFFF;
        SYS_USW_SET_USER_MAC(p_entry->l2pdu_by_mac.mac_mask, cam_mac_da_mask);
    }

    return CTC_E_NONE;
}

/**
 @brief  Classify layer2 pdu based on macda,macda-low24 bit, layer2 header protocol
*/
int32
sys_usw_l2pdu_classify_l2pdu(uint8 lchip, ctc_pdu_l2pdu_type_t l2pdu_type, uint8 index, ctc_pdu_l2pdu_key_t* p_key)
{
    CTC_PTR_VALID_CHECK(p_key);
    SYS_LCHIP_CHECK_ACTIVE(lchip);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l2pdu type:%d\n", l2pdu_type);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    switch (l2pdu_type)
    {
    case CTC_PDU_L2PDU_TYPE_L2HDR_PROTO:
        CTC_ERROR_RETURN(_sys_usw_l2pdu_classify_l2pdu_by_l2hdr_proto(lchip, index, p_key));
        break;

    case CTC_PDU_L2PDU_TYPE_MACDA:
        CTC_ERROR_RETURN(_sys_usw_l2pdu_classify_l2pdu_by_macda(lchip, index, p_key));
        break;

    case CTC_PDU_L2PDU_TYPE_MACDA_LOW24:
        CTC_ERROR_RETURN(_sys_usw_l2pdu_classify_l2pdu_by_macda_low24(lchip, index, p_key));
        break;

    default:
        return CTC_E_INVALID_PARAM;
        break;
    }

    return CTC_E_NONE;
}

int32
sys_usw_l2pdu_get_classified_key(uint8 lchip, ctc_pdu_l2pdu_type_t l2pdu_type, uint8 index, ctc_pdu_l2pdu_key_t* p_key)
{
    CTC_PTR_VALID_CHECK(p_key);
    SYS_LCHIP_CHECK_ACTIVE(lchip);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l2pdu type:%d\n", l2pdu_type);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    switch (l2pdu_type)
    {
    case CTC_PDU_L2PDU_TYPE_L2HDR_PROTO:
        CTC_ERROR_RETURN(_sys_usw_l2pdu_get_classified_key_by_l2hdr_proto(lchip, index, p_key));
        break;

    case CTC_PDU_L2PDU_TYPE_MACDA:
        CTC_ERROR_RETURN(_sys_usw_l2pdu_get_classified_key_by_macda(lchip, index, p_key));
        break;

    case CTC_PDU_L2PDU_TYPE_MACDA_LOW24:
        CTC_ERROR_RETURN(_sys_usw_l2pdu_get_classified_key_by_macda_low24(lchip, index, p_key));
        break;

    default:
        return CTC_E_INVALID_PARAM;
        break;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_l2pdu_set_global_action_by_l3type(uint8 lchip, uint8 index, ctc_pdu_global_l2pdu_action_t* p_ctl)
{
    int32  step = 0;
    uint32 cmd = 0;
    uint32 protocol_exception = 0;
    uint32 exception_sub_index = 0;
    uint32 exception_sub_index_field_id = 0;
    IpeBridgeCtl_m ctl;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "action index:%d\n", p_ctl->action_index);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "copy to cpu:%d\n", p_ctl->entry_valid);

    if (index >= MAX_CTC_PARSER_L3_TYPE)
    {
        return CTC_E_INVALID_PARAM;
    }

    step = IpeBridgeCtl_array_1_protocolExceptionSubIndex_f - IpeBridgeCtl_array_0_protocolExceptionSubIndex_f;
    exception_sub_index_field_id = IpeBridgeCtl_array_0_protocolExceptionSubIndex_f + (index * step);
    exception_sub_index = p_ctl->action_index;

    sal_memset(&ctl, 0, sizeof(IpeBridgeCtl_m));
    cmd = DRV_IOR(IpeBridgeCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ctl));

    GetIpeBridgeCtl(A, protocolException_f, &ctl, &protocol_exception);
    if (p_ctl->entry_valid)
    {
        protocol_exception |= (1 << index);
    }
    else
    {
        protocol_exception &= (~(1 << index));
    }

    SetIpeBridgeCtl(V, protocolException_f, &ctl, protocol_exception);
    DRV_SET_FIELD_V(lchip, IpeBridgeCtl_t, exception_sub_index_field_id, &ctl, exception_sub_index);

    cmd = DRV_IOW(IpeBridgeCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_get_global_action_by_l3type(uint8 lchip, uint8 index, ctc_pdu_global_l2pdu_action_t* p_ctl)
{
    int32  step = 0;

    uint32 cmd = 0;
    uint32 protocol_exception = 0;
    uint32 exception_sub_index = 0;
    uint32 exception_sub_index_field_id = 0;
    IpeBridgeCtl_m ctl;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    if (index >= MAX_CTC_PARSER_L3_TYPE)
    {
        return CTC_E_INVALID_PARAM;
    }

    step = IpeBridgeCtl_array_1_protocolExceptionSubIndex_f - IpeBridgeCtl_array_0_protocolExceptionSubIndex_f;
    exception_sub_index_field_id = IpeBridgeCtl_array_0_protocolExceptionSubIndex_f + (index * step);

    sal_memset(&ctl, 0, sizeof(IpeBridgeCtl_m));
    cmd = DRV_IOR(IpeBridgeCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ctl));

    GetIpeBridgeCtl(A, protocolException_f, &ctl, &protocol_exception);
    p_ctl->entry_valid = (protocol_exception & (1 << index)) ? 1 : 0;

    DRV_GET_FIELD_A(lchip, IpeBridgeCtl_t, exception_sub_index_field_id, &ctl, &exception_sub_index);
    p_ctl->action_index = exception_sub_index;
    if (p_ctl->action_index)
    {
        p_ctl->entry_valid = 1;
    }

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "action index:%d\n", p_ctl->action_index);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "copy to cpu:%d\n", p_ctl->entry_valid);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_set_global_action_of_fixed_l2pdu(uint8 lchip, ctc_pdu_l2pdu_type_t type,
                                                       ctc_pdu_global_l2pdu_action_t* p_action)
{
    uint32 cmd = 0;
    uint32 enable = p_action->entry_valid ? 1 : 0;
    IpeBpduEscapeCtl_m ctl;
    IpeIntfMapperCtl_m ipe_intf_mapper_ctl;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "action index:%d\n", p_action->action_index);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "copy to cpu:%d\n", p_action->entry_valid);

    sal_memset(&ctl, 0, sizeof(IpeBpduEscapeCtl_m));
    sal_memset(&ipe_intf_mapper_ctl, 0, sizeof(IpeIntfMapperCtl_m));

    cmd = DRV_IOR(IpeIntfMapperCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_intf_mapper_ctl));

    cmd = DRV_IOR(IpeBpduEscapeCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ctl));

    switch (type)
    {
    case CTC_PDU_L2PDU_TYPE_BPDU:
        SetIpeBpduEscapeCtl(V, bpduExceptionEn_f, &ctl, enable);
        SetIpeBpduEscapeCtl(V, bpduExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L2PDU_TYPE_SLOW_PROTO:
        SetIpeBpduEscapeCtl(V, slowProtocolExceptionEn_f, &ctl, enable);
        SetIpeBpduEscapeCtl(V, slowProtocolExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L2PDU_TYPE_EFM_OAM:
        SetIpeBpduEscapeCtl(V, efmSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L2PDU_TYPE_EAPOL:
        SetIpeBpduEscapeCtl(V, eapolExceptionEn_f, &ctl, enable);
        SetIpeBpduEscapeCtl(V, eapolExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L2PDU_TYPE_LLDP:
        SetIpeBpduEscapeCtl(V, lldpExceptionEn_f, &ctl, enable);
        SetIpeBpduEscapeCtl(V, lldpExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L2PDU_TYPE_ISIS:
        SetIpeBpduEscapeCtl(V, l2isisExceptionEn_f, &ctl, enable);
        SetIpeBpduEscapeCtl(V, l2isisExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L2PDU_TYPE_FIP:
        CTC_MAX_VALUE_CHECK(p_action->action_index,15);
        SetIpeIntfMapperCtl(V, fipExceptionSubIndex_f, &ipe_intf_mapper_ctl, p_action->action_index);
        break;

    default:
        break;
    }

    cmd = DRV_IOW(IpeBpduEscapeCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ctl));

    cmd = DRV_IOW(IpeIntfMapperCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_intf_mapper_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_get_global_action_of_fixed_l2pdu(uint8 lchip, ctc_pdu_l2pdu_type_t type,
                                                       ctc_pdu_global_l2pdu_action_t* p_action)
{
    uint32 cmd = 0;
    uint32 exception_en = 0;
    uint32 exception_sub_index = 0;
    IpeBpduEscapeCtl_m ctl;
    IpeIntfMapperCtl_m ipe_intf_mapper_ctl;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&ctl, 0, sizeof(IpeBpduEscapeCtl_m));
    cmd = DRV_IOR(IpeBpduEscapeCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ctl));

    sal_memset(&ipe_intf_mapper_ctl, 0, sizeof(IpeIntfMapperCtl_m));
    cmd = DRV_IOR(IpeIntfMapperCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_intf_mapper_ctl));

    switch (type)
    {
    case CTC_PDU_L2PDU_TYPE_BPDU:
        GetIpeBpduEscapeCtl(A, bpduExceptionEn_f, &ctl, &exception_en);
        GetIpeBpduEscapeCtl(A, bpduExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L2PDU_TYPE_SLOW_PROTO:
        GetIpeBpduEscapeCtl(A, slowProtocolExceptionEn_f, &ctl, &exception_en);
        GetIpeBpduEscapeCtl(A, slowProtocolExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L2PDU_TYPE_EFM_OAM:
        exception_en = 1;
        GetIpeBpduEscapeCtl(A, efmSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L2PDU_TYPE_EAPOL:
        GetIpeBpduEscapeCtl(A, eapolExceptionEn_f, &ctl, &exception_en);
        GetIpeBpduEscapeCtl(A, eapolExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L2PDU_TYPE_LLDP:
        GetIpeBpduEscapeCtl(A, lldpExceptionEn_f, &ctl, &exception_en);
        GetIpeBpduEscapeCtl(A, lldpExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L2PDU_TYPE_ISIS:
        GetIpeBpduEscapeCtl(A, l2isisExceptionEn_f, &ctl, &exception_en);
        GetIpeBpduEscapeCtl(A, l2isisExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L2PDU_TYPE_FIP:
        exception_en = 1;
        GetIpeIntfMapperCtl(A, fipExceptionSubIndex_f, &ipe_intf_mapper_ctl, &exception_sub_index);
        break;

    default:
        break;
    }

    p_action->entry_valid = exception_en;
    p_action->action_index = exception_sub_index;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "action index:%d\n", p_action->action_index);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "copy to cpu:%d\n", p_action->entry_valid);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_set_global_action_by_l2hdr_proto(uint8 lchip, uint8 index, ctc_pdu_global_l2pdu_action_t* p_action)
{
    int32  step = 0;
    uint32 cmd = 0;

    uint32 field = 0;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "entry valid:%d\n", p_action->entry_valid);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "action index:%d\n", p_action->action_index);

    if (index >= MCHIP_CAP(SYS_CAP_L2PDU_BASED_L2HDR_PTL_ENTRY))
    {
        return CTC_E_INVALID_PARAM;
    }

    step = IpeBpduProtocolEscapeCamResult3_array_1_cam3ExceptionSubIndex_f
           - IpeBpduProtocolEscapeCamResult3_array_0_cam3ExceptionSubIndex_f;
    field = p_action->action_index;
    cmd = DRV_IOW(IpeBpduProtocolEscapeCamResult3_t, IpeBpduProtocolEscapeCamResult3_array_0_cam3ExceptionSubIndex_f + (step * index));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

    step = IpeBpduProtocolEscapeCam3_array_1_cam3EntryValid_f
           - IpeBpduProtocolEscapeCam3_array_0_cam3EntryValid_f;
    field = p_action->entry_valid;
    cmd = DRV_IOW(IpeBpduProtocolEscapeCam3_t, IpeBpduProtocolEscapeCam3_array_0_cam3EntryValid_f + (step * index));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_get_global_action_by_l2hdr_proto(uint8 lchip, uint8 index, ctc_pdu_global_l2pdu_action_t* p_action)
{
    int32  step = 0;

    uint32 cmd = 0;
    uint32 field = 0;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    if (index >= MCHIP_CAP(SYS_CAP_L2PDU_BASED_L2HDR_PTL_ENTRY))
    {
        return CTC_E_INVALID_PARAM;
    }

    step = IpeBpduProtocolEscapeCamResult3_array_1_cam3ExceptionSubIndex_f
           - IpeBpduProtocolEscapeCamResult3_array_0_cam3ExceptionSubIndex_f;

    cmd = DRV_IOR(IpeBpduProtocolEscapeCamResult3_t,
                  IpeBpduProtocolEscapeCamResult3_array_0_cam3ExceptionSubIndex_f + (index * step) );
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));
    p_action->action_index = field;

    step = IpeBpduProtocolEscapeCam3_array_1_cam3EntryValid_f - IpeBpduProtocolEscapeCam3_array_0_cam3EntryValid_f;

    cmd = DRV_IOR(IpeBpduProtocolEscapeCam3_t, IpeBpduProtocolEscapeCam3_array_0_cam3EntryValid_f + (index * step));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

    p_action->entry_valid = field;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "entry valid:%d\n", p_action->entry_valid);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "action index:%d\n", p_action->action_index);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_set_global_action_by_macda(uint8 lchip, uint8 index, ctc_pdu_global_l2pdu_action_t* p_ctl)
{
    int32  step = 0;
    uint32 cmd = 0;

    uint32 entry_valid = 0;
    uint32 exception_sub_index = 0;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "entry valid:%d\n", p_ctl->entry_valid);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "action index:%d\n", p_ctl->action_index);

    if (index >= MCHIP_CAP(SYS_CAP_L2PDU_BASED_MACDA_ENTRY))
    {
        return CTC_E_INVALID_PARAM;
    }

    entry_valid = p_ctl->entry_valid ? 1 : 0;
    exception_sub_index = p_ctl->action_index;

    step = IpeBpduProtocolEscapeCam_array_1_cam1EntryValid_f - IpeBpduProtocolEscapeCam_array_0_cam1EntryValid_f;
    cmd = DRV_IOW(IpeBpduProtocolEscapeCam_t, IpeBpduProtocolEscapeCam_array_0_cam1EntryValid_f + (step * index));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &entry_valid));

    step = IpeBpduProtocolEscapeCamResult_array_1_cam1ExceptionSubIndex_f - IpeBpduProtocolEscapeCamResult_array_0_cam1ExceptionSubIndex_f;
    cmd = DRV_IOW(IpeBpduProtocolEscapeCamResult_t, IpeBpduProtocolEscapeCamResult_array_0_cam1ExceptionSubIndex_f + (step * index));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &exception_sub_index));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_get_global_action_by_macda(uint8 lchip, uint8 index, ctc_pdu_global_l2pdu_action_t* p_action)
{
    int32  step = 0;

    uint32 cmd = 0;
    uint32 entry_valid = 0;
    uint32 exception_sub_index = 0;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    if (index >= MCHIP_CAP(SYS_CAP_L2PDU_BASED_MACDA_ENTRY))
    {
        return CTC_E_INVALID_PARAM;
    }

    step = IpeBpduProtocolEscapeCam_array_1_cam1EntryValid_f - IpeBpduProtocolEscapeCam_array_0_cam1EntryValid_f;
    cmd = DRV_IOR(IpeBpduProtocolEscapeCam_t, IpeBpduProtocolEscapeCam_array_0_cam1EntryValid_f + (step * index));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &entry_valid));

    step = IpeBpduProtocolEscapeCamResult_array_1_cam1ExceptionSubIndex_f - IpeBpduProtocolEscapeCamResult_array_0_cam1ExceptionSubIndex_f;
    cmd = DRV_IOR(IpeBpduProtocolEscapeCamResult_t, IpeBpduProtocolEscapeCamResult_array_0_cam1ExceptionSubIndex_f + (step * index));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &exception_sub_index));

    p_action->entry_valid = entry_valid;
    p_action->action_index = exception_sub_index;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "entry valid:%d\n", p_action->entry_valid);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "action index:%d\n", p_action->action_index);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_set_global_action_by_macda_low24(uint8 lchip, uint8 index, ctc_pdu_global_l2pdu_action_t* p_action)
{
    int32  step = 0;
    uint32 cmd = 0;

    uint32 entry_valid = 0;
    uint32 exception_sub_index = 0;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "entry valid:%d\n", p_action->entry_valid);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "action index:%d\n", p_action->action_index);

    if (index >= MCHIP_CAP(SYS_CAP_L2PDU_BASED_MACDA_LOW24_ENTRY))
    {
        return CTC_E_INVALID_PARAM;
    }

    entry_valid = p_action->entry_valid ? 1 : 0;
    exception_sub_index = p_action->action_index;

    if (index < 2)
    {
        step = IpeBpduProtocolEscapeCam2_array_1_cam2EntryValid_f
               - IpeBpduProtocolEscapeCam2_array_0_cam2EntryValid_f;
        cmd = DRV_IOW(IpeBpduProtocolEscapeCam2_t, IpeBpduProtocolEscapeCam2_array_0_cam2EntryValid_f + (step * index));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &entry_valid));

        step = IpeBpduProtocolEscapeCamResult2_array_1_cam2ExceptionSubIndex_f
               - IpeBpduProtocolEscapeCamResult2_array_0_cam2ExceptionSubIndex_f;
        cmd = DRV_IOW(IpeBpduProtocolEscapeCamResult2_t, IpeBpduProtocolEscapeCamResult2_array_0_cam2ExceptionSubIndex_f + (step * index));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &exception_sub_index));
    }
    else
    {
        step = IpeBpduProtocolEscapeCam4_array_1_cam4EntryValid_f - IpeBpduProtocolEscapeCam4_array_0_cam4EntryValid_f;
        cmd = DRV_IOW(IpeBpduProtocolEscapeCam4_t,
                      IpeBpduProtocolEscapeCam4_array_0_cam4EntryValid_f + (step * (index - 2)));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &entry_valid));

        step = IpeBpduProtocolEscapeCamResult4_array_1_cam4ExceptionSubIndex_f
               - IpeBpduProtocolEscapeCamResult4_array_0_cam4ExceptionSubIndex_f;
        cmd = DRV_IOW(IpeBpduProtocolEscapeCamResult4_t,
                      IpeBpduProtocolEscapeCamResult4_array_0_cam4ExceptionSubIndex_f + (step * (index - 2)));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &exception_sub_index));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_get_global_action_by_macda_low24(uint8 lchip, uint8 index, ctc_pdu_global_l2pdu_action_t* p_action)
{
    int32  step = 0;
    uint32 cmd = 0;

    uint32 entry_valid = 0;
    uint32 exception_sub_index = 0;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    if (index >= MCHIP_CAP(SYS_CAP_L2PDU_BASED_MACDA_LOW24_ENTRY))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (index < 2)
    {
        step = IpeBpduProtocolEscapeCam2_array_1_cam2EntryValid_f - IpeBpduProtocolEscapeCam2_array_0_cam2EntryValid_f;
        cmd = DRV_IOR(IpeBpduProtocolEscapeCam2_t, IpeBpduProtocolEscapeCam2_array_0_cam2EntryValid_f + (step * index));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &entry_valid));

        step = IpeBpduProtocolEscapeCamResult2_array_1_cam2ExceptionSubIndex_f
               - IpeBpduProtocolEscapeCamResult2_array_0_cam2ExceptionSubIndex_f;
        cmd = DRV_IOR(IpeBpduProtocolEscapeCamResult2_t, IpeBpduProtocolEscapeCamResult2_array_0_cam2ExceptionSubIndex_f + (step * index));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &exception_sub_index));
    }
    else
    {
        step = IpeBpduProtocolEscapeCam4_array_1_cam4EntryValid_f - IpeBpduProtocolEscapeCam4_array_0_cam4EntryValid_f;
        cmd = DRV_IOR(IpeBpduProtocolEscapeCam4_t,
                      IpeBpduProtocolEscapeCam4_array_0_cam4EntryValid_f + (step * (index - 2)));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &entry_valid));

        step = IpeBpduProtocolEscapeCamResult4_array_1_cam4ExceptionSubIndex_f
               - IpeBpduProtocolEscapeCamResult4_array_0_cam4ExceptionSubIndex_f;
        cmd = DRV_IOR(IpeBpduProtocolEscapeCamResult4_t,
                      IpeBpduProtocolEscapeCamResult4_array_0_cam4ExceptionSubIndex_f + (step * (index - 2)));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &exception_sub_index));
    }

    p_action->entry_valid = entry_valid;
    p_action->action_index = exception_sub_index;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "entry valid:%d\n", p_action->entry_valid);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "action index:%d\n", p_action->action_index);

    return CTC_E_NONE;
}

/**
 @brief  Set layer2 pdu global property
*/
int32
sys_usw_l2pdu_set_global_action(uint8 lchip, ctc_pdu_l2pdu_type_t l2pdu_type, uint8 index,
                                       ctc_pdu_global_l2pdu_action_t* p_action)
{
    CTC_PTR_VALID_CHECK(p_action);
    SYS_LCHIP_CHECK_ACTIVE(lchip);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l2pdu type:%d\n", l2pdu_type);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    /*layer3 type pdu can not per port control, action index: 32-53*/
    if (CTC_PDU_L2PDU_TYPE_L3TYPE == l2pdu_type)
    {
        SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
			return CTC_E_NOT_SUPPORT;

    }
    else
    {
        if (p_action->action_index > MCHIP_CAP(SYS_CAP_L2PDU_PORT_ACTION_INDEX))
        {
            return CTC_E_INVALID_PARAM;
        }

        switch (l2pdu_type)
        {
        case CTC_PDU_L2PDU_TYPE_BPDU:
        case CTC_PDU_L2PDU_TYPE_SLOW_PROTO:
        case CTC_PDU_L2PDU_TYPE_EFM_OAM:
        case CTC_PDU_L2PDU_TYPE_EAPOL:
        case CTC_PDU_L2PDU_TYPE_LLDP:
        case CTC_PDU_L2PDU_TYPE_ISIS:
        case CTC_PDU_L2PDU_TYPE_FIP:

            CTC_ERROR_RETURN(_sys_usw_l2pdu_set_global_action_of_fixed_l2pdu(lchip, l2pdu_type, p_action));
            break;

        case CTC_PDU_L2PDU_TYPE_L2HDR_PROTO:
            CTC_ERROR_RETURN(_sys_usw_l2pdu_set_global_action_by_l2hdr_proto(lchip, index, p_action));
            break;

        case CTC_PDU_L2PDU_TYPE_MACDA:
            CTC_ERROR_RETURN(_sys_usw_l2pdu_set_global_action_by_macda(lchip, index, p_action));
            break;

        case CTC_PDU_L2PDU_TYPE_MACDA_LOW24:
            CTC_ERROR_RETURN(_sys_usw_l2pdu_set_global_action_by_macda_low24(lchip, index, p_action));
            break;

        default:
            break;
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_l2pdu_get_global_action(uint8 lchip, ctc_pdu_l2pdu_type_t l2pdu_type, uint8 index,
                                      ctc_pdu_global_l2pdu_action_t* p_action)
{
    CTC_PTR_VALID_CHECK(p_action);
    SYS_LCHIP_CHECK_ACTIVE(lchip);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l2pdu type:%d\n", l2pdu_type);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    switch (l2pdu_type)
    {
    case CTC_PDU_L2PDU_TYPE_L2HDR_PROTO:
        CTC_ERROR_RETURN(_sys_usw_l2pdu_get_global_action_by_l2hdr_proto(lchip, index, p_action));
        break;

    case CTC_PDU_L2PDU_TYPE_MACDA:
        CTC_ERROR_RETURN(_sys_usw_l2pdu_get_global_action_by_macda(lchip, index, p_action));
        break;

    case CTC_PDU_L2PDU_TYPE_MACDA_LOW24:
        CTC_ERROR_RETURN(_sys_usw_l2pdu_get_global_action_by_macda_low24(lchip, index, p_action));
        break;

    case CTC_PDU_L2PDU_TYPE_L3TYPE:
        CTC_ERROR_RETURN(_sys_usw_l2pdu_get_global_action_by_l3type(lchip, index, p_action));
        break;

    case CTC_PDU_L2PDU_TYPE_BPDU:
    case CTC_PDU_L2PDU_TYPE_SLOW_PROTO:
    case CTC_PDU_L2PDU_TYPE_EFM_OAM:
    case CTC_PDU_L2PDU_TYPE_EAPOL:
    case CTC_PDU_L2PDU_TYPE_LLDP:
    case CTC_PDU_L2PDU_TYPE_ISIS:
    case CTC_PDU_L2PDU_TYPE_FIP:

        CTC_ERROR_RETURN(_sys_usw_l2pdu_get_global_action_of_fixed_l2pdu(lchip, l2pdu_type, p_action));
        break;

    default:
        break;
    }

    return CTC_E_NONE;
}

/**
 @brief  Per port control layer2 pdu action
*/
int32
sys_usw_l2pdu_set_port_action(uint8 lchip, uint32 gport, uint8 action_index, ctc_pdu_port_l2pdu_action_t action)
{
    uint32 excp2_en = 0;
    uint32 excp2_discard = 0;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "gport:0x%04X\n", gport);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "action index:%d\n", action_index);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "action:%d\n", action);
    SYS_LCHIP_CHECK_ACTIVE(lchip);
    if (action_index > MCHIP_CAP(SYS_CAP_L2PDU_PORT_ACTION_INDEX))
    {
        return CTC_E_INVALID_PARAM;
    }

    switch (action)
    {
    case CTC_PDU_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU:
        /*excption2En set, exception2Discard set*/
        CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_EN, &excp2_en));
        CTC_SET_FLAG(excp2_en, (1 << action_index));
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_EN, excp2_en));

        CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_DISCARD, &excp2_discard));
        CTC_SET_FLAG(excp2_discard, (1 << action_index));
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_DISCARD, excp2_discard));
        break;

    case CTC_PDU_L2PDU_ACTION_TYPE_COPY_TO_CPU:
        /*excption2En set , exception2Discard clear*/
        CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_EN, &excp2_en));
        CTC_SET_FLAG(excp2_en, (1 << action_index));
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_EN, excp2_en));

        CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_DISCARD, &excp2_discard));
        CTC_UNSET_FLAG(excp2_discard, (1 << action_index));
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_DISCARD, excp2_discard));
        break;

    case CTC_PDU_L2PDU_ACTION_TYPE_FWD:
        /*excption2En clear , exception2Discard clear*/
        CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_EN, &excp2_en));
        CTC_UNSET_FLAG(excp2_en, (1 << action_index));
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_EN, excp2_en));

        CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_DISCARD, &excp2_discard));
        CTC_UNSET_FLAG(excp2_discard, (1 << action_index));
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_DISCARD, excp2_discard));
        break;

    case CTC_PDU_L2PDU_ACTION_TYPE_DISCARD:
        /*excption2En clear , exception2Discard set*/
        CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_EN, &excp2_en));
        CTC_UNSET_FLAG(excp2_en, (1 << action_index));
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_EN, excp2_en));

        CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_DISCARD, &excp2_discard));
        CTC_SET_FLAG(excp2_discard, (1 << action_index));
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_DISCARD, excp2_discard));
        break;

    default:
        return CTC_E_INVALID_PARAM;
        break;
    }

    return CTC_E_NONE;
}

int32
sys_usw_l2pdu_get_port_action(uint8 lchip, uint32 gport, uint8 action_index, ctc_pdu_port_l2pdu_action_t* p_action)
{
    uint32 excp2_en = 0;
    uint32 excp2_discard = 0;
    uint8  excp2_en_flag = 0;
    uint8  excp2_discard_flag = 0;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "gport:0x%04X\n", gport);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "action index:%d\n", action_index);
    SYS_LCHIP_CHECK_ACTIVE(lchip);
    if (action_index > MCHIP_CAP(SYS_CAP_L2PDU_PORT_ACTION_INDEX))
    {
        return CTC_E_INVALID_PARAM;
    }

    /*excption2En set, exception2Discard set*/
    CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_EN, &excp2_en));
    if (CTC_FLAG_ISSET(excp2_en, (1 << action_index)))
    {
        excp2_en_flag = 1;
    }

    CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_DISCARD, &excp2_discard));
    if (CTC_FLAG_ISSET(excp2_discard, (1 << action_index)))
    {
        excp2_discard_flag = 1;
    }

    if (excp2_en_flag && excp2_discard_flag)
    {
        *p_action = CTC_PDU_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU;
    }
    else if (excp2_en_flag)
    {
        *p_action = CTC_PDU_L2PDU_ACTION_TYPE_COPY_TO_CPU;
    }
    else if (excp2_discard_flag)
    {
        *p_action = CTC_PDU_L2PDU_ACTION_TYPE_DISCARD;
    }
    else
    {
        *p_action = CTC_PDU_L2PDU_ACTION_TYPE_FWD;
    }

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "action:%d\n", *p_action);

    return CTC_E_NONE;
}

int32
sys_usw_l2pdu_show_port_action(uint8 lchip, uint32 gport)
{
    uint8 action_index = 0;
    uint32 excp2_en = 0;
    uint32 excp2_discard = 0;
    uint8 excp2_en_flag = 0;
    uint8 excp2_discard_flag = 0;
    SYS_LCHIP_CHECK_ACTIVE(lchip);
    LCHIP_CHECK(lchip);
    /*excption2En set, exception2Discard set*/
    CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_EN, &excp2_en));
    CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_EXCEPTION_DISCARD, &excp2_discard));

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n--------------------------------------------------------------------------------------------------------------\n");
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  Action Index               Action(Forward, Copy-to-cpu, Redirect-to-cpu or Discard)\n");
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------------------------------------------------------------------------------------------------------\n");

    for (action_index = 0; action_index < 32; action_index++)
    {
        excp2_en_flag = CTC_FLAG_ISSET(excp2_en, (1 << action_index));
        excp2_discard_flag = CTC_FLAG_ISSET(excp2_discard, (1 << action_index));
        if (excp2_en_flag && excp2_discard_flag)
        {
            SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-27d%s\n", action_index, "Redirect-to-cpu");
        }
        else if (excp2_en_flag)
        {
            SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-27d%s\n", action_index, "Copy-to-cpu");
        }
        else if (excp2_discard_flag)
        {
            SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-27d%s\n", action_index, "Discard");
        }
        else
        {
            SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "  %-27d%s\n", action_index, "Forward");
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_classify_l3pdu_by_l3hdr_proto(uint8 lchip, uint8 index, ctc_pdu_l3pdu_key_t* p_entry)
{
    int32  step = 0;
    uint32 cmd = 0;

    uint32 l3_header_protocol = 0;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l3 header protocol:%d\n", p_entry->l3hdr_proto);

    if (index >= MCHIP_CAP(SYS_CAP_L3PDU_BASED_L3HDR_PROTO))
    {
        return CTC_E_INVALID_PARAM;
    }

    l3_header_protocol = p_entry->l3hdr_proto;

    step = IpeException3Cam_array_1_l3HeaderProtocol_f - IpeException3Cam_array_0_l3HeaderProtocol_f;
    cmd = DRV_IOW(IpeException3Cam_t, IpeException3Cam_array_0_l3HeaderProtocol_f + (index * step));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &l3_header_protocol));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_get_classified_key_by_l3hdr_proto(uint8 lchip, uint8 index, ctc_pdu_l3pdu_key_t* p_entry)
{
    int32  step = 0;

    uint32 cmd = 0;
    uint32 l3_header_protocol = 0;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    if (index >= MCHIP_CAP(SYS_CAP_L3PDU_BASED_L3HDR_PROTO))
    {
        return CTC_E_INVALID_PARAM;
    }

    step = IpeException3Cam_array_1_l3HeaderProtocol_f - IpeException3Cam_array_0_l3HeaderProtocol_f;
    cmd = DRV_IOR(IpeException3Cam_t, IpeException3Cam_array_0_l3HeaderProtocol_f + (index * step));
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &l3_header_protocol));

    p_entry->l3hdr_proto = l3_header_protocol;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "l3 header protocol:%d\n", p_entry->l3hdr_proto);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_classify_l3pdu_by_layer4_port(uint8 lchip, uint8 index, ctc_pdu_l3pdu_key_t* p_entry)
{
    int32  step = 0;
    uint32 cmd = 0;

    uint32 dest_port = 0;
    uint32 is_tcp_mask = 0;
    uint32 is_tcp_value = 0;
    uint32 is_udp_mask = 0;
    uint32 is_udp_value = 0;
    IpeException3Cam2_m cam;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "dest port:%d  is tcp value:%d is udp value:%d\n",
                       dest_port, p_entry->l3pdu_by_port.is_tcp, p_entry->l3pdu_by_port.is_udp);

    if (index >= MCHIP_CAP(SYS_CAP_L3PDU_BASED_PORT))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_entry->l3pdu_by_port.is_tcp && p_entry->l3pdu_by_port.is_udp)
    {
        return CTC_E_INVALID_PARAM;
    }

    dest_port = p_entry->l3pdu_by_port.dest_port;
    is_tcp_value = p_entry->l3pdu_by_port.is_tcp ? 1 : 0;
    is_udp_value = p_entry->l3pdu_by_port.is_udp ? 1 : 0;

    if (is_tcp_value || is_udp_value)
    {
        is_tcp_mask = p_entry->l3pdu_by_port.is_tcp ? 1 : 0;
        is_udp_mask = p_entry->l3pdu_by_port.is_udp ? 1 : 0;
    }
    else
    {
        is_tcp_mask = 1;
        is_udp_mask = 1;
    }

    step = IpeException3Cam2_array_1_cam2EntryValid_f - IpeException3Cam2_array_0_cam2EntryValid_f;

    sal_memset(&cam, 0, sizeof(IpeException3Cam2_m));
    cmd = DRV_IOR(IpeException3Cam2_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    DRV_SET_FIELD_V(lchip, IpeException3Cam2_t, IpeException3Cam2_array_0_destPort_f + (index * step), &cam, dest_port);
    DRV_SET_FIELD_V(lchip, IpeException3Cam2_t, IpeException3Cam2_array_0_isTcpMask_f + (index * step), &cam, is_tcp_mask);
    DRV_SET_FIELD_V(lchip, IpeException3Cam2_t, IpeException3Cam2_array_0_isTcpValue_f + (index * step), &cam, is_tcp_value);
    DRV_SET_FIELD_V(lchip, IpeException3Cam2_t, IpeException3Cam2_array_0_isUdpMask_f + (index * step), &cam, is_udp_mask);
    DRV_SET_FIELD_V(lchip, IpeException3Cam2_t, IpeException3Cam2_array_0_isUdpValue_f + (index * step), &cam, is_udp_value);

    cmd = DRV_IOW(IpeException3Cam2_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_get_classified_key_by_layer4_port(uint8 lchip, uint8 index, ctc_pdu_l3pdu_key_t* p_entry)
{
    int32  step = 0;

    uint32 cmd = 0;
    uint32 dest_port = 0;
    IpeException3Cam2_m cam;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    if (index >= MCHIP_CAP(SYS_CAP_L3PDU_BASED_PORT))
    {
        return CTC_E_INVALID_PARAM;
    }

    step = IpeException3Cam2_array_1_cam2EntryValid_f - IpeException3Cam2_array_0_cam2EntryValid_f;

    sal_memset(&cam, 0, sizeof(IpeException3Cam2_m));
    cmd = DRV_IOR(IpeException3Cam2_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    p_entry->l3pdu_by_port.dest_port = DRV_GET_FIELD_V(lchip, IpeException3Cam2_t, IpeException3Cam2_array_0_destPort_f + (index * step), &cam);
    p_entry->l3pdu_by_port.is_tcp = DRV_GET_FIELD_V(lchip, IpeException3Cam2_t, IpeException3Cam2_array_0_isTcpValue_f + (index * step), &cam);
    p_entry->l3pdu_by_port.is_udp = DRV_GET_FIELD_V(lchip, IpeException3Cam2_t, IpeException3Cam2_array_0_isUdpValue_f + (index * step), &cam);

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "dest port:%d  is tcp value:%d is udp value:%d\n",
                     dest_port, p_entry->l3pdu_by_port.is_tcp, p_entry->l3pdu_by_port.is_udp);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_classify_l3pdu_by_layer3_ipda(uint8 lchip, uint8 index, ctc_pdu_l3pdu_key_t* p_entry)
{
    int32  step = 0;
    uint32 cmd = 0;

    uint32 is_ipv4_value = 0;
    uint32 is_ipv6_value = 0;
    IpeException3Cam3_m cam;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "is ipv4 value:%d, is ipv6 value:%d, ipda value:0x%X.\n",
                     p_entry->l3pdu_by_ipda.is_ipv4, p_entry->l3pdu_by_ipda.is_ipv6, p_entry->l3pdu_by_ipda.ipda);

    if (index >= MCHIP_CAP(SYS_CAP_L3PDU_BASED_IPDA))
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_entry->l3pdu_by_ipda.ipda > 0xFFF)
    {
        return CTC_E_INVALID_PARAM;
    }

    is_ipv4_value = p_entry->l3pdu_by_ipda.is_ipv4 ? 1 : 0;
    is_ipv6_value = p_entry->l3pdu_by_ipda.is_ipv6 ? 1 : 0;

    sal_memset(&cam, 0, sizeof(IpeException3Cam3_m));
    cmd = DRV_IOR(IpeException3Cam3_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    step = IpeException3Cam3_array_1_cam3EntryValid_f - IpeException3Cam3_array_0_cam3EntryValid_f;

    DRV_SET_FIELD_V(lchip, IpeException3Cam3_t, IpeException3Cam3_array_0_isV4Mask_f + (index * step), &cam, 1);
    DRV_SET_FIELD_V(lchip, IpeException3Cam3_t, IpeException3Cam3_array_0_isV4_f + (index * step), &cam, is_ipv4_value);
    DRV_SET_FIELD_V(lchip, IpeException3Cam3_t, IpeException3Cam3_array_0_isV6Mask_f + (index * step), &cam, 1);
    DRV_SET_FIELD_V(lchip, IpeException3Cam3_t, IpeException3Cam3_array_0_isV6_f + (index * step), &cam, is_ipv6_value);
    DRV_SET_FIELD_V(lchip, IpeException3Cam3_t, IpeException3Cam3_array_0_addrHigh_f + (index * step), &cam, (p_entry->l3pdu_by_ipda.ipda >> 8) & 0xF);
    DRV_SET_FIELD_V(lchip, IpeException3Cam3_t, IpeException3Cam3_array_0_addrHighMask_f + (index * step), &cam, 0xF);
    DRV_SET_FIELD_V(lchip, IpeException3Cam3_t, IpeException3Cam3_array_0_addrLow_f + (index * step), &cam, p_entry->l3pdu_by_ipda.ipda & 0xFF);
    DRV_SET_FIELD_V(lchip, IpeException3Cam3_t, IpeException3Cam3_array_0_addrLowMask_f + (index * step), &cam, 0xFF);

    cmd = DRV_IOW(IpeException3Cam3_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_get_l3pdu_by_layer3_ipda(uint8 lchip, uint8 index, ctc_pdu_l3pdu_key_t* p_entry)
{
    int32  step = 0;
    uint32 cmd = 0;

    IpeException3Cam3_m cam;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "is ipv4 value:%d, is ipv6 value:%d, ipda value:0x%X.\n",
                     p_entry->l3pdu_by_ipda.is_ipv4, p_entry->l3pdu_by_ipda.is_ipv6, p_entry->l3pdu_by_ipda.ipda);

    if (index >= MCHIP_CAP(SYS_CAP_L3PDU_BASED_IPDA))
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&cam, 0, sizeof(IpeException3Cam3_m));
    cmd = DRV_IOR(IpeException3Cam3_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    step = IpeException3Cam3_array_1_cam3EntryValid_f - IpeException3Cam3_array_0_cam3EntryValid_f;
    p_entry->l3pdu_by_ipda.is_ipv4 = DRV_GET_FIELD_V(lchip, IpeException3Cam3_t, IpeException3Cam3_array_0_isV4_f + (index * step), &cam);
    p_entry->l3pdu_by_ipda.is_ipv6 = DRV_GET_FIELD_V(lchip, IpeException3Cam3_t, IpeException3Cam3_array_0_isV6_f + (index * step), &cam);
    p_entry->l3pdu_by_ipda.ipda = DRV_GET_FIELD_V(lchip, IpeException3Cam3_t, IpeException3Cam3_array_0_addrHigh_f + (index * step), &cam) << 8;
    p_entry->l3pdu_by_ipda.ipda |= DRV_GET_FIELD_V(lchip, IpeException3Cam3_t, IpeException3Cam3_array_0_addrLow_f + (index * step), &cam);

    cmd = DRV_IOW(IpeException3Cam3_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));
    return CTC_E_NONE;
}

/**
 @brief  Classify layer3 pdu based on layer3 header protocol, layer4 dest port
*/
int32
sys_usw_l3pdu_classify_l3pdu(uint8 lchip, ctc_pdu_l3pdu_type_t l3pdu_type, uint8 index,
                                    ctc_pdu_l3pdu_key_t* p_key)
{
    CTC_PTR_VALID_CHECK(p_key);
    SYS_LCHIP_CHECK_ACTIVE(lchip);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l3pdu type:%d\n", l3pdu_type);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    switch (l3pdu_type)
    {
    case CTC_PDU_L3PDU_TYPE_L3HDR_PROTO:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_classify_l3pdu_by_l3hdr_proto(lchip, index, p_key));
        break;

    case CTC_PDU_L3PDU_TYPE_LAYER3_IPDA:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_classify_l3pdu_by_layer3_ipda(lchip, index, p_key));
        break;

    case CTC_PDU_L3PDU_TYPE_LAYER4_PORT:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_classify_l3pdu_by_layer4_port(lchip, index, p_key));
        break;
    case CTC_PDU_L3PDU_TYPE_CXP:
	   	{
		   IpeException3Ctl_m excep3_ctl;
		   uint32 cmd = 0;
           cmd = DRV_IOR(IpeException3Ctl_t, DRV_ENTRY_FLAG);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &excep3_ctl));
           SetIpeException3Ctl(V, sxpLayer4Port_f,&excep3_ctl,p_key->l3pdu_by_port.dest_port);
		   SetIpeException3Ctl(V, sxpExceptionEn_f,&excep3_ctl,1);
		   SetIpeException3Ctl(V, sxpExceptionSubIndex_f,&excep3_ctl,CTC_L3PDU_ACTION_INDEX_CXP);
           cmd = DRV_IOW(IpeException3Ctl_t, DRV_ENTRY_FLAG);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &excep3_ctl));
		}
        break;
    default:
        return CTC_E_INVALID_PARAM;
        break;
    }

    return CTC_E_NONE;
}

int32
sys_usw_l3pdu_get_classified_key(uint8 lchip, ctc_pdu_l3pdu_type_t l3pdu_type, uint8 index, ctc_pdu_l3pdu_key_t* p_key)
{
    CTC_PTR_VALID_CHECK(p_key);
    SYS_LCHIP_CHECK_ACTIVE(lchip);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l3pdu type:%d\n", l3pdu_type);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    switch (l3pdu_type)
    {
    case CTC_PDU_L3PDU_TYPE_L3HDR_PROTO:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_get_classified_key_by_l3hdr_proto(lchip, index, p_key));
        break;

    case CTC_PDU_L3PDU_TYPE_LAYER3_IPDA:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_get_l3pdu_by_layer3_ipda(lchip, index, p_key));
        break;

    case CTC_PDU_L3PDU_TYPE_LAYER4_PORT:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_get_classified_key_by_layer4_port(lchip, index, p_key));
        break;
	 case CTC_PDU_L3PDU_TYPE_CXP:
	   	{
		   IpeException3Ctl_m excep3_ctl;
		   uint32 cmd = 0;
           cmd = DRV_IOR(IpeException3Ctl_t, DRV_ENTRY_FLAG);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &excep3_ctl));
           p_key->l3pdu_by_port.dest_port = GetIpeException3Ctl(V, sxpLayer4Port_f,&excep3_ctl);
		   p_key->l3pdu_by_port.is_tcp = 1;
           p_key->l3pdu_by_port.is_udp = 0;
		}
        break;

    default:
        return CTC_E_INVALID_PARAM;
        break;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_set_global_action_of_fixed_l3pdu(uint8 lchip, ctc_pdu_l3pdu_type_t l3pdu_type,
                                                       ctc_pdu_global_l3pdu_action_t* p_action)
{
    uint32 cmd = 0;

    IpeException3Ctl_m ctl;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "action index:%d\n", p_action->action_index);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "entry valid:%d\n", p_action->entry_valid);

    sal_memset(&ctl, 0, sizeof(IpeException3Ctl_m));

    cmd = DRV_IOR(IpeException3Ctl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ctl));

    switch (l3pdu_type)
    {
    case CTC_PDU_L3PDU_TYPE_OSPF:
        SetIpeException3Ctl(V, ospfv4ExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, ospfv4ExceptionSubIndex_f,
                            &ctl, p_action->action_index);

        SetIpeException3Ctl(V, ospfv6ExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, ospfv6ExceptionSubIndex_f,
                            &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_PIM:
        SetIpeException3Ctl(V, pimv4ExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, pimv4ExceptionSubIndex_f,
                            &ctl, p_action->action_index);

        SetIpeException3Ctl(V, pimv6ExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, pimv6ExceptionSubIndex_f,
                            &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_VRRP:
        SetIpeException3Ctl(V, vrrpv4ExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, vrrpv4ExceptionSubIndex_f,
                            &ctl, p_action->action_index);
        SetIpeException3Ctl(V, vrrpv6ExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, vrrpv6ExceptionSubIndex_f,
                            &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_RSVP:
        SetIpeException3Ctl(V, rsvpExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, rsvpExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_RIP:
        SetIpeException3Ctl(V, ripExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, ripExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_BGP:
        SetIpeException3Ctl(V, bgpv4ExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, bgpv4ExceptionSubIndex_f,
                            &ctl, p_action->action_index);
        SetIpeException3Ctl(V, bgpv6ExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, bgpv6ExceptionSubIndex_f,
                            &ctl, p_action->action_index);
        break;
    case CTC_PDU_L3PDU_TYPE_IPV4BGP:
        SetIpeException3Ctl(V, bgpv4ExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, bgpv4ExceptionSubIndex_f,
                            &ctl, p_action->action_index);
        break;
    case CTC_PDU_L3PDU_TYPE_IPV6BGP:
        SetIpeException3Ctl(V, bgpv6ExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, bgpv6ExceptionSubIndex_f,
                            &ctl, p_action->action_index);
        break;
    case CTC_PDU_L3PDU_TYPE_MSDP:
        SetIpeException3Ctl(V, msdpExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, msdpExceptionSubIndex_f,
                            &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_LDP:
        SetIpeException3Ctl(V, ldpExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, ldpExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_IPV4OSPF:
        SetIpeException3Ctl(V, ospfv4ExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, ospfv4ExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_IPV6OSPF:
        SetIpeException3Ctl(V, ospfv6ExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, ospfv6ExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_IPV4PIM:
        SetIpeException3Ctl(V, pimv4ExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, pimv4ExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_IPV6PIM:
        SetIpeException3Ctl(V, pimv6ExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, pimv6ExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_IPV4VRRP:
        SetIpeException3Ctl(V, vrrpv4ExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, vrrpv4ExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_IPV6VRRP:
        SetIpeException3Ctl(V, vrrpv6ExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, vrrpv6ExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_RS:
        SetIpeException3Ctl(V, icmpv6RsExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, icmpv6RsExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_RA:
        SetIpeException3Ctl(V, icmpv6RaExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, icmpv6RaExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_NS:
        SetIpeException3Ctl(V, icmpv6NsExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, icmpv6NsExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_NA:
        SetIpeException3Ctl(V, icmpv6NaExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, icmpv6NaExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_REDIRECT:
        SetIpeException3Ctl(V, icmpv6RedirectExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, icmpv6RedirectExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_RIPNG:
        SetIpeException3Ctl(V, ripngExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, ripngExceptionSubIndex_f, &ctl, p_action->action_index);
        break;
     case CTC_PDU_L3PDU_TYPE_CXP:
        SetIpeException3Ctl(V, sxpExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, sxpExceptionSubIndex_f, &ctl, p_action->action_index);
        break;

    case CTC_PDU_L3PDU_TYPE_ISIS:
        SYS_CHIP_SUPPORT_CHECK(lchip, DRV_DUET2);
        SetIpeException3Ctl(V, isisExceptionEn14_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, isisExceptionEn15_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, isisExceptionSubIndex_f, &ctl, p_action->action_index);
        break;
    case CTC_PDU_L3PDU_TYPE_IGMP_QUERY:
        SYS_CHIP_SUPPORT_CHECK(lchip, DRV_DUET2);
        SetIpeException3Ctl(V, igmpQueryExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, igmpQueryExceptionSubIndex_f, &ctl, p_action->action_index);
        break;
    case CTC_PDU_L3PDU_TYPE_IGMP_REPORT_LEAVE:
        SYS_CHIP_SUPPORT_CHECK(lchip, DRV_DUET2);
        SetIpeException3Ctl(V, igmpRepLevExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, igmpRepLevExceptionSubIndex_f, &ctl, p_action->action_index);
        break;
    case CTC_PDU_L3PDU_TYPE_IGMP_UNKNOWN:
        SYS_CHIP_SUPPORT_CHECK(lchip, DRV_DUET2);
        SetIpeException3Ctl(V, igmpUnknownExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, igmpUnknownExceptionSubIndex_f, &ctl, p_action->action_index);
        break;
    case CTC_PDU_L3PDU_TYPE_MLD_QUERY:
        SYS_CHIP_SUPPORT_CHECK(lchip, DRV_DUET2);
        SetIpeException3Ctl(V, mldQueryExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, mldQueryExceptionSubIndex_f, &ctl, p_action->action_index);
        break;
    case CTC_PDU_L3PDU_TYPE_MLD_REPORT_DONE:
        SYS_CHIP_SUPPORT_CHECK(lchip, DRV_DUET2);
        SetIpeException3Ctl(V, mldRepDoneExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, mldRepAndDoneExceptionSubIndex_f, &ctl, p_action->action_index);
        break;
    case CTC_PDU_L3PDU_TYPE_MLD_UNKNOWN:
        SYS_CHIP_SUPPORT_CHECK(lchip, DRV_DUET2);
        SetIpeException3Ctl(V, mldUnknownExceptionEn_f, &ctl, p_action->entry_valid);
        SetIpeException3Ctl(V, mldUnknownExceptionSubIndex_f, &ctl, p_action->action_index);
        break;
    
    default:
        return CTC_E_INVALID_PARAM;
        break;
    }

    cmd = DRV_IOW(IpeException3Ctl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_get_global_action_of_fixed_l3pdu(uint8 lchip, ctc_pdu_l3pdu_type_t l3pdu_type,
                                                      ctc_pdu_global_l3pdu_action_t* p_action)
{
    uint32 cmd = 0;
    uint32 entry_valid = 0;
    uint32 exception_sub_index = 0;
    IpeException3Ctl_m ctl;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&ctl, 0, sizeof(IpeException3Ctl_m));
    cmd = DRV_IOR(IpeException3Ctl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ctl));

    switch (l3pdu_type)
    {
    case CTC_PDU_L3PDU_TYPE_OSPF:
        GetIpeException3Ctl(A, ospfv4ExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, ospfv4ExceptionSubIndex_f, &ctl, &exception_sub_index);
        if (!entry_valid)
        {
            break;
        }
        GetIpeException3Ctl(A, ospfv4ExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, ospfv4ExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_PIM:
        GetIpeException3Ctl(A, pimv4ExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, pimv4ExceptionSubIndex_f, &ctl, &exception_sub_index);
        if (!entry_valid)
        {
            break;
        }
        GetIpeException3Ctl(A, pimv6ExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, pimv6ExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_VRRP:
        GetIpeException3Ctl(A, vrrpv4ExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, vrrpv4ExceptionSubIndex_f, &ctl, &exception_sub_index);
        if (!entry_valid)
        {
            break;
        }
        GetIpeException3Ctl(A, vrrpv6ExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, vrrpv6ExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_RSVP:
        GetIpeException3Ctl(A, rsvpExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, rsvpExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_RIP:
        GetIpeException3Ctl(A, ripExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, ripExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_BGP:
        GetIpeException3Ctl(A, bgpv4ExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, bgpv4ExceptionSubIndex_f,
                            &ctl, &exception_sub_index);

        break;
   case CTC_PDU_L3PDU_TYPE_IPV4BGP:
        GetIpeException3Ctl(A, bgpv4ExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, bgpv4ExceptionSubIndex_f,
                            &ctl, &exception_sub_index);

        break;
   case CTC_PDU_L3PDU_TYPE_IPV6BGP:
        GetIpeException3Ctl(A, bgpv6ExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, bgpv6ExceptionSubIndex_f,
                            &ctl, &exception_sub_index);

        break;
    case CTC_PDU_L3PDU_TYPE_MSDP:
        GetIpeException3Ctl(A, msdpExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, msdpExceptionSubIndex_f,
                            &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_LDP:
        GetIpeException3Ctl(A, ldpExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, ldpExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_IPV4OSPF:
        GetIpeException3Ctl(A, ospfv4ExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, ospfv4ExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_IPV6OSPF:
        GetIpeException3Ctl(A, ospfv6ExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, ospfv6ExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_IPV4PIM:
        GetIpeException3Ctl(A, pimv4ExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, pimv4ExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_IPV6PIM:
        GetIpeException3Ctl(A, pimv6ExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, pimv6ExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_IPV4VRRP:
        GetIpeException3Ctl(A, vrrpv4ExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, vrrpv4ExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_IPV6VRRP:
        GetIpeException3Ctl(A, vrrpv6ExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, vrrpv6ExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_RS:
        GetIpeException3Ctl(A, icmpv6RsExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, icmpv6RsExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_RA:
        GetIpeException3Ctl(A, icmpv6RaExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, icmpv6RaExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_NS:
        GetIpeException3Ctl(A, icmpv6NsExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, icmpv6NsExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_NA:
        GetIpeException3Ctl(A, icmpv6NaExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, icmpv6NaExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_REDIRECT:
        GetIpeException3Ctl(A, icmpv6RedirectExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, icmpv6RedirectExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_RIPNG:
        GetIpeException3Ctl(A, ripngExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, ripngExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;
    case CTC_PDU_L3PDU_TYPE_CXP:
        GetIpeException3Ctl(A, sxpExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, sxpExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;

    case CTC_PDU_L3PDU_TYPE_ISIS:
        SYS_CHIP_SUPPORT_CHECK(lchip, DRV_DUET2);
        GetIpeException3Ctl(A, isisExceptionEn15_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, isisExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;
    case CTC_PDU_L3PDU_TYPE_IGMP_QUERY:
        SYS_CHIP_SUPPORT_CHECK(lchip, DRV_DUET2);
        GetIpeException3Ctl(A, igmpQueryExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, igmpQueryExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;
    case CTC_PDU_L3PDU_TYPE_IGMP_REPORT_LEAVE:
        SYS_CHIP_SUPPORT_CHECK(lchip, DRV_DUET2);
        GetIpeException3Ctl(A, igmpRepLevExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, igmpRepLevExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;
    case CTC_PDU_L3PDU_TYPE_IGMP_UNKNOWN:
        SYS_CHIP_SUPPORT_CHECK(lchip, DRV_DUET2);
        GetIpeException3Ctl(A, igmpUnknownExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, igmpUnknownExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;
    case CTC_PDU_L3PDU_TYPE_MLD_QUERY:
        SYS_CHIP_SUPPORT_CHECK(lchip, DRV_DUET2);
        GetIpeException3Ctl(A, mldQueryExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, mldQueryExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;
    case CTC_PDU_L3PDU_TYPE_MLD_REPORT_DONE:
        SYS_CHIP_SUPPORT_CHECK(lchip, DRV_DUET2);
        GetIpeException3Ctl(A, mldRepDoneExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, mldRepAndDoneExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;
    case CTC_PDU_L3PDU_TYPE_MLD_UNKNOWN:
        SYS_CHIP_SUPPORT_CHECK(lchip, DRV_DUET2);
        GetIpeException3Ctl(A, mldUnknownExceptionEn_f, &ctl, &entry_valid);
        GetIpeException3Ctl(A, mldUnknownExceptionSubIndex_f, &ctl, &exception_sub_index);
        break;
    
    default:
        break;
    }

    p_action->entry_valid = entry_valid;
    p_action->action_index = exception_sub_index;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "action index:%d\n", p_action->action_index);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "entry valid:%d\n", p_action->entry_valid);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_set_global_action_by_l3hdr_proto(uint8 lchip, uint8 index, ctc_pdu_global_l3pdu_action_t* p_action)
{
    int32  step = 0;
    uint32 cmd = 0;

    uint32 entry_valid = 0;
    uint32 exception_sub_index = 0;
    IpeException3Cam_m cam;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d entry valid:%d  action index:%d\n", index, p_action->entry_valid, p_action->action_index);

    if (index >= MCHIP_CAP(SYS_CAP_L3PDU_BASED_L3HDR_PROTO))
    {
        return CTC_E_INVALID_PARAM;
    }

    entry_valid = p_action->entry_valid ? 1 : 0;
    exception_sub_index = p_action->action_index;

    sal_memset(&cam, 0, sizeof(IpeException3Cam_m));
    cmd = DRV_IOR(IpeException3Cam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    step = IpeException3Cam_array_1_camEntryValid_f - IpeException3Cam_array_0_camEntryValid_f;
    DRV_SET_FIELD_V(lchip, IpeException3Cam_t, IpeException3Cam_array_0_camEntryValid_f + (index * step), &cam, entry_valid);
    DRV_SET_FIELD_V(lchip, IpeException3Cam_t, IpeException3Cam_array_0_camExceptionSubIndex_f + (index * step), &cam, exception_sub_index);

    cmd = DRV_IOW(IpeException3Cam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_get_global_action_by_l3hdr_proto(uint8 lchip, uint8 index, ctc_pdu_global_l3pdu_action_t* p_action)
{
    int32  step = 0;

    uint32 cmd = 0;
    uint32 entry_valid = 0;
    uint32 exception_sub_index = 0;
    IpeException3Cam_m cam;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (index >= MCHIP_CAP(SYS_CAP_L3PDU_BASED_L3HDR_PROTO))
    {
        return CTC_E_INVALID_PARAM;
    }

    step = IpeException3Cam_array_1_camEntryValid_f - IpeException3Cam_array_0_camEntryValid_f;
    sal_memset(&cam, 0, sizeof(IpeException3Cam_m));
    cmd = DRV_IOR(IpeException3Cam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    DRV_GET_FIELD_A(lchip, IpeException3Cam_t, IpeException3Cam_array_0_camEntryValid_f + (index * step), &cam, &entry_valid);
    DRV_GET_FIELD_A(lchip, IpeException3Cam_t, IpeException3Cam_array_0_camExceptionSubIndex_f + (index * step), &cam, &exception_sub_index);

    p_action->entry_valid = entry_valid;
    p_action->action_index = exception_sub_index;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "index:%d entry valid:%d  action index:%d\n", index, p_action->entry_valid, p_action->action_index);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_set_global_action_by_layer4_port(uint8 lchip, uint8 index, ctc_pdu_global_l3pdu_action_t* p_action)
{
    int32  step = 0;
    uint32 cmd = 0;

    IpeException3Cam2_m cam;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d entry valid:%d  action index:%d\n", index, p_action->entry_valid, p_action->action_index);

    if (index >= MCHIP_CAP(SYS_CAP_L3PDU_BASED_PORT))
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&cam, 0, sizeof(IpeException3Cam2_m));
    cmd = DRV_IOR(IpeException3Cam2_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    step = IpeException3Cam2_array_1_cam2EntryValid_f - IpeException3Cam2_array_0_cam2EntryValid_f;
    DRV_SET_FIELD_V(lchip, IpeException3Cam2_t, IpeException3Cam2_array_0_cam2EntryValid_f + (index * step), &cam, (p_action->entry_valid ? 1 : 0));
    DRV_SET_FIELD_V(lchip, IpeException3Cam2_t, IpeException3Cam2_array_0_cam2ExceptionSubIndex_f + (index * step), &cam, p_action->action_index);

    cmd = DRV_IOW(IpeException3Cam2_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_get_global_action_by_layer4_port(uint8 lchip, uint8 index, ctc_pdu_global_l3pdu_action_t* p_action)
{
    int32  step = 0;
    uint32 cmd = 0;

    uint32 entry_valid = 0;
    uint32 exception_sub_index = 0;
    IpeException3Cam2_m cam;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (index >= MCHIP_CAP(SYS_CAP_L3PDU_BASED_PORT))
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&cam, 0, sizeof(IpeException3Cam2_m));
    cmd = DRV_IOR(IpeException3Cam2_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    step = IpeException3Cam2_array_1_cam2EntryValid_f - IpeException3Cam2_array_0_cam2EntryValid_f;
    DRV_GET_FIELD_A(lchip, IpeException3Cam2_t, IpeException3Cam2_array_0_cam2EntryValid_f + (index * step), &cam, &entry_valid);
    DRV_GET_FIELD_A(lchip, IpeException3Cam2_t, IpeException3Cam2_array_0_cam2ExceptionSubIndex_f + (index * step), &cam, &exception_sub_index);

    p_action->entry_valid = entry_valid;
    p_action->action_index = exception_sub_index;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "index:%d entry valid:%d  action index:%d\n",
                     index, p_action->entry_valid, p_action->action_index);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_set_global_action_by_layer3_ipda(uint8 lchip, uint8 index, ctc_pdu_global_l3pdu_action_t* p_action)
{
    uint32 cmd = 0;

    int32  step = IpeException3Cam3_array_1_cam3EntryValid_f - IpeException3Cam3_array_0_cam3EntryValid_f;
    uint32 entry_valid_field_id = IpeException3Cam3_array_0_cam3EntryValid_f;
    uint32 exception_sub_index_field_id = IpeException3Cam3_array_0_cam3ExceptionSubIndex_f;
    IpeException3Cam3_m cam;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d entry valid:%d  action index:%d\n", index, p_action->entry_valid, p_action->action_index);

    if (index >= MCHIP_CAP(SYS_CAP_L3PDU_BASED_IPDA))
    {
        return CTC_E_INVALID_PARAM;
    }

    entry_valid_field_id += step * index;
    exception_sub_index_field_id += step * index;

    sal_memset(&cam, 0, sizeof(IpeException3Cam3_m));
    cmd = DRV_IOR(IpeException3Cam3_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    DRV_SET_FIELD_V(lchip, IpeException3Cam3_t, entry_valid_field_id, &cam, (p_action->entry_valid ? 1 : 0));
    DRV_SET_FIELD_V(lchip, IpeException3Cam3_t, exception_sub_index_field_id, &cam, p_action->action_index);

    cmd = DRV_IOW(IpeException3Cam3_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_get_global_action_by_layer3_ipda(uint8 lchip, uint8 index, ctc_pdu_global_l3pdu_action_t* p_action)
{
    uint32 cmd = 0;

    int32  step = IpeException3Cam3_array_1_cam3EntryValid_f - IpeException3Cam3_array_0_cam3EntryValid_f;
    uint32 entry_valid = 0;
    uint32 exception_sub_index = 0;
    uint32 entry_valid_field_id = IpeException3Cam3_array_0_cam3EntryValid_f;
    uint32 exception_sub_index_field_id = IpeException3Cam3_array_0_cam3ExceptionSubIndex_f;
    IpeException3Cam3_m cam;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (index >= MCHIP_CAP(SYS_CAP_L3PDU_BASED_IPDA))
    {
        return CTC_E_INVALID_PARAM;
    }

    entry_valid_field_id += step * index;
    exception_sub_index_field_id += step * index;

    sal_memset(&cam, 0, sizeof(IpeException3Cam3_m));
    cmd = DRV_IOR(IpeException3Cam3_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam));

    DRV_GET_FIELD_A(lchip, IpeException3Cam3_t, entry_valid_field_id, &cam, &entry_valid);
    DRV_GET_FIELD_A(lchip, IpeException3Cam3_t, exception_sub_index_field_id, &cam, &exception_sub_index);

    p_action->entry_valid = entry_valid;
    p_action->action_index = exception_sub_index;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "index:%d entry valid:%d  action index:%d\n",
                     index, p_action->entry_valid, p_action->action_index);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_set_global_action_by_layer4_type(uint8 lchip, uint8 index, ctc_pdu_global_l3pdu_action_t* p_action)
{
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

}

STATIC int32
_sys_usw_l3pdu_get_global_action_by_layer4_type(uint8 lchip, uint8 index, ctc_pdu_global_l3pdu_action_t* p_action)
{
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
			return CTC_E_NOT_SUPPORT;

}

STATIC int32
_sys_usw_l3pdu_set_global_action_misc(uint8 lchip, ctc_pdu_l3pdu_type_t l3pdu_type, ctc_pdu_global_l3pdu_action_t* p_action)
{
    uint32 cmd = 0;
    IpeIntfMapperCtl_m ipe_intf_mapper_ctl;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&ipe_intf_mapper_ctl, 0, sizeof(IpeIntfMapperCtl_m));
    cmd = DRV_IOR(IpeIntfMapperCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_intf_mapper_ctl));

    if (CTC_PDU_L3PDU_TYPE_ARP == l3pdu_type)
    {
        SetIpeIntfMapperCtl(V, arpCheckExceptionEn_f, &ipe_intf_mapper_ctl, p_action->entry_valid);
        SetIpeIntfMapperCtl(V, arpExceptionSubIndex_f, &ipe_intf_mapper_ctl, p_action->action_index);
    }
    else if (CTC_PDU_L3PDU_TYPE_DHCP == l3pdu_type)
    {
        SetIpeIntfMapperCtl(V, dhcpUnicastExceptionDisable_f, &ipe_intf_mapper_ctl, p_action->entry_valid ? 0 : 1);
        SetIpeIntfMapperCtl(V, dhcpExceptionSubIndex_f, &ipe_intf_mapper_ctl, p_action->action_index);
    }

    cmd = DRV_IOW(IpeIntfMapperCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_intf_mapper_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_get_global_action_misc(uint8 lchip, ctc_pdu_l3pdu_type_t l3pdu_type, ctc_pdu_global_l3pdu_action_t* p_action)
{
    uint32 cmd = 0;
    IpeIntfMapperCtl_m ipe_intf_mapper_ctl;

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&ipe_intf_mapper_ctl, 0, sizeof(IpeIntfMapperCtl_m));

    cmd = DRV_IOR(IpeIntfMapperCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_intf_mapper_ctl));

    if (CTC_PDU_L3PDU_TYPE_ARP == l3pdu_type)
    {
        p_action->entry_valid = GetIpeIntfMapperCtl(V, arpCheckExceptionEn_f, &ipe_intf_mapper_ctl);
        p_action->action_index = GetIpeIntfMapperCtl(V, arpExceptionSubIndex_f, &ipe_intf_mapper_ctl);
    }
    else if (CTC_PDU_L3PDU_TYPE_DHCP == l3pdu_type)
    {
        p_action->entry_valid = GetIpeIntfMapperCtl(V, dhcpUnicastExceptionDisable_f, &ipe_intf_mapper_ctl) ? 0 : 1;
        p_action->action_index = GetIpeIntfMapperCtl(V, dhcpExceptionSubIndex_f, &ipe_intf_mapper_ctl);
    }

    return CTC_E_NONE;
}

/**
 @brief  Set layer3 pdu global property
*/
int32
sys_usw_l3pdu_set_global_action(uint8 lchip, ctc_pdu_l3pdu_type_t l3pdu_type, uint8 index, ctc_pdu_global_l3pdu_action_t* p_action)
{
    CTC_PTR_VALID_CHECK(p_action);
    SYS_LCHIP_CHECK_ACTIVE(lchip);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l3pdu type:%d\n", l3pdu_type);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);

    if (p_action->action_index > MCHIP_CAP(SYS_CAP_L3PDU_ACTION_INDEX))
    {
        return CTC_E_INVALID_PARAM;
    }

    switch (l3pdu_type)
    {
    case CTC_PDU_L3PDU_TYPE_OSPF:
    case CTC_PDU_L3PDU_TYPE_PIM:
    case CTC_PDU_L3PDU_TYPE_VRRP:
    case CTC_PDU_L3PDU_TYPE_RSVP:
    case CTC_PDU_L3PDU_TYPE_RIP:
    case CTC_PDU_L3PDU_TYPE_BGP:
    case CTC_PDU_L3PDU_TYPE_IPV4BGP:
    case CTC_PDU_L3PDU_TYPE_IPV6BGP:
    case CTC_PDU_L3PDU_TYPE_MSDP:
    case CTC_PDU_L3PDU_TYPE_LDP:
    case CTC_PDU_L3PDU_TYPE_IPV4OSPF:
    case CTC_PDU_L3PDU_TYPE_IPV6OSPF:
    case CTC_PDU_L3PDU_TYPE_IPV4PIM:
    case CTC_PDU_L3PDU_TYPE_IPV6PIM:
    case CTC_PDU_L3PDU_TYPE_IPV4VRRP:
    case CTC_PDU_L3PDU_TYPE_IPV6VRRP:
    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_RS:
    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_RA:
    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_NS:
    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_NA:
    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_REDIRECT:
    case CTC_PDU_L3PDU_TYPE_RIPNG:
	case CTC_PDU_L3PDU_TYPE_CXP:
    case CTC_PDU_L3PDU_TYPE_ISIS:
    case CTC_PDU_L3PDU_TYPE_IGMP_QUERY:
    case CTC_PDU_L3PDU_TYPE_IGMP_REPORT_LEAVE:
    case CTC_PDU_L3PDU_TYPE_IGMP_UNKNOWN:
    case CTC_PDU_L3PDU_TYPE_MLD_QUERY:
    case CTC_PDU_L3PDU_TYPE_MLD_REPORT_DONE:
    case CTC_PDU_L3PDU_TYPE_MLD_UNKNOWN:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_set_global_action_of_fixed_l3pdu(lchip, l3pdu_type, p_action));
        break;

    case CTC_PDU_L3PDU_TYPE_L3HDR_PROTO:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_set_global_action_by_l3hdr_proto(lchip, index, p_action));
        break;

    case CTC_PDU_L3PDU_TYPE_LAYER3_IPDA:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_set_global_action_by_layer3_ipda(lchip, index, p_action));
        break;

    case CTC_PDU_L3PDU_TYPE_LAYER4_PORT:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_set_global_action_by_layer4_port(lchip, index, p_action));
        break;

    case CTC_PDU_L3PDU_TYPE_LAYER4_TYPE:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_set_global_action_by_layer4_type(lchip, index, p_action));
        break;

    case CTC_PDU_L3PDU_TYPE_ARP:
    case CTC_PDU_L3PDU_TYPE_DHCP:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_set_global_action_misc(lchip, l3pdu_type, p_action));
        break;

    default:
        break;
    }

    return CTC_E_NONE;
}

int32
sys_usw_l3pdu_get_global_action(uint8 lchip, ctc_pdu_l3pdu_type_t l3pdu_type, uint8 index, ctc_pdu_global_l3pdu_action_t* p_action)
{
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "l3pdu type:%d\n", l3pdu_type);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "index:%d\n", index);
    SYS_LCHIP_CHECK_ACTIVE(lchip);
    switch (l3pdu_type)
    {
    case CTC_PDU_L3PDU_TYPE_OSPF:
    case CTC_PDU_L3PDU_TYPE_PIM:
    case CTC_PDU_L3PDU_TYPE_VRRP:
    case CTC_PDU_L3PDU_TYPE_RSVP:
    case CTC_PDU_L3PDU_TYPE_RIP:
    case CTC_PDU_L3PDU_TYPE_BGP:
    case CTC_PDU_L3PDU_TYPE_MSDP:
    case CTC_PDU_L3PDU_TYPE_LDP:
    case CTC_PDU_L3PDU_TYPE_IPV4OSPF:
    case CTC_PDU_L3PDU_TYPE_IPV6OSPF:
    case CTC_PDU_L3PDU_TYPE_IPV4PIM:
    case CTC_PDU_L3PDU_TYPE_IPV6PIM:
    case CTC_PDU_L3PDU_TYPE_IPV4VRRP:
    case CTC_PDU_L3PDU_TYPE_IPV6VRRP:
    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_RS:
    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_RA:
    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_NS:
    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_NA:
    case CTC_PDU_L3PDU_TYPE_ICMPIPV6_REDIRECT:
    case CTC_PDU_L3PDU_TYPE_RIPNG:
	case CTC_PDU_L3PDU_TYPE_CXP:
    case CTC_PDU_L3PDU_TYPE_IPV4BGP:
    case CTC_PDU_L3PDU_TYPE_IPV6BGP:
    case CTC_PDU_L3PDU_TYPE_ISIS:
    case CTC_PDU_L3PDU_TYPE_IGMP_QUERY:
    case CTC_PDU_L3PDU_TYPE_IGMP_REPORT_LEAVE:
    case CTC_PDU_L3PDU_TYPE_IGMP_UNKNOWN:
    case CTC_PDU_L3PDU_TYPE_MLD_QUERY:
    case CTC_PDU_L3PDU_TYPE_MLD_REPORT_DONE:
    case CTC_PDU_L3PDU_TYPE_MLD_UNKNOWN:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_get_global_action_of_fixed_l3pdu(lchip, l3pdu_type, p_action));
        break;

    case CTC_PDU_L3PDU_TYPE_L3HDR_PROTO:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_get_global_action_by_l3hdr_proto(lchip, index, p_action));
        break;

    case CTC_PDU_L3PDU_TYPE_LAYER3_IPDA:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_get_global_action_by_layer3_ipda(lchip, index, p_action));
        break;

    case CTC_PDU_L3PDU_TYPE_LAYER4_PORT:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_get_global_action_by_layer4_port(lchip, index, p_action));
        break;

    case CTC_PDU_L3PDU_TYPE_LAYER4_TYPE:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_get_global_action_by_layer4_type(lchip, index, p_action));
        break;

    case CTC_PDU_L3PDU_TYPE_ARP:
    case CTC_PDU_L3PDU_TYPE_DHCP:
        CTC_ERROR_RETURN(_sys_usw_l3pdu_get_global_action_misc(lchip, l3pdu_type, p_action));
        break;

    default:
        break;
    }

    return CTC_E_NONE;
}

/**
 @brief  Per layer3 interface control layer3 pdu action
*/
int32
sys_usw_l3pdu_set_l3if_action(uint8 lchip, uint16 l3ifid, uint8 action_index, ctc_pdu_l3if_l3pdu_action_t action)
{
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_LCHIP_CHECK_ACTIVE(lchip);
    if (action_index > MCHIP_CAP(SYS_CAP_L3PDU_L3IF_ACTION_INDEX))
    {
        return CTC_E_INVALID_PARAM;
    }

    switch (action)
    {
    case CTC_PDU_L3PDU_ACTION_TYPE_FWD:
        CTC_ERROR_RETURN(sys_usw_l3if_set_exception3_en(lchip, l3ifid, action_index, FALSE));
        break;

    case CTC_PDU_L3PDU_ACTION_TYPE_COPY_TO_CPU:
        CTC_ERROR_RETURN(sys_usw_l3if_set_exception3_en(lchip, l3ifid, action_index, TRUE));
        break;

    default:
        break;
    }

    return CTC_E_NONE;
}

int32
sys_usw_l3pdu_get_l3if_action(uint8 lchip, uint16 l3ifid, uint8 action_index, ctc_pdu_l3if_l3pdu_action_t* action)
{
    bool enbale = 0;
    SYS_LCHIP_CHECK_ACTIVE(lchip);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (action_index > MCHIP_CAP(SYS_CAP_L3PDU_L3IF_ACTION_INDEX))
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_l3if_get_exception3_en(lchip, l3ifid,  action_index, &enbale));

    if (enbale == TRUE)
    {
        *action = CTC_PDU_L3PDU_ACTION_TYPE_COPY_TO_CPU;
    }
    else
    {
        *action = CTC_PDU_L3PDU_ACTION_TYPE_FWD;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_exception_enable_init(uint8 lchip)
{
    uint32 cmd = 0;
    IpeBpduEscapeCtl_m ctl;

    sal_memset(&ctl, 0, sizeof(IpeBpduEscapeCtl_m));

    SetIpeBpduEscapeCtl(V, bpduEscapeEn_f, &ctl, 0xF);

    /* BPDU, SLOW_PROTOCOL, EAPOL, LLDP, L2ISIS auto identified */
    SetIpeBpduEscapeCtl(V, bpduExceptionEn_f, &ctl, 1);
    SetIpeBpduEscapeCtl(V, bpduExceptionSubIndex_f,
                        &ctl, CTC_L2PDU_ACTION_INDEX_BPDU);
    SetIpeBpduEscapeCtl(V, slowProtocolExceptionEn_f, &ctl, 1);
    SetIpeBpduEscapeCtl(V, slowProtocolExceptionSubIndex_f,
                        &ctl, CTC_L2PDU_ACTION_INDEX_SLOW_PROTO);
    SetIpeBpduEscapeCtl(V, efmSubIndex_f, &ctl, CTC_L2PDU_ACTION_INDEX_EFM_OAM);
    SetIpeBpduEscapeCtl(V, eapolExceptionEn_f, &ctl, 1);
    SetIpeBpduEscapeCtl(V, eapolExceptionSubIndex_f,
                        &ctl, CTC_L2PDU_ACTION_INDEX_EAPOL);
    SetIpeBpduEscapeCtl(V, lldpExceptionEn_f, &ctl, 1);
    SetIpeBpduEscapeCtl(V, lldpExceptionSubIndex_f,
                        &ctl, CTC_L2PDU_ACTION_INDEX_LLDP);
    SetIpeBpduEscapeCtl(V, l2isisExceptionEn_f, &ctl, 1);
    SetIpeBpduEscapeCtl(V, l2isisExceptionSubIndex_f,
                        &ctl, CTC_L2PDU_ACTION_INDEX_ISIS);

    cmd = DRV_IOW(IpeBpduEscapeCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_exception_enable_init(uint8 lchip)
{
    uint32 cmd = 0;
    IpeException3Ctl_m ctl;

    sal_memset(&ctl, 0, sizeof(IpeException3Ctl_m));

    cmd = DRV_IOR(IpeException3Ctl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ctl));

    SetIpeException3Ctl(V, exceptionCamEn_f, &ctl, 1);
    SetIpeException3Ctl(V, exceptionCamEn2_f, &ctl, 1);
    SetIpeException3Ctl(V, exceptionCamEn3_f, &ctl, 1);

    /* RIP, OSPF, BGP, PIM, VRRP, RSVP, MSDP, LDP auto identified */
    SetIpeException3Ctl(V, ospfv4ExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, ospfv4ExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_OSPF);
    SetIpeException3Ctl(V, ospfv6ExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, ospfv6ExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_OSPF);
    SetIpeException3Ctl(V, pimv4ExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, pimv4ExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_PIM);
    SetIpeException3Ctl(V, pimv6ExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, pimv6ExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_PIM);
    SetIpeException3Ctl(V, vrrpv4ExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, vrrpv4ExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_VRRP);
    SetIpeException3Ctl(V, vrrpv6ExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, vrrpv6ExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_VRRP);
    SetIpeException3Ctl(V, rsvpExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, rsvpExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_RSVP);
    SetIpeException3Ctl(V, ripExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, ripExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_RIP);
    SetIpeException3Ctl(V, bgpv4ExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, bgpv4ExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_BGP);
    SetIpeException3Ctl(V, bgpv6ExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, bgpv6ExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_BGP);
    SetIpeException3Ctl(V, msdpExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, msdpExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_MSDP);
    SetIpeException3Ctl(V, ldpExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, ldpExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_LDP);
    SetIpeException3Ctl(V, icmpv6RsExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, icmpv6RsExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_ICMPV6);
    SetIpeException3Ctl(V, icmpv6RaExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, icmpv6RaExceptionSubIndex_f,
                       &ctl, CTC_L3PDU_ACTION_INDEX_ICMPV6);
    SetIpeException3Ctl(V, icmpv6NsExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, icmpv6NsExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_ICMPV6);
    SetIpeException3Ctl(V, icmpv6NaExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, icmpv6NaExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_ICMPV6);
    SetIpeException3Ctl(V, icmpv6RedirectExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, icmpv6RedirectExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_ICMPV6);
    SetIpeException3Ctl(V, ripngExceptionEn_f, &ctl, 1);
    SetIpeException3Ctl(V, ripngExceptionSubIndex_f,
                        &ctl, CTC_L3PDU_ACTION_INDEX_RIPNG);

    cmd = DRV_IOW(IpeException3Ctl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_cam_init(uint8 lchip)
{
    int32 cmd = 0;

    IpeBpduProtocolEscapeCam_m cam1;
    IpeBpduProtocolEscapeCam2_m cam2;
    IpeBpduProtocolEscapeCam3_m cam3;
    IpeBpduProtocolEscapeCam4_m cam4;
    IpeBpduProtocolEscapeCamResult_m rst1;
    IpeBpduProtocolEscapeCamResult2_m rst2;
    IpeBpduProtocolEscapeCamResult3_m rst3;
    IpeBpduProtocolEscapeCamResult4_m rst4;

    sal_memset(&cam1, 0, sizeof(IpeBpduProtocolEscapeCam_m));
    sal_memset(&cam2, 0, sizeof(IpeBpduProtocolEscapeCam2_m));
    sal_memset(&cam3, 0, sizeof(IpeBpduProtocolEscapeCam3_m));
    sal_memset(&cam4, 0, sizeof(IpeBpduProtocolEscapeCam4_m));

    sal_memset(&rst1, 0, sizeof(IpeBpduProtocolEscapeCamResult_m));
    sal_memset(&rst2, 0, sizeof(IpeBpduProtocolEscapeCamResult2_m));
    sal_memset(&rst3, 0, sizeof(IpeBpduProtocolEscapeCamResult3_m));
    sal_memset(&rst4, 0, sizeof(IpeBpduProtocolEscapeCamResult4_m));

    cmd = DRV_IOW(IpeBpduProtocolEscapeCam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam1));

    cmd = DRV_IOW(IpeBpduProtocolEscapeCamResult_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rst1));

    cmd = DRV_IOW(IpeBpduProtocolEscapeCam2_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam2));

    cmd = DRV_IOW(IpeBpduProtocolEscapeCamResult2_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rst2));

    cmd = DRV_IOW(IpeBpduProtocolEscapeCam3_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam3));

    cmd = DRV_IOW(IpeBpduProtocolEscapeCamResult3_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rst3));

    cmd = DRV_IOW(IpeBpduProtocolEscapeCam4_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam4));

    cmd = DRV_IOW(IpeBpduProtocolEscapeCamResult4_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rst4));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l3pdu_cam_init(uint8 lchip)
{
    int32 cmd ;


    IpeException3Cam_m cam1;
    IpeException3Cam2_m cam2;
    IpeException3Cam3_m cam3;

    sal_memset(&cam1, 0, sizeof(IpeException3Cam_m));
    sal_memset(&cam2, 0, sizeof(IpeException3Cam2_m));
    sal_memset(&cam3, 0, sizeof(IpeException3Cam3_m));

    cmd  = DRV_IOW(IpeException3Cam_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam1));

    cmd  = DRV_IOW(IpeException3Cam2_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam2));

    cmd  = DRV_IOW(IpeException3Cam3_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &cam3));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_l2pdu_init_fixed_port_action(uint8 lchip)
{
    uint8  gchip = 0;
    uint32 i = 0;
    uint16 gport = 0;

    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    for (i = 0; i < SYS_PHY_PORT_NUM_PER_SLICE; i++)
    {
        gport = CTC_MAP_LPORT_TO_GPORT(gchip, SYS_MAP_DRV_LPORT_TO_SYS_LPORT(i));
        sys_usw_l2pdu_set_port_action(lchip, gport, CTC_L2PDU_ACTION_INDEX_BPDU, CTC_PDU_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU);
    }

    return CTC_E_NONE;
}

/**
 @brief init pdu module
*/
int32
sys_usw_pdu_init(uint8 lchip)
{
    CTC_ERROR_RETURN(_sys_usw_l2pdu_exception_enable_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_l2pdu_cam_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_l3pdu_exception_enable_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_l3pdu_cam_init(lchip));

    CTC_ERROR_RETURN(_sys_usw_l2pdu_init_fixed_port_action(lchip));

    return CTC_E_NONE;
}

/**
 @brief deinit pdu module
*/
int32
sys_usw_pdu_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    return CTC_E_NONE;
}

#define __1_SHOW_L2PDU_FUN__

#define ___________PDU_OUTER_FUNCTION___________
int32 _sys_usw_pdu_show_l2pdu_static_cfg(uint8 lchip,uint8 l2pdu_type)
{
  ctc_pdu_global_l2pdu_action_t  action;
  char* column[4] = {0};
  sal_memset(&action, 0, sizeof(ctc_pdu_global_l2pdu_action_t));
  switch(l2pdu_type)
  {
  case CTC_PDU_L2PDU_TYPE_BPDU:
      column[0] = "BPDU"; /*Type*/
      column[1] = "0180.c200.0000";    /*MACDA*/
      column[2] = "-";    /*EthType*/
        break;
  case CTC_PDU_L2PDU_TYPE_SLOW_PROTO:
      column[0] = "Slow Protocol";
      column[1] = "-";
      column[2] = "0x8809";
       break;
  case CTC_PDU_L2PDU_TYPE_EAPOL:
      column[0] = "EAPOL";
      column[1] = "-";
      column[2] = "0x888E";
       break;
  case CTC_PDU_L2PDU_TYPE_LLDP:
      column[0] = "LLDP"; /*Type*/
      column[1] = "-";    /*MACDA*/
      column[2] = "0x88CC";    /*EthType*/
      break;
  case CTC_PDU_L2PDU_TYPE_ISIS:
      column[0] = "ISIS"; /*Type*/
      column[1] = "-";    /*MACDA*/
      column[2] = "0x22F4";    /*EthType*/
      break;
  case CTC_PDU_L2PDU_TYPE_EFM_OAM:
      column[0] = "EFM_OAM"; /*Type*/
      column[1] = "0180.c200.0000";    /*MACDA*/
      column[2] = "0x8809";    /*EthType*/
       break;
  case CTC_PDU_L2PDU_TYPE_FIP:
      column[0] = "FIP"; /*Type*/
      column[1] = "-";    /*MACDA*/
      column[2] = "0x8914";    /*EthType*/
       break;
   default :
       return CTC_E_NONE;
  }
  _sys_usw_l2pdu_get_global_action_of_fixed_l2pdu(lchip,l2pdu_type,&action);
  CTC_DEBUG_OUT(pdu, pdu, PDU_SYS, CTC_DEBUG_LEVEL_DUMP, "%-16s%-17s%-13s%-15d%-5s\n",
     column[0],column[1], column[2], action.action_index,action.entry_valid?"True":"False");
 return CTC_E_NONE;
}

int32 _sys_usw_pdu_show_l2pdu_dynamic_cfg(uint8 lchip, uint8 l2pdu_type)
{
    ctc_pdu_l2pdu_key_t l2pdu_key;
    ctc_pdu_global_l2pdu_action_t pdu_action;
    int32 ret = 0;
    uint8 idx = 0;
    char* column[4] = {0};
    char  str[32] = {0};

    column[0] = "-"; /*Type*/
    column[1] = "-";    /*MACDA*/
    column[2] = "-";    /*EthType*/

    for (idx = 0; idx < 16 ; idx++)
    {
        ret = sys_usw_l2pdu_get_classified_key( lchip,l2pdu_type, idx, &l2pdu_key);
        ret = ret ? ret: sys_usw_l2pdu_get_global_action(lchip,l2pdu_type, idx, &pdu_action);
        if (ret != CTC_E_NONE)
        {
            continue;
        }
        switch(l2pdu_type)
        {
        case CTC_PDU_L2PDU_TYPE_L2HDR_PROTO:
            if (pdu_action.entry_valid)
            {
                sal_sprintf(str, "0x%-4x", l2pdu_key.l2hdr_proto);
                column[2] = str;
                column[0] = "User-defined";
            }
            else
            {
                column[0] = "None-defined";
                column[2] = "x";
            }
            break;
        case CTC_PDU_L2PDU_TYPE_MACDA:

            if (pdu_action.entry_valid)
            {
                column[1] = sys_output_mac(l2pdu_key.l2pdu_by_mac.mac);
                column[0] = "User-defined";
            }
            else
            {
                column[0] = "None-defined";
                column[1] = "xxxx.xxxx.xxxx";
            }
            break;
            break;
        case CTC_PDU_L2PDU_TYPE_MACDA_LOW24:
            if (pdu_action.entry_valid)
            {
                column[1] = sys_output_mac(l2pdu_key.l2pdu_by_mac.mac);
                column[0] = "User-defined";
            }
            else
            {
                column[0] = "None-defined";
                column[1] = "0180.C2xx.xxxx";
            }
            break;
        default :
            return CTC_E_NONE;
        }
        CTC_DEBUG_OUT(pdu, pdu, PDU_SYS, CTC_DEBUG_LEVEL_DUMP, "%-16s%-17s%-13s%-15d%-5s\n",
                      column[0] , column[1], column[2], pdu_action.action_index, pdu_action.entry_valid?"True":"False");
    }
    return CTC_E_NONE;
}
int32
sys_usw_pdu_show_l2pdu_cfg(uint8 lchip)
{
    uint32 idx = 0;

    LCHIP_CHECK(lchip);
    SYS_LCHIP_CHECK_ACTIVE(lchip);
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "L2pdu:\n");
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Note: for MAC address the token \"x\" used as an indication\n");
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "      of a max hex value 0xF that user can configure.\n");
     SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------------------\n");
    CTC_DEBUG_OUT(pdu, pdu, PDU_SYS, CTC_DEBUG_LEVEL_DUMP, "%-16s%-17s%-13s%-15s%-5s\n",
     "Type","MAC-DA", " Ether-type", "Action-index","Valid");

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------------------------\n");

    for (idx = 0; idx < MAX_CTC_PDU_L2PDU_TYPE; idx++)
    {
        if ((CTC_PDU_L2PDU_TYPE_L3TYPE == idx) || (CTC_PDU_L2PDU_TYPE_L2HDR_PROTO == idx)
           || (CTC_PDU_L2PDU_TYPE_MACDA == idx) || (CTC_PDU_L2PDU_TYPE_MACDA_LOW24 == idx)
           )
        {
           continue;
        }
        else
        {
          _sys_usw_pdu_show_l2pdu_static_cfg(lchip,idx);
        }
    }

    for (idx = 0; idx < MAX_CTC_PDU_L2PDU_TYPE; idx++)
    {
        if ((CTC_PDU_L2PDU_TYPE_L3TYPE == idx) || (CTC_PDU_L2PDU_TYPE_L2HDR_PROTO == idx)
           || (CTC_PDU_L2PDU_TYPE_MACDA == idx) || (CTC_PDU_L2PDU_TYPE_MACDA_LOW24 == idx)
           )
        {
          _sys_usw_pdu_show_l2pdu_dynamic_cfg(lchip,idx);
        }
    }
    return CTC_E_NONE;
}
int32 _sys_usw_pdu_show_l3pdu_static_cfg(uint8 lchip,uint8 l3pdu_type)
{
  ctc_pdu_global_l3pdu_action_t  action;
  char* column[5] = {0};
  sal_memset(&action, 0, sizeof(ctc_pdu_global_l3pdu_action_t));
  switch(l3pdu_type)
  {
  case CTC_PDU_L3PDU_TYPE_RSVP:               /**< [GB.GG.D2] Layer3 protocol = 46:  static classify by protocol.*/
      column[0] = "RSVP"; /*Type*/
      column[1] = "-";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "46";   /*Protocol*/
      column[4] = "-";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_RIP:                /**< [GB.GG.D2] UDP:  for GB DestPort = 520 or DestPort = 521:  for GG DestPort = 520:  static classify by upd port.*/
      column[0] = "RIP";
      column[1] = "IPv4";
      column[2] = "-";
      column[3] = "-";
      column[4] = "520";
      break;
  case CTC_PDU_L3PDU_TYPE_MSDP:               /**< [GB.GG.D2] TCP:  DestPort = 639:  static classify by upd port.*/
      column[0] = "MSDP";
      column[1] = "-";
      column[2] = "-";
      column[3] = "-";
      column[4] = "520";
      break;
  case CTC_PDU_L3PDU_TYPE_LDP:                /**< [GB.GG.D2] TCP or UDP:  DestPort = 646:  other ldp destport pdu classify
                                                    by CTC_PDU_L3PDU_TYPE_LAYER4_PORT or forwarding to cpu.*/
      column[0] = "LDP"; /*Type*/
      column[1] = "-";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "-";   /*Protocol*/
      column[4] = "646";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_IPV4OSPF:           /**< [GG.D2] IPv4:  layer3 protocol = 89:  static classify by protocol.*/
      column[0] = "OSPFv4"; /*Type*/
      column[1] = "IPv4";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "89";   /*Protocol*/
      column[4] = "-";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_IPV6OSPF:           /**< [GG.D2] IPv6:  layer3 protocol = 89:  static classify by protocol.*/
      column[0] = "OSPFv6"; /*Type*/
      column[1] = "IPv6";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "89";   /*Protocol*/
      column[4] = "-";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_IPV4PIM:            /**< [GG.D2] IPv4:  layer3 protocol = 103:  static classify by protocol.*/
      column[0] = "PIM"; /*Type*/
      column[1] = "IPv4";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "103";   /*Protocol*/
      column[4] = "-";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_IPV6PIM:            /**< [GG.D2] IPv6:  layer3 protocol = 103:  static classify by protocol.*/
      column[0] = "PIMV6"; /*Type*/
      column[1] = "IPv6";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "103";   /*Protocol*/
      column[4] = "-";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_IPV4VRRP:           /**< [GG.D2] IPv4:  layer3 protocol = 112:  static classify by protocol.*/
      column[0] = "VRRPv4"; /*Type*/
      column[1] = "IPv4";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "112";   /*Protocol*/
      column[4] = "-";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_IPV6VRRP:           /**< [GG.D2] IPv6:  layer3 protocol = 112:  static classify by protocol.*/
      column[0] = "VRRPv6"; /*Type*/
      column[1] = "IPv6";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "112";   /*Protocol*/
      column[4] = "-";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_IPV4BGP:            /**< [GG.D2] IPv4:  TCP:  DestPort = 179:  static classify by protocol.*/
      column[0] = "BGPv4"; /*Type*/
      column[1] = "IPv4";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "-";   /*Protocol*/
      column[4] = "179";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_IPV6BGP:            /**< [GG.D2] IPv6:  TCP:  DestPort = 179:  static classify by protocol.*/
      column[0] = "BGPv6"; /*Type*/
      column[1] = "IPv6";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "-";   /*Protocol*/
      column[4] = "179";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_ICMPIPV6_RS:        /**< [GG.D2] IPv6:  layer3 protocol = 58:  layer4 source port = 133:  static classify by protocol.*/
     column[0] = "ICMPV6 RS"; /*Type*/
      column[1] = "IPv6";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "58";   /*Protocol*/
      column[4] = "133";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_ICMPIPV6_RA:        /**< [GG.D2] IPv6:  layer3 protocol = 58:  layer4 source port = 134:  static classify by protocol.*/
  column[0] = "ICMPV6 RA"; /*Type*/
      column[1] = "IPv6";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "58";   /*Protocol*/
      column[4] = "134";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_ICMPIPV6_NS:        /**< [GG.D2] IPv6:  layer3 protocol = 58:  layer4 source port = 135:  static classify by protocol.*/
  column[0] = "ICMPV6 NS"; /*Type*/
      column[1] = "IPv6";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "58";   /*Protocol*/
      column[4] = "135";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_ICMPIPV6_NA:        /**< [GG.D2] IPv6:  layer3 protocol = 58:  layer4 source port = 136:  static classify by protocol.*/
  column[0] = "ICMPV6 NA"; /*Type*/
      column[1] = "IPv6";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "58";   /*Protocol*/
      column[4] = "136";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_ICMPIPV6_REDIRECT:  /**< [GG.D2] IPv6:  layer3 protocol = 58:  layer4 source port = 137:  static classify by protocol.*/
  column[0] = "ICMPV6 Redirect"; /*Type*/
      column[1] = "IPv6";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "58";   /*Protocol*/
      column[4] = "137";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_RIPNG:              /**< [GG.D2] UDP:  DestPort = 521:  static classify by upd port.*/
      column[0] = "RIPNG"; /*Type*/
      column[1] = "IPv6";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "-";   /*Protocol*/
      column[4] = "521";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_ARP:                /**< [GG.D2] Ether type = 0x0806*/
     column[0] = "ARP"; /*Type*/
      column[1] = "ARP";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "-";   /*Protocol*/
      column[4] = "-";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_DHCP:               /**< [GG.D2] IPv4 UDP l4DestPort = 67 or 68:  IPv6 UDP l4DestPort = 546 or 547.*/
      column[0] = "DHCP"; /*Type*/
      column[1] = "IPv4";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "-";   /*Protocol*/
      column[4] = "67|68";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_CXP :               /**< [D2] CXP (CID Exchange Protocol) action index*/
      column[0] = "CXP"; /*Type*/
      column[1] = "IPv4";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "-";   /*Protocol*/
      column[4] = "x";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_ISIS:               /**< [TM] Macda = 01:80:C2:00:00:14 or 01:80:C2:00:00:15£¬STATIC classify by mac.*/
      column[0] = "ISIS"; /*Type*/
      column[1] = "-";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "-";   /*Protocol*/
      column[4] = "-";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_IGMP_QUERY:          /**< [TM] IPv4, layer4 protocol = 7 £¬IgmpType = 0x11, STATIC classify by protocol.*/
      column[0] = "IGMP QUERY"; /*Type*/
      column[1] = "IPv4";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "2";  /*Protocol*/
      column[4] = "-";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_IGMP_REPORT_LEAVE:    /**< [TM] IPv4, layer4 protocol = 7 £¬ IgmpType = 0x12 or 0x16 or 0x17 or 0x22, STATIC classify by protocol.*/
      column[0] = "IGMP REPORT LEAVE"; /*Type*/
      column[1] = "IPv4";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "2";  /*Protocol*/
      column[4] = "-";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_IGMP_UNKNOWN:         /**< [TM] IPv4, layer4 protocol = 7 £¬ IgmpType £¡= 0x11 & 0x12 & 0x16 & 0x22 & 0x17, STATIC classify by protocol.*/
      column[0] = "IGMP UNKNOWN"; /*Type*/
      column[1] = "IPv4";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "2";   /*Protocol*/
      column[4] = "-";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_MLD_QUERY:            /**< [TM] IPv6, layer4 protocol = 7 £¬ IgmpType = 130, STATIC classify by protocol.*/
      column[0] = "MLD QUERY"; /*Type*/
      column[1] = "IPv6";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "58";   /*Protocol*/
      column[4] = "-";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_MLD_REPORT_DONE:      /**< [TM] IPv6, layer4 protocol = 7 £¬ IgmpType = 131 or 132 or 143, STATIC classify by protocol.*/
      column[0] = "MLD REPORT DONE"; /*Type*/
      column[1] = "IPv6";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "58";   /*Protocol*/
      column[4] = "-";    /*L4 Port*/
      break;
  case CTC_PDU_L3PDU_TYPE_MLD_UNKNOWN:           /**< [TM] IPv6, layer4 protocol = 7 £¬ IgmpType £¡= 130 & 131 & 132 & 143, STATIC classify by protocol.*/
      column[0] = "MLD UNKNOWN"; /*Type*/
      column[1] = "IPv6";    /*L3 Type*/
      column[2] = "-";    /*IPDA*/
      column[3] = "58";   /*Protocol*/
      column[4] = "-";    /*L4 Port*/
      break;
   default :
       return CTC_E_NONE;
  }
  _sys_usw_l3pdu_get_global_action_of_fixed_l3pdu(lchip,l3pdu_type,&action);
  CTC_DEBUG_OUT(pdu, pdu, PDU_SYS, CTC_DEBUG_LEVEL_DUMP, "%-18s%-14s%-21s%-11s%-14s%-15d%-5s\n",
                      column[0],column[1], column[2],column[3], column[4], action.action_index,action.entry_valid?"True":"False");

  if(l3pdu_type == CTC_PDU_L3PDU_TYPE_DHCP)
  {
     column[0] = "DHCPv6"; /*Type*/
     column[1] = "IPv6";    /*L3 Type*/
     column[4] = "546|547";    /*L4 Port*/

    CTC_DEBUG_OUT(pdu, pdu, PDU_SYS, CTC_DEBUG_LEVEL_DUMP, "%-18s%-14s%-21s%-11s%-14s%-15d%-5s\n",
                        column[0],column[1], column[2],column[3], column[4], action.action_index,action.entry_valid?"True":"False");

  }
  if(l3pdu_type == CTC_PDU_L3PDU_TYPE_ICMPIPV6_REDIRECT)
  {
     column[0] = "IGMP"; /*Type*/
     column[1] = "IPv4";    /*L3 Type*/
     column[2] = "-";    /*IPDA*/
     column[3] = "2";    /*Protocol*/
     column[4] = "-";    /*L4 Port*/
    CTC_DEBUG_OUT(pdu, pdu, PDU_SYS, CTC_DEBUG_LEVEL_DUMP, "%-18s%-14s%-21s%-11s%-14s%-15s%-5s\n",
                        column[0],column[1], column[2],column[3], column[4], "-","True");

  }

  return CTC_E_NONE;
}
int32 _sys_usw_pdu_show_l3pdu_dynamic_cfg(uint8 lchip, uint8 l3pdu_type)
{
    ctc_pdu_l3pdu_key_t l3pdu_key;
    ctc_pdu_global_l3pdu_action_t pdu_action;
    int32 ret = 0;
    uint8 idx = 0;
    char* column[5] = {0};
    char  str[32] = {0};;
    column[0] = "-"; /*Type*/
    column[1] = "-";    /*L3 Type*/
    column[2] = "-";    /*IPDA*/
    column[3] = "-";   /*Protocol*/
    column[4] = "-";    /*L4 Port*/

    for (idx = 0; idx < 8 ; idx++)
    {
        ret = sys_usw_l3pdu_get_classified_key( lchip, l3pdu_type, idx, &l3pdu_key);
        ret = ret ? ret: sys_usw_l3pdu_get_global_action(lchip, l3pdu_type, idx, &pdu_action);
        if (ret != CTC_E_NONE)
        {
            continue;
        }
        switch(l3pdu_type)
        {
        case CTC_PDU_L3PDU_TYPE_L3HDR_PROTO:
            if (pdu_action.entry_valid)
            {
                sal_sprintf(str, "%d", l3pdu_key.l3hdr_proto);
                column[3] = str;
                column[0] = "User-defined";
            }
            else
            {
                column[0] = "None-defined";
                column[3] = "x";
            }
            break;
        case CTC_PDU_L3PDU_TYPE_LAYER3_IPDA:
            if (pdu_action.entry_valid)
            {
                if (l3pdu_key.l3pdu_by_ipda.is_ipv4)
                {
                    column[1] = "Ipv4";
                    sal_sprintf(str, "224.0.%d.%d/32", (l3pdu_key.l3pdu_by_ipda.ipda >> 8) & 0xF, (l3pdu_key.l3pdu_by_ipda.ipda & 0xFF));
                }
                else if (l3pdu_key.l3pdu_by_ipda.is_ipv6)/*!is_ipv4 && is_ipv6*/

                {
                    column[1] = "Ipv6";
                    sal_sprintf(str, "FF0%x::%x/128", (l3pdu_key.l3pdu_by_ipda.ipda >> 8)&0xF, (l3pdu_key.l3pdu_by_ipda.ipda & 0xFF));

                }
                else/*!is_ipv4 && !is_ipv6*/
                {
                    column[1] = "-";
                    sal_sprintf(str,"-");
                }
                column[2] = str;
                column[0] = "User-defined";
            }
            else
            {
                column[0] = "None-defined";
                column[1] = "Ipv4";
                column[2] = "224.0.x(0-15).x/32";
            }
            break;
        case CTC_PDU_L3PDU_TYPE_LAYER4_PORT:
            if (pdu_action.entry_valid)
            {
                sal_sprintf(str, "%d", l3pdu_key.l3pdu_by_port.dest_port);
                column[4] = str;
                column[0] = "User-defined";
            }
            else
            {
                column[0] = "None-defined";
                column[4] = "x";
            }
            break;
        case CTC_PDU_L3PDU_TYPE_LAYER4_TYPE:
        default :
            return CTC_E_NONE;
        }
        CTC_DEBUG_OUT(pdu, pdu, PDU_SYS, CTC_DEBUG_LEVEL_DUMP, "%-18s%-14s%-21s%-11s%-14s%-15d%-5s\n",
                      column[0], column[1], column[2], column[3], column[4], pdu_action.action_index, pdu_action.entry_valid?"True":"False");
        if(l3pdu_type == CTC_PDU_L3PDU_TYPE_LAYER3_IPDA)
         {
             column[0] = "";
             column[1] = "Ipv6";
             if (pdu_action.entry_valid)
             {
                 if (!((l3pdu_key.l3pdu_by_ipda.is_ipv4) && (l3pdu_key.l3pdu_by_ipda.is_ipv6)))
                 {
                     continue;
                 }
                 sal_sprintf(str, "FF0%x::%x/128", (l3pdu_key.l3pdu_by_ipda.ipda >> 8)&0xF, (l3pdu_key.l3pdu_by_ipda.ipda & 0xFF));
                 column[2] = str;
             }
             else
             {
                 column[2] = "FF0x::xx/128";
             }
             CTC_DEBUG_OUT(pdu, pdu, PDU_SYS, CTC_DEBUG_LEVEL_DUMP, "%-18s%-14s%-21s%-11s%-14s%-15d%-5s\n",
                           column[0], column[1], column[2], column[3], column[4], pdu_action.action_index, pdu_action.entry_valid?"True":"False");         
         }
    }
    return CTC_E_NONE;
}

int32
sys_usw_pdu_show_l3pdu_cfg(uint8 lchip)
{

    uint32 index = 0;

    LCHIP_CHECK(lchip);
    SYS_LCHIP_CHECK_ACTIVE(lchip);

    /*l3pdu based on l3-header proto*/
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "L3pdu:\n");
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Note: for IPv4 address the token \"x\" used as an indication\n");
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "      of a max value 255 that user can configure.\n");
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "      for IPv6 address the token \"x\" means a max hex value 0xF.\n");
    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------------------------------------------------------------------------------\n");

    CTC_DEBUG_OUT(pdu, pdu, PDU_SYS, CTC_DEBUG_LEVEL_DUMP, "%-18s%-14s%-21s%-11s%-14s%-15s%-5s\n",
                        "Type","Layer3-Type", "IP-DA","Protocol", "Layer4-port","Action-index","Valid");

    SYS_PDU_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------------------------------------------------------------------------------\n");

    for (index = 0; index < MAX_CTC_PDU_L3PDU_TYPE ; index++)
    {
        if ((CTC_PDU_L3PDU_TYPE_L3HDR_PROTO == index) || (CTC_PDU_L3PDU_TYPE_LAYER3_IPDA == index)
           || (CTC_PDU_L3PDU_TYPE_LAYER4_PORT == index) || (CTC_PDU_L3PDU_TYPE_LAYER4_TYPE == index) )
        {
          continue;
        }
        else
        {
           _sys_usw_pdu_show_l3pdu_static_cfg(lchip,index);
        }

    }
    for (index = 0; index < MAX_CTC_PDU_L3PDU_TYPE ; index++)
    {
        if ((CTC_PDU_L3PDU_TYPE_L3HDR_PROTO == index) || (CTC_PDU_L3PDU_TYPE_LAYER3_IPDA == index)
           || (CTC_PDU_L3PDU_TYPE_LAYER4_PORT == index) || (CTC_PDU_L3PDU_TYPE_LAYER4_TYPE == index) )
        {
          _sys_usw_pdu_show_l3pdu_dynamic_cfg(lchip,index);
        }
    }
    return CTC_E_NONE;
}

