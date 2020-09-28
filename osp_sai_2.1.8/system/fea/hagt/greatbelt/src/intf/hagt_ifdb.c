/****************************************************************************
ifeq ($(PRODUCT), sai)
all_targets += fea
endif
ifeq ($(PRODUCT), sai)
all_targets += fea
endif
* $Id$
*  Centec Interface private information related MACRO, ENUM, Date Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : ychen
* Date          : 2010-07-19 
* Reason        : First Create.
****************************************************************************/

/******************************************************************************
* Header Files 
******************************************************************************/
#include "hagt_inc.h"
#include "hagt_if.h"
#include "hagt_if_priv.h"

/******************************************************************************
* Defines and Macros
******************************************************************************/


/******************************************************************************
* Global and Declaration
******************************************************************************/

/******************************************************************************
*
* Functions
*
******************************************************************************/
/* modified by cuixl for bug 17874, 2012-02-21 */

/***************************************************************************************************
 * Name         : _hagt_ifdb_agg_vector_add 
 * Purpose      : add agg port info to vector     
 * Input        : trunkid: agg trunkid
                  data: port info
 * Output       : N/A              
 * Return       : TRUE or FALSE
 * Note         : N/A
***************************************************************************************************/
static bool
_hagt_ifdb_agg_vector_add(uint16 trunkid, void* data)
{
    return ctclib_vector_add (g_pst_hagt_if_master->pst_agg_port_info, trunkid -1 , data); 
}

/***************************************************************************************************
 * Name         : _hagt_ifdb_agg_vector_del 
 * Purpose      : del agg  port from vector     
 * Input        : trunkid: agg trunkid
 * Output       : N/A              
 * Return       : hagt_if_port_info_t*
 * Note         : N/A
***************************************************************************************************/
static void *
_hagt_ifdb_agg_vector_del(uint16 trunkid)
{
    /* modified by cuixl for bug 18726, 2012-03-27 */
    /* when deleting agg interface, we should reset agg stats*/
    hagt_if_port_stats_info_t* p_stats_info = NULL;
    glb_if_stats_t* p_stats_db = NULL;
    p_stats_info = &g_pst_hagt_if_master->agg_port_stats_info;
    /* Modifed by weizj for bug 36460 */
    p_stats_db = p_stats_info->p_port_stats[trunkid - 1];

    if (p_stats_db)
    {
        sal_memset(p_stats_db, 0, sizeof(glb_if_stats_t));
    }

    return ctclib_vector_del (g_pst_hagt_if_master->pst_agg_port_info, (uint32 )(trunkid - 1));
}

/***************************************************************************************************
 * Name         : _hagt_ifdb_vector_get 
 * Purpose      : get port info from vector     
 * Input        : gportid: global portid       
 * Output       : N/A              
 * Return       : hagt_if_port_info_t*
 * Note         : N/A
***************************************************************************************************/
static void*
_hagt_ifdb_agg_vector_get(uint16 trunkid)
{
    return ctclib_vector_get (g_pst_hagt_if_master->pst_agg_port_info, trunkid - 1);    
}

/***************************************************************************************************
 * Name         : _hagt_ifdb_vector_add 
 * Purpose      : add port info to vector     
 * Input        : gportid: global portid
                  data: port info
 * Output       : N/A              
 * Return       : TRUE or FALSE
 * Note         : N/A
***************************************************************************************************/
static bool
_hagt_ifdb_vector_add(uint16 gportid, void* data)
{
    uint8 gchip_id;
    uint8 lchip_id;
    /* Modifed by weizj for bug 36460 */
    uint16 lport_id;

    gchip_id = GLB_GET_GLOBAL_CHIPID(gportid);
    lport_id = GLB_GET_LOCAL_PORTID(gportid); 
    lchip_id = hagt_chip_glb2local(gchip_id);

    if (lchip_id >= CTC_MAX_LOCAL_CHIP_NUM)
    {
        return FALSE;
    }
    return ctclib_vector_add (g_pst_hagt_if_master->pst_port_info[lchip_id], lport_id, data); 
}
/***************************************************************************************************
 * Name         : _hagt_ifdb_vector_del 
 * Purpose      : del port info from vector     
 * Input        : gportid: global portid       
 * Output       : N/A              
 * Return       : hagt_if_port_info_t*
 * Note         : N/A
***************************************************************************************************/
static void*
_hagt_ifdb_vector_del(uint16 gportid)
{
    uint8 gchip_id;
    uint8 lchip_id;
    /* Modifed by weizj for bug 36460 */
    uint16 lport_id;

    gchip_id = GLB_GET_GLOBAL_CHIPID(gportid);
    lport_id = GLB_GET_LOCAL_PORTID(gportid); 
    lchip_id = hagt_chip_glb2local(gchip_id);

    if (lchip_id >= CTC_MAX_LOCAL_CHIP_NUM)
    {
        return NULL;
    }

    return ctclib_vector_del (g_pst_hagt_if_master->pst_port_info[lchip_id], lport_id);
}
/***************************************************************************************************
 * Name         : _hagt_ifdb_vector_get 
 * Purpose      : get port info from vector     
 * Input        : gportid: global portid       
 * Output       : N/A              
 * Return       : hagt_if_port_info_t*
 * Note         : N/A
***************************************************************************************************/
static void*
_hagt_ifdb_vector_get(uint16 gportid)
{
    uint8 gchip_id;
    uint8 lchip_id;
    /* Modifed by weizj for bug 36460 */
    uint16 lport_id;

    gchip_id = GLB_GET_GLOBAL_CHIPID(gportid);
    lport_id = GLB_GET_LOCAL_PORTID(gportid); 
    lchip_id = hagt_chip_glb2local(gchip_id);

    if (lchip_id >= CTC_MAX_LOCAL_CHIP_NUM)
    {
        return NULL;
    }

    return ctclib_vector_get (g_pst_hagt_if_master->pst_port_info[lchip_id], lport_id);
}
/***************************************************************************************************
 * Name         : _hagt_ifdb_calloc_port_info 
 * Purpose      : malloc port memory      
 * Input        : N/A         
 * Output       : N/A              
 * Return       : hagt_if_port_info_t pointer
 * Note         : N/A
***************************************************************************************************/
static inline hagt_if_port_info_t*
_hagt_ifdb_calloc_port_info (void)
{ 
    return (hagt_if_port_info_t*)HAGT_CALLOC(MEM_FEA_IFMGR_MODULE, sizeof(hagt_if_port_info_t)); 
}
/***************************************************************************************************
 * Name         : _hagt_ifdb_free_port_info 
 * Purpose      : free port memory      
 * Input        : pst_port_info: interface pointer         
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static inline int32
_hagt_ifdb_free_port_info (hagt_if_port_info_t* pst_port_info)
{    
    HAGT_IF_PTR_VALID_CHECK(pst_port_info);
    
    HAGT_FREE(MEM_FEA_IFMGR_MODULE, pst_port_info); 
    
    return HAGT_E_NONE;
}

/* modified by cuixl for bug 17874, 2012-02-21 */
/***************************************************************************************************
 * Name         : hagt_ifdb_port_vector_get 
 * Purpose      : get port info from vector     
 * Input        : gportid: global portid or agg trunk id       
 * Output       : N/A              
 * Return       : hagt_if_port_info_t*
 * Note         : N/A
***************************************************************************************************/
void*
hagt_ifdb_port_vector_get(uint16 gportid)
{
    uint16 tid;

    if (GLB_IS_LINKAGG_PORT(gportid))
    {
        tid = GLB_TRANS_AGGPORT_TO_TID(gportid);
        return _hagt_ifdb_agg_vector_get(tid);
    }
    else
    {
        return _hagt_ifdb_vector_get(gportid);
    }
}

/***************************************************************************************************
 * Name         : _hagt_ifdb_add_agg_interface 
 * Purpose      : create agg in database      
 * Input        : trunkid: agg trunk id, must be tid        
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
int32
hagt_ifdb_add_agg_interface (uint16 trunkid)
{
    bool ret = FALSE; 
    hagt_if_port_info_t* pst_port_info = NULL;

    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  trunkid=%d.", trunkid);

    /*2. add to vector*/   
    pst_port_info = _hagt_ifdb_calloc_port_info(); 
    if (NULL == pst_port_info)
    {
        HAGT_IF_LOG_ERR("memory allocate failed for hagt interface.");
        return HAGT_E_NO_MEMORY;
    }
    pst_port_info->trunkid = trunkid;
    pst_port_info->notify_intval[GLB_IF_STATS_REQ_IMISH] = GLB_IF_STATS_LOAD_DEF_INTVAL;
    
    HAGT_IF_STATS_LOCK;
    ret = _hagt_ifdb_agg_vector_add (trunkid, (void*)pst_port_info);
    HAGT_IF_STATS_UNLOCK;
    
    if (FALSE == ret)
    {
        _hagt_ifdb_free_port_info (pst_port_info);
        return HAGT_E_INVALID_PARAM;
    }
   
    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_ifdb_del_agg_interface 
 * Purpose      : del agg port in database      
 * Input        : trunkid: agg trunk id, must be tid        
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
int32
hagt_ifdb_del_agg_interface (uint16 trunkid)
{
    hagt_if_port_info_t* pst_port_info = NULL;

    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  trunkid=%d.", trunkid);

    HAGT_IF_STATS_LOCK;
    
    pst_port_info = _hagt_ifdb_agg_vector_get (trunkid); 
    if (NULL == pst_port_info)
    {
        HAGT_IF_STATS_UNLOCK;
        HAGT_IF_LOG_ERR("agg port already destroyed.");
        return HAGT_E_NOT_FOUND;
    }
    
    _hagt_ifdb_agg_vector_del (trunkid);
    
    HAGT_IF_STATS_UNLOCK;
    
    _hagt_ifdb_free_port_info (pst_port_info);

    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : hagt_ifdb_create_interface 
 * Purpose      : create port in database      
 * Input        : gportid: global port id, must be physical interface        
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
int32
hagt_ifdb_create_interface (uint16 gportid, uint16 slotno, uint16 portno, uint16 subportno)
{
    bool ret = FALSE; 
    hagt_if_port_info_t* pst_port_info = NULL;

    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  gportid=%d.", gportid);

    /*1. check parameter*/
    if (GLB_IS_LINKAGG_PORT(gportid) || (FALSE == HAGT_GPORT_IS_LOCAL(gportid)))
    {
        return HAGT_E_NONE;
    }

    /*2. add to vector*/   
    pst_port_info = _hagt_ifdb_calloc_port_info(); 
    if (NULL == pst_port_info)
    {
        HAGT_IF_LOG_ERR("memory allocate failed for hagt interface.");
        return HAGT_E_NO_MEMORY;
    }
    pst_port_info->gportid = gportid;
    /* modified by cuixl for bug 17874, 2012-02-27 */
    pst_port_info->notify_intval[GLB_IF_STATS_REQ_IMISH] = GLB_IF_STATS_LOAD_DEF_INTVAL;
    /*Added by xgu for hardware learning, dump hw fdb database need ifname, 2013-9-16*/
    pst_port_info->slotno = slotno;
    pst_port_info->portno = portno;
    pst_port_info->subportno = subportno;

    ret = _hagt_ifdb_vector_add (gportid, (void*)pst_port_info); 
    if (FALSE == ret)
    {
        _hagt_ifdb_free_port_info (pst_port_info);
        return HAGT_E_INVALID_PARAM;
    }

    return HAGT_E_NONE;
}
/***************************************************************************************************
 * Name         : hagt_ifdb_destroy_interface 
 * Purpose      : destroy port in database      
 * Input        : gportid: global port id, must be physical interface        
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
int32
hagt_ifdb_destroy_interface (uint16 gportid)
{   
    hagt_if_port_info_t* pst_port_info = NULL;

    /*1. check parameter*/
    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  gportid=%d.", gportid);
    
    if (GLB_IS_LINKAGG_PORT(gportid) || (FALSE == HAGT_GPORT_IS_LOCAL(gportid)))
    {
        return HAGT_E_NONE;
    }

    /*2. release vector*/   
    pst_port_info = _hagt_ifdb_vector_get (gportid);
    if (NULL == pst_port_info)
    {
        HAGT_IF_LOG_ERR("port already destroyed.");
        return HAGT_E_NOT_FOUND;
    }
    
    _hagt_ifdb_vector_del (gportid);
    _hagt_ifdb_free_port_info (pst_port_info);      
    
    return HAGT_E_NONE;
} 
/***************************************************************************************************
 * Name         : hagt_ifdb_init 
 * Purpose      : init hagt interface module database     
 * Input        : N/A        
 * Output       : N/A              
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
int32
hagt_ifdb_init (void)
{ 
    uint32 index;
    for (index = 0; index < CTC_MAX_LOCAL_CHIP_NUM; index++)
    {
        /* modified by cuixl for bug 17874 , 2012-02-20. init ifdb with phy port */
        g_pst_hagt_if_master->pst_port_info[index] =  ctclib_vector_init(1, GLB_PORT_NUM_PER_CHIP);
        if (NULL == g_pst_hagt_if_master->pst_port_info[index])
        {
            HAGT_IF_LOG_ERR("init hagt interface vector %d failed.", index);
            return HAGT_E_NOT_INIT;
        }
    }        
    /* modified by cuixl for bug 17874 , 2012-02-20. init ifdb with agg port */    
    g_pst_hagt_if_master->pst_agg_port_info =  ctclib_vector_init(1, GLB_LINKAGG_MAX_GROUP);
    if (NULL == g_pst_hagt_if_master->pst_agg_port_info)
    {
        HAGT_IF_LOG_ERR("init hagt agg interface vector failed.");
        return HAGT_E_NOT_INIT;
    }
    
    return HAGT_E_NONE;
}
/***************************************************************************************************
 * Name         : _hagt_ifdb_get_vlan_tag_ctrl 
 * Purpose      : get vlan tag ctrl based on port flag    
 * Input        : gportid: global portid        
 * Output       : pst_tag_ctrl:   ctc_vlantag_ctl_t            
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_ifdb_get_vlan_tag_ctrl (uint16 gportid, ctc_vlantag_ctl_t* pst_tag_ctrl)
{  
    hagt_if_port_info_t* pst_port = NULL;
    ctc_vlantag_ctl_t tag_ctrl = CTC_VLANCTL_DROP_ALL;

    /*check pointer in high level API*/
    /*1. dump information*/
    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  gportid=%d.", gportid);

    /*2. get port flags*/
    pst_port = _hagt_ifdb_vector_get (gportid);
    if (NULL == pst_port)
    {
        HAGT_IF_LOG_ERR("get port pointer failed.");
        return HAGT_E_NOT_FOUND;
    }
    HAGT_IFDB_DEBUG("  port flags=0x%x", pst_port->flag);

     /*allow all, routed port, vpls. ROUTE_EN flag must check first*/
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_ROUTED_EN)
        || GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FALG_L2VPN_PORT_MODE))
    {
        tag_ctrl = CTC_VLANCTL_ALLOW_ALL_PACKETS;
        goto vlan_tag_out;
    }

#if 0 /*delete by huangxt. for dot1x is controlled by receive_en*/
    /*3. drop all, dot1x related*/
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_DOT1X_UNAUTHED)
        && GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_DOT1X_DIR_IN))
    {
        tag_ctrl = CTC_VLANCTL_DROP_ALL;
        goto vlan_tag_out;
    }
#endif

#if 0 /*delete by weij for efm, use HAGT_IF_PORT_FLAG_EFM_PAR_ACTION_DISCARD instead*/
    /*drop all, efm related*/
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_EFM_RX_DISCARD))
    {
       tag_ctrl = CTC_VLANCTL_DROP_ALL;
       goto vlan_tag_out;
    }
#endif

    /*drop all, mirror related*/
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_MIRROR_DEST))
    {
        tag_ctrl = CTC_VLANCTL_DROP_ALL;
        goto vlan_tag_out;
    }

    /* Added by Yan Xing'an, for openflow, 2015-01-07 */
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_OPENFLOW_IGNORE_VLAN_FILTER_CTL))
    {
        tag_ctrl = CTC_VLANCTL_ALLOW_ALL_PACKETS;
        goto vlan_tag_out;
    }

    /* add vlan mapping codes by liwh for bug 13480, 2010-11-15 
       allow all, qinq related */
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_VLAN_MAPPING_QINQ_ENABLE))
    {
        tag_ctrl = CTC_VLANCTL_ALLOW_ALL_PACKETS;
        goto vlan_tag_out;
    }
    /*drop all, agg related*/
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_DIS_COLLECTOR)
        || (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_AGGREGATED)
        && !GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_AGG_OPRED)))     
    {
        tag_ctrl = CTC_VLANCTL_DROP_ALL;
        goto vlan_tag_out;
    }    

    /*allow all, tag ctrl*/
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_ALLOW_TAGGED)
        && GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_ALLOW_UNTAGGED))
    {
        tag_ctrl = CTC_VLANCTL_ALLOW_ALL_PACKETS;
        goto vlan_tag_out;
    }

    /*allow all tagged packet*/
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_ALLOW_TAGGED))
    {
        tag_ctrl = CTC_VLANCTL_DROP_ALL_UNTAGGED;
        goto vlan_tag_out;
    }
    /*allow all untagged packet*/
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_ALLOW_UNTAGGED))
    {
        tag_ctrl = CTC_VLANCTL_DROP_ALL_TAGGED;
        goto vlan_tag_out;
    }   

vlan_tag_out:
    *pst_tag_ctrl = tag_ctrl;
    
    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_ifdb_get_receive_en 
 * Purpose      : get port receive enable based on port flag    
 * Input        : gportid: global portid        
 * Output       : p_rx_en:   TRUE=rx enable. FALSE=rx disable            
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_ifdb_get_receive_en (uint16 gportid, bool* p_rx_en)
{
    hagt_if_port_info_t* pst_port = NULL;  
    bool rx_en = TRUE;

    /*check pointer in high level API*/
     /*1. dump information*/
    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  gportid=%d.", gportid);

    /*2. get port flags*/  
    pst_port = _hagt_ifdb_vector_get (gportid);
    if (NULL == pst_port)
    {
        HAGT_IF_LOG_ERR("get port pointer failed.");
        return HAGT_E_NOT_FOUND;
    }
    HAGT_IFDB_DEBUG("  port flags=0x%x", pst_port->flag);

    /*3. running flags*/
    if(!GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_RUNNING))
    {
        rx_en = FALSE;
        goto rx_out;
    }
    
    /*4. dot1x related*/
    if ((GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_DOT1X_UNAUTHED)
        && GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_DOT1X_DIR_IN)))
    {
        rx_en = FALSE;
        goto rx_out;
    }

    /*5. efm related*/
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_EFM_PAR_ACTION_DISCARD))
    {
       rx_en = FALSE; 
       goto rx_out;
    }
    
rx_out:
    *p_rx_en = rx_en;
    
    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_ifdb_get_transmit_en 
 * Purpose      : get port transmit enable based on port flag    
 * Input        : gportid: global portid        
 * Output       : p_tx_en:   TRUE=tx enable. FALSE=tx disable            
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_ifdb_get_transmit_en (uint16 gportid, bool* p_tx_en)
{
    hagt_if_port_info_t* pst_port = NULL;  
    bool tx_en = TRUE;    

    /*check pointer in high level API*/
     /*1. dump information*/
    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  gportid=%d.", gportid);

    /*2. get port flags*/ 
    pst_port = _hagt_ifdb_vector_get (gportid);
    if (NULL == pst_port)
    {
        HAGT_IF_LOG_ERR("get port pointer failed.");
        return HAGT_E_NOT_FOUND;
    }
    HAGT_IFDB_DEBUG("  port flags=0x%x", pst_port->flag);

    /*3. dot1x related*/
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_DOT1X_UNAUTHED)
        && GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_DOT1X_DIR_OUT))
    {
        tx_en = FALSE;
        goto tx_out;
    }

    /*mirror dest related*/
    if (!GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_RUNNING)
        || GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_MIRROR_DEST))
    {
        tx_en = FALSE;
        goto tx_out;
    }

    /*efm related*/
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_EFM_MUX_ACTION_DISCARD))
    {
       tx_en = FALSE; 
       goto tx_out;
    }

    /*agg related*/   
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_DIS_DISTRIBUTOR))
    {
        tx_en = FALSE;
        goto tx_out;
    } 
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_AGGREGATED)
        && (!GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_AGG_OPRED)))
    {
        tx_en = FALSE;
    }
tx_out:
    *p_tx_en = tx_en;
    
    return HAGT_E_NONE;
}
/***************************************************************************************************
 * Name         : _hagt_ifdb_get_dot1q_type 
 * Purpose      : get dot1q type based on port flag    
 * Input        : gportid: global portid        
 * Output       : pst_dot1q_type:   ctc_dot1q_type_t            
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_ifdb_get_dot1q_type (uint16 gportid, ctc_dot1q_type_t* pst_dot1q_type)
{
    hagt_if_port_info_t* pst_port = NULL;    

    /*check pointer in high level API*/
     /*1. dump information*/
    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  gportid=%d.", gportid);
    *pst_dot1q_type = CTC_DOT1Q_TYPE_BOTH;

    /*2. get port flags*/   
    pst_port = _hagt_ifdb_vector_get (gportid);
    if (NULL == pst_port)
    {
        HAGT_IF_LOG_ERR("get port pointer failed.");
        return HAGT_E_NOT_FOUND;
    }
    HAGT_IFDB_DEBUG("  port flags=0x%x", pst_port->flag);

    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_IS_STACKING_PORT))
    {
        *pst_dot1q_type = CTC_DOT1Q_TYPE_BOTH;        
    }
    else if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_ROUTED_EN))
    {
        *pst_dot1q_type = CTC_DOT1Q_TYPE_NONE;        
    }
    else if (!GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_ALLOW_TAGGED))
    {
        *pst_dot1q_type = CTC_DOT1Q_TYPE_CVLAN;
    }
    /* add vlan mapping codes by liwh for bug 13480, 2010-11-15 
       strip outer tag when packet is sent from qinq port */
    else if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_VLAN_MAPPING_QINQ_ENABLE))
    {
        *pst_dot1q_type = CTC_DOT1Q_TYPE_CVLAN;
    }
    else
    {
        *pst_dot1q_type = CTC_DOT1Q_TYPE_BOTH;
    }

    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_ifdb_get_keep_vlan_tag 
 * Purpose      : get keep vlan tag based on port flag    
 * Input        : gportid: global portid        
 * Output       : pst_dot1q_type:   ctc_dot1q_type_t            
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_ifdb_get_keep_vlan_tag (uint16 gportid, bool* keep_vlan_tag)
{
    hagt_if_port_info_t* pst_port = NULL;  
    bool kvt = TRUE;   

     /*1. dump information*/
    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  gportid=%d.", gportid);


    /*2. get port flags*/   
    pst_port = _hagt_ifdb_vector_get (gportid);
    if (NULL == pst_port)
    {
        HAGT_IF_LOG_ERR("get port pointer failed.");
        return HAGT_E_NOT_FOUND;
    }
    HAGT_IFDB_DEBUG("  port flags=0x%x", pst_port->flag);

    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FALG_L2VPN_VLAN_MODE))
    {
        kvt  = FALSE;  
        goto OUT;
    }

OUT:
    *keep_vlan_tag = kvt;

    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_ifdb_get_ingress_vlan_filter 
 * Purpose      : get ingress vlan filter based on port flag    
 * Input        : gportid: global portid        
 * Output       : pst_dot1q_type:   ctc_dot1q_type_t            
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_ifdb_get_ingress_vlan_filter (uint16 gportid, bool* ingress_vlan_filter)
{
    hagt_if_port_info_t* pst_port = NULL;  
    bool ivf = TRUE; 

     /*1. dump information*/
    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  gportid=%d.", gportid);


    /*2. get port flags*/   
    pst_port = _hagt_ifdb_vector_get (gportid);
    if (NULL == pst_port)
    {
        HAGT_IF_LOG_ERR("get port pointer failed.");
        return HAGT_E_NOT_FOUND;
    }
    HAGT_IFDB_DEBUG("  port flags=0x%x", pst_port->flag);
    
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FALG_L2VPN_ENABLE))
    {
        ivf  = FALSE;    
        goto OUT;
    }

    if (!GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_INGRESS_FILTER_EN))
    {
        ivf  = FALSE;    
        goto OUT;
    }

    /* Added by Yan Xing'an, for openflow, 2015-01-07 */
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_OPENFLOW_IGNORE_VLAN_FILTER_CTL))
    {
        ivf  = FALSE;    
        goto OUT;
    }

OUT:
    *ingress_vlan_filter = ivf;

    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_ifdb_get_ingress_vlan_filter 
 * Purpose      : get egress vlan filter based on port flag    
 * Input        : gportid: global portid        
 * Output       : pst_dot1q_type:   ctc_dot1q_type_t            
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_ifdb_get_egress_vlan_filter (uint16 gportid, bool* egress_vlan_filter)
{
    hagt_if_port_info_t* pst_port = NULL;  
    bool evf = TRUE;    

     /*1. dump information*/
    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  gportid=%d.", gportid);


    /*2. get port flags*/   
    pst_port = _hagt_ifdb_vector_get (gportid);
    if (NULL == pst_port)
    {
        HAGT_IF_LOG_ERR("get port pointer failed.");
        return HAGT_E_NOT_FOUND;
    }
    HAGT_IFDB_DEBUG("  port flags=0x%x", pst_port->flag);
    
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FALG_L2VPN_ENABLE))
    {
        evf  = FALSE;    
        goto OUT;
    }

    if (!GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_EGRESS_FILTER_EN))
    {
        evf  = FALSE;    
        goto OUT;
    }
    
    /* Added by Yan Xing'an, for openflow, 2015-01-07 */
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_OPENFLOW_IGNORE_VLAN_FILTER_CTL))
    {
        evf  = FALSE;    
        goto OUT;
    }

OUT:
    *egress_vlan_filter = evf;

    return HAGT_E_NONE;
}

/***************************************************************************************************
 * Name         : _hagt_ifdb_get_src_discard 
 * Purpose      : get src-discard based on port flag    
 * Input        : gportid: global portid        
 * Output       : pst_dot1q_type:   ctc_dot1q_type_t            
 * Return       : HAGT_E_NONE
 * Note         : N/A
***************************************************************************************************/
static int32
_hagt_ifdb_get_src_discard (uint16 gportid, bool* src_discard)
{
    hagt_if_port_info_t* pst_port = NULL;  
    bool src_dis = FALSE;    

     /*1. dump information*/
    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  gportid=%d.", gportid);


    /*2. get port flags*/   
    pst_port = _hagt_ifdb_vector_get (gportid);
    if (NULL == pst_port)
    {
        HAGT_IF_LOG_ERR("get port pointer failed.");
        return HAGT_E_NOT_FOUND;
    }
    HAGT_IFDB_DEBUG("  port flags=0x%x", pst_port->flag);
    
    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_PHY_LPBK_EXTERNAL))
    {
        src_dis  = TRUE;
        goto OUT;
    }

OUT:
    *src_discard = src_dis;

    return HAGT_E_NONE;
}
/* modified by cuixl for bug 17874, 2012-02-21 */

/******************************************************************************
 * Name         : hagt_ifdb_set_port_agg_attribute
 * Purpose      : set port as agg member
 * Input        : 
                gportid: global port, must be in the local linecard and must not be aggregated port
                trunkid:    agg trunk id
                enalbe:  1=enalbe, flag set; 0=disable, flag unset
 * Output       : N/A
 * Return       : HAGT_E_NOT_FOUND: port is not found or not in this linecard; HAGT_E_NONE: ok
 * Note         : N/A
******************************************************************************/
int32
hagt_ifdb_set_port_agg_attribute (uint16 trunkid, uint16 gportid, bool enable)
{
    hagt_if_port_info_t* pst_port_info = NULL;
    
    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  trunkid=%d, gportid=0x%x, enable=%d.", trunkid, gportid, enable);

    if (FALSE == HAGT_GPORT_IS_LOCAL(gportid))
    {
        return HAGT_E_NONE;
    }

    /*2. get port pointer info*/
    HAGT_IF_STATS_LOCK;
    pst_port_info = _hagt_ifdb_vector_get (gportid);
    if (NULL == pst_port_info || pst_port_info->gportid != gportid)
    {
        HAGT_IF_STATS_UNLOCK;
        HAGT_IF_LOG_ERR("get port pointer failed.");
        return HAGT_E_NOT_FOUND;
    }

    /* add port to agg*/
    if (enable)
    {
        /* read phy port stats before it add to agg */
        //hagt_if_get_phy_port_mac_stats(gportid);
        
        pst_port_info->trunkid = trunkid;
    }
    else
    {
        /* read phy port stats to agg before it remove from agg */
        //hagt_if_get_phy_port_mac_stats(gportid);    
        pst_port_info->trunkid = 0;
    }

    HAGT_IF_STATS_UNLOCK;
    
    return HAGT_E_NONE;   
}


/******************************************************************************
 * Name         : hagt_ifdb_set_port_flag
 * Purpose      : set vlan tag ctrl, transmit enable or dot1q type
 * Input        : 
                gportid: global port, must be in the local linecard and must not be aggregated port
                flag:    port flag, which is used to calculate the vlan tag ctrl, dot1q type and transmit enable
                enalbe:  1=enalbe, flag set; 0=disable, flag unset
 * Output       : N/A
 * Return       : HAGT_E_NOT_FOUND: port is not found or not in this linecard; HAGT_E_NONE: ok
 * Note         : N/A
******************************************************************************/
int32
hagt_ifdb_set_port_flag (uint16 gportid, hagt_if_flag_t flag, bool enable)
{
    hagt_if_port_info_t* pst_port_info = NULL;
    hagt_if_tbl_filed_info_t curr_status;   

    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  gportid=%d, flag=0x%x, enable=%d.", gportid, flag, enable);
    
    if (GLB_IS_LINKAGG_PORT(gportid) || (FALSE == HAGT_GPORT_IS_LOCAL(gportid)))
    {
        return HAGT_E_NONE;
    }

    /*2. get port pointer info*/ 
    pst_port_info = _hagt_ifdb_vector_get (gportid);
    if (NULL == pst_port_info || pst_port_info->gportid != gportid)
    {
        HAGT_IF_LOG_ERR("get port pointer failed, gportid(%d)", gportid);
        return HAGT_E_NOT_FOUND;
    }
    HAGT_IFDB_DEBUG("  port flags=0x%x, old vlan tag ctrl=%d, transmit_en=%d, dot1q type=%d", 
        pst_port_info->flag, pst_port_info->status.tag_ctrl, pst_port_info->status.transmit_en, 
        pst_port_info->status.dot1q_type);

    /*3. set port flags*/
    if (enable)
    {
        GLB_SET_FLAG (pst_port_info->flag, flag);
    }
    else
    {
        GLB_UNSET_FLAG (pst_port_info->flag, flag);
    }

    /*4. get vlan tag ctrl, transmit enable, and dot1q type*/
    HAGT_IF_ERROR_RETURN(_hagt_ifdb_get_vlan_tag_ctrl (gportid, &curr_status.tag_ctrl));
    HAGT_IF_ERROR_RETURN(_hagt_ifdb_get_transmit_en (gportid, &curr_status.transmit_en));
    HAGT_IF_ERROR_RETURN(_hagt_ifdb_get_receive_en (gportid, &curr_status.receive_en));
    HAGT_IF_ERROR_RETURN(_hagt_ifdb_get_dot1q_type (gportid, &curr_status.dot1q_type));
    HAGT_IF_ERROR_RETURN(_hagt_ifdb_get_keep_vlan_tag (gportid, &curr_status.keep_vlan_tag));
    HAGT_IF_ERROR_RETURN(_hagt_ifdb_get_ingress_vlan_filter(gportid, &curr_status.ingress_vlan_filter));
    HAGT_IF_ERROR_RETURN(_hagt_ifdb_get_egress_vlan_filter(gportid, &curr_status.egress_vlan_filter));
    HAGT_IF_ERROR_RETURN(_hagt_ifdb_get_src_discard(gportid, &curr_status.src_discard));
    
    HAGT_IFDB_DEBUG("  caculated current vlan tag ctrl=%d, transmit_en=%d, receive_en=%d, dot1q type=%d", 
        curr_status.tag_ctrl, curr_status.transmit_en, curr_status.receive_en, curr_status.dot1q_type);

    if ((curr_status.tag_ctrl != pst_port_info->status.tag_ctrl) || (!pst_port_info->status_configued))
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_vlan_ctl (gportid, curr_status.tag_ctrl));
        pst_port_info->status.tag_ctrl = curr_status.tag_ctrl;
    }

/* modified by liwh for bug 24501, 2013-11-19 */
#ifdef _GLB_UML_SYSTEM_
    if (curr_status.transmit_en != pst_port_info->status.transmit_en)
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_transmit_en (gportid, curr_status.transmit_en));
        pst_port_info->status.transmit_en = curr_status.transmit_en;
    }
    
    if (curr_status.receive_en != pst_port_info->status.receive_en)
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_receive_en(gportid, curr_status.receive_en));
        pst_port_info->status.receive_en = curr_status.receive_en;
    }
#endif

    if (HAGT_IF_PORT_FLAG_MIRROR_DEST == flag)
    {
        if (enable)
        {
            HAGT_IF_ERROR_RETURN(ctc_port_set_transmit_en (gportid, FALSE));
            HAGT_IF_ERROR_RETURN(ctc_port_set_receive_en(gportid, FALSE));
        }
        else
        {
            HAGT_IF_ERROR_RETURN(ctc_port_set_transmit_en (gportid, TRUE));
            HAGT_IF_ERROR_RETURN(ctc_port_set_receive_en(gportid, TRUE));
        }
    }

    if ((curr_status.dot1q_type != pst_port_info->status.dot1q_type) || (!pst_port_info->status_configued))
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_dot1q_type (gportid, curr_status.dot1q_type));
        pst_port_info->status.dot1q_type = curr_status.dot1q_type;
    }    
    
    if ((curr_status.keep_vlan_tag != pst_port_info->status.keep_vlan_tag) || (!pst_port_info->status_configued))
    {
        // TODO: GB chip not support keep vlan tag. Commend by xgu, 2012-12-7. HAGT_IF_ERROR_RETURN(ctc_port_set_keep_vlan_tag (gportid, curr_status.keep_vlan_tag));
        pst_port_info->status.keep_vlan_tag = curr_status.keep_vlan_tag;
    }    

    if ((curr_status.ingress_vlan_filter != pst_port_info->status.ingress_vlan_filter) || (!pst_port_info->status_configued))
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_vlan_filter_en(gportid, CTC_INGRESS, curr_status.ingress_vlan_filter));
        pst_port_info->status.ingress_vlan_filter = curr_status.ingress_vlan_filter;
    }   

    
    if ((curr_status.egress_vlan_filter != pst_port_info->status.egress_vlan_filter) || (!pst_port_info->status_configued))
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_vlan_filter_en(gportid, CTC_EGRESS, curr_status.egress_vlan_filter));
        pst_port_info->status.egress_vlan_filter= curr_status.egress_vlan_filter;
    }        

    if ((curr_status.src_discard != pst_port_info->status.src_discard) || (!pst_port_info->status_configued))
    {
        HAGT_IF_ERROR_RETURN(ctc_port_set_srcdiscard_en(gportid, curr_status.src_discard));
        pst_port_info->status.src_discard = curr_status.src_discard;
    }

    pst_port_info->status_configued = TRUE;
    return HAGT_E_NONE;   
}
/******************************************************************************
 * Name         : hagt_ifdb_get_port_flag
 * Purpose      : get port flag
 * Input        : 
                gportid: global port, must be in the local linecard and must not be aggregated port
                p_flag:    port flag, which is used to calculate the vlan tag ctrl, dot1q type and transmit enable            
 * Output       : N/A
 * Return       : HAGT_E_xxxx
 * Note         : N/A
******************************************************************************/
int32
hagt_ifdb_get_port_flag(uint16 gportid, hagt_if_flag_t* p_flag)
{
    hagt_if_port_info_t* pst_port_info = NULL;

    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  gportid=%d.", gportid);
    
    if (GLB_IS_LINKAGG_PORT(gportid) || (FALSE == HAGT_GPORT_IS_LOCAL(gportid)))
    {
        return HAGT_E_NONE;
    }

    /*2. get port pointer info*/  
    pst_port_info = _hagt_ifdb_vector_get (gportid);
    if (NULL == pst_port_info || pst_port_info->gportid != gportid)
    {
        HAGT_IF_LOG_ERR("get port pointer failed.");
        return HAGT_E_NOT_FOUND;
    }

    *p_flag = pst_port_info->flag;

    return HAGT_E_NONE;    
}

/******************************************************************************
 * Name         : hagt_ifdb_get_port_ifname
 * Purpose      : get interface name by gportid
 * Input        : 
                gportid: global port, must be in the local linecard and must not be aggregated port
 * Output       : ifname
 * Return       : ifname(NULL if not fount the interface)
 * Note         : N/A
******************************************************************************/
int32
hagt_ifdb_get_port_ifname(uint16 gportid, char *ifname)
{
    hagt_if_port_info_t* pst_port_info = NULL;

#define HAGT_BRIDGE_SET_IF_NAME(ifname,slot_no,port_no) \
    snprintf(ifname, 16, "eth-%d-%d", slot_no, port_no)    

    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  gportid=%d.", gportid);
    
    if ((0 == GLB_IS_LINKAGG_PORT(gportid)) && (FALSE == HAGT_GPORT_IS_LOCAL(gportid)))
    {
        return HAGT_E_INVALID_PARAM;
    }

    if(GLB_IS_LINKAGG_PORT(gportid))
    {
        snprintf(ifname, 16, "agg%d", GLB_TRANS_AGGPORT_TO_TID(gportid));
        return HAGT_E_NONE;
    }
    /*2. get port pointer info*/  
    pst_port_info = _hagt_ifdb_vector_get (gportid);
    if (NULL == pst_port_info || pst_port_info->gportid != gportid)
    {
        HAGT_IF_LOG_ERR("get port pointer failed.");
        return HAGT_E_INVALID_PARAM;
    }
    if_build_ethname_by_slot_port(ifname, pst_port_info->slotno, pst_port_info->portno, pst_port_info->subportno);

    return HAGT_E_NONE;
}

/*added by yanxa, for bug 15018, 2011-06-02*/
/******************************************************************************
 * Name         : _hagt_ifdb_change_port_flag
 * Purpose      : change port flag when port is changed to routed or switch
 * Input        : 
                pst_port_info: port information structure
 * Output       : N/A
 * Return       : HAGT_E_NONE
 * Note         : This function only used for hagt_ifdb_change_port_mode
                  When port is changed to routed or switch, some flags may
                  be cleared or reserved.
******************************************************************************/
static int32
_hagt_ifdb_change_port_flag (hagt_if_port_info_t* pst_port_info)
{
    hagt_if_flag_t old_flag;

    old_flag = pst_port_info->flag;
    pst_port_info->flag = 0;

    /*recover phy loopback external*/
    if (GLB_FLAG_ISSET(old_flag, HAGT_IF_PORT_FLAG_PHY_LPBK_EXTERNAL))
    {
        GLB_SET_FLAG(pst_port_info->flag, HAGT_IF_PORT_FLAG_PHY_LPBK_EXTERNAL);
    }

    /* Added by yanxa, for bug 16102, 2011-08-31 */
    /*recover is-10G flag*/
    if (GLB_FLAG_ISSET(old_flag, HAGT_IF_PORT_FLAG_IS_10G_PORT))
    {
        GLB_SET_FLAG(pst_port_info->flag, HAGT_IF_PORT_FLAG_IS_10G_PORT);
    }

    return HAGT_E_NONE;
}

/******************************************************************************
 * Name         : hagt_ifdb_change_port_mode
 * Purpose      : change port to routed or switch, should change port flag
 * Input        : 
                gportid: global port, must be in the local linecard and must not be aggregated port               
                route_en:  1=enalbe 0=disable
 * Output       : N/A
 * Return       : HAGT_E_NONE
 * Note         : this function only used for interface module
******************************************************************************/
int32
hagt_ifdb_change_port_mode (uint16 gportid, bool route_en)
{
    hagt_if_port_info_t* pst_port_info = NULL;  

    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  gportid=%d, route_en=%d.", gportid, route_en);

    if (GLB_IS_LINKAGG_PORT(gportid) || (FALSE == HAGT_GPORT_IS_LOCAL(gportid)))
    {
        return HAGT_E_NONE;
    }

     /*1. get port pointer info*/ 
    pst_port_info = _hagt_ifdb_vector_get (gportid);
    if (NULL == pst_port_info || pst_port_info->gportid != gportid)
    {
        HAGT_IF_LOG_ERR("get port pointer failed.");
        return HAGT_E_NOT_FOUND;
    }

    /*modified by yanxa, for bug 15018, 2011-06-02*/
    /*2. modify flag when port is changed to switched or routed port*/
    HAGT_IF_ERROR_RETURN(_hagt_ifdb_change_port_flag(pst_port_info));
    
    if (route_en)
    {        
        HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag (gportid, HAGT_IF_PORT_FLAG_ROUTED_EN, TRUE));
    }
    else
    {
        /*defalut l2port should be access port, allowed untagged packet*/
        HAGT_IF_ERROR_RETURN(hagt_ifdb_set_port_flag (gportid, HAGT_IF_PORT_FLAG_ALLOW_UNTAGGED, TRUE));
    }

    return HAGT_E_NONE;
}

int32
hagt_ifdb_is_stacking_port (uint16 gportid)
{
    hagt_if_port_info_t* pst_port = NULL;    

    /*check pointer in high level API*/
     /*1. dump information*/
    HAGT_IFDB_DEBUG("Enter into %s.", __FUNCTION__);
    HAGT_IFDB_DEBUG("  gportid=%d.", gportid);

    /*2. get port flags*/   
    pst_port = _hagt_ifdb_vector_get (gportid);
    if (NULL == pst_port)
    {
        HAGT_IF_LOG_ERR("get port pointer failed.");
        return FALSE;
    }

    if (GLB_FLAG_ISSET(pst_port->flag, HAGT_IF_PORT_FLAG_IS_STACKING_PORT))
    {
        HAGT_IF_LOG_ERR("Stacking port, gportid=%d.", gportid);
        return TRUE;        
    }

    return FALSE;
}
