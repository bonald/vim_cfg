/****************************************************************************
* $Id$
*  Centec fdb information related MACRO, ENUM, Date Structure defines file
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
#include "hsrv_brgif.h"
#include "gen/tbl_fea_fdb_define.h"
#include "gen/tbl_fea_fdb.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_mcfdb.h"
#include "gen/tbl_fea_brg_if_define.h"
#include "gen/tbl_fea_brg_if.h"
#include "gen/tbl_mlag_define.h"
#include "gen/tbl_mlag.h"
#include "glb_tempfile_define.h"
#include "glb_openflow_define.h"
#include "ctc_api.h"
#include "hsrv_interface.h"
#include "ctc_sai_port.h"
#include "ctc_task.h"

extern sal_mutex_t *g_hsrv_brgif_mutex;
ctc_task_t* p_learn_enable_timer = NULL;

/* modified by liwh for bug 37200, 2016-08-24 */
extern int32 hsrv_fdb_set_all_flushing(bool set);
/* liwh end */

extern int32
hsrv_tunneldb_cdb_fdb_oper_add_function(tbl_fea_fdb_t *p_fdb);

extern int32
hsrv_tunneldb_cdb_fdb_oper_del_function(tbl_fea_fdb_t *p_fdb);

extern int32
hsrv_tunneldb_cdb_fdb_oper_update_function(tbl_fea_fdb_t *p_fdb);

typedef enum
{
    HSRV_ADD_FDB_CREATE_NEW_FDB_FAILURE,
    HSRV_ADD_FDB_ADD_TO_SAI_FAILURE,
    HSRV_ADD_FDB_ADD_TO_HSRV_DB_FAILURE,
} hsrv_add_new_fdb_failure_t;

extern hsrv_fdb_t g_hsrv_fdb;

#define __HSRV_FDB_DB_IF_VLAN_FUNC__

hsrv_fdb_vlan_t*
hsrv_fdb_vlan_get(vid_t vid)
{
    if (GLB_MAX_VLAN_ID <= vid)
    {
        return NULL;
    }
    return (g_hsrv_fdb.vlan_array[vid]);
}

int32
hsrv_fdb_vlan_add(vid_t vid)
{
    hsrv_fdb_vlan_t *p_db_vlan = NULL;

    /* 1. lookup entry exist */
    p_db_vlan = hsrv_fdb_vlan_get(vid);
    if (p_db_vlan)
    {
        return HSRV_E_NONE;
    }

    /* 2. alloc a new db entry */
    p_db_vlan = XCALLOC(MEM_FEA_HSRV_VLAN, sizeof(hsrv_fdb_vlan_t));
    if (NULL == p_db_vlan)
    {
        return HSRV_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    p_db_vlan->vid = vid;
#ifdef FDB_SLIST
    p_db_vlan->fdb_list = ctclib_slist_create(NULL, NULL);
#else
    CTCLIB_TAILQ_INIT(&p_db_vlan->fdb_queue);
#endif

    /* 4. add to db */
    g_hsrv_fdb.vlan_array[vid] = p_db_vlan;

    return HSRV_E_NONE;
}

int32
hsrv_fdb_vlan_del(vid_t vid)
{
    hsrv_fdb_vlan_t *p_db_vlan = NULL;

    /* 1. lookup entry exist */
    p_db_vlan = hsrv_fdb_vlan_get(vid);
    if (NULL == p_db_vlan)
    {
        return HSRV_E_NONE;
    }

    /* 3. delete from db */
    g_hsrv_fdb.vlan_array[vid] = NULL;

    /* 4. free db entry */
#ifdef FDB_SLIST
    ctclib_slist_delete(p_db_vlan->fdb_list);
#endif
    XFREE(MEM_FEA_HSRV_VLAN, p_db_vlan);

    return HSRV_E_NONE;
}

tbl_fea_fdb_t*
hsrv_fdb_lookup_fdb(tbl_fea_fdb_t *p_fdb)
{
    return tbl_fea_fdb_get_fea_fdb(&p_fdb->key);
}

#define __HSRV_FDB_SECURITY_FUNC__

int32
hsrv_vlan_security_check_security_mac_num(vid_t vid, uint32 add_entry, uint32 *p_allow_add)
{
    tbl_vlan_key_t key;
    tbl_vlan_t *p_db_vlan = NULL;

    key.vid = vid;
    p_db_vlan = tbl_vlan_get_vlan(&key);
    if (NULL == p_db_vlan)
    {
        return HSRV_E_NONE;
    }


    HSRV_FDB_DEBUG(" db: max_mac_count=%u, cur_mac_count=%u",
        p_db_vlan->max_mac_count, p_db_vlan->cur_mac_count);

    if (add_entry)
    {
#ifdef GREATBELT
        /* GG added the dynamic FDB in chip, and hsrv must add this FDB, otherwise, the SW/HW FDB is inconsistent */
        if (p_db_vlan->cur_mac_count == p_db_vlan->max_mac_count)
        {
            *p_allow_add = FALSE;
            return HSRV_E_NONE;
        }
#endif
        p_db_vlan->cur_mac_count++;
        *p_allow_add = TRUE;
    }
    else
    {
        if (p_db_vlan->cur_mac_count <= 0)
        {
            HSRV_FDB_DEBUG("vid = %d, current mac number equals or less than 0.\n", vid);
            return HSRV_E_INVALID_PARAM;
        }

        p_db_vlan->cur_mac_count--;
    }

    HSRV_FDB_DEBUG(" new db: max_mac_count=%u, cur_mac_count=%u",
        p_db_vlan->max_mac_count, p_db_vlan->cur_mac_count);

    return HSRV_E_NONE;
}

int32
hsrv_security_check_security_mac_num(tbl_fea_brg_if_t *p_db_if, uint32 add_entry, uint32 is_static, uint32 *p_allow_add)
{
    /*Add mac security entry number */
    if (add_entry)
    {
#ifndef GREATBELT   /* GOLDENGATE & DUET2 */
        if (p_db_if->curr_mac >= p_db_if->max_mac)
        {
            *p_allow_add = FALSE;
            return HSRV_E_NONE;
        }
#else     
        if (p_db_if->curr_mac + p_db_if->curr_mac_static >= p_db_if->max_mac)
        {
            *p_allow_add = FALSE;
            return HSRV_E_NONE;
        }
#endif
        *p_allow_add = TRUE;
     }
    /*Delete mac security entry number*/
    else
    {
        if (p_db_if->curr_mac < 0)
        {
            log_sys(M_MOD_INTERNAL, E_ERROR, "ifindex = %d, current mac number less than 0.\n", p_db_if->key.ifindex);
            return HSRV_E_INVALID_PARAM;
        }
    }

    return HSRV_E_NONE;
}

int32
hsrv_security_op_security(uint32 ifindex, ds_brgif_t  *p_brgif)
{
    tbl_interface_t  *p_db_if = NULL;
    ds_brgif_t       *p_db_brgif = NULL;
    
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (!p_db_if)
    {
        HSRV_LOG_ERR("hsrv_security_op_security can't find tbl_interface for ifindex %d", ifindex);

    }
    p_db_brgif = p_brgif;
    if (!p_db_brgif)
    {
        p_db_brgif = ds_brgif_get_brgif(p_db_if);
        if (NULL == p_db_brgif)
        {
            HSRV_LOG_ERR("hsrv_security_op_security can't find ds_brgif for ifindex %d", ifindex);

        }
    }
    hsrv_lag_set_mem_port_security(p_db_if,p_db_brgif);
    return HSRV_E_NONE;
}

int32
hsrv_vlan_security_op_security(vid_t vid)
{
#if 0 /*Removed by xgu, for GG system support hw based vlan security, no need this logic in SW here, 2014-3-27*/
    hsrv_vlan_security_data_t* p_vlan_data = NULL;

    HSRV_FDB_DEBUG_FUNC();

    HSRV_FDB_DEBUG("  vid=%d", vid);

    p_vlan_data = _hsrv_vlan_security_lookup_vlan_data(vid);
    HSRV_PTR_CHECK(p_vlan_data);

    HSRV_FDB_DEBUG(" db: max-mac-num=%u, cur-mac-num=%u, action=%s",
        p_vlan_data->max_mac_num, p_vlan_data->cur_mac_num, GLB_VLAN_SEC_ACT_TO_STR(p_vlan_data->action));

    if (p_vlan_data->is_changing)
    {
        /*The current mac num is increasing to the max mac num*/
        if (p_vlan_data->cur_mac_num == p_vlan_data->max_mac_num)
        {
            hsrv_vlan_security_set_maclimit(NULL, vid, p_vlan_data->action,0);
        }
        /*The current mac num is decreasing from the max mac num*/
        else if (p_vlan_data->cur_mac_num < p_vlan_data->max_mac_num)
        {
            hsrv_vlan_security_set_maclimit(NULL, vid, GLB_VLAN_SECURITY_ACTION_NONE);
        }
        p_vlan_data->is_changing = FALSE;
    }
#endif
    return HSRV_E_NONE;
}

#define __HSRV_FDB_DB_FUNC__

static int32
_hsrv_fdb_check_fdb_number()
{
    int32 rc = HSRV_E_NONE;
    tbl_openflow_t    *p_openflow   = tbl_openflow_get_openflow();

    if (p_openflow->fdb_flow_valid)
    {
        /* add for hybrid fdb flow resource check */
        if ((g_hsrv_fdb.hsrv_valid_fdb_number + GLB_OPENFLOW_FDB_FLOW_MAX_NUM) 
             >= lib_spec_get_field(TBL_SYS_SPEC_FLD_UCAST_FDB))
        {
            rc = GLB_E_RESOURCE_FULL;
        }
    }
    else
    {
        if (g_hsrv_fdb.hsrv_valid_fdb_number >= lib_spec_get_field(TBL_SYS_SPEC_FLD_UCAST_FDB))
        {
            rc = GLB_E_RESOURCE_FULL;
        }
    }

    return rc;
}

int32
hsrv_fdb_set_learning_en(uint32 enable)
{
    int32 ret = 0;
    ctc_learning_action_info_t learning_action;

    if (g_hsrv_fdb.hsrv_learn_enable == enable)
    {
        return HSRV_E_NONE;
    }

    g_hsrv_fdb.hsrv_learn_enable = enable;

    learning_action.action = CTC_LEARNING_ACTION_MAC_TABLE_FULL;
    learning_action.value = (enable) ? FALSE : TRUE;
    ret = ctc_set_learning_action(&learning_action);
    if (ret < 0)
    {
        return HSRV_E_INVALID_PARAM;
    }
    return ret;
}

static void
_hsrv_fdb_set_learning_en_timer_handle()
{
    p_learn_enable_timer = NULL;
    
    /* modified by liwh for bug 37200, 2016-08-24 */
    tbl_brg_global_t  *p_brg_glb = NULL;

    p_brg_glb = tbl_brg_global_get_brg_global();
    if (p_brg_glb && p_brg_glb->fdb_all_flushing)
    {
         hsrv_fdb_flush_fdb_all();
         hsrv_fdb_set_all_flushing(FALSE);   
    }
    /* liwh end */


    if (!g_hsrv_fdb.hsrv_learn_enable)
    {
        if (GLB_E_RESOURCE_FULL != _hsrv_fdb_check_fdb_number())
        {
            hsrv_fdb_set_learning_en(TRUE);
        }
    }

    p_learn_enable_timer = CTC_TASK_ADD_TIME(_hsrv_fdb_set_learning_en_timer_handle, NULL, HSRV_FDB_LEARNING_EN_TIME);

    return;
}

/* modified by liwh for bug 37200, 2016-08-25 */
int32
hsrv_fdb_reset_learning_timer(void)
{
    if (p_learn_enable_timer)
    {
        CTC_TASK_DEL_TIME(p_learn_enable_timer);
        p_learn_enable_timer = NULL;
    }
    
    p_learn_enable_timer = CTC_TASK_ADD_TIME(_hsrv_fdb_set_learning_en_timer_handle, NULL, HSRV_FDB_LEARNING_EN_TIME);

    return HSRV_E_NONE;
}
/* liwh end */

int32
_hsrv_fdb_get_dynamic_one(tbl_fea_fdb_t *p_fdb, tbl_iter_args_t pargs)
{
    tbl_fea_fdb_t **pp_db_fdb = NULL;

    if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
    {
        pp_db_fdb = (tbl_fea_fdb_t**)pargs.argv[0];
        *pp_db_fdb = p_fdb;
        return HSRV_E_ENTRY_EXIST;
    }
    return HSRV_E_NONE;
}

static int32
_hsrv_fdb_remove_one_dynamic_fdb()
{
    tbl_fea_fdb_t *p_db_fdb = NULL;
    tbl_iter_args_t args;

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = &p_db_fdb;

    tbl_fea_fdb_iterate((TBL_ITER_CB_FUNC)_hsrv_fdb_get_dynamic_one, &args);
    if (p_db_fdb)
    {
        _hsrv_fdb_del_fdb(p_db_fdb, FALSE, TRUE);
    }

    return HSRV_E_NONE;
}

#ifdef GREATBELT
int32
_hsrv_fdb_get_security_dynamic_one(tbl_fea_fdb_t *p_fdb, tbl_iter_args_t *pargs)
{
    tbl_fea_fdb_t **pp_db_fdb = NULL;
    uint32 ifindex = *((uint32 *)pargs->argv[0]);
    
    if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC) && GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
    {
        if (ifindex == p_fdb->ifindex)
        {
            pp_db_fdb = (tbl_fea_fdb_t**)pargs->argv[1];
            *pp_db_fdb = p_fdb;
            return HSRV_E_ENTRY_EXIST;
        }  
    }
    return HSRV_E_NONE;
}

static int32
_hsrv_fdb_remove_one_security_dynamic_fdb(uint32 ifindex)
{
    tbl_fea_fdb_t *p_db_fdb = NULL;
    tbl_iter_args_t args;

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = &ifindex;
    args.argv[1] = &p_db_fdb;

    tbl_fea_fdb_iterate((TBL_ITER_CB_FUNC)_hsrv_fdb_get_security_dynamic_one, &args);
    if (p_db_fdb)
    {
        _hsrv_fdb_del_fdb(p_db_fdb, FALSE, TRUE);
    }

    return HSRV_E_NONE;
}
#endif

int32
hsrv_fdb_add_check_full(tbl_fea_fdb_t *p_fdb)
{
    if (GLB_E_RESOURCE_FULL == _hsrv_fdb_check_fdb_number())
    {
        hsrv_fdb_set_learning_en(FALSE);
        if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC) && (p_fdb->key.fid < GLB_MAX_VLAN_ID))
        {
            _hsrv_fdb_remove_one_dynamic_fdb();
        }
        else if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC) && (p_fdb->key.fid < GLB_MAX_VLAN_ID))
        {
            logid_sys(LOG_FDB_4_OVERFLOW);
            return GLB_E_RESOURCE_FULL;
        }
    }
    return HSRV_E_NONE;
}

static void
_hsrv_fdb_count_fdb_number(tbl_fea_fdb_t *p_fdb, uint32 add)
{
    tbl_fea_brg_if_t *p_db_if = NULL;
    tbl_fea_brg_if_key_t fea_brg_if_key;

    sal_memset(&fea_brg_if_key, 0, sizeof(fea_brg_if_key));
    fea_brg_if_key.ifindex = p_fdb->ifindex;
    p_db_if = tbl_fea_brg_if_get_fea_brg_if(&fea_brg_if_key); /* Modified by yejl for bug 39502, do not check the p_db_if is NULL*/

    /* static security entry */
    if (add)
    {
        g_hsrv_fdb.hsrv_valid_fdb_number++;
    }
    else
    {
        g_hsrv_fdb.hsrv_valid_fdb_number--;
    }

    if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC | GLB_FDB_FLAG_MAC_SECURITY_EN))
    {
        /* static security entry */
        if (add)
        {
            g_hsrv_fdb.static_security_entry_cnt++;
            if (NULL != p_db_if) /* Modified by yejl for bug 39502, check the p_db_if is NULL or not*/
            {
                p_db_if->curr_mac_static++;
            }
        }
        else
        {
            g_hsrv_fdb.static_security_entry_cnt--;
            if (NULL != p_db_if)/* Modified by yejl for bug 39502, check the p_db_if is NULL or not*/
            {
                p_db_if->curr_mac_static--;
            }
        }
    }
    else if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
    {
        /* dynamic security entry */
        if (add)
        {
            g_hsrv_fdb.dynamic_security_entry_cnt++;
            if (NULL != p_db_if)/* Modified by yejl for bug 39502, check the p_db_if is NULL or not*/
            {
                p_db_if->curr_mac++;
            }
        }
        else
        {
            g_hsrv_fdb.dynamic_security_entry_cnt--;
            if (NULL != p_db_if)/* Modified by yejl for bug 39502, check the p_db_if is NULL or not*/
            {
                p_db_if->curr_mac--;
            }
        }
    }
    else if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC) && (p_fdb->key.fid < GLB_MAX_VLAN_ID))
    {
        /* static FDB entry */
        if (!GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_FWD_TOCPU))
        {
            if (add)
            {
                g_hsrv_fdb.static_fdb_cnt++;
                if (NULL != p_db_if)/* Modified by yejl for bug 39502, check the p_db_if is NULL or not*/
                {
                    p_db_if->curr_mac_static++;
                }
            }
            else
            {
                g_hsrv_fdb.static_fdb_cnt--;
                if (NULL != p_db_if)/* Modified by yejl for bug 39502, check the p_db_if is NULL or not*/
                {
                    p_db_if->curr_mac_static--;
                }
            }
        }
    }
    else
    {
        /* dynamic FDB entry */
        if (add)
        {
            g_hsrv_fdb.dynamic_fdb_cnt++;
        }
        else
        {
            g_hsrv_fdb.dynamic_fdb_cnt--;
        }
    }

    return;
}

static int32 inline
_hsrv_bridge_add_new_fdb_failure(tbl_fea_fdb_t *p_fdb, tbl_fea_brg_if_t *p_db_if, hsrv_add_new_fdb_failure_t type)
{
    uint32 allow_add = FALSE;

    /* delete security entry number */
    if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
    {
        hsrv_security_check_security_mac_num(p_db_if, FALSE, (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC)), &allow_add);
    }

    /* delete vlan security entry number */
    if (p_fdb->key.fid < GLB_MAX_VLAN_ID)
    {
        hsrv_vlan_security_check_security_mac_num(p_fdb->key.fid, FALSE, &allow_add);
    }

#ifdef FDB_TODO
    /* release fdb node */
    if ((HSRV_ADD_FDB_ADD_TO_HAGT_FAILURE == type) || (HSRV_ADD_FDB_CREATE_NEW_FDB_FAILURE == type))
    {
        if (pst_fdb_node)
        {
            if (GLB_FLAG_ISSET(pst_fdb_node->flag, GLB_FDB_FLAG_STATIC))
            {
                /* add static FDB fail in HAGT, need to add this entry into fail static FDB list */
                _hsrv_bridge_add_fail_static_fdb(pst_fdb_node);
            }
            /* modified by yaom for bug 17873 20120113 */
            if (pst_fdb_node->vpls_info)
            {
                HSRV_FREE(CTCLIB_MEM_FDB_MODULE, pst_fdb_node->vpls_info);
                pst_fdb_node->vpls_info = NULL;
            }
            HSRV_FREE(CTCLIB_MEM_FDB_MODULE, pst_fdb_node);
        }
    }

    /* delete fdb from hagt and release node */
    if (HSRV_ADD_FDB_ADD_TO_HSRV_DB_FAILURE == type)
    {
        _hsrv_brg_del_fdb_from_hagt(pst_fdb_node);
        if (pst_fdb_node)
        {
            /* modified by yaom for bug 17873 20120113 */
            if (pst_fdb_node->vpls_info)
            {
                HSRV_FREE(CTCLIB_MEM_FDB_MODULE, pst_fdb_node->vpls_info);
                pst_fdb_node->vpls_info = NULL;
            }
            HSRV_FREE(CTCLIB_MEM_FDB_MODULE, pst_fdb_node);
        }
    }

#endif
    return HSRV_E_NONE;
}

static int32
_hsrv_fdb_db_list_add(tbl_fea_fdb_t *p_db_fdb)
{
    tbl_fea_brg_if_t *p_db_if = NULL;
    hsrv_fdb_vlan_t *p_vlan = NULL;
#ifdef FDB_SLIST
    ctclib_slistnode_t *p_node = NULL;
#endif

    p_db_if = hsrv_brgif_get(p_db_fdb->ifindex);
    if (NULL == p_db_if)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

    p_vlan = hsrv_fdb_vlan_get(p_db_fdb->key.fid);
    if (NULL == p_vlan)
    {
        return HSRV_E_ENTRY_NOT_EXIST;
    }

#ifdef FDB_SLIST
    p_node = ctclib_slistnode_insert_sort(p_db_if->fdb_list, p_db_fdb);
    if (NULL == p_node)
    {
        return HSRV_E_NO_MEMORY;
    }
#else
    CTCLIB_TAILQ_INSERT_HEAD(&p_db_if->fdb_queue, p_db_fdb, port_entry);
#endif

#ifdef FDB_SLIST
    p_node = ctclib_slistnode_insert_sort(p_vlan->fdb_list, p_db_fdb);
    if (NULL == p_node)
    {
        ctclib_slistnode_delete(p_db_if->fdb_list, p_db_fdb);
        return HSRV_E_NO_MEMORY;
    }
#else
    CTCLIB_TAILQ_INSERT_HEAD(&p_vlan->fdb_queue, p_db_fdb, vlan_entry);
#endif

    return HSRV_E_NONE;
}

static int32
_hsrv_fdb_db_list_update_port(uint32 old_ifindex, tbl_fea_fdb_t *p_db_fdb)
{
    tbl_fea_brg_if_t *p_old_if = NULL;
    tbl_fea_brg_if_t *p_new_if = NULL;
#ifdef FDB_SLIST
    ctclib_slistnode_t *p_node = NULL;
#endif

    p_old_if = hsrv_brgif_get(old_ifindex);
    if (p_old_if)
    {
#ifdef FDB_SLIST
        ctclib_slistnode_delete(p_old_if->fdb_list, p_db_fdb);
#else
        CTCLIB_TAILQ_REMOVE(&p_old_if->fdb_queue, p_db_fdb, port_entry);
        p_db_fdb->port_entry.tqe_next = NULL;
        p_db_fdb->port_entry.tqe_prev = NULL;
#endif
    }

    p_new_if = hsrv_brgif_get(p_db_fdb->ifindex);
    if (p_new_if)
    {
#ifdef FDB_SLIST
        p_node = ctclib_slistnode_insert_sort(p_new_if->fdb_list, p_db_fdb);
        if (NULL == p_node)
        {
            return HSRV_E_NO_MEMORY;
        }
#else
        CTCLIB_TAILQ_INSERT_HEAD(&p_new_if->fdb_queue, p_db_fdb, port_entry);
#endif
    }

    return HSRV_E_NONE;
}

static int32
_hsrv_fdb_db_list_del(tbl_fea_fdb_t *p_db_fdb)
{
    tbl_fea_brg_if_t *p_db_if = NULL;
    hsrv_fdb_vlan_t *p_vlan = NULL;
    int32 rc = HSRV_E_NONE;

    p_db_if = hsrv_brgif_get(p_db_fdb->ifindex);
    if (p_db_if)
    {
#ifdef FDB_SLIST
        rc |= ctclib_slistnode_delete(p_db_if->fdb_list, p_db_fdb);
#else
        CTCLIB_TAILQ_REMOVE(&p_db_if->fdb_queue, p_db_fdb, port_entry);
        p_db_fdb->port_entry.tqe_next = NULL;
        p_db_fdb->port_entry.tqe_prev = NULL;
#endif
    }

    p_vlan = hsrv_fdb_vlan_get(p_db_fdb->key.fid);
    if (p_vlan)
    {
#ifdef FDB_SLIST
        rc |= ctclib_slistnode_delete(p_vlan->fdb_list, p_db_fdb);
#else
        CTCLIB_TAILQ_REMOVE(&p_vlan->fdb_queue, p_db_fdb, vlan_entry);
        p_db_fdb->vlan_entry.tqe_next = NULL;
        p_db_fdb->vlan_entry.tqe_prev = NULL;
#endif
    }

    return rc;
}

int32
hsrv_fdb_db_add_fdb(tbl_fea_fdb_t *p_fdb, tbl_fea_brg_if_t *p_db_if)
{
    tbl_fea_fdb_t *p_db_fdb = NULL;
    int32 rc = HSRV_E_NONE;

    /* 1. add to hash */
    rc = tbl_fea_fdb_add_fea_fdb(p_fdb);
    if (rc < 0)
    {
        /* this is exception process function, ret will not be used. */
        _hsrv_bridge_add_new_fdb_failure(p_fdb, p_db_if, HSRV_ADD_FDB_CREATE_NEW_FDB_FAILURE);
        return rc;
    }

    /* 2. get hash entry */
    p_db_fdb = tbl_fea_fdb_get_fea_fdb(&p_fdb->key);
    if (NULL == p_db_fdb)
    {
        return HSRV_E_NO_MEMORY;
    }

    /* 3. add to interface/vlan list */
    rc = _hsrv_fdb_db_list_add(p_db_fdb);
    if (rc < 0)
    {
        /* this is exception process function, ret will not be used. */
        _hsrv_bridge_add_new_fdb_failure(p_fdb, p_db_if, HSRV_ADD_FDB_ADD_TO_HSRV_DB_FAILURE);
        return rc;
    }

    /* 4. update counters */
    _hsrv_fdb_count_fdb_number(p_db_fdb, TRUE);

#ifdef GREATBELT
    /*After fdb is added successfully, some operation, such as setting discard in chip may be processed here*/
    if (p_fdb->key.fid < GLB_MAX_VLAN_ID)
    {
        if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
        {
            if (p_db_if->curr_mac_static + p_db_if->curr_mac > p_db_if->max_mac)
            {
                if (GLB_FLAG_ISSET(p_fdb->flags, GLB_FDB_FLAG_STATIC))
                {
                    _hsrv_fdb_remove_one_security_dynamic_fdb(p_fdb->ifindex);
                }
            }
            hsrv_security_op_security(p_fdb->ifindex, NULL);
        }
        hsrv_vlan_security_op_security(p_fdb->key.fid);
    }
#endif

    hsrv_tunneldb_cdb_fdb_oper_add_function(p_fdb);

    return HSRV_E_NONE;
}

int32
hsrv_fdb_db_update_fdb(tbl_fea_fdb_t *p_fdb, tbl_fea_fdb_t *p_db_fdb)
{
    uint32 old_ifindex = 0;
    int32 rc = HSRV_E_NONE;
    tbl_fea_fdb_t db_fdb_for_tunnel;

    sal_memcpy(&db_fdb_for_tunnel, p_db_fdb, sizeof(tbl_fea_fdb_t));
    old_ifindex = p_db_fdb->ifindex;
    _hsrv_fdb_count_fdb_number(p_db_fdb, FALSE);

    p_db_fdb->ifindex = p_fdb->ifindex;
    p_db_fdb->portid = p_fdb->portid;
    p_db_fdb->flags = p_fdb->flags;
    p_db_fdb->mlag_source_ifindex = p_fdb->mlag_source_ifindex;
    rc = _hsrv_fdb_db_list_update_port(old_ifindex, p_db_fdb);
    if (rc < 0)
    {
        return rc;
    }

    _hsrv_fdb_count_fdb_number(p_db_fdb, TRUE);

    hsrv_tunneldb_cdb_fdb_oper_update_function(&db_fdb_for_tunnel);
    
    return HSRV_E_NONE;
}

int32
hsrv_fdb_db_del_fdb(tbl_fea_fdb_t *p_db_fdb)
{
    tbl_fea_brg_if_t *p_db_if = NULL;
    uint32 allow_add = FALSE;
    int32 rc = HSRV_E_NONE;
    tbl_fea_fdb_t db_fdb_for_tunnel;

    /* 1. del from interface/vlan list */
    sal_memcpy(&db_fdb_for_tunnel, p_db_fdb, sizeof(tbl_fea_fdb_t));
    
    rc = _hsrv_fdb_db_list_del(p_db_fdb);
    if (rc < 0)
    {
        log_sys(M_MOD_INTERNAL, E_WARNING, "Delete FDB failure from interface/vlan list: %02X:%02X:%02X:%02X:%02X:%02X vid: %u",
                    p_db_fdb->key.mac[0], p_db_fdb->key.mac[1], p_db_fdb->key.mac[2],
                    p_db_fdb->key.mac[3], p_db_fdb->key.mac[4], p_db_fdb->key.mac[5],
                    p_db_fdb->key.fid);
    }

    /* 2. change counters */
    if (g_hsrv_fdb.hsrv_valid_fdb_number)
    {
        HSRV_FDB_DEBUG("The current vlaid fdb number is %d, reduce one fdb entry\n",
                          g_hsrv_fdb.hsrv_valid_fdb_number);
        _hsrv_fdb_count_fdb_number(p_db_fdb, FALSE);
    }
    else
    {
        HSRV_FDB_DEBUG("The current valid fdb number is %d, the num is abnormal\n",
                          g_hsrv_fdb.hsrv_valid_fdb_number);
    }

    /* delete security mac number for old fdb entry */
    if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN))
    {
        p_db_if = hsrv_brgif_get(p_db_fdb->ifindex);
        if (p_db_if)
        {
            hsrv_security_check_security_mac_num(p_db_if, FALSE,
                (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC)) , &allow_add);
#ifdef GREATBELT
            hsrv_security_op_security(p_db_fdb->ifindex, NULL);
#endif
        }
    }

    /* delete vlan security mac number for old fdb entry */
    if (p_db_fdb->key.fid < GLB_MAX_VLAN_ID)
    {
        hsrv_vlan_security_check_security_mac_num(p_db_fdb->key.fid, FALSE, &allow_add);
        hsrv_vlan_security_op_security(p_db_fdb->key.fid);
    }

    /* 3. del from hash and free, should at last */
    rc = tbl_fea_fdb_del_fea_fdb(&p_db_fdb->key);
    if (rc < 0)
    {
        return rc;
    }

    hsrv_tunneldb_cdb_fdb_oper_del_function(&db_fdb_for_tunnel);
    
    return rc;
}

#define __HSRV_FDB_DB_GET__

int32
hsrv_fdb_sw_get_count(cfg_cmd_para_t *para)
{
    tbl_mcfdb_master_t *p_mcfdb_master = tbl_mcfdb_get_master();
    FILE *fp = NULL;
    uint32 static_security_cnt = 0;
    uint32 static_cnt = 0;
    uint32 dynamic_security_cnt = 0;
    uint32 dynamic_cnt = 0;
    uint32 multicast_cnt = 0;
    uint32 total_security_cnt = 0;
    uint32 total_cnt = 0;

    static_security_cnt = g_hsrv_fdb.static_security_entry_cnt;
    static_cnt = g_hsrv_fdb.static_fdb_cnt + static_security_cnt;
    dynamic_security_cnt = g_hsrv_fdb.dynamic_security_entry_cnt;
    dynamic_cnt = g_hsrv_fdb.dynamic_fdb_cnt + dynamic_security_cnt;
    total_security_cnt = static_security_cnt + dynamic_security_cnt;
    multicast_cnt = p_mcfdb_master->mcfdb_hash->count;
    total_cnt = static_cnt + dynamic_cnt + multicast_cnt;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "MAC Address Count:\n");
    sal_fprintf(fp, "(Include MAC security entry)\n");
    sal_fprintf(fp, "-------------------------------------------\n");
    sal_fprintf(fp, "Dynamic Address Count  : %5u    (Security: %u)\n", dynamic_cnt, dynamic_security_cnt);
    sal_fprintf(fp, "Static  Address Count  : %5u    (Security: %u)\n", static_cnt, static_security_cnt);
    sal_fprintf(fp, "Multicast Address Count: %5u    (Security: %u)\n", multicast_cnt, 0);
    sal_fprintf(fp, "Total Mac Addresses    : %5u    (Security: %u)\n", total_cnt, total_security_cnt);

    sal_fclose(fp);
    fp = NULL;

    return HSRV_E_NONE;
}

int32
hsrv_fdb_show_one(tbl_fea_fdb_t *p_db_fdb, hsrv_mac_addr_type_t type, FILE *fp)
{
    char ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_SIZE];
    char mac_str[CMD_BUF_32];
    char type_str[CMD_BUF_32];
    char mlag_str[CMD_BUF_32];
    uint32 is_static = FALSE;
    uint32 is_security = FALSE;

    is_static = GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC);
    is_security = GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MAC_SECURITY_EN);

    if ((!is_security) &&(type == HSRV_MAC_ADDR_TYPE_STATIC_PORT_SECURITY ||
        type == HSRV_MAC_ADDR_TYPE_DYNAMIC_PORT_SECURITY || type == HSRV_MAC_ADDR_TYPE_ALL_PORT_SECURITY))
    {
        return HSRV_E_NONE;
    }

    switch (type)
    {
    case HSRV_MAC_ADDR_TYPE_STATIC:
        if (!is_static)
        {
            return PM_E_NONE;
        }
        break;
    case HSRV_MAC_ADDR_TYPE_DYNAMIC:
        if (is_static)
        {
            return PM_E_NONE;
        }
        break;
    case HSRV_MAC_ADDR_TYPE_STATIC_PORT_SECURITY:
        if (!is_static || !is_security)
        {
            return PM_E_NONE;
        }
        break;
    case HSRV_MAC_ADDR_TYPE_DYNAMIC_PORT_SECURITY:
        if (is_static || !is_security)
        {
            return PM_E_NONE;
        }
        break;
    case HSRV_MAC_ADDR_TYPE_ALL_PORT_SECURITY:
        if (!is_security)
        {
            return PM_E_NONE;
        }
        break;

    default:
        break;
    }

    type_str[0] = CMD_ZERO_CHAR;
    if (is_static)
    {
        sal_strcat(type_str, "static");
    }
    else
    {
        sal_strcat(type_str, "dynamic");
    }
    if (is_security)
    {
        sal_strcat(type_str, "(*)");
    }

    tbl_interface_get_name_by_ifindex(p_db_fdb->ifindex, ifname, IFNAME_SIZE);
    IFNAME_ETH2SHORT(ifname, ifname_ext);

    mlag_str[0] = CMD_ZERO_CHAR;
    if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG))
    {
        sal_snprintf(mlag_str, CMD_BUF_32, "(M%s%s)",
            GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_OUT)?"O":"",
            GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_IN)?"I":"");
    }

     sal_fprintf(fp, "%-7u %-16s %-12s %s%s\n",
        p_db_fdb->key.fid,
        cdb_mac_addr_val2str(mac_str, CMD_BUF_32, p_db_fdb->key.mac),
        type_str,
        ifname_ext,
        mlag_str);

    return HSRV_E_NONE;
}

int32
hsrv_fdb_sw_show_banner(FILE *fp)
{
    sal_fprintf(fp, "            Mac Address Table\n");
    sal_fprintf(fp, "----------------------------------------------\n");
#ifndef OFPRODUCT
    sal_fprintf(fp, "(*)  - Security Entry     (M)  - MLAG Entry\n");
    sal_fprintf(fp, "(MO) - MLAG Output Entry  (MI) - MLAG Input Entry\n");
#endif /* OFPRODUCT */
    sal_fprintf(fp, "%-7s %-16s %-12s %-8s\n", "VLAN ID", "MAC Address", "Type", "Port");
    sal_fprintf(fp, "-------+----------------+------------+--------\n");
    return HSRV_E_NONE;
}

int32
hsrv_mcfdb_show_one(tbl_mcfdb_t *p_db_mcfdb, hsrv_mac_addr_type_t type, uint32 spec_ifindex, FILE *fp)
{
    char ifname[IFNAME_SIZE];
    char ifname_ext[IFNAME_SIZE];
    char mac_str[CMD_BUF_32];
    char type_str[CMD_BUF_32];
    char type_cfg_str[CMD_BUF_32];
    uint32 ifindex = 0;

    type_str[0] = CMD_ZERO_CHAR;
    type_cfg_str[0] = CMD_ZERO_CHAR;
    sal_strcat(type_str, "static");
    sal_strcat(type_cfg_str, "static(cfg)");
    cdb_mac_addr_val2str(mac_str, CMD_BUF_32, p_db_mcfdb->key.mac);

    if (GLB_IFINDEX_ALL == spec_ifindex)
    {
        GLB_BMP_ITER_BEGIN(p_db_mcfdb->l2mc_port, ifindex)
        {
            tbl_interface_get_name_by_ifindex(ifindex, ifname, IFNAME_SIZE);
            IFNAME_ETH2SHORT(ifname, ifname_ext);
            sal_fprintf(fp, "%-7u %-16s %-12s %s\n",
                p_db_mcfdb->key.fid, mac_str, (p_db_mcfdb->manual ? type_cfg_str : type_str), ifname_ext);
        }
        GLB_BMP_ITER_END(p_db_mcfdb->l2mc_port, ifindex);
    }
    else
    {
        tbl_interface_get_name_by_ifindex(spec_ifindex, ifname, IFNAME_SIZE);
        IFNAME_ETH2SHORT(ifname, ifname_ext);
        sal_fprintf(fp, "%-7u %-16s %-12s %s\n",
            p_db_mcfdb->key.fid, mac_str, (p_db_mcfdb->manual ? type_cfg_str : type_str), ifname_ext);
    }

    return HSRV_E_NONE;
}

int32
hsrv_fdb_sw_get_l2mc_interface(int32 ifindex, hsrv_mac_addr_type_t type, FILE *fp)
{
    tbl_mcfdb_master_t *p_master = tbl_mcfdb_get_master();
    tbl_mcfdb_t *p_db_mcfdb = NULL;
    ctclib_hash_backet_t *hb = NULL;
    ctclib_hash_backet_t *next = NULL;
    uint32 i = 0;

    if (HSRV_MAC_ADDR_TYPE_DYNAMIC == type || HSRV_MAC_ADDR_TYPE_UNICAST_ALL == type)
    {
        return HSRV_E_NONE;
    }

    for (i = 0; i < p_master->mcfdb_hash->size; i++)
    {
        for (hb = p_master->mcfdb_hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            p_db_mcfdb = hb->data;
            if (GLB_BMP_ISSET(p_db_mcfdb->l2mc_port, ifindex))
            {
                hsrv_mcfdb_show_one(p_db_mcfdb, type, ifindex, fp);
            }
        }
    }

    return HSRV_E_NONE;
}

int32
hsrv_fdb_sw_get_l2mc_vlan(vid_t vid, hsrv_mac_addr_type_t type, uint8 *mac, FILE *fp)
{
    tbl_mcfdb_master_t *p_master = tbl_mcfdb_get_master();
    tbl_mcfdb_t *p_db_mcfdb = NULL;
    ctclib_hash_backet_t *hb = NULL;
    ctclib_hash_backet_t *next = NULL;
    uint32 i = 0;

    if (HSRV_MAC_ADDR_TYPE_DYNAMIC == type || HSRV_MAC_ADDR_TYPE_UNICAST_ALL == type)
    {
        return HSRV_E_NONE;
    }

    for (i = 0; i < p_master->mcfdb_hash->size; i++)
    {
        for (hb = p_master->mcfdb_hash->index[i]; hb; hb = next)
        {
            next = hb->next;
            p_db_mcfdb = hb->data;
            if (p_db_mcfdb->key.fid == vid)
            {
                if ((NULL == mac) || (0 == sal_memcmp(mac, p_db_mcfdb->key.mac, MAC_ADDR_LEN)))
                {
                    hsrv_mcfdb_show_one(p_db_mcfdb, type, GLB_IFINDEX_ALL, fp);
                }
            }
        }
    }

    return HSRV_E_NONE;
}

int32
hsrv_fdb_sw_get_l2mc(hsrv_mac_addr_type_t type, uint8 *mac, FILE *fp)
{
    tbl_vlan_key_t key;
    tbl_vlan_t *p_db_vlan = NULL;
    vid_t vid = 0;

    if (HSRV_MAC_ADDR_TYPE_DYNAMIC == type || HSRV_MAC_ADDR_TYPE_UNICAST_ALL == type)
    {
        return HSRV_E_NONE;
    }

    for (vid = 0; vid < GLB_MAX_VLAN_ID; vid++)
    {
        key.vid = vid;
        p_db_vlan = tbl_vlan_get_vlan(&key);
        if (NULL == p_db_vlan)
        {
            continue;
        }

        hsrv_fdb_sw_get_l2mc_vlan(vid, type, mac, fp);
    }

    return HSRV_E_NONE;
}

/*added by yejl for bug 39605, add the cli of show mac-address-table add-fdb-fail*/
int32
hsrv_fdb_sw_get_add_fdb_fail(hsrv_mac_addr_type_t type, cfg_cmd_para_t *para)
{
#ifdef FDB_SLIST
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
#else
    tbl_fea_fdb_t *p_db_fdb_next = NULL;
#endif
    tbl_fea_fdb_t *p_db_fdb = NULL;
    hsrv_fdb_vlan_t *p_vlan = NULL;
    vid_t vid = 0;
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);

    hsrv_fdb_sw_show_banner(fp);

    if (HSRV_MAC_ADDR_TYPE_MULTICAST != type)
    {
        for (vid = GLB_DEF_VLAN_ID; vid < GLB_MAX_VLAN_ID; vid++)
        {
            p_vlan = hsrv_fdb_vlan_get(vid);
            if (NULL == p_vlan)
            {
                continue;
            }

#ifdef FDB_SLIST
            CTCLIB_SLIST_LOOP_DEL(p_vlan->fdb_list, p_db_fdb, listnode, next)
#else
            CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_vlan->fdb_queue, vlan_entry)
#endif
            {
                if (p_db_fdb->fea_fail)
                {
                    hsrv_fdb_show_one(p_db_fdb, type, fp);
                }
            }
        }
    }

    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);

    sal_fclose(fp);

    return rc;
}


int32
hsrv_fdb_sw_get_all(hsrv_mac_addr_type_t type, cfg_cmd_para_t *para)
{
#ifdef FDB_SLIST
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
#else
    tbl_fea_fdb_t *p_db_fdb_next = NULL;
#endif
    tbl_fea_fdb_t *p_db_fdb = NULL;
    hsrv_fdb_vlan_t *p_vlan = NULL;
    vid_t vid = 0;
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);

    hsrv_fdb_sw_show_banner(fp);

    if (HSRV_MAC_ADDR_TYPE_MULTICAST != type)
    {
        for (vid = GLB_DEF_VLAN_ID; vid < GLB_MAX_VLAN_ID; vid++)
        {
            p_vlan = hsrv_fdb_vlan_get(vid);
            if (NULL == p_vlan)
            {
                continue;
            }

#ifdef FDB_SLIST
            CTCLIB_SLIST_LOOP_DEL(p_vlan->fdb_list, p_db_fdb, listnode, next)
#else
            CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_vlan->fdb_queue, vlan_entry)
#endif
            {
                hsrv_fdb_show_one(p_db_fdb, type, fp);
            }
        }
    }

    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);

    hsrv_fdb_sw_get_l2mc(type, NULL, fp);

    sal_fclose(fp);

    return rc;
}

int32
hsrv_fdb_sw_get_errdisable_fdb_loop(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    tbl_brg_global_t *p_brg_glb = tbl_brg_global_get_brg_global();

    sal_fprintf(fp, "Errdisable FDB loop information\n");
    sal_fprintf(fp, "Bucket Max Size:      %u\n", p_brg_glb->fdb_loop_max_size);
    sal_fprintf(fp, "Bucket Token Rate:    %u\n", p_brg_glb->fdb_loop_add_rate);
    sal_fprintf(fp, "Current Token Count:  %u\n", p_brg_glb->fdb_loop_curr_size);

    sal_fclose(fp);
    return rc;
}


int32
hsrv_fdb_sw_get_interface(int32 ifindex, hsrv_mac_addr_type_t type, cfg_cmd_para_t *para)
{
#ifdef FDB_SLIST
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
#else
    tbl_fea_fdb_t *p_db_fdb_next = NULL;
#endif
    tbl_fea_fdb_t *p_db_fdb = NULL;
    tbl_fea_brg_if_t *p_db_if = NULL;
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    hsrv_fdb_sw_show_banner(fp);

    if (HSRV_MAC_ADDR_TYPE_MULTICAST != type)
    {
        /* get port fdb queue */
        p_db_if = hsrv_brgif_get(ifindex);
        if (p_db_if)
        {
            sal_mutex_lock(g_hsrv_fdb.fdb_mutex);
#ifdef FDB_SLIST
            CTCLIB_SLIST_LOOP_DEL(p_db_if->fdb_list, p_db_fdb, listnode, next)
#else
            CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_db_if->fdb_queue, port_entry)
#endif
            {
                hsrv_fdb_show_one(p_db_fdb, type, fp);
            }
            sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
        }
    }
    hsrv_fdb_sw_get_l2mc_interface(ifindex, type, fp);

    sal_fclose(fp);

    return rc;
}

int32
hsrv_fdb_sw_get_mac(uint8 *mac, hsrv_mac_addr_type_t type, cfg_cmd_para_t *para)
{
#ifdef FDB_SLIST
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
#else
    tbl_fea_fdb_t *p_db_fdb_next = NULL;
#endif
    tbl_fea_fdb_t *p_db_fdb = NULL;
    hsrv_fdb_vlan_t *p_vlan = NULL;
    vid_t vid = 0;
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);

    hsrv_fdb_sw_show_banner(fp);

    if (HSRV_MAC_ADDR_TYPE_MULTICAST != type)
    {
        for (vid = GLB_DEF_VLAN_ID; vid < GLB_MAX_VLAN_ID; vid++)
        {
            p_vlan = hsrv_fdb_vlan_get(vid);
            if (NULL == p_vlan)
            {
                continue;
            }

#ifdef FDB_SLIST
            CTCLIB_SLIST_LOOP_DEL(p_vlan->fdb_list, p_db_fdb, listnode, next)
#else
            CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_vlan->fdb_queue, vlan_entry)
#endif
            {
                if (0 == sal_memcmp(mac, p_db_fdb->key.mac, MAC_ADDR_LEN))
                {
                    hsrv_fdb_show_one(p_db_fdb, type, fp);
                }
            }
        }
    }

    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);

    hsrv_fdb_sw_get_l2mc(type, mac, fp);

    sal_fclose(fp);

    return rc;
}

int32
hsrv_fdb_sw_get_vlan(vid_t vid, hsrv_mac_addr_type_t type, cfg_cmd_para_t *para)
{
#ifdef FDB_SLIST
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
#else
    tbl_fea_fdb_t *p_db_fdb_next = NULL;
#endif
    tbl_fea_fdb_t *p_db_fdb = NULL;
    hsrv_fdb_vlan_t *p_vlan = NULL;
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    hsrv_fdb_sw_show_banner(fp);

    if (HSRV_MAC_ADDR_TYPE_MULTICAST != type)
    {
        p_vlan = hsrv_fdb_vlan_get(vid);
        if (p_vlan)
        {
            sal_mutex_lock(g_hsrv_fdb.fdb_mutex);
#ifdef FDB_SLIST
            CTCLIB_SLIST_LOOP_DEL(p_vlan->fdb_list, p_db_fdb, listnode, next)
#else
            CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_vlan->fdb_queue, vlan_entry)
#endif
            {
                hsrv_fdb_show_one(p_db_fdb, type, fp);
            }
            sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
        }
    }

    hsrv_fdb_sw_get_l2mc_vlan(vid, type, NULL, fp);

    sal_fclose(fp);

    return rc;
}

#define __HSRV_FDB_DB_FLUSH__

int32
hsrv_fdb_db_flush_mlag_mi()
{
#ifdef FDB_SLIST
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
#else
    tbl_fea_fdb_t *p_db_fdb_next = NULL;
#endif
    tbl_fea_fdb_t *p_db_fdb = NULL;
    hsrv_fdb_vlan_t *p_vlan = NULL;
    vid_t vid = 0;

    sal_mutex_lock(g_hsrv_brgif_mutex);
    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);
    for (vid = GLB_DEF_VLAN_ID; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_vlan = hsrv_fdb_vlan_get(vid);
        if (NULL == p_vlan)
        {
            continue;
        }

#ifdef FDB_SLIST
        CTCLIB_SLIST_LOOP_DEL(p_vlan->fdb_list, p_db_fdb, listnode, next)
#else
        CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_vlan->fdb_queue, vlan_entry)
#endif
        {
            if (!GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC))
            {
                if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_IN))
                {
                    _hsrv_fdb_del_fdb(p_db_fdb, FALSE, FALSE);
                }
            }
        }
    }

    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    hsrv_fdb_mlag_tx_flush_all();

    return HSRV_E_NONE;
}

/* modified by liwh for bug 53553, 2019-10-23 */
void 
hsrv_fdb_lock()
{
    sal_mutex_lock(g_hsrv_brgif_mutex);
    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);
    

    return;
}

void 
hsrv_fdb_unlock()
{
    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    sal_mutex_unlock(g_hsrv_brgif_mutex);

    return;
}
/* liwh end */

int32
hsrv_fdb_db_flush_all()
{
#ifdef FDB_SLIST
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
#else
    tbl_fea_fdb_t *p_db_fdb_next = NULL;
#endif
    tbl_fea_fdb_t *p_db_fdb = NULL;
    hsrv_fdb_vlan_t *p_vlan = NULL;
    vid_t vid = 0;

    /* modified by liwh for bug 53553, 2019-10-23 */
    //sal_mutex_lock(g_hsrv_brgif_mutex);
    //sal_mutex_lock(g_hsrv_fdb.fdb_mutex);
    /* liwh end */
    
    for (vid = GLB_DEF_VLAN_ID; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_vlan = hsrv_fdb_vlan_get(vid);
        if (NULL == p_vlan)
        {
            continue;
        }

#ifdef FDB_SLIST
        CTCLIB_SLIST_LOOP_DEL(p_vlan->fdb_list, p_db_fdb, listnode, next)
#else
        CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_vlan->fdb_queue, vlan_entry)
#endif
        {
            if (!GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC))
            {
                hsrv_fdb_del_fdb_one_by_flush(p_db_fdb);
                hsrv_fdb_db_del_fdb(p_db_fdb);
            }
        }
    }

    /* modified by liwh for bug 53553, 2019-10-23 */
    //sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    //sal_mutex_unlock(g_hsrv_brgif_mutex);
    
    //hsrv_fdb_mlag_tx_flush_all();
    /* liwh end */

    return HSRV_E_NONE;
}

/* modified by liwh for bug 53553, 2019-10-23 */
int32
//hsrv_fdb_db_flush_vlan(vid_t vid)
hsrv_fdb_db_flush_vlan(vid_t vid, uint32 *p_has_entry)
/* liwh end */
{
#ifdef FDB_SLIST
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
#else
    tbl_fea_fdb_t *p_db_fdb_next = NULL;
#endif
    tbl_fea_fdb_t *p_db_fdb = NULL;
    hsrv_fdb_vlan_t *p_vlan = NULL;
    uint32 has_entry = FALSE;

    /* modified by liwh for bug 53553, 2019-10-23 */
    //sal_mutex_lock(g_hsrv_brgif_mutex);
    //sal_mutex_lock(g_hsrv_fdb.fdb_mutex);
    /* liwh end */
    
    p_vlan = hsrv_fdb_vlan_get(vid);
    if (p_vlan)
    {
#ifdef FDB_SLIST
        CTCLIB_SLIST_LOOP_DEL(p_vlan->fdb_list, p_db_fdb, listnode, next)
#else
        CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_vlan->fdb_queue, vlan_entry)
#endif
        {
            if (!GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC))
            {
                has_entry = TRUE;
                hsrv_fdb_del_fdb_one_by_flush(p_db_fdb);
                hsrv_fdb_db_del_fdb(p_db_fdb);
            }
        }
    }

    /* modified by liwh for bug 53553, 2019-10-23 */
    //sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    //sal_mutex_unlock(g_hsrv_brgif_mutex); 

    /*if (has_entry)
    {
        hsrv_fdb_mlag_tx_flush_vlan(vid);
    }*/

    *p_has_entry = has_entry;
    /* liwh end */

    return HSRV_E_NONE;
}

/* modified by liwh for bug 53553, 2019-10-23 */
int32
//hsrv_fdb_db_flush_port(uint32 ifindex, uint32 mlag_sync)
hsrv_fdb_db_flush_port(uint32 ifindex, uint32 mlag_sync, uint32 *p_has_entry)
/* liwh end */
{
#ifdef FDB_SLIST
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
#else
    tbl_fea_fdb_t *p_db_fdb_next = NULL;
#endif
    tbl_fea_fdb_t *p_db_fdb = NULL;
    tbl_fea_brg_if_t *p_db_if = NULL;
    uint32 has_entry = FALSE;

    /* modified by liwh for bug 53553, 2019-10-23 */
    //sal_mutex_lock(g_hsrv_brgif_mutex);
    //sal_mutex_lock(g_hsrv_fdb.fdb_mutex);
    /* liwh end */
    
    p_db_if = hsrv_brgif_get(ifindex);
    if (p_db_if)
    {
#ifdef FDB_SLIST
        CTCLIB_SLIST_LOOP_DEL(p_db_if->fdb_list, p_db_fdb, listnode, next)
#else
        CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_db_if->fdb_queue, port_entry)
#endif
        {
            if (!GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC))
            {
                has_entry = TRUE;
                /* Added by kcao 2017-01-13 for bug 41837, is del FDB on MLAG peer-link, need sync to peer */
                if (GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_IN))
                {
                    /* for only MO could sync to peer, change MI to MO temply */
                    GLB_UNSET_FLAG(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_IN);
                    GLB_SET_FLAG(p_db_fdb->flags, GLB_FDB_FLAG_MLAG_SYNC_OUT);
                    hsrv_fdb_mlag_tx_fdb(p_db_fdb, GLB_MLAG_FDB_SYNC_DEL);
                }
                _hsrv_fdb_del_fdb(p_db_fdb, FALSE, FALSE);
            }
        }
    }

    /* modified by liwh for bug 53553, 2019-10-23 */
    //sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    //sal_mutex_unlock(g_hsrv_brgif_mutex);

    /* if (has_entry && mlag_sync)
    {
        hsrv_fdb_mlag_tx_flush_port(ifindex);
    } */

    *p_has_entry = has_entry;
    /* liwh end */

    return HSRV_E_NONE;
}

/* Added by yejl for bug 39502, flush the fdb in DB by port */
int32
hsrv_fdb_db_flush_by_port(uint32 ifindex)
{
    ctclib_hash_backet_t *hb_next = NULL;
    ctclib_hash_backet_t *pst_hb = NULL;
    tbl_fea_fdb_master_t* p_master = tbl_fea_fdb_get_master();
    tbl_fea_fdb_t *p_db_fdb = NULL;
    uint32 has_entry = FALSE;
    int32 index = 0;

    sal_mutex_lock(g_hsrv_brgif_mutex);
    sal_mutex_lock(g_hsrv_fdb.fdb_mutex);

    for (index = 0; index < p_master->fdb_hash->size; index++)
    {
        for (pst_hb = p_master->fdb_hash->index[index]; pst_hb; pst_hb = hb_next)
        {
            hb_next = pst_hb->next;
            if (!pst_hb->data)
            {
                continue;
            }
            p_db_fdb = (tbl_fea_fdb_t*)pst_hb->data;
            if (p_db_fdb->ifindex == ifindex)
            {
                if (!GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC))
                {
                    has_entry = TRUE;
                    hsrv_fdb_del_fdb_one_by_flush(p_db_fdb);
                    hsrv_fdb_db_del_fdb(p_db_fdb);
                }
            }
        }
    }

    sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    sal_mutex_unlock(g_hsrv_brgif_mutex);
    
    if (has_entry)
    {
        hsrv_fdb_mlag_tx_flush_port(ifindex);
    }

    return HSRV_E_NONE;
}

/* modified by liwh for bug 53553, 2019-10-23 */
int32
//hsrv_fdb_db_flush_port_vlan(uint32 ifindex, vid_t vid)
hsrv_fdb_db_flush_port_vlan(uint32 ifindex, vid_t vid, uint32 *p_has_entry)
/* liwh end */
{
#ifdef FDB_SLIST
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
#else
    tbl_fea_fdb_t *p_db_fdb_next = NULL;
#endif
    tbl_fea_fdb_t *p_db_fdb = NULL;
    hsrv_fdb_vlan_t *p_vlan = NULL;
    uint32 has_entry = FALSE;

    /* modified by liwh for bug 53553, 2019-10-23 */
    //sal_mutex_lock(g_hsrv_brgif_mutex);
    //sal_mutex_lock(g_hsrv_fdb.fdb_mutex);
    /* liwh end */
    p_vlan = hsrv_fdb_vlan_get(vid);
    if (p_vlan)
    {
#ifdef FDB_SLIST
        CTCLIB_SLIST_LOOP_DEL(p_vlan->fdb_list, p_db_fdb, listnode, next)
#else
        CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_vlan->fdb_queue, vlan_entry)
#endif
        {
            if (p_db_fdb->ifindex == ifindex)
            {
                if (!GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC))
                {
                    has_entry = TRUE;
                    hsrv_fdb_del_fdb_one_by_flush(p_db_fdb);
                    hsrv_fdb_db_del_fdb(p_db_fdb);
                }
            }
        }
    }

    /* modified by liwh for bug 53553, 2019-10-23 */
    //sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    //sal_mutex_unlock(g_hsrv_brgif_mutex);

    /* if (has_entry)
    {
        hsrv_fdb_mlag_tx_flush_port_vlan(ifindex, vid);
    }*/

    *p_has_entry = has_entry;
    /* liwh end */

    return HSRV_E_NONE;
}

/* modified by liwh for bug 53553, 2019-10-23 */
int32
//hsrv_fdb_db_flush_mac(uint8 *mac)
hsrv_fdb_db_flush_mac(uint8 *mac, uint32 *p_has_entry)
/* liwh end */
{
#ifdef FDB_SLIST
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
#else
    tbl_fea_fdb_t *p_db_fdb_next = NULL;
#endif
    tbl_fea_fdb_t *p_db_fdb = NULL;
    hsrv_fdb_vlan_t *p_vlan = NULL;
    vid_t vid = 0;
    uint32 has_entry = FALSE;

    /* modified by liwh for bug 53553, 2019-10-23 */
    //sal_mutex_lock(g_hsrv_brgif_mutex);
    //sal_mutex_lock(g_hsrv_fdb.fdb_mutex);
    /* liwh end */
    
    for (vid = GLB_DEF_VLAN_ID; vid < GLB_MAX_VLAN_ID; vid++)
    {
        p_vlan = hsrv_fdb_vlan_get(vid);
        if (NULL == p_vlan)
        {
            continue;
        }

#ifdef FDB_SLIST
        CTCLIB_SLIST_LOOP_DEL(p_vlan->fdb_list, p_db_fdb, listnode, next)
#else
        CTCLIB_TAILQ_FOREACH_DEL(p_db_fdb, p_db_fdb_next, &p_vlan->fdb_queue, vlan_entry)
#endif
        {
            if (0 == sal_memcmp(mac, p_db_fdb->key.mac, MAC_ADDR_LEN))
            {
                if (!GLB_FLAG_ISSET(p_db_fdb->flags, GLB_FDB_FLAG_STATIC))
                {
                    has_entry = TRUE;
                    hsrv_fdb_del_fdb_one_by_flush(p_db_fdb);
                    hsrv_fdb_db_del_fdb(p_db_fdb);
                }
            }
        }
    }

    /* modified by liwh for bug 53553, 2019-10-23 */
    //sal_mutex_unlock(g_hsrv_fdb.fdb_mutex);
    //sal_mutex_unlock(g_hsrv_brgif_mutex);
    
    /*if (has_entry)
    {
        hsrv_fdb_mlag_tx_flush_mac(mac);
    }*/

    *p_has_entry = has_entry;
    /* liwh end */
    
    return HSRV_E_NONE;
}

#define __HSRV_FDB_DB_CMD__

int32
hsrv_fdb_db_cmd_process_fdb(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    hsrv_mac_addr_type_t type = HSRV_MAC_ADDR_TYPE_ALL;
    vid_t vid = 0;
    int32 ifindex = 0;
    mac_addr_t mac;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            CFG_INVALID_PARAM_RET("Need show type parameter");
        }
        else if (1 <= argc)
        {
            if (0 == sal_strcmp(argv[0], "count"))
            {
                rc = hsrv_fdb_sw_get_count(para);
                return rc;
            }
            if (0 == sal_strcmp(argv[0], "add-fdb-fail"))
            {
                rc = hsrv_fdb_sw_get_add_fdb_fail(HSRV_MAC_ADDR_TYPE_ALL, para);
                return rc;
            }

            if (0 == sal_strcmp(argv[0], "static"))
            {
                type = HSRV_MAC_ADDR_TYPE_STATIC;
            }
            else if (0 == sal_strcmp(argv[0], "dynamic"))
            {
                type = HSRV_MAC_ADDR_TYPE_DYNAMIC;
            }
            else if (0 == sal_strcmp(argv[0], "multicast"))
            {
                type = HSRV_MAC_ADDR_TYPE_MULTICAST;
            }
            else if (0 == sal_strcmp(argv[0], "all"))
            {
                type = HSRV_MAC_ADDR_TYPE_ALL;
            }
            else if (0 == sal_strcmp(argv[0], "fdb-loop"))
            {
                rc = hsrv_fdb_sw_get_errdisable_fdb_loop(para);
                return rc;
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid show type %s", argv[0]);
            }

            if (1 == argc)
            {
                rc = hsrv_fdb_sw_get_all(type, para);
            }
            else if (3 == argc)
            {
                if (0 == sal_strcmp(argv[1], "interface"))
                {
                    ifindex = tbl_interface_get_ifindex_by_name(argv[2]);
                    if (ifindex < 0)
                    {
                        CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[2]);
                    }
                    rc = hsrv_fdb_sw_get_interface(ifindex, type, para);
                }
                else if (0 == sal_strcmp(argv[1], "vlan"))
                {
                    PM_CFG_GET_UINT(vid, argv[2], para->p_rs);
                    rc = hsrv_fdb_sw_get_vlan(vid, type, para);
                }
                else if (0 == sal_strcmp(argv[1], "address"))
                {
                    sal_memset(mac, 0, sizeof(mac));
                    rc = cdb_mac_str2val(mac, argv[2]);
                    if (rc < 0)
                    {
                        CFG_INVALID_PARAM_RET("Unable to translate MAC address %s", argv[2]);
                    }
                    rc = hsrv_fdb_sw_get_mac(mac, type, para);
                }
                else
                {
                    CFG_INVALID_PARAM_RET("Invalid type %s", argv[1]);
                }
            }
            else
            {
                CFG_INVALID_PARAM_RET("The parameter number %u is not invalid", argc);
            }
        }

        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
_hsrv_fea_fdb_db_cmd_get_fdb(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_fea_fdb_iterate((TBL_ITER_CB_FUNC)tbl_fea_fdb_dump_one, &args);

    sal_fclose(fp);

    return PM_E_NONE;
}

int32
hsrv_fdb_db_cmd_process_fea_fdb(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    cdb_node_t* p_node = NULL;
    tbl_fdb_t fdb;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(para->p_msg->u_hdr.cfg.id);

    sal_memset(&fdb, 0, sizeof(fdb));

    switch (para->oper)
    {
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        if (TBL_FEA_FDB == p_node->id)
        {
            rc = _hsrv_fea_fdb_db_cmd_get_fdb(para, p_node, &field);
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

static int32
hsrv_fdb_learning_en_init()
{
    g_hsrv_fdb.hsrv_learn_enable = TRUE;
    p_learn_enable_timer = CTC_TASK_ADD_TIME(_hsrv_fdb_set_learning_en_timer_handle, NULL, HSRV_FDB_LEARNING_EN_TIME);
    return HSRV_E_NONE;
}

int32
hsrv_fdb_db_init()
{
    hsrv_fdb_learning_en_init();
    sal_mutex_create(&g_hsrv_fdb.fdb_mutex);
    sal_mutex_create(&g_hsrv_fdb.mlag_sync_mutex);
    cdb_register_cfg_tbl_cb(TBL_FEA_FDB, hsrv_fdb_db_cmd_process_fea_fdb);
    cdb_register_cfg_act_cb(ACT_SHOW_FDB, hsrv_fdb_db_cmd_process_fdb);

    return HSRV_E_NONE;
}

