#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))
/**
 @file sys_duet2_ipuc.c

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
#include "ctc_linklist.h"

#include "sys_usw_chip.h"
#include "sys_usw_ofb.h"
#include "sys_usw_ftm.h"
#include "sys_usw_ipuc.h"
#include "sys_duet2_calpm.h"
#include "sys_duet2_ipuc_tcam.h"

#include "usw/include/drv_common.h"

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
extern sys_ipuc_master_t* p_usw_ipuc_master[];

/****************************************************************************
 *
* Function
*
*****************************************************************************/

#define ____IPUC_TCAM_INNER_FUNCTION________________________
STATIC int32
  _sys_duet2_ipuc_tcam_write_ipv4_key(uint8 lchip, sys_ipuc_tcam_data_t *p_data)
{
    sys_ipuc_info_t  *p_ipuc_info    = NULL;
    sys_calpm_prefix_t  *p_calpm_prefix  = NULL;
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;
    sys_ipuc_tbl_type_t tbl_type = SYS_TBL_TCAM;
    sys_calpm_tbl_t* p_table = NULL;
    uint32 key_index = 0;
    uint8 masklen = 0;
    uint8 is_pointer = 0;
    ds_t tcam40key, tcam40mask;
    tbl_entry_t tbl_ipkey;
    uint32 key_id, sa_key_id;
    uint32 ip_mask = 0;
    uint32 cmd = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_PTR_VALID_CHECK(p_data->info);

    if (p_data->key_type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        p_ipuc_info = (sys_ipuc_info_t*)p_data->info;

        masklen = p_ipuc_info->masklen;
        key_index = p_ipuc_info->key_index;
        if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE))
        {
            route_mode = SYS_PUBLIC_MODE;
        }
    }
    else if (p_data->key_type == SYS_IPUC_TCAM_FLAG_ALPM)
    {
        p_calpm_prefix = (sys_calpm_prefix_t*)p_data->info;

        masklen = p_data->masklen;
        is_pointer = (masklen > p_calpm_prefix->prefix_len);
        key_index = p_calpm_prefix->key_index[is_pointer];
        p_table = p_calpm_prefix->p_ntable;
        if (is_pointer)
        {
            CTC_PTR_VALID_CHECK(p_table);
        }
    }

    tbl_type = (route_mode == SYS_PRIVATE_MODE) ? SYS_TBL_TCAM : SYS_TBL_TCAM_PUB;
    sys_usw_get_tbl_id(lchip, CTC_IP_VER_4, 1, tbl_type, &key_id, NULL);

    sal_memset(&tcam40key, 0, sizeof(tcam40key));
    sal_memset(&tcam40mask, 0, sizeof(tcam40mask));

    /* write key */
    tbl_ipkey.data_entry = (uint32*)&tcam40key;
    tbl_ipkey.mask_entry = (uint32*)&tcam40mask;

    if (p_data->key_type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        SetDsLpmTcamIpv4HalfKey(V, ipAddr_f, tcam40key, p_ipuc_info->ip.ipv4);
        SetDsLpmTcamIpv4HalfKey(V, vrfId_f, tcam40key, p_ipuc_info->vrf_id);
        IPV4_LEN_TO_MASK(ip_mask, masklen);
    }
    else if (p_data->key_type == SYS_IPUC_TCAM_FLAG_ALPM)
    {
        SetDsLpmTcamIpv4HalfKey(V, ipAddr_f, tcam40key, p_calpm_prefix->ip32[0]);
        SetDsLpmTcamIpv4HalfKey(V, vrfId_f, tcam40key, p_calpm_prefix->vrf_id);
        IPV4_LEN_TO_MASK(ip_mask, p_calpm_prefix->prefix_len);
    }

    SetDsLpmTcamIpv4HalfKey(V, ipAddr_f, tcam40mask, ip_mask);
    if (route_mode == SYS_PRIVATE_MODE)
    {
        SetDsLpmTcamIpv4HalfKey(V, vrfId_f, tcam40mask, 0x1FFF);
    }
    else
    {
        SetDsLpmTcamIpv4HalfKey(V, vrfId_f, tcam40mask, 0);
    }
    SetDsLpmTcamIpv4HalfKey(V, lpmTcamKeyType_f, tcam40key, 0);
    SetDsLpmTcamIpv4HalfKey(V, lpmTcamKeyType_f, tcam40mask, 0x1);

    cmd = DRV_IOW(key_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, key_index, cmd, &tbl_ipkey);

    if(CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_SA_LOOKUP))
    {
        sys_usw_get_tbl_id(lchip, CTC_IP_VER_4, 0, tbl_type, &sa_key_id, NULL);

        cmd = DRV_IOW(sa_key_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, key_index, cmd, &tbl_ipkey);
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: write %s[%d]\n", TABLE_NAME(lchip, key_id), key_index);

    return CTC_E_NONE;
}

STATIC int32
  _sys_duet2_ipuc_tcam_write_ipv4_ad(uint8 lchip, sys_ipuc_tcam_data_t *p_data)
{
    sys_ipuc_info_t  *p_ipuc_info    = NULL;
    sys_calpm_prefix_t  *p_calpm_prefix  = NULL;
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;
    sys_ipuc_tbl_type_t tbl_type = SYS_TBL_TCAM;
    sys_calpm_tbl_t* p_table = NULL;
    uint32 ad_index = 0;
    uint32 key_index = 0;
    uint8 masklen = 0;
    uint8 is_pointer = 0;
    ds_t tcam_ad;
    uint32 ad_id, sa_ad_id;
    uint32 cmd = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_PTR_VALID_CHECK(p_data->info);
    sal_memset(&tcam_ad, 0, sizeof(tcam_ad));

    if (p_data->key_type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        p_ipuc_info = (sys_ipuc_info_t*)p_data->info;

        masklen = p_ipuc_info->masklen;
        key_index = p_ipuc_info->key_index;
        ad_index = p_ipuc_info->ad_index;
        if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE))
        {
            route_mode = SYS_PUBLIC_MODE;
        }
    }
    else if (p_data->key_type == SYS_IPUC_TCAM_FLAG_ALPM)
    {
        p_calpm_prefix = (sys_calpm_prefix_t*)p_data->info;

        masklen = p_data->masklen;
        is_pointer = (masklen > p_calpm_prefix->prefix_len);
        key_index = p_calpm_prefix->key_index[is_pointer];
        ad_index = p_data->ad_index;
        p_table = p_calpm_prefix->p_ntable;
        if (is_pointer)
        {
            CTC_PTR_VALID_CHECK(p_table);
        }
    }

    if (p_data->opt_type == DO_UPDATE)
    {
        if (key_index == INVALID_NEXTHOP_OFFSET)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "key index is invalid %d, ad_index = %d\r\n", key_index, ad_index);
            return CTC_E_INVALID_PARAM;
        }

        sys_usw_get_tbl_id(lchip, CTC_IP_VER_4, 1, tbl_type, NULL, &ad_id);

        cmd = DRV_IOR(ad_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, key_index, cmd, &tcam_ad);

        SetDsLpmTcamIpv4HalfKeyAd(V, nexthop_f, &tcam_ad, ad_index);

        cmd = DRV_IOW(ad_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, key_index, cmd, &tcam_ad);

        if(CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_SA_LOOKUP))
        {
            sys_usw_get_tbl_id(lchip, CTC_IP_VER_4, 0, tbl_type, NULL, &sa_ad_id);

            cmd = DRV_IOW(sa_ad_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, key_index, cmd, &tcam_ad);
        }
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: update %s[%d] ad_index = %d\n", TABLE_NAME(lchip, ad_id), key_index, ad_index);

        return CTC_E_NONE;
    }

    tbl_type = (route_mode == SYS_PRIVATE_MODE) ? SYS_TBL_TCAM : SYS_TBL_TCAM_PUB;
    sys_usw_get_tbl_id(lchip, CTC_IP_VER_4, 1, tbl_type, NULL, &ad_id);

    if (p_data->key_type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        SetDsLpmTcamIpv4HalfKeyAd(V, nexthop_f, tcam_ad, ad_index);
    }
    else if (p_data->key_type == SYS_IPUC_TCAM_FLAG_ALPM)
    {
        if ((p_calpm_prefix->key_index[CALPM_TYPE_NEXTHOP] != INVALID_POINTER_OFFSET) ||
            (p_calpm_prefix->is_pushed && p_calpm_prefix->is_mask_valid))
        {
            if (!is_pointer)
            {
                ad_index = p_data->ad_index;
            }
            else
            {
                cmd = DRV_IOR(ad_id, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, key_index, cmd, &tcam_ad);
                ad_index = GetDsLpmTcamIpv4HalfKeyAd(V, nexthop_f, tcam_ad);
            }
            sal_memset(&tcam_ad, 0, sizeof(tcam_ad));
            SetDsLpmTcamIpv4HalfKeyAd(V, nexthop_f, tcam_ad, ad_index);
        }
        else
        {
            SetDsLpmTcamIpv4HalfKeyAd(V, nexthop_f, tcam_ad, INVALID_DRV_NEXTHOP_OFFSET);
        }

        if (p_calpm_prefix->key_index[CALPM_TYPE_POINTER] != INVALID_POINTER_OFFSET)
        {
            SetDsLpmTcamIpv4HalfKeyAd(V, pointer_f, tcam_ad, p_table->stage_base);
            SetDsLpmTcamIpv4HalfKeyAd(V, lpmPipelineValid_f, tcam_ad, 1);
            SetDsLpmTcamIpv4HalfKeyAd(V, indexMask_f, tcam_ad, p_table->idx_mask);
            SetDsLpmTcamIpv4HalfKeyAd(V, lpmStartByte_f, tcam_ad, (p_calpm_prefix->prefix_len/8));
        }
        else
        {
            SetDsLpmTcamIpv4HalfKeyAd(V, lpmPipelineValid_f, tcam_ad, 0);
        }
    }

    cmd = DRV_IOW(ad_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, key_index, cmd, &tcam_ad);

    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_SA_LOOKUP))
    {
        sys_usw_get_tbl_id(lchip, CTC_IP_VER_4, 0, tbl_type, NULL, &sa_ad_id);

        cmd = DRV_IOW(sa_ad_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, key_index, cmd, &tcam_ad);
    }
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: write %s[%d] ad\n", TABLE_NAME(lchip, ad_id), key_index);

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_ipuc_tcam_write_ipv6_key(uint8 lchip, sys_ipuc_tcam_data_t *p_data)
{
    sys_ipuc_info_t  *p_ipuc_info    = NULL;
    sys_calpm_prefix_t  *p_calpm_prefix  = NULL;
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;
    sys_ipuc_tbl_type_t tbl_type = SYS_TBL_TCAM;
    sys_calpm_tbl_t* p_table = NULL;
    ipv6_addr_t ipv6_data, ipv6_mask;
    ds_t tcamkey, tcammask;
    tbl_entry_t tbl_ipkey;
    uint32 key_id, sa_key_id;
    uint32 cmd = 0;
    uint32 key_index = 0;
    uint8 use_short_key = 0;
    uint8 masklen = 0;
    uint8 is_pointer = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_PTR_VALID_CHECK(p_data->info);

    if (p_data->key_type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        p_ipuc_info = (sys_ipuc_info_t*)p_data->info;

        masklen = p_ipuc_info->masklen;
        key_index = p_ipuc_info->key_index;
        if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE))
        {
            route_mode = SYS_PUBLIC_MODE;
        }
    }
    else if (p_data->key_type == SYS_IPUC_TCAM_FLAG_ALPM)
    {
        p_calpm_prefix = (sys_calpm_prefix_t*)p_data->info;

        masklen = p_data->masklen;
        is_pointer = (masklen > p_calpm_prefix->prefix_len);
        key_index = p_calpm_prefix->key_index[is_pointer];
        p_table = p_calpm_prefix->p_ntable;
        if (is_pointer)
        {
            CTC_PTR_VALID_CHECK(p_table);
        }
    }

    use_short_key = IS_SHORT_KEY(route_mode, key_index);
    if (route_mode == SYS_PRIVATE_MODE)
    {
        tbl_type = use_short_key ? SYS_TBL_TCAM_SHORT : SYS_TBL_TCAM;
    }
    else
    {
        tbl_type = use_short_key ? SYS_TBL_TCAM_PUB_SHORT : SYS_TBL_TCAM_PUB;
    }
    key_index = use_short_key ? (key_index/2) : (key_index/4);

    sys_usw_get_tbl_id(lchip, CTC_IP_VER_6, 1, tbl_type, &key_id, NULL);

    sal_memset(&tcamkey, 0, sizeof(ds_t));
    sal_memset(&tcammask, 0, sizeof(ds_t));
    sal_memset(&ipv6_mask, 0, sizeof(ipv6_addr_t));

    if (use_short_key)
    {
        if (p_data->key_type == SYS_IPUC_TCAM_FLAG_TCAM)
        {
            sal_memset(&ipv6_data, 0, sizeof(ipv6_addr_t));
            ipv6_data[1] = p_ipuc_info->ip.ipv6[3];
            ipv6_data[0] = p_ipuc_info->ip.ipv6[2];
            SetDsLpmTcamIpv6SingleKey(A, ipAddr_f, tcamkey, ipv6_data);
            SetDsLpmTcamIpv6SingleKey(V, vrfId_f, tcamkey, p_ipuc_info->vrf_id);
            IPV6_LEN_TO_MASK(ipv6_mask, masklen);
        }
        else if (p_data->key_type == SYS_IPUC_TCAM_FLAG_ALPM)
        {
            sal_memset(&ipv6_data, 0, sizeof(ipv6_addr_t));
            ipv6_data[1] = p_calpm_prefix->ip32[0];
            ipv6_data[0] = p_calpm_prefix->ip32[1];
            SetDsLpmTcamIpv6SingleKey(A, ipAddr_f, tcamkey, ipv6_data);
            SetDsLpmTcamIpv6SingleKey(V, vrfId_f, tcamkey, p_calpm_prefix->vrf_id);
            IPV6_LEN_TO_MASK(ipv6_mask, p_calpm_prefix->prefix_len);
        }
        sal_memcpy(&ipv6_mask[0], &ipv6_mask[2], sizeof(uint32) * 2);

        SetDsLpmTcamIpv6SingleKey(A, ipAddr_f, tcammask, ipv6_mask);
        if (route_mode == SYS_PRIVATE_MODE)
        {
            SetDsLpmTcamIpv6SingleKey(V, vrfId_f, tcammask, 0x1FFF);
        }
        else
        {
            SetDsLpmTcamIpv6SingleKey(V, vrfId_f, tcammask, 0);
        }

        SetDsLpmTcamIpv6SingleKey(V, lpmTcamKeyType0_f, tcamkey, 1);
        SetDsLpmTcamIpv6SingleKey(V, lpmTcamKeyType1_f, tcamkey, 1);

        SetDsLpmTcamIpv6SingleKey(V, lpmTcamKeyType0_f, tcammask, 0x1);
        SetDsLpmTcamIpv6SingleKey(V, lpmTcamKeyType1_f, tcammask, 0x1);
    }
    else
    {
        if (p_data->key_type == SYS_IPUC_TCAM_FLAG_TCAM)
        {
            sal_memcpy(&ipv6_data, &(p_ipuc_info->ip.ipv6), sizeof(ipv6_addr_t));
            SetDsLpmTcamIpv6DoubleKey0(A, ipAddr_f, tcamkey, ipv6_data);
            SetDsLpmTcamIpv6DoubleKey0(V, vrfId_f, tcamkey, p_ipuc_info->vrf_id);
            IPV6_LEN_TO_MASK(ipv6_mask, masklen);
        }
        else if (p_data->key_type == SYS_IPUC_TCAM_FLAG_ALPM)
        {
            sal_memset(&ipv6_data, 0, sizeof(ipv6_addr_t));
            ipv6_data[3] = p_calpm_prefix->ip32[0];
            ipv6_data[2] = p_calpm_prefix->ip32[1];
            SetDsLpmTcamIpv6DoubleKey0(A, ipAddr_f, tcamkey, ipv6_data);
            SetDsLpmTcamIpv6DoubleKey0(V, vrfId_f, tcamkey, p_calpm_prefix->vrf_id);
            IPV6_LEN_TO_MASK(ipv6_mask, p_calpm_prefix->prefix_len);
        }

        /* write key */
        /* DRV_SET_FIELD_A, ipv6_data must use little india */
        SetDsLpmTcamIpv6DoubleKey0(A, ipAddr_f, tcammask, ipv6_mask);
        if (route_mode == SYS_PRIVATE_MODE)
        {
            SetDsLpmTcamIpv6DoubleKey0(V, vrfId_f, tcammask, 0x1FFF);
        }
        else
        {
            SetDsLpmTcamIpv6DoubleKey0(V, vrfId_f, tcammask, 0);
        }

        SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType0_f, tcamkey, 1);
        SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType1_f, tcamkey, 1);
        SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType2_f, tcamkey, 1);
        SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType3_f, tcamkey, 1);

        SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType0_f, tcammask, 0x1);
        SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType1_f, tcammask, 0x1);
        SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType2_f, tcammask, 0x1);
        SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType3_f, tcammask, 0x1);
    }
    tbl_ipkey.data_entry = (uint32*)&tcamkey;
    tbl_ipkey.mask_entry = (uint32*)&tcammask;

    cmd = DRV_IOW(key_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, key_index, cmd, &tbl_ipkey);

    if(CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_6], SYS_TCAM_SA_LOOKUP))
    {
        sys_usw_get_tbl_id(lchip, CTC_IP_VER_6, 0, tbl_type, &sa_key_id, NULL);

        cmd = DRV_IOW(sa_key_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, key_index, cmd, &tbl_ipkey);
    }
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: write %s[%d]\n", TABLE_NAME(lchip, key_id), key_index);

    return CTC_E_NONE;
}

STATIC int32
_sys_duet2_ipuc_tcam_write_ipv6_ad(uint8 lchip, sys_ipuc_tcam_data_t *p_data)
{
    sys_ipuc_info_t  *p_ipuc_info    = NULL;
    sys_calpm_prefix_t  *p_calpm_prefix  = NULL;
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;
    sys_ipuc_tbl_type_t tbl_type = SYS_TBL_TCAM;
    sys_calpm_tbl_t* p_table = NULL;
    ds_t tcam_ad;
    uint32 ad_id, sa_ad_id;
    uint32 cmd = 0;
    uint32 ad_index = 0;
    uint32 key_index = 0;
    uint8 use_short_key = 0;
    uint8 masklen = 0;
    uint8 is_pointer = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_PTR_VALID_CHECK(p_data->info);
    sal_memset(&tcam_ad, 0, sizeof(tcam_ad));

    if (p_data->key_type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        p_ipuc_info = (sys_ipuc_info_t*)p_data->info;

        masklen = p_ipuc_info->masklen;
        key_index = p_ipuc_info->key_index;
        ad_index = p_ipuc_info->ad_index;
        if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE))
        {
            route_mode = SYS_PUBLIC_MODE;
        }
    }
    else if (p_data->key_type == SYS_IPUC_TCAM_FLAG_ALPM)
    {
        p_calpm_prefix = (sys_calpm_prefix_t*)p_data->info;

        masklen = p_data->masklen;
        is_pointer = (masklen > p_calpm_prefix->prefix_len);
        key_index = p_calpm_prefix->key_index[is_pointer];
        ad_index = p_data->ad_index;
        p_table = p_calpm_prefix->p_ntable;
        if (is_pointer)
        {
            CTC_PTR_VALID_CHECK(p_table);
        }
    }

    if (p_data->opt_type == DO_UPDATE)
    {
        if (key_index == INVALID_NEXTHOP_OFFSET)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "key index is invalid %d, ad_index = %d\r\n", key_index, ad_index);
            return CTC_E_INVALID_PARAM;
        }

        use_short_key = IS_SHORT_KEY(SYS_PRIVATE_MODE, key_index);
        tbl_type = use_short_key ? SYS_TBL_TCAM_SHORT : SYS_TBL_TCAM;

        sys_usw_get_tbl_id(lchip, CTC_IP_VER_6, 1, tbl_type, NULL, &ad_id);

        cmd = DRV_IOR(ad_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, key_index, cmd, &tcam_ad);

        SetDsLpmTcamIpv6DoubleKey0Ad(V, nexthop_f, &tcam_ad, ad_index);

        cmd = DRV_IOW(ad_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, key_index, cmd, &tcam_ad);

        if(CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_6], SYS_TCAM_SA_LOOKUP))
        {
            sys_usw_get_tbl_id(lchip, CTC_IP_VER_6, 0, tbl_type, NULL, &sa_ad_id);

            cmd = DRV_IOW(sa_ad_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, key_index, cmd, &tcam_ad);
        }
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: update %s[%d] ad_index = %d\n", TABLE_NAME(lchip, ad_id), key_index, ad_index);

        return CTC_E_NONE;
    }

    use_short_key = IS_SHORT_KEY(route_mode, key_index);
    if (route_mode == SYS_PRIVATE_MODE)
    {
        tbl_type = use_short_key ? SYS_TBL_TCAM_SHORT : SYS_TBL_TCAM;
    }
    else
    {
        tbl_type = use_short_key ? SYS_TBL_TCAM_PUB_SHORT : SYS_TBL_TCAM_PUB;
    }

    sys_usw_get_tbl_id(lchip, CTC_IP_VER_6, 1, tbl_type, NULL, &ad_id);

    if (p_data->key_type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        sal_memset(&tcam_ad, 0, sizeof(tcam_ad));
        SetDsLpmTcamIpv6DoubleKey0Ad(V, nexthop_f, tcam_ad, ad_index);
    }
    else if (p_data->key_type == SYS_IPUC_TCAM_FLAG_ALPM)
    {
        if ((p_calpm_prefix->key_index[CALPM_TYPE_NEXTHOP] != INVALID_POINTER_OFFSET) ||
            (p_calpm_prefix->is_pushed && p_calpm_prefix->is_mask_valid))
        {
            if (!is_pointer)
            {
                ad_index = p_data->ad_index;
            }
            else
            {
                cmd = DRV_IOR(ad_id, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, key_index, cmd, &tcam_ad);
                ad_index = GetDsLpmTcamIpv6DoubleKey0Ad(V, nexthop_f, tcam_ad);
            }
            sal_memset(&tcam_ad, 0, sizeof(tcam_ad));
            SetDsLpmTcamIpv6DoubleKey0Ad(V, nexthop_f, tcam_ad, ad_index);
        }
        else
        {
            SetDsLpmTcamIpv6DoubleKey0Ad(V, nexthop_f, tcam_ad, INVALID_DRV_NEXTHOP_OFFSET);
        }

        if (p_calpm_prefix->key_index[CALPM_TYPE_POINTER] != INVALID_POINTER_OFFSET)
        {
            SetDsLpmTcamIpv6DoubleKey0Ad(V, pointer_f, tcam_ad, p_table->stage_base);
            SetDsLpmTcamIpv6DoubleKey0Ad(V, lpmPipelineValid_f, tcam_ad, 1);
            SetDsLpmTcamIpv6DoubleKey0Ad(V, indexMask_f, tcam_ad, p_table->idx_mask);
            SetDsLpmTcamIpv6DoubleKey0Ad(V, lpmStartByte_f, tcam_ad, (p_calpm_prefix->prefix_len/8));
        }
        else
        {
            SetDsLpmTcamIpv6DoubleKey0Ad(V, lpmPipelineValid_f, tcam_ad, 0);
        }
    }

    cmd = DRV_IOW(ad_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, key_index, cmd, &tcam_ad);

    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_6], SYS_TCAM_SA_LOOKUP))
    {
        sys_usw_get_tbl_id(lchip, CTC_IP_VER_6, 0, tbl_type, NULL, &sa_ad_id);

        cmd = DRV_IOW(sa_ad_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, key_index, cmd, &tcam_ad);
    }
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: write %s[%d]\n", TABLE_NAME(lchip, ad_id), key_index);

    return CTC_E_NONE;
}

int32
_sys_duet2_ipuc_tcam_remove_key(uint8 lchip, sys_ipuc_tcam_data_t* p_data)
{
    sys_ipuc_info_t  *p_ipuc_info    = NULL;
    sys_calpm_prefix_t  *p_calpm_prefix  = NULL;
    sys_ipuc_tbl_type_t tbl_type = 0;
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;
    sys_calpm_tbl_t* p_table = NULL;
    ds_t tcam_ad;
    uint32 key_id, sa_key_id;
    uint32 ad_id, sa_ad_id;
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 key_index = 0;
    uint32 tcam_ad_index = 0;
    uint8 is_pointer = 0;
    uint8 use_short_key = 0;
    uint8 ip_ver = CTC_IP_VER_4;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_PTR_VALID_CHECK(p_data->info);

    if (p_data->key_type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        p_ipuc_info = (sys_ipuc_info_t*)p_data->info;

        ip_ver = SYS_IPUC_VER(p_ipuc_info);
        key_index = p_ipuc_info->key_index;
        if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE))
        {
            route_mode = SYS_PUBLIC_MODE;
        }
    }
    else if (p_data->key_type == SYS_IPUC_TCAM_FLAG_ALPM)
    {
        p_calpm_prefix = (sys_calpm_prefix_t*)p_data->info;

        ip_ver = p_calpm_prefix->ip_ver;
        key_index = p_data->key_index;
        p_table = p_calpm_prefix->p_ntable;
        is_pointer = (p_data->masklen > p_calpm_prefix->prefix_len);
    }
    tcam_ad_index = key_index;

    if (ip_ver == CTC_IP_VER_4)
    {
        tbl_type = (route_mode == SYS_PRIVATE_MODE) ? SYS_TBL_TCAM : SYS_TBL_TCAM_PUB;
    }
    else
    {
        use_short_key = IS_SHORT_KEY(route_mode, key_index);
        if (route_mode == SYS_PRIVATE_MODE)
        {
            tbl_type = use_short_key ? SYS_TBL_TCAM_SHORT : SYS_TBL_TCAM;
        }
        else
        {
            tbl_type = use_short_key ? SYS_TBL_TCAM_PUB_SHORT : SYS_TBL_TCAM_PUB;
        }
        key_index = use_short_key ? (key_index/2) : (key_index/4);
    }

    sys_usw_get_tbl_id(lchip, ip_ver, 1, tbl_type, &key_id, &ad_id);

    if (p_data->key_type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        cmd = DRV_IOD(key_id, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, key_index, cmd, &cmd);
        if(CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_TCAM_SA_LOOKUP))
        {
            sys_usw_get_tbl_id(lchip, ip_ver, 0, tbl_type, &sa_key_id, NULL);

            cmd = DRV_IOD(sa_key_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, key_index, cmd, &cmd);
        }
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: remove %s key index = %d\n", TABLE_NAME(lchip, key_id), key_index);
    }
    else if (p_data->key_type == SYS_IPUC_TCAM_FLAG_ALPM)
    {
        sys_usw_get_tbl_id(lchip, ip_ver, 0, tbl_type, &sa_key_id, &sa_ad_id);

        if ((p_calpm_prefix->key_index[CALPM_TYPE_POINTER] != INVALID_POINTER_OFFSET)
        && (p_calpm_prefix->key_index[CALPM_TYPE_NEXTHOP] != INVALID_POINTER_OFFSET))
        {
            /* only need update tcam ad */
            cmd = DRV_IOR(ad_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, tcam_ad_index, cmd, &tcam_ad);
            if (is_pointer)
            {
                if ((!p_table) || (0 == p_table->count))
                {
                    SetDsLpmTcamIpv4HalfKeyAd(V, pointer_f, tcam_ad, INVALID_DRV_POINTER_OFFSET);
                    SetDsLpmTcamIpv4HalfKeyAd(V, indexMask_f, tcam_ad, 0);
                }
                else
                {
                    SetDsLpmTcamIpv4HalfKeyAd(V, pointer_f, tcam_ad, p_table->stage_base);
                    SetDsLpmTcamIpv4HalfKeyAd(V, indexMask_f, tcam_ad, p_table->idx_mask);
                }
            }
            else
            {
                SetDsLpmTcamIpv4HalfKeyAd(V, nexthop_f, tcam_ad, INVALID_DRV_NEXTHOP_OFFSET); /*nexthop is 18bit*/
            }

            cmd = DRV_IOW(ad_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, tcam_ad_index, cmd, &tcam_ad);

            if(CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_SA_LOOKUP))
            {
                cmd = DRV_IOW(sa_ad_id, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, tcam_ad_index, cmd, &tcam_ad);
            }
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: rewrite %s[%d] key\n", TABLE_NAME(lchip, key_id), key_index);
        }
        else
        {
            if (is_pointer)
            {
                if (p_table && p_table->count)
                {
                    /* only need update tcam ad */
                    cmd = DRV_IOR(ad_id, DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, tcam_ad_index, cmd, &tcam_ad);

                    SetDsLpmTcamIpv4HalfKeyAd(V, pointer_f, tcam_ad, p_table->stage_base);
                    SetDsLpmTcamIpv4HalfKeyAd(V, indexMask_f, tcam_ad, p_table->idx_mask);

                    cmd = DRV_IOW(ad_id, DRV_ENTRY_FLAG);
                    DRV_IOCTL(lchip, tcam_ad_index, cmd, &tcam_ad);

                    if(CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_SA_LOOKUP))
                    {
                        cmd = DRV_IOW(sa_ad_id, DRV_ENTRY_FLAG);
                        DRV_IOCTL(lchip, tcam_ad_index, cmd, &tcam_ad);
                    }
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: rewrite %s[%d] key\n", TABLE_NAME(lchip, key_id), key_index);

                    return CTC_E_NONE;
                }
            }

            /* remove tcam key */
            cmd = DRV_IOD(key_id, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, key_index, cmd, &cmd);
            if(CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_TCAM_SA_LOOKUP))
            {
                cmd = DRV_IOD(sa_key_id, DRV_ENTRY_FLAG);
                DRV_IOCTL(lchip, key_index, cmd, &cmd);
            }
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: remove %s[%d] key\n", TABLE_NAME(lchip, key_id), key_index);
        }
    }

    return ret;
}

int32
_sys_duet2_ipuc_tcam_move_key(uint8 lchip, uint32 new_index, uint32 old_index, sys_ipuc_ofb_cb_t *p_ofb_cb)
{
    uint32 cmdr, cmdw;
    ds_t tcam_ad;
    ds_t tcamkey, tcammask;
    tbl_entry_t tbl_ipkey;
    sys_ipuc_tcam_data_t tcam_data = {0};
    sys_ipuc_info_t  *p_ipuc_info    = NULL;
    sys_calpm_prefix_t  *p_calpm_prefix  = NULL;
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;
    sys_ipuc_tbl_type_t tbl_type[2]; /* index 0 is old, index 1 is new */
    uint32 key_id[2], sa_key_id[2];
    uint32 ad_id[2], sa_ad_id[2];
    uint32 key_index[2];
    uint8 use_short_key[2] = {0};
    uint8 ip_ver = CTC_IP_VER_4;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (p_ofb_cb->type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        p_ipuc_info = (sys_ipuc_info_t*)p_ofb_cb->user_data;

        if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_IS_IPV6))
        {
            ip_ver = CTC_IP_VER_6;
        }

        if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE))
        {
            route_mode = SYS_PUBLIC_MODE;
        }
    }
    else if (p_ofb_cb->type == SYS_IPUC_TCAM_FLAG_ALPM)
    {
        p_calpm_prefix = (sys_calpm_prefix_t*)p_ofb_cb->user_data;

        ip_ver = p_calpm_prefix->ip_ver;
    }

    if (ip_ver == CTC_IP_VER_4)
    {
        key_index[0] = old_index;
        key_index[1] = new_index;
    }
    else
    {
        use_short_key[0] = IS_SHORT_KEY(route_mode, old_index);
        use_short_key[1] = IS_SHORT_KEY(route_mode, new_index);
        key_index[0] = use_short_key[0] ? (old_index/2) : (old_index/4);
        key_index[1] = use_short_key[1] ? (new_index/2) : (new_index/4);
    }

    if (route_mode == SYS_PRIVATE_MODE)
    {
        tbl_type[0] = use_short_key[0] ? SYS_TBL_TCAM_SHORT : SYS_TBL_TCAM;
        tbl_type[1] = use_short_key[1] ? SYS_TBL_TCAM_SHORT : SYS_TBL_TCAM;
    }
    else
    {
        tbl_type[0] = use_short_key[0] ? SYS_TBL_TCAM_PUB_SHORT : SYS_TBL_TCAM_PUB;
        tbl_type[1] = use_short_key[1] ? SYS_TBL_TCAM_PUB_SHORT : SYS_TBL_TCAM_PUB;
    }
    sys_usw_get_tbl_id(lchip, ip_ver, 1, tbl_type[0], &key_id[0], &ad_id[0]);
    sys_usw_get_tbl_id(lchip, ip_ver, 1, tbl_type[1], &key_id[1], &ad_id[1]);

    cmdr = DRV_IOR(ad_id[0], DRV_ENTRY_FLAG);
    cmdw = DRV_IOW(ad_id[1], DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, old_index, cmdr, &tcam_ad);
    DRV_IOCTL(lchip, new_index, cmdw, &tcam_ad);

    cmdr = DRV_IOR(key_id[0], DRV_ENTRY_FLAG);
    cmdw = DRV_IOW(key_id[1], DRV_ENTRY_FLAG);
    tbl_ipkey.data_entry = (uint32*)&tcamkey;
    tbl_ipkey.mask_entry = (uint32*)&tcammask;
    DRV_IOCTL(lchip, key_index[0], cmdr, &tbl_ipkey);
    if (use_short_key[0] != use_short_key[1])
    {
        uint16 vrfid, vrfid_mask;
        ipv6_addr_t ipv6_data, ipv6_mask;
        if (use_short_key[0])
        {
            GetDsLpmTcamIpv6SingleKey(A, ipAddr_f, &tcamkey, ipv6_data);
            GetDsLpmTcamIpv6SingleKey(A, ipAddr_f, &tcammask, ipv6_mask);
            sal_memcpy(&ipv6_data[2], &ipv6_data[0], sizeof(uint32) * 2);
            sal_memset(&ipv6_data[0], 0, sizeof(uint32) * 2);
            sal_memcpy(&ipv6_mask[2], &ipv6_mask[0], sizeof(uint32) * 2);
            sal_memset(&ipv6_mask[0], 0, sizeof(uint32) * 2);

            vrfid = GetDsLpmTcamIpv6SingleKey(V, vrfId_f, &tcamkey);
            vrfid_mask = GetDsLpmTcamIpv6SingleKey(V, vrfId_f, &tcammask);

            SetDsLpmTcamIpv6DoubleKey0(A, ipAddr_f, tcamkey, ipv6_data);
            SetDsLpmTcamIpv6DoubleKey0(V, vrfId_f, tcamkey, vrfid);
            SetDsLpmTcamIpv6DoubleKey0(A, ipAddr_f, tcammask, ipv6_mask);
            SetDsLpmTcamIpv6DoubleKey0(V, vrfId_f, tcammask, vrfid_mask);

            SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType0_f, tcamkey, 1);
            SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType1_f, tcamkey, 1);
            SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType2_f, tcamkey, 1);
            SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType3_f, tcamkey, 1);

            SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType0_f, tcammask, 0x1);
            SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType1_f, tcammask, 0x1);
            SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType2_f, tcammask, 0x1);
            SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType3_f, tcammask, 0x1);
        }
        else
        {
            GetDsLpmTcamIpv6DoubleKey0(A, ipAddr_f, &tcamkey, ipv6_data);
            GetDsLpmTcamIpv6DoubleKey0(A, ipAddr_f, &tcammask, ipv6_mask);
            sal_memcpy(&ipv6_data[0], &ipv6_data[2], sizeof(uint32) * 2);
            sal_memcpy(&ipv6_mask[0], &ipv6_mask[2], sizeof(uint32) * 2);

            vrfid = GetDsLpmTcamIpv6DoubleKey0(V, vrfId_f, &tcamkey);
            vrfid_mask = GetDsLpmTcamIpv6DoubleKey0(V, vrfId_f, &tcammask);

            SetDsLpmTcamIpv6SingleKey(A, ipAddr_f, tcamkey, ipv6_data);
            SetDsLpmTcamIpv6SingleKey(V, vrfId_f, tcamkey, vrfid);
            SetDsLpmTcamIpv6SingleKey(A, ipAddr_f, tcammask, ipv6_mask);
            SetDsLpmTcamIpv6SingleKey(V, vrfId_f, tcammask, vrfid_mask);

            SetDsLpmTcamIpv6SingleKey(V, lpmTcamKeyType0_f, tcamkey, 1);
            SetDsLpmTcamIpv6SingleKey(V, lpmTcamKeyType1_f, tcamkey, 1);
            SetDsLpmTcamIpv6SingleKey(V, lpmTcamKeyType0_f, tcammask, 0x1);
            SetDsLpmTcamIpv6SingleKey(V, lpmTcamKeyType1_f, tcammask, 0x1);
        }
    }

    DRV_IOCTL(lchip, key_index[1], cmdw, &tbl_ipkey);

    if(CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_TCAM_SA_LOOKUP))
    {
        sys_usw_get_tbl_id(lchip, ip_ver, 0, tbl_type[0], &sa_key_id[0], &sa_ad_id[0]);
        sys_usw_get_tbl_id(lchip, ip_ver, 0, tbl_type[1], &sa_key_id[1], &sa_ad_id[1]);

        cmdr = DRV_IOR(sa_ad_id[0], DRV_ENTRY_FLAG);
        cmdw = DRV_IOW(sa_ad_id[1], DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, old_index, cmdr, &tcam_ad);
        DRV_IOCTL(lchip, new_index, cmdw, &tcam_ad);

        cmdw = DRV_IOW(sa_key_id[1], DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, key_index[1], cmdw, &tbl_ipkey);
    }

    tcam_data.key_type = p_ofb_cb->type;
    tcam_data.info = p_ofb_cb->user_data;
    tcam_data.masklen = p_ofb_cb->masklen;

    /* remove key from old offset */
    if (p_ofb_cb->type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        p_ipuc_info->key_index = old_index;
        _sys_duet2_ipuc_tcam_remove_key(lchip, &tcam_data);
    }
    else
    {
        tcam_data.key_index = old_index;
        _sys_duet2_ipuc_tcam_remove_key(lchip, &tcam_data);
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: move from %s[%d] -> %s[%d] \n", TABLE_NAME(lchip, key_id[0]), old_index, TABLE_NAME(lchip, key_id[1]), new_index);

    return CTC_E_NONE;
}

int32
_sys_duet2_ipuc_tcam_push_down_ad(uint8 lchip, void *p_fndata, void *pdata)
{
    uint8 find = 0;
    uint32 ip_mask = 0;
    ipv6_addr_t ipv6_mask;
    sys_ipuc_info_t  *p_ipuc_info    = NULL;
    sys_calpm_prefix_t  *p_calpm_prefix  = NULL;
    sys_ipuc_tcam_data_t update_data = {0};
    sys_ipuc_ofb_cb_t *p_data = NULL;
    sys_ipuc_ofb_cb_t *p_fn_data = NULL;

    p_fn_data = (sys_ipuc_ofb_cb_t*)p_fndata;
    p_data = (sys_ipuc_ofb_cb_t*)pdata;

    if (p_data->type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        p_ipuc_info = (sys_ipuc_info_t*)p_data->user_data;

        if (CTC_IP_VER_4 == SYS_IPUC_VER(p_ipuc_info))
        {
            IPV4_LEN_TO_MASK(ip_mask, p_ipuc_info->masklen);
        }
        else
        {
            IPV6_LEN_TO_MASK(ipv6_mask, p_ipuc_info->masklen);
        }

        if (p_fn_data->type != SYS_IPUC_TCAM_FLAG_ALPM)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "this is not calpm route!\n");
            return CTC_E_UNEXPECT;
        }

        p_calpm_prefix = (sys_calpm_prefix_t*)p_fn_data->user_data;

        if ((p_calpm_prefix->is_pushed && (p_calpm_prefix->masklen_pushed > p_ipuc_info->masklen)) ||
            (p_calpm_prefix->key_index[CALPM_TYPE_NEXTHOP] != INVALID_NEXTHOP_OFFSET))
        {
            return CTC_E_NONE;
        }

        if (SYS_IPUC_VER(p_ipuc_info) == CTC_IP_VER_4)
        {
            if (((p_ipuc_info->ip.ipv4 & ip_mask) == (p_calpm_prefix->ip32[0] & ip_mask))
                && (p_ipuc_info->vrf_id == p_calpm_prefix->vrf_id))
            {
                find = 1;
            }
        }
        else
        {
            if (((p_ipuc_info->ip.ipv6[3] & ipv6_mask[3]) == (p_calpm_prefix->ip32[0] & ipv6_mask[3])) &&
                ((p_ipuc_info->ip.ipv6[2] & ipv6_mask[2]) == (p_calpm_prefix->ip32[1] & ipv6_mask[2])) &&
                (p_ipuc_info->vrf_id == p_calpm_prefix->vrf_id))
            {
                find = 1;
            }
        }

        /* 2. Find, than update the hash key with the ad_index */
        if (find)
        {
            update_data.opt_type = DO_UPDATE;
            update_data.key_type = SYS_IPUC_TCAM_FLAG_ALPM;
            update_data.masklen = p_calpm_prefix->prefix_len + 1;
            update_data.ad_index = p_ipuc_info->ad_index;
            update_data.info = (void*)p_calpm_prefix;
            if (p_calpm_prefix->ip_ver == CTC_IP_VER_4)
            {
                _sys_duet2_ipuc_tcam_write_ipv4_ad(lchip, &update_data);
            }
            else
            {
                _sys_duet2_ipuc_tcam_write_ipv6_ad(lchip, &update_data);
            }

            p_calpm_prefix->masklen_pushed = p_ipuc_info->masklen;
            p_calpm_prefix->is_mask_valid = 1;
            p_calpm_prefix->is_pushed = 1;
        }
    }
    else if (p_data->type == SYS_IPUC_TCAM_FLAG_ALPM)
    {
        p_calpm_prefix = (sys_calpm_prefix_t*)p_data->user_data;
        if (NULL == p_calpm_prefix)
        {
            return CTC_E_NONE;
        }

        if (p_fn_data->type != SYS_IPUC_TCAM_FLAG_TCAM)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "this is not tcam route!\n");
            return CTC_E_UNEXPECT;
        }

        p_ipuc_info = (sys_ipuc_info_t*)p_fn_data->user_data;
        if (NULL == p_ipuc_info)
        {
            return CTC_E_NOT_EXIST;
        }

        if ((p_calpm_prefix->is_pushed && (p_calpm_prefix->masklen_pushed > p_ipuc_info->masklen)) ||
            (p_calpm_prefix->key_index[CALPM_TYPE_NEXTHOP] != INVALID_NEXTHOP_OFFSET))
        {
            return CTC_E_NONE;
        }

        if (p_ipuc_info->masklen >= sys_duet2_calpm_get_prefix_len(lchip, SYS_IPUC_VER(p_ipuc_info)))
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "push down calpm route error, ipuc masklen: %d, calpm prefix: %d\n",
                p_ipuc_info->masklen, sys_duet2_calpm_get_prefix_len(lchip, SYS_IPUC_VER(p_ipuc_info)));
            return CTC_E_UNEXPECT;
        }

        if (SYS_IPUC_VER(p_ipuc_info) == CTC_IP_VER_4)
        {
            IPV4_LEN_TO_MASK(ip_mask, p_ipuc_info->masklen);
            if (((p_ipuc_info->ip.ipv4 & ip_mask) == (p_calpm_prefix->ip32[0] & ip_mask))
                && (p_ipuc_info->vrf_id == p_calpm_prefix->vrf_id))
            {
                find = 1;
            }
        }
        else
        {
            IPV6_LEN_TO_MASK(ipv6_mask, p_ipuc_info->masklen);
            if (((p_ipuc_info->ip.ipv6[3] & ipv6_mask[3]) == (p_calpm_prefix->ip32[0] & ipv6_mask[3])) &&
                ((p_ipuc_info->ip.ipv6[2] & ipv6_mask[2]) == (p_calpm_prefix->ip32[1] & ipv6_mask[2])) &&
                (p_ipuc_info->vrf_id == p_calpm_prefix->vrf_id))
            {
                find = 1;
            }
        }

        /* 2. Find, than update the hash key with the ad_index */
        if (find)
        {
            update_data.opt_type = DO_UPDATE;
            update_data.key_type = SYS_IPUC_TCAM_FLAG_ALPM;
            update_data.masklen = p_calpm_prefix->prefix_len + 1;
            update_data.ad_index = p_ipuc_info->ad_index;
            update_data.info = (void*)p_calpm_prefix;
            if (p_calpm_prefix->ip_ver == CTC_IP_VER_4)
            {
                _sys_duet2_ipuc_tcam_write_ipv4_ad(lchip, &update_data);
            }
            else
            {
                _sys_duet2_ipuc_tcam_write_ipv6_ad(lchip, &update_data);
            }

            p_calpm_prefix->masklen_pushed = p_ipuc_info->masklen;
            p_calpm_prefix->is_mask_valid = 1;
            p_calpm_prefix->is_pushed = 1;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_duet2_ipuc_tcam_push_up_ad(uint8 lchip, void *p_fndata, void *pdata)
{
    uint8 find = 0;
    uint32 ip_mask = 0;
    ipv6_addr_t ipv6_mask;
    ctc_slist_t* prefix_slist = NULL;
    sys_ipuc_info_t  *p_ipuc_info    = NULL;
    sys_calpm_prefix_t  *p_calpm_prefix  = NULL;
    sys_ipuc_ofb_cb_t *p_data = NULL;
    sys_ipuc_ofb_cb_t *p_fn_data = NULL;
    sys_calpm_prefix_node_t *prefix_node = NULL;

    p_fn_data = (sys_ipuc_ofb_cb_t*)p_fndata;
    p_data = (sys_ipuc_ofb_cb_t*)pdata;

    if (p_data->type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        prefix_slist = (ctc_slist_t*)p_data->priv_data;
        p_ipuc_info = (sys_ipuc_info_t*)p_data->user_data;

        if (CTC_IP_VER_4 == SYS_IPUC_VER(p_ipuc_info))
        {
            IPV4_LEN_TO_MASK(ip_mask, p_ipuc_info->masklen);
        }
        else
        {
            IPV6_LEN_TO_MASK(ipv6_mask, p_ipuc_info->masklen);
        }

        if (p_fn_data->type != SYS_IPUC_TCAM_FLAG_ALPM)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "this is not calpm route!\n");
            return CTC_E_UNEXPECT;
        }

        p_calpm_prefix = (sys_calpm_prefix_t*)p_fn_data->user_data;

        if (!p_calpm_prefix->is_pushed)
        {
            return CTC_E_NONE;
        }

        if (SYS_IPUC_VER(p_ipuc_info) == CTC_IP_VER_4)
        {
            if (((p_ipuc_info->ip.ipv4 & ip_mask) == (p_calpm_prefix->ip32[0] & ip_mask)) &&
                    (p_ipuc_info->vrf_id == p_calpm_prefix->vrf_id) &&
                    p_calpm_prefix->is_pushed &&
                    (p_calpm_prefix->masklen_pushed == p_ipuc_info->masklen))
            {
                find = 1;
            }
        }
        else
        {
            if (((p_ipuc_info->ip.ipv6[3] & ipv6_mask[3]) == (p_calpm_prefix->ip32[0] & ipv6_mask[3])) &&
                    ((p_ipuc_info->ip.ipv6[2] & ipv6_mask[2]) == (p_calpm_prefix->ip32[1] & ipv6_mask[2])) &&
                    (p_ipuc_info->vrf_id == p_calpm_prefix->vrf_id) &&
                    p_calpm_prefix->is_pushed &&
                    (p_calpm_prefix->masklen_pushed == p_ipuc_info->masklen))
            {
                find = 1;
            }
        }

        /* 2. Find, than update ad index with other little masklen route */
        if (find)
        {
            sys_ipuc_tcam_data_t update_data = {0};

            prefix_node = (sys_calpm_prefix_node_t*)mem_malloc(MEM_IPUC_MODULE, sizeof(sys_calpm_prefix_node_t));
            if (NULL == prefix_node)
            {
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
                return CTC_E_NO_MEMORY;
            }

            update_data.opt_type = DO_UPDATE;
            update_data.key_type = SYS_IPUC_TCAM_FLAG_ALPM;
            update_data.masklen = p_calpm_prefix->prefix_len + 1;
            update_data.ad_index = INVALID_DRV_NEXTHOP_OFFSET;
            update_data.info = (void*)p_calpm_prefix;
            if (p_calpm_prefix->ip_ver == CTC_IP_VER_4)
            {
                _sys_duet2_ipuc_tcam_write_ipv4_ad(lchip, &update_data);
            }
            else
            {
                _sys_duet2_ipuc_tcam_write_ipv6_ad(lchip, &update_data);
            }

            p_calpm_prefix->masklen_pushed = 0;
            p_calpm_prefix->is_mask_valid = 0;
            p_calpm_prefix->is_pushed = 0;

            prefix_node->p_calpm_prefix= p_calpm_prefix;
            prefix_node->head.next = NULL;

            ctc_slist_add_tail(prefix_slist, &prefix_node->head);
        }
    }

    return CTC_E_NONE;
}

int32
_sys_duet2_ipuc_tcam_get_max_block_num(uint8 lchip,
                                uint8 ip_ver,
                                sys_ipuc_route_mode_t route_mode,
                                uint8* max_block_num)
{
    uint8 calpm_adj = 0;

    if ((route_mode == SYS_PRIVATE_MODE) && CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_ALPM_LOOKUP))
    {
         if (ip_ver == CTC_IP_VER_4)
        {
            calpm_adj = ((sys_duet2_calpm_get_prefix_len(lchip, ip_ver) == 16) && p_usw_ipuc_master[lchip]->host_lpm_mode);
            *max_block_num = 32 - MAX_CALPM_MASKLEN(lchip, ip_ver) + p_usw_ipuc_master[lchip]->host_lpm_mode +
                                calpm_adj + sys_duet2_calpm_get_prefix_len(lchip, ip_ver) + 1;
        }
        else
        {
            *max_block_num = 128 - MAX_CALPM_MASKLEN(lchip, ip_ver) + p_usw_ipuc_master[lchip]->host_lpm_mode +
                                sys_duet2_calpm_get_prefix_len(lchip, ip_ver) + 1;
        }
    }
    else
    {
         if (ip_ver == CTC_IP_VER_4)
        {
            *max_block_num = 32 + 1;
            *max_block_num += (route_mode == SYS_PRIVATE_MODE) ? p_usw_ipuc_master[lchip]->host_lpm_mode : 0;
        }
        else
        {
            *max_block_num = 128 + 1;
            *max_block_num += (route_mode == SYS_PRIVATE_MODE) ? p_usw_ipuc_master[lchip]->host_lpm_mode : 0;
        }
    }

    return CTC_E_NONE;
}

int32
_sys_duet2_ipuc_tcam_get_calpm_blockid(uint8 lchip, uint8 ip_ver, uint8* block_id)
{
    uint8 calpm_adj = 0;
    uint8 ipv6_max_block = 0;

    if (!CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_ALPM_LOOKUP))
    {
        *block_id = 0;
        return CTC_E_NONE;
    }

     if (ip_ver == CTC_IP_VER_4)
    {
        calpm_adj = ((sys_duet2_calpm_get_prefix_len(lchip, ip_ver) == 16) && p_usw_ipuc_master[lchip]->host_lpm_mode);
        if (p_usw_ipuc_master[lchip]->tcam_share_mode[SYS_PRIVATE_MODE] != SYS_SHARE_NONE)
        {
            _sys_duet2_ipuc_tcam_get_max_block_num(lchip, CTC_IP_VER_6, SYS_PRIVATE_MODE, &ipv6_max_block);
        }
        *block_id = ipv6_max_block + 32 - MAX_CALPM_MASKLEN(lchip, ip_ver) + p_usw_ipuc_master[lchip]->host_lpm_mode + calpm_adj;
    }
    else
    {
        *block_id = 128 - MAX_CALPM_MASKLEN(lchip, ip_ver) + p_usw_ipuc_master[lchip]->host_lpm_mode;
    }

    return CTC_E_NONE;
}

int32
_sys_duet2_ipuc_tcam_get_blockid(uint8 lchip,
                                uint8 ip_ver,
                                uint8 masklen,
                                sys_ipuc_route_mode_t route_mode,
                                uint8 hash_conflict,
                                uint8* block_id)
{
    uint8 calpm_adj = 0;
    uint8 ipv6_max_block = 0;

    if ((route_mode == SYS_PRIVATE_MODE) && CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_ALPM_LOOKUP))
    {
        if (ip_ver == CTC_IP_VER_4)
        {
            calpm_adj = ((sys_duet2_calpm_get_prefix_len(lchip, ip_ver) == 16) && p_usw_ipuc_master[lchip]->host_lpm_mode);
            if (p_usw_ipuc_master[lchip]->tcam_share_mode[SYS_PRIVATE_MODE] != SYS_SHARE_NONE)
            {
                _sys_duet2_ipuc_tcam_get_max_block_num(lchip, CTC_IP_VER_6, SYS_PRIVATE_MODE, &ipv6_max_block);
            }

            if (masklen >= MAX_CALPM_MASKLEN(lchip, ip_ver))
            {
                if (IS_MAX_MASKLEN(ip_ver, masklen))
                {
                    *block_id = hash_conflict ? p_usw_ipuc_master[lchip]->host_lpm_mode : 0;
                }
                else
                {
                    *block_id = 32 - masklen + p_usw_ipuc_master[lchip]->host_lpm_mode;
                }
            }
            else if ((masklen >= MIN_CALPM_MASKLEN(lchip, ip_ver)) && (masklen < MAX_CALPM_MASKLEN(lchip, ip_ver)))
            {
                *block_id = 32 - MAX_CALPM_MASKLEN(lchip, ip_ver) + p_usw_ipuc_master[lchip]->host_lpm_mode + calpm_adj;
            }
            else
            {
                *block_id = 32 - MAX_CALPM_MASKLEN(lchip, ip_ver) + p_usw_ipuc_master[lchip]->host_lpm_mode + calpm_adj +
                                (sys_duet2_calpm_get_prefix_len(lchip, ip_ver) - masklen);
            }

            *block_id += ipv6_max_block;
        }
        else
        {
            if (masklen >= MAX_CALPM_MASKLEN(lchip, ip_ver))
            {
                if (IS_MAX_MASKLEN(ip_ver, masklen) && hash_conflict)
                {
                    *block_id = p_usw_ipuc_master[lchip]->host_lpm_mode;
                }
                else if (IS_MAX_MASKLEN(ip_ver, masklen))
                {
                    *block_id = 0;
                }
                else
                {
                    *block_id = 128 - masklen + p_usw_ipuc_master[lchip]->host_lpm_mode;
                }
            }
            else if ((masklen >= MIN_CALPM_MASKLEN(lchip, ip_ver)) && (masklen < MAX_CALPM_MASKLEN(lchip, ip_ver)))
            {
                *block_id = 128 - MAX_CALPM_MASKLEN(lchip, ip_ver) + p_usw_ipuc_master[lchip]->host_lpm_mode;
            }
            else
            {
                *block_id = 128 - MAX_CALPM_MASKLEN(lchip, ip_ver) + p_usw_ipuc_master[lchip]->host_lpm_mode +
                                (sys_duet2_calpm_get_prefix_len(lchip, ip_ver) - masklen);
            }
        }
    }
    else
    {
        if (ip_ver == CTC_IP_VER_4)
        {
            *block_id = 32 - masklen;
            if (p_usw_ipuc_master[lchip]->tcam_share_mode[SYS_PRIVATE_MODE] != SYS_SHARE_NONE)
            {
                _sys_duet2_ipuc_tcam_get_max_block_num(lchip, CTC_IP_VER_6, route_mode, &ipv6_max_block);
            }
            *block_id += ipv6_max_block;
        }
        else
        {
            *block_id = 128 - masklen;
        }
        *block_id += (route_mode == SYS_PRIVATE_MODE) ? p_usw_ipuc_master[lchip]->host_lpm_mode : 0;
    }

    return CTC_E_NONE;
}

#define ____IPUC_TCAM_OUTER_FUNCTION________________________
int32
sys_duet2_ipuc_tcam_get_blockid(uint8 lchip, sys_ipuc_tcam_data_t *p_data, uint8 *block_id)
{
    uint8 ip_ver = 0;
    uint8 masklen = 0;
    uint8 hash_conflict = 0;
    sys_ipuc_info_t  *p_ipuc_info    = NULL;
    sys_calpm_prefix_t  *p_calpm_prefix  = NULL;
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;

    if (p_data->key_type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        p_ipuc_info = (sys_ipuc_info_t*)p_data->info;

        ip_ver = SYS_IPUC_VER(p_ipuc_info);
        masklen = p_ipuc_info->masklen;
        hash_conflict = CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_HASH_CONFLICT);
        route_mode = CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE);
    }
    else if (p_data->key_type == SYS_IPUC_TCAM_FLAG_ALPM)
    {
        p_calpm_prefix = (sys_calpm_prefix_t*)p_data->info;

        ip_ver = p_calpm_prefix->ip_ver;
        masklen = p_data->masklen;
    }

    return _sys_duet2_ipuc_tcam_get_blockid(lchip, ip_ver, masklen, route_mode, hash_conflict, block_id);
}

int32
sys_duet2_ipuc_tcam_write_key(uint8 lchip, sys_ipuc_tcam_data_t *p_data)
{
    sys_ipuc_info_t  *p_ipuc_info    = NULL;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_PTR_VALID_CHECK(p_data->info);

    p_ipuc_info = (sys_ipuc_info_t*)p_data->info;

    if (p_data->opt_type == DO_DEL)
    {
        return _sys_duet2_ipuc_tcam_remove_key(lchip, p_data);
    }

    if (CTC_IP_VER_4 == SYS_IPUC_VER(p_ipuc_info))
    {
        _sys_duet2_ipuc_tcam_write_ipv4_key(lchip, p_data);
    }
    else
    {
        _sys_duet2_ipuc_tcam_write_ipv6_key(lchip, p_data);
    }

    return CTC_E_NONE;
}

int32
sys_duet2_ipuc_tcam_write_ad(uint8 lchip, sys_ipuc_tcam_data_t *p_data)
{
    sys_ipuc_info_t  *p_ipuc_info    = NULL;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_PTR_VALID_CHECK(p_data->info);

    p_ipuc_info = (sys_ipuc_info_t*)p_data->info;

    if (p_data->opt_type != DO_DEL)
    {
        if (CTC_IP_VER_4 == SYS_IPUC_VER(p_ipuc_info))
        {
            _sys_duet2_ipuc_tcam_write_ipv4_ad(lchip, p_data);
        }
        else
        {
            _sys_duet2_ipuc_tcam_write_ipv6_ad(lchip, p_data);
        }
    }

    sys_duet2_ipuc_tcam_update_ad(lchip, p_data);

    return CTC_E_NONE;
}

int32
sys_duet2_ipuc_tcam_write_calpm_key(uint8 lchip, sys_ipuc_tcam_data_t *p_data)
{
    sys_calpm_prefix_t  *p_calpm_prefix  = NULL;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_PTR_VALID_CHECK(p_data->info);

    p_calpm_prefix = (sys_calpm_prefix_t*)p_data->info;

    if (p_data->opt_type == DO_DEL)
    {
        return _sys_duet2_ipuc_tcam_remove_key(lchip, p_data);
    }

    if (CTC_IP_VER_4 == p_calpm_prefix->ip_ver)
    {
        _sys_duet2_ipuc_tcam_write_ipv4_key(lchip, p_data);
    }
    else
    {
        _sys_duet2_ipuc_tcam_write_ipv6_key(lchip, p_data);
    }

    return CTC_E_NONE;
}

int32
sys_duet2_ipuc_tcam_write_calpm_ad(uint8 lchip, sys_ipuc_tcam_data_t *p_data)
{
    sys_calpm_prefix_t  *p_calpm_prefix  = NULL;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_PTR_VALID_CHECK(p_data->info);

    p_calpm_prefix = (sys_calpm_prefix_t*)p_data->info;

    if (CTC_IP_VER_4 == p_calpm_prefix->ip_ver)
    {
        _sys_duet2_ipuc_tcam_write_ipv4_ad(lchip, p_data);
    }
    else
    {
        _sys_duet2_ipuc_tcam_write_ipv6_ad(lchip, p_data);
    }

    return CTC_E_NONE;
}

int32
sys_duet2_ipuc_tcam_move(
                            uint8 lchip,
                            uint32 new_index,
                            uint32 old_index,
                            void *pdata)
{
    sys_ipuc_ofb_cb_t *p_ofb_cb = (sys_ipuc_ofb_cb_t *)pdata;
    sys_ipuc_info_t  *p_ipuc_info    = NULL;
    sys_calpm_prefix_t  *p_calpm_prefix  = NULL;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (p_ofb_cb->type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        p_ipuc_info = (sys_ipuc_info_t*)p_ofb_cb->user_data;
    }
    else if (p_ofb_cb->type == SYS_IPUC_TCAM_FLAG_ALPM)
    {
        p_calpm_prefix = (sys_calpm_prefix_t*)p_ofb_cb->user_data;
    }
    else
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "old sort offset type invalid, old_index: %d, new_index: %d\n", old_index, new_index);
        return CTC_E_NOT_EXIST;
    }

    _sys_duet2_ipuc_tcam_move_key(lchip, new_index, old_index, p_ofb_cb);

    if (p_ofb_cb->type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        p_ipuc_info->key_index = new_index;
    }
    else
    {
        if (p_calpm_prefix->key_index[CALPM_TYPE_NEXTHOP] != INVALID_POINTER_OFFSET)
        {
            p_calpm_prefix->key_index[CALPM_TYPE_NEXTHOP] = new_index;
        }

        if (p_calpm_prefix->key_index[CALPM_TYPE_POINTER] != INVALID_POINTER_OFFSET)
        {
            p_calpm_prefix->key_index[CALPM_TYPE_POINTER] = new_index;
        }
    }

    return CTC_E_NONE;
}

int32
sys_duet2_ipuc_tcam_update_ad(uint8 lchip, sys_ipuc_tcam_data_t *p_data)
{
    uint8 ip_ver = 0;
    uint8 masklen = 0;
    uint8 min_block_id = 0;
    uint8 max_block_id = 0;
    sys_ipuc_info_t  *p_ipuc_info = NULL;
    sys_ipuc_ofb_cb_t ofb_cb = {0};
    sys_calpm_prefix_t  *p_calpm_prefix  = NULL;
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_PTR_VALID_CHECK(p_data->info);

    if (p_data->key_type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        p_ipuc_info = (sys_ipuc_info_t*)p_data->info;

        ip_ver = SYS_IPUC_VER(p_ipuc_info);
        masklen = p_ipuc_info->masklen;
        if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE))
        {
            return CTC_E_NONE;
        }

        if (masklen >= sys_duet2_calpm_get_prefix_len(lchip, ip_ver))
        {
            return CTC_E_NONE;
        }

        _sys_duet2_ipuc_tcam_get_calpm_blockid(lchip, ip_ver, &max_block_id);
        min_block_id = max_block_id;
    }
    else if (p_data->key_type == SYS_IPUC_TCAM_FLAG_ALPM)
    {
        p_calpm_prefix = (sys_calpm_prefix_t*)p_data->info;

        ip_ver = p_calpm_prefix->ip_ver;

        _sys_duet2_ipuc_tcam_get_blockid(lchip, ip_ver, p_calpm_prefix->prefix_len - 1, route_mode, 0, &min_block_id);
        _sys_duet2_ipuc_tcam_get_blockid(lchip, ip_ver, 0, route_mode, 0, &max_block_id);
    }

    ofb_cb.type = p_data->key_type;
    ofb_cb.user_data = p_data->info;

    if (p_data->opt_type != DO_DEL)
    {
        sys_usw_ofb_traverse(lchip, p_usw_ipuc_master[lchip]->share_ofb_type[route_mode], min_block_id, max_block_id, _sys_duet2_ipuc_tcam_push_down_ad, &ofb_cb);
        if (p_data->key_type == SYS_IPUC_TCAM_FLAG_ALPM)
        {
            p_calpm_prefix->is_pushed = 1;
        }
    }
    else
    {
        sys_ipuc_tcam_data_t tcam_data = {0};
        ctc_slistnode_t *cur_node, *next_node;
        sys_calpm_prefix_node_t *prefix_node = NULL;

        ofb_cb.priv_data = (void*)ctc_slist_new();
        CTC_PTR_VALID_CHECK(ofb_cb.priv_data);
        sys_usw_ofb_traverse(lchip, p_usw_ipuc_master[lchip]->share_ofb_type[route_mode], min_block_id, max_block_id, _sys_duet2_ipuc_tcam_push_up_ad, &ofb_cb);

        CTC_SLIST_LOOP_DEL((ctc_slist_t*)ofb_cb.priv_data, cur_node, next_node)
        {
            prefix_node = _ctc_container_of(cur_node, sys_calpm_prefix_node_t, head);
            if (!prefix_node)
            {
                break;
            }

            tcam_data.opt_type = DO_ADD;
            tcam_data.key_type = SYS_IPUC_TCAM_FLAG_ALPM;
            tcam_data.info = (void*)prefix_node->p_calpm_prefix;
            sys_duet2_ipuc_tcam_update_ad(lchip, &tcam_data);

            ctc_slist_delete_node((ctc_slist_t*)ofb_cb.priv_data, &prefix_node->head);
            mem_free(prefix_node);
        }

        ctc_slist_free((ctc_slist_t*)ofb_cb.priv_data);
    }

    return CTC_E_NONE;
}

int32
sys_duet2_ipuc_tcam_show_key(uint8 lchip, sys_ipuc_tcam_data_t *p_data)
{
    sys_ipuc_info_t  *p_ipuc_info    = NULL;
    sys_calpm_prefix_t  *p_calpm_prefix  = NULL;
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;
    sys_ipuc_tbl_type_t tbl_type = 0;
    uint32 key_id = 0;
    uint32 ad_id = 0;
    uint32 key_index = 0;
    uint32 tcam_ad_index = 0;
    uint8 masklen = 0;
    uint8 ip_ver = CTC_IP_VER_4;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_PTR_VALID_CHECK(p_data->info);

    if (p_data->key_type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        p_ipuc_info = (sys_ipuc_info_t*)p_data->info;

        ip_ver = SYS_IPUC_VER(p_ipuc_info);
        masklen = p_ipuc_info->masklen;
        key_index = p_ipuc_info->key_index;
        if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE))
        {
            route_mode = SYS_PUBLIC_MODE;
        }
    }
    else if (p_data->key_type == SYS_IPUC_TCAM_FLAG_ALPM)
    {
        p_calpm_prefix = (sys_calpm_prefix_t*)p_data->info;

        ip_ver = p_calpm_prefix->ip_ver;
        masklen = p_data->masklen;
        key_index = p_calpm_prefix->key_index[masklen > p_calpm_prefix->prefix_len];
    }
    tcam_ad_index = key_index;

    if (ip_ver == CTC_IP_VER_4)
    {
        tbl_type = (route_mode == SYS_PRIVATE_MODE) ? SYS_TBL_TCAM : SYS_TBL_TCAM_PUB;
    }
    else
    {
        if (route_mode == SYS_PRIVATE_MODE)
        {
            tbl_type = IS_SHORT_KEY(route_mode, key_index) ? SYS_TBL_TCAM_SHORT : SYS_TBL_TCAM;
        }
        else
        {
            tbl_type = IS_SHORT_KEY(route_mode, key_index) ? SYS_TBL_TCAM_PUB_SHORT : SYS_TBL_TCAM_PUB;
        }
        key_index = IS_SHORT_KEY(route_mode, key_index) ? (key_index/2) : (key_index/4);
    }

    sys_usw_get_tbl_id(lchip, ip_ver, 1, tbl_type, &key_id, &ad_id);

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-30s:%d\n", TABLE_NAME(lchip, key_id), key_index);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-30s:%d\n", TABLE_NAME(lchip, ad_id), tcam_ad_index);
    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_TCAM_SA_LOOKUP))
    {
        sys_usw_get_tbl_id(lchip, ip_ver, 0, tbl_type, &key_id, &ad_id);

        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-30s:%d\n", TABLE_NAME(lchip, key_id), key_index);
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-30s:%d\n", TABLE_NAME(lchip, ad_id), tcam_ad_index);
    }

    return CTC_E_NONE;
}

int32
sys_duet2_ipuc_tcam_show_status(uint8 lchip)
{
    uint8 ip_ver = 0;
    uint16 total_size = 0;
    uint16 total_entry = 0;
    uint16 total_usage = 0;
    sys_ipuc_route_mode_t route_mode;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s\n", "Tcam", " ");

    for (route_mode = SYS_PRIVATE_MODE; route_mode < MAX_ROUTE_MODE; route_mode++)
    {
        if (((route_mode == SYS_PRIVATE_MODE) && !CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_LOOKUP)) ||
            ((route_mode == SYS_PUBLIC_MODE) && !CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_4], SYS_TCAM_PUB_LOOKUP)))
        {
            continue;
        }

        total_size = 0;
        total_entry = 0;
        total_usage = 0;
        for (ip_ver = CTC_IP_VER_4; ip_ver < MAX_CTC_IP_VER; ip_ver++)
        {
            if (route_mode == SYS_PRIVATE_MODE)
            {
                if (ip_ver == CTC_IP_VER_4)
                {
                    total_size += p_usw_ipuc_master[lchip]->max_entry_num[ip_ver][SYS_TBL_TCAM];
                    total_entry += p_usw_ipuc_master[lchip]->route_stats[ip_ver][SYS_IPUC_TCAM];
                    total_usage += p_usw_ipuc_master[lchip]->route_stats[ip_ver][SYS_IPUC_TCAM];
                }
                else
                {
                    total_size += (p_usw_ipuc_master[lchip]->max_entry_num[ip_ver][SYS_TBL_TCAM_SHORT] +
                                                p_usw_ipuc_master[lchip]->max_entry_num[ip_ver][SYS_TBL_TCAM]);
                    total_entry += (p_usw_ipuc_master[lchip]->route_stats[ip_ver][SYS_IPUC_TCAM_SHORT] +
                                                p_usw_ipuc_master[lchip]->route_stats[ip_ver][SYS_IPUC_TCAM]);
                    total_usage += (p_usw_ipuc_master[lchip]->route_stats[ip_ver][SYS_IPUC_TCAM_SHORT] * 2 +
                                                p_usw_ipuc_master[lchip]->route_stats[ip_ver][SYS_IPUC_TCAM] * 4);
                }
            }
            else
            {
                if (ip_ver == CTC_IP_VER_4)
                {
                    total_size += p_usw_ipuc_master[lchip]->max_entry_num[ip_ver][SYS_TBL_TCAM_PUB];
                    total_entry += p_usw_ipuc_master[lchip]->route_stats[ip_ver][SYS_IPUC_TCAM_PUB];
                    total_usage += p_usw_ipuc_master[lchip]->route_stats[ip_ver][SYS_IPUC_TCAM_PUB];
                }
                else
                {
                    total_size += (p_usw_ipuc_master[lchip]->max_entry_num[ip_ver][SYS_TBL_TCAM_PUB_SHORT] +
                                                p_usw_ipuc_master[lchip]->max_entry_num[ip_ver][SYS_TBL_TCAM_PUB]);
                    total_entry += (p_usw_ipuc_master[lchip]->route_stats[ip_ver][SYS_IPUC_TCAM_PUB_SHORT] +
                                                p_usw_ipuc_master[lchip]->route_stats[ip_ver][SYS_IPUC_TCAM_PUB]);
                    total_usage += (p_usw_ipuc_master[lchip]->route_stats[ip_ver][SYS_IPUC_TCAM_PUB_SHORT] * 2 +
                                                p_usw_ipuc_master[lchip]->route_stats[ip_ver][SYS_IPUC_TCAM_PUB] * 4);
                }
            }
        }

        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s", (route_mode == SYS_PRIVATE_MODE) ? "--Private" : "--Public", " ");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15d%-4s",  total_size, " ");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15d%-4s%-15d\n", total_entry, " ", total_usage);

        for (ip_ver = CTC_IP_VER_4; ip_ver < MAX_CTC_IP_VER; ip_ver++)
        {
            if (route_mode == SYS_PRIVATE_MODE)
            {
                if (ip_ver == CTC_IP_VER_4)
                {
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s", "  IPv4", " ");
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s",  "-", " ");
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15d%-4s%-15d\n", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_TCAM], " ",
                        p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_TCAM]);
                }
                else
                {
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s", "  IPv6", " ");
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s",  "-", " ");
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15d%-4s%-15d\n", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_TCAM], " ",
                        p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_TCAM] * 4);

                    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_6], SYS_TCAM_SHORT_LOOKUP))
                    {
                        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s", "  IPv6-Short", " ");
                        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s", "-", " ");
                        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15d%-4s%-15d\n", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_TCAM_SHORT], " ",
                            p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_TCAM_SHORT] * 2);
                    }
                }
            }
            else
            {
                if (ip_ver == CTC_IP_VER_4)
                {
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s", "  IPv4", " ");
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s",  "-", " ");
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15d%-4s%-15d\n", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_TCAM_PUB], " ",
                        p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_4][SYS_IPUC_TCAM_PUB]);
                }
                else
                {
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s", "  IPv6", " ");
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s", "-", " ");
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15d%-4s%-15d\n", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_TCAM_PUB], " ",
                        p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_TCAM_PUB] * 4);

                    if (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_6], SYS_TCAM_SHORT_LOOKUP))
                    {
                        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s", "  IPv6-Short", " ");
                        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s", "-", " ");
                        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15d%-4s%-15d\n", p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_TCAM_PUB_SHORT], " ",
                            p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_TCAM_PUB_SHORT] * 2);
                    }
                }
            }
        }
   }

    return CTC_E_NONE;
}

int32
_sys_duet2_ipuc_tcam_init_private(uint8 lchip, void* ofb_cb)
{
    uint8 step = 0;
    uint8 block_id = 0;
    uint8 ofb_type = 0;
    uint8 calpm_block_id = 0;
    uint16 calpm_block_size = 0;
    uint8 tmp_block_max = 0;
    uint8 block_max[MAX_CTC_IP_VER] = {0};
    uint32 total_size = 0;
    uint32 table_size = 0;
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;
    sys_usw_ofb_param_t ofb_param = {0};
    ofb_param.ofb_cb = ofb_cb;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM])
    {/* don't enable tcam ipv6 short key*/
        _sys_duet2_ipuc_tcam_get_max_block_num(lchip, CTC_IP_VER_6, route_mode, &block_max[CTC_IP_VER_6]);
        table_size += p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM];
        p_usw_ipuc_master[lchip]->tcam_share_mode[route_mode] = SYS_SHARE_IPV6;
    }

    if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT])
    {/* enable tcam ipv6 short key*/
        if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM])
        {
            p_usw_ipuc_master[lchip]->tcam_share_mode[route_mode] = SYS_SHARE_ALL;
            p_usw_ipuc_master[lchip]->short_key_boundary[route_mode] = table_size;
        }

        table_size += p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT];
    }

    if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM])
    {/* don't enable tcam ipv6 */
        _sys_duet2_ipuc_tcam_get_max_block_num(lchip, CTC_IP_VER_4, route_mode, &block_max[CTC_IP_VER_4]);
        table_size += p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM];
    }

    CTC_ERROR_RETURN(sys_usw_ofb_init(lchip, block_max[CTC_IP_VER_4] + block_max[CTC_IP_VER_6], table_size, &ofb_type));
    p_usw_ipuc_master[lchip]->share_ofb_type[route_mode] = ofb_type;

    if (p_usw_ipuc_master[lchip]->tcam_share_mode[route_mode] == SYS_SHARE_IPV6)
    {
        ofb_param.max_limit_offset = 0;
        ofb_param.multiple = 4;
        table_size = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM];
        step = ofb_param.multiple;
        calpm_block_size = table_size - (block_max[CTC_IP_VER_6] - 1) * step;
        calpm_block_size = calpm_block_size ?  (calpm_block_size/ofb_param.multiple) * ofb_param.multiple : step;
        _sys_duet2_ipuc_tcam_get_calpm_blockid(lchip, CTC_IP_VER_6, &calpm_block_id);
        for (block_id = 0; block_id < block_max[CTC_IP_VER_6]; block_id++)
        {
            ofb_param.size = (block_id == calpm_block_id) ? calpm_block_size : step;

            if (block_id == tmp_block_max - 1)
            {
                ofb_param.size = table_size - (tmp_block_max-1) * step;
            }

            if ((total_size + ofb_param.size) > table_size)
            {
                ofb_param.size = (total_size < table_size) ? (table_size - total_size) : 0;
            }

            CTC_ERROR_RETURN(sys_usw_ofb_init_offset(lchip, ofb_type, block_id, &ofb_param));
            total_size += ofb_param.size;
        }
    }
    else if (p_usw_ipuc_master[lchip]->tcam_share_mode[route_mode] == SYS_SHARE_ALL)
    {
        ofb_param.multiple = 4;
        tmp_block_max = 64 + p_usw_ipuc_master[lchip]->host_lpm_mode;
        table_size = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM];
        step = table_size/64;
        step = step ? (step/ofb_param.multiple)*ofb_param.multiple : ofb_param.multiple;
        ofb_param.size = step;
        ofb_param.max_limit_offset = table_size - 1;
        for (block_id = 0; block_id < tmp_block_max; block_id++)
        {
            if (block_id == tmp_block_max - 1)
            {
                ofb_param.size = table_size - (tmp_block_max-1) * step;
            }

            if ((total_size + ofb_param.size) > table_size)
            {
                ofb_param.size = (total_size < table_size) ? (table_size - total_size) : 0;
            }

            CTC_ERROR_RETURN(sys_usw_ofb_init_offset(lchip, ofb_type, block_id, &ofb_param));
            total_size += ofb_param.size;
        }

        total_size = 0;
        ofb_param.max_limit_offset = 0;
        ofb_param.multiple = 2;
        tmp_block_max = block_max[CTC_IP_VER_6] - (64 + p_usw_ipuc_master[lchip]->host_lpm_mode);
        table_size = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT];
        step = ofb_param.multiple;
        calpm_block_size = table_size - (tmp_block_max - 1) * step;
        calpm_block_size = calpm_block_size ?  (calpm_block_size/ofb_param.multiple) * ofb_param.multiple : step;
        _sys_duet2_ipuc_tcam_get_calpm_blockid(lchip, CTC_IP_VER_6, &calpm_block_id);
        for (; block_id < block_max[CTC_IP_VER_6]; block_id++)
        {
            ofb_param.size = (block_id == calpm_block_id) ? calpm_block_size : step;

            if (block_id == block_max[CTC_IP_VER_6] - 1)
            {
                ofb_param.size = table_size - (tmp_block_max-1) * step;
            }

            if ((total_size + ofb_param.size) > table_size)
            {
                ofb_param.size = (total_size < table_size) ? (table_size - total_size) : 0;
            }

            CTC_ERROR_RETURN(sys_usw_ofb_init_offset(lchip, ofb_type, block_id, &ofb_param));
            total_size += ofb_param.size;
        }
    }

    total_size = 0;
    step = 4;
    ofb_param.max_limit_offset = 0;
    ofb_param.multiple = 1;
    table_size = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM];
    calpm_block_size = table_size - (block_max[CTC_IP_VER_4] - 1) * step;
    calpm_block_size = calpm_block_size ?  (calpm_block_size/ofb_param.multiple) * ofb_param.multiple : step;
    _sys_duet2_ipuc_tcam_get_calpm_blockid(lchip, CTC_IP_VER_4, &calpm_block_id);
    for (; block_id < (block_max[CTC_IP_VER_4] + block_max[CTC_IP_VER_6]); block_id++)
    {
        ofb_param.size = (block_id == calpm_block_id) ? calpm_block_size : step;

        if (block_id == block_max[CTC_IP_VER_4] + block_max[CTC_IP_VER_6] - 1)
        {
            ofb_param.size = table_size - (tmp_block_max-1) * step;
        }

        if ((total_size + ofb_param.size) > table_size)
        {
            ofb_param.size = (total_size < table_size) ? (table_size - total_size) : 0;
        }

        CTC_ERROR_RETURN(sys_usw_ofb_init_offset(lchip, ofb_type, block_id, &ofb_param));
        total_size += ofb_param.size;
    }

    return CTC_E_NONE;
}

int32
sys_duet2_ipuc_tcam_init_public(uint8 lchip, void* ofb_cb)
{
    uint8 step = 0;
    uint8 block_id = 0;
    uint8 ofb_type = 0;
    uint8 tmp_block_max = 0;
    uint8 block_max[MAX_CTC_IP_VER] = {0};
    uint32 total_size = 0;
    uint32 table_size = 0;
    sys_ipuc_route_mode_t route_mode = SYS_PUBLIC_MODE;
    sys_usw_ofb_param_t ofb_param = {0};
    ofb_param.ofb_cb = ofb_cb;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_PUB])
    {/* don't enable tcam ipv6 short key*/
        _sys_duet2_ipuc_tcam_get_max_block_num(lchip, CTC_IP_VER_6, route_mode, &block_max[CTC_IP_VER_6]);
        table_size += p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_PUB];
        p_usw_ipuc_master[lchip]->tcam_share_mode[route_mode] = SYS_SHARE_IPV6;
    }

    if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_PUB_SHORT])
    {/* enable tcam ipv6 short key*/
        if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_PUB])
        {
            p_usw_ipuc_master[lchip]->tcam_share_mode[route_mode] = SYS_SHARE_ALL;
            p_usw_ipuc_master[lchip]->short_key_boundary[route_mode] = table_size;
        }

        table_size += p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_PUB_SHORT];
    }

    if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM_PUB])
    {/* don't enable tcam ipv6 */
        _sys_duet2_ipuc_tcam_get_max_block_num(lchip, CTC_IP_VER_4, route_mode, &block_max[CTC_IP_VER_4]);
        table_size += p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM_PUB];
    }

    CTC_ERROR_RETURN(sys_usw_ofb_init(lchip, block_max[CTC_IP_VER_4] + block_max[CTC_IP_VER_6], table_size, &ofb_type));
    p_usw_ipuc_master[lchip]->share_ofb_type[route_mode] = ofb_type;

    if (p_usw_ipuc_master[lchip]->tcam_share_mode[route_mode] == SYS_SHARE_IPV6)
    {
        ofb_param.max_limit_offset = 0;
        ofb_param.multiple = 4;
        table_size = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_PUB];
        step = table_size/128;
        step = step ? (step/ofb_param.multiple) * ofb_param.multiple : ofb_param.multiple;
        ofb_param.size = step;
        for (block_id = 0; block_id < block_max[CTC_IP_VER_6]; block_id++)
        {
            if (block_id == block_max[CTC_IP_VER_6] - 1)
            {
                ofb_param.size = table_size - total_size;
            }

            if ((total_size + ofb_param.size) > table_size)
            {
                ofb_param.size = (total_size < table_size) ? (table_size - total_size) : 0;
            }

            CTC_ERROR_RETURN(sys_usw_ofb_init_offset(lchip, ofb_type, block_id, &ofb_param));
            total_size += ofb_param.size;
        }
    }
    else if (p_usw_ipuc_master[lchip]->tcam_share_mode[route_mode] == SYS_SHARE_ALL)
    {
        ofb_param.multiple = 4;
        tmp_block_max = 64 + p_usw_ipuc_master[lchip]->host_lpm_mode;
        table_size = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_PUB];
        step = table_size/64;
        step = step ? (step/ofb_param.multiple) * ofb_param.multiple : ofb_param.multiple;
        ofb_param.max_limit_offset = table_size - 1;
        ofb_param.size = step;
        for (block_id = 0; block_id < tmp_block_max; block_id++)
        {
            if (block_id == tmp_block_max - 1)
            {
                ofb_param.size = table_size - total_size;
            }

            if ((total_size + ofb_param.size) > table_size)
            {
                ofb_param.size = (total_size < table_size) ? (table_size - total_size) : 0;
            }

            CTC_ERROR_RETURN(sys_usw_ofb_init_offset(lchip, ofb_type, block_id, &ofb_param));
            total_size += ofb_param.size;
        }

        total_size = 0;
        ofb_param.max_limit_offset = 0;
        ofb_param.multiple = 2;
        tmp_block_max = block_max[CTC_IP_VER_6] - (64 + p_usw_ipuc_master[lchip]->host_lpm_mode);
        table_size = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_PUB_SHORT];
        step = table_size/64;
        step = step ? (step/ofb_param.multiple) * ofb_param.multiple : ofb_param.multiple;
        ofb_param.size = step;
        for (; block_id < block_max[CTC_IP_VER_6]; block_id++)
        {
            if (block_id == block_max[CTC_IP_VER_6] -1)
            {
                ofb_param.size = table_size - total_size;
            }

            if ((total_size + ofb_param.size) > table_size)
            {
                ofb_param.size = (total_size < table_size) ? (table_size - total_size) : 0;
            }

            CTC_ERROR_RETURN(sys_usw_ofb_init_offset(lchip, ofb_type, block_id, &ofb_param));
            total_size += ofb_param.size;
        }
    }

    total_size = 0;
    ofb_param.max_limit_offset = 0;
    ofb_param.multiple = 1;
    table_size = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM_PUB];
    step = table_size/32;
    if (p_usw_ipuc_master[lchip]->tcam_share_mode[route_mode] == SYS_SHARE_IPV6)
    {
        step = step ? (step/4) * 4 : 4;
    }
    ofb_param.size = step;

    for (; block_id < (block_max[CTC_IP_VER_4] + block_max[CTC_IP_VER_6]); block_id++)
    {
        if (block_id == block_max[CTC_IP_VER_4] + block_max[CTC_IP_VER_6] - 1)
        {
            ofb_param.size = table_size - total_size;
        }

        if ((total_size + ofb_param.size) > table_size)
        {
            ofb_param.size = (total_size < table_size) ? (table_size - total_size) : 0;
        }

        CTC_ERROR_RETURN(sys_usw_ofb_init_offset(lchip, ofb_type, block_id, &ofb_param));
        total_size += ofb_param.size;
    }

    return CTC_E_NONE;
}

int32
sys_duet2_ipuc_tcam_init(uint8 lchip, sys_ipuc_route_mode_t route_mode, void* ofb_cb_fn)
{
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (route_mode == SYS_PRIVATE_MODE)
    {
        return _sys_duet2_ipuc_tcam_init_private(lchip, ofb_cb_fn);
    }
    else
    {
        return sys_duet2_ipuc_tcam_init_public(lchip, ofb_cb_fn);
    }

    return CTC_E_NONE;
}

int32
sys_duet2_ipuc_tcam_deinit(uint8 lchip, sys_ipuc_route_mode_t route_mode)
{
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (p_usw_ipuc_master[lchip]->share_ofb_type[route_mode])
    {
        sys_usw_ofb_deinit(lchip, p_usw_ipuc_master[lchip]->share_ofb_type[route_mode]);
    }

    return CTC_E_NONE;
}

#endif

