/* Copyright (C) 2007-2008 CentecNetworks, Inc. All Rights Reserved. */
#if 0
#ifndef _CENTEC_DHCRELAY_SNMP_H
#define _CENTEC_DHCRELAY_SNMP_H

#ifdef HAVE_SNMP

/* DHCP-MIB. */
#define DHCPMIB SNMP_ENTERPRISE_COMMON_OID,36,1
#define DHCPRMIB SNMP_ENTERPRISE_COMMON_OID,36,3
#define DHCPRGLOBALMIB SNMP_ENTERPRISE_COMMON_OID,36,3,1
#define DHCPRSERVERMIB SNMP_ENTERPRISE_COMMON_OID,36,3,2

/* Hack for SNMP values */
#define COUNTER        ASN_COUNTER
#define COUNTER64      ASN_COUNTER64
#define GAUGE          ASN_GAUGE
#define IPADDRESS      ASN_IPADDRESS
#define STRING         ASN_OCTET_STR
#define INTEGER        ASN_INTEGER
#define TIMETICKS      ASN_TIMETICKS
#define OBJECT_ID      ASN_OBJECT_ID
#define ROWSTATUS      ASN_INTEGER
#define DISPLAYSTRING  ASN_OCTET_STR
#define TRUTHVALUE     ASN_INTEGER

#define NOTACCESSIBLE  NOACCESS

/* TruthValue */
#define DHCP_TRUTH_VALUE_TRUE    1
#define DHCP_TRUTH_VALUE_FALSE   2

#define DHCP_SNMP_ROW_STATUS_MIN            0
#define DHCP_SNMP_ROW_STATUS_ACTIVE         1   
#define DHCP_SNMP_ROW_STATUS_NOTINSERVICE   2
#define DHCP_SNMP_ROW_STATUS_NOTREADY       3
#define DHCP_SNMP_ROW_STATUS_CREATEANDGO    4
#define DHCP_SNMP_ROW_STATUS_CREATEANDWAIT  5
#define DHCP_SNMP_ROW_STATUS_DESTROY        6
#define DHCP_SNMP_ROW_STATUS_MAX            7

#define DHCP_SNMP_RETURN_INTEGER(V) \
  do { \
    *var_len = sizeof (int); \
    dhcp_int_val = V; \
    return (u_char *) &dhcp_int_val; \
  } while (0)

#define DHCP_SNMP_RETURN_STRING(STR) \
  do { \
    if (STR != NULL)\
      {\
        *var_len = pal_strlen(STR);\
        pal_strcpy((char *)dhcp_char_str, STR);\
        return dhcp_char_str;\
      }\
    else\
      {\
        *var_len = 0;\
        return (unsigned char *)"\0";\
      }\
  } while (0)
  
#define DHCP_SNMP_RETURN_STRING0() \
  do { \
        *var_len = 0;\
        return "\0";\
  } while (0)  

/* centecDhcpEnable */
enum
{
    DHCPENABLE = 1,
};

/*
 *    DHCP relay
 */

 /* centecDhcpREnable */
enum
{
    DHCPRENABLE = 1,
    DHCPRINFOTRUSTALL
};

/*centecDhcpRIfOption82TrustTable*/
enum
{
    DHCPRIFOPTION82TRUST = 1
};

/*centecDhcpRIfSelectTable*/
enum
{
    DHCPRIFSELECTSERVER = 1
};

/*centecDhcpRInfoOption*/
enum
{
    DHCPROPTION82 = 1,
    DHCPROPTION82CHECK,
    DHCPROPTION82POLICY
};

/*centecDhcpRIpToGroupTable*/
enum
{
    DHCPRGROUPGROUPID = 1,
    DHCPRGROUPSERVERIP,
    DHCPRGROUPSTATUS
};

/* centecDhcpRStatisticsGroup */
enum
{
    DHCPRRXCLIENTPKTNUM = 1,
    DHCPRRXSERVERPKTNUM,
    DHCPRRXCLIENTERRPKTNUM,
    DHCPRRXSERVERERRPKTNUM,
    DHCPRRXBOGUSPKTNUM,
    DHCPRRXBADIDPKTNUM,
    DHCPRRXCORRUPTPKTNUM,
    DHCPRRXMISSAGENTOPPKTNUM,
    DHCPRRXMISSCIRCUITIDPKTNUM,
    DHCPRRXSTATISTICSRESET
};

void dhcrelay_snmp_init (struct lib_globals *);

#ifdef HAVE_IPV6
/* DHCPV6-MIB. */
#define DHCPV6MIB SNMP_ENTERPRISE_COMMON_OID,36,6
#define DHCPV6RMIB SNMP_ENTERPRISE_COMMON_OID,36,8
#define DHCPV6RGLOBALMIB SNMP_ENTERPRISE_COMMON_OID,36,8,1
#define DHCPV6RSERVERMIB SNMP_ENTERPRISE_COMMON_OID,36,8,2
#define DHCPV6ROPTIONMIB SNMP_ENTERPRISE_COMMON_OID,36,8,3
#define DHCPV6RSTATSMIB SNMP_ENTERPRISE_COMMON_OID,36,8,4

/* centecDhcpv6Enable */
enum
{
    DHCPV6ENABLE = 1
};

/* centecDhcpv6rglobal */
enum
{
    DHCPV6RENABLE = 5
};

/*centecDhcpv6RIpToGroupTable*/
enum
{
    DHCPV6RGROUPGROUPID = 11,
    DHCPV6RGROUPSERVERIPIF,
    DHCPV6RGROUPSTATUS
};

/*centecDhcpv6ROptionl*/
enum
{
    DHCPV6RREMOTEID = 15,
    DHCPV6RREMOTEIDDUID,
    DHCPV6RREMOTEIDIFNAME,
    DHCPV6RREMOTEIDVLAN,
    DHCPV6RPDENABLE,
    DHCPV6RPDDISTANCE,
    DHCPV6RPDALLCLEAR,
    DHCPV6RPDENTRYID,    
    DHCPV6RPDIFNAME,
    DHCPV6RPDCLIENTIP,
    DHCPV6RPDPREFIX,
    DHCPV6RPDEXPIRY,
    DHCPV6RPDSTATUS  
};

/* centecDhcpv6RStatisticsGroup */
enum
{
    DHCPV6RRXCLIENTPKTNUM = 31,
    DHCPV6RRXSERVERPKTNUM,
    DHCPV6RRXCLIENTERRPKTNUM,
    DHCPV6RRXSERVERERRPKTNUM,
    DHCPV6RRXSTATISTICSRESET
};
void dhcprelay6_snmp_init (struct lib_globals *);
#endif /* HAVE_IPV6 */
#endif /* HAVE_SNMP */
#endif /* _CENTEC_DHCRELAY_SNMP_H */
#endif
