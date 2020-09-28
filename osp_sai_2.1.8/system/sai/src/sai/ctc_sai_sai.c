#include <sai.h>
#include <sal.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_fdb.h>
#include <ctc_sai_vlan.h>
#include <ctc_sai_switch.h>
#include <ctc_sai_port.h>
#include <ctc_sai_neighbor.h>
#include <ctc_sai_routerintf.h>
#include <ctc_sai_hostif.h>
#include <ctc_sai_router.h>
#include <ctc_sai_route.h>
#include <ctc_sai_raps_mc_group.h>
#include <ctc_sai_lag.h>
#include <ctc_sai_stp.h>
#include <ctc_sai_nexthop.h>
#include <ctc_sai_nexthop_group.h>
#include <ctc_sai_acl.h>
#include <ctc_sai_mirror.h>
#ifdef USW
#include <ctc_sai_d2_qosmaps.h>
#else
#include <ctc_sai_qosmaps.h>
#endif
#include <ctc_sai_policer.h>
#include <ctc_sai_queue.h>
#include <ctc_sai_wred.h>
#include <ctc_sai_scheduler.h>
#include <ctc_sai_hash.h>
#include <ctc_sai_isolation.h>
#include <ctc_sai_l2mc.h>
#include <ctc_sai_ipmc.h>
#include <ctc_sai_openflow.h>
#include <ctc_sai_vlanclass.h>
#include <ctc_sai_macescape.h>
#ifdef TAPPRODUCT
#include <ctc_sai_tap_group.h>
#endif
#include <ctc_packet.h>
#include <ctc_sai_ptp.h>
#include <ctc_sai_copp.h>
#include <ctc_sai_udf.h>
#include <ctc_sai_vrrp.h>
#include <ctc_sai_vlanmap.h>
#include <ctc_sai_nat.h>
#include <ctc_sai_cfm.h>
#include "genlog.h"
#include "sal_memmngr.h"

extern sal_err_t sal_init(void);

uint32 g_sai_debug_flag = FALSE;
uint32 g_ctc_debug_flag = FALSE;

/*file for cnos_config*/
#define GLB_CNOS_CONFIG_FILE   "/tmp/cnos_config_file"

ctc_sai_sai_sys_info_t g_sai_sys_info = {
    .api_queue          = NULL,
    .sai_init_status    = UNINITIALIZED,
    .sai_switch_notification_table = {NULL},
};

#define ________SAI_SAI_SAI_INNER_API_FUNC
static int32_t
__sai_list_cmp_cb_t(
void* val1,
void* val2)
{
    return ((ctc_sai_api_reg_info_t*)val1)->id == ((ctc_sai_api_reg_info_t*)val2)->id ? 1 : 0;
}

#define ________SAI_SAI_SAI_OUTER_API_FUNC
ctc_sai_sai_sys_info_t*
ctc_sai_get_sys_info()
{
    return &g_sai_sys_info;
}

sai_status_t
api_reg_register_fn(
    _In_  ctc_sai_api_reg_info_t    *preg)
{
    if(ctc_listnode_lookup(g_sai_sys_info.api_queue,preg))
    {
        return SAI_STATUS_SUCCESS;
    }

    ctc_listnode_add(g_sai_sys_info.api_queue, preg);
    return SAI_STATUS_SUCCESS;
}

sai_status_t
api_reg_unregister_fn(
    _In_  ctc_sai_api_reg_info_t    *preg)
{
    if(ctc_listnode_lookup(g_sai_sys_info.api_queue,preg))
    {
        ctc_listnode_delete(g_sai_sys_info.api_queue, preg);
        return SAI_STATUS_SUCCESS;
    }

    return SAI_STATUS_ITEM_NOT_FOUND;
}

sai_status_t
api_reg_traverse(
    _In_  api_reg_traverse_fn   fn,
    _In_  void                  *data)
{
    sai_status_t            ret     = SAI_STATUS_SUCCESS;
    ctc_listnode_t          *pnode  = NULL;
    ctc_sai_api_reg_info_t  *pvalue = NULL;

    if(NULL == fn)
    {
        return SAI_STATUS_FAILURE;
    }

    CTC_LIST_LOOP(g_sai_sys_info.api_queue,pvalue,pnode)
    {
        if(ret = fn(pvalue,data),ret != SAI_STATUS_SUCCESS)
        {
            log_sys(M_MOD_SAI, E_ERROR, "Init SAI module %d failed, ret = %d", pvalue->id, ret);
            return ret;
        }
    }

    return ret;
}

sai_status_t
__ctc_sai_cnos_config_profile_set(sal_file_t fp)
{
#ifdef GOLDENGATE
    sal_fprintf(fp, "config_tap_timestamp = 1\n");
    sal_fprintf(fp, "config_tap_trunction = 1\n");
	sal_fprintf(fp, "config_tap_trunction_length_max = 144\n");
    sal_fprintf(fp, "config_tap_untag     = 1\n");

    sal_fprintf(fp, "config_acl_gre        = 1\n");
    sal_fprintf(fp, "config_acl_nvgre      = 1\n");
    sal_fprintf(fp, "config_acl_flow_match = 1\n");

    sal_fprintf(fp, "config_hybrid_force_back = 1\n");
    sal_fprintf(fp, "config_hybrid_mpls = 1\n");
    sal_fprintf(fp, "config_hybrid_tunnel = 1\n");
    sal_fprintf(fp, "config_hybrid_arp_edit = 1\n");
    sal_fprintf(fp, "config_hybrid_set_qinq = 1\n");
    sal_fprintf(fp, "config_hybrid_need_mpls_parser_cli = 0\n");
    sal_fprintf(fp, "config_hybrid_some_field = 1\n");
    sal_fprintf(fp, "config_acl_vxlan     = 1\n");
    sal_fprintf(fp, "config_acl_edit_src_ip  = 1\n");
    sal_fprintf(fp, "config_acl_strip_header  = 1\n"); 
    sal_fprintf(fp, "config_parser_linkagg_inner_hash  = 1\n");
    sal_fprintf(fp, "config_select_hash_key_not_only_ip = 1\n");

    sal_fprintf(fp, "config_acl_udf = 1\n");
    sal_fprintf(fp, "config_tap_acl_tcam_res_share = 1\n");
    sal_fprintf(fp, "config_hybrid_mpls_tp_oam = 1\n");
    sal_fprintf(fp, "config_hybrid_vlan_edit_before_group = 1\n");
    sal_fprintf(fp, "config_hybrid_v4_v6_exchange = 1\n");
    sal_fprintf(fp, "config_hybrid_ipv6_edit = 1\n");
    sal_fprintf(fp, "config_hybrid_pop_all_mpls = 1\n");
#elif USW
    sal_fprintf(fp, "config_tap_timestamp = 1\n");
    sal_fprintf(fp, "config_tap_trunction = 1\n");
    sal_fprintf(fp, "config_tap_untag     = 1\n");

    sal_fprintf(fp, "config_acl_gre        = 1\n");
    sal_fprintf(fp, "config_acl_nvgre      = 1\n");
    sal_fprintf(fp, "config_acl_flow_match = 1\n");

    sal_fprintf(fp, "config_hybrid_force_back = 1\n");
    sal_fprintf(fp, "config_hybrid_mpls = 1\n");
    sal_fprintf(fp, "config_hybrid_tunnel = 1\n");
    sal_fprintf(fp, "config_hybrid_arp_edit = 1\n");
    sal_fprintf(fp, "config_hybrid_set_qinq = 1\n");
    sal_fprintf(fp, "config_hybrid_need_mpls_parser_cli = 0\n");
    sal_fprintf(fp, "config_hybrid_some_field = 1\n");
    sal_fprintf(fp, "config_acl_vxlan     = 1\n");
    sal_fprintf(fp, "config_acl_edit_src_ip  = 1\n");
    sal_fprintf(fp, "config_acl_strip_header  = 1\n"); 
    sal_fprintf(fp, "config_parser_linkagg_inner_hash  = 1\n");
    sal_fprintf(fp, "config_select_hash_key_not_only_ip = 1\n");

    sal_fprintf(fp, "config_acl_udf = 1\n");
    sal_fprintf(fp, "config_tap_acl_tcam_res_share = 1\n");
    sal_fprintf(fp, "config_hybrid_mpls_tp_oam = 1\n");
    sal_fprintf(fp, "config_hybrid_vlan_edit_before_group = 0\n");
    sal_fprintf(fp, "config_hybrid_v4_v6_exchange = 1\n");
    sal_fprintf(fp, "config_hybrid_ipv6_edit = 1\n");
    sal_fprintf(fp, "config_hybrid_pop_all_mpls = 1\n");
#elif GREATBELT
    sal_fprintf(fp, "config_tap_timestamp = 0\n");
    sal_fprintf(fp, "config_tap_trunction = 0\n");
    sal_fprintf(fp, "config_tap_untag     = 0\n");
    sal_fprintf(fp, "config_acl_vxlan     = 0\n");
    sal_fprintf(fp, "config_acl_edit_src_ip  = 0\n");
    sal_fprintf(fp, "config_acl_strip_header  = 0\n");
    sal_fprintf(fp, "config_parser_linkagg_inner_hash  = 0\n");

    sal_fprintf(fp, "config_hybrid_force_back = 0\n");
    sal_fprintf(fp, "config_hybrid_mpls = 1\n");
    sal_fprintf(fp, "config_hybrid_tunnel = 0\n");
    sal_fprintf(fp, "config_hybrid_arp_edit = 0\n");
    sal_fprintf(fp, "config_hybrid_set_qinq = 0\n");
    sal_fprintf(fp, "config_hybrid_need_mpls_parser_cli = 1\n");
    sal_fprintf(fp, "config_hybrid_some_field = 0\n");
    sal_fprintf(fp, "config_select_hash_key_not_only_ip = 0\n");

    sal_fprintf(fp, "config_acl_udf = 0\n");

    sal_fprintf(fp, "config_tap_acl_tcam_res_share = 0\n");
    sal_fprintf(fp, "config_hybrid_mpls_tp_oam = 0\n");
    sal_fprintf(fp, "config_hybrid_vlan_edit_before_group = 0\n");
    sal_fprintf(fp, "config_hybrid_v4_v6_exchange = 0\n");
    sal_fprintf(fp, "config_hybrid_ipv6_edit = 0\n");
    sal_fprintf(fp, "config_hybrid_pop_all_mpls = 0\n");
#endif
    
    return SAI_STATUS_SUCCESS;
}

sai_status_t
__ctc_sai_cnos_config_profile()
{
    sal_file_t fp;

    fp = sal_fopen(GLB_CNOS_CONFIG_FILE, "w+");
    if (fp == NULL)
    {
        return SAI_STATUS_FAILURE;
    }
    
    __ctc_sai_cnos_config_profile_set(fp);

    sal_fclose(fp);
    
    return SAI_STATUS_SUCCESS;
}

#define ________SAI_SAI_SAI_API_FUNC
/*
* Routine Description:
*     Adapter module initialization call. This is NOT for SDK initialization.
*
* Arguments:
*     [in] flags - reserved for future use, must be zero
*     [in] services - methods table with services provided by adapter host
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
sai_api_initialize(
    _In_ uint64_t flags,
    _In_ const service_method_table_t* services)
{
    /* init mem module */
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ret = mem_mgr_init();
    if (ret != 0)
    {
        return ret;
    }
    ret = ctc_debug_init();
    if (ret != 0)
    {
        return ret;
    }

    ret = sal_init();
    if (ret != 0)
    {
        return ret;
    }
    
    g_sai_sys_info.api_queue = ctc_list_create(__sai_list_cmp_cb_t,NULL);

    if(NULL == g_sai_sys_info.api_queue)
    {
        return SAI_STATUS_NO_MEMORY;
    }

    /* initialize sai mode */
    ctclib_opb_init();
    ctc_sai_switch_init();
    ctc_sai_stp_init();
    ctc_sai_fdb_init();
    ctc_sai_vlan_init();
    ctc_sai_vlanmap_init();
    ctc_sai_port_init();
    ctc_sai_vr_init();
    ctc_sai_routerintf_init();
    ctc_sai_neighbor_init();
    ctc_sai_nexthop_init();
    ctc_sai_nexthop_group_init();
    ctc_sai_route_init();
    ctc_sai_aps_group_init();
    ctc_sai_lag_init();
    ctc_sai_mirror_init();
    ctc_sai_acl_init();
    ctc_sai_hostinterface_init();
    ctc_sai_queue_init();
    ctc_sai_qosmaps_init();
    ctc_sai_policer_init();
    ctc_sai_wred_init();
    ctc_sai_scheduler_init();
    ctc_sai_hash_init();
    ctc_sai_isolation_init();
    ctc_sai_l2mc_init();
    ctc_sai_ipmc_init();
    ctc_sai_openflow_init();
    ctc_sai_vlanclass_init();
    ctc_sai_macescape_init();
#ifdef TAPPRODUCT    
    ctc_sai_tap_group_init();
#endif
    ctc_sai_ptp_init();
    ctc_sai_copp_init();
    ctc_sai_udf_init();

    ctc_sai_vrrp_init();

    ctc_sai_nat_init();/* added by taocy for NAT, 2019-06-03*/
    ctc_sai_cfm_init(); /* modified by liwh for bug 53010, 2019-07-22 */
    
    g_sai_sys_info.sai_init_status = INITIALIZED;

    __ctc_sai_cnos_config_profile();


    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*   Uninitialization of the adapter module. SAI functionalities, retrieved via
*   sai_api_query() cannot be used after this call.
*
* Arguments:
*   None
*
* Return Values:
*   SAI_STATUS_SUCCESS on success
*   Failure status code on error
*/
sai_status_t
sai_api_unitialize(void)
{
    g_sai_sys_info.sai_init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*     Set log level for sai api module. The default log level is SAI_LOG_WARN.
*
* Arguments:
*     [in] sai_api_id - SAI api ID
*     [in] log_level - log level
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
sai_log_set(
    _In_ sai_api_t sai_api_id,
    _In_ sai_log_level_t log_level)
{
    ctc_sai_get_sys_info()->log_level[sai_api_id] = log_level;
    return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*     Query sai object type.
*
* Arguments:
*     [in] sai_object_id_t
*
* Return Values:
*    Return SAI_OBJECT_TYPE_NULL when sai_object_id is not valid.
*    Otherwise, return a valid sai object type SAI_OBJECT_TYPE_XXX
*/
sai_object_type_t
sai_object_type_query(
    _In_ sai_object_id_t sai_object_id)
{
    return CTC_SAI_OBJECT_TYPE_GET(sai_object_id);
}

/*
* Routine Description:
*     Retrieve a pointer to the C-style method table for desired SAI
*     functionality as specified by the given sai_api_id.
*
* Arguments:
*     [in] sai_api_id - SAI api ID
*     [out] api_method_table - Caller allocated method table
*           The table must remain valid until the sai_api_shutdown() is called
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t
sai_api_query(
    _In_ sai_api_t sai_api_id,
    _Out_ void** api_method_table)
{
    ctc_listnode_t     *pnode = NULL;
    ctc_sai_api_reg_info_t *pvalue= NULL;

    if(UNINITIALIZED == g_sai_sys_info.sai_init_status)
    {
        return SAI_STATUS_UNINITIALIZED;
    }

    CTC_LIST_LOOP(g_sai_sys_info.api_queue,pvalue,pnode)
    {
        if(sai_api_id == pvalue->id)
        {
            if(UNINITIALIZED == pvalue->init_status)
            {
                return SAI_STATUS_UNINITIALIZED;
            }
            *api_method_table = pvalue->api_method_table;
            return SAI_STATUS_SUCCESS;
        }
    }
    return SAI_STATUS_ITEM_NOT_FOUND;
}
