#if (FEATURE_MODE == 0)
/**
 @file sys_usw_fcoe.c

 @date 2015-10-14

 @version v2.0

 The file contains all fcoe related function
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/

#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_fcoe.h"
#include "ctc_parser.h"
#include "ctc_debug.h"

#include "sys_usw_chip.h"
#include "sys_usw_dma.h"
#include "sys_usw_ftm.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_l2_fdb.h"
#include "sys_usw_fcoe.h"

#include "drv_api.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
#define SYS_FCOE_DISCARD_FWD_PTR    0


/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
sys_fcoe_master_t* p_usw_fcoe_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};


/****************************************************************************
 *
* Function
*
*****************************************************************************/


STATIC int32
_sys_usw_fcoe_write_da(uint8 lchip, uint32 index, ctc_fcoe_route_t* p_fcoe_route)
{
    uint32 cmd = 0;
    uint32 field_value = 0;
    uint32 fwd_offset = 0;
    sys_nh_info_dsnh_t nh_info;
    DsFcoeDa_m dsfcoeda;

    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ad index:0x%x\n", index);

    CTC_PTR_VALID_CHECK(p_fcoe_route);

    sal_memset(&dsfcoeda, 0, sizeof(dsfcoeda));
    sal_memset(&nh_info, 0, sizeof(sys_nh_info_dsnh_t));

    if (CTC_FLAG_ISSET(p_fcoe_route->flag, CTC_FCOE_FLAG_DST_DISCARD))
    {
        SetDsFcoeDa(V, dsFwdPtr_f, &dsfcoeda, SYS_FCOE_DISCARD_FWD_PTR);
        SetDsFcoeDa(V, nextHopPtrValid_f, &dsfcoeda, 0);
    }
    else
    {

        CTC_ERROR_RETURN(sys_usw_nh_get_nhinfo(lchip, p_fcoe_route->nh_id, &nh_info, 0));

        /* Not merge dsfwd */
        if (nh_info.ecmp_valid)
        {
            SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Ecmp number is %d\r\n", nh_info.ecmp_cnt);
            field_value = nh_info.ecmp_group_id;
            SetDsFcoeDa(V, ecmpGroupId_f, &dsfcoeda, field_value);
            SetDsFcoeDa(V, ecmpEn_f, &dsfcoeda, 1);
            SetDsFcoeDa(V, nextHopPtrValid_f, &dsfcoeda, 0);
        }
        else
        {

            if(nh_info.merge_dsfwd == 1 )
            {
                SetDsFcoeDa(V, adDestMap_f, &dsfcoeda, nh_info.dest_map);
                SetDsFcoeDa(V, adApsBridgeEn_f, &dsfcoeda, nh_info.aps_en);
                SetDsFcoeDa(V, nextHopPtrValid_f, &dsfcoeda, 1);
                SetDsFcoeDa(V, adNextHopExt_f, &dsfcoeda, nh_info.nexthop_ext);
                SetDsFcoeDa(V, adNextHopPtr_f, &dsfcoeda, nh_info.dsnh_offset);
            }
            else
            {
                CTC_ERROR_RETURN(sys_usw_nh_get_dsfwd_offset(lchip, p_fcoe_route->nh_id, &fwd_offset, 0));
                field_value = fwd_offset & 0xFFFF;
                SetDsFcoeDa(V, dsFwdPtr_f, &dsfcoeda, field_value);
                SetDsFcoeDa(V, nextHopPtrValid_f, &dsfcoeda, 0);
            }

            SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "DsFcoeDa.ds_fwd_ptr:0x%x\n", field_value);
        }
    }

    cmd = DRV_IOW(DsFcoeDa_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsfcoeda));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_fcoe_write_hash_key(uint8 lchip, uint32 ad_index, ctc_fcoe_route_t* p_fcoe_route, bool is_rpf, bool is_remove)
{
    drv_acc_in_t fib_acc_in;
    drv_acc_out_t fib_acc_out;
    drv_acc_in_t cpu_acc_in;
    drv_acc_out_t cpu_acc_out;
    DsFibHost0FcoeHashKey_m fcoehashkey;
    DsFibHost1FcoeRpfHashKey_m rpfhashkey;

    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "ad_index = 0x%x\n", ad_index);
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "is_rpf = 0x%x\n", is_rpf);
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "is_remove = 0x%x\n", is_remove);

    CTC_PTR_VALID_CHECK(p_fcoe_route);

    sal_memset(&fib_acc_in, 0, sizeof(fib_acc_in));
    sal_memset(&fib_acc_out, 0, sizeof(fib_acc_out));
    sal_memset(&cpu_acc_in, 0, sizeof(cpu_acc_in));
    sal_memset(&cpu_acc_out, 0, sizeof(cpu_acc_out));
    sal_memset(&fcoehashkey, 0, sizeof(fcoehashkey));
    sal_memset(&rpfhashkey, 0, sizeof(rpfhashkey));

    if (FALSE == is_rpf)
    {
        SetDsFibHost0FcoeHashKey(V, fcoeDid_f, &fcoehashkey, p_fcoe_route->fcid);
        SetDsFibHost0FcoeHashKey(V, vsiId_f, &fcoehashkey, p_fcoe_route->fid);
        SetDsFibHost0FcoeHashKey(V, valid_f, &fcoehashkey, !is_remove);
        SetDsFibHost0FcoeHashKey(V, dsAdIndex_f, &fcoehashkey, ad_index & 0x3ffff);
        SetDsFibHost0FcoeHashKey(V, hashType_f, &fcoehashkey, FIBHOST0PRIMARYHASHTYPE_FCOE);

        fib_acc_in.data = (void*)(&fcoehashkey);
        fib_acc_in.type = DRV_ACC_TYPE_ADD;
        fib_acc_in.tbl_id = DsFibHost0FcoeHashKey_t;
        fib_acc_in.op_type = DRV_ACC_OP_BY_KEY;
        CTC_BIT_SET(fib_acc_in.flag, DRV_ACC_OVERWRITE_EN);
        CTC_ERROR_RETURN(drv_acc_api(lchip, &fib_acc_in, &fib_acc_out));


        SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "key_index = 0x%x\n", fib_acc_out.key_index);

        if (fib_acc_out.is_conflict)
        {
            SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Hash Conflict \n");
            return CTC_E_HASH_CONFLICT;
        }
    }
    else
    {
        SetDsFibHost1FcoeRpfHashKey(V, fcoeSid_f, &rpfhashkey, p_fcoe_route->fcid);
        SetDsFibHost1FcoeRpfHashKey(V, vsiId_f, &rpfhashkey, p_fcoe_route->fid);
        SetDsFibHost1FcoeRpfHashKey(V, valid_f, &rpfhashkey, 1);
        SetDsFibHost1FcoeRpfHashKey(V, dsAdIndex_f, &rpfhashkey, ad_index & 0x3ffff);
        SetDsFibHost1FcoeRpfHashKey(V, hashType_f, &rpfhashkey, FIBHOST1PRIMARYHASHTYPE_OTHER);
        SetDsFibHost1FcoeRpfHashKey(V, _type_f, &rpfhashkey, 0);

        cpu_acc_in.data = (void*)(&rpfhashkey);
        cpu_acc_in.tbl_id = DsFibHost1FcoeRpfHashKey_t;
        cpu_acc_in.type = is_remove?DRV_ACC_TYPE_DEL:DRV_ACC_TYPE_ADD;
        cpu_acc_in.op_type = DRV_ACC_OP_BY_KEY;

        CTC_ERROR_RETURN(drv_acc_api(lchip, &cpu_acc_in, &cpu_acc_out));

        SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "key_index = 0x%x\n", cpu_acc_out.key_index);

        if (cpu_acc_out.is_conflict)
        {
            SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Hash Conflict \n");
            return CTC_E_HASH_CONFLICT;
        }
    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_fcoe_write_sa(uint8 lchip, uint32 index, ctc_fcoe_route_t* p_fcoe_route)
{
    uint32 cmd = 0;
    uint32 field_value = 0;
    hw_mac_addr_t hw_mac = { 0 };
    DsFcoeSa_m dsfcoesa;

    CTC_PTR_VALID_CHECK(p_fcoe_route);

    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sal_memset(&dsfcoesa, 0, sizeof(dsfcoesa));

    field_value = CTC_FLAG_ISSET(p_fcoe_route->flag, CTC_FCOE_FLAG_RPF_CHECK);
    SetDsFcoeSa(V, rpfCheckEn_f, &dsfcoesa, field_value);
    SetDsFcoeSa(V, globalSrcPort_f, &dsfcoesa, SYS_MAP_CTC_GPORT_TO_DRV_GPORT(p_fcoe_route->src_gport));

    field_value = CTC_FLAG_ISSET(p_fcoe_route->flag, CTC_FCOE_FLAG_MACSA_CHECK);
    SetDsFcoeSa(V, macSaCheckEn_f, &dsfcoesa, field_value);
    SYS_USW_SET_HW_MAC(hw_mac, p_fcoe_route->mac_sa);
    SetDsFcoeSa(A, macSa_f, &dsfcoesa, hw_mac);

    field_value = CTC_FLAG_ISSET(p_fcoe_route->flag, CTC_FCOE_FLAG_SRC_DISCARD);
    SetDsFcoeSa(V, srcDiscard_f, &dsfcoesa, field_value);

    cmd = DRV_IOW(DsFcoeSa_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &dsfcoesa));

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_fcoe_lookup_hash_key(uint8 lchip, uint32* ad_index, ctc_fcoe_route_t* p_fcoe_route, bool is_rpf)
{
    drv_acc_in_t fib_acc_in;
    drv_acc_out_t fib_acc_out;
    drv_acc_in_t cpu_acc_in;
    drv_acc_out_t cpu_acc_out;
    DsFibHost0FcoeHashKey_m fcoehashkey;
    DsFibHost1FcoeRpfHashKey_m rpfhashkey;

    CTC_PTR_VALID_CHECK(ad_index);
    CTC_PTR_VALID_CHECK(p_fcoe_route);

    sal_memset(&fib_acc_in, 0, sizeof(fib_acc_in));
    sal_memset(&fib_acc_out, 0, sizeof(fib_acc_out));
    sal_memset(&cpu_acc_in, 0, sizeof(cpu_acc_in));
    sal_memset(&cpu_acc_out, 0, sizeof(cpu_acc_out));
    sal_memset(&fcoehashkey, 0, sizeof(fcoehashkey));
    sal_memset(&rpfhashkey, 0, sizeof(rpfhashkey));

    if (FALSE == is_rpf)
    {
        SetDsFibHost0FcoeHashKey(V, fcoeDid_f, &fcoehashkey, p_fcoe_route->fcid);
        SetDsFibHost0FcoeHashKey(V, vsiId_f, &fcoehashkey, p_fcoe_route->fid);
        SetDsFibHost0FcoeHashKey(V, valid_f, &fcoehashkey, 1);
        SetDsFibHost0FcoeHashKey(V, hashType_f, &fcoehashkey, FIBHOST0PRIMARYHASHTYPE_FCOE);

        fib_acc_in.type = DRV_ACC_TYPE_LOOKUP;
        fib_acc_in.tbl_id = DsFibHost0FcoeHashKey_t;
        fib_acc_in.op_type = DRV_ACC_OP_BY_KEY;
        fib_acc_in.data = (void*)&fcoehashkey;

        CTC_ERROR_RETURN(drv_acc_api(lchip, &fib_acc_in, &fib_acc_out));

        if (fib_acc_out.is_conflict)
        {
            SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Hash Conflict \n");
            return CTC_E_HASH_CONFLICT;
        }
        else if (!fib_acc_out.is_hit)
        {
            SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
            return CTC_E_NOT_EXIST;
        }
        else
        {
            *ad_index = fib_acc_out.ad_index;
        }
    }
    else
    {
        SetDsFibHost1FcoeRpfHashKey(V, fcoeSid_f, &rpfhashkey, p_fcoe_route->fcid);
        SetDsFibHost1FcoeRpfHashKey(V, vsiId_f, &rpfhashkey, p_fcoe_route->fid);
        SetDsFibHost1FcoeRpfHashKey(V, valid_f, &rpfhashkey, 1);
        SetDsFibHost1FcoeRpfHashKey(V, hashType_f, &rpfhashkey, FIBHOST1PRIMARYHASHTYPE_OTHER);
        SetDsFibHost1FcoeRpfHashKey(V, _type_f, &rpfhashkey, 0);

        cpu_acc_in.type =  DRV_ACC_TYPE_LOOKUP;
        cpu_acc_in.op_type = DRV_ACC_OP_BY_KEY;
        cpu_acc_in.tbl_id = DsFibHost1FcoeRpfHashKey_t;
        cpu_acc_in.data = (void*)(&rpfhashkey);
        CTC_ERROR_RETURN(drv_acc_api(lchip, &cpu_acc_in, &cpu_acc_out));

        if (cpu_acc_out.is_conflict)
        {
            SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Hash Conflict \n");
            return CTC_E_HASH_CONFLICT;
        }
        else if (!cpu_acc_out.is_hit)
        {
            SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry not exist \n");
            return CTC_E_NOT_EXIST;
        }
        else
        {
            cpu_acc_in.index = cpu_acc_out.key_index;
            cpu_acc_in.type = DRV_ACC_TYPE_LOOKUP;
            cpu_acc_in.op_type = DRV_ACC_OP_BY_INDEX;
            cpu_acc_in.tbl_id = DsFibHost1FcoeRpfHashKey_t;
            cpu_acc_in.data = (void*)(&rpfhashkey);

            sal_memset(&cpu_acc_out, 0, sizeof(cpu_acc_out));
            CTC_ERROR_RETURN(drv_acc_api(lchip, &cpu_acc_in, &cpu_acc_out));
            *ad_index = GetDsFibHost1FcoeRpfHashKey(V, dsAdIndex_f, &cpu_acc_out.data);
        }

    }
    return CTC_E_NONE;
}


int32
sys_usw_fcoe_add_route(uint8 lchip, ctc_fcoe_route_t* p_fcoe_route)
{
    uint32 ad_index1 = 0;
    uint32 ad_index2 = 0;
    int32 ret = 0;
    int32 ret1 = 0;
    int32 ret2 = 0;

    SYS_FCOE_INIT_CHECK;
    CTC_PTR_VALID_CHECK(p_fcoe_route);
    SYS_L2_FID_CHECK(p_fcoe_route->fid);
    CTC_MAX_VALUE_CHECK(p_fcoe_route->fcid, 0xffffff);

    if (CTC_FLAG_ISSET(p_fcoe_route->flag, CTC_FCOE_FLAG_RPF_CHECK))
    {
        SYS_GLOBAL_PORT_CHECK(p_fcoe_route->src_gport);
        if (!CTC_IS_LINKAGG_PORT(p_fcoe_route->src_gport))
        {
            ret = sys_usw_chip_is_local(lchip, SYS_MAP_CTC_GPORT_TO_GCHIP(p_fcoe_route->src_gport));
            if (FALSE == ret)
            {
                SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Chip is not local! \n");
                return CTC_E_INVALID_CHIP_ID;
            }
        }
    }

    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Flag = 0x%x\n", p_fcoe_route->flag);
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Fcid = 0x%x\n", p_fcoe_route->fcid);
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Fid = 0x%x\n", p_fcoe_route->fid);
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Nhid = 0x%x\n", p_fcoe_route->nh_id);

    if (p_fcoe_route->fcid)
    {
        /* add da */
        ret1 = _sys_usw_fcoe_lookup_hash_key(lchip, &ad_index1, p_fcoe_route, FALSE);
        if (ret1 == CTC_E_HASH_CONFLICT)
        {
            return ret1;
        }
        else
        {
            if (ret1 == CTC_E_NOT_EXIST)
            {
                CTC_ERROR_RETURN(sys_usw_ftm_alloc_table_offset(lchip, DsFcoeDa_t, 0, 1, 1, &ad_index1));
                CTC_ERROR_GOTO(_sys_usw_fcoe_write_hash_key(lchip, ad_index1, p_fcoe_route, FALSE, FALSE), ret, ERROR_RETURN1);
            }

            /* update ad when entry exist */
            CTC_ERROR_GOTO(_sys_usw_fcoe_write_da(lchip, ad_index1, p_fcoe_route), ret, ERROR_RETURN1);
        }


        /* add sa */
        ret2 = _sys_usw_fcoe_lookup_hash_key(lchip, &ad_index2, p_fcoe_route, TRUE);
        if (ret2 == CTC_E_HASH_CONFLICT)
        {
            ret = ret2;
            goto ERROR_RETURN1;
        }
        else
        {
            if (ret2 == CTC_E_NOT_EXIST)
            {
                CTC_ERROR_GOTO(sys_usw_ftm_alloc_table_offset(lchip, DsFcoeDa_t, 0, 1, 1, &ad_index2), ret, ERROR_RETURN1);
                CTC_ERROR_GOTO(_sys_usw_fcoe_write_hash_key(lchip, ad_index2, p_fcoe_route, TRUE, FALSE), ret, ERROR_RETURN2);
            }

            /* update ad when entry exist */
            CTC_ERROR_GOTO(_sys_usw_fcoe_write_sa(lchip, ad_index2, p_fcoe_route), ret, ERROR_RETURN2);
        }
    }
    else
    {
        /* default da */
        ad_index1 = p_usw_fcoe_master[lchip]->default_base + p_fcoe_route->fid;
        CTC_ERROR_RETURN(_sys_usw_fcoe_write_da(lchip, ad_index1, p_fcoe_route));

        /* default sa */
        ad_index2 = p_usw_fcoe_master[lchip]->rpf_default_base + p_fcoe_route->fid;
        CTC_ERROR_RETURN(_sys_usw_fcoe_write_sa(lchip, ad_index2, p_fcoe_route));
    }

    return CTC_E_NONE;


ERROR_RETURN2:
    if (ret2 == CTC_E_NOT_EXIST)
    {
        sys_usw_ftm_free_table_offset(lchip, DsFcoeDa_t, 0, 1, ad_index2);
        _sys_usw_fcoe_write_hash_key(lchip, ad_index2, p_fcoe_route, TRUE, TRUE);
    }

ERROR_RETURN1:
    if (ret1 == CTC_E_NOT_EXIST)
    {
        sys_usw_ftm_free_table_offset(lchip, DsFcoeDa_t, 0, 1, ad_index1);
        _sys_usw_fcoe_write_hash_key(lchip, ad_index1, p_fcoe_route, FALSE, TRUE);
    }
    return ret;
}

int32
sys_usw_fcoe_remove_route(uint8 lchip, ctc_fcoe_route_t* p_fcoe_route)
{
    uint32 ad_index = 0;
    ctc_fcoe_route_t fcoe_route;

    SYS_FCOE_INIT_CHECK;
    CTC_PTR_VALID_CHECK(p_fcoe_route);
    SYS_L2_FID_CHECK(p_fcoe_route->fid);
    CTC_MAX_VALUE_CHECK(p_fcoe_route->fcid, 0xffffff);

    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Fcid = 0x%x\n", p_fcoe_route->fcid);
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "Fid = 0x%x\n", p_fcoe_route->fid);


    if (p_fcoe_route->fcid)
    {
        /* remove da */
        CTC_ERROR_RETURN(_sys_usw_fcoe_lookup_hash_key(lchip, &ad_index, p_fcoe_route, FALSE));
        CTC_ERROR_RETURN(_sys_usw_fcoe_write_hash_key(lchip, ad_index, p_fcoe_route, FALSE, TRUE));
        CTC_ERROR_RETURN(sys_usw_ftm_free_table_offset(lchip, DsFcoeDa_t, 0, 1, ad_index));

        /* remove sa */
        CTC_ERROR_RETURN(_sys_usw_fcoe_lookup_hash_key(lchip, &ad_index, p_fcoe_route, TRUE));
        CTC_ERROR_RETURN(_sys_usw_fcoe_write_hash_key(lchip, ad_index, p_fcoe_route, TRUE, TRUE));
        CTC_ERROR_RETURN(sys_usw_ftm_free_table_offset(lchip, DsFcoeDa_t, 0, 1, ad_index));
    }
    else
    {
        /* default route discard */
        ad_index = p_usw_fcoe_master[lchip]->default_base + p_fcoe_route->fid;
        p_fcoe_route->flag = CTC_FCOE_FLAG_DST_DISCARD;
        CTC_ERROR_RETURN(_sys_usw_fcoe_write_da(lchip, ad_index, p_fcoe_route));

        /* default sa do nothing */
        sal_memset(&fcoe_route, 0, sizeof(fcoe_route));
        ad_index = p_usw_fcoe_master[lchip]->rpf_default_base + p_fcoe_route->fid;
        CTC_ERROR_RETURN(_sys_usw_fcoe_write_sa(lchip, ad_index, &fcoe_route));
    }

    return CTC_E_NONE;
}

#define DS_FIB_HOST0_FCOE_HASH_KEY_BYTES                                 12
#define DS_FIB_HOST1_FCOE_RPF_HASH_KEY_BYTES                                 12
int32
sys_usw_fcoe_rpf_traverse(uint8 lchip, sys_fcoe_traverse_fn fn, sys_fcoe_traverse_t* p_data)
{
    int32 ret = 0;
    uint32 index = 0;
    uint32 count = 0;
    sys_dma_tbl_rw_t dma_rw;
    uint32 cfg_addr = 0;
    uint32* p_buffer = NULL;
    void* p_key = NULL;
    uint32 hash_type = 0;
    uint8 valid = 0;
    ctc_fcoe_route_t fcoe_data;
    uint32 base_entry = 0;
    uint32 entry_cnt = 0;
    uint32 next_index = 0;
    uint32 entry_index = 0;
    drv_work_platform_type_t platform_type;
    uint32 max_entry_num = 0;

    SYS_FCOE_INIT_CHECK;
    CTC_PTR_VALID_CHECK(fn);
    CTC_PTR_VALID_CHECK(p_data);

    /* get 85bits bits entry num */
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsFibHost1FcoeRpfHashKey_t, &max_entry_num));
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "fcoe_max_entry_num = %d\n", max_entry_num);
    if (p_data->entry_num > max_entry_num)
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&fcoe_data, 0, sizeof(ctc_fcoe_route_t));
    sal_memset(&dma_rw, 0, sizeof(ctc_dma_tbl_rw_t));


    p_buffer = (uint32*)mem_malloc(MEM_FCOE_MODULE, 400*DS_FIB_HOST1_FCOE_RPF_HASH_KEY_BYTES);
    if (NULL == p_buffer)
    {
        SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }

    base_entry = p_data->start_index;

    do
    {
        if (base_entry >= max_entry_num)
        {
            p_data->is_end = 1;
            mem_free(p_buffer);
            return CTC_E_NONE;
        }

        entry_cnt = ((base_entry+400) <= max_entry_num) ? 400 : (max_entry_num-base_entry);
        drv_get_platform_type(lchip, &platform_type);
        if (platform_type == HW_PLATFORM)
        {
            CTC_ERROR_GOTO(drv_get_table_property(lchip, DRV_TABLE_PROP_HW_ADDR, DsFibHost1FcoeRpfHashKey_t, base_entry, &cfg_addr), ret, error);
        }
        else
        {
            /*special process for uml*/
            cfg_addr = (DsFibHost1FcoeRpfHashKey_t << 18)|base_entry;
        }


        /*Using Dma to read data, using 80bits entry read*/
        dma_rw.rflag = 1;
        dma_rw.tbl_addr = cfg_addr;
        dma_rw.entry_num = entry_cnt;
        dma_rw.entry_len = DS_FIB_HOST1_FCOE_RPF_HASH_KEY_BYTES;
        dma_rw.buffer = p_buffer;

        CTC_ERROR_GOTO(sys_usw_dma_rw_table(lchip, &dma_rw), ret, error);

        for (index = 0; index < entry_cnt; index++)
        {
            entry_index = base_entry + index;
            p_key = ((DsFibHost1FcoeRpfHashKey_m*)p_buffer + index);
            valid = GetDsFibHost1FcoeRpfHashKey(V, valid_f, p_key);
            hash_type = GetDsFibHost1FcoeRpfHashKey(V, hashType_f, p_key);
            if ((!valid)||(hash_type != FIBHOST1PRIMARYHASHTYPE_OTHER))
            {
                if (entry_index > max_entry_num)
                {
                    p_data->is_end = 1;
                    goto end_pro;
                }
                else
                {
                    continue;
                }
            }

            sal_memset(&fcoe_data, 0, sizeof(ctc_fcoe_route_t));

            fcoe_data.fcid = GetDsFibHost1FcoeRpfHashKey(V, fcoeSid_f, p_key);
            fcoe_data.fid = GetDsFibHost1FcoeRpfHashKey(V, vsiId_f, p_key);
            fcoe_data.nh_id = GetDsFibHost1FcoeRpfHashKey(V, dsAdIndex_f, p_key); /* nh_id used as ad index */

            fn(&fcoe_data, &entry_index);
            count++;

            if (entry_index > (max_entry_num-2))
            {
                p_data->is_end = 1;
                goto end_pro;
            }
            else if (count >= p_data->entry_num)
            {
                next_index = entry_index + 1;
                goto end_pro;
            }

        }

        if (p_data->is_end)
        {
            break;
        }
        else
        {
            base_entry += 400;
        }

    }while(count < p_data->entry_num);

end_pro:
    p_data->next_traverse_index = next_index;

error:
    mem_free(p_buffer);

    return ret;
}

int32
sys_usw_fcoe_show_rpf_entry_info(ctc_fcoe_route_t* p_info, void* user_data)
{
    uint32 entry_index = 0;


    if(user_data != NULL)
    {
        entry_index = *(uint32*)user_data + 32; /* add cam index */
    }

    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-9x", entry_index);
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-9x", p_info->fcid);
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7d", p_info->fid);
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-8x\n", p_info->nh_id);

    return CTC_E_NONE;
}


int32
sys_usw_fcoe_traverse(uint8 lchip, sys_fcoe_traverse_fn fn, sys_fcoe_traverse_t* p_data)
{
    int32 ret = 0;
    uint32 index = 0;
    uint32 count = 0;
    sys_dma_tbl_rw_t dma_rw;
    uint32 cfg_addr = 0;
    uint32* p_buffer = NULL;
    void* p_key = NULL;
    uint32 hash_type = 0;
    uint8 valid = 0;
    ctc_fcoe_route_t fcoe_data;
    uint32 base_entry = 0;
    uint32 entry_cnt = 0;
    uint32 next_index = 0;
    uint32 entry_index = 0;
    drv_work_platform_type_t platform_type;
    uint32 max_entry_num = 0;
    uint32 cmd1 = 0;
    uint32 cmd2 = 0;
    uint32 cmd3 = 0;
    uint32 cmd4 = 0;
    uint32 fwd_cmd = 0;
    uint32 fwd_ptr = 0;
    uint32 destmap = 0;
    uint16 lport = 0;
    uint8 gchip = 0;
    DsFwd_m dsfwd;
    DsFwdHalf_m dsfwd_half;

    SYS_FCOE_INIT_CHECK;
    CTC_PTR_VALID_CHECK(fn);
    CTC_PTR_VALID_CHECK(p_data);

    sal_memset(&dsfwd, 0, sizeof(DsFwd_m));
    sal_memset(&dsfwd_half, 0, sizeof(DsFwdHalf_m));

    /* get 85bits bits entry num */
    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsFibHost0FcoeHashKey_t, &max_entry_num));
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "fcoe_max_entry_num = %d\n", max_entry_num);
    if (p_data->entry_num > max_entry_num)
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&fcoe_data, 0, sizeof(ctc_fcoe_route_t));
    sal_memset(&dma_rw, 0, sizeof(ctc_dma_tbl_rw_t));


    p_buffer = (uint32*)mem_malloc(MEM_FCOE_MODULE, 400*DS_FIB_HOST0_FCOE_HASH_KEY_BYTES);
    if (NULL == p_buffer)
    {
        SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }

    base_entry = p_data->start_index;
    cmd1 = DRV_IOR(DsFcoeDa_t, DsFcoeDa_dsFwdPtr_f);
    cmd2 = DRV_IOR(DsFcoeDa_t, DsFcoeDa_ecmpEn_f);
    cmd3 = DRV_IOR(DsFcoeDa_t, DsFcoeDa_nextHopPtrValid_f);
    cmd4 = DRV_IOR(DsFcoeDa_t, DsFcoeDa_adDestMap_f);
    fwd_cmd = DRV_IOR(DsFwd_t, DRV_ENTRY_FLAG);

    do
    {
        if (base_entry >= max_entry_num)
        {
            p_data->is_end = 1;
            mem_free(p_buffer);
            return CTC_E_NONE;
        }

        entry_cnt = ((base_entry+400) <= max_entry_num) ? 400 : (max_entry_num-base_entry);
        drv_get_platform_type(lchip, &platform_type);
        if (platform_type == HW_PLATFORM)
        {
            CTC_ERROR_GOTO(drv_get_table_property(lchip, DRV_TABLE_PROP_HW_ADDR, DsFibHost0FcoeHashKey_t, base_entry, &cfg_addr), ret, error);
        }
        else
        {
            /*special process for uml*/
            cfg_addr = (DsFibHost0FcoeHashKey_t << 18)|base_entry;
        }


        /*Using Dma to read data, using 80bits entry read*/
        dma_rw.rflag = 1;
        dma_rw.tbl_addr = cfg_addr;
        dma_rw.entry_num = entry_cnt;
        dma_rw.entry_len = DS_FIB_HOST0_FCOE_HASH_KEY_BYTES;
        dma_rw.buffer = p_buffer;

        CTC_ERROR_GOTO(sys_usw_dma_rw_table(lchip, &dma_rw), ret, error);

        for (index = 0; index < entry_cnt; index++)
        {
            entry_index = base_entry + index;
            p_key = ((DsFibHost0FcoeHashKey_m*)p_buffer + index);
            valid = GetDsFibHost0FcoeHashKey(V, valid_f, p_key);
            hash_type = GetDsFibHost0FcoeHashKey(V, hashType_f, p_key);
            if ((!valid)||(hash_type != FIBHOST0PRIMARYHASHTYPE_FCOE))
            {
                if (entry_index > max_entry_num)
                {
                    p_data->is_end = 1;
                    goto end_pro;
                }
                else
                {
                    continue;
                }
            }

            sal_memset(&fcoe_data, 0, sizeof(ctc_fcoe_route_t));

            fcoe_data.fcid = GetDsFibHost0FcoeHashKey(V, fcoeDid_f, p_key);
            fcoe_data.fid = GetDsFibHost0FcoeHashKey(V, vsiId_f, p_key);
            fcoe_data.nh_id = GetDsFibHost0FcoeHashKey(V, dsAdIndex_f, p_key); /* nh_id used as ad index */

            CTC_ERROR_RETURN(DRV_IOCTL(lchip, fcoe_data.nh_id, cmd2, &fwd_ptr));
            if (fwd_ptr)
            {
                fcoe_data.flag = 1; /* destport is ecmp */
            }
            else
            {
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, fcoe_data.nh_id, cmd3, &fwd_ptr)); /*fwd_ptr used as nextHopPtrValid*/
                if(fwd_ptr)
                {
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, fcoe_data.nh_id, cmd4, &destmap));
                }
                else
                {
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, fcoe_data.nh_id, cmd1, &fwd_ptr));
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, (fwd_ptr >> 1), fwd_cmd, &dsfwd));

                    if (0 == GetDsFwd(V, isHalf_f, &dsfwd))
                    {
                        destmap = GetDsFwd(V, destMap_f, &dsfwd);
                    }
                    else
                    {
                        if (fwd_ptr % 2 == 0)
                        {
                            GetDsFwdDualHalf(A, g_0_dsFwdHalf_f,   &dsfwd,   &dsfwd_half);
                        }
                        else
                        {
                            GetDsFwdDualHalf(A, g_1_dsFwdHalf_f,   &dsfwd,   &dsfwd_half);
                        }

                        destmap = GetDsFwdHalf(V, destMap_f, &dsfwd_half);
                    }
                }

                if (CTC_IS_BIT_SET(destmap, 18))
                {
                    fcoe_data.flag = 1; /* destport is mcast */
                }
                else
                {
                    gchip = SYS_DECODE_DESTMAP_GCHIP(destmap);
                    lport = (destmap & 0x1FF);
                    fcoe_data.src_gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport); /* src_gport used as destport */
                }
            }
            fn(&fcoe_data, &entry_index);
            count++;

            if (entry_index > (max_entry_num-2))
            {
                p_data->is_end = 1;
                goto end_pro;
            }
            else if (count >= p_data->entry_num)
            {
                next_index = entry_index + 1;
                goto end_pro;
            }

        }

        if (p_data->is_end)
        {
            break;
        }
        else
        {
            base_entry += 400;
        }

    }while(count < p_data->entry_num);

end_pro:
    p_data->next_traverse_index = next_index;

error:
    mem_free(p_buffer);

    return ret;
}

int32
sys_usw_fcoe_show_entry_info(ctc_fcoe_route_t* p_info, void* user_data)
{
    uint32 entry_index = 0;


    if(user_data != NULL)
    {
        entry_index = *(uint32*)user_data + 32; /* add cam index */
    }

    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-9x", entry_index);
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-9x", p_info->fcid);
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7d", p_info->fid);

    if (p_info->flag == 1)
    {
        SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-10s", "-");
    }
    else
    {
        SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%04x    ", p_info->src_gport);
    }

    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "0x%-8x\n", p_info->nh_id);

    /*show entry detail info*/


    return CTC_E_NONE;
}

int32
sys_usw_fcoe_dump_entry(uint8 lchip, void* user_data)
{
    sys_fcoe_traverse_t fcoe_traverse;

    SYS_FCOE_INIT_CHECK;

    /*dump DsFibHost0FcoeHashKey*/
    sal_memset(&fcoe_traverse, 0, sizeof(sys_fcoe_traverse_t));

    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n[DsFibHost0FcoeHashKey]");
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n%-11s%-11s%-7s%-10s%-8s\n", "ENTRY_IDX", "FCID", "FID", "GPORT", "AD_IDX[DsFcoeDa]");
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-----------------------------------------------\n");


    while (!fcoe_traverse.is_end)
    {
        fcoe_traverse.entry_num = 100;
        fcoe_traverse.start_index = fcoe_traverse.next_traverse_index;
        CTC_ERROR_RETURN(sys_usw_fcoe_traverse(lchip, sys_usw_fcoe_show_entry_info, &fcoe_traverse));
    }

    /*dump DsFibHost1FcoeRpfHashKey*/
    sal_memset(&fcoe_traverse, 0, sizeof(sys_fcoe_traverse_t));

    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n[DsFibHost1FcoeRpfHashKey]");
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n%-11s%-11s%-7s%-8s\n", "ENTRY_IDX", "FCID", "FID", "AD_IDX[DsFcoeSa]");
    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-----------------------------------------------\n");

    while (!fcoe_traverse.is_end)
    {
        fcoe_traverse.entry_num = 100;
        fcoe_traverse.start_index = fcoe_traverse.next_traverse_index;
        CTC_ERROR_RETURN(sys_usw_fcoe_rpf_traverse(lchip, sys_usw_fcoe_show_rpf_entry_info, &fcoe_traverse));
    }


    return CTC_E_NONE;
}


int32
sys_usw_fcoe_init(uint8 lchip, void* fcoe_global_cfg)
{
    int32  ret = 0;
    uint32 entry_num = 0;
    uint32 cmd = 0;
    uint32 offset = 0;
    uint16 max_fid_num = 0;
    FibEngineLookupResultCtl_m fib_engine_lookup_result_ctl;

    SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    LCHIP_CHECK(lchip);
    /* check init */
    if (NULL != p_usw_fcoe_master[lchip])
    {
        return CTC_E_NONE;
    }


    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsFibHost0FcoeHashKey_t, &entry_num));
    if (!entry_num)
    {   /* no resources */
        return CTC_E_NONE;
    }

    p_usw_fcoe_master[lchip] = mem_malloc(MEM_FCOE_MODULE, sizeof(sys_fcoe_master_t));
    if (NULL == p_usw_fcoe_master[lchip])
    {
        SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;
    }

    sal_memset(p_usw_fcoe_master[lchip], 0, sizeof(sys_fcoe_master_t));


    /* init default entry base */
    if (pl2_usw_master[lchip])
    {
        max_fid_num = pl2_usw_master[lchip]->cfg_max_fid + 1;
    }
    else
    {
        max_fid_num = 5 * 1024;
    }

    ret = sys_usw_ftm_alloc_table_offset(lchip, DsFcoeDa_t, CTC_INGRESS, max_fid_num, 1, &offset);
    if (ret)
    {
        SYS_FCOE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No resource in ASIC \n");
        return CTC_E_NO_RESOURCE;
    }

    CTC_ERROR_RETURN(sys_usw_ftm_alloc_table_offset(lchip, DsFcoeDa_t, CTC_INGRESS, max_fid_num, 1, &entry_num));

    p_usw_fcoe_master[lchip]->default_base = offset;
    p_usw_fcoe_master[lchip]->rpf_default_base = entry_num;
    p_usw_fcoe_master[lchip]->max_fid_num = max_fid_num;

    sal_memset(&fib_engine_lookup_result_ctl, 0, sizeof(fib_engine_lookup_result_ctl));
    cmd = DRV_IOR(FibEngineLookupResultCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_engine_lookup_result_ctl));

    cmd = DRV_IOW(FibEngineLookupResultCtl_t, DRV_ENTRY_FLAG);

    /* da default entry base */
    SetFibEngineLookupResultCtl(V, gFcoeDaLookupResultCtl_defaultEntryPtr_f, &fib_engine_lookup_result_ctl, offset);
    SetFibEngineLookupResultCtl(V, gFcoeDaLookupResultCtl_defaultEntryEn_f, &fib_engine_lookup_result_ctl, TRUE);

    /* rpf sa default entry base */
    SetFibEngineLookupResultCtl(V, gFcoeSaLookupResultCtl_defaultEntryPtr_f, &fib_engine_lookup_result_ctl, entry_num);
    SetFibEngineLookupResultCtl(V, gFcoeSaLookupResultCtl_defaultEntryEn_f, &fib_engine_lookup_result_ctl, TRUE);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_engine_lookup_result_ctl));    

    /* default entry: da discard, sa none */

    return CTC_E_NONE;
}

int32
sys_usw_fcoe_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);
    if (NULL == p_usw_fcoe_master[lchip])
    {
        return CTC_E_NONE;
    }

    sys_usw_ftm_free_table_offset(lchip, DsFcoeDa_t, CTC_INGRESS, p_usw_fcoe_master[lchip]->max_fid_num, p_usw_fcoe_master[lchip]->default_base);
    sys_usw_ftm_free_table_offset(lchip, DsFcoeDa_t, CTC_INGRESS, p_usw_fcoe_master[lchip]->max_fid_num, p_usw_fcoe_master[lchip]->rpf_default_base);

    mem_free(p_usw_fcoe_master[lchip]);

    return CTC_E_NONE;
}

#endif
