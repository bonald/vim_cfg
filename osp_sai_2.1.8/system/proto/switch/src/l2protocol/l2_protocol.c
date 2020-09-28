#include "proto.h"
#include "lib_tblinc.h"
#include "glb_if_define.h"
#include "glb_l2_define.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_l2_protocol_define.h"
#include "gen/tbl_l2_protocol.h"
#include "gen/tbl_brg_global_define.h" 
#include "gen/tbl_brg_global.h" 
#include "gen/tbl_interface_define.h" 
#include "gen/tbl_interface.h" 
#include "gen/tbl_vlan_define.h"
#include "ctclib_opb.h"
#include "switch.h"
#include "errdisable.h"
#include "l2_protocol.h"
#include "if_agg.h"
#include "if_mgr.h"
#include "hsrv_msg.h"
#include "hsrv_msg_if.h"

extern mac_addr_t l2pro_lldp_dmac;
extern mac_addr_t l2pro_bpdu_dmac;
extern mac_addr_t l2pro_lacp_dmac;
extern mac_addr_t l2pro_eapal_dmac;

ctclib_opb_t    l2protocol_index_opb;

int32
l2_protocol_add_l2_protocol(tbl_l2_protocol_t* p_l2_protocol)
{
    int32 rc = PM_E_NONE;
    tbl_l2_protocol_t* p_db_l2_protocol = NULL;
    
    p_db_l2_protocol = tbl_l2_protocol_get_l2_protocol(&(p_l2_protocol->key));
    if (NULL != p_db_l2_protocol)
    {
        return PM_E_NONE;
    }
    PM_E_RETURN(tbl_l2_protocol_add_l2_protocol(p_l2_protocol));
    return rc;
}

int32
l2_protocol_set_l2_protocol_group_mac(tbl_l2_protocol_t* p_l2_protocol)
{
    int32 rc = PM_E_NONE;
    tbl_l2_protocol_t* p_db_l2_protocol = NULL;
    
    p_db_l2_protocol = tbl_l2_protocol_get_l2_protocol(&(p_l2_protocol->key));
    if (NULL == p_db_l2_protocol)
    {
        return PM_E_NOT_EXIST;
    }
    PM_E_RETURN(tbl_l2_protocol_set_l2_protocol_field(p_l2_protocol, TBL_L2_PROTOCOL_FLD_GROUP_MAC));
    return rc;
}

int32
l2_protocol_del_user_define_protocol_on_interface(tbl_l2_protocol_t* p_db_l2protocol)
{
    int32 rc = PM_E_NONE;
    uint32  proto_index = 0;
    tbl_interface_t* p_db_if = NULL;
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    
    ds_l2protocol_t* p_ds_l2protocol = NULL;
    ctclib_slistnode_t *listnode1 = NULL;
    ctclib_slistnode_t *next = NULL;

    proto_index = p_db_l2protocol->protocol_index;
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (GLB_IF_MODE_L2 != p_db_if->mode)
        {
            continue;
        }
        if (INTERFACE_IS_LAG_MEMBER(p_db_if))
        {
            continue;
        }
        if ((p_db_if->hw_type != GLB_IF_TYPE_PORT_IF) && 
            (p_db_if->hw_type != GLB_IF_TYPE_PORT_IF))
        {
            continue;
        }
        CTCLIB_SLIST_LOOP_DEL(p_db_if->l2protocol.obj_list, p_ds_l2protocol, listnode1, next)
        {
            if ((0 == sal_strcmp(p_db_l2protocol->key.name, p_ds_l2protocol->key.l2pro_name)))
            {
                PM_E_RETURN(ds_l2protocol_del_l2protocol(p_db_if, p_ds_l2protocol));
            }
        }
        p_db_if->l2pro_if_action_vlan.l2action[proto_index] = 0; 
        p_db_if->l2pro_if_action_vlan.l2vlan[proto_index] = 0; 
        PM_E_RETURN(tbl_interface_set_interface_field(p_db_if, TBL_INTERFACE_FLD_L2PRO_IF_ACTION_VLAN));
    }
    return rc;
}

int32
l2_protocol_init(void)
{
    int32 rc = PM_E_NONE;
    tbl_l2_protocol_t l2protocol;
    tbl_l2_protocol_t* p_db_l2protocol = NULL;

    /*add l2_protocol table of dot1x*/
    sal_memset(&l2protocol, 0, sizeof(l2protocol));
    sal_strncpy(l2protocol.key.name, "dot1x", GLB_L2_PROTOCOL_NAME_SIZE+1);
    p_db_l2protocol = tbl_l2_protocol_get_l2_protocol((&l2protocol.key));
    if (NULL == p_db_l2protocol)
    {
        l2protocol.protocol_index = GLB_L2_PROTO_INDEX_0;
        l2protocol.is_standard_pro = TRUE;
        sal_memcpy(l2protocol.protocol_mac, l2pro_eapal_dmac, MAC_ADDR_LEN);
        PM_E_RETURN(tbl_l2_protocol_add_l2_protocol(&l2protocol));
    }

    /*add l2_protocol table of lldp*/
    sal_memset(&l2protocol, 0, sizeof(l2protocol));
    sal_strncpy(l2protocol.key.name, "lldp", GLB_L2_PROTOCOL_NAME_SIZE+1);
    p_db_l2protocol = tbl_l2_protocol_get_l2_protocol((&l2protocol.key));
    if (NULL == p_db_l2protocol)
    {
        l2protocol.protocol_index = GLB_L2_PROTO_INDEX_1;
        l2protocol.is_standard_pro = TRUE;
        sal_memcpy(l2protocol.protocol_mac, l2pro_lldp_dmac, MAC_ADDR_LEN);
        PM_E_RETURN(tbl_l2_protocol_add_l2_protocol(&l2protocol));
    }

    /*add l2_protocol table of slow-proto*/
    sal_memset(&l2protocol, 0, sizeof(l2protocol));
    sal_strncpy(l2protocol.key.name, "slow-proto", GLB_L2_PROTOCOL_NAME_SIZE+1);
    p_db_l2protocol = tbl_l2_protocol_get_l2_protocol((&l2protocol.key));
    if (NULL == p_db_l2protocol)
    {
        l2protocol.protocol_index = GLB_L2_PROTO_INDEX_2;
        l2protocol.is_standard_pro = TRUE;
        sal_memcpy(l2protocol.protocol_mac, l2pro_lacp_dmac, MAC_ADDR_LEN);
        PM_E_RETURN(tbl_l2_protocol_add_l2_protocol(&l2protocol));
    }

    /*add l2_protocol table of stp*/
    sal_memset(&l2protocol, 0, sizeof(l2protocol));
    sal_strncpy(l2protocol.key.name, "stp", GLB_L2_PROTOCOL_NAME_SIZE+1);
    p_db_l2protocol = tbl_l2_protocol_get_l2_protocol((&l2protocol.key));
    if (NULL == p_db_l2protocol)
    {
        l2protocol.protocol_index = GLB_L2_PROTO_INDEX_3;
        l2protocol.is_standard_pro = TRUE;
        sal_memcpy(l2protocol.protocol_mac, l2pro_bpdu_dmac, MAC_ADDR_LEN);
        PM_E_RETURN(tbl_l2_protocol_add_l2_protocol(&l2protocol));
    }
    return rc;
}

int32
l2_protocol_start(switch_master_t *master)
{
    cdb_register_cfg_tbl_cb(TBL_L2_PROTOCOL, l2_protocol_cmd_process_l2_protocol);
    cdb_register_cfg_act_cb(ACT_SHOW_l2_PROTOCOL, l2_protocol_cmd_show_l2_protocol_process);

    /*Added by yejl for l2protocol */
    ctclib_opb_create(&l2protocol_index_opb, 4, GLB_L2_PROTOCOL_NUM, "l2protocol index");
    /*end by yejl*/

    return PM_E_NONE;
}

