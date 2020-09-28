#include "sal_types.h"
#include "dal.h"

static dal_access_type_t g_dal_access = DAL_PCIE_MM;

/* set device access type, must be configured before dal_op_init */
int32
dal_set_device_access_type(dal_access_type_t device_type)
{
    if (device_type >= DAL_MAX_ACCESS_TYPE)
    {
        return DAL_E_INVALID_ACCESS;
    }

    g_dal_access = device_type;
    return 0;
}

/* get device access type */
int32
dal_get_device_access_type(dal_access_type_t* p_device_type)
{
    *p_device_type = g_dal_access;
    return 0;
}