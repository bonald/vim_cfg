
#include "sys_usw_ipuc.h"
#include "sys_tsingma_ipuc_tcam.h"
#include "sys_tsingma_nalpm.h"
#include "sys_usw_ofb.h"
#include "sys_usw_opf.h"
#include "sys_usw_common.h"
#include "sys_usw_ftm.h"
#include "sys_usw_l3if.h"
#include "ctc_hmap.h"
#include "drv_api.h"
#include "usw/include/drv_common.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_nexthop_api.h"


struct sys_nalpm_match_tcam_idx_s
{
    sys_nalpm_route_info_t* p_new_route;
    sys_nalpm_route_info_t* p_AD_route;
    sys_nalpm_tcam_item_t * p_tcam_item;
    uint16 tcam_idx;
};
typedef struct sys_nalpm_match_tcam_idx_s sys_nalpm_match_tcam_idx_t;

struct sys_nalpm_route_and_lchip_s
{
    sys_nalpm_route_info_t * p_route;
    uint8 lchip;
    uint8 origin_lchip;
    uint32 nh_id;
};
typedef struct sys_nalpm_route_and_lchip_s sys_nalpm_route_and_lchip_t;

struct sys_nalpm_son2father_s
{
    trie_t * prefix_trie;
    trie_node_t * p_son_node;
    trie_node_t * p_father_node;
};
typedef struct sys_nalpm_son2father_s sys_nalpm_son2father_t;

struct sys_nalpm_install_route_trie_param_s
{
    uint8 lchip;
    uint8 *p_v4_cnt;
    sys_nalpm_tcam_item_t * p_install_tcam;
    sys_nalpm_route_info_t** p_new_route;
    ip_addr_t* ip_addr;
    uint8 new_mask_len;
};
typedef struct sys_nalpm_install_route_trie_param_s sys_nalpm_install_route_trie_param_t;

extern sys_ipuc_master_t* p_usw_ipuc_master[];
sys_nalpm_master_t* g_sys_nalpm_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};
extern int32 _sys_usw_ipuc_bind_nexthop(uint8 lchip, sys_ipuc_param_t* p_sys_param, uint8 is_bind);
int32
_sys_tsingma_nalpm_get_real_tcam_index(uint8 lchip, uint16 soft_tcam_index, uint8 *real_lchip, uint16* real_tcam_index)
{
    uint8 i = 0;
    *real_lchip = 0xff;
    if(!p_usw_ipuc_master[lchip]->rchain_en)
    {
        *real_lchip = lchip;
        *real_tcam_index = soft_tcam_index;
    }
    for(i=0; i<=p_usw_ipuc_master[lchip]->rchain_tail; i++)
    {
        if(soft_tcam_index<p_usw_ipuc_master[i]->rchain_boundary && (soft_tcam_index >= (i ? p_usw_ipuc_master[i-1]->rchain_boundary : 0)))
        {
            *real_lchip = i;
            *real_tcam_index = soft_tcam_index - (i ? p_usw_ipuc_master[i-1]->rchain_boundary : 0);
            break;
        }
    }
    return CTC_E_NONE;
}

int32
_sys_tsingma_nalpm_get_route_num_per_tcam(uint8 lchip, sys_nalpm_tcam_item_t* p_tcam_item, uint8* cnt)
{
    uint8 i = 0, j = 0;
    uint8 snake_num = 0;
    _sys_tsingma_nalpm_get_snake_num(lchip, p_tcam_item->p_prefix_info->ip_ver, &snake_num);
    for (i = 0; i < snake_num; i++)
    {
        for (j = 0; j < ROUTE_NUM_PER_SNAKE_V4; j++)
        {
            if (NULL != p_tcam_item->p_ipuc_info_array[i][j])
            {
                (*cnt)++;
            }
        }
    }
    return CTC_E_NONE;
}


void
_sys_tsingma_nalpm_get_snake_num(uint8 lchip, uint8 ip_ver, uint8* p_snake_num)
{
    if (DRV_IS_TSINGMA(lchip))
    {
        *p_snake_num = 2;
    }
    else
    {
        if (CTC_IP_VER_6 == ip_ver && !g_sys_nalpm_master[lchip]->ipsa_enable)
        {
            *p_snake_num = p_usw_ipuc_master[lchip]->snake_per_group * 2;
        }
        else
        {
            *p_snake_num = p_usw_ipuc_master[lchip]->snake_per_group;
        }
    }
}

extern int32 _sys_tsingma_ipuc_tcam_get_blockid(uint8 lchip,
                                uint8 ip_ver,
                                uint8 masklen,
                                sys_ipuc_route_mode_t route_mode,
                                uint8 hash_conflict,
                                uint8* block_id);


int32
_sys_tsingma_nalpm_search_father_pfx(trie_t * prefix_trie, trie_node_t * p_son_node, trie_node_t ** p_father_node);
#if 0
int32
_sys_tsingma_nalpm_check_tcam_resource(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param)
{
    uint8 block_id = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_ERROR_RETURN(_sys_tsingma_ipuc_tcam_get_blockid(lchip,  p_ipuc_param->ip_ver, p_ipuc_param->masklen, SYS_PRIVATE_MODE,  0,&block_id));

    return sys_usw_ofb_check_resource(lchip, p_usw_ipuc_master[lchip]->share_ofb_type[SYS_PRIVATE_MODE],  block_id);
}
#endif

int32
_sys_tsingma_nalpm_tcam_idx_alloc(uint8 lchip, ctc_ipuc_param_t*  p_ipuc_param, sys_nalpm_tcam_item_t* p_tcam_item)
{
    uint8 ip_ver = p_tcam_item->p_prefix_info->ip_ver;
    sys_ipuc_tcam_data_t tcam_data = {0};
    tcam_data.key_type = SYS_IPUC_TCAM_FLAG_ALPM;
    tcam_data.masklen = p_tcam_item->p_prefix_info->tcam_masklen;
    tcam_data.ipuc_param = p_ipuc_param;
    tcam_data.info = (void*)(p_tcam_item);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if(p_usw_ipuc_master[lchip]->lpm_tcam_spec[ip_ver] != 0
     && p_usw_ipuc_master[lchip]->lpm_tcam_used_cnt[ip_ver] >= p_usw_ipuc_master[lchip]->lpm_tcam_spec[ip_ver])
    {
        return CTC_E_NO_RESOURCE;
    }
    if(p_ipuc_param->masklen > 64 &&
       p_usw_ipuc_master[lchip]->route_stats[CTC_IP_VER_6][SYS_IPUC_TCAM] >= p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM])
    {
        return CTC_E_NO_RESOURCE;
    }

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        tcam_data.key_index = p_tcam_item->tcam_idx;
    }
    CTC_ERROR_RETURN(sys_usw_ipuc_alloc_tcam_key_index(lchip, &tcam_data));
    p_tcam_item->tcam_idx= tcam_data.key_index;
    if(ip_ver && IS_SHORT_KEY(0, p_tcam_item->tcam_idx))
    {
        p_usw_ipuc_master[lchip]->lpm_tcam_used_cnt[ip_ver] += 2;
    }
    else if(ip_ver)
    {
        p_usw_ipuc_master[lchip]->lpm_tcam_used_cnt[ip_ver] += 4;
    }
    else
    {
        p_usw_ipuc_master[lchip]->lpm_tcam_used_cnt[ip_ver]++;
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "%s(), tcam_idx=[%u]\n",__FUNCTION__, p_tcam_item->tcam_idx);
    return CTC_E_NONE;
}

int32
_sys_tsingma_nalpm_tcam_idx_free(uint8 lchip, ctc_ipuc_param_t*  p_ipuc_param, sys_nalpm_tcam_item_t* p_tcam_item)
{
    uint8 ip_ver = p_tcam_item->p_prefix_info->ip_ver;
    sys_ipuc_tcam_data_t tcam_data = {0};

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "%s(), tcam_idx=[%u]\n",__FUNCTION__, p_tcam_item->tcam_idx);
    tcam_data.key_type = SYS_IPUC_TCAM_FLAG_ALPM;
    tcam_data.masklen = p_tcam_item->p_prefix_info->tcam_masklen;
    tcam_data.ipuc_param = p_ipuc_param;
    tcam_data.key_index = p_tcam_item->tcam_idx;
    tcam_data.info = (void*)(p_tcam_item);
    CTC_ERROR_RETURN(sys_usw_ipuc_free_tcam_key_index(lchip, &tcam_data));

    if(ip_ver && IS_SHORT_KEY(0, p_tcam_item->tcam_idx))
    {
        p_usw_ipuc_master[lchip]->lpm_tcam_used_cnt[ip_ver] -= 2;
    }
    else if(ip_ver)
    {
        p_usw_ipuc_master[lchip]->lpm_tcam_used_cnt[ip_ver] -= 4;
    }
    else
    {
        p_usw_ipuc_master[lchip]->lpm_tcam_used_cnt[ip_ver]--;
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_nalpm_sram_idx_alloc(uint8 lchip, uint8  ip_ver, uint16* p_sram_idx)
{
    int32 ret = CTC_E_NONE;
    uint32 sram_idx = 0;
    uint32 block_size = 0;
    sys_usw_opf_t opf;
    CTC_PTR_VALID_CHECK(p_sram_idx);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    if(DRV_IS_TM2(lchip) &&  CTC_IP_VER_6 == ip_ver && !g_sys_nalpm_master[lchip]->ipsa_enable)
    {
        opf.multiple = 8;
        block_size = 8;
    }
    else
    {
        opf.multiple = 4;
        block_size = 4;
    }
    opf.pool_type = g_sys_nalpm_master[lchip]->opf_type_nalpm;

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, block_size, *p_sram_idx));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, block_size, &sram_idx));
    }
    *p_sram_idx = (uint16)sram_idx;

    return ret;

}

int32
_sys_tsingma_nalpm_sram_idx_free(uint8 lchip, uint16 sram_idx)
{
    sys_usw_opf_t opf;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_index = 0;
    opf.pool_type = g_sys_nalpm_master[lchip]->opf_type_nalpm;

    return sys_usw_opf_free_offset( lchip, &opf, 4, sram_idx);

}
STATIC int32
_sys_nalpm_convert_to_msb(uint32* ipaddr, uint32 masklen, uint32* hw_ipaddr, uint8 ip_ver)
{
    if (0 == masklen)
    {
        *hw_ipaddr = 0;
    }
    else
    {
        if ( CTC_IP_VER_4 == ip_ver )
        {
            *hw_ipaddr = (*ipaddr << (32 - masklen));
        }
        else
        {
            if (masklen <= 32)
            {
                *(hw_ipaddr + 0) = *(ipaddr + 3) << (32 - masklen);
            }
            else if(32 < masklen &&  masklen < 64)
            {
                *(hw_ipaddr + 0) = (*(ipaddr + 2) << (64 - masklen)) + (*(ipaddr + 3) >> (32 - (64 - masklen)));
                *(hw_ipaddr + 1) = (*(ipaddr + 3) << (64 - masklen));
            }
            else if(masklen == 64)
            {
                *(hw_ipaddr + 0) = *(ipaddr + 2);
                *(hw_ipaddr + 1) = *(ipaddr + 3);
            }
            else if(64 < masklen &&  masklen < 96)
            {
                *(hw_ipaddr + 0) = (*(ipaddr + 1) << (96 - masklen)) +  (*(ipaddr + 2) >> (32 - (96 - masklen)));
                *(hw_ipaddr + 1) = (*(ipaddr + 2) << (96 - masklen)) +  (*(ipaddr + 3) >> (32 - (96 - masklen)));
                *(hw_ipaddr + 2) = (*(ipaddr + 3) << (96 - masklen));
            }
            else if(masklen == 96)
            {
                *(hw_ipaddr + 0) = *(ipaddr + 1);
                *(hw_ipaddr + 1) = *(ipaddr + 2);
                *(hw_ipaddr + 2) = *(ipaddr + 3);
            }
            else if(96 < masklen &&  masklen < 128)
            {
                *(hw_ipaddr + 0) = (*(ipaddr + 0) << (128 - masklen)) + (*(ipaddr + 1) >> (32 - (128 - masklen)));
                *(hw_ipaddr + 1) = (*(ipaddr + 1) << (128 - masklen)) + (*(ipaddr + 2) >> (32 - (128 - masklen)));
                *(hw_ipaddr + 2) = (*(ipaddr + 2) << (128 - masklen)) + (*(ipaddr + 3) >> (32 - (128 - masklen)));
                *(hw_ipaddr + 3) = (*(ipaddr + 3) << (128 - masklen));
            }
        }
    }



    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_nalpm_route_cmp(sys_nalpm_route_info_t* p_route_info_0,
                               sys_nalpm_route_info_t* p_route_info_1)
{
    if(!p_route_info_0 || !p_route_info_1)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_route_info_0->ip_ver != p_route_info_1->ip_ver)
    {
        return CTC_E_PARAM_CONFLICT;
    }

    if (p_route_info_0->vrf_id != p_route_info_1->vrf_id)
    {
        return CTC_E_PARAM_CONFLICT;
    }

    if (p_route_info_0->route_masklen != p_route_info_1->route_masklen)
    {
        return CTC_E_PARAM_CONFLICT;
    }

    if (0 != sal_memcmp(p_route_info_0->ip, p_route_info_1->ip,
                        (CTC_IP_VER_4 == p_route_info_0->ip_ver)? sizeof(ip_addr_t):sizeof(ipv6_addr_t)))
    {
        return CTC_E_PARAM_CONFLICT;
    }


    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_nalpm_route_match(uint8 lchip, sys_nalpm_route_info_t* p_pfx_info, sys_nalpm_route_info_t* p_ipuc_info_1)
{
    uint32 masklen = 0;
    uint32 mask_pfx = 0;
    uint32 hw_pfx[4] = {0};

    if(!p_pfx_info || !p_ipuc_info_1)
    {
        return CTC_E_INVALID_PARAM;
    }

    if(p_pfx_info->tcam_masklen < p_ipuc_info_1->route_masklen)
    {
        masklen = p_pfx_info->tcam_masklen;
    }
    else
    {
        masklen = p_ipuc_info_1->route_masklen;
    }

    if (p_pfx_info->ip_ver != p_ipuc_info_1->ip_ver)
    {
        return CTC_E_PARAM_CONFLICT;
    }
    if (p_pfx_info->vrf_id != p_ipuc_info_1->vrf_id)
    {
        return CTC_E_PARAM_CONFLICT;
    }

    _sys_nalpm_convert_to_msb(p_pfx_info->ip, p_pfx_info->tcam_masklen, hw_pfx, p_ipuc_info_1->ip_ver);

    if (CTC_IP_VER_4 == p_pfx_info->ip_ver)
    {
        mask_pfx = g_sys_nalpm_master[lchip]->len2pfx[masklen];
        if ((hw_pfx[0] & mask_pfx) != (p_ipuc_info_1->ip[0] & mask_pfx))
        {
            return CTC_E_PARAM_CONFLICT;
        }
    }
    else
    {
        mask_pfx = g_sys_nalpm_master[lchip]->len2pfx[masklen%32];
        if (sal_memcmp(hw_pfx, p_ipuc_info_1->ip,  (masklen/32)*sizeof(uint32))
            || ((hw_pfx[masklen/32] & mask_pfx) != (p_ipuc_info_1->ip[masklen/32] & mask_pfx)))
        {
            return CTC_E_PARAM_CONFLICT;
        }

    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_nalpm_update_ad(uint8 new_lchip, uint8 old_lchip, sys_nalpm_route_info_t* p_route_info, uint8 is_ad_route, uint32 nh_id, uint32* new_ad_index)
{
    sys_ipuc_param_t temp_sys_param;
    uint32 cmd = 0;
    uint32 old_ad_index = 0;
    sys_nh_info_dsnh_t nh_info;
    DsIpDa_m dsipda;
    uint32 tmp_nhid = 0;
    sys_ipuc_info_t tmp_info;

    sal_memset(&temp_sys_param, 0, sizeof(sys_ipuc_param_t));
    sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));
    _sys_usw_ipuc_read_ipda(old_lchip, p_route_info->ad_idx, &dsipda);
    temp_sys_param.param.ip_ver = p_route_info->ip_ver;
    temp_sys_param.param.masklen = p_route_info->route_masklen;
    temp_sys_param.param.vrf_id = p_route_info->vrf_id;
    if (p_route_info->ip_ver)
    {
        sal_memcpy(temp_sys_param.param.ip.ipv6, p_route_info->ip, IP_ADDR_SIZE(CTC_IP_VER_6));
    }
    else
    {
        temp_sys_param.param.ip.ipv4 = *(p_route_info->ip);
    }
    SYS_IP_ADDRESS_SORT2(temp_sys_param.param);
    if (nh_id)
    {
        tmp_nhid = nh_id;
        sal_memset(&tmp_info, 0, sizeof(sys_ipuc_info_t));
        temp_sys_param.info = &tmp_info;
    }
    else
    {
        CTC_ERROR_RETURN(_sys_usw_ipuc_db_lookup(0, &temp_sys_param));
        tmp_nhid = temp_sys_param.info->nh_id;
    }
    old_ad_index = temp_sys_param.info->ad_index;
    sys_usw_nh_get_nhinfo(new_lchip, tmp_nhid, &nh_info, 0);
    _sys_tsingma_nalpm_renew_dsipda(new_lchip, &dsipda, nh_info);
    _sys_usw_ipuc_add_ad_profile(new_lchip, &temp_sys_param, (void*)&dsipda, 0);
    if (!is_ad_route)
    {
        _sys_usw_ipuc_remove_ad_profile(old_lchip, p_route_info->ad_idx);
        p_route_info->ad_idx = temp_sys_param.info->ad_index;
    }
    cmd = DRV_IOW(DsIpDa_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(new_lchip, temp_sys_param.info->ad_index, cmd, &dsipda);
    if (is_ad_route)
    {
        *new_ad_index = temp_sys_param.info->ad_index;
        temp_sys_param.info->ad_index = old_ad_index;
    }

    return CTC_E_NONE;
}

int32
_sys_nalpm_write_sram(uint8 lchip, sys_nalpm_tcam_item_t* p_tcam_item, sys_nalpm_route_info_t* p_route_info, uint8 snake_idx, uint8 entry_idx, uint8 is_update)
{
    sys_nalpm_sram_entry_type_t type = p_tcam_item->sram_type[snake_idx];
    uint32 cmd = 0;
    uint32 DsNeoLpmMemorySnake[48/4] = {0};
    void *ptr = DsNeoLpmMemorySnake;
    uint32 ad_index = p_route_info->ad_idx;
    uint32 masklen = p_route_info->route_masklen;
    uint32* p_hw_ipaddr = 0;
    uint32 tbl_id = 0, tbl_id1 = 0;
    ipv6_addr_t ipv6_addr;
    uint16 real_tcam_index = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "p_tcam_item=[%p],    ip:[%x] ,    snake_idx=[%u]\n",p_tcam_item, p_route_info->ip[0], snake_idx);
    if(p_usw_ipuc_master[lchip]->rchain_en)
    {
        _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_tcam_item->tcam_idx, &lchip, &real_tcam_index);
    }
    if (CTC_IP_VER_4 == p_route_info->ip_ver)
    {
       SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO," pfx_IPv4[0x%x/%d]",p_route_info->ip[0], p_route_info->route_masklen);
    }
    else
    {
       SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO," pfx_IPv6[0x%x:0x%x:0x%x:0x%x/%d]",
       p_route_info->ip[3],
       p_route_info->ip[2],
       p_route_info->ip[1],
       p_route_info->ip[0],
       p_route_info->route_masklen);
    }
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"AD-idx:[0x%x]\n",p_route_info->ad_idx);
    switch (type)
    {
        case SYS_NALPM_SRAM_TYPE_V4_32:
            if(DRV_IS_TSINGMA(lchip))
            {
                tbl_id = snake_idx ? DsNeoLpmIpv4Bit32Snake1_t : DsNeoLpmIpv4Bit32Snake_t;
                tbl_id1 =  snake_idx ? DsNeoLpmIpv4Bit32Snake3_t : DsNeoLpmIpv4Bit32Snake2_t;
            }
            else
            {
                if(g_sys_nalpm_master[lchip]->ipsa_enable)
                {
                    tbl_id = DsNeoLpmIpv4Bit32Snake_t;
                    tbl_id1 = DsNeoLpmIpv4Bit32Snake1_t;
                }
                else
                {
                    tbl_id = snake_idx >= SYS_NALPM_MAX_SNAKE/2 ? DsNeoLpmIpv4Bit32Snake1_t : DsNeoLpmIpv4Bit32Snake_t;
                }
            }
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SRAM_HW_INDEX(lchip, p_tcam_item->sram_idx, snake_idx), cmd, ptr));
            p_hw_ipaddr = p_route_info->ip;
            switch (entry_idx)
            {
                case 0:
                    if(!is_update)
                    {
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_0_ipAddr_f, p_hw_ipaddr, ptr);
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_0_maskLen_f, &masklen, ptr);
                    }
                    DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_0_nexthop_f, &ad_index, ptr);

                    break;
                case 1:
                    if(!is_update)
                    {
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_1_ipAddr_f, p_hw_ipaddr, ptr);
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_1_maskLen_f, &masklen, ptr);
                    }
                    DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_1_nexthop_f, &ad_index, ptr);
                    break;
                case 2:
                    if(!is_update)
                    {
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_2_ipAddr_f, p_hw_ipaddr, ptr);
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_2_maskLen_f, &masklen, ptr);
                    }
                    DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_2_nexthop_f, &ad_index, ptr);
                    break;
                case 3:
                    if(!is_update)
                    {
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_3_ipAddr_f, p_hw_ipaddr, ptr);
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_3_maskLen_f, &masklen, ptr);
                    }
                    DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_3_nexthop_f, &ad_index, ptr);

                    break;
                case 4:
                    if(!is_update)
                    {
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_4_ipAddr_f, p_hw_ipaddr, ptr);
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_4_maskLen_f, &masklen, ptr);
                    }
                    DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_4_nexthop_f, &ad_index, ptr);
                    break;
                case 5:
                    if(!is_update)
                    {
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_5_ipAddr_f, p_hw_ipaddr, ptr);
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_5_maskLen_f, &masklen, ptr);
                    }
                    DRV_IOW_FIELD(lchip, DsNeoLpmIpv4Bit32Snake_t, DsNeoLpmIpv4Bit32Snake_entry_5_nexthop_f, &ad_index, ptr);
                    break;

            }

            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SRAM_HW_INDEX(lchip, p_tcam_item->sram_idx, snake_idx), cmd, ptr));
            if(g_sys_nalpm_master[lchip]->ipsa_enable)
            {
                cmd = DRV_IOW(tbl_id1, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, SRAM_HW_INDEX(lchip, p_tcam_item->sram_idx, snake_idx), cmd, ptr));
            }
            break;
        case SYS_NALPM_SRAM_TYPE_V6_64:
            if(DRV_IS_TSINGMA(lchip))
            {
                tbl_id = snake_idx ? DsNeoLpmIpv6Bit64Snake1_t : DsNeoLpmIpv6Bit64Snake_t;
                tbl_id1 =  snake_idx ? DsNeoLpmIpv6Bit64Snake3_t : DsNeoLpmIpv6Bit64Snake2_t;
            }
            else
            {
                if(g_sys_nalpm_master[lchip]->ipsa_enable)
                {
                    tbl_id = DsNeoLpmIpv6Bit64Snake_t;
                    tbl_id1 = DsNeoLpmIpv6Bit64Snake1_t;
                }
                else
                {
                    tbl_id = snake_idx >= SYS_NALPM_MAX_SNAKE/2 ? DsNeoLpmIpv6Bit64Snake1_t : DsNeoLpmIpv6Bit64Snake_t;
                }
            }
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SRAM_HW_INDEX(lchip, p_tcam_item->sram_idx, snake_idx), cmd, ptr));
            sal_memset(ipv6_addr, 0, sizeof(ipv6_addr_t));
            ipv6_addr[0] = p_route_info->ip[1];
            ipv6_addr[1] = p_route_info->ip[0];
            switch (entry_idx)
            {
                case 0:
                    if(!is_update)
                    {
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv6Bit64Snake_t, DsNeoLpmIpv6Bit64Snake_entry_0_ipAddr_f, ipv6_addr, ptr);
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv6Bit64Snake_t, DsNeoLpmIpv6Bit64Snake_entry_0_maskLen_f, &masklen, ptr);
                    }
                    DRV_IOW_FIELD(lchip, DsNeoLpmIpv6Bit64Snake_t, DsNeoLpmIpv6Bit64Snake_entry_0_nexthop_f, &ad_index, ptr);
                    break;
                case 1:
                    if(!is_update)
                    {
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv6Bit64Snake_t, DsNeoLpmIpv6Bit64Snake_entry_1_ipAddr_f, ipv6_addr, ptr);
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv6Bit64Snake_t, DsNeoLpmIpv6Bit64Snake_entry_1_maskLen_f, &masklen, ptr);
                    }
                    DRV_IOW_FIELD(lchip, DsNeoLpmIpv6Bit64Snake_t, DsNeoLpmIpv6Bit64Snake_entry_1_nexthop_f, &ad_index, ptr);
                    break;
                case 2:
                    if(!is_update)
                    {
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv6Bit64Snake_t, DsNeoLpmIpv6Bit64Snake_entry_2_ipAddr_f, ipv6_addr, ptr);
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv6Bit64Snake_t, DsNeoLpmIpv6Bit64Snake_entry_2_maskLen_f, &masklen, ptr);
                    }
                    DRV_IOW_FIELD(lchip, DsNeoLpmIpv6Bit64Snake_t, DsNeoLpmIpv6Bit64Snake_entry_2_nexthop_f, &ad_index, ptr);
                    break;

            }
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SRAM_HW_INDEX(lchip, p_tcam_item->sram_idx, snake_idx), cmd, ptr));
            if(g_sys_nalpm_master[lchip]->ipsa_enable)
            {
                cmd = DRV_IOW(tbl_id1, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, SRAM_HW_INDEX(lchip, p_tcam_item->sram_idx, snake_idx), cmd, ptr));
            }
            break;
        case SYS_NALPM_SRAM_TYPE_V6_128:
            if(DRV_IS_TSINGMA(lchip))
            {
                tbl_id = snake_idx ? DsNeoLpmIpv6Bit128Snake1_t : DsNeoLpmIpv6Bit128Snake_t;
                tbl_id1 =  snake_idx ? DsNeoLpmIpv6Bit128Snake3_t : DsNeoLpmIpv6Bit128Snake2_t;
            }
            else
            {
                if(g_sys_nalpm_master[lchip]->ipsa_enable)
                {
                    tbl_id = DsNeoLpmIpv6Bit128Snake_t;
                    tbl_id1 = DsNeoLpmIpv6Bit128Snake1_t;
                }
                else
                {
                    tbl_id = snake_idx >= SYS_NALPM_MAX_SNAKE/2 ? DsNeoLpmIpv6Bit128Snake1_t : DsNeoLpmIpv6Bit128Snake_t;
                }
            }
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SRAM_HW_INDEX(lchip, p_tcam_item->sram_idx, snake_idx), cmd, ptr));
            sal_memset(ipv6_addr, 0, sizeof(ipv6_addr_t));
            ipv6_addr[0] = p_route_info->ip[3];
            ipv6_addr[1] = p_route_info->ip[2];
            ipv6_addr[2] = p_route_info->ip[1];
            ipv6_addr[3] = p_route_info->ip[0];
            switch (entry_idx)
            {
                case 0:
                    if(!is_update)
                    {
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv6Bit128Snake_t, DsNeoLpmIpv6Bit128Snake_entry_0_ipAddr_f, ipv6_addr, ptr);
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv6Bit128Snake_t, DsNeoLpmIpv6Bit128Snake_entry_0_maskLen_f, &masklen, ptr);
                    }
                    DRV_IOW_FIELD(lchip, DsNeoLpmIpv6Bit128Snake_t, DsNeoLpmIpv6Bit128Snake_entry_0_nexthop_f, &ad_index, ptr);
                    break;
                case 1:
                    if(!is_update)
                    {
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv6Bit128Snake_t, DsNeoLpmIpv6Bit128Snake_entry_1_ipAddr_f, ipv6_addr, ptr);
                        DRV_IOW_FIELD(lchip, DsNeoLpmIpv6Bit128Snake_t, DsNeoLpmIpv6Bit128Snake_entry_1_maskLen_f, &masklen, ptr);
                    }
                    DRV_IOW_FIELD(lchip, DsNeoLpmIpv6Bit128Snake_t, DsNeoLpmIpv6Bit128Snake_entry_1_nexthop_f, &ad_index, ptr);
                    break;
            }
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, SRAM_HW_INDEX(lchip, p_tcam_item->sram_idx, snake_idx), cmd, ptr));
            if(g_sys_nalpm_master[lchip]->ipsa_enable)
            {
                cmd = DRV_IOW(tbl_id1, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, SRAM_HW_INDEX(lchip, p_tcam_item->sram_idx, snake_idx), cmd, ptr));
            }
            break;
       default:
            break;

    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "nalpm: write %s[%d],entry_idx[%d]\n", TABLE_NAME(lchip, tbl_id), SRAM_HW_INDEX(lchip, p_tcam_item->sram_idx, snake_idx), entry_idx);
    return CTC_E_NONE;

}


int32
_sys_nalpm_clear_sram(uint8 lchip, sys_nalpm_tcam_item_t* p_tcam_item, uint8 snake_idx, uint8 offset, uint8 clear_snake_type)
{
    sys_nalpm_route_info_t*   p_route_info_void = NULL;
    uint8 route_per_snake = 0;
    uint8 i = 0;
    int32 ret = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    p_route_info_void = mem_malloc(MEM_IPUC_MODULE, ROUTE_INFO_SIZE(p_tcam_item->p_prefix_info->ip_ver));
    if(!p_route_info_void)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_route_info_void, 0 , (ROUTE_INFO_SIZE(p_tcam_item->p_prefix_info->ip_ver)));
    p_route_info_void->ip_ver = p_tcam_item->p_prefix_info->ip_ver;
    p_route_info_void->route_masklen = 0;

    CTC_ERROR_GOTO(_sys_nalpm_write_sram(lchip, p_tcam_item, p_route_info_void, snake_idx, offset, 0 ), ret, error0);

    mem_free(p_route_info_void);

    if(CTC_IP_VER_4 == p_tcam_item->p_prefix_info->ip_ver)
    {
        route_per_snake = ROUTE_NUM_PER_SNAKE_V4;
    }
    else
    {
        route_per_snake = (SYS_NALPM_SRAM_TYPE_V6_64 == p_tcam_item->sram_type[snake_idx]) ? ROUTE_NUM_PER_SNAKE_V6_64 : ROUTE_NUM_PER_SNAKE_V6_128;
/*
        if (DRV_IS_TSINGMA(lchip))
        {
            route_per_snake = ROUTE_NUM_PER_SNAKE_V6_128;
        }
        else
        {
            route_per_snake = (SYS_NALPM_SRAM_TYPE_V6_64 == p_tcam_item->sram_type[snake_idx]) ? ROUTE_NUM_PER_SNAKE_V6_64 : ROUTE_NUM_PER_SNAKE_V6_128;
        }
*/
    }
    for(i = 0; i < route_per_snake; i++)
    {
        if(NULL != p_tcam_item->p_ipuc_info_array[snake_idx][i])
        {
            break;
        }
    }
    if(i == route_per_snake && clear_snake_type)
    {
        p_tcam_item->sram_type[snake_idx] = SYS_NALPM_SRAM_TYPE_VOID;
    }

    return CTC_E_NONE;

error0:
    mem_free(p_route_info_void);
    return ret;
}

int32
_sys_tsingma_nalpm_route_lkp(uint8 lchip, sys_nalpm_route_info_t* p_route_info, sys_nalpm_route_store_info_t* p_lkp_rlt)
{
    int32 ret = CTC_E_NONE;
    uint8 i = 0, j = 0, void_i = 0, void_j = 0;
    sys_nalpm_prefix_trie_payload_t * p_prefix_trie_payload = NULL;
    trie_node_t* p_node = NULL;
    uint8 snake_num = 0;
    uint8 snake_route_num = 0;
    uint32 * p_trie_key = NULL;
    uint8 find_void_position = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if(NULL == g_sys_nalpm_master[lchip]->prefix_trie[p_route_info->ip_ver][p_route_info->vrf_id])
    {
        return CTC_E_NOT_EXIST;
    }

    p_trie_key = mem_malloc(MEM_IPUC_MODULE, IP_ADDR_SIZE(p_route_info->ip_ver));
    if(!p_trie_key)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_trie_key, 0 ,(IP_ADDR_SIZE(p_route_info->ip_ver)));
    sal_memcpy(p_trie_key, p_route_info->ip, (IP_ADDR_SIZE(p_route_info->ip_ver)));

    CTC_ERROR_RETURN(sys_nalpm_trie_find_lpm (g_sys_nalpm_master[lchip]->prefix_trie[p_route_info->ip_ver][p_route_info->vrf_id], p_trie_key,
                                            p_route_info->route_masklen, &p_node, 1));
    if(p_node == NULL)
    {
         ret = CTC_E_NOT_EXIST;
         goto END;
    }
    p_prefix_trie_payload =(sys_nalpm_prefix_trie_payload_t*)p_node;
    p_lkp_rlt->tcam_hit = TRUE;
    p_lkp_rlt->p_tcam_item = &p_prefix_trie_payload->tcam_item;
    p_lkp_rlt->sram_hit= FALSE;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "(%d)    prefix len[%u], tcam_idx=[%u] \n",
                    __LINE__, p_lkp_rlt->p_tcam_item->p_prefix_info->tcam_masklen, p_lkp_rlt->p_tcam_item->tcam_idx);

    _sys_tsingma_nalpm_get_snake_num(lchip, p_route_info->ip_ver, &snake_num);

    if(CTC_IP_VER_4 == p_route_info->ip_ver)
    {
        snake_route_num = ROUTE_NUM_PER_SNAKE_V4;
    }

    for(i = 0; i < snake_num; i++)
    {
        if(SYS_NALPM_SRAM_TYPE_VOID == p_prefix_trie_payload->tcam_item.sram_type[i])
        {
            if(!find_void_position)
            {
                void_i = i;
                void_j = 0;
                find_void_position = 1;
            }
            continue;
        }
        if(CTC_IP_VER_6 == p_route_info->ip_ver)
        {
            snake_route_num = (SYS_NALPM_SRAM_TYPE_V6_64 == p_prefix_trie_payload->tcam_item.sram_type[i]) ? ROUTE_NUM_PER_SNAKE_V6_64 : ROUTE_NUM_PER_SNAKE_V6_128;
        }

        for(j = 0; j < snake_route_num; j++)
        {
            if(!find_void_position)
            {
                if (NULL == p_prefix_trie_payload->tcam_item.p_ipuc_info_array[i][j])
                {
                    if(CTC_IP_VER_4 == p_route_info->ip_ver
                    ||(SYS_NALPM_SRAM_TYPE_V6_64 == p_prefix_trie_payload->tcam_item.sram_type[i] && p_route_info->route_masklen <= 64)
                        || (SYS_NALPM_SRAM_TYPE_V6_128 == p_prefix_trie_payload->tcam_item.sram_type[i] && p_route_info->route_masklen > 64)
                        || DRV_IS_TSINGMA(lchip))
                    {
                        void_i = i;
                        void_j = j;
                        find_void_position = 1;
                    }
                    continue;
                }
            }
            if (0 == _sys_tsingma_nalpm_route_cmp(p_route_info, p_prefix_trie_payload->tcam_item.p_ipuc_info_array[i][j]))
            {
                p_lkp_rlt->sram_hit = TRUE;
                break; /* find exact route*/
            }
        }
        if(p_lkp_rlt->sram_hit == TRUE)
        {
            break;
        }
    }

    if (p_lkp_rlt->sram_hit == FALSE) /*not found*/
    {

        p_lkp_rlt->snake_idx = void_i;
        p_lkp_rlt->entry_offset = void_j;
    }
    else /* find*/
    {
        p_lkp_rlt->snake_idx = i;
        p_lkp_rlt->entry_offset = j;
    }

END:
    mem_free(p_trie_key);

    return ret;
}

int32
_sys_tsingma_nalpm_read_tcam(uint8 lchip, sys_nalpm_tcam_item_t* p_tcam_item, uint32* p_ad_idx)
{
    uint8 ip_ver = p_tcam_item->p_prefix_info->ip_ver;
    sys_ipuc_tcam_ad_t ad;
    int32 ret = CTC_E_NONE;
    uint16 real_tcam_index = 0;
    if(p_usw_ipuc_master[lchip]->rchain_en)
    {
        _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_tcam_item->tcam_idx, &lchip, &real_tcam_index);
    }
    else
    {
        real_tcam_index = p_tcam_item->tcam_idx;
    }

    sal_memset(&ad, 0 , sizeof(ad));
    ad.tcamAd_key_idx = real_tcam_index;;
    CTC_ERROR_RETURN(sys_tsingma_ipuc_tcam_read_ad( lchip,  ip_ver,  SYS_PRIVATE_MODE, &ad));
    *p_ad_idx = ad.nexthop;

    return ret;
}

int32
_sys_tsingma_nalpm_write_tcam(uint8 lchip, sys_nalpm_tcam_item_t* p_tcam_item, sys_ipuc_opt_type_t opt, uint32 update_ad_idx)
{
    sys_ipuc_tcam_key_t key;
    sys_ipuc_tcam_ad_t ad;
    uint8 ip_ver = p_tcam_item->p_prefix_info->ip_ver;
    int32 ret = CTC_E_NONE;
    uint16 real_tcam_index = 0;
    if(p_usw_ipuc_master[lchip]->rchain_en)
    {
        _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_tcam_item->tcam_idx, &lchip, &real_tcam_index);
    }
    else
    {
        real_tcam_index = p_tcam_item->tcam_idx;
    }

    if(DO_ADD == opt || DO_UPDATE == opt)
    {
        sal_memset(&ad, 0 , sizeof(ad));
        ad.tcamAd_key_idx = real_tcam_index;
        ad.lpmPipelineValid = 1;
        if(DRV_IS_TSINGMA(lchip))
        {

            if(p_tcam_item->p_prefix_info->ip_ver)
            {
                if(p_tcam_item->sram_type[0] == SYS_NALPM_SRAM_TYPE_V6_64 || p_tcam_item->sram_type[1] == SYS_NALPM_SRAM_TYPE_V6_64)
                {
                    ad.lpmStartByte = SYS_NALPM_SRAM_TYPE_V6_64;
                }
                else
                {
                    ad.lpmStartByte = SYS_NALPM_SRAM_TYPE_V6_128;
                }
            }
            else
            {
                ad.lpmStartByte = SYS_NALPM_SRAM_TYPE_V4_32;
            }
        }
        ad.pointer = (p_tcam_item->sram_idx);
        ad.nexthop = (DO_UPDATE == opt)? update_ad_idx : p_tcam_item->p_prefix_info->ad_idx;
        p_tcam_item->p_prefix_info->ad_idx = ad.nexthop;

        CTC_ERROR_RETURN(sys_tsingma_ipuc_tcam_write_ad2hw( lchip,  ip_ver,  SYS_PRIVATE_MODE, &ad, opt));
    }

    if(opt != DO_UPDATE)
    {
        sal_memset(&key, 0 , sizeof(key));
        sal_memcpy(&key.ip, p_tcam_item->p_prefix_info->ip,  IP_ADDR_SIZE(ip_ver));
        key.key_idx = real_tcam_index;
        if(ip_ver == CTC_IP_VER_4)
        {
            _sys_nalpm_convert_to_msb(p_tcam_item->p_prefix_info->ip, p_tcam_item->p_prefix_info->tcam_masklen, &key.ip.ipv4, CTC_IP_VER_4);
        }
        else
        {
            _sys_nalpm_convert_to_msb(p_tcam_item->p_prefix_info->ip, p_tcam_item->p_prefix_info->tcam_masklen, key.ip.ipv6, CTC_IP_VER_6);
        }
        key.mask_len = p_tcam_item->p_prefix_info->tcam_masklen;
        key.vrfId = p_tcam_item->p_prefix_info->vrf_id;
        key.vrfId_mask =  0x1FFF;

        CTC_ERROR_RETURN(sys_tsingma_ipuc_tcam_write_key2hw( lchip,  ip_ver,  SYS_PRIVATE_MODE, &key, opt));
    }

    return ret;
}

int32
_sys_tsingma_nalpm_route_vrf_init(uint8 lchip, ctc_ipuc_param_t*  p_ipuc_param)
{
    uint16 sram_idx = 0;
    uint8  ip_ver = 0;
    uint16 vrf_id = 0;
    uint32 pfx_len = 0;
    sys_nalpm_tcam_item_t* p_tcam_item = NULL;
    sys_nalpm_prefix_trie_payload_t* p_prefix_payload = NULL;
    uint32 pfx[4] = {0};
    trie_t *prefix_trie = NULL;
    sys_nalpm_route_info_t* p_pfx_info = NULL;
    int32 ret = CTC_E_NONE;
    uint8 real_lchip = 0;
    uint16 real_tcam_index = 0;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    ip_ver = p_ipuc_param->ip_ver;
    vrf_id = p_ipuc_param->vrf_id;
    p_prefix_payload = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_nalpm_prefix_trie_payload_t));
    if(NULL == p_prefix_payload)
    {
        ret =  CTC_E_NO_MEMORY;
        goto error1;
    }
    sal_memset(p_prefix_payload, 0, sizeof(sys_nalpm_prefix_trie_payload_t));
    p_tcam_item =  &(p_prefix_payload->tcam_item);

    p_pfx_info = mem_malloc(MEM_IPUC_MODULE, ROUTE_INFO_SIZE(ip_ver));
    if(NULL == p_pfx_info)
    {
        ret =  CTC_E_NO_MEMORY;
        goto error1;
    }
    sal_memset(p_pfx_info, 0, ROUTE_INFO_SIZE(ip_ver));
    p_pfx_info->ip_ver = ip_ver;
    p_pfx_info->vrf_id = vrf_id;
     /*p_pfx_info->ad_idx = p_usw_ipuc_master[lchip]->default_base[ip_ver];*/
    p_pfx_info->ad_idx = INVALID_DRV_NEXTHOP_OFFSET;
    p_tcam_item->p_prefix_info = p_pfx_info;
    CTC_ERROR_GOTO(_sys_tsingma_nalpm_tcam_idx_alloc(lchip, p_ipuc_param, p_tcam_item),ret, error0);
    _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_tcam_item->tcam_idx, &real_lchip, &real_tcam_index);
    CTC_ERROR_GOTO(_sys_tsingma_nalpm_sram_idx_alloc(real_lchip, ip_ver, &sram_idx), ret, error1);
    p_tcam_item->sram_idx = sram_idx;

    p_tcam_item->p_AD_route = NULL;
    p_tcam_item->sram_type[0] = ip_ver ?
        (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_TCAM_SHORT_LOOKUP)? SYS_NALPM_SRAM_TYPE_V6_64 : SYS_NALPM_SRAM_TYPE_V6_128) : SYS_NALPM_SRAM_TYPE_V4_32;
    p_tcam_item->sram_type[1] = ip_ver ?
        (CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_TCAM_SHORT_LOOKUP)? SYS_NALPM_SRAM_TYPE_V6_64 : SYS_NALPM_SRAM_TYPE_V6_128) : SYS_NALPM_SRAM_TYPE_V4_32;
    /*add to prefix trie*/
    /*add to prefix trie*/
    CTC_ERROR_GOTO(sys_nalpm_trie_init ((CTC_IP_VER_4 == ip_ver)? SYS_NALPM_V4_MASK_LEN_MAX : SYS_NALPM_V6_MASK_LEN_MAX,
                                    &(g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrf_id])),
                   ret, error2);

    prefix_trie = g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrf_id];
    CTC_ERROR_GOTO(sys_nalpm_trie_insert (prefix_trie, pfx, NULL, pfx_len, (trie_node_t *) p_prefix_payload, 0), ret, error3);

    CTC_ERROR_GOTO(sys_nalpm_trie_init (CTC_IP_VER_4 == ip_ver? SYS_NALPM_V4_MASK_LEN_MAX : SYS_NALPM_V6_MASK_LEN_MAX, &p_tcam_item->trie),ret, error4);
    CTC_ERROR_GOTO(_sys_tsingma_nalpm_write_tcam( lchip, p_tcam_item, DO_ADD, 0), ret, error5);

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "(%d)    p_prefix_payload [%p]\n",__LINE__, p_prefix_payload);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "(%d)    p_tcam_item [%p]\n", __LINE__, p_tcam_item);

    return ret;

error5:
    sys_nalpm_trie_destroy(p_tcam_item->trie);
error4:
    sys_nalpm_trie_delete(prefix_trie, pfx, pfx_len, (trie_node_t * *) &p_prefix_payload,  0);
error3:
    sys_nalpm_trie_destroy(prefix_trie);
    g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrf_id] = NULL;
error2:
    _sys_tsingma_nalpm_sram_idx_free( lchip,  sram_idx);
error1:
    _sys_tsingma_nalpm_tcam_idx_free( lchip, p_ipuc_param, p_tcam_item);
error0:
    if(p_prefix_payload)
        mem_free(p_prefix_payload);
    if(p_pfx_info)
        mem_free(p_pfx_info);

    return ret;
}

int32
_sys_tsingma_nalpm_route_vrf_deinit(uint8 lchip, ctc_ipuc_param_t*  p_ipuc_param)
{
    sys_nalpm_tcam_item_t* p_tcam_item = NULL;
    sys_nalpm_prefix_trie_payload_t * p_pfx_payload = NULL;
    uint32 pfx_len = 0;
    uint32 pfx[4] = {0};
    trie_t* vrf_trie;
    uint8  ip_ver = 0;
    uint16 vrf_id = 0;
    uint8 real_lchip = 0;
    uint16 real_index = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    /* prepare data */
    ip_ver = p_ipuc_param->ip_ver;
    vrf_id = p_ipuc_param->vrf_id;

    vrf_trie = g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrf_id];
    p_pfx_payload = CONTAINER_OF(vrf_trie->trie, sys_nalpm_prefix_trie_payload_t, node);
    p_tcam_item = &p_pfx_payload->tcam_item;

    if(p_usw_ipuc_master[lchip]->rchain_en)
    {
        _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_tcam_item->tcam_idx, &real_lchip, &real_index);
    }
    else
    {
        real_lchip = lchip;
    }


    CTC_ERROR_RETURN(_sys_tsingma_nalpm_tcam_idx_free(lchip, p_ipuc_param, p_tcam_item));

    CTC_ERROR_RETURN(_sys_tsingma_nalpm_sram_idx_free(real_lchip, p_tcam_item->sram_idx));

    CTC_ERROR_RETURN(sys_nalpm_trie_destroy(p_tcam_item->trie));
    /*pfx trie*/
    vrf_trie = g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrf_id];
    CTC_ERROR_RETURN(sys_nalpm_trie_delete (vrf_trie, pfx, pfx_len, (trie_node_t **)&p_pfx_payload, 0));
    if (NULL != vrf_trie->trie)  /* sram entry is empty, release tcam*/
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---[%d]---\n",__LINE__);
        return CTC_E_UNEXPECT;
    }
    CTC_ERROR_RETURN(sys_nalpm_trie_destroy(vrf_trie));
    CTC_ERROR_RETURN(_sys_tsingma_nalpm_write_tcam(lchip, p_tcam_item, DO_DEL, 0));
    g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrf_id] = NULL;


    mem_free(p_pfx_payload->tcam_item.p_prefix_info);
    mem_free(p_pfx_payload);
    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_nalpm_sram_entry_is_full(uint8 lchip, sys_nalpm_tcam_item_t *p_tcam_item, sys_nalpm_route_info_t*  p_route_info)
{
    uint8 i = 0, j =0;
    uint8 snake_num = 0;
    uint8 snake_route_num = 0;

    _sys_tsingma_nalpm_get_snake_num(lchip, p_route_info->ip_ver, &snake_num);

    if(CTC_IP_VER_4 == p_route_info->ip_ver)
    {
        snake_route_num = ROUTE_NUM_PER_SNAKE_V4;
    }

    for(i = 0; i < snake_num; i++)
    {
        if(CTC_IP_VER_6 == p_route_info->ip_ver)
        {
            if (SYS_NALPM_SRAM_TYPE_VOID == p_tcam_item->sram_type[i])
            {
                return FALSE;
            }
            snake_route_num = (SYS_NALPM_SRAM_TYPE_V6_64 == p_tcam_item->sram_type[i]) ? ROUTE_NUM_PER_SNAKE_V6_64 : ROUTE_NUM_PER_SNAKE_V6_128;
        }

        for(j = 0; j < snake_route_num; j++)
        {
            if (NULL == p_tcam_item->p_ipuc_info_array[i][j])
            {
                if (CTC_IP_VER_4 == p_route_info->ip_ver
                ||(SYS_NALPM_SRAM_TYPE_V6_64 == p_tcam_item->sram_type[i] && p_route_info->route_masklen <= 64)
                ||(SYS_NALPM_SRAM_TYPE_V6_128 == p_tcam_item->sram_type[i] && p_route_info->route_masklen > 64)
                || DRV_IS_TSINGMA(lchip))
                {
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

int
_sys_tsingma_nalpm_route_install_trie_payload (trie_node_t * node, void *data)
{
    uint8 i = 0, j = 0, snake_num = 0, route_per_snake = 0;

    sys_nalpm_install_route_trie_param_t* p_install_route_trie_param = data;
    sys_nalpm_tcam_item_t* p_tcam_item = p_install_route_trie_param->p_install_tcam;
    uint8 lchip = p_install_route_trie_param->lchip;
    uint8* p_v4_cnt = p_install_route_trie_param->p_v4_cnt;
    uint8 inserted = 0;
    uint8 new_masklen = p_install_route_trie_param->new_mask_len;
    sys_nalpm_route_info_t* p_route_info = NULL;

    sys_nalpm_trie_payload_t* p_payload = CONTAINER_OF(node, sys_nalpm_trie_payload_t, node);

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (node->type != PAYLOAD)
    {
        return CTC_E_NONE;
    }
    p_route_info = p_payload->p_route_info;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_payload=[%p], node = [%p],p_route_info=[%p]\n", p_payload, node , p_route_info);

    _sys_tsingma_nalpm_get_snake_num(lchip, p_route_info->ip_ver, &snake_num);

    if (p_route_info->ip_ver == CTC_IP_VER_4)
    {
        p_tcam_item->sram_type[*p_v4_cnt / ROUTE_NUM_PER_SNAKE_V4] = SYS_NALPM_SRAM_TYPE_V4_32;
        p_tcam_item->p_ipuc_info_array[*p_v4_cnt / ROUTE_NUM_PER_SNAKE_V4][*p_v4_cnt % ROUTE_NUM_PER_SNAKE_V4] = p_route_info;
        (*p_v4_cnt)++;
        if (p_install_route_trie_param->p_new_route && p_install_route_trie_param->ip_addr
            && 0 == sal_memcmp(p_route_info->ip, p_install_route_trie_param->ip_addr, sizeof(ip_addr_t))
            && new_masklen == p_route_info->route_masklen)
        {
            *(p_install_route_trie_param->p_new_route) = p_route_info;
        }
    }
    else
    {
        for (; i < snake_num; i++)
        {
            if (p_route_info->route_masklen <= 64 && CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_6], SYS_TCAM_SHORT_LOOKUP))
            {
                p_tcam_item->sram_type[i] = SYS_NALPM_SRAM_TYPE_V6_64;
            }
            else
            {
                p_tcam_item->sram_type[i] = SYS_NALPM_SRAM_TYPE_V6_128;
            }

            route_per_snake = (SYS_NALPM_SRAM_TYPE_V6_64 == p_tcam_item->sram_type[i]) ? ROUTE_NUM_PER_SNAKE_V6_64 : ROUTE_NUM_PER_SNAKE_V6_128;
            for (j = 0; j < route_per_snake; j++)
            {
                if (NULL == p_tcam_item->p_ipuc_info_array[i][j] && !inserted
                    && ((SYS_NALPM_SRAM_TYPE_V6_64 == p_tcam_item->sram_type[i] && p_route_info->route_masklen <= 64)
                || (SYS_NALPM_SRAM_TYPE_V6_128 == p_tcam_item->sram_type[i] && p_route_info->route_masklen > 64)
                || DRV_IS_TSINGMA(lchip)))
                {
                    p_tcam_item->p_ipuc_info_array[i][j] = p_route_info;
                    inserted = 1;
                }
                if (p_install_route_trie_param->p_new_route && p_install_route_trie_param->ip_addr
                    && (p_route_info->ip[0] == (p_install_route_trie_param->ip_addr[3]))
                    && (p_route_info->ip[1] == (p_install_route_trie_param->ip_addr[2]))
                && (p_route_info->ip[2] == (p_install_route_trie_param->ip_addr[1]))
                && (p_route_info->ip[3] == (p_install_route_trie_param->ip_addr[0]))
                && new_masklen == p_route_info->route_masklen)
                {
                    *(p_install_route_trie_param->p_new_route) = p_route_info;
                }
            }
        }
        if (!inserted)
        {
            return CTC_E_NO_RESOURCE;
        }
    }
    return CTC_E_NONE;
}


STATIC int32
_sys_tsingma_nalpm_renew_route_trie(uint8 lchip,sys_nalpm_tcam_item_t* p_tcam_item, ip_addr_t* ip_addr, uint8 new_addr_masklen, sys_nalpm_route_info_t** p_new_route, uint8 new_tcam)
{
    int32 ret = CTC_E_NONE;
    uint8 v4_cnt = 0;
    uint8 i = 0, j = 0;
    uint8  cmp_size = 0;
    uint8 snake_num = 0;
    uint32 t = 0;
    sys_nalpm_install_route_trie_param_t install_route_trie_param;
    sys_nalpm_route_info_t* p_old_ipuc_info_array[2*ROUTE_NUM_PER_SNAKE_V4];
    sys_nalpm_route_info_t* p_new_ipuc_info_array[2*ROUTE_NUM_PER_SNAKE_V4];
    ip_addr_t* sort_ip_addr;

    install_route_trie_param.lchip = lchip;
    install_route_trie_param.p_install_tcam = p_tcam_item;
    install_route_trie_param.p_v4_cnt= &v4_cnt;
    install_route_trie_param.p_new_route = p_new_route;
    install_route_trie_param.ip_addr = ip_addr;
    install_route_trie_param.new_mask_len = new_addr_masklen;
    cmp_size = (p_tcam_item->p_prefix_info->ip_ver== CTC_IP_VER_4)?4:16;
    sort_ip_addr = mem_malloc(MEM_IPUC_MODULE, cmp_size);
    if (ip_addr)
    {
        sal_memcpy(sort_ip_addr, ip_addr, cmp_size);
    }
    if (CTC_IP_VER_6 == p_tcam_item->p_prefix_info->ip_ver)
    {
        t = sort_ip_addr[0];
        sort_ip_addr[0] = sort_ip_addr[3];
        sort_ip_addr[3] = t;

        t = sort_ip_addr[1];
        sort_ip_addr[1] = sort_ip_addr[2];
        sort_ip_addr[2] = t;
    }

    _sys_tsingma_nalpm_get_snake_num(lchip, p_tcam_item->p_prefix_info->ip_ver, &snake_num);
    for (i = 0; (i < snake_num) && !new_tcam; i++)
    {
        for (j = 0; j < ROUTE_NUM_PER_SNAKE_V4; j++)
        {
            if (p_tcam_item->p_ipuc_info_array[i][j])
            {
                p_old_ipuc_info_array[6*i+j] = p_tcam_item->p_ipuc_info_array[i][j];
            }
            else
            {
                p_old_ipuc_info_array[6*i+j] = NULL;
            }
        }
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    sal_memset(p_tcam_item->p_ipuc_info_array, 0 , sizeof(p_tcam_item->p_ipuc_info_array));
    CTC_ERROR_RETURN(sys_nalpm_trie_traverse (p_tcam_item->trie->trie, _sys_tsingma_nalpm_route_install_trie_payload, (void*)&install_route_trie_param, _TRIE_INORDER_TRAVERSE));
    if (!new_tcam)
    {
        sys_nalpm_route_info_t* p_new_data = NULL;
        sys_nalpm_route_info_t* p_old_data = NULL;
        void** p_final_data = NULL;
        uint8  old_idx = 0;
        uint8  new_idx = 0;
        uint8  find_new = 0;
        uint8 route_num_per_snake = 0;
        sys_nalpm_route_info_t * new_add = 0;
        uint8  cur_idx = 0;
        for (i = 0; i < snake_num; i++)
        {
            for (j = 0; j < ROUTE_NUM_PER_SNAKE_V4; j++)
            {
                if (p_tcam_item->p_ipuc_info_array[i][j])
                {
                    p_new_ipuc_info_array[6*i + j] = p_tcam_item->p_ipuc_info_array[i][j];
                    p_tcam_item->p_ipuc_info_array[i][j] = NULL;
                }
                else
                {
                    p_new_ipuc_info_array[6*i + j] = NULL;
                }
            }
        }
        p_final_data = (void**)&p_tcam_item->p_ipuc_info_array[0][0];

        if (CTC_IP_VER_4 == p_tcam_item->p_prefix_info->ip_ver)
        {
            route_num_per_snake = ROUTE_NUM_PER_SNAKE_V4;
        }
        else
        {
            route_num_per_snake = (SYS_NALPM_SRAM_TYPE_V6_64 == (p_tcam_item->sram_type[0] | p_tcam_item->sram_type[1]))?
            ROUTE_NUM_PER_SNAKE_V6_64:ROUTE_NUM_PER_SNAKE_V6_128;
        }

        for (old_idx = 0; old_idx < 12; old_idx++)
        {
            p_old_data = (sys_nalpm_route_info_t*)(p_old_ipuc_info_array[old_idx]);
            if (!p_old_data)
            {
                continue;
            }
            for (new_idx = 0; new_idx < 12; new_idx++)
            {
                p_new_data = (sys_nalpm_route_info_t*)p_new_ipuc_info_array[new_idx];
                if (p_new_data && (p_old_data == p_new_data))
                {
                    *p_final_data = p_new_data;
                    cur_idx++;
                    p_final_data = (cur_idx >= route_num_per_snake)?(p_final_data + (6 - route_num_per_snake + 1)):p_final_data + 1;
                    if (cur_idx >= route_num_per_snake)
                    {
                        cur_idx = 0;
                    }
                    break;
                }
                if (p_new_data && !sal_memcmp(sort_ip_addr, p_new_data->ip, cmp_size) && !find_new && (new_addr_masklen == p_new_data->route_masklen))
                {
                    new_add = p_new_data;
                    find_new = 1;
                }
            }
        }
        if (find_new)
        {
            *p_final_data = new_add;
            *p_new_route = *p_final_data;
        }
    }
    mem_free(sort_ip_addr);
    return ret;
}

int32
_sys_tsingma_nalpm_write_route_trie(uint8 lchip,sys_nalpm_tcam_item_t* p_tcam_item, uint8 new_tcam)
{
    int32 ret = CTC_E_NONE;
    uint32 i = 0, j = 0;
    uint8 snake_num = 0, route_per_snake = 0;
    uint8 clear_snake_type =0;
    uint8 ip_ver = p_tcam_item->p_prefix_info->ip_ver;
    sys_nalpm_route_info_t* p_route_info = NULL;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    _sys_tsingma_nalpm_get_snake_num(lchip, ip_ver, &snake_num);
    if (ip_ver == CTC_IP_VER_4)
    {
        for(i = 0; i < (snake_num * ROUTE_NUM_PER_SNAKE_V4); i++)
        {
            p_route_info = p_tcam_item->p_ipuc_info_array[i/ROUTE_NUM_PER_SNAKE_V4][i%ROUTE_NUM_PER_SNAKE_V4];
            clear_snake_type = (0 == ((i + 1) % ROUTE_NUM_PER_SNAKE_V4))?1:0;
            clear_snake_type = clear_snake_type && !new_tcam;
            if(NULL == p_route_info)
            {
                _sys_nalpm_clear_sram(lchip, p_tcam_item, i/ROUTE_NUM_PER_SNAKE_V4, i % ROUTE_NUM_PER_SNAKE_V4, clear_snake_type);
            }
            else
            {
                CTC_ERROR_RETURN(_sys_nalpm_write_sram(lchip, p_tcam_item, p_route_info, i / ROUTE_NUM_PER_SNAKE_V4, i % ROUTE_NUM_PER_SNAKE_V4, 0));
            }
        }
    }
    else
    {
        for (i = 0; i < snake_num; i++)
        {
            route_per_snake = (SYS_NALPM_SRAM_TYPE_V6_64 == p_tcam_item->sram_type[i]) ? ROUTE_NUM_PER_SNAKE_V6_64 : ROUTE_NUM_PER_SNAKE_V6_128;
            for (j = 0; j < route_per_snake; j++)
            {
                clear_snake_type = ((j + 1) == route_per_snake)?1:0;
                clear_snake_type = clear_snake_type && !new_tcam;
                p_route_info = p_tcam_item->p_ipuc_info_array[i][j];
                if (NULL == p_tcam_item->p_ipuc_info_array[i][j])
                {
                    _sys_nalpm_clear_sram(lchip, p_tcam_item, i, j, clear_snake_type);
                }
                else
                {
                    CTC_ERROR_RETURN(_sys_nalpm_write_sram(lchip, p_tcam_item, p_route_info, i, j, 0));
                }
            }
        }
    }
    return ret;
}

int32
_sys_tsingma_nalpm_is_father_pfx(trie_node_t * p_node, void * data)
{
    uint8 i = 0;
    sys_nalpm_son2father_t * p_son2father = (sys_nalpm_son2father_t *)data;
    for(i = 0; i < 2; i++)
    {
        if(p_node->child[i].child_node != NULL)
        {
            if(p_node->child[i].child_node == p_son2father->p_son_node)
            {
                if(p_node->type == PAYLOAD)
                {
                    p_son2father->p_father_node = p_node;
                }
                else
                {
                    CTC_ERROR_RETURN(_sys_tsingma_nalpm_search_father_pfx(p_son2father->prefix_trie, p_node, &p_son2father->p_father_node));
                }
                return CTC_E_NONE;
            }
        }
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_nalpm_search_father_pfx(trie_t * prefix_trie, trie_node_t * p_son_node, trie_node_t ** p_father_node)
{
    sys_nalpm_son2father_t son2father;

    son2father.p_son_node = p_son_node;
    son2father.p_father_node = NULL;
    son2father.prefix_trie = prefix_trie;
    CTC_ERROR_RETURN(sys_nalpm_trie_traverse (prefix_trie->trie, _sys_tsingma_nalpm_is_father_pfx,&son2father, _TRIE_PREORDER_TRAVERSE));

    *p_father_node = son2father.p_father_node;

    return CTC_E_NONE;
}

int32
_sys_tsingma_nalpm_search_best_route_in_sram(uint8 lchip, trie_node_t *p_prefix_node, sys_nalpm_route_info_t* p_pfx_route,sys_nalpm_route_info_t** pp_best_route)
{
    int32 ret = CTC_E_NONE;
    uint8 route_per_snake = 0;
    uint8 best_snake = 0xff;
    uint8 best_offset = 0xff;
    uint8 i = 0, j = 0, snake_num = 0;
    sys_nalpm_tcam_item_t* p_tcam_item = NULL;
    sys_nalpm_prefix_trie_payload_t * p_prefix_trie_payload = CONTAINER_OF(p_prefix_node, sys_nalpm_prefix_trie_payload_t, node);

    *pp_best_route = NULL;
    p_tcam_item = &(p_prefix_trie_payload->tcam_item);

    if(p_tcam_item->p_AD_route)
    {
        if(0 == _sys_tsingma_nalpm_route_match(lchip, p_pfx_route, p_tcam_item->p_AD_route))
        {
            *pp_best_route = p_tcam_item->p_AD_route;
        }
    }
    _sys_tsingma_nalpm_get_snake_num(lchip, p_pfx_route->ip_ver, &snake_num);

    for (; i < snake_num; i++)
    {
        if (SYS_NALPM_SRAM_TYPE_VOID == p_tcam_item->sram_type[i])
        {
            continue;
        }
        if(p_pfx_route->ip_ver == CTC_IP_VER_4)
        {
            route_per_snake = ROUTE_NUM_PER_SNAKE_V4;
        }
        else
        {
            route_per_snake = (SYS_NALPM_SRAM_TYPE_V6_64 == p_tcam_item->sram_type[i]) ? ROUTE_NUM_PER_SNAKE_V6_64 : ROUTE_NUM_PER_SNAKE_V6_128;
        }
        for (j = 0; j < route_per_snake; j++)
        {
            if (NULL != p_tcam_item->p_ipuc_info_array[i][j])
            {
                if (_sys_tsingma_nalpm_route_match(lchip, p_pfx_route, p_tcam_item->p_ipuc_info_array[i][j]))
                {
                    continue;
                }
                if (0xff == best_snake
                || p_tcam_item->p_ipuc_info_array[i][j]->route_masklen > p_tcam_item->p_ipuc_info_array[best_snake][best_offset]->route_masklen)
                {
                    best_snake = i;
                    best_offset = j;
                }
            }
        }
    }
    if(best_snake != 0xff)
    {
        (*pp_best_route) = p_tcam_item->p_ipuc_info_array[best_snake][best_offset];
    }

    return ret;
}

int32
_sys_tsingma_nalpm_update_longer_pfx_AD_add(trie_node_t* p_longer_pfx_node,void* data)
{
    int32 ret = CTC_E_NONE;
    sys_nalpm_route_and_lchip_t* route_and_lchip = (sys_nalpm_route_and_lchip_t*) data;
    uint8 lchip = route_and_lchip->lchip;
    uint8 origin_lchip = route_and_lchip->origin_lchip;
    uint32 nh_id = route_and_lchip->nh_id;
    sys_nalpm_route_info_t* p_new_route = route_and_lchip->p_route;
    sys_nalpm_prefix_trie_payload_t * p_prefix_trie_payload = NULL;

    if(p_longer_pfx_node->type  != PAYLOAD)
    {
        return ret;
    }
    p_prefix_trie_payload = CONTAINER_OF(p_longer_pfx_node, sys_nalpm_prefix_trie_payload_t, node);
    if((p_prefix_trie_payload->tcam_item.p_prefix_info->tcam_masklen > p_new_route->route_masklen) &&
        (0 != _sys_tsingma_nalpm_route_match(lchip, p_prefix_trie_payload->tcam_item.p_prefix_info, p_new_route)))
    {
        return CTC_E_NOT_EXIST;
    }
    if(_sys_tsingma_nalpm_route_match(lchip, p_prefix_trie_payload->tcam_item.p_prefix_info, p_new_route) == 0)
    {
        if(p_prefix_trie_payload->tcam_item.p_AD_route == NULL
            || p_prefix_trie_payload->tcam_item.p_AD_route->route_masklen <= p_new_route->route_masklen)
        {
            uint8 real_lchip = 0;
            uint16 real_index = 0;
            _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_prefix_trie_payload->tcam_item.tcam_idx, &real_lchip, &real_index);
            if (p_usw_ipuc_master[lchip]->rchain_en && origin_lchip != real_lchip)
            {
                uint32 new_ad_index = 0;
                CTC_ERROR_RETURN(_sys_tsingma_nalpm_update_ad(real_lchip, origin_lchip, p_new_route, 1, nh_id, &new_ad_index));
                CTC_ERROR_RETURN(_sys_tsingma_nalpm_write_tcam( lchip, &(p_prefix_trie_payload->tcam_item), DO_UPDATE, new_ad_index));
                p_prefix_trie_payload->tcam_item.p_prefix_info->ad_idx = new_ad_index;
                p_prefix_trie_payload->tcam_item.p_AD_route = p_new_route;
            }
            else
            {
                p_prefix_trie_payload->tcam_item.p_AD_route = p_new_route;
                CTC_ERROR_RETURN(_sys_tsingma_nalpm_write_tcam( lchip, &(p_prefix_trie_payload->tcam_item), DO_UPDATE, p_new_route->ad_idx));
            }
        }
    }

    return ret;
}

int32
_sys_tsingma_nalpm_add_route_cover(uint8 lchip,sys_nalpm_tcam_item_t* p_tcam_item,sys_nalpm_route_info_t* p_new_route, uint32 nh_id)
{
    uint8 real_lchip = 0;
    uint16 real_index = 0;
    int32 ret = CTC_E_NONE;
    trie_node_t* p_prefix_node = NULL;
    sys_nalpm_route_and_lchip_t route_and_lchip;
    sys_nalpm_prefix_trie_payload_t* p_prefix_trie_payload = NULL;

    route_and_lchip.lchip = lchip;
    route_and_lchip.p_route = p_new_route;
    _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_tcam_item->tcam_idx, &real_lchip, &real_index);
    route_and_lchip.origin_lchip = real_lchip;
    route_and_lchip.nh_id = nh_id;

    /*find trie node of  current tcam_pfx from prefix_trie */
    p_prefix_trie_payload = CONTAINER_OF(p_tcam_item, sys_nalpm_prefix_trie_payload_t, tcam_item);
    p_prefix_node = &(p_prefix_trie_payload->node);
     /*CTC_ERROR_RETURN(trie_search (prefix_trie,p_tcam_item->p_prefix_info->ip, p_tcam_item->p_prefix_info->tcam_masklen, &p_prefix_node));*/
    if(p_prefix_node->child[0].child_node)
    {
        sys_nalpm_trie_traverse (p_prefix_node->child[0].child_node, _sys_tsingma_nalpm_update_longer_pfx_AD_add,(void *)&route_and_lchip, _TRIE_PREORDER_AD_ROUTE_TRAVERSE);
    }

    if(p_prefix_node->child[1].child_node)
    {
        sys_nalpm_trie_traverse (p_prefix_node->child[1].child_node, _sys_tsingma_nalpm_update_longer_pfx_AD_add,(void *)&route_and_lchip, _TRIE_PREORDER_AD_ROUTE_TRAVERSE);
    }

    return ret;
}

int32
_sys_tsingma_nalpm_update_longer_pfx_AD_del(trie_node_t* p_longer_pfx_node,void* data)
{
    int32 ret = CTC_E_NONE;
    uint8 curr_lchip = 0;
    uint16 curr_index = 0;
    sys_nalpm_route_info_t *p_route_info = NULL;
    sys_nalpm_route_and_lchip_t* p_route_and_lchip = (sys_nalpm_route_and_lchip_t*)data;
    uint8 lchip = p_route_and_lchip->lchip;
    uint8 origin_lchip = 0;
    sys_nalpm_route_info_t * p_del_route = p_route_and_lchip->p_route;
    sys_nalpm_prefix_trie_payload_t * p_prefix_trie_payload = NULL;
    sys_nalpm_prefix_trie_payload_t * p_root_payload = NULL;
    trie_node_t * p_father_node = NULL;
    trie_t *prefix_trie = NULL;
    sys_nalpm_route_info_t* p_best_route = NULL;
    uint32 ad_idx = INVALID_DRV_NEXTHOP_OFFSET;


    if(p_longer_pfx_node->type  != PAYLOAD)
    {
        return ret;
    }

    p_prefix_trie_payload = CONTAINER_OF(p_longer_pfx_node, sys_nalpm_prefix_trie_payload_t, node);
    ret = _sys_tsingma_nalpm_route_match(lchip, p_prefix_trie_payload->tcam_item.p_prefix_info, p_del_route);
    if (0 != ret)
    {
        return ret;
    }

    if(0 != _sys_tsingma_nalpm_route_cmp(p_prefix_trie_payload->tcam_item.p_AD_route, p_del_route))
    {
        return ret;
    }

    prefix_trie = g_sys_nalpm_master[lchip]->prefix_trie[p_del_route->ip_ver][p_del_route->vrf_id];
    CTC_ERROR_RETURN(_sys_tsingma_nalpm_search_father_pfx(prefix_trie, p_longer_pfx_node, &p_father_node));
    if(NULL == p_father_node)
    {
        return ret;
    }

    CTC_ERROR_RETURN(_sys_tsingma_nalpm_search_best_route_in_sram(lchip, p_father_node, p_prefix_trie_payload->tcam_item.p_prefix_info, &p_best_route));
    p_prefix_trie_payload->tcam_item.p_AD_route = p_best_route;
    _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_prefix_trie_payload->tcam_item.tcam_idx, &curr_lchip, &curr_index);
    if (p_best_route)
    {
        sys_nalpm_prefix_trie_payload_t * p_father_payload = NULL;
        uint8 father_lchip = 0;
        uint16 father_index = 0;
        p_father_payload = CONTAINER_OF(p_father_node, sys_nalpm_prefix_trie_payload_t, node);
        _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_father_payload->tcam_item.tcam_idx, &father_lchip, &father_index);

        if (p_usw_ipuc_master[lchip]->rchain_en && father_lchip != curr_lchip)
        {
            uint32 new_ad_index = 0;
            CTC_ERROR_RETURN(_sys_tsingma_nalpm_update_ad(curr_lchip, origin_lchip, p_best_route, 1, 0, &new_ad_index));
            CTC_ERROR_RETURN(_sys_tsingma_nalpm_write_tcam( lchip, &(p_prefix_trie_payload->tcam_item), DO_UPDATE, new_ad_index));
        }
        else
        {
            CTC_ERROR_RETURN(_sys_tsingma_nalpm_write_tcam( lchip, &(p_prefix_trie_payload->tcam_item), DO_UPDATE, p_best_route->ad_idx));
        }
    }
    else
    {
        uint8 root_lchip = 0;
        uint16 root_index = 0;

        p_root_payload = CONTAINER_OF(prefix_trie->trie, sys_nalpm_prefix_trie_payload_t, node);
        _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_root_payload->tcam_item.tcam_idx, &root_lchip, &root_index);
        CTC_ERROR_RETURN(_sys_tsingma_nalpm_read_tcam(lchip, &p_root_payload->tcam_item, &ad_idx));
        if (p_usw_ipuc_master[lchip]->rchain_en && curr_lchip != root_lchip && 0x3ffff != ad_idx)
        {
            uint32 new_ad_index = 0;
            p_route_info = mem_malloc(MEM_IPUC_MODULE, ROUTE_INFO_SIZE(p_del_route->ip_ver));
            if (!p_route_info)
            {
                return CTC_E_NO_MEMORY;
            }
            sal_memset(p_route_info, 0, ROUTE_INFO_SIZE(p_del_route->ip_ver));
            p_route_info->ad_idx = ad_idx;
            p_route_info->ip_ver = p_del_route->ip_ver;
            p_route_info->route_masklen = 0;
            p_route_info->vrf_id = p_del_route->vrf_id;
            CTC_ERROR_GOTO(_sys_tsingma_nalpm_update_ad(curr_lchip, root_lchip, p_route_info, 1, 0, &new_ad_index), ret, error0);
            CTC_ERROR_GOTO(_sys_tsingma_nalpm_write_tcam( lchip, &(p_prefix_trie_payload->tcam_item), DO_UPDATE, new_ad_index), ret, error0);
        }
        else
        {
            CTC_ERROR_RETURN(_sys_tsingma_nalpm_write_tcam( lchip, &(p_prefix_trie_payload->tcam_item), DO_UPDATE, ad_idx));
        }
    }

error0:
    if (p_route_info)
    {
        mem_free(p_route_info);
    }
    return ret;
}

int32
_sys_tsingma_nalpm_deletes_route_cover(uint8 lchip, sys_nalpm_tcam_item_t* p_tcam_item, sys_nalpm_route_info_t* p_del_route)
{
    int32 ret = CTC_E_NONE;
    trie_node_t* p_prefix_node = NULL;
    sys_nalpm_route_and_lchip_t del_route_lchip;
    sys_nalpm_prefix_trie_payload_t* p_prefix_trie_payload = NULL;

    /*find trie node of  current tcam_pfx from prefix_trie */
    p_prefix_trie_payload = CONTAINER_OF(p_tcam_item, sys_nalpm_prefix_trie_payload_t, tcam_item);
    p_prefix_node = &(p_prefix_trie_payload->node);
     /*CTC_ERROR_RETURN(trie_search (prefix_trie,p_tcam_item->p_prefix_info->ip, p_tcam_item->p_prefix_info->tcam_masklen, &p_prefix_node));*/

    del_route_lchip.p_route = p_del_route;
    del_route_lchip.lchip = lchip;

    if(p_prefix_node->child[0].child_node)
    {
        sys_nalpm_trie_traverse (p_prefix_node->child[0].child_node, _sys_tsingma_nalpm_update_longer_pfx_AD_del,(void *)&del_route_lchip, _TRIE_PREORDER_AD_ROUTE_TRAVERSE);
    }

    if(p_prefix_node->child[1].child_node)
    {
        sys_nalpm_trie_traverse (p_prefix_node->child[1].child_node, _sys_tsingma_nalpm_update_longer_pfx_AD_del, (void *)&del_route_lchip, _TRIE_PREORDER_AD_ROUTE_TRAVERSE);
    }
    return ret;
}

int32
_sys_tsingma_nalpm_update_longer_pfx_AD_default(trie_node_t* p_longer_pfx_node,void* data)
{
    int32 ret = CTC_E_NONE;
    sys_nalpm_route_and_lchip_t* p_route_and_lchip = (sys_nalpm_route_and_lchip_t*)data;
    sys_nalpm_prefix_trie_payload_t * p_prefix_trie_payload = NULL;
    uint32 ad_idx = p_route_and_lchip->p_route->ad_idx;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    if(p_longer_pfx_node->type  != PAYLOAD)
    {
        return ret;
    }

    p_prefix_trie_payload = CONTAINER_OF(p_longer_pfx_node, sys_nalpm_prefix_trie_payload_t, node);

    if(NULL == p_prefix_trie_payload->tcam_item.p_AD_route)
    {
        CTC_ERROR_RETURN(_sys_tsingma_nalpm_write_tcam(p_route_and_lchip->lchip, &(p_prefix_trie_payload->tcam_item), DO_UPDATE, ad_idx));
    }

    return ret;
}

int32
_sys_tsingma_nalpm_default_route_cover(uint8 lchip,uint8 ip_ver, uint16 vrf_id, uint32 ad_index)
{
    int32 ret = CTC_E_NONE;
    trie_node_t* p_prefix_node = NULL;
    sys_nalpm_route_and_lchip_t default_info;

    p_prefix_node = g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrf_id]->trie;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    default_info.p_route = mem_malloc(MEM_IPUC_MODULE, ROUTE_INFO_SIZE(ip_ver));
    if(NULL == default_info.p_route)
    {
        return CTC_E_NO_MEMORY;
    }
    default_info.lchip = lchip;
    default_info.p_route->ad_idx = ad_index;
    default_info.p_route->ip_ver = ip_ver;
    default_info.p_route->vrf_id = vrf_id;

    if(p_prefix_node->child[0].child_node)
    {
        CTC_ERROR_GOTO(sys_nalpm_trie_traverse (p_prefix_node->child[0].child_node, _sys_tsingma_nalpm_update_longer_pfx_AD_default,(void *)&default_info, _TRIE_PREORDER_TRAVERSE), ret,error);
    }

    if(p_prefix_node->child[1].child_node)
    {
        CTC_ERROR_GOTO(sys_nalpm_trie_traverse (p_prefix_node->child[1].child_node, _sys_tsingma_nalpm_update_longer_pfx_AD_default,(void *)&default_info, _TRIE_PREORDER_TRAVERSE), ret,error);
    }

error:
    mem_free(default_info.p_route);

    return ret;
}

int32
_sys_tsingma_nalpm_add_directly(uint8 lchip, sys_nalpm_tcam_item_t* p_tcam_item, sys_nalpm_route_info_t*  p_route_info, sys_nalpm_route_store_info_t *p_lkp_rlt, uint32 nh_id)
{

    int ret = CTC_E_NONE;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if(SYS_NALPM_SRAM_TYPE_VOID == p_tcam_item->sram_type[p_lkp_rlt->snake_idx])
    {
        if (CTC_IP_VER_4 == p_route_info->ip_ver)
        {
            p_tcam_item->sram_type[p_lkp_rlt->snake_idx] = SYS_NALPM_SRAM_TYPE_V4_32;
        }
        else
        {
            if (p_route_info->route_masklen <= 64 && p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT])
            {
                p_tcam_item->sram_type[p_lkp_rlt->snake_idx] = SYS_NALPM_SRAM_TYPE_V6_64;
            }
            else
            {
                p_tcam_item->sram_type[p_lkp_rlt->snake_idx] = SYS_NALPM_SRAM_TYPE_V6_128;
            }
        }
    }

    CTC_ERROR_RETURN(_sys_nalpm_write_sram(lchip, p_tcam_item, p_route_info, p_lkp_rlt->snake_idx, p_lkp_rlt->entry_offset, 0));

    p_tcam_item->p_ipuc_info_array[p_lkp_rlt->snake_idx][p_lkp_rlt->entry_offset] = p_route_info;

    g_sys_nalpm_master[lchip]->vrf_route_cnt[p_route_info->ip_ver][p_route_info->vrf_id]++;

    ret = _sys_tsingma_nalpm_add_route_cover(lchip, p_tcam_item, p_route_info, nh_id);
    if(ret < 0)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "---[%d]--- \r\n", __LINE__);
        return ret;
    }

    return ret;

}

int32
_sys_tsingma_nalpm_renew_dsipda(uint8 lchip, void* dsipda, sys_nh_info_dsnh_t nhinfo)
{
    if (!GetDsIpDa(V, nextHopPtrValid_f, dsipda))
    {
        if (nhinfo.ecmp_valid)
        {
            return CTC_E_NONE;
        }
        SetDsIpDa(V, dsFwdPtrOrEcmpGroupId_f, dsipda, nhinfo.dsfwd_offset);
    }
    else
    {
        SetDsIpDa(V, adNextHopPtr_f, dsipda, nhinfo.dsnh_offset);
    }
    return CTC_E_NONE;
}

int32
_sys_tsingma_nalpm_move_route(uint8 lchip, uint8 new_lchip, uint8 old_lchip, sys_nalpm_route_info_t* p_route_info)
{
    sys_ipuc_param_t temp_sys_param;
    uint32 cmd = 0;
    sys_nh_info_dsnh_t nh_info;
    DsIpDa_m dsipda;

    sal_memset(&temp_sys_param, 0, sizeof(sys_ipuc_param_t));
    _sys_usw_ipuc_read_ipda(old_lchip, p_route_info->ad_idx, &dsipda);
    temp_sys_param.param.ip_ver = p_route_info->ip_ver;
    temp_sys_param.param.masklen = p_route_info->route_masklen;
    temp_sys_param.param.vrf_id = p_route_info->vrf_id;
    if (p_route_info->ip_ver)
    {
        sal_memcpy(temp_sys_param.param.ip.ipv6, p_route_info->ip, IP_ADDR_SIZE(CTC_IP_VER_6));
    }
    else
    {
        temp_sys_param.param.ip.ipv4 = *(p_route_info->ip);
    }
    SYS_IP_ADDRESS_SORT2(temp_sys_param.param);
    _sys_usw_ipuc_db_lookup(0, &temp_sys_param);

    sys_usw_nh_get_nhinfo(new_lchip, temp_sys_param.info->nh_id, &nh_info, 0);
    _sys_tsingma_nalpm_renew_dsipda(lchip, &dsipda, nh_info);
    _sys_usw_ipuc_add_ad_profile(new_lchip, &temp_sys_param, (void*)&dsipda, 0);
    _sys_usw_ipuc_remove_ad_profile(old_lchip, p_route_info->ad_idx);
    p_route_info->ad_idx = temp_sys_param.info->ad_index;
    cmd = DRV_IOW(DsIpDa_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(new_lchip, temp_sys_param.info->ad_index, cmd, &dsipda);
    temp_sys_param.info->real_lchip = new_lchip;
    return CTC_E_NONE;
}

int32
_sys_tsingma_nalpm_add_split(uint8 lchip, sys_nalpm_tcam_item_t* p_tcam_item, sys_ipuc_param_t* p_sys_ipuc_param)
{
    int ret = CTC_E_NONE;
    uint8 i = 0, j = 0;
    uint32 max_split_len = 0;
    trie_t *prefix_trie = NULL;
    uint32 split_pfx[4];
    uint32 pfx_len = 0;
    trie_node_t* split_trie_root = NULL;
    sys_nalpm_route_info_t*  p_pfx_info= NULL;
    sys_nalpm_route_info_t* p_best_route = NULL;
    sys_nalpm_tcam_item_t* p_new_tcam_item = NULL;
    sys_nalpm_prefix_trie_payload_t* p_prefix_payload = NULL;
    sys_nalpm_prefix_trie_payload_t * p_root_payload = NULL;
    uint32 ad_idx = INVALID_DRV_NEXTHOP_OFFSET;
    uint16 sram_idx = 0;
    uint8  ip_ver = 0;
    uint16 vrf_id = 0;
    uint8  real_lchip = 0;
    uint8 new_tcam_lchip = 0;
    uint16 new_tcam_index = 0;
    uint8 origin_real_lchip = 0;
    uint16 origin_real_index = 0;
    uint8 is_find_in_new = 0;
    sys_nalpm_route_info_t* find_route = NULL;
    trie_t *clone_trie = NULL;
    sys_nalpm_prefix_trie_payload_t* p_old_prefix_payload = NULL;
    ctc_ipuc_param_t* p_ipuc_param = &p_sys_ipuc_param->param;
    uint32 cmd = 0;
    sys_nh_info_dsnh_t nh_info;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    ip_ver = p_ipuc_param->ip_ver;
    vrf_id = p_ipuc_param->vrf_id;


    /*backup trie*/
    CTC_ERROR_RETURN(sys_nalpm_trie_clone(p_tcam_item->trie, &clone_trie));

    max_split_len = ((ip_ver == CTC_IP_VER_4)? SYS_NALPM_V4_MASK_LEN_MAX : SYS_NALPM_V6_MASK_LEN_MAX);
    CTC_ERROR_GOTO(sys_nalpm_trie_split (p_tcam_item->trie, max_split_len, FALSE, split_pfx, &pfx_len, &split_trie_root, NULL, FALSE),ret,error2);
    p_pfx_info = mem_malloc(MEM_IPUC_MODULE, ROUTE_INFO_SIZE(ip_ver));
    if(NULL == p_pfx_info)
    {
        ret =  CTC_E_NO_MEMORY;
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "---[%d]--- \r\n", __LINE__);
        goto error2;
    }
    sal_memset(p_pfx_info, 0, ROUTE_INFO_SIZE(ip_ver));

    if (ip_ver == CTC_IP_VER_4)
    {
        p_pfx_info->ip[0] = split_pfx[0];
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "(%d)    split_pfx [%x], pfx_len[%u]\n", __LINE__, split_pfx[0], pfx_len);
    }
    else
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "(%d)    split_pfx [%x][%x][%x][%x], pfx_len[%u]\n",
                        __LINE__, split_pfx[0], split_pfx[1], split_pfx[2], split_pfx[3], pfx_len);
        p_pfx_info->ip[0] = split_pfx[0];
        p_pfx_info->ip[1] = split_pfx[1];
        p_pfx_info->ip[2] = split_pfx[2];
        p_pfx_info->ip[3] = split_pfx[3];
    }
    p_pfx_info->tcam_masklen = pfx_len;
    p_pfx_info->ip_ver = ip_ver;
    p_pfx_info->vrf_id = vrf_id;

    p_prefix_payload = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_nalpm_prefix_trie_payload_t));
    if(NULL == p_prefix_payload)
    {
        ret =  CTC_E_NO_MEMORY;
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "---[%d]--- \r\n", __LINE__);
        goto error2;
    }
    sal_memset(p_prefix_payload, 0, sizeof(sys_nalpm_prefix_trie_payload_t));
    p_new_tcam_item = &p_prefix_payload->tcam_item;
    p_new_tcam_item->p_prefix_info = p_pfx_info;
    CTC_ERROR_GOTO(_sys_tsingma_nalpm_tcam_idx_alloc(lchip, p_ipuc_param, p_new_tcam_item),ret, error3);
    _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_tcam_item->tcam_idx, &origin_real_lchip, &origin_real_index);
    if(p_usw_ipuc_master[lchip]->rchain_en)
    {
        _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_new_tcam_item->tcam_idx, &new_tcam_lchip, &new_tcam_index);
        CTC_ERROR_GOTO(_sys_tsingma_nalpm_sram_idx_alloc(new_tcam_lchip, ip_ver, &sram_idx), ret, error4);
    }
    else
    {
        CTC_ERROR_GOTO(_sys_tsingma_nalpm_sram_idx_alloc(lchip, ip_ver, &sram_idx), ret, error4);
    }

    p_new_tcam_item->sram_idx = sram_idx;
    CTC_ERROR_GOTO(sys_nalpm_trie_init (max_split_len, &(p_new_tcam_item->trie)),ret, error5);
    p_new_tcam_item->trie->trie = split_trie_root;


    /*add new tcam pfx to prefix trie*/
    prefix_trie = g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrf_id];
    CTC_ERROR_GOTO(sys_nalpm_trie_insert (prefix_trie, split_pfx, NULL, pfx_len, (trie_node_t *) p_prefix_payload, 0),ret, error6);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "(%d)    p_prefix_payload [%p]\n",__LINE__,p_prefix_payload);

    /* write route info to soft table */
    CTC_ERROR_GOTO(_sys_tsingma_nalpm_renew_route_trie(lchip, p_new_tcam_item, ip_ver?p_sys_ipuc_param->param.ip.ipv6:&p_sys_ipuc_param->param.ip.ipv4, p_sys_ipuc_param->param.masklen, &find_route, 1),ret , error6);
    if (find_route)
    {
        is_find_in_new = 1;
    }
    CTC_ERROR_GOTO(_sys_tsingma_nalpm_renew_route_trie(lchip, p_tcam_item, ip_ver?p_sys_ipuc_param->param.ip.ipv6:&p_sys_ipuc_param->param.ip.ipv4, p_sys_ipuc_param->param.masklen, &find_route, 0), ret, error6);

    if (p_usw_ipuc_master[lchip]->rchain_en && origin_real_lchip == new_tcam_lchip)
    {
        sys_usw_nh_get_nhinfo(new_tcam_lchip, p_sys_ipuc_param->param.nh_id, &nh_info, 0);
        _sys_tsingma_nalpm_renew_dsipda(lchip, p_sys_ipuc_param->p_dsipda, nh_info);
        CTC_ERROR_GOTO(_sys_usw_ipuc_add_ad_profile(new_tcam_lchip, p_sys_ipuc_param, p_sys_ipuc_param->p_dsipda, 0), ret, error5);
        cmd = DRV_IOW(DsIpDa_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(new_tcam_lchip, p_sys_ipuc_param->info->ad_index, cmd, p_sys_ipuc_param->p_dsipda);
        find_route->ad_idx = p_sys_ipuc_param->info->ad_index;
    }
    else if (p_usw_ipuc_master[lchip]->rchain_en)
    {
        /* origin_real_lchip != real_lchip */
        for (i = 0; i < SYS_NALPM_MAX_SNAKE; i++)
        {
            for (j = 0; j < ROUTE_NUM_PER_SNAKE_V4; j++)
            {
                if (p_new_tcam_item->p_ipuc_info_array[i][j] == NULL)
                {
                    continue;
                }
                if (ip_ver)
                {
                    if ((p_new_tcam_item->p_ipuc_info_array[i][j]->ip[0] == p_sys_ipuc_param->param.ip.ipv6[3])
                        && (p_new_tcam_item->p_ipuc_info_array[i][j]->ip[1] == p_sys_ipuc_param->param.ip.ipv6[2])
                    && (p_new_tcam_item->p_ipuc_info_array[i][j]->ip[2] == p_sys_ipuc_param->param.ip.ipv6[1])
                    && (p_new_tcam_item->p_ipuc_info_array[i][j]->ip[3] == p_sys_ipuc_param->param.ip.ipv6[0])
                    && (p_new_tcam_item->p_ipuc_info_array[i][j]->route_masklen == p_sys_ipuc_param->param.masklen))
                    {
                        continue;
                    }
                }
                else
                {
                    if (*p_new_tcam_item->p_ipuc_info_array[i][j]->ip == p_sys_ipuc_param->param.ip.ipv4
                        && p_new_tcam_item->p_ipuc_info_array[i][j]->route_masklen == p_sys_ipuc_param->param.masklen)
                    {
                        continue;
                    }
                }
                /* update all other items' ad in new tcam */
                _sys_tsingma_nalpm_move_route(lchip, new_tcam_lchip, origin_real_lchip, p_new_tcam_item->p_ipuc_info_array[i][j]);
            }
        }
        if (is_find_in_new)
        {
            real_lchip = new_tcam_lchip;
        }
        else
        {
            real_lchip = origin_real_lchip;
        }
        sys_usw_nh_get_nhinfo(real_lchip, p_sys_ipuc_param->param.nh_id, &nh_info, 0);
        _sys_tsingma_nalpm_renew_dsipda(lchip, p_sys_ipuc_param->p_dsipda, nh_info);
        CTC_ERROR_GOTO(_sys_usw_ipuc_add_ad_profile(real_lchip, p_sys_ipuc_param, p_sys_ipuc_param->p_dsipda, 0), ret, error5);
        cmd = DRV_IOW(DsIpDa_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(real_lchip, p_sys_ipuc_param->info->ad_index, cmd, p_sys_ipuc_param->p_dsipda);
        find_route->ad_idx = p_sys_ipuc_param->info->ad_index;
    }

    p_old_prefix_payload = CONTAINER_OF(p_tcam_item, sys_nalpm_prefix_trie_payload_t, tcam_item);
    CTC_ERROR_GOTO(_sys_tsingma_nalpm_search_best_route_in_sram(lchip, &(p_old_prefix_payload->node), p_new_tcam_item->p_prefix_info, &p_best_route), ret, error6);
    if (p_best_route)
    {
        if (!p_usw_ipuc_master[lchip]->rchain_en || origin_real_lchip == new_tcam_lchip)
        {
            p_new_tcam_item->p_prefix_info->ad_idx = p_best_route->ad_idx;
        }
        else
        {
            uint32 new_ad_index = 0;
            uint32 nhid = 0;
            if (p_best_route == find_route)
            {
                nhid = p_sys_ipuc_param->param.nh_id;
            }
            else
            {
                nhid = 0;
            }
            CTC_ERROR_GOTO(_sys_tsingma_nalpm_update_ad(new_tcam_lchip, origin_real_lchip, p_best_route, 1, nhid, &new_ad_index), ret, error6);
            p_new_tcam_item->p_prefix_info->ad_idx = new_ad_index;
        }
    }
    else
    {
        p_root_payload = CONTAINER_OF(prefix_trie->trie, sys_nalpm_prefix_trie_payload_t, node);
        CTC_ERROR_GOTO(_sys_tsingma_nalpm_read_tcam(lchip, &p_root_payload->tcam_item, &ad_idx), ret, error6);
        if (!p_usw_ipuc_master[lchip]->rchain_en || 0x3FFFF == ad_idx)
        {
            p_new_tcam_item->p_prefix_info->ad_idx = ad_idx;
        }
        else
        {
            uint32 new_ad_index = 0;
            sys_nalpm_route_info_t *p_route_info;
            p_route_info = mem_malloc(MEM_IPUC_MODULE, ROUTE_INFO_SIZE(ip_ver));
            if (!p_route_info)
            {
                return CTC_E_NO_MEMORY;
            }
            sal_memset(p_route_info, 0, ROUTE_INFO_SIZE(ip_ver));
            p_route_info->ad_idx = ad_idx;
            p_route_info->ip_ver = ip_ver;
            p_route_info->route_masklen = 0;
            p_route_info->vrf_id = p_tcam_item->p_prefix_info->vrf_id;
            ret = _sys_tsingma_nalpm_update_ad(new_tcam_lchip, origin_real_lchip, p_route_info, 1, 0, &new_ad_index);
            if (ret)
            {
                mem_free(p_route_info);
                goto error6;
            }
            p_new_tcam_item->p_prefix_info->ad_idx = new_ad_index;
            mem_free(p_route_info);
        }
    }
    p_new_tcam_item->p_AD_route = p_best_route;

    /*install to hw*/
    CTC_ERROR_GOTO(_sys_tsingma_nalpm_write_route_trie(lchip, p_new_tcam_item, 1), ret, error6);
    CTC_ERROR_GOTO(_sys_tsingma_nalpm_write_tcam(lchip, p_new_tcam_item, DO_ADD, 0),ret, error6);

    CTC_ERROR_GOTO(_sys_tsingma_nalpm_write_route_trie(lchip, p_tcam_item, 0), ret, error6);

    g_sys_nalpm_master[lchip]->vrf_route_cnt[ip_ver][vrf_id]++;
    if (p_usw_ipuc_master[lchip]->rchain_en && origin_real_lchip != new_tcam_lchip)
    {
        uint8 new_cnt = 0;
        _sys_tsingma_nalpm_get_route_num_per_tcam(0, p_new_tcam_item, &new_cnt);
        *(p_usw_ipuc_master[0]->rchain_stats + origin_real_lchip) = *(p_usw_ipuc_master[lchip]->rchain_stats + origin_real_lchip) + 1 - new_cnt;
        *(p_usw_ipuc_master[0]->rchain_stats + new_tcam_lchip) += new_cnt;
        if (is_find_in_new)
        {
            p_sys_ipuc_param->info->real_lchip = new_tcam_lchip;
        }
        else
        {
            p_sys_ipuc_param->info->real_lchip = origin_real_lchip;
        }
    }
    else if(p_usw_ipuc_master[lchip]->rchain_en)
    {
        *(p_usw_ipuc_master[0]->rchain_stats + new_tcam_lchip) += 1;
        p_sys_ipuc_param->info->real_lchip = new_tcam_lchip;
    }

    sys_nalpm_trie_clear(clone_trie->trie);
    sys_nalpm_trie_destroy(clone_trie);
    return ret;
error6:
    sys_nalpm_trie_destroy((p_new_tcam_item->trie));
error5:
    if(p_usw_ipuc_master[lchip]->rchain_en)
    {
        _sys_tsingma_nalpm_sram_idx_free(new_tcam_lchip, sram_idx);
        _sys_usw_ipuc_unbuild_ipda(lchip, p_sys_ipuc_param);
    }
    else
    {
        _sys_tsingma_nalpm_sram_idx_free(lchip, sram_idx);
    }
error4:
    _sys_tsingma_nalpm_tcam_idx_free(lchip, p_ipuc_param, p_new_tcam_item);
error3:
    if(p_prefix_payload)
         mem_free(p_prefix_payload);
error2:
    sys_nalpm_trie_clear(p_tcam_item->trie->trie);
    sys_nalpm_trie_clear(split_trie_root);
    sys_nalpm_trie_destroy(p_tcam_item->trie);
    p_tcam_item->trie = clone_trie;
    if(p_pfx_info)
        mem_free(p_pfx_info);

    return ret;
}

int32
_sys_tsingma_nalpm_search_tcam_item(trie_node_t * p_node, void * user_data)
{
    sys_nalpm_prefix_trie_payload_t* p_payload = (sys_nalpm_prefix_trie_payload_t* )p_node;
    sys_nalpm_match_tcam_idx_t * p_match_tcam = user_data;

    if (p_node->type == INTERNAL)
    {
         return CTC_E_NONE;
    }

    if(p_payload->tcam_item.tcam_idx == p_match_tcam->tcam_idx)
    {
        p_match_tcam->p_tcam_item = &p_payload->tcam_item;
    }

    if(!_sys_tsingma_nalpm_route_cmp(p_payload->tcam_item.p_AD_route,p_match_tcam->p_new_route))
    {
        if(NULL == p_match_tcam->p_AD_route)
        {
            p_match_tcam->p_AD_route = p_payload->tcam_item.p_AD_route;
        }
        else
        {
            mem_free(p_payload->tcam_item.p_AD_route);
            p_payload->tcam_item.p_AD_route = p_match_tcam->p_AD_route;
        }
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_nalpm_auto_merge(uint8 lchip, uint8 enable)
{
    SYS_NALPM_INIT_CHECK;
    g_sys_nalpm_master[lchip]->frag_arrange_enable = enable;
    return CTC_E_NONE;
}

int32
sys_tsingma_nalpm_set_fragment_status(uint8 lchip, uint8 ip_ver, uint8 status)
{
    SYS_NALPM_INIT_CHECK;
    g_sys_nalpm_master[lchip]->frag_arrange_status[ip_ver] = status;
    return CTC_E_NONE;
}

int32
sys_tsingma_nalpm_get_fragment_status(uint8 lchip, uint8 ip_ver, uint8* status)
{
    SYS_NALPM_INIT_CHECK;
    *status = g_sys_nalpm_master[lchip]->frag_arrange_status[ip_ver];
    return CTC_E_NONE;
}

int32
sys_tsingma_nalpm_get_fragment_auto_enable(uint8 lchip, uint8* enable)
{
    SYS_NALPM_INIT_CHECK;
    *enable = g_sys_nalpm_master[lchip]->frag_arrange_enable;
    return CTC_E_NONE;
}


typedef enum _alpm_cb_merge_type_e {
    ACB_MERGE_INVALID = 0,
    ACB_MERGE_CHILD_TO_PARENT,
    ACB_MERGE_PARENT_TO_CHILD
} _alpm_cb_merge_type_t;

#define ACB_REPART_THRESHOLD 8
#define ACB_MERGE_THRESHOLD  8

uint8 _alpm_merge_state[128] = {0};

#define _ALPM_MERGE_CHANGE      0
#define _ALPM_MERGE_UNCHANGE    1


#define ALPM_MERGE_REQD(lchip, vrf_id)              \
        (_alpm_merge_state[vrf_id] != _ALPM_MERGE_UNCHANGE)
#define ALPM_MERGE_STATE_CHKED(lchip, vrf_id)        \
         (_alpm_merge_state[vrf_id] = _ALPM_MERGE_UNCHANGE)
#define ALPM_MERGE_STATE_CHANGED(lchip, vrf_id)     \
        (_alpm_merge_state[vrf_id] = _ALPM_MERGE_CHANGE)


typedef sys_nalpm_prefix_trie_payload_t sys_alpm_pvt_node_t;

#define ALPM_PVT_TRIE(lchip, vrf_id, ip_ver) g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrf_id]
#define PVT_BKT_TRIE(pvt_node)          ((pvt_node)->tcam_item.trie)

typedef struct _alpm_cb_merge_ctrl_s {
    uint8 lchip;
    uint32 merge_count;
    uint32 vrf_id;
    uint8 ip_ver;
    uint32 bnk_pbkt;
    uint32 ent_pbnk;

    uint32 max_cnt;
}sys_alpm_cb_merge_ctrl_t;



int32
sys_tsingma_nalpm_get_snake_sram_used(uint8 lchip,
                         uint8 ip_ver,
                         uint32 vrf_id,
                         sys_alpm_pvt_node_t *pvt_node)
{
    uint32 used_cnt = 0;
    uint8 i = 0, j =0;
    uint8 snake_num = 0;
    uint8 snake_route_num = 0;
    sys_nalpm_tcam_item_t* p_tcam_item = NULL;

    p_tcam_item = &pvt_node->tcam_item;

    _sys_tsingma_nalpm_get_snake_num(lchip, ip_ver, &snake_num);

    if(CTC_IP_VER_4 == ip_ver)
    {
        snake_route_num = ROUTE_NUM_PER_SNAKE_V4;
    }

    for(i = 0; i < snake_num; i++)
    {
        if (CTC_IP_VER_6 == ip_ver)
        {
            if (SYS_NALPM_SRAM_TYPE_VOID == p_tcam_item->sram_type[i])
            {
                continue;
            }
            snake_route_num = (SYS_NALPM_SRAM_TYPE_V6_64 == p_tcam_item->sram_type[i]) ?
            ROUTE_NUM_PER_SNAKE_V6_64 : ROUTE_NUM_PER_SNAKE_V6_128;
        }

        for (j = 0; j < snake_route_num; j++)
        {
            if (p_tcam_item->p_ipuc_info_array[i][j])
            {
                used_cnt++;
            }
        }
    }


    return used_cnt;
}


STATIC int32
_sys_tsingma_remove_pvt_resource(uint8 lchip, sys_alpm_pvt_node_t *pvt_node)
{
    ctc_ipuc_param_t ipuc_param;
    uint8 real_lchip = 0;
    uint16 real_index = 0;

    sal_memset(&ipuc_param, 0, sizeof(ipuc_param));
    ipuc_param.ip_ver = pvt_node->tcam_item.p_prefix_info->ip_ver;

    _sys_tsingma_nalpm_get_real_tcam_index(lchip, pvt_node->tcam_item.tcam_idx, &real_lchip, &real_index);

    CTC_ERROR_RETURN(_sys_tsingma_nalpm_write_tcam(lchip, &pvt_node->tcam_item, DO_DEL, 0));
    CTC_ERROR_RETURN(_sys_tsingma_nalpm_tcam_idx_free(lchip, &ipuc_param, &pvt_node->tcam_item));
    CTC_ERROR_RETURN(_sys_tsingma_nalpm_sram_idx_free(real_lchip, pvt_node->tcam_item.sram_idx));

    return 0;
}

STATIC int32
_sys_tsingma_nalpm_merge_pvt(uint8 lchip,
                  sys_alpm_pvt_node_t *src_pvt,
                  sys_alpm_pvt_node_t *dst_pvt,
                  uint32 merge_dir)
{
    int32 rv = CTC_E_NONE;
    uint8 ip_ver = 0;
    uint32 vrf_id = 0;
    uint32 key_len = 0;
    trie_node_t *tn = NULL;
    sys_alpm_pvt_node_t *prt_pvt = NULL;
    sys_alpm_pvt_node_t *chd_pvt = NULL;
    sys_nalpm_tcam_item_t tcam_item;
    trie_t *clone_prt_trie = NULL;
    trie_t *clone_chd_trie = NULL;

    uint32 key[4] = {0};

    if (merge_dir == ACB_MERGE_PARENT_TO_CHILD)
    {
        prt_pvt = src_pvt;
        chd_pvt = dst_pvt;
    }
    else
    {
        prt_pvt = dst_pvt;
        chd_pvt = src_pvt;
    }

    /* 3. Merge trie */
    ip_ver = chd_pvt->tcam_item.p_prefix_info->ip_ver;
    vrf_id = chd_pvt->tcam_item.p_prefix_info->vrf_id;

    sal_memset(key, 0 , sizeof(key));
    sal_memset(&tcam_item, 0, sizeof(tcam_item));
    sal_memcpy(key, chd_pvt->tcam_item.p_prefix_info->ip, IP_ADDR_SIZE(ip_ver));
    sal_memcpy(&tcam_item, &chd_pvt->tcam_item, sizeof(tcam_item));

    CTC_ERROR_RETURN(sys_nalpm_trie_clone(PVT_BKT_TRIE(prt_pvt), &clone_prt_trie));
    CTC_ERROR_RETURN(sys_nalpm_trie_clone(PVT_BKT_TRIE(chd_pvt), &clone_chd_trie));

    key_len = chd_pvt->tcam_item.p_prefix_info->tcam_masklen;
    CTC_ERROR_GOTO(trie_merge(PVT_BKT_TRIE(prt_pvt), PVT_BKT_TRIE(chd_pvt)->trie,  key, key_len), rv, error1);


    if (merge_dir == ACB_MERGE_PARENT_TO_CHILD)
    {
        /*更新lpm parent 前缀信息，更新tcam*/
    }

    /*update new snake sram entry*/
    rv = _sys_tsingma_nalpm_renew_route_trie(lchip, &prt_pvt->tcam_item, NULL, 0, NULL, 1);
    if (rv < 0)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_tsingma_nalpm_renew_route_trie failed, rv=%d\n", rv);
    }
    _sys_tsingma_nalpm_write_route_trie(lchip, &prt_pvt->tcam_item, 0);

    /* 6.remove prefix  resource*/
    rv = _sys_tsingma_remove_pvt_resource(lchip, chd_pvt);
    if (rv < 0)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "alpm_remove_pvt_node failed, rv=%d\n", rv);
    }
    /* 7. Destroy child pvt from trie, tn is chd_pvt */
    rv = trie_delete(ALPM_PVT_TRIE(lchip, vrf_id, ip_ver), key, key_len, &tn);
    if (rv < 0)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "trie_delete failed, rv=%d\n", rv);
    }


    /*remove source trie*/


    sys_nalpm_trie_destroy(PVT_BKT_TRIE(chd_pvt));
    mem_free(chd_pvt->tcam_item.p_prefix_info);
    mem_free(chd_pvt);

    sys_nalpm_trie_clear(clone_prt_trie->trie);
    sys_nalpm_trie_clear(clone_chd_trie->trie);
    sys_nalpm_trie_destroy(clone_prt_trie);
    sys_nalpm_trie_destroy(clone_chd_trie);
    return rv;
error1:
    sys_nalpm_trie_clear(prt_pvt->tcam_item.trie->trie);
    sys_nalpm_trie_destroy(prt_pvt->tcam_item.trie);
    sys_nalpm_trie_clear(chd_pvt->tcam_item.trie->trie);
    sys_nalpm_trie_destroy(chd_pvt->tcam_item.trie);
    prt_pvt->tcam_item.trie = clone_prt_trie;
    chd_pvt->tcam_item.trie = clone_prt_trie;
    return rv;
}


STATIC int32
sys_tsingma_nalpm_merge_cb(trie_node_t *ptrie, trie_node_t *trie,
                 trie_traverse_states_e_t *state, void *info)
{
    int32 rv = 0;
    uint32 lchip = 0;
    uint32 bnkpb = 0;
    uint32 prt_entpb = 0;
    uint32 chd_entpb = 0;
    uint32 vrf_id = 0;
    uint8 ip_ver = 0;
    uint8 prt_real_lchip = 0;
    uint8 chd_real_lchip = 0;
    uint16 real_tcam_index = 0;

    sys_alpm_pvt_node_t *prt_pvt, *chd_pvt, *src_pvt, *dst_pvt;
    sys_alpm_cb_merge_ctrl_t *merge_ctrl;
    _alpm_cb_merge_type_t merge_dir = ACB_MERGE_INVALID;

    prt_pvt = (sys_alpm_pvt_node_t *)ptrie;
    chd_pvt = (sys_alpm_pvt_node_t *)trie;
    if (!prt_pvt)
    {
        return CTC_E_NONE;
    }
    if (p_usw_ipuc_master[lchip]->rchain_en)
    {
        _sys_tsingma_nalpm_get_real_tcam_index(lchip, (prt_pvt->tcam_item.tcam_idx), &prt_real_lchip, &real_tcam_index);
        _sys_tsingma_nalpm_get_real_tcam_index(lchip, (chd_pvt->tcam_item.tcam_idx), &chd_real_lchip, &real_tcam_index);
    }
    if ((prt_real_lchip != chd_real_lchip) && p_usw_ipuc_master[lchip]->rchain_en)
    {
        return CTC_E_NONE;
    }

    if (PVT_BKT_TRIE(prt_pvt)->trie == NULL)
    {
        merge_dir = ACB_MERGE_PARENT_TO_CHILD;
    }

    if (PVT_BKT_TRIE(chd_pvt)->trie == NULL)
    {
        merge_dir = ACB_MERGE_CHILD_TO_PARENT;
    }

    merge_ctrl = (sys_alpm_cb_merge_ctrl_t *)info;
    lchip   = merge_ctrl->lchip;
    ip_ver = merge_ctrl->ip_ver;
    vrf_id = merge_ctrl->vrf_id;
    bnkpb  = merge_ctrl->bnk_pbkt;/*一个bucket的bank数*/
    /*一个bank的条目数*/
    if(ip_ver)
    {
        prt_entpb = (prt_pvt->tcam_item.sram_type[0] == SYS_NALPM_SRAM_TYPE_V6_64) ? ROUTE_NUM_PER_SNAKE_V6_64 : ROUTE_NUM_PER_SNAKE_V6_128;
        chd_entpb = (chd_pvt->tcam_item.sram_type[0] == SYS_NALPM_SRAM_TYPE_V6_64) ? ROUTE_NUM_PER_SNAKE_V6_64 : ROUTE_NUM_PER_SNAKE_V6_128;
    }
    else
    {
        prt_entpb = ROUTE_NUM_PER_SNAKE_V4;
        chd_entpb = ROUTE_NUM_PER_SNAKE_V4;
    }

    if (merge_dir == ACB_MERGE_INVALID)
    {
        uint32 prt_entrys = 0;
        uint32 chd_entrys = 0;


        prt_entrys = bnkpb*prt_entpb - sys_tsingma_nalpm_get_snake_sram_used(lchip, ip_ver, vrf_id, prt_pvt); /*父结点 空的bank数量*/
        chd_entrys = bnkpb*chd_entpb - sys_tsingma_nalpm_get_snake_sram_used(lchip, ip_ver, vrf_id, chd_pvt);/*子结点 空的bank数量*/


        if (PVT_BKT_TRIE(chd_pvt)->trie->count <= prt_entrys)
        {
            merge_dir   = ACB_MERGE_CHILD_TO_PARENT;
        }
        else if (PVT_BKT_TRIE(prt_pvt)->trie->count <= chd_entrys)
        {
            merge_dir   = ACB_MERGE_PARENT_TO_CHILD;
        }
        else
        {
            return CTC_E_NONE;
        }

    }

    if (merge_dir == ACB_MERGE_PARENT_TO_CHILD)
    {
        src_pvt = prt_pvt;
        dst_pvt = chd_pvt;
    }
    else
    {
        src_pvt = chd_pvt;
        dst_pvt = prt_pvt;
    }

    rv = _sys_tsingma_nalpm_merge_pvt(lchip, src_pvt, dst_pvt, merge_dir);

    if (rv < 0)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "alpm merge failed rv %d\n", rv);
        return rv;
    }

    *state = TRIE_TRAVERSE_STATE_DELETED;

    ++merge_ctrl->merge_count;

    return CTC_E_NONE;

}



int32
sys_tsingma_nalpm_merge(uint8 lchip, uint32 vrf_id, uint8 ip_ver)
{
    uint32 rv = CTC_E_NONE;
    trie_t *pvt_trie;
    sys_alpm_cb_merge_ctrl_t merge_ctrl;

    LCHIP_CHECK(lchip);
    CTC_IP_VER_CHECK(ip_ver);
    SYS_IP_VRFID_CHECK(vrf_id, ip_ver);

    SYS_NALPM_INIT_CHECK;


    pvt_trie = ALPM_PVT_TRIE(lchip, vrf_id, ip_ver);

    if (!ALPM_MERGE_REQD(lchip, vrf_id) ||
        pvt_trie == NULL ||
    pvt_trie->trie == NULL || MERGE_NO_RESOURCE == g_sys_nalpm_master[lchip]->frag_arrange_status[ip_ver])
    {
        return CTC_E_NONE;
    }

    sal_memset(&merge_ctrl, 0, sizeof(merge_ctrl));
    merge_ctrl.lchip     = lchip;
    merge_ctrl.vrf_id   = vrf_id;
    merge_ctrl.ip_ver   = ip_ver;
    merge_ctrl.bnk_pbkt = 2;
    /* can not be sure ent_pbnk because ipv6 will be 3 or 2 */
    merge_ctrl.ent_pbnk = (ip_ver == CTC_IP_VER_4)?6:3;

    rv = sys_nalpm_trie_traverse2(pvt_trie, sys_tsingma_nalpm_merge_cb, &merge_ctrl,  _TRIE_POSTORDER_TRAVERSE);
    if (rv < 0)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "Logic bucket merge failed, ip_ver %d vrf_id %d rv %d.\n", ip_ver, vrf_id, rv);
    }

    if (merge_ctrl.merge_count == 0)
    {
        rv = CTC_E_NO_RESOURCE;
    }

    return rv;
}



int32
sys_tsingma_nalpm_add(uint8 lchip, sys_ipuc_param_t* p_sys_ipuc_param, uint32 ad_index, void* data)
{
    sys_nalpm_route_info_t*  p_route_info = NULL;
    sys_nalpm_route_store_info_t lkp_rlt;
    sys_nalpm_route_store_info_t *p_lkp_rlt = &lkp_rlt;
    sys_nalpm_tcam_item_t* p_tcam_item = NULL;
    sys_nalpm_trie_payload_t* p_route_payload = NULL;
    sys_wb_nalpm_info_t* p_wb_nalpm_info = (sys_wb_nalpm_info_t*)data;
    ctc_ipuc_param_t* p_ipuc_param= &p_sys_ipuc_param->param;
    uint8 ip_ver = 0;
    int32 ret = CTC_E_NONE;
    uint32 key[4] = {0};
    uint8 real_lchip = 0;
    uint16 real_index = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    /* prepare data */
    ip_ver = p_ipuc_param->ip_ver;
    p_route_info = mem_malloc(MEM_IPUC_MODULE, ROUTE_INFO_SIZE(ip_ver));
    if(NULL == p_route_info)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_route_info, 0 , (ROUTE_INFO_SIZE(ip_ver)));
    p_route_info->ip_ver = ip_ver;
    sal_memcpy(p_route_info->ip, &p_ipuc_param->ip, IP_ADDR_SIZE(ip_ver));
    p_route_info->ad_idx = ad_index;
    p_route_info->route_masklen = p_ipuc_param->masklen;
    p_route_info->vrf_id = p_ipuc_param->vrf_id;
    SYS_IP_V6_SORT(p_route_info);

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        sys_nalpm_match_tcam_idx_t match_tcam;
        if(NULL == g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][p_route_info->vrf_id])
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "---[%d]---prefix_trie = NULL, ip_ver=[%u] vrfid =[%u],\r\n",__LINE__, ip_ver,p_route_info->vrf_id);
            ret = CTC_E_INVALID_PARAM;
            goto error1;
        }

        if(p_ipuc_param->masklen== 0)
        {
            g_sys_nalpm_master[lchip]->vrf_route_cnt[ip_ver][p_route_info->vrf_id]++;
            mem_free(p_route_info);
            return ret;
        }
        sal_memset(&match_tcam, 0 , sizeof(sys_nalpm_match_tcam_idx_t));
        match_tcam.tcam_idx = p_wb_nalpm_info->tcam_idx;
        match_tcam.p_new_route = p_route_info;
        CTC_ERROR_GOTO(sys_nalpm_trie_traverse(g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][p_route_info->vrf_id]->trie,
                                        _sys_tsingma_nalpm_search_tcam_item,(void *)&match_tcam,
                                        _TRIE_PREORDER_TRAVERSE),ret, error1);
        if(match_tcam.p_AD_route)
        {
            mem_free(p_route_info);
            p_route_info = match_tcam.p_AD_route;
        }
        p_tcam_item = match_tcam.p_tcam_item;
        p_route_payload = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_nalpm_trie_payload_t));
        if (NULL == p_route_payload)
        {
            ret = CTC_E_NO_MEMORY;
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "---[%d]--- \r\n", __LINE__);
            goto error1;
        }
        sal_memset(p_route_payload, 0 , sizeof(sys_nalpm_trie_payload_t));
        p_route_payload->p_route_info = p_route_info;
        sal_memcpy(key, p_route_info->ip, IP_ADDR_SIZE(p_route_info->ip_ver));

        ret = sys_nalpm_trie_insert (p_tcam_item->trie, key, NULL, p_route_info->route_masklen, (trie_node_t *) p_route_payload, 1);
        if (CTC_E_NONE != ret)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "!!!trie_insert failed: %d\r\n", ret);
            goto error2;
        }
        if(p_tcam_item->sram_type[p_wb_nalpm_info->snake_idx] == SYS_NALPM_SRAM_TYPE_VOID)
        {
            if(ip_ver == CTC_IP_VER_4)
            {
                p_tcam_item->sram_type[p_wb_nalpm_info->snake_idx] = SYS_NALPM_SRAM_TYPE_V4_32;
            }
            else if (p_route_info->route_masklen <= 64 && CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[CTC_IP_VER_6], SYS_TCAM_SHORT_LOOKUP))
            {
                p_tcam_item->sram_type[p_wb_nalpm_info->snake_idx] = SYS_NALPM_SRAM_TYPE_V6_64;
            }
            else
            {
                p_tcam_item->sram_type[p_wb_nalpm_info->snake_idx] = SYS_NALPM_SRAM_TYPE_V6_128;
            }
        }
        else
        {
            if(NULL != p_tcam_item->p_ipuc_info_array[p_wb_nalpm_info->snake_idx][p_wb_nalpm_info->entry_offset]
                || (p_tcam_item->sram_type[p_wb_nalpm_info->snake_idx]== SYS_NALPM_SRAM_TYPE_V6_64 && p_route_info->route_masklen > 64))
            {
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "---[%d]--- WB sram_type error\r\n", __LINE__);
                goto error2;
            }
        }
        p_tcam_item->p_ipuc_info_array[p_wb_nalpm_info->snake_idx][p_wb_nalpm_info->entry_offset] = p_route_info;
        g_sys_nalpm_master[lchip]->vrf_route_cnt[p_route_info->ip_ver][p_route_info->vrf_id]++;
    }
    else
    {
         /* vrf is not inited*/
        if(g_sys_nalpm_master[lchip]->vrf_route_cnt[ip_ver][p_route_info->vrf_id] == 0)
        {
             /* init vrf memory and default route*/
            CTC_ERROR_GOTO(_sys_tsingma_nalpm_route_vrf_init(lchip, p_ipuc_param),ret, error1);
        }

        if(p_ipuc_param->masklen== 0)
        {
            CTC_ERROR_GOTO(sys_tsingma_nalpm_update( lchip, p_sys_ipuc_param, ad_index),ret, error1);
            g_sys_nalpm_master[lchip]->vrf_route_cnt[ip_ver][p_route_info->vrf_id]++;
            mem_free(p_route_info);
            return ret;
        }

        /* 1. lookup sw node */
        sal_memset(p_lkp_rlt, 0 , sizeof(sys_nalpm_route_store_info_t));
        CTC_ERROR_GOTO(_sys_tsingma_nalpm_route_lkp(lchip, p_route_info, p_lkp_rlt), ret, error1);
        if (!p_lkp_rlt->tcam_hit || p_lkp_rlt->p_tcam_item == NULL)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "---[%d]--- tcam not hit\r\n", __LINE__);
            ret =  CTC_E_NOT_EXIST;
            goto error1;
        }
        if (p_lkp_rlt->sram_hit)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "---[%d]--- sram hit\r\n", __LINE__);
            ret =  CTC_E_EXIST;
            goto error1;
        }
        p_tcam_item = p_lkp_rlt->p_tcam_item;
        /* should not split tree when arrange fragment, this will lead to use more tcam resource */
        if(_sys_tsingma_nalpm_sram_entry_is_full(lchip, p_tcam_item, p_route_info) && (uint8*)data)
        {
            ret = CTC_E_NOT_SUPPORT;
            goto error1;
        }

        p_route_payload = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_nalpm_trie_payload_t));
        if (NULL == p_route_payload)
        {
            ret = CTC_E_NO_MEMORY;
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "---[%d]--- \r\n", __LINE__);
            goto error1;
        }
        sal_memset(p_route_payload, 0 , sizeof(sys_nalpm_trie_payload_t));
        p_route_payload->p_route_info = p_route_info;
        sal_memcpy(key, p_route_info->ip, IP_ADDR_SIZE(p_route_info->ip_ver));

        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "(%d)  p_route_payload=[%p],p_route_info=[%p], ip_ver[%u]\n", __LINE__, p_route_payload, p_route_info, p_route_info->ip_ver);



        ret = sys_nalpm_trie_insert (p_tcam_item->trie, key, NULL, p_route_info->route_masklen, (trie_node_t *) p_route_payload, 1);
        if (CTC_E_NONE != ret)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "!!!trie_insert failed: %d\r\n", ret);
            goto error2;
        }

        if (!_sys_tsingma_nalpm_sram_entry_is_full(lchip, p_tcam_item, p_route_info))
        {
            uint32 cmd = 0;
            if (p_usw_ipuc_master[lchip]->rchain_en)
            {
                _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_tcam_item->tcam_idx, &real_lchip, &real_index);
                if (0xff == real_lchip)
                {
                    return CTC_E_INVALID_PARAM;
                }
                if (real_lchip)
                {
                    sys_nh_info_dsnh_t nh_info;
                    sys_usw_nh_get_nhinfo(real_lchip, p_sys_ipuc_param->param.nh_id, &nh_info, 0);
                    _sys_tsingma_nalpm_renew_dsipda(lchip, p_sys_ipuc_param->p_dsipda, nh_info);
                }
                ret = _sys_usw_ipuc_add_ad_profile(real_lchip, p_sys_ipuc_param, p_sys_ipuc_param->p_dsipda, 0);
                if (ret)
                {
                    _sys_usw_ipuc_bind_nexthop(0, p_sys_ipuc_param, 0);
                    return ret;
                }
                cmd = DRV_IOW(DsIpDa_t, DRV_ENTRY_FLAG);
                DRV_IOCTL(real_lchip, p_sys_ipuc_param->info->ad_index, cmd, p_sys_ipuc_param->p_dsipda);
                p_route_info->ad_idx = p_sys_ipuc_param->info->ad_index;
            }
            CTC_ERROR_GOTO(_sys_tsingma_nalpm_add_directly(lchip, p_tcam_item, p_route_info, p_lkp_rlt, p_sys_ipuc_param->param.nh_id), ret, error4);
            if(p_usw_ipuc_master[lchip]->rchain_en)
            {
                *(p_usw_ipuc_master[0]->rchain_stats + real_lchip) += 1;
                p_sys_ipuc_param->info->real_lchip = real_lchip;
            }
        }
        else
        {
            CTC_ERROR_GOTO(_sys_tsingma_nalpm_add_split(lchip, p_tcam_item, p_sys_ipuc_param), ret, error3);
        }
    }

    return ret;

error4:
    if(p_usw_ipuc_master[lchip]->rchain_en)
    {
        _sys_usw_ipuc_unbuild_ipda(lchip, (p_sys_ipuc_param));
    }
error3:
    sys_nalpm_trie_delete (p_tcam_item->trie, key, p_route_info->route_masklen, (trie_node_t **) &p_route_payload, 0);
error2:
    if(p_route_payload)
        mem_free(p_route_payload);
error1:
    if(p_route_info)
        mem_free(p_route_info);

    return ret;

}

int32
sys_tsingma_nalpm_del(uint8 lchip, sys_ipuc_param_t* p_sys_ipuc_param)
{
    ctc_ipuc_param_t* p_ipuc_param = &p_sys_ipuc_param->param;
    sys_nalpm_route_info_t*  p_route_info = NULL;
    sys_nalpm_route_store_info_t lkp_rlt;
    sys_nalpm_route_store_info_t *p_lkp_rlt = &lkp_rlt;
    sys_nalpm_trie_payload_t* p_route_payload = NULL;
    sys_nalpm_prefix_trie_payload_t* p_pfx_payload = NULL;
    uint32 pfx_len = 0;
    uint32 key[4] = {0};
    uint8 ip_ver = 0;
    uint16 vrf_id = 0;
    int32 ret = CTC_E_NONE;
    uint8 i = 0, j = 0;
    uint8 snake_num = 0;
    uint8 snake_route_num = 0;
    uint8 route_cnt = 0;
    uint8 real_lchip = 0;
    uint16 real_index = 0;


    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    ip_ver = p_ipuc_param->ip_ver;
    vrf_id = p_ipuc_param->vrf_id;

    if(p_ipuc_param->masklen== 0)
    {
        if(NULL == g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][p_ipuc_param->vrf_id])
        {
            return CTC_E_NOT_EXIST;
        }
        CTC_ERROR_RETURN(sys_tsingma_nalpm_update( lchip, p_sys_ipuc_param, INVALID_DRV_NEXTHOP_OFFSET));
        g_sys_nalpm_master[lchip]->vrf_route_cnt[ip_ver][vrf_id]--;
        if(g_sys_nalpm_master[lchip]->vrf_route_cnt[ip_ver][vrf_id] == 0)
        {
            CTC_ERROR_RETURN( _sys_tsingma_nalpm_route_vrf_deinit(lchip, p_ipuc_param));
        }
        return ret;
    }
    /* prepare data */
    p_route_info = mem_malloc(MEM_IPUC_MODULE, ROUTE_INFO_SIZE(ip_ver));
    if(NULL == p_route_info)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_route_info, 0 , (ROUTE_INFO_SIZE(ip_ver)));
    p_route_info->ip_ver = ip_ver;
    sal_memcpy(p_route_info->ip, &p_ipuc_param->ip, IP_ADDR_SIZE(ip_ver));
    p_route_info->route_masklen = p_ipuc_param->masklen;
    p_route_info->vrf_id = p_ipuc_param->vrf_id;
    SYS_IP_V6_SORT(p_route_info);

    /* 1. lookup sw node */
    ret = _sys_tsingma_nalpm_route_lkp( lchip, p_route_info,p_lkp_rlt);
    mem_free(p_route_info);
    if(ret < 0 || !p_lkp_rlt->sram_hit)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "can't find  route\n");
        return CTC_E_NOT_EXIST;
    }

    p_route_info = p_lkp_rlt->p_tcam_item->p_ipuc_info_array[p_lkp_rlt->snake_idx][p_lkp_rlt->entry_offset];
     /* del route*/
    sal_memcpy(key , p_route_info->ip, IP_ADDR_SIZE(ip_ver));

    pfx_len = p_route_info->route_masklen;

    CTC_ERROR_RETURN(sys_nalpm_trie_delete (p_lkp_rlt->p_tcam_item->trie, key, pfx_len, (trie_node_t **) &p_route_payload, 1));

    CTC_ERROR_RETURN(_sys_nalpm_clear_sram(lchip, p_lkp_rlt->p_tcam_item, p_lkp_rlt->snake_idx, p_lkp_rlt->entry_offset, 1));

    g_sys_nalpm_master[lchip]->vrf_route_cnt[ip_ver][vrf_id]--;
    p_lkp_rlt->p_tcam_item->p_ipuc_info_array[p_lkp_rlt->snake_idx][p_lkp_rlt->entry_offset] = NULL;

    CTC_ERROR_RETURN(_sys_tsingma_nalpm_deletes_route_cover(lchip, p_lkp_rlt->p_tcam_item, p_route_payload->p_route_info));
    mem_free(p_route_payload->p_route_info);
    mem_free(p_route_payload);

    if (p_usw_ipuc_master[lchip]->rchain_en)
    {
        _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_lkp_rlt->p_tcam_item->tcam_idx, &real_lchip, &real_index);
    }

    if (!p_lkp_rlt->p_tcam_item->trie->trie)  /* sram entry is empty, release tcam*/
    {
        if(CONTAINER_OF(g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrf_id]->trie, sys_nalpm_prefix_trie_payload_t, node)
            != CONTAINER_OF(p_lkp_rlt->p_tcam_item, sys_nalpm_prefix_trie_payload_t, tcam_item)
          )
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "(%d)  sram entry is empty, release tcam\n",__LINE__);
            CTC_ERROR_RETURN(sys_nalpm_trie_destroy(p_lkp_rlt->p_tcam_item->trie));
            sal_memset(key, 0 , sizeof(key));
            sal_memcpy(key, p_lkp_rlt->p_tcam_item->p_prefix_info->ip, IP_ADDR_SIZE(ip_ver));
            CTC_ERROR_RETURN(sys_nalpm_trie_delete (g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrf_id],
                                            key, p_lkp_rlt->p_tcam_item->p_prefix_info->tcam_masklen, (trie_node_t **) &p_pfx_payload, 0));
            CTC_ERROR_RETURN(_sys_tsingma_nalpm_write_tcam(lchip, p_lkp_rlt->p_tcam_item, DO_DEL, 0));
            CTC_ERROR_RETURN(_sys_tsingma_nalpm_tcam_idx_free(lchip, p_ipuc_param, p_lkp_rlt->p_tcam_item));
            if(p_usw_ipuc_master[lchip]->rchain_en)
            {
                if (0xff == real_lchip)
                {
                    return CTC_E_INVALID_PARAM;
                }
                CTC_ERROR_RETURN(_sys_tsingma_nalpm_sram_idx_free(real_lchip, p_lkp_rlt->p_tcam_item->sram_idx));
            }
            else
            {
                CTC_ERROR_RETURN(_sys_tsingma_nalpm_sram_idx_free(lchip, p_lkp_rlt->p_tcam_item->sram_idx));
            }

            mem_free(p_pfx_payload->tcam_item.p_prefix_info);
            mem_free(p_pfx_payload);
        }
    }
    else
    {
        _sys_tsingma_nalpm_get_snake_num(lchip, ip_ver, &snake_num);

        if (CTC_IP_VER_4 == ip_ver)
        {
            snake_route_num = ROUTE_NUM_PER_SNAKE_V4;
        }

        for (i = 0; i < snake_num; i++)
        {
            if (CTC_IP_VER_6 == ip_ver)
            {
                if (SYS_NALPM_SRAM_TYPE_VOID == p_lkp_rlt->p_tcam_item->sram_type[i])
                {
                    continue;
                }
                snake_route_num = (SYS_NALPM_SRAM_TYPE_V6_64 == p_lkp_rlt->p_tcam_item->sram_type[i]) ? ROUTE_NUM_PER_SNAKE_V6_64 : ROUTE_NUM_PER_SNAKE_V6_128;
            }

            for (j = 0; j < snake_route_num; j++)
            {
                if (NULL != p_lkp_rlt->p_tcam_item->p_ipuc_info_array[i][j])
                {
                    route_cnt++;
                }
            }
        }
    }

    if(g_sys_nalpm_master[lchip]->vrf_route_cnt[ip_ver][vrf_id] == 0 || g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrf_id]->trie == NULL)
    {
        CTC_ERROR_RETURN(_sys_tsingma_nalpm_route_vrf_deinit(lchip, p_ipuc_param));
    }
    if(p_usw_ipuc_master[lchip]->rchain_en)
    {
        *(p_usw_ipuc_master[lchip]->rchain_stats + real_lchip) -= 1;
    }

    return ret;
}

int32
sys_tsingma_nalpm_update(uint8 lchip, sys_ipuc_param_t* p_sys_ipuc_param, uint32 ad_index)
{
    ctc_ipuc_param_t* p_ipuc_param = &p_sys_ipuc_param->param;
    sys_nalpm_route_info_t*  p_route_info = NULL;
    sys_nalpm_route_store_info_t lkp_rlt;
    sys_nalpm_route_store_info_t *p_lkp_rlt = &lkp_rlt;
    uint8 ip_ver = 0;
    int32 ret = CTC_E_NONE;
    uint8 real_lchip = 0;
    uint16 real_index = 0;
    sys_nalpm_prefix_trie_payload_t* p_prefix_payload = NULL;
    sys_nh_info_dsnh_t nh_info;
    uint32 cmd = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    /* prepare data */
    ip_ver = p_ipuc_param->ip_ver;
    if(p_ipuc_param->masklen == 0)
    {
        if (NULL == g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][p_ipuc_param->vrf_id])
        {
            return CTC_E_NOT_EXIST;
        }
        p_prefix_payload = CONTAINER_OF(g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][p_ipuc_param->vrf_id]->trie, sys_nalpm_prefix_trie_payload_t, node);
        if (p_usw_ipuc_master[lchip]->rchain_en && !p_sys_ipuc_param->is_update && INVALID_DRV_NEXTHOP_OFFSET != ad_index)
        {
            _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_prefix_payload->tcam_item.tcam_idx, &real_lchip, &real_index);
            p_sys_ipuc_param->info->real_lchip = real_lchip;
            sys_usw_nh_get_nhinfo(real_lchip, p_sys_ipuc_param->param.nh_id, &nh_info, 0);
            _sys_tsingma_nalpm_renew_dsipda(lchip, p_sys_ipuc_param->p_dsipda, nh_info);
            CTC_ERROR_RETURN(_sys_usw_ipuc_add_ad_profile(real_lchip, p_sys_ipuc_param, p_sys_ipuc_param->p_dsipda, p_sys_ipuc_param->is_update));
            cmd = DRV_IOW(DsIpDa_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(real_lchip, p_sys_ipuc_param->info->ad_index, cmd, p_sys_ipuc_param->p_dsipda);
            ad_index = p_sys_ipuc_param->info->ad_index;
            p_prefix_payload->tcam_item.p_prefix_info->ad_idx = ad_index;
            *(p_usw_ipuc_master[lchip]->rchain_stats + real_lchip) += 1;
        }
        else if (p_usw_ipuc_master[lchip]->rchain_en && INVALID_DRV_NEXTHOP_OFFSET == ad_index)
        {
            _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_prefix_payload->tcam_item.tcam_idx, &real_lchip, &real_index);
            p_prefix_payload->tcam_item.p_prefix_info->ad_idx = ad_index;
            *(p_usw_ipuc_master[lchip]->rchain_stats + p_sys_ipuc_param->info->real_lchip) -= 1;
        }

        CTC_ERROR_RETURN(_sys_tsingma_nalpm_write_tcam( lchip, &(p_prefix_payload->tcam_item), DO_UPDATE, ad_index));
        CTC_ERROR_RETURN(_sys_tsingma_nalpm_default_route_cover(lchip, ip_ver, p_ipuc_param->vrf_id, ad_index));
        return ret;
    }

    p_route_info = mem_malloc(MEM_IPUC_MODULE, ROUTE_INFO_SIZE(ip_ver));
    if(NULL == p_route_info)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_route_info, 0 , (ROUTE_INFO_SIZE(ip_ver)));
    p_route_info->ip_ver = ip_ver;
    sal_memcpy(p_route_info->ip, &p_ipuc_param->ip, IP_ADDR_SIZE(ip_ver));
    p_route_info->route_masklen = p_ipuc_param->masklen;
    p_route_info->vrf_id = p_ipuc_param->vrf_id;
    p_route_info->ad_idx = ad_index;
    SYS_IP_V6_SORT(p_route_info);

    /* 1. lookup sw node */
    CTC_ERROR_GOTO(_sys_tsingma_nalpm_route_lkp( lchip, p_route_info,p_lkp_rlt), ret, error);
    if(!p_lkp_rlt->sram_hit)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "(%d)   can't find  ip[%x]\n",__LINE__, p_route_info->ip[0]);
        ret = CTC_E_NOT_EXIST;
        goto error;
    }
    if (p_usw_ipuc_master[lchip]->rchain_en && !p_sys_ipuc_param->is_update)
    {
        _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_lkp_rlt->p_tcam_item->tcam_idx, &real_lchip, &real_index);
        p_sys_ipuc_param->info->real_lchip = real_lchip;
    }

    p_lkp_rlt->p_tcam_item->p_ipuc_info_array[p_lkp_rlt->snake_idx][p_lkp_rlt->entry_offset]->ad_idx = p_route_info->ad_idx;
    CTC_ERROR_GOTO(_sys_nalpm_write_sram( lchip, p_lkp_rlt->p_tcam_item, p_lkp_rlt->p_tcam_item->p_ipuc_info_array[p_lkp_rlt->snake_idx][p_lkp_rlt->entry_offset],
                                         p_lkp_rlt->snake_idx, p_lkp_rlt->entry_offset, 1),
                   ret, error);

    CTC_ERROR_GOTO(_sys_tsingma_nalpm_add_route_cover(lchip, p_lkp_rlt->p_tcam_item, p_lkp_rlt->p_tcam_item->p_ipuc_info_array[p_lkp_rlt->snake_idx][p_lkp_rlt->entry_offset], 0), ret, error);
error:
    if(p_route_info)
        mem_free(p_route_info);

    return ret;
}

int32
sys_tsingma_nalpm_deinit(uint8 lchip)
{

    if(NULL == g_sys_nalpm_master[lchip])
    {
        return CTC_E_NONE;
    }
    sys_usw_opf_deinit(lchip, g_sys_nalpm_master[lchip]->opf_type_nalpm);

    mem_free(g_sys_nalpm_master[lchip]->prefix_trie[CTC_IP_VER_4]);
    mem_free(g_sys_nalpm_master[lchip]->prefix_trie[CTC_IP_VER_6]);
    mem_free(g_sys_nalpm_master[lchip]->vrf_route_cnt[CTC_IP_VER_4]);
    mem_free(g_sys_nalpm_master[lchip]->vrf_route_cnt[CTC_IP_VER_6]);

    mem_free(g_sys_nalpm_master[lchip]);

    return CTC_E_NONE;
}

int32
sys_tsingma_nalpm_init(uint8 lchip, uint8 para1, uint8 ipsa_enable)
{
    sys_usw_opf_t opf;
    uint16 i = 0, j = 0;
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 lpm_pipeline_if_ctl[4/4] = {0};
    uint16 vrfid_num[MAX_CTC_IP_VER] = {0};
    uint32 entry_num = 0;
    uint32 max_size = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (g_sys_nalpm_master[lchip])
    {
        CTC_ERROR_RETURN(sys_tsingma_nalpm_deinit(lchip));
    }

    g_sys_nalpm_master[lchip] = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_nalpm_master_t));
    if(NULL == g_sys_nalpm_master[lchip])
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(g_sys_nalpm_master[lchip], 0, sizeof(sys_nalpm_master_t));

    g_sys_nalpm_master[lchip]->split_mode = 1;
    g_sys_nalpm_master[lchip]->ipsa_enable = ipsa_enable;

    vrfid_num[CTC_IP_VER_4] = MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID)+1;
    vrfid_num[CTC_IP_VER_6] = MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID)+1;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "vrfid_num v4 [%u], v6 [%u]\n",vrfid_num[CTC_IP_VER_4], vrfid_num[CTC_IP_VER_6]);

    g_sys_nalpm_master[lchip]->prefix_trie[CTC_IP_VER_4] = mem_malloc(MEM_IPUC_MODULE, sizeof(trie_t *) * vrfid_num[CTC_IP_VER_4]);
    g_sys_nalpm_master[lchip]->prefix_trie[CTC_IP_VER_6] = mem_malloc(MEM_IPUC_MODULE, sizeof(trie_t *) * vrfid_num[CTC_IP_VER_6]);
    g_sys_nalpm_master[lchip]->vrf_route_cnt[CTC_IP_VER_4] = mem_malloc(MEM_IPUC_MODULE, sizeof(uint32) * vrfid_num[CTC_IP_VER_4]);
    g_sys_nalpm_master[lchip]->vrf_route_cnt[CTC_IP_VER_6] = mem_malloc(MEM_IPUC_MODULE, sizeof(uint32) * vrfid_num[CTC_IP_VER_6]);

    if (!g_sys_nalpm_master[lchip]->prefix_trie[CTC_IP_VER_4] || !g_sys_nalpm_master[lchip]->prefix_trie[CTC_IP_VER_6]
        || !g_sys_nalpm_master[lchip]->vrf_route_cnt[CTC_IP_VER_4] || !g_sys_nalpm_master[lchip]->vrf_route_cnt[CTC_IP_VER_6])
    {
        goto error0;
    }

    sal_memset(g_sys_nalpm_master[lchip]->prefix_trie[CTC_IP_VER_4], 0, sizeof(trie_t *) * vrfid_num[CTC_IP_VER_4]);
    sal_memset(g_sys_nalpm_master[lchip]->prefix_trie[CTC_IP_VER_6], 0, sizeof(trie_t *) * vrfid_num[CTC_IP_VER_6]);
    sal_memset(g_sys_nalpm_master[lchip]->vrf_route_cnt[CTC_IP_VER_4], 0, sizeof(uint32) * vrfid_num[CTC_IP_VER_4]);
    sal_memset(g_sys_nalpm_master[lchip]->vrf_route_cnt[CTC_IP_VER_6], 0, sizeof(uint32) * vrfid_num[CTC_IP_VER_6]);


    /*SRAM idx*/
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &g_sys_nalpm_master[lchip]->opf_type_nalpm, 1, "opf-type-nalpm"));
    opf.pool_index = 0;
    opf.pool_type  = g_sys_nalpm_master[lchip]->opf_type_nalpm;
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsNeoLpmIpv4Bit32Snake_t, &entry_num));
    max_size = DRV_IS_TM2(lchip) ? entry_num/p_usw_ipuc_master[lchip]->snake_per_group : entry_num;
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 0, max_size));

    for (i = 0; i <= SYS_NALPM_V4_MASK_LEN_MAX ; i++)
    {
        for (j = 0; j < i ; j++)
        {
            g_sys_nalpm_master[lchip]->len2pfx[i] |= 1 << (SYS_NALPM_V4_MASK_LEN_MAX - 1 - j);
        }
    }


    cmd = DRV_IOR(LpmPipelineIfCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, lpm_pipeline_if_ctl));
    value = 1;
    DRV_IOW_FIELD(0, LpmPipelineIfCtl_t, LpmPipelineIfCtl_neoLpmEn_f, &value, lpm_pipeline_if_ctl);
    cmd = DRV_IOW(LpmPipelineIfCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, lpm_pipeline_if_ctl));

    /* disable fragment arrangement by default */
    g_sys_nalpm_master[lchip]->frag_arrange_enable = 0;

    return CTC_E_NONE;
error0:
    if (g_sys_nalpm_master[lchip]->prefix_trie[CTC_IP_VER_4])
    {
        mem_free(g_sys_nalpm_master[lchip]->prefix_trie[CTC_IP_VER_4]);
    }
    if (g_sys_nalpm_master[lchip]->prefix_trie[CTC_IP_VER_6])
    {
        mem_free(g_sys_nalpm_master[lchip]->prefix_trie[CTC_IP_VER_6]);
    }
    if (g_sys_nalpm_master[lchip]->vrf_route_cnt[CTC_IP_VER_4])
    {
        mem_free(g_sys_nalpm_master[lchip]->vrf_route_cnt[CTC_IP_VER_4]);
    }
    if (g_sys_nalpm_master[lchip]->vrf_route_cnt[CTC_IP_VER_6])
    {
        mem_free(g_sys_nalpm_master[lchip]->vrf_route_cnt[CTC_IP_VER_6]);
    }
    if (g_sys_nalpm_master[lchip])
    {
        mem_free(g_sys_nalpm_master[lchip]);
    }
    return CTC_E_NO_MEMORY;
}

int32
sys_tsingma_nalpm_wb_get_info(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param, void* p_alpm_info)
{
    sys_wb_nalpm_info_t* p_wb_nalpm_info = (sys_wb_nalpm_info_t*)p_alpm_info;
    sys_nalpm_route_info_t*  p_route_info = NULL;
    sys_nalpm_route_store_info_t lkp_rlt;
    uint8 ip_ver = 0;
    int32 ret = CTC_E_NONE;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    ip_ver = p_ipuc_param->ip_ver;
    p_route_info = mem_malloc(MEM_IPUC_MODULE, ROUTE_INFO_SIZE(ip_ver));
    if(NULL == p_route_info)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_route_info, 0 , (ROUTE_INFO_SIZE(ip_ver)));
    p_route_info->ip_ver = ip_ver;
    sal_memcpy(p_route_info->ip, &p_ipuc_param->ip, IP_ADDR_SIZE(ip_ver));
    p_route_info->route_masklen = p_ipuc_param->masklen;
    p_route_info->vrf_id = p_ipuc_param->vrf_id;
    SYS_IP_V6_SORT(p_route_info);

    sal_memset(&lkp_rlt, 0 , sizeof(sys_nalpm_route_store_info_t));
    ret = _sys_tsingma_nalpm_route_lkp(lchip, p_route_info, &lkp_rlt);
    mem_free(p_route_info);
    if(!lkp_rlt.tcam_hit || lkp_rlt.p_tcam_item == NULL)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "---[%d]--- tcam not hit\r\n", __LINE__);
        return CTC_E_NOT_EXIST;
    }
    if(!lkp_rlt.sram_hit && p_ipuc_param->masklen > 0)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "---[%d]--- sram not hit\r\n", __LINE__);
        return  CTC_E_NOT_EXIST;
    }

    p_wb_nalpm_info->tcam_idx = lkp_rlt.p_tcam_item->tcam_idx;
    p_wb_nalpm_info->snake_idx= lkp_rlt.snake_idx;
    p_wb_nalpm_info->entry_offset = lkp_rlt.entry_offset;
    return ret;
}

void
_sys_tsingma_nalpm_wb_mapping_prefix(sys_nalpm_tcam_item_t* p_tcam_item, sys_wb_nalpm_prefix_info_t* p_wb_prefix_info, uint8 is_sync)
{
    if (is_sync)
    {
        p_wb_prefix_info->tcam_idx = p_tcam_item->tcam_idx;
        p_wb_prefix_info->sram_idx = p_tcam_item->sram_idx;
        p_wb_prefix_info->pfx_len =  p_tcam_item->p_prefix_info->tcam_masklen;
        p_wb_prefix_info->ip_ver = p_tcam_item->p_prefix_info->ip_ver;
        p_wb_prefix_info->vrf_id= p_tcam_item->p_prefix_info->vrf_id;
         /*_sys_nalpm_convert_to_msb(p_tcam_item->p_prefix_info->ip, p_tcam_item->p_prefix_info->tcam_masklen, p_wb_prefix_info->pfx_addr, p_tcam_item->p_prefix_info->ip_ver);*/
        sal_memcpy(p_wb_prefix_info->pfx_addr, p_tcam_item->p_prefix_info->ip, IP_ADDR_SIZE(p_wb_prefix_info->ip_ver));
        if(p_tcam_item->p_AD_route)
        {
            sal_memcpy(p_wb_prefix_info->ad_route_addr, p_tcam_item->p_AD_route->ip, IP_ADDR_SIZE(p_wb_prefix_info->ip_ver));
            p_wb_prefix_info->ad_route_masklen = p_tcam_item->p_AD_route->route_masklen;
        }
    }
    else
    {
        p_tcam_item->tcam_idx = p_wb_prefix_info->tcam_idx;
        p_tcam_item->sram_idx = p_wb_prefix_info->sram_idx;
        p_tcam_item->p_prefix_info->tcam_masklen = p_wb_prefix_info->pfx_len;
        p_tcam_item->p_prefix_info->ip_ver = p_wb_prefix_info->ip_ver;
        p_tcam_item->p_prefix_info->vrf_id= p_wb_prefix_info->vrf_id;
        sal_memcpy(p_tcam_item->p_prefix_info->ip, p_wb_prefix_info->pfx_addr, IP_ADDR_SIZE(p_wb_prefix_info->ip_ver));
        if(p_wb_prefix_info->ad_route_masklen)
        {
            sal_memcpy(p_tcam_item->p_AD_route->ip, p_wb_prefix_info->ad_route_addr, IP_ADDR_SIZE(p_wb_prefix_info->ip_ver));
            p_tcam_item->p_AD_route->route_masklen = p_wb_prefix_info->ad_route_masklen;
            p_tcam_item->p_AD_route->ip_ver = p_wb_prefix_info->ip_ver;
            p_tcam_item->p_AD_route->vrf_id= p_wb_prefix_info->vrf_id;
        }
    }

    return;
}

int32
_sys_tsingma_nalpm_wb_prefix_sync(trie_node_t * p_node, void * user_data)
{
    uint16 max_entry_cnt = 0;
    sys_wb_nalpm_prefix_info_t  *p_wb_prefix_info;
    sys_traverse_t *data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)(data->data);
    sys_nalpm_prefix_trie_payload_t* p_payload = (sys_nalpm_prefix_trie_payload_t* )p_node;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    if (p_node->type == INTERNAL)
    {
         return CTC_E_NONE;
    }
    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_prefix_info = (sys_wb_nalpm_prefix_info_t *)wb_data->buffer + wb_data->valid_cnt;
    sal_memset(p_wb_prefix_info, 0, sizeof(sys_wb_nalpm_prefix_info_t));
    p_wb_prefix_info->lchip = data->value1;
    p_wb_prefix_info->vrf_id = data->value2;
    p_wb_prefix_info->ip_ver = data->value3;
    _sys_tsingma_nalpm_wb_mapping_prefix(&p_payload->tcam_item, p_wb_prefix_info, 1);
    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}
int32
sys_tsingma_nalpm_wb_prefix_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_traverse_t user_data;
    uint16 i = 0, j = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    if(NULL == g_sys_nalpm_master[lchip])
    {
        return CTC_E_NONE;
    }
    wb_data.buffer = mem_malloc(MEM_IPUC_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_nalpm_prefix_info_t, CTC_FEATURE_IPUC, SYS_WB_APPID_IPUC_SUBID_INFO1);
    user_data.data = &wb_data;

    wb_data.valid_cnt = 0;
    for(i = 0; i <= MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID); i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (NULL == g_sys_nalpm_master[lchip]->prefix_trie[j][i])
            {
                continue;
            }
            user_data.value1 = lchip;
            user_data.value2 = i;
            user_data.value3 = j;
            CTC_ERROR_RETURN(sys_nalpm_trie_traverse (g_sys_nalpm_master[lchip]->prefix_trie[j][i]->trie, _sys_tsingma_nalpm_wb_prefix_sync,(void *)&user_data, _TRIE_PREORDER_TRAVERSE));
        }
    }
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
    }

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32
sys_tsingma_nalpm_wb_prefix_restore(uint8 lchip)
{
    uint16 entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t wb_query;
    sys_wb_nalpm_prefix_info_t wb_prefix_info;
    sys_nalpm_prefix_trie_payload_t* p_prefix_payload = NULL;
    sys_nalpm_tcam_item_t* p_tcam_item = NULL;
    ctc_ipuc_param_t ipuc_param;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_IPUC_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_nalpm_prefix_info_t, CTC_FEATURE_IPUC, SYS_WB_APPID_IPUC_SUBID_INFO1);
    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));

    sal_memcpy(&wb_prefix_info, (sys_wb_nalpm_prefix_info_t *)wb_query.buffer + entry_cnt++, wb_query.key_len + wb_query.data_len);
    if(NULL == g_sys_nalpm_master[lchip]->prefix_trie[wb_prefix_info.ip_ver][wb_prefix_info.vrf_id])
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "init prefix_trie ip_ver=[%u] vrfid =[%u],\r\n", wb_prefix_info.ip_ver,wb_prefix_info.vrf_id);
        CTC_ERROR_GOTO(sys_nalpm_trie_init ((CTC_IP_VER_4 == wb_prefix_info.ip_ver)? SYS_NALPM_V4_MASK_LEN_MAX : SYS_NALPM_V6_MASK_LEN_MAX,
                                    &g_sys_nalpm_master[lchip]->prefix_trie[wb_prefix_info.ip_ver][wb_prefix_info.vrf_id]), ret, error) ;
    }
    p_prefix_payload = mem_malloc(MEM_IPUC_MODULE, sizeof(sys_nalpm_prefix_trie_payload_t));
    if(NULL == p_prefix_payload)
    {
        ret =  CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(p_prefix_payload, 0, sizeof(sys_nalpm_prefix_trie_payload_t));

    p_tcam_item =  &(p_prefix_payload->tcam_item);
    p_tcam_item->p_prefix_info = mem_malloc(MEM_IPUC_MODULE, ROUTE_INFO_SIZE(wb_prefix_info.ip_ver));
    if(NULL == p_tcam_item->p_prefix_info)
    {
        ret =  CTC_E_NO_MEMORY;
        goto error;
    }
    sal_memset(p_tcam_item->p_prefix_info, 0, ROUTE_INFO_SIZE(wb_prefix_info.ip_ver));

    if(wb_prefix_info.ad_route_masklen)
    {
        p_tcam_item->p_AD_route = mem_malloc(MEM_IPUC_MODULE, ROUTE_INFO_SIZE(wb_prefix_info.ip_ver));
        if (NULL == p_tcam_item->p_AD_route)
        {
            ret =  CTC_E_NO_MEMORY;
            goto error;
        }
        sal_memset(p_tcam_item->p_AD_route, 0, ROUTE_INFO_SIZE(wb_prefix_info.ip_ver));
    }
    _sys_tsingma_nalpm_wb_mapping_prefix(p_tcam_item, &wb_prefix_info, 0);
    sal_memset(&ipuc_param, 0, sizeof(ipuc_param));
    ipuc_param.ip_ver = wb_prefix_info.ip_ver;
    CTC_ERROR_GOTO(_sys_tsingma_nalpm_tcam_idx_alloc(lchip, &ipuc_param, p_tcam_item),ret, error);
    CTC_ERROR_GOTO(_sys_tsingma_nalpm_sram_idx_alloc(lchip, wb_prefix_info.ip_ver, &wb_prefix_info.sram_idx),ret, error);

    /*add to tcam item trie*/
    if(CTC_IP_VER_4 == wb_prefix_info.ip_ver)
    {
        CTC_ERROR_GOTO(sys_nalpm_trie_init (SYS_NALPM_V4_MASK_LEN_MAX, &p_tcam_item->trie),ret, error);
    }
    else
    {
        CTC_ERROR_GOTO(sys_nalpm_trie_init (SYS_NALPM_V6_MASK_LEN_MAX, &p_tcam_item->trie),ret, error);
    }
    CTC_ERROR_GOTO(sys_nalpm_trie_insert (g_sys_nalpm_master[lchip]->prefix_trie[wb_prefix_info.ip_ver][wb_prefix_info.vrf_id], wb_prefix_info.pfx_addr, NULL, wb_prefix_info.pfx_len, (trie_node_t *) p_prefix_payload, 0), ret, error);
    CTC_WB_QUERY_ENTRY_END((&wb_query));
    goto done;

error:
    if(p_tcam_item->p_prefix_info)
    {
        mem_free(p_tcam_item->p_prefix_info);
    }

    if(p_tcam_item->p_AD_route)
    {
        mem_free(p_tcam_item->p_AD_route);
    }

    if(p_prefix_payload)
    {
        mem_free(p_prefix_payload);
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

    return ret;
}

int32
_sys_tsingma_nalpm_dump_route_trie_node(trie_node_t * node, void *data)
{
    trie_node_t * trie = node;
    sys_nalpm_trie_payload_t* p_payload = NULL;
    sys_nalpm_route_info_t *p_ipuc_info = NULL;
    trie_dump_node_t *p_dump_node = (trie_dump_node_t *)data;
    uint32 level = p_dump_node->father_lvl;
    trie_node_t *p_father = p_dump_node->father;
    trie_node_t *p_root = p_dump_node->root;
    char* node_type_str[3] = {"T", "L", "R"};
    uint32 node_type = 0;
    char bit_str[32] = { 0 };
    char buf[CTC_IPV6_ADDR_STR_LEN];

    if (NULL == trie)
    {
        return CTC_E_NONE;
    }

    if (p_root == trie) /*root_node*/
    {
        node_type = 0;
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    }
    else if (p_father->child[0].child_node == trie)
    {
        node_type = 1;
    }
    else
    {
        node_type = 2;
    }

    p_payload = (sys_nalpm_trie_payload_t* )node;
    p_ipuc_info = p_payload->p_route_info;

    while (level-- != 0)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "    ");
    }

    sal_memset(bit_str, 0, sizeof(bit_str));
    NALPM_PFX2BITS(trie->skip_addr, trie->skip_len, bit_str);
    if (trie->type == PAYLOAD)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
        "%s%d[%p], type[%s], skip_addr/len[%db'%s/%d], cnt[%d] Child[%p/%p],",
         node_type_str[node_type], p_dump_node->father_lvl, trie,"P",
         trie->skip_len, bit_str, trie->skip_len, trie->count, trie->child[0].child_node,
         trie->child[1].child_node);

         if (CTC_IP_VER_4 == p_ipuc_info->ip_ver)
         {
            /*
             SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " pfx_IPv4[0x%x/%d]", p_ipuc_info->ip[0], p_ipuc_info->route_masklen);
             */
             uint32 tempip = sal_ntohl(p_ipuc_info->ip[0]);
             sal_inet_ntop(AF_INET, &tempip, buf, CTC_IPV6_ADDR_STR_LEN);
             SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "[%-s/%d]   ", buf,p_ipuc_info->route_masklen);
         }
         else
         {
         /*
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP," pfx_IPv6[0x%x:0x%x:0x%x:0x%x/%d]",
            p_ipuc_info->ip[0],
            p_ipuc_info->ip[1],
            p_ipuc_info->ip[2],
            p_ipuc_info->ip[3],
            p_ipuc_info->route_masklen);
        */
            uint32 ipv6_address[4] = {0, 0, 0, 0};
            ipv6_address[0] = sal_ntohl(p_ipuc_info->ip[0]);
            ipv6_address[1] = sal_ntohl(p_ipuc_info->ip[1]);
            ipv6_address[2] = sal_ntohl(p_ipuc_info->ip[2]);
            ipv6_address[3] = sal_ntohl(p_ipuc_info->ip[3]);

            sal_inet_ntop(AF_INET6, ipv6_address, buf, CTC_IPV6_ADDR_STR_LEN);
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "[%-s/%d]    ", buf,p_ipuc_info->route_masklen);
         }
         SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"AD-idx:[0x%x]\n",p_ipuc_info->ad_idx);

    }
    else
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
        "%s%d[%p], type[%s], skip_addr/len[%db'%s/%d], cnt[%d] Child[%p/%p]\n",
        node_type_str[node_type], p_dump_node->father_lvl, trie,"I",
        trie->skip_len, bit_str, trie->skip_len, trie->count, trie->child[0].child_node,
        trie->child[1].child_node);
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_nalpm_dump_pfx_trie_node(trie_node_t * node, void *data)
{
    trie_node_t * trie = node;
    sys_nalpm_prefix_trie_payload_t* p_payload = NULL;
    sys_nalpm_route_info_t *p_ipuc_info = NULL;
    trie_dump_node_t *p_dump_node = (trie_dump_node_t *)data;
    uint32 level = p_dump_node->father_lvl;
    trie_node_t *p_father = p_dump_node->father;
    trie_node_t *p_root = p_dump_node->root;
    char* node_type_str[3] = {"T", "L", "R"};
    uint32 node_type = 0;
    char bit_str[32] = { 0 };
    if (NULL == trie)
    {
        return CTC_E_NONE;
    }

    if (p_root == trie) /*root_node*/
    {
        node_type = 0;
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
    }
    else if (p_father->child[0].child_node == trie)
    {
        node_type = 1;
    }
    else
    {
        node_type = 2;
    }

    p_payload = (sys_nalpm_prefix_trie_payload_t* )node;
    p_ipuc_info = p_payload->tcam_item.p_prefix_info;

    while (level-- != 0)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "    ");
    }

    sal_memset(bit_str, 0, sizeof(bit_str));
    NALPM_PFX2BITS(trie->skip_addr, trie->skip_len, bit_str);
    if (trie->type == PAYLOAD)
    {
        if(p_ipuc_info == NULL)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "p_ipuc_info == NULL\n");
            return CTC_E_NONE;
        }
        if (CTC_IP_VER_4 == p_ipuc_info->ip_ver)
        {

            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                             "tcam_idx[%u], %s%d[%p], type[%s], skip_addr/len[%db'%s/%d], cnt[%d] Child[%p/%p], pfx_IPv4[0x%x/%d]\n",
                             p_payload->tcam_item.tcam_idx , node_type_str[node_type], p_dump_node->father_lvl, trie, (trie->type == PAYLOAD) ? "P" : "I",
                             trie->skip_len, bit_str, trie->skip_len, trie->count, trie->child[0].child_node,
                             trie->child[1].child_node,
                             p_ipuc_info->ip[0], p_ipuc_info->tcam_masklen);
        }
        else
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
                             "tcam_idx[%u], %s%d[%p], type[%s], skip_addr/len[0x%x/%d], cnt[%d] Child[%p/%p], pfx_IPv6[0x%x:0x%x:0x%x:0x%x/%d]\n",
                             p_payload->tcam_item.tcam_idx , node_type_str[node_type], p_dump_node->father_lvl, trie, (trie->type == PAYLOAD) ? "P" : "I",
                             trie->skip_addr, trie->skip_len, trie->count, trie->child[0].child_node,
                             trie->child[1].child_node,
                             p_ipuc_info->ip[0],
                             p_ipuc_info->ip[1],
                             p_ipuc_info->ip[2],
                             p_ipuc_info->ip[3],
                             p_ipuc_info->tcam_masklen);
        }
    }
    else
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,
        "%s%d[%p], type[%s], skip_addr/len[%db'%s/%d], cnt[%d] Child[%p/%p]\n",
         node_type_str[node_type], p_dump_node->father_lvl, trie, (trie->type == PAYLOAD) ? "P" : "I",
         trie->skip_len, bit_str, trie->skip_len, trie->count, trie->child[0].child_node,
         trie->child[1].child_node);
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_nalpm_dump_pfx_trie(uint8 lchip, uint16 vrfid, uint8 ip_ver)
{
    trie_dump_node_t dump_node;
    uint32 ret = CTC_E_NONE;

    LCHIP_CHECK(lchip);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_NALPM_INIT_CHECK;

    CTC_MAX_VALUE_CHECK(ip_ver, CTC_IP_VER_6);
    CTC_MAX_VALUE_CHECK(vrfid, MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID));

    if(NULL == g_sys_nalpm_master[lchip])
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "---[%d]--- \r\n", __LINE__);
        return CTC_E_NONE;
    }
    if(NULL == g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrfid])
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "---[%d]---[%p] \r\n", __LINE__, &g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrfid]);
        return ret;
    }
    if(NULL == g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrfid]->trie)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "---[%d]--- \r\n", __LINE__);
        return ret;
    }
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-----------prefix num:[%u]----total route[%u]------\n",
                        g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrfid]->trie->count,
                        g_sys_nalpm_master[lchip]->vrf_route_cnt[ip_ver][vrfid]);

    dump_node.father = g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrfid]->trie;
    dump_node.father_lvl = 0;
    dump_node.root =  g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrfid]->trie;

    ret = sys_nalpm_trie_traverse(g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrfid]->trie, _sys_tsingma_nalpm_dump_pfx_trie_node, &dump_node, _TRIE_DUMP_TRAVERSE);

    return ret;
}

int32
_sys_tsingma_nalpm_dump_route_trie(trie_node_t * node, void *data)
{
    uint16* p_tcam_idx = (uint16*)data;
    sys_nalpm_prefix_trie_payload_t* p_payload = (sys_nalpm_prefix_trie_payload_t*)node;
    sys_nalpm_tcam_item_t* p_tcam_item = &p_payload->tcam_item;
    trie_dump_node_t dump_node;
    uint32 ret = CTC_E_NONE;
    sys_nalpm_route_info_t* ad_route = NULL;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    if (p_tcam_item->trie != NULL)
    {
        if (p_tcam_item->tcam_idx == *p_tcam_idx)
        {
            dump_node.father = p_tcam_item->trie->trie;
            dump_node.father_lvl = 0;
            dump_node.root =  p_tcam_item->trie->trie;
            if (NULL == p_tcam_item->trie->trie)
            {
                return ret;
            }
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------AD route----------\n");
            ad_route = p_tcam_item->p_AD_route;
            if(ad_route != NULL)
            {
                if (CTC_IP_VER_4 == ad_route->ip_ver)
                {
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " pfx_IPv4[0x%x/%d]\n", ad_route->ip[0], ad_route->route_masklen);
                }
                else
                {
                    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " pfx_IPv6[0x%x:0x%x:0x%x:0x%x/%d]\n",
                                     ad_route->ip[0],
                                     ad_route->ip[1],
                                     ad_route->ip[2],
                                     ad_route->ip[3],
                                     ad_route->route_masklen);
                }
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "AD-idx:[0x%x]\n", ad_route->ad_idx);
            }
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------route num:[%u]----------\n", p_tcam_item->trie->trie->count);
            ret = sys_nalpm_trie_traverse(p_tcam_item->trie->trie, _sys_tsingma_nalpm_dump_route_trie_node, &dump_node, _TRIE_DUMP_TRAVERSE);
        }
    }
    return ret;
}

int32
sys_tsingma_nalpm_dump_route_trie(uint8 lchip, uint16 vrfid, uint8 ip_ver, uint16 tcam_idx)
{
    uint32 ret = CTC_E_NONE;

    LCHIP_CHECK(lchip);
    CTC_MAX_VALUE_CHECK(ip_ver, CTC_IP_VER_6);
    CTC_MAX_VALUE_CHECK(vrfid, MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID));

    if(NULL == g_sys_nalpm_master[lchip])
    {
        return CTC_E_NONE;
    }
    if(NULL == g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrfid]
        || NULL == g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrfid]->trie)
    {
        return ret;
    }
    ret = sys_nalpm_trie_traverse(g_sys_nalpm_master[lchip]->prefix_trie[ip_ver][vrfid]->trie, _sys_tsingma_nalpm_dump_route_trie, &tcam_idx, _TRIE_DUMP_TRAVERSE);

    return ret;
}

int32
sys_tsingma_nalpm_show_route_info(uint8 lchip, ctc_ipuc_param_t* p_ipuc_param)
{
    sys_nalpm_route_info_t*  p_route_info = NULL;
    sys_nalpm_route_store_info_t lkp_rlt;
    sys_nalpm_route_store_info_t *p_lkp_rlt = &lkp_rlt;
    sys_nalpm_tcam_item_t* p_tcam_item = NULL;
    sys_ipuc_tcam_data_t tcam_data;
    uint8 ip_ver = 0;
    int32 ret = CTC_E_NONE;
    uint32 pfx[4] = {0};

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_NALPM_INIT_CHECK;

    /* prepare data */
    ip_ver = p_ipuc_param->ip_ver;

    p_route_info = mem_malloc(MEM_IPUC_MODULE, ROUTE_INFO_SIZE(ip_ver));
    if(NULL == p_route_info)
    {
        return CTC_E_NO_MEMORY;
    }

    sal_memset(p_route_info, 0 , (ROUTE_INFO_SIZE(ip_ver)));
    p_route_info->ip_ver = ip_ver;
    sal_memcpy(p_route_info->ip, &p_ipuc_param->ip, IP_ADDR_SIZE(ip_ver));
    p_route_info->route_masklen = p_ipuc_param->masklen;
    p_route_info->vrf_id = p_ipuc_param->vrf_id;
    SYS_IP_V6_SORT(p_route_info);

    /* 1. lookup sw node */
    ret = _sys_tsingma_nalpm_route_lkp( lchip, p_route_info,p_lkp_rlt);
    mem_free(p_route_info);
    if(ret < 0 || !p_lkp_rlt->tcam_hit || (!p_lkp_rlt->sram_hit && p_ipuc_param->masklen != 0))
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "(%d)   can't find  router\n",__LINE__);
        return CTC_E_NOT_EXIST;
    }

    if (MCHIP_API(lchip)->ipuc_show_tcam_key)
    {
        tcam_data.key_type = SYS_IPUC_TCAM_FLAG_ALPM;
        tcam_data.key_index = p_lkp_rlt->p_tcam_item->tcam_idx;
        tcam_data.masklen = p_ipuc_param->masklen;
        tcam_data.ipuc_param = p_ipuc_param;
        MCHIP_API(lchip)->ipuc_show_tcam_key(lchip, &tcam_data);
    }
    if(p_ipuc_param->masklen == 0)
    {
        return ret;
    }
    p_tcam_item = p_lkp_rlt->p_tcam_item;
    if(p_tcam_item->sram_type[p_lkp_rlt->snake_idx] == SYS_NALPM_SRAM_TYPE_V4_32)
    {
        if((DRV_IS_TSINGMA(lchip)?p_lkp_rlt->snake_idx+1:p_lkp_rlt->snake_idx) < p_usw_ipuc_master[lchip]->snake_per_group)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-30s:%d\n", "DsNeoLpmIpv4Bit32Snake", SRAM_HW_INDEX(lchip, p_tcam_item->sram_idx, p_lkp_rlt->snake_idx));
        }
        else
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-30s:%d\n", "DsNeoLpmIpv4Bit32Snake1", SRAM_HW_INDEX(lchip, p_tcam_item->sram_idx, p_lkp_rlt->snake_idx));
        }
    }
    if(p_tcam_item->sram_type[p_lkp_rlt->snake_idx] == SYS_NALPM_SRAM_TYPE_V6_64)
    {
        if((DRV_IS_TSINGMA(lchip)?p_lkp_rlt->snake_idx+1:p_lkp_rlt->snake_idx) < p_usw_ipuc_master[lchip]->snake_per_group)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-30s:%d\n", "DsNeoLpmIpv6Bit64Snake", SRAM_HW_INDEX(lchip, p_tcam_item->sram_idx, p_lkp_rlt->snake_idx));
        }
        else
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-30s:%d\n", "DsNeoLpmIpv6Bit64Snake1", SRAM_HW_INDEX(lchip, p_tcam_item->sram_idx, p_lkp_rlt->snake_idx));
        }
    }
    if(p_tcam_item->sram_type[p_lkp_rlt->snake_idx] == SYS_NALPM_SRAM_TYPE_V6_128)
    {
        if((DRV_IS_TSINGMA(lchip)?p_lkp_rlt->snake_idx+1:p_lkp_rlt->snake_idx) < p_usw_ipuc_master[lchip]->snake_per_group)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-30s:%d\n", "DsNeoLpmIpv6Bit128Snake", SRAM_HW_INDEX(lchip, p_tcam_item->sram_idx, p_lkp_rlt->snake_idx));
        }
        else
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-30s:%d\n", "DsNeoLpmIpv6Bit128Snake1", SRAM_HW_INDEX(lchip, p_tcam_item->sram_idx, p_lkp_rlt->snake_idx));
        }
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-30s:%d\n", "entry_offset", p_lkp_rlt->entry_offset);
    _sys_nalpm_convert_to_msb(p_tcam_item->p_prefix_info->ip, p_tcam_item->p_prefix_info->tcam_masklen, pfx, ip_ver);
    if (CTC_IP_VER_4 == ip_ver)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-30s:[0x%x/%d]\n","tcam prefix",pfx[0],p_tcam_item->p_prefix_info->tcam_masklen);
    }
    else
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-30s:[0x%x:%x:%x:%x/%d]\n",
                        "tcam prefix",
                        pfx[0],
                        pfx[1],
                        pfx[2],
                        pfx[3],
                        p_tcam_item->p_prefix_info->tcam_masklen);
    }

    return ret;
}

int32
sys_tsingma_nalpm_show_sram_usage(uint8 lchip)
{
    sys_usw_opf_t opf;
    uint32 entry_num = 0;
    opf.pool_index = 0;
    SYS_NALPM_INIT_CHECK;
    opf.pool_type = g_sys_nalpm_master[lchip]->opf_type_nalpm;
    sys_usw_ftm_query_table_entry_num(lchip, DsNeoLpmIpv4Bit32Snake_t, &entry_num);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Lchip %d SRAM usage: %u/%u 3W\n", lchip, sys_usw_opf_get_alloced_cnt(lchip, &opf), entry_num);
    if((p_usw_ipuc_master[lchip]->rchain_en && p_usw_ipuc_master[lchip]->rchain_tail == lchip) || !p_usw_ipuc_master[lchip]->rchain_en)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-------------------------------------------------------\n");
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Auto run fragment merge: %u\n", g_sys_nalpm_master[lchip]->frag_arrange_enable);
    }
    return CTC_E_NONE;
}

int32
sys_tsingma_nalpm_dump_db(uint8 lchip, sal_file_t p_f, ctc_global_dump_db_t* p_dump_param)
{
    uint32 i = 0;
    SYS_NALPM_INIT_CHECK;
    SYS_DUMP_DB_LOG(p_f, "%s\n", "# NALPM");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-30s: %u\n", "Split mode", g_sys_nalpm_master[lchip]->split_mode);
    SYS_DUMP_DB_LOG(p_f, "%-30s: %u\n", "Nalpm opf type", g_sys_nalpm_master[lchip]->opf_type_nalpm);
    SYS_DUMP_DB_LOG(p_f, "%-30s: %u\n", "Ipsa enable", g_sys_nalpm_master[lchip]->ipsa_enable);
    SYS_DUMP_DB_LOG(p_f, "%-30s: %u\n", "Auto frag arrange enable", g_sys_nalpm_master[lchip]->frag_arrange_enable);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
    if (p_dump_param->detail)
    {
        SYS_DUMP_DB_LOG(p_f, "%s\n", "IPv4 route cnt per vrf:");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
        SYS_DUMP_DB_LOG(p_f, "%-10s%-10s\n", "vrfid", "count");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
        for(i = 0; i <= MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID); i++)
        {
            SYS_DUMP_DB_LOG(p_f, "%-10u%-10u\n", i, g_sys_nalpm_master[lchip]->vrf_route_cnt[CTC_IP_VER_4][i]);
        }
        SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "IPv6 route cnt per vrf:");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
        SYS_DUMP_DB_LOG(p_f, "%-10s%-10s\n", "vrfid", "count");
        SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
        for(i = 0; i <= MCHIP_CAP(SYS_CAP_SPEC_MAX_VRFID); i++)
        {
            SYS_DUMP_DB_LOG(p_f, "%-10u%-10u\n", i, g_sys_nalpm_master[lchip]->vrf_route_cnt[CTC_IP_VER_6][i]);
        }
        SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
    }
    sys_usw_opf_fprint_alloc_used_info(lchip, g_sys_nalpm_master[lchip]->opf_type_nalpm, p_f);

    return CTC_E_NONE;

}



