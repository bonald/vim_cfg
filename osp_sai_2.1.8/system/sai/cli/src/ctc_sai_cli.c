#include "sal.h"
#include "ctc_cli.h"
#include <sai.h>
#include "ctc_sai_sai_cli.h"
#include "ctc_sai_fdb_cli.h"
#include "ctc_sai_switch_cli.h"
#include "ctc_sai_vlan_cli.h"
#include "ctc_sai_port_cli.h"
#include "ctc_sai_router_cli.h"
#include "ctc_sai_route_cli.h"
#include "ctc_sai_router_intf_cli.h"
#include "ctc_sai_neighbor_cli.h"
#include "ctc_sai_nexthop_cli.h"
#include "ctc_sai_nexthop_group_cli.h"
#include "ctc_sai_stp_cli.h"
#include "ctc_sai_lag_cli.h"
#include "ctc_sai_acl_cli.h"
#include "ctc_sai_qosmaps_cli.h"
#include "ctc_sai_queue_cli.h"
#include "ctc_sai_wred_cli.h"
#include "ctc_sai_policer_cli.h"
#include "ctc_sai_scheduler_cli.h"
#include "ctc_sai_hostif_cli.h"
#include "ctc_sai_mirror_cli.h"
#include "ctc_sai_isolation_cli.h"
#include "ctc_sai_ptf_cli.h"
#include "ctc_sai_debug.h"
#include "ctc_sai_vlanclass_cli.h"

const char* ctc_sai_get_error_desc(sai_status_t error)
{
    static char szunkonw[32] = "";

    switch(error)
    {
        case SAI_STATUS_SUCCESS:
            return "Status success";
        case SAI_STATUS_FAILURE:
            return "General failure";
        case SAI_STATUS_NOT_SUPPORTED:
            return "The request is not supported";
        case SAI_STATUS_NO_MEMORY:
            return "Not enough memory to complete the operation";
        case SAI_STATUS_INSUFFICIENT_RESOURCES:
            return "Insufficient system resources exist to complete the operation";
        case SAI_STATUS_INVALID_PARAMETER:
            return "An invalid parameter was passed to a function";
        case SAI_STATUS_ITEM_ALREADY_EXISTS:
            return "An item already exists";
        case SAI_STATUS_ITEM_NOT_FOUND:
            return "An item was not found";
        case SAI_STATUS_BUFFER_OVERFLOW:
            return "The data was too large to fit into the specified buffer.";
        case SAI_STATUS_INVALID_PORT_NUMBER:
            return "Invalid port number";
        case SAI_STATUS_INVALID_PORT_MEMBER:
            return "Invalid port member";
        case SAI_STATUS_INVALID_VLAN_ID:
            return "Invalid VLAN id";
        case SAI_STATUS_UNINITIALIZED:
            return "Object is uninitialized";
        case SAI_STATUS_TABLE_FULL:
            return "Table is full";
        case SAI_STATUS_OBJECT_IN_USE:
            return "SAI Object is in use";
    }

    if((SAI_STATUS_INVALID_ATTRIBUTE_0 & error) == SAI_STATUS_INVALID_ATTRIBUTE_0)
    {
        sal_snprintf(szunkonw,sizeof(szunkonw),"%s\n","Attribute is invalid");
    }

    if((SAI_STATUS_INVALID_ATTR_VALUE_0 & error) == SAI_STATUS_INVALID_ATTR_VALUE_0)
    {
        sal_snprintf(szunkonw,sizeof(szunkonw),"%s\n","Invalid attribute value : ");
    }

    if((SAI_STATUS_ATTR_NOT_IMPLEMENTED_0 & error) == SAI_STATUS_ATTR_NOT_IMPLEMENTED_0)
    {
        sal_snprintf(szunkonw,sizeof(szunkonw),"%s\n","Attribute is not implmented : ");
    }

    if((SAI_STATUS_UNKNOWN_ATTRIBUTE_0 & error) == SAI_STATUS_UNKNOWN_ATTRIBUTE_0)
    {
        sal_snprintf(szunkonw,sizeof(szunkonw),"%s\n","Attribute is unknown : ");
    }

    if((SAI_STATUS_ATTR_NOT_SUPPORTED_0 & error) == SAI_STATUS_ATTR_NOT_SUPPORTED_0)
    {
        sal_snprintf(szunkonw,sizeof(szunkonw),"%s\n","Attribute is not supported : ");
    }

    sal_snprintf(szunkonw,sizeof(szunkonw),"%s : %x\n","unkown error",error);
    return szunkonw;
}

int32
ctc_sai_cli_init(uint8 cli_tree_mode)
{
#if 0
    ctc_sai_sai_cli_init(cli_tree_mode);
    ctc_sai_fdb_cli_init(cli_tree_mode);
    ctc_sai_vlan_cli_init(cli_tree_mode);
    ctc_sai_mirror_cli_init(cli_tree_mode);
    ctc_sai_switch_cli_init(cli_tree_mode);
    ctc_sai_port_cli_init(cli_tree_mode);
    ctc_sai_router_cli_init(cli_tree_mode);
    ctc_sai_router_intf_cli_init(cli_tree_mode);
    ctc_sai_neighbor_cli_init(cli_tree_mode);
    ctc_sai_nexthop_cli_init(cli_tree_mode);
    ctc_sai_nexthop_group_cli_init(cli_tree_mode);
    ctc_sai_route_cli_init(cli_tree_mode);
    ctc_sai_stp_cli_init(cli_tree_mode);
    ctc_sai_lag_cli_init(cli_tree_mode);
    ctc_sai_acl_cli_init(cli_tree_mode);
    ctc_sai_policer_cli_init(cli_tree_mode);
    ctc_sai_wred_cli_init(cli_tree_mode);
    ctc_sai_queue_cli_init(cli_tree_mode);
    ctc_sai_scheduler_cli_init(cli_tree_mode);
    ctc_sai_qos_map_cli_init(cli_tree_mode);
    ctc_sai_hostif_init(cli_tree_mode);
    ctc_sai_isolation_cli_init(cli_tree_mode);
    ctc_sai_ptf_cli_init(cli_tree_mode);
    ctc_sai_debug_cli_init(cli_tree_mode);
    ctc_sai_vlanclass_cli_init(cli_tree_mode);
#endif
    return CLI_SUCCESS;
}
