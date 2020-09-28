#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))

/**
 @file sys_usw_ipmc.h

 @date 2011-11-07

 @version v2.0

*/
#ifndef _SYS_USW_IPMC_H
#define _SYS_USW_IPMC_H
#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_debug.h"
#include "ctc_stats.h"
#include "ctc_hash.h"
#include "ctc_vector.h"
#include "ctc_ipmc.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
#define SYS_IPMC_INIT_CHECK() \
    do { \
        LCHIP_CHECK(lchip); \
        if (p_usw_ipmc_master[lchip] == NULL) \
        { \
            return CTC_E_NOT_INIT; \
        } \
    } while (0)


#define SYS_IPMC_DBG_OUT(level, FMT, ...)                              \
    {                                                                       \
        CTC_DEBUG_OUT(ip,  ipmc, IPMC_SYS, level, FMT, ##__VA_ARGS__);  \
    }

#define IPMC_LOCK   \
    if (p_usw_ipmc_master[lchip]->p_ipmc_mutex) sal_mutex_lock(p_usw_ipmc_master[lchip]->p_ipmc_mutex)

#define IPMC_UNLOCK \
    if (p_usw_ipmc_master[lchip]->p_ipmc_mutex) sal_mutex_unlock(p_usw_ipmc_master[lchip]->p_ipmc_mutex)

#define CTC_ERROR_RETURN_WITH_IPMC_UNLOCK(op) \
    do { \
        int32 rv; \
        if ((rv = (op)) < 0) \
        { \
            sal_mutex_unlock(p_usw_ipmc_master[lchip]->p_ipmc_mutex); \
            return (rv); \
        } \
    } while (0)

#define CTC_RETURN_IPMC_UNLOCK(op) \
    do \
    { \
        sal_mutex_unlock(p_usw_ipmc_master[lchip]->p_ipmc_mutex); \
        return (op); \
    } while (0)

#define SYS_IPMC_MASK_LEN_CHECK(ver, src_mask, dst_mask)         \
    do                                                \
    {                                                 \
        uint8 max_mask_len = (ver == CTC_IP_VER_6) ? 128 : 32; \
        if (((src_mask) != 0 && (src_mask) != max_mask_len) \
            || (dst_mask != max_mask_len)){         \
            return CTC_E_INVALID_PARAM; }               \
    } while (0)

#define SYS_IPMC_IPV6_ADDRESS_SORT(ipv6)                                          \
    {                                                                           \
        uint32 t;                                                           \
        t = ipv6[0];                                \
        ipv6[0] = ipv6[3];  \
        ipv6[3] = t;                                \
                                                                        \
        t = ipv6[1];                                \
        ipv6[1] = ipv6[2];  \
        ipv6[2] = t;                                \
    }

#define SYS_IPMC_GROUP_ADDRESS_SORT(address)                                          \
    {                                                                           \
        uint32 t;                                                           \
        t = address.ipv6.group_addr[0];                                \
        address.ipv6.group_addr[0] = address.ipv6.group_addr[3];  \
        address.ipv6.group_addr[3] = t;                                \
                                                                        \
        t = address.ipv6.group_addr[1];                                \
        address.ipv6.group_addr[1] = address.ipv6.group_addr[2];  \
        address.ipv6.group_addr[2] = t;                                \
                                                                        \
        t = address.ipv6.src_addr[0];                                  \
        address.ipv6.src_addr[0] = address.ipv6.src_addr[3];      \
        address.ipv6.src_addr[3] = t;                                  \
                                                                        \
        t = address.ipv6.src_addr[1];                                  \
        address.ipv6.src_addr[1] = address.ipv6.src_addr[2];      \
        address.ipv6.src_addr[2] = t;                                  \
    }

#define SYS_IPMC_GROUP_ADDRESS_CHECK(ver, address) \
    do \
    {                           \
        if (CTC_IP_VER_4 == (ver)) \
        { \
            if (0xE != ((address).ipv4.group_addr >> 28)) \
            { \
                return CTC_E_INVALID_PARAM; \
            } \
        } \
        else                    \
        { \
            if (0xFF != ((((address).ipv6.group_addr[0]) >> 24) & 0xFF)) \
            { \
                return CTC_E_INVALID_PARAM; \
            } \
        } \
    } while (0)

#define IPMC_MASK_IPV4_ADDR(ip, len)  \
    do                          \
    {                           \
        uint32 mask = (len) ? ~((1 << (CTC_IPV4_ADDR_LEN_IN_BIT - (len))) - 1) : 0; \
        (ip) &= mask;             \
    } while (0)

#define IPMC_MASK_IPV6_ADDR(ip, len)  \
    do                          \
    {                           \
        uint8 feedlen = CTC_IPV6_ADDR_LEN_IN_BIT - (len); \
        uint8 sublen = feedlen % 32; \
        int8 index = feedlen / 32; \
        if (sublen)              \
        {                       \
            uint32 mask = ~(1 << sublen) - 1; \
            (ip)[index] &= mask;  \
        }                       \
        index--;                \
        for (; index >= 0; index--) \
        {                       \
            (ip)[index] = 0;      \
        }                       \
    } while (0)

#define SYS_IPMC_VRFID_CHECK(address, ver) \
    do \
    {  uint16 vrfid = 0; \
       if (CTC_IP_VER_4 == (ver)) \
       { \
           vrfid = (address).ipv4.vrfid; \
       } \
       else \
       { \
           vrfid = (address).ipv6.vrfid; \
       } \
       if ((vrfid) > (MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID)) && (vrfid != 0)) \
       { \
           return CTC_E_BADID; \
       } \
    } while (0)

#define SYS_L2MC_VRFID_CHECK(address, ver) \
    do \
    {  uint16 vrfid = 0; \
       if (CTC_IP_VER_4 == (ver)) \
       { \
           vrfid = (address).ipv4.vrfid; \
       } \
       else \
       { \
           vrfid = (address).ipv6.vrfid; \
       } \
       if ((vrfid) < CTC_MIN_VLAN_ID || (vrfid) > CTC_MAX_VLAN_ID) \
       { \
           return CTC_E_BADID; \
       } \
    } while (0)

#define SYS_IPMC_MASK_ADDR(address, src_mask_len, group_mask_len, ver) \
    do                          \
    {                           \
        if (CTC_IP_VER_4 == (ver)) \
        { \
            IPMC_MASK_IPV4_ADDR((address).ipv4.src_addr, (src_mask_len)); \
            IPMC_MASK_IPV4_ADDR((address).ipv4.group_addr, (group_mask_len)); \
        } \
        else                    \
        { \
            IPMC_MASK_IPV6_ADDR((address).ipv6.src_addr, (src_mask_len)); \
            IPMC_MASK_IPV6_ADDR((address).ipv6.group_addr, (group_mask_len)); \
        } \
    } while (0)

#define SYS_IPMC_BIDIPIM_CHECK(rp_id) \
    if (p_usw_ipmc_master[lchip]->bidipim_mode == SYS_IPMC_BIDIPIM_MODE0)\
    {\
        CTC_MAX_VALUE_CHECK(rp_id, 15);\
    }\
    else if (p_usw_ipmc_master[lchip]->bidipim_mode == SYS_IPMC_BIDIPIM_MODE1)\
    {\
        CTC_MAX_VALUE_CHECK(rp_id, 31);\
    }\
    else if (p_usw_ipmc_master[lchip]->bidipim_mode == SYS_IPMC_BIDIPIM_MODE2)\
    {\
        CTC_MAX_VALUE_CHECK(rp_id, 63);\
    }\
    else if (p_usw_ipmc_master[lchip]->bidipim_mode == SYS_IPMC_BIDIPIM_MODE3)\
    {\
        CTC_MAX_VALUE_CHECK(rp_id, 127);\
    }

/*g_pointer:v4:15 bit; v6:12bit*/
/*v4:15 bit; v6:12bit, allocation the pointer by hash table and the opf only used to resovle hash confilct*/
#define SYS_USW_IPMC_V4_G_POINTER_SIZE 32768
#define SYS_USW_IPMC_V6_G_POINTER_SIZE 4096
#define SYS_USW_IPMC_L2_V6_G_POINTER_SIZE 1024
#define SYS_USW_IPMC_V4_G_POINTER_HASH_SIZE 30720
#define SYS_USW_IPMC_V6_G_POINTER_HASH_SIZE 3328
#define SYS_USW_IPMC_L2_V6_G_POINTER_HASH_SIZE 768
#define SYS_USW_IPMC_V4_G_POINTER_OPF_SIZE  (SYS_USW_IPMC_V4_G_POINTER_SIZE - SYS_USW_IPMC_V4_G_POINTER_HASH_SIZE)
#define SYS_USW_IPMC_V6_G_POINTER_OPF_SIZE (SYS_USW_IPMC_V6_G_POINTER_SIZE - SYS_USW_IPMC_V6_G_POINTER_HASH_SIZE)
#define SYS_USW_IPMC_L2_V6_G_POINTER_OPF_SIZE (SYS_USW_IPMC_L2_V6_G_POINTER_SIZE - SYS_USW_IPMC_L2_V6_G_POINTER_HASH_SIZE)
#define SYS_USW_IPMC_VECTOR_BLOCK_SIZE 256

enum sys_ipmc_type_e
{
    SYS_IPMC_V4,           /**< IPMC vrf id is config by l3if */
    SYS_IPMC_V4_L2MC,           /**< IPMC vrf id is config by l3if */
    SYS_IPMC_V6 ,           /**< IPMC vrf id is config by l3if */
    SYS_IPMC_V6_L2MC,          /**< IPMC vrf id is svlan id */
    SYS_IPMC_TYPE_MAX
};
typedef enum sys_ipmc_type_e sys_ipmc_type_t;

/**
 @brief Ipmc bidipim mode
*/
enum sys_ipmc_bidipim_mode_e
{
    SYS_IPMC_BIDIPIM_MODE0 = 0,           /**< IPMC bidipim (1k interface * 16 rp group) */
    SYS_IPMC_BIDIPIM_MODE1 = 1,           /**< IPMC bidipim (512 interface * 32 rp group) */
    SYS_IPMC_BIDIPIM_MODE2 = 2,           /**< IPMC bidipim (256 interface * 64 rp group) */
    SYS_IPMC_BIDIPIM_MODE3 = 3            /**< IPMC bidipim (128 interface * 128 rp group) */
};
typedef enum sys_ipmc_bidipim_mode_e sys_ipmc_bidipim_mode_t;

/**
 @brief Ipmc mode
*/
enum sys_ipmc_vrf_mode_e
{
    SYS_IPMC_VRF_MODE_VRF = 0,           /**< IPMC vrf id is config by l3if */
    SYS_IPMC_VRF_MODE_SVLAN = 1          /**< IPMC vrf id is svlan id */
};
typedef enum sys_ipmc_vrf_mode_e sys_ipmc_vrf_mode_t;

struct sys_ipmc_g_node_s
{
    uint32  ref_cnt:16;
    uint32  ipmc_type:2;
    uint32  is_group:1;  /*indictae the (*,g) is valid mcast group and have mcast member*/
    uint32  rsv:13;
    uint16  pointer;   /*ipv4 :15 bit,ipv6 :10bit*/
    uint16  vrfid;

    union
    {
        ip_addr_t  ipv4;
        ipv6_addr_t ipv6;
    } ip_addr;
};
typedef struct sys_ipmc_g_node_s sys_ipmc_g_node_t;

struct sys_ipmc_group_node_s
{
    ctc_list_pointer_node_t list_head;
    uint16  ad_index;
    uint8   src_ip_mask_len;
    uint8   group_ip_mask_len;
    uint32  is_l2mc:1;
    uint32  ip_version:1;
    uint32  share_grp:1;
    uint32  is_drop:1;
    uint32  with_nexthop:1;
    uint32  is_rd_cpu:1;
    uint32  rsv:26;

    union
    {
        ctc_ipmc_ipv4_addr_t ipv4;
        ctc_ipmc_ipv6_addr_t ipv6;
    } address;
};
typedef struct sys_ipmc_group_node_s sys_ipmc_group_node_t;

struct sys_ipmc_group_param_s
{
    uint8  ipmc_type;
    uint32 nh_id;
    uint16 rpf_id;
    uint8  rpf_chk_en;
    uint8  rp_chk_en;
    uint8  rpf_mode; /*0:rpf profile;1:rpf ifid*/
    uint16 hw_grp_id;

    uint8   l2mc_mode;
    uint8   is_conflict;
    uint8   is_g_pointer_key;
    uint8   is_group_exist;
    uint8   is_key_exist;
    uint8   is_default_route;
    uint8   is_drop;
    uint8   is_rd_cpu;

    uint32  key_index;
    uint32  ad_index;
    uint32  tbl_id;
    uint32  g_key_index;
    uint32  g_key_tbl_id;

    ds_t    key;
    sys_ipmc_g_node_t g_node;
    ctc_ipmc_group_info_t *group;
};
typedef struct sys_ipmc_group_param_s sys_ipmc_group_param_t;

struct sys_ipmc_master_s
{
    sal_mutex_t* p_ipmc_mutex;
    uint8 bidipim_mode;
    uint32 default_base[SYS_IPMC_TYPE_MAX];
    uint32 ipmc_group_cnt[SYS_IPMC_TYPE_MAX];
    uint32 ipmc_group_sg_cnt[SYS_IPMC_TYPE_MAX];
    uint8  opf_g_pointer[SYS_IPMC_TYPE_MAX];
    ctc_vector_t *group_vec;
    ctc_hash_t* g_hash[SYS_IPMC_TYPE_MAX];    /*sys_ipmc_g_node_t,store (*,g) mcast group*/
};
typedef struct sys_ipmc_master_s sys_ipmc_master_t;

extern int32
sys_usw_ipmc_init(uint8 lchip);

extern int32
_sys_usw_ipmc_check(uint8 lchip, ctc_ipmc_group_info_t* p_group);

extern int32
sys_usw_ipmc_add_group(uint8 lchip, ctc_ipmc_group_info_t* p_group);

extern int32
sys_usw_ipmc_remove_group(uint8 lchip, ctc_ipmc_group_info_t* p_group);

extern int32
sys_usw_ipmc_add_member(uint8 lchip, ctc_ipmc_group_info_t* p_group);

extern int32
sys_usw_ipmc_remove_member(uint8 lchip, ctc_ipmc_group_info_t* p_group);

extern int32
sys_usw_ipmc_add_rp_intf(uint8 lchip, ctc_ipmc_rp_t * p_rp);

extern int32
sys_usw_ipmc_remove_rp_intf(uint8 lchip, ctc_ipmc_rp_t * p_rp);

extern int32
sys_usw_ipmc_update_rpf(uint8 lchip, ctc_ipmc_group_info_t* p_group);

extern int32
sys_usw_ipmc_get_group_info(uint8 lchip, ctc_ipmc_group_info_t* p_group);

extern int32
sys_usw_ipmc_add_default_entry(uint8 lchip, uint8 ip_version, ctc_ipmc_default_action_t type);

extern int32
_sys_usw_ipmc_write_key(uint8 lchip,  sys_ipmc_group_param_t* p_group_node,uint8 is_add);

extern int32
sys_usw_ipmc_set_mcast_force_route(uint8 lchip, ctc_ipmc_force_route_t* p_data);

extern int32
sys_usw_ipmc_get_mcast_force_route(uint8 lchip, ctc_ipmc_force_route_t* p_data);

extern int32
sys_usw_ipmc_set_entry_hit(uint8 lchip, ctc_ipmc_group_info_t* p_group, uint8 hit);

extern int32
sys_usw_ipmc_get_entry_hit(uint8 lchip, ctc_ipmc_group_info_t* p_group, uint8* hit);

extern int32
sys_usw_ipmc_deinit(uint8 lchip);
#ifdef __cplusplus
}
#endif

#endif

#endif
