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
enum ctc_cli_stress_test_l2_type_e
{
    CTC_CLI_STRESS_L2_TYPE_VLAN_MAPING,
    CTC_CLI_STRESS_L2_TYPE_VLAN_MAPING_EGRESS,
    CTC_CLI_STRESS_L2_TYPE_VLAN_CLASSIFIER,
    CTC_CLI_STRESS_L2_TYPE_FDB,
    CTC_CLI_STRESS_L2_TYPE_MCAST,
    CTC_CLI_STRESS_L2_TYPE_MCAST_MEMBER,
    CTC_CLI_STRESS_L2_TYPE_MAX
};

struct ctc_cli_stress_test_l2_s
{
    uint16 gport;
    uint32 entry_num[CTC_CLI_STRESS_L2_TYPE_MAX];
    uint8 step[CTC_CLI_STRESS_L2_TYPE_MAX];
    ctc_l2_addr_t  *fdb;
    ctc_l2_mcast_addr_t *l2mc_addr;
    ctc_vlan_mapping_t *p_vlan_mapping;
    ctc_egress_vlan_mapping_t *p_egress_vlan_mapping;
    ctc_vlan_class_t *p_vlan_calss;
};
typedef struct ctc_cli_stress_test_l2_s ctc_cli_stress_test_l2_t;

ctc_cli_stress_test_l2_t  *g_ctc_stress_l2;

#define __VLAN_MAPPING__
#define VLAN_MAPPING_KEY_FLAG (CTC_VLAN_MAPPING_KEY_CVID | CTC_VLAN_MAPPING_KEY_SVID | CTC_VLAN_MAPPING_KEY_CTAG_COS | CTC_VLAN_MAPPING_KEY_STAG_COS | CTC_VLAN_MAPPING_KEY_MAC_SA | CTC_VLAN_MAPPING_KEY_IPV4_SA | CTC_VLAN_MAPPING_KEY_IPV6_SA)
#define EGRESS_VLAN_MAPPING_KEY_FLAG (CTC_EGRESS_VLAN_MAPPING_KEY_CVID | CTC_EGRESS_VLAN_MAPPING_KEY_SVID | CTC_EGRESS_VLAN_MAPPING_KEY_CTAG_COS | CTC_EGRESS_VLAN_MAPPING_KEY_STAG_COS)

STATIC int32
_ctc_stress_l2_cli_parse_common(char** argv, uint16 argc, ctc_cli_stress_t* stress, uint8 l2_type)
{
    uint8 tmp_argi = 0;

    if(g_ctc_stress_l2->entry_num[l2_type])
    {
     ctc_cli_out("Other stress test have used, please remove first! \n");
     return CLI_ERROR;
    }
    sal_memset(&(g_ctc_stress_l2->step[l2_type]), 1, sizeof(g_ctc_stress_l2->step[l2_type]));

    tmp_argi = CTC_CLI_GET_ARGC_INDEX_ENHANCE("entry-num");
    if (tmp_argi != 0xFF)
    {
        CTC_CLI_GET_UINT32_RANGE("entry-num", g_ctc_stress_l2->entry_num[l2_type], argv[tmp_argi + 1], 0, CTC_MAX_UINT32_VALUE);
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
        tmp_argi = CTC_CLI_GET_ARGC_INDEX("step");
        if (0xFF != tmp_argi)
        {
            CTC_CLI_GET_UINT8_RANGE("step", g_ctc_stress_l2->step[l2_type], argv[tmp_argi + 1], 0, CTC_MAX_UINT8_VALUE);
        }
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


STATIC int32
_ctc_cli_stress_vlan_mapping(uint16 gport, ctc_vlan_mapping_t vlan_mapping, ctc_cli_stress_t stress_param, uint32 entry_num, uint8 step)
{
    int32 ret = CLI_SUCCESS;
    uint16  loop = 0;
    uint32 valid_count = 0;
    sal_systime_t tv1,tv2;
    char str_buf[32] = {0};
    ctc_cli_stress_output_t output;

    if(CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type && g_ctc_stress_l2->p_vlan_mapping == NULL)
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
        if (NULL == g_ctc_stress_l2->p_vlan_mapping)
        {
            g_ctc_stress_l2->p_vlan_mapping = (ctc_vlan_mapping_t*)sal_malloc(sizeof(ctc_vlan_mapping_t) * entry_num);
            if (NULL == g_ctc_stress_l2->p_vlan_mapping)
            {
                ctc_cli_out("Memory alloc error\n");
                return CLI_ERROR;
            }
            sal_memset(g_ctc_stress_l2->p_vlan_mapping, 0, sizeof(ctc_vlan_mapping_t) * entry_num);
            g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_VLAN_MAPING] = entry_num;
        }
    }

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
    {
        entry_num = g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_VLAN_MAPING];
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
                    switch ((vlan_mapping.key) & (VLAN_MAPPING_KEY_FLAG))
                    {
                        case (CTC_VLAN_MAPPING_KEY_CVID):
                            vlan_mapping.old_cvid = (vlan_mapping.old_cvid + step) % 4096;
                            break;
                        case (CTC_VLAN_MAPPING_KEY_SVID):
                            vlan_mapping.old_svid = (vlan_mapping.old_svid + step) % 4096;
                            break;
                        case (CTC_VLAN_MAPPING_KEY_CVID | CTC_VLAN_MAPPING_KEY_SVID):
                            vlan_mapping.old_svid = (vlan_mapping.old_svid + step) % 4096;
                            vlan_mapping.old_cvid = (vlan_mapping.old_cvid + step) % 4096;
                            break;
                        case (CTC_VLAN_MAPPING_KEY_CVID | CTC_VLAN_MAPPING_KEY_CTAG_COS):
                            vlan_mapping.old_cvid = (vlan_mapping.old_cvid + step) % 4096;
                            vlan_mapping.old_ccos = (vlan_mapping.old_ccos + step) % 8;
                            break;
                        case (CTC_VLAN_MAPPING_KEY_SVID | CTC_VLAN_MAPPING_KEY_STAG_COS):
                            vlan_mapping.old_svid = (vlan_mapping.old_svid + step) % 4096;
                            vlan_mapping.old_scos = (vlan_mapping.old_scos + step) % 8;
                            break;
                        default:
                            sal_free(g_ctc_stress_l2->p_vlan_mapping);
                            g_ctc_stress_l2->p_vlan_mapping = NULL;
                            return CTC_E_NOT_SUPPORT;
                            break;
                    }
                }
            }
            else if(CTC_CLI_STRESS_DATA_MODE_RANDOM == stress_param.data_mode)
            {
                /* key random and action fix */
                switch ((vlan_mapping.key) & (VLAN_MAPPING_KEY_FLAG))
                {
                    case (CTC_VLAN_MAPPING_KEY_CVID):
                        vlan_mapping.old_cvid = sal_rand() % 4096;
                        break;
                    case (CTC_VLAN_MAPPING_KEY_SVID):
                        vlan_mapping.old_svid = sal_rand() % 4096;
                        break;
                    case (CTC_VLAN_MAPPING_KEY_CVID | CTC_VLAN_MAPPING_KEY_SVID):
                        vlan_mapping.old_svid = sal_rand() % 4096;
                        vlan_mapping.old_cvid = sal_rand() % 4096;
                        break;
                    case (CTC_VLAN_MAPPING_KEY_CVID | CTC_VLAN_MAPPING_KEY_CTAG_COS):
                        vlan_mapping.old_cvid = sal_rand() % 4096;
                        vlan_mapping.old_ccos = sal_rand() % 8;
                        break;
                    case (CTC_VLAN_MAPPING_KEY_SVID | CTC_VLAN_MAPPING_KEY_STAG_COS):
                        vlan_mapping.old_svid = sal_rand() % 4096;
                        vlan_mapping.old_scos = sal_rand() % 8;
                        break;
                    default:
                        sal_free(g_ctc_stress_l2->p_vlan_mapping);
                        g_ctc_stress_l2->p_vlan_mapping = NULL;
                        return CTC_E_NOT_SUPPORT;
                        break;
                }
            }
            else if(CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress_param.data_mode)
            {
                /* key random and action random */
                switch ((vlan_mapping.key) & (VLAN_MAPPING_KEY_FLAG))
                {
                    case (CTC_VLAN_MAPPING_KEY_CVID):
                        vlan_mapping.old_cvid = sal_rand() % 4096;
                        break;
                    case (CTC_VLAN_MAPPING_KEY_SVID):
                        vlan_mapping.old_svid = sal_rand() % 4096;
                        break;
                    case (CTC_VLAN_MAPPING_KEY_CVID | CTC_VLAN_MAPPING_KEY_SVID):
                        vlan_mapping.old_svid = sal_rand() % 4096;
                        vlan_mapping.old_cvid = sal_rand() % 4096;
                        break;
                    case (CTC_VLAN_MAPPING_KEY_CVID | CTC_VLAN_MAPPING_KEY_CTAG_COS):
                        vlan_mapping.old_cvid = sal_rand() % 4096;
                        vlan_mapping.old_ccos = sal_rand() % 8;
                        break;
                    case (CTC_VLAN_MAPPING_KEY_SVID | CTC_VLAN_MAPPING_KEY_STAG_COS):
                        vlan_mapping.old_svid = sal_rand() % 4096;
                        vlan_mapping.old_scos = sal_rand() % 8;
                        break;
                    default:
                        sal_free(g_ctc_stress_l2->p_vlan_mapping);
                        g_ctc_stress_l2->p_vlan_mapping = NULL;
                        return CTC_E_NOT_SUPPORT;
                        break;
                }
                vlan_mapping.new_svid = sal_rand() % 4096;
                vlan_mapping.new_scos = sal_rand() % 8;
                vlan_mapping.stag_op  = sal_rand() % 6;
                vlan_mapping.svid_sl  = sal_rand() % 4;
                vlan_mapping.scos_sl  = sal_rand() % 4;
                vlan_mapping.new_cvid = sal_rand() % 4096;
                vlan_mapping.new_ccos = sal_rand() % 8;
                vlan_mapping.ctag_op  = sal_rand() % 6;
                vlan_mapping.cvid_sl  = sal_rand() % 4;
                vlan_mapping.ccos_sl  = sal_rand() % 4;

            }

            /* make sure vlan id do not equal to zeros */
            vlan_mapping.old_svid = vlan_mapping.old_svid ? vlan_mapping.old_svid : 1;
            vlan_mapping.old_cvid = vlan_mapping.old_cvid ? vlan_mapping.old_cvid : 1;
            vlan_mapping.new_svid = vlan_mapping.new_svid ? vlan_mapping.new_svid : 1;
            vlan_mapping.new_cvid = vlan_mapping.new_cvid ? vlan_mapping.new_cvid : 1;

            /* add tunnel */
            sal_memcpy(g_ctc_stress_l2->p_vlan_mapping + loop, &vlan_mapping, sizeof(ctc_vlan_mapping_t));

            if (g_ctcs_api_en)
            {
                ret = ctcs_vlan_add_vlan_mapping(g_api_lchip, gport, &vlan_mapping);
            }
            else
            {
                ret = ctc_vlan_add_vlan_mapping(gport, &vlan_mapping);
            }

            if (ret == CTC_E_NONE)
            {
                 /*ctc_cli_out("Add %d normal\n", loop + 1);*/
            }
            else
            {
                ctc_cli_out("Add %d error: %s\n", loop + 1, ctc_get_error_desc(ret));
            }

        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
        {
            sal_memcpy(&vlan_mapping, g_ctc_stress_l2->p_vlan_mapping + loop, sizeof(ctc_vlan_mapping_t));

            if(g_ctcs_api_en)
            {
                ret = ctcs_vlan_remove_vlan_mapping(g_api_lchip, gport, &vlan_mapping);
            }
            else
            {
                ret = ctc_vlan_remove_vlan_mapping(gport, &vlan_mapping);
            }

            if (ret == CTC_E_NONE)
            {
                /* ctc_cli_out("Remove %d normal\n", loop + 1);*/
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
        g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_VLAN_MAPING] = 0;
        sal_free(g_ctc_stress_l2->p_vlan_mapping);
        g_ctc_stress_l2->p_vlan_mapping = NULL;
    }

    return CLI_SUCCESS;
}

int32
_ctc_cli_stress_egress_vlan_mapping(uint16 gport, ctc_egress_vlan_mapping_t vlan_mapping, ctc_cli_stress_t stress_param, uint32 entry_num, uint8 step)
{
    int32 ret = CLI_SUCCESS;
    uint16  loop = 0;
    uint32 valid_count = 0;
    sal_systime_t tv1,tv2;
    char str_buf[32] = {0};
    ctc_cli_stress_output_t output;

    if(CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type && g_ctc_stress_l2->p_egress_vlan_mapping == NULL)
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
        if (NULL == g_ctc_stress_l2->p_egress_vlan_mapping)
        {
            g_ctc_stress_l2->p_egress_vlan_mapping = (ctc_egress_vlan_mapping_t*)sal_malloc(sizeof(ctc_egress_vlan_mapping_t) * entry_num);
            if (NULL == g_ctc_stress_l2->p_egress_vlan_mapping)
            {
                ctc_cli_out("Memory alloc error\n");
                return CLI_ERROR;
            }
            sal_memset(g_ctc_stress_l2->p_egress_vlan_mapping, 0, sizeof(ctc_egress_vlan_mapping_t) * entry_num);
            g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_VLAN_MAPING_EGRESS] = entry_num;
        }
    }

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
    {
        entry_num = g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_VLAN_MAPING_EGRESS];
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
                    switch ((vlan_mapping.key) & (EGRESS_VLAN_MAPPING_KEY_FLAG))
                    {
                        case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID):
                            vlan_mapping.old_cvid = (vlan_mapping.old_cvid + step) % 4096;
                            break;
                        case (CTC_EGRESS_VLAN_MAPPING_KEY_SVID):
                            vlan_mapping.old_svid = (vlan_mapping.old_svid + step) % 4096;
                            break;
                        case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID | CTC_EGRESS_VLAN_MAPPING_KEY_SVID):
                            vlan_mapping.old_svid = (vlan_mapping.old_svid + step) % 4096;
                            vlan_mapping.old_cvid = (vlan_mapping.old_cvid + step) % 4096;
                            break;
                        case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID | CTC_EGRESS_VLAN_MAPPING_KEY_CTAG_COS):
                            vlan_mapping.old_cvid = (vlan_mapping.old_cvid + step) % 4096;
                            vlan_mapping.old_ccos = (vlan_mapping.old_ccos + step) % 8;
                            break;
                        case (CTC_EGRESS_VLAN_MAPPING_KEY_SVID | CTC_EGRESS_VLAN_MAPPING_KEY_STAG_COS):
                            vlan_mapping.old_svid = (vlan_mapping.old_svid + step) % 4096;
                            vlan_mapping.old_scos = (vlan_mapping.old_scos + step) % 8;
                            break;
                        default:
                            sal_free(g_ctc_stress_l2->p_egress_vlan_mapping);
                            g_ctc_stress_l2->p_egress_vlan_mapping = NULL;
                            return CTC_E_NOT_SUPPORT;
                            break;
                    }
                }
            }
            else if(CTC_CLI_STRESS_DATA_MODE_RANDOM == stress_param.data_mode)
            {
                /* key random and action fix */
                switch ((vlan_mapping.key) & (EGRESS_VLAN_MAPPING_KEY_FLAG))
                {
                    case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID):
                        vlan_mapping.old_cvid = sal_rand() % 4096;
                        break;
                    case (CTC_EGRESS_VLAN_MAPPING_KEY_SVID):
                        vlan_mapping.old_svid = sal_rand() % 4096;
                        break;
                    case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID | CTC_EGRESS_VLAN_MAPPING_KEY_SVID):
                        vlan_mapping.old_svid = sal_rand() % 4096;
                        vlan_mapping.old_cvid = sal_rand() % 4096;
                        break;
                    case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID | CTC_EGRESS_VLAN_MAPPING_KEY_CTAG_COS):
                        vlan_mapping.old_cvid = sal_rand() % 4096;
                        vlan_mapping.old_ccos = sal_rand() % 8;
                        break;
                    case (CTC_EGRESS_VLAN_MAPPING_KEY_SVID | CTC_EGRESS_VLAN_MAPPING_KEY_STAG_COS):
                        vlan_mapping.old_svid = sal_rand() % 4096;
                        vlan_mapping.old_scos = sal_rand() % 8;
                        break;
                    default:
                        sal_free(g_ctc_stress_l2->p_egress_vlan_mapping);
                        g_ctc_stress_l2->p_egress_vlan_mapping = NULL;
                        return CTC_E_NOT_SUPPORT;
                        break;
                }
            }
            else if(CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress_param.data_mode)
            {
                /* key random and action random */
                switch ((vlan_mapping.key) & (EGRESS_VLAN_MAPPING_KEY_FLAG))
                {
                    case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID):
                        vlan_mapping.old_cvid = sal_rand() % 4096;
                        break;
                    case (CTC_EGRESS_VLAN_MAPPING_KEY_SVID):
                        vlan_mapping.old_svid = sal_rand() % 4096;
                        break;
                    case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID | CTC_EGRESS_VLAN_MAPPING_KEY_SVID):
                        vlan_mapping.old_svid = sal_rand() % 4096;
                        vlan_mapping.old_cvid = sal_rand() % 4096;
                        break;
                    case (CTC_EGRESS_VLAN_MAPPING_KEY_CVID | CTC_EGRESS_VLAN_MAPPING_KEY_CTAG_COS):
                        vlan_mapping.old_cvid = sal_rand() % 4096;
                        vlan_mapping.old_ccos = sal_rand() % 8;
                        break;
                    case (CTC_EGRESS_VLAN_MAPPING_KEY_SVID | CTC_EGRESS_VLAN_MAPPING_KEY_STAG_COS):
                        vlan_mapping.old_svid = sal_rand() % 4096;
                        vlan_mapping.old_scos = sal_rand() % 8;
                        break;
                    default:
                        sal_free(g_ctc_stress_l2->p_egress_vlan_mapping);
                        g_ctc_stress_l2->p_egress_vlan_mapping = NULL;
                        return CTC_E_NOT_SUPPORT;
                        break;
                }

                /* action is also random */
                vlan_mapping.new_svid = sal_rand() % 4096;
                vlan_mapping.new_scos = sal_rand() % 8;
                vlan_mapping.stag_op  = sal_rand() % 6;
                vlan_mapping.svid_sl  = sal_rand() % 4;
                vlan_mapping.scos_sl  = sal_rand() % 4;
                vlan_mapping.new_cvid = sal_rand() % 4096;
                vlan_mapping.new_ccos = sal_rand() % 8;
                vlan_mapping.ctag_op  = sal_rand() % 6;
                vlan_mapping.cvid_sl  = sal_rand() % 4;
                vlan_mapping.ccos_sl  = sal_rand() % 4;

            }

            /* make sure vlan id do not equal to zeros */
            vlan_mapping.old_svid = vlan_mapping.old_svid ? vlan_mapping.old_svid : 1;
            vlan_mapping.old_cvid = vlan_mapping.old_cvid ? vlan_mapping.old_cvid : 1;
            vlan_mapping.new_svid = vlan_mapping.new_svid ? vlan_mapping.new_svid : 1;
            vlan_mapping.new_cvid = vlan_mapping.new_cvid ? vlan_mapping.new_cvid : 1;

            /* add tunnel */
            sal_memcpy(g_ctc_stress_l2->p_egress_vlan_mapping + loop, &vlan_mapping, sizeof(ctc_egress_vlan_mapping_t));

            if (g_ctcs_api_en)
            {
                ret = ctcs_vlan_add_egress_vlan_mapping(g_api_lchip, gport, &vlan_mapping);
            }
            else
            {
                ret = ctc_vlan_add_egress_vlan_mapping(gport, &vlan_mapping);
            }

            if (ret == CTC_E_NONE)
            {
                 /*ctc_cli_out("Add %d normal\n", loop + 1);*/
            }
            else
            {
                ctc_cli_out("Add %d error: %s\n", loop + 1, ctc_get_error_desc(ret));
            }

        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
        {
            sal_memcpy(&vlan_mapping, g_ctc_stress_l2->p_egress_vlan_mapping + loop, sizeof(ctc_egress_vlan_mapping_t));

            if(g_ctcs_api_en)
            {
                ret = ctcs_vlan_remove_egress_vlan_mapping(g_api_lchip, gport, &vlan_mapping);
            }
            else
            {
                ret = ctc_vlan_remove_egress_vlan_mapping(gport, &vlan_mapping);
            }

            if (ret == CTC_E_NONE)
            {
                 /*ctc_cli_out("Remove %d normal\n", loop + 1);*/
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
        g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_VLAN_MAPING_EGRESS] = 0;
        sal_free(g_ctc_stress_l2->p_egress_vlan_mapping);
        g_ctc_stress_l2->p_egress_vlan_mapping = NULL;
    }

    return CLI_SUCCESS;
}

int32
_ctc_cli_stress_vlan_class(ctc_vlan_class_t vlan_class, ctc_cli_stress_t stress_param, uint32 entry_num, uint8 step)
{
    int32 ret = CLI_SUCCESS;
    uint16  loop = 0;
    uint32 valid_count = 0;
    uint32 mac_temp = 0;
    sal_systime_t tv1,tv2;
    char str_buf[32] = {0};
    ctc_cli_stress_output_t output;

    if(CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type && g_ctc_stress_l2->p_vlan_calss == NULL)
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
        if (NULL == g_ctc_stress_l2->p_vlan_calss)
        {
            g_ctc_stress_l2->p_vlan_calss = (ctc_vlan_class_t*)sal_malloc(sizeof(ctc_vlan_class_t) * entry_num);
            if (NULL == g_ctc_stress_l2->p_vlan_calss)
            {
                ctc_cli_out("Memory alloc error\n");
                return CLI_ERROR;
            }
            sal_memset(g_ctc_stress_l2->p_vlan_calss, 0, sizeof(ctc_vlan_class_t) * entry_num);
            g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_VLAN_CLASSIFIER] = entry_num;
        }
    }

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
    {
        entry_num = g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_VLAN_CLASSIFIER];
    }

    if(entry_num == 0)
    {
        ctc_cli_out("Invalid entry num\n");
        return CLI_ERROR;
    }

    mac_temp = (vlan_class.vlan_class.mac[2] << 24) | (vlan_class.vlan_class.mac[3] << 16) | (vlan_class.vlan_class.mac[4] << 8) | (vlan_class.vlan_class.mac[5]);

    for (loop = 0; loop <= entry_num - 1; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress_param.op_type)
        {
            if (CTC_CLI_STRESS_DATA_MODE_INCREASE == stress_param.data_mode)
            {
                /* key increase and action fix */
                if (loop != 0)
                {
                    if (CTC_VLAN_CLASS_MAC == vlan_class.type)
                    {
                        mac_temp += step;
                        vlan_class.vlan_class.mac[5] = (mac_temp & 0xFF);
                        vlan_class.vlan_class.mac[4] = ((mac_temp >> 8)  & 0xFF);
                        vlan_class.vlan_class.mac[3] = ((mac_temp >> 16) & 0xFF);
                        vlan_class.vlan_class.mac[2] = ((mac_temp >> 24) & 0xFF);
                    }
                    else if (CTC_VLAN_CLASS_IPV4 == vlan_class.type)
                    {
                        vlan_class.vlan_class.ipv4_sa = vlan_class.vlan_class.ipv4_sa + step;
                    }
                    else if (CTC_VLAN_CLASS_IPV6 == vlan_class.type)
                    {
                        vlan_class.vlan_class.ipv6_sa[0] = vlan_class.vlan_class.ipv6_sa[0] + step;
                    }
                }
            }
            else if(CTC_CLI_STRESS_DATA_MODE_RANDOM == stress_param.data_mode)
            {
                /* key random and action fix */
                if (CTC_VLAN_CLASS_MAC == vlan_class.type)
                {
                    vlan_class.vlan_class.mac[0] = sal_rand() % 256;
                    vlan_class.vlan_class.mac[1] = sal_rand() % 256;
                    vlan_class.vlan_class.mac[2] = sal_rand() % 256;
                    vlan_class.vlan_class.mac[3] = sal_rand() % 256;
                    vlan_class.vlan_class.mac[4] = sal_rand() % 256;
                    vlan_class.vlan_class.mac[5] = sal_rand() % 256;
                }
                else if (CTC_VLAN_CLASS_IPV4 == vlan_class.type)
                {
                    vlan_class.vlan_class.ipv4_sa = sal_rand();
                }
                else if (CTC_VLAN_CLASS_IPV6 == vlan_class.type)
                {
                    vlan_class.vlan_class.ipv6_sa[0] = sal_rand();
                    vlan_class.vlan_class.ipv6_sa[1] = sal_rand();
                    vlan_class.vlan_class.ipv6_sa[2] = sal_rand();
                    vlan_class.vlan_class.ipv6_sa[3] = sal_rand();
                }
            }
            else if(CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress_param.data_mode)
            {
                /* key random and action random */
                if (CTC_VLAN_CLASS_MAC == vlan_class.type)
                {
                    vlan_class.vlan_class.mac[0] = sal_rand() % 256;
                    vlan_class.vlan_class.mac[1] = sal_rand() % 256;
                    vlan_class.vlan_class.mac[2] = sal_rand() % 256;
                    vlan_class.vlan_class.mac[3] = sal_rand() % 256;
                    vlan_class.vlan_class.mac[4] = sal_rand() % 256;
                    vlan_class.vlan_class.mac[5] = sal_rand() % 256;
                }
                else if (CTC_VLAN_CLASS_IPV4 == vlan_class.type)
                {
                    vlan_class.vlan_class.ipv4_sa = sal_rand();
                }
                else if (CTC_VLAN_CLASS_IPV6 == vlan_class.type)
                {
                    vlan_class.vlan_class.ipv6_sa[0] = sal_rand();
                    vlan_class.vlan_class.ipv6_sa[1] = sal_rand();
                    vlan_class.vlan_class.ipv6_sa[2] = sal_rand();
                    vlan_class.vlan_class.ipv6_sa[3] = sal_rand();
                }

                vlan_class.vlan_id = sal_rand() % 4096;
                vlan_class.vlan_id = vlan_class.vlan_id ? vlan_class.vlan_id : 1;
            }

            /* add tunnel */
            sal_memcpy(g_ctc_stress_l2->p_vlan_calss + loop, &vlan_class, sizeof(ctc_vlan_class_t));

            if (g_ctcs_api_en)
            {
                ret = ctcs_vlan_add_vlan_class(g_api_lchip, &vlan_class);
            }
            else
            {
                ret = ctc_vlan_add_vlan_class(&vlan_class);
            }

            if (ret == CTC_E_NONE)
            {
                 /*ctc_cli_out("Add %d normal\n", loop + 1);*/
            }
            else
            {
                ctc_cli_out("Add %d error: %s\n", loop + 1, ctc_get_error_desc(ret));
            }

        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
        {
            sal_memcpy(&vlan_class, g_ctc_stress_l2->p_vlan_calss + loop, sizeof(ctc_vlan_class_t));

            if(g_ctcs_api_en)
            {
                ret = ctcs_vlan_remove_vlan_class(g_api_lchip, &vlan_class);
            }
            else
            {
                ret = ctc_vlan_remove_vlan_class(&vlan_class);
            }

            if (ret == CTC_E_NONE)
            {
                 /*ctc_cli_out("Remove %d normal\n", loop + 1);*/
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
        g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_VLAN_CLASSIFIER] = 0;
        sal_free(g_ctc_stress_l2->p_vlan_calss);
        g_ctc_stress_l2->p_vlan_calss = NULL;
    }

    return CLI_SUCCESS;
}

int32
_ctc_cli_stress_fdb(ctc_cli_stress_t* stress, ctc_l2_addr_t* l2_addr, uint8 flag)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count = 0;
    uint32 fdb_num = 0;
    sal_systime_t tv1;
    sal_systime_t tv2;
    char str_buf[32] = {0};
    ctc_cli_stress_output_t output;
    uint8 is_random = 0;
    uint8 is_random_all = 0;
    uint8 static_flag = 0;
    ctc_l2_addr_t param;
    uint32 temp1 = 0, temp2;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = { 0 };

    ctc_l2_flush_fdb_t Flush;
    sal_memset(&Flush, 0, sizeof(ctc_l2_flush_fdb_t));
    Flush.flush_type = CTC_L2_FDB_ENTRY_OP_ALL;
    Flush.flush_flag = CTC_L2_FDB_ENTRY_ALL;

    sal_memset(&param, 0, sizeof(ctc_l2_addr_t));
    is_random = (CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode ) ? 1:0;
    is_random_all = (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode ) ? 1:0;
    fdb_num = g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_FDB];

    if (g_ctc_stress_l2->fdb && (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type))
    {
        ctc_cli_out("Other fdb stress have used, please remove first! \n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        sal_memcpy(&param, l2_addr, sizeof(ctc_l2_addr_t));
        static_flag = param.flag & 1;
        g_ctc_stress_l2->fdb = (ctc_l2_addr_t*)sal_malloc(fdb_num*sizeof(ctc_l2_addr_t));
        if (g_ctc_stress_l2->fdb == NULL)
        {
            ctc_cli_out("Memory alloc error ! \n");
            return CLI_ERROR;
        }
    }

    ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type || CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type )
    {
        for (loop = 0; loop < fdb_num; loop++)
        {
            if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
            {
                if (is_random || is_random_all)
                {
                    *((uint32*)&param.mac[0]) = sal_rand();
                    *((uint16*)&param.mac[4]) = sal_rand();
                     param.fid = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_FID] + 1;
                     temp1  = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_CHIP_NUM]; /*gchip*/
                     temp2  = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM];   /*lport*/
                     param.gport = (is_random)?(param.gport):(CTC_MAP_LPORT_TO_GPORT(temp1,temp2));
                     param.flag = (is_random)?(param.flag):(sal_rand() % (CTC_L2_FLAG_PENDING_ENTRY <<1));
                     param.flag = (is_random)?(param.flag):(param.flag || static_flag);
                }
                else
                {
                    *((uint32*)&param.mac[2]) = (*((uint32*)&param.mac[2])) + g_ctc_stress_l2->step[CTC_CLI_STRESS_L2_TYPE_FDB];
                    param.fid = (param.fid + (g_ctc_stress_l2->step[CTC_CLI_STRESS_L2_TYPE_FDB])) % capability[CTC_GLOBAL_CAPABILITY_MAX_FID];
                }
                param.mac[0] &= 0xFE;
                if (flag)
                {
                    param.fid = 0xFFFF;
                }
                sal_memcpy(g_ctc_stress_l2->fdb + loop, &param, sizeof(ctc_l2_addr_t));

                if (g_ctcs_api_en)
                {
                    ret =  ctcs_l2_add_fdb(g_api_lchip, &param);
                }
                else
                {
                    ret =  ctc_l2_add_fdb(&param);
                }

            }
            else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
            {
                if (!g_ctc_stress_l2 || !g_ctc_stress_l2->fdb)
                {
                    ctc_cli_out("No FDB resource need to remove \n");
                    return CLI_SUCCESS;
                }
                if (g_ctcs_api_en)
                {
                    ret = ctcs_l2_remove_fdb(g_api_lchip, (g_ctc_stress_l2->fdb + loop));
                }
                else
                {
                    ret = ctc_l2_remove_fdb(g_ctc_stress_l2->fdb + loop);
                }
            }
            if (ret == CTC_E_NONE)
            {
                valid_count++;
            }
            CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);
        }
    }
    ctc_cli_out("Operate valid count: %d \n", valid_count);
    if (CTC_CLI_STRESS_OP_TYPE_UPDATE == stress->op_type)
    {
        if (!g_ctc_stress_l2 || !g_ctc_stress_l2->fdb)
        {
            ctc_cli_out("No FDB resource need to flush \n");
            return CLI_SUCCESS;
        }

        if (g_ctcs_api_en)
        {
            ret = ctcs_l2_flush_fdb(g_api_lchip, &Flush);
        }
        else
        {
            ret = ctc_l2_flush_fdb(&Flush);
        }

        if (ret == CTC_E_NONE)
        {
            ctc_cli_out("Flush Fdb Success, but can't flush black hole mac and system mac !!!\n");
        }
        else
        {
            ctc_cli_out("Flush Fdb error: %s\n", ctc_get_error_desc(ret));
        }
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf, "test number:%5d", g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_FDB]);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type || CTC_CLI_STRESS_OP_TYPE_UPDATE == stress->op_type)
    {
        g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_FDB] = 0;
        g_ctc_stress_l2->step[CTC_CLI_STRESS_L2_TYPE_FDB] = 0;
        if (g_ctc_stress_l2->fdb)
        {
            sal_free(g_ctc_stress_l2->fdb);
            g_ctc_stress_l2->fdb = NULL;
        }
    }
    return ret;
}

int32
_ctc_cli_stress_l2_mcast(ctc_cli_stress_t* stress, ctc_l2_mcast_addr_t* mcast)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count = 0;
    uint32 group_num = 0;
    sal_systime_t tv1;
    sal_systime_t tv2;
    char str_buf[32] = {0};
    ctc_cli_stress_output_t output;
    uint8 is_random = 0;
    uint8 is_random_all = 0;
    uint8 step = 0;
    ctc_l2_mcast_addr_t param;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = { 0 };

    sal_memset(&param, 0, sizeof(ctc_l2_mcast_addr_t));
    is_random = (CTC_CLI_STRESS_DATA_MODE_RANDOM == stress->data_mode ) ? 1:0;
    is_random_all = (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode ) ? 1:0;
    group_num = g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_MCAST];
    step = g_ctc_stress_l2->step[CTC_CLI_STRESS_L2_TYPE_MCAST];

    if (g_ctc_stress_l2->l2mc_addr && (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type))
    {
        ctc_cli_out("Other fdb stress have used, please remove first! \n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
    {
        sal_memcpy(&param, mcast, sizeof(ctc_l2_mcast_addr_t));
        g_ctc_stress_l2->l2mc_addr = (ctc_l2_mcast_addr_t*)sal_malloc(group_num*sizeof(ctc_l2_mcast_addr_t));
        if (g_ctc_stress_l2->l2mc_addr == NULL)
        {
            ctc_cli_out("Memory alloc error ! \n");
            return CLI_ERROR;
        }
    }

    ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);

    for (loop = 0; loop < group_num; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress->op_type)
        {
            if (is_random || is_random_all)
            {
                *((uint32*)&param.mac[0]) = sal_rand();
                *((uint16*)&param.mac[4]) = sal_rand();
                param.fid = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_FID] + 1;
                param.l2mc_grp_id = sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MCAST_GROUP_NUM] + 1;
            }
            else
            {
                *((uint32*)&param.mac[2]) = (*((uint32*)&param.mac[2])) + step;
                param.fid = (param.fid + step) % capability[CTC_GLOBAL_CAPABILITY_MAX_FID] + 1;
                param.l2mc_grp_id = (param.l2mc_grp_id + step) % capability[CTC_GLOBAL_CAPABILITY_MCAST_GROUP_NUM];
            }
            param.mac[0] |= 0x01;
            sal_memcpy(g_ctc_stress_l2->l2mc_addr + loop, &param, sizeof(ctc_l2_mcast_addr_t));
            if (g_ctcs_api_en)
            {
                ret =  ctcs_l2mcast_add_addr(g_api_lchip, &param);
            }
            else
            {
                ret =  ctc_l2mcast_add_addr(&param);
            }

        }
        else
        {
            if (!g_ctc_stress_l2 || !g_ctc_stress_l2->l2mc_addr)
            {
                ctc_cli_out("No Mcast Group resource need to remove \n");
                return CLI_SUCCESS;
            }
            if (g_ctcs_api_en)
            {
                ret = ctcs_l2mcast_remove_addr(g_api_lchip, (g_ctc_stress_l2->l2mc_addr + loop));
            }
            else
            {
                ret = ctc_l2mcast_remove_addr(g_ctc_stress_l2->l2mc_addr + loop);
            }
        }
        if (ret == CTC_E_NONE)
        {
            valid_count++;
        }
        CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);
     }
     ctc_cli_out("Operate valid count: %d \n", valid_count);
     if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress->op_type)
     {
         g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_MCAST] = 0;
         g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_MCAST_MEMBER] = 0;
         g_ctc_stress_l2->step[CTC_CLI_STRESS_L2_TYPE_MCAST] = 0;
         if (g_ctc_stress_l2->l2mc_addr)
         {
             sal_free(g_ctc_stress_l2->l2mc_addr);
             g_ctc_stress_l2->l2mc_addr = NULL;
         }
     }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf, "test number:%5d", g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_MCAST]);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    return ret;
}

int32
_ctc_cli_stress_l2_mcast_member(ctc_cli_stress_t* stress, ctc_l2_mcast_addr_t* mcast)
{
    int32 ret = CLI_SUCCESS;
    uint32 loop_grp = 0;
    uint32 loop_mem = 0;
    uint32 valid_count = 0;
    uint32 group_num = 0;
    uint32 member_num = 0;
    uint32 temp_gchip = 0;
    uint32 temp_lport = 0;
    sal_systime_t tv1;
    sal_systime_t tv2;
    char str_buf[32] = { 0 };

    ctc_cli_stress_output_t output;
    uint8 is_random_all = 0;
    ctc_l2_mcast_addr_t param;
    uint32 capability[CTC_GLOBAL_CAPABILITY_MAX] = { 0 };

    sal_memset(&param, 0, sizeof(ctc_l2_mcast_addr_t));
    is_random_all = (CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress->data_mode ) ? 1:0;
    group_num = g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_MCAST];
    member_num = g_ctc_stress_l2->entry_num [CTC_CLI_STRESS_L2_TYPE_MCAST_MEMBER];

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv1);
    }

    ctc_global_ctl_get(CTC_GLOBAL_CHIP_CAPABILITY, capability);

    for (loop_grp = 0; loop_grp < group_num; loop_grp++)
    {
        sal_memcpy(&param, mcast, sizeof(ctc_l2_mcast_addr_t));
        sal_memcpy(&(param.mac), (g_ctc_stress_l2->l2mc_addr + loop_grp)->mac, sizeof(param.mac));
        param.fid = (g_ctc_stress_l2->l2mc_addr + loop_grp)->fid;

        if (param.port_bmp_en)
        {
            if (g_ctcs_api_en)
            {
                ret =  ctcs_l2mcast_add_member(g_api_lchip, &param);
            }
            else
            {
                ret =  ctc_l2mcast_add_member(&param);
            }
        }
        else
        {
            for (loop_mem = 0; loop_mem < member_num; loop_mem++)
            {
                if ( is_random_all)
                {/*AD random*/
                    param.member.nh_id = (param.with_nh) ? ((param.member.nh_id + 1) % CTC_NH_DEFAULT_MAX_EXTERNAL_NEXTHOP_NUM + 3) : 0;
                    temp_gchip  = (param.with_nh) ? 0 : (sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_CHIP_NUM]); /*gchip*/
                    temp_lport  = (param.with_nh) ? 0 : (sal_rand() % capability[CTC_GLOBAL_CAPABILITY_MAX_PORT_NUM]);   /*lport*/
                    param.member.mem_port = CTC_MAP_LPORT_TO_GPORT(temp_gchip, temp_lport);
                }
                else /*AD fixed, nhid incr for perf test*/
                {
                    param.member.nh_id = (param.with_nh) ? ((param.member.nh_id + 1) % CTC_NH_DEFAULT_MAX_EXTERNAL_NEXTHOP_NUM + 3) : 0;
                }
                if (g_ctcs_api_en)
                {
                    ret =  ctcs_l2mcast_add_member(g_api_lchip, &param);
                }
                else
                {
                    ret =  ctc_l2mcast_add_member(&param);
                }
                if (ret == CTC_E_NONE)
                {
                    valid_count++;
                }
                CTC_CLI_STRESS_CHECK_RETURN(ret, stress->test_type, valid_count);
            }
            ctc_cli_out("Operate valid count: %d \n", valid_count);
        }
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress->test_type)
    {
        sal_gettime(&tv2);
        sal_sprintf(str_buf, "test number:%5d", g_ctc_stress_l2->entry_num[CTC_CLI_STRESS_L2_TYPE_MCAST]);
        output.buf = str_buf;
        output.op = stress->op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    return ret;
}


CTC_CLI(ctc_cli_stress_vlan_mapping_add,
        ctc_cli_stress_vlan_mapping_add_cmd,
        "vlan mapping add port GPORT_ID {svid VLAN_ID | cvid VLAN_ID | stag-cos COS | ctag-cos COS |} mapping-to {new-cvid VLAN_ID | new-scos COS | new-ccos COS | new-svid VLAN_ID | stag-op TAG_OP (sl-svid TAG_SL|) (sl-scos TAG_SL|) | ctag-op TAG_OP (sl-cvid TAG_SL|) (sl-ccos TAG_SL|) | use-flex | scl-id SCL_ID } (random | incr (step STEP|) | random-all) (entry-num NUMBER) (spec | stress | perf)",
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Add vlan mapping entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Stag vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Ctag vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Stag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        "Ctag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        "Mapping to",
        "Mapped cvlan",
        "Vlan id value",
        "New stag cos",
        CTC_CLI_COS_RANGE_DESC,
        "New ctag cos",
        CTC_CLI_COS_RANGE_DESC,
        "Mapped svlan",
        "Vlan id value",
        "Stag operation type",
        CTC_CLI_TAG_OP_DESC,
        "Svid select",
        CTC_CLI_TAG_SL_DESC,
        "Scos select",
        CTC_CLI_TAG_SL_DESC,
        "Ctag operation type",
        CTC_CLI_TAG_OP_DESC,
        "Cvid select",
        CTC_CLI_TAG_SL_DESC,
        "Ccos select",
        CTC_CLI_TAG_SL_DESC,
        "Use tcam key",
        "Scl ID",
        CTC_CLI_SCL_ID_VALUE,
        "Key value is random, action is fixed",
        "Key value is increase, action is fixed",
        "Increase step",
        "Step value",
        "Key value is random, action is random",
        "Entry num",
        "Number",
        "Spec test",
        "Stress test",
        "Performance test")
{
    /* if do not specify step value, default step is 1 */
    int32 ret = CTC_E_NONE;
    uint32 index = 0xFF;
    uint16 gport = 0;
    ctc_vlan_mapping_t vlan_mapping;
    ctc_cli_stress_t stress_param;

    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));
    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    vlan_mapping.key = CTC_VLAN_MAPPING_KEY_NONE;

    index = CTC_CLI_GET_ARGC_INDEX("svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan id", vlan_mapping.old_svid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_SVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan id", vlan_mapping.old_cvid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("stag cos", vlan_mapping.old_scos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_STAG_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("ctag cos", vlan_mapping.old_ccos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_VLAN_MAPPING_KEY_CTAG_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("new-svid", vlan_mapping.new_svid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);

        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_SVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-scos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("new-scos", vlan_mapping.new_scos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_SCOS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-ccos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("new-ccos", vlan_mapping.new_ccos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_CCOS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("new-cvid", vlan_mapping.new_cvid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.action |= CTC_VLAN_MAPPING_OUTPUT_CVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stag-op");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("stag-op", vlan_mapping.stag_op, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-svid", vlan_mapping.svid_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-scos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-scos", vlan_mapping.scos_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ctag-op");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("ctag-op", vlan_mapping.ctag_op, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-cvid", vlan_mapping.cvid_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-ccos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-ccos", vlan_mapping.ccos_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("use-flex");
    if (0xFF != index)
    {
        vlan_mapping.flag |= CTC_VLAN_MAPPING_FLAG_USE_FLEX;
    }

    index = CTC_CLI_GET_ARGC_INDEX("scl-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("scl-id", vlan_mapping.scl_id, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    ret = _ctc_stress_l2_cli_parse_common(&(argv[0]), argc, &stress_param, CTC_CLI_STRESS_L2_TYPE_VLAN_MAPING);
    if (ret)
    {
        return ret;
    }
    g_ctc_stress_l2->gport = gport;

    ret = _ctc_cli_stress_vlan_mapping(gport, vlan_mapping, stress_param, g_ctc_stress_l2 -> entry_num[CTC_CLI_STRESS_L2_TYPE_VLAN_MAPING], g_ctc_stress_l2 -> step[CTC_CLI_STRESS_L2_TYPE_VLAN_MAPING]);

    return ret;
}

CTC_CLI(ctc_cli_stress_vlan_mapping_remove,
        ctc_cli_stress_vlan_mapping_remove_cmd,
        "vlan-mapping remove",
        "Advance vlan module",
        "Remove vlan mapping entry")
{
    int32 ret = CTC_E_NONE;
    uint32 entry_num = 0;
    ctc_vlan_mapping_t vlan_mapping;
    ctc_cli_stress_t stress_param;

    sal_memset(&vlan_mapping, 0, sizeof(ctc_vlan_mapping_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_MAX;
    stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_MAX;

    ret = _ctc_cli_stress_vlan_mapping(g_ctc_stress_l2->gport, vlan_mapping, stress_param, entry_num, 0);

    return ret;
}

CTC_CLI(ctc_cli_stress_egress_vlan_mapping_add,
        ctc_cli_stress_egress_vlan_mapping_add_cmd,
        "vlan mapping egress add port GPORT_ID {svid VLAN_ID | stag-cos COS | cvid VLAN_ID | ctag-cos COS} mapping-to {new-cvid VLAN_ID | new-svid VLAN_ID | new-scos COS | new-ccos COS| stag-op TAG_OP (sl-svid TAG_SL|) (sl-scos TAG_SL|) | ctag-op TAG_OP (sl-cvid TAG_SL|) (sl-ccos TAG_SL|)} (random | incr (step STEP|) | random-all) (entry-num NUMBER) (spec | stress | perf)",
        CTC_CLI_VLAN_M_STR,
        "Vlan mapping",
        "Use egress vlan mapping",
        "Add vlan mapping entry",
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Stag vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Stag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        "Ctag vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Ctag priority or cos",
        CTC_CLI_COS_RANGE_DESC,
        "Mapping to",
        "Mapped cvlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Mapped svlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "New stag cos",
        CTC_CLI_COS_RANGE_DESC,
        "New ctag cos",
        CTC_CLI_COS_RANGE_DESC,
        "Stag operation type",
        CTC_CLI_TAG_OP_DESC,
        "Svid select",
        CTC_CLI_TAG_SL_DESC,
        "Scos select",
        CTC_CLI_TAG_SL_DESC,
        "Ctag operation type",
        CTC_CLI_TAG_OP_DESC,
        "Cvid select",
        CTC_CLI_TAG_SL_DESC,
        "Ccos select",
        CTC_CLI_TAG_SL_DESC,
        "Key value is random, action is fixed",
        "Key value is increase, action is fixed",
        "Increase step",
        "Step value",
        "Key value is random, action is random",
        "Entry num",
        "Number",
        "Spec test",
        "Stress test",
        "Performance test")
{
    /* if do not specify step value, default step is 1 */
    uint8 index = 0xFF;
    uint16 gport = 0;
    int32 ret = 0;
    ctc_egress_vlan_mapping_t vlan_mapping;
    ctc_cli_stress_t stress_param;

    sal_memset(&vlan_mapping, 0, sizeof(ctc_egress_vlan_mapping_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    CTC_CLI_GET_UINT16_RANGE("gport", gport, argv[0], 0, CTC_MAX_UINT16_VALUE);

    vlan_mapping.key = CTC_EGRESS_VLAN_MAPPING_KEY_NONE;

    index = CTC_CLI_GET_ARGC_INDEX("svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("svlan id", vlan_mapping.old_svid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_EGRESS_VLAN_MAPPING_KEY_SVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("cvlan id", vlan_mapping.old_cvid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.key |= CTC_EGRESS_VLAN_MAPPING_KEY_CVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("stag cos", vlan_mapping.old_scos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_EGRESS_VLAN_MAPPING_KEY_STAG_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ctag-cos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("ctag cos", vlan_mapping.old_ccos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.key |= CTC_EGRESS_VLAN_MAPPING_KEY_CTAG_COS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("new-svid", vlan_mapping.new_svid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);

        vlan_mapping.action |= CTC_EGRESS_VLAN_MAPPING_OUTPUT_SVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT16_RANGE("new-cvid", vlan_mapping.new_cvid, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        vlan_mapping.action |= CTC_EGRESS_VLAN_MAPPING_OUTPUT_CVID;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-scos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("new-scos", vlan_mapping.new_scos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.action |= CTC_EGRESS_VLAN_MAPPING_OUTPUT_SCOS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("new-ccos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("new-ccos", vlan_mapping.new_ccos, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
        vlan_mapping.action |= CTC_EGRESS_VLAN_MAPPING_OUTPUT_CCOS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("stag-op");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("stag-op", vlan_mapping.stag_op, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-svid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-svid", vlan_mapping.svid_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-scos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-scos", vlan_mapping.scos_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ctag-op");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("ctag-op", vlan_mapping.ctag_op, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-cvid");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-cvid", vlan_mapping.cvid_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    index = CTC_CLI_GET_ARGC_INDEX("sl-ccos");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8_RANGE("sl-ccos", vlan_mapping.ccos_sl, argv[index + 1], 0, CTC_MAX_UINT8_VALUE);
    }

    ret = _ctc_stress_l2_cli_parse_common(&(argv[0]), argc, &stress_param, CTC_CLI_STRESS_L2_TYPE_VLAN_MAPING_EGRESS);
    if (ret)
    {
        return ret;
    }
    stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;

    g_ctc_stress_l2->gport = gport;

    ret = _ctc_cli_stress_egress_vlan_mapping(gport, vlan_mapping, stress_param, g_ctc_stress_l2 -> entry_num[CTC_CLI_STRESS_L2_TYPE_VLAN_MAPING_EGRESS], g_ctc_stress_l2 -> step[CTC_CLI_STRESS_L2_TYPE_VLAN_MAPING_EGRESS]);

    return ret;
}

CTC_CLI(ctc_cli_stress_egress_vlan_mapping_remove,
        ctc_cli_stress_egress_vlan_mapping_remove_cmd,
        "vlan-mapping egress remove",
        "Advance vlan module",
        "Egress vlan mapping",
        "Remove egress vlan mapping entry")
{
    int32 ret = CTC_E_NONE;
    uint32 entry_num = 0;
    ctc_egress_vlan_mapping_t vlan_mapping;
    ctc_cli_stress_t stress_param;

    sal_memset(&vlan_mapping, 0, sizeof(ctc_egress_vlan_mapping_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_MAX;
    stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_MAX;

    ret = _ctc_cli_stress_egress_vlan_mapping(g_ctc_stress_l2->gport, vlan_mapping, stress_param, entry_num, 0);

    return ret;
}

CTC_CLI(ctc_cli_stress_vlan_class_add,
        ctc_cli_stress_vlan_class_add_cmd,
        "vlan classifier add (mac macsa MAC |ipv4 ip-sa A.B.C.D | ipv6 ip-sa X:X::X:X) vlan VLAN_ID (random | incr (step STEP|) | random-all ) (entry-num NUMBER) (spec | stress | perf)",
        CTC_CLI_VLAN_M_STR,
        "Vlan classification",
        "Add",
        "Mac based vlan class",
        CTC_CLI_MACSA_DESC,
        CTC_CLI_MAC_FORMAT,
        "IPv4 based vlan class",
        "IPv4 source address",
        CTC_CLI_IPV4_FORMAT,
        "IPv6 based vlan class",
        "IPv6 source address",
        CTC_CLI_IPV6_FORMAT,
        "Vlan",
        CTC_CLI_VLAN_RANGE_DESC,
        "Key value is random, action is fixed",
        "Key value is increase, action is fixed",
        "Increase step",
        "Step value",
        "Key value is random, action is random",
        "Entry num",
        "Number",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    ipv6_addr_t ipv6_address;
    ctc_vlan_class_t vlan_class;
    ctc_cli_stress_t stress_param;

    sal_memset(&ipv6_address, 0, sizeof(ipv6_addr_t));
    sal_memset(&vlan_class, 0, sizeof(ctc_vlan_class_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    index = CTC_CLI_GET_ARGC_INDEX("mac");
    if (0xFF != index)
    {
        vlan_class.type = CTC_VLAN_CLASS_MAC;
        CTC_CLI_GET_MAC_ADDRESS("source mac address", vlan_class.vlan_class.mac, argv[index + 2]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv4");
    if (0xFF != index)
    {
        vlan_class.type = CTC_VLAN_CLASS_IPV4;
        CTC_CLI_GET_IPV4_ADDRESS("ipv4 source address", vlan_class.vlan_class.ipv4_sa, argv[index + 2]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ipv6");
    if (0xFF != index)
    {
        vlan_class.type = CTC_VLAN_CLASS_IPV6;
        CTC_CLI_GET_IPV6_ADDRESS("ipv6 source address", ipv6_address, argv[index + 2]);
        vlan_class.vlan_class.ipv6_sa[0] = sal_htonl(ipv6_address[0]);
        vlan_class.vlan_class.ipv6_sa[1] = sal_htonl(ipv6_address[1]);
        vlan_class.vlan_class.ipv6_sa[2] = sal_htonl(ipv6_address[2]);
        vlan_class.vlan_class.ipv6_sa[3] = sal_htonl(ipv6_address[3]);
    }
    /* the way of getting vlan id is depend on cli struct */
    CTC_CLI_GET_UINT16_RANGE("vlan id", vlan_class.vlan_id, argv[3], 0, CTC_MAX_UINT16_VALUE);

    ret = _ctc_stress_l2_cli_parse_common(&(argv[0]), argc, &stress_param,CTC_CLI_STRESS_L2_TYPE_VLAN_CLASSIFIER);
    if (ret)
    {
        return ret;
    }

    stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;

    ret = _ctc_cli_stress_vlan_class(vlan_class, stress_param, g_ctc_stress_l2 -> entry_num[CTC_CLI_STRESS_L2_TYPE_VLAN_CLASSIFIER], g_ctc_stress_l2 -> step[CTC_CLI_STRESS_L2_TYPE_VLAN_CLASSIFIER]);

    return ret;
}

CTC_CLI(ctc_cli_stress_vlan_class_remove,
        ctc_cli_stress_vlan_class_remove_cmd,
        "vlan classifier remove",
        "Advance vlan module",
        "Vlan classifier",
        "Remove vlan classifier entry")
{
    int32 ret = CTC_E_NONE;
    uint32 entry_num = 0;
    ctc_vlan_class_t vlan_class;
    ctc_cli_stress_t stress_param;

    sal_memset(&vlan_class, 0, sizeof(ctc_vlan_class_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_MAX;
    stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_MAX;

    ret = _ctc_cli_stress_vlan_class(vlan_class, stress_param, entry_num, 0);

    return ret;
}

#define __FDB__
CTC_CLI(ctc_cli_stress_l2_addr_add,
        ctc_cli_stress_l2_addr_add_cmd,
        "l2 fdb add mac MAC fid FID port GPORT_ID \
       ({static | black-hole-mac | remote-dynamic | discard | src-discard | src-discard-to-cpu | copy-to-cpu | bind-port | raw-pkt-elog-cpu | service-queue | untagged | protocol-entry | system-mac | ptp-entry | self-address | white-list-entry|aging-disable|learn-limit-exempt}|)\
       (random|incr (step STEP|)|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        CTC_CLI_L2_M_STR,
        CTC_CLI_FDB_DESC,
        CTC_CLI_ADD,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        CTC_CLI_GPORT_DESC,
        CTC_CLI_GPORT_ID_DESC,
        "Static fdb table",
        "black hole mac",
        "Remote chip learing dynamic fdb",
        "Destination discard fdb table",
        "Source discard fdb table",
        "Source discard and send to cpu fdb table",
        "Forward and copy to cpu fdb table",
        "Port bind fdb table",
        "Send raw packet to cpu,using for terminating protocol pakcet(LBM/DMM/LMM) to cpu",
        "Packet will enqueue by service id",
        "Output packet will has no vlan tag",
        "Protocol entry enable exception flag",
        "Indicate the entry is system mac, it can't be deleted by flush api, using for MAC DA",
        "PTP mac address for ptp process",
        "Indicate mac address is switch's",
        "Indicate the entry is white list entry, it won't do aging, and will do learning",
        "Indicate mac address should not be aged",
        "Learn limit exempt",
        "Key value is random, Ad is fixed",
        "Key value is increase, Ad is fixed",
        "Increase by step",
        "Num of step",
        "Key value is random, Ad is random",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    ctc_l2_addr_t l2_addr;
    uint8 index = 0xFF;
    ctc_cli_stress_t stress;
    uint8 flag = 0;

   /*1) parse port/nexthop */
   sal_memset(&l2_addr, 0, sizeof(ctc_l2_addr_t));
   sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));

   CTC_CLI_GET_MAC_ADDRESS("mac address", l2_addr.mac, argv[0]);
   CTC_CLI_GET_UINT16_RANGE("FID", l2_addr.fid, argv[1], 0, CTC_MAX_UINT16_VALUE);
   CTC_CLI_GET_UINT16_RANGE("port", l2_addr.gport, argv[2], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("static");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_IS_STATIC;
    }
    index = CTC_CLI_GET_ARGC_INDEX("black-hole-mac");

    if (0xFF != index)
    {
        flag = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("remote-dynamic");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_REMOTE_DYNAMIC;
    }

    index = CTC_CLI_GET_ARGC_INDEX("discard");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_DISCARD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-discard");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_SRC_DISCARD;
    }

    index = CTC_CLI_GET_ARGC_INDEX("src-discard-to-cpu");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_SRC_DISCARD_TOCPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("copy-to-cpu");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_COPY_TO_CPU;
    }

    index = CTC_CLI_GET_ARGC_INDEX("bind-port");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_BIND_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("protocol-entry");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_PROTOCOL_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("system-mac");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_SYSTEM_RSV;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ptp-entry");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_PTP_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("self-address");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_SELF_ADDRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("white-list-entry");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_WHITE_LIST_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("aging-disable");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_AGING_DISABLE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("learn-limit-exempt");
    if (0xFF != index)
    {
        l2_addr.flag |= CTC_L2_FLAG_LEARN_LIMIT_EXEMPT;
    }

    ret = _ctc_stress_l2_cli_parse_common(&(argv[0]), argc, &stress, CTC_CLI_STRESS_L2_TYPE_FDB);

    if (ret)
    {
        return ret;
    }
   stress.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;
   ret = _ctc_cli_stress_fdb(&stress, &l2_addr, flag);
   return ret;

}

CTC_CLI(ctc_cli_stress_l2_addr_remove,
        ctc_cli_stress_l2_addr_remove_cmd,
        "l2 fdb remove (black-hole-mac|)(perf|)",
        "L2 module",
        "Fdb",
        "Remove fdb entry one by one",
        "black hole mac",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    ctc_cli_stress_t stress;
    uint8 index = 0xFF;
    uint8 flag = 0;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));
    stress.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    stress.data_mode = CTC_CLI_STRESS_TEST_TYPE_STRESS;

    index = CTC_CLI_GET_ARGC_INDEX("black-hole-mac");
    if (0xFF != index)
    {
        flag = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_fdb(&stress, 0, flag);

    return ret;
}

CTC_CLI(ctc_cli_stress_l2_addr_flush,
        ctc_cli_stress_l2_addr_flush_cmd,
        "l2 fdb flush",
        "L2 module",
        "Fdb",
        "Flush fdb entry by all all")
{
    int32 ret = CLI_SUCCESS;

    ctc_cli_stress_t stress;
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));

    stress.op_type = CTC_CLI_STRESS_OP_TYPE_UPDATE;
    stress.test_type = CTC_CLI_STRESS_TEST_TYPE_MAX;
    stress.data_mode = CTC_CLI_STRESS_DATA_MODE_MAX;

    ret = _ctc_cli_stress_fdb(&stress, 0, 0);

    return ret;
}


CTC_CLI(ctc_cli_stress_l2_mcast_group_add,
        ctc_cli_stress_l2_mcast_group_add_cmd,
        "l2 mcast add group GROUP_ID mac MAC fid FID ({protocol-exp | ptp-entry | self-address}|)(random|incr (step STEP|)|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        CTC_CLI_L2_M_STR,
        "Multicast",
        CTC_CLI_ADD,
        "Multicast Group",
        CTC_CLI_GLOBAL_MCASTGRP_ID_DESC,
        CTC_CLI_MAC_DESC,
        CTC_CLI_MAC_FORMAT,
        CTC_CLI_FID_DESC,
        CTC_CLI_FID_ID_DESC,
        "Protocol Entry flag",
        "PTP mac address for ptp process",
        "Indicate mac address is switch's",
        "Key value is random, Ad is fixed",
        "Key value is increase, Ad is fixed",
        "Increase by step",
        "Num of step",
        "Key value is random, Ad is random",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    ctc_l2_mcast_addr_t l2_mcast;
    uint8 index = 0xFF;
    ctc_cli_stress_t stress;

    sal_memset(&l2_mcast, 0, sizeof(ctc_l2_mcast_addr_t));
    sal_memset(&stress, 0, sizeof(ctc_cli_stress_t));

    CTC_CLI_GET_UINT16_RANGE("group id", l2_mcast.l2mc_grp_id, argv[0], 0, CTC_MAX_UINT16_VALUE);
    CTC_CLI_GET_MAC_ADDRESS("mac address", l2_mcast.mac, argv[1]);
    CTC_CLI_GET_UINT16_RANGE("FID", l2_mcast.fid, argv[2], 0, CTC_MAX_UINT16_VALUE);

    CTC_CLI_GET_UINT16_RANGE("forwarding id", l2_mcast.fid, argv[2], 0, CTC_MAX_UINT16_VALUE);

    index = CTC_CLI_GET_ARGC_INDEX("protocol-exp");
    if (0xFF != index)
    {
        l2_mcast.flag |= CTC_L2_FLAG_PROTOCOL_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("ptp-entry");
    if (0xFF != index)
    {
        l2_mcast.flag |= CTC_L2_FLAG_PTP_ENTRY;
    }

    index = CTC_CLI_GET_ARGC_INDEX("self-address");
    if (0xFF != index)
    {
        l2_mcast.flag |= CTC_L2_FLAG_SELF_ADDRESS;
    }

    ret = _ctc_stress_l2_cli_parse_common(&(argv[0]), argc, &stress, CTC_CLI_STRESS_L2_TYPE_MCAST);

    if (ret)
    {
        return ret;
    }
   stress.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;
   ret = _ctc_cli_stress_l2_mcast(&stress, &l2_mcast);
   return ret;

}

CTC_CLI(ctc_cli_stress_l2_mcast_group_remove,
        ctc_cli_stress_l2_mcast_group_remove_cmd,
        "l2 mcast remove group (perf|)",
        "L2 module",
        "Mcast",
        "Remove",
        "Mcast group",
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
    ret = _ctc_cli_stress_l2_mcast(&stress, 0);

    return ret;
}

CTC_CLI(ctc_cli_stress_l2_mcast_member_add,
        ctc_cli_stress_l2_mcast_member_add_cmd,
        "l2 mcast add member (({port GPORT_ID | nexthop NH_ID}) | (" CTC_CLI_PORT_BITMAP_STR "(gchip GCHIP_ID|)))  (remote-chip|) (service-queue|) (assign-output|)(random|incr (step STEP|)|random-all) (entry-num ENTRY_NUM) (spec|stress|perf)",
        CTC_CLI_L2_M_STR,
        "L2 multicast",
        CTC_CLI_ADD,
        "The member of multicast group",
        "Global port /Remote chip id",
        "Local member  gport:gchip(8bit) +local phy port(8bit),  remote chip entry,gport:  gchip(8bit) + remote gchip id(8bit)",
        "Next hop",
        CTC_CLI_NH_ID_STR,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_PORT_BITMAP_DESC,
        CTC_CLI_PORT_BITMAP_VALUE_DESC,
        CTC_CLI_GCHIP_DESC,
        CTC_CLI_GCHIP_ID_DESC,
        "Indicate the member is remote chip entry",
        "Enable transport by service queue",
        "Indicate output port is user assigned",
        "Key value is random, Ad is fixed",
        "Key value is increase, Ad is fixed",
        "Increase by step",
        "Num of step",
        "Key value is random, Ad is random",
        "Entry number",
        "<0-0xFFFFFFFF>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret  = CLI_SUCCESS;
    uint8 index = 0;
    ctc_l2_mcast_addr_t l2mc_addr;
    ctc_cli_stress_t stress;

    sal_memset(&l2mc_addr, 0, sizeof(ctc_l2_mcast_addr_t));

    l2mc_addr.member_invalid = 1;
    index = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (index == 0xFF)
    {
        index = CTC_CLI_GET_ARGC_INDEX("port");
        if (index != 0xFF)
        {
            l2mc_addr.member_invalid  = 0;
            CTC_CLI_GET_UINT16_RANGE("gport", l2mc_addr.member.mem_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }
    else
    {
        index = CTC_CLI_GET_ARGC_INDEX("port");
        if (index != 0xFF)
        {
            l2mc_addr.member_invalid  = 0;
            CTC_CLI_GET_UINT16_RANGE("remote", l2mc_addr.member.mem_port, argv[index + 1], 0, CTC_MAX_UINT16_VALUE);
        }
    }
    index = CTC_CLI_GET_ARGC_INDEX("nexthop");
    if (index != 0xFF)
    {
        l2mc_addr.with_nh = 1;
        l2mc_addr.member_invalid  = 0;
        CTC_CLI_GET_UINT32_RANGE("nh id",  l2mc_addr.member.nh_id, argv[index + 1], 0, CTC_MAX_UINT32_VALUE);
    }
    index = CTC_CLI_GET_ARGC_INDEX("remote-chip");
    if (index != 0xFF)
    {
        l2mc_addr.remote_chip = TRUE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("assign-output");
    if (index != 0xFF)
    {
        l2mc_addr.flag |= CTC_L2_FLAG_ASSIGN_OUTPUT_PORT;
    }
    l2mc_addr.port_bmp_en = 0;
    CTC_CLI_PORT_BITMAP_GET(l2mc_addr.member.port_bmp);
    if (l2mc_addr.member.port_bmp[0] || l2mc_addr.member.port_bmp[1] ||
        l2mc_addr.member.port_bmp[8] || l2mc_addr.member.port_bmp[9])
    {
        l2mc_addr.port_bmp_en = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("gchip");
    if (index != 0xFF)
    {
        CTC_CLI_GET_UINT8("gchip", l2mc_addr.gchip_id, argv[index + 1]);
    }

    ret = _ctc_stress_l2_cli_parse_common(&(argv[0]), argc, &stress, CTC_CLI_STRESS_L2_TYPE_MCAST_MEMBER);

    if (ret)
    {
        return ret;
    }
   stress.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;
   ret = _ctc_cli_stress_l2_mcast_member(&stress, &l2mc_addr);
   return ret;
}

int32
ctc_stress_test_l2_cli_init(void)
{
    g_ctc_stress_l2 = (ctc_cli_stress_test_l2_t*)sal_malloc(sizeof(ctc_cli_stress_test_l2_t));
    if (!g_ctc_stress_l2)
    {
        return CLI_ERROR;
    }

    sal_memset(g_ctc_stress_l2, 0, sizeof(ctc_cli_stress_test_l2_t));

    install_element(CTC_STRESS_MODE, &ctc_cli_stress_vlan_mapping_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_vlan_mapping_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_egress_vlan_mapping_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_egress_vlan_mapping_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_vlan_class_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_vlan_class_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_l2_addr_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_l2_addr_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_l2_addr_flush_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_l2_mcast_group_add_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_l2_mcast_group_remove_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_l2_mcast_member_add_cmd);

    return CLI_SUCCESS;
}


