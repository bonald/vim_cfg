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
#include "ctc_linklist.h"
#include "ctc_hmap.h"

#include "sys_usw_chip.h"
#include "sys_usw_ofb.h"
#include "sys_usw_ftm.h"
#include "sys_usw_ipuc.h"
#include "sys_usw_l3if.h"
#include "usw/include/drv_common.h"
#include "sys_duet2_ipuc_tcam.h"
#include "sys_tsingma_ipuc_tcam.h"
#include "sys_tsingma_nalpm.h"
#include "sys_usw_nexthop_api.h"

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
extern sys_ipuc_master_t* p_usw_ipuc_master[];
extern sys_nalpm_master_t* g_sys_nalpm_master[];

extern int32
_sys_tsingma_nalpm_get_real_tcam_index(uint8 lchip, uint16 soft_tcam_index, uint8 * real_lchip, uint16 * real_tcam_index);

/****************************************************************************
 *
* Function
*
*****************************************************************************/

int32
_sys_tsingma_nalpm_get_tcam_block_num(uint8 lchip,
                                uint8 ip_ver,
                                sys_ipuc_route_mode_t route_mode,
                                uint16* max_block_num)
{
    if ((route_mode == SYS_PRIVATE_MODE) && CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_ALPM_LOOKUP))
    {
         if (ip_ver == CTC_IP_VER_4)
        {
            /*resolve hash conflict + /32 host route + 0~31*/
            *max_block_num = p_usw_ipuc_master[lchip]->host_lpm_mode + 1 + 32;
        }
        else
        {
            if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM])
            {
                /*resolve hash conflict + /128 host route + 0~127*/
                *max_block_num = p_usw_ipuc_master[lchip]->host_lpm_mode + 1 + 128;
            }
            else
            {
                 /*1~64 + default entry*/
                 *max_block_num =  64 + 1;
            }
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
            if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM])
            {
                *max_block_num = 128 + 1;
                *max_block_num += (route_mode == SYS_PRIVATE_MODE) ? p_usw_ipuc_master[lchip]->host_lpm_mode : 0;
            }
            else
            {
               /*1~64 + default entry*/
               *max_block_num = 64 + 1;
            }
        }
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "max_block_num=[%d]\n",*max_block_num);
    return CTC_E_NONE;
}

int32
_sys_tsingma_ipuc_tcam_init_private(uint8 lchip, void* ofb_cb)
{
    uint8 ofb_type = 0;
    uint16 block_max[MAX_CTC_IP_VER] = {0};
    uint32 table_size = 0;
    uint32 total_size = 0;
    uint32 tmp_block_max = 0;
    uint32 block_id = 0;
    uint16 vrfid_num[MAX_CTC_IP_VER] = {0};
    uint32 mask_len = 0;
    sys_usw_ofb_param_t ofb_param = {0};
    uint32 average_size = 0;
    uint32 remain_size = 0;

    ofb_param.ofb_cb = ofb_cb;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    vrfid_num[CTC_IP_VER_4] = 1;
    vrfid_num[CTC_IP_VER_6] = 1;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ipv4_num=[%d],ipv6_single_num=[%d],ipv6_num=[%d] \n",
                    p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM],
                    p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT],
                    p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM]);
    if (p_usw_ipuc_master[lchip]->rchain_en && 0 != lchip)
    {
        if ((p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM] +
            p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT] +
            p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM]) < SYS_IPUC_LPM_TCAM_MAX_INDEX)
            {
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "All tcam resource need to be alloced to IPUC in rchain mode\n");
                return CTC_E_INVALID_PARAM;
            }
    }

    if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM])
    {/* tcam ipv6 double key*/
        _sys_tsingma_nalpm_get_tcam_block_num(lchip, CTC_IP_VER_6, SYS_PRIVATE_MODE, &block_max[CTC_IP_VER_6]);
        table_size += p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM];
        p_usw_ipuc_master[lchip]->tcam_share_mode[SYS_PRIVATE_MODE] = SYS_SHARE_IPV6;
    }

    if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT])
    {/* enable tcam ipv6 short key*/
        if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM])
        {
            p_usw_ipuc_master[lchip]->short_key_boundary[SYS_PRIVATE_MODE] = table_size;
        }

        _sys_tsingma_nalpm_get_tcam_block_num(lchip, CTC_IP_VER_6, SYS_PRIVATE_MODE, &block_max[CTC_IP_VER_6]);
        table_size += p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT];
        p_usw_ipuc_master[lchip]->tcam_share_mode[SYS_PRIVATE_MODE] = SYS_SHARE_ALL;
    }

    if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM])
    {/* tcam ipv4 */
        _sys_tsingma_nalpm_get_tcam_block_num(lchip, CTC_IP_VER_4, SYS_PRIVATE_MODE, &block_max[CTC_IP_VER_4]);
        table_size += p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM];
    }

    if (p_usw_ipuc_master[lchip]->rchain_en)
    {
        /* table size = first chip tcam + others' all tcam,
         Other chip tcam should not alloc more than sram max number*/
        table_size += SYS_IPUC_LPM_TCAM_MAX_INDEX * p_usw_ipuc_master[lchip]->rchain_tail;
    }

    CTC_ERROR_RETURN(sys_usw_ofb_init(lchip, block_max[CTC_IP_VER_4] + block_max[CTC_IP_VER_6], table_size, &ofb_type));
    p_usw_ipuc_master[lchip]->share_ofb_type[SYS_PRIVATE_MODE] = ofb_type;

    if (p_usw_ipuc_master[lchip]->tcam_share_mode[SYS_PRIVATE_MODE] == SYS_SHARE_IPV6)
    {
        ofb_param.max_limit_offset = 0;
        ofb_param.multiple = 4;

        table_size = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM];
        average_size = ((table_size)/(block_max[CTC_IP_VER_6]- vrfid_num[CTC_IP_VER_6])/ofb_param.multiple) *ofb_param.multiple;
        for (block_id = 0; block_id < block_max[CTC_IP_VER_6]; block_id++)
        {   /* 1~47:4   0:vrfid_num*4  65~127:4*/
           // mask_len = (p_usw_ipuc_master[lchip]->host_lpm_mode + 1 + 127 - block_id);
            mask_len = block_max[CTC_IP_VER_6] - block_id;
            if(mask_len == 0)
            {
                ofb_param.size = vrfid_num[CTC_IP_VER_6]*ofb_param.multiple;
            }
            else
            {
                ofb_param.size = average_size;
            }
            CTC_ERROR_RETURN(sys_usw_ofb_init_offset(lchip, ofb_type, block_id, &ofb_param));
            total_size += ofb_param.size;
        }
        remain_size = table_size - total_size;
    }
    else if (p_usw_ipuc_master[lchip]->tcam_share_mode[SYS_PRIVATE_MODE] == SYS_SHARE_ALL)
    {
        block_id = 0;
        table_size = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM];

        if (table_size)
        {
            ofb_param.multiple = 4;
            tmp_block_max = block_max[CTC_IP_VER_6]- 65;
            ofb_param.max_limit_offset = table_size - 1;
            ofb_param.size = 4;
            average_size = ((table_size)/(tmp_block_max)/ofb_param.multiple) *ofb_param.multiple;
            for (block_id = 0; block_id < tmp_block_max; block_id++)
            {
                if(block_id== (tmp_block_max-1) )
                {
                    ofb_param.size = table_size - average_size *block_id ;
                }
                else
                {
                    ofb_param.size = average_size;
                }
                SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "--2 init block id: %d, size: %d, table size: %d\n", block_id, ofb_param.size, table_size);
                CTC_ERROR_RETURN(sys_usw_ofb_init_offset(lchip, ofb_type, block_id, &ofb_param));
            }
        }

        ofb_param.max_limit_offset = 0;
        ofb_param.multiple = 2;
        table_size = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_SHORT];
        average_size = ((table_size - vrfid_num[CTC_IP_VER_6]*ofb_param.multiple) /(block_max[CTC_IP_VER_6]-tmp_block_max-1)/ofb_param.multiple) *ofb_param.multiple;
        for (; block_id < block_max[CTC_IP_VER_6]; block_id++)
        {
            // mask_len = (p_usw_ipuc_master[lchip]->host_lpm_mode + 1 + 127 - block_id);
            mask_len = block_max[CTC_IP_VER_6] - block_id -1;
            if(mask_len == 64)
            {
                ofb_param.size =  table_size - average_size*(block_max[CTC_IP_VER_6] -2-tmp_block_max) - vrfid_num[CTC_IP_VER_6]*ofb_param.multiple;
            }
            else if(mask_len == 0)
            {
                ofb_param.size = vrfid_num[CTC_IP_VER_6]*ofb_param.multiple;
            }
            else
            {
              ofb_param.size = average_size;
            }
            CTC_ERROR_RETURN(sys_usw_ofb_init_offset(lchip, ofb_type, block_id, &ofb_param));
        }
    }

    ofb_param.max_limit_offset = 0;
    ofb_param.multiple = 1;
    if(p_usw_ipuc_master[lchip]->rchain_en)
    {
        /* total table size = first chip ipv4 tcam + SYS_IPUC_LPM_TCAM_MAX_INDEX * other chip number */
        table_size = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM] +
            SYS_IPUC_LPM_TCAM_MAX_INDEX * p_usw_ipuc_master[lchip]->rchain_tail + remain_size;
    }
    else
    {
        table_size = p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_4][SYS_TBL_TCAM] + remain_size;
    }
    average_size = (table_size - vrfid_num[CTC_IP_VER_4]) /(block_max[CTC_IP_VER_4]-1);

    for (; block_id < (block_max[CTC_IP_VER_4] + block_max[CTC_IP_VER_6]); block_id++)
    {
        mask_len = block_max[CTC_IP_VER_6] + block_max[CTC_IP_VER_4] - block_id - 1;
        if(mask_len == 32)
        {
            ofb_param.size = table_size - vrfid_num[CTC_IP_VER_4] - average_size*(block_max[CTC_IP_VER_4] - 2);
        }
        else if(mask_len == 0)
        {
            ofb_param.size = vrfid_num[CTC_IP_VER_4];
        }
        else
        {
            ofb_param.size = average_size;
        }
        CTC_ERROR_RETURN(sys_usw_ofb_init_offset(lchip, ofb_type, block_id, &ofb_param));
    }

    return CTC_E_NONE;
}

int32
_sys_tsingma_ipuc_tcam_get_blockid(uint8 lchip,
                                uint8 ip_ver,
                                uint8 masklen,
                                sys_ipuc_route_mode_t route_mode,
                                uint8 hash_conflict,
                                uint8* block_id)
{
    uint16 ipv6_max_block = 0;

    if ((route_mode == SYS_PRIVATE_MODE) && CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_ALPM_LOOKUP))
    {
        if (ip_ver == CTC_IP_VER_4)
        {
            if (p_usw_ipuc_master[lchip]->tcam_share_mode[SYS_PRIVATE_MODE] != SYS_SHARE_NONE)
            {
                _sys_tsingma_nalpm_get_tcam_block_num(lchip, CTC_IP_VER_6, SYS_PRIVATE_MODE, &ipv6_max_block);
            }

            if(masklen < 32)
            {/*nalpm block*/
                *block_id = p_usw_ipuc_master[lchip]->host_lpm_mode + 1 + (31 - masklen);
            }
            else if(masklen == 32 && hash_conflict)
            {
                *block_id = p_usw_ipuc_master[lchip]->host_lpm_mode;
            }
            else if(masklen == 32 && !hash_conflict)
            {
                *block_id = 0;
            }
            else
            {
                return CTC_E_INVALID_PARAM;
            }

            *block_id += ipv6_max_block;

        }
        else
        {
            if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM])
            {
                if (masklen < 128)
                {/*nalpm block*/
                    *block_id = p_usw_ipuc_master[lchip]->host_lpm_mode + 1 + (127 - masklen);
                }
                else if(masklen == 128 && hash_conflict)
                {
                    *block_id = p_usw_ipuc_master[lchip]->host_lpm_mode;
                }
                else if(masklen == 128 && !hash_conflict)
                {
                    *block_id = 0;
                }
                else
                {
                    return CTC_E_INVALID_PARAM;
                }
            }
            else
            {
                if (masklen <= 64)
                {/*nalpm block*/
                    *block_id = (64 - masklen);
                }
                 else
                {
                    return CTC_E_INVALID_PARAM;
                }
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
                _sys_tsingma_nalpm_get_tcam_block_num(lchip, CTC_IP_VER_6, route_mode, &ipv6_max_block);
            }
            *block_id += ipv6_max_block;
            *block_id += (route_mode == SYS_PRIVATE_MODE) ? p_usw_ipuc_master[lchip]->host_lpm_mode : 0;
        }
        else
        {
            if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM])
            {
                *block_id = 128 - masklen;
                *block_id += (route_mode == SYS_PRIVATE_MODE) ? p_usw_ipuc_master[lchip]->host_lpm_mode : 0;
            }
            else
            {
                *block_id = 64 - masklen;
            }
        }

    }
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "block_id=[%d]\n",*block_id);

    return CTC_E_NONE;
}

int32
sys_tsingma_ipuc_tcam_get_blockid(uint8 lchip, sys_ipuc_tcam_data_t *p_data, uint8 *p_block_id)
{
    uint8 ip_ver = 0;
    uint8 masklen = 0;
    uint8 hash_conflict = 0;
    sys_ipuc_info_t  *p_ipuc_info    = NULL;
    ctc_ipuc_param_t  *p_ipuc_param    = NULL;
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
        p_ipuc_param = p_data->ipuc_param;
        ip_ver = p_ipuc_param->ip_ver;
        masklen = p_data->masklen;
    }

    return _sys_tsingma_ipuc_tcam_get_blockid(lchip, ip_ver, masklen, route_mode, hash_conflict, p_block_id);
}

int32
_sys_tsingma_ipuc_route_move(uint8 old_lchip, uint16 old_index, uint8 new_lchip, uint16 new_index, uint8 ip_ver, sys_ipuc_route_mode_t route_mode, sys_ipuc_ofb_cb_t* p_ofb_cb)
{
    sys_ipuc_tcam_ad_t ad;
    sys_ipuc_tcam_key_t key;
    int32 ret = CTC_E_NONE;
    uint16 sram_idx = 0;
    uint8 i = 0, j=0;
    uint8 snake_num = 0, route_per_snake = 0;
    sys_nalpm_route_info_t* p_route_info = NULL;
    DsIpDa_m dsipda;
    uint32 glb_idx = 0;
    uint32 count = 0;
    uint32 total_count = 0;
    uint8 lchip = 0;
    sys_nh_info_dsnh_t nhinfo;
    sys_ipuc_param_t* p_sys_param = NULL;
    sys_nalpm_tcam_item_t* p_tcam_item = NULL;
    _sys_tsingma_nalpm_get_snake_num(0, ip_ver, &snake_num);

    sal_memset(&ad, 0, sizeof(sys_ipuc_tcam_ad_t));
    sal_memset(&key, 0, sizeof(sys_ipuc_tcam_key_t));
    sal_memset(&nhinfo, 0, sizeof(sys_nh_info_dsnh_t));

    sys_usw_ftm_get_alloced_table_count(new_lchip, DsIpDa_t, &count);
    sys_usw_ftm_query_table_entry_num(new_lchip, DsIpDa_t, &total_count);
    if (SYS_IPUC_TCAM_FLAG_ALPM == p_ofb_cb->type)
    {
        p_tcam_item = (sys_nalpm_tcam_item_t*)p_ofb_cb->user_data;
    }
    if(CTC_IP_VER_4 == ip_ver)
    {
        if(total_count - count < 13)
        {
            /* ad route dsipda + 12 entry dsipda */
            return CTC_E_NO_RESOURCE;
        }
    }
    else
    {
        if (total_count - count < 7)
        {
            /* ad route dsipda + 6 entry dsipda */
            return CTC_E_NO_RESOURCE;
        }
    }

    CTC_ERROR_RETURN(_sys_tsingma_nalpm_sram_idx_alloc(new_lchip, ip_ver, &sram_idx));
    p_sys_param =  mem_malloc(MEM_IPUC_MODULE, sizeof(sys_ipuc_param_t));

    ad.tcamAd_key_idx = old_index;
    CTC_ERROR_GOTO(sys_tsingma_ipuc_tcam_read_ad(old_lchip, ip_ver, route_mode, &ad), ret, error0);
    /* lchip used for GetDsIpDa */
    lchip = old_lchip;
    /* copy ad route's dsipda from old chip to new chip*/
    if(ad.nexthop != 0x3ffff)
    {
        /* There are 3 situation if ad.nexthop != 0x3ffff:
        1. Tcam ad represent ad route
        2. Tcam ad represent DO_TCAM ad
        3. Tcam ad represent default route
        4. Tcam ad represent hash conflict route's ad */
        uint8 need_update_rlchip = 0;
        uint8 real_lchip = 0;
        uint16 real_index = 0;
        uint32 cmd = 0;
        sys_nalpm_prefix_trie_payload_t* p_prefix_payload = NULL;
        sal_memset(p_sys_param, 0, sizeof(sys_ipuc_param_t));
        _sys_usw_ipuc_read_ipda(old_lchip, ad.nexthop, &dsipda);
        if (SYS_IPUC_TCAM_FLAG_ALPM == p_ofb_cb->type && p_tcam_item->p_AD_route)
        {
            /* ad route */
            if (!ip_ver)
            {
                p_sys_param->param.ip.ipv4 = *p_tcam_item->p_AD_route->ip;
            }
            else
            {
                sal_memcpy(p_sys_param->param.ip.ipv6, p_tcam_item->p_AD_route->ip, IP_ADDR_SIZE(CTC_IP_VER_6));
            }
            p_sys_param->param.ip_ver = ip_ver;
            p_sys_param->param.masklen = p_tcam_item->p_AD_route->route_masklen;
            p_sys_param->param.vrf_id = p_tcam_item->p_AD_route->vrf_id;
            need_update_rlchip = 0;
        }
        else
        {
            /* DO_TCAM route + default route */
            key.key_idx = old_index;
            sys_tsingma_ipuc_tcam_read_key(old_lchip, ip_ver, route_mode, &key);
            if(!ip_ver)
            {
                p_sys_param->param.ip.ipv4 = key.ip.ipv4;
            }
            else
            {
                sal_memcpy(p_sys_param->param.ip.ipv6, key.ip.ipv6, IP_ADDR_SIZE(CTC_IP_VER_6));
            }
            p_sys_param->param.ip_ver = ip_ver;
            p_sys_param->param.masklen = key.mask_len;
            p_sys_param->param.vrf_id =key.vrfId;
            if (32 == p_sys_param->param.masklen || 128 == p_sys_param->param.masklen)
            {
                CTC_SET_FLAG(p_sys_param->param.route_flag, CTC_IPUC_FLAG_HOST_USE_LPM);
            }
            if (!p_sys_param->param.masklen)
            {
                /* if tcam key masklen == 0 then is real default route */
                need_update_rlchip = 1;
            }
            else
            {
                if (NULL != g_sys_nalpm_master[0]->prefix_trie[ip_ver][p_sys_param->param.vrf_id])
                {
                    p_prefix_payload = CONTAINER_OF(g_sys_nalpm_master[0]->prefix_trie[ip_ver][p_sys_param->param.vrf_id]->trie, sys_nalpm_prefix_trie_payload_t, node);
                    _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_prefix_payload->tcam_item.tcam_idx, &real_lchip, &real_index);
                    if (old_index == real_index && old_lchip == real_lchip)
                    {
                        need_update_rlchip = 1;
                    }
                    else
                    {
                        need_update_rlchip = 0;
                    }
                }
                else
                {
                    /* vrf not init, this route must be DO_TCAM route */
                    need_update_rlchip = 1;
                }
            }
        }
        SYS_IP_ADDRESS_SORT2(p_sys_param->param);
        CTC_ERROR_GOTO(_sys_usw_ipuc_db_lookup(0, p_sys_param), ret, error0);
        if (NULL == p_sys_param->info && (32 == p_sys_param->param.masklen || 128 == p_sys_param->param.masklen))
        {
            /* this might be hash conflict route */
            CTC_UNSET_FLAG(p_sys_param->param.route_flag, CTC_IPUC_FLAG_HOST_USE_LPM);
            CTC_ERROR_GOTO(_sys_usw_ipuc_db_lookup(0, p_sys_param), ret, error0);
            need_update_rlchip = 1;
        }
        if (NULL == p_sys_param->info)
        {
            /* ad index equal to default route ad */
            sal_memset(p_sys_param, 0, sizeof(sys_ipuc_param_t));
            p_sys_param->param.ip_ver = ip_ver;
            p_sys_param->param.masklen = 0;
            p_sys_param->param.vrf_id = p_tcam_item->p_prefix_info->vrf_id;
            CTC_ERROR_GOTO(_sys_usw_ipuc_db_lookup(0, p_sys_param), ret, error0);
            p_prefix_payload = CONTAINER_OF(g_sys_nalpm_master[0]->prefix_trie[ip_ver][p_sys_param->param.vrf_id]->trie, sys_nalpm_prefix_trie_payload_t, node);
            _sys_tsingma_nalpm_get_real_tcam_index(lchip, p_prefix_payload->tcam_item.tcam_idx, &real_lchip, &real_index);
            if (old_index == real_index && old_lchip == real_lchip)
            {
                need_update_rlchip = 1;
            }
            else
            {
                need_update_rlchip = 0;
            }
        }

        if (need_update_rlchip)
        {
            if (new_lchip != 0)
            {
                glb_idx = p_usw_ipuc_master[new_lchip - 1]->rchain_boundary + new_index;
            }
            else
            {
                glb_idx = new_index;
            }
            p_sys_param->info->key_index = glb_idx;
            p_sys_param->info->real_lchip = new_lchip;
            *(p_usw_ipuc_master[0]->rchain_stats + old_lchip) -= 1;
            *(p_usw_ipuc_master[0]->rchain_stats + new_lchip) += 1;
        }
        CTC_ERROR_GOTO(sys_usw_nh_get_nhinfo(new_lchip, p_sys_param->info->nh_id, &nhinfo, 0), ret, error0);
        _sys_tsingma_nalpm_renew_dsipda(lchip, &dsipda, nhinfo);
        _sys_usw_ipuc_add_ad_profile(new_lchip, p_sys_param, (void*)&dsipda, 0);
        cmd = DRV_IOW(DsIpDa_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(new_lchip, p_sys_param->info->ad_index, cmd, &dsipda);
        _sys_usw_ipuc_remove_ad_profile(old_lchip, ad.nexthop);
        if (SYS_IPUC_TCAM_FLAG_ALPM == p_ofb_cb->type && p_tcam_item->p_AD_route)
        {
            p_tcam_item->p_AD_route->ad_idx = p_sys_param->info->ad_index;
        }
        if (SYS_IPUC_TCAM_FLAG_ALPM == p_ofb_cb->type)
        {
            p_tcam_item->p_prefix_info->ad_idx = p_sys_param->info->ad_index;
        }
        ad.nexthop = p_sys_param->info->ad_index;
    }

    /* copy sram ad from old chip to new chip */
    if(ad.lpmPipelineValid)
    {
        /* process each route's dsipda and write snake */
        if(CTC_IP_VER_4 == ip_ver)
        {
            for (i = 0; i < (snake_num * ROUTE_NUM_PER_SNAKE_V4); i++)
            {
                p_route_info = p_tcam_item->p_ipuc_info_array[i/ROUTE_NUM_PER_SNAKE_V4][i%ROUTE_NUM_PER_SNAKE_V4];
                if (NULL != p_route_info)
                {
                    _sys_tsingma_nalpm_move_route(lchip, new_lchip, old_lchip, p_route_info);
                    _sys_nalpm_clear_sram(lchip, p_tcam_item, i/ROUTE_NUM_PER_SNAKE_V4, i%ROUTE_NUM_PER_SNAKE_V4, 1);
                    *(p_usw_ipuc_master[0]->rchain_stats + old_lchip) -= 1;
                    *(p_usw_ipuc_master[0]->rchain_stats + new_lchip) += 1;
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
                    p_route_info = p_tcam_item->p_ipuc_info_array[i][j];
                    if (NULL != p_route_info)
                    {
                        _sys_tsingma_nalpm_move_route(lchip, new_lchip, old_lchip, p_route_info);
                        _sys_nalpm_clear_sram(lchip, p_tcam_item, i, j, 1);
                        *(p_usw_ipuc_master[0]->rchain_stats + old_lchip) -= 1;
                        *(p_usw_ipuc_master[0]->rchain_stats + new_lchip) += 1;
                    }
                }
            }
        }

        p_tcam_item->sram_idx = sram_idx;
        if (new_lchip != 0)
        {
            glb_idx = p_usw_ipuc_master[new_lchip - 1]->rchain_boundary + new_index;
        }
        else
        {
            glb_idx = new_index;
        }
        p_tcam_item->tcam_idx = glb_idx;
        _sys_tsingma_nalpm_write_route_trie(new_lchip, p_tcam_item, 1);
        /* free old chip sram index */
        _sys_tsingma_nalpm_sram_idx_free(old_lchip, ad.pointer);
        ad.pointer = sram_idx;
    }
    ad.tcamAd_key_idx = new_index;
    sys_tsingma_ipuc_tcam_write_ad2hw(new_lchip, ip_ver, route_mode, &ad, DO_ADD);

    /* copy tcam key from old chip to new chip */
    key.key_idx = old_index;
    sys_tsingma_ipuc_tcam_read_key(old_lchip, ip_ver, route_mode, &key);
    key.key_idx = new_index;
    sys_tsingma_ipuc_tcam_write_key2hw(new_lchip, ip_ver, route_mode, &key, DO_ADD);
    /* delete old chip tcam key */
    key.key_idx = old_index;
    sys_tsingma_ipuc_tcam_write_key2hw(old_lchip, ip_ver, route_mode, &key, DO_DEL);
    mem_free(p_sys_param);
    return ret;

error0:
    mem_free(p_sys_param);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "ipuc route move error!\n");
    _sys_tsingma_nalpm_sram_idx_free(new_lchip, sram_idx);
    return ret;
}

int32
_sys_tsingma_ipuc_tcam_move(uint8 lchip, uint8 ip_ver, sys_ipuc_route_mode_t route_mode, uint32 new_index, uint32 old_index)
{
    uint32 cmdr, cmdw;
    ds_t lpmtcam_ad;
    ds_t tcamkey, tcammask;
    tbl_entry_t tbl_ipkey;
    sys_ipuc_tbl_type_t tbl_type[2]; /* index 0 is old, index 1 is new */
    uint32 key_id[2], sa_key_id[2];
    uint32 ad_id[2], sa_ad_id[2];
    uint16 key_index[2];
    uint8 use_short_key[2] = {0};
    sys_ipuc_tcam_key_t key;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (new_index == old_index)
    {
        return CTC_E_NONE;
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
    DRV_IOCTL(lchip, old_index, cmdr, &lpmtcam_ad);
    DRV_IOCTL(lchip, new_index, cmdw, &lpmtcam_ad);

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
        DRV_IOCTL(lchip, old_index, cmdr, &lpmtcam_ad);
        DRV_IOCTL(lchip, new_index, cmdw, &lpmtcam_ad);

        cmdw = DRV_IOW(sa_key_id[1], DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, key_index[1], cmdw, &tbl_ipkey);
    }

    key.key_idx = old_index;
    sys_tsingma_ipuc_tcam_write_key2hw( lchip, ip_ver, route_mode ,&key, DO_DEL);
    if(!route_mode && ip_ver && IS_SHORT_KEY(0, new_index) && !IS_SHORT_KEY(0, old_index))
    {
        p_usw_ipuc_master[lchip]->lpm_tcam_used_cnt[ip_ver] -= 2;
    }
    else if(!route_mode && ip_ver && !IS_SHORT_KEY(0, new_index) && IS_SHORT_KEY(0, old_index))
    {
        p_usw_ipuc_master[lchip]->lpm_tcam_used_cnt[ip_ver] += 2;
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: move from %s[%d] -> %s[%d] \n", TABLE_NAME(lchip, key_id[0]), old_index, TABLE_NAME(lchip, key_id[1]), new_index);

    return CTC_E_NONE;
}

int32
sys_tsingma_ipuc_tcam_move(uint8 lchip, uint32 new_index, uint32 old_index, void *pdata)
{
    sys_ipuc_ofb_cb_t *p_ofb_cb = (sys_ipuc_ofb_cb_t *)pdata;
    uint8 ip_ver = CTC_IP_VER_4;
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;
    sys_ipuc_info_t  *p_ipuc_info    = NULL;
    uint8 old_real_lchip = 0;
    uint8 new_real_lchip = 0;
    uint16 old_real_index = 0;
    uint16 new_real_index = 0;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    ip_ver = p_ofb_cb->ip_ver;
    route_mode = p_ofb_cb->route_mode;

    if (p_usw_ipuc_master[lchip]->rchain_en)
    {
        _sys_tsingma_nalpm_get_real_tcam_index(lchip, old_index, &old_real_lchip, &old_real_index);
        _sys_tsingma_nalpm_get_real_tcam_index(lchip, new_index, &new_real_lchip, &new_real_index);
        if(0xff == old_real_lchip || 0xff == new_real_lchip)
        {
            return CTC_E_INTR_INVALID_PARAM;
        }
    }
    else
    {
        new_real_lchip = lchip;
        old_real_index = lchip;
        new_real_index = new_index;
        old_real_index = old_index;
    }

    if(p_usw_ipuc_master[lchip]->rchain_en && (old_real_lchip != new_real_lchip))
    {
        CTC_ERROR_RETURN(_sys_tsingma_ipuc_route_move(old_real_lchip, old_real_index, new_real_lchip, new_real_index, ip_ver, route_mode, (sys_ipuc_ofb_cb_t*)p_ofb_cb));
    }
    else
    {
        _sys_tsingma_ipuc_tcam_move(new_real_lchip, ip_ver, route_mode,  new_real_index,  old_real_index);
    }

    if (p_ofb_cb->type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        p_ipuc_info = (sys_ipuc_info_t*)p_ofb_cb->user_data;
        p_ipuc_info->key_index = new_index;
    }
    else
    {
        sys_nalpm_tcam_item_t* p_tcam_item = NULL;
        p_tcam_item = (sys_nalpm_tcam_item_t*)p_ofb_cb->user_data;
        p_tcam_item->tcam_idx = new_index;
    }

    return CTC_E_NONE;

}

int32
sys_tsingma_ipuc_tcam_write_key(uint8 lchip, sys_ipuc_tcam_data_t *p_data)
{
    sys_ipuc_tcam_key_t key;
    uint8 ip_ver = CTC_IP_VER_4;
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;
    sys_ipuc_info_t  *p_ipuc_info    = NULL;
    uint32 t;
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    CTC_PTR_VALID_CHECK(p_data);
    CTC_PTR_VALID_CHECK(p_data->info);

    p_ipuc_info = (sys_ipuc_info_t*)p_data->info;
    /* write ipuc tcam key entry */
    ip_ver = SYS_IPUC_VER(p_ipuc_info);
    sal_memset(&key, 0 , sizeof(key));
    key.mask_len = p_ipuc_info->masklen;
    key.key_idx = p_ipuc_info->key_index;
    sal_memcpy(&key.ip,  &p_ipuc_info->ip, sizeof(key.ip));
    key.vrfId = p_ipuc_info->vrf_id;
    key.vrfId_mask =  0x1FFF;
    if(ip_ver == CTC_IP_VER_6)
    {
        t = key.ip.ipv6[0];
        key.ip.ipv6[0] = key.ip.ipv6[3];
        key.ip.ipv6[3] = t;

        t = key.ip.ipv6[1];
        key.ip.ipv6[1] = key.ip.ipv6[2];
        key.ip.ipv6[2] = t;
    }
    if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE))
    {
         key.vrfId_mask = 0;
         route_mode = SYS_PUBLIC_MODE;
    }

    CTC_ERROR_RETURN(sys_tsingma_ipuc_tcam_write_key2hw( lchip,  ip_ver,  route_mode, &key,  p_data->opt_type));

    return CTC_E_NONE;
}

int32
sys_tsingma_ipuc_tcam_write_key2hw(uint8 lchip, ctc_ip_ver_t ip_ver, sys_ipuc_route_mode_t route_mode, sys_ipuc_tcam_key_t* p_key, sys_ipuc_opt_type_t opt)
{
    tbl_entry_t tbl_ipkey;
    ds_t tcamkey, tcammask;
    uint32 cmd = 0;
    sys_ipuc_tbl_type_t tbl_type;
    uint32 key_idx;
    uint32 da_table, sa_table;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    CTC_PTR_VALID_CHECK(p_key);
    if(DO_ADD != opt && DO_DEL != opt)
    {
        return CTC_E_INVALID_PARAM;
    }
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ip_ver[%d],route_mode[%d],key_idx[%d],mask_len[%d],vrfId[%d]\n",
                        ip_ver,route_mode,p_key->key_idx,p_key->mask_len,p_key->vrfId);
    key_idx = p_key->key_idx;
    if(SYS_PUBLIC_MODE == route_mode)
    {
        if((ip_ver == CTC_IP_VER_6) && (IS_SHORT_KEY(route_mode, key_idx)))
        {
            tbl_type = SYS_TBL_TCAM_PUB_SHORT;
        }
        else
        {
            tbl_type = SYS_TBL_TCAM_PUB;
        }
    }
    else
    {
        if((ip_ver == CTC_IP_VER_6) && (IS_SHORT_KEY(route_mode, key_idx)))
        {
            tbl_type = SYS_TBL_TCAM_SHORT;
        }
        else
        {
            tbl_type = SYS_TBL_TCAM;
        }
    }

    sys_usw_get_tbl_id(lchip, ip_ver, 1, tbl_type, &da_table, NULL);
    if(DO_ADD == opt)
    {
        /* write key */
        sal_memset(&tcamkey, 0, sizeof(ds_t));
        sal_memset(&tcammask, 0, sizeof(ds_t));
        tbl_ipkey.data_entry = (uint32*)&tcamkey;
        tbl_ipkey.mask_entry = (uint32*)&tcammask;

        if(CTC_IP_VER_4 == ip_ver)
        {
            ip_addr_t ipv4_mask;
            /*key fields*/
            SetDsLpmTcamIpv4HalfKey(V, ipAddr_f, tcamkey, p_key->ip.ipv4);
            SetDsLpmTcamIpv4HalfKey(V, vrfId_f, tcamkey, p_key->vrfId);
            SetDsLpmTcamIpv4HalfKey(V, lpmTcamKeyType_f, tcamkey, 0);

            /*key mask*/
            IPV4_LEN_TO_MASK(ipv4_mask, p_key->mask_len);
            SetDsLpmTcamIpv4HalfKey(V, ipAddr_f, tcammask, ipv4_mask);
            SetDsLpmTcamIpv4HalfKey(V, vrfId_f, tcammask, p_key->vrfId_mask);
            SetDsLpmTcamIpv4HalfKey(V, lpmTcamKeyType_f, tcammask, 0x1);
        }
        else
        {
            ipv6_addr_t ipv6_data, ipv6_mask;
            sal_memset(&ipv6_data, 0, sizeof(ipv6_addr_t));
            if (IS_SHORT_KEY(route_mode, key_idx))
            {
                key_idx = key_idx/2;
                /*key fields*/
                ipv6_data[0] = p_key->ip.ipv6[1];
                ipv6_data[1] = p_key->ip.ipv6[0];
                SetDsLpmTcamIpv6SingleKey(A, ipAddr_f, tcamkey, ipv6_data);
                SetDsLpmTcamIpv6SingleKey(V, vrfId_f, tcamkey, p_key->vrfId);
                SetDsLpmTcamIpv6SingleKey(V, lpmTcamKeyType0_f, tcamkey, 1);
                SetDsLpmTcamIpv6SingleKey(V, lpmTcamKeyType1_f, tcamkey, 1);

                 /*key mask*/
                IPV6_LEN_TO_MASK(ipv6_mask, p_key->mask_len);
                sal_memcpy(&ipv6_mask[0], &ipv6_mask[2], sizeof(uint32) * 2);

                SetDsLpmTcamIpv6SingleKey(A, ipAddr_f, tcammask, ipv6_mask);
                SetDsLpmTcamIpv6SingleKey(V, vrfId_f, tcammask, p_key->vrfId_mask);
                SetDsLpmTcamIpv6SingleKey(V, lpmTcamKeyType0_f, tcammask, 0x1);
                SetDsLpmTcamIpv6SingleKey(V, lpmTcamKeyType1_f, tcammask, 0x1);
            }
            else
            {
                key_idx = key_idx/4;
                ipv6_data[0] = p_key->ip.ipv6[3];
                ipv6_data[1] = p_key->ip.ipv6[2];
                ipv6_data[2] = p_key->ip.ipv6[1];
                ipv6_data[3] = p_key->ip.ipv6[0];
                SetDsLpmTcamIpv6DoubleKey0(A, ipAddr_f, tcamkey, ipv6_data);
                SetDsLpmTcamIpv6DoubleKey0(V, vrfId_f, tcamkey, p_key->vrfId);
                SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType0_f, tcamkey, 1);
                SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType1_f, tcamkey, 1);
                SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType2_f, tcamkey, 1);
                SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType3_f, tcamkey, 1);

                IPV6_LEN_TO_MASK(ipv6_mask, p_key->mask_len);
                SetDsLpmTcamIpv6DoubleKey0(A, ipAddr_f, tcammask, ipv6_mask);
                SetDsLpmTcamIpv6DoubleKey0(V, vrfId_f, tcammask, p_key->vrfId_mask);
                SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType0_f, tcammask, 0x1);
                SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType1_f, tcammask, 0x1);
                SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType2_f, tcammask, 0x1);
                SetDsLpmTcamIpv6DoubleKey0(V, lpmTcamKeyType3_f, tcammask, 0x1);
            }
        }
        cmd = DRV_IOW(da_table, DRV_ENTRY_FLAG);
    }
    else
    {
        if(CTC_IP_VER_6 == ip_ver)
        {
            if (IS_SHORT_KEY(route_mode, key_idx))
            {
                key_idx = key_idx/2;
            }
            else
            {
                key_idx = key_idx/4;
            }
        }
        cmd = DRV_IOD(da_table, DRV_ENTRY_FLAG);
    }
    DRV_IOCTL(lchip, key_idx, cmd, &tbl_ipkey);

    if(CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_TCAM_SA_LOOKUP))
    {
        sys_usw_get_tbl_id(lchip, ip_ver, 0, tbl_type, &sa_table, NULL);
        if(DO_ADD == opt)
        {
            cmd = DRV_IOW(sa_table, DRV_ENTRY_FLAG);
        }
        else
        {
            cmd = DRV_IOD(sa_table, DRV_ENTRY_FLAG);
        }
        DRV_IOCTL(lchip, key_idx, cmd, &tbl_ipkey);
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: write %s[%d]\n", TABLE_NAME(lchip, da_table), key_idx);

    return CTC_E_NONE;

}

int32
sys_tsingma_ipuc_tcam_read_key(uint8 lchip, ctc_ip_ver_t ip_ver, sys_ipuc_route_mode_t route_mode, sys_ipuc_tcam_key_t* p_key)
{
    tbl_entry_t tbl_ipkey;
    ds_t tcamkey, tcammask;
    uint32 cmd = 0;
    sys_ipuc_tbl_type_t tbl_type;
    uint32 key_idx;
    uint32 da_table;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    CTC_PTR_VALID_CHECK(p_key);

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "ip_ver[%d],route_mode[%d],key_idx[%d],mask_len[%d],vrfId[%d]\n",
                        ip_ver,route_mode,p_key->key_idx,p_key->mask_len,p_key->vrfId);
    key_idx = p_key->key_idx;
    if(ip_ver)
    {
        if (IS_SHORT_KEY(route_mode, key_idx))
        {
            key_idx = key_idx/2;
        }
        else
        {
            key_idx = key_idx / 4;
        }
    }
    if(SYS_PUBLIC_MODE == route_mode)
    {
        if((ip_ver == CTC_IP_VER_6) && (IS_SHORT_KEY(route_mode, key_idx)))
        {
            tbl_type = SYS_TBL_TCAM_PUB_SHORT;
        }
        else
        {
            tbl_type = SYS_TBL_TCAM_PUB;
        }
    }
    else
    {
        if((ip_ver == CTC_IP_VER_6) && (IS_SHORT_KEY(route_mode, key_idx)))
        {
            tbl_type = SYS_TBL_TCAM_SHORT;
        }
        else
        {
            tbl_type = SYS_TBL_TCAM;
        }
    }

    sys_usw_get_tbl_id(lchip, ip_ver, 1, tbl_type, &da_table, NULL);

    sal_memset(&tcamkey, 0, sizeof(ds_t));
    sal_memset(&tcammask, 0, sizeof(ds_t));
    tbl_ipkey.data_entry = (uint32*)&tcamkey;
    tbl_ipkey.mask_entry = (uint32*)&tcammask;

    cmd = DRV_IOR(da_table, DRV_ENTRY_FLAG);

    DRV_IOCTL(lchip, key_idx, cmd, &tbl_ipkey);

    if (CTC_IP_VER_4 == ip_ver)
    {
        ip_addr_t ipv4_mask;
        /*key fields*/
        GetDsLpmTcamIpv4HalfKey(A, ipAddr_f, tcamkey, &p_key->ip.ipv4);
        GetDsLpmTcamIpv4HalfKey(A, vrfId_f, tcamkey, &p_key->vrfId);

        /*key mask*/
        GetDsLpmTcamIpv4HalfKey(A, ipAddr_f, tcammask, &ipv4_mask);
        IPV4_MASK_TO_LEN(ipv4_mask, p_key->mask_len);
        GetDsLpmTcamIpv4HalfKey(A, vrfId_f, tcammask, &p_key->vrfId_mask);
    }
    else
    {
        ipv6_addr_t ipv6_data, ipv6_mask;
        sal_memset(&ipv6_data, 0, sizeof(ipv6_addr_t));
        sal_memset(&ipv6_mask, 0, sizeof(ipv6_addr_t));
        if (IS_SHORT_KEY(route_mode, key_idx))
        {
            /*key fields*/
            GetDsLpmTcamIpv6SingleKey(A, ipAddr_f, tcamkey, &ipv6_data);
            p_key->ip.ipv6[1] = ipv6_data[0];
            p_key->ip.ipv6[0] = ipv6_data[1];
            GetDsLpmTcamIpv6SingleKey(A, vrfId_f, tcamkey, &p_key->vrfId);

            /*key mask*/
            GetDsLpmTcamIpv6SingleKey(A, ipAddr_f, tcammask, &ipv6_mask);
            IPV6_MASK_TO_LEN(ipv6_mask, p_key->mask_len);
            GetDsLpmTcamIpv6SingleKey(A, vrfId_f, tcammask, &p_key->vrfId_mask);
        }
        else
        {
            GetDsLpmTcamIpv6DoubleKey0(A, ipAddr_f, tcamkey, &ipv6_data);
            p_key->ip.ipv6[3] = ipv6_data[0];
            p_key->ip.ipv6[2] = ipv6_data[1];
            p_key->ip.ipv6[1] = ipv6_data[2];
            p_key->ip.ipv6[0] = ipv6_data[3];
            GetDsLpmTcamIpv6DoubleKey0(A, vrfId_f, tcamkey, &p_key->vrfId);

            GetDsLpmTcamIpv6DoubleKey0(A, ipAddr_f, tcammask, &ipv6_mask);
            IPV6_MASK_TO_LEN(ipv6_mask, p_key->mask_len);
            GetDsLpmTcamIpv6DoubleKey0(A, vrfId_f, tcammask, &p_key->vrfId_mask);
        }
    }

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: read %s[%d]\n", TABLE_NAME(lchip, da_table), key_idx);

    return CTC_E_NONE;

}

int32
sys_tsingma_ipuc_tcam_write_ad(uint8 lchip, sys_ipuc_tcam_data_t *p_data)
{
    sys_ipuc_info_t  *p_ipuc_info    = NULL;
    sys_ipuc_tcam_ad_t ad;
    uint8 ip_ver = CTC_IP_VER_4;
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    CTC_PTR_VALID_CHECK(p_data->info);

    p_ipuc_info = (sys_ipuc_info_t*)p_data->info;
    ip_ver = SYS_IPUC_VER(p_ipuc_info);

    sal_memset(&ad, 0 , sizeof(ad));
    ad.tcamAd_key_idx = p_ipuc_info->key_index;
    ad.nexthop = p_ipuc_info->ad_index;

    if (CTC_FLAG_ISSET(p_ipuc_info->route_flag, SYS_IPUC_FLAG_PUBLIC_ROUTE))
    {
         route_mode = SYS_PUBLIC_MODE;
    }

    CTC_ERROR_RETURN(sys_tsingma_ipuc_tcam_write_ad2hw( lchip,  ip_ver,  route_mode, &ad,  p_data->opt_type));

    return CTC_E_NONE;

}

int32
sys_tsingma_ipuc_tcam_read_ad(uint8 lchip, ctc_ip_ver_t ip_ver, sys_ipuc_route_mode_t route_mode, sys_ipuc_tcam_ad_t* p_ad)
{
    ds_t lpmtcam_ad;
    sys_ipuc_tbl_type_t tbl_type;
    uint32 ad_table;
    uint32 cmd = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (p_ad->tcamAd_key_idx== INVALID_NEXTHOP_OFFSET)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "tcamAd_key_idx is invalid %d\r\n", p_ad->tcamAd_key_idx);
        return CTC_E_INVALID_PARAM;
    }

    if(SYS_PUBLIC_MODE == route_mode)
    {
        if((ip_ver == CTC_IP_VER_6) && (IS_SHORT_KEY(route_mode, p_ad->tcamAd_key_idx)))
        {
            tbl_type = SYS_TBL_TCAM_PUB_SHORT;
        }
        else
        {
            tbl_type = SYS_TBL_TCAM_PUB;
        }
    }
    else
    {
        if((ip_ver == CTC_IP_VER_6) && (IS_SHORT_KEY(route_mode, p_ad->tcamAd_key_idx)))
        {
            tbl_type = SYS_TBL_TCAM_SHORT;
        }
        else
        {
            tbl_type = SYS_TBL_TCAM;
        }
    }

    sal_memset(&lpmtcam_ad, 0, sizeof(lpmtcam_ad));
    sys_usw_get_tbl_id(lchip, ip_ver, 1, tbl_type, NULL, &ad_table);
    cmd = DRV_IOR(ad_table, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, p_ad->tcamAd_key_idx, cmd, &lpmtcam_ad);

    p_ad->nexthop = GetDsLpmTcamAd(V, nexthop_f, &lpmtcam_ad);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: nexthop_f[%x] \n",p_ad->nexthop);
    p_ad->pointer = GetDsLpmTcamAd(V, pointer_f, &lpmtcam_ad);
    p_ad->lpmPipelineValid = GetDsLpmTcamAd(V, lpmPipelineValid_f, &lpmtcam_ad);
    p_ad->indexMask = GetDsLpmTcamAd(V, indexMask_f, &lpmtcam_ad);
    p_ad->lpmStartByte = GetDsLpmTcamAd(V, lpmStartByte_f, &lpmtcam_ad);

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: read %s[%d] \n", TABLE_NAME(lchip, ad_table), p_ad->tcamAd_key_idx);

    return CTC_E_NONE;
}

int32
sys_tsingma_ipuc_tcam_write_ad2hw(uint8 lchip, ctc_ip_ver_t ip_ver, sys_ipuc_route_mode_t route_mode, sys_ipuc_tcam_ad_t* p_ad, sys_ipuc_opt_type_t opt)
{

    ds_t lpmtcam_ad;
    sys_ipuc_tbl_type_t tbl_type;
    uint32 ad_table, sa_ad_table;
    uint32 cmd = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (p_ad->tcamAd_key_idx== INVALID_NEXTHOP_OFFSET)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "tcamAd_key_idx is invalid %d\r\n", p_ad->tcamAd_key_idx);
        return CTC_E_INVALID_PARAM;
    }
    if(opt != DO_ADD && opt != DO_UPDATE)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "opt invalid \r\n");
        return CTC_E_NONE;
    }

    if(SYS_PUBLIC_MODE == route_mode)
    {
        if((ip_ver == CTC_IP_VER_6) && (IS_SHORT_KEY(route_mode, p_ad->tcamAd_key_idx)))
        {
            tbl_type = SYS_TBL_TCAM_PUB_SHORT;
        }
        else
        {
            tbl_type = SYS_TBL_TCAM_PUB;
        }
    }
    else
    {
        if((ip_ver == CTC_IP_VER_6) && (IS_SHORT_KEY(route_mode, p_ad->tcamAd_key_idx)))
        {
            tbl_type = SYS_TBL_TCAM_SHORT;
        }
        else
        {
            tbl_type = SYS_TBL_TCAM;
        }
    }

    sal_memset(&lpmtcam_ad, 0, sizeof(lpmtcam_ad));
    sys_usw_get_tbl_id(lchip, ip_ver, 1, tbl_type, NULL, &ad_table);
    cmd = DRV_IOR(ad_table, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, p_ad->tcamAd_key_idx, cmd, &lpmtcam_ad);
    SetDsLpmTcamAd(V, nexthop_f, &lpmtcam_ad, p_ad->nexthop);
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: nexthop_f[%x] \n", p_ad->nexthop);
    if(opt == DO_ADD)
    {
        SetDsLpmTcamAd(V, pointer_f, &lpmtcam_ad, p_ad->pointer);
        SetDsLpmTcamAd(V, lpmPipelineValid_f, &lpmtcam_ad, p_ad->lpmPipelineValid);
        SetDsLpmTcamAd(V, indexMask_f, &lpmtcam_ad, p_ad->indexMask);
        SetDsLpmTcamAd(V, lpmStartByte_f, &lpmtcam_ad, p_ad->lpmStartByte);
    }
    cmd = DRV_IOW(ad_table, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, p_ad->tcamAd_key_idx, cmd, &lpmtcam_ad);

    if(CTC_FLAG_ISSET(p_usw_ipuc_master[lchip]->lookup_mode[ip_ver], SYS_TCAM_SA_LOOKUP))
    {
        sys_usw_get_tbl_id(lchip, ip_ver, 0, tbl_type, NULL, &sa_ad_table);

        cmd = DRV_IOW(sa_ad_table, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, p_ad->tcamAd_key_idx, cmd, &lpmtcam_ad);
    }
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPUC_TCAM: update %s[%d] \n", TABLE_NAME(lchip, ad_table), p_ad->tcamAd_key_idx);

    return CTC_E_NONE;
}

int32
sys_tsingma_ipuc_tcam_show_key(uint8 lchip, sys_ipuc_tcam_data_t *p_data)
{
    sys_ipuc_route_mode_t route_mode = SYS_PRIVATE_MODE;
    sys_ipuc_tbl_type_t tbl_type = 0;
    uint32 key_id = 0;
    uint32 ad_id = 0;
    uint16 key_index = 0;
    uint16 tcam_ad_index = 0;
    uint8 ip_ver = CTC_IP_VER_4;
    sys_ipuc_info_t *p_ipuc_info = NULL;
    uint8 real_lchip = 0;
    uint16 real_index = 0;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    if (p_data->key_type == SYS_IPUC_TCAM_FLAG_TCAM)
    {
        CTC_PTR_VALID_CHECK(p_data->info);
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
        CTC_PTR_VALID_CHECK(p_data->ipuc_param);
        key_index = p_data->key_index;
        ip_ver = p_data->ipuc_param->ip_ver;
    }
    tcam_ad_index = key_index;

    if(p_usw_ipuc_master[lchip]->rchain_en)
    {
        _sys_tsingma_nalpm_get_real_tcam_index(lchip, tcam_ad_index, &real_lchip, &real_index);
        if (0xff == real_lchip)
        {
            return CTC_E_INTR_INVALID_PARAM;
        }
        tcam_ad_index = real_index;
        key_index = real_index;
    }

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

    if(p_usw_ipuc_master[lchip]->rchain_en)
    {
        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " --%-30s:%d\n", "Real lchip", real_lchip);
    }

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
sys_tsingma_ipuc_tcam_show_status(uint8 lchip)
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
        if (p_usw_ipuc_master[lchip]->rchain_en)
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15d%-4s",  total_size + SYS_IPUC_LPM_TCAM_MAX_INDEX*p_usw_ipuc_master[lchip]->rchain_tail, " ");
        }
        else
        {
            SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15d%-4s",  total_size, " ");
        }
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
                        SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-15s%-4s",  "-", " ");
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
_sys_tsingma_ipuc_tcam_init_public(uint8 lchip, void* ofb_cb)
{
    uint8 step = 0;
    uint8 block_id = 0;
    uint8 ofb_type = 0;
    uint8 tmp_block_max = 0;
    uint16 block_max[MAX_CTC_IP_VER] = {0};
    uint32 total_size = 0;
    uint32 table_size = 0;
    sys_ipuc_route_mode_t route_mode = SYS_PUBLIC_MODE;
    sys_usw_ofb_param_t ofb_param = {0};
    ofb_param.ofb_cb = ofb_cb;

    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (p_usw_ipuc_master[lchip]->max_entry_num[CTC_IP_VER_6][SYS_TBL_TCAM_PUB])
    {/* don't enable tcam ipv6 short key*/
        _sys_tsingma_nalpm_get_tcam_block_num(lchip, CTC_IP_VER_6, route_mode, &block_max[CTC_IP_VER_6]);
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
        _sys_tsingma_nalpm_get_tcam_block_num(lchip, CTC_IP_VER_4, route_mode, &block_max[CTC_IP_VER_4]);
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
sys_tsingma_ipuc_tcam_init(uint8 lchip, sys_ipuc_route_mode_t route_mode, void* ofb_cb_fn)
{
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    if (route_mode == SYS_PRIVATE_MODE)
    {
        return _sys_tsingma_ipuc_tcam_init_private(lchip, ofb_cb_fn);
    }
    else
    {
        return _sys_tsingma_ipuc_tcam_init_public(lchip, ofb_cb_fn);
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_ipuc_tcam_deinit(uint8 lchip, sys_ipuc_route_mode_t route_mode)
{
    SYS_IPUC_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    if (p_usw_ipuc_master[lchip]->share_ofb_type[route_mode])
    {
        sys_usw_ofb_deinit(lchip, p_usw_ipuc_master[lchip]->share_ofb_type[route_mode]);
    }

    return CTC_E_NONE;
}

