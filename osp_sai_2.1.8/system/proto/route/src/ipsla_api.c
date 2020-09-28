
#include <linux/icmp.h>
#include <linux/filter.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "proto.h"
#include "sal.h"
#include "sal_types.h"
#include "glb_ip_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_ipsla_global_define.h"
#include "gen/tbl_ipsla_global.h"
#include "gen/tbl_ipsla_entry_define.h"
#include "gen/tbl_ipsla_entry.h"
#include "gen/tbl_ipsla_packet_stats_define.h"
#include "gen/tbl_ipsla_packet_stats.h"
#include "gen/tbl_ipsla_test_stats_define.h"
#include "gen/tbl_ipsla_test_stats.h"
#include "gen/tbl_route_debug_define.h"
#include "gen/tbl_route_debug.h"
#include "ipsla.h"
#include "ipsla_api.h"


uint32
ipsla_debug_is_on(uint32 flag)
{
    tbl_route_debug_t *p_db_glb = NULL;

    p_db_glb = tbl_route_debug_get_route_debug();
    if (p_db_glb)
    {
        if (GLB_FLAG_ISSET(p_db_glb->ipsla, flag))
        {
            return TRUE;
        }
    }

    return FALSE;
}

int32
ipsla_check_ip(struct sal_in4_addr *addr)
{
    if (IN_BADCLASS(sal_ntoh32(addr->s_addr))
        || IN_MULTICAST(sal_ntoh32(addr->s_addr))
        || IN_LOOPBACK(sal_ntoh32(addr->s_addr))
        || IPV4_NET0(sal_ntoh32(addr->s_addr))
        || INADDR_BROADCAST == addr->s_addr
        || INADDR_ANY == addr->s_addr)
    {
        return IPSLA_ENTRY_INPUT_IPADDRESS_ERR;
    }

    return IPSLA_SUCCESS;
}

uint16
ipsla_cal_chksum(uint16 *addr,int32 len)
{
    int32 nleft = len;
    int32 sum   = 0;
    uint16 *w = addr;
    uint16 answer = 0;

    /* added icmp header, unit is 2 bytes */
    while (nleft > 1)
    {
        sum   += *w++;
        nleft -= 2;
    }

    /* if ICMP header is odd number, use the last byte as the largest byte of two bytes, the lowest byte set zero*/
    if (nleft == 1)
    {
        *(uint8 *)(&answer) = *(uint8 *)w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return answer;
}

int32
ipsla_adjust_probe_result(tbl_ipsla_entry_t *p_db_entry, int32 config_cnt)
{
    int32 rc     = PM_E_NONE;
    tbl_ipsla_packet_stats_t  ipsla_pstats;
    tbl_ipsla_packet_stats_t *p_db_ipsla_pstats = NULL;
    int32 packet_index = p_db_entry->probe_result_index - 1;
    int32 packet_del_index = 0;

    if (p_db_entry->probe_result_config_cnt > config_cnt)
    {
        if (packet_index > config_cnt)
        {
            int32 packet_del_count = p_db_entry->probe_result_config_cnt - config_cnt;
            for (packet_del_index = packet_index - config_cnt; packet_del_index > 0 && packet_del_count > 0;
                 packet_del_index--, packet_del_count--)
            {
                ipsla_pstats.key.entry_id = p_db_entry->key.entry_id;
                ipsla_pstats.key.packet_index = packet_del_index;
                p_db_ipsla_pstats = tbl_ipsla_packet_stats_get_ipsla_packet_stats(&ipsla_pstats.key);
                if (p_db_ipsla_pstats)
                {
                    tbl_ipsla_packet_stats_del_ipsla_packet_stats(&ipsla_pstats.key);
                }
            }
        }
        else
        {
            /* packet_index <= value, no need to del packet stats */
        }
    }
    else
    {
        /* p_db_entry->probe_result_config_cnt < config_cnt, do nothing */
    }

    return rc;
}

int32
ipsla_adjust_test_result(tbl_ipsla_entry_t *p_db_entry, int32 config_cnt)
{
    int32 rc     = PM_E_NONE;
    tbl_ipsla_test_stats_t  ipsla_tstats;
    tbl_ipsla_test_stats_t *p_db_ipsla_tstats = NULL;
    int32 test_index = p_db_entry->test_result_index - 1;
    int32 test_del_index = 0;

    if (p_db_entry->probe_result_config_cnt > config_cnt)
    {
        if (test_index > config_cnt)
        {
            int32 packet_del_count = p_db_entry->probe_result_config_cnt - config_cnt;
            for (test_del_index = test_index - config_cnt; test_del_index > 0 && packet_del_count > 0;
                 test_del_index--, packet_del_count--)
            {
                ipsla_tstats.key.entry_id = p_db_entry->key.entry_id;
                ipsla_tstats.key.test_index = test_del_index;
                p_db_ipsla_tstats = tbl_ipsla_test_stats_get_ipsla_test_stats(&ipsla_tstats.key);
                if (p_db_ipsla_tstats)
                {
                    tbl_ipsla_test_stats_del_ipsla_test_stats(&ipsla_tstats.key);
                }
            }
        }
        else
        {
            /* test_index <= value, no need to del test stats */
        }
    }
    else
    {
        /* p_db_entry->probe_result_config_cnt < config_cnt, do nothing */
    }

    return rc;
}

void
ipsla_process_probe_result(tbl_ipsla_entry_t *p_db_entry)
{
    tbl_ipsla_global_t       *p_db_ipsla_glb = NULL;
    tbl_ipsla_packet_stats_t  ipsla_pstats;
    tbl_ipsla_packet_stats_key_t  ipsla_pstats_del;
    tbl_ipsla_packet_stats_t *p_db_ipsla_pstats = NULL;

    sal_memset(&ipsla_pstats, 0, sizeof(ipsla_pstats));
    ipsla_pstats.key.entry_id = p_db_entry->key.entry_id;
    ipsla_pstats.key.packet_index = p_db_entry->probe_result_index;

    ipsla_pstats.send_time = p_db_entry->send_time;
    ipsla_pstats.testid    = p_db_entry->testid;
    ipsla_pstats.index     = p_db_entry->packets_per_test - p_db_entry->running_probe_cnt;
    ipsla_pstats.rtt       = p_db_entry->rtt;
    ipsla_pstats.status    = p_db_entry->echo_reply;
    ipsla_pstats.timeout   = p_db_entry->timeout;
    ipsla_pstats.threshold = p_db_entry->threshold;

    char *ipsla_echo_replay_code_str[IPSLA_ECHO_REPLY_CODE_MAX] = {
        "Timeout", "Success", "Overthreshold", "SendFail"
    };

    IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_EVENT, "IPSLA Event: Result of entry %d test %d index %d is %s\n",
            p_db_entry->key.entry_id, p_db_entry->testid, p_db_entry->packets_per_test - p_db_entry->running_probe_cnt, 
            ipsla_echo_replay_code_str[p_db_entry->echo_reply]);

    if ((IPSLA_GET_ECHO_REPLY == p_db_entry->echo_reply) ||
        (IPSLA_GET_ECHO_OVERTHRESHOLD == p_db_entry->echo_reply))
    {
        p_db_entry->total_receive++;
    }

    p_db_entry->rtt_sum += p_db_entry->rtt;

    if ((p_db_entry->rtt > p_db_entry->rtt_max) || (0 == p_db_entry->rtt_max))
    {
        p_db_entry->rtt_max = p_db_entry->rtt;
    }

    if ((p_db_entry->rtt < p_db_entry->rtt_min) || (0 == p_db_entry->rtt_min))
    {
        p_db_entry->rtt_min = p_db_entry->rtt;
    }

    p_db_ipsla_glb = tbl_ipsla_global_get_ipsla_global();

    if (p_db_ipsla_glb->trap_pkt_timeout
        && (IPSLA_NO_ECHO_REPLY == p_db_entry->echo_reply
        || IPSLA_GET_ECHO_FAILED == p_db_entry->echo_reply))
    {
        /* TODO FOR SNMP trap */
        //ipsla_snmp_trap_pkt_timeout(p_db_entry, &result);

        IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_EVENT, "IPSLA Event: Send packet-timeout trap for entry %d test %d index %d\n",
                p_db_entry->key.entry_id, p_db_entry->testid, p_db_entry->packets_per_test - p_db_entry->running_probe_cnt);
    }

    if (p_db_ipsla_glb->trap_overthreshold &&
        IPSLA_GET_ECHO_OVERTHRESHOLD == p_db_entry->echo_reply)
    {
        //ipsla_snmp_trap_pkt_overthreshold(p_db_entry, &result);

        IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_EVENT, "IPSLA Event: Send packet-overthreshold trap for entry %d test %d index %d\n",
                p_db_entry->key.entry_id, p_db_entry->testid, p_db_entry->packets_per_test - p_db_entry->running_probe_cnt);
    }

    if(p_db_entry->probe_result_index <= p_db_entry->probe_result_config_cnt)
    {
        p_db_ipsla_pstats = tbl_ipsla_packet_stats_get_ipsla_packet_stats(&ipsla_pstats.key);
        if (p_db_ipsla_pstats)
        {
            tbl_ipsla_packet_stats_del_ipsla_packet_stats(&ipsla_pstats.key);
            tbl_ipsla_packet_stats_add_ipsla_packet_stats(&ipsla_pstats);
        }
        else
        {
            tbl_ipsla_packet_stats_add_ipsla_packet_stats(&ipsla_pstats);
        }
    }
    else
    {
        sal_memset(&ipsla_pstats_del, 0, sizeof(ipsla_pstats_del));
        ipsla_pstats_del.entry_id = p_db_entry->key.entry_id;
        ipsla_pstats_del.packet_index = p_db_entry->probe_result_index - p_db_entry->probe_result_config_cnt;

        p_db_ipsla_pstats = tbl_ipsla_packet_stats_get_ipsla_packet_stats(&ipsla_pstats_del);
        if (p_db_ipsla_pstats)
        {
             tbl_ipsla_packet_stats_del_ipsla_packet_stats(&ipsla_pstats_del);
        }

        tbl_ipsla_packet_stats_add_ipsla_packet_stats(&ipsla_pstats);
    }
    p_db_entry->probe_result_index++;

    p_db_entry->last_send_time = p_db_entry->send_time;
    if (p_db_entry->rtt || (IPSLA_GET_ECHO_REPLY == p_db_entry->echo_reply)
                        || (IPSLA_GET_ECHO_OVERTHRESHOLD == p_db_entry->echo_reply))
    {
        p_db_entry->running_rtt_sum += p_db_entry->rtt;
        p_db_entry->receive_cnt++;

        if (IPSLA_GET_ECHO_OVERTHRESHOLD == p_db_entry->echo_reply)
            p_db_entry->overthreshold_cnt++;

        if ((p_db_entry->rtt > p_db_entry->running_rtt_max) || (0 == p_db_entry->running_rtt_max))
            p_db_entry->running_rtt_max = p_db_entry->rtt;

        if ((p_db_entry->rtt < p_db_entry->running_rtt_min) || (0 == p_db_entry->running_rtt_min))
            p_db_entry->running_rtt_min = p_db_entry->rtt;
    }
    else
    {
        if (IPSLA_GET_ECHO_FAILED == p_db_entry->echo_reply)
            p_db_entry->sendfail_cnt++;
    }

    return;
}

int32
ipsla_save_test_result(tbl_ipsla_entry_t *p_db_entry)
{
    int32 rc     = PM_E_NONE;
    tbl_ipsla_test_stats_t  ipsla_tstats;
    tbl_ipsla_test_stats_key_t  ipsla_tstats_del;
    tbl_ipsla_test_stats_t *p_db_ipsla_tstats = NULL;

    sal_memset(&ipsla_tstats, 0, sizeof(ipsla_tstats));
    ipsla_tstats.key.entry_id   = p_db_entry->key.entry_id;
    ipsla_tstats.key.test_index = p_db_entry->test_result_index;
    ipsla_tstats.last_send_time = p_db_entry->last_send_time;
    ipsla_tstats.testid  = p_db_entry->testid;
    ipsla_tstats.send_cnt     = p_db_entry->send_cnt;
    ipsla_tstats.receive_cnt  = p_db_entry->receive_cnt;
    ipsla_tstats.sendfail_cnt = p_db_entry->sendfail_cnt;
    ipsla_tstats.fail_percent = p_db_entry->running_fail_percent;
    ipsla_tstats.rtt_sum = p_db_entry->running_rtt_sum;
    ipsla_tstats.rtt_max = p_db_entry->running_rtt_max;
    ipsla_tstats.rtt_min = p_db_entry->running_rtt_min;
    ipsla_tstats.success = p_db_entry->running_success;
    ipsla_tstats.timeout = p_db_entry->running_timeout;
    ipsla_tstats.overthreshold_cnt = p_db_entry->overthreshold_cnt;

    if(p_db_entry->test_result_index <= p_db_entry->probe_result_config_cnt)
    {
        p_db_ipsla_tstats = tbl_ipsla_test_stats_get_ipsla_test_stats(&ipsla_tstats.key);
        if (p_db_ipsla_tstats)
        {
            tbl_ipsla_test_stats_del_ipsla_test_stats(&p_db_ipsla_tstats->key);
        }

        tbl_ipsla_test_stats_add_ipsla_test_stats(&ipsla_tstats);
    }
    else
    {
        sal_memset(&ipsla_tstats_del, 0, sizeof(ipsla_tstats_del));
        ipsla_tstats_del.entry_id = p_db_entry->key.entry_id;
        ipsla_tstats_del.test_index = p_db_entry->test_result_index - p_db_entry->test_result_config_cnt;

        p_db_ipsla_tstats = tbl_ipsla_test_stats_get_ipsla_test_stats(&ipsla_tstats_del);
        if(p_db_ipsla_tstats)
        {
            tbl_ipsla_test_stats_del_ipsla_test_stats(&ipsla_tstats_del);
        }

        tbl_ipsla_test_stats_add_ipsla_test_stats(&ipsla_tstats);
    }
    p_db_entry->test_result_index++;

    return rc;
}

int32
ipsla_send_icmp_packet(tbl_ipsla_entry_t *p_db_entry)
{
    struct sockaddr_in dest_addr;
    int32    ret;
    struct icmphdr *icmp_p;
    struct sal_timeval  tv;

    sal_gettimeofday(&tv);

    if (NULL == p_db_entry->buf)
    {
        IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_PACKET, "IPSLA Packet:(SEND) send echo packet failure since no icmp packet for entry %d test %d index %d\n",
            p_db_entry->key.entry_id, p_db_entry->testid, p_db_entry->packets_per_test - p_db_entry->running_probe_cnt + 1);

        p_db_entry->send_time = (uint64)(tv.tv_sec) * 1000 + (uint64)(tv.tv_usec) / 1000;
        return IPSLA_PING_SEND_FAILURE;
    }

    if (p_db_entry->sock_flag)
    {
        if (GLB_FLAG_ISSET(p_db_entry->sock_flag, IPSLA_SOCK_FLAG_ERR_BIND_DEVICE)
            && p_db_entry->src_ifname)
        {
            IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_PACKET, "IPSLA Packet:(SEND) send echo packet failure for interface %s entry %d test %d index %d\n",
                p_db_entry->src_ifname, p_db_entry->key.entry_id, p_db_entry->testid, p_db_entry->packets_per_test - p_db_entry->running_probe_cnt + 1);
        }
        else
        {
            IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_PACKET, "IPSLA Packet:(SEND) send echo packet failure since sock error %d entry %d test %d index %d\n",
                p_db_entry->sock_flag, p_db_entry->key.entry_id, p_db_entry->testid, p_db_entry->packets_per_test - p_db_entry->running_probe_cnt + 1);
        }

        p_db_entry->send_time = (uint64)(tv.tv_sec) * 1000 + (uint64)(tv.tv_usec) / 1000;
        return IPSLA_PING_SEND_FAILURE;
    }

    if (GLB_FLAG_ISSET(p_db_entry->sock_flag, IPSLA_SOCK_FLAG_ERR_BIND_SRCIP))
    {
        struct sockaddr_in source = { .sin_family = AF_INET };
        sal_memcpy(&source.sin_addr, &p_db_entry->src_inaddr.s_addr, sizeof(source.sin_addr));
        if (sal_bind(p_db_entry->send_sock, (struct sockaddr *) &source, sizeof(source)) == -1) 
        {
            p_db_entry->send_time = (uint64)(tv.tv_sec) * 1000 + (uint64)(tv.tv_usec) / 1000;
            return IPSLA_PING_SEND_FAILURE;
        }
        else
        {
            GLB_UNSET_FLAG(p_db_entry->sock_flag, IPSLA_SOCK_FLAG_ERR_BIND_SRCIP);
        }
    }

    sal_memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    sal_memcpy((char *)&dest_addr.sin_addr, (char *)&p_db_entry->inaddr, sizeof(p_db_entry->inaddr));

    /* icmp header */
    icmp_p = (struct icmphdr *)(p_db_entry->buf);
    (icmp_p->un).echo.sequence = sal_hton16(p_db_entry->sequence);

    /* set send time */
    sal_memcpy(icmp_p+1, &tv, sizeof(tv));

    /* checksum */
    icmp_p->checksum = 0;
    icmp_p->checksum = ipsla_cal_chksum((uint16 *)icmp_p, p_db_entry->data_size + sizeof (struct icmphdr) + 8);

    ret = sendto(p_db_entry->send_sock, p_db_entry->buf, p_db_entry->packet_length,
                 0, (struct sockaddr *)&dest_addr,sizeof(dest_addr));

    p_db_entry->send_time = (uint64)(tv.tv_sec) * 1000 + (uint64)(tv.tv_usec) / 1000;

    if (ret <0)
    {
        IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_PACKET, "IPSLA Packet:(SEND) send echo packet failure for ipsla entry %d test %u index %u\n",
                    p_db_entry->key.entry_id, p_db_entry->testid, p_db_entry->packets_per_test- p_db_entry->running_probe_cnt + 1);
        return IPSLA_PING_SEND_FAILURE;
    }

    IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_PACKET, "IPSLA Packet:(SEND) send echo packet for ipsla entry %d test %u index %u\n",
                    p_db_entry->key.entry_id, p_db_entry->testid, p_db_entry->packets_per_test - p_db_entry->running_probe_cnt + 1);

    return IPSLA_PING_SUCCESS;
}

int32
ipsla_probe_start(tbl_ipsla_entry_t *p_db_entry)
{
    int32 rc     = PM_E_NONE;
    int32 result = 0;

    if (p_db_entry->running_interval
        || p_db_entry->running_timeout
        || !p_db_entry->running_probe_cnt)
    {
        IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_PACKET, "IPSLA Event: cant start probe for ipsla entry %d since interval %u timeout %u count %u\n",
            p_db_entry->key.entry_id, p_db_entry->running_interval, p_db_entry->running_timeout, p_db_entry->running_probe_cnt);
        return rc;
    }

    /* send icmp packet */
    result = ipsla_send_icmp_packet(p_db_entry);
    if (result == IPSLA_PING_SEND_FAILURE)
    {
        p_db_entry->echo_reply = IPSLA_GET_ECHO_FAILED;
    }

    p_db_entry->total_send++;
    p_db_entry->running_probe_cnt--;
    p_db_entry->send_cnt++;
    p_db_entry->running_timeout = p_db_entry->timeout;

    if (p_db_entry->running_probe_cnt)
    {
        p_db_entry->running_interval = p_db_entry->interval;
    }

    return rc;
}

void
ipsla_update_entry_status(tbl_ipsla_entry_t *p_db_entry)
{
    tbl_ipsla_global_t   *p_db_ipsla_glb = NULL;

    p_db_ipsla_glb = tbl_ipsla_global_get_ipsla_global();

    if (p_db_entry->running_timeout)
    {
        if (p_db_entry->echo_reply)
        {
            p_db_entry->running_timeout = 0;
        }
        else
        {
            p_db_entry->running_timeout--;
        }

        /* process result */
        if (!p_db_entry->running_timeout)
        {
            ipsla_process_probe_result(p_db_entry);

            if (!p_db_entry->running_probe_cnt)
            {
                /* set return code */
                p_db_entry->running_fail_percent = ((p_db_entry->send_cnt
                    - p_db_entry->receive_cnt) * 100) / p_db_entry->send_cnt;

                if (p_db_entry->running_fail_percent < p_db_entry->fail_percent)
                {
                    IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_EVENT, "IPSLA Event : entry %d test %d OK\n",
                                    p_db_entry->key.entry_id, p_db_entry->testid);
                    if (ECHO_TIMEOUT == p_db_entry->returncode)
                    {
                        logid_sys(LOG_IPSLA_4_IPSLA_EVENT, p_db_entry->key.entry_id, p_db_entry->testid,
                                  "fail", "success");
                    }
                    p_db_entry->returncode = ECHO_OK;
                    p_db_entry->running_success = 1;
                }
                else
                {
                    IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_EVENT, "IPSLA Event : entry %d test %d Timeout\n",
                                    p_db_entry->key.entry_id, p_db_entry->testid);
                    if (ECHO_OK == p_db_entry->returncode)
                    {
                        logid_sys(LOG_IPSLA_4_IPSLA_EVENT, p_db_entry->key.entry_id, p_db_entry->testid,
                                  "success", "fail");
                    }
                    p_db_entry->returncode = ECHO_TIMEOUT;
                    p_db_entry->running_success = 0;
                }

                if ((p_db_ipsla_glb->trap_test_failure && !p_db_entry->running_success)
                    || (p_db_ipsla_glb->trap_test_success && p_db_entry->running_success))
                {
                    /* YUZGTODO FOR SNMP trap */
                    //ipsla_snmp_trap_test(p_db_entry, &p_db_entry->running_test);

                    IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_EVENT, "IPSLA Event: Send %s trap for entry %d test %d\n",
                            p_db_entry->running_success ? "test-success" : "test-failure",
                            p_db_entry->key.entry_id,  p_db_entry->testid);
                }

                /* save to test result tbl */
                ipsla_save_test_result(p_db_entry);
            }
            else
            {
                p_db_entry->running_fail_percent = ((p_db_entry->send_cnt
                    - p_db_entry->receive_cnt) * 100) / p_db_entry->packets_per_test;

                if (p_db_entry->running_fail_percent >= p_db_entry->fail_percent)
                {
                    if (ECHO_OK == p_db_entry->returncode)
                    {
                        logid_sys(LOG_IPSLA_4_IPSLA_EVENT, p_db_entry->key.entry_id, p_db_entry->testid,
                                  "success", "fail");
                    }
                    p_db_entry->returncode = ECHO_TIMEOUT;
                }
            }

            p_db_entry->sequence++;
            p_db_entry->echo_reply = 0;
            p_db_entry->rtt = 0;
        }
    }

    if (p_db_entry->running_interval)
        p_db_entry->running_interval--;

    if (!p_db_entry->running_interval && p_db_entry->running_probe_cnt)
    {
        /* start next probe */
        ipsla_probe_start(p_db_entry);
    }

    if (p_db_entry->running_frequency != 0)
        p_db_entry->running_frequency--;

    if (p_db_entry->running_frequency == 0)
    {
        /* start next test */
        p_db_entry->testid++;

        p_db_entry->running_frequency = p_db_entry->frequency;
        p_db_entry->running_probe_cnt = p_db_entry->packets_per_test;

        p_db_entry->send_cnt          = 0;
        p_db_entry->receive_cnt       = 0;
        p_db_entry->overthreshold_cnt = 0;
        p_db_entry->running_rtt_min   = 0;
        p_db_entry->running_rtt_max   = 0;
        p_db_entry->running_rtt_sum   = 0;
        p_db_entry->running_success   = 0;
        p_db_entry->running_fail_percent = 0;

        ipsla_probe_start(p_db_entry);
    }

    return;
}

void
ipsla_timer(tbl_ipsla_global_t *p_db_ipsla_glb)
{
    uint32     entry_id      = 0;
    tbl_ipsla_entry_t *p_db_entry = NULL;
    tbl_ipsla_entry_master_t *p_master  = NULL;
    uint16     ipsla_entry_num = p_db_ipsla_glb->ipsla_entry_num;

    p_master = tbl_ipsla_entry_get_master();
    if (!p_master)
    {
        IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_EVENT, "Tbl_ipsla_entry is not exist");
        return;
    }

    /* Loop through all entries */
    for (entry_id = 1; entry_id <= GLB_IPSLA_ENTRY_MAX && ipsla_entry_num > 0; entry_id++)
    {
        p_db_entry = p_master->ipslaentry_array[entry_id];
        if (!p_db_entry)
        {
            continue;
        }
        else
        {
            ipsla_entry_num--;
            if (p_db_entry->admin_state == IPSLA_DISABLE)
            {
                continue;
            }
            else
            {
                ipsla_update_entry_status(p_db_entry);
            }
        }
    }

    return;
}

void
ipsla_sys_timer_update(void *p_data)
{
    tbl_ipsla_global_t *p_db_ipsla_glb = (tbl_ipsla_global_t*)p_data;

    if (!p_db_ipsla_glb)
    {
        IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_EVENT, "Get p_db_ipsla_glb wrong\n");
        return;
    }

    p_db_ipsla_glb->t_ipsla_timer = NULL;

    ipsla_timer(p_db_ipsla_glb);

    p_db_ipsla_glb->t_ipsla_timer = CTC_TASK_ADD_TIME(ipsla_sys_timer_update, p_db_ipsla_glb, 1);

    return;
}

int32
ipsla_sys_init_timer(tbl_ipsla_global_t *p_db_ipsla_glb, bool start)
{
    int32 interval = 1;

    if (NULL == p_db_ipsla_glb)
    {
        return PM_E_INVALID_PARAM;
    }

    CTC_TASK_STOP_TIMER(p_db_ipsla_glb->t_ipsla_timer);

    if (start)
    {
        p_db_ipsla_glb->t_ipsla_timer = CTC_TASK_ADD_TIME(ipsla_sys_timer_update, p_db_ipsla_glb, interval);
    }

    return PM_E_NONE;
}

int32
ipsla_enable_entry(tbl_ipsla_entry_t *p_db_entry)
{
    int32  rc = PM_E_NONE;

    if ((p_db_entry->timeout >= p_db_entry->interval)
         || (p_db_entry->interval >= p_db_entry->frequency)
         || (p_db_entry->frequency <= p_db_entry->interval
          * (p_db_entry->packets_per_test - 1) + p_db_entry->timeout)
         || (p_db_entry->timeout > p_db_entry->frequency))
    {
        p_db_entry->frequency = GLB_IPSLA_FREQUENCY_DEFAULT;
        p_db_entry->timeout   = GLB_IPSLA_TIMEOUT_DEFAULT;
        p_db_entry->threshold = GLB_IPSLA_THRESHOLD_DEFAULT;
        p_db_entry->interval  = GLB_IPSLA_INTERVAL_DEFAULT;
        p_db_entry->packets_per_test = GLB_IPSLA_PING_PROBE_COUNT_DEFAULT;
    }

    if (!p_db_entry->t_ipsla_read)
    {
        rc = ipsla_create_socket(p_db_entry);
        if (rc != IPSLA_SUCCESS)
        {
            p_db_entry->returncode = ECHO_UNREACHABLE;
            return rc;
        }

        p_db_entry->t_ipsla_read = CTC_TASK_ADD_READ(ipsla_ip_recv_packet, p_db_entry, p_db_entry->sock);
    }

    return rc;
}

int32
ipsla_disable_entry(tbl_ipsla_entry_t *p_db_entry)
{
    int32  rc = PM_E_NONE;

    p_db_entry->admin_state = IPSLA_DISABLE;
    p_db_entry->running_frequency = 0;
    p_db_entry->running_timeout =0;
    p_db_entry->running_interval =0;
    p_db_entry->running_probe_cnt = 0;
    p_db_entry->returncode = ECHO_UNKNOWN;

    if (p_db_entry->t_ipsla_read)
    {
        /* cancel read socket */
        CTC_TASK_DEL_READ(p_db_entry->t_ipsla_read);

        /* cleanup the socket. */
        sal_shutdown(p_db_entry->sock, SHUT_WR);
        sal_close(p_db_entry->sock);

        if (p_db_entry->send_sock != p_db_entry->sock)
        {
            sal_shutdown(p_db_entry->send_sock, SHUT_WR);
            sal_close(p_db_entry->send_sock);

            p_db_entry->send_sock = 0;
        }
        p_db_entry->sock = 0;

        sal_memset(p_db_entry->buf, 0, sizeof(p_db_entry->buf));
    }

    return rc;
}

int32
ipsla_process_src_intf_delete(tbl_ipsla_entry_t *p_db_entry)
{
    int32   rc  = IPSLA_SUCCESS;

    sal_memset(p_db_entry->src_ifname, 0, IFNAME_SIZE);
    PM_E_RETURN(tbl_ipsla_entry_set_ipsla_entry_field(p_db_entry, TBL_IPSLA_ENTRY_FLD_SRC_IFNAME));

    if(IPSLA_ENABLE == p_db_entry->admin_state)
    {
        PM_E_RETURN(ipsla_disable_entry(p_db_entry));
        if (p_db_entry->buf)
        {
            sal_memset(p_db_entry->buf, 0, sizeof(p_db_entry->buf));
        }

        /* restart ipsla schedule */
        rc = ipsla_create_socket(p_db_entry);
        if(rc != IPSLA_SUCCESS)
        {
            return rc;
        }

        p_db_entry->t_ipsla_read = CTC_TASK_ADD_READ(ipsla_ip_recv_packet, p_db_entry, p_db_entry->sock);
        p_db_entry->admin_state = IPSLA_ENABLE;
    }

    return rc;
}

int32
ipsla_update_src_intf_delete(tbl_interface_t *p_db_if)
{
    int32     rc       = PM_E_NONE;
    uint32    entry_id = 0;
    tbl_ipsla_entry_master_t *p_master  = NULL;
    tbl_ipsla_entry_t        *p_db_entry = NULL;
    tbl_ipsla_global_t       *p_db_ipsla_glb = NULL;

    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type
     && GLB_IF_TYPE_VLAN_IF != p_db_if->hw_type
     && GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
        return PM_E_NONE;
    }

    p_db_ipsla_glb = tbl_ipsla_global_get_ipsla_global();
    if (!p_db_ipsla_glb)
    {
        return PM_E_NONE;
    }
    uint16 ipsla_entry_num = p_db_ipsla_glb->ipsla_entry_num;

    p_master = tbl_ipsla_entry_get_master();
    if (!p_master)
    {
        return PM_E_NONE;
    }

    for (entry_id = 1; entry_id <= GLB_IPSLA_ENTRY_MAX && ipsla_entry_num > 0; entry_id++)
    {
        p_db_entry = p_master->ipslaentry_array[entry_id];
        if (!p_db_entry)
        {
            continue;
        }

        if(!sal_strncmp(p_db_if->key.name, p_db_entry->src_ifname, IFNAME_SIZE))
        {
            rc = ipsla_process_src_intf_delete(p_db_entry);
        }
        ipsla_entry_num--;
    }

    return rc;
}

int32
ipsla_packet_new(tbl_ipsla_entry_t *p_db_entry)
{
    struct icmphdr *icmp_p = NULL;;
    int32   hdrsize;
    int32   datasize;
    uint8  *data_p = NULL;

    if (p_db_entry->buf)
    {
        sal_memset(p_db_entry->buf, 0, sizeof(p_db_entry->buf));
    }

    hdrsize = sizeof(struct icmphdr) + 8;   /* icmphdr + timestamp */
    p_db_entry->packet_length = p_db_entry->data_size + hdrsize;
    datasize = (p_db_entry->data_size % 4) ? (p_db_entry->data_size / 4 + 1) * 4 : p_db_entry->data_size;

    /* icmp header*/
    icmp_p = (struct icmphdr *)(p_db_entry->buf);
    icmp_p->type = ICMP_ECHO;
    icmp_p->code =0;
    (icmp_p->un).echo.id = sal_hton16(p_db_entry->key.entry_id);

    /* construct data skip timestamp 8 bytes */
    data_p = (uint8*)(icmp_p + 1) + 8;

    int32 i = 0;
    for (i = 0; i < datasize - 3; i += 4)
    {
        data_p[i] = p_db_entry->data_pattern >> 24;
        data_p[i + 1] = (p_db_entry->data_pattern >> 16) & 0xFF;
        data_p[i + 2] = (p_db_entry->data_pattern >> 8) & 0xFF;
        data_p[i + 3] = p_db_entry->data_pattern & 0xFF;
    }

    return IPSLA_SUCCESS;
}

int32
ipsla_create_socket(tbl_ipsla_entry_t *p_db_entry)
{
    int32   ret = 0;
    int32   sock;
    int32   identity  = 0;          /* process id to identify our packets */
    int32   send_sock = 0;
    int32   recv_sock = 0;
    struct sock_fprog sock_filter;

    p_db_entry->sock_flag = 0;

    if ((sock = socket(AF_INET, SOCK_RAW, ICMP_TYPE)) < 0)
    {
        GLB_SET_FLAG(p_db_entry->sock_flag, IPSLA_SOCK_FLAG_ERR_CREATE);
        return IPSLA_CREATE_SOCKET_ERR;
    }

    /* icmp filter */
    static struct sock_filter insns[] = {
             BPF_STMT(BPF_LD  | BPF_H   | BPF_ABS, 4),  /* Load icmp echo identity */
             BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 0xAAAA, 0, 1),
             BPF_STMT(BPF_RET | BPF_K,  ~ 0U),
             BPF_STMT(BPF_LD  | BPF_B   | BPF_ABS, 0),  /* Load icmp type */
             BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, ICMP_ECHOREPLY, 1, 0),
             BPF_STMT(BPF_RET | BPF_K,  ~ 0U),
             BPF_STMT(BPF_RET | BPF_K,  0),
    };

    sock_filter.filter = insns;
    sock_filter.len = sizeof(insns) / sizeof(insns[0]);

    identity = p_db_entry->key.entry_id;

    insns[1] = (struct sock_filter)BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, identity, 0, 1);

    if (setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER, &sock_filter, sizeof(sock_filter)) < 0)
    {
        GLB_SET_FLAG(p_db_entry->sock_flag, IPSLA_SOCK_FLAG_ERR_FILTER);
        sal_close(sock);
        return IPSLA_CREATE_SOCKET_ERR;
    }
    recv_sock = sock;

#ifdef SO_TIMESTAMP
    int32 on = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_TIMESTAMP, &on, sizeof(on)) < 0)
    {
        GLB_SET_FLAG(p_db_entry->sock_flag, IPSLA_SOCK_FLAG_ERR_SET_OPTION);
        sal_close(sock);
        return IPSLA_CREATE_SOCKET_ERR;
    }
#endif

#ifdef SO_BINDTODEVICE
    if (strlen(p_db_entry->src_ifname))
    {
        if ((send_sock = socket(AF_INET, SOCK_RAW, ICMP_TYPE)) < 0)
        {
            GLB_SET_FLAG(p_db_entry->sock_flag, IPSLA_SOCK_FLAG_ERR_CREATE);
            sal_close(recv_sock);
            return IPSLA_CREATE_SOCKET_ERR;
        }

        if (strlen(p_db_entry->src_ifname))
        {

            if (setsockopt (send_sock, SOL_SOCKET, SO_BINDTODEVICE, p_db_entry->src_ifname, IFNAME_SIZE) < 0)
            {
                GLB_SET_FLAG(p_db_entry->sock_flag, IPSLA_SOCK_FLAG_ERR_BIND_DEVICE);
            }
        }
    }
#endif

    if (!send_sock)
    {
        send_sock = sock;
    }

    if (p_db_entry->src_inaddr.s_addr)
    {
        struct sockaddr_in source = { .sin_family = AF_INET };
        sal_memcpy(&source.sin_addr, &p_db_entry->src_inaddr.s_addr, sizeof(source.sin_addr));
        if (sal_bind(send_sock, (struct sockaddr *)&source, sizeof(source)) == -1)
        {
            GLB_SET_FLAG(p_db_entry->sock_flag, IPSLA_SOCK_FLAG_ERR_BIND_SRCIP);
        }
    }

    if (p_db_entry->ttl)
    {
        int32 ittl = p_db_entry->ttl;
        if (setsockopt(send_sock, IPPROTO_IP, IP_TTL, &ittl, sizeof(ittl)) < 0)
        {
            GLB_SET_FLAG(p_db_entry->sock_flag, IPSLA_SOCK_FLAG_ERR_SET_OPTION);
        }
    }

    if (p_db_entry->tos)
    {
        int32 settos = p_db_entry->tos;
        if (setsockopt(send_sock, IPPROTO_IP, IP_TOS, (char *)&settos, sizeof(int32)) < 0)
        {
            GLB_SET_FLAG(p_db_entry->sock_flag, IPSLA_SOCK_FLAG_ERR_SET_OPTION);
        }
    }

    ret = ipsla_packet_new(p_db_entry);
    if (ret)
    {
        sal_close(send_sock);
        if (recv_sock != send_sock)
        {
            sal_close(recv_sock);
        }
        return IPSLA_ENTRY_PKT_NEW_ERR;
    }

    p_db_entry->sock = recv_sock;
    p_db_entry->send_sock = send_sock;

    return IPSLA_SUCCESS;
}

void
ipsla_ip_recv_packet(void *p_data)
{
    tbl_ipsla_entry_t *p_db_entry = (tbl_ipsla_entry_t *)p_data;

    char   ibuf[ICMP_PKT_BUF];   /* space for receiving packets */
    struct sal_sockaddr_in4 from;
    int32    len = ICMP_PKT_BUF;
    int32    nbytes;

    int32    iphdrlen;
    struct ip      *ip;
    struct icmphdr *icmp_p;

    int32    sequence_id;
    int32    packetpid;
    char   addrbuf[128];
    char   ans_data[4096];

    struct iovec    iov;
    struct msghdr   msg;
    struct cmsghdr *c;

    struct timeval *recv_timep = NULL;
    struct timeval  recv_time;
    uint64 recv_time_ms = 0;

    sal_memset(&from, 0, sizeof(struct sal_sockaddr_in4));

    iov.iov_base  = ibuf;
    iov.iov_len   = len;
    memset(&msg, 0, sizeof(msg));
    msg.msg_name    = addrbuf;
    msg.msg_namelen = sizeof(addrbuf);
    msg.msg_iov     = &iov;
    msg.msg_iovlen  = 1;
    msg.msg_control = ans_data;
    msg.msg_controllen = sizeof(ans_data);

    nbytes = recvmsg(p_db_entry->sock, &msg, 0);
    if (nbytes < 0)
    {
        return;
    }

    for (c = CMSG_FIRSTHDR(&msg); c; c = CMSG_NXTHDR(&msg, c))
    {
        if (c->cmsg_level != SOL_SOCKET || c->cmsg_type != SO_TIMESTAMP)
            continue;

        if (c->cmsg_len < CMSG_LEN(sizeof(struct timeval)))
            continue;

        recv_timep = (struct timeval*)CMSG_DATA(c);
    }

    if (recv_timep == NULL)
    {
        if (ioctl(p_db_entry->sock, SIOCGSTAMP, &recv_time) < 0)
        {
            gettimeofday(&recv_time, NULL);
        }

        recv_timep = &recv_time;
    }

    /* process ip header and icmp header */
    ip = (struct ip *)msg.msg_iov->iov_base;
    iphdrlen = ip->ip_hl << 2;
    icmp_p   = (struct icmphdr *)(msg.msg_iov->iov_base + iphdrlen);

    /* check length */
    len -= iphdrlen;
    if (len < 8)
    {
        IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_PACKET, "IPSLA Packet:(RECV) receive echo packet length is less than 8 for ipsla entry %d\n",
                    p_db_entry->key.entry_id);
        return;
    }

    /* check type */
    if (icmp_p->type != ICMP_ECHOREPLY)
    {
        IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_PACKET, "IPSLA Packet:(RECV) receive echo packet type is %d not echoreply for ipsla entry %d\n",
                    icmp_p->type, p_db_entry->key.entry_id);
        return;
    }

    /* check pid and entry id */
    packetpid = sal_ntoh16(*(uint16 *)&(icmp_p->un).echo.id);
    if (packetpid == p_db_entry->key.entry_id)
    {
        sequence_id = sal_ntoh16(*(uint16 *)&(icmp_p->un).echo.sequence);
        if (sequence_id != p_db_entry->sequence)
        {
            IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_PACKET, "IPSLA Packet:(RECV) entry %d receive echo packet with sequence %d, current sequence is %d\n",
                    p_db_entry->key.entry_id, sequence_id, p_db_entry->sequence);
            return;
        }
    }
    else
    {
        IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_PACKET, "IPSLA Packet:(RECV) entry %d receive echo packet id is %d\n",
                    p_db_entry->key.entry_id, packetpid);
        return;
    }

    if (IPSLA_GET_ECHO_REPLY == p_db_entry->echo_reply
    || IPSLA_GET_ECHO_OVERTHRESHOLD == p_db_entry->echo_reply)
    {
        IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_PACKET, "IPSLA Packet:(RECV) entry %d receive dup echo packet with sequence %d\n",
                p_db_entry->key.entry_id, sequence_id);
        return;
    }
    else
    {
        recv_time_ms = (uint64)(recv_timep->tv_sec) * 1000 + (uint64)(recv_timep->tv_usec) / 1000;
        p_db_entry->rtt = recv_time_ms - p_db_entry->send_time;

        if (p_db_entry->rtt < p_db_entry->threshold)
        {
            p_db_entry->echo_reply = IPSLA_GET_ECHO_REPLY;
        }
        else
        {
            p_db_entry->echo_reply = IPSLA_GET_ECHO_OVERTHRESHOLD;
        }

        IPSLA_LOG_DEBUG(RTDBG_FLAG_IPSLA_PACKET, "IPSLA Packet:(RECV) receive echo reply for ipsla entry %d test %d index %d sequence %d rtt %ums\n",
                p_db_entry->key.entry_id, p_db_entry->testid, p_db_entry->packets_per_test - p_db_entry->running_probe_cnt, sequence_id, p_db_entry->rtt);
    }

    return;
}

int32
ipsla_start()
{
    cdb_register_cfg_tbl_cb(TBL_IPSLA_GLOBAL, ipsla_cmd_process_global);
    cdb_register_cfg_tbl_cb(TBL_IPSLA_ENTRY, ipsla_cmd_process_entry);

    cdb_register_cfg_act_cb(ACT_SHOW_IPSLA, ipsla_cmd_show_ipsla);
    cdb_register_cfg_act_cb(ACT_CLEAR_IPSLA_STATS, ipsla_cmd_process_act_clear);

    return PM_E_NONE;
}


