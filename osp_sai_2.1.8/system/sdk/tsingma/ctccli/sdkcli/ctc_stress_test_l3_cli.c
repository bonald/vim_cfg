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

#define CTC_CLI_IPMC_MEMBER_L3IF_ALL_STR \
    "(sub-if (gport GPORT_ID | ("CTC_CLI_PORT_BITMAP_STR" (gchip GCHIP_ID|))) vlanid VLAN_ID | \
    vlan-if vlanid VLAN_ID (gport GPORT_ID | ("CTC_CLI_PORT_BITMAP_STR" (gchip GCHIP_ID|))) (vlan-port|) | \
    phy-if (gport GPORT_ID | ("CTC_CLI_PORT_BITMAP_STR" (gchip GCHIP_ID|))) |nexthop-id NHID (assign-port GPORT_ID|)|gport GPORT_ID)"
#define CTC_CLI_IPMC_L3IF_ALL_DESC \
    "Sub interface", CTC_CLI_GPORT_DESC, CTC_CLI_GPORT_ID_DESC, \
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_GCHIP_DESC,CTC_CLI_GCHIP_ID_DESC,\
    CTC_CLI_VLAN_DESC, CTC_CLI_VLAN_RANGE_DESC, \
    "VLAN interface", CTC_CLI_VLAN_DESC, CTC_CLI_VLAN_RANGE_DESC, \
    CTC_CLI_GPORT_DESC, CTC_CLI_GPORT_ID_DESC, \
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_GCHIP_DESC,CTC_CLI_GCHIP_ID_DESC,"Vlan port" , \
    "Physical interface/remote chip id", CTC_CLI_GPORT_DESC, CTC_CLI_GPORT_ID_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_PORT_BITMAP_DESC,CTC_CLI_PORT_BITMAP_VALUE_DESC,\
    CTC_CLI_GCHIP_DESC,CTC_CLI_GCHIP_ID_DESC,\
    "Nexthop ID", CTC_CLI_NH_ID_STR, "Assign port", CTC_CLI_GPORT_ID_DESC, CTC_CLI_GPORT_DESC, CTC_CLI_GPORT_ID_DESC

struct ctc_cli_stress_test_l3_s
{
    uint8  incr_step;
    uint8   prefix_len;
    uint16  ipuc_vrfid_base;
    uint16  ipuc_vrfid_max;
    uint32  ipuc_tunnel_nh_entry_num;
    uint32  ipuc_tunnel_param_entry_num;
    uint32  ipuc_nat_param_entry_num;
    uint32  ipuc_param_entry_num;
    uint32  ipmc_group_num;
    uint32  ipmc_member_num;
    uint32  ipmc_ipsa_num;
    uint32* ipuc_tunnel_nh;
    ctc_ipuc_tunnel_param_t* ipuc_tunnel_param;
    ctc_ipuc_nat_sa_param_t* ipuc_nat_param;
    ctc_ipuc_param_t* ipuc_param;
    ctc_ipmc_group_info_t* ipmc_group;
};
typedef struct ctc_cli_stress_test_l3_s ctc_cli_stress_test_l3_t;
ctc_cli_stress_test_l3_t  *g_ctc_stress_l3;

extern int32 sys_tsingma_nalpm_cb_merge(uint8 lchip, uint32 vrf_id, uint8 ip_ver);

static int32
_ctc_cli_stress_ipmc_group(ctc_cli_stress_t* stress, ctc_ipmc_group_info_t* group_info)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop_grp = 0, loop_ipsa = 0;
    uint32 valid_count = 0;
    uint32 total_num = 0;
    sal_systime_t tv1;
    sal_systime_t tv2;
    char str_buf[32] = {0};
    ctc_cli_stress_output_t output;
    uint8 is_increase = 0;
    uint8 step = 0;
    ctc_ipmc_group_info_t* p_param = NULL;
    uint32 max_num = 0;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = { 0 };

    is_increase = (CTC_CLI_STRESS_DATA_MODE_INCREASE== stress->data_mode ) ? 1:0;
    ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);

    max_num = CTC_MAX_UINT32_VALUE / sizeof(ctc_ipmc_group_info_t);
    g_ctc_stress_l3->ipmc_group_num = (g_ctc_stress_l3->ipmc_group_num * g_ctc_stress_l3->ipmc_ipsa_num > max_num) ?
        (max_num / g_ctc_stress_l3->ipmc_ipsa_num) : g_ctc_stress_l3->ipmc_group_num;
    total_num = g_ctc_stress_l3->ipmc_group_num * g_ctc_stress_l3->ipmc_ipsa_num;

    step = g_ctc_stress_l3->incr_step;

    if (g_ctc_stress_l3->ipmc_group && (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type))
    {
        ctc_cli_out("Other fdb stress have used, please remove first! \n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    p_param = (ctc_ipmc_group_info_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_ipmc_group_info_t));
    if (NULL == p_param)
    {
        return CLI_ERROR;
    }
    sal_memset(p_param, 0, sizeof(ctc_ipmc_group_info_t));
    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        sal_memcpy(p_param, group_info, sizeof(ctc_ipmc_group_info_t));
        g_ctc_stress_l3->ipmc_group  = (ctc_ipmc_group_info_t*)sal_malloc(total_num * sizeof(ctc_ipmc_group_info_t));
        if (g_ctc_stress_l3->ipmc_group == NULL)
        {
            ctc_cli_out("Memory alloc error ! \n");
            mem_free(p_param);
            return CLI_ERROR;
        }
    }

    for (loop_grp = 0; loop_grp < g_ctc_stress_l3->ipmc_group_num; loop_grp++)
    {
        for (loop_ipsa = 0; loop_ipsa < g_ctc_stress_l3->ipmc_ipsa_num; loop_ipsa++)
        {
            if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
            {
                p_param->group_id += 1;
                if (CTC_IP_VER_4 == p_param->ip_version)
                {
                    p_param->address.ipv4.group_addr = (is_increase) ? (p_param->address.ipv4.group_addr + step):(sal_rand());
                    p_param->address.ipv4.group_addr |= 0xE0000000;
                    p_param->address.ipv4.group_addr &= 0xEFFFFFFF;
                    p_param->address.ipv4.src_addr = (is_increase) ? (p_param->address.ipv4.src_addr + step):(sal_rand());
                    p_param->address.ipv4.vrfid = (is_increase) ? (p_param->address.ipv4.vrfid + step) : (sal_rand());
                    p_param->address.ipv4.vrfid = p_param->address.ipv4.vrfid % capability[CTC_GLOBAL_CAPABILITY_MAX_VRFID];
                }
                else
                {
                    p_param->address.ipv6.group_addr[3] = (is_increase) ? (p_param->address.ipv6.group_addr[3] + step) : (sal_rand());
                    p_param->address.ipv6.group_addr[2] = (is_increase) ? (p_param->address.ipv6.group_addr[2]) : (sal_rand());
                    p_param->address.ipv6.group_addr[1] = (is_increase) ? (p_param->address.ipv6.group_addr[1]) : (sal_rand());
                    p_param->address.ipv6.group_addr[0] = (is_increase) ? (p_param->address.ipv6.group_addr[0]) : (sal_rand());
                    p_param->address.ipv6.group_addr[0] |= 0xFF000000;
                    p_param->address.ipv6.src_addr[3] = (is_increase) ? (p_param->address.ipv6.src_addr[3] + step) : (sal_rand());
                    p_param->address.ipv6.src_addr[2] = (is_increase) ? (p_param->address.ipv6.src_addr[2]) : (sal_rand());
                    p_param->address.ipv6.src_addr[1] = (is_increase) ? (p_param->address.ipv6.src_addr[1]) : (sal_rand());
                    p_param->address.ipv6.src_addr[0] = (is_increase) ? (p_param->address.ipv6.src_addr[0]) : (sal_rand());
                    p_param->address.ipv6.vrfid = (is_increase) ? (p_param->address.ipv6.vrfid + step) : (sal_rand());
                    p_param->address.ipv6.vrfid = p_param->address.ipv6.vrfid % capability[CTC_GLOBAL_CAPABILITY_MAX_VRFID];
                }
                sal_memcpy(g_ctc_stress_l3->ipmc_group + (loop_ipsa + loop_grp*g_ctc_stress_l3->ipmc_ipsa_num), p_param, sizeof(ctc_ipmc_group_info_t));
                if (g_ctcs_api_en)
                {
                    ret = ctcs_ipmc_add_group(g_api_lchip, p_param);
                }
                else
                {
                    ret = ctc_ipmc_add_group(p_param);
                }
            }
            else
            {
                if (!g_ctc_stress_l3 || !g_ctc_stress_l3->ipmc_group)
                {
                    ctc_cli_out("No ipmc group need to remove \n");
                    mem_free(p_param);
                    return CLI_SUCCESS;
                }
                if (g_ctcs_api_en)
                {
                    ret = ctcs_ipmc_remove_group(g_api_lchip, (g_ctc_stress_l3->ipmc_group + (loop_ipsa + loop_grp*g_ctc_stress_l3->ipmc_ipsa_num)));
                }
                else
                {
                    ret = ctc_ipmc_remove_group(g_ctc_stress_l3->ipmc_group + (loop_ipsa + loop_grp*g_ctc_stress_l3->ipmc_ipsa_num));
                }
            }
            if (ret == CTC_E_NONE)
            {
                valid_count++;
            }
            if ((ret < 0) && (CTC_CLI_STRESS_TEST_TYPE_STRESS != stress->test_type))
            {
                if((CTC_CLI_STRESS_TEST_TYPE_SPEC == stress->test_type) && ((ret == CTC_E_NO_RESOURCE) || (ret == CTC_E_PROFILE_NO_RESOURCE)
                || (ret == CTC_E_NO_MEMORY)))
                {
                    ctc_cli_out("Operate valid count: %d,  ret: %d, %s\n", valid_count, ret, ctc_get_error_desc(ret));
                }
                mem_free(p_param);
                return CLI_ERROR;
            }
        }
    }
    mem_free(p_param);

    ctc_cli_out("Operate valid count: %d \n", valid_count);
    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf, "test number:%5d", g_ctc_stress_l3->ipmc_group_num);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }
    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
    {
        g_ctc_stress_l3->ipmc_group_num = 0;
        g_ctc_stress_l3->ipmc_ipsa_num = 0;
        g_ctc_stress_l3->incr_step = 0;
        if (g_ctc_stress_l3->ipmc_group)
        {
            sal_free(g_ctc_stress_l3->ipmc_group);
            g_ctc_stress_l3->ipmc_group = NULL;
        }
    }
    return ret;
}

int32
_ctc_cli_stress_ipmc_member(ctc_cli_stress_t* stress, ctc_ipmc_group_info_t* p_group_info)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop_grp = 0;
    uint32 loop_mem = 0;
    uint32 loop = 0;
    uint32 total_num = 0;
    uint8 is_random_all = 0;
    uint32 valid_count = 0;
    ctc_ipmc_group_info_t* p_param = NULL;
    sal_systime_t tv1;
    sal_systime_t tv2;
    char str_buf[32] = { 0 };
    ctc_cli_stress_output_t output;
    ctc_l3if_t l3if_info;
    uint16 l3if_id = 0;

    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = { 0 };
    is_random_all = (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode ) ? 1:0;
    total_num = g_ctc_stress_l3->ipmc_group_num * g_ctc_stress_l3->ipmc_ipsa_num;
    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }
    ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);

    p_param = (ctc_ipmc_group_info_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_ipmc_group_info_t));
    if (NULL == p_param)
    {
        return CLI_ERROR;
    }
    sal_memcpy(p_param, p_group_info, sizeof(ctc_ipmc_group_info_t));
    if (p_param->ipmc_member[0].port_bmp_en || !is_random_all)
    {  /*1. port_bmp_en ||  !is_random_all only do one times*/
        g_ctc_stress_l3->ipmc_member_num = 1;
    }

    for (loop_grp = 0; loop_grp < total_num; loop_grp++)
    {
        sal_memcpy(&p_param->address, &((g_ctc_stress_l3->ipmc_group + loop_grp)->address), sizeof(ctc_ipmc_addr_info_t));
        p_param->group_ip_mask_len = (g_ctc_stress_l3->ipmc_group + loop_grp)->group_ip_mask_len;
        p_param->src_ip_mask_len = (g_ctc_stress_l3->ipmc_group + loop_grp)->src_ip_mask_len;
        p_param->ip_version = (g_ctc_stress_l3->ipmc_group + loop_grp)->ip_version;

        for (loop = 0; loop < (g_ctc_stress_l3->ipmc_member_num/CTC_IPMC_MAX_MEMBER_PER_GROUP + 1); loop++)
        {
             p_param->member_number = (loop == g_ctc_stress_l3->ipmc_member_num/CTC_IPMC_MAX_MEMBER_PER_GROUP) ? (g_ctc_stress_l3->ipmc_member_num%CTC_IPMC_MAX_MEMBER_PER_GROUP) : CTC_IPMC_MAX_MEMBER_PER_GROUP;
             for (loop_mem = 0; loop_mem < p_param->member_number; loop_mem++)
             {
                  /*1. port_bmp_en ||  !is_random_all only do one times*/
                 if(p_param->ipmc_member[0].port_bmp_en || !is_random_all)
                 {/*do nothing*/
                    break;
                 }

                  /*2. is_random_all AD random*/
                 if(!p_param->ipmc_member[0].is_nh )
                 {
                    l3if_id =  sal_rand() % (capability[CTC_GLOBAL_CAPABILITY_L3IF_NUM] -1);
                    l3if_id += 1; /*valid l3if id :1~1022*/
                    l3if_info.l3if_type = 0xFF;
                    if(ctc_l3if_get_l3if_id(&l3if_info, &l3if_id) != CTC_E_NONE)
                    {
                        loop_mem--;
                        continue;
                    }
                    p_param->ipmc_member[loop_mem].global_port = l3if_info.gport;
                    p_param->ipmc_member[loop_mem].vlan_id = l3if_info.vlan_id;
                    p_param->ipmc_member[loop_mem].l3_if_type = l3if_info.l3if_type;
                 }
                 else
                 {
                    p_param->ipmc_member[loop_mem].is_nh = p_param->ipmc_member[0].is_nh;
                    p_param->ipmc_member[loop_mem].nh_id = (p_group_info->ipmc_member[0].nh_id + loop*64 + loop_mem) % CTC_NH_DEFAULT_MAX_EXTERNAL_NEXTHOP_NUM + 3;
                 }
             }
             if (g_ctcs_api_en)
             {
                 ret = ctcs_ipmc_add_member(g_api_lchip, p_param);
             }
             else
             {
                 ret = ctc_ipmc_add_member(p_param);
             }
         }
         if (ret == CTC_E_NONE)
         {
             valid_count++;
         }
        if ((ret < 0) && (CTC_CLI_STRESS_TEST_TYPE_STRESS != stress->test_type))
        {
            if((CTC_CLI_STRESS_TEST_TYPE_SPEC == stress->test_type) && ((ret == CTC_E_NO_RESOURCE) || (ret == CTC_E_PROFILE_NO_RESOURCE)
            || (ret == CTC_E_NO_MEMORY)))
            {
                ctc_cli_out("Operate valid count: %d,  ret: %d, %s\n", valid_count, ret, ctc_get_error_desc(ret));
            }
            mem_free(p_param);
            return CLI_ERROR;
        }
    }
    mem_free(p_param);
    ctc_cli_out("Operate valid count: %d \n", valid_count);
    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf, "test number:%5d", g_ctc_stress_l3->ipmc_group_num);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }
    return ret;
}

static int32
_ctc_cli_stress_ipuc_tunnel(ctc_ipuc_tunnel_param_t* param, ctc_cli_stress_t op)
{
    int32 ret = CLI_SUCCESS;
    uint8 acl_cnt = 0;
    uint16  loop = 0;
    uint16  loop1 = 0;
    uint32 valid_count =0;
    sal_systime_t tv1,tv2;
    char str_buf[32] = {0};
    ctc_cli_stress_output_t output;
    ctc_ipuc_tunnel_param_t ipuc_tunnel_param;

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == op.test_type)
    {
        sal_gettime(&tv1);
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == op.op_type)
    {
        if (NULL == g_ctc_stress_l3->ipuc_tunnel_param)
        {
            g_ctc_stress_l3->ipuc_tunnel_param = (ctc_ipuc_tunnel_param_t*)sal_malloc(sizeof(ctc_ipuc_tunnel_param_t)*g_ctc_stress_l3->ipuc_tunnel_param_entry_num);
            if (NULL == g_ctc_stress_l3->ipuc_tunnel_param)
            {
                ctc_cli_out("Memory alloc error\n");
                return CLI_SUCCESS;
            }
        }
        sal_memset(g_ctc_stress_l3->ipuc_tunnel_param, 0, sizeof(ctc_ipuc_tunnel_param_t)*g_ctc_stress_l3->ipuc_tunnel_param_entry_num);
    }

    if (NULL == g_ctc_stress_l3->ipuc_tunnel_param)
    {
        ctc_cli_out("Memory not alloc\n");
        return CLI_SUCCESS;
    }

    for (loop = 0; loop < g_ctc_stress_l3->ipuc_tunnel_param_entry_num; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == op.op_type)
        {
            sal_memcpy(&ipuc_tunnel_param, param, sizeof(ctc_ipuc_tunnel_param_t));
            if ((CTC_CLI_STRESS_DATA_MODE_RANDOM == op.data_mode) || (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == op.data_mode))
            {
                if (CTC_IP_VER_4 == param->ip_ver)
                {
                    ipuc_tunnel_param.ip_da.ipv4 = sal_rand();
                    if (ipuc_tunnel_param.ip_sa.ipv4)
                    {
                        ipuc_tunnel_param.ip_sa.ipv4 = sal_rand();
                        CTC_SET_FLAG(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA);
                    }
                }
                else
                {
                    ipuc_tunnel_param.ip_da.ipv6[0] = sal_rand();
                    ipuc_tunnel_param.ip_da.ipv6[1] = sal_rand();
                    ipuc_tunnel_param.ip_da.ipv6[2] = sal_rand();
                    ipuc_tunnel_param.ip_da.ipv6[3] = sal_rand();

                    if (ipuc_tunnel_param.ip_sa.ipv6[0] || ipuc_tunnel_param.ip_sa.ipv6[1] ||
                        ipuc_tunnel_param.ip_sa.ipv6[2] || ipuc_tunnel_param.ip_sa.ipv6[3])
                    {
                        ipuc_tunnel_param.ip_sa.ipv6[0] = sal_rand();
                        ipuc_tunnel_param.ip_sa.ipv6[1] = sal_rand();
                        ipuc_tunnel_param.ip_sa.ipv6[2] = sal_rand();
                        ipuc_tunnel_param.ip_sa.ipv6[3] = sal_rand();
                        CTC_SET_FLAG(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA);
                    }
                }
            }
            else
            {
                if (CTC_IP_VER_4 == param->ip_ver)
                {
                    ipuc_tunnel_param.ip_da.ipv4 += loop;
                    if (ipuc_tunnel_param.ip_sa.ipv4)
                    {
                        ipuc_tunnel_param.ip_sa.ipv4 += loop;
                        CTC_SET_FLAG(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA);
                    }
                }
                else
                {
                    if (ipuc_tunnel_param.ip_da.ipv6[3]+loop < ipuc_tunnel_param.ip_da.ipv6[3])
                    {
                        ipuc_tunnel_param.ip_da.ipv6[2]++;
                    }
                    ipuc_tunnel_param.ip_da.ipv6[3] += loop;

                    if (ipuc_tunnel_param.ip_sa.ipv6[0] || ipuc_tunnel_param.ip_sa.ipv6[1] ||
                        ipuc_tunnel_param.ip_sa.ipv6[2] || ipuc_tunnel_param.ip_sa.ipv6[3])
                    {
                        if (ipuc_tunnel_param.ip_sa.ipv6[3]+loop < ipuc_tunnel_param.ip_sa.ipv6[3])
                        {
                            ipuc_tunnel_param.ip_sa.ipv6[2]++;
                        }
                        ipuc_tunnel_param.ip_sa.ipv6[3] += loop;
                        CTC_SET_FLAG(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA);
                    }
                }
            }

            if (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL != op.data_mode)
            {
                if (ipuc_tunnel_param.payload_type == MAX_CTC_IPUC_TUNNEL_PAYLOAD_TYPE)
                {
                    if (CTC_IP_VER_4 == ipuc_tunnel_param.ip_ver)
                    {
                        ipuc_tunnel_param.payload_type = CTC_IPUC_TUNNEL_PAYLOAD_TYPE_V6;
                    }
                    else
                    {
                        ipuc_tunnel_param.payload_type = CTC_IPUC_TUNNEL_PAYLOAD_TYPE_V4;
                    }
                }
            }
            else
            {
                if (ipuc_tunnel_param.payload_type == MAX_CTC_IPUC_TUNNEL_PAYLOAD_TYPE)
                {
                    ipuc_tunnel_param.payload_type = sal_rand() % MAX_CTC_IPUC_TUNNEL_PAYLOAD_TYPE;
                }

                if (!ipuc_tunnel_param.flag)
                {
                    ipuc_tunnel_param.flag = sal_rand()%0x1FFFFF;
                }

                if (CTC_FLAG_ISSET(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_STATS_EN))
                {
                    if (!ipuc_tunnel_param.stats_id)
                    {
                        CTC_UNSET_FLAG(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_STATS_EN);
                    }
                }

                if (CTC_FLAG_ISSET(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_INNER_VRF_EN))
                {
                    ipuc_tunnel_param.vrf_id = sal_rand()%CTC_MAX_VRFID_ID;
                }

                if (CTC_FLAG_ISSET(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_LKUP_BY_OUTER_HEAD))
                {
                    if (!ipuc_tunnel_param.nh_id)
                    {
                        ipuc_tunnel_param.nh_id = 2;
                    }
                }

                if (CTC_FLAG_ISSET(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_ACL_EN))
                {
                    if (acl_cnt < 16)
                    {
                        ipuc_tunnel_param.acl_lkup_num = sal_rand()%8;
                        for (loop1 = 0; loop1 < ipuc_tunnel_param.acl_lkup_num; loop1++)
                        {
                            ipuc_tunnel_param.acl_prop[loop1].acl_priority = loop1;
                            ipuc_tunnel_param.acl_prop[loop1].tcam_lkup_type = sal_rand()%CTC_ACL_TCAM_LKUP_TYPE_MAX;
                            ipuc_tunnel_param.acl_prop[loop1].hash_lkup_type = sal_rand()%CTC_ACL_HASH_LKUP_TYPE_MAX;
                       }
                       acl_cnt++;
                   }
                   else
                   {
                        CTC_UNSET_FLAG(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_ACL_EN);
                   }
                }

                if (CTC_FLAG_ISSET(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_SERVICE_POLICER_EN))
                {
                    if (!ipuc_tunnel_param.policer_id)
                    {
                        CTC_UNSET_FLAG(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_SERVICE_POLICER_EN);
                    }
                }

                if (CTC_FLAG_ISSET(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY))
                {
                    if (ipuc_tunnel_param.payload_type == CTC_IPUC_TUNNEL_PAYLOAD_TYPE_GRE)
                    {
                        ipuc_tunnel_param.gre_key = sal_rand()+1;
                    }
                    else
                    {
                        CTC_UNSET_FLAG(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY);
                    }
                }

                if(CTC_FLAG_ISSET(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_METADATA_EN))
                {
                    ipuc_tunnel_param.metadata = sal_rand()%0x3FFF;
                }

                if (CTC_FLAG_ISSET(ipuc_tunnel_param.flag,CTC_IPUC_TUNNEL_FLAG_V4_RPF_CHECK))
                {
                    uint8 l3if_num = sal_rand()%CTC_IPUC_IP_TUNNEL_MAX_IF_NUM;
                    sal_memset(ipuc_tunnel_param.l3_inf, 0, sizeof(uint16) * CTC_IPUC_IP_TUNNEL_MAX_IF_NUM);
                    for (loop1 = 0; loop1 < l3if_num; loop1++)
                    {
                        ipuc_tunnel_param.l3_inf[loop1] = loop1;
                   }
                }

#if 0
                if (ipuc_tunnel_param.cid)
                {
                    CTC_SET_FLAG(ipuc_tunnel_param.flag,CTC_IPUC_TUNNEL_FLAG_RPF_CHECK_DISABLE);
                    CTC_UNSET_FLAG(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_USE_OUTER_TTL);
                }

                if (CTC_FLAG_ISSET(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_STATS_EN))
                {
                    CTC_UNSET_FLAG(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_METADATA_EN);
                }

                if (CTC_FLAG_ISSET(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_INNER_VRF_EN) &&
                    CTC_FLAG_ISSET(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_ACL_EN) &&
                    CTC_FLAG_ISSET(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_LKUP_BY_OUTER_HEAD))
                {
                    ipuc_tunnel_param.vrf_id = 0;
                    CTC_UNSET_FLAG(ipuc_tunnel_param.flag, CTC_IPUC_TUNNEL_FLAG_INNER_VRF_EN);
                }
#endif
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_ipuc_add_tunnel(g_api_lchip, &ipuc_tunnel_param);
            }
            else
            {
                ret = ctc_ipuc_add_tunnel(&ipuc_tunnel_param);
            }

            if (CTC_CLI_STRESS_OP_TYPE_ADD == op.op_type)
            {
                sal_memcpy(g_ctc_stress_l3->ipuc_tunnel_param + loop, &ipuc_tunnel_param, sizeof(ctc_ipuc_tunnel_param_t));
            }
        }
        else
        {
            sal_memcpy(&ipuc_tunnel_param, g_ctc_stress_l3->ipuc_tunnel_param + loop, sizeof(ctc_ipuc_tunnel_param_t));

            if(g_ctcs_api_en)
            {
                ret = ctcs_ipuc_remove_tunnel(g_api_lchip, &ipuc_tunnel_param);
            }
            else
            {
                ret = ctc_ipuc_remove_tunnel(&ipuc_tunnel_param);
            }
        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, op.test_type, valid_count);
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == op.test_type)
    {
        sal_gettime(&tv2);

        sal_sprintf(str_buf,"Entry: %5d\n", g_ctc_stress_l3->ipuc_tunnel_param_entry_num);
        output.buf = str_buf;
        output.op = op.op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("Operate valid count: %d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == op.op_type)
    {
        sal_free(g_ctc_stress_l3->ipuc_tunnel_param);
        g_ctc_stress_l3->ipuc_tunnel_param = NULL;
        g_ctc_stress_l3->ipuc_tunnel_param_entry_num = 0;
    }

    return CLI_SUCCESS;
}

static int32
_ctc_cli_stress_ipuc_natsa(ctc_ipuc_nat_sa_param_t* param, ctc_cli_stress_t op)
{
    int32 ret = CLI_SUCCESS;
    uint16  loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1,tv2;
    char str_buf[32] = {0};
    ctc_cli_stress_output_t output;
    ctc_ipuc_nat_sa_param_t ipuc_nat_param;

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == op.test_type)
    {
        sal_gettime(&tv1);
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == op.op_type)
    {
        if (NULL == g_ctc_stress_l3->ipuc_nat_param)
        {
            g_ctc_stress_l3->ipuc_nat_param = (ctc_ipuc_nat_sa_param_t*)sal_malloc(sizeof(ctc_ipuc_nat_sa_param_t)*g_ctc_stress_l3->ipuc_nat_param_entry_num);
            if (NULL == g_ctc_stress_l3->ipuc_nat_param)
            {
                ctc_cli_out("Memory alloc error\n");
                return CLI_SUCCESS;
            }
        }
        sal_memset(g_ctc_stress_l3->ipuc_nat_param, 0, sizeof(ctc_ipuc_nat_sa_param_t)*g_ctc_stress_l3->ipuc_nat_param_entry_num);
    }

    if (NULL == g_ctc_stress_l3->ipuc_nat_param)
    {
        ctc_cli_out("Memory not alloc\n");
        return CLI_SUCCESS;
    }

    for (loop = 0; loop < g_ctc_stress_l3->ipuc_nat_param_entry_num; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == op.op_type)
        {
            sal_memcpy(&ipuc_nat_param, param, sizeof(ctc_ipuc_nat_sa_param_t));

            if ((CTC_CLI_STRESS_DATA_MODE_RANDOM == op.data_mode) || (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == op.data_mode))
            {
                ipuc_nat_param.vrf_id = sal_rand()%CTC_MAX_VRFID_ID;
                ipuc_nat_param.ipsa.ipv4 = sal_rand();
                if (ipuc_nat_param.l4_src_port)
                {
                    ipuc_nat_param.l4_src_port = (uint16)(sal_rand()+1);
                }
            }
            else
            {
                ipuc_nat_param.vrf_id = (ipuc_nat_param.vrf_id+loop)%CTC_MAX_VRFID_ID;
                ipuc_nat_param.ipsa.ipv4 = ipuc_nat_param.ipsa.ipv4+loop;
                if (ipuc_nat_param.l4_src_port)
                {
                    ipuc_nat_param.l4_src_port = ipuc_nat_param.l4_src_port+loop;
                }
            }

            if (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == op.data_mode)
            {
                ipuc_nat_param.new_ipsa.ipv4 = sal_rand();
                ipuc_nat_param.new_l4_src_port = (uint16)sal_rand();
            }

            if(g_ctcs_api_en)
            {
                ret = ctcs_ipuc_add_nat_sa(g_api_lchip, &ipuc_nat_param);
            }
            else
            {
                ret = ctc_ipuc_add_nat_sa(&ipuc_nat_param);
            }

            sal_memcpy(g_ctc_stress_l3->ipuc_nat_param + loop, &ipuc_nat_param, sizeof(ctc_ipuc_nat_sa_param_t));
        }
        else
        {
            sal_memcpy(&ipuc_nat_param, g_ctc_stress_l3->ipuc_nat_param + loop, sizeof(ctc_ipuc_nat_sa_param_t));

            if(g_ctcs_api_en)
            {
                ret = ctcs_ipuc_remove_nat_sa(g_api_lchip, &ipuc_nat_param);
            }
            else
            {
                ret = ctc_ipuc_remove_nat_sa(&ipuc_nat_param);
            }
        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, op.test_type, valid_count);
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == op.test_type)
    {
        sal_gettime(&tv2);

        sal_sprintf(str_buf,"Entry: %5d\n", g_ctc_stress_l3->ipuc_nat_param_entry_num);
        output.buf = str_buf;
        output.op = op.op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("Operate valid count: %d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == op.op_type)
    {
        sal_free(g_ctc_stress_l3->ipuc_nat_param);
        g_ctc_stress_l3->ipuc_nat_param = NULL;
        g_ctc_stress_l3->ipuc_nat_param_entry_num = 0;
    }

    return CLI_SUCCESS;
}

static int32
_ctc_cli_stress_ipuc(ctc_ipuc_param_t* param, ctc_cli_stress_t op, uint8 output_to_file)
{
    int32 ret = CLI_SUCCESS;
    sal_systime_t tv1,tv2;
    uint8  loop1 = 0;
    uint16 ip_bit[4] = {0};
    uint32 loop = 0;
    uint32 valid_count = 0;
    uint32 ipchg = 0;
    uint32 chg_max = 0;
    uint32 random_max = 0;
    uint32 max= 0xfffffffe;
    char str_buf[32] = {0};
    uint32 flagid[4] = {1, 4, 65536, 8192};
    ctc_cli_stress_output_t output;
    ctc_ipuc_param_t ipuc_param;
    sal_file_t fp = NULL;

    sal_memset(&ipuc_param, 0, sizeof(ctc_ipuc_param_t));

    if (g_ctc_stress_l3->ipuc_param && CTC_CLI_STRESS_OP_TYPE_ADD == op.op_type)
    {
        ctc_cli_out("Other ipuc stress have used, please remove first! \n");
        return CLI_ERROR;
    }
    if (CTC_CLI_STRESS_OP_TYPE_ADD == op.op_type)
    {
        if (g_ctc_stress_l3->prefix_len >= param->masklen && g_ctc_stress_l3->ipuc_vrfid_max == 0)
        {
            ctc_cli_out("stress test param invalid! \n");
            return CLI_ERROR;
        }
        if ((g_ctc_stress_l3->ipuc_vrfid_base >= 64) || (g_ctc_stress_l3->ipuc_vrfid_max >= 64))
        {
            ctc_cli_out("vrfid invalid! \n");
            return CLI_ERROR;
        }
        sal_memcpy(&ipuc_param, param, sizeof(ctc_ipuc_param_t));
        g_ctc_stress_l3->ipuc_param = (ctc_ipuc_param_t*)sal_malloc((g_ctc_stress_l3->ipuc_param_entry_num)*sizeof(ctc_ipuc_param_t));
        if ( g_ctc_stress_l3->ipuc_param == NULL)
        {
            ctc_cli_out("Memory alloc error! \n");
            return CLI_ERROR;
        }
        sal_memset(g_ctc_stress_l3->ipuc_param, 0, sizeof(ctc_ipuc_param_t)*(g_ctc_stress_l3->ipuc_param_entry_num));
    }
    if (NULL == g_ctc_stress_l3->ipuc_param)
    {
        ctc_cli_out("Memory not alloc! \n");
        return CLI_ERROR;
    }
    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == op.op_type && (param->ip_ver!=g_ctc_stress_l3->ipuc_param->ip_ver))
    {
        ctc_cli_out("Ip version wrong! \n");
        return CLI_ERROR;
    }
    for (loop = 0; loop < g_ctc_stress_l3->ipuc_param_entry_num; loop++)
    {
        if((CTC_CLI_STRESS_DATA_MODE_RANDOM == op.data_mode || CTC_CLI_STRESS_DATA_MODE_INCREASE == op.data_mode)\
            && CTC_CLI_STRESS_OP_TYPE_ADD == op.op_type && g_ctc_stress_l3->prefix_len < param->masklen)
        {
            random_max = (sal_rand()%((1 << 8) - 1)<<24)+(sal_rand()%((1 << 8) - 1)<<16)+(sal_rand()%((1 << 8) - 1)<<8)+ (sal_rand()%((1 << 8) - 1));
            if (CTC_IP_VER_4 == param->ip_ver)
            {
                ipchg = ipuc_param.ip.ipv4 >> (32 - param->masklen);
                chg_max = param->masklen - g_ctc_stress_l3->prefix_len;
                ipchg = (CTC_CLI_STRESS_DATA_MODE_INCREASE == op.data_mode) ? (ipchg + g_ctc_stress_l3->incr_step) : ((chg_max == 32)?(random_max):(sal_rand()%((1 << chg_max) - 1)));
                ipchg = (ipchg << (32 + g_ctc_stress_l3->prefix_len -param->masklen )) >> g_ctc_stress_l3->prefix_len;
                if(g_ctc_stress_l3->prefix_len ==0)
                {
                    ipuc_param.ip.ipv4=ipchg+((param->masklen==32)?0:((param->ip.ipv4<<param->masklen)>>param->masklen));
                }
                else
                {
                    ipuc_param.ip.ipv4=((param->ip.ipv4>>(32-g_ctc_stress_l3->prefix_len))<<(32-g_ctc_stress_l3->prefix_len))+ipchg+((param->masklen==32)?0:((param->ip.ipv4<<param->masklen)>>param->masklen));
                }
            }
            else
            {
                ip_bit[0] = g_ctc_stress_l3->prefix_len / 32;
                ip_bit[1] = g_ctc_stress_l3->prefix_len % 32;
                ip_bit[3] = param->masklen % 32;
                ip_bit[2] = (ip_bit[3] == 0) ? ((param->masklen / 32) - 1) : (param->masklen / 32);
                ip_bit[3] = (ip_bit[3] == 0) ? 32 : (param->masklen % 32);
                if (ip_bit[0] == ip_bit[2])
                {
                    ipchg = (ipuc_param.ip.ipv6[(ip_bit[0])] << ip_bit[1]) >> (32 - ip_bit[3] + ip_bit[1]);
                    chg_max = ip_bit[3] - ip_bit[1];
                    ipchg = (CTC_CLI_STRESS_DATA_MODE_INCREASE == op.data_mode) ? (ipchg + g_ctc_stress_l3->incr_step) : ((chg_max == 32)?(random_max):(sal_rand()%((1 << chg_max) - 1)));
                    ipchg = (ipchg << (32 + ip_bit[1] -ip_bit[3])) >> ip_bit[1];
                    if(ip_bit[1] == 0)
                    {
                        ipuc_param.ip.ipv6[(ip_bit[0])] = ipchg + ((ip_bit[3]==32)?0:((ipuc_param.ip.ipv6[(ip_bit[0])]<<ip_bit[3])>>ip_bit[3]));
                    }
                    else
                    {
                        ipuc_param.ip.ipv6[(ip_bit[0])] = ((ipuc_param.ip.ipv6[(ip_bit[0])]>>(32-ip_bit[1]))<<(32-ip_bit[1]))\
                            +ipchg+((ip_bit[3]==32)?0:((ipuc_param.ip.ipv6[(ip_bit[0])]<<ip_bit[3])>>ip_bit[3]));
                    }

                }
                else if (CTC_CLI_STRESS_DATA_MODE_RANDOM == op.data_mode)
                {
                    ipchg = (param->ip.ipv6[(ip_bit[0])] << ip_bit[1]) >> ip_bit[1];
                    chg_max = 32 - ip_bit[1];
                    ipchg = (chg_max == 32)?(random_max):(sal_rand()%((1 << chg_max) - 1));
                    ipchg = (ipchg << ip_bit[1]) >> ip_bit[1];
                    ipuc_param.ip.ipv6[(ip_bit[0])] = ((ip_bit[1]==0)?0:((param->ip.ipv6[(ip_bit[0])] >> (32 - ip_bit[1])) << (32 - ip_bit[1]))) + ipchg;
                    ipchg = param->ip.ipv6[(ip_bit[2])] >> (32 - ip_bit[3]);
                    chg_max = ip_bit[3];
                    ipchg = (chg_max == 32)?(random_max):(sal_rand()%(((uint64)1 << chg_max) - 1));
                    ipchg = ipchg << (32 - ip_bit[3]);
                    ipuc_param.ip.ipv6[(ip_bit[2])] = ((ip_bit[3]==32)?0:((param->ip.ipv6[(ip_bit[2])] << ip_bit[3])>> ip_bit[3])) +ipchg;
                    if(ip_bit[2] != ip_bit[0]+1)
                    {
                        for (loop1 = 0; loop1 < (ip_bit[2]-ip_bit[0]-1); loop1++)
                        {
                            ipuc_param.ip.ipv6[(ip_bit[0])+loop1+1] = (sal_rand()%((1 << 8) - 1)<<24)+(sal_rand()%((1 << 8) - 1)<<16)+(sal_rand()%((1 << 8) - 1)<<8)+ (sal_rand()%((1 << 8) - 1));
                        }
                    }
                }
                else
                {
                    ipuc_param.ip.ipv6[ip_bit[2]] = (g_ctc_stress_l3->incr_step<<(32-ip_bit[3])) + ipuc_param.ip.ipv6[ip_bit[2]];
                    if(((g_ctc_stress_l3->incr_step<<(32-ip_bit[3])) + ipuc_param.ip.ipv6[ip_bit[2]]) <= ipuc_param.ip.ipv6[ip_bit[2]])
                    {

                        if(ip_bit[0]==(ip_bit[2]-1))
                        {
                            if(((ipuc_param.ip.ipv6[ip_bit[0]]<<ip_bit[1])>>ip_bit[1]) > ((ip_bit[1]==0)?max:((1 << (32 - ip_bit[1])) - 2)))
                            {
                                ipuc_param.ip.ipv6[ip_bit[0]] = param->ip.ipv6[ip_bit[0]];
                            }
                            else
                            {
                                ipuc_param.ip.ipv6[ip_bit[0]] = ipuc_param.ip.ipv6[ip_bit[0]] + 1;
                            }
                        }
                        else if(ip_bit[0]==(ip_bit[2]-2))
                        {
                            ipuc_param.ip.ipv6[ip_bit[2] - 1] = (ipuc_param.ip.ipv6[ip_bit[2]-1])+1;
                            if(((ipuc_param.ip.ipv6[ip_bit[2]-1])+1) < (ipuc_param.ip.ipv6[ip_bit[2]-1]))
                            {
                                if(((ipuc_param.ip.ipv6[ip_bit[0]]<<ip_bit[1])>>ip_bit[1]) > ((ip_bit[1]==0)?max:((1 << (32 - ip_bit[1])) - 2)))
                                {
                                    ipuc_param.ip.ipv6[ip_bit[0]] = param->ip.ipv6[ip_bit[0]];
                                }
                                else
                                {
                                    ipuc_param.ip.ipv6[ip_bit[0]] = ipuc_param.ip.ipv6[ip_bit[0]] + 1;
                                }
                            }
                        }
                        else
                        {
                            ipuc_param.ip.ipv6[ip_bit[2] - 1] = (ipuc_param.ip.ipv6[ip_bit[2]-1])+1;
                            if(((ipuc_param.ip.ipv6[ip_bit[2]-1])+1) < (ipuc_param.ip.ipv6[ip_bit[2]-1]))
                            {
                                ipuc_param.ip.ipv6[ip_bit[2] - 2] = (ipuc_param.ip.ipv6[ip_bit[2]-2])+1;
                                if(((ipuc_param.ip.ipv6[ip_bit[2]-2])+1) < (ipuc_param.ip.ipv6[ip_bit[2]-2]))
                                {
                                    if(((ipuc_param.ip.ipv6[ip_bit[0]]<<ip_bit[1])>>ip_bit[1]) > ((ip_bit[1]==0)?max:((1 << (32 - ip_bit[1])) - 2)))
                                    {
                                        ipuc_param.ip.ipv6[ip_bit[0]] = param->ip.ipv6[ip_bit[0]];
                                    }
                                    else
                                    {
                                        ipuc_param.ip.ipv6[ip_bit[0]] = ipuc_param.ip.ipv6[ip_bit[0]] + 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if(g_ctc_stress_l3->ipuc_vrfid_max != 0)
            {
                if(CTC_CLI_STRESS_DATA_MODE_INCREASE == op.data_mode)
                {
                    ipuc_param.vrf_id =((g_ctc_stress_l3->ipuc_vrfid_max - ipuc_param.vrf_id) < g_ctc_stress_l3->incr_step) ?g_ctc_stress_l3->ipuc_vrfid_base:(ipuc_param.vrf_id+g_ctc_stress_l3->incr_step);
                }
                else
                {
                    ipuc_param.vrf_id =g_ctc_stress_l3->ipuc_vrfid_base + sal_rand()%(g_ctc_stress_l3->ipuc_vrfid_max-g_ctc_stress_l3->ipuc_vrfid_base);
                }
            }
            else
            {
                ipuc_param.vrf_id = g_ctc_stress_l3->ipuc_vrfid_base;
            }
            sal_memcpy(g_ctc_stress_l3->ipuc_param + loop, &ipuc_param, sizeof(ctc_ipuc_param_t));
        }
        if(CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL== op.data_mode && CTC_CLI_STRESS_OP_TYPE_ADD == op.op_type)
        {
            if(CTC_IP_VER_4 == param->ip_ver)
            {
                ipuc_param.ip.ipv4 = (sal_rand()%((1 << 8) - 1)<<24)+(sal_rand()%((1 << 8) - 1)<<16)+(sal_rand()%((1 << 8) - 1)<<8)+ (sal_rand()%((1 << 8) - 1));
                ipuc_param.masklen = sal_rand()%33;
                ipuc_param.route_flag = flagid[(sal_rand()%4)];
                ipuc_param.vrf_id = sal_rand()%64;
            }
            else
            {
                ipuc_param.ip.ipv6[0] = (sal_rand()%((1 << 8) - 1)<<24)+(sal_rand()%((1 << 8) - 1)<<16)+(sal_rand()%((1 << 8) - 1)<<8)+ (sal_rand()%((1 << 8) - 1));
                ipuc_param.ip.ipv6[1] = (sal_rand()%((1 << 8) - 1)<<24)+(sal_rand()%((1 << 8) - 1)<<16)+(sal_rand()%((1 << 8) - 1)<<8)+ (sal_rand()%((1 << 8) - 1));
                ipuc_param.ip.ipv6[2] = (sal_rand()%((1 << 8) - 1)<<24)+(sal_rand()%((1 << 8) - 1)<<16)+(sal_rand()%((1 << 8) - 1)<<8)+ (sal_rand()%((1 << 8) - 1));
                ipuc_param.ip.ipv6[3] = (sal_rand()%((1 << 8) - 1)<<24)+(sal_rand()%((1 << 8) - 1)<<16)+(sal_rand()%((1 << 8) - 1)<<8)+ (sal_rand()%((1 << 8) - 1));
                ipuc_param.masklen = sal_rand()%129;
                ipuc_param.route_flag = flagid[(sal_rand()%4)];
                ipuc_param.vrf_id = sal_rand()%64;
            }
            sal_memcpy(g_ctc_stress_l3->ipuc_param + loop, &ipuc_param, sizeof(ctc_ipuc_param_t));
        }
    }
    if (CTC_CLI_STRESS_TEST_TYPE_PERF == op.test_type)
    {
        sal_gettime(&tv1);
    }
    if (CTC_CLI_STRESS_OP_TYPE_ADD == op.op_type && 1 == output_to_file)
    {
        fp = sal_fopen("ipuc_add.txt", "w");
    }
    if(CTC_CLI_STRESS_OP_TYPE_REMOVE == op.op_type && 1 == output_to_file)
    {
        fp = sal_fopen("ipuc_remove.txt", "w");
    }
    for (loop = 0; loop < g_ctc_stress_l3->ipuc_param_entry_num; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == op.op_type)
        {
            sal_memcpy(&ipuc_param, g_ctc_stress_l3->ipuc_param + loop, sizeof(ctc_ipuc_param_t));
            ret = g_ctcs_api_en ? ctcs_ipuc_add(g_api_lchip, &ipuc_param) : ctc_ipuc_add(&ipuc_param);
            if (CTC_E_TOO_MANY_FRAGMENT == ret)
            {
                ctc_ipuc_arrange_fragment(NULL);
                ret = g_ctcs_api_en ? ctcs_ipuc_add(g_api_lchip, &ipuc_param) : ctc_ipuc_add(&ipuc_param);
            }
            if(1 == output_to_file)
            {
              if(CTC_IP_VER_4 == param->ip_ver)
              {
                  sal_fprintf(fp, "ipuc add %d %d.%d.%d.%d %d 2\n", ipuc_param.vrf_id, ipuc_param.ip.ipv4 >> 24,
                          (ipuc_param.ip.ipv4 >> 16)&0xff,
                          (ipuc_param.ip.ipv4 >> 8)&0xff,
                          (ipuc_param.ip.ipv4)&0xff,
                          ipuc_param.masklen);
              }
              else
              {
                  sal_fprintf(fp, "ipuc ipv6 add %d %x:%x:%x:%x:%x:%x:%x:%x %d 2\n" , ipuc_param.vrf_id, ipuc_param.ip.ipv6[0] >> 16,
                          ipuc_param.ip.ipv6[0]&0xffff,
                          ipuc_param.ip.ipv6[1] >> 16,
                          ipuc_param.ip.ipv6[1]&0xffff,
                          ipuc_param.ip.ipv6[2] >> 16,
                          ipuc_param.ip.ipv6[2]&0xffff,
                          ipuc_param.ip.ipv6[3] >> 16,
                          ipuc_param.ip.ipv6[3]&0xffff,
                          ipuc_param.masklen);
              }
            }
        }
        if(CTC_CLI_STRESS_OP_TYPE_REMOVE == op.op_type)
        {
            sal_memcpy(&ipuc_param, g_ctc_stress_l3->ipuc_param + loop, sizeof(ctc_ipuc_param_t));
            ret = g_ctcs_api_en ? ctcs_ipuc_remove(g_api_lchip, &ipuc_param) : ctc_ipuc_remove(&ipuc_param);
            if(1 == output_to_file)
            {
              if(CTC_IP_VER_4 == param->ip_ver)
              {
                  sal_fprintf(fp, "ipuc remove %d %d.%d.%d.%d %d 2\n", ipuc_param.vrf_id, ipuc_param.ip.ipv4 >> 24,
                          (ipuc_param.ip.ipv4 >> 16)&0xff,
                          (ipuc_param.ip.ipv4 >> 8)&0xff,
                          (ipuc_param.ip.ipv4)&0xff,
                          ipuc_param.masklen);
              }
              else
              {
                  sal_fprintf(fp, "ipuc ipv6 remove %d %x:%x:%x:%x:%x:%x:%x:%x %d 2\n" , ipuc_param.vrf_id, ipuc_param.ip.ipv6[0] >> 16,
                          ipuc_param.ip.ipv6[0]&0xffff,
                          ipuc_param.ip.ipv6[1] >> 16,
                          ipuc_param.ip.ipv6[1]&0xffff,
                          ipuc_param.ip.ipv6[2] >> 16,
                          ipuc_param.ip.ipv6[2]&0xffff,
                          ipuc_param.ip.ipv6[3] >> 16,
                          ipuc_param.ip.ipv6[3]&0xffff,
                          ipuc_param.masklen);
              }
            }
        }
        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
    }
    if( 1 == output_to_file)
    {
        sal_fclose(fp);
    }
    if (CTC_CLI_STRESS_TEST_TYPE_STRESS== op.test_type)
    {
        ctc_cli_out("Operate valid count: %d \n", valid_count);
    }
    if (CTC_CLI_STRESS_TEST_TYPE_PERF == op.test_type)
    {
        sal_gettime(&tv2);

        sal_sprintf(str_buf,"Entry: %5d", g_ctc_stress_l3->ipuc_param_entry_num);
        output.buf = str_buf;
        output.op = op.op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }
    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == op.op_type)
    {
        sal_free(g_ctc_stress_l3->ipuc_param);
        g_ctc_stress_l3->ipuc_param = NULL;
        g_ctc_stress_l3->ipuc_param_entry_num = 0;
    }
    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stress_ipmc_group_add,
        ctc_cli_stress_ipmc_group_add_cmd,
        "ipmc group GROUP_BASE add (ipv4-group-address A.B.C.D MASK_LEN ipv4-source-address A.B.C.D MASK_LEN | ipv6-group-address X:X::X:X MASK_LEN ipv6-source-address X:X::X:X MASK_LEN) (vrf VRFID|) (ttl-check | rpf-check | rpf-fail-to-cpu | ip-based-l2mc | ptp-entry | copy-tocpu |)(random|incr (step STEP|)|random-all) (group-num GRP_NUM ipsa-num IPSA_NUM) (spec|stress|perf)",
        CTC_CLI_IPMC_M_STR,
        CTC_CLI_IPMC_GROUP_DESC,
        "Group base",
        "Add",
        "IPv4 group address class D IP address",
        CTC_CLI_IPV4_FORMAT,
        "Ipv4 length of group address mask <32>",
        "IPv4 source address",
        CTC_CLI_IPV4_FORMAT,
        "IPv4 length of source address mask <0 or 32>",
        "IPv6 group address class D IP address",
        CTC_CLI_IPV6_FORMAT,
        "Ipv6 length of group address mask <128>",
        "IPv6 Source address",
        CTC_CLI_IPV6_FORMAT,
        "IPv6 length of source address mask <0 or 128>",
        "VRF id or FID",
        "VRF id or FID value",
        "TTL check enable",
        "RPF check enable",
        "If RPF check fail, sent to cpu",
        "Do IP based L2 mcast",
        "Ptp-entry",
        "Copy to cpu",
        "Key value is random, Ad is fixed",
        "Key value is increase, Ad is fixed",
        "Increase by step",
        "Num of step",
        "Key value is random, Ad is random",
        "Group ipda number",
        "<0-0xFFFFFFFF>",
        "Source ipsa number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint8  index = 0;
    ctc_ipmc_group_info_t* p_group_info = NULL;
    uint32 ipv6_address[4] = {0, 0, 0, 0};
    ctc_cli_stress_t stress;
    uint32 flag = 0;
    uint32 group_id = 0;
    ctc_ipmc_addr_info_t address;
    uint8 ip_version = 0;
    uint8 group_ip_mask_len = 0;
    uint8 src_ip_mask_len = 0;
    uint8   rp_id = 0;
    uint32 stats_id = 0;

    sal_memset(&address, 0, sizeof(ctc_ipmc_addr_info_t));
    CTC_CLI_GET_UINT32("group_base", group_id, argv[0]);

    if (g_ctc_stress_l3->ipmc_group_num)
    {
        ctc_cli_out("Other stress test have used, please remove first! \n");
        return CLI_ERROR;
    }
    g_ctc_stress_l3->incr_step = 1;
    stress.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ipv4-group-address");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ip", address.ipv4.group_addr, argv[index + 1]);
        ip_version = CTC_IP_VER_4;
        CTC_CLI_GET_UINT8("mask", group_ip_mask_len, argv[index + 2]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ipv4-source-address");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ip", address.ipv4.src_addr, argv[index + 1]);
        CTC_CLI_GET_UINT8("mask", src_ip_mask_len, argv[index + 2]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ipv6-group-address");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[index + 1]);
        /* adjust endian */
        address.ipv6.group_addr[0] = sal_htonl(ipv6_address[0]);
        address.ipv6.group_addr[1] = sal_htonl(ipv6_address[1]);
        address.ipv6.group_addr[2] = sal_htonl(ipv6_address[2]);
        address.ipv6.group_addr[3] = sal_htonl(ipv6_address[3]);
        ip_version = CTC_IP_VER_6;
        CTC_CLI_GET_UINT8("mask", group_ip_mask_len, argv[index + 2]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ipv6-source-address");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[index + 1]);
        /* adjust endian */
        address.ipv6.src_addr[0] = sal_htonl(ipv6_address[0]);
        address.ipv6.src_addr[1] = sal_htonl(ipv6_address[1]);
        address.ipv6.src_addr[2] = sal_htonl(ipv6_address[2]);
        address.ipv6.src_addr[3] = sal_htonl(ipv6_address[3]);
        CTC_CLI_GET_UINT8("mask", src_ip_mask_len, argv[index + 2]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("vrf");
    if (index != 0xFF)
    {
        if (CTC_IP_VER_4 == ip_version)
        {
            CTC_CLI_GET_UINT32("vrf", address.ipv4.vrfid, argv[index + 1]);
        }
        else
        {
            CTC_CLI_GET_UINT32("vrf", address.ipv6.vrfid, argv[index + 1]);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ttl-check");
    if (index != 0xFF)
    {
        flag |= CTC_IPMC_FLAG_TTL_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-check");
    if (index != 0xFF)
    {
        flag |= CTC_IPMC_FLAG_RPF_CHECK;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("rpf-fail-to-cpu");
    if (index != 0xFF)
    {
        flag |= CTC_IPMC_FLAG_RPF_FAIL_BRIDGE_AND_TOCPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stats");
        if (index != 0xFF)
    {
        flag |= CTC_IPMC_FLAG_STATS;
        CTC_CLI_GET_UINT32("stats-id", stats_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ip-based-l2mc");
    if (index != 0xFF)
    {
        flag |= CTC_IPMC_FLAG_L2MC;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ptp-entry");
    if (index != 0xFF)
    {
        flag |= CTC_IPMC_FLAG_PTP_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("redirect-tocpu");
    if (index != 0xFF)
    {
        flag |= CTC_IPMC_FLAG_REDIRECT_TOCPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("copy-tocpu");
    if (index != 0xFF)
    {
        flag |= CTC_IPMC_FLAG_COPY_TOCPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("bidipim");
    if (index != 0xFF)
    {
        flag |= CTC_IPMC_FLAG_BIDPIM_CHECK;
        CTC_CLI_GET_UINT8_RANGE("rp-id", rp_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("bidipim-fail-to-cpu");
    if (index != 0xFF)
    {
        flag |= CTC_IPMC_FLAG_BIDIPIM_FAIL_BRIDGE_AND_TOCPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("group-num");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("grp_num", g_ctc_stress_l3->ipmc_group_num, argv[index + 1]);
    }
     index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ipsa-num");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("ipsa_num", g_ctc_stress_l3->ipmc_ipsa_num, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random");
    if (index != 0xFF)
    {
        stress.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("incr");
    if (index != 0xFF)
    {
        stress.data_mode = CTC_CLI_STRESS_DATA_MODE_INCREASE;
        index = CTC_CLI_GET_ARGC_INDEX("step");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("step", g_ctc_stress_l3->incr_step, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random-all");
    if (index != 0xFF)
    {
        stress.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL;
    }

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    p_group_info = (ctc_ipmc_group_info_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_ipmc_group_info_t));
    if (NULL == p_group_info)
    {
        return CLI_ERROR;
    }
    sal_memset(p_group_info, 0, sizeof(ctc_ipmc_group_info_t));
    p_group_info->address.ipv4.vrfid = 0;
    p_group_info->address.ipv6.vrfid = 0;
    p_group_info->group_id = group_id;
    p_group_info->ip_version = ip_version;
    p_group_info->group_ip_mask_len = group_ip_mask_len;
    p_group_info->src_ip_mask_len = src_ip_mask_len;
    p_group_info->flag = flag;
    p_group_info->stats_id = stats_id;
    p_group_info->rp_id = rp_id;
    ret = _ctc_cli_stress_ipmc_group(&stress, p_group_info);
    mem_free(p_group_info);

    return ret;
}

CTC_CLI(ctc_cli_stress_ipmc_group_remove,
        ctc_cli_stress_ipmc_group_remove_cmd,
        "ipmc group GROUP_BASE remove (perf|)",
        CTC_CLI_IPMC_M_STR,
        CTC_CLI_IPMC_GROUP_DESC,
        "Group base",
        "Remove ipmc group",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    ctc_cli_stress_t stress;
    uint8 index = 0xFF;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    stress.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    stress.data_mode = CTC_CLI_STRESS_DATA_MODE_MAX;
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }
    ret = _ctc_cli_stress_ipmc_group(&stress, 0);

    return ret;
}


CTC_CLI(ctc_cli_stress_ipmc_member,
        ctc_cli_stress_ipmc_member_cmd,
        "ipmc member add " \
        "(ip-based-l2mc |) ("CTC_CLI_IPMC_MEMBER_L3IF_ALL_STR" (remote-chip |))(random|incr (step STEP|)|random-all) (member-num MEMBER_NUM) (spec|stress|perf)",
        CTC_CLI_IPMC_M_STR,
        CTC_CLI_IPMC_MEMBER_DESC,
        "Add",
        "IP based l2 mcast",
        CTC_CLI_IPMC_L3IF_ALL_DESC,
        "Indicate the member is remote chip entry",
        "Key value is random, Ad is fixed",
        "Key value is increase, Ad is fixed",
        "Increase by step",
        "Num of step",
        "Key value is random, Ad is random",
        "ipmc member number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    ctc_cli_stress_t stress;
    ctc_ipmc_group_info_t* p_group_info = NULL;
    uint32 l3_if_type = 0;
    uint32 flag = 0;
    uint32 vlan_id = 0;
    uint32 gport = 0;
    uint32 nh_id = 0;
    uint8 gchip_id = 0;
    uint8 member_number = 0;
    uint8 port_bmp_en = 0;
    bool is_nh = FALSE;
    bool remote_chip = FALSE;
    ctc_port_bitmap_t  port_bmp;

    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));

    l3_if_type = MAX_L3IF_TYPE_NUM;
    g_ctc_stress_l3->incr_step = 1;
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("phy-if");
    if (index != 0xFF)
    {
        l3_if_type = CTC_L3IF_TYPE_PHY_IF;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ip-based-l2mc");
    if (index != 0xFF)
    {
        flag |= CTC_IPMC_FLAG_L2MC;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("sub-if");
    if (index != 0xFF)
    {
        l3_if_type = CTC_L3IF_TYPE_SUB_IF;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vlan-if");
    if (index != 0xFF)
    {
        l3_if_type = CTC_L3IF_TYPE_VLAN_IF;
    }
    index = CTC_CLI_GET_ARGC_INDEX("vlanid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("vlanid", vlan_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
     /*index = CTC_CLI_GET_SPECIFIC_INDEX("gport", 1);*/
    if (index != 0xFF)
    {
        member_number = 1;
        CTC_CLI_GET_UINT32_RANGE("portId", gport, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("nexthop-id");
    if (index != 0xFF)
    {
        is_nh = TRUE;
        CTC_CLI_GET_UINT32("NexthopID", nh_id, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("remote-chip");
    if (index != 0xFF)
    {
        remote_chip = TRUE;
    }
    port_bmp_en = 0;
    CTC_CLI_PORT_BITMAP_GET(port_bmp);
    if (port_bmp[0] || port_bmp[1]
        || port_bmp[8] || port_bmp[9])
    {
        port_bmp_en = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("gchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("gchip", gchip_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("member-num");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("member-num", g_ctc_stress_l3->ipmc_member_num, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random");
    if (index != 0xFF)
    {
        stress.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM;
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("incr");
    if (index != 0xFF)
    {
        stress.data_mode = CTC_CLI_STRESS_DATA_MODE_INCREASE;
        index = CTC_CLI_GET_ARGC_INDEX("step");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("step", g_ctc_stress_l3->incr_step, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random-all");
    if (index != 0xFF)
    {
        stress.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL;
    }
    stress.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    p_group_info = (ctc_ipmc_group_info_t*)mem_malloc(MEM_CLI_MODULE, sizeof(ctc_ipmc_group_info_t));
    if (NULL == p_group_info)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_group_info, 0, sizeof(ctc_ipmc_group_info_t));

    p_group_info->ipmc_member[0].l3_if_type = l3_if_type;
    p_group_info->flag = flag;
    p_group_info->ipmc_member[0].vlan_id = vlan_id;
    p_group_info->member_number = member_number;
    p_group_info->ipmc_member[0].global_port = gport;
    p_group_info->ipmc_member[0].is_nh = is_nh;
    p_group_info->ipmc_member[0].nh_id = nh_id;
    p_group_info->ipmc_member[0].remote_chip = remote_chip;
    p_group_info->ipmc_member[0].port_bmp_en = port_bmp_en;
    p_group_info->ipmc_member[0].gchip_id = gchip_id;
    sal_memcpy(p_group_info->ipmc_member[0].port_bmp, port_bmp, sizeof(ctc_port_bitmap_t));

    ret = _ctc_cli_stress_ipmc_member(&stress, p_group_info);

    mem_free(p_group_info);
    return ret;
}

CTC_CLI(ctc_cli_stress_ipuc_add_tunnel_ipv4,
        ctc_cli_stress_ipuc_add_tunnel_ipv4_cmd,
        "ipuc tunnel add ipda-base A.B.C.D {ipsa-base A.B.C.D | tunnel-payload-type TYPE | logic-port Port |cid CID | nh-id NHID | stats STATSID | policer-id POLICER_ID | flag FLAG |} (random|incr|random-all) entry-num NUM (spec|stress|perf)",
        CTC_CLI_IPUC_M_STR,
        "This route is ip tunnel",
        "Add ip tunnel route",
        "Ipda base",
        CTC_CLI_IPV4_FORMAT,
        "Ipsa base",
        CTC_CLI_IPV4_FORMAT,
        "Tunnel-payload-type",
        "Payload type",
        "Logic port",
        "Port",
        "Cid",
        "CID",
        "Nexthop",
        "Nexthop id",
        "Stats",
        "Stats id",
        "Policer",
        "Policer id",
        "tunnel flag",
        "Misc flag",
        "Random key",
        "Increase key",
        "Random key and ad",
        "Entry num",
        "NUM",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 num_index = 0;
    ctc_ipuc_tunnel_param_t param = {0};
    ctc_cli_stress_t op = {0};

    if (g_ctc_stress_l3->ipuc_tunnel_param_entry_num)
    {
        ctc_cli_out("Other stress test have used, please remove first! \n");
        return CLI_ERROR;
    }
    op.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;
    param.payload_type = MAX_CTC_IPUC_TUNNEL_PAYLOAD_TYPE;
    param.ip_ver = CTC_IP_VER_4;

    CTC_CLI_GET_IPV4_ADDRESS("ip", param.ip_da.ipv4, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ipsa-base");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ip", param.ip_sa.ipv4, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tunnel-payload-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("type", param.payload_type, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("port", param.logic_port, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("cid", param.cid, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("nh-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("nh-id", param.nh_id, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stats");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("type", param.stats_id, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("policer-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("policer-id", param.policer_id, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("flag");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("flag", param.flag, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("incr");
    if (index != 0xFF)
    {
        num_index = index;
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_INCREASE;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random");
    if (index != 0xFF)
    {
        num_index = index;
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random-all");
    if (index != 0xFF)
    {
        num_index = index;
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL;
    }

    CTC_CLI_GET_UINT16("entry-num", g_ctc_stress_l3->ipuc_tunnel_param_entry_num, argv[num_index+1]);

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != index)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != index)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_ipuc_tunnel(&param, op);

    if (ret)
    {
        g_ctc_stress_l3->ipuc_tunnel_param_entry_num = 0;
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stress_ipuc_add_tunnel_ipv6,
        ctc_cli_stress_ipuc_add_tunnel_ipv6_cmd,
        "ipuc ipv6 tunnel add ipda-base X:X:X:X {ipsa-base X:X:X:X | tunnel-payload-type TYPE | logic-port Port |cid CID | nh-id NHID | stats STATSID | policer-id POLICER_ID | flag FLAG |} (random|incr|random-all) entry-num NUM (spec|stress|perf)",
        CTC_CLI_IPUC_M_STR,
        "Ipv6",
        "This route is ip tunnel",
        "Add ip tunnel route",
        "Ipda base",
        CTC_CLI_IPV6_FORMAT,
        "Ipsa base",
        CTC_CLI_IPV6_FORMAT,
        "Tunnel-payload-type",
        "Payload type",
        "Logic port",
        "Port",
        "Cid",
        "CID",
        "Nexthop",
        "Nexthop id",
        "Stats",
        "Stats id",
        "Policer",
        "Policer id",
        "tunnel flag",
        "Misc flag",
        "Random key",
        "Increase key",
        "Random key and ad",
        "Entry num",
        "NUM",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 num_index = 0;
    ipv6_addr_t ipv6_address;
    ctc_ipuc_tunnel_param_t param = {0};
    ctc_cli_stress_t op = {0};

    if (g_ctc_stress_l3->ipuc_tunnel_param_entry_num)
    {
        ctc_cli_out("Other stress test have used, please remove first! \n");
        return CLI_ERROR;
    }
    op.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;
    param.payload_type = MAX_CTC_IPUC_TUNNEL_PAYLOAD_TYPE;
    param.ip_ver = CTC_IP_VER_6;

    CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[0]);
    /* adjust endian */
    param.ip_da.ipv6[0] = sal_htonl(ipv6_address[0]);
    param.ip_da.ipv6[1] = sal_htonl(ipv6_address[1]);
    param.ip_da.ipv6[2] = sal_htonl(ipv6_address[2]);
    param.ip_da.ipv6[3] = sal_htonl(ipv6_address[3]);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ipsa-base");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV6_ADDRESS("ipv6", ipv6_address, argv[index+1]);
        /* adjust endian */
        param.ip_sa.ipv6[0] = sal_htonl(ipv6_address[0]);
        param.ip_sa.ipv6[1] = sal_htonl(ipv6_address[1]);
        param.ip_sa.ipv6[2] = sal_htonl(ipv6_address[2]);
        param.ip_sa.ipv6[3] = sal_htonl(ipv6_address[3]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tunnel-payload-type");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("type", param.payload_type, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("logic-port");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("port", param.logic_port, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("cid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("cid", param.cid, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("nh-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("nh-id", param.nh_id, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("stats");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("statsid", param.stats_id, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("policer-id");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("policer-id", param.policer_id, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("flag");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("flag", param.flag, argv[index+1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("incr");
    if (index != 0xFF)
    {
        num_index = index;
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_INCREASE;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random");
    if (index != 0xFF)
    {
        num_index = index;
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random-all");
    if (index != 0xFF)
    {
        num_index = index;
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL;
    }

    CTC_CLI_GET_UINT16("entry-num", g_ctc_stress_l3->ipuc_tunnel_param_entry_num, argv[num_index+1]);

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != index)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != index)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_ipuc_tunnel(&param, op);

    if (ret)
    {
        g_ctc_stress_l3->ipuc_tunnel_param_entry_num = 0;
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return ret;
}

CTC_CLI(ctc_cli_stress_ipuc_remove_tunnel,
        ctc_cli_stress_ipuc_remove_tunnel_cmd,
        "ipuc (ipv6|) tunnel remove",
        CTC_CLI_IPUC_M_STR,
        "Ipv6",
        "This route is ip tunnel",
        "Remove all ip tunnel route")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    ctc_ipuc_tunnel_param_t param = {0};
    ctc_cli_stress_t op = {0};

    op.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;

    param.ip_ver = CTC_IP_VER_4;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ipv6");
    if (index != 0xFF)
    {
        param.ip_ver = CTC_IP_VER_6;
    }

    ret = _ctc_cli_stress_ipuc_tunnel(&param, op);

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stress_ipuc_add_nat_sa_ipv4,
        ctc_cli_stress_ipuc_add_nat_sa_ipv4_cmd,
        "ipuc nat-sa add vrfid-base VRFID ipsa-base A.B.C.D (l4-src-port-base L4PORT (tcp-port|)|) (new-ipsa-base A.B.C.D|) (new-l4-src-port-base L4PORT|) (random|incr|random-all) entry-num NUM (spec|stress|perf)",
        CTC_CLI_IPUC_M_STR,
        "Nat sa",
        "Add",
        "Vrfid base",
        CTC_CLI_VRFID_ID_DESC,
        "Ipsa base",
        CTC_CLI_IPV4_FORMAT,
        "Layer4 base source port for NAPT",
        CTC_CLI_L4_PORT_VALUE,
        "Indicate l4-src-port is tcp port or not. If set is TCP port else is UDP port",
        "New ipsa base",
        CTC_CLI_IPV4_FORMAT,
        "Layer4 destination base port for NAPT",
        CTC_CLI_L4_PORT_VALUE,
        "Random key",
        "Increase key",
        "Random key and ad",
        "Entry num",
        "NUM",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 num_index = 0;
    ctc_ipuc_nat_sa_param_t param = {0};
    ctc_cli_stress_t op = {0};

    if (g_ctc_stress_l3->ipuc_nat_param_entry_num)
    {
        ctc_cli_out("Other stress test have used, please remove first! \n");
        return CLI_ERROR;
    }
    op.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;
    param.ip_ver = CTC_IP_VER_4;

    CTC_CLI_GET_UINT16_RANGE("vrfid", param.vrf_id, argv[0], 0, CTC_MAX_UINT8_VALUE);
    CTC_CLI_GET_IPV4_ADDRESS("ip", param.ipsa.ipv4, argv[1]);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("l4-src-port-base");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("l4-src-port", param.l4_src_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("tcp-port");
        if (index != 0xFF)
        {
            param.flag = CTC_IPUC_NAT_FLAG_USE_TCP_PORT;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("new-ipsa-base");
    if (index != 0xFF)
    {
        CTC_CLI_GET_IPV4_ADDRESS("new ip", param.new_ipsa.ipv4, argv[index+1]);
    }


    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("new-l4-src-port-base");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16_RANGE("new-l4-src-port", param.new_l4_src_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("incr");
    if (index != 0xFF)
    {
        num_index = index;
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_INCREASE;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random");
    if (index != 0xFF)
    {
        num_index = index;
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random-all");
    if (index != 0xFF)
    {
        num_index = index;
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL;
    }

    CTC_CLI_GET_UINT16("entry-num", g_ctc_stress_l3->ipuc_nat_param_entry_num, argv[num_index+1]);

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != index)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != index)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_ipuc_natsa(&param, op);

    if (ret)
    {
        g_ctc_stress_l3->ipuc_nat_param_entry_num = 0;
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stress_ipuc_remove_nat_sa_ipv4,
        ctc_cli_stress_ipuc_remove_nat_sa_ipv4_cmd,
        "ipuc nat-sa remove",
        CTC_CLI_IPUC_M_STR,
        "This route is ip nat sa",
        "Remove all nat sa route")
{
    int32 ret = CLI_SUCCESS;
    ctc_ipuc_nat_sa_param_t param = {0};
    ctc_cli_stress_t op = {0};

    op.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;

    param.ip_ver = CTC_IP_VER_4;

    ret = _ctc_cli_stress_ipuc_natsa(&param, op);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_stress_ipuc_ipv4,
        ctc_cli_stress_ipuc_ipv4_cmd,
        "ipuc add vrfid BASE (vrfmax MAX |) A.B.C.D (mask MASK_LEN) (prefix PRE-LEN|) (nhid NHID|) (flag FLAG|) (random |incr STEP|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)(output|)",
        CTC_CLI_IPUC_M_STR,
        "Add",
        "Vrfid base(<64)",
        "Base value",
        "Vrfid max(<64)",
        "Max value",
        CTC_CLI_IPV4_FORMAT,
        "Mask",
        "Mask length",
        "Prefix(only used in Random and Incr mode)",
        "Prefix length",
        "Nexthop id",
        CTC_CLI_NH_ID_STR,
        "Route flag",
        "Misc flag, RPF_CHECK:0x01, ICMP_CHECK:0x04, HOST_USE_LPM:0x10000, PUBLIC_ROUTE:0x2000",
        "Random key",
        "Increase key",
        "Increase step",
        "Random key and ad",
        "Entry-num",
        "Entry num",
        "Spec test",
        "Stress test",
        "Performance test",
        "Output to file")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 ipindex = 0;
    uint8 output = 0;
    ctc_ipuc_param_t param = {0};
    ctc_cli_stress_t op = {0};
    op.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;
    param.ip_ver = CTC_IP_VER_4;
    g_ctc_stress_l3->ipuc_vrfid_base = 0;
    g_ctc_stress_l3->ipuc_vrfid_max = 0;
    g_ctc_stress_l3->prefix_len = 0;
    param.nh_id = 1;
    param.route_flag = 0;
    ipindex = 1;

    if (g_ctc_stress_l3->ipuc_param_entry_num)
    {
        ctc_cli_out("Other stress test have used, please remove first! \n");
        return CLI_ERROR;
    }
    CTC_CLI_GET_UINT16("vrfid", g_ctc_stress_l3->ipuc_vrfid_base, argv[0]);
    CTC_CLI_GET_UINT16("vrfid", param.vrf_id, argv[0]);
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vrfmax");
    if (index != 0xFF)
    {
        ipindex = ipindex + 2;
        CTC_CLI_GET_UINT16("vrfmax", g_ctc_stress_l3->ipuc_vrfid_max, argv[index + 1]);
    }
    CTC_CLI_GET_IPV4_ADDRESS("ipaddr", param.ip.ipv4, argv[ipindex]);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mask");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("mask", param.masklen, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("prefix");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("prefix", g_ctc_stress_l3->prefix_len, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("nhid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("nhid", param.nh_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("flag");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("flag", param.route_flag, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("incr");
    if (index != 0xFF)
    {
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_INCREASE;
        CTC_CLI_GET_UINT32("step", g_ctc_stress_l3->incr_step, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random");
    if (index != 0xFF)
    {
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random-all");
    if (index != 0xFF)
    {
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("entry-num");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("entry-num", g_ctc_stress_l3->ipuc_param_entry_num, argv[index + 1]);
        if (g_ctc_stress_l3->ipuc_param_entry_num > (600*1024))
        {
            ctc_cli_out("Invalid entry num!\n");
            g_ctc_stress_l3->ipuc_param_entry_num = 0;
            return CLI_ERROR;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != index)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != index)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("output");
    if (0xFF != index)
    {
        output = 1;
    }

    ret = _ctc_cli_stress_ipuc(&param,op, output);

   if (ret)
    {
        g_ctc_stress_l3->ipuc_param_entry_num = 0;
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_stress_ipuc_ipv6,
        ctc_cli_stress_ipuc_ipv6_cmd,
        "ipuc ipv6 add vrfid BASE (vrfmax MAX |) X:X::X:X (mask MASK_LEN) (prefix PRE-LEN|) (nhid NHID|) (flag FLAG|) (random |incr STEP|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)(output|)",
        CTC_CLI_IPUC_M_STR,
        CTC_CLI_IPV6_STR,
        "Add",
        "Vrfid base(<64)",
        "Base value",
        "Vrfid max(<64)",
        "Max value",
        CTC_CLI_IPV6_FORMAT,
        "Mask",
        "Mask length",
        "Prefix(only used in Random and Incr mode)",
        "Prefix length",
        "Nexthop id",
        CTC_CLI_NH_ID_STR,
        "Route flag",
        "Misc flag, RPF_CHECK:0x01, ICMP_CHECK:0x04, HOST_USE_LPM:0x10000, PUBLIC_ROUTE:0x2000",
        "Random key",
        "Increase key",
        "Increase step",
        "Random key and ad",
        "Entry-num",
        "Entry num",
        "Spec test",
        "Stress test",
        "Performance test",
        "output to file")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 ipindex = 0;
    uint8 output = 0;
    ipv6_addr_t ipv6_address;
    ctc_ipuc_param_t param = {0};
    ctc_cli_stress_t op = {0};
    op.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;
    param.ip_ver = CTC_IP_VER_6;
    g_ctc_stress_l3->ipuc_vrfid_base = 0;
    g_ctc_stress_l3->ipuc_vrfid_max = 0;
    g_ctc_stress_l3->prefix_len = 0;
    param.nh_id = 1;
    param.route_flag = 0;
    ipindex = 1;
    if (g_ctc_stress_l3->ipuc_param_entry_num)
    {
        ctc_cli_out("Other stress test have used, please remove first! \n");
        return CLI_ERROR;
    }

    CTC_CLI_GET_UINT16("vrfid", g_ctc_stress_l3->ipuc_vrfid_base, argv[0]);
    CTC_CLI_GET_UINT16("vrfid", param.vrf_id, argv[0]);
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("vrfmax");
    if (index != 0xFF)
    {
        ipindex=ipindex+2;
        CTC_CLI_GET_UINT16("vrfmax", g_ctc_stress_l3->ipuc_vrfid_max, argv[index + 1]);
    }
    CTC_CLI_GET_IPV6_ADDRESS("ipaddr", ipv6_address, argv[ipindex]);
    param.ip.ipv6[0] = sal_htonl(ipv6_address[0]);
    param.ip.ipv6[1] = sal_htonl(ipv6_address[1]);
    param.ip.ipv6[2] = sal_htonl(ipv6_address[2]);
    param.ip.ipv6[3] = sal_htonl(ipv6_address[3]);

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("mask");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("mask", param.masklen, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("prefix");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("prefix", g_ctc_stress_l3->prefix_len, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("nhid");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT16("nhid", param.nh_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("flag");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("flag", param.route_flag, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("incr");
    if (index != 0xFF)
    {
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_INCREASE;
        CTC_CLI_GET_UINT32("step", g_ctc_stress_l3->incr_step, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random");
    if (index != 0xFF)
    {
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("random-all");
    if (index != 0xFF)
    {
        op.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("entry-num");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT32("entry-num", g_ctc_stress_l3->ipuc_param_entry_num, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != index)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != index)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    index = CTC_CLI_GET_ARGC_INDEX("output");
    if (0xFF != index)
    {
        output = 1;
    }

    ret = _ctc_cli_stress_ipuc(&param,op, output);

   if (ret)
    {
        g_ctc_stress_l3->ipuc_param_entry_num = 0;
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stress_ipuc_remove,
        ctc_cli_stress_ipuc_remove_cmd,
        "ipuc (ipv6|) remove (perf|)(output|)",
        CTC_CLI_IPUC_M_STR,
        "Ipv6",
        "Remove all ip route",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 output = 0;
    ctc_ipuc_param_t param = {0};
    ctc_cli_stress_t op = {0};

    op.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    op.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    param.ip_ver = CTC_IP_VER_4;

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("ipv6");
    if (index != 0xFF)
    {
        param.ip_ver = CTC_IP_VER_6;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("perf");
    if (index != 0xFF)
    {
        op.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("output");
    if (index != 0xFF)
    {
        output = 1;
    }

    ret = _ctc_cli_stress_ipuc(&param, op, output);

    if (ret)
    {
        ctc_cli_out("%% %s \n\r", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

int32
ctc_stress_test_l3_cli_init(void)
{
    g_ctc_stress_l3 = (ctc_cli_stress_test_l3_t*)sal_malloc(sizeof(ctc_cli_stress_test_l3_t));
    if(!g_ctc_stress_l3)
    {
        return CLI_ERROR;
    }
    sal_memset(g_ctc_stress_l3, 0, sizeof(ctc_cli_stress_test_l3_t));

    install_element(CTC_STRESS_MODE, &ctc_cli_stress_ipmc_group_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_ipmc_group_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_ipmc_member_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_ipuc_add_tunnel_ipv4_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_ipuc_add_tunnel_ipv6_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_ipuc_add_nat_sa_ipv4_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_ipuc_remove_tunnel_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_ipuc_remove_nat_sa_ipv4_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_ipuc_ipv4_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_ipuc_ipv6_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_ipuc_remove_cmd);
    return CLI_SUCCESS;
}


