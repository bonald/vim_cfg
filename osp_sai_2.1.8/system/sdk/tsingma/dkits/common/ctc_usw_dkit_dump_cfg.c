#include "sal.h"
#include "drv_api.h"
#include "usw/include/drv_common.h"
#include "usw/include/drv_ftm.h"
#include "ctc_cli.h"
#include "ctc_dkit.h"
#include "ctc_dkit_cli.h"
#include "ctc_dkit_common.h"
#include "ctc_usw_dkit.h"
#include "ctc_usw_dkit_drv.h"
#include "ctc_usw_dkit_tcam.h"
#include "ctc_usw_dkit_dump_tbl.h"
#include "ctc_usw_dkit_hash_db.h"
#include "ctc_usw_dkit_memory.h"
#include "ctc_usw_dkit_misc.h"

#if 0
#include "ctc_usw_dkit_misc.h"
#endif

#include "ctc_usw_dkit_dump_cfg.h"

#define CTC_DKITS_DUMP_CFG_SID_STATIC     0x1FFFFFFF
#define CTC_DKITS_DUMP_CFG_SID_NO_ALLOC   0x2FFFFFFF
#define CTC_DKITS_DUMP_CFG_SHOW_NO_ALLOC  1

#define CTC_DKITS_DUMP_CFG_ERROR_RETURN(op) \
    { \
        int32 rv = (op); \
        if ((rv < 0) || (1 == rv)) \
        { \
            CTC_DKIT_PRINT("Dkits Error Happened!! Fun:%s()  Line:%d ret:%d\n",__FUNCTION__,__LINE__, rv); \
            return (CLI_ERROR); \
        } \
    }

typedef int32 (*travel_callback_fun_t)(uint8, void*, tbls_id_t, void*, void*, void*);

enum swap_direction_e
{
    HOST_TO_NETWORK,
    NETWORK_TO_HOST
};
typedef enum swap_direction_e swap_direction_t;

enum ctc_dkits_fib_hash_fun_type_e
{
    CTC_DKITS_FIB_HASH_FUN_TYPE_BRIDGE,
    CTC_DKITS_FIB_HASH_FUN_TYPE_IPUC_IPV4,
    CTC_DKITS_FIB_HASH_FUN_TYPE_IPUC_IPV6,
    CTC_DKITS_FIB_HASH_FUN_TYPE_IPMC_IPV4,
    CTC_DKITS_FIB_HASH_FUN_TYPE_IPMC_IPV6,
    CTC_DKITS_FIB_HASH_FUN_TYPE_FCOE,
    CTC_DKITS_FIB_HASH_FUN_TYPE_TRILL,

    CTC_DKITS_FIB_HASH_FUN_TYPE_NUM
};

enum ctc_dkits_fib0_hash_class_type_e
{
    CTC_DKITS_FIB0_HASH_CLASS_TYPE_FDB,
    CTC_DKITS_FIB0_HASH_CLASS_TYPE_IPV4_UCAST,
    CTC_DKITS_FIB0_HASH_CLASS_TYPE_IPV4_MCAST,
    CTC_DKITS_FIB0_HASH_CLASS_TYPE_IPV4_NATSA,
    CTC_DKITS_FIB0_HASH_CLASS_TYPE_IPV6_UCAST,
    CTC_DKITS_FIB0_HASH_CLASS_TYPE_IPV6_MCAST,
    CTC_DKITS_FIB0_HASH_CLASS_TYPE_FCOE,
    CTC_DKITS_FIB0_HASH_CLASS_TYPE_TRILL,

    CTC_DKITS_FIB0_HASH_CLASS_TYPE_NUM
};

enum ctc_dkits_fib1_hash_class_type_e
{
    CTC_DKITS_FIB1_HASH_CLASS_TYPE_IPV4_NAT,
    CTC_DKITS_FIB1_HASH_CLASS_TYPE_IPV4_MCAST,
    CTC_DKITS_FIB1_HASH_CLASS_TYPE_IPV6_NAT,
    CTC_DKITS_FIB1_HASH_CLASS_TYPE_IPV6_MCAST,
    CTC_DKITS_FIB1_HASH_CLASS_TYPE_L2_MCAST,
    CTC_DKITS_FIB1_HASH_CLASS_TYPE_OTHER,

    CTC_DKITS_FIB1_HASH_CLASS_TYPE_NUM
};

enum ctc_dkits_xcl_fun_type_e
{
    CTC_DKITS_XCL_FUN_TYPE_BRIDGE,
    CTC_DKITS_XCL_FUN_TYPE_IPV4,
    CTC_DKITS_XCL_FUN_TYPE_IPV6,
    CTC_DKITS_XCL_FUN_TYPE_L2L3,

    CTC_DKITS_XCL_FUN_TYPE_NUM
};

enum ctc_dkits_nat_pbr_tcam_fun_type_e
{
    CTC_DKITS_NAT_PBR_TCAM_FUN_TYPE_PREFIX,
    CTC_DKITS_NAT_PBR_TCAM_FUN_TYPE_NAT_IPV4,
    CTC_DKITS_NAT_PBR_TCAM_FUN_TYPE_NAT_IPV6,
    CTC_DKITS_NAT_PBR_TCAM_FUN_TYPE_PBR_IPV4,
    CTC_DKITS_NAT_PBR_TCAM_FUN_TYPE_PBR_IPV6,
    CTC_DKITS_NAT_PBR_TCAM_FUN_TYPE_NUM
};

enum ctc_dkits_flow_hash_fun_type_e
{
    CTC_DKITS_FLOW_HASH_FUN_TYPE_BRIDGE,
    CTC_DKITS_FLOW_HASH_FUN_TYPE_L2L3,
    CTC_DKITS_FLOW_HASH_FUN_TYPE_IPV4,
    CTC_DKITS_FLOW_HASH_FUN_TYPE_IPV6,
    CTC_DKITS_FLOW_HASH_FUN_TYPE_MPLS,
    CTC_DKITS_FLOW_HASH_FUN_TYPE_NUM
};

enum ctc_dkits_flow_hash_class_type_e
{
    CTC_DKITS_FLOW_HASH_CLASS_TYPE_MAC,
    CTC_DKITS_FLOW_HASH_CLASS_TYPE_L2L3,
    CTC_DKITS_FLOW_HASH_CLASS_TYPE_IPV4,
    CTC_DKITS_FLOW_HASH_CLASS_TYPE_IPV6,
    CTC_DKITS_FLOW_HASH_CLASS_TYPE_MPLS,
    CTC_DKITS_FLOW_HASH_CLASS_TYPE_NUM
};

enum ctc_dkits_xcoam_hash_fun_type_e
{
    CTC_DKITS_XCOAM_HASH_FUN_TYPE_SCL,
    CTC_DKITS_XCOAM_HASH_FUN_TYPE_PBB_TUNNEL,
    CTC_DKITS_XCOAM_HASH_FUN_TYPE_OAM,
    CTC_DKITS_XCOAM_HASH_FUN_TYPE_NUM
};

enum ctc_dkits_xcoam_hash_class_type_e
{
    CTC_DKITS_XCOAM_HASH_CLASS_TYPE_SCL,
    CTC_DKITS_XCOAM_HASH_CLASS_TYPE_TUNNEL,
    CTC_DKITS_XCOAM_HASH_CLASS_TYPE_OAM,
    CTC_DKITS_XCOAM_HASH_CLASS_TYPE_NUM
};

enum ctc_dkits_nh_fun_type_e
{
    CTC_DKITS_NH_TBL_TYPE_FWD,
    CTC_DKITS_NH_TBL_TYPE_MET,
    CTC_DKITS_NH_TBL_TYPE_NEXTHOP,
    CTC_DKITS_NH_TBL_TYPE_EDIT,

    CTC_DKITS_NH_TBL_TYPE_NUM
};

enum ctc_dkits_scl_hash_fun_type_e
{
    CTC_DKITS_SCL_HASH_FUN_TYPE_BRIDGE,
    CTC_DKITS_SCL_HASH_FUN_TYPE_IPV4,
    CTC_DKITS_SCL_HASH_FUN_TYPE_IPV6,
    CTC_DKITS_SCL_HASH_FUN_TYPE_IP_TUNNEL,
    CTC_DKITS_SCL_HASH_FUN_TYPE_NVGRE_TUNNEL,
    CTC_DKITS_SCL_HASH_FUN_TYPE_VXLAN_TUNNEL,
    CTC_DKITS_SCL_HASH_FUN_TYPE_MPLS_TUNNEL,
    CTC_DKITS_SCL_HASH_FUN_TYPE_PBB_TUNNEL,
    CTC_DKITS_SCL_HASH_FUN_TYPE_TRILL_TUNNEL,
    CTC_DKITS_SCL_HASH_FUN_TYPE_WLAN,
    CTC_DKITS_SCL_HASH_FUN_TYPE_NUM
};

enum ctc_dkits_scl_hash_class_type_e
{
    CTC_DKITS_SCL_HASH_CLASS_TYPE_SCL,
    CTC_DKITS_SCL_HASH_CLASS_TYPE_IPV4_TUNNEL,
    CTC_DKITS_SCL_HASH_CLASS_TYPE_IPV6_TUNNEL,
    CTC_DKITS_SCL_HASH_CLASS_TYPE_NVGRE_TUNNEL,
    CTC_DKITS_SCL_HASH_CLASS_TYPE_VXLAN_TUNNEL,
    CTC_DKITS_SCL_HASH_CLASS_TYPE_MPLS_TUNNEL,
    CTC_DKITS_SCL_HASH_CLASS_TYPE_PBB_TUNNEL,
    CTC_DKITS_SCL_HASH_CLASS_TYPE_TRILL_TUNNEL,
    CTC_DKITS_SCL_HASH_CLASS_TYPE_WLAN_TUNNEL,
    CTC_DKITS_SCL_HASH_CLASS_TYPE_WLAN,
    CTC_DKITS_SCL_HASH_CLASS_TYPE_8021BR,
    CTC_DKITS_SCL_HASH_CLASS_TYPE_NUM
};

enum ctc_dkits_ipfix_hash_fun_type_e
{
    CTC_DKITS_IPFIX_HASH_FUN_TYPE_BRIDGE,
    CTC_DKITS_IPFIX_HASH_FUN_TYPE_L2L3,
    CTC_DKITS_IPFIX_HASH_FUN_TYPE_IPV4,
    CTC_DKITS_IPFIX_HASH_FUN_TYPE_IPV6,
    CTC_DKITS_IPFIX_HASH_FUN_TYPE_MPLS,
    CTC_DKITS_IPFIX_HASH_FUN_TYPE_NUM
};

enum ctc_dkits_ipfix_hash_class_type_e
{
    CTC_DKITS_IPFIX_HASH_CLASS_TYPE_MAC,
    CTC_DKITS_IPFIX_HASH_CLASS_TYPE_L2L3,
    CTC_DKITS_IPFIX_HASH_CLASS_TYPE_IPV4,
    CTC_DKITS_IPFIX_HASH_CLASS_TYPE_IPV6,
    CTC_DKITS_IPFIX_HASH_CLASS_TYPE_MPLS,
    CTC_DKITS_IPFIX_HASH_CLASS_TYPE_NUM
};

enum ctc_dkits_hash_tbl_module_e
{
    CTC_DKITS_HASH_FUN_TYPE_FIB,
    CTC_DKITS_HASH_FUN_TYPE_FLOW,
    CTC_DKITS_HASH_FUN_TYPE_SCL,
    CTC_DKITS_HASH_FUN_TYPE_XCOAM,
    CTC_DKITS_HASH_FUN_TYPE_IPFIX,
    CTC_DKITS_HASH_FUN_TYPE_NUM
};

enum ctc_dkits_acl_tcam_fun_type_e
{
    CTC_DKITS_ACL_TCAM_FUN_TYPE_BRIDGE,
    CTC_DKITS_ACL_TCAM_FUN_TYPE_IPV4,
    CTC_DKITS_ACL_TCAM_FUN_TYPE_IPV6,
    CTC_DKITS_ACL_TCAM_FUN_TYPE_L2L3,
    CTC_DKITS_ACL_TCAM_FUN_TYPE_IPFIX,
    CTC_DKITS_ACL_TCAM_FUN_TYPE_FWD,
    CTC_DKITS_ACL_TCAM_FUN_TYPE_CATEGORY,
    CTC_DKITS_ACL_TCAM_FUN_TYPE_NUM
};

enum ctc_dkits_scl_tcam_fun_type_e
{
    CTC_DKITS_SCL_TCAM_FUN_TYPE_BRIDGE,
    CTC_DKITS_SCL_TCAM_FUN_TYPE_IPV4,
    CTC_DKITS_SCL_TCAM_FUN_TYPE_IPV6,
    CTC_DKITS_SCL_TCAM_FUN_TYPE_L2L3,
    CTC_DKITS_SCL_TCAM_FUN_TYPE_USERID,
    CTC_DKITS_SCL_TCAM_FUN_TYPE_NUM
};

enum ctc_dkits_fwd_tcam_fun_type_e
{
    CTC_DKITS_FWD_TCAM_FUN_TYPE_CID,
    CTC_DKITS_FWD_TCAM_FUN_TYPE_NUM
};

enum ctc_dkits_qmgr_tcam_fun_type_e
{
    CTC_DKITS_QMGR_TCAM_FUN_TYPE_QUEUE_MAP,
    CTC_DKITS_QMGR_TCAM_FUN_TYPE_NUM
};

#define CTC_DKITS_DUMP_CFG_TCAM_FUN_TYPE_NUM

enum ctc_dkits_tcam_tbl_module_e
{
    CTC_DKITS_TCAM_FUN_TYPE_SCL,
    CTC_DKITS_TCAM_FUN_TYPE_ACL,
    CTC_DKITS_TCAM_FUN_TYPE_IP,
    CTC_DKITS_TCAM_FUN_TYPE_FWD,
    CTC_DKITS_TCAM_FUN_TYPE_QMGR,
    CTC_DKITS_TCAM_FUN_TYPE_NUM
};
typedef enum ctc_dkits_tcam_tbl_module_e ctc_dkits_tcam_tbl_module_t;

enum ctc_dkits_format_type_e
{
    CTC_DKITS_FORMAT_TYPE_NONE,
    CTC_DKITS_FORMAT_TYPE_1BYTE,
    CTC_DKITS_FORMAT_TYPE_2BYTE,
    CTC_DKITS_FORMAT_TYPE_4BYTE,
    CTC_DKITS_FORMAT_TYPE_NUM
};
typedef enum ctc_dkits_format_type_e ctc_dkits_format_type_t;

enum ctc_dkits_dump_tbl_diff_flag_e
{
    CTC_DKITS_DUMP_TBL_DIFF_FLAG_TBL = 1U << 0,
    CTC_DKITS_DUMP_TBL_DIFF_FLAG_KEY = 1U << 1,
    CTC_DKITS_DUMP_TBL_DIFF_FLAG_AD  = 1U << 2,
    CTC_DKITS_DUMP_TBL_DIFF_FLAG_ALL = 3
};
typedef enum ctc_dkits_dump_tbl_diff_flag_e ctc_dkits_dump_tbl_diff_flag_t;

struct ctc_dkits_hash_key_filed_s
{
    tbls_id_t tblid;
    uint32    type;
    uint32    valid;
};
typedef struct ctc_dkits_hash_key_filed_s ctc_dkits_hash_key_filed_t;

extern int32
drv_get_host_type (uint8 lchip);

struct ctc_dkits_tcam_tbl_info_s
{
    uint32                         valid;
    uint32                         tcam_type;
    tbls_id_t                      ad_tblid;
};
typedef struct ctc_dkits_tcam_tbl_info_s ctc_dkits_tcam_tbl_info_t;

struct ctc_dkits_dump_hash_usage_s
{
    uint32  key[CTC_DKITS_HASH_DB_MODULE_NUM][CTC_DKITS_HASH_DB_MODULE_MAX_HASH_TYPE][4][MAX_HASH_LEVEL];
    uint32  cam[CTC_DKITS_HASH_DB_MODULE_NUM][CTC_DKITS_HASH_DB_MODULE_MAX_HASH_TYPE][4];
};
typedef struct ctc_dkits_dump_hash_usage_s ctc_dkits_dump_hash_usage_t;

struct ctc_dkits_dump_fun_usage_s
{
    uint32  key[CTC_DKITS_HASH_DB_MODULE_NUM][4*MAX_HASH_LEVEL][5];
    uint32  cam[CTC_DKITS_HASH_DB_MODULE_NUM][4*MAX_HASH_LEVEL];
};
typedef struct ctc_dkits_dump_fun_usage_s ctc_dkits_dump_fun_usage_t;

tbls_id_t* usw_fib_hash_key_tbl[] =
{
    usw_fib_hash_bridge_key_tbl,
    usw_fib_hash_ipuc_ipv4_key_tbl, usw_fib_hash_ipuc_ipv6_key_tbl,
    usw_fib_hash_ipmc_ipv4_key_tbl, usw_fib_hash_ipmc_ipv6_key_tbl,
    usw_fib_hash_fcoe_key_tbl, usw_fib_hash_trill_key_tbl, NULL
};

tbls_id_t* usw_scl_hash_key_tbl[] =
{
    usw_scl_hash_bridge_key_tbl,
    usw_scl_hash_ipv4_key_tbl,         usw_scl_hash_ipv6_key_tbl,
    usw_scl_hash_iptunnel_key_tbl,     usw_scl_hash_nvgre_tunnel_key_tbl,
    usw_scl_hash_vxlan_tunnel_key_tbl, usw_scl_hash_mpls_tunnel_key_tbl,
    usw_scl_hash_pbb_tunnel_key_tbl,   usw_scl_hash_trill_tunnel_key_tbl,
    usw_scl_hash_wlan_key_tbl, NULL
};

ctc_dkits_tcam_key_inf_t* usw_acl_tcam_key_inf[] =
{
    usw_acl_tcam_bridge_key_inf,    usw_acl_tcam_ipv4_key_inf,
    usw_acl_tcam_ipv6_key_inf,      usw_acl_tcam_l2l3_key_inf,
    usw_acl_tcam_interface_key_inf, usw_acl_tcam_fwd_key_inf,
    usw_acl_tcam_category_key_inf,      NULL
};

ctc_dkits_tcam_key_inf_t* usw_scl_tcam_key_inf[] =
{
    usw_scl_tcam_bridge_key_inf, usw_scl_tcam_ipv4_key_inf,
    usw_scl_tcam_ipv6_key_inf,   usw_scl_tcam_l2l3_key_inf,
    usw_scl_tcam_userid_key_inf, NULL
};

ctc_dkits_tcam_key_inf_t* usw_ip_tcam_key_inf[] =
{
    usw_ip_tcam_prefix_key_inf,
    usw_ip_tcam_nat_ipv4_key_inf, usw_ip_tcam_nat_ipv6_key_inf,
    usw_ip_tcam_pbr_ipv4_key_inf, usw_ip_tcam_pbr_ipv6_key_inf, NULL
};

ctc_dkits_tcam_key_inf_t* usw_cid_tcam_key_inf[] =
{
    usw_static_cid_tcam_key_inf, NULL
};

ctc_dkits_tcam_key_inf_t* usw_qmgr_tcam_key_inf[] =
{
    usw_static_qmgr_tcam_key_inf, NULL
};

tbls_id_t* usw_ipfix_hash_key_tbl[] =
{
    usw_ipfix_hash_bridge_key_tbl, usw_ipfix_hash_l2l3_key_tbl, usw_ipfix_hash_ipv4_key_tbl,
    usw_ipfix_hash_ipv6_key_tbl, usw_ipfix_hash_mpls_key_tbl, NULL
};

tbls_id_t* usw_flow_hash_key_tbl[] =
{
    usw_flow_hash_bridge_key_tbl, usw_flow_hash_l2l3_key_tbl, usw_flow_hash_ipv4_key_tbl,
    usw_flow_hash_ipv6_key_tbl, usw_flow_hash_mpls_key_tbl, NULL
};

tbls_id_t* usw_nh_tbl[] =
{
    usw_nh_fwd_tbl, usw_nh_met_tbl, usw_nh_nexthop_tbl, usw_nh_edit_tbl, NULL
};

tbls_id_t* usw_xcoam_hash_tbl[] =
{
    usw_xcoam_hash_scl_tbl,
    usw_xcoam_hash_tunnel_tbl,
    usw_xcoam_hash_oam_tbl,
    NULL
};

tbls_id_t **usw_hash_tbl[] =
{
    usw_fib_hash_key_tbl, usw_flow_hash_key_tbl, usw_scl_hash_key_tbl, usw_xcoam_hash_tbl, usw_ipfix_hash_key_tbl
};

ctc_dkits_hash_key_filed_t usw_hash_key_field[CTC_DKITS_HASH_DB_MODULE_NUM] =
{
    {DsEgressXcOamPortCrossHashKey_t, DsEgressXcOamPortCrossHashKey_hashType_f, DsEgressXcOamPortCrossHashKey_valid_f},
    {DsFibHost0MacHashKey_t,          DsFibHost0MacHashKey_hashType_f,          DsFibHost0MacHashKey_valid_f},
    {DsFibHost1Ipv4McastHashKey_t,    DsFibHost1Ipv4McastHashKey_hashType_f,    DsFibHost1Ipv4McastHashKey_valid_f},
    {DsFlowL2HashKey_t,               DsFlowL2HashKey_hashKeyType_f,            DsFlowL2HashKey_hashKeyType_f},
    {DsIpfixL2HashKey_t,              DsIpfixL2HashKey_hashKeyType_f,           DsIpfixL2HashKey_hashKeyType_f},
    {DsUserIdCvlanCosPortHashKey_t,   DsUserIdCvlanCosPortHashKey_hashType_f,   DsUserIdCvlanCosPortHashKey_valid_f}
};

ctc_dkits_tcam_key_inf_t** usw_tcam_key_inf[] =
{
    usw_scl_tcam_key_inf, usw_acl_tcam_key_inf, usw_ip_tcam_key_inf, usw_cid_tcam_key_inf, usw_qmgr_tcam_key_inf
};

struct ctc_dkits_dump_tbl_block_data_s
{
    uint32 entry;
    uint8  repeat;
}__attribute__((packed));
typedef struct ctc_dkits_dump_tbl_block_data_s ctc_dkits_dump_tbl_block_data_t;
extern ctc_dkit_chip_api_t* g_dkit_chip_api[CTC_DKITS_MAX_LOCAL_CHIP_NUM];
#define ________DKIT_DUMP_INTERNAL_FUNCTION_________

#define _____COMMON_____

void
ctc_usw_dkits_dump_get_tbl_type(uint8 lchip, tbls_id_t, ctc_dkits_dump_tbl_type_t*);

void
ctc_usw_dkits_dump_diff_status(uint8 lchip, uint8 reset, tbls_id_t* p_tblid, uint32* p_idx)
{
    static tbls_id_t lastest_tblid = MaxTblId_t;
    static uint32 lastest_tblidx = 0xFFFFFFFF;
    if (reset)
    {
        lastest_tblid = MaxTblId_t;
        lastest_tblidx = 0xFFFFFFFF;
    }
    if (NULL != p_tblid)
    {
        if (MaxTblId_t == *p_tblid)
        {
            *p_tblid = lastest_tblid;
        }
        else
        {
            lastest_tblid = *p_tblid;
        }
    }
    if  (NULL != p_idx)
    {
        if (0xFFFFFFFF == *p_idx)
        {
            *p_idx = lastest_tblidx;
        }
        else
        {
            lastest_tblidx = *p_idx;
        }
    }

    return;
}

void
ctc_usw_dkits_dump_diff_write(uint8 lchip, tbls_id_t tblid_, uint32 idx_, uint32 bits, char* p_field_name, uint32* p_src_value, uint32* p_dst_value, sal_file_t p_wf)
{
    tbls_id_t tblid = MaxTblId_t;
    uint32  fvi = 0;
    uint32  fwn = 0;
    uint32  idx = 0xFFFFFFFF;
    char str[100] = {0};
    char str2[100] = {0};
    char format[50] = {0};
    char format2[50] = {0};

    fwn = (bits / (sizeof(uint32) * 8)) + ((bits % (sizeof(uint32) * 8)) ? 1 : 0);
    ctc_usw_dkits_dump_diff_status(lchip, 0, &tblid, &idx);

    for (fvi = 0; fvi < fwn; fvi++)
    {
        if (0 == fvi)
        {
            if ((tblid != tblid_) || (idx != idx_))
            {
                tblid = tblid_;
                idx = idx_;
                ctc_usw_dkits_dump_diff_status(lchip, 0, &tblid, &idx);

                sal_sprintf(str, "%s[%u]", TABLE_NAME(lchip, tblid), idx);
                ctc_dkits_dump_cfg_brief_str(str, sal_strlen(TABLE_NAME(lchip, tblid)), ((int8)(sal_strlen(str)) - TBL_NAME_IDX_MAX_PRINT_LEN));
                sal_sprintf(str2, "%s", p_field_name);
                ctc_dkits_dump_cfg_brief_str(str2, sal_strlen(p_field_name), ((int8)(sal_strlen(str2)) - FIELD_NAME_MAX_PRINT_LEN));

                sal_sprintf(format, "%%-%ds%%-%ds", TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN,
                                                    FIELD_NAME_MAX_PRINT_LEN + COLUNM_SPACE_LEN);

                sal_fprintf(p_wf, format, str, str2);
                sal_sprintf(format2, "%%-%ds", DUMP_ENTRY_WORD_STR_LEN + DUMP_ENTRY_WORD_SPACE_LEN);
                sal_fprintf(p_wf, format2, CTC_DKITS_DUMP_HEX(str, format, p_src_value[fvi], 8));
                sal_fprintf(p_wf, format2, CTC_DKITS_DUMP_HEX(str, format, p_dst_value[fvi], 8));
                sal_fprintf(p_wf, "\n");
            }
            else
            {
                sal_sprintf(str2, "%s", p_field_name);
                ctc_dkits_dump_cfg_brief_str(str2, sal_strlen(p_field_name), ((int8)(sal_strlen(str2)) - FIELD_NAME_MAX_PRINT_LEN));

                sal_sprintf(format, "%%-%ds%%-%ds", TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN,
                                                    FIELD_NAME_MAX_PRINT_LEN + COLUNM_SPACE_LEN);

                sal_fprintf(p_wf, format, " ", str2);
                sal_sprintf(format2, "%%-%ds", DUMP_ENTRY_WORD_STR_LEN + DUMP_ENTRY_WORD_SPACE_LEN);
                sal_fprintf(p_wf, format2, CTC_DKITS_DUMP_HEX(str, format, p_src_value[fvi], 8));
                sal_fprintf(p_wf, format2, CTC_DKITS_DUMP_HEX(str, format, p_dst_value[fvi], 8));
                sal_fprintf(p_wf, "\n");
            }
        }
        else
        {
            sal_sprintf(format, "%%-%ds%%-%ds", TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN,
                                                FIELD_NAME_MAX_PRINT_LEN + COLUNM_SPACE_LEN);

            sal_fprintf(p_wf, format, " ", " ");
            sal_sprintf(format2, "%%-%ds", DUMP_ENTRY_WORD_STR_LEN + DUMP_ENTRY_WORD_SPACE_LEN);
            sal_fprintf(p_wf, format2, CTC_DKITS_DUMP_HEX(str, format, p_src_value[fvi], 8));
            sal_fprintf(p_wf, format2, CTC_DKITS_DUMP_HEX(str, format, p_dst_value[fvi], 8));
            sal_fprintf(p_wf, "\n");
        }
    }

    return;
}

int32
ctc_usw_dkits_dump_diff_field(uint8 lchip, ctc_dkits_dump_key_tbl_info_t* p_key_tbl_info, ctc_dkits_dump_tbl_entry_t* p_src_tbl, ctc_dkits_dump_tbl_entry_t* p_dst_tbl, sal_file_t p_wf)
{
    uint32 tfi = 0, fwn = 0, fvi = 0, is_diff = 0;
    uint32* p_src_value = NULL;
    uint32* p_dst_value = NULL;
    fields_t* p_fld_ptr = NULL;
    tables_info_t* p_tbl_ptr = NULL;
    int32 ret = DKIT_E_NONE;

    p_src_value = (uint32*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(uint32)*MAX_ENTRY_WORD);
    p_dst_value = (uint32*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(uint32)*MAX_ENTRY_WORD);
    if ((NULL == p_src_value) || (NULL == p_dst_value))
    {
        ret = DKIT_E_NO_MEMORY;
        goto out;
    }
    sal_memset(p_src_value, 0 , sizeof(uint32)*MAX_ENTRY_WORD);
    sal_memset(p_dst_value, 0 , sizeof(uint32)*MAX_ENTRY_WORD);
    p_tbl_ptr = TABLE_INFO_PTR(lchip, p_key_tbl_info->tblid);

    for (tfi = 0; tfi < TABLE_FIELD_NUM(lchip, p_key_tbl_info->tblid); tfi++)
    {
        p_fld_ptr = &(p_tbl_ptr->ptr_fields[tfi]);

        sal_memset(p_src_value, 0 , sizeof(uint32)*MAX_ENTRY_WORD);
        sal_memset(p_dst_value, 0 , sizeof(uint32)*MAX_ENTRY_WORD);
        drv_get_field(lchip, p_key_tbl_info->tblid, tfi, p_src_tbl->data_entry, p_src_value);
        drv_get_field(lchip, p_key_tbl_info->tblid, tfi, p_dst_tbl->data_entry, p_dst_value);

        fwn = (CTC_DKITS_FIELD_LEN(p_fld_ptr) / (sizeof(uint32) * 8)) + ((CTC_DKITS_FIELD_LEN(p_fld_ptr) % (sizeof(uint32) * 8)) ? 1 : 0);
        for (fvi = 0; fvi < fwn; fvi++)
        {
            if (p_src_value[fvi] != p_dst_value[fvi])
            {
                is_diff = 1;
                break;
            }
        }

        if (is_diff)
        {
            is_diff = 0;
            fwn = (CTC_DKITS_FIELD_LEN(p_fld_ptr) / (sizeof(uint32) * 8)) + ((CTC_DKITS_FIELD_LEN(p_fld_ptr) % (sizeof(uint32) * 8)) ? 1 : 0);

            sal_memset(p_src_value, 0 , sizeof(uint32)*MAX_ENTRY_WORD);
            sal_memset(p_dst_value, 0 , sizeof(uint32)*MAX_ENTRY_WORD);
            drv_get_field(lchip, p_key_tbl_info->tblid, tfi, p_src_tbl->data_entry, p_src_value);
            drv_get_field(lchip, p_key_tbl_info->tblid, tfi, p_dst_tbl->data_entry, p_dst_value);

            ctc_usw_dkits_dump_diff_write(lchip, p_key_tbl_info->tblid, p_key_tbl_info->idx, (uint32)CTC_DKITS_FIELD_LEN(p_fld_ptr),
                                       p_fld_ptr->ptr_field_name, p_src_value, p_dst_value, p_wf);
        }
    }

out:
    if (p_src_value)
    {
        mem_free(p_src_value);
    }
    if (p_dst_value)
    {
        mem_free(p_dst_value);
    }
    return ret;
}

STATIC int32
_ctc_usw_dkits_dump_select_format(uint8 lchip, ctc_dkits_dump_tbl_block_hdr_t* p_tbl_block_hdr, uint32* p_data_entry, uint32* p_mask_entry)
{
    uint8  repeat = 0;
    uint16 word = 0;
    uint32 tblid = CTC_DKITS_DUMP_BLOCK_HDR_TBLID(p_tbl_block_hdr);

    word = TABLE_ENTRY_SIZE(lchip, tblid) / 4;

    ctc_dkits_dump_evaluate_entry(word, p_data_entry, &repeat);
    if (p_mask_entry)
    {
        ctc_dkits_dump_evaluate_entry(word, p_mask_entry, &repeat);
    }

    if ((repeat * 4) > (word - repeat))
    {
        p_tbl_block_hdr->format = CTC_DKITS_FORMAT_TYPE_4BYTE;
    }
    else
    {
        p_tbl_block_hdr->format = CTC_DKITS_FORMAT_TYPE_NONE;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_dump_format_entry(uint8 lchip, ctc_dkits_dump_tbl_block_hdr_t* p_tbl_block_hdr, uint32* p_entry, uint8* p_buf, uint16* p_buf_len)
{
    ctc_dkits_dump_tbl_block_data_t tbl_block_data;
    uint32 i = 0, j = 0, entry = 0;
    uint8  repeat = 0;
    uint32 tblid = CTC_DKITS_DUMP_BLOCK_HDR_TBLID(p_tbl_block_hdr);

    for (i = 0; i < TABLE_ENTRY_SIZE(lchip, tblid) / 4;)
    {
        repeat = 0;
        entry= *(p_entry + i);

        sal_memset(&tbl_block_data, 0, sizeof(ctc_dkits_dump_tbl_block_data_t));
        /* compress all zero word */
        for (j = i + 1; j < TABLE_ENTRY_SIZE(lchip, tblid) / 4; j++)
        {
            if (entry != *(p_entry + j))
            {
                break;
            }

            if (entry == *(p_entry + j))
            {
                repeat++;
            }
        }
        if (0 != repeat)
        {
            tbl_block_data.repeat = repeat;
            i = j;
        }
        else
        {
            i++;
        }
        tbl_block_data.repeat++;
        tbl_block_data.entry = entry;

        sal_memcpy(p_buf + *p_buf_len, &tbl_block_data, sizeof(ctc_dkits_dump_tbl_block_data_t));
        *p_buf_len += sizeof(ctc_dkits_dump_tbl_block_data_t);
    }

    return CLI_SUCCESS;
}

STATIC void
 _ctc_usw_dkits_dump_cmp_tbl_entry(uint8 lchip, ctc_dkits_dump_key_tbl_info_t* p_key_tbl_info,
                               ctc_dkits_dump_tbl_entry_t* p_src_entry,
                               ctc_dkits_dump_tbl_entry_t* p_dst_entry,
                               ctc_dkits_dump_tbl_diff_flag_t* p_diff_flag)
{
    uint32 i = 0;

    *p_diff_flag = 0;
    if (MaxTblId_t != p_key_tbl_info->tblid)
    {
        for (i = 0; i < (TABLE_ENTRY_SIZE(lchip, p_key_tbl_info->tblid) / DRV_BYTES_PER_WORD); i++)
        {
            if ((p_src_entry->data_entry[i] != p_dst_entry->data_entry[i])
               || (p_src_entry->mask_entry[i] != p_dst_entry->mask_entry[i]))
            {
                *p_diff_flag |= CTC_DKITS_DUMP_TBL_DIFF_FLAG_KEY;
            }
        }
    }
    if (MaxTblId_t != p_key_tbl_info->ad_tblid)
    {
        for (i = 0; i < (TABLE_ENTRY_SIZE(lchip, p_key_tbl_info->ad_tblid) / DRV_BYTES_PER_WORD); i++)
        {
            if (p_src_entry->ad_entry[i] != p_dst_entry->ad_entry[i])
            {
                *p_diff_flag |= CTC_DKITS_DUMP_TBL_DIFF_FLAG_AD;
            }
        }
    }

    return;
}

STATIC void
_ctc_usw_dkits_dump_diff_ctl(uint8 lchip, tbls_id_t tblid, uint8 tbl_type, uint32* p_line_word, uint32* p_tbl_word)
{
    if ((CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY == tbl_type)
        || (CTC_DKITS_DUMP_TBL_TYPE_HASH_NO_AD== tbl_type)
        || (CTC_DKITS_DUMP_TBL_TYPE_HASH_AD == tbl_type))
    {
        *p_line_word = DIFF_HASH_WORD_NUM_PER_LINE;
        *p_tbl_word = (TABLE_ENTRY_SIZE(lchip, tblid) / DRV_BYTES_PER_WORD) * 2;/* hash key&ad */
    }
    else if ((CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == tbl_type)
            || (CTC_DKITS_DUMP_TBL_TYPE_TCAM_AD == tbl_type))
    {
        *p_line_word = DIFF_TCAM_WORD_NUM_PER_LINE;
        *p_tbl_word = (TABLE_ENTRY_SIZE(lchip, tblid) / DRV_BYTES_PER_WORD) * 4;/* key&mask&ad*/
    }

    return;
}

STATIC uint32
_ctc_usw_dkits_dump_diff_check(uint8 lchip, tbls_id_t tblid, uint8 tbl_type,
                           ctc_dkits_dump_tbl_entry_t* p_src_entry,
                           ctc_dkits_dump_tbl_entry_t* p_dst_entry)
{
    if (((CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY == tbl_type)
       || (CTC_DKITS_DUMP_TBL_TYPE_HASH_NO_AD== tbl_type)
       || (CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == tbl_type)))
    {
        if ((NULL != p_src_entry) && (NULL == p_src_entry->data_entry))
        {
            CTC_DKIT_PRINT("Error, diff %s src data is null!\n", TABLE_NAME(lchip, tblid));
            goto CTC_DKITS_DUMP_DIFF_CHECK_ERROR_END;
        }
        if ((NULL != p_dst_entry) && (NULL == p_dst_entry->data_entry))
        {
            CTC_DKIT_PRINT("Error, diff %s dst data is null!\n", TABLE_NAME(lchip, tblid));
            goto CTC_DKITS_DUMP_DIFF_CHECK_ERROR_END;
        }
    }

    if (CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == tbl_type)
    {
        if ((NULL != p_src_entry) && (NULL == p_src_entry->mask_entry))
        {
            CTC_DKIT_PRINT("Error, diff %s src mask is null!\n", TABLE_NAME(lchip, tblid));
            goto CTC_DKITS_DUMP_DIFF_CHECK_ERROR_END;
        }
        if ((NULL != p_dst_entry) && (NULL == p_dst_entry->mask_entry))
        {
            CTC_DKIT_PRINT("Error, diff %s dst mask is null!\n", TABLE_NAME(lchip, tblid));
            goto CTC_DKITS_DUMP_DIFF_CHECK_ERROR_END;
        }
    }

    if ((CTC_DKITS_DUMP_TBL_TYPE_HASH_AD == tbl_type)
       || (CTC_DKITS_DUMP_TBL_TYPE_TCAM_AD == tbl_type))
    {
        if ((NULL != p_src_entry) && (NULL == p_src_entry->data_entry))
        {
            CTC_DKIT_PRINT("Error, diff %s src ad is null!\n", TABLE_NAME(lchip, tblid));
            goto CTC_DKITS_DUMP_DIFF_CHECK_ERROR_END;
        }
        if ((NULL != p_dst_entry) && (NULL == p_dst_entry->data_entry))
        {
            CTC_DKIT_PRINT("Error, diff %s dst ad is null!\n", TABLE_NAME(lchip, tblid));
            goto CTC_DKITS_DUMP_DIFF_CHECK_ERROR_END;
        }
    }

    return 1;

CTC_DKITS_DUMP_DIFF_CHECK_ERROR_END:

    return 0;
}

STATIC void
_ctc_usw_dkits_dump_diff_entry(uint8 lchip, uint32 idx, tbls_id_t tblid, uint32* p_word_idx, uint8 tbl_type,
                           ctc_dkits_dump_tbl_entry_t* p_src_entry,
                           ctc_dkits_dump_tbl_entry_t* p_dst_entry,
                           uint32** pp_entry, uint32* p_is_empty)
{
    ctc_dkits_dump_tbl_entry_t* p_cmp_entry = NULL;
    static uint32 src_data_word = 0, dst_data_word = 0, src_mask_word = 0, dst_mask_word = 0;
    static uint32 line_word = 0, tbl_word = 0, is_empty = 0;
    static uint32 alter = 0, shift = 0;
    uint32* p_data_word = NULL;
    uint32* p_mask_word = NULL;

    *p_is_empty = 0;

    if (0 == idx)
    {
        src_data_word = 0;
        dst_data_word = 0;
        src_mask_word = 0;
        dst_mask_word = 0;
        is_empty = 0;
        if ((CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == tbl_type)
           || (CTC_DKITS_DUMP_TBL_TYPE_TCAM_AD == tbl_type))
        {
            alter = DIFF_TCAM_WORD_ALTER_NUM;
            shift = alter * 2;

        }
        else if ((CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY == tbl_type)
                || (CTC_DKITS_DUMP_TBL_TYPE_HASH_AD == tbl_type)
                || (CTC_DKITS_DUMP_TBL_TYPE_HASH_NO_AD== tbl_type))
        {
            alter = DIFF_HASH_WORD_ALTER_NUM;
            shift = alter;
        }
        _ctc_usw_dkits_dump_diff_ctl(lchip, tblid, tbl_type, &line_word, &tbl_word);
    }

    if (0 == (idx % alter))
    {
        if (0 == ((idx / shift) % 2))
        {
            p_cmp_entry = p_src_entry;
            p_data_word = &src_data_word;
            p_mask_word = &src_mask_word;
        }
        else
        {
            p_cmp_entry = p_dst_entry;
            p_data_word = &dst_data_word;
            p_mask_word = &dst_mask_word;
        }

        if ((CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY == tbl_type)
            || (CTC_DKITS_DUMP_TBL_TYPE_HASH_NO_AD== tbl_type)
           || (CTC_DKITS_DUMP_TBL_TYPE_HASH_AD == tbl_type))
        {
            if (NULL != p_cmp_entry)
            {
                *p_word_idx = *p_data_word;
                *p_data_word += alter;
                *pp_entry = (CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY == tbl_type || CTC_DKITS_DUMP_TBL_TYPE_HASH_NO_AD == tbl_type) ?
                             p_cmp_entry->data_entry : p_cmp_entry->ad_entry;
            }
            else
            {
                *pp_entry = NULL;
                is_empty = 0;
            }
        }
        else if (CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == tbl_type)
        {
            if (NULL != p_cmp_entry)
            {
                if (0 == ((idx / alter) % 2))
                {
                    *p_word_idx = *p_data_word;
                    *p_data_word += alter;
                    *pp_entry = p_cmp_entry->data_entry;
                }
                else
                {
                    *p_word_idx = *p_mask_word;
                    *p_mask_word += alter;
                    *pp_entry = p_cmp_entry->mask_entry;
                }
            }
            else
            {
                *pp_entry = NULL;
                is_empty = 0;
            }
        }
        else if (CTC_DKITS_DUMP_TBL_TYPE_TCAM_AD == tbl_type)
        {
            if (NULL != p_cmp_entry)
            {
                if (0 == ((idx / alter) % 2))
                {
                    *p_word_idx = *p_data_word;
                    *p_data_word += alter;
                    *pp_entry = p_cmp_entry->ad_entry;
                }
                else
                {
                    *pp_entry = NULL;
                    is_empty = 1;
                }
            }
            else
            {
                *pp_entry = NULL;
                if (0 == ((idx / alter) % 2))
                {
                    is_empty = 0;
                }
                else
                {
                    is_empty = 1;
                }
            }
        }
    }
    else
    {
        (*p_word_idx)++;
    }

    *p_is_empty = is_empty;

    return;
}

STATIC int32
_ctc_usw_dkits_dump_diff_key(uint8 lchip, ctc_dkits_dump_key_tbl_info_t* p_key_tbl_info, ctc_dkits_dump_tbl_entry_t* p_src_entry, ctc_dkits_dump_tbl_entry_t* p_dst_entry, sal_file_t p_wf)
{
    uint32 i = 0, j = 0, valid = 0;
    tbls_id_t tblid[] = {p_key_tbl_info->tblid, p_key_tbl_info->ad_tblid, MaxTblId_t};
    uint32 idx[2] = {p_key_tbl_info->idx, p_key_tbl_info->ad_idx};
    char str[300] = {0};
    char tbl[300] = {0};
    char format[100] = {0};
    char format2[100] = {0};
    uint32* p_entry = NULL;
    uint32 line_word = 0, tbl_word = 0, word_idx = 0, is_empty = 0;
    ctc_dkits_dump_tbl_type_t tbl_type = p_key_tbl_info->tbl_type;

    for (i = 0; MaxTblId_t != tblid[i]; i++)
    {
        if ((CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY == p_key_tbl_info->tbl_type) && i)
        {
            tbl_type = CTC_DKITS_DUMP_TBL_TYPE_HASH_AD;
        }

        if ((CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == p_key_tbl_info->tbl_type) && i)
        {
            tbl_type = CTC_DKITS_DUMP_TBL_TYPE_TCAM_AD;
        }

        valid = _ctc_usw_dkits_dump_diff_check(lchip, tblid[i], tbl_type, p_src_entry, p_dst_entry);
        if (!valid)
        {
            continue;
        }
        _ctc_usw_dkits_dump_diff_ctl(lchip, tblid[i], tbl_type, &line_word, &tbl_word);

        for (j = 0; j < tbl_word; j++)
        {
            _ctc_usw_dkits_dump_diff_entry(lchip, j, tblid[i], &word_idx, tbl_type,
                                       p_src_entry, p_dst_entry, &p_entry, &is_empty);

            if (0 == (j % line_word))
            {
                sal_sprintf(format2, "%%-%ds%%-%ds", TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN, DUMP_ENTRY_WORD_STR_LEN + DUMP_ENTRY_WORD_SPACE_LEN);
                if (0 == j)
                {
                    sal_sprintf(tbl, "%s[%u]", TABLE_NAME(lchip, tblid[i]), idx[i]);
                    ctc_dkits_dump_cfg_brief_str(tbl, sal_strlen(TABLE_NAME(lchip, tblid[i])), ((int8)(sal_strlen(tbl)) - TBL_NAME_IDX_MAX_PRINT_LEN));
                    if (NULL != p_entry)
                    {
                        sal_fprintf(p_wf, format2, tbl, CTC_DKITS_DUMP_HEX(str, format, p_entry[word_idx], 8));
                    }
                    else
                    {
                        sal_fprintf(p_wf, format2, tbl, is_empty ? " " : "--------");
                    }
                }
                else
                {
                    if (NULL != p_entry)
                    {
                        sal_fprintf(p_wf, format2, " ", CTC_DKITS_DUMP_HEX(str, format, p_entry[word_idx], 8));
                    }
                    else
                    {
                        sal_fprintf(p_wf, format2, " ", is_empty ? " " : "--------");
                    }
                }
            }
            else
            {
                sal_sprintf(format2, "%%-%ds", DUMP_ENTRY_WORD_STR_LEN + DUMP_ENTRY_WORD_SPACE_LEN);
                if (NULL != p_entry)
                {
                    sal_fprintf(p_wf, format2, CTC_DKITS_DUMP_HEX(str, format, p_entry[word_idx], 8));
                }
                else
                {
                    sal_fprintf(p_wf, format2, is_empty ? " " : "--------");
                }
            }
            if (0 == ((j + 1) % line_word))
            {
                sal_fprintf(p_wf, "%s", "\n");
            }
        }
        if (0 != j % line_word)
        {
            sal_fprintf(p_wf, "\n");
        }
    }

    return CLI_SUCCESS;
}

/*p_flag: bit0--hash header, bit1--tcam header*/
STATIC void
_ctc_usw_dkits_dump_diff(uint8 lchip, ctc_dkits_dump_tbl_type_t tbl_type, ctc_dkits_dump_key_tbl_info_t* p_key_tbl_info, ctc_dkits_dump_tbl_entry_t* p_src_tbl,
    ctc_dkits_dump_tbl_entry_t* p_dst_tbl, uint32* p_flag, sal_file_t p_wf)
{
    ctc_dkits_dump_tbl_diff_flag_t diff_flag = 0;

    if ((NULL != p_src_tbl) && (NULL != p_dst_tbl))
    {
        _ctc_usw_dkits_dump_cmp_tbl_entry(lchip, p_key_tbl_info, p_src_tbl, p_dst_tbl, &diff_flag);
        if (!(DKITS_FLAG_ISSET(diff_flag, CTC_DKITS_DUMP_TBL_DIFF_FLAG_TBL)
           || DKITS_FLAG_ISSET(diff_flag, CTC_DKITS_DUMP_TBL_DIFF_FLAG_KEY)
           || DKITS_FLAG_ISSET(diff_flag, CTC_DKITS_DUMP_TBL_DIFF_FLAG_AD)))
        {
            return;
        }
    }

    /*Dump header*/
    if ((CTC_DKITS_DUMP_TBL_TYPE_HASH_NO_AD== tbl_type) || (CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY == tbl_type))
    {
        if (!DKITS_IS_BIT_SET(*p_flag, 0))
        {
            DKITS_BIT_SET(*p_flag, 0);
            ctc_dkits_dump_txt_header(CTC_DKITS_DUMP_TEXT_HEADER_DIFF_HASH, p_wf);
        }
    }
    else if (CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == tbl_type)
    {
        if (!DKITS_IS_BIT_SET(*p_flag, 1))
        {
            DKITS_BIT_SET(*p_flag, 1);
            ctc_dkits_dump_txt_header(CTC_DKITS_DUMP_TEXT_HEADER_DIFF_TCAM, p_wf);
        }
    }

    if ((CTC_DKITS_DUMP_TBL_TYPE_STATIC == tbl_type) || (CTC_DKITS_DUMP_TBL_TYPE_SERDES == tbl_type))
    {
        if ((NULL == p_src_tbl) || (NULL == p_dst_tbl))
        {
            CTC_DKIT_PRINT("Diff static tbl field error, ");
            if (NULL == p_src_tbl)
            {
                CTC_DKIT_PRINT("src tbl is NULL, ");
            }
            if (NULL == p_dst_tbl)
            {
                CTC_DKIT_PRINT("dst tbl is NULL, ");
            }
            CTC_DKIT_PRINT("tblName:%s, index:%u\n", TABLE_NAME(lchip, p_key_tbl_info->tblid), p_key_tbl_info->idx);
            return;
        }

        if (CTC_DKITS_DUMP_TBL_TYPE_SERDES == tbl_type)
        {
            ctc_dkits_dump_diff_flex(p_key_tbl_info, p_src_tbl, p_dst_tbl, p_wf);
        }
        else
        {
            ctc_usw_dkits_dump_diff_field(lchip, p_key_tbl_info, p_src_tbl, p_dst_tbl, p_wf);
        }

    }
    else if ((CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY == tbl_type)
            || (CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == tbl_type)
            || (CTC_DKITS_DUMP_TBL_TYPE_HASH_NO_AD == tbl_type))
    {
        if ((NULL == p_src_tbl) && (NULL == p_dst_tbl))
        {
            CTC_DKIT_PRINT("Diff key tbl error!\n");
            return;
        }
        _ctc_usw_dkits_dump_diff_key(lchip, p_key_tbl_info, p_src_tbl, p_dst_tbl, p_wf);
    }

    return;
}

int32
ctc_usw_dkits_dump_tbl2data(uint8 lchip, uint16 tbl_id, uint16 index, uint32* p_data_entry, uint32* p_mask_entry, sal_file_t p_f)
{
    uint8  data[500] = {0}, is_tcam = 0;
    uint16 mem_len = 0;
    ctc_dkits_dump_tbl_block_hdr_t tbl_block_hdr;

    sal_memset(&tbl_block_hdr, 0, sizeof(ctc_dkits_dump_tbl_block_hdr_t));

    tbl_block_hdr.tblid_0_7 = tbl_id & 0xFF;
    tbl_block_hdr.tblid_8_12 = (tbl_id >> 8) & 0x1F;
    tbl_block_hdr.idx = index;
    is_tcam = (NULL == p_mask_entry) ? 0 : 1;

    _ctc_usw_dkits_dump_select_format(lchip, &tbl_block_hdr, p_data_entry, p_mask_entry);

    if (CTC_DKITS_FORMAT_TYPE_4BYTE == tbl_block_hdr.format)
    {
        sal_memset(data, 0, sizeof(data));
        _ctc_usw_dkits_dump_format_entry(lchip, &tbl_block_hdr, p_data_entry, data, &mem_len);

        if (1 == is_tcam)
        {
            _ctc_usw_dkits_dump_format_entry(lchip, &tbl_block_hdr, p_mask_entry, data, &mem_len);
        }
        tbl_block_hdr.entry_num = (mem_len / sizeof(ctc_dkits_dump_tbl_block_data_t)) - 1;
        sal_fwrite(&tbl_block_hdr, sizeof(ctc_dkits_dump_tbl_block_hdr_t), 1, p_f);
        sal_fwrite(data, mem_len, 1, p_f);
    }
    else
    {
        sal_memset(data, 0, sizeof(data));
        ctc_dkits_dump_plain_entry((TABLE_ENTRY_SIZE(lchip, tbl_id)/4), p_data_entry, data, &mem_len);

        if (1 == is_tcam)
        {
            ctc_dkits_dump_plain_entry((TABLE_ENTRY_SIZE(lchip, tbl_id)/4), p_mask_entry, data, &mem_len);
        }
        tbl_block_hdr.entry_num = (mem_len / sizeof(uint32)) - 1;
        sal_fwrite(&tbl_block_hdr, sizeof(ctc_dkits_dump_tbl_block_hdr_t), 1, p_f);
        sal_fwrite(data, mem_len, 1, p_f);
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_dkits_dump_cmp_tbl(uint8 lchip, sal_file_t p_srf, sal_file_t p_drf, uint8 src_cfg_endian, uint8 dst_cfg_endian, sal_file_t p_wf)
{
    uint32 read_src = 1, read_dst = 1, src_end = 0, dst_end = 0;
    uint32 seop = 0, deop = 0;
    uint32* src_data_entry = NULL;
    uint32* src_mask_entry = NULL;
    uint32* src_ad_entry = NULL;
    uint32* dst_data_entry = NULL;
    uint32* dst_mask_entry = NULL;
    uint32* dst_ad_entry = NULL;
    ctc_dkits_dump_tbl_entry_t src_tbl_entry, dst_tbl_entry;
    ctc_dkits_dump_key_tbl_info_t src_key_tbl_info, dst_key_tbl_info;
    ctc_dkits_dump_key_tbl_info_t ad_tbl_info;
    int32 ret = 0;
    uint32 flag = 0;

    src_data_entry = (uint32*)sal_malloc(MAX_ENTRY_WORD * sizeof(uint32));
    src_mask_entry = (uint32*)sal_malloc(MAX_ENTRY_WORD * sizeof(uint32));
    src_ad_entry = (uint32*)sal_malloc(MAX_ENTRY_WORD * sizeof(uint32));
    dst_data_entry = (uint32*)sal_malloc(MAX_ENTRY_WORD * sizeof(uint32));
    dst_mask_entry = (uint32*)sal_malloc(MAX_ENTRY_WORD * sizeof(uint32));
    dst_ad_entry = (uint32*)sal_malloc(MAX_ENTRY_WORD * sizeof(uint32));

    if (!src_data_entry || !src_mask_entry || !src_ad_entry || !dst_data_entry || !dst_mask_entry || !dst_ad_entry)
    {
        ret = CLI_ERROR;
        goto error_return;
    }

    sal_memset(&src_tbl_entry, 0, sizeof(ctc_dkits_dump_tbl_entry_t));
    sal_memset(&dst_tbl_entry, 0, sizeof(ctc_dkits_dump_tbl_entry_t));

    src_tbl_entry.data_entry = src_data_entry;
    src_tbl_entry.mask_entry = src_mask_entry;
    src_tbl_entry.ad_entry = src_ad_entry;

    dst_tbl_entry.data_entry = dst_data_entry;
    dst_tbl_entry.mask_entry = dst_mask_entry;
    dst_tbl_entry.ad_entry = dst_ad_entry;

    ret = sal_fseek(p_srf, 0, SEEK_END);
    if(0 != ret)
    {
        return CLI_ERROR;
    }

    seop = sal_ftell(p_srf);
    if (seop < sizeof(ctc_dkits_dump_centec_file_hdr_t))
    {
        return CLI_SUCCESS;
    }
    ret = sal_fseek(p_srf, sizeof(ctc_dkits_dump_centec_file_hdr_t), SEEK_SET);
    if(0 != ret)
    {
        return CLI_ERROR;
    }

    ret = sal_fseek(p_drf, 0, SEEK_END);
    if(0 != ret)
    {
        return CLI_ERROR;
    }
    deop = sal_ftell(p_drf);
    if (deop < sizeof(ctc_dkits_dump_centec_file_hdr_t))
    {
        return CLI_SUCCESS;
    }
    ret = sal_fseek(p_drf, sizeof(ctc_dkits_dump_centec_file_hdr_t), SEEK_SET);
    if(0 != ret)
    {
        return CLI_ERROR;
    }

    while (!src_end || !dst_end)
    {
        if (read_src)
        {
            if ((1 != sal_feof(p_srf)) && (seop != sal_ftell(p_srf)))
            {
                sal_memset(src_data_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));
                sal_memset(src_mask_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));
                sal_memset(src_ad_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));

                src_key_tbl_info.tblid = MaxTblId_t;
                src_key_tbl_info.ad_tblid = MaxTblId_t;

                /*get key info*/
                ret = ctc_dkits_dump_get_tbl_entry(lchip, p_srf, src_cfg_endian, &src_tbl_entry, &src_key_tbl_info);
                if (ret)
                {
                    continue;
                }

                if (CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY == src_key_tbl_info.tbl_type)
                {
                    /*get ad info*/
                    ctc_dkits_dump_get_tbl_entry(lchip, p_srf, src_cfg_endian, &src_tbl_entry, &ad_tbl_info);
                    src_key_tbl_info.ad_tblid = ad_tbl_info.ad_tblid;
                    src_key_tbl_info.ad_idx = ad_tbl_info.ad_idx;
                }
                else if (CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == src_key_tbl_info.tbl_type)
                {
                    /*get ad info*/
                    ctc_dkits_dump_get_tbl_entry(lchip, p_srf, src_cfg_endian, &src_tbl_entry, &ad_tbl_info);
                    src_key_tbl_info.ad_tblid = ad_tbl_info.ad_tblid;
                    src_key_tbl_info.ad_idx = ad_tbl_info.ad_idx;
                }
            }
            else
            {
                src_end = 1;
            }
        }

        if (read_dst)
        {
            if ((1 != sal_feof(p_drf)) && (deop != sal_ftell(p_drf)))
            {
                sal_memset(dst_data_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));
                sal_memset(dst_mask_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));
                sal_memset(dst_ad_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));

                dst_key_tbl_info.tblid = MaxTblId_t;
                dst_key_tbl_info.ad_tblid = MaxTblId_t;

                /*get key info*/
                ret = ctc_dkits_dump_get_tbl_entry(lchip, p_drf, dst_cfg_endian, &dst_tbl_entry, &dst_key_tbl_info);
                if (ret)
                {
                    continue;
                }

                if (CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY == dst_key_tbl_info.tbl_type)
                {
                    ctc_dkits_dump_get_tbl_entry(lchip, p_drf, dst_cfg_endian, &dst_tbl_entry, &ad_tbl_info);
                    dst_key_tbl_info.ad_tblid = ad_tbl_info.ad_tblid;
                    dst_key_tbl_info.ad_idx = ad_tbl_info.ad_idx;
                }
                else if (CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == dst_key_tbl_info.tbl_type)
                {
                    ctc_dkits_dump_get_tbl_entry(lchip, p_drf, dst_cfg_endian, &dst_tbl_entry, &ad_tbl_info);
                    dst_key_tbl_info.ad_tblid = ad_tbl_info.ad_tblid;
                    dst_key_tbl_info.ad_idx = ad_tbl_info.ad_idx;
                }

            }
            else
            {
                dst_end = 1;
            }
        }

        if (!src_end && dst_end)
        {
            _ctc_usw_dkits_dump_diff(lchip, src_key_tbl_info.tbl_type, &src_key_tbl_info, &src_tbl_entry, NULL, &flag, p_wf);
            read_src = 1;
            read_dst = 0;
        }
        else if (src_end && !dst_end)
        {
            _ctc_usw_dkits_dump_diff(lchip, dst_key_tbl_info.tbl_type, &dst_key_tbl_info, NULL, &dst_tbl_entry, &flag, p_wf);
            read_src = 0;
            read_dst = 1;
        }
        else if ((!src_end && !dst_end) && (src_key_tbl_info.tblid == dst_key_tbl_info.tblid))
        {
            if (src_key_tbl_info.idx == dst_key_tbl_info.idx)
            {
                _ctc_usw_dkits_dump_diff(lchip, src_key_tbl_info.tbl_type, &src_key_tbl_info, &src_tbl_entry, &dst_tbl_entry, &flag, p_wf);
                read_src = 1;
                read_dst = 1;
            }
            else if (src_key_tbl_info.idx < dst_key_tbl_info.idx)
            {
                _ctc_usw_dkits_dump_diff(lchip, src_key_tbl_info.tbl_type, &src_key_tbl_info, &src_tbl_entry, NULL, &flag, p_wf);
                read_src = 1;
                read_dst = 0;
            }
            else if (src_key_tbl_info.idx > dst_key_tbl_info.idx)
            {
                _ctc_usw_dkits_dump_diff(lchip, dst_key_tbl_info.tbl_type, &dst_key_tbl_info, NULL, &dst_tbl_entry, &flag, p_wf);
                read_src = 0;
                read_dst = 1;
            }
        }
        else if ((!src_end && !dst_end) && (src_key_tbl_info.tblid != dst_key_tbl_info.tblid))
        {
            if (((CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY == src_key_tbl_info.tbl_type || CTC_DKITS_DUMP_TBL_TYPE_HASH_NO_AD== src_key_tbl_info.tbl_type)
                && (CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY == dst_key_tbl_info.tbl_type || CTC_DKITS_DUMP_TBL_TYPE_HASH_NO_AD== dst_key_tbl_info.tbl_type))
                ||  ((CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == src_key_tbl_info.tbl_type)
                && (CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == dst_key_tbl_info.tbl_type)))
            {
                if (src_key_tbl_info.order < dst_key_tbl_info.order)
                {
                    _ctc_usw_dkits_dump_diff(lchip, src_key_tbl_info.tbl_type, &src_key_tbl_info, &src_tbl_entry, NULL, &flag, p_wf);
                    read_src = 1;
                    read_dst = 0;
                }
                else if (src_key_tbl_info.order > dst_key_tbl_info.order)
                {
                    _ctc_usw_dkits_dump_diff(lchip, dst_key_tbl_info.tbl_type, &dst_key_tbl_info, NULL, &dst_tbl_entry, &flag, p_wf);
                    read_src = 0;
                    read_dst = 1;
                }
            }
            else if ((CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY == src_key_tbl_info.tbl_type || CTC_DKITS_DUMP_TBL_TYPE_HASH_NO_AD== src_key_tbl_info.tbl_type)
                     && (CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == dst_key_tbl_info.tbl_type))
            {
                _ctc_usw_dkits_dump_diff(lchip, src_key_tbl_info.tbl_type, &src_key_tbl_info, &src_tbl_entry, NULL, &flag, p_wf);
                read_src = 1;
                read_dst = 0;
            }
            else if ((CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == src_key_tbl_info.tbl_type)
                     && (CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY == dst_key_tbl_info.tbl_type || CTC_DKITS_DUMP_TBL_TYPE_HASH_NO_AD == dst_key_tbl_info.tbl_type))
            {
                _ctc_usw_dkits_dump_diff(lchip, dst_key_tbl_info.tbl_type, &dst_key_tbl_info, NULL, &dst_tbl_entry, &flag, p_wf);
                read_src = 0;
                read_dst = 1;
            }
        }
    }

error_return:
    if (src_data_entry)
    {
        sal_free(src_data_entry);
    }
    if (src_mask_entry)
    {
        sal_free(src_mask_entry);
    }
    if (src_ad_entry)
    {
        sal_free(src_ad_entry);
    }
    if (dst_data_entry)
    {
        sal_free(dst_data_entry);
    }
    if (dst_mask_entry)
    {
        sal_free(dst_mask_entry);
    }
    if (dst_ad_entry)
    {
        sal_free(dst_ad_entry);
    }
    return ret;
}

STATIC int32
_ctc_usw_dkits_dump_hash2module(uint8 lchip, tbls_id_t tblid, ctc_dkits_hash_db_module_t* p_module_type)
{
    if ((DsEgressXcOamBfdHashKey_t == tblid)
       || (DsEgressXcOamCvlanCosPortHashKey_t == tblid)
       || (DsEgressXcOamCvlanPortHashKey_t == tblid)
       || (DsEgressXcOamDoubleVlanPortHashKey_t == tblid)
       || (DsEgressXcOamEthHashKey_t == tblid)
       || (DsEgressXcOamMplsLabelHashKey_t == tblid)
       || (DsEgressXcOamMplsSectionHashKey_t == tblid)
       || (DsEgressXcOamPortCrossHashKey_t == tblid)
       || (DsEgressXcOamPortHashKey_t == tblid)
       || (DsEgressXcOamPortVlanCrossHashKey_t == tblid)
       || (DsEgressXcOamRmepHashKey_t == tblid)
       || (DsEgressXcOamSvlanCosPortHashKey_t == tblid)
       || (DsEgressXcOamSvlanPortHashKey_t == tblid)
       || (DsEgressXcOamSvlanPortMacHashKey_t == tblid)
       || (DsEgressXcOamTunnelPbbHashKey_t == tblid))
    {
        *p_module_type = CTC_DKITS_HASH_DB_MODULE_EGRESS_XC;
    }
    else if ((DsFibHost0Ipv4HashKey_t == tblid)
            || (DsFibHost0FcoeHashKey_t == tblid)
            || (DsFibHost0Ipv6McastHashKey_t == tblid)
            || (DsFibHost0Ipv6UcastHashKey_t == tblid)
            || (DsFibHost0MacHashKey_t == tblid)
            || (DsFibHost0MacIpv6McastHashKey_t == tblid)
            || (DsFibHost0TrillHashKey_t == tblid))
    {
        *p_module_type = CTC_DKITS_HASH_DB_MODULE_FIB_HOST0;
    }
    else if ((DsFibHost1FcoeRpfHashKey_t == tblid)
            || (DsFibHost1Ipv4McastHashKey_t == tblid)
            || (DsFibHost1Ipv4NatDaPortHashKey_t == tblid)
            || (DsFibHost1Ipv4NatSaPortHashKey_t == tblid)
            || (DsFibHost1Ipv6McastHashKey_t == tblid)
            || (DsFibHost1Ipv6NatDaPortHashKey_t == tblid)
            || (DsFibHost1Ipv6NatSaPortHashKey_t == tblid)
            || (DsFibHost1MacIpv4McastHashKey_t == tblid)
            || (DsFibHost1MacIpv6McastHashKey_t == tblid)
            || (DsFibHost1TrillMcastVlanHashKey_t == tblid))
    {
        *p_module_type = CTC_DKITS_HASH_DB_MODULE_FIB_HOST1;
    }
    else if  ((DsFlowL2HashKey_t == tblid)
             || (DsFlowL2L3HashKey_t == tblid)
             || (DsFlowL3Ipv4HashKey_t == tblid)
             || (DsFlowL3Ipv6HashKey_t == tblid)
             || (DsFlowL3MplsHashKey_t == tblid))
    {
        *p_module_type = CTC_DKITS_HASH_DB_MODULE_FLOW;
    }
    else if ((DsIpfixL2HashKey_t == tblid)
            || (DsIpfixL2L3HashKey_t == tblid)
            || (DsIpfixL3Ipv4HashKey_t == tblid)
            || (DsIpfixL3Ipv6HashKey_t == tblid)
            || (DsIpfixL3MplsHashKey_t == tblid))
    {
        *p_module_type = CTC_DKITS_HASH_DB_MODULE_IPFIX;
    }
    else if ((DsUserIdCvlanCosPortHashKey_t == tblid)
            || (DsUserIdCvlanPortHashKey_t == tblid)
            || (DsUserIdDoubleVlanPortHashKey_t == tblid)
            || (DsUserIdIpv4PortHashKey_t == tblid)
            || (DsUserIdIpv4SaHashKey_t == tblid)
            || (DsUserIdIpv6PortHashKey_t == tblid)
            || (DsUserIdIpv6SaHashKey_t == tblid)
            || (DsUserIdMacHashKey_t == tblid)
            || (DsUserIdMacPortHashKey_t == tblid)
            || (DsUserIdPortHashKey_t == tblid)
            || (DsUserIdSclFlowL2HashKey_t == tblid)
            || (DsUserIdSvlanCosPortHashKey_t == tblid)
            || (DsUserIdSvlanHashKey_t == tblid)
            || (DsUserIdSvlanMacSaHashKey_t == tblid)
            || (DsUserIdSvlanPortHashKey_t == tblid)
            || (DsUserIdTunnelIpv4DaHashKey_t == tblid)
            || (DsUserIdTunnelIpv4GreKeyHashKey_t == tblid)
            || (DsUserIdTunnelIpv4HashKey_t == tblid)
            || (DsUserIdTunnelIpv4McNvgreMode0HashKey_t == tblid)
            || (DsUserIdTunnelIpv4McVxlanMode0HashKey_t == tblid)
            || (DsUserIdTunnelIpv4NvgreMode1HashKey_t == tblid)
            || (DsUserIdTunnelIpv4RpfHashKey_t == tblid)
            || (DsUserIdTunnelIpv4UcNvgreMode0HashKey_t == tblid)
            || (DsUserIdTunnelIpv4UcNvgreMode1HashKey_t == tblid)
            || (DsUserIdTunnelIpv4UcVxlanMode0HashKey_t == tblid)
            || (DsUserIdTunnelIpv4UcVxlanMode1HashKey_t == tblid)
            || (DsUserIdTunnelIpv4UdpHashKey_t == tblid)
            || (DsUserIdTunnelIpv4VxlanMode1HashKey_t == tblid)
            || (DsUserIdTunnelIpv6McNvgreMode0HashKey_t == tblid)
            || (DsUserIdTunnelIpv6McNvgreMode1HashKey_t == tblid)
            || (DsUserIdTunnelIpv6McVxlanMode0HashKey_t == tblid)
            || (DsUserIdTunnelIpv6McVxlanMode1HashKey_t == tblid)
            || (DsUserIdTunnelIpv6UcNvgreMode0HashKey_t == tblid)
            || (DsUserIdTunnelIpv6UcNvgreMode1HashKey_t == tblid)
            || (DsUserIdTunnelIpv6UcVxlanMode0HashKey_t == tblid)
            || (DsUserIdTunnelIpv6UcVxlanMode1HashKey_t == tblid)
            || (DsUserIdTunnelMplsHashKey_t == tblid)
            || (DsUserIdTunnelPbbHashKey_t == tblid)
            || (DsUserIdTunnelTrillMcAdjHashKey_t == tblid)
            || (DsUserIdTunnelTrillMcDecapHashKey_t == tblid)
            || (DsUserIdTunnelTrillMcRpfHashKey_t == tblid)
            || (DsUserIdTunnelTrillUcDecapHashKey_t == tblid)
            || (DsUserIdTunnelTrillUcRpfHashKey_t == tblid)
            || (DsUserIdTunnelCapwapRmacHashKey_t == tblid)
            || (DsUserIdTunnelCapwapRmacRidHashKey_t == tblid)
            || (DsUserIdTunnelIpv4CapwapHashKey_t == tblid)
            || (DsUserIdTunnelIpv6CapwapHashKey_t == tblid)
            || (DsUserIdCapwapMacDaForwardHashKey_t == tblid)
            || (DsUserIdCapwapStaStatusHashKey_t == tblid)
            || (DsUserIdCapwapStaStatusMcHashKey_t == tblid)
            || (DsUserIdCapwapVlanForwardHashKey_t == tblid)
            || (DsUserIdEcidNameSpaceHashKey_t == tblid)
            || (DsUserIdIngEcidNameSpaceHashKey_t == tblid))
    {
        *p_module_type = CTC_DKITS_HASH_DB_MODULE_USERID;
    }
    else
    {
        *p_module_type = CTC_DKITS_HASH_DB_MODULE_NUM;
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_dump_cfg_brief_str(uint8 lchip, char* p_str, uint8 pos, int8 extr_len)
{
    if (extr_len < 0)
    {
        return CLI_ERROR;
    }

    sal_memcpy(p_str + pos - extr_len, p_str + pos, (sal_strlen(p_str + pos) + 1));

    return CLI_SUCCESS;
}

#if 0
STATIC uint32
_ctc_usw_dkits_dump_valid_static_tbl(uint8 lchip, tbls_id_t tblid)
{
    uint32 i = 0;

    for (i = 0; MaxTblId_t != usw_include_tbl[i]; i++)
    {
        if (tblid == usw_include_tbl[i])
        {
            return 1;
        }
    }

    return 0;
}
#endif

STATIC uint32
_ctc_usw_dkits_dump_invalid_static_tbl(uint8 lchip, tbls_id_t tblid)
{
    uint32 i = 0;
 /*-    char name[100] = {0};*/
/*
    if ((HssAnethWrapperInterruptNormal0_t <= tblid) && (tblid <= HssAnethWrapperInterruptNormal95_t))
    {
        return 1;
    }

    if ((HssAnethMon0_t <= tblid) && (tblid <= HssAnethMon95_t))
    {
        return 1;
    }

    if ((HssAnethCfg0_t <= tblid) && (tblid <= HssAnethCfg95_t))
    {
        return 1;
    }

    drv_get_table_property(0, tblid, DRV_TABLE_PROP_GET_NAME, name);

    if (sal_memcmp(name, "Aneth", 4) == 0)
    {
        return 1;
    }
*/


    for (i = 0; MaxTblId_t != usw_exclude_tbl[i]; i++)
    {
        if (tblid == usw_exclude_tbl[i])
        {
            return 1;
        }
    }

    return 0;
}

STATIC int32
_ctc_usw_dkits_dump_tcam_key2ad(uint8 lchip, tbls_id_t tblid, uint32 key_type, uint32 ad_type, tbls_id_t* p_ad_tblid)
{
    uint8 step = 0;

    *p_ad_tblid = MaxTblId_t;

    if (DKITS_TCAM_IGS_SCL0_KEY(tblid) || DKITS_TCAM_IGS_SCL1_KEY(tblid))
    {
        step = DKITS_TCAM_IGS_SCL0_KEY(tblid) ? 0 : 1;

        switch (ad_type)
        {
        case DKITS_TCAM_SCL_AD_USERID:
            *p_ad_tblid = DsUserId0Tcam_t + (DsUserId1Tcam_t - DsUserId0Tcam_t) * step;
            break;

        case DKITS_TCAM_SCL_AD_TUNNEL:
            *p_ad_tblid = DsTunnelId0Tcam_t + (DsTunnelId1Tcam_t - DsTunnelId0Tcam_t) * step;
            break;

        case DKITS_TCAM_SCL_AD_FLOW:
            *p_ad_tblid = DsSclFlow0Tcam_t + (DsSclFlow1Tcam_t - DsSclFlow0Tcam_t) * step;
            break;

        default:
            *p_ad_tblid = MaxTblId_t;
        }
    }
    else if (DKITS_TCAM_IGS_ACL0_KEY(tblid) || DKITS_TCAM_IGS_ACL1_KEY(tblid) || DKITS_TCAM_IGS_ACL2_KEY(tblid) || DKITS_TCAM_IGS_ACL3_KEY(tblid)
            || DKITS_TCAM_IGS_ACL4_KEY(tblid) || DKITS_TCAM_IGS_ACL5_KEY(tblid) || DKITS_TCAM_IGS_ACL6_KEY(tblid) || DKITS_TCAM_IGS_ACL7_KEY(tblid))
    {
        step = DKITS_TCAM_IGS_ACL0_KEY(tblid) ? 0 : (DKITS_TCAM_IGS_ACL1_KEY(tblid) ? 1 : (DKITS_TCAM_IGS_ACL2_KEY(tblid)       \
               ? 2 : (DKITS_TCAM_IGS_ACL3_KEY(tblid) ? 3 : (DKITS_TCAM_IGS_ACL4_KEY(tblid) ? 4 : (DKITS_TCAM_IGS_ACL5_KEY(tblid)  \
               ? 5 : (DKITS_TCAM_IGS_ACL6_KEY(tblid) ? 6 : 7))))));

        if (DKITS_TCAM_ACL_KEY_FORWARD320 == key_type)
        {
            *p_ad_tblid = DsIngAcl0ForwardBasicTcamAd_t + (DsIngAcl1ForwardBasicTcamAd_t - DsIngAcl0ForwardBasicTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_FORWARD640 == key_type)
        {
            *p_ad_tblid = DsIngAcl0ForwardExtTcamAd_t + (DsIngAcl1ForwardExtTcamAd_t - DsIngAcl0ForwardExtTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_SHORT80 == key_type)
        {
            *p_ad_tblid = DsIngAcl0IpfixEnableTcamAd_t + (DsIngAcl1IpfixEnableTcamAd_t - DsIngAcl0IpfixEnableTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_IPV6320 == key_type)
        {
            *p_ad_tblid = DsIngAcl0Ipv6BasicTcamAd_t + (DsIngAcl1Ipv6BasicTcamAd_t - DsIngAcl0Ipv6BasicTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_IPV6640 == key_type)
        {
            *p_ad_tblid = DsIngAcl0Ipv6ExtTcamAd_t + (DsIngAcl1Ipv6ExtTcamAd_t - DsIngAcl0Ipv6ExtTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_L3160 == key_type)
        {
            *p_ad_tblid = DsIngAcl0L3BasicTcamAd_t + (DsIngAcl1L3BasicTcamAd_t - DsIngAcl0L3BasicTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_L3320 == key_type)
        {
            *p_ad_tblid = DsIngAcl0L3ExtTcamAd_t + (DsIngAcl1L3ExtTcamAd_t - DsIngAcl0L3ExtTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_MACIPV6640 == key_type)
        {
            *p_ad_tblid = DsIngAcl0MacIpv6TcamAd_t + (DsIngAcl1MacIpv6TcamAd_t - DsIngAcl0MacIpv6TcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_MAC160 == key_type)
        {
            *p_ad_tblid = DsIngAcl0MacTcamAd_t + (DsIngAcl1MacTcamAd_t - DsIngAcl0MacTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_MACL3320 == key_type)
        {
            *p_ad_tblid = DsIngAcl0MacL3BasicTcamAd_t + (DsIngAcl1MacL3BasicTcamAd_t - DsIngAcl0MacL3BasicTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_MACL3640 == key_type)
        {
            *p_ad_tblid = DsIngAcl0MacL3ExtTcamAd_t + (DsIngAcl1MacL3ExtTcamAd_t - DsIngAcl0MacL3ExtTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_CID160 == key_type)
        {
            *p_ad_tblid = DsAclQosCidKey160Ing0_t + (DsAclQosCidKey160Ing1_t - DsAclQosCidKey160Ing0_t) * step;
        }
    }
    else if (DKITS_TCAM_EGS_ACL0_KEY(tblid) || DKITS_TCAM_EGS_ACL1_KEY(tblid) || DKITS_TCAM_EGS_ACL2_KEY(tblid))
    {
        step = DKITS_TCAM_EGS_ACL0_KEY(tblid) ? 0 : (DKITS_TCAM_EGS_ACL1_KEY(tblid) ? 1 : 2);

        if (DKITS_TCAM_ACL_KEY_SHORT80 == key_type)
        {
            *p_ad_tblid = DsEgrAcl0IpfixEnableTcamAd_t + (DsEgrAcl1IpfixEnableTcamAd_t - DsEgrAcl0IpfixEnableTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_IPV6320 == key_type)
        {
            *p_ad_tblid = DsEgrAcl0Ipv6BasicTcamAd_t + (DsEgrAcl1Ipv6BasicTcamAd_t - DsEgrAcl0Ipv6BasicTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_IPV6640 == key_type)
        {
            *p_ad_tblid = DsEgrAcl0Ipv6ExtTcamAd_t + (DsEgrAcl1Ipv6ExtTcamAd_t - DsEgrAcl0Ipv6ExtTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_L3160 == key_type)
        {
            *p_ad_tblid = DsEgrAcl0L3BasicTcamAd_t + (DsEgrAcl1L3BasicTcamAd_t - DsEgrAcl0L3BasicTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_L3320 == key_type)
        {
            *p_ad_tblid = DsEgrAcl0L3ExtTcamAd_t + (DsEgrAcl1L3ExtTcamAd_t - DsEgrAcl0L3ExtTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_MACIPV6640 == key_type)
        {
            *p_ad_tblid = DsEgrAcl0MacIpv6TcamAd_t + (DsEgrAcl1MacIpv6TcamAd_t - DsEgrAcl0MacIpv6TcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_MAC160 == key_type)
        {
            *p_ad_tblid = DsEgrAcl0MacTcamAd_t + (DsEgrAcl1MacTcamAd_t - DsEgrAcl0MacTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_MACL3320 == key_type)
        {
            *p_ad_tblid = DsEgrAcl0MacL3BasicTcamAd_t + (DsEgrAcl1MacL3BasicTcamAd_t - DsEgrAcl0MacL3BasicTcamAd_t) * step;
        }
        else if (DKITS_TCAM_ACL_KEY_MACL3640 == key_type)
        {
            *p_ad_tblid = DsEgrAcl0MacL3ExtTcamAd_t + (DsEgrAcl1MacL3ExtTcamAd_t - DsEgrAcl0MacL3ExtTcamAd_t) * step;
        }
    }
    else if (DKITS_TCAM_IPLKP0_KEY(tblid))
    {
        if (DsLpmTcamIpv4HalfKeyLookup1_t == tblid)
        {
            *p_ad_tblid = DsLpmTcamIpv4HalfKeyAdLookup1_t;
        }
        else if (DsLpmTcamIpv6DoubleKey0Lookup1_t == tblid)
        {
            *p_ad_tblid = DsLpmTcamIpv6DoubleKey0AdLookup1_t;
        }
        else if  (DsLpmTcamIpv4SaHalfKeyLookup1_t == tblid)
        {
            *p_ad_tblid = DsLpmTcamIpv4SaHalfKeyAdLookup1_t;
        }
        else if (DsLpmTcamIpv6SaDoubleKey0Lookup1_t == tblid)
        {
            *p_ad_tblid = DsLpmTcamIpv6SaDoubleKey0AdLookup1_t;
        }
        else if (DsLpmTcamIpv4DaPubHalfKeyLookup1_t == tblid)
        {
            *p_ad_tblid = DsLpmTcamIpv4DaPubHalfKeyAdLookup1_t;
        }
        else if (DsLpmTcamIpv6DaPubDoubleKey0Lookup1_t == tblid)
        {
            *p_ad_tblid = DsLpmTcamIpv6DaPubDoubleKey0AdLookup1_t;
        }
        else if (DsLpmTcamIpv4SaPubHalfKeyLookup1_t == tblid)
        {
            *p_ad_tblid = DsLpmTcamIpv4SaPubHalfKeyAdLookup1_t;
        }
        else if (DsLpmTcamIpv6SaPubDoubleKey0Lookup1_t == tblid)
        {
            *p_ad_tblid = DsLpmTcamIpv6SaPubDoubleKey0AdLookup1_t;
        }
    }
    else if (DKITS_TCAM_IPLKP1_KEY(tblid))
    {
        if (DsLpmTcamIpv4HalfKeyLookup2_t == tblid)
        {
            *p_ad_tblid = DsLpmTcamIpv4HalfKeyAdLookup2_t;
        }
        else if (DsLpmTcamIpv6DoubleKey0Lookup2_t == tblid)
        {
            *p_ad_tblid = DsLpmTcamIpv6DoubleKey0AdLookup2_t;
        }
        else if (DsLpmTcamIpv4SaHalfKeyLookup2_t == tblid)
        {
            *p_ad_tblid = DsLpmTcamIpv4SaHalfKeyAdLookup2_t;
        }
        else if (DsLpmTcamIpv6SaDoubleKey0Lookup2_t == tblid)
        {
            *p_ad_tblid = DsLpmTcamIpv6SaDoubleKey0AdLookup2_t;
        }
        else if (DsLpmTcamIpv4DaPubHalfKeyLookup2_t == tblid)
        {
            *p_ad_tblid = DsLpmTcamIpv4DaPubHalfKeyAdLookup2_t;
        }
        else if (DsLpmTcamIpv6DaPubDoubleKey0Lookup2_t == tblid)
        {
            *p_ad_tblid = DsLpmTcamIpv6DaPubDoubleKey0AdLookup2_t;
        }
        else if (DsLpmTcamIpv4SaPubHalfKeyLookup2_t == tblid)
        {
            *p_ad_tblid = DsLpmTcamIpv4SaPubHalfKeyAdLookup2_t;
        }
        else if (DsLpmTcamIpv6SaPubDoubleKey0Lookup2_t == tblid)
        {
            *p_ad_tblid = DsLpmTcamIpv6SaPubDoubleKey0AdLookup2_t;
        }
        else
        {
            *p_ad_tblid = MaxTblId_t;
        }
    }
    else if (DKITS_TCAM_NATPBR_KEY(tblid))
    {
        *p_ad_tblid = DsLpmTcamAd_t;
    }
    else if (DsCategoryIdPairTcamKey_t == tblid)
    {
        *p_ad_tblid = DsCategoryIdPairTcamAd_t;
    }
    else if (DsQueueMapTcamKey_t == tblid)
    {
        *p_ad_tblid = DsQueueMapTcamData_t;
    }
    else
    {
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_dump_get_hash_type(uint8 lchip, tbls_id_t tblid, ctc_dkits_ds_t* p_ds, uint32* p_hash_type)
{
    int32  ret = CLI_SUCCESS;
    uint32 primary_type = 0, ipv4_type = 0;
    ctc_dkits_hash_db_module_t module = CTC_DKITS_HASH_DB_MODULE_INVALID;

    CTC_DKITS_DUMP_CFG_ERROR_RETURN(_ctc_usw_dkits_dump_hash2module(lchip, tblid, &module));
    primary_type = DRV_GET_FIELD_V(lchip, usw_hash_key_field[module].tblid, usw_hash_key_field[module].type, p_ds);

    if (CTC_DKITS_HASH_DB_MODULE_FIB_HOST1 == module)
    {
        ipv4_type = GetDsFibHost1Ipv4NatDaPortHashKey(V, ipv4Type_f, p_ds);
        switch (primary_type)
        {
            case FIBHOST1PRIMARYHASHTYPE_IPV4:
                if (0 == ipv4_type)
                {
                    *p_hash_type = FIBHOST1HASHTYPE_IPV4NATDAPORT;
                }
                else if (1 == ipv4_type)
                {
                    *p_hash_type = FIBHOST1HASHTYPE_IPV4MCAST;
                }
                else if (2 == ipv4_type)
                {
                    *p_hash_type = FIBHOST1HASHTYPE_MACIPV4MCAST;
                }
                else if (3 == ipv4_type)
                {
                    *p_hash_type = FIBHOST1HASHTYPE_IPV4NATSAPORT;
                }
                break;

            case FIBHOST1PRIMARYHASHTYPE_IPV6NATDA:
                *p_hash_type = FIBHOST1HASHTYPE_IPV6NATDAPORT;
                break;

            case FIBHOST1PRIMARYHASHTYPE_IPV6NATSA:
                *p_hash_type = FIBHOST1HASHTYPE_IPV6NATSAPORT;
                break;

            case FIBHOST1PRIMARYHASHTYPE_OTHER:
                if (0 == ipv4_type)
                {
                    *p_hash_type = FIBHOST1HASHTYPE_FCOERPF;
                }
                else if (1 == ipv4_type)
                {
                    *p_hash_type = FIBHOST1HASHTYPE_TRILLMCASTVLAN;
                }
                else if (2 == ipv4_type)
                {
                    *p_hash_type = FIBHOST1HASHTYPE_IPV6MCAST;
                }
                else if (3 == ipv4_type)
                {
                    *p_hash_type = FIBHOST1HASHTYPE_MACIPV6MCAST;
                }
                break;

            default:
                return CLI_ERROR;
        }
    }
    else
    {
        *p_hash_type = primary_type;
    }

    return ret;
}

STATIC int32
_ctc_usw_dkits_dump_get_hash_info(uint8 lchip, tbls_id_t tblid, ctc_dkits_hash_tbl_info_t* p_hash_tbl_info)
{
    uint8 i = 0;
    ctc_dkits_hash_db_module_t module_type;

    sal_memset(p_hash_tbl_info, 0, sizeof(ctc_dkits_hash_tbl_info_t));
    CTC_DKITS_DUMP_CFG_ERROR_RETURN(_ctc_usw_dkits_dump_hash2module(lchip, tblid, &module_type));

    for (i = 0; i < CTC_DKITS_HASH_DB_KEY_NUM(module_type); i++)
    {
        if (tblid == CTC_DKITS_HASH_DB_KEY_TBLID(module_type, i))
        {
            p_hash_tbl_info->valid = 1;
            p_hash_tbl_info->adidx_fldid = CTC_DKITS_HASH_DB_ADIDX_FLDID(module_type, i);
            p_hash_tbl_info->ad_tblid = CTC_DKITS_HASH_DB_AD_TBLID(module_type, i);
            p_hash_tbl_info->hash_type = CTC_DKITS_HASH_DB_HASH_TYPE(module_type, i);
            return CLI_SUCCESS;
        }
    }

    return CLI_ERROR;
}

STATIC void
_ctc_usw_dkits_dump_get_tbl_order(uint8 lchip, tbls_id_t tblid, uint32* p_order)
{
    uint32 i = 0, j = 0, k =0, count = 0;
    uint32 hash_fun[] = {CTC_DKITS_FIB_HASH_FUN_TYPE_NUM, CTC_DKITS_FLOW_HASH_FUN_TYPE_NUM,
                         CTC_DKITS_SCL_HASH_FUN_TYPE_NUM, CTC_DKITS_XCOAM_HASH_FUN_TYPE_NUM,
                         CTC_DKITS_IPFIX_HASH_FUN_TYPE_NUM};
    uint32 tcam_fun[] = {CTC_DKITS_SCL_TCAM_FUN_TYPE_NUM, CTC_DKITS_ACL_TCAM_FUN_TYPE_NUM,
                         CTC_DKITS_NAT_PBR_TCAM_FUN_TYPE_NUM, CTC_DKITS_FWD_TCAM_FUN_TYPE_NUM, CTC_DKITS_QMGR_TCAM_FUN_TYPE_NUM};
    ctc_dkits_dump_tbl_type_t tbl_type;

    ctc_usw_dkits_dump_get_tbl_type(lchip, tblid, &tbl_type);

    if (CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == tbl_type)
    {
        for (i = 0; i < CTC_DKITS_TCAM_FUN_TYPE_NUM; i++)
        {
            for (j = 0; j < tcam_fun[i]; j++)
            {
                for (k = 0; MaxTblId_t != usw_tcam_key_inf[i][j][k].tblid; k++)
                {
                    count++;
                    if (usw_tcam_key_inf[i][j][k].tblid == tblid)
                    {
                        *p_order = count | DUMP_CFG_TCAM_ORDER_OFFSET;
                        goto CTC_DKITS_DUMP_GET_TBL_INFO_END;
                    }
                }
            }
        }
    }
    else if ((CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY == tbl_type)
            || (CTC_DKITS_DUMP_TBL_TYPE_HASH_NO_AD== tbl_type)
            || (CTC_DKITS_DUMP_TBL_TYPE_HASH_AD == tbl_type))
    {
        for (i = 0; i < CTC_DKITS_HASH_FUN_TYPE_NUM; i++)
        {
            for (j = 0; j < hash_fun[i]; j++)
            {
                for (k = 0; MaxTblId_t != usw_hash_tbl[i][j][k]; k++)
                {
                    count++;
                    if (tblid == usw_hash_tbl[i][j][k])
                    {
                        *p_order = count | DUMP_CFG_HASH_ORDER_OFFSET;
                        goto CTC_DKITS_DUMP_GET_TBL_INFO_END;
                    }
                }
            }
        }
    }
    else if (CTC_DKITS_DUMP_TBL_TYPE_HN == tbl_type)
    {
        for (i = 0; i < CTC_DKITS_NH_TBL_TYPE_NUM; i++)
        {
            for (j = 0; MaxTblId_t != usw_nh_tbl[i][j]; j++)
            {
                count++;
                if (tblid == usw_nh_tbl[i][j])
                {
                    if (NULL != p_order)
                    {
                        *p_order = count | DUMP_CFG_NH_ORDER_OFFSET;
                        goto CTC_DKITS_DUMP_GET_TBL_INFO_END;
                    }
                }
            }
        }
    }

CTC_DKITS_DUMP_GET_TBL_INFO_END:

    return;
}

STATIC int32
_ctc_usw_dkits_dump_parser_tcam_type(uint8 lchip, tbls_id_t tblid, ctc_dkits_dump_tbl_entry_t* p_tbl_entry, uint32* p_key_type)
{
    tbl_entry_t tcam_key;
    uint32      ad_type = 0;

    sal_memset(&tcam_key, 0, sizeof(tcam_key));

    tcam_key.data_entry = (uint32*)p_tbl_entry->data_entry;
    tcam_key.mask_entry = (uint32*)p_tbl_entry->mask_entry;

    CTC_DKITS_DUMP_CFG_ERROR_RETURN(ctc_usw_dkits_tcam_parser_key_type(lchip, tblid, &tcam_key, p_key_type, &ad_type));

    return CLI_SUCCESS;
}

void
ctc_usw_dkits_dump_get_tbl_type(uint8 lchip, tbls_id_t tblid, ctc_dkits_dump_tbl_type_t* p_tbl_type)
{
    ctc_dkits_hash_tbl_info_t hash_tbl_info;

    *p_tbl_type = CTC_DKITS_DUMP_TBL_TYPE_STATIC;

    if (((HsMacroPrtReq0_t <= tblid) && (HsMacroPrtReq2_t >= tblid)) ||
        ((CsMacroPrtReq0_t <= tblid) && (CsMacroPrtReq3_t >= tblid)))
    {
        *p_tbl_type = CTC_DKITS_DUMP_TBL_TYPE_SERDES;
    }
    else if (NULL == TABLE_EXT_INFO_PTR(lchip, tblid) ||  (TABLE_EXT_TYPE(lchip, tblid) == 1))
    {
        if (DsCategoryIdPairTcamAd_t == tblid)
        {
            *p_tbl_type = CTC_DKITS_DUMP_TBL_TYPE_TCAM_AD;
        }

#if 0
        n = ctc_usw_dkit_drv_get_bus_list_num(lchip);
        for (i = 0; i < n; i++)
        {
            if (tblid == DKIT_BUS_GET_FIFO_ID(i))
            {
                *p_tbl_type = CTC_DKITS_DUMP_TBL_TYPE_DBG;
                break;
            }
        }
 #endif
    }
    else if ((EXT_INFO_TYPE_TCAM == TABLE_EXT_INFO_TYPE(lchip, tblid))
       || (EXT_INFO_TYPE_LPM_TCAM_IP == TABLE_EXT_INFO_TYPE(lchip, tblid))
       || (EXT_INFO_TYPE_LPM_TCAM_NAT == TABLE_EXT_INFO_TYPE(lchip, tblid))
       || (EXT_INFO_TYPE_STATIC_TCAM_KEY == TABLE_EXT_INFO_TYPE(lchip, tblid)))
    {
        *p_tbl_type = CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY;
    }
    else if ((EXT_INFO_TYPE_TCAM_AD == TABLE_EXT_INFO_TYPE(lchip, tblid))
            || (EXT_INFO_TYPE_TCAM_LPM_AD == TABLE_EXT_INFO_TYPE(lchip, tblid))
            || (EXT_INFO_TYPE_TCAM_NAT_AD == TABLE_EXT_INFO_TYPE(lchip, tblid)))
    {
        *p_tbl_type = CTC_DKITS_DUMP_TBL_TYPE_TCAM_AD;
    }
    else
    {
        if ((DsFcoeDa_t == tblid)
           || (DsIpDa_t == tblid)
           || (DsMac_t == tblid)
           || (DsTrillDa_t == tblid)
           || (DsFcoeSa_t == tblid)
           || (DsIpSaNat_t == tblid)
           || (DsFlow_t == tblid)
           || (DsIpfixSessionRecord_t == tblid)
           || (DsUserId_t == tblid)
           || (DsSclFlow_t == tblid)
           || (DsTunnelId_t == tblid)
           || (DsMpls_t == tblid))
        {
            *p_tbl_type = CTC_DKITS_DUMP_TBL_TYPE_HASH_AD;
        }
        else if ((DsFwd_t == tblid)
                || (DsFwdDualHalf_t == tblid)
                || (DsFwdHalf_t == tblid)
                || (DsMetEntry3W_t == tblid)
                || (DsMetEntry6W_t == tblid)
                || (DsNextHop4W_t == tblid)
                || (DsNextHop8W_t == tblid)
                || (DsL23Edit3W_t == tblid)
                || (DsL23Edit12W_t == tblid)
                || (DsL23Edit6W_t == tblid)
                || (DsL2EditEth3W_t == tblid)
                || (DsL2EditEth6W_t == tblid)
                || (DsL2EditInnerSwap_t == tblid)
                || (DsL2EditFlex_t == tblid)
                || (DsL2EditLoopback_t == tblid)
                || (DsL2EditPbb4W_t == tblid)
                || (DsL2EditPbb8W_t == tblid)
                || (DsL2EditSwap_t == tblid)
                || (DsL2EditOf_t == tblid)
                || (DsL3EditFlex_t == tblid)
                || (DsL3EditLoopback_t == tblid)
                || (DsL3EditMpls3W_t == tblid)
                || (DsL3EditNat3W_t == tblid)
                || (DsL3EditNat6W_t == tblid)
                || (DsL3EditOf6W_t == tblid)
                || (DsL3EditOf12W_t == tblid)
                || (DsL3EditOf12WIpv6Only_t == tblid)
                || (DsL3EditOf12WArpData_t == tblid)
                || (DsL3EditOf12WIpv6L4_t == tblid)
                || (DsL3EditTrill_t == tblid)
                || (DsL3EditTunnelV4_t == tblid)
                || (DsL3EditTunnelV6_t == tblid)
                || (DsL3Edit12W1st_t == tblid)
                || (DsL3Edit12W2nd_t == tblid)
                || (DsL2Edit12WInner_t == tblid)
                || (DsL2Edit12WShare_t == tblid)
                || (DsL2Edit6WShare_t == tblid)
                || (DsL2EditDsLite_t == tblid)
                || (TempInnerEdit12W_t == tblid)
                || (TempOuterEdit12W_t == tblid))

        {
            *p_tbl_type = CTC_DKITS_DUMP_TBL_TYPE_HN;
        }
        else
        {
            sal_memset(&hash_tbl_info, 0, sizeof(ctc_dkits_hash_tbl_info_t));
            _ctc_usw_dkits_dump_get_hash_info(lchip, tblid, &hash_tbl_info);
            if (hash_tbl_info.valid)
            {
                *p_tbl_type = CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY;
            }
            else
            {
                *p_tbl_type = CTC_DKITS_DUMP_TBL_TYPE_INVALID;
                CTC_DKIT_PRINT("Error tbl type: %s\n", TABLE_NAME(lchip, tblid));
            }
        }
    }

    return;
}

STATIC int32
_ctc_usw_dkits_dump_search_cam(uint8 lchip, tbls_id_t tblid, uint8 expect_type, ctc_dkits_ds_t* p_key, uint32* p_hit, uint32* p_idx)
{
    int32  ret = CLI_SUCCESS;
    ctc_dkits_hash_db_module_t module = CTC_DKITS_HASH_DB_MODULE_INVALID;
    uint16 i = 0, j = 0, num = 0;
    uint32 cmd = 0, hash_type = 0, valid = 0;
    ctc_dkits_ds_t cam, key;

    _ctc_usw_dkits_dump_hash2module(lchip, tblid, &module);
    *p_hit = 0;

    if (MaxTblId_t == CTC_DKITS_HASH_DB_CAM_TBLID(module))
    {
        return ret;
    }

    if (TABLE_MAX_INDEX(lchip, tblid) == 0)
    {
        return ret;
    }
    num = TABLE_ENTRY_SIZE(lchip, CTC_DKITS_HASH_DB_CAM_TBLID(module)) / TABLE_ENTRY_SIZE(lchip, tblid);

    for (i = 0; i < TABLE_MAX_INDEX(lchip, CTC_DKITS_HASH_DB_CAM_TBLID(module)); i++)
    {
        sal_memset(&cam, 0, sizeof(ctc_dkits_ds_t));
        cmd = DRV_IOR(CTC_DKITS_HASH_DB_CAM_TBLID(module), DRV_ENTRY_FLAG);
        ret = DRV_IOCTL(0, i, cmd, &cam);
        if (ret < 0)
        {
            CTC_DKIT_PRINT_DEBUG("%s %d, error read tbl %s[%u]\n",
                                  __FUNCTION__, __LINE__,
                                  TABLE_NAME(lchip, CTC_DKITS_HASH_DB_CAM_TBLID(module)), i);
            return ret;
        }

        for (j = 0; j < num; j++)
        {
            sal_memcpy(&key, (uint8*)&cam + (j * TABLE_ENTRY_SIZE(lchip, tblid)), TABLE_ENTRY_SIZE(lchip, tblid));
            CTC_DKITS_DUMP_CFG_ERROR_RETURN(_ctc_usw_dkits_dump_get_hash_type(lchip, usw_hash_key_field[module].tblid, &key, &hash_type));
            ctc_usw_dkits_hash_db_entry_valid(lchip, module, hash_type, (uint32*)&key, &valid);

            if ((!valid) || (hash_type != expect_type))
            {
                continue;
            }
            sal_memcpy(&p_key[j], &key, TABLE_ENTRY_SIZE(lchip, tblid));
            p_idx[*p_hit] = i * 4 + j;
            (*p_hit)++;
        }
    }

    return ret;
}
STATIC int32
_ctc_usw_dkits_dump_write_invalid_tbl(uint8 lchip, uint32 tbl_id)
{
    if ((CFHeader_t == tbl_id)||(DynamicAdShareRam3W_t == tbl_id)||(DynamicAdShareRam6W_t == tbl_id)||(DynamicAdShareRam12W_t == tbl_id)
        ||(DynamicEditShareRam3W_t == tbl_id)||(DynamicEditShareRam6W_t == tbl_id)||(DynamicEditShareRam12W_t == tbl_id)||(DynamicKeyShareRam3W_t == tbl_id)
        ||(DynamicKeyShareRam6W_t == tbl_id)||(DynamicKeyShareRam12W_t == tbl_id)||(LpmTcamTcamMem_t == tbl_id)||(GeneralMiscChanIdCfg_t == tbl_id)
        ||(LpmTcamAdMem_t == tbl_id))
    {
        return 1;
    }
    return 0;
}
#define _____DECODE____
int32
ctc_usw_dkits_dump_decode_entry(uint8 lchip, void* p_para1, void* p_para2)
{
    ctc_dkits_hash_tbl_info_t hash_tbl_info;
    ctc_dkits_dump_tbl_type_t type = 0;
    tbls_id_t tb_id = MaxTblId_t;
    uint32 index = 0;
    ctc_dkits_dump_tbl_block_hdr_t* p_hdr = NULL;
    ctc_dkits_dump_key_tbl_info_t*  p_info = NULL;

    p_hdr = (ctc_dkits_dump_tbl_block_hdr_t*)p_para1;
    p_info = (ctc_dkits_dump_key_tbl_info_t*)p_para2;

    tb_id = CTC_DKITS_DUMP_BLOCK_HDR_TBLID(p_hdr);
    index = CTC_DKITS_DUMP_BLOCK_HDR_TBLIDX(p_hdr);

    ctc_usw_dkits_dump_get_tbl_type(lchip, tb_id, &type);

    if (CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY == type)
    {
        CTC_DKITS_DUMP_CFG_ERROR_RETURN(_ctc_usw_dkits_dump_get_hash_info(lchip, tb_id, &hash_tbl_info));
    }

    if ((CTC_DKITS_DUMP_TBL_TYPE_HASH_AD == type) || (CTC_DKITS_DUMP_TBL_TYPE_TCAM_AD == type))
    {
        p_info->ad_tblid = tb_id;
        p_info->ad_idx = index;
        p_info->tbl_type = type;
    }
    else
    {
        _ctc_usw_dkits_dump_get_tbl_order(lchip, tb_id, &p_info->order);
        p_info->tblid = tb_id;
        p_info->idx = index;
        p_info->tbl_type = type;
    }

    return 0;
}

#define _____DUMP_____
STATIC int32
_ctc_usw_dkits_dump_hash_tbl(uint8 lchip, uint8 fun_module, uint8 fun, sal_file_t p_f)
{
    int32  ret = CLI_SUCCESS;
    uint32 i = 0, j = 0, cmd = 0, ad_idx = 0, hash_type = 0, valid = 0, step = 0, hit = 0;
    uint32* idx = NULL;
    ctc_dkits_hash_tbl_info_t hash_tbl_info;
    ctc_dkits_hash_db_module_t module;
    ctc_dkits_ds_t* cam_key = NULL;
    ctc_dkits_ds_t* ad = NULL;
    ctc_dkits_ds_t* hash_key = NULL;

    idx = (uint32*)sal_malloc(100 * sizeof(uint32));
    if (NULL == idx)
    {
        return CLI_ERROR;
    }
    cam_key = (ctc_dkits_ds_t*)sal_malloc(32 * sizeof(ctc_dkits_ds_t));
    if (NULL == cam_key)
    {
        ret = CLI_ERROR;
        goto DKITS_DUMP_HASH_TBL_ERROR;
    }
    ad = (ctc_dkits_ds_t*)sal_malloc(sizeof(ctc_dkits_ds_t));
    if (NULL == ad)
    {
        ret = CLI_ERROR;
        goto DKITS_DUMP_HASH_TBL_ERROR;
    }
    hash_key = (ctc_dkits_ds_t*)sal_malloc(sizeof(ctc_dkits_ds_t));
    if (NULL == hash_key)
    {
        ret = CLI_ERROR;
        goto DKITS_DUMP_HASH_TBL_ERROR;
    }
    sal_memset(idx, 0, 100 * sizeof(uint32));
    sal_memset(cam_key, 0, 32 * sizeof(ctc_dkits_ds_t));
    sal_memset(ad, 0, sizeof(ctc_dkits_ds_t));
    sal_memset(hash_key, 0, sizeof(ctc_dkits_ds_t));

    for (i = 0; MaxTblId_t != usw_hash_tbl[fun_module][fun][i]; i++)
    {
        ret = _ctc_usw_dkits_dump_get_hash_info(lchip, usw_hash_tbl[fun_module][fun][i], &hash_tbl_info);
        if (CLI_SUCCESS != ret)
        {
            CTC_DKIT_PRINT_DEBUG("%s %d, Error hash tbl %s\n",
                                  __FUNCTION__, __LINE__, TABLE_NAME(lchip, usw_hash_tbl[fun_module][fun][i]));
            goto DKITS_DUMP_HASH_TBL_ERROR;
        }

        step = (TABLE_ENTRY_SIZE(lchip, usw_hash_tbl[fun_module][fun][i]) / DRV_BYTES_PER_ENTRY);
        _ctc_usw_dkits_dump_hash2module(lchip, usw_hash_tbl[fun_module][fun][i], &module);

        for (j = 0; j < TABLE_MAX_INDEX(lchip, usw_hash_tbl[fun_module][fun][i]); j += step)
        {
            cmd = DRV_IOR(usw_hash_tbl[fun_module][fun][i], DRV_ENTRY_FLAG);
            ret = DRV_IOCTL(lchip, j, cmd, hash_key);
            if (ret < 0)
            {
                ret = CLI_ERROR;
                CTC_DKIT_PRINT_DEBUG("Error read tbl %s[%u]\n", TABLE_NAME(lchip, usw_hash_tbl[fun_module][fun][i]), j);
                goto DKITS_DUMP_HASH_TBL_ERROR;
            }

            if (DsFibHost0Ipv4HashKey_t == usw_hash_tbl[fun_module][fun][i])
            {
                uint32 ipv4_type = 0;
                /*
                 * 2'b00: ipv4 uc
                 * 2'b01: ipv4 mc
                 * 2'b10: ipda lookup for l2mc
                 * 2'b11: natsa
                 */
                ipv4_type = GetDsFibHost0Ipv4HashKey(V, ipv4Type_f, hash_key);
                if ((CTC_DKITS_FIB_HASH_FUN_TYPE_IPUC_IPV4 == fun) && ((1 == ipv4_type) || (2 == ipv4_type)))
                {
                    continue;
                }
                if ((CTC_DKITS_FIB_HASH_FUN_TYPE_IPMC_IPV4 == fun) && (0 == ipv4_type))
                {
                    continue;
                }
            }

            ret = _ctc_usw_dkits_dump_get_hash_type(lchip, usw_hash_key_field[module].tblid, hash_key, &hash_type);
            if (CLI_SUCCESS != ret)
            {
                CTC_DKIT_PRINT_DEBUG(" %s %d, Get hash type error!\n", __FUNCTION__, __LINE__);
                goto DKITS_DUMP_HASH_TBL_ERROR;
            }

            ret = ctc_usw_dkits_hash_db_entry_valid(lchip, module, hash_type, (uint32*)hash_key, &valid);
            if (CLI_SUCCESS != ret)
            {
                CTC_DKIT_PRINT_DEBUG(" %s %d, Get hash valid error!\n", __FUNCTION__, __LINE__);
                goto DKITS_DUMP_HASH_TBL_ERROR;
            }

            if ((!valid) || (hash_type != hash_tbl_info.hash_type))
            {
                continue;
            }

            ctc_usw_dkits_dump_tbl2data(lchip, usw_hash_tbl[fun_module][fun][i], j + 32, hash_key->entry, NULL, p_f);

            if (CTC_DKITS_TBL_INVALID_FLDID != hash_tbl_info.adidx_fldid)
            {
                ad_idx = DRV_GET_FIELD_V(lchip, usw_hash_tbl[fun_module][fun][i], hash_tbl_info.adidx_fldid, hash_key);
            }
            else
            {
                ad_idx = j;
            }

            if (MaxTblId_t != hash_tbl_info.ad_tblid)
            {
                cmd = DRV_IOR(hash_tbl_info.ad_tblid, DRV_ENTRY_FLAG);
                ret = DRV_IOCTL(lchip, ad_idx, cmd, ad);
                if (ret < 0)
                {
                    CTC_DKIT_PRINT_DEBUG("Error read tbl %s[%u]\n", TABLE_NAME(lchip, hash_tbl_info.ad_tblid), ad_idx);
                    ret = CLI_ERROR;
                    goto DKITS_DUMP_HASH_TBL_ERROR;
                }
                ctc_usw_dkits_dump_tbl2data(lchip, hash_tbl_info.ad_tblid, ad_idx, ad->entry, NULL, p_f);
            }
        }

        sal_memset(idx, 0, 100 * sizeof(uint32));
        _ctc_usw_dkits_dump_search_cam(lchip, usw_hash_tbl[fun_module][fun][i], hash_tbl_info.hash_type, cam_key, &hit, idx);

        for (j = 0; j < hit; j++)
        {
            ctc_usw_dkits_dump_tbl2data(lchip, usw_hash_tbl[fun_module][fun][i], idx[j], cam_key[j].entry, NULL, p_f);

            if (CTC_DKITS_TBL_INVALID_FLDID != hash_tbl_info.adidx_fldid)
            {
                ad_idx = DRV_GET_FIELD_V(lchip, usw_hash_tbl[fun_module][fun][i], hash_tbl_info.adidx_fldid, cam_key[j].entry);
            }
            else
            {
                ad_idx = j;
            }
            if (MaxTblId_t != hash_tbl_info.ad_tblid)
            {
                cmd = DRV_IOR(hash_tbl_info.ad_tblid, DRV_ENTRY_FLAG);
                ret = DRV_IOCTL(lchip, ad_idx, cmd, ad);
                if (ret < 0)
                {
                    CTC_DKIT_PRINT_DEBUG("%s %d, error read tbl %s[%u]\n",
                                         __FUNCTION__, __LINE__, TABLE_NAME(lchip, hash_tbl_info.ad_tblid), ad_idx);
                    ret = CLI_ERROR;
                    goto DKITS_DUMP_HASH_TBL_ERROR;
                }
                ctc_usw_dkits_dump_tbl2data(lchip, hash_tbl_info.ad_tblid, ad_idx, ad->entry, NULL, p_f);
            }
        }
    }

DKITS_DUMP_HASH_TBL_ERROR:
    if (NULL != idx)
    {
        sal_free(idx);
    }
    if (NULL != cam_key)
    {
       sal_free(cam_key);
    }
    if (NULL != ad)
    {
       sal_free(ad);
    }
    if (NULL != hash_key)
    {
       sal_free(hash_key);
    }

    return ret;
}

STATIC int32
_ctc_usw_dkits_dump_write_tbl(uint8 lchip, void* p_para4, tbls_id_t tblid, void* p_para1, void* p_para2, void* p_para3)
{
    uint32 i = 0, cmd = 0;
    sal_file_t p_f = (sal_file_t)p_para1;
    uint32 data_entry[MAX_ENTRY_WORD] = {0};
    uint32* p_total_count = (uint32*)p_para2;
    uint32* p_curr_count = (uint32*)p_para3;

    lchip = *((uint8*)p_para4);

    cmd = DRV_IOR(tblid, DRV_ENTRY_FLAG);

    if (_ctc_usw_dkits_dump_write_invalid_tbl(lchip, tblid))
    {
        return CLI_SUCCESS;
    }

    for (i = 0; i < TABLE_MAX_INDEX(lchip, tblid); i++)
    {
        if (0 == ((*p_curr_count) % (*p_total_count / 30)))
        {
            sal_udelay(10);
        }
        CTC_DKITS_DUMP_CFG_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, &data_entry));
        *p_curr_count += 1;
        CTC_DKITS_DUMP_CFG_ERROR_RETURN(ctc_usw_dkits_dump_tbl2data(lchip, tblid, i, data_entry, NULL, p_f));
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_dump_count_tbl(uint8 lchip, void* p_para4,tbls_id_t tblid, void* p_para1, void* p_para2, void* p_para3)
{
    *((uint32*)p_para1) += TABLE_MAX_INDEX(lchip, tblid);

    return CLI_SUCCESS;
}


STATIC int32
_ctc_usw_dkits_dump_travel_static_cfg(uint8 lchip, void* p_para1, void* p_para2, void* p_para3, tbls_id_t* p_tblid, travel_callback_fun_t p_fun)
{
    int32   ret = 0;
    uint32 count = 0, i = 0;

    for (i = 0; MaxTblId_t != p_tblid[i]; i++)
    {
        ret = (*p_fun)(lchip, p_para1, p_tblid[i], p_para2, p_para3, &count);
    }

    return ret;
}


STATIC int32
_ctc_usw_dkits_dump_travel_static_all(uint8 lchip, void* p_para1, void* p_para2, void* p_para3, travel_callback_fun_t p_fun)
{
    int32 ret = 0;
    uint32 i = 0;
    uint32 count = 0;

    for (i = 0; i < MaxTblId_t; i++)
    {
        if (ctc_usw_dkit_memory_is_invalid_table(lchip, i))
        {
            continue;
        }

        if (NULL == TABLE_EXT_INFO_PTR(lchip, i))
        {
	        if (_ctc_usw_dkits_dump_invalid_static_tbl(lchip, i))
	        {
	            continue;
	        }
            ret = (*p_fun)(lchip, p_para1, i, p_para2, p_para3, &count);
            if (CLI_ERROR == ret)
            {
                return CLI_ERROR;
            }
        }
    }

     /*ret = (*p_fun)(EfdElephantFlowBitmapCtl_t, p_para1, p_para2, &count);*/

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_dump_indirect(uint8 lchip, sal_file_t p_f, ctc_dkits_dump_func_flag_t flag)
{
    int32 ret = 0;
    if(g_dkit_chip_api[lchip]->dkits_dump_indirect)
    {
        ret = g_dkit_chip_api[lchip]->dkits_dump_indirect(lchip, p_f, flag);
    }

    return ret;
}

STATIC int32
_ctc_usw_dkits_dump_tcam_tbl(uint8 lchip, ctc_dkits_tcam_tbl_module_t module, uint8 fun, sal_file_t p_f)
{
    ds_t   data = {0}, mask = {0};
    uint32 t = 0, i = 0, ad_offset = 0, is_empty = 0, cmd = 0, max_idx = 0, mode = 0;
    tbl_entry_t tcam_key;
    uint32 key_type = 0, ad_type = 0;
    tbls_id_t ad_tblid;

    tcam_key.data_entry = (uint32 *)&data;
    tcam_key.mask_entry = (uint32 *)&mask;

    cmd = DRV_IOR(LpmTcamCtl_t, LpmTcamCtl_privatePublicLookupMode_f);
    CTC_DKITS_DUMP_CFG_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mode));

    for (t = 0; MaxTblId_t != usw_tcam_key_inf[module][fun][t].tblid; t++)
    {
        if (0 == TABLE_MAX_INDEX(lchip, usw_tcam_key_inf[module][fun][t].tblid))
        {
            continue;
        }

        if (((DKITS_TCAM_LPM_USAGE_PUBLIC_IPDA_IPSA_FULL == mode) || (DKITS_TCAM_LPM_USAGE_PRIVATE_IPDA_IPSA_FULL == mode))
           && ((DsLpmTcamIpv4SaHalfKeyLookup1_t      == usw_tcam_key_inf[module][fun][t].tblid)
           || (DsLpmTcamIpv6SaDoubleKey0Lookup1_t    == usw_tcam_key_inf[module][fun][t].tblid)
           || (DsLpmTcamIpv4SaPubHalfKeyLookup1_t    == usw_tcam_key_inf[module][fun][t].tblid)
           || (DsLpmTcamIpv6SaPubDoubleKey0Lookup1_t == usw_tcam_key_inf[module][fun][t].tblid)
           || (DsLpmTcamIpv6SaSingleKey_t            == usw_tcam_key_inf[module][fun][t].tblid)
           || (DsLpmTcamIpv6SaPubSingleKey_t         == usw_tcam_key_inf[module][fun][t].tblid)
           || (DsLpmTcamIpv4SaHalfKeyLookup2_t       == usw_tcam_key_inf[module][fun][t].tblid)
           || (DsLpmTcamIpv6SaDoubleKey0Lookup2_t    == usw_tcam_key_inf[module][fun][t].tblid)
           || (DsLpmTcamIpv4SaPubHalfKeyLookup2_t    == usw_tcam_key_inf[module][fun][t].tblid)
           || (DsLpmTcamIpv6SaPubDoubleKey0Lookup2_t == usw_tcam_key_inf[module][fun][t].tblid)))
        {
            continue;
        }

        CTC_DKITS_DUMP_CFG_ERROR_RETURN(drv_usw_ftm_get_entry_num(lchip, usw_tcam_key_inf[module][fun][t].tblid, &max_idx));

        for (i = 0; i < max_idx; i++)
        {
            sal_memset(&data, 0, sizeof(data));
            sal_memset(&mask, 0, sizeof(mask));

            CTC_DKITS_DUMP_CFG_ERROR_RETURN(ctc_usw_dkits_tcam_read_tcam_key(lchip, usw_tcam_key_inf[module][fun][t].tblid, i, &is_empty, &tcam_key));

            if (!is_empty)
            {
                CTC_DKITS_DUMP_CFG_ERROR_RETURN(ctc_usw_dkits_tcam_parser_key_type(lchip, usw_tcam_key_inf[module][fun][t].tblid, &tcam_key, &key_type, &ad_type));
                if (key_type == usw_tcam_key_inf[module][fun][t].key_type)
                {
                    ctc_usw_dkits_dump_tbl2data(lchip, usw_tcam_key_inf[module][fun][t].tblid,
                                            i, tcam_key.data_entry, tcam_key.mask_entry, p_f);

                    CTC_DKITS_DUMP_CFG_ERROR_RETURN(_ctc_usw_dkits_dump_tcam_key2ad(lchip, usw_tcam_key_inf[module][fun][t].tblid, key_type, ad_type, &ad_tblid));
                    if (MaxTblId_t != ad_tblid)
                    {
                        cmd = DRV_IOR(ad_tblid, DRV_ENTRY_FLAG);

                        if (DKITS_TCAM_IGS_SCL0_KEY(usw_tcam_key_inf[module][fun][t].tblid)
                           || DKITS_TCAM_IGS_SCL1_KEY(usw_tcam_key_inf[module][fun][t].tblid))
                        {
                            if (1 == (TABLE_ENTRY_SIZE(lchip, usw_tcam_key_inf[module][fun][t].tblid) / DRV_BYTES_PER_ENTRY))
                            {
                                ad_offset = ad_offset / 2;
                            }
                            else if (4 == (TABLE_ENTRY_SIZE(lchip, usw_tcam_key_inf[module][fun][t].tblid) / DRV_BYTES_PER_ENTRY))
                            {
                                ad_offset = ad_offset * 2;
                            }
                            else if (8 == (TABLE_ENTRY_SIZE(lchip, usw_tcam_key_inf[module][fun][t].tblid) / DRV_BYTES_PER_ENTRY))
                            {
                                ad_offset = ad_offset * 4;
                            }
                        }
                        CTC_DKITS_DUMP_CFG_ERROR_RETURN(DRV_IOCTL(lchip, ad_offset, cmd, data));
                        ctc_usw_dkits_dump_tbl2data(lchip, ad_tblid, ad_offset, data, NULL, p_f);
                    }
                }
            }
        }
    }

    return CLI_SUCCESS;
}

#define _____TCAM_USAGE_____

STATIC int32
_ctc_usw_dkits_dump_tcam_resc(uint8 lchip, dkits_tcam_block_type_t type)
{
    uint32  i = 0, j = 0, is_empty = 0, sum = 0, key_type = 0, sid = 0xFFFFFFFF, entry_num = 0;
    uint32  alloc[DKITS_TCAM_PER_BLOCK_MAX_KEY_TYPE] = {0};
    uint32  used[DKITS_TCAM_PER_BLOCK_MAX_KEY_TYPE] = {0};
    uint32  total_alloc = 0, total_used = 0, key_size = 0;
    tbl_entry_t tcam_key;
    ds_t    data;
    ds_t    mask;
    ctc_dkits_dump_tbl_entry_t tbl_entry;

    sal_memset(&data, 0, sizeof(data));
    sal_memset(&mask, 0, sizeof(mask));

    tcam_key.data_entry = (uint32 *)&data;
    tcam_key.mask_entry = (uint32 *)&mask;

    for (i = 0; MaxTblId_t != ctc_dkits_tcam_tbl[type][i].tblid; i++)
    {
        sum = 0;
        CTC_DKITS_DUMP_CFG_ERROR_RETURN(drv_usw_ftm_get_entry_num(lchip, ctc_dkits_tcam_tbl[type][i].tblid, &entry_num));

        for (j = 0; j < entry_num; j++)
        {
            CTC_DKITS_DUMP_CFG_ERROR_RETURN(ctc_usw_dkits_tcam_read_tcam_key(lchip, ctc_dkits_tcam_tbl[type][i].tblid, j, &is_empty, &tcam_key));

            if (!is_empty)
            {
                sal_memset(&tbl_entry, 0, sizeof(ctc_dkits_dump_tbl_entry_t));
                tbl_entry.data_entry = tcam_key.data_entry;
                tbl_entry.mask_entry = tcam_key.mask_entry;
                CTC_DKITS_DUMP_CFG_ERROR_RETURN(_ctc_usw_dkits_dump_parser_tcam_type(lchip, ctc_dkits_tcam_tbl[type][i].tblid, &tbl_entry, &key_type));
                if (DRV_IS_TSINGMA(lchip) &&
                    ((DsScl0MacIpv6Key640_t == ctc_dkits_tcam_tbl[type][i].tblid) || (DsScl1MacIpv6Key640_t == ctc_dkits_tcam_tbl[type][i].tblid)
                || (DsScl2MacIpv6Key640_t == ctc_dkits_tcam_tbl[type][i].tblid) || (DsScl3MacIpv6Key640_t == ctc_dkits_tcam_tbl[type][i].tblid)))
                {
                    ctc_dkits_tcam_tbl[type][i].key_type = DKITS_TCAM_SCL_KEY_USERID_12W;
                }
                if (ctc_dkits_tcam_tbl[type][i].key_type == key_type)
                {
                    sum++;
                }
            }
        }

        key_size = 0;
        if (TABLE_EXT_INFO_PTR(lchip, ctc_dkits_tcam_tbl[type][i].tblid))
        {
            key_size = TCAM_KEY_SIZE(lchip, ctc_dkits_tcam_tbl[type][i].tblid);
        }

        if (DRV_TABLE_TYPE_STATIC_TCAM_KEY == drv_usw_get_table_type(lchip, ctc_dkits_tcam_tbl[type][i].tblid))
        {
            alloc[i]    = entry_num;
            used[i]     = sum;
        }
        else if ((drv_usw_get_table_type(lchip, ctc_dkits_tcam_tbl[type][i].tblid) == DRV_TABLE_TYPE_LPM_TCAM_IP)
                || (drv_usw_get_table_type(lchip, ctc_dkits_tcam_tbl[type][i].tblid) == DRV_TABLE_TYPE_LPM_TCAM_NAT))
        {
            alloc[i]    = entry_num * key_size / DRV_LPM_KEY_BYTES_PER_ENTRY;
            used[i]     = sum * key_size / DRV_LPM_KEY_BYTES_PER_ENTRY;
        }
        else
        {
            alloc[i]    = entry_num * key_size / DRV_BYTES_PER_ENTRY;
            used[i]     = sum * key_size / DRV_BYTES_PER_ENTRY;
        }

        if (sid != ctc_dkits_tcam_tbl[type][i].sid)
        {
            sid = ctc_dkits_tcam_tbl[type][i].sid;
            total_alloc += alloc[i];
        }
        total_used  += used[i];
    }

    sid = 0xFFFFFFFF;
    for (i = 0; MaxTblId_t != ctc_dkits_tcam_tbl[type][i].tblid; i++)
    {
        if (0 == i)
        {
            CTC_DKIT_PRINT(" %-23s%-38s%-13u%u\n",
                           tcam_block_desc[type], "AllKey",
                           total_alloc, total_used);
        }
        if (CTC_DKITS_DUMP_CFG_SID_NO_ALLOC != ctc_dkits_tcam_tbl[type][i].sid)
        {
            if (sid != ctc_dkits_tcam_tbl[type][i].sid)
            {
                sid = ctc_dkits_tcam_tbl[type][i].sid;
                CTC_DKIT_PRINT(" %-23s%-38s%-13u%u\n",
                               " ", TABLE_NAME(lchip, ctc_dkits_tcam_tbl[type][i].tblid),
                               alloc[i], used[i]);
            }
            else
            {
                CTC_DKIT_PRINT(" %-23s%-38s%-13s%u\n",
                               " ", TABLE_NAME(lchip, ctc_dkits_tcam_tbl[type][i].tblid),
                               "", used[i]);
            }
        }
        else
        {
#if CTC_DKITS_DUMP_CFG_SHOW_NO_ALLOC
            sid = ctc_dkits_tcam_tbl[type][i].sid;
            CTC_DKIT_PRINT(" %-23s%-38s%-13s%s\n",
                           " ", TABLE_NAME(lchip, ctc_dkits_tcam_tbl[type][i].tblid),
                           "-", "-");
#endif
        }
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_dump_tcam_resc_info(uint8 lchip)
{
    uint32 i = 0, j = 0, m = 0, n = 0, idx_num = 0;
    uint32 offset = 0, blk = 0, key_offset = 0;
    ctc_dkits_tcam_tbl_t tcam_tbl_tmp;

    for (i = 0; i < DKITS_TCAM_BLOCK_TYPE_NUM; i++)
    {
        for (j = 0; (MaxTblId_t != ctc_dkits_tcam_tbl[i][j].tblid); j++)
        {
            CTC_DKITS_DUMP_CFG_ERROR_RETURN(drv_usw_ftm_get_entry_num(lchip, ctc_dkits_tcam_tbl[i][j].tblid, &idx_num));
            if (0 == idx_num)
            {
                /* not allocate */
                ctc_dkits_tcam_tbl[i][j].sid = CTC_DKITS_DUMP_CFG_SID_NO_ALLOC;
            }
            else
            {
                blk = 0xFFFFFFFF;
                CTC_DKITS_DUMP_CFG_ERROR_RETURN(ctc_usw_dkits_tcam_entry_offset(lchip, ctc_dkits_tcam_tbl[i][j].tblid, key_offset, &offset, &blk));
                if (0xFFFFFFFF != blk)
                {
                    ctc_dkits_tcam_tbl[i][j].sid = CTC_DKITS_DUMP_TCAM_SHARE_INF(offset, blk);
                }
                else
                {
                    ctc_dkits_tcam_tbl[i][j].sid = CTC_DKITS_DUMP_CFG_SID_STATIC;
                }
            }
        }

        for (m = 0; m < j - 1; m++)  /*sort sid for dump different size of share key*/
        {
            for (n = m + 1; n < j; n++)
            {
                if (ctc_dkits_tcam_tbl[i][m].sid > ctc_dkits_tcam_tbl[i][n].sid)
                {
                    tcam_tbl_tmp.tblid      = ctc_dkits_tcam_tbl[i][m].tblid;
                    tcam_tbl_tmp.key_type   = ctc_dkits_tcam_tbl[i][m].key_type;
                    tcam_tbl_tmp.sid        = ctc_dkits_tcam_tbl[i][m].sid;

                    ctc_dkits_tcam_tbl[i][m].tblid    = ctc_dkits_tcam_tbl[i][n].tblid;
                    ctc_dkits_tcam_tbl[i][m].key_type = ctc_dkits_tcam_tbl[i][n].key_type;
                    ctc_dkits_tcam_tbl[i][m].sid      = ctc_dkits_tcam_tbl[i][n].sid;

                    ctc_dkits_tcam_tbl[i][n].tblid    = tcam_tbl_tmp.tblid;
                    ctc_dkits_tcam_tbl[i][n].key_type = tcam_tbl_tmp.key_type;
                    ctc_dkits_tcam_tbl[i][n].sid      = tcam_tbl_tmp.sid;
                }
            }
        }
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_dump_tcam_mem_usage(uint8 lchip)
{
    uint8 tcam_block_type = 0;
    CTC_DKITS_DUMP_CFG_ERROR_RETURN(_ctc_usw_dkits_dump_tcam_resc_info(lchip));

    CTC_DKIT_PRINT("\n");
    CTC_DKIT_PRINT(" Note:\"-\" means the key not be allocated in currennt memory profile.\n");
    CTC_DKIT_PRINT(" -----------------------------------------------------------------------------------\n");
    CTC_DKIT_PRINT(" %-23s%-38s%-13s%s\n", "TcamModule", "KeyName", "Total(80b)", "Used(80b)");
    CTC_DKIT_PRINT(" -----------------------------------------------------------------------------------\n");

    for (tcam_block_type = 0; tcam_block_type < DKITS_TCAM_BLOCK_TYPE_NUM; tcam_block_type++)
    {
        if (DRV_IS_DUET2(lchip) && ((tcam_block_type == DKITS_TCAM_BLOCK_TYPE_IGS_SCL2) || (tcam_block_type == DKITS_TCAM_BLOCK_TYPE_IGS_SCL3)))
        {
            continue;
        }
        CTC_DKITS_DUMP_CFG_ERROR_RETURN(_ctc_usw_dkits_dump_tcam_resc(lchip, tcam_block_type));
        sal_udelay(10);
    }
    CTC_DKIT_PRINT(" -----------------------------------------------------------------------------------\n\n");

    return CLI_SUCCESS;
}

#define _____HASH_USAGE_____

STATIC void
_ctc_usw_dkits_dump_hash_class_key(uint8 lchip, ctc_dkits_hash_db_module_t module_type, uint32 key_type, uint32 sub_type, uint32* p_class_type)
{
    if (CTC_DKITS_HASH_DB_MODULE_EGRESS_XC == module_type)
    {
        switch (key_type)
        {
            case EGRESSXCOAMHASHTYPE_DOUBLEVLANPORT:
            case EGRESSXCOAMHASHTYPE_SVLANPORT:
            case EGRESSXCOAMHASHTYPE_CVLANPORT:
            case EGRESSXCOAMHASHTYPE_SVLANCOSPORT:
            case EGRESSXCOAMHASHTYPE_CVLANCOSPORT:
            case EGRESSXCOAMHASHTYPE_PORTVLANCROSS:
            case EGRESSXCOAMHASHTYPE_PORTCROSS:
            case EGRESSXCOAMHASHTYPE_PORT:
            case EGRESSXCOAMHASHTYPE_SVLANPORTMAC:
                *p_class_type = CTC_DKITS_XCOAM_HASH_FUN_TYPE_SCL;
                break;
            case EGRESSXCOAMHASHTYPE_TUNNELPBB:
                *p_class_type = CTC_DKITS_XCOAM_HASH_FUN_TYPE_PBB_TUNNEL;
                break;
            case EGRESSXCOAMHASHTYPE_ETH:
            case EGRESSXCOAMHASHTYPE_BFD:
            case EGRESSXCOAMHASHTYPE_MPLSLABEL:
            case EGRESSXCOAMHASHTYPE_MPLSSECTION:
            case EGRESSXCOAMHASHTYPE_RMEP:
                *p_class_type = CTC_DKITS_XCOAM_HASH_FUN_TYPE_OAM;
                break;
            default:
                break;
        }
    }
    else if (CTC_DKITS_HASH_DB_MODULE_FIB_HOST0 == module_type)
    {
        switch (key_type)
        {
            case FIBHOST0PRIMARYHASHTYPE_FCOE:
                *p_class_type = CTC_DKITS_FIB0_HASH_CLASS_TYPE_FCOE;
                break;
            case FIBHOST0PRIMARYHASHTYPE_IPV4:
                 /*
                  * 2'b00: ipv4 uc
                  * 2'b01: ipv4 mc
                  * 2'b10: ipda lookup for l2mc
                  * 2'b10: ip natsa
                  */
                if (0 == sub_type)
                {
                    *p_class_type = CTC_DKITS_FIB0_HASH_CLASS_TYPE_IPV4_UCAST;
                }
                else if ((1 == sub_type) || (2 == sub_type))
                {
                    *p_class_type = CTC_DKITS_FIB0_HASH_CLASS_TYPE_IPV4_MCAST;
                }
                else if (3 == sub_type)
                {
                    *p_class_type = CTC_DKITS_FIB0_HASH_CLASS_TYPE_IPV4_NATSA;
                }
                break;
            case FIBHOST0PRIMARYHASHTYPE_IPV6MCAST:
            case FIBHOST0PRIMARYHASHTYPE_MACIPV6MCAST:
                *p_class_type = CTC_DKITS_FIB0_HASH_CLASS_TYPE_IPV6_MCAST;
                break;
            case FIBHOST0PRIMARYHASHTYPE_IPV6UCAST:
                *p_class_type = CTC_DKITS_FIB0_HASH_CLASS_TYPE_IPV6_UCAST;
                break;
            case FIBHOST0PRIMARYHASHTYPE_MAC:
                *p_class_type = CTC_DKITS_FIB0_HASH_CLASS_TYPE_FDB;
                break;
            case FIBHOST0PRIMARYHASHTYPE_TRILL:
                *p_class_type = CTC_DKITS_FIB0_HASH_CLASS_TYPE_TRILL;
                break;
            default:
                break;
        }
    }
    else if (CTC_DKITS_HASH_DB_MODULE_FIB_HOST1 == module_type)
    {
        switch (key_type)
        {
            case FIBHOST1HASHTYPE_IPV4MCAST:
                *p_class_type = CTC_DKITS_FIB1_HASH_CLASS_TYPE_IPV4_MCAST;
                break;
            case FIBHOST1HASHTYPE_IPV6MCAST:
                *p_class_type = CTC_DKITS_FIB1_HASH_CLASS_TYPE_IPV6_MCAST;
                break;
            case FIBHOST1HASHTYPE_MACIPV4MCAST:
            case FIBHOST1HASHTYPE_MACIPV6MCAST:
                *p_class_type = CTC_DKITS_FIB1_HASH_CLASS_TYPE_L2_MCAST;
                break;
            case FIBHOST1HASHTYPE_IPV4NATDAPORT:
            case FIBHOST1HASHTYPE_IPV4NATSAPORT:
                *p_class_type = CTC_DKITS_FIB1_HASH_CLASS_TYPE_IPV4_NAT;
                break;
            case FIBHOST1HASHTYPE_IPV6NATDAPORT:
            case FIBHOST1HASHTYPE_IPV6NATSAPORT:
                *p_class_type = CTC_DKITS_FIB1_HASH_CLASS_TYPE_IPV6_NAT;
                break;
            case FIBHOST1HASHTYPE_FCOERPF:
            case FIBHOST1HASHTYPE_TRILLMCASTVLAN:
                *p_class_type = CTC_DKITS_FIB1_HASH_CLASS_TYPE_OTHER;
                break;
            default:
                break;
        }
    }
    else if (CTC_DKITS_HASH_DB_MODULE_FLOW == module_type)
    {
        switch (key_type)
        {
            case FLOWHASHTYPE_L2:
                *p_class_type = CTC_DKITS_FLOW_HASH_CLASS_TYPE_MAC;
                break;
            case FLOWHASHTYPE_L2L3:
                *p_class_type = CTC_DKITS_FLOW_HASH_CLASS_TYPE_L2L3;
                break;
            case FLOWHASHTYPE_L3IPV4:
                *p_class_type = CTC_DKITS_FLOW_HASH_CLASS_TYPE_IPV4;
                break;
            case FLOWHASHTYPE_L3IPV6:
                *p_class_type = CTC_DKITS_FLOW_HASH_CLASS_TYPE_IPV6;
                break;
            case FLOWHASHTYPE_L3MPLS:
                *p_class_type = CTC_DKITS_FLOW_HASH_CLASS_TYPE_MPLS;
                break;
            default:
                break;
        }
    }
    else if (CTC_DKITS_HASH_DB_MODULE_IPFIX == module_type)
    {
        switch (key_type)
        {
            case IPFIXHASHTYPE_L2:
                *p_class_type = CTC_DKITS_IPFIX_HASH_CLASS_TYPE_MAC;
                break;
            case IPFIXHASHTYPE_L2L3:
                *p_class_type = CTC_DKITS_IPFIX_HASH_CLASS_TYPE_L2L3;
                break;
            case IPFIXHASHTYPE_L3IPV4:
                *p_class_type = CTC_DKITS_IPFIX_HASH_CLASS_TYPE_IPV4;
                break;
            case IPFIXHASHTYPE_L3IPV6:
                *p_class_type = CTC_DKITS_IPFIX_HASH_CLASS_TYPE_IPV6;
                break;
            case IPFIXHASHTYPE_L3MPLS:
                *p_class_type = CTC_DKITS_IPFIX_HASH_CLASS_TYPE_MPLS;
                break;
            default:
                break;
        }
    }
    else if (CTC_DKITS_HASH_DB_MODULE_USERID == module_type)
    {
        switch (key_type)
        {
            case USERIDHASHTYPE_DOUBLEVLANPORT:
            case USERIDHASHTYPE_SVLANPORT:
            case USERIDHASHTYPE_CVLANPORT:
            case USERIDHASHTYPE_SVLANCOSPORT:
            case USERIDHASHTYPE_CVLANCOSPORT:
            case USERIDHASHTYPE_PORT:
            case USERIDHASHTYPE_MAC:
            case USERIDHASHTYPE_SVLAN:
            case USERIDHASHTYPE_SVLANMACSA:
            //case DRV_ENUM(DRV_USERID_HASHTYPE_SCLFLOWL2):
            case USERIDHASHTYPE_IPV4PORT:
            case USERIDHASHTYPE_IPV6PORT:
            case USERIDHASHTYPE_MACPORT:
            case USERIDHASHTYPE_IPV4SA:
            case USERIDHASHTYPE_IPV6SA:
                *p_class_type = CTC_DKITS_SCL_HASH_CLASS_TYPE_SCL;
                break;
            case USERIDHASHTYPE_TUNNELIPV4:
            case USERIDHASHTYPE_TUNNELIPV4GREKEY:
            case USERIDHASHTYPE_TUNNELIPV4UDP:
            case USERIDHASHTYPE_TUNNELIPV4RPF:
            //case DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA):
                *p_class_type = CTC_DKITS_SCL_HASH_CLASS_TYPE_IPV4_TUNNEL;
                break;
            case USERIDHASHTYPE_TUNNELTRILLUCRPF:
            case USERIDHASHTYPE_TUNNELTRILLUCDECAP:
            case USERIDHASHTYPE_TUNNELTRILLMCRPF:
            case USERIDHASHTYPE_TUNNELTRILLMCDECAP:
            case USERIDHASHTYPE_TUNNELTRILLMCADJ:
                *p_class_type = CTC_DKITS_SCL_HASH_CLASS_TYPE_TRILL_TUNNEL;
                break;
            case USERIDHASHTYPE_TUNNELPBB:
                *p_class_type = CTC_DKITS_SCL_HASH_CLASS_TYPE_PBB_TUNNEL;
                break;
            case USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE0:
            case USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE1:
            case USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE0:
            case USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE1:
            case USERIDHASHTYPE_TUNNELIPV4MCVXLANMODE0:
            case USERIDHASHTYPE_TUNNELIPV4VXLANMODE1:
            case USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE0:
            case USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE1:
                *p_class_type = CTC_DKITS_SCL_HASH_CLASS_TYPE_VXLAN_TUNNEL;
                break;
            case USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE0:
            case USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE1:
            case USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE0:
            case USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE1:
            case USERIDHASHTYPE_TUNNELIPV4MCNVGREMODE0:
            case USERIDHASHTYPE_TUNNELIPV4NVGREMODE1:
            case USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE0:
            case USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE1:
                *p_class_type = CTC_DKITS_SCL_HASH_CLASS_TYPE_NVGRE_TUNNEL;
                break;
            case USERIDHASHTYPE_TUNNELMPLS:
                *p_class_type = CTC_DKITS_SCL_HASH_CLASS_TYPE_MPLS_TUNNEL;
                break;
            case USERIDHASHTYPE_CAPWAPSTASTATUS:
            case USERIDHASHTYPE_CAPWAPSTASTATUSMC:
            case USERIDHASHTYPE_CAPWAPMACDAFORWARD:
            case USERIDHASHTYPE_CAPWAPVLANFORWARD:
                *p_class_type = CTC_DKITS_SCL_HASH_CLASS_TYPE_WLAN;
                break;
            case USERIDHASHTYPE_TUNNELIPV4CAPWAP:
            case USERIDHASHTYPE_TUNNELIPV6CAPWAP:
            case USERIDHASHTYPE_TUNNELCAPWAPRMAC:
            case USERIDHASHTYPE_TUNNELCAPWAPRMACRID:
                *p_class_type = CTC_DKITS_SCL_HASH_CLASS_TYPE_WLAN_TUNNEL;
                break;
            case USERIDHASHTYPE_ECIDNAMESPACE:
            case USERIDHASHTYPE_INGECIDNAMESPACE:
                *p_class_type = CTC_DKITS_SCL_HASH_CLASS_TYPE_8021BR;
                break;
            default:
                if(key_type == DRV_ENUM(DRV_USERID_HASHTYPE_SCLFLOWL2))
                {
                    *p_class_type = CTC_DKITS_SCL_HASH_CLASS_TYPE_SCL;
                }
                else if(key_type == DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA))
                {
                    *p_class_type = CTC_DKITS_SCL_HASH_CLASS_TYPE_IPV4_TUNNEL;
                }
                break;
       }
    }

    return;
}

STATIC int32
_ctc_usw_dkits_dump_hash_show_mem_usage(uint8 lchip)
{
    int32  ret = CLI_SUCCESS;
    char   desc[50];
    uint32 i = 0, j = 0, t = 0, valid = 0, step = 0;
    uint32 dynamic_mem_bitmap = 0, total = 0, used = 0, cmd = 0, start = 0, end = 0;
    uint32** sram_alloc = NULL;
    uint32** sram_used = NULL;
    ctc_dkits_ds_t ds;
    char*  module[] = {"OAM/EgsScl", "FibHost0/Ipuc/Ipmc", "FibHost1/L2mc/Ipuc/Ipmc", "AclFlow", "Ipfix", "SclFlow/IgsScl/TunnelDecp/MPLS"};

    sram_alloc = (uint32**)sal_malloc(MAX_DRV_BLOCK_NUM * sizeof(uint32*));
    if (NULL == sram_alloc)
    {
        return CLI_ERROR;
    }

    sal_memset(sram_alloc, 0, MAX_DRV_BLOCK_NUM * sizeof(uint32*));
    for(i = 0; i < MAX_DRV_BLOCK_NUM; i++)
    {
        sram_alloc[i] = (uint32*)sal_malloc(CTC_DKITS_HASH_DB_MODULE_NUM * sizeof(uint32));
        if (NULL == sram_alloc[i])
        {
            ret = CLI_ERROR;
            goto clean;
        }
        sal_memset(sram_alloc[i], 0, CTC_DKITS_HASH_DB_MODULE_NUM * sizeof(uint32));
    }
    sram_used = (uint32**)sal_malloc(MAX_DRV_BLOCK_NUM * sizeof(uint32*));
    if (NULL == sram_used)
    {
        ret = CLI_ERROR;
        goto clean;
    }

    sal_memset(sram_used, 0, MAX_DRV_BLOCK_NUM * sizeof(uint32*));
    for(i = 0; i < MAX_DRV_BLOCK_NUM; i++)
    {
        sram_used[i] = (uint32*)sal_malloc(CTC_DKITS_HASH_DB_MODULE_NUM * sizeof(uint32));
        if (NULL == sram_used[i])
        {
            ret = CLI_ERROR;
            goto clean;
        }
        sal_memset(sram_used[i], 0, CTC_DKITS_HASH_DB_MODULE_NUM * sizeof(uint32));
    }

    CTC_DKIT_PRINT("\n");
    CTC_DKIT_PRINT(" ------------------------------------------------------------------\n");
    CTC_DKIT_PRINT(" %-9s%-32s%-9s%-9s%s\n", "SramId", "HashModule", "Total", "Used", "Percent");
    CTC_DKIT_PRINT(" ------------------------------------------------------------------\n");

    for (i = 0; i < CTC_DKITS_HASH_DB_MODULE_NUM; i++)
    {
        dynamic_mem_bitmap = (TABLE_INFO_PTR(lchip, usw_hash_key_field[i].tblid)
                             && TABLE_EXT_INFO_PTR(lchip, usw_hash_key_field[i].tblid)
                             && DYNAMIC_EXT_INFO_PTR(lchip, usw_hash_key_field[i].tblid))
                             ? DYNAMIC_BITMAP(lchip, usw_hash_key_field[i].tblid) : 0;

        for (j = 0; j < MAX_DRV_BLOCK_NUM; j++)
        {
            if (IS_BIT_SET(dynamic_mem_bitmap, j))
            {
                start = DYNAMIC_START_INDEX(lchip, usw_hash_key_field[i].tblid, j);
                end = DYNAMIC_END_INDEX(lchip, usw_hash_key_field[i].tblid, j);

                cmd = DRV_IOR(usw_hash_key_field[i].tblid, DRV_ENTRY_FLAG);
                step = (TABLE_ENTRY_SIZE(lchip, usw_hash_key_field[i].tblid) / DRV_BYTES_PER_ENTRY);

                for (t = start; t <= end; t += step)
                {
                    sal_memset(&ds, 0, sizeof(ctc_dkits_ds_t));
                    ret = DRV_IOCTL(lchip, t, cmd, &ds);
                    if (ret < 0)
                    {
                        CTC_DKIT_PRINT("Error read tbl %s[%u]\n", TABLE_NAME(lchip, usw_hash_key_field[i].tblid), t);
                        ret = CLI_ERROR;
                        goto clean;
                    }
                    valid = DRV_GET_FIELD_V(lchip, usw_hash_key_field[i].tblid, usw_hash_key_field[i].type, &ds);
                    if (valid)
                    {
                        sram_used[j][i] += TABLE_ENTRY_SIZE(lchip, usw_hash_key_field[i].tblid)/DRV_BYTES_PER_ENTRY;
                    }
                }
                sram_alloc[j][i] = DYNAMIC_ENTRY_NUM(lchip, usw_hash_key_field[i].tblid, j);
            }
            else
            {
                sram_alloc[j][i] = 0xFFFFFFFF;
            }
        }
    }

    for (i = 0; i < MAX_DRV_BLOCK_NUM; i++)
    {
        total = 0;
        used = 0;
        for (j = 0; j < CTC_DKITS_HASH_DB_MODULE_NUM; j++)
        {
            if (0xFFFFFFFF != sram_alloc[i][j])
            {
                total += sram_alloc[i][j];
            }
            used += sram_used[i][j];
        }
        for (j = 0; j < CTC_DKITS_HASH_DB_MODULE_NUM; j++)
        {
            if (0xFFFFFFFF != sram_alloc[i][j])
            {
                sal_sprintf(desc, "%u", sram_alloc[i][j]);
                break;
            }
        }

        if (j != CTC_DKITS_HASH_DB_MODULE_NUM)
        {
            if (0 == total)
            {
                sal_sprintf(desc, "%u", 0U);
            }
            else if (((used * 100) / total) < 1)
            {
                sal_sprintf(desc, "%s", "< 1");
            }
            else
            {
                sal_sprintf(desc, "%u", (used * 100) / total);
            }

            CTC_DKIT_PRINT(" %-9u%-32s%-9u%-9u%s\n", i + 1, module[j], total, used, desc);
        }
    }

    CTC_DKIT_PRINT("\n");
    CTC_DKIT_PRINT(" ---------------------------------------------------------\n");
    CTC_DKIT_PRINT(" %-32s%-9s%-9s%s\n", "HashModule", "Total", "Used", "Percent");
    CTC_DKIT_PRINT(" ---------------------------------------------------------\n");

    for (i = 0; i < CTC_DKITS_HASH_DB_MODULE_NUM; i++)
    {
        total = 0;
        used = 0;
        for (j = 0; j < MAX_DRV_BLOCK_NUM; j++)
        {
            if (0xFFFFFFFF != sram_alloc[j][i])
            {
                total += sram_alloc[j][i];
            }
            used += sram_used[j][i];
        }

        if (0 == total)
        {
            sal_sprintf(desc, "%u", 0U);
        }
        else if (((used * 100) / total) < 1)
        {
            sal_sprintf(desc, "%s", "< 1");
        }
        else
        {
            sal_sprintf(desc, "%u", (used * 100) / total);
        }
        CTC_DKIT_PRINT(" %-32s%-9u%-9u%s\n", module[i], total, used, desc);
    }
    CTC_DKIT_PRINT("\n");

clean:
    if(sram_alloc)
    {
        for(i = 0; i < MAX_DRV_BLOCK_NUM; i++)
        {
            sram_alloc[i]? sal_free(sram_alloc[i]):0;
        }
        sal_free(sram_alloc);
    }

    if(sram_used)
    {
        for(i = 0; i < MAX_DRV_BLOCK_NUM; i++)
        {
            sram_used[i]? sal_free(sram_used[i]) : 0;
        }
        sal_free(sram_used);
    }
    return ret;
}

STATIC int32
_ctc_usw_dkits_dump_hash_show_fun_usage(uint8 lchip, ctc_dkits_dump_fun_usage_t* p_fun_usage)
{
    uint32 i = 0, j = 0, v = 0, total = 0, level_total[5] = {0}, cam_total[CTC_DKITS_HASH_DB_MODULE_NUM] = {0};
    uint32 level_num = 0, level_en[MAX_HASH_LEVEL] = {0};
    uint32 hash_class[] = {CTC_DKITS_XCOAM_HASH_CLASS_TYPE_NUM,
                           CTC_DKITS_FIB0_HASH_CLASS_TYPE_NUM,
                           CTC_DKITS_FIB1_HASH_CLASS_TYPE_NUM,
                           CTC_DKITS_FLOW_HASH_CLASS_TYPE_NUM,
                           CTC_DKITS_IPFIX_HASH_CLASS_TYPE_NUM,
                           CTC_DKITS_SCL_HASH_CLASS_TYPE_NUM};

    char* class_desc[CTC_DKITS_HASH_DB_MODULE_NUM][12] = {{"EgsScl",    "PbbTunnel",     "OAM"},
                                                    {"FDB",       "IPv4Host",      "IPv4Mcast",    "IPv4NATSA",    "IPv6Ucast",
                                                     "IPv6Mcast", "Fcoe",          "Trill"},
                                                    {"IPv4NatDa/Sa",      "IPv4Mcast",    "IPv6NatDa/Sa", "IPv6Mcast",
                                                     "L2Mcast",   "Fcoe/Trill"},
                                                    {"Mac/Vlan/Cos/Port", "Mac/Vlan/Cos/IP/MPLS", "IPv4", "IPv6",
                                                     "MPLS"},
                                                    {"Mac/Vlan/Cos/Port", "Mac/Vlan/Cos/IP/MPLS", "IPv4", "IPv6",
                                                     "MPLS"},
                                                    {"SclFlow/IgsScl",    "IPv4Tunnel",      "IPv6Tunnel",
                                                     "NvgreTunnel",       "VxlanTunnel",     "MplsTunnel",  "PbbTunnel",
                                                     "TrillTunnel",       "WLANTunnel",    "WLAN",      "8021BR"}};
    char    desc[50] = {0};
    char*   module_desc[CTC_DKITS_HASH_DB_MODULE_NUM] = {"Oam/EgsScl", "FibHost0",   "FibHost1",
                                                   "AclFlow",    "IpFix",      "SclFlow/IgsScl/TunnelDecp/MPLS"};

    CTC_DKIT_PRINT("\n");
    /* CTC_DKIT_PRINT(" '-' means memory not allocated for the hash level.\n"); */
    for (i = 0; i < CTC_DKITS_HASH_DB_MODULE_NUM; i++)
    {
        CTC_DKIT_PRINT(" --------------------------------------------------------------------------------------------------\n");
        CTC_DKIT_PRINT(" %-32s%-9s", module_desc[i], "Total");
        level_num = CTC_DKITS_HASH_DB_LEVE_NUM(i);
        for (v = 0; v < level_num; v++)
        {
            sal_sprintf(desc, "Level%u", v);
            CTC_DKIT_PRINT("%-9s", desc);
        }

        if (MaxTblId_t != CTC_DKITS_HASH_DB_CAM_TBLID(i))
        {
            CTC_DKIT_PRINT("%-9s", "Cam");
        }
        CTC_DKIT_PRINT("\n");
        CTC_DKIT_PRINT(" --------------------------------------------------------------------------------------------------\n");

        total = 0;
        sal_memset(level_total, 0, sizeof(level_total));

        cam_total[i] = 0;
        for (j = 0; j < hash_class[i]; j++)
        {
            cam_total[i] += p_fun_usage->cam[i][j];

            for (v = 0; v < level_num; v++)
            {
                total += p_fun_usage->key[i][j][v];
                level_total[v] += p_fun_usage->key[i][j][v];
            }
        }

        CTC_DKIT_PRINT(" %-32s%-9u", "All", total);
        ctc_usw_dkits_hash_db_level_en(lchip, i, level_en);
        for (v = 0; v < level_num; v++)
        {
            if (level_en[v])
            {
                CTC_DKIT_PRINT("%-9u", level_total[v]);
            }
            else
            {
                CTC_DKIT_PRINT("%-9s", "-");
            }
        }

        if (MaxTblId_t != CTC_DKITS_HASH_DB_CAM_TBLID(i))
        {
            CTC_DKIT_PRINT("%-9u", cam_total[i]);
        }
        CTC_DKIT_PRINT("\n");

        for (j = 0; j < hash_class[i]; j++)
        {
            total = 0;
            for (v = 0; v < level_num; v++)
            {
                total += p_fun_usage->key[i][j][v];
            }

            CTC_DKIT_PRINT(" %-32s%-9u", class_desc[i][j], total);
            for (v = 0; v < level_num; v++)
            {
                if (level_en[v])
                {
                    CTC_DKIT_PRINT("%-9u", p_fun_usage->key[i][j][v]);
                }
                else
                {
                    CTC_DKIT_PRINT("%-9s", "-");
                }
            }

            if (MaxTblId_t != CTC_DKITS_HASH_DB_CAM_TBLID(i))
            {
                CTC_DKIT_PRINT("%-9u", p_fun_usage->cam[i][j]);
            }

            CTC_DKIT_PRINT("\n");
        }
    }
    CTC_DKIT_PRINT("\n");

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_dump_hash_show_key_usage(uint8 lchip, ctc_dkits_dump_hash_usage_t* p_hash_usage)
{
    uint32  i = 0, j = 0, k = 0, s = 0, sub_type_num = 0, level_en[MAX_HASH_LEVEL] = {0};
    uint32  total = 0, level_total[5] = {0}, cam_total[CTC_DKITS_HASH_DB_MODULE_NUM] = {0};
    uint8   level_num = 0;
    char    desc[50];
    char    format[50] = {0};
    char*   module_desc[CTC_DKITS_HASH_DB_MODULE_NUM] = {"Oam/EgsScl", "FibHost0", "FibHost1",
                                                         "AclFlow",    "IpFix",    "SclFlow/IgsScl/TunnelDecp/MPLS"};
    uint32  tbl_name_width[CTC_DKITS_HASH_DB_MODULE_NUM] = {38, 38, 38, 38, 38, 38};
    ctc_dkits_hash_tbl_info_t hash_tbl_info;

    CTC_DKIT_PRINT("\n");
    CTC_DKIT_PRINT(" '-' means memory not allocated for the hash level.\n");
    for (i = 0; i < CTC_DKITS_HASH_DB_MODULE_NUM; i++)
    {
        CTC_DKIT_PRINT(" -----------------------------------------------------------------------------------------------------------\n");

        CTC_DKITS_DUMP_CFG_ERROR_RETURN(ctc_usw_dkits_hash_db_level_en(lchip, i, level_en));
        sal_sprintf(format, " %%-%us%%-9s", tbl_name_width[i] + 3);
        CTC_DKIT_PRINT(format, module_desc[i], "Total");

        level_num = CTC_DKITS_HASH_DB_LEVE_NUM(i);
        for (k = 0; k < level_num; k++)
        {
            sal_sprintf(desc, "Level%u", k);
            CTC_DKIT_PRINT("%-9s", desc);
        }

        if (MaxTblId_t != CTC_DKITS_HASH_DB_CAM_TBLID(i))
        {
            CTC_DKIT_PRINT("%-9s", "Cam");
        }
        CTC_DKIT_PRINT("\n");
        CTC_DKIT_PRINT(" -----------------------------------------------------------------------------------------------------------\n");

        total = 0;
        sal_memset(level_total, 0, sizeof(level_total));

        cam_total[i] = 0;
        for (j = 0; j < CTC_DKITS_HASH_DB_KEY_NUM(i); j++)
        {
            sub_type_num = ((CTC_DKITS_HASH_DB_MODULE_FIB_HOST0 == i) && (FIBHOST0HASHTYPE_IPV4 == j)) ? 3 : 1;

            sal_memset(&hash_tbl_info, 0, sizeof(ctc_dkits_hash_tbl_info_t));
            CTC_DKITS_DUMP_CFG_ERROR_RETURN(_ctc_usw_dkits_dump_get_hash_info(lchip, CTC_DKITS_HASH_DB_KEY_TBLID(i, j), &hash_tbl_info));

            for (s = 0; s < sub_type_num; s++)
            {
                cam_total[i] += p_hash_usage->cam[i][hash_tbl_info.hash_type][s];
            }

            for (s = 0; s < sub_type_num; s++)
            {
                for (k = 0; k < level_num; k++)
                {
                    total += p_hash_usage->key[i][hash_tbl_info.hash_type][s][k];
                    level_total[k] += p_hash_usage->key[i][hash_tbl_info.hash_type][s][k];
                }
            }
        }

        sal_sprintf(format, " %%-%us%%-9u", tbl_name_width[i] + 3);
        CTC_DKIT_PRINT(format, "All", total);
        for (k = 0; k < level_num; k++)
        {
            if (level_en[k])
            {
                CTC_DKIT_PRINT("%-9u", level_total[k]);
            }
            else
            {
                CTC_DKIT_PRINT("%-9s", "-");
            }
        }

        if (MaxTblId_t != CTC_DKITS_HASH_DB_CAM_TBLID(i))
        {
            CTC_DKIT_PRINT("%-9u", cam_total[i]);
        }
        CTC_DKIT_PRINT("\n");

        for (j = 0; j < CTC_DKITS_HASH_DB_KEY_NUM(i); j++)
        {
            total = 0;
            sub_type_num = ((CTC_DKITS_HASH_DB_MODULE_FIB_HOST0 == i) && (FIBHOST0HASHTYPE_IPV4 == j)) ? 3 : 1;

            sal_memset(&hash_tbl_info, 0, sizeof(ctc_dkits_hash_tbl_info_t));
            CTC_DKITS_DUMP_CFG_ERROR_RETURN(_ctc_usw_dkits_dump_get_hash_info(lchip, CTC_DKITS_HASH_DB_KEY_TBLID(i, j), &hash_tbl_info));

            for (s = 0; s < sub_type_num; s++)
            {
                for (k = 0; k < level_num; k++)
                {
                    total += p_hash_usage->key[i][hash_tbl_info.hash_type][s][k];
                }
            }

            sal_strcpy(desc, TABLE_NAME(lchip, CTC_DKITS_HASH_DB_KEY_TBLID(i, j)));
            _ctc_usw_dkits_dump_cfg_brief_str(lchip, desc, sal_strlen(desc),\
                                                     ((sal_strlen(desc) > tbl_name_width[i]) \
                                                     ? (sal_strlen(desc) - tbl_name_width[i]) : 0));

            CTC_DKIT_PRINT(format, desc, total);
            for (k = 0; k < level_num; k++)
            {
                if (level_en[k])
                {
                    total = 0;
                    for (s = 0; s < sub_type_num; s++)
                    {
                        total += p_hash_usage->key[i][hash_tbl_info.hash_type][s][k];
                    }

                    CTC_DKIT_PRINT("%-9u", total);
                }
                else
                {
                    CTC_DKIT_PRINT("%-9s", "-");
                }
            }

            if (MaxTblId_t != CTC_DKITS_HASH_DB_CAM_TBLID(i))
            {
                total = 0;
                for (s = 0; s < sub_type_num; s++)
                {
                    total += p_hash_usage->cam[i][hash_tbl_info.hash_type][s];
                }

                CTC_DKIT_PRINT("%-9u", total);
            }

            CTC_DKIT_PRINT("\n");
        }
    }

    CTC_DKIT_PRINT("\n");

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_dump_hash_get_key_level(uint8 lchip, ctc_dkits_hash_db_module_t module, uint32 key_idx, uint32* p_level)
{
    uint32 couple_mode[MAX_HASH_LEVEL] = {0};
    uint32 index_base = 0, level_en[MAX_HASH_LEVEL] = {0};
    uint8  i = 0;

    CTC_DKITS_DUMP_CFG_ERROR_RETURN(ctc_usw_dkits_hash_db_level_en(lchip, module, level_en));
    CTC_DKITS_DUMP_CFG_ERROR_RETURN(ctc_usw_dkits_hash_db_get_couple_mode(lchip, module, couple_mode));

    for (i = 0; i < CTC_DKITS_HASH_DB_LEVE_NUM(module); i++)
    {
        if (!level_en[i])
        {
            continue;
        }
        index_base = (CTC_DKITS_HASH_DB_DEPTH(module, i) * CTC_DKITS_HASH_DB_BUCKET_NUM(module, i));
        index_base = index_base * (couple_mode[i] ? 2 : 1);
        if (key_idx < index_base)
        {
            *p_level = i;
            break;
        }
        key_idx -= index_base;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_dump_hash_get_cam_usage(uint8 lchip, ctc_dkits_hash_db_module_t module, ctc_dkits_dump_hash_usage_t* p_hash_usage)
{
    int32   ret = CLI_SUCCESS;
    uint32  i = 0, j = 0, hit = 0;
    uint32* idx = NULL;
    tbls_id_t tblid = MaxTblId_t;
    ctc_dkits_ds_t* key = NULL;
    ctc_dkits_hash_tbl_info_t hash_tbl_info;

    idx = (uint32*)sal_malloc(100 * sizeof(uint32));
    if (NULL == idx)
    {
        goto DKITS_DUMP_HASH_GET_CAM_USAGE;
    }
    key = (ctc_dkits_ds_t*)sal_malloc(32 * sizeof(ctc_dkits_ds_t));
    if (NULL == key)
    {
        goto DKITS_DUMP_HASH_GET_CAM_USAGE;
    }

    sal_memset(idx, 0, 100 * sizeof(uint32));
    sal_memset(key, 0, 32 * sizeof(ctc_dkits_ds_t));

    if (MaxTblId_t == CTC_DKITS_HASH_DB_CAM_TBLID(module))
    {
        ret = CLI_SUCCESS;
        goto DKITS_DUMP_HASH_GET_CAM_USAGE;
    }

    for (i = 0; i < CTC_DKITS_HASH_DB_KEY_NUM(module); i++)
    {
        tblid = CTC_DKITS_HASH_DB_KEY_TBLID(module, i);
        ret = _ctc_usw_dkits_dump_get_hash_info(lchip, tblid, &hash_tbl_info);
        if ((ret < 0) || (1 == ret))
        {
            goto DKITS_DUMP_HASH_GET_CAM_USAGE;
        }

        sal_memset(idx, 0, 100 * sizeof(uint32));
        ret = _ctc_usw_dkits_dump_search_cam(lchip, tblid, hash_tbl_info.hash_type, key, &hit, idx);
        if (CLI_SUCCESS != ret)
        {
            goto DKITS_DUMP_HASH_GET_CAM_USAGE;
        }

        for (j = 0; j < hit; j++)
        {
            if ((CTC_DKITS_HASH_DB_MODULE_FIB_HOST0 == module)
               && (FIBHOST0HASHTYPE_IPV4 == hash_tbl_info.hash_type))
            {
                uint32 ipv4_type = 0;

                /*
                 * 2'b00: ipv4 uc
                 * 2'b01: ipv4 mc
                 * 2'b10: ipda lookup for l2mc
                 * 2'b11: natsa
                 */
                ipv4_type = GetDsFibHost0Ipv4HashKey(V, ipv4Type_f, &key[j]);
                p_hash_usage->cam[module][hash_tbl_info.hash_type][ipv4_type]++;
            }
            else
            {
                p_hash_usage->cam[module][hash_tbl_info.hash_type][0]++;
            }
        }
    }

DKITS_DUMP_HASH_GET_CAM_USAGE:
    if (NULL != idx)
    {
        sal_free(idx);
    }
    if (NULL != key)
    {
        sal_free(key);
    }

    return ret;
}

STATIC int32
_ctc_usw_dkits_dump_hash_get_fun_usage(uint8 lchip, ctc_dkits_dump_hash_usage_t* p_hash_usage, ctc_dkits_dump_fun_usage_t* p_fun_usage)
{
    ctc_dkits_hash_tbl_info_t hash_tbl_info;
    uint32 i = 0, j = 0, v = 0, s = 0, fun = 0, level_num = 0, sub_type_num = 0;

    for (i = 0; i < CTC_DKITS_HASH_DB_MODULE_NUM; i++)
    {
        for (j = 0; j < CTC_DKITS_HASH_DB_KEY_NUM(i); j++)
        {
            level_num = CTC_DKITS_HASH_DB_LEVE_NUM(i);
            sub_type_num = ((CTC_DKITS_HASH_DB_MODULE_FIB_HOST0 == i) && (FIBHOST0HASHTYPE_IPV4 == j)) ? 3 : 1;

            sal_memset(&hash_tbl_info, 0, sizeof(ctc_dkits_hash_tbl_info_t));
            _ctc_usw_dkits_dump_get_hash_info(lchip, CTC_DKITS_HASH_DB_KEY_TBLID(i, j), &hash_tbl_info);

            for (v = 0; v < level_num; v++)
            {
                for (s = 0; s < sub_type_num; s++)
                {
                    _ctc_usw_dkits_dump_hash_class_key(lchip, i, hash_tbl_info.hash_type, s, &fun);
                    p_fun_usage->key[i][fun][v] += p_hash_usage->key[i][hash_tbl_info.hash_type][s][v];
                }
            }
            for (s = 0; s < sub_type_num; s++)
            {
                _ctc_usw_dkits_dump_hash_class_key(lchip, i, hash_tbl_info.hash_type, s, &fun);
                p_fun_usage->cam[i][fun] += p_hash_usage->cam[i][hash_tbl_info.hash_type][s];
            }
        }
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_dump_hash_get_key_usage(uint8 lchip, ctc_dkits_dump_hash_usage_t* p_hash_usage)
{
    uint32  i = 0, j = 0, t = 0, valid = 0, level = 0;
    uint32  cmd = 0, step = 0, type = 0;
    ctc_dkits_ds_t ds;
    tbls_id_t tblid = MaxTblId_t;
    ctc_dkits_hash_tbl_info_t hash_tbl_info;

    for (i = 0; i < CTC_DKITS_HASH_DB_MODULE_NUM; i++)
    {
        CTC_DKITS_DUMP_CFG_ERROR_RETURN(_ctc_usw_dkits_dump_hash_get_cam_usage(lchip, i, p_hash_usage));

        for (j = 0; j < CTC_DKITS_HASH_DB_KEY_NUM(i); j++)
        {
            tblid = CTC_DKITS_HASH_DB_KEY_TBLID(i, j);
            step = TABLE_ENTRY_SIZE(lchip, tblid) / DRV_BYTES_PER_ENTRY;

            cmd = DRV_IOR(tblid, DRV_ENTRY_FLAG);

            for (t = 0; t < TABLE_MAX_INDEX(lchip, tblid); t += step)
            {
                sal_memset(&ds, 0, sizeof(ctc_dkits_ds_t));
                CTC_DKITS_DUMP_CFG_ERROR_RETURN(DRV_IOCTL(lchip, t, cmd, &ds));

                valid = DRV_GET_FIELD_V(lchip, usw_hash_key_field[i].tblid, usw_hash_key_field[i].valid, &ds);
                if (valid)
                {
                    _ctc_usw_dkits_dump_get_hash_type(lchip, tblid, &ds, &type);
                    sal_memset(&hash_tbl_info, 0, sizeof(ctc_dkits_hash_tbl_info_t));
                    CTC_DKITS_DUMP_CFG_ERROR_RETURN(_ctc_usw_dkits_dump_get_hash_info(lchip, tblid, &hash_tbl_info));
                    if (type == hash_tbl_info.hash_type)
                    {
                        CTC_DKITS_DUMP_CFG_ERROR_RETURN(_ctc_usw_dkits_dump_hash_get_key_level(lchip, i, t, &level));

                        if (DsFibHost0Ipv4HashKey_t == tblid)
                        {
                            uint32 ipv4_type = 0;
                            /*
                            * 2'b00: ipv4 uc      IPV4KEYTYPE_IPUC
                            * 2'b01: ipv4 mc      IPV4KEYTYPE_IPMC
                            * 2'b10  layer2 type  IPV4KEYTYPE_L2MC
                            * 2'b11: natsa        IPV4KEYTYPE_NATSA
                            */
                            ipv4_type = GetDsFibHost0Ipv4HashKey(V, ipv4Type_f, &ds);
                            p_hash_usage->key[i][hash_tbl_info.hash_type][ipv4_type][level]++;
                        }
                        else
                        {
                            p_hash_usage->key[i][hash_tbl_info.hash_type][0][level]++;
                        }
                    }
                }
            }
        }
        sal_udelay(10);
    }

    return CLI_SUCCESS;
}

#define ________DKIT_DUMP_EXTERNAL_FUNCTION_________

int32
ctc_usw_dkits_dump_static(uint8 lchip, sal_file_t p_f, ctc_dkits_dump_func_flag_t flag)
{
    int32  ret = CLI_SUCCESS;
    uint32 count = 0;

    CTC_DKIT_LCHIP_CHECK(lchip);

    if (DKITS_IS_BIT_SET(flag, CTC_DKITS_DUMP_FUNC_STATIC_HSS))
    {
        if (1 == SDK_WORK_PLATFORM)
        {
            return 0;
        }
        if (DRV_IS_DUET2(lchip))
        {
            ret = _ctc_usw_dkits_dump_travel_static_cfg(lchip, NULL, &count, NULL, dt2_serdes_tbl, &_ctc_usw_dkits_dump_count_tbl);
            ret = ret ? ret : _ctc_usw_dkits_dump_travel_static_cfg(lchip, &lchip, p_f, &count, dt2_serdes_tbl, &_ctc_usw_dkits_dump_write_tbl);
        }
        else if (DRV_IS_TSINGMA(lchip))
        {
            ret = _ctc_usw_dkits_dump_travel_static_cfg(lchip, NULL, &count, NULL, tm_serdes_tbl, &_ctc_usw_dkits_dump_count_tbl);
            ret = ret ? ret : _ctc_usw_dkits_dump_travel_static_cfg(lchip, &lchip, p_f, &count, tm_serdes_tbl, &_ctc_usw_dkits_dump_write_tbl);
        }
    }
    else if (DKITS_IS_BIT_SET(flag, CTC_DKITS_DUMP_FUNC_STATIC_SERDES))
    {
        if (1 == SDK_WORK_PLATFORM)
        {
            return 0;
        }

        ret = _ctc_usw_dkits_dump_indirect(lchip, p_f, flag);
    }
    else if (DKITS_IS_BIT_SET(flag, CTC_DKITS_DUMP_FUNC_STATIC_ALL))
    {
        ret = _ctc_usw_dkits_dump_travel_static_all(lchip, NULL, &count, NULL, &_ctc_usw_dkits_dump_count_tbl);
        ret = ret ? ret : _ctc_usw_dkits_dump_travel_static_all(lchip, (void*)&lchip, p_f, &count, &_ctc_usw_dkits_dump_write_tbl);
    }

    return ret;
}

int32
ctc_usw_dkits_dump_dynamic(uint8 lchip, sal_file_t p_f, ctc_dkits_dump_func_flag_t flag)
{
    int32 ret = CLI_SUCCESS;
    uint8 index = 0;

    CTC_DKIT_LCHIP_CHECK(lchip);

    /* Hash key dump */
    if (DKITS_IS_BIT_SET(flag, CTC_DKITS_DUMP_FUNC_DYN_FDB))
    {
        /*only store FIB Host0*/
        ret = _ctc_usw_dkits_dump_hash_tbl(lchip,CTC_DKITS_HASH_FUN_TYPE_FIB, CTC_DKITS_FIB_HASH_FUN_TYPE_BRIDGE, p_f);
    }

    if (DKITS_IS_BIT_SET(flag, CTC_DKITS_DUMP_FUNC_DYN_SCL))
    {
        /* Hash scl */
        for (index = 0; index < CTC_DKITS_SCL_HASH_FUN_TYPE_NUM; index++)
        {
            ret = ret ? ret: _ctc_usw_dkits_dump_hash_tbl(lchip, CTC_DKITS_HASH_FUN_TYPE_SCL, index, p_f);
        }

        /*egress scl shard with oam */
        ret = ret ? ret: _ctc_usw_dkits_dump_hash_tbl(lchip, CTC_DKITS_HASH_FUN_TYPE_XCOAM, CTC_DKITS_XCOAM_HASH_FUN_TYPE_SCL, p_f);
        ret = ret ? ret: _ctc_usw_dkits_dump_hash_tbl(lchip, CTC_DKITS_HASH_FUN_TYPE_XCOAM, CTC_DKITS_XCOAM_HASH_FUN_TYPE_PBB_TUNNEL, p_f);
    }

    if (DKITS_IS_BIT_SET(flag, CTC_DKITS_DUMP_FUNC_DYN_ACL))
    {
        /* Hash flow */
        for (index = 0; index < CTC_DKITS_FLOW_HASH_FUN_TYPE_NUM; index++)
        {
            ret = ret ? ret: _ctc_usw_dkits_dump_hash_tbl(lchip, CTC_DKITS_HASH_FUN_TYPE_FLOW, index, p_f);
        }
    }

    if (DKITS_IS_BIT_SET(flag, CTC_DKITS_DUMP_FUNC_DYN_OAM))
    {
        /* just hash */
        ret = ret ? ret: _ctc_usw_dkits_dump_hash_tbl(lchip, CTC_DKITS_HASH_FUN_TYPE_XCOAM, CTC_DKITS_XCOAM_HASH_FUN_TYPE_OAM, p_f);
    }

    if (DKITS_IS_BIT_SET(flag, CTC_DKITS_DUMP_FUNC_DYN_IPUC))
    {
        /* Host route */
        ret = ret ? ret: _ctc_usw_dkits_dump_hash_tbl(lchip, CTC_DKITS_HASH_FUN_TYPE_FIB, CTC_DKITS_FIB_HASH_FUN_TYPE_IPUC_IPV4, p_f);
        ret = ret ? ret: _ctc_usw_dkits_dump_hash_tbl(lchip, CTC_DKITS_HASH_FUN_TYPE_FIB, CTC_DKITS_FIB_HASH_FUN_TYPE_IPUC_IPV6, p_f);
    }

    if (DKITS_IS_BIT_SET(flag, CTC_DKITS_DUMP_FUNC_DYN_IPMC))
    {
        /* Host route */
        ret = ret ? ret: _ctc_usw_dkits_dump_hash_tbl(lchip, CTC_DKITS_HASH_FUN_TYPE_FIB, CTC_DKITS_FIB_HASH_FUN_TYPE_IPMC_IPV4, p_f);
        ret = ret ? ret: _ctc_usw_dkits_dump_hash_tbl(lchip, CTC_DKITS_HASH_FUN_TYPE_FIB, CTC_DKITS_FIB_HASH_FUN_TYPE_IPMC_IPV6, p_f);
    }

    if (DKITS_IS_BIT_SET(flag, CTC_DKITS_DUMP_FUNC_DYN_IPFIX))
    {
        for (index = CTC_DKITS_IPFIX_HASH_FUN_TYPE_BRIDGE; index < CTC_DKITS_IPFIX_HASH_FUN_TYPE_NUM; index++)
        {
            ret = ret ? ret: _ctc_usw_dkits_dump_hash_tbl(lchip, CTC_DKITS_HASH_FUN_TYPE_IPFIX, index, p_f);
        }
    }

    /* Tcam key dump */
    if (DKITS_IS_BIT_SET(flag, CTC_DKITS_DUMP_FUNC_DYN_SCL))
    {
        /* Tcam scl */
        for (index = 0; index < CTC_DKITS_SCL_TCAM_FUN_TYPE_NUM; index++)
        {
            ret = ret ? ret: _ctc_usw_dkits_dump_tcam_tbl(lchip, CTC_DKITS_TCAM_FUN_TYPE_SCL, index, p_f);
        }
    }

    if (DKITS_IS_BIT_SET(flag, CTC_DKITS_DUMP_FUNC_DYN_ACL))
    {
        /* Tcam acl */
        for (index = 0; index < CTC_DKITS_ACL_TCAM_FUN_TYPE_NUM; index++)
        {
            if (CTC_DKITS_ACL_TCAM_FUN_TYPE_IPFIX != index)
            {
                ret = ret ? ret: _ctc_usw_dkits_dump_tcam_tbl(lchip, CTC_DKITS_TCAM_FUN_TYPE_ACL, index, p_f);
            }
        }
    }

    if (DKITS_IS_BIT_SET(flag, CTC_DKITS_DUMP_FUNC_DYN_IPUC))
    {
        /* Other route */
        for (index = CTC_DKITS_NAT_PBR_TCAM_FUN_TYPE_PREFIX; index < CTC_DKITS_NAT_PBR_TCAM_FUN_TYPE_NUM; index++)
        {
            ret = ret ? ret: _ctc_usw_dkits_dump_tcam_tbl(lchip, CTC_DKITS_TCAM_FUN_TYPE_IP, index, p_f);
        }
    }

    if (DKITS_IS_BIT_SET(flag, CTC_DKITS_DUMP_FUNC_DYN_IPFIX))
    {
        ret = ret ? ret: _ctc_usw_dkits_dump_tcam_tbl(lchip, CTC_DKITS_TCAM_FUN_TYPE_ACL, CTC_DKITS_ACL_TCAM_FUN_TYPE_IPFIX, p_f);
    }

    if (DKITS_IS_BIT_SET(flag, CTC_DKITS_DUMP_FUNC_DYN_CID))
    {
        ret = ret ? ret: _ctc_usw_dkits_dump_tcam_tbl(lchip, CTC_DKITS_TCAM_FUN_TYPE_FWD, CTC_DKITS_FWD_TCAM_FUN_TYPE_CID, p_f);
    }

    if (DKITS_IS_BIT_SET(flag, CTC_DKITS_DUMP_FUNC_DYN_QUE))
    {
        ret = ret ? ret: _ctc_usw_dkits_dump_tcam_tbl(lchip, CTC_DKITS_TCAM_FUN_TYPE_QMGR, CTC_DKITS_QMGR_TCAM_FUN_TYPE_QUEUE_MAP, p_f);
    }

    return ret;
}

int32
ctc_usw_dkits_dump_memory_usage(uint8 lchip, void* p_para)
{
    ctc_dkits_dump_hash_usage_t* p_hash_usage = NULL;
    ctc_dkits_dump_fun_usage_t* p_fun_usage = NULL;
    int32 ret = CLI_SUCCESS;

    ctc_dkits_dump_usage_type_t type = *((ctc_dkits_dump_usage_type_t*)p_para);

    DRV_INIT_CHECK(lchip);

    if (CTC_DKITS_DUMP_USAGE_TCAM_KEY == type)
    {
        CTC_DKITS_DUMP_CFG_ERROR_RETURN(_ctc_usw_dkits_dump_tcam_mem_usage(lchip));
    }
    else if (CTC_DKITS_DUMP_USAGE_HASH_BRIEF == type)
    {
        p_hash_usage = (ctc_dkits_dump_hash_usage_t*)sal_malloc(sizeof(ctc_dkits_dump_hash_usage_t));
        if (NULL == p_hash_usage)
        {
            CTC_DKIT_PRINT("%s %d, malloc hash_usage failed!\n", __FUNCTION__, __LINE__);
            goto DUMP_MEMORY_USAGE_ERROR;
        }
        p_fun_usage = (ctc_dkits_dump_fun_usage_t*)sal_malloc(sizeof(ctc_dkits_dump_fun_usage_t));
        if (NULL == p_fun_usage)
        {
            CTC_DKIT_PRINT("%s %d, malloc fun_usage failed!\n", __FUNCTION__, __LINE__);
            goto DUMP_MEMORY_USAGE_ERROR;
        }

        sal_memset(p_hash_usage, 0, sizeof(ctc_dkits_dump_hash_usage_t));
        sal_memset(p_fun_usage, 0, sizeof(ctc_dkits_dump_fun_usage_t));

        ret = _ctc_usw_dkits_dump_hash_get_key_usage(lchip, p_hash_usage);
        if ((ret < 0) || (1 == ret))
        {
            goto DUMP_MEMORY_USAGE_ERROR;
        }
        ret = _ctc_usw_dkits_dump_hash_get_fun_usage(lchip, p_hash_usage, p_fun_usage);
        if ((ret < 0) || (1 == ret))
        {
            goto DUMP_MEMORY_USAGE_ERROR;
        }
        ret = _ctc_usw_dkits_dump_hash_show_fun_usage(lchip, p_fun_usage);
        if ((ret < 0) || (1 == ret))
        {
            goto DUMP_MEMORY_USAGE_ERROR;
        }
    }
    else if (CTC_DKITS_DUMP_USAGE_HASH_DETAIL == type)
    {
        p_hash_usage = (ctc_dkits_dump_hash_usage_t*)sal_malloc(sizeof(ctc_dkits_dump_hash_usage_t));
        if (NULL == p_hash_usage)
        {
            CTC_DKIT_PRINT("%s %d, malloc hash_usage failed!\n", __FUNCTION__, __LINE__);
            goto DUMP_MEMORY_USAGE_ERROR;
        }
        sal_memset(p_hash_usage, 0, sizeof(ctc_dkits_dump_hash_usage_t));

        ret = _ctc_usw_dkits_dump_hash_get_key_usage(lchip, p_hash_usage);
        if ((ret < 0) || (1 == ret))
        {
            goto DUMP_MEMORY_USAGE_ERROR;
        }
        ret = _ctc_usw_dkits_dump_hash_show_key_usage(lchip, p_hash_usage);
        if ((ret < 0) || (1 == ret))
        {
            goto DUMP_MEMORY_USAGE_ERROR;
        }
    }
    else if (CTC_DKITS_DUMP_USAGE_HASH_MEM == type)
    {
        CTC_DKITS_DUMP_CFG_ERROR_RETURN(_ctc_usw_dkits_dump_hash_show_mem_usage(lchip));
    }

DUMP_MEMORY_USAGE_ERROR:
    if (NULL != p_hash_usage)
    {
        sal_free(p_hash_usage);
    }

    if (NULL != p_fun_usage)
    {
        sal_free(p_fun_usage);
    }

    return ret;
}

int32
ctc_usw_dkits_dump_cfg_dump(uint8 lchip, void* p_info)
{
    int32 ret = CLI_SUCCESS;
    ctc_dkits_dump_cfg_t* p_dump_cfg = NULL;
    sal_file_t p_wf = NULL;
    char cfg_file[300] = {0};
    ctc_dkits_dump_centec_file_hdr_t centec_file_hdr;
    sal_time_t begine_time, end_time;
    host_type_t host_endian = HOST_LE;
    char chip_name[CTC_DKIT_CHIP_NAME_LEN] = {0};

    DKITS_PTR_VALID_CHECK(p_info);
    DRV_INIT_CHECK(lchip);

    p_dump_cfg = (ctc_dkits_dump_cfg_t*)p_info;

    sal_sprintf(cfg_file, "%s%s", p_dump_cfg->file, DUMP_CENTEC_FILE_POSTFIX);

    p_wf = sal_fopen(cfg_file, "wb+");

    if (NULL == p_wf)
    {
        CTC_DKIT_PRINT(" Store file: %s failed!\n\n", cfg_file);
        goto CTC_DKITS_DUMP_CFG_END;
    }

    sal_memset(&centec_file_hdr, 0, sizeof(ctc_dkits_dump_centec_file_hdr_t));
    sal_strcpy((char*)(centec_file_hdr.centec_cfg_stamp), CENTEC_CFG_STAMP);
    sal_strcpy((char*)(centec_file_hdr.version), CTC_DKITS_VERSION_STR);
    sal_strcpy((char*)(centec_file_hdr.sdk_release_data), CTC_DKITS_RELEASE_DATE);
    sal_strcpy((char*)(centec_file_hdr.copy_right_time), CTC_DKITS_COPYRIGHT_TIME);
    ctc_usw_dkit_get_chip_name(lchip, chip_name);
    sal_strcpy((char*)(centec_file_hdr.chip_name), chip_name);
    centec_file_hdr.func_flag = p_dump_cfg->func_flag;
    host_endian = drv_get_host_type(lchip);
    centec_file_hdr.endian_type = host_endian;
    sal_fwrite(&centec_file_hdr, sizeof(ctc_dkits_dump_centec_file_hdr_t), 1, p_wf);

    if (DKITS_IS_BIT_SET(p_dump_cfg->func_flag, CTC_DKITS_DUMP_FUNC_STATIC_ALL)
        || DKITS_IS_BIT_SET(p_dump_cfg->func_flag, CTC_DKITS_DUMP_FUNC_STATIC_HSS)
        || DKITS_IS_BIT_SET(p_dump_cfg->func_flag, CTC_DKITS_DUMP_FUNC_STATIC_SERDES))
    {
        sal_time(&begine_time);
        ret = ctc_usw_dkits_dump_static(lchip, p_wf, p_dump_cfg->func_flag);
        sal_time(&end_time);

        if (CLI_ERROR == ret)
        {
            goto CTC_DKITS_DUMP_CFG_END;
        }
    }
    else
    {
        ret = ctc_usw_dkits_dump_dynamic(lchip, p_wf, p_dump_cfg->func_flag);
    }

    if (CLI_ERROR == ret)
    {
        goto CTC_DKITS_DUMP_CFG_END;
    }

    CTC_DKIT_PRINT(" Store file: %s\n\n", cfg_file);
     /*CTC_DKIT_PRINT("\nTime elapse: %lf(s)\n", interval);*/

CTC_DKITS_DUMP_CFG_END:
    if (NULL != p_wf)
    {
        sal_fclose(p_wf);
    }
    return CLI_SUCCESS;
}

int32
ctc_usw_dkits_dump_cfg_decode(uint8 lchip, void* p_para)
{
    int32 ret = CLI_SUCCESS;
    char* p_file = NULL;
    char* p_str = NULL;
    sal_file_t p_rf = NULL;
    sal_file_t p_wf = NULL;
    char txt_file_path[256] = {0};
    uint8 data_translate = 0;
    sal_file_t p_file_temp = NULL;
    ctc_dkits_dump_centec_file_hdr_t centec_file_hdr;
    sal_time_t begine_time, end_time;
    host_type_t cfg_endian = HOST_LE, host_endian = HOST_LE;
    ctc_dkits_dump_cfg_t*    p_dump_cfg = (ctc_dkits_dump_cfg_t*)p_para;
    char chip_name[CTC_DKIT_CHIP_NAME_LEN] = {0};

    data_translate = ((ctc_dkits_dump_cfg_t*)p_para)->data_translate;

    DKITS_PTR_VALID_CHECK(p_para);
    DRV_INIT_CHECK(lchip);
    p_file = p_dump_cfg->file;

    sal_strcpy(txt_file_path, p_file);
    p_str = sal_strstr(txt_file_path, DUMP_CENTEC_FILE_POSTFIX);
    if (NULL != p_str)
    {
        *p_str = '\0';
    }

    sal_strcat(txt_file_path, DUMP_DECODC_TXT_FILE_POSTFIX);

    p_rf = sal_fopen(p_file, "rb");
    if (NULL == p_rf)
    {
        CTC_DKIT_PRINT(" Access centec file: %s failed!\n\n", p_file);
        goto CTC_DKITS_DUMP_CFG_DECODE_END;
    }

    sal_memset(&centec_file_hdr, 0, sizeof(ctc_dkits_dump_centec_file_hdr_t));
    if (1 != sal_fread(&centec_file_hdr, sizeof(ctc_dkits_dump_centec_file_hdr_t), 1, p_rf))
    {
        goto CTC_DKITS_DUMP_CFG_DECODE_END;
    }

    cfg_endian = (0 == centec_file_hdr.endian_type) ? HOST_LE : HOST_BE;
    host_endian = drv_get_host_type(lchip);

    if (cfg_endian != host_endian)
    {
        ctc_dkits_dump_swap32((uint32*)&centec_file_hdr.func_flag, sizeof(centec_file_hdr.func_flag) / 4);
    }

    if (0 != sal_strcmp((char*)(centec_file_hdr.centec_cfg_stamp), CENTEC_CFG_STAMP))
    {
        CTC_DKIT_PRINT(" File: %s is not centec file!\n\n", p_file);
        goto CTC_DKITS_DUMP_CFG_DECODE_END;
    }

    ctc_usw_dkit_get_chip_name(lchip, chip_name);
    if (0 != sal_strcmp((char*)(centec_file_hdr.chip_name), chip_name))
    {
        CTC_DKIT_PRINT(" Centec file: %s chip series mismatch!\n\n", p_file);
        goto CTC_DKITS_DUMP_CFG_DECODE_END;
    }

    if (0 != sal_strcmp((char*)(centec_file_hdr.version), CTC_DKITS_VERSION_STR))
    {
        CTC_DKIT_PRINT(" Centec file: %s chip series mismatch!\n\n", p_file);
        goto CTC_DKITS_DUMP_CFG_DECODE_END;
    }

    p_wf = sal_fopen(txt_file_path, "wt");
    if (NULL == p_wf)
    {
        CTC_DKIT_PRINT(" Write %s file failed!\n\n", txt_file_path);
        goto CTC_DKITS_DUMP_CFG_DECODE_END;
    }

    ctc_usw_dkit_misc_chip_info(lchip, p_wf);

    if (data_translate)
    {
        p_file_temp = dkits_log_file;
        dkits_log_file = p_wf;
    }

    sal_time(&begine_time);

    ret = ctc_usw_dkits_dump_data2text(lchip, p_rf, cfg_endian, p_wf, data_translate);
    sal_time(&end_time);

    if (data_translate)
    {
        dkits_log_file = p_file_temp;
    }

    if (CLI_ERROR == ret)
    {
        goto CTC_DKITS_DUMP_CFG_DECODE_END;
    }

    CTC_DKIT_PRINT(" Convert centec file: %s to text file: %s\n\n", p_file, txt_file_path);
     /*CTC_DKIT_PRINT("\nTime elapse: %lf(s)\n", interval);*/

CTC_DKITS_DUMP_CFG_DECODE_END:
    if (NULL != p_wf)
    {
        sal_fclose(p_wf);
    }

    if (NULL != p_rf)
    {
        sal_fclose(p_rf);
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_dkits_dump_cfg_cmp(uint8 lchip, void* p_para1, void* p_para2)
{
    sal_file_t p_srf = NULL;
    sal_file_t p_drf = NULL;
    sal_file_t p_wf = NULL;
    char dump_diff_file[256] = "dump_cfg.diff";
    ctc_dkits_dump_centec_file_hdr_t centec_file_hdr1;
    ctc_dkits_dump_centec_file_hdr_t centec_file_hdr2;
    sal_time_t begine_time, end_time;
    host_type_t src_cfg_endian = HOST_LE, dst_cfg_endian = HOST_LE, host_endian = HOST_LE;
    int32 ret = CLI_SUCCESS;
    char chip_name[CTC_DKIT_CHIP_NAME_LEN] = {0};

    DKITS_PTR_VALID_CHECK(p_para1);
    DKITS_PTR_VALID_CHECK(p_para2);
    DRV_INIT_CHECK(lchip);

    p_srf = sal_fopen((char*)p_para1, "rb");
    if (NULL == p_srf)
    {
        CTC_DKIT_PRINT(" Read file1: %s failed!\n\n", (char*)p_para1);
        return CLI_ERROR;
    }

    p_drf = sal_fopen((char*)p_para2, "rb");
    if (NULL == p_drf)
    {
        CTC_DKIT_PRINT(" Read file2: %s failed!\n\n", (char*)p_para2);
        ret = CLI_ERROR;
        goto CTC_DKITS_DUMP_CFG_CMP_END;
    }

    sal_memset(&centec_file_hdr1, 0, sizeof(ctc_dkits_dump_centec_file_hdr_t));
    sal_memset(&centec_file_hdr2, 0, sizeof(ctc_dkits_dump_centec_file_hdr_t));

    if(1 != sal_fread(&centec_file_hdr1, sizeof(ctc_dkits_dump_centec_file_hdr_t), 1, p_srf))
    {
        CTC_DKIT_PRINT(" Read file_hdr1  failed!\n\n");
        ret = CLI_ERROR;
        goto CTC_DKITS_DUMP_CFG_CMP_END;
    }
    if(1 != sal_fread(&centec_file_hdr2, sizeof(ctc_dkits_dump_centec_file_hdr_t), 1, p_drf))
    {
        CTC_DKIT_PRINT(" Read file_hdr2  failed!\n\n");
        ret = CLI_ERROR;
        goto CTC_DKITS_DUMP_CFG_CMP_END;
    }

    host_endian = drv_get_host_type(lchip);
    src_cfg_endian = (0 == centec_file_hdr1.endian_type) ? HOST_LE : HOST_BE;
    dst_cfg_endian = (0 == centec_file_hdr2.endian_type) ? HOST_LE : HOST_BE;

    if (src_cfg_endian != host_endian)
    {
        ctc_dkits_dump_swap32((uint32*)&centec_file_hdr1.func_flag, sizeof(centec_file_hdr1.func_flag) / 4);
    }

    if (dst_cfg_endian != host_endian)
    {
        ctc_dkits_dump_swap32((uint32*)&centec_file_hdr2.func_flag, sizeof(centec_file_hdr2.func_flag) / 4);
    }

    if (0 != sal_strcmp((char*)(centec_file_hdr1.centec_cfg_stamp), CENTEC_CFG_STAMP))
    {
        CTC_DKIT_PRINT(" File: %s is not centec file!\n\n", (char*)p_para1);
        goto CTC_DKITS_DUMP_CFG_CMP_END;
    }

    if (0 != sal_strcmp((char*)(centec_file_hdr2.centec_cfg_stamp), CENTEC_CFG_STAMP))
    {
        CTC_DKIT_PRINT(" File: %s is not centec file!\n\n", (char*)p_para2);
        goto CTC_DKITS_DUMP_CFG_CMP_END;
    }

    ctc_usw_dkit_get_chip_name(lchip, chip_name);
    if (0 != sal_strcmp((char*)(centec_file_hdr1.chip_name), chip_name))
    {
        CTC_DKIT_PRINT(" File: %s is not centec file!\n\n", (char*)p_para1);
        goto CTC_DKITS_DUMP_CFG_CMP_END;
    }

    if (0 != sal_strcmp((char*)(centec_file_hdr2.chip_name), chip_name))
    {
        CTC_DKIT_PRINT(" File: %s is not centec file!\n\n", (char*)p_para2);
        goto CTC_DKITS_DUMP_CFG_CMP_END;
    }

    if (0 != sal_strcmp((char*)(centec_file_hdr1.version), (char*)(centec_file_hdr2.version)))
    {
        CTC_DKIT_PRINT(" Centec file1 version: %s,  centec file2 version: %s, mismatch!\n\n",
                       centec_file_hdr1.version, centec_file_hdr2.version);
        goto CTC_DKITS_DUMP_CFG_CMP_END;
    }

    if (centec_file_hdr1.func_flag != centec_file_hdr2.func_flag)
    {
        CTC_DKIT_PRINT(" Centec file1 and centec file2 dump option mismatch!\n\n");
        goto CTC_DKITS_DUMP_CFG_CMP_END;
    }

    p_wf = sal_fopen(dump_diff_file, "wt+");
    if (NULL == p_wf)
    {
        CTC_DKIT_PRINT(" Create diff file: %s\n\n", dump_diff_file);
        goto CTC_DKITS_DUMP_CFG_CMP_END;
    }

    sal_fprintf(p_wf, "File1:%s\n", (char*)p_para1);
    sal_fprintf(p_wf, "File2:%s\n", (char*)p_para2);

    if (DKITS_IS_BIT_SET(centec_file_hdr1.func_flag, CTC_DKITS_DUMP_FUNC_STATIC_ALL)
       || DKITS_IS_BIT_SET(centec_file_hdr1.func_flag, CTC_DKITS_DUMP_FUNC_STATIC_HSS))
    {
        ctc_dkits_dump_txt_header(CTC_DKITS_DUMP_TEXT_HEADER_DIFF_TBL, p_wf);
    }
    else if (DKITS_IS_BIT_SET(centec_file_hdr1.func_flag, CTC_DKITS_DUMP_FUNC_STATIC_SERDES))
    {
        ctc_dkits_dump_txt_header(CTC_DKITS_DUMP_TEXT_HEADER_DIFF_SERDES, p_wf);
    }

    ctc_usw_dkits_dump_diff_status(lchip, 1, NULL, NULL);
    sal_time(&begine_time);
    ctc_usw_dkits_dump_cmp_tbl(lchip, p_srf, p_drf, src_cfg_endian, dst_cfg_endian, p_wf);
    sal_time(&end_time);

    CTC_DKIT_PRINT(" Diff file: %s\n\n", dump_diff_file);
     /*CTC_DKIT_PRINT("Time elapse: %lf(s)\n", interval);*/

CTC_DKITS_DUMP_CFG_CMP_END:

    if (NULL != p_wf)
    {
        sal_fclose(p_wf);
    }
    if (NULL != p_srf)
    {
        sal_fclose(p_srf);
    }
    if (NULL != p_drf)
    {
        sal_fclose(p_drf);
    }

    return ret;
}

int32
ctc_usw_dkits_dump_data2text(uint8 lchip, sal_file_t p_rf, uint8 cfg_endian, sal_file_t p_wf, uint8 data_translate)
{
    uint32 i = 0, eop = 0, tbl_header = 0;
    char* str = NULL;
    char* tbl = NULL;
    char format[100] = {0};
    char format2[100] = {0};
    uint32* data_entry = NULL;
    uint32* mask_entry = NULL;
    uint32* ad_entry = NULL;
    uint32* p_entry_data = NULL;
    uint32 tbl_word = 0, word_num_per_line = 0, word_idx = 0, data_word = 0, mask_word = 0;
    ctc_dkits_dump_tbl_entry_t tbl_entry;
    uint8 header_type = CTC_DKITS_DUMP_TEXT_HEADER_NUM;
    static uint32 pre = 0;
    uint32 cur = 0;
    ctc_dkits_dump_serdes_tbl_t* serdes_tbl = NULL;
    uint32 serdes_id = 0;
    uint32 register_mode = 0;
    uint32 offset = 0;
    uint32 value = 0;
    char* p_mode[CTC_DKIT_SERDES_PLLB+1] = {"Tx", "Rx", "Common", "PLLA", "PLLB"};
    ctc_dkits_dump_key_tbl_info_t tbl_info;
    tbls_id_t tb_id = 0;
    uint32 index = 0;
    int32 ret = 0;
    ctc_dkit_memory_para_t* p_memory_para = NULL;

    str = (char*)mem_malloc(MEM_SYSTEM_MODULE, 300 * sizeof(char));
    tbl = (char*)mem_malloc(MEM_SYSTEM_MODULE, 300 * sizeof(char));
    data_entry = (uint32*)mem_malloc(MEM_SYSTEM_MODULE, MAX_ENTRY_WORD * sizeof(uint32));
    mask_entry = (uint32*)mem_malloc(MEM_SYSTEM_MODULE, MAX_ENTRY_WORD * sizeof(uint32));
    ad_entry = (uint32*)mem_malloc(MEM_SYSTEM_MODULE, MAX_ENTRY_WORD * sizeof(uint32));
    p_memory_para = (ctc_dkit_memory_para_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(ctc_dkit_memory_para_t));

    if (!str || !tbl || !data_entry || !mask_entry || !ad_entry || !p_memory_para)
    {
        ret = CLI_ERROR;
        goto exit;
    }

    sal_memset(str, 0, 300 * sizeof(char));
    sal_memset(tbl, 0, 300 * sizeof(char));
    sal_memset(data_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));
    sal_memset(mask_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));
    sal_memset(ad_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));
    sal_memset(p_memory_para, 0, sizeof(ctc_dkit_memory_para_t));

    tbl_entry.data_entry = data_entry;
    tbl_entry.mask_entry = mask_entry;
    tbl_entry.ad_entry = ad_entry;

    ret =sal_fseek(p_rf, 0, SEEK_END);
    if(0 != ret)
    {
        ret = CLI_ERROR;
        goto exit;
    }
    eop = sal_ftell(p_rf);
    if (eop <= sizeof(ctc_dkits_dump_centec_file_hdr_t))
    {
        goto DATA2TEXT_ERROR;
    }
    ret = sal_fseek(p_rf, sizeof(ctc_dkits_dump_centec_file_hdr_t), SEEK_SET);
    if(0 != ret)
    {
        ret = CLI_ERROR;
        goto exit;
    }

    while (eop != sal_ftell(p_rf) && (1 != sal_feof(p_rf)))
    {
        cur = sal_ftell(p_rf);
        if ((cur - pre) > (eop/10))
        {
            sal_udelay(10);
            pre = cur;
        }

        sal_memset(data_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));
        sal_memset(mask_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));
        sal_memset(ad_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));
        data_word = 0;
        mask_word = 0;

         ctc_dkits_dump_get_tbl_entry(lchip, p_rf, cfg_endian, &tbl_entry, &tbl_info);

        /*Default using key info*/
        tb_id = tbl_info.tblid;
        index = tbl_info.idx;

        switch(tbl_info.tbl_type)
        {
            case CTC_DKITS_DUMP_TBL_TYPE_STATIC:
            case CTC_DKITS_DUMP_TBL_TYPE_HASH_KEY:
            case CTC_DKITS_DUMP_TBL_TYPE_HASH_NO_AD:
                tbl_header = (CTC_DKITS_DUMP_TEXT_HEADER_DUMP_TBL != header_type) ? 0 : 1;
                header_type = CTC_DKITS_DUMP_TEXT_HEADER_DUMP_TBL;
                tbl_word = TABLE_ENTRY_SIZE(lchip, tb_id) / DRV_BYTES_PER_WORD;
                break;

            case CTC_DKITS_DUMP_TBL_TYPE_SERDES:
                tbl_header = (CTC_DKITS_DUMP_TEXT_HEADER_DUMP_SERDES != header_type) ? 0 : 1;
                header_type = CTC_DKITS_DUMP_TEXT_HEADER_DUMP_SERDES;
                tbl_word = 1;
                break;

            case CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY:
                tbl_header = (CTC_DKITS_DUMP_TEXT_HEADER_DUMP_TCAM != header_type) ? 0 : 1;
                header_type = CTC_DKITS_DUMP_TEXT_HEADER_DUMP_TCAM;
                tbl_word = (TABLE_ENTRY_SIZE(lchip, tb_id) / DRV_BYTES_PER_WORD) * 2;
                tbl_word = (tbl_word < DUMP_TCAM_WORD_NUM_PER_LINE) ? DUMP_TCAM_WORD_NUM_PER_LINE : tbl_word;
                break;

            case CTC_DKITS_DUMP_TBL_TYPE_TCAM_AD:
                tbl_header = (CTC_DKITS_DUMP_TEXT_HEADER_DUMP_TCAM != header_type) ? 0 : 1;
                header_type = CTC_DKITS_DUMP_TEXT_HEADER_DUMP_TCAM;
                tbl_word = (TABLE_ENTRY_SIZE(lchip, tbl_info.ad_tblid) / DRV_BYTES_PER_WORD);
                break;

            default:
               break;
        }

        if ((0 == tbl_header) && (!data_translate || (CTC_DKITS_DUMP_TBL_TYPE_SERDES == tbl_info.tbl_type)))
        {
            tbl_header = 1;
            ctc_dkits_dump_txt_header(header_type, p_wf);
        }

        for (i = 0; i < tbl_word; i++)
        {
            if (CTC_DKITS_DUMP_TBL_TYPE_SERDES == tbl_info.tbl_type)
            {
                word_num_per_line = DUMP_TBL_WORD_NUM_PER_LINE;
                sal_sprintf(format2, "%%-%du%%-%ds%%-%du0x%%0%dx", 10, 10, 10, 4);
                serdes_tbl = (ctc_dkits_dump_serdes_tbl_t*)tbl_entry.data_entry;
                serdes_id = serdes_tbl->serdes_id;
                register_mode = serdes_tbl->serdes_mode;
                offset = serdes_tbl->offset;
                value = serdes_tbl->data;

                sal_fprintf(p_wf, format2, serdes_id, ((register_mode<=CTC_DKIT_SERDES_PLLB)?p_mode[register_mode]:""), offset, value);
                continue;
            }
            else if (CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == tbl_info.tbl_type)
            {
                word_num_per_line = DUMP_TCAM_WORD_NUM_PER_LINE;

                if (0 == (i%(DUMP_TCAM_WORD_NUM_PER_LINE/2)))
                {
                    if (0 == ((i/(DUMP_TCAM_WORD_NUM_PER_LINE/2))%2))
                    {
                        word_idx = data_word;
                        data_word += DUMP_TCAM_WORD_NUM_PER_LINE/2;
                        p_entry_data = tbl_entry.data_entry;
                    }
                    else
                    {
                        word_idx = mask_word;
                        mask_word += DUMP_TCAM_WORD_NUM_PER_LINE/2;
                        p_entry_data = tbl_entry.mask_entry;
                    }
                }
                else
                {
                    word_idx++;
                }
            }
            else
            {
                word_idx = i;
                if (CTC_DKITS_DUMP_TBL_TYPE_HASH_AD == tbl_info.tbl_type)
                {
                    word_num_per_line = DUMP_TBL_WORD_NUM_PER_LINE;
                    p_entry_data = tbl_entry.ad_entry;
                    tb_id = tbl_info.ad_tblid;
                    index = tbl_info.ad_idx;
                }
                else if (CTC_DKITS_DUMP_TBL_TYPE_TCAM_AD == tbl_info.tbl_type)
                {
                    word_num_per_line = DUMP_TCAM_WORD_NUM_PER_LINE / 2;
                    p_entry_data = tbl_entry.ad_entry;
                    tb_id = tbl_info.ad_tblid;
                    index = tbl_info.ad_idx;
                }
                else
                {
                    word_num_per_line = DUMP_TBL_WORD_NUM_PER_LINE;
                    p_entry_data = tbl_entry.data_entry;
                }
            }

            if (data_translate)
            {
                break;
            }

            if (0 == (i % word_num_per_line))
            {
                sal_sprintf(format2, "%%-%ds%%-%ds", TBL_NAME_IDX_MAX_PRINT_LEN + COLUNM_SPACE_LEN, DUMP_ENTRY_WORD_STR_LEN + DUMP_ENTRY_WORD_SPACE_LEN);
                if (0 == i)
                {
                    sal_sprintf(tbl, "%s[%u]", TABLE_NAME(lchip, tb_id), index);
                    ctc_dkits_dump_cfg_brief_str(tbl, sal_strlen(TABLE_NAME(lchip, tb_id)), ((int8)(sal_strlen(tbl)) - TBL_NAME_IDX_MAX_PRINT_LEN));
                    sal_fprintf(p_wf, format2, tbl, CTC_DKITS_DUMP_HEX(str, format, p_entry_data[word_idx], 8));
                }
                else
                {
                    sal_fprintf(p_wf, format2, " ", CTC_DKITS_DUMP_HEX(str, format, p_entry_data[word_idx], 8));
                }
            }
            else
            {
                sal_sprintf(format2, "%%-%ds", DUMP_ENTRY_WORD_STR_LEN + DUMP_ENTRY_WORD_SPACE_LEN);
                if (((tbl_word/2) > (TABLE_ENTRY_SIZE(lchip, tb_id)/DRV_BYTES_PER_WORD))
                   && ((i%(word_num_per_line/2)) >= (TABLE_ENTRY_SIZE(lchip, tb_id)/DRV_BYTES_PER_WORD)))
                {
                    sal_fprintf(p_wf, format2, " ", " ");
                }
                else
                {
                    sal_fprintf(p_wf, format2, CTC_DKITS_DUMP_HEX(str, format, p_entry_data[word_idx], 8));
                }
            }
            if (0 == ((i + 1) % word_num_per_line))
            {
                sal_fprintf(p_wf, "%s", "\n");
            }
        }

        if ((0 != i % word_num_per_line) && (!data_translate || (CTC_DKITS_DUMP_TBL_TYPE_SERDES == tbl_info.tbl_type)))
        {
            sal_fprintf(p_wf, "\n");
        }

         if ((data_translate) && (tbl_info.tbl_type != CTC_DKITS_DUMP_TBL_TYPE_SERDES))
        {
            CTC_DKIT_PRINT("%s[%d]:\n", TABLE_NAME(lchip, tb_id), index);
            CTC_DKIT_PRINT("-------------------------------------------------------------\n");
            sal_memset(p_memory_para, 0, sizeof(ctc_dkit_memory_para_t));

            p_memory_para->param[1] = lchip;
            p_memory_para->param[2] = 0;
            p_memory_para->param[3] = tb_id;
            p_memory_para->param[4] = 0;
            if (CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == tbl_info.tbl_type)
            {
                sal_memcpy(p_memory_para->value, tbl_entry.data_entry, CTC_DKITS_MAX_ENTRY_WORD);
                sal_memcpy(p_memory_para->mask, tbl_entry.mask_entry, CTC_DKITS_MAX_ENTRY_WORD);
            }
            else
            {
                sal_memcpy(p_memory_para->value, p_entry_data, CTC_DKITS_MAX_ENTRY_WORD);
            }
            p_memory_para->type = CTC_DKIT_MEMORY_SHOW_TBL_BY_DATA;

            ctc_usw_dkit_memory_process(p_memory_para);
        }



    }

    pre = 0;
DATA2TEXT_ERROR:
exit:
    if (str)
    {
        mem_free(str);
    }
    if (tbl)
    {
        mem_free(tbl);
    }
    if (data_entry)
    {
        mem_free(data_entry);
    }
    if (mask_entry)
    {
        mem_free(mask_entry);
    }
    if (ad_entry)
    {
        mem_free(ad_entry);
    }
    if (p_memory_para)
    {
        mem_free(p_memory_para);
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_dkits_dump_data2tbl(uint8 lchip, ctc_dkits_dump_tbl_type_t tbl_type, uint8 cfg_endian, ctc_dkits_dump_tbl_block_hdr_t* p_tbl_block_hdr, uint8* p_buf, ctc_dkits_dump_tbl_entry_t* p_tbl_entry)
{
    uint8 i = 0, j  = 0, bytes = 0, count = 0;
    ctc_dkits_dump_tbl_block_data_t tbl_block_data;
    uint8 host_endian;
    tbls_id_t tblid = CTC_DKITS_DUMP_BLOCK_HDR_TBLID(p_tbl_block_hdr);

    host_endian = drv_get_host_type(lchip);

    for (i = 0; i < p_tbl_block_hdr->entry_num + 1; i++)
    {
        sal_memset(&tbl_block_data, 0, sizeof(ctc_dkits_dump_tbl_block_data_t));
        bytes = CTC_DKITS_DUMP_BLOCK_DATA_BYTE(p_tbl_block_hdr->format);

        if (CTC_DKITS_FORMAT_TYPE_4BYTE == p_tbl_block_hdr->format)
        {
            sal_memcpy(&tbl_block_data, p_buf + (i * bytes), bytes);
        }
        else
        {
            sal_memcpy(&(tbl_block_data.entry), p_buf + (i * bytes), bytes);
            tbl_block_data.repeat = 1;
        }

        if (cfg_endian != host_endian)
        {
            ctc_dkits_dump_swap32((uint32*)&tbl_block_data.entry, 1);
        }

        for (j = 0; j < tbl_block_data.repeat; j++)
        {
            if ((CTC_DKITS_DUMP_TBL_TYPE_TCAM_KEY == tbl_type)
                && ((count * sizeof(uint32)) >= TABLE_ENTRY_SIZE(lchip, tblid)))
            {
                p_tbl_entry->mask_entry[count - (TABLE_ENTRY_SIZE(lchip, tblid) / sizeof(uint32))] = tbl_block_data.entry;
            }
            else if ((CTC_DKITS_DUMP_TBL_TYPE_HASH_AD == tbl_type)
                    || (CTC_DKITS_DUMP_TBL_TYPE_TCAM_AD == tbl_type))
            {
                p_tbl_entry->ad_entry[count] = tbl_block_data.entry;
            }
            else
            {
                p_tbl_entry->data_entry[count] = tbl_block_data.entry;
            }
            count++;
        }
    }

    return CLI_SUCCESS;
}

