
#include "proto.h"

#include "glb_ptp_define.h"
#include "gen/tbl_ptp_port_define.h"

#include "ptp_desc.h"
#include "ptp_debug.h"

 void
ptp_desc_buffer(char *buffer, int32 buffer_size, uint32 is_tx)
{
    return;
    int i;
    int count = 0;
    char buf[543];

    memset(buf, 0, sizeof(buf));
    count += snprintf(buf + count, sizeof(buf) - count, "raw:");
    for (i = 0 ; i < buffer_size ; ++i) 
    {
        if (count > sizeof(buf)) 
        {
            break;
        }
        
        if (!(i % 8)) 
        {
            /* The syslog-ng we used not permit multi-line log msg, every
             * newline character present a split new log msg */
            count += snprintf(buf + count, sizeof(buf) - count, " %02hhx", buffer[i]);
        } 
        else 
        {
            count += snprintf(buf + count, sizeof(buf) - count, "%02hhx", buffer[i]);
        }
    }
    if (is_tx)
    {
        PTP_DBG_PACKET_TX("Print buffer: size %d,%s\n", buffer_size, buf);
    }
    else
    {
        PTP_DBG_PACKET_RX("Print buffer: size %d,%s\n", buffer_size, buf);
    }
    return;
}

#define PTP_DESC_BUF_LEN 64

static char g_desc_str[PTP_DESC_BUF_LEN];

char *
ptp_desc_bool(uint32 v)
{
    return (v) ? "true" : "false";
}

char *
ptp_desc_enable(uint32 v)
{
    return (v) ? "enable" : "disable";
}

char *
ptp_desc_ifname(tbl_ptp_port_t* p_ptp_port)
{
    static char ptp_port_name_str[IFNAME_EXT_SIZE] = {0};
    
    IFNAME_ETH2FULL(p_ptp_port->key.name, ptp_port_name_str);

    return ptp_port_name_str;
}

char *
ptp_desc_uint64(uint64 v)
{
    sal_memset(g_desc_str, 0x00, PTP_DESC_BUF_LEN);
    sal_snprintf(g_desc_str, PTP_DESC_BUF_LEN, "%"PRIu64, v);
    return g_desc_str;
}

char *
ptp_desc_float64(float64 v)
{
    sal_memset(g_desc_str, 0x00, PTP_DESC_BUF_LEN);
    sal_snprintf(g_desc_str, PTP_DESC_BUF_LEN, "%e", v);
    return g_desc_str;
}

char *
ptp_desc_encap(ptp_net_protocol_t net_protocol)
{
    switch (net_protocol)
    {
    case PTP_NET_PROTO_RESERVED:
        return "Reserved";
    case PTP_NET_PROTO_UDP_IPV4:
    case PTP_NET_PROTO_UDP_IPV4_UNICAST:
        return "UDP/IPv4";
    case PTP_NET_PROTO_UDP_IPV6:
        return "UDP/IPv6";
    case PTP_NET_PROTO_IEEE_802_3:
        return "IEEE 802.3";
    default:
        return "Unknown Protocol";
    }
}

char *
ptp_desc_dev_type(ptp_device_type_t type)
{
    switch (type)
    {
    case PTP_DEVICE_OC:
        return "ordinary clock";
    case PTP_DEVICE_BC:
        return "boundary clock";
    case PTP_DEVICE_E2ETC:
        return "end-to-end transparent clock";
    case PTP_DEVICE_P2PTC:
        return "peer-to-peer transparent clock";

    default:
        return "unknown clock";
    }
}

char *
ptp_desc_two_step(uint32 two_step)
{
    if (two_step)
    {
        return "two-step";
    }
    else
    {
        return "one-step";
    }
}

char *
ptp_desc_delay_mechanism(ptp_delay_mechanism_t delay_mechanism)
{
    switch (delay_mechanism)
    {
    case PTP_DELAY_MECHANISM_E2E:
        return "normal";
    case PTP_DELAY_MECHANISM_P2P:
        return "peer";
    case PTP_DELAY_MECHANISM_DISABLE:
        return "disable";

    default:
        return "unknown";
    }
}

char *
ptp_desc_state(ptp_state_t state)
{
    switch (state)
    {
    case PTP_STATE_INITIALIZING:
        return "initializing";
    case PTP_STATE_FAULTY:
        return "faulty";
    case PTP_STATE_DISABLED:
        return "disabled";
    case PTP_STATE_LISTENING:
        return "listening";
    case PTP_STATE_PRE_MASTER:
        return "pre_master";
    case PTP_STATE_MASTER:
        return "master";
    case PTP_STATE_PASSIVE:
        return "passive";
    case PTP_STATE_UNCALIBRATED:
        return "uncalibrated";
    case PTP_STATE_SLAVE:
        return "slave";

    default:
        return "unknown";
    }
}

uint32
ptp_desc_dev_type_str2val(char *str)
{
    if (!sal_strcmp(str, "oc"))
    {
        return PTP_DEVICE_OC;
    }
    else if (!sal_strcmp(str, "bc"))
    {
        return PTP_DEVICE_BC;
    }
    else if (!sal_strcmp(str, "e2etc"))
    {
        return PTP_DEVICE_E2ETC;
    }
    else if (!sal_strcmp(str, "p2ptc"))
    {
        return PTP_DEVICE_P2PTC;
    }
    else 
    {
        //printf("%s %d, error\n", __func__, __LINE__);
        return PTP_STR_TO_VAL_ERROR;
    }
}

uint32
ptp_desc_delay_mechanism_str2val(char *str)
{
    if (!sal_strcmp(str, "disable"))
    {
        return PTP_DELAY_MECHANISM_DISABLE;
    }
    else if (!sal_strcmp(str, "normal"))
    {
        return PTP_DELAY_MECHANISM_E2E;
    }
    else if (!sal_strcmp(str, "peer"))
    {
        return PTP_DELAY_MECHANISM_P2P;
    }
    else 
    {
        //printf("%s %d, error\n", __func__, __LINE__);
        return PTP_STR_TO_VAL_ERROR;
    }
}


uint32
ptp_desc_time_source_str2val(char *str)
{
    if (!sal_strcmp(str, "atomic-clock"))
    {
        return PTP_TIME_SOURCE_ATOMIC_CLOCK;
    }
    else if (!sal_strcmp(str, "gps"))
    {
        return PTP_TIME_SOURCE_GPS;
    }
    else if (!sal_strcmp(str, "terrestrial-radio"))
    {
        return PTP_TIME_SOURCE_TERRESTRIAL_RADIO;
    }
    else if (!sal_strcmp(str, "ptp"))
    {
        return PTP_TIME_SOURCE_PTP;
    }
    else if (!sal_strcmp(str, "ntp"))
    {
        return PTP_TIME_SOURCE_NTP;
    }
    else if (!sal_strcmp(str, "hand-set"))
    {
        return PTP_TIME_SOURCE_HAND_SET;
    }
    else if (!sal_strcmp(str, "other"))
    {
        return PTP_TIME_SOURCE_OTHER;
    }
    else if (!sal_strcmp(str, "internal-oscillator"))
    {
        return PTP_TIME_SOURCE_INTERNAL_OSCILLATOR;
    }
    else 
    {
        //printf("%s %d, error\n", __func__, __LINE__);
        return PTP_STR_TO_VAL_ERROR;
    }
}

uint32
ptp_desc_two_step_str2val(char *str)
{
    if (!sal_strcmp(str, "one-step"))
    {
        return FALSE;
    }
    else if (!sal_strcmp(str, "two-step"))
    {
        return TRUE;
    }
    else 
    {
        //printf("%s %d, error\n", __func__, __LINE__);
        return PTP_STR_TO_VAL_ERROR;
    }
}

uint32
ptp_desc_statistics_packet_type_str2val(char *str)
{
    if (!sal_strcmp(str, "receive"))
    {
        return PTP_STATS_RX;
    }
    else if (!sal_strcmp(str, "send"))
    {
        return PTP_STATS_TX;
    }
    else if (!sal_strcmp(str, "discard"))
    {
        return PTP_STATS_DROP;
    }
    else 
    {
        //printf("%s %d, error\n", __func__, __LINE__);
        return PTP_STR_TO_VAL_ERROR;
    }
}

