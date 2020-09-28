/****************************************************************************
* $Id$
*  Define some internel type and global variables for user-space genlog
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : zhoucheng
* Date          : 2007-11-13 16:54
* Reason        : First Create.
****************************************************************************/
#ifndef _GENLOG_INNER_H_
#define _GENLOG_INNER_H_

/*
 * module name pair
 * notice: please keep the order with E_MODULE!!
 */
S_MODULE_NAME_PAIR gModuleNameArr[] =
{
    {M_MOD_UNKNOWN,     "UNKNOWN"},
    {M_MOD_LOG,         "LOG"},
    {M_MOD_LIB,         "LIB"},
    {M_MOD_NETLINK,     "NETLINK"},

    {M_MOD_INTF,        "INTERFACE"},
    {M_MOD_VLAN,        "VLAN"},
    {M_MOD_FDB,         "FDB"},
    {M_MOD_LAG,         "LAG"},
    {M_MOD_LACP,        "LACP"},
    {M_MOD_MSTP,        "MSTP"},
    {M_MOD_ROUTE,       "ROUTE"},
    {M_MOD_ARP,         "ARP"},
    {M_MOD_IGMP,        "IGMP"},
    {M_MOD_IGSP,        "IGMPSnooping"},
    {M_MOD_LLDP,        "LLDP"},
    
    {M_MOD_CFG,         "CFG"},
    {M_MOD_CDS,         "CDS"},
    {M_MOD_CHSM,        "CHSM"},
    {M_MOD_FEI,         "FEI"},
    {M_MOD_HSRV,        "HSRV"},
    {M_MOD_LSRV,        "LSRV"},
    {M_MOD_HAGT,        "HAGT"},
    {M_MOD_LCM,         "LCM"},
    {M_MOD_LCSH,        "LCSH"},
    {M_MOD_SDK,         "SDK"},
    {M_MOD_DRV,         "DRV"},
    {M_MOD_APP,         "APP"},
    {M_MOD_IPSOUR,      "IPSOUR"},
    {M_MOD_DHCPRLY,     "DHCPRLY"},
    {M_MOD_DHCPCLI,     "DHCPCLI"},
    {M_MOD_DHCPSNOOPING, "DHCPSNOOPING"},
    {M_MOD_DHCPSNOOPING6, "DHCPSNOOPING6"},
    {M_MOD_DHCPRLY6,     "DHCPRLY6"},
    {M_MOD_DHCLIENT,     "DHCLIENT"},
    {M_MOD_DHCLIENT6,     "DHCLIENT6"},
    {M_MOD_VLAN_SECURITY, "VLANSEC"},
    {M_MOD_MLAG,        "MLAG"},
    {M_MOD_INTERNAL,    "INTERNAL"},
    {M_MOD_SAI,         "SAI"},
    {M_MOD_AUTHD,       "AUTHD"},
    {M_MOD_BHM,         "BHM"},
    {M_MOD_ARPINSPECTION, "ARPINSPECTION"},
    {M_MOD_SFLOW,       "SFLOW"},
    {M_MOD_OPENFLOW,    "OPENFLOW"},  
    {M_MOD_VRRP,        "VRRP"},
    {M_MOD_SSM,         "SSM"},
    {M_MOD_IPSLA,       "IPSLA"},
    {M_MOD_TRACK,       "TRACK"},
    {M_MOD_PTP,       "PTP"},
    {M_MOD_G8131,       "G8131"},
    {M_MOD_TPOAM,       "TPOAM"},
    {M_MOD_ND,            "ND"},
    {M_MOD_NCS,         "NETCONF"},
    {M_MOD_SNMPAGT,     "SNMPAGT"},
    {M_MOD_RMON,        "RMON"},
    {M_MOD_DHCPD6,     "DHCPD6"},
    {M_MOD_NAT,        "NAT"},
    {M_MOD_G8032,        "G8032"},
    {M_MOD_PIMSM,       "PIMSM"},
    {M_MOD_MAX,         "UNKNOWN"},
};

/*modified by weij for bug 18671, 2012-03-22*/
log_message_rate_limit_t g_log_rate_limit_arr[] =
{
    {LOG_RATE_LIMIT_TYPE_DEF, 0, 0, 0},
    {LOG_RATE_LIMIT_TYPE_LIB_EXC_MEM_THRESHOLD, M_LOG_FREQ_INTVAL_1, 0, 0},
    {LOG_RATE_LIMIT_TYPE_DHCP_SNOOPING_IPSG_LIMIT, M_LOG_FREQ_INTVAL_1, 0, 0}, /* modified by liuyang for bug 19756 2012-7-4 */    
    {LOG_RATE_LIMIT_TYPE_MAX, 0, 0, 0}
};

#endif /* !_GENLOG_INNER_H_ */
