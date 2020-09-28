/**
 @file ctcs_isr.c

 @date 2010-1-26

 @version v2.0

*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "sal.h"
#include "api/include/ctcs_api.h"
#include "ctc_app_isr.h"
#include "ctc_app.h"
#include "ctc_app_cfg_chip_profile.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
int32
ctcs_app_isr_sw_learning_process_isr(uint8 gchip, void* p_data)
{
    ctc_learning_cache_t* p_cache = (ctc_learning_cache_t*)p_data;
    ctc_l2_addr_t l2_addr;
    uint32 index = 0;
    uint32 nh_id = 0;
    uint32 with_nh = FALSE;
    int32 ret = CTC_E_NONE;
    uint8 lchip = 0;

    sal_memset(&l2_addr, 0, sizeof(l2_addr));

    ctc_app_get_lchip_id(gchip, &lchip);

    for (index = 0; index < p_cache->entry_num; index++)
    {
        with_nh = FALSE;

        /* pizzabox */
        l2_addr.flag = 0;
        l2_addr.fid = p_cache->learning_entry[index].fid;
        l2_addr.flag = 0;

        /*Using learning fifo*/
        if (!p_cache->sync_mode)
        {
            l2_addr.mac[0] = (p_cache->learning_entry[index].mac_sa_32to47 & 0xff00) >> 8;
            l2_addr.mac[1] = (p_cache->learning_entry[index].mac_sa_32to47 & 0xff);
            l2_addr.mac[2] = (p_cache->learning_entry[index].mac_sa_0to31 & 0xff000000) >> 24;
            l2_addr.mac[3] = (p_cache->learning_entry[index].mac_sa_0to31 & 0xff0000) >> 16;
            l2_addr.mac[4] = (p_cache->learning_entry[index].mac_sa_0to31 & 0xff00) >> 8;
            l2_addr.mac[5] = (p_cache->learning_entry[index].mac_sa_0to31 & 0xff);
        }
        else
        {
            /*Using Dma*/
            l2_addr.mac[0] = p_cache->learning_entry[index].mac[0];
            l2_addr.mac[1] = p_cache->learning_entry[index].mac[1];
            l2_addr.mac[2] = p_cache->learning_entry[index].mac[2];
            l2_addr.mac[3] = p_cache->learning_entry[index].mac[3];
            l2_addr.mac[4] = p_cache->learning_entry[index].mac[4];
            l2_addr.mac[5] = p_cache->learning_entry[index].mac[5];
        }

        if (p_cache->learning_entry[index].is_ether_oam)
        {
            /* Ethernet OAM  --- mip ccm database */
            /* shoud send msg to protocol module  for  mip ccm database */
        }

        if (p_cache->learning_entry[index].is_logic_port)
        {
            l2_addr.gport = p_cache->learning_entry[index].logic_port;
            if (ctcs_l2_get_nhid_by_logic_port(lchip, l2_addr.gport, &nh_id) < 0)
            {
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Can't find nh_id by logic port ,Learning failed \n");
                continue;
            }
            else
            {
                with_nh = TRUE;
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Learning: nh_id->%d \n", nh_id);
            }


        }
        else
        {
            /* no need to add vlan+fid->nh,because add_fdb_with_nh use logic-port study */

            /*normal bridge */
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " Learning: normal bridge!!!!!!!!!!!\n");
            l2_addr.gport = p_cache->learning_entry[index].global_src_port;
        }

        if (!p_cache->learning_entry[index].is_hw_sync)
        {
            if (!with_nh)
            {
                ret = ctcs_l2_add_fdb(lchip, &l2_addr);
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Learning: l2_addr.gport->0x%x \n", l2_addr.gport);
            }
            else
            {
                ret = ctcs_l2_add_fdb_with_nexthop(lchip, &l2_addr, nh_id);
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Learning: logic_port->0x%x \n", l2_addr.gport);
            }
        }
        else
        {
            if(p_cache->learning_entry[index].is_logic_port)
            {
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Learning Sync: MAC:%.4x.%.4x.%.4x  FID:%-5d  LogicPort:%d\n",
                                                    sal_ntohs(*(unsigned short*)&l2_addr.mac[0]),
                                                    sal_ntohs(*(unsigned short*)&l2_addr.mac[2]),
                                                    sal_ntohs(*(unsigned short*)&l2_addr.mac[4]),
                                                    l2_addr.fid, l2_addr.gport);
            }
            else
            {
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Learning Sync: MAC:%.4x.%.4x.%.4x  FID:%-5d  GPort:0x%x\n",
                                                    sal_ntohs(*(unsigned short*)&l2_addr.mac[0]),
                                                    sal_ntohs(*(unsigned short*)&l2_addr.mac[2]),
                                                    sal_ntohs(*(unsigned short*)&l2_addr.mac[4]),
                                                    l2_addr.fid, l2_addr.gport);
            }
        }

        if (ret)
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Learning fdb fail, ret:%d \n", ret);
        }

    }

    return ret;
}

int32
ctcs_app_isr_sw_aging_process_isr(uint8 gchip, void* p_data)
{
    ctc_aging_fifo_info_t* p_fifo = (ctc_aging_fifo_info_t*)p_data;
    ctc_aging_info_entry_t* p_entry = NULL;
    uint32 i = 0;
    int32 ret = CTC_E_NONE;
    ctc_l2_addr_t l2_addr;
    char buf[CTC_IPV6_ADDR_STR_LEN];
    ctc_ipuc_param_t ipuc_param;
    uint8 lchip = 0;

    ctc_app_get_lchip_id(gchip, &lchip);

    /*Using learning fifo*/
    if (!p_fifo->sync_mode)
    {
        for (i = 0; i < p_fifo->fifo_idx_num; i++)
        {
            ret = ctcs_l2_remove_fdb_by_index(lchip, p_fifo->aging_index_array[i]);
            if (ret < 0)
            {
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Remove aging index %d fail, error %d:%s\n",
                           p_fifo->aging_index_array[i], ret, ctc_get_error_desc(ret));
            }
        }
    }
    else
    {
        /*Using Dma*/
        for (i = 0; i < p_fifo->fifo_idx_num; i++)
        {
            p_entry = &(p_fifo->aging_entry[i]);
            if (p_entry->aging_type == CTC_AGING_TYPE_MAC)
            {
                sal_memset(&l2_addr, 0, sizeof(ctc_l2_addr_t));

                l2_addr.fid = p_entry->fid;
                sal_memcpy(l2_addr.mac, p_entry->mac, sizeof(mac_addr_t));

                CTC_SET_FLAG(l2_addr.flag, CTC_L2_FLAG_REMOVE_DYNAMIC);

                if (!p_entry->is_hw_sync)
                {
                    ret = ctcs_l2_remove_fdb(lchip, &l2_addr);
                }
                else
                {
                    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Aging Sync: MAC:%.4x.%.4x.%.4x  FID:%-5d\n",
                                                        sal_ntohs(*(unsigned short*)&l2_addr.mac[0]),
                                                        sal_ntohs(*(unsigned short*)&l2_addr.mac[2]),
                                                        sal_ntohs(*(unsigned short*)&l2_addr.mac[4]),
                                                        l2_addr.fid);
                }

            }
            else if (p_entry->aging_type == CTC_AGING_TYPE_IPV4_UCAST)
            {
                uint32 tempip = sal_ntohl(p_entry->ip_da.ipv4);

                sal_memset(&ipuc_param, 0, sizeof(ctc_ipuc_param_t));

                sal_inet_ntop(AF_INET, &tempip, buf, CTC_IPV6_ADDR_STR_LEN);

                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Aging IPV4:%-30s  VRFID:%-5d  masklen:32\n", buf, p_entry->vrf_id);

                ipuc_param.ip.ipv4 = p_entry->ip_da.ipv4;
                ipuc_param.vrf_id = p_entry->vrf_id;
                ipuc_param.masklen = 32;
                ipuc_param.ip_ver = CTC_IP_VER_4;
                ret = ctcs_ipuc_remove(lchip, &ipuc_param);
            }
            else if (p_entry->aging_type == CTC_AGING_TYPE_IPV6_UCAST)
            {
                uint32 ipv6_address[4] = {0, 0, 0, 0};
                sal_memset(&ipuc_param, 0, sizeof(ctc_ipuc_param_t));

                ipv6_address[0] = sal_ntohl(p_entry->ip_da.ipv6[0]);
                ipv6_address[1] = sal_ntohl(p_entry->ip_da.ipv6[1]);
                ipv6_address[2] = sal_ntohl(p_entry->ip_da.ipv6[2]);
                ipv6_address[3] = sal_ntohl(p_entry->ip_da.ipv6[3]);

                sal_inet_ntop(AF_INET6, ipv6_address, buf, CTC_IPV6_ADDR_STR_LEN);
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Aging IPV6:%-44s  VRFID:%-5d  masklen:128\n", buf, p_entry->vrf_id);

                sal_memcpy(&(ipuc_param.ip.ipv6), &(p_entry->ip_da.ipv6), sizeof(ipuc_param.ip.ipv6));
                ipuc_param.vrf_id = p_entry->vrf_id;
                ipuc_param.masklen = 128;
                ipuc_param.ip_ver = CTC_IP_VER_6;
                ret = ctcs_ipuc_remove(lchip, &ipuc_param);
            }

            if (ret)
            {
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Aging fdb fail, ret:%d \n", ret);
            }
        }
    }
    return CTC_E_NONE;
}

char*
ctcs_app_intr_abnormal_action_str(ctc_interrupt_fatal_intr_action_t action)
{
    switch (action)
    {
    case CTC_INTERRUPT_FATAL_INTR_NULL:
        return "Ignore";
    case CTC_INTERRUPT_FATAL_INTR_LOG:
        return "Log";
    case CTC_INTERRUPT_FATAL_INTR_RESET:
        return "Reset Chip";
    default:
        return "Unknown";
    }
}

int32
ctcs_app_isr_chip_abnormal_isr(uint8 gchip, void* p_data)
{
    ctc_interrupt_abnormal_status_t* p_status = NULL;
    char action_str[20];

    sal_memset(action_str, 0, sizeof(action_str));
    CTC_PTR_VALID_CHECK(p_data);
    p_status = (ctc_interrupt_abnormal_status_t*)p_data;

    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "### Detect Chip Abnormal Interrupt ###\n");
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Type          :   %d\n", p_status->type.intr);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Sub Type      :   %d\n", p_status->type.sub_intr);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Low Type      :   %d\n", p_status->type.low_intr);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Status Bits   :   %d\n", p_status->status.bit_count);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Status        :   %08X %08X %08X \n",
        p_status->status.bmp[0], p_status->status.bmp[1], p_status->status.bmp[2]);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Action        :   %s\n", ctcs_app_intr_abnormal_action_str(p_status->action));

    return CTC_E_NONE;
}

int32
ctcs_app_isr_port_link_isr(uint8 gchip, void* p_data)
{
    ctc_port_link_status_t* p_link = NULL;
    uint16 gport = 0;
    uint8 lchip = 0;
    bool is_link = FALSE;

    p_link = (ctc_port_link_status_t*)p_data;
    gport = p_link->gport;

    ctc_app_get_lchip_id(gchip, &lchip);
    ctcs_port_get_mac_link_up(lchip, gport, &is_link);
    if (is_link)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport 0x%04X Link Up, Port is enabled! \n", gport);
    }
    else
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport 0x%04X Link Down, Port is disabled, please do port enable when linkup again! \n", gport);
    }

    /*Step1: Disable Port*/

    /*Do other thing for link down*/

    return CTC_E_NONE;
}

int32
ctcs_app_isr_ecc_error_isr(uint8 gchip, void* p_data)
{
    uint8 err_len = 0;
    char* ecc_error_type[] = {"EccSingleBitError", "EccMultipleBitError", "ParityError", "TcamError", "Other"};
    char* ecc_error_action[] = {"NoAction", "Log", "ChipReset", "PortReset"};

    ctc_interrupt_ecc_t* p_ecc = NULL;
    char action_str[20];
    char* p_time_str = NULL;
    sal_time_t tm;

    sal_memset(action_str, 0, sizeof(action_str));
    CTC_PTR_VALID_CHECK(p_data);
    p_ecc = (ctc_interrupt_ecc_t*)p_data;

    sal_time(&tm);
    p_time_str = sal_ctime(&tm);

    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " Error happen   :   %s", p_time_str);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " ------------------------------------------- \n");

    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " ErrType        :   %s\n",
                   (p_ecc->type >= (sizeof(ecc_error_type)/sizeof(char*)))
                   ? "Unknown" : ecc_error_type[p_ecc->type]);

    if (0xFC000000 == (p_ecc->tbl_id & 0xFF000000))
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                        " IntrTblId      :   %u\n",(p_ecc->tbl_id & 0xFFFFFF));
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                        " IntrFldId      :   %u\n", p_ecc->tbl_index);
    }
    else if (0xFD000000 == (p_ecc->tbl_id & 0xFF000000))
    {
        err_len = (p_ecc->tbl_id >> 16) & 0xFF;
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                        " TcamAdRamId    :   %u\n",(p_ecc->tbl_id & 0xFFFF));
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                        " ErrOffset      :   0x%08X\n", p_ecc->tbl_index);
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                        " ErrLength      :   %u(Byte)\n", err_len);
    }
    else if (0xFE000000 == (p_ecc->tbl_id & 0xFF000000))
    {
        err_len = (p_ecc->tbl_id >> 20) & 0xF;
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                        " TcamKeyRamId   :   %u\n", (p_ecc->tbl_id & 0xFFFFF));
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                        " ErrOffset      :   0x%08X\n", p_ecc->tbl_index);
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                        " ErrLength      :   %u(Byte)\n", err_len);
    }
    else if (0xFF000000 == (p_ecc->tbl_id & 0xFF000000))
    {
        err_len = (p_ecc->tbl_id >> 16) & 0xFF;
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                        " DynamicRamId   :   %u\n", (p_ecc->tbl_id & 0xFFFF));
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                        " ErrOffset      :   0x%08X\n", p_ecc->tbl_index);
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                        " ErrLength      :   %u(Byte)\n", err_len);
    }
    else
    {
        if (CTC_INTERRUPT_ECC_TYPE_SBE == p_ecc->type)
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                            " TableID        :   %u\n", p_ecc->tbl_id);
        }
        else
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                            " TableID        :   %u\n", p_ecc->tbl_id);
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                            " TableIndex     :   %u\n", p_ecc->tbl_index);
        }
    }

    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " ErrAction      :   %s\n",
                   (p_ecc->action >= (sizeof(ecc_error_action)/sizeof(char*)))
                   ? "Unknown" : ecc_error_action[p_ecc->action]);

    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " ErrRecover     :   %s\n",
                    p_ecc->recover ? "Yes" : "No");
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    return CTC_E_NONE;
}

int32
ctcs_app_isr_ipfix_overflow_isr(uint8 gchip, void* p_data)
{
    ctc_ipfix_event_t* p_event = (ctc_ipfix_event_t*)p_data;
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Ipfix overflow happend\n");
    if(p_event->exceed_threshold)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "    flow usage exceed threshold");
    }
    else
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "    flow usage down to threshold");
    }
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    return CTC_E_NONE;
}

/****************************************************************************
 *
* Function
*
*****************************************************************************/

int32
ctcs_app_isr_init(uint8 lchip)
{
    /* 1. register event callback */
    ctcs_interrupt_register_event_cb(lchip, CTC_EVENT_L2_SW_LEARNING,    ctcs_app_isr_sw_learning_process_isr);
    ctcs_interrupt_register_event_cb(lchip, CTC_EVENT_L2_SW_AGING,       ctcs_app_isr_sw_aging_process_isr);
    ctcs_interrupt_register_event_cb(lchip, CTC_EVENT_ABNORMAL_INTR,     ctcs_app_isr_chip_abnormal_isr);
    ctcs_interrupt_register_event_cb(lchip, CTC_EVENT_PORT_LINK_CHANGE,  ctcs_app_isr_port_link_isr);
    ctcs_interrupt_register_event_cb(lchip, CTC_EVENT_ECC_ERROR,         ctcs_app_isr_ecc_error_isr);

    ctcs_interrupt_register_event_cb(lchip, CTC_EVENT_IPFIX_OVERFLOW,    ctcs_app_isr_ipfix_overflow_isr);

    return CTC_E_NONE;
}

