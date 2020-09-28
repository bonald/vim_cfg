
#ifdef HAVE_NETCONF

#include "proto.h"
#include "ncutil.h"
#include "ncs.h"
#include "ncs_api.h"
#include "ncs_yang.h"

extern sr_subscription_ctx_t *sr_ttp_subscription;

static void 
ncs_module_install_cb(const char *module_name, const char *revision, sr_module_state_t state,
        void *private_ctx);

static void 
ncs_hwvtep_ttp_subscribe_init();

static void 
ncs_hwvtep_ttp_unsubscribe_init();

void
ncs_nc_reinit(void *th)
{
    static int last_warn = 0;
    if (ncs_nc_init() != 0) {
        /* reconnect failed */
        time_t now = time(NULL);
        if (now > last_warn) {
            last_warn = now + 60;
            log_sys(M_MOD_INTERNAL, E_CRITICAL, "ncs_nc_reinit failed");
        }
        ctc_task_add_timer(TASK_PRI_NORMAL, ncs_nc_reinit, NULL, 1);
        return;
    }
    return;
}

void
ncs_nc_reinit_cb()
{
    ctc_task_add_timer(TASK_PRI_NORMAL, ncs_nc_reinit, NULL, 1);
    return;
}

static void 
ncs_hwvtep_ttp_subscribe_init()
{
    int32 rc = PM_E_NONE;

    /* subscribe module, accpect all changes, becase sysrepo support roll back if error */
    rc = ctc_nc_module_change_subscribe(NULL, "hwvtep-ttp",
        ncs_hwvtep_ttp_module_change_cb, NULL, 0, SR_SUBSCR_DEFAULT, &sr_ttp_subscription);
    if (SR_ERR_OK != rc) {
       log_sys(M_MOD_INTERNAL, E_ERROR, "ncs_nc_init subscribe to module hwvtep-ttp failed");
    }

    /*subscribe switch list */
    rc = ctc_nc_subtree_change_subscribe(NULL, "/hwvtep-ttp:capable-switch/logical-switches/switch",
        ncs_hwvtep_ttp_switch_list_change_cb, NULL, 0, SR_SUBSCR_CTX_REUSE, &sr_ttp_subscription);
    if (SR_ERR_OK != rc) {
       log_sys(M_MOD_INTERNAL, E_ERROR, "ncs_nc_init subscribe to switch list failed");
    }

    /*subscribe controller list */
    rc = ctc_nc_subtree_change_subscribe(NULL, "/hwvtep-ttp:capable-switch/logical-switches/switch/controllers/controller",
        ncs_hwvtep_ttp_controller_list_change_cb, NULL, 0, SR_SUBSCR_CTX_REUSE, &sr_ttp_subscription);
    if (SR_ERR_OK != rc) {
       log_sys(M_MOD_INTERNAL, E_ERROR, "ncs_nc_init subscribe to controller list failed");
    }

    /* controller provider */
    rc = ctc_nc_dp_get_items_subscribe(NULL, "/hwvtep-ttp:capable-switch/logical-switches/switch/controllers/controller/state",
        ncs_hwvtep_ttp_controller_provider_cb, NULL, SR_SUBSCR_CTX_REUSE, &sr_ttp_subscription);
    if (SR_ERR_OK != rc) {
       log_sys(M_MOD_INTERNAL, E_ERROR, "ncs_nc_init subscribe to controller provider failed");
    }

    /* subscribe interface list */
    rc = ctc_nc_subtree_change_subscribe(NULL, "/hwvtep-ttp:capable-switch/resources/port",
        ncs_hwvtep_ttp_interface_list_change_cb, NULL, 0, SR_SUBSCR_CTX_REUSE, &sr_ttp_subscription);
    if (SR_ERR_OK != rc) {
       log_sys(M_MOD_INTERNAL, E_ERROR, "ncs_nc_init subscribe to interface resource list failed");
    }

    /* interface provider */
    rc = ctc_nc_dp_get_items_subscribe(NULL, "/hwvtep-ttp:capable-switch/resources/port",
        ncs_hwvtep_ttp_interface_provider_cb, NULL, SR_SUBSCR_CTX_REUSE, &sr_ttp_subscription);
    if (SR_ERR_OK != rc) {
       log_sys(M_MOD_INTERNAL, E_ERROR, "ncs_nc_init subscribe to interface provider failed");
    }

    /* subscribe switch reference interface list */
    rc = ctc_nc_subtree_change_subscribe(NULL, "/hwvtep-ttp:capable-switch/logical-switches/switch/resources/port",
        ncs_hwvtep_ttp_interface_ref_list_change_cb, NULL, 0, SR_SUBSCR_CTX_REUSE, &sr_ttp_subscription);
    if (SR_ERR_OK != rc) {
       log_sys(M_MOD_INTERNAL, E_ERROR, "ncs_nc_init subscribe to switch reference interface list failed");
    }

   rc = ctc_nc_rpc_subscribe(NULL, "/hwvtep-ttp:test-rpc", ncs_rpc_test_cb, NULL, SR_SUBSCR_CTX_REUSE, &sr_ttp_subscription);
   if (SR_ERR_OK != rc) {
       log_sys(M_MOD_INTERNAL, E_ERROR, "ncs_nc_init subscribe to rpc test failed");
   }

   ncs_hwvtep_ttp_switch_init();
   
}

static void 
ncs_hwvtep_ttp_unsubscribe_init()
{
    int32 rc = PM_E_NONE;

    ncs_hwvtep_ttp_switch_reset();

    rc = ctc_nc_unsubscribe(NULL, sr_ttp_subscription);
    if (SR_ERR_OK != rc) {
       log_sys(M_MOD_INTERNAL, E_ERROR, "ncs_nc_init unsubscribe failed");
    }
}

static void
ncs_oam_aps_subscribe_init()
{
    int32 rc = PM_E_NONE;

    /*subscribe aps group list*/
    rc = ctc_nc_subtree_change_subscribe(NULL, "/centec-oam-aps:aps-groups/aps-group", ncs_oam_aps_group_list_change_cb, NULL, 0, SR_SUBSCR_CTX_REUSE, NULL);
    if (SR_ERR_OK != rc)
    {
         log_sys(M_MOD_INTERNAL, E_ERROR, "ncs_nc_init subscribe to aps group list failed");
    }

    /*subscribe oam session list*/
    rc = ctc_nc_subtree_change_subscribe(NULL, "/centec-oam-aps:oam-sessions/oam-session", ncs_oam_aps_session_list_change_cb, NULL, 0, SR_SUBSCR_CTX_REUSE, NULL);
    if (SR_ERR_OK != rc)
    {
         log_sys(M_MOD_INTERNAL, E_ERROR, "ncs_nc_init subscribe to oam session list failed");
    }
}

static void 
ncs_module_install_cb(const char *module_name, const char *revision, sr_module_state_t state,
        void *private_ctx)
{
    if (0 == sal_strcmp(module_name, "hwvtep-ttp") && state == SR_MS_IMPLEMENTED)
    {
        ctc_task_add_timer(TASK_PRI_LOW, ncs_hwvtep_ttp_subscribe_init, NULL, 1);
    }
    else if (0 == sal_strcmp(module_name, "hwvtep-ttp") && state == SR_MS_UNINSTALLED)
    {
        ctc_task_add_timer(TASK_PRI_LOW, ncs_hwvtep_ttp_unsubscribe_init, NULL, 1);
    }
    else if (0 == sal_strcmp(module_name, "centec-oam-aps") && state == SR_MS_IMPLEMENTED)
    {
        ctc_task_add_timer(TASK_PRI_LOW, ncs_oam_aps_subscribe_init, NULL, 1);
    }
}


int32
ncs_nc_init()
{
    int32 rc = PM_E_NONE;

    /* init netconf client */
    if (ncutil_init("ncs", NULL, ncs_nc_reinit_cb) != 0) {
        log_sys(M_MOD_INTERNAL, E_CRITICAL, "ncs_nc_init failed");
        rc = PM_E_FAIL;
        goto err_out;
    }

    /* Changes datastore to which the session is tied to */
    rc = ctc_nc_session_switch_ds(NULL, SR_DS_RUNNING);
    if (SR_ERR_OK != rc) {
       log_sys(M_MOD_INTERNAL, E_ERROR, "ncs_nc_init change session datastore to running datastore failed");
       goto err_out;
    }

    /* register module install and uninstall callback */
    rc = ctc_nc_module_install_subscribe(NULL, ncs_module_install_cb, NULL, SR_SUBSCR_DEFAULT, NULL);
    if (SR_ERR_OK != rc) {
       log_sys(M_MOD_INTERNAL, E_ERROR, "ncs_nc_init subscribe to module install failed");
       goto err_out;
    }

    /* register hostname change callback */
    rc = ctc_nc_subtree_change_subscribe(NULL, "/ietf-system:system/hostname",
        ncs_ietf_system_hostname_change_cb, NULL, 0, SR_SUBSCR_CTX_REUSE, NULL);
    if (SR_ERR_OK != rc) {
       log_sys(M_MOD_INTERNAL, E_ERROR, "ncs_nc_init subscribe to hostname failed");
       goto err_out;
    }

    ncs_register_cb();

    return PM_E_NONE;
err_out:
    ncutil_deinit();
    return rc;
}

#endif /* !HAVE_NETCONF */

