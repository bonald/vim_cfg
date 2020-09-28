/****************************************************************************
* $Id$
*  Centec switch information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Centecnetworks
* Date           : 2019-07-10
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hsrv_inc.h"
#include "hsrv_mirror.h"
#include "gen/tbl_cfm_global_define.h"
#include "gen/tbl_cfm_global.h"
#include <linux/ip.h>
#include <sys/statfs.h> 
#include "sal_common.h"
#include "saicfm.h"
#include "../../../../sai/src/oam/ctc_sai_cfm.h"
#include "gen/tbl_cfm_md_define.h"
#include "gen/tbl_cfm_md.h"
#include "gen/tbl_cfm_ma_define.h"
#include "gen/tbl_cfm_ma.h"
#include "gen/tbl_cfm_maid_define.h"
#include "gen/tbl_cfm_maid.h"
#include "gen/tbl_cfm_lmep_define.h"
#include "gen/tbl_cfm_lmep.h"
#include "gen/tbl_cfm_rmep_define.h"
#include "gen/tbl_cfm_rmep.h"
#include "hsrv_cfm.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/
hsrv_cfm_t    *g_pst_hsrv_cfm = NULL;

extern int32 sai_cfm_isr(SAI_CFM_ISR_CB fn);
/******************************************************************************
* Defines and Macros
******************************************************************************/
#define CTC_CFM_OAM_CCM_SIM_INTERVAL   1000

#ifdef _GLB_UML_SYSTEM_
extern int32
cm_oam_update_ccm_timer(uint32 chip_id, uint32 min_ptr, uint32 max_ptr, uint32 update_times);
extern int32 cm_oam_scan_defect_timer(uint32 chip_id, uint32 scan_times);
#endif

int32
hsrv_cfm_set_lmep_cc_enable(tbl_cfm_lmep_t   *p_cfm_lmep, sai_cfm_api_t  *cfm_api, uint8  cfm_mode);

int32
hsrv_cfm_set_enable(tbl_cfm_global_t   *p_cfm_global)
{
    int32                              rc = PM_E_NONE;
    tbl_cfm_lmep_master_t   *p_db_glb = NULL;
    tbl_cfm_lmep_t               *p_lmep = NULL;
    ctclib_slistnode_t             *listnode = NULL;
    sai_cfm_api_t                  *cfm_api = NULL;
    uint8                               lmep_cci_enable = 0;

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_CFM,(void**)&cfm_api));

     /* check exist */
    p_db_glb = tbl_cfm_lmep_get_master();
    if (!p_db_glb)
    {
        return PM_E_FAIL;
    }


    /* set local mep cci enabled */
    CTCLIB_SLIST_LOOP(p_db_glb->cfm_lmep_list, p_lmep, listnode)
    {
        /* reserve lmep cci enable */
        lmep_cci_enable = p_lmep->cci_enabled;

        /* set cci_enable to sdk */
        p_lmep->cci_enabled = p_lmep->cci_enabled && p_cfm_global->cfm_enable;
        hsrv_cfm_set_lmep_cc_enable(p_lmep, cfm_api, p_cfm_global->cfm_mode);

        /* recover lmep_cci_enable */
        p_lmep->cci_enabled = lmep_cci_enable;
    }

    return rc;
}

int32
hsrv_cmd_process_cfm_global(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_cfm_global_t         *p_cfm_global = NULL;
    int32                          rc = PM_E_NONE;
    sai_cfm_api_t             *cfm_api;
    sai_attribute_t            attr[1];
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    tbl_interface_t             *p_db_if = NULL;
    ctclib_slistnode_t         *listnode = NULL;
    sai_object_id_t            port_oid;

    CFG_DBG_FUNC();

    sal_memset(attr, 0, sizeof(attr));

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_CFM,(void**)&cfm_api));

    switch (oper)
    {
    case CDB_OPER_SET:
        p_cfm_global = (tbl_cfm_global_t*)p_tbl;
        
        switch (field_id)
        {
        case TBL_CFM_GLOBAL_FLD_CFM_ENABLE:
            sal_memset(attr, 0, sizeof(attr));
            attr[0].id = SAI_CFM_ENABEL;
            attr[0].value.u32 = p_cfm_global->cfm_enable;

            CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
            {
                if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
                {
                    continue;
                }
                
                hsrv_interface_get_objectid(p_db_if, &port_oid);
                HSRV_IF_ERROR_RETURN(cfm_api->cfm_enable(port_oid, attr));

#ifdef GREATBELT
                hsrv_interface_set_port_mac(p_db_if);
#endif
            }

            hsrv_cfm_set_enable(p_cfm_global);
            break;

        default:
            break;
        }
        break;      
    case CDB_OPER_GET:
        break;
    case CDB_OPER_DEL:
        break;

    default:
        break;
    }
    
    return rc;
}

int32
hsrv_cmd_process_cfm_maid(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    tbl_cfm_maid_t         *p_cfm_maid = NULL;
    int32                          rc = PM_E_NONE;
    sai_cfm_api_t             *cfm_api;
    sai_attribute_t            attr[2];
    tbl_cfm_global_t        *p_master = tbl_cfm_global_get_cfm_global();

    CFG_DBG_FUNC();

    sal_memset(attr, 0, sizeof(attr));

    if (!p_master)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    if (!p_tbl)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_CFM,(void**)&cfm_api));

    if (!cfm_api)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_cfm_maid = (tbl_cfm_maid_t*)p_tbl;
        attr[0].id = SAI_CFM_MODE;
        attr[0].value.u32 = p_master->cfm_mode;
        attr[1].id = SAI_CFM_MAID;
        sal_memcpy(attr[1].value.chardata, p_cfm_maid->key.name, CFM_MAID_MAX_LENGTH);
        HSRV_IF_ERROR_RETURN(cfm_api->cfm_add_maid(attr));
        break;
        
    case CDB_OPER_DEL:
        p_cfm_maid = (tbl_cfm_maid_t*)p_tbl;
        attr[0].id = SAI_CFM_MODE;
        attr[0].value.u32 = p_master->cfm_mode;
        attr[1].id = SAI_CFM_MAID;
        sal_memcpy(attr[1].value.chardata, p_cfm_maid->key.name, CFM_MAID_MAX_LENGTH);
        HSRV_IF_ERROR_RETURN(cfm_api->cfm_del_maid(attr));
        break;

    default:
        break;
    }
    
    return rc;
}

int32
hsrv_cmd_process_cfm_ma(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    int32                          rc = PM_E_NONE;

    return rc;
}

static void
_hagt_cfm_update_timer_handler (void *p_data)
{
#ifdef _GLB_UML_SYSTEM_
    uint8 lchip_num = 0;
    uint8 lchip;

   ctc_get_local_chip_num(&lchip_num);
    for (lchip = 0; lchip < lchip_num; lchip ++)
    { 
#ifdef GREATBELT
       cm_oam_update_ccm_timer(lchip, 2, 12, 1);
       cm_oam_scan_defect_timer(lchip, 1);
#endif

#ifdef GOLDENGATE
        cm_oam_update_ccm_timer(lchip, 2050, 2060, 1);
#endif
        //cm_oam_scan_defect_timer(lchip, 1);
    }
#endif /* _GLB_UML_SYSTEM_ */

    g_pst_hsrv_cfm->pst_cfm_timer = CTC_TASK_ADD_TIME_MSEC(_hagt_cfm_update_timer_handler, NULL, CTC_CFM_OAM_CCM_SIM_INTERVAL);
    
    if (!g_pst_hsrv_cfm->pst_cfm_timer)
    {
        HSRV_LOG_ERR("Update CFM OAM engine failed.\n");
        return;
    }

    return;
}

int32
hsrv_cfm_add_lmep(tbl_cfm_lmep_t   *p_cfm_lmep, sai_cfm_api_t  *cfm_api, uint8  cfm_mode)
{
    int32                          rc = PM_E_NONE;
    sai_attribute_t            attr[1];
    uint32                        index = 0;
    tbl_interface_t            *p_db_if = NULL;
    sai_object_id_t            port_oid;
    tbl_cfm_lmep_t           lmep;
    tbl_cfm_lmep_t           *p_lmep = NULL;

    sal_memset(attr, 0, sizeof(attr));

    /* find local mep */
    sal_memcpy(&lmep, &p_cfm_lmep->key, sizeof(tbl_cfm_lmep_key_t));
    p_lmep = tbl_cfm_lmep_get_cfm_lmep(&lmep.key);
    if (!p_lmep)
    {   
        HSRV_LOG_WARN("Local mep is not exist"); 
    }

    /* reserve mep */
    /* alloc index */
    index = ctclib_opf_alloc_offset(g_pst_hsrv_cfm->pst_mep_opf, 0, 1);
    if( CTCLIB_OPF_INVALID_OFFSET == index)
    {
        HSRV_LOG_WARN("Not enough MEP offset resource.");
        return HSRV_E_RESOURCE_FULL;
    }

    attr[0].value.cfm.mode = cfm_mode;

    p_db_if = tbl_interface_get_interface_by_ifindex(p_cfm_lmep->key.ifindex);
    if (!p_db_if)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    hsrv_interface_get_objectid(p_db_if, &port_oid);
    attr[0].value.cfm.glPort = port_oid;

    attr[0].value.cfm.vlanid = p_cfm_lmep->key.vlan_id;
    attr[0].value.cfm.direction = p_cfm_lmep->direction;
    attr[0].value.cfm.level = p_cfm_lmep->key.level;

    attr[0].value.cfm.ccmGportid = port_oid;
    sal_memcpy(attr[0].value.cfm.maid, p_cfm_lmep->maid, CFM_MAID_MAX_LENGTH);

    if (GLB_IF_MODE_L2 == p_db_if->mode)
    {
        attr[0].value.cfm.mpAddrMode = HSRV_CFM_MP_ADDR_INDIVIDUAL;
    }
    else
    {
        attr[0].value.cfm.mpAddrMode = HSRV_CFM_MP_ADDR_SHARED;    
    }

    if (GLB_CFM_MODE_Y1731 == cfm_mode)
    {
        attr[0].value.cfm.enableDM = TRUE;
    }
    
    attr[0].value.cfm.active = TRUE;
    attr[0].value.cfm.ccmInterval = p_cfm_lmep->cci_interval;

    attr[0].value.cfm.mepOnCpu = FALSE;
    attr[0].value.cfm.tpidType = GLB_TPID_TYPE_1;

    attr[0].value.cfm.mepid = p_cfm_lmep->mep_id;
    attr[0].value.cfm.mepIndex = index;
    attr[0].value.cfm.ifindex = p_cfm_lmep->key.ifindex;

    HSRV_IF_ERROR_RETURN(cfm_api->cfm_add_lmep(attr));

    /* reserve mep_index */
    p_lmep->mep_index = index;
    tbl_cfm_lmep_set_cfm_lmep_field(p_lmep, TBL_CFM_LMEP_FLD_MEP_INDEX);

    if (!g_pst_hsrv_cfm->pst_cfm_timer)
    {
        g_pst_hsrv_cfm->pst_cfm_timer = CTC_TASK_ADD_TIME_MSEC(_hagt_cfm_update_timer_handler, NULL, CTC_CFM_OAM_CCM_SIM_INTERVAL);
    }
    
    return rc;
}

int32
hsrv_cfm_del_lmep(tbl_cfm_lmep_t   *p_cfm_lmep, sai_cfm_api_t  *cfm_api, uint8  cfm_mode)
{
    int32                          rc = PM_E_NONE;
    sai_attribute_t            attr[1];
    tbl_interface_t            *p_db_if = NULL;
    sai_object_id_t            port_oid;
    uint32                        index = 0;

    sal_memset(attr, 0, sizeof(attr));

    attr[0].value.cfm.mode = cfm_mode;
    p_db_if = tbl_interface_get_interface_by_ifindex(p_cfm_lmep->key.ifindex);
    if (!p_db_if)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    hsrv_interface_get_objectid(p_db_if, &port_oid);
    attr[0].value.cfm.glPort = port_oid;

    attr[0].value.cfm.vlanid = p_cfm_lmep->key.vlan_id;
    attr[0].value.cfm.direction = p_cfm_lmep->direction;
    attr[0].value.cfm.level = p_cfm_lmep->key.level;

    HSRV_IF_ERROR_RETURN(cfm_api->cfm_del_lmep(attr));

    /* free mep index */
    index = attr[0].value.cfm.mepIndex;
    ctclib_opf_free_offset(g_pst_hsrv_cfm->pst_mep_opf, 0, 1, index);

    /* delete timer */
    if (g_pst_hsrv_cfm->pst_cfm_timer)
    {
        CTC_TASK_DEL_TIME(g_pst_hsrv_cfm->pst_cfm_timer);
    }

    return rc;
}

int32
hsrv_cfm_set_lmep_cc_enable(tbl_cfm_lmep_t   *p_cfm_lmep, sai_cfm_api_t  *cfm_api, uint8  cfm_mode)
{
    int32                          rc = PM_E_NONE;
    tbl_interface_t            *p_db_if = NULL;
    sai_object_id_t            port_oid;
    sai_attribute_t            attr[1];
    tbl_cfm_lmep_t           lmep;
    tbl_cfm_lmep_t           *p_db_lmep = NULL;

    sal_memset(attr, 0, sizeof(attr));

    attr[0].value.cfm.mode = cfm_mode;

    p_db_if = tbl_interface_get_interface_by_ifindex(p_cfm_lmep->key.ifindex);
    if (!p_db_if)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    /* find local mep */
    sal_memcpy(&lmep, &p_cfm_lmep->key, sizeof(tbl_cfm_lmep_key_t));
    p_db_lmep = tbl_cfm_lmep_get_cfm_lmep(&lmep.key);
    if (!p_db_lmep)
    {   
        HSRV_LOG_WARN("Local mep is not exist"); 
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    
    hsrv_interface_get_objectid(p_db_if, &port_oid);
    attr[0].value.cfm.glPort = port_oid;

    attr[0].value.cfm.vlanid = p_cfm_lmep->key.vlan_id;
    attr[0].value.cfm.direction = p_cfm_lmep->direction;
    attr[0].value.cfm.level = p_cfm_lmep->key.level;
    attr[0].value.cfm.cci_enable = p_cfm_lmep->cci_enabled;
    attr[0].value.cfm.mepIndex = p_db_lmep->mep_index;

    HSRV_IF_ERROR_RETURN(cfm_api->cfm_set_cc_enable(attr));
    
    return rc;
}

int32
hsrv_cfm_set_lmep_ccm_vlan_cos(tbl_cfm_lmep_t   *p_cfm_lmep, sai_cfm_api_t  *cfm_api, uint8  cfm_mode)
{
    int32                          rc = PM_E_NONE;
    tbl_interface_t            *p_db_if = NULL;
    sai_object_id_t            port_oid;
    sai_attribute_t            attr[1];

    sal_memset(attr, 0, sizeof(attr));

    attr[0].value.cfm.mode = cfm_mode;

    p_db_if = tbl_interface_get_interface_by_ifindex(p_cfm_lmep->key.ifindex);
    if (!p_db_if)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    hsrv_interface_get_objectid(p_db_if, &port_oid);
    attr[0].value.cfm.glPort = port_oid;

    attr[0].value.cfm.vlanid = p_cfm_lmep->key.vlan_id;
    attr[0].value.cfm.direction = p_cfm_lmep->direction;
    attr[0].value.cfm.level = p_cfm_lmep->key.level;
    attr[0].value.cfm.ccm_vlan_cos = p_cfm_lmep->ccm_vlan_cos;

    HSRV_IF_ERROR_RETURN(cfm_api->cfm_set_ccm_vlan_cos(attr));
    
    return rc;
}

int32
hsrv_cfm_set_lmep_rdi(tbl_cfm_lmep_t   *p_cfm_lmep, sai_cfm_api_t  *cfm_api, uint8  cfm_mode)
{
    int32                          rc = PM_E_NONE;
    tbl_interface_t            *p_db_if = NULL;
    sai_object_id_t            port_oid;
    sai_attribute_t            attr[1];
    tbl_cfm_lmep_t*          p_cfm_db_lmep = NULL;
    tbl_cfm_lmep_key_t     lmep_key;

    sal_memset(&lmep_key, 0, sizeof(tbl_cfm_lmep_key_t));

    /* find local mep */
    sal_memcpy(&lmep_key, &p_cfm_lmep->key, sizeof(tbl_cfm_lmep_key_t));
    p_cfm_db_lmep = tbl_cfm_lmep_get_cfm_lmep(&lmep_key);
    if (!p_cfm_db_lmep)
    {
        return HSRV_E_ENTRY_NOT_EXIST;    
    }

    sal_memset(attr, 0, sizeof(attr));

    attr[0].value.cfm.mode = cfm_mode;

    p_db_if = tbl_interface_get_interface_by_ifindex(p_cfm_lmep->key.ifindex);
    if (!p_db_if)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    hsrv_interface_get_objectid(p_db_if, &port_oid);
    attr[0].value.cfm.glPort = port_oid;

    attr[0].value.cfm.vlanid = p_cfm_lmep->key.vlan_id;
    attr[0].value.cfm.direction = p_cfm_lmep->direction;
    attr[0].value.cfm.level = p_cfm_lmep->key.level;
    attr[0].value.cfm.rdi = p_cfm_lmep->rdi;
    attr[0].value.cfm.mepIndex = p_cfm_db_lmep->mep_index;

    /* modified by liwh for bug 53686, 2019-10-18 */
    //HSRV_IF_ERROR_RETURN(cfm_api->cfm_set_rdi(attr));
    cfm_api->cfm_set_rdi(attr);
    /* liwh end */
    
    return rc;
}

int32
hsrv_cfm_add_rmep(tbl_cfm_rmep_t   *p_cfm_rmep, sai_cfm_api_t  *cfm_api, uint8  cfm_mode)
{
    int32                          rc = PM_E_NONE;
    sai_attribute_t            attr[1];
    uint32                        index = 0;
    tbl_interface_t            *p_db_if = NULL;
    sai_object_id_t            port_oid;

    sal_memset(attr, 0, sizeof(attr));

    /* reserve remote mep */
    /* alloc index */
    index = ctclib_opf_alloc_offset(g_pst_hsrv_cfm->pst_mep_opf, 0, 1);
    if( CTCLIB_OPF_INVALID_OFFSET == index)
    {
        HSRV_LOG_WARN("Not enough MEP offset resource.");
        return HSRV_E_RESOURCE_FULL;
    }

    attr[0].value.cfm.mode = cfm_mode;

    p_db_if = tbl_interface_get_interface_by_ifindex(p_cfm_rmep->ifindex);
    if (!p_db_if)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    hsrv_interface_get_objectid(p_db_if, &port_oid);
    attr[0].value.cfm.glPort = port_oid;

    attr[0].value.cfm.vlanid = p_cfm_rmep->key.vlan_id;
    attr[0].value.cfm.level = p_cfm_rmep->key.level;
    
    attr[0].value.cfm.active = TRUE;
    attr[0].value.cfm.mepid = p_cfm_rmep->key.rmep_id;
    attr[0].value.cfm.mepIndex = index;
    attr[0].value.cfm.ifindex = p_cfm_rmep->ifindex;
    sal_memcpy(attr[0].value.cfm.rmep_mac, p_cfm_rmep->mac_addr, GLB_ETH_ADDR_LEN);

    HSRV_IF_ERROR_RETURN(cfm_api->cfm_add_rmep(attr));
    
    return rc;
}

int32
hsrv_cfm_del_rmep(tbl_cfm_rmep_t   *p_cfm_rmep, sai_cfm_api_t  *cfm_api, uint8  cfm_mode)
{
    int32                          rc = PM_E_NONE;
    sai_attribute_t            attr[1];
    tbl_interface_t            *p_db_if = NULL;
    sai_object_id_t            port_oid;
    uint32                        index = 0;
    tbl_cfm_rmep_t          *p_cfm_db_rmep = NULL;

    sal_memset(attr, 0, sizeof(attr));

    p_cfm_db_rmep = tbl_cfm_rmep_get_cfm_rmep(&p_cfm_rmep->key);
    if (!p_cfm_db_rmep)
    {
        return HSRV_E_ENTRY_NOT_EXIST;    
    }

    attr[0].value.cfm.mode = cfm_mode;
    p_db_if = tbl_interface_get_interface_by_ifindex(p_cfm_db_rmep->ifindex);
    if (!p_db_if)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }
    
    hsrv_interface_get_objectid(p_db_if, &port_oid);
    attr[0].value.cfm.glPort = port_oid;
    attr[0].value.cfm.vlanid = p_cfm_db_rmep->key.vlan_id;
    attr[0].value.cfm.level = p_cfm_db_rmep->key.level;
    attr[0].value.cfm.mepid = p_cfm_db_rmep->key.rmep_id;

    HSRV_IF_ERROR_RETURN(cfm_api->cfm_del_rmep(attr));

    /* free mep index */
    index = attr[0].value.cfm.mepIndex;
    ctclib_opf_free_offset(g_pst_hsrv_cfm->pst_mep_opf, 0, 1, index);

    return rc;
}

int32
hsrv_cmd_process_cfm_lmep(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    int32                          rc = PM_E_NONE;
    tbl_cfm_lmep_t           *p_cfm_lmep = NULL;
    sai_cfm_api_t             *cfm_api = NULL;
    tbl_cfm_global_t         *p_glb_master = tbl_cfm_global_get_cfm_global();

    CFG_DBG_FUNC();

    if (!p_glb_master)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    if (!p_tbl)
    {
        return HSRV_E_ENTRY_NOT_EXIST;   
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_CFM,(void**)&cfm_api));

    if (!cfm_api)
    {
        return HSRV_E_ENTRY_NOT_EXIST;   
    }

    switch (oper)
    {
    case CDB_OPER_ADD:
        //p_cfm_lmep = (tbl_cfm_lmep_t*)p_tbl;
        //hsrv_cfm_add_lmep(p_cfm_lmep, cfm_api, p_glb_master->cfm_mode);
        break;

    case CDB_OPER_SET:
        p_cfm_lmep = (tbl_cfm_lmep_t*)p_tbl;
        switch (field_id)
        {
        case TBL_CFM_LMEP_FLD_CCI_ENABLED:
            if (p_glb_master->cfm_enable)
            {
                hsrv_cfm_set_lmep_cc_enable(p_cfm_lmep, cfm_api, p_glb_master->cfm_mode);
            }
            break;

        case TBL_CFM_LMEP_FLD_CCM_VLAN_COS:
            hsrv_cfm_set_lmep_ccm_vlan_cos(p_cfm_lmep, cfm_api, p_glb_master->cfm_mode);
            break;

        case TBL_CFM_LMEP_FLD_RDI:
            hsrv_cfm_set_lmep_rdi(p_cfm_lmep, cfm_api, p_glb_master->cfm_mode);
            break;
            
        default:
            break;
        }

        break;
    case CDB_OPER_DEL:
        p_cfm_lmep = (tbl_cfm_lmep_t*)p_tbl;
        hsrv_cfm_del_lmep(p_cfm_lmep, cfm_api, p_glb_master->cfm_mode);
        break;

    default:
        break;
    }
    
    return rc;
}

int32
hsrv_cmd_process_cfm_lmep_after(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    int32                          rc = PM_E_NONE;
    tbl_cfm_lmep_t           *p_cfm_lmep = NULL;
    sai_cfm_api_t             *cfm_api = NULL;
    tbl_cfm_global_t         *p_glb_master = tbl_cfm_global_get_cfm_global();

    CFG_DBG_FUNC();

    if (!p_glb_master)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    if (!p_tbl)
    {
        return HSRV_E_ENTRY_NOT_EXIST;   
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_CFM,(void**)&cfm_api));

    if (!cfm_api)
    {
        return HSRV_E_ENTRY_NOT_EXIST;   
    }

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_cfm_lmep = (tbl_cfm_lmep_t*)p_tbl;
        hsrv_cfm_add_lmep(p_cfm_lmep, cfm_api, p_glb_master->cfm_mode);
        break;

    case CDB_OPER_SET:
        break;
        
    case CDB_OPER_DEL:
        break;

    default:
        break;
    }
    
    return rc;
}

int32
hsrv_cmd_process_cfm_rmep(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
        void *p_tbl, void *p_ds)
{
    int32                          rc = PM_E_NONE;
    tbl_cfm_rmep_t           *p_cfm_rmep = NULL;
    sai_cfm_api_t             *cfm_api = NULL;
    tbl_cfm_global_t         *p_glb_master = tbl_cfm_global_get_cfm_global();

    CFG_DBG_FUNC();

    if (!p_glb_master)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    if (!p_tbl)
    {
        return HSRV_E_ENTRY_NOT_EXIST;   
    }

    HSRV_IF_ERROR_RETURN(sai_api_query(SAI_API_CFM,(void**)&cfm_api));

    if (!cfm_api)
    {
        return HSRV_E_ENTRY_NOT_EXIST;   
    }

    switch (oper)
    {
    case CDB_OPER_ADD:
        p_cfm_rmep = (tbl_cfm_rmep_t*)p_tbl;
        if (p_cfm_rmep->local_mep_id)
        {
            hsrv_cfm_add_rmep(p_cfm_rmep, cfm_api, p_glb_master->cfm_mode);
        }
        break;

    case CDB_OPER_SET:
        p_cfm_rmep = (tbl_cfm_rmep_t*)p_tbl;
        switch (field_id)
        {
        case TBL_CFM_RMEP_FLD_LOCAL_MEP_ID:
            if (p_cfm_rmep->local_mep_id)
            {
                hsrv_cfm_add_rmep(p_cfm_rmep, cfm_api, p_glb_master->cfm_mode);
            }
            else
            {
                hsrv_cfm_del_rmep(p_cfm_rmep, cfm_api, p_glb_master->cfm_mode);
            }
            break;
            
        default:
            break;
        }
        break;
        
    case CDB_OPER_DEL:
        p_cfm_rmep = (tbl_cfm_rmep_t*)p_tbl;
        hsrv_cfm_del_rmep(p_cfm_rmep, cfm_api, p_glb_master->cfm_mode);
        break;

    default:
        break;
    }
    
    return rc;
}

int32
hsrv_cfm_isr_cb(sai_attribute_t *attr)
{
    int32                           rc = PM_E_NONE;
    tbl_cfm_rmep_key_t     rmep_key;
    tbl_cfm_rmep_t*          p_cfm_db_rmep = NULL;
    tbl_cfm_lmep_key_t     lmep_key;
    tbl_cfm_lmep_t*          p_cfm_db_lmep = NULL;

    sal_memset(&rmep_key, 0, sizeof(tbl_cfm_rmep_key_t));
    sal_memset(&lmep_key, 0, sizeof(tbl_cfm_lmep_key_t));

    if (!attr)
    {
        return HSRV_E_NONE;
    }

    /* set local mep error */
    lmep_key.level = attr->value.cfm.level;
    lmep_key.vlan_id = attr->value.cfm.vlanid;
    lmep_key.ifindex = attr->value.cfm.ifindex;
    p_cfm_db_lmep = tbl_cfm_lmep_get_cfm_lmep(&lmep_key);
    if (!p_cfm_db_lmep)
    {
        return HSRV_E_ENTRY_NOT_EXIST;    
    }

    if (p_cfm_db_lmep->d_unexp_mep != attr->value.cfm.d_unexp_mep)
    {
        p_cfm_db_lmep->d_unexp_mep = attr->value.cfm.d_unexp_mep;
        tbl_cfm_lmep_set_cfm_lmep_field(p_cfm_db_lmep, TBL_CFM_LMEP_FLD_D_UNEXP_MEP);
    }

    if (p_cfm_db_lmep->d_mismerge != attr->value.cfm.d_mismerge)
    {
        p_cfm_db_lmep->d_mismerge = attr->value.cfm.d_mismerge;
        tbl_cfm_lmep_set_cfm_lmep_field(p_cfm_db_lmep, TBL_CFM_LMEP_FLD_D_MISMERGE);
    }

    if (p_cfm_db_lmep->d_meg_lvl != attr->value.cfm.d_meg_lvl)
    {
        p_cfm_db_lmep->d_meg_lvl = attr->value.cfm.d_meg_lvl;
        tbl_cfm_lmep_set_cfm_lmep_field(p_cfm_db_lmep, TBL_CFM_LMEP_FLD_D_MEG_LVL);
    }

    /* rdi won't be sent to opm because opm will clear rdi;
         in opm, rdi will be set when report cfm error */
    /*if (p_cfm_db_lmep->rdi != attr->value.cfm.rdi)
    {
        p_cfm_db_lmep->rdi = attr->value.cfm.rdi;
        tbl_cfm_lmep_set_cfm_lmep_field(p_cfm_db_lmep, TBL_CFM_LMEP_FLD_RDI);
    }*/
    
    /* set remote mep errror */
    if (attr->value.cfm.rmepIndex)
    {
        rmep_key.level = attr->value.cfm.level;
        rmep_key.vlan_id = attr->value.cfm.vlanid;
        rmep_key.rmep_id = attr->value.cfm.rmepid;
        p_cfm_db_rmep = tbl_cfm_rmep_get_cfm_rmep(&rmep_key);
        if (!p_cfm_db_rmep)
        {
            return HSRV_E_ENTRY_NOT_EXIST;    
        }

        if (p_cfm_db_rmep->d_loc != attr->value.cfm.d_loc)
        {
            p_cfm_db_rmep->d_loc = attr->value.cfm.d_loc;
            tbl_cfm_rmep_set_cfm_rmep_field(p_cfm_db_rmep, TBL_CFM_RMEP_FLD_D_LOC);
        }

        if (p_cfm_db_rmep->first_pkt_rx != attr->value.cfm.first_pkt_rx)
        {
            p_cfm_db_rmep->first_pkt_rx = attr->value.cfm.first_pkt_rx;
            tbl_cfm_rmep_set_cfm_rmep_field(p_cfm_db_rmep, TBL_CFM_RMEP_FLD_FIRST_PKT_RX);
        }

        if (p_cfm_db_rmep->d_unexp_period != attr->value.cfm.d_unexp_period)
        {
            p_cfm_db_rmep->d_unexp_period = attr->value.cfm.d_unexp_period;
            tbl_cfm_rmep_set_cfm_rmep_field(p_cfm_db_rmep, TBL_CFM_RMEP_FLD_D_UNEXP_PERIOD);
        }

        if (p_cfm_db_rmep->ma_sa_mismatch != attr->value.cfm.ma_sa_mismatch)
        {
            p_cfm_db_rmep->ma_sa_mismatch = attr->value.cfm.ma_sa_mismatch;
            tbl_cfm_rmep_set_cfm_rmep_field(p_cfm_db_rmep, TBL_CFM_RMEP_FLD_MA_SA_MISMATCH);

            sal_memcpy(p_cfm_db_rmep->mac_addr, attr->value.cfm.rmep_mac, GLB_ETH_ADDR_LEN);
            tbl_cfm_rmep_set_cfm_rmep_field(p_cfm_db_rmep, TBL_CFM_RMEP_FLD_MAC_ADDR);            
        }

        if (p_cfm_db_rmep->rmep_last_rdi != attr->value.cfm.rmep_last_rdi)
        {
            p_cfm_db_rmep->rmep_last_rdi = attr->value.cfm.rmep_last_rdi;
            tbl_cfm_rmep_set_cfm_rmep_field(p_cfm_db_rmep, TBL_CFM_RMEP_FLD_RMEP_LAST_RDI);
        }
    }
    
    return  rc;
}

int32
hsrv_cfm_isr_cb_init()
{
    return sai_cfm_isr(hsrv_cfm_isr_cb);
}

int32
hsrv_cfm_start()
{
    uint32   start_offset = 0;
    
    g_pst_hsrv_cfm = HSRV_CALLOC(MEM_FEA_HSRV_MODULE, sizeof(hsrv_cfm_t));
    HSRV_MEM_CHECK(g_pst_hsrv_cfm);

    g_pst_hsrv_cfm->pst_mep_opf = ctclib_opf_init(1);

#ifdef GREATBELT
    start_offset = 2;
#else
    start_offset = GLB_CFM_MEP_INDEX_MIN;
#endif

    ctclib_opf_init_offset(g_pst_hsrv_cfm->pst_mep_opf, 0, start_offset, 
        GLB_CFM_MEP_NUM_MAX);

    cdb_pm_subscribe_tbl(TBL_CFM_GLOBAL, hsrv_cmd_process_cfm_global, NULL);
    cdb_pm_subscribe_tbl(TBL_CFM_MA, hsrv_cmd_process_cfm_ma, NULL);
    cdb_pm_subscribe_tbl(TBL_CFM_MAID, hsrv_cmd_process_cfm_maid, NULL); 
    cdb_pm_subscribe_tbl(TBL_CFM_LMEP, hsrv_cmd_process_cfm_lmep, hsrv_cmd_process_cfm_lmep_after);
    cdb_pm_subscribe_tbl(TBL_CFM_RMEP, hsrv_cmd_process_cfm_rmep, NULL);

    hsrv_cfm_isr_cb_init();
    
    return HSRV_E_NONE;
}

