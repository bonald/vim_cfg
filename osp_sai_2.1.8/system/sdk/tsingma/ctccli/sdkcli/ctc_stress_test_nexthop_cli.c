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

enum ctc_cli_stress_test_nexthop_type_e
{
    CTC_CLI_STRESS_NEXTHOP_TYPE_ARP,
    CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC,
    CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V4,
    CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V6,
    CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS,
    CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL,
    CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN,
    CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP,
    CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST,
    CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST_MEM,
    CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL,
    CTC_CLI_STRESS_NEXTHOP_TYPE_MISC,
    CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE,
    CTC_CLI_STRESS_NEXTHOP_TYPE_MAX
};
typedef enum ctc_cli_stress_test_nexthop_type_e ctc_cli_stress_test_nexthop_type_t;

struct ctc_cli_stress_test_nexthop_s
{
    uint32* p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MAX];
    uint16* p_arp_id;
    ctc_mcast_nh_param_group_t** p_mcast_member;
    uint8   mpls_nh_type;
    uint16  max_tunnel_id;
    uint32  entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MAX];

    uint16  arp_base;
    uint32  grp_base;
    uint32  nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_MAX];
    uint32  ori_mem_size[CTC_CLI_STRESS_NEXTHOP_TYPE_MAX];
};
typedef struct ctc_cli_stress_test_nexthop_s ctc_cli_stress_test_nexthop_t;

ctc_cli_stress_test_nexthop_t  *g_ctc_stress_nexthop;


STATIC int32
_ctc_stress_nexthop_cli_parse_common(char** argv, uint16 argc, ctc_cli_stress_t* stress, uint8 nh_type)
{
    uint8 tmp_argi = 0;
    if (g_ctc_stress_nexthop->entry_num[nh_type])
    {
        ctc_cli_out("Other stress test have used, please remove first! \n");
        return CLI_ERROR;
    }
    tmp_argi = CTC_CLI_GET_ARGC_INDEX("entry-num");
    if (0xFF != tmp_argi)
    {
        CTC_CLI_GET_UINT32_RANGE("entry-num", g_ctc_stress_nexthop->entry_num[nh_type], argv[tmp_argi + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random");
    if (tmp_argi != 0xFF)
    {
        stress->data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM;
    }
    tmp_argi = CTC_CLI_GET_ARGC_INDEX_ENHANCE("incr");
    if (tmp_argi != 0xFF)
    {
        stress->data_mode = CTC_CLI_STRESS_DATA_MODE_INCREASE;
    }
    tmp_argi = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random-all");
    if (tmp_argi != 0xFF)
    {
        stress->data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL;
    }

    stress->op_type = CTC_CLI_STRESS_OP_TYPE_ADD;

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != tmp_argi)
    {
        stress->test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != tmp_argi)
    {
        stress->test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }

    tmp_argi = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != tmp_argi)
    {
        stress->test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    return CLI_SUCCESS;
}

#define __ARP__
int32
_ctc_cli_stress_arp(ctc_cli_stress_t* stress, uint8 vlan_valid)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1;
    sal_systime_t tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};
    uint8 is_random = 0;
    uint8 is_random_all = 0;
    uint16 arp_id = 0;
    ctc_nh_nexthop_mac_param_t param;

    is_random = (CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode ) ? 1:0;
    is_random_all = (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode ) ? 1:0;

    sal_memset(&param, 0, sizeof(ctc_nh_nexthop_mac_param_t));

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    if (g_ctc_stress_nexthop->p_arp_id && (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type))
    {
        ctc_cli_out("Other arp stress have used, please remove first! \n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        g_ctc_stress_nexthop->p_arp_id = (uint16*)sal_malloc(g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_ARP]*sizeof(uint16));
        if (!g_ctc_stress_nexthop->p_arp_id)
        {
            return CLI_ERROR;
        }
    }

    for (loop = 0; loop < g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_ARP]; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
        {
            if (is_random || is_random_all)
            {
                arp_id = (is_random)?((sal_rand() % 16384) + 1):(loop + 1);
                *((uint32*)&param.mac[0]) = sal_rand();
                *((uint16*)&param.mac[4]) = sal_rand();
                param.vlan_id = (is_random)?((sal_rand() % 4095) + 1):((loop%4095) + 1);
                param.gport = (is_random)?(sal_rand() % 64):(loop%64);
            }
            else
            {
                arp_id = g_ctc_stress_nexthop->arp_base + loop;
                *((uint32*)&param.mac[0]) = loop+1;
                if (vlan_valid)
                {
                    param.vlan_id =((loop%4095) + 1);
                }
                param.gport = (loop%64);
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_add_nexthop_mac(g_api_lchip, arp_id, &param);
            }
            else
            {
                ret = ctc_nh_add_nexthop_mac(arp_id, &param);
            }

            if (ret == 0)
            {
                if (g_ctc_stress_nexthop && g_ctc_stress_nexthop->p_arp_id)
                {
                    *((uint16*)g_ctc_stress_nexthop->p_arp_id+ loop) = arp_id;
                }
            }
        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
        {
            if (!g_ctc_stress_nexthop || !g_ctc_stress_nexthop->p_arp_id)
            {
                ctc_cli_out("No Arp resource need to remove \n");
                return CLI_SUCCESS;
            }

            if (NULL == g_ctc_stress_nexthop->p_arp_id + loop)
            {
                continue;
            }

            arp_id = *(g_ctc_stress_nexthop->p_arp_id + loop);

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_remove_nexthop_mac(g_api_lchip, arp_id);
            }
            else
            {
                ret = ctc_nh_remove_nexthop_mac(arp_id);
            }
        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);

    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf,"test number:%5d", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_ARP]);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_ARP] = 0;
        if (g_ctc_stress_nexthop->p_arp_id)
        {
            sal_free(g_ctc_stress_nexthop->p_arp_id);
            g_ctc_stress_nexthop->p_arp_id = NULL;
        }
    }

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_arp_add,
        ctc_cli_stress_nexthop_arp_add_cmd,
        "nexthop arp add arp-base ARP (vlan-valid|)(random|incr|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        "Nexthop module",
        "Arp",
        "Add",
        "Arp Base",
        "Arp Id",
        "Vlan valid",
        "Random",
        "Increase",
        "Random all",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint32 index = 0xFF;
    mem_table_t mem_info;
    int8* m_name = NULL;
    mem_table_t mem_info1;
    uint8 vlan_valid = 0;

    ctc_cli_stress_t stress;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&mem_info, 0, sizeof(mem_table_t));
    sal_memset(&mem_info1, 0, sizeof(mem_table_t));

    CTC_CLI_GET_UINT16("arp-id", g_ctc_stress_nexthop->arp_base, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("vlan-valid");
    if (0xFF != index)
    {
        vlan_valid = 1;
    }

    ret = _ctc_stress_nexthop_cli_parse_common(&(argv[0]), argc, &stress, CTC_CLI_STRESS_NEXTHOP_TYPE_ARP);
    if (ret)
    {
        return ret;
    }

    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info, &m_name);
    g_ctc_stress_nexthop->ori_mem_size[CTC_CLI_STRESS_NEXTHOP_TYPE_ARP] = mem_info.size;
    /*1. create phy interface */
    {
        uint16 l3if_id = 0;
        ctc_l3if_t l3if;

        sal_memset(&l3if, 0, sizeof(ctc_l3if_t));
        l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;

        for (index = 0; index < 64; index++)
        {
            l3if.gport = index;
            l3if_id = index + 1;
            if(g_ctcs_api_en)
            {
                 ret = ctcs_l3if_create(g_api_lchip, l3if_id, &l3if);
            }
            else
            {
                ret = ctc_l3if_create(l3if_id, &l3if);
            }

            if (ret && (ret != CTC_E_EXIST))
            {
                ctc_cli_out("Create l3if failed, gport:0x%x ,ret:%s\n", index, ctc_get_error_desc(ret));
                return ret;
            }
        }
    }

    /*2. create 16k arp */
    ret = _ctc_cli_stress_arp(&stress, vlan_valid);
    if (ret)
    {
        /* g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_ARP] = 0; */
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info1, &m_name);

    /* ctc_cli_out("Install %u Arp Entry\n", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_ARP]); */

    return ret;

}


CTC_CLI(ctc_cli_stress_nexthop_arp_remove,
        ctc_cli_stress_nexthop_arp_remove_cmd,
        "nexthop arp remove(perf|)",
        "Nexthop module",
        "Arp",
        "Remove",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint16 index = 0;

    ctc_cli_stress_t stress;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_arp(&stress, 0);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }

    {
        uint16 l3if_id = 0;
        ctc_l3if_t l3if;

        sal_memset(&l3if, 0, sizeof(ctc_l3if_t));
        l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;

        for (index = 0; index < 64; index++)
        {
            l3if.gport = index;
            l3if_id = index + 1;
            if(g_ctcs_api_en)
            {
                 ctcs_l3if_destory(g_api_lchip, l3if_id, &l3if);
            }
            else
            {
                ctc_l3if_destory(l3if_id, &l3if);
            }
        }
    }

    return ret;
}

#define __IPUC__
int32
_ctc_cli_stress_ipuc_nh(ctc_cli_stress_t* stress)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1;
    sal_systime_t tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};
    uint8 is_random = 0;
    uint8 is_random_all = 0;
    ctc_ip_nh_param_t param;
    uint32 nhid = 0;
    uint32 temp1 = 0;
    uint32 temp2 = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = { 0 };

    is_random = (CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode ) ? 1:0;
    is_random_all = (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode ) ? 1:0;

    ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);

    sal_memset(&param, 0, sizeof(ctc_ip_nh_param_t));
    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC] && (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type))
    {
        ctc_cli_out("Other ipuc stress have used, please remove first! \n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC] = (uint32*)sal_malloc(g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC]*sizeof(uint32));
        if (!g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC])
        {
            return CLI_ERROR;
        }
    }

    for (loop = 0; loop < g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC]; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
        {
            nhid = g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC] + loop;
            if (is_random || is_random_all)
            {
                temp1  = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_CHIP_NUM]; /*gchip*/
                temp2  = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];   /*lport*/

                param.flag      = sal_rand();
                param.oif.gport = (CTC_MAP_LPORT_TO_GPORT(temp1,temp2));
                param.oif.vid   = (sal_rand()%4096);
                param.oif.is_l2_port = 1;
                *((uint32*)&param.mac[0]) = sal_rand();
                *((uint16*)&param.mac[4]) = sal_rand();
                *((uint32*)&param.mac_sa[0]) = sal_rand();
                *((uint16*)&param.mac_sa[4]) = sal_rand();
                param.p_oif.gport = (CTC_MAP_LPORT_TO_GPORT(temp1,temp2));
                param.p_oif.vid   = (sal_rand()%4096);
                param.p_oif.is_l2_port = 1;
                *((uint32*)&param.p_mac[0]) = sal_rand();
                *((uint16*)&param.p_mac[4]) = sal_rand();
                param.mtu_no_chk = 1;
                param.arp_id = g_ctc_stress_nexthop->arp_base+loop;
                param.p_arp_id = g_ctc_stress_nexthop->arp_base+loop;
            }
            else
            {
                param.arp_id = g_ctc_stress_nexthop->arp_base+loop;
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_add_ipuc(g_api_lchip, nhid, &param);
            }
            else
            {
                ret = ctc_nh_add_ipuc(nhid, &param);
            }

            if (ret == 0)
            {
                if (g_ctc_stress_nexthop && g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC])
                {
                    *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC] + loop) = nhid;
                }
            }
        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
        {
            if (!g_ctc_stress_nexthop || !g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC])
            {
                ctc_cli_out("No Ipuc resource need to remove \n");
                return CLI_SUCCESS;
            }

            if (NULL == g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC] + loop)
            {
                continue;
            }

            nhid = *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC] + loop);

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_remove_ipuc(g_api_lchip, nhid);
            }
            else
            {
                ret = ctc_nh_remove_ipuc(nhid);
            }
        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);

    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf,"test number:%5d", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC]);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC] = 0;

        if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC])
        {
            sal_free(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC]);
            g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC] = NULL;
        }
    }

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_ipuc_add,
        ctc_cli_stress_nexthop_ipuc_add_cmd,
        "nexthop ipuc add nh-base NHID arp ARP_BASE (random|incr|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        "Nexthop module",
        "Ipuc",
        "Add",
        "Nhid base",
        "Base value",
        "Arp",
        "Arp ID",
        "Random",
        "Increase",
        "Random all",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint32 index = 0xFF;
    mem_table_t mem_info;
    int8* m_name = NULL;
    mem_table_t mem_info1;

    ctc_cli_stress_t stress;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&mem_info, 0, sizeof(mem_table_t));
    sal_memset(&mem_info1, 0, sizeof(mem_table_t));

    CTC_CLI_GET_UINT32("nhid", g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC], argv[0]);
    CTC_CLI_GET_UINT32("arp id", g_ctc_stress_nexthop->arp_base, argv[1]);

    ret = _ctc_stress_nexthop_cli_parse_common(&(argv[0]), argc, &stress, CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC);
    if (ret)
    {
        return ret;
    }

    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info, &m_name);
    g_ctc_stress_nexthop->ori_mem_size[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC] = mem_info.size;
    /*1. create phy interface */
    {
        uint16 l3if_id = 0;
        ctc_l3if_t l3if;

        sal_memset(&l3if, 0, sizeof(ctc_l3if_t));
        l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;

        for (index = 0; index < 64; index++)
        {
            l3if.gport = index;
            l3if_id = index + 1;
            if(g_ctcs_api_en)
            {
                 ret = ctcs_l3if_create(g_api_lchip, l3if_id, &l3if);
            }
            else
            {
                ret = ctc_l3if_create(l3if_id, &l3if);
            }

            if (ret && (ret != CTC_E_EXIST))
            {
                ctc_cli_out("Create l3if failed, gport:0x%x ,ret:%s\n", index, ctc_get_error_desc(ret));
                return ret;
            }
        }
    }

    /*2. create ipuc */
    ret = _ctc_cli_stress_ipuc_nh(&stress);
    if (ret)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC] = 0;
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }

    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info1, &m_name);

    ctc_cli_out("Install %u ipuc Entry\n", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC]);

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_ipuc_remove,
        ctc_cli_stress_nexthop_ipuc_remove_cmd,
        "nexthop ipuc remove(perf|)",
        "Nexthop module",
        "Ipuc",
        "Remove",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint16 index = 0;

    ctc_cli_stress_t stress;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_ipuc_nh(&stress);

    {
        uint16 l3if_id = 0;
        ctc_l3if_t l3if;

        sal_memset(&l3if, 0, sizeof(ctc_l3if_t));
        l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;

        for (index = 0; index < 64; index++)
        {
            l3if.gport = index;
            l3if_id = index + 1;
            if(g_ctcs_api_en)
            {
                 ctcs_l3if_destory(g_api_lchip, l3if_id, &l3if);
            }
            else
            {
                ctc_l3if_destory(l3if_id, &l3if);
            }
        }
    }

    return ret;
}

#define __IP_TUNNEL__
int32
_ctc_cli_stress_ip_tunnel(ctc_ip_tunnel_nh_param_t* p_para, ctc_cli_stress_t* stress)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1;
    sal_systime_t tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};
    ctc_ip_tunnel_nh_param_t param;
    uint32 nhid = 0;
    mac_addr_t mac = {0};
    uint8 is_v6 = 0;
    uint32* p_nhid = NULL;
    uint32 tunnel_num = 0;

    is_v6 = (p_para->tunnel_info.tunnel_type == CTC_TUNNEL_TYPE_IPV4_IN6 ||
             p_para->tunnel_info.tunnel_type == CTC_TUNNEL_TYPE_IPV6_IN6 ||
             p_para->tunnel_info.tunnel_type == CTC_TUNNEL_TYPE_GRE_IN6 )?1:0;


    sal_memset(&param, 0, sizeof(ctc_ip_tunnel_nh_param_t));
    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    p_nhid = is_v6?(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V6]):
        (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V4]);
    tunnel_num = is_v6?g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V6]:
         g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V4];

    if (p_nhid && (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type))
    {
        ctc_cli_out("Other tunnel nexthop stress have used, please remove first! \n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        if (is_v6)
        {
            p_nhid = (uint32*)sal_malloc(g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V6]*sizeof(uint32));
            if (!p_nhid)
            {
                return CLI_ERROR;
            }
            g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V6] = p_nhid;
        }
        else
        {
            p_nhid = (uint32*)sal_malloc(g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V4]*sizeof(uint32));
            if (!p_nhid)
            {
                return CLI_ERROR;
            }
            g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V4] = p_nhid;
        }
    }

    for (loop = 0; loop < tunnel_num; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
        {
            if (is_v6)
            {
                nhid = g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V6] + loop;
            }
            else
            {
                nhid = g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V4] + loop;
            }
            param.arp_id = p_para->arp_id + (loop%1024);

            if ((CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode) || (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode))
            {
                param.flag = sal_rand();
                *((uint32*)&param.mac[0]) = sal_rand();
                *((uint16*)&param.mac[4]) = sal_rand();
                *((uint32*)&param.mac_sa[0]) = sal_rand();
                *((uint16*)&param.mac_sa[4]) = sal_rand();
                param.tunnel_info.dscp_domain = (sal_rand()%16);
                param.tunnel_info.tunnel_type = (sal_rand()%MAX_CTC_TUNNEL_TYPE);
                param.tunnel_info.ttl = (sal_rand()%256);
                param.tunnel_info.flow_label_mode = (sal_rand()%MAX_CTC_NH_FLOW_LABEL);
                param.tunnel_info.inner_packet_type = sal_rand();
                *((uint32*)&param.tunnel_info.inner_macda[0]) = sal_rand();
                *((uint16*)&param.tunnel_info.inner_macda[4]) = sal_rand();
                *((uint32*)&param.tunnel_info.inner_macsa[0]) = sal_rand();
                *((uint16*)&param.tunnel_info.inner_macsa[4]) = sal_rand();
                param.tunnel_info.logic_port = sal_rand();

                if (is_v6)
                {
                    param.tunnel_info.ip_sa.ipv6[0] = sal_rand();
                    param.tunnel_info.ip_sa.ipv6[1] = sal_rand();
                    param.tunnel_info.ip_sa.ipv6[2] = sal_rand();
                    param.tunnel_info.ip_sa.ipv6[3] = sal_rand();

                    param.tunnel_info.ip_da.ipv6[0] = sal_rand();
                    param.tunnel_info.ip_da.ipv6[1] = sal_rand();
                    param.tunnel_info.ip_da.ipv6[2] = sal_rand();
                    param.tunnel_info.ip_da.ipv6[3] = sal_rand();                }
                else
                {
                    param.tunnel_info.ip_sa.ipv4 = sal_rand();
                    param.tunnel_info.ip_da.ipv4 = sal_rand();
                }

            }
            else
            {
                if (sal_memcmp(p_para->mac_sa, mac, 6))
                {
                    *((uint32*)&param.mac_sa[0]) += loop;
                }

                param.mac[0] += loop;
                param.tunnel_info.inner_macda[0] += loop;
                param.tunnel_info.inner_macsa[0] += loop;
                param.tunnel_info.tunnel_type = p_para->tunnel_info.tunnel_type;
                param.tunnel_info.ttl = 100;
                param.tunnel_info.inner_packet_type = PKT_TYPE_ETH;

                if (is_v6)
                {
                    param.tunnel_info.ip_sa.ipv6[0] = ((loop%16)+1);
                    param.tunnel_info.ip_da.ipv6[0] = ((loop%16)+1);
               }
                else
                {
                    param.tunnel_info.ip_sa.ipv4 += loop;
                    param.tunnel_info.ip_da.ipv4 += loop;
                }

            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_add_ip_tunnel(g_api_lchip, nhid, &param);
            }
            else
            {
                ret = ctc_nh_add_ip_tunnel(nhid, &param);
            }

            if (ret == 0)
            {
                if (g_ctc_stress_nexthop && p_nhid)
                {
                    *(p_nhid + loop) = nhid;
                }
            }
        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
        {
            if (!g_ctc_stress_nexthop || !p_nhid)
            {
                ctc_cli_out("No Ipuc resource need to remove \n");
                return CLI_SUCCESS;
            }

            if (NULL == p_nhid + loop)
            {
                continue;
            }

            nhid = *(p_nhid + loop);

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_remove_ip_tunnel(g_api_lchip, nhid);
            }
            else
            {
                ret = ctc_nh_remove_ip_tunnel(nhid);
            }

        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);

    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf,"test number:%5d", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IPUC]);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
    {
        if (is_v6)
        {
            g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V6] = 0;
            g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V6] = NULL;
        }
        else
        {
            g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V4] = 0;
            g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V4] = NULL;
        }

        if (p_nhid)
        {
            sal_free(p_nhid);
            p_nhid = NULL;
        }
    }

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_ip_tunnel_v4_add,
        ctc_cli_stress_nexthop_ip_tunnel_v4_add_cmd,
        "nexthop ip-tunnel-v4 add nh-base NHID arp-id ARP (macsa|) (ip6in4|ip4in4|ip6to4 | 6rd |isatap|grein4)   \
            (random|incr|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        "Nexthop module",
        "Ip tunnel",
        "Add",
        "Nhid base",
        "Nhid",
        "Arp id",
        "Arp ID Value",
        "Mac sa valid",
        "Ip6in4 tunnel",
        "Ip4in4 tunnel",
        "Ip6to4 tunnel",
        "6Rd tunnel",
        "Isatap tunnel",
        "Grein4 tunnel",
        "Random",
        "Increase",
        "Random all",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint32 index = 0xFF;
    mem_table_t mem_info;
    int8* m_name = NULL;
    mem_table_t mem_info1;
    uint16 arp_base = 0;
    ctc_ip_tunnel_nh_param_t nh_para;
    ctc_cli_stress_t stress;

    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&mem_info, 0, sizeof(mem_table_t));
    sal_memset(&mem_info1, 0, sizeof(mem_table_t));
    sal_memset(&nh_para, 0, sizeof(ctc_ip_tunnel_nh_param_t));

    CTC_CLI_GET_UINT32("nhid", g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V4], argv[0]);
    CTC_CLI_GET_UINT16("arp-id", arp_base, argv[1]);
    nh_para.arp_id = arp_base;
    g_ctc_stress_nexthop->arp_base = arp_base;

    index = CTC_CLI_GET_ARGC_INDEX("macsa");
    if (0xFF != index)
    {
        nh_para.mac_sa[0] = 0;
        nh_para.mac_sa[1] = 0;
        nh_para.mac_sa[2] = 0;
        nh_para.mac_sa[3] = 0;
        nh_para.mac_sa[4] = 0;
        nh_para.mac_sa[5] = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip6in4");
    if (0xFF != index)
    {
        nh_para.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_IPV6_IN4;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip4in4");
    if (0xFF != index)
    {
        nh_para.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_IPV4_IN4;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip6to4");
    if (0xFF != index)
    {
        nh_para.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_6TO4;
    }

    index = CTC_CLI_GET_ARGC_INDEX("6rd");
    if (0xFF != index)
    {
        nh_para.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_6RD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("isatap");
    if (0xFF != index)
    {
        nh_para.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_ISATAP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("grein4");
    if (0xFF != index)
    {
        nh_para.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_GRE_IN4;
    }

    ret = (_ctc_stress_nexthop_cli_parse_common(&(argv[0]), argc, &stress, CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V4));
    if (ret)
    {
        return ret;
    }

    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info, &m_name);
    g_ctc_stress_nexthop->ori_mem_size[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V4] = mem_info.size;
    /*1. create phy interface */
    {
        uint16 l3if_id = 0;
        ctc_l3if_t l3if;

        sal_memset(&l3if, 0, sizeof(ctc_l3if_t));
        l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;

        for (index = 0; index < 64; index++)
        {
            l3if.gport = index;
            l3if_id = index + 1;
            if(g_ctcs_api_en)
            {
                 ret = ctcs_l3if_create(g_api_lchip, l3if_id, &l3if);
            }
            else
            {
                ret = ctc_l3if_create(l3if_id, &l3if);
            }

            if (ret && (ret != CTC_E_EXIST))
            {
                ctc_cli_out("Create l3if failed, gport:0x%x ,ret:%s\n", index, ctc_get_error_desc(ret));
                return ret;
            }
        }
    }

    ret = _ctc_cli_stress_ip_tunnel(&nh_para, &stress);
    if (ret)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V4] = 0;
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }

    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info1, &m_name);

    ctc_cli_out("Install %u ip-tunnel-v4 Entry\n", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V4]);

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_ip_tunnel_v4_remove,
        ctc_cli_stress_nexthop_ip_tunnel_v4_remove_cmd,
        "nexthop ip-tunnel-v4 remove (perf|)",
        "Nexthop module",
        "Ip tunnel",
        "Remove",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint16 index = 0;
    ctc_cli_stress_t stress;
    ctc_ip_tunnel_nh_param_t para;

    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&para, 0, sizeof(ctc_ip_tunnel_nh_param_t));

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_ip_tunnel(&para, &stress);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    {
        uint16 l3if_id = 0;
        ctc_l3if_t l3if;

        sal_memset(&l3if, 0, sizeof(ctc_l3if_t));
        l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;

        for (index = 0; index < 64; index++)
        {
            l3if.gport = index;
            l3if_id = index + 1;
            if(g_ctcs_api_en)
            {
                 ctcs_l3if_destory(g_api_lchip, l3if_id, &l3if);
            }
            else
            {
                ctc_l3if_destory(l3if_id, &l3if);
            }
        }
    }
    return ret;
}

CTC_CLI(ctc_cli_stress_nexthop_ip_tunnel_v6_add,
        ctc_cli_stress_nexthop_ip_tunnel_v6_add_cmd,
        "nexthop ip-tunnel-v6 add nh-base NHID arp-id ARP (macsa|) (ip4in6|ip6in6|grein6 ) (random|incr|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        "Nexthop module",
        "Ip tunnel",
        "Add",
        "Nhid base",
        "Nhid",
        "Arp id",
        "Arp ID Value",
        "Mac sa valid",
        "Ip4in6 tunnel",
        "Ip6in6 tunnel",
        "Grein6 tunnel",
        "Random",
        "Increase",
        "Random all",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint32 index = 0xFF;
    mem_table_t mem_info;
    int8* m_name = NULL;
    mem_table_t mem_info1;
    uint16 arp_base = 0;
    ctc_ip_tunnel_nh_param_t nh_para;
    ctc_cli_stress_t stress;

    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&mem_info, 0, sizeof(mem_table_t));
    sal_memset(&mem_info1, 0, sizeof(mem_table_t));
    sal_memset(&nh_para, 0, sizeof(ctc_ip_tunnel_nh_param_t));

    CTC_CLI_GET_UINT32("nhid", g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V6], argv[0]);

    CTC_CLI_GET_UINT16("arp-id", arp_base, argv[1]);
    nh_para.arp_id = arp_base;
    g_ctc_stress_nexthop->arp_base = arp_base;

    index = CTC_CLI_GET_ARGC_INDEX("macsa");
    if (0xFF != index)
    {
        nh_para.mac_sa[0] = 0;
        nh_para.mac_sa[1] = 0;
        nh_para.mac_sa[2] = 0;
        nh_para.mac_sa[3] = 0;
        nh_para.mac_sa[4] = 0;
        nh_para.mac_sa[5] = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip4in6");
    if (0xFF != index)
    {
        nh_para.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_IPV4_IN6;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip6in6");
    if (0xFF != index)
    {
        nh_para.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_IPV6_IN6;
    }

    index = CTC_CLI_GET_ARGC_INDEX("grein6");
    if (0xFF != index)
    {
        nh_para.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_GRE_IN6;
    }

    ret = (_ctc_stress_nexthop_cli_parse_common(&(argv[0]), argc, &stress, CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V6));
    if (ret)
    {
        return ret;
    }

    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info, &m_name);
    g_ctc_stress_nexthop->ori_mem_size[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V6] = mem_info.size;
    /*1. create phy interface */
    {
        uint16 l3if_id = 0;
        ctc_l3if_t l3if;

        sal_memset(&l3if, 0, sizeof(ctc_l3if_t));
        l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;

        for (index = 0; index < 64; index++)
        {
            l3if.gport = index;
            l3if_id = index + 1;
            if(g_ctcs_api_en)
            {
                 ret = ctcs_l3if_create(g_api_lchip, l3if_id, &l3if);
            }
            else
            {
                ret = ctc_l3if_create(l3if_id, &l3if);
            }

            if (ret && (ret != CTC_E_EXIST))
            {
                ctc_cli_out("Create l3if failed, gport:0x%x ,ret:%s\n", index, ctc_get_error_desc(ret));
                return ret;
            }
        }
    }

    ret = _ctc_cli_stress_ip_tunnel(&nh_para, &stress);
    if (ret)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V6] = 0;
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info1, &m_name);

    ctc_cli_out("Install %u ip-tunnel-v6 Entry\n", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_IP_TUNNEL_V6]);

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_ip_tunnel_v6_remove,
        ctc_cli_stress_nexthop_ip_tunnel_v6_remove_cmd,
        "nexthop ip-tunnel-v6 remove (perf|)",
        "Nexthop module",
        "Ip tunnel",
        "Remove",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint16 index = 0;
    ctc_cli_stress_t stress;
    ctc_ip_tunnel_nh_param_t para;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&para, 0, sizeof(ctc_ip_tunnel_nh_param_t));

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    para.tunnel_info.tunnel_type = CTC_TUNNEL_TYPE_IPV4_IN6;

    ret = _ctc_cli_stress_ip_tunnel(&para, &stress);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    {
        uint16 l3if_id = 0;
        ctc_l3if_t l3if;

        sal_memset(&l3if, 0, sizeof(ctc_l3if_t));
        l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;

        for (index = 0; index < 64; index++)
        {
            l3if.gport = index;
            l3if_id = index + 1;
            if(g_ctcs_api_en)
            {
                 ctcs_l3if_destory(g_api_lchip, l3if_id, &l3if);
            }
            else
            {
                ctc_l3if_destory(l3if_id, &l3if);
            }
        }
    }

    return ret;
}

#define __MPLS__
int32
_ctc_cli_stress_mpls_tunnel(ctc_mpls_nexthop_tunnel_param_t* mpls_tunnel, ctc_cli_stress_t* stress)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1;
    sal_systime_t tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};
    uint32 tunnel_id = 0;
    uint8 index = 0;
    uint32 temp1 = 0;
    uint32 temp2 = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = { 0 };

    if (mpls_tunnel->nh_param.label_num > 2)
    {
        return CLI_ERROR;
    }

    ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL] && (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type))
    {
        ctc_cli_out("Other mpls tunnel stress have used, please remove first! \n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL] = (uint32*)sal_malloc(g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL]*sizeof(uint32));
        if (!g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL])
        {
            return CLI_ERROR;
        }
    }

    /*create mpls tunnel*/
    for (loop = 0; loop < g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL]; loop++)
    {

        tunnel_id = g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL]+loop;

        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
        {
            if ((CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode) || (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode))
            {
                temp1  = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_CHIP_NUM]; /*gchip*/
                temp2  = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];   /*lport*/

                mpls_tunnel->nh_param.arp_id = g_ctc_stress_nexthop->arp_base+(loop%1024);
                *((uint32*)&mpls_tunnel->nh_param.mac[0]) = sal_rand();
                *((uint16*)&mpls_tunnel->nh_param.mac[4]) = sal_rand();
                *((uint32*)&mpls_tunnel->nh_param.mac_sa[0]) = sal_rand();
                *((uint16*)&mpls_tunnel->nh_param.mac_sa[4]) = sal_rand();
                mpls_tunnel->nh_param.oif.ecmp_if_id = sal_rand();
                mpls_tunnel->nh_param.oif.gport =(CTC_MAP_LPORT_TO_GPORT(temp1,temp2));
                mpls_tunnel->nh_param.oif.vid = (sal_rand()%4096);
                mpls_tunnel->nh_param.oif.is_l2_port = 1;

                for (index = 0; index < mpls_tunnel->nh_param.label_num; index++)
                {
                    mpls_tunnel->nh_param.tunnel_label[index].exp = (sal_rand()%8);
                    mpls_tunnel->nh_param.tunnel_label[index].lable_flag = sal_rand();
                    mpls_tunnel->nh_param.tunnel_label[index].ttl = (sal_rand()%256);
                    mpls_tunnel->nh_param.tunnel_label[index].exp_type = (sal_rand()%MAX_CTC_NH_EXP_SELECT_MODE);
                    mpls_tunnel->nh_param.tunnel_label[index].label = sal_rand();
                    mpls_tunnel->nh_param.tunnel_label[index].exp_domain = (sal_rand()%16);
                }
            }
            else
            {
                mpls_tunnel->nh_param.arp_id = g_ctc_stress_nexthop->arp_base+(loop%1024);
                mpls_tunnel->nh_param.mac[0] = loop;
                mpls_tunnel->nh_param.mac_sa[0] = loop;

                for (index = 0; index < mpls_tunnel->nh_param.label_num; index++)
                {
                    CTC_SET_FLAG(mpls_tunnel->nh_param.tunnel_label[index].lable_flag, CTC_MPLS_NH_LABEL_IS_VALID);                 \
                    mpls_tunnel->nh_param.tunnel_label[index].exp = (loop%8);
                    mpls_tunnel->nh_param.tunnel_label[index].ttl = ((loop%254)+1);
                    mpls_tunnel->nh_param.tunnel_label[index].label ++;
                    mpls_tunnel->nh_param.tunnel_label[index].exp_domain = (loop%8);
                }
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_add_mpls_tunnel_label(g_api_lchip, tunnel_id, mpls_tunnel);
            }
            else
            {
                ret = ctc_nh_add_mpls_tunnel_label(tunnel_id, mpls_tunnel);
            }

            if (ret == 0)
            {
                if (g_ctc_stress_nexthop && g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL])
                {
                    *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL]+ loop) = tunnel_id;
                }
                g_ctc_stress_nexthop->max_tunnel_id = tunnel_id;
            }
        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
        {
            if (!g_ctc_stress_nexthop || !g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL])
            {
                ctc_cli_out("No Mpls Tunnel resource need to remove \n");
                return CLI_SUCCESS;
            }

            if (NULL == g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL] + loop)
            {
                continue;
            }

            tunnel_id = *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL] + loop);

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_remove_mpls_tunnel_label(g_api_lchip, tunnel_id);
            }
            else
            {
                ret = ctc_nh_remove_mpls_tunnel_label(tunnel_id);
            }

        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }

        CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);

    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf,"test number:%5d", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL]);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL] = 0;

        if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL])
        {
            sal_free(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL]);
            g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL] = NULL;
        }
    }
    return ret;

}

STATIC int32
_ctc_cli_stress_mpls_encode_pop_para(ctc_mpls_nexthop_pop_param_t* p_pop, uint8 is_random, uint32 index)
{
    int32 ret = CTC_E_NONE;

    p_pop->ttl_mode = is_random?(sal_rand()%CTC_MPLS_MAX_TUNNEL_MODE):CTC_MPLS_TUNNEL_MODE_UNIFORM;
    p_pop->nh_com.mpls_nh_flag = is_random?sal_rand():0;
    p_pop->nh_com.opcode = is_random?(sal_rand()%CTC_MPLS_NH_OP_MAX):CTC_MPLS_NH_PHP;
    *((uint32*)&p_pop->nh_com.mac[0]) = is_random?sal_rand():index;
    *((uint16*)&p_pop->nh_com.mac[4]) = is_random?sal_rand():0;
    *((uint32*)&p_pop->nh_com.mac_sa[0]) = is_random?sal_rand():index;
    *((uint16*)&p_pop->nh_com.mac_sa[4]) = is_random?sal_rand():0;
    p_pop->nh_com.vlan_info.flag = is_random?sal_rand():0;
    p_pop->nh_com.vlan_info.cvlan_edit_type = is_random?(sal_rand()%CTC_VLAN_EGRESS_EDIT_MAX):CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
    p_pop->nh_com.vlan_info.svlan_edit_type = is_random?(sal_rand()%CTC_VLAN_EGRESS_EDIT_MAX):CTC_VLAN_EGRESS_EDIT_KEEP_VLAN_UNCHANGE;
    p_pop->nh_com.vlan_info.output_cvid = is_random?(sal_rand()%4096):1;
    p_pop->nh_com.vlan_info.output_svid = is_random?(sal_rand()%4096):1;
    p_pop->nh_com.vlan_info.edit_flag = is_random?sal_rand():0;
    p_pop->nh_com.vlan_info.stag_cos = is_random?(sal_rand()%8):0;
    p_pop->nh_com.vlan_info.user_vlanptr = is_random?sal_rand():0;

    return ret;
}

STATIC int32
_ctc_cli_stress_mpls_encode_push_para(ctc_mpls_nexthop_push_param_t* p_push, uint8 is_random, uint32 index)
{
    int32 ret = CTC_E_NONE;

    if (p_push->tunnel_id > g_ctc_stress_nexthop->max_tunnel_id)
    {
        p_push->tunnel_id = (p_push->tunnel_id%g_ctc_stress_nexthop->max_tunnel_id);
    }
    p_push->seq_num_index = is_random?sal_rand():0;
    p_push->mtu_check_en = is_random?sal_rand():0;
    p_push->martini_encap_type = is_random?(sal_rand()%2):0;
    p_push->martini_encap_valid = is_random?sal_rand():1;
    p_push->nh_com.mpls_nh_flag = is_random?sal_rand():0;
    p_push->nh_com.opcode = is_random?(sal_rand()%CTC_MPLS_NH_OP_MAX):CTC_MPLS_NH_PUSH_OP_L2VPN;
    *((uint32*)&p_push->nh_com.mac[0]) = is_random?sal_rand():index;
    *((uint16*)&p_push->nh_com.mac[4]) = is_random?sal_rand():0;
    *((uint32*)&p_push->nh_com.mac_sa[0]) = is_random?sal_rand():index;
    *((uint16*)&p_push->nh_com.mac_sa[4]) = is_random?sal_rand():0;
    p_push->nh_com.vlan_info.flag = is_random?sal_rand():0;
    p_push->nh_com.vlan_info.cvlan_edit_type = is_random?sal_rand():1;
    p_push->nh_com.vlan_info.svlan_edit_type = is_random?sal_rand():1;
    p_push->nh_com.vlan_info.output_cvid = is_random?(sal_rand()%CTC_VLAN_EGRESS_EDIT_MAX):1;
    p_push->nh_com.vlan_info.output_svid = is_random?(sal_rand()%CTC_VLAN_EGRESS_EDIT_MAX):1;
    p_push->nh_com.vlan_info.edit_flag = is_random?sal_rand():0;
    p_push->nh_com.vlan_info.stag_cos = is_random?(sal_rand()%8):0;
    p_push->nh_com.vlan_info.user_vlanptr = is_random?sal_rand():0;

    if (p_push->label_num)
    {
        if (!is_random)
        {
            p_push->push_label[0].label++;
        }
        else
        {
            p_push->push_label[0].label = sal_rand();
        }
    }

    return ret;
}

int32
_ctc_cli_stress_mpls(ctc_mpls_nexthop_param_t* p_para, ctc_cli_stress_t* stress)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1;
    sal_systime_t tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};
    uint32 nhid = 0;
    uint8  is_pop = 0;
    uint16 arp_base = p_para->nh_para.nh_param_pop.arp_id;
    uint16 p_arp_base = p_para->nh_p_para.nh_p_param_pop.arp_id;
    uint16 tunnel_base = p_para->nh_para.nh_param_push.tunnel_id;

    is_pop = (p_para->nh_prop == CTC_MPLS_NH_POP_TYPE)?1:0;

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS] && (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type))
    {
        ctc_cli_out("Other mpls nexthop stress have used, please remove first! \n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS] = (uint32*)sal_malloc(g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS]*sizeof(uint32));
        if (!g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS])
        {
            return CLI_ERROR;
        }
    }

    for (loop = 0; loop < g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS]; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
        {
            nhid = loop+g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS];
            if ((CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode) || (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode))
            {
                p_para->flag = sal_rand();
                p_para->logic_port = sal_rand();
                if (is_pop)
                {
                    p_para->nh_para.nh_param_pop.arp_id = loop + arp_base;

                    _ctc_cli_stress_mpls_encode_pop_para(&p_para->nh_para.nh_param_pop, 1, loop);
                    if (p_para->aps_en)
                    {
                        p_para->nh_p_para.nh_p_param_pop.arp_id = loop + p_arp_base;
                        _ctc_cli_stress_mpls_encode_pop_para(&p_para->nh_p_para.nh_p_param_pop, 1, loop);
                    }
                }
                else
                {
                    p_para->nh_para.nh_param_push.tunnel_id = tunnel_base+loop;
                    _ctc_cli_stress_mpls_encode_push_para(&p_para->nh_para.nh_param_push, 1, loop);
                     if (p_para->aps_en)
                    {
                        _ctc_cli_stress_mpls_encode_push_para(&p_para->nh_p_para.nh_p_param_push, 1, loop);
                    }
                }
            }
            else
            {
                if (is_pop)
                {
                    _ctc_cli_stress_mpls_encode_pop_para(&p_para->nh_para.nh_param_pop, 0, loop);
                    if (p_para->aps_en)
                    {
                        _ctc_cli_stress_mpls_encode_pop_para(&p_para->nh_p_para.nh_p_param_pop, 0, loop);
                    }
                }
                else
                {
                    p_para->nh_para.nh_param_push.tunnel_id = tunnel_base+loop;
                    _ctc_cli_stress_mpls_encode_push_para(&p_para->nh_para.nh_param_push, 0, loop);
                     if (p_para->aps_en)
                    {
                        _ctc_cli_stress_mpls_encode_push_para(&p_para->nh_p_para.nh_p_param_push, 0, loop);
                    }
                }
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_add_mpls(g_api_lchip, nhid, p_para);
            }
            else
            {
                ret = ctc_nh_add_mpls(nhid, p_para);
            }

            if (ret == 0)
            {
                if (g_ctc_stress_nexthop && g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS])
                {
                    *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS] + loop) = nhid;
                }
            }
        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
        {
            if (!g_ctc_stress_nexthop || !g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS])
            {
                ctc_cli_out("No Ipuc resource need to remove \n");
                return CLI_SUCCESS;
            }

            if (NULL == g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS] + loop)
            {
                continue;
            }

            nhid = *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS] + loop);

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_remove_mpls(g_api_lchip, nhid);
            }
            else
            {
                ret = ctc_nh_remove_mpls(nhid);
            }

        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);

    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf,"test number:%5d", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS]);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS] = 0;

        if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS])
        {
            sal_free(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS]);
            g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS] = NULL;
        }
    }
    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_mpls_add_tunnel,
        ctc_cli_stress_nexthop_mpls_add_tunnel_cmd,
        "nexthop mpls-tunnel add nh-base NHID arp-id ARP (aps-group-id ID |) (label-num NUM) (label0-base BASE0) (label1-base BASE1 |) (is-spme |) (random|incr|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        "Nexthop module",
        "Mpls tunnel",
        "Add",
        "Nhid base",
        "Nhid",
        "Arp id",
        "Arp ID Value",
        "Aps group",
        "Aps group ID",
        "Label num",
        "Tunnel label num",
        "Label0 base",
        "Value",
        "Label1 base",
        "Value",
        "Spme",
        "Random",
        "Increase",
        "Random all",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint32 index = 0xFF;
    mem_table_t mem_info;
    int8* m_name = NULL;
    mem_table_t mem_info1;
    uint16 arp_base = 0;
    ctc_mpls_nexthop_tunnel_param_t mpls_tunnel;
    ctc_cli_stress_t stress;

    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&mem_info, 0, sizeof(mem_table_t));
    sal_memset(&mem_info1, 0, sizeof(mem_table_t));
    sal_memset(&mpls_tunnel, 0, sizeof(ctc_mpls_nexthop_tunnel_param_t));

    CTC_CLI_GET_UINT32("nhid", g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL], argv[0]);

    CTC_CLI_GET_UINT16("arp-id", arp_base, argv[1]);
    g_ctc_stress_nexthop->arp_base = arp_base;
    mpls_tunnel.nh_param.arp_id = arp_base;

    index = CTC_CLI_GET_ARGC_INDEX("aps-group-id");
    if (0xFF != index)
    {
        CTC_SET_FLAG(mpls_tunnel.aps_flag, CTC_NH_MPLS_APS_EN);
        CTC_CLI_GET_UINT16_RANGE("aps group id", mpls_tunnel.aps_bridge_group_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-spme");
    if (0xFF != index)
    {
        mpls_tunnel.nh_param.is_spme = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("label-num");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("label num", mpls_tunnel.nh_param.label_num, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("label0-base");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("label 0", mpls_tunnel.nh_param.tunnel_label[0].label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("label1-base");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32_RANGE("label 1", mpls_tunnel.nh_param.tunnel_label[1].label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    ret = (_ctc_stress_nexthop_cli_parse_common(&(argv[0]), argc, &stress, CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL));
    if (ret)
    {
        return ret;
    }

    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info, &m_name);
    g_ctc_stress_nexthop->ori_mem_size[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL] = mem_info.size;
    /*1. create phy interface */
    {
        uint16 l3if_id = 0;
        ctc_l3if_t l3if;

        sal_memset(&l3if, 0, sizeof(ctc_l3if_t));
        l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;

        for (index = 0; index < 64; index++)
        {
            l3if.gport = index;
            l3if_id = index + 1;
            if(g_ctcs_api_en)
            {
                 ret = ctcs_l3if_create(g_api_lchip, l3if_id, &l3if);
            }
            else
            {
                ret = ctc_l3if_create(l3if_id, &l3if);
            }

            if (ret && (ret != CTC_E_EXIST))
            {
                ctc_cli_out("Create l3if failed, gport:0x%x ,ret:%s\n", index, ctc_get_error_desc(ret));
                return ret;
            }
        }
    }

    ret = _ctc_cli_stress_mpls_tunnel(&mpls_tunnel, &stress);
    if (ret)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL] = 0;
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info1, &m_name);

    ctc_cli_out("Install %u mpls tunnel entry\n", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS_TUNNEL]);

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_mpls_add,
        ctc_cli_stress_nexthop_mpls_add_cmd,
        "nexthop mpls add nh-base NHID (push (tunnel-base TUNNEL) (pw-label LABEL |) |pop (arp-id ARP)) (aps-group-id ID (p-tunnel-base BASE)(p-pw-label LABEL |) |)\
            (random|incr|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        "Nexthop module",
        "Mpls nexthop",
        "Add",
        "Nhid base",
        "Base value",
        "Push",
        "Tunnel base",
        "Value",
        "Pw label",
        "pw label value",
        "Pop",
        "Arp id",
        "Arp ID Value",
        "Aps group",
        "Aps group ID",
        "Protect tunnel base",
        "Tunnel base",
        "Protect pw label",
        "Value",
        "Random",
        "Increase",
        "Random all",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint32 index = 0xFF;
    mem_table_t mem_info;
    int8* m_name = NULL;
    mem_table_t mem_info1;
    uint16 arp_base = 0;
    ctc_mpls_nexthop_param_t nh_para;
    ctc_cli_stress_t stress;

    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&mem_info, 0, sizeof(mem_table_t));
    sal_memset(&mem_info1, 0, sizeof(mem_table_t));
    sal_memset(&nh_para, 0, sizeof(ctc_mpls_nexthop_param_t));

    CTC_CLI_GET_UINT32("Nexthop ID", g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS], argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("pop");
    if (0xFF != index)
    {
        nh_para.nh_prop = CTC_MPLS_NH_POP_TYPE;

        index = CTC_CLI_GET_ARGC_INDEX("arp-id");
        {
            CTC_CLI_GET_UINT16_RANGE("arp id", arp_base, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
            nh_para.nh_para.nh_param_pop.arp_id = arp_base;
            g_ctc_stress_nexthop->arp_base = arp_base;
        }
    }
    else
    {
        nh_para.nh_prop = CTC_MPLS_NH_PUSH_TYPE;

        index = CTC_CLI_GET_ARGC_INDEX("tunnel-base");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("tunnel base", nh_para.nh_para.nh_param_push.tunnel_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("pw-label");
        if (0xFF != index)
        {
            nh_para.nh_para.nh_param_push.label_num = 1;
            CTC_CLI_GET_UINT32_RANGE("pw-label", nh_para.nh_para.nh_param_push.push_label[0].label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }
    }

    g_ctc_stress_nexthop->mpls_nh_type = nh_para.nh_prop;

    index = CTC_CLI_GET_ARGC_INDEX("aps-group-id");
    if (0xFF != index)
    {
        nh_para.aps_en = 1;
        CTC_CLI_GET_UINT16_RANGE("aps group id", nh_para.aps_bridge_group_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);

        index = CTC_CLI_GET_ARGC_INDEX("p-tunnel-base");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT16_RANGE("protect tunnel", nh_para.nh_p_para.nh_p_param_push.tunnel_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }

        index = CTC_CLI_GET_ARGC_INDEX("p-pw-label");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT32_RANGE("protect label", nh_para.nh_p_para.nh_p_param_push.push_label[0].label, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
        }
    }

    ret = (_ctc_stress_nexthop_cli_parse_common(&(argv[0]), argc, &stress, CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS));
    if (ret)
    {
        return ret;
    }

    if (!g_ctc_stress_nexthop->max_tunnel_id && g_ctc_stress_nexthop->mpls_nh_type == CTC_MPLS_NH_PUSH_TYPE)
    {
        ctc_cli_out("No MPLS tunnel resource \n");
        return CLI_ERROR;
    }

    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info, &m_name);
    g_ctc_stress_nexthop->ori_mem_size[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS] = mem_info.size;

    if (g_ctc_stress_nexthop->mpls_nh_type == CTC_MPLS_NH_POP_TYPE)
    {
        /*1. create phy interface */
        {
            uint16 l3if_id = 0;
            ctc_l3if_t l3if;

            sal_memset(&l3if, 0, sizeof(ctc_l3if_t));
            l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;

            for (index = 0; index < 64; index++)
            {
                l3if.gport = index;
                l3if_id = index + 1;
                if(g_ctcs_api_en)
                {
                     ret = ctcs_l3if_create(g_api_lchip, l3if_id, &l3if);
                }
                else
                {
                    ret = ctc_l3if_create(l3if_id, &l3if);
                }

                if (ret && (ret != CTC_E_EXIST))
                {
                    ctc_cli_out("Create l3if failed, gport:0x%x ,ret:%s\n", index, ctc_get_error_desc(ret));
                    return ret;
                }
            }
        }
    }

    ret = _ctc_cli_stress_mpls(&nh_para, &stress);
    if (ret)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS] = 0;
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info1, &m_name);

    ctc_cli_out("Install %u mpls Entry\n", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MPLS]);

    return ret;

}


CTC_CLI(ctc_cli_stress_nexthop_mpls_remove_tunnel,
        ctc_cli_stress_nexthop_mpls_remove_tunnel_cmd,
        "nexthop mpls-tunnel remove (perf|)",
        "Nexthop module",
        "Mpls tunnel",
        "Remove",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint16 index = 0;
    ctc_mpls_nexthop_tunnel_param_t para;
    ctc_cli_stress_t stress;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&para, 0, sizeof(ctc_mpls_nexthop_tunnel_param_t));

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_mpls_tunnel(&para, &stress);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    {
        uint16 l3if_id = 0;
        ctc_l3if_t l3if;

        sal_memset(&l3if, 0, sizeof(ctc_l3if_t));
        l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;

        for (index = 0; index < 64; index++)
        {
            l3if.gport = index;
            l3if_id = index + 1;
            if(g_ctcs_api_en)
            {
                 ctcs_l3if_destory(g_api_lchip, l3if_id, &l3if);
            }
            else
            {
                ctc_l3if_destory(l3if_id, &l3if);
            }
        }
    }

    return ret;
}


CTC_CLI(ctc_cli_stress_nexthop_mpls_remove,
        ctc_cli_stress_nexthop_mpls_remove_cmd,
        "nexthop mpls remove (perf|)",
        "Nexthop module",
        "Mpls",
        "Remove",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint16 index = 0;
    ctc_mpls_nexthop_param_t para;
    ctc_cli_stress_t stress;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&para, 0, sizeof(ctc_mpls_nexthop_param_t));

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_mpls(&para, &stress);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    if (g_ctc_stress_nexthop->mpls_nh_type == CTC_MPLS_NH_POP_TYPE)
    {
        {
            uint16 l3if_id = 0;
            ctc_l3if_t l3if;

            sal_memset(&l3if, 0, sizeof(ctc_l3if_t));
            l3if.l3if_type = CTC_L3IF_TYPE_PHY_IF;

            for (index = 0; index < 64; index++)
            {
                l3if.gport = index;
                l3if_id = index + 1;
                if(g_ctcs_api_en)
                {
                     ctcs_l3if_destory(g_api_lchip, l3if_id, &l3if);
                }
                else
                {
                    ctc_l3if_destory(l3if_id, &l3if);
                }
            }
        }
    }

    return ret;
}
#define __WLAN__
int32
_ctc_cli_stress_wlan(ctc_nh_wlan_tunnel_param_t* p_para, ctc_cli_stress_t* stress)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1;
    sal_systime_t tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};
    uint32 nhid = 0;
    uint8 is_v6 = 0;
    ipv6_addr_t ipv6_tmp[16];
    ip_addr_t ipv4_tmp[16] = {0};
    uint8  cnt = 0;

    is_v6 = CTC_FLAG_ISSET(p_para->flag, CTC_NH_WLAN_TUNNEL_FLAG_IPV6);

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN] && (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type))
    {
        ctc_cli_out("Other wlan nexthop stress have used, please remove first! \n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN] = (uint32*)sal_malloc(g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN]*sizeof(uint32));
        if (!g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN])
        {
            return CLI_ERROR;
        }
    }

    for (loop = 0; loop < g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN]; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
        {
            nhid = loop+g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN];

            if ((CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode) || (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode))
            {
                p_para->flag = sal_rand();
                if (is_v6)
                {
                    p_para->ip_sa.ipv6[0] = (cnt<16)?sal_rand():ipv6_tmp[(loop%16)][0];
                    p_para->ip_sa.ipv6[1] = (cnt<16)?sal_rand():ipv6_tmp[(loop%16)][1];
                    p_para->ip_sa.ipv6[2] = (cnt<16)?sal_rand():ipv6_tmp[(loop%16)][2];
                    p_para->ip_sa.ipv6[3] = (cnt<16)?sal_rand():ipv6_tmp[(loop%16)][3];

                    if (cnt < 16)
                    {
                        ipv6_tmp[cnt][0] = p_para->ip_sa.ipv6[0];
                        ipv6_tmp[cnt][1] = p_para->ip_sa.ipv6[1];
                        ipv6_tmp[cnt][2] = p_para->ip_sa.ipv6[2];
                        ipv6_tmp[cnt][3] = p_para->ip_sa.ipv6[3];
                        cnt++;
                    }

                    p_para->ip_da.ipv6[0] = sal_rand();
                    p_para->ip_da.ipv6[1] = sal_rand();
                    p_para->ip_da.ipv6[2] = sal_rand();
                    p_para->ip_da.ipv6[3] = sal_rand();
                }
                else
                {
                    p_para->ip_sa.ipv4 = (cnt<16)?sal_rand():ipv4_tmp[(loop%16)];
                    p_para->ip_da.ipv4 = (cnt<16)? sal_rand():ipv4_tmp[(loop%16)];

                    if (cnt < 16)
                    {
                        ipv4_tmp[cnt] = p_para->ip_sa.ipv4;
                        cnt++;
                    }
                }

                p_para->flow_label = sal_rand();
                p_para->flow_label_mode = (sal_rand()%MAX_CTC_NH_FLOW_LABEL);
                p_para->l4_src_port = sal_rand();
                p_para->l4_dst_port = sal_rand();
                p_para->ttl = (sal_rand()%256);
                p_para->dscp_domain = (sal_rand()%16);
                p_para->cos_domain = (sal_rand()%8);
                p_para->radio_id = sal_rand();
                p_para->dot11_sub_type = (sal_rand()%16);
                p_para->bssid_bitmap = sal_rand();
                p_para->vlan_id = (sal_rand()%4096);
                p_para->encrypt_id = sal_rand();
                p_para->logic_port = sal_rand();
                p_para->stats_id = 0;

                *((uint32*)&p_para->radio_mac[0]) = sal_rand();
                *((uint16*)&p_para->radio_mac[4]) = sal_rand();
                *((uint32*)&p_para->mac_da[0]) = sal_rand();
                *((uint16*)&p_para->mac_da[4]) = sal_rand();

            }
            else
            {
                if (is_v6)
                {
                    p_para->ip_sa.ipv6[3] =(cnt < 16)?(p_para->ip_sa.ipv6[3]+1):ipv6_tmp[(loop%16)][3];
                    p_para->ip_da.ipv6[3]++;

                    if (cnt < 16)
                    {
                        ipv6_tmp[cnt][3] = p_para->ip_sa.ipv6[3];
                        cnt++;
                    }
                }
                else
                {
                    p_para->ip_sa.ipv4 =(cnt < 16)?(p_para->ip_sa.ipv4+1):ipv4_tmp[(loop%16)];
                    p_para->ip_da.ipv4++;
                    if (cnt < 16)
                    {
                        ipv4_tmp[cnt] = p_para->ip_sa.ipv4;
                        cnt++;
                    }
                }

                p_para->flow_label = 100;
                p_para->flow_label_mode = CTC_NH_FLOW_LABEL_ASSIGN;
                p_para->l4_src_port++;
                p_para->l4_dst_port++;
                p_para->dscp_select = CTC_NH_DSCP_SELECT_MAP;
                p_para->dscp_or_tos = 0;
                p_para->dscp_domain = 0;
                p_para->cos_domain = 0;
                p_para->radio_id++;
                p_para->dot11_sub_type = ((p_para->dot11_sub_type+1)%16);
                p_para->bssid_bitmap = 0;
                p_para->vlan_id = ((p_para->vlan_id+1)%4095)+1;
                p_para->is_pop = 0;
                p_para->encrypt_id++;

                *((uint32*)&p_para->radio_mac[0]) = *((uint32*)&p_para->radio_mac[0]) + 1;

                if (p_para->mac_da[0] == 0x55)
                {
                    *((uint32*)&p_para->mac_da[0]) = *((uint32*)&p_para->mac_da[0]) + 1;
                }
                else
                {
                    *((uint32*)&p_para->mac_da[0]) = 0;
                }

            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_add_wlan_tunnel(g_api_lchip, nhid, p_para);
            }
            else
            {
                ret = ctc_nh_add_wlan_tunnel(nhid, p_para);
            }

            if (ret == 0)
            {
                if (g_ctc_stress_nexthop && g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN])
                {
                    *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN] + loop) = nhid;
                }
            }
        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
        {
            if (!g_ctc_stress_nexthop || !g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN])
            {
                ctc_cli_out("No Ipuc resource need to remove \n");
                return CLI_SUCCESS;
            }

            if (NULL == g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN] + loop)
            {
                continue;
            }

            nhid = *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN] + loop);

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_remove_wlan_tunnel(g_api_lchip, nhid);
            }
            else
            {
                ret = ctc_nh_remove_wlan_tunnel(nhid);
            }
        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);

    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf,"test number:%5d", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN]);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN] = 0;

        if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN])
        {
            sal_free(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN]);
            g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN] = NULL;
        }
    }

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_wlan_add,
        ctc_cli_stress_nexthop_wlan_add_cmd,
        "nexthop wlan add nh-base NHID (tunnel-v4 {ipsa A.B.C.D|ipda A.B.C.D|} | tunnel-v6 {ipsa X:X::X:X|ipda X:X::X:X|}) {mac-da|radio-mac|multicast-en|split-mac|is-route-packet|wds|map-cos|}  \
            (random|incr|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        "Nexthop module",
        "Wlan tunnel",
        "Add",
        "NHID base",
        "NHID",
        "Tunnel v4",
        "Ipv4 sa",
        CTC_CLI_IPV4_FORMAT,
        "Ipv4 da",
        CTC_CLI_IPV4_FORMAT,
        "Tunnel v6",
        "Ipv6 sa",
        CTC_CLI_IPV6_FORMAT,
        "Ipv6 da",
        CTC_CLI_IPV6_FORMAT,
        "Mac da edit",
        "Radio mac valid",
        "Mucast enable",
        "Split mac",
        "Route packet",
        "wds",
        "Cos map",
        "Random",
        "Increase",
        "Random all",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint32 index = 0xFF;
    mem_table_t mem_info;
    int8* m_name = NULL;
    mem_table_t mem_info1;
    ctc_cli_stress_t stress;
    ctc_nh_wlan_tunnel_param_t nh_para;
    ipv6_addr_t ipv6_address;

    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&mem_info, 0, sizeof(mem_table_t));
    sal_memset(&mem_info1, 0, sizeof(mem_table_t));
    sal_memset(&nh_para, 0, sizeof(ctc_nh_wlan_tunnel_param_t));

    CTC_CLI_GET_UINT32("Nexthop ID", g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN], argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("tunnel-v4");
    if (0xFF != index)
    {
        /*IPSA IPDA*/
        index = CTC_CLI_GET_ARGC_INDEX("ipsa");
        if (0xFF != index)
        {
            CTC_CLI_GET_IPV4_ADDRESS("ipsa", nh_para.ip_sa.ipv4, argv[index + 1]);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipda");
        if (0xFF != index)
        {
            CTC_CLI_GET_IPV4_ADDRESS("ipda", nh_para.ip_da.ipv4, argv[index + 1]);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("tunnel-v6");
    if (0xFF != index)    /*tunnel v6 ip header info*/
    {
        CTC_SET_FLAG(nh_para.flag, CTC_NH_WLAN_TUNNEL_FLAG_IPV6);
        /*IPSA IPDA*/
        index = CTC_CLI_GET_ARGC_INDEX("ipsa");
        if (0xFF != index)
        {
            CTC_CLI_GET_IPV6_ADDRESS("ipsa", ipv6_address, argv[index + 1]);
            /* adjust endian */
            nh_para.ip_sa.ipv6[0] = sal_htonl(ipv6_address[0]);
            nh_para.ip_sa.ipv6[1] = sal_htonl(ipv6_address[1]);
            nh_para.ip_sa.ipv6[2] = sal_htonl(ipv6_address[2]);
            nh_para.ip_sa.ipv6[3] = sal_htonl(ipv6_address[3]);
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipda");
        if (0xFF != index)
        {
            CTC_CLI_GET_IPV6_ADDRESS("ipda", ipv6_address, argv[index + 1]);
            /* adjust endian */
            nh_para.ip_da.ipv6[0] = sal_htonl(ipv6_address[0]);
            nh_para.ip_da.ipv6[1] = sal_htonl(ipv6_address[1]);
            nh_para.ip_da.ipv6[2] = sal_htonl(ipv6_address[2]);
            nh_para.ip_da.ipv6[3] = sal_htonl(ipv6_address[3]);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("mac-da");
    if (0xFF != index)
    {
        /*special flag indicate edit macda*/
        nh_para.mac_da[0] = 0x55;
    }

    index = CTC_CLI_GET_ARGC_INDEX("radio-mac");
    if (0xFF != index)
    {
        CTC_SET_FLAG(nh_para.flag, CTC_NH_WLAN_TUNNEL_FLAG_RADIO_MAC_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("multicast-en");
    if (0xFF != index)
    {
        CTC_SET_FLAG(nh_para.flag, CTC_NH_WLAN_TUNNEL_FLAG_MC_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("split-mac");
    if (0xFF != index)
    {
        CTC_SET_FLAG(nh_para.flag, CTC_NH_WLAN_TUNNEL_FLAG_SPLIT_MAC_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-route-packet");
    if (0xFF != index)
    {
        CTC_SET_FLAG(nh_para.flag, CTC_NH_WLAN_TUNNEL_FLAG_IS_ROUTE_PACKET);
    }

    index = CTC_CLI_GET_ARGC_INDEX("wds");
    if (0xFF != index)
    {
        CTC_SET_FLAG(nh_para.flag, CTC_NH_WLAN_TUNNEL_FLAG_WDS_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("map-cos");
    if (0xFF != index)
    {
        CTC_SET_FLAG(nh_para.flag, CTC_NH_WLAN_TUNNEL_FLAG_MAP_COS);
    }

    ret = (_ctc_stress_nexthop_cli_parse_common(&(argv[0]), argc, &stress, CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN));
    if (ret)
    {
        return ret;
    }

    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info, &m_name);
    g_ctc_stress_nexthop->ori_mem_size[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN] = mem_info.size;

    ret = _ctc_cli_stress_wlan(&nh_para, &stress);
    if (ret)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN] = 0;
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info1, &m_name);

    ctc_cli_out("Install %u wlan tunnel Entry\n", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_WLAN]);

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_wlan_remove,
        ctc_cli_stress_nexthop_wlan_remove_cmd,
        "nexthop wlan remove (perf|)",
        "Nexthop module",
        "Wlan tunnel",
        "Remove",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    ctc_nh_wlan_tunnel_param_t para;
    ctc_cli_stress_t stress;
    uint8 index = 0;

    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&para, 0, sizeof(ctc_nh_wlan_tunnel_param_t));

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_wlan(&para, &stress);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }

    return ret;
}
#define __ECMP__
int32
_ctc_cli_stress_ecmp(ctc_nh_ecmp_nh_param_t* p_para, ctc_cli_stress_t* stress)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1;
    sal_systime_t tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};
    uint32 nhid = 0;
    uint16 index = 0;

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP] && (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type))
    {
        ctc_cli_out("Other ecmp nexthop stress have used, please remove first! \n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP] = (uint32*)sal_malloc(g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP]*sizeof(uint32));
        if (!g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP])
        {
            return CLI_ERROR;
        }
    }

    for (loop = 0; loop < g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP]; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
        {
            nhid = g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP] + loop;
            if ((CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode) || (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode))
            {
            }
            else
            {
            }

            for (index = 0; index < ((p_para->nh_num>=64)?63:p_para->nh_num); index++)
            {
                p_para->nhid[index] = p_para->nhid[0]+index;
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_add_ecmp(g_api_lchip, nhid, p_para);
            }
            else
            {
                ret = ctc_nh_add_ecmp(nhid, p_para);
            }

            if (ret == 0)
            {
                if (g_ctc_stress_nexthop && g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP])
                {
                    *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP] + loop) = nhid;
                }
            }
        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
        {
            if (!g_ctc_stress_nexthop || !g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP])
            {
                ctc_cli_out("No Ipuc resource need to remove \n");
                return CLI_SUCCESS;
            }

            if (NULL == g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP] + loop)
            {
                continue;
            }

            nhid = *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP] + loop);

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_remove_ecmp(g_api_lchip, nhid);
            }
            else
            {
                ret = ctc_nh_remove_ecmp(nhid);
            }

            if (ret == CTC_E_NONE)
            {
                 /*-ctc_cli_out("Remove Arp %d normal\n", loop+1);*/
            }
            else
            {
                ctc_cli_out("Remove NHID %d error: %s\n", loop+1, ctc_get_error_desc(ret));
            }
        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);

    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf,"test number:%5d", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP]);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP] = 0;

        if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP])
        {
            sal_free(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP]);
            g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP] = NULL;
        }
    }

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_ecmp_add,
        ctc_cli_stress_nexthop_ecmp_add_cmd,
        "nexthop ecmp add nh-base NHID member-base BASE member-num NUM (xerspan |)(random|incr|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        "Nexthop module",
        "Ecmp nexthop",
        "Add",
        "Ecmp nhid base",
        "NHID",
        "Ecmp Member base",
        "Ecmp member nhid",
        "Xerspan",
        "Random",
        "Increase",
        "Random all",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint32 index = 0xFF;
    mem_table_t mem_info;
    int8* m_name = NULL;
    mem_table_t mem_info1;
    ctc_nh_ecmp_nh_param_t nh_para;

    ctc_cli_stress_t stress;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&mem_info, 0, sizeof(mem_table_t));
    sal_memset(&mem_info1, 0, sizeof(mem_table_t));
    sal_memset(&nh_para, 0, sizeof(ctc_nh_ecmp_nh_param_t));

    CTC_CLI_GET_UINT32("Nexthop ID", g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP], argv[0]);
    CTC_CLI_GET_UINT32("Member ID", nh_para.nhid[0], argv[1]);
    CTC_CLI_GET_UINT32("Member num", nh_para.nh_num, argv[2]);

    nh_para.type = CTC_NH_ECMP_TYPE_STATIC;

    index = CTC_CLI_GET_ARGC_INDEX("xerspan");
    if (0xFF != index)
    {
        nh_para.type = CTC_NH_ECMP_TYPE_XERSPAN;
    }

    ret = (_ctc_stress_nexthop_cli_parse_common(&(argv[0]), argc, &stress, CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP));
    if (ret)
    {
        return ret;
    }

    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info, &m_name);
    g_ctc_stress_nexthop->ori_mem_size[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP] = mem_info.size;

    ret = _ctc_cli_stress_ecmp(&nh_para, &stress);
    if (ret)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP] = 0;
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info1, &m_name);

    ctc_cli_out("Install %u Ecmp Entry\n", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_ECMP]);

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_ecmp_remove,
        ctc_cli_stress_nexthop_ecmp_remove_cmd,
        "nexthop ecmp remove (perf|)",
        "Nexthop module",
        "Ecmp",
        "Remove",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    ctc_nh_ecmp_nh_param_t para;
    ctc_cli_stress_t stress;
    uint8 index = 0;

    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&para, 0, sizeof(ctc_nh_ecmp_nh_param_t));

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_ecmp(&para, &stress);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }

    return ret;
}
#define __MCAST__
int32
_ctc_cli_stress_mcast(ctc_cli_stress_t* stress)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1;
    sal_systime_t tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};
    uint32 nhid = 0;
    ctc_l2dflt_addr_t l2_grp;
    ctc_mcast_nh_param_group_t mcast_para;
    uint32 temp1 = 0;
    uint32 temp2 = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = { 0 };

    sal_memset(&l2_grp, 0, sizeof(ctc_l2dflt_addr_t));
    sal_memset(&mcast_para, 0, sizeof(ctc_mcast_nh_param_group_t));

    ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST] && (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type))
    {
        ctc_cli_out("Other mcast nexthop stress have used, please remove first! \n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST] = (uint32*)sal_malloc(g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST]*sizeof(uint32));
        if (!g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST])
        {
            return CLI_ERROR;
        }

        g_ctc_stress_nexthop->p_mcast_member = (ctc_mcast_nh_param_group_t**)sal_malloc(g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST]*sizeof(void*));
        if (!g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST])
        {
            return CLI_ERROR;
        }
    }

    for (loop = 0; loop < g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST]; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
        {
            nhid = g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST]+loop;
            if ((CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode) || (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode))
            {
                temp1  = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_CHIP_NUM]; /*gchip*/
                temp2  = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];   /*lport*/

                mcast_para.mc_grp_id = g_ctc_stress_nexthop->grp_base+loop;
                mcast_para.opcode = 0;
                mcast_para.mem_info.member_type = (sal_rand()%CTC_NH_PARAM_MEM_INVALID);
                mcast_para.mem_info.destid = (CTC_MAP_LPORT_TO_GPORT(temp1,temp2));
                mcast_para.mem_info.vid = (sal_rand()%4096);
                mcast_para.mem_info.l3if_type = CTC_L3IF_TYPE_VLAN_IF;
                mcast_para.mem_info.is_vlan_port = 1;
                mcast_para.mem_info.port_bmp_en = sal_rand();
                mcast_para.mem_info.port_bmp[0] = sal_rand();
            }
            else
            {
                mcast_para.mc_grp_id = g_ctc_stress_nexthop->grp_base+loop;
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_add_mcast(g_api_lchip, nhid, &mcast_para);
            }
            else
            {
                ret = ctc_nh_add_mcast(nhid, &mcast_para);
            }

            if (ret == 0)
            {
                if (g_ctc_stress_nexthop && g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST])
                {
                    *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST]+ loop) = nhid;
                }
            }

        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
        {
            if (!g_ctc_stress_nexthop)
            {
                ctc_cli_out("No Mcast resource need to remove \n");
                return CLI_SUCCESS;
            }

            if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST])
            {
                if (NULL == g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST] + loop)
                {
                    continue;
                }

                nhid = *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST] + loop);

                if(g_ctcs_api_en)
                {
                    ret = ctcs_nh_remove_mcast(g_api_lchip, nhid);
                }
                else
                {
                    ret = ctc_nh_remove_mcast(nhid);
                }
            }
        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);

    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf,"test number:%5d", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST]);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type && g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST])
    {
        sal_free(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST]);
        sal_free(g_ctc_stress_nexthop->p_mcast_member);
        g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST] = NULL;
        g_ctc_stress_nexthop->p_mcast_member = NULL;
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST] = 0;
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    return ret;

}

int32
_ctc_cli_stress_mcast_member(ctc_mcast_nh_param_group_t*p_para, ctc_cli_stress_t* stress, uint32 nhid)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1;
    sal_systime_t tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};
    ctc_mcast_nh_param_group_t* p_mcast_para;
    uint32 temp1 = 0;
    uint32 temp2 = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = { 0 };

    ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    if (!g_ctc_stress_nexthop->p_mcast_member)
    {
        ctc_cli_out("Mcast stress group not create! \n");
        return CLI_ERROR;
    }

    if (nhid >= (g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST]+
                    g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST]))
    {
        return CLI_ERROR;
    }

    if (g_ctc_stress_nexthop->p_mcast_member[nhid] && (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type))
    {
        ctc_cli_out("Other mcast member stress have used, please remove first, nhid:%d! \n", loop);
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        g_ctc_stress_nexthop->p_mcast_member[nhid] = (ctc_mcast_nh_param_group_t*)sal_malloc(g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST_MEM]*sizeof(ctc_mcast_nh_param_group_t));
        if (!g_ctc_stress_nexthop->p_mcast_member[nhid])
        {
            return CLI_ERROR;
        }
    }

    for (loop = 0; loop < g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST_MEM]; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
        {
            if ((CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode) || (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode))
            {
                temp1  = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_CHIP_NUM]; /*gchip*/
                temp2  = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];   /*lport*/

                p_para->mem_info.ref_nhid++;
                p_para->mem_info.destid = (CTC_MAP_LPORT_TO_GPORT(temp1,temp2));
                p_para->mem_info.vid = (sal_rand()%4096);
                p_para->mem_info.l3if_type = CTC_L3IF_TYPE_VLAN_IF;
                p_para->mem_info.is_vlan_port = 1;
                p_para->mem_info.is_source_check_dis = sal_rand();
                p_para->mem_info.port_bmp_en = sal_rand();
                p_para->mem_info.port_bmp[0] = sal_rand();
            }
            else
            {
                if (p_para->mem_info.member_type == CTC_NH_PARAM_MEM_BRGMC_LOCAL)
                {
                    p_para->mem_info.destid = (loop%64);
                    p_para->mem_info.vid = (sal_rand()%4095)+1;
                }
                else if (p_para->mem_info.member_type == CTC_NH_PARAM_MEM_IPMC_LOCAL)
                {
                    p_para->mem_info.destid = (loop%64);
                    p_para->mem_info.vid = ((loop%4095)+1);
                    p_para->mem_info.l3if_type = CTC_L3IF_TYPE_VLAN_IF;
                }
                else if (p_para->mem_info.member_type == CTC_NH_PARAM_MEM_LOCAL_WITH_NH)
                {
                    p_para->mem_info.destid = (loop%64);
                    p_para->mem_info.vid = ((loop%4095)+1);
                    p_para->mem_info.l3if_type = CTC_L3IF_TYPE_VLAN_IF;
                    p_para->mem_info.ref_nhid++;
                }

            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_update_mcast(g_api_lchip, nhid, p_para);
            }
            else
            {
                ret = ctc_nh_update_mcast(nhid, p_para);
            }

            if (ret == 0)
            {
                if (g_ctc_stress_nexthop && g_ctc_stress_nexthop->p_mcast_member[nhid])
                {
                    sal_memcpy(g_ctc_stress_nexthop->p_mcast_member[nhid], p_para, sizeof(ctc_mcast_nh_param_group_t));
                    g_ctc_stress_nexthop->p_mcast_member ++;
                }
            }

        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
        {
            if (!g_ctc_stress_nexthop || !g_ctc_stress_nexthop->p_mcast_member[nhid])
            {
                ctc_cli_out("No Mcast resource need to remove \n");
                return CLI_SUCCESS;
            }

            if (g_ctc_stress_nexthop->p_mcast_member[nhid])
            {
                if (NULL == g_ctc_stress_nexthop->p_mcast_member[nhid] + loop)
                {
                    continue;
                }

                p_mcast_para = (g_ctc_stress_nexthop->p_mcast_member[nhid] + loop);

                p_mcast_para->opcode = CTC_NH_PARAM_MCAST_DEL_MEMBER;

                if(g_ctcs_api_en)
                {
                    ret = ctcs_nh_update_mcast(g_api_lchip,nhid, p_mcast_para);
                }
                else
                {
                    ret = ctc_nh_update_mcast(nhid, p_mcast_para);
                }

            }
        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);

    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf,"test number:%5d", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST_MEM]);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type && g_ctc_stress_nexthop->p_mcast_member)
    {
        sal_free(g_ctc_stress_nexthop->p_mcast_member);
        g_ctc_stress_nexthop->p_mcast_member = NULL;
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST_MEM] = 0;
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_mcast_group_add,
        ctc_cli_stress_nexthop_mcast_group_add_cmd,
        "nexthop mcast group add nh-base NHID grp-base GRP (random|incr|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        "Nexthop module",
        "Mcast nexthop",
        "Group",
        "Add",
        "Nexthop base",
        "NHID",
        "Group base",
        "Group ID",
        "Random",
        "Increase",
        "Random all",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    mem_table_t mem_info;
    int8* m_name = NULL;
    mem_table_t mem_info1;

    ctc_cli_stress_t stress;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&mem_info, 0, sizeof(mem_table_t));
    sal_memset(&mem_info1, 0, sizeof(mem_table_t));

    CTC_CLI_GET_UINT32("Nexthop ID", g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST], argv[0]);
    CTC_CLI_GET_UINT32("Group ID", g_ctc_stress_nexthop->grp_base, argv[1]);

    ret = (_ctc_stress_nexthop_cli_parse_common(&(argv[0]), argc, &stress, CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST));
    if (ret)
    {
        return ret;
    }

    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info, &m_name);
    g_ctc_stress_nexthop->ori_mem_size[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST] = mem_info.size;

    ret = _ctc_cli_stress_mcast(&stress);
    if (ret)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST] = 0;
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    mem_mgr_get_mtype_info(MEM_NEXTHOP_MODULE, &mem_info1, &m_name);

    ctc_cli_out("Install %u mcast Entry\n", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST]);

    return ret;
}

CTC_CLI(ctc_cli_stress_nexthop_mcast_member_add,
        ctc_cli_stress_nexthop_mcast_member_add_cmd,
        "nexthop mcast member add NHID group GRP (bridge-member|ip-member|nh-member (nhid NH_BASE)) (random|incr|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        "Nexthop module",
        "Mcast nexthop",
        "Add",
        "Mcast member",
        "NHID",
        "Group",
        "Group ID",
        "Bridge member",
        "Ip member",
        "Nexthop member",
        "Member nhid",
        "Nhid base",
        "Random",
        "Increase",
        "Random all",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint32 index = 0xFF;
    ctc_l2dflt_addr_t l2_grp;
    ctc_mcast_nh_param_group_t mcast_para;
    uint32 nhid = 0;
    uint32 grp_id = 0;

    ctc_cli_stress_t stress;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&l2_grp, 0, sizeof(ctc_l2dflt_addr_t));
    sal_memset(&mcast_para, 0, sizeof(ctc_mcast_nh_param_group_t));

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);
    CTC_CLI_GET_UINT32("Group ID", grp_id, argv[1]);

    mcast_para.mc_grp_id = grp_id;
    mcast_para.opcode = CTC_NH_PARAM_MCAST_ADD_MEMBER;

    index = CTC_CLI_GET_ARGC_INDEX("bridge-member");
    if (0xFF != index)
    {
        mcast_para.mem_info.member_type = CTC_NH_PARAM_MEM_BRGMC_LOCAL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip-member");
    if (0xFF != index)
    {
        mcast_para.mem_info.member_type = CTC_NH_PARAM_MEM_IPMC_LOCAL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("nh-member");
    if (0xFF != index)
    {
        mcast_para.mem_info.member_type = CTC_NH_PARAM_MEM_LOCAL_WITH_NH;
        CTC_CLI_GET_UINT32_RANGE("nh id", mcast_para.mem_info.ref_nhid, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }

    ret = (_ctc_stress_nexthop_cli_parse_common(&(argv[0]), argc, &stress, CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST_MEM));
    if (ret)
    {
        return ret;
    }

    ret = _ctc_cli_stress_mcast_member(&mcast_para, &stress, nhid);
    if (ret)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST_MEM] = 0;
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }

    ctc_cli_out("Install %u Mcast Member Entry\n", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MCAST_MEM]);

    return ret;
}

CTC_CLI(ctc_cli_stress_nexthop_mcast_group_remove,
        ctc_cli_stress_nexthop_mcast_group_remove_cmd,
        "nexthop mcast group remove (perf|)",
        "Nexthop module",
        "Mcast",
        "Group",
        "Remove",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    ctc_cli_stress_t stress;
    uint8 index = 0;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_mcast(&stress);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }

    return ret;
}

CTC_CLI(ctc_cli_stress_nexthop_mcast_member_remove,
        ctc_cli_stress_nexthop_mcast_member_remove_cmd,
        "nexthop mcast member remove nhid NHID (perf|)",
        "Nexthop module",
        "Mcast",
        "Member",
        "Remove",
        "Nhid",
        "Value",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    ctc_cli_stress_t stress;
    ctc_mcast_nh_param_group_t mcast_para;
    uint32 nhid = 0;
    uint8 index = 0;

    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&mcast_para, 0, sizeof(ctc_mcast_nh_param_group_t));

    CTC_CLI_GET_UINT32("Nexthop ID", nhid, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;

    ret = _ctc_cli_stress_mcast_member(&mcast_para,&stress, nhid);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }

    return ret;
}

int32
_ctc_cli_stress_trill(ctc_cli_stress_t* stress)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1;
    sal_systime_t tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};
    uint8 is_random = 0;
    uint8 is_random_all = 0;
    ctc_nh_trill_param_t param;
    uint32 nhid = 0;
    uint32 temp1 = 0;
    uint32 temp2 = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = { 0 };

    is_random = (CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode ) ? 1:0;
    is_random_all = (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode ) ? 1:0;

    ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);

    sal_memset(&param, 0, sizeof(ctc_nh_trill_param_t));
    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL] && (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type))
    {
        ctc_cli_out("Other trill stress have used, please remove first! \n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL] = (uint32*)sal_malloc(g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL]*sizeof(uint32));
        if (!g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL])
        {
            return CLI_ERROR;
        }
    }

    for (loop = 0; loop < g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL]; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
        {
            nhid = g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL] + loop;
            if (is_random || is_random_all)
            {
                temp1  = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_CHIP_NUM]; /*gchip*/
                temp2  = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];   /*lport*/

                param.oif.is_l2_port = 1;
                param.flag      = sal_rand();
                param.upd_type  = (sal_rand()%MAX_CTC_NH_UPDTYPE);
                param.oif.gport = (CTC_MAP_LPORT_TO_GPORT(temp1,temp2));
                param.oif.vid   = (sal_rand()%4095);
                param.oif.ecmp_if_id = sal_rand();
                *((uint32*)&param.mac[0]) = sal_rand();
                *((uint16*)&param.mac[4]) = sal_rand();
                param.ingress_nickname = sal_rand();
                param.egress_nickname = sal_rand();
                param.ttl =  (sal_rand()%255);
                param.mtu_size = sal_rand();
            }
            else
            {
                param.oif.is_l2_port = 1;
                param.flag      = 0;
                param.upd_type  = CTC_NH_UPD_FWD_ATTR;
                param.oif.gport = (loop%64);
                param.oif.vid   = (loop%4094)+1;
                *((uint32*)&param.mac[0]) += loop;
                param.dsnh_offset +=  loop;
                param.ingress_nickname += loop;
                param.egress_nickname += loop;
                param.ttl = (loop%256);
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_add_trill(g_api_lchip, nhid, &param);
            }
            else
            {
                ret = ctc_nh_add_trill(nhid, &param);
            }

            if (ret == 0)
            {
                if (g_ctc_stress_nexthop && g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL])
                {
                    *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL] + loop) = nhid;
                }
            }
        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
        {
            if (!g_ctc_stress_nexthop || !g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL])
            {
                ctc_cli_out("No Trill resource need to remove \n");
                return CLI_SUCCESS;
            }

            if (NULL == g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL] + loop)
            {
                continue;
            }

            nhid = *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL] + loop);

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_remove_trill(g_api_lchip, nhid);
            }
            else
            {
                ret = ctc_nh_remove_trill(nhid);
            }

            if (ret == CTC_E_NONE)
            {
                 /*-ctc_cli_out("Remove Arp %d normal\n", loop+1);*/
            }
            else
            {
                /*- ctc_cli_out("Remove NHID %d error: %s\n", loop+1, ctc_get_error_desc(ret));*/
            }
        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);

    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf,"test number:%5d", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL]);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL] = 0;

        if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL])
        {
            sal_free(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL]);
            g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL] = NULL;
        }
    }

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_trill_add,
        ctc_cli_stress_nexthop_trill_add_cmd,
        "nexthop trill add nh-base NHID (random|incr|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        "Nexthop module",
        "Trill",
        "Add",
        "Nhid base",
        "Base value",
        "Random",
        "Increase",
        "Random all",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;

    ctc_cli_stress_t stress;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));

    CTC_CLI_GET_UINT32("nhid", g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL], argv[0]);

    ret = _ctc_stress_nexthop_cli_parse_common(&(argv[0]), argc, &stress, CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL);
    if (ret)
    {
        return ret;
    }

    /*2. create ipuc */
    ret = _ctc_cli_stress_trill(&stress);
    if (ret)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL] = 0;
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    ctc_cli_out("Install %u trill Entry\n", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_TRILL]);

    return ret;

}


CTC_CLI(ctc_cli_stress_nexthop_trill_remove,
        ctc_cli_stress_nexthop_trill_remove_cmd,
        "nexthop trill remove (perf|)",
        "Nexthop module",
        "Trill",
        "Remove",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;

    ctc_cli_stress_t stress;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_trill(&stress);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    return ret;
}

int32
_ctc_cli_stress_misc(ctc_misc_nh_param_t* p_para, ctc_cli_stress_t* stress)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1;
    sal_systime_t tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};
    ctc_misc_nh_param_t param;
    uint32 nhid = 0;
    uint32 temp1 = 0;
    uint32 temp2 = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = { 0 };

    sal_memset(&param, 0, sizeof(ctc_misc_nh_param_t));
    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);

    if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC] && (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type))
    {
        ctc_cli_out("Other misc stress have used, please remove first! \n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC] =
            (uint32*)sal_malloc(g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC]*sizeof(uint32));
        if (!g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC])
        {
            return CLI_ERROR;
        }
    }

    for (loop = 0; loop <  g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC]; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
        {
            nhid = g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC] + loop;
            if ((CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode) || (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode))
            {
                temp1  = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_CHIP_NUM]; /*gchip*/
                temp2  = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];   /*lport*/

                param.is_oif = 0;
                param.gport = (CTC_MAP_LPORT_TO_GPORT(temp1,temp2));
                param.dsnh_offset = sal_rand();
                param.truncated_len = sal_rand();
                param.oif.gport = (CTC_MAP_LPORT_TO_GPORT(temp1,temp2));
                param.oif.vid   = (sal_rand()%4095);
                param.oif.is_l2_port = 1;
                param.oif.ecmp_if_id = 0;
                if (p_para->type == CTC_MISC_NH_TYPE_REPLACE_L2HDR)
                {
                    param.misc_param.l2edit.flag = sal_rand();
                    *((uint32*)&param.misc_param.l2edit.mac_da[0]) = sal_rand();
                    *((uint16*)&param.misc_param.l2edit.mac_da[4]) = sal_rand();
                    *((uint32*)&param.misc_param.l2edit.mac_sa[0]) = sal_rand();
                    *((uint16*)&param.misc_param.l2edit.mac_sa[4]) = sal_rand();
                    param.misc_param.l2edit.vlan_id = (sal_rand()%4095);
                    param.misc_param.l2edit.is_reflective = 0;
                }
                else if (p_para->type == CTC_MISC_NH_TYPE_REPLACE_L2_L3HDR)
                {
                    param.misc_param.l2_l3edit.flag = sal_rand();
                    *((uint32*)&param.misc_param.l2_l3edit.mac_da[0]) = sal_rand();
                    *((uint16*)&param.misc_param.l2_l3edit.mac_da[4]) = sal_rand();
                    *((uint32*)&param.misc_param.l2_l3edit.mac_sa[0]) = sal_rand();
                    *((uint16*)&param.misc_param.l2_l3edit.mac_sa[4]) = sal_rand();
                    param.misc_param.l2_l3edit.dst_port = (CTC_MAP_LPORT_TO_GPORT(temp1,temp2));
                    param.misc_param.l2_l3edit.is_reflective = 0;
                    param.misc_param.l2_l3edit.ipda = sal_rand();
                    param.misc_param.l2_l3edit.vlan_edit_info.flag = sal_rand();
                    param.misc_param.l2_l3edit.vlan_edit_info.cvlan_edit_type = (sal_rand()%CTC_VLAN_EGRESS_EDIT_MAX);
                    param.misc_param.l2_l3edit.vlan_edit_info.svlan_edit_type = (sal_rand()%CTC_VLAN_EGRESS_EDIT_MAX);
                    param.misc_param.l2_l3edit.vlan_edit_info.output_cvid = (sal_rand()%4095);
                    param.misc_param.l2_l3edit.vlan_edit_info.output_svid = (sal_rand()%4095);
                    param.misc_param.l2_l3edit.vlan_edit_info.edit_flag = sal_rand();
                    param.misc_param.l2_l3edit.vlan_edit_info.svlan_tpid_index = 0;
                    param.misc_param.l2_l3edit.vlan_edit_info.stag_cos = (sal_rand()%8);
                    param.misc_param.l2_l3edit.vlan_edit_info.user_vlanptr = sal_rand();
                    param.misc_param.l2_l3edit.vlan_edit_info.mtu_size = 0;
                }
                else if (p_para->type == CTC_MISC_NH_TYPE_TO_CPU)
                {
                    param.misc_param.l2_l3edit.flag = sal_rand();
                    param.misc_param.cpu_reason.cpu_reason_id = (sal_rand()%CTC_PKT_CPU_REASON_MAX_COUNT);
                }
                else if (p_para->type == CTC_MISC_NH_TYPE_FLEX_EDIT_HDR)
                {
                    param.misc_param.flex_edit.flag = sal_rand();
                    *((uint32*)&param.misc_param.flex_edit.mac_da[0]) = sal_rand();
                    *((uint16*)&param.misc_param.flex_edit.mac_da[4]) = sal_rand();
                    *((uint32*)&param.misc_param.flex_edit.mac_sa[0]) = sal_rand();
                    *((uint16*)&param.misc_param.flex_edit.mac_sa[4]) = sal_rand();
                    param.misc_param.flex_edit.is_reflective = 0;
                    param.misc_param.flex_edit.packet_type = (sal_rand()%CTC_MISC_NH_PACKET_TYPE_MAX);
                    param.misc_param.flex_edit.ether_type = sal_rand();
                    param.misc_param.flex_edit.stag_op = (sal_rand()%CTC_VLAN_TAG_OP_MAX);
                    param.misc_param.flex_edit.svid_sl = (sal_rand()%CTC_VLAN_TAG_SL_MAX);
                    param.misc_param.flex_edit.scos_sl = (sal_rand()%CTC_VLAN_TAG_SL_MAX);
                    param.misc_param.flex_edit.ctag_op = (sal_rand()%CTC_VLAN_TAG_OP_MAX);
                    param.misc_param.flex_edit.cvid_sl = (sal_rand()%CTC_VLAN_TAG_SL_MAX);
                    param.misc_param.flex_edit.ccos_sl = (sal_rand()%CTC_VLAN_TAG_SL_MAX);
                    param.misc_param.flex_edit.new_svid = (sal_rand()%4095);
                    param.misc_param.flex_edit.new_cvid =(sal_rand()%4095);
                    param.misc_param.flex_edit.new_scos = (sal_rand()%8);
                    param.misc_param.flex_edit.new_ccos = (sal_rand()%8);
                    param.misc_param.flex_edit.ip_sa.ipv4 = sal_rand();
                    param.misc_param.flex_edit.ip_da.ipv4 = sal_rand();
                    param.misc_param.flex_edit.protocol = sal_rand();
                    param.misc_param.flex_edit.ttl = (sal_rand()%255);
                    param.misc_param.flex_edit.ecn = (sal_rand()%4);
                    param.misc_param.flex_edit.ecn_select = (sal_rand()%MAX_CTC_NH_ECN_SELECT_MODE);
                    param.misc_param.flex_edit.dscp_select = (sal_rand()%MAX_CTC_NH_ECN_SELECT_MODE);
                    param.misc_param.flex_edit.dscp_or_tos = 0;
                    param.misc_param.flex_edit.flow_label = sal_rand();
                    param.misc_param.flex_edit.l4_src_port = sal_rand();
                    param.misc_param.flex_edit.l4_dst_port = sal_rand();
                    param.misc_param.flex_edit.gre_key = sal_rand();
                    param.misc_param.flex_edit.icmp_type = sal_rand();
                    param.misc_param.flex_edit.icmp_code = sal_rand();
                    param.misc_param.flex_edit.arp_ht = sal_rand();
                    param.misc_param.flex_edit.arp_pt = sal_rand();
                    param.misc_param.flex_edit.arp_halen = sal_rand();
                    param.misc_param.flex_edit.arp_palen = sal_rand();
                    param.misc_param.flex_edit.arp_op = sal_rand();
                    param.misc_param.flex_edit.arp_spa = sal_rand();
                    param.misc_param.flex_edit.arp_tpa = sal_rand();
                    *((uint32*)&param.misc_param.flex_edit.arp_sha[0]) = sal_rand();
                    *((uint16*)&param.misc_param.flex_edit.arp_sha[4]) = sal_rand();
                    *((uint32*)&param.misc_param.flex_edit.arp_tha[0]) = sal_rand();
                    *((uint16*)&param.misc_param.flex_edit.arp_tha[4]) = sal_rand();
                }
                else
                {
                    return CLI_ERROR;
                }
            }
            else
            {
                param.gport = (loop%255);
                param.oif.gport = (loop%255);
                param.oif.vid   = (loop%4095);
                param.oif.is_l2_port = 1;
                if (p_para->type == CTC_MISC_NH_TYPE_REPLACE_L2HDR)
                {
                    param.misc_param.l2edit.flag = CTC_MISC_NH_L2_EDIT_REPLACE_MAC_SA;
                    *((uint32*)&param.misc_param.l2edit.mac_sa[0]) += loop;
                    param.misc_param.l2edit.vlan_id = (loop%4095);
                }
                else if (p_para->type == CTC_MISC_NH_TYPE_REPLACE_L2_L3HDR)
                {
                    param.misc_param.l2_l3edit.flag = (CTC_MISC_NH_L2_EDIT_REPLACE_MAC_DA|CTC_MISC_NH_L3_EDIT_REPLACE_IPDA);
                    *((uint32*)&param.misc_param.l2_l3edit.mac_da[0]) += loop;
                    param.misc_param.l2_l3edit.ipda += loop;
                }
                else if (p_para->type == CTC_MISC_NH_TYPE_TO_CPU)
                {
                    param.misc_param.l2_l3edit.flag = 0;
                    param.misc_param.cpu_reason.cpu_reason_id += loop;
                }
                else if (p_para->type == CTC_MISC_NH_TYPE_FLEX_EDIT_HDR)
                {
                    param.misc_param.flex_edit.flag = (CTC_MISC_NH_FLEX_EDIT_REPLACE_L2_HDR|CTC_MISC_NH_FLEX_EDIT_REPLACE_IP_HDR);
                    *((uint32*)&param.misc_param.flex_edit.mac_da[0]) += loop;
                    *((uint32*)&param.misc_param.flex_edit.mac_sa[0]) += loop;
                    param.misc_param.flex_edit.ip_sa.ipv4 += loop;
                    param.misc_param.flex_edit.ip_da.ipv4 += loop;
                    param.misc_param.flex_edit.ecn_select = CTC_NH_ECN_SELECT_ASSIGN;
                    param.misc_param.flex_edit.dscp_select = CTC_NH_DSCP_SELECT_ASSIGN;
                    param.misc_param.flex_edit.l4_src_port += loop;
                    param.misc_param.flex_edit.l4_dst_port += loop;
                }
                else
                {
                    return CLI_ERROR;
                }
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_add_misc(g_api_lchip, nhid, &param);
            }
            else
            {
                ret = ctc_nh_add_misc(nhid, &param);
            }

            if (ret == 0)
            {
                if (g_ctc_stress_nexthop && g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC])
                {
                    *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC] + loop) = nhid;
                }
            }
        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
        {
            if (!g_ctc_stress_nexthop || !g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC])
            {
                ctc_cli_out("No Misc resource need to remove \n");
                return CLI_SUCCESS;
            }

            if (NULL == g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC] + loop)
            {
                continue;
            }

            nhid = *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC] + loop);

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_remove_misc(g_api_lchip, nhid);
            }
            else
            {
                ret = ctc_nh_remove_misc(nhid);
            }

        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);

    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf,"test number:%5d", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC]);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC] = 0;

        if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC])
        {
            sal_free(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC]);
            g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC] = NULL;
        }
    }

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_misc_add,
        ctc_cli_stress_nexthop_misc_add_cmd,
        "nexthop misc add nh-base NHID type (replace-l2|replace-l2l3|to-cpu|flex-edit|l2-with-ts)(random|incr|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        "Nexthop module",
        "Misc",
        "Add",
        "Nhid base",
        "Base value",
        "Replace L2 header",
        "Replace L2&l3 header",
        "To Cpu",
        "Flex edit l2/l3/l4",
        "Add layer2 header with time stamp",
        "Random",
        "Increase",
        "Random all",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint32 index = 0xFF;
    ctc_misc_nh_param_t misc_para;
    ctc_cli_stress_t stress;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&misc_para, 0, sizeof(ctc_misc_nh_param_t));

    CTC_CLI_GET_UINT32("nhid", g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC], argv[0]);
    index = CTC_CLI_GET_ARGC_INDEX("replace-l2");
    if (0xFF != index)
    {
        misc_para.type = CTC_MISC_NH_TYPE_REPLACE_L2HDR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("replace-l2");
    if (0xFF != index)
    {
        misc_para.type = CTC_MISC_NH_TYPE_REPLACE_L2_L3HDR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("to-cpu");
    if (0xFF != index)
    {
        misc_para.type = CTC_MISC_NH_TYPE_TO_CPU;
    }
    index = CTC_CLI_GET_ARGC_INDEX("flex-edit");
    if (0xFF != index)
    {
        misc_para.type = CTC_MISC_NH_TYPE_FLEX_EDIT_HDR;
    }
    index = CTC_CLI_GET_ARGC_INDEX("l2-with-ts");
    if (0xFF != index)
    {
        misc_para.type = CTC_MISC_NH_TYPE_OVER_L2_WITH_TS;
    }

    ret = _ctc_stress_nexthop_cli_parse_common(&(argv[0]), argc, &stress, CTC_CLI_STRESS_NEXTHOP_TYPE_MISC);
    if (ret)
    {
        return ret;
    }

    /*2. create ipuc */
    ret = _ctc_cli_stress_misc(&misc_para, &stress);
    if (ret)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC] = 0;
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    ctc_cli_out("Install %u misc Entry\n", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_MISC]);

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_misc_remove,
        ctc_cli_stress_nexthop_misc_remove_cmd,
        "nexthop misc remove (perf|)",
        "Nexthop module",
        "Misc",
        "Remove",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    ctc_misc_nh_param_t para;
    uint8 index = 0;

    ctc_cli_stress_t stress;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&para, 0, sizeof(ctc_misc_nh_param_t));

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }


    ret = _ctc_cli_stress_misc(&para, &stress);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    return ret;
}


int32
_ctc_cli_stress_xlate(ctc_cli_stress_t* stress)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1;
    sal_systime_t tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};
    uint8 is_random = 0;
    uint8 is_random_all = 0;
    ctc_vlan_edit_nh_param_t param;
    uint32 nhid = 0;
    uint32 temp1 = 0;
    uint32 temp2 = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = { 0 };

    is_random = (CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode ) ? 1:0;
    is_random_all = (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode ) ? 1:0;

    sal_memset(&param, 0, sizeof(ctc_vlan_edit_nh_param_t));
    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);

    if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE] && (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type))
    {
        ctc_cli_out("Other trill stress have used, please remove first! \n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE] = (uint32*)sal_malloc(g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE]*sizeof(uint32));
        if (!g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE])
        {
            return CLI_ERROR;
        }
    }

    for (loop = 0; loop < g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE]; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
        {
            nhid = g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE] + loop;
            if (is_random || is_random_all)
            {
                temp1  = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_CHIP_NUM]; /*gchip*/
                temp2  = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];   /*lport*/

                param.dsnh_offset      = 0;
                param.gport_or_aps_bridge_id = (CTC_MAP_LPORT_TO_GPORT(temp1,temp2));
                param.aps_en = 0;
                param.logic_port_valid =  0;
                param.logic_port = sal_rand();
                param.logic_port_check = sal_rand();
                param.vlan_edit_info.flag = sal_rand();
                param.vlan_edit_info.cvlan_edit_type = (sal_rand()%CTC_VLAN_EGRESS_EDIT_MAX);
                param.vlan_edit_info.svlan_edit_type = (sal_rand()%CTC_VLAN_EGRESS_EDIT_MAX);
                param.vlan_edit_info.output_cvid = (sal_rand()%4095);
                param.vlan_edit_info.output_svid = (sal_rand()%4095);
                param.vlan_edit_info.edit_flag = sal_rand();
                param.vlan_edit_info.svlan_tpid_index = 0;
                param.vlan_edit_info.stag_cos = (sal_rand()%8);
                param.vlan_edit_info.user_vlanptr = sal_rand();
                param.vlan_edit_info.mtu_size = (sal_rand()%0x3fff);
                param.vlan_edit_info_p.flag = sal_rand();
                param.vlan_edit_info_p.cvlan_edit_type = (sal_rand()%CTC_VLAN_EGRESS_EDIT_MAX);
                param.vlan_edit_info_p.svlan_edit_type = (sal_rand()%CTC_VLAN_EGRESS_EDIT_MAX);
                param.vlan_edit_info_p.output_cvid = (sal_rand()%4095);
                param.vlan_edit_info_p.output_svid =(sal_rand()%4095);
                param.vlan_edit_info_p.edit_flag = sal_rand();
                param.vlan_edit_info_p.svlan_tpid_index = 0;
                param.vlan_edit_info_p.stag_cos = (sal_rand()%8);
                param.vlan_edit_info_p.user_vlanptr = sal_rand();
                param.vlan_edit_info_p.mtu_size = sal_rand();
            }
            else
            {
                param.gport_or_aps_bridge_id = 0;
                param.aps_en = 0;
                param.logic_port_valid =  0;
                param.vlan_edit_info.cvlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
                param.vlan_edit_info.svlan_edit_type = CTC_VLAN_EGRESS_EDIT_INSERT_VLAN;
                param.vlan_edit_info.output_cvid = (loop%4094)+1;
                param.vlan_edit_info.output_svid = (loop%4094)+1;
                param.vlan_edit_info.edit_flag = (CTC_VLAN_EGRESS_EDIT_OUPUT_CVID_VALID|CTC_VLAN_EGRESS_EDIT_OUPUT_SVID_VALID);
                param.vlan_edit_info.svlan_tpid_index = 0;
                param.vlan_edit_info.stag_cos = 0;
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_add_xlate(g_api_lchip, nhid, &param);
            }
            else
            {
                ret = ctc_nh_add_xlate(nhid, &param);
            }

            if (ret == 0)
            {
                if (g_ctc_stress_nexthop && g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE])
                {
                    *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE] + loop) = nhid;
                }
            }
        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
        {
            if (!g_ctc_stress_nexthop || !g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE])
            {
                ctc_cli_out("No Trill resource need to remove \n");
                return CLI_SUCCESS;
            }

            if (NULL == g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE] + loop)
            {
                continue;
            }

            nhid = *(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE] + loop);

            if(g_ctcs_api_en)
            {
                ret = ctcs_nh_remove_xlate(g_api_lchip, nhid);
            }
            else
            {
                ret = ctc_nh_remove_xlate(nhid);
            }

        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);

    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf,"test number:%5d", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE]);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE] = 0;

        if (g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE])
        {
            sal_free(g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE]);
            g_ctc_stress_nexthop->p_nhid[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE] = NULL;
        }
    }

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_xlate_add,
        ctc_cli_stress_nexthop_xlate_add_cmd,
        "nexthop xlate add nh-base NHID (random|incr|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        "Nexthop module",
        "Xlate",
        "Add",
        "Nhid base",
        "Base value",
        "Random",
        "Increase",
        "Random all",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;

    ctc_cli_stress_t stress;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));

    CTC_CLI_GET_UINT32("nhid", g_ctc_stress_nexthop->nh_base[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE], argv[0]);

    ret = _ctc_stress_nexthop_cli_parse_common(&(argv[0]), argc, &stress, CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE);
    if (ret)
    {
        return ret;
    }

    /*2. create ipuc */
    ret = _ctc_cli_stress_xlate(&stress);
    if (ret)
    {
        g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE] = 0;
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    ctc_cli_out("Install %u xlate Entry\n", g_ctc_stress_nexthop->entry_num[CTC_CLI_STRESS_NEXTHOP_TYPE_XLATE]);

    return ret;

}

CTC_CLI(ctc_cli_stress_nexthop_xlate_remove,
        ctc_cli_stress_nexthop_xlate_remove_cmd,
        "nexthop xlate remove (perf|)",
        "Nexthop module",
        "Xlate",
        "Remove",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;

    ctc_cli_stress_t stress;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_xlate(&stress);
    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
    }
    return ret;
}

int32
ctc_stress_test_nexthop_cli_init(void)
{
    g_ctc_stress_nexthop = (ctc_cli_stress_test_nexthop_t*)sal_malloc(sizeof(ctc_cli_stress_test_nexthop_t));
    if(!g_ctc_stress_nexthop)
    {
        return CLI_ERROR;
    }

    sal_memset(g_ctc_stress_nexthop, 0 , sizeof(ctc_cli_stress_test_nexthop_t));

    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_arp_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_arp_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_ipuc_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_ipuc_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_ip_tunnel_v4_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_ip_tunnel_v6_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_ip_tunnel_v4_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_ip_tunnel_v6_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_mpls_add_tunnel_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_mpls_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_mpls_remove_tunnel_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_mpls_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_wlan_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_wlan_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_ecmp_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_ecmp_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_mcast_group_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_mcast_group_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_mcast_member_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_mcast_member_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_trill_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_trill_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_misc_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_misc_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_xlate_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_nexthop_xlate_remove_cmd);
    return CLI_SUCCESS;
}


