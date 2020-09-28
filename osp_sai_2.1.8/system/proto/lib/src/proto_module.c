
#include "proto.h"

const char*
proto_module_desc(pm_id_t pm_id)
{
    switch (pm_id)
    {
    case PM_ID_CTL:
        return "Ctl";
    case PM_ID_CCS:
        return "CCS";
    case PM_ID_CDS:
        return "CDS";
    case PM_ID_SWITCH:
        return "Switch";
    case PM_ID_ROUTED:
        return "Routed";
    case PM_ID_OPM:
        return "OamApsMgr";
    case PM_ID_CHSM:
        return "CHSM";
    case PM_ID_APPCFG:
        return "AppCfg";
    case PM_ID_FEA:
        return "FEA";
    case PM_ID_OPENFLOW:
        return "ovs-vswitchd";
    case PM_ID_DHCRELAY:
        return "DhcpRelay";
    case PM_ID_DHCSNOOPING:
        return "DhcpSnooping";
    case PM_ID_DHCLIENT:
        return "DhcpClient";
    case PM_ID_PTP:
        return "Ptp";
    case PM_ID_BHM:
        return "BHM";
    case PM_ID_EVENTMGR:
        return "EventMgr";
    case PM_ID_SSM:
        return "SSM";
    case PM_ID_SUBSCRIBER:
        return "Subscriber";
    case PM_ID_NCS:
        return "NCS";
    case PM_ID_DHCPD6:
        return "DHCPD6";
    case PM_ID_AUTHD:
        return "Authd";
    case PM_ID_DHCLIENT6:
        return "DHCLIENT6";
    default:
        return "Invalid";
    }
}

const char*
proto_module_name_desc(pm_id_t pm_id)
{
    switch (pm_id)
    {
    case PM_ID_DHCPD6:
        return "DHCPD6";
    case PM_ID_CTL:
        return "cdbctl";
    case PM_ID_CCS:
        return "ccs";
    case PM_ID_CDS:
        return "cds";
    case PM_ID_SWITCH:
        return "switch";
    case PM_ID_ROUTED:
        return "routed";
    case PM_ID_OPM:
        return "opm";
    case PM_ID_CHSM:
        return "chsm";
    case PM_ID_APPCFG:
        return "appcfg";
    case PM_ID_FEA:
        return "fea";
    case PM_ID_OPENFLOW:
        return "ovs-vswitchd";
    case PM_ID_DHCRELAY:
        return "dhcrelay";
    case PM_ID_DHCSNOOPING:
        return "dhcsnooping";
    case PM_ID_DHCLIENT:
        return "dhcpclient";
    case PM_ID_PTP:
        return "ptp";
    case PM_ID_BHM:
        return "bhm";
    case PM_ID_EVENTMGR:
        return "eventmgr";
    case PM_ID_SSM:
        return "ssm";
    case PM_ID_SUBSCRIBER:
        return "subscriber";
    case PM_ID_AUTHD:
        return "authd";
    case PM_ID_NCS:
        return "ncs";
    case PM_ID_DHCLIENT6:
        return "dhclient6";
    default:
        return "invalid";
    }
}

const char*
proto_module_ext_name_desc(pm_id_ext_t pm_id)
{
    switch (pm_id)
    {
    case PM_ID_EXT_ZEBRA:
        return "zebra";
    case PM_ID_EXT_OSPFD:
        return "ospfd";
    case PM_ID_EXT_BGPD:
        return "bgpd";
    case PM_ID_EXT_PIMD:
        return "pimd";
    case PM_ID_EXT_OVSDB:
        return "ovsdb-server";
    default:
        return "invalid";
    }
}