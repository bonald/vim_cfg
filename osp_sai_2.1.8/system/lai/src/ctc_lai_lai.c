#include <laiinc.h>


ctc_lai_lai_sys_info_t g_lai_sys_info = {
    .api_queue          = {0},
    .lai_init_status    = UNINITIALIZED,
    .lai_card_notification_table = {NULL},
};

ctc_lai_lai_sys_info_t*
ctc_lai_get_sys_info()
{
    return &g_lai_sys_info;
}


lai_status_t
lai_api_reg_register_fn(
    _In_  ctc_lai_api_reg_info_t    *preg)
{
    g_lai_sys_info.api_queue[preg->id] = preg;

    return LAI_STATUS_SUCCESS;
}

lai_status_t
lai_api_reg_unregister_fn(
    _In_  ctc_lai_api_reg_info_t    *preg)
{
    g_lai_sys_info.api_queue[preg->id] = NULL;
    return LAI_STATUS_ITEM_NOT_FOUND;
}

/*
* Routine Description:
*     Adapter module initialization call. This is NOT for SDK initialization.
*
* Arguments:
*     [in] flags - reserved for future use, must be zero
*     [in] noti_cb - Hardware notification callback function register here.
*
* Return Values:
*    LAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
lai_status_t
lai_api_initialize(
    _In_ uint64_t flags,
    _In_ lai_card_notification_t* noti_cb)
{
    /* initialize lai mode */
    ctc_lai_card_init(noti_cb);
    ctc_lai_fan_init();
    ctc_lai_psu_init();
    ctc_lai_led_init();
    ctc_lai_port_init();
    ctc_lai_thermal_init();
    ctc_lai_fiber_init();
    ctc_lai_synce_init();
    ctc_lai_vcm_init();
    ctc_lai_poe_init();

    g_lai_sys_info.lai_init_status = INITIALIZED;

    return LAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*   Uninitialization of the adapter module. LAI functionalities, retrieved via
*   lai_api_query() cannot be used after this call.
*
* Arguments:
*   None
*
* Return Values:
*   LAI_STATUS_SUCCESS on success
*   Failure status code on error
*/
lai_status_t
lai_api_unitialize(void)
{
    g_lai_sys_info.lai_init_status = UNINITIALIZED;
    return LAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*     Retrieve a pointer to the C-style method table for desired LAI
*     functionality as specified by the given lai_api_id.
*
* Arguments:
*     [in] lai_api_id - LAI api ID
*     [out] api_method_table - Caller allocated method table
*           The table must remain valid until the lai_api_shutdown() is called
*
* Return Values:
*    LAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
lai_status_t
lai_api_query(
    _In_ lai_api_t lai_api_id,
    _Out_ void** api_method_table)
{

    ctc_lai_api_reg_info_t *pvalue= NULL;

    if(lai_api_id >= LAI_API_MAX)
    {
        return LAI_STATUS_ITEM_NOT_FOUND;
    }
    pvalue = g_lai_sys_info.api_queue[lai_api_id];
    if(lai_api_id == pvalue->id)
    {

        *api_method_table = pvalue->api_method_table;
        return LAI_STATUS_SUCCESS;
    }

    return LAI_STATUS_ITEM_NOT_FOUND;
}

