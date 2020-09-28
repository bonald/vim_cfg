#include <sai.h>
#include <sal.h>
#include "ctc_api.h"
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_notifier_call.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_fdb.h>
#include <ctc_sai_cfm.h>
#include <ctc_sai_routerintf.h>
#include <ctc_sai_hostif.h>
#include <ctc_sai_nexthop.h>
#include <ctc_sai_port.h>
#include <ctc_sai_debug.h>
#include "gen/stbl_cfm_lmep_define.h"
#include "gen/stbl_cfm_lmep.h"
#include "gen/stbl_cfm_rmep_define.h"
#include "gen/stbl_cfm_rmep.h"

/******************************************************************************
* Global and Declaration
******************************************************************************/
SAI_CFM_ISR_CB  g_hsrv_cfm_isr_cb = NULL;
#ifdef GREATBELT
static bool    g_hsrv_cfm_set_intr_callback = FALSE;
#endif

int32                                                                                                                                          
ctc_cfm_isr_callback(uint8 gchip, void* p_data);

/*
* Routine Description:
*    Create neighbor entry
*
* Arguments:
*    [in] enable - cfm globally enable
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*
* Note: IP address expected in Network Byte Order.
*/
sai_status_t
ctc_sai_cfm_enable(
    _In_ sai_object_id_t port_id,
    _In_ const sai_attribute_t *attr_list)
{
    ctc_oam_property_t prop;
    sai_status_t        ret     = SAI_STATUS_SUCCESS;

    sal_memset(&prop, 0, sizeof(ctc_oam_property_t));

    prop.oam_pro_type = CTC_OAM_PROPERTY_TYPE_Y1731;
    prop.u.y1731.cfg_type = CTC_OAM_Y1731_CFG_TYPE_PORT_OAM_EN;

    prop.u.y1731.gport = port_id;
    prop.u.y1731.dir = CTC_BOTH_DIRECTION;
    prop.u.y1731.value = attr_list->value.u32;
    ret = ctc_oam_set_property(&prop); 
    if (ret)
    {
        return SAI_STATUS_FAILURE;
    }

#ifdef GREATBELT
#ifndef _GLB_UML_SYSTEM_
    if (attr_list->value.u32)
    {
        ctc_port_set_transmit_en(port_id, TRUE);
        ctc_port_set_receive_en(port_id, TRUE);
    }
#endif

    if (!g_hsrv_cfm_set_intr_callback)
    {
        ctc_interrupt_register_event_cb(CTC_EVENT_OAM_STATUS_UPDATE, ctc_cfm_isr_callback);
        g_hsrv_cfm_set_intr_callback = TRUE;
   }
#endif
        
    return SAI_STATUS_SUCCESS;
}

/**
 * Routine Description:
 *    @brief add cfm maid
 *
 * Arguments:
 *    @param[in] attr_list - cfm maid
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t
ctc_sai_cfm_add_maid(
    _In_ const sai_attribute_t *attr_list)
{
    ctc_oam_maid_t   maid;
    sai_status_t          ret     = SAI_STATUS_SUCCESS;

    sal_memset(&maid, 0, sizeof(ctc_oam_maid_t));

    if (SAI_CFM_MODE == attr_list[0].id)
    {       
        if (GLB_CFM_MODE_DOT1AG == attr_list[0].value.u32)
        {
            maid.mep_type = CTC_OAM_MEP_TYPE_ETH_1AG;
            maid.maid_len = GLB_CFM_MAID_LENGTH_48BYTES;
        }
        else
        {
            maid.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;
            /* modified by liwh for bug 53666 & 53667, 2019-09-21 */
            //maid.maid_len = GLB_CFM_MAID_LENGTH_16BYTES;
            maid.maid_len = GLB_CFM_MAID_LENGTH_48BYTES;
            /* liwh end */
        }
    }

    sal_memcpy(maid.maid, attr_list[1].value.chardata, GLB_CFM_MAID_LENGTH_48BYTES);
    
    ret = ctc_oam_add_maid(&maid);
    if (ret)
    {
        return SAI_STATUS_FAILURE;
    }
        
    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_cfm_del_maid(
    _In_ const sai_attribute_t *attr_list)
{
    ctc_oam_maid_t   maid;
    sai_status_t          ret     = SAI_STATUS_SUCCESS;

    sal_memset(&maid, 0, sizeof(ctc_oam_maid_t));

    if (SAI_CFM_MODE == attr_list[0].id)
    {
        if (GLB_CFM_MODE_DOT1AG == attr_list[0].value.u32)
        {
            maid.mep_type = CTC_OAM_MEP_TYPE_ETH_1AG;
            maid.maid_len = GLB_CFM_MAID_LENGTH_48BYTES;
        }
        else
        {
            maid.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;
            /* modified by liwh for bug 53666 & 53667, 2019-09-21 */
            //maid.maid_len = GLB_CFM_MAID_LENGTH_16BYTES;
            maid.maid_len = GLB_CFM_MAID_LENGTH_48BYTES;
            /* liwh end */
        }
    }

    sal_memcpy(maid.maid, attr_list[1].value.chardata, GLB_CFM_MAID_LENGTH_48BYTES);
        
    ret = ctc_oam_remove_maid(&maid);
    if (ret)
    {
        return SAI_STATUS_FAILURE;
    }
        
    return SAI_STATUS_SUCCESS;
}

/**
 * Routine Description:
 *    @brief add cfm local mep
 *
 * Arguments:
 *    @param[in] attr_list - cfm local mep parameters
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t
ctc_sai_cfm_add_lmep(
    _In_ const sai_attribute_t *attr_list)
{
    ctc_oam_lmep_t    lmep;
    sai_status_t          ret = SAI_STATUS_SUCCESS;
    stbl_cfm_lmep_t    slmep;

    sal_memset(&lmep, 0, sizeof(ctc_oam_lmep_t));
    sal_memset(&slmep, 0, sizeof(stbl_cfm_lmep_t));

    /* add local mep to sdk */
    sal_memcpy(lmep.maid.maid, attr_list[0].value.cfm.maid, GLB_CFM_MAID_LENGTH_48BYTES);
    if (GLB_CFM_MODE_DOT1AG == attr_list[0].value.cfm.mode)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_ETH_1AG;
        lmep.maid.mep_type = CTC_OAM_MEP_TYPE_ETH_1AG;
        lmep.maid.maid_len = GLB_CFM_MAID_LENGTH_48BYTES;
    }
    else
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;
        lmep.maid.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;  
        /* modified by liwh for bug 53666 & 53667, 2019-09-21 */
        //lmep.maid.maid_len = GLB_CFM_MAID_LENGTH_16BYTES;
        lmep.maid.maid_len = GLB_CFM_MAID_LENGTH_48BYTES;
        /* liwh end */
    }

    lmep.key.u.eth.gport = CTC_SAI_OBJECT_INDEX_GET(attr_list[0].value.cfm.glPort);
    lmep.key.u.eth.vlan_id = attr_list[0].value.cfm.vlanid;
    if (GLB_VLAN_NONE == lmep.key.u.eth.vlan_id)
    {
        CTC_SET_FLAG(lmep.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
    }

#if (defined GOLDENGATE) || (defined GREATBELT)
    lmep.key.u.eth.ccm_vlan_id = lmep.key.u.eth.vlan_id;
    lmep.key.u.eth.ccm_vlan_id_valid = TRUE;
#endif

    if (attr_list[0].value.cfm.direction)
    {
        CTC_SET_FLAG(lmep.key.flag, CTC_OAM_KEY_FLAG_UP_MEP);
    }
    lmep.key.u.eth.md_level = attr_list[0].value.cfm.level;

    if (attr_list[0].value.cfm.active)
    {
        CTC_SET_FLAG(lmep.u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_EN);
    }
    
    if (attr_list[0].value.cfm.enableDM)
    {
        CTC_SET_FLAG(lmep.u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_DM_EN);
    }
    
    if (attr_list[0].value.cfm.mepOnCpu)
    {
        CTC_SET_FLAG(lmep.u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_ON_CPU);
    }

    if (attr_list[0].value.cfm.mpAddrMode)
    {
        CTC_SET_FLAG(lmep.u.y1731_lmep.flag, CTC_OAM_Y1731_LMEP_FLAG_SHARE_MAC);
    }
    
    lmep.u.y1731_lmep.ccm_interval = attr_list[0].value.cfm.ccmInterval;
    lmep.u.y1731_lmep.mep_id = attr_list[0].value.cfm.mepid;
    lmep.lmep_index = attr_list[0].value.cfm.mepIndex;
    
    /* nsm tpid type 1 response to ctc svlan tpid 2 */
    if (GLB_TPID_TYPE_1 == attr_list[0].value.cfm.tpidType)
    {
        lmep.u.y1731_lmep.tpid_index = CTC_PARSER_L2_TPID_SVLAN_TPID_0;
    }
    else
    {
        lmep.u.y1731_lmep.tpid_index = CTC_PARSER_L2_TPID_SVLAN_TPID_1;
    }
    
    ret = ctc_oam_add_lmep(&lmep);
    if (ret)
    {
        return SAI_STATUS_FAILURE;
    }

    /* add local mep to db */
    slmep.key.index = lmep.lmep_index;
    slmep.gport = lmep.key.u.eth.gport;
    slmep.md_level = lmep.key.u.eth.md_level;
    slmep.vlan_id = lmep.key.u.eth.vlan_id;
    slmep.mep_id = lmep.u.y1731_lmep.mep_id;
    slmep.cci_interval = lmep.u.y1731_lmep.ccm_interval;
    slmep.direction = attr_list[0].value.cfm.direction;
    slmep.ifindex = attr_list[0].value.cfm.ifindex;
    slmep.mode = attr_list[0].value.cfm.mode;
    sal_memcpy(slmep.maid, lmep.maid.maid, GLB_CFM_MAID_LENGTH_48BYTES);

    ret = stbl_cfm_lmep_add_cfm_lmep(&slmep);
    if (ret)
    {
        return SAI_STATUS_FAILURE;
    }
        
    return SAI_STATUS_SUCCESS;
}

/**
 * Routine Description:
 *    @brief del cfm local mep
 *
 * Arguments:
 *    @param[in] attr_list - cfm local mep parameters
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t
ctc_sai_cfm_del_lmep(
    _Inout_ sai_attribute_t *attr_list)
{
    ctc_oam_lmep_t                  lmep;
    stbl_cfm_lmep_master_t      *p_sdb_lmep_master = NULL;
    ctclib_slistnode_t                 *listnode = NULL;
    stbl_cfm_lmep_t                  *p_sdb_lmep = NULL;
    bool                                     lmep_exist = FALSE;

    sal_memset(&lmep, 0, sizeof(ctc_oam_lmep_t));

    /* set local mep parameters before delete from sdk */
    sal_memcpy(lmep.maid.maid, attr_list[0].value.cfm.maid, GLB_CFM_MAID_LENGTH_48BYTES);
    if (GLB_CFM_MODE_DOT1AG == attr_list[0].value.cfm.mode)
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_ETH_1AG;
        lmep.maid.mep_type = CTC_OAM_MEP_TYPE_ETH_1AG;
        lmep.maid.maid_len = GLB_CFM_MAID_LENGTH_48BYTES;
    }
    else
    {
        lmep.key.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;
        lmep.maid.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;
        /* modified by liwh for bug 53666 & 53667, 2019-09-21 */
        //lmep.maid.maid_len = GLB_CFM_MAID_LENGTH_16BYTES;
        lmep.maid.maid_len = GLB_CFM_MAID_LENGTH_48BYTES;
        /* liwh end */
    }

    lmep.key.u.eth.gport = attr_list[0].value.cfm.glPort;
    lmep.key.u.eth.vlan_id = attr_list[0].value.cfm.vlanid;
    if (GLB_VLAN_NONE == lmep.key.u.eth.vlan_id)
    {
        CTC_SET_FLAG(lmep.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
    }
    if (attr_list[0].value.cfm.direction)
    {
        CTC_SET_FLAG(lmep.key.flag, CTC_OAM_KEY_FLAG_UP_MEP);
    }
    lmep.key.u.eth.md_level = attr_list[0].value.cfm.level;
    lmep.u.y1731_lmep.mep_id = attr_list[0].value.cfm.mepid;
    
    /* get local mep index from db */
    p_sdb_lmep_master = stbl_cfm_lmep_get_master();
    if (!p_sdb_lmep_master)
    {
        CTC_SAI_DEBUG("%s %d local mep gport %llx vlan_id %d md_level %d not exist\n", __func__, __LINE__,
            lmep.key.u.eth.gport, lmep.key.u.eth.vlan_id, lmep.key.u.eth.md_level);
        return SAI_STATUS_SUCCESS;
    }

    CTCLIB_SLIST_LOOP(p_sdb_lmep_master->cfm_lmep_list, p_sdb_lmep, listnode)
    {
        if ((p_sdb_lmep->gport == lmep.key.u.eth.gport)
            && (p_sdb_lmep->vlan_id == lmep.key.u.eth.vlan_id)
            && (p_sdb_lmep->md_level == lmep.key.u.eth.md_level))
        {
            lmep_exist = TRUE;
            break;    
        }
    }

    if (!lmep_exist)
    {
        CTC_SAI_DEBUG("%s %d local mep gport %llx vlan_id %d md_level %d not exist\n", __func__, __LINE__,
            lmep.key.u.eth.gport, lmep.key.u.eth.vlan_id, lmep.key.u.eth.md_level);
        return SAI_STATUS_SUCCESS;
    }

    /* reserve mep index and mep number */
    attr_list->value.cfm.mepIndex = p_sdb_lmep->key.index;
    attr_list->value.cfm.mep_num = p_sdb_lmep_master->cfm_lmep_hash->count - 1;
    lmep.lmep_index = p_sdb_lmep->key.index;

    /* delete local mep from db */
    stbl_cfm_lmep_del_cfm_lmep(&p_sdb_lmep->key);

    /* delete local mep from sdk */
    ctc_oam_remove_lmep(&lmep);

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_cfm_set_lmep_cc_enable_to_sdk(stbl_cfm_lmep_t    *p_db_lmep)
{
    ctc_oam_update_t lmep_update;
    sai_status_t          ret = SAI_STATUS_SUCCESS;
    
    sal_memset(&lmep_update, 0, sizeof(ctc_oam_update_t));
    
    /* set cci enable to sdk */
    if (GLB_CFM_MODE_DOT1AG == p_db_lmep->mode)
    {
        lmep_update.key.mep_type = CTC_OAM_MEP_TYPE_ETH_1AG;
    }
    else
    {
        lmep_update.key.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;
    }

    lmep_update.key.u.eth.gport = p_db_lmep->gport;
    lmep_update.key.u.eth.vlan_id = p_db_lmep->vlan_id;
    if (GLB_VLAN_NONE == lmep_update.key.u.eth.vlan_id)
    {
        CTC_SET_FLAG(lmep_update.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
    }
    
    if (p_db_lmep->direction)
    {
        CTC_SET_FLAG(lmep_update.key.flag, CTC_OAM_KEY_FLAG_UP_MEP);
    }
    lmep_update.key.u.eth.md_level = p_db_lmep->md_level;
    
    lmep_update.is_local = TRUE;
    lmep_update.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_CCM_EN;

    if (p_db_lmep->cci_enabled)
    {
        lmep_update.update_value = TRUE;
    }

    /* set cc enable/disable */
    ret = ctc_oam_update_lmep(&lmep_update);
    if (ret)
    {
        return SAI_STATUS_FAILURE;
    }
    
    return SAI_STATUS_SUCCESS;
}

#ifdef GREATBELT
sai_status_t
ctc_sai_cfm_set_lmep_cc_enable_to_sdk_for_port(uint32 port_id, bool enable)
{
    stbl_cfm_lmep_master_t      *p_sdb_lmep_master = NULL;
    ctclib_slistnode_t                 *listnode = NULL;
    stbl_cfm_lmep_t                  *p_sdb_lmep = NULL;
    
    /* get local mep index from db */
    p_sdb_lmep_master = stbl_cfm_lmep_get_master();
    if (!p_sdb_lmep_master)
    {
        return SAI_STATUS_SUCCESS;
    }

    CTCLIB_SLIST_LOOP(p_sdb_lmep_master->cfm_lmep_list, p_sdb_lmep, listnode)
    {
        if (p_sdb_lmep->gport == port_id)
        {
            /* up mep, needn't process */
            if (p_sdb_lmep->direction)
            {
                continue;    
            }

            /* cci disable needn't process */
            if (!p_sdb_lmep->cci_enabled)
            {
                continue;    
            }

            p_sdb_lmep->cci_enabled = enable;
            ctc_sai_cfm_set_lmep_cc_enable_to_sdk(p_sdb_lmep);
            p_sdb_lmep->cci_enabled = TRUE;  
            continue;    
        }
    }

    return SAI_STATUS_SUCCESS;
}
#endif


/**
 * Routine Description:
 *    @brief set lmep cc enable
 *
 * Arguments:
 *    @param[in] attr_list - cfm local mep parameters
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t
ctc_sai_cfm_set_lmep_cc_enable(
    _In_ const sai_attribute_t *attr_list)
{
    stbl_cfm_lmep_key_t       lmep_key;
    stbl_cfm_lmep_t             *p_db_lmep = NULL;    
    sai_status_t                    ret = SAI_STATUS_SUCCESS;
#ifdef GREATBELT    
    ctc_sai_port_flag_t          port_flag = 0;
#endif

    /* reserve cci_enabled */ 
    lmep_key.index = attr_list[0].value.cfm.mepIndex;
    p_db_lmep = stbl_cfm_lmep_get_cfm_lmep(&lmep_key);
    if (!p_db_lmep)
    {
        return SAI_STATUS_FAILURE;
    }

    p_db_lmep->cci_enabled = attr_list[0].value.cfm.cci_enable;

#ifdef GREATBELT
    /* check interface status for GB down mep
         if interface is not up, don't set cc enable to sdk */
    if (!p_db_lmep->direction)
    {
        ctc_sai_port_get_port_flag(attr_list[0].value.cfm.glPort, &port_flag);
        if (!CTC_FLAG_ISSET(port_flag, SAI_PORT_FLAG_RUNNING))
        {
             return SAI_STATUS_SUCCESS;   
        }
    }
#endif

    ret = ctc_sai_cfm_set_lmep_cc_enable_to_sdk(p_db_lmep);
    if (ret)
    {
        return ret;    
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * Routine Description:
 *    @brief set lmep ccm vlan cos
 *
 * Arguments:
 *    @param[in] attr_list - cfm local mep parameters
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t
ctc_sai_cfm_set_lmep_ccm_vlan_cos(
    _In_ const sai_attribute_t *attr_list)
{
    ctc_oam_update_t lmep_update;
    sai_status_t          ret = SAI_STATUS_SUCCESS;
    
    sal_memset(&lmep_update, 0, sizeof(ctc_oam_update_t));

    if (GLB_CFM_MODE_DOT1AG == attr_list[0].value.cfm.mode)
    {
        lmep_update.key.mep_type = CTC_OAM_MEP_TYPE_ETH_1AG;
    }
    else
    {
        lmep_update.key.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;
    }

    lmep_update.key.u.eth.gport = attr_list[0].value.cfm.glPort;
    lmep_update.key.u.eth.vlan_id = attr_list[0].value.cfm.vlanid;
    if (GLB_VLAN_NONE == lmep_update.key.u.eth.vlan_id)
    {
        CTC_SET_FLAG(lmep_update.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
    }
    
    if (attr_list[0].value.cfm.direction)
    {
        CTC_SET_FLAG(lmep_update.key.flag, CTC_OAM_KEY_FLAG_UP_MEP);
    }
    lmep_update.key.u.eth.md_level = attr_list[0].value.cfm.level;
    
    lmep_update.is_local = TRUE;
    lmep_update.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_TX_COS_EXP;
    lmep_update.update_value = attr_list[0].value.cfm.ccm_vlan_cos;
        
    /* set ccm vlan cos */
    ret = ctc_oam_update_lmep(&lmep_update);
    if (ret)
    {
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * Routine Description:
 *    @brief add cfm remote mep
 *
 * Arguments:
 *    @param[in] attr_list - cfm remote mep parameters
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t
ctc_sai_cfm_add_rmep(
    _In_ const sai_attribute_t *attr_list)
{
    ctc_oam_rmep_t    rmep;
    sai_status_t          ret = SAI_STATUS_SUCCESS;
    stbl_cfm_rmep_t    srmep;

    sal_memset(&rmep, 0, sizeof(ctc_oam_rmep_t));
    sal_memset(&srmep, 0, sizeof(stbl_cfm_rmep_t));

    /* add remote mep to sdk */
    if (GLB_CFM_MODE_DOT1AG == attr_list[0].value.cfm.mode)
    {
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_ETH_1AG;
    }
    else
    {
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;
    }

    rmep.key.u.eth.gport = attr_list[0].value.cfm.glPort;
    rmep.key.u.eth.vlan_id = attr_list[0].value.cfm.vlanid;
    if (GLB_VLAN_NONE == rmep.key.u.eth.vlan_id)
    {
        CTC_SET_FLAG(rmep.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
    }

    if (attr_list[0].value.cfm.direction)
    {
        CTC_SET_FLAG(rmep.key.flag, CTC_OAM_KEY_FLAG_UP_MEP);
    }
    rmep.key.u.eth.md_level = attr_list[0].value.cfm.level;

    if (attr_list[0].value.cfm.active)
    {
        CTC_SET_FLAG(rmep.u.y1731_rmep.flag, CTC_OAM_Y1731_LMEP_FLAG_MEP_EN);
    }

    CTC_SET_FLAG(rmep.u.y1731_rmep.flag, CTC_OAM_Y1731_RMEP_FLAG_MAC_UPDATE_EN);
    
    rmep.u.y1731_rmep.rmep_id = attr_list[0].value.cfm.mepid;
    rmep.rmep_index = attr_list[0].value.cfm.mepIndex;

    sal_memcpy(rmep.u.y1731_rmep.rmep_mac, attr_list[0].value.cfm.rmep_mac, 6);
    
    ret = ctc_oam_add_rmep(&rmep);
    if (ret)
    {
        return SAI_STATUS_FAILURE;
    }

    /* add remote mep to db */
    srmep.key.index = rmep.rmep_index;
    srmep.gport = rmep.key.u.eth.gport;
    srmep.md_level = rmep.key.u.eth.md_level;
    srmep.vlan_id = rmep.key.u.eth.vlan_id;
    srmep.rmep_id = rmep.u.y1731_rmep.rmep_id;
    srmep.ifindex = attr_list[0].value.cfm.ifindex;
    sal_memcpy(srmep.rmep_mac, rmep.u.y1731_rmep.rmep_mac, GLB_ETH_ADDR_LEN);

    ret = stbl_cfm_rmep_add_cfm_rmep(&srmep);
    if (ret)
    {
        return SAI_STATUS_FAILURE;
    }
        
    return SAI_STATUS_SUCCESS;
}

/**
 * Routine Description:
 *    @brief del cfm remote mep
 *
 * Arguments:
 *    @param[in] attr_list - cfm remote mep parameters
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t
ctc_sai_cfm_del_rmep(
    _Inout_ sai_attribute_t *attr_list)
{
    ctc_oam_rmep_t                  rmep;
    stbl_cfm_rmep_master_t      *p_sdb_rmep_master = NULL;
    ctclib_slistnode_t                 *listnode = NULL;
    stbl_cfm_rmep_t                  *p_sdb_rmep = NULL;
    bool                                     rmep_exist = FALSE;

    sal_memset(&rmep, 0, sizeof(ctc_oam_rmep_t));

    /* set local mep parameters before delete from sdk */
    if (GLB_CFM_MODE_DOT1AG == attr_list[0].value.cfm.mode)
    {
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_ETH_1AG;
    }
    else
    {
        rmep.key.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;
    }

    rmep.key.u.eth.gport = attr_list[0].value.cfm.glPort;
    rmep.key.u.eth.vlan_id = attr_list[0].value.cfm.vlanid;
    if (GLB_VLAN_NONE == rmep.key.u.eth.vlan_id)
    {
        CTC_SET_FLAG(rmep.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
    }
    
    rmep.key.u.eth.md_level = attr_list[0].value.cfm.level;
    rmep.u.y1731_rmep.rmep_id = attr_list[0].value.cfm.mepid;
    
    /* get remote mep index from db */
    p_sdb_rmep_master = stbl_cfm_rmep_get_master();

    CTCLIB_SLIST_LOOP(p_sdb_rmep_master->cfm_rmep_list, p_sdb_rmep, listnode)
    {
        if ((p_sdb_rmep->gport == rmep.key.u.eth.gport)
            && (p_sdb_rmep->vlan_id == rmep.key.u.eth.vlan_id)
            && (p_sdb_rmep->rmep_id == rmep.u.y1731_rmep.rmep_id))
        {
            rmep_exist = TRUE;
            break;    
        }
    }

    if (!rmep_exist)
    {
        CTC_SAI_DEBUG("%s %d remote mep gport %llx vlan_id %d md_level %d not exist\n", __func__, __LINE__,
            rmep.key.u.eth.gport, rmep.key.u.eth.vlan_id, rmep.key.u.eth.md_level);
        return SAI_STATUS_SUCCESS;
    }

    /* reserve mep index and mep number */
    attr_list->value.cfm.mepIndex = p_sdb_rmep->key.index;
    rmep.rmep_index = p_sdb_rmep->key.index;

    /* delete remote mep from db */
    stbl_cfm_rmep_del_cfm_rmep(&p_sdb_rmep->key);

    /* delete remote mep from sdk */
    ctc_oam_remove_rmep(&rmep);

    return SAI_STATUS_SUCCESS;
}

/**
 * Routine Description:
 *    @brief set lmep rdi
 *
 * Arguments:
 *    @param[in] attr_list - cfm local mep parameters
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t
ctc_sai_cfm_set_lmep_rdi(
    _In_ const sai_attribute_t *attr_list)
{
    ctc_oam_update_t lmep_update;
    sai_status_t          ret = SAI_STATUS_SUCCESS;
    
    sal_memset(&lmep_update, 0, sizeof(ctc_oam_update_t));

    if (GLB_CFM_MODE_DOT1AG == attr_list[0].value.cfm.mode)
    {
        lmep_update.key.mep_type = CTC_OAM_MEP_TYPE_ETH_1AG;
    }
    else
    {
        lmep_update.key.mep_type = CTC_OAM_MEP_TYPE_ETH_Y1731;
    }

    lmep_update.key.u.eth.gport = attr_list[0].value.cfm.glPort;
    lmep_update.key.u.eth.vlan_id = attr_list[0].value.cfm.vlanid;
    if (GLB_VLAN_NONE == lmep_update.key.u.eth.vlan_id)
    {
        CTC_SET_FLAG(lmep_update.key.flag, CTC_OAM_KEY_FLAG_LINK_SECTION_OAM);
    }
    
    if (attr_list[0].value.cfm.direction)
    {
        CTC_SET_FLAG(lmep_update.key.flag, CTC_OAM_KEY_FLAG_UP_MEP);
    }
    lmep_update.key.u.eth.md_level = attr_list[0].value.cfm.level;
    
    lmep_update.is_local = TRUE;
    lmep_update.update_type = CTC_OAM_Y1731_LMEP_UPDATE_TYPE_RDI;
    lmep_update.update_value = 0;

    /* clear lmep rdi */
    ret = ctc_oam_update_lmep(&lmep_update);
    if (ret)
    {
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_cfm_init()
{
    return CTC_SAI_NOTIFY_DONE;
}

static sai_status_t
__init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;

    CTC_SAI_ERROR_GOTO(ctc_cfm_init(),ret,out);
    preg->init_status =  INITIALIZED;

out:
    return ret;
}

static sai_status_t
__exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.2 */
static sai_cfm_api_t      g_sai_api_func = {
    .cfm_enable      = ctc_sai_cfm_enable,
    .cfm_add_maid  = ctc_sai_cfm_add_maid,
    .cfm_del_maid   = ctc_sai_cfm_del_maid,
    .cfm_add_lmep  = ctc_sai_cfm_add_lmep,
    .cfm_del_lmep   = ctc_sai_cfm_del_lmep,
    .cfm_add_rmep  = ctc_sai_cfm_add_rmep,
    .cfm_del_rmep   = ctc_sai_cfm_del_rmep,
    .cfm_set_cc_enable = ctc_sai_cfm_set_lmep_cc_enable,
    .cfm_set_ccm_vlan_cos = ctc_sai_cfm_set_lmep_ccm_vlan_cos,
    .cfm_set_rdi = ctc_sai_cfm_set_lmep_rdi,
};

static ctc_sai_api_reg_info_t g_api_reg_info = {
        .id         = SAI_API_CFM,
        .init_func  = __init_mode_fn,
        .exit_func  = __exit_mode_fn,
        .api_method_table = &g_sai_api_func,
        .private_data     = NULL,
};

int32
sai_cfm_isr(SAI_CFM_ISR_CB fn)
{
    g_hsrv_cfm_isr_cb = fn;
    return SAI_STATUS_SUCCESS;
}

int32
ctc_cfm_isr(sai_attribute_t *attr)
{
    if (NULL != g_hsrv_cfm_isr_cb)
    {
        (*g_hsrv_cfm_isr_cb)(attr);
    }
    
    return 0;
}

int32                                                                                                                                          
ctc_cfm_isr_callback(uint8 gchip, void* p_data)    
{
    ctc_oam_event_t            *p_event = NULL;
    uint32                            i = 0;
    sai_attribute_t                attr[1];
    stbl_cfm_lmep_key_t      lmep_key;
    stbl_cfm_lmep_t             *p_slmep = NULL;
    stbl_cfm_rmep_key_t      rmep_key;
    stbl_cfm_rmep_t             *p_srmep = NULL;
    int32                              rc = SAI_STATUS_SUCCESS;
    
    if (!p_data)                                                                                                                               
    {                                                                                                                                          
        return SAI_STATUS_SUCCESS;                                                                                                                    
    }

    CTC_SAI_DEBUG_FUNC();
    
    p_event = (ctc_oam_event_t*)p_data;
    for (i = 0; i < p_event->valid_entry_num; i++)                                                                                             
    {
        if ((CTC_OAM_MEP_TYPE_ETH_1AG != p_event->oam_event_entry[i].mep_type) 
            && (CTC_OAM_MEP_TYPE_ETH_Y1731 != p_event->oam_event_entry[i].mep_type)) 
        {                                                                                                                                      
            continue;                                                                                                                          
        }
        
        /* modified by liwh for bug 54201, 2019-11-07 */
        sal_memset(attr, 0, sizeof(attr));
        /* liwh end */

        attr[0].value.cfm.mepIndex = p_event->oam_event_entry[i].lmep_index;

        if (GLB_FLAG_ISSET(p_event->oam_event_entry[i].event_bmp, CTC_OAM_DEFECT_UNEXPECTED_MEP))
        {
             attr[0].value.cfm.d_unexp_mep = TRUE;
        }
        
        if (GLB_FLAG_ISSET(p_event->oam_event_entry[i].event_bmp, CTC_OAM_DEFECT_MISMERGE))
        {
            attr[0].value.cfm.d_mismerge = TRUE;
        }
        
        if (GLB_FLAG_ISSET(p_event->oam_event_entry[i].event_bmp, CTC_OAM_DEFECT_UNEXPECTED_LEVEL))
        {
            attr[0].value.cfm.d_meg_lvl = TRUE;
        }
        
        if (GLB_FLAG_ISSET(p_event->oam_event_entry[i].event_bmp, CTC_OAM_DEFECT_EVENT_RDI_TX))
        {
            attr[0].value.cfm.rdi = TRUE;
        }

        if (p_event->oam_event_entry[i].is_remote)
        {
            attr[0].value.cfm.rmepIndex = p_event->oam_event_entry[i].rmep_index;

            if (GLB_FLAG_ISSET(p_event->oam_event_entry[i].event_bmp, CTC_OAM_DEFECT_EVENT_RX_FIRST_PKT))
            {
                attr[0].value.cfm.first_pkt_rx = TRUE;
            }
            
            if (GLB_FLAG_ISSET(p_event->oam_event_entry[i].event_bmp, CTC_OAM_DEFECT_DLOC))
            {
                attr[0].value.cfm.d_loc = TRUE;
            }
            
            if (GLB_FLAG_ISSET(p_event->oam_event_entry[i].event_bmp, CTC_OAM_DEFECT_UNEXPECTED_PERIOD))
            {
                attr[0].value.cfm.d_unexp_period = TRUE;
            }
            
            if (GLB_FLAG_ISSET(p_event->oam_event_entry[i].event_bmp, CTC_OAM_DEFECT_EVENT_RDI_RX))
            {
                attr[0].value.cfm.rmep_last_rdi = TRUE;
            }

            if (GLB_FLAG_ISSET(p_event->oam_event_entry[i].event_bmp, CTC_OAM_DEFECT_SRC_MAC_MISMATCH))
            {
                attr[0].value.cfm.ma_sa_mismatch = TRUE;    
            }
        }

        CTC_SAI_DEBUG("ctc_cfm_isr_callback mep index %d is_remote %d", p_event->oam_event_entry[i].lmep_index,
            p_event->oam_event_entry[i].is_remote);

        /* get stbl local mep */
        sal_memset(&lmep_key, 0, sizeof(stbl_cfm_lmep_key_t));
        lmep_key.index = p_event->oam_event_entry[i].lmep_index;
        p_slmep = stbl_cfm_lmep_get_cfm_lmep(&lmep_key);
        if (!p_slmep)
        {
            continue;
        }

        //p_slmep->rdi = attr[0].value.cfm.rdi;
        
        attr[0].value.cfm.level = p_slmep->md_level;
        attr[0].value.cfm.vlanid = p_slmep->vlan_id;
        attr[0].value.cfm.ifindex = p_slmep->ifindex;         

        /* get stbl remote mep */
        if (p_event->oam_event_entry[i].is_remote)
        {
            sal_memset(&rmep_key, 0, sizeof(stbl_cfm_rmep_key_t));
            rmep_key.index = p_event->oam_event_entry[i].rmep_index;
            p_srmep = stbl_cfm_rmep_get_cfm_rmep(&rmep_key);
            if (!p_srmep)
            {
                continue;
            }
            
            attr[0].value.cfm.rmepid = p_srmep->rmep_id;

             if (attr[0].value.cfm.ma_sa_mismatch)
            {
                ctc_oam_mep_info_t  mep_info;    
                
                mep_info.mep_index = rmep_key.index;
                mep_info.is_rmep = TRUE;
                rc = ctc_oam_get_mep_info(0, &mep_info);
                if (rc)
                {
                    CTC_SAI_DEBUG("Get rmep info failure for mep index %d", rmep_key.index);
                }

                sal_memcpy(attr[0].value.cfm.rmep_mac, mep_info.rmep.y1731_rmep.mac_sa, GLB_ETH_ADDR_LEN);
            }
        }
        
        ctc_cfm_isr(attr);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
ctc_sai_cfm_init()
{
    api_reg_register_fn(&g_api_reg_info);

    return SAI_STATUS_SUCCESS;
}

