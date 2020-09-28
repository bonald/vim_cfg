
#ifndef __PROTO_CONFIG_H__
#define __PROTO_CONFIG_H__

#define HAVE_IPV6   1

/* if want to cut code, should not define CONFIG_XXX in TAPPRODUCT */
#define CONFIG_L2
#define CONFIG_ERRDISABLE
#define CONFIG_MSTP
#define CONFIG_SFLOW
#define CONFIG_MIRROR
#define CONFIG_IPV4
#define CONFIG_ARP
#define CONFIG_PVLAN
#define CONFIG_LLDP
#define CONFIG_IGSP     /* IGMP Snooping */
#define CONFIG_VLANCLASS
#define CONFIG_VLANMAPPING

/* Below features could be configured */
#define CONFIG_MLAG
#define CONFIG_ERPS
#define CONFIG_OPENFLOW
#define CONFIG_TPOAM

#ifdef  TTPPRODUCT
#undef  CONFIG_TPOAM
#endif

/* TAP also using python agent for support snmpset */
#ifndef TAPPRODUCT
/* if defined, use c based snmpagent, otherwise use python based snmp_agent_main.py */
#define CONFIG_SNMPAGT
#endif /* !TAPPRODUCT */

#endif /* !__PROTO_CONFIG_H__ */

