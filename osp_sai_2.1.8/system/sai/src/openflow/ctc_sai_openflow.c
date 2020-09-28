#include <sal.h>
#include <sai.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_openflow.h>
#include <ctc_sai_routerintf.h>
#include <ctc_sai_debug.h>
#include <ctc_api.h>

static sai_openflow_api_t g_sai_openflow_api_func = {
    .create_openflow_port = NULL,
    .remove_openflow_port = NULL,    
};

sai_status_t
__openflow_init_mode_fn(
    _In_  ctc_sai_api_reg_info_t *preg,
    _In_  void *private_data)
{
    preg->init_status = INITIALIZED;

    return SAI_STATUS_SUCCESS;
}

sai_status_t
__openflow_exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

static ctc_sai_api_reg_info_t g_openflow_api_reg_info = {
        .id         = SAI_API_OPENFLOW,
        .init_func  = __openflow_init_mode_fn,
        .exit_func  = __openflow_exit_mode_fn,
        .api_method_table = &g_sai_openflow_api_func,
        .private_data     = NULL,
};

sai_status_t
ctc_sai_openflow_init()
{
    api_reg_register_fn(&g_openflow_api_reg_info);

    return SAI_STATUS_SUCCESS;
}

