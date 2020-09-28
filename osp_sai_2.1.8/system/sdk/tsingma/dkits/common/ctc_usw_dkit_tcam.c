#include "usw/include/drv_common.h"
#include "usw/include/drv_io.h"
#include "drv_api.h"
#include "ctc_cli.h"
#include "ctc_dkit.h"
#include "usw/include/drv_ftm.h"
#include "usw/include/drv_enum.h"
#include "ctc_usw_dkit.h"
#include "ctc_usw_dkit_drv.h"
#include "ctc_usw_dkit_discard.h"
#include "ctc_usw_dkit_discard_type.h"
#include "ctc_usw_dkit_path.h"
#include "ctc_usw_dkit_captured_info.h"
#include "ctc_usw_dkit_tcam.h"
#include "ctc_usw_dkit_dump_tbl.h"


#define DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM                256*4
#define DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM                256*4
#define DRV_LPM_TCAM_KEY2_MAX_ENTRY_NUM                256*4
#define DRV_LPM_TCAM_KEY3_MAX_ENTRY_NUM                256*4
#define DRV_LPM_TCAM_KEY4_MAX_ENTRY_NUM                256*4
#define DRV_LPM_TCAM_KEY5_MAX_ENTRY_NUM                256*4
#define DRV_LPM_TCAM_KEY6_MAX_ENTRY_NUM                256*4
#define DRV_LPM_TCAM_KEY7_MAX_ENTRY_NUM                256*4
#define DRV_LPM_TCAM_KEY8_MAX_ENTRY_NUM                512*4
#define DRV_LPM_TCAM_KEY9_MAX_ENTRY_NUM                512*4
#define DRV_LPM_TCAM_KEY10_MAX_ENTRY_NUM               512*4
#define DRV_LPM_TCAM_KEY11_MAX_ENTRY_NUM               512*4

#define DS_QUEUE_MAP_TCAM_KEY_BYTES                    (DRV_IS_DUET2(lchip) ? 8 : sizeof(DsQueueMapTcamKey_m))


#define CTC_DKITS_INVALID_INDEX             0xFFFFFFFF
#define CTC_DKITS_INVALID_TBLID             0xFFFFFFFF
#define CTC_DKITS_FLOW_TCAM_BLK_NUM         17
#define CTC_DKITS_LPM_TCAM_BLK_NUM          12
#define CTC_DKITS_OCTO_BITS                  8

#define DKITS_TCAM_FLOW_TCAM_BLK_BITS            5
#define DKITS_TCAM_IP_TCAM_BLK_BITS              2
#define DKITS_TCAM_NAT_TCAM_BLK_BITS             2

#define DKITS_TCAM_FLOW_DETECT_MEM_NUM           1
#define DKITS_TCAM_DETECT_MEM_NUM                1
#define DKITS_TCAM_BLOCK_INVALID_TYPE            0xFF
#define DRV_TIME_OUT                             1000    /* Time out setting */
#define CTC_DKITS_TCAM_TYPE_FLAG_NUM             7
#define CTC_DKITS_TCAM_DETECT_MAX_REQ            8

#define CTC_DKITS_TCAM_FLOW_TCAM_DETECT_BIST_DELAY       2
#define CTC_DKITS_TCAM_FLOW_TCAM_DETECT_CAPTURE_DELAY    1

#define CTC_DKITS_TCAM_LPM_TCAM_DETECT_BIST_DELAY        4
#define CTC_DKITS_TCAM_LPM_TCAM_DETECT_CAPTURE_DELAY     4

#define CTC_DKITS_TCAM_CID_TCAM_DETECT_BIST_DELAY        1
#define CTC_DKITS_TCAM_CID_TCAM_DETECT_CAPTURE_DELAY     2

#define CTC_DKITS_TCAM_QUE_TCAM_DETECT_BIST_DELAY        1
#define CTC_DKITS_TCAM_QUE_TCAM_DETECT_CAPTURE_DELAY     2

#define CTC_DKITS_TCAM_FLOW_KEY_UNIT             10      /* word */
#define CTC_DKITS_TCAM_UINT32_BITS               32

#define CTC_DKITS_TCAM_SCL_KEY_DRV_TYPE_L2KEY     0x0
#define CTC_DKITS_TCAM_SCL_KEY_DRV_TYPE_L3KEY     0x1
#define CTC_DKITS_TCAM_SCL_KEY_DRV_TYPE_L2L3KEY   0x2
#define CTC_DKITS_TCAM_SCL_KEY_DRV_TYPE_USERID    0x3

#define CTC_DKITS_TCAM_FLOW_TCAM_LOCK(lchip)         sal_mutex_lock(p_drv_master[lchip]->p_flow_tcam_mutex)
#define CTC_DKITS_TCAM_FLOW_TCAM_UNLOCK(lchip)       sal_mutex_unlock(p_drv_master[lchip]->p_flow_tcam_mutex)
#define CTC_DKITS_TCAM_LPM_IP_TCAM_LOCK(lchip)       sal_mutex_lock(p_drv_master[lchip]->p_lpm_ip_tcam_mutex)
#define CTC_DKITS_TCAM_LPM_IP_TCAM_UNLOCK(lchip)     sal_mutex_unlock(p_drv_master[lchip]->p_lpm_ip_tcam_mutex)
#define CTC_DKITS_TCAM_LPM_NAT_TCAM_LOCK(lchip)      sal_mutex_lock(p_drv_master[lchip]->p_lpm_nat_tcam_mutex)
#define CTC_DKITS_TCAM_LPM_NAT_TCAM_UNLOCK(lchip)    sal_mutex_unlock(p_drv_master[lchip]->p_lpm_nat_tcam_mutex)

#define CTC_DKITS_TCAM_SCL_TCAM_BLOCK(block)         ((DKITS_TCAM_BLOCK_TYPE_IGS_SCL0 == block)    \
                                                     || (DKITS_TCAM_BLOCK_TYPE_IGS_SCL1 == block)  \
                                                     || (DKITS_TCAM_BLOCK_TYPE_IGS_SCL2 == block)  \
                                                     || (DKITS_TCAM_BLOCK_TYPE_IGS_SCL3 == block))

#define CTC_DKITS_TCAM_IGS_ACL_TCAM_BLOCK(block)     ((DKITS_TCAM_BLOCK_TYPE_IGS_ACL0 == block)    \
                                                     || (DKITS_TCAM_BLOCK_TYPE_IGS_ACL1 == block)  \
                                                     || (DKITS_TCAM_BLOCK_TYPE_IGS_ACL2 == block)  \
                                                     || (DKITS_TCAM_BLOCK_TYPE_IGS_ACL3 == block)  \
                                                     || (DKITS_TCAM_BLOCK_TYPE_IGS_ACL3 == block)  \
                                                     || (DKITS_TCAM_BLOCK_TYPE_IGS_ACL4 == block)  \
                                                     || (DKITS_TCAM_BLOCK_TYPE_IGS_ACL5 == block)  \
                                                     || (DKITS_TCAM_BLOCK_TYPE_IGS_ACL6 == block)  \
                                                     || (DKITS_TCAM_BLOCK_TYPE_IGS_ACL7 == block))

#define CTC_DKITS_TCAM_EGS_ACL_TCAM_BLOCK(block)     ((DKITS_TCAM_BLOCK_TYPE_EGS_ACL0 == block)    \
                                                     || (DKITS_TCAM_BLOCK_TYPE_EGS_ACL1 == block)  \
                                                     || (DKITS_TCAM_BLOCK_TYPE_EGS_ACL2 == block))

#define CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block)        (CTC_DKITS_TCAM_SCL_TCAM_BLOCK(block)         \
                                                     || CTC_DKITS_TCAM_IGS_ACL_TCAM_BLOCK(block)   \
                                                     || CTC_DKITS_TCAM_EGS_ACL_TCAM_BLOCK(block))

#define CTC_DKITS_TCAM_IP_TCAM_BLOCK(block)          ((DKITS_TCAM_BLOCK_TYPE_LPM_LKP0 == block)      \
                                                     || (DKITS_TCAM_BLOCK_TYPE_LPM_LKP1 == block))

#define CTC_DKITS_TCAM_IS_LPM_TCAM0(tcam_blk)          ((tcam_blk >= 0) && (tcam_blk <=3))
#define CTC_DKITS_TCAM_IS_LPM_TCAM1(tcam_blk)          ((tcam_blk >= 4) && (tcam_blk <=7))
#define CTC_DKITS_TCAM_IS_LPM_TCAM2(tcam_blk)          ((tcam_blk >= 8) && (tcam_blk <=11))

#define CTC_DKITS_TCAM_NAT_TCAM_BLOCK(block)         (DKITS_TCAM_BLOCK_TYPE_NAT_PBR == block)

#define CTC_DKITS_TCAM_CID_TCAM_BLOCK(block)         (DKITS_TCAM_BLOCK_TYPE_CATEGORYID == block)

#define CTC_DKITS_TCAM_QUE_TCAM_BLOCK(block)         (DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE == block)

#define DKITS_TCAM_FLOW_RLT_INDEX(tcamid, index)     ((tcamid << (16 - DKITS_TCAM_FLOW_TCAM_BLK_BITS)) | index)
#define DKITS_TCAM_FLOW_RLT_TCAMID(index)            ((index >> (16 - DKITS_TCAM_FLOW_TCAM_BLK_BITS)) & ((1U << DKITS_TCAM_FLOW_TCAM_BLK_BITS) - 1))
#define DKITS_TCAM_FLOW_RLT_TCAMIDX(index)           (index & ((1U << (16 - DKITS_TCAM_FLOW_TCAM_BLK_BITS)) - 1))

#define DKITS_TCAM_IP_RLT_INDEX(tcamid, index)       ((tcamid << (15 - DKITS_TCAM_IP_TCAM_BLK_BITS)) | index)
#define DKITS_TCAM_IP_RLT_TCAMID(index)              ((index >> (15 - DKITS_TCAM_IP_TCAM_BLK_BITS)) & ((1U << DKITS_TCAM_IP_TCAM_BLK_BITS) - 1))
#define DKITS_TCAM_IP_RLT_TCAMIDX(index)             (index & ((1U << (15 - DKITS_TCAM_IP_TCAM_BLK_BITS)) - 1))

#define DKITS_TCAM_NAT_RLT_INDEX(tcamid, index)      ((tcamid << (14 - DKITS_TCAM_NAT_TCAM_BLK_BITS)) | index)
#define DKITS_TCAM_NAT_RLT_TCAMID(index)             ((index >> (14 - DKITS_TCAM_NAT_TCAM_BLK_BITS)) & ((1U << DKITS_TCAM_NAT_TCAM_BLK_BITS) - 1))
#define DKITS_TCAM_NAT_RLT_TCAMIDX(index)            (index & ((1U << (14 - DKITS_TCAM_NAT_TCAM_BLK_BITS)) - 1))

#define DKITS_TCAM_IP_NAT_TCAM(tblid)                ((EXT_INFO_TYPE_LPM_TCAM_IP == TABLE_EXT_INFO_TYPE(lchip, tblid))     \
                                                     || (EXT_INFO_TYPE_LPM_TCAM_NAT == TABLE_EXT_INFO_TYPE(lchip, tblid))  \
                                                     || (EXT_INFO_TYPE_TCAM_LPM_AD == TABLE_EXT_INFO_TYPE(lchip, tblid))   \
                                                     || (EXT_INFO_TYPE_TCAM_NAT_AD == TABLE_EXT_INFO_TYPE(lchip, tblid)))

#define CTC_DKITS_TCAM_ERROR_RETURN(op) \
    { \
        int32 rv = (op); \
        if ((rv < 0) || (1 == rv)) \
        { \
            CTC_DKIT_PRINT(" Dkits Error!! Fun:%s()  Line:%d ret:%d\n",__FUNCTION__,__LINE__, rv); \
            return (CLI_ERROR); \
        } \
    }

struct dkits_tcam_tbl_info_s
{
    uint32  id           :16;
    uint32  idx          :14;
    uint32  result_valid :1;
    uint32  lookup_valid :1;
};
typedef struct dkits_tcam_tbl_info_s dkits_tcam_tbl_info_t;

struct dkits_tcam_capture_info_s
{
    dkits_tcam_tbl_info_t tbl[CTC_DKITS_TCAM_DETECT_MAX_REQ];
};
typedef struct dkits_tcam_capture_info_s dkits_tcam_capture_info_t;

struct ds_12word_key_s
{
    uint32 field[DRV_WORDS_PER_ENTRY * 20];
};
typedef struct ds_12word_key_s ds_12word_key_t;

#if (HOST_IS_LE == 1)
struct dkits_tcam_cid_req_s
{
    uint32 dest_category_id_classfied   :1;
    uint32 src_category_id_classfied    :1;
    uint32 src_category_id              :8;
    uint32 dest_category_id             :8;
    uint32 rsv0                         :2;
    uint32 seq                          :8;
    uint32 key_size                     :2;
    uint32 rsv1                         :1;
    uint32 valid                        :1;
};
#else
struct dkits_tcam_cid_req_s
{
    uint32 valid                        :1;
    uint32 rsv1                         :1;
    uint32 key_size                     :2;
    uint32 seq                          :8;
    uint32 rsv0                         :2;
    uint32 dest_category_id             :8;
    uint32 src_category_id              :8;
    uint32 src_category_id_classfied    :1;
    uint32 dest_category_id_classfied   :1;
};
#endif
typedef struct dkits_tcam_cid_req_s dkits_tcam_cid_req_t;

enum dkits_tcam_scl_key_size_type_e
{
    DKITS_TCAM_SCL_KEY_SIZE_TYPE_3W,
    DKITS_TCAM_SCL_KEY_SIZE_TYPE_6W,
    DKITS_TCAM_SCL_KEY_SIZE_TYPE_12W,
    DKITS_TCAM_SCL_KEY_SIZE_TYPE_24W,
    DKITS_TCAM_SCL_KEY_SIZE_TYPE_NUM,
    DKITS_TCAM_SCL_KEY_SIZE_TYPE_INVALID = DKITS_TCAM_SCL_KEY_SIZE_TYPE_NUM
};
typedef enum dkits_tcam_scl_key_size_type_e dkits_tcam_scl_key_size_type_t;

enum dkits_flow_tcam_lookup_type_e
{
    DKITS_FLOW_TCAM_LOOKUP_TYPE_L2KEY,
    DKITS_FLOW_TCAM_LOOKUP_TYPE_L2L3KEY,
    DKITS_FLOW_TCAM_LOOKUP_TYPE_L3KEY,
    DKITS_FLOW_TCAM_LOOKUP_TYPE_VLANKEY
};
typedef enum dkits_flow_tcam_lookup_type_e dkits_flow_tcam_lookup_type_t;

enum dkits_lpm_tcam_key_size_type_e
{
    DKITS_LPM_TCAM_KEY_SIZE_TYPE_HALF,
    DKITS_LPM_TCAM_KEY_SIZE_TYPE_SINGLE,
    DKITS_LPM_TCAM_KEY_SIZE_TYPE_DOUBLE,
    DKITS_LPM_TCAM_KEY_SIZE_TYPE_NUM
};
typedef enum dkits_lpm_tcam_key_size_type_e dkits_lpm_tcam_key_size_type_t;

enum dkits_lpm_ip_key_type_e
{
    DKITS_LPM_IP_KEY_TYPE_IPV4,
    DKITS_LPM_IP_KEY_TYPE_IPV6,
    DKITS_LPM_IP_KEY_TYPE_NUM
};
typedef enum dkits_lpm_ip_key_type_e dkits_lpm_ip_key_type_t;

enum dkits_lpm_nat_key_type_e
{
    DKITS_LPM_NAT_KEY_TYPE_IPV4PBR,
    DKITS_LPM_NAT_KEY_TYPE_IPV6PBR,
    DKITS_LPM_NAT_KEY_TYPE_IPV4NAT,
    DKITS_LPM_NAT_KEY_TYPE_IPV6NAT,
    DKITS_LPM_NAT_KEY_TYPE_NUM
};
typedef enum dkits_lpm_nat_key_type_e dkits_lpm_nat_key_type_t;

enum dkits_lpm_tcam_module_e
{
    DKITS_LPM_TCAM_MODULE_LKPUP0,
    DKITS_LPM_TCAM_MODULE_LKPUP1,
    DKITS_LPM_TCAM_MODULE_NATPBR,
    DKITS_LPM_TCAM_MODULE_NUM
};
typedef enum dkits_lpm_tcam_module_e dkits_lpm_tcam_module_t;

enum dkits_tcam_capture_tick_e
{
    DKITS_TCAM_CAPTURE_TICK_BEGINE,
    DKITS_TCAM_CAPTURE_TICK_TERMINATE,
    DKITS_TCAM_CAPTURE_TICK_LASTEST,
    DKITS_TCAM_CAPTURE_TICK_NUM
};
typedef enum dkits_tcam_capture_tick_e dkits_tcam_capture_tick_t;

enum dkits_tcam_detect_property_e
{
    DKITS_TCAM_DETECT_BIST_EN,
    DKITS_TCAM_DETECT_BIST_ONCE,
    DKITS_TCAM_DETECT_CAPTURE_EN,
    DKITS_TCAM_DETECT_CAPTURE_ONCE,
    DKITS_TCAM_DETECT_DELAY,
    DKITS_TCAM_DETECT_NUM,
    DKITS_TCAM_DETECT_CAPTURE_VEC = DKITS_TCAM_DETECT_CAPTURE_EN,
    DKITS_TCAM_DETECT_BIST_VEC = DKITS_TCAM_DETECT_CAPTURE_EN
};
typedef enum dkits_tcam_detect_property_e dkits_tcam_detect_property_t;

enum dkits_tcam_detect_type_e
{
    DKITS_TCAM_DETECT_TYPE_BIST,
    DKITS_TCAM_DETECT_TYPE_CAPTURE,
    DKITS_TCAM_DETECT_TYPE_NUM,
    DKITS_TCAM_DETECT_TYPE_INVALID
};
typedef enum dkits_tcam_detect_type_e dkits_tcam_detect_type_t;

char* tcam_block_desc[] = {"Ingress SCL0",           "Ingress SCL1",
                           "Ingress SCL2",           "Ingress SCL3",
                           "Ingress ACL0",           "Ingress ACL1",
                           "Ingress ACL2",           "Ingress ACL3",
                           "Ingress ACL4",           "Ingress ACL5",
                           "Ingress ACL6",           "Ingress ACL7",
                           "Egress ACL0",            "Egress ACL1",
                           "Egress ACL2",            "IP LPM First Lookup",
                           "IP LPM Second Lookup",   "IP PBR/NAT",
                           "CategoryID Lookup",      "Service QueueMap"};

static tbls_id_t tcam_mem[DKITS_TCAM_BLOCK_TYPE_NUM][DKITS_TCAM_PER_BLOCK_MAX_KEY_TYPE] =
{
    {DsUserId0TcamKey80_t,     DsScl0L3Key160_t, DsScl0Ipv6Key320_t, DsScl0MacIpv6Key640_t,    MaxTblId_t},
    {DsUserId1TcamKey80_t,     DsScl1L3Key160_t, DsScl1Ipv6Key320_t, DsScl1MacIpv6Key640_t,    MaxTblId_t},
    {DsScl2L3Key160_t,        DsScl2Ipv6Key320_t, DsScl2MacIpv6Key640_t,    MaxTblId_t},
    {DsScl1L3Key160_t,        DsScl3Ipv6Key320_t, DsScl3MacIpv6Key640_t,    MaxTblId_t},

    {DsAclQosKey80Ing0_t, DsAclQosL3Key160Ing0_t,   DsAclQosL3Key320Ing0_t,  DsAclQosIpv6Key640Ing0_t,  MaxTblId_t},
    {DsAclQosKey80Ing1_t, DsAclQosL3Key160Ing1_t,   DsAclQosL3Key320Ing1_t,  DsAclQosIpv6Key640Ing1_t,  MaxTblId_t},
    {DsAclQosKey80Ing2_t, DsAclQosL3Key160Ing2_t,   DsAclQosL3Key320Ing2_t,  DsAclQosIpv6Key640Ing2_t,  MaxTblId_t},
    {DsAclQosKey80Ing3_t, DsAclQosL3Key160Ing3_t,   DsAclQosL3Key320Ing3_t,  DsAclQosIpv6Key640Ing3_t,  MaxTblId_t},
    {DsAclQosKey80Ing4_t, DsAclQosL3Key160Ing4_t,   DsAclQosL3Key320Ing4_t,  DsAclQosIpv6Key640Ing4_t,  MaxTblId_t},
    {DsAclQosKey80Ing5_t, DsAclQosL3Key160Ing5_t,   DsAclQosL3Key320Ing5_t,  DsAclQosIpv6Key640Ing5_t,  MaxTblId_t},
    {DsAclQosKey80Ing6_t, DsAclQosL3Key160Ing6_t,   DsAclQosL3Key320Ing6_t,  DsAclQosIpv6Key640Ing6_t,  MaxTblId_t},
    {DsAclQosKey80Ing7_t, DsAclQosL3Key160Ing7_t,   DsAclQosL3Key320Ing7_t,  DsAclQosIpv6Key640Ing7_t,  MaxTblId_t},

    {DsAclQosKey80Egr0_t,      DsAclQosL3Key160Egr0_t,  DsAclQosL3Key320Egr0_t,
     DsAclQosIpv6Key640Egr0_t, MaxTblId_t},

    {DsAclQosKey80Egr1_t,      DsAclQosL3Key160Egr1_t,  DsAclQosL3Key320Egr1_t,
     DsAclQosIpv6Key640Egr1_t, MaxTblId_t},

    {DsAclQosKey80Egr2_t,      DsAclQosL3Key160Egr2_t,  DsAclQosL3Key320Egr2_t,
     DsAclQosIpv6Key640Egr2_t, MaxTblId_t},

    {DsLpmTcamIpv4HalfKey_t,
     DsLpmTcamIpv6DoubleKey0_t,
     DsLpmTcamIpv4NatDoubleKey_t,
     DsLpmTcamIpv6DoubleKey0_t,
     MaxTblId_t},

    {CTC_DKITS_INVALID_TBLID, MaxTblId_t},

    {CTC_DKITS_INVALID_TBLID, CTC_DKITS_INVALID_TBLID, MaxTblId_t},

    {DsCategoryIdPairTcamKey_t,           MaxTblId_t},
    {DsQueueMapTcamKey_t,                 MaxTblId_t}
};

ctc_dkits_tcam_tbl_t ctc_dkits_tcam_tbl[DKITS_TCAM_BLOCK_TYPE_NUM][DKITS_TCAM_PER_BLOCK_MAX_KEY_TYPE] =
{
    {{DsUserId0TcamKey80_t,                  DKITS_TCAM_SCL_KEY_USERID_3W,    0},
     {DsUserId0TcamKey160_t,                 DKITS_TCAM_SCL_KEY_USERID_6W,    0},
     {DsUserId0TcamKey320_t,                 DKITS_TCAM_SCL_KEY_USERID_12W,   0},
     {DsScl0MacKey160_t,                     DKITS_TCAM_SCL_KEY_L2,           0},
     {DsScl0L3Key160_t,                      DKITS_TCAM_SCL_KEY_L3_IPV4,      0},
     {DsScl0Ipv6Key320_t,                    DKITS_TCAM_SCL_KEY_L3_IPV6,      0},
     {DsScl0MacL3Key320_t,                   DKITS_TCAM_SCL_KEY_L2L3_IPV4,    0},
     {DsScl0MacIpv6Key640_t,                 DKITS_TCAM_SCL_KEY_L2L3_IPV6,    0},
     {MaxTblId_t,                            DKITS_TCAM_SCL_KEY_NUM,          0}},

    {{DsUserId1TcamKey80_t,                  DKITS_TCAM_SCL_KEY_USERID_3W,    0},
     {DsUserId1TcamKey160_t,                 DKITS_TCAM_SCL_KEY_USERID_6W,    0},
     {DsUserId1TcamKey320_t,                 DKITS_TCAM_SCL_KEY_USERID_12W,   0},
     {DsScl1MacKey160_t,                     DKITS_TCAM_SCL_KEY_L2,           0},
     {DsScl1L3Key160_t,                      DKITS_TCAM_SCL_KEY_L3_IPV4,      0},
     {DsScl1Ipv6Key320_t,                    DKITS_TCAM_SCL_KEY_L3_IPV6,      0},
     {DsScl1MacL3Key320_t,                   DKITS_TCAM_SCL_KEY_L2L3_IPV4,    0},
     {DsScl1MacIpv6Key640_t,                 DKITS_TCAM_SCL_KEY_L2L3_IPV6,    0},
     {MaxTblId_t,                            DKITS_TCAM_SCL_KEY_NUM,          0}},

    {{DsScl2MacKey160_t,                     DKITS_TCAM_SCL_KEY_L2,           0},
     {DsScl2L3Key160_t,                      DKITS_TCAM_SCL_KEY_L3_IPV4,      0},
     {DsScl2Ipv6Key320_t,                    DKITS_TCAM_SCL_KEY_L3_IPV6,      0},
     {DsScl2MacL3Key320_t,                   DKITS_TCAM_SCL_KEY_L2L3_IPV4,    0},
     {DsScl2MacIpv6Key640_t,                 DKITS_TCAM_SCL_KEY_L2L3_IPV6,    0},
     {MaxTblId_t,                            DKITS_TCAM_SCL_KEY_NUM,          0}},

    {{DsScl3L3Key160_t,                      DKITS_TCAM_SCL_KEY_L3_IPV4,      0},
     {DsScl3Ipv6Key320_t,                    DKITS_TCAM_SCL_KEY_L3_IPV6,      0},
     {DsScl3MacL3Key320_t,                   DKITS_TCAM_SCL_KEY_L2L3_IPV4,    0},
     {DsScl3MacIpv6Key640_t,                 DKITS_TCAM_SCL_KEY_L2L3_IPV6,    0},
     {MaxTblId_t,                            DKITS_TCAM_SCL_KEY_NUM,          0}},

    {{DsAclQosMacKey160Ing0_t,               DKITS_TCAM_ACL_KEY_MAC160,       0},
     {DsAclQosL3Key160Ing0_t,                DKITS_TCAM_ACL_KEY_L3160,        0},
     {DsAclQosL3Key320Ing0_t,                DKITS_TCAM_ACL_KEY_L3320,        0},
     {DsAclQosIpv6Key320Ing0_t,              DKITS_TCAM_ACL_KEY_IPV6320,      0},
     {DsAclQosIpv6Key640Ing0_t,              DKITS_TCAM_ACL_KEY_IPV6640,      0},
     {DsAclQosMacL3Key320Ing0_t,             DKITS_TCAM_ACL_KEY_MACL3320,     0},
     {DsAclQosMacL3Key640Ing0_t,             DKITS_TCAM_ACL_KEY_MACL3640,     0},
     {DsAclQosMacIpv6Key640Ing0_t,           DKITS_TCAM_ACL_KEY_MACIPV6640,   0},
     {DsAclQosCidKey160Ing0_t,               DKITS_TCAM_ACL_KEY_CID160,       0},
     {DsAclQosKey80Ing0_t,                   DKITS_TCAM_ACL_KEY_SHORT80,      0},
     {DsAclQosForwardKey320Ing0_t,           DKITS_TCAM_ACL_KEY_FORWARD320,   0},
     {DsAclQosForwardKey640Ing0_t,           DKITS_TCAM_ACL_KEY_FORWARD640,   0},
     {DsAclQosCoppKey320Ing0_t,              DKITS_TCAM_ACL_KEY_COPP320,      0},
     {DsAclQosCoppKey640Ing0_t,              DKITS_TCAM_ACL_KEY_COPP640,      0},
     {MaxTblId_t,                            DKITS_TCAM_ACL_KEY_NUM,          0}},

    {{DsAclQosMacKey160Ing1_t,               DKITS_TCAM_ACL_KEY_MAC160,       0},
     {DsAclQosL3Key160Ing1_t,                DKITS_TCAM_ACL_KEY_L3160,        0},
     {DsAclQosL3Key320Ing1_t,                DKITS_TCAM_ACL_KEY_L3320,        0},
     {DsAclQosIpv6Key320Ing1_t,              DKITS_TCAM_ACL_KEY_IPV6320,      0},
     {DsAclQosIpv6Key640Ing1_t,              DKITS_TCAM_ACL_KEY_IPV6640,      0},
     {DsAclQosMacL3Key320Ing1_t,             DKITS_TCAM_ACL_KEY_MACL3320,     0},
     {DsAclQosMacL3Key640Ing1_t,             DKITS_TCAM_ACL_KEY_MACL3640,     0},
     {DsAclQosMacIpv6Key640Ing1_t,           DKITS_TCAM_ACL_KEY_MACIPV6640,   0},
     {DsAclQosCidKey160Ing1_t,               DKITS_TCAM_ACL_KEY_CID160,       0},
     {DsAclQosKey80Ing1_t,                   DKITS_TCAM_ACL_KEY_SHORT80,      0},
     {DsAclQosForwardKey320Ing1_t,           DKITS_TCAM_ACL_KEY_FORWARD320,   0},
     {DsAclQosForwardKey640Ing1_t,           DKITS_TCAM_ACL_KEY_FORWARD640,   0},
     {DsAclQosCoppKey320Ing1_t,              DKITS_TCAM_ACL_KEY_COPP320,      0},
     {DsAclQosCoppKey640Ing1_t,              DKITS_TCAM_ACL_KEY_COPP640,      0},
     {MaxTblId_t,                            DKITS_TCAM_ACL_KEY_NUM,          0}},

    {{DsAclQosMacKey160Ing2_t,               DKITS_TCAM_ACL_KEY_MAC160,       0},
     {DsAclQosL3Key160Ing2_t,                DKITS_TCAM_ACL_KEY_L3160,        0},
     {DsAclQosL3Key320Ing2_t,                DKITS_TCAM_ACL_KEY_L3320,        0},
     {DsAclQosIpv6Key320Ing2_t,              DKITS_TCAM_ACL_KEY_IPV6320,      0},
     {DsAclQosIpv6Key640Ing2_t,              DKITS_TCAM_ACL_KEY_IPV6640,      0},
     {DsAclQosMacL3Key320Ing2_t,             DKITS_TCAM_ACL_KEY_MACL3320,     0},
     {DsAclQosMacL3Key640Ing2_t,             DKITS_TCAM_ACL_KEY_MACL3640,     0},
     {DsAclQosMacIpv6Key640Ing2_t,           DKITS_TCAM_ACL_KEY_MACIPV6640,   0},
     {DsAclQosCidKey160Ing2_t,               DKITS_TCAM_ACL_KEY_CID160,       0},
     {DsAclQosKey80Ing2_t,                   DKITS_TCAM_ACL_KEY_SHORT80,      0},
     {DsAclQosForwardKey320Ing2_t,           DKITS_TCAM_ACL_KEY_FORWARD320,   0},
     {DsAclQosForwardKey640Ing2_t,           DKITS_TCAM_ACL_KEY_FORWARD640,   0},
     {DsAclQosCoppKey320Ing2_t,              DKITS_TCAM_ACL_KEY_COPP320,      0},
     {DsAclQosCoppKey640Ing2_t,              DKITS_TCAM_ACL_KEY_COPP640,      0},
     {MaxTblId_t,                            DKITS_TCAM_ACL_KEY_NUM,          0}},

    {{DsAclQosMacKey160Ing3_t,               DKITS_TCAM_ACL_KEY_MAC160,       0},
     {DsAclQosL3Key160Ing3_t,                DKITS_TCAM_ACL_KEY_L3160,        0},
     {DsAclQosL3Key320Ing3_t,                DKITS_TCAM_ACL_KEY_L3320,        0},
     {DsAclQosIpv6Key320Ing3_t,              DKITS_TCAM_ACL_KEY_IPV6320,      0},
     {DsAclQosIpv6Key640Ing3_t,              DKITS_TCAM_ACL_KEY_IPV6640,      0},
     {DsAclQosMacL3Key320Ing3_t,             DKITS_TCAM_ACL_KEY_MACL3320,     0},
     {DsAclQosMacL3Key640Ing3_t,             DKITS_TCAM_ACL_KEY_MACL3640,     0},
     {DsAclQosMacIpv6Key640Ing3_t,           DKITS_TCAM_ACL_KEY_MACIPV6640,   0},
     {DsAclQosCidKey160Ing3_t,               DKITS_TCAM_ACL_KEY_CID160,       0},
     {DsAclQosKey80Ing3_t,                   DKITS_TCAM_ACL_KEY_SHORT80,      0},
     {DsAclQosForwardKey320Ing3_t,           DKITS_TCAM_ACL_KEY_FORWARD320,   0},
     {DsAclQosForwardKey640Ing3_t,           DKITS_TCAM_ACL_KEY_FORWARD640,   0},
     {DsAclQosCoppKey320Ing3_t,              DKITS_TCAM_ACL_KEY_COPP320,      0},
     {DsAclQosCoppKey640Ing3_t,              DKITS_TCAM_ACL_KEY_COPP640,      0},
     {MaxTblId_t,                            DKITS_TCAM_ACL_KEY_NUM,          0}},

    {{DsAclQosMacKey160Ing4_t,               DKITS_TCAM_ACL_KEY_MAC160,       0},
     {DsAclQosL3Key160Ing4_t,                DKITS_TCAM_ACL_KEY_L3160,        0},
     {DsAclQosL3Key320Ing4_t,                DKITS_TCAM_ACL_KEY_L3320,        0},
     {DsAclQosIpv6Key320Ing4_t,              DKITS_TCAM_ACL_KEY_IPV6320,      0},
     {DsAclQosIpv6Key640Ing4_t,              DKITS_TCAM_ACL_KEY_IPV6640,      0},
     {DsAclQosMacL3Key320Ing4_t,             DKITS_TCAM_ACL_KEY_MACL3320,     0},
     {DsAclQosMacL3Key640Ing4_t,             DKITS_TCAM_ACL_KEY_MACL3640,     0},
     {DsAclQosMacIpv6Key640Ing4_t,           DKITS_TCAM_ACL_KEY_MACIPV6640,   0},
     {DsAclQosCidKey160Ing4_t,               DKITS_TCAM_ACL_KEY_CID160,       0},
     {DsAclQosKey80Ing4_t,                   DKITS_TCAM_ACL_KEY_SHORT80,      0},
     {DsAclQosForwardKey320Ing4_t,           DKITS_TCAM_ACL_KEY_FORWARD320,   0},
     {DsAclQosForwardKey640Ing4_t,           DKITS_TCAM_ACL_KEY_FORWARD640,   0},
     {DsAclQosCoppKey320Ing4_t,              DKITS_TCAM_ACL_KEY_COPP320,      0},
     {DsAclQosCoppKey640Ing4_t,              DKITS_TCAM_ACL_KEY_COPP640,      0},
     {MaxTblId_t,                            DKITS_TCAM_ACL_KEY_NUM,          0}},

    {{DsAclQosMacKey160Ing5_t,               DKITS_TCAM_ACL_KEY_MAC160,       0},
     {DsAclQosL3Key160Ing5_t,                DKITS_TCAM_ACL_KEY_L3160,        0},
     {DsAclQosL3Key320Ing5_t,                DKITS_TCAM_ACL_KEY_L3320,        0},
     {DsAclQosIpv6Key320Ing5_t,              DKITS_TCAM_ACL_KEY_IPV6320,      0},
     {DsAclQosIpv6Key640Ing5_t,              DKITS_TCAM_ACL_KEY_IPV6640,      0},
     {DsAclQosMacL3Key320Ing5_t,             DKITS_TCAM_ACL_KEY_MACL3320,     0},
     {DsAclQosMacL3Key640Ing5_t,             DKITS_TCAM_ACL_KEY_MACL3640,     0},
     {DsAclQosMacIpv6Key640Ing5_t,           DKITS_TCAM_ACL_KEY_MACIPV6640,   0},
     {DsAclQosCidKey160Ing5_t,               DKITS_TCAM_ACL_KEY_CID160,       0},
     {DsAclQosKey80Ing5_t,                   DKITS_TCAM_ACL_KEY_SHORT80,      0},
     {DsAclQosForwardKey320Ing5_t,           DKITS_TCAM_ACL_KEY_FORWARD320,   0},
     {DsAclQosForwardKey640Ing5_t,           DKITS_TCAM_ACL_KEY_FORWARD640,   0},
     {DsAclQosCoppKey320Ing5_t,              DKITS_TCAM_ACL_KEY_COPP320,      0},
     {DsAclQosCoppKey640Ing5_t,              DKITS_TCAM_ACL_KEY_COPP640,      0},
     {MaxTblId_t,                            DKITS_TCAM_ACL_KEY_NUM,          0}},

    {{DsAclQosMacKey160Ing6_t,               DKITS_TCAM_ACL_KEY_MAC160,       0},
     {DsAclQosL3Key160Ing6_t,                DKITS_TCAM_ACL_KEY_L3160,        0},
     {DsAclQosL3Key320Ing6_t,                DKITS_TCAM_ACL_KEY_L3320,        0},
     {DsAclQosIpv6Key320Ing6_t,              DKITS_TCAM_ACL_KEY_IPV6320,      0},
     {DsAclQosIpv6Key640Ing6_t,              DKITS_TCAM_ACL_KEY_IPV6640,      0},
     {DsAclQosMacL3Key320Ing6_t,             DKITS_TCAM_ACL_KEY_MACL3320,     0},
     {DsAclQosMacL3Key640Ing6_t,             DKITS_TCAM_ACL_KEY_MACL3640,     0},
     {DsAclQosMacIpv6Key640Ing6_t,           DKITS_TCAM_ACL_KEY_MACIPV6640,   0},
     {DsAclQosCidKey160Ing6_t,               DKITS_TCAM_ACL_KEY_CID160,       0},
     {DsAclQosKey80Ing6_t,                   DKITS_TCAM_ACL_KEY_SHORT80,      0},
     {DsAclQosForwardKey320Ing6_t,           DKITS_TCAM_ACL_KEY_FORWARD320,   0},
     {DsAclQosForwardKey640Ing6_t,           DKITS_TCAM_ACL_KEY_FORWARD640,   0},
     {DsAclQosCoppKey320Ing6_t,              DKITS_TCAM_ACL_KEY_COPP320,      0},
     {DsAclQosCoppKey640Ing6_t,              DKITS_TCAM_ACL_KEY_COPP640,      0},
     {MaxTblId_t,                            DKITS_TCAM_ACL_KEY_NUM,          0}},

    {{DsAclQosMacKey160Ing7_t,               DKITS_TCAM_ACL_KEY_MAC160,       0},
     {DsAclQosL3Key160Ing7_t,                DKITS_TCAM_ACL_KEY_L3160,        0},
     {DsAclQosL3Key320Ing7_t,                DKITS_TCAM_ACL_KEY_L3320,        0},
     {DsAclQosIpv6Key320Ing7_t,              DKITS_TCAM_ACL_KEY_IPV6320,      0},
     {DsAclQosIpv6Key640Ing7_t,              DKITS_TCAM_ACL_KEY_IPV6640,      0},
     {DsAclQosMacL3Key320Ing7_t,             DKITS_TCAM_ACL_KEY_MACL3320,     0},
     {DsAclQosMacL3Key640Ing7_t,             DKITS_TCAM_ACL_KEY_MACL3640,     0},
     {DsAclQosMacIpv6Key640Ing7_t,           DKITS_TCAM_ACL_KEY_MACIPV6640,   0},
     {DsAclQosCidKey160Ing7_t,               DKITS_TCAM_ACL_KEY_CID160,       0},
     {DsAclQosKey80Ing7_t,                   DKITS_TCAM_ACL_KEY_SHORT80,      0},
     {DsAclQosForwardKey320Ing7_t,           DKITS_TCAM_ACL_KEY_FORWARD320,   0},
     {DsAclQosForwardKey640Ing7_t,           DKITS_TCAM_ACL_KEY_FORWARD640,   0},
     {DsAclQosCoppKey320Ing7_t,              DKITS_TCAM_ACL_KEY_COPP320,      0},
     {DsAclQosCoppKey640Ing7_t,              DKITS_TCAM_ACL_KEY_COPP640,      0},
     {MaxTblId_t,                            DKITS_TCAM_ACL_KEY_NUM,          0}},

    {{DsAclQosMacKey160Egr0_t,               DKITS_TCAM_ACL_KEY_MAC160,       0},
     {DsAclQosL3Key160Egr0_t,                DKITS_TCAM_ACL_KEY_L3160,        0},
     {DsAclQosL3Key320Egr0_t,                DKITS_TCAM_ACL_KEY_L3320,        0},
     {DsAclQosIpv6Key320Egr0_t,              DKITS_TCAM_ACL_KEY_IPV6320,      0},
     {DsAclQosIpv6Key640Egr0_t,              DKITS_TCAM_ACL_KEY_IPV6640,      0},
     {DsAclQosMacL3Key320Egr0_t,             DKITS_TCAM_ACL_KEY_MACL3320,     0},
     {DsAclQosMacL3Key640Egr0_t,             DKITS_TCAM_ACL_KEY_MACL3640,     0},
     {DsAclQosMacIpv6Key640Egr0_t,           DKITS_TCAM_ACL_KEY_MACIPV6640,   0},
     {DsAclQosKey80Egr0_t,                   DKITS_TCAM_ACL_KEY_SHORT80,      0},
     {MaxTblId_t,                            DKITS_TCAM_ACL_KEY_NUM,          0}},

    {{DsAclQosMacKey160Egr1_t,               DKITS_TCAM_ACL_KEY_MAC160,       0},
     {DsAclQosL3Key160Egr1_t,                DKITS_TCAM_ACL_KEY_L3160,        0},
     {DsAclQosL3Key320Egr1_t,                DKITS_TCAM_ACL_KEY_L3320,        0},
     {DsAclQosIpv6Key320Egr1_t,              DKITS_TCAM_ACL_KEY_IPV6320,      0},
     {DsAclQosIpv6Key640Egr1_t,              DKITS_TCAM_ACL_KEY_IPV6640,      0},
     {DsAclQosMacL3Key320Egr1_t,             DKITS_TCAM_ACL_KEY_MACL3320,     0},
     {DsAclQosMacL3Key640Egr1_t,             DKITS_TCAM_ACL_KEY_MACL3640,     0},
     {DsAclQosMacIpv6Key640Egr1_t,           DKITS_TCAM_ACL_KEY_MACIPV6640,   0},
     {DsAclQosKey80Egr1_t,                   DKITS_TCAM_ACL_KEY_SHORT80,      0},
     {MaxTblId_t,                            DKITS_TCAM_ACL_KEY_NUM,          0}},

    {{DsAclQosMacKey160Egr2_t,               DKITS_TCAM_ACL_KEY_MAC160,       0},
     {DsAclQosL3Key160Egr2_t,                DKITS_TCAM_ACL_KEY_L3160,        0},
     {DsAclQosL3Key320Egr2_t,                DKITS_TCAM_ACL_KEY_L3320,        0},
     {DsAclQosIpv6Key320Egr2_t,              DKITS_TCAM_ACL_KEY_IPV6320,      0},
     {DsAclQosIpv6Key640Egr2_t,              DKITS_TCAM_ACL_KEY_IPV6640,      0},
     {DsAclQosMacL3Key320Egr2_t,             DKITS_TCAM_ACL_KEY_MACL3320,     0},
     {DsAclQosMacL3Key640Egr2_t,             DKITS_TCAM_ACL_KEY_MACL3640,     0},
     {DsAclQosMacIpv6Key640Egr2_t,           DKITS_TCAM_ACL_KEY_MACIPV6640,   0},
     {DsAclQosKey80Egr2_t,                   DKITS_TCAM_ACL_KEY_SHORT80,      0},
     {MaxTblId_t,                            DKITS_TCAM_ACL_KEY_NUM,          0}},

    {{DsLpmTcamIpv4DaPubHalfKey_t,           DKITS_TCAM_IP_KEY_IPV4UC,        0},
     {DsLpmTcamIpv6DaPubDoubleKey0_t,        DKITS_TCAM_IP_KEY_IPV6UC,        0},
     {DsLpmTcamIpv6DaPubSingleKey_t,         DKITS_TCAM_IP_KEY_IPV6UC,        0},
     {DsLpmTcamIpv6SaPubSingleKey_t,         DKITS_TCAM_IP_KEY_IPV6UC,        0},
     {MaxTblId_t,                            DKITS_TCAM_ACL_KEY_NUM,          0}},

    {{MaxTblId_t,                            DKITS_TCAM_ACL_KEY_NUM,          0}},

    {{DsLpmTcamIpv4NatDoubleKey_t,           DKITS_TCAM_NATPBR_KEY_IPV4NAT,   0},
     {MaxTblId_t,                            DKITS_TCAM_NATPBR_KEY_NUM,       0}},

    {{DsCategoryIdPairTcamKey_t,             0,                               0},
     {MaxTblId_t,                            0,                               0}},

    {{DsQueueMapTcamKey_t,                   0,                               0},
     {MaxTblId_t,                            0,                               0}}
};



static ds_t capture_ds[CTC_DKITS_TCAM_DETECT_MAX_REQ];

#define ________DKITS_TCAM_INNER_FUNCTION__________

#define __0_CHIP_TCAM__

extern int32
drv_usw_chip_flow_tcam_get_blknum_index(uint8, tbls_id_t, uint32, uint32*, uint32*, uint32*);

extern int32
drv_usw_chip_lpm_tcam_get_blknum_index(uint8, tbls_id_t, uint32, uint32*, uint32*);

STATIC int32
_ctc_usw_dkits_tcam_get_lpm_tcam_blknum_index(uint8 lchip, tbls_id_t tbl_id, uint32 index, uint32 *blknum, uint32 *local_idx)
{
    uint8 addr_offset = 0;
    uint32 blk_id = 0;
    uint32 map_index = 0;

    if ((drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_LPM_TCAM_IP) || (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_LPM_TCAM_NAT))
    {
        for (blk_id = 0; blk_id < MAX_LPM_TCAM_NUM; blk_id++)
        {
            CTC_DKITS_TCAM_ERROR_RETURN(drv_usw_get_tbl_index_base(lchip, tbl_id, index, &addr_offset));

            if (!IS_BIT_SET(TCAM_BITMAP(lchip, tbl_id), blk_id))
            {
                continue;
            }

            if ((index >= TCAM_START_INDEX(lchip, tbl_id, blk_id)) && (index <= TCAM_END_INDEX(lchip, tbl_id, blk_id)))
            {
                map_index = index - TCAM_START_INDEX(lchip, tbl_id, blk_id);

                *blknum = blk_id;
                *local_idx = map_index;

                break;
            }
        }
    }
    else if((drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_TCAM_LPM_AD) || (drv_usw_get_table_type(lchip, tbl_id) == DRV_TABLE_TYPE_TCAM_NAT_AD))
    {
        for (blk_id = 0; blk_id < MAX_LPM_TCAM_NUM; blk_id++)
        {
            CTC_DKITS_TCAM_ERROR_RETURN(drv_usw_get_tbl_index_base(lchip, tbl_id, index, &addr_offset));

            if (!IS_BIT_SET(TCAM_BITMAP(lchip, tbl_id), blk_id))
            {
                continue;
            }

            if ((index >= TCAM_START_INDEX(lchip, tbl_id, blk_id)) && (index <= TCAM_END_INDEX(lchip, tbl_id, blk_id)))
            {
                map_index = index - TCAM_START_INDEX(lchip, tbl_id, blk_id);

                *blknum = blk_id;
                *local_idx = map_index;

                break;
            }
        }
    }
    else
    {
        DRV_DBG_INFO("\nInvalid table id %d when get flow tcam block number and index!\n", tbl_id);
        return DRV_E_INVALID_TBL;
    }

    return DRV_E_NONE;
}

/**
 @brief Real embeded tcam read operation I/O
*/
STATIC int32
_ctc_usw_dkits_tcam_read_enque_tcam_entry(uint8 lchip, uint32 index, uint32* data, uint32* mask)
{
    uint32 qmgr_enq_tcam_mem[Q_MGR_ENQ_TCAM_MEM_BYTES/4] = {0};
    uint32 cmd = 0;
    int32  ret = DRV_E_NONE;
    uint8  chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;
    uint32 entry_valid = 0;

    DRV_PTR_VALID_CHECK(data);

    /* drv_get_lchip_base(&chip_base); */

    CTC_DKITS_TCAM_FLOW_TCAM_LOCK(lchip_offset);

    idx = index;
    cmd = DRV_IOR(QMgrEnqTcamMem_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, qmgr_enq_tcam_mem), p_drv_master[lchip_offset]->p_flow_tcam_mutex);

    CTC_DKITS_TCAM_FLOW_TCAM_UNLOCK(lchip_offset);

    DRV_IOR_FIELD(lchip, QMgrEnqTcamMem_t, QMgrEnqTcamMem_tcamEntryValid_f, &entry_valid, qmgr_enq_tcam_mem);

    if(!entry_valid)
    {
        /* DRV_DBG_INFO("\nEnq Tcam Memory is invalid!\n"); */
        /* return DRV_E_INVALID_TCAM_TYPE; */
    }

    /* Read Tcam MASK field is X, DATA field is Y */
    sal_memcpy((uint8*)mask, (uint8*)qmgr_enq_tcam_mem,DS_QUEUE_MAP_TCAM_KEY_BYTES);
    sal_memcpy((uint8*)data,(uint8*)qmgr_enq_tcam_mem + DS_QUEUE_MAP_TCAM_KEY_BYTES, DS_QUEUE_MAP_TCAM_KEY_BYTES);

    return ret;
}

STATIC int32
_ctc_usw_dkits_tcam_read_cid_tcam_entry(uint8 lchip, uint32 index, uint32* data, uint32* mask)
{
    uint32 ipe_cid_tcam_mem[IPE_CID_TCAM_MEM_BYTES/4] = {0};
    uint32 cmd = 0;
    int32  ret = DRV_E_NONE;
    uint8  chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;
    uint32 entry_valid = 0;

    DRV_PTR_VALID_CHECK(data);

    /* drv_get_lchip_base(&chip_base); */

    CTC_DKITS_TCAM_FLOW_TCAM_LOCK(lchip_offset);

    idx = index;
    cmd = DRV_IOR(IpeCidTcamMem_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, ipe_cid_tcam_mem), p_drv_master[lchip]->p_flow_tcam_mutex);

    CTC_DKITS_TCAM_FLOW_TCAM_UNLOCK(lchip_offset);

    DRV_IOR_FIELD(lchip, IpeCidTcamMem_t, IpeCidTcamMem_tcamEntryValid_f, &entry_valid, ipe_cid_tcam_mem);

    if(!entry_valid)
    {
        /* DRV_DBG_INFO("\nCid Tcam Memory is invalid!\n"); */
        /* return DRV_E_INVALID_TCAM_TYPE; */
    }

     /* Read Tcam MASK field is X, DATA field is Y */
    sal_memcpy((uint8*)mask, (uint8*)ipe_cid_tcam_mem, DS_CATEGORY_ID_PAIR_TCAM_LOOKUP_KEY_BYTES);
    sal_memcpy((uint8*)data, (uint8*)ipe_cid_tcam_mem + DS_CATEGORY_ID_PAIR_TCAM_LOOKUP_KEY_BYTES,
              DS_CATEGORY_ID_PAIR_TCAM_LOOKUP_KEY_BYTES);

    return ret;
}

STATIC int32
_ctc_usw_dkits_tcam_read_flow_tcam_entry(uint8 lchip, uint32 blknum, uint32 index, uint32* data, uint32* mask)
{
    uint32 flow_tcam_tcam_mem[FLOW_TCAM_TCAM_MEM_BYTES/4] = {0};
    uint32 cmd = 0;
    int32  ret = DRV_E_NONE;
    uint8  chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;
    uint32 entry_valid = 0;

    DRV_PTR_VALID_CHECK(data);

    /* drv_get_lchip_base(&chip_base); */

    CTC_DKITS_TCAM_FLOW_TCAM_LOCK(lchip_offset);

    idx = (blknum << 11) | index;
    cmd = DRV_IOR(FlowTcamTcamMem_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, flow_tcam_tcam_mem), p_drv_master[lchip_offset]->p_flow_tcam_mutex);

    CTC_DKITS_TCAM_FLOW_TCAM_UNLOCK(lchip_offset);
    DRV_IOR_FIELD(lchip, FlowTcamTcamMem_t, FlowTcamTcamMem_tcamEntryValid_f, &entry_valid, flow_tcam_tcam_mem);

    if(!entry_valid)
    {
        /* -DRV_DBG_INFO("\nFlow Tcam Memory is invalid!, idx:%d\n", idx); */
        /* -return DRV_E_INVALID_TCAM_TYPE; */
    }

    /*
    Read Tcam MASK field is X, DATA field is Y
    */
    sal_memcpy((uint8*)mask, (uint8*)flow_tcam_tcam_mem, DRV_BYTES_PER_ENTRY);
    sal_memcpy((uint8*)data, (uint8*)flow_tcam_tcam_mem + DRV_BYTES_PER_ENTRY, DRV_BYTES_PER_ENTRY);

    return ret;
}

STATIC int32
_ctc_usw_dkits_tcam_read_lpm_tcam_ip_entry(uint8 lchip, uint32 blknum, uint32 index, uint32* data, uint32* mask)
{
    uint32 lpm_tcam_tcam_mem[LPM_TCAM_TCAM_MEM_BYTES/4] = {0};
    uint32 cmd = 0;
    int32  ret = DRV_E_NONE;
    uint8  chip_base = 0;
    uint32 lchip_offset = lchip + chip_base;
    uint32 idx = 0;
    uint32 entry_valid = 0;

    DRV_PTR_VALID_CHECK(data);

    /* drv_get_lchip_base(&chip_base); */

    CTC_DKITS_TCAM_LPM_IP_TCAM_LOCK(lchip_offset);

    if (DRV_IS_DUET2(lchip))
    {
        idx = (blknum << (12)) | index;
    }
    else
    {
        idx = (blknum << (13)) | index;
    }

    cmd = DRV_IOR(LpmTcamTcamMem_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN_WITH_UNLOCK(
    DRV_IOCTL(lchip_offset, idx, cmd, lpm_tcam_tcam_mem), p_drv_master[lchip_offset]->p_lpm_ip_tcam_mutex);

    CTC_DKITS_TCAM_LPM_IP_TCAM_UNLOCK(lchip_offset);

    DRV_IOR_FIELD(lchip, LpmTcamTcamMem_t, LpmTcamTcamMem_tcamEntryValid_f, &entry_valid, lpm_tcam_tcam_mem);

    if(!entry_valid)
    {
        /* DRV_DBG_INFO("\nTcam Memory is invalid!\n"); */
        /* return DRV_E_INVALID_TCAM_TYPE; */
    }

     /* Read Tcam MASK field is X, DATA field is Y */
    sal_memcpy((uint8*)mask, (uint8*)lpm_tcam_tcam_mem, DRV_LPM_KEY_BYTES_PER_ENTRY);
    sal_memcpy((uint8*)data, (uint8*)lpm_tcam_tcam_mem + DRV_LPM_KEY_BYTES_PER_ENTRY, DRV_LPM_KEY_BYTES_PER_ENTRY);

    return ret;
}

/**
 @brief convert embeded tcam content from X/Y format to data/mask format
*/
STATIC int32
_ctc_usw_dkits_tcam_resolve_empty(uint8 lchip, uint32 tcam_entry_width, uint32 *data, uint32 *mask, uint32* p_is_empty)
{
    uint32 bit_pos = 0;
    uint32 index = 0, bit_offset = 0;

    /* data[1] -- [64,80]; data[2] -- [32,63]; data[3] -- [0,31] */
    for (bit_pos = 0; bit_pos < tcam_entry_width; bit_pos++)
    {
        index = bit_pos / 32;
        bit_offset = bit_pos % 32;

        if ((IS_BIT_SET(data[index], bit_offset))
            && IS_BIT_SET(mask[index], bit_offset))    /* X=1, Y=1: No Write but read. */
        {
            *p_is_empty |= 1;
            break;
        }
    }

    return CLI_SUCCESS;
}

int32
_ctc_usw_dkits_tcam_tbl_empty(uint8 lchip, tbls_id_t tbl_id, uint32 index, uint32* p_empty)
{
    ds_t    data;
    ds_t    mask;
    uint32* p_mask = NULL;
    uint32* p_data = NULL;
    uint32  key_size = TABLE_EXT_INFO_PTR(lchip, tbl_id) ? TCAM_KEY_SIZE(lchip, tbl_id) : 0;
    uint32  entry_num_each_idx = 0, entry_idx = 0;
    bool    is_lpm_tcam = FALSE;
    /* uint8   lchip = lchip_offset + drv_init_chip_info.drv_init_lchip_base; */
    uint32  blknum = 0, local_idx = 0, is_sec_half = 0;
    tbl_entry_t entry;

    sal_memset(&data, 0, sizeof(data));
    sal_memset(&mask, 0, sizeof(mask));
    sal_memset(&entry, 0, sizeof(tbl_entry_t));

    entry.data_entry = (uint32 *)&data;
    entry.mask_entry = (uint32 *)&mask;

    DRV_INIT_CHECK(lchip);
    DRV_TBL_ID_VALID_CHECK(lchip, tbl_id);

    *p_empty = 0;

    if ((DRV_TABLE_TYPE_TCAM != drv_usw_get_table_type(lchip, tbl_id))
        && (DRV_TABLE_TYPE_LPM_TCAM_IP != drv_usw_get_table_type(lchip, tbl_id))
        && (DRV_TABLE_TYPE_LPM_TCAM_NAT != drv_usw_get_table_type(lchip, tbl_id))
        && (DRV_TABLE_TYPE_STATIC_TCAM_KEY != drv_usw_get_table_type(lchip, tbl_id)))
    {
        CTC_DKIT_PRINT("@@ DKIT ERROR! INVALID Tcam key TblID! %s %d %s, TblID: %u, \n",
                       __FILE__, __LINE__, __FUNCTION__, tbl_id);
        return CLI_ERROR;
    }

    if (TABLE_MAX_INDEX(lchip, tbl_id) <= index)
    {
        CTC_DKIT_PRINT("@@ DKIT ERROR! %s %d %s chip-0x%x, tbl-%d, index-0x%x exceeds the max_index-0x%x.\n",
                        __FILE__, __LINE__, __FUNCTION__, lchip, tbl_id, index, TABLE_MAX_INDEX(lchip, tbl_id));
        return DRV_E_INVALID_TBL;
    }

    if (DRV_TABLE_TYPE_TCAM == drv_usw_get_table_type(lchip, tbl_id))
    {
        /* flow tcam w/r per 80bit */
        entry_num_each_idx = key_size / DRV_BYTES_PER_ENTRY;
        CTC_DKITS_TCAM_ERROR_RETURN(drv_usw_chip_flow_tcam_get_blknum_index(lchip, tbl_id, index, &blknum, &local_idx, &is_sec_half));
        local_idx = local_idx * (key_size/DRV_BYTES_PER_ENTRY);
    }
    else if ((DRV_TABLE_TYPE_LPM_TCAM_IP == drv_usw_get_table_type(lchip, tbl_id))
            || (DRV_TABLE_TYPE_LPM_TCAM_NAT == drv_usw_get_table_type(lchip, tbl_id)))
    {
        CTC_DKITS_TCAM_ERROR_RETURN(drv_usw_chip_lpm_tcam_get_blknum_index(lchip, tbl_id, index, &blknum, &local_idx));
        is_lpm_tcam = TRUE;
        entry_num_each_idx = key_size / DRV_LPM_KEY_BYTES_PER_ENTRY;
    }
    else if (DRV_TABLE_TYPE_STATIC_TCAM_KEY == drv_usw_get_table_type(lchip, tbl_id))
    {
        entry_num_each_idx = 1;
        local_idx = index;
    }

    p_data = entry.data_entry;
    p_mask = entry.mask_entry;

    for (entry_idx = 0; entry_idx < entry_num_each_idx; entry_idx++)
    {
        /* read real tcam address */
        if (DsQueueMapTcamKey_t == tbl_id)
        {
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_read_enque_tcam_entry(lchip, local_idx, data, mask));
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_resolve_empty(lchip,
                                                                         TABLE_ENTRY_SIZE(lchip, tbl_id)*sizeof(uint32)*8,
                                                                         p_data, p_mask, p_empty));
        }
        else if (DsCategoryIdPairTcamKey_t == tbl_id)
        {
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_read_cid_tcam_entry(lchip, local_idx, data, mask));
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_resolve_empty(lchip,
                                                                         TABLE_ENTRY_SIZE(lchip, tbl_id)*sizeof(uint32)*8,
                                                                         p_data, p_mask, p_empty));
        }
        else if (!is_lpm_tcam)
        {
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_read_flow_tcam_entry(lchip, blknum, local_idx,
                                                                               data + entry_idx*DRV_WORDS_PER_ENTRY,
                                                                               mask + entry_idx*DRV_WORDS_PER_ENTRY));
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_resolve_empty(lchip,
                                                                         DRV_BYTES_PER_ENTRY*8,
                                                                         p_data + entry_idx*DRV_WORDS_PER_ENTRY,
                                                                         p_mask + entry_idx*DRV_WORDS_PER_ENTRY,
                                                                         p_empty));
        }
        else
        {
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_read_lpm_tcam_ip_entry(lchip, blknum, local_idx,
                                                                             data + entry_idx*DRV_LPM_WORDS_PER_ENTRY,
                                                                             mask + entry_idx*DRV_LPM_WORDS_PER_ENTRY));
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_resolve_empty(lchip,
                                                                         DRV_LPM_KEY_BYTES_PER_ENTRY*8,
                                                                         p_data + entry_idx*DRV_LPM_WORDS_PER_ENTRY,
                                                                         p_mask + entry_idx*DRV_LPM_WORDS_PER_ENTRY,
                                                                         p_empty));
        }
        local_idx++;
    }

    return CLI_SUCCESS;
}

#define __1_COMMON__

STATIC int32
_ctc_usw_dkits_tcam_tbl2block(tbls_id_t tblid, dkits_tcam_block_type_t* p_block)
{
    if (DKITS_TCAM_IGS_SCL0_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_IGS_SCL0;
    }
    else if (DKITS_TCAM_IGS_SCL1_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_IGS_SCL1;
    }
    else if (DKITS_TCAM_IGS_SCL2_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_IGS_SCL2;
    }
    else if (DKITS_TCAM_IGS_SCL3_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_IGS_SCL3;
    }
    else if (DKITS_TCAM_IGS_ACL0_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_IGS_ACL0;
    }
    else if (DKITS_TCAM_IGS_ACL1_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_IGS_ACL1;
    }
    else if (DKITS_TCAM_IGS_ACL2_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_IGS_ACL2;
    }
    else if (DKITS_TCAM_IGS_ACL3_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_IGS_ACL3;
    }
    else if (DKITS_TCAM_IGS_ACL4_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_IGS_ACL4;
    }
    else if (DKITS_TCAM_IGS_ACL5_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_IGS_ACL5;
    }
    else if (DKITS_TCAM_IGS_ACL6_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_IGS_ACL6;
    }
    else if (DKITS_TCAM_IGS_ACL7_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_IGS_ACL7;
    }
    else if (DKITS_TCAM_EGS_ACL0_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_EGS_ACL0;
    }
    else if (DKITS_TCAM_EGS_ACL1_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_EGS_ACL1;
    }
    else if (DKITS_TCAM_EGS_ACL2_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_EGS_ACL2;
    }
    else if (DKITS_TCAM_IPLKP0_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_LPM_LKP0;
    }
    else if (DKITS_TCAM_IPLKP1_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_LPM_LKP1;
    }
    else if (DKITS_TCAM_NATPBR_KEY(tblid))
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_NAT_PBR;
    }
    else if (DsCategoryIdPairTcamKey_t == tblid)
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_CATEGORYID;
    }
    else if (DsQueueMapTcamKey_t == tblid)
    {
        *p_block = DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE;
    }
    else
    {
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_info2block(uint8 lchip , ctc_dkits_tcam_type_flag_t tcam_flag, uint32 priority, dkits_tcam_block_type_t* p_block)
{
    *p_block = DKITS_TCAM_BLOCK_TYPE_INVALID;

    switch (tcam_flag)
    {
    case CTC_DKITS_TCAM_TYPE_FLAG_IGS_SCL:
        if (priority > ((DRV_IS_DUET2(lchip)?DKITS_TCAM_BLOCK_TYPE_IGS_SCL1:DKITS_TCAM_BLOCK_TYPE_IGS_SCL3) - DKITS_TCAM_BLOCK_TYPE_IGS_SCL0))
        {
            /* CTC_DKIT_PRINT_DEBUG(" @@ DKIT ERROR! %s %d %s Invalid SCL Lookup Poriority:%u.\n", */
            /*                     __FILE__, __LINE__, __FUNCTION__, priority);                    */
            return CLI_ERROR;
        }
        *p_block = DKITS_TCAM_BLOCK_TYPE_IGS_SCL0 + priority;
        break;
    case CTC_DKITS_TCAM_TYPE_FLAG_IGS_ACL:
        if (priority > (DKITS_TCAM_BLOCK_TYPE_IGS_ACL7 - DKITS_TCAM_BLOCK_TYPE_IGS_ACL0))
        {
            /*CTC_DKIT_PRINT_DEBUG(" @@ DKIT ERROR! %s %d %s Invalid Ingress ACL Lookup Poriority:%u.\n",*/
            /*                     __FILE__, __LINE__, __FUNCTION__, priority);                          */
            return CLI_ERROR;
        }
        *p_block = DKITS_TCAM_BLOCK_TYPE_IGS_ACL0 + priority;
        break;
    case CTC_DKITS_TCAM_TYPE_FLAG_EGS_ACL:
        if (priority > (DKITS_TCAM_BLOCK_TYPE_EGS_ACL2 - DKITS_TCAM_BLOCK_TYPE_EGS_ACL0))
        {
            /*CTC_DKIT_PRINT_DEBUG(" @@ DKIT ERROR! %s %d %s Invalid Egress ACL Lookup Poriority:%u.\n",*/
            /*                     __FILE__, __LINE__, __FUNCTION__, priority);                         */
            return CLI_ERROR;
        }
        *p_block = DKITS_TCAM_BLOCK_TYPE_EGS_ACL0 + priority;
        break;
    case CTC_DKITS_TCAM_TYPE_FLAG_IPPREFIX:
        if (priority > (DKITS_TCAM_BLOCK_TYPE_LPM_LKP1 - DKITS_TCAM_BLOCK_TYPE_LPM_LKP0))
        {
            /*CTC_DKIT_PRINT_DEBUG("@@ DKIT ERROR! %s %d %s Invalid IPUC Lookup Poriority:%u.\n",       */
            /*                     __FILE__, __LINE__, __FUNCTION__, priority);                         */
            return CLI_ERROR;
        }
        *p_block = DKITS_TCAM_BLOCK_TYPE_LPM_LKP0 + priority;
        break;
    case CTC_DKITS_TCAM_TYPE_FLAG_IPNAT:
        if ((priority > 0) && (0xFFFFFFFF != priority))
        {
            /*CTC_DKIT_PRINT_DEBUG("@@ DKIT ERROR! %s %d %s Invalid IPUC Lookup Poriority:%u.\n",       */
            /*                     __FILE__, __LINE__, __FUNCTION__, priority);                         */
            return CLI_ERROR;
        }
        *p_block = DKITS_TCAM_BLOCK_TYPE_NAT_PBR;
        break;
    case CTC_DKITS_TCAM_TYPE_FLAG_CATEGORYID:
        if ((priority > 0) && (0xFFFFFFFF != priority))
        {
            /*CTC_DKIT_PRINT_DEBUG("@@ DKIT ERROR! %s %d %s Invalid IPUC Lookup Poriority:%u.\n",       */
            /*                     __FILE__, __LINE__, __FUNCTION__, priority);                         */
            return CLI_ERROR;
        }
        *p_block = DKITS_TCAM_BLOCK_TYPE_CATEGORYID;
        break;
    case CTC_DKITS_TCAM_TYPE_FLAG_SERVICE_QUEUE:
        if ((priority > 0) && (0xFFFFFFFF != priority))
        {
            /*CTC_DKIT_PRINT_DEBUG("@@ DKIT ERROR! %s %d %s Invalid IPUC Lookup Poriority:%u.\n",       */
            /*                     __FILE__, __LINE__, __FUNCTION__, priority);                         */
            return CLI_ERROR;
        }
        *p_block = DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE;
        break;
    default:
        *p_block = DKITS_TCAM_BLOCK_TYPE_INVALID;
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_key2tbl(uint8 lchip, ctc_dkits_tcam_info_t* p_tcam_info, tbls_id_t* p_tblid, uint32* p_tblnum)
{
    uint32 i = 0;
    dkits_tcam_block_type_t block_type = DKITS_TCAM_BLOCK_TYPE_INVALID;

    *p_tblnum = 0;
    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_info2block(lchip, p_tcam_info->tcam_type, p_tcam_info->priority, &block_type));

    for (i = 0; MaxTblId_t != ctc_dkits_tcam_tbl[block_type][i].tblid; i++)
    {
        if (p_tcam_info->key_type == ctc_dkits_tcam_tbl[block_type][i].key_type)
        {
            p_tblid[*p_tblnum] = ctc_dkits_tcam_tbl[block_type][i].tblid;
            (*p_tblnum)++;
        }
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_tbl2class(tbls_id_t tblid, dkits_tcam_lpm_class_type_t* p_class)
{
    switch (tblid)
    {
    case DsLpmTcamIpv4HalfKey_t:
    case DsLpmTcamIpv6SingleKey_t:
    case DsLpmTcamIpv6DoubleKey0Lookup1_t:
        *p_class = DKITS_TCAM_LPM_CLASS_IGS_LPM0;
        break;

    case DsLpmTcamIpv4SaHalfKeyLookup1_t:
    case DsLpmTcamIpv6SaSingleKey_t:
    case DsLpmTcamIpv6SaDoubleKey0Lookup1_t:
        *p_class = DKITS_TCAM_LPM_CLASS_IGS_LPM0SAPRI;
        break;

    case DsLpmTcamIpv4DaPubHalfKeyLookup1_t:
    case DsLpmTcamIpv6DaPubSingleKey_t:
    case DsLpmTcamIpv6DaPubDoubleKey0Lookup1_t:
        *p_class = DKITS_TCAM_LPM_CLASS_IGS_LPM0DAPUB;
        break;

    case DsLpmTcamIpv4SaPubHalfKeyLookup1_t:
    case DsLpmTcamIpv6SaPubSingleKey_t:
    case DsLpmTcamIpv6SaPubDoubleKey0Lookup1_t:
        *p_class = DKITS_TCAM_LPM_CLASS_IGS_LPM0SAPUB;
        break;

    case DsLpmTcamIpv4HalfKeyLookup2_t:
    case DsLpmTcamIpv6DoubleKey0Lookup2_t:
        *p_class = DKITS_TCAM_LPM_CLASS_IGS_LPM0_LK2;
        break;

    case DsLpmTcamIpv4SaHalfKeyLookup2_t:
    case DsLpmTcamIpv6SaDoubleKey0Lookup2_t:
        *p_class = DKITS_TCAM_LPM_CLASS_IGS_LPM0SAPRI_LK2;
        break;

    case DsLpmTcamIpv4DaPubHalfKeyLookup2_t:
    case DsLpmTcamIpv6DaPubDoubleKey0Lookup2_t:
        *p_class = DKITS_TCAM_LPM_CLASS_IGS_LPM0DAPUB_LK2;
        break;

    case DsLpmTcamIpv4SaPubHalfKeyLookup2_t:
    case DsLpmTcamIpv6SaPubDoubleKey0Lookup2_t:
        *p_class = DKITS_TCAM_LPM_CLASS_IGS_LPM0SAPUB_LK2;
        break;

    case DsLpmTcamIpv4DoubleKey_t:
    case DsLpmTcamIpv4PbrDoubleKey_t:
    case DsLpmTcamIpv4NatDoubleKey_t:
    case DsLpmTcamIpv6QuadKey_t:
    case DsLpmTcamIpv6DoubleKey1_t:
        *p_class = DKITS_TCAM_LPM_CLASS_IGS_NAT_PBR;
        break;

    default:
        *p_class = DKITS_TCAM_LPM_CLASS_INVALID;
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

/* offset input unit is DKITS_LPM_TCAM_MODULE_XXX, output unit is 12 lpm tcam block */
STATIC int32
_ctc_usw_dkits_tcam_lpm_block2offset(uint32 blknum, uint32* p_offset)
{
    switch (blknum)
    {
    case 0:
    case 4:
    case 8:
        ;
        break;
    case 1:
        *p_offset = DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM;
        break;
    case 2:
        *p_offset = DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM;
        break;
    case 3:
        *p_offset = DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY2_MAX_ENTRY_NUM;
        break;
    case 5:
        *p_offset = DRV_LPM_TCAM_KEY4_MAX_ENTRY_NUM;
        break;
    case 6:
        *p_offset = DRV_LPM_TCAM_KEY4_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY5_MAX_ENTRY_NUM;
        break;
    case 7:
        *p_offset = DRV_LPM_TCAM_KEY4_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY5_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY6_MAX_ENTRY_NUM;
        break;
    case 9:
        *p_offset = DRV_LPM_TCAM_KEY8_MAX_ENTRY_NUM;
        break;
    case 10:
        *p_offset = DRV_LPM_TCAM_KEY8_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY9_MAX_ENTRY_NUM;
        break;
    case 11:
        *p_offset = DRV_LPM_TCAM_KEY8_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY9_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY10_MAX_ENTRY_NUM;
        break;
    default:
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_lpm_module2tcamid(dkits_lpm_tcam_module_t module, uint32* p_tcamid)
{
    switch (module)
    {
    /* 000 XXX*/
    case DKITS_LPM_TCAM_MODULE_LKPUP0:
        *p_tcamid = 0;
        break;
    /* 001 XXX*/
    case DKITS_LPM_TCAM_MODULE_LKPUP1:
    /* 01 XXX*/
    case DKITS_LPM_TCAM_MODULE_NATPBR:
        *p_tcamid = 1;
        break;
    default:
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

/* offset unit is DKITS_LPM_TCAM_MODULE_XXX */
STATIC int32
_ctc_usw_dkits_tcam_lpm_offset2block(dkits_lpm_tcam_module_t module, uint32 offset, uint32* p_blknum)
{
    if (DKITS_LPM_TCAM_MODULE_LKPUP0 == module)
    {
        if (offset < DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM)
        {
            *p_blknum = 0;
        }
        else if ((offset >= DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM)
                 && (offset < DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM))
        {
            *p_blknum = 1;
        }
        else if ((offset >= DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM)
                && (offset < DRV_LPM_TCAM_KEY0_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY1_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY2_MAX_ENTRY_NUM))
        {
            *p_blknum = 2;
        }
        else
        {
            *p_blknum = 3;
        }
    }
    else if (DKITS_LPM_TCAM_MODULE_LKPUP1 == module)
    {
        if (offset < DRV_LPM_TCAM_KEY4_MAX_ENTRY_NUM)
        {
            *p_blknum = 4;
        }
        else if ((offset >= DRV_LPM_TCAM_KEY4_MAX_ENTRY_NUM)
                && (offset < DRV_LPM_TCAM_KEY4_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY5_MAX_ENTRY_NUM))
        {
            *p_blknum = 5;
        }
        else if ((offset >= DRV_LPM_TCAM_KEY4_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY5_MAX_ENTRY_NUM)
                && (offset < DRV_LPM_TCAM_KEY4_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY5_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY6_MAX_ENTRY_NUM))
        {
            *p_blknum = 6;
        }
        else
        {
            *p_blknum = 7;
        }
    }
    else if (DKITS_LPM_TCAM_MODULE_NATPBR == module)
    {
        if (offset < DRV_LPM_TCAM_KEY8_MAX_ENTRY_NUM)
        {
            *p_blknum = 8;
        }
        else if ((offset >= DRV_LPM_TCAM_KEY8_MAX_ENTRY_NUM)
                && (offset < DRV_LPM_TCAM_KEY8_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY9_MAX_ENTRY_NUM))
        {
            *p_blknum = 9;
        }
        else if ((offset >= DRV_LPM_TCAM_KEY8_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY9_MAX_ENTRY_NUM)
                && (offset < DRV_LPM_TCAM_KEY8_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY9_MAX_ENTRY_NUM + DRV_LPM_TCAM_KEY10_MAX_ENTRY_NUM))
        {
            *p_blknum = 10;
        }
        else
        {
            *p_blknum = 11;
        }
    }
    else
    {
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_detect_delay(dkits_tcam_block_type_t block, uint32* p_delay, dkits_tcam_detect_type_t type)
{
    if (CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block))
    {
        *p_delay = (DKITS_TCAM_DETECT_TYPE_BIST == type) ? CTC_DKITS_TCAM_FLOW_TCAM_DETECT_BIST_DELAY
                                        : CTC_DKITS_TCAM_FLOW_TCAM_DETECT_CAPTURE_DELAY;
    }
    else if (CTC_DKITS_TCAM_IP_TCAM_BLOCK(block) || CTC_DKITS_TCAM_NAT_TCAM_BLOCK(block))
    {
        *p_delay = (DKITS_TCAM_DETECT_TYPE_BIST == type) ? CTC_DKITS_TCAM_LPM_TCAM_DETECT_BIST_DELAY
                                        : CTC_DKITS_TCAM_LPM_TCAM_DETECT_CAPTURE_DELAY;
    }
    else if (CTC_DKITS_TCAM_CID_TCAM_BLOCK(block))
    {
        *p_delay = (DKITS_TCAM_DETECT_TYPE_BIST == type) ? CTC_DKITS_TCAM_CID_TCAM_DETECT_BIST_DELAY
                                        : CTC_DKITS_TCAM_CID_TCAM_DETECT_CAPTURE_DELAY;
    }
    else if (CTC_DKITS_TCAM_QUE_TCAM_BLOCK(block))
    {
        *p_delay = (DKITS_TCAM_DETECT_TYPE_BIST == type) ? CTC_DKITS_TCAM_QUE_TCAM_DETECT_BIST_DELAY
                                        : CTC_DKITS_TCAM_QUE_TCAM_DETECT_CAPTURE_DELAY;
    }
    else
    {
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_tbl_index(tbls_id_t tblid, uint32* p_blk, uint32 offset, uint32* p_tbl_idx)
{
    uint8  lchip = 0;
    uint32 idx = 0;
    uint32 is_sec_half = 0;
    uint32 entry_offset = 0;
    uint32 entry_num = 0;
    uint32 tcamid = 0;
    dkits_tcam_block_type_t block_type = DKITS_TCAM_BLOCK_TYPE_INVALID;
    dkits_lpm_tcam_module_t module = DKITS_LPM_TCAM_MODULE_NUM;

     _ctc_usw_dkits_tcam_tbl2block(tblid, &block_type);

    if (CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block_type))
    {
        CTC_DKIT_PRINT_DEBUG(" %-52s %u\n", __FUNCTION__, __LINE__);

        offset = offset & 0x1FFF;
        CTC_DKITS_TCAM_ERROR_RETURN(drv_usw_chip_flow_tcam_get_blknum_index(lchip, tblid, TCAM_START_INDEX(lchip, tblid, *p_blk), p_blk, &idx, &is_sec_half));
        CTC_DKIT_PRINT_DEBUG(" %-52s %u\n", __FUNCTION__, __LINE__);

        idx = idx * (TCAM_KEY_SIZE(lchip, tblid)/DRV_BYTES_PER_ENTRY);

        /* offset unit is 3w */
        if (idx  > offset)
        {
            *p_tbl_idx = CTC_DKITS_INVALID_INDEX;
        }
        else
        {
            entry_num =  TABLE_ENTRY_SIZE(lchip, tblid) / DRV_BYTES_PER_ENTRY;
            *p_tbl_idx = TCAM_START_INDEX(lchip, tblid, *p_blk) + ((offset - (idx * entry_num)) / entry_num);
        }
        CTC_DKIT_PRINT_DEBUG(" %-52s %u\n", __FUNCTION__, __LINE__);
    }
    else if (CTC_DKITS_TCAM_IP_TCAM_BLOCK(block_type) || CTC_DKITS_TCAM_NAT_TCAM_BLOCK(block_type))
    {
        if (DKITS_TCAM_BLOCK_TYPE_LPM_LKP0 == block_type)
        {
            module = DKITS_LPM_TCAM_MODULE_LKPUP0;
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_lpm_module2tcamid(module, &tcamid));
            if (tcamid != DKITS_TCAM_IP_RLT_TCAMID(offset))
            {
                CTC_DKIT_PRINT(" %s %d, %s is mismatch with its result lpm tcam module:%u.\n",
                               __FUNCTION__, __LINE__, TABLE_NAME(lchip, tblid), module);
            }
            offset = DKITS_TCAM_IP_RLT_TCAMIDX(offset);
        }
        else if (DKITS_TCAM_BLOCK_TYPE_LPM_LKP1 == block_type)
        {
            module = DKITS_LPM_TCAM_MODULE_LKPUP1;
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_lpm_module2tcamid(module, &tcamid));
            if (tcamid != DKITS_TCAM_IP_RLT_TCAMID(offset))
            {
                CTC_DKIT_PRINT(" %s %d, %s is mismatch with its result lpm tcam module:%u.\n",
                               __FUNCTION__, __LINE__, TABLE_NAME(lchip, tblid), module);
            }
            offset = DKITS_TCAM_IP_RLT_TCAMIDX(offset);
        }
        else
        {
            module = DKITS_LPM_TCAM_MODULE_NATPBR;
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_lpm_module2tcamid(module, &tcamid));
            if (tcamid != DKITS_TCAM_NAT_RLT_TCAMID(offset))
            {
                CTC_DKIT_PRINT(" %s %d, %s is mismatch with its result lpm tcam module:%u.\n",
                               __FUNCTION__, __LINE__, TABLE_NAME(lchip, tblid), module);
            }
            offset = DKITS_TCAM_NAT_RLT_TCAMIDX(offset);
        }

        /* offset unit is tcam module */
        CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_lpm_offset2block(module, offset, p_blk));
        /* offset unit is tcam block */
        CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_lpm_block2offset(*p_blk, &entry_offset));
        offset -= entry_offset;
        CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_get_lpm_tcam_blknum_index(lchip, tblid, TCAM_START_INDEX(lchip, tblid, *p_blk), p_blk, &idx));
        idx = idx * (TCAM_KEY_SIZE(lchip, tblid)/DRV_LPM_KEY_BYTES_PER_ENTRY);

        /* offset unit is 2w */
        if (idx > offset)
        {
            *p_tbl_idx = CTC_DKITS_INVALID_INDEX;
        }
        else
        {
            entry_num = TCAM_KEY_SIZE(lchip, tblid) / DRV_LPM_KEY_BYTES_PER_ENTRY;
            *p_tbl_idx = TCAM_START_INDEX(lchip, tblid, *p_blk) + ((offset - (idx * entry_num)) / entry_num);
        }
    }
    else if ((DKITS_TCAM_BLOCK_TYPE_CATEGORYID == block_type) || (DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE == block_type))
    {
        *p_tbl_idx = offset;
    }

    return CLI_SUCCESS;
}

STATIC void
_ctc_usw_dkits_tcam_add_seg(uint32 seg, uint16 seg_len, uint16* p_seg_offset, uint8* p_ds)
{
    uint8   start_byte_index = 0;
    uint8   offset_in_byte = (*p_seg_offset) % 8;
    int16   remain_bits = seg_len;
    int16   remain_bytes = 8;
    uint64  field64 = seg;

    start_byte_index = (*p_seg_offset) / 8;
    offset_in_byte = (*p_seg_offset) % 8;

    field64 = field64 << (64 - seg_len);
    field64 = field64 >> offset_in_byte;
    remain_bits += offset_in_byte;

    do
    {
        p_ds[start_byte_index++] |= ((field64 >> ((remain_bytes - 1) * 8)) & 0xFF);
        remain_bits -= 8;
        remain_bytes--;
    } while (remain_bits > 0);

    *p_seg_offset += seg_len;
}

STATIC int32
_ctc_usw_dkits_tcam_xbits2key(uint8 lchip, tbls_id_t tblid, void* p_src_key, void* p_dst_key)
{
    uint8   word_index = 0;
    uint16  seg_offset = 0;
    uint32  value = 0;
    dkits_tcam_block_type_t block = DKITS_TCAM_BLOCK_TYPE_INVALID;

    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_tbl2block(tblid, &block));

    if (CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block))
    {
        for (word_index = 0; word_index < TABLE_ENTRY_SIZE(lchip, tblid) / DRV_BYTES_PER_WORD; word_index++)
        {
            if (0 == ((word_index + 1) % 3))
            {
                value = ((uint32*)p_src_key)[word_index] & 0xFFFF;
                _ctc_usw_dkits_tcam_add_seg(value, 32, &seg_offset, (uint8*)p_dst_key);
                value = (((uint32*)p_src_key)[word_index] >> 16) & 0xFFFF;
                _ctc_usw_dkits_tcam_add_seg(value, 16, &seg_offset, (uint8*)p_dst_key);
            }
            else
            {
                _ctc_usw_dkits_tcam_add_seg(((uint32*)p_src_key)[word_index], 32, &seg_offset, (uint8*)p_dst_key);
            }
        }
    }
    else if (CTC_DKITS_TCAM_IP_TCAM_BLOCK(block) || CTC_DKITS_TCAM_NAT_TCAM_BLOCK(block))
    {
        for (word_index = 0; word_index < TABLE_ENTRY_SIZE(lchip, tblid) / DRV_BYTES_PER_WORD; word_index++)
        {
            if (0 == ((word_index + 1) % 2))
            {
                value = ((uint32*)p_src_key)[word_index] & 0x3FFF;
                _ctc_usw_dkits_tcam_add_seg(value, 32, &seg_offset, (uint8*)p_dst_key);
                value = (((uint32*)p_src_key)[word_index] >> 14) & 0x3FFFF;
                _ctc_usw_dkits_tcam_add_seg(value, 18, &seg_offset, (uint8*)p_dst_key);
            }
            else
            {
                _ctc_usw_dkits_tcam_add_seg(((uint32*)p_src_key)[word_index], 32, &seg_offset, (uint8*)p_dst_key);
            }
        }
    }
    else if (DKITS_TCAM_BLOCK_TYPE_CATEGORYID == block)
    {
        for (word_index = 0; word_index < TABLE_ENTRY_SIZE(lchip, tblid) / DRV_BYTES_PER_WORD; word_index++)
        {
            ((uint32*)p_dst_key)[word_index] = ((uint32*)p_src_key)[word_index];
        }
    }
    else if (DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE == block)
    {
        for (word_index = 0; word_index < TABLE_ENTRY_SIZE(lchip, tblid) / DRV_BYTES_PER_WORD; word_index++)
        {
            ((uint32*)p_dst_key)[word_index] = ((uint32*)p_src_key)[word_index];
        }
    }

    return DRV_E_NONE;
}

STATIC int32
_ctc_usw_dkits_tcam_build_key(uint8 lchip, tbls_id_t tblid, tbl_entry_t* p_tcam_key, uint32* p_ds)
{
    uint32       i = 0, word = 0;

    word = TCAM_KEY_SIZE(lchip, tblid)/DRV_BYTES_PER_WORD;
    for (i = 0; i < word; i++)
    {
        p_ds[i] = p_tcam_key->data_entry[i] & p_tcam_key->mask_entry[i];
    }

    CTC_DKIT_PRINT_DEBUG("                                                            ");
    for (i = 0; i < word; i++)
    {
        CTC_DKIT_PRINT_DEBUG("key[%u]=0x%08X ", i, p_ds[i]);
        if ((0 == ((i + 1) % 5)) && (word != (i + 1)))
        {
            CTC_DKIT_PRINT_DEBUG("\n                                                            ");
        }
    }
    CTC_DKIT_PRINT_DEBUG("\n");

    return DRV_E_NONE;
}

STATIC int32
_ctc_usw_dkits_tcam_set_detect(uint8 lchip, dkits_tcam_block_type_t block, dkits_tcam_detect_property_t detect_property, uint32 value)
{
    uint32 cmd = 0, i = 0, step = 0, bmp = 0;
    ds_t   ds = {0};

    if (CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block))
    {
        cmd = DRV_IOR(FlowTcamBistCtl_t, DRV_ENTRY_FLAG);
        CTC_DKITS_TCAM_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds));

        switch (detect_property)
        {
        case DKITS_TCAM_DETECT_BIST_EN:
            if (value)
            {
                DKITS_BIT_SET(bmp, block);
            }
            else
            {
                DKITS_BIT_UNSET(bmp, block);
            }
            SetFlowTcamBistCtl(V, cfgBistEn_f, &ds, bmp);
            CTC_DKIT_PRINT_DEBUG(" FlowTcamBistCtl[0].cfgBistEn:0x%04X", bmp);
            break;

        case DKITS_TCAM_DETECT_BIST_ONCE:
            SetFlowTcamBistCtl(V, cfgBistOnce_f, &ds, value);
            CTC_DKIT_PRINT_DEBUG(" cfgBistOnce:%u", value);
            break;

        case DKITS_TCAM_DETECT_CAPTURE_EN:
            if (value)
            {
                DKITS_BIT_SET(bmp, block);
            }
            else
            {
                DKITS_BIT_UNSET(bmp, block);
            }
            SetFlowTcamBistCtl(V, cfgCaptureEn_f, &ds, bmp);
            CTC_DKIT_PRINT_DEBUG(" FlowTcamBistCtl[0].cfgCaptureEn:0x%04X", bmp);
            break;

        case DKITS_TCAM_DETECT_CAPTURE_ONCE:
            SetFlowTcamBistCtl(V, cfgCaptureOnce_f, &ds, value);
            CTC_DKIT_PRINT_DEBUG(" cfgCaptureOnce:%u", value);
            break;

        case DKITS_TCAM_DETECT_DELAY:
            SetFlowTcamBistCtl(V, cfgCaptureIndexDly_f, &ds, value);
            CTC_DKIT_PRINT_DEBUG(" cfgCaptureIndexDly:%u", value);
            break;

        default:
            return CLI_ERROR;
        }

        cmd = DRV_IOW(FlowTcamBistCtl_t, DRV_ENTRY_FLAG);
        CTC_DKITS_TCAM_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds));
    }
    else if (CTC_DKITS_TCAM_IP_TCAM_BLOCK(block) || CTC_DKITS_TCAM_NAT_TCAM_BLOCK(block))
    {
        i = block - DKITS_TCAM_BLOCK_TYPE_LPM_LKP0;
        step = LpmTcamBistCtrl_cfgDaSa1BistEn_f - LpmTcamBistCtrl_cfgDaSa0BistEn_f;

        cmd = DRV_IOR(LpmTcamBistCtrl_t, DRV_ENTRY_FLAG);
        CTC_DKITS_TCAM_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds));

        switch (detect_property)
        {
        case DKITS_TCAM_DETECT_BIST_EN:
            DRV_SET_FIELD_V(lchip, LpmTcamBistCtrl_t, LpmTcamBistCtrl_cfgDaSa0BistEn_f + (i * step), &ds, value);

            if (DKITS_TCAM_BLOCK_TYPE_LPM_LKP0 == block)
            {
                CTC_DKIT_PRINT_DEBUG(" LpmTcamBistCtrl[0].cfgDaSa0BistEn:%u \n", value);
            }
            else if (DKITS_TCAM_BLOCK_TYPE_LPM_LKP1 == block)
            {
                CTC_DKIT_PRINT_DEBUG(" LpmTcamBistCtrl[0].cfgDaSa1BistEn:%u \n", value);
            }
            else
            {
                CTC_DKIT_PRINT_DEBUG(" LpmTcamBistCtrl[0].cfgShareBistEn:%u \n", value);
            }
            break;

        case DKITS_TCAM_DETECT_BIST_ONCE:
            DRV_SET_FIELD_V(lchip, LpmTcamBistCtrl_t, LpmTcamBistCtrl_cfgDaSa0BistOnce_f + (i * step), &ds, value);
            if (DKITS_TCAM_BLOCK_TYPE_LPM_LKP0 == block)
            {
                CTC_DKIT_PRINT_DEBUG(" cfgDaSa0BistOnce:%u \n", value);
            }
            else if (DKITS_TCAM_BLOCK_TYPE_LPM_LKP1 == block)
            {
                CTC_DKIT_PRINT_DEBUG(" cfgDaSa1BistOnce:%u \n", value);
            }
            else
            {
                CTC_DKIT_PRINT_DEBUG(" cfgShareBistOnce:%u \n", value);
            }
            break;

        case DKITS_TCAM_DETECT_CAPTURE_EN:
            DRV_SET_FIELD_V(lchip, LpmTcamBistCtrl_t, LpmTcamBistCtrl_cfgDaSa0CaptureEn_f + (i * step), &ds, value);
            if (DKITS_TCAM_BLOCK_TYPE_LPM_LKP0 == block)
            {
                CTC_DKIT_PRINT_DEBUG(" LpmTcamBistCtrl[0].cfgDaSa0CaptureEn:%u \n", value);
            }
            else if (DKITS_TCAM_BLOCK_TYPE_LPM_LKP1 == block)
            {
                CTC_DKIT_PRINT_DEBUG(" LpmTcamBistCtrl[0].cfgDaSa1CaptureEn:%u \n", value);
            }
            else
            {
                CTC_DKIT_PRINT_DEBUG(" LpmTcamBistCtrl[0].cfgShareCaptureEn:%u \n", value);
            }
            break;

        case DKITS_TCAM_DETECT_CAPTURE_ONCE:
            DRV_SET_FIELD_V(lchip, LpmTcamBistCtrl_t, LpmTcamBistCtrl_cfgDaSa0CaptureOnce_f + (i * step), &ds, value);
            if (DKITS_TCAM_BLOCK_TYPE_LPM_LKP0 == block)
            {
                CTC_DKIT_PRINT_DEBUG(" cfgDaSa0CaptureOnce:%u \n", value);
            }
            else if (DKITS_TCAM_BLOCK_TYPE_LPM_LKP1 == block)
            {
                CTC_DKIT_PRINT_DEBUG(" cfgDaSa1CaptureOnce:%u \n", value);
            }
            else
            {
                CTC_DKIT_PRINT_DEBUG(" cfgShareCaptureOnce:%u \n", value);
            }
            break;

        case DKITS_TCAM_DETECT_DELAY:
            DRV_SET_FIELD_V(lchip, LpmTcamBistCtrl_t, LpmTcamBistCtrl_cfgCaptureIndexDly_f, &ds, value);
            CTC_DKIT_PRINT_DEBUG(" cfgCaptureIndexDly:%u \n", value);
            break;

        default:
            return CLI_ERROR;
        }

        cmd = DRV_IOW(LpmTcamBistCtrl_t, DRV_ENTRY_FLAG);
        CTC_DKITS_TCAM_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds));
    }
    else if (DKITS_TCAM_BLOCK_TYPE_CATEGORYID == block)
    {
        cmd = DRV_IOR(IpeCidTcamBistCtl_t, DRV_ENTRY_FLAG);
        CTC_DKITS_TCAM_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds));

        switch (detect_property)
        {
        case DKITS_TCAM_DETECT_BIST_EN:
            SetIpeCidTcamBistCtl(V, cfgBistEn_f, &ds, value);
            CTC_DKIT_PRINT_DEBUG(" IpeCidTcamBistCtl[0].cfgBistEn:%u", value);
            break;

        case DKITS_TCAM_DETECT_BIST_ONCE:
            SetIpeCidTcamBistCtl(V, cfgBistOnce_f, &ds, value);
            CTC_DKIT_PRINT_DEBUG(" cfgBistOnce:%u", value);
            break;

        case DKITS_TCAM_DETECT_CAPTURE_EN:
            SetIpeCidTcamBistCtl(V, cfgCaptureEn_f, &ds, value);
            CTC_DKIT_PRINT_DEBUG(" IpeCidTcamBistCtl[0].cfgCaptureEn:%u", value);
            break;

        case DKITS_TCAM_DETECT_CAPTURE_ONCE:
            SetIpeCidTcamBistCtl(V, cfgCaptureOnce_f, &ds, value);
            CTC_DKIT_PRINT_DEBUG(" cfgCaptureOnce:%u", value);
            break;

        case DKITS_TCAM_DETECT_DELAY:
            SetIpeCidTcamBistCtl(V, cfgCaptureIndexDly_f, &ds, value);
            CTC_DKIT_PRINT_DEBUG(" cfgCaptureIndexDly:%u", value);
            break;

        default:
            return CLI_ERROR;
        }

        cmd = DRV_IOW(IpeCidTcamBistCtl_t, DRV_ENTRY_FLAG);
        CTC_DKITS_TCAM_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds));
    }
    else if (DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE == block)
    {
        cmd = DRV_IOR(QMgrEnqTcamBistCtl_t, DRV_ENTRY_FLAG);
        CTC_DKITS_TCAM_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds));

        switch (detect_property)
        {
        case DKITS_TCAM_DETECT_BIST_EN:
            SetQMgrEnqTcamBistCtl(V, tcamCfgBistEn_f, &ds, value);
            CTC_DKIT_PRINT_DEBUG(" QMgrEnqTcamBistCtl[0].cfgBistEn:%u", value);
            break;

        case DKITS_TCAM_DETECT_BIST_ONCE:
            SetQMgrEnqTcamBistCtl(V, tcamCfgBistOnce_f, &ds, value);
            CTC_DKIT_PRINT_DEBUG(" cfgBistOnce:%u", value);
            break;

        case DKITS_TCAM_DETECT_CAPTURE_EN:
            SetQMgrEnqTcamBistCtl(V, tcamCfgCaptureEn_f, &ds, value);
            CTC_DKIT_PRINT_DEBUG(" QMgrEnqTcamBistCtl[0].cfgCaptureEn:%u", value);
            break;

        case DKITS_TCAM_DETECT_CAPTURE_ONCE:
            SetQMgrEnqTcamBistCtl(V, tcamCfgCaptureOnce_f, &ds, value);
            CTC_DKIT_PRINT_DEBUG(" cfgCaptureOnce:%u", value);
            break;

        case DKITS_TCAM_DETECT_DELAY:
            SetQMgrEnqTcamBistCtl(V, tcamCfgCaptureIndexDly_f, &ds, value);
            CTC_DKIT_PRINT_DEBUG(" cfgCaptureIndexDly:%u", value);
            break;

        default:
            return CLI_ERROR;
        }

        cmd = DRV_IOW(QMgrEnqTcamBistCtl_t, DRV_ENTRY_FLAG);
        CTC_DKITS_TCAM_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds));
    }
    else
    {
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_get_detect(uint8 lchip, dkits_tcam_block_type_t block, dkits_tcam_detect_property_t detect_property, uint32* p_value)
{
    uint32 cmd = 0, i = 0, step = 0;
    ds_t   ds = {0};

    if (CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block))
    {
        cmd = DRV_IOR(FlowTcamBistCtl_t, DRV_ENTRY_FLAG);
        CTC_DKITS_TCAM_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds));

        switch (detect_property)
        {
        case DKITS_TCAM_DETECT_BIST_EN:
            GetFlowTcamBistCtl(A, cfgBistEn_f, &ds, p_value);
            if (DKITS_IS_BIT_SET(*p_value, block))
            {
                *p_value = 1;
            }
            else
            {
                *p_value = 0;
            }
            break;

        case DKITS_TCAM_DETECT_BIST_ONCE:
            GetFlowTcamBistCtl(A, cfgBistOnce_f, &ds, p_value);
            break;

        case DKITS_TCAM_DETECT_CAPTURE_EN:
            GetFlowTcamBistCtl(A, cfgCaptureEn_f, &ds, p_value);
            if (DKITS_IS_BIT_SET(*p_value, block))
            {
                *p_value = 1;
            }
            else
            {
                *p_value = 0;
            }
            break;

        case DKITS_TCAM_DETECT_CAPTURE_ONCE:
            GetFlowTcamBistCtl(A, cfgCaptureOnce_f, &ds, p_value);
            break;

        case DKITS_TCAM_DETECT_DELAY:
            GetFlowTcamBistCtl(A, cfgCaptureIndexDly_f, &ds, p_value);
            break;

        default:
            return CLI_ERROR;
        }
    }
    else if (CTC_DKITS_TCAM_IP_TCAM_BLOCK(block) || CTC_DKITS_TCAM_NAT_TCAM_BLOCK(block))
    {
        i = block - DKITS_TCAM_BLOCK_TYPE_LPM_LKP0;
        step = LpmTcamBistCtrl_cfgDaSa1BistEn_f - LpmTcamBistCtrl_cfgDaSa0BistEn_f;

        cmd = DRV_IOR(LpmTcamBistCtrl_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &ds);

        switch (detect_property)
        {
        case DKITS_TCAM_DETECT_BIST_EN:
            DRV_GET_FIELD_A(lchip, LpmTcamBistCtrl_t, LpmTcamBistCtrl_cfgDaSa0BistEn_f + (i * step), &ds, p_value);
            break;

        case DKITS_TCAM_DETECT_BIST_ONCE:
            DRV_GET_FIELD_A(lchip, LpmTcamBistCtrl_t, LpmTcamBistCtrl_cfgDaSa0BistOnce_f + (i * step), &ds, p_value);
            break;

        case DKITS_TCAM_DETECT_CAPTURE_EN:
            DRV_GET_FIELD_A(lchip, LpmTcamBistCtrl_t, LpmTcamBistCtrl_cfgDaSa0CaptureEn_f + (i * step), &ds, p_value);
            break;

        case DKITS_TCAM_DETECT_CAPTURE_ONCE:
            DRV_GET_FIELD_A(lchip, LpmTcamBistCtrl_t, LpmTcamBistCtrl_cfgDaSa0CaptureOnce_f + (i * step), &ds, p_value);
            break;

        case DKITS_TCAM_DETECT_DELAY:
            DRV_GET_FIELD_A(lchip, LpmTcamBistCtrl_t, LpmTcamBistCtrl_cfgCaptureIndexDly_f, &ds, p_value);
            break;

        default:
            return CLI_ERROR;
        }
    }
    else if (DKITS_TCAM_BLOCK_TYPE_CATEGORYID == block)
    {
        cmd = DRV_IOR(IpeCidTcamBistCtl_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &ds);

        switch (detect_property)
        {
        case DKITS_TCAM_DETECT_BIST_EN:
            GetIpeCidTcamBistCtl(A, cfgBistEn_f, &ds, p_value);
            break;

        case DKITS_TCAM_DETECT_BIST_ONCE:
            GetIpeCidTcamBistCtl(A, cfgBistOnce_f, &ds, p_value);
            break;

        case DKITS_TCAM_DETECT_CAPTURE_EN:
            GetIpeCidTcamBistCtl(A, cfgCaptureEn_f, &ds, p_value);
            break;

        case DKITS_TCAM_DETECT_CAPTURE_ONCE:
            GetIpeCidTcamBistCtl(A, cfgCaptureOnce_f, &ds, p_value);
            break;

        case DKITS_TCAM_DETECT_DELAY:
            GetIpeCidTcamBistCtl(A, cfgCaptureIndexDly_f, &ds, p_value);
            break;

        default:
            return CLI_ERROR;
        }
    }
    else if (DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE == block)
    {
        cmd = DRV_IOR(QMgrEnqTcamBistCtl_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, 0, cmd, &ds);

        switch (detect_property)
        {
        case DKITS_TCAM_DETECT_BIST_EN:
            GetQMgrEnqTcamBistCtl(A, tcamCfgBistEn_f, &ds, p_value);
            break;

        case DKITS_TCAM_DETECT_BIST_ONCE:
            GetQMgrEnqTcamBistCtl(A, tcamCfgBistOnce_f, &ds, p_value);
            break;

        case DKITS_TCAM_DETECT_CAPTURE_EN:
            GetQMgrEnqTcamBistCtl(A, tcamCfgCaptureEn_f, &ds, p_value);
            break;

        case DKITS_TCAM_DETECT_CAPTURE_ONCE:
            GetQMgrEnqTcamBistCtl(A, tcamCfgCaptureOnce_f, &ds, p_value);
            break;

        case DKITS_TCAM_DETECT_DELAY:
            GetQMgrEnqTcamBistCtl(A, tcamCfgCaptureIndexDly_f, &ds, p_value);
            break;

        default:
            return CLI_ERROR;
        }
    }
    else
    {
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_reset_detect(uint8 lchip, dkits_tcam_block_type_t block, dkits_tcam_detect_type_t detect_type)
{
    uint32 i = 0, delay = 0;
    uint32 cmd = 0;
    ds_t   ds = {0};

    CTC_DKIT_PRINT_DEBUG(" %-52s %u:", __FUNCTION__, __LINE__);
    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_set_detect(lchip, block, DKITS_TCAM_DETECT_BIST_EN, 0));
    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_set_detect(lchip, block, DKITS_TCAM_DETECT_BIST_ONCE, 0));
    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_set_detect(lchip, block, DKITS_TCAM_DETECT_CAPTURE_EN, 0));
    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_set_detect(lchip, block, DKITS_TCAM_DETECT_CAPTURE_ONCE, 0));
    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_detect_delay(block, &delay, detect_type));
    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_set_detect(lchip, block, DKITS_TCAM_DETECT_DELAY, delay));

    CTC_DKIT_PRINT_DEBUG("\n");

    if (CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block))
    {
        for (i = 0; i < TABLE_MAX_INDEX(lchip, FlowTcamBistReqFA_t); i++)
        {
            sal_memset(&ds, 0, sizeof(ds_t));
            cmd = DRV_IOW(FlowTcamBistReqFA_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &ds);
        }

        for (i = 0; i < TABLE_MAX_INDEX(lchip, FlowTcamBistResultFA_t); i++)
        {
            sal_memset(&ds, 0, sizeof(ds_t));
            cmd = DRV_IOW(FlowTcamBistResultFA_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &ds);
        }
    }
    else if (CTC_DKITS_TCAM_IP_TCAM_BLOCK(block) || CTC_DKITS_TCAM_NAT_TCAM_BLOCK(block))
    {
        for (i = 0; i < TABLE_MAX_INDEX(lchip, LpmTcamBistReq0FA_t); i++)
        {
            sal_memset(&ds, 0, sizeof(ds_t));
            cmd = DRV_IOW(LpmTcamBistReq0FA_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &ds);
        }

        for (i = 0; i < TABLE_MAX_INDEX(lchip, LpmTcamBistReq1FA_t); i++)
        {
            sal_memset(&ds, 0, sizeof(ds_t));
            cmd = DRV_IOW(LpmTcamBistReq1FA_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &ds);
        }

        for (i = 0; i < TABLE_MAX_INDEX(lchip, LpmTcamBistReq2FA_t); i++)
        {
            sal_memset(&ds, 0, sizeof(ds_t));
            cmd = DRV_IOW(LpmTcamBistReq2FA_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &ds);
        }

        for (i = 0; i < TABLE_MAX_INDEX(lchip, LpmTcamBistResult0FA_t); i++)
        {
            sal_memset(&ds, 0, sizeof(ds_t));
            cmd = DRV_IOW(LpmTcamBistResult0FA_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &ds);
        }

        for (i = 0; i < TABLE_MAX_INDEX(lchip, LpmTcamBistResult1FA_t); i++)
        {
            sal_memset(&ds, 0, sizeof(ds_t));
            cmd = DRV_IOW(LpmTcamBistResult1FA_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &ds);
        }

        for (i = 0; i < TABLE_MAX_INDEX(lchip, LpmTcamBistResult2FA_t); i++)
        {
            sal_memset(&ds, 0, sizeof(ds_t));
            cmd = DRV_IOW(LpmTcamBistResult2FA_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &ds);
        }
    }
    else if (DKITS_TCAM_BLOCK_TYPE_CATEGORYID == block)
    {
        for (i = 0; i < TABLE_MAX_INDEX(lchip, IpeCidTcamBistReq_t); i++)
        {
            sal_memset(&ds, 0, sizeof(ds_t));
            cmd = DRV_IOW(IpeCidTcamBistReq_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &ds);
        }

        for (i = 0; i < TABLE_MAX_INDEX(lchip, IpeCidTcamBistResult_t); i++)
        {
            sal_memset(&ds, 0, sizeof(ds_t));
            cmd = DRV_IOW(IpeCidTcamBistResult_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &ds);
        }
    }
    else if (DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE == block)
    {
        for (i = 0; i < TABLE_MAX_INDEX(lchip, QMgrEnqTcamBistReqFa_t); i++)
        {
            sal_memset(&ds, 0, sizeof(ds_t));
            cmd = DRV_IOW(QMgrEnqTcamBistReqFa_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &ds);
        }

        for (i = 0; i < TABLE_MAX_INDEX(lchip, QMgrEnqTcamBistResultFa_t); i++)
        {
            sal_memset(&ds, 0, sizeof(ds_t));
            cmd = DRV_IOW(QMgrEnqTcamBistResultFa_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &ds);
        }
    }

    return DRV_E_NONE;
}

int32
ctc_usw_dkits_tcam_entry_offset(uint8 lchip, tbls_id_t tblid, uint32 tbl_idx, uint32* p_offset, uint32* p_tcam_blk)
{
    uint32 is_sec_half = 0;
    uint32 idx = 0;
    uint32 block = 0;
    uint32 entry_offset = 0;

    CTC_DKITS_TCAM_ERROR_RETURN(drv_usw_ftm_get_entry_num(lchip, tblid, &idx));
    if (0 == idx)
    {
        CTC_DKIT_PRINT_DEBUG("%s %d, %s is not allocate.\n", __FUNCTION__, __LINE__, TABLE_NAME(lchip, tblid));
        return CLI_SUCCESS;
    }

    _ctc_usw_dkits_tcam_tbl2block(tblid, &block);

    if (CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block))
    {
        drv_usw_chip_flow_tcam_get_blknum_index(lchip, tblid, tbl_idx, p_tcam_blk, &idx, &is_sec_half);

        /* unit is 3w */
        *p_offset = idx * (TCAM_KEY_SIZE(lchip, tblid)/DRV_BYTES_PER_ENTRY);
    }
    else if (CTC_DKITS_TCAM_IP_TCAM_BLOCK(block) || CTC_DKITS_TCAM_NAT_TCAM_BLOCK(block))
    {
        _ctc_usw_dkits_tcam_get_lpm_tcam_blknum_index(lchip, tblid, tbl_idx, p_tcam_blk, &idx);
        _ctc_usw_dkits_tcam_lpm_block2offset(*p_tcam_blk, &entry_offset);
         /*drv_chip_lpm_tcam_get_blknum_index(lchip, tblid, tbl_idx, p_tcam_blk, &idx);*/

        /* unit is 2w */
        *p_offset = (idx * (TCAM_KEY_SIZE(lchip, tblid)/DRV_LPM_KEY_BYTES_PER_ENTRY)) + entry_offset;
    }
    else if ((DKITS_TCAM_BLOCK_TYPE_CATEGORYID == block) || (DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE == block))
    {
        *p_offset = tbl_idx;
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_dkits_tcam_mem2tbl(uint8 lchip, uint32 mem_id, uint32 key_type, uint32* p_tblid)
{
    dkits_tcam_block_type_t     block = DKITS_TCAM_BLOCK_TYPE_INVALID;
    dkits_tcam_lpm_class_type_t class = DKITS_TCAM_LPM_CLASS_INVALID;

    *p_tblid = MaxTblId_t;
    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_tbl2block(mem_id, &block));

    if (DsUserId0TcamKey80_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_SCL_KEY_USERID_3W == key_type) ? DsUserId0TcamKey80_t : MaxTblId_t;
    }
    else if (DsUserId1TcamKey80_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_SCL_KEY_USERID_3W == key_type) ? DsUserId1TcamKey80_t : MaxTblId_t;
    }
    else if (DsScl0L3Key160_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_SCL_KEY_USERID_6W == key_type) ? DsUserId0TcamKey160_t
                   : ((DKITS_TCAM_SCL_KEY_L2 == key_type) ? DsScl0MacKey160_t
                   : ((DKITS_TCAM_SCL_KEY_L3_IPV4 == key_type) ? DsScl0L3Key160_t : MaxTblId_t));
    }
    else if (DsScl1L3Key160_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_SCL_KEY_USERID_6W == key_type) ? DsUserId1TcamKey160_t
                   : ((DKITS_TCAM_SCL_KEY_L2 == key_type) ? DsScl1MacKey160_t
                   : ((DKITS_TCAM_SCL_KEY_L3_IPV4 == key_type) ? DsScl1L3Key160_t : MaxTblId_t));
    }
    else if (DsScl0Ipv6Key320_t == mem_id)
    {
         *p_tblid = (DKITS_TCAM_SCL_KEY_USERID_12W == key_type) ? DsUserId0TcamKey320_t
                    : ((DKITS_TCAM_SCL_KEY_L3_IPV6 == key_type) ? DsScl0Ipv6Key320_t
                    : ((DKITS_TCAM_SCL_KEY_L2L3_IPV6 == key_type) ? DsScl0MacL3Key320_t : MaxTblId_t));
    }
    else if (DsScl1Ipv6Key320_t == mem_id)
    {
         *p_tblid = (DKITS_TCAM_SCL_KEY_USERID_12W == key_type) ? DsUserId1TcamKey320_t
                    : ((DKITS_TCAM_SCL_KEY_L3_IPV6 == key_type) ? DsScl1Ipv6Key320_t
                    : ((DKITS_TCAM_SCL_KEY_L2L3_IPV6 == key_type) ? DsScl1MacL3Key320_t : MaxTblId_t));
    }
    else if (DsScl0MacIpv6Key640_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_SCL_KEY_L2L3_IPV6 == key_type) ? DsScl0MacIpv6Key640_t : MaxTblId_t;
    }
    else if (DsScl1MacIpv6Key640_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_SCL_KEY_L2L3_IPV6 == key_type) ? DsScl1MacIpv6Key640_t : MaxTblId_t;
    }
    else if (DsAclQosL3Key160Ing0_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_MAC160 == key_type) ? DsAclQosMacKey160Ing0_t
                   : ((DKITS_TCAM_ACL_KEY_L3160 == key_type) ? DsAclQosL3Key160Ing0_t
                   : ((DKITS_TCAM_ACL_KEY_CID160 == key_type) ? DsAclQosCidKey160Ing0_t : MaxTblId_t));
    }
    else if (DsAclQosL3Key160Ing1_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_MAC160 == key_type) ? DsAclQosMacKey160Ing1_t
                   : ((DKITS_TCAM_ACL_KEY_L3160 == key_type) ? DsAclQosL3Key160Ing1_t
                   : ((DKITS_TCAM_ACL_KEY_CID160 == key_type) ? DsAclQosCidKey160Ing1_t : MaxTblId_t));
    }
    else if (DsAclQosL3Key160Ing2_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_MAC160 == key_type) ? DsAclQosMacKey160Ing2_t
                   : ((DKITS_TCAM_ACL_KEY_L3160 == key_type) ? DsAclQosL3Key160Ing2_t
                   : ((DKITS_TCAM_ACL_KEY_CID160 == key_type) ? DsAclQosCidKey160Ing2_t : MaxTblId_t));
    }
    else if (DsAclQosL3Key160Ing3_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_MAC160 == key_type) ? DsAclQosMacKey160Ing3_t
                   : ((DKITS_TCAM_ACL_KEY_L3160 == key_type) ? DsAclQosL3Key160Ing3_t
                   : ((DKITS_TCAM_ACL_KEY_CID160 == key_type) ? DsAclQosCidKey160Ing3_t : MaxTblId_t));
    }
    else if (DsAclQosL3Key160Ing4_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_MAC160 == key_type) ? DsAclQosMacKey160Ing4_t
                   : ((DKITS_TCAM_ACL_KEY_L3160 == key_type) ? DsAclQosL3Key160Ing4_t
                   : ((DKITS_TCAM_ACL_KEY_CID160 == key_type) ? DsAclQosCidKey160Ing4_t : MaxTblId_t));
    }
    else if (DsAclQosL3Key160Ing5_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_MAC160 == key_type) ? DsAclQosMacKey160Ing5_t
                   : ((DKITS_TCAM_ACL_KEY_L3160 == key_type) ? DsAclQosL3Key160Ing5_t
                   : ((DKITS_TCAM_ACL_KEY_CID160 == key_type) ? DsAclQosCidKey160Ing5_t : MaxTblId_t));
    }
    else if (DsAclQosL3Key160Ing6_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_MAC160 == key_type) ? DsAclQosMacKey160Ing6_t
                   : ((DKITS_TCAM_ACL_KEY_L3160 == key_type) ? DsAclQosL3Key160Ing6_t
                   : ((DKITS_TCAM_ACL_KEY_CID160 == key_type) ? DsAclQosCidKey160Ing6_t : MaxTblId_t));
    }
    else if (DsAclQosL3Key160Ing7_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_MAC160 == key_type) ? DsAclQosMacKey160Ing7_t
                   : ((DKITS_TCAM_ACL_KEY_L3160 == key_type) ? DsAclQosL3Key160Ing7_t
                   : ((DKITS_TCAM_ACL_KEY_CID160 == key_type) ? DsAclQosCidKey160Ing7_t : MaxTblId_t));
    }
    else if (DsAclQosL3Key320Ing0_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_L3320 == key_type) ? DsAclQosL3Key320Ing0_t
                   : ((DKITS_TCAM_ACL_KEY_IPV6320 == key_type) ? DsAclQosIpv6Key320Ing0_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3320 == key_type) ? DsAclQosMacL3Key320Ing0_t
                   : ((DKITS_TCAM_ACL_KEY_FORWARD320 == key_type) ? DsAclQosForwardKey320Ing0_t : MaxTblId_t)));
    }
    else if (DsAclQosL3Key320Ing1_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_L3320 == key_type) ? DsAclQosL3Key320Ing1_t
                   : ((DKITS_TCAM_ACL_KEY_IPV6320 == key_type) ? DsAclQosIpv6Key320Ing1_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3320 == key_type) ? DsAclQosMacL3Key320Ing1_t
                   : ((DKITS_TCAM_ACL_KEY_FORWARD320 == key_type) ? DsAclQosForwardKey320Ing1_t : MaxTblId_t)));
    }
    else if (DsAclQosL3Key320Ing2_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_L3320 == key_type) ? DsAclQosL3Key320Ing2_t
                   : ((DKITS_TCAM_ACL_KEY_IPV6320 == key_type) ? DsAclQosIpv6Key320Ing2_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3320 == key_type) ? DsAclQosMacL3Key320Ing2_t
                   : ((DKITS_TCAM_ACL_KEY_FORWARD320 == key_type) ? DsAclQosForwardKey320Ing2_t : MaxTblId_t)));
    }
    else if (DsAclQosL3Key320Ing3_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_L3320 == key_type) ? DsAclQosL3Key320Ing3_t
                   : ((DKITS_TCAM_ACL_KEY_IPV6320 == key_type) ? DsAclQosIpv6Key320Ing3_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3320 == key_type) ? DsAclQosMacL3Key320Ing3_t
                   : ((DKITS_TCAM_ACL_KEY_FORWARD320 == key_type) ? DsAclQosForwardKey320Ing3_t : MaxTblId_t)));
    }
    else if (DsAclQosL3Key320Ing4_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_L3320 == key_type) ? DsAclQosL3Key320Ing4_t
                   : ((DKITS_TCAM_ACL_KEY_IPV6320 == key_type) ? DsAclQosIpv6Key320Ing4_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3320 == key_type) ? DsAclQosMacL3Key320Ing4_t
                   : ((DKITS_TCAM_ACL_KEY_FORWARD320 == key_type) ? DsAclQosForwardKey320Ing4_t : MaxTblId_t)));
    }
    else if (DsAclQosL3Key320Ing5_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_L3320 == key_type) ? DsAclQosL3Key320Ing5_t
                   : ((DKITS_TCAM_ACL_KEY_IPV6320 == key_type) ? DsAclQosIpv6Key320Ing5_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3320 == key_type) ? DsAclQosMacL3Key320Ing5_t
                   : ((DKITS_TCAM_ACL_KEY_FORWARD320 == key_type) ? DsAclQosForwardKey320Ing5_t : MaxTblId_t)));
    }
    else if (DsAclQosL3Key320Ing6_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_L3320 == key_type) ? DsAclQosL3Key320Ing6_t
                   : ((DKITS_TCAM_ACL_KEY_IPV6320 == key_type) ? DsAclQosIpv6Key320Ing6_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3320 == key_type) ? DsAclQosMacL3Key320Ing6_t
                   : ((DKITS_TCAM_ACL_KEY_FORWARD320 == key_type) ? DsAclQosForwardKey320Ing6_t : MaxTblId_t)));
    }
    else if (DsAclQosL3Key320Ing7_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_L3320 == key_type) ? DsAclQosL3Key320Ing7_t
                   : ((DKITS_TCAM_ACL_KEY_IPV6320 == key_type) ? DsAclQosIpv6Key320Ing7_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3320 == key_type) ? DsAclQosMacL3Key320Ing7_t
                   : ((DKITS_TCAM_ACL_KEY_FORWARD320 == key_type) ? DsAclQosForwardKey320Ing7_t : MaxTblId_t)));
    }
    else if (DsAclQosIpv6Key640Ing0_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_IPV6640 == key_type) ? DsAclQosIpv6Key640Ing0_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3640 == key_type) ? DsAclQosMacL3Key640Ing0_t
                   : ((DKITS_TCAM_ACL_KEY_MACIPV6640 == key_type) ? DsAclQosMacIpv6Key640Ing0_t
                   : ((DKITS_TCAM_ACL_KEY_FORWARD640 == key_type) ? DsAclQosForwardKey640Ing0_t : MaxTblId_t)));
    }
    else if (DsAclQosIpv6Key640Ing1_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_IPV6640 == key_type) ? DsAclQosIpv6Key640Ing1_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3640 == key_type) ? DsAclQosMacL3Key640Ing1_t
                   : ((DKITS_TCAM_ACL_KEY_MACIPV6640 == key_type) ? DsAclQosMacIpv6Key640Ing1_t
                   : ((DKITS_TCAM_ACL_KEY_FORWARD640 == key_type) ? DsAclQosForwardKey640Ing1_t : MaxTblId_t)));
    }
    else if (DsAclQosIpv6Key640Ing2_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_IPV6640 == key_type) ? DsAclQosIpv6Key640Ing2_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3640 == key_type) ? DsAclQosMacL3Key640Ing2_t
                   : ((DKITS_TCAM_ACL_KEY_MACIPV6640 == key_type) ? DsAclQosMacIpv6Key640Ing2_t
                   : ((DKITS_TCAM_ACL_KEY_FORWARD640 == key_type) ? DsAclQosForwardKey640Ing2_t : MaxTblId_t)));
    }
    else if (DsAclQosIpv6Key640Ing3_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_IPV6640 == key_type) ? DsAclQosIpv6Key640Ing3_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3640 == key_type) ? DsAclQosMacL3Key640Ing3_t
                   : ((DKITS_TCAM_ACL_KEY_MACIPV6640 == key_type) ? DsAclQosMacIpv6Key640Ing3_t
                   : ((DKITS_TCAM_ACL_KEY_FORWARD640 == key_type) ? DsAclQosForwardKey640Ing3_t : MaxTblId_t)));
    }
    else if (DsAclQosIpv6Key640Ing4_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_IPV6640 == key_type) ? DsAclQosIpv6Key640Ing4_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3640 == key_type) ? DsAclQosMacL3Key640Ing4_t
                   : ((DKITS_TCAM_ACL_KEY_MACIPV6640 == key_type) ? DsAclQosMacIpv6Key640Ing4_t
                   : ((DKITS_TCAM_ACL_KEY_FORWARD640 == key_type) ? DsAclQosForwardKey640Ing4_t : MaxTblId_t)));
    }
    else if (DsAclQosIpv6Key640Ing5_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_IPV6640 == key_type) ? DsAclQosIpv6Key640Ing5_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3640 == key_type) ? DsAclQosMacL3Key640Ing5_t
                   : ((DKITS_TCAM_ACL_KEY_MACIPV6640 == key_type) ? DsAclQosMacIpv6Key640Ing5_t
                   : ((DKITS_TCAM_ACL_KEY_FORWARD640 == key_type) ? DsAclQosForwardKey640Ing5_t : MaxTblId_t)));
    }
    else if (DsAclQosIpv6Key640Ing6_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_IPV6640 == key_type) ? DsAclQosIpv6Key640Ing6_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3640 == key_type) ? DsAclQosMacL3Key640Ing6_t
                   : ((DKITS_TCAM_ACL_KEY_MACIPV6640 == key_type) ? DsAclQosMacIpv6Key640Ing6_t
                   : ((DKITS_TCAM_ACL_KEY_FORWARD640 == key_type) ? DsAclQosForwardKey640Ing6_t : MaxTblId_t)));
    }
    else if (DsAclQosIpv6Key640Ing7_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_IPV6640 == key_type) ? DsAclQosIpv6Key640Ing7_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3640 == key_type) ? DsAclQosMacL3Key640Ing7_t
                   : ((DKITS_TCAM_ACL_KEY_MACIPV6640 == key_type) ? DsAclQosMacIpv6Key640Ing7_t
                   : ((DKITS_TCAM_ACL_KEY_FORWARD640 == key_type) ? DsAclQosForwardKey640Ing7_t : MaxTblId_t)));
    }
    else if (DsAclQosKey80Egr0_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_SHORT80 == key_type) ? DsAclQosKey80Egr0_t : MaxTblId_t;
    }
    else if (DsAclQosKey80Egr1_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_SHORT80 == key_type) ? DsAclQosKey80Egr1_t : MaxTblId_t;
    }
    else if (DsAclQosKey80Egr2_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_SHORT80 == key_type) ? DsAclQosKey80Egr2_t : MaxTblId_t;
    }
    else if (DsAclQosL3Key160Egr0_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_MAC160 == key_type) ? DsAclQosMacKey160Egr0_t
                   : (DKITS_TCAM_ACL_KEY_L3160 == key_type) ? DsAclQosL3Key160Egr0_t : MaxTblId_t;
    }
    else if (DsAclQosL3Key160Egr1_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_MAC160 == key_type) ? DsAclQosMacKey160Egr1_t
                   : (DKITS_TCAM_ACL_KEY_L3160 == key_type) ? DsAclQosL3Key160Egr1_t : MaxTblId_t;
    }
    else if (DsAclQosL3Key160Egr2_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_MAC160 == key_type) ? DsAclQosMacKey160Egr2_t
                   : (DKITS_TCAM_ACL_KEY_L3160 == key_type) ? DsAclQosL3Key160Egr2_t : MaxTblId_t;
    }
    else if (DsAclQosL3Key320Egr0_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_L3320 == key_type) ? DsAclQosL3Key320Egr0_t
                   : ((DKITS_TCAM_ACL_KEY_IPV6320 == key_type) ? DsAclQosIpv6Key320Egr0_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3320 == key_type) ? DsAclQosMacL3Key320Egr0_t : MaxTblId_t));
    }
    else if (DsAclQosL3Key320Egr1_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_L3320 == key_type) ? DsAclQosL3Key320Egr1_t
                   : ((DKITS_TCAM_ACL_KEY_IPV6320 == key_type) ? DsAclQosIpv6Key320Egr1_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3320 == key_type) ? DsAclQosMacL3Key320Egr1_t : MaxTblId_t));
    }
    else if (DsAclQosL3Key320Egr2_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_L3320 == key_type) ? DsAclQosL3Key320Egr2_t
                   : ((DKITS_TCAM_ACL_KEY_IPV6320 == key_type) ? DsAclQosIpv6Key320Egr2_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3320 == key_type) ? DsAclQosMacL3Key320Egr2_t : MaxTblId_t));
    }
    else if (DsAclQosIpv6Key640Egr0_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_IPV6640 == key_type) ? DsAclQosIpv6Key640Egr0_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3640 == key_type) ? DsAclQosMacL3Key640Egr0_t
                   : ((DKITS_TCAM_ACL_KEY_MACIPV6640 == key_type) ? DsAclQosMacIpv6Key640Egr0_t : MaxTblId_t));
    }
    else if (DsAclQosIpv6Key640Egr1_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_IPV6640 == key_type) ? DsAclQosIpv6Key640Egr1_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3640 == key_type) ? DsAclQosMacL3Key640Egr1_t
                   : ((DKITS_TCAM_ACL_KEY_MACIPV6640 == key_type) ? DsAclQosMacIpv6Key640Egr1_t : MaxTblId_t));
    }
    else if (DsAclQosIpv6Key640Egr2_t == mem_id)
    {
        *p_tblid = (DKITS_TCAM_ACL_KEY_IPV6640 == key_type) ? DsAclQosIpv6Key640Egr2_t
                   : ((DKITS_TCAM_ACL_KEY_MACL3640 == key_type) ? DsAclQosMacL3Key640Egr2_t
                   : ((DKITS_TCAM_ACL_KEY_MACIPV6640 == key_type) ? DsAclQosMacIpv6Key640Egr2_t : MaxTblId_t));
    }
    else if ((DKITS_TCAM_BLOCK_TYPE_LPM_LKP0 == block) || (DKITS_TCAM_BLOCK_TYPE_LPM_LKP1 == block)
            || (DKITS_TCAM_BLOCK_TYPE_NAT_PBR == block))
    {
        _ctc_usw_dkits_tcam_tbl2class(mem_id, &class);

        switch (class)
        {
        case DKITS_TCAM_LPM_CLASS_IGS_LPM0:
            /* DsLpmTcamIpv6SingleKey_t ? */
            *p_tblid = (DKITS_TCAM_IP_KEY_IPV4UC == key_type) ? DsLpmTcamIpv4HalfKey_t : DsLpmTcamIpv6DoubleKey0_t;
            break;

        case DKITS_TCAM_LPM_CLASS_IGS_LPM0SAPRI:
            /* DsLpmTcamIpv6SaSingleKey_t ? */
            *p_tblid = (DKITS_TCAM_IP_KEY_IPV4UC == key_type) ? DsLpmTcamIpv4SaHalfKeyLookup1_t : DsLpmTcamIpv6SaDoubleKey0Lookup1_t;
            break;

        case DKITS_TCAM_LPM_CLASS_IGS_LPM0DAPUB:
            /* DsLpmTcamIpv6DaPubSingleKey_t ? */
            *p_tblid = (DKITS_TCAM_IP_KEY_IPV4UC == key_type) ? DsLpmTcamIpv4DaPubHalfKeyLookup1_t : DsLpmTcamIpv6DaPubDoubleKey0Lookup1_t;
            break;

        case DKITS_TCAM_LPM_CLASS_IGS_LPM0SAPUB:
            /* DsLpmTcamIpv6SaPubSingleKey_t ? */
            *p_tblid = (DKITS_TCAM_IP_KEY_IPV4UC == key_type) ? DsLpmTcamIpv4SaPubHalfKeyLookup1_t : DsLpmTcamIpv6SaPubDoubleKey0Lookup1_t;
            break;

        case DKITS_TCAM_LPM_CLASS_IGS_LPM0_LK2:
            *p_tblid = (DKITS_TCAM_IP_KEY_IPV4UC == key_type) ? DsLpmTcamIpv4HalfKeyLookup2_t : DsLpmTcamIpv6DoubleKey0Lookup2_t;
            break;

        case DKITS_TCAM_LPM_CLASS_IGS_LPM0SAPRI_LK2:
            *p_tblid = (DKITS_TCAM_IP_KEY_IPV4UC == key_type) ? DsLpmTcamIpv4SaHalfKeyLookup2_t : DsLpmTcamIpv6SaDoubleKey0Lookup2_t;
            break;

        case DKITS_TCAM_LPM_CLASS_IGS_LPM0DAPUB_LK2:
            *p_tblid = (DKITS_TCAM_IP_KEY_IPV4UC == key_type) ? DsLpmTcamIpv4DaPubHalfKeyLookup2_t : DsLpmTcamIpv6DaPubDoubleKey0Lookup2_t;
            break;

        case DKITS_TCAM_LPM_CLASS_IGS_LPM0SAPUB_LK2:
            *p_tblid = (DKITS_TCAM_IP_KEY_IPV4UC == key_type) ? DsLpmTcamIpv4SaPubHalfKeyLookup2_t : DsLpmTcamIpv6SaPubDoubleKey0Lookup2_t;
            break;

        case DKITS_TCAM_LPM_CLASS_IGS_NAT_PBR:
            switch (key_type)
            {
            case DKITS_TCAM_NATPBR_KEY_IPV4PBR:
                *p_tblid = DsLpmTcamIpv4PbrDoubleKey_t;
                break;

            case DKITS_TCAM_NATPBR_KEY_IPV6PBR:
                *p_tblid = DsLpmTcamIpv6QuadKey_t;
                break;

            case DKITS_TCAM_NATPBR_KEY_IPV4NAT:
                *p_tblid = DsLpmTcamIpv4NatDoubleKey_t;
                break;

            case DKITS_TCAM_NATPBR_KEY_IPV6NAT:
                *p_tblid = DsLpmTcamIpv6DoubleKey1_t;
                break;
            }
            break;
        default:
            CTC_DKIT_PRINT("@@ DKIT ERROR! %s %u %s Invalid Scan Tcam Tbl :%s.\n",
                           __FILE__, __LINE__, __FUNCTION__, TABLE_NAME(lchip, mem_id));
            break;
        }
    }
    else if (DsCategoryIdPairTcamKey_t == mem_id)
    {
        *p_tblid = DsCategoryIdPairTcamKey_t;
    }
    else if (DsQueueMapTcamKey_t == mem_id)
    {
        *p_tblid = DsQueueMapTcamKey_t;
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_dkits_tcam_parser_key_type(uint8 lchip, tbls_id_t tblid, tbl_entry_t* p_tcam_key, uint32* p_tcam_key_type, uint32* p_ad_type)
{
    uint32 hash_type = 0, is_ipv6 = 0;

    if (DKITS_TCAM_IGS_SCL0_KEY(tblid) || DKITS_TCAM_IGS_SCL1_KEY(tblid)
        || DKITS_TCAM_IGS_SCL2_KEY(tblid) || DKITS_TCAM_IGS_SCL3_KEY(tblid))
    {
        if (DKITS_TCAM_IGS_SCL0_CFL(tblid) || DKITS_TCAM_IGS_SCL1_CFL(tblid))
        {
            hash_type = GetDsUserId0TcamKey80(V, hashType_f, p_tcam_key->data_entry);
            hash_type &= GetDsUserId0TcamKey80(V, hashType_f, p_tcam_key->mask_entry);

            switch (hash_type)
            {
            case USERIDHASHTYPE_DOUBLEVLANPORT:
            case USERIDHASHTYPE_CVLANPORT:
            case USERIDHASHTYPE_SVLANPORT:
            case USERIDHASHTYPE_PORT:
            case USERIDHASHTYPE_CVLANCOSPORT:
            case USERIDHASHTYPE_SVLANCOSPORT:
            case USERIDHASHTYPE_MAC:
            case USERIDHASHTYPE_MACPORT:
            case USERIDHASHTYPE_IPV4SA:
            case USERIDHASHTYPE_IPV4PORT:
            case USERIDHASHTYPE_TUNNELIPV4GREKEY:
            case USERIDHASHTYPE_TUNNELIPV4RPF:
            case USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE0:
            case USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE1:
            case USERIDHASHTYPE_TUNNELIPV4MCNVGREMODE0:
            case USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE0:
            case USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE1:
            case USERIDHASHTYPE_TUNNELIPV4MCVXLANMODE0:
            case USERIDHASHTYPE_TUNNELIPV4CAPWAP:
            case USERIDHASHTYPE_TUNNELCAPWAPRMAC:
            case USERIDHASHTYPE_TUNNELCAPWAPRMACRID:
            case USERIDHASHTYPE_CAPWAPSTASTATUS:
            case USERIDHASHTYPE_CAPWAPSTASTATUSMC:
            case USERIDHASHTYPE_CAPWAPVLANFORWARD:
            case USERIDHASHTYPE_CAPWAPMACDAFORWARD:
            case USERIDHASHTYPE_TUNNELMPLS:
                *p_tcam_key_type = DKITS_TCAM_SCL_KEY_USERID_3W;
                break;

            /*160*/
            case USERIDHASHTYPE_IPV6SA:
            case USERIDHASHTYPE_TUNNELIPV4:
            case USERIDHASHTYPE_TUNNELIPV4NVGREMODE1:
            case USERIDHASHTYPE_TUNNELIPV4VXLANMODE1:
                *p_tcam_key_type = DKITS_TCAM_SCL_KEY_USERID_6W;
                break;

            /*320*/
            case USERIDHASHTYPE_IPV6PORT:
            case USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE0:
            case USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE1:
            case USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE0:
            case USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE1:
            case USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE0:
            case USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE1:
            case USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE0:
            case USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE1:
            case USERIDHASHTYPE_TUNNELIPV6CAPWAP:
                *p_tcam_key_type = DKITS_TCAM_SCL_KEY_USERID_12W;
                break;

            default:
                if(hash_type == DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA))
                {
                    *p_tcam_key_type = DKITS_TCAM_SCL_KEY_USERID_3W;
                }
                else if(hash_type == DRV_ENUM(DRV_USERID_HASHTYPE_SCLFLOWL2))
                {
                    *p_tcam_key_type = DKITS_TCAM_SCL_KEY_USERID_6W;
                }
                else
                {
                    *p_tcam_key_type = DKITS_TCAM_SCL_KEY_NUM;
                }
                break;
            }

            switch (hash_type)
            {
            /* The key contains port info be decide ad type by DsPhyPortExt's tcam1IsSclFlow. */
            case USERIDHASHTYPE_DOUBLEVLANPORT:
            case USERIDHASHTYPE_SVLANPORT:
            case USERIDHASHTYPE_CVLANPORT:
            case USERIDHASHTYPE_SVLANCOSPORT:
            case USERIDHASHTYPE_CVLANCOSPORT:
            case USERIDHASHTYPE_MACPORT:
            case USERIDHASHTYPE_IPV4PORT:
            case USERIDHASHTYPE_PORT:
            case USERIDHASHTYPE_IPV6PORT:

            case USERIDHASHTYPE_MAC:
            case USERIDHASHTYPE_IPV4SA:
            case USERIDHASHTYPE_SVLANMACSA:
            case USERIDHASHTYPE_SVLAN:
            case USERIDHASHTYPE_ECIDNAMESPACE:
            case USERIDHASHTYPE_INGECIDNAMESPACE:
            case USERIDHASHTYPE_IPV6SA:
            case USERIDHASHTYPE_CAPWAPSTASTATUS:
            case USERIDHASHTYPE_CAPWAPSTASTATUSMC:
            case USERIDHASHTYPE_CAPWAPMACDAFORWARD:
            case USERIDHASHTYPE_CAPWAPVLANFORWARD:
                *p_ad_type = DKITS_TCAM_SCL_AD_USERID;
                break;

            case USERIDHASHTYPE_TUNNELIPV4:
            case USERIDHASHTYPE_TUNNELIPV4GREKEY:
            case USERIDHASHTYPE_TUNNELIPV4UDP:
            case USERIDHASHTYPE_TUNNELPBB:
            case USERIDHASHTYPE_TUNNELTRILLUCRPF:
            case USERIDHASHTYPE_TUNNELTRILLUCDECAP:
            case USERIDHASHTYPE_TUNNELTRILLMCRPF:
            case USERIDHASHTYPE_TUNNELTRILLMCDECAP:
            case USERIDHASHTYPE_TUNNELTRILLMCADJ:
            case USERIDHASHTYPE_TUNNELIPV4RPF:
            case USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE0:
            case USERIDHASHTYPE_TUNNELIPV4UCVXLANMODE1:
            case USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE0:
            case USERIDHASHTYPE_TUNNELIPV6UCVXLANMODE1:
            case USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE0:
            case USERIDHASHTYPE_TUNNELIPV4UCNVGREMODE1:
            case USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE0:
            case USERIDHASHTYPE_TUNNELIPV6UCNVGREMODE1:
            case USERIDHASHTYPE_TUNNELIPV4MCVXLANMODE0:
            case USERIDHASHTYPE_TUNNELIPV4VXLANMODE1:
            case USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE0:
            case USERIDHASHTYPE_TUNNELIPV6MCVXLANMODE1:
            case USERIDHASHTYPE_TUNNELIPV4MCNVGREMODE0:
            case USERIDHASHTYPE_TUNNELIPV4NVGREMODE1:
            case USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE0:
            case USERIDHASHTYPE_TUNNELIPV6MCNVGREMODE1:
            case USERIDHASHTYPE_TUNNELIPV4CAPWAP:
            case USERIDHASHTYPE_TUNNELIPV6CAPWAP:
            case USERIDHASHTYPE_TUNNELCAPWAPRMAC:
            case USERIDHASHTYPE_TUNNELCAPWAPRMACRID:
            case USERIDHASHTYPE_TUNNELMPLS:
                *p_ad_type = DKITS_TCAM_SCL_AD_TUNNEL;
                break;
            default:
                if(hash_type == DRV_ENUM(DRV_USERID_HASHTYPE_TUNNELIPV4DA))
                {
                    *p_ad_type = DKITS_TCAM_SCL_AD_TUNNEL;
                }
                else if(hash_type == DRV_ENUM(DRV_USERID_HASHTYPE_SCLFLOWL2))
                {
                    *p_ad_type = DKITS_TCAM_SCL_AD_FLOW;
                }
                else
                {
                    *p_ad_type = DKITS_TCAM_SCL_AD_NUM;
                }
            }
        }
        else
        {
            *p_tcam_key_type = GetDsScl0MacKey160(V, sclKeyType0_f, p_tcam_key->data_entry);
            *p_tcam_key_type &= GetDsScl0MacKey160(V, sclKeyType0_f, p_tcam_key->mask_entry);

            if (CTC_DKITS_TCAM_SCL_KEY_DRV_TYPE_L3KEY == *p_tcam_key_type)
            {
                is_ipv6  = GetDsScl0L3Key160(V, isIpv6Key_f, p_tcam_key->data_entry);
                is_ipv6 &= GetDsScl0L3Key160(V, isIpv6Key_f, p_tcam_key->mask_entry);
                *p_tcam_key_type = (is_ipv6) ? DKITS_TCAM_SCL_KEY_L3_IPV6 : DKITS_TCAM_SCL_KEY_L3_IPV4;
            }
            else if (CTC_DKITS_TCAM_SCL_KEY_DRV_TYPE_L2L3KEY == *p_tcam_key_type)
            {
                is_ipv6  = GetDsScl1MacL3Key320(V, isIpv6Key_f, p_tcam_key->data_entry);
                is_ipv6 &= GetDsScl1MacL3Key320(V, isIpv6Key_f, p_tcam_key->mask_entry);
                *p_tcam_key_type = (is_ipv6) ? DKITS_TCAM_SCL_KEY_L2L3_IPV6 : DKITS_TCAM_SCL_KEY_L2L3_IPV4;
            }
            *p_ad_type = GetDsScl0MacKey160(V, _type_f, p_tcam_key->data_entry);
            *p_ad_type &= GetDsScl0MacKey160(V, _type_f, p_tcam_key->mask_entry);
        }
    }
    else if (DKITS_TCAM_IGS_ACL0_KEY(tblid) || DKITS_TCAM_IGS_ACL1_KEY(tblid)
            || DKITS_TCAM_IGS_ACL2_KEY(tblid) || DKITS_TCAM_IGS_ACL3_KEY(tblid)
            || DKITS_TCAM_IGS_ACL4_KEY(tblid) || DKITS_TCAM_IGS_ACL5_KEY(tblid)
            || DKITS_TCAM_IGS_ACL6_KEY(tblid) || DKITS_TCAM_IGS_ACL7_KEY(tblid)
            || DKITS_TCAM_EGS_ACL0_KEY(tblid) || DKITS_TCAM_EGS_ACL1_KEY(tblid)
            || DKITS_TCAM_EGS_ACL2_KEY(tblid))
    {
        if ((DsAclQosKey80Ing0_t == tblid) || (DsAclQosKey80Ing1_t == tblid)
           || (DsAclQosKey80Ing2_t == tblid) || (DsAclQosKey80Ing3_t == tblid)
           || (DsAclQosKey80Ing4_t == tblid) || (DsAclQosKey80Ing5_t == tblid)
           || (DsAclQosKey80Ing6_t == tblid) || (DsAclQosKey80Ing7_t == tblid)
           || (DsAclQosKey80Egr0_t == tblid) || (DsAclQosKey80Egr1_t == tblid)
           || (DsAclQosKey80Egr2_t == tblid) || (DsAclQosCoppKey640Ing0_t == tblid)
           || (DsAclQosCoppKey640Ing1_t == tblid) || (DsAclQosCoppKey640Ing2_t == tblid)
           || (DsAclQosCoppKey640Ing3_t == tblid) || (DsAclQosCoppKey640Ing4_t == tblid)
           || (DsAclQosCoppKey640Ing5_t == tblid) || (DsAclQosCoppKey640Ing6_t == tblid)
           || (DsAclQosCoppKey640Ing7_t == tblid))
        {
            /*cannot get correct 80bit key_type from key, not support 80bit key bist*/
            *p_tcam_key_type = DKITS_TCAM_ACL_KEY_NUM;
        }
        else
        {
            *p_tcam_key_type = GetDsAclQosMacKey160Ing0(V, aclQosKeyType_f, p_tcam_key->data_entry);
            *p_tcam_key_type &= GetDsAclQosMacKey160Ing0(V, aclQosKeyType_f, p_tcam_key->mask_entry);
        }
    }
    else if (DKITS_TCAM_IPLKP0_KEY(tblid) || DKITS_TCAM_IPLKP1_KEY(tblid))
    {
        *p_tcam_key_type = GetDsLpmTcamIpv4HalfKey(V, lpmTcamKeyType_f, p_tcam_key->data_entry);
        *p_tcam_key_type &= GetDsLpmTcamIpv4HalfKey(V, lpmTcamKeyType_f, p_tcam_key->mask_entry);
    }
    else if (DKITS_TCAM_NATPBR_KEY(tblid))
    {
        *p_tcam_key_type = GetDsLpmTcamIpv4PbrDoubleKey(V, lpmTcamKeyType_f, p_tcam_key->data_entry);
        *p_tcam_key_type &= GetDsLpmTcamIpv4PbrDoubleKey(V, lpmTcamKeyType_f, p_tcam_key->mask_entry);
    }
    else if (DsCategoryIdPairTcamKey_t == tblid)
    {
        *p_tcam_key_type = 0;
    }
    else if (DsQueueMapTcamKey_t == tblid)
    {
        *p_tcam_key_type = 0;
    }
    else
    {
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_dkits_tcam_read_tcam_type(uint8 lchip, tbls_id_t tbl_id, uint32 idx, uint32* p_key_type)
{
    int32       ret = 0;
    uint32      cmd = 0;
    uint32      ad_type = 0;
    tbl_entry_t tcam_key;
    ds_t        data;
    ds_t        mask;

    sal_memset(&tcam_key, 0, sizeof(tcam_key));
    sal_memset(&data, 0, sizeof(data));
    sal_memset(&mask, 0, sizeof(mask));

    tcam_key.data_entry = (uint32 *) &data;
    tcam_key.mask_entry = (uint32 *) &mask;

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, idx, cmd, &tcam_key);

    ret = ctc_usw_dkits_tcam_parser_key_type(lchip, tbl_id, &tcam_key, p_key_type, &ad_type);

    return ret;
}

int32
ctc_usw_dkits_tcam_read_tcam_key(uint8 lchip, tbls_id_t tbl_id, uint32 index, uint32* p_empty, tbl_entry_t* p_tcam_key)
{
    int32  ret = 0;
    uint32 cmd = 0;
    ds_t mask_entry_tmp;
    sal_memset(p_tcam_key->mask_entry, 0, sizeof(ds_t));
    sal_memset(&mask_entry_tmp, 0, sizeof(ds_t));

    if (TABLE_MAX_INDEX(lchip, tbl_id) <= index)
    {
        CTC_DKIT_PRINT("@@ DKIT ERROR! %s %d %s chip-0x%x, tbl-%d, index-0x%x exceeds the max_index-0x%x.\n",
                        __FILE__, __LINE__, __FUNCTION__, lchip, tbl_id, index, TABLE_MAX_INDEX(lchip, tbl_id));
        return CLI_ERROR;
    }

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, index, cmd, p_tcam_key);
    if (ret < 0)
    {
        CTC_DKIT_PRINT("Read tcam key %s:%u error!\n", TABLE_NAME(lchip, tbl_id), index);
        return CLI_ERROR;
    }

    if (0 == SDK_WORK_PLATFORM)
    {
        _ctc_usw_dkits_tcam_tbl_empty(lchip, tbl_id, index, p_empty);
    }
    else /*uml*/
    {
        if (sal_memcmp((uint8*)p_tcam_key->mask_entry, (uint8*)&mask_entry_tmp, TCAM_KEY_SIZE(lchip, tbl_id)))
        {
            *p_empty = 0;
        }
        else
        {
            *p_empty = 1;
        }
    }

    return CLI_SUCCESS;
}

#define __2_BIST__

STATIC int32
_ctc_usw_dkits_tcam_bist_start(uint8 lchip, dkits_tcam_block_type_t block, uint8 bist_once_en)
{
    uint32 delay = 0;

    CTC_DKIT_PRINT_DEBUG(" %-52s %u:", __FUNCTION__, __LINE__);
    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_set_detect(lchip, block, DKITS_TCAM_DETECT_BIST_EN, 1));
    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_set_detect(lchip, block, DKITS_TCAM_DETECT_BIST_ONCE,  bist_once_en));
    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_detect_delay(block, &delay, DKITS_TCAM_DETECT_TYPE_BIST));
    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_set_detect(lchip, block, DKITS_TCAM_DETECT_DELAY, delay));
    if (!bist_once_en)
    {
        sal_task_sleep(1); /*delay 1ms for stress bist*/
        CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_set_detect(lchip, block, DKITS_TCAM_DETECT_BIST_EN, 0));
    }
    CTC_DKIT_PRINT_DEBUG("\n");

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_bist_detect(uint8 lchip, dkits_tcam_block_type_t block, uint32* p_done)
{
    uint32 cmd = 0, i = 0, step = 0;

    if (CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block))
    {
        cmd = DRV_IOR(FlowTcamBistStatus_t, FlowTcamBistStatus_bistDone_f);
        DRV_IOCTL(lchip, 0, cmd, p_done);
    }
    else if (CTC_DKITS_TCAM_IP_TCAM_BLOCK(block) || CTC_DKITS_TCAM_NAT_TCAM_BLOCK(block))
    {
        i = block - DKITS_TCAM_BLOCK_TYPE_LPM_LKP0;
        step = LpmTcamBistStatus_daSa1BistDone_f - LpmTcamBistStatus_daSa0BistDone_f;
        cmd = DRV_IOR(LpmTcamBistStatus_t, LpmTcamBistStatus_daSa0BistDone_f + (i * step));
        DRV_IOCTL(lchip, 0, cmd, p_done);
    }
    else if (DKITS_TCAM_BLOCK_TYPE_CATEGORYID == block)
    {
        cmd = DRV_IOR(IpeCidTcamBistStatus_t, IpeCidTcamBistStatus_bistDone_f);
        DRV_IOCTL(lchip, 0, cmd, p_done);
    }
    else if (DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE == block)
    {
        cmd = DRV_IOR(QMgrEnqTcamBistStatus_t, QMgrEnqTcamBistStatus_tcamBistDone_f);
        DRV_IOCTL(lchip, 0, cmd, p_done);
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_bist_tbl_brief_status(tbls_id_t* p_tblid, uint8 is_get)
{
    static tbls_id_t tblid_static = MaxTblId_t;
    if (is_get)
    {
        *p_tblid = tblid_static;
    }
    else/*set tblid_static*/
    {
        if (NULL == p_tblid)
        {
            tblid_static = MaxTblId_t;
        }
        else
        {
            tblid_static = *p_tblid;
        }
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_bist_present_brief(uint8 lchip, uint32 tbl_id, uint32 expect_tbl_idx, uint32 result_tbl_idx)
{
    tbls_id_t tblid = MaxTblId_t;

    if (expect_tbl_idx != result_tbl_idx)
    {
        _ctc_usw_dkits_tcam_bist_tbl_brief_status(&tblid, 0);
         /*CTC_DKIT_PRINT("    Failed table:%s index:0x%04X\n", TABLE_NAME(lchip, tbl_id), expect_tbl_idx);*/
    }
    return CLI_SUCCESS;
}

STATIC void
_ctc_usw_dkits_tcam_bist_show_result(uint8 lchip, tbls_id_t tblid, uint32 expect_tbl_idx, uint32 result_tbl_idx)
{
    _ctc_usw_dkits_tcam_bist_present_brief(lchip, tblid, expect_tbl_idx, result_tbl_idx);
    return;
}

STATIC int32
_ctc_usw_dkits_tcam_bist_present_result(uint8 lchip, tbls_id_t mem_id, uint32 tcam_blk, uint8 entry_num, uint8* mismatch_cnt)
{
    uint8  i = 0;
    uint32 cmd = 0;
    uint32 valid = 0;
    uint32 hit = 0;
    uint32 hit1 = 0;
    uint32 key_type = 0;
    uint32 expect_offset = 0;
    uint32 real_offset = 0;
    uint32 real_offset1 = 0;
    uint32 expect_tbl_idx = 0;
    uint32 result_tbl_idx = 0;
    uint32 tbl_id = 0;
    uint32 block = 0;
    ds_t   ds = {0};
    uint32 tblid = 0;

    _ctc_usw_dkits_tcam_tbl2block(mem_id, &block);

    if (CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block))
    {
        for (i = 0; i < entry_num; i++)
        {
            cmd = DRV_IOR(FlowTcamBistResultFA_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &ds);

            GetFlowTcamBistResultFA(A, expectIndex_f, &ds, &expect_offset);
            GetFlowTcamBistResultFA(A, realIndex_f, &ds, &real_offset);
            GetFlowTcamBistResultFA(A, expectIndexValid_f, &ds, &valid);
            GetFlowTcamBistResultFA(A, realIndexHit_f, &ds, &hit);

            if (valid)
            {
                if (!((TABLE_ENTRY_SIZE(lchip, DsScl0MacIpv6Key640_t) == TABLE_ENTRY_SIZE(lchip, mem_id)) && (1 == (i % 2))))
                {
                    _ctc_usw_dkits_tcam_tbl_index(mem_id, &tcam_blk, DKITS_TCAM_FLOW_RLT_TCAMIDX(expect_offset), &expect_tbl_idx);

                    if (hit)
                    {
                        _ctc_usw_dkits_tcam_tbl_index(mem_id, &tcam_blk, DKITS_TCAM_FLOW_RLT_TCAMIDX(real_offset), &result_tbl_idx);
                        GetFlowTcamBistResultFA(A, missmatchCnt_f, &ds, mismatch_cnt);
                    }
                    ctc_usw_dkits_tcam_read_tcam_type(lchip, mem_id, expect_tbl_idx, &key_type);
                    CTC_DKITS_TCAM_ERROR_RETURN(ctc_usw_dkits_tcam_mem2tbl(lchip, mem_id, key_type, &tbl_id));
                    _ctc_usw_dkits_tcam_bist_show_result(lchip, tbl_id, expect_tbl_idx, result_tbl_idx);
                    CTC_DKIT_PRINT_DEBUG(" %-52s %u: FlowTcamBistResultFA[%u], expectIndex = 0x%04X, realIndex = 0x%04X, expectIndexValid = %u, realIndexHit = %u.\n",
                                         __FUNCTION__, __LINE__, i, expect_offset, real_offset, valid, hit);
                }
            }
        }
    }
    else if (CTC_DKITS_TCAM_IP_TCAM_BLOCK(block) || CTC_DKITS_TCAM_NAT_TCAM_BLOCK(block))
    {
        if (CTC_DKITS_TCAM_IS_LPM_TCAM2(tcam_blk))
        {
            tblid = LpmTcamBistResult2FA_t;
        }
        else if (CTC_DKITS_TCAM_IS_LPM_TCAM1(tcam_blk))
        {
            tblid = LpmTcamBistResult1FA_t;
        }
        else
        {
            tblid = LpmTcamBistResult0FA_t;
        }
        for (i = 0; i < entry_num; i++)
        {
            cmd = DRV_IOR(tblid + (block - DKITS_TCAM_BLOCK_TYPE_LPM_LKP0), DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &ds);

            GetLpmTcamBistResult0FA(A, expectIndex_f, &ds, &expect_offset);
            GetLpmTcamBistResult0FA(A, realIndex0_f, &ds, &real_offset);
            GetLpmTcamBistResult0FA(A, realIndex1_f, &ds, &real_offset1);
            GetLpmTcamBistResult0FA(A, expectIndexValid_f, &ds, &valid);
            GetLpmTcamBistResult0FA(A, realIndex0Hit_f, &ds, &hit);
            GetLpmTcamBistResult0FA(A, realIndex1Hit_f, &ds, &hit1);

            if (valid)
            {
                _ctc_usw_dkits_tcam_tbl_index(mem_id, &tcam_blk, expect_offset, &expect_tbl_idx);

                if (hit)
                {
                    _ctc_usw_dkits_tcam_tbl_index(mem_id, &tcam_blk, real_offset, &result_tbl_idx);
                    GetLpmTcamBistResult0FA(A, missmatchCnt_f, &ds, mismatch_cnt);
                }
                ctc_usw_dkits_tcam_read_tcam_type(lchip, mem_id, expect_tbl_idx, &key_type);
                CTC_DKITS_TCAM_ERROR_RETURN(ctc_usw_dkits_tcam_mem2tbl(lchip, mem_id, key_type, &tbl_id));
                _ctc_usw_dkits_tcam_bist_show_result(lchip, tbl_id, expect_tbl_idx, result_tbl_idx);
                CTC_DKIT_PRINT_DEBUG(" %-52s %u, %s[%u] expect_offset = %u, real_offset = %u, hit = %u.\n",
                                     __FUNCTION__, __LINE__, TABLE_NAME(lchip, tbl_id),
                                     expect_offset/(TABLE_ENTRY_SIZE(lchip, tbl_id)/DRV_LPM_KEY_BYTES_PER_ENTRY),
                                     expect_offset, real_offset, hit);
            }
        }
    }
    else if (DKITS_TCAM_BLOCK_TYPE_CATEGORYID == block)
    {
        for (i = 0; i < entry_num; i++)
        {
            cmd = DRV_IOR(IpeCidTcamBistResult_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &ds);

            GetIpeCidTcamBistResult(A, expectIndex_f,      &ds, &expect_tbl_idx);
            GetIpeCidTcamBistResult(A, readIndex_f,        &ds, &result_tbl_idx);
            GetIpeCidTcamBistResult(A, expectValid_f,      &ds, &valid);
            GetIpeCidTcamBistResult(A, readIndexHit_f,     &ds, &hit);

            if (valid)
            {
                if (hit)
                {
                    GetIpeCidTcamBistResult(A, missmatchCnt_f, &ds, mismatch_cnt);
                }
                _ctc_usw_dkits_tcam_bist_show_result(lchip, mem_id, expect_tbl_idx, result_tbl_idx);
                CTC_DKIT_PRINT_DEBUG(" %-52s %u, tbl name = %s, tbl idx = %u, expect offset = %u, real offset = %u, hit = %u.\n",
                                __FUNCTION__, __LINE__, TABLE_NAME(lchip, mem_id),
                                expect_tbl_idx, expect_tbl_idx, result_tbl_idx, hit);
            }
        }
    }
    else if (DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE == block)
    {
        for (i = 0; i < entry_num; i++)
        {
            cmd = DRV_IOR(QMgrEnqTcamBistResultFa_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &ds);

            GetQMgrEnqTcamBistResultFa(A, expectIndex_f,      &ds, &expect_tbl_idx);
            GetQMgrEnqTcamBistResultFa(A, realIndex_f,        &ds, &result_tbl_idx);
            GetQMgrEnqTcamBistResultFa(A, expectIndexValid_f, &ds, &valid);
            GetQMgrEnqTcamBistResultFa(A, realIndexHit_f,     &ds, &hit);

            if (valid)
            {
                if (hit)
                {
                    GetQMgrEnqTcamBistResultFa(A, missmatchCnt_f, &ds, mismatch_cnt);
                }
                _ctc_usw_dkits_tcam_bist_show_result(lchip, mem_id, expect_tbl_idx, result_tbl_idx);
                CTC_DKIT_PRINT_DEBUG(" %-52s %u, tbl name = %s, tbl idx = %u, expect offset = %u, real offset = %u, hit = %u.\n",
                                 __FUNCTION__, __LINE__, TABLE_NAME(lchip, mem_id),
                                 expect_tbl_idx, expect_tbl_idx, result_tbl_idx, hit);
            }
        }
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_bist_deliver_result(uint8 lchip, tbls_id_t tblid, uint32 tcam_blk, dkits_tcam_block_type_t block, uint8 num, uint8* mismatch_cnt)
{
    uint8 sleep_num = 0;
    uint32 done = 0;

    while (1)
    {
        _ctc_usw_dkits_tcam_bist_detect(lchip, block, &done);
        if (done)
        {
            _ctc_usw_dkits_tcam_bist_present_result(lchip, tblid, tcam_blk, num, mismatch_cnt);
            break;
        }
        else
        {
            sleep_num++;
            if (5 == sleep_num)
            {
                break;
            }
            sal_task_sleep(1);
        }
    }

    return 0;
}

STATIC int32
_ctc_usw_dkits_tcam_bist(ctc_dkits_tcam_info_t* p_tcam_info, tbls_id_t tblid, uint32 bist_entry, uint32 tcam_blk)
{
    dkits_tcam_block_type_t block = 0;
    uint8 lchip = 0;
    uint8 bist_once_en = 0;

    DKITS_PTR_VALID_CHECK(p_tcam_info);
    lchip = p_tcam_info->lchip;
    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_tbl2block(tblid, &block));
    if (CTC_DKITS_TCAM_IP_TCAM_BLOCK(block) || CTC_DKITS_TCAM_NAT_TCAM_BLOCK(block))
    {
        if(CTC_DKITS_TCAM_IS_LPM_TCAM0(tcam_blk))
        {
            block = DKITS_TCAM_BLOCK_TYPE_LPM_LKP0;
        }
        else if(CTC_DKITS_TCAM_IS_LPM_TCAM1(tcam_blk))
        {
            block = DKITS_TCAM_BLOCK_TYPE_LPM_LKP1;
        }
        else
        {
            block = DKITS_TCAM_BLOCK_TYPE_NAT_PBR;
        }
    }
    bist_once_en = p_tcam_info->is_stress ? 0 : 1;
    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_bist_start(lchip, block, bist_once_en));
    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_bist_deliver_result(lchip, tblid, tcam_blk, block, bist_entry, (uint8*)&(p_tcam_info->mismatch_cnt)));
    CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_reset_detect(lchip, block, DKITS_TCAM_DETECT_TYPE_BIST));
    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_bist_write_expect_result(uint8 lchip, tbls_id_t tblid, uint8 mem_idx, uint32 tcam_blk, uint32 expect_idx)
{
    uint32 cmd = 0;
    uint32 block = 0, i = 0, tcamid = 0;
    ds_t   ds = {0};
    uint32 tbl_id = 0;

    _ctc_usw_dkits_tcam_tbl2block(tblid, &block);

    if (CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block))
    {
        if ((DRV_BYTES_PER_ENTRY*8) == TCAM_KEY_SIZE(lchip, tblid))
        {
            SetFlowTcamBistResultFA(V, expectIndexValid_f, &ds, 1);
            SetFlowTcamBistResultFA(V, expectIndex_f,      &ds, DKITS_TCAM_FLOW_RLT_INDEX(tcam_blk, expect_idx));

            CTC_DKIT_PRINT_DEBUG(" %-52s %u: %s[%u], tcamBlock:%-2u, offset:0x%04X, %s[%u].expectIndex:0x%04X.\n",
                                 __FUNCTION__, __LINE__, TABLE_NAME(lchip, tblid),
                                 (expect_idx*DRV_BYTES_PER_ENTRY)/TABLE_ENTRY_SIZE(lchip, tblid), tcam_blk, expect_idx,
                                 TABLE_NAME(lchip, FlowTcamBistResultFA_t), mem_idx,
                                 DKITS_TCAM_FLOW_RLT_INDEX(tcam_blk, expect_idx));

            cmd = DRV_IOW(FlowTcamBistResultFA_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, mem_idx, cmd, &ds);
            mem_idx++;
        }

        SetFlowTcamBistResultFA(V, expectIndexValid_f, &ds, 1);
        SetFlowTcamBistResultFA(V, expectIndex_f,      &ds, DKITS_TCAM_FLOW_RLT_INDEX(tcam_blk, expect_idx));
        CTC_DKIT_PRINT_DEBUG(" %-52s %u: %s[%u], tcamBlock:%-2u, offset:0x%04X, %s[%u].expectIndex:0x%04X.\n",
                             __FUNCTION__, __LINE__, TABLE_NAME(lchip, tblid),
                             (expect_idx*DRV_BYTES_PER_ENTRY)/TABLE_ENTRY_SIZE(lchip, tblid), tcam_blk, expect_idx,
                             TABLE_NAME(lchip, FlowTcamBistResultFA_t), mem_idx,
                             DKITS_TCAM_FLOW_RLT_INDEX(tcam_blk, expect_idx));

        cmd = DRV_IOW(FlowTcamBistResultFA_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, mem_idx, cmd, &ds);
    }
    else if (CTC_DKITS_TCAM_IP_TCAM_BLOCK(block) || CTC_DKITS_TCAM_NAT_TCAM_BLOCK(block))
    {
        i = block - DKITS_TCAM_BLOCK_TYPE_LPM_LKP0;
        _ctc_usw_dkits_tcam_lpm_module2tcamid(tcam_blk >> 2, &tcamid);
        if (CTC_DKITS_TCAM_IS_LPM_TCAM2(tcam_blk))
        {
            tbl_id = LpmTcamBistResult2FA_t;
        }
        else if (CTC_DKITS_TCAM_IS_LPM_TCAM1(tcam_blk))
        {
            tbl_id = LpmTcamBistResult1FA_t;
        }
        else
        {
            tbl_id = LpmTcamBistResult0FA_t;
        }
        SetLpmTcamBistResult0FA(V, expectIndexValid_f, &ds, 1);
        if ((DKITS_TCAM_BLOCK_TYPE_LPM_LKP0 == block) || (DKITS_TCAM_BLOCK_TYPE_LPM_LKP1 == block))
        {
            SetLpmTcamBistResult0FA(V, expectIndex_f, &ds, DKITS_TCAM_IP_RLT_INDEX(tcamid, expect_idx));

            CTC_DKIT_PRINT_DEBUG(" %-52s %u: %s[%u] tcamBlock:%-2u, offset:0x%04X, %s[%u].expectIndex:0x%04X.\n",
                                 __FUNCTION__, __LINE__, TABLE_NAME(lchip, tblid),
                                 (expect_idx*DRV_LPM_KEY_BYTES_PER_ENTRY)/TABLE_ENTRY_SIZE(lchip, tblid), tcam_blk, expect_idx,
                                 TABLE_NAME(lchip, tbl_id + i), mem_idx,
                                 DKITS_TCAM_IP_RLT_INDEX(tcamid, expect_idx));
        }
        else
        {
            SetLpmTcamBistResult0FA(V, expectIndex_f, &ds, DKITS_TCAM_NAT_RLT_INDEX(tcamid, expect_idx));

            CTC_DKIT_PRINT_DEBUG(" %-52s %u: %s[%u], tcamBlock:%-2u, offset:0x%04X, %s[%u].expectIndex:0x%04X.\n",
                                 __FUNCTION__, __LINE__, TABLE_NAME(lchip, tblid),
                                 (expect_idx*DRV_LPM_KEY_BYTES_PER_ENTRY)/TABLE_ENTRY_SIZE(lchip, tblid), tcam_blk, expect_idx,
                                 TABLE_NAME(lchip, tbl_id + i), mem_idx,
                                 DKITS_TCAM_NAT_RLT_INDEX(tcamid, expect_idx));
        }

        cmd = DRV_IOW(tbl_id + i, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, mem_idx, cmd, &ds);
    }
    else if (DKITS_TCAM_BLOCK_TYPE_CATEGORYID == block)
    {
        SetIpeCidTcamBistResult(V, expectValid_f, &ds, 1);
        SetIpeCidTcamBistResult(V, expectIndex_f, &ds, expect_idx);

        CTC_DKIT_PRINT_DEBUG(" %-52s %u: %s[%u], IpeCidTcamBistResult[%u].expectIndex:0x%04X.\n",
                             __FUNCTION__, __LINE__, TABLE_NAME(lchip, DsCategoryIdPairTcamKey_t),
                             expect_idx, mem_idx, expect_idx);

        cmd = DRV_IOW(IpeCidTcamBistResult_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, mem_idx, cmd, &ds);
    }
    else if (DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE == block)
    {
        SetQMgrEnqTcamBistResultFa(V, expectIndexValid_f, &ds, 1);
        SetQMgrEnqTcamBistResultFa(V, expectIndex_f, &ds, expect_idx);

        CTC_DKIT_PRINT_DEBUG(" %-52s %u: %s[%u], QMgrEnqTcamBistResultFa[%u].expectIndex:0x%04X.\n",
                             __FUNCTION__, __LINE__, TABLE_NAME(lchip, DsQueueMapTcamKey_t),
                             expect_idx, mem_idx, expect_idx);

        cmd = DRV_IOW(QMgrEnqTcamBistResultFa_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, mem_idx, cmd, &ds);
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_bist_write_lookup_info(uint8 lchip, tbls_id_t tblid, tbl_entry_t* p_tcam_key, uint32* p_mem_idx, uint32 tcam_blk)
{
    uint32    cmd = 0;
    int32     i = 0;
    ds_t      ds = {0};
    ds_t      req = {0};
    uint32 tbl_id = 0;

    dkits_tcam_block_type_t block = DKITS_TCAM_BLOCK_TYPE_INVALID;

    _ctc_usw_dkits_tcam_tbl2block(tblid, &block);

    if (CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block))
    {
        if ((DRV_BYTES_PER_ENTRY*8) == TCAM_KEY_SIZE(lchip, tblid))  /*key size 640 bits*/
        {
            SetFlowTcamBistReqFA(V, bistReqKeySize_f,  &req, 3);
            SetFlowTcamBistReqFA(V, bistReqKeyValid_f, &req, 1);

            for (i = 0; i <  TABLE_ENTRY_SIZE(lchip, DsScl0MacIpv6Key640_t)/TABLE_ENTRY_SIZE(lchip, DsScl0Ipv6Key320_t); i++)
            {
                CTC_DKIT_PRINT_DEBUG(" %-52s %u: %s, FlowTcamBistReqFA[%u], bistReqKeySize = %u, bistReqKeyValid  = %u, bistReqKeyEnd = %u\n",
                                      __FUNCTION__, __LINE__, TABLE_NAME(lchip, tblid), *p_mem_idx, 3, 1, i);

                _ctc_usw_dkits_tcam_build_key(lchip, tblid, p_tcam_key, (uint32*)&ds);
                sal_memcpy((uint8*)&req + (i * DRV_BYTES_PER_ENTRY * 4), (uint8*)&ds, (DRV_BYTES_PER_ENTRY*4));
                SetFlowTcamBistReqFA(V, bistReqKeyEnd_f, &req, i);
                cmd = DRV_IOW(FlowTcamBistReqFA_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, *p_mem_idx + i, cmd, &req);
                if (0 == i)
                {
                    (*p_mem_idx)++;
                }
            }
        }
        else
        {
            CTC_DKIT_PRINT_DEBUG(" %-52s %u: %s, FlowTcamBistReqFA[%u], bistReqKeySize = %u, bistReqKeyValid  = %u, bistReqKeyEnd = %u\n",
                                 __FUNCTION__, __LINE__, TABLE_NAME(lchip, tblid), *p_mem_idx,
                                 ((TCAM_KEY_SIZE(lchip, tblid) / DRV_BYTES_PER_ENTRY) >> 1), 1, 1);

            _ctc_usw_dkits_tcam_build_key(lchip, tblid, p_tcam_key, (uint32*)&ds);
            if (DRV_BYTES_PER_ENTRY == TCAM_KEY_SIZE(lchip, tblid))
            {
                sal_memcpy((uint8*)&req, (uint8*)&ds, TCAM_KEY_SIZE(lchip, tblid));
                sal_memcpy((uint8*)&req + TCAM_KEY_SIZE(lchip, tblid), (uint8*)&ds , TCAM_KEY_SIZE(lchip, tblid));
                sal_memcpy((uint8*)&req + TCAM_KEY_SIZE(lchip, tblid)*2, (uint8*)&ds , TCAM_KEY_SIZE(lchip, tblid));
                sal_memcpy((uint8*)&req + TCAM_KEY_SIZE(lchip, tblid)*3, (uint8*)&ds, TCAM_KEY_SIZE(lchip, tblid));
            }
            else if ((DRV_BYTES_PER_ENTRY*2) == TCAM_KEY_SIZE(lchip, tblid))
            {
                sal_memcpy((uint8*)&req, (uint8*)&ds, TCAM_KEY_SIZE(lchip, tblid));
                sal_memcpy((uint8*)&req+ TCAM_KEY_SIZE(lchip, tblid), (uint8*)&ds , TCAM_KEY_SIZE(lchip, tblid));
            }
            else if ((DRV_BYTES_PER_ENTRY*4) == TCAM_KEY_SIZE(lchip, tblid))
            {
                sal_memcpy((uint8*)&req, (uint8*)&ds, TCAM_KEY_SIZE(lchip, tblid));
            }
            SetFlowTcamBistReqFA(V, bistReqKeySize_f,  &req, ((TCAM_KEY_SIZE(lchip, tblid) / DRV_BYTES_PER_ENTRY) >> 1));
            SetFlowTcamBistReqFA(V, bistReqKeyValid_f, &req,  1);
            SetFlowTcamBistReqFA(V, bistReqKeyEnd_f,   &req,  1);
            cmd = DRV_IOW(FlowTcamBistReqFA_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, *p_mem_idx, cmd, &req);
        }
    }
    else if (CTC_DKITS_TCAM_IP_TCAM_BLOCK(block) || CTC_DKITS_TCAM_NAT_TCAM_BLOCK(block))
    {
        if (CTC_DKITS_TCAM_IS_LPM_TCAM2(tcam_blk))
        {
            tbl_id = LpmTcamBistReq2FA_t;
        }
        else if (CTC_DKITS_TCAM_IS_LPM_TCAM1(tcam_blk))
        {
            tbl_id = LpmTcamBistReq1FA_t;
        }
        else
        {
            tbl_id = LpmTcamBistReq0FA_t;
        }
        CTC_DKIT_PRINT_DEBUG(" %-52s %u: %s, %s[%u], bistReqKeySize = %u, bistReqKeyValid  = %u\n",
                             __FUNCTION__, __LINE__, TABLE_NAME(lchip, tblid),
                             TABLE_NAME(lchip, tbl_id + (block - DKITS_TCAM_BLOCK_TYPE_LPM_LKP0)),
                             *p_mem_idx, ((TCAM_KEY_SIZE(lchip, tblid)/DRV_LPM_KEY_BYTES_PER_ENTRY) >> 1), 1);

        _ctc_usw_dkits_tcam_build_key(lchip, tblid, p_tcam_key, (uint32*)&ds);
        sal_memcpy((uint8*)&req, (uint8*)&ds, TCAM_KEY_SIZE(lchip, tblid));
        if (DRV_LPM_KEY_BYTES_PER_ENTRY == TCAM_KEY_SIZE(lchip, tblid)) /*key size 46 bits*/
        {
            sal_memcpy((uint8*)&req + TCAM_KEY_SIZE(lchip, tblid), (uint8*)&ds , TCAM_KEY_SIZE(lchip, tblid));
            sal_memcpy((uint8*)&req + TCAM_KEY_SIZE(lchip, tblid)*2, (uint8*)&ds, TCAM_KEY_SIZE(lchip, tblid));
            sal_memcpy((uint8*)&req + TCAM_KEY_SIZE(lchip, tblid)*3, (uint8*)&ds , TCAM_KEY_SIZE(lchip, tblid));
        }
        SetLpmTcamBistReq0FA(V, bistReqKeySize_f, &req, ((TCAM_KEY_SIZE(lchip, tblid)/DRV_LPM_KEY_BYTES_PER_ENTRY) >> 1));
        SetLpmTcamBistReq0FA(V, bistReqKeyValid_f, &req, 1);
        cmd = DRV_IOW(tbl_id + (block - DKITS_TCAM_BLOCK_TYPE_LPM_LKP0), DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, *p_mem_idx, cmd, &req);
    }
    else if (DKITS_TCAM_BLOCK_TYPE_CATEGORYID == block)
    {
        CTC_DKIT_PRINT_DEBUG(" %-52s %u: %s, %s[%u], bistReqKeySize = %u, bistReqKeyValid  = %u\n",
                             __FUNCTION__, __LINE__, TABLE_NAME(lchip, DsCategoryIdPairTcamKey_t),
                             TABLE_NAME(lchip, IpeCidTcamBistReq_t), *p_mem_idx, 0, 1);

        _ctc_usw_dkits_tcam_build_key(lchip, DsCategoryIdPairTcamKey_t, p_tcam_key, (uint32*)&ds);
        sal_memcpy((uint8*)&req, (uint8*)&ds, TCAM_KEY_SIZE(lchip, tblid));
        SetIpeCidTcamBistReq(V, keySize_f, &req, 0);
        SetIpeCidTcamBistReq(V, valid_f,   &req, 1);
        cmd = DRV_IOW(IpeCidTcamBistReq_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, *p_mem_idx, cmd, &req);
    }
    else if (DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE == block)
    {
        CTC_DKIT_PRINT_DEBUG(" %-52s %u: %s, %s[%u], bistReqKeySize = %u, bistReqKeyValid  = %u\n",
                             __FUNCTION__, __LINE__, TABLE_NAME(lchip, DsQueueMapTcamKey_t),
                             TABLE_NAME(lchip, QMgrEnqTcamBistReqFa_t), *p_mem_idx, 0, 1);

        _ctc_usw_dkits_tcam_build_key(lchip, DsQueueMapTcamKey_t, p_tcam_key, (uint32*)&ds);
        sal_memcpy((uint8*)&req, (uint8*)&ds, TCAM_KEY_SIZE(lchip, tblid));
        SetQMgrEnqTcamBistReqFa(V, bistReqKeySize_f,  &req, 0);
        SetQMgrEnqTcamBistReqFa(V, bistReqKeyValid_f, &req, 1);
        cmd = DRV_IOW(QMgrEnqTcamBistReqFa_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, *p_mem_idx, cmd, &req);
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_bist_build_request(uint8 lchip, uint32 tbl_id, uint32 tbl_idx, tbl_entry_t* p_tcam_key, uint32* p_mem_idx, uint32* p_tcam_blk)
{
    uint32 entry_offset = 0;

    ctc_usw_dkits_tcam_entry_offset(lchip, tbl_id, tbl_idx, &entry_offset, p_tcam_blk);
    _ctc_usw_dkits_tcam_bist_write_expect_result(lchip, tbl_id, *p_mem_idx, *p_tcam_blk, entry_offset);
    _ctc_usw_dkits_tcam_bist_write_lookup_info(lchip, tbl_id, p_tcam_key, p_mem_idx, *p_tcam_blk);

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_bist_scan_key_index(ctc_dkits_tcam_info_t* p_tcam_info, tbls_id_t tblid, uint32 key_idx, uint32 max_idx, uint32* p_num)
{
    ds_t        data = {0};
    ds_t        mask = {0};
    uint32      empty = 0;
    uint32      tcam_blk = 0;
    tbl_entry_t tcam_key;
    uint8 lchip = 0;
    DKITS_PTR_VALID_CHECK(p_tcam_info);

    lchip = p_tcam_info->lchip;
    sal_memset(&data, 0, sizeof(data));
    sal_memset(&mask, 0, sizeof(mask));
    tcam_key.data_entry = (uint32 *)&data;
    tcam_key.mask_entry = (uint32 *)&mask;

    ctc_usw_dkits_tcam_read_tcam_key(lchip, tblid, key_idx, &empty, &tcam_key);
    if (!empty)
    {
        CTC_DKIT_PRINT_DEBUG(" %-52s %u: bist request index:%u num:%u\n", __FUNCTION__, __LINE__, key_idx, *p_num);
        _ctc_usw_dkits_tcam_bist_build_request(lchip, tblid, key_idx, &tcam_key, p_num, &tcam_blk);
        (*p_num)++;
        CTC_DKIT_PRINT_DEBUG(" %-52s %u: bist request num:%u\n", __FUNCTION__, __LINE__, *p_num);
        _ctc_usw_dkits_tcam_bist(p_tcam_info, tblid, *p_num, tcam_blk);
        CTC_DKIT_PRINT_DEBUG("---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
        *p_num = 0;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_bist_scan_tcam_tbl(ctc_dkits_tcam_info_t* p_tcam_info, uint32 tblid)
{
    uint32 i = 0;
    uint32 num = 0;
    uint32 entry_num = 0;
    uint32 cmd = 0;
    tbl_entry_t tcam_key;
    ds_t data = {0};
    ds_t mask = {0};
    tbls_id_t tblid_tmp = MaxTblId_t - 1;
    uint8 lchip = 0;
    DKITS_PTR_VALID_CHECK(p_tcam_info);

    lchip = p_tcam_info->lchip;
    drv_usw_ftm_get_entry_num(p_tcam_info->lchip, tblid, &entry_num);
    CTC_DKIT_PRINT_DEBUG(" %-52s %u: tblid = %u entry_num =%u\n", __FUNCTION__, __LINE__, tblid, entry_num);
    if (DsCategoryIdPairTcamKey_t == tblid)
    {
        entry_num = 32;
    }
    if (DsQueueMapTcamKey_t == tblid)
    {
        entry_num = 32;
    }
    if (p_tcam_info->is_stress)
    {
        sal_memset(&data, 0x5A, sizeof(data));
        sal_memset(&mask, 0xFF, sizeof(mask));
        tcam_key.data_entry = (uint32 *)&data;
        tcam_key.mask_entry = (uint32 *)&mask;
        for (i = 0; i < entry_num; i++)
        {
            cmd = DRV_IOW(tblid, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &tcam_key);
            _ctc_usw_dkits_tcam_bist_scan_key_index(p_tcam_info, tblid, i, entry_num, &num);
            _ctc_usw_dkits_tcam_bist_tbl_brief_status(&tblid_tmp, 1);
            if (MaxTblId_t == tblid_tmp)
            {
                CTC_DKIT_PRINT("    Stress Bist fail!\n");
                CTC_DKIT_PRINT("    TCAM SCAN MisMatchCnt %u\n", p_tcam_info->mismatch_cnt);
                CTC_DKIT_PRINT("    TCAM key name: %s  index=%u\n", TABLE_NAME(lchip, tblid), i);
                tblid_tmp = MaxTblId_t - 1;
                _ctc_usw_dkits_tcam_bist_tbl_brief_status(&tblid_tmp, 0);
                return CLI_SUCCESS;
            }
            else
            {
                cmd = DRV_IOD(tblid, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, i, cmd, &tcam_key);
            }
        }

    }
    else/*bist once by tcam key index*/
    {
        if (p_tcam_info->index >= entry_num)
        {
            CTC_DKIT_PRINT("Key Index Error, p_tcam_info->index = %u exceed max index value %u\n", p_tcam_info->index, entry_num - 1);
            return CLI_ERROR;
        }
        /*Write tcam key by index*/
        sal_memset(&data, 0x5A, sizeof(data));
        sal_memset(&mask, 0xFF, sizeof(mask));
        tcam_key.data_entry = (uint32 *)&data;
        tcam_key.mask_entry = (uint32 *)&mask;
        cmd = DRV_IOW(tblid, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, p_tcam_info->index, cmd, &tcam_key);

        _ctc_usw_dkits_tcam_bist_scan_key_index(p_tcam_info, tblid, p_tcam_info->index, entry_num, &num);

        /*Delete tcam key by index*/
        cmd = DRV_IOD(tblid, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, p_tcam_info->index, cmd, &tcam_key);
    }
    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_bist_scan_tcam_type(ctc_dkits_tcam_info_t* p_tcam_info, dkits_tcam_block_type_t block)
{
    uint32  i = 0;
    uint8 key_size = 0;
    uint8 lchip = 0;
    DKITS_PTR_VALID_CHECK(p_tcam_info);
    lchip = p_tcam_info->lchip;
    key_size = p_tcam_info->key_size;
    for (i = 0; (MaxTblId_t != tcam_mem[block][i]); i++)
    {
        if ((CTC_DKITS_INVALID_TBLID == tcam_mem[block][i])
            || ((NULL != TABLE_EXT_INFO_PTR(lchip, tcam_mem[block][i])) && (0 == TABLE_MAX_INDEX(lchip, tcam_mem[block][i])))
        || ((key_size >= CTC_DKITS_TCAM_BIST_SIZE_MAX)))
        {
            continue;
        }
        if (CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block)
            && (((key_size == CTC_DKITS_TCAM_BIST_SIZE_HALF) && ((DRV_BYTES_PER_ENTRY*8) != TCAM_KEY_SIZE(lchip, tcam_mem[block][i])))
        || ((key_size == CTC_DKITS_TCAM_BIST_SIZE_SINGLE) && ((DRV_BYTES_PER_ENTRY*4) != TCAM_KEY_SIZE(lchip, tcam_mem[block][i])))
        || ((key_size == CTC_DKITS_TCAM_BIST_SIZE_DOUBLE) && ((DRV_BYTES_PER_ENTRY*2) != TCAM_KEY_SIZE(lchip, tcam_mem[block][i])))
        || ((key_size == CTC_DKITS_TCAM_BIST_SIZE_QUAD) && ((DRV_BYTES_PER_ENTRY) != TCAM_KEY_SIZE(lchip, tcam_mem[block][i])))))
        {
            continue;
        }
        if ((CTC_DKITS_TCAM_IP_TCAM_BLOCK(block) || (CTC_DKITS_TCAM_NAT_TCAM_BLOCK(block)))
            && (((key_size == CTC_DKITS_TCAM_BIST_SIZE_HALF) && (DRV_LPM_KEY_BYTES_PER_ENTRY*8 != TCAM_KEY_SIZE(lchip, tcam_mem[block][i])))
        || ((key_size == CTC_DKITS_TCAM_BIST_SIZE_SINGLE) && (DRV_LPM_KEY_BYTES_PER_ENTRY*4 != TCAM_KEY_SIZE(lchip, tcam_mem[block][i])))
        || ((key_size == CTC_DKITS_TCAM_BIST_SIZE_DOUBLE) && (DRV_LPM_KEY_BYTES_PER_ENTRY*2 != TCAM_KEY_SIZE(lchip, tcam_mem[block][i])))
        || ((key_size == CTC_DKITS_TCAM_BIST_SIZE_QUAD) && (DRV_LPM_KEY_BYTES_PER_ENTRY != TCAM_KEY_SIZE(lchip, tcam_mem[block][i])))))
        {
            continue;
        }
        _ctc_usw_dkits_tcam_bist_scan_tcam_tbl(p_tcam_info, tcam_mem[block][i]);
    }


    return CLI_SUCCESS;
}

#define __3_CAPTURE__

STATIC uint32
_ctc_usw_dkits_tcam_capture_status(dkits_tcam_block_type_t block, uint8 is_set, uint32 value)
{
    static uint32 tcam_flag = 0;

    if (is_set)
    {
        if (value)
        {
            DKITS_BIT_SET(tcam_flag, block);
        }
        else
        {
            DKITS_BIT_UNSET(tcam_flag, block);
        }
    }

    return tcam_flag;
}

STATIC int32
_ctc_usw_dkits_tcam_capture_present_result(uint8 lchip, dkits_tcam_capture_info_t* p_capture_info, uint32* p_idx, uint32 idx)
{
    uint32 i = 0;
    dkits_tcam_block_type_t block = DKITS_TCAM_BLOCK_TYPE_INVALID;
    uint32 fld_idx = 0;
    uint32 cur_fld_idx = 0;
    uint32 value[MAX_ENTRY_WORD] = {0};
    uint32 field_num = 0;
    uint8  uint32_id = 0;
    char   str[35] = {0};
    char   format[10] = {0};
    fields_t* fld_ptr = NULL;
    tables_info_t* tbl_ptr = NULL;
    char* ptr_field_name = NULL;

    for (i = 0; i < DKITS_TCAM_PER_TYPE_PRIORITY_NUM; i++)
    {
        if (1 == p_capture_info->tbl[i].lookup_valid)
        {
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_tbl2block(p_capture_info->tbl[i].id, &block));
            if ((0xFFFFFFFF == idx) || ((0xFFFFFFFF != idx) && (++(*p_idx) == idx)))
            {
                if (1 == p_capture_info->tbl[i].result_valid)
                {
                    CTC_DKIT_PRINT(" %-6u%-27s%-38s%-15s%u\n",
                               ++(*p_idx), tcam_block_desc[block], TABLE_NAME(lchip, p_capture_info->tbl[i].id),
                               "True", p_capture_info->tbl[i].idx);
                }
                else
                {
                    CTC_DKIT_PRINT(" %-6u%-27s%-38s%-15s%s\n",
                                   ++(*p_idx), tcam_block_desc[block], TABLE_NAME(lchip, p_capture_info->tbl[i].id),
                                   "False", "-");
                }

                if (0xFFFFFFFF != idx)
                {
                    CTC_DKIT_PRINT(" -----------------------------------------------------------------------------------------------------\n");
                    CTC_DKIT_PRINT("\n");
                    CTC_DKIT_PRINT(" %-8s%-13s%-9s%-19s%5s0x%04x\n", "FldID", "Value", "BitLen", "FldName","Index=",
                                   p_capture_info->tbl[i].idx);
                    CTC_DKIT_PRINT(" -------------------------------------------------------------\n");

                    tbl_ptr = TABLE_INFO_PTR(lchip, p_capture_info->tbl[i].id);
                    field_num = tbl_ptr->field_num;

                    for (fld_idx = fld_idx; fld_idx < field_num; fld_idx++)
                    {
                        sal_memset(value, 0 , sizeof(value));
                        fld_ptr = &(tbl_ptr->ptr_fields[fld_idx]);

                        drv_get_field(lchip, p_capture_info->tbl[i].id, fld_ptr->field_id, &capture_ds[idx], value);

                        cur_fld_idx = fld_idx;
                        ptr_field_name = fld_ptr->ptr_field_name;

                        CTC_DKIT_PRINT(" %-8u%-13s%-9u%-40s\n", cur_fld_idx, CTC_DKITS_HEX_FORMAT(str, format, value[0], 8), fld_ptr->bits, ptr_field_name);
                        for (uint32_id = 1; uint32_id < ((fld_ptr->bits + 31)/32); uint32_id++)
                        {
                            CTC_DKIT_PRINT("%-8s%s\n", " ", CTC_DKITS_HEX_FORMAT(str, format, value[uint32_id], 8));
                        }
                    }
                }
            }
        }
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_capture_decap_request(uint8 lchip, dkits_tcam_block_type_t block, dkits_tcam_capture_info_t* p_capture_info)
{
    uint8     i = 0, offset = 0;
    uint32    cmd = 0, key_size = 0, key_type = 0, req_end = 0;
    tbls_id_t tblid = MaxTblId_t;
    ds_t      req = {0}, key = {0}, ds = {0};

    if (CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block))
    {
        for (i = 0; i < TABLE_MAX_INDEX(lchip, FlowTcamBistReqFA_t); i++)
        {
            cmd = DRV_IOR(FlowTcamBistReqFA_t, DRV_ENTRY_FLAG);
            CTC_DKITS_TCAM_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, &req));

            p_capture_info->tbl[i].lookup_valid = GetFlowTcamBistReqFA(V, bistReqKeyValid_f, &req);
            GetFlowTcamBistReqFA(A, bistReqKeySize_f,  &req, &key_size);
            GetFlowTcamBistReqFA(A, bistReqKey_f,      &req, ((uint8*)&key) + offset);
            GetFlowTcamBistReqFA(A, bistReqKeyEnd_f,   &req, &req_end);

            CTC_DKIT_PRINT_DEBUG(" %-52s %u: FlowTcamBistReqFA[%u].bistReqKeyValid:%u, bistReqKeyEnd:%u, bistReqKeySize:%u\n",
                                 __FUNCTION__, __LINE__, i, p_capture_info->tbl[i].lookup_valid, req_end, key_size);

            if (p_capture_info->tbl[i].lookup_valid)
            {
                if (!req_end)
                {
                    if (3 == key_size)
                    {
                        p_capture_info->tbl[i].lookup_valid = 0;
                        offset = TABLE_ENTRY_SIZE(lchip, tcam_mem[block][key_size]) >> 1;
                        continue;
                    }
                    else
                    {
                        CTC_DKIT_PRINT(" @@ DKIT ERROR! %s %d %s Capture block:%u, request valid:%u, request end:%u.\n",
                                       __FILE__, __LINE__, __FUNCTION__, block, p_capture_info->tbl[i].lookup_valid, req_end);
                    }
                }
                else
                {
                    if (CTC_DKITS_TCAM_SCL_TCAM_BLOCK(block))
                    {
                        _ctc_usw_dkits_tcam_xbits2key(lchip, tcam_mem[block][key_size], &key, &ds);
                        GetDsUserId0TcamKey80(A, sclKeyType_f, &key, &key_type);
                        CTC_DKITS_TCAM_ERROR_RETURN(ctc_usw_dkits_tcam_mem2tbl(lchip, tcam_mem[block][key_size], key_type, &tblid));
                        p_capture_info->tbl[i].id = tblid;
                        CTC_DKIT_PRINT_DEBUG(" Tcam capture scl block %u table_id:%u, key_type:%u\n", block, tblid, key_type);
                    }
                    else if (CTC_DKITS_TCAM_IGS_ACL_TCAM_BLOCK(block) || CTC_DKITS_TCAM_EGS_ACL_TCAM_BLOCK(block))
                    {
                        _ctc_usw_dkits_tcam_xbits2key(lchip, tcam_mem[block][key_size], &key, &ds);
                        if (0 == key_size)
                        {
                            if (CTC_DKITS_TCAM_IGS_ACL_TCAM_BLOCK(block))
                            {
                                p_capture_info->tbl[i].id = DsAclQosKey80Ing0_t + (block - DKITS_TCAM_BLOCK_TYPE_IGS_ACL0);
                            }
                            else
                            {
                                p_capture_info->tbl[i].id = DsAclQosKey80Egr0_t + (block - DKITS_TCAM_BLOCK_TYPE_EGS_ACL0);
                            }
                        }
                        else
                        {
                            GetDsAclQosL3Key160Ing0(A, aclQosKeyType_f, &key, &key_type);
                            CTC_DKITS_TCAM_ERROR_RETURN(ctc_usw_dkits_tcam_mem2tbl(lchip, tcam_mem[block][key_size], key_type, &tblid));
                            p_capture_info->tbl[i].id = tblid;
                        }
                    }
                    sal_memcpy(&capture_ds[i], &ds, sizeof(ds_t));
                }
            }
        }
    }
    else if (CTC_DKITS_TCAM_IP_TCAM_BLOCK(block) || CTC_DKITS_TCAM_NAT_TCAM_BLOCK(block))
    {
        for (i = 0; i < TABLE_MAX_INDEX(lchip, LpmTcamBistReq0FA_t); i++)
        {
            if (DKITS_TCAM_BLOCK_TYPE_LPM_LKP0 == block)
            {
                cmd = DRV_IOR(LpmTcamBistReq0FA_t, DRV_ENTRY_FLAG);
                CTC_DKITS_TCAM_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, &req));

                p_capture_info->tbl[i].lookup_valid = GetLpmTcamBistReq0FA(V, bistReqKeyValid_f, &req);
                GetLpmTcamBistReq0FA(A, bistReqKeySize_f, &req, &key_size);
                GetLpmTcamBistReq0FA(A, bistReqKey_f,     &req, &key);
                CTC_DKIT_PRINT_DEBUG(" %-52s %u: LpmTcamBistReq0FA[0].bistReqKeyValid:%u bistReqKeySize:%u\n",
                                     __FUNCTION__, __LINE__, p_capture_info->tbl[i].lookup_valid, key_size);
            }
            else if (DKITS_TCAM_BLOCK_TYPE_LPM_LKP1 == block)
            {
                cmd = DRV_IOR(LpmTcamBistReq1FA_t, DRV_ENTRY_FLAG);
                CTC_DKITS_TCAM_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, &req));

                p_capture_info->tbl[i].lookup_valid = GetLpmTcamBistReq1FA(V, bistReqKeyValid_f, &req);
                GetLpmTcamBistReq1FA(A, bistReqKeySize_f, &req, &key_size);
                GetLpmTcamBistReq1FA(A, bistReqKey_f,     &req, &key);
                CTC_DKIT_PRINT_DEBUG(" %-52s %u: LpmTcamBistReq1FA[0].bistReqKeyValid:%u bistReqKeySize:%u\n",
                                     __FUNCTION__, __LINE__, p_capture_info->tbl[i].lookup_valid, key_size);
            }
            else if (DKITS_TCAM_BLOCK_TYPE_NAT_PBR == block)
            {
                cmd = DRV_IOR(LpmTcamBistReq2FA_t, DRV_ENTRY_FLAG);
                CTC_DKITS_TCAM_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, &req));

                p_capture_info->tbl[i].lookup_valid = GetLpmTcamBistReq2FA(V, bistReqKeyValid_f, &req);
                GetLpmTcamBistReq2FA(A, bistReqKeySize_f, &req, &key_size);
                GetLpmTcamBistReq2FA(A, bistReqKey_f,     &req, &key);
                CTC_DKIT_PRINT_DEBUG(" %-52s %u: LpmTcamBistReq2FA[0].bistReqKeyValid:%u bistReqKeySize:%u\n",
                                     __FUNCTION__, __LINE__, p_capture_info->tbl[i].lookup_valid, key_size);
            }

            if (p_capture_info->tbl[i].lookup_valid)
            {
                if (CTC_DKITS_TCAM_IP_TCAM_BLOCK(block))
                {
                    if (key_size > 2)
                    {
                        CTC_DKIT_PRINT(" %-52s %u: IP LKP0/1 invalid key size:%u\n", __FUNCTION__, __LINE__, key_size);
                        return CLI_ERROR;
                    }
                    _ctc_usw_dkits_tcam_xbits2key(lchip, tcam_mem[block][key_size], &key, &ds);
                    GetDsLpmTcamIpv4HalfKeyLookup1(A, lpmTcamKeyType_f, &key, &key_type);
                }
                else
                {
                    if ((2 != key_size) && (3 != key_size))
                    {
                        CTC_DKIT_PRINT(" %-52s %u: IP NAT/PBR invalid key size:%u\n", __FUNCTION__, __LINE__, key_size);
                        return CLI_ERROR;
                    }
                    _ctc_usw_dkits_tcam_xbits2key(lchip, tcam_mem[block][key_size], &key, &ds);
                    GetDsLpmTcamIpv4PbrDoubleKey(A, lpmTcamKeyType_f, &key, &key_type);
                }

                CTC_DKITS_TCAM_ERROR_RETURN(ctc_usw_dkits_tcam_mem2tbl(lchip, tcam_mem[block][key_size], key_type, &tblid));
                p_capture_info->tbl[i].id = tblid;
                CTC_DKIT_PRINT_DEBUG(" %-52s %u: key_type:%u, %s\n", __FUNCTION__, __LINE__, key_type, TABLE_NAME(lchip, tblid));
            }
            sal_memcpy(&capture_ds[i], &ds, sizeof(ds_t));
        }
    }
    else if (DKITS_TCAM_BLOCK_TYPE_CATEGORYID == block)
    {
        for (i = 0; i < TABLE_MAX_INDEX(lchip, IpeCidTcamBistReq_t); i++)
        {
            cmd = DRV_IOR(IpeCidTcamBistReq_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &ds);

            p_capture_info->tbl[i].lookup_valid = GetIpeCidTcamBistReq(V, valid_f, &ds);
            GetIpeCidTcamBistReq(A, keySize_f, &ds, &key_size);
            CTC_DKIT_PRINT_DEBUG(" %-52s %u: IpeCidTcamBistReq[0].valid:%u, keySize:%u\n",
                                 __FUNCTION__, __LINE__, p_capture_info->tbl[i].lookup_valid, key_size);

            p_capture_info->tbl[i].id = DsCategoryIdPairTcamKey_t;
            sal_memcpy(&capture_ds[i], &ds, sizeof(ds_t));
        }
    }
    else if (DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE == block)
    {
        for (i = 0; i < TABLE_MAX_INDEX(lchip, QMgrEnqTcamBistReqFa_t); i++)
        {
            cmd = DRV_IOR(QMgrEnqTcamBistReqFa_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &ds);

            p_capture_info->tbl[i].lookup_valid = GetQMgrEnqTcamBistReqFa(V, bistReqKeyValid_f, &ds);
            GetQMgrEnqTcamBistReqFa(A, bistReqKeySize_f,  &ds, &key_size);

            CTC_DKIT_PRINT_DEBUG(" %-52s %u: QMgrEnqTcamBistReqFa[0].valid:%u, keySize:%u\n",
                                 __FUNCTION__, __LINE__, p_capture_info->tbl[i].lookup_valid, key_size);

            p_capture_info->tbl[i].lookup_valid = 1;
            p_capture_info->tbl[i].id = DsQueueMapTcamKey_t;
            sal_memcpy(&capture_ds[i], &ds, sizeof(ds_t));
        }
    }
    else
    {
        return CLI_ERROR;
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_capture_deliver_result(uint8 lchip, dkits_tcam_block_type_t block, dkits_tcam_capture_info_t* p_capture_info)
{
    uint32 i = 0, j = 0, index = 0, hit = 0, idx = 0;
    uint32 cmd = 0, blk = 0;
    ds_t   rlt = {0};

    if (CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block))
    {
        for (i = 0; i < TABLE_MAX_INDEX(lchip, FlowTcamBistResultFA_t); i++)
        {
            if (!p_capture_info->tbl[i].lookup_valid)
            {
                continue;
            }

            cmd = DRV_IOR(FlowTcamBistResultFA_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &rlt);

            GetFlowTcamBistResultFA(A, realIndex_f,    &rlt, &index);
            GetFlowTcamBistResultFA(A, realIndexHit_f, &rlt, &hit);

            CTC_DKIT_PRINT_DEBUG(" %-52s %u: FlowTcamBistResultFA[%u].realIndex:0x%04X realIndexHit:%u\n",
                                 __FUNCTION__, __LINE__, i, index, hit);

            if (hit)
            {
                blk = DKITS_TCAM_FLOW_RLT_TCAMID(index);
                _ctc_usw_dkits_tcam_tbl_index(p_capture_info->tbl[i].id, &blk, DKITS_TCAM_FLOW_RLT_TCAMIDX(index), &idx);
                p_capture_info->tbl[i].idx = idx;
                if (CTC_DKITS_INVALID_INDEX != idx)
                {
                    p_capture_info->tbl[i].result_valid = 1;
                }
            }
        }
    }
    else if (CTC_DKITS_TCAM_IP_TCAM_BLOCK(block) || CTC_DKITS_TCAM_NAT_TCAM_BLOCK(block))
    {
        j = block - DKITS_TCAM_BLOCK_TYPE_LPM_LKP0;

        for (i = 0; i < TABLE_MAX_INDEX(lchip, LpmTcamBistResult0FA_t + j); i++)
        {
            cmd = DRV_IOR(LpmTcamBistResult0FA_t + j, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &rlt);

            GetLpmTcamBistResult0FA(A, realIndex0_f,    &rlt, &index);
            GetLpmTcamBistResult0FA(A, realIndex0Hit_f, &rlt, &hit);

            CTC_DKIT_PRINT_DEBUG(" %-52s %u: LpmTcamBistResult%uFA[%u].realIndex:0x%04X, hit:%u\n",
                                 __FUNCTION__, __LINE__, j, i, index, hit);

            if (hit)
            {
                if ((DKITS_TCAM_BLOCK_TYPE_LPM_LKP0 == block) || (DKITS_TCAM_BLOCK_TYPE_LPM_LKP1 == block))
                {
                    blk = DKITS_TCAM_IP_RLT_TCAMID(index);
                    _ctc_usw_dkits_tcam_tbl_index(p_capture_info->tbl[i].id, &blk, DKITS_TCAM_IP_RLT_TCAMIDX(index), &idx);
                    p_capture_info->tbl[i].idx = idx;
                }
                else
                {
                    blk = DKITS_TCAM_NAT_RLT_TCAMID(index);
                    _ctc_usw_dkits_tcam_tbl_index(p_capture_info->tbl[i].id, &blk, DKITS_TCAM_NAT_RLT_TCAMIDX(index), &idx);
                    p_capture_info->tbl[i].idx = idx;
                }

                if (CTC_DKITS_INVALID_INDEX != idx)
                {
                    p_capture_info->tbl[i].result_valid = 1;
                }
            }
        }
    }
    else if (DKITS_TCAM_BLOCK_TYPE_CATEGORYID == block)
    {
        for (i = 0; i < TABLE_MAX_INDEX(lchip, IpeCidTcamBistResult_t); i++)
        {
            cmd = DRV_IOR(IpeCidTcamBistResult_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &rlt);

            GetIpeCidTcamBistResult(A, readIndex_f,    &rlt, &index);
            GetIpeCidTcamBistResult(A, readIndexHit_f, &rlt, &hit);

            CTC_DKIT_PRINT_DEBUG(" %-52s %u: IpeCidTcamBistResult[%u].realIndex:0x%04X, hit:%u\n",
                                 __FUNCTION__, __LINE__, i, index, hit);

            if (hit)
            {
                p_capture_info->tbl[i].idx = index;
                p_capture_info->tbl[i].result_valid = 1;

            }
        }
    }
    else if (DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE == block)
    {
        for (i = 0; i < TABLE_MAX_INDEX(lchip, QMgrEnqTcamBistResultFa_t); i++)
        {
            cmd = DRV_IOR(QMgrEnqTcamBistResultFa_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, i, cmd, &rlt);

            GetQMgrEnqTcamBistResultFa(A, realIndex_f,    &rlt, &index);
            GetQMgrEnqTcamBistResultFa(A, realIndexHit_f, &rlt, &hit);

            CTC_DKIT_PRINT_DEBUG(" %-52s %u: QMgrEnqTcamBistResultFa[%u].realIndex:0x%04X, hit:%u\n",
                                 __FUNCTION__, __LINE__, i, index, hit);

            if (hit)
            {
                p_capture_info->tbl[i].idx = index;
                p_capture_info->tbl[i].result_valid = 1;
            }
        }
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_capture_timestamp(dkits_tcam_block_type_t block, dkits_tcam_capture_tick_t tick, sal_time_t* p_stamp)
{
    static sal_time_t flow_time;
    static sal_time_t ip_lkp0_time;
    static sal_time_t ip_lkp1_time;
    static sal_time_t nat_time;
    static sal_time_t cid_time;
    static sal_time_t quemap_time;

    if (DKITS_TCAM_CAPTURE_TICK_BEGINE == tick)
    {
        if (CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block))
        {
            sal_time(&flow_time);
        }
        else if ((DKITS_TCAM_BLOCK_TYPE_LPM_LKP0 == block))
        {
            sal_time(&ip_lkp0_time);
        }
        else if ((DKITS_TCAM_BLOCK_TYPE_LPM_LKP1 == block))
        {
            sal_time(&ip_lkp1_time);
        }
        else if ((DKITS_TCAM_BLOCK_TYPE_NAT_PBR == block))
        {
            sal_time(&nat_time);
        }
        else if (DKITS_TCAM_BLOCK_TYPE_CATEGORYID == block)
        {
            sal_time(&cid_time);
        }
        else if (DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE == block)
        {
            sal_time(&quemap_time);
        }
    }
    else if (DKITS_TCAM_CAPTURE_TICK_TERMINATE == tick)
    {
        sal_time(p_stamp);
    }
    else if (DKITS_TCAM_CAPTURE_TICK_LASTEST == tick)
    {
        if (CTC_DKITS_TCAM_FLOW_TCAM_BLOCK(block))
        {
            sal_memcpy((uint8*)p_stamp, (uint8*)&flow_time, sizeof(sal_time_t));
        }
        else if (DKITS_TCAM_BLOCK_TYPE_LPM_LKP0 == block)
        {
            sal_memcpy((uint8*)p_stamp, (uint8*)&ip_lkp0_time, sizeof(sal_time_t));
        }
        else if (DKITS_TCAM_BLOCK_TYPE_LPM_LKP1 == block)
        {
            sal_memcpy((uint8*)p_stamp, (uint8*)&ip_lkp1_time, sizeof(sal_time_t));
        }
        else if (DKITS_TCAM_BLOCK_TYPE_NAT_PBR == block)
        {
            sal_memcpy((uint8*)p_stamp, (uint8*)&nat_time, sizeof(sal_time_t));
        }
        else if (DKITS_TCAM_BLOCK_TYPE_CATEGORYID == block)
        {
            sal_memcpy((uint8*)p_stamp, (uint8*)&cid_time, sizeof(sal_time_t));
        }
        else if (DKITS_TCAM_BLOCK_TYPE_SERVICE_QUEUE == block)
        {
            sal_memcpy((uint8*)p_stamp, (uint8*)&quemap_time, sizeof(sal_time_t));
        }
    }

    return  CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkits_tcam_capture_explore_result(uint8 lchip, dkits_tcam_block_type_t block, uint32 idx)
{
    int32  ret = CLI_SUCCESS;
    uint32 captured_vec = 0;
    uint32 no = 0;
    dkits_tcam_capture_info_t capture_info;

    captured_vec = _ctc_usw_dkits_tcam_capture_status(block, FALSE, 0);
    if (!DKITS_IS_BIT_SET(captured_vec, block))
    {
        CTC_DKIT_PRINT(" Tcam capture toward %s tcam block has not been started yet!\n", tcam_block_desc[block]);
        return CLI_SUCCESS;
    }

    CTC_DKIT_PRINT(" %-6s%-27s%-38s%-15s%s\n", "No.", "TcamModule", "TcamTable", "ResultValid", "KeyIndex");
    CTC_DKIT_PRINT(" -----------------------------------------------------------------------------------------------------\n");

    sal_memset(&capture_info, 0, sizeof(dkits_tcam_capture_info_t));
    ret = ret ? ret : _ctc_usw_dkits_tcam_capture_decap_request(lchip, block, &capture_info);
    ret = ret ? ret : _ctc_usw_dkits_tcam_capture_deliver_result(lchip, block, &capture_info);
    ret = ret ? ret : _ctc_usw_dkits_tcam_capture_present_result(lchip, &capture_info, &no, idx);

    return ret;
}

STATIC int32
_ctc_usw_dkits_tcam_check_exclusive(ctc_dkits_tcam_info_t* p_tcam_info)
{
    uint32 i = 0;
    uint32 exclude = 0;

    for (i = 0; i <= DKITS_TCAM_BLOCK_TYPE_EGS_ACL2; i++)
    {
        if (DKITS_IS_BIT_SET(p_tcam_info->tcam_type, i))
        {
            if (0 == exclude)
            {
                exclude = 1;
            }
            else
            {
                CTC_DKIT_PRINT("igs-scl, igs-acl and egs-acl tcam bist/capture should be exclusive with each other.\n");
                return CLI_ERROR;
            }
        }
    }

    return CLI_SUCCESS;
}

#define ________DKITS_TCAM_EXTERNAL_FUNCTION__________
int32
ctc_usw_dkits_tcam_scan(void* p_info)
{
    uint32 i = 0, j = 0, k = 0, tcam_type = 0;
    tbls_id_t tblid_tmp = MaxTblId_t - 1;
    ctc_dkits_tcam_info_t* p_tcam_info = NULL;
    dkits_tcam_block_type_t block = DKITS_TCAM_BLOCK_TYPE_INVALID;

    DKITS_PTR_VALID_CHECK(p_info);
    p_tcam_info = (ctc_dkits_tcam_info_t*)p_info;
    CTC_DKIT_LCHIP_CHECK(p_tcam_info->lchip);
    DRV_INIT_CHECK(p_tcam_info->lchip);
    if (1 == SDK_WORK_PLATFORM)
    {
        CTC_DKIT_PRINT("    UML do not support tcan scan!\n");
        return CLI_SUCCESS;
    }
    /*Reset*/
    for (i = 0; i < DKITS_TCAM_BLOCK_TYPE_NUM; i++)
    {
        tcam_type = 0;
        if (0 == DKITS_IS_BIT_SET(p_tcam_info->tcam_type, i))
        {
            continue;
        }
        DKITS_BIT_SET(tcam_type, i);
        _ctc_usw_dkits_tcam_info2block(p_tcam_info->lchip, tcam_type, 0, &block);
        CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_reset_detect(p_tcam_info->lchip, block, DKITS_TCAM_DETECT_TYPE_BIST));
    }

    k = 0;
    CTC_DKIT_PRINT("\n");
    CTC_DKIT_PRINT(" Tcam scan result:\n");
    for (i = 0; i < CTC_DKITS_TCAM_TYPE_FLAG_NUM; i++)
    {
        tcam_type = 0;
        if (0 == DKITS_IS_BIT_SET(p_tcam_info->tcam_type, i))
        {
            continue;
        }
        DKITS_BIT_SET(tcam_type, i);
        for (j = 0; j < DKITS_TCAM_PER_TYPE_PRIORITY_NUM; j++)
        {
            _ctc_usw_dkits_tcam_info2block(p_tcam_info->lchip, tcam_type, j, &block);
            if (DKITS_TCAM_BLOCK_TYPE_INVALID == block)
            {
                break;
            }

            _ctc_usw_dkits_tcam_bist_tbl_brief_status(&tblid_tmp, 0);
            CTC_DKIT_PRINT(" ----------------------------------------------------\n");
            k++;
            CTC_DKIT_PRINT(" %u. Scan %s\n", k, tcam_block_desc[block]);
            _ctc_usw_dkits_tcam_bist_scan_tcam_type(p_tcam_info, block);
            _ctc_usw_dkits_tcam_bist_tbl_brief_status(&tblid_tmp, 1);
            if (MaxTblId_t == tblid_tmp)
            {
                CTC_DKIT_PRINT("    Result fail!\n");
                CTC_DKIT_PRINT("    TCAM SCAN MisMatchCnt %u\n", p_tcam_info->mismatch_cnt);
                tblid_tmp = MaxTblId_t - 1;
                _ctc_usw_dkits_tcam_bist_tbl_brief_status(&tblid_tmp, 0);
            }
            else
            {
                CTC_DKIT_PRINT("    Result pass!\n");
            }
        }
    }
    CTC_DKIT_PRINT("\n");
    for (i = 0; i < DKITS_TCAM_BLOCK_TYPE_NUM; i++)
    {
        tcam_type = 0;
        if (0 == DKITS_IS_BIT_SET(p_tcam_info->tcam_type, i))
        {
            continue;
        }
        DKITS_BIT_SET(tcam_type, i);
        _ctc_usw_dkits_tcam_info2block(p_tcam_info->lchip, tcam_type, 0, &block);
        CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_reset_detect(p_tcam_info->lchip, block, DKITS_TCAM_DETECT_TYPE_BIST));
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_dkits_tcam_capture_start(void* p_info)
{
    int32  ret = CLI_SUCCESS;
    uint32 i = 0, tcam_type = 0, delay = 0;
    ctc_dkits_tcam_info_t* p_tcam_info = NULL;
    dkits_tcam_block_type_t block = DKITS_TCAM_BLOCK_TYPE_INVALID;

    DKITS_PTR_VALID_CHECK(p_info);
    p_tcam_info = (ctc_dkits_tcam_info_t*)p_info;
    CTC_DKIT_LCHIP_CHECK(p_tcam_info->lchip);
    DRV_INIT_CHECK(p_tcam_info->lchip);

    ret = _ctc_usw_dkits_tcam_check_exclusive(p_tcam_info);
    if (CLI_SUCCESS != ret)
    {
        return CLI_SUCCESS;
    }

    for (i = 0; i < CTC_DKITS_TCAM_TYPE_FLAG_NUM; i++)
    {
        tcam_type = 0;
        if (DKITS_IS_BIT_SET(p_tcam_info->tcam_type, i))
        {
            DKITS_BIT_SET(tcam_type, i);
            _ctc_usw_dkits_tcam_info2block(p_tcam_info->lchip, tcam_type, p_tcam_info->priority, &block);
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_reset_detect(p_tcam_info->lchip, block, DKITS_TCAM_DETECT_TYPE_CAPTURE));

            CTC_DKIT_PRINT_DEBUG(" %-52s %u: Start capture:%s", __FUNCTION__, __LINE__, tcam_block_desc[block]);
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_set_detect(p_tcam_info->lchip, block, DKITS_TCAM_DETECT_CAPTURE_EN,   1));
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_set_detect(p_tcam_info->lchip, block, DKITS_TCAM_DETECT_CAPTURE_ONCE, 1));
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_detect_delay(block, &delay, DKITS_TCAM_DETECT_TYPE_CAPTURE));
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_set_detect(p_tcam_info->lchip, block, DKITS_TCAM_DETECT_DELAY, delay));
            CTC_DKIT_PRINT_DEBUG("\n");
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_capture_timestamp(block, DKITS_TCAM_CAPTURE_TICK_BEGINE, NULL));
        }
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_dkits_tcam_capture_stop(void* p_info)
{
    int32  ret = CLI_SUCCESS;
    uint32 tcam_type = 0, i = 0;
    uint32 capture_en = 0;
    sal_time_t start, now;
    ctc_dkits_tcam_info_t* p_tcam_info = NULL;
    char  *p_str1 = NULL, *p_str2 = NULL, *p_tmp = NULL;
    dkits_tcam_block_type_t block = DKITS_TCAM_BLOCK_TYPE_INVALID;

    DKITS_PTR_VALID_CHECK(p_info);
    p_tcam_info = (ctc_dkits_tcam_info_t*)p_info;
    CTC_DKIT_LCHIP_CHECK(p_tcam_info->lchip);
    DRV_INIT_CHECK(p_tcam_info->lchip);

    ret = _ctc_usw_dkits_tcam_check_exclusive(p_tcam_info);
    if (CLI_SUCCESS != ret)
    {
        return CLI_SUCCESS;
    }

    for (i = 0; i < CTC_DKITS_TCAM_TYPE_FLAG_NUM; i++)
    {
        tcam_type = 0;
        if (DKITS_IS_BIT_SET(p_tcam_info->tcam_type, i))
        {
            DKITS_BIT_SET(tcam_type, i);
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_info2block(p_tcam_info->lchip, tcam_type, p_tcam_info->priority, &block));
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_get_detect(p_tcam_info->lchip, block, DKITS_TCAM_DETECT_CAPTURE_EN, &capture_en));

            if (!capture_en)
            {
                CTC_DKIT_PRINT(" Tcam capture toward %s has not been started yet!\n", tcam_block_desc[block]);
            }
            else
            {
                _ctc_usw_dkits_tcam_capture_status(block, TRUE, 1);
                CTC_DKIT_PRINT_DEBUG(" %-52s %u:", __FUNCTION__, __LINE__);
                CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_set_detect(p_tcam_info->lchip, block, DKITS_TCAM_DETECT_CAPTURE_EN, 0));
                CTC_DKIT_PRINT_DEBUG("\n");
                CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_capture_timestamp(block, DKITS_TCAM_CAPTURE_TICK_LASTEST, &start));

                sal_time(&now);
                p_str1 = sal_ctime(&start);
                p_str2 = sal_ctime(&now);
                p_tmp = sal_strchr(p_str1, '\n');
                if (NULL != p_tmp)
                {
                    *p_tmp = '\0';
                }
                p_tmp = p_str2 ? sal_strchr(p_str2, '\n') : NULL;
                if (NULL != p_tmp)
                {
                    *p_tmp = '\0';
                }
                CTC_DKIT_PRINT(" %s start capture timestamp %s, stop capture timestamp %s.\n", tcam_block_desc[block], p_str1, p_str2);
                CTC_DKIT_PRINT("\n");
                CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_capture_explore_result(p_tcam_info->lchip, block, 0xFFFFFFFF));
            }
        }
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_dkits_tcam_capture_show(void* p_info)
{
    int32  ret = CLI_SUCCESS;
    uint32 i = 0, tcam_type = 0;
    uint32 capture_en = 0;
    ctc_dkits_tcam_info_t* p_tcam_info = NULL;
    dkits_tcam_block_type_t block = DKITS_TCAM_BLOCK_TYPE_INVALID;

    DKITS_PTR_VALID_CHECK(p_info);
    p_tcam_info = (ctc_dkits_tcam_info_t*)p_info;
    CTC_DKIT_LCHIP_CHECK(p_tcam_info->lchip);
    DRV_INIT_CHECK(p_tcam_info->lchip);

    ret = _ctc_usw_dkits_tcam_check_exclusive(p_tcam_info);
    if (CLI_SUCCESS != ret)
    {
        return CLI_SUCCESS;
    }

    for (i = 0; i < CTC_DKITS_TCAM_TYPE_FLAG_NUM; i++)
    {
        tcam_type = 0;
        if (DKITS_IS_BIT_SET(p_tcam_info->tcam_type, i))
        {
            DKITS_BIT_SET(tcam_type, i);

            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_info2block(p_tcam_info->lchip, tcam_type, p_tcam_info->priority, &block));
            CTC_DKIT_PRINT_DEBUG(" %-52s %u:", __FUNCTION__, __LINE__);
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_get_detect(p_tcam_info->lchip, block, DKITS_TCAM_DETECT_CAPTURE_EN, &capture_en));
            CTC_DKIT_PRINT_DEBUG("\n");

            if (0 != capture_en)
            {
                CTC_DKIT_PRINT(" Please stop the capture of %s tcam type before show capture result!\n", tcam_block_desc[block]);
                return CLI_SUCCESS;
            }
            CTC_DKITS_TCAM_ERROR_RETURN(_ctc_usw_dkits_tcam_capture_explore_result(p_tcam_info->lchip, block, p_tcam_info->index));
        }
    }

    return CLI_SUCCESS;
}

int32
ctc_usw_dkits_tcam_show_key_type(uint8 lchip)
{
    uint8  tcam_type = 0, prio = 0, key_type = 0, t = 0, head = 0;
    uint32 tblnum = 0;
    int32 ret = 0;
    tbls_id_t tblid[DKITS_TCAM_PER_BLOCK_MAX_KEY_TYPE] = {MaxTblId_t};
    ctc_dkits_tcam_info_t tcam_info;
    dkits_tcam_block_type_t block_type = DKITS_TCAM_BLOCK_TYPE_INVALID;

    DRV_INIT_CHECK(lchip);
    CTC_DKIT_PRINT("\n");
    CTC_DKIT_PRINT(" %-24s%-14s%s\n", "TcamType", "KeyType", "TcamTblName");
    CTC_DKIT_PRINT(" %s\n", "-------------------------------------------------------------------");

    for (tcam_type = 0; tcam_type < CTC_DKITS_TCAM_TYPE_FLAG_NUM; tcam_type++)
    {
        for (prio = 0; prio < DKITS_TCAM_PER_TYPE_PRIORITY_NUM; prio++)
        {
            _ctc_usw_dkits_tcam_info2block(lchip, (1 << tcam_type), prio, &block_type);
            if (DKITS_TCAM_BLOCK_TYPE_INVALID == block_type)
            {
                break;
            }
            head = 0;
            for (key_type = 0; key_type < DKITS_TCAM_PER_BLOCK_MAX_KEY_TYPE; key_type++)
            {
                tcam_info.tcam_type = 1 << tcam_type;
                tcam_info.priority = prio;
                tcam_info.key_type = key_type;
                _ctc_usw_dkits_tcam_key2tbl(lchip, &tcam_info, tblid, &tblnum);

                for (t = 0; t < tblnum; t++)
                {
                    ret = _ctc_usw_dkits_tcam_tbl2block(tblid[t], &block_type);
                    if ((0 == head) && (ret != CLI_ERROR))
                    {
                        head = 1;
                        CTC_DKIT_PRINT(" %-24s%-14u%s\n", tcam_block_desc[block_type], key_type, TABLE_NAME(lchip, tblid[t]));
                    }
                    else if (ret != CLI_ERROR)
                    {
                        CTC_DKIT_PRINT(" %-24s%-14u%s\n", "", key_type, TABLE_NAME(lchip, tblid[t]));
                    }
                }
            }
        }
    }
    CTC_DKIT_PRINT("\n");

    return CLI_SUCCESS;
}

