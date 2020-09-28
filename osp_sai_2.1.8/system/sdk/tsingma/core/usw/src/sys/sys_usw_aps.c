/**
 @file sys_usw_aps.c

 @date 2010-3-11

 @version v2.0

*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_error.h"
#include "ctc_port.h"
#include "ctc_interrupt.h"
#include "sys_usw_chip.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_aps.h"
#include "sys_usw_common.h"
#include "sys_usw_l3if.h"
#include "sys_usw_interrupt.h"
#include "sys_usw_ftm.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_register.h"

#include "drv_api.h"
/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
#define SYS_APS_L3IFID_VAILD_CHECK(l3if_id)         \
    if (l3if_id > (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1)) \
    {                                               \
        SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid interface ID! \n");  \
		return CTC_E_BADID;                         \
    }
#define SYS_APS_MPLS_L3IF_CHECK(aps_group)   \
    if ((CTC_FLAG_ISSET(aps_group->aps_flag, CTC_APS_FLAG_IS_MPLS)) \
        && (((!aps_group->next_w_aps_en) && (!aps_group->w_l3if_id)) \
            || ((!aps_group->next_p_aps_en) && (!aps_group->p_l3if_id)))) \
    { \
        SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Mpls aps must configure l3if id \n"); \
		return CTC_E_INVALID_CONFIG;                                       \
    }

#define SYS_APS_INIT_CHECK() \
do { \
    LCHIP_CHECK(lchip); \
    if (p_usw_aps_master[lchip] == NULL){ \
        SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
    return CTC_E_NOT_INIT;\
   } \
} while (0)

#define SYS_APS_FILELD_SET(fld, val) DRV_SET_FIELD_V(lchip, DsApsBridge_t, fld , &ds_aps_bridge, val)
#define SYS_APS_FILELD_GET(fld, val) drv_get_field(DsApsBridge_t, fld , &ds_aps_bridge, val)
/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
enum sys_aps_flag_e
{
    SYS_APS_FLAG_NEXT_W_APS_EN               = 1U<<0,   /**< Use next_w_aps neglect working_gport */
    SYS_APS_FLAG_NEXT_P_APS_EN               = 1U<<1,   /**< Use next_w_aps neglect protection_gport */
    SYS_APS_FLAG_L2_APS_PHYSICAL_ISOLATED    = 1U<<2,   /**< Indicate port aps use physical isolated path */
    SYS_APS_FLAG_HW_BASED_FAILOVER_EN        = 1U<<3,   /**< Used for 1-level link fast aps, by hardware switch path when link down */
    SYS_APS_FLAG_RAPS                        = 1U<<4,   /**< Used for raps */
    SYS_APS_FLAG_BIND_EN                     = 1U<<5,   /**< bind mpls pw or mpls tunnel  */
    SYS_APS_FLAG_BIND_MPLS_TUNNEL_EN         = 1U<<6,   /**< bind mpls tunnel */
    SYS_APS_FLAG_MPLS                        = 1U<<7,   /**< mpls aps enable */
    SYS_APS_FLAG_BIND_W_EN                   = 1U<<8,   /**<  working path */
    SYS_APS_FLAG_BIND_P_EN                   = 1U<<9,   /**<  protection path */

    MAX_SYS_APS_FLAG
};
typedef enum sys_aps_flag_e sys_aps_flag_t;

struct sys_aps_bridge_s
{
    uint16 w_l3if_id;                   /**< working l3if id */
    uint16 p_l3if_id;                   /**< protection l3if id */

    uint16 raps_group_id;
    uint16 flag;                /**< it's the sys_aps_flag_t value */
        uint32 nh_id;

};
typedef struct sys_aps_bridge_s sys_aps_bridge_t;

struct sys_aps_master_s
{
    ctc_vector_t* aps_bridge_vector;
    sal_mutex_t* p_aps_mutex;

    uint16 aps_used_num;
};
typedef struct sys_aps_master_s sys_aps_master_t;
sys_aps_master_t* p_usw_aps_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};
extern int32 sys_usw_nh_update_dsma(uint8 lchip, void* p_nh_info, void* user_date);
/****************************************************************************
 *
* Function
*
*****************************************************************************/
STATIC int32
_sys_usw_aps_set_bridge_group(uint8 lchip, uint16 group_id, ctc_aps_bridge_group_t* aps_group, DsApsBridge_m* ds_aps_bridge)
{
    uint8 gchip   = 0;
    uint16 dest_id   = 0;

    int32 ret = CTC_E_NONE;
    sys_l3if_prop_t w_l3if, p_l3if; 

    if (!aps_group->next_w_aps_en)
    {
        gchip   = SYS_MAP_CTC_GPORT_TO_GCHIP(aps_group->working_gport);
        dest_id = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(aps_group->working_gport);
        SetDsApsBridge(V,workingDestMap_f, ds_aps_bridge, SYS_ENCODE_DESTMAP(gchip, dest_id));
        SetDsApsBridge(V,workingNextApsBridgeEn_f, ds_aps_bridge, 0);
        SetDsApsBridge(V,workingIsLinkAggregation_f, ds_aps_bridge, 0);
        SetDsApsBridge(V, workingRemoteChip_f, ds_aps_bridge, 0);
        SetDsApsBridge(V,workingUcastId_f, ds_aps_bridge, dest_id);
        if (CTC_IS_LINKAGG_PORT(aps_group->working_gport))
        {
            SetDsApsBridge(V,workingIsLinkAggregation_f, ds_aps_bridge, 1);
        }
        else if (FALSE == sys_usw_chip_is_local(lchip, gchip))
        {
            SetDsApsBridge(V, workingRemoteChip_f, ds_aps_bridge, 1);
            SetDsApsBridge(V, workingUcastId_f, ds_aps_bridge, 0);
        }
        /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-27 */
        if ((aps_group->w_l3if_id) && (1021 == aps_group->w_l3if_id))
        {
            SetDsApsBridge(V,workingDestVlanPtr_f, ds_aps_bridge, 5117);   /*MAX_CTC_L3IF_ID -1 + 4096*/
        }
        //if (aps_group->w_l3if_id)
        else if (aps_group->w_l3if_id)
        /* liwh end */
        {
            sal_memset(&w_l3if, 0, sizeof(sys_l3if_prop_t));
            w_l3if.l3if_id = aps_group->w_l3if_id;
            CTC_ERROR_RETURN(sys_usw_l3if_get_l3if_info(lchip, 1, &w_l3if));
            if ((CTC_L3IF_TYPE_PHY_IF == w_l3if.l3if_type) && (w_l3if.gport != aps_group->working_gport))
            {
                ret = CTC_E_APS_INTERFACE_ERROR;
                return ret;
            }
            SetDsApsBridge(V,workingDestVlanPtr_f, ds_aps_bridge, w_l3if.vlan_ptr);
        }

    }
    else
    {
        SetDsApsBridge(V, workingNextApsBridgeEn_f, ds_aps_bridge, 1);
        SetDsApsBridge(V, workingDestMap_f, ds_aps_bridge, aps_group->next_aps.w_group_id);
    }

    if (!aps_group->next_p_aps_en)
    {
        gchip   = SYS_MAP_CTC_GPORT_TO_GCHIP(aps_group->protection_gport);
        dest_id = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(aps_group->protection_gport);
        SetDsApsBridge(V, protectingDestMap_f, ds_aps_bridge, SYS_ENCODE_DESTMAP(gchip, dest_id));
        SetDsApsBridge(V, protectingNextApsBridgeEn_f, ds_aps_bridge, 0);
        SetDsApsBridge(V, protectingUcastId_f, ds_aps_bridge, dest_id);
        SetDsApsBridge(V, protectingIsLinkAggregation_f, ds_aps_bridge, 0);
        if (CTC_IS_LINKAGG_PORT(aps_group->protection_gport))
        {
            SetDsApsBridge(V, protectingIsLinkAggregation_f, ds_aps_bridge, 1);
        }
        else if (FALSE == sys_usw_chip_is_local(lchip, gchip))
        {
            SetDsApsBridge(V, protectingRemoteChip_f, ds_aps_bridge, 1);
            SetDsApsBridge(V, protectingUcastId_f, ds_aps_bridge, 0);
        }
        /* modified by liwh for bug 46591 to support mpls tpoam, 2018-03-27 */
        if ((aps_group->p_l3if_id) && (1021 == aps_group->p_l3if_id))
        {
            SetDsApsBridge(V,workingDestVlanPtr_f, ds_aps_bridge, 5117);   /*MAX_CTC_L3IF_ID -1 + 4096*/
        }
        //if (aps_group->w_l3if_id)
        else if (aps_group->p_l3if_id)
        /* liwh end */
        {
            sal_memset(&p_l3if, 0, sizeof(sys_l3if_prop_t));
            p_l3if.l3if_id = aps_group->p_l3if_id;
            CTC_ERROR_RETURN(sys_usw_l3if_get_l3if_info(lchip, 1, &w_l3if));
            if ((CTC_L3IF_TYPE_PHY_IF == w_l3if.l3if_type) && (w_l3if.gport != aps_group->working_gport))
            {
                ret = CTC_E_APS_INTERFACE_ERROR;
                return ret;
            }
            SetDsApsBridge(V,workingDestVlanPtr_f, ds_aps_bridge, p_l3if.vlan_ptr);
        }
    }
    else
    {
        SetDsApsBridge(V, protectingNextApsBridgeEn_f, ds_aps_bridge, 1);
        SetDsApsBridge(V, protectingDestMap_f, ds_aps_bridge, aps_group->next_aps.p_group_id);
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_aps_free_node_data(void* node_data, void* user_data)
{
    if (node_data)
    {
        mem_free(node_data);
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_aps_get_protection(uint8 lchip, uint16 group_id, bool* protect_en)
{
    int32 ret = CTC_E_NONE;
    sys_aps_bridge_t* p_aps_bridge = NULL;
    uint32 protecting_en = 0;
    uint32 cmd = 0;

    p_aps_bridge = ctc_vector_get(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);

    if (NULL == p_aps_bridge)
    {
        *protect_en = FALSE;
        CTC_ERROR_RETURN(CTC_E_NOT_EXIST);
    }

    cmd = DRV_IOR(DsApsBridge_t, DsApsBridge_protectingEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &protecting_en));
    *protect_en = protecting_en;

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "group_id=%d, protect_en=%d\n", group_id, *protect_en);

    return ret;

}


int32
_sys_usw_aps_wb_traverse_sync_node(void *data,uint32 vec_index, void *user_data)
{
    int32 ret;
    sys_aps_bridge_t *p_aps_node = (sys_aps_bridge_t*)data;
    sys_traverse_t *wb_traverse = (sys_traverse_t*)user_data;
    ctc_wb_data_t *wb_data = wb_traverse->data;
    uint16 max_buffer_cnt = wb_data->buffer_len/(wb_data->key_len + wb_data->data_len);
    sys_wb_aps_node_t* p_wb_aps_node = NULL;

    p_wb_aps_node = (sys_wb_aps_node_t *)wb_data->buffer + wb_data->valid_cnt;

    sal_memset(p_wb_aps_node, 0, sizeof(sys_wb_aps_node_t));

    p_wb_aps_node->group_id = vec_index;
    p_wb_aps_node->w_l3if_id = p_aps_node->w_l3if_id;
    p_wb_aps_node->p_l3if_id = p_aps_node->p_l3if_id;
    p_wb_aps_node->raps_group_id = p_aps_node->raps_group_id;
    p_wb_aps_node->flag = p_aps_node->flag;
    p_wb_aps_node->nh_id = p_aps_node->nh_id;

    if (++wb_data->valid_cnt ==  max_buffer_cnt)
    {
        ret = ctc_wb_add_entry(wb_data);
        if ( ret != CTC_E_NONE )
        {
           return ret;
        }
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_aps_get_ports(uint8 lchip, uint16 group_id, ctc_aps_bridge_group_t* aps_group)
{
    sys_aps_bridge_t* p_aps_bridge = NULL;
    DsApsBridge_m ds_aps_bridge;
    uint32 cmd = 0;
    uint32 dest_map = 0;
    uint32 globle_port = 0;


    sal_memset(aps_group, 0, sizeof(ctc_aps_bridge_group_t));
    p_aps_bridge = ctc_vector_get(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);
    if (NULL == p_aps_bridge)
    {
        SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Group dont exist \n");
		return CTC_E_NOT_EXIST;
    }

    sal_memset(&ds_aps_bridge, 0, sizeof(ds_aps_bridge));
    cmd = DRV_IOR(DsApsBridge_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, group_id, cmd, &ds_aps_bridge), p_usw_aps_master[lchip]->p_aps_mutex);

    aps_group->protect_en = GetDsApsBridge(V, protectingEn_f, &ds_aps_bridge);
    if (p_aps_bridge->flag & SYS_APS_FLAG_NEXT_W_APS_EN)
    {
        aps_group->next_w_aps_en = 1;
        aps_group->next_aps.w_group_id = GetDsApsBridge(V, workingDestMap_f, &ds_aps_bridge);
    }
    else
    {
        GetDsApsBridge(A, workingDestMap_f, &ds_aps_bridge, &dest_map);

        CTC_ERROR_RETURN(sys_usw_nh_map_destmap_to_port(lchip, dest_map, &globle_port));

        aps_group->working_gport = globle_port;
        aps_group->w_l3if_id = p_aps_bridge->w_l3if_id;
    }

    if (p_aps_bridge->flag & SYS_APS_FLAG_NEXT_P_APS_EN)
    {
        aps_group->next_p_aps_en = 1;
        aps_group->next_aps.p_group_id = GetDsApsBridge(V, protectingDestMap_f, &ds_aps_bridge);
    }
    else
    {
        GetDsApsBridge(A, protectingDestMap_f, &ds_aps_bridge, &dest_map);
        CTC_ERROR_RETURN(sys_usw_nh_map_destmap_to_port(lchip, dest_map, &globle_port));

        aps_group->protection_gport = globle_port;
        aps_group->p_l3if_id = p_aps_bridge->p_l3if_id;
    }

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_aps_show_one_bridge(uint8 lchip, uint16 group_id, uint8 detail)
{
    sys_aps_bridge_t* p_aps_bridge = NULL;
    ctc_aps_bridge_group_t aps_group;
    char w_path[16]={0};
    char p_path[16]={0};
    char* w_flag = "P";
    char* p_flag = "P";
    char w_ifid[8] = "-";
    char p_ifid[8] = "-";
    char* hw = "NO";
    char* w_p = "W";
    uint8 raps_valid = 0;
    bool protection_en;
    uint32 nhid = 0;


    SYS_APS_INIT_CHECK();

    p_aps_bridge = ctc_vector_get(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);
    if (NULL == p_aps_bridge)
    {
        SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Group dont exist \n");
		return CTC_E_NOT_EXIST;
    }

    sal_memset(&aps_group, 0, sizeof(ctc_aps_bridge_group_t));
    _sys_usw_aps_get_ports(lchip, group_id, &aps_group);
    if(!CTC_FLAG_ISSET(p_aps_bridge->flag, SYS_APS_FLAG_NEXT_W_APS_EN))
    {
        sal_sprintf(w_path, "0x%04x", aps_group.working_gport);
         w_flag = "P";
    }
    else
    {
        sal_sprintf(w_path, "%d", aps_group.next_aps.w_group_id);
         w_flag = "G";
    }

    if (p_aps_bridge->w_l3if_id)
    {
        sal_sprintf(w_ifid, "%d", p_aps_bridge->w_l3if_id);
    }

    if(!CTC_FLAG_ISSET(p_aps_bridge->flag,SYS_APS_FLAG_RAPS))
    {
        if (!CTC_FLAG_ISSET(p_aps_bridge->flag, SYS_APS_FLAG_NEXT_P_APS_EN))
        {
            sal_sprintf(p_path, "0x%04x", aps_group.protection_gport);
            p_flag = "P";
        }
        else
        {
            sal_sprintf(p_path, "%d", aps_group.next_aps.p_group_id);
            p_flag = "G";
        }

        if (p_aps_bridge->p_l3if_id)
        {
            sal_sprintf(p_ifid, "%d", p_aps_bridge->p_l3if_id);
        }
    }
    else
    {
        sal_sprintf(p_path, "%d", p_aps_bridge->raps_group_id);
        p_flag = "R";

        if (CTC_E_NOT_EXIST != sys_usw_nh_get_mcast_nh(lchip, p_aps_bridge->raps_group_id, &nhid))
        {
            raps_valid = 1;
        }
    }

    if (CTC_FLAG_ISSET(p_aps_bridge->flag, SYS_APS_FLAG_HW_BASED_FAILOVER_EN))
    {
        hw = "YES";
    }

    CTC_ERROR_RETURN(_sys_usw_aps_get_protection(lchip, group_id, &protection_en));
    if (protection_en)
    {
        w_p = "P";
    }
    else
    {
        w_p = "W";
    }

    SYS_APS_DUMP("%-6d%-13s%-7s%-9s%-10s%-7s%-8s%-6s%-6s\n", group_id, w_path, w_flag, w_ifid, p_path, p_flag, p_ifid, w_p, hw);
    if (detail)
    {
        SYS_APS_DUMP("\nDetail information\n");
        SYS_APS_DUMP("----------------------\n");
        SYS_APS_DUMP("%-15s:%d\n","--DsApsBridge", group_id);
        if (raps_valid)
        {
            SYS_APS_DUMP("%-15s:%u\n", "--Raps nexthop", nhid);
        }
    }
    return CTC_E_NONE;
}

#define __SYS_API__

int32
sys_usw_aps_set_protection(uint8 lchip, uint16 group_id, bool protect_en)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 field_val = 0;
    sys_aps_bridge_t* p_aps_bridge = NULL;

    SYS_APS_INIT_CHECK();

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "group_id=%d, protect_en=%d\n", group_id, protect_en);

    SYS_APS_GROUP_ID_CHECK(group_id);

    APS_LOCK(lchip);
    p_aps_bridge = ctc_vector_get(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);

    if (NULL == p_aps_bridge)
    {
        ret = CTC_E_NOT_EXIST;
        goto out;
    }

    cmd = DRV_IOW(DsApsBridge_t, DsApsBridge_protectingEn_f);
    field_val = (TRUE == protect_en) ? 1 : 0;
    ret = DRV_IOCTL(lchip, group_id, cmd, &field_val);

    /*update dsma for tp oam*/
    if (CTC_FLAG_ISSET(p_aps_bridge->flag,SYS_APS_FLAG_BIND_MPLS_TUNNEL_EN))
    {
        ret = sys_usw_nh_traverse_mpls_nexthop(lchip, p_aps_bridge->nh_id, sys_usw_nh_update_dsma, (void*)&protect_en);
    }

out:
    APS_UNLOCK(lchip);
    return ret;

}

int32
sys_usw_aps_get_protection(uint8 lchip, uint16 group_id, bool* protect_en)
{
    int32 ret = CTC_E_NONE;

    SYS_APS_INIT_CHECK();
    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(protect_en);
    SYS_APS_GROUP_ID_CHECK(group_id);

    APS_LOCK(lchip);
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_aps_get_protection(lchip, group_id, protect_en), p_usw_aps_master[lchip]->p_aps_mutex);
    APS_UNLOCK(lchip);
    return ret;

}

int32
sys_usw_aps_create_bridge_group(uint8 lchip, uint16 group_id, ctc_aps_bridge_group_t* aps_group)
{
    int32 ret = CTC_E_NONE;
    uint8 gchip   = 0;
    uint16 dest_id   = 0;
    uint32 cmd = 0;
    sys_aps_bridge_t* p_aps_bridge = NULL;
    DsApsBridge_m ds_aps_bridge;
    uint32 nhid = 0;

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_APS_INIT_CHECK();

    SYS_APS_GROUP_ID_CHECK(group_id);
    CTC_PTR_VALID_CHECK(aps_group);
    sys_usw_get_gchip_id(lchip, &gchip);

    if(CTC_FLAG_ISSET(aps_group->aps_flag, CTC_APS_FLAG_IS_MPLS))
    {/*mpls aps */

      aps_group->working_gport = CTC_MAP_LPORT_TO_GPORT(gchip,SYS_RSV_PORT_DROP_ID);
      aps_group->protection_gport = CTC_MAP_LPORT_TO_GPORT(gchip,SYS_RSV_PORT_DROP_ID);
    }

    SYS_GLOBAL_PORT_CHECK(aps_group->working_gport);
    SYS_GLOBAL_PORT_CHECK(aps_group->protection_gport);

    if (aps_group->raps_en) /*RAPS protection*/
    {
        if (CTC_E_NOT_EXIST == sys_usw_nh_get_mcast_nh(lchip, aps_group->raps_group_id, &nhid))
        {
            SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Raps group dont exist \n");
            return CTC_E_NOT_EXIST;
        }
    }

    SYS_APS_BRIDGE_DUMP(group_id, aps_group);

    if (aps_group->next_w_aps_en)
    {
        SYS_APS_GROUP_ID_CHECK(aps_group->next_aps.w_group_id);
    }
    if (aps_group->next_p_aps_en)
    {
        SYS_APS_GROUP_ID_CHECK(aps_group->next_aps.p_group_id);
    }
    if (aps_group->aps_failover_en)
    {
        if((aps_group->next_w_aps_en)||(aps_group->next_p_aps_en))
        {
            SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Aps only support hw-based aps for 1 level \n");
			return CTC_E_INVALID_CONFIG;

        }
        if(CTC_IS_LINKAGG_PORT(aps_group->working_gport)||CTC_IS_LINKAGG_PORT(aps_group->protection_gport))
        {
            SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Aps only support hw-based aps for phyport \n");
			return CTC_E_NOT_SUPPORT;
        }
    }

    APS_LOCK(lchip);
    p_aps_bridge = ctc_vector_get(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);
    if (NULL != p_aps_bridge)
    {
        ret = CTC_E_EXIST;
        goto error0;
    }

    p_aps_bridge = mem_malloc(MEM_APS_MODULE, sizeof(sys_aps_bridge_t));
    if (NULL == p_aps_bridge)
    {
        ret = CTC_E_NO_MEMORY;
        goto error0;
    }

    sal_memset(p_aps_bridge, 0, sizeof(sys_aps_bridge_t));
    sal_memset(&ds_aps_bridge, 0, sizeof(DsApsBridge_m));
    SYS_APS_FILELD_SET(DsApsBridge_protectingEn_f, aps_group->protect_en);
    SYS_APS_FILELD_SET(DsApsBridge_spmeApsEn_f, 0);
    SYS_APS_FILELD_SET(DsApsBridge_differentNexthop_f, 1);
    if (aps_group->physical_isolated)
    {
        CTC_SET_FLAG(p_aps_bridge->flag, SYS_APS_FLAG_L2_APS_PHYSICAL_ISOLATED);
        SYS_APS_FILELD_SET(DsApsBridge_differentNexthop_f, 0);
    }
    if(aps_group->raps_en)
    {
        CTC_SET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_RAPS);
    }
    p_aps_bridge->raps_group_id   = aps_group->raps_group_id;

    if(aps_group->aps_failover_en)
    {
        CTC_SET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_HW_BASED_FAILOVER_EN);
        SYS_APS_FILELD_SET(DsApsBridge_linkChangeEn_f, 1);
    }
    if(aps_group->next_w_aps_en)
    {
        CTC_SET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_NEXT_W_APS_EN);
    }
    if(aps_group->next_p_aps_en)
    {
        CTC_SET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_NEXT_P_APS_EN);
    }

    ret = _sys_usw_aps_set_bridge_group(lchip,  group_id, aps_group, &ds_aps_bridge);
    if (ret != CTC_E_NONE)
    {
        goto error1;
    }

    if (FALSE == ctc_vector_add(p_usw_aps_master[lchip]->aps_bridge_vector, group_id, p_aps_bridge))
    {
        ret = CTC_E_NO_MEMORY;
        goto error1;
    }

    if(aps_group->raps_en)
    {
        sys_usw_get_gchip_id(lchip, &gchip);
        dest_id = aps_group->raps_group_id;
        SYS_APS_FILELD_SET(DsApsBridge_protectingDestMap_f, SYS_ENCODE_MCAST_IPE_DESTMAP(dest_id));
    }

    cmd = DRV_IOW(DsApsBridge_t, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, group_id, cmd, &ds_aps_bridge);
    if (ret)
    {
        ret = CTC_E_HW_FAIL;
        goto error2;
    }

    p_usw_aps_master[lchip]->aps_used_num++;
    APS_UNLOCK(lchip);
    return CTC_E_NONE;

error2:
    ctc_vector_del(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);
error1:
    mem_free(p_aps_bridge);
error0:
    APS_UNLOCK(lchip);
    return ret;

}

int32
sys_usw_aps_update_bridge_group(uint8 lchip, uint16 group_id, ctc_aps_bridge_group_t* aps_group)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    sys_aps_bridge_t* p_aps_bridge = NULL;
    DsApsBridge_m ds_aps_bridge;

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_APS_INIT_CHECK();
    SYS_APS_GROUP_ID_CHECK(group_id);
    CTC_PTR_VALID_CHECK(aps_group);
    SYS_GLOBAL_PORT_CHECK(aps_group->working_gport);
    SYS_GLOBAL_PORT_CHECK(aps_group->protection_gport);
    SYS_APS_L3IFID_VAILD_CHECK(aps_group->w_l3if_id);
    SYS_APS_L3IFID_VAILD_CHECK(aps_group->p_l3if_id);
    SYS_APS_MPLS_L3IF_CHECK(aps_group);
    if (aps_group->next_w_aps_en)
    {
        SYS_APS_GROUP_ID_CHECK(aps_group->next_aps.w_group_id);
    }
    if (aps_group->next_p_aps_en)
    {
        SYS_APS_GROUP_ID_CHECK(aps_group->next_aps.p_group_id);
    }
    SYS_APS_BRIDGE_DUMP(group_id, aps_group);

    APS_LOCK(lchip);
    p_aps_bridge = ctc_vector_get(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);
    if (NULL == p_aps_bridge)
    {
        ret = CTC_E_NOT_EXIST;
        goto out;
    }

    if (aps_group->raps_en != CTC_FLAG_ISSET(p_aps_bridge->flag,SYS_APS_FLAG_RAPS))
    {
        ret = CTC_E_INVALID_CONFIG;
        goto out;
    }

    /*mpls aps bridge must update by Nexthop module*/
    if(CTC_FLAG_ISSET(p_aps_bridge->flag,SYS_APS_FLAG_BIND_EN))
    {
      ret = CTC_E_NONE;
      goto out;
    }

    if(aps_group->next_w_aps_en)
    {
        CTC_SET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_NEXT_W_APS_EN);
    }
    else
    {
        CTC_UNSET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_NEXT_W_APS_EN);
    }

    if(aps_group->next_p_aps_en)
    {
        CTC_SET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_NEXT_P_APS_EN);
    }
    else
    {
        CTC_UNSET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_NEXT_P_APS_EN);
    }

    sal_memset(&ds_aps_bridge, 0, sizeof(DsApsBridge_m));
    cmd = DRV_IOR(DsApsBridge_t, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, group_id, cmd, &ds_aps_bridge);
    ret = ret ? ret : _sys_usw_aps_set_bridge_group(lchip,  group_id, aps_group, &ds_aps_bridge);
    cmd = DRV_IOW(DsApsBridge_t, DRV_ENTRY_FLAG);
    ret = ret ? ret : DRV_IOCTL(lchip, group_id, cmd, &ds_aps_bridge);

out:
        APS_UNLOCK(lchip);
        return ret;
}

int32
sys_usw_aps_remove_bridge_group(uint8 lchip, uint16 group_id)
{
    uint8 gchip = 0;
    uint32 cmd = 0;
    int32 ret = CTC_E_NONE;
    DsApsBridge_m ds_aps_bridge;
    sys_aps_bridge_t* p_aps_bridge = NULL;

    SYS_APS_INIT_CHECK();

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "group_id %d\n", group_id);

    SYS_APS_GROUP_ID_CHECK(group_id);

    APS_LOCK(lchip);
    p_aps_bridge = ctc_vector_del(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);
    if (NULL == p_aps_bridge)
    {
        ret = CTC_E_NOT_EXIST;
        goto out;
    }
    mem_free(p_aps_bridge);

    sal_memset(&ds_aps_bridge, 0, sizeof(ds_aps_bridge));
    cmd = DRV_IOR(DsApsBridge_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, group_id, cmd, &ds_aps_bridge);

    SYS_APS_FILELD_SET(DsApsBridge_linkChangeEn_f,0);
    sys_usw_get_gchip_id(lchip, &gchip);
    SYS_APS_FILELD_SET(DsApsBridge_workingDestMap_f, SYS_ENCODE_DESTMAP( gchip, SYS_RSV_PORT_DROP_ID));
    SYS_APS_FILELD_SET(DsApsBridge_protectingDestMap_f, SYS_ENCODE_DESTMAP( gchip, SYS_RSV_PORT_DROP_ID));
    cmd = DRV_IOW(DsApsBridge_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, group_id, cmd, &ds_aps_bridge);

    p_usw_aps_master[lchip]->aps_used_num = 0? 0 : (p_usw_aps_master[lchip]->aps_used_num - 1);

out:
    APS_UNLOCK(lchip);
    return ret;

}

int32
sys_usw_aps_set_bridge_path(uint8 lchip, uint16 group_id, uint32 gport, bool is_working)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint8 gchip   = 0;
    uint16 dest_id   = 0;
    sys_aps_bridge_t* p_aps_bridge = NULL;
    DsApsBridge_m ds_aps_bridge;

    SYS_APS_INIT_CHECK();

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "group_id=%d, gport=0x%04x\n", group_id, gport);

    SYS_APS_GROUP_ID_CHECK(group_id);
    SYS_GLOBAL_PORT_CHECK(gport);

    APS_LOCK(lchip);
    p_aps_bridge = ctc_vector_get(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);
    if (NULL == p_aps_bridge)
    {
        APS_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }
    /*mpls aps bridge must update by Nexthop module*/
    if(CTC_FLAG_ISSET(p_aps_bridge->flag,SYS_APS_FLAG_BIND_EN))
    {
       APS_UNLOCK(lchip);
      return CTC_E_NONE;
    }
    sal_memset(&ds_aps_bridge, 0, sizeof(ds_aps_bridge));
    cmd = DRV_IOR(DsApsBridge_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, group_id, cmd, &ds_aps_bridge), p_usw_aps_master[lchip]->p_aps_mutex);

    gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport);
    dest_id = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);
    if (is_working)
    {
        SYS_APS_FILELD_SET(DsApsBridge_workingNextApsBridgeEn_f, 0);
        SYS_APS_FILELD_SET(DsApsBridge_workingIsLinkAggregation_f, 0);
        SYS_APS_FILELD_SET(DsApsBridge_workingRemoteChip_f, 0);
        SYS_APS_FILELD_SET(DsApsBridge_workingDestMap_f, SYS_ENCODE_DESTMAP( gchip, dest_id));
        SYS_APS_FILELD_SET(DsApsBridge_workingUcastId_f , dest_id);
        if (CTC_IS_LINKAGG_PORT(gport))
        {
            SYS_APS_FILELD_SET(DsApsBridge_workingIsLinkAggregation_f, 1);
        }
        else if (FALSE == sys_usw_chip_is_local(lchip, gchip))
        {
            SYS_APS_FILELD_SET(DsApsBridge_workingRemoteChip_f, 1);
            SYS_APS_FILELD_SET(DsApsBridge_workingUcastId_f , 0);
        }

    }
    else
    {
        SYS_APS_FILELD_SET(DsApsBridge_protectingNextApsBridgeEn_f, 0);
        gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(gport);
        dest_id = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(gport);
        SYS_APS_FILELD_SET(DsApsBridge_protectingDestMap_f, SYS_ENCODE_DESTMAP( gchip, dest_id));
        SYS_APS_FILELD_SET(DsApsBridge_protectingIsLinkAggregation_f, 0);
        if (CTC_IS_LINKAGG_PORT(gport))
        {
            SYS_APS_FILELD_SET(DsApsBridge_protectingIsLinkAggregation_f, 1);
        }
        else if (FALSE == sys_usw_chip_is_local(lchip, gchip))
        {
            SYS_APS_FILELD_SET(DsApsBridge_protectingRemoteChip_f, 1);
            SYS_APS_FILELD_SET(DsApsBridge_protectingUcastId_f , 0);
        }
        SYS_APS_FILELD_SET(DsApsBridge_protectingUcastId_f , dest_id);

    }

    cmd = DRV_IOW(DsApsBridge_t, DRV_ENTRY_FLAG);
    ret = ret ? ret : DRV_IOCTL(lchip, group_id, cmd, &ds_aps_bridge);


    APS_UNLOCK(lchip);
    return ret;

}


int32
sys_usw_aps_get_bridge_path(uint8 lchip, uint16 group_id, uint32* gport, bool is_working)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 dest_map = 0;
    uint32 globle_port = 0;
    sys_aps_bridge_t* p_aps_bridge = NULL;
    DsApsBridge_m ds_aps_bridge;

    SYS_APS_INIT_CHECK();

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(gport);
    SYS_APS_GROUP_ID_CHECK(group_id);

    APS_LOCK(lchip);
    p_aps_bridge = ctc_vector_get(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);
    if (NULL == p_aps_bridge)
    {
        *gport = CTC_MAX_UINT16_VALUE;  /*invalid value*/
        CTC_ERROR_RETURN_WITH_UNLOCK(CTC_E_NOT_EXIST, p_usw_aps_master[lchip]->p_aps_mutex);
    }

    sal_memset(&ds_aps_bridge, 0, sizeof(ds_aps_bridge));
    cmd = DRV_IOR(DsApsBridge_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, group_id, cmd, &ds_aps_bridge), p_usw_aps_master[lchip]->p_aps_mutex);

    if (is_working)
    {
        if (CTC_FLAG_ISSET(p_aps_bridge->flag, SYS_APS_FLAG_NEXT_W_APS_EN))
        {
            ret = CTC_E_EXIST;
            goto out;
        }
        GetDsApsBridge(A, workingDestMap_f, &ds_aps_bridge, &dest_map);
    }
    else
    {
        if (CTC_FLAG_ISSET(p_aps_bridge->flag, SYS_APS_FLAG_NEXT_P_APS_EN))
        {
            ret = CTC_E_EXIST;
            goto out;
        }
        GetDsApsBridge(A, protectingDestMap_f, &ds_aps_bridge, &dest_map);

    }

    CTC_ERROR_RETURN_WITH_UNLOCK(
    sys_usw_nh_map_destmap_to_port(lchip, dest_map, &globle_port),
        p_usw_aps_master[lchip]->p_aps_mutex);

    *gport = globle_port;

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "group_id=%d, gport=0x%04x\n", group_id, *gport);

out:
    APS_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_aps_create_raps_mcast_group(uint8 lchip, uint16 group_id)
{
    sys_nh_param_mcast_group_t  nh_mcast_group;

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_APS_INIT_CHECK();
    sal_memset(&nh_mcast_group, 0, sizeof(sys_nh_param_mcast_group_t));
    nh_mcast_group.dsfwd_valid= 0;
    CTC_ERROR_RETURN(sys_usw_mcast_nh_create(lchip, group_id, &nh_mcast_group));

    return CTC_E_NONE;
}

int32
sys_usw_aps_update_raps_mcast_member(uint8 lchip, ctc_raps_member_t* p_raps_mem)
{
    sys_nh_param_mcast_group_t  nh_param_mcast_group;
    uint16 lport = 0;
    uint16 dest_port = 0;
    uint32 nhid = 0;

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_APS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_raps_mem);
    sal_memset(&nh_param_mcast_group, 0, sizeof(sys_nh_param_mcast_group_t));

    if (CTC_E_NOT_EXIST == sys_usw_nh_get_mcast_nh(lchip, p_raps_mem->group_id, &nhid))
    {
        SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Raps group dont exist \n");
        return CTC_E_NOT_EXIST;
    }

    nh_param_mcast_group.nhid = nhid;
    nh_param_mcast_group.dsfwd_valid = 0;

    if (p_raps_mem->remote_chip)
    {
        nh_param_mcast_group.mem_info.member_type = SYS_NH_PARAM_MCAST_MEM_REMOTE;
    }
    else
    {
        nh_param_mcast_group.mem_info.member_type = SYS_NH_PARAM_BRGMC_MEM_RAPS;
    }

    if (!p_raps_mem->remove_flag)
    {
        nh_param_mcast_group.opcode = SYS_NH_PARAM_MCAST_ADD_MEMBER;
    }
    else
    {
        nh_param_mcast_group.opcode = SYS_NH_PARAM_MCAST_DEL_MEMBER;
    }

    dest_port = p_raps_mem->mem_port;
    lport = CTC_MAP_GPORT_TO_LPORT(dest_port);
    if (CTC_IS_LINKAGG_PORT(dest_port))
    {
        nh_param_mcast_group.mem_info.is_linkagg = 1;
        nh_param_mcast_group.mem_info.destid = lport;
    }
    else
    {
        nh_param_mcast_group.mem_info.is_linkagg = 0;
        nh_param_mcast_group.mem_info.destid = lport;
    }

    CTC_ERROR_RETURN(sys_usw_mcast_nh_update(lchip, &nh_param_mcast_group));

    return CTC_E_NONE;

}

int32
sys_usw_aps_remove_raps_mcast_group(uint8 lchip, uint16 group_id)
{
    int32 ret = CTC_E_NONE;
    uint32 nhid = 0;

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_APS_INIT_CHECK();
    if (CTC_E_NOT_EXIST == sys_usw_nh_get_mcast_nh(lchip, group_id, &nhid))
    {
        SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Raps group dont exist \n");
		return CTC_E_NOT_EXIST;
    }

    ret = sys_usw_mcast_nh_delete(lchip, nhid);

    return ret;
}

int32
sys_usw_aps_set_share_nh(uint8 lchip, uint16 group_id, bool share_nh)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 field_val = 0;
    sys_aps_bridge_t* p_aps_bridge = NULL;

    SYS_APS_INIT_CHECK();

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "group_id=%d,share_nh=%d", group_id, share_nh);

    SYS_APS_GROUP_ID_CHECK(group_id);

    APS_LOCK(lchip);
    p_aps_bridge = ctc_vector_get(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);

    if (NULL == p_aps_bridge)
    {
        ret = CTC_E_NOT_EXIST;
        goto out;
    }

    cmd = DRV_IOW(DsApsBridge_t, DsApsBridge_differentNexthop_f);
    field_val = (TRUE == share_nh) ? 0 : 1;
    ret = DRV_IOCTL(lchip, group_id, cmd, &field_val);

out:
    APS_UNLOCK(lchip);
    return ret;

}


int32
sys_usw_aps_get_share_nh(uint8 lchip, uint16 group_id, bool* share_nh)
{
    int32 ret = CTC_E_NONE;
    sys_aps_bridge_t* p_aps_bridge = NULL;

    SYS_APS_INIT_CHECK();

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(share_nh);
    SYS_APS_GROUP_ID_CHECK(group_id);

    APS_LOCK(lchip);
    p_aps_bridge = ctc_vector_get(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);
    if (NULL == p_aps_bridge)
    {
        *share_nh = 0;
        CTC_ERROR_RETURN_WITH_UNLOCK(CTC_E_NOT_EXIST, p_usw_aps_master[lchip]->p_aps_mutex);
    }

    *share_nh = CTC_FLAG_ISSET(p_aps_bridge->flag,SYS_APS_FLAG_L2_APS_PHYSICAL_ISOLATED);
    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "group_id=%d,share_nh=%d\n", group_id, *share_nh);

    APS_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_aps_get_next_group(uint8 lchip, uint16 group_id, uint16* next_group_id, bool is_working)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd;
    sys_aps_bridge_t* p_aps_bridge = NULL;
    DsApsBridge_m ds_aps_bridge;

    SYS_APS_INIT_CHECK();

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(next_group_id);
    SYS_APS_GROUP_ID_CHECK(group_id);

    APS_LOCK(lchip);
    p_aps_bridge = ctc_vector_get(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);
    *next_group_id = CTC_MAX_UINT16_VALUE;
    if (NULL == p_aps_bridge)
    {
        APS_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    if ((is_working && (!CTC_FLAG_ISSET(p_aps_bridge->flag, SYS_APS_FLAG_NEXT_W_APS_EN)))
        || (!is_working && (!CTC_FLAG_ISSET(p_aps_bridge->flag, SYS_APS_FLAG_NEXT_P_APS_EN))))
    {
        APS_UNLOCK(lchip);
        return CTC_E_NONE;
    }

    sal_memset(&ds_aps_bridge, 0, sizeof(ds_aps_bridge));
    cmd  = DRV_IOR(DsApsBridge_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, group_id, cmd, &ds_aps_bridge), p_usw_aps_master[lchip]->p_aps_mutex);

    if (is_working)
    {
        *next_group_id = GetDsApsBridge(V, workingDestMap_f, &ds_aps_bridge);
    }
    else
    {
        *next_group_id = GetDsApsBridge(V, protectingDestMap_f, &ds_aps_bridge);
    }
    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "group_id=%d, next_aps_id=%d\n", group_id, *next_group_id);

    APS_UNLOCK(lchip);
    return ret;
}

#if 0
int32
sys_usw_aps_get_current_working_port(uint8 lchip, uint16 group_id, uint32* gport)
{
    int32 ret = CTC_E_NONE;
    uint32 cmd;
    sys_aps_bridge_t* p_aps_bridge = NULL;
    DsApsBridge_m ds_aps_bridge;
    uint8 protection    = 0;
    uint8 next_is_aps   = 0;
    uint32 dest_map     = 0;
    uint32 globle_port = 0;

    SYS_APS_INIT_CHECK();
    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(gport);
    SYS_APS_GROUP_ID_CHECK(group_id);

    /*check aps exist or not*/
    APS_LOCK(lchip);
    p_aps_bridge = ctc_vector_get(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);
    if (NULL == p_aps_bridge)
    {
        *gport = CTC_MAX_UINT16_VALUE;
        CTC_ERROR_RETURN_WITH_UNLOCK(CTC_E_NOT_EXIST, p_usw_aps_master[lchip]->p_aps_mutex);
    }
    APS_UNLOCK(lchip);

    /*Get 1st level aps group*/
    sal_memset(&ds_aps_bridge, 0, sizeof(ds_aps_bridge));
    cmd  = DRV_IOR(DsApsBridge_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, group_id, cmd, &ds_aps_bridge));

    protection = GetDsApsBridge(V, protectingEn_f, &ds_aps_bridge);

    if (protection)
    {
        next_is_aps = GetDsApsBridge(V, protectingNextApsBridgeEn_f, &ds_aps_bridge);
        dest_map    = GetDsApsBridge(V, protectingDestMap_f, &ds_aps_bridge);
    }
    else
    {
        next_is_aps = GetDsApsBridge(V, workingNextApsBridgeEn_f, &ds_aps_bridge);
        dest_map    = GetDsApsBridge(V, workingDestMap_f, &ds_aps_bridge);
    }

    if(!next_is_aps)
    {
        CTC_ERROR_RETURN(sys_usw_nh_map_destmap_to_port(lchip, dest_map, &globle_port));
        *gport = globle_port;
    }
    else
    {/*Get 2nd level aps group*/
        return sys_usw_aps_get_current_working_port(lchip, dest_map, gport);
    }

    return ret;
}
#endif

int32
sys_usw_aps_bind_nexthop(uint8 lchip,uint16 group_id,sys_aps_bind_nh_param_t *p_bind_nh)
{
       uint32 cmd;
    sys_aps_bridge_t* p_aps_bridge = NULL;
    DsApsBridge_m ds_aps_bridge;
    uint8 gchip = 0;
    uint16 dest_id = 0;
    sys_l3if_prop_t l3if_prop;

    SYS_APS_INIT_CHECK();

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "bind group_id :%d,working_path :%d next_aps_en :%d mpls_tunnel:%d destmap_profile:%d edit_ptr_valid:%d\n",\
          group_id, p_bind_nh->working_path,p_bind_nh->next_aps_en,p_bind_nh->mpls_tunnel,p_bind_nh->destmap_profile,p_bind_nh->edit_ptr_valid);
    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "bind spme_en :%d l3if_id :%d dest_id:%d edit_ptr:%d nh_id(tunnel ID):%d\n",\
           p_bind_nh->spme_en,p_bind_nh->l3if_id,p_bind_nh->dest_id,p_bind_nh->edit_ptr,p_bind_nh->nh_id);
    SYS_APS_GROUP_ID_CHECK(group_id);

    /*check aps exist or not*/
    APS_LOCK(lchip);
    p_aps_bridge = ctc_vector_get(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);
    if (NULL == p_aps_bridge)
    {
        APS_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;

    }

    if(CTC_FLAG_ISSET(p_aps_bridge->flag,SYS_APS_FLAG_BIND_EN))
    {
      if((p_bind_nh->mpls_tunnel != CTC_FLAG_ISSET(p_aps_bridge->flag,SYS_APS_FLAG_BIND_MPLS_TUNNEL_EN))
        || (p_aps_bridge->nh_id != p_bind_nh->nh_id))
      { /*repeat bind  : one group only can bind pw nexthop or Mpls tunnel at same time. */

         SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " bind fail : old id :%d new id:%d \n",p_aps_bridge->nh_id, p_bind_nh->nh_id);

         APS_UNLOCK(lchip);
        return CTC_E_PARAM_CONFLICT;
      }
    }
     sal_memset(&ds_aps_bridge, 0, sizeof(ds_aps_bridge));
    cmd = DRV_IOR(DsApsBridge_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, group_id, cmd, &ds_aps_bridge), p_usw_aps_master[lchip]->p_aps_mutex);


    if (p_bind_nh->working_path)
    {
        if (p_bind_nh->next_aps_en)
        {
            SetDsApsBridge(V, workingNextApsBridgeEn_f, &ds_aps_bridge, 1);
            SetDsApsBridge(V, workingDestMap_f, &ds_aps_bridge, p_bind_nh->dest_id);
            CTC_SET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_NEXT_W_APS_EN);
        }
        else
        {
            if(p_bind_nh->destmap_profile)
            {
              SetDsApsBridge(V, workingDestMap_f, &ds_aps_bridge, SYS_ENCODE_ARP_DESTMAP(p_bind_nh->dest_id));
              dest_id = ((p_bind_nh->dest_id & 0x1ff) | 0x800);
            }
            else
            {
               gchip   = SYS_MAP_CTC_GPORT_TO_GCHIP(p_bind_nh->dest_id);
               dest_id = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_bind_nh->dest_id);
               SetDsApsBridge(V, workingDestMap_f, &ds_aps_bridge, SYS_ENCODE_DESTMAP(gchip, dest_id));
            }

            SetDsApsBridge(V, workingNextApsBridgeEn_f, &ds_aps_bridge, 0);
            SetDsApsBridge(V, workingIsLinkAggregation_f, &ds_aps_bridge, 0);
            SetDsApsBridge(V, workingRemoteChip_f, &ds_aps_bridge, 0);
            SetDsApsBridge(V, workingUcastId_f, &ds_aps_bridge, dest_id);
            if (CTC_IS_LINKAGG_PORT(p_bind_nh->dest_id))
            {
                SetDsApsBridge(V, workingIsLinkAggregation_f, &ds_aps_bridge, 1);
            }
            else if (FALSE == sys_usw_chip_is_local(lchip, gchip))
            {
                SetDsApsBridge(V, workingRemoteChip_f, &ds_aps_bridge, 1);
                SetDsApsBridge(V, workingUcastId_f, &ds_aps_bridge, 0);
            }

            if (p_bind_nh->l3if_id)
            {
                sal_memset(&l3if_prop, 0, sizeof(sys_l3if_prop_t));
                l3if_prop.l3if_id = p_bind_nh->l3if_id;
                sys_usw_l3if_get_l3if_info(lchip, 1, &l3if_prop);
                p_aps_bridge->w_l3if_id = p_bind_nh->l3if_id;
                SetDsApsBridge(V, workingDestVlanPtr_f, &ds_aps_bridge, l3if_prop.vlan_ptr);
            }
             CTC_UNSET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_NEXT_W_APS_EN);
        }
        if( p_bind_nh->edit_ptr_valid)
        {
          SetDsApsBridge(V,workingOuterEditPtr_f,&ds_aps_bridge, p_bind_nh->edit_ptr);
          SetDsApsBridge(V,workingOuterEditPtrType_f,&ds_aps_bridge, p_bind_nh->edit_ptr_type);
          SetDsApsBridge(V,workingOuterEditLocation_f,&ds_aps_bridge, p_bind_nh->edit_ptr_location);
        }
        CTC_SET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_BIND_W_EN);

    }
    else
    {

       if (p_bind_nh->next_aps_en)
        {
            SetDsApsBridge(V, protectingNextApsBridgeEn_f, &ds_aps_bridge, 1);
            SetDsApsBridge(V, protectingDestMap_f, &ds_aps_bridge, p_bind_nh->dest_id);
            CTC_SET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_NEXT_P_APS_EN);
        }
        else
        {
            if(p_bind_nh->destmap_profile)
            {
               SetDsApsBridge(V, protectingDestMap_f, &ds_aps_bridge, SYS_ENCODE_ARP_DESTMAP(p_bind_nh->dest_id));
               dest_id = ((p_bind_nh->dest_id & 0x1ff) | 0x800);
            }
            else
            {
               gchip   = SYS_MAP_CTC_GPORT_TO_GCHIP(p_bind_nh->dest_id);
               dest_id = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_bind_nh->dest_id);
               SetDsApsBridge(V, protectingDestMap_f, &ds_aps_bridge, SYS_ENCODE_DESTMAP(gchip, dest_id));
            }
            SetDsApsBridge(V, protectingNextApsBridgeEn_f, &ds_aps_bridge, 0);
            SetDsApsBridge(V, protectingUcastId_f, &ds_aps_bridge, dest_id);
            SetDsApsBridge(V, protectingIsLinkAggregation_f, &ds_aps_bridge, 0);
            if (CTC_IS_LINKAGG_PORT(p_bind_nh->dest_id))
            {
                SetDsApsBridge(V, protectingIsLinkAggregation_f, &ds_aps_bridge, 1);
            }
            else if (FALSE == sys_usw_chip_is_local(lchip, gchip))
            {
                SetDsApsBridge(V, protectingRemoteChip_f, &ds_aps_bridge, 1);
                SetDsApsBridge(V, protectingUcastId_f, &ds_aps_bridge, 0);
            }

            if (p_bind_nh->l3if_id)
            {
                sal_memset(&l3if_prop, 0, sizeof(sys_l3if_prop_t));
                l3if_prop.l3if_id = p_bind_nh->l3if_id;
                p_aps_bridge->p_l3if_id = p_bind_nh->l3if_id;
                sys_usw_l3if_get_l3if_info(lchip, 1, &l3if_prop);
                SetDsApsBridge(V, protectingDestVlanPtr_f, &ds_aps_bridge, l3if_prop.vlan_ptr);
            }
            CTC_UNSET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_NEXT_P_APS_EN);
        }

        if( p_bind_nh->edit_ptr_valid)
        {
          SetDsApsBridge(V,protectingOuterEditPtr_f,&ds_aps_bridge, p_bind_nh->edit_ptr);
          SetDsApsBridge(V,protectingOuterEditPtrType_f,&ds_aps_bridge, p_bind_nh->edit_ptr_type);
          SetDsApsBridge(V,protectingOuterEditLocation_f,&ds_aps_bridge, p_bind_nh->edit_ptr_location);
        }
        CTC_SET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_BIND_P_EN);
    }

    SetDsApsBridge(V, spmeApsEn_f, &ds_aps_bridge, p_bind_nh->spme_en);
    if (!p_bind_nh->mpls_tunnel)
    {
        /*different nexthop should only be set at create mpls nexthop, tunnel update donot set this bit*/
        SetDsApsBridge(V, differentNexthop_f, &ds_aps_bridge, p_bind_nh->different_nh);
    }
    cmd = DRV_IOW(DsApsBridge_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, group_id, cmd, &ds_aps_bridge), p_usw_aps_master[lchip]->p_aps_mutex);

    if(p_bind_nh->mpls_tunnel)
    {
        CTC_SET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_BIND_MPLS_TUNNEL_EN);
    }
    p_aps_bridge->nh_id =  p_bind_nh->nh_id;
    CTC_SET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_BIND_EN);

    APS_UNLOCK(lchip);

   return CTC_E_NONE;
}

int32
sys_usw_aps_unbind_nexthop(uint8 lchip,uint16 group_id,sys_aps_bind_nh_param_t *p_bind_nh)
{

    uint32 cmd;
    sys_aps_bridge_t* p_aps_bridge = NULL;
    DsApsBridge_m ds_aps_bridge;
    uint8 gchip = 0;
    uint16 dest_id = 0;


    SYS_APS_INIT_CHECK();

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "bind group_id :%d working_path :%d next_aps_en :%d mpls_tunnel:%d destmap_profile:%d edit_ptr_valid:%d\n",\
         group_id, p_bind_nh->working_path,p_bind_nh->next_aps_en,p_bind_nh->mpls_tunnel,p_bind_nh->destmap_profile,p_bind_nh->edit_ptr_valid);
    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "bind spme_en :%d l3if_id :%d dest_id:%d edit_ptr:%d nh_id(tunnel ID):%d\n",\
           p_bind_nh->spme_en,p_bind_nh->l3if_id,p_bind_nh->dest_id,p_bind_nh->edit_ptr,p_bind_nh->nh_id);

    SYS_APS_GROUP_ID_CHECK(group_id);

    /*check aps exist or not*/
    APS_LOCK(lchip);
    p_aps_bridge = ctc_vector_get(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);
    if (NULL == p_aps_bridge)
    {
        CTC_ERROR_RETURN_WITH_UNLOCK(CTC_E_NOT_EXIST, p_usw_aps_master[lchip]->p_aps_mutex);
    }

    if(CTC_FLAG_ISSET(p_aps_bridge->flag,SYS_APS_FLAG_BIND_EN))
    {
      if((p_bind_nh->mpls_tunnel != CTC_FLAG_ISSET(p_aps_bridge->flag,SYS_APS_FLAG_BIND_MPLS_TUNNEL_EN))
        || (p_aps_bridge->nh_id != p_bind_nh->nh_id))
      { /*repeat bind  : one group only can bind pw nexthop or Mpls tunnel at same time. */
         SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "unbind fail : old id :%d new id:%d \n",p_aps_bridge->nh_id, p_bind_nh->nh_id);
         APS_UNLOCK(lchip);
         return CTC_E_PARAM_CONFLICT;
      }
    }

    sal_memset(&ds_aps_bridge, 0, sizeof(ds_aps_bridge));
    cmd = DRV_IOR(DsApsBridge_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, group_id, cmd, &ds_aps_bridge), p_usw_aps_master[lchip]->p_aps_mutex);

    dest_id =  SYS_RSV_PORT_DROP_ID ;
    sys_usw_get_gchip_id( lchip,  &gchip);
    if (p_bind_nh->working_path)
    {
         SetDsApsBridge(V, workingDestMap_f, &ds_aps_bridge, SYS_ENCODE_DESTMAP(gchip, dest_id));
         SetDsApsBridge(V, workingNextApsBridgeEn_f, &ds_aps_bridge, 0);
         SetDsApsBridge(V, workingIsLinkAggregation_f, &ds_aps_bridge, 0);
         SetDsApsBridge(V, workingRemoteChip_f, &ds_aps_bridge, 0);
         SetDsApsBridge(V, workingUcastId_f, &ds_aps_bridge, dest_id);
         SetDsApsBridge(V, workingDestVlanPtr_f, &ds_aps_bridge, 0);
         CTC_UNSET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_NEXT_W_APS_EN);
         CTC_UNSET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_BIND_W_EN);

    }
    else
    {
        SetDsApsBridge(V, protectingDestMap_f, &ds_aps_bridge, SYS_ENCODE_DESTMAP(gchip, dest_id));
        SetDsApsBridge(V, protectingUcastId_f, &ds_aps_bridge, dest_id);
        SetDsApsBridge(V, protectingNextApsBridgeEn_f, &ds_aps_bridge, 0);
        SetDsApsBridge(V, protectingIsLinkAggregation_f, &ds_aps_bridge, 0);
        SetDsApsBridge(V, protectingDestVlanPtr_f, &ds_aps_bridge, 0);
        CTC_UNSET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_NEXT_P_APS_EN);

        CTC_UNSET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_BIND_P_EN);

    }
    SetDsApsBridge(V, spmeApsEn_f, &ds_aps_bridge, 0);
    cmd = DRV_IOW(DsApsBridge_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, group_id, cmd, &ds_aps_bridge), p_usw_aps_master[lchip]->p_aps_mutex);
    if(!CTC_FLAG_ISSET(p_aps_bridge->flag,SYS_APS_FLAG_BIND_W_EN)
    && !CTC_FLAG_ISSET(p_aps_bridge->flag,SYS_APS_FLAG_BIND_P_EN))
    {
        CTC_UNSET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_BIND_MPLS_TUNNEL_EN);
       p_aps_bridge->nh_id =  0;
       CTC_UNSET_FLAG(p_aps_bridge->flag,SYS_APS_FLAG_BIND_EN);
    }
    APS_UNLOCK(lchip);
    return CTC_E_NONE;
}
int32
sys_usw_aps_get_ports(uint8 lchip, uint16 group_id, ctc_aps_bridge_group_t* aps_group)
{
    int32 ret = 0;

    SYS_APS_INIT_CHECK();
    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_APS_GROUP_ID_CHECK(group_id);

    APS_LOCK(lchip);

    ret = _sys_usw_aps_get_ports(lchip, group_id, aps_group);
    if (ret < 0)
    {
        APS_UNLOCK(lchip);
        return ret;
    }

    APS_UNLOCK(lchip);
    return CTC_E_NONE;

}

int32
sys_usw_aps_show_status(uint8 lchip)
{
    uint32 entry_num = 0;

    SYS_APS_INIT_CHECK();

    sys_usw_ftm_query_table_entry_num(0, DsApsBridge_t, &entry_num);

    SYS_APS_DUMP("-----------APS status------------\n");
    SYS_APS_DUMP("%-22s: %d\n", "APS group total", entry_num);
    SYS_APS_DUMP("%-22s: %d\n", "APS group allocated", p_usw_aps_master[lchip]->aps_used_num);

    return CTC_E_NONE;
}

int32
sys_usw_aps_ftm_cb(uint8 lchip, uint32 *entry_num)
{
    LCHIP_CHECK(lchip);
    SYS_APS_INIT_CHECK();

    *entry_num = p_usw_aps_master[lchip]->aps_used_num;
    
    return CTC_E_NONE;
}


int32
sys_usw_aps_show_bridge(uint8 lchip, uint16 group_id, uint8 detail)
{
    sys_aps_bridge_t* p_aps_bridge = NULL;
    uint32 entry_num = 0;
    uint16 i = 0;
    SYS_APS_INIT_CHECK();

    APS_LOCK(lchip);

    sys_usw_ftm_query_table_entry_num(lchip, DsApsBridge_t, &entry_num);

    if (0xFFFF == group_id)
    {
        if (0 == p_usw_aps_master[lchip]->aps_used_num)
        {
            SYS_APS_DUMP("Not any aps group!!\n");
            SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Group dont exist \n");
            APS_UNLOCK(lchip);
			return CTC_E_NOT_EXIST;
        }
    }
    else
    {
        p_aps_bridge = ctc_vector_get(p_usw_aps_master[lchip]->aps_bridge_vector, group_id);
        if (NULL == p_aps_bridge)
        {
            SYS_APS_DUMP("No aps group %d!!\n", group_id);
            SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Group dont exist \n");
            APS_UNLOCK(lchip);
			return CTC_E_NOT_EXIST;
        }
    }

    SYS_APS_DUMP("Flag: %-15s\n", "P - Port");
    SYS_APS_DUMP("      %-15s\n",  "G - APS Group");
    SYS_APS_DUMP("      %-15s\n\n", "R - RAPS Group");
    SYS_APS_DUMP("-------------------------------------------------------------\n");
    SYS_APS_DUMP("%-6s%-13s%-7s%-9s%-10s%-7s%-8s%-6s%-6s\n", "G_ID", "W", "W-Flag", "W-IFID", "P", "P-Flag", "P-IFID", "W/P", "HW");
    SYS_APS_DUMP("-------------------------------------------------------------\n");


    if (0xFFFF == group_id)
    {
        for (i = 0; i < entry_num; i++)
        {
           _sys_usw_aps_show_one_bridge(lchip, i, detail);
        }
    }
    else
    {
        _sys_usw_aps_show_one_bridge(lchip, group_id, detail);
    }

    APS_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_aps_failover_detect(uint8 lchip, uint32 intr, void* p_data)
{
    uint32 cmd = 0;
    MetFifoLinkState_m chan_down;
    uint8 index = 0;
    uint32 entry_num = 0;

    sys_usw_ftm_query_table_entry_num(lchip, MetFifoLinkState_t, &entry_num);
    SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "MetFifoLinkState entry num :%d\n", entry_num);

    /* APS protect switch done interrupt */
    for (index = 0; index < entry_num; index++)
    {
        cmd = DRV_IOR(MetFifoLinkState_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &chan_down));

        if (GetMetFifoLinkState(V, linkDown_f,&chan_down))
        {
            SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Aps channel failover, channel Id:0x%x\n ", index);
            cmd = DRV_IOR(MetFifoLinkState_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &chan_down));
            SetMetFifoLinkState(V, linkDown_f, &chan_down, 0);
            cmd = DRV_IOW(MetFifoLinkState_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &chan_down));
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_aps_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_wb_aps_master_t* p_wb_aps_master = NULL;
    sys_traverse_t  wb_aps_traverse;
    uint8  work_status = 0;

    SYS_APS_INIT_CHECK();
    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	return CTC_E_NONE;
    }
    /*syncup  aps_matser*/
    wb_data.buffer = mem_malloc(MEM_APS_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_aps_master_t, CTC_FEATURE_APS, SYS_WB_APPID_APS_SUBID_MASTER);

    p_wb_aps_master = (sys_wb_aps_master_t  *)wb_data.buffer;

    p_wb_aps_master->version = SYS_WB_VERSION_APS;
    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    /*sync aps nodes*/
    wb_aps_traverse.data = &wb_data;
    wb_aps_traverse.value1 = lchip;
    wb_aps_traverse.value2 = 0;
    wb_aps_traverse.value3 = 0;
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_aps_node_t, CTC_FEATURE_APS, SYS_WB_APPID_APS_SUBID_NODE);
    ctc_vector_traverse2(p_usw_aps_master[lchip]->aps_bridge_vector, 0, _sys_usw_aps_wb_traverse_sync_node, &wb_aps_traverse);

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
sys_usw_aps_dump_db(uint8 lchip, sal_file_t dump_db_fp,ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;

    SYS_APS_INIT_CHECK();
    APS_LOCK(lchip);

    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "# APS");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "{");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","aps_used_num",p_usw_aps_master[lchip]->aps_used_num);
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "}");

    APS_UNLOCK(lchip);

    return ret;
}

int32
sys_usw_aps_wb_restore(uint8 lchip)
{
    ctc_wb_query_t    wb_query;
    ctc_wb_query_t* p_wb_query = &wb_query;
    sys_wb_aps_master_t wb_aps_master ;
    int32 ret = 0;
    uint16 entry_cnt = 0;
    sys_wb_aps_node_t  wb_aps_node ;
    sys_aps_bridge_t*aps_node =NULL;

    if (NULL == p_usw_aps_master[lchip] || !p_usw_aps_master[lchip]->aps_bridge_vector)
    {
        SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        return CTC_E_NOT_INIT;
    }

    /*restore APS*/
    wb_query.buffer = mem_malloc(MEM_APS_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_aps_master_t, CTC_FEATURE_APS, SYS_WB_APPID_APS_SUBID_MASTER);

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);

    /*restore  aps_master*/
    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "query aps master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_NONE;
        goto done;
    }

    sal_memcpy(&wb_aps_master,wb_query.buffer, wb_query.key_len+wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_APS, wb_aps_master.version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_aps_node_t, CTC_FEATURE_APS, SYS_WB_APPID_APS_SUBID_NODE);

    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);
    sal_memcpy(&wb_aps_node, (sys_wb_aps_node_t*)wb_query.buffer + entry_cnt++, wb_query.key_len+wb_query.data_len);
    aps_node = mem_malloc(MEM_APS_MODULE, sizeof(sys_aps_bridge_t));
    if (NULL == aps_node)
    {
        SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        goto done;
    }
    sal_memset(aps_node, 0, sizeof(sys_aps_bridge_t));
    aps_node->flag = wb_aps_node.flag;
    aps_node->raps_group_id = wb_aps_node.raps_group_id;
    aps_node->p_l3if_id = wb_aps_node.p_l3if_id;
    aps_node->w_l3if_id = wb_aps_node.w_l3if_id;
    aps_node->nh_id = wb_aps_node.nh_id;

    ctc_vector_add(p_usw_aps_master[lchip]->aps_bridge_vector,  wb_aps_node.group_id, aps_node);

    p_usw_aps_master[lchip]->aps_used_num++;

    CTC_WB_QUERY_ENTRY_END(p_wb_query);

done:
    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    if (wb_query.key)
    {
        mem_free(wb_query.key);
    }

    return ret;
}

int32
sys_usw_aps_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint16 index = 0;
    DsApsBridge_m ds_aps_bridge;
    MetFifoCtl_m ds_met_fifo;
    uint32 entry_num = 0;
    uint8  work_status = 0;

    if (NULL != p_usw_aps_master[lchip])
    {
        return CTC_E_NONE;
    }
    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
    }

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsApsBridge_t,
                                                                    &entry_num));
    if (!entry_num)
    {
        return CTC_E_NOT_INIT;
    }

    p_usw_aps_master[lchip] = mem_malloc(MEM_APS_MODULE, sizeof(sys_aps_master_t));
    if (NULL == p_usw_aps_master[lchip])
    {
        SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }

    sal_memset(p_usw_aps_master[lchip], 0, sizeof(sys_aps_master_t));
#ifndef PACKET_TX_USE_SPINLOCK
    sal_mutex_create(&(p_usw_aps_master[lchip]->p_aps_mutex));
#else
    sal_spinlock_create((sal_spinlock_t**)&(p_usw_aps_master[lchip]->p_aps_mutex));
#endif
    if (NULL == p_usw_aps_master[lchip]->p_aps_mutex)
    {
        mem_free(p_usw_aps_master[lchip]);
        SYS_APS_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Create mutex fail\n");
		return CTC_E_NO_MEMORY;
    }

    p_usw_aps_master[lchip]->aps_bridge_vector
        = ctc_vector_init(CTC_APS_GROUP_BLOCK, entry_num / CTC_APS_GROUP_BLOCK);
    if (NULL == p_usw_aps_master[lchip]->aps_bridge_vector)
    {
        sal_mutex_destroy(p_usw_aps_master[lchip]->p_aps_mutex);
        mem_free(p_usw_aps_master[lchip]);
        return CTC_E_NO_MEMORY;
    }
    ctc_vector_reserve(p_usw_aps_master[lchip]->aps_bridge_vector, 1);

    sal_memset(&ds_aps_bridge, 0, sizeof(DsApsBridge_m));
    cmd = DRV_IOW(DsApsBridge_t, DRV_ENTRY_FLAG);
    for (index = 0; index < entry_num; index++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_aps_bridge));
    }
    entry_num = (0 == entry_num) ? 0 : (entry_num -1);

     /* configure hw-based failover aps*/
    cmd = DRV_IOR(MetFifoCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_met_fifo));
    SetMetFifoCtl(V, linkChangeEn_f, &ds_met_fifo, 1);
    SetMetFifoCtl(V, maxApsGroupNum_f, &ds_met_fifo, entry_num);
    cmd = DRV_IOW(MetFifoCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_met_fifo));

    /*register hw-failover isr callback*/
    sys_usw_interrupt_register_isr(lchip, SYS_INTR_FUNC_MET_LINK_SCAN_DONE, sys_usw_aps_failover_detect);
    /* dump-db register */
    CTC_ERROR_RETURN(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_APS, sys_usw_aps_dump_db));

    CTC_ERROR_RETURN(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_APS, sys_usw_aps_wb_sync));

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_aps_wb_restore(lchip));
    }

    MCHIP_CAP(SYS_CAP_SPEC_APS_GROUP_NUM) = entry_num;
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    return CTC_E_NONE;
}

int32
sys_usw_aps_deinit(uint8 lchip)
{
    if (!p_usw_aps_master[lchip])
    {
        return CTC_E_NONE;
    }

    /*free vector data*/
    ctc_vector_traverse(p_usw_aps_master[lchip]->aps_bridge_vector, (vector_traversal_fn)_sys_usw_aps_free_node_data, NULL);
    ctc_vector_release(p_usw_aps_master[lchip]->aps_bridge_vector);

#ifndef PACKET_TX_USE_SPINLOCK
    sal_mutex_destroy(p_usw_aps_master[lchip]->p_aps_mutex);
#else
    sal_spinlock_destroy((sal_spinlock_t*)p_usw_aps_master[lchip]->p_aps_mutex);
#endif
    mem_free(p_usw_aps_master[lchip]);

    return 0;
}
