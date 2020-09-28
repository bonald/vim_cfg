#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))

/**
 @file sys_usw_ipuc.c

 @date 2011-11-30

 @version v2.0

 The file contains all ipuc related function
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/

#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_ipuc.h"
#include "ctc_parser.h"
#include "ctc_debug.h"
#include "ctc_packet.h"
#include "ctc_pdu.h"

#include "sys_usw_chip.h"
#include "sys_usw_opf.h"
#include "sys_usw_ofb.h"
#include "sys_usw_ftm.h"
#include "sys_usw_rpf_spool.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_ipuc.h"
#include "sys_usw_ip_tunnel.h"
#include "sys_duet2_calpm.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_l3if.h"
#include "sys_usw_stats_api.h"
#include "sys_usw_common.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_learning_aging.h"
#include "sys_usw_register.h"
#include "sys_usw_wb_common.h"
#include "sys_duet2_ipuc_tcam.h"
#include "tsingma/sys_tsingma_nalpm.h"

#include "usw/include/drv_ftm.h"
#include "usw/include/drv_common.h"
#include "ctc_avl_tree.h"
/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
typedef uint32 (* updateIpDa_fn ) (uint8 lchip, void* data, void* change_nh_param);

extern sys_ip_tunnel_master_t* p_usw_ip_tunnel_master[];

sal_timer_t* sys_usw_aid_timer = NULL;

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
sys_ipuc_master_t* p_usw_ipuc_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#define SYS_IPUC_LOCK \
    if(p_usw_ipuc_master[lchip]->mutex) sal_mutex_lock(p_usw_ipuc_master[lchip]->mutex)

#define SYS_IPUC_UNLOCK \
    if(p_usw_ipuc_master[lchip]->mutex) sal_mutex_unlock(p_usw_ipuc_master[lchip]->mutex)

#define SYS_LIST_LOCK \
    if(p_usw_ipuc_master[lchip]->lpm_mutex) sal_mutex_lock(p_usw_ipuc_master[lchip]->lpm_mutex)

#define SYS_LIST_UNLOCK \
    if(p_usw_ipuc_master[lchip]->lpm_mutex) sal_mutex_unlock(p_usw_ipuc_master[lchip]->lpm_mutex)

#define CTC_ERROR_RETURN_IPUC_UNLOCK(op) \
    do \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            sal_mutex_unlock(p_usw_ipuc_master[lchip]->mutex); \
            return (rv); \
        } \
    } while (0)

#define CTC_RETURN_IPUC_UNLOCK(op) \
    do \
    { \
        sal_mutex_unlock(p_usw_ipuc_master[lchip]->mutex); \
        return (op); \
    } while (0)

#define SYS_IP_CHECK_VERSION_ENABLE(ver)                        \
    {                                                               \
        if ((!p_usw_ipuc_master[lchip]) || !p_usw_ipuc_master[lchip]->version_en[ver])     \
        {                                                           \
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [IPUC] SDK is not assigned resource for this version \n");\
			return CTC_E_NOT_READY;\
        }                                                           \
    }

#define ROT(x, k) (((x) << (k)) | ((x) >> (32 - (k))))

#define MIX(a, b, c) \
    do \
    { \
        a -= c;  a ^= ROT(c, 4);  c += b; \
        b -= a;  b ^= ROT(a, 6);  a += c; \
        c -= b;  c ^= ROT(b, 8);  b += a; \
        a -= c;  a ^= ROT(c, 16);  c += b; \
        b -= a;  b ^= ROT(a, 19);  a += c; \
        c -= b;  c ^= ROT(b, 4);  b += a; \
    } while (0)

#define FINAL(a, b, c) \
    { \
        c ^= b; c -= ROT(b, 14); \
        a ^= c; a -= ROT(c, 11); \
        b ^= a; b -= ROT(a, 25); \
        c ^= b; c -= ROT(b, 16); \
        a ^= c; a -= ROT(c, 4);  \
        b ^= a; b -= ROT(a, 14); \
        c ^= b; c -= ROT(b, 24); \
    }

#define WAIT_LOOP 2000
#define VRF_MAP_DEST_MAP_BASE 100
/****************************************************************************
 *
* Function
*
*****************************************************************************/

extern int32
sys_usw_ipuc_update_ipda(uint8 lchip, void* data, void* change_nh_param);

extern int32
sys_tsingma_nalpm_wb_prefix_sync(uint8 lchip);

extern int32
sys_tsingma_nalpm_wb_prefix_restore(uint8 lchip);

void
sys_usw_ipuc_update_route_stats(uint8 lchip, uint8 ip_ver, sys_ipuc_route_stats_t type, uint8 opt);

#define __TCAM_API___

int32
sys_usw_ipuc_alloc_tcam_key_index(uint8 lchip, sys_ipuc_tcam_data_t *p_data)
{
    uint8 ip_ver = 0;
    uint8 block_id = 0;
    uint32 key_index = 0;
    int32 ret = CTC_E_NONE;
    sys_ipuc_ofb_cb_t *p_ofb_cb = NULL;
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_PTR_VALID_CHECK(p_data->info);
    CTC_PTR_VALID_CHECK(p_data->ipuc_param);
    CTC_PTR_VALID_CHECK(MCHIP_API(lchip)->ipuc_tcam_get_blockid);

    ip_ver = p_data->ipuc_param->ip_ver;
    route_mode = CTC_FLAG_ISSET(p_data->ipuc_param->route_flag, CTC_IPUC_FLAG_PUBLIC_ROUTE)? SYS_PUBLIC_MODE: SYS_PRIVATE_MODE;

    CTC_ERROR_RETURN(MCHIP_API(lchip)->ipuc_tcam_get_blockid(lchip, p_data, &block_id));

    p_ofb_cb = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_ipuc_ofb_cb_t));
    if (NULL == p_ofb_cb)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_ofb_cb, 0, sizeof(sys_ipuc_ofb_cb_t));

    p_ofb_cb->type = p_data->key_type;
    p_ofb_cb->ip_ver = ip_ver;
    p_ofb_cb->route_mode = route_mode;
    p_ofb_cb->masklen = p_data->ipuc_param->masklen;
    p_ofb_cb->user_data = p_data->info;

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        key_index = p_data->key_index;
        ret = sys_usw_ofb_alloc_offset_from_position(lchip, p_usw_ipuc_master[lchip]->share_ofb_type[route_mode], block_id, key_index, (void*)p_ofb_cb);
        if (ret)
        {
            mem_free(p_ofb_cb);
            return ret;
        }
    }
    else
    {
        ret = sys_usw_ofb_alloc_offset(lchip, p_usw_ipuc_master[lchip]->share_ofb_type[route_mode], block_id, &key_index, (void*)p_ofb_cb);
        if (ret)
        {
            mem_free(p_ofb_cb);
            return ret;
        }
        p_data->key_index = key_index;

        if (route_mode == SYS_PRIVATE_MODE)
        {
            if ((ip_ver == CTC_IP_VER_6) && (IS_SHORT_KEY(route_mode, key_index)))
            {
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_TCAM_SHORT, 0);
            }
            else
            {
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_TCAM, 0);
            }
        }
        else
        {
            if ((ip_ver == CTC_IP_VER_6) && (IS_SHORT_KEY(route_mode, key_index)))
            {
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_TCAM_PUB_SHORT, 0);
            }
            else
            {
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_TCAM_PUB, 0);
            }
        }
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: alloc %s %s %s tcam key index = %d, block_id = %d, ofb_type = %d\n", (ip_ver == CTC_IP_VER_4) ? "IPv4" : "IPv6",
                                    (route_mode == SYS_PRIVATE_MODE) ? "private" : "public", ((ip_ver == CTC_IP_VER_6) && (IS_SHORT_KEY(route_mode, key_index))) ? "ShortKey" : "FullKey",
                                    key_index, block_id, p_usw_ipuc_master[lchip]->share_ofb_type[route_mode]);

    return CTC_E_NONE;
}

int32
sys_usw_ipuc_free_tcam_key_index(uint8 lchip, sys_ipuc_tcam_data_t *p_data)
{
    uint8 ip_ver = 0;
    uint8 block_id = 0;
    uint32 key_index = 0;
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_PTR_VALID_CHECK(p_data->info);
    CTC_PTR_VALID_CHECK(p_data->ipuc_param);
    CTC_PTR_VALID_CHECK(MCHIP_API(lchip)->ipuc_tcam_get_blockid);

    key_index = p_data->key_index;
    ip_ver = p_data->ipuc_param->ip_ver;
    route_mode = CTC_FLAG_ISSET(p_data->ipuc_param->route_flag, CTC_IPUC_FLAG_PUBLIC_ROUTE);

    CTC_ERROR_RETURN(MCHIP_API(lchip)->ipuc_tcam_get_blockid(lchip, p_data, &block_id));

    CTC_ERROR_RETURN(sys_usw_ofb_free_offset(lchip, p_usw_ipuc_master[lchip]->share_ofb_type[route_mode], block_id, key_index));
    /*mem_free(p_ofb_cb) in sys_usw_ofb_free_offset() */

    if(MCHIP_API(lchip)->ipuc_alpm_set_fragment_status)
    {
        MCHIP_API(lchip)->ipuc_alpm_set_fragment_status(lchip, ip_ver, MERGE_OK);
    }

    if (route_mode == SYS_PRIVATE_MODE)
    {
        if ((ip_ver == CTC_IP_VER_6) && (IS_SHORT_KEY(route_mode, key_index)))
        {
            sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_TCAM_SHORT, 1);
        }
        else
        {
            sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_TCAM, 1);
        }
    }
    else
    {
        if ((ip_ver == CTC_IP_VER_6) && (IS_SHORT_KEY(route_mode, key_index)))
        {
            sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_TCAM_PUB_SHORT, 1);
        }
        else
        {
            sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_TCAM_PUB, 1);
        }
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: free %s %s %s tcam key index = %d, block_id = %d, ofb_type = %d\n", (ip_ver == CTC_IP_VER_4) ? "IPv4" : "IPv6",
                                    (route_mode == SYS_PRIVATE_MODE) ? "private" : "public", ((ip_ver == CTC_IP_VER_6) && (IS_SHORT_KEY(route_mode, key_index))) ? "ShortKey" : "FullKey",
                                    key_index, block_id, p_usw_ipuc_master[lchip]->share_ofb_type[route_mode]);

    return CTC_E_NONE;
}

int32
sys_usw_ipuc_tcam_move_cb(
                            uint8 lchip,
                            uint32 new_index,
                            uint32 old_index,
                            void *pdata)
{
    uint8 ip_ver = CTC_IP_VER_4;
    sys_ipuc_ofb_cb_t *p_ofb_cb = NULL;
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (new_index == old_index)
    {
        return CTC_E_NONE;
    }

    p_ofb_cb = (sys_ipuc_ofb_cb_t*)pdata;
    if (NULL == p_ofb_cb)
    {
        return CTC_E_NONE;
    }
    CTC_PTR_VALID_CHECK(p_ofb_cb->user_data);
    CTC_PTR_VALID_CHECK(MCHIP_API(lchip)->ipuc_tcam_move);

    CTC_ERROR_RETURN(MCHIP_API(lchip)->ipuc_tcam_move(lchip, new_index, old_index, p_ofb_cb));

    ip_ver = p_ofb_cb->ip_ver;
    route_mode = p_ofb_cb->route_mode;

    if (ip_ver == CTC_IP_VER_6)
    {
        if (route_mode == SYS_PRIVATE_MODE)
        {
            if (IS_SHORT_KEY(route_mode, old_index) && !IS_SHORT_KEY(route_mode, new_index))
            {
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_TCAM_SHORT, 1);
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_TCAM, 0);
            }
            else if (!IS_SHORT_KEY(route_mode, old_index) && IS_SHORT_KEY(route_mode, new_index))
            {
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_TCAM_SHORT, 0);
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_TCAM, 1);
            }
        }
        else
        {
            if (IS_SHORT_KEY(route_mode, old_index) && !IS_SHORT_KEY(route_mode, new_index))
            {
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_TCAM_PUB_SHORT, 1);
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_TCAM_PUB, 0);
            }
            else if (!IS_SHORT_KEY(route_mode, old_index) && IS_SHORT_KEY(route_mode, new_index))
            {
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_TCAM_PUB_SHORT, 0);
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_TCAM_PUB, 1);
            }
        }
    }

    return CTC_E_NONE;
}

int32 _sys_usw_ipuc_bind_nexthop(uint8 lchip, sys_ipuc_param_t* p_sys_param, uint8 is_bind);

int32
sys_usw_ipuc_add_tcam(uint8 lchip, sys_ipuc_param_t *p_sys_param)
{
    int32 ret = CTC_E_NONE;
    uint8 real_lchip = 0;
    uint16 real_index = 0;
    sys_ipuc_tcam_data_t tcam_data = {0};
    sys_ipuc_info_t temp_info = {0};

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(MCHIP_API(lchip)->ipuc_tcam_write_key);
    CTC_PTR_VALID_CHECK(MCHIP_API(lchip)->ipuc_tcam_write_ad);

    tcam_data.opt_type = DO_ADD;
    tcam_data.key_type = SYS_IPUC_TCAM_FLAG_TCAM;
    tcam_data.ipuc_param = &p_sys_param->param;
    tcam_data.info = p_sys_param->info;

    ret = sys_usw_ipuc_alloc_tcam_key_index(lchip, &tcam_data);

    if (p_usw_ipuc_master[lchip]->rchain_en)
    {
        sal_memcpy(&temp_info, p_sys_param->info, sizeof(sys_ipuc_info_t));
        tcam_data.info = &temp_info;
    }

    if (ret)
    {
        _sys_usw_ipuc_bind_nexthop(0, p_sys_param, 0);
        return ret;
    }
    if (MCHIP_API(lchip)->ipuc_alpm_get_real_tcam_index && p_usw_ipuc_master[lchip]->rchain_en)
    {
        MCHIP_API(lchip)->ipuc_alpm_get_real_tcam_index(lchip, tcam_data.key_index, &real_lchip, &real_index);
        p_sys_param->info->key_index = tcam_data.key_index;
        p_sys_param->info->real_lchip = real_lchip;
        ((sys_ipuc_info_t*)tcam_data.info)->key_index = real_index;
    }
    else
    {
        p_sys_param->info->key_index = tcam_data.key_index;
        p_sys_param->info->real_lchip = lchip;
        real_lchip = lchip;
    }
    if (p_usw_ipuc_master[lchip]->rchain_en)
    {
        uint32 cmd = 0;
        if (real_lchip && !GetDsIpDa(V, nextHopPtrValid_f, p_sys_param->p_dsipda))
        {
            sys_nh_info_dsnh_t nh_info;
            sys_usw_nh_get_nhinfo(real_lchip, p_sys_param->info->nh_id, &nh_info, 0);
            SetDsIpDa(V, dsFwdPtrOrEcmpGroupId_f, p_sys_param->p_dsipda, nh_info.dsfwd_offset);
        }
        ret = _sys_usw_ipuc_add_ad_profile(real_lchip, p_sys_param, p_sys_param->p_dsipda, 0);
        temp_info.ad_index = p_sys_param->info->ad_index;
        if (ret)
        {
            _sys_usw_ipuc_bind_nexthop(0, p_sys_param, 0);
            return ret;
        }
        cmd = DRV_IOW(DsIpDa_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(real_lchip, p_sys_param->info->ad_index, cmd, p_sys_param->p_dsipda);
    }

    CTC_ERROR_GOTO(MCHIP_API(lchip)->ipuc_tcam_write_ad(real_lchip, &tcam_data), ret, error0);

    /* write ipuc tcam key entry */
    CTC_ERROR_GOTO(MCHIP_API(lchip)->ipuc_tcam_write_key(real_lchip, &tcam_data), ret, error1);

    if (p_usw_ipuc_master[lchip]->rchain_en)
    {
        *(p_usw_ipuc_master[0]->rchain_stats + real_lchip) += 1;
    }

    return CTC_E_NONE;

error1:
    tcam_data.opt_type = DO_DEL;
    MCHIP_API(lchip)->ipuc_tcam_write_ad(real_lchip, &tcam_data);

error0:
    sys_usw_ipuc_free_tcam_key_index(real_lchip, &tcam_data);

    return ret;
}

int32
sys_usw_ipuc_del_tcam(uint8 lchip, sys_ipuc_param_t *p_sys_param)
{
    sys_ipuc_tcam_data_t tcam_data = {0};
    uint16 real_index = 0;
    sys_ipuc_info_t temp_info = {0};

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(MCHIP_API(lchip)->ipuc_tcam_write_key);
    CTC_PTR_VALID_CHECK(MCHIP_API(lchip)->ipuc_tcam_write_ad);

    tcam_data.opt_type = DO_DEL;
    tcam_data.key_type = SYS_IPUC_TCAM_FLAG_TCAM;
    tcam_data.key_index = p_sys_param->info->key_index;
    tcam_data.ipuc_param = &p_sys_param->param;
    tcam_data.info = p_sys_param->info;
    if (p_usw_ipuc_master[lchip]->rchain_en)
    {
        sal_memcpy(&temp_info, p_sys_param->info, sizeof(sys_ipuc_info_t));
        tcam_data.info = &temp_info;
        MCHIP_API(lchip)->ipuc_alpm_get_real_tcam_index(lchip, ((sys_ipuc_info_t*)tcam_data.info)->key_index, &lchip, &real_index);
        ((sys_ipuc_info_t*)tcam_data.info)->key_index = real_index;
    }

    CTC_ERROR_RETURN(MCHIP_API(lchip)->ipuc_tcam_write_key(lchip, &tcam_data));

    /* TCAM do not need to build remove offset, only need to clear soft offset info */
    CTC_ERROR_RETURN(sys_usw_ipuc_free_tcam_key_index(p_usw_ipuc_master[lchip]->rchain_en?0:lchip, &tcam_data));

    CTC_ERROR_RETURN(MCHIP_API(lchip)->ipuc_tcam_write_ad(lchip, &tcam_data));

    if (p_usw_ipuc_master[lchip]->rchain_en)
    {
        *(p_usw_ipuc_master[0]->rchain_stats + p_sys_param->info->real_lchip) -= 1;
    }

    return CTC_E_NONE;
}

int32
sys_usw_ipuc_update_tcam(uint8 lchip, sys_ipuc_param_t *p_sys_param)
{
    sys_ipuc_tcam_data_t tcam_data = {0};
    uint16 real_index = 0;
    sys_ipuc_info_t temp_info = {0};

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(MCHIP_API(lchip)->ipuc_tcam_write_key);
    CTC_PTR_VALID_CHECK(MCHIP_API(lchip)->ipuc_tcam_write_ad);

    tcam_data.opt_type = DO_UPDATE;
    tcam_data.key_type = SYS_IPUC_TCAM_FLAG_TCAM;
    tcam_data.ipuc_param = &p_sys_param->param;
    tcam_data.info = p_sys_param->info;
    if (p_usw_ipuc_master[lchip]->rchain_en)
    {
        sal_memcpy(&temp_info, p_sys_param->info, sizeof(sys_ipuc_info_t));
        tcam_data.info = &temp_info;
        MCHIP_API(lchip)->ipuc_alpm_get_real_tcam_index(lchip, ((sys_ipuc_info_t*)tcam_data.info)->key_index, &lchip, &real_index);
        ((sys_ipuc_info_t*)tcam_data.info)->key_index = real_index;
    }

    CTC_ERROR_RETURN(MCHIP_API(lchip)->ipuc_tcam_write_ad(lchip, &tcam_data));

    return CTC_E_NONE;
}

#define __IPUC_COMMON___


/**
 @brief function of make hash key for ipuc table

 @param[in] p_ipuc_info, information should be maintained for ipuc

 @return CTC_E_XXX
 */
STATIC uint32
_sys_usw_ipv4_hash_make(sys_ipuc_info_t* p_ipuc_info)
{
    uint32 a, b, c;

    /* Set up the internal state */
    a = b = c = 0xdeadbeef + (((uint32)96) << 2);

    a += p_ipuc_info->ip.ipv4;
    b += p_ipuc_info->vrf_id;
    b += (p_ipuc_info->masklen << 16);
    b += p_ipuc_info->l4_dst_port;
    MIX(a, b, c);

    return (c & IPUC_IPV4_HASH_MASK);
}

STATIC uint32
_sys_usw_aid_hash_make(sys_ipuc_pending_route_t* p_ipuc_info)
{
    sys_ipuc_aid_key_t key;
    sal_memset(key.ip.ipv6, 0, sizeof(ipv6_addr_t));
    key.ip_ver = p_ipuc_info->sys_param.param.ip_ver;
    key.vrf_id = p_ipuc_info->sys_param.param.vrf_id;
    key.masklen = p_ipuc_info->sys_param.param.masklen;
    sal_memcpy(key.ip.ipv6, p_ipuc_info->sys_param.param.ip.ipv6 ,sizeof(ipv6_addr_t));

    return ctc_hash_caculate( sizeof(sys_ipuc_aid_key_t), &key);
}


STATIC uint32
_sys_usw_aid_hash_cmp(sys_ipuc_pending_route_t* p_ipuc_info1, sys_ipuc_pending_route_t* p_ipuc_info)
{

    if (p_ipuc_info->sys_param.param.ip_ver != p_ipuc_info1->sys_param.param.ip_ver)
    {
        return FALSE;
    }
    if (p_ipuc_info->sys_param.param.vrf_id != p_ipuc_info1->sys_param.param.vrf_id)
    {
        return FALSE;
    }

    if (p_ipuc_info->sys_param.param.masklen != p_ipuc_info1->sys_param.param.masklen)
    {
        return FALSE;
    }

    if (sal_memcmp(p_ipuc_info->sys_param.param.ip.ipv6, p_ipuc_info1->sys_param.param.ip.ipv6, sizeof(ipv6_addr_t)))
    {
        return FALSE;
    }

    return TRUE;
}


STATIC uint32
_sys_usw_ipv6_hash_make(sys_ipuc_info_t* p_ipuc_info)
{
    uint32 a, b, c;

    /* Set up the internal state */
    a = b = c = 0xdeadbeef + (((uint32)192) << 2);

    a += p_ipuc_info->ip.ipv6[0];
    b += p_ipuc_info->ip.ipv6[1];
    c += p_ipuc_info->ip.ipv6[2];
    MIX(a, b, c);

    a += p_ipuc_info->ip.ipv6[3];
    b += p_ipuc_info->vrf_id;
    b += (p_ipuc_info->masklen << 16);
    MIX(a, b, c);

    FINAL(a, b, c);

    return (c & IPUC_IPV6_HASH_MASK);
}

STATIC bool
_sys_usw_ipv4_hash_cmp(sys_ipuc_info_t* p_ipuc_info1, sys_ipuc_info_t* p_ipuc_info)
{
    if (p_ipuc_info1->vrf_id != p_ipuc_info->vrf_id)
    {
        return FALSE;
    }

    if (p_ipuc_info1->masklen != p_ipuc_info->masklen)
    {
        return FALSE;
    }

    if (p_ipuc_info1->ip.ipv4 != p_ipuc_info->ip.ipv4)
    {
        return FALSE;
    }

    if (p_ipuc_info1->l4_dst_port != p_ipuc_info->l4_dst_port)
    {
        return FALSE;
    }

    if ((p_ipuc_info1->route_flag & SYS_IPUC_FLAG_IS_TCP_PORT)
        != (p_ipuc_info->route_flag & SYS_IPUC_FLAG_IS_TCP_PORT))
    {
        return FALSE;
    }

    if ((p_ipuc_info1->route_flag & SYS_IPUC_FLAG_HOST_USE_LPM)
        != (p_ipuc_info->route_flag & SYS_IPUC_FLAG_HOST_USE_LPM))
    {
        return FALSE;
    }

    if ((p_ipuc_info1->route_flag & SYS_IPUC_FLAG_PUBLIC_ROUTE)
        != (p_ipuc_info->route_flag & SYS_IPUC_FLAG_PUBLIC_ROUTE))
    {
        return FALSE;
    }

    return TRUE;
}

STATIC bool
_sys_usw_ipv6_hash_cmp(sys_ipuc_info_t* p_ipuc_info1, sys_ipuc_info_t* p_ipuc_info)
{
    if (p_ipuc_info1->vrf_id != p_ipuc_info->vrf_id)
    {
        return FALSE;
    }

    if (p_ipuc_info1->masklen != p_ipuc_info->masklen)
    {
        return FALSE;
    }

    if (p_ipuc_info1->ip.ipv6[3] != p_ipuc_info->ip.ipv6[3])
    {
        return FALSE;
    }

    if (p_ipuc_info1->ip.ipv6[2] != p_ipuc_info->ip.ipv6[2])
    {
        return FALSE;
    }

    if (p_ipuc_info1->ip.ipv6[1] != p_ipuc_info->ip.ipv6[1])
    {
        return FALSE;
    }

    if (p_ipuc_info1->ip.ipv6[0] != p_ipuc_info->ip.ipv6[0])
    {
        return FALSE;
    }

    if ((p_ipuc_info1->route_flag & SYS_IPUC_FLAG_HOST_USE_LPM)
        != (p_ipuc_info->route_flag & SYS_IPUC_FLAG_HOST_USE_LPM))
    {
        return FALSE;
    }

    if ((p_ipuc_info1->route_flag & SYS_IPUC_FLAG_PUBLIC_ROUTE)
        != (p_ipuc_info->route_flag & SYS_IPUC_FLAG_PUBLIC_ROUTE))
    {
        return FALSE;
    }

    return TRUE;
}

STATIC uint32
_sys_usw_ipuc_ad_profile_make(sys_ipuc_ad_spool_t* p_ipuc_ad)
{
    uint32 a, b, c;
    uint32* k = (uint32*)p_ipuc_ad;
    uint8*  k8;
    uint8   length = sizeof(sys_ipuc_ad_spool_t) - sizeof(uint32);

    /* Set up the internal state */
    a = b = c = 0xdeadbeef;

    /*------ all but last block: aligned reads and affect 32 bits of (a,b,c) */
    while (length > 12)
    {
        a += k[0];
        b += k[1];
        c += k[2];
        MIX(a, b, c);
        length -= 12;
        k += 3;
    }

    k8 = (uint8*)k;

    switch (length)
    {
    case 12:
        c += k[2];
        b += k[1];
        a += k[0];
        break;

    case 11:
        c += ((uint8)k8[10]) << 16;       /* fall through */

    case 10:
        c += ((uint8)k8[9]) << 8;         /* fall through */

    case 9:
        c += k8[8];                          /* fall through */

    case 8:
        b += k[1];
        a += k[0];
        break;

    case 7:
        b += ((uint8)k8[6]) << 16;        /* fall through */

    case 6:
        b += ((uint8)k8[5]) << 8;         /* fall through */

    case 5:
        b += k8[4];                          /* fall through */

    case 4:
        a += k[0];
        break;

    case 3:
        a += ((uint8)k8[2]) << 16;        /* fall through */

    case 2:
        a += ((uint8)k8[1]) << 8;         /* fall through */

    case 1:
        a += k8[0];
        break;

    case 0:
        return c;
    }

    FINAL(a, b, c);
    return c;
}

STATIC bool
_sys_usw_ipuc_ad_profile_cmp(sys_ipuc_ad_spool_t* p_bucket, sys_ipuc_ad_spool_t* p_new)
{
    if (!p_bucket || !p_new)
    {
        return FALSE;
    }

    if (sal_memcmp (&p_bucket->ad, &p_new->ad, sizeof(DsIpDa_m)))
    {
        return FALSE;
    }

    return TRUE;
}

STATIC int32
_sys_usw_ipuc_ad_profile_build_index(sys_ipuc_ad_spool_t* p_bucket, uint8* p_lchip)
{
    uint32 value_32 = 0;

    if (!(CTC_WB_ENABLE && (CTC_WB_STATUS(*p_lchip) == CTC_WB_STATUS_RELOADING)))
    {
        CTC_ERROR_RETURN(sys_usw_ftm_alloc_table_offset(*p_lchip, DsIpDa_t, 0, 1, 1, &value_32));
        p_bucket->ad_index = value_32 & 0xFFFF;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipuc_ad_profile_free_index(sys_ipuc_ad_spool_t* p_bucket, uint8* p_lchip)
{
    CTC_ERROR_RETURN(sys_usw_ftm_free_table_offset(*p_lchip, DsIpDa_t, 0, 1, p_bucket->ad_index));

    return CTC_E_NONE;
}

int32
_sys_usw_ipuc_add_ad_profile(uint8 lchip, sys_ipuc_param_t* p_sys_param, void* ad_profile, uint8 update)
{
    uint32 cmd = 0;
    sys_ipuc_ad_spool_t ipuc_ad_new;
    sys_ipuc_ad_spool_t ipuc_ad_temp;
    sys_ipuc_ad_spool_t* p_ipuc_ad_old = NULL;
    sys_ipuc_ad_spool_t* p_ipuc_ad_get = NULL;

    CTC_PTR_VALID_CHECK(ad_profile);

    sal_memset(&ipuc_ad_new, 0, sizeof(sys_ipuc_ad_spool_t));
    sal_memcpy(&ipuc_ad_new.ad, ad_profile, sizeof(DsIpDa_m));

    if (update)
    {
        sal_memset(&ipuc_ad_temp, 0, sizeof(sys_ipuc_ad_spool_t));
        p_ipuc_ad_old = &ipuc_ad_temp;

        cmd = DRV_IOR(DsIpDa_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, p_sys_param->info->ad_index, cmd, &p_ipuc_ad_old->ad);
    }

    CTC_ERROR_RETURN(ctc_spool_add(p_usw_ipuc_master[lchip]->ad_spool, &ipuc_ad_new, p_ipuc_ad_old, &p_ipuc_ad_get));
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC: alloc ad_index = %d\n", p_ipuc_ad_get->ad_index);

    p_sys_param->info->ad_index = p_ipuc_ad_get->ad_index;

    return CTC_E_NONE;
}

int32
_sys_usw_ipuc_remove_ad_profile(uint8 lchip, uint32 ad_index)
{
    uint32 cmd = 0;
    sys_ipuc_ad_spool_t ipuc_ad;

    sal_memset(&ipuc_ad, 0, sizeof(sys_ipuc_ad_spool_t));

    ipuc_ad.ad_index = ad_index;

    cmd = DRV_IOR(DsIpDa_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, ipuc_ad.ad_index, cmd, &ipuc_ad.ad);

    CTC_ERROR_RETURN(ctc_spool_remove(p_usw_ipuc_master[lchip]->ad_spool, &ipuc_ad, NULL));
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC: remove ad_index = %d\n", ad_index);

    return CTC_E_NONE;
}

STATIC void
_sys_usw_ipuc_param_to_info(uint8 lchip, ctc_ipuc_param_t *p_ctc_param, sys_ipuc_info_t *p_ipuc_info)
{
    uint32 route_flag = 0;
    uint8 addr_size = 0;

    if ( CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_MERGE_KEY))
    {
        CTC_SET_FLAG(route_flag, SYS_IPUC_FLAG_MERGE_KEY);
    }
    if ( CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_DEFAULT))
    {
        CTC_SET_FLAG(route_flag, SYS_IPUC_FLAG_DEFAULT);
    }
    if ( CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_IS_BIND_NH))
    {
        CTC_SET_FLAG(route_flag, SYS_IPUC_FLAG_IS_BIND_NH);
    }
    if ( CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_HASH_CONFLICT))
    {
        CTC_SET_FLAG(route_flag, SYS_IPUC_FLAG_HASH_CONFLICT);
    }

    addr_size = (p_ctc_param->ip_ver == CTC_IP_VER_4) ? sizeof(ip_addr_t) : sizeof(ipv6_addr_t);

    if (CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_PUBLIC_ROUTE))
    {
        p_ipuc_info->vrf_id = 0;
    }
    else
    {
        p_ipuc_info->vrf_id = p_ctc_param->vrf_id;
    }
    p_ipuc_info->masklen = p_ctc_param->masklen;
    p_ipuc_info->l4_dst_port = p_ctc_param->l4_dst_port;

    sal_memcpy(&(p_ipuc_info->ip), &(p_ctc_param->ip), addr_size);

    p_ipuc_info->route_flag = route_flag;
    SYS_IPUC_CTC2SYS_FLAG_CONVERT(p_ctc_param->route_flag, p_ipuc_info->route_flag);
    if (p_ctc_param->l4_dst_port && p_ctc_param->is_tcp_port)
    {
        CTC_SET_FLAG(p_ipuc_info->route_flag, SYS_IPUC_FLAG_IS_TCP_PORT);
    }

    if (p_ctc_param->ip_ver == CTC_IP_VER_6)
    {
        CTC_SET_FLAG(p_ipuc_info->route_flag, SYS_IPUC_FLAG_IS_IPV6);
    }

    p_ipuc_info->nh_id = p_ctc_param->nh_id;
    p_ipuc_info->gport = p_ctc_param->gport;
    p_ipuc_info->cid = p_ctc_param->cid;
    p_ipuc_info->stats_id = p_ctc_param->stats_id;

    return;
}

STATIC void
_sys_usw_ipuc_info_to_param(uint8 lchip, ctc_ipuc_param_t *p_ctc_param, sys_ipuc_info_t *p_ipuc_info)
{
    uint8 addr_size = 0;

    sal_memset(p_ctc_param, 0, sizeof(ctc_ipuc_param_t));

    addr_size = (SYS_IPUC_VER(p_ipuc_info) == CTC_IP_VER_4) ? sizeof(ip_addr_t) : sizeof(ipv6_addr_t);
    sal_memcpy(&(p_ctc_param->ip), &(p_ipuc_info->ip), addr_size);

    p_ctc_param->ip_ver = SYS_IPUC_VER(p_ipuc_info);
    p_ctc_param->vrf_id = p_ipuc_info->vrf_id;
    p_ctc_param->masklen = p_ipuc_info->masklen;

    p_ctc_param->l4_dst_port = p_ipuc_info->l4_dst_port;
    p_ctc_param->nh_id = p_ipuc_info->nh_id;
    p_ctc_param->gport = p_ipuc_info->gport;
    p_ctc_param->cid = p_ipuc_info->cid;
    p_ctc_param->stats_id = p_ipuc_info->stats_id;

    SYS_IPUC_SYS2CTC_FLAG_CONVERT(p_ctc_param->route_flag, p_ipuc_info->route_flag);

    if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_IS_TCP_PORT))
    {
        p_ctc_param->is_tcp_port = 1;
    }

    return;
}

STATIC int32
_sys_usw_ipuc_db_get_info_cb(void* p_data_o, void* p_data_m)
{
    sys_ipuc_traverse_t* trav_data = (sys_ipuc_traverse_t*)p_data_m;
    sys_ipuc_param_list_t* p_param_list = (sys_ipuc_param_list_t*)(trav_data->data1);
    sys_ipuc_info_t* p_ipuc_info = (sys_ipuc_info_t*)p_data_o;
    uint8 lchip = trav_data->lchip;

    if (p_ipuc_info->route_opt != DO_ALPM)
    {
        return CTC_E_NONE;
    }

    p_param_list->p_ipuc_param = mem_malloc(MEM_IPUC_MODULE, sizeof(ctc_ipuc_param_t));
    if (NULL == p_param_list->p_ipuc_param)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }

    p_param_list->ad_index = p_ipuc_info->ad_index;
    _sys_usw_ipuc_info_to_param(lchip, p_param_list->p_ipuc_param, p_ipuc_info);

    p_param_list->p_next_param = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_ipuc_param_list_t));
    if (NULL == p_param_list->p_next_param)
    {
        mem_free(p_param_list->p_ipuc_param);
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_param_list->p_next_param, 0, sizeof(sys_ipuc_param_list_t));

    trav_data->value1++;
    trav_data->data = p_param_list;
    trav_data->data1 = (void*)p_param_list->p_next_param;

    return CTC_E_NONE;
}

/**
 @brief function of lookup ip route information

 @param[in] pp_ipuc_info, information used for lookup ipuc entry
 @param[out] pp_ipuc_info, information of ipuc entry finded

 @return CTC_E_XXX
 */
int32
_sys_usw_ipuc_db_lookup(uint8 lchip,
                sys_ipuc_param_t* p_sys_param)
{
    uint8 ip_ver = p_sys_param->param.ip_ver;
    sys_ipuc_info_t ipuc_info = {0};

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    _sys_usw_ipuc_param_to_info(lchip, &p_sys_param->param, &ipuc_info);

    p_sys_param->info = ctc_hash_lookup(p_usw_ipuc_master[lchip]->fib_hash[ip_ver], &ipuc_info);

    return CTC_E_NONE;
}

/**
 @brief function of add ip route information

 @param[in] p_ipuc_info, information should be maintained for ipuc

 @return CTC_E_XXX
 */
int32
_sys_usw_ipuc_db_add(uint8 lchip, sys_ipuc_param_t* p_sys_param)
{
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    ctc_hash_insert(p_usw_ipuc_master[lchip]->fib_hash[p_sys_param->param.ip_ver], p_sys_param->info);

    return CTC_E_NONE;
}

/**
 @brief function of add ip route information

 @param[in] p_ipuc_info, information should be maintained for ipuc

 @return CTC_E_XXX
 */
int32
_sys_usw_ipuc_db_remove(uint8 lchip, sys_ipuc_param_t* p_sys_param)
{
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    ctc_hash_remove(p_usw_ipuc_master[lchip]->fib_hash[p_sys_param->param.ip_ver], p_sys_param->info);

    return CTC_E_NONE;
}

int32
sys_usw_ipuc_hash_free_node_data(void* node_data, void* user_data)
{
    mem_free(node_data);

    return CTC_E_NONE;
}

int32
_sys_usw_ipuc_db_traverse(uint8 lchip,
                uint8 ip_ver,
                hash_traversal_fn   fn,
                void                *data)
{
    return ctc_hash_traverse_through(p_usw_ipuc_master[lchip]->fib_hash[ip_ver], fn, data);
}

int32
sys_usw_ipuc_db_init(uint8 lchip)
{
    uint32 entry_num = 0;
    uint32 ad_table_size = 0;
    ctc_spool_t spool;

    p_usw_ipuc_master[lchip]->fib_hash[CTC_IP_VER_4]   = ctc_hash_create(1,
                                (IPUC_IPV4_HASH_MASK + 1),
                                (hash_key_fn)_sys_usw_ipv4_hash_make,
                                (hash_cmp_fn)_sys_usw_ipv4_hash_cmp);

    p_usw_ipuc_master[lchip]->fib_hash[CTC_IP_VER_6]   = ctc_hash_create(1,
                                (IPUC_IPV6_HASH_MASK + 1),
                                (hash_key_fn)_sys_usw_ipv6_hash_make,
                                (hash_cmp_fn)_sys_usw_ipv6_hash_cmp);

    /* init ad spool */
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsIpDa_t, &ad_table_size));
    ad_table_size = (ad_table_size + SYS_IPUC_AD_SPOOL_BLOCK_SIZE - 1) /
        SYS_IPUC_AD_SPOOL_BLOCK_SIZE * SYS_IPUC_AD_SPOOL_BLOCK_SIZE;

    sal_memset(&spool, 0, sizeof(ctc_spool_t));
    spool.lchip = lchip;
    spool.block_num = ad_table_size / SYS_IPUC_AD_SPOOL_BLOCK_SIZE;
    spool.block_size = SYS_IPUC_AD_SPOOL_BLOCK_SIZE;
    spool.max_count = ad_table_size;
    spool.user_data_size = sizeof(sys_ipuc_ad_spool_t);
    spool.spool_key = (hash_key_fn)_sys_usw_ipuc_ad_profile_make;
    spool.spool_cmp = (hash_cmp_fn)_sys_usw_ipuc_ad_profile_cmp;
    spool.spool_alloc = (spool_alloc_fn)_sys_usw_ipuc_ad_profile_build_index;
    spool.spool_free = (spool_free_fn)_sys_usw_ipuc_ad_profile_free_index;
    p_usw_ipuc_master[lchip]->ad_spool = ctc_spool_create(&spool);
    if (!p_usw_ipuc_master[lchip]->ad_spool)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }
    sys_usw_ftm_query_table_entry_num(lchip, DsLpmLookupKey_t, &entry_num);
    MCHIP_CAP(SYS_CAP_SPEC_LPM_ROUTE_ENTRY_NUM) = entry_num;

    return CTC_E_NONE;
}

int32
sys_usw_ipuc_db_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);

    ctc_spool_free(p_usw_ipuc_master[lchip]->ad_spool);

    ctc_hash_traverse(p_usw_ipuc_master[lchip]->fib_hash[CTC_IP_VER_4], (hash_traversal_fn)sys_usw_ipuc_hash_free_node_data, NULL);
    ctc_hash_free(p_usw_ipuc_master[lchip]->fib_hash[CTC_IP_VER_4]);

    ctc_hash_traverse(p_usw_ipuc_master[lchip]->fib_hash[CTC_IP_VER_6], (hash_traversal_fn)sys_usw_ipuc_hash_free_node_data, NULL);
    ctc_hash_free(p_usw_ipuc_master[lchip]->fib_hash[CTC_IP_VER_6]);

    return CTC_E_NONE;
}
#if 0
hash_cmp_fn
sys_usw_get_hash_cmp(uint8 lchip, ctc_ip_ver_t ctc_ip_ver)
{
    if (CTC_IP_VER_4 == ctc_ip_ver)
    {
        return (hash_cmp_fn) & _sys_usw_ipv4_hash_cmp;
    }
    else if (CTC_IP_VER_6 == ctc_ip_ver)
    {
        return (hash_cmp_fn) & _sys_usw_ipv6_hash_cmp;
    }
    else
    {
        return NULL;
    }
}
#endif
int32
sys_usw_get_tbl_id(uint8 lchip, uint8 ip_ver, uint8 is_da, sys_ipuc_tbl_type_t tbl_type, uint32* key_id, uint32* ad_id)
{
    uint32 key_tbl_id = 0;
    uint32 ad_tbl_id = 0;

    switch(tbl_type)
    {
        case SYS_TBL_HASH:
            key_tbl_id = (ip_ver == CTC_IP_VER_4) ? DsFibHost0Ipv4HashKey_t : DsFibHost0Ipv6UcastHashKey_t;
            break;
        case SYS_TBL_HASH_NAPT:
            key_tbl_id = DsFibHost1Ipv4NatDaPortHashKey_t;
            break;
        case SYS_TBL_ALPM:
            if (DRV_IS_DUET2(lchip))
            {
                key_tbl_id = DsLpmLookupKey_t;
            }
            else
            {
                key_tbl_id = DsNeoLpmIpv4Bit32Snake_t;
            }
            break;
        case SYS_TBL_TCAM:
            if (is_da)
            {
                key_tbl_id = (ip_ver == CTC_IP_VER_4) ? DsLpmTcamIpv4HalfKey_t : DsLpmTcamIpv6DoubleKey0_t;
                ad_tbl_id = (ip_ver == CTC_IP_VER_4) ? DsLpmTcamIpv4HalfKeyAd_t : DsLpmTcamIpv6DoubleKey0Ad_t;
            }
            else
            {
                key_tbl_id = (ip_ver == CTC_IP_VER_4) ? DsLpmTcamIpv4SaHalfKey_t : DsLpmTcamIpv6SaDoubleKey0_t;
                ad_tbl_id = (ip_ver == CTC_IP_VER_4) ? DsLpmTcamIpv4SaHalfKeyAd_t : DsLpmTcamIpv6SaDoubleKey0Ad_t;
            }
            break;
        case SYS_TBL_TCAM_PUB:
            if (is_da)
            {
                key_tbl_id = (ip_ver == CTC_IP_VER_4) ? DsLpmTcamIpv4DaPubHalfKey_t : DsLpmTcamIpv6DaPubDoubleKey0_t;
                ad_tbl_id = (ip_ver == CTC_IP_VER_4) ? DsLpmTcamIpv4DaPubHalfKeyAd_t : DsLpmTcamIpv6DaPubDoubleKey0Ad_t;
            }
            else
            {
                key_tbl_id = (ip_ver == CTC_IP_VER_4) ? DsLpmTcamIpv4SaPubHalfKey_t : DsLpmTcamIpv6SaPubDoubleKey0_t;
                ad_tbl_id = (ip_ver == CTC_IP_VER_4) ? DsLpmTcamIpv4SaPubHalfKeyAd_t : DsLpmTcamIpv6SaPubDoubleKey0Ad_t;
            }
            break;
        case SYS_TBL_TCAM_SHORT:
            if (is_da)
            {
                key_tbl_id = DsLpmTcamIpv6SingleKey_t;
                ad_tbl_id = DsLpmTcamIpv6SingleKeyAd_t;
            }
            else
            {
                key_tbl_id = DsLpmTcamIpv6SaSingleKey_t;
                ad_tbl_id = DsLpmTcamIpv6SaSingleKeyAd_t;
            }
            break;
        case SYS_TBL_TCAM_PUB_SHORT:
            if (is_da)
            {
                key_tbl_id = DsLpmTcamIpv6DaPubSingleKey_t;
                ad_tbl_id = DsLpmTcamIpv6DaPubSingleKeyAd_t;
            }
            else
            {
                key_tbl_id = DsLpmTcamIpv6SaPubSingleKey_t;
                ad_tbl_id = DsLpmTcamIpv6SaPubSingleKeyAd_t;
            }
            break;
        default:
            key_tbl_id = 0;
            ad_tbl_id = 0;
            break;
    }

    if (key_id)
    {
        *key_id = key_tbl_id;
    }

    if (ad_id)
    {
        *ad_id = ad_tbl_id;
    }

    return CTC_E_NONE;
}

void
sys_usw_ipuc_update_route_stats(uint8 lchip, uint8 ip_ver, sys_ipuc_route_stats_t type, uint8 opt)
{
    if(0 == opt)
    {
        p_usw_ipuc_master[lchip]->route_stats[ip_ver][type]++;
    }
    else
    {
        p_usw_ipuc_master[lchip]->route_stats[ip_ver][type]--;
    }
    return;
}

#define ___________IPUC_INNER_FUNCTION________________________
#define __0_IPDA__
extern int32 _sys_usw_ipuc_process_hash_key(uint8 lchip, sys_ipuc_param_t* p_sys_param, sys_ipuc_opt_type_t opt);

STATIC int32
_sys_usw_ipuc_add_rpf(uint8 lchip, sys_ipuc_param_t* p_sys_param)
{
    ctc_ipuc_param_t* p_ctc_param = &p_sys_param->param;
    sys_ipuc_info_t* p_ipuc_info = p_sys_param->info;
    sys_nh_info_dsnh_t nh_info = {0};
    sys_nh_info_dsnh_t nh_info_1 = {0};
    sys_rpf_info_t rpf_info = {0};
    uint32 cmd = 0;
    uint32 rpf_check_port = 0;
    uint32 idx = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    rpf_info.is_ipmc = 0;
    if (p_ctc_param->masklen == 0)
    {
        rpf_info.rpf_intf_cnt = 0;

        CTC_ERROR_RETURN(sys_usw_rpf_add_profile(lchip, &rpf_info));
        p_ipuc_info->rpf_mode = rpf_info.mode;
        p_ipuc_info->rpf_id =  rpf_info.rpf_id;

        return CTC_E_NONE;
    }

    if (!CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_ICMP_CHECK) &&
        !CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_RPF_CHECK))
    {
        p_ipuc_info->rpf_mode = 0;
        p_ipuc_info->rpf_id = 0;

        return CTC_E_NONE;
    }

    if (CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_CONNECT))
    {
        if ((p_ctc_param->l3_inf == SYS_L3IF_INVALID_L3IF_ID)
            || (p_ctc_param->l3_inf > (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1)))
        {
            return CTC_E_BADID;
        }
    }

    cmd = DRV_IOR(IpeRouteCtl_t, IpeRouteCtl_rpfCheckAgainstPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rpf_check_port));

    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, p_ctc_param->nh_id, &nh_info, 0));
    if ((nh_info.ecmp_valid)
        && CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_RPF_CHECK))
    {
        rpf_info.rpf_intf_cnt = (nh_info.ecmp_cnt > SYS_USW_MAX_RPF_IF_NUM) ? SYS_USW_MAX_RPF_IF_NUM : nh_info.ecmp_cnt;
        for (idx = 0; idx < rpf_info.rpf_intf_cnt; idx++)
        {
            if (!rpf_check_port)
            {
                CTC_ERROR_RETURN(sys_usw_nh_get_l3ifid(lchip, nh_info.nh_array[idx], &rpf_info.rpf_intf[idx]));
                if (rpf_info.rpf_intf[idx] == SYS_L3IF_INVALID_L3IF_ID)
                {
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop type is invalid \n");
                    return CTC_E_INVALID_CONFIG;
                }
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, nh_info.nh_array[idx], &nh_info_1, 0));
                rpf_info.rpf_intf[idx] = nh_info_1.gport;
            }
        }
    }
    else if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_RPF_CHECK)
                && rpf_check_port
                && p_ctc_param->rpf_port_num > 1)
    {
        rpf_info.rpf_intf_cnt = p_ctc_param->rpf_port_num;
        for (idx = 0; idx < p_ctc_param->rpf_port_num; idx++)
        {
            rpf_info.rpf_intf[idx] = p_ctc_param->rpf_port[idx];
        }
    }
    else
    {
        rpf_info.rpf_intf_cnt = 1;
        if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_ICMP_CHECK)
            || CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_RPF_CHECK))
        {
            /* icmp check must use interfaceId*/
            if ((!rpf_check_port)
                || CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_ICMP_CHECK))
            {
                if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_ASSIGN_PORT))
                {
                    rpf_info.rpf_intf[0] = p_ctc_param->rpf_port[0];
                }
                else if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_CONNECT))
                {
                    rpf_info.rpf_intf[0] = p_ctc_param->l3_inf;
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_nh_get_l3ifid(lchip, p_ctc_param->nh_id, &rpf_info.rpf_intf[0]));
                    if ((rpf_info.rpf_intf[0] == SYS_L3IF_INVALID_L3IF_ID) && (SYS_NH_TYPE_MPLS != nh_info.nh_entry_type))
                    {
                        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [Nexthop] Nexthop type is invalid \n");
			return CTC_E_INVALID_CONFIG;
                    }
                }
            }
            else
            {
                if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_ASSIGN_PORT)
                    && (p_ctc_param->rpf_port_num == 0))
                {
                    return CTC_E_INVALID_PARAM;
                }

                if (p_ctc_param->rpf_port_num > 0)
                {
                    rpf_info.rpf_intf[0] = p_ctc_param->rpf_port[0];
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, p_ctc_param->nh_id, &nh_info, 0));
                    rpf_info.rpf_intf[0] = nh_info.gport;
                }
            }
        }
    }

    CTC_ERROR_RETURN(sys_usw_rpf_add_profile(lchip, &rpf_info));
    p_ipuc_info->rpf_mode = rpf_info.mode;
    p_ipuc_info->rpf_id = rpf_info.rpf_id;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipuc_build_ipda_nexthop(uint8 lchip, sys_ipuc_param_t* p_sys_param, sys_nh_info_dsnh_t* p_dsnh_info, DsIpDa_m* p_dsipda)
{
    uint8   gchip_id    = 0;
    uint16  lport       = 0;
    uint32  dest_id     = 0;
    uint32 dsnh_offset = 0;
    uint32  derive_mode       = 0;
    uint8 adjust_len_idx = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    dsnh_offset = p_dsnh_info->dsnh_offset;

    if (CTC_FLAG_ISSET(p_sys_param->info->route_flag, SYS_IPUC_FLAG_ASSIGN_PORT))
    {
        gchip_id = CTC_MAP_GPORT_TO_GCHIP(p_sys_param->param.gport);
        lport = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_sys_param->param.gport);
        dest_id = SYS_ENCODE_DESTMAP(gchip_id, lport);
    }
    else
    {
        dest_id = p_dsnh_info->dest_map;
    }
    if(DRV_IS_TSINGMA(lchip)|| DRV_IS_DUET2(lchip))
    {
        SetDsIpDa(V, adApsBridgeEn_f, p_dsipda, p_dsnh_info->aps_en);
        SetDsIpDa(V, adDestMap_f, p_dsipda, dest_id);
        SetDsIpDa(V, adNextHopPtr_f, p_dsipda, dsnh_offset);
        SetDsIpDa(V, adNextHopExt_f, p_dsipda, p_dsnh_info->nexthop_ext);
        if(0 != p_dsnh_info->adjust_len)
        {
            sys_usw_lkup_adjust_len_index(lchip, p_dsnh_info->adjust_len, &adjust_len_idx);
            SetDsIpDa(V, adLengthAdjustType_f, p_dsipda, adjust_len_idx);
        }
        else
        {
            SetDsIpDa(V, adLengthAdjustType_f, p_dsipda, 0);
        }
        SetDsIpDa(V, nextHopPtrValid_f, p_dsipda, 1);
    }

    CTC_ERROR_RETURN(sys_usw_global_ctl_get(lchip, CTC_GLOBAL_NH_ARP_MACDA_DERIVE_MODE, &derive_mode));

    if (derive_mode == CTC_GLOBAL_MACDA_DERIVE_FROM_NH)
    {
        if(DRV_IS_TSINGMA(lchip)|| DRV_IS_DUET2(lchip))
        {
            SetDsIpDa(V, adMacDaLowBits_f, p_dsipda, p_dsnh_info->cloud_sec_en << 3);
            SetDsIpDa(V, adNextHopSaveEn_f, p_dsipda, 0);
        }
    }
    else if (derive_mode == CTC_GLOBAL_MACDA_DERIVE_FROM_NH_ROUTE0)
    {
        if(DRV_IS_TSINGMA(lchip)|| DRV_IS_DUET2(lchip))
        {
            SetDsIpDa(V, adMacDaLowBits_f, p_dsipda, ((p_sys_param->param.mac[5]&0x3) | (p_dsnh_info->cloud_sec_en << 3)));
            SetDsIpDa(V, adNextHopSaveEn_f, p_dsipda, 1);
        }
    }
    else if (derive_mode == CTC_GLOBAL_MACDA_DERIVE_FROM_NH_ROUTE1)
    {
        if(DRV_IS_TSINGMA(lchip)|| DRV_IS_DUET2(lchip))
        {
            SetDsIpDa(V, adMacDaLowBits_f, p_dsipda, ((p_sys_param->param.mac[5]&0x7) | (p_dsnh_info->cloud_sec_en << 3)));
            SetDsIpDa(V, adNextHopSaveEn_f, p_dsipda, 1);
        }

    }
    else if (derive_mode == CTC_GLOBAL_MACDA_DERIVE_FROM_NH_ROUTE2 && !p_dsnh_info->cloud_sec_en)
    {
        if(DRV_IS_TSINGMA(lchip)|| DRV_IS_DUET2(lchip))
        {
            SetDsIpDa(V, adMacDaLowBits_f, p_dsipda, (p_sys_param->param.mac[5]&0xf));
            SetDsIpDa(V, adNextHopSaveEn_f, p_dsipda, 1);
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_ipuc_bind_nexthop(uint8 lchip, sys_ipuc_param_t* p_sys_param, uint8 is_bind)
{
    sys_nh_update_dsnh_param_t update_dsnh;
    uint32 nh_id = 0;
    int32 ret = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&update_dsnh, 0, sizeof(sys_nh_update_dsnh_param_t));

    nh_id = (p_sys_param->info && p_sys_param->info->nh_id) ? p_sys_param->info->nh_id : p_sys_param->param.nh_id;
    /* host route, write dsnh to ipda */
    update_dsnh.data = is_bind ? p_sys_param->info : NULL;
    update_dsnh.updateAd = is_bind ? sys_usw_ipuc_update_ipda : NULL;
    ret = sys_usw_nh_bind_dsfwd_cb(p_usw_ipuc_master[lchip]->rchain_en?0:lchip, nh_id, &update_dsnh);
    if (CTC_E_NONE == ret)
    {
        if (is_bind)
        {
            CTC_SET_FLAG(p_sys_param->info->route_flag, SYS_IPUC_FLAG_IS_BIND_NH);
        }
        else
        {
            CTC_UNSET_FLAG(p_sys_param->info->route_flag, SYS_IPUC_FLAG_IS_BIND_NH);
        }
    }

    return ret;
}

int32
_sys_usw_ipuc_build_ipda(uint8 lchip, sys_ipuc_param_t* p_sys_param)
{
    sys_ipuc_info_t* p_ipuc_info = p_sys_param->info;
    sys_nh_info_dsnh_t nh_info = {0};
    int32 ret = CTC_E_NONE;
    uint8 have_dsfwd = 0;
    uint32 derive_mode = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    lchip = p_usw_ipuc_master[lchip]->rchain_en?p_sys_param->info->real_lchip:lchip;
    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, p_sys_param->param.nh_id, &nh_info, 0));
    CTC_ERROR_RETURN(sys_usw_global_ctl_get(lchip, CTC_GLOBAL_NH_ARP_MACDA_DERIVE_MODE, &derive_mode));

    have_dsfwd = (nh_info.dsfwd_valid ||(nh_info.merge_dsfwd == 2) || nh_info.ecmp_valid || nh_info.is_ivi ||
        CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_STATS_EN)) &&
        !CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_ASSIGN_PORT) && !nh_info.is_ecmp_intf;

    if (!have_dsfwd && !CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_ASSIGN_PORT) &&
        (nh_info.merge_dsfwd != 1) && !derive_mode)
    {
        /*do not using dsfwd, do nexthop and ipuc bind*/
        ret = _sys_usw_ipuc_bind_nexthop(lchip, p_sys_param, 1);
        if (CTC_E_IN_USE == ret)
        {
            /*nh have already bind, do unbind, using dsfwd*/
            CTC_UNSET_FLAG(p_ipuc_info->route_flag, SYS_IPUC_FLAG_IS_BIND_NH);
            have_dsfwd = 1;
        }
        else if (ret < 0)
        {
            /*Bind fail, using dsfwd*/
            have_dsfwd = 1;
        }
    }

    if (!have_dsfwd || (nh_info.merge_dsfwd == 1))
    {
        CTC_SET_FLAG(p_ipuc_info->route_flag, SYS_IPUC_FLAG_MERGE_KEY);
    }
    else
    {
        CTC_UNSET_FLAG(p_ipuc_info->route_flag, SYS_IPUC_FLAG_MERGE_KEY);
    }

    /* add rpf */
    CTC_ERROR_GOTO(_sys_usw_ipuc_add_rpf(lchip, p_sys_param), ret, error0);

    return CTC_E_NONE;

error0:
    if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_IS_BIND_NH))
    {
        _sys_usw_ipuc_bind_nexthop(lchip, p_sys_param, 0);
    }

    return ret;
}

int32
_sys_usw_ipuc_unbuild_ipda(uint8 lchip, sys_ipuc_param_t* p_sys_param)
{
    sys_ipuc_info_t* p_ipuc_info = p_sys_param->info;
    sys_rpf_info_t rpf_info = {0};

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    /* update nexthop info, remove nh bind */
    if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_IS_BIND_NH))
    {
        if (p_usw_ipuc_master[lchip]->rchain_en)
        {
            CTC_ERROR_RETURN(_sys_usw_ipuc_bind_nexthop(0, p_sys_param, 0));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_ipuc_bind_nexthop(lchip, p_sys_param, 0));
        }
    }

    /* remove rpf */
    if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_RPF_CHECK))
    {
        rpf_info.is_ipmc = 0;
        rpf_info.mode = p_ipuc_info->rpf_mode;
        rpf_info.rpf_id = p_ipuc_info->rpf_id;
        CTC_ERROR_RETURN(sys_usw_rpf_remove_profile(lchip, &rpf_info));
    }

    /* free ad profile */
    lchip = p_usw_ipuc_master[lchip]->rchain_en?p_ipuc_info->real_lchip:lchip;
    _sys_usw_ipuc_remove_ad_profile(lchip, p_ipuc_info->ad_index);

    return CTC_E_NONE;
}

int32
_sys_usw_ipuc_add_ending(uint8 lchip, sys_ipuc_param_t* p_sys_param)
{
    uint8 ip_ver = CTC_IP_VER_4;

    if(CTC_FLAG_ISSET(p_sys_param->info->route_flag, SYS_IPUC_FLAG_IS_IPV6))
    {
        ip_ver = CTC_IP_VER_6;
    }

    SYS_IPUC_LOCK;

    /* stats */
    if (IS_MAX_MASKLEN(ip_ver, p_sys_param->info->masklen) && !CTC_FLAG_ISSET(p_sys_param->info->route_flag, SYS_IPUC_FLAG_HOST_USE_LPM))
    {
        if(!p_sys_param->info->l4_dst_port)
        {
            sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_HOST, 0);
        }
    }
    else if(!CTC_FLAG_ISSET(p_sys_param->info->route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE))
    {
        sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_LPM, 0);
    }

    /* 7. write to soft table */
    _sys_usw_ipuc_db_add(lchip, p_sys_param);

    SYS_IPUC_UNLOCK;

    return CTC_E_NONE;
}

int32
_sys_usw_ipuc_remove_ending(uint8 lchip, sys_ipuc_param_t* p_sys_param)
{
    uint8 ip_ver = CTC_IP_VER_4;

    if(CTC_FLAG_ISSET(p_sys_param->info->route_flag, SYS_IPUC_FLAG_IS_IPV6))
    {
        ip_ver = CTC_IP_VER_6;
    }

    SYS_IPUC_LOCK;
    CTC_ERROR_RETURN_IPUC_UNLOCK(_sys_usw_ipuc_db_remove(lchip, p_sys_param));
    CTC_ERROR_RETURN_IPUC_UNLOCK(_sys_usw_ipuc_unbuild_ipda(lchip, p_sys_param));
    /* stats */
    if (IS_MAX_MASKLEN(ip_ver, p_sys_param->info->masklen) && !CTC_FLAG_ISSET(p_sys_param->info->route_flag, SYS_IPUC_FLAG_HOST_USE_LPM))
    {
        if(!p_sys_param->info->l4_dst_port)
        {
            sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_HOST, 1);
        }
    }
    else if(!CTC_FLAG_ISSET(p_sys_param->info->route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE))
    {
        sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_LPM, 1);
    }

    mem_free(p_sys_param->info);

    SYS_IPUC_UNLOCK;
    return CTC_E_NONE;
}

int32
_sys_usw_ipuc_update_ending(uint8 lchip, sys_ipuc_info_t* p_old_info)
{
    sys_rpf_info_t rpf_info = {0};

    if (CTC_FLAG_ISSET(p_old_info->route_flag, SYS_IPUC_FLAG_RPF_CHECK))
    {
        rpf_info.is_ipmc = 0;
        rpf_info.mode = p_old_info->rpf_mode;
        rpf_info.rpf_id = p_old_info->rpf_id;
        sys_usw_rpf_remove_profile(lchip, &rpf_info);
    }
    /* free old ad profile */
    if (p_usw_ipuc_master[lchip]->rchain_en)
    {
        lchip = p_old_info->real_lchip;
    }
    _sys_usw_ipuc_remove_ad_profile(lchip, p_old_info->ad_index);

    return CTC_E_NONE;
}

int32
_sys_usw_ipuc_read_ipda(uint8 lchip, uint16 ad_index, void* dsipda)
{
    uint32 cmd = 0;
    cmd = DRV_IOR(DsIpDa_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, ad_index, cmd, dsipda);
    return CTC_E_NONE;
}

int32
_sys_usw_ipuc_write_ipda(uint8 lchip, sys_ipuc_param_t* p_sys_param, uint8 by_user)
{
    /* DsIp_m and ds_ipv4_ucast_da_tcam_t are the same*/
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    DsIpDa_m dsipda;
    uint32 field_value = 0;
    sys_nh_info_dsnh_t nh_info;
    sys_rpf_info_t rpf_info = {0};
    uint32 fwd_offset;
    uint32 stats_ptr = 0;
    uint32 route_flag = p_sys_param->info->route_flag;
    sys_ipuc_info_t* p_ipuc_info = p_sys_param->info;
    ctc_ipuc_param_t* p_ctc_param = &p_sys_param->param;
    uint8 write_ipda = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&dsipda, 0, sizeof(dsipda));
    sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));

    write_ipda = (p_usw_ipuc_master[lchip]->rchain_en && (p_sys_param->info->route_opt != DO_HASH) && !p_sys_param->is_update)?0:1;
    lchip = (p_sys_param->is_update && p_usw_ipuc_master[lchip]->rchain_en)?p_sys_param->info->real_lchip:lchip;

    if (p_ctc_param->cid)
    {
        SetDsIpDa(V, categoryIdValid_f, &dsipda, 1);
        if(DRV_IS_TSINGMA(lchip)|| DRV_IS_DUET2(lchip))
        {
            SetDsIpDa(V, categoryId_f, &dsipda, p_sys_param->param.cid);
        }
        #if defined (TSINGMA) || (DUET2)
        #else
        else
        {
            SetDsIpDa(V, categoryIdOrServiceId_f, &dsipda, p_sys_param->param.cid);
        }
        #endif
    }
    else
    {
        SetDsIpDa(V, categoryIdValid_f, &dsipda, 0);
    }

    field_value = CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_TTL_CHECK);
    SetDsIpDa(V, ttlCheckEn_f, &dsipda, field_value);

  /*   field_value = CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_CPU);*/
  /*   SetDsIpDa(V, exception3CtlEn_f, &dsipda, field_value);*/

    field_value = CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_PROTOCOL_ENTRY);
    SetDsIpDa(V, exception3CtlEn_f, &dsipda, field_value);

    if(DRV_IS_TSINGMA(lchip)|| DRV_IS_DUET2(lchip))
    {
        field_value = CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_SELF_ADDRESS);
        SetDsIpDa(V, selfAddress_f, &dsipda, field_value);
    }
    field_value = CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_ICMP_CHECK);
    SetDsIpDa(V, icmpCheckEn_f, &dsipda, field_value);

    field_value = CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_ICMP_ERR_MSG_CHECK);
    SetDsIpDa(V, icmpErrMsgCheckEn_f, &dsipda, field_value);

    if(!p_sys_param->param.masklen)
    {
        if (DRV_IS_DUET2(lchip))
        {
            field_value = 1;
            SetDsIpDa(V, isDefaultRoute_f, &dsipda, field_value);
        }
        else
        {
            if (by_user)
            {
                field_value = 1;
                SetDsIpDa(V, isDefaultRoute_f, &dsipda, field_value);
                field_value = 0;
                SetDsIpDa(V, isCtcDefaultRoute_f, &dsipda, field_value);
            }
            else
            {
                field_value = 0;
                SetDsIpDa(V, isDefaultRoute_f, &dsipda, field_value);
                field_value = 1;
                SetDsIpDa(V, isCtcDefaultRoute_f, &dsipda, field_value);
            }
        }
    }

    field_value = CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_RPF_CHECK);
    if(DRV_IS_TSINGMA(lchip)|| DRV_IS_DUET2(lchip))
    {
        SetDsIpDa(V, rpfCheckEn_f, &dsipda, field_value);
        field_value = p_ipuc_info->rpf_id;
        SetDsIpDa(V, rpfIfId_f, &dsipda, field_value);
    }
    #if defined (TSINGMA) || (DUET2)
    #else
    else
    {
        if (field_value)
        {
            field_value = p_ipuc_info->rpf_id;
        }
        else
        {
            field_value = 0xffff;
        }
        SetDsIpDa(V, u2_g1_rpfIfId_f, &dsipda, field_value);
        field_value = 0;
        SetDsIpDa(V, u2Type_f, &dsipda, field_value);
    }
    #endif
    field_value = CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_CANCEL_NAT);
    SetDsIpDa(V, l3IfType_f, &dsipda, field_value);  /* 0- external,1- internal */

    field_value = p_ipuc_info->rpf_mode;
    SetDsIpDa(V, rpfCheckMode_f, &dsipda, field_value);

    CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, p_ctc_param->nh_id, &nh_info, 0));

    if(DRV_IS_TSINGMA(lchip)|| DRV_IS_DUET2(lchip))
    {
        if (nh_info.is_ivi)
        {
            SetDsIpDa(V, iviEnable_f, &dsipda, 1);  /* ivi enable */
            SetDsIpDa(V, ptEnable_f, &dsipda, 1);   /* need do ipv4-ipv6 address translate */
        }
    }
    if(CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_CPU))
    {
        if (DRV_IS_DUET2(lchip))
        {
            /*to CPU must be equal to 63(get the low 5bits is 31), refer to sys_usw_pdu.h:SYS_L3PDU_PER_L3IF_ACTION_INDEX_RSV_IPDA_TO_CPU*/
            field_value = CTC_L3PDU_ACTION_INDEX_IPDA & 0x1F;
        }
        else
        {
            field_value = CTC_L3PDU_ACTION_INDEX_IPDA;
        }
        SetDsIpDa(V, exceptionSubIndex_f, &dsipda, field_value);
    }

    if (CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_MERGE_KEY))
    {
        CTC_ERROR_RETURN(_sys_usw_ipuc_build_ipda_nexthop(lchip, p_sys_param, &nh_info, &dsipda));
    }
    else
    {
        if (nh_info.ecmp_valid)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Ecmp number is %d\r\n", nh_info.ecmp_cnt);
            if(DRV_IS_TSINGMA(lchip)|| DRV_IS_DUET2(lchip))
            {
                SetDsIpDa(V, ecmpEn_f, &dsipda, 1);
                field_value = nh_info.ecmp_group_id & 0x3FF;       /* ecmp group id 10bit */
                SetDsIpDa(V, dsFwdPtrOrEcmpGroupId_f, &dsipda, field_value);
            }
            #if defined (TSINGMA) || (DUET2)
            #else
            else
            {
                SetDsIpDa(V, u1Type_f, &dsipda, 0);
                field_value = nh_info.ecmp_group_id & 0xFFF;
                SetDsIpDa(V, u1_g1_ecmpGroupId_f, &dsipda, field_value);
            }
            #endif
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, p_ctc_param->nh_id, &fwd_offset, 0));
            field_value = fwd_offset & 0xFFFF;
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC: old ad index = %d, dsfwd ptr = 0x%x\n",
                                 p_ipuc_info->ad_index, field_value);
            if(DRV_IS_TSINGMA(lchip)|| DRV_IS_DUET2(lchip))
            {
                SetDsIpDa(V, dsFwdPtrOrEcmpGroupId_f, &dsipda, field_value);

                SetDsIpDa(V, ecmpEn_f, &dsipda, 0);

                if (nh_info.is_ecmp_intf)
                {
                    SetDsIpDa(V, ecmpGroupIdHighBits_f, &dsipda, (nh_info.ecmp_group_id >> 3)&0x7F);
                    SetDsIpDa(V, ptEnable_f, &dsipda, (nh_info.ecmp_group_id >> 2)&0x1);
                    SetDsIpDa(V, isatapCheckEn_f, &dsipda, (nh_info.ecmp_group_id >> 1)&0x1);
                    SetDsIpDa(V, priorityPathEn_f, &dsipda, nh_info.ecmp_group_id && 0x1);
                    SetDsIpDa(V, ecmpTunnelMode_f, &dsipda, 1);
                }

                if (CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_STATS_EN))
                {
                    CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr((p_usw_ipuc_master[lchip]->rchain_en?0:lchip), p_ctc_param->stats_id, &stats_ptr));
                    SetDsIpDa(V, tunnelGreOption_f, &dsipda, stats_ptr&0x7);
                    SetDsIpDa(V, tunnelPacketType_f, &dsipda, (stats_ptr >> 3)&0x7);
                    SetDsIpDa(V, tunnelPayloadOffset_f, &dsipda, (stats_ptr >> 6)&0xF);
                    SetDsIpDa(V, payloadOffsetType_f, &dsipda, (stats_ptr >> 10)&0x1);
                    SetDsIpDa(V, payloadSelect_f, &dsipda, (stats_ptr >> 11)&0x3);
                    if (DRV_IS_DUET2(lchip))
                    {
                        SetDsIpDa(V, statsPtrHighBits_f, &dsipda, (stats_ptr >> 13)&0x3);
                    }
                    else
                    {
                        SetDsIpDa(V, statsPtrHighBits_f, &dsipda, (stats_ptr >> 13)&0x7);
                    }
                    SetDsIpDa(V, statsOrDecapSel_f, &dsipda, 1);
                }
            }
            #if defined (TSINGMA) || (DUET2)
            #else
            else
            {
                SetDsIpDa(V, u1_g3_dsFwdPtr_f, &dsipda, field_value);
                SetDsIpDa(V, u1Type_f, &dsipda, 2);

                if (nh_info.is_ecmp_intf)
                {
                     SetDsIpDa(V, u1_g3_ecmpTunnelMode_f, &dsipda, 1);
                     SetDsIpDa(V, u1_g3_ecmpGroupId_f, &dsipda, nh_info.ecmp_group_id & 0xFFF);
                }
                if (CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_STATS_EN))
                {
                    CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr((p_usw_ipuc_master[lchip]->rchain_en?0:lchip), p_ctc_param->stats_id, &stats_ptr));
                    SetDsIpDa(V, u1_g3_statsPtr_f, &dsipda, stats_ptr);
                }
            }
            #endif

        }
    }

    if (!CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_DEFAULT) && write_ipda)
    {
        /* _sys_usw_ipuc_add_ad_profile param 3 is always 0 beacause we remove old dsipda in update ending func */
        ret = _sys_usw_ipuc_add_ad_profile(lchip, p_sys_param, (void*)&dsipda, 0);
        if (ret)
        {
            _sys_usw_ipuc_bind_nexthop(lchip, p_sys_param, 0);
            if (CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_RPF_CHECK))
            {
                rpf_info.is_ipmc = 0;
                rpf_info.mode = p_ipuc_info->rpf_mode;
                rpf_info.rpf_id = p_ipuc_info->rpf_id;
                sys_usw_rpf_remove_profile(lchip, &rpf_info);
            }

            return ret;
        }
    }
    if(DRV_IS_TSINGMA(lchip) || DRV_IS_DUET2(lchip) )
    {
        cmd = DRV_IOW(DsIpDa_t, DRV_ENTRY_FLAG);
    }
    #if defined (TSINGMA) || (DUET2)
    #else
    else
    {
        cmd = DRV_IOW(DsIp_t, DRV_ENTRY_FLAG);
    }
    #endif

    if (write_ipda || CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_DEFAULT))
    {
        DRV_IOCTL(lchip, p_ipuc_info->ad_index, cmd, &dsipda);
    }

    if (p_sys_param->p_dsipda)
    {
        sal_memcpy(p_sys_param->p_dsipda, &dsipda, sizeof(dsipda));
    }
    return CTC_E_NONE;
}

int32
_sys_usw_ipuc_write_dft_destmap(uint8 lchip)
{
    uint32 cmd = 0;
    DsIpDa_m dsipda;
    uint32 field_value = 0;
    uint16 ad_index = 0;
    uint16 vrfid = 0;
    uint32 dest_id = 0;
    uint8 ip_ver = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if(lchip == sys_usw_get_local_chip_num()-1)
    {
        return CTC_E_NONE;
    }

    sal_memset(&dsipda, 0, sizeof(dsipda));

    if (DRV_IS_DUET2(lchip))
    {
        field_value = 1;
        SetDsIpDa(V, isDefaultRoute_f, &dsipda, field_value);
    }
    else
    {
        field_value = 0;
        SetDsIpDa(V, isDefaultRoute_f, &dsipda, field_value);
        field_value = 1;
        SetDsIpDa(V, isCtcDefaultRoute_f, &dsipda, field_value);
    }
    for(ip_ver = CTC_IP_VER_4; ip_ver < MAX_CTC_IP_VER; ip_ver++)
    {
        for (vrfid = 0; vrfid <= MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID); vrfid++)
        {
            /* p_usw_ipuc_master[lchip]->rchain_gchip is used to route chain default entry */
            dest_id = SYS_ENCODE_DESTMAP(p_usw_ipuc_master[lchip]->rchain_gchip, VRF_MAP_DEST_MAP_BASE + vrfid);
            if (DRV_IS_TSINGMA(lchip) || DRV_IS_DUET2(lchip))
            {
                SetDsIpDa(V, adDestMap_f, &dsipda, dest_id);
                SetDsIpDa(V, nextHopPtrValid_f, &dsipda, 1);
                SetDsIpDa(V, adNextHopPtr_f, &dsipda, 0xfffe);
            }

            cmd = DRV_IOW(DsIpDa_t, DRV_ENTRY_FLAG);
            ad_index = p_usw_ipuc_master[lchip]->default_base[ip_ver] + (vrfid & 0x3FFF);
            DRV_IOCTL(lchip, ad_index, cmd, &dsipda);
        }
    }

    return CTC_E_NONE;
}

#define __1_KEY__
STATIC int32
_sys_usw_ipuc_traverse_pre(void* entry, void* p_trav)
{
    uint8 addr_len = 0;
    ctc_ipuc_param_t ipuc_param;
    ctc_ipuc_param_t* p_ipuc_param = &ipuc_param;
    sys_ipuc_info_t* p_ipuc_info = entry;
    hash_traversal_fn fn = ((sys_ipuc_traverse_t*)p_trav)->fn;
    void* data = ((sys_ipuc_traverse_t*)p_trav)->data;

    sal_memset(p_ipuc_param, 0, sizeof(ctc_ipuc_param_t));

    p_ipuc_param->nh_id = p_ipuc_info->nh_id;
    p_ipuc_param->vrf_id = p_ipuc_info->vrf_id;
    p_ipuc_param->route_flag = 0;
    SYS_IPUC_SYS2CTC_FLAG_CONVERT(p_ipuc_param->route_flag, p_ipuc_info->route_flag);
    p_ipuc_param->masklen = p_ipuc_info->masklen;
    p_ipuc_param->l4_dst_port = p_ipuc_info->l4_dst_port;
    p_ipuc_param->is_tcp_port = CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_IS_TCP_PORT)
        ? 1 : 0;
    p_ipuc_param->ip_ver = SYS_IPUC_VER(p_ipuc_info);

    addr_len = (p_ipuc_param->ip_ver == CTC_IP_VER_4) ? sizeof(ip_addr_t): sizeof(ipv6_addr_t);
    sal_memcpy(&(p_ipuc_param->ip), &(p_ipuc_info->ip), addr_len);

    return (* fn)(p_ipuc_param, data);
}

uint32
_sys_usw_ipuc_get_store_position(uint8 lchip, sys_ipuc_param_t* p_sys_param, uint8* opt)
{
    ctc_ipuc_param_t* p_ctc_param = &p_sys_param->param;
    uint8 ip_ver = p_ctc_param->ip_ver;
    uint8 masklen = p_ctc_param->masklen;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_PUBLIC_ROUTE) ||
            CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_HOST_USE_LPM) ||
            p_sys_param->hash_conflict)
    {
        if (!CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_TCAM_LOOKUP))
        {
            return CTC_E_NO_RESOURCE;
        }
        *opt = DO_TCAM;
    }
    else if ((IS_MAX_MASKLEN(ip_ver, masklen) &&
                    !CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_HOST_USE_LPM) &&
                    CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_HASH_LOOKUP)) ||
                    p_ctc_param->l4_dst_port)
    {
        if (p_ctc_param->l4_dst_port)
        {
            if (!CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_HASH_NAPT_LOOKUP))
            {
                return CTC_E_NO_RESOURCE;
            }
        }
        *opt = DO_HASH;
    }
    //TBD-
#ifdef DUET2
    else if (DRV_IS_DUET2(lchip)&&((masklen >= MIN_CALPM_MASKLEN(lchip, ip_ver)) && (masklen <= MAX_CALPM_MASKLEN(lchip, ip_ver))) &&
                    !(IS_MAX_MASKLEN(ip_ver, masklen) && p_usw_ipuc_master[lchip]->host_lpm_mode) &&
                    CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_ALPM_LOOKUP))
    {
        *opt = DO_ALPM;
    }
#endif
    else if(DRV_IS_TSINGMA(lchip) && CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_TCAM_SHORT_LOOKUP) && p_sys_param->param.masklen > 64)
    {
        *opt = DO_TCAM;
    }
    else if( !DRV_IS_DUET2(lchip)&&!(IS_MAX_MASKLEN(ip_ver, masklen) && p_usw_ipuc_master[lchip]->host_lpm_mode) &&
                    CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_ALPM_LOOKUP))
    {
        *opt = DO_ALPM;
    }
    else if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_TCAM_LOOKUP))
    {
        *opt = DO_TCAM;
    }
    else
    {
        return CTC_E_NO_RESOURCE;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_ipuc_process_hash_key(uint8 lchip, sys_ipuc_param_t* p_sys_param, sys_ipuc_opt_type_t opt)
{
    uint8 valid = 0;
    uint8 l4PortType = 0;
    ds_t key;
    ipv6_addr_t ipv6_data;
    drv_acc_in_t fib_acc_in;
    drv_acc_out_t fib_acc_out;
    sys_ipuc_tbl_type_t tbl_type;
    sys_ipuc_info_t* p_ipuc_info = p_sys_param->info;
    ctc_ipuc_param_t* p_ctc_param = &p_sys_param->param;

    sal_memset(&key, 0, sizeof(ds_t));
    sal_memset(&ipv6_data, 0, sizeof(ipv6_data));
    sal_memset(&fib_acc_in, 0, sizeof(fib_acc_in));
    sal_memset(&fib_acc_out, 0, sizeof(fib_acc_out));

    if (!p_sys_param->param.l4_dst_port)
    {
        valid = (opt == DO_DEL) ? 0 : 1;
        if (CTC_IP_VER_4 == p_sys_param->param.ip_ver)
        {
            SetDsFibHost0Ipv4HashKey(V, ipDa_f, &key, p_ctc_param->ip.ipv4);
            SetDsFibHost0Ipv4HashKey(V, vrfId_f, &key, p_ctc_param->vrf_id);
            SetDsFibHost0Ipv4HashKey(V, valid_f, &key, valid);
            SetDsFibHost0Ipv4HashKey(V, dsAdIndex_f, &key, p_ipuc_info->ad_index);
            SetDsFibHost0Ipv4HashKey(V, ipv4Type_f, &key, 0);
            SetDsFibHost0Ipv4HashKey(V, hashType_f, &key, FIBHOST0PRIMARYHASHTYPE_IPV4);
        }
        else
        {
            /* DRV_SET_FIELD_A, ipv6_data must use little india */
            sal_memcpy(&ipv6_data, &(p_ctc_param->ip.ipv6), sizeof(ipv6_addr_t));
            SetDsFibHost0Ipv6UcastHashKey(A, ipDa_f, &key, ipv6_data);
            SetDsFibHost0Ipv6UcastHashKey(V, vrfId_f, &key, p_ctc_param->vrf_id);
            SetDsFibHost0Ipv6UcastHashKey(V, valid0_f, &key, valid);
            SetDsFibHost0Ipv6UcastHashKey(V, valid1_f, &key, valid);
            SetDsFibHost0Ipv6UcastHashKey(V, dsAdIndex_f, &key, p_ipuc_info->ad_index);
            SetDsFibHost0Ipv6UcastHashKey(V, hashType0_f, &key, FIBHOST0PRIMARYHASHTYPE_IPV6UCAST);
            SetDsFibHost0Ipv6UcastHashKey(V, hashType1_f, &key, FIBHOST0PRIMARYHASHTYPE_IPV6UCAST);
        }
        tbl_type = SYS_TBL_HASH;
        sys_usw_get_tbl_id(lchip, p_ctc_param->ip_ver, 1, tbl_type, &fib_acc_in.tbl_id, NULL);
        if(opt == DO_ADD)
        {
            fib_acc_in.type = DRV_ACC_TYPE_LOOKUP;
            fib_acc_in.op_type = DRV_ACC_OP_BY_KEY;
            fib_acc_in.data = (void*)key;

            CTC_ERROR_RETURN(drv_acc_api(lchip, &fib_acc_in, &fib_acc_out));
            if (fib_acc_out.is_conflict ||
                (fib_acc_out.key_index < 32 && !p_usw_ipuc_master[lchip]->arc[CTC_IP_VER_6] && p_ctc_param->ip_ver == CTC_IP_VER_4))
            {
                return CTC_E_HASH_CONFLICT;
            }
            fib_acc_in.type = DRV_ACC_TYPE_ADD;
            fib_acc_in.index = fib_acc_out.key_index;
            fib_acc_in.op_type = DRV_ACC_OP_BY_INDEX;
            sal_memset(&fib_acc_out, 0, sizeof(fib_acc_out));
        }
        else
        {
            fib_acc_in.type = (opt == DO_LOOKUP) ? DRV_ACC_TYPE_LOOKUP : DRV_ACC_TYPE_ADD;
            if (opt != DO_LOOKUP)
            {
                CTC_BIT_SET(fib_acc_in.flag, DRV_ACC_OVERWRITE_EN);
            }
            fib_acc_in.op_type = DRV_ACC_OP_BY_KEY;
            fib_acc_in.data = (void*)key;
            fib_acc_in.index = p_ipuc_info->key_index;
        }
    }
    else
    {
        valid = 1;
        l4PortType = (p_ipuc_info->route_flag & SYS_IPUC_FLAG_IS_TCP_PORT) ? 3 : 2;
        SetDsFibHost1Ipv4NatDaPortHashKey(V, ipDa_f, &key, p_ctc_param->ip.ipv4);
        SetDsFibHost1Ipv4NatDaPortHashKey(V, l4DestPort_f, &key, p_ctc_param->l4_dst_port);
        SetDsFibHost1Ipv4NatDaPortHashKey(V, l4PortType_f, &key, l4PortType);
        SetDsFibHost1Ipv4NatDaPortHashKey(V, vrfId_f, &key, p_ctc_param->vrf_id);
        SetDsFibHost1Ipv4NatDaPortHashKey(V, valid_f, &key, valid);
        SetDsFibHost1Ipv4NatDaPortHashKey(V, dsAdIndex_f, &key, p_ipuc_info->ad_index);
        SetDsFibHost1Ipv4NatDaPortHashKey(V, ipv4Type_f, &key, 0);
        SetDsFibHost1Ipv4NatDaPortHashKey(V, hashType_f, &key, FIBHOST1PRIMARYHASHTYPE_IPV4);
        tbl_type = SYS_TBL_HASH_NAPT;
        sys_usw_get_tbl_id(lchip, p_ctc_param->ip_ver, 1, tbl_type, &fib_acc_in.tbl_id, NULL);
        fib_acc_in.type = (opt == DO_LOOKUP) ? DRV_ACC_TYPE_LOOKUP : ((opt == DO_DEL)?DRV_ACC_TYPE_DEL : DRV_ACC_TYPE_ADD);
        fib_acc_in.op_type = DRV_ACC_OP_BY_KEY;
        fib_acc_in.data = (void*)key;
    }

    CTC_ERROR_RETURN(drv_acc_api(lchip, &fib_acc_in, &fib_acc_out));
    if (fib_acc_out.is_conflict)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [IPUC] Hash confict \n");
        return CTC_E_HASH_CONFLICT;
    }
    p_ipuc_info->key_index = fib_acc_in.index;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC: %s %s key_index = 0x%x\n", (opt == DO_ADD) ? "write" : "remove", TABLE_NAME(lchip, fib_acc_in.tbl_id), p_ipuc_info->key_index);

    return CTC_E_NONE;
}

#define __3_OTHER__
STATIC int32
_sys_usw_ipuc_check(uint8 lchip, ctc_ipuc_param_t* p_ctc_param)
{

    CTC_PTR_VALID_CHECK(p_ctc_param);
    CTC_IP_VER_CHECK(p_ctc_param->ip_ver);
    SYS_IP_CHECK_VERSION_ENABLE(p_ctc_param->ip_ver);
    SYS_IP_VRFID_CHECK(p_ctc_param->vrf_id, p_ctc_param->ip_ver);
    SYS_IPUC_MASK_LEN_CHECK(p_ctc_param->ip_ver, p_ctc_param->masklen);
    SYS_IP_ADDRESS_SORT(p_ctc_param);
    SYS_IP_ADDR_MASK(p_ctc_param->ip, p_ctc_param->masklen, p_ctc_param->ip_ver);
    SYS_USW_CID_CHECK(lchip,p_ctc_param->cid);
    SYS_GLOBAL_PORT_CHECK(p_ctc_param->gport);
    if((1 == p_ctc_param->nh_id || 2 == p_ctc_param->nh_id) && CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_ASSIGN_PORT))
    {
        return CTC_E_INVALID_PARAM;
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipuc_spec_check(uint8 lchip, ctc_ipuc_param_t* p_ctc_param)
{
    uint8 step[2] = {1, 2};

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (IS_MAX_MASKLEN(p_ctc_param->ip_ver, p_ctc_param->masklen) &&
            !CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_HOST_USE_LPM) &&
            !p_ctc_param->l4_dst_port)
    {
        if (p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_HOST] + p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_HOST] * 2
                + step[p_ctc_param->ip_ver] > SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_IPUC_HOST))
        {
            return CTC_E_NO_RESOURCE;
        }
    }
    else if (!IS_MAX_MASKLEN(p_ctc_param->ip_ver, p_ctc_param->masklen) || CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_HOST_USE_LPM))
    {
        if (p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_LPM] + p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_LPM]
            >= SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_IPUC_LPM))
        {
            return CTC_E_NO_RESOURCE;
        }
    }
    else if (IS_MAX_MASKLEN(p_ctc_param->ip_ver, p_ctc_param->masklen) && p_ctc_param->l4_dst_port)
    {
        if ((p_usw_ip_tunnel_master[lchip]->snat_hash_count + p_usw_ip_tunnel_master[lchip]->napt_hash_count) >=
            SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_NAPT))
        {
            return CTC_E_NO_RESOURCE;
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipuc_flag_check(uint8 lchip, ctc_ipuc_param_t* p_ctc_param, uint8 check_nh)
{
    sys_nh_info_dsnh_t nh_info;
    uint32 cmd = 0;
    uint32 rpf_check_port = 0;

    CTC_PTR_VALID_CHECK(p_ctc_param);
    sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));

    if (CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_RPF_CHECK) && p_usw_ipuc_master[lchip]->rchain_en)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }
    if (CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_TTL_CHECK) && CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_TTL_NO_CHECK))
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Config Conflict \n");
        return CTC_E_PARAM_CONFLICT;
    }
    if (CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_AGING_EN))
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    if (CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_PUBLIC_ROUTE) &&
        !CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_PUB_LOOKUP))
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    /* host use lpm flag check */
    if (CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_HOST_USE_LPM))
    {
        if (!p_usw_ipuc_master[lchip]->host_lpm_mode)
        {
            return CTC_E_NOT_SUPPORT;
        }

        if (!IS_MAX_MASKLEN(p_ctc_param->ip_ver, p_ctc_param->masklen) || CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_PUBLIC_ROUTE))
        /* clear host use lpm flag */
        CTC_UNSET_FLAG(p_ctc_param->route_flag, CTC_IPUC_FLAG_HOST_USE_LPM);
    }

    if (check_nh && p_ctc_param->nh_id)
    {
        CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, p_ctc_param->nh_id, &nh_info, 0));

        /* icmp flag check */
        if ((nh_info.ecmp_valid) && CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_ICMP_CHECK))
        {
            /* clear icmp-check flag */
            CTC_UNSET_FLAG(p_ctc_param->route_flag, CTC_IPUC_FLAG_ICMP_CHECK);
        }

        if ((nh_info.ecmp_valid)
            && CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_RPF_CHECK)
            && nh_info.ecmp_cnt > SYS_USW_MAX_RPF_IF_NUM)
        {
            return CTC_E_INVALID_PARAM;
        }

        /* assign port check */
        if (CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_ASSIGN_PORT)
            && nh_info.ecmp_valid)
        {
            return CTC_E_INVALID_PARAM;
        }
    }

    cmd = DRV_IOR(IpeRouteCtl_t, IpeRouteCtl_rpfCheckAgainstPort_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rpf_check_port));

    if (rpf_check_port
        && CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_ICMP_CHECK)
        && CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_RPF_CHECK))
    {
        /* when rpf check port, icmp check and rpf check can not both support */
        return CTC_E_INVALID_PARAM;
    }

    /* NAPT check */
    if (p_ctc_param->l4_dst_port > 0)
    {
        if (!IS_MAX_MASKLEN(p_ctc_param->ip_ver, p_ctc_param->masklen))
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [IPUC] NAPT not support this mask length. for IPV4 only support mask length 32 \n");
            return CTC_E_INVALID_CONFIG;
        }

        if (p_ctc_param->ip_ver == CTC_IP_VER_6)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
            return CTC_E_NOT_SUPPORT;
        }

        if (CTC_FLAG_ISSET(p_ctc_param->route_flag, CTC_IPUC_FLAG_HOST_USE_LPM))
        {
            CTC_UNSET_FLAG(p_ctc_param->route_flag, CTC_IPUC_FLAG_HOST_USE_LPM);
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_ipuc_rpf_check_port(uint8 lchip, uint8 enable)
{
    uint32 cmd = 0;
    uint32 field_value = 0;

    LCHIP_CHECK(lchip);

    cmd = DRV_IOW(IpeRouteCtl_t, IpeRouteCtl_rpfCheckAgainstPort_f);
    field_value = enable ? 1 : 0;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    return CTC_E_NONE;
}

int32
_sys_usw_ipuc_host_pre_process(uint8 lchip, sys_ipuc_param_t* p_sys_param)
{
    uint32 route_flag = p_sys_param->param.route_flag;
    uint8 ip_ver = p_sys_param->param.ip_ver;
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 hash_key_index = 0;
    uint32 tcam_key_index = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (CTC_FLAG_ISSET(p_sys_param->param.route_flag, CTC_IPUC_FLAG_HOST_USE_LPM))
    {
        CTC_UNSET_FLAG(p_sys_param->param.route_flag, CTC_IPUC_FLAG_HOST_USE_LPM);
        ret = _sys_usw_ipuc_db_lookup(lchip, p_sys_param);
        if (ret || !p_sys_param->info)
        {
            CTC_SET_FLAG(p_sys_param->param.route_flag, CTC_IPUC_FLAG_HOST_USE_LPM);
            return ret;
        }

        if (p_sys_param->info->route_opt == DO_HASH)
        {
            /* add to tcam first */
            hash_key_index = p_sys_param->info->key_index;
            CTC_SET_FLAG(p_sys_param->info->route_flag, SYS_IPUC_FLAG_HASH_CONFLICT);
            if (p_usw_ipuc_master[lchip]->rchain_en)
            {
                cmd = DRV_IOR(DsIpDa_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(0, p_sys_param->info->ad_index, cmd, p_sys_param->p_dsipda);
            }
            ret = sys_usw_ipuc_add_tcam(lchip, p_sys_param);
            if (ret)
            {
                CTC_UNSET_FLAG(p_sys_param->info->route_flag, SYS_IPUC_FLAG_HASH_CONFLICT);
                CTC_SET_FLAG(p_sys_param->param.route_flag, CTC_IPUC_FLAG_HOST_USE_LPM);
                return ret;
            }
            tcam_key_index = p_sys_param->info->key_index;
            /* delete hash */
            p_sys_param->info->key_index = hash_key_index;
            ret = _sys_usw_ipuc_process_hash_key(lchip, p_sys_param, DO_DEL);
            if (ret)
            {
                p_sys_param->info->key_index = tcam_key_index;
                sys_usw_ipuc_del_tcam(lchip, p_sys_param);
                CTC_UNSET_FLAG(p_sys_param->info->route_flag, SYS_IPUC_FLAG_HASH_CONFLICT);
                CTC_SET_FLAG(p_sys_param->param.route_flag, CTC_IPUC_FLAG_HOST_USE_LPM);
                return ret;
            }
            p_sys_param->info->key_index = tcam_key_index;

            p_sys_param->info->route_opt = DO_TCAM;
            sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_HASH, 1);
            sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_CONFLICT_TCAM, 0);
        }
    }
    else
    {
        CTC_SET_FLAG(p_sys_param->param.route_flag, CTC_IPUC_FLAG_HOST_USE_LPM);
        CTC_ERROR_RETURN(_sys_usw_ipuc_db_lookup(lchip, p_sys_param));
        if (p_sys_param->info)
        {
            p_sys_param->hash_conflict = 1;
        }
    }
    p_sys_param->param.route_flag = route_flag;

    return CTC_E_NONE;
}
STATIC int32
_sys_lpm_ipaddr_cmp(void* p_data_new, void* p_data_old)
{
     ctc_ipuc_param_t* p_new = ( ctc_ipuc_param_t*)p_data_new;
     ctc_ipuc_param_t* p_old = ( ctc_ipuc_param_t*)p_data_old;


    if (p_new->ip_ver == CTC_IP_VER_4)
    {
        if (p_new->ip.ipv4 == p_old->ip.ipv4)
            return 0;
        else if(p_new->ip.ipv4 > p_old->ip.ipv4)
            return 1;
        else
            return -1;
    }
    else
    {

        if ((p_new->ip.ipv6[3] ==  p_old->ip.ipv6[3]) && (p_new->ip.ipv6[2] ==  p_old->ip.ipv6[2])
            && (p_new->ip.ipv6[1] ==  p_old->ip.ipv6[1]) && (p_new->ip.ipv6[0] ==  p_old->ip.ipv6[0]))
        {
            return 0;
        }

        if (p_new->ip.ipv6[3] > p_old->ip.ipv6[3])
        {
            return 1;
        }

        if ((p_new->ip.ipv6[3] ==  p_old->ip.ipv6[3]) && (p_new->ip.ipv6[2] >  p_old->ip.ipv6[2]))
        {
            return 1;
        }

        if ((p_new->ip.ipv6[3] ==  p_old->ip.ipv6[3]) && (p_new->ip.ipv6[2] ==  p_old->ip.ipv6[2]) &&( p_new->ip.ipv6[1] >  p_old->ip.ipv6[1]))
        {
            return 1;
        }

        if ((p_new->ip.ipv6[3] ==  p_old->ip.ipv6[3]) && (p_new->ip.ipv6[2] ==  p_old->ip.ipv6[2])
            &&(p_new->ip.ipv6[1] ==  p_old->ip.ipv6[1] ) && (p_new->ip.ipv6[0] >  p_old->ip.ipv6[0]))
        {
            return 1;
        }

        return -1;
    }


}



int32 _sys_lpm_ipaddr_traverse(void *data, void *user_data, void *user_data2)
{
    uint32* p_array_idx =   (uint32*) user_data2;

    /*TBD000000000000000000000000000000000000 ,need testing  */
     *((ctc_ipuc_param_t**)user_data + *p_array_idx)  =  (ctc_ipuc_param_t*)data;
     (*p_array_idx)++;

    return CTC_E_NONE;
}

/*1- increase ;0 -normal; -1-decrease*/
int32 _sys_lpm_judge_order( ctc_slist_t* slist)
{
    uint16 i = 0, cnt = 0;
    uint32 array_idx = 0;
    ctc_avl_tree_t*      sort_tree;
    ctc_ipuc_param_t  **p_temp_ipuc_array;
    struct ctc_slistnode_s*  pnext   = NULL;
    sys_ipuc_pending_route_t *pending_node;
    ctc_ipuc_param_t *p_param ;
    p_temp_ipuc_array = (ctc_ipuc_param_t**)mem_malloc(MEM_IPUC_MODULE, sizeof(void*)*slist->count);
    if (!p_temp_ipuc_array)
    {
        return CTC_E_NO_MEMORY;
    }

    ctc_avl_create(&sort_tree, 0, _sys_lpm_ipaddr_cmp);

    CTC_SLIST_LOOP(slist, pnext)
    {
        pending_node = (sys_ipuc_pending_route_t*)pnext;
        ctc_avl_insert(sort_tree, &pending_node->sys_param.param);
    }

    ctc_avl_traverse2(sort_tree, _sys_lpm_ipaddr_traverse, p_temp_ipuc_array, &array_idx);

    ctc_avl_tree_free(&sort_tree, NULL);
    i = 0;
    cnt = slist->count;
    CTC_SLIST_LOOP(slist, pnext)
    {
        pending_node = (sys_ipuc_pending_route_t*)pnext;
        p_param = &pending_node->sys_param.param;

        if (p_param->ip_ver == CTC_IP_VER_4)
        {

            if ( p_param->ip.ipv4 != p_temp_ipuc_array[i]->ip.ipv4)
            {
                break;
            }
        }
        else
        {
            if ( (p_param->ip.ipv6[0] !=  p_temp_ipuc_array[i]->ip.ipv6[0]) || (p_param->ip.ipv6[1] !=  p_temp_ipuc_array[i]->ip.ipv6[1])
                || ( p_param->ip.ipv6[2] !=  p_temp_ipuc_array[i]->ip.ipv6[2]) || ( p_param->ip.ipv6[3] !=  p_temp_ipuc_array[i]->ip.ipv6[3]))
            {
                break;
            }
        }
        i++;

    }
    if (i == slist->count)
    {
        mem_free(p_temp_ipuc_array);
        return 1;
    }
    i = 0;
    CTC_SLIST_LOOP(slist, pnext)
    {
        pending_node = (sys_ipuc_pending_route_t*)pnext;
        p_param = &pending_node->sys_param.param;

        if (p_param->ip_ver == CTC_IP_VER_4)
        {

            if ( p_param->ip.ipv4 != p_temp_ipuc_array[i]->ip.ipv4)
            {
                break;
            }
        }
        else
        {
            if ( (p_param->ip.ipv6[0] !=  p_temp_ipuc_array[cnt - i - 1]->ip.ipv6[0]) || (p_param->ip.ipv6[1] !=  p_temp_ipuc_array[cnt - i -1]->ip.ipv6[1])
                || ( p_param->ip.ipv6[2] !=  p_temp_ipuc_array[cnt - i - 1]->ip.ipv6[2]) || ( p_param->ip.ipv6[3] !=  p_temp_ipuc_array[cnt - i -1]->ip.ipv6[3]))
            {
                break;
            }
        }
        i++;

    }
    mem_free(p_temp_ipuc_array);
    if (i == cnt)
    {
        return -1;
    }
    else
    {
        return 0;
    }

}
/*
0 add node; 1 get and del node;
*/
int32
_sys_usw_ipuc_pop_pending_list(uint8 lchip, ctc_slist_t* slist,  ctc_slistnode_t* node, ctc_slistnode_t** head)
{

    sal_systime_t current_tv;

    SYS_LIST_LOCK;
    sal_gettime(&current_tv);
    p_usw_ipuc_master[lchip]->last_tv = current_tv;
    if (p_usw_ipuc_master[lchip]->alpm_async == 2 )
    {
        uint8 bkt = 0xff;
        uint32 node_cnt = 0;

        if (p_usw_ipuc_master[lchip]->cbkt_status[0] == 2)
        {
            bkt = 0;
        }
        else  if(p_usw_ipuc_master[lchip]->cbkt_status[1] == 2 )
        {
            bkt = 1;
        }
        if (bkt != 0xFF)
        {
            *head =  ctc_hash_lookup3(p_usw_ipuc_master[lchip]->cbkt[bkt], &p_usw_ipuc_master[lchip]->cbkt_idx[bkt], &node_cnt);

            if (*head  == NULL)
            {
                p_usw_ipuc_master[lchip]->cbkt_status[bkt] = 0;
                p_usw_ipuc_master[lchip]->cbkt_idx[bkt] = 0;
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "~~~~~~ pop clear hash bucket cache cbuket %d cnt :%u  ~~~~~~\n ", bkt, p_usw_ipuc_master[lchip]->cbkt[bkt]->count);
            }
            else
            {
                p_usw_ipuc_master[lchip]->cbkt_idx[bkt] += ((node_cnt > 1) ? 0 : 1);
                ctc_hash_remove(p_usw_ipuc_master[lchip]->cbkt[bkt], *head);
                sal_sem_give(p_usw_ipuc_master[lchip]->alpm_sem);
            }
        }
    }
    else
    {
        *head = CTC_SLISTHEAD(p_usw_ipuc_master[lchip]->pending_route_list);
        ctc_slist_delete_node(p_usw_ipuc_master[lchip]->pending_route_list, *head);
    }
    SYS_LIST_UNLOCK;
    return CTC_E_NONE;
}



/*
0 add node; 1 get and del node;
*/
int32
_sys_usw_ipuc_push_pending_list(uint8 lchip, ctc_slist_t* slist,  ctc_slistnode_t* node, ctc_slistnode_t** head)
{
    uint32 deta_ts = 0;
    uint8 i = 0;
    sal_systime_t current_tv;

    if (NULL == slist)
    {
        return 0;
    }
    sal_gettime(&current_tv);
    deta_ts = current_tv.tv_sec - p_usw_ipuc_master[lchip]->last_tv.tv_sec;
    p_usw_ipuc_master[lchip]->last_tv = current_tv;

    if(deta_ts > 10) /*10s*/
    {
      p_usw_ipuc_master[lchip]->aid_end =0;
    }

    /*order detect ,and choose the right asynchronous mode*/
    if (p_usw_ipuc_master[lchip]->aid_en
        &&!p_usw_ipuc_master[lchip]->aid_end)
    {
        if (slist->count < p_usw_ipuc_master[lchip]->aid_list_len )
        {

            p_usw_ipuc_master[lchip]->alpm_async = 3;
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ip address: 0x%x\n", ((sys_ipuc_pending_route_t*)(node))->sys_param.param.ip.ipv4);
            ctc_slist_add_tail(slist, node);
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "order detecting , alpm_async = 3, slist cnt: %u  ~~~~~~\n ", slist->count);
        }
        else
        {
            int32 order_mode = 0;
            struct ctc_slistnode_s*  pnext   = NULL;

            ctc_slist_add_tail(slist, node);
            order_mode = _sys_lpm_judge_order(slist);
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "detect  order_mode = %d  ~~~~~~\n ", order_mode);

            p_usw_ipuc_master[lchip]->aid_end  = 1;
            if (order_mode == 1 || order_mode == -1)
            {
                p_usw_ipuc_master[lchip]->alpm_async = 2;
                p_usw_ipuc_master[lchip]->cbkt_status[0] = 1 ;
                CTC_SLIST_LOOP(slist, pnext)
                {
                    ctc_hash_insert(p_usw_ipuc_master[lchip]->cbkt[0], (sys_ipuc_pending_route_t*)pnext);
                }
                ctc_slist_delete_all_node(slist);
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "alpm_async = 2  ~~~~~~\n ");
            }
            else
            {
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, ":alpm_async = 1  ~~~~~~\n ");
                p_usw_ipuc_master[lchip]->alpm_async = 1;
                for (i = 0; i < slist->count; i++)
                {
                    sal_sem_give(p_usw_ipuc_master[lchip]->alpm_sem);
                }
            }
        }
        return 0;
    }

    SYS_LIST_LOCK;
        if (p_usw_ipuc_master[lchip]->alpm_async == 2 )
        {
            if ((p_usw_ipuc_master[lchip]->cbkt_status[0] == 0 || p_usw_ipuc_master[lchip]->cbkt_status[0] == 1)
                &&  (p_usw_ipuc_master[lchip]->cbkt_status[1] != 1))
            {
                ctc_hash_insert(p_usw_ipuc_master[lchip]->cbkt[0], node);
                p_usw_ipuc_master[lchip]->cbkt_status[0] = 1;
                if ((p_usw_ipuc_master[lchip]->cbkt[0]->count >=16384) && (p_usw_ipuc_master[lchip]->cbkt_status[1] == 0))
                {
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"~~~~~~  cbuket 0 done,cnt :%u  ~~~~~~\n ",p_usw_ipuc_master[lchip]->cbkt[0]->count);
                    p_usw_ipuc_master[lchip]->cbkt_status[0] =  2;
                    sal_sem_give(p_usw_ipuc_master[lchip]->alpm_sem);
                }

            }
            else
            {
                ctc_hash_insert(p_usw_ipuc_master[lchip]->cbkt[1], node);
                p_usw_ipuc_master[lchip]->cbkt_status[1] = 1;
                if ((p_usw_ipuc_master[lchip]->cbkt[1]->count >= 16384) && (p_usw_ipuc_master[lchip]->cbkt_status[0] == 0))
                {
                    p_usw_ipuc_master[lchip]->cbkt_status[1] =  2;
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"~~~~~~  cbuket 1 done cnt :%u  ~~~~~~\n ",p_usw_ipuc_master[lchip]->cbkt[1]->count);
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"~~~~~~  cbuket 0 cnt :%u  ~~~~~~\n ",p_usw_ipuc_master[lchip]->cbkt[0]->count);
                    sal_sem_give(p_usw_ipuc_master[lchip]->alpm_sem);
                }
            }

        }
        else
        {

            if (slist->count < p_usw_ipuc_master[lchip]->pending_list_len)
            {
                ctc_slist_add_tail(slist, node);
                sal_sem_give(p_usw_ipuc_master[lchip]->alpm_sem);
            }
            else
            {
                if(((sys_ipuc_pending_route_t*)(node))->operation == 0)
                {
                    mem_free(((sys_ipuc_pending_route_t*)(node))->sys_param.info);
                }
                SYS_LIST_UNLOCK;
                return CTC_E_NOT_READY;
            }
        }

     SYS_LIST_UNLOCK;
    return CTC_E_NONE;
}

int32
_sys_usw_ipuc_alpm_process(uint8 lchip, sys_ipuc_pending_route_t* p_pending_route)
{
    uint8 ip_ver = 0;
    int32 ret = CTC_E_NONE;
    uint8 auto_frag = 0;

    if (p_pending_route->operation != 0)
    {
        if (p_usw_ipuc_master[lchip]->alpm_async)
        {
            SYS_IPUC_LOCK;
            _sys_usw_ipuc_db_lookup(lchip, &(p_pending_route->sys_param));
            SYS_IPUC_UNLOCK;
            if(p_pending_route->sys_param.info == NULL)
            {
                p_pending_route->ret = CTC_E_NOT_EXIST;
                mem_free(p_pending_route);
                return CTC_E_NOT_EXIST;
            }
        }
        else
        {
            if (p_pending_route->sys_param.info == NULL)
            {
                return CTC_E_NOT_EXIST;
            }
        }
    }
    if (CTC_FLAG_ISSET(p_pending_route->sys_param.info->route_flag, SYS_IPUC_FLAG_IS_IPV6))
    {
        ip_ver = CTC_IP_VER_6;
    }
    else
    {
        ip_ver = CTC_IP_VER_4;
    }
    if (p_pending_route->operation == 0)/*add*/
    {
        if (p_pending_route->sys_param.info->route_opt == DO_ALPM)
        {
            ret = MCHIP_API(lchip)->ipuc_alpm_add(lchip, &(p_pending_route->sys_param), p_pending_route->sys_param.info->ad_index, NULL);
            if(CTC_E_NO_RESOURCE == ret)
            {
                if (DRV_IS_TSINGMA(lchip) && MCHIP_API(lchip)->ipuc_alpm_set_fragment_status && MCHIP_API(lchip)->ipuc_alpm_get_fragment_auto_enable &&
                    MCHIP_API(lchip)->ipuc_alpm_arrange_fragment)
                {
                    MCHIP_API(lchip)->ipuc_alpm_get_fragment_auto_enable(lchip, &auto_frag);
                    if (auto_frag)
                    {
                        ret = MCHIP_API(lchip)->ipuc_alpm_merge(lchip, p_pending_route->sys_param.info->vrf_id, ip_ver);
                        if (ret)
                        {
                            MCHIP_API(lchip)->ipuc_alpm_set_fragment_status(lchip, ip_ver, MERGE_NO_RESOURCE);
                        }
                        else
                        {
                            ret = MCHIP_API(lchip)->ipuc_alpm_add(lchip, &(p_pending_route->sys_param), p_pending_route->sys_param.info->ad_index, NULL);

                        }
                    }
                }
            }
            if (!ret)
            {
                if (p_usw_ipuc_master[lchip]->alpm_async)
                {
                    SYS_IPUC_LOCK;
                }
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_ALPM, 0);
                if (p_usw_ipuc_master[lchip]->alpm_async)
                {
                    SYS_IPUC_UNLOCK;
                }
            }
        }
        else
        {
            ret = sys_usw_ipuc_add_tcam(lchip, &(p_pending_route->sys_param));
            if (!ret && CTC_FLAG_ISSET(p_pending_route->sys_param.info->route_flag, SYS_IPUC_FLAG_HASH_CONFLICT))
            {
                if (p_usw_ipuc_master[lchip]->alpm_async)
                {
                    SYS_IPUC_LOCK;
                }
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_CONFLICT_TCAM, 0);
                if (p_usw_ipuc_master[lchip]->alpm_async)
                {
                    SYS_IPUC_UNLOCK;
                }
            }
        }
        if (ret == CTC_E_NONE)
        {
            if (!p_usw_ipuc_master[lchip]->alpm_async)
            {
                SYS_IPUC_UNLOCK;
            }
            ret = _sys_usw_ipuc_add_ending(lchip, &(p_pending_route->sys_param));
            if (!p_usw_ipuc_master[lchip]->alpm_async)
            {
                SYS_IPUC_LOCK;
            }
        }
    }
    else if (p_pending_route->operation == 1)/*del*/
    {
        if (p_pending_route->sys_param.info->route_opt == DO_ALPM)
        {
            ret = MCHIP_API(lchip)->ipuc_alpm_del(lchip, &(p_pending_route->sys_param));
            if (!ret)
            {
                if (p_usw_ipuc_master[lchip]->alpm_async)
                {
                    SYS_IPUC_LOCK;
                }
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_ALPM, 1);
                if (p_usw_ipuc_master[lchip]->alpm_async)
                {
                    SYS_IPUC_UNLOCK;
                }
            }
        }
        else
        {
            ret = sys_usw_ipuc_del_tcam(lchip, &(p_pending_route->sys_param));
            if (!ret && CTC_FLAG_ISSET(p_pending_route->sys_param.info->route_flag, SYS_IPUC_FLAG_HASH_CONFLICT))
            {
                if (p_usw_ipuc_master[lchip]->alpm_async)
                {
                    SYS_IPUC_LOCK;
                }
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_CONFLICT_TCAM, 1);
                if (p_usw_ipuc_master[lchip]->alpm_async)
                {
                    SYS_IPUC_UNLOCK;
                }
            }
        }

        if (ret == CTC_E_NONE)
        {
            if (!p_usw_ipuc_master[lchip]->alpm_async)
            {
                SYS_IPUC_UNLOCK;
            }
            ret = _sys_usw_ipuc_remove_ending(lchip, &(p_pending_route->sys_param));
            if (!p_usw_ipuc_master[lchip]->alpm_async)
            {
                SYS_IPUC_LOCK;
            }
        }
    }
    else/*update*/
    {
        if (p_pending_route->sys_param.info->route_opt == DO_ALPM)
        {
            ret = MCHIP_API(lchip)->ipuc_alpm_update(lchip, &(p_pending_route->sys_param), p_pending_route->sys_param.info->ad_index);
        }
        else
        {
            ret = sys_usw_ipuc_update_tcam(lchip, &(p_pending_route->sys_param));
        }

        if (ret == CTC_E_NONE && p_usw_ipuc_master[lchip]->alpm_async)
        {
            ret = _sys_usw_ipuc_update_ending(lchip, &(p_pending_route->old_info));
        }
    }
    return ret;
}

STATIC void
_sys_usw_ipuc_alpm_thread(void *param)
{
    int32 ret = 0;
    uint8 lchip = (uintptr)param;
    uint8 db_size = 0;
    ctc_slistnode_t* head = NULL;
    ctc_ipuc_param_t ctc_param = {0};
    sys_ipuc_pending_route_t* p_pending_route = NULL;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    while(1)
    {
        ret = sal_sem_take(p_usw_ipuc_master[lchip]->alpm_sem, SAL_SEM_FOREVER);
        if (0 != ret)
        {
            continue;
        }
        SYS_LCHIP_CHECK_ACTIVE_IN_THREAD(lchip);
        if(p_usw_ipuc_master[lchip]->alpm_thread_disable)
        {
            return;
        }
        _sys_usw_ipuc_pop_pending_list(lchip, p_usw_ipuc_master[lchip]->pending_route_list,  NULL, &head);
        p_pending_route = (sys_ipuc_pending_route_t*)head;

        if (p_pending_route == NULL)
        {
            continue;
        }
        if(p_pending_route->operation == 0)
        {
            ret = _sys_usw_ipuc_spec_check(lchip, &(p_pending_route->sys_param.param));
            if (ret)
            {
                if(NULL != p_usw_ipuc_master[lchip]->callback)
                {
                    _sys_usw_ipuc_info_to_param(lchip, &ctc_param, p_pending_route->sys_param.info);
                    p_usw_ipuc_master[lchip]->callback(lchip, &ctc_param, p_usw_ipuc_master[lchip]->user_data);
                }
                mem_free(p_pending_route->sys_param.info);
                if (p_pending_route->sys_param.p_dsipda)
                {
                    mem_free(p_pending_route->sys_param.p_dsipda);
                }
                mem_free(p_pending_route);
                continue;
            }
        }
        ret = _sys_usw_ipuc_alpm_process(lchip, p_pending_route);
        if(CTC_E_NOT_EXIST == ret)
        {
            continue;
        }
        if (p_usw_ipuc_master[lchip]->alpm_async)
        {
            if(ret)
            {
                if(NULL != p_usw_ipuc_master[lchip]->callback)
                {
                    _sys_usw_ipuc_info_to_param(lchip, &ctc_param, p_pending_route->sys_param.info);
                    p_usw_ipuc_master[lchip]->callback(lchip, &ctc_param, p_usw_ipuc_master[lchip]->user_data);
                }
                if(p_pending_route->operation == 0)
                {
                    _sys_usw_ipuc_unbuild_ipda(lchip, &p_pending_route->sys_param);
                    mem_free(p_pending_route->sys_param.info);
                }
                if(p_pending_route->operation == 2)
                {
                    _sys_usw_ipuc_unbuild_ipda(lchip, &p_pending_route->sys_param);
                    if(CTC_FLAG_ISSET(p_pending_route->old_info.route_flag, SYS_IPUC_FLAG_IS_BIND_NH))
                    {
                        db_size = (p_pending_route->sys_param.param.ip_ver== CTC_IP_VER_4) ? (sizeof(ipv6_addr_t) - sizeof(ip_addr_t)) : 0;
                        db_size = sizeof(sys_ipuc_info_t) - db_size;
                        sal_memcpy(p_pending_route->sys_param.info, &(p_pending_route->old_info), db_size);
                        _sys_usw_ipuc_bind_nexthop(lchip, &p_pending_route->sys_param, 1);
                    }
                }
            }
            if (p_pending_route->sys_param.p_dsipda)
            {
                mem_free(p_pending_route->sys_param.p_dsipda);
            }
            mem_free(p_pending_route);
        }
    }
}

#define ___________IPUC_OUTER_FUNCTION________________________
#define __0_IPUC_API__
int32
_sys_usw_ipuc_add_default_entry(uint8 lchip, uint8 ip_ver, uint16 vrfid, uint32 nh_id, uint8 by_user)
{
    ctc_ipuc_param_t ctc_param = {0};
    sys_ipuc_param_t sys_param = {0};
    sys_ipuc_info_t  ipuc_info = {0};

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    ctc_param.masklen = 0;
    ctc_param.nh_id = nh_id;
    ipuc_info.route_flag |= (ip_ver == CTC_IP_VER_6) ? SYS_IPUC_FLAG_IS_IPV6 : 0;
    ipuc_info.route_flag |= SYS_IPUC_FLAG_DEFAULT | SYS_IPUC_FLAG_RPF_CHECK;
    ipuc_info.ad_index = p_usw_ipuc_master[lchip]->default_base[ip_ver] + (vrfid & 0x3FFF);

    sys_param.param = ctc_param;
    sys_param.info = &ipuc_info;

    CTC_ERROR_RETURN(_sys_usw_ipuc_add_rpf(lchip, &sys_param));

    CTC_ERROR_RETURN(_sys_usw_ipuc_write_ipda(lchip, &sys_param, by_user));

    return CTC_E_NONE;
}

int32
sys_usw_ipuc_add_default_entry(uint8 lchip, uint8 ip_ver, uint32 nh_id)
{
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    SYS_IPUC_INIT_CHECK;
    CTC_IP_VER_CHECK(ip_ver);
    SYS_IP_CHECK_VERSION_ENABLE(ip_ver);
    if(p_usw_ipuc_master[lchip]->rchain_en)
    {
        lchip = p_usw_ipuc_master[lchip]->rchain_tail;
    }

    CTC_ERROR_RETURN(_sys_usw_ipuc_add_default_entry(lchip, ip_ver, 0, nh_id, 1));

    p_usw_ipuc_master[lchip]->default_nhid[ip_ver] = nh_id;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ipuc_update(uint8 lchip,
                sys_ipuc_param_t *p_sys_param)
{
    sys_ipuc_info_t* p_new_info = p_sys_param->info;
    sys_ipuc_info_t old_info = {0};
    int32 ret = CTC_E_NONE;
    uint8 old_bind_flag = 0;
    uint8 db_size = 0;
    sys_ipuc_param_t unbind_param;
    ctc_ipuc_param_t unbind_param_ctc;
    sys_ipuc_pending_route_t* p_pending_route = NULL;
    sys_ipuc_pending_route_t  route_sync;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&unbind_param, 0, sizeof(unbind_param));
    sal_memset(&unbind_param_ctc, 0, sizeof(unbind_param_ctc));

     /* save old rpf info and ad index */
    db_size = (p_sys_param->param.ip_ver == CTC_IP_VER_4) ? (sizeof(ipv6_addr_t) - sizeof(ip_addr_t)) : 0;
    db_size = sizeof(sys_ipuc_info_t) - db_size;
    sal_memcpy(&old_info, p_new_info, db_size);

    /* 1. remove old ad resource */
    old_bind_flag = CTC_FLAG_ISSET(old_info.route_flag, SYS_IPUC_FLAG_IS_BIND_NH) ? 1 : 0;

    unbind_param.info = p_sys_param->info;
    unbind_param.param = unbind_param_ctc;
    unbind_param_ctc.nh_id = old_info.nh_id;

    if (old_bind_flag)
    {
        _sys_usw_ipuc_bind_nexthop(lchip, &unbind_param, 0);
    }

    _sys_usw_ipuc_param_to_info(lchip, &p_sys_param->param, p_new_info);

    /* 2. build and write ipda, include ad profile */
    CTC_ERROR_GOTO(_sys_usw_ipuc_build_ipda(lchip, p_sys_param), ret, error0);
    CTC_ERROR_GOTO(_sys_usw_ipuc_write_ipda(lchip, p_sys_param , 1), ret, error1);

    /* 3. write key */
    if(p_new_info->ad_index != old_info.ad_index || p_usw_ipuc_master[lchip]->rchain_en)
    {
        if (!p_usw_ipuc_master[lchip]->alpm_async)
        {
            p_pending_route = &route_sync;
        }
        else
        {
            p_pending_route = mem_malloc(MEM_IPUC_MODULE,  sizeof(sys_ipuc_pending_route_t));
            if (p_pending_route == NULL)
            {
                ret = CTC_E_NO_MEMORY;
                goto error1;
            }
        }

        sal_memset(p_pending_route, 0 , sizeof(sys_ipuc_pending_route_t));
        p_pending_route->sys_param = *p_sys_param;
        p_pending_route->operation = 2;
        sal_memcpy(&p_pending_route->old_info, &old_info, db_size);
        if (p_new_info->route_opt == DO_HASH)
        {
            CTC_ERROR_GOTO(_sys_usw_ipuc_process_hash_key(lchip, p_sys_param, DO_UPDATE), ret, error1);
            _sys_usw_ipuc_update_ending(lchip, &old_info);
        }
        else if (!p_usw_ipuc_master[lchip]->alpm_async)
        {
            ret = _sys_usw_ipuc_alpm_process(lchip, p_pending_route);
            if (ret != CTC_E_NONE)
            {
                _sys_usw_ipuc_unbuild_ipda(lchip, p_sys_param);
                if (old_bind_flag)
                {
                    sal_memcpy(p_new_info, &old_info, db_size);
                    _sys_usw_ipuc_bind_nexthop(lchip, p_sys_param, 1);
                }
            }
        }
        else
        {
            CTC_ERROR_GOTO(_sys_usw_ipuc_push_pending_list(lchip, p_usw_ipuc_master[lchip]->pending_route_list,  &p_pending_route->node_head, NULL),ret,error1);
            return ret;
        }

    }
    if (!p_usw_ipuc_master[lchip]->alpm_async)
    {
        _sys_usw_ipuc_update_ending(lchip, &old_info);
    }
    if (p_sys_param->p_dsipda)
    {
        mem_free(p_sys_param->p_dsipda);
    }

    return CTC_E_NONE;

error1:
    if(p_pending_route && p_usw_ipuc_master[lchip]->alpm_async)
    {
        mem_free(p_pending_route);
    }
    _sys_usw_ipuc_unbuild_ipda(lchip, p_sys_param);

error0:
    sal_memcpy(p_new_info, &old_info, db_size);
    if (old_bind_flag)
    {
        _sys_usw_ipuc_bind_nexthop(lchip, p_sys_param, 1);
    }

    if (p_sys_param->p_dsipda)
    {
        mem_free(p_sys_param->p_dsipda);
    }
    return ret;
}

int32
sys_usw_ipuc_add(uint8 lchip, ctc_ipuc_param_t* p_ctc_param_info)
{
    int32 ret = CTC_E_NONE;
    uint16 db_size = 0;
    uint8 opt = 0;
    uint8 ip_ver = p_ctc_param_info->ip_ver;
    sys_ipuc_param_t sys_param = {0};
    sys_ipuc_pending_route_t* p_pending_route = NULL;
    sys_ipuc_pending_route_t  route_sync;
    DsIpDa_m* p_dsipda = NULL;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_IPUC_INIT_CHECK;
    if(p_usw_ipuc_master[lchip]->rchain_en && lchip != 0)
    {
        return ret;
    }

    sal_memcpy(&sys_param.param, p_ctc_param_info, sizeof(ctc_ipuc_param_t));

    /* 1. param check and debug out */
    CTC_ERROR_RETURN(_sys_usw_ipuc_check(lchip, &sys_param.param));
    SYS_IP_FUNC_DBG_DUMP((&sys_param.param));

    /* para check */
    CTC_ERROR_RETURN(_sys_usw_ipuc_flag_check(lchip, &sys_param.param, 1));
    SYS_IPUC_LOCK;
    if (p_usw_ipuc_master[lchip]->rchain_en)
    {
        p_dsipda = mem_malloc(MEM_IPUC_MODULE, sizeof(DsIpDa_m));
        if (p_dsipda == NULL)
        {
            CTC_RETURN_IPUC_UNLOCK(CTC_E_NO_MEMORY);
        }
        sal_memset(p_dsipda, 0, sizeof(DsIpDa_m));
        sys_param.p_dsipda = p_dsipda;
    }
    /* 2. lookup sw node */
    if (IS_MAX_MASKLEN(ip_ver, sys_param.param.masklen) && p_usw_ipuc_master[lchip]->host_lpm_mode)
    {
        CTC_ERROR_GOTO((_sys_usw_ipuc_host_pre_process(lchip, &sys_param)), ret, error0);
    }

    CTC_ERROR_GOTO((_sys_usw_ipuc_db_lookup(lchip, &sys_param)), ret, error0);
    if (sys_param.info)
    {
        sys_param.is_update = 1;
        CTC_ERROR_GOTO((_sys_usw_ipuc_update(lchip, &sys_param)), ret, error0);
        if (sys_param.p_dsipda)
        {
            mem_free(sys_param.p_dsipda);
        }
        CTC_RETURN_IPUC_UNLOCK(CTC_E_NONE);
    }

    if (!p_usw_ipuc_master[lchip]->alpm_async)
    {
        CTC_ERROR_GOTO((_sys_usw_ipuc_spec_check(lchip, &sys_param.param)), ret, error0);
    }

    /* 3. Judge operation */
    CTC_ERROR_GOTO((_sys_usw_ipuc_get_store_position(lchip, &sys_param, &opt)), ret, error0);

    db_size = (ip_ver == CTC_IP_VER_4) ? (sizeof(ipv6_addr_t) - sizeof(ip_addr_t)) : 0;
    db_size = sizeof(sys_ipuc_info_t) - db_size;
    sys_param.info = mem_malloc(MEM_IPUC_MODULE,  db_size);
    if (NULL == sys_param.info)
    {
        CTC_ERROR_GOTO((CTC_E_NO_MEMORY), ret, error0);
    }
    sal_memset(sys_param.info, 0, db_size);

    _sys_usw_ipuc_param_to_info(lchip, &sys_param.param, sys_param.info);
    sys_param.info->route_opt = opt;
    if (sys_param.hash_conflict == 1)
    {
        CTC_SET_FLAG(sys_param.info->route_flag, SYS_IPUC_FLAG_HASH_CONFLICT);
    }

    /* 3. build ipda, include lpm ad profile */
    CTC_ERROR_GOTO(_sys_usw_ipuc_build_ipda(lchip, &sys_param), ret, error1);

    /* 4. write ipda */
    CTC_ERROR_GOTO(_sys_usw_ipuc_write_ipda(lchip, &sys_param, 1), ret, error2);

    /* 5. build key index */
    if (sys_param.info->route_opt == DO_HASH)     /* for host route, add it to hash table directly */
    {
        ret = _sys_usw_ipuc_process_hash_key(lchip, &sys_param, DO_ADD);
        if (ret)
        {
            if (sys_param.param.l4_dst_port ||
                    !CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_TCAM_LOOKUP))   /* NAPT or there's no tcam to solve conflict */
            {
                goto error2;
            }
            if(ret == CTC_E_HASH_CONFLICT && !p_usw_ipuc_master[lchip]->arc[ip_ver])
            {
                goto error2;
            }
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add host route failed ret: %d. Than write to tcam. \r\n", ret);

            sys_param.info->route_opt = DO_TCAM;
            CTC_SET_FLAG(sys_param.info->route_flag, SYS_IPUC_FLAG_HASH_CONFLICT);
        }
        else
        {
            if (sys_param.param.l4_dst_port)
            {
                p_usw_ip_tunnel_master[lchip]->napt_hash_count++;
            }
            else
            {
                sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_HASH, 0);
            }

            SYS_IPUC_UNLOCK;
            _sys_usw_ipuc_add_ending(lchip, &sys_param);
            if (sys_param.p_dsipda)
            {
                mem_free(sys_param.p_dsipda);
            }
            return CTC_E_NONE;
        }
    }

    if(sys_param.info->route_opt != DO_HASH)
    {
        if (!p_usw_ipuc_master[lchip]->alpm_async)
        {
            p_pending_route = &route_sync;
        }
        else
        {
            p_pending_route = mem_malloc(MEM_IPUC_MODULE,  sizeof(sys_ipuc_pending_route_t));
            if (p_pending_route == NULL)
            {
                ret = CTC_E_NO_MEMORY;
                goto error2;
            }
        }

        sal_memset(p_pending_route, 0 , sizeof(sys_ipuc_pending_route_t));
        sal_memcpy(&p_pending_route->sys_param, &sys_param, sizeof(sys_ipuc_param_t));
        //-p_pending_route->sys_param = sys_param;
        p_pending_route->operation = 0;

        if (!p_usw_ipuc_master[lchip]->alpm_async)
        {
            ret =_sys_usw_ipuc_alpm_process(lchip, p_pending_route);
        }
        else
        {
            ret = _sys_usw_ipuc_push_pending_list(lchip, p_usw_ipuc_master[lchip]->pending_route_list,  &p_pending_route->node_head, NULL);
        }

        if(ret != CTC_E_NONE)
        {
            goto error2;
        }
        SYS_IPUC_UNLOCK;

    }

    if (sys_param.p_dsipda)
    {
        mem_free(sys_param.p_dsipda);
    }
    return ret;

error2:
    if(p_pending_route && p_usw_ipuc_master[lchip]->alpm_async)
    {
        mem_free(p_pending_route);
    }

    if (!p_usw_ipuc_master[lchip]->rchain_en)
    {
        _sys_usw_ipuc_unbuild_ipda(lchip, &sys_param);
    }
error1:
    mem_free(sys_param.info);
error0:
    if (sys_param.p_dsipda)
    {
        mem_free(sys_param.p_dsipda);
    }
    CTC_RETURN_IPUC_UNLOCK(ret);
}

/**
 @brief function of remove ip route

 @param[in] p_ipuc_param, parameters used to remove ip route

 @return CTC_E_XXX
 */
int32
sys_usw_ipuc_remove(uint8 lchip, ctc_ipuc_param_t* p_ctc_param_info)
{
    uint8 ip_ver = p_ctc_param_info->ip_ver;
    sys_ipuc_param_t sys_param = {0};
    sys_ipuc_pending_route_t* p_pending_route = NULL;
    sys_ipuc_pending_route_t route_sync;
    int32 ret = CTC_E_NONE;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_IPUC_INIT_CHECK;

    sal_memcpy(&sys_param.param, p_ctc_param_info, sizeof(ctc_ipuc_param_t));

    /* 1. param check and debug out */
    CTC_ERROR_RETURN(_sys_usw_ipuc_check(lchip, &sys_param.param));
    SYS_IP_FUNC_DBG_DUMP((&sys_param.param));

    /* para check */
    CTC_ERROR_RETURN(_sys_usw_ipuc_flag_check(lchip, &sys_param.param, 0));

    SYS_IPUC_LOCK;
    _sys_usw_ipuc_db_lookup(lchip, &sys_param);
    if (!p_usw_ipuc_master[lchip]->alpm_async)
    {
        p_pending_route = &route_sync;
    }
    else
    {
        p_pending_route = mem_malloc(MEM_IPUC_MODULE,  sizeof(sys_ipuc_pending_route_t));
        if (p_pending_route == NULL)
        {
            ret = CTC_E_NO_MEMORY;
            CTC_RETURN_IPUC_UNLOCK (ret);
        }
    }
    sal_memset(p_pending_route, 0 , sizeof(sys_ipuc_pending_route_t));
    sal_memcpy(&p_pending_route->sys_param, &sys_param, sizeof(sys_param));
    p_pending_route->operation = 1;
    if ( (IS_MAX_MASKLEN(ip_ver, sys_param.param.masklen) &&
                    !CTC_FLAG_ISSET(sys_param.param.route_flag, CTC_IPUC_FLAG_HOST_USE_LPM) &&
                    CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_HASH_LOOKUP)) ||
                    sys_param.param.l4_dst_port)
    {
        /*do HOST remove*/
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "remove route from hash\n");
        /* lookup for ipuc entrise */
        if (NULL == sys_param.info || CTC_FLAG_ISSET(sys_param.info->route_flag, SYS_IPUC_FLAG_HASH_CONFLICT)
            || CTC_FLAG_ISSET(sys_param.info->route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE))
        {
            if (!p_usw_ipuc_master[lchip]->alpm_async)
            {
                goto DO_PROCESS;
            }
            else
            {
                goto DO_THREAD;
            }
        }

        ret = _sys_usw_ipuc_process_hash_key(lchip, &sys_param, DO_DEL);
        if (ret)
        {
            if (p_usw_ipuc_master[lchip]->alpm_async)
            {
                mem_free(p_pending_route);
            }
            SYS_IPUC_UNLOCK;
            return ret;
        }

        if (sys_param.param.l4_dst_port)
        {
            p_usw_ip_tunnel_master[lchip]->napt_hash_count--;
        }
        else
        {
            sys_usw_ipuc_update_route_stats(lchip,  ip_ver, SYS_IPUC_HASH, 1);
        }

        SYS_IPUC_UNLOCK;
        ret = _sys_usw_ipuc_remove_ending(lchip, &sys_param);
    }
    else
    {
        if (!p_usw_ipuc_master[lchip]->alpm_async)
        {
DO_PROCESS:
            ret = _sys_usw_ipuc_alpm_process(lchip, p_pending_route);
            SYS_IPUC_UNLOCK;
            return ret;
        }
        else
        {
DO_THREAD:
            ret = _sys_usw_ipuc_push_pending_list(lchip, p_usw_ipuc_master[lchip]->pending_route_list,  &p_pending_route->node_head, NULL);
            if(ret)
            {
                mem_free(p_pending_route);
                SYS_IPUC_UNLOCK;
                return (ret);
            }
            SYS_IPUC_UNLOCK;
            return (ret);
        }
    }
    return(ret);
}

int32
sys_usw_ipuc_get(uint8 lchip, ctc_ipuc_param_t* p_ctc_param_info)
{
    int32 ret = CTC_E_NONE;
    sys_ipuc_param_t sys_param = {0};
    ipv6_addr_t ipv6;

    SYS_IPUC_INIT_CHECK;

    CTC_PTR_VALID_CHECK(p_ctc_param_info);
    sal_memset(&ipv6, 0, sizeof(ipv6_addr_t));
    if (p_ctc_param_info->ip_ver == CTC_IP_VER_6)
    {
        sal_memcpy(&ipv6, &p_ctc_param_info->ip.ipv6, sizeof(ipv6_addr_t));
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    CTC_ERROR_RETURN(_sys_usw_ipuc_check(lchip, p_ctc_param_info));
    SYS_IP_FUNC_DBG_DUMP(p_ctc_param_info);

    /* para check */
    CTC_ERROR_RETURN(_sys_usw_ipuc_flag_check(lchip, p_ctc_param_info, 0));
    sys_param.param = *p_ctc_param_info;

    SYS_IPUC_LOCK;
    CTC_ERROR_RETURN(_sys_usw_ipuc_db_lookup(lchip, &sys_param));
    if (NULL == sys_param.info)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
        ret = CTC_E_NOT_EXIST;
        goto error0;
    }

    sal_memset(p_ctc_param_info->rpf_port, 0, sizeof(uint32) * SYS_RPF_CHK_MODE_IFID);
    if (CTC_FLAG_ISSET(sys_param.info->route_flag, SYS_IPUC_FLAG_RPF_CHECK))
    {
        if (sys_param.info->rpf_mode == SYS_RPF_CHK_MODE_IFID)
        {
            p_ctc_param_info->rpf_port_num = 1;
            p_ctc_param_info->rpf_port[0] = sys_param.info->rpf_id;
        }
        else
        {
            uint8 loop = 0;
            uint8 valid = 0;
            uint32 cmd = 0;
            DsRpf_m ds_rpf;

            cmd = DRV_IOR(DsRpf_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, sys_param.info->rpf_id, cmd, &ds_rpf), ret, error0);

            for (loop = 0; loop < SYS_USW_MAX_RPF_IF_NUM; loop++)
            {
                valid = GetDsRpf(V, array_0_rpfIfIdValid_f+2*loop, &ds_rpf);
                if (0 == valid)
                {
                    break;
                }

                p_ctc_param_info->rpf_port[loop] = GetDsRpf(V, array_0_rpfIfId_f+2*loop, &ds_rpf);
                p_ctc_param_info->rpf_port_num++;
            }
        }
    }

    if (p_ctc_param_info->ip_ver == CTC_IP_VER_6)
    {
        sal_memcpy(&p_ctc_param_info->ip.ipv6, &ipv6, sizeof(ipv6_addr_t));
    }

    p_ctc_param_info->nh_id = sys_param.info->nh_id;
    p_ctc_param_info->stats_id = sys_param.info->stats_id;
    p_ctc_param_info->cid = sys_param.info->cid;
    p_ctc_param_info->l4_dst_port = sys_param.info->l4_dst_port;
    p_ctc_param_info->gport = sys_param.info->gport;

    p_ctc_param_info->route_flag = 0;
    SYS_IPUC_SYS2CTC_FLAG_CONVERT(p_ctc_param_info->route_flag, sys_param.info->route_flag);

    SYS_IPUC_UNLOCK;
    return CTC_E_NONE;

error0:
    if (p_ctc_param_info->ip_ver == CTC_IP_VER_6)
    {
        sal_memcpy(&p_ctc_param_info->ip.ipv6, &ipv6, sizeof(ipv6_addr_t));
    }
    SYS_IPUC_UNLOCK;

    return ret;
}

int32
sys_usw_ipuc_update_ipda(uint8 lchip, void* data, void* change_nh_param)
{
    sys_ipuc_param_t sys_param = {0};
    sys_ipuc_info_t* p_ipuc_info = (sys_ipuc_info_t*)data;
    sys_nh_info_dsnh_t* p_dsnh_info = change_nh_param;
    DsIpDa_m   dsipda;
    DsIpDa_m   old_dsipda;
    int32 ret = CTC_E_NONE;
    uint32 cmdr  = 0;
    uint32 cmdw  = 0;
    uint32 fwd_offset = 0;
    uint32 old_ad_idx = p_ipuc_info->ad_index;

    _sys_usw_ipuc_info_to_param(lchip, &sys_param.param, p_ipuc_info);
    sys_param.info = p_ipuc_info;

    if (p_usw_ipuc_master[lchip]->rchain_en)
    {
        lchip = p_ipuc_info->real_lchip;
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    sal_memset(&dsipda, 0, sizeof(dsipda));
    sal_memset(&old_dsipda, 0, sizeof(DsIpDa_m));

    /*do lock when update nexthop, p_dsnh_info is not NULL, otherwise don't lock when bind nexthop*/
    if (p_dsnh_info)
    {
        SYS_IPUC_LOCK;
    }

    cmdr = DRV_IOR(DsIpDa_t, DRV_ENTRY_FLAG);
    cmdw = DRV_IOW(DsIpDa_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, p_ipuc_info->ad_index, cmdr, &dsipda);

    sal_memcpy(&old_dsipda, &dsipda, sizeof(DsIpDa_m));

    if (!p_dsnh_info)
    {
        /*update ad from merge dsfwd to using dsfwd*/

        /*1. alloc dsfwd*/
        CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, p_ipuc_info->nh_id, &fwd_offset, 1));

        /*2. update ad*/
        if(DRV_IS_TSINGMA(lchip)|| DRV_IS_DUET2(lchip))
        {
            SetDsIpDa(V, nextHopPtrValid_f, &dsipda, 0);
            SetDsIpDa(V, adDestMap_f, &dsipda, 0);
            SetDsIpDa(V, adNextHopPtr_f, &dsipda, 0);
            SetDsIpDa(V, adNextHopExt_f, &dsipda, 0);
            SetDsIpDa(V, adLengthAdjustType_f, &dsipda, 0);
            SetDsIpDa(V, dsFwdPtrOrEcmpGroupId_f, &dsipda, fwd_offset);
        }
        CTC_UNSET_FLAG(p_ipuc_info->route_flag, SYS_IPUC_FLAG_MERGE_KEY);
        CTC_UNSET_FLAG(p_ipuc_info->route_flag, SYS_IPUC_FLAG_IS_BIND_NH);
    }
    else
    {
        sys_nh_info_dsnh_t dsnh_info;
        sal_memset(&dsnh_info, 0, sizeof(sys_nh_info_dsnh_t));
        if(DRV_IS_TSINGMA(lchip)|| DRV_IS_DUET2(lchip))
        {
            if (GetDsIpDa(V, nextHopPtrValid_f, &dsipda) == 0)
            {
                CTC_RETURN_IPUC_UNLOCK(CTC_E_NONE);
            }

        }
        sys_usw_nh_get_nhinfo(lchip, p_ipuc_info->nh_id, &dsnh_info, 1);
        p_dsnh_info->dsnh_offset = dsnh_info.dsnh_offset;
        CTC_ERROR_RETURN_IPUC_UNLOCK(_sys_usw_ipuc_build_ipda_nexthop(lchip, &sys_param, p_dsnh_info, &dsipda));
    }

    _sys_usw_ipuc_add_ad_profile(lchip, &sys_param, (void*)&dsipda, 1);

    DRV_IOCTL(lchip, p_ipuc_info->ad_index, cmdw, &dsipda);

    if (old_ad_idx != p_ipuc_info->ad_index)
    {
        /*need update hash key*/
        if (sys_param.info->route_opt == DO_HASH)
        {
            CTC_ERROR_GOTO(_sys_usw_ipuc_process_hash_key(lchip, &sys_param, DO_UPDATE), ret, error0);
        }
        else if (sys_param.info->route_opt == DO_ALPM)
        {
            CTC_PTR_VALID_CHECK(MCHIP_API(lchip)->ipuc_alpm_update);
            CTC_ERROR_GOTO(MCHIP_API(lchip)->ipuc_alpm_update(lchip, &sys_param, p_ipuc_info->ad_index), ret, error0);
        }
        else if (sys_param.info->route_opt == DO_TCAM)
        {
            /* write ipuc tcam key entry */
            CTC_ERROR_GOTO(sys_usw_ipuc_update_tcam(lchip, &sys_param), ret, error0);
        }
    }

    if (p_dsnh_info)
    {
        SYS_IPUC_UNLOCK;
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC update_da: p_ipuc_info->ad_index:0x%x \n", p_ipuc_info->ad_index);

    return CTC_E_NONE;

error0:
    _sys_usw_ipuc_add_ad_profile(lchip, &sys_param, (void*)&old_dsipda, 1);
    if (p_dsnh_info)
    {
        SYS_IPUC_UNLOCK;
    }

    return ret;
}

int32
sys_usw_ipuc_traverse(uint8 lchip, uint8 ip_ver, ctc_ipuc_traverse_fn  fn, void* data)
{
    hash_traversal_fn  fun = _sys_usw_ipuc_traverse_pre;
    sys_ipuc_traverse_t trav;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    SYS_IPUC_INIT_CHECK;
    SYS_IP_CHECK_VERSION_ENABLE(ip_ver);

    trav.data = data;
    trav.fn = fn;
    trav.lchip = lchip;
    if (NULL == fn)
    {
        return CTC_E_NONE;
    }

   return  _sys_usw_ipuc_db_traverse(lchip, ip_ver, fun, (void*)&trav);
}

int32
sys_usw_ipuc_host_ftm_cb(uint8 lchip, sys_ftm_specs_info_t* specs_info)
{
    LCHIP_CHECK(lchip);
    SYS_IPUC_INIT_CHECK;
    CTC_PTR_VALID_CHECK(specs_info);

    if(DRV_IS_DUET2(lchip))
    {
        specs_info->used_size = p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_HOST] +
                                              p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_HOST] * 2;
        return CTC_E_NONE;
    }

    if(specs_info->type == 0)
    {
        specs_info->used_size = p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_HOST];
    }
    else 
    {
        specs_info->used_size = p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_HOST] * 2;
    }

    return CTC_E_NONE;
}

int32
sys_usw_ipuc_alpm_ftm_cb(uint8 lchip, sys_ftm_specs_info_t* specs_info)
{
    LCHIP_CHECK(lchip);
    SYS_IPUC_INIT_CHECK;
    CTC_PTR_VALID_CHECK(specs_info);

    if(DRV_IS_DUET2(lchip))
    {
        specs_info->used_size = p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_ALPM] +
                                              p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_ALPM];
        return CTC_E_NONE;
    }

    if(specs_info->type == 0)
    {
        specs_info->used_size = p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_ALPM];
    }
    else 
    {
        specs_info->used_size = p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_ALPM];
    }

    return CTC_E_NONE;
}

int32
sys_usw_ipuc_set_global_property(uint8 lchip, ctc_ipuc_global_property_t* p_global_prop)
{
    uint32 cmdr = 0;
    uint32 cmdwl = 0;
    uint32 cmdwr = 0;
    ds_t ipe_lookup_route_ctl;
    ds_t ipe_route_ctl;
    void* vall = NULL;
    void* valr = NULL;
    uint32 value1 = 0;
    uint32 value2 = 0;
    uint64 cpu_mask = 0;
    uint8 lchip_num = 0;
    uint8 lchip_tmp = 0;

    SYS_IPUC_INIT_CHECK;
    CTC_PTR_VALID_CHECK(p_global_prop);

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (CTC_FLAG_ISSET(p_global_prop->valid_flag, CTC_IP_GLB_PROP_ASYNC_EN))
    {
        p_usw_ipuc_master[lchip]->alpm_async = p_global_prop->async_en;
        if(p_usw_ipuc_master[lchip]->alpm_async)
        {
            if (p_usw_ipuc_master[lchip]->p_alpm_task)
            {
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC async mode is enable, mode: %u\n", p_usw_ipuc_master[lchip]->alpm_async);
                return CTC_E_ENTRY_EXIST;
            }
            else
            {
                /*create alpm thread*/
                p_usw_ipuc_master[lchip]->alpm_thread_disable = 0;
                CTC_ERROR_RETURN(sal_sem_create(&p_usw_ipuc_master[lchip]->alpm_sem, 0));

                cpu_mask = sys_usw_chip_get_affinity(lchip, 1);
                CTC_ERROR_RETURN(sys_usw_task_create(lchip, &p_usw_ipuc_master[lchip]->p_alpm_task, "ipuc_alpm",
                                                     SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_HIGH, SAL_TASK_TYPE_ALPM, cpu_mask, _sys_usw_ipuc_alpm_thread, (void*)(uintptr)lchip));
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC async mode is enable, mode: %u\n", p_usw_ipuc_master[lchip]->alpm_async);
            }
        }
        else
        {
            if(p_usw_ipuc_master[lchip]->p_alpm_task)
            {
                p_usw_ipuc_master[lchip]->alpm_thread_disable = 1;
                sal_sem_give(p_usw_ipuc_master[lchip]->alpm_sem);
                sal_task_destroy(p_usw_ipuc_master[lchip]->p_alpm_task);
                CTC_ERROR_RETURN(sal_sem_destroy(p_usw_ipuc_master[lchip]->alpm_sem));
                p_usw_ipuc_master[lchip]->alpm_sem = NULL;
                p_usw_ipuc_master[lchip]->p_alpm_task = NULL;
            }
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC async mode is disable\n");
        }
    }

    if (CTC_FLAG_ISSET(p_global_prop->valid_flag, CTC_IP_GLB_PROP_PENDING_LIST_LEN))
    {
        p_usw_ipuc_master[lchip]->pending_list_len = p_global_prop->pending_list_len;
    }

    sal_memset(&ipe_lookup_route_ctl, 0, sizeof(ipe_lookup_route_ctl));
    sal_memset(&ipe_route_ctl, 0, sizeof(ipe_route_ctl));

    cmdr = DRV_IOR(IpeLookupRouteCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmdr, &ipe_lookup_route_ctl));

    cmdr = DRV_IOR(IpeRouteCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmdr, &ipe_route_ctl));

    if (CTC_FLAG_ISSET(p_global_prop->valid_flag, CTC_IP_GLB_PROP_V4_MARTIAN_CHECK_EN))
    {
        GetIpeLookupRouteCtl(A, martianCheckEn_f, ipe_lookup_route_ctl, &value1);
        value1 &= 0x1FFC0;  /* clear low 6 bit */
        value1 |= (p_global_prop->v4_martian_check_en ? 0xF : 0);
        SetIpeLookupRouteCtl(V, martianCheckEn_f, ipe_lookup_route_ctl, value1);
        value2 = value1 ? 0 : 1;
        SetIpeLookupRouteCtl(V, martianAddressCheckDisable_f, ipe_lookup_route_ctl, value2);

        cmdwl = DRV_IOW(IpeLookupRouteCtl_t, DRV_ENTRY_FLAG);
        vall = &ipe_lookup_route_ctl;
    }

    if (CTC_FLAG_ISSET(p_global_prop->valid_flag, CTC_IP_GLB_PROP_V6_MARTIAN_CHECK_EN))
    {
        GetIpeLookupRouteCtl(A, martianCheckEn_f, ipe_lookup_route_ctl, &value1);
        value1 &= 0x3F;    /* clear high 11 bit */
        value1 |= (p_global_prop->v6_martian_check_en) ? 0x1BFC0 : 0;
        SetIpeLookupRouteCtl(V, martianCheckEn_f, ipe_lookup_route_ctl, value1);
        value2 = value1 ? 0 : 1;
        SetIpeLookupRouteCtl(V, martianAddressCheckDisable_f, ipe_lookup_route_ctl, value2);

        cmdwl = DRV_IOW(IpeLookupRouteCtl_t, DRV_ENTRY_FLAG);
        vall = &ipe_lookup_route_ctl;
    }

    if (CTC_FLAG_ISSET(p_global_prop->valid_flag, CTC_IP_GLB_PROP_MCAST_MACDA_IP_UNMATCH_CHECK))
    {
        value1 = (p_global_prop->mcast_match_check_en) ? 0 : 1;
        SetIpeRouteCtl(V, mcastAddressMatchCheckDisable_f, ipe_route_ctl, value1);

        cmdwr = DRV_IOW(IpeRouteCtl_t, DRV_ENTRY_FLAG);
        valr = &ipe_route_ctl;
    }

    if (CTC_FLAG_ISSET(p_global_prop->valid_flag, CTC_IP_GLB_PROP_TTL_THRESHOLD))
    {
        SetIpeRouteCtl(V, ipTtlLimit_f, ipe_route_ctl, p_global_prop->ip_ttl_threshold);

        cmdwr = DRV_IOW(IpeRouteCtl_t, DRV_ENTRY_FLAG);
        valr = &ipe_route_ctl;
    }

    if (cmdwl != 0)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmdwl, vall));
    }

    if (cmdwr != 0)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmdwr, valr));
    }

    lchip_num = sys_usw_get_local_chip_num();
    for (lchip_tmp = 0; (lchip_tmp < lchip_num) && p_usw_ipuc_master[lchip]->rchain_en; lchip_tmp++)
    {
        if (cmdwl != 0)
        {
            CTC_ERROR_RETURN(DRV_IOCTL(lchip_tmp, 0, cmdwl, vall));
        }

        if (cmdwr != 0)
        {
            CTC_ERROR_RETURN(DRV_IOCTL(lchip_tmp, 0, cmdwr, valr));
        }
    }
    return CTC_E_NONE;
}

int32
sys_usw_ipuc_arrange_fragment(uint8 lchip)
{
    sys_ipuc_param_list_t *p_param_list = NULL;
    sys_ipuc_param_list_t *p_param_cur = NULL;
    sys_ipuc_param_list_t *p_cur = NULL;
    sys_ipuc_traverse_t trav_data = {0};
    uint8  ip_ver = 0;
    uint16  vrf = 0;
    int32 ret = CTC_E_NONE;

    SYS_IPUC_INIT_CHECK;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_IPUC_LOCK;

    if(DRV_IS_TSINGMA(lchip) && MCHIP_API(lchip)->ipuc_alpm_merge)
    {
        for (ip_ver = CTC_IP_VER_4; ip_ver < MAX_CTC_IP_VER; ip_ver++)
        {
            for (vrf = 0; vrf <= MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID); vrf++)
            {
                ret = MCHIP_API(lchip)->ipuc_alpm_merge(lchip, vrf, ip_ver);
            }
        }
        SYS_IPUC_UNLOCK;
        return ret;
    }

    p_param_list = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_ipuc_param_list_t));
    if (NULL == p_param_list)
    {
        SYS_IPUC_UNLOCK;
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_param_list, 0, sizeof(sys_ipuc_param_list_t));

    trav_data.lchip = lchip;
    trav_data.data = (void*)p_param_list;
    trav_data.data1 = (void*)p_param_list;

    for(ip_ver = CTC_IP_VER_4; ip_ver < MAX_CTC_IP_VER; ip_ver++)
    {
        /* prepare sort table */
        _sys_usw_ipuc_db_traverse(lchip, ip_ver, _sys_usw_ipuc_db_get_info_cb, &trav_data);
        if ((ip_ver == CTC_IP_VER_6) && (trav_data.data != trav_data.data1))
        {
            p_cur = (sys_ipuc_param_list_t *)trav_data.data;
            mem_free(p_cur->p_next_param);
            p_cur->p_next_param = NULL;
        }
    }
    SYS_IPUC_UNLOCK;

    /* ipuc count must bigger than one */
    if (p_param_list->p_next_param)
    {
        if (MCHIP_API(lchip)->ipuc_alpm_arrange_fragment)
        {
            CTC_ERROR_RETURN(MCHIP_API(lchip)->ipuc_alpm_arrange_fragment(lchip, p_param_list));
        }
    }

    while(p_param_list)
    {
        p_param_cur = p_param_list;
        p_param_list = p_param_cur->p_next_param;
        mem_free(p_param_cur->p_ipuc_param);
        mem_free(p_param_cur);
    }

    return CTC_E_NONE;
}

int32
sys_usw_ipuc_set_entry_hit(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param, uint8 hit)
{
    uint8 timer = 0;
    sys_ipuc_param_t sys_param = {0};
    uint8 domain_type = 0;

    SYS_IPUC_INIT_CHECK;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    /* para check */
    CTC_ERROR_RETURN(_sys_usw_ipuc_check(lchip, p_ipuc_param));
    CTC_ERROR_RETURN(_sys_usw_ipuc_flag_check(lchip, p_ipuc_param, 1));
    SYS_IP_FUNC_DBG_DUMP(p_ipuc_param);

    sal_memcpy(&sys_param.param, p_ipuc_param, sizeof(ctc_ipuc_param_t));

    SYS_IPUC_LOCK;
    /*check entry exist*/
    CTC_ERROR_RETURN_IPUC_UNLOCK(_sys_usw_ipuc_db_lookup(lchip, &sys_param));
    if (!sys_param.info)
    {
        CTC_RETURN_IPUC_UNLOCK(CTC_E_NOT_EXIST);
    }

    if ((sys_param.info->route_opt != DO_HASH) || CTC_FLAG_ISSET(p_ipuc_param->route_flag, CTC_IPUC_FLAG_HOST_USE_LPM))
    {
        CTC_RETURN_IPUC_UNLOCK(CTC_E_NOT_SUPPORT);
    }

    domain_type = (sys_param.info->route_opt == DO_HASH)?((sys_param.info->l4_dst_port)?SYS_AGING_DOMAIN_HOST1:SYS_AGING_DOMAIN_IP_HASH):SYS_AGING_DOMAIN_TCAM;/* SYSTEM MODIFIED, Modified by taocy for SDK bug 107356, 2019-07-17 */
    hit = hit ? 1:0;
    CTC_ERROR_RETURN_IPUC_UNLOCK(sys_usw_aging_get_aging_timer(lchip, domain_type, sys_param.info->key_index, &timer));
    CTC_ERROR_RETURN_IPUC_UNLOCK(sys_usw_aging_set_aging_status(lchip, domain_type, sys_param.info->key_index, timer, hit));
    SYS_IPUC_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_ipuc_get_entry_hit(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param, uint8* hit)
{
    sys_ipuc_param_t sys_param = {0};
    uint8 domain_type = 0;

    SYS_IPUC_INIT_CHECK;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    /* para check */
    CTC_PTR_VALID_CHECK(hit);
    CTC_ERROR_RETURN(_sys_usw_ipuc_check(lchip, p_ipuc_param));
    CTC_ERROR_RETURN(_sys_usw_ipuc_flag_check(lchip, p_ipuc_param, 1));
    SYS_IP_FUNC_DBG_DUMP(p_ipuc_param);
    sal_memcpy(&sys_param.param, p_ipuc_param, sizeof(ctc_ipuc_param_t));

    SYS_IPUC_LOCK;
    /*check entry exist*/
    CTC_ERROR_RETURN_IPUC_UNLOCK(_sys_usw_ipuc_db_lookup(lchip, &sys_param));
    if (!sys_param.info)
    {
        CTC_RETURN_IPUC_UNLOCK(CTC_E_NOT_EXIST);
    }
    if (sys_param.info->route_opt != DO_HASH)
    {
        CTC_RETURN_IPUC_UNLOCK(CTC_E_NOT_SUPPORT);
    }
    domain_type = (sys_param.info->route_opt == DO_HASH)?((sys_param.info->l4_dst_port)?SYS_AGING_DOMAIN_HOST1:SYS_AGING_DOMAIN_IP_HASH):SYS_AGING_DOMAIN_TCAM;/* SYSTEM MODIFIED, Modified by taocy for SDK bug 107356, 2019-07-17 */
    CTC_ERROR_RETURN_IPUC_UNLOCK(sys_usw_aging_get_aging_status(lchip, domain_type, sys_param.info->key_index, hit));
    SYS_IPUC_UNLOCK;

    return CTC_E_NONE;
}

#define __1_SHOW__
int32
sys_usw_ipuc_retrieve_ip(uint8 lchip, sys_ipuc_info_t *p_sys_ipuc_info)
{
    sys_ipuc_tcam_data_t tcam_data = {0};
    ctc_ipuc_param_t ctc_param = {0};
    sys_ipuc_tbl_type_t tbl_type = SYS_TBL_HASH;
    uint32 tbl_id = 0;

    SYS_IPUC_INIT_CHECK;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nDetail Table offset\n");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------\n");

    if (p_sys_ipuc_info->route_opt == DO_HASH)
    {
        tbl_type = p_sys_ipuc_info->l4_dst_port ? SYS_TBL_HASH_NAPT : SYS_TBL_HASH;
        sys_usw_get_tbl_id(lchip, SYS_IPUC_VER(p_sys_ipuc_info), 1, tbl_type, &tbl_id, NULL);
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-25s:%d\n", TABLE_NAME(lchip, tbl_id), p_sys_ipuc_info->key_index);
    }
    else if (p_sys_ipuc_info->route_opt == DO_TCAM)
    {
        if (MCHIP_API(lchip)->ipuc_show_tcam_key)
        {
            tcam_data.key_type = SYS_IPUC_TCAM_FLAG_TCAM;
            tcam_data.key_index = p_sys_ipuc_info->key_index;
            tcam_data.info = (void*)p_sys_ipuc_info;

            MCHIP_API(lchip)->ipuc_show_tcam_key(lchip, &tcam_data);
        }
    }
    else if (p_sys_ipuc_info->route_opt == DO_ALPM)
    {
        if (MCHIP_API(lchip)->ipuc_alpm_show_alpm_key)
        {
            _sys_usw_ipuc_info_to_param(lchip, &ctc_param, p_sys_ipuc_info);
            MCHIP_API(lchip)->ipuc_alpm_show_alpm_key(lchip, &ctc_param);
        }
    }

    /* ad table */
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-25s:%d\n","DsIpDa", p_sys_ipuc_info->ad_index);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------\n");

    return CTC_E_NONE;
}

int32
sys_usw_show_ipuc_info(sys_ipuc_info_t* p_ipuc_data, void* data)
{
    uint32 detail = 0;
    uint8   lchip = 0;
    uint32  route_flag = 0;
    char buf[CTC_IPV6_ADDR_STR_LEN];

#define SYS_IPUC_MASK_LEN 5
#define SYS_IPUC_L4PORT_LEN 7
    char buf2[20] = {0};
    char buf3[20] = {0};
    char buf_s[1] = {0};

    detail = *((uint32*)(((sys_ipuc_traverse_t*)data)->data));
    lchip  = ((sys_ipuc_traverse_t*)data)->lchip;

    sal_sprintf(buf2, "/%d", p_ipuc_data->masklen);
    if (p_ipuc_data->l4_dst_port)
    {
        sal_sprintf(buf3, "/%d", p_ipuc_data->l4_dst_port);
    }

    if (CTC_FLAG_ISSET(p_ipuc_data->route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE))
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-5s", "*");
    }
    else
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-5d", p_ipuc_data->vrf_id);
    }

    if (SYS_IPUC_VER(p_ipuc_data) == CTC_IP_VER_4)
    {
        uint32 tempip = sal_ntohl(p_ipuc_data->ip.ipv4);
        sal_inet_ntop(AF_INET, &tempip, buf, CTC_IPV6_ADDR_STR_LEN);
        sal_strncat(buf, buf2, SYS_IPUC_MASK_LEN);
        sal_strncat(buf, buf3, SYS_IPUC_L4PORT_LEN);
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-28s", buf);
    }
    else
    {
        uint32 ipv6_address[4] = {0, 0, 0, 0};

        ipv6_address[0] = sal_ntohl(p_ipuc_data->ip.ipv6[3]);
        ipv6_address[1] = sal_ntohl(p_ipuc_data->ip.ipv6[2]);
        ipv6_address[2] = sal_ntohl(p_ipuc_data->ip.ipv6[1]);
        ipv6_address[3] = sal_ntohl(p_ipuc_data->ip.ipv6[0]);

        sal_inet_ntop(AF_INET6, ipv6_address, buf, CTC_IPV6_ADDR_STR_LEN);
        sal_strncat(buf, buf2, SYS_IPUC_MASK_LEN);
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-44s", buf);
    }

    buf2[0] = '\0';
    route_flag = p_ipuc_data->route_flag;
    CTC_UNSET_FLAG(route_flag, SYS_IPUC_FLAG_MERGE_KEY);
    CTC_UNSET_FLAG(route_flag, SYS_IPUC_FLAG_IS_IPV6);
    CTC_UNSET_FLAG(route_flag, SYS_IPUC_FLAG_DEFAULT);

    if (CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_RPF_CHECK))
    {
        buf_s[0] = 'R';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_TTL_CHECK))
    {
        buf_s[0] = 'T';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_ICMP_CHECK))
    {
        buf_s[0] = 'I';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_ICMP_ERR_MSG_CHECK))
    {
        buf_s[0] = 'E';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_CPU))
    {
        buf_s[0] = 'C';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_PROTOCOL_ENTRY))
    {
        buf_s[0] = 'P';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_SELF_ADDRESS))
    {
        buf_s[0] = 'S';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(route_flag, SYS_IPUC_FLAG_IS_TCP_PORT))
    {
        buf_s[0] = 'U';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(p_ipuc_data->route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE))
    {
        buf_s[0] = 'B';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(p_ipuc_data->route_flag, SYS_IPUC_FLAG_MERGE_KEY))
    {
        buf_s[0] = 'M';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(p_ipuc_data->route_flag, SYS_IPUC_FLAG_HOST_USE_LPM))
    {
        buf_s[0] = 'L';
        sal_strncat(buf2, buf_s, 1);
    }

    if (CTC_FLAG_ISSET(p_ipuc_data->route_flag, SYS_IPUC_FLAG_CONNECT))
    {
        buf_s[0] = 'X';
        sal_strncat(buf2, buf_s, 1);
    }

    if ('\0' == buf2[0])
    {
        buf_s[0] = 'O';
        sal_strncat(buf2, buf_s, 1);
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-4u   %-5s   %-7s\n\r",
                      p_ipuc_data->nh_id, buf2,
                      (p_ipuc_data->route_opt != DO_HASH) ? "FALSE":"TRUE");

    if (detail)
    {
        sys_usw_ipuc_retrieve_ip(lchip, p_ipuc_data);
    }

    return CTC_E_NONE;
}

int32
sys_usw_ipuc_db_show(uint8 lchip,
                ctc_ip_ver_t            ip_ver,
                sys_ipuc_info_t         *p_ipuc_data,
                uint32                  detail)
{
    sys_ipuc_traverse_t travs;

    LCHIP_CHECK(lchip);
	SYS_IPUC_INIT_CHECK;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Offset: T-TCAM    S-SRAM\n\r");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Flags:  R-RPF check    T-TTL check    I-ICMP redirect check      C-Send to CPU\n\r");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "        X-Connect      P-Protocol entry           S-Self address     U-TCP port\n\r");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "        E-ICMP error msg check    B-Public route    M-Merge dsfwd      L-Host use Lpm\n\r");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "        O-None flag\n\r");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------------------------------------------------------------------\n\r");
    if (ip_ver == CTC_IP_VER_4)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "VRF   Route                      NHID   Flags   In_SRAM\n\r");
    }
    else
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "VRF   Route                                      NHID   Flags   In_SRAM\n\r");
    }
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------------------------------------------------------------------\n\r");

    travs.data = &detail;
    travs.lchip = lchip;

    if(p_ipuc_data)
    {
        sys_usw_show_ipuc_info(p_ipuc_data, (void*)&travs);
    }
    else
    {
        _sys_usw_ipuc_db_traverse(lchip, ip_ver,
                    (hash_traversal_fn)sys_usw_show_ipuc_info, (void*)&travs);
    }

    return CTC_E_NONE;
}

int32
sys_usw_ipuc_show_status(uint8 lchip)
{
    uint8 v4_frag_status = 0;
    uint8 v6_frag_status = 0;
    int32 ret = 0;
    uint8 i=0;
    SYS_IPUC_INIT_CHECK;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------Default action--------------------------------\n");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15s%-4s%s\n", "TYPE", " ", "Nexthop", " ", "AD");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------------------\n");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15d%-4s%d\n", "--IPv4", " ", p_usw_ipuc_master[lchip]->default_nhid[CTC_IP_VER_4], " ", p_usw_ipuc_master[lchip]->default_base[CTC_IP_VER_4]);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15d%-4s%d\n", "--IPv6", " ", p_usw_ipuc_master[lchip]->default_nhid[CTC_IP_VER_6], " ", p_usw_ipuc_master[lchip]->default_base[CTC_IP_VER_6]);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------------------\n");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------SPEC Resource Usage---------------------------\n");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15s%-4s%s\n", "TYPE", " ", "SPEC", " ", "COUNT");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------------------\n");

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%d\n", "HOST Route", " ", SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_IPUC_HOST));
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15s%-4s%d\n", "--IPv4", " ", "- ", " ", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_HOST]);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15s%-4s%d\n", "--IPv6", " ", "-", " ", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_HOST]);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%d\n", "LPM Route", " ", SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_IPUC_LPM));
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15s%-4s%d\n", "--IPv4", " ", "-", " ", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_LPM]);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15s%-4s%d\n", "--IPv6", " ", "-", " ", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_LPM]);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------------------\n");

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------Memory Usage----------------------------------\n");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15s%-4s%-15s%-4s%-15s\n", "TYPE", " ", "SIZE", " ", "Entries", " ", "Usage");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------------------\n");

    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_HASH_LOOKUP))
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%d\n", "Hash", " ", p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_HASH]);
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15s%-4s", "--IPv4", " ", "-", " ");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15d%-4s%-15d\n", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_HASH], " ", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_HASH]);
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15s%-4s", "--IPv6", " ", "-", " ");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15d%-4s%-15d\n", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_HASH], " ", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_HASH] * 2);
    }

    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_LOOKUP))
    {
        if (MCHIP_API(lchip)->ipuc_show_tcam_status)
        {
            MCHIP_API(lchip)->ipuc_show_tcam_status(lchip);
        }
    }

    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_ALPM_LOOKUP))
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s\n", "ALPM", " ");
        if (MCHIP_API(lchip)->ipuc_alpm_show_status)
        {
            MCHIP_API(lchip)->ipuc_alpm_show_status(lchip);
        }
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15s%-4s", "--IPv4", " ", "/", " ");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15d%-4s%-15s\n", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_ALPM], " ", "/");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15s%-4s", "--IPv6", " ", "/", " ");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15d%-4s%-15s\n", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_ALPM], " ", "/");
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------------------\n");

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------Hash Conflict Status--------------------------\n");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-10s\n", "TYPE", " ", "COUNT");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------------------\n");

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s\n", "In TCAM");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%d\n", "--IPv4", " ", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_CONFLICT_TCAM]);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%d\n", "--IPv6", " ", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_CONFLICT_TCAM]);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------------------\n");

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------Nat Status------------------------------------\n");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s%-15s%-4s%s\n", "TYPE", " ", "SIZE", " ", "COUNT");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------------------\n");

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s", "IPV4 SNAT Tcam", " ");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15d%-4s",  p_usw_ip_tunnel_master[lchip]->max_snat_tcam_num, " ");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%d\n", p_usw_ip_tunnel_master[lchip]->snat_tcam_count);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s", "IPV4 SNAT Hash", " ");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15d%-4s",  (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_HASH_NAPT] ?
        SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_NAPT) : 0), " ");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%d\n", p_usw_ip_tunnel_master[lchip]->snat_hash_count);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s", "IPV4 NAPT Hash", " ");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s",  (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_HASH_NAPT] ?
        "-" : "0"), " ");
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%d\n", p_usw_ip_tunnel_master[lchip]->napt_hash_count);
    if (DRV_IS_TSINGMA(lchip))
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------------------\n");

        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "LPM TCAM usage v4:%u/%u, v6:%u/%u\n",
                         p_usw_ipuc_master[lchip]->lpm_tcam_used_cnt[CTC_IP_VER_4],
                         p_usw_ipuc_master[lchip]->lpm_tcam_spec[CTC_IP_VER_4],
                         p_usw_ipuc_master[lchip]->lpm_tcam_used_cnt[CTC_IP_VER_6],
                         p_usw_ipuc_master[lchip]->lpm_tcam_spec[CTC_IP_VER_6]);
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------\n");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Auto resolve conflict v4:%d, v6:%d\n",
                         p_usw_ipuc_master[lchip]->arc[CTC_IP_VER_4],
                         p_usw_ipuc_master[lchip]->arc[CTC_IP_VER_6]);
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------\n");
        ret = MCHIP_API(lchip)->ipuc_show_sram_usage(lchip);
        if(CTC_E_NOT_INIT == ret)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Feature not initialized\n");
        }
        if(p_usw_ipuc_master[lchip]->rchain_en && 0 == lchip)
        {
            for (i = 1; i <= p_usw_ipuc_master[lchip]->rchain_tail; i++)
            {
                MCHIP_API(lchip)->ipuc_show_sram_usage(i);
            }
        }
        if (MCHIP_API(lchip)->ipuc_alpm_get_fragment_status)
        {
            ret = MCHIP_API(lchip)->ipuc_alpm_get_fragment_status(lchip, CTC_IP_VER_4, &v4_frag_status);
            ret = MCHIP_API(lchip)->ipuc_alpm_get_fragment_status(lchip, CTC_IP_VER_6, &v6_frag_status);
            if(CTC_E_NOT_INIT == ret)
            {
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------\n");
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Feature not initialized\n");
            }
            else
            {
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------\n");
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "IPv4 Merge status: %u\n", v4_frag_status);
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "IPv6 Merge status: %u\n", v6_frag_status);
            }
        }
    }
    if(p_usw_ipuc_master[lchip]->rchain_en && 0 == lchip)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------\n");
        for (i = 0; i <= p_usw_ipuc_master[lchip]->rchain_tail; i++)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Lchip %d lpm route: %u\n", i, *(p_usw_ipuc_master[lchip]->rchain_stats + i));
        }
    }

    return CTC_E_NONE;
}

int32
sys_usw_ipuc_show_debug_info(uint8 lchip, ctc_ipuc_param_t* p_ctc_param_info)
{
    sys_ipuc_param_t sys_param = {0};

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_IPUC_INIT_CHECK;

    sal_memcpy(&sys_param.param, p_ctc_param_info, sizeof(ctc_ipuc_param_t));
    /* 1. param check and debug out */
    CTC_ERROR_RETURN(_sys_usw_ipuc_check(lchip, &sys_param.param));
    SYS_IP_FUNC_DBG_DUMP((&sys_param.param));

    /* para check */
    CTC_ERROR_RETURN(_sys_usw_ipuc_flag_check(lchip, &sys_param.param, 0));

    SYS_IPUC_LOCK;
    /* lookup for ipuc entrise */
    CTC_ERROR_RETURN_IPUC_UNLOCK(_sys_usw_ipuc_db_lookup(lchip, &sys_param));
    if (!sys_param.info)
    {
        CTC_RETURN_IPUC_UNLOCK(CTC_E_NOT_EXIST);
    }

    CTC_ERROR_RETURN_IPUC_UNLOCK(sys_usw_ipuc_db_show(lchip, sys_param.param.ip_ver, sys_param.info, 1));

    SYS_IPUC_UNLOCK;

    return CTC_E_NONE;
}

extern int32
sys_usw_ipuc_dump_ipuc_info(uint8 lchip, ctc_ipuc_param_t* p_ctc_param_info)
{
    sys_ipuc_param_t sys_param = {0};
    uint32 detail = 0;
    sys_ipuc_traverse_t travs;


    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_IPUC_INIT_CHECK;
    sal_memset(&travs, 0, sizeof(sys_ipuc_traverse_t));

    sal_memcpy(&sys_param.param, p_ctc_param_info, sizeof(ctc_ipuc_param_t));
    /* 1. param check and debug out */
    CTC_ERROR_RETURN(_sys_usw_ipuc_check(lchip, &sys_param.param));
    SYS_IP_FUNC_DBG_DUMP((&sys_param.param));

    /* para check */
    CTC_ERROR_RETURN(_sys_usw_ipuc_flag_check(lchip, &sys_param.param, 0));

    SYS_IPUC_LOCK;
    /* lookup for ipuc entrise */
    CTC_ERROR_RETURN_IPUC_UNLOCK(_sys_usw_ipuc_db_lookup(lchip, &sys_param));
    if (!sys_param.info)
    {
        CTC_RETURN_IPUC_UNLOCK(CTC_E_NOT_EXIST);
    }

    travs.data = &detail;
    travs.lchip = lchip;

    if(sys_param.info)
    {
        sys_usw_show_ipuc_info(sys_param.info, (void*)&travs);
    }

    SYS_IPUC_UNLOCK;

    return CTC_E_NONE;
}

int32
_sys_usw_ipuc_wb_mapping_info(uint8 lchip, sys_wb_ipuc_info_t *p_wb_ipuc_info, sys_ipuc_info_t *p_ipuc_info, uint8 sync)
{
    uint8 addr_size = 0;
    ctc_ipuc_param_t ctc_param = {0};

    if (sync)
    {
        sal_memset(&p_wb_ipuc_info->lpm_info, 0, sizeof(p_wb_ipuc_info->lpm_info));

        p_wb_ipuc_info->masklen = p_ipuc_info->masklen;
        p_wb_ipuc_info->vrf_id = p_ipuc_info->vrf_id;

        addr_size = (SYS_IPUC_VER(p_ipuc_info) == CTC_IP_VER_4) ? sizeof(ip_addr_t): sizeof(ipv6_addr_t);
        sal_memcpy(p_wb_ipuc_info->ip, &(p_ipuc_info)->ip, addr_size);

        if (SYS_IPUC_VER(p_ipuc_info) == CTC_IP_VER_4)
        {
            sal_memset(&p_wb_ipuc_info->ip[1], 0, sizeof(uint32) * 3);
        }
        p_wb_ipuc_info->route_flag = p_ipuc_info->route_flag;
        p_wb_ipuc_info->nh_id = p_ipuc_info->nh_id;
        p_wb_ipuc_info->ad_index = p_ipuc_info->ad_index;
        p_wb_ipuc_info->key_index = p_ipuc_info->key_index;
        p_wb_ipuc_info->rpf_id = p_ipuc_info->rpf_id;
        p_wb_ipuc_info->rpf_mode = p_ipuc_info->rpf_mode;
        p_wb_ipuc_info->l4_dst_port = p_ipuc_info->l4_dst_port;
        p_wb_ipuc_info->cid = p_ipuc_info->cid;
        p_wb_ipuc_info->gport = p_ipuc_info->gport;
        p_wb_ipuc_info->route_opt = p_ipuc_info->route_opt;
        p_wb_ipuc_info->stats_id = p_ipuc_info->stats_id;

        if (DO_ALPM != p_ipuc_info->route_opt)
        {
            return CTC_E_NONE;
        }

        _sys_usw_ipuc_info_to_param(lchip, &ctc_param, p_ipuc_info);

         /*sys_duet2_calpm_get_wb_info(lchip, &ctc_param, &p_wb_ipuc_info->lpm_info);*/
        if (MCHIP_API(lchip)->ipuc_alpm_get_wb_info)
        {
            CTC_ERROR_RETURN(MCHIP_API(lchip)->ipuc_alpm_get_wb_info(lchip, &ctc_param, &p_wb_ipuc_info->lpm_info));
        }

    }
    else
    {
        p_ipuc_info->nh_id = p_wb_ipuc_info->nh_id;
        p_ipuc_info->ad_index = p_wb_ipuc_info->ad_index;
        p_ipuc_info->key_index = p_wb_ipuc_info->key_index;
        p_ipuc_info->rpf_id = p_wb_ipuc_info->rpf_id;
        p_ipuc_info->vrf_id = p_wb_ipuc_info->vrf_id;
        p_ipuc_info->route_flag = p_wb_ipuc_info->route_flag;
        p_ipuc_info->masklen = p_wb_ipuc_info->masklen;
        p_ipuc_info->rpf_mode = p_wb_ipuc_info->rpf_mode;
        p_ipuc_info->l4_dst_port = p_wb_ipuc_info->l4_dst_port;
        p_ipuc_info->cid = p_wb_ipuc_info->cid;
        p_ipuc_info->gport = p_wb_ipuc_info->gport;
        p_ipuc_info->route_opt = p_wb_ipuc_info->route_opt;
        p_ipuc_info->stats_id = p_wb_ipuc_info->stats_id;

        addr_size = (SYS_IPUC_VER(p_ipuc_info) == CTC_IP_VER_4) ? sizeof(ip_addr_t): sizeof(ipv6_addr_t);
        sal_memcpy(&(p_ipuc_info)->ip, &(p_wb_ipuc_info)->ip, addr_size);

        if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_RPF_CHECK) && p_ipuc_info->rpf_mode == SYS_RPF_CHK_MODE_PROFILE
            && p_ipuc_info->rpf_id >= SYS_RPF_PROFILE_START_INDEX)
        {
            CTC_ERROR_RETURN(sys_usw_rpf_restore_profile(lchip, p_ipuc_info->rpf_id));
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_ipuc_wb_sync_info_func(sys_ipuc_info_t *p_ipuc_info, void *user_data)
{
    uint16 max_entry_cnt = 0;
    sys_wb_ipuc_info_t  *p_wb_ipuc_info;
    sys_traverse_t *data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)(data->data);
    uint8 lchip = (uint8)(data->value1);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_ipuc_info = (sys_wb_ipuc_info_t *)wb_data->buffer + wb_data->valid_cnt;

    CTC_ERROR_RETURN(_sys_usw_ipuc_wb_mapping_info(lchip, p_wb_ipuc_info, p_ipuc_info, 1));
    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32
sys_usw_ipuc_wb_sync(uint8 lchip)
{
    uint32 cmd = 0;
    uint8 ip_ver = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_traverse_t ipuc_hash_data;
    sys_wb_ipuc_master_t  *p_wb_ipuc_master;
    uint8 work_status = 0;

    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	return CTC_E_NONE;
    }

    /*syncup ipuc matser*/
    wb_data.buffer = mem_malloc(MEM_IPUC_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_ipuc_master_t, CTC_FEATURE_IPUC, SYS_WB_APPID_IPUC_SUBID_MASTER);

    p_wb_ipuc_master = (sys_wb_ipuc_master_t  *)wb_data.buffer;

    p_wb_ipuc_master->lchip = lchip;
    p_wb_ipuc_master->version = SYS_WB_VERSION_IPUC;
    p_wb_ipuc_master->host_lpm_mode = p_usw_ipuc_master[lchip]->host_lpm_mode;
    p_wb_ipuc_master->pending_list_len = p_usw_ipuc_master[lchip]->pending_list_len;
    cmd = DRV_IOR(LpmTcamCtl_t, LpmTcamCtl_privatePublicLookupMode_f);
    DRV_IOCTL(lchip, 0, cmd, &p_wb_ipuc_master->tcam_mode);

    sal_memcpy(p_wb_ipuc_master->route_stats, p_usw_ipuc_master[lchip]->route_stats, sizeof(uint32) * MAX_CTC_IP_VER * MAX_STATS_TYPE);

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    if (MCHIP_API(lchip)->ipuc_alpm_mapping_wb_master)
    {
        CTC_ERROR_GOTO(MCHIP_API(lchip)->ipuc_alpm_mapping_wb_master(lchip, 1), ret, done);
    }

    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_LOOKUP))
    {
        CTC_ERROR_GOTO(sys_usw_ofb_wb_sync(lchip, p_usw_ipuc_master[lchip]->share_ofb_type[SYS_PRIVATE_MODE],
                                            CTC_FEATURE_IPUC, SYS_WB_APPID_IPUC_SUBID_OFB), ret, done);
    }

    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_PUB_LOOKUP))
    {
        CTC_ERROR_GOTO(sys_usw_ofb_wb_sync(lchip, p_usw_ipuc_master[lchip]->share_ofb_type[SYS_PUBLIC_MODE],
                                        CTC_FEATURE_IPUC, SYS_WB_APPID_IPUC_SUBID_OFB), ret, done);
    }

    /*syncup ipuc_info*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_ipuc_info_t, CTC_FEATURE_IPUC, SYS_WB_APPID_IPUC_SUBID_INFO);
    ipuc_hash_data.data = &wb_data;
    ipuc_hash_data.value1 = lchip;

    for (ip_ver = CTC_IP_VER_4; ip_ver < MAX_CTC_IP_VER; ip_ver++)
    {
        if (!p_usw_ipuc_master[lchip]->fib_hash[ip_ver])
        {
            continue;
        }

        wb_data.valid_cnt = 0;
        CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_ipuc_master[lchip]->fib_hash[ip_ver], (hash_traversal_fn) _sys_usw_ipuc_wb_sync_info_func, (void *)&ipuc_hash_data), ret, done);
        if (wb_data.valid_cnt > 0)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
            wb_data.valid_cnt = 0;
        }
    }

#ifdef TSINGMA //TBD_
    /*syncup nalpm prefix info*/
    if(!DRV_IS_DUET2(lchip))
    {
        ret = sys_tsingma_nalpm_wb_prefix_sync(lchip);
    }
#endif

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32
sys_usw_ipuc_wb_restore(uint8 lchip)
{
    uint8 db_size = 0;
    uint32 tcam_mode = 0;
    uint16 entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    ctc_wb_query_t wb_query;
    ctc_ipuc_param_t ctc_param = {0};
    sys_wb_ipuc_master_t  wb_ipuc_master = {0};
    sys_ipuc_info_t *p_ipuc_info;
    sys_wb_ipuc_info_t  wb_ipuc_info = {0};
    sys_ipuc_ad_spool_t ipuc_ad_new;
    sys_nh_update_dsnh_param_t update_dsnh;
    sys_ipuc_param_t sys_param;

    sal_memset(&ipuc_ad_new, 0, sizeof(sys_ipuc_ad_spool_t));
    sal_memset(&update_dsnh, 0, sizeof(sys_nh_update_dsnh_param_t));

    wb_query.buffer = mem_malloc(MEM_IPUC_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_ipuc_master_t, CTC_FEATURE_IPUC, SYS_WB_APPID_IPUC_SUBID_MASTER);

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);

    /*restore  ipuc_master*/
    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "query ipuc master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_NONE;
        goto done;
    }

    sal_memcpy((uint8*)&wb_ipuc_master, (uint8*)wb_query.buffer, wb_query.key_len + wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_IPUC, wb_ipuc_master.version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    cmd = DRV_IOR(LpmTcamCtl_t, LpmTcamCtl_privatePublicLookupMode_f);
    DRV_IOCTL(lchip, 0, cmd, &tcam_mode);

    if ((tcam_mode !=  wb_ipuc_master.tcam_mode) ||
        (p_usw_ipuc_master[lchip]->host_lpm_mode != wb_ipuc_master.host_lpm_mode))
    {
        ret = CTC_E_VERSION_MISMATCH;
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,
            "ipuc init and warmboot tcam mode mismatch! tcam mode: %u|%u use lpm enable: %u|%u \n",
            tcam_mode, wb_ipuc_master.tcam_mode,
            p_usw_ipuc_master[lchip]->host_lpm_mode, wb_ipuc_master.host_lpm_mode);

        goto done;
    }

    p_usw_ipuc_master[lchip]->pending_list_len = wb_ipuc_master.pending_list_len;

    sal_memcpy(p_usw_ipuc_master[lchip]->route_stats, wb_ipuc_master.route_stats, sizeof(uint32) * MAX_CTC_IP_VER * MAX_STATS_TYPE);

    cmd = DRV_IOR(FibEngineLookupResultCtl_t, FibEngineLookupResultCtl_gIpv4UcastLookupResultCtl_defaultEntryBase_f);
    DRV_IOCTL(lchip, 0, cmd, &p_usw_ipuc_master[lchip]->default_base[CTC_IP_VER_4]);

    cmd = DRV_IOR(FibEngineLookupResultCtl_t, FibEngineLookupResultCtl_gIpv6UcastLookupResultCtl_defaultEntryBase_f);
    DRV_IOCTL(lchip, 0, cmd, &p_usw_ipuc_master[lchip]->default_base[CTC_IP_VER_6]);

    if (MCHIP_API(lchip)->ipuc_alpm_mapping_wb_master)
    {
        CTC_ERROR_GOTO(MCHIP_API(lchip)->ipuc_alpm_mapping_wb_master(lchip, 0), ret, done);
    }

    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_LOOKUP))
    {
        CTC_ERROR_GOTO(sys_usw_ofb_wb_restore(lchip, p_usw_ipuc_master[lchip]->share_ofb_type[SYS_PRIVATE_MODE],
                                            CTC_FEATURE_IPUC, SYS_WB_APPID_IPUC_SUBID_OFB), ret, done);
    }

    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_PUB_LOOKUP))
    {
        CTC_ERROR_GOTO(sys_usw_ofb_wb_restore(lchip, p_usw_ipuc_master[lchip]->share_ofb_type[SYS_PUBLIC_MODE],
                                        CTC_FEATURE_IPUC, SYS_WB_APPID_IPUC_SUBID_OFB), ret, done);
    }
#ifdef TSINGMA //TBD_
    /*restore nalpm prefix info*/
    if(!DRV_IS_DUET2(lchip))
    {
        CTC_ERROR_GOTO(sys_tsingma_nalpm_wb_prefix_restore(lchip),ret,done);
    }
#endif
    /*restore  ipuc_info*/
    CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_ipuc_info_t, CTC_FEATURE_IPUC, SYS_WB_APPID_IPUC_SUBID_INFO);

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_ipuc_info, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        db_size = (SYS_IPUC_VER(&wb_ipuc_info) == CTC_IP_VER_4) ? (sizeof(ipv6_addr_t) - sizeof(ip_addr_t)) : 0;
        db_size = sizeof(sys_ipuc_info_t) - db_size;
        p_ipuc_info = mem_malloc(MEM_IPUC_MODULE,  db_size);
        if (NULL == p_ipuc_info)
        {
            ret = CTC_E_NO_MEMORY;

            goto done;
        }
        sal_memset(p_ipuc_info, 0, db_size);

        ret = _sys_usw_ipuc_wb_mapping_info(lchip, &wb_ipuc_info, p_ipuc_info, 0);
        if (ret)
        {
            mem_free(p_ipuc_info);
            continue;
        }

        if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_IS_BIND_NH))
        {
            update_dsnh.data = p_ipuc_info;
            update_dsnh.updateAd = sys_usw_ipuc_update_ipda;
            ret = sys_usw_nh_bind_dsfwd_cb(lchip, p_ipuc_info->nh_id, &update_dsnh);
        }

        /*add ad spool*/
        cmd = DRV_IOR(DsIpDa_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, p_ipuc_info->ad_index, cmd, &ipuc_ad_new.ad);
        ipuc_ad_new.ad_index = p_ipuc_info->ad_index;

        ret = ctc_spool_add(p_usw_ipuc_master[lchip]->ad_spool, &ipuc_ad_new, NULL, NULL);
        if (ret < 0)
        {
            CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ctc_spool_add error! ret: %d.\n", ret);
            mem_free(p_ipuc_info);
            continue;
        }

        if (p_ipuc_info->route_opt != DO_HASH)
        {
            sal_memset(&sys_param, 0, sizeof(sys_ipuc_param_t));
            _sys_usw_ipuc_info_to_param(lchip, &ctc_param, p_ipuc_info);
            sys_param.info = p_ipuc_info;
        }

        /*add calpm route once again*/
        if (p_ipuc_info->route_opt == DO_ALPM)
        {
            if (MCHIP_API(lchip)->ipuc_alpm_add)
            {
                sal_memset(&sys_param, 0, sizeof(sys_ipuc_param_t));
                _sys_usw_ipuc_info_to_param(lchip, &sys_param.param, p_ipuc_info);
                ret = MCHIP_API(lchip)->ipuc_alpm_add(lchip, &sys_param, p_ipuc_info->ad_index, (void*)&wb_ipuc_info.lpm_info);
                if (ret)
                {
                    CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "add alpm error! ret: %d.\n", ret);
                    mem_free(p_ipuc_info);
                    continue;
                }
            }
        }/*alloc Tcam key index from position*/
        else if (DO_TCAM == p_ipuc_info->route_opt)
        {
            sys_ipuc_tcam_data_t tcam_data = {0};

            tcam_data.opt_type = DO_ADD;
            tcam_data.key_index = p_ipuc_info->key_index;
            tcam_data.key_type = SYS_IPUC_TCAM_FLAG_TCAM;
            tcam_data.ipuc_param = &ctc_param;
            tcam_data.info = (void*)p_ipuc_info;

            ret = sys_usw_ipuc_alloc_tcam_key_index(lchip, &tcam_data);
            if (ret)
            {
                CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "sys_usw_ipuc_alloc_tcam_key_index %u error! ret: %d.\n", p_ipuc_info->key_index, ret);
                mem_free(p_ipuc_info);
                continue;
            }
            //SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "alloc sort key index from %u.\n", p_ipuc_info->key_index);
        }

        ctc_hash_insert(p_usw_ipuc_master[lchip]->fib_hash[SYS_IPUC_VER(p_ipuc_info)], p_ipuc_info);
    CTC_WB_QUERY_ENTRY_END((&wb_query));

done:
    if (wb_query.key)
    {
        mem_free(wb_query.key);
    }

    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    return ret;
}

int32
_sys_usw_ipuc_ad_fprintf_traverse(void *node, sys_dump_db_traverse_param_t* user_data)
{
    sal_file_t p_file = (sal_file_t)user_data->value0;
    sys_ipuc_ad_spool_t* ipuc_ad = NULL;
    uint32* ptr = NULL;
    uint8 i = 0;
    uint8* mode = (uint8 *)user_data->value2;
    ipuc_ad = (sys_ipuc_ad_spool_t*)((ctc_spool_node_t*)node)->data;
    ptr = (uint32 *)&(ipuc_ad->ad);
    if(*mode == 1)
    {
        SYS_DUMP_DB_LOG(p_file, "%-10u%-10u%-10u0x", *(uint32 *)user_data->value1, ((ctc_spool_node_t*)node)->ref_cnt, ipuc_ad->ad_index);
        for (i = 0; i <= 2; i++)
        {
            SYS_DUMP_DB_LOG(p_file, "%08x", *ptr);
            ptr++;
        }
        SYS_DUMP_DB_LOG(p_file, "\n");
        SYS_DUMP_DB_LOG(p_file, "%s\n", "-----------------------------------------------------------------------------------");
    }
    (*(uint32 *)user_data->value1)++;

    return CTC_E_NONE;
}

int32
sys_usw_ipuc_dump_db(uint8 lchip, sal_file_t p_f, ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;
    uint32 cnt = 1;
    uint8 mode = 1;
    sys_dump_db_traverse_param_t    cb_data;
    sal_memset(&cb_data, 0, sizeof(sys_dump_db_traverse_param_t));
    cb_data.value0 = p_f;
    cb_data.value1 = &cnt;
    cb_data.value2 = &mode;

    SYS_IPUC_INIT_CHECK;
    SYS_IPUC_LOCK;
    SYS_DUMP_DB_LOG(p_f, "%s\n", "# IPUC");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-40s\n", "Default action:");
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "IPv4 default base", p_usw_ipuc_master[lchip]->default_base[CTC_IP_VER_4]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "IPv6 default base", p_usw_ipuc_master[lchip]->default_base[CTC_IP_VER_6]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "IPv4 default nexthop", p_usw_ipuc_master[lchip]->default_nhid[CTC_IP_VER_4]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "IPv6 default nexthop", p_usw_ipuc_master[lchip]->default_nhid[CTC_IP_VER_6]);
    SYS_DUMP_DB_LOG(p_f, "%-40s\n", "Spec:");
    SYS_DUMP_DB_LOG(p_f, "%-40s\n", "--IPv4:");
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "Hash", p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_HASH]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "Hash NAPT:", p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_HASH_NAPT]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "ALPM:", p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_ALPM]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "TCAM:", p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "TCAM PUB", p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM_PUB]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "TCAM SHORT", p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM_SHORT]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "TCAM PUB SHORT", p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM_PUB_SHORT]);
    SYS_DUMP_DB_LOG(p_f, "%-40s\n", "--IPv6:");
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "Hash", p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_HASH]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "Hash NAPT", p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_HASH_NAPT]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "ALPM", p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_ALPM]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "TCAM", p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "TCAM PUB", p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_PUB]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "TCAM SHORT", p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "TCAM PUB SHORT", p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_PUB_SHORT]);
    SYS_DUMP_DB_LOG(p_f, "%-40s\n", "Route stats:");
    SYS_DUMP_DB_LOG(p_f, "%-40s\n", "--IPv4:");
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "Host", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_HOST]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "LPM", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_LPM]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "HASH", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_HASH]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "ALPM", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_ALPM]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "Conflict Tcam", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_CONFLICT_TCAM]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "Tcam", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_TCAM]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "Tcam Pub", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_TCAM_PUB]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "Tcam Short", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_TCAM_SHORT]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "Tcam Pub Short", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_TCAM_PUB_SHORT]);
    SYS_DUMP_DB_LOG(p_f, "%-40s\n", "--IPv6:");
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "Host", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_HOST]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "LPM", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_LPM]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "HASH", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_HASH]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "ALPM", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_ALPM]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "Conflict Tcam", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_CONFLICT_TCAM]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "Tcam", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_TCAM]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "Tcam Pub", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_TCAM_PUB]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "Tcam Short", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_TCAM_SHORT]);
    SYS_DUMP_DB_LOG(p_f, "  %-38s: %u\n", "Tcam Pub Short", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_TCAM_PUB_SHORT]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "Short key boundary(Private mode)", p_usw_ipuc_master[lchip]->short_key_boundary[SYS_PRIVATE_MODE]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "Short key boundary(Public mode)", p_usw_ipuc_master[lchip]->short_key_boundary[SYS_PUBLIC_MODE]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "Tcam share mode(Private mode)", p_usw_ipuc_master[lchip]->tcam_share_mode[SYS_PRIVATE_MODE]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "Tcam share mode(Public mode)", p_usw_ipuc_master[lchip]->tcam_share_mode[SYS_PUBLIC_MODE]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "Share ofb type(Private mode)", p_usw_ipuc_master[lchip]->share_ofb_type[SYS_PRIVATE_MODE]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "Share ofb type(Public mode)", p_usw_ipuc_master[lchip]->share_ofb_type[SYS_PUBLIC_MODE]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: 0x%x\n", "IPv4 Lookup mode", p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: 0x%x\n", "IPv6 Lookup mode", p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_6]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "IPv4 enable", p_usw_ipuc_master[lchip]->version_en[CTC_IP_VER_4]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "IPv6 enable", p_usw_ipuc_master[lchip]->version_en[CTC_IP_VER_6]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "Host lpm mode", p_usw_ipuc_master[lchip]->host_lpm_mode);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "Snake per group", p_usw_ipuc_master[lchip]->snake_per_group);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "Snake per group", p_usw_ipuc_master[lchip]->snake_per_group);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "Pending list length", p_usw_ipuc_master[lchip]->pending_list_len);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "ALPM async mode", p_usw_ipuc_master[lchip]->alpm_async);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "IPv4 lpm tcam spec", p_usw_ipuc_master[lchip]->lpm_tcam_spec[CTC_IP_VER_4]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "IPv6 lpm tcam spec", p_usw_ipuc_master[lchip]->lpm_tcam_spec[CTC_IP_VER_6]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "IPv4 lpm tcam used", p_usw_ipuc_master[lchip]->lpm_tcam_used_cnt[CTC_IP_VER_4]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "IPv6 lpm tcam used", p_usw_ipuc_master[lchip]->lpm_tcam_used_cnt[CTC_IP_VER_6]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "IPv4 auto resolve conflict use tcam", p_usw_ipuc_master[lchip]->arc[CTC_IP_VER_4]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "IPv6 auto resolve conflict use tcam", p_usw_ipuc_master[lchip]->arc[CTC_IP_VER_6]);
    SYS_DUMP_DB_LOG(p_f, "%-40s: %u\n", "LPM thread disable", p_usw_ipuc_master[lchip]->alpm_thread_disable);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
    if(p_dump_param->detail)
    {
        SYS_DUMP_DB_LOG(p_f, "Spool type: %s\n","ad_spool(DsIpDa)");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
        SYS_DUMP_DB_LOG(p_f, "%-10s%-10s%-10s%-10s\n", "Node", "Ref_cnt", "Ad_index", "Data");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
        ctc_spool_traverse(p_usw_ipuc_master[lchip]->ad_spool, (spool_traversal_fn)_sys_usw_ipuc_ad_fprintf_traverse , (void*)(&cb_data));
    }
    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_LOOKUP))
    {
        sys_usw_ofb_dump_db(lchip, p_usw_ipuc_master[lchip]->share_ofb_type[SYS_PRIVATE_MODE], p_f);
    }
    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_PUB_LOOKUP))
    {
        sys_usw_ofb_dump_db(lchip, p_usw_ipuc_master[lchip]->share_ofb_type[SYS_PUBLIC_MODE], p_f);
    }

    SYS_IPUC_UNLOCK;
    if(MCHIP_API(lchip)->ipuc_alpm_dump_db)
    {
        MCHIP_API(lchip)->ipuc_alpm_dump_db(lchip, p_f, p_dump_param);
    }
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");
    return ret;
}

int32
_sys_usw_ipuc_resource_init(uint8 lchip)
{
    uint8 ip_ver = 0;
    uint8 tbl_type = 0;
    uint8 short_enable = 0;
    uint8 lookup_mode = 0;
    uint32 tbl_id = 0;
    uint32 entry_num = 0;
    uint32 tbl_size[MAX_ROUTE_MODE][3];

    sys_usw_ftm_query_lpm_tcam_init_size(lchip, (uint32 **)tbl_size);

    p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM] = tbl_size[SYS_PRIVATE_MODE][0];
    p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT] =
            tbl_size[SYS_PRIVATE_MODE][1] ? (tbl_size[SYS_PRIVATE_MODE][1] - tbl_size[SYS_PRIVATE_MODE][0]) : 0;
    p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM] =
            tbl_size[SYS_PRIVATE_MODE][2] - (tbl_size[SYS_PRIVATE_MODE][1] ? tbl_size[SYS_PRIVATE_MODE][1] : tbl_size[SYS_PRIVATE_MODE][0]);

    short_enable = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT] ? 1 : 0;
    if (!p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM])
    {
        if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT])
        {
            p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM] =
                    p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT];
            p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT] = 0;
        }
    }

    p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_PUB] = tbl_size[SYS_PUBLIC_MODE][0];
    p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_PUB_SHORT] =
            tbl_size[SYS_PUBLIC_MODE][1] ? (tbl_size[SYS_PUBLIC_MODE][1] - tbl_size[SYS_PUBLIC_MODE][0]) : 0;
    p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM_PUB] =
            tbl_size[SYS_PUBLIC_MODE][2] - (tbl_size[SYS_PUBLIC_MODE][1] ? tbl_size[SYS_PUBLIC_MODE][1] : tbl_size[SYS_PUBLIC_MODE][0]);
    if (!p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM_PUB])
    {
        if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_PUB_SHORT])
        {
            p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM_PUB] =
                    p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_PUB_SHORT];
            p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_PUB_SHORT] = 0;
        }
    }

    for (ip_ver = CTC_IP_VER_4; ip_ver < MAX_CTC_IP_VER; ip_ver++)
    {   /* hash, napt_hash, lpm pipeline */
        for (tbl_type = SYS_TBL_HASH; tbl_type < SYS_TBL_TCAM; tbl_type++)
        {
            sys_usw_get_tbl_id(lchip, ip_ver, 1, tbl_type, &tbl_id, NULL);
            CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, tbl_id, &entry_num));
            if (entry_num > 0)
            {
                lookup_mode = tbl_type;

                CTC_BIT_SET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], lookup_mode);
                p_usw_ipuc_master[lchip]->max_entry_num[ip_ver][tbl_type] = entry_num;
            }
        }

        sys_usw_get_tbl_id(lchip, ip_ver, 0, SYS_TBL_TCAM, &tbl_id, NULL);
        CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, tbl_id, &entry_num));
        if (entry_num > 0)
        {
            lookup_mode = SYS_TCAM_SA_LOOKUP;
            CTC_SET_FLAG(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], lookup_mode);
        }

        if (p_usw_ipuc_master[lchip]->max_entry_num[ip_ver][SYS_TBL_TCAM] || p_usw_ipuc_master[lchip]->max_entry_num[ip_ver][SYS_TBL_TCAM_SHORT])
        {
            lookup_mode = SYS_TCAM_LOOKUP;
            CTC_SET_FLAG(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], lookup_mode);
        }
        if (0 == p_usw_ipuc_master[lchip]->max_entry_num[ip_ver][SYS_TBL_TCAM] &&
            0 ==  p_usw_ipuc_master[lchip]->max_entry_num[ip_ver][SYS_TBL_TCAM_SHORT])
        {
            CTC_UNSET_FLAG(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_ALPM_LOOKUP);
        }

        if (p_usw_ipuc_master[lchip]->max_entry_num[ip_ver][SYS_TBL_TCAM_PUB])
        {
            lookup_mode = SYS_TCAM_PUB_LOOKUP;
            CTC_SET_FLAG(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], lookup_mode);
        }

        if (short_enable)
        {
            lookup_mode = SYS_TCAM_SHORT_LOOKUP;
            CTC_SET_FLAG(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_6], lookup_mode);
            CTC_SET_FLAG(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_6], SYS_TCAM_LOOKUP);
        }

        if (p_usw_ipuc_master[lchip]->lookup_mode[ip_ver])
        {
            p_usw_ipuc_master[lchip]->version_en[ip_ver] = TRUE;
        }
    }
    if(DRV_IS_TSINGMA(lchip))
    {
        p_usw_ipuc_master[lchip]->snake_per_group = 2;
    }
    else
    {
        p_usw_ipuc_master[lchip]->snake_per_group = TABLE_MAX_INDEX(lchip, DsNeoLpmIpv4Bit32Snake_t) / 16384;
    }

    return CTC_E_NONE;
}

int32
sys_usw_ipuc_set_route_spec(uint8 lchip, uint32 v4_spec, uint32 v6_spec)
{
    uint32 entry_num_v4 = 0;
    uint32 entry_num_v6_single = 0;
    uint32 entry_num_v6_double = 0;
    uint32 all = 0;

    LCHIP_CHECK(lchip);
    entry_num_v4 = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM];
    entry_num_v6_single = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT];
    entry_num_v6_double = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM];
    if(entry_num_v6_double && (v6_spec%4 != 0))
    {
        return CTC_E_INVALID_PARAM;
    }
    if(entry_num_v6_single && !entry_num_v6_double && (v6_spec%2 != 0))
    {
        return CTC_E_INVALID_PARAM;
    }

    all = entry_num_v4 + entry_num_v6_single + entry_num_v6_double;
    if (v4_spec > all || v6_spec > all)
    {
        return CTC_E_INVALID_PARAM;
    }
    if(!v4_spec && !v6_spec)
    {
        p_usw_ipuc_master[lchip]->lpm_tcam_spec[CTC_IP_VER_4] = entry_num_v4;
        p_usw_ipuc_master[lchip]->lpm_tcam_spec[CTC_IP_VER_6] = entry_num_v6_single + entry_num_v6_double;
    }
    if(v4_spec)
    {
        p_usw_ipuc_master[lchip]->lpm_tcam_spec[CTC_IP_VER_4] = v4_spec;
    }
    if(v6_spec)
    {
        p_usw_ipuc_master[lchip]->lpm_tcam_spec[CTC_IP_VER_6] = v6_spec;
    }

    if(!v4_spec)
    {
        p_usw_ipuc_master[lchip]->lpm_tcam_spec[CTC_IP_VER_4] = all - v6_spec;/*v6_spec musn`t be 0*/
    }

    if(!v6_spec)
    {
        if (entry_num_v6_double)
        {
            v6_spec = (all - v4_spec) / 4 * 4;
        }
        else
        {
            v6_spec = (all - v4_spec) / 2 * 2;
        }
        p_usw_ipuc_master[lchip]->lpm_tcam_spec[CTC_IP_VER_6] = v6_spec;/*v4_spec musn`t be 0*/
    }

    return CTC_E_NONE;
}



STATIC void
_sys_usw_ipuc_aid_timeout_handler(void* arg)
{
    uint8 lchip = (uintptr)arg;
    sal_systime_t current_tv;
    uint32 interval = 0;
    uint8 i = 0;

    sal_gettime(&current_tv);
    SYS_LIST_LOCK;
    interval = current_tv.tv_sec - p_usw_ipuc_master[lchip]->last_tv.tv_sec;

    if(3 == p_usw_ipuc_master[lchip]->alpm_async) /*5s*/
    {/*clear pending cache*/
        if (interval >= 2 && p_usw_ipuc_master[lchip]->pending_route_list->count)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "!!!!!!!!!!!current_tv.tv_sec: %u, interval: %u  ~~~~~~\n ", current_tv.tv_sec, interval);
            p_usw_ipuc_master[lchip]->alpm_async = 1;
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "~clear order detecting cache,and use async = 1  ~~~~~~\n ");
            for (i = 0; i < p_usw_ipuc_master[lchip]->pending_route_list->count; i++)
            {
                sal_sem_give(p_usw_ipuc_master[lchip]->alpm_sem);
            }
        }
    }
    else if((p_usw_ipuc_master[lchip]->alpm_async == 2) &&(interval >= 2)) /*1s*/
    {/*clear hash bucket cache*/
        if (p_usw_ipuc_master[lchip]->cbkt_status[0] == 1 )
        {
            p_usw_ipuc_master[lchip]->cbkt_status[0] = 2;
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"~~~~~~clear  cbuket 0 cache  cnt :%u  ~~~~~~\n ", p_usw_ipuc_master[lchip]->cbkt[0]->count);
            sal_sem_give(p_usw_ipuc_master[lchip]->alpm_sem);
        }
        else if(p_usw_ipuc_master[lchip]->cbkt_status[1] == 1 )
        {
            p_usw_ipuc_master[lchip]->cbkt_status[1] =  2;
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"~~~~~~ clear cbuket 1 cache  cnt :%u  ~~~~~~\n ", p_usw_ipuc_master[lchip]->cbkt[1]->count);
            sal_sem_give(p_usw_ipuc_master[lchip]->alpm_sem);
        }
    }
    SYS_LIST_UNLOCK;
}

int32
sys_usw_ipuc_set_aid(uint8 lchip, uint8 aid_en,uint8 aid_list_len, uint8 async)
{

    int32 ret = 0;
    uintptr lchip_tmp = lchip;
    sal_systime_t current_tv;
    ctc_ipuc_global_property_t global_prop;

    sal_memset(&global_prop, 0, sizeof(ctc_ipuc_global_property_t));

    global_prop.valid_flag = CTC_IP_GLB_PROP_ASYNC_EN;
    global_prop.async_en = async;

    CTC_ERROR_RETURN(sys_usw_ipuc_set_global_property(lchip, &global_prop));
    p_usw_ipuc_master[lchip]->aid_end = 0;
    p_usw_ipuc_master[lchip]->aid_list_len = aid_list_len;
    p_usw_ipuc_master[lchip]->aid_en = aid_en;


    if ( p_usw_ipuc_master[lchip]->aid_en )
    {
        uint8 index = 0;

        for (index = 0; index < 2 ; index++)
        {
            if ( !p_usw_ipuc_master[lchip]->cbkt[index] )
                p_usw_ipuc_master[lchip]->cbkt[index] = ctc_hash_create(16,
                                                                        1024,
                                                                        (hash_key_fn)_sys_usw_aid_hash_make,
                                                                        (hash_cmp_fn)_sys_usw_aid_hash_cmp);

        }

        if (!sys_usw_aid_timer)
        {
            ret = sal_timer_create(&sys_usw_aid_timer, _sys_usw_ipuc_aid_timeout_handler, (void*)lchip_tmp);
            if (0 != ret)
            {
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sal_timer_create failed  0x%x\n", ret);
                return ret ;
            }
        }
        else
        {
            sal_timer_stop(sys_usw_aid_timer);
        }
        sal_timer_start(sys_usw_aid_timer, 1);
        sal_gettime(&current_tv);
        p_usw_ipuc_master[lchip]->last_tv.tv_sec = current_tv.tv_sec;
    }
    else
    {
        if (sys_usw_aid_timer)
        {
            sal_timer_stop(sys_usw_aid_timer);
            sal_timer_destroy(sys_usw_aid_timer);
            sys_usw_aid_timer = NULL;
        }
    }
   return CTC_E_NONE;
}
int32
sys_usw_ipuc_show_aid_info(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "aid_en:%d\n", p_usw_ipuc_master[lchip]->aid_en);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "async:%d\n", p_usw_ipuc_master[lchip]->alpm_async);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "cbkt_status[0]:%u cbkt_status[1]:%u\n", p_usw_ipuc_master[lchip]->cbkt_status[0], p_usw_ipuc_master[lchip]->cbkt_status[1]);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "cbkt_idx[0]:%u cbkt_idx[1]:%u\n", p_usw_ipuc_master[lchip]->cbkt_idx[0], p_usw_ipuc_master[lchip]->cbkt_idx[1]);


    return CTC_E_NONE;
}

int32
sys_usw_ipuc_init(uint8 lchip, ctc_ipuc_global_cfg_t* p_ipuc_global_cfg)
{
    uint8 ipsa_enable = 0;
    int32  ret = 0;
    uint32 cmd = 0;
    uint32 offset = 0;
    uint32 field_value = 0;
    uint8 ip_ver = CTC_IP_VER_4;
    uint8 alloc_opf_flag[MAX_CTC_IP_VER] = {0};
    ds_t fib_engine_lookup_result_ctl;
    ds_t fib_engine_lookup_ctl;
    uint64 cpu_mask = 0;
    uint8  work_status = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if(NULL != p_usw_ipuc_master[lchip])
    {
        return CTC_E_NONE;
    }
    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
    }
    LCHIP_CHECK(lchip);

    sal_memset(&fib_engine_lookup_result_ctl, 0, sizeof(fib_engine_lookup_result_ctl));
    sal_memset(&fib_engine_lookup_ctl, 0, sizeof(fib_engine_lookup_ctl));

    p_usw_ipuc_master[lchip] = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_ipuc_master_t));
    if (NULL == p_usw_ipuc_master[lchip])
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        CTC_ERROR_RETURN(CTC_E_NO_MEMORY);
    }
    sal_memset(p_usw_ipuc_master[lchip], 0, sizeof(sys_ipuc_master_t));

    p_usw_ipuc_master[lchip]->arc[CTC_IP_VER_4] = 1;
    p_usw_ipuc_master[lchip]->arc[CTC_IP_VER_6] = 1;
    sal_mutex_create(&p_usw_ipuc_master[lchip]->mutex);
    if (NULL == p_usw_ipuc_master[lchip]->mutex)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto error0;
    }

    sal_mutex_create(&p_usw_ipuc_master[lchip]->lpm_mutex);
    if (NULL == p_usw_ipuc_master[lchip]->lpm_mutex)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret = CTC_E_NO_MEMORY;
        goto error0;
    }

    p_usw_ipuc_master[lchip]->host_lpm_mode = 1;
    p_usw_ipuc_master[lchip]->alpm_async = 0;
    p_usw_ipuc_master[lchip]->pending_list_len = 200*1024;
    p_usw_ipuc_master[lchip]->callback = p_ipuc_global_cfg->callback;
    p_usw_ipuc_master[lchip]->user_data = p_ipuc_global_cfg->user_data;

    CTC_ERROR_GOTO(_sys_usw_ipuc_resource_init(lchip), ret, error1);

    if ((p_usw_ipuc_master[lchip]->version_en[CTC_IP_VER_4] + p_usw_ipuc_master[lchip]->version_en[CTC_IP_VER_6]) == 0)
    {
        /* no resource for ipuc, not init ipuc */
        goto error1;
    }

    CTC_ERROR_GOTO(sys_usw_ipuc_db_init(lchip), ret, error1);

    cmd = DRV_IOR(FibEngineLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &fib_engine_lookup_ctl), ret, error2);

    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_PUB_LOOKUP))
    {
        SetFibEngineLookupCtl(V, privatePublicLookupModeEn_f, fib_engine_lookup_ctl, 1);    /* set public lookup */
    }

    SetFibEngineLookupCtl(V, hostUrpfEn_f, fib_engine_lookup_ctl, 1);    /*neighbor route support rpf*/

    /* calpm must init before tcam init, because tcam block init need calpm prefix mode*/
    if(CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_ALPM_LOOKUP))
    {
        ipsa_enable = CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_SA_LOOKUP);

        if (MCHIP_API(lchip)->ipuc_alpm_init)
        {
            CTC_ERROR_GOTO(MCHIP_API(lchip)->ipuc_alpm_init(lchip, p_ipuc_global_cfg->use_hash8, ipsa_enable), ret, error2);
            SetFibEngineLookupCtl(V, lpmPipelineEn_f, fib_engine_lookup_ctl, 1);
        }
    }
    if (sys_usw_chip_get_rchain_en())
    {
        p_usw_ipuc_master[lchip]->rchain_en = 1;
        p_usw_ipuc_master[lchip]->rchain_tail= sys_usw_get_local_chip_num()-1;
        sal_memset(&p_usw_ipuc_master[lchip]->rchain_boundary, 0, sizeof(uint16));
        if (lchip == 0)
        {
            p_usw_ipuc_master[lchip]->rchain_boundary = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM] + p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT] + p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM];
            p_usw_ipuc_master[lchip]->rchain_stats = mem_malloc(MEM_IPUC_MODULE, sizeof(uint32)*sys_usw_get_local_chip_num());
            sal_memset(p_usw_ipuc_master[lchip]->rchain_stats, 0, sizeof(uint32)*sys_usw_get_local_chip_num());
        }
        else
        {
            p_usw_ipuc_master[lchip]->rchain_boundary = p_usw_ipuc_master[lchip - 1]->rchain_boundary + SYS_IPUC_LPM_TCAM_MAX_INDEX;
        }
        if(0x1f == sys_usw_chip_get_rchain_gchip() || 0x7f == sys_usw_chip_get_rchain_gchip())
        {
            return CTC_E_INVALID_CHIP_ID;
        }
        p_usw_ipuc_master[lchip]->rchain_gchip = sys_usw_chip_get_rchain_gchip();
    }

    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_LOOKUP))
    {
        if (MCHIP_API(lchip)->ipuc_tcam_init)
        {
            CTC_ERROR_GOTO(MCHIP_API(lchip)->ipuc_tcam_init(lchip, SYS_PRIVATE_MODE, sys_usw_ipuc_tcam_move_cb), ret, error3);
        }
    }

    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_PUB_LOOKUP))
    {
        if (MCHIP_API(lchip)->ipuc_tcam_init)
        {
            CTC_ERROR_GOTO(MCHIP_API(lchip)->ipuc_tcam_init(lchip, SYS_PUBLIC_MODE, sys_usw_ipuc_tcam_move_cb), ret, error4);
        }
    }

    cmd = DRV_IOW(FibEngineLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &fib_engine_lookup_ctl), ret, error5);

    CTC_ERROR_GOTO(sys_usw_ip_tunnel_init(lchip), ret, error5);
    CTC_ERROR_GOTO(sys_usw_rpf_init(lchip), ret, error6);

    p_usw_ipuc_master[lchip]->arc[CTC_IP_VER_6] = (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM] ==0) ? 0 : 1;

    /* deal with default entry*/
    for (ip_ver = CTC_IP_VER_4; ip_ver < MAX_CTC_IP_VER; ip_ver++)
    {
        if (!p_usw_ipuc_master[lchip]->version_en[ip_ver])
        {
            continue;
        }
        if (p_usw_ipuc_master[lchip]->rchain_en && lchip != p_usw_ipuc_master[lchip]->rchain_tail)
        {
            CTC_ERROR_GOTO(sys_usw_ftm_alloc_table_offset(lchip, DsIpDa_t, 0, MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID)+1, 1, &offset), ret, error7);
        }
        else
        {
            CTC_ERROR_GOTO(sys_usw_ftm_alloc_table_offset(lchip, DsIpDa_t, 0, 1, 1, &offset), ret, error7);
        }
        alloc_opf_flag[ip_ver] = 1;

        p_usw_ipuc_master[lchip]->default_base[ip_ver] = offset;

        SYS_IP_CHECK_VERSION_ENABLE(ip_ver);
        CTC_ERROR_GOTO(_sys_usw_ipuc_add_default_entry(lchip, ip_ver, 0, SYS_NH_RESOLVED_NHID_FOR_DROP, 0), ret, error7);
        p_usw_ipuc_master[lchip]->default_nhid[ip_ver] = SYS_NH_RESOLVED_NHID_FOR_DROP;

        cmd = DRV_IOR(FibEngineLookupResultCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &fib_engine_lookup_result_ctl), ret, error7);

        if (CTC_IP_VER_4 == ip_ver)
        {
            cmd = DRV_IOW(FibEngineLookupResultCtl_t, DRV_ENTRY_FLAG);

            /* ucast da default entry base */
            SetFibEngineLookupResultCtl(V, gIpv4UcastLookupResultCtl_defaultEntryBase_f, fib_engine_lookup_result_ctl, offset);
            SetFibEngineLookupResultCtl(V, gIpv4UcastLookupResultCtl_defaultEntryEn_f, fib_engine_lookup_result_ctl, 1);

            /* ucast rpf sa default entry base */
            SetFibEngineLookupResultCtl(V, gIpv4RpfLookupResultCtl_defaultEntryBase_f, fib_engine_lookup_result_ctl, offset);
            SetFibEngineLookupResultCtl(V, gIpv4RpfLookupResultCtl_defaultEntryEn_f, fib_engine_lookup_result_ctl, 1);

            /* ucast default action don't care vrf*/
            SetFibEngineLookupResultCtl(V, gIpv4UcastLookupResultCtl_defaultEntryType_f, fib_engine_lookup_result_ctl, 1);

            CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &fib_engine_lookup_result_ctl), ret, error7);
        }
        else if (CTC_IP_VER_6 == ip_ver)
        {
            cmd = DRV_IOW(FibEngineLookupResultCtl_t, DRV_ENTRY_FLAG);

            /* ucast da default entry base */
            SetFibEngineLookupResultCtl(V, gIpv6UcastLookupResultCtl_defaultEntryBase_f, fib_engine_lookup_result_ctl, offset);
            SetFibEngineLookupResultCtl(V, gIpv6UcastLookupResultCtl_defaultEntryEn_f, fib_engine_lookup_result_ctl, 1);

            /* ucast rpf sa default entry base */
            SetFibEngineLookupResultCtl(V, gIpv6RpfLookupResultCtl_defaultEntryBase_f, fib_engine_lookup_result_ctl, offset);
            SetFibEngineLookupResultCtl(V, gIpv6RpfLookupResultCtl_defaultEntryEn_f, fib_engine_lookup_result_ctl, 1);

            /* ucast default action don't care vrf*/
            SetFibEngineLookupResultCtl(V, gIpv6UcastLookupResultCtl_defaultEntryType_f, fib_engine_lookup_result_ctl, 1);

            CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &fib_engine_lookup_result_ctl), ret, error7);
        }
    }

    /* other init */
    cmd = DRV_IOW(IpeRouteCtl_t, IpeRouteCtl_rpfCheckAgainstPort_f);
    field_value = p_ipuc_global_cfg->rpf_check_port;
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error7);

    /* SYSTEM MODIFIED by zhaoqd for support local arp proxy and icmp redirect, discard pkt forward, 2019-04-04 */
    cmd = DRV_IOW(IpeRouteCtl_t, IpeRouteCtl_discardIcmpRedirectPacket_f);
    field_value = 1;
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error7);
    
    cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_maxIpv4IcmpLengthEn_f);
    field_value = 0;
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error7);

    cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_maxIpv6IcmpLengthEn_f);
    field_value = 0;
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error7);

    cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_maxIcmpLengthEn_f);
    field_value = 0;
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error7);

    /*default using as cloud sec*/
    field_value = 1;
    cmd = DRV_IOW(IpeRouteCtl_t, IpeRouteCtl_cloudSecUseShareBit_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error7);

    field_value = 1;
    cmd = DRV_IOW(IpeLookupCtl_t, IpeLookupCtl_macSaLookupDisableForUrpf_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error7);

    /* SYSTEM MODIFIED by wangjj for support local arp proxy and icmp redirect, discard pkt forward, 2016-11-17 */
    cmd = DRV_IOW(IpeRouteCtl_t, IpeRouteCtl_discardIcmpRedirectPacket_f);
    field_value = 1;
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error7);

    sys_usw_ftm_register_callback(lchip, CTC_FTM_SPEC_IPUC_HOST,
                sys_usw_ipuc_host_ftm_cb);
    sys_usw_ftm_register_callback(lchip, CTC_FTM_SPEC_IPUC_LPM,
                sys_usw_ipuc_alpm_ftm_cb);

    CTC_ERROR_GOTO(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_IPUC, sys_usw_ipuc_wb_sync), ret, error7);

    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
        CTC_ERROR_GOTO(sys_usw_ipuc_wb_restore(lchip), ret, error7);
    }

    /*create alpm route list*/
    if(NULL == (p_usw_ipuc_master[lchip]->pending_route_list = ctc_slist_new()))
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        ret =  CTC_E_NO_MEMORY;
        goto error7;/*roll back todo */
    }

    /*update main thread*/
    cpu_mask = sys_usw_chip_get_affinity(lchip, 0);
    if (cpu_mask)
    {
        if(ctc_sal_task_set_affinity_prio(0, cpu_mask, 0) < 0)
        {
            ret = CTC_E_INIT_FAIL;
            goto error7;
        }
    }
    if (p_usw_ipuc_master[lchip]->rchain_en)
    {
        field_value = 0;
        if (lchip != p_usw_ipuc_master[lchip]->rchain_tail)
        {
            cmd = DRV_IOR(FibEngineLookupResultCtl_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &fib_engine_lookup_result_ctl);
            DRV_SET_FIELD_V(lchip, FibEngineLookupResultCtl_t,                                        \
            FibEngineLookupResultCtl_gIpv4UcastLookupResultCtl_defaultEntryType_f, \
            &fib_engine_lookup_result_ctl, field_value);
            DRV_SET_FIELD_V(lchip, FibEngineLookupResultCtl_t,                                        \
            FibEngineLookupResultCtl_gIpv6UcastLookupResultCtl_defaultEntryType_f, \
            &fib_engine_lookup_result_ctl, field_value);
            cmd = DRV_IOW(FibEngineLookupResultCtl_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, 0, cmd, &fib_engine_lookup_result_ctl);
        }
        if(0 != lchip)
        {
            field_value = 1;
            cmd = DRV_IOW(IpeHeaderAdjustCtl_t, IpeHeaderAdjustCtl_cFlexLocalPortMode_f);
            DRV_IOCTL(lchip, 0, cmd, &field_value);
        }
        _sys_usw_ipuc_write_dft_destmap(lchip);
    }
    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_IPUC, sys_usw_ipuc_dump_db), ret, error7);
    p_usw_ipuc_master[lchip]->alpm_thread_disable = 1;
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    return CTC_E_NONE;

error7:
    for (ip_ver = CTC_IP_VER_4; ip_ver < MAX_CTC_IP_VER; ip_ver++)
    {
        if (alloc_opf_flag[ip_ver])
        {
            if (p_usw_ipuc_master[lchip]->rchain_en)
            {
                sys_usw_ftm_free_table_offset(lchip, DsIpDa_t, 0, MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID), p_usw_ipuc_master[lchip]->default_base[ip_ver]);
            }
            else
            {
                sys_usw_ftm_free_table_offset(lchip, DsIpDa_t, 0, 1, p_usw_ipuc_master[lchip]->default_base[ip_ver]);
            }
        }
    }

error6:
    sys_usw_ip_tunnel_deinit(lchip);

error5:
    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_PUB_LOOKUP))
    {
        if (MCHIP_API(lchip)->ipuc_tcam_deinit)
        {
            MCHIP_API(lchip)->ipuc_tcam_deinit(lchip, SYS_PUBLIC_MODE);
        }
    }

error4:
    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_LOOKUP))
    {
        if (MCHIP_API(lchip)->ipuc_tcam_deinit)
        {
            MCHIP_API(lchip)->ipuc_tcam_deinit(lchip, SYS_PRIVATE_MODE);
        }
    }

error3:
    if(CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_ALPM_LOOKUP))
    {
        ipsa_enable = CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_SA_LOOKUP);

        if (MCHIP_API(lchip)->ipuc_alpm_deinit)
        {
            MCHIP_API(lchip)->ipuc_alpm_deinit(lchip);
        }
    }

error2:
    sys_usw_ipuc_db_deinit(lchip);

error1:
    if (p_usw_ipuc_master[lchip]->mutex)
    {
        sal_mutex_destroy(p_usw_ipuc_master[lchip]->mutex);
    }
    if (p_usw_ipuc_master[lchip]->lpm_mutex)
    {
        sal_mutex_destroy(p_usw_ipuc_master[lchip]->lpm_mutex);
    }

error0:
    mem_free(p_usw_ipuc_master[lchip]);

    return ret;
}

int32
sys_usw_ipuc_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == p_usw_ipuc_master[lchip])
    {
        return CTC_E_NONE;
    }

    if (MCHIP_API(lchip)->ipuc_alpm_deinit)
    {
        MCHIP_API(lchip)->ipuc_alpm_deinit(lchip);
    }

    sys_usw_ipuc_db_deinit(lchip);

    if (MCHIP_API(lchip)->ipuc_tcam_deinit)
    {
        MCHIP_API(lchip)->ipuc_tcam_deinit(lchip, SYS_PRIVATE_MODE);
        MCHIP_API(lchip)->ipuc_tcam_deinit(lchip, SYS_PUBLIC_MODE);
    }

    sys_usw_rpf_deinit(lchip);

    sys_usw_ip_tunnel_deinit(lchip);

    if (p_usw_ipuc_master[lchip]->mutex)
    {
        sal_mutex_destroy(p_usw_ipuc_master[lchip]->mutex);
    }

    if (p_usw_ipuc_master[lchip]->lpm_mutex)
    {
        sal_mutex_destroy(p_usw_ipuc_master[lchip]->lpm_mutex);
    }
    if(p_usw_ipuc_master[lchip]->p_alpm_task)
    {
        sal_task_destroy(p_usw_ipuc_master[lchip]->p_alpm_task);
        sal_sem_destroy(p_usw_ipuc_master[lchip]->alpm_sem);
    }
    ctc_slist_free(p_usw_ipuc_master[lchip]->pending_route_list);
    if (p_usw_ipuc_master[lchip]->rchain_en && 0 == lchip)
    {
        mem_free(p_usw_ipuc_master[lchip]->rchain_stats);
    }
    mem_free(p_usw_ipuc_master[lchip]);

    if (sys_usw_aid_timer)
    {
        sal_timer_stop(sys_usw_aid_timer);
        sal_timer_destroy(sys_usw_aid_timer);
        sys_usw_aid_timer = NULL;
    }
    return CTC_E_NONE;
}



#endif

