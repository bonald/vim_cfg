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
#include "ctc_chip.h"
#include "ctc_stress_test_cli.h"


#define   ________LINKAGG_STRESS________

enum cli_stress_linkagg_remove_type_e
{
    CLI_STRESS_LINKAGG_REMOVE_TYPE_ALL = 0,
    CLI_STRESS_LINKAGG_REMOVE_TYPE_NUMBER,
    CLI_STRESS_LINKAGG_REMOVE_TYPE_PERCENTAGE
};
typedef enum cli_stress_linkagg_remove_type_e   cli_stress_linkagg_remove_type_t;

enum cli_stress_linkagg_member_type_e
{
    CLI_STRESS_LINKAGG_MEMBER_TYPE_DEFAULT = 0,
    CLI_STRESS_LINKAGG_MEMBER_TYPE_NUMBER,
    CLI_STRESS_LINKAGG_MEMBER_TYPE_RANDOM
};
typedef enum cli_stress_linkagg_member_type_e   cli_stress_linkagg_member_type_t;


#define CLI_STRESS_LINKAGG_GROUP_ID_MIN     0
#define CLI_STRESS_LINKAGG_GROUP_ID_MAX     63
#define CLI_STRESS_LINKAGG_GROUP_NUMBER     64


struct ctc_cli_stress_test_interface_s
{
    /* linkagg */
    uint32 linkagg_grp_cnt;
};
typedef struct ctc_cli_stress_test_interface_s ctc_cli_stress_test_interface_t;

ctc_cli_stress_test_interface_t  *g_ctc_stress_interface;






#define   ________LINKAGG_STRESS________

STATIC int32
_ctc_cli_stress_linkagg_rand(uint32 min, uint32 max, uint32 *p_value)
{
    if (min >= max)
        return -1;

    *p_value = ((uint32)sal_rand() % (max -min + 1)) + min;

    return 0;
}

STATIC uint32
_ctc_cli_stress_linkagg_get_member_number_default(uint8 group_type)
{
    uint32 member_number = 0;

    switch (group_type)
    {
        case CTC_LINKAGG_MODE_STATIC:
        case CTC_LINKAGG_MODE_STATIC_FAILOVER:
        case CTC_LINKAGG_MODE_RR:
        {
            member_number = 16;
            break;
        }
        case CTC_LINKAGG_MODE_DLB:
        case CTC_LINKAGG_MODE_RESILIENT:
        {
            member_number = 32;
            break;
        }
        default:
        {
            break;
        }
    }

    return member_number;
}

STATIC uint32
_ctc_cli_stress_linkagg_get_member_number_random(uint8 group_type)
{
    uint32 random_number = 0;
    uint32 member_number = 0;

    switch (group_type)
    {
        case CTC_LINKAGG_MODE_STATIC:
        {
            (void)_ctc_cli_stress_linkagg_rand(1, 28, &random_number);
            if (25 == random_number)
            {
                member_number = 32;
            }
            else if (26 == random_number)
            {
                member_number = 64;
            }
            else if (27 == random_number)
            {
                member_number = 128;
            }
            else if (28 == random_number)
            {
                member_number = 255;
            }
            else
            {
                member_number = random_number;
            }
            break;
        }
        case CTC_LINKAGG_MODE_STATIC_FAILOVER:
        case CTC_LINKAGG_MODE_RR:
        {
            (void)_ctc_cli_stress_linkagg_rand(1, 24, &random_number);
            member_number = random_number;
            break;
        }
        case CTC_LINKAGG_MODE_DLB:
        case CTC_LINKAGG_MODE_RESILIENT:
        {
            (void)_ctc_cli_stress_linkagg_rand(1, 4, &random_number);
            if (1 == random_number)
            {
                member_number = 32;
            }
            else if (2 == random_number)
            {
                member_number = 64;
            }
            else if (3 == random_number)
            {
                member_number = 128;
            }
            else
            {
                member_number = 255;
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return member_number;
}

int32
_ctc_cli_stress_linkagg_remove_group_all(void)
{
    int32 ret;
    uint32 total_number;
    int32 loop;
    uint8 tid;

    total_number = g_ctc_stress_interface->linkagg_grp_cnt;

    if (0 == total_number)
    {
        return CLI_SUCCESS;
    }

    for (loop = 0; loop < total_number; loop++)
    {
        tid = total_number - loop - 1;

        if (g_ctcs_api_en)
        {
            ret = ctcs_linkagg_destroy(g_api_lchip, tid);
        }
        else
        {
            ret = ctc_linkagg_destroy(tid);
        }

        if (CTC_E_NONE != ret)
        {
            ctc_cli_out("%%ctc_linkagg_destroy failed, tid = %u\n", tid);
        }
    }

    g_ctc_stress_interface->linkagg_grp_cnt = 0;

    return CLI_SUCCESS;
}

int32
_ctc_cli_stress_linkagg_remove_group_number(uint16 number)
{
    int32 ret;
    uint32 total_number;
    uint32 target_number;
    int32 loop;
    uint8 tid;

    if (0 == number)
    {
        return CLI_SUCCESS;
    }

    total_number = g_ctc_stress_interface->linkagg_grp_cnt;

    if (0 == total_number)
    {
        return CLI_SUCCESS;
    }

    if (number >= total_number)
    {
        target_number = total_number;
    }
    else
    {
        target_number = number;
    }

    for (loop = 0; loop < target_number; loop++)
    {
        tid = total_number - loop - 1;

        if (g_ctcs_api_en)
        {
            ret = ctcs_linkagg_destroy(g_api_lchip, tid);
        }
        else
        {
            ret = ctc_linkagg_destroy(tid);
        }

        if (CTC_E_NONE != ret)
        {
            ctc_cli_out("%%ctc_linkagg_destroy failed, tid = %u\n", tid);
        }
    }

    g_ctc_stress_interface->linkagg_grp_cnt = total_number - target_number;

    return CLI_SUCCESS;
}

int32
_ctc_cli_stress_linkagg_remove_group_percentage(uint16 percentage)
{
    int32 ret;
    uint32 target_number;
    uint32 total_number;

    total_number = g_ctc_stress_interface->linkagg_grp_cnt;

    target_number = (total_number * percentage) / 100;
    if ((total_number * percentage % 100) >= 50)
    {
        target_number++;
    }

    ret = _ctc_cli_stress_linkagg_remove_group_number(target_number);

    return ret;
}

int32
_ctc_cli_stress_linkagg_add_single_group(ctc_linkagg_group_t *linkagg_grp)
{
    int32 ret = CTC_E_NONE;
    uint8 max_num = 0;
    int32 loop;
    uint8 tid;
    uint16 mid;
    uint32 gport;

    if (g_ctcs_api_en)
    {
        ret = ctcs_linkagg_create(g_api_lchip, linkagg_grp);
    }
    else
    {
        ret = ctc_linkagg_create(linkagg_grp);
    }

    if (CTC_E_NONE != ret)
    {
        ctc_cli_out("%%ctc_linkagg_create failed, ret = %d, tid = %u, linkagg_mode = %u, flag = %u, member_num = %u\n",
                            ret, linkagg_grp->tid, linkagg_grp->linkagg_mode, linkagg_grp->flag, linkagg_grp->member_num);
        return ret;
    }

    tid = linkagg_grp->tid;
    mid = 0;
    max_num = linkagg_grp->member_num;
    for (loop = 0; loop < max_num; loop++)
    {
        gport = (mid << 6) | tid;
        while (CTC_IS_LINKAGG_PORT(gport))
        {
            mid++;
            gport = (mid << 6) | tid;
        };

        if (g_ctcs_api_en)
        {
            ret = ctcs_linkagg_add_port(g_api_lchip, tid, gport);
        }
        else
        {
            ret = ctc_linkagg_add_port(tid, gport);
        }

        if (CTC_E_NONE != ret)
        {
            ctc_cli_out("%%ctc_linkagg_add_port failed, ret = %d, tid = %u, gport = %u\n", ret, tid, gport);
            return CTC_E_NONE;
        }

        mid++;
    }

    return CTC_E_NONE;
}

int32
_ctc_cli_stress_linkagg_add_random_group(uint32 group_number, uint8 member_type, uint32 member_number, ctc_cli_stress_test_type_t type)
{
    int32 ret;
    uint32 loop = 0,valid_count =0;
    sal_systime_t tv1,tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};
    ctc_linkagg_group_t linkagg_grp;
    uint8 tid;
    uint8 group_type;
    uint16 flag;
    uint32 rand_value;

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == type)
    {
        sal_gettime(&tv1);
    }

    tid = g_ctc_stress_interface->linkagg_grp_cnt;

    for (loop = 0; loop < group_number; loop++)
    {
        if (tid > CLI_STRESS_LINKAGG_GROUP_ID_MAX)
        {
            ctc_cli_out("%%tid exceed max\n");
            break;
        }

        rand_value = 0;
        (void)_ctc_cli_stress_linkagg_rand(CTC_LINKAGG_MODE_STATIC, CTC_LINKAGG_MODE_RESILIENT, &rand_value);
        group_type = (uint8)rand_value;

        flag = 0;
        if ((CTC_LINKAGG_MODE_STATIC == group_type) || (CTC_LINKAGG_MODE_STATIC_FAILOVER == group_type))
        {
            rand_value = 0;
            (void)_ctc_cli_stress_linkagg_rand(0, 1, &rand_value);
            if (0 != rand_value)
            {
                flag |= CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST;
            }
        }
        else if (CTC_LINKAGG_MODE_RR == group_type)
        {
            rand_value = 0;
            (void)_ctc_cli_stress_linkagg_rand(0, 1, &rand_value);
            if (0 != rand_value)
            {
                flag |= CTC_LINKAGG_GROUP_FLAG_RANDOM_RR;
            }
        }

        sal_memset(&linkagg_grp, 0, sizeof(ctc_linkagg_group_t));
        linkagg_grp.tid = tid;
        linkagg_grp.linkagg_mode = group_type;
        linkagg_grp.flag = flag;

        switch (member_type)
        {
            case CLI_STRESS_LINKAGG_MEMBER_TYPE_DEFAULT:
            {
                linkagg_grp.member_num = _ctc_cli_stress_linkagg_get_member_number_default(group_type);
                break;
            }
            case CLI_STRESS_LINKAGG_MEMBER_TYPE_NUMBER:
            {
                linkagg_grp.member_num = member_number;
                break;
            }
            case CLI_STRESS_LINKAGG_MEMBER_TYPE_RANDOM:
            {
                linkagg_grp.member_num = _ctc_cli_stress_linkagg_get_member_number_random(group_type);
                break;
            }
            default:
            {
                linkagg_grp.member_num = _ctc_cli_stress_linkagg_get_member_number_default(group_type);
                break;
            }
        }

        ret = _ctc_cli_stress_linkagg_add_single_group(&linkagg_grp);

        g_ctc_stress_interface->linkagg_grp_cnt++;

        CTC_CLI_STRESS_CHECK_RETURN(ret, type, valid_count);

        if (CTC_E_NONE == ret)
        {
            valid_count++;
        }

        tid++;
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == type)
    {
        sal_gettime(&tv2);

        sal_sprintf(str_buf,"Linkagg Entry:%5d", group_number);
        output.buf = str_buf;
        output.op = CTC_CLI_STRESS_OP_TYPE_ADD;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    return CLI_SUCCESS;
}

int32
_ctc_cli_stress_linkagg_add_group(uint8 group_type, uint16 flag, uint32 group_number, uint8 member_type, uint32 member_number, ctc_cli_stress_test_type_t type)
{
    int32 ret = 0;
    uint32 loop = 0,valid_count =0;
    sal_systime_t tv1,tv2;
    ctc_cli_stress_output_t output;
    char str_buf[32] = {0};
    ctc_linkagg_group_t linkagg_grp;
    uint8 tid;

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == type)
    {
        sal_gettime(&tv1);
    }

    tid = g_ctc_stress_interface->linkagg_grp_cnt;

    for (loop = 0; loop < group_number; loop++)
    {
        if (tid > CLI_STRESS_LINKAGG_GROUP_ID_MAX)
        {
            ctc_cli_out("%%tid exceed max\n");
            break;
        }

        sal_memset(&linkagg_grp, 0, sizeof(ctc_linkagg_group_t));
        linkagg_grp.tid = tid;
        linkagg_grp.linkagg_mode = group_type;
        linkagg_grp.flag = flag;

        switch (member_type)
        {
            case CLI_STRESS_LINKAGG_MEMBER_TYPE_DEFAULT:
            {
                linkagg_grp.member_num = _ctc_cli_stress_linkagg_get_member_number_default(group_type);
                break;
            }
            case CLI_STRESS_LINKAGG_MEMBER_TYPE_NUMBER:
            {
                linkagg_grp.member_num = member_number;
                break;
            }
            case CLI_STRESS_LINKAGG_MEMBER_TYPE_RANDOM:
            {
                linkagg_grp.member_num = _ctc_cli_stress_linkagg_get_member_number_random(group_type);
                break;
            }
            default:
            {
                linkagg_grp.member_num = _ctc_cli_stress_linkagg_get_member_number_default(group_type);
                break;
            }
        }

        ret = _ctc_cli_stress_linkagg_add_single_group(&linkagg_grp);

        g_ctc_stress_interface->linkagg_grp_cnt++;

        CTC_CLI_STRESS_CHECK_RETURN(ret, type, valid_count);

        if (CTC_E_NONE == ret)
        {
            valid_count++;
        }

        tid++;
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == type)
    {
        sal_gettime(&tv2);

        sal_sprintf(str_buf,"Linkagg Group Entry:%5d", group_number);
        output.buf = str_buf;
        output.op = CTC_CLI_STRESS_OP_TYPE_ADD;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("%%valid_count=%d \n", valid_count);

    return ret;
}

CTC_CLI(ctc_cli_stress_linkagg_remove_group,
        ctc_cli_stress_linkagg_remove_group_cmd,
        "linkagg remove group (all | number NUMBER | percentage PERCENTAGE)",
        CTC_CLI_LINKAGG_M_STR,
        "Reomve and uninstall",
        "Group",
        "All",
        "Number",
        "<1-64>",
        "Percentage",
        "<1-100>",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint32 index;
    uint8 remove_type;
    uint16 number = 0;
    uint16 percentage = 0;

    index = 0;
    if (0 == sal_strcmp(argv[index], "all"))
    {
        remove_type = CLI_STRESS_LINKAGG_REMOVE_TYPE_ALL;
        index++;
    }
    else if (0 == sal_strcmp(argv[index], "number"))
    {
        remove_type = CLI_STRESS_LINKAGG_REMOVE_TYPE_NUMBER;
        index++;

        CTC_CLI_GET_UINT16("number", number, argv[index]);
        index++;
    }
    else if (0 == sal_strcmp(argv[index], "percentage"))
    {
        remove_type = CLI_STRESS_LINKAGG_REMOVE_TYPE_PERCENTAGE;
        index++;

        CTC_CLI_GET_UINT16("percentage", percentage, argv[index]);
        index++;
    }
    else
    {
        ctc_cli_out("%% Unknown remove type %s\n", argv[index]);
        return CLI_ERROR;
    }

    if (CLI_STRESS_LINKAGG_REMOVE_TYPE_ALL == remove_type)
    {
        ret = _ctc_cli_stress_linkagg_remove_group_all();
    }
    else if (CLI_STRESS_LINKAGG_REMOVE_TYPE_NUMBER == remove_type)
    {
        ret = _ctc_cli_stress_linkagg_remove_group_number(number);
    }
    else if (CLI_STRESS_LINKAGG_REMOVE_TYPE_PERCENTAGE == remove_type)
    {
        ret = _ctc_cli_stress_linkagg_remove_group_percentage(percentage);
    }
    else
    {
        return CLI_SUCCESS;
    }

    return ret;
}

CTC_CLI(ctc_cli_stress_linkagg_add_group_random,
        ctc_cli_stress_linkagg_add_group_random_cmd,
        "linkagg add group "
        "group_type random "
        "entry_number (NUMBER | max | random) (member_number (NUMBER | random) |) (spec | stress | perf)",
        CTC_CLI_LINKAGG_M_STR,
        "Add and install",
        "Group",
        "Group Type",
        "Random group type",
        "Entry Number",
        "<1-64>",
        "Max number",
        "Random number",
        "Member Number",
        "<1-255>",
        "Random number",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    ctc_cli_stress_test_type_t test_type = CTC_CLI_STRESS_TEST_TYPE_MAX;
    uint32 group_number;
    uint8 member_type;
    uint32 member_number;
    uint32 index;

    index = 0;
    if (0 == sal_strcmp(argv[index], "max"))
    {
        group_number = CLI_STRESS_LINKAGG_GROUP_NUMBER;
        index++;
    }
    else if (0 == sal_strcmp(argv[index], "random"))
    {
        (void)_ctc_cli_stress_linkagg_rand(1, CLI_STRESS_LINKAGG_GROUP_NUMBER, &group_number);
        index++;
    }
    else
    {
        CTC_CLI_GET_UINT32("group_number", group_number, argv[index]);
        index++;
    }

    member_type = CLI_STRESS_LINKAGG_MEMBER_TYPE_DEFAULT;
    member_number = 0;
    if (0 == sal_strcmp(argv[index], "member_number"))
    {
        index++;

        if (0 == sal_strcmp(argv[index], "random"))
        {
            member_type = CLI_STRESS_LINKAGG_MEMBER_TYPE_RANDOM;
            index++;
        }
        else
        {
            member_type = CLI_STRESS_LINKAGG_MEMBER_TYPE_NUMBER;
            CTC_CLI_GET_UINT32("member_number", member_number, argv[index]);
            index++;
        }
    }

    if (0 == sal_strcmp(argv[index], "spec"))
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
        index++;
    }
    else if (0 == sal_strcmp(argv[index], "stress"))
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
        index++;
    }
    else if (0 == sal_strcmp(argv[index], "perf"))
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
        index++;
    }
    else
    {
        ctc_cli_out("%% Unknown type %s\n", argv[index]);
        return CLI_ERROR;
    }

    ret = _ctc_cli_stress_linkagg_add_random_group(group_number, member_type, member_number, test_type);

    return ret;
}

CTC_CLI(ctc_cli_stress_linkagg_add_group,
        ctc_cli_stress_linkagg_add_group_cmd,
        "linkagg add group "
        "group_type (static (local_first |) | failover (local_first |) | rr (random |) | dynamic | resilient) "
        "entry_number (NUMBER | max | random) (member_number (NUMBER | random) |) (spec | stress | perf)",
        CTC_CLI_LINKAGG_M_STR,
        "Add and install",
        "Group",
        "Group Type",
        "STATIC",
        "Local member first",
        "STATIC FAILOVER",
        "Local member first",
        "Round Robin",
        "Random member per round",
        "Dynamic",
        "Resilient",
        "Entry Number",
        "<1-64>",
        "Max number",
        "Random number",
        "Member Number",
        "<1-255>",
        "Random number",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    ctc_cli_stress_test_type_t test_type = CTC_CLI_STRESS_TEST_TYPE_MAX;
    uint8 group_type;
    uint16 flag;
    uint32 group_number;
    uint8 member_type;
    uint32 member_number;
    uint32 index;

    index = 0;
    flag = 0;
    if (0 == sal_strcmp(argv[index], "static"))
    {
        group_type = CTC_LINKAGG_MODE_STATIC;
        index++;

        if (0 == sal_strcmp(argv[index], "local_first"))
        {
            flag |= CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST;
            index++;
        }
    }
    else if (0 == sal_strcmp(argv[index], "failover"))
    {
        group_type = CTC_LINKAGG_MODE_STATIC_FAILOVER;
        index++;

        if (0 == sal_strcmp(argv[index], "local_first"))
        {
            flag |= CTC_LINKAGG_GROUP_FLAG_LOCAL_MEMBER_FIRST;
            index++;
        }
    }
    else if (0 == sal_strcmp(argv[index], "rr"))
    {
        group_type = CTC_LINKAGG_MODE_RR;
        index++;

        if (0 == sal_strcmp(argv[index], "random"))
        {
            flag |= CTC_LINKAGG_GROUP_FLAG_RANDOM_RR;
            index++;
        }
    }
    else if (0 == sal_strcmp(argv[index], "dynamic"))
    {
        group_type = CTC_LINKAGG_MODE_DLB;
        index++;
    }
    else if (0 == sal_strcmp(argv[index], "resilient"))
    {
        group_type = CTC_LINKAGG_MODE_RESILIENT;
        index++;
    }
    else
    {
        ctc_cli_out("%% Unknown group_type %s\n", argv[index]);
        return CLI_ERROR;
    }

    if (0 == sal_strcmp(argv[index], "max"))
    {
        group_number = CLI_STRESS_LINKAGG_GROUP_NUMBER;
        index++;
    }
    else if (0 == sal_strcmp(argv[index], "random"))
    {
        (void)_ctc_cli_stress_linkagg_rand(1, CLI_STRESS_LINKAGG_GROUP_NUMBER, &group_number);
        index++;
    }
    else
    {
        CTC_CLI_GET_UINT32("group_number", group_number, argv[index]);
        index++;
    }

    member_type = CLI_STRESS_LINKAGG_MEMBER_TYPE_DEFAULT;
    member_number = 0;
    if (0 == sal_strcmp(argv[index], "member_number"))
    {
        index++;

        if (0 == sal_strcmp(argv[index], "random"))
        {
            member_type = CLI_STRESS_LINKAGG_MEMBER_TYPE_RANDOM;
            index++;
        }
        else
        {
            member_type = CLI_STRESS_LINKAGG_MEMBER_TYPE_NUMBER;
            CTC_CLI_GET_UINT32("member_number", member_number, argv[index]);
            index++;
        }
    }

    if (0 == sal_strcmp(argv[index], "spec"))
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_SPEC;
        index++;
    }
    else if (0 == sal_strcmp(argv[index], "stress"))
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
        index++;
    }
    else if (0 == sal_strcmp(argv[index], "perf"))
    {
        test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
        index++;
    }
    else
    {
        ctc_cli_out("%% Unknown type %s\n", argv[index]);
        return CLI_ERROR;
    }

    ret = _ctc_cli_stress_linkagg_add_group(group_type, flag, group_number, member_type, member_number, test_type);

    return ret;
}


CTC_CLI(ctc_cli_port_set_snake_test_en,
        ctc_cli_port_set_snake_test_en_cmd,
        "port snake test (enable | disable) (lchip LCHIP|)",
        CTC_CLI_PORT_M_STR,
        "Snake",
        "Test",
        CTC_CLI_ENABLE,
        CTC_CLI_DISABLE,
        CTC_CLI_LCHIP_ID_STR,
        CTC_CLI_LCHIP_ID_VALUE)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;
    uint8 loop = 0;
    uint8 phy_port_id = 0;
    uint32 nh_id1 = 0;
    uint32 nh_id2 = 0;
    uint32 phy_port_switch = 0;
    uint8 port_serders_id = 0;
    uint8 port_serders_num = 0;
    uint8 lchip = 0;
    uint8 gchip = 0;
    uint32 gport[CTC_MAX_PHY_PORT] = {0};
    bool enable = 0;
    ctc_global_panel_ports_t phy_ports;
    ctc_port_serdes_info_t serdes_port;
    ctc_chip_serdes_loopback_t  lb_param;

    sal_memset(&phy_ports, 0, sizeof(phy_ports));
    sal_memset(&serdes_port, 0, sizeof(serdes_port));
    sal_memset(&lb_param, 0, sizeof(lb_param));

    index = CTC_CLI_GET_ARGC_INDEX("disable");
    if (0xFF != index)
    {
        enable = 0;
    }

    index = CTC_CLI_GET_ARGC_INDEX("enable");
    if (0xFF != index)
    {
        enable = 1;
    }
    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("lchip", lchip, argv[index + 1]);
    }

    phy_ports.lchip = lchip;
    if(g_ctcs_api_en)
    {
         ret = ctcs_global_ctl_get(g_api_lchip, CTC_GLOBAL_PANEL_PORTS, (void*)&phy_ports);
    }
    else
    {
        ret = ctc_global_ctl_get(CTC_GLOBAL_PANEL_PORTS, (void*)&phy_ports);
    }

    if (ret < 0 )
    {
        return CLI_ERROR;
    }

    for (loop = 0; loop < phy_ports.count; loop++)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_get_gchip_id(g_api_lchip, &gchip);
        }
        else
        {
            ret = ctc_get_gchip_id(lchip, &gchip);
        }

        if (ret < 0 )
        {
            return CLI_ERROR;
        }
        gport[loop] = CTC_MAP_LPORT_TO_GPORT(gchip, phy_ports.lport[loop]);
    }

    phy_port_switch = gport[phy_ports.count - 1];
    gport[phy_ports.count - 1] = gport[1];
    gport[1] = phy_port_switch;

    for (loop = 1; loop < phy_ports.count  ; loop++)
    {
        /*set the port cross connect expect the last port */
        if (g_ctcs_api_en)
        {
            ret = ctcs_nh_get_l2uc(g_api_lchip, gport[loop], CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC, &nh_id1);
        }
        else
        {
            ret = ctc_nh_get_l2uc(gport[loop], CTC_NH_PARAM_BRGUC_SUB_TYPE_BASIC, &nh_id1);
        }

        if (ret < 0 )
        {
            return CLI_ERROR;
        }
        phy_port_id = loop - 1;
        nh_id2 = enable ? nh_id1: 0xFFFFFFFF;

        if (g_ctcs_api_en)
        {
            ret = ctcs_port_set_cross_connect(g_api_lchip, gport[phy_port_id], nh_id2);
        }
        else
        {
            ret = ctc_port_set_cross_connect(gport[phy_port_id], nh_id2);
        }

        if (ret < 0 )
        {
            return CLI_ERROR;
        }

        /*enable internal lookback expect the first port and the last port*/
        if (loop == (phy_ports.count - 1))
        {
            break;
        }
        if(g_ctcs_api_en)
        {
            ret = ctcs_port_get_capability(g_api_lchip, gport[loop], CTC_PORT_CAP_TYPE_SERDES_INFO, (void*)&serdes_port);
        }
        else
        {
            ret = ctc_port_get_capability(gport[loop], CTC_PORT_CAP_TYPE_SERDES_INFO, (void*)&serdes_port);
        }
        if (ret < 0 )
        {
            return CLI_ERROR;
        }

        port_serders_id = serdes_port.serdes_id;
        for (port_serders_num = 0; port_serders_num < serdes_port.serdes_num ; port_serders_num++)
        {

            lb_param.enable =  enable ? 1 : 0;
            lb_param.mode = 0;
            lb_param.serdes_id = port_serders_id;
            if(g_ctcs_api_en)
            {
                ret = ctcs_chip_set_property(g_api_lchip, CTC_CHIP_PROP_SERDES_LOOPBACK, (void*)&lb_param);
            }
            else
            {
                ret = ctc_chip_set_property(lchip, CTC_CHIP_PROP_SERDES_LOOPBACK, (void*)&lb_param);
            }
            if (ret < 0 )
            {
                return CLI_ERROR;
            }
            port_serders_id++;
        }

    }

    return ret;
}

int32
ctc_stress_test_interface_cli_init(void)
{
    g_ctc_stress_interface = (ctc_cli_stress_test_interface_t*)sal_malloc(sizeof(ctc_cli_stress_test_interface_t));
    if (!g_ctc_stress_interface)
    {
        return CLI_ERROR;
    }

    sal_memset(g_ctc_stress_interface, 0, sizeof(ctc_cli_stress_test_interface_t));

    install_element(CTC_STRESS_MODE, &ctc_cli_stress_linkagg_add_group_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_linkagg_add_group_random_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_linkagg_remove_group_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_port_set_snake_test_en_cmd);

    return CLI_SUCCESS;
}

