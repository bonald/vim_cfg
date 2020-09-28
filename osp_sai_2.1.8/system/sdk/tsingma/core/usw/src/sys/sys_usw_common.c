/**
 @file sys_usw_common.c

 @date 2011-11-28

 @version v2.0

 This file provides functions which can be used for GB
*/

#include "ctc_qos.h"
#include "ctc_error.h"
#include "ctc_port.h"
#include "ctc_register.h"
#include "ctc_vlan.h"
#include "sal.h"

#include "sys_usw_common.h"

#include "usw/include/drv_io.h"

/* feature support array, 1 means enable, 0 means disable */
uint8 feature_support_arr[CTC_FEATURE_MAX] =
    /*PORT VLAN LINKAGG CHIP FTM NEXTHOP L2 L3IF IPUC IPMC IP_TUNNEL SCL ACL QOS SECURITY STATS MPLS OAM APS PTP DMA INTERRUPT PACKET PDU MIRROR BPE STACKING OVERLAY IPFIX EFD MONITOR FCOE TRILL WLAN, NPM VLAN_MAPPING DOT1AE DIAG*/
    { 1,   1,   1,       1,   1,  1,      1,   1,   1,  1,   1,        1,  1,  1,  1,       1,    1,   1,  1,  1,  1,   1,        1,     1,  1,     1,   1,       1,   1,    1,  1,      1,    1,    1,   1,   1,       1,    1,     1, 1, 1, 1};

extern int32
sys_usw_ftm_query_table_entry_num(uint8 lchip, uint32 table_id, uint32* entry_num);

char*
sys_output_mac(mac_addr_t in_mac)
{
    static char out_mac[20] = {0};

    sal_sprintf(out_mac, "%.2x%.2x.%.2x%.2x.%.2x%.2x", in_mac[0], in_mac[1], in_mac[2], in_mac[3], in_mac[4], in_mac[5]);
    return out_mac;
}


uint8
sys_usw_map_acl_tcam_lkup_type(uint8 lchip, uint8 ctc_lkup_t)
{
    switch(ctc_lkup_t)
    {
        case CTC_ACL_TCAM_LKUP_TYPE_L2:
            return SYS_ACL_TCAM_LKUP_TYPE_L2;
        case CTC_ACL_TCAM_LKUP_TYPE_L2_L3:
            return SYS_ACL_TCAM_LKUP_TYPE_L2_L3;
        case CTC_ACL_TCAM_LKUP_TYPE_L3:
            return SYS_ACL_TCAM_LKUP_TYPE_L3;
        case CTC_ACL_TCAM_LKUP_TYPE_VLAN:
            return SYS_ACL_TCAM_LKUP_TYPE_VLAN;
        case CTC_ACL_TCAM_LKUP_TYPE_L3_EXT:
            return SYS_ACL_TCAM_LKUP_TYPE_L3_EXT;
        case CTC_ACL_TCAM_LKUP_TYPE_L2_L3_EXT:
            return SYS_ACL_TCAM_LKUP_TYPE_L2_L3_EXT;
        case CTC_ACL_TCAM_LKUP_TYPE_CID:
            return SYS_ACL_TCAM_LKUP_TYPE_CID;
        case CTC_ACL_TCAM_LKUP_TYPE_INTERFACE:
            return SYS_ACL_TCAM_LKUP_TYPE_INTERFACE;
        case CTC_ACL_TCAM_LKUP_TYPE_FORWARD:
            return SYS_ACL_TCAM_LKUP_TYPE_FORWARD;
        case CTC_ACL_TCAM_LKUP_TYPE_FORWARD_EXT:
            return SYS_ACL_TCAM_LKUP_TYPE_FORWARD_EXT;
        case CTC_ACL_TCAM_LKUP_TYPE_UDF:
            return SYS_ACL_TCAM_LKUP_TYPE_UDF;
        case CTC_ACL_TCAM_LKUP_TYPE_COPP:
            return SYS_ACL_TCAM_LKUP_TYPE_COPP;
        default:
            return SYS_ACL_TCAM_LKUP_TYPE_NONE;
    }
}

uint8
sys_usw_unmap_acl_tcam_lkup_type(uint8 lchip, uint8 sys_lkup_t)
{
    switch(sys_lkup_t)
    {
        case SYS_ACL_TCAM_LKUP_TYPE_L2:
            return CTC_ACL_TCAM_LKUP_TYPE_L2;
        case SYS_ACL_TCAM_LKUP_TYPE_L2_L3:
            return CTC_ACL_TCAM_LKUP_TYPE_L2_L3;
        case SYS_ACL_TCAM_LKUP_TYPE_L3:
            return  CTC_ACL_TCAM_LKUP_TYPE_L3;
        case SYS_ACL_TCAM_LKUP_TYPE_VLAN:
            return CTC_ACL_TCAM_LKUP_TYPE_VLAN;
        case SYS_ACL_TCAM_LKUP_TYPE_L3_EXT:
            return  CTC_ACL_TCAM_LKUP_TYPE_L3_EXT;
        case SYS_ACL_TCAM_LKUP_TYPE_L2_L3_EXT:
            return CTC_ACL_TCAM_LKUP_TYPE_L2_L3_EXT;
        case SYS_ACL_TCAM_LKUP_TYPE_CID:
            return CTC_ACL_TCAM_LKUP_TYPE_CID;
        case SYS_ACL_TCAM_LKUP_TYPE_INTERFACE:
            return CTC_ACL_TCAM_LKUP_TYPE_INTERFACE;
        case SYS_ACL_TCAM_LKUP_TYPE_FORWARD:
            return CTC_ACL_TCAM_LKUP_TYPE_FORWARD;
        case SYS_ACL_TCAM_LKUP_TYPE_FORWARD_EXT:
            return CTC_ACL_TCAM_LKUP_TYPE_FORWARD_EXT;
        case SYS_ACL_TCAM_LKUP_TYPE_COPP:
            return CTC_ACL_TCAM_LKUP_TYPE_COPP;
        case SYS_ACL_TCAM_LKUP_TYPE_UDF:
            return CTC_ACL_TCAM_LKUP_TYPE_UDF;
        default:
            return CTC_ACL_TCAM_LKUP_TYPE_L2;
    }
}


/* value = division << shift_bits,is_negative = 0,the computed value is slightly larger*/
int32
sys_usw_common_get_compress_near_division_value(uint8 lchip, uint32 value,uint8 division_wide,
                                                                  uint8 shift_bit_wide, uint16* division, uint16* shift_bits, uint8 is_negative)
{
    uint8 i = 0;
    uint32 valuetmp = 0;
    uint32 base = 0;
    uint16 max_division = 0;
    uint16 max_shift_bit = 0;

    CTC_PTR_VALID_CHECK(division);
    CTC_PTR_VALID_CHECK(shift_bits);
    max_division = 1 << division_wide;
    max_shift_bit = 1 << shift_bit_wide;

    for (i = 0; i < max_shift_bit; i++)
    {
        base = (value >> i);

        if (base < max_division)
        {
            *division = base;
            *shift_bits = i;
            valuetmp = base << i;
            if (valuetmp != value)
            {
                break;
            }
            return CTC_E_NONE;
        }
    }

    valuetmp = value / max_division;
    if(0 == valuetmp)
    {
        *division = value;
        *shift_bits = 0;
        return CTC_E_NONE;
    }

    for (i = 1; i < max_shift_bit; i++)
    {
        if ((valuetmp >= (1 << (i - 1))) && (valuetmp <= ((1 << i)- 1)))
        {
            if(0 == is_negative)
            {
                if ((value + ((1 << i) - 1)) / (1 << i) >= max_division)
                {
                    i++;
                }

                *shift_bits = i;
                *division = (value + ((1 << i) - 1)) / (1 << i);
            }
            else
            {
                if (value  / (1 << i) >= max_division)
                {
                    i++;
                }

                *shift_bits = i;
                *division = value / (1 << i);
            }
            return CTC_E_NONE;
        }
    }

    return CTC_E_INVALID_PARAM;

}

extern uint32
sys_usw_global_get_chip_capability(uint8 lchip, ctc_global_capability_type_t type);

uint32
sys_usw_common_get_vport_num(uint8 lchip)
{
    return MCHIP_CAP(SYS_CAP_SPEC_LOGIC_PORT_NUM);
}

bool
sys_usw_common_check_feature_support(uint8 feature)
{
    if (feature_support_arr[feature])
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

const char*
sys_usw_get_pkt_type_desc(uint8 pkt_type)
{
    switch(pkt_type)
    {
        case CTC_PARSER_PKT_TYPE_ETHERNET:
            return "ETHERNET";
        case CTC_PARSER_PKT_TYPE_IP_OR_IPV6:
            return "IP/IPv4";
        case CTC_PARSER_PKT_TYPE_MPLS:
            return "MPLS";
        case CTC_PARSER_PKT_TYPE_IPV6:
            return "IPV6";
        case CTC_PARSER_PKT_TYPE_MCAST_MPLS:
            return "MCAST_MPLS";
        case CTC_PARSER_PKT_TYPE_CESOETH:
            return "CESOETH";
        case CTC_PARSER_PKT_TYPE_FLEXIBLE:
            return "FLEXIBLE";
        case CTC_PARSER_PKT_TYPE_RESERVED:
            return "RESERVED";
        default:
            return "Error type";
    }

}

int32
sys_usw_dword_reverse_copy(uint32* dest, uint32* src, uint32 len)
{
    uint32 i = 0;
    for (i = 0; i < len; i++)
    {
        *(dest + i) = *(src + (len - i - 1));
    }
    return 0;
}

int32
sys_usw_byte_reverse_copy(uint8* dest, uint8* src, uint32 len)
{
    uint32 i = 0;
    for (i = 0; i < len; i++)
    {
        *(dest + i) = *(src + (len - 1 - i));
    }
    return 0;
}

int32
sys_usw_swap32(uint32* data, int32 len, uint32 hton)
{
    int32 cnt = 0;

    for (cnt = 0; cnt < len; cnt++)
    {
        if (hton)
        {
            data[cnt] = sal_htonl(data[cnt]);
        }
        else
        {
            data[cnt] = sal_ntohl(data[cnt]);
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_task_create(uint8 lchip,
                                sal_task_t** ptask,
                                char* name,
                                size_t stack_size,
                                int prio,
                                sal_task_type_t task_type,
                                unsigned long long cpu_mask,
                                void (* start)(void*),
                                void* arg)
{
    int32 ret = 0;
    sal_task_t tmp_task;

    sal_memset(&tmp_task, 0, sizeof(sal_task_t));
    tmp_task.cpu_mask = cpu_mask;
    tmp_task.task_type = task_type;
    sal_sprintf(tmp_task.name, "%s-%d", name, lchip);

    ret = sal_task_create(ptask, tmp_task.name,stack_size, prio, start, arg);

    return ret;
}

int32
sys_usw_ma_clear_write_cache(uint8 lchip)
{
    uint32 cmd = 0;
    DsMaName_m ds_ma_name;
    uint8  trigger_timer = 0;
    uint32 ma_name_entry_num = 0;
    uint8  rsv_maname_idx = 0;

    if (!DRV_IS_DUET2(lchip))
    {
        return CTC_E_NONE;
    }

    sys_usw_ftm_query_table_entry_num(lchip, DsMaName_t,  &ma_name_entry_num);
    if (!ma_name_entry_num)
    {
        return CTC_E_NONE;
    }

    rsv_maname_idx = ma_name_entry_num - 1;
    cmd = DRV_IOW(DsMaName_t, DRV_ENTRY_FLAG);
    sal_memset(&ds_ma_name, 0xff, sizeof(ds_ma_name));
    for (trigger_timer = 0; trigger_timer < 8; trigger_timer++)
    {
        DRV_IOCTL(lchip, rsv_maname_idx, cmd, &ds_ma_name);
    }
    return CTC_E_NONE;
}

int32
sys_usw_ma_add_to_asic(uint8 lchip, uint32 ma_index, void* dsma)
{
    uint32 cmd = 0;

    cmd = DRV_IOW(DsMa_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, ma_index, cmd, dsma));
    sys_usw_ma_clear_write_cache(lchip);
    return CTC_E_NONE;
}

