/****************************************************************************
* $Id$
*  Centec Iptables related MACRO and ENUM, Structure defines file
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* Revision      : R0.01
* Author        : Wenxin Hu
* Date          : 2015-02-11 10:00
* Reason        : First Create.
****************************************************************************/

#ifndef __GLB_IPTABLES_H__
#define __GLB_IPTABLES_H__

#ifdef _GLB_UML_SYSTEM_
#define IPTABLES_CMD_PREFIX                   "iptables"
#define IPTABLES_NETNS_CMD_PREFIX             "ip netns exec mgmt iptables"
#else
#define IPTABLES_CMD_PREFIX                   "xtables-multi iptables"
#define IPTABLES_NETNS_CMD_PREFIX             "ip netns exec mgmt xtables-multi iptables"
#endif

#define IPTABLES_CHAIN_NAME_TELNET            "telnet_acl"
#define IPTABLES_CHAIN_NAME_SSH               "ssh_acl"

//#ifdef _KERNEL_VER_3_6_
//#define IPTABLES_NETNS_CMD_PREFIX             "chvrf 64 xtables-multi iptables"
//#else
//#define IPTABLES_NETNS_CMD_PREFIX             "ip netns exec mgmt xtables-multi iptables"
//#endif

#define GLB_IPTABLES_MAX_IP_LEN               16
#define GLB_IPTABLES_MIN_MAXCOUNT_NUM         0
#define GLB_IPTABLES_DEFAULT_MAXCOUNT         500
#define GLB_IPTABLES_MAX_MAXCOUNT_NUM         1000
#define GLB_IPTABLES_DEFAULT_LENGTH           28
#define GLB_IPTABLES_MAX_LENGTH               65535
#define GLB_IPTABLES_MAX_CMD_BUFSZ            1024  
#define GLB_IPTABLES_MAX_LINE_BUFSZ           256

#define GLB_IPTABLES_EXEC_SUCCESS                     0
#define GLB_IPTABLES_EXEC_SYS_FAIL                    -1
#define GLB_IPTABLES_EXEC_INPUT_ERR                   -2

#define GLB_IPTABLES_CFG_FLAG_ICMP             (1 << 0)
#define GLB_IPTABLES_CFG_FLAG_SMURF            (1 << 1)
#define GLB_IPTABLES_CFG_FLAG_FRAGGLE          (1 << 2)
#define GLB_IPTABLES_CFG_FLAG_UDP              (1 << 3)
#define GLB_IPTABLES_CFG_FLAG_TCP              (1 << 4)
#define GLB_IPTABLES_CFG_FLAG_SMALL_PKT        (1 << 5)
#define GLB_IPTABLES_CFG_FLAG_MACEQ            (1 << 6)
#define GLB_IPTABLES_CFG_FLAG_IPEQ             (1 << 7)

typedef enum
{
   GLB_IPTABLES_CHAIN_INPUT         =  1,
   GLB_IPTABLES_CHAIN_FOWRWARD,
   GLB_IPTABLES_CHAIN_OUTPUT,
   GLB_IPTABLES_CHAIN_ICMP_FLOOD,
   GLB_IPTABLES_CHAIN_SMURF,
   GLB_IPTABLES_CHAIN_SYN_FLOOD,
   GLB_IPTABLES_CHAIN_UDP_FRAGGLE
}glb_iptables_chain_type_t;

typedef enum
{
   GLB_IPTABLES_SMALPKT_STATISTIC         =  0,
   GLB_IPTABLES_ICMP_STATISTIC,
   GLB_IPTABLES_SMURF_STATISTIC,
   GLB_IPTABLES_SYN_STATISTIC,
   GLB_IPTABLES_FRAGGLE_STATISTIC,
   GLB_IPTABLES_UDP_STATISTIC
}glb_iptables_statistic_type_t;


#endif /* __GLB_IPTABLES_H__ */

