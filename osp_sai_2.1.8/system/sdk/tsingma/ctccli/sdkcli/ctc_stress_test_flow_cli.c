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
#include "ctc_acl_cli.h"

#define CTC_CLI_ACL_ENTRY_TYPE_STR_STRESS "mac-entry| ipv4-entry (key-size (single|double)|)|ipv6-entry | mpls-entry \
    | pbr-ipv4-entry| pbr-ipv6-entry| ipv4-ext-entry\
    | mac-ipv4-entry| mac-ipv4-ext-entry| ipv6-ext-entry| mac-ipv6-entry\
    | cid-entry| ipfix-basic-entry| forward-entry| forward-ext-entry| copp-entry| copp-ext-entry"

#define CTC_CLI_ACL_ENTRY_TYPE_DESC_STRESS \
    "mac entry", "ipv4 entry","Key size for GG", "Single width for GG ","Double width for GG", "ipv6 entry", "mpls entry", \
    "pbr ipv4 entry  for GG", "pbr ipv6 entry  for GG", "ipv4 extend entry  for DT2", \
    "mac ipv4 entry for DT2", "mac ipv4 extend entry for DT2", "ipv6 extend entry for DT2", "mac ipv6 entry for DT2", \
    "cid entry for DT2", "ipfix basic entry for DT2", "forward entry for DT2", "forward extend entry for DT2", "copp entry for DT2", "copp extend entry for DT2"

#define CTC_CLI_ACL_KEY_TYPE_SET_MODE_0 \
do{\
    index = CTC_CLI_GET_ARGC_INDEX("mac-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key.type = CTC_ACL_KEY_MAC;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv4-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key.type = CTC_ACL_KEY_IPV4;\
        index = CTC_CLI_GET_ARGC_INDEX("key-size");\
        acl_entry.key.u.ipv4_key.key_size  = CTC_ACL_KEY_SIZE_SINGLE;\
        if (INDEX_VALID(index))\
        {\
            if (CLI_CLI_STR_EQUAL("double", (index+1)))\
            {\
                acl_entry.key.u.ipv4_key.key_size  = CTC_ACL_KEY_SIZE_DOUBLE;\
            }\
            else if (CLI_CLI_STR_EQUAL("single", (index+1)))\
            {\
                acl_entry.key.u.ipv4_key.key_size  = CTC_ACL_KEY_SIZE_SINGLE;\
            }\
        }\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("ipv6-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key.type = CTC_ACL_KEY_IPV6;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("mpls-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key.type = CTC_ACL_KEY_MPLS;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("pbr-ipv4-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key.type = CTC_ACL_KEY_PBR_IPV4;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("pbr-ipv6-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key.type = CTC_ACL_KEY_PBR_IPV6;\
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("hash-mac-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key.type = CTC_ACL_KEY_HASH_MAC;\
        gid = CTC_ACL_GROUP_ID_HASH_MAC;    \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("hash-ipv4-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key.type = CTC_ACL_KEY_HASH_IPV4;\
        gid = CTC_ACL_GROUP_ID_HASH_IPV4;   \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("hash-l2-l3-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key.type = CTC_ACL_KEY_HASH_L2_L3;\
        gid = CTC_ACL_GROUP_ID_HASH_L2_L3;  \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("hash-mpls-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key.type = CTC_ACL_KEY_HASH_MPLS;\
        gid = CTC_ACL_GROUP_ID_HASH_MPLS;   \
        break;\
    }\
    index = CTC_CLI_GET_ARGC_INDEX("hash-ipv6-entry");\
    if (0xFF != index) \
    {\
        acl_entry.key.type = CTC_ACL_KEY_HASH_IPV6;\
        gid = CTC_ACL_GROUP_ID_HASH_IPV6;   \
        break;\
    }\
}while(0);

enum ctc_cli_stress_entry_data_mode_e
{
    CTC_CLI_STRESS_ENTRY_DATA_MODE_INCREASE,
    CTC_CLI_STRESS_ENTRY_DATA_MODE_DESCEND,
    CTC_CLI_STRESS_ENTRY_DATA_MODE_RANDOM,
    CTC_CLI_STRESS_ENTRY_DATA_MODE_MAX
};
typedef enum ctc_cli_stress_entry_data_mode_e ctc_cli_stress_entry_data_mode_t;

struct ctc_cli_stress_acl_s
{
    uint8 entry_id_mode;   /* Entry id mode;       refer to ctc_cli_stress_entry_data_mode_t */
    uint8 entry_pri_mode;  /* Entry priority mode; refer to ctc_cli_stress_entry_data_mode_t */
};
typedef struct ctc_cli_stress_acl_s ctc_cli_stress_acl_t;

struct ctc_cli_stress_test_flow_s
{
    uint32* p_id[23];      /*p_id[0]~ p_id[7] is used for storing acl group id ingress, priority 0~7;
                             p_id[8]~ p_id[10] is used for storing acl group id egress, priority 0~2;
                             p_id[11]~ p_id[18] is used for storing acl entry id ingress, priority 0~7;
                             p_id[19]~ p_id[21] is used for storing acl entry id egress,  priority 0~2;
                             p_id[22] is used for storing acl hash entry id; */

    uint32 num[23];        /*num[0]~ num[7] is used for storing acl group num ingress, priority 0~7;
                             num[8]~ num[10] is used for storing acl group num egress, priority 0~2;
                             num[11]~ num[18] is used for storing acl entry num ingress, priority 0~7;
                             num[19]~ num[21] is used for storing acl entry num egress, priority 0~2;
                             num[22] is used for storing acl hash entry num; */

    uint32 group_id[11];   /*for Destroy the Group created by user assigned;
                             group_id[0]~ group_id[7] is used for storing acl group id ingress, priority 0~7;
                             group_id[8]~ group_id[10] is used for storing acl group id egress, priority 0~2;*/

    ctc_cli_stress_acl_t stress_acl;
    /* scl struct */
    uint32* p_group_id;
    uint32* p_entry_id;
    uint32  group_num;
    uint32  entry_num;
};
typedef struct ctc_cli_stress_test_flow_s ctc_cli_stress_test_flow_t;

ctc_cli_stress_test_flow_t  *g_ctc_stress_flow;


#define   ________ACL_STRESS________

STATIC int32
_ctc_cli_stress_acl_create_group(uint32 group_id, ctc_acl_group_info_t group_info,
                               ctc_cli_stress_t stress_param, uint32 group_num)
{
    int32  ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1,tv2;
    char str_buf[32] = {0};
    ctc_cli_stress_output_t output;

    sal_memset(&output, 0, sizeof(ctc_cli_stress_output_t));

    /*check ;otherwise  p_id[12] will overstep the boundary(now change other), it will result in coredump */
    if((group_info.priority >7) ||((CTC_EGRESS == group_info.dir) && (group_info.priority >2)))
    {
        ctc_cli_out("Invalid Param\n");
        return CLI_ERROR;
    }

    if((CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type) && (NULL == g_ctc_stress_flow->p_id[group_info.dir * 8 + group_info.priority]))
    {
        ctc_cli_out("Must create group first\n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress_param.test_type)
    {
        sal_gettime(&tv1);
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress_param.op_type)
    {
        g_ctc_stress_flow->num[group_info.dir * 8 + group_info.priority]= group_num;
        g_ctc_stress_flow->p_id[group_info.dir * 8 + group_info.priority] = (uint32*)sal_malloc(sizeof(uint32)*group_num);
        if(!g_ctc_stress_flow->p_id[group_info.dir * 8 + group_info.priority])
        {
            ctc_cli_out("Memory alloc error\n");
            return CLI_ERROR;
        }
    }

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
    {
        group_num = g_ctc_stress_flow->num[group_info.dir * 8 + group_info.priority];
    }

    if(group_num == 0)
    {
        ctc_cli_out("Invalid group num\n");
        return CLI_ERROR;
    }

    for (loop = 0; loop <= group_num - 1; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress_param.op_type)
        {
            if (CTC_CLI_STRESS_DATA_MODE_INCREASE == stress_param.data_mode)
            {
                /* key increase and action fix ,here only have group_id*/
                if (loop != 0)
                {
                    group_id = (group_id + 1);
                }
            }
            else if(CTC_CLI_STRESS_DATA_MODE_RANDOM == stress_param.data_mode)
            {
                /* key random and action fix,here only have group_id */
                group_id = sal_rand();
            }
            else if(CTC_CLI_STRESS_DATA_MODE_RANDOM_ALL == stress_param.data_mode)
            {
                ctc_cli_out("not support random all\n");
                return CLI_ERROR;
            }

      /*       sal_memcpy(g_ctc_stress_flow->p_id[group_info.dir * 8 + group_info.priority] + loop, &group_id, sizeof(uint32));*/
            *(g_ctc_stress_flow->p_id[group_info.dir * 8 + group_info.priority] + loop) = group_id;

            /* create group */
            if (g_ctcs_api_en)
            {
                ret = ctcs_acl_create_group(g_api_lchip, group_id, &group_info);
            }
            else
            {
                ret = ctc_acl_create_group(group_id, &group_info);
            }

        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
        {
            /* sal_memcpy(&group_id, g_ctc_stress_flow->p_id[group_info.dir * 8 + group_info.priority] + loop, sizeof(uint32));*/

            group_id = *(g_ctc_stress_flow->p_id[group_info.dir * 8 + group_info.priority] + loop);

            if(g_ctcs_api_en)
            {
                ret = ctcs_acl_destroy_group(g_api_lchip, group_id);
            }
            else
            {
                ret = ctc_acl_destroy_group(group_id);
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

        sal_sprintf(str_buf, "Group: %5d\n", group_num);
        output.buf = str_buf;
        output.op = stress_param.op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("Operate valid count: %d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
    {
        sal_free(g_ctc_stress_flow->p_id[group_info.dir * 8 + group_info.priority]);
        g_ctc_stress_flow->p_id[group_info.dir * 8 + group_info.priority] = NULL;
        g_ctc_stress_flow->num[group_info.dir * 8 + group_info.priority]= 0;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_stress_acl_create_group,
        ctc_cli_stress_acl_create_group_cmd,
        "acl create group GROUP_ID (priority PRIORITY |) direction (ingress|egress) (lchip LCHIP|) \
        (port-bitmap (pbmp3 PORT_BITMAP_3 PORT_BITMAP_2|) PORT_BITMAP_HIGH PORT_BITMAP_LOW| \
        global| vlan-class CLASS_ID | port-class CLASS_ID |service-id SERVICE_ID |pbr-class CLASS_ID| l3if-class CLASS_ID | gport GPORT | hash  | none ) \
        (random | incr | all-random) group-num NUMBER (spec | stress | perf) ",
        CTC_CLI_ACL_STR,
        "Create ACL group",
        CTC_CLI_ACL_GROUP_ID_STR,
        CTC_CLI_ACL_S_GROUP_ID_VALUE,
        CTC_CLI_ACL_GROUP_PRIO_STR,
        CTC_CLI_ACL_GROUP_PRIO_VALUE,
        "Direction",
        "Ingress",
        "Egress",
        "Lchip",
        CTC_CLI_LCHIP_ID_VALUE,
        CTC_CLI_ACL_PORT_BITMAP,
        "<3-0>",
        CTC_CLI_ACL_PORT_BITMAP_VALUE,
        CTC_CLI_ACL_PORT_BITMAP_VALUE,
        CTC_CLI_ACL_PORT_BITMAP_HIGH_VALUE,
        CTC_CLI_ACL_PORT_BITMAP_LOW_VALUE,
        "Global ACL, mask ports and vlans",
        "Vlan class ACL, against a class(group) of vlan",
        CTC_CLI_ACL_VLAN_CLASS_ID_VALUE,
        "Port class ACL, against a class(group) of port",
        CTC_CLI_ACL_PORT_CLASS_ID_VALUE,
        "Service ACL, against a service",
        CTC_CLI_ACL_SERVICE_ID_VALUE,
        "Pbr class ACL, against a class(group) of l3 source interface",
        CTC_CLI_ACL_PBR_CLASS_ID_VALUE,
        "L3if class acl",
        "l3if class id",
        "Port ACL, care gport",
        CTC_CLI_GPORT_ID_DESC,
        "Hash type",
        "None type",
        "Group id is random, other is fixed",
        "Group id is increase, other is fixed",
        "Group id is random, other is random",
        "Group num",
        "Number",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32  ret = CLI_SUCCESS;
    uint8  index = 0xFF;
    uint32 group_id = 0;
    uint32 group_num = 0;
    uint32 temp = 0;
    ctc_acl_group_info_t ginfo;
    ctc_cli_stress_t stress_param;
    ctc_port_bitmap_t pbmp = {0};

    sal_memset(&ginfo, 0, sizeof(ctc_acl_group_info_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    CTC_CLI_GET_UINT32("group id", group_id, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("group prioirty", ginfo.priority, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (INDEX_VALID(index))
    {
        ginfo.dir = CTC_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (INDEX_VALID(index))
    {
        ginfo.dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("lchip");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("lchip id", ginfo.lchip, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("port-bitmap");
    if (INDEX_VALID(index))
    {
        if (INDEX_VALID(CTC_CLI_GET_ARGC_INDEX("pbmp3")))
        {
            index = (CTC_CLI_GET_ARGC_INDEX("pbmp3"));
            CTC_CLI_GET_UINT32("port-bitmap 3", pbmp[3], argv[index + 1]);
            CTC_CLI_GET_UINT32("port-bitmap 2", pbmp[2], argv[index + 2]);
            CTC_CLI_GET_UINT32("port-bitmap 1", pbmp[1], argv[index + 3]);
            CTC_CLI_GET_UINT32("port-bitmap 0", pbmp[0], argv[index + 4]);

            cli_acl_port_bitmap_mapping(ginfo.un.port_bitmap, pbmp);
        }
        else
        {
            CTC_CLI_GET_UINT32("port-bitmap 1", pbmp[1], argv[index + 1]);
            CTC_CLI_GET_UINT32("port-bitmap 0", pbmp[0], argv[index + 2]);

            cli_acl_port_bitmap_mapping(ginfo.un.port_bitmap, pbmp);
        }

        ginfo.type  = CTC_ACL_GROUP_TYPE_PORT_BITMAP;

    }

    index = CTC_CLI_GET_ARGC_INDEX("global");
    if (INDEX_VALID(index))
    {
        ginfo.type  = CTC_ACL_GROUP_TYPE_GLOBAL;
    }

    index = CTC_CLI_GET_ARGC_INDEX("vlan-class");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("vlan class id", ginfo.un.vlan_class_id, argv[index + 1]);
        ginfo.type  = CTC_ACL_GROUP_TYPE_VLAN_CLASS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("port-class");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("port class id", ginfo.un.port_class_id, argv[index + 1]);
        ginfo.type  = CTC_ACL_GROUP_TYPE_PORT_CLASS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("service-id");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("service id", ginfo.un.service_id, argv[index + 1]);
        ginfo.type  = CTC_ACL_GROUP_TYPE_SERVICE_ACL;

    }

    index = CTC_CLI_GET_ARGC_INDEX("pbr-class");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("pbr class id", ginfo.un.pbr_class_id, argv[index + 1]);
        ginfo.type  = CTC_ACL_GROUP_TYPE_PBR_CLASS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("gport");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("gport", ginfo.un.gport, argv[index + 1]);
        ginfo.type  = CTC_ACL_GROUP_TYPE_PORT;
    }

    index = CTC_CLI_GET_ARGC_INDEX("none");
    if (INDEX_VALID(index))
    {
        ginfo.type  = CTC_ACL_GROUP_TYPE_NONE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("l3if-class");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT16("l3if class id", ginfo.un.l3if_class_id, argv[index + 1]);
        ginfo.type  = CTC_ACL_GROUP_TYPE_L3IF_CLASS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("hash");
    if (INDEX_VALID(index))
    {
        ginfo.type  = CTC_ACL_GROUP_TYPE_HASH;
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

    CTC_CLI_GET_UINT32("group-num", group_num, argv[temp + 1]);

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

    ret = _ctc_cli_stress_acl_create_group(group_id, ginfo, stress_param, group_num);

    return ret;
}

CTC_CLI(ctc_cli_stress_acl_destroy_group,
        ctc_cli_stress_acl_destroy_group_cmd,
        "acl destroy group (priority PRIORITY |) direction (ingress|egress) (perf |)",
        CTC_CLI_ACL_STR,
        "Destroy ACL group",
        CTC_CLI_ACL_GROUP_ID_STR,
        CTC_CLI_ACL_GROUP_PRIO_STR,
        CTC_CLI_ACL_GROUP_PRIO_VALUE,
        "Direction",
        "Ingress",
        "Egress",
        "Performance test")
{
    int32  ret = CTC_E_NONE;
    uint8  index = 0xFF;
    uint32 group_id = 0;
    uint32 group_num = 0;
    ctc_acl_group_info_t ginfo;
    ctc_cli_stress_t stress_param;

    sal_memset(&ginfo, 0, sizeof(ctc_acl_group_info_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    index = CTC_CLI_GET_ARGC_INDEX("priority");
    if (INDEX_VALID(index))
    {
        CTC_CLI_GET_UINT8("group prioirty", ginfo.priority, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (INDEX_VALID(index))
    {
        ginfo.dir = CTC_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (INDEX_VALID(index))
    {
        ginfo.dir = CTC_EGRESS;
    }

    stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_MAX;
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_acl_create_group(group_id, ginfo, stress_param, group_num);

    return ret;
}

STATIC int32
_ctc_cli_stress_acl_add_entry(uint32 group_id, uint8 group_pri, uint8 group_dir, ctc_acl_entry_t acl_entry,
                                   ctc_cli_stress_t stress_param, uint32 entry_num)
{
    int32  ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1,tv2;
    char str_buf[32] = {0};
    uint32 entry_id = 0;
    uint32 entry_pri = 0;
    ctc_cli_stress_output_t output;
    ctc_acl_group_info_t group_info;

    entry_id = acl_entry.entry_id;
    entry_pri = acl_entry.priority;
    sal_memset(&output, 0, sizeof(ctc_cli_stress_output_t));
    sal_memset(&group_info, 0 ,sizeof(ctc_acl_group_info_t));

    /*check ;otherwise  p_id[12] will overstep the boundary(now change other), it will result in coredump */
    if((group_pri >7) ||((CTC_EGRESS == group_dir) && (group_pri >2)))
    {
        ctc_cli_out("Invalid Param\n");
        return CLI_ERROR;
    }

    if((CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type) && (NULL == g_ctc_stress_flow->p_id[(group_dir * 8 + group_pri)+11]))
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
        group_info.dir = group_dir;
        group_info.lchip = 0;
        group_info.priority = group_pri;
        group_info.type = CTC_ACL_GROUP_TYPE_NONE;

        if(g_ctcs_api_en)
        {
            ret = ctcs_acl_create_group(g_api_lchip, group_id, &group_info);
        }
        else
        {
            ret = ctc_acl_create_group(group_id, &group_info);
        }

        if (ret < 0)
        {
            ctc_cli_out("Create group first Fail\n");
            ctc_cli_out("%% ret=%d, %s\n", ret, ctc_get_error_desc(ret));

        }

        g_ctc_stress_flow->num[(group_dir * 8 + group_pri)+11] = entry_num;
        g_ctc_stress_flow->p_id[(group_dir * 8 + group_pri)+11] = (uint32*)sal_malloc(sizeof(uint32)*entry_num);
        g_ctc_stress_flow->group_id[group_dir * 8 + group_pri] = group_id;
        if(!g_ctc_stress_flow->p_id[(group_dir * 8 + group_pri)+11])
        {
            ctc_cli_out("Memory alloc error\n");
            return CLI_ERROR;
        }
    }

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
    {
        entry_num = g_ctc_stress_flow->num[(group_dir * 8 + group_pri)+11] ;
        group_id = g_ctc_stress_flow->group_id[group_dir * 8 + group_pri] ;
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
            /* entry id */
            if (CTC_CLI_STRESS_ENTRY_DATA_MODE_INCREASE == g_ctc_stress_flow->stress_acl.entry_id_mode)
            {
              /* entry id increase */
                if (loop != 0)
                {
                    entry_id = (entry_id + 1) % (0xFFFFFFFF);
                }
            }
            else if(CTC_CLI_STRESS_ENTRY_DATA_MODE_DESCEND == g_ctc_stress_flow->stress_acl.entry_id_mode)
            {
             /* entry id descend */
                if(loop != 0)
                {
                    if((entry_id - 1) < 0)
                    {
                        entry_id = 0xFFFFFFFF;
                    }
                    entry_id = entry_id - 1;
                }
            }
            else if(CTC_CLI_STRESS_ENTRY_DATA_MODE_RANDOM == g_ctc_stress_flow->stress_acl.entry_id_mode)
            {
                /* entry id random */
                entry_id = sal_rand() % (0xFFFFFFFF);
            }

            /* entry priority */
            if (CTC_CLI_STRESS_ENTRY_DATA_MODE_INCREASE == g_ctc_stress_flow->stress_acl.entry_pri_mode)
            {
              /* entry priority increase */
                if (loop != 0)
                {
                    entry_pri = (entry_pri + 1) % (0xFFFFFFFF);
                }
            }
            else if(CTC_CLI_STRESS_ENTRY_DATA_MODE_DESCEND == g_ctc_stress_flow->stress_acl.entry_pri_mode)
            {
             /* entry priority descend */
                if(loop != 0)
                {
                    if((entry_pri - 1) < 0)
                    {
                        entry_pri = 0xFFFFFFFF;
                    }
                    entry_pri = entry_pri - 1;
                }
            }
            else if(CTC_CLI_STRESS_ENTRY_DATA_MODE_RANDOM == g_ctc_stress_flow->stress_acl.entry_pri_mode)
            {
                /* entry priority random */
                entry_pri = sal_rand() % (0xFFFFFFFF);
            }

            acl_entry.entry_id = entry_id;
            acl_entry.priority= entry_pri;

            sal_memcpy(g_ctc_stress_flow->p_id[(group_dir * 8 + group_pri)+11]  + loop, &entry_id, sizeof(uint32));

            /* add entry */
            if (g_ctcs_api_en)
            {
                ret = ctcs_acl_add_entry(g_api_lchip, group_id, &acl_entry);
            }
            else
            {
                ret = ctc_acl_add_entry(group_id, &acl_entry);
            }

        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
        {
            sal_memcpy(&entry_id,  g_ctc_stress_flow->p_id[(group_dir * 8 + group_pri)+11]  + loop, sizeof(uint32));

            if(g_ctcs_api_en)
            {
                ret = ctcs_acl_remove_entry(g_api_lchip, entry_id);
            }
            else
            {
                ret = ctc_acl_remove_entry(entry_id);
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

        sal_sprintf(str_buf, "Group: %5d\n", entry_num);
        output.buf = str_buf;
        output.op = stress_param.op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("Operate valid count: %d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
    {
        if(g_ctcs_api_en)
        {
            ret = ctcs_acl_destroy_group(g_api_lchip, group_id);
        }
        else
        {
            ret = ctc_acl_destroy_group(group_id);
        }

        if (ret == CTC_E_NONE)
        {
            ctc_cli_out("Destroy the Group created by user assigned, group id:  %u \n", group_id);
        }
        else
        {
            ctc_cli_out("Destroy the Group created by user assigned, group id: %d error: %s\n", group_id, ctc_get_error_desc(ret));
        }

        sal_free(g_ctc_stress_flow->p_id[(group_dir * 8 + group_pri)+11]);
        g_ctc_stress_flow->p_id[(group_dir * 8 + group_pri)+11] = NULL;
        g_ctc_stress_flow->num[(group_dir * 8 + group_pri)+11] = 0;
        g_ctc_stress_flow->group_id[(group_dir * 8 + group_pri)] = 0;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stress_acl_add_entry,
        ctc_cli_stress_acl_add_entry_cmd,
        "acl add group GROUP_ID (group-priority GROUP_PRI|) group-direction (ingress|egress) (entry ENTRY_ID)(incr-id | dec-id | random-id)(entry-priority PRIORITY(incr-pri |dec-pri | random-pri)|)  \
        (field-mode |)(" CTC_CLI_ACL_ENTRY_TYPE_STR_STRESS ")(entry-num NUMBER) (spec | stress | perf) ",
        CTC_CLI_ACL_STR,
        "Add one entry to software table",
        CTC_CLI_ACL_GROUP_ID_STR,
        CTC_CLI_ACL_B_GROUP_ID_VALUE,
        CTC_CLI_ACL_GROUP_PRIO_STR,
        CTC_CLI_ACL_GROUP_PRIO_VALUE,
        "Direction",
        "Ingress",
        "Egress",
        CTC_CLI_ACL_ENTRY_ID_STR,
        CTC_CLI_ACL_ENTRY_ID_VALUE,
        "Entry ID is increase",
        "Entry ID is decsend",
        "Entry ID is random",
        CTC_CLI_ACL_ENTRY_PRIO_STR,
        CTC_CLI_ACL_ENTRY_PRIO_VALUE,
        "Entry Priority is increase",
        "Entry Priority is decsend",
        "Entry Priority is random",
        "Use key and action field to add entry",
        CTC_CLI_ACL_ENTRY_TYPE_DESC_STRESS,
        "Entry num",
        "Number",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32  ret = CLI_SUCCESS;
    uint8  index = 0xFF;
    uint32 gid = 0;
    uint8  group_pri= 0;
    uint8  group_dir= 0;
    uint32 entry_num = 0;
    ctc_acl_entry_t acl_entry;
    ctc_cli_stress_t stress_param;

    sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    CTC_CLI_GET_UINT32("group id", gid, argv[0]);

    index = CTC_CLI_GET_ARGC_INDEX("group-priority");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("group priority", group_pri, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (INDEX_VALID(index))
    {
        group_dir = CTC_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (INDEX_VALID(index))
    {
        group_dir = CTC_EGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("entry");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("entry id", acl_entry.entry_id, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("entry-priority");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("entry-priority", acl_entry.priority, argv[index + 1]);
        acl_entry.priority_valid = 1;
    }

    index = CTC_CLI_GET_ARGC_INDEX("field-mode ");
    if (0xFF != index)
    {
        acl_entry.mode = 1;
    }

    if( 1 == acl_entry.mode)
    {
        CTC_CLI_ACL_KEY_TYPE_SET
    }
    else
    {
        CTC_CLI_ACL_KEY_TYPE_SET_MODE_0
    }

    index = CTC_CLI_GET_ARGC_INDEX("incr-id");
    if (0xFF != index)
    {
        g_ctc_stress_flow->stress_acl.entry_id_mode = CTC_CLI_STRESS_ENTRY_DATA_MODE_INCREASE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dec-id");
    if (0xFF != index)
    {
        g_ctc_stress_flow->stress_acl.entry_id_mode = CTC_CLI_STRESS_ENTRY_DATA_MODE_DESCEND;
    }

    index = CTC_CLI_GET_ARGC_INDEX("random-id");
    if (0xFF != index)
    {
        g_ctc_stress_flow->stress_acl.entry_id_mode = CTC_CLI_STRESS_ENTRY_DATA_MODE_RANDOM;
    }

    index = CTC_CLI_GET_ARGC_INDEX("incr-pri");
    if (0xFF != index)
    {
        g_ctc_stress_flow->stress_acl.entry_pri_mode = CTC_CLI_STRESS_ENTRY_DATA_MODE_INCREASE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dec-pri");
    if (0xFF != index)
    {
        g_ctc_stress_flow->stress_acl.entry_pri_mode = CTC_CLI_STRESS_ENTRY_DATA_MODE_DESCEND;
    }

    index = CTC_CLI_GET_ARGC_INDEX("random-pri");
    if (0xFF != index)
    {
        g_ctc_stress_flow->stress_acl.entry_pri_mode = CTC_CLI_STRESS_ENTRY_DATA_MODE_RANDOM;
    }

    index = CTC_CLI_GET_ARGC_INDEX("entry-num");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("entry-num", entry_num, argv[index + 1]);
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

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;

    ret = _ctc_cli_stress_acl_add_entry(gid,group_pri,group_dir,acl_entry,
                                        stress_param, entry_num);

    return ret;
}

CTC_CLI(ctc_cli_stress_acl_remove_entry,
        ctc_cli_stress_acl_remove_entry_cmd,
        "acl remove entry (group-priority GROUP_PRI|) group-direction (ingress|egress) (perf |)",
        CTC_CLI_ACL_STR,
        "Remove one entry from software table",
        CTC_CLI_ACL_ENTRY_ID_STR, "Group priority", "Group direction", "Performance test")
{
    int32  ret = CLI_SUCCESS;
    uint8  index = 0xFF;
    uint32 group_id = 0;
    uint8  group_pri = 0;
    uint8  group_dir= 0;
    uint32 entry_num = 0;
    ctc_acl_entry_t acl_entry;
    ctc_cli_stress_t stress_param;

    sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    index = CTC_CLI_GET_ARGC_INDEX("group-priority");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("group priority", group_pri, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("ingress");
    if (INDEX_VALID(index))
    {
        group_dir = CTC_INGRESS;
    }

    index = CTC_CLI_GET_ARGC_INDEX("egress");
    if (INDEX_VALID(index))
    {
        group_dir = CTC_EGRESS;
    }

    stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_MAX;
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_acl_add_entry(group_id,group_pri,group_dir,acl_entry,
                                        stress_param, entry_num);

    return ret;

}

STATIC int32
_ctc_cli_stress_acl_add_hash_entry(uint32 group_id, ctc_acl_entry_t acl_entry, ctc_cli_stress_t stress_param, uint32 entry_num)
{
    int32  ret = CLI_SUCCESS;
    uint32 loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1,tv2;
    char str_buf[32] = {0};
    uint32 entry_id = 0;
    ctc_cli_stress_output_t output;
    ctc_acl_group_info_t group_info;

    entry_id = acl_entry.entry_id;

    sal_memset(&output, 0, sizeof(ctc_cli_stress_output_t));
    sal_memset(&group_info, 0 ,sizeof(ctc_acl_group_info_t));

    if((CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type) && (NULL == g_ctc_stress_flow->p_id[22]))
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
        g_ctc_stress_flow->num[22] = entry_num;
        g_ctc_stress_flow->p_id[22] = (uint32*)sal_malloc(sizeof(uint32)*entry_num);
        if(! g_ctc_stress_flow->p_id[22])
        {
            ctc_cli_out("Memory alloc error\n");
            return CLI_ERROR;
        }
    }

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
    {
        entry_num = g_ctc_stress_flow->num[22] ;
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
            /* entry id */
            if (CTC_CLI_STRESS_ENTRY_DATA_MODE_INCREASE == g_ctc_stress_flow->stress_acl.entry_id_mode)
            {
                /* entry id increase */
                if (loop != 0)
                {
                    entry_id = (entry_id + 1) % (0xFFFFFFFF);
                }
            }
            else if(CTC_CLI_STRESS_ENTRY_DATA_MODE_DESCEND == g_ctc_stress_flow->stress_acl.entry_id_mode)
            {
                /* entry id descend */
                if (loop != 0)
                {
                    if (entry_id < 1)
                    {
                        entry_id = 0xFFFFFFFF;
                    }
                    entry_id = entry_id - 1;
                }
            }
            else if(CTC_CLI_STRESS_ENTRY_DATA_MODE_RANDOM == g_ctc_stress_flow->stress_acl.entry_id_mode)
            {
                /* entry id random */
                entry_id = sal_rand() % (0xFFFFFFFF);
            }

            acl_entry.entry_id = entry_id;

            sal_memcpy(g_ctc_stress_flow->p_id[22]  + loop, &entry_id, sizeof(uint32));

            /* add different key fot gg gb*/
            switch (acl_entry.key.type)
            {
                case CTC_ACL_KEY_HASH_MAC:
                    acl_entry.key.u.hash_mac_key.mac_sa[0] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_mac_key.mac_sa[1] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_mac_key.mac_sa[2] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_mac_key.mac_sa[3] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_mac_key.mac_sa[4] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_mac_key.mac_sa[5] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_mac_key.mac_da[0] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_mac_key.mac_da[1] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_mac_key.mac_da[2] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_mac_key.mac_da[3] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_mac_key.mac_da[4] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_mac_key.mac_da[5] = sal_rand() % (0xFF);
                    break;
                case CTC_ACL_KEY_HASH_IPV4:
                    acl_entry.key.u.hash_ipv4_key.ip_sa = sal_rand() % 0xffffffff;
                    acl_entry.key.u.hash_ipv4_key.ip_da = ~(sal_rand() % 0xffffffff);
                    acl_entry.key.u.hash_ipv4_key.dscp = sal_rand() % 64;
                    break;
                case CTC_ACL_KEY_HASH_IPV6:
                    acl_entry.key.u.hash_ipv6_key.ip_sa[0] = sal_rand() % 0xffffffff;
                    acl_entry.key.u.hash_ipv6_key.ip_sa[1] = sal_rand() % 0xffffffff;
                    acl_entry.key.u.hash_ipv6_key.ip_sa[2] = sal_rand() % 0xffffffff;
                    acl_entry.key.u.hash_ipv6_key.ip_sa[3] = sal_rand() % 0xffffffff;
                    acl_entry.key.u.hash_ipv6_key.ip_da[0] = sal_rand() % 0xffffffff;
                    acl_entry.key.u.hash_ipv6_key.ip_da[1] = sal_rand() % 0xffffffff;
                    acl_entry.key.u.hash_ipv6_key.ip_da[2] = sal_rand() % 0xffffffff;
                    acl_entry.key.u.hash_ipv6_key.ip_da[3] = sal_rand() % 0xffffffff;
                    acl_entry.key.u.hash_ipv6_key.dscp = sal_rand() % 64;
                    break;
                case CTC_ACL_KEY_HASH_L2_L3:
                    acl_entry.key.u.hash_l2_l3_key.mac_sa[0] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_l2_l3_key.mac_sa[0] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_l2_l3_key.mac_sa[0] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_l2_l3_key.mac_sa[0] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_l2_l3_key.mac_sa[0] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_l2_l3_key.mac_sa[0] = sal_rand() % (0xFF);
                    acl_entry.key.u.hash_l2_l3_key.ip_sa = sal_rand() % 0xFFFFFFFF;
                    break;
                case CTC_ACL_KEY_HASH_MPLS:
                    acl_entry.key.u.hash_mpls_key.mpls_label0_label = sal_rand() % 0xFFFFF;
                    acl_entry.key.u.hash_mpls_key.mpls_label0_exp = sal_rand() & 0x7;
                    acl_entry.key.u.hash_mpls_key.mpls_label0_s = sal_rand() % 2;
                    acl_entry.key.u.hash_mpls_key.mpls_label0_ttl = sal_rand() & 0xff;
                    break;
                default:
                    break;
            }


            /* add entry */
            if (g_ctcs_api_en)
            {
                ret = ctcs_acl_add_entry(g_api_lchip, group_id, &acl_entry);
            }
            else
            {
                ret = ctc_acl_add_entry(group_id, &acl_entry);
            }

        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
        {
            sal_memcpy(&entry_id,  g_ctc_stress_flow->p_id[22] + loop, sizeof(uint32));

            if(g_ctcs_api_en)
            {
                ret = ctcs_acl_remove_entry(g_api_lchip, entry_id);
            }
            else
            {
                ret = ctc_acl_remove_entry(entry_id);
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

        sal_sprintf(str_buf, "Group: %5d\n", entry_num);
        output.buf = str_buf;
        output.op = stress_param.op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("Operate valid count: %d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
    {
        sal_free(g_ctc_stress_flow->p_id[22]);
        g_ctc_stress_flow->p_id[22] = NULL;
        g_ctc_stress_flow->num[22] = 0;
    }

    return CLI_SUCCESS;
}


CTC_CLI(ctc_cli_stress_acl_add_hash_entry,
        ctc_cli_stress_acl_add_hash_entry_cmd,
        "acl add hash-entry (field-mode |) (" CTC_CLI_ACL_HASH_ENTRY_TYPE_STR ") (entry ENTRY_ID)(incr-id | dec-id | random-id)\
        (field-sel-id SEL_ID) (entry-num NUMBER) (spec | stress | perf) ",
        CTC_CLI_ACL_STR,
        "Add one entry to software table",
        "Hash entry",
        "Use key and action field to add entry",
        CTC_CLI_ACL_HASH_ENTRY_TYPE_DESC,
        CTC_CLI_ACL_ENTRY_ID_STR,
        CTC_CLI_ACL_ENTRY_ID_VALUE,
        "Entry ID is increase",
        "Entry ID is decsend",
        "Entry ID is random",
        "Hash key field select id ",
        "Field select ID",
        "Entry num",
        "Number",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32  ret = CLI_SUCCESS;
    uint8  index = 0xFF;
    uint32 gid = 0;
    uint32 entry_num = 0;
    ctc_acl_entry_t acl_entry;
    ctc_cli_stress_t stress_param;

    sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    index = CTC_CLI_GET_ARGC_INDEX("entry");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("entry id", acl_entry.entry_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("incr-id");
    if (0xFF != index)
    {
        g_ctc_stress_flow->stress_acl.entry_id_mode = CTC_CLI_STRESS_ENTRY_DATA_MODE_INCREASE;
    }

    index = CTC_CLI_GET_ARGC_INDEX("dec-id");
    if (0xFF != index)
    {
        g_ctc_stress_flow->stress_acl.entry_id_mode = CTC_CLI_STRESS_ENTRY_DATA_MODE_DESCEND;
    }

    index = CTC_CLI_GET_ARGC_INDEX("random-id");
    if (0xFF != index)
    {
        g_ctc_stress_flow->stress_acl.entry_id_mode = CTC_CLI_STRESS_ENTRY_DATA_MODE_RANDOM;
    }

    index = CTC_CLI_GET_ARGC_INDEX("field-mode ");
    if (0xFF != index)
    {
        acl_entry.mode = 1;
    }

    if( 1 == acl_entry.mode)
    {
        CTC_CLI_ACL_KEY_TYPE_SET
    }
    else
    {
        CTC_CLI_ACL_KEY_TYPE_SET_MODE_0
    }

    index = CTC_CLI_GET_ARGC_INDEX("field-sel-id");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("field-sel-id", acl_entry.hash_field_sel_id, argv[index + 1]);
    }

    index = CTC_CLI_GET_ARGC_INDEX("entry-num");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT32("entry-num", entry_num, argv[index + 1]);
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

    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_ADD;
    ret = _ctc_cli_stress_acl_add_hash_entry(gid, acl_entry,stress_param, entry_num);

    return ret;
}

CTC_CLI(ctc_cli_stress_acl_remove_hash_entry,
        ctc_cli_stress_acl_remove_hash_entry_cmd,
        "acl remove hash-entry (" CTC_CLI_ACL_HASH_ENTRY_TYPE_STR ") (perf|)",
        CTC_CLI_ACL_STR,
        "Remove one entry from software table",
        "Hash entry",
        CTC_CLI_ACL_HASH_ENTRY_TYPE_DESC, "Group priority", "Group direction", "Performance test")
{
    int32  ret = CLI_SUCCESS;
    uint32 entry_num = 0;
    uint32 gid = 0;
    uint8  index = 0xFF;
    ctc_acl_entry_t acl_entry;
    ctc_cli_stress_t stress_param;

    sal_memset(&acl_entry, 0, sizeof(ctc_acl_entry_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_STRESS;
    stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_MAX;
    index = CTC_CLI_GET_ARGC_INDEX("perf");
    if (0xFF != index)
    {
        stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_PERF;
    }

    ret = _ctc_cli_stress_acl_add_hash_entry(gid, acl_entry,stress_param, entry_num);

    return ret;

}


#define   __SCL_STRESS_TEST__

#define SCL_ENTRY_KEY_TYPE \
"| ipv4-hash-entry \
 | ipv6-hash-entry \
 | mac-hash-entry \
 | port-cvlan-cos-hash-entry \
 | port-cvlan-hash-entry \
 | port-double-vlan-hash-entry \
 | port-hash-entry \
 | port-ipv4-hash-entry \
 | port-ipv6-hash-entry \
 | port-mac-hash-entry \
 | port-svlan-cos-hash-entry \
 | port-svlan-hash-entry \
 | port-cross-hash-entry \
 | mac-entry \
 | ipv4-entry \
 | ipv4-single-entry \
 | ipv6-entry \
 | ipv6-single-entry \
 | vlan-entry"

#define SCL_ENTRY_KEY_TYPE_DESC \
"Ipv4 hash entry", \
"Ipv6 hash entry", \
"Mac hash entry", \
"Port cvlan cos hash entry", \
"Port cvlan hash entry", \
"Port double vlan hash entry", \
"Port hash entry", \
"Port ipv4 hash entry", \
"Port ipv6 hash entry", \
"Port mac hash entry", \
"Port svlan cos hash entry", \
"Port svlan hash entry", \
"Port cross hash entry", \
"Mac tcam entry", \
"Ipv4 tcam entry", \
"Ipv4 single tcam entry", \
"Ipv6 tcam entry", \
"Ipv6 single tcam entry", \
"Vlan tcam entry"


STATIC int32
_ctc_cli_stress_scl_group(uint32 group_id, ctc_scl_group_info_t group_info, ctc_cli_stress_t stress_param, uint32 group_num)
{
    int32 ret = CLI_SUCCESS;
    uint32  loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1,tv2;
    char str_buf[32] = {0};
    ctc_cli_stress_output_t output;

    if(CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type && g_ctc_stress_flow->p_group_id == NULL)
    {
        ctc_cli_out("Must create group first\n");
        return CLI_ERROR;
    }

    if (CTC_CLI_STRESS_TEST_TYPE_PERF == stress_param.test_type)
    {
        sal_gettime(&tv1);
    }

    if (CTC_CLI_STRESS_OP_TYPE_ADD == stress_param.op_type)
    {
        if (g_ctc_stress_flow->p_group_id)
        {
            /* must remove the added group before adding the new group */
            ctc_cli_out("Must remove group first\n");
            return CLI_ERROR;
        }
        else
        {
            g_ctc_stress_flow->p_group_id = (uint32*)sal_malloc(sizeof(uint32) * group_num);
            if (NULL == g_ctc_stress_flow->p_group_id)
            {
                ctc_cli_out("Memory alloc error\n");
                return CLI_ERROR;
            }
            sal_memset(g_ctc_stress_flow->p_group_id, 0, sizeof(uint32) * group_num);
            g_ctc_stress_flow->group_num = group_num;
        }
    }

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
    {
        group_num = g_ctc_stress_flow->group_num;
    }

    if(group_num == 0)
    {
        ctc_cli_out("Invalid group num\n");
        return CLI_ERROR;
    }

    for (loop = 0; loop <= group_num - 1; loop++)
    {
        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress_param.op_type)
        {
            if (CTC_CLI_STRESS_DATA_MODE_INCREASE == stress_param.data_mode)
            {
                /* key increase */
                if (loop != 0)
                {
                    group_id += 1;
                }
            }
            else if(CTC_CLI_STRESS_DATA_MODE_RANDOM == stress_param.data_mode)
            {
                /* key random */
                group_id = sal_rand();
            }

            /* add tunnel */
            *(g_ctc_stress_flow->p_group_id + loop) = group_id;

            if (g_ctcs_api_en)
            {
                ret = ctcs_scl_create_group(g_api_lchip, group_id, &group_info);
            }
            else
            {
                ret = ctc_scl_create_group(group_id, &group_info);
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
            group_id = *(g_ctc_stress_flow->p_group_id + loop);

            if(g_ctcs_api_en)
            {
                ret = ctcs_scl_destroy_group(g_api_lchip, group_id);
            }
            else
            {
                ret = ctc_scl_destroy_group(group_id);
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

        sal_sprintf(str_buf, "Entry: %5d\n", group_num);
        output.buf = str_buf;
        output.op = stress_param.op_type;
        output.tv_start = tv1;
        output.tv_end = tv2;
        ctc_cli_stress_test_output(&output);
    }

    ctc_cli_out("Operate valid count: %d \n", valid_count);

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
    {
        sal_free(g_ctc_stress_flow->p_group_id);
        g_ctc_stress_flow->p_group_id = NULL;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_cli_stress_scl_entry(uint32 group_id, ctc_scl_entry_t scl_entry, ctc_cli_stress_t stress_param, uint32 entry_num)
{
    uint16 svlan = 0;
    uint16 cvlan = 0;
    uint8 stagcos = 0;
    uint8 ctagcos = 0;
    int32 ret = CLI_SUCCESS;
    uint32  loop = 0;
    uint32 valid_count =0;
    sal_systime_t tv1,tv2;
    char str_buf[32] = {0};
    ctc_cli_stress_output_t output;
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;
    uint32 mac_temp = 0;
    mac_addr_t mac;
    mac_addr_t mac_mask;
    uint32 ip_sa = 0;
    ipv6_addr_t ipv6_sa;
    ipv6_addr_t ipv6_sa_mask;

    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));
    sal_memset(&field_port, 0, sizeof(ctc_field_port_t));
    sal_memset(&mac, 0, sizeof(mac_addr_t));
    sal_memset(&mac_mask, 0xFF, sizeof(mac_addr_t));
    sal_memset(&ipv6_sa, 0, sizeof(ipv6_addr_t));
    sal_memset(&ipv6_sa_mask, 0xFF, sizeof(ipv6_addr_t));

    if(CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type && g_ctc_stress_flow->p_entry_id == NULL)
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
        if (g_ctc_stress_flow->p_entry_id)
        {
            ctc_cli_out("Must remove entry first\n");
            return CLI_ERROR;
        }
        else
        {
            g_ctc_stress_flow->p_entry_id = (uint32*)sal_malloc(sizeof(uint32) * entry_num);
            if (NULL == g_ctc_stress_flow->p_entry_id)
            {
                ctc_cli_out("Memory alloc error\n");
                return CLI_ERROR;
            }
            sal_memset(g_ctc_stress_flow->p_entry_id, 0, sizeof(uint32) * entry_num);
            g_ctc_stress_flow->entry_num = entry_num;
        }
    }

    if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
    {
        entry_num = g_ctc_stress_flow->entry_num;
    }

    if(entry_num == 0)
    {
        ctc_cli_out("Invalid entry num\n");
        return CLI_ERROR;
    }

    for (loop = 0; loop <= entry_num - 1; loop++)
    {
        /* process entry priority per loop */
        if (CTC_CLI_STRESS_PRIORITY_MODE_FIXED == stress_param.priority_mode)
        {
            scl_entry.priority = scl_entry.priority;
        }
        else if (CTC_CLI_STRESS_PRIORITY_MODE_INCREASE == stress_param.priority_mode)
        {
            if (0xFFFFFFFF != scl_entry.priority)
            {
                scl_entry.priority += 1;
            }
        }
        else if (CTC_CLI_STRESS_PRIORITY_MODE_DECREASE == stress_param.priority_mode)
        {
            if (0 != scl_entry.priority)
            {
                scl_entry.priority -= 1;
            }
        }
        else if (CTC_CLI_STRESS_PRIORITY_MODE_RANDOM == stress_param.priority_mode)
        {
            scl_entry.priority = sal_rand();
        }

        if (CTC_CLI_STRESS_OP_TYPE_ADD == stress_param.op_type)
        {
            if (CTC_CLI_STRESS_DATA_MODE_INCREASE == stress_param.data_mode)
            {
                /* key increase */
                if (loop != 0)
                {
                    scl_entry.entry_id += 1;
                }
                if(!scl_entry.mode)
                {
                    switch(scl_entry.key.type)
                    {
                        case CTC_SCL_KEY_HASH_PORT:
                            scl_entry.key.u.hash_port_key.gport = (scl_entry.key.u.hash_port_key.gport + 1) % 256;
                            break;
                        case CTC_SCL_KEY_HASH_PORT_CVLAN:
                            scl_entry.key.u.hash_port_cvlan_key.gport = (scl_entry.key.u.hash_port_cvlan_key.gport + 1) % 256;
                            scl_entry.key.u.hash_port_cvlan_key.cvlan = (scl_entry.key.u.hash_port_cvlan_key.cvlan + 1) % 4095 + 1;
                            break;
                        case CTC_SCL_KEY_HASH_PORT_SVLAN:
                            scl_entry.key.u.hash_port_svlan_key.gport = (scl_entry.key.u.hash_port_svlan_key.gport + 1) % 256;
                            scl_entry.key.u.hash_port_svlan_key.svlan = (scl_entry.key.u.hash_port_svlan_key.svlan + 1) % 4095 + 1;
                            break;
                        case CTC_SCL_KEY_HASH_PORT_2VLAN:
                            scl_entry.key.u.hash_port_2vlan_key.gport = (scl_entry.key.u.hash_port_2vlan_key.gport + 1) % 256;
                            scl_entry.key.u.hash_port_2vlan_key.svlan = (scl_entry.key.u.hash_port_2vlan_key.svlan + 1) % 4095 + 1;
                            scl_entry.key.u.hash_port_2vlan_key.cvlan = (scl_entry.key.u.hash_port_2vlan_key.cvlan + 1) % 4095 + 1;
                            break;
                        case CTC_SCL_KEY_HASH_PORT_CVLAN_COS:
                            scl_entry.key.u.hash_port_cvlan_cos_key.gport = (scl_entry.key.u.hash_port_cvlan_cos_key.gport + 1) % 256;
                            scl_entry.key.u.hash_port_cvlan_cos_key.cvlan = (scl_entry.key.u.hash_port_cvlan_cos_key.cvlan + 1) % 4095 + 1;
                            scl_entry.key.u.hash_port_cvlan_cos_key.ctag_cos = (scl_entry.key.u.hash_port_cvlan_cos_key.ctag_cos + 1) % 8;
                            break;
                        case CTC_SCL_KEY_HASH_PORT_SVLAN_COS:
                            scl_entry.key.u.hash_port_svlan_cos_key.gport = (scl_entry.key.u.hash_port_svlan_cos_key.gport + 1) % 256;
                            scl_entry.key.u.hash_port_svlan_cos_key.svlan = (scl_entry.key.u.hash_port_svlan_cos_key.svlan + 1) % 4095 + 1;
                            scl_entry.key.u.hash_port_svlan_cos_key.stag_cos = (scl_entry.key.u.hash_port_svlan_cos_key.stag_cos + 1) % 8;
                            break;
                        case CTC_SCL_KEY_HASH_MAC:
                            mac_temp += 1;
                            scl_entry.key.u.hash_mac_key.mac[5] = (mac_temp & 0xFF);
                            scl_entry.key.u.hash_mac_key.mac[4] = ((mac_temp >> 8)  & 0xFF);
                            scl_entry.key.u.hash_mac_key.mac[3] = ((mac_temp >> 16) & 0xFF);
                            scl_entry.key.u.hash_mac_key.mac[2] = ((mac_temp >> 24) & 0xFF);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_MAC:
                            scl_entry.key.u.hash_port_mac_key.gport = (scl_entry.key.u.hash_port_mac_key.gport + 1) % 256;
                            mac_temp += 1;
                            scl_entry.key.u.hash_port_mac_key.mac[5] = (mac_temp & 0xFF);
                            scl_entry.key.u.hash_port_mac_key.mac[4] = ((mac_temp >> 8)  & 0xFF);
                            scl_entry.key.u.hash_port_mac_key.mac[3] = ((mac_temp >> 16) & 0xFF);
                            scl_entry.key.u.hash_port_mac_key.mac[2] = ((mac_temp >> 24) & 0xFF);
                            break;
                        case CTC_SCL_KEY_HASH_IPV4:
                            scl_entry.key.u.hash_ipv4_key.ip_sa += 1;
                            break;
                        case CTC_SCL_KEY_HASH_PORT_IPV4:
                            scl_entry.key.u.hash_port_ipv4_key.gport = (scl_entry.key.u.hash_port_ipv4_key.gport + 1) % 256;
                            scl_entry.key.u.hash_port_ipv4_key.ip_sa += 1;
                            break;
                        case CTC_SCL_KEY_HASH_IPV6:
                            scl_entry.key.u.hash_ipv6_key.ip_sa[0] += 1;
                            break;
                        case CTC_SCL_KEY_HASH_PORT_IPV6:
                            scl_entry.key.u.hash_port_ipv6_key.gport = (scl_entry.key.u.hash_port_ipv6_key.gport + 1) % 256;
                            scl_entry.key.u.hash_port_ipv6_key.ip_sa[0] += 1;
                            break;
                        /* tcam key start */
                        case CTC_SCL_KEY_TCAM_VLAN:
                            scl_entry.key.u.tcam_vlan_key.cvlan = (scl_entry.key.u.tcam_vlan_key.cvlan + 1) % 4095 + 1;
                            scl_entry.key.u.tcam_vlan_key.cvlan_mask = 0xFFF;
                            CTC_SET_FLAG(scl_entry.key.u.tcam_vlan_key.flag, CTC_SCL_TCAM_VLAN_KEY_FLAG_CVLAN);
                            scl_entry.key.u.tcam_vlan_key.svlan = (scl_entry.key.u.tcam_vlan_key.svlan + 1) % 4095 + 1;
                            scl_entry.key.u.tcam_vlan_key.svlan_mask = 0xFFF;
                            CTC_SET_FLAG(scl_entry.key.u.tcam_vlan_key.flag, CTC_SCL_TCAM_VLAN_KEY_FLAG_SVLAN);
                            break;
                        case CTC_SCL_KEY_TCAM_MAC:
                            scl_entry.key.u.tcam_mac_key.cvlan = (scl_entry.key.u.tcam_mac_key.cvlan + 1) % 4095 + 1;
                            scl_entry.key.u.tcam_mac_key.cvlan_mask = 0xFFF;
                            CTC_SET_FLAG(scl_entry.key.u.tcam_mac_key.flag, CTC_SCL_TCAM_MAC_KEY_FLAG_CVLAN);
                            mac_temp += 1;
                            scl_entry.key.u.tcam_mac_key.mac_sa[5] = (mac_temp & 0xFF);
                            scl_entry.key.u.tcam_mac_key.mac_sa[4] = ((mac_temp >> 8)  & 0xFF);
                            scl_entry.key.u.tcam_mac_key.mac_sa[3] = ((mac_temp >> 16) & 0xFF);
                            scl_entry.key.u.tcam_mac_key.mac_sa[2] = ((mac_temp >> 24) & 0xFF);
                            CTC_SET_FLAG(scl_entry.key.u.tcam_mac_key.flag, CTC_SCL_TCAM_MAC_KEY_FLAG_MAC_SA);
                            break;
                        case CTC_SCL_KEY_TCAM_IPV4:
                            scl_entry.key.u.tcam_ipv4_key.cvlan = (scl_entry.key.u.tcam_ipv4_key.cvlan + 1) % 4095 + 1;
                            scl_entry.key.u.tcam_ipv4_key.cvlan_mask = 0xFFF;
                            CTC_SET_FLAG(scl_entry.key.u.tcam_ipv4_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_CVLAN);
                            scl_entry.key.u.tcam_ipv4_key.ip_sa += 1;
                            scl_entry.key.u.tcam_ipv4_key.ip_sa_mask = 0xFFFFFFFF;
                            CTC_SET_FLAG(scl_entry.key.u.tcam_ipv4_key.flag, CTC_SCL_TCAM_IPV4_KEY_FLAG_IP_SA);
                            /* for dt2, can only double key size */
                            scl_entry.key.u.tcam_ipv4_key.key_size = CTC_SCL_KEY_SIZE_DOUBLE;
                            break;
                        case CTC_SCL_KEY_TCAM_IPV6:
                            scl_entry.key.u.tcam_ipv6_key.cvlan = (scl_entry.key.u.tcam_ipv6_key.cvlan + 1) % 4095 + 1;
                            scl_entry.key.u.tcam_ipv6_key.cvlan_mask = 0xFFF;
                            CTC_SET_FLAG(scl_entry.key.u.tcam_ipv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_CVLAN);
                            scl_entry.key.u.tcam_ipv6_key.ip_sa[0] += 1;
                            sal_memset(&scl_entry.key.u.tcam_ipv6_key.ip_sa_mask, 0xFF, sizeof(ipv6_addr_t));
                            CTC_SET_FLAG(scl_entry.key.u.tcam_ipv6_key.flag, CTC_SCL_TCAM_IPV6_KEY_FLAG_IP_SA);
                            break;
                        default:
                            sal_free(g_ctc_stress_flow->p_entry_id);
                            g_ctc_stress_flow->p_entry_id = NULL;
                            return CTC_E_NOT_SUPPORT;
                            break;
                    }
                }

                if (g_ctcs_api_en)
                {
                    ret = ctcs_scl_add_entry(g_api_lchip, group_id, &scl_entry);
                }
                else
                {
                    ret = ctc_scl_add_entry(group_id, &scl_entry);
                }

                if(scl_entry.mode)
                {
                    switch(scl_entry.key_type)
                    {
                        case CTC_SCL_KEY_HASH_PORT:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = (field_port.gport + 1) % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_CVLAN:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = (field_port.gport + 1) % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            cvlan = (cvlan + 1) % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_CVLAN_ID;
                            field_key.data = cvlan;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_SVLAN:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = (field_port.gport + 1) % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            svlan = (svlan + 1) % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_SVLAN_ID;
                            field_key.data = svlan;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_2VLAN:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = (field_port.gport + 1) % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            cvlan = (cvlan + 1) % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_CVLAN_ID;
                            field_key.data = cvlan;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            svlan = (svlan + 1) % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_SVLAN_ID;
                            field_key.data = svlan;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_CVLAN_COS:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = (field_port.gport + 1) % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            cvlan = (cvlan + 1) % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_CVLAN_ID;
                            field_key.data = cvlan;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            ctagcos = (ctagcos + 1) % 8;
                            field_key.type = CTC_FIELD_KEY_CTAG_COS;
                            field_key.data = ctagcos;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_SVLAN_COS:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = (field_port.gport + 1) % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            svlan = (svlan + 1) % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_SVLAN_ID;
                            field_key.data = svlan;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            stagcos = (stagcos + 1) % 8;
                            field_key.type = CTC_FIELD_KEY_STAG_COS;
                            field_key.data = stagcos;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_MAC:
                            field_key.type = CTC_FIELD_KEY_MAC_SA;
                            mac_temp += 1;
                            mac[5] = (mac_temp & 0xFF);
                            mac[4] = ((mac_temp >> 8)  & 0xFF);
                            mac[3] = ((mac_temp >> 16) & 0xFF);
                            mac[2] = ((mac_temp >> 24) & 0xFF);
                            field_key.ext_data = &mac;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_MAC:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = (field_port.gport + 1) % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_MAC_SA;
                            mac_temp += 1;
                            mac[5] = (mac_temp & 0xFF);
                            mac[4] = ((mac_temp >> 8)  & 0xFF);
                            mac[3] = ((mac_temp >> 16) & 0xFF);
                            mac[2] = ((mac_temp >> 24) & 0xFF);
                            field_key.ext_data = &mac;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_IPV4:
                            field_key.type = CTC_FIELD_KEY_IP_SA;
                            ip_sa = ip_sa + 1;
                            field_key.data = ip_sa;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_IPV4:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = (field_port.gport + 1) % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_IP_SA;
                            ip_sa = ip_sa + 1;
                            field_key.data = ip_sa;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_IPV6:
                            field_key.type = CTC_FIELD_KEY_IPV6_SA;
                            ipv6_sa[0] = ipv6_sa[0] + 1;
                            field_key.ext_data = &ipv6_sa;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_IPV6:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = (field_port.gport + 1) % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_IPV6_SA;
                            ipv6_sa[0] = ipv6_sa[0] + 1;
                            field_key.ext_data = &ipv6_sa;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_TCAM_MAC:
                            cvlan = (cvlan + 1) % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_CVLAN_ID;
                            field_key.data = cvlan;
                            field_key.mask = 0xFFF;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_MAC_SA;
                            mac_temp += 1;
                            mac[5] = (mac_temp & 0xFF);
                            mac[4] = ((mac_temp >> 8)  & 0xFF);
                            mac[3] = ((mac_temp >> 16) & 0xFF);
                            mac[2] = ((mac_temp >> 24) & 0xFF);
                            field_key.ext_data = &mac;
                            field_key.ext_mask = &mac_mask;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_TCAM_IPV4:
                            field_key.type = CTC_FIELD_KEY_L3_TYPE;
                            field_key.data = CTC_PARSER_L3_TYPE_IPV4;
                            field_key.mask = 0xF;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            cvlan = (cvlan + 1) % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_CVLAN_ID;
                            field_key.data = cvlan;
                            field_key.mask = 0xFFF;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_IP_SA;
                            ip_sa = ip_sa + 1;
                            field_key.data = ip_sa;
                            field_key.mask = 0xFFFFFFFF;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_TCAM_IPV4_SINGLE:
                            field_key.type = CTC_FIELD_KEY_L3_TYPE;
                            field_key.data = CTC_PARSER_L3_TYPE_IPV4;
                            field_key.mask = 0xF;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_IP_SA;
                            ip_sa = ip_sa + 1;
                            field_key.data = ip_sa;
                            field_key.mask = 0xFFFFFFFF;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_TCAM_IPV6:
                            cvlan = (cvlan + 1) % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_CVLAN_ID;
                            field_key.data = cvlan;
                            field_key.mask = 0xFFF;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_IPV6_SA;
                            ipv6_sa[0] = ipv6_sa[0] + 1;
                            field_key.ext_data = &ipv6_sa;
                            field_key.ext_mask = &ipv6_sa_mask;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_TCAM_IPV6_SINGLE:
                            field_key.type = CTC_FIELD_KEY_IPV6_SA;
                            ipv6_sa[3] = ipv6_sa[3] + 1;
                            field_key.ext_data = &ipv6_sa;
                            field_key.ext_mask = &ipv6_sa_mask;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        default:
                            sal_free(g_ctc_stress_flow->p_entry_id);
                            g_ctc_stress_flow->p_entry_id = NULL;
                            return CTC_E_NOT_SUPPORT;
                            break;
                    }
                }
            }
            else if(CTC_CLI_STRESS_DATA_MODE_RANDOM == stress_param.data_mode)
            {
                /* key random */
                scl_entry.entry_id = sal_rand();

                if(!scl_entry.mode)
                {
                    switch(scl_entry.key.type)
                    {
                        case CTC_SCL_KEY_HASH_PORT:
                            scl_entry.key.u.hash_port_key.gport = sal_rand() % 256;
                            break;
                        case CTC_SCL_KEY_HASH_PORT_CVLAN:
                            scl_entry.key.u.hash_port_cvlan_key.gport = sal_rand() % 256;
                            scl_entry.key.u.hash_port_cvlan_key.cvlan = sal_rand() % 4095 + 1;
                            break;
                        case CTC_SCL_KEY_HASH_PORT_SVLAN:
                            scl_entry.key.u.hash_port_svlan_key.gport = sal_rand() % 256;
                            scl_entry.key.u.hash_port_svlan_key.svlan = sal_rand() % 4095 + 1;
                            break;
                        case CTC_SCL_KEY_HASH_PORT_2VLAN:
                            scl_entry.key.u.hash_port_2vlan_key.gport = sal_rand() % 256;
                            scl_entry.key.u.hash_port_2vlan_key.svlan = sal_rand() % 4095 + 1;
                            scl_entry.key.u.hash_port_2vlan_key.cvlan = sal_rand() % 4095 + 1;
                            break;
                        case CTC_SCL_KEY_HASH_PORT_CVLAN_COS:
                            scl_entry.key.u.hash_port_cvlan_cos_key.gport = sal_rand() % 256;
                            scl_entry.key.u.hash_port_cvlan_cos_key.cvlan = sal_rand() % 4095 + 1;
                            scl_entry.key.u.hash_port_cvlan_cos_key.ctag_cos = sal_rand() % 8;
                            break;
                        case CTC_SCL_KEY_HASH_PORT_SVLAN_COS:
                            scl_entry.key.u.hash_port_svlan_cos_key.gport = sal_rand() % 256;
                            scl_entry.key.u.hash_port_svlan_cos_key.svlan = sal_rand() % 4095 + 1;
                            scl_entry.key.u.hash_port_svlan_cos_key.stag_cos = sal_rand() % 8;
                            break;
                        case CTC_SCL_KEY_HASH_MAC:
                            mac_temp = sal_rand();
                            scl_entry.key.u.hash_mac_key.mac[5] = (mac_temp & 0xFF);
                            scl_entry.key.u.hash_mac_key.mac[4] = ((mac_temp >> 8)  & 0xFF);
                            scl_entry.key.u.hash_mac_key.mac[3] = ((mac_temp >> 16) & 0xFF);
                            scl_entry.key.u.hash_mac_key.mac[2] = ((mac_temp >> 24) & 0xFF);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_MAC:
                            scl_entry.key.u.hash_port_mac_key.gport = sal_rand() % 256;
                            mac_temp = sal_rand();
                            scl_entry.key.u.hash_port_mac_key.mac[5] = (mac_temp & 0xFF);
                            scl_entry.key.u.hash_port_mac_key.mac[4] = ((mac_temp >> 8)  & 0xFF);
                            scl_entry.key.u.hash_port_mac_key.mac[3] = ((mac_temp >> 16) & 0xFF);
                            scl_entry.key.u.hash_port_mac_key.mac[2] = ((mac_temp >> 24) & 0xFF);
                            break;
                        case CTC_SCL_KEY_HASH_IPV4:
                            scl_entry.key.u.hash_ipv4_key.ip_sa = sal_rand();
                            break;
                        case CTC_SCL_KEY_HASH_PORT_IPV4:
                            scl_entry.key.u.hash_port_ipv4_key.gport = sal_rand() % 256;
                            scl_entry.key.u.hash_port_ipv4_key.ip_sa = sal_rand();
                            break;
                        case CTC_SCL_KEY_HASH_IPV6:
                            scl_entry.key.u.hash_ipv6_key.ip_sa[0] = sal_rand();
                            break;
                        case CTC_SCL_KEY_HASH_PORT_IPV6:
                            scl_entry.key.u.hash_port_ipv6_key.gport = sal_rand() % 256;
                            scl_entry.key.u.hash_port_ipv6_key.ip_sa[0] = sal_rand();
                            break;
                        /* tcam key start */
                        case CTC_SCL_KEY_TCAM_VLAN:
                            scl_entry.key.u.tcam_vlan_key.cvlan = sal_rand() % 4095 + 1;
                            scl_entry.key.u.tcam_vlan_key.cvlan_mask = 0xFFF;
                            scl_entry.key.u.tcam_vlan_key.svlan = sal_rand() % 4095 + 1;
                            scl_entry.key.u.tcam_vlan_key.svlan_mask = 0xFFF;
                            break;
                        case CTC_SCL_KEY_TCAM_MAC:
                            scl_entry.key.u.tcam_mac_key.cvlan = sal_rand() % 4095 + 1;
                            scl_entry.key.u.tcam_mac_key.cvlan_mask = 0xFFF;
                            mac_temp = sal_rand();
                            scl_entry.key.u.tcam_mac_key.mac_sa[5] = (mac_temp & 0xFF);
                            scl_entry.key.u.tcam_mac_key.mac_sa[4] = ((mac_temp >> 8)  & 0xFF);
                            scl_entry.key.u.tcam_mac_key.mac_sa[3] = ((mac_temp >> 16) & 0xFF);
                            scl_entry.key.u.tcam_mac_key.mac_sa[2] = ((mac_temp >> 24) & 0xFF);
                            break;
                        case CTC_SCL_KEY_TCAM_IPV4:
                            scl_entry.key.u.tcam_ipv4_key.cvlan = sal_rand() % 4095 + 1;
                            scl_entry.key.u.tcam_ipv4_key.cvlan_mask = 0xFFF;
                            scl_entry.key.u.tcam_ipv4_key.ip_sa = sal_rand();
                            scl_entry.key.u.tcam_ipv4_key.ip_sa_mask = 0xFFFFFFFF;
                            scl_entry.key.u.tcam_ipv4_key.key_size = CTC_SCL_KEY_SIZE_DOUBLE;
                            break;
                        case CTC_SCL_KEY_TCAM_IPV6:
                            scl_entry.key.u.tcam_ipv6_key.cvlan = sal_rand() % 4095 + 1;
                            scl_entry.key.u.tcam_ipv6_key.cvlan_mask = 0xFFF;
                            scl_entry.key.u.tcam_ipv6_key.ip_sa[0] = sal_rand();
                            sal_memset(&scl_entry.key.u.tcam_ipv6_key.ip_sa_mask, 0xFF, sizeof(ipv6_addr_t));
                            break;
                        default:
                            sal_free(g_ctc_stress_flow->p_entry_id);
                            g_ctc_stress_flow->p_entry_id = NULL;
                            return CTC_E_NOT_SUPPORT;
                            break;
                    }
                }

                if (g_ctcs_api_en)
                {
                    ret = ctcs_scl_add_entry(g_api_lchip, group_id, &scl_entry);
                }
                else
                {
                    ret = ctc_scl_add_entry(group_id, &scl_entry);
                }

                if(scl_entry.mode)
                {
                    switch(scl_entry.key_type)
                    {
                        case CTC_SCL_KEY_HASH_PORT:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = sal_rand() % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_CVLAN:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = sal_rand() % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            cvlan = sal_rand() % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_CVLAN_ID;
                            field_key.data = cvlan;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_SVLAN:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = sal_rand() % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            svlan = sal_rand() % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_SVLAN_ID;
                            field_key.data = svlan;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_2VLAN:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = sal_rand() % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            cvlan = sal_rand() % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_CVLAN_ID;
                            field_key.data = cvlan;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            svlan = sal_rand() % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_SVLAN_ID;
                            field_key.data = svlan;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_CVLAN_COS:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = sal_rand() % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            cvlan = sal_rand() % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_CVLAN_ID;
                            field_key.data = cvlan;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            ctagcos = sal_rand() % 8;
                            field_key.type = CTC_FIELD_KEY_CTAG_COS;
                            field_key.data = ctagcos;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_SVLAN_COS:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = sal_rand() % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            svlan = sal_rand() % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_SVLAN_ID;
                            field_key.data = svlan;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            stagcos = sal_rand() % 8;
                            field_key.type = CTC_FIELD_KEY_STAG_COS;
                            field_key.data = stagcos;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_MAC:
                            field_key.type = CTC_FIELD_KEY_MAC_SA;
                            mac_temp = sal_rand();
                            mac[5] = (mac_temp & 0xFF);
                            mac[4] = ((mac_temp >> 8)  & 0xFF);
                            mac[3] = ((mac_temp >> 16) & 0xFF);
                            mac[2] = ((mac_temp >> 24) & 0xFF);
                            field_key.ext_data = &mac;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_MAC:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = sal_rand() % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_MAC_SA;
                            mac_temp = sal_rand();
                            mac[5] = (mac_temp & 0xFF);
                            mac[4] = ((mac_temp >> 8)  & 0xFF);
                            mac[3] = ((mac_temp >> 16) & 0xFF);
                            mac[2] = ((mac_temp >> 24) & 0xFF);
                            field_key.ext_data = &mac;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_IPV4:
                            field_key.type = CTC_FIELD_KEY_IP_SA;
                            ip_sa = sal_rand();
                            field_key.data = ip_sa;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_IPV4:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = sal_rand() % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_IP_SA;
                            ip_sa = sal_rand();
                            field_key.data = ip_sa;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_IPV6:
                            field_key.type = CTC_FIELD_KEY_IPV6_SA;
                            ipv6_sa[0] = sal_rand();
                            field_key.ext_data = &ipv6_sa;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_HASH_PORT_IPV6:
                            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                            field_port.gport = sal_rand() % 256;
                            field_key.type = CTC_FIELD_KEY_PORT;
                            field_key.ext_data = (void*)(&field_port);
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_IPV6_SA;
                            ipv6_sa[0] = sal_rand();
                            field_key.ext_data = &ipv6_sa;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_HASH_VALID;
                            field_key.data = 1;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_TCAM_MAC:
                            cvlan = sal_rand() % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_CVLAN_ID;
                            field_key.data = cvlan;
                            field_key.mask = 0xFFF;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_MAC_SA;
                            mac_temp = sal_rand();
                            mac[5] = (mac_temp & 0xFF);
                            mac[4] = ((mac_temp >> 8)  & 0xFF);
                            mac[3] = ((mac_temp >> 16) & 0xFF);
                            mac[2] = ((mac_temp >> 24) & 0xFF);
                            field_key.ext_data = &mac;
                            field_key.ext_mask = &mac_mask;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_TCAM_IPV4:
                            field_key.type = CTC_FIELD_KEY_L3_TYPE;
                            field_key.data = CTC_PARSER_L3_TYPE_IPV4;
                            field_key.mask = 0xF;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            cvlan = sal_rand() % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_CVLAN_ID;
                            field_key.data = cvlan;
                            field_key.mask = 0xFFF;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_IP_SA;
                            ip_sa = sal_rand();
                            field_key.data = ip_sa;
                            field_key.mask = 0xFFFFFFFF;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_TCAM_IPV4_SINGLE:
                            field_key.type = CTC_FIELD_KEY_L3_TYPE;
                            field_key.data = CTC_PARSER_L3_TYPE_IPV4;
                            field_key.mask = 0xF;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_IP_SA;
                            ip_sa = sal_rand();
                            field_key.data = ip_sa;
                            field_key.mask = 0xFFFFFFFF;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_TCAM_IPV6:
                            cvlan = sal_rand() % 4095 + 1;
                            field_key.type = CTC_FIELD_KEY_CVLAN_ID;
                            field_key.data = cvlan;
                            field_key.mask = 0xFFF;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            field_key.type = CTC_FIELD_KEY_IPV6_SA;
                            ipv6_sa[0] = sal_rand();
                            field_key.ext_data = &ipv6_sa;
                            field_key.ext_mask = &ipv6_sa_mask;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        case CTC_SCL_KEY_TCAM_IPV6_SINGLE:
                            field_key.type = CTC_FIELD_KEY_IPV6_SA;
                            ipv6_sa[3] = sal_rand();
                            field_key.ext_data = &ipv6_sa;
                            field_key.ext_mask = &ipv6_sa_mask;
                            ret = ret ? ret : ctc_scl_add_key_field(scl_entry.entry_id, &field_key);
                            break;
                        default:
                            sal_free(g_ctc_stress_flow->p_entry_id);
                            g_ctc_stress_flow->p_entry_id = NULL;
                            return CTC_E_NOT_SUPPORT;
                            break;
                    }
                }
            }

            *(g_ctc_stress_flow->p_entry_id + loop) = scl_entry.entry_id;

            if (ret == CTC_E_NONE)
            {
                /* ctc_cli_out("Add %d normal\n", loop + 1);*/
            }
            else
            {
                ctc_cli_out("Add %d error: %s\n", loop + 1, ctc_get_error_desc(ret));
            }

        }
        else if (CTC_CLI_STRESS_OP_TYPE_REMOVE == stress_param.op_type)
        {
            scl_entry.entry_id = *(g_ctc_stress_flow->p_entry_id + loop);

            if(g_ctcs_api_en)
            {
                ret = ctcs_scl_remove_entry(g_api_lchip, scl_entry.entry_id);
            }
            else
            {
                ret = ctc_scl_remove_entry(scl_entry.entry_id);
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
        sal_free(g_ctc_stress_flow->p_entry_id);
        g_ctc_stress_flow->p_entry_id = NULL;
    }

    return CLI_SUCCESS;
}

CTC_CLI(ctc_cli_stress_scl_create_group,
        ctc_cli_stress_scl_create_group_cmd,
        "scl create group GROUP_ID (random | incr) entry-num NUMBER (spec | stress | perf)",
        CTC_CLI_SCL_STR,
        "Create SCL group",
        CTC_CLI_SCL_GROUP_ID_STR,
        CTC_CLI_SCL_NOMAL_GROUP_ID_VALUE,
        "Key value is random",
        "Key value is increase",
        "Entry num",
        "Number",
        "Spec test",
        "Stress test",
        "Performance test")
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0xFF;
    uint8 temp = 0;
    uint32 group_id = 0;
    uint32 entry_num = 0;
    ctc_scl_group_info_t ginfo;
    ctc_cli_stress_t stress_param;

    sal_memset(&ginfo, 0, sizeof(ctc_scl_group_info_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    CTC_CLI_GET_UINT32("group id", group_id, argv[0]);

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

    ret = _ctc_cli_stress_scl_group(group_id, ginfo, stress_param, entry_num);

    return ret;
}

CTC_CLI(ctc_cli_stress_scl_destroy_group,
        ctc_cli_stress_scl_destroy_group_cmd,
        "scl destroy group",
        "SCL module",
        "Destroy group",
        "SCL group")
{
    int32 ret = CTC_E_NONE;
    uint32 entry_num = 0;
    uint32 group_id = 0;
    ctc_scl_group_info_t group_info;
    ctc_cli_stress_t stress_param;

    sal_memset(&group_info, 0, sizeof(ctc_scl_group_info_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_MAX;
    stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_MAX;

    ret = _ctc_cli_stress_scl_group(group_id, group_info, stress_param, entry_num);

    return ret;
}

CTC_CLI(ctc_cli_stress_scl_add_entry,
        ctc_cli_stress_scl_add_entry_cmd,
        "scl add group GROUP_ID entry ENTRY_ID (entry-priority PRIORITY (fixed-pri | inc-pri | dec-pri | random-pri) |)(mode MODE |) ("SCL_ENTRY_KEY_TYPE") (ingress-action | egress-action | flow-action) (random | incr) entry-num NUMBER (spec | stress | perf)",
        CTC_CLI_SCL_STR,
        "Add one entry to software table",
        CTC_CLI_SCL_GROUP_ID_STR,
        CTC_CLI_SCL_NOMAL_GROUP_ID_VALUE,
        CTC_CLI_SCL_ENTRY_ID_STR,
        CTC_CLI_SCL_ENTRY_ID_VALUE,
        CTC_CLI_SCL_ENTRY_PRIO_STR,
        CTC_CLI_SCL_ENTRY_PRIO_VALUE,
        "Priority is fixed",
        "Priority is increase",
        "Priority is decrease",
        "Priority is random",
        "Mode",
        "0: use sturct to install key and action; 1: use field to install key and action",
        SCL_ENTRY_KEY_TYPE_DESC,
        "Ingress action",
        "Egress action",
        "Flow action",
        "Key value is random, action is fixed",
        "Key value is increase, action is fixed",
        "Entry num",
        "Number",
        "Spec test",
        "Stress test",
        "Performance test")
{
    uint8 index = 0;
    uint8 mode = 0;
    uint8 temp = 0;
    uint32 entry_num = 0;
    int32 ret = CTC_E_NONE;
    uint32 gid = 0;
    ctc_scl_entry_t scl_entry;
    ctc_cli_stress_t stress_param;

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    CTC_CLI_GET_UINT32("group id", gid, argv[0]);
    CTC_CLI_GET_UINT32("entry id", scl_entry.entry_id, argv[1]);

    index = CTC_CLI_GET_ARGC_INDEX("entry-priority");
    if (0xFF != index)
    {
        scl_entry.priority_valid = 1;
        CTC_CLI_GET_UINT32("entry priority", scl_entry.priority, argv[index + 1]);
    }
    index = CTC_CLI_GET_ARGC_INDEX("fixed-pri");
    if (0xFF != index)
    {
        stress_param.priority_mode = CTC_CLI_STRESS_PRIORITY_MODE_FIXED;
    }
    index = CTC_CLI_GET_ARGC_INDEX("inc-pri");
    if (0xFF != index)
    {
        stress_param.priority_mode = CTC_CLI_STRESS_PRIORITY_MODE_INCREASE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("dec-pri");
    if (0xFF != index)
    {
        stress_param.priority_mode = CTC_CLI_STRESS_PRIORITY_MODE_DECREASE;
    }
    index = CTC_CLI_GET_ARGC_INDEX("random-pri");
    if (0xFF != index)
    {
        stress_param.priority_mode = CTC_CLI_STRESS_PRIORITY_MODE_RANDOM;
    }

    index = CTC_CLI_GET_ARGC_INDEX("mode");
    if (0xFF != index)
    {
        CTC_CLI_GET_UINT8("mode", mode, argv[index + 1]);
        if (1 == mode)
        {
            scl_entry.mode = 1;
        }
    }

    if(mode)
    {
        /* dt2 use */
        index = CTC_CLI_GET_ARGC_INDEX("ipv4-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_HASH_IPV4;
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipv6-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_HASH_IPV6;
        }

        index = CTC_CLI_GET_ARGC_INDEX("l2-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_HASH_L2;
        }
        index = CTC_CLI_GET_ARGC_INDEX("mac-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_HASH_MAC;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-cvlan-cos-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_HASH_PORT_CVLAN_COS;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-cvlan-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_HASH_PORT_CVLAN;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-double-vlan-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_HASH_PORT_2VLAN;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_HASH_PORT;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-ipv4-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_HASH_PORT_IPV4;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-ipv6-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_HASH_PORT_IPV6;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-mac-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_HASH_PORT_MAC;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-svlan-cos-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_HASH_PORT_SVLAN_COS;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-svlan-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_HASH_PORT_SVLAN;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-cross-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_HASH_PORT_CROSS;
        }
        index = CTC_CLI_GET_ARGC_INDEX("mac-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_TCAM_MAC;
        }
        index = CTC_CLI_GET_ARGC_INDEX("ipv4-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_TCAM_IPV4;
        }
        index = CTC_CLI_GET_ARGC_INDEX("ipv4-single-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_TCAM_IPV4_SINGLE;
        }
        index = CTC_CLI_GET_ARGC_INDEX("ipv6-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_TCAM_IPV6;
        }
        index = CTC_CLI_GET_ARGC_INDEX("ipv6-single-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_TCAM_IPV6_SINGLE;
        }
        index = CTC_CLI_GET_ARGC_INDEX("vlan-entry");
        if (0xFF != index)
        {
            scl_entry.key_type = CTC_SCL_KEY_TCAM_VLAN;
        }

        index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
        if (0xFF != index)
        {
            scl_entry.action_type = CTC_SCL_ACTION_INGRESS;
        }

        index = CTC_CLI_GET_ARGC_INDEX("egress-action");
        if (0xFF != index)
        {
            scl_entry.action_type = CTC_SCL_ACTION_EGRESS;
        }

        index = CTC_CLI_GET_ARGC_INDEX("flow-action");
        if (0xFF != index)
        {
            scl_entry.action_type = CTC_SCL_ACTION_FLOW;
        }
    }
    else
    {
        /* gg and dt2 use */
        index = CTC_CLI_GET_ARGC_INDEX("ipv4-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_HASH_IPV4;
        }

        index = CTC_CLI_GET_ARGC_INDEX("ipv6-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_HASH_IPV6;
        }

        index = CTC_CLI_GET_ARGC_INDEX("l2-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_HASH_L2;
        }
        index = CTC_CLI_GET_ARGC_INDEX("mac-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_HASH_MAC;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-cvlan-cos-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_CVLAN_COS;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-cvlan-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_CVLAN;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-double-vlan-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_2VLAN;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_HASH_PORT;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-ipv4-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_IPV4;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-ipv6-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_IPV6;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-mac-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_MAC;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-svlan-cos-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_SVLAN_COS;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-svlan-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_SVLAN;
        }
        index = CTC_CLI_GET_ARGC_INDEX("port-cross-hash-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_HASH_PORT_CROSS;
        }
        index = CTC_CLI_GET_ARGC_INDEX("mac-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_TCAM_MAC;
        }
        index = CTC_CLI_GET_ARGC_INDEX("ipv4-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_TCAM_IPV4;
        }
        index = CTC_CLI_GET_ARGC_INDEX("ipv4-single-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_TCAM_IPV4_SINGLE;
        }
        index = CTC_CLI_GET_ARGC_INDEX("ipv6-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_TCAM_IPV6;
        }
        index = CTC_CLI_GET_ARGC_INDEX("ipv6-single-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_TCAM_IPV6_SINGLE;
        }
        index = CTC_CLI_GET_ARGC_INDEX("vlan-entry");
        if (0xFF != index)
        {
            scl_entry.key.type = CTC_SCL_KEY_TCAM_VLAN;
        }

        index = CTC_CLI_GET_ARGC_INDEX("ingress-action");
        if (0xFF != index)
        {
            scl_entry.action.type = CTC_SCL_ACTION_INGRESS;
        }

        index = CTC_CLI_GET_ARGC_INDEX("egress-action");
        if (0xFF != index)
        {
            scl_entry.action.type = CTC_SCL_ACTION_EGRESS;
        }

        index = CTC_CLI_GET_ARGC_INDEX("flow-action");
        if (0xFF != index)
        {
            scl_entry.action.type = CTC_SCL_ACTION_FLOW;
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

    ret = _ctc_cli_stress_scl_entry(gid, scl_entry, stress_param, entry_num);

    return ret;
}


CTC_CLI(ctc_cli_stress_scl_remove_entry,
        ctc_cli_stress_scl_remove_entry_cmd,
        "scl remove entry",
        "SCL module",
        "Remove entry",
        "SCL entry")
{
    int32 ret = CTC_E_NONE;
    uint32 entry_num = 0;
    uint32 group_id = 0;
    ctc_scl_entry_t scl_entry;
    ctc_cli_stress_t stress_param;

    sal_memset(&scl_entry, 0, sizeof(ctc_scl_entry_t));
    sal_memset(&stress_param, 0, sizeof(ctc_cli_stress_t));

    stress_param.op_type = CTC_CLI_STRESS_OP_TYPE_REMOVE;
    stress_param.test_type = CTC_CLI_STRESS_TEST_TYPE_MAX;
    stress_param.data_mode = CTC_CLI_STRESS_DATA_MODE_MAX;
    stress_param.priority_mode = CTC_CLI_STRESS_PRIORITY_MODE_MAX;

    ret = _ctc_cli_stress_scl_entry(group_id, scl_entry, stress_param, entry_num);

    return ret;
}

int32
ctc_stress_test_flow_cli_init(void)
{
    g_ctc_stress_flow = (ctc_cli_stress_test_flow_t*)sal_malloc(sizeof(ctc_cli_stress_test_flow_t));
    if(!g_ctc_stress_flow)
    {
        return CLI_ERROR;
    }

    sal_memset(g_ctc_stress_flow, 0, sizeof(ctc_cli_stress_test_flow_t));

    install_element(CTC_STRESS_MODE, &ctc_cli_stress_acl_create_group_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_acl_destroy_group_cmd);

    install_element(CTC_STRESS_MODE, &ctc_cli_stress_acl_add_entry_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_acl_remove_entry_cmd);

    install_element(CTC_STRESS_MODE, &ctc_cli_stress_acl_add_hash_entry_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_acl_remove_hash_entry_cmd);

    install_element(CTC_STRESS_MODE, &ctc_cli_stress_scl_create_group_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_scl_destroy_group_cmd);

    install_element(CTC_STRESS_MODE, &ctc_cli_stress_scl_add_entry_cmd);
    install_element(CTC_STRESS_MODE, &ctc_cli_stress_scl_remove_entry_cmd);

    return CLI_SUCCESS;
}


