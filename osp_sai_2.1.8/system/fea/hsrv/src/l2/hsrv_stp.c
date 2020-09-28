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
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_global.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_instance.h"
#include "gen/tbl_erps_domain_define.h"
#include "gen/tbl_erps_domain.h"
#include "gen/tbl_erps_ring_define.h"
#include "gen/tbl_erps_ring.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/

/******************************************************************************
* Defines and Macros
******************************************************************************/

int32
_hsrv_stp_add_instance_set_fea(uint32 instance)
{
    tbl_mstp_global_t *p_global = tbl_mstp_global_get_mstp_global();
    tbl_mstp_port_master_t *p_master = tbl_mstp_port_get_master();
    tbl_mstp_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 fea_state = 0;
    
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        if (p_global->enable)
        {
            if (p_db_port->port_enable)
            {
                fea_state = GLB_BR_PORT_STATE_BLOCKING;
            }
            else
            {
                fea_state = GLB_BR_PORT_STATE_DISABLED;
            }
        }
        else
        {
            fea_state = GLB_BR_PORT_STATE_DISABLED;
        }
        hsrv_stp_set_port_state(p_db_port->ifindex, instance, fea_state);
        hsrv_brgif_set_msti_port_state(p_db_port->ifindex, instance, fea_state);
    }

    return HSRV_E_NONE;
}

int32
hsrv_stp_add_instance(uint32 instance)
{
    sai_stp_api_t*  stp_api = NULL;
    tbl_mstp_instance_key_t key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    sai_object_id_t inst_oid = 0;

    key.instance = instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&key);
    if (NULL == p_db_inst)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_STP,(void**)&stp_api));
    
    stp_api->create_stp(&inst_oid, 0, NULL);
    p_db_inst->instance_oid = inst_oid;
    tbl_mstp_instance_set_mstp_instance_field(p_db_inst, TBL_MSTP_INSTANCE_FLD_INSTANCE_OID);

    _hsrv_stp_add_instance_set_fea(instance);

    return HSRV_E_NONE;
}

int32
hsrv_stp_del_instance(uint32 instance)
{
    sai_stp_api_t*  stp_api = NULL;
    tbl_mstp_instance_key_t key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    sai_object_id_t inst_oid = 0;

    key.instance = instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&key);
    if (NULL == p_db_inst)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    
    inst_oid = p_db_inst->instance_oid;
    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_STP,(void**)&stp_api));
    
    stp_api->remove_stp(inst_oid);

    return HSRV_E_NONE;
}

uint32 g_stp_set_state_count = 0;

int32
hsrv_stp_set_port_state(uint32 ifindex, uint32 instance, uint32 fea_state)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_t *p_db_mem_if = NULL;
    sai_stp_api_t*  stp_api = NULL;
    sai_object_id_t inst_oid;
    sai_object_id_t port_oid;   
    sai_port_stp_port_state_t sai_state;

    g_stp_set_state_count++;
//    log_sys(M_MOD_HSRV, E_ERROR, "KCAODEBUG count %u, ifindex %u, instance %u, fea_state %u\n", g_stp_set_state_count, ifindex, instance, fea_state);
    
    switch (fea_state)
    {
    case GLB_BR_PORT_STATE_BLOCKING:
        sai_state = SAI_PORT_STP_STATE_BLOCKING;
        break;
    case GLB_BR_PORT_STATE_FORWARDING:
        sai_state = SAI_PORT_STP_STATE_FORWARDING;
        break;
    case GLB_BR_PORT_STATE_LISTENING:
    case GLB_BR_PORT_STATE_LEARNING:
        sai_state = SAI_PORT_STP_STATE_LEARNING;
        break;
    case GLB_BR_PORT_STATE_DISABLED:
        sai_state = SAI_PORT_STP_STATE_FORWARDING;
        break;
    default:
        return HSRV_E_INVALID_PARAM;
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_STP,(void**)&stp_api));
    if (GLB_DEFAULT_INSTANCE == instance)
    {
        inst_oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_STP_INSTANCE, instance);
    }
    else
    {
        inst_key.instance = instance;
        p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
        if (NULL == p_db_inst)
        {
            return HSRV_E_INVALID_PTR;
        }
        inst_oid = p_db_inst->instance_oid;
    }

    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (NULL == p_db_if)
    {
        /* check interface exist */
        return HSRV_E_NONE;
    }

    /* modified by liwh for bug 54357, 2019-11-12 */
    if (GLB_BR_PORT_STATE_DISABLED == fea_state)
    {
        if (p_db_if->current_g8032_ring_count)
        {
            return HSRV_E_NONE;   
        }

        if (p_db_if->erps_enable)
        {
             return HSRV_E_NONE;    
        }
    }
    /* liwh end */

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        if (NULL == p_db_if->lag)
        {
            return HSRV_E_NONE;
        }

        /* LAG should iterate all member ports */
        CTCLIB_SLIST_LOOP(p_db_if->lag->member_ports.obj_list, p_db_mem_if, listnode)
        {
            port_oid = p_db_mem_if->portid;
            stp_api->set_stp_port_state(inst_oid, port_oid, sai_state);
        }
    }
    else
    {
        port_oid = p_db_if->portid;
        HSRV_IF_ERROR_RETURN(stp_api->set_stp_port_state(inst_oid, port_oid, sai_state));
    }

    return HSRV_E_NONE;
}

char*
hsrv_stp_state_str(uint32 state)
{
    switch (state)
    {
    case GLB_BR_PORT_STATE_DISABLED:
        return "disabled";
    case GLB_BR_PORT_STATE_LISTENING:
        return "listening";
    case GLB_BR_PORT_STATE_LEARNING:
        return "learning";
    case GLB_BR_PORT_STATE_FORWARDING:
        return "forwarding";
    case GLB_BR_PORT_STATE_BLOCKING:
        return "blocking";
    default:
        return "invalid";
    }
}

int32
hsrv_mstp_port_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_mstp_port_t* p_mstp_port = NULL;
    uint32 instance = MSTP_DEFAULT_INSTANCE;

    p_mstp_port = (tbl_mstp_port_t*)p_tbl;

    switch (oper)
    {
    case CDB_OPER_SET:
        switch (field_id)
        {
        case TBL_MSTP_PORT_FLD_FEA_STATE:
            HSRV_STP_DEBUG("FEA set port %s instance %u state %s", p_mstp_port->key.name, instance, hsrv_stp_state_str(p_mstp_port->fea_state));
            HSRV_IF_ERROR_RETURN(hsrv_stp_set_port_state(p_mstp_port->ifindex, instance, p_mstp_port->fea_state));
            HSRV_IF_ERROR_RETURN(hsrv_brgif_set_mstp_port_state(p_mstp_port->ifindex, p_mstp_port->fea_state));
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
hsrv_msti_port_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_msti_port_t* p_msti_port = NULL;

    p_msti_port = (tbl_msti_port_t*)p_tbl;

    switch (oper)
    {
    case CDB_OPER_SET:
        switch (field_id)
        {
        case TBL_MSTI_PORT_FLD_FEA_STATE:
            HSRV_STP_DEBUG("FEA set port %s instance %u state %s", p_msti_port->key.name, p_msti_port->key.instance, hsrv_stp_state_str(p_msti_port->fea_state));
            HSRV_IF_ERROR_RETURN(hsrv_stp_set_port_state(p_msti_port->ifindex, p_msti_port->key.instance, p_msti_port->fea_state));
            HSRV_IF_ERROR_RETURN(hsrv_brgif_set_msti_port_state(p_msti_port->ifindex, p_msti_port->key.instance, p_msti_port->fea_state));
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
hsrv_mstp_instance_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_mstp_instance_t* p_mstp_inst = NULL;

    p_mstp_inst = (tbl_mstp_instance_t*)p_tbl;

    switch (oper)
    {
    case CDB_OPER_DEL:
            HSRV_IF_ERROR_RETURN(hsrv_stp_del_instance(p_mstp_inst->key.instance));
            break;
    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_mstp_instance_sync_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_mstp_instance_t* p_mstp_inst = NULL;

    p_mstp_inst = (tbl_mstp_instance_t*)p_tbl;

    switch (oper)
    {
    case CDB_OPER_ADD:
            HSRV_IF_ERROR_RETURN(hsrv_stp_add_instance(p_mstp_inst->key.instance));
            break;
    default:
        break;
    }

    return HSRV_E_NONE;
}

int32
hsrv_erps_ring_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_erps_ring_t           *p_ring = NULL;
    tbl_mstp_instance_key_t   inst_key;
    tbl_mstp_instance_t       *p_db_inst = NULL;
    uint32                    index = 0;

    p_ring = (tbl_erps_ring_t*)p_tbl;

    /* modified by wangjj for fix erps bug 41054, 2016-12-01 */
    #if 0
    tbl_erps_ring_t* p_db_ring = NULL;
    p_db_ring = tbl_erps_ring_get_erps_ring(&ring_key);
    if (!p_db_ring)
    {
        HSRV_LOG_ERR("ERPS ring %u hasn't been created", p_ring->key.ring_id);
        return HSRV_E_NONE;
    }
    #endif

    switch (oper)
    {
    case CDB_OPER_SET:
        switch (field_id)
        {
        case TBL_ERPS_RING_FLD_BLOCKED_PORT_IFINDEX: 
            /* modified by liwh for bug 42704, 2017-03-13 */
            if (p_ring && (0 != p_ring->instance_id_num))
            {
                for(index = 0; index < p_ring->instance_id_num; index++)
                {
                    inst_key.instance = p_ring->instance_id[index];
                    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
                    if (NULL == p_db_inst)
                    {
                        continue;
                    }

                    HSRV_STP_DEBUG("ERPS ring %d %d set port ifindex %d instance %d blocked %d", 
                        p_ring->key.domain_id, p_ring->key.ring_id, p_ring->blocked_port_ifindex, 
                        p_ring->instance_id[index], p_ring->blocked);
                    
                    if (p_ring->blocked)
                    {
                        HSRV_IF_ERROR_RETURN(hsrv_stp_set_port_state(p_ring->blocked_port_ifindex, 
                            p_ring->instance_id[index], GLB_BR_PORT_STATE_BLOCKING));
                        hsrv_brgif_set_msti_port_state(p_ring->blocked_port_ifindex, p_ring->instance_id[index], GLB_BR_PORT_STATE_BLOCKING);
                    }
                    else
                    {
                        HSRV_IF_ERROR_RETURN(hsrv_stp_set_port_state(p_ring->blocked_port_ifindex, 
                            p_ring->instance_id[index], GLB_BR_PORT_STATE_FORWARDING));
                        hsrv_brgif_set_msti_port_state(p_ring->blocked_port_ifindex, p_ring->instance_id[index], GLB_BR_PORT_STATE_FORWARDING);
                    }
                }
            }
            /* liwh end */
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

char*
hsrv_stp_ctc_state_str(uint8 state)
{
    switch (state)
    {
    case CTC_STP_FORWARDING:
        return "Forwarding";
    case CTC_STP_BLOCKING:
        return "Blocking";
    case CTC_STP_LEARNING:
        return "Learning";
    default:
        return "Invalid";
    }
}

int32
hsrv_stp_show_msti_port_one(tbl_mstp_port_t *p_db_port, tbl_mstp_instance_t *p_db_inst, uint32 gport, FILE *fp)
{
    uint8 stp_state = 0;
    uint8 stpid = 0;
    int32 rc = 0;

    stpid = CTC_SAI_OBJECT_INDEX_GET(p_db_inst->instance_oid);
    rc = ctc_stp_get_state(gport, stpid, &stp_state);
    if (rc < 0)
    {
        sal_fprintf(fp, "Inst[%u, %u]: rc %d\n", p_db_inst->key.instance, stpid, rc);
        return HSRV_E_NONE;
    }
    sal_fprintf(fp, "Inst[%u, %u]: %u(%s)\n", p_db_inst->key.instance, stpid, stp_state, hsrv_stp_ctc_state_str(stp_state));

    return HSRV_E_NONE;
}

int32
hsrv_stp_show_one_interface(tbl_mstp_port_t *p_db_port, tbl_mstp_instance_t *p_db_inst, FILE *fp)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_msti_port_t *p_db_msti_port = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_interface_t *p_db_mem_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *listnode_lag = NULL;
    tbl_mstp_instance_t *p_db_inst_iter = NULL;
    uint32 gport = 0;
    char ifname_ext[IFNAME_EXT_SIZE];
    char ifname_ext_mem[IFNAME_EXT_SIZE];
    
    p_db_if = tbl_interface_get_interface_by_name(p_db_port->key.name);
    if (NULL == p_db_if)
    {
        IFNAME_ETH2SHORT(p_db_port->key.name, ifname_ext);
        sal_fprintf(fp, "Interface %s not found\n", ifname_ext);
        return HSRV_E_NONE;
    }

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        IFNAME_ETH2SHORT(p_db_port->key.name, ifname_ext);
        
        /* LAG should iterate all member ports */
        CTCLIB_SLIST_LOOP(p_db_if->lag->member_ports.obj_list, p_db_mem_if, listnode_lag)
        {
            IFNAME_ETH2SHORT(p_db_mem_if->key.name, ifname_ext_mem);
        
            gport = CTC_SAI_OBJECT_INDEX_GET(p_db_mem_if->portid);
            sal_fprintf(fp, "----- Interface %s member %s ifindex %u gport 0x%04X -----\n", ifname_ext, ifname_ext_mem, p_db_port->ifindex, gport);

            if (p_db_inst)
            {
                hsrv_stp_show_msti_port_one(p_db_port, p_db_inst, gport, fp);
            }
            else
            {
                /* show CIST */
                inst_key.instance = GLB_DEFAULT_INSTANCE;
                p_db_inst_iter = tbl_mstp_instance_get_mstp_instance(&inst_key);
                hsrv_stp_show_msti_port_one(p_db_port, p_db_inst_iter, gport, fp);

                /* show MSTI */
                CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
                {
                    inst_key.instance = p_db_msti_port->key.instance;
                    p_db_inst_iter = tbl_mstp_instance_get_mstp_instance(&inst_key);
                    if (NULL == p_db_inst_iter)
                    {
                        continue;
                    }
                    hsrv_stp_show_msti_port_one(p_db_port, p_db_inst_iter, gport, fp);
                }
            }
        }
    }
    else
    {
        IFNAME_ETH2SHORT(p_db_port->key.name, ifname_ext);
    
        gport = CTC_SAI_OBJECT_INDEX_GET(p_db_if->portid);
        sal_fprintf(fp, "----- Interface %s ifindex %u gport 0x%04X -----\n", ifname_ext, p_db_port->ifindex, gport);
        
        if (p_db_inst)
        {
            hsrv_stp_show_msti_port_one(p_db_port, p_db_inst, gport, fp);
        }
        else
        {
            /* show CIST */
            inst_key.instance = GLB_DEFAULT_INSTANCE;
            p_db_inst_iter = tbl_mstp_instance_get_mstp_instance(&inst_key);
            hsrv_stp_show_msti_port_one(p_db_port, p_db_inst_iter, gport, fp);

            /* show MSTI */
            CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
            {
                inst_key.instance = p_db_msti_port->key.instance;
                p_db_inst_iter = tbl_mstp_instance_get_mstp_instance(&inst_key);
                if (NULL == p_db_inst_iter)
                {
                    continue;
                }
                hsrv_stp_show_msti_port_one(p_db_port, p_db_inst_iter, gport, fp);
            }
        }
    }
            
    return HSRV_E_NONE;
}

uint8
hsrv_stp_state_mapping(uint8 fea_state)
{
    uint8 ctc_state = 0;
    
    switch (fea_state)
    {
    case GLB_BR_PORT_STATE_BLOCKING:
        ctc_state = CTC_STP_BLOCKING;
        break;
    case GLB_BR_PORT_STATE_FORWARDING:
        ctc_state = CTC_STP_FORWARDING;
        break;
    case GLB_BR_PORT_STATE_LISTENING:
    case GLB_BR_PORT_STATE_LEARNING:
        ctc_state = CTC_STP_LEARNING;
        break;
    case GLB_BR_PORT_STATE_DISABLED:
        ctc_state = CTC_STP_FORWARDING;
        break;
    default:
        ctc_state = CTC_STP_UNAVAIL;
        break;
    }

    return ctc_state;
}

int32
_hsrv_stp_check_show_banner(FILE *fp)
{
    sal_fprintf(fp, "%-8s %-16s %-10s %-6s %-16s %-16s\n", 
        "Count", "Interface", "Instance", "StpId", "SW State", "HW State");
    sal_fprintf(fp, "--------+----------------+----------+------+----------------+----------------\n");

    return 0;
}

int32
_hsrv_stp_check_show_mismatch(uint32 mismatch_count, char *ifname, uint32 instance, uint8 stpid, uint8 sw_state, uint8 hw_state, FILE *fp)
{
    char sw_state_str[CMD_BUF_32];
    char hw_state_str[CMD_BUF_32];

    sal_snprintf(sw_state_str, CMD_BUF_32, "%u(%s)", sw_state, hsrv_stp_ctc_state_str(sw_state));
    sal_snprintf(hw_state_str, CMD_BUF_32, "%u(%s)", hw_state, hsrv_stp_ctc_state_str(hw_state));
    
    sal_fprintf(fp, "%-8u %-16s %-10u %-6u %-16s %-16s\n", 
        mismatch_count, 
        ifname, 
        instance, 
        stpid, 
        sw_state_str, 
        hw_state_str);

    return 0;
}

int32
hsrv_stp_check_one_interface(tbl_interface_t *p_db_if, tbl_mstp_port_t *p_db_port, tbl_mstp_instance_t *p_db_inst, FILE *fp)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_interface_t *p_db_mem_if = NULL;
    tbl_msti_port_t *p_db_msti_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *listnode_lag = NULL;
    uint32 gport = 0;
    uint8 sw_state = 0;
    uint8 hw_state = 0;
    uint8 stpid = 0;
    int32 rc = 0;
    uint32 mismatch_count = 0;
    char ifname_ext[IFNAME_EXT_SIZE];
    char ifname_ext_mem[IFNAME_EXT_SIZE];
    char agg_mem_ifname[CMD_BUF_64];

    if (GLB_IF_TYPE_LINKAGG_IF == p_db_if->hw_type)
    {
        IFNAME_ETH2SHORT(p_db_port->key.name, ifname_ext);
        
        if (NULL == p_db_if->lag)
        {
            return HSRV_E_NONE;
        }

        /* 1. check CIST state */
        sw_state = hsrv_stp_state_mapping(p_db_port->fea_state);
        CTCLIB_SLIST_LOOP(p_db_if->lag->member_ports.obj_list, p_db_mem_if, listnode_lag)
        {
            IFNAME_ETH2SHORT(p_db_mem_if->key.name, ifname_ext_mem);
            
            hw_state = CTC_STP_UNAVAIL;
            gport = CTC_SAI_OBJECT_INDEX_GET(p_db_mem_if->portid);
            stpid = GLB_DEFAULT_INSTANCE;
            rc = ctc_stp_get_state(gport, GLB_DEFAULT_INSTANCE, &hw_state);
            if (sw_state != hw_state)
            {
                if (!mismatch_count)
                {
                    _hsrv_stp_check_show_banner(fp);
                }
                mismatch_count++;
                sal_sprintf(agg_mem_ifname, "%s/%s", ifname_ext, ifname_ext_mem);
                _hsrv_stp_check_show_mismatch(mismatch_count, agg_mem_ifname, GLB_DEFAULT_INSTANCE, GLB_DEFAULT_INSTANCE, sw_state, hw_state, fp);
            }
        }

        /* 2. check CIST state */
        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
        {
            IFNAME_ETH2SHORT(p_db_mem_if->key.name, ifname_ext_mem);
            inst_key.instance = p_db_msti_port->key.instance;
            p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
            if (NULL == p_db_inst)
            {
                continue;
            }
            sw_state = hsrv_stp_state_mapping(p_db_msti_port->fea_state);

            CTCLIB_SLIST_LOOP(p_db_if->lag->member_ports.obj_list, p_db_mem_if, listnode_lag)
            {
                hw_state = CTC_STP_UNAVAIL;
                gport = CTC_SAI_OBJECT_INDEX_GET(p_db_mem_if->portid);
                stpid = CTC_SAI_OBJECT_INDEX_GET(p_db_inst->instance_oid);
                rc = ctc_stp_get_state(gport, stpid, &hw_state);
                if (sw_state != hw_state)
                {
                    if (!mismatch_count)
                    {
                        _hsrv_stp_check_show_banner(fp);
                    }
                    mismatch_count++;
                    sal_sprintf(agg_mem_ifname, "%s/%s", ifname_ext, ifname_ext_mem);
                    _hsrv_stp_check_show_mismatch(mismatch_count, agg_mem_ifname, inst_key.instance, stpid, sw_state, hw_state, fp);
                }
            }
        }
    }
    else
    {
        IFNAME_ETH2SHORT(p_db_port->key.name, ifname_ext);
    
        /* 1. check CIST state */
        sw_state = hsrv_stp_state_mapping(p_db_port->fea_state);
        hw_state = CTC_STP_UNAVAIL;
        gport = CTC_SAI_OBJECT_INDEX_GET(p_db_if->portid);
        stpid = GLB_DEFAULT_INSTANCE;
        rc = ctc_stp_get_state(gport, GLB_DEFAULT_INSTANCE, &hw_state);
        if (sw_state != hw_state)
        {
            if (!mismatch_count)
            {
                _hsrv_stp_check_show_banner(fp);
            }
            mismatch_count++;
            _hsrv_stp_check_show_mismatch(mismatch_count, ifname_ext, GLB_DEFAULT_INSTANCE, GLB_DEFAULT_INSTANCE, sw_state, hw_state, fp);
        }

        /* 2. check CIST state */
        CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti_port, listnode)
        {
            inst_key.instance = p_db_msti_port->key.instance;
            p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
            if (NULL == p_db_inst)
            {
                continue;
            }
            sw_state = hsrv_stp_state_mapping(p_db_msti_port->fea_state);
            hw_state = CTC_STP_UNAVAIL;
            gport = CTC_SAI_OBJECT_INDEX_GET(p_db_if->portid);
            stpid = CTC_SAI_OBJECT_INDEX_GET(p_db_inst->instance_oid);
            rc = ctc_stp_get_state(gport, stpid, &hw_state);
            if (sw_state != hw_state)
            {
                if (!mismatch_count)
                {
                    _hsrv_stp_check_show_banner(fp);
                }
                mismatch_count++;
                _hsrv_stp_check_show_mismatch(mismatch_count, ifname_ext, inst_key.instance, stpid, sw_state, hw_state, fp);
            }
        }
    }

    return rc;
}

int32 
hsrv_stp_cmd_show_hybrid_stp(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    tbl_interface_t *p_db_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    FILE *fp = NULL;
    vid_t vid = 0;
    uint8 stpid = 0;
    uint8 state = 0;
    uint32 gport = 0;
    int32 rc = HSRV_E_NONE;
    vlan_bmp_t inst0;
    vlan_bmp_t inst1;
    char vlanstr[MAX_CMD_STR_LEN];
    char *statestr[] = {"FWD", "BLOCK", "LEARN", "INVALID"};

    sal_memset(&inst0, 0, sizeof(inst0));
    sal_memset(&inst1, 0, sizeof(inst1));

    for (vid = GLB_VLAN_DEFAULT_VID; vid <= GLB_VLAN_MAX; vid++)
    {
        stpid = 0;
        ctc_stp_get_vlan_stpid(vid, &stpid);
        if (HSRV_DEF_INSTANCE == stpid)
        {
            GLB_BMP_SET(inst0, vid);
        }
        else if (HSRV_OF_INSTANCE == stpid)
        {
            GLB_BMP_SET(inst1, vid);
        }
    }

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    cdb_bitmap_val2str(vlanstr, MAX_CMD_STR_LEN, inst0, GLB_VLAN_BMP_WORD_MAX);
    sal_fprintf(fp, "STP-INST[0] VLANs: %s\n", vlanstr);
    sal_fprintf(fp, "%-20s %-8s %-8s %-8s\n", "IFNAME", "GPORT", "INST", "STATE");
    sal_fprintf(fp, "--------------------+--------+--------+--------\n");
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
        {
            continue;
        }
        gport = CTC_SAI_OBJECT_INDEX_GET(p_db_if->portid);
        ctc_stp_get_state(gport, HSRV_DEF_INSTANCE, &state);
        sal_fprintf(fp, "%-20s %-8u %-8u %-8s\n", p_db_if->key.name, gport, HSRV_DEF_INSTANCE, statestr[state]);
    }
   
    cdb_bitmap_val2str(vlanstr, MAX_CMD_STR_LEN, inst1, GLB_VLAN_BMP_WORD_MAX);
    sal_fprintf(fp, "FWD-INST[1] VLANs: %s\n", vlanstr);
    sal_fprintf(fp, "%-20s %-8s %-8s %-8s\n", "IFNAME", "GPORT", "INST", "STATE");
    sal_fprintf(fp, "--------------------+--------+--------+--------\n");
    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
        {
            continue;
        }
        gport = CTC_SAI_OBJECT_INDEX_GET(p_db_if->portid);
        ctc_stp_get_state(gport, HSRV_OF_INSTANCE, &state);
        sal_fprintf(fp, "%-20s %-8u %-8u %-8s\n", p_db_if->key.name, gport, HSRV_OF_INSTANCE, statestr[state]);
    }
    
    sal_fclose(fp);
    
    return rc;
}

int32 
hsrv_stp_cmd_show_inst_state(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_port_key_t port_key;
    tbl_mstp_port_master_t *p_if_master = tbl_mstp_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_mstp_port_t *p_db_port = NULL;
    tbl_mstp_instance_t *p_db_inst = NULL;
    uint32 value = 0;
    FILE *fp = NULL;
    tbl_interface_t *p_db_if = NULL;
    uint32 do_check = FALSE;
    int32 rc = HSRV_E_NONE;
    
    sal_memset(&port_key, 0, sizeof(port_key));

    if (2 == argc)
    {
        if (sal_strlen(argv[1]))
        {
            sal_strcpy(port_key.name, argv[1]);
            p_db_port = tbl_mstp_port_get_mstp_port(&port_key);
            if (NULL == p_db_port)
            {
                CFG_CONFLICT_RET("Interface %s not found", argv[1]);
            }
        }

        if (sal_strlen(argv[0]))
        {
            PM_CFG_GET_INTEGER(value, argv[0], para->p_rs);
            inst_key.instance = value;
            p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
            if (NULL == p_db_inst)
            {
                CFG_CONFLICT_RET("Instance %s not found", argv[0]);
            }
        }
    }
    else if (1 == argc)
    {
        if (0 == sal_strcmp("check", argv[0]))
        {
            do_check = TRUE;
        }
        else
        {
            CFG_CONFLICT_RET("Invalid parameter %s", argv[0]);
        }
    }    
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (do_check)
    {
        CTCLIB_SLIST_LOOP(p_if_master->port_list, p_db_port, listnode)
        {
            p_db_if = tbl_interface_get_interface_by_name(p_db_port->key.name);
            if (NULL == p_db_if)
            {
                continue;
            }
            if (!tbl_interface_is_running(p_db_if))
            {
                continue;
            }
            hsrv_stp_check_one_interface(p_db_if, p_db_port, p_db_inst, fp);
        }
    }
    else
    {
        if (p_db_port)
        {
            hsrv_stp_show_one_interface(p_db_port, p_db_inst, fp);
        }
        else
        {
            CTCLIB_SLIST_LOOP(p_if_master->port_list, p_db_port, listnode)
            {
                p_db_if = tbl_interface_get_interface_by_name(p_db_port->key.name);
                if (NULL == p_db_if)
                {
                    continue;
                }
                if (!tbl_interface_is_running(p_db_if))
                {
                    continue;
                }
                hsrv_stp_show_one_interface(p_db_port, p_db_inst, fp);
            }
        }
    }

    sal_fclose(fp);

    return HSRV_E_NONE;
}

int32
hsrv_stp_set_of_instance_state_fwd()
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    tbl_interface_t *p_db_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 fea_state = GLB_BR_PORT_STATE_FORWARDING;
    uint32 instance = HSRV_OF_INSTANCE;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        hsrv_stp_set_port_state(p_db_if->ifindex, instance, fea_state);
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_stp_exclude_all_vlan(uint32 exclude)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    vid_t vid = 0;

    if (exclude)
    {
/* modified by liwh for bug 54430, 2019-11-19 
   for TM, when vlan is not created, ipe will use vlan ptr 0, set vlan ptr 0 stpid to 64 */
#if defined (TSINGMA) || defined (DUET2)
        ctc_stp_set_vlan_stpid(0, HSRV_OF_INSTANCE);
#endif
/* liwh end */
        
        for (vid = GLB_VLAN_DEFAULT_VID; vid <= GLB_VLAN_MAX; vid++)
        {
            if (GLB_BMP_ISSET(p_db_glb->of_include_vlan_bmp, vid))
            {
                continue;
            }
            ctc_stp_set_vlan_stpid(vid, HSRV_OF_INSTANCE);
        }
    }
    else
    {
/* modified by liwh for bug 54430, 2019-11-19 */
#if defined (TSINGMA) || defined (DUET2)
        ctc_stp_set_vlan_stpid(0, HSRV_DEF_INSTANCE);
#endif
/* liwh end */
   
        for (vid = GLB_VLAN_DEFAULT_VID; vid <= GLB_VLAN_MAX; vid++)
        {
            if (GLB_BMP_ISSET(p_db_glb->of_include_vlan_bmp, vid))
            {
                continue;
            }
            ctc_stp_set_vlan_stpid(vid, HSRV_DEF_INSTANCE);
        }
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_stp_set_include_vlan(tbl_mstp_global_t *p_glb)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    vid_t vid = 0;

    /* Added by kcao for bug 50557, not change to OF_INSANCE if exclude all vlan is FALSE */
    if (p_db_glb->of_exclude_all_vlan)
    {
        GLB_BMP_ITER_BEGIN(p_db_glb->of_include_vlan_bmp, vid)
        {
            if (!GLB_BMP_ISSET(p_glb->of_include_vlan_bmp, vid))
            {
                ctc_stp_set_vlan_stpid(vid, HSRV_OF_INSTANCE);
            }
        }
        GLB_BMP_ITER_END(p_db_glb->of_include_vlan_bmp, vid);
    }
    
    GLB_BMP_ITER_BEGIN(p_glb->of_include_vlan_bmp, vid)
    {
        if (!GLB_BMP_ISSET(p_db_glb->of_include_vlan_bmp, vid))
        {
            ctc_stp_set_vlan_stpid(vid, HSRV_DEF_INSTANCE);
        }
    }
    GLB_BMP_ITER_END(p_glb->of_include_vlan_bmp, vid);
    
    return HSRV_E_NONE;
}

int32
hsrv_stp_set_enable_hybrid(uint32 enable)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_interface_t *p_db_if = NULL;
    uint32 gport = 0;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
    {
        if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
        {
            continue;
        }
        gport = CTC_SAI_OBJECT_INDEX_GET(p_db_if->portid);
        if (enable)
        {
            ctc_l2pdu_set_port_action(gport, CTC_L2PDU_ACTION_INDEX_BPDU, CTC_PDU_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU);
        }
        else
        {
            ctc_l2pdu_set_port_action(gport, CTC_L2PDU_ACTION_INDEX_BPDU, CTC_PDU_L2PDU_ACTION_TYPE_FWD);
        }
    }
    
    return HSRV_E_NONE;
}

int32
hsrv_mstp_global_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds)
{
    tbl_mstp_global_t *p_glb = NULL;

    /*1. debug out infor*/
    HSRV_IF_DEBUG("Enter into %s.", __FUNCTION__);
    HSRV_PTR_CHECK(p_tbl);

    /*3. process tbl*/
    switch (oper)
    {
    case CDB_OPER_SET:
        p_glb = (tbl_mstp_global_t*)p_tbl;
        HSRV_PTR_CHECK(p_glb);

        switch (field_id)
        {
        case TBL_MSTP_GLOBAL_FLD_OF_EXCLUDE_ALL_VLAN:
            HSRV_IF_ERROR_RETURN(hsrv_stp_exclude_all_vlan(p_glb->of_exclude_all_vlan));
            break;
            
        case TBL_MSTP_GLOBAL_FLD_OF_INCLUDE_VLAN_BMP:
            HSRV_IF_ERROR_RETURN(hsrv_stp_set_include_vlan(p_glb));
            break;

        case TBL_MSTP_GLOBAL_FLD_ENABLE:
#ifdef OFPRODUCT
            HSRV_IF_ERROR_RETURN(hsrv_stp_set_enable_hybrid(p_glb->enable));
            hsrv_stp_set_of_instance_state_fwd();
#endif /* OFPRODUCT */
            break;

        default:
            break;
        }

    default:
        return HSRV_E_INVALID_PARAM;
    }

    return HSRV_E_NONE;
}

int32
hsrv_stp_start()
{
    cdb_register_cfg_act_cb(ACT_SHOW_INST_STATE, hsrv_stp_cmd_show_inst_state);
    cdb_pm_subscribe_tbl(TBL_MSTP_GLOBAL, hsrv_mstp_global_sync, NULL);

#ifdef OFPRODUCT
    cdb_register_cfg_act_cb(ACT_SHOW_HYBRID_STP, hsrv_stp_cmd_show_hybrid_stp);
#endif
    
    return HSRV_E_NONE;
}

