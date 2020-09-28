/**
 @file sys_dmps_api.c

 @date 2009-10-19

 @version v2.0

 The file define APIs of chip of sys layer
*/
/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/
#include "ctc_error.h"
#include "sys_usw_chip.h"
#include "sys_usw_port.h"
#include "sys_usw_mac.h"
#include "sys_usw_datapath.h"
#include "sys_usw_peri.h"
#include "sys_usw_dmps.h"

#include "drv_api.h"

/****************************************************************************
 *
 * Global and static
 *
 *****************************************************************************/

/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/

int32
sys_usw_mac_set_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32 value)
{
    int32 ret = 0;
    if(MCHIP_API(lchip)->mac_set_property)
    {
        ret = MCHIP_API(lchip)->mac_set_property(lchip, gport, port_prop, value);
    }
    else return CTC_E_INVALID_PTR;

    return ret;
}

int32
sys_usw_mac_get_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32* p_value)
{
    int32 ret = 0;
    if(MCHIP_API(lchip)->mac_get_property)
    {
        ret = MCHIP_API(lchip)->mac_get_property(lchip, gport, port_prop, p_value);
    }
    else return CTC_E_INVALID_PTR;

    return ret;
}

int32
sys_usw_mac_set_interface_mode(uint8 lchip, uint32 gport, ctc_port_if_mode_t* if_mode)
{
    int32 ret = 0;
    if(MCHIP_API(lchip)->mac_set_interface_mode)
    {
        ret = MCHIP_API(lchip)->mac_set_interface_mode(lchip, gport, if_mode);
    }
    else return CTC_E_INVALID_PTR;

    return ret;
}

int32
sys_usw_mac_get_link_up(uint8 lchip, uint32 gport, uint32* p_is_up, uint32 is_port)
{
    int32 ret = 0;
    if(MCHIP_API(lchip)->mac_get_link_up)
    {
        ret = MCHIP_API(lchip)->mac_get_link_up(lchip, gport, p_is_up, is_port);
    }
    else return CTC_E_INVALID_PTR;

    return ret;
}

int32
sys_usw_mac_get_capability(uint8 lchip, uint32 gport, ctc_port_capability_type_t type, void* p_value)
{
    int32 ret = 0;
    if(MCHIP_API(lchip)->mac_get_capability)
    {
        ret = MCHIP_API(lchip)->mac_get_capability(lchip, gport, type, p_value);
    }
    else return CTC_E_INVALID_PTR;

    return ret;
}

int32
sys_usw_mac_self_checking(uint8 lchip, uint32 gport)
{
    int32 ret = 0;
    if(MCHIP_API(lchip)->mac_self_checking)
    {
        ret = MCHIP_API(lchip)->mac_self_checking(lchip, gport);
    }
    else return CTC_E_INVALID_PTR;

    return ret;
}

int32
sys_usw_serdes_set_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value)
{
    int32 ret = 0;
    if(MCHIP_API(lchip)->serdes_set_property)
    {
        ret = MCHIP_API(lchip)->serdes_set_property(lchip, chip_prop, p_value);
    }
    else return CTC_E_INVALID_PTR;

    return ret;
}

int32
sys_usw_serdes_get_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value)
{
    int32 ret = 0;
    if(MCHIP_API(lchip)->serdes_get_property)
    {
        ret = MCHIP_API(lchip)->serdes_get_property(lchip, chip_prop, p_value);
    }
    else return CTC_E_INVALID_PTR;

    return ret;
}

int32
sys_usw_serdes_set_mode(uint8 lchip, ctc_chip_serdes_info_t* p_serdes_info)
{
    int32 ret = 0;
    if(MCHIP_API(lchip)->serdes_set_mode)
    {
        ret = MCHIP_API(lchip)->serdes_set_mode(lchip, p_serdes_info);
    }
    else return CTC_E_INVALID_PTR;

    return ret;
}

int32
sys_usw_dmps_set_port_property(uint8 lchip, uint32 gport, sys_dmps_port_property_t dmps_port_prop, void *p_value)
{
    int32  ret = CTC_E_NONE;

    CTC_PTR_VALID_CHECK(p_value);

    switch (dmps_port_prop)
    {
    case SYS_DMPS_PORT_PROP_XPIPE_ENABLE:
        ret = sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_XPIPE_EN, *(uint32*)p_value);
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return ret;
}

int32
sys_usw_dmps_get_port_property(uint8 lchip, uint32 gport, sys_dmps_port_property_t dmps_port_prop, void *p_value)
{
    int32  ret = CTC_E_NONE;
    uint8  i   = 0;
    uint16 lport = 0;
    uint16 chip_port = 0;
    uint16 serdes_id = 0;
    uint8 serdes_num = 0;
    sys_datapath_lport_attr_t* p_port = NULL;
    sys_dmps_serdes_info_t *p_dmps_serdes = NULL;
    uint32 *p_tmp_value = NULL;

    CTC_PTR_VALID_CHECK(p_value);

    switch (dmps_port_prop)
    {
    case SYS_DMPS_PORT_PROP_EN:
        ret = sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_MAC_EN, (uint32*)p_value);
        break;
    case SYS_DMPS_PORT_PROP_LINK_UP:
        ret = sys_usw_mac_get_link_up(lchip, gport, (uint32*)p_value, 0);
        break;
    case SYS_DMPS_PORT_PROP_TX_IPG:
        ret = sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_MAC_TX_IPG, (uint32*)p_value);
        break;
    case SYS_DMPS_PORT_PROP_MAC_ID:
        p_tmp_value = (uint32 *)p_value;
        ret = sys_usw_mac_get_port_capability(lchip, CTC_MAP_GPORT_TO_LPORT(gport), &p_port);
        if (ret < 0)
        {
            *p_tmp_value = 0xff;
            return ret;
        }

        if (p_port->port_type != SYS_DMPS_NETWORK_PORT)
        {
            *p_tmp_value = 0xff;
        }
        else
        {
            *p_tmp_value = p_port->mac_id + p_port->slice_id*64;
        }

        break;
    case SYS_DMPS_PORT_PROP_CHAN_ID:
        p_tmp_value = (uint32 *)p_value;

        if(FALSE == sys_usw_chip_is_local(lchip, CTC_MAP_GPORT_TO_GCHIP(gport)))
        {
            *p_tmp_value = 0xff;
            return CTC_E_INVALID_CHIP_ID;
        }

        ret = sys_usw_mac_get_port_capability(lchip, CTC_MAP_GPORT_TO_LPORT(gport), &p_port);
        if (ret < 0)
        {
            *p_tmp_value = 0xff;
            return ret;
        }

        if (SYS_DMPS_RSV_PORT == p_port->port_type)
        {
            *p_tmp_value = 0xff;
        }
        else
        {
            *p_tmp_value = p_port->chan_id + p_port->slice_id*84;
        }

        break;
    case SYS_DMPS_PORT_PROP_SLICE_ID:
        p_tmp_value = (uint32 *)p_value;
        *p_tmp_value = 0;
        break;
    case SYS_DMPS_PORT_PROP_SERDES:

#ifdef EMULATION_ENV /*EMULATION testing*/
    break;
#endif

        p_dmps_serdes = (sys_dmps_serdes_info_t*)p_value;
        lport = CTC_MAP_GPORT_TO_LPORT(gport);
        ret = sys_usw_mac_get_port_capability(lchip, CTC_MAP_GPORT_TO_LPORT(gport), &p_port);
        if (ret < 0)
        {
            p_dmps_serdes->serdes_id[0] = 0xff;
            p_dmps_serdes->serdes_id[1] = 0xff;
            p_dmps_serdes->serdes_id[2] = 0xff;
            p_dmps_serdes->serdes_id[3] = 0xff;
            p_dmps_serdes->serdes_mode = 0xff;
            p_dmps_serdes->serdes_num = 0;
            return ret;
        }

        if (SYS_DMPS_NETWORK_PORT != p_port->port_type)
        {
            p_dmps_serdes->serdes_id[0] = 0xff;
            p_dmps_serdes->serdes_id[1] = 0xff;
            p_dmps_serdes->serdes_id[2] = 0xff;
            p_dmps_serdes->serdes_id[3] = 0xff;
            p_dmps_serdes->serdes_mode = 0xff;
            p_dmps_serdes->serdes_num = 0;
        }
        else
        {
            chip_port = SYS_MAP_SYS_LPORT_TO_DRV_LPORT(lport) - p_port->slice_id*256;
            CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_with_lport(lchip, chip_port, p_port->slice_id, &serdes_id, &serdes_num));
            
            p_dmps_serdes->serdes_id[0] = 0xff;
            p_dmps_serdes->serdes_id[1] = 0xff;
            p_dmps_serdes->serdes_id[2] = 0xff;
            p_dmps_serdes->serdes_id[3] = 0xff;
            if ((CTC_CHIP_SERDES_CG_MODE == p_port->pcs_mode) || (CTC_CHIP_SERDES_XLG_MODE == p_port->pcs_mode)
                || (CTC_CHIP_SERDES_XAUI_MODE == p_port->pcs_mode) || (CTC_CHIP_SERDES_DXAUI_MODE == p_port->pcs_mode)
                || ((CTC_CHIP_SERDES_LG_MODE == p_port->pcs_mode)&& (0 == p_port->flag)))
            {
                for (i = 0; i < serdes_num; i++)
                {
                    p_dmps_serdes->serdes_id[i] = serdes_id + i;
                }
            }
            else if ((CTC_CHIP_SERDES_LG_MODE == p_port->pcs_mode)&& (1 == p_port->flag))
            {
                if (chip_port % 4)  /* 50G port 2 */
                {
                    p_dmps_serdes->serdes_id[0] = serdes_id;
                    p_dmps_serdes->serdes_id[1] = serdes_id+3;
                }
                else    /* 50G port 0 */
                {
                    p_dmps_serdes->serdes_id[0] = serdes_id;
                    p_dmps_serdes->serdes_id[1] = serdes_id+1;
                }
            }
            else
            {
                p_dmps_serdes->serdes_id[0] = serdes_id;
            }
            p_dmps_serdes->serdes_mode = p_port->pcs_mode;
            p_dmps_serdes->serdes_num = serdes_num;
        }

        break;
    case SYS_DMPS_PORT_PROP_PORT_TYPE:
        p_tmp_value = (uint32 *)p_value;
        ret = sys_usw_mac_get_port_capability(lchip, CTC_MAP_GPORT_TO_LPORT(gport), &p_port);
        if (ret < 0)
        {
            *p_tmp_value = 0xff;
            return ret;
        }
        *p_tmp_value = p_port->port_type;
        break;
    case SYS_DMPS_PORT_PROP_SPEED_MODE:
        p_tmp_value = (uint32 *)p_value;
        ret = sys_usw_mac_get_port_capability(lchip, CTC_MAP_GPORT_TO_LPORT(gport), &p_port);
        if (ret < 0)
        {
            *p_tmp_value = 0xff;
            return ret;
        }
        *p_tmp_value = p_port->speed_mode;
        break;
    case SYS_DMPS_PORT_PROP_WLAN_ENABLE:
        p_tmp_value = (uint32 *)p_value;
        *p_tmp_value = sys_usw_datapath_get_wlan_en(lchip);
        break;
    case SYS_DMPS_PORT_PROP_DOT1AE_ENABLE:
        p_tmp_value = (uint32 *)p_value;
        *p_tmp_value = sys_usw_datapath_get_dot1ae_en(lchip);
        break;
    case SYS_DMPS_PORT_PROP_MAC_STATS_ID:
        p_tmp_value = (uint32 *)p_value;
        ret = sys_usw_mac_get_port_capability(lchip, CTC_MAP_GPORT_TO_LPORT(gport), &p_port);
        if (ret < 0)
        {
            *p_tmp_value = 0xff;
            return ret;
        }

        if (p_port->port_type != SYS_DMPS_NETWORK_PORT)
        {
            *p_tmp_value = 0xff;
            break;
        }

        if(DRV_IS_DUET2(lchip))
        {
            *p_tmp_value = p_port->mac_id + p_port->slice_id*64;
        }
        else
        {
            *p_tmp_value = p_port->tbl_id;
        }
        break;
    case SYS_DMPS_PORT_PROP_XPIPE_ENABLE:
        ret = sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_XPIPE_EN, (uint32*)p_value);
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    return ret;
}

int32
sys_usw_dmps_set_port_chan_mac_mapping(uint8 lchip, uint32 gport, uint8 chan_id, uint8 mac_id)
{
    uint32 cmd = 0;
    uint8 slice_id = 0;
    uint16 lport = 0;
    NetRxChannelMap_m chan_map;
    EpeScheduleNetPortToChanRa_m port_to_chan;
    EpeScheduleNetChanToPortRa_m chan_to_port;
    IpeHeaderAdjustPhyPortMap_m ipe_port_map;
    EpeHeaderAdjustPhyPortMap_m epe_port_map;

    if (SYS_DMPS_NETWORK_PORT != gport)
    {
        return CTC_E_INVALID_CONFIG;
    }

    slice_id = SYS_MAP_CHANID_TO_SLICE(chan_id);
    lport = CTC_MAP_GPORT_TO_LPORT(gport);

    /* step 1, Set port-chan mapping */
    /* 1.1 IPE */
    cmd = DRV_IOR((IpeHeaderAdjustPhyPortMap_t), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &ipe_port_map);
    SetIpeHeaderAdjustPhyPortMap(V, localPhyPort_f, &ipe_port_map, lport);
    cmd = DRV_IOW((IpeHeaderAdjustPhyPortMap_t), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &ipe_port_map);
    /* 1.2 EPE */
    cmd = DRV_IOR(EpeHeaderAdjustPhyPortMap_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &epe_port_map);
    SetEpeHeaderAdjustPhyPortMap(V, localPhyPort_f, &epe_port_map, lport);
    cmd = DRV_IOW(EpeHeaderAdjustPhyPortMap_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &epe_port_map);

    /* step 2, Set chan-mac mapping */
    /* 2.1 NetRxChannelMap */
    SetNetRxChannelMap(V, cfgPortToChanMapping_f, &chan_map, chan_id);
    cmd = DRV_IOW((NetRxChannelMap_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &chan_map);
    SetNetRxChannelMap(V, cfgChanToPortMapping_f, &chan_map, mac_id);
    cmd = DRV_IOW((NetRxChannelMap_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, mac_id, cmd, &chan_map);
    /* 2.2 EpeSch*/
    SetEpeScheduleNetPortToChanRa(V, internalChanId_f, &port_to_chan, chan_id);
    cmd = DRV_IOW((EpeScheduleNetPortToChanRa_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, mac_id, cmd, &port_to_chan);

    SetEpeScheduleNetChanToPortRa(V, physicalPortId_f, &chan_to_port, mac_id);
    cmd = DRV_IOW((EpeScheduleNetChanToPortRa_t+slice_id), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &chan_to_port);

    /* step 3, Update DS */
    ///TODO:
    return CTC_E_NONE;
}

/*chan_id is global chan id, return port is sys Port*/
uint16
sys_usw_dmps_get_lport_with_chan(uint8 lchip, uint8 chan_id)
{
    uint32 cmd = 0;
    IpeHeaderAdjustPhyPortMap_m port_map;
    uint16 chip_lport = 0;
    uint16 drv_lport = 0;
    uint8 slice_id = 0;

    slice_id = SYS_MAP_CHANID_TO_SLICE(chan_id);

    cmd = DRV_IOR((IpeHeaderAdjustPhyPortMap_t), DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, chan_id, cmd, &port_map);

    chip_lport = GetIpeHeaderAdjustPhyPortMap(V,localPhyPort_f,&port_map);

    drv_lport = slice_id*256+chip_lport;

    return SYS_MAP_DRV_LPORT_TO_SYS_LPORT(drv_lport);
}

bool
sys_usw_dmps_check_mac_valid(uint8 lchip, uint8 mac_id)
{
    uint16 lport = 0;
    uint8 slice_id = 0;
    uint8 slice_mac = 0;

    slice_id = (mac_id >= 64)?1:0;
    slice_mac = mac_id - slice_id*64;

    lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, slice_mac);
    if (lport == SYS_DATAPATH_USELESS_MAC)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*mac_id is global mac id, return port is sys Port*/
uint16
sys_usw_dmps_get_lport_with_mac(uint8 lchip, uint8 mac_id)
{
    uint16 chip_lport = 0;
    uint16 drv_lport = 0;
    uint8 slice_id = 0;
    uint8 slice_mac = 0;

    slice_id = (mac_id >= 64)?1:0;
    slice_mac = mac_id - slice_id*64;

    chip_lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, slice_mac);
    if (SYS_COMMON_USELESS_MAC == chip_lport)
    {
        return SYS_COMMON_USELESS_MAC;
    }

    drv_lport = slice_id*256+chip_lport;

    return SYS_MAP_DRV_LPORT_TO_SYS_LPORT(drv_lport);
}

/*mac_id is global mac id, return port is sys Port*/
uint16
sys_usw_dmps_get_lport_with_mac_tbl_id(uint8 lchip, uint8 mac_tbl_id)
{
    uint16 chip_lport = 0;
    uint16 drv_lport = 0;
    uint8 slice_id = 0;
    uint8 slice_mac = 0;

    slice_id = 0;
    slice_mac = mac_tbl_id;

    chip_lport = sys_usw_datapath_get_lport_with_mac_tbl_id(lchip, slice_id, slice_mac);
    if (SYS_COMMON_USELESS_MAC == chip_lport)
    {
        return SYS_COMMON_USELESS_MAC;
    }

    drv_lport = slice_id*256+chip_lport;

    return SYS_MAP_DRV_LPORT_TO_SYS_LPORT(drv_lport);
}


uint8
sys_usw_dmps_get_mac_type(uint8 lchip, uint8 mac_id)
{
    uint16 lport = 0;
    sys_datapath_lport_attr_t* p_port_cap = NULL;
    uint8 slice_id = 0;
    uint8 slice_mac = 0;

    slice_id = (mac_id >= 64)?1:0;
    slice_mac = mac_id - slice_id*64;

    lport = sys_usw_datapath_get_lport_with_mac(lchip, slice_id, slice_mac);
    if (lport == SYS_DATAPATH_USELESS_MAC)
    {
        return 0xff;
    }

    p_port_cap = sys_usw_datapath_get_port_capability(lchip, lport, slice_id);
    if (p_port_cap == NULL)
    {
        return 0xff;
    }

    return p_port_cap->speed_mode;
}


uint32 SYS_GET_MAC_ID(uint8 lchip, uint32 gport)
{
    uint32 val = 0;
    sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_MAC_ID, (void*)&val);
    return val;
}

uint32 SYS_GET_CHANNEL_ID(uint8 lchip, uint32 gport)
{
    uint32 val = 0;
    sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_CHAN_ID, (void*)&val);
    return val;
}

/*get clock core, core_type:0-plla, 1-pllb*/
uint16
sys_usw_dmps_get_core_clock(uint8 lchip, uint8 core_type)
{
    return sys_usw_datapath_get_core_clock(lchip, core_type);
}


int32
sys_usw_dmps_set_dlb_chan_type(uint8 lchip, uint8 chan_id)
{

#ifdef EMULATION_ENV
    return 0;
#endif

    CTC_ERROR_RETURN(sys_usw_peri_set_dlb_chan_type(lchip, chan_id));
    return CTC_E_NONE;
}

