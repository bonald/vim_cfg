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

#define CTC_CLI_MPLS_COMM_OPT \
    "stats STATS_ID |trust-outer-exp |qos-use-outer-info | acl-use-outer-info | flex-edit |"
#define CTC_CLI_MPLS_POLICER_STR \
    "Set policer type after setting id,default type is flow policer"
#define CTC_CLI_MPLS_COMM_STR \
    "Statistics",\
    "1~0xFFFFFFFF",\
    "Use outer label's exp",\
    "Use the QOS information from outer header",\
    "Use the ACL information from outer header",\
    "edit label directly, do not need to edit outer eth header"

#define CTC_CLI_STRESS_TEST_TUNNEL_MPLS_MAX_ENTRY 4096
#define SYS_MPLS_MAX_LABEL              0XFFFFF
#define SYS_MPLS_MAX_LABEL_SPACE   0xFF

#define CTC_CLI_STRESS_TEST_TUNNEL_MPLS_PARAM_CHECK(label, labelspace) \
    do\
    {\
        if (SYS_MPLS_MAX_LABEL < label)\
        {\
            label = 0;\
        }\
        if (SYS_MPLS_MAX_LABEL_SPACE < labelspace)\
        {\
            labelspace = 0;\
        }\
    }while(0)

struct ctc_cli_stress_test_tunnel_s
{
    uint32 overlay_entry_num;
    uint32 mpls_entry_num;
    ctc_overlay_tunnel_param_t* p_tunnel_param;
    ctc_mpls_ilm_t* p_mpls_param;
};
typedef struct ctc_cli_stress_test_tunnel_s ctc_cli_stress_test_tunnel_t;

struct ctc_cli_stress_test_tunnel_mpls_s
{
    uint8 add_spaceid;
    uint8 space_base;
    uint8 rsv[2];
    uint32 label_base;
    uint32 entry_num;
};
typedef struct ctc_cli_stress_test_tunnel_mpls_s ctc_cli_stress_test_tunnel_mpls_t;

ctc_cli_stress_test_tunnel_mpls_t g_ctc_stress_mpls;
ctc_cli_stress_test_tunnel_t* g_ctc_stress_tunnel;

#define   ________OVERLAY_STRESS________

STATIC int32
_ctc_cli_stress_overlay_tunnel(ctc_overlay_tunnel_param_t tunnel_param, ctc_cli_stress_t stress_param, uint32 entry_num)
{
    int32 ret = CLI_SUCCESS;
    uint16  loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1,tv2;
    char str_buf[32] = {0};
    ctc_cli_stress_output_t output;

    if(CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type && g_ctc_stress_tunnel->p_tunnel_param == NULL)
    {
        ctc_cli_out("Must add entry first\n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress_param.test_type)
    {
        sal_gettime(&tv1);
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress_param.op_type)
    {
        if (NULL == g_ctc_stress_tunnel->p_tunnel_param)
        {
            g_ctc_stress_tunnel->p_tunnel_param = (ctc_overlay_tunnel_param_t*)sal_malloc(sizeof(ctc_overlay_tunnel_param_t) * entry_num);
            if (NULL == g_ctc_stress_tunnel->p_tunnel_param)
            {
                ctc_cli_out("Memory alloc error\n");
                return CLI_ERROR;
            }
            sal_memset(g_ctc_stress_tunnel->p_tunnel_param, 0, sizeof(ctc_overlay_tunnel_param_t) * entry_num);
            g_ctc_stress_tunnel->overlay_entry_num = entry_num;
        }
    }

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
    {
        entry_num = g_ctc_stress_tunnel->overlay_entry_num;
    }

    if(entry_num == 0)
    {
        ctc_cli_out("Invalid entry num\n");
        return CLI_ERROR;
    }

    for (loop = 0; loop <= entry_num - 1; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress_param.op_type)
        {
            if (CTC_CLI_STRESS_DATA_MODE_INCREASE == stress_param.data_mode)
            {
                /* key increase and action fix */
                if (loop != 0)
                {
                    tunnel_param.src_vn_id = (tunnel_param.src_vn_id + 1) % (0xFFFFFF);
                    if (CTC_FLAG_ISSET(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_USE_IPSA))
                    {
                        tunnel_param.ipsa.ipv4 = tunnel_param.ipsa.ipv4 + 1;
                    }
                }
            }
            else if(CTC_CLI_STRESS_DATA_MODE_RANDOM == stress_param.data_mode)
            {
                /* key random and action fix */
                tunnel_param.src_vn_id = sal_rand() % (0xFFFFFF);
                if (CTC_FLAG_ISSET(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_USE_IPSA))
                {
                    tunnel_param.ipsa.ipv4 = sal_rand();
                }
            }
            else if(CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress_param.data_mode)
            {
                /* key random and action random */
                tunnel_param.src_vn_id = sal_rand() % (0xFFFFFF);
                 /*tunnel_param.ipda.ipv4 = sal_rand();*/
                if (CTC_FLAG_ISSET(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_USE_IPSA))
                {
                    tunnel_param.ipsa.ipv4 = sal_rand();
                }

                if(CTC_FLAG_ISSET(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_OUTPUT_NHID))
                {
                    tunnel_param.action.nh_id = sal_rand();
                }
                else
                {
                    tunnel_param.action.dst_vn_id = sal_rand() % (0xFFFFFF);
                }

                if(CTC_FLAG_ISSET(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_STATS_EN))
                {
                    tunnel_param.stats_id = sal_rand() % (0xFFFF);
                }

                if(CTC_FLAG_ISSET(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_SERVICE_POLICER_EN))
                {
                    tunnel_param.policer_id = sal_rand() % (0xFFFF);
                }

                if(CTC_FLAG_ISSET(tunnel_param.flag, CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_TCAM_EN))
                {
                    tunnel_param.acl_tcam_lookup_type = sal_rand() % (CTC_ACL_TCAM_LKUP_TYPE_MAX);
                    tunnel_param.acl_tcam_label = sal_rand() % (256);
                }
            }

            /* add tunnel */
            sal_memcpy(g_ctc_stress_tunnel->p_tunnel_param + loop, &tunnel_param, sizeof(ctc_overlay_tunnel_param_t));

            if (g_ctcs_api_en)
            {
                ret = ctcs_overlay_tunnel_add_tunnel(g_api_lchip, &tunnel_param);
            }
            else
            {
                ret = ctc_overlay_tunnel_add_tunnel(&tunnel_param);
            }

            if (ret == CTC_E_NONE)
            {
                ctc_cli_out("Add %d normal\n", loop + 1);
            }
            else
            {
                ctc_cli_out("Add %d error: %s\n", loop + 1, ctc_get_error_desc(ret));
            }

        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
        {
            sal_memcpy(&tunnel_param, g_ctc_stress_tunnel->p_tunnel_param + loop, sizeof(ctc_overlay_tunnel_param_t));

            if(g_ctcs_api_en)
            {
                ret = ctcs_overlay_tunnel_remove_tunnel(g_api_lchip, &tunnel_param);
            }
            else
            {
                ret = ctc_overlay_tunnel_remove_tunnel(&tunnel_param);
            }

            if (ret == CTC_E_NONE)
            {
                ctc_cli_out("Remove %d normal\n", loop + 1);
            }
            else
            {
                ctc_cli_out("Remove %d error: %s\n", loop + 1, ctc_get_error_desc(ret));
            }
        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress_param.test_type, valid_count);
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress_param.test_type)
    {
        sal_gettime(&tv2);

        sal_sprintf(str_buf, "Entry: %5d\n", entry_num);
        output.buf = str_buf;
        output.op = stress_param.op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("Operate valid count: %d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
    {
        sal_free(g_ctc_stress_tunnel->p_tunnel_param);
        g_ctc_stress_tunnel->p_tunnel_param = NULL;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stress_overlay_tunnel_add,
        ctc_cli_stress_overlay_tunnel_add_cmd,
        "overlay-tunnel (nvgre | vxlan | vxlan-gpe | geneve) add src-vnid VNID ipda A.B.C.D (ipsa A.B.C.D | ) (dst-vnid VNID | nhid NHID) {stats STATS_ID | policer-id POLICER_ID | acl-tcam-en tcam-type TCAM_TYPE tcam-label TCAM_LABEL | } (random | incr | all-random) entry-num NUMBER (spec | stress | perf)",
        "Overlay-tunnel module",
        "This is a NvGRE type tunnel",
        "This is a VxLAN type tunnel",
        "This is a VxLAN-GPE type tunnel",
        "This is a GENEVE type tunnel",
        "Add overlay tunnel terminate entry",
        "Overlay tunnel virtual subnet ID",
        "Virtual subnet ID range",
        "Destination ip address",
        "Destination ip address",
        "Source ip address",
        "Source ip address",
        "Destination overlay tunnel virtual subnet ID",
        "Virtual subnet ID range",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_NH_ID_STR,
        "Statistic",
        CTC_CLI_STATS_ID_VAL,
        "Policer Id",
        "<1-0xFFFF>",
        "Set ACL Tcam lookup property",
        "Tcam type",
        "TYPE",
        "Tcam label",
        "<0-0xFF>",
        "Key value is random, action is fixed",
        "Key value is increase, action is fixed",
        "Key value is random, action is random",
        "Entry num",
        "Number",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CTC_E_NONE;
    uint32 index = 0xFF;
    uint32 entry_num = 0;
    uint32 temp = 0;
    ctc_overlay_tunnel_param_t tunnel_param;
    ctc_cli_stress_t stress_param;

    sal_memset(&tunnel_param, 0, sizeof(ctc_overlay_tunnel_param_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    index = CTC_CLI_GET_ARGC_INDEX("nvgre");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_NVGRE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vxlan");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_VXLAN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vxlan-gpe");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_VXLAN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("geneve");
    if (0xFF != index)
    {
        tunnel_param.type = CTC_OVERLAY_TUNNEL_TYPE_GENEVE;
    }

    CTC_CLI_GET_UINT32("src-vnid", tunnel_param.src_vn_id, argv[1]);
    CTC_CLI_GET_IPV4_ADDRESS("ipda", tunnel_param.ipda.ipv4, argv[2]);

    index = CTC_CLI_GET_ARGC_INDEX("ipsa");
    if (0xFF != index)
    {
        CTC_CLI_GET_IPV4_ADDRESS("ipsa", tunnel_param.ipsa.ipv4, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_USE_IPSA;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dst-vnid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("dst-vnid", tunnel_param.action.dst_vn_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("nhid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("nhid", tunnel_param.action.nh_id, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_OUTPUT_NHID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stats");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("stats-id", tunnel_param.stats_id, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_STATS_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX("policer-id");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("policer-id", tunnel_param.policer_id, argv[index + 1]);
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_SERVICE_POLICER_EN;
    }

    index = CTC_CLI_GET_ARGC_INDEX_ENHANCE("acl-tcam-en");
    if (index != 0xFF)
    {
        tunnel_param.flag |= CTC_OVERLAY_TUNNEL_FLAG_ACL_FLOW_TCAM_EN;
        index = CTC_CLI_GET_ARGC_INDEX("tcam-type");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("tcam-type", tunnel_param.acl_tcam_lookup_type, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }
        index = CTC_CLI_GET_ARGC_INDEX("tcam-label");
        if (0xFF != index)
        {
            CTC_CLI_GET_UINT8_RANGE("tcam-label", tunnel_param.acl_tcam_label, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("random");
    if (0xFF != index)
    {
        stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM;
        temp = index;
    }

    index = CTC_CLI_GET_ARGC_INDEX("incr");
    if (0xFF != index)
    {
        stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_INCREASE;
        temp = index;
    }

    index = CTC_CLI_GET_ARGC_INDEX("all-random");
    if (0xFF != index)
    {
        stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL;
        temp = index;
    }

    CTC_CLI_GET_UINT32("entry-num", entry_num, argv[temp + 1]);

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != index)
    {
        stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != index)
    {
        stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;

    ret = _ctc_cli_stress_overlay_tunnel(tunnel_param, stress_param, entry_num);

    return ret;
}

CTC_CLI(ctc_cli_stress_overlay_tunnel_remove,
        ctc_cli_stress_overlay_tunnel_remove_cmd,
        "overlay-tunnel remove",
        "Overlay-tunnel module",
        "Remove overlay tunnel terminate entry")
{
    int32 ret = CTC_E_NONE;
    uint32 entry_num = 0;
    ctc_overlay_tunnel_param_t tunnel_param;
    ctc_cli_stress_t stress_param;

    sal_memset(&tunnel_param, 0, sizeof(ctc_overlay_tunnel_param_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_MAX;
    stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_MAX;

    ret = _ctc_cli_stress_overlay_tunnel(tunnel_param, stress_param, entry_num);

    return ret;
}

int32
_ctc_stress_test_tunnel_mpls(ctc_cli_stress_t stress_param, ctc_mpls_ilm_t* p_mpls_ilm, ctc_cli_stress_test_tunnel_mpls_t mpls_test)
{
    uint32 loop = 0;
    uint32 label_temp = mpls_test.label_base;
    uint32 valid_count =0;
    int32 ret = CTC_E_NONE;
    uint16 space_temp = mpls_test.space_base;
    char str_buf[32] = {0};
    sal_systime_t tv1,tv2;
    ctc_cli_stress_output_t output;

    if(CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type && g_ctc_stress_tunnel->p_mpls_param == NULL)
    {
        ctc_cli_out("Must add entry first\n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress_param.test_type)
    {
        sal_gettime(&tv1);
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress_param.op_type)
    {
        if (NULL == g_ctc_stress_tunnel->p_mpls_param)
        {
            g_ctc_stress_tunnel->p_mpls_param = (ctc_mpls_ilm_t*)sal_malloc(sizeof(ctc_mpls_ilm_t) * mpls_test.entry_num);
            if (NULL == g_ctc_stress_tunnel->p_mpls_param)
            {
                ctc_cli_out("Memory alloc error\n");
                return CLI_ERROR;
            }
            sal_memset(g_ctc_stress_tunnel->p_mpls_param, 0, sizeof(ctc_mpls_ilm_t) * mpls_test.entry_num);
        }
        g_ctc_stress_tunnel->mpls_entry_num = mpls_test.entry_num;
    }

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
    {
        mpls_test.entry_num = g_ctc_stress_tunnel->mpls_entry_num;
    }

    if(0 == mpls_test.entry_num)
    {
        ctc_cli_out("Invalid entry num\n");
        return CLI_ERROR;
    }

    for (loop = 0; loop <= mpls_test.entry_num - 1; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress_param.op_type)
        {
            if (CTC_CLI_STRESS_DATA_MODE_INCREASE == stress_param.data_mode)
            {
                label_temp ++;
                space_temp = (mpls_test.add_spaceid)?(space_temp + 1):space_temp;
                CTC_CLI_STRESS_TEST_TUNNEL_MPLS_PARAM_CHECK(label_temp, space_temp);
            }
            else if(CTC_CLI_STRESS_DATA_MODE_RANDOM == stress_param.data_mode)
            {
                label_temp = sal_rand() % (0xFFFFF);
                space_temp = sal_rand() % (0xFF);
            }
            else if(CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress_param.data_mode)
            {
                label_temp = sal_rand() % (0xFFFFF);
                space_temp = sal_rand() % (0xFF);
                p_mpls_ilm->pwid = sal_rand() % (0xFFF);
            }
            p_mpls_ilm->label = label_temp;
            p_mpls_ilm->spaceid = space_temp;
            if (((p_mpls_ilm)->type != CTC_MPLS_LABEL_TYPE_VPLS)
                && ((p_mpls_ilm)->type != CTC_MPLS_LABEL_TYPE_GATEWAY)
                && ((p_mpls_ilm)->type != CTC_MPLS_LABEL_TYPE_VPWS))
            {
                p_mpls_ilm->pwid = 0;
            }

            /* add tunnel */
            sal_memcpy(g_ctc_stress_tunnel->p_mpls_param + loop, p_mpls_ilm, sizeof(ctc_mpls_ilm_t));
            if (g_ctcs_api_en)
            {
                ret = ctcs_mpls_add_ilm(g_api_lchip, p_mpls_ilm);
            }
            else
            {
                ret = ctc_mpls_add_ilm(p_mpls_ilm);
            }

            if (ret == CTC_E_NONE)
            {
                ctc_cli_out("Add %d normal\n", loop + 1);
            }
            else
            {
                ctc_cli_out("Add %d error: %s\n", loop + 1, ctc_get_error_desc(ret));
            }
        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
        {
            sal_memcpy(p_mpls_ilm, g_ctc_stress_tunnel->p_mpls_param + loop, sizeof(ctc_mpls_ilm_t));
            if(g_ctcs_api_en)
            {
                ret = ctcs_mpls_del_ilm(g_api_lchip, p_mpls_ilm);
            }
            else
            {
                ret = ctc_mpls_del_ilm(p_mpls_ilm);
            }
            if (ret == CTC_E_NONE)
            {
                ctc_cli_out("Remove %d normal\n", loop + 1);
            }
            else
            {
                ctc_cli_out("Remove %d error: %s\n", loop + 1, ctc_get_error_desc(ret));
            }
        }

        if(ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress_param.test_type, valid_count);
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress_param.test_type)
    {
        sal_gettime(&tv2);

        sal_sprintf(str_buf, "Entry: %5d", mpls_test.entry_num);
        output.buf = str_buf;
        output.op = stress_param.op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("Operate valid count: %d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
    {
        sal_free(g_ctc_stress_tunnel->p_tunnel_param);
        g_ctc_stress_tunnel->p_tunnel_param = NULL;
    }
    return ret;
}


CTC_CLI(ctc_cli_stress_mpls_add_normal_ilm_tcam,
        ctc_cli_stress_mpls_add_normal_ilm_tcam_cmd,
        "mpls ilm add {nexthop-id NHID  \
        | (uniform | short-pipe | pipe) | (pop (decap |) (interface-label-space SPACEID|))\
        | oam | aps-select GROUP_ID (working-path|protection-path)\
        | trust-outer-exp | (ipv6 | ip) | control-word \
        | policer-id POLICERID (service-policer|) | is-bos | is-tcam |"CTC_CLI_MPLS_COMM_OPT" \
        |(entry-num NUMBER) | add-space |(label-base LABELBASE)| (space-base SPACEBASE) |random | incr | all-random | (spec | stress | perform)}",
        CTC_CLI_MPLS_M_STR,
        "Incoming label mapping",
        "Add ilm",
        "Nexthop id",
        CTC_CLI_NH_ID_STR,
        "Uniform model",
        "Short pipe model",
        "Pipe model",
        "Pop operation label",
        "Decap Inner Packet",
        "Output mpls interface label space",
        "Interface label space id, <1-255>, 0 means invalid",
        "OAM Enable",
        "APS select ",
        "GROUP_ID",
        "Indicate working path",
        "Indicate protection path",
        "Use outer label's exp",
        "Inner packet type is ipv6",
        "Inner packet type is ipv4 or ipv6",
        "Control word used",
        "Set Policer ID",
        "Policer ID 1 - 16383",
        CTC_CLI_MPLS_POLICER_STR,
        "label is VC label",
        "Is tcam",
        CTC_CLI_MPLS_COMM_STR,
        "how many entry will be set",
        "entry num",
        "add space",
        "label base",
        "label base",
        "space base",
        "space base",
        "random",
        "increase",
        "random all",
        "spec test",
        "stress test",
        "perform test")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    ctc_mpls_ilm_t ilm_info;
    bool b_nh = FALSE;
    ctc_cli_stress_test_tunnel_mpls_t mpls_test_temp;
    ctc_cli_stress_t stress_param;

    sal_memset(&ilm_info, 0, sizeof(ctc_mpls_ilm_t));
    sal_memset(&mpls_test_temp, 0, sizeof(ctc_cli_stress_test_tunnel_mpls_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    index = CTC_CLI_GET_ARGC_INDEX("nexthop-id");
    if (0xFF != index)
    {
        b_nh = TRUE;
        CTC_CLI_GET_UINT32("nexthop-id", ilm_info.nh_id, argv[index + 1]);
    }

    /* default is uniform mode!!! */
    ilm_info.model = CTC_MPLS_TUNNEL_MODE_UNIFORM;

    index = CTC_CLI_GET_ARGC_INDEX("pipe");
    if (0xFF != index)
    {
        ilm_info.model = CTC_MPLS_TUNNEL_MODE_PIPE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("short-pipe");
    if (0xFF != index)
    {
        ilm_info.model = CTC_MPLS_TUNNEL_MODE_SHORT_PIPE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("pop");
    if (0xFF != index)
    {
        ilm_info.pop = TRUE;
    }
    else if (FALSE == b_nh)
    {
        ctc_cli_out("%% %s \n\r", "Only pop mode don't need to config nexthop!");
        return CLI_ERROR;
    }

    index = CTC_CLI_GET_ARGC_INDEX("decap");
    if (0xFF != index)
    {
        ilm_info.decap = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("interface-label-space");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("interface-label-space", ilm_info.out_intf_spaceid , argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("oam");
    if (0xFF != index)
    {
        ilm_info.oam_en = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("aps-select");
    if (0xFF != index)
    {
        ilm_info.id_type |= CTC_MPLS_ID_APS_SELECT;
        CTC_CLI_GET_UINT16_RANGE("aps-select ", ilm_info.flw_vrf_srv_aps.aps_select_grp_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        CTC_CLI_GET_UINT16_RANGE("aps-select ", ilm_info.aps_select_grp_id, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);

        if (CLI_CLI_STR_EQUAL("protection-path", index + 2))
        {
            ilm_info.aps_select_protect_path = 1;
        }
        else
        {
            ilm_info.aps_select_protect_path = 0;
        }
    }

    index = CTC_CLI_GET_ARGC_INDEX("stats");
    if (0xFF != index)
    {
        ilm_info.id_type |= CTC_MPLS_ID_STATS;
        CTC_CLI_GET_UINT32("stats", ilm_info.stats_id, argv[index + 1]);
    }

    ilm_info.inner_pkt_type = CTC_MPLS_INNER_IPV4;
    index = CTC_CLI_GET_ARGC_INDEX("ipv6");
    if (0xFF != index)
    {
        ilm_info.inner_pkt_type = CTC_MPLS_INNER_IPV6;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip");
    if (0xFF != index)
    {
        ilm_info.inner_pkt_type = CTC_MPLS_INNER_IP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("trust-outer-exp");
    if (0xFF != index)
    {
        ilm_info.trust_outer_exp = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-use-outer-info");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_ACL_USE_OUTER_INFO);
    }

    /* qos use outer info */
    index  = CTC_CLI_GET_ARGC_INDEX("qos-use-outer-info");
    if (0xFF != index)
    {
        ilm_info.qos_use_outer_info = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("control-word");
    if (0xFF != index)
    {
        ilm_info.cwen = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("policer-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16("policer-id", ilm_info.policer_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("service-policer");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_SERVICE_POLICER_EN);
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-bos");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_BOS_LABLE );
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-tcam");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_USE_FLEX );
    }

    index = CTC_CLI_GET_ARGC_INDEX("flex-edit");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_FLEX_EDIT );
    }

    ilm_info.type = CTC_MPLS_LABEL_TYPE_NORMAL;

    index = CTC_CLI_GET_ARGC_INDEX("add");
    if (0xFF != index)
    {
        stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("entry-num");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("entry-num", mpls_test_temp.entry_num, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("add-space");
    if (0xFF != index)
    {
        mpls_test_temp.add_spaceid = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("label-base");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("label-base", mpls_test_temp.label_base, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("space-base");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("space-base", mpls_test_temp.space_base, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("random");
    if (0xFF != index)
    {
        stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM;
    }

    index = CTC_CLI_GET_ARGC_INDEX("incr");
    if (0xFF != index)
    {
        stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_INCREASE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("all-random");
    if (0xFF != index)
    {
        stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != index)
    {
        stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != index)
    {
        stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("perform");
    if (0xFF != index)
    {
        stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_stress_test_tunnel_mpls(stress_param, &ilm_info, mpls_test_temp);
    return ret;
}

CTC_CLI(ctc_cli_stress_mpls_remove_normal_ilm_tcam,
        ctc_cli_stress_mpls_remove_normal_ilm_tcam_cmd,
        "mpls ilm remove",
        CTC_CLI_MPLS_M_STR,
        "Incoming label mapping",
        "Remove ilm(inluding l3vpn-vc, vpls-vc and vpws-vc)")
{
    int32 ret = CLI_SUCCESS;
    ctc_mpls_ilm_t ilm_info;
    ctc_cli_stress_test_tunnel_mpls_t mpls_test_temp;
    ctc_cli_stress_t stress_param;

    sal_memset(&ilm_info, 0, sizeof(ctc_mpls_ilm_t));
    sal_memset(&mpls_test_temp, 0, sizeof(ctc_cli_stress_test_tunnel_mpls_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_MAX;
    stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_MAX;

    ret = _ctc_stress_test_tunnel_mpls(stress_param, &ilm_info, mpls_test_temp);
    return ret;
}

CTC_CLI(ctc_cli_stress_mpls_add_l3vpn_ilm_tcam,
        ctc_cli_stress_mpls_add_l3vpn_ilm_tcam_cmd,
        "mpls l3vpn-vc add  NHID {(uniform | short-pipe | pipe)| \
        (ipv6 | ip) | oam |"CTC_CLI_MPLS_COMM_OPT" | is-tcam | (entry-num NUMBER) | (label-base LABELBASE)| (space-base SPACEBASE) | add-space | random | incr | all-random | (spec | stress | perform) }",
        CTC_CLI_MPLS_M_STR,
        "L3vpn vc label",
        "Add ilm",
        CTC_CLI_NH_ID_STR,
        "Uniform model",
        "Short pipe model",
        "Pipe model",
        "Inner packet type is ipv6",
        "Inner packet type is ipv4 or ipv6",
        "OAM Enable",
        CTC_CLI_MPLS_COMM_STR,
        "Is tcam",
        "how many entry will be set",
        "entry num",
        "add space",
        "label base",
        "label base",
        "space base",
        "space base",
        "random",
        "increase",
        "random all",
        "spec test",
        "stress test",
        "perform test")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    ctc_mpls_ilm_t ilm_info;
    ctc_cli_stress_t stress_param;
    ctc_cli_stress_test_tunnel_mpls_t mpls_test_temp;

    sal_memset(&ilm_info, 0, sizeof(ctc_mpls_ilm_t));
    sal_memset(&mpls_test_temp, 0, sizeof(ctc_cli_stress_test_tunnel_mpls_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));
    stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;

    CTC_CLI_GET_UINT32("nexthop", ilm_info.nh_id, argv[0]);
    ilm_info.id_type |= CTC_MPLS_ID_VRF;
    ilm_info.model = CTC_MPLS_TUNNEL_MODE_UNIFORM;

    index = CTC_CLI_GET_ARGC_INDEX("uniform");
    if (0xFF != index)
    {
        ilm_info.model = CTC_MPLS_TUNNEL_MODE_UNIFORM;
    }
    else if (0xFF != CTC_CLI_GET_ARGC_INDEX("short-pipe"))
    {
        ilm_info.model = CTC_MPLS_TUNNEL_MODE_SHORT_PIPE;
    }
    else
    {
        ilm_info.model = CTC_MPLS_TUNNEL_MODE_PIPE;
    }

    /* qos use outer info */
    index  = CTC_CLI_GET_ARGC_INDEX("qos-use-outer-info");
    if (0xFF != index)
    {
        ilm_info.qos_use_outer_info = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("acl-use-outer-info");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_ACL_USE_OUTER_INFO);
    }

    ilm_info.inner_pkt_type = CTC_MPLS_INNER_IPV4;
    index = CTC_CLI_GET_ARGC_INDEX("ipv6");
    if (0xFF != index)
    {
        ilm_info.inner_pkt_type = CTC_MPLS_INNER_IPV6;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ip");
    if (0xFF != index)
    {
        ilm_info.inner_pkt_type = CTC_MPLS_INNER_IP;
    }

    index = CTC_CLI_GET_ARGC_INDEX("trust-outer-exp");
    if (0xFF != index)
    {
        ilm_info.trust_outer_exp = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("oam");
    if (0xFF != index)
    {
        ilm_info.oam_en = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stats");
    if (0xFF != index)
    {
        ilm_info.id_type |= CTC_MPLS_ID_STATS;
        CTC_CLI_GET_UINT32("stats", ilm_info.stats_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("is-tcam");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_USE_FLEX );
    }

    index = CTC_CLI_GET_ARGC_INDEX("flex-edit");
    if (0xFF != index)
    {
        CTC_SET_FLAG(ilm_info.flag, CTC_MPLS_ILM_FLAG_FLEX_EDIT );
    }

    ilm_info.type = CTC_MPLS_LABEL_TYPE_L3VPN;
    ilm_info.pwid = 0;

    index = CTC_CLI_GET_ARGC_INDEX("entry-num");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("entry-num", mpls_test_temp.entry_num, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("add-space");
    if (0xFF != index)
    {
        mpls_test_temp.add_spaceid = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("label-base");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("label-base", mpls_test_temp.label_base, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("space-base");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("space-base", mpls_test_temp.space_base, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("random");
    if (0xFF != index)
    {
        stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM;
    }

    index = CTC_CLI_GET_ARGC_INDEX("incr");
    if (0xFF != index)
    {
        stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_INCREASE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("all-random");
    if (0xFF != index)
    {
        stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("spec");
    if (0xFF != index)
    {
        stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stress");
    if (0xFF != index)
    {
        stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("perform");
    if (0xFF != index)
    {
        stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_stress_test_tunnel_mpls(stress_param, &ilm_info, mpls_test_temp);

    return ret;
}

int32
ctc_stress_test_tunnel_cli_init(void)
{
    g_ctc_stress_tunnel = (ctc_cli_stress_test_tunnel_t*)sal_malloc(sizeof(ctc_cli_stress_test_tunnel_t));

    if (!g_ctc_stress_tunnel)
    {
        return CLI_ERROR;
    }

    sal_memset(g_ctc_stress_tunnel, 0, sizeof(ctc_cli_stress_test_tunnel_t));

    install_element(CTC_STRESS_MODE, &ctc_cli_stress_overlay_tunnel_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_overlay_tunnel_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_mpls_add_normal_ilm_tcam_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_mpls_remove_normal_ilm_tcam_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_mpls_add_l3vpn_ilm_tcam_cmd);

    return CLI_SUCCESS;
}


