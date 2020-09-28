/**
 @file sys_usw_security.c

 @date 2010-2-26

 @version v2.0

---file comments----
*/

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_hash.h"
#include "ctc_port.h"
#include "ctc_vector.h"
#include "ctc_linklist.h"
#include "ctc_spool.h"
#include "ctc_const.h"
#include "ctc_field.h"
#include "ctc_macro.h"

#include "sys_usw_opf.h"
#include "sys_usw_security.h"
#include "sys_usw_port.h"
#include "sys_usw_vlan.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_chip.h"
#include "sys_usw_common.h"
#include "sys_usw_ftm.h"
#include "sys_usw_internal_port.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_internal_port.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_register.h"
#include "sys_usw_interrupt.h"

#include "drv_api.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/

/**
 @brief define hash lookup type for scl
*/
enum sys_port_scl_hash_type_e
{
    SYS_PORT_HASH_SCL_MAC_SA          = 0x6,    /**< MACSA */
    SYS_PORT_HASH_SCL_PORT_MAC_SA     = 0x7,    /**< Port+MACSA 160bit */
    SYS_PORT_HASH_SCL_IPV4_SA         = 0x8,    /**< Ip4SA */
    SYS_PORT_HASH_SCL_PORT_IPV4_SA    = 0x9,    /**< Port+IPv4SA */
    SYS_PORT_HASH_SCL_MAX
};
typedef enum sys_port_scl_hash_type_e sys_port_scl_hash_type_t;

enum sys_security_ipsg_group_sub_type_e
{

  SYS_SECURITY_IPSG_GROUP_SUB_TYPE_HASH,
       SYS_SECURITY_IPSG_GROUP_SUB_TYPE_HASH1,
       SYS_SECURITY_IPSG_GROUP_SUB_TYPE_TCAM0,
       SYS_SECURITY_IPSG_GROUP_SUB_TYPE_TCAM1,
       SYS_SECURITY_IPSG_GROUP_SUB_TYPE_DEFAULT_TCAM0,
       SYS_SECURITY_IPSG_GROUP_SUB_TYPE_DEFAULT_TCAM1,
       SYS_SECURITY_IPSG_GROUP_SUB_TYPE_MAX
};
typedef enum sys_security_ipsg_group_sub_type_e sys_security_ipsg_group_sub_type_t;

/*default 64k*/




#define SYS_LEARN_LIMIT_BLOCK_SIZE  4

#define SYS_STMCTL_BLOCK_SIZE 4
#define SYS_STMCTL_BLOCK_NUM  16
#define SYS_STMCTL_MAX_PTR 128

#define SYS_STMCTL_CHK_UCAST(type) \
    { \
        if (CTC_SECURITY_STORM_CTL_KNOWN_UCAST == type  \
            || CTC_SECURITY_STORM_CTL_ALL_UCAST == type)\
        { \
            SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");\
			return CTC_E_NOT_SUPPORT;\
 \
        } \
    }

#define SYS_STMCTL_CHK_MCAST(mode, type) \
    { \
        if (mode) \
        { \
            if (CTC_SECURITY_STORM_CTL_ALL_MCAST == type) \
            { \
                SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "[STMCTL] Invalid mcast storm ctl mode \n");\
                return CTC_E_INVALID_CONFIG; \
            } \
        } \
        else \
        { \
            if (CTC_SECURITY_STORM_CTL_KNOWN_MCAST == type \
                || CTC_SECURITY_STORM_CTL_UNKNOWN_MCAST == type) \
            { \
                SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "[STMCTL] Invalid mcast storm ctl mode \n");\
                return CTC_E_INVALID_CONFIG; \
            } \
        } \
    }

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
enum sys_usw_security_ipsg_flag_e
{
    SYS_SECURITY_IPSG_MAC_SUPPORT = 0x01,
    SYS_SECURITY_IPSG_IPV4_SUPPORT = 0x02,
    SYS_SECURITY_IPSG_IPV6_SUPPORT = 0x04,
    SYS_SECURITY_IPSG_MAX
};
typedef enum sys_usw_security_ipsg_flag_e sys_usw_security_ipsg_flag_t;

enum sys_usw_security_stmctl_mode_e
{
    SYS_USW_SECURITY_MODE_MERGE,
    SYS_USW_SECURITY_MODE_SEPERATE,
    SYS_USW_SECURITY_MODE_NUM
};
typedef enum sys_usw_security_stmctl_mode_e sys_usw_security_stmctl_mode_t;

struct sys_stmctl_profile_s
{
    uint8  op;  /*port or vlan*/
    uint8  is_y;

    uint8  rate_frac;
    uint8  rate_shift;
    uint8  threshold_shift;
    uint8  rsv[3];
    uint16 threshold;
    uint16 rate;

    uint32 profile_id;
};
typedef struct sys_stmctl_profile_s sys_stmctl_profile_t;

struct sys_learn_limit_trhold_node_s
{
    uint32  value;
    uint32  index;
};
typedef struct sys_learn_limit_trhold_node_s sys_learn_limit_trhold_node_t;

struct sys_security_master_s
{
    ctc_security_stmctl_glb_cfg_t stmctl_cfg;
    uint8  flag;     /*ipsg*/
    uint8  limit_opf_type;
    uint8  stmctl_profile0_opf_type;
    uint8  stmctl_profile1_opf_type;
    uint8  storm_ctl_opf_type;
    uint8  tcam_group[2];    /* create scl tcam group once*/
    uint8  hash_group[2];    /* create scl hash group once*/
    uint8  ip_src_guard_def;    /* add default entry when add first entry */
    uint8  rsv[2];
    ctc_spool_t* trhold_spool;
    ctc_spool_t* stmctl_spool;
    sal_mutex_t* sec_mutex;   /**< mutex for security */
    uint8  port_id[SYS_STMCTL_MAX_PTR];  /* port stormctl ptr map port id*/
    uint16  vlan_id[SYS_STMCTL_MAX_PTR]; /* vlan stormctl ptr map vlan id */
};
typedef struct sys_security_master_s sys_security_master_t;

sys_security_master_t* usw_security_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#define SYS_SECURITY_LOCK(lchip) \
    if(usw_security_master[lchip]->sec_mutex) sal_mutex_lock(usw_security_master[lchip]->sec_mutex)

#define SYS_SECURITY_UNLOCK(lchip) \
    if(usw_security_master[lchip]->sec_mutex) sal_mutex_unlock(usw_security_master[lchip]->sec_mutex)

#define CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, op) \
    do \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            sal_mutex_unlock(usw_security_master[lchip]->sec_mutex); \
            return rv; \
        } \
    } while (0)

#define SYS_SECURITY_INIT_CHECK(lchip) \
    do { \
        SYS_LCHIP_CHECK_ACTIVE(lchip); \
        if (usw_security_master[lchip] == NULL){ \
            SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
			return CTC_E_NOT_INIT;\
 } \
    } while (0)

#define SYS_LEARN_LIMIT_NUM_CHECK(LIMIT_NUM)\
    do {    \
            if (((LIMIT_NUM) > MCHIP_CAP(SYS_CAP_LEARN_LIMIT_MAX) ) && ((LIMIT_NUM) != 0xFFFFFFFF)) { \
                SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");\
                return CTC_E_INVALID_PARAM; }\
    } while (0)

#define MIX(a,b,c)                  \
do                                  \
{                                   \
    a -= b; a -= c; a ^= (c>>13);   \
    b -= c; b -= a; b ^= (a<<8);    \
    c -= a; c -= b; c ^= (b>>13);   \
    a -= b; a -= c; a ^= (c>>12);   \
    b -= c; b -= a; b ^= (a<<16);   \
    c -= a; c -= b; c ^= (b>>5);    \
    a -= b; a -= c; a ^= (c>>3);    \
    b -= c; b -= a; b ^= (a<<10);   \
    c -= a; c -= b; c ^= (b>>15);   \
} while(0)

STATIC int32
_sys_usw_mac_security_get_learn_limit(uint8 lchip, ctc_security_learn_limit_t* p_learning_limit, uint32* p_running_num);
STATIC int32
_sys_usw_mac_security_set_port_mac_limit(uint8 lchip, uint32 gport, ctc_maclimit_action_t action);
STATIC int32
_sys_usw_mac_security_set_vlan_mac_limit(uint8 lchip, uint16 vlan_id, ctc_maclimit_action_t action);
STATIC int32
_sys_usw_mac_security_set_system_mac_limit(uint8 lchip, ctc_maclimit_action_t action);
STATIC int32
_sys_usw_security_stmctl_build_index(sys_stmctl_profile_t* p_node, uint8* p_lchip);
STATIC int32
_sys_usw_security_stmctl_free_index(sys_stmctl_profile_t* p_node, uint8* p_lchip);

/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
#define __________Security_Hash_____________
STATIC uint32
_sys_usw_learn_limit_hash_make(sys_learn_limit_trhold_node_t* p_node)
{
    uint32 a, b, c;

    a = b = c = 0xdeadbeef + (((uint32)96) << 2);

    a += p_node->value;
    MIX(a, b, c);

    return (c);
}

STATIC bool
_sys_usw_learn_limit_hash_cmp(sys_learn_limit_trhold_node_t* p_bucket, sys_learn_limit_trhold_node_t* p_new)
{
    if (!p_bucket || !p_new)
    {
        return FALSE;
    }

    if (p_bucket->value != p_new->value)
    {
        return FALSE;
    }

    return TRUE;
}

STATIC uint32
_sys_usw_security_stmctl_hash_make(sys_stmctl_profile_t* p_node)
{
    uint32 size;
    uint8  * k;

    CTC_PTR_VALID_CHECK(p_node);

    size = sizeof(sys_stmctl_profile_t) - sizeof(uint32);
    k    = (uint8 *) p_node;
    return ctc_hash_caculate(size, k);
}

STATIC bool
_sys_usw_security_stmctl_hash_cmp(sys_stmctl_profile_t* p_node0, sys_stmctl_profile_t* p_node1)
{
    uint32 size = 0;
    if (!p_node0 || !p_node1)
    {
        return FALSE;
    }

    size = sizeof(sys_stmctl_profile_t) - sizeof(uint32);
    if(!sal_memcmp(p_node0, p_node1, size))
    {
        return TRUE;
    }

    return FALSE;
}

#define __________Security_Opf_____________
STATIC int32
_sys_usw_security_build_limit_index(sys_learn_limit_trhold_node_t* p_node, uint8* p_lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type  = usw_security_master[*p_lchip]->limit_opf_type;

    if (CTC_WB_ENABLE && (CTC_WB_STATUS(*p_lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_node->index));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
        p_node->index = value_32;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_security_free_limit_index(sys_learn_limit_trhold_node_t* p_node, uint8* p_lchip)
{
    uint32 cmd       = 0;
    uint32 max_limit = 0;
    uint8 lchip = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);

    lchip = *p_lchip;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type  = usw_security_master[*p_lchip]->limit_opf_type;

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, p_node->index));

    max_limit = MCHIP_CAP(SYS_CAP_LEARN_LIMIT_MAX);


    cmd = DRV_IOW(MacLimitThreshold_t, MacLimitThreshold_maxCount_f);
    CTC_ERROR_RETURN(DRV_IOCTL(*p_lchip, p_node->index, cmd, &max_limit));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_security_spool_init(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_spool_t spool;

    sys_learn_limit_trhold_node_t static_node;

    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    sal_memset(&static_node, 0, sizeof(sys_learn_limit_trhold_node_t));

    spool.lchip = lchip;
    spool.block_num = MCHIP_CAP(SYS_CAP_LEARN_LIMIT_PROFILE_NUM) / SYS_LEARN_LIMIT_BLOCK_SIZE;
    spool.block_size = SYS_LEARN_LIMIT_BLOCK_SIZE;
    /*include static node, add 1*/
    spool.max_count = MCHIP_CAP(SYS_CAP_LEARN_LIMIT_PROFILE_NUM) ;
    spool.user_data_size = sizeof(sys_learn_limit_trhold_node_t);
    spool.spool_key = (hash_key_fn)_sys_usw_learn_limit_hash_make;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_learn_limit_hash_cmp;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_security_build_limit_index;
    spool.spool_free  = (spool_free_fn)_sys_usw_security_free_limit_index;
    usw_security_master[lchip]->trhold_spool = ctc_spool_create(&spool);
    if (NULL == usw_security_master[lchip]->trhold_spool)
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto error0;
    }

    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = SYS_STMCTL_BLOCK_SIZE;
    spool.block_size = SYS_STMCTL_BLOCK_NUM;
    spool.max_count = MCHIP_CAP(SYS_CAP_STMCTL_MAC_COUNT);
    spool.user_data_size = sizeof(sys_stmctl_profile_t);
    spool.spool_key = (hash_key_fn)_sys_usw_security_stmctl_hash_make;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_security_stmctl_hash_cmp;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_security_stmctl_build_index;
    spool.spool_free  = (spool_free_fn)_sys_usw_security_stmctl_free_index;
    usw_security_master[lchip]->stmctl_spool= ctc_spool_create(&spool);
    if (NULL == usw_security_master[lchip]->stmctl_spool)
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto error1;
    }

    static_node.value = 0xFFFFFFFF;
    /*opf alloc from 1*/
    static_node.index = 0;
    CTC_ERROR_GOTO(ctc_spool_static_add(usw_security_master[lchip]->trhold_spool, &static_node), ret, error2);

    return CTC_E_NONE;

error2:
    ctc_spool_free(usw_security_master[lchip]->stmctl_spool);
    usw_security_master[lchip]->stmctl_spool = NULL;
error1:
    ctc_spool_free(usw_security_master[lchip]->trhold_spool);
    usw_security_master[lchip]->trhold_spool = NULL;
error0:
    return ret;
}

STATIC int32
_sys_usw_security_spool_deinit(uint8 lchip)
{
    ctc_spool_free(usw_security_master[lchip]->stmctl_spool);
    usw_security_master[lchip]->stmctl_spool = NULL;

    ctc_spool_free(usw_security_master[lchip]->trhold_spool);
    usw_security_master[lchip]->trhold_spool = NULL;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_security_opf_init(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    uint32 entry_num = 0;
    sys_usw_opf_t opf;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &usw_security_master[lchip]->limit_opf_type, 1, "opf-learn-limit"), ret, error0);

    opf.pool_type = usw_security_master[lchip]->limit_opf_type;
    opf.pool_index = 0;
    /* start offset is 1 , 0 is reserve for unlimit :MCHIP_CAP(SYS_CAP_LEARN_LIMIT_MAX) */
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 1, MCHIP_CAP(SYS_CAP_LEARN_LIMIT_PROFILE_NUM) - 1),ret,error1);

    /*port and vlan*/
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsIpeStormCtl0Config_t, &entry_num), ret, error1);

    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &usw_security_master[lchip]->storm_ctl_opf_type, 2, "opf-storm-ctl"), ret, error1);

    opf.pool_type = usw_security_master[lchip]->storm_ctl_opf_type;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 0, entry_num / 2), ret, error2);

    opf.pool_index = 1;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 0, entry_num / 2), ret, error2);

    /*port: profileX and profileY*/
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsIpeStormCtl0ProfileX_t, &entry_num), ret, error2);

    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &usw_security_master[lchip]->stmctl_profile0_opf_type, 2, "opf-port-stmctl-profile"), ret, error2);

    opf.pool_type = usw_security_master[lchip]->stmctl_profile0_opf_type;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 1, entry_num - 1), ret, error3);

    opf.pool_index = 1;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 1, entry_num - 1), ret, error3);

    /*vlan: profileX and profileY*/
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsIpeStormCtl1ProfileX_t, &entry_num), ret, error3);

    CTC_ERROR_GOTO(sys_usw_opf_init(lchip, &usw_security_master[lchip]->stmctl_profile1_opf_type, 2, "opf-vlan-stmctl-profile"), ret, error3);

    opf.pool_type = usw_security_master[lchip]->stmctl_profile1_opf_type;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 1, entry_num - 1), ret, error4);

    opf.pool_index = 1;
    CTC_ERROR_GOTO(sys_usw_opf_init_offset(lchip, &opf, 1, entry_num - 1), ret, error4);

    return CTC_E_NONE;
error4:
    sys_usw_opf_deinit(lchip, usw_security_master[lchip]->stmctl_profile1_opf_type);
error3:
    sys_usw_opf_deinit(lchip, usw_security_master[lchip]->stmctl_profile0_opf_type);
error2:
    sys_usw_opf_deinit(lchip, usw_security_master[lchip]->storm_ctl_opf_type);
error1:
    sys_usw_opf_deinit(lchip, usw_security_master[lchip]->limit_opf_type);
error0:
    return ret;
}

STATIC int32
_sys_usw_security_opf_deinit(uint8 lchip)
{
    sys_usw_opf_deinit(lchip, usw_security_master[lchip]->stmctl_profile1_opf_type);

    sys_usw_opf_deinit(lchip, usw_security_master[lchip]->stmctl_profile0_opf_type);

    sys_usw_opf_deinit(lchip, usw_security_master[lchip]->storm_ctl_opf_type);

    sys_usw_opf_deinit(lchip, usw_security_master[lchip]->limit_opf_type);

    return CTC_E_NONE;
}

#define __________Learn_Limit_____________
/*mac limit mode: 0:Hw, 1:Sw*/
STATIC int32
_sys_usw_mac_limit_get_mode(uint8 lchip, uint8* p_mode)
{
    uint32 cmd = 0;
    IpeLearningCtl_m ipe_learn_ctl;

    sal_memset(&ipe_learn_ctl, 0, sizeof(IpeLearningCtl_m));

    cmd = DRV_IOR(IpeLearningCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_learn_ctl));

    *p_mode = !(GetIpeLearningCtl(V, hwPortMacLimitEn_f, &ipe_learn_ctl));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_set_syetem_security_learn_en(uint8 lchip, uint32 value)
{
    uint32 cmd = 0;

    value = (value) ? 0 : 1;
    cmd = DRV_IOW(IpeLookupCtl_t, IpeLookupCtl_learningDisable_f);

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_get_system_security_learn_en(uint8 lchip, uint32* value)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(value);
    cmd = DRV_IOR(IpeLookupCtl_t, IpeLookupCtl_learningDisable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, value));
    *value = (*value) ? 0 : 1;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_set_syetem_security_excep_en(uint8 lchip, uint32 value)
{
    uint32 cmd = 0;

    cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_systemSecurityExceptionEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_get_system_security_excep_en(uint8 lchip, uint32* value)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(value);

    cmd = DRV_IOR(IpeLearningCtl_t, IpeLearningCtl_systemSecurityExceptionEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, value));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_set_system_mac_security_discard_en(uint8 lchip, uint32 value, uint8 limit_mode)
{
    uint32 cmd = 0;

    /*Sw mode*/
    if (limit_mode)
    {
        cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_systemMacSecurityDiscard_f);
    }
    else
    {
        cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_systemMacSecurityEn_f);
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_get_system_mac_security_discard_en(uint8 lchip, uint32* value)
{
    uint32 cmd = 0;

    CTC_PTR_VALID_CHECK(value);

    cmd = DRV_IOR(IpeLearningCtl_t, IpeLearningCtl_systemMacSecurityDiscard_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, value));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_security_set_port_learn_limit(uint8 lchip, ctc_security_learn_limit_t* p_learning_limit)
{
    uint32 cmd = 0;
    uint32 index = 0;
    uint16 lport = 0;
    uint32 thrhld_index = 0;
    uint32 limit = 0;
    uint16 gport = 0;
    ctc_security_learn_limit_t runing_limit;
    uint32 runing_cnt = 0;
    sys_learn_limit_trhold_node_t new_node;
    sys_learn_limit_trhold_node_t old_node;
    sys_learn_limit_trhold_node_t* p_node_get = NULL;

    drv_acc_in_t  in;
    drv_acc_out_t out;

    sal_memset(&in,0,sizeof(drv_acc_in_t));
    sal_memset(&out,0,sizeof(drv_acc_out_t));
    sal_memset(&new_node, 0, sizeof(sys_learn_limit_trhold_node_t));
    sal_memset(&old_node, 0, sizeof(sys_learn_limit_trhold_node_t));

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (!CTC_IS_LINKAGG_PORT(p_learning_limit->gport))
    {
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_learning_limit->gport, lchip, lport);

        SYS_MAP_GCHIP_TO_LCHIP(CTC_MAP_GPORT_TO_GCHIP(p_learning_limit->gport), lchip);
        CTC_ERROR_RETURN(sys_usw_port_get_global_port(lchip, CTC_MAP_GPORT_TO_LPORT(p_learning_limit->gport), &gport));
    }
    else
    {
        gport = p_learning_limit->gport;
    }

    if (CTC_IS_LINKAGG_PORT(gport))
    {
	    if(DRV_IS_DUET2(lchip))
		{
            index =  gport & 0x3F;
	    }
		else
		{
			index =  gport & 0xFF;
		}
    }
    else
    {
        index = lport + MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM);
    }

    cmd = DRV_IOR(DsMacLimitThreshold_t, DsMacLimitThreshold_thresholdIndex_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &thrhld_index));

    /**max num 0xFFFFFFFF means disable*/
    if (p_learning_limit->limit_num == 0xFFFFFFFF)
    {
        thrhld_index = 0;
    }

    if (0 != thrhld_index)
    {   /*old exist.*/
        cmd = DRV_IOR(MacLimitThreshold_t, MacLimitThreshold_maxCount_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, thrhld_index, cmd, &limit));
    }
    else
    {
        limit = MCHIP_CAP(SYS_CAP_LEARN_LIMIT_MAX);
    }

    /*new == old, do nothing*/
    if (limit == p_learning_limit->limit_num)
    {
        return CTC_E_NONE;
    }

    new_node.value = p_learning_limit->limit_num;
    old_node.value = limit;
    CTC_ERROR_RETURN(ctc_spool_add(usw_security_master[lchip]->trhold_spool, &new_node, &old_node, &p_node_get));

    /*when p_learning_limit->limit_num == 0, spool will match static ndoe. index is 0*/
    thrhld_index = p_node_get->index;

    cmd = DRV_IOW(DsMacLimitThreshold_t, DsMacLimitThreshold_thresholdIndex_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &thrhld_index));

    if(0 != thrhld_index)
    {
        cmd = DRV_IOW(MacLimitThreshold_t, MacLimitThreshold_maxCount_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, thrhld_index, cmd, &(p_learning_limit->limit_num)));
    }

    /* clear discard bit */
    if (limit < p_learning_limit->limit_num)
    {
        in.tbl_id = DsMacSecurity_t;
        in.type = DRV_ACC_TYPE_ADD;
        in.op_type = DRV_ACC_OP_BY_PORT;
        in.gport = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(gport);
        in.limit_cnt = 0;
        CTC_BIT_SET(in.flag, DRV_ACC_MAC_LIMIT_STATUS);
        drv_acc_api(lchip, &in, &out);
    }
    else
    {
        /* get running count */
        sal_memset(&runing_limit, 0, sizeof(ctc_security_learn_limit_t));
        runing_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_PORT;
        runing_limit.gport = p_learning_limit->gport;
        CTC_ERROR_RETURN(_sys_usw_mac_security_get_learn_limit(lchip, &runing_limit, &runing_cnt));

        if (runing_cnt >= p_learning_limit->limit_num)
        {
            in.tbl_id = DsMacSecurity_t;
            in.type = DRV_ACC_TYPE_ADD;
            in.op_type = DRV_ACC_OP_BY_PORT;
            in.gport = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(gport);
            in.limit_cnt = 1;
            CTC_BIT_SET(in.flag, DRV_ACC_MAC_LIMIT_STATUS);
            drv_acc_api(lchip, &in, &out);
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_security_get_port_learn_limit(uint8 lchip, ctc_security_learn_limit_t* p_learning_limit)
{
    uint32 cmd = 0;
    uint32 index = 0;
    uint16 lport = 0;
    uint32 thrhld_index = 0;
    uint32 limit = 0;
    uint32 discard_en = 0;
    uint32 except_en = 0;
    uint16 gport = 0;
    drv_acc_in_t  in;
    drv_acc_out_t out;

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&in,0,sizeof(drv_acc_in_t));
    sal_memset(&out,0,sizeof(drv_acc_out_t));

    if (!CTC_IS_LINKAGG_PORT(p_learning_limit->gport))
    {
        SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(p_learning_limit->gport, lchip, lport);

        SYS_MAP_GCHIP_TO_LCHIP(CTC_MAP_GPORT_TO_GCHIP(p_learning_limit->gport), lchip);
        CTC_ERROR_RETURN(sys_usw_port_get_global_port(lchip, lport, &gport));
    }
    else
    {
        gport = p_learning_limit->gport;
    }

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        if(DRV_IS_DUET2(lchip))
		{
		    index =  gport & 0x3F;
	    }
		else
		{
		    index =  gport & 0xFF;
		}
    }
    else
    {
         index = lport + MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM);
    }

    in.tbl_id = DsMacLimitCount_t;
    in.type = DRV_ACC_TYPE_LOOKUP;
    in.op_type = DRV_ACC_OP_BY_PORT;
    in.gport = SYS_MAP_CTC_GPORT_TO_DRV_GPORT(gport);

    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    p_learning_limit->limit_count= out.query_cnt;

    cmd = DRV_IOR(DsMacLimitThreshold_t, DsMacLimitThreshold_thresholdIndex_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &thrhld_index));

    cmd = DRV_IOR(MacLimitThreshold_t, MacLimitThreshold_maxCount_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, thrhld_index, cmd, &limit));
    p_learning_limit->limit_num = limit;

    if (!CTC_IS_LINKAGG_PORT(p_learning_limit->gport))
    {
        CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, p_learning_limit->gport,
                         SYS_PORT_PROP_MAC_SECURITY_DISCARD, &discard_en));
        CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, p_learning_limit->gport,
                         SYS_PORT_PROP_SECURITY_EXCP_EN, &except_en));
    }

    if (discard_en && except_en)
    {
        p_learning_limit->limit_action= CTC_MACLIMIT_ACTION_TOCPU;
    }
    else if (discard_en)
    {
        p_learning_limit->limit_action = CTC_MACLIMIT_ACTION_DISCARD;
    }
    else
    {
        p_learning_limit->limit_action = CTC_MACLIMIT_ACTION_FWD;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_security_set_vlan_learn_limit(uint8 lchip, ctc_security_learn_limit_t* p_learning_limit)
{
    uint32 cmd = 0;
    uint32 index = 0;
    uint32 thrhld_index = 0;
    uint32 limit = 0;
    drv_acc_in_t  in;
    drv_acc_out_t out;
    ctc_security_learn_limit_t runing_limit;
    uint32 runing_cnt = 0;
    uint16 fid = 0;
    sys_learn_limit_trhold_node_t new_node;
    sys_learn_limit_trhold_node_t old_node;
    sys_learn_limit_trhold_node_t* p_node_get = NULL;

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&in,0,sizeof(drv_acc_in_t));
    sal_memset(&out,0,sizeof(drv_acc_out_t));
    sal_memset(&new_node, 0, sizeof(sys_learn_limit_trhold_node_t));
    sal_memset(&old_node, 0, sizeof(sys_learn_limit_trhold_node_t));

    index = MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM) + 256 + (p_learning_limit->vlan & 0xFFF);

    cmd = DRV_IOR(DsMacLimitThreshold_t, DsMacLimitThreshold_thresholdIndex_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &thrhld_index));
    /**max num 0xFFFFFFFF means disable*/
    if (p_learning_limit->limit_num == 0xFFFFFFFF)
    {
        thrhld_index = 0;
    }

    if (0 != thrhld_index)
    {
        cmd = DRV_IOR(MacLimitThreshold_t, MacLimitThreshold_maxCount_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, thrhld_index, cmd, &limit));
    }
    else
    {
        limit = MCHIP_CAP(SYS_CAP_LEARN_LIMIT_MAX);
    }

    /*new == old, do nothing.*/
    if (limit == p_learning_limit->limit_num)
    {
        return CTC_E_NONE;
    }

    new_node.value = p_learning_limit->limit_num;
    old_node.value = limit;
    CTC_ERROR_RETURN(ctc_spool_add(usw_security_master[lchip]->trhold_spool, &new_node, &old_node, &p_node_get));

    /*when p_learning_limit->limit_num == 0, spool will match static ndoe. index is 0*/
    thrhld_index = p_node_get->index;

    cmd = DRV_IOW(DsMacLimitThreshold_t, DsMacLimitThreshold_thresholdIndex_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &thrhld_index));

    if(0 != thrhld_index)
    {
        cmd = DRV_IOW(MacLimitThreshold_t, MacLimitThreshold_maxCount_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, thrhld_index, cmd, &(p_learning_limit->limit_num)));
    }

    /* clear discard bit */
    if (limit < p_learning_limit->limit_num)
    {
        fid =  p_learning_limit->vlan;
        in.tbl_id = DsMacSecurity_t;
        in.type = DRV_ACC_TYPE_ADD;
        in.op_type = DRV_ACC_OP_BY_VLAN;
        in.data = &fid;
        in.limit_cnt = 0;
        CTC_BIT_SET(in.flag, DRV_ACC_MAC_LIMIT_STATUS);
        drv_acc_api(lchip, &in, &out);
    }
    else
    {
        /* get running count */
        sal_memset(&runing_limit, 0, sizeof(ctc_security_learn_limit_t));
        runing_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_VLAN;
        runing_limit.vlan = p_learning_limit->vlan;
        CTC_ERROR_RETURN(_sys_usw_mac_security_get_learn_limit(lchip, &runing_limit, &runing_cnt));

        if (runing_cnt >= p_learning_limit->limit_num)
        {
            fid =  p_learning_limit->vlan;
            in.tbl_id = DsMacSecurity_t;
            in.type = DRV_ACC_TYPE_ADD;
            in.op_type = DRV_ACC_OP_BY_VLAN;
            in.data = &fid;
            in.limit_cnt = 1;
            CTC_BIT_SET(in.flag, DRV_ACC_MAC_LIMIT_STATUS);
            drv_acc_api(lchip, &in, &out);
        }
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_security_get_vlan_learn_limit(uint8 lchip, ctc_security_learn_limit_t* p_learning_limit)
{
    uint32 cmd = 0;
    uint32 index = 0;
    uint32 thrhld_index = 0;
    uint32 limit = 0;
    uint32 discard_en = 0;
    uint32 except_en = 0;
    drv_acc_in_t  in;
    drv_acc_out_t out;

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    index = MCHIP_CAP(SYS_CAP_LINKAGG_GROUP_NUM) + 256 + (p_learning_limit->vlan & 0xFFF);

    sal_memset(&in,0,sizeof(drv_acc_in_t));
    sal_memset(&out,0,sizeof(drv_acc_out_t));


    in.tbl_id = DsMacLimitCount_t;
    in.type = DRV_ACC_TYPE_LOOKUP;
    in.op_type = DRV_ACC_OP_BY_VLAN;
    in.data = &(p_learning_limit->vlan);
    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));
    p_learning_limit->limit_count= out.query_cnt;


    cmd = DRV_IOR(DsMacLimitThreshold_t, DsMacLimitThreshold_thresholdIndex_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &thrhld_index));

    cmd = DRV_IOR(MacLimitThreshold_t, MacLimitThreshold_maxCount_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, thrhld_index, cmd, &limit));
    p_learning_limit->limit_num = limit;

    CTC_ERROR_RETURN(sys_usw_vlan_get_internal_property(lchip, p_learning_limit->vlan,
        SYS_VLAN_PROP_MAC_SECURITY_VLAN_DISCARD, &discard_en));
    CTC_ERROR_RETURN(sys_usw_vlan_get_internal_property(lchip, p_learning_limit->vlan,
        SYS_VLAN_PROP_MAC_SECURITY_VLAN_EXCEPTION,&except_en));

    if (discard_en && except_en)
    {
        p_learning_limit->limit_action = CTC_MACLIMIT_ACTION_TOCPU;
    }
    else if (discard_en)
    {
        p_learning_limit->limit_action = CTC_MACLIMIT_ACTION_DISCARD;
    }
    else
    {
        p_learning_limit->limit_action = CTC_MACLIMIT_ACTION_FWD;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_security_set_system_learn_limit(uint8 lchip, ctc_security_learn_limit_t* p_learning_limit)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 limit = 0;
    uint32 loop = 0;
    uint32 tmp_value = 0;
    drv_acc_in_t  in;
    drv_acc_out_t out;
    ctc_security_learn_limit_t runing_limit;
    uint32 runing_cnt = 0;

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&in,0,sizeof(drv_acc_in_t));
    sal_memset(&out,0,sizeof(drv_acc_out_t));

    cmd = DRV_IOR(MacLimitSystem_t, MacLimitSystem_dynamicMaxCount_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &limit));


    if (p_learning_limit->limit_num == 0xFFFFFFFF)
    {
        value = MCHIP_CAP(SYS_CAP_LEARN_LIMIT_MAX);
    }
    else
    {
        value = p_learning_limit->limit_num;
    }

    for(loop = 0; loop < 20; loop ++)
    {
        cmd = DRV_IOW(MacLimitSystem_t, MacLimitSystem_dynamicMaxCount_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOR(MacLimitSystem_t, MacLimitSystem_dynamicMaxCount_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp_value));
        if(tmp_value == value)
        {
            break;
        }
        if(loop == 19)
        {
            return CTC_E_HW_FAIL;
        }
    }

    /* clear discard bit */
    if ( limit < value )
    {
        in.tbl_id = DsMacSecurity_t;
        in.type = DRV_ACC_TYPE_ADD;
        in.op_type = DRV_ACC_OP_BY_ALL;
        in.limit_cnt = 0;
        CTC_BIT_SET(in.flag, DRV_ACC_MAC_LIMIT_STATUS);
        drv_acc_api(lchip, &in, &out);
    }
    else
    {
        /* get running count */
        sal_memset(&runing_limit, 0, sizeof(ctc_security_learn_limit_t));
        runing_limit.limit_type = CTC_SECURITY_LEARN_LIMIT_TYPE_SYSTEM;
        CTC_ERROR_RETURN(_sys_usw_mac_security_get_learn_limit(lchip, &runing_limit, &runing_cnt));

        if (runing_cnt >= value)
        {
            in.tbl_id = DsMacSecurity_t;
            in.type = DRV_ACC_TYPE_ADD;
            in.op_type = DRV_ACC_OP_BY_ALL;
            in.limit_cnt = 1;
            CTC_BIT_SET(in.flag, DRV_ACC_MAC_LIMIT_STATUS);
            drv_acc_api(lchip, &in, &out);
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_security_get_system_learn_limit(uint8 lchip, ctc_security_learn_limit_t* p_learning_limit)
{
    uint32 cmd = 0;
    uint32 discard_en = 0;
    uint32 except_en = 0;
    uint32 value = 0;
    drv_acc_in_t  in;
    drv_acc_out_t out;

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&in,0,sizeof(drv_acc_in_t));
    sal_memset(&out,0,sizeof(drv_acc_out_t));

    in.tbl_id = DsMacLimitCount_t;
    in.type = DRV_ACC_TYPE_LOOKUP;
    in.op_type = DRV_ACC_OP_BY_ALL;

    drv_acc_api(lchip, &in, &out);

    p_learning_limit->limit_count= out.query_cnt;


    cmd = DRV_IOR(MacLimitSystem_t, MacLimitSystem_dynamicMaxCount_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    p_learning_limit->limit_num= value;

    cmd = DRV_IOR(IpeLearningCtl_t, IpeLearningCtl_systemMacSecurityEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &discard_en));

    cmd = DRV_IOR(IpeLearningCtl_t, IpeLearningCtl_systemSecurityExceptionEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &except_en));
    if (discard_en && except_en)
    {
        p_learning_limit->limit_action = CTC_MACLIMIT_ACTION_TOCPU;
    }
    else if (discard_en)
    {
        p_learning_limit->limit_action = CTC_MACLIMIT_ACTION_DISCARD;
    }
    else
    {
        p_learning_limit->limit_action = CTC_MACLIMIT_ACTION_FWD;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_security_set_learn_limit_num(uint8 lchip, ctc_security_learn_limit_t* p_learning_limit)
{
    uint8 limit_mode = 0;

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_ERROR_RETURN(_sys_usw_mac_limit_get_mode(lchip, &limit_mode));
    if (limit_mode)
    {
        /*Software do count, donot set limit num*/
        return CTC_E_NONE;
    }

    if (CTC_SECURITY_LEARN_LIMIT_TYPE_PORT == p_learning_limit->limit_type)
    {
        CTC_ERROR_RETURN(_sys_usw_mac_security_set_port_learn_limit(lchip, p_learning_limit));
    }
    else if (CTC_SECURITY_LEARN_LIMIT_TYPE_VLAN == p_learning_limit->limit_type )
    {
        CTC_ERROR_RETURN(_sys_usw_mac_security_set_vlan_learn_limit(lchip, p_learning_limit));
    }
    else if (CTC_SECURITY_LEARN_LIMIT_TYPE_SYSTEM == p_learning_limit->limit_type )
    {
        CTC_ERROR_RETURN(_sys_usw_mac_security_set_system_learn_limit(lchip, p_learning_limit));
    }
    else
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "[INTERRUPT] parameter is invalid \n");
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_security_set_learn_limit_action(uint8 lchip, ctc_security_learn_limit_t* p_learning_limit)
{
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if (CTC_SECURITY_LEARN_LIMIT_TYPE_PORT == p_learning_limit->limit_type)
    {
        CTC_ERROR_RETURN(_sys_usw_mac_security_set_port_mac_limit(lchip, p_learning_limit->gport, p_learning_limit->limit_action));
    }
    else if (CTC_SECURITY_LEARN_LIMIT_TYPE_VLAN == p_learning_limit->limit_type )
    {
        CTC_VLAN_RANGE_CHECK(p_learning_limit->vlan);

        CTC_ERROR_RETURN(_sys_usw_mac_security_set_vlan_mac_limit(lchip, p_learning_limit->vlan, p_learning_limit->limit_action));
    }
    else if (CTC_SECURITY_LEARN_LIMIT_TYPE_SYSTEM == p_learning_limit->limit_type )
    {
        CTC_ERROR_RETURN(_sys_usw_mac_security_set_system_mac_limit(lchip, p_learning_limit->limit_action));
    }
    else
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "[INTERRUPT] parameter is invalid \n");
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_mac_security_set_fid_learn_limit_action(uint8 lchip, uint16 fid, ctc_maclimit_action_t action)
{
    uint32 cmd = 0;
    uint32 field_id = 0;
    uint32 index = 0;
    uint32 learn_en = 0;
    uint32 mac_sec_discard = 0;
    uint32 mac_sec_excep = 0;
    SYS_SECURITY_INIT_CHECK(lchip);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    switch (action)
    {
        case CTC_MACLIMIT_ACTION_NONE:      /*learning eanble, forwarding, not discard, learnt*/
            mac_sec_discard = FALSE;
            mac_sec_excep = FALSE;
            learn_en = TRUE;
            break;

        case CTC_MACLIMIT_ACTION_FWD:       /*learning disable, forwarding*/
            mac_sec_discard = FALSE;
            mac_sec_excep = FALSE;
            learn_en = FALSE;
            break;

        case CTC_MACLIMIT_ACTION_DISCARD:   /*learning enable, discard, not learnt*/
            mac_sec_discard = TRUE;
            mac_sec_excep = FALSE;
            learn_en = TRUE;
            break;

        case CTC_MACLIMIT_ACTION_TOCPU:     /*learning enable, discard and to cpu, not learnt*/
            mac_sec_discard = TRUE;
            mac_sec_excep = TRUE;
            learn_en = TRUE;
            break;

        default:
             SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
            return CTC_E_INVALID_PARAM;
    }

    index = (fid >> 2) & 0xFFF;
    field_id = DsVsi_array_0_macSecurityVsiDiscard_f + (fid & 0x3)*8 ;
    cmd = DRV_IOW(DsVsi_t, field_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_sec_discard));

    field_id = DsVsi_array_0_macSecurityVsiExceptionEn_f + (fid & 0x3)*8 ;
    cmd = DRV_IOW(DsVsi_t, field_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &mac_sec_excep));

    learn_en = learn_en?0:1;
    field_id = DsVsi_array_0_vsiLearningDisable_f + (fid & 0x3)*8 ;
    cmd = DRV_IOW(DsVsi_t, field_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &learn_en));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_security_get_learn_limit(uint8 lchip, ctc_security_learn_limit_t* p_learning_limit, uint32* p_running_num)
{
    SYS_SECURITY_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_learning_limit);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (CTC_SECURITY_LEARN_LIMIT_TYPE_PORT == p_learning_limit->limit_type)
    {
        CTC_ERROR_RETURN(_sys_usw_mac_security_get_port_learn_limit(lchip, p_learning_limit));
    }
    else if (CTC_SECURITY_LEARN_LIMIT_TYPE_VLAN == p_learning_limit->limit_type )
    {
        CTC_ERROR_RETURN(_sys_usw_mac_security_get_vlan_learn_limit(lchip, p_learning_limit));
    }
    else if (CTC_SECURITY_LEARN_LIMIT_TYPE_SYSTEM == p_learning_limit->limit_type )
    {
        CTC_ERROR_RETURN(_sys_usw_mac_security_get_system_learn_limit(lchip, p_learning_limit));
    }
    else
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "[INTERRUPT] parameter is invalid \n");
        return CTC_E_INVALID_PARAM;
    }

    if (p_learning_limit->limit_num == MCHIP_CAP(SYS_CAP_LEARN_LIMIT_MAX))
    {
        p_learning_limit->limit_num = 0xFFFFFFFF;
    }

    if (p_running_num)
    {
        *p_running_num = p_learning_limit->limit_count;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_security_set_port_mac_limit(uint8 lchip, uint32 gport, ctc_maclimit_action_t action)
{
    uint8 limit_mode = 0;

    SYS_SECURITY_INIT_CHECK(lchip);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (CTC_IS_LINKAGG_PORT(gport))
    {
        return CTC_E_NONE;
    }
    CTC_ERROR_RETURN(_sys_usw_mac_limit_get_mode(lchip, &limit_mode));

    switch (action)
    {
    case CTC_MACLIMIT_ACTION_NONE:          /*learning enable, forwarding, not discard, learnt*/
        /*Only sw mode have none action */
        if (!limit_mode)
        {
            SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
            return CTC_E_NOT_SUPPORT;

        }

        CTC_ERROR_RETURN(sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_LEARNING_EN, TRUE));
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_MAC_SECURITY_DISCARD, FALSE));
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_SECURITY_EXCP_EN, FALSE));
        break;

    case CTC_MACLIMIT_ACTION_FWD:           /*learning disable, forwarding*/
        /*Only sw mode need set learning enable/disable */
        if (limit_mode)
        {
            CTC_ERROR_RETURN(sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_LEARNING_EN, FALSE));
        }

        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_MAC_SECURITY_DISCARD, FALSE));
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_SECURITY_EXCP_EN, FALSE));
        break;

    case CTC_MACLIMIT_ACTION_DISCARD:       /*learning enable, discard, not learnt*/
        /*Only sw mode need set learning enable/disable */
        if (limit_mode)
        {
            CTC_ERROR_RETURN(sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_LEARNING_EN, TRUE));
        }
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_MAC_SECURITY_DISCARD, TRUE));
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_SECURITY_EXCP_EN, FALSE));
        break;

    case CTC_MACLIMIT_ACTION_TOCPU:         /*learning enable, discard and to cpu, not learnt*/
        /*Only sw mode need set learning enable/disable */
        if (limit_mode)
        {
        CTC_ERROR_RETURN(sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_LEARNING_EN, TRUE));
        }

        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_MAC_SECURITY_DISCARD, TRUE));
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_SECURITY_EXCP_EN, TRUE));
        break;

    default:
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
        return CTC_E_INVALID_PARAM;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_security_set_vlan_mac_limit(uint8 lchip, uint16 vlan_id, ctc_maclimit_action_t action)
{
    uint8 limit_mode = 0;

    SYS_SECURITY_INIT_CHECK(lchip);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_mac_limit_get_mode(lchip, &limit_mode));

    switch (action)
    {
    case CTC_MACLIMIT_ACTION_NONE:      /*learning eanble, forwarding, not discard, learnt*/
        /*Only sw mode have none action */
        if (!limit_mode)
        {
            SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
            return CTC_E_NOT_SUPPORT;

        }

        CTC_ERROR_RETURN(sys_usw_vlan_set_property(lchip, vlan_id, CTC_VLAN_PROP_LEARNING_EN, TRUE));
        CTC_ERROR_RETURN(sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_MAC_SECURITY_VLAN_DISCARD, FALSE));
        CTC_ERROR_RETURN(sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_MAC_SECURITY_VLAN_EXCEPTION, FALSE));
        break;

    case CTC_MACLIMIT_ACTION_FWD:       /*learning disable, forwarding*/
        /*Only sw mode need set learning enable/disable */
        if (limit_mode)
        {
        CTC_ERROR_RETURN(sys_usw_vlan_set_property(lchip, vlan_id, CTC_VLAN_PROP_LEARNING_EN, FALSE));
        }
        CTC_ERROR_RETURN(sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_MAC_SECURITY_VLAN_DISCARD, FALSE));
        CTC_ERROR_RETURN(sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_MAC_SECURITY_VLAN_EXCEPTION, FALSE));
        break;

    case CTC_MACLIMIT_ACTION_DISCARD:   /*learning enable, discard, not learnt*/
        /*Only sw mode need set learning enable/disable */
        if (limit_mode)
        {
        CTC_ERROR_RETURN(sys_usw_vlan_set_property(lchip, vlan_id, CTC_VLAN_PROP_LEARNING_EN, TRUE));
        }

        CTC_ERROR_RETURN(sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_MAC_SECURITY_VLAN_DISCARD, TRUE));
        CTC_ERROR_RETURN(sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_MAC_SECURITY_VLAN_EXCEPTION, FALSE));
        break;

    case CTC_MACLIMIT_ACTION_TOCPU:     /*learning enable, discard and to cpu, not learnt*/
        /*Only sw mode need set learning enable/disable */
        if (limit_mode)
        {
        CTC_ERROR_RETURN(sys_usw_vlan_set_property(lchip, vlan_id, CTC_VLAN_PROP_LEARNING_EN, TRUE));
        }

        CTC_ERROR_RETURN(sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_MAC_SECURITY_VLAN_DISCARD, TRUE));
        CTC_ERROR_RETURN(sys_usw_vlan_set_internal_property(lchip, vlan_id, SYS_VLAN_PROP_MAC_SECURITY_VLAN_EXCEPTION, TRUE));
        break;

    default:
         SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_mac_security_set_system_mac_limit(uint8 lchip, ctc_maclimit_action_t action)
{
    uint8 sec_mode = 0;

    SYS_SECURITY_INIT_CHECK(lchip);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_mac_limit_get_mode(lchip, &sec_mode));

    switch (action)
    {
    case CTC_MACLIMIT_ACTION_NONE:      /*learning eanble, forwarding, not discard, learnt*/
        /*Only sw mode have none action */
        if (!sec_mode)
        {
            SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
            return CTC_E_NOT_SUPPORT;

        }

        CTC_ERROR_RETURN(_sys_usw_set_syetem_security_learn_en(lchip, TRUE));
        CTC_ERROR_RETURN(_sys_usw_set_system_mac_security_discard_en(lchip, FALSE, sec_mode));
        CTC_ERROR_RETURN(_sys_usw_set_syetem_security_excep_en(lchip, FALSE));
        break;

    case CTC_MACLIMIT_ACTION_FWD:       /*learning disable, forwarding*/
        if (sec_mode)
        {
            CTC_ERROR_RETURN(_sys_usw_set_syetem_security_learn_en(lchip, FALSE));
        }

        CTC_ERROR_RETURN(_sys_usw_set_system_mac_security_discard_en(lchip, FALSE, sec_mode));
        CTC_ERROR_RETURN(_sys_usw_set_syetem_security_excep_en(lchip, FALSE));
        break;

    case CTC_MACLIMIT_ACTION_DISCARD:   /*learning enable, discard, not learnt*/
        if (sec_mode)
        {
            CTC_ERROR_RETURN(_sys_usw_set_syetem_security_learn_en(lchip, TRUE));
        }

        CTC_ERROR_RETURN(_sys_usw_set_system_mac_security_discard_en(lchip, TRUE, sec_mode));
        CTC_ERROR_RETURN(_sys_usw_set_syetem_security_excep_en(lchip, FALSE));
        break;

    case CTC_MACLIMIT_ACTION_TOCPU:     /*learning enable, discard and to cpu, not learnt*/
        if (sec_mode)
        {
            CTC_ERROR_RETURN(_sys_usw_set_syetem_security_learn_en(lchip, TRUE));
        }

        CTC_ERROR_RETURN(_sys_usw_set_system_mac_security_discard_en(lchip, TRUE, sec_mode));
        CTC_ERROR_RETURN(_sys_usw_set_syetem_security_excep_en(lchip, TRUE));
        break;

    default:
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
        return CTC_E_INVALID_PARAM;

    }

    return CTC_E_NONE;

}

#define _________Learn_Limit_API_____________
/*mac limit mode: 0:Hw, 1:Sw*/
int32
sys_usw_mac_limit_set_mode(uint8 lchip, uint8 mode)
{
    uint32 cmd = 0;
    IpeLearningCtl_m ipe_learn_ctl;
    SYS_SECURITY_INIT_CHECK(lchip);

    sal_memset(&ipe_learn_ctl, 0, sizeof(IpeLearningCtl_m));
    SYS_SECURITY_LOCK(lchip);
    cmd = DRV_IOR(IpeLearningCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, 0, cmd, &ipe_learn_ctl));

    SetIpeLearningCtl(V, hwPortMacLimitEn_f, &ipe_learn_ctl, (mode)?0:1);
    SetIpeLearningCtl(V, hwVsiMacLimitEn_f, &ipe_learn_ctl, (mode)?0:1);

    cmd = DRV_IOW(IpeLearningCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, 0, cmd, &ipe_learn_ctl));
    SYS_SECURITY_UNLOCK(lchip);
    return CTC_E_NONE;
}

/*mac security*/
int32
sys_usw_mac_security_set_port_security(uint8 lchip, uint32 gport, bool enable)
{
    uint32 value = enable;

    SYS_SECURITY_INIT_CHECK(lchip);

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, gport, SYS_PORT_PROP_SECURITY_EN, value));

    return CTC_E_NONE;
}
int32
sys_usw_mac_security_get_port_security(uint8 lchip, uint32 gport, bool* p_enable)
{
    uint32 value = 0;

    SYS_SECURITY_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_enable);

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_SECURITY_EN, &value));
    *p_enable = (value) ? 1 : 0;

    return CTC_E_NONE;
}

int32
sys_usw_mac_security_set_port_mac_limit(uint8 lchip, uint32 gport, ctc_maclimit_action_t action)
{
    SYS_SECURITY_INIT_CHECK(lchip);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SECURITY_LOCK(lchip);
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_mac_security_set_port_mac_limit(lchip, gport, action));
    SYS_SECURITY_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_mac_security_get_port_mac_limit(uint8 lchip, uint32 gport, ctc_maclimit_action_t* action)
{
    uint32 port_learn_en;
    uint32 mac_security_discard_en;
    uint32 security_excp_en;
    int32 ret = CTC_E_NONE;
    uint8 limit_mode = 0;

    SYS_SECURITY_INIT_CHECK(lchip);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(action);
    SYS_SECURITY_LOCK(lchip);
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_mac_limit_get_mode(lchip, &limit_mode));
    SYS_SECURITY_UNLOCK(lchip);
    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_LEARNING_EN, &port_learn_en));
    CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_MAC_SECURITY_DISCARD, &mac_security_discard_en));
    CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, gport, SYS_PORT_PROP_SECURITY_EXCP_EN, &security_excp_en));

    if ((port_learn_en == TRUE) && mac_security_discard_en == FALSE && security_excp_en == FALSE)
    {
        *action = (limit_mode)?CTC_MACLIMIT_ACTION_NONE:CTC_MACLIMIT_ACTION_FWD;
    }
    else if ((port_learn_en == FALSE || (!limit_mode)) && mac_security_discard_en == FALSE && security_excp_en == FALSE)
    {
        *action = CTC_MACLIMIT_ACTION_FWD;
    }
    else if ((port_learn_en == TRUE || (!limit_mode)) && mac_security_discard_en == TRUE && security_excp_en == FALSE)
    {
        *action = CTC_MACLIMIT_ACTION_DISCARD;
    }
    else if ((port_learn_en == TRUE || (!limit_mode)) && mac_security_discard_en == TRUE && security_excp_en == TRUE)
    {
        *action = CTC_MACLIMIT_ACTION_TOCPU;
    }
    else
    {
         SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
        return CTC_E_INVALID_PARAM;
    }

    return ret;
}

int32
sys_usw_mac_security_set_vlan_mac_limit(uint8 lchip, uint16 vlan_id, ctc_maclimit_action_t action)
{
    SYS_SECURITY_INIT_CHECK(lchip);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SECURITY_LOCK(lchip);
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_mac_security_set_vlan_mac_limit(lchip, vlan_id, action));
    SYS_SECURITY_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_mac_security_get_vlan_mac_limit(uint8 lchip, uint16 vlan_id, ctc_maclimit_action_t* action)
{
    uint32 learn_en;
    uint32 mac_security_discard_en;
    uint32 security_excp_en;
    int32 ret = CTC_E_NONE;
    uint8 limit_mode = 0;

    SYS_SECURITY_INIT_CHECK(lchip);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(action);

    SYS_SECURITY_LOCK(lchip);
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_mac_limit_get_mode(lchip, &limit_mode));
    SYS_SECURITY_UNLOCK(lchip);
    CTC_ERROR_RETURN(sys_usw_vlan_get_property(lchip, vlan_id, CTC_VLAN_PROP_LEARNING_EN, &learn_en));
    CTC_ERROR_RETURN(sys_usw_vlan_get_internal_property(lchip, vlan_id, SYS_VLAN_PROP_MAC_SECURITY_VLAN_DISCARD, &mac_security_discard_en));
    CTC_ERROR_RETURN(sys_usw_vlan_get_internal_property(lchip, vlan_id, SYS_VLAN_PROP_MAC_SECURITY_VLAN_EXCEPTION, &security_excp_en));

    if ((learn_en == TRUE )  && mac_security_discard_en == FALSE && security_excp_en == FALSE)
    {
        *action = (limit_mode)?CTC_MACLIMIT_ACTION_NONE:CTC_MACLIMIT_ACTION_FWD;
    }
    else if ((learn_en == FALSE|| !limit_mode) && mac_security_discard_en == FALSE && security_excp_en == FALSE)
    {
        *action = CTC_MACLIMIT_ACTION_FWD;
    }
    else if (learn_en == TRUE && mac_security_discard_en == TRUE && security_excp_en == FALSE)
    {
        *action = CTC_MACLIMIT_ACTION_DISCARD;
    }
    else if ((learn_en == TRUE|| !limit_mode) && mac_security_discard_en == TRUE && security_excp_en == TRUE)
    {
        *action = CTC_MACLIMIT_ACTION_TOCPU;
    }
    else
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
        return CTC_E_INVALID_PARAM;
    }

    return ret;
}

int32
sys_usw_mac_security_set_system_mac_limit(uint8 lchip, ctc_maclimit_action_t action)
{
    SYS_SECURITY_INIT_CHECK(lchip);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SECURITY_LOCK(lchip);
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_mac_security_set_system_mac_limit(lchip, action));
    SYS_SECURITY_UNLOCK(lchip);
    return CTC_E_NONE;

}

int32
sys_usw_mac_security_get_system_mac_limit(uint8 lchip, ctc_maclimit_action_t* action)
{
    uint32 learn_en;
    uint32 mac_security_discard_en;
    uint32 security_excp_en;
    int32 ret = CTC_E_NONE;
    uint8 limit_mode = 0;

    SYS_SECURITY_INIT_CHECK(lchip);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(action);
    SYS_SECURITY_LOCK(lchip);
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_get_system_security_learn_en(lchip, &learn_en));
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_get_system_mac_security_discard_en(lchip, &mac_security_discard_en));
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_get_system_security_excep_en(lchip, &security_excp_en));
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_mac_limit_get_mode(lchip, &limit_mode));
    SYS_SECURITY_UNLOCK(lchip);
    if (learn_en == TRUE && mac_security_discard_en == FALSE && security_excp_en == FALSE)
    {
        *action = (limit_mode)?CTC_MACLIMIT_ACTION_NONE:CTC_MACLIMIT_ACTION_FWD;
    }
    else if (learn_en == FALSE && mac_security_discard_en == FALSE && security_excp_en == FALSE)
    {
        *action = CTC_MACLIMIT_ACTION_FWD;
    }
    else if (learn_en == TRUE && mac_security_discard_en == TRUE && security_excp_en == FALSE)
    {
        *action = CTC_MACLIMIT_ACTION_DISCARD;
    }
    else if (learn_en == TRUE && mac_security_discard_en == TRUE && security_excp_en == TRUE)
    {
        *action = CTC_MACLIMIT_ACTION_TOCPU;
    }
    else
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
        return CTC_E_INVALID_PARAM;
    }

    return ret;
}

int32
sys_usw_mac_security_set_learn_limit(uint8 lchip, ctc_security_learn_limit_t* p_learning_limit)
{
    SYS_SECURITY_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_learning_limit);
    SYS_LEARN_LIMIT_NUM_CHECK(p_learning_limit->limit_num);

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SECURITY_LOCK(lchip);
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_mac_security_set_learn_limit_action(lchip, p_learning_limit));
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_mac_security_set_learn_limit_num(lchip, p_learning_limit));
    SYS_SECURITY_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_mac_security_get_learn_limit(uint8 lchip, ctc_security_learn_limit_t* p_learning_limit, uint32* p_running_num)
{
    SYS_SECURITY_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_learning_limit);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SECURITY_LOCK(lchip);
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_mac_security_get_learn_limit(lchip, p_learning_limit, p_running_num));
    SYS_SECURITY_UNLOCK(lchip);
    return CTC_E_NONE;
}
#define __________IPSG_____________
STATIC int32
_sys_usw_ip_source_guard_lookup_entry_id(uint8 lchip, uint32 gport, uint8 scl_id, uint32* entry_id)
{
    uint8 loop = 0;
    uint8 key_type[3] = {0};
    int32 ret = 0;
    sys_scl_lkup_key_t lkup_key;
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;
    ctc_field_port_t mask_port;

    sal_memset(&field_key, 0, sizeof(field_key));
    sal_memset(&field_port, 0, sizeof(field_port));
    sal_memset(&mask_port, 0, sizeof(mask_port));

    key_type[0] = SYS_SCL_KEY_HASH_PORT_MAC;
    key_type[1] = SYS_SCL_KEY_HASH_PORT_IPV4;
    key_type[2] = SYS_SCL_KEY_HASH_PORT_IPV6;


    for(loop=0; loop < 3; loop++)
    {
        sal_memset(&lkup_key, 0, sizeof(lkup_key));
        lkup_key.group_priority = scl_id;
        lkup_key.group_id = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_SECURITY,
                                     (SYS_SECURITY_IPSG_GROUP_SUB_TYPE_DEFAULT_TCAM0 + scl_id), CTC_FIELD_PORT_TYPE_GPORT, 0, 0);
        lkup_key.resolve_conflict = 1;
        lkup_key.key_type = key_type[loop];
        field_key.type = SYS_SCL_FIELD_KEY_COMMON;
        CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));

        field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
        field_port.gport = gport;
        mask_port.gport = 0xffff;
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &field_port;
        field_key.ext_mask = &mask_port;
        CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key));

        ret = sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key);
        if(ret)
        {
            return (ret == CTC_E_NOT_EXIST)? CTC_E_NONE : ret;
        }
                    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lookuped entry id is %0x\n", lkup_key.entry_id);
        entry_id[loop] = lkup_key.entry_id;
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ip_source_guard_remove_default_entry(uint8 lchip, uint32 gport, uint8 scl_id)
{
    uint8 loop_type = 0;
    uint32 entry_id_tmp[3]= {0};

    CTC_ERROR_RETURN(_sys_usw_ip_source_guard_lookup_entry_id(lchip, gport, scl_id, entry_id_tmp));
    for(loop_type=0; loop_type < 3; loop_type++)
    {
        if(entry_id_tmp[loop_type] == 0)
        {
            continue;
        }
        sys_usw_scl_uninstall_entry(lchip, entry_id_tmp[loop_type], 1);
        sys_usw_scl_remove_entry(lchip, entry_id_tmp[loop_type], 1);
    }

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_ip_source_guard_add_default_entry(uint8 lchip, uint32 gport, uint8 scl_id)
{
    ctc_scl_entry_t def_entry;
    int32 ret = CTC_E_NONE;
    uint32 gid = 0;
    uint8 loop = 0;
    uint8 key_type[3] = {0};
    uint32 entry_id[3] = {0};
    ctc_scl_field_action_t field_action;
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;
    ctc_field_port_t mask_port;

    sal_memset(&def_entry, 0, sizeof(def_entry));
    sal_memset(&field_action, 0, sizeof(field_action));
    sal_memset(&field_key, 0, sizeof(field_key));
    sal_memset(&field_port, 0, sizeof(field_port));
    sal_memset(&mask_port, 0, sizeof(mask_port));

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SECURITY_INIT_CHECK(lchip);

    if (DRV_IS_DUET2(lchip))
    {
        return CTC_E_NONE;
    }

    /*lookup default entry whether exist, only need to check one entry*/
    CTC_ERROR_RETURN(_sys_usw_ip_source_guard_lookup_entry_id(lchip, gport, scl_id, entry_id));

    /* default entry in tcam0-3. */
    def_entry.priority_valid = 1;
    def_entry.priority       = 0;
    def_entry.action_type = SYS_SCL_ACTION_INGRESS;

    key_type[0] = SYS_SCL_KEY_HASH_PORT_MAC;
    key_type[1] = SYS_SCL_KEY_HASH_PORT_IPV4;
    key_type[2] = SYS_SCL_KEY_HASH_PORT_IPV6;

    gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_SECURITY,
                                 (SYS_SECURITY_IPSG_GROUP_SUB_TYPE_DEFAULT_TCAM0 + scl_id), CTC_FIELD_PORT_TYPE_GPORT, 0, 0);
    for(loop=0; loop < 3;loop++)
    {
        if(entry_id[loop] != 0)
        {
            continue;
        }

        def_entry.resolve_conflict = 1;
        def_entry.key_type = key_type[loop];

        field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
        field_port.gport = gport;
        mask_port.gport = 0xffff;
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &field_port;
        field_key.ext_mask = &mask_port;
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;

        CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid, &def_entry, 1), ret, error_0);
        CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, def_entry.entry_id, &field_key), ret, error_0);
        CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, def_entry.entry_id, &field_action), ret, error_0);
        CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, def_entry.entry_id, 1), ret, error_0);
    }

    return ret;

error_0:
    _sys_usw_ip_source_guard_remove_default_entry(lchip, gport, scl_id);

    return ret;
}

STATIC int32
_sys_usw_ip_source_guard_init_default_entry_group(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    uint32 gid[2] = {0};
    uint8 index = 0;
    uint8 loop = 0;
    ctc_scl_group_info_t group_info;
    ctc_scl_entry_t def_entry;
    ctc_field_key_t field_key;
    ctc_field_port_t field_port;
    ctc_field_port_t mask_port;
    ctc_scl_field_action_t field_action;
    uint8 key_type[3] = {SYS_SCL_KEY_TCAM_MAC, SYS_SCL_KEY_TCAM_IPV4_SINGLE, SYS_SCL_KEY_TCAM_IPV6};
    uint8 flag[3] = {SYS_SECURITY_IPSG_MAC_SUPPORT, SYS_SECURITY_IPSG_IPV4_SUPPORT, SYS_SECURITY_IPSG_IPV6_SUPPORT};
    uint32 array_table[6] = {DsScl0MacKey160_t, DsScl1MacKey160_t, DsScl0L3Key160_t, DsScl1L3Key160_t, DsScl0Ipv6Key320_t, DsScl1Ipv6Key320_t};
    uint32 table_size = 0;

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SECURITY_INIT_CHECK(lchip);

    sal_memset(&group_info, 0, sizeof(group_info));
    sal_memset(&def_entry, 0, sizeof(def_entry));
    sal_memset(&field_key, 0, sizeof(field_key));
    sal_memset(&field_port, 0, sizeof(field_port));
    sal_memset(&mask_port, 0, sizeof(mask_port));
    sal_memset(&field_action, 0, sizeof(field_action));
    def_entry.mode = 1;
    def_entry.action_type = CTC_SCL_ACTION_INGRESS;
    field_port.type = CTC_FIELD_PORT_TYPE_PORT_CLASS;
    field_port.port_class_id = MCHIP_CAP(SYS_CAP_SCL_LABEL_FOR_IPSG);
    mask_port.port_class_id = 0xFFFF;
    field_key.type = CTC_FIELD_KEY_PORT;
    field_key.ext_data = &field_port;
    field_key.ext_mask = &mask_port;
    field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
    for (index = 0; index < 2; index++)
    {
        group_info.priority = index;
        gid[index] = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_SECURITY,
                                     (SYS_SECURITY_IPSG_GROUP_SUB_TYPE_DEFAULT_TCAM0 + index), CTC_FIELD_PORT_TYPE_GPORT, 0, 0);
        ret = sys_usw_scl_create_group(lchip, gid[index], &group_info, 1);
        if ((ret < 0 ) && (ret != CTC_E_EXIST))
        {
            goto error_0;
        }
        for(loop=0; loop < 3; loop++)
        {
            CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, array_table[loop*2+index], &table_size), ret, error_0);
            if (table_size == 0)
            {
                continue;
            }
            CTC_SET_FLAG(usw_security_master[lchip]->flag , flag[loop]);
        }
        for(loop=0; loop < 3; loop++)
        {
            def_entry.key_type = key_type[loop];
            def_entry.priority = 0;
            def_entry.priority_valid = 1;
            CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid[index], &def_entry, 1), ret, error_0);
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, def_entry.entry_id, &field_key), ret, error_0);
            CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, def_entry.entry_id, &field_action), ret, error_0);
        }
        CTC_ERROR_GOTO(sys_usw_scl_install_group(lchip, gid[index], &group_info, 1), ret, error_0);
    }
    return ret;

error_0:

    for (index = 0; index < 2; index++)
    {
        sys_usw_scl_uninstall_group(lchip, gid[index], 1);
        sys_usw_scl_remove_all_entry(lchip, gid[index], 1);
        sys_usw_scl_destroy_group(lchip, gid[index], 1);
    }

    return ret;
}
int32 _sys_usw_ip_source_guard_init_logic_port_default_entry(uint8 lchip, uint32 gid_tcam[2], ctc_security_ip_source_guard_elem_t* p_elem)
{
    int32 ret = CTC_E_NONE;
    ctc_scl_entry_t scl_entry;
    ctc_field_port_t field_port;
    ctc_field_key_t field_key;
    ctc_scl_field_action_t field_action;
    ctc_field_port_t mask_port;
    uint8 loop_num = DRV_IS_DUET2(lchip) ? 1 : 3;
    uint8 key_type[3] = {SYS_SCL_KEY_HASH_PORT_IPV4, SYS_SCL_KEY_HASH_PORT_IPV6, SYS_SCL_KEY_HASH_PORT_MAC};
    uint8 loop = 0;
    uint32 roll_back_entry_id[3] = {0};
    sal_memset(&mask_port, 0, sizeof(ctc_field_port_t));
    sal_memset(&scl_entry, 0, sizeof(scl_entry));
    sal_memset(&field_port, 0, sizeof(field_port));
    sal_memset(&field_key, 0, sizeof(field_key));

    for(loop=0; loop < loop_num; loop++)
    {
        /*add default entry*/
        scl_entry.priority_valid = 1;
        scl_entry.priority       = 0;
        scl_entry.resolve_conflict = DRV_IS_DUET2(lchip) ? 0: 1;
        scl_entry.key_type = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_MAC : key_type[loop];
        scl_entry.action_type = SYS_SCL_ACTION_INGRESS;

        CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid_tcam[p_elem->scl_id], &scl_entry,1), ret, error_1);

        roll_back_entry_id[loop] = scl_entry.entry_id;
        field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
        field_port.logic_port = p_elem->gport;

        mask_port.logic_port = 0xffff;
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_mask = &mask_port;
        field_key.ext_data = &field_port;
        CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_1);

        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
        CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, error_1);
        CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, error_1);
    }

    return CTC_E_NONE;
error_1:
    for(loop=0; loop < loop_num; loop++)
    {
        if(roll_back_entry_id[loop])
        {
            sys_usw_scl_uninstall_entry(lchip, roll_back_entry_id[loop], 1);
            sys_usw_scl_remove_entry(lchip, roll_back_entry_id[loop], 1);
        }
    }
    return ret;
}
#define __________IPSG_API_____________
/*ip source guard*/
int32
sys_usw_ip_source_guard_add_entry(uint8 lchip, ctc_security_ip_source_guard_elem_t* p_elem)
{
    /*init*/
    uint32 gid_tcam[2] = {0};
    uint32 gid_hash[2] = {0};
    uint8 mac_0[6] = {0};
    uint32 ipv4_0 = 0;
    uint32 ipv6_0[4] = {0};
    int32 ret = CTC_E_NONE;
    ctc_scl_entry_t scl_entry;
    ctc_field_port_t field_port;
    ctc_field_key_t field_key;
    ctc_scl_field_action_t field_action;
    ctc_scl_bind_t field_data;
    ctc_scl_group_info_t group_info;
    ctc_field_port_t mask_port;
    mac_addr_t mask_mac;
    ipv6_addr_t mask_ipv6;
    uint8 create_default_grp = 0;
    sal_memset(&mask_ipv6, 0xFF, sizeof(ipv6_addr_t));
    sal_memset(&mask_mac, 0xFF, sizeof(mac_addr_t));
    sal_memset(&mask_port, 0, sizeof(ctc_field_port_t));
    sal_memset(&scl_entry, 0, sizeof(scl_entry));
    sal_memset(&field_data, 0, sizeof(field_data));
    sal_memset(&field_port, 0, sizeof(field_port));
    sal_memset(&field_key, 0, sizeof(field_key));
    sal_memset(&field_action, 0, sizeof(field_action));
    sal_memset(&group_info, 0, sizeof(group_info));

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*check parameter*/
    SYS_SECURITY_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_elem);
    if(CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
    {
        CTC_LOGIC_PORT_RANGE_CHECK(p_elem->gport);
        if (!(p_elem->gport))
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    else
    {
        SYS_GLOBAL_PORT_CHECK(p_elem->gport);
    }
    if ((p_elem->ip_src_guard_type == CTC_SECURITY_BINDING_TYPE_IP_VLAN)
        ||(p_elem->ip_src_guard_type == CTC_SECURITY_BINDING_TYPE_IP_MAC_VLAN)
        ||(p_elem->ip_src_guard_type == CTC_SECURITY_BINDING_TYPE_MAC_VLAN))
    {
        CTC_VLAN_RANGE_CHECK(p_elem->vid);
    }
    /********This is Lock,Please add unlock before 'return'********/
    SYS_SECURITY_LOCK(lchip);
    /*set default entry*/
    if(0 == usw_security_master[lchip]->ip_src_guard_def)
    {
        if(DRV_IS_DUET2(lchip))
        {
            CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_ip_source_guard_add_default_entry(lchip, 0, 0));
            CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_ip_source_guard_add_default_entry(lchip, 0, 1));
        }
        usw_security_master[lchip]->ip_src_guard_def = 1;
    }

    /*show debug info*/
    if(CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "logic port:0x%04X\n", p_elem->gport);
    }
    else
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport:0x%04X\n", p_elem->gport);
    }
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ip source guard type:%d\n", p_elem->ip_src_guard_type);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ipv4 or ipv6:%s\n", (CTC_IP_VER_4 == p_elem->ipv4_or_ipv6) ? "ipv4" : "ipv6");

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ipv4sa:%d.%d.%d.%d\n", (p_elem->ipv4_sa >> 24) & 0xFF, (p_elem->ipv4_sa >> 16) & 0xFF, \
                         (p_elem->ipv4_sa >> 8) & 0xFF, p_elem->ipv4_sa & 0xFF);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ipv6sa:%x%x%x%x\n", p_elem->ipv6_sa[0], p_elem->ipv6_sa[1], \
                         p_elem->ipv6_sa[2], p_elem->ipv6_sa[3]);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "mac:%x:%x:%x:%x:%x:%x\n", p_elem->mac_sa[0], p_elem->mac_sa[1], p_elem->mac_sa[2], \
                         p_elem->mac_sa[3], p_elem->mac_sa[4], p_elem->mac_sa[5]);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "vlan:%d\n", p_elem->vid);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "tcam:%d\n", p_elem->scl_id);

    if (p_elem->scl_id >= 2)
    {
        SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
        SYS_SECURITY_UNLOCK(lchip);
        return CTC_E_INVALID_PARAM;
    }

    /*port+ipv6 with mac use hash*/
    if (p_elem->ip_src_guard_type == CTC_SECURITY_BINDING_TYPE_IPV6_MAC && !CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_FLEX))
    {
        if (!(CTC_FLAG_ISSET(usw_security_master[lchip]->flag, SYS_SECURITY_IPSG_IPV6_SUPPORT)))
        {
            SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
            SYS_SECURITY_UNLOCK(lchip);
            return CTC_E_NOT_SUPPORT;

        }
        /*create group*/
        gid_hash[0] = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_SECURITY, SYS_SECURITY_IPSG_GROUP_SUB_TYPE_HASH, CTC_FIELD_PORT_TYPE_GPORT, 0, 0);
        gid_hash[1] = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_SECURITY, SYS_SECURITY_IPSG_GROUP_SUB_TYPE_HASH1, CTC_FIELD_PORT_TYPE_GPORT, 0, 0);
        group_info.un.gport = p_elem->gport;
        group_info.priority = p_elem->scl_id;
        if (0 == usw_security_master[lchip]->hash_group[p_elem->scl_id])
        {
            create_default_grp = 1;
            CTC_ERROR_GOTO(sys_usw_scl_create_group(lchip, gid_hash[p_elem->scl_id], &group_info, 1), ret, error_0);
            usw_security_master[lchip]->hash_group[p_elem->scl_id] = 1;
        }
        /*add entry*/
        scl_entry.key_type = SYS_SCL_KEY_HASH_PORT_IPV6;
        scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
        CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid_hash[p_elem->scl_id], &scl_entry,1), ret, error_1);
        /*add key field*/
        if(CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
        {
            field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            field_port.logic_port = p_elem->gport;
        }
        else
        {
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            field_port.gport = p_elem->gport;
        }
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &field_port;
        CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
        field_key.type = CTC_FIELD_KEY_IPV6_SA;
        field_key.ext_data = p_elem->ipv6_sa;
        CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        field_key.data = 1;
        CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
        /*add action field*/
        field_action.type = CTC_SCL_FIELD_ACTION_TYPE_BINDING_EN;
        field_data.type = CTC_SCL_BIND_TYPE_MACSA;
        sal_memcpy(field_data.mac_sa , p_elem->mac_sa, sizeof(mac_addr_t));
        field_action.ext_data = &field_data;
        CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, error_2);
        CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, error_2);
        SYS_SECURITY_UNLOCK(lchip);
        return ret;
    }

    if (((p_elem->ip_src_guard_type == (CTC_SECURITY_BINDING_TYPE_IP)) \
        ||(p_elem->ip_src_guard_type == (CTC_SECURITY_BINDING_TYPE_IP_MAC)) \
        ||(p_elem->ip_src_guard_type == (CTC_SECURITY_BINDING_TYPE_IP_VLAN))) \
        && (!(CTC_FLAG_ISSET(usw_security_master[lchip]->flag, SYS_SECURITY_IPSG_IPV4_SUPPORT))))
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        SYS_SECURITY_UNLOCK(lchip);
        return CTC_E_NOT_SUPPORT;

    }

    if (((p_elem->ip_src_guard_type == (CTC_SECURITY_BINDING_TYPE_MAC)) \
        ||(p_elem->ip_src_guard_type == (CTC_SECURITY_BINDING_TYPE_IP_MAC_VLAN)) \
        ||(p_elem->ip_src_guard_type == (CTC_SECURITY_BINDING_TYPE_MAC_VLAN))) \
        && (!(CTC_FLAG_ISSET(usw_security_master[lchip]->flag, SYS_SECURITY_IPSG_MAC_SUPPORT))))
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        SYS_SECURITY_UNLOCK(lchip);
        return CTC_E_NOT_SUPPORT;

    }

    if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_FLEX))
    {
        gid_tcam[0] = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_SECURITY, SYS_SECURITY_IPSG_GROUP_SUB_TYPE_TCAM0, CTC_FIELD_PORT_TYPE_GPORT, 0, 0);
        gid_tcam[1] = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_SECURITY, SYS_SECURITY_IPSG_GROUP_SUB_TYPE_TCAM1, CTC_FIELD_PORT_TYPE_GPORT, 0, 0);
        /*using tcam*/
        /*create group*/
        scl_entry.resolve_conflict = DRV_IS_DUET2(lchip) ? 0:1;
        if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
        {
            if (0 == usw_security_master[lchip]->tcam_group[p_elem->scl_id])
            {
                create_default_grp = 1;
                group_info.priority = p_elem->scl_id;
                CTC_ERROR_GOTO(sys_usw_scl_create_group(lchip, gid_tcam[p_elem->scl_id], &group_info, 1), ret, error_0);
                usw_security_master[lchip]->tcam_group[p_elem->scl_id] = 1;
            }
        }
        else
        {
            if (0 == usw_security_master[lchip]->tcam_group[p_elem->scl_id])
            {
                create_default_grp = 1;
                group_info.priority = p_elem->scl_id;
                CTC_ERROR_GOTO(sys_usw_scl_create_group(lchip, gid_tcam[p_elem->scl_id], &group_info, 1), ret, error_0);
                usw_security_master[lchip]->tcam_group[p_elem->scl_id] = 1;
            }
        }
         /*per logic port default entry*//*system needs*/
        if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT) && (p_elem->gport != 0)
            && !sal_memcmp(&(p_elem->ipv4_sa), &ipv4_0, sizeof(ip_addr_t)) && !sal_memcmp(&(p_elem->ipv6_sa), &ipv6_0, sizeof(ipv6_addr_t))
            && !sal_memcmp(&(p_elem->mac_sa), &mac_0, sizeof(mac_addr_t)) && (p_elem->vid == 0))
        {
            ret = _sys_usw_ip_source_guard_init_logic_port_default_entry(lchip, gid_tcam, p_elem);
            SYS_SECURITY_UNLOCK(lchip);
            return ret;
        }

        switch (p_elem->ip_src_guard_type)
        {
        case CTC_SECURITY_BINDING_TYPE_IPV6_MAC:
            if (!(CTC_FLAG_ISSET(usw_security_master[lchip]->flag, SYS_SECURITY_IPSG_IPV6_SUPPORT)))
            {
                SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
                SYS_SECURITY_UNLOCK(lchip);
                return CTC_E_NOT_SUPPORT;

            }
            scl_entry.key_type = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_IPV6 : SYS_SCL_KEY_HASH_PORT_IPV6;
            scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
            CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid_tcam[p_elem->scl_id], &scl_entry,1), ret, error_1);
            if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
                mask_port.logic_port = 0xffff;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
                mask_port.gport = 0xffff;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_mask = &mask_port;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_IPV6_SA;
            field_key.ext_mask = &mask_ipv6;
            field_key.ext_data = &(p_elem->ipv6_sa);
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_BINDING_EN;
            field_data.type = CTC_SCL_BIND_TYPE_MACSA;
            sal_memcpy(field_data.mac_sa , p_elem->mac_sa, sizeof(mac_addr_t));
            field_action.ext_data = &field_data;
            CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, error_2);
            break;

        case CTC_SECURITY_BINDING_TYPE_IP:
            scl_entry.key_type = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_IPV4_SINGLE : SYS_SCL_KEY_HASH_PORT_IPV4;
            scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
            CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid_tcam[p_elem->scl_id], &scl_entry,1), ret, error_1);

            if(DRV_IS_DUET2(lchip))
            {
                field_key.type = CTC_FIELD_KEY_L3_TYPE;
                field_key.data = CTC_PARSER_L3_TYPE_IPV4;
                field_key.mask = 0xF;
                CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            }

            if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
                mask_port.logic_port = 0xffff;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
                mask_port.gport = 0xffff;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_mask = &mask_port;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_IP_SA;
            field_key.data = p_elem->ipv4_sa;
            field_key.mask = 0xFFFFFFFF;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            /* do nothing */
            break;

        case CTC_SECURITY_BINDING_TYPE_IP_MAC:
            scl_entry.key_type = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_IPV4_SINGLE : SYS_SCL_KEY_HASH_PORT_IPV4;
            scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
            CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid_tcam[p_elem->scl_id], &scl_entry,1), ret, error_1);

            if(DRV_IS_DUET2(lchip))
            {
                field_key.type = CTC_FIELD_KEY_L3_TYPE;
                field_key.data = CTC_PARSER_L3_TYPE_IPV4;
                field_key.mask = 0xF;
                CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            }

            if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
                mask_port.logic_port = 0xffff;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
                mask_port.gport = 0xffff;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_mask = &mask_port;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_IP_SA;
            field_key.data = p_elem->ipv4_sa;
            field_key.mask = 0xFFFFFFFF;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_BINDING_EN;
            field_data.type = CTC_SCL_BIND_TYPE_MACSA;
            sal_memcpy(field_data.mac_sa, p_elem->mac_sa, sizeof(mac_addr_t));
            field_action.ext_data = &field_data;
            CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, error_2);
            break;

        case CTC_SECURITY_BINDING_TYPE_IP_VLAN:
            scl_entry.key_type = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_IPV4_SINGLE : SYS_SCL_KEY_HASH_PORT_IPV4;
            scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
            CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid_tcam[p_elem->scl_id], &scl_entry,1), ret, error_1);

            if(DRV_IS_DUET2(lchip))
            {
                field_key.type = CTC_FIELD_KEY_L3_TYPE;
                field_key.data = CTC_PARSER_L3_TYPE_IPV4;
                field_key.mask = 0xF;
                CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            }

            if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
                mask_port.logic_port = 0xffff;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
                mask_port.gport = 0xffff;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_mask = &mask_port;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_IP_SA;
            field_key.data = p_elem->ipv4_sa;
            field_key.mask = 0xFFFFFFFF;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_BINDING_EN;
            field_data.type = CTC_SCL_BIND_TYPE_VLAN;
            field_data.vlan_id = p_elem->vid;
            field_action.ext_data = &field_data;
            CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, error_2);
            break;

        case CTC_SECURITY_BINDING_TYPE_IP_MAC_VLAN:
            scl_entry.key_type = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_MAC : SYS_SCL_KEY_HASH_PORT_MAC;
            scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
            CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid_tcam[p_elem->scl_id], &scl_entry,1), ret, error_1);
            if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
                mask_port.logic_port = 0xffff;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
                mask_port.gport = 0xffff;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_mask = &mask_port;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_MAC_SA;
            field_key.ext_data = &p_elem->mac_sa;
            field_key.ext_mask = &mask_mac;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_BINDING_EN;
            field_data.type = CTC_SCL_BIND_TYPE_IPV4SA_VLAN;
            field_data.vlan_id = p_elem->vid;
            field_data.ipv4_sa = p_elem->ipv4_sa;
            field_action.ext_data = &field_data;
            CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, error_2);
            break;

        case CTC_SECURITY_BINDING_TYPE_MAC:
            scl_entry.key_type = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_MAC : SYS_SCL_KEY_HASH_PORT_MAC;
            scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
            CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid_tcam[p_elem->scl_id], &scl_entry,1), ret, error_1);
            if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
                mask_port.logic_port = 0xffff;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
                mask_port.gport = 0xffff;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_mask = &mask_port;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_MAC_SA;
            field_key.ext_data = &p_elem->mac_sa;
            field_key.ext_mask = &mask_mac;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            /* do nothing */
            break;

        case CTC_SECURITY_BINDING_TYPE_MAC_VLAN:
            scl_entry.key_type = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_MAC : SYS_SCL_KEY_HASH_PORT_MAC;
            scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
            CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid_tcam[p_elem->scl_id], &scl_entry,1), ret, error_1);
            if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
                mask_port.logic_port = 0xffff;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
                mask_port.gport = 0xffff;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_mask = &mask_port;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_MAC_SA;
            field_key.ext_data = &p_elem->mac_sa;
            field_key.ext_mask = &mask_mac;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_BINDING_EN;
            field_data.type = CTC_SCL_BIND_TYPE_VLAN;
            field_data.vlan_id = p_elem->vid;
            field_action.ext_data = &field_data;
            CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, error_2);
            break;

        default:
            SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
            SYS_SECURITY_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
    }
    else /*using hash*/
    {
        /*create group*/
        gid_hash[0] = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_SECURITY, SYS_SECURITY_IPSG_GROUP_SUB_TYPE_HASH, CTC_FIELD_PORT_TYPE_GPORT, 0, 0);
        gid_hash[1] = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_SECURITY, SYS_SECURITY_IPSG_GROUP_SUB_TYPE_HASH1, CTC_FIELD_PORT_TYPE_GPORT, 0, 0);
        group_info.un.gport = p_elem->gport;
        group_info.priority = p_elem->scl_id;
        if (0 == usw_security_master[lchip]->hash_group[p_elem->scl_id])
        {
            create_default_grp = 1;
            CTC_ERROR_GOTO(sys_usw_scl_create_group(lchip, gid_hash[p_elem->scl_id], &group_info, 1), ret, error_0);
            usw_security_master[lchip]->hash_group[p_elem->scl_id] = 1;
        }
        switch (p_elem->ip_src_guard_type)
        {
        case CTC_SECURITY_BINDING_TYPE_IP:
            scl_entry.key_type = SYS_SCL_KEY_HASH_PORT_IPV4;
            scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
            CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid_hash[p_elem->scl_id], &scl_entry,1), ret, error_1);
            if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_IP_SA;
            field_key.data = p_elem->ipv4_sa;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_HASH_VALID;
            field_key.data = 1;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            /* do nothing */
            break;

        case CTC_SECURITY_BINDING_TYPE_IP_MAC:
            scl_entry.key_type = SYS_SCL_KEY_HASH_PORT_IPV4;
            scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
            CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid_hash[p_elem->scl_id], &scl_entry,1), ret, error_1);
            if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_IP_SA;
            field_key.data = p_elem->ipv4_sa;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_HASH_VALID;
            field_key.data = 1;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_BINDING_EN;
            field_data.type = CTC_SCL_BIND_TYPE_MACSA;
            sal_memcpy(field_data.mac_sa , p_elem->mac_sa, sizeof(mac_addr_t));
            field_action.ext_data = &field_data;
            CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, error_2);
            break;

        case CTC_SECURITY_BINDING_TYPE_IP_VLAN:
            scl_entry.key_type = SYS_SCL_KEY_HASH_PORT_IPV4;
            scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
            CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid_hash[p_elem->scl_id], &scl_entry,1), ret, error_1);
            if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_IP_SA;
            field_key.data = p_elem->ipv4_sa;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_HASH_VALID;
            field_key.data = 1;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_BINDING_EN;
            field_data.type = CTC_SCL_BIND_TYPE_VLAN;
            field_data.vlan_id = p_elem->vid;
            field_action.ext_data = &field_data;
            CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, error_2);
            break;

        case CTC_SECURITY_BINDING_TYPE_IP_MAC_VLAN:
            scl_entry.key_type = SYS_SCL_KEY_HASH_PORT_MAC;
            scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
            CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid_hash[p_elem->scl_id], &scl_entry,1), ret, error_1);
            if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_MAC_SA;
            field_key.ext_data = &(p_elem->mac_sa);
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_HASH_VALID;
            field_key.data = 1;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_BINDING_EN;
            field_data.type = CTC_SCL_BIND_TYPE_IPV4SA_VLAN;
            field_data.vlan_id = p_elem->vid;
            field_data.ipv4_sa = p_elem->ipv4_sa;
            field_action.ext_data = &field_data;
            CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, error_2);
            break;

        case CTC_SECURITY_BINDING_TYPE_MAC:
            scl_entry.key_type = SYS_SCL_KEY_HASH_PORT_MAC;
            scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
            CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid_hash[p_elem->scl_id], &scl_entry,1), ret, error_1);
            if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_MAC_SA;
            field_key.ext_data = &(p_elem->mac_sa);
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_HASH_VALID;
            field_key.data = 1;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            /* do nothing */
            break;

        case CTC_SECURITY_BINDING_TYPE_MAC_VLAN:
            scl_entry.key_type = SYS_SCL_KEY_HASH_PORT_MAC;
            scl_entry.action_type = SYS_SCL_ACTION_INGRESS;
            CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, gid_hash[p_elem->scl_id], &scl_entry,1), ret, error_1);
            if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_MAC_SA;
            field_key.ext_data = &(p_elem->mac_sa);
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_key.type = CTC_FIELD_KEY_HASH_VALID;
            field_key.data = 1;
            CTC_ERROR_GOTO(sys_usw_scl_add_key_field(lchip, scl_entry.entry_id, &field_key), ret, error_2);
            field_action.type = CTC_SCL_FIELD_ACTION_TYPE_BINDING_EN;
            field_data.type = CTC_SCL_BIND_TYPE_VLAN;
            field_data.vlan_id = p_elem->vid;
            field_action.ext_data = &field_data;
            CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, scl_entry.entry_id, &field_action), ret, error_2);
            break;

        default:
            SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
            SYS_SECURITY_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
    }
    CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, scl_entry.entry_id, 1), ret, error_2);
    SYS_SECURITY_UNLOCK(lchip);
    return ret;
    error_2:
        sys_usw_scl_remove_entry(lchip, scl_entry.entry_id, 1);
    error_1:
        if (create_default_grp)
        {
            if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_FLEX))
            {
                sys_usw_scl_destroy_group(lchip, gid_tcam[p_elem->scl_id], 1);
                usw_security_master[lchip]->tcam_group[p_elem->scl_id] = 0;
            }
            else
            {
                sys_usw_scl_destroy_group(lchip, gid_hash[p_elem->scl_id], 1);
                usw_security_master[lchip]->hash_group[p_elem->scl_id] = 0;
            }

        }
    error_0:
        SYS_SECURITY_UNLOCK(lchip);
        return ret;
}

int32
sys_usw_ip_source_guard_remove_entry(uint8 lchip, ctc_security_ip_source_guard_elem_t* p_elem)
{
    uint32 gid = 0;
    uint8 mac_0[6] = {0};
    uint32 ipv4_0 = 0;
    uint32 ipv6_0[4] = {0};
    ctc_scl_entry_t* p_scl_entry = NULL;
    sys_scl_lkup_key_t lkup_key;
    ctc_field_port_t field_port;
    ctc_field_port_t mask_port;
    ctc_field_key_t field_key;
    mac_addr_t mask_mac;
    ipv6_addr_t mask_ipv6;
    int32 ret = CTC_E_NONE;

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_SECURITY_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_elem);
    if(CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
    {
        CTC_LOGIC_PORT_RANGE_CHECK(p_elem->gport);
    }
    else
    {
        SYS_GLOBAL_PORT_CHECK(p_elem->gport);
    }
    sal_memset(&lkup_key, 0, sizeof(lkup_key));
    sal_memset(&field_port, 0, sizeof(field_port));
    sal_memset(&mask_port, 0, sizeof(mask_port));
    sal_memset(&field_key, 0, sizeof(field_key));
    sal_memset(&mask_mac, 0xFF, sizeof(mac_addr_t));
    sal_memset(&mask_ipv6, 0xFF, sizeof(ipv6_addr_t));
    sal_memset(&mask_port, 0, sizeof(ctc_field_port_t));
    p_scl_entry = (ctc_scl_entry_t*)mem_malloc(MEM_STMCTL_MODULE, sizeof(ctc_scl_entry_t));
    if (NULL == p_scl_entry)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_scl_entry, 0, sizeof(ctc_scl_entry_t));

    if(CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "logic port:0x%04X\n", p_elem->gport);
    }
    else
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gport:0x%04X\n", p_elem->gport);
    }
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ip source guard type:%d\n", p_elem->ip_src_guard_type);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ipv4 or ipv6:%d\n", p_elem->ipv4_or_ipv6);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ipv4sa:%d.%d.%d.%d\n", (p_elem->ipv4_sa >> 24) & 0xFF, (p_elem->ipv4_sa >> 16) & 0xFF, \
                         (p_elem->ipv4_sa >> 8) & 0xFF, p_elem->ipv4_sa & 0xFF);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ipv6sa:%x%x%x%x\n", p_elem->ipv6_sa[0], p_elem->ipv6_sa[1], \
                         p_elem->ipv6_sa[2], p_elem->ipv6_sa[3]);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "mac:%x %x %x %x %x %x\n", p_elem->mac_sa[0], p_elem->mac_sa[1], p_elem->mac_sa[2], \
                         p_elem->mac_sa[3], p_elem->mac_sa[4], p_elem->mac_sa[5]);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "vlan:%d\n", p_elem->vid);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "tcam:%d\n", p_elem->scl_id);
    /********This is Lock,Please add unlock before 'return'********/
    SYS_SECURITY_LOCK(lchip);
    if (p_elem->ip_src_guard_type == CTC_SECURITY_BINDING_TYPE_IPV6_MAC && !CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_FLEX))
    {
        lkup_key.key_type = SYS_SCL_KEY_HASH_PORT_IPV6;
        lkup_key.action_type = SYS_SCL_ACTION_INGRESS;

        if (0 == p_elem->scl_id)
        {
            gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_SECURITY, SYS_SECURITY_IPSG_GROUP_SUB_TYPE_HASH, CTC_FIELD_PORT_TYPE_GPORT, 0, 0);
            lkup_key.group_id = gid;
            lkup_key.group_priority = p_elem->scl_id;
        }
        else if (1 == p_elem->scl_id && (!DRV_IS_DUET2(lchip)))
        {
            gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_SECURITY, SYS_SECURITY_IPSG_GROUP_SUB_TYPE_HASH1, CTC_FIELD_PORT_TYPE_GPORT, 0, 0);
            lkup_key.group_id = gid;
            lkup_key.group_priority = p_elem->scl_id;
        }
        else
        {
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
            ret = CTC_E_INVALID_PARAM;
            goto out1;
        }

        if(CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
        {
            field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
            field_port.logic_port = p_elem->gport;
        }
        else
        {
            field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
            field_port.gport = p_elem->gport;
        }
        field_key.type = CTC_FIELD_KEY_PORT;
        field_key.ext_data = &field_port;
        CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
        field_key.type = CTC_FIELD_KEY_IPV6_SA;
        field_key.ext_data = &p_elem->ipv6_sa;
        CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        field_key.data = 1;
        CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
        field_key.type = SYS_SCL_FIELD_KEY_COMMON;
        CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
        CTC_ERROR_GOTO(sys_usw_scl_get_entry_id_by_lkup_key(lchip,&lkup_key), ret, out1);
        p_scl_entry->entry_id = lkup_key.entry_id;
        CTC_ERROR_GOTO(sys_usw_scl_uninstall_entry(lchip, p_scl_entry->entry_id, 1), ret, out1);
        CTC_ERROR_GOTO(sys_usw_scl_remove_entry(lchip, p_scl_entry->entry_id, 1), ret, out1);
        goto out1;
    }
    if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_FLEX))
    {
        lkup_key.resolve_conflict = DRV_IS_DUET2(lchip) ? 0 :1;
        if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
        {
            gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_SECURITY, (p_elem->scl_id ? SYS_SECURITY_IPSG_GROUP_SUB_TYPE_TCAM1 : SYS_SECURITY_IPSG_GROUP_SUB_TYPE_TCAM0) , CTC_FIELD_PORT_TYPE_GPORT, 0, 0);
            lkup_key.group_id = gid;
            lkup_key.group_priority = p_elem->scl_id;
        }
        else
        {
            if (0 == p_elem->scl_id)
            {
                gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_SECURITY, SYS_SECURITY_IPSG_GROUP_SUB_TYPE_TCAM0, CTC_FIELD_PORT_TYPE_GPORT, 0, 0);
                lkup_key.group_id = gid;
                lkup_key.group_priority = 0;
            }
            else if (1 == p_elem->scl_id)
            {
                gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_SECURITY, SYS_SECURITY_IPSG_GROUP_SUB_TYPE_TCAM1, CTC_FIELD_PORT_TYPE_GPORT, 0, 0);
                lkup_key.group_id = gid;
                lkup_key.group_priority = 1;
            }
            else
            {
                SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
                ret = CTC_E_INVALID_PARAM;
                goto out1;
            }

        }

        /*per logic port default entry*/
        if (CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT) && (p_elem->gport != 0)
            && !sal_memcmp(&(p_elem->ipv4_sa), &ipv4_0, sizeof(ip_addr_t)) && !sal_memcmp(&(p_elem->ipv6_sa), &ipv6_0, sizeof(ipv6_addr_t))
            && !sal_memcmp(&(p_elem->mac_sa), &mac_0, sizeof(mac_addr_t)) && (p_elem->vid == 0))
        {
            uint8 loop_num = DRV_IS_DUET2(lchip) ? 1 : 3;
            uint8 key_type[3] = {SYS_SCL_KEY_HASH_PORT_IPV4, SYS_SCL_KEY_HASH_PORT_IPV6, SYS_SCL_KEY_HASH_PORT_MAC};
            uint8 loop = 0;

            for(loop=0; loop < loop_num; loop++)
            {
                sal_memset(&lkup_key.temp_entry, 0, sizeof(lkup_key.temp_entry));
                p_scl_entry->priority_valid = 1;
                p_scl_entry->priority       = 0;
                p_scl_entry->resolve_conflict = DRV_IS_DUET2(lchip) ? 0: 1;
                lkup_key.key_type = DRV_IS_DUET2(lchip) ? CTC_SCL_KEY_TCAM_MAC: key_type[loop];
                lkup_key.action_type = SYS_SCL_ACTION_INGRESS;

                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
                mask_port.logic_port = 0xffff;
                field_key.type = CTC_FIELD_KEY_PORT;
                field_key.ext_mask = &mask_port;
                field_key.ext_data = &field_port;
                CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
                field_key.type = SYS_SCL_FIELD_KEY_COMMON;
                CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
                CTC_ERROR_GOTO(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key), ret, out1);
                p_scl_entry->entry_id = lkup_key.entry_id;
                CTC_ERROR_GOTO(sys_usw_scl_uninstall_entry(lchip, p_scl_entry->entry_id, 1), ret, out1);
                CTC_ERROR_GOTO(sys_usw_scl_remove_entry(lchip, p_scl_entry->entry_id, 1), ret, out1);
            }
            ret = CTC_E_NONE;
            goto out1;
        }

        switch (p_elem->ip_src_guard_type)
        {
        case CTC_SECURITY_BINDING_TYPE_IPV6_MAC:
            lkup_key.key_type = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_IPV6 : SYS_SCL_KEY_HASH_PORT_IPV6;
            lkup_key.action_type = SYS_SCL_ACTION_INGRESS;
            if(CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
                mask_port.logic_port = 0xFFFF;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
                mask_port.gport = 0xFFFF;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_mask = &mask_port;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            field_key.type = CTC_FIELD_KEY_IPV6_SA;
            field_key.ext_mask = &mask_ipv6;
            field_key.ext_data = p_elem->ipv6_sa;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            field_key.type = SYS_SCL_FIELD_KEY_COMMON;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            CTC_ERROR_GOTO(sys_usw_scl_get_entry_id_by_lkup_key(lchip,&lkup_key), ret, out1);
            p_scl_entry->entry_id = lkup_key.entry_id;
            CTC_ERROR_GOTO(sys_usw_scl_uninstall_entry(lchip, p_scl_entry->entry_id, 1), ret, out1);
            CTC_ERROR_GOTO(sys_usw_scl_remove_entry(lchip, p_scl_entry->entry_id, 1), ret, out1);
            break;

        case CTC_SECURITY_BINDING_TYPE_IP:
        case CTC_SECURITY_BINDING_TYPE_IP_MAC:
        case CTC_SECURITY_BINDING_TYPE_IP_VLAN:
            lkup_key.key_type = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_IPV4_SINGLE: SYS_SCL_KEY_HASH_PORT_IPV4;
            lkup_key.action_type = SYS_SCL_ACTION_INGRESS;

            if(CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
                mask_port.logic_port = 0xFFFF;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
                mask_port.gport = 0xFFFF;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_mask = &mask_port;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);

            if(DRV_IS_DUET2(lchip))
            {
                field_key.type = CTC_FIELD_KEY_L3_TYPE;
                field_key.data = CTC_PARSER_L3_TYPE_IPV4;
                field_key.mask = 0xF;
                CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            }

            field_key.type = CTC_FIELD_KEY_IP_SA;
            field_key.data = p_elem->ipv4_sa;
            field_key.mask = 0xFFFFFFFF;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            field_key.type = SYS_SCL_FIELD_KEY_COMMON;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            CTC_ERROR_GOTO(sys_usw_scl_get_entry_id_by_lkup_key(lchip,&lkup_key), ret, out1);
            p_scl_entry->entry_id = lkup_key.entry_id;
            CTC_ERROR_GOTO(sys_usw_scl_uninstall_entry(lchip, p_scl_entry->entry_id, 1), ret, out1);
            CTC_ERROR_GOTO(sys_usw_scl_remove_entry(lchip, p_scl_entry->entry_id, 1), ret, out1);
            break;

        case CTC_SECURITY_BINDING_TYPE_IP_MAC_VLAN:
        case CTC_SECURITY_BINDING_TYPE_MAC:
        case CTC_SECURITY_BINDING_TYPE_MAC_VLAN:
            lkup_key.key_type = DRV_IS_DUET2(lchip) ?  SYS_SCL_KEY_TCAM_MAC : SYS_SCL_KEY_HASH_PORT_MAC;
            lkup_key.action_type = SYS_SCL_ACTION_INGRESS;
            if(CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
                mask_port.logic_port = 0xFFFF;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
                mask_port.gport = 0xFFFF;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_mask = &mask_port;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            field_key.type = CTC_FIELD_KEY_MAC_SA;
            field_key.ext_data = &p_elem->mac_sa;
            field_key.ext_mask = &mask_mac;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            field_key.type = SYS_SCL_FIELD_KEY_COMMON;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            CTC_ERROR_GOTO(sys_usw_scl_get_entry_id_by_lkup_key(lchip,&lkup_key), ret, out1);
            p_scl_entry->entry_id = lkup_key.entry_id;
            CTC_ERROR_GOTO(sys_usw_scl_uninstall_entry(lchip, p_scl_entry->entry_id, 1), ret, out1);
            CTC_ERROR_GOTO(sys_usw_scl_remove_entry(lchip, p_scl_entry->entry_id, 1), ret, out1);
            break;

        default:
            SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
            ret = CTC_E_INVALID_PARAM;
            goto out1;
        }
    }
    else
    {
        if (0 == p_elem->scl_id)
        {
            gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_SECURITY, SYS_SECURITY_IPSG_GROUP_SUB_TYPE_HASH, CTC_FIELD_PORT_TYPE_GPORT, 0, 0);
            lkup_key.group_id = gid;
            lkup_key.group_priority = p_elem->scl_id;
        }
        else if (1 == p_elem->scl_id && (!DRV_IS_DUET2(lchip)))
        {
            gid = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_SECURITY, SYS_SECURITY_IPSG_GROUP_SUB_TYPE_HASH1, CTC_FIELD_PORT_TYPE_GPORT, 0, 0);
            lkup_key.group_id = gid;
            lkup_key.group_priority = p_elem->scl_id;
        }
        else
        {
            SYS_PORT_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
            ret = CTC_E_INVALID_PARAM;
            goto out1;
        }

        switch (p_elem->ip_src_guard_type)
        {
        case CTC_SECURITY_BINDING_TYPE_IP:
        case CTC_SECURITY_BINDING_TYPE_IP_MAC:
        case CTC_SECURITY_BINDING_TYPE_IP_VLAN:
            lkup_key.key_type = SYS_SCL_KEY_HASH_PORT_IPV4;
            lkup_key.action_type = SYS_SCL_ACTION_INGRESS;
            if(CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            field_key.type = CTC_FIELD_KEY_IP_SA;
            field_key.data = p_elem->ipv4_sa;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            field_key.type = CTC_FIELD_KEY_HASH_VALID;
            field_key.data = 1;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            field_key.type = SYS_SCL_FIELD_KEY_COMMON;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            CTC_ERROR_GOTO(sys_usw_scl_get_entry_id_by_lkup_key(lchip,&lkup_key), ret, out1);
            p_scl_entry->entry_id = lkup_key.entry_id;
            CTC_ERROR_GOTO(sys_usw_scl_uninstall_entry(lchip, p_scl_entry->entry_id, 1), ret, out1);
            CTC_ERROR_GOTO(sys_usw_scl_remove_entry(lchip, p_scl_entry->entry_id, 1), ret, out1);
            break;

        case CTC_SECURITY_BINDING_TYPE_IP_MAC_VLAN:
        case CTC_SECURITY_BINDING_TYPE_MAC:
        case CTC_SECURITY_BINDING_TYPE_MAC_VLAN:
            lkup_key.key_type = SYS_SCL_KEY_HASH_PORT_MAC;
            lkup_key.action_type = SYS_SCL_ACTION_INGRESS;
            if(CTC_FLAG_ISSET(p_elem->flag, CTC_SECURITY_IP_SOURCE_GUARD_FLAG_USE_LOGIC_PORT))
            {
                field_port.type = CTC_FIELD_PORT_TYPE_LOGIC_PORT;
                field_port.logic_port = p_elem->gport;
            }
            else
            {
                field_port.type = CTC_FIELD_PORT_TYPE_GPORT;
                field_port.gport = p_elem->gport;
            }
            field_key.type = CTC_FIELD_KEY_PORT;
            field_key.ext_data = &field_port;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            field_key.type = CTC_FIELD_KEY_MAC_SA;
            field_key.ext_data = &p_elem->mac_sa;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            field_key.type = CTC_FIELD_KEY_HASH_VALID;
            field_key.data = 1;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            field_key.type = SYS_SCL_FIELD_KEY_COMMON;
            CTC_ERROR_GOTO(sys_usw_scl_construct_lkup_key(lchip, &field_key, &lkup_key), ret, out1);
            CTC_ERROR_GOTO(sys_usw_scl_get_entry_id_by_lkup_key(lchip,&lkup_key), ret, out1);
            p_scl_entry->entry_id = lkup_key.entry_id;
            CTC_ERROR_GOTO(sys_usw_scl_uninstall_entry(lchip, p_scl_entry->entry_id, 1), ret, out1);
            CTC_ERROR_GOTO(sys_usw_scl_remove_entry(lchip, p_scl_entry->entry_id, 1), ret, out1);
            break;

        default:
            SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Invalid parameter \n");
            ret = CTC_E_INVALID_PARAM;
            goto out1;
        }
    }

out1:
    SYS_SECURITY_UNLOCK(lchip);

    if (p_scl_entry)
    {
        mem_free(p_scl_entry);
    }

    return ret;
}
int32
sys_usw_ip_source_guard_remove_default_entry(uint8 lchip, uint32 gport, uint8 scl_id)
{
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_SECURITY_INIT_CHECK(lchip);
    SYS_GLOBAL_PORT_CHECK(gport);

    SYS_SECURITY_LOCK(lchip);
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_ip_source_guard_remove_default_entry(lchip, gport, scl_id));

    SYS_SECURITY_UNLOCK(lchip);
    return CTC_E_NONE;
}
/*For D2, do not care gport & scl_id*/
int32
sys_usw_ip_source_guard_add_default_entry(uint8 lchip, uint32 gport, uint8 scl_id)
{
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_SECURITY_INIT_CHECK(lchip);
    SYS_GLOBAL_PORT_CHECK(gport);


    SYS_SECURITY_LOCK(lchip);
    if(DRV_IS_DUET2(lchip))
    {
        if(0 == usw_security_master[lchip]->ip_src_guard_def)
        {
            CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_ip_source_guard_add_default_entry(lchip, 0, 0));
            CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_ip_source_guard_add_default_entry(lchip, 0, 1));
            usw_security_master[lchip]->ip_src_guard_def = 1;
        }
    }
    else
    {
        CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_ip_source_guard_add_default_entry(lchip, gport, scl_id));
    }
    SYS_SECURITY_UNLOCK(lchip);
    return CTC_E_NONE;
}
#define __________Port_Isolation_API_____________
int32
sys_usw_port_isolation_set_route_obey_isolated_en(uint8 lchip, bool enable)
{
    uint32 cmd = 0;
    uint32 tmp = 0;
    uint8 mode = 0;
    uint32 value = 0;
    int32 index = 0;

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SECURITY_INIT_CHECK(lchip);

    sys_usw_port_get_isolation_mode(lchip, &mode);
    SYS_SECURITY_LOCK(lchip);
    if (mode)
    {
        tmp = enable ? 1 : 0;
        cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_routeObeyIsolate_f);
        CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, 0, cmd, &tmp));
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "enable:%d\n", enable);
    }
    else
    {
        if (enable)
        {
            value = 0;
            index = 2;          /*IPUC*/
            cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
            CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, index, cmd, &value));
            value = 1;
            index = 3;          /*IPMC*/
            cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
            CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, index, cmd, &value));
            value = 2;
            index = (1 << 4) + 2;          /*IPUC*/
            cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
            CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, index, cmd, &value));
            value = 3;
            index = (1 << 4) + 3;          /*IPMC*/
            cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
            CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, index, cmd, &value));
        }
        else
        {
            value = 5;
            index = 2;          /*IPUC*/
            cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
            CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, index, cmd, &value));
            index = (1 << 4) + 2;          /*IPUC*/
            cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
            CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, index, cmd, &value));
            index = 3;          /*IPMC*/
            cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
            CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, index, cmd, &value));
            index = (1 << 4) + 3;          /*IPMC*/
            cmd = DRV_IOW(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
            CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, index, cmd, &value));
        }
    }
    SYS_SECURITY_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_port_isolation_get_route_obey_isolated_en(uint8 lchip, bool* p_enable)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint8 mode = 0;
    int32 index = 0;

    SYS_SECURITY_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_enable);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sys_usw_port_get_isolation_mode(lchip, &mode);
    SYS_SECURITY_LOCK(lchip);
    if (mode)
    {
        cmd = DRV_IOR(EpeNextHopCtl_t, EpeNextHopCtl_routeObeyIsolate_f);
        CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, 0, cmd, &field_val));
        *p_enable = (field_val == 1) ? TRUE : FALSE;
    }
    else
    {
        index = 2;          /*IPUC*/
        cmd = DRV_IOR(DsPortIsolationGroupMapping_t, DsPortIsolationGroupMapping_portIsolateType_f);
        CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, index, cmd, &field_val));
        *p_enable = (5 != field_val) ? TRUE : FALSE;
    }
    SYS_SECURITY_UNLOCK(lchip);
    return CTC_E_NONE;
}

#define __________Storm_Control_____________
STATIC int32
_sys_usw_storm_ctl_get_profile(uint8 lchip, sys_stmctl_profile_t *profile)
{
    uint8  step = 0;
    uint32 cmd = 0;
    DsIpeStormCtl0ProfileX_m stmctl_profile;

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&stmctl_profile, 0, sizeof(DsIpeStormCtl0ProfileX_m));

    if (CTC_SECURITY_STORM_CTL_OP_VLAN == profile->op)
    {
        step = DsIpeStormCtl1Config_t - DsIpeStormCtl0Config_t;
    }
    else
    {
        step = 0;
    }

    if (profile->is_y)
    {
        cmd = DRV_IOR(DsIpeStormCtl0ProfileY_t + step, DRV_ENTRY_FLAG);
    }
    else
    {
        cmd = DRV_IOR(DsIpeStormCtl0ProfileX_t + step, DRV_ENTRY_FLAG);
    }

    CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile->profile_id, cmd, &stmctl_profile));

    profile->rate = GetDsIpeStormCtl0ProfileX(V, rate_f, &stmctl_profile);
    profile->rate_shift = GetDsIpeStormCtl0ProfileX(V, rateShift_f, &stmctl_profile);
    profile->rate_frac = GetDsIpeStormCtl0ProfileX(V, rateFrac_f, &stmctl_profile);
    profile->threshold = GetDsIpeStormCtl0ProfileX(V, threshold_f, &stmctl_profile);
    profile->threshold_shift = GetDsIpeStormCtl0ProfileX(V, thresholdShift_f, &stmctl_profile);

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_security_stmctl_build_index(sys_stmctl_profile_t* p_node, uint8* p_lchip)
{
    uint32 value_32 = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    if(!p_node->is_y)
    {
        opf.pool_index = 0;
    }
    else
    {
        opf.pool_index = 1;
    }
    if(CTC_SECURITY_STORM_CTL_OP_PORT == p_node->op)
    {
        opf.pool_type = usw_security_master[*p_lchip]->stmctl_profile0_opf_type;
    }
    else
    {
        opf.pool_type = usw_security_master[*p_lchip]->stmctl_profile1_opf_type;
    }
    if (CTC_WB_ENABLE && (CTC_WB_STATUS(*p_lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(*p_lchip, &opf, 1, p_node->profile_id));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(*p_lchip, &opf, 1, &value_32));
        p_node->profile_id= value_32;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_security_stmctl_free_index(sys_stmctl_profile_t* p_node, uint8* p_lchip)
{
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(p_node);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    if(!p_node->is_y)
    {
        opf.pool_index = 0;
    }
    else
    {
        opf.pool_index = 1;
    }
    if(CTC_SECURITY_STORM_CTL_OP_PORT == p_node->op)
    {
        opf.pool_type = usw_security_master[*p_lchip]->stmctl_profile0_opf_type;
    }
    else
    {
        opf.pool_type = usw_security_master[*p_lchip]->stmctl_profile1_opf_type;
    }

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(*p_lchip, &opf, 1, p_node->profile_id));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stmctl_get_pkt_type_stmctl_base(uint8 lchip, ctc_security_storm_ctl_type_t type, uint8* o_stmctl_base)
{
    uint8 stmctl_base = 0;
    int32 ret = CTC_E_NONE;

    if (CTC_SECURITY_STORM_CTL_BCAST == type)
    {
        stmctl_base = 0;
    }
    else if (CTC_SECURITY_STORM_CTL_UNKNOWN_UCAST == type)
    {
        stmctl_base = 1;
    }
    else if ((CTC_SECURITY_STORM_CTL_KNOWN_MCAST == type)
                && (usw_security_master[lchip]->stmctl_cfg.mstorm_ctl_mode))
    {
        stmctl_base = 2;
    }
    else if (  ((CTC_SECURITY_STORM_CTL_UNKNOWN_MCAST == type)
                && (usw_security_master[lchip]->stmctl_cfg.mstorm_ctl_mode))
            || ((CTC_SECURITY_STORM_CTL_ALL_MCAST == type)
                && (!usw_security_master[lchip]->stmctl_cfg.mstorm_ctl_mode))  )
    {
        stmctl_base = 3;
    }
    else
    {
        stmctl_base = 4;
        ret  = CTC_E_NOT_SUPPORT;
    }

    if(o_stmctl_base)
    {
        *o_stmctl_base = stmctl_base;
    }

    return ret;
}

STATIC int32
_sys_usw_stmctl_get_pkt_type_from_stmctl_base(uint8 lchip, ctc_security_storm_ctl_type_t* type, uint8 stmctl_base)
{
    int32 ret = CTC_E_NONE;

    if (stmctl_base == 0)
    {
        *type = CTC_SECURITY_STORM_CTL_BCAST;
    }
    else if (stmctl_base == 1)
    {
        *type = CTC_SECURITY_STORM_CTL_UNKNOWN_UCAST;
    }
    else if ((stmctl_base == 2)
                && (usw_security_master[lchip]->stmctl_cfg.mstorm_ctl_mode))
    {
        *type = CTC_SECURITY_STORM_CTL_KNOWN_MCAST;
    }
    else if (stmctl_base == 3 && (usw_security_master[lchip]->stmctl_cfg.mstorm_ctl_mode))
    {
        *type = CTC_SECURITY_STORM_CTL_UNKNOWN_MCAST;
    }
    else if (stmctl_base == 3 && (!usw_security_master[lchip]->stmctl_cfg.mstorm_ctl_mode))
    {
        *type = CTC_SECURITY_STORM_CTL_ALL_MCAST;
    }
    else
    {
        ret  = CTC_E_NOT_SUPPORT;
    }

    return ret;
}
STATIC int32
_sys_usw_stmctl_alloc_stmctl_offset(uint8 lchip,
                        ctc_security_stmctl_cfg_t* p_stmctl_cfg, uint32* p_offset,uint8 *first)
{
    uint32 stmctl_en = 0;
    uint32 offset = 0;
    int32  ret = CTC_E_NONE;
    sys_usw_opf_t opf;
    uint8 base = 0;

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_stmctl_get_pkt_type_stmctl_base(lchip, p_stmctl_cfg->type, &base));


    if (CTC_SECURITY_STORM_CTL_OP_PORT == p_stmctl_cfg->op)
    {
        CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, p_stmctl_cfg->gport, SYS_PORT_PROP_STMCTL_EN, &stmctl_en));

        if (stmctl_en)
        {
            CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, p_stmctl_cfg->gport, SYS_PORT_PROP_STMCTL_OFFSET, &offset));
            *first = 0;
        }
        else
        {/*alloc offset */
            sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
            opf.pool_type = usw_security_master[lchip]->storm_ctl_opf_type;
            opf.pool_index = 0;
            ret = sys_usw_opf_alloc_offset(lchip, &opf, 1, &offset);
            if (CTC_E_NONE != ret)
            {
                return ret;
            }
            *first = 1;
            usw_security_master[lchip]->port_id[offset] = p_stmctl_cfg->gport;
        }
        offset = (base & 0x3) | ((offset & 0xFF) << 2);
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_vlan_get_internal_property(lchip, p_stmctl_cfg->vlan_id, SYS_VLAN_PROP_VLAN_STORM_CTL_EN, &stmctl_en));

        if (stmctl_en)
        {
            CTC_ERROR_RETURN(sys_usw_vlan_get_internal_property(lchip, p_stmctl_cfg->vlan_id, SYS_VLAN_PROP_VLAN_STORM_CTL_PTR, &offset));
             *first = 0;
        }
        else
        {
            sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
            opf.pool_type = usw_security_master[lchip]->storm_ctl_opf_type;
            opf.pool_index = 1;
            ret = sys_usw_opf_alloc_offset(lchip, &opf, 1, &offset);
            if (CTC_E_NONE != ret)
            {
                return ret;
            }
            *first = 1;
            usw_security_master[lchip]->vlan_id[offset] = p_stmctl_cfg->vlan_id;
        }
        offset = (base & 0x3) | ((offset & 0xFF) << 2);
    }

    if(p_offset)
    {
        *p_offset = offset;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_stmctl_free_stmctl_offset(uint8 lchip, ctc_security_storm_ctl_op_t op,
                                                            uint32 offset)
{
    int32  ret = CTC_E_NONE;
    sys_usw_opf_t opf;

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    offset = offset >> 2;
    if (CTC_SECURITY_STORM_CTL_OP_PORT == op)
    {

        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type = usw_security_master[lchip]->storm_ctl_opf_type;
        opf.pool_index = 0;
        ret = sys_usw_opf_free_offset(lchip, &opf, 1, offset);
        if (CTC_E_NONE != ret)
        {
            return ret;
        }
        usw_security_master[lchip]->port_id[offset] = 0;
    }
    else
    {
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type = usw_security_master[lchip]->storm_ctl_opf_type;
        opf.pool_index = 1;
        ret = sys_usw_opf_free_offset(lchip, &opf, 1, offset);
        if (CTC_E_NONE != ret)
        {
            return ret;
        }
        usw_security_master[lchip]->vlan_id[offset] = 0;
    }

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_security_get_policer_gran_by_rate(uint8 lchip, uint32 rate, uint32* granularity)
{
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(granularity);

#define M 1000

    if (rate <= 2*M)/*2M */
    {
        *granularity = 1;
    }
    else if(rate <= 100*M)/*100M */
    {
        *granularity = 32;
    }
    else if(rate <= 1000*M)  /*1G */
    {
        *granularity = 64;
    }
    else if(rate <= 2000*M)/*2G */
    {
        *granularity = 128;
    }
    else if(rate <= 4000*M)/*4G */
    {
        *granularity = 256;
    }
    else if(rate <= 10000*M)/*10G */
    {
        *granularity = 512;
    }
    else if(rate <= 40000*M)/*40G */
    {
        *granularity = 1024;
    }
    else if(rate <= 100000*M)/*100G */
    {
        *granularity = 2048;
    }
    else
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Unexpect value/result \n");
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_storm_ctl_build_profile(uint8 lchip,
                        ctc_security_stmctl_cfg_t* p_stmctl_cfg, sys_stmctl_profile_t* p_profile)
{
    uint16 bit_width = 256;
    uint8  is_pps    = 0;
    uint32 user_rate = 0; /*pps or kbps*/
    uint32 hw_rate   = 0;
    uint32 hw_thrd   = 0;
    uint32 granularity = 0;

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if (CTC_SECURITY_STORM_CTL_MODE_KPPS == p_stmctl_cfg->mode)
    {
        user_rate = p_stmctl_cfg->threshold * 1000;
        CTC_MAX_VALUE_CHECK(user_rate, MCHIP_CAP(SYS_CAP_STMCTL_MAX_PPS));
    }
    else if (CTC_SECURITY_STORM_CTL_MODE_PPS == p_stmctl_cfg->mode)
    {
        user_rate = p_stmctl_cfg->threshold;
        CTC_MAX_VALUE_CHECK(user_rate, MCHIP_CAP(SYS_CAP_STMCTL_MAX_PPS));
    }
    else if (CTC_SECURITY_STORM_CTL_MODE_BPS == p_stmctl_cfg->mode)
    {
        user_rate = ((uint64)p_stmctl_cfg->threshold * 8) / 1000;
        CTC_MAX_VALUE_CHECK(user_rate, MCHIP_CAP(SYS_CAP_STMCTL_MAX_KBPS));
    }
    else if (CTC_SECURITY_STORM_CTL_MODE_KBPS == p_stmctl_cfg->mode)
    {
        user_rate = p_stmctl_cfg->threshold * 8;
        CTC_MAX_VALUE_CHECK(user_rate, MCHIP_CAP(SYS_CAP_STMCTL_MAX_KBPS));
    }

    if(CTC_SECURITY_STORM_CTL_MODE_PPS == p_stmctl_cfg->mode
        || CTC_SECURITY_STORM_CTL_MODE_KPPS == p_stmctl_cfg->mode)
    {
        is_pps = 1;
    }
    else
    {
        is_pps = 0;
        /*kbps*/
        CTC_ERROR_RETURN(_sys_usw_security_get_policer_gran_by_rate(lchip, user_rate, &granularity));
    }

    CTC_ERROR_RETURN(
        sys_usw_qos_policer_map_token_rate_user_to_hw(lchip,
                                                             is_pps,
                                                             user_rate,
                                                             &hw_rate,
                                                             bit_width,
                                                             granularity,
                                                             MCHIP_CAP(SYS_CAP_STMCTL_UPD_FREQ)));
    if(is_pps != 0)
    {
        user_rate = user_rate * MCHIP_CAP(SYS_CAP_QOS_POLICER_PPS_PACKET_BYTES);
    }
    else
    {
        user_rate = user_rate * 125;
    }

    if(user_rate >= (0xfff << 0xf))
    {
        user_rate = (0xfff << 0xf);
    }

    CTC_ERROR_RETURN(sys_usw_qos_map_token_thrd_user_to_hw(lchip,
                                                                   user_rate,
                                                                   &hw_thrd,
                                                                   4,
                                                                   (1 << 12) -1));

    p_profile->op         = p_stmctl_cfg->op;
    p_profile->rate       = hw_rate >> 12;
    p_profile->rate_shift = (hw_rate >> 8) & 0xF;
    p_profile->rate_frac  = hw_rate & 0xFF;
    p_profile->threshold  = (hw_thrd >> 4) & 0xFFF;
    p_profile->threshold_shift = hw_thrd & 0xF;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_storm_ctl_get_profile_from_hw(uint8 lchip, uint32 stmctl_offset,
                        ctc_security_stmctl_cfg_t* p_stmctl_cfg, sys_stmctl_profile_t* p_profile)
{
    uint32 cmd = 0;
    uint8  step = 0;
    DsIpeStormCtl0Config_m stmctl_cfg;
    DsIpeStormCtl0ProfileX_m stmctl_profile;

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&stmctl_cfg, 0, sizeof(DsIpeStormCtl0Config_m));
    sal_memset(&stmctl_profile, 0, sizeof(DsIpeStormCtl0ProfileX_m));

    if(CTC_SECURITY_STORM_CTL_OP_VLAN == p_stmctl_cfg->op)
    {
        step = DsIpeStormCtl1Config_t - DsIpeStormCtl0Config_t;
    }

    cmd = DRV_IOR(DsIpeStormCtl0Config_t + step, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, stmctl_offset >> 1, cmd, &stmctl_cfg));
    if(stmctl_offset & 0x1)
    {
        p_profile->profile_id = GetDsIpeStormCtl0Config(V, profIdY_f, &stmctl_cfg);
        if(p_profile->profile_id)
        {
            cmd = DRV_IOR(DsIpeStormCtl0ProfileY_t + step, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_profile->profile_id, cmd, &stmctl_profile));
        }
    }
    else
    {
        p_profile->profile_id = GetDsIpeStormCtl0Config(V, profIdX_f, &stmctl_cfg);
        if(p_profile->profile_id)
        {
            cmd = DRV_IOR(DsIpeStormCtl0ProfileX_t + step, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_profile->profile_id, cmd, &stmctl_profile));
        }
    }

    if(p_profile->profile_id)
    {
        p_profile->is_y       = stmctl_offset & 0x1;
        p_profile->op         = p_stmctl_cfg->op;
        p_profile->rate       = GetDsIpeStormCtl0ProfileX(V, rate_f, &stmctl_profile);
        p_profile->rate_shift = GetDsIpeStormCtl0ProfileX(V, rateShift_f, &stmctl_profile);
        p_profile->rate_frac  = GetDsIpeStormCtl0ProfileX(V, rateFrac_f, &stmctl_profile);
        p_profile->threshold  = GetDsIpeStormCtl0ProfileX(V, threshold_f, &stmctl_profile);
        p_profile->threshold_shift = GetDsIpeStormCtl0ProfileX(V, thresholdShift_f, &stmctl_profile);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_storm_ctl_add_hw(uint8 lchip, uint32 stmctl_offset,
                ctc_security_stmctl_cfg_t* p_stmctl_cfg, sys_stmctl_profile_t profile,uint8 is_add)
{
    uint8  is_pps = 0;
    uint8  step = 0;
    uint32 cmd = 0;
    DsIpeStormCtl0Config_m stmctl_cfg;
    DsIpeStormCtl0ProfileX_m stmctl_profile;

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&stmctl_cfg, 0, sizeof(DsIpeStormCtl0Config_m));
    sal_memset(&stmctl_profile, 0, sizeof(DsIpeStormCtl0ProfileX_m));

    if(CTC_SECURITY_STORM_CTL_MODE_PPS == p_stmctl_cfg->mode
        || CTC_SECURITY_STORM_CTL_MODE_KPPS == p_stmctl_cfg->mode)
    {
        is_pps = 1;
    }
    if(CTC_SECURITY_STORM_CTL_OP_VLAN == p_stmctl_cfg->op)
    {
        step = DsIpeStormCtl1Config_t - DsIpeStormCtl0Config_t;
    }
    if(is_add)
    {
       SetDsIpeStormCtl0ProfileX(V, ppsShift_f,  &stmctl_profile, 7);   /**/
       SetDsIpeStormCtl0ProfileX(V, rate_f,      &stmctl_profile, profile.rate);
       SetDsIpeStormCtl0ProfileX(V, rateShift_f, &stmctl_profile, profile.rate_shift);
       SetDsIpeStormCtl0ProfileX(V, rateFrac_f,  &stmctl_profile, profile.rate_frac);
       SetDsIpeStormCtl0ProfileX(V, threshold_f,  &stmctl_profile, profile.threshold);
       SetDsIpeStormCtl0ProfileX(V, thresholdShift_f,  &stmctl_profile, profile.threshold_shift);
       if(stmctl_offset & 0x1)
       {
           cmd = DRV_IOW(DsIpeStormCtl0ProfileY_t + step, DRV_ENTRY_FLAG);
       }
       else
       {
           cmd = DRV_IOW(DsIpeStormCtl0ProfileX_t + step, DRV_ENTRY_FLAG);
       }
       CTC_ERROR_RETURN(DRV_IOCTL(lchip, profile.profile_id, cmd, &stmctl_profile));
    }

    /*write hw -- config*/
    cmd = DRV_IOR(DsIpeStormCtl0Config_t + step, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, stmctl_offset >> 1, cmd, &stmctl_cfg));
    if(stmctl_offset & 0x1)
    {
        SetDsIpeStormCtl0Config(V, exceptionEnY_f, &stmctl_cfg,is_add? p_stmctl_cfg->discarded_to_cpu:0);
        SetDsIpeStormCtl0Config(V, ppsModeY_f,     &stmctl_cfg, is_add?is_pps:0);
        SetDsIpeStormCtl0Config(V, profIdY_f, &stmctl_cfg, is_add?profile.profile_id:0);
    }
    else
    {
        SetDsIpeStormCtl0Config(V, exceptionEnX_f, &stmctl_cfg, is_add? p_stmctl_cfg->discarded_to_cpu:0);
        SetDsIpeStormCtl0Config(V, ppsModeX_f,     &stmctl_cfg, is_add?is_pps:0);
        SetDsIpeStormCtl0Config(V, profIdX_f, &stmctl_cfg,is_add?profile.profile_id:0);
    }
    cmd = DRV_IOW(DsIpeStormCtl0Config_t + step, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, stmctl_offset >> 1, cmd, &stmctl_cfg));

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_storm_ctl_set_interface_en(uint8 lchip, ctc_security_stmctl_cfg_t* p_stmctl_cfg,uint32 offset,uint8 enable)
{
    uint32 stmctl_offset = enable ? (offset >> 2) :0;
    /*6. set stmctl_ptr/stmctl_en on port/vlan*/
    if(CTC_SECURITY_STORM_CTL_OP_PORT == p_stmctl_cfg->op)
    {
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, p_stmctl_cfg->gport,
                                                        SYS_PORT_PROP_STMCTL_OFFSET, stmctl_offset));
        CTC_ERROR_RETURN(sys_usw_port_set_internal_property(lchip, p_stmctl_cfg->gport,
                                                        SYS_PORT_PROP_STMCTL_EN, enable ?1:0));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_vlan_set_internal_property(lchip, p_stmctl_cfg->vlan_id,
                                            SYS_VLAN_PROP_VLAN_STORM_CTL_PTR, stmctl_offset));
        CTC_ERROR_RETURN(sys_usw_vlan_set_internal_property(lchip, p_stmctl_cfg->vlan_id,
                                            SYS_VLAN_PROP_VLAN_STORM_CTL_EN, enable ?1:0));
    }
     return CTC_E_NONE;
}

STATIC int32
_sys_usw_storm_ctl_get_interface_en(uint8 lchip, ctc_security_stmctl_cfg_t* p_stmctl_cfg,uint32 *offset,uint8 *enable)
{
     uint32 value = 0;
     uint32 port_stmctl_offset;
     uint8 base = 0;

    CTC_ERROR_RETURN(_sys_usw_stmctl_get_pkt_type_stmctl_base(lchip, p_stmctl_cfg->type, &base));

     if (CTC_SECURITY_STORM_CTL_OP_PORT == p_stmctl_cfg->op)
    {
        CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, p_stmctl_cfg->gport, SYS_PORT_PROP_STMCTL_EN, &value));
                CTC_ERROR_RETURN(sys_usw_port_get_internal_property(lchip, p_stmctl_cfg->gport, SYS_PORT_PROP_STMCTL_OFFSET, &port_stmctl_offset));

    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_vlan_get_internal_property(lchip, p_stmctl_cfg->vlan_id, SYS_VLAN_PROP_VLAN_STORM_CTL_EN, &value));
        CTC_ERROR_RETURN(sys_usw_vlan_get_internal_property(lchip, p_stmctl_cfg->vlan_id, SYS_VLAN_PROP_VLAN_STORM_CTL_PTR, &port_stmctl_offset));

    }
    *offset = ((port_stmctl_offset& 0xFF) << 2) | (base & 0x3);
    *enable =  value ? 1:0;
    return CTC_E_NONE;
}
STATIC int32
_sys_usw_storm_ctl_set_mode_enable(uint8 lchip, ctc_security_stmctl_cfg_t* p_stmctl_cfg)
{
    uint32 stmctl_offset = 0;
    int32  ret = CTC_E_NONE;
    sys_stmctl_profile_t  new_profile;
    sys_stmctl_profile_t  old_profile;
    sys_stmctl_profile_t* p_profile_get;
    DsIpeStormCtl0ProfileX_m stmctl_profile;
    uint8 first_alloc = 0;

    CTC_PTR_VALID_CHECK(p_stmctl_cfg);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&stmctl_profile, 0, sizeof(DsIpeStormCtl0ProfileX_m));
    sal_memset(&new_profile, 0, sizeof(sys_stmctl_profile_t));
    sal_memset(&old_profile, 0, sizeof(sys_stmctl_profile_t));


    /*1. get offset*/
    CTC_ERROR_RETURN(_sys_usw_stmctl_alloc_stmctl_offset(lchip,  p_stmctl_cfg, &stmctl_offset,&first_alloc));

    /*2. get old profile if old exist*/
    CTC_ERROR_GOTO(_sys_usw_storm_ctl_get_profile_from_hw(lchip, stmctl_offset, p_stmctl_cfg, &old_profile), ret, ERROR1);

    /*3. set new profile*/
    CTC_ERROR_GOTO(_sys_usw_storm_ctl_build_profile(lchip, p_stmctl_cfg, &new_profile), ret, ERROR1);
    new_profile.is_y = stmctl_offset & 0x1;

    /*4. update profile*/
    if(old_profile.profile_id == 0)
    {
        CTC_ERROR_GOTO(ctc_spool_add(usw_security_master[lchip]->stmctl_spool, &new_profile, NULL, &p_profile_get), ret, ERROR1);
    }
    else
    {
        CTC_ERROR_GOTO(ctc_spool_add(usw_security_master[lchip]->stmctl_spool, &new_profile, &old_profile, &p_profile_get), ret, ERROR1);
    }

    /*5. write hw -- profile*/
    CTC_ERROR_GOTO(_sys_usw_storm_ctl_add_hw(lchip, stmctl_offset, p_stmctl_cfg, *p_profile_get, 1), ret, ERROR2);

    /*6. set stmctl_ptr/stmctl_en on port/vlan*/
    CTC_ERROR_GOTO( _sys_usw_storm_ctl_set_interface_en(lchip,p_stmctl_cfg,stmctl_offset, 1), ret, ERROR2);

    return CTC_E_NONE;


ERROR2:
    ctc_spool_remove(usw_security_master[lchip]->stmctl_spool, &p_profile_get, NULL);

ERROR1:
    if(first_alloc)
    {
         _sys_usw_storm_ctl_set_interface_en(lchip,p_stmctl_cfg,stmctl_offset, 0);
         _sys_usw_stmctl_free_stmctl_offset(lchip, p_stmctl_cfg->op, stmctl_offset);
    }

    return ret;
}

STATIC int32
_sys_usw_storm_ctl_set_mode_disable(uint8 lchip, ctc_security_stmctl_cfg_t* p_stmctl_cfg)
{
    uint8  step          = 0;
    uint8  stmctl_type   = 0;
    uint8 stmctl_en     = 0;
    uint32 cmd           = 0;
    uint8  base          = 0;
    uint32 stmctl_offset = 0;
    uint32 tmp_offset = 0;
    uint8   exist = 0;
    sys_stmctl_profile_t  profile;
    DsIpeStormCtl0Config_m   stmctl_cfg;
    DsIpeStormCtl0ProfileX_m stmctl_profile;

    CTC_PTR_VALID_CHECK(p_stmctl_cfg);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&profile,        0, sizeof(sys_stmctl_profile_t));
    sal_memset(&stmctl_cfg,     0, sizeof(DsIpeStormCtl0Config_m));
    sal_memset(&stmctl_profile, 0, sizeof(DsIpeStormCtl0ProfileX_m));

    CTC_ERROR_RETURN(_sys_usw_storm_ctl_get_interface_en( lchip, p_stmctl_cfg,&stmctl_offset,&stmctl_en));
    if(!stmctl_en)
    {
         return CTC_E_NONE;
    }
    step = (CTC_SECURITY_STORM_CTL_OP_VLAN == p_stmctl_cfg->op) ?(DsIpeStormCtl1Config_t - DsIpeStormCtl0Config_t) :0;


    CTC_ERROR_RETURN(_sys_usw_storm_ctl_get_profile_from_hw(lchip, stmctl_offset, p_stmctl_cfg, &profile));
    if(profile.profile_id)
    {
        ctc_spool_remove(usw_security_master[lchip]->stmctl_spool, &profile, NULL);
        _sys_usw_storm_ctl_add_hw(lchip, stmctl_offset, p_stmctl_cfg, profile, 0);

    }
    /*2. check if all mode is cleared in the port  */
    for (stmctl_type = CTC_SECURITY_STORM_CTL_BCAST; stmctl_type < CTC_SECURITY_STORM_CTL_MAX; stmctl_type++)
    {
        _sys_usw_stmctl_get_pkt_type_stmctl_base(lchip, stmctl_type, &base);
        if(base >= 4)
         {/*not support stmctl_type*/
            continue;
        }
        tmp_offset = ((stmctl_offset >> 2) << 2) | (base & 0x3) ;
        cmd = DRV_IOR(DsIpeStormCtl0Config_t + step, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tmp_offset >> 1, cmd, &stmctl_cfg));
        if(tmp_offset & 0x1)
        {
            profile.profile_id = GetDsIpeStormCtl0Config(V, profIdY_f, &stmctl_cfg);
        }
        else
        {
            profile.profile_id = GetDsIpeStormCtl0Config(V, profIdX_f, &stmctl_cfg);
        }
        if(profile.profile_id)
        {
            exist = 1;
            break;
        }
    }
    if(!exist)
    {
        _sys_usw_storm_ctl_set_interface_en( lchip,  p_stmctl_cfg, stmctl_offset , 0);
        _sys_usw_stmctl_free_stmctl_offset(lchip, p_stmctl_cfg->op, stmctl_offset);
    }

    return CTC_E_NONE;
}
STATIC int32
sys_usw_security_stormctl_build_data(uint8 lchip, CTC_INTERRUPT_EVENT_FUNC event_cb, uint32 stormctl_table_id, uint32 stormctl_field_id)
{
    uint8  gchip;
    ctc_security_stormctl_event_t event;
    uint32 cmd = 0;
    uint16 stormctl_ptr = 0;
    uint8 loop = 0;
    uint8 stmctl_base = 0;
    uint32 value = 0;
    uint32 offset = 0;
    uint8 index = 0;
    uint32 entry_num = 0;

    sal_memset(&event, 0, sizeof(ctc_security_stormctl_event_t));
    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, stormctl_table_id, &entry_num));
    for(index = 0; index < entry_num; index ++)
    {
        cmd = DRV_IOR(stormctl_table_id, stormctl_field_id);
        DRV_IOCTL(lchip, index, cmd, &value);
        if(value == 0)
        {
            continue;
        }
        for(loop = 0; loop < CTC_SECURITY_STORMCTL_EVENT_MAX_NUM; loop ++)
        {
            if(!CTC_IS_BIT_SET(value,loop))
            {
                continue;
            }
            stormctl_ptr = (index << 5) + loop;
            stmctl_base = stormctl_ptr & 0x3;
            offset = stormctl_ptr >> 2;
            CTC_ERROR_RETURN(_sys_usw_stmctl_get_pkt_type_from_stmctl_base(lchip, &(event.stormctl_state[event.valid_num].type), stmctl_base));
            if(stormctl_table_id == IpeStormCtl0State_t)
            {
                event.stormctl_state[event.valid_num].gport = usw_security_master[lchip]->port_id[offset];
                event.stormctl_state[event.valid_num].op = CTC_SECURITY_STORM_CTL_OP_PORT;
            }
            else
            {
                event.stormctl_state[event.valid_num].vlan_id = usw_security_master[lchip]->vlan_id[offset];
                event.stormctl_state[event.valid_num].op = CTC_SECURITY_STORM_CTL_OP_VLAN;
            }

            event.valid_num ++;
        }
        CTC_ERROR_RETURN(event_cb(gchip, &event));
        sal_memset(&event, 0, sizeof(ctc_security_stormctl_event_t));
    }

    return CTC_E_NONE;
}
STATIC int32
sys_usw_security_stormctl_state_isr(uint8 lchip, uint32 intr, void* p_data)
{
    uint32 stormctl_table_id = 0;
    uint32 stormctl_field_id = 0;
    CTC_INTERRUPT_EVENT_FUNC event_cb;

    CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_STORMCTL_STATE, &event_cb));
    if(event_cb)
    {
        stormctl_table_id = IpeStormCtl0State_t;
        stormctl_field_id = IpeStormCtl0State_violationOccurred_f;
        CTC_ERROR_RETURN(sys_usw_security_stormctl_build_data(lchip, event_cb, stormctl_table_id, stormctl_field_id));

        stormctl_table_id = IpeStormCtl1State_t;
        stormctl_field_id = IpeStormCtl1State_violationOccurred_f;
        CTC_ERROR_RETURN(sys_usw_security_stormctl_build_data(lchip, event_cb, stormctl_table_id, stormctl_field_id));
    }

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_security_stmctl_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 entry_num = 0;
    uint32 value = 1;
    IpeStormCtl_m ipe_storm_ctl;
    RefDivStormCtlUpdPulse_m stmctl_udp_pulse;
    IpeStormCtl0EngineCtl_m engine_ctl;
    DsIpeStormCtlSharingProfile_m sharing_profile;
    DsIpeStormCtl0ProfileX_m init_profile;
    BufferStoreCtl_m buffer_store_ctl;

    sal_memset(&ipe_storm_ctl,    0, sizeof(IpeStormCtl_m));
    sal_memset(&engine_ctl,       0, sizeof(IpeStormCtl0EngineCtl_m));
    sal_memset(&sharing_profile,  0, sizeof(DsIpeStormCtlSharingProfile_m));
    sal_memset(&stmctl_udp_pulse, 0, sizeof(RefDivStormCtlUpdPulse_m));
    sal_memset(&init_profile,     0, sizeof(DsIpeStormCtl0ProfileX_m));
    sal_memset(&buffer_store_ctl, 0, sizeof(BufferStoreCtl_m));

    /*storm ctl support known unicast/mcast mode*/
    /* usw_security_master[lchip]->stmctl_cfg.ustorm_ctl_mode = SYS_USW_SECURITY_MODE_SEPERATE;*/
    usw_security_master[lchip]->stmctl_cfg.mstorm_ctl_mode = SYS_USW_SECURITY_MODE_SEPERATE;
    usw_security_master[lchip]->stmctl_cfg.ipg_en = CTC_SECURITY_STORM_CTL_MODE_PPS;
  /*   usw_security_master[lchip]->stmctl_cfg.granularity = CTC_SECURITY_STORM_CTL_GRANU_1000;*/
    /* StormCtlCfg */
    cmd = DRV_IOW(StormCtlCfg_t, StormCtlCfg_stormCtlViolationInteruptEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    /*IpeStormCtl0EngineCtl, IpeStormCtl1EngineCtl*/
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsIpeStormCtl0Config_t, &entry_num));
    SetIpeStormCtl0EngineCtl(V, ipeStormCtl0EngineCtlBucketPassConditionMode_f, &engine_ctl, 1);
    SetIpeStormCtl0EngineCtl(V, ipeStormCtl0EngineCtlInterval_f, &engine_ctl, 1);
    SetIpeStormCtl0EngineCtl(V, ipeStormCtl0EngineCtlRefreshEn_f, &engine_ctl, 1);
    SetIpeStormCtl0EngineCtl(V, ipeStormCtl0EngineCtlMaxPtr_f, &engine_ctl, entry_num - 1);
    cmd = DRV_IOW(IpeStormCtl0EngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &engine_ctl));
    cmd = DRV_IOW(IpeStormCtl1EngineCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &engine_ctl));

    /*RefDivStormCtlUpdPulse*/
    SetRefDivStormCtlUpdPulse(V, cfgResetDivStormCtl0UpdPulse_f, &stmctl_udp_pulse, 0);
    SetRefDivStormCtlUpdPulse(V, cfgRefDivStormCtl0UpdPulse_f, &stmctl_udp_pulse, MCHIP_CAP(SYS_CAP_STMCTL_DIV_PULSE));
    SetRefDivStormCtlUpdPulse(V, cfgResetDivStormCtl1UpdPulse_f, &stmctl_udp_pulse, 0);
    SetRefDivStormCtlUpdPulse(V, cfgRefDivStormCtl1UpdPulse_f, &stmctl_udp_pulse, MCHIP_CAP(SYS_CAP_STMCTL_DIV_PULSE));
    cmd = DRV_IOW(RefDivStormCtlUpdPulse_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &stmctl_udp_pulse));

    /*IpeStormCtl*/
    cmd = DRV_IOR(IpeStormCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_storm_ctl));
    SetIpeStormCtl(V, ipgEnStormCtl_f, &ipe_storm_ctl, 0);
    SetIpeStormCtl(V, oamObeyStormCtl_f, &ipe_storm_ctl, 0);
    SetIpeStormCtl(V, lagecyStormCtlControlMode_f, &ipe_storm_ctl, 1);
    SetIpeStormCtl(V, stormCtlMode_f, &ipe_storm_ctl, 1); /*1: support port and vlan
                                                            0: only support port*/
    /*bc*/
    SetIpeStormCtl(V, g_1_ptrOffset_f, &ipe_storm_ctl, 0);
    SetIpeStormCtl(V, g_5_ptrOffset_f, &ipe_storm_ctl, 0);
    /*unknow uc*/
    SetIpeStormCtl(V, g_0_ptrOffset_f, &ipe_storm_ctl, 1);
    /*seperate know/unknow mc*/
    SetIpeStormCtl(V, g_2_ptrOffset_f, &ipe_storm_ctl, 3);
    SetIpeStormCtl(V, g_6_ptrOffset_f, &ipe_storm_ctl, 2);
    /*if ptrOffset(2) == 0, stormCtlPtrValid will be 0*/
    SetIpeStormCtl(V, g_3_ptrOffset_f, &ipe_storm_ctl, 4);
    SetIpeStormCtl(V, g_4_ptrOffset_f, &ipe_storm_ctl, 4);/*know uc*/
    SetIpeStormCtl(V, g_7_ptrOffset_f, &ipe_storm_ctl, 4);
    cmd = DRV_IOW(IpeStormCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_storm_ctl));

    /*ctl0: not green, ctl1: not green*/
    SetDsIpeStormCtlSharingProfile(V, drop_f, &sharing_profile, 1);
    SetDsIpeStormCtlSharingProfile(V, g_0_decX_f, &sharing_profile, 0);
    SetDsIpeStormCtlSharingProfile(V, g_0_decY_f, &sharing_profile, 0);
    SetDsIpeStormCtlSharingProfile(V, g_1_decX_f, &sharing_profile, 0);
    SetDsIpeStormCtlSharingProfile(V, g_1_decY_f, &sharing_profile, 0);
    cmd = DRV_IOW(DsIpeStormCtlSharingProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &sharing_profile));

    /*ctl0: not green, ctl1: green*/
    SetDsIpeStormCtlSharingProfile(V, drop_f, &sharing_profile, 1);
    SetDsIpeStormCtlSharingProfile(V, g_0_decX_f, &sharing_profile, 0);
    SetDsIpeStormCtlSharingProfile(V, g_0_decY_f, &sharing_profile, 0);
    SetDsIpeStormCtlSharingProfile(V, g_1_decX_f, &sharing_profile, 0);
    SetDsIpeStormCtlSharingProfile(V, g_1_decY_f, &sharing_profile, 0);
    cmd = DRV_IOW(DsIpeStormCtlSharingProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 1, cmd, &sharing_profile));

    /*ctl0: green, ctl1: not green*/
    SetDsIpeStormCtlSharingProfile(V, drop_f, &sharing_profile, 1);
    SetDsIpeStormCtlSharingProfile(V, g_0_decX_f, &sharing_profile, 0);
    SetDsIpeStormCtlSharingProfile(V, g_0_decY_f, &sharing_profile, 0);
    SetDsIpeStormCtlSharingProfile(V, g_1_decX_f, &sharing_profile, 0);
    SetDsIpeStormCtlSharingProfile(V, g_1_decY_f, &sharing_profile, 0);
    cmd = DRV_IOW(DsIpeStormCtlSharingProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 2, cmd, &sharing_profile));

    /*ctl0: green, ctl1: green*/
    SetDsIpeStormCtlSharingProfile(V, drop_f, &sharing_profile, 0);
    SetDsIpeStormCtlSharingProfile(V, g_0_decX_f, &sharing_profile, 1);
    SetDsIpeStormCtlSharingProfile(V, g_0_decY_f, &sharing_profile, 1);
    SetDsIpeStormCtlSharingProfile(V, g_1_decX_f, &sharing_profile, 1);
    SetDsIpeStormCtlSharingProfile(V, g_1_decY_f, &sharing_profile, 1);
    cmd = DRV_IOW(DsIpeStormCtlSharingProfile_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 3, cmd, &sharing_profile));

    /*init profileX/Y 0*/
    SetDsIpeStormCtl0ProfileX(V, ppsShift_f, &init_profile, 7);
    SetDsIpeStormCtl0ProfileX(V, rateFrac_f, &init_profile, 0xFF);
    SetDsIpeStormCtl0ProfileX(V, rateMaxShift_f, &init_profile, 0xF);
    SetDsIpeStormCtl0ProfileX(V, rateMax_f, &init_profile, 0xFFFF);
    SetDsIpeStormCtl0ProfileX(V, rateShift_f, &init_profile, 0xF);
    SetDsIpeStormCtl0ProfileX(V, rate_f, &init_profile, 0xFFFF);
    SetDsIpeStormCtl0ProfileX(V, thresholdShift_f, &init_profile, 0xF);
    SetDsIpeStormCtl0ProfileX(V, threshold_f, &init_profile, 0xFFF);
    cmd = DRV_IOW(DsIpeStormCtl0ProfileX_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &init_profile));
    cmd = DRV_IOW(DsIpeStormCtl0ProfileY_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &init_profile));
    cmd = DRV_IOW(DsIpeStormCtl1ProfileX_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &init_profile));
    cmd = DRV_IOW(DsIpeStormCtl1ProfileY_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &init_profile));

    CTC_ERROR_RETURN(sys_usw_interrupt_register_isr(lchip, SYS_INTR_FUNC_STMCTL_STATE, sys_usw_security_stormctl_state_isr));

    return CTC_E_NONE;
}
#define __________Storm_Control_API_____________
int32
sys_usw_storm_ctl_set_cfg(uint8 lchip, ctc_security_stmctl_cfg_t* p_stmctl_cfg)
{
    SYS_SECURITY_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_stmctl_cfg);

    if (CTC_SECURITY_STORM_CTL_OP_PORT == p_stmctl_cfg->op)
    {
        SYS_GLOBAL_PORT_CHECK(p_stmctl_cfg->gport);
    }
    else
    {
        CTC_VLAN_RANGE_CHECK(p_stmctl_cfg->vlan_id);
    }

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,
                         "type:%d, mode:%d, gport:0x%04X, vlan:%d, op:%d, storm_en:%d, discarded_to_cpu:%d, threshold :%d \n",
                         p_stmctl_cfg->type, p_stmctl_cfg->mode, p_stmctl_cfg->gport, p_stmctl_cfg->vlan_id,
                         p_stmctl_cfg->op, p_stmctl_cfg->storm_en, p_stmctl_cfg->discarded_to_cpu, p_stmctl_cfg->threshold);

    /*ucast mode judge*/
    SYS_STMCTL_CHK_UCAST(p_stmctl_cfg->type);

    /*Mcast mode judge*/
    SYS_STMCTL_CHK_MCAST(usw_security_master[lchip]->stmctl_cfg.mstorm_ctl_mode, p_stmctl_cfg->type);
    SYS_SECURITY_LOCK(lchip);
    if(p_stmctl_cfg->storm_en)
    {
        CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_storm_ctl_set_mode_enable(lchip, p_stmctl_cfg));
    }
    else
    {
        CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_storm_ctl_set_mode_disable(lchip, p_stmctl_cfg));
    }
    SYS_SECURITY_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_storm_ctl_get_cfg(uint8 lchip, ctc_security_stmctl_cfg_t* p_stmctl_cfg)
{
    uint8  step          = 0;
    uint32 cmd           = 0;
    uint8 stmctl_en     = 0;
    uint32 stmctl_offset = 0;
    uint8  is_pps        = 0;
    uint32 hw_rate       = 0;
    uint32 user_rate     = 0;
    sys_stmctl_profile_t  profile;
    DsIpeStormCtl0Config_m stmctl_cfg;

    SYS_SECURITY_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_stmctl_cfg);

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,
                         "type:%d, mode:%d, gport:0x%04X ,vlan:%d, op:%d, storm_en:%d, discarded_to_cpu:%d ,threshold:%d\n",
                         p_stmctl_cfg->type, p_stmctl_cfg->mode, p_stmctl_cfg->gport, p_stmctl_cfg->vlan_id,
                         p_stmctl_cfg->op, p_stmctl_cfg->storm_en, p_stmctl_cfg->discarded_to_cpu, p_stmctl_cfg->threshold);

    /*ucast mode judge*/
    SYS_STMCTL_CHK_UCAST(p_stmctl_cfg->type);

    /*Mcast mode judge*/
    SYS_STMCTL_CHK_MCAST(usw_security_master[lchip]->stmctl_cfg.mstorm_ctl_mode, p_stmctl_cfg->type);

    sal_memset(&profile, 0, sizeof(sys_stmctl_profile_t));
    sal_memset(&stmctl_cfg, 0, sizeof(DsIpeStormCtl0Config_m));
    SYS_SECURITY_LOCK(lchip);

    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_storm_ctl_get_interface_en( lchip, p_stmctl_cfg,&stmctl_offset,&stmctl_en));


    if(!stmctl_en)
    {
       SYS_SECURITY_UNLOCK(lchip);
       return CTC_E_NONE;
    }
    step = (CTC_SECURITY_STORM_CTL_OP_VLAN == p_stmctl_cfg->op) ?(DsIpeStormCtl1Config_t - DsIpeStormCtl0Config_t) :0;

    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, _sys_usw_storm_ctl_get_profile_from_hw(lchip, stmctl_offset, p_stmctl_cfg, &profile));
    if(profile.profile_id)
    {
        p_stmctl_cfg->op = profile.op;
        p_stmctl_cfg->storm_en = 1;

        cmd = DRV_IOR(DsIpeStormCtl0Config_t + step, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, stmctl_offset >> 1, cmd, &stmctl_cfg));

        if(stmctl_offset & 0x1)
        {
            p_stmctl_cfg->discarded_to_cpu = GetDsIpeStormCtl0Config(V, exceptionEnY_f, &stmctl_cfg);
            is_pps = GetDsIpeStormCtl0Config(V, ppsModeY_f, &stmctl_cfg);
        }
        else
        {
            p_stmctl_cfg->discarded_to_cpu = GetDsIpeStormCtl0Config(V, exceptionEnX_f, &stmctl_cfg);
            is_pps = GetDsIpeStormCtl0Config(V, ppsModeX_f, &stmctl_cfg);
        }
        hw_rate = (profile.rate << 12) | (profile.rate_shift << 8) | (profile.rate_frac);
        CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, sys_usw_qos_policer_map_token_rate_hw_to_user(lchip, is_pps, hw_rate, &user_rate, MCHIP_CAP(SYS_CAP_STMCTL_UPD_FREQ)));
        if(is_pps)
        {
            p_stmctl_cfg->mode = CTC_SECURITY_STORM_CTL_MODE_PPS;
            p_stmctl_cfg->threshold = user_rate;
        }
        else
        {
            p_stmctl_cfg->mode = CTC_SECURITY_STORM_CTL_MODE_KBPS;
            p_stmctl_cfg->threshold = user_rate / 8;    /*Byte*/
        }
    }

    SYS_SECURITY_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_storm_ctl_set_global_cfg(uint8 lchip, ctc_security_stmctl_glb_cfg_t* p_glb_cfg)
{
    uint32 cmd = 0;
    uint32 value = 0;
    IpeStormCtl_m ipe_storm_ctl;

    SYS_SECURITY_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_glb_cfg);

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ipg_en%d ustorm_ctl_mode:%d,mstorm_ctl_mode:%d\n",\
                          p_glb_cfg->ipg_en, p_glb_cfg->ustorm_ctl_mode, p_glb_cfg->mstorm_ctl_mode);

    SYS_SECURITY_LOCK(lchip);
    sal_memset(&ipe_storm_ctl, 0, sizeof(IpeStormCtl_m));
    cmd = DRV_IOR(IpeStormCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, 0, cmd, &ipe_storm_ctl));

    value = p_glb_cfg->ipg_en ? 1 : 0;
    SetIpeStormCtl(V, ipgEnStormCtl_f, &ipe_storm_ctl, value);

    /*bc*/
    SetIpeStormCtl(V, g_1_ptrOffset_f, &ipe_storm_ctl, 0);
    SetIpeStormCtl(V, g_5_ptrOffset_f, &ipe_storm_ctl, 0);
    /*unknow uc*/
    SetIpeStormCtl(V, g_0_ptrOffset_f, &ipe_storm_ctl, 1);
    if(p_glb_cfg->mstorm_ctl_mode)
    {   /*seperate know/unknow mc*/
        SetIpeStormCtl(V, g_2_ptrOffset_f, &ipe_storm_ctl, 3);
        SetIpeStormCtl(V, g_6_ptrOffset_f, &ipe_storm_ctl, 2);
    }
    else    /*merge know/unknow mc*/
    {
        SetIpeStormCtl(V, g_2_ptrOffset_f, &ipe_storm_ctl, 3);
        SetIpeStormCtl(V, g_6_ptrOffset_f, &ipe_storm_ctl, 3);
    }

    /*if ptrOffset(2) == 0, stormCtlPtrValid will be 0*/
    SetIpeStormCtl(V, g_3_ptrOffset_f, &ipe_storm_ctl, 4);
    SetIpeStormCtl(V, g_4_ptrOffset_f, &ipe_storm_ctl, 4);/*know uc*/
    SetIpeStormCtl(V, g_7_ptrOffset_f, &ipe_storm_ctl, 4);

    cmd = DRV_IOW(IpeStormCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, 0, cmd, &ipe_storm_ctl));

    usw_security_master[lchip]->stmctl_cfg.ipg_en          = p_glb_cfg->ipg_en;
    usw_security_master[lchip]->stmctl_cfg.mstorm_ctl_mode = p_glb_cfg->mstorm_ctl_mode;
    SYS_SECURITY_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_storm_ctl_get_global_cfg(uint8 lchip, ctc_security_stmctl_glb_cfg_t* p_glb_cfg)
{
    uint32 cmd = 0;
    IpeStormCtl_m ipe_storm_ctl;

    SYS_SECURITY_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_glb_cfg);

    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&ipe_storm_ctl, 0, sizeof(IpeStormCtl_m));

    SYS_SECURITY_LOCK(lchip);
    cmd = DRV_IOR(IpeStormCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, 0, cmd, &ipe_storm_ctl));

    p_glb_cfg->ipg_en = GetIpeStormCtl(V, ipgEnStormCtl_f, &ipe_storm_ctl);
    p_glb_cfg->mstorm_ctl_mode = usw_security_master[lchip]->stmctl_cfg.mstorm_ctl_mode;


    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ipg_en%d ustorm_ctl_mode:%d,mstorm_ctl_mode:%d\n",\
                           p_glb_cfg->ipg_en, p_glb_cfg->ustorm_ctl_mode, p_glb_cfg->mstorm_ctl_mode);
    SYS_SECURITY_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_security_show_storm_ctl(uint8 lchip, uint8 op, uint32 gport_or_vlan)
{
    uint8  base  = 0;
    uint8  step  = 0;
    uint8  is_pps = 0;
    uint32 cmd   = 0;
    uint32 profile_id = 0;
    uint32 stmctl_offset = 0;
    uint32 tmp_offset = 0;
    uint8 stmctl_en = 0;
    uint8  stmctl_type = 0;
    DsIpeStormCtl0Config_m stmctl_cfg;
    char*  type_name[7] = {"bc", "know-mc", "unknow-mc", "mc", "know-uc", "unkown-uc", "uc"};
    ctc_security_stmctl_cfg_t sec_stmctl_cfg;

    SYS_SECURITY_INIT_CHECK(lchip);

    sal_memset(&stmctl_cfg, 0, sizeof(DsIpeStormCtl0Config_m));
    SYS_SECURITY_LOCK(lchip);

    sal_memset(&sec_stmctl_cfg, 0, sizeof(ctc_security_stmctl_cfg_t));

    sec_stmctl_cfg.op = op;
    sec_stmctl_cfg.gport = gport_or_vlan;
    sec_stmctl_cfg.vlan_id = gport_or_vlan;
    CTC_ERROR_RETURN_SECURITY_UNLOCK( lchip, _sys_usw_storm_ctl_get_interface_en( lchip, &sec_stmctl_cfg,&stmctl_offset,&stmctl_en));

    if (CTC_SECURITY_STORM_CTL_OP_PORT == op)
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "DsIpeStormCtl0Config, DsIpeStormCtl0ProfileX/Y\n");
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Gport     Type        Mode      Profile-Id  Offset    \n");
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------\n");
        if(!stmctl_en)
        {
            SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "storm ctl not enable on port %d\n", gport_or_vlan);
            SYS_SECURITY_UNLOCK(lchip);
            return CTC_E_NONE;
        }
    }
    else
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "DsIpeStormCtl1Config, DsIpeStormCtl1ProfileX/Y\n");
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Vlan-Id   Type        Mode      Profile-Id  Offset    \n");
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------\n");
        if(!stmctl_en)
        {
            SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "storm ctl not enable on vlan %d\n", gport_or_vlan);
            SYS_SECURITY_UNLOCK(lchip);
            return CTC_E_NONE;
        }
    }
    step = (CTC_SECURITY_STORM_CTL_OP_VLAN == op) ?(DsIpeStormCtl1Config_t - DsIpeStormCtl0Config_t) :0;
    for (stmctl_type = CTC_SECURITY_STORM_CTL_BCAST; stmctl_type < CTC_SECURITY_STORM_CTL_MAX; stmctl_type++)
    {
        _sys_usw_stmctl_get_pkt_type_stmctl_base(lchip, stmctl_type, &base);
        if(base >= 4)
         {/*not support stmctl_type*/
            continue;
        }
        tmp_offset =  ((stmctl_offset >> 2) << 2) | (base & 0x3);
        cmd = DRV_IOR(DsIpeStormCtl0Config_t + step, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_SECURITY_UNLOCK(lchip, DRV_IOCTL(lchip, tmp_offset >> 1, cmd, &stmctl_cfg));
        if(tmp_offset & 0x1)
        {
            profile_id = GetDsIpeStormCtl0Config(V, profIdY_f, &stmctl_cfg);
            is_pps = GetDsIpeStormCtl0Config(V, ppsModeY_f, &stmctl_cfg);
        }
        else
        {
            profile_id = GetDsIpeStormCtl0Config(V, profIdX_f, &stmctl_cfg);
            is_pps = GetDsIpeStormCtl0Config(V, ppsModeX_f, &stmctl_cfg);
        }
        if(profile_id)
        {
            SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-8x%-12s%-10s%-3s%-9d%-8u\n", gport_or_vlan,
                   type_name[stmctl_type], is_pps?"pps":"bps",
                   (tmp_offset&0x1)?"Y: ":"X: ", profile_id, tmp_offset>>1);
        }

    }

    SYS_SECURITY_UNLOCK(lchip);
    return CTC_E_NONE;
}

/*just for test*/
int32
sys_usw_security_show_spool_status(uint8 lchip)
{
    SYS_SECURITY_INIT_CHECK(lchip);
    SYS_SECURITY_LOCK(lchip);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "No.   name        max_count    used_cout\n");
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "1     limit       %-13u%-10u\n", usw_security_master[lchip]->trhold_spool->max_count, \
                                                       usw_security_master[lchip]->trhold_spool->count);
    SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "1     stm-ctl     %-13u%-10u\n", usw_security_master[lchip]->stmctl_spool->max_count, \
                                                       usw_security_master[lchip]->stmctl_spool->count);
    SYS_SECURITY_UNLOCK(lchip);
    return CTC_E_NONE;
}

STATIC int32
sys_usw_security_trhold_profile_wb_sync_func(void* node, void* user_data)
{
    uint16 max_entry_cnt = 0;
    sys_learn_limit_trhold_node_t *p_trhold_profile = (sys_learn_limit_trhold_node_t *)((ctc_spool_node_t *)node)->data;
    sys_wb_security_trhold_profile_t *p_wb_trhold_profile;
    sys_traverse_t *traversal_data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)traversal_data->data;

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_trhold_profile = (sys_wb_security_trhold_profile_t *)wb_data->buffer + wb_data->valid_cnt;

    sal_memset(p_wb_trhold_profile, 0, sizeof(sys_wb_security_trhold_profile_t));

    p_wb_trhold_profile->value = p_trhold_profile->value;
    p_wb_trhold_profile->index = p_trhold_profile->index;

    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_storm_ctl_wb_restore_spool(uint8 lchip, uint32 field_x, uint32 field_y, sys_stmctl_profile_t *p_stmctl_profile)
{
    if(field_x != 0)
    {
        p_stmctl_profile->is_y = 0;
        p_stmctl_profile->profile_id = field_x;
        CTC_ERROR_RETURN(_sys_usw_storm_ctl_get_profile(lchip, p_stmctl_profile));
        CTC_ERROR_RETURN(ctc_spool_add(usw_security_master[lchip]->stmctl_spool,
                                                                   p_stmctl_profile,
                                                                   NULL,
                                                                   NULL));
    }
    if(field_y != 0)
    {
        p_stmctl_profile->is_y = 1;
        p_stmctl_profile->profile_id = field_y;
        CTC_ERROR_RETURN(_sys_usw_storm_ctl_get_profile(lchip, p_stmctl_profile));
        CTC_ERROR_RETURN(ctc_spool_add(usw_security_master[lchip]->stmctl_spool,
                                                                   p_stmctl_profile,
                                                                   NULL,
                                                                   NULL));
    }

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_security_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_traverse_t user_data;
    sys_wb_security_master_t *p_wb_security_master;
    uint8 work_status = 0;
    if (NULL == usw_security_master[lchip])
    {
        return CTC_E_NONE;
    }
    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	return CTC_E_NONE;
    }
    SYS_SECURITY_LOCK(lchip);

    /*syncup buffer*/
    wb_data.buffer = mem_malloc(MEM_STMCTL_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        SYS_SECURITY_UNLOCK(lchip);
        return CTC_E_NO_MEMORY;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    user_data.data = &wb_data;
    user_data.value1 = lchip;

    /* syncup master */
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_security_master_t, CTC_FEATURE_SECURITY, SYS_WB_APPID_SECURITY_SUBID_MASTER);

    p_wb_security_master = (sys_wb_security_master_t *)wb_data.buffer;
    sal_memset(p_wb_security_master, 0, sizeof(sys_wb_security_master_t));

    p_wb_security_master->lchip = lchip;
    p_wb_security_master->version = SYS_WB_VERSION_SECURITY;

    if (usw_security_master[lchip]->flag & SYS_SECURITY_IPSG_MAC_SUPPORT)
    {
        p_wb_security_master->ipsg_flag |= SYS_SECURITY_IPSG_MAC_SUPPORT;
    }

    if (usw_security_master[lchip]->flag & SYS_SECURITY_IPSG_IPV4_SUPPORT)
    {
        p_wb_security_master->ipsg_flag |= SYS_SECURITY_IPSG_IPV4_SUPPORT;
    }

    if (usw_security_master[lchip]->flag & SYS_SECURITY_IPSG_IPV6_SUPPORT)
    {
        p_wb_security_master->ipsg_flag |= SYS_SECURITY_IPSG_IPV6_SUPPORT;
    }

    p_wb_security_master->ip_src_guard_def = usw_security_master[lchip]->ip_src_guard_def;
    p_wb_security_master->hash_group0 = usw_security_master[lchip]->hash_group[0];
    p_wb_security_master->hash_group1 = usw_security_master[lchip]->hash_group[1];
    p_wb_security_master->tcam_group0 = usw_security_master[lchip]->tcam_group[0];
    p_wb_security_master->tcam_group1 = usw_security_master[lchip]->tcam_group[1];

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    /* syncup trhold profile */
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_security_trhold_profile_t, CTC_FEATURE_SECURITY, SYS_WB_APPID_SECURITY_SUBID_TRHOLD_PROFILE);

    CTC_ERROR_GOTO(ctc_spool_traverse(usw_security_master[lchip]->trhold_spool,
                                                sys_usw_security_trhold_profile_wb_sync_func, &user_data), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    SYS_SECURITY_UNLOCK(lchip);

    return ret;
}

STATIC int32
_sys_usw_security_wb_restore(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    uint16 entry_cnt = 0;
    ctc_wb_query_t wb_query;
    sys_wb_security_master_t wb_security_master;
    sys_wb_security_trhold_profile_t wb_trhold_profile;
    sys_learn_limit_trhold_node_t trhold_profile;
    sys_learn_limit_trhold_node_t *p_trhold_profile;
    sys_usw_opf_t opf;
    sys_stmctl_profile_t stmctl_profile;
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 profile_id = 0;
    uint32 field_x = 0;
    uint32 field_y = 0;
    uint32 entry_num = 0;
    uint32 loop = 0;
    uint32 temploop = 0;
    uint8  gchip = 0;
    IpeStormCtl_m ipe_storm_ctl;

    SYS_SECURITY_LOCK(lchip);

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_STMCTL_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    /* restore master */
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_security_master_t, CTC_FEATURE_SECURITY, SYS_WB_APPID_SECURITY_SUBID_MASTER);

    sal_memset(&wb_security_master, 0, sizeof(sys_wb_security_master_t));

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);
    if ((wb_query.valid_cnt != 1) || (wb_query.is_end != 1))
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "query security master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_NONE;
        goto done;
    }

    sal_memcpy((uint8 *)&wb_security_master, (uint8 *)wb_query.buffer, wb_query.key_len + wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_SECURITY, wb_security_master.version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    if (wb_security_master.ipsg_flag & SYS_SECURITY_IPSG_MAC_SUPPORT)
    {
        usw_security_master[lchip]->flag |= SYS_SECURITY_IPSG_MAC_SUPPORT;
    }

    if (wb_security_master.ipsg_flag & SYS_SECURITY_IPSG_IPV4_SUPPORT)
    {
        usw_security_master[lchip]->flag |= SYS_SECURITY_IPSG_IPV4_SUPPORT;
    }

    if (wb_security_master.ipsg_flag & SYS_SECURITY_IPSG_IPV6_SUPPORT)
    {
        usw_security_master[lchip]->flag |= SYS_SECURITY_IPSG_IPV6_SUPPORT;
    }

    usw_security_master[lchip]->ip_src_guard_def = wb_security_master.ip_src_guard_def;
    usw_security_master[lchip]->hash_group[0] = wb_security_master.hash_group0;
    usw_security_master[lchip]->hash_group[1] = wb_security_master.hash_group1;
    usw_security_master[lchip]->tcam_group[0] = wb_security_master.tcam_group0;
    usw_security_master[lchip]->tcam_group[1] = wb_security_master.tcam_group1;
    sal_memset(&ipe_storm_ctl, 0, sizeof(IpeStormCtl_m));
    cmd = DRV_IOR(IpeStormCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &ipe_storm_ctl), ret, done);

    value = GetIpeStormCtl(V, ipgEnStormCtl_f, &ipe_storm_ctl);
    usw_security_master[lchip]->stmctl_cfg.ipg_en = value;

    value = GetIpeStormCtl(V, g_6_ptrOffset_f, &ipe_storm_ctl);
    switch (value)
    {
        case 3:
        {
            usw_security_master[lchip]->stmctl_cfg.mstorm_ctl_mode = SYS_USW_SECURITY_MODE_MERGE;
            break;
        }
        case 2:
        {
            usw_security_master[lchip]->stmctl_cfg.mstorm_ctl_mode = SYS_USW_SECURITY_MODE_SEPERATE;
            break;
        }
        default:
        {
            usw_security_master[lchip]->stmctl_cfg.mstorm_ctl_mode = SYS_USW_SECURITY_MODE_SEPERATE;
            break;
        }
    }

    /* restore trhold profile */
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_security_trhold_profile_t, CTC_FEATURE_SECURITY, SYS_WB_APPID_SECURITY_SUBID_TRHOLD_PROFILE);

    sal_memset(&wb_trhold_profile, 0, sizeof(sys_wb_security_trhold_profile_t));

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));

    sal_memcpy((uint8 *)&wb_trhold_profile, (uint8 *)wb_query.buffer + entry_cnt * (wb_query.key_len + wb_query.data_len),
                                                    (wb_query.key_len + wb_query.data_len));
    entry_cnt++;

    sal_memset(&trhold_profile, 0, sizeof(sys_learn_limit_trhold_node_t));
    trhold_profile.value = wb_trhold_profile.value;
    trhold_profile.index = wb_trhold_profile.index;

    CTC_ERROR_GOTO(ctc_spool_add(usw_security_master[lchip]->trhold_spool,
                                                                   &trhold_profile,
                                                                   NULL,
                                                                   &p_trhold_profile), ret, done);

    CTC_WB_QUERY_ENTRY_END((&wb_query));

    /* restore stmctl offset */
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = usw_security_master[lchip]->storm_ctl_opf_type;
    opf.pool_index = 0;
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsSrcPort_t, &entry_num), ret, done);
    for(loop = 0 ;loop < entry_num; loop ++)
    {
        cmd = DRV_IOR(DsSrcPort_t , DsSrcPort_portStormCtlEn_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, loop, cmd, &value), ret, done);
        if(value == 0)
        {
            continue;
        }

        cmd = DRV_IOR(DsSrcPort_t , DsSrcPort_portStormCtlPtr_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, loop, cmd, &value), ret, done);
        CTC_ERROR_GOTO(sys_usw_get_gchip_id(lchip, &gchip), ret, done);
        usw_security_master[lchip]->port_id[value] = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, loop);
        CTC_ERROR_GOTO(sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, value), ret, done);
        for(temploop = 0 ;temploop < 2; temploop ++)
        {
            cmd = DRV_IOR(DsIpeStormCtl0Config_t , DsIpeStormCtl0Config_profIdX_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, value*2+temploop, cmd, &field_x), ret, done);
            cmd = DRV_IOR(DsIpeStormCtl0Config_t , DsIpeStormCtl0Config_profIdY_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, value*2+temploop, cmd, &field_y), ret, done);
            sal_memset(&stmctl_profile, 0, sizeof(sys_stmctl_profile_t));
            stmctl_profile.op = CTC_SECURITY_STORM_CTL_OP_PORT;
            CTC_ERROR_GOTO(_sys_usw_storm_ctl_wb_restore_spool(lchip, field_x, field_y, &stmctl_profile), ret, done);
        }


    }

    opf.pool_index = 1;
    CTC_ERROR_GOTO(sys_usw_ftm_query_table_entry_num(lchip, DsSrcVlanProfile_t, &entry_num), ret, done);
    for(loop = 0 ;loop < entry_num; loop ++)
    {
        cmd = DRV_IOR(DsSrcVlanProfile_t , DsSrcVlanProfile_vlanStormCtlEn_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, loop, cmd, &value), ret, done);
        if(value == 0)
        {
            continue;
        }
        cmd = DRV_IOR(DsSrcVlanProfile_t , DsSrcVlanProfile_vlanStormCtlPtr_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, loop, cmd, &value), ret, done);
        CTC_ERROR_GOTO(sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, value), ret, done);
        for(temploop = 0 ;temploop < 4096; temploop ++)
        {
            cmd = DRV_IOR(DsSrcVlan_t , DsSrcVlan_profileId_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, loop, cmd, &profile_id), ret, done);
            if(profile_id == loop)
            {
                usw_security_master[lchip]->vlan_id[value] = temploop;
            }
        }
        for(temploop = 0 ;temploop < 2; temploop ++)
        {
            cmd = DRV_IOR(DsIpeStormCtl1Config_t , DsIpeStormCtl1Config_profIdX_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, value*2+temploop, cmd, &field_x), ret, done);
            cmd = DRV_IOR(DsIpeStormCtl1Config_t , DsIpeStormCtl1Config_profIdY_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, value*2+temploop, cmd, &field_y), ret, done);
            sal_memset(&stmctl_profile, 0, sizeof(sys_stmctl_profile_t));
            stmctl_profile.op = CTC_SECURITY_STORM_CTL_OP_VLAN;
            CTC_ERROR_GOTO(_sys_usw_storm_ctl_wb_restore_spool(lchip, field_x, field_y, &stmctl_profile), ret, done);

        }
    }

done:
    if (wb_query.key)
    {
        mem_free(wb_query.key);
    }

    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    SYS_SECURITY_UNLOCK(lchip);

    return ret;
}

STATIC int32
_sys_usw_security_trhold_traverse_fprintf_pool(void *node, sys_dump_db_traverse_param_t* user_date)
{
    sal_file_t                  p_file      = (sal_file_t)user_date->value0;
    uint32*                     cnt         = (uint32 *)(user_date->value1);
    sys_learn_limit_trhold_node_t *node_profile = (sys_learn_limit_trhold_node_t *)(((ctc_spool_node_t*)node)->data);
    if (node_profile->value == 0xffffffff)
    {
        SYS_DUMP_DB_LOG(p_file, "%-7u%-11u%-10u%-10s\n",*cnt, node_profile->value, node_profile->index, "static");
    }
    else
    {
        SYS_DUMP_DB_LOG(p_file, "%-7u%-11u%-10u%-10u\n",*cnt, node_profile->value, node_profile->index,((ctc_spool_node_t*)node)->ref_cnt);
    }


    (*cnt)++;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_security_stmctl_traverse_fprintf_pool(void *node, sys_dump_db_traverse_param_t* user_date)
{
    sal_file_t                  p_file      = (sal_file_t)user_date->value0;
    uint32*                     cnt         = (uint32 *)(user_date->value1);
    sys_stmctl_profile_t *node_profile = (sys_stmctl_profile_t *)(((ctc_spool_node_t*)node)->data);

    SYS_DUMP_DB_LOG(p_file, "%-7u%-7u%-7u%-10u%-13u%-16u%-13u%-10u%-11u%-10u\n",*cnt, node_profile->op, node_profile->is_y, node_profile->rate_frac, node_profile->rate_shift,
        node_profile->threshold_shift, node_profile->threshold,  node_profile->rate,  node_profile->profile_id, ((ctc_spool_node_t*)node)->ref_cnt);

    (*cnt)++;

    return CTC_E_NONE;
}


int32
sys_usw_security_dump_db(uint8 lchip, sal_file_t dump_db_fp,ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;
    sys_dump_db_traverse_param_t    cb_data;
    uint32 num_cnt = 1;

    SYS_SECURITY_INIT_CHECK(lchip);
    SYS_SECURITY_LOCK(lchip);

    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "# Security");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "{");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","stmctl_cfg.ipg_en",usw_security_master[lchip]->stmctl_cfg.ipg_en);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","stmctl_cfg.ustorm_ctl_mode",usw_security_master[lchip]->stmctl_cfg.ustorm_ctl_mode);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","stmctl_cfg.mstorm_ctl_mode",usw_security_master[lchip]->stmctl_cfg.mstorm_ctl_mode);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","stmctl_cfg.granularity",usw_security_master[lchip]->stmctl_cfg.granularity);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","flag",usw_security_master[lchip]->flag);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","limit_opf_type",usw_security_master[lchip]->limit_opf_type);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","stmctl_profile0_opf_type",usw_security_master[lchip]->stmctl_profile0_opf_type);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","stmctl_profile1_opf_type",usw_security_master[lchip]->stmctl_profile1_opf_type);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","storm_ctl_opf_type",usw_security_master[lchip]->storm_ctl_opf_type);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","tcam_group[0]",usw_security_master[lchip]->tcam_group[0]);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","tcam_group[1]",usw_security_master[lchip]->tcam_group[1]);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","hash_group[0]",usw_security_master[lchip]->hash_group[0]);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","hash_group[1]",usw_security_master[lchip]->hash_group[1]);
    SYS_DUMP_DB_LOG(dump_db_fp, "%-30s:%u\n","ip_src_guard_def",usw_security_master[lchip]->ip_src_guard_def);

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = dump_db_fp;
    num_cnt = 1;
    cb_data.value1 = &num_cnt;
    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "Spool type: %s\n","security_trhold_pool");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-7s%-11s%-10s%-10s\n","Node","value","index","refcnt");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    ctc_spool_traverse(usw_security_master[lchip]->trhold_spool, (spool_traversal_fn)_sys_usw_security_trhold_traverse_fprintf_pool , (void*)(&cb_data));
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "---------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "\n");

    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = dump_db_fp;
    num_cnt = 1;
    cb_data.value1 = &num_cnt;
    SYS_DUMP_DB_LOG(dump_db_fp, "\n");
    SYS_DUMP_DB_LOG(dump_db_fp, "Spool type: %s\n","security_stmctl_pool");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%-7s%-7s%-7s%-10s%-13s%-16s%-13s%-10s%-11s%-10s\n","Node","op","is_y","rate_frac", "rate_shift","threshold_shift","threshold","rate", "profile_id","refcnt");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------------------------------------");
    ctc_spool_traverse(usw_security_master[lchip]->stmctl_spool, (spool_traversal_fn)_sys_usw_security_stmctl_traverse_fprintf_pool , (void*)(&cb_data));
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "----------------------------------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(dump_db_fp, "%s\n", "}");

    SYS_SECURITY_UNLOCK(lchip);

    return ret;
}

int32
sys_usw_security_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 max_limit = MCHIP_CAP(SYS_CAP_LEARN_LIMIT_MAX);
    uint32 value = 0;
    uint32 index = 0;
    int32 ret;
    uint8 work_status = 0;
    LCHIP_CHECK(lchip);

    if (NULL != usw_security_master[lchip])
    {
        return CTC_E_NONE;
    }
    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
    }
    usw_security_master[lchip] = (sys_security_master_t*)mem_malloc(MEM_STMCTL_MODULE, sizeof(sys_security_master_t));
    if (NULL == usw_security_master[lchip])
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto error0;
    }

    sal_memset(usw_security_master[lchip], 0, sizeof(sys_security_master_t));

    sal_mutex_create(&usw_security_master[lchip]->sec_mutex);
    if (NULL == usw_security_master[lchip]->sec_mutex)
    {
        SYS_SECURITY_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Create mutex fail\n");
        ret = CTC_E_NO_MEMORY;
        goto error1;
    }

    /* init opf */
    CTC_ERROR_GOTO(_sys_usw_security_opf_init(lchip), ret, error2);

    /* creat spool  */
    CTC_ERROR_GOTO(_sys_usw_security_spool_init(lchip), ret, error3);

    /* enable port/vlan/system learn limit global and enable station move global*/
    value = 1;
    cmd = DRV_IOW(FibAccelerationCtl_t, FibAccelerationCtl_portMacLimitAlwaysUpdate_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(FibAccelerationCtl_t, FibAccelerationCtl_profileMacLimitAlwaysUpdate_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(FibAccelerationCtl_t, FibAccelerationCtl_vlanMacLimitAlwaysUpdate_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(FibAccelerationCtl_t, FibAccelerationCtl_dynamicMacLimitAlwaysUpdate_f);

    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(FibAccelerationCtl_t, FibAccelerationCtl_dynamicMacLimitEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(FibAccelerationCtl_t, FibAccelerationCtl_profileMacLimitEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(FibAccelerationCtl_t, FibAccelerationCtl_hwSystemDynamicMacLimitEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_systemMacSecurityEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(FibAccelerationCtl_t, FibAccelerationCtl_vlanMacLimitEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(FibAccelerationCtl_t, FibAccelerationCtl_hwVlanMacLimitEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(IpeLearningCtl_t,IpeLearningCtl_hwVsiMacLimitEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(FibAccelerationCtl_t, FibAccelerationCtl_portMacLimitEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(FibAccelerationCtl_t, FibAccelerationCtl_hwPortMacLimitEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(IpeLearningCtl_t,IpeLearningCtl_hwPortMacLimitEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_stationMoveLearnEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_securityMode_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(FibAccelerationCtl_t,FibAccelerationCtl_dynamicMacDecreaseEn_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);
    cmd = DRV_IOW(IpeLearningCtl_t, IpeLearningCtl_pendingEntrySecurityDisable_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value), ret, error4);

    cmd = DRV_IOW(MacLimitThreshold_t, MacLimitThreshold_maxCount_f);
    for (index = 0; index < MCHIP_CAP(SYS_CAP_LEARN_LIMIT_PROFILE_NUM); index++)
    {
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, index, cmd, &max_limit), ret, error4);
    }

    /* init system learn limit to max */
    cmd = DRV_IOW(MacLimitSystem_t, MacLimitSystem_dynamicMaxCount_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &max_limit), ret, error4);

     /*core_frequency = sys_usw_get_core_freq(lchip, 0);*/
    /*init default storm cfg*/
    CTC_ERROR_GOTO(_sys_usw_security_stmctl_init(lchip), ret, error4);

    usw_security_master[lchip]->ip_src_guard_def = 0;

    /* warmboot register */
    CTC_ERROR_GOTO(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_SECURITY, _sys_usw_security_wb_sync), ret, error4);

    /* warmboot data restore */
    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING) && (work_status == CTC_FTM_MEM_CHANGE_NONE))
    {
        CTC_ERROR_GOTO(_sys_usw_security_wb_restore(lchip), ret, error4);
    }
    else
    {
        CTC_ERROR_GOTO(_sys_usw_ip_source_guard_init_default_entry_group(lchip), ret, error4);
    }

    /* dump-db register */
    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_SECURITY, sys_usw_security_dump_db), ret, error4);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    return CTC_E_NONE;

error4:
    _sys_usw_security_spool_deinit(lchip);
error3:
    _sys_usw_security_opf_deinit(lchip);
error2:
    ctc_sal_mutex_destroy(usw_security_master[lchip]->sec_mutex);
error1:
    mem_free(usw_security_master[lchip]);
error0:
    return ret;
}


int32
sys_usw_security_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == usw_security_master[lchip])
    {
        return CTC_E_NONE;
    }
    sal_mutex_destroy(usw_security_master[lchip]->sec_mutex);
    _sys_usw_security_spool_deinit(lchip);
    _sys_usw_security_opf_deinit(lchip);
    mem_free(usw_security_master[lchip]);

    return CTC_E_NONE;
}
