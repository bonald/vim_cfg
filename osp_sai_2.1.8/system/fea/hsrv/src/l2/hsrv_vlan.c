/****************************************************************************
* $Id$
*  Centec switch information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date          : 2015-07-10
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "hsrv_stp.h"
#include "gen/tbl_igsp_global_define.h"
#include "gen/tbl_igsp_global.h"
#include "gen/tbl_igsp_intf_define.h"
#include "gen/tbl_igsp_intf.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_instance.h"
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_global.h"
#include "gen/tbl_openflow_define.h"
#include "gen/tbl_openflow.h"
#include <ctc_sai_common_attr.h>
#include <ctc_sai_acl.h>


/******************************************************************************
* Global and Declaration
******************************************************************************/

/******************************************************************************
* Defines and Macros
******************************************************************************/

int32
hsrv_vlan_check_port_in_enable_vlan(vid_t vid, uint32 ifindex, uint32 *is_set)
{
    tbl_vlan_key_t key;
    tbl_vlan_t *p_db_vlan = NULL;

    key.vid = vid;
    p_db_vlan = tbl_vlan_get_vlan(&key);
    if (NULL == p_db_vlan)
    {
        *is_set = FALSE;
        return HSRV_E_NONE;
    }

    if (!p_db_vlan->enable)
    {
        *is_set = FALSE;
        return HSRV_E_NONE;
    }

    if (GLB_BMP_ISSET(p_db_vlan->member_port, ifindex))
    {
        *is_set = TRUE;
    }

    return HSRV_E_NONE;
}

int32
hsrv_vlan_update_include_vlan_hybrid(uint16 vid, uint8 stpid)
{
#ifdef OFPRODUCT
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();

    if (p_db_glb->of_exclude_all_vlan)
    {
        if (!GLB_BMP_ISSET(p_db_glb->of_include_vlan_bmp, vid))
        {
            ctc_stp_set_vlan_stpid(vid, stpid);
        }
    }
#endif /* OFPRODUCT */

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vlan_creat_vlan
 * Purpose      : creat vlan           
 * Input        : vid, VLAN ID
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_vlan_creat_vlan(uint16 vid)
{
    sai_vlan_api_t*  vlan_api = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN,(void**)&vlan_api));
    HSRV_IF_ERROR_RETURN(vlan_api->create_vlan(vid));
    HSRV_IF_ERROR_RETURN(hsrv_fdb_vlan_add(vid));

    hsrv_vlan_update_include_vlan_hybrid(vid, HSRV_OF_INSTANCE);
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vlan_delete_vlan
 * Purpose      : delete vlan           
 * Input        : vid, VLAN ID
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_vlan_delete_vlan(uint16 vid)
{
    sai_vlan_api_t*  vlan_api = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN,(void**)&vlan_api));
#ifdef OFPRODUCT
    tbl_openflow_t  *p_db_openflow = tbl_openflow_get_openflow();

    /* if hybrid extend mac table valid, we don't really remove vlan here, disable learning instead */
    if (p_db_openflow->fdb_flow_valid)
    {
        sai_attribute_t attr[1];
        attr[0].id = SAI_VLAN_ATTR_LEARN_DISABLE;
        attr[0].value.booldata = TRUE;

        vlan_api->set_vlan_attribute(vid, attr);
    }

    if (!p_db_openflow->fdb_flow_valid)
#endif
        HSRV_IF_ERROR_RETURN(vlan_api->remove_vlan(vid));
    HSRV_IF_ERROR_RETURN(hsrv_fdb_vlan_del(vid));
    
    hsrv_vlan_update_include_vlan_hybrid(vid, HSRV_OF_INSTANCE);
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vlan_set_stp_instance
 * Purpose      : set VLAN instance ID          
 * Input        : vid, VLAN ID
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_vlan_set_stp_instance(uint16 vid, uint32 instanceid)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    sai_vlan_api_t*  vlan_api = NULL;
    sai_attribute_t attr[1];

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN,(void**)&vlan_api));

    inst_key.instance = instanceid;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (NULL == p_db_inst)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_VLAN_ATTR_STP_INSTANCE;
    attr[0].value.oid = p_db_inst->instance_oid;
    HSRV_IF_ERROR_RETURN(vlan_api->set_vlan_attribute(vid, attr));

    hsrv_vlan_update_include_vlan_hybrid(vid, HSRV_OF_INSTANCE);

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vlan_add_port_to_vlan
 * Purpose      : add port to VLAN, (1)for VLAN ingrss filter;(2)for VLAN default flooding entry         
 * Input        : vid: VLAN ID;
                  tagmode: untag mode, tagged mode, ptag mode;
                  portid: portid created from SAI;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_vlan_add_port_to_vlan(uint16 vid, uint16 tagmode, uint64 portid)
{
    sai_vlan_api_t*  vlan_api = NULL;
    sai_vlan_port_t vlan_port[1];

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN,(void**)&vlan_api));

    vlan_port[0].tagging_mode = tagmode;
    vlan_port[0].port_id = portid;
    HSRV_IF_ERROR_RETURN(vlan_api->add_ports_to_vlan(vid,1, vlan_port));
    
    return HSRV_E_NONE;
}


/***************************************************************************************************
 * Name         : hsrv_vlan_add_ports_to_vlan
 * Purpose      : add all vlan member port when delete hybrid fdb flow table default entry       
 * Input        : N/A  
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_vlan_add_ports_to_vlan()
{
    tbl_vlan_master_t *p_master = tbl_vlan_get_master();
    tbl_vlan_t        *p_db_vlan = NULL;
    tbl_interface_t   *p_mem_if = NULL;
    ds_brgif_t        *p_brgif = NULL;
    uint32 vid = 0;
    int32  ifindex = 0;
    uint16 tagging_mode = 0;
    sai_object_id_t oid = 0;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    for (vid = 1; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_vlan = p_master->vlan_array[vid];
        if (NULL == p_db_vlan)
        {
            continue;
        }

        GLB_BMP_ITER_BEGIN(p_db_vlan->member_port, ifindex)
        {
            p_mem_if = tbl_interface_get_interface_by_ifindex(ifindex);
            if (NULL == p_mem_if)
            {
                continue;
            }

            p_brgif = p_mem_if->brgif;
            if (NULL == p_brgif)
            {
                continue;
            }

            HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_mem_if->ifindex, &oid));
            if (p_mem_if->vlan_type == GLB_VLAN_PORT_TYPE_ACCESS)
            {
                tagging_mode = SAI_VLAN_PORT_UNTAGGED;
            }
            else if (p_mem_if->vlan_type == GLB_VLAN_PORT_TYPE_TRUNK)
            {
                if (GLB_BMP_ISSET(p_brgif->untagged_vlan.vlan_bmp, p_db_vlan->key.vid))
                {
                    tagging_mode = SAI_VLAN_PORT_UNTAGGED;
                }
                else
                {
                    tagging_mode = SAI_VLAN_PORT_TAGGED;
                }
            }
            else if (p_mem_if->vlan_type == GLB_VLAN_PORT_TYPE_QINQ)
            {
                tagging_mode = SAI_VLAN_PORT_TAGGED;
            }
            else
            {
                continue;
            }

            hsrv_vlan_add_port_to_vlan(p_db_vlan->key.vid, tagging_mode, oid);
        }
        GLB_BMP_ITER_END(p_db_vlan->member_port, ifindex);
    }

    return rc;
}


/***************************************************************************************************
 * Name         : hsrv_vlan_remove_port_from_vlan
 * Purpose      : remove port from VLAN, (1)for VLAN ingrss filter;(2)for VLAN default flooding entry         
 * Input        : vid: VLAN ID;
                  tagmode: untag mode, tagged mode, ptag mode;
                  portid: portid created from SAI;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_vlan_remove_port_from_vlan(uint16 vid, uint16 tagmode, uint64 portid)
{
    sai_vlan_api_t*  vlan_api = NULL;
    sai_vlan_port_t vlan_port[1];

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN,(void**)&vlan_api));
    vlan_port[0].port_id = portid;
    HSRV_IF_ERROR_RETURN(vlan_api->remove_ports_from_vlan(vid, 1, vlan_port));
    
    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_vlan_process_member_ports
 * Purpose      : process VLAN member port change.       
 * Input        : p_vlan, current table vlan data;
                  p_db_vlan, original table vlan data;
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_vlan_process_member_port(tbl_vlan_t *p_vlan)
{
    tbl_interface_t* p_if = NULL;
    ds_brgif_t* p_brgif = NULL;
    uint16 tagging_mode = 0;
    sai_object_id_t oid = 0;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);


    if ((p_vlan->op_ifindex >> 32) == GLB_VLAN_OP_ACTION_ADD)
    {
        /*add port to vlan*/
        p_if = tbl_interface_get_interface_by_ifindex(p_vlan->op_ifindex&0xffffffff);
        HSRV_PTR_CHECK(p_if);

        if ((p_if->dot1x_guest_vlan)
            && (!GLB_FLAG_ISSET(p_if->dot1x_flag, GLB_IF_DOT1X_MODE_MAC)))
        {
            return HSRV_E_NONE;   
        }

        p_brgif = p_if->brgif;
        if (NULL == p_brgif)
        {
            return HSRV_E_NONE;
        }
        
        /* added by yejl for bug 40393 (and 37364), 2016-08-30 */
        if (!(tbl_interface_is_running(p_if)))
        {
            if (p_if->vlan_type == GLB_VLAN_PORT_TYPE_ACCESS)
            {
                if (p_vlan->key.vid == p_brgif->pvid)
                {
                    return HSRV_E_NONE;
                }
            }
            else if (p_if->vlan_type == GLB_VLAN_PORT_TYPE_TRUNK)
            {
                if (p_vlan->key.vid == p_brgif->pvid)
                {
                    return HSRV_E_NONE;
                }
            }     
        }
#if 0
        /* modified by yejl for bug 37364, 2016-06-13 */
        if ((p_vlan->key.vid == GLB_DEF_VLAN_ID) && !(tbl_interface_is_running(p_if)))
        {
            return HSRV_E_NONE;
        }
#endif
        /* modified by yejl for bug 39334, 2016-07-18 */
        if (GLB_FLAG_ISSET(p_if->mirror_enable, MIRROR_DEST_FLAG))
        {
            return HSRV_E_NONE;
        }
        
        HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_if->ifindex, &oid));

        /*1, for GG, we don't need consider AGG interface, SAI load agg member to port and default entry */
        /*2, for GB, we have to consider AGG interface, SAI load agg member to port and load agg if to default entry*/

        if (p_if->vlan_type == GLB_VLAN_PORT_TYPE_ACCESS)
        {
            tagging_mode = SAI_VLAN_PORT_UNTAGGED;
        }
        else if (p_if->vlan_type == GLB_VLAN_PORT_TYPE_TRUNK)
        {
            if (GLB_BMP_ISSET(p_brgif->untagged_vlan.vlan_bmp, p_vlan->key.vid))
            {
                tagging_mode = SAI_VLAN_PORT_UNTAGGED;
            }
            else
            {
                tagging_mode = SAI_VLAN_PORT_TAGGED;
            }
        }
        else if (p_if->vlan_type == GLB_VLAN_PORT_TYPE_QINQ)
        {
            tagging_mode = SAI_VLAN_PORT_TAGGED;
        }
        else
        {
            return HSRV_E_INVALID_PARAM;
        }

        HSRV_IF_ERROR_RETURN(hsrv_vlan_add_port_to_vlan(p_vlan->key.vid, tagging_mode, oid));
    }
    else if ((p_vlan->op_ifindex >> 32) == GLB_VLAN_OP_ACTION_REMOVE)
    {
        /*delete port from Vlan*/
        p_if = tbl_interface_get_interface_by_ifindex((p_vlan->op_ifindex&0xffffffff));
        HSRV_PTR_CHECK(p_if);

        /* modified by liwh for bug 38543, 2016-05-17 */
        if ((p_if->dot1x_guest_vlan)
            && ((!GLB_FLAG_ISSET(p_if->dot1x_flag, GLB_IF_DOT1X_MODE_MAC))
              || (GLB_FLAG_ISSET(p_if->dot1x_flag, GLB_IF_DOT1X_MODE_MAC)
                && (p_if->dot1x_guest_vlan == p_vlan->key.vid))))
        {
            return HSRV_E_NONE;   
        }
        /* liwh end */

        HSRV_IF_ERROR_RETURN(hsrv_interface_ifindex_to_objectid(p_if->ifindex, &oid));

        HSRV_IF_ERROR_RETURN(hsrv_vlan_remove_port_from_vlan(p_vlan->key.vid,tagging_mode, oid));
    }

    return HSRV_E_NONE;
}

static int32
_hsrv_vlan_get_dhcp_excp_type(uint32 glb_type, uint32* hagt_type)
{
    switch(glb_type)
    {
        case GLB_DHCPPKT_FORWORD_CPU:
            *hagt_type = CTC_EXCP_FWD_AND_TO_CPU;
            break;
            
        case GLB_DHCPPKT_FORWORD:
            *hagt_type = CTC_EXCP_NORMAL_FWD;
            break;
            
        case GLB_DHCPPKT_CPU:
            *hagt_type = CTC_EXCP_DISCARD_AND_TO_CPU;
            break;
            
        case GLB_DHCPPKT_DISGARD:
            *hagt_type = CTC_EXCP_DISCARD;
            break;

        default:
            HSRV_LOG_ERR("Sync vlan error: Invalid dhcp exception type %d", glb_type);
            return HSRV_E_INVALID_PARAM;
    }
    return HSRV_E_NONE;
}

int32
hsrv_vlan_get_dhcp_excp_type(uint32 flag, uint32* p_excp_type)
{
    if (GLB_FLAG_ISSET(flag, HSRV_VLAN_FLAG_DHCP_SNOOPING_EN))
    {
        /*always forward to cpu*/
        *p_excp_type = GLB_DHCPPKT_CPU;
    }
    else if ( GLB_FLAG_ISSET(flag, HSRV_VLAN_FLAG_DHCP_RELAY_EN))
    {
        /* modified by liwh for bug 50528, 2018-12-26 
             for vlanif, dhcp pkt should be sent to CPU and discard, according to E series */
        *p_excp_type = GLB_DHCPPKT_CPU;
        /* liwh end */
    }
    else
    {
        /*only do normal forward*/
        *p_excp_type = GLB_DHCPPKT_FORWORD;
    }

    return HSRV_E_NONE;
}

static int32
_hsrv_vlan_get_arp_excp_type(uint32 glb_type, uint32* hagt_type)
{
    switch(glb_type)
    {
        case GLB_ARPPKT_FORWORD_CPU:
            *hagt_type = CTC_EXCP_FWD_AND_TO_CPU;
            break;
            
        case GLB_ARPPKT_FORWORD:
            *hagt_type = CTC_EXCP_NORMAL_FWD;
            break;
            
        case GLB_ARPPKT_CPU:
            *hagt_type = CTC_EXCP_DISCARD_AND_TO_CPU;
            break;
            
        case GLB_ARPPKT_DISGARD:
            *hagt_type = CTC_EXCP_DISCARD;
            break;

        default:
            HSRV_LOG_ERR("Sync vlan error: Invalid arp exception type %d", glb_type);
            return HSRV_E_INVALID_PARAM;
    }
    return HSRV_E_NONE;
}

int32
hsrv_vlan_get_arp_excp_type(uint32 flag, uint32* p_excp_type)
{
    if (GLB_FLAG_ISSET(flag, HSRV_VLAN_FLAG_ARP_SNOOPING_ENABLE))
    {
        /*always forward to cpu*/
        *p_excp_type = GLB_ARPPKT_CPU;
    }
    else if ( GLB_FLAG_ISSET(flag, HSRV_VLAN_FLAG_VLANIF_ENABLE))
    {
        /* for unicast pkt should be forword in vlan when only l3 dhcp was enabled*/
        *p_excp_type = GLB_ARPPKT_FORWORD_CPU;
    }
    else
    {
        /*only do normal forward*/
        *p_excp_type = GLB_ARPPKT_FORWORD;
    }

    return HSRV_E_NONE;
}


int32
hsrv_vlan_set_dhcp_excp_type(uint16 vlan_id, uint32 flag)
{
    uint32          dhcp_excp_type  = 0;
    uint32          sai_dhcp_excp   = 0;
    sai_vlan_api_t  *vlan_api       = NULL;
    sai_attribute_t attr[1];

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN,(void**)&vlan_api));

    /*3. get dhcp exception type*/
    hsrv_vlan_get_dhcp_excp_type(flag, &dhcp_excp_type);
    _hsrv_vlan_get_dhcp_excp_type(dhcp_excp_type, &sai_dhcp_excp);

    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_VLAN_ATTR_DHCPSNOOPING;
    attr[0].value.u32 = sai_dhcp_excp;
    HSRV_IF_ERROR_RETURN(vlan_api->set_vlan_attribute(vlan_id, attr));
    
    return HSRV_E_NONE;
}

int32
hsrv_vlan_set_arp_excp_type(uint16 vlan_id, uint32 flag)
{
    uint32          arp_excp_type  = 0;
    uint32          sai_arp_excp   = 0;
    sai_vlan_api_t  *vlan_api       = NULL;
    sai_attribute_t attr[1];

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN,(void**)&vlan_api));

    /*3. get dhcp exception type*/
    hsrv_vlan_get_arp_excp_type(flag, &arp_excp_type);
    _hsrv_vlan_get_arp_excp_type(arp_excp_type, &sai_arp_excp);

    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_VLAN_ATTR_ARPSNOOPING;
    attr[0].value.u32 = sai_arp_excp;
    HSRV_IF_ERROR_RETURN(vlan_api->set_vlan_attribute(vlan_id, attr));
    
    return HSRV_E_NONE;
}

int32
hsrv_vlan_get_vlan_stats(tbl_vlan_stats_t* p_db_vlan_stats)
{
    sai_vlan_api_t  *vlan_api       = NULL;
    sai_attribute_t attr[8];
    uint16 filed_id = 0;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN,(void**)&vlan_api));

    attr[0].id = SAI_VLAN_ATTR_L2_IN_PACKET;
    attr[1].id = SAI_VLAN_ATTR_L2_IN_BYTE;
    attr[2].id = SAI_VLAN_ATTR_L2_OUT_PACKET;
    attr[3].id = SAI_VLAN_ATTR_L2_OUT_BYTE;
    attr[4].id = SAI_VLAN_ATTR_L3_IN_PACKET;
    attr[5].id = SAI_VLAN_ATTR_L3_IN_BYTE;
    attr[6].id = SAI_VLAN_ATTR_L3_OUT_PACKET;
    attr[7].id = SAI_VLAN_ATTR_L3_OUT_BYTE;
    
    
    HSRV_IF_ERROR_RETURN(vlan_api->get_vlan_stats_info(p_db_vlan_stats->key.vid, attr));

    p_db_vlan_stats->pktcntl2in = attr[0].value.u64;
    p_db_vlan_stats->bytecntl2in = attr[1].value.u64;
    p_db_vlan_stats->pktcntl2out = attr[2].value.u64;
    p_db_vlan_stats->bytecntl2out = attr[3].value.u64;
    p_db_vlan_stats->pktcntl3in = attr[4].value.u64;
    p_db_vlan_stats->bytecntl3in = attr[5].value.u64;
    p_db_vlan_stats->pktcntl3out = attr[6].value.u64;
    p_db_vlan_stats->bytecntl3out = attr[7].value.u64;

    /*sync statistics to ncs for netconf*/
    for (filed_id = 0; filed_id < TBL_VLAN_STATS_FLD_MAX; filed_id++)
    {
        cdb_sync_tbl_set(TBL_VLAN_STATS, filed_id, p_db_vlan_stats);
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_vlan_clear_vlan_stats(vid_t vid)
{
    sai_vlan_api_t  *vlan_api       = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN,(void**)&vlan_api));

    HSRV_IF_ERROR_RETURN(vlan_api->clear_vlan_stats(vid));

    return HSRV_E_NONE;

}

static int32
_hsrv_vlan_stats_show(tbl_vlan_stats_t *p_db_vlan_stats, FILE *fp)
{
    uint64 inpkt = 0;
    uint64 inbyte = 0;
    uint64 outpkt = 0;
    uint64 outbyte = 0;
    HSRV_IF_ERROR_RETURN(hsrv_vlan_get_vlan_stats(p_db_vlan_stats));

    inpkt = p_db_vlan_stats->pktcntl2in + p_db_vlan_stats->pktcntl3in;
    inbyte = p_db_vlan_stats->bytecntl2in + p_db_vlan_stats->bytecntl3in;
    outpkt = p_db_vlan_stats->pktcntl2out + p_db_vlan_stats->pktcntl3out;
    outbyte = p_db_vlan_stats->bytecntl2out + p_db_vlan_stats->bytecntl3out;


    sal_fprintf (fp, "%-6s%d\n", "VLAN: ", p_db_vlan_stats->key.vid); 
    sal_fprintf (fp, "%s\n", "------------+---------------------+-----------------------------");
    sal_fprintf (fp, "%-20s%-22s%-22s\n", "Item", "Packets", "Bytes"); 
    sal_fprintf (fp, "%s\n", "----------------------------------------------------------------");
    sal_fprintf (fp, "%-20s%-22"PRIu64"%-22"PRIu64"\n", "Inbound:", inpkt, inbyte); 
    sal_fprintf (fp, "%-20s%-22"PRIu64"%-22"PRIu64"\n", "Outbound:", outpkt, outbyte); 
    sal_fprintf (fp, "%s\n", "----------------------------------------------------------------");

    return PM_E_NONE;
            
}

static int32
_hsrv_vlan_stats_dump(tbl_vlan_stats_t *p_db_vlan_stats, FILE *fp)
{
    tbl_iter_args_t args;
    field_parse_t field;

    sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = &field;
    args.argv[1] = fp;
    
    if (!p_db_vlan_stats)
    {
        tbl_vlan_stats_iterate((TBL_ITER_CB_FUNC)tbl_vlan_stats_dump_one, &args);
    }
    else
    {
        tbl_vlan_stats_dump_one(p_db_vlan_stats, &args);
    }

    return PM_E_NONE;
}

int32
hsrv_vlan_stats_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    vid_t vid = 0;
    tbl_vlan_key_t key;
    tbl_vlan_t* p_db_vlan = NULL;
    tbl_vlan_stats_t* p_db_vlan_stats = NULL;
    uint32 is_dump = FALSE;
    tbl_vlan_stats_key_t vlan_stats_key;
    FILE *fp = NULL;
    int32 rc = HSRV_E_NONE;

    switch (para->oper)
    {
        case CDB_OPER_SET:
            PM_CFG_GET_UINT(vid, argv[0], para->p_rs);
            key.vid = vid;
            p_db_vlan = tbl_vlan_get_vlan(&key);
            if (NULL == p_db_vlan)
            {
                CFG_CONFLICT_RET("Vlan %u dose not exist", vid);
            }
            
            vlan_stats_key.vid = vid;
            p_db_vlan_stats = tbl_vlan_stats_get_vlan_stats(&vlan_stats_key);
            if (NULL == p_db_vlan_stats)
            {
                CFG_CONFLICT_RET("Vlan %u statistics not be enabled", vid);
            }
            
            HSRV_IF_ERROR_RETURN(hsrv_vlan_clear_vlan_stats(vid));
                  
            p_db_vlan_stats->pktcntl2in = 0;
            p_db_vlan_stats->pktcntl2out = 0;
            p_db_vlan_stats->bytecntl2in = 0;
            p_db_vlan_stats->bytecntl2out = 0;
            p_db_vlan_stats->pktcntl3in = 0;
            p_db_vlan_stats->pktcntl3out = 0;
            p_db_vlan_stats->bytecntl3in = 0;
            p_db_vlan_stats->bytecntl3out = 0;
            
        case CDB_OPER_GET:
            if (0 == argc)
            {
                vid = GLB_ALL_VLAN_ID;
                is_dump = TRUE;
            }
            else if (1 <= argc)
            {
                is_dump = (1 == argc) ? TRUE : FALSE;
                PM_CFG_GET_UINT(vid, argv[0], para->p_rs);
                key.vid = vid;
                p_db_vlan = tbl_vlan_get_vlan(&key);
                if (!p_db_vlan)
                {
                    CFG_CONFLICT_RET("Vlan %u does not exist", vid);
                }
                vlan_stats_key.vid = vid;
                p_db_vlan_stats = tbl_vlan_stats_get_vlan_stats(&vlan_stats_key);
                if (!p_db_vlan_stats)
                {
                    CFG_CONFLICT_RET("Vlan %u statistics not be enabled", vid);
                }
            }

            CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
            fp = sal_fopen(para->p_show->path, "w+");
            if (is_dump)
            {
                rc = _hsrv_vlan_stats_dump(p_db_vlan_stats, fp);
            }
            else
            {
                rc = _hsrv_vlan_stats_show(p_db_vlan_stats, fp);
            }
            sal_fclose(fp);
            break;
        
        default:
            break;
                
    }

    return rc;
}

int32
hsrv_vlan_set_stats_en(tbl_vlan_t *p_vlan)
{
    sai_vlan_api_t  *vlan_api       = NULL;
    sai_attribute_t attr[1];
    tbl_rif_t rif;
    tbl_rif_t *p_rif = NULL;
    tbl_vlan_stats_t vlan_stats;
    tbl_vlan_stats_key_t  vlan_stats_key;
    uint32 l3if_id = 0;
    uint32 u32_list[2];
    uint32 enable = 0;
    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN,(void**)&vlan_api));
    
    sal_memset(&vlan_stats, 0, sizeof(tbl_vlan_stats_t));
    enable = p_vlan->stats_en;
    vlan_stats_key.vid = p_vlan->key.vid;
    vlan_stats.key.vid = p_vlan->key.vid;
    if (enable)
    {      
        tbl_vlan_stats_add_vlan_stats(&vlan_stats);
    }
    else
    {
        tbl_vlan_stats_del_vlan_stats(&vlan_stats_key);
    }
    sal_memset(&rif, 0, sizeof(tbl_rif_t));
    sal_sprintf(rif.key.name, "vlan%u", p_vlan->key.vid);
    
    p_rif = tbl_rif_get_rif(&rif.key);
    if (p_rif)
    {
        l3if_id = (uint32)p_rif->rif_id;
    }

    u32_list[0] = enable;
    u32_list[1] = l3if_id;
    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_VLAN_ATTR_STATS_EN;
    attr[0].value.u32list.list = u32_list;
    HSRV_IF_ERROR_RETURN(vlan_api->set_vlan_attribute(p_vlan->key.vid, attr));
    
    return HSRV_E_NONE;

}

/***************************************************************************************************
 * Name         : hsrv_vlan_sync
 * Purpose      : load vlan cdb to sai          
 * Input        : p_tbl_node: table node pointer
                  p_ds_node: ds node pointer
                  oper: operation code
                  field_id: field id with operation
                  p_tbl: cdb table pointer
                  p_ds:cdb ds pointer
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_vlan_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_interface_t* p_db_if = NULL;
    tbl_interface_key_t if_key;
    tbl_vlan_key_t* p_vlan_key = NULL;
    tbl_vlan_t  *p_vlan = NULL;
    tbl_vlan_t  *p_db_vlan = NULL;
    uint32      flags = 0;
    /* modified by liwh for bug 51199, 2019-03-05 */
    tbl_route_if_key_t  route_if_key;
    tbl_route_if_t*       p_route_if = NULL;
    /* liwh end */

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    
    switch (oper)
    {
    case CDB_OPER_ADD:
        p_vlan = (tbl_vlan_t*)p_tbl;
        HSRV_IF_ERROR_RETURN(hsrv_vlan_creat_vlan(p_vlan->key.vid));
        break;

    case CDB_OPER_DEL:
        p_vlan_key = (tbl_vlan_key_t*)p_tbl;
        HSRV_IF_ERROR_RETURN(hsrv_vlan_delete_vlan(p_vlan_key->vid));
        break;

    case CDB_OPER_SET:
        p_vlan = (tbl_vlan_t*)p_tbl;
        switch (field_id)
        {
            case TBL_VLAN_FLD_INSTANCE:
                HSRV_IF_ERROR_RETURN(hsrv_vlan_set_stp_instance(p_vlan->key.vid, p_vlan->instance));
                break;

            case TBL_VLAN_FLD_MEMBER_PORT:
                p_db_vlan = tbl_vlan_get_vlan(&p_vlan->key); 
                HSRV_PTR_CHECK(p_db_vlan);
               // HSRV_IF_ERROR_RETURN(hsrv_vlan_process_member_ports(p_vlan, p_db_vlan));
                break;
            case TBL_VLAN_FLD_OP_IFINDEX:
                p_db_vlan = tbl_vlan_get_vlan(&p_vlan->key); 
                HSRV_PTR_CHECK(p_db_vlan);
                HSRV_IF_ERROR_RETURN(hsrv_vlan_process_member_port(p_vlan));
                break;

            case TBL_VLAN_FLD_DHCSNOOPING:
                if (p_vlan->dhcsnooping)
                {
                    flags = HSRV_VLAN_FLAG_DHCP_SNOOPING_EN;
                }

                /* modified by liwh for bug 51199, 2019-03-05 
                    if dhcp relay or client is set for vlanif, dhcp packet should be sent to CPU */
                if (!p_vlan->dhcsnooping)
                {
                    sal_memset(&route_if_key, 0, sizeof(tbl_route_if_key_t));
                    sprintf(route_if_key.name, "vlan%d", p_vlan->key.vid);

                    p_route_if = tbl_route_if_get_route_if(&route_if_key);
                    if (p_route_if && p_route_if->dhcp_pdu_enabled)
                    {
                        flags = HSRV_VLAN_FLAG_DHCP_RELAY_EN;    
                    }
                }
                /* liwh end */
                
                HSRV_IF_ERROR_RETURN(hsrv_vlan_set_dhcp_excp_type(p_vlan->key.vid, flags));
                break;
            case TBL_VLAN_FLD_ARPSNOOPING:
                if (p_vlan->arpsnooping)
                {
                    flags = HSRV_VLAN_FLAG_ARP_SNOOPING_ENABLE;
                }
                else 
                {
                    /*added by yejl to fix bug44070, 2017-05-19*/
                    sal_memset(&if_key, 0, sizeof(if_key));
                    sal_sprintf(if_key.name, "vlan%u", p_vlan->key.vid);
                    p_db_if = tbl_interface_get_interface(&if_key);
                    if (p_db_if)
                    {
                        flags = HSRV_VLAN_FLAG_VLANIF_ENABLE;
                    }
                    /*ended by yejl*/
                }
                HSRV_IF_ERROR_RETURN(hsrv_vlan_set_arp_excp_type(p_vlan->key.vid, flags));
                break;
                
            case TBL_VLAN_FLD_STATS_EN:
                HSRV_IF_ERROR_RETURN(hsrv_vlan_set_stats_en(p_vlan));
                break;
                
            default:
                break;
        }
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

/***************************************************************************************************
 * Name         : hsrv_igsp_intf_set_enable
 * Purpose      : set VLAN instance ID          
 * Input        : vid, VLAN ID
 * Output       : N/A                  
 * Return       : HSRV_E_XXXX
 * Note         : N/A
***************************************************************************************************/
int32
hsrv_igsp_intf_set_enable(uint16 vid, uint32 enable)
{
    sai_vlan_api_t*  vlan_api = NULL;
    sai_attribute_t attr[1];

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN,(void**)&vlan_api));

#ifdef OFPRODUCT
    tbl_igsp_global_t *p_db_glb = tbl_igsp_global_get_igsp_global();
    if (!p_db_glb->host_join_enable)
    {
        return HSRV_E_NONE;
    }
#endif

    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_VLAN_ATTR_IGMP_SNOOPING_EN;
    attr[0].value.booldata = enable;
    HSRV_IF_ERROR_RETURN(vlan_api->set_vlan_attribute(vid, attr));
    
    return HSRV_E_NONE;
}

int32
hsrv_igsp_intf_discard_unknown(uint16 vid, uint32 enable)
{
    sai_vlan_api_t*  vlan_api = NULL;
    sai_attribute_t attr[1];

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN,(void**)&vlan_api));

    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_VLAN_ATTR_IGMP_SNOOPING_DISCARD_UNKNOWN;
    attr[0].value.booldata = enable;
    HSRV_IF_ERROR_RETURN(vlan_api->set_vlan_attribute(vid, attr));
    
    return HSRV_E_NONE;
}

static int32
_hsrv_igsp_intf_add_mrouter_iter_fn(tbl_mcfdb_t *p_db_mcfdb, tbl_iter_args_t *pargs)
{
    tbl_igsp_intf_t* p_igsp_if = NULL;
    uint32 *p_ifindex = NULL;
    uint32 ifindex = 0;

    p_igsp_if = (tbl_igsp_intf_t*)(pargs->argv[0]);
    p_ifindex = (uint32*)(pargs->argv[1]);
    ifindex = *p_ifindex;

    if (p_db_mcfdb->key.fid == p_igsp_if->key.vid)
    {
        /* Added by kcao 2016-09-02 for bug 40578, not duplicate add for port is mcfdb's member port */
        if (!GLB_BMP_ISSET(p_db_mcfdb->l2mc_port, ifindex))
        {
            hsrv_mcfdb_sai_add_member(p_db_mcfdb, ifindex);
        }
    }

    return PM_E_NONE;
}

static int32
_hsrv_igsp_intf_del_mrouter_iter_fn(tbl_mcfdb_t *p_db_mcfdb, tbl_iter_args_t *pargs)
{
    tbl_igsp_intf_t* p_igsp_if = NULL;
    uint32 *p_ifindex = NULL;
    uint32 ifindex = 0;

    p_igsp_if = (tbl_igsp_intf_t*)(pargs->argv[0]);
    p_ifindex = (uint32*)(pargs->argv[1]);
    ifindex = *p_ifindex;

    if (p_db_mcfdb->key.fid == p_igsp_if->key.vid)
    {
        /* Added by kcao 2016-09-02 for bug 40578, not remove for port is mcfdb's member port */
        if (!GLB_BMP_ISSET(p_db_mcfdb->l2mc_port, ifindex))
        {
            hsrv_mcfdb_sai_del_member(p_db_mcfdb, ifindex);
        }
    }

    return PM_E_NONE;
}

int32
_hsrv_igsp_intf_add_mrouter(tbl_igsp_intf_t* p_igsp_if, ds_mrouter_port_t* p_mrouter_port)
{
    tbl_iter_args_t args;
    tbl_interface_t *p_db_if = NULL;
    uint32 ifindex = 0;

    p_db_if = tbl_interface_get_interface_by_name(p_mrouter_port->key.name);
    if (NULL == p_db_if)
    {
        return HSRV_E_NONE;
    }
    ifindex = p_db_if->ifindex;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_igsp_if;
    args.argv[1] = &ifindex;
 
    tbl_mcfdb_iterate((TBL_ITER_CB_FUNC)_hsrv_igsp_intf_add_mrouter_iter_fn, &args);

    return HSRV_E_NONE;
}

int32
_hsrv_igsp_intf_del_mrouter(tbl_igsp_intf_t* p_igsp_if, ds_mrouter_port_t* p_mrouter_port)
{
    tbl_iter_args_t args;
    tbl_interface_t *p_db_if = NULL;
    uint32 ifindex = 0;

    p_db_if = tbl_interface_get_interface_by_name(p_mrouter_port->key.name);
    if (NULL == p_db_if)
    {
        return HSRV_E_NONE;
    }
    ifindex = p_db_if->ifindex;
    
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_igsp_if;
    args.argv[1] = &ifindex;
 
    tbl_mcfdb_iterate((TBL_ITER_CB_FUNC)_hsrv_igsp_intf_del_mrouter_iter_fn, &args);

    return HSRV_E_NONE;
}

int32
hsrv_igsp_intf_sync_mrouter(tbl_igsp_intf_t* p_igsp_if, ds_mrouter_port_t* p_mrouter_port, uint32 oper)
{
    switch (oper)
    {
    case CDB_OPER_ADD:
        _hsrv_igsp_intf_add_mrouter(p_igsp_if, p_mrouter_port);
        break;

    case CDB_OPER_DEL:
        _hsrv_igsp_intf_del_mrouter(p_igsp_if, p_mrouter_port);
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_igsp_mcfdb_mrouter_list_cb(tbl_mcfdb_t *p_mcfdb, uint32 is_add)
{
    ctclib_slistnode_t *node = NULL;
    ctclib_slistnode_t *nnext = NULL;
    tbl_igsp_intf_key_t key;
    tbl_igsp_intf_t *p_db_igsp_if = NULL;
    ds_mrouter_port_t *p_db_mr_port = NULL;
    tbl_interface_t *p_db_if = NULL;

    key.vid = p_mcfdb->key.fid;
    p_db_igsp_if = tbl_igsp_intf_get_igsp_intf(&key);
    if (NULL == p_db_igsp_if)
    {
        return PM_E_NOT_EXIST;
    }

    CTCLIB_SLIST_LOOP_DEL(p_db_igsp_if->mrouter_list.obj_list, p_db_mr_port, node, nnext)
    {
        p_db_if = tbl_interface_get_interface_by_name(p_db_mr_port->key.name);
        if (NULL == p_db_if)
        {
            continue;
        }

        if (is_add)
        {
            hsrv_mcfdb_sai_add_member(p_mcfdb, p_db_if->ifindex);
        }
        else
        {
            hsrv_mcfdb_sai_del_member(p_mcfdb, p_db_if->ifindex);
        }

    }

    return HSRV_E_NONE;
}

uint32 igsp_host_join_acl_id = 0;

int32
hsrv_igsp_global_set_host_join(uint32 enable)
{
#ifdef OFPRODUCT
    tbl_vlan_master_t *p_vlan_master = tbl_vlan_get_master();
    sai_vlan_api_t*  vlan_api = NULL;
    sai_attribute_t attr[1];
    tbl_vlan_t *p_db_vlan = NULL;
    uint32 vid = 0;
    int32 rc = HSRV_E_NONE;
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_VLAN,(void**)&vlan_api));

    sal_memset(attr, 0, sizeof(attr));
    attr[0].id = SAI_VLAN_ATTR_IGMP_SNOOPING_EN;
    attr[0].value.booldata = enable;

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_db_vlan = p_vlan_master->vlan_array[vid];
        if (NULL == p_db_vlan)
        {
            continue;
        }
        HSRV_IF_ERROR_RETURN(vlan_api->set_vlan_attribute(vid, attr));

    }

    ctc_acl_entry_t  acl_entry;
    stbl_acl_glb_t *p_acl_glb = stbl_acl_glb_get_acl_glb();
    /* bug52509 igmp packet local processing have higher priority than flows */
    if (enable)
    {
        sal_memset(&acl_entry, 0, sizeof(acl_entry));
        acl_entry.key.type = CTC_ACL_KEY_IPV4;
        acl_entry.key.u.ipv4_key.key_size = CTC_ACL_KEY_SIZE_DOUBLE;
        acl_entry.priority = SAI_ATTR_ACL_ENTRY_MAXIMUM_PRIORITY;
        acl_entry.priority_valid = TRUE;
#ifdef USW    
        acl_entry.mode = 0;
#endif
        ctclib_opb_alloc_offset(&p_acl_glb->sdk_acl_entry_opb, &igsp_host_join_acl_id);
        acl_entry.entry_id = igsp_host_join_acl_id;
#ifndef GREATBELT
        acl_entry.key.u.ipv4_key.l3_type      = CTC_PARSER_L3_TYPE_IPV4;
        acl_entry.key.u.ipv4_key.l3_type_mask = 0xF;
        CTC_SET_FLAG(acl_entry.key.u.ipv4_key.flag, CTC_ACL_IPV4_KEY_FLAG_L3_TYPE);
#else
        acl_entry.key.u.ipv4_key.eth_type      = 0x0800;
        acl_entry.key.u.ipv4_key.eth_type_mask = 0xFFFF;
        CTC_SET_FLAG(acl_entry.key.u.ipv4_key.flag, CTC_ACL_IPV4_KEY_FLAG_ETH_TYPE);
#endif
        acl_entry.key.u.ipv4_key.l4_protocol = 2;
        acl_entry.key.u.ipv4_key.l4_protocol_mask = 0xF;
        CTC_SET_FLAG(acl_entry.key.u.ipv4_key.flag, CTC_ACL_IPV4_KEY_FLAG_L4_PROTOCOL);

        acl_entry.key.u.ipv4_key.igmp_type = 0x10;
        acl_entry.key.u.ipv4_key.igmp_type_mask = 0xF0;
        CTC_SET_FLAG(acl_entry.key.u.ipv4_key.flag, CTC_ACL_IPV4_KEY_SUB_FLAG_IGMP_TYPE);

        rc = ctc_acl_add_entry(SAI_ACL_WORM_FILTER_GROUP, &acl_entry);
        if (rc)
        {
            ctclib_opb_free_offset(&p_acl_glb->sdk_acl_entry_opb, igsp_host_join_acl_id);
            return rc;
        }

        rc = ctc_acl_install_entry(acl_entry.entry_id);
        if (rc)
        {
            ctclib_opb_free_offset(&p_acl_glb->sdk_acl_entry_opb, igsp_host_join_acl_id);
            return rc;
        }
    }
    else
    {
        ctc_acl_uninstall_entry(igsp_host_join_acl_id);
        ctc_acl_remove_entry(igsp_host_join_acl_id);
        ctclib_opb_free_offset(&p_acl_glb->sdk_acl_entry_opb, igsp_host_join_acl_id);
    }
#endif

    return HSRV_E_NONE;
}

int32
hsrv_igsp_global_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_igsp_global_t *p_igsp_glb = NULL;
    
    switch (oper)
    {
    case CDB_OPER_SET:
        p_igsp_glb = (tbl_igsp_global_t*)p_tbl;
        switch (field_id)
        {
        case TBL_IGSP_GLOBAL_FLD_HOST_JOIN_ENABLE:
            HSRV_IF_ERROR_RETURN(hsrv_igsp_global_set_host_join(p_igsp_glb->host_join_enable));
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}
    
int32
hsrv_igsp_intf_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_igsp_intf_t *p_igsp_if = NULL;
    ds_mrouter_port_t *p_mrouter_port = NULL;

    HSRV_VLAN_DEBUG("Enter into %s.", __FUNCTION__);
    
    switch (oper)
    {
    case CDB_OPER_ADD:
    case CDB_OPER_DEL:
        p_igsp_if = (tbl_igsp_intf_t*)p_tbl;
        if (p_ds_node)
        {
            p_mrouter_port = (ds_mrouter_port_t*)p_ds;
            return hsrv_igsp_intf_sync_mrouter(p_igsp_if, p_mrouter_port, oper);
        }
        break;

    case CDB_OPER_SET:
        p_igsp_if = (tbl_igsp_intf_t*)p_tbl;
        switch (field_id)
        {
        case TBL_IGSP_INTF_FLD_ENABLE:
            HSRV_IF_ERROR_RETURN(hsrv_igsp_intf_set_enable(p_igsp_if->key.vid, p_igsp_if->enable));
            break;

        case TBL_IGSP_INTF_FLD_DISCARD_UNKNOWN_ACT:
            HSRV_IF_ERROR_RETURN(hsrv_igsp_intf_discard_unknown(p_igsp_if->key.vid, p_igsp_if->discard_unknown_act));
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return HSRV_E_NONE;
}
