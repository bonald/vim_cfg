#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_bfd_db.c

 @date 2013-01-24

 @version v2.0

  This file contains oam sys layer function implementation
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_oam.h"
#include "ctc_error.h"
#include "ctc_common.h"
#include "ctc_linklist.h"
#include "ctc_hash.h"
#include "ctc_parser.h"

#include "sys_usw_common.h"
#include "sys_usw_chip.h"

#include "sys_usw_ftm.h"

#include "sys_usw_nexthop_api.h"
#include "sys_usw_mpls.h"
#include "sys_usw_oam.h"
#include "sys_usw_oam_db.h"
#include "sys_usw_oam_com.h"
#include "sys_usw_oam_debug.h"
#include "sys_usw_oam_bfd_db.h"

#include "drv_api.h"

/****************************************************************************
*
* Defines and Macros
*
*****************************************************************************/

extern sys_oam_master_t* g_oam_master[CTC_MAX_LOCAL_CHIP_NUM];
extern int32 sys_usw_global_ctl_get(uint8 lchip, ctc_global_control_type_t type, void* value);


/****************************************************************************
*
* Function
*
**
***************************************************************************/

#define IPV6_EDIT
int32
_sys_usw_bfd_alloc_ipv6_idx(uint8 lchip, void* p_add, uint8* p_index)
{
    int32 ret = CTC_E_NONE;
    uint8 idx = 0;
    uint8 max_idx = 0;
    sys_bfd_ip_addr_v6_node_t* p_node = NULL;
    sys_bfd_ip_addr_v6_node_t* p_free = NULL;

    uint8 free_idx = SYS_OAM_BFD_INVALID_IPV6_NUM;
    int32 match = 1;
    uint32 cmd;
    DsBfdV6Addr_m ds_bfd_v6_addr;
    ipv6_addr_t tmp = {0};

    if (NULL == g_oam_master[lchip])
    {
        return CTC_E_NONE;
    }

    sal_memset(&ds_bfd_v6_addr, 0, sizeof(DsBfdV6Addr_m));
    max_idx = MCHIP_CAP(SYS_CAP_OAM_BFD_IPV6_MAX_IPSA_NUM);

    p_node = (sys_bfd_ip_addr_v6_node_t*)g_oam_master[lchip]->ipv6_bmp ;
    *p_index = SYS_OAM_BFD_INVALID_IPV6_NUM;

    for (idx = 0; idx <  max_idx ; idx++)
    {
        if ((p_node->count == 0) && (free_idx == SYS_OAM_BFD_INVALID_IPV6_NUM))
        {
            free_idx = idx;
            p_free = p_node;
            break;
        }
        else if (p_node->count != 0)
        {
            match = sal_memcmp(&(p_node->ipv6), p_add, sizeof(ipv6_addr_t));
        }

        if (0 == match)
        {
            *p_index = idx;
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Use old IPV6 INDEX = %d old count is %d \n", *p_index, p_node->count);
            p_node->count++;
            return CTC_E_NONE;
        }
        p_node++;

    }

    if (SYS_OAM_BFD_INVALID_IPV6_NUM != free_idx)
    {
        *p_index = free_idx;
        p_free->count++;
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_BFDV6ADDR, 1, 1);
        sal_memcpy(&(p_free->ipv6), p_add, (sizeof(ipv6_addr_t)));
    }
    else
    {
        return CTC_E_NO_RESOURCE;
    }

    cmd = DRV_IOW(DsBfdV6Addr_t, DRV_ENTRY_FLAG);
    p_node = (sys_bfd_ip_addr_v6_node_t*)p_free;
    tmp[0] = p_node->ipv6[3];
    tmp[1] = p_node->ipv6[2];
    tmp[2] = p_node->ipv6[1];
    tmp[3] = p_node->ipv6[0];
    SetDsBfdV6Addr(A, addr_f, &ds_bfd_v6_addr, tmp);

    ret = DRV_IOCTL(lchip, free_idx, cmd, &ds_bfd_v6_addr);
    if (ret < 0)
    {
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_BFDV6ADDR, 0, 1);
    }
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Add new IPV6 INDEX = %d \n", free_idx);

    return ret;
}

int32
_sys_usw_bfd_free_ipv6_idx(uint8 lchip, uint8 index)
{
    uint8 idx = 0;
    uint8 max_idx = 0;
    sys_bfd_ip_addr_v6_node_t* p_node;
    uint32 cmd = 0;
    DsBfdV6Addr_m ds_bfd_v6_addr;

    if (index == SYS_OAM_BFD_INVALID_IPV6_NUM)
    {
        return CTC_E_NONE;
    }

    if (index > MCHIP_CAP(SYS_CAP_OAM_BFD_IPV6_MAX_IPSA_NUM))
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&ds_bfd_v6_addr, 0, sizeof(DsBfdV6Addr_m));
    max_idx =  MCHIP_CAP(SYS_CAP_OAM_BFD_IPV6_MAX_IPSA_NUM);
    p_node = (sys_bfd_ip_addr_v6_node_t*)g_oam_master[lchip]->ipv6_bmp;

    for (idx = 0; idx < max_idx; idx++)
    {
        if (idx == index)
        {
            break;
        }

        p_node++;
    }

    if (0 == p_node->count)
    {
        return CTC_E_NONE;
    }

    p_node->count--;
    if (0 == p_node->count)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Free IPV6 INDEX = %d \n", index);
        sal_memset(&(p_node->ipv6), 0, (sizeof(ipv6_addr_t)));
        cmd = DRV_IOW(DsBfdV6Addr_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_bfd_v6_addr));
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_BFDV6ADDR, 0, 1);
        return CTC_E_NONE;
    }

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "IPV6 INDEX = %d Cannot freee for counter is %d \n", index, p_node->count);
    return CTC_E_NONE;
}


#define CHAN "Function Begin"

sys_oam_chan_bfd_t*
_sys_usw_ip_bfd_chan_lkup(uint8 lchip, ctc_oam_key_t* p_key_parm)
{
    sys_oam_chan_bfd_t sys_oam_bfd_chan;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&sys_oam_bfd_chan, 0, sizeof(sys_oam_chan_bfd_t));
    sys_oam_bfd_chan.key.com.mep_type   = p_key_parm->mep_type;
    sys_oam_bfd_chan.key.my_discr       = p_key_parm->u.bfd.discr;
    sys_oam_bfd_chan.com.mep_type        = p_key_parm->mep_type;
    if (!DRV_IS_DUET2(lchip))
    {
        sys_oam_bfd_chan.key.is_sbfd_reflector = CTC_FLAG_ISSET(p_key_parm->flag, CTC_OAM_KEY_FLAG_SBFD_REFLECTOR);
    }


    return (sys_oam_chan_bfd_t*)_sys_usw_oam_chan_lkup(lchip, &sys_oam_bfd_chan.com);
}

sys_oam_chan_bfd_t*
_sys_usw_ip_bfd_chan_build_node(uint8 lchip, ctc_oam_lmep_t* p_chan_param)
{
    sys_oam_chan_bfd_t* p_sys_chan_bfd = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sys_chan_bfd = (sys_oam_chan_bfd_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_chan_bfd_t));
    if (NULL != p_sys_chan_bfd)
    {
        sal_memset(p_sys_chan_bfd, 0, sizeof(sys_oam_chan_bfd_t));
        p_sys_chan_bfd->com.mep_type        = p_chan_param->key.mep_type;
        p_sys_chan_bfd->com.master_chipid   = p_chan_param->u.bfd_lmep.master_gchip;

        p_sys_chan_bfd->key.com.mep_type    = p_chan_param->key.mep_type;
        p_sys_chan_bfd->key.my_discr        = p_chan_param->key.u.bfd.discr;
        if (!DRV_IS_DUET2(lchip))
        {
            p_sys_chan_bfd->key.is_sbfd_reflector = CTC_FLAG_ISSET(p_chan_param->key.flag, CTC_OAM_KEY_FLAG_SBFD_REFLECTOR);
        }
    }

    return p_sys_chan_bfd;
}

int32
_sys_usw_ip_bfd_chan_free_node(uint8 lchip, sys_oam_chan_bfd_t* p_sys_chan_bfd)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    mem_free(p_sys_chan_bfd);
    p_sys_chan_bfd = NULL;

    return CTC_E_NONE;
}

int32
_sys_usw_ip_bfd_chan_build_index(uint8 lchip, sys_oam_chan_bfd_t* p_sys_chan_bfd)
{
    DsEgressXcOamBfdHashKey_m oam_key;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_chan_bfd);

    sal_memset(&oam_key, 0, sizeof(DsEgressXcOamBfdHashKey_m));
    SetDsEgressXcOamBfdHashKey(V, hashType_f, &oam_key, EGRESSXCOAMHASHTYPE_BFD);
    SetDsEgressXcOamBfdHashKey(V, valid_f, &oam_key, 1);
    SetDsEgressXcOamBfdHashKey(V, myDiscriminator_f, &oam_key, p_sys_chan_bfd->key.my_discr);
    SetDsEgressXcOamBfdHashKey(V, isSbfdReflector_f, &oam_key, p_sys_chan_bfd->key.is_sbfd_reflector);
    CTC_ERROR_RETURN( sys_usw_oam_key_lookup_io(lchip, DsEgressXcOamBfdHashKey_t,
                                                       &p_sys_chan_bfd->key.com.key_index , & oam_key));

    p_sys_chan_bfd->key.com.key_alloc = SYS_OAM_KEY_HASH;

    return CTC_E_NONE;
}

int32
_sys_usw_ip_bfd_chan_free_index(uint8 lchip, sys_oam_chan_bfd_t* p_sys_chan_bfd)
{
    DsEgressXcOamBfdHashKey_m oam_key;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_chan_bfd);

    if (p_sys_chan_bfd->key.com.key_alloc == SYS_OAM_KEY_HASH)
    {
        sal_memset(&oam_key, 0, sizeof(DsEgressXcOamBfdHashKey_m));
        SetDsEgressXcOamBfdHashKey(V, valid_f, &oam_key, 0);
        CTC_ERROR_RETURN( sys_usw_oam_key_delete_io(lchip, DsEgressXcOamBfdHashKey_t,
                                                           p_sys_chan_bfd->key.com.key_index , &oam_key));

    }

    return CTC_E_NONE;
}


int32
_sys_usw_ip_bfd_chan_add_to_asic(uint8 lchip, sys_oam_chan_bfd_t* p_sys_chan_bfd)
{
    DsEgressXcOamBfdHashKey_m ds_bfd_oam_key;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_sys_chan_bfd);

    if (p_sys_chan_bfd->key.com.key_alloc == SYS_OAM_KEY_HASH)
    {
        sal_memset(&ds_bfd_oam_key, 0, sizeof(DsEgressXcOamBfdHashKey_m));

        SetDsEgressXcOamBfdHashKey(V, hashType_f,&ds_bfd_oam_key,EGRESSXCOAMHASHTYPE_BFD);
        SetDsEgressXcOamBfdHashKey(V, valid_f,&ds_bfd_oam_key,1);
        SetDsEgressXcOamBfdHashKey(V, myDiscriminator_f,&ds_bfd_oam_key,p_sys_chan_bfd->key.my_discr);
        SetDsEgressXcOamBfdHashKey(V, oamDestChipId_f,&ds_bfd_oam_key,p_sys_chan_bfd->com.master_chipid);
        SetDsEgressXcOamBfdHashKey(V, isSbfdReflector_f,&ds_bfd_oam_key,p_sys_chan_bfd->key.is_sbfd_reflector);

        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"KEY: ds_bfd_oam_key.my_discriminator = 0x%x\n", p_sys_chan_bfd->key.my_discr);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"KEY: ds_bfd_oam_key.hash_type        = 0x%x\n",           EGRESSXCOAMHASHTYPE_BFD);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"KEY: ds_bfd_oam_key.valid            = 0x%x\n",               1);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"KEY: ds_bfd_oam_key.oam_dest_chip_id = 0x%x\n",  p_sys_chan_bfd->com.master_chipid);

        CTC_ERROR_RETURN( sys_usw_oam_key_write_io(lchip, DsEgressXcOamBfdHashKey_t,
                                                          p_sys_chan_bfd->key.com.key_index , &ds_bfd_oam_key));
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 1, 1);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "alloc DsEgressXcOamBfdHashKey, key_index:%d \n", p_sys_chan_bfd->key.com.key_index);

    }

    return CTC_E_NONE;

}

int32
_sys_usw_ip_bfd_chan_del_from_asic(uint8 lchip, sys_oam_chan_bfd_t* p_sys_chan_bfd)
{
    DsEgressXcOamBfdHashKey_m ds_bfd_oam_key;
    uint32 index      = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_chan_bfd);

    if (p_sys_chan_bfd->key.com.key_alloc == SYS_OAM_KEY_HASH)
    {
        sal_memset(&ds_bfd_oam_key, 0, sizeof(DsEgressXcOamBfdHashKey_m));
        index = p_sys_chan_bfd->key.com.key_index;
        CTC_ERROR_RETURN( sys_usw_oam_key_delete_io(lchip, DsEgressXcOamBfdHashKey_t,
                                                          index , &ds_bfd_oam_key));
        SYS_OAM_TBL_CNT(lchip, SYS_OAM_TBL_LOOKUP_KEY, 0, 1);
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "free DsEgressXcOamBfdHashKey, key_index:%d \n", index);
    }

    return CTC_E_NONE;
}

int32
_sys_usw_ip_bfd_add_chan(uint8 lchip, void* p_chan, void** p_sys_chan)
{
    ctc_oam_lmep_t* p_chan_param   = (ctc_oam_lmep_t*)p_chan;
    sys_oam_chan_bfd_t* p_sys_chan_bfd = NULL;
    int32 ret = CTC_E_NONE;

    /*1. lookup chan and check exist*/
    p_sys_chan_bfd = _sys_usw_ip_bfd_chan_lkup(lchip, &p_chan_param->key);
    if (NULL != p_sys_chan_bfd)
    {
        ret = CTC_E_EXIST;
        (*p_sys_chan) = p_sys_chan_bfd;
        goto RETURN;
    }

    /*2. build chan sys node*/
    p_sys_chan_bfd = _sys_usw_ip_bfd_chan_build_node(lchip, p_chan_param);
    if (NULL == p_sys_chan_bfd)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: build node fail!!\n");
        ret = CTC_E_NO_MEMORY;
        goto RETURN;
    }

    /*3. build chan index*/
    ret = _sys_usw_ip_bfd_chan_build_index(lchip, p_sys_chan_bfd);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: build index fail!!\n");
        goto STEP_3;
    }

    /*4. add chan to db*/
    ret = _sys_usw_oam_chan_add_to_db(lchip, &p_sys_chan_bfd->com);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: add db fail!!\n");
        goto STEP_4;
    }

    /*5. add key/chan to asic*/
    ret = _sys_usw_ip_bfd_chan_add_to_asic(lchip, p_sys_chan_bfd);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: add asic fail!!\n");
        goto STEP_5;
    }

    (*p_sys_chan) = p_sys_chan_bfd;

    goto RETURN;

STEP_5:
    _sys_usw_oam_chan_del_from_db(lchip, &p_sys_chan_bfd->com);
STEP_4:
    _sys_usw_ip_bfd_chan_free_index(lchip, p_sys_chan_bfd);
STEP_3:
    _sys_usw_ip_bfd_chan_free_node(lchip, p_sys_chan_bfd);

RETURN:
    return ret;
}


int32
_sys_usw_ip_bfd_remove_chan(uint8 lchip, void* p_chan)
{
    ctc_oam_lmep_t* p_chan_param = (ctc_oam_lmep_t*)p_chan;
    sys_oam_chan_bfd_t* p_sys_chan = NULL;
    int32 ret = CTC_E_NONE;

    /*1. lookup chan and check exist*/
    p_sys_chan = _sys_usw_ip_bfd_chan_lkup(lchip, &p_chan_param->key);
    if (NULL == p_sys_chan)
    {
        ret = CTC_E_NOT_EXIST;
        goto RETURN;
    }

    if (CTC_SLISTCOUNT(p_sys_chan->com.lmep_list) > 0)
    { /*need consider mip*/
        ret = CTC_E_EXIST;
        goto RETURN;
    }

    /*2. remove key/chan from asic*/
    ret = _sys_usw_ip_bfd_chan_del_from_asic(lchip, p_sys_chan);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: del asic fail!!\n");
        goto RETURN;
    }

    /*3. remove chan from db*/
    ret = _sys_usw_oam_chan_del_from_db(lchip, &p_sys_chan->com);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: del db fail!!\n");
        goto RETURN;
    }

    /*4. free chan node*/
    ret = _sys_usw_ip_bfd_chan_free_node(lchip, p_sys_chan);
    if (CTC_E_NONE != ret)
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR,"Chan: free index fail!!\n");
        goto RETURN;
    }

RETURN:
    return ret;
}

int32
_sys_usw_bfd_add_chan(uint8 lchip, void* p_chan, void** p_sys_chan)
{
    ctc_oam_lmep_t *p_chan_param = NULL;
    uint8 mep_type  = CTC_OAM_MEP_TYPE_MAX;
    int32 ret       = CTC_E_NONE;

    p_chan_param    = (ctc_oam_lmep_t *)p_chan;
    mep_type        = p_chan_param->key.mep_type;

    if (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
    {
        ret = _sys_usw_tp_add_chan(lchip, p_chan_param, p_sys_chan);
    }
    else if ((CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type) || (CTC_OAM_MEP_TYPE_IP_BFD == mep_type)
        || (CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type))
    {

        ret = _sys_usw_ip_bfd_add_chan(lchip, p_chan_param, p_sys_chan);
    }

    return ret;

}

int32
_sys_usw_bfd_remove_chan(uint8 lchip, void* p_chan)
{
    ctc_oam_lmep_t *p_chan_param = NULL;
    uint8 mep_type  = CTC_OAM_MEP_TYPE_MAX;
    int32 ret       = CTC_E_NONE;

    p_chan_param    = (ctc_oam_lmep_t *)p_chan;
    mep_type        = p_chan_param->key.mep_type;

    if (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
    {
        ret = _sys_usw_tp_remove_chan(lchip, p_chan_param);
    }
    else if ((CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type) || (CTC_OAM_MEP_TYPE_IP_BFD == mep_type)
        || (CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type))
    {
        ret = _sys_usw_ip_bfd_remove_chan(lchip, p_chan_param);
    }

    return ret;
}


int32
_sys_usw_bfd_update_chan(uint8 lchip, sys_oam_lmep_t* p_sys_lmep_bfd, bool is_add, ctc_oam_lmep_t*  p_oam_lmep)
{
    sys_oam_chan_tp_t*  p_sys_chan_tp   = NULL;
    sys_oam_chan_bfd_t* p_sys_chan_bfd  = NULL;
    sys_oam_chan_com_t* p_sys_chan_com  = NULL;
    sys_oam_key_com_t  temp_com;
    sys_oam_key_com_t*  p_key_com       = &temp_com;

    DsEgressXcOamBfdHashKey_m ds_bfd_oam_key;
    DsEgressXcOamMplsLabelHashKey_m mpls_oam_key;
    DsEgressXcOamMplsSectionHashKey_m section_oam_key;

    void* p_hashkey = &ds_bfd_oam_key;

    uint8  mep_type     =  CTC_OAM_MEP_TYPE_MAX;
    uint32 index        = 0;
    uint32 tbl_id = 0;
    uint32  lm_en        = 0;
    uint32  oam_check_type = SYS_OAM_TP_NONE_TYPE;
    uint32 mep_index_in_key = 0;
	uint32 tmp[2] = {0};
    uint32 ret = 0;
    sys_mpls_oam_t mpls_info;
    uint32 mode = 0;
    sys_nh_update_oam_info_t oam_info;


    sal_memset(p_key_com, 0, sizeof(sys_oam_key_com_t));
    sal_memset(p_hashkey, 0, sizeof(DsEgressXcOamBfdHashKey_m));
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    mep_type    = p_sys_lmep_bfd->p_sys_chan->mep_type;

    p_sys_chan_com = p_sys_lmep_bfd->p_sys_chan;

    if (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
    {
        p_sys_chan_tp   = (sys_oam_chan_tp_t*)p_sys_chan_com;
        p_key_com       = &p_sys_chan_tp->key.com;
        /*mep index*/
        if (is_add)
        {
            p_sys_chan_tp->mep_index = p_sys_lmep_bfd->lmep_index;
            p_sys_chan_tp->mep_index_in_key = p_sys_lmep_bfd->lmep_index/2;
            oam_check_type = SYS_OAM_TP_MEP_TYPE;
             /* mpls tp use label build hash table for mep_index as key!*/
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"Chan: MPLS TP Update mepindex[0x%x] in DsEgressXcOamMplsLabelHashKey_t\n", p_sys_chan_tp->mep_index_in_key);

            if (!CTC_FLAG_ISSET(p_sys_lmep_bfd->flag, CTC_OAM_BFD_LMEP_FLAG_MEP_ON_CPU)
                || (p_sys_chan_tp->key.section_oam))
            {
                CTC_ERROR_RETURN(_sys_usw_tp_chan_build_index(lchip, p_sys_chan_tp));
                CTC_ERROR_RETURN(_sys_usw_tp_chan_add_to_asic(lchip, p_sys_chan_tp));
            }
        }
        else
        {
            p_sys_chan_tp->mep_index = 0x1FFF;
            p_sys_chan_tp->mep_index_in_key = 0x1FFF;
            oam_check_type = SYS_OAM_TP_NONE_TYPE;
        }
        mep_index_in_key = p_sys_chan_tp->mep_index_in_key;

        if ((p_sys_chan_tp->key.section_oam)
            && (!g_oam_master[lchip]->tp_section_oam_based_l3if)
            && (!CTC_IS_LINKAGG_PORT(p_sys_chan_tp->key.gport_l3if_id))
            && (FALSE == sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(p_sys_chan_tp->key.gport_l3if_id))))
        {
            return CTC_E_NONE;
        }

    }
    else if ((CTC_OAM_MEP_TYPE_IP_BFD == mep_type) || (CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type)
        || (CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type))
    {
        p_sys_chan_bfd  = (sys_oam_chan_bfd_t*)p_sys_chan_com;
        p_key_com       = &p_sys_chan_bfd->key.com;
        /*mep index*/
        if (is_add)
        {
            p_sys_chan_bfd->mep_index = p_sys_lmep_bfd->lmep_index;
            oam_check_type = SYS_OAM_TP_MEP_TYPE;
        }
        else
        {
            p_sys_chan_bfd->mep_index = 0x1FFF;
            oam_check_type = SYS_OAM_TP_NONE_TYPE;
        }
    }



    /*lm enable*/
    if (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
    {
        if (CTC_FLAG_ISSET(p_sys_lmep_bfd->flag, CTC_OAM_BFD_LMEP_FLAG_LM_EN) && is_add)
        {
            lm_en   = 1;
        }
        else
        {
            lm_en   = 0;
        }
    }

    if (p_key_com && (SYS_OAM_KEY_HASH == p_key_com->key_alloc))
    {
        if ((CTC_OAM_MEP_TYPE_IP_BFD == mep_type) || (CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type)
            || (CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type))
        {
            sal_memset(&ds_bfd_oam_key, 0, sizeof(DsEgressXcOamBfdHashKey_m));
            p_hashkey  = &ds_bfd_oam_key;
            tbl_id = DsEgressXcOamBfdHashKey_t;
        }
        else if(CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
        {
            if (p_sys_chan_tp->key.section_oam)
            {
                sal_memset(&section_oam_key, 0, sizeof(DsEgressXcOamMplsSectionHashKey_m));
                p_hashkey = &section_oam_key;
                tbl_id = DsEgressXcOamMplsSectionHashKey_t;
            }
            else
            {
                sal_memset(&mpls_oam_key, 0, sizeof(DsEgressXcOamMplsLabelHashKey_m));
                p_hashkey = &mpls_oam_key;
                tbl_id = DsEgressXcOamMplsLabelHashKey_t;
            }
        }
    }


    index = p_key_com->key_index;
    CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, tbl_id, index, p_hashkey));

    if ((CTC_OAM_MEP_TYPE_IP_BFD == mep_type) || (CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type)
        || (CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type))
    {
        SetDsEgressXcOamBfdHashKey(V, mepIndex_f, &ds_bfd_oam_key, p_sys_lmep_bfd->lmep_index);
        SetDsEgressXcOamBfdHashKey(V, bfdSingleHop_f, &ds_bfd_oam_key,
                                   CTC_FLAG_ISSET(p_sys_lmep_bfd->flag, CTC_OAM_BFD_LMEP_FLAG_IP_SINGLE_HOP));
        SetDsEgressXcOamBfdHashKey(V, bfdBindingEn_f, &ds_bfd_oam_key,
                                   CTC_FLAG_ISSET(p_sys_lmep_bfd->flag, CTC_OAM_BFD_LMEP_FLAG_BINDING_EN));
        if ((CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type) && CTC_FLAG_ISSET(p_sys_lmep_bfd->flag, CTC_OAM_BFD_LMEP_FLAG_BINDING_EN))
        {
            SetDsEgressXcOamBfdHashKey(V, bfdBindingType_f, &ds_bfd_oam_key, 2);
            SetDsEgressXcOamBfdHashKey(V, u1_mpls_mplsOamIndex_f, &ds_bfd_oam_key, p_sys_lmep_bfd->lmep_index/2);
        }
        else if (CTC_FLAG_ISSET(p_sys_lmep_bfd->flag, CTC_OAM_BFD_LMEP_FLAG_IPV6_ENCAP)
            && CTC_FLAG_ISSET(p_sys_lmep_bfd->flag, CTC_OAM_BFD_LMEP_FLAG_BINDING_EN))
        {
            SetDsEgressXcOamBfdHashKey(V, bfdBindingType_f, &ds_bfd_oam_key, 1);
            tmp[0] = p_oam_lmep->u.bfd_lmep.ipv6_da[3];
            tmp[1] = p_oam_lmep->u.bfd_lmep.ipv6_da[2];
            SetDsEgressXcOamBfdHashKey(A, u1_ipv6_ipv6Sa_f, &ds_bfd_oam_key, tmp);
        }
        else if(CTC_FLAG_ISSET(p_sys_lmep_bfd->flag, CTC_OAM_BFD_LMEP_FLAG_BINDING_EN))
        {
            SetDsEgressXcOamBfdHashKey(V, u1_ipv4_ipSa_f, &ds_bfd_oam_key, p_oam_lmep->u.bfd_lmep.ip4_da);
        }
    }
    else
    {
        if (p_sys_chan_tp->key.section_oam)
        {
            SetDsEgressXcOamMplsSectionHashKey(V, mepIndex_f, p_hashkey, p_sys_lmep_bfd->lmep_index);
            if (lm_en)
            {
                SetDsEgressXcOamMplsSectionHashKey(V, lmIndexBase_f       , p_hashkey, p_sys_chan_tp->com.lm_index_base) ;
                SetDsEgressXcOamMplsSectionHashKey(V, lmCos_f           , p_hashkey, p_sys_chan_tp->lm_cos) ;
                SetDsEgressXcOamMplsSectionHashKey(V, lmType_f            , p_hashkey, p_sys_chan_tp->lm_type) ;
                SetDsEgressXcOamMplsSectionHashKey(V, mplsLmType_f            , p_hashkey, 1) ;/*64bits stats*/
            }
        }
        else
        {
            SetDsEgressXcOamMplsLabelHashKey(V, mepIndex_f          ,  p_hashkey, p_sys_chan_tp->mep_index)
            if (lm_en)
            {
                SetDsEgressXcOamMplsLabelHashKey(V, lmIndexBase_f       , p_hashkey, p_sys_chan_tp->com.lm_index_base)
                SetDsEgressXcOamMplsLabelHashKey(V, lmCos_f           , p_hashkey, p_sys_chan_tp->lm_cos) ;
                SetDsEgressXcOamMplsLabelHashKey(V, lmType_f            , p_hashkey, p_sys_chan_tp->lm_type  ) ;
                SetDsEgressXcOamMplsLabelHashKey(V, lmCosType_f         , p_hashkey,  p_sys_chan_tp->lm_cos_type) ;
                SetDsEgressXcOamMplsLabelHashKey(V, mplsLmType_f         , p_hashkey, 1) ;/*64bits stats*/
            }
        }
    }

    if ((CTC_OAM_MEP_TYPE_IP_BFD == mep_type) || (CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type)
        || (CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type))
    {
        CTC_ERROR_RETURN(sys_usw_oam_key_write_io(lchip, tbl_id, index, p_hashkey));
    }
    else if((CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
        &&((!CTC_FLAG_ISSET(p_sys_lmep_bfd->flag, CTC_OAM_BFD_LMEP_FLAG_MEP_ON_CPU))||(p_sys_chan_tp->key.section_oam)))
    {
        CTC_ERROR_RETURN(sys_usw_oam_key_write_io(lchip, tbl_id, index, p_hashkey));
    }


    if (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
    {
        if (!p_sys_chan_tp->key.section_oam)
        {
            /* update dsmpls */
            ctc_mpls_property_t mpls_pro_info;

            sal_memset(&oam_info, 0, sizeof(sys_nh_update_oam_info_t));
            sal_memset(&mpls_pro_info, 0, sizeof(ctc_mpls_property_t));
            mpls_pro_info.label = p_sys_chan_tp->key.label;
            mpls_pro_info.space_id = p_sys_chan_tp->key.spaceid;
            if ((!is_add) && DRV_IS_DUET2(lchip))
            {
                mpls_info.label = p_sys_chan_tp->key.label;
                mpls_info.spaceid = p_sys_chan_tp->key.spaceid;
                mpls_info.oam_type = 0; /*lkup vpws oam info*/
                ret = sys_usw_mpls_get_oam_info(lchip, &mpls_info);
                if (CTC_E_NONE == ret)
                {
                    sys_usw_global_ctl_get(lchip, CTC_GLOBAL_TPOAM_VPWSOAM_COEXIST, &mode);
                    mep_index_in_key = mode ? (mpls_info.oamid + 2048) : mpls_info.oamid; /*When remove TPOAM, restore VPWS info if VPWS exists*/
                }
            }
            mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_IDX;
            mpls_pro_info.value = (void*)(&mep_index_in_key);
            CTC_ERROR_RETURN(sys_usw_mpls_set_ilm_property(lchip, &mpls_pro_info));
            mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_CHK;
            mpls_pro_info.value = (void*)(&oam_check_type);
            CTC_ERROR_RETURN(sys_usw_mpls_set_ilm_property(lchip, &mpls_pro_info));
            mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_LMEN;
            mpls_pro_info.value = (void*)(&lm_en);
            CTC_ERROR_RETURN(sys_usw_mpls_set_ilm_property(lchip, &mpls_pro_info));

            /*update nexthop to enbale egress lm to be add*/
            oam_info.lm_en = lm_en;
            oam_info.is_protection_path = CTC_FLAG_ISSET(p_sys_lmep_bfd->flag, CTC_OAM_BFD_LMEP_FLAG_PROTECTION_PATH);
            oam_info.oam_mep_index = p_sys_chan_tp->mep_index_in_key;
            sys_usw_nh_update_oam_en(lchip, p_sys_lmep_bfd->nhid,&oam_info);

            /*update VPWS OAM key by TPOAM*/
            if (DRV_IS_DUET2(lchip))
            {
                mpls_info.label = p_sys_chan_tp->key.label;
                mpls_info.spaceid = p_sys_chan_tp->key.spaceid;
                mpls_info.oam_type = 0; /*lkup vpws oam info*/
                ret = sys_usw_mpls_get_oam_info(lchip, &mpls_info);
                if (CTC_E_NONE == ret)
                {
                    _sys_usw_oam_update_vpws_key(lchip, mpls_info.gport, mpls_info.oamid, p_sys_chan_tp, is_add);
                }
            }

        }
        else
        {
            /*need update port or interface lm enable*/
            uint32 cmd      = 0;
            uint32 field    = 0;

            field = CTC_FLAG_ISSET(p_sys_lmep_bfd->flag, CTC_OAM_BFD_LMEP_FLAG_LM_EN);
            if (g_oam_master[lchip]->tp_section_oam_based_l3if)
            {
                index = p_sys_chan_tp->key.gport_l3if_id;

                cmd = DRV_IOR(DsSrcInterface_t, DsSrcInterface_mplsSectionLmEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));
                cmd = DRV_IOR(DsDestInterface_t, DsDestInterface_mplsSectionLmEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));
            }
            else
            {
                index = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_sys_chan_tp->key.gport_l3if_id);
                cmd = DRV_IOR(DsSrcPort_t, DsSrcPort_mplsSectionLmEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));
                cmd = DRV_IOR(DsDestPort_t, DsDestPort_mplsSectionLmEn_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));
            }
        }

    }
    else if (CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type)
    {

        /* update dsmpls for VCCV BFD*/
        if ((p_sys_lmep_bfd->mpls_in_label) &&(CTC_FLAG_ISSET(p_sys_lmep_bfd->flag, CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV)
            || CTC_FLAG_ISSET(p_sys_lmep_bfd->flag, CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV_IPV4)
        || CTC_FLAG_ISSET(p_sys_lmep_bfd->flag, CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV_IPV6)))

        {
            ctc_mpls_property_t mpls_pro_info;
            uint32 field    = 1;
            sal_memset(&mpls_pro_info, 0, sizeof(ctc_mpls_property_t));
            mpls_pro_info.label = p_sys_lmep_bfd->mpls_in_label;
            mpls_pro_info.space_id = p_sys_lmep_bfd->spaceid;

            mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_VCCV_BFD;
            mpls_pro_info.value = (void*)(&field);
            CTC_ERROR_RETURN(sys_usw_mpls_set_ilm_property(lchip, &mpls_pro_info));
        }
    }

    sal_memset(&oam_info, 0, sizeof(sys_nh_update_oam_info_t));
    oam_info.update_type = 1;
    oam_info.dsma_en = is_add?1:0;
    oam_info.ma_idx  = p_sys_lmep_bfd->ma_index;
    oam_info.mep_index = p_sys_lmep_bfd->lmep_index;
    oam_info.mep_type = 0;
    oam_info.is_protection_path = CTC_FLAG_ISSET(p_sys_lmep_bfd->flag, CTC_OAM_BFD_LMEP_FLAG_PROTECTION_PATH);
    sys_usw_nh_update_oam_en(lchip, p_sys_lmep_bfd->nhid,&oam_info);

    return CTC_E_NONE;
}

#if 0
int32
_sys_usw_bfd_chan_update_reomte(uint8 lchip, ctc_oam_lmep_t* p_lmep, sys_oam_chan_tp_t* p_sys_chan_tp, bool is_add)
{
    DsEgressXcOamMplsSectionHashKey_m mpls_section_hash_key;
    void *p_key = NULL;
    uint32 index       = 0;
    uint32 tbl_id    = 0;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*mep index*/
    if (is_add)
    {
        p_sys_chan_tp->mep_index = p_lmep->lmep_index;
        p_sys_chan_tp->mep_index_in_key = p_lmep->lmep_index/2;
    }
    else
    {
        p_sys_chan_tp->mep_index = 0x1FFF;
        p_sys_chan_tp->mep_index_in_key = 0x1FFF;
    }

    if (p_sys_chan_tp->key.section_oam)
    {
        p_key = &mpls_section_hash_key;
        sal_memset(p_key, 0, sizeof(DsEgressXcOamMplsSectionHashKey_m));
        tbl_id = DsEgressXcOamMplsSectionHashKey_t;
    }
    else
    {
        p_key = &mpls_section_hash_key;
        sal_memset(p_key, 0, sizeof(DsEgressXcOamMplsLabelHashKey_m));
        tbl_id = DsEgressXcOamMplsLabelHashKey_t;
    }

    index = p_sys_chan_tp->key.com.key_index;
    CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, tbl_id, index, p_key));

    if (p_sys_chan_tp->key.section_oam)
    {
        SetDsEgressXcOamMplsSectionHashKey(V, mepIndex_f          ,  p_key, p_sys_chan_tp->mep_index) ;
    }
    else
    {
        SetDsEgressXcOamMplsLabelHashKey(V, mepIndex_f          ,  p_key, p_sys_chan_tp->mep_index) ;
    }
    CTC_ERROR_RETURN(sys_usw_oam_key_write_io(lchip, tbl_id, index, p_key));


    return CTC_E_NONE;
}
#endif

sys_oam_chan_com_t*
_sys_usw_bfd_chan_lkup(uint8 lchip, ctc_oam_key_t* p_key_parm)
{
    sys_oam_chan_com_t*     p_sys_oam_chan      = NULL;

    uint8 mep_type = CTC_OAM_MEP_TYPE_MAX;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    mep_type    = p_key_parm->mep_type;
    if (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
    {
        p_sys_oam_chan  = (sys_oam_chan_com_t*)_sys_usw_tp_chan_lkup(lchip, p_key_parm);
    }
    else if ((CTC_OAM_MEP_TYPE_IP_BFD == mep_type) || (CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type)
        || (CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type))
    {
        p_sys_oam_chan  = (sys_oam_chan_com_t*)_sys_usw_ip_bfd_chan_lkup(lchip, p_key_parm);
    }

    return p_sys_oam_chan;

}

#define LMEP "Function Begin"

sys_oam_lmep_t*
_sys_usw_bfd_lmep_lkup(uint8 lchip, sys_oam_chan_com_t* p_sys_chan)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    return (sys_oam_lmep_t*)_sys_usw_oam_lmep_lkup(lchip, p_sys_chan, NULL);
}

sys_oam_lmep_t*
_sys_usw_bfd_lmep_build_node(uint8 lchip, ctc_oam_lmep_t* p_lmep_param)
{
    sys_oam_lmep_t* p_sys_lmep      = NULL;
    ctc_oam_bfd_lmep_t* p_ctc_bfd_lmep  = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_ctc_bfd_lmep = &p_lmep_param->u.bfd_lmep;

    p_sys_lmep = (sys_oam_lmep_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_lmep_t));
    if (NULL != p_sys_lmep)
    {
        sal_memset(p_sys_lmep, 0, sizeof(sys_oam_lmep_t));
        p_sys_lmep->flag = p_ctc_bfd_lmep->flag;
        p_sys_lmep->mep_on_cpu = CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_MEP_ON_CPU);
        if (!g_oam_master[lchip]->mep_index_alloc_by_sdk)
        {
            p_sys_lmep->lmep_index = p_lmep_param->lmep_index;   /*index alloc by system*/
        }
        p_sys_lmep->nhid = p_ctc_bfd_lmep->nhid;

        if((CTC_OAM_MEP_TYPE_IP_BFD == p_lmep_param->key.mep_type)
            || (CTC_OAM_MEP_TYPE_MPLS_BFD == p_lmep_param->key.mep_type)
            || (CTC_OAM_MEP_TYPE_MICRO_BFD == p_lmep_param->key.mep_type))
        {
            p_sys_lmep->local_discr = p_lmep_param->key.u.bfd.discr;
            p_sys_lmep->mpls_in_label       = p_ctc_bfd_lmep->mpls_in_label;
            p_sys_lmep->spaceid             = p_ctc_bfd_lmep->mpls_spaceid;
        }
        else
        {
            p_sys_lmep->local_discr = p_ctc_bfd_lmep->local_discr;
            p_sys_lmep->mpls_in_label = p_lmep_param->key.u.tp.label;
            p_sys_lmep->spaceid = p_lmep_param->key.u.tp.mpls_spaceid;
        }
    }

    return p_sys_lmep;
}

int32
_sys_usw_bfd_lmep_free_node(uint8 lchip, sys_oam_lmep_t* p_sys_lmep)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    mem_free(p_sys_lmep);
    p_sys_lmep = NULL;
    return CTC_E_NONE;
}

int32
_sys_usw_bfd_lmep_build_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_build_index(lchip, p_sys_lmep));
    return CTC_E_NONE;
}

int32
_sys_usw_bfd_lmep_free_index(uint8 lchip, sys_oam_lmep_t* p_sys_lmep)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_free_index(lchip, p_sys_lmep));
    return CTC_E_NONE;
}

int32
_sys_usw_bfd_lmep_add_to_db(uint8 lchip, sys_oam_lmep_t* p_sys_lmep)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_add_to_db(lchip, p_sys_lmep));
    return CTC_E_NONE;
}

int32
_sys_usw_bfd_lmep_del_from_db(uint8 lchip, sys_oam_lmep_t* p_sys_lmep)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_del_from_db(lchip, p_sys_lmep));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_bfd_lmep_add_tbl_to_asic(uint8 lchip, sys_oam_lmep_t* p_sys_lmep, ctc_oam_lmep_t* p_oam_lmep)
{
    sys_oam_chan_tp_t*  p_sys_chan_tp   = NULL;
    sys_oam_maid_com_t* p_sys_maid  = NULL;
    ctc_oam_bfd_lmep_t* p_ctc_bfd_lmep = NULL;

    DsMa_m ds_ma;
    DsBfdMep_m ds_bfd_mep;
    DsBfdMep_m DsBfdMep_mask;
    DsBfdRmep_m ds_bfd_rmep;
    DsBfdRmep_m DsBfdRmep_mask;
    tbl_entry_t  tbl_entry;

    uint32 cmd        = 0;
    uint32 index      = 0;
    uint32 lmep_flag  = 0;
     /*uint8 aps_en  = 0;*/
    uint8 u_active = 0;
     /*uint8 is_csf_tx_en = 0;*/
     /*uint8 is_csf_en = 0;*/
    uint8 mep_type = CTC_OAM_MEP_TYPE_MAX;
    uint8 csf_type = 0;
    uint8  ipv6_sa_index = 0;
    uint8  ipv6_da_index = 0;
    uint32 ipDa_h = 0;
    uint32 ipDa_l = 0;
    sys_oam_nhop_info_t oam_nhop_info;
    p_ctc_bfd_lmep = &(p_oam_lmep->u.bfd_lmep);

    mep_type       = p_sys_lmep->p_sys_chan->mep_type;

    if (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
    {
        p_sys_chan_tp = (sys_oam_chan_tp_t*)p_sys_lmep->p_sys_chan;
    }

    p_sys_maid  = p_sys_lmep->p_sys_maid;

    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
    {
        goto RETURN;
    }

    lmep_flag = p_sys_lmep->flag;

    sal_memset(&ds_ma, 0, sizeof(DsMa_m));
    sal_memset(&ds_bfd_mep, 0, sizeof(DsBfdMep_m));
    sal_memset(&DsBfdMep_mask, 0, sizeof(DsBfdMep_m));
    sal_memset(&ds_bfd_rmep, 0, sizeof(DsBfdRmep_m));
    sal_memset(&DsBfdRmep_mask, 0, sizeof(DsBfdRmep_m));
    sal_memset(&oam_nhop_info, 0, sizeof(oam_nhop_info));

    CTC_ERROR_RETURN(_sys_usw_oam_get_nexthop_info(lchip, p_sys_lmep->nhid, CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_PROTECTION_PATH), &oam_nhop_info));

    /* set ma table entry */
    SetDsMa(V, maIdLengthType_f,&ds_ma,((p_sys_maid == NULL) ? 0 : (p_sys_maid->maid_entry_num >> 1)));
    SetDsMa(V, maNameIndex_f,&ds_ma,((p_sys_maid == NULL) ? 0 : p_sys_maid->maid_index));
    SetDsMa(V, maNameLen_f,&ds_ma,((p_sys_maid == NULL) ? 0 : p_sys_maid->maid_len));
    SetDsMa(V, priorityIndex_f,&ds_ma, p_ctc_bfd_lmep->tx_cos_exp);
    SetDsMa(V, useOamExp_f,&ds_ma, p_ctc_bfd_lmep->tx_cos_exp?1:0);

    /*must set localDisc_f before Sbfd-reflector*/
    SetDsBfdMep(V, localDisc_f, &ds_bfd_mep, p_sys_lmep->local_discr);

    if (CTC_OAM_MEP_TYPE_IP_BFD == mep_type)
    {
        SetDsMa(V, txWithSendId_f,&ds_ma, 1);    /* UPD header with BFD PDU */
        SetDsMa(V, rxOamType_f,&ds_ma, 2);

        if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_IPV4_ENCAP))
        {
            SetDsMa(V, txWithPortStatus_f,&ds_ma, 1);  /* IPv4 header with BFD PDU */
            SetDsMa(V, packetType_f,&ds_ma, 1);      /* pkt_type 0x0800 */
        }

        if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_IPV6_ENCAP))
        {
            SetDsMa(V, ipv6Hdr_f,&ds_ma, 1);  /* IPv6 header with BFD PDU */
            SetDsMa(V, packetType_f,&ds_ma, 3);      /* pkt_type 0x86DD */
        }

        if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_INITIATOR))
        {
            SetDsMa(V, sBfdType_f, &ds_ma , 1);
        }

        if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_REFLECTOR))
        {
            SetDsMa(V, sBfdType_f, &ds_ma , 2);
            /*sbfd localDisc means payloadOffset, default 127 for IP Sbfd*/
            SetDsBfdMep(V, localDisc_f,&ds_bfd_mep,127);
            SetDsBfdMep(V, localDisc_f,&DsBfdMep_mask,0);
        }

    }
    else if (CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type)
    {
        SetDsMa(V, txWithSendId_f, &ds_ma, 1);    /* UPD header with BFD PDU */
        SetDsMa(V, rxOamType_f, &ds_ma, 2);

        SetDsBfdMep(V, microBFD_f, &ds_bfd_mep, 1);
        SetDsBfdMep(V, microBFD_f, &DsBfdMep_mask, 0);

        if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_IPV4_ENCAP))
        {
            SetDsMa(V, txWithPortStatus_f, &ds_ma, 1);  /* IPv4 header with BFD PDU */
            SetDsMa(V, packetType_f, &ds_ma, 1);      /* pkt_type 0x0800 */
        }

        if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_IPV6_ENCAP))
        {
            SetDsMa(V, ipv6Hdr_f, &ds_ma, 1);  /* IPv6 header with BFD PDU */
            SetDsMa(V, packetType_f, &ds_ma, 3);      /* pkt_type 0x86DD */
        }
    }
    else if (CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type)
    {
        if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_INITIATOR))
        {
            SetDsMa(V, sBfdType_f, &ds_ma , 1);
        }
        else if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_REFLECTOR))
        {
            SetDsMa(V, sBfdType_f, &ds_ma , 2);
        }

        if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV))
        {
            SetDsMa(V, rxOamType_f,&ds_ma, 8);
            SetDsMa(V, txUntaggedOam_f,&ds_ma, 1);
            SetDsMa(V, mplsLabelValid_f,&ds_ma, 0xF);

            if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_REFLECTOR))
            {
                SetDsBfdMep(V, localDisc_f, &ds_bfd_mep, 4);
                SetDsBfdMep(V, localDisc_f, &DsBfdMep_mask, 0);
            }
        }
        else if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV_IPV4))
        {
            SetDsMa(V, rxOamType_f,&ds_ma, 8);
            SetDsMa(V, txUntaggedOam_f,&ds_ma, 1);
            SetDsMa(V, mplsLabelValid_f,&ds_ma, 0xF);
            SetDsMa(V, txWithPortStatus_f,&ds_ma, 1); /* with IPv4 header, only for ip bfd over mpls*/
            SetDsMa(V, txWithSendId_f,&ds_ma, 1); /* UPD header with BFD PDU */
            SetDsMa(V, packetType_f,&ds_ma, 1);      /* pkt_type 0x0800 */
            if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_REFLECTOR))
            {
                SetDsBfdMep(V, localDisc_f, &ds_bfd_mep, 28);
                SetDsBfdMep(V, localDisc_f, &DsBfdMep_mask, 0);
            }
        }
        else if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV_IPV6))
        {
            SetDsMa(V, rxOamType_f,&ds_ma, 8);
            SetDsMa(V, txUntaggedOam_f,&ds_ma, 1);
            SetDsMa(V, mplsLabelValid_f,&ds_ma, 0xF);
            SetDsMa(V, ipv6Hdr_f,&ds_ma, 1);  /* IPv6 header with BFD PDU */
            SetDsMa(V, txWithSendId_f,&ds_ma, 1);    /* UPD header with BFD PDU */
            SetDsMa(V, packetType_f,&ds_ma, 3);      /* pkt_type 0x86DD */
            if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_REFLECTOR))
            {
                SetDsBfdMep(V, localDisc_f, &ds_bfd_mep, 48);
                SetDsBfdMep(V, localDisc_f, &DsBfdMep_mask, 0);
            }
        }
        else
        {
            SetDsMa(V, rxOamType_f, &ds_ma, 7);
            SetDsMa(V, mplsLabelValid_f, &ds_ma, 0x7);
            if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_IPV6_ENCAP))
            {
                SetDsMa(V, ipv6Hdr_f, &ds_ma, 1);  /* IPv6 header with BFD PDU */
                SetDsMa(V, txWithSendId_f, &ds_ma, 1);    /* UPD header with BFD PDU */
                SetDsMa(V, packetType_f, &ds_ma, 3);      /* pkt_type 0x86DD */
                if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_REFLECTOR))
                {
                    SetDsBfdMep(V, localDisc_f, &ds_bfd_mep, 48);
                    SetDsBfdMep(V, localDisc_f, &DsBfdMep_mask, 0);
                }
            }
            else if(CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_IPV4_ENCAP))
            {
                SetDsMa(V, txWithPortStatus_f, &ds_ma, 1); /* with IPv4 header, only for ip bfd over mpls*/
                SetDsMa(V, txWithSendId_f, &ds_ma, 1); /* UPD header with BFD PDU */
                SetDsMa(V, packetType_f,&ds_ma, 1);
                if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_REFLECTOR))
                {
                    SetDsBfdMep(V, localDisc_f, &ds_bfd_mep, 28);
                    SetDsBfdMep(V, localDisc_f, &DsBfdMep_mask, 0);
                }
            }
        }
    }
    else if (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
    {
        /* MPLS TP OAM with GAL or not */
        uint8 tx_untagged_oam       = CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_WITHOUT_GAL);
        uint8 mpls_label_valid      = p_sys_chan_tp->key.section_oam ? 0 : 0xF;
        uint8 linkoam               = p_sys_chan_tp->key.section_oam ? 1 : 0;

        SetDsMa(V, txUntaggedOam_f,&ds_ma, tx_untagged_oam); /* with IPv4 header, only for ip bfd over mpls*/
        SetDsMa(V, mplsLabelValid_f,&ds_ma, mpls_label_valid); /* UPD header with BFD PDU */
        SetDsMa(V, linkoam_f,&ds_ma, linkoam);
        SetDsMa(V, rxOamType_f,&ds_ma,8);

        if(tx_untagged_oam)
        {
             /* for Mac_Header+Tunnel+PW(sbit)+GACH+BFD*/
            SetDsMa(V, packetType_f,&ds_ma, 7);      /* pkt_type PACKETTYPE_RESERVED */
        }
        else
        {
             /* for Mac_Header+Tunnel+PW+GAL13(sbit)+GACH+BFD*/
            SetDsMa(V, packetType_f,&ds_ma, 2);      /* pkt_type PACKETTYPE_MPLS */
        }

         /* lmep CC*/
        SetDsMa(V, csfInterval_f , &ds_ma , 3);
    }
    SetDsBfdMep(V, ccmInterval_f, &ds_bfd_mep, 1);

     /*aps_en  = CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_APS_EN);*/

    SetDsMa(V, mplsTtl_f,&ds_ma,p_ctc_bfd_lmep->ttl);

    SetDsMa(V, nextHopExt_f, &ds_ma, oam_nhop_info.nexthop_is_8w);
    SetDsMa(V, nextHopPtr_f, &ds_ma, oam_nhop_info.dsnh_offset);

    SetDsBfdMep(V, destMap_f, &ds_bfd_mep, oam_nhop_info.dest_map);
    SetDsMa(V, tunnelApsEn_f,           &ds_ma , oam_nhop_info.mep_on_tunnel);
    SetDsMa(V, protectingPath_f,        &ds_ma , CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_PROTECTION_PATH));
    SetDsMa(V, mdLvl_f,&ds_ma,0);

    /* set mep table entry */
    u_active  = CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_MEP_EN);

    SetDsBfdMep(V, active_f,&ds_bfd_mep,u_active);
    SetDsBfdMep(V, maIndex_f,&ds_bfd_mep,p_sys_lmep->ma_index);
    SetDsBfdMep(V, localStat_f,&ds_bfd_mep,p_ctc_bfd_lmep->local_state);
    SetDsBfdMep(V, localDiag_f,&ds_bfd_mep,p_ctc_bfd_lmep->local_diag);


    SetDsBfdMep(V, desiredMinTxInterval_f,&ds_bfd_mep,p_ctc_bfd_lmep->desired_min_tx_interval);

#if (SDK_WORK_PLATFORM == 0)
    SetDsBfdMep(V, helloWhile_f,&ds_bfd_mep,1000);
    SetDsBfdMep(V, actualMinTxInterval_f,&ds_bfd_mep,1000);
#else
     /* for uml ccm timer interval 250ms, hellowhile = 4,250*4 = 1000ms*/
    SetDsBfdMep(V, helloWhile_f,&ds_bfd_mep,4);
    SetDsBfdMep(V, actualMinTxInterval_f,&ds_bfd_mep,4);
#endif

    if ((CTC_OAM_MEP_TYPE_IP_BFD == mep_type) || (CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type)
        || (CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type))
    {
        SetDsBfdMep(V, mepType_f,&ds_bfd_mep,5);
        SetDsBfdMep(V, bfdSrcport_f,&ds_bfd_mep,p_ctc_bfd_lmep->bfd_src_port);

        if ((CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type)
            &&(CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV)
               ||(CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV_IPV4))
               ||(CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV_IPV6))))
        {
            SetDsBfdMep(V, mepType_f,&ds_bfd_mep,7);
        }
    }
    else if (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
    {
        SetDsBfdMep(V, mepType_f,&ds_bfd_mep,7);

         /*is_csf_tx_en         = CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_TX_CSF_EN);*/
         /*is_csf_en            = CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_CSF_EN);*/
        csf_type = _sys_usw_bfd_csf_convert(lchip, p_ctc_bfd_lmep->tx_csf_type, TRUE);
        SetDsBfdMep(V, csfType_f,&ds_bfd_mep,csf_type);
    }

    SetDsBfdMep(V, rxCsfWhile_f,&ds_bfd_mep,14);
    SetDsBfdMep(V, isBfd_f,&ds_bfd_mep,1);
    SetDsBfdMep(V, isRemote_f,&ds_bfd_mep,0);
    SetDsBfdMep(V, isUp_f,&ds_bfd_mep,0);
    SetDsBfdMep(V, ethOamP2PMode_f,&ds_bfd_mep,1);
    SetDsBfdMep(V, cciEn_f,&ds_bfd_mep,0);

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"LMEP: ds_bfd_mep.active            = %d\n", u_active);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"LMEP: ds_bfd_mep.ma_index          = %d\n", p_sys_lmep->ma_index);
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_INFO,"LMEP: ds_bfd_mep.is_remote         = %d\n", 0);

    SetDsBfdRmep(V, localDefectMult_f, &ds_bfd_rmep, p_ctc_bfd_lmep->local_detect_mult);
    SetDsBfdRmep(V, isRemote_f,&ds_bfd_rmep,1);
    SetDsBfdRmep(V, isBfd_f,&ds_bfd_rmep,1);
    if (CTC_OAM_MEP_TYPE_IP_BFD == mep_type || CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type)
    {
        SetDsBfdRmep(V, ipBfdTtl_f , &ds_bfd_rmep, p_ctc_bfd_lmep->ttl);
    }

    if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_IPV4_ENCAP))
    {
         /* edit ip_sa*/
        SetDsBfdRmep(V, ipSa_f, &ds_bfd_rmep, p_ctc_bfd_lmep->ip4_sa);
         /* edit ip_da*/
        ipDa_h = ((p_ctc_bfd_lmep->ip4_da) >> 20)&0xfff;
        SetDsBfdRmep(V, ipDaH_f, &ds_bfd_rmep, ipDa_h);
        ipDa_l = (p_ctc_bfd_lmep->ip4_da)&0xfffff;
    }

    if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_IPV6_ENCAP))
    {
        CTC_ERROR_RETURN(_sys_usw_bfd_alloc_ipv6_idx(lchip, (void*)&(p_ctc_bfd_lmep->ipv6_sa), &ipv6_sa_index));
        SetDsBfdRmep(V, ipSa_f, &ds_bfd_rmep, ipv6_sa_index);
        CTC_ERROR_RETURN(_sys_usw_bfd_alloc_ipv6_idx(lchip, (void*)&(p_ctc_bfd_lmep->ipv6_da), &ipv6_da_index));
        ipDa_l = ipv6_da_index;
    }
    SetDsBfdMep(V, ipDaL_f, &ds_bfd_mep, ipDa_l);
    SetDsBfdMep(V, ipDaL_f, &DsBfdMep_mask, 0);

    index = p_sys_lmep->ma_index;
    CTC_ERROR_RETURN(sys_usw_ma_add_to_asic(lchip, index, &ds_ma));

    index = p_sys_lmep->lmep_index;
    cmd = DRV_IOW(DsBfdMep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_bfd_mep;
    tbl_entry.mask_entry = (uint32*)&DsBfdMep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));

    index = p_sys_lmep->lmep_index + 1;
    cmd = DRV_IOW(DsBfdRmep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_bfd_rmep;
    tbl_entry.mask_entry = (uint32*)&DsBfdRmep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));
RETURN:

    return CTC_E_NONE;
}

int32
_sys_usw_bfd_lmep_add_to_asic(uint8 lchip, sys_oam_lmep_t* p_sys_lmep, ctc_oam_lmep_t*  p_oam_lmep)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_lmep);
    CTC_PTR_VALID_CHECK(p_oam_lmep);

    if (CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_MEP_ON_CPU))
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(_sys_usw_bfd_lmep_add_tbl_to_asic(lchip, p_sys_lmep, p_oam_lmep));

    return CTC_E_NONE;
}

int32
_sys_usw_bfd_lmep_del_from_asic(uint8 lchip, sys_oam_lmep_t* p_sys_lmep)
{
    uint32 cmd          = 0;
    uint8 ipv6_da_index = 0;
    uint32 index        = 0;
    uint32 temp = 0;
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_MEP_ON_CPU))
    {
        return CTC_E_NONE;
    }

    if ((p_sys_lmep->p_sys_chan->mep_type == CTC_OAM_MEP_TYPE_IP_BFD)
        || (p_sys_lmep->p_sys_chan->mep_type == CTC_OAM_MEP_TYPE_MPLS_BFD)
    || (p_sys_lmep->p_sys_chan->mep_type == CTC_OAM_MEP_TYPE_MICRO_BFD))
    {

        if (CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_IPV6_ENCAP))
        {
            index = p_sys_lmep->lmep_index;
            cmd = DRV_IOR(DsBfdMep_t, DsBfdMep_ipDaL_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &temp));
            ipv6_da_index = temp;
            CTC_ERROR_RETURN(_sys_usw_bfd_free_ipv6_idx(lchip, ipv6_da_index));
        }
    }

    CTC_ERROR_RETURN(_sys_usw_oam_lmep_del_from_asic(lchip, p_sys_lmep));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_bfd_lmep_update_tbl_asic(uint8 lchip, ctc_oam_update_t* p_update, sys_oam_lmep_t* p_sys_lmep)
{
    uint32 cmd        = 0;
    uint32 index      = 0;
    uint8 csf_type    = 0;
    uint8 dcsf = 0;

    DsMa_m ds_ma;                    /*DS_MA */
    DsBfdMep_m ds_bfd_mep;          /* DS_ETH_MEP */
    DsBfdMep_m DsBfdMep_mask;          /* DS_ETH_MEP */
    DsBfdRmep_m ds_bfd_rmep;
    DsBfdRmep_m DsBfdRmep_mask;
    tbl_entry_t  tbl_entry;

    sys_oam_rmep_t* p_sys_rmep = NULL;
    sys_oam_nhop_info_t oam_nhop_info;

    uint16 desired_min_tx_interval = 0;
    uint16 required_min_rx_interval = 0;
    ctc_oam_bfd_timer_t*  oam_bfd_timer = NULL;
    uint32 detect_mult = 0;
    uint8 mep_type = CTC_OAM_MEP_TYPE_MAX;
    sal_systime_t tv;
    uint32 interval = 3000000;  /*3s*/
    uint32 tmp = 0;

    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
    {
        goto RETURN;
    }

    sal_memset(&ds_bfd_mep, 0, sizeof(DsBfdMep_m));
    sal_memset(&DsBfdMep_mask, 0xFF, sizeof(DsBfdMep_m));

    index = p_sys_lmep->lmep_index;
    cmd = DRV_IOR(DsBfdMep_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_bfd_mep));

    sal_memset(&ds_ma, 0, sizeof(DsMa_m));
    index = p_sys_lmep->ma_index;
    cmd = DRV_IOR(DsMa_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_ma));

    mep_type = p_sys_lmep->p_sys_chan->mep_type;
    p_sys_rmep = (sys_oam_rmep_t*)CTC_SLISTHEAD(p_sys_lmep->rmep_list);
    if ((NULL == p_sys_rmep) && (!CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_REFLECTOR)))
    {
        SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Remote mep is not found \n");
        return CTC_E_NOT_EXIST;
    }

    if (p_sys_rmep)
    {
        sal_memset(&ds_bfd_rmep, 0, sizeof(DsBfdRmep_m));
        sal_memset(&DsBfdRmep_mask, 0xFF, sizeof(DsBfdRmep_m));
        index = p_sys_rmep->rmep_index;
        cmd = DRV_IOR(DsBfdRmep_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_bfd_rmep));
    }

    switch (p_update->update_type)
    {
    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_MEP_EN:
        if (CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_MEP_EN))
        {
            SetDsBfdMep(V, active_f,&ds_bfd_mep,1);
             /*SetDsBfdMep(V, rxSlowInterval_f, &ds_bfd_slow_interval_cfg, 1000);*/
        }
        else
        {
            SetDsBfdMep(V, active_f,&ds_bfd_mep,0);
        }
        SetDsBfdMep(V, active_f, &DsBfdMep_mask, 0);
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_CC_EN:
        if (1 == GetDsBfdRmep(V, isCsfEn_f, &ds_bfd_rmep))
        {
            return CTC_E_INVALID_CONFIG;
        }
        SetDsBfdMep(V, cciEn_f, &ds_bfd_mep, p_update->update_value);
        SetDsBfdMep(V, cciEn_f, &DsBfdMep_mask, 0);
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_CV_EN:
        SetDsBfdRmep(V, isCvEn_f, &ds_bfd_rmep, p_update->update_value);
        SetDsBfdRmep(V, isCvEn_f, &DsBfdRmep_mask, 0);
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_COS_EXP:
        SetDsMa(V, priorityIndex_f,&ds_ma, p_update->update_value);
        SetDsMa(V, useOamExp_f,&ds_ma, (p_update->update_value)?1:0);
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_LMEP_SF_STATE:
        return CTC_E_NOT_SUPPORT;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_CSF_EN:
        if (1 == GetDsBfdMep(V, cciEn_f, &ds_bfd_mep))
        {
            return CTC_E_INVALID_CONFIG;
        }
        SetDsBfdRmep(V, isCsfEn_f,&ds_bfd_rmep,(CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_CSF_EN)));
        SetDsBfdRmep(V, isCsfEn_f,&DsBfdRmep_mask,0);
        SetDsBfdRmep(V, isCsfTxEn_f,&ds_bfd_rmep,0);
        SetDsBfdRmep(V, isCsfTxEn_f, &DsBfdRmep_mask, 0);
        SetDsBfdRmep(V, firstPktRx_f           , &ds_bfd_rmep , (CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_CSF_EN)));
        SetDsBfdRmep(V, firstPktRx_f           , &DsBfdRmep_mask ,  0);
        SetDsBfdRmep(V, dLoc_f                 , &ds_bfd_rmep , (CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_CSF_EN)));
        SetDsBfdRmep(V, dLoc_f           , &DsBfdRmep_mask ,  0);
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_CSF_EN:
        if (1 == GetDsBfdMep(V, cciEn_f, &ds_bfd_mep))
        {
            return CTC_E_INVALID_CONFIG;
        }
        SetDsBfdRmep(V, isCsfTxEn_f,&ds_bfd_rmep,CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_TX_CSF_EN));
        SetDsBfdRmep(V, isCsfTxEn_f,&DsBfdRmep_mask,0);
        SetDsBfdRmep(V, isCsfEn_f,&ds_bfd_rmep,CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_TX_CSF_EN));
        SetDsBfdRmep(V, isCsfEn_f, &DsBfdRmep_mask, 0);
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_CSF_TYPE:
        {
            csf_type = _sys_usw_bfd_csf_convert(lchip, p_update->update_value, TRUE);
            SetDsBfdMep(V, csfType_f,&ds_bfd_mep,csf_type);
            SetDsBfdMep(V, csfType_f,&DsBfdMep_mask,0);
        }
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_DCSF:
        {
            dcsf = p_update->update_value;
            SetDsBfdMep(V, dCsf_f, &ds_bfd_mep, dcsf);
            SetDsBfdMep(V, dCsf_f, &DsBfdMep_mask, 0);
        }
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_DIAG:
        SetDsBfdMep(V, localDiag_f,&ds_bfd_mep,p_update->update_value);
        SetDsBfdMep(V, localDiag_f,&DsBfdMep_mask,0);
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_STATE:
        SetDsBfdMep(V, localStat_f,&ds_bfd_mep, p_update->update_value);
        SetDsBfdMep(V, localStat_f,&DsBfdMep_mask,0);
        if (CTC_OAM_BFD_STATE_ADMIN_DOWN == p_update->update_value)
        {
            SetDsBfdMep(V, helloWhile_f, &ds_bfd_mep, 0);
            SetDsBfdMep(V, helloWhile_f, &DsBfdMep_mask, 0);
        }
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_DISCR:
        SetDsBfdMep(V, localDisc_f,&ds_bfd_mep, p_update->update_value);
        SetDsBfdMep(V, localDisc_f,&DsBfdMep_mask,0);
        break;
    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_RX_TIMER:
        /*update rx timer begin*/
        oam_bfd_timer = (ctc_oam_bfd_timer_t*)p_update->p_update_value;
        required_min_rx_interval = oam_bfd_timer[1].min_interval;
        detect_mult = oam_bfd_timer[1].detect_mult;
        if (CTC_OAM_BFD_STATE_UP == GetDsBfdRmep(V, remoteStat_f, &ds_bfd_rmep))
        {
            sal_gettime(&tv);
            if ((((tv.tv_sec - p_sys_lmep->tv_p_set.tv_sec)*1000000 + (tv.tv_usec - p_sys_lmep->tv_p_set.tv_usec)) < interval)
                && !g_oam_master[lchip]->timer_update_disable)
            {
                return CTC_E_HW_BUSY;
            }
            if (required_min_rx_interval > GetDsBfdRmep(V, actualRxInterval_f, &ds_bfd_rmep))
            {
                SetDsBfdRmep(V, actualRxInterval_f, &ds_bfd_rmep, required_min_rx_interval);
                SetDsBfdRmep(V, actualRxInterval_f, &DsBfdRmep_mask, 0);
            }
        }
        SetDsBfdRmep(V, requiredMinRxInterval_f, &ds_bfd_rmep, required_min_rx_interval);
        SetDsBfdRmep(V, requiredMinRxInterval_f, &DsBfdRmep_mask, 0);
        SetDsBfdRmep(V, defectMult_f, &ds_bfd_rmep, detect_mult);
        SetDsBfdRmep(V, defectMult_f, &DsBfdRmep_mask, 0);
        SetDsBfdRmep(V, defectWhile_f, &ds_bfd_rmep, detect_mult*GetDsBfdRmep(V, actualRxInterval_f, &ds_bfd_rmep));
        SetDsBfdRmep(V, defectWhile_f, &DsBfdRmep_mask, 0);
        /*update tx timer begin, can not break*/
    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_TIMER:
        desired_min_tx_interval  = ((ctc_oam_bfd_timer_t*)p_update->p_update_value)->min_interval;
        if (CTC_OAM_BFD_STATE_UP == GetDsBfdMep(V, localStat_f, &ds_bfd_mep))
        {
            if ((GetDsBfdRmep(V, pbit_f, &ds_bfd_rmep) || GetDsBfdRmep(V, fbit_f, &ds_bfd_rmep))
                && !g_oam_master[lchip]->timer_update_disable)
            {
                return CTC_E_HW_BUSY;
            }
            sal_gettime(&tv);
            if ((((tv.tv_sec - p_sys_lmep->tv_p_set.tv_sec)*1000000 + (tv.tv_usec - p_sys_lmep->tv_p_set.tv_usec)) < interval)
                && !g_oam_master[lchip]->timer_update_disable)
            {
                return CTC_E_HW_BUSY;
            }
            sal_gettime(&p_sys_lmep->tv_p_set);
            if ((1 == GetDsBfdMep(V, ccTxMode_f, &ds_bfd_mep)) && (desired_min_tx_interval != 3))
            {
                SetDsBfdRmep(V, defectWhile_f, &ds_bfd_rmep, 1000);
                SetDsBfdRmep(V, defectWhile_f, &DsBfdRmep_mask, 0);
                SetDsBfdMep(V, ccTxMode_f, &ds_bfd_mep, 0);
                SetDsBfdMep(V, ccTxMode_f, &DsBfdMep_mask, 0);
            }
            SetDsBfdRmep(V, pbit_f, &ds_bfd_rmep, 1);
            SetDsBfdRmep(V, pbit_f, &DsBfdRmep_mask, 0);
            SetDsBfdRmep(V, fbit_f, &ds_bfd_rmep, 0);
            SetDsBfdRmep(V, fbit_f, &DsBfdRmep_mask, 0);
            if (desired_min_tx_interval < GetDsBfdMep(V, actualMinTxInterval_f, &ds_bfd_mep))
            {
                SetDsBfdMep(V, actualMinTxInterval_f, &ds_bfd_mep, desired_min_tx_interval);
                SetDsBfdMep(V, actualMinTxInterval_f, &DsBfdMep_mask, 0);
            }
        }
        SetDsBfdMep(V, desiredMinTxInterval_f, &ds_bfd_mep, desired_min_tx_interval);
        SetDsBfdMep(V, desiredMinTxInterval_f, &DsBfdMep_mask, 0);
        SetDsBfdRmep(V, localDefectMult_f,&ds_bfd_rmep,((ctc_oam_bfd_timer_t*)p_update->p_update_value)->detect_mult);
        SetDsBfdRmep(V, localDefectMult_f,&DsBfdRmep_mask,0);
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_ACTUAL_TX_TIMER:
        SetDsBfdMep(V, actualMinTxInterval_f, &ds_bfd_mep, ((ctc_oam_bfd_timer_t*)p_update->p_update_value)->min_interval);
        SetDsBfdMep(V, actualMinTxInterval_f, &DsBfdMep_mask, 0);
        tmp = (((ctc_oam_bfd_timer_t*)p_update->p_update_value)->min_interval > 100) ? 100 : ((ctc_oam_bfd_timer_t*)p_update->p_update_value)->min_interval;
        SetDsBfdMep(V, helloWhile_f, &ds_bfd_mep, (p_sys_lmep->lmep_index / 2) % tmp);
        SetDsBfdMep(V, helloWhile_f, &DsBfdMep_mask, 0);
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_NHOP:
    {
        sys_nh_update_oam_info_t oam_info;
        sal_memset(&oam_nhop_info, 0, sizeof(sys_oam_nhop_info_t));
        CTC_ERROR_RETURN(_sys_usw_oam_get_nexthop_info(lchip, p_sys_lmep->nhid, CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_PROTECTION_PATH), &oam_nhop_info));
        SetDsMa(V, nextHopExt_f,&ds_ma,oam_nhop_info.nexthop_is_8w);
        SetDsMa(V, nextHopPtr_f,&ds_ma, oam_nhop_info.dsnh_offset);

        SetDsBfdMep(V, destMap_f,&ds_bfd_mep,oam_nhop_info.dest_map);
        SetDsBfdMep(V, destMap_f,&DsBfdMep_mask,0);
        SetDsBfdRmep(V, apsBridgeEn_f,&ds_bfd_rmep,oam_nhop_info.aps_bridge_en);
        SetDsBfdRmep(V, apsBridgeEn_f,&DsBfdRmep_mask,0);
        SetDsMa(V, tunnelApsEn_f,           &ds_ma , oam_nhop_info.mep_on_tunnel);
        SetDsMa(V, protectingPath_f,        &ds_ma , CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_PROTECTION_PATH));

        sal_memset(&oam_info, 0, sizeof(sys_nh_update_oam_info_t));
        oam_info.update_type = 1;
        oam_info.dsma_en = 1;
        oam_info.ma_idx  = p_sys_lmep->ma_index;
        oam_info.mep_index = p_sys_lmep->lmep_index;
        oam_info.mep_type = 0;
        oam_info.is_protection_path = CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_PROTECTION_PATH);
        sys_usw_nh_update_oam_en(lchip, p_sys_lmep->nhid,&oam_info);

    }
        break;
    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_TTL:
        if (CTC_OAM_MEP_TYPE_IP_BFD == mep_type || CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type
            || CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type)
        {
            SetDsBfdRmep(V, ipBfdTtl_f , &ds_bfd_rmep, p_update->update_value);
        }

        SetDsMa(V, mplsTtl_f, &ds_ma, p_update->update_value);
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }
    if (g_oam_master[lchip]->timer_update_disable)
    {
        SetDsBfdRmep(V, defectWhile_f, &ds_bfd_rmep, 0x3FFF);
        SetDsBfdRmep(V, defectWhile_f, &DsBfdRmep_mask, 0);
    }
    index = p_sys_lmep->ma_index;
    CTC_ERROR_RETURN(sys_usw_ma_add_to_asic(lchip, index, &ds_ma));

    index = p_sys_lmep->lmep_index;
    cmd = DRV_IOW(DsBfdMep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_bfd_mep;
    tbl_entry.mask_entry = (uint32*)&DsBfdMep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));

    if (p_sys_rmep)
    {
        index = p_sys_rmep->rmep_index;
        cmd = DRV_IOW(DsBfdRmep_t, DRV_ENTRY_FLAG);
        tbl_entry.data_entry = (uint32*)&ds_bfd_rmep;
        tbl_entry.mask_entry = (uint32*)&DsBfdRmep_mask;
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));
    }

RETURN:
    return CTC_E_NONE;
}


int32
_sys_usw_bfd_lmep_update_asic(uint8 lchip, ctc_oam_update_t* p_update,
                                         sys_oam_lmep_t* p_sys_lmep)
{
    uint32 old_nhid = 0;
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_update);
    CTC_PTR_VALID_CHECK(p_sys_lmep);

    /*update db */
    switch (p_update->update_type)
    {
    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_MEP_EN:
          (1 == p_update->update_value) ?
            (CTC_SET_FLAG(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_MEP_EN)) : (CTC_UNSET_FLAG(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_MEP_EN));
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_CC_EN:
        if(CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_REFLECTOR))
        {
            return CTC_E_NOT_SUPPORT;
        }
    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_CV_EN:
    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_LMEP_SF_STATE:
    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_DCSF:
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_TIMER:
    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_RX_TIMER:
        if(CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_REFLECTOR))
        {
            return CTC_E_NOT_SUPPORT;
        }
        break;
    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_ACTUAL_TX_TIMER:
        if (CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_REFLECTOR))
        {
            return CTC_E_NOT_SUPPORT;
        }
        break;
    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_COS_EXP:
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_CSF_EN:
        (1 == p_update->update_value) ?
            (CTC_SET_FLAG(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_CSF_EN)) : (CTC_UNSET_FLAG(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_CSF_EN));
        break;
    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_CSF_EN:
        (1 == p_update->update_value) ? (CTC_SET_FLAG(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_TX_CSF_EN)) : (CTC_UNSET_FLAG(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_TX_CSF_EN));
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_TX_CSF_TYPE:
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_DIAG:
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_STATE:
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_DISCR:
        p_sys_lmep->local_discr = p_update->update_value;  /*update DB for debug show*/
        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_NHOP:
        old_nhid = p_sys_lmep->nhid;
        p_sys_lmep->nhid        = p_update->update_value;
        break;
    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_TTL:
        break;
    case  CTC_OAM_BFD_LMEP_UPDATE_TYPE_MASTER_GCHIP:
        CTC_ERROR_RETURN(_sys_usw_oam_lmep_update_master_chip(lchip, p_sys_lmep->p_sys_chan, p_update->update_value));
        return CTC_E_NONE;
    case  CTC_OAM_BFD_LMEP_UPDATE_TYPE_LABEL:
        CTC_ERROR_RETURN(_sys_usw_oam_lmep_update_label(lchip, p_sys_lmep->p_sys_chan, (ctc_oam_tp_key_t*)p_update->p_update_value));
        return CTC_E_NONE;
    default:
        return CTC_E_INVALID_PARAM;
    }

    /*update asic tabel */
    CTC_ERROR_RETURN(_sys_usw_bfd_lmep_update_tbl_asic(lchip, p_update, p_sys_lmep));

    if ((p_update->update_type == CTC_OAM_BFD_LMEP_UPDATE_TYPE_NHOP) && old_nhid)
    {
        sys_nh_update_oam_info_t oam_info;
        sal_memset(&oam_info, 0, sizeof(sys_nh_update_oam_info_t));
        /*clear old nhid bind info*/
        oam_info.update_type = 1;
        oam_info.is_protection_path = CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_Y1731_LMEP_FLAG_PROTECTION_PATH);
        oam_info.dsma_en = 0;
        oam_info.mep_index = p_sys_lmep->lmep_index;
        sys_usw_nh_update_oam_en(lchip, old_nhid, &oam_info);
    }

    return CTC_E_NONE;
}


int32
_sys_usw_bfd_update_lmep_lm(uint8 lchip, ctc_oam_update_t* p_update,
                                         sys_oam_lmep_t* p_sys_lmep)
{
    uint32 index        = 0;
    uint32 lm_en        = 0;
    int32  ret       = CTC_E_NONE;
    DsEgressXcOamMplsLabelHashKey_m mpls_oam_key;
    DsEgressXcOamMplsSectionHashKey_m section_oam_key;
    void* p_hashkey = NULL;
    sys_oam_key_com_t*  p_key_com       = NULL;
    uint32 tbl_id = 0;
    sys_oam_chan_tp_t* p_sys_chan_tp          = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    /*1. lookup chan and check exist*/
    p_sys_chan_tp = (sys_oam_chan_tp_t*)p_sys_lmep->p_sys_chan;
    p_key_com = &p_sys_chan_tp->key.com;

    if (CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_LM_EN))
    {
        if((CTC_OAM_BFD_LMEP_UPDATE_TYPE_LM_COS_TYPE == p_update->update_type)
            || (CTC_OAM_BFD_LMEP_UPDATE_TYPE_LM_COS == p_update->update_type))
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "  Config is invalid lm is Enable\n");
			return CTC_E_INVALID_CONFIG;
        }
    }

    /*2. lmep db & asic update*/
    switch (p_update->update_type)
    {
    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_LM_EN:
        if (1 == p_update->update_value)
        {
            CTC_SET_FLAG(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_LM_EN);
            p_sys_chan_tp->lm_type = CTC_OAM_LM_TYPE_SINGLE;
            if (!p_sys_chan_tp->lm_index_alloced)
            {
                CTC_ERROR_RETURN(_sys_usw_oam_lm_build_index(lchip, &p_sys_chan_tp->com, FALSE, 0xFF));
                p_sys_chan_tp->lm_index_alloced = TRUE;
            }
        }
        else
        {
            CTC_UNSET_FLAG(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_LM_EN);
            p_sys_chan_tp->lm_type = CTC_OAM_LM_TYPE_NONE;
            if (p_sys_chan_tp->lm_index_alloced)
            {
                CTC_ERROR_RETURN(_sys_usw_oam_lm_free_index(lchip, &p_sys_chan_tp->com, FALSE, 0xFF));
                p_sys_chan_tp->lm_index_alloced = FALSE;
            }
        }

        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_LM_COS_TYPE:
        p_sys_chan_tp->lm_cos_type = p_update->update_value;

        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_LM_COS:
        p_sys_chan_tp->lm_cos = p_update->update_value;

        break;

    case CTC_OAM_BFD_LMEP_UPDATE_TYPE_LOCK:
        if (p_sys_chan_tp->key.section_oam)
        {
            SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " TP section BFD do not support lock\n");
            return CTC_E_NOT_SUPPORT;
        }
        CTC_MAX_VALUE_CHECK(p_update->update_value, 1);
        p_sys_lmep->lock_en = p_update->update_value;

        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    lm_en = CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_LM_EN);

    if (p_sys_chan_tp->key.section_oam)
    {
        sal_memset(&section_oam_key, 0, sizeof(DsEgressXcOamMplsSectionHashKey_m));
        p_hashkey = &section_oam_key;
        tbl_id = DsEgressXcOamMplsSectionHashKey_t;
    }
    else
    {
        sal_memset(&mpls_oam_key, 0, sizeof(DsEgressXcOamMplsLabelHashKey_m));
        p_hashkey = &mpls_oam_key;
        tbl_id = DsEgressXcOamMplsLabelHashKey_t;
    }

    index = p_key_com->key_index;
    CTC_ERROR_RETURN(sys_usw_oam_key_read_io(lchip, tbl_id, index, p_hashkey));

    if (p_sys_chan_tp->key.section_oam)
    {
        SetDsEgressXcOamMplsSectionHashKey(V, lmIndexBase_f , p_hashkey, p_sys_chan_tp->com.lm_index_base) ;
        SetDsEgressXcOamMplsSectionHashKey(V, lmCos_f       , p_hashkey, p_sys_chan_tp->lm_cos) ;
        SetDsEgressXcOamMplsSectionHashKey(V, lmType_f      , p_hashkey, p_sys_chan_tp->lm_type) ;
        SetDsEgressXcOamMplsSectionHashKey(V, mplsLmType_f      , p_hashkey, 1) ;/*64bits stats*/
    }
    else
    {
        SetDsEgressXcOamMplsLabelHashKey(V, lmIndexBase_f   , p_hashkey, p_sys_chan_tp->com.lm_index_base)
        SetDsEgressXcOamMplsLabelHashKey(V, lmCos_f         , p_hashkey, p_sys_chan_tp->lm_cos) ;
        SetDsEgressXcOamMplsLabelHashKey(V, lmType_f        , p_hashkey, p_sys_chan_tp->lm_type  ) ;
        SetDsEgressXcOamMplsLabelHashKey(V, mplsLmType_f      , p_hashkey, 1) ;/*64bits stats*/
    }

    CTC_ERROR_RETURN(sys_usw_oam_key_write_io(lchip, tbl_id, index, p_hashkey));

    if (!p_sys_chan_tp->key.section_oam)
    {
        /* update dsmpls */
        ctc_mpls_property_t mpls_pro_info;
        sys_nh_update_oam_info_t oam_info;
        uint32 value = 0;

        sal_memset(&oam_info, 0, sizeof(sys_nh_update_oam_info_t));
        sal_memset(&mpls_pro_info, 0, sizeof(ctc_mpls_property_t));
        mpls_pro_info.label = p_sys_chan_tp->key.label;
        mpls_pro_info.space_id = p_sys_chan_tp->key.spaceid;

        mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_LMEN;
        mpls_pro_info.value = (void*)(&lm_en);
        CTC_ERROR_RETURN(sys_usw_mpls_set_ilm_property(lchip, &mpls_pro_info));

        if(CTC_OAM_BFD_LMEP_UPDATE_TYPE_LOCK == p_update->update_type)
        {
            mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_CHK;
            value = p_sys_lmep->lock_en? 5 : 1;
            mpls_pro_info.value = (void*)(&value);
            CTC_ERROR_RETURN(sys_usw_mpls_set_ilm_property(lchip, &mpls_pro_info));
        }

        oam_info.lm_en = lm_en;
        oam_info.lock_en = p_sys_lmep->lock_en;
        oam_info.is_protection_path =  CTC_FLAG_ISSET(p_sys_lmep->flag, CTC_OAM_BFD_LMEP_FLAG_PROTECTION_PATH);
        oam_info.oam_mep_index = p_sys_chan_tp->mep_index_in_key;
        ret = sys_usw_nh_update_oam_en(lchip, p_sys_lmep->nhid, &oam_info);
        if (ret && (CTC_OAM_Y1731_LMEP_UPDATE_TYPE_LOCK == p_update->update_type))
        {
            mpls_pro_info.property_type = SYS_MPLS_ILM_OAM_CHK;
            value = p_sys_lmep->lock_en? 1 : 5;
            mpls_pro_info.value = (void*)(&value);
            sys_usw_mpls_set_ilm_property(lchip, &mpls_pro_info);
            return ret;
        }

    }
    else
    { /*need update port or interface lm enable*/
        uint32 cmd      = 0;
        uint32 field    = 0;
        uint8  gchip    = 0;

        field = lm_en;
        if (g_oam_master[lchip]->tp_section_oam_based_l3if)
        {
            index = p_sys_chan_tp->key.gport_l3if_id;
            cmd = DRV_IOW(DsSrcInterface_t, DsSrcInterface_mplsSectionLmEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));
            cmd = DRV_IOW(DsDestInterface_t, DsDestInterface_mplsSectionLmEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));
        }
        else
        {
            index = SYS_MAP_CTC_GPORT_TO_DRV_LPORT(p_sys_chan_tp->key.gport_l3if_id);
            gchip = SYS_MAP_CTC_GPORT_TO_GCHIP(p_sys_chan_tp->key.gport_l3if_id);
            if (FALSE == sys_usw_chip_is_local(lchip, gchip))
            {
                return CTC_E_NONE;
            }
            cmd = DRV_IOW(DsSrcPort_t, DsSrcPort_mplsSectionLmEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));
            cmd = DRV_IOW(DsDestPort_t, DsDestPort_mplsSectionLmEn_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));
        }
    }

    return CTC_E_NONE;
}


#define RMEP "Function Begin"

sys_oam_rmep_t*
_sys_usw_bfd_rmep_lkup(uint8 lchip, sys_oam_lmep_t* p_sys_lmep)
{
    sys_oam_rmep_t sys_oam_rmep_bfd;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&sys_oam_rmep_bfd, 0, sizeof(sys_oam_rmep_t));

    return (sys_oam_rmep_t*)_sys_usw_oam_rmep_lkup(lchip, p_sys_lmep, &sys_oam_rmep_bfd);
}


sys_oam_rmep_t*
_sys_usw_bfd_rmep_build_node(uint8 lchip, ctc_oam_rmep_t* p_rmep_param)
{
    sys_oam_rmep_t* p_sys_oam_rmep_com = NULL;

    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sys_oam_rmep_com = (sys_oam_rmep_t*)mem_malloc(MEM_OAM_MODULE, sizeof(sys_oam_rmep_t));
    if (NULL != p_sys_oam_rmep_com)
    {
        sal_memset(p_sys_oam_rmep_com, 0, sizeof(sys_oam_rmep_t));
        p_sys_oam_rmep_com->rmep_index = p_rmep_param->rmep_index;   /*index alloc by system*/
        p_sys_oam_rmep_com->flag = p_rmep_param->u.bfd_rmep.flag;
    }

    return p_sys_oam_rmep_com;
}

int32
_sys_usw_bfd_rmep_free_node(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_bfd)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    mem_free(p_sys_rmep_bfd);
    p_sys_rmep_bfd = NULL;

    return CTC_E_NONE;
}

int32
_sys_usw_bfd_rmep_build_index(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_bfd)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_build_index(lchip, p_sys_rmep_bfd));

    return CTC_E_NONE;
}

int32
_sys_usw_bfd_rmep_free_index(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_bfd)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_free_index(lchip, p_sys_rmep_bfd));

    return CTC_E_NONE;
}

int32
_sys_usw_bfd_rmep_add_to_db(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_bfd)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_add_to_db(lchip, p_sys_rmep_bfd));
    return CTC_E_NONE;
}

int32
_sys_usw_bfd_rmep_del_from_db(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_bfd)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_del_from_db(lchip, p_sys_rmep_bfd));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_bfd_rmep_add_tbl_to_asic(uint8 lchip, sys_oam_rmep_t* p_sys_rmep,
                                                                 ctc_oam_bfd_rmep_t* p_ctc_rmep)
{
    uint8 mep_type      = CTC_OAM_MEP_TYPE_MAX;

    DsBfdRmep_m ds_bfd_rmep;
    DsBfdRmep_m DsBfdRmep_mask;
    tbl_entry_t  tbl_entry;
    sys_oam_lmep_t* p_sys_lmep = NULL;
    sys_oam_nhop_info_t oam_nhop_info;

    uint32 cmd          = 0;
    uint32 index        = 0;
    uint32 rmep_flag    = 0;
    uint32 lmep_flag    = 0;
    uint32 maid_index = 0;
    uint32 maid_entry_num = 0;
    uint8 u_active = 0;
     /*uint8 u_aps_en = 0;*/
    p_sys_lmep     = p_sys_rmep->p_sys_lmep;
    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
    {
        goto RETURN;
    }

    mep_type    = p_sys_lmep->p_sys_chan->mep_type;

    rmep_flag   = p_ctc_rmep->flag;
    lmep_flag   = ((sys_oam_lmep_t*)p_sys_lmep)->flag;

    sal_memset(&ds_bfd_rmep, 0, sizeof(DsBfdRmep_m));
    sal_memset(&DsBfdRmep_mask, 0, sizeof(DsBfdRmep_m));

    sal_memset(&oam_nhop_info, 0, sizeof(sys_oam_nhop_info_t));
    CTC_ERROR_RETURN(_sys_usw_oam_get_nexthop_info(lchip, p_sys_lmep->nhid, CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_PROTECTION_PATH), &oam_nhop_info));
    index = p_sys_rmep->rmep_index;
    cmd = DRV_IOR(DsBfdRmep_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_bfd_rmep));

    u_active      = CTC_FLAG_ISSET(rmep_flag, CTC_OAM_BFD_RMEP_FLAG_MEP_EN);
     /*u_aps_en      = CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_APS_EN);*/
    SetDsBfdRmep(V, active_f, &ds_bfd_rmep, u_active);

    SetDsBfdRmep(V, isRemote_f,&ds_bfd_rmep,1);
    SetDsBfdRmep(V, isBfd_f,&ds_bfd_rmep,1);
    SetDsBfdRmep(V, ethOamP2PMode_f,&ds_bfd_rmep,1);
    SetDsBfdRmep(V, discChkEn_f,&ds_bfd_rmep,1);
    SetDsBfdRmep(V, pbit_f,&ds_bfd_rmep,0);
    SetDsBfdRmep(V, fbit_f,&ds_bfd_rmep,0);
    if (g_oam_master[lchip]->timer_update_disable)
    {
        SetDsBfdRmep(V, firstPktRx_f, &ds_bfd_rmep , 1);
    }
    else
    {
        SetDsBfdRmep(V, firstPktRx_f, &ds_bfd_rmep, 0);
    }

    SetDsBfdRmep(V, apsSignalFailRemote_f,&ds_bfd_rmep,0);
    SetDsBfdRmep(V, learnEn_f,&ds_bfd_rmep,1);

    SetDsBfdRmep(V, remoteDiag_f,&ds_bfd_rmep,p_ctc_rmep->remote_diag);
    SetDsBfdRmep(V, remoteStat_f,&ds_bfd_rmep,p_ctc_rmep->remote_state);
    SetDsBfdRmep(V, remoteDisc_f,&ds_bfd_rmep,p_ctc_rmep->remote_discr);
    SetDsBfdRmep(V, defectMult_f,&ds_bfd_rmep,p_ctc_rmep->remote_detect_mult);

    SetDsBfdRmep(V, requiredMinRxInterval_f,&ds_bfd_rmep, p_ctc_rmep->required_min_rx_interval);
#if (SDK_WORK_PLATFORM == 0)
    SetDsBfdRmep(V, actualRxInterval_f,&ds_bfd_rmep,1000);
#else
    SetDsBfdRmep(V, actualRxInterval_f,&ds_bfd_rmep,4);
#endif

    if (oam_nhop_info.aps_bridge_en)
    {/*use multicast to process aps group*/
        SetDsBfdRmep(V, apsBridgeEn_f,&ds_bfd_rmep,1);
    }

    if (CTC_OAM_MEP_TYPE_IP_BFD == mep_type || CTC_OAM_MEP_TYPE_MICRO_BFD == mep_type)
    {
        uint8 is_multi_hop    = !CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_IP_SINGLE_HOP);
        SetDsBfdRmep(V, isMultiHop_f,&ds_bfd_rmep,is_multi_hop);
    }
    else if (CTC_OAM_MEP_TYPE_MPLS_BFD == mep_type)
    {
        SetDsBfdRmep(V, ipBfdTtl_f , &ds_bfd_rmep, 255);   /*VCCV BFD*/
        if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV))
        {
            SetDsBfdRmep(V, achChanType_f, &ds_bfd_rmep, 3);   /*0x7*/
            if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_INITIATOR)
                || CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_SBFD_REFLECTOR))
            {
                SetDsBfdRmep(V, achChanType_f, &ds_bfd_rmep, 5);    /*using default ach type 0x0008*/
                SetDsBfdRmep(V, achChanType_f, &DsBfdRmep_mask, 0);
            }
        }
        else if(CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV_IPV4))
        {
            SetDsBfdRmep(V, achChanType_f,&ds_bfd_rmep,2);  /* 0x21*/
        }
        else if(CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_MPLS_PW_VCCV_IPV6))
        {
            SetDsBfdRmep(V, achChanType_f,&ds_bfd_rmep,4);  /* 0x57*/
        }
        else
        {
            SetDsBfdRmep(V, ipBfdTtl_f , &ds_bfd_rmep, 1);   /*MPLS BFD*/
        }
    }
    else if (CTC_OAM_MEP_TYPE_MPLS_TP_BFD == mep_type)
    {
        uint8 is_csf_en       = CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_CSF_EN);

        SetDsBfdRmep(V, isCvEn_f,&ds_bfd_rmep,0);
        SetDsBfdRmep(V, isCsfEn_f,&ds_bfd_rmep,is_csf_en);
        SetDsBfdRmep(V, achChanType_f,&ds_bfd_rmep,1);


        /*{5'd0,maIdLengthType[1:0], maNameIndex[13:0], ccmInterval[2:0],cvWhile[2:0], dMisConWhile[3:0],dMisCon}*/
        if (CTC_FLAG_ISSET(rmep_flag, CTC_OAM_BFD_RMEP_FLAG_TP_WITH_MEP_ID))
        {
            /*mepid index*/
            maid_index = (p_sys_rmep->p_sys_maid->maid_index) & 0x3FFF;
            SetDsBfdRmep(V, maNameIndex_f,&ds_bfd_rmep,maid_index);
            /*mepid length*/
            maid_entry_num= (p_sys_rmep->p_sys_maid->maid_entry_num >> 1) & 0x3;
            SetDsBfdRmep(V, maIdLengthType_f,&ds_bfd_rmep,maid_entry_num);
            /*interval to 1s*/
#if (SDK_WORK_PLATFORM == 0)
            SetDsBfdRmep(V, ccmInterval_f, &ds_bfd_rmep, 4);
#else
            SetDsBfdRmep(V, ccmInterval_f, &ds_bfd_rmep, 1);
#endif
            /*cv while*/
            SetDsBfdRmep(V, cvWhile_f,&ds_bfd_rmep,1);
            /*dMisconwhile*/
            SetDsBfdRmep(V, dMisConWhile_f,&ds_bfd_rmep,4);
        }

        SetDsBfdRmep(V, ccTxMode_f,&ds_bfd_rmep,1);
         /*SetDsBfdRmep(V, actualRxInterval_f,&ds_bfd_rmep,4);*/

    }



    cmd = DRV_IOW(DsBfdRmep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_bfd_rmep;
    tbl_entry.mask_entry = (uint32*)&DsBfdRmep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));
RETURN:

    return CTC_E_NONE;
}

int32
_sys_usw_bfd_rmep_add_to_asic(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_com, ctc_oam_bfd_rmep_t* p_ctc_rmep)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sys_rmep_com);
    CTC_PTR_VALID_CHECK(p_ctc_rmep);

    CTC_ERROR_RETURN(_sys_usw_bfd_rmep_add_tbl_to_asic(lchip, p_sys_rmep_com, p_ctc_rmep));

    return CTC_E_NONE;
}

int32
_sys_usw_bfd_rmep_del_from_asic(uint8 lchip, sys_oam_rmep_t* p_sys_rmep_bfd)
{
    sys_oam_rmep_t* p_sys_rmep = NULL;
    uint32 cmd        = 0;
    uint32 index      = 0;
    uint8 ipv6_sa_index = 0;
    uint32 lmep_flag = 0;

    DsBfdRmep_m  ds_bfd_rmep;
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    p_sys_rmep = p_sys_rmep_bfd;
    if ((p_sys_rmep->p_sys_lmep->p_sys_chan->mep_type == CTC_OAM_MEP_TYPE_IP_BFD)
        ||(p_sys_rmep->p_sys_lmep->p_sys_chan->mep_type == CTC_OAM_MEP_TYPE_MPLS_BFD)
        ||(p_sys_rmep->p_sys_lmep->p_sys_chan->mep_type == CTC_OAM_MEP_TYPE_MICRO_BFD))
    {
        lmep_flag   = p_sys_rmep->p_sys_lmep->flag;
        if (CTC_FLAG_ISSET(lmep_flag, CTC_OAM_BFD_LMEP_FLAG_IPV6_ENCAP))
        {
            index = p_sys_rmep->rmep_index;
            cmd = DRV_IOR(DsBfdRmep_t, DRV_ENTRY_FLAG);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_bfd_rmep));
            ipv6_sa_index = GetDsBfdRmep(V, ipSa_f , &ds_bfd_rmep);
            CTC_ERROR_RETURN(_sys_usw_bfd_free_ipv6_idx(lchip, ipv6_sa_index));
        }
    }

    CTC_ERROR_RETURN(_sys_usw_oam_rmep_del_from_asic(lchip, p_sys_rmep_bfd));

    return CTC_E_NONE;
}

int32
_sys_usw_bfd_rmep_update_tbl_asic(uint8 lchip, ctc_oam_update_t* p_update, sys_oam_rmep_t* p_sys_rmep)
{
    sys_oam_lmep_t* p_sys_lmep = NULL;
    uint32 cmd        = 0;
    uint32 index      = 0;
    uint32 l_index    = 0;
    DsBfdRmep_m ds_bfd_rmep;
    DsBfdRmep_m DsBfdRmep_mask;
    DsBfdMep_m ds_bfd_mep;
    DsBfdMep_m DsBfdMep_mask;
    tbl_entry_t  tbl_entry;
    uint16 required_min_rx_interval = 0;
    uint8 detect_mult = 0;
    sal_systime_t tv;
    uint32 interval = 3000000;  /*3s*/

    p_sys_lmep = p_sys_rmep->p_sys_lmep;
    if (SYS_OAM_NO_MEP_OPERATION(lchip, p_sys_lmep->p_sys_chan->master_chipid))
    {
        goto RETURN;
    }

    index = p_sys_rmep->rmep_index;
    l_index = index - 1;
    sal_memset(&ds_bfd_rmep, 0, sizeof(DsBfdRmep_m));
    sal_memset(&DsBfdRmep_mask, 0xFF, sizeof(DsBfdRmep_m));
    cmd = DRV_IOR(DsBfdRmep_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &ds_bfd_rmep));

    sal_memset(&ds_bfd_mep, 0, sizeof(DsBfdMep_m));
    sal_memset(&DsBfdMep_mask, 0xFF, sizeof(DsBfdMep_m));
    cmd = DRV_IOR(DsBfdMep_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, l_index, cmd, &ds_bfd_mep));

    switch (p_update->update_type)
    {
        case CTC_OAM_BFD_RMEP_UPDATE_TYPE_MEP_EN:
            if (CTC_FLAG_ISSET(p_sys_rmep->flag, CTC_OAM_BFD_RMEP_FLAG_MEP_EN))
            {
                SetDsBfdRmep(V, active_f, &ds_bfd_rmep, 1);
            }
            else
            {
                SetDsBfdRmep(V, active_f, &ds_bfd_rmep, 0);
            }
            SetDsBfdRmep(V, active_f, &DsBfdRmep_mask, 0);
            break;

        case CTC_OAM_BFD_RMEP_UPDATE_TYPE_RMEP_SF_STATE:
            return CTC_E_NOT_SUPPORT;

        case CTC_OAM_BFD_RMEP_UPDATE_TYPE_DIAG:
            SetDsBfdRmep(V, remoteDiag_f, &ds_bfd_rmep, p_update->update_value);
            SetDsBfdRmep(V, remoteDiag_f, &DsBfdRmep_mask, 0);
            break;

        case CTC_OAM_BFD_RMEP_UPDATE_TYPE_STATE:
            SetDsBfdRmep(V, remoteStat_f, &ds_bfd_rmep, p_update->update_value);
            SetDsBfdRmep(V, remoteStat_f, &DsBfdRmep_mask, 0);
            break;

        case CTC_OAM_BFD_RMEP_UPDATE_TYPE_DISCR:
            SetDsBfdRmep(V, remoteDisc_f, &ds_bfd_rmep, p_update->update_value);
            SetDsBfdRmep(V, remoteDisc_f, &DsBfdRmep_mask, 0);
            break;

        case CTC_OAM_BFD_RMEP_UPDATE_TYPE_RX_TIMER:
            required_min_rx_interval = ((ctc_oam_bfd_timer_t*)p_update->p_update_value)->min_interval;
            detect_mult = ((ctc_oam_bfd_timer_t*)p_update->p_update_value)->detect_mult;
            if (CTC_OAM_BFD_STATE_UP == GetDsBfdRmep(V, remoteStat_f, &ds_bfd_rmep))
            {
                if ((GetDsBfdRmep(V, pbit_f, &ds_bfd_rmep) || GetDsBfdRmep(V, fbit_f, &ds_bfd_rmep))
                    && !g_oam_master[lchip]->timer_update_disable)
                {
                    return CTC_E_HW_BUSY;
                }
                sal_gettime(&tv);
                if ((((tv.tv_sec - p_sys_lmep->tv_p_set.tv_sec)*1000000 + (tv.tv_usec - p_sys_lmep->tv_p_set.tv_usec)) < interval)
                    && !g_oam_master[lchip]->timer_update_disable)
                {
                    return CTC_E_HW_BUSY;
                }
                sal_gettime(&p_sys_lmep->tv_p_set);
                SetDsBfdRmep(V, pbit_f, &ds_bfd_rmep, 1);
                SetDsBfdRmep(V, pbit_f, &DsBfdRmep_mask, 0);
                SetDsBfdRmep(V, fbit_f, &ds_bfd_rmep, 0);
                SetDsBfdRmep(V, fbit_f, &DsBfdRmep_mask, 0);
                if (required_min_rx_interval > GetDsBfdRmep(V, actualRxInterval_f, &ds_bfd_rmep))
                {
                    SetDsBfdRmep(V, actualRxInterval_f, &ds_bfd_rmep, required_min_rx_interval);
                    SetDsBfdRmep(V, actualRxInterval_f, &DsBfdRmep_mask, 0);
                }
            }
            SetDsBfdRmep(V, requiredMinRxInterval_f, &ds_bfd_rmep, required_min_rx_interval);
            SetDsBfdRmep(V, requiredMinRxInterval_f, &DsBfdRmep_mask, 0);
            SetDsBfdRmep(V, defectMult_f, &ds_bfd_rmep, detect_mult);
            SetDsBfdRmep(V, defectMult_f, &DsBfdRmep_mask, 0);
            SetDsBfdRmep(V, defectWhile_f, &ds_bfd_rmep, detect_mult*GetDsBfdRmep(V, actualRxInterval_f, &ds_bfd_rmep));
            SetDsBfdRmep(V, defectWhile_f, &DsBfdRmep_mask, 0);
            break;

        case CTC_OAM_BFD_RMEP_UPDATE_TYPE_ACTUAL_RX_TIMER:
            SetDsBfdRmep(V, actualRxInterval_f, &ds_bfd_rmep, ((ctc_oam_bfd_timer_t*)p_update->p_update_value)->min_interval);
            SetDsBfdRmep(V, actualRxInterval_f, &DsBfdRmep_mask, 0);
            SetDsBfdRmep(V, defectWhile_f, &ds_bfd_rmep, GetDsBfdRmep(V, defectMult_f, &ds_bfd_rmep)*GetDsBfdRmep(V, actualRxInterval_f, &ds_bfd_rmep));
            SetDsBfdRmep(V, defectWhile_f, &DsBfdRmep_mask, 0);
            break;
        case CTC_OAM_BFD_RMEP_UPDATE_TYPE_LEARN_EN:
            SetDsBfdRmep(V, learnEn_f, &ds_bfd_rmep, p_update->update_value);
            SetDsBfdRmep(V, learnEn_f, &DsBfdRmep_mask, 0);
            break;

        case CTC_OAM_BFD_RMEP_UPDATE_TYPE_HW_APS:
            {
                ctc_oam_hw_aps_t* p_hw_aps = NULL;
                p_hw_aps = (ctc_oam_hw_aps_t*)p_update->p_update_value;
                SetDsBfdMep(V, apsGroupId_f, &ds_bfd_mep , (p_hw_aps->aps_group_id >> 6) & 0x3f);
                SetDsBfdMep(V, apsGroupId_f, &DsBfdMep_mask ,  0);
                SetDsBfdRmep(V, apsGroupId_f, &ds_bfd_rmep , (p_hw_aps->aps_group_id & 0x3f));
                SetDsBfdRmep(V, apsGroupId_f, &DsBfdRmep_mask ,  0);
                SetDsBfdRmep(V, protectionPath_f, &ds_bfd_rmep, (p_hw_aps->protection_path) ? 1 : 0);
                SetDsBfdRmep(V, protectionPath_f, &DsBfdRmep_mask , 0);
            }
            break;
        case CTC_OAM_BFD_RMEP_UPDATE_TYPE_HW_APS_EN:
            SetDsBfdRmep(V, fastApsEn_f, &ds_bfd_rmep , p_update->update_value);
            SetDsBfdRmep(V, fastApsEn_f, &DsBfdRmep_mask , 0);
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }
    if (g_oam_master[lchip]->timer_update_disable)
    {
        SetDsBfdRmep(V, defectWhile_f, &ds_bfd_rmep, 0x3FFF);
        SetDsBfdRmep(V, defectWhile_f, &DsBfdRmep_mask, 0);
    }
    cmd = DRV_IOW(DsBfdRmep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_bfd_rmep;
    tbl_entry.mask_entry = (uint32*)&DsBfdRmep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &tbl_entry));

    cmd = DRV_IOW(DsBfdMep_t, DRV_ENTRY_FLAG);
    tbl_entry.data_entry = (uint32*)&ds_bfd_mep;
    tbl_entry.mask_entry = (uint32*)&DsBfdMep_mask;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, l_index, cmd, &tbl_entry));
RETURN:

    return CTC_E_NONE;
}


int32
_sys_usw_bfd_rmep_update_asic(uint8 lchip, ctc_oam_update_t* p_update, sys_oam_rmep_t* p_sys_rmep)
{
    SYS_OAM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_update);

    switch (p_update->update_type)
    {
    case CTC_OAM_BFD_RMEP_UPDATE_TYPE_MEP_EN:
        if (1 == p_update->update_value)
        {
            CTC_SET_FLAG(p_sys_rmep->flag, CTC_OAM_BFD_RMEP_FLAG_MEP_EN);
        }
        else
        {
            CTC_UNSET_FLAG(p_sys_rmep->flag, CTC_OAM_BFD_RMEP_FLAG_MEP_EN);
        }
        break;

    case CTC_OAM_BFD_RMEP_UPDATE_TYPE_RMEP_SF_STATE:
        return CTC_E_NOT_SUPPORT;

    case CTC_OAM_BFD_RMEP_UPDATE_TYPE_DIAG:
        break;

    case CTC_OAM_BFD_RMEP_UPDATE_TYPE_STATE:
        break;

    case CTC_OAM_BFD_RMEP_UPDATE_TYPE_DISCR:
        break;

    case CTC_OAM_BFD_RMEP_UPDATE_TYPE_RX_TIMER:
         /*p_ctc_bfd_rmep->remote_detect_mult          = ((ctc_oam_bfd_timer_t*)p_update->p_update_value)->detect_mult;*/
        break;
    case CTC_OAM_BFD_RMEP_UPDATE_TYPE_ACTUAL_RX_TIMER:
        break;
    case CTC_OAM_BFD_RMEP_UPDATE_TYPE_LEARN_EN:
        break;
    case CTC_OAM_BFD_RMEP_UPDATE_TYPE_HW_APS:
        break;
    case CTC_OAM_BFD_RMEP_UPDATE_TYPE_HW_APS_EN:
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_bfd_rmep_update_tbl_asic(lchip, p_update, p_sys_rmep));

    return CTC_E_NONE;
}

#define GLOBAL_SET "Function Begin"

int32
_sys_usw_bfd_set_slow_interval(uint8 lchip, uint32 interval)
{
    uint32 cmd = 0;
    DsBfdSlowIntervalCfg_m ds_bfd_slow_interval_cfg;

    if (interval > 1000) /*1s*/
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&ds_bfd_slow_interval_cfg, 0, sizeof(DsBfdSlowIntervalCfg_m));

    SetDsBfdSlowIntervalCfg(V, txSlowInterval_f, &ds_bfd_slow_interval_cfg, interval);
    SetDsBfdSlowIntervalCfg(V, rxSlowInterval_f, &ds_bfd_slow_interval_cfg, interval);

    cmd = DRV_IOW(DsBfdSlowIntervalCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_bfd_slow_interval_cfg));

    return CTC_E_NONE;
}

int32
_sys_usw_bfd_set_timer_neg_interval(uint8 lchip, ctc_oam_bfd_timer_neg_t* interval)
{
    uint32 cmd = 0;
    uint8 i = 0;
    DsBfdIntervalCam_m ds_bfd_interval_cam;
    CTC_PTR_VALID_CHECK(interval);
    if (interval->interval_num > CTC_OAM_BFD_TIMER_NEG_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }
    for (i = 0; i < interval->interval_num; i++)
    {
        if (interval->interval[i] > 0x3FF)
        {
            return CTC_E_INVALID_PARAM;
        }
        cmd = DRV_IOR(DsBfdIntervalCam_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, &ds_bfd_interval_cam));
        if (interval->is_rx)
        {
            SetDsBfdIntervalCam(V, rxInterval_f, &ds_bfd_interval_cam, (interval->interval[i]));
        }
        else
        {
            SetDsBfdIntervalCam(V, txInterval_f, &ds_bfd_interval_cam, (interval->interval[i]));
        }
        cmd = DRV_IOW(DsBfdIntervalCam_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, &ds_bfd_interval_cam));
    }
    return CTC_E_NONE;
}

#define INIT "Function Begin"

int32
_sys_usw_bfd_register_init(uint8 lchip, ctc_oam_global_t* p_oam_glb)
{
#define BFD_INTERVAL_CAM_NUM 32
    uint32 cmd      = 0;
    uint32 field    = 0;
    uint8 i = 0;
    uint32 index    = 0;
    ParserLayer4AppCtl_m parser_layer4_app_ctl;
    ParserLayer4AchCtl_m parser_layer4_ach_ctl;
    OamParserEtherCtl_m  oam_parser_ether_ctl;
    IpeMplsCtl_m          ipe_mpls_ctl;
    EpeAclQosCtl_m       epe_acl_qos_ctl;
    IpeLookupCtl_m        ipe_lookup_ctl;
    OamEtherTxCtl_m      oam_ether_tx_ctl;
    OamRxProcEtherCtl_m oam_rx_proc_ether_ctl;
    OamHeaderEditCtl_m   oam_header_edit_ctl;
    DsBfdSlowIntervalCfg_m ds_bfd_slow_interval_cfg;
    DsBfdIntervalCam_m ds_bfd_interval_cam;
    EpeL2EditCtl_m epe_l2_edit;
    uint16 bfd_support_interval[BFD_INTERVAL_CAM_NUM] =
    {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1023
    };
    hw_mac_addr_t   hw_mac = {0};
    mac_addr_t micro_bfd_mac = {1 , 0, 0x5E, 0x90, 0, 1};
    SYS_USW_SET_HW_MAC(hw_mac, (uint8*)micro_bfd_mac);

    /*Parser*/
    sal_memset(&parser_layer4_app_ctl, 0, sizeof(ParserLayer4AppCtl_m));
    cmd = DRV_IOR(ParserLayer4AppCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &parser_layer4_app_ctl));
    SetParserLayer4AppCtl(V, bfdEn_f, &parser_layer4_app_ctl, 1);
    SetParserLayer4AppCtl(V, sBfdEn_f, &parser_layer4_app_ctl, 1);
    cmd = DRV_IOW(ParserLayer4AppCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &parser_layer4_app_ctl));

    sal_memset(&parser_layer4_ach_ctl, 0, sizeof(ParserLayer4AchCtl_m));
    cmd = DRV_IOR(ParserLayer4AchCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &parser_layer4_ach_ctl));
    SetParserLayer4AchCtl(V, achBfdCcType_f, &parser_layer4_ach_ctl, 0x0022);
    SetParserLayer4AchCtl(V, achBfdCvType_f, &parser_layer4_ach_ctl, 0x0023);
    SetParserLayer4AchCtl(V, achDlmType_f,   &parser_layer4_ach_ctl, 0x000A);
    SetParserLayer4AchCtl(V, achIlmDmType_f, &parser_layer4_ach_ctl, 0x000B);
    SetParserLayer4AchCtl(V, achDmType_f,    &parser_layer4_ach_ctl, 0x000C);
    SetParserLayer4AchCtl(V, achDlmDmType_f, &parser_layer4_ach_ctl, 0x000D);
    SetParserLayer4AchCtl(V, achIlmDmType_f, &parser_layer4_ach_ctl, 0x000E);
    cmd = DRV_IOW(ParserLayer4AchCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &parser_layer4_ach_ctl));

    sal_memset(&oam_parser_ether_ctl, 0, sizeof(OamParserEtherCtl_m));
    cmd = DRV_IOR(OamParserEtherCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_parser_ether_ctl));
    SetOamParserEtherCtl(V, achBfdChanTypeCc_f, &oam_parser_ether_ctl, 0x0022);
    SetOamParserEtherCtl(V, achBfdChanTypeCv_f, &oam_parser_ether_ctl, 0x0023);
    SetOamParserEtherCtl(V, achChanTypeDm_f, &oam_parser_ether_ctl, 0x000C);
    SetOamParserEtherCtl(V, achChanTypeDlmdm_f, &oam_parser_ether_ctl, 0x000D);
    SetOamParserEtherCtl(V, achChanTypeDlm_f, &oam_parser_ether_ctl, 0x000A);
    SetOamParserEtherCtl(V, achChanTypeCsf_f, &oam_parser_ether_ctl, (p_oam_glb->tp_csf_ach_chan_type));
    SetOamParserEtherCtl(V, achChanTypeMcc_f , &oam_parser_ether_ctl, 1);
    SetOamParserEtherCtl(V, achChanTypeScc_f , &oam_parser_ether_ctl, 2);
    SetOamParserEtherCtl(V, mplsTpOamAlertLabel_f, &oam_parser_ether_ctl, 13);
    SetOamParserEtherCtl(V, mplstpDmMsgLen_f, &oam_parser_ether_ctl, 44);
    SetOamParserEtherCtl(V, mplstpDlmMsgLen_f, &oam_parser_ether_ctl, 52);
    SetOamParserEtherCtl(V, mplstpDlmdmMsgLen_f, &oam_parser_ether_ctl, 76);

    SetOamParserEtherCtl(V, bfdLenCheckEn_f, &oam_parser_ether_ctl, 1);
    SetOamParserEtherCtl(V, bfdP2pCheckEn_f, &oam_parser_ether_ctl, 1);
    SetOamParserEtherCtl(V, bfdPfbitConflictCheckEn_f, &oam_parser_ether_ctl, 1);
    SetOamParserEtherCtl(V, bfdDetectMultCheckEn_f, &oam_parser_ether_ctl, 1);
    SetOamParserEtherCtl(V, bfdAbitCheckEn_f, &oam_parser_ether_ctl, 1);
    SetOamParserEtherCtl(V, bfdMultipointChk_f, &oam_parser_ether_ctl, 1);
    SetOamParserEtherCtl(V, bfdAchMepIdTypeChkEn_f, &oam_parser_ether_ctl, 1);
    SetOamParserEtherCtl(V, bfdAchMepIdMaxType_f, &oam_parser_ether_ctl, 2);
    SetOamParserEtherCtl(V, bfdAchMepIdLenChkEn_f, &oam_parser_ether_ctl, 1);
    SetOamParserEtherCtl(V, bfdAchMepIdMaxLen_f, &oam_parser_ether_ctl, 32);
    SetOamParserEtherCtl(V, achBfdDefaultChanType_f, &oam_parser_ether_ctl, 8);  /*used as sbfd ach type*/
    cmd = DRV_IOW(OamParserEtherCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_parser_ether_ctl));

    /*ipe lookup*/
    sal_memset(&ipe_lookup_ctl, 0, sizeof(IpeLookupCtl_m));
    cmd = DRV_IOR(IpeLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_lookup_ctl));
    SetIpeLookupCtl(V, mplsSectionOamUsePort_f, &ipe_lookup_ctl, ((g_oam_master[lchip]->tp_section_oam_based_l3if ? 0 : 1)));
    SetIpeLookupCtl(V, bfdSingleHopTtl_f, &ipe_lookup_ctl, 255);

     /* vccv_bfd_ttl_chk_en*/
    SetIpeLookupCtl(V, vccvBfdTtlChkEn_f, &ipe_lookup_ctl, 0);
     /* mpls_bfd_ttl_chk_en*/
    SetIpeLookupCtl(V, mplsBfdTtlChkEn_f, &ipe_lookup_ctl, 0);
    SetIpeLookupCtl(V, ipBfdTtlChkEn_f, &ipe_lookup_ctl, 0);
    SetIpeLookupCtl(V, microBfdDestPort_f, &ipe_lookup_ctl, 6784);    /*support micro-bfd*/
    cmd = DRV_IOW(IpeLookupCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_lookup_ctl));

    sal_memset(&ipe_mpls_ctl, 0, sizeof(IpeMplsCtl_m));
    cmd = DRV_IOR(IpeMplsCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_mpls_ctl));
    SetIpeMplsCtl(V, oamAlertLabel1_f, &ipe_mpls_ctl, 13);
    SetIpeMplsCtl(V, galSbitCheckEn_f, &ipe_mpls_ctl, 1);
    SetIpeMplsCtl(V, galTtlCheckEn_f, &ipe_mpls_ctl, 1);

    SetIpeMplsCtl(V, ttlOneOamEn_f, &ipe_mpls_ctl, 1); /*D2 spec change for MPLS BFD */
    SetIpeMplsCtl(V, ttlOneOamChkSbit_f, &ipe_mpls_ctl, 1);

    cmd = DRV_IOW(IpeMplsCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_mpls_ctl));

    sal_memset(&epe_acl_qos_ctl, 0, sizeof(EpeAclQosCtl_m));
    cmd = DRV_IOR(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_qos_ctl));
    SetEpeAclQosCtl(V, mplsSectionOamUsePort_f, &epe_acl_qos_ctl, (g_oam_master[lchip]->tp_section_oam_based_l3if ? 0 : 1));

    cmd = DRV_IOW(EpeAclQosCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_acl_qos_ctl));
    field = 1;
    cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_microBfdRouterMacEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));
    /*ipe/epe oam*/
    /*OAM*/
    sal_memset(&oam_rx_proc_ether_ctl, 0, sizeof(OamRxProcEtherCtl_m));
    cmd = DRV_IOR(OamRxProcEtherCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_rx_proc_ether_ctl));

    SetOamRxProcEtherCtl(V, portbasedSectionOam_f, &oam_rx_proc_ether_ctl, (g_oam_master[lchip]->tp_section_oam_based_l3if ? 0 : 1));
    SetOamRxProcEtherCtl(V, tpCsfToCpu_f, &oam_rx_proc_ether_ctl, 0);
    SetOamRxProcEtherCtl(V, tpCvToCpu_f, &oam_rx_proc_ether_ctl, 0);
    SetOamRxProcEtherCtl(V, tpCsfClear_f, &oam_rx_proc_ether_ctl, p_oam_glb->tp_csf_clear);
    SetOamRxProcEtherCtl(V, tpCsfRdi_f, &oam_rx_proc_ether_ctl, p_oam_glb->tp_csf_rdi);
    SetOamRxProcEtherCtl(V, tpCsfFdi_f, &oam_rx_proc_ether_ctl, p_oam_glb->tp_csf_fdi);
    SetOamRxProcEtherCtl(V, tpCsfLos_f, &oam_rx_proc_ether_ctl, p_oam_glb->tp_csf_los);
    SetOamRxProcEtherCtl(V, dMisConWhileCfg_f, &oam_rx_proc_ether_ctl, 14);
    SetOamRxProcEtherCtl(V, maxBfdInterval_f, &oam_rx_proc_ether_ctl, 0xFFFFFFFF);
    SetOamRxProcEtherCtl(V, mplstpdlmOutbandtoCpu_f, &oam_rx_proc_ether_ctl, 1);
    SetOamRxProcEtherCtl(V, updateRxDisc_f, &oam_rx_proc_ether_ctl, 0);
    SetOamRxProcEtherCtl(V, timerAutoNegotiation_f, &oam_rx_proc_ether_ctl, 1);
    SetOamRxProcEtherCtl(V, sbfdUseMplsEncap_f, &oam_rx_proc_ether_ctl, 1);
    SetOamRxProcEtherCtl(V, bfdUpRcvFbitSetPbit_f, &oam_rx_proc_ether_ctl, 1);
    SetOamRxProcEtherCtl(V, microBfdLearnEn_f, &oam_rx_proc_ether_ctl, 1);
    /*GG RTL bug, D2 need check EXCEPTION_BFD_DISC_MISMATCH*/
    SetOamRxProcEtherCtl(V, localDiscMismatchDiscard_f, &oam_rx_proc_ether_ctl, 1);
    SetOamRxProcEtherCtl(V, remoteDiscMismatchDiscard_f, &oam_rx_proc_ether_ctl, 1);
    SetOamRxProcEtherCtl(V, ethCsfRxIntervalChkEn_f, &oam_rx_proc_ether_ctl, 1);
    SetOamRxProcEtherCtl(V, tpCsfRxIntervalChkEn_f, &oam_rx_proc_ether_ctl, 1);
    SetOamRxProcEtherCtl(V, txIntervalJitteredMode_f, &oam_rx_proc_ether_ctl, 1);
    cmd = DRV_IOW(OamRxProcEtherCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_rx_proc_ether_ctl));

    field = 4;
    cmd = DRV_IOW(OamUpdateCtl_t, OamUpdateCtl_cvWhileCfg_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field));

    sal_memset(&oam_ether_tx_ctl, 0, sizeof(OamEtherTxCtl_m));
    cmd = DRV_IOR(OamEtherTxCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_ether_tx_ctl));
    SetOamEtherTxCtl(V, achHeaderL_f, &oam_ether_tx_ctl, 0x1000);
    SetOamEtherTxCtl(V, bfdMultiHopUdpDstPort_f, &oam_ether_tx_ctl, 4784);
    SetOamEtherTxCtl(V, bfdUdpDstPort_f, &oam_ether_tx_ctl, 3784);
    SetOamEtherTxCtl(V, ccIntervalUse33ms_f, &oam_ether_tx_ctl, 1);
    SetOamEtherTxCtl(V, microBfdUdpPort_f, &oam_ether_tx_ctl, 6784);  /*support micro-bfd*/
    if (!DRV_IS_DUET2(lchip))
    {
        SetOamEtherTxCtl(V, microBfdTxDiffMacEn_f, &oam_ether_tx_ctl, 1);
    }
    cmd = DRV_IOW(OamEtherTxCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_ether_tx_ctl));

    sal_memset(&oam_header_edit_ctl, 0, sizeof(OamHeaderEditCtl_m));
    cmd = DRV_IOR(OamHeaderEditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_header_edit_ctl));
   /*ZHAOMC  SetOamHeaderEditCtl(V, mplsdlmTlvChk_f, &oam_header_edit_ctl, 1);*/
    SetOamHeaderEditCtl(V, mplsdlmVerChk_f, &oam_header_edit_ctl, 1);
    SetOamHeaderEditCtl(V, mplsdlmQueryCodeChk_f, &oam_header_edit_ctl, 1);
    SetOamHeaderEditCtl(V, mplsdlmQueryCodeMaxValue_f, &oam_header_edit_ctl, 2);
    SetOamHeaderEditCtl(V, mplsdlmDflagsChk_f, &oam_header_edit_ctl, 1);
    SetOamHeaderEditCtl(V, sbfdL4ChecksumMode_f, &oam_header_edit_ctl, 1);
    SetOamHeaderEditCtl(V, sbfdL4SrcportMode_f, &oam_header_edit_ctl, 0);
    SetOamHeaderEditCtl(V, twampUpdChksumEn_f, &oam_header_edit_ctl, 1);
    cmd = DRV_IOW(OamHeaderEditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &oam_header_edit_ctl));

    field = 15;
    cmd = DRV_IOW(DsOamExcp_t, DsOamExcp__priority_f);
    index = SYS_OAM_EXCP_BFD_TIMER_NEG;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &field));

    cmd = DRV_IOR(EpeL2EditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_l2_edit));
    SetEpeL2EditCtl(A, microBfdMac_f, &epe_l2_edit,  hw_mac);
    cmd = DRV_IOW(EpeL2EditCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_l2_edit));

     /* config slow interval*/
    sal_memset(&ds_bfd_slow_interval_cfg, 0, sizeof(DsBfdSlowIntervalCfg_m));
#if (SDK_WORK_PLATFORM == 0)
    {
        SetDsBfdSlowIntervalCfg(V, txSlowInterval_f, &ds_bfd_slow_interval_cfg, 1000);
        SetDsBfdSlowIntervalCfg(V, rxSlowInterval_f, &ds_bfd_slow_interval_cfg, 1000);
    }
#else
    SetDsBfdSlowIntervalCfg(V, txSlowInterval_f, &ds_bfd_slow_interval_cfg, 4);
    SetDsBfdSlowIntervalCfg(V, rxSlowInterval_f, &ds_bfd_slow_interval_cfg, 4);
#endif
    cmd = DRV_IOW(DsBfdSlowIntervalCfg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds_bfd_slow_interval_cfg));

     /* config BfdIntervalCam*/
    cmd = DRV_IOW(DsBfdIntervalCam_t, DRV_ENTRY_FLAG);
    for (i = 0; i < BFD_INTERVAL_CAM_NUM ; i++)
    {
        sal_memset(&ds_bfd_interval_cam, 0, sizeof(DsBfdIntervalCam_m));
        SetDsBfdIntervalCam(V, is33ms_f, &ds_bfd_interval_cam, 0);
        SetDsBfdIntervalCam(V, rxInterval_f, &ds_bfd_interval_cam, bfd_support_interval[i]);
        SetDsBfdIntervalCam(V, txInterval_f, &ds_bfd_interval_cam, bfd_support_interval[i]);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, i, cmd, &ds_bfd_interval_cam));
    }

    sal_memset(&ds_bfd_interval_cam, 0, sizeof(DsBfdIntervalCam_m));
    SetDsBfdIntervalCam(V, is33ms_f, &ds_bfd_interval_cam, 1);
    SetDsBfdIntervalCam(V, rxInterval_f, &ds_bfd_interval_cam, 3);
    SetDsBfdIntervalCam(V, txInterval_f, &ds_bfd_interval_cam, 3);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, BFD_INTERVAL_CAM_NUM-1, cmd, &ds_bfd_interval_cam));

    return CTC_E_NONE;
}

#endif

