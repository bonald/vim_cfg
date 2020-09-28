
#ifdef HAVE_NETCONF

#include "proto.h"
#include "ncutil.h"
#include "ncs.h"
#include "ncs_api.h"

int
ncs_ietf_system_hostname_change_cb(sr_session_ctx_t *srs, const char *module_name, 
    sr_notif_event_t event, void *private_ctx) 
{
    int rc;
    sr_val_t *val = NULL;

    /* read params from the user / sysrepod */
    rc = sr_get_item(srs, "/ietf-system:system/hostname", &val);
    if (rc != SR_ERR_OK) 
    {
        goto err_out;
    }

    /* check param type */
    if (SR_STRING_T != val->type) 
    {
        rc = SR_ERR_INVAL_ARG;
        goto err_out;
    }

    if (SR_EV_VERIFY == event) 
    {
        rc = ncs_api_set_hostname(CDB_CFG_NC_CHECK, val->data.string_val);
        if (rc < 0)
        {
            rc = SR_ERR_INVAL_ARG;
            goto err_out;
        }
    }
    else if (SR_EV_APPLY == event) 
    {
        rc = ncs_api_set_hostname(CDB_CFG_NC_SET, val->data.string_val);
        if (rc < 0)
        {
            rc = SR_ERR_OPERATION_FAILED;
            goto err_out;
        }
    } 
    else if (SR_EV_ENABLED == event) 
    {
        /* Startup stage: subscription establish event
         *   XXX: copy from startup config to running config
         */
    }

    rc = SR_ERR_OK;

err_out:
    if (NULL != val) 
    {
        sr_free_val(val);
    }
    return rc;
}

#endif /* !HAVE_NETCONF */

