#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
/**
 @file sys_usw_ip_tunnel.c

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

#include "sys_usw_ftm.h"
#include "sys_usw_opf.h"
#include "sys_usw_chip.h"
#include "sys_usw_register.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_ip_tunnel.h"
#include "sys_usw_l3if.h"

#include "sys_usw_wb_common.h"
#include "sys_usw_learning_aging.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
enum sys_ip_tunnel_group_sub_type_e
{
    SYS_IP_TUNNEL_GROUP_SUB_TYPE_HASH,
    SYS_IP_TUNNEL_GROUP_SUB_TYPE_HASH1,
    SYS_IP_TUNNEL_GROUP_SUB_TYPE_TCAM0,
    SYS_IP_TUNNEL_GROUP_SUB_TYPE_TCAM1,
    SYS_IP_TUNNEL_GROUP_SUB_TYPE_NUM
};
typedef enum sys_ip_tunnel_group_sub_type_e sys_ip_tunnel_group_sub_type_t;

sys_ip_tunnel_master_t* p_usw_ip_tunnel_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

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

#define SYS_IPUC_TUNNEL_INIT_CHECK                                        \
    {                                                              \
        LCHIP_CHECK(lchip);                                        \
        if (p_usw_ip_tunnel_master[lchip] == NULL)                          \
        {                                                          \
            return CTC_E_NOT_INIT;                                 \
        }                                                          \
    }

#define SYS_IPUC_TUNNEL_CREAT_LOCK                   \
    do                                            \
    {                                             \
        sal_mutex_create(&p_usw_ip_tunnel_master[lchip]->mutex); \
        if (NULL == p_usw_ip_tunnel_master[lchip]->mutex)  \
        { \
            CTC_ERROR_RETURN(CTC_E_NO_MEMORY); \
        } \
    } while (0)

#define SYS_IPUC_TUNNEL_LOCK \
    if(p_usw_ip_tunnel_master[lchip]->mutex) sal_mutex_lock(p_usw_ip_tunnel_master[lchip]->mutex)

#define SYS_IPUC_TUNNEL_UNLOCK \
    if(p_usw_ip_tunnel_master[lchip]->mutex) sal_mutex_unlock(p_usw_ip_tunnel_master[lchip]->mutex)

#define CTC_ERROR_RETURN_IPUC_TUNNEL_UNLOCK(op) \
    do \
    { \
        int32 rv = (op); \
        if (rv < 0) \
        { \
            sal_mutex_unlock(p_usw_ip_tunnel_master[lchip]->mutex); \
            return (rv); \
        } \
    } while (0)

#define CTC_RETURN_IPUC_TUNNEL_UNLOCK(op) \
    do \
    { \
        sal_mutex_unlock(p_usw_ip_tunnel_master[lchip]->mutex); \
        return (op); \
    } while (0)

#define SYS_IPUC_TUNNEL_KEY_CHECK(val)                                            \
    {                                                                           \
        if ((CTC_FLAG_ISSET((val)->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY)\
            ||CTC_FLAG_ISSET((val)->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_CHECKSUM)\
            ||CTC_FLAG_ISSET((val)->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_SEQ_NUM))      \
            && \
            (((val)->payload_type != CTC_IPUC_TUNNEL_PAYLOAD_TYPE_GRE)       \
            ||(((val)->gre_key == 0)&&CTC_FLAG_ISSET((val)->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY)))){                                     \
                        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " [IPUC] IP-tunnel payload type %d is mismatch with grekey 0x%x\n", (val)->payload_type, (val)->gre_key);\
			return CTC_E_INVALID_PARAM;\
 		}                                       \
    }

#define SYS_IPUC_TUNNEL_AD_CHECK(val)                                                 \
    {                                                                               \
        if (CTC_FLAG_ISSET((val)->flag, CTC_IPUC_TUNNEL_FLAG_LKUP_BY_OUTER_HEAD)    \
            && ((val)->nh_id == 0)){                                                \
            return CTC_E_INVALID_PARAM; }                                           \
        if (!CTC_FLAG_ISSET((val)->flag, CTC_IPUC_TUNNEL_FLAG_INNER_VRF_EN)       \
            && (0 != (val)->vrf_id)){                                               \
            return CTC_E_INVALID_PARAM; }                                           \
    }

#define SYS_IPUC_TUNNEL_ADDRESS_SORT(val)             \
    {                                                   \
        if (CTC_IP_VER_6 == (val)->ip_ver)               \
        {                                               \
            uint32 t;                                   \
            t = val->ip_da.ipv6[0];                        \
            val->ip_da.ipv6[0] = val->ip_da.ipv6[3];          \
            val->ip_da.ipv6[3] = t;                        \
                                                    \
            t = val->ip_da.ipv6[1];                        \
            val->ip_da.ipv6[1] = val->ip_da.ipv6[2];          \
            val->ip_da.ipv6[2] = t;                        \
            if (CTC_FLAG_ISSET(val->flag,                \
                               CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA))   \
            {                                           \
                t = val->ip_sa.ipv6[0];                 \
                val->ip_sa.ipv6[0] = val->ip_sa.ipv6[3]; \
                val->ip_sa.ipv6[3] = t;                 \
                                                    \
                t = val->ip_sa.ipv6[1];                 \
                val->ip_sa.ipv6[1] = val->ip_sa.ipv6[2]; \
                val->ip_sa.ipv6[2] = t;                 \
            }                                           \
        }                                               \
    }

#define SYS_IPUC_TUNNEL_FUNC_DBG_DUMP(val)                                                    \
    {                                                                                           \
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);                         \
        if ((CTC_IP_VER_4 == val->ip_ver))                                                       \
        {                                                                                       \
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,                                             \
                             "route_flag: 0x%x  ip_ver: %s  ipda: 0x%x  ipsa: 0x%x\n",                                 \
                             val->flag, "IPv4", val->ip_da.ipv4,                                                 \
                             (CTC_FLAG_ISSET(val->flag, CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA)) ? val->ip_sa.ipv4 : 0);          \
        }                                                                                       \
        else                                                                                    \
        {                                                                                       \
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM,                                            \
                             "route_flag: 0x%x  ip_ver: %s  ipda: %x%x%x%x  ipsa: %x%x%x%x\n",                     \
                             val->flag, "IPv6", val->ip_da.ipv6[0], val->ip_da.ipv6[1],                              \
                             val->ip_da.ipv6[2], val->ip_da.ipv6[3],                                                \
                             val->ip_sa.ipv6[0], val->ip_sa.ipv6[1], val->ip_sa.ipv6[2], val->ip_sa.ipv6[3]);   \
        }                                                                                       \
    }

extern int32
_sys_usw_aging_get_host1_aging_ptr(uint8 lchip, uint32 key_index, uint32* aging_ptr);

/****************************************************************************
 *
* Function
*
*****************************************************************************/
STATIC uint32
_sys_usw_ip_tunnel_natsa_hash_make(sys_ip_tunnel_natsa_info_t* p_nat_info)
{
    uint32 a, b, c;

    /* Set up the internal state */
    a = b = c = 0xdeadbeef + (((uint32)96) << 2);

    a += p_nat_info->ipv4;
    b += p_nat_info->vrf_id;
    b += (p_nat_info->l4_src_port << 16);
    c += p_nat_info->is_tcp_port;
    MIX(a, b, c);

    return (c & IPUC_IPV4_HASH_MASK);
}

STATIC bool
_sys_usw_ip_tunnel_natsa_hash_cmp(sys_ip_tunnel_natsa_info_t* p_nat_info1, sys_ip_tunnel_natsa_info_t* p_nat_info)
{
    if (p_nat_info1->vrf_id != p_nat_info->vrf_id)
    {
        return FALSE;
    }

    if (p_nat_info1->l4_src_port != p_nat_info->l4_src_port)
    {
        return FALSE;
    }

    if (p_nat_info1->is_tcp_port != p_nat_info->is_tcp_port)
    {
        return FALSE;
    }

    if (p_nat_info1->ipv4 != p_nat_info->ipv4)
    {
        return FALSE;
    }

    return TRUE;
}

int32
_sys_usw_ip_tunnel_hash_free_node_data(void* node_data, void* user_data)
{
    mem_free(node_data);

    return CTC_E_NONE;
}

int32
_sys_usw_ip_tunnel_build_hash_natsa_key(uint8 lchip, sys_ip_tunnel_natsa_info_t* p_nat_info, DsFibHost1Ipv4NatSaPortHashKey_m* port_hash)
{
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    SetDsFibHost1Ipv4NatSaPortHashKey(V, hashType_f, port_hash, FIBHOST1PRIMARYHASHTYPE_IPV4);
    SetDsFibHost1Ipv4NatSaPortHashKey(V, l4PortType_f, port_hash, (p_nat_info->is_tcp_port ? 3 : 2));
    SetDsFibHost1Ipv4NatSaPortHashKey(V, dsAdIndex_f, port_hash, p_nat_info->ad_offset);
    SetDsFibHost1Ipv4NatSaPortHashKey(V, vrfId_f, port_hash, p_nat_info->vrf_id);
    SetDsFibHost1Ipv4NatSaPortHashKey(V, l4SourcePort_f, port_hash, p_nat_info->l4_src_port);
    SetDsFibHost1Ipv4NatSaPortHashKey(V, ipSa_f, port_hash, p_nat_info->ipv4);
    SetDsFibHost1Ipv4NatSaPortHashKey(V, ipv4Type_f, port_hash, 3 /* IPV4KEYTYPE_NATSA */);
    SetDsFibHost1Ipv4NatSaPortHashKey(V, valid_f, port_hash, 1);

    return CTC_E_NONE;
}

int32
_sys_usw_ip_tunnel_alloc_natsa_hash_key_index(uint8 lchip, sys_ip_tunnel_natsa_info_t* p_nat_info)
{
    drv_acc_in_t in;
    drv_acc_out_t out;
    DsFibHost1Ipv4NatSaPortHashKey_m port_hash;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));
    sal_memset(&port_hash, 0, sizeof(port_hash));

    _sys_usw_ip_tunnel_build_hash_natsa_key(lchip, p_nat_info, &port_hash);

    in.type = DRV_ACC_TYPE_LOOKUP;
    in.op_type = DRV_ACC_OP_BY_KEY;
    in.tbl_id = DsFibHost1Ipv4NatSaPortHashKey_t;
    in.data   = &port_hash;

    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    p_nat_info->in_tcam = out.is_conflict;
    if (!out.is_conflict)
    {
        if (out.key_index < 32)
        {
            p_nat_info->in_tcam = 1;
        }
        else
        {
            p_nat_info->key_offset = out.key_index;
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC NAT HASH: p_nat_info->key_offset:%d \n", p_nat_info->key_offset);
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_ip_tunnel_add_hash_natsa_key(uint8 lchip, sys_ip_tunnel_natsa_info_t* p_nat_info)
{
    drv_acc_in_t in;
    drv_acc_out_t out;
    DsFibHost1Ipv4NatSaPortHashKey_m port_hash;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));
    sal_memset(&port_hash, 0, sizeof(port_hash));

    _sys_usw_ip_tunnel_build_hash_natsa_key(lchip, p_nat_info, &port_hash);

    in.type = DRV_ACC_TYPE_ADD;
    in.op_type = DRV_ACC_OP_BY_INDEX;
    in.tbl_id = DsFibHost1Ipv4NatSaPortHashKey_t;
    in.index = p_nat_info->key_offset;
    in.data   = &port_hash;

    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    return CTC_E_NONE;
}

int32
_sys_usw_ip_tunnel_remove_hash_natsa_key(uint8 lchip, sys_ip_tunnel_natsa_info_t* p_nat_info)
{
    drv_acc_in_t in;
    drv_acc_out_t out;
    DsFibHost1Ipv4NatSaPortHashKey_m port_hash;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));
    sal_memset(&port_hash, 0, sizeof(port_hash));

    _sys_usw_ip_tunnel_build_hash_natsa_key(lchip, p_nat_info, &port_hash);

    in.type = DRV_ACC_TYPE_DEL;
    in.op_type = DRV_ACC_OP_BY_INDEX;
    in.tbl_id = DsFibHost1Ipv4NatSaPortHashKey_t;
    in.index = p_nat_info->key_offset;
    in.data   = &port_hash;

    CTC_ERROR_RETURN(drv_acc_api(lchip, &in, &out));

    return CTC_E_NONE;
}

int32
_sys_usw_ip_tunnel_write_nat_key(uint8 lchip, sys_ip_tunnel_natsa_info_t* p_nat_info)
{
    ds_t v4_key, v4_mask;
    ds_t dsad;
    tbl_entry_t tbl_ipkey;
    uint32 cmd;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&v4_key, 0, sizeof(v4_key));
    sal_memset(&v4_mask, 0, sizeof(v4_mask));
    sal_memset(&dsad, 0, sizeof(dsad));

    /* write ad */
    SetDsLpmTcamIpv4NatDoubleKeyAd(V, nexthop_f, dsad, p_nat_info->ad_offset);
    cmd = DRV_IOW(DsLpmTcamIpv4NatDoubleKeyAd_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, p_nat_info->key_offset*4, cmd, &dsad);

    /* write key */
    SetDsLpmTcamIpv4NatDoubleKey(V, lpmTcamKeyType_f, v4_key, 2);
    SetDsLpmTcamIpv4NatDoubleKey(V, ipSa_f, v4_key, p_nat_info->ipv4);
    SetDsLpmTcamIpv4NatDoubleKey(V, vrfId_f, v4_key, p_nat_info->vrf_id);

    SetDsLpmTcamIpv4NatDoubleKey(V, lpmTcamKeyType_f, v4_mask, 0x3);      /*tmp mask for spec is not sync */
    SetDsLpmTcamIpv4NatDoubleKey(V, ipSa_f, v4_mask, 0xFFFFFFFF);
    SetDsLpmTcamIpv4NatDoubleKey(V, vrfId_f, v4_mask, 0x7FF);

    if (p_nat_info->l4_src_port)
    {
        SetDsLpmTcamIpv4NatDoubleKey(V, l4SourcePort_f, v4_key, p_nat_info->l4_src_port);
        SetDsLpmTcamIpv4NatDoubleKey(V, isTcp_f, v4_key, p_nat_info->is_tcp_port);
        SetDsLpmTcamIpv4NatDoubleKey(V, isUdp_f, v4_key, !p_nat_info->is_tcp_port);
        SetDsLpmTcamIpv4NatDoubleKey(V, fragInfo_f, v4_key, 0);

        SetDsLpmTcamIpv4NatDoubleKey(V, l4SourcePort_f, v4_mask, 0xFFFF);
        SetDsLpmTcamIpv4NatDoubleKey(V, isTcp_f, v4_mask, 0x1);
        SetDsLpmTcamIpv4NatDoubleKey(V, isUdp_f, v4_mask, 0x1);
        SetDsLpmTcamIpv4NatDoubleKey(V, fragInfo_f, v4_mask, 0x3);
    }

    tbl_ipkey.data_entry = (uint32*)&v4_key;
    tbl_ipkey.mask_entry = (uint32*)&v4_mask;

    cmd = DRV_IOW(DsLpmTcamIpv4NatDoubleKey_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, p_nat_info->key_offset, cmd, &tbl_ipkey);

    return CTC_E_NONE;
}

int32
_sys_usw_ip_tunnel_remove_nat_key(uint8 lchip, sys_ip_tunnel_natsa_info_t* p_nat_info)
{
    uint32 cmd = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Remove IPUC NAT: p_nat_info->key_offset:%d \n", p_nat_info->key_offset);

    cmd = DRV_IOD(DsLpmTcamIpv4DoubleKey_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, p_nat_info->key_offset, cmd, &cmd);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ip_tunnel_alloc_nat_tcam_ad_index(uint8 lchip, sys_ip_tunnel_natsa_info_t* p_nat_info)
{
    sys_usw_opf_t opf;
    uint32 ad_offset = 0;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type = p_usw_ip_tunnel_master[lchip]->opf_type_ipv4_nat;

    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, 1, &ad_offset));

    p_nat_info->key_offset = ad_offset;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ip_tunnel_free_nat_tcam_ad_index(uint8 lchip, sys_ip_tunnel_natsa_info_t* p_nat_info)
{
    sys_usw_opf_t opf;
    uint32 ad_offset = 0;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type = p_usw_ip_tunnel_master[lchip]->opf_type_ipv4_nat;

    ad_offset = p_nat_info->key_offset;

    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, 1, ad_offset));

    return CTC_E_NONE;
}

int32
_sys_usw_ip_tunnel_natsa_write_ipsa(uint8 lchip, sys_ip_tunnel_natsa_info_t* p_nat_info)
{
    uint32 cmd = 0;
    ds_t dsnatsa;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&dsnatsa, 0, sizeof(dsnatsa));

    SetDsIpSaNat(V, replaceIpSa_f, dsnatsa, 1);
    SetDsIpSaNat(V, ipSa_f, dsnatsa, p_nat_info->new_ipv4);
    if (p_nat_info->l4_src_port)
    {
        SetDsIpSaNat(V, replaceL4SourcePort_f, dsnatsa, 1);
        SetDsIpSaNat(V, l4SourcePort_f, dsnatsa, p_nat_info->new_l4_src_port);
    }

    cmd = DRV_IOW(DsIpSaNat_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, p_nat_info->ad_offset, cmd, &dsnatsa));

    return CTC_E_NONE;
}

int32
_sys_usw_ip_tunnel_natsa_db_add(uint8 lchip, sys_ip_tunnel_natsa_info_t* p_nat_info)
{
    ctc_hash_insert(p_usw_ip_tunnel_master[lchip]->nat_hash, p_nat_info);

    return CTC_E_NONE;
}

int32
_sys_usw_ip_tunnel_natsa_db_remove(uint8 lchip, sys_ip_tunnel_natsa_info_t* p_nat_info)
{
    ctc_hash_remove(p_usw_ip_tunnel_master[lchip]->nat_hash, p_nat_info);
    mem_free(p_nat_info);

    return CTC_E_NONE;
}

int32
_sys_usw_ip_tunnel_natsa_db_lookup(uint8 lchip, sys_ip_tunnel_natsa_info_t** pp_ip_tunnel_natsa_info)
{
    sys_ip_tunnel_natsa_info_t* p_ip_tunnel_natsa_info = NULL;

    p_ip_tunnel_natsa_info = ctc_hash_lookup(p_usw_ip_tunnel_master[lchip]->nat_hash, *pp_ip_tunnel_natsa_info);

    *pp_ip_tunnel_natsa_info = p_ip_tunnel_natsa_info;

    return CTC_E_NONE;
}

int32
sys_usw_ip_tunnel_add_natsa_default_entry(uint8 lchip)
{
    uint32 cmd;
    int32 ret = CTC_E_NONE;
    uint32 entry_num = 0;
    uint32 dsfwd_offset;
    uint32 ad_offset = 0;
    ds_t fib_engine_lookup_result_ctl;
    ds_t dsnatsa;

    SYS_IPUC_TUNNEL_INIT_CHECK;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&dsnatsa, 0, sizeof(dsnatsa));
    sal_memset(&fib_engine_lookup_result_ctl, 0, sizeof(fib_engine_lookup_result_ctl));

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsLpmTcamIpv4NatDoubleKey_t, &entry_num));
    CTC_ERROR_RETURN(sys_usw_ftm_alloc_table_offset(lchip, DsIpDa_t, 0, 1, 1, &ad_offset));

    /* write natsa */
    CTC_ERROR_GOTO(sys_usw_nh_get_dsfwd_offset(lchip, SYS_NH_RESOLVED_NHID_FOR_TOCPU, &dsfwd_offset, 0), ret, error0);
    SetDsIpSaNat(V, ipSaFwdPtrValid_f, dsnatsa, 1);
    SetDsIpSaNat(V, dsFwdPtrOrEcmpGroupId_f, dsnatsa, dsfwd_offset);

    cmd = DRV_IOW(DsIpSaNat_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, ad_offset, cmd, &dsnatsa);

    cmd = DRV_IOR(FibEngineLookupResultCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &fib_engine_lookup_result_ctl), ret, error0);

    /* ucast da default entry base */
    SetFibEngineLookupResultCtl(V, gIpv4NatSaLookupResultCtl_defaultEntryBase_f, fib_engine_lookup_result_ctl, ad_offset);
    SetFibEngineLookupResultCtl(V, gIpv4NatSaLookupResultCtl_defaultEntryEn_f, fib_engine_lookup_result_ctl, 1);

    /* ucast default action don't care vrf*/
    SetFibEngineLookupResultCtl(V, gIpv4NatSaLookupResultCtl_defaultEntryType_f, fib_engine_lookup_result_ctl, 1);

    cmd = DRV_IOW(FibEngineLookupResultCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &fib_engine_lookup_result_ctl), ret, error0);

    return CTC_E_NONE;

error0:
    sys_usw_ftm_free_table_offset(lchip, DsIpDa_t, 0, 1, ad_offset);

    return ret;
}

int32
sys_usw_ip_tunnel_add_natsa(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ip_tunnel_natsa_param)
{
    int32  ret = CTC_E_NONE;
    uint32 table_id = DsIpDa_t;
    uint8  do_hash = 1;
    uint32 ad_offset = 0;
    sys_ip_tunnel_natsa_info_t nat_info;
    sys_ip_tunnel_natsa_info_t* p_nat_info = NULL;

    SYS_IPUC_TUNNEL_INIT_CHECK;

    /* param check and debug out */
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    CTC_PTR_VALID_CHECK(p_ip_tunnel_natsa_param);
    CTC_IP_VER_CHECK(p_ip_tunnel_natsa_param->ip_ver);
    CTC_MAX_VALUE_CHECK(p_ip_tunnel_natsa_param->ip_ver, CTC_IP_VER_4)
    SYS_IP_VRFID_CHECK(p_ip_tunnel_natsa_param->vrf_id, p_ip_tunnel_natsa_param->ip_ver);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "vrf_id: %d  flag: 0x%x  ipsa: 0x%x  new ipsa: 0x%x\n",
                             p_ip_tunnel_natsa_param->vrf_id, p_ip_tunnel_natsa_param->flag,
                             p_ip_tunnel_natsa_param->ipsa.ipv4, p_ip_tunnel_natsa_param->new_ipsa.ipv4);

    /* prepare data */
    sal_memset(&nat_info, 0, sizeof(nat_info));
    p_nat_info = &nat_info;

    SYS_IPUC_NAT_KEY_MAP(p_ip_tunnel_natsa_param, p_nat_info);

    SYS_IPUC_TUNNEL_LOCK;
    /* 1. lookup sw node */
    CTC_ERROR_RETURN_IPUC_TUNNEL_UNLOCK(_sys_usw_ip_tunnel_natsa_db_lookup(lchip, &p_nat_info));
    if (NULL != p_nat_info)
    {
        CTC_ERROR_RETURN_IPUC_TUNNEL_UNLOCK(CTC_E_EXIST);
    }

    p_nat_info = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_ip_tunnel_natsa_info_t));
    if (NULL == p_nat_info)
    {
        CTC_ERROR_RETURN_IPUC_TUNNEL_UNLOCK(CTC_E_NO_MEMORY);
    }
    sal_memset(p_nat_info, 0, sizeof(sys_ip_tunnel_natsa_info_t));

    SYS_IPUC_NAT_KEY_MAP(p_ip_tunnel_natsa_param, p_nat_info);
    SYS_IPUC_NAT_DATA_MAP(p_ip_tunnel_natsa_param, p_nat_info);

    /* check hash enable */
    if ((!CTC_FLAG_ISSET(p_usw_ip_tunnel_master[lchip]->lookup_mode[p_ip_tunnel_natsa_param->ip_ver], SYS_NAT_HASH_LOOKUP))
        || (p_nat_info->l4_src_port == 0))
    {
        do_hash = FALSE;
    }

    /* 2. alloc key offset */
    if (do_hash)
    {
        if ((p_usw_ip_tunnel_master[lchip]->snat_hash_count + p_usw_ip_tunnel_master[lchip]->napt_hash_count) >=
            SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_NAPT))
        {
            mem_free(p_nat_info);
            CTC_ERROR_RETURN_IPUC_TUNNEL_UNLOCK(CTC_E_NO_RESOURCE);
        }

        ret = _sys_usw_ip_tunnel_alloc_natsa_hash_key_index(lchip, p_nat_info);
        if (ret)
        {
            goto error2;
        }

        if (p_nat_info->in_tcam)
        {
            do_hash = 0;
        }
    }

    if (!do_hash)
    {
        if (!CTC_FLAG_ISSET(p_usw_ip_tunnel_master[lchip]->lookup_mode[p_ip_tunnel_natsa_param->ip_ver], SYS_NAT_TCAM_LOOKUP))
        {
            ret = CTC_E_NO_RESOURCE;
            goto error2;
        }

        CTC_ERROR_GOTO(_sys_usw_ip_tunnel_alloc_nat_tcam_ad_index(lchip, p_nat_info), ret, error2);
        p_nat_info->in_tcam = 1;
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC NAT TCAM: p_nat_info->key_offset:%d \n", p_nat_info->key_offset);
    }

    /* 3. alloc ad offset */
    CTC_ERROR_GOTO(sys_usw_ftm_alloc_table_offset(lchip, table_id, 0, 1, 1, &ad_offset), ret, error1);
    p_nat_info->ad_offset = ad_offset;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC NAT ad_offset: p_nat_info->ad_offset:%d \n", p_nat_info->ad_offset);

    /* 4. write nat sa */
    CTC_ERROR_GOTO(_sys_usw_ip_tunnel_natsa_write_ipsa(lchip, p_nat_info), ret, error0);

    /* 5. write nat key (if do_hash = true, write hash key to asic, else write Tcam key) */
    if (do_hash)
    {
        /* write ipuc hash key entry */
        CTC_ERROR_GOTO(_sys_usw_ip_tunnel_add_hash_natsa_key(lchip, p_nat_info), ret, error0);

        p_usw_ip_tunnel_master[lchip]->snat_hash_count ++;
    }
    else
    {
        CTC_ERROR_GOTO(_sys_usw_ip_tunnel_write_nat_key(lchip, p_nat_info), ret, error0);

        p_usw_ip_tunnel_master[lchip]->snat_tcam_count ++;
    }

    /* 6. write to soft table */
    _sys_usw_ip_tunnel_natsa_db_add(lchip, p_nat_info);

    CTC_RETURN_IPUC_TUNNEL_UNLOCK(CTC_E_NONE);

error0:
    sys_usw_ftm_free_table_offset(lchip, table_id, 0, 1, ad_offset);

error1:
    if (!do_hash)
    {
        _sys_usw_ip_tunnel_free_nat_tcam_ad_index(lchip, p_nat_info);
    }

error2:
    mem_free(p_nat_info);
    CTC_RETURN_IPUC_TUNNEL_UNLOCK(ret);
}

int32
sys_usw_ip_tunnel_remove_natsa(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ip_tunnel_natsa_param)
{
    sys_ip_tunnel_natsa_info_t nat_info;
    sys_ip_tunnel_natsa_info_t* p_nat_info = NULL;

    SYS_IPUC_TUNNEL_INIT_CHECK;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    CTC_PTR_VALID_CHECK(p_ip_tunnel_natsa_param);
    CTC_IP_VER_CHECK(p_ip_tunnel_natsa_param->ip_ver);
    CTC_MAX_VALUE_CHECK(p_ip_tunnel_natsa_param->ip_ver, CTC_IP_VER_4)
    SYS_IP_VRFID_CHECK(p_ip_tunnel_natsa_param->vrf_id, p_ip_tunnel_natsa_param->ip_ver);
    /*SYS_IP_FUNC_DBG_DUMP(p_ip_tunnel_param);*/

    /* prepare data */
    sal_memset(&nat_info, 0, sizeof(nat_info));
    p_nat_info = &nat_info;

    SYS_IPUC_NAT_KEY_MAP(p_ip_tunnel_natsa_param, p_nat_info);

    SYS_IPUC_TUNNEL_LOCK;
    /* lookup for ipuc entrise */
    CTC_ERROR_RETURN_IPUC_TUNNEL_UNLOCK(_sys_usw_ip_tunnel_natsa_db_lookup(lchip, &p_nat_info));
    if (!p_nat_info)
    {
        CTC_RETURN_IPUC_TUNNEL_UNLOCK(CTC_E_NOT_EXIST);
    }

    if (p_nat_info->in_tcam)
    {   /*do TCAM remove*/
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "TCAM remove\n");

        /* remove nat key entry from TCAM */
        _sys_usw_ip_tunnel_remove_nat_key(lchip, p_nat_info);

        /* TCAM do not need to build remove offset, only need to clear soft offset info */
        _sys_usw_ip_tunnel_free_nat_tcam_ad_index(lchip, p_nat_info);
    }
    else
    {   /* do HASH remove */
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "HASH remove\n");

        /* remove hash key entry */
        _sys_usw_ip_tunnel_remove_hash_natsa_key(lchip, p_nat_info);
    }

    /* free ad offset */
    sys_usw_ftm_free_table_offset(lchip, DsIpDa_t, 0, 1, p_nat_info->ad_offset);

    /* decrease count */
    if (p_nat_info->in_tcam)
    {
        p_usw_ip_tunnel_master[lchip]->snat_tcam_count --;
    }
    else
    {
        p_usw_ip_tunnel_master[lchip]->snat_hash_count--;
    }

    /* write to soft table */
    _sys_usw_ip_tunnel_natsa_db_remove(lchip, p_nat_info);

    SYS_IPUC_TUNNEL_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_ip_tunnel_set_natsa_entry_hit(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ip_tunnel_natsa_param, uint8 hit)
{
    uint32 aging_ptr = 0;
    sys_ip_tunnel_natsa_info_t nat_info_buf;
    sys_ip_tunnel_natsa_info_t* p_nat_info = &nat_info_buf;
    drv_acc_in_t in;
    drv_acc_out_t out;
    uint32 aging_tcam_base = 0;

    aging_tcam_base = DRV_IS_DUET2(lchip) ? 8*1024 : 16*1024;
    sal_memset(&in, 0, sizeof(drv_acc_in_t));
    sal_memset(&out, 0, sizeof(drv_acc_out_t));
    sal_memset(p_nat_info, 0, sizeof(sys_ip_tunnel_natsa_info_t));

    SYS_IPUC_NAT_KEY_MAP(p_ip_tunnel_natsa_param, p_nat_info);
    SYS_IPUC_NAT_DATA_MAP(p_ip_tunnel_natsa_param, p_nat_info);

    SYS_IPUC_TUNNEL_LOCK;
    CTC_ERROR_RETURN_IPUC_TUNNEL_UNLOCK(_sys_usw_ip_tunnel_natsa_db_lookup(lchip, &p_nat_info));
    if (!p_nat_info)
    {
        CTC_RETURN_IPUC_TUNNEL_UNLOCK(CTC_E_NOT_EXIST);
    }

    /* check hash enable */
	/*SYSTEM MODIFIED by taocy for SDK bug 107372*/
    if (((!CTC_FLAG_ISSET(p_usw_ip_tunnel_master[lchip]->lookup_mode[p_ip_tunnel_natsa_param->ip_ver], SYS_NAT_HASH_LOOKUP))
        && (0 == p_nat_info->in_tcam)) ||
        ((!CTC_FLAG_ISSET(p_usw_ip_tunnel_master[lchip]->lookup_mode[p_ip_tunnel_natsa_param->ip_ver], SYS_NAT_TCAM_LOOKUP))
        && (1 == p_nat_info->in_tcam)) ||
        ((1 == p_nat_info->in_tcam) && (1 == hit)))
	/*SYSTEM MODIFIED by taocy for SDK bug 107372*/
    {
        SYS_IPUC_TUNNEL_UNLOCK;
        return CTC_E_INVALID_PARAM;
    }

    /*SYSTEM MODIFIED by taocy for SDK bug 107372*/
    if (p_nat_info->in_tcam)
    {
        uint8 timer = 0;
        CTC_ERROR_RETURN_IPUC_TUNNEL_UNLOCK(sys_usw_aging_get_aging_timer(lchip, SYS_AGING_DOMAIN_TCAM, p_nat_info->key_offset, &timer));
        CTC_ERROR_RETURN_IPUC_TUNNEL_UNLOCK(sys_usw_aging_set_aging_status(lchip, SYS_AGING_DOMAIN_TCAM, p_nat_info->key_offset, timer, hit));
        SYS_IPUC_TUNNEL_UNLOCK;
        return CTC_E_NONE;
    }
	/*SYSTEM MODIFIED by taocy for SDK bug 107372*/
    CTC_ERROR_RETURN_IPUC_TUNNEL_UNLOCK(_sys_usw_aging_get_host1_aging_ptr(lchip, p_nat_info->key_offset, &aging_ptr));

    CTC_BIT_SET(in.flag, DRV_ACC_AGING_EN);
    in.type = DRV_ACC_TYPE_UPDATE;
    in.op_type = DRV_ACC_OP_BY_INDEX;
    in.index =  aging_ptr - aging_tcam_base;
    in.timer_index = 0;
    in.data = &hit;
    in.tbl_id = DsAging_t;
    CTC_ERROR_RETURN_IPUC_TUNNEL_UNLOCK(drv_acc_api(lchip, &in, &out));

    SYS_IPUC_TUNNEL_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_ip_tunnel_get_natsa_entry_hit(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_ip_tunnel_natsa_param, uint8* hit)
{
    uint32 aging_ptr = 0;
    sys_ip_tunnel_natsa_info_t nat_info_buf;
    sys_ip_tunnel_natsa_info_t* p_nat_info = &nat_info_buf;
    drv_acc_in_t in;
    drv_acc_out_t out;
    uint32 aging_tcam_base = 0;
    DsAgingStatus_m   ds_aging_status;
    uint32            aging_status = 0;

    aging_tcam_base = DRV_IS_DUET2(lchip) ? 8*1024 : 16*1024;
    sal_memset(&in, 0, sizeof(drv_acc_in_t));
    sal_memset(&out, 0, sizeof(drv_acc_out_t));
    sal_memset(p_nat_info, 0, sizeof(sys_ip_tunnel_natsa_info_t));

    SYS_IPUC_NAT_KEY_MAP(p_ip_tunnel_natsa_param, p_nat_info);
    SYS_IPUC_NAT_DATA_MAP(p_ip_tunnel_natsa_param, p_nat_info);

    SYS_IPUC_TUNNEL_LOCK;
    CTC_ERROR_RETURN_IPUC_TUNNEL_UNLOCK(_sys_usw_ip_tunnel_natsa_db_lookup(lchip, &p_nat_info));
    if (!p_nat_info)
    {
        CTC_RETURN_IPUC_TUNNEL_UNLOCK(CTC_E_NOT_EXIST);
    }

    /* check hash enable */
	/*SYSTEM MODIFIED by taocy for SDK bug 107372*/
    if (((!CTC_FLAG_ISSET(p_usw_ip_tunnel_master[lchip]->lookup_mode[p_ip_tunnel_natsa_param->ip_ver], SYS_NAT_HASH_LOOKUP))
        && (0 == p_nat_info->in_tcam)) ||
        ((!CTC_FLAG_ISSET(p_usw_ip_tunnel_master[lchip]->lookup_mode[p_ip_tunnel_natsa_param->ip_ver], SYS_NAT_TCAM_LOOKUP))
        && (1 == p_nat_info->in_tcam)))
	/*SYSTEM MODIFIED by taocy for SDK bug 107372*/
    {
        SYS_IPUC_TUNNEL_UNLOCK;
        return CTC_E_INVALID_PARAM;
    }

	/*SYSTEM MODIFIED by taocy for SDK bug 107372*/
    if (1 == p_nat_info->in_tcam)
    {
        CTC_ERROR_RETURN_IPUC_TUNNEL_UNLOCK(sys_usw_aging_get_aging_status(lchip, SYS_AGING_DOMAIN_TCAM, p_nat_info->key_offset, hit));
        SYS_IPUC_TUNNEL_UNLOCK;
        return CTC_E_NONE;
    }
	/*SYSTEM MODIFIED by taocy for SDK bug 107372*/

    CTC_ERROR_RETURN_IPUC_TUNNEL_UNLOCK(_sys_usw_aging_get_host1_aging_ptr(lchip, p_nat_info->key_offset, &aging_ptr));
    aging_ptr -= aging_tcam_base;

    CTC_ERROR_RETURN_IPUC_TUNNEL_UNLOCK(DRV_IOCTL(lchip, aging_ptr >> 5,
                               DRV_IOR(DsAgingStatusFib_t, DRV_ENTRY_FLAG), &ds_aging_status));

    aging_status = GetDsAgingStatusFib(V, array_0_agingStatus_f + (aging_ptr & 0x1f),  &ds_aging_status);

    *hit = (uint8)aging_status;

    
    SYS_IPUC_TUNNEL_UNLOCK;
    
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ip_tunnel_natsa_db_traverse(uint8 lchip, uint8 ip_ver, hash_traversal_fn fn, void* data)
{
    return ctc_hash_traverse_through(p_usw_ip_tunnel_master[lchip]->nat_hash, fn, data);
}

STATIC int32
_sys_usw_ip_tunnel_show_natsa_info(sys_ip_tunnel_natsa_info_t* p_nat_info, void* data)
{
    char buf[CTC_IPV6_ADDR_STR_LEN] = {0};
    uint32 tempip = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8d", p_nat_info->vrf_id);

    tempip = sal_ntohl(p_nat_info->ipv4);
    sal_inet_ntop(AF_INET, &tempip, buf, CTC_IPV6_ADDR_STR_LEN);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-18s", buf);

    if (p_nat_info->l4_src_port)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-12u", p_nat_info->l4_src_port);
    }
    else
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-12s", "-");
    }

    tempip = sal_ntohl(p_nat_info->new_ipv4);
    sal_inet_ntop(AF_INET, &tempip, buf, CTC_IPV6_ADDR_STR_LEN);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-18s", buf);

    if (p_nat_info->new_l4_src_port)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10u", p_nat_info->new_l4_src_port);
    }
    else
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s", "-");
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s%-10s%-10u%u\n",
                      (p_nat_info->l4_src_port ? (p_nat_info->is_tcp_port ? "TCP" : "UDP") : "-"),
                      (p_nat_info->in_tcam ? "T" : "S"),(p_nat_info->key_offset),(p_nat_info->ad_offset));

    return CTC_E_NONE;
}

int32
sys_usw_ip_tunnel_show_natsa_db(uint8 lchip, ctc_ip_ver_t ip_ver, uint32 detail)
{
    LCHIP_CHECK(lchip);
    SYS_IPUC_TUNNEL_INIT_CHECK;

    if (ip_ver == CTC_IP_VER_4)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "TcamKey Table : DsLpmTcamIpv4NatDoubleKey\n\r");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "SramKey Table : DsFibHost1Ipv4NatSaPortHashKey\n\r");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Ad Table      : DsIpSaNat\n\r");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "In-SRAM: T-TCAM    S-SRAM\n\r");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "T-TCAM Count: %u   S-SRAM Count: %u\n\r", p_usw_ip_tunnel_master[lchip]->snat_tcam_count, p_usw_ip_tunnel_master[lchip]->snat_hash_count);
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-8s%-18s%-12s%-18s%-10s%-10s%-10s%-10s%s\n",
                          "VRF", "IPSA", "Port", "New-IPSA", "New-Port", "TCP/UDP", "In-SRAM", "Key-Index", "AD-Index");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------------------------------------------------------------------------\n");
    }
    else
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not supported \n");
        return CTC_E_NOT_SUPPORT;
    }

    _sys_usw_ip_tunnel_natsa_db_traverse(lchip, ip_ver, (hash_traversal_fn)_sys_usw_ip_tunnel_show_natsa_info, (void*)&detail);
    return CTC_E_NONE;
}

/*SYSTEM MODIFIED by taocy for SDK bug 107372*/
int32
sys_usw_ip_tunnel_show_natsa(uint8 lchip, ctc_ipuc_nat_sa_param_t* p_nat_info)
{
    uint32 temp = 0;
    sys_ip_tunnel_natsa_info_t nat_info;
    sys_ip_tunnel_natsa_info_t* p_temp_nat_info = &nat_info;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    CTC_PTR_VALID_CHECK(p_nat_info);
    CTC_IP_VER_CHECK(p_nat_info->ip_ver);
    CTC_MAX_VALUE_CHECK(p_nat_info->ip_ver, CTC_IP_VER_4)
    SYS_IP_VRFID_CHECK(p_nat_info->vrf_id, p_nat_info->ip_ver);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "vrf_id: %d  flag: 0x%x  ipsa: 0x%x  new ipsa: 0x%x\n",
                             p_nat_info->vrf_id, p_nat_info->flag,
                             p_nat_info->ipsa.ipv4, p_nat_info->new_ipsa.ipv4);

    sal_memset(&nat_info, 0, sizeof(sys_ip_tunnel_natsa_info_t));

    SYS_IPUC_NAT_KEY_MAP(p_nat_info, p_temp_nat_info);
    CTC_ERROR_RETURN(_sys_usw_ip_tunnel_natsa_db_lookup(lchip, &p_temp_nat_info));
    if (!p_temp_nat_info)
    {
        return CTC_E_ENTRY_NOT_EXIST;
    }
    _sys_usw_ip_tunnel_show_natsa_info(p_temp_nat_info, &temp);
    return CTC_E_NONE;
}
/*SYSTEM MODIFIED by taocy for SDK bug 107372*/

int32
sys_usw_ip_tunnel_ftm_cb(uint8 lchip, sys_ftm_specs_info_t* specs_info)
{
    LCHIP_CHECK(lchip);
    SYS_IPUC_TUNNEL_INIT_CHECK;
    CTC_PTR_VALID_CHECK(specs_info);

    specs_info->used_size = p_usw_ip_tunnel_master[lchip]->snat_hash_count + p_usw_ip_tunnel_master[lchip]->napt_hash_count;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ip_tunnel_check(uint8 lchip, ctc_ipuc_tunnel_param_t* p_ip_tunnel_param)
{
    CTC_PTR_VALID_CHECK(p_ip_tunnel_param);
    CTC_IP_VER_CHECK(p_ip_tunnel_param->ip_ver);
    SYS_IPUC_TUNNEL_KEY_CHECK(p_ip_tunnel_param);
    SYS_IPUC_TUNNEL_AD_CHECK(p_ip_tunnel_param);
    CTC_MAX_VALUE_CHECK(p_ip_tunnel_param->scl_id, 1);
    CTC_MAX_VALUE_CHECK(p_ip_tunnel_param->payload_type, MAX_CTC_IPUC_TUNNEL_PAYLOAD_TYPE-1);
    SYS_USW_CID_CHECK(lchip,p_ip_tunnel_param->cid);
    CTC_MAX_VALUE_CHECK(p_ip_tunnel_param->logic_port, MCHIP_CAP(SYS_CAP_NH_MAX_LOGIC_DEST_PORT));

    if (p_ip_tunnel_param->service_id)
    {
        return CTC_E_NOT_SUPPORT;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ip_tunnel_flag_check(uint8 lchip, ctc_ipuc_tunnel_param_t* p_ip_tunnel_param)
{
    if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_V4_RPF_CHECK) &&
        CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_RPF_CHECK_DISABLE))
    {
        return CTC_E_PARAM_CONFLICT;
    }

    if (p_ip_tunnel_param->cid &&
        !CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_RPF_CHECK_DISABLE))
    {
        return CTC_E_PARAM_CONFLICT;
    }

    if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_USE_OUTER_TTL) &&
        p_ip_tunnel_param->cid)
    {
        return CTC_E_PARAM_CONFLICT;
    }

    if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_INNER_VRF_EN) &&
        CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_ACL_EN) &&
        CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_LKUP_BY_OUTER_HEAD))
    {
        return CTC_E_PARAM_CONFLICT;
    }

    if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_STATS_EN) &&
        CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_METADATA_EN))
    {
        return CTC_E_PARAM_CONFLICT;
    }

    return CTC_E_NONE;
}

STATIC uint32
_sys_usw_ip_tunnel_get_tunnel_key_type(uint8 lchip, ctc_ipuc_tunnel_param_t* p_ip_tunnel_param, uint8* p_key_type, uint8* is_hash_key, uint8* resolve_conflict)
{
    *is_hash_key = 1;
    if (CTC_IP_VER_4 == p_ip_tunnel_param->ip_ver)
    {
        if(CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_V4_RPF_CHECK))
        {
            *p_key_type = SYS_SCL_KEY_HASH_TUNNEL_IPV4_RPF;
        }
        else if (!CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA))
        {
            if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY))
            {
                *p_key_type = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_IPV4: SYS_SCL_KEY_HASH_TUNNEL_IPV4_GRE;
                *is_hash_key = 0;
                *resolve_conflict = DRV_IS_DUET2(lchip) ? 0:1;
            }
            else
            {
                *p_key_type                        = SYS_SCL_KEY_HASH_TUNNEL_IPV4_DA;
            }
        }
        else if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY))
        {
            *p_key_type                        = SYS_SCL_KEY_HASH_TUNNEL_IPV4_GRE;
        }
        else if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA))
        {
            *p_key_type              = SYS_SCL_KEY_HASH_TUNNEL_IPV4;
        }

        if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_USE_FLEX))
        {
            *p_key_type = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_IPV4:(*p_key_type);
            *is_hash_key = 0;
            *resolve_conflict = DRV_IS_DUET2(lchip) ? 0:1;
        }
    }
    else
    {
        if (!CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA))
        {
            if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY))
            {
                *p_key_type = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_IPV6: SYS_SCL_KEY_HASH_TUNNEL_IPV6_GREKEY;
                *is_hash_key = 0;
                *resolve_conflict = DRV_IS_DUET2(lchip) ? 0:1;
            }
            else
            {
                *p_key_type                        = SYS_SCL_KEY_HASH_TUNNEL_IPV6_DA;
            }
        }
        else if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY))
        {
            *p_key_type                        = SYS_SCL_KEY_HASH_TUNNEL_IPV6_GREKEY;
        }
        else if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA))
        {
            *p_key_type              = SYS_SCL_KEY_HASH_TUNNEL_IPV6;
        }

        if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_USE_FLEX) || (DRV_IS_DUET2(lchip)))
        {
            *p_key_type = DRV_IS_DUET2(lchip) ? SYS_SCL_KEY_TCAM_IPV6:(*p_key_type);
            *is_hash_key = 0;
            *resolve_conflict = DRV_IS_DUET2(lchip) ? 0:1;
        }
    }

    return CTC_E_NONE;
}
STATIC int32
_sys_usw_ip_tunnel_build_tunnel_key(uint8 lchip, ctc_ipuc_tunnel_param_t* p_ip_tunnel_param, ctc_scl_entry_t* sys_scl_entry,
                                                            sys_scl_lkup_key_t* lkup_key, uint8 is_add, uint8 is_hash_key)
{
    ctc_field_key_t field_key;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&field_key, 0, sizeof(ctc_field_key_t));

    if(!is_add)
    {
        field_key.type = SYS_SCL_FIELD_KEY_COMMON;
        CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
    }

    if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_V4_RPF_CHECK))
    {
        if (CTC_IP_VER_4 == p_ip_tunnel_param->ip_ver)
        {
            /*deal flag CTC_IPUC_TUNNEL_FLAG_USE_FLEX and CTC_IPUC_TUNNEL_FLAG_V4_RPF_CHECK*/
            if (!is_hash_key && DRV_IS_DUET2(lchip))
            {
                /* tcam entry */
                field_key.type = CTC_FIELD_KEY_L3_TYPE;
                field_key.data = CTC_PARSER_L3_TYPE_IPV4;
                field_key.mask = 0xF;
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
                }
            }
            field_key.type = CTC_FIELD_KEY_IP_SA;
            field_key.data = p_ip_tunnel_param->ip_sa.ipv4;
            field_key.mask = 0xFFFFFFFF;
            if(is_add)
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
            }
        }
        else
        {
            ipv6_addr_t temp_ipv6_mask;
            sal_memset(&temp_ipv6_mask, 0xFF, sizeof(ipv6_addr_t));

            field_key.type = CTC_FIELD_KEY_IPV6_SA;
            field_key.ext_data = &p_ip_tunnel_param->ip_sa.ipv6;
            field_key.ext_mask = &temp_ipv6_mask;
            if(is_add)
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
            }
        }
    }
    else
    {
        if (CTC_IP_VER_4 == p_ip_tunnel_param->ip_ver)
        {
            /*
            field_key.type = CTC_FIELD_KEY_L3_TYPE;
            field_key.data = CTC_PARSER_L3_TYPE_IPV4;
            field_key.mask = 0xFF;
            if(is_add)
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
            }
            */
            if (!is_hash_key && DRV_IS_DUET2(lchip))
            {
                /* tcam entry */
                field_key.type = CTC_FIELD_KEY_L3_TYPE;
                field_key.data = CTC_PARSER_L3_TYPE_IPV4;
                field_key.mask = 0xF;
                if (is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
                }
            }
            field_key.type = CTC_FIELD_KEY_IP_DA;
            field_key.data = p_ip_tunnel_param->ip_da.ipv4;
            field_key.mask = 0xFFFFFFFF;
            if(is_add)
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
            }

            if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA))
            {
                field_key.type = CTC_FIELD_KEY_IP_SA;
                field_key.data = p_ip_tunnel_param->ip_sa.ipv4;
                field_key.mask = 0xFFFFFFFF;
                if(is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
                }
            }

            if (p_ip_tunnel_param->payload_type == CTC_IPUC_TUNNEL_PAYLOAD_TYPE_GRE)
            {
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = CTC_PARSER_L4_TYPE_GRE;
                field_key.mask = 0xFFFFFFFF;
                if(is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
                }
            }
            else if (p_ip_tunnel_param->payload_type == CTC_IPUC_TUNNEL_PAYLOAD_TYPE_V4)
            {
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = CTC_PARSER_L4_TYPE_IPINIP;
                field_key.mask = 0xFFFFFFFF;
                if(is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
                }
            }
            else if (p_ip_tunnel_param->payload_type == CTC_IPUC_TUNNEL_PAYLOAD_TYPE_V6)
            {
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = CTC_PARSER_L4_TYPE_V6INIP;
                field_key.mask = 0xFFFFFFFF;
                if(is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
                }
            }

            if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY))
            {
                field_key.type = CTC_FIELD_KEY_GRE_KEY;
                field_key.data = p_ip_tunnel_param->gre_key;
                field_key.mask = 0xFFFFFFFF;
                if(is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
                }
            }

        }
        else
        {
            ipv6_addr_t temp_ipv6_mask;
            sal_memset(&temp_ipv6_mask, 0xFF, sizeof(ipv6_addr_t));

            /*
            field_key.type = CTC_FIELD_KEY_L3_TYPE;
            field_key.data = CTC_PARSER_L3_TYPE_IPV6;
            field_key.mask = 0xFF;
            if(is_add)
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
            }
            */

            field_key.type = CTC_FIELD_KEY_IPV6_DA;
            field_key.ext_data = &p_ip_tunnel_param->ip_da.ipv6;
            field_key.ext_mask = &temp_ipv6_mask;
            if(is_add)
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
            }

            if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_LKUP_WITH_IPSA))
            {
                field_key.type = CTC_FIELD_KEY_IPV6_SA;
                field_key.ext_data = &p_ip_tunnel_param->ip_sa.ipv6;
                field_key.ext_mask = &temp_ipv6_mask;
                if(is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
                }
            }

            if (p_ip_tunnel_param->payload_type == CTC_IPUC_TUNNEL_PAYLOAD_TYPE_GRE)
            {
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = CTC_PARSER_L4_TYPE_GRE;
                field_key.mask = 0xFFFFFFFF;
                if(is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
                }

            }
            else if (p_ip_tunnel_param->payload_type == CTC_IPUC_TUNNEL_PAYLOAD_TYPE_V4)
            {
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = CTC_PARSER_L4_TYPE_IPINV6;
                field_key.mask = 0xFFFFFFFF;
                if(is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
                }

            }
            else if (p_ip_tunnel_param->payload_type == CTC_IPUC_TUNNEL_PAYLOAD_TYPE_V6)
            {
                field_key.type = CTC_FIELD_KEY_L4_TYPE;
                field_key.data = CTC_PARSER_L4_TYPE_V6INV6;
                field_key.mask = 0xFFFFFFFF;
                if(is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
                }

            }

            if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY))
            {
                field_key.type = CTC_FIELD_KEY_GRE_KEY;
                field_key.data = p_ip_tunnel_param->gre_key;
                field_key.mask = 0xFFFFFFFF;
                if(is_add)
                {
                    CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
                }
                else
                {
                    CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
                }

            }
        }
    }

    if(!is_hash_key)
    {
        if (!CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_V4_RPF_CHECK) && DRV_IS_DUET2(lchip))
        {
            /* won't decap fragment packet */
            field_key.type = CTC_FIELD_KEY_IP_FRAG;
            field_key.data = CTC_IP_FRAG_NON;
            field_key.mask = 0xFF;
            if(is_add)
            {
                CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
            }
        }
    }
    else
    {
        field_key.type = CTC_FIELD_KEY_HASH_VALID;
        if(is_add)
        {
            CTC_ERROR_RETURN(sys_usw_scl_add_key_field(lchip, sys_scl_entry->entry_id, &field_key));
        }
        else
        {
            CTC_ERROR_RETURN(sys_usw_scl_construct_lkup_key(lchip, &field_key, lkup_key));
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ip_tunnel_build_tunnel_action_filed(uint8 lchip, uint32 entry_id, ctc_ipuc_tunnel_param_t* p_ip_tunnel_param)
{
    uint8 loop = 0;
    ctc_scl_field_action_t scl_action_field = {0};
    sys_scl_iptunnel_t  tunnel_action = {0};
    uint8 gre_options_num = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    if(CTC_FLAG_ISSET(p_ip_tunnel_param->flag,CTC_IPUC_TUNNEL_FLAG_ACL_LKUP_BY_OUTER_HEAD) && CTC_FLAG_ISSET(p_ip_tunnel_param->flag,CTC_IPUC_TUNNEL_FLAG_ACL_LKUP_FOLLOW_PORT))
    {
        return CTC_E_NOT_SUPPORT;
    }
    if(CTC_FLAG_ISSET(p_ip_tunnel_param->flag,CTC_IPUC_TUNNEL_FLAG_QOS_USE_OUTER_INFO) && CTC_FLAG_ISSET(p_ip_tunnel_param->flag,CTC_IPUC_TUNNEL_FLAG_QOS_FOLLOW_PORT))
    {
        return CTC_E_NOT_SUPPORT;
    }

    if(CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_V4_RPF_CHECK))
    {
        scl_action_field.type = SYS_SCL_FIELD_ACTION_TYPE_IP_TUNNEL_DECAP;
        for (loop = 0; loop < CTC_IPUC_IP_TUNNEL_MAX_IF_NUM; loop++)
        {
            CTC_MAX_VALUE_CHECK(p_ip_tunnel_param->l3_inf[loop], (MCHIP_CAP(SYS_CAP_SPEC_L3IF_NUM)-1));
        }
        tunnel_action.rpf_check_en = 1;
        sal_memcpy(&tunnel_action.l3_if, &p_ip_tunnel_param->l3_inf, sizeof(p_ip_tunnel_param->l3_inf));
        tunnel_action.rpf_check_fail_tocpu = CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_V4_MORE_RPF);

        scl_action_field.ext_data = &tunnel_action;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &scl_action_field));
    }
    else
    {
        if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_STATS_EN))
        {
            scl_action_field.type = CTC_SCL_FIELD_ACTION_TYPE_STATS;
            scl_action_field.data0 = p_ip_tunnel_param->stats_id;
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &scl_action_field));
        }

        if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_DISCARD))
        {
            scl_action_field.type = CTC_SCL_FIELD_ACTION_TYPE_DISCARD;
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &scl_action_field));
        }

        if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_SERVICE_ACL_EN))
        {
            scl_action_field.type = CTC_SCL_FIELD_ACTION_TYPE_SERVICE_ACL_EN;
            scl_action_field.data0 = 1;
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &scl_action_field));
        }

        if (p_ip_tunnel_param->logic_port)
        {
            ctc_scl_logic_port_t logic_port;
            logic_port.logic_port =  p_ip_tunnel_param->logic_port;
            logic_port.logic_port_type = 0;
            scl_action_field.type = CTC_SCL_FIELD_ACTION_TYPE_LOGIC_PORT;
            scl_action_field.ext_data = &logic_port;
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &scl_action_field));
        }

        if (p_ip_tunnel_param->policer_id)
        {
            scl_action_field.type = CTC_SCL_FIELD_ACTION_TYPE_POLICER_ID;
            scl_action_field.data0 = p_ip_tunnel_param->policer_id;
            if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_SERVICE_POLICER_EN))
            {
                scl_action_field.data1 = 1;
            }
            else
            {
                scl_action_field.data1 = 0;
            }
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &scl_action_field));
        }

        if (p_ip_tunnel_param->cid)
        {
            scl_action_field.type = CTC_SCL_FIELD_ACTION_TYPE_SRC_CID;
            scl_action_field.data0 = p_ip_tunnel_param->cid;
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &scl_action_field));
        }

        if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_INNER_VRF_EN))
        {
            SYS_IP_VRFID_CHECK(p_ip_tunnel_param->vrf_id, p_ip_tunnel_param->ip_ver);
            if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_ACL_EN))
            {
                scl_action_field.type = CTC_SCL_FIELD_ACTION_TYPE_FID;
                scl_action_field.data0 = (1 << 14 | p_ip_tunnel_param->vrf_id);
            }
            else
            {
                scl_action_field.type = CTC_SCL_FIELD_ACTION_TYPE_VRFID;
                scl_action_field.data0 = p_ip_tunnel_param->vrf_id;
            }
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &scl_action_field));
        }

        if(CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_QOS_MAP))
        {
            scl_action_field.type = CTC_SCL_FIELD_ACTION_TYPE_QOS_MAP;
            scl_action_field.ext_data= &p_ip_tunnel_param->qos_map;
            CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &scl_action_field));
        }

        scl_action_field.type = SYS_SCL_FIELD_ACTION_TYPE_IP_TUNNEL_DECAP;

        tunnel_action.inner_lkup_en = CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_LKUP_BY_OUTER_HEAD) ? 0 : 1;
        tunnel_action.payload_pktType =
            (p_ip_tunnel_param->payload_type == CTC_IPUC_TUNNEL_PAYLOAD_TYPE_GRE) ? 0 :
            ((p_ip_tunnel_param->payload_type == CTC_IPUC_TUNNEL_PAYLOAD_TYPE_V4) ? PKT_TYPE_IPV4 : PKT_TYPE_IPV6);

        if (p_ip_tunnel_param->payload_type == CTC_IPUC_TUNNEL_PAYLOAD_TYPE_GRE)/*IpeTunnelDecapCtl.offsetByteShift = 1*/
        {
            gre_options_num = CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY)
                              + CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_CHECKSUM)
                              + CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_SEQ_NUM);
            tunnel_action.payload_offset = (DRV_IS_DUET2(lchip)) ? (1 + gre_options_num)*2 : 4;
            tunnel_action.tunnel_type = gre_options_num? SYS_SCL_TUNNEL_ACTION_TUNNEL_TYPE_GRE : 0;
            if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_KEY))
            {
                CTC_BIT_SET(tunnel_action.gre_options, 1);
            }
            if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_CHECKSUM))
            {
                CTC_BIT_SET(tunnel_action.gre_options, 2);
            }
            if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_GRE_WITH_SEQ_NUM))
            {
                CTC_BIT_SET(tunnel_action.gre_options, 0);
            }
        }


        if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_LKUP_BY_OUTER_HEAD))
        {
            tunnel_action.nexthop_id = p_ip_tunnel_param->nh_id;
            tunnel_action.nexthop_id_valid = 1;
        }

        if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_ACL_EN))
        {
            for (loop = 0; loop < p_ip_tunnel_param->acl_lkup_num; loop++)
            {
                SYS_ACL_PROPERTY_CHECK((&p_ip_tunnel_param->acl_prop[loop]));
            }
            sal_memcpy(tunnel_action.acl_prop, p_ip_tunnel_param->acl_prop, sizeof(ctc_acl_property_t) * p_ip_tunnel_param->acl_lkup_num);
            tunnel_action.acl_lkup_num = p_ip_tunnel_param->acl_lkup_num;
        }

        if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_ISATAP_CHECK_EN))
        {
            if ((p_ip_tunnel_param->payload_type == CTC_IPUC_TUNNEL_PAYLOAD_TYPE_V6) &&
                    (p_ip_tunnel_param->ip_ver == CTC_IP_VER_4))
            {
                tunnel_action.tunnel_type = SYS_SCL_TUNNEL_ACTION_TUNNEL_TYPE_IPV6_IN_IP;
                tunnel_action.isatap_check_en = 1;
            }
        }

        if(CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_METADATA_EN))
        {
            CTC_MAX_VALUE_CHECK(p_ip_tunnel_param->metadata, 0x3FFF);
            tunnel_action.metadata = p_ip_tunnel_param->metadata;
            tunnel_action.meta_data_valid = 1;
        }

        tunnel_action.rpf_check_request = !CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_RPF_CHECK_DISABLE);
        tunnel_action.ttl_check_en = CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_TTL_CHECK);
        tunnel_action.use_outer_ttl = CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_USE_OUTER_TTL);

        if(CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_ACL_LKUP_BY_OUTER_HEAD))
        {
            tunnel_action.acl_use_outer_info = 1;
            tunnel_action.acl_use_outer_info_valid = 1;
        }
        else if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_ACL_LKUP_FOLLOW_PORT))
        {
            tunnel_action.acl_use_outer_info = 0;
            tunnel_action.acl_use_outer_info_valid = 0;
        }
        else
        {
            /* keep the default action the same as GG */
            tunnel_action.acl_use_outer_info = 0;
            tunnel_action.acl_use_outer_info_valid = 1;
        }

        if(CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_QOS_USE_OUTER_INFO))
        {
            tunnel_action.classify_use_outer_info = 1;
            tunnel_action.classify_use_outer_info_valid = 1;
        }
        else if (CTC_FLAG_ISSET(p_ip_tunnel_param->flag, CTC_IPUC_TUNNEL_FLAG_QOS_FOLLOW_PORT))
        {
            tunnel_action.classify_use_outer_info = 0;
            tunnel_action.classify_use_outer_info_valid = 0;
        }
        else
        {
            /* keep the default action the same as GG */
            tunnel_action.classify_use_outer_info = 0;
            tunnel_action.classify_use_outer_info_valid = 1;
        }
        scl_action_field.ext_data = &tunnel_action;
        CTC_ERROR_RETURN(sys_usw_scl_add_action_field(lchip, entry_id, &scl_action_field));
    }

    return CTC_E_NONE;

}
int32
sys_usw_ip_tunnel_add_tunnel(uint8 lchip, ctc_ipuc_tunnel_param_t* p_ip_tunnel_param)
{
    uint8  is_add = 0;
    uint8  key_type = 0;
    uint8  is_hash_key = 0;
    uint8  resolve_conflict = 0;
    uint32 group_id = 0;
    int32  ret = 0;
    ctc_scl_entry_t*  p_scl_entry = NULL;
    sys_scl_lkup_key_t lkup_key;
    ctc_scl_group_info_t group_info = {0};
    ctc_scl_field_action_t scl_action_field = {0};

    SYS_IPUC_TUNNEL_INIT_CHECK;

    CTC_ERROR_RETURN(_sys_usw_ip_tunnel_check(lchip, p_ip_tunnel_param));
    SYS_IPUC_TUNNEL_FUNC_DBG_DUMP(p_ip_tunnel_param);

    CTC_ERROR_RETURN(_sys_usw_ip_tunnel_flag_check(lchip, p_ip_tunnel_param));

    sal_memset(&lkup_key, 0, sizeof(lkup_key));

    _sys_usw_ip_tunnel_get_tunnel_key_type(lchip, p_ip_tunnel_param, &key_type, &is_hash_key, &resolve_conflict);
    if(is_hash_key)
    {
        p_ip_tunnel_param->scl_id = DRV_IS_DUET2(lchip) ? 0 : p_ip_tunnel_param->scl_id;
        group_info.type = CTC_SCL_GROUP_TYPE_HASH;
        group_info.priority = p_ip_tunnel_param->scl_id;
        group_id = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_IP_TUNNEL, (SYS_IP_TUNNEL_GROUP_SUB_TYPE_HASH+p_ip_tunnel_param->scl_id), 0, 0, 0);
        ret = sys_usw_scl_create_group(lchip, group_id, &group_info, 1);
        if ((ret < 0 ) && (ret != CTC_E_EXIST))
        {
            return ret;
        }
    }
    else
    {
        group_id = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_IP_TUNNEL, (SYS_IP_TUNNEL_GROUP_SUB_TYPE_TCAM0+p_ip_tunnel_param->scl_id), 0, 0, 0);
        group_info.type = CTC_SCL_GROUP_TYPE_NONE;
        group_info.priority = p_ip_tunnel_param->scl_id;
        ret = sys_usw_scl_create_group(lchip, group_id, &group_info, 1);
        if ((ret < 0 ) && (ret != CTC_E_EXIST))
        {
            return ret;
        }
    }
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "add ip tunnel to group id: 0x%x\n", group_id);

    lkup_key.key_type = key_type;
    lkup_key.action_type = SYS_SCL_ACTION_TUNNEL;
    lkup_key.group_id = group_id;
    lkup_key.group_priority = p_ip_tunnel_param->scl_id;
    lkup_key.resolve_conflict = resolve_conflict;
    CTC_ERROR_RETURN(_sys_usw_ip_tunnel_build_tunnel_key(lchip, p_ip_tunnel_param, NULL, &lkup_key, 0, is_hash_key));

    p_scl_entry = (ctc_scl_entry_t*)mem_malloc(MEM_IPUC_MODULE, sizeof(ctc_scl_entry_t));
    if (NULL == p_scl_entry)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_scl_entry, 0, sizeof(ctc_scl_entry_t));

    ret = sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key);
    if((ret != CTC_E_NONE) && (ret != CTC_E_NOT_EXIST))
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "get entry id error\n");
        goto out;
    }
    else if(ret == CTC_E_NOT_EXIST)
    {
         /*add entry*/
        is_add = 1;
        p_scl_entry->key_type = key_type;
        p_scl_entry->action_type = SYS_SCL_ACTION_TUNNEL;
        p_scl_entry->resolve_conflict = resolve_conflict;
        CTC_ERROR_GOTO(sys_usw_scl_add_entry(lchip, group_id, p_scl_entry, 1), ret, out);

        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "create new scl entry, entry_id: 0x%x\n", p_scl_entry->entry_id);

        /* build scl key */
        CTC_ERROR_GOTO(_sys_usw_ip_tunnel_build_tunnel_key(lchip, p_ip_tunnel_param, p_scl_entry, NULL, 1, is_hash_key), ret, error0);
    }
    else
    {
        p_scl_entry->entry_id = lkup_key.entry_id;
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "scl entry is exist,  entry_id: 0x%x\n", p_scl_entry->entry_id);

        scl_action_field.type = CTC_SCL_FIELD_ACTION_TYPE_PENDING;
        CTC_ERROR_GOTO(sys_usw_scl_add_action_field(lchip, lkup_key.entry_id, &scl_action_field), ret, out);
    }

    CTC_ERROR_GOTO(_sys_usw_ip_tunnel_build_tunnel_action_filed(lchip, p_scl_entry->entry_id, p_ip_tunnel_param), ret, error0);
    CTC_ERROR_GOTO(sys_usw_scl_install_entry(lchip, p_scl_entry->entry_id, 1), ret, error0);

    ret = CTC_E_NONE;
    goto out;

error0:
    if (is_add)
    {
        sys_usw_scl_remove_entry(lchip, p_scl_entry->entry_id, 1);
    }
    else
    {
        sys_usw_scl_remove_action_field(lchip, lkup_key.entry_id, &scl_action_field);
    }

out:
    if (p_scl_entry)
    {
        mem_free(p_scl_entry);
    }

    return ret;
}

int32
sys_usw_ip_tunnel_remove_tunnel(uint8 lchip, ctc_ipuc_tunnel_param_t* p_ip_tunnel_param)
{
    sys_scl_lkup_key_t lkup_key;
    uint8              is_hash_key;
    uint8              resolve_conflict = 0;

    CTC_ERROR_RETURN(_sys_usw_ip_tunnel_check(lchip, p_ip_tunnel_param));
    SYS_IPUC_TUNNEL_FUNC_DBG_DUMP(p_ip_tunnel_param);

    CTC_ERROR_RETURN(_sys_usw_ip_tunnel_flag_check(lchip, p_ip_tunnel_param));

    sal_memset(&lkup_key, 0, sizeof(lkup_key));

    _sys_usw_ip_tunnel_get_tunnel_key_type(lchip, p_ip_tunnel_param, &lkup_key.key_type, &is_hash_key, &resolve_conflict);
    lkup_key.action_type = SYS_SCL_ACTION_TUNNEL;
    lkup_key.group_priority = p_ip_tunnel_param->scl_id;
    lkup_key.resolve_conflict = resolve_conflict;
    if(is_hash_key)
    {
        p_ip_tunnel_param->scl_id = DRV_IS_DUET2(lchip) ? 0 : p_ip_tunnel_param->scl_id;
        lkup_key.group_id = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_IP_TUNNEL, (SYS_IP_TUNNEL_GROUP_SUB_TYPE_HASH+p_ip_tunnel_param->scl_id), 0, 0, 0);
    }
    else
    {
        lkup_key.group_id = SYS_SCL_ENCODE_INNER_GRP_ID(CTC_FEATURE_IP_TUNNEL, (SYS_IP_TUNNEL_GROUP_SUB_TYPE_TCAM0+p_ip_tunnel_param->scl_id), 0, 0, 0);
    }

    CTC_ERROR_RETURN(_sys_usw_ip_tunnel_build_tunnel_key(lchip, p_ip_tunnel_param, NULL, &lkup_key, 0, is_hash_key));

    CTC_ERROR_RETURN(sys_usw_scl_get_entry_id_by_lkup_key(lchip, &lkup_key));
    sys_usw_scl_uninstall_entry(lchip, lkup_key.entry_id, 1);
    sys_usw_scl_remove_entry(lchip, lkup_key.entry_id, 1);

    return CTC_E_NONE;
}

int32
_sys_usw_ip_tunnel_wb_mapping_natsa(uint8 lchip, sys_wb_ip_tunnel_natsa_info_t *p_wb_natsa_info, sys_ip_tunnel_natsa_info_t *p_natsa_info, uint8 sync)
{
    sys_usw_opf_t opf = {0};

    if (sync)
    {
        p_wb_natsa_info->ipv4 = p_natsa_info->ipv4;
        p_wb_natsa_info->new_ipv4 = p_natsa_info->new_ipv4;
        p_wb_natsa_info->l4_src_port = p_natsa_info->l4_src_port;
        p_wb_natsa_info->vrf_id = p_natsa_info->vrf_id;
        p_wb_natsa_info->is_tcp_port = p_natsa_info->is_tcp_port;
        p_wb_natsa_info->ad_offset = p_natsa_info->ad_offset;
        p_wb_natsa_info->key_offset = p_natsa_info->key_offset;
        p_wb_natsa_info->new_l4_src_port = p_natsa_info->new_l4_src_port;
        p_wb_natsa_info->in_tcam = p_natsa_info->in_tcam;
    }
    else
    {
        p_natsa_info->ipv4 = p_wb_natsa_info->ipv4;
        p_natsa_info->new_ipv4 = p_wb_natsa_info->new_ipv4;
        p_natsa_info->l4_src_port = p_wb_natsa_info->l4_src_port;
        p_natsa_info->vrf_id = p_wb_natsa_info->vrf_id;
        p_natsa_info->is_tcp_port = p_wb_natsa_info->is_tcp_port;
        p_natsa_info->ad_offset = p_wb_natsa_info->ad_offset;
        p_natsa_info->key_offset = p_wb_natsa_info->key_offset;
        p_natsa_info->new_l4_src_port = p_wb_natsa_info->new_l4_src_port;
        p_natsa_info->in_tcam = p_wb_natsa_info->in_tcam;

        if (p_natsa_info->in_tcam)
        {
            opf.pool_index = 0;
            opf.pool_type = p_usw_ip_tunnel_master[lchip]->opf_type_ipv4_nat;

            CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_natsa_info->key_offset));
        }

    }

    return CTC_E_NONE;
}

int32
_sys_usw_ip_tunnel_wb_sync_natsa_func(sys_ip_tunnel_natsa_info_t *p_natsa_info, void *user_data)
{
    uint16 max_entry_cnt = 0;
    sys_wb_ip_tunnel_natsa_info_t  *p_wb_natsa_info;
    sys_traverse_t *data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)(data->data);
    uint8 lchip = (uint8)(data->value1);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_natsa_info = (sys_wb_ip_tunnel_natsa_info_t *)wb_data->buffer + wb_data->valid_cnt;

    CTC_ERROR_RETURN(_sys_usw_ip_tunnel_wb_mapping_natsa(lchip, p_wb_natsa_info, p_natsa_info, 1));
    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32
sys_usw_ip_tunnel_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_traverse_t ip_tunnel_hash_data;
    sys_wb_ip_tunnel_master_t  *p_wb_ip_tunnel_master;

    /*syncup ipuc matser*/
    wb_data.buffer = mem_malloc(MEM_IPUC_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_ip_tunnel_master_t, CTC_FEATURE_IP_TUNNEL, SYS_WB_APPID_IP_TUNNEL_SUBID_MASTER);

    p_wb_ip_tunnel_master = (sys_wb_ip_tunnel_master_t *)wb_data.buffer;

    p_wb_ip_tunnel_master->lchip = lchip;
    p_wb_ip_tunnel_master->version = SYS_WB_VERSION_IP_TUNNEL;
    p_wb_ip_tunnel_master->snat_hash_count = p_usw_ip_tunnel_master[lchip]->snat_hash_count ;
    p_wb_ip_tunnel_master->snat_tcam_count = p_usw_ip_tunnel_master[lchip]->snat_tcam_count ;
    p_wb_ip_tunnel_master->napt_hash_count = p_usw_ip_tunnel_master[lchip]->napt_hash_count ;

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    /*syncup ip_tunnel_info*/
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_ip_tunnel_natsa_info_t, CTC_FEATURE_IP_TUNNEL, SYS_WB_APPID_IP_TUNNEL_SUBID_NATSA_INFO);
    ip_tunnel_hash_data.data = &wb_data;
    ip_tunnel_hash_data.value1 = lchip;

    CTC_ERROR_GOTO(ctc_hash_traverse(p_usw_ip_tunnel_master[lchip]->nat_hash, (hash_traversal_fn) _sys_usw_ip_tunnel_wb_sync_natsa_func, (void *)&ip_tunnel_hash_data), ret, done);
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

    return ret;
}

int32
sys_usw_ip_tunnel_wb_restore(uint8 lchip)
{
    uint16 entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    sys_usw_opf_t opf;
    ctc_wb_query_t wb_query;
    sys_wb_ip_tunnel_master_t  wb_ip_tunnel_master = {0};
    sys_ip_tunnel_natsa_info_t* p_natsa_info = NULL;
    sys_wb_ip_tunnel_natsa_info_t wb_natsa_info = {0};

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));

    wb_query.buffer = mem_malloc(MEM_IPUC_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_ip_tunnel_master_t, CTC_FEATURE_IP_TUNNEL, SYS_WB_APPID_IP_TUNNEL_SUBID_MASTER);

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);

    /*restore  ip_tunnel_master*/
    if (wb_query.valid_cnt != 1 || wb_query.is_end != 1)
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "query ipuc master error! valid_cnt: %d, is_end: %d.\n", wb_query.valid_cnt, wb_query.is_end);
        ret = CTC_E_INVALID_CONFIG;
        goto done;
    }

    sal_memcpy((uint8*)&wb_ip_tunnel_master, (uint8*)wb_query.buffer, wb_query.key_len + wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_IP_TUNNEL, wb_ip_tunnel_master.version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }

    p_usw_ip_tunnel_master[lchip]->snat_hash_count = wb_ip_tunnel_master.snat_hash_count ;
    p_usw_ip_tunnel_master[lchip]->snat_tcam_count = wb_ip_tunnel_master.snat_tcam_count ;
    p_usw_ip_tunnel_master[lchip]->napt_hash_count = wb_ip_tunnel_master.napt_hash_count ;

    /*restore  natsa info*/
    CTC_WB_INIT_QUERY_T((&wb_query),sys_wb_ip_tunnel_natsa_info_t, CTC_FEATURE_IP_TUNNEL, SYS_WB_APPID_IP_TUNNEL_SUBID_NATSA_INFO);

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)&wb_natsa_info, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;

        p_natsa_info = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_ip_tunnel_natsa_info_t));
        if (NULL == p_natsa_info)
        {
            ret = CTC_E_NO_MEMORY;

            goto done;
        }
        sal_memset(p_natsa_info, 0, sizeof(sys_ip_tunnel_natsa_info_t));

        ret = _sys_usw_ip_tunnel_wb_mapping_natsa(lchip, &wb_natsa_info, p_natsa_info, 0);
        if (ret)
        {
            mem_free(p_natsa_info);
            continue;
        }

        SYS_IPUC_TUNNEL_LOCK;
        ctc_hash_insert(p_usw_ip_tunnel_master[lchip]->nat_hash, p_natsa_info);
        SYS_IPUC_TUNNEL_UNLOCK;
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
sys_usw_ip_tunnel_dump_db(uint8 lchip, sal_file_t p_f, ctc_global_dump_db_t* p_dump_param)
{
    SYS_IPUC_TUNNEL_INIT_CHECK;
    SYS_IPUC_TUNNEL_LOCK;

    SYS_DUMP_DB_LOG(p_f, "%s\n", "# Ip Tunnel");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Max snat tcam number", p_usw_ip_tunnel_master[lchip]->max_snat_tcam_num);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Napt hash count", p_usw_ip_tunnel_master[lchip]->napt_hash_count);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Snat hash count", p_usw_ip_tunnel_master[lchip]->snat_hash_count);
    SYS_DUMP_DB_LOG(p_f, "%-36s: %u\n","Snat tcam count", p_usw_ip_tunnel_master[lchip]->snat_tcam_count);
    SYS_DUMP_DB_LOG(p_f, "%-36s: [0x%x] [0x%x]\n","Lookup mode", \
        p_usw_ip_tunnel_master[lchip]->lookup_mode[CTC_IP_VER_4], p_usw_ip_tunnel_master[lchip]->lookup_mode[CTC_IP_VER_6]);

    SYS_DUMP_DB_LOG(p_f, "\n");
    sys_usw_opf_fprint_alloc_used_info(lchip, p_usw_ip_tunnel_master[lchip]->opf_type_ipv4_nat, p_f);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");

    SYS_IPUC_TUNNEL_UNLOCK;

    return CTC_E_NONE;
}

int32
sys_usw_ip_tunnel_init(uint8 lchip)
{
    uint32 entry_num = 0;
    sys_usw_opf_t opf;
    uint32 cmd;
    IpeTunnelDecapCtl_m p_IpeTunnelDecapCtl;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    sal_memset(&p_IpeTunnelDecapCtl, 0, sizeof(p_IpeTunnelDecapCtl));

    p_usw_ip_tunnel_master[lchip] = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_ip_tunnel_master_t));
    if (NULL == p_usw_ip_tunnel_master[lchip])
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_usw_ip_tunnel_master[lchip], 0, sizeof(sys_ip_tunnel_master_t));

    sal_mutex_create(&p_usw_ip_tunnel_master[lchip]->mutex);
    if (NULL == p_usw_ip_tunnel_master[lchip]->mutex)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        CTC_ERROR_RETURN(CTC_E_NO_MEMORY);
    }

    /* ipv4/ipv6 NAT host hash resource */
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsFibHost1Ipv4NatSaPortHashKey_t, &entry_num));
    if (entry_num > 0)
    {
        p_usw_ip_tunnel_master[lchip]->lookup_mode[CTC_IP_VER_4] += SYS_NAT_HASH_LOOKUP;
        p_usw_ip_tunnel_master[lchip]->lookup_mode[CTC_IP_VER_6] += SYS_NAT_HASH_LOOKUP;
    }

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsLpmTcamIpv4NatDoubleKey_t, &entry_num));
    p_usw_ip_tunnel_master[lchip]->max_snat_tcam_num = entry_num;
    if(entry_num)
    {
         p_usw_ip_tunnel_master[lchip]->lookup_mode[CTC_IP_VER_4] += SYS_NAT_TCAM_LOOKUP;
        p_usw_ip_tunnel_master[lchip]->lookup_mode[CTC_IP_VER_6] += SYS_NAT_TCAM_LOOKUP;

        CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &p_usw_ip_tunnel_master[lchip]->opf_type_ipv4_nat, 1, "opf-ip-tunnel-ipv4-nat"));
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_index = 0;
        opf.pool_type = p_usw_ip_tunnel_master[lchip]->opf_type_ipv4_nat;
        CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 0, entry_num));
    }

    /* add nat sa default entry, use tcam default entry */
    CTC_ERROR_RETURN(sys_usw_ip_tunnel_add_natsa_default_entry(lchip));

     /*init for IPv4*/
    if (!p_usw_ip_tunnel_master[lchip]->nat_hash)
    {
        p_usw_ip_tunnel_master[lchip]->nat_hash = ctc_hash_create(1,
                                                        (IPUC_IPV4_HASH_MASK + 1),
                                                        (hash_key_fn)_sys_usw_ip_tunnel_natsa_hash_make,
                                                        (hash_cmp_fn)_sys_usw_ip_tunnel_natsa_hash_cmp);
        if (!p_usw_ip_tunnel_master[lchip]->nat_hash)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
            return CTC_E_NO_MEMORY;
        }
    }

    cmd = DRV_IOR(IpeTunnelDecapCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &p_IpeTunnelDecapCtl));

    SetIpeTunnelDecapCtl(V, greOption1CheckEn_f, &p_IpeTunnelDecapCtl, 1);
    SetIpeTunnelDecapCtl(V, greOption3CheckEn_f, &p_IpeTunnelDecapCtl, 1);
    SetIpeTunnelDecapCtl(V, greProtocolCheckEn_f, &p_IpeTunnelDecapCtl, 1);

    cmd = DRV_IOW(IpeTunnelDecapCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &p_IpeTunnelDecapCtl));

    sys_usw_ftm_register_callback(lchip, CTC_FTM_SPEC_NAPT, sys_usw_ip_tunnel_ftm_cb);

    CTC_ERROR_RETURN(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_IP_TUNNEL, sys_usw_ip_tunnel_wb_sync));

    if (CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING))
    {
	  CTC_ERROR_RETURN(sys_usw_ip_tunnel_wb_restore(lchip));
    }

    CTC_ERROR_RETURN(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_IP_TUNNEL, sys_usw_ip_tunnel_dump_db));
    return CTC_E_NONE;
}

int32
sys_usw_ip_tunnel_deinit(uint8 lchip)
{

    LCHIP_CHECK(lchip);
    if (NULL == p_usw_ip_tunnel_master[lchip])
    {
        return CTC_E_NONE;
    }

    ctc_hash_traverse(p_usw_ip_tunnel_master[lchip]->nat_hash, (hash_traversal_fn)_sys_usw_ip_tunnel_hash_free_node_data, NULL);
    ctc_hash_free(p_usw_ip_tunnel_master[lchip]->nat_hash);

    sys_usw_opf_deinit(lchip, p_usw_ip_tunnel_master[lchip]->opf_type_ipv4_nat);

    if (p_usw_ip_tunnel_master[lchip]->mutex)
    {
        sal_mutex_destroy(p_usw_ip_tunnel_master[lchip]->mutex);
    }
    mem_free(p_usw_ip_tunnel_master[lchip]);

    return CTC_E_NONE;
}

#endif
