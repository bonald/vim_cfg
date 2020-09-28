/**
 @file ctc_cpu_traffic_cli.h

 @date 2010-05-25

 @version v2.0

*/

#ifndef _CTC_CPU_TRAFFIC_CLI_H_
#define _CTC_CPU_TRAFFIC_CLI_H_

#include "sal.h"
#include "ctc_cli.h"
#include "ctc_cli_common.h"

#define CTC_CLI_PACKET_TO_CPU_REASON_STR_TOKEN \
    "{user-id | proto-vlan | bridge sub-index INDEX| route-ipda sub-index INDEX| route-icmp | learning | mcast-rpf | \
    security sub-index INDEX| mtu-dontfrag | mtu-frag | ttl | mcast-ttl | tunnel-mtu-dontfrag | \
    tunnel-mtu-frag | bfd-udp | ucast-ip-option | ucast-gre-unknown | more-rpf | \
    ucast-link-id-fail | igmp-snooping-pkt | igs-l2-span0 | igs-l2-span1 | igs-l2-span2 | \
    igs-l2-span3 | igs-l3-span0 | igs-l3-span1 | igs-l3-span2 | igs-l3-span3 | igs-acl-log0 | \
    igs-acl-log1 | igs-acl-log2 | igs-acl-log3 | igs-qos-log0 | igs-qos-log1 | igs-qos-log2 | \
    igs-qos-log3 | egs-l2-span0 | egs-l2-span1 | egs-l2-span2 | egs-l2-span3 | egs-l3-span0 | \
    egs-l3-span1 | egs-l3-span2 | egs-l3-span3 | egs-acl-log0 | egs-acl-log1 | egs-acl-log2 | \
    egs-acl-log3 | egs-qos-log0 | egs-qos-log1 | egs-qos-log2 | egs-qos-log3 | igs-port-log | \
    igs-seqnum-check | igs-parser-ptp sub-index INDEX | igs-oam | egs-port-log | egs-seqnum-check | \
    egs-parser-ptp sub-index INDEX | egs-oam}"

#define CTC_CLI_PACKET_TO_CPU_REASON_STR_HELPER \
    "Ingress userid", \
    "Ingress protocol vlan", \
    "Ingress bridge", \
    "Ingress bridge exception sub index", \
    "<0-15>", \
    "Ingress route exception for IP-DA", \
    "Ingress route exception for IP-DA sub index", \
    "<0-15>", \
    "Ingress route exception for ICMP redirect", \
    "Ingress learning exception for cache full", \
    "Ingress route multicast RPF fail", \
    "Ingress security", \
    "Ingress security exception sub index", \
    "<0-4>", \
    "Egress mtu check fail and original packet don't frag", \
    "Egress mtu check fail and original packet can frag", \
    "Egress TTL equals to 0", \
    "Egress multicast TTL threshold", \
    "Egress tunnel mtu check fail and original packet don't frag", \
    "Egress tunnel mtu check fail and original packet can frag", \
    "Egress BFD UDP", \
    "Ucast IP options", \
    "Ucast GRE unknown option or protocol", \
    "Ucast or Mcast more RPF", \
    "Ucast Link ID check failure", \
    "IGMP Snooping packet", \
    "Ingress l2 span 0", \
    "Ingress l2 span 1", \
    "Ingress l2 span 2", \
    "Ingress l2 span 3", \
    "Ingress l3 span 0", \
    "Ingress l3 span 1", \
    "Ingress l3 span 2", \
    "Ingress l3 span 3", \
    "Ingress acl log 0", \
    "Ingress acl log 1", \
    "Ingress acl log 2", \
    "Ingress acl log 3", \
    "Ingress qos log 0", \
    "Ingress qos log 1", \
    "Ingress qos log 2", \
    "Ingress qos log 3", \
    "Egress l2 span 0", \
    "Egress l2 span 1", \
    "Egress l2 span 2", \
    "Egress l2 span 3", \
    "Egress l3 span 0", \
    "Egress l3 span 1", \
    "Egress l3 span 2", \
    "Egress l3 span 3", \
    "Egress acl log 0", \
    "Egress acl log 1", \
    "Egress acl log 2", \
    "Egress acl log 3", \
    "Egress qos log 0", \
    "Egress qos log 1", \
    "Egress qos log 2", \
    "Egress qos log 3", \
    "Ingress port log", \
    "Ingress sequence number check", \
    "Ingress parser PTP", \
    "Ingress parser PTP exception sub index", \
    "<0-6>", \
    "Ingress OAM", \
    "Egress port log", \
    "Egress sequence number check", \
    "Egress parser PTP", \
    "Egress parser PTP exception sub index", \
    "<0-3>", \
    "Egress OAM"

#ifdef __cplusplus
extern "C" {
#endif

extern int32
ctc_cpu_traffic_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif

