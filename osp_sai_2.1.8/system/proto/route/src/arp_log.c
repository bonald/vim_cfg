/**************************************************************************
* arp_log.c   :   arp log message define
*
* (C) Copyright Centec Networks Inc.  All rights reserved.
*
* Modify History:
* 2011 - 10 - 29
* Revision          R0.1
* Author            chenyuefang
*                       Email: ychen@centecnetworks.com
* Date               2011 - 10 - 29
* Reason   :       First Create.
**************************************************************************/
#include "proto.h"
#include "arp_log.h"

log_message_t g_arp_log_message[] =
{
    { 0, "Unrecognized Error Code" },   
    { ARP_4_DB_CACHE_OVERFLOW, "%% No ARP buffer space available" }, 
    { ARP_4_DUPLICATE_IP_ADDR, "%% Duplicate use of %s detected, interface name is %s" },
    { ARP_4_DISCARD_PACKET, "%% ARP packet has been discard, there are %d packets discarded in total" },
    { ARP_3_MEM_EXCEED, "%% Exceed system memory threshhold, discard ARP packet, there are %d packets discarded in total" },
};

