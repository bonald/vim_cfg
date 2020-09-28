/**
 @file ctc_stress_test_cli.c

 @date 2017-03-21

 @version v1.0

 The file applies cli to test sdk module of SDK
*/
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctc_error.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_stress_test_cli.h"
#include "ctc_packet.h"

struct ctc_cli_stress_test_misc_s
{
    uint32 stats_entry_num;
    uint32* stats_id_list;
};
typedef struct ctc_cli_stress_test_misc_s ctc_cli_stress_test_misc_t;

ctc_cli_stress_test_misc_t *g_ctc_stress_misc = NULL;

#if (FEATURE_MODE == 0)
int32
_ctc_cli_stress_wlan_tunnel(ctc_wlan_tunnel_t* p_tunnel_param, uint32 num, uint8 op, uint8 type)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0,valid_count =0;
    sal_systime_t tv1,tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == type)
    {
        sal_gettime(&tv1);
    }


    p_tunnel_param->ipsa.ipv4 = 0x0a0a0a01;
    p_tunnel_param->ipda.ipv4 = 0xc0a80101;
    p_tunnel_param->l4_port = 5000;

    if(0 != p_tunnel_param->mode)
    {
        p_tunnel_param->mode = 2;

        p_tunnel_param->bssid[0] = 0;
        p_tunnel_param->bssid[1] = 0;
        p_tunnel_param->bssid[2] = 0;
        p_tunnel_param->bssid[3] = 0;
        p_tunnel_param->bssid[4] = 0;
        p_tunnel_param->bssid[5] = 1;

        p_tunnel_param->radio_id = 1;
    }

    for (loop = 0; loop < num; loop++)
    {
        ret = (CTC_CLI_STRESS_OP_TYPE_ADD == op) ? ctc_wlan_add_tunnel(p_tunnel_param) : ctc_wlan_remove_tunnel(p_tunnel_param);
        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        else if(ret < 0 && (ret == CTC_E_HASH_CONFLICT) && CTC_CLI_STRESS_TEST_TYPE_SPEC == type)
        {
            ctc_cli_out("hash conflict!! now config to tcam, valid_count:%d\n",valid_count);
            p_tunnel_param->flag |= CTC_WLAN_TUNNEL_FLAG_RESOLVE_HASH_CONFLICT;
        }
        else if(ret < 0 && (ret != CTC_E_HASH_CONFLICT) && (ret != CTC_E_NO_RESOURCE) && (ret != CTC_E_NOT_EXIST))
        {
            ctc_cli_out("%%valid_count=%d,  ret=%d, %s\ndst_port:%d\n", valid_count, ret, ctc_get_error_desc(ret),p_tunnel_param->l4_port);
            return CLI_ERROR;
        }
        else
        {
            ctc_cli_out("%%valid_count=%d,loop = %d, ret=%d, %s\n", valid_count,loop, ret, ctc_get_error_desc(ret));
        }
        p_tunnel_param->l4_port++;

        p_tunnel_param->bssid[0] =
            (++(p_tunnel_param->bssid[0]) == 255) ? ((p_tunnel_param->bssid[1])++, 0) : p_tunnel_param->bssid[0];

    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == type)
    {
        sal_gettime(&tv2);

        sal_sprintf(str_buf,"wlan %s tunnel number: %d",(CTC_CLI_STRESS_OP_TYPE_ADD == op)? "add":"remove", num);
        output.buf = str_buf;
        output.op = op;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("wlan %s tunnel loop: %d,valid_count=%d \n",(CTC_CLI_STRESS_OP_TYPE_ADD == op)? "add":"remove", loop,valid_count);

    return ret;
}
CTC_CLI(ctc_cli_stress_wlan_tunnel,
        ctc_cli_stress_wlan_tunnel_cmd,
        "wlan tunnel (add|remove) (mode MODE|) (resolve-conflict|) (tunnel-num NUMBER) (spec|stress|perf)",
        "WLAN module",
        "Wlan tunnel",
        "Add",
        "Remove",
        "Tunnel mode",
        "0:network, 1:bssid&rid",
        "use scl tcam entry",
        "tunnel number"
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint32 number = 0;
    uint8 index = 0xFF;
    uint8 operation = CTC_CLI_STRESS_OP_TYPE_ADD;
    uint8 test_type = CTC_CLI_STRESS_TEST_TYPE_MAX;
    ctc_wlan_tunnel_t tunnel_param;


    sal_memset(&tunnel_param, 0 ,sizeof(tunnel_param));

    tunnel_param.mode = CTC_WLAN_TUNNEL_MODE_NETWORK;

    index = CTC_CLI_GET_ARGC_INDEX("remove");
    if (0xFF != index)
    {
        operation = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mode");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("mode", tunnel_param.mode, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("resolve-conflict");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_WLAN_TUNNEL_FLAG_RESOLVE_HASH_CONFLICT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("tunnel-num");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("tunnel num", number, argv[index + 1]);
    }
     index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != index)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != index)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_wlan_tunnel(&tunnel_param, number, operation, test_type);

    return ret;
}

#endif

#define   ________SCL_STRESS________

int32
_ctc_cli_stress_scl_add_all(ctc_scl_entry_t* scl_entry, ctc_field_key_t* field_key, ctc_scl_field_action_t* field_action, uint32 num, uint8 op, uint8 type)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0,valid_count =0;
    uint32 field_key_data = 0;
    sal_systime_t tv1,tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};
    ctc_scl_group_info_t ginfo;


    sal_memset(&ginfo, 0, sizeof(ctc_scl_group_info_t));

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == type)
    {
        sal_gettime(&tv1);
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == op)
    {
        ginfo.type  = CTC_SCL_GROUP_TYPE_NONE;
        if(g_ctcs_api_en)
        {
            ret = ctcs_scl_create_group(g_api_lchip, 0, &ginfo);
        }
        else
        {
            ret = ctc_scl_create_group(0, &ginfo);
        }

        CTC_CLI_STRESS_CHECK_RETURN(ret, type, valid_count);

        for (loop = 0; loop < num; loop++)
        {
            scl_entry->entry_id = loop;

            if(g_ctcs_api_en)
            {
                ret = ctcs_scl_add_entry(g_api_lchip, 0, scl_entry);
            }
            else
            {
                ret = ctc_scl_add_entry(0, scl_entry);
            }

            if(ret == CTC_E_NONE)
            {
                valid_count++;
            }
            CTC_CLI_STRESS_CHECK_RETURN(ret, type, valid_count);

            field_key_data = field_key->data;

            if(g_ctcs_api_en)
            {
                ret = ctcs_scl_add_key_field(g_api_lchip, scl_entry->entry_id, field_key);
            }
            else
            {
                ret = ctc_scl_add_key_field(scl_entry->entry_id, field_key);
            }

            CTC_CLI_STRESS_CHECK_RETURN(ret, type, valid_count);

            if (CTC_SCL_KEY_HASH_IPV4 == scl_entry->key_type)
            {
                field_key->type = CTC_FIELD_KEY_HASH_VALID;
                field_key->data = 1;

                if(g_ctcs_api_en)
                {
                    ret = ctcs_scl_add_key_field(g_api_lchip, scl_entry->entry_id, field_key);
                }
                else
                {
                    ret = ctc_scl_add_key_field(scl_entry->entry_id, field_key);
                }

                CTC_CLI_STRESS_CHECK_RETURN(ret, type, valid_count);
                field_key->type = CTC_FIELD_KEY_IP_SA;
            }

            field_key->data = field_key_data + 1;

            if(g_ctcs_api_en)
            {
                ret = ctcs_scl_add_action_field(g_api_lchip, scl_entry->entry_id, field_action);
            }
            else
            {
                ret = ctc_scl_add_action_field(scl_entry->entry_id, field_action);
            }

            CTC_CLI_STRESS_CHECK_RETURN(ret, type, valid_count);
        }

        if(g_ctcs_api_en)
        {
            ret = ctcs_scl_install_group(g_api_lchip, 0, NULL);
        }
        else
        {
            ret = ctc_scl_install_group(0, NULL);
        }

        CTC_CLI_STRESS_CHECK_RETURN(ret, type, valid_count);
    }
    else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == op)
    {
        if(g_ctcs_api_en)
        {
            ret = ctcs_scl_uninstall_group(g_api_lchip, 0);
        }
        else
        {
            ret = ctc_scl_uninstall_group(0);
        }

        CTC_CLI_STRESS_CHECK_RETURN(ret, type, valid_count);
        for (loop = 0; loop < num; loop++)
        {
            scl_entry->entry_id = loop;

            if(g_ctcs_api_en)
            {
                ret = ctcs_scl_remove_entry(g_api_lchip, scl_entry->entry_id);
            }
            else
            {
                ret = ctc_scl_remove_entry(scl_entry->entry_id);
            }

            if(ret == CTC_E_NONE)
            {
                valid_count++;
            }
            CTC_CLI_STRESS_CHECK_RETURN(ret, type, valid_count);
        }

        if(g_ctcs_api_en)
        {
            ret = ctcs_scl_destroy_group(g_api_lchip, 0);
        }
        else
        {
            ret = ctc_scl_destroy_group(0);
        }

        CTC_CLI_STRESS_CHECK_RETURN(ret, type, valid_count);
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == type)
    {
        sal_gettime(&tv2);

        sal_sprintf(str_buf,"SCL Entry:%5d", num);
        output.buf = str_buf;
        output.op = op;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    return ret;

}


CTC_CLI(ctc_cli_stress_scl_add_all,
        ctc_cli_stress_scl_add_all_cmd,
        "scl (add-all|remove-all) (ipv4-hash-entry | ipv4-tcam-entry) ip-sa A.B.C.D entry-num NUMBER (spec|stress|perf)",
        CTC_CLI_SCL_STR,
        "Add and install",
        "Unstall and remove",
        "Ipv4 hash entry",
        "Ipv4 tcam entry",
        "Ipv4 Sa",
        CTC_CLI_IPV4_FORMAT,
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint32 number = 0;
    uint8 index = 0xFF;
    uint8 operation = CTC_CLI_STRESS_OP_TYPE_MAX;
    uint8 test_type = CTC_CLI_STRESS_TEST_TYPE_MAX;
    ctc_scl_entry_t temp_scl_entry;
    ctc_scl_entry_t* scl_entry = &temp_scl_entry;
    ctc_scl_action_t* p_action = &scl_entry->action;
    ctc_field_key_t field_key;
    ctc_scl_field_action_t field_action;

    sal_memset(scl_entry, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));

    scl_entry->mode = 1;

    index = CTC_CLI_GET_ARGC_INDEX("ipv4-hash-entry");
    if (0xFF != index)
    {
        scl_entry->key.type = CTC_SCL_KEY_HASH_IPV4;
        scl_entry->key_type = CTC_SCL_KEY_HASH_IPV4;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv4-tcam-entry");
    if (0xFF != index)
    {
        scl_entry->key.type = CTC_SCL_KEY_TCAM_IPV4;
        scl_entry->key_type = CTC_SCL_KEY_TCAM_IPV4;
    }

    scl_entry->entry_id = 0;

    p_action->type = CTC_SCL_ACTION_INGRESS;
    scl_entry->action_type = CTC_SCL_ACTION_INGRESS;

    if (0 == sal_memcmp("a", argv[0], 1))
    {
        operation = CTC_CLI_STRESS_OP_TYPE_ADD;
    }
    else if (0 == sal_memcmp("r", argv[0], 1))
    {
        operation = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    }


    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != index)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != index)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    field_key.type = CTC_FIELD_KEY_IP_SA;
    CTC_CLI_GET_IPV4_ADDRESS("value", field_key.data, argv[2]);
    field_key.mask = 0xFFFFFFFF;

    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;

    CTC_CLI_GET_UINT32_RANGE("entry-num", number, argv[3], 0, CTC_MAX_UINT32_VALUE);

    ret = _ctc_cli_stress_scl_add_all(scl_entry, &field_key, &field_action, number, operation, test_type);

    return ret;
}

#define   ________packet_STRESS________

struct ctc_cli_stress_test_packet_s
{
    ctc_pkt_tx_t pkt_tx;
    uint32 tx_num;
    uint32 pkt_len;
    uint8* pkt_buf;
    uint8 is_zero_copy;
    uint8 is_buf_valid;
    uint8 is_async;

};
typedef struct ctc_cli_stress_test_packet_s ctc_cli_stress_test_packet_t;

STATIC void ctc_stress_pkt_tx_cli_callback_sys(ctc_pkt_tx_t* pkt, void* user_data)
{
    mem_free(user_data);
}

STATIC void ctc_stress_pkt_tx_cli_callback_dma(ctc_pkt_tx_t* pkt, void* user_data)
{
    if(g_ctcs_api_en)
    {
        ctcs_packet_tx_free(g_api_lchip,user_data);
    }
    else
    {
        ctc_packet_tx_free(user_data);
    }
}

STATIC int32
_ctc_cli_stress_packet_rand_32(uint32 min, uint32 max, uint32 *p_value)
{
    if (min >= max)
        return -1;

    *p_value = ((uint32)sal_rand() % (max -min + 1)) + min;

    return 0;
}

STATIC int32
_ctc_cli_stress_packet_rand_16(uint32 min, uint32 max, uint16 *p_value)
{
    if (min >= max)
        return -1;

    *p_value = (uint16)((uint32)sal_rand() % (max -min + 1)) + min;

    return 0;
}
STATIC int32
_ctc_cli_stress_packet_rand_8(uint32 min, uint32 max, uint8 *p_value)
{
    if (min >= max)
        return -1;

    *p_value = (uint8)((uint32)sal_rand() % (max -min + 1)) + min;

    return 0;
}

STATIC int32
_ctc_cli_stress_packet_tx( ctc_cli_stress_test_packet_t* g_ctc_stress_packet_tx,ctc_cli_stress_t op)
{
    int32 ret = CLI_SUCCESS;
    uint32 cnt = 0;
    uint32 valid_count =0;
    sal_systime_t tv1,tv2;
    char str_buf[32] = {0};
    ctc_cli_stress_output_t output;
    ctc_pkt_info_t* p_tx_info = &g_ctc_stress_packet_tx->pkt_tx.tx_info;
    uint8* p_user_data = NULL;
    ctc_pkt_skb_t* p_skb = &(g_ctc_stress_packet_tx->pkt_tx.skb);
    uint8* p_data = NULL;
    uint32 pkt_len = g_ctc_stress_packet_tx->pkt_len;
    uint8* pkt_buf = g_ctc_stress_packet_tx->pkt_buf;
    int32 mem_type = -1;

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == op.test_type)
    {
        sal_gettime(&tv1);
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == op.op_type)
    {
        while(cnt < g_ctc_stress_packet_tx->tx_num)
        {
            sal_memset(p_skb, 0 , sizeof(ctc_pkt_skb_t));
            if ((CTC_CLI_STRESS_DATA_MODE_RANDOM == op.data_mode) || (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == op.data_mode))
            {
                if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_SRC_SVID_VALID))
                {
                    _ctc_cli_stress_packet_rand_16(CTC_MIN_VLAN_ID, CTC_MAX_VLAN_ID, &p_tx_info->src_svid);
                }
                if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_SRC_CVID_VALID))
                {
                    _ctc_cli_stress_packet_rand_16(CTC_MIN_VLAN_ID, CTC_MAX_VLAN_ID, &p_tx_info->src_cvid);
                }
                if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_PRIORITY))
                {
                    _ctc_cli_stress_packet_rand_8(0, 15, &p_tx_info->priority);
                }
                if (CTC_FLAG_ISSET(p_tx_info->flags, CTC_PKT_FLAG_COLOR))
                {
                    _ctc_cli_stress_packet_rand_8(0, CTC_QOS_COLOR_GREEN, &p_tx_info->color);
                }

                if (!pkt_len)
                {
                    _ctc_cli_stress_packet_rand_32(12, 9600, &pkt_len);
                }
                if (!p_tx_info->src_cos)
                {
                    _ctc_cli_stress_packet_rand_8(0, 7, &p_tx_info->src_cos);
                }
                if (!p_tx_info->ttl)
                {
                    _ctc_cli_stress_packet_rand_8(1, 255, &p_tx_info->ttl);
                }
                if (!p_tx_info->fid)
                {
                    _ctc_cli_stress_packet_rand_16(0, CTC_MAX_FID_ID, &p_tx_info->fid);
                }
            }

            if(g_ctc_stress_packet_tx->is_zero_copy)
            {
                if(g_ctcs_api_en)
                {
                    ret = ctcs_packet_tx_alloc(g_api_lchip,pkt_len, (void**)&p_user_data);
                }
                else
                {
                    ret = ctc_packet_tx_alloc(pkt_len, (void**)&p_user_data);
                }
                if (ret < 0)
                {
                    if(CTC_E_NO_MEMORY == ret)
                    {
                        continue;
                    }
                    else
                    {
                        ctc_cli_out("%% ctc_packet_tx_alloc,ret = %d, %s\n", ret, ctc_get_error_desc(ret));
                        return CLI_ERROR;
                    }
                }
                sal_memcpy(p_user_data, pkt_buf, pkt_len);
                p_skb->data = p_user_data;
                p_skb->len= pkt_len;
                mem_type = 0;
            }
            else if(g_ctc_stress_packet_tx->is_buf_valid)
            {
                ctc_packet_skb_init(p_skb);
                p_data = ctc_packet_skb_put(p_skb, pkt_len);
                sal_memcpy(p_data, pkt_buf, pkt_len);
                mem_type = 2;
            }
            else
            {
                 p_user_data = mem_malloc(MEM_CLI_MODULE, pkt_len);
                 if(!p_user_data)
                 {
                        return CLI_ERROR;
                 }
                 sal_memcpy(p_user_data, pkt_buf, pkt_len);
                 p_skb->data = p_user_data;
                 p_skb->len= pkt_len;
                 mem_type = 1;
            }

            if (g_ctc_stress_packet_tx->is_async)
            {
                if(0 == mem_type)
                {
                    g_ctc_stress_packet_tx->pkt_tx.callback = ctc_stress_pkt_tx_cli_callback_dma;
                    g_ctc_stress_packet_tx->pkt_tx.user_data = (void*)p_user_data;
                }
                else if (1 == mem_type)
                {
                    g_ctc_stress_packet_tx->pkt_tx.callback = ctc_stress_pkt_tx_cli_callback_sys;
                    g_ctc_stress_packet_tx->pkt_tx.user_data = (void*)p_user_data;
                }
            }

            if(g_ctcs_api_en)
            {
                 ret = ctcs_packet_tx(g_api_lchip,  &g_ctc_stress_packet_tx->pkt_tx);
            }
            else
            {
                ret = ctc_packet_tx(&g_ctc_stress_packet_tx->pkt_tx);
            }
            if (ret < 0)
            {
                if(CTC_E_NO_MEMORY == ret)
                {
                    if(mem_type == 1)
                    {
                        mem_free(p_user_data);
                    }
                    else if (mem_type == 0)
                    {
                        if(g_ctcs_api_en)
                        {
                            ctcs_packet_tx_free(g_api_lchip,(void*)p_user_data);
                        }
                        else
                        {
                            ctc_packet_tx_free((void*)p_user_data);
                        }
                    }
                    continue;
                }
                else
                {
                    ctc_cli_out("%% ctc_packet_tx,ret = %d, %s,valid_count: %5d\n", ret, ctc_get_error_desc(ret),valid_count);
                    return CLI_ERROR;
                }
            }
            if((!g_ctc_stress_packet_tx->is_async) && (mem_type == 1))
            {
                mem_free(p_user_data);
            }
            valid_count++;
            cnt++;
        }
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == op.test_type)
    {
        sal_gettime(&tv2);

        sal_sprintf(str_buf,"valid_count: %5d\n", valid_count);
        output.buf = str_buf;
        output.op = op.op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stress_packet_tx,
        ctc_cli_stress_packet_tx_cmd,
        "packet (spec|stress|perf) (mode MODE) {dest-gport DEST_GPORT (assign-dest-port|)| is-mcast dest-group-id DEST_GID | src-svid SRC_SVID| src-cvid SRC_CVID | src-port SRC_GPORT | oper-type OPER_TYPE | priority PRIORITY | color COLOR | src-cos SRC_COS |\
        ttl TTL | is-critical |(nh-offset NH_OFFSET (nh-offset-is-8w |)|bypass) | linkagg-hash HASH | vlan-domain VLAN-DOMAIN | fid FID | svlan-tpid SVLAN_TPID | isolated-id ISOLATED_ID |\
        ingress-mode |cancel-dot1ae| async | (zero-copy| use-skb-buf|)}\
        (pkt-sample (ucast| mcast| eth-lbm)) (pkt-len LEN|) (count COUNT) (random|incr|random-all)",
        CTC_CLI_PACKET_M_STR,
        "Send packet",
        "Spec test",
        "Stress test",
        "Performance test",
        "0 DMA, 1 Ethernet",
        "Destination global port ID",
        CTC_CLI_GPORT_ID_DESC,
        "Cpu assign the out port for stacking",
        "Is multicast packet",
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        "Group ID value",
        "Source S-VLAN",
        CTC_CLI_VLAN_RANGE_DESC,
        "Source C-VLAN",
        CTC_CLI_VLAN_RANGE_DESC,
        "Source port",
        CTC_CLI_GPORT_ID_WITHOUT_LINKAGG_DESC,
        "Operation type",
        "Refer to ctc_pkt_oper_type_t",
        "Priority of the packet",
        "Priority value",
        "Color of the packet",
        "Refer to ctc_qos_color_t",
        "COS of the packet",
        "Range is [0, 7]",
        "TTL of the packet",
        "Valur of ttl",
        "Packet should not be droped in queue",
        CTC_CLI_NH_DSNH_OFFSET_STR,
        CTC_CLI_NH_DSNH_OFFSET_VALUE_STR,
        "Nexthop is 8w",
        "Nexthop is bypass",
        "Hash of LAG for load balance",
        "Hash Value",
        "Vlan domain",
        "Refer to ctc_port_vlan_domain_type_t",
        "Forwarding Id",
        CTC_CLI_FID_ID_DESC,
        "Svlan tpid",
        "Tpid value",
        "Port isolated id",
        CTC_CLI_PORT_ISOLATION_ID_DESC,
        "Ingress mode",
        "Pakcet won't be encrypted",
        "Transmit pkt asynchronously",
        "zero copy mode to tx",
        "pkt data is in skb.buf",
        "Pkt sample",
        "unicast sample",
        "multicast sample",
        "ethernet oam lbm sample",
        "pkt length",
        "length value",
        "Pkt count",
        "count value",
        "Random key",
        "Increase key",
        "Random key and ad")
{
    static uint8 pkt_buf_uc[CTC_PKT_MTU] = {
        0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x81, 0x00, 0x00, 0x0a,
        0x08, 0x00, 0x45, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x22, 0xf5, 0xc7, 0xc7,
        0xc7, 0x01, 0xc8, 0x01, 0x01, 0x01, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa,
        0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa,
        0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa, 0xaa, 0xdd, 0xaa, 0xaa
    };
    static uint8 pkt_buf_mc[CTC_PKT_MTU] = {
        0x01, 0x00, 0x5E, 0x7F, 0x00, 0x01, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x81, 0x00, 0x00, 0x0a,
        0x08, 0x00, 0x45, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x22, 0xF5, 0xC7, 0xC7,
        0xC7, 0x01, 0xC8, 0x01, 0x01, 0x01, 0xAA, 0xAA, 0xAA, 0xDD, 0xAA, 0xAA, 0xAA, 0xDD, 0xAA, 0xAA,
        0xAA, 0xDD, 0xAA, 0xAA, 0xAA, 0xDD, 0xAA, 0xAA, 0xAA, 0xDD, 0xAA, 0xAA, 0xAA, 0xDD, 0xAA, 0xAA,
        0xAA, 0xDD, 0xAA, 0xAA, 0xAA, 0xDD, 0xAA, 0xAA, 0xAA, 0xDD, 0xAA, 0xAA, 0xAA, 0xDD, 0xAA, 0xAA
    };
    static uint8 pkt_buf_cfm_lbm[100] = {
        0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x81, 0x00, 0x00, 0x0a,
        0x89, 0x02, 0xa0, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    int32 idx = 0;
    int32 ret = 0;
    uint32 pkt_len = 0;
    ctc_cli_stress_t op = {0};
    ctc_pkt_tx_t* p_pkt_tx = NULL;
    ctc_pkt_info_t* p_tx_info = NULL;

    ctc_cli_stress_test_packet_t* g_ctc_stress_packet_tx = (ctc_cli_stress_test_packet_t*)sal_malloc(sizeof(ctc_cli_stress_test_packet_t));
    if(!g_ctc_stress_packet_tx)
    {
        return CLI_ERROR;
    }
    sal_memset(g_ctc_stress_packet_tx, 0, sizeof(ctc_cli_stress_test_packet_t));
    p_pkt_tx = &g_ctc_stress_packet_tx->pkt_tx;
    p_tx_info = &(p_pkt_tx->tx_info);
    op.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;

    idx = CTC_CLI_GET_ARGC_INDEX("mode");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8("mode", p_pkt_tx->mode, argv[idx + 1])
         /*pkt_tx.mode = (mode == 1 ? CTC_PKT_MODE_ETH : CTC_PKT_MODE_DMA)*/
    }
    idx = CTC_CLI_GET_ARGC_INDEX("dest-gport");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("dest-gport", p_tx_info->dest_gport, argv[idx + 1]);
    }

    idx = CTC_CLI_GET_ARGC_INDEX("assign-dest-port");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_ASSIGN_DEST_PORT;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("is-mcast");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_MCAST;

        idx = CTC_CLI_GET_ARGC_INDEX("dest-group-id");
        if (0xFF != idx)
        {
             CTC_CLI_GET_UINT32("dest-group-id", p_tx_info->dest_group_id, argv[idx + 1]);
        }
    }

    idx = CTC_CLI_GET_ARGC_INDEX("src-svid");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_SRC_SVID_VALID;
        CTC_CLI_GET_UINT16("src-svid", p_tx_info->src_svid, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("src-cvid");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_SRC_CVID_VALID;
        CTC_CLI_GET_UINT16("src-cvid", p_tx_info->src_cvid, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("src-port");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_SRC_PORT_VALID;
        CTC_CLI_GET_UINT16("src-port", p_tx_info->src_port, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("oper-type");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8("oper-type", p_tx_info->oper_type, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("priority");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_PRIORITY;
        CTC_CLI_GET_UINT8("priority", p_tx_info->priority, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("color");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_COLOR;
        CTC_CLI_GET_UINT8("color", p_tx_info->color, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("src-cos");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8("src-cos", p_tx_info->src_cos, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("ttl");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8("ttl", p_tx_info->ttl, argv[idx + 1]);
    }
    else
    {
        p_tx_info->ttl = 1;
    }
    idx = CTC_CLI_GET_ARGC_INDEX("is-critical");
    if (0xFF != idx)
    {
        p_tx_info->is_critical = 1;
    }
    idx = CTC_CLI_GET_ARGC_INDEX("nhid");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("nhid", p_tx_info->nhid, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("nh-offset");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_NH_OFFSET_VALID;
        CTC_CLI_GET_UINT32("nh-offset", p_tx_info->nh_offset, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("nh-offset-is-8w");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_NH_OFFSET_IS_8W;
    }
    idx = CTC_CLI_GET_ARGC_INDEX("bypass");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_NH_OFFSET_BYPASS;
    }
    idx = CTC_CLI_GET_ARGC_INDEX("linkagg-hash");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_HASH_VALID;
        CTC_CLI_GET_UINT8("linkagg-hash", p_tx_info->hash, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("vlan-domain");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8("vlan-domain", p_tx_info->vlan_domain, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("fid");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT8("fid", p_tx_info->fid, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("svlan-tpid");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("svlan-tpid", p_tx_info->svlan_tpid, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("isolated-id");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT16("isolated-id", p_tx_info->isolated_id, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX("ingress-mode");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_INGRESS_MODE;
    }
    idx = CTC_CLI_GET_ARGC_INDEX("cancel-dot1ae");
    if (0xFF != idx)
    {
        p_tx_info->flags |= CTC_PKT_FLAG_CANCEL_DOT1AE;
    }
    idx = CTC_CLI_GET_ARGC_INDEX("async");
    if (0xFF != idx)
    {
        g_ctc_stress_packet_tx->is_async = 1;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("zero-copy");
    if (0xFF != idx)
    {
        g_ctc_stress_packet_tx->is_zero_copy= 1;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("use-skb-buf");
    if (0xFF != idx)
    {
        g_ctc_stress_packet_tx->is_buf_valid= 1;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("pkt-sample");
    if (0xFF != idx)
    {
        /* packet type */
        if (sal_strncmp(argv[idx + 1], "ucast", sal_strlen("ucast"))== 0)
        {
            g_ctc_stress_packet_tx->pkt_buf = pkt_buf_uc;
            g_ctc_stress_packet_tx->pkt_len = 80;
        }
        else if (sal_strncmp(argv[idx + 1], "mcast", sal_strlen("mcast")) == 0)
        {
            g_ctc_stress_packet_tx->pkt_buf = pkt_buf_mc;
            g_ctc_stress_packet_tx->pkt_len = 80;
        }
        else if (sal_strncmp(argv[idx + 1], "eth-lbm", sal_strlen("eth-lbm")) == 0)
        {
            g_ctc_stress_packet_tx->pkt_buf = pkt_buf_cfm_lbm;
            g_ctc_stress_packet_tx->pkt_len = 60;
        }
    }

    idx = CTC_CLI_GET_ARGC_INDEX("pkt-len");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("pkt len", pkt_len, argv[idx + 1]);
        if(pkt_len > CTC_PKT_MTU || pkt_len < 1)
        {
            ctc_cli_out("%%0 <  pkt-len <= %d \n", ret, CTC_PKT_MTU);
            return CLI_ERROR;
        }
        g_ctc_stress_packet_tx->pkt_len = pkt_len;
    }


    idx = CTC_CLI_GET_ARGC_INDEX("count");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("count",g_ctc_stress_packet_tx->tx_num, argv[idx + 1]);
    }
    idx = CTC_CLI_GET_ARGC_INDEX_ENHANCE("incr");
    if (idx != 0xFF)
    {
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_INCREASE;
    }

    idx = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random");
    if (idx != 0xFF)
    {
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM;
    }

    idx = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random-all");
    if (idx != 0xFF)
    {
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL;
    }
    idx = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != idx)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != idx)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }

    idx = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != idx)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_packet_tx(g_ctc_stress_packet_tx ,op);

    if (ret < 0)
    {
        sal_free(g_ctc_stress_packet_tx);
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }
    sal_free(g_ctc_stress_packet_tx);
    return CLI_SUCCESS;
}

#define   ________STATS_STRESS________

enum ctc_cli_stress_statsid_mode_e
{
    CTC_CLI_STRESS_STATSID_MODE_RANDOM,
    CTC_CLI_STRESS_STATSID_MODE_INCR,
    CTC_CLI_STRESS_STATSID_MODE_MAX
};
typedef enum ctc_cli_stress_statsid_mode_e ctc_cli_stress_statsid_mode_t;

STATIC int32
_ctc_cli_stress_stats_rand(uint32 min, uint32 max, uint32 *p_value)
{
    if (min >= max)
        return -1;

    *p_value = (((uint32)sal_rand() % (max - min + 1))) + min;

    return 0;
}

CTC_CLI(ctc_cli_stress_stats_create1,
        ctc_cli_stress_stats_create1_cmd,
        "stats flow-stats create statsid "
        "random-all "
        "(entry-num NUMBER) "
        "(spec | stress | perf)",
        CTC_CLI_STATS_STR,
        "Flow statistics",
        "Create",
        CTC_CLI_STATS_ID_DESC,
        "Random key and ad",
        "Entry number",
        "Specific number",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = 0;
    uint8 index = 0xFF;
    ctc_stats_statsid_t stats_statsid;
    uint32 i = 0;
    uint32 create_cnt = 0;
    ctc_cli_stress_test_type_t test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    sal_systime_t tv1,tv2;
    char str_buf[32] = {0};
    ctc_cli_stress_output_t output;
    uint32 success_cnt = 0;
    uint32 fail_cnt = 0;
    uint32 color_aware_success = 0;
    uint32 color_aware_fail = 0;
    uint32 value = 0;
    uint32* statsid_list = NULL;

    sal_memset(&stats_statsid, 0, sizeof(ctc_stats_statsid_t));

    index = CTC_CLI_GET_ARGC_INDEX("entry-num");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("entry number", create_cnt, argv[index+1], 1, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (index != 0xFF)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (index != 0xFF)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (index != 0xFF)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    if (NULL == g_ctc_stress_misc->stats_id_list)
    {
        statsid_list = sal_malloc(sizeof(uint32)*create_cnt);
        if (NULL == statsid_list)
        {
            return CLI_ERROR;
        }
        g_ctc_stress_misc->stats_id_list = statsid_list;
    }
    else
    {
        statsid_list = sal_malloc(sizeof(uint32)*(create_cnt+g_ctc_stress_misc->stats_entry_num));
        if (NULL == statsid_list)
        {
            return CLI_ERROR;
        }
        sal_memcpy(statsid_list, g_ctc_stress_misc->stats_id_list, sizeof(uint32)*g_ctc_stress_misc->stats_entry_num);
        sal_free(g_ctc_stress_misc->stats_id_list);
        g_ctc_stress_misc->stats_id_list = statsid_list;
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == test_type)
    {
        sal_gettime(&tv1);
    }

    for (i=0; i<create_cnt; i++)
    {
        (void)_ctc_cli_stress_stats_rand(CTC_INGRESS, CTC_EGRESS, &value);
        stats_statsid.dir = (ctc_direction_t)value;

        (void)_ctc_cli_stress_stats_rand(0, CTC_STATS_STATSID_TYPE_MAX-1, &value);
        stats_statsid.type = (ctc_stats_statsid_type_t)value;

        switch (stats_statsid.type)
        {
            case CTC_STATS_STATSID_TYPE_MPLS:
            {
                (void)_ctc_cli_stress_stats_rand(0, 1, &value);
                stats_statsid.statsid.is_vc_label = value;
                break;
            }
            case CTC_STATS_STATSID_TYPE_ACL:
            {
                if (stats_statsid.dir == CTC_INGRESS)
                {
                    (void)_ctc_cli_stress_stats_rand(0, 7, &value);
                }
                else
                {
                    (void)_ctc_cli_stress_stats_rand(0, 2, &value);
                }
                stats_statsid.statsid.acl_priority = value;
                (void)_ctc_cli_stress_stats_rand(0, 1, &value);
                stats_statsid.color_aware = value;
                break;
            }
            default:
                break;
        }

        (void)_ctc_cli_stress_stats_rand(1, CTC_MAX_UINT32_VALUE, &value);
        stats_statsid.stats_id = value;

        if(g_ctcs_api_en)
        {
            ret = ctcs_stats_create_statsid(g_api_lchip, &stats_statsid);
        }
        else
        {
            ret = ctc_stats_create_statsid(&stats_statsid);
        }

        if (ret == CTC_E_INVALID_PARAM)
        {
            ctc_cli_out("%% Failed(%d): %s \n", ret, ctc_get_error_desc(ret));
        }

        CTC_CLI_STRESS_CHECK_RETURN(ret, test_type, success_cnt);

        if (!ret)
        {
            statsid_list = g_ctc_stress_misc->stats_id_list + g_ctc_stress_misc->stats_entry_num;
            *statsid_list = stats_statsid.stats_id;
            g_ctc_stress_misc->stats_entry_num++;
        }

        if ((stats_statsid.type == CTC_STATS_STATSID_TYPE_ACL) && (stats_statsid.color_aware))
        {
            color_aware_success = ((ret==0) ? color_aware_success+1: color_aware_success);
            color_aware_fail = ((ret!=0) ? color_aware_fail+1: color_aware_fail);
        }

        success_cnt = ((ret==0) ? success_cnt+1: success_cnt);
        fail_cnt = ((ret!=0) ? fail_cnt+1: fail_cnt);
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == test_type)
    {
        sal_gettime(&tv2);

        sal_memset(&output, 0, sizeof(output));
        sal_sprintf(str_buf,"Create Stats Entry:%5d", create_cnt);
        output.buf = str_buf;
        output.op = CTC_CLI_STRESS_OP_TYPE_ADD;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%Total  : %u \n", g_ctc_stress_misc->stats_entry_num);
    ctc_cli_out("%%Add    : %u \n", create_cnt);
    ctc_cli_out("%%Success: %u(color-aware %u) \n", success_cnt, color_aware_success);
    ctc_cli_out("%%Fail   : %u(color-aware %u) \n", fail_cnt, color_aware_fail);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stress_stats_create2,
        ctc_cli_stress_stats_create2_cmd,
        "stats flow-stats create statsid type "
        "(vlan | vrf | ipmc | ip | mac | flow-hash | mpls (vc-label|) | scl | acl (priority PRIORITY) (color-aware|) | tunnel | nexthop | nh-mpls-pw | nh-mpls-lsp | nh-mcast | l3if | fid | ecmp | sdc ) "
        "(ingress | egress) "
        "random "
        "(entry-num NUMBER) "
        "(spec | stress | perf)",
        CTC_CLI_STATS_STR,
        "Flow statistics",
        "Create",
        CTC_CLI_STATS_ID_DESC,
        "Statistics type",
        "Vlan",
        "Vrf",
        "Ipmc",
        "Ip",
        "Mac",
        "Flow hash",
        "Mpls",
        "Vc label",
        "Scl",
        "Acl",
        CTC_CLI_ACL_GROUP_PRIO_STR,
        CTC_CLI_ACL_GROUP_PRIO_VALUE,
        "Color aware",
        "Tunnel",
        "Nexthop",
        "Nexthop mpls pw",
        "Nexthop mpls lsp",
        "Nexthop mcast",
        "L3if",
        "Fid",
        "Ecmp",
        "SDC",
        "Ingress",
        "Egress",
        "Random key",
        "Entry number",
        "Specific number",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = 0;
    uint8 index = 0xFF;
    uint32 i = 0;
    ctc_stats_statsid_t stats_statsid;
    uint32 create_cnt = 0;
    ctc_cli_stress_test_type_t test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    sal_systime_t tv1,tv2;
    char str_buf[32] = {0};
    ctc_cli_stress_output_t output;
    uint32 success_cnt = 0;
    uint32 fail_cnt = 0;
    uint32 color_aware_success = 0;
    uint32 color_aware_fail = 0;
    uint32 value = 0;
    uint32* statsid_list = NULL;

    sal_memset(&stats_statsid, 0, sizeof(ctc_stats_statsid_t));

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (index != 0xFF)
    {
        stats_statsid.dir = CTC_INGRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (index != 0xFF)
    {
        stats_statsid.dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_VLAN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("vrf");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_VRF;
    }
    index = CTC_CLI_GET_ARGC_INDEX("ipmc");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_IPMC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_IP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mac");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_MAC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-hash");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_FLOW_HASH;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mpls");
    if (index != 0xFF)
    {
        index = CTC_CLI_GET_ARGC_INDEX("vc-label");
        if (index != 0xFF)
        {
            stats_statsid.statsid.is_vc_label = 1;
        }
        stats_statsid.type = CTC_STATS_STATSID_TYPE_MPLS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("scl");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_SCL;
    }
    index = CTC_CLI_GET_ARGC_INDEX("acl");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_ACL;
        index = CTC_CLI_GET_ARGC_INDEX("priority");
        if (index != 0xFF)
        {
            if (stats_statsid.dir == CTC_INGRESS)
            {
                CTC_CLI_GET_UINT8_RANGE("acl priority", stats_statsid.statsid.acl_priority, argv[index + 1] ,0, 7);
            }
            else
            {
                CTC_CLI_GET_UINT8_RANGE("acl priority", stats_statsid.statsid.acl_priority, argv[index + 1] ,0, 2);
            }
        }
        index = CTC_CLI_GET_ARGC_INDEX("color-aware");
        if (index != 0xFF)
        {
            stats_statsid.color_aware = 1;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("tunnel");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_TUNNEL;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nexthop");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_NEXTHOP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nh-mpls-pw");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_NEXTHOP_MPLS_PW;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nh-mpls-lsp");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_NEXTHOP_MPLS_LSP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nh-mcast");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_NEXTHOP_MCAST;
    }
    index = CTC_CLI_GET_ARGC_INDEX("l3if");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_L3IF;
    }
    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_FID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecmp");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_ECMP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("sdc");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_SDC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("entry-num");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("entry number", create_cnt, argv[index+1], 1, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (index != 0xFF)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (index != 0xFF)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (index != 0xFF)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    if (NULL == g_ctc_stress_misc->stats_id_list)
    {
        statsid_list = sal_malloc(sizeof(uint32)*create_cnt);
        if (NULL == statsid_list)
        {
            return CLI_ERROR;
        }
        g_ctc_stress_misc->stats_id_list = statsid_list;
    }
    else
    {
        statsid_list = sal_malloc(sizeof(uint32)*(create_cnt+g_ctc_stress_misc->stats_entry_num));
        if (NULL == statsid_list)
        {
            return CLI_ERROR;
        }
        sal_memcpy(statsid_list, g_ctc_stress_misc->stats_id_list, sizeof(uint32)*g_ctc_stress_misc->stats_entry_num);
        sal_free(g_ctc_stress_misc->stats_id_list);
        g_ctc_stress_misc->stats_id_list = statsid_list;
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == test_type)
    {
        sal_gettime(&tv1);
    }

    for (i=0; i<create_cnt; i++)
    {
        (void)_ctc_cli_stress_stats_rand(1, CTC_MAX_UINT32_VALUE, &value);
        stats_statsid.stats_id = value;

        if(g_ctcs_api_en)
        {
            ret = ctcs_stats_create_statsid(g_api_lchip, &stats_statsid);
        }
        else
        {
            ret = ctc_stats_create_statsid(&stats_statsid);
        }

        if (ret == CTC_E_INVALID_PARAM)
        {
            ctc_cli_out("%% Failed(%d): %s \n", ret, ctc_get_error_desc(ret));
        }

        CTC_CLI_STRESS_CHECK_RETURN(ret, test_type, success_cnt);

        if (!ret)
        {
            statsid_list = g_ctc_stress_misc->stats_id_list + g_ctc_stress_misc->stats_entry_num;
            *statsid_list = stats_statsid.stats_id;
            g_ctc_stress_misc->stats_entry_num++;
        }

        if ((stats_statsid.type == CTC_STATS_STATSID_TYPE_ACL) && (stats_statsid.color_aware))
        {
            color_aware_success = ((ret==0) ? color_aware_success+1: color_aware_success);
            color_aware_fail = ((ret!=0) ? color_aware_fail+1: color_aware_fail);
        }

        success_cnt = ((ret==0) ? success_cnt+1: success_cnt);
        fail_cnt = ((ret!=0) ? fail_cnt+1: fail_cnt);
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == test_type)
    {
        sal_gettime(&tv2);

        sal_memset(&output, 0, sizeof(output));
        sal_sprintf(str_buf,"Create Stats Entry:%5d", create_cnt);
        output.buf = str_buf;
        output.op = CTC_CLI_STRESS_OP_TYPE_ADD;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%Total  : %u \n", g_ctc_stress_misc->stats_entry_num);
    ctc_cli_out("%%Add    : %u \n", create_cnt);
    ctc_cli_out("%%Success: %u(color-aware %u) \n", success_cnt, color_aware_success);
    ctc_cli_out("%%Fail   : %u(color-aware %u) \n", fail_cnt, color_aware_fail);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stress_stats_create3,
        ctc_cli_stress_stats_create3_cmd,
        "stats flow-stats create (statsid NUMBER) type "
        "(vlan | vrf | ipmc | ip | mac | flow-hash | mpls (vc-label|) | scl | acl (priority PRIORITY) (color-aware|) | tunnel | nexthop | nh-mpls-pw | nh-mpls-lsp | nh-mcast | l3if | fid | ecmp | sdc ) "
        "(ingress | egress) "
        "(incr STEP) "
        "(entry-num NUMBER) "
        "(spec | stress | perf)",
        CTC_CLI_STATS_STR,
        "Flow statistics",
        "Create",
        CTC_CLI_STATS_ID_DESC,
        CTC_CLI_STATS_ID_VAL,
        "Statistics type",
        "Vlan",
        "Vrf",
        "Ipmc",
        "Ip",
        "Mac",
        "Flow hash",
        "Mpls",
        "Vc label",
        "Scl",
        "Acl",
        CTC_CLI_ACL_GROUP_PRIO_STR,
        CTC_CLI_ACL_GROUP_PRIO_VALUE,
        "Color aware",
        "Tunnel",
        "Nexthop",
        "Nexthop mpls pw",
        "Nexthop mpls lsp",
        "Nexthop mcast",
        "L3if",
        "Fid",
        "Ecmp",
        "SDC",
        "Ingress",
        "Egress",
        "Increase key",
        "Increase key step value",
        "Entry number",
        "Specific number",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = 0;
    uint8 index = 0xFF;
    uint32 i = 0;
    ctc_stats_statsid_t stats_statsid;
    uint32 create_cnt = 0;
    ctc_cli_stress_test_type_t test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    uint32 statsid_base = 0;
    uint32 statsid_step = 0;
    sal_systime_t tv1,tv2;
    char str_buf[32] = {0};
    ctc_cli_stress_output_t output;
    uint32 success_cnt = 0;
    uint32 fail_cnt = 0;
    uint32 color_aware_success = 0;
    uint32 color_aware_fail = 0;
    uint32 value = 0;
    uint64 cfg_statsid = 0;
    uint64 max_statsid = 0;
    uint32* statsid_list = NULL;

    sal_memset(&stats_statsid, 0, sizeof(ctc_stats_statsid_t));

    index = CTC_CLI_GET_ARGC_INDEX("statsid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("statsid base number", statsid_base, argv[index+1], 1, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (index != 0xFF)
    {
        stats_statsid.dir = CTC_INGRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (index != 0xFF)
    {
        stats_statsid.dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_VLAN;
    }
    index = CTC_CLI_GET_ARGC_INDEX("vrf");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_VRF;
    }
    index = CTC_CLI_GET_ARGC_INDEX("ipmc");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_IPMC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("ip");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_IP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mac");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_MAC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("flow-hash");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_FLOW_HASH;
    }
    index = CTC_CLI_GET_ARGC_INDEX("mpls");
    if (index != 0xFF)
    {
        index = CTC_CLI_GET_ARGC_INDEX("vc-label");
        if (index != 0xFF)
        {
            stats_statsid.statsid.is_vc_label = 1;
        }
        stats_statsid.type = CTC_STATS_STATSID_TYPE_MPLS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("scl");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_SCL;
    }
    index = CTC_CLI_GET_ARGC_INDEX("acl");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_ACL;
        index = CTC_CLI_GET_ARGC_INDEX("priority");
        if (index != 0xFF)
        {
            if (stats_statsid.dir == CTC_INGRESS)
            {
                CTC_CLI_GET_UINT8_RANGE("acl priority", stats_statsid.statsid.acl_priority, argv[index + 1] ,0, 7);
            }
            else
            {
                CTC_CLI_GET_UINT8_RANGE("acl priority", stats_statsid.statsid.acl_priority, argv[index + 1] ,0, 2);
            }
        }
        index = CTC_CLI_GET_ARGC_INDEX("color-aware");
        if (index != 0xFF)
        {
            stats_statsid.color_aware = 1;
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("tunnel");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_TUNNEL;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nexthop");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_NEXTHOP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nh-mpls-pw");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_NEXTHOP_MPLS_PW;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nh-mpls-lsp");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_NEXTHOP_MPLS_LSP;
    }
    index = CTC_CLI_GET_ARGC_INDEX("nh-mcast");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_NEXTHOP_MCAST;
    }
    index = CTC_CLI_GET_ARGC_INDEX("l3if");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_L3IF;
    }
    index = CTC_CLI_GET_ARGC_INDEX("fid");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_FID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ecmp");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_ECMP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("sdc");
    if (index != 0xFF)
    {
        stats_statsid.type = CTC_STATS_STATSID_TYPE_SDC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("incr");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("step number", statsid_step, argv[index+1], 1, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("entry-num");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("entry number", create_cnt, argv[index+1], 1, CTC_MAX_UINT32_VALUE);
    }

    cfg_statsid = (uint64)statsid_base + (uint64)statsid_step * (uint64)create_cnt;
    max_statsid = CTC_MAX_UINT32_VALUE;
    if (cfg_statsid > max_statsid)
    {
        ctc_cli_out("%% StatsId out of range(0x%x-0x%x) \n", 1, CTC_MAX_UINT32_VALUE);
        return CLI_ERROR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (index != 0xFF)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (index != 0xFF)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (index != 0xFF)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    if (NULL == g_ctc_stress_misc->stats_id_list)
    {
        statsid_list = sal_malloc(sizeof(uint32)*create_cnt);
        if (NULL == statsid_list)
        {
            return CLI_ERROR;
        }
        g_ctc_stress_misc->stats_id_list = statsid_list;
    }
    else
    {
        statsid_list = sal_malloc(sizeof(uint32)*(create_cnt+g_ctc_stress_misc->stats_entry_num));
        if (NULL == statsid_list)
        {
            return CLI_ERROR;
        }
        sal_memcpy(statsid_list, g_ctc_stress_misc->stats_id_list, sizeof(uint32)*g_ctc_stress_misc->stats_entry_num);
        sal_free(g_ctc_stress_misc->stats_id_list);
        g_ctc_stress_misc->stats_id_list = statsid_list;
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == test_type)
    {
        sal_gettime(&tv1);
    }

    for (i=0; i<create_cnt; i++)
    {
        value = statsid_base + i * statsid_step;
        stats_statsid.stats_id = value;

        if(g_ctcs_api_en)
        {
            ret = ctcs_stats_create_statsid(g_api_lchip, &stats_statsid);
        }
        else
        {
            ret = ctc_stats_create_statsid(&stats_statsid);
        }

        if (ret == CTC_E_INVALID_PARAM)
        {
            ctc_cli_out("%% Failed(%d): %s \n", ret, ctc_get_error_desc(ret));
        }

        CTC_CLI_STRESS_CHECK_RETURN(ret, test_type, success_cnt);

        if (!ret)
        {
            statsid_list = g_ctc_stress_misc->stats_id_list + g_ctc_stress_misc->stats_entry_num;
            *statsid_list = stats_statsid.stats_id;
            g_ctc_stress_misc->stats_entry_num++;
        }

        if ((stats_statsid.type == CTC_STATS_STATSID_TYPE_ACL) && (stats_statsid.color_aware))
        {
            color_aware_success = ((ret==0) ? color_aware_success+1: color_aware_success);
            color_aware_fail = ((ret!=0) ? color_aware_fail+1: color_aware_fail);
        }

        success_cnt = ((ret==0) ? success_cnt+1: success_cnt);
        fail_cnt = ((ret!=0) ? fail_cnt+1: fail_cnt);
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == test_type)
    {
        sal_gettime(&tv2);

        sal_memset(&output, 0, sizeof(output));
        sal_sprintf(str_buf,"Create Stats Entry:%5d", create_cnt);
        output.buf = str_buf;
        output.op = CTC_CLI_STRESS_OP_TYPE_ADD;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%Total  : %u \n", g_ctc_stress_misc->stats_entry_num);
    ctc_cli_out("%%Add    : %u \n", create_cnt);
    ctc_cli_out("%%Success: %u(color-aware %u) \n", success_cnt, color_aware_success);
    ctc_cli_out("%%Fail   : %u(color-aware %u) \n", fail_cnt, color_aware_fail);

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stress_stats_destroy,
        ctc_cli_stress_stats_destroy_cmd,
        "stats flow-stats destroy all (spec | stress | perf)",
        CTC_CLI_STATS_STR,
        "Flow statistics",
        "Destroy",
        "All",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = 0;
    uint8 index = 0xFF;
    uint32 i = 0;
    uint32 statsid = 0;
    uint32 success_cnt = 0;
    uint32 fail_cnt = 0;
    ctc_cli_stress_test_type_t test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    ctc_cli_stress_output_t output;
    sal_systime_t tv1,tv2;
    char str_buf[32] = {0};

    if (NULL == g_ctc_stress_misc)
    {
        return CLI_ERROR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (index != 0xFF)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (index != 0xFF)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (index != 0xFF)
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == test_type)
    {
        sal_gettime(&tv1);
    }

    if ((NULL != g_ctc_stress_misc->stats_id_list) && (0 != g_ctc_stress_misc->stats_entry_num))
    {
        for (i=0; i<g_ctc_stress_misc->stats_entry_num; i++)
        {
            statsid = g_ctc_stress_misc->stats_id_list[i];

            if(g_ctcs_api_en)
            {
                ret = ctcs_stats_destroy_statsid(g_api_lchip, statsid);
            }
            else
            {
                ret = ctc_stats_destroy_statsid(statsid);
            }
            if (ret)
            {
                ctc_cli_out("%% Failed(%d): %s \n", ret, ctc_get_error_desc(ret));
            }

            success_cnt = ((ret==0) ? success_cnt+1: success_cnt);
            fail_cnt = ((ret!=0) ? fail_cnt+1: fail_cnt);
        }
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == test_type)
    {
        sal_gettime(&tv2);

        sal_memset(&output, 0, sizeof(output));
        sal_sprintf(str_buf,"Destroy Stats Entry:%5d", g_ctc_stress_misc->stats_entry_num);
        output.buf = str_buf;
        output.op = CTC_CLI_STRESS_OP_TYPE_REMOVE;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%Total  : %u \n", g_ctc_stress_misc->stats_entry_num);
    ctc_cli_out("%%Success: %u \n", success_cnt);
    ctc_cli_out("%%Fail   : %u \n", fail_cnt);

    g_ctc_stress_misc->stats_entry_num = 0;
    if (NULL != g_ctc_stress_misc->stats_id_list)
    {
        sal_free(g_ctc_stress_misc->stats_id_list);
        g_ctc_stress_misc->stats_id_list = NULL;
    }

    return CLI_SUCCESS;


}

#define   ________STRESS_CLI_INIT________

int32
ctc_stress_test_misc_cli_init(void)
{

    g_ctc_stress_misc = sal_malloc(sizeof(ctc_cli_stress_test_misc_t));
    if(!g_ctc_stress_misc)
    {
        return CLI_ERROR;
    }
    sal_memset(g_ctc_stress_misc, 0, sizeof(ctc_cli_stress_test_misc_t));

#if (FEATURE_MODE == 0)
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_wlan_tunnel_cmd);
#endif
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_scl_add_all_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_packet_tx_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_stats_create1_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_stats_create2_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_stats_create3_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_stats_destroy_cmd);

    return CLI_SUCCESS;
}
