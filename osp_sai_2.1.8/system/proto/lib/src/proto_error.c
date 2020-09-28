
#include "proto.h"

const char*
pm_err_desc(int32 error)
{
    switch (error)
    {
    case PM_E_NONE:
        return "success";
    default:
        return "unknown error";
    }
}
