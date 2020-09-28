/**
 @file sys_usw_flow_stats.c

 @date 2009-12-22

 @version v2.0

*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_error.h"
#include "ctc_macro.h"
#include "ctc_stats.h"
#include "ctc_hash.h"
#include "sys_usw_common.h"
#include "sys_usw_stats.h"
#include "sys_usw_ftm.h"
#include "sys_usw_chip.h"
#include "sys_usw_opf.h"
#include "sys_usw_port.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_l3if.h"
#include "sys_usw_nexthop_api.h"
#include "sys_usw_dma.h"
#include "sys_usw_interrupt.h"
#include "sys_usw_register.h"
#include "sys_usw_acl_api.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_dmps.h"
#include "sys_usw_register.h"

#include "drv_api.h"
#include "usw/include/drv_common.h"

/****************************************************************************
 *
* global variable
*
*****************************************************************************/
sys_stats_master_t* usw_flow_stats_master[CTC_MAX_LOCAL_CHIP_NUM] =
{
    NULL
};
static tbls_id_t usw_flow_stats_ram_to_cache[SYS_STATS_RAM_MAX] =
{
    DsStatsEgressGlobal0_t,
    DsStatsEgressGlobal1_t,
    DsStatsEgressGlobal2_t,
    DsStatsEgressGlobal3_t,
    DsStatsEgressACL0_t,
    DsStatsIngressGlobal0_t,
    DsStatsIngressGlobal1_t,
    DsStatsIngressGlobal2_t,
    DsStatsIngressGlobal3_t,
    DsStatsIngressACL0_t,
    DsStatsIngressACL1_t,
    DsStatsIngressACL2_t,
    DsStatsIngressACL3_t,
    DsStatsQueue_t
};

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/
#define SYS_STATS_INIT_CHECK() \
    do { \
        LCHIP_CHECK(lchip); \
        if (usw_flow_stats_master[lchip] == NULL){ \
            SYS_STATS_DBG_ERROR(" Feature not initialized \n");\
			return CTC_E_NOT_INIT;\
        } \
    } while (0)

#define SYS_STATS_TYPE_CHECK(type) \
    do { \
        if ((type < CTC_STATS_TYPE_FWD) || (type >= CTC_STATS_TYPE_MAX)) \
            return CTC_E_NOT_SUPPORT; \
    } while (0)

#define SYS_STATS_TYPE_ENABLE(type) (usw_flow_stats_master[lchip]->stats_bitmap & type)

#define STATS_LOCK()   \
    do { \
        if (usw_flow_stats_master[lchip]->p_stats_mutex) sal_mutex_lock(usw_flow_stats_master[lchip]->p_stats_mutex); \
    } while (0)

#define STATS_UNLOCK() \
    do { \
        if (usw_flow_stats_master[lchip]->p_stats_mutex) sal_mutex_unlock(usw_flow_stats_master[lchip]->p_stats_mutex); \
    } while (0)

#define CTC_ERROR_RETURN_UNLOCK(op) \
    do { \
        int32 rv; \
        if ((rv = (op)) < 0) \
        { \
            sal_mutex_unlock(usw_flow_stats_master[lchip]->p_stats_mutex); \
            return (rv); \
        } \
    } while (0)

/*not suit for acl & sdc */
#define SYS_STATS_PARSER_RAM_INDEX(type, dir, statsptr) ((SYS_STATS_TYPE_QUEUE == (type)) ? SYS_STATS_RAM_QUEUE : \
                                                                                        ((statsptr >> (DRV_IS_DUET2(lchip)? 12 : 13)) + ( dir == CTC_INGRESS ? SYS_STATS_RAM_IPE_GLOBAL0 : 0)))
#define SYS_STATS_ENCODE_SYS_PTR(type, offset)  (((type) << 16) | ((offset)&0xFFFF))

#define SYS_STATS_ENCODE_CHIP_PTR(type, offset) (DRV_IS_DUET2(lchip)? \
                                                    ((SYS_STATS_RAM_SDC == (type)) ? ((1 << 14) | ((offset) & 0x3FFF)) : \
                                                                                        ((((((type) >= SYS_STATS_RAM_IPE_PRIVATE0) && ((type) <= SYS_STATS_RAM_IPE_PRIVATE3)) || (type)== SYS_STATS_RAM_EPE_PRIVATE0) << 15) \
                                                                                            | (((SYS_STATS_RAM_QUEUE == (type)) ? 0 : ((type) <= SYS_STATS_RAM_EPE_PRIVATE0 ? (type)%4 : ((type)-1)%4)) << 12) \
                                                                                            | ((offset) & 0xFFF)))\
                                                    :((SYS_STATS_RAM_SDC == (type)) ? ((1 << 15) | ((offset) & 0x7FFF)) : \
                                                                                        ((((((type) >= SYS_STATS_RAM_IPE_PRIVATE0) && ((type) <= SYS_STATS_RAM_IPE_PRIVATE3)) || (type)== SYS_STATS_RAM_EPE_PRIVATE0) << 16) \
                                                                                            | (((SYS_STATS_RAM_QUEUE == (type)) ? 0 : ((type) <= SYS_STATS_RAM_EPE_PRIVATE0 ? (type)%4 : ((type)-1)%4)) << 13) \
                                                                                            | ((offset) & 0x1FFF))))

#define SYS_STATS_DECODE_PTR_RAM(ptr)          (((ptr)>>16)&0xF)
#define SYS_STATS_DECODE_PTR_OFFSET(ptr)        ((ptr)&0xFFFF)

extern int32
ctc_hash_get_count(ctc_hash_t* hash, uint32* count);
extern int32
sys_usw_mac_stats_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param);
extern int32
sys_usw_flow_stats_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param);
#define __1_STATS_COMMON__
STATIC int32
_sys_usw_flow_stats_map_type(uint8 lchip, ctc_stats_statsid_type_t ctc_type, uint8 is_vc, uint8* p_sys_type, uint8* p_dir)
{
    uint8 type = 0;
    CTC_PTR_VALID_CHECK(p_sys_type);

    switch(ctc_type)
    {
    case CTC_STATS_STATSID_TYPE_VLAN:
        type = SYS_STATS_TYPE_VLAN;
        *p_dir = CTC_BOTH_DIRECTION;
        break;

    case CTC_STATS_STATSID_TYPE_VRF:
        type = SYS_STATS_TYPE_VRF;
        *p_dir = CTC_INGRESS;
        break;

    case CTC_STATS_STATSID_TYPE_ACL:
        type = SYS_STATS_TYPE_ACL;
        *p_dir = CTC_BOTH_DIRECTION;
        break;

    case CTC_STATS_STATSID_TYPE_IP/*CTC_STATS_STATSID_TYPE_IPMC*/:
        type = SYS_STATS_TYPE_FIB;
        *p_dir = CTC_INGRESS;
        break;

    case CTC_STATS_STATSID_TYPE_MAC:
        type = SYS_STATS_TYPE_MAC;
        *p_dir = CTC_INGRESS;
        break;

    case CTC_STATS_STATSID_TYPE_FLOW_HASH:
        type = SYS_STATS_TYPE_FLOW_HASH;
        *p_dir = CTC_INGRESS;
        break;

    case CTC_STATS_STATSID_TYPE_MPLS:
        type = is_vc ? SYS_STATS_TYPE_PW : SYS_STATS_TYPE_LSP;
        *p_dir = CTC_INGRESS;
        break;

    case CTC_STATS_STATSID_TYPE_MPLS_PW:
        type = SYS_STATS_TYPE_PW;
        *p_dir = CTC_INGRESS;
        break;

    case CTC_STATS_STATSID_TYPE_TUNNEL:
        type = SYS_STATS_TYPE_TUNNEL;
        *p_dir = CTC_BOTH_DIRECTION;
        break;

    case CTC_STATS_STATSID_TYPE_SCL:
        type = SYS_STATS_TYPE_SCL;
        *p_dir = CTC_BOTH_DIRECTION;
        break;

    case CTC_STATS_STATSID_TYPE_NEXTHOP:
        type = SYS_STATS_TYPE_FWD_NH;
        *p_dir = CTC_EGRESS;
        break;

    case CTC_STATS_STATSID_TYPE_NEXTHOP_MPLS_PW:
        type = SYS_STATS_TYPE_PW;
        *p_dir = CTC_EGRESS;
        break;

    case CTC_STATS_STATSID_TYPE_NEXTHOP_MPLS_LSP:
        type = SYS_STATS_TYPE_LSP;
        *p_dir = CTC_EGRESS;
        break;

    case CTC_STATS_STATSID_TYPE_L3IF:
        type = SYS_STATS_TYPE_INTF;
        *p_dir = CTC_BOTH_DIRECTION;
        break;

    case CTC_STATS_STATSID_TYPE_FID:
        type = SYS_STATS_TYPE_VLAN;
        *p_dir = CTC_BOTH_DIRECTION;
        break;

    case CTC_STATS_STATSID_TYPE_NEXTHOP_MCAST:
        type = SYS_STATS_TYPE_FWD_NH;
        *p_dir = CTC_INGRESS;
        break;

    case CTC_STATS_STATSID_TYPE_ECMP:
        type = SYS_STATS_TYPE_ECMP;
        *p_dir = CTC_INGRESS;
        break;

    case CTC_STATS_STATSID_TYPE_SDC:
        type = SYS_STATS_TYPE_SDC;
        *p_dir = CTC_INGRESS;
        break;
    case CTC_STATS_STATSID_TYPE_PORT:
        type = SYS_STATS_TYPE_PORT;
        *p_dir = CTC_BOTH_DIRECTION;
        break;
    case CTC_STATS_STATSID_TYPE_POLICER0:
        type = SYS_STATS_TYPE_POLICER0;
        *p_dir = CTC_BOTH_DIRECTION;
        break;
    case CTC_STATS_STATSID_TYPE_POLICER1:
        type = SYS_STATS_TYPE_POLICER1;
        *p_dir = CTC_BOTH_DIRECTION;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    *p_sys_type = type;

    return CTC_E_NONE;
}

STATIC int32
_sys_stats_flow_stats_map_dma_blkid(uint8 lchip, uint32 stats_ptr, uint8* p_blkid)
{
    uint8 ram = 0;
    uint32 offset = 0;
    uint8 blkid = 0;
    sys_stats_ram_t* p_ram = NULL;

    CTC_PTR_VALID_CHECK(p_blkid);

    ram = SYS_STATS_DECODE_PTR_RAM(stats_ptr);
    offset = SYS_STATS_DECODE_PTR_OFFSET(stats_ptr);
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[ram]);

    blkid = (p_ram->base_idx + offset)/MCHIP_CAP(SYS_CAP_STATS_DMA_BLOCK_SIZE);
    *p_blkid = blkid;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_flow_stats_map_ramid(uint8 lchip, uint8 num, sys_stats_statsid_t* p_statsid, uint8* p_ramid)
{
    sys_stats_ram_t* p_ram = NULL;
    uint8 i = 0;
    ctc_acl_league_t league;
    sys_stats_ram_type_t acl_ram[SYS_STATS_RAM_MAX] = {SYS_STATS_RAM_EPE_PRIVATE0,
                                        SYS_STATS_RAM_EPE_GLOBAL0, SYS_STATS_RAM_EPE_GLOBAL1 ,SYS_STATS_RAM_EPE_GLOBAL2 , SYS_STATS_RAM_EPE_GLOBAL3,
                                        SYS_STATS_RAM_IPE_PRIVATE0, SYS_STATS_RAM_IPE_PRIVATE1, SYS_STATS_RAM_IPE_PRIVATE2, SYS_STATS_RAM_IPE_PRIVATE3,
                                        SYS_STATS_RAM_IPE_GLOBAL0, SYS_STATS_RAM_IPE_GLOBAL1, SYS_STATS_RAM_IPE_GLOBAL2, SYS_STATS_RAM_IPE_GLOBAL3,
                                        SYS_STATS_RAM_QUEUE, SYS_STATS_RAM_SDC};

    CTC_PTR_VALID_CHECK(p_ramid);
    CTC_MIN_VALUE_CHECK(num, 1);

    *p_ramid = SYS_STATS_RAM_MAX;

    if (SYS_STATS_TYPE_ACL == p_statsid->stats_id_type)
    {
        sal_memset(&league, 0, sizeof(league));
        league.dir = p_statsid->dir;
        league.acl_priority = p_statsid->u.acl_priority;
        CTC_ERROR_RETURN(sys_usw_acl_get_league_mode(lchip, &league));

        SYS_STATS_DBG_INFO("Acl direction %d, prioriy is %d, lookup level bitmap 0x%-4x\n", league.dir, league.acl_priority, league.lkup_level_bitmap);
    }

    for (i = 0; i < SYS_STATS_RAM_MAX; i++)
    {
        p_ram = &(usw_flow_stats_master[lchip]->stats_ram[acl_ram[i]]);
        if (CTC_IS_BIT_SET(p_ram->stats_bmp, p_statsid->stats_id_type) && (p_ram->used_cnt + num <= p_ram->total_cnt - 1))
        {
            if ((p_statsid->dir == CTC_INGRESS && acl_ram[i] >= SYS_STATS_RAM_IPE_GLOBAL0)
                || (p_statsid->dir == CTC_EGRESS && acl_ram[i] < SYS_STATS_RAM_IPE_GLOBAL0))
            {
                if (SYS_STATS_TYPE_ACL == p_statsid->stats_id_type && (!CTC_IS_BIT_SET(league.lkup_level_bitmap, p_ram->acl_priority)))
                {
                    continue;
                }
                *p_ramid = acl_ram[i];
                break;
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_flow_stats_ds2count(uint8 lchip, ds_t* p_ds, ctc_stats_basic_t* p_count)
{
    uint64 value = 0;
    uint32 count[2] = {0};

    CTC_PTR_VALID_CHECK(p_count);

    value = GetDsStats(V, packetCount_f, p_ds);
    p_count->packet_count = value;

    value = GetDsStats(A, byteCount_f, p_ds, count);
    value |= count[1];
    value <<= 32;
    value |= count[0];
    p_count->byte_count = value;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_flow_stats_read_count(uint8 lchip, uint16 stats_idx, ctc_stats_basic_t* p_count)
{
    uint32 cmd = 0;
    ds_t   ds;
    drv_work_platform_type_t platform_type = 0;

    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_count);

    sal_memset(&ds, 0, sizeof(ds_t));
    sal_memset(p_count, 0, sizeof(ctc_stats_basic_t));

    cmd = DRV_IOR(DsStats_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, stats_idx, cmd, &ds));

    CTC_ERROR_RETURN(_sys_usw_flow_stats_ds2count(lchip, &ds, p_count));

    CTC_ERROR_RETURN(drv_get_platform_type(lchip, &platform_type));
    if ((SW_SIM_PLATFORM == platform_type) && usw_flow_stats_master[lchip]->clear_read_en)
    {
        sal_memset(ds, 0, sizeof(ds_t));
        cmd = DRV_IOW(DsStats_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, stats_idx, cmd, &ds);
        SYS_STATS_DBG_INFO( "clear stats:0x%04X by software in uml\n", stats_idx);
    }

    return CTC_E_NONE;
}

#define __2_STATS_DB__
STATIC uint32
_sys_usw_flow_stats_hash_key_make(sys_stats_flow_stats_t* p_flow_hash)
{
    uint32 size = 0;
    uint32 stats_ptr = 0;
    uint8* k = NULL;

    CTC_PTR_VALID_CHECK(p_flow_hash);

    stats_ptr = p_flow_hash->stats_ptr;
    size = sizeof(stats_ptr);

    k = (uint8*)(&stats_ptr);

    return ctc_hash_caculate(size, k);
}

STATIC int32
_sys_usw_flow_stats_hash_key_cmp(sys_stats_flow_stats_t* p_flow_hash1, sys_stats_flow_stats_t* p_flow_hash)
{
    CTC_PTR_VALID_CHECK(p_flow_hash1);
    CTC_PTR_VALID_CHECK(p_flow_hash);

    if (p_flow_hash1->stats_ptr != p_flow_hash->stats_ptr)
    {
        return FALSE;
    }

    return TRUE;
}

STATIC int32
_sys_usw_flow_stats_entry_create(uint8 lchip, uint16 num, uint32 stats_ptr, uint32 flag)
{
    uint16 i = 0;
    uint8 ram = 0;
    uint8 blkid = 0;
    uint16 proc_num = num;
    sys_stats_flow_stats_t flow_stats;
    sys_stats_flow_stats_t* p_flow_stats = NULL;
    void* p_ret = NULL;

    CTC_MIN_VALUE_CHECK(num, 1);

    sal_memset(&flow_stats, 0, sizeof(flow_stats));

    for (i=0; i<proc_num; i++)
    {
        flow_stats.stats_ptr = stats_ptr + i;
        p_flow_stats = ctc_hash_lookup(usw_flow_stats_master[lchip]->flow_stats_hash, &flow_stats);
        if (p_flow_stats)
        {
            SYS_STATS_DBG_ERROR(" Entry exists stats_ptr=0x%.8x p_flow_stats=%p \n", (uint32)p_flow_stats->stats_ptr, p_flow_stats);
            continue;
        }

        p_flow_stats = (sys_stats_flow_stats_t*)mem_malloc(MEM_STATS_MODULE, sizeof(sys_stats_flow_stats_t));
        if (NULL == p_flow_stats)
        {
            proc_num = i;
            SYS_STATS_DBG_ERROR(" No memory \n");
            goto error_roll;
        }

        sal_memset(p_flow_stats, 0, sizeof(sys_stats_flow_stats_t));
        p_flow_stats->stats_ptr = stats_ptr + i;
        p_flow_stats->flag = flag;

        /* add it to fwd stats hash */
        p_ret = ctc_hash_insert(usw_flow_stats_master[lchip]->flow_stats_hash, p_flow_stats);
        if (NULL == p_ret)
        {
            SYS_STATS_DBG_ERROR(" Entry insert fail stats_ptr=0x%.8x p_flow_stats=%p \n", p_flow_stats->stats_ptr, p_flow_stats);
            goto error_roll;
        }

        ram = SYS_STATS_DECODE_PTR_RAM(stats_ptr + i);
        if (ram < SYS_STATS_RAM_SDC)
        {
            _sys_stats_flow_stats_map_dma_blkid(lchip, stats_ptr + i, &blkid);
            ctc_list_pointer_insert_tail(&(usw_flow_stats_master[lchip]->flow_stats_list[blkid]), &(p_flow_stats->list_node));
        }
    }

    return CTC_E_NONE;

error_roll:

    for (i=0; i<proc_num; i++)
    {
        flow_stats.stats_ptr = stats_ptr + i;
        p_flow_stats = ctc_hash_lookup(usw_flow_stats_master[lchip]->flow_stats_hash, &flow_stats);
        if (!p_flow_stats)
        {
            continue;
        }

        ram = SYS_STATS_DECODE_PTR_RAM(stats_ptr + i);
        if (ram < SYS_STATS_RAM_SDC)
        {
            _sys_stats_flow_stats_map_dma_blkid(lchip, stats_ptr + i, &blkid);
            ctc_list_pointer_delete(&(usw_flow_stats_master[lchip]->flow_stats_list[blkid]), &(p_flow_stats->list_node));
        }

        ctc_hash_remove(usw_flow_stats_master[lchip]->flow_stats_hash, p_flow_stats);
        mem_free(p_flow_stats);
    }

    return CTC_E_NO_MEMORY;
}

STATIC int32
_sys_usw_flow_stats_entry_delete(uint8 lchip, uint16 num, uint32 stats_ptr)
{
    uint16 i = 0;
    uint8 ram = 0;
    uint8 blkid = 0;
    int32 ret = CTC_E_NONE;
    sys_stats_flow_stats_t flow_stats;
    sys_stats_flow_stats_t* p_flow_stats = NULL;
    void* p_ret = NULL;

    CTC_MIN_VALUE_CHECK(num, 1);

    sal_memset(&flow_stats, 0, sizeof(flow_stats));

    for (i=0; i<num; i++)
    {
        flow_stats.stats_ptr = stats_ptr + i;
        p_flow_stats = ctc_hash_lookup(usw_flow_stats_master[lchip]->flow_stats_hash, &flow_stats);
        if (p_flow_stats)
        {
            ram = SYS_STATS_DECODE_PTR_RAM(stats_ptr + i);
            if (ram < SYS_STATS_RAM_SDC)
            {
                _sys_stats_flow_stats_map_dma_blkid(lchip, stats_ptr + i, &blkid);
                ctc_list_pointer_delete(&(usw_flow_stats_master[lchip]->flow_stats_list[blkid]), &(p_flow_stats->list_node));
            }
            p_ret = ctc_hash_remove(usw_flow_stats_master[lchip]->flow_stats_hash, p_flow_stats);
            if (NULL == p_ret)
            {
                SYS_STATS_DBG_ERROR(" Entry remove fail stats_ptr=0x%.8x p_flow_stats=%p \n", (uint32)p_flow_stats->stats_ptr, p_flow_stats);
            }
            mem_free(p_flow_stats);
        }
        else
        {
            SYS_STATS_DBG_ERROR(" Entry not exist \n");
    		ret = CTC_E_NOT_EXIST;
            continue;
        }
    }

    return ret;
}

STATIC int32
_sys_usw_flow_stats_entry_lookup(uint8 lchip, uint32 stats_ptr, sys_stats_flow_stats_t** pp_fwd_stats)
{
    sys_stats_flow_stats_t flow_stats;
    sys_stats_flow_stats_t* p_flow_stats = NULL;

    CTC_PTR_VALID_CHECK(pp_fwd_stats);
    sal_memset(&flow_stats, 0, sizeof(flow_stats));

    *pp_fwd_stats = NULL;

    flow_stats.stats_ptr = stats_ptr;
    p_flow_stats = ctc_hash_lookup(usw_flow_stats_master[lchip]->flow_stats_hash, &flow_stats);
    if (p_flow_stats)
    {
        *pp_fwd_stats = p_flow_stats;
    }

    return CTC_E_NONE;
}

#define __3_STATS_STATSPTR__
STATIC int32
_sys_usw_flow_stats_alloc_statsptr(uint8 lchip, uint16 num, sys_stats_statsid_t* p_statsid)
{
    uint32 offset = 0;
    sys_usw_opf_t opf;
    sys_stats_prop_t* p_prop = NULL;
    sys_stats_ram_t* p_ram = NULL;

    SYS_STATS_DBG_FUNC();
    CTC_MIN_VALUE_CHECK(num, 1);

    p_prop = &(usw_flow_stats_master[lchip]->stats_type[p_statsid->dir][p_statsid->stats_id_type]);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = usw_flow_stats_master[lchip]->opf_type_flow_stats;

    CTC_ERROR_RETURN(_sys_usw_flow_stats_map_ramid(lchip, num, p_statsid, &opf.pool_index));
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[opf.pool_index]);

    if (opf.pool_index >= SYS_STATS_RAM_MAX
        || usw_flow_stats_master[lchip]->alloc_count + num > TABLE_MAX_INDEX(lchip, DsStats_t)
        || (!CTC_IS_BIT_SET(p_ram->stats_bmp, p_statsid->stats_id_type)))
    {
        SYS_STATS_DBG_ERROR(" No resource in ASIC \n");
        return CTC_E_NO_RESOURCE;
    }
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, num, &offset));
    SYS_STATS_DBG_INFO("dir:%d, ram:%d, offset:%u.\n",p_statsid->dir, opf.pool_index, offset);
    p_statsid->stats_ptr = SYS_STATS_ENCODE_SYS_PTR(opf.pool_index, offset);
    SYS_STATS_DBG_INFO("alloc flow stats stats_ptr:0x%04X\n", p_statsid->stats_ptr);

    p_ram->used_cnt += num;
    p_prop->used_cnt += num;
    p_prop->used_cnt_ram[opf.pool_index] += num;
    usw_flow_stats_master[lchip]->alloc_count += num;

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_flow_stats_free_statsptr(uint8 lchip, uint16 num, sys_stats_statsid_t* p_statsid)
{
    sys_usw_opf_t opf;
    sys_stats_ram_t* p_ram = NULL;
    sys_stats_prop_t* p_prop = NULL;

    SYS_STATS_DBG_FUNC();
    CTC_MIN_VALUE_CHECK(num, 1);

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = usw_flow_stats_master[lchip]->opf_type_flow_stats;
    opf.pool_index = SYS_STATS_DECODE_PTR_RAM(p_statsid->stats_ptr);

    SYS_STATS_DBG_INFO("free flow stats_ptr:0x%08X, ram:%u, index:%u\n",
                       p_statsid->stats_ptr, opf.pool_index, SYS_STATS_DECODE_PTR_OFFSET(p_statsid->stats_ptr));
    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, num, SYS_STATS_DECODE_PTR_OFFSET(p_statsid->stats_ptr)));

    p_prop = &(usw_flow_stats_master[lchip]->stats_type[p_statsid->dir][p_statsid->stats_id_type]);
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[opf.pool_index]);
    p_ram->used_cnt -= num;
    p_prop->used_cnt -= num;
    p_prop->used_cnt_ram[opf.pool_index] -= num;
    usw_flow_stats_master[lchip]->alloc_count -= num;
    return CTC_E_NONE;
}

#define __4_FLOW_STATS__

STATIC int32
_sys_usw_flow_stats_get_stats(uint8 lchip, uint32 stats_ptr, uint16 num, ctc_stats_basic_t* p_count)
{
    uint16 i = 0;
    uint8 ram = 0;
    uint32 stats_idx = 0;
    uint32 tmp_ptr = 0;
    sys_stats_ram_t* p_ram = NULL;
    sys_stats_flow_stats_t* p_fwd_stats = NULL;
    ctc_stats_basic_t* p_cnt = NULL;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_count);
    CTC_MIN_VALUE_CHECK(num, 1);

    sal_memset(p_count, 0, sizeof(ctc_stats_basic_t) * num);

    for (i=0; i<num; i++)
    {
        tmp_ptr = stats_ptr + i;
        p_cnt = p_count + i;

        CTC_ERROR_RETURN(_sys_usw_flow_stats_entry_lookup(lchip, tmp_ptr, &p_fwd_stats));

        ram = SYS_STATS_DECODE_PTR_RAM(tmp_ptr);
        if (ram != SYS_STATS_RAM_SDC)
        {
            p_ram = &(usw_flow_stats_master[lchip]->stats_ram[ram]);
            stats_idx = p_ram->base_idx + SYS_STATS_DECODE_PTR_OFFSET(tmp_ptr);
            CTC_ERROR_RETURN(_sys_usw_flow_stats_read_count(lchip, stats_idx, p_cnt));
        }

        if (NULL != p_fwd_stats)
        {
            if (usw_flow_stats_master[lchip]->clear_read_en || (ram == SYS_STATS_RAM_SDC))
            {
                p_fwd_stats->packet_count += p_cnt->packet_count;
                p_fwd_stats->byte_count += p_cnt->byte_count;

                p_cnt->packet_count = p_fwd_stats->packet_count;
                p_cnt->byte_count = p_fwd_stats->byte_count;
            }
            else
            {
                p_fwd_stats->packet_count = p_cnt->packet_count;
                p_fwd_stats->byte_count = p_cnt->byte_count;
            }
        }
        else
        {
            CTC_ERROR_RETURN(_sys_usw_flow_stats_entry_create(lchip, 1, tmp_ptr, 0));
            CTC_ERROR_RETURN(_sys_usw_flow_stats_entry_lookup(lchip, tmp_ptr, &p_fwd_stats));
            if (!p_fwd_stats)
            {
                SYS_STATS_DBG_ERROR(" No memory \n");
    			return CTC_E_NO_MEMORY;
            }

            p_fwd_stats->packet_count = p_cnt->packet_count;
            p_fwd_stats->byte_count = p_cnt->byte_count;
        }

        SYS_STATS_DBG_INFO("stats_ptr:0x%08X, stats_idx:0x%08X, packets:%"PRId64", bytes:%"PRId64"\n",
                           tmp_ptr, stats_idx, p_cnt->packet_count, p_cnt->byte_count);
    }

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_flow_stats_clear_stats(uint8 lchip, uint16 num, uint32 stats_ptr)
{
    uint16 i = 0;
    ds_t   ds = {0};
    uint32 cmd = 0;
    uint8  ram = 0;
    uint32 stats_idx = 0;
    uint32 tmp_ptr = 0;
    ctc_stats_basic_t count;
    sys_stats_ram_t* p_ram = NULL;
    sys_stats_flow_stats_t* p_fwd_stats = NULL;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    CTC_MIN_VALUE_CHECK(num, 1);
    SYS_STATS_DBG_INFO("stats_ptr:0x%08X\n", stats_ptr);

    for (i=0; i<num; i++)
    {
        tmp_ptr = stats_ptr+i;

        CTC_ERROR_RETURN(_sys_usw_flow_stats_entry_lookup(lchip, tmp_ptr, &p_fwd_stats));
        if (NULL != p_fwd_stats)
        {
            SYS_STATS_DBG_INFO("clear stats_ptr:0x%08X stats db.\n", tmp_ptr);
            p_fwd_stats->packet_count = 0;
            p_fwd_stats->byte_count = 0;

            ram = SYS_STATS_DECODE_PTR_RAM(tmp_ptr);
            if (ram != SYS_STATS_RAM_SDC)
            {
                p_ram = &(usw_flow_stats_master[lchip]->stats_ram[ram]);
                stats_idx = p_ram->base_idx + SYS_STATS_DECODE_PTR_OFFSET(tmp_ptr);
                CTC_ERROR_RETURN(_sys_usw_flow_stats_read_count(lchip, stats_idx, &count));

                /* cache may contain count but not reach threshold, may influence next stats */
                cmd = DRV_IOW(usw_flow_stats_ram_to_cache[(tmp_ptr>>16)&0xF], DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, (tmp_ptr&0xFFFF), cmd, &ds));
            }
        }
    }

    return CTC_E_NONE;
}

#define __5_STATS_SYNC__
STATIC int32
_sys_usw_flow_stats_sync_sdc_stats(uint8 lchip, void* p_data)
{
    int32 ret = 0;
    uint32 stats_ptr[CTC_BOTH_DIRECTION] = {0};
    uint32 pkt_len[CTC_BOTH_DIRECTION] = {0};
    uint8 valid[CTC_BOTH_DIRECTION] = {0};
    sys_dma_info_t* p_info = NULL;
    DmaToCpuSdcFifo_m* p_sdc = NULL;
    sys_stats_flow_stats_t* p_fwd_stats = NULL;
    uint8 index = 0;
    uint32 stats_ptr_used = 0;
    uint32 pkt_len_used = 0;
    uint8 sub_idx = 0;

    p_info = (sys_dma_info_t*)p_data;

    SYS_STATS_INIT_CHECK();

    for (index = 0; index < p_info->entry_num; index++)
    {
        p_sdc = (DmaToCpuSdcFifo_m*)p_info->p_data + index;

        valid[0] = 0;
        valid[1] = 0;
        if (GetDmaToCpuSdcFifo(V,ipeSdcValid_f, p_sdc))
        {
            stats_ptr[CTC_INGRESS] = GetDmaToCpuSdcFifo(V,ipeSdcPtr_f, p_sdc);
            pkt_len[CTC_INGRESS] = GetDmaToCpuSdcFifo(V,ipeSdcLen_f, p_sdc);
            stats_ptr[CTC_INGRESS] = SYS_STATS_ENCODE_SYS_PTR(SYS_STATS_RAM_SDC, stats_ptr[CTC_INGRESS]);
            valid[CTC_INGRESS] = 1;
        }

        if(GetDmaToCpuSdcFifo(V,epeSdcValid_f, p_sdc))
        {
            stats_ptr[CTC_EGRESS] = GetDmaToCpuSdcFifo(V,epeSdcPtr_f, p_sdc);
            pkt_len[CTC_EGRESS] = GetDmaToCpuSdcFifo(V,epeSdcLen_f, p_sdc);
            stats_ptr[CTC_EGRESS] = SYS_STATS_ENCODE_SYS_PTR(SYS_STATS_RAM_SDC, stats_ptr[CTC_EGRESS]);
            valid[CTC_EGRESS] = 1;
        }

        STATS_LOCK();

        for (sub_idx = 0; sub_idx < CTC_BOTH_DIRECTION; sub_idx++)
        {
            if (valid[sub_idx])
            {
                stats_ptr_used = stats_ptr[sub_idx];
                pkt_len_used = pkt_len[sub_idx];

                SYS_STATS_DBG_INFO( "Stats_ptr:0x%x \n", stats_ptr_used);
                SYS_STATS_DBG_INFO( "Pkt Len:0x%x \n", pkt_len_used);

                CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_entry_lookup(lchip, stats_ptr_used, &p_fwd_stats));

                if (NULL != p_fwd_stats)
                {
                    p_fwd_stats->packet_count += 1;
                    p_fwd_stats->byte_count += pkt_len_used;
                }
                else
                {
                    /*add to db*/
                    ret = _sys_usw_flow_stats_entry_create(lchip, 1, stats_ptr_used, 0);
                    if (ret)
                    {
                        continue;
                    }
                    ret = _sys_usw_flow_stats_entry_lookup(lchip, stats_ptr_used, &p_fwd_stats);
                    if (ret || !p_fwd_stats)
                    {
                        _sys_usw_flow_stats_entry_delete(lchip, 1, stats_ptr_used);
                        continue;
                    }

                    p_fwd_stats->stats_ptr = stats_ptr_used;
                    p_fwd_stats->packet_count = 1;
                    p_fwd_stats->byte_count = pkt_len_used;
                }
            }
        }

        STATS_UNLOCK();
    }


    return CTC_E_NONE;

}

STATIC int32
_sys_usw_flow_stats_sync_flow_stats(uint8 lchip, void* p_data)
{
    int32 ret = 0;
    uint8 ram = 0;
    uint8  blkid = 0;
    uint8* p_addr = NULL;
    ctc_list_pointer_node_t* p_node = NULL;
    sys_stats_flow_stats_t* p_flow_stats = NULL;
    sys_dma_reg_t* p_dma_reg = (sys_dma_reg_t*)p_data;
    DsStats_m hw_stats;
    ctc_stats_basic_t sw_count;
    uint16 stats_offset = 0;
    uint32 ram_offset = 0;
    sys_stats_ram_t* p_ram = NULL;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_DBG_INFO("BlockId:%d, Address:%p\n", *((uint8*)p_dma_reg->p_ext), p_dma_reg->p_data);

    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_data);

    blkid = *((uint16*)p_dma_reg->p_ext);
    CTC_MAX_VALUE_CHECK(blkid, MCHIP_CAP(SYS_CAP_STATS_DMA_BLOCK_NUM)-1);
    p_addr = p_dma_reg->p_data;

    STATS_LOCK();

    for (p_node = ctc_list_pointer_head(&(usw_flow_stats_master[lchip]->flow_stats_list[blkid])); p_node; p_node = ctc_list_pointer_next(p_node))
     /*-CTC_LIST_POINTER_LOOP(p_node, &(usw_flow_stats_master[lchip]->flow_stats_list[blkid]));*/
    {
        p_flow_stats = _ctc_container_of(p_node, sys_stats_flow_stats_t, list_node);

        sal_memset(&sw_count, 0, sizeof(ctc_stats_basic_t));

        ram_offset = SYS_STATS_DECODE_PTR_OFFSET(p_flow_stats->stats_ptr);
        ram = SYS_STATS_DECODE_PTR_RAM(p_flow_stats->stats_ptr);
        p_ram = &(usw_flow_stats_master[lchip]->stats_ram[ram]);
        stats_offset = (p_ram->base_idx + ram_offset) % MCHIP_CAP(SYS_CAP_STATS_DMA_BLOCK_SIZE);

        sal_memcpy(&hw_stats, (uint8*)p_addr + stats_offset * 8, sizeof(hw_stats));
        ret = _sys_usw_flow_stats_ds2count(lchip, (ds_t*)(&hw_stats), &sw_count);
        if (ret)
        {
            SYS_STATS_DBG_ERROR("Dma sync error stats_ptr:0x%04X\n", p_flow_stats->stats_ptr);
        }
        else
        {
            p_flow_stats->packet_count += sw_count.packet_count;
            p_flow_stats->byte_count += sw_count.byte_count;
        }
    }

    STATS_UNLOCK();

    return CTC_E_NONE;
}

#define __6_STATS_STATSID__
STATIC int32
_sys_usw_flow_stats_id_hash_key_make(sys_stats_statsid_t* statsid)
{
    return statsid->stats_id;
}

STATIC int32
_sys_usw_flow_stats_id_hash_key_cmp(sys_stats_statsid_t* statsid_0, sys_stats_statsid_t* statsid_1)
{
    return (statsid_0->stats_id == statsid_1->stats_id);
}

STATIC int32
_sys_usw_flow_stats_id_hash_value_cmp(sys_stats_statsid_t* p_statsid_lkp, sys_stats_statsid_t* p_statsid_info)
{
    if (p_statsid_lkp->hw_stats_ptr == p_statsid_info->hw_stats_ptr
        && p_statsid_lkp->dir == p_statsid_info->dir
        && p_statsid_lkp->stats_id_type == p_statsid_info->stats_id_type)
    {
        p_statsid_info->stats_id = p_statsid_lkp->stats_id;
        return CTC_E_EXIST;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_flow_stats_alloc_statsid(uint8 lchip, uint32* p_statsid)
{
    sys_stats_statsid_t  sys_statsid_lkp;
    sys_stats_statsid_t* p_sys_stats_id = NULL;
    sys_usw_opf_t opf;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_statsid);

    /*statsid maybe alloc*/
    if (usw_flow_stats_master[lchip]->stats_mode == CTC_STATS_MODE_USER)
    {
        CTC_MIN_VALUE_CHECK((*p_statsid), 1);

        sal_memset(&sys_statsid_lkp, 0, sizeof(sys_statsid_lkp));
        sys_statsid_lkp.stats_id = *p_statsid;
        p_sys_stats_id = ctc_hash_lookup(usw_flow_stats_master[lchip]->flow_stats_id_hash, &sys_statsid_lkp);
        if (p_sys_stats_id != NULL)
        {
            SYS_STATS_DBG_ERROR(" Entry already exist \n");
			return CTC_E_EXIST;
        }
    }
    else
    {
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type = usw_flow_stats_master[lchip]->opf_type_stats_id;

        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, 1, p_statsid));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_flow_stats_free_statsid(uint8 lchip, uint32 stats_id)
{
    sys_usw_opf_t opf;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    CTC_MIN_VALUE_CHECK(stats_id, 1);

    if (usw_flow_stats_master[lchip]->stats_mode == CTC_STATS_MODE_DEFINE)
    {
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
        opf.pool_type = usw_flow_stats_master[lchip]->opf_type_stats_id;
        CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, 1, stats_id));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_flow_stats_create_statsid(uint8 lchip, sys_stats_statsid_t* p_statsid)
{
    uint32 ret = CTC_E_NONE;
    uint16 stats_num = 0;
    sys_stats_statsid_t* p_sys_statsid = NULL;
    sys_stats_prop_t* p_prop = NULL;
    uint16 entry_num = 0;

    SYS_STATS_DBG_FUNC();

    p_sys_statsid = mem_malloc(MEM_STATS_MODULE,  sizeof(sys_stats_statsid_t));
    if (p_sys_statsid == NULL)
    {
        SYS_STATS_DBG_ERROR(" No memory \n");
        return CTC_E_NO_MEMORY;
    }

    ret = _sys_usw_flow_stats_alloc_statsid(lchip, &(p_statsid->stats_id));
    if (ret != CTC_E_NONE)
    {
        goto error_roll_0;
    }

    sal_memcpy(p_sys_statsid, p_statsid, sizeof(sys_stats_statsid_t));

    /* stats_ptr from sys_usw_flow_stats_add_ecmp_stats which called by nexthop */
    if (SYS_STATS_TYPE_ECMP != p_sys_statsid->stats_id_type)
    {
        stats_num = (p_sys_statsid->color_aware ? 3: 1);
        ret = _sys_usw_flow_stats_alloc_statsptr(lchip, stats_num, p_sys_statsid);
        if (ret != CTC_E_NONE)
        {
            goto error_roll_1;
        }
        ret = _sys_usw_flow_stats_entry_create(lchip, stats_num, p_sys_statsid->stats_ptr, SYS_STATS_PTR_FLAG_HAVE_STATSID);
        if (ret != CTC_E_NONE)
        {
            goto error_roll_2;
        }
    }
    else
    {
        if (!SYS_STATS_TYPE_ENABLE(CTC_STATS_ECMP_STATS))
        {
            SYS_STATS_DBG_ERROR(" Ecmp stats not support \n");
            ret = CTC_E_NOT_SUPPORT;
            goto error_roll_1;
        }
        entry_num = 0;
        CTC_ERROR_GOTO((sys_usw_nh_get_max_ecmp_group_num(lchip, &entry_num)),ret, error_roll_1);
        p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_ECMP]);
        if(p_prop->used_cnt < entry_num && p_prop->ram_bmp)
        {
            p_prop->used_cnt++;
        }
        else
        {
            ret = CTC_E_NO_RESOURCE;
            goto error_roll_1;
        }
    }

    if (p_sys_statsid->stats_ptr)
    {
        _sys_usw_flow_stats_clear_stats(lchip, stats_num, p_sys_statsid->stats_ptr);
    }

    /*statsid and stats sw*/
    ctc_hash_insert(usw_flow_stats_master[lchip]->flow_stats_id_hash, p_sys_statsid);

    return ret;

error_roll_2:
    _sys_usw_flow_stats_free_statsptr(lchip, stats_num, p_sys_statsid);
error_roll_1:
    _sys_usw_flow_stats_free_statsid(lchip, p_statsid->stats_id);
error_roll_0:
    mem_free(p_sys_statsid);
    return ret;
}

STATIC int32
_sys_usw_flow_stats_destroy_statsid(uint8 lchip, sys_stats_statsid_t* p_statsid)
{
    uint16 stats_num = 0;
    sys_stats_statsid_t* p_statsid_rse = NULL;

    SYS_STATS_DBG_FUNC();

    p_statsid_rse = ctc_hash_lookup(usw_flow_stats_master[lchip]->flow_stats_id_hash, p_statsid);
    if (p_statsid_rse == NULL)
    {
        SYS_STATS_DBG_ERROR(" Entry not exist \n");
		return CTC_E_NOT_EXIST;
    }

    if (p_statsid_rse->stats_ptr)
    {
        _sys_usw_flow_stats_clear_stats(lchip, stats_num, p_statsid_rse->stats_ptr);
    }

    if (SYS_STATS_TYPE_ECMP != p_statsid_rse->stats_id_type)
    {
        stats_num = (p_statsid_rse->color_aware ? 3: 1);
        CTC_ERROR_RETURN(_sys_usw_flow_stats_free_statsptr(lchip, stats_num, p_statsid_rse));
        CTC_ERROR_RETURN(_sys_usw_flow_stats_entry_delete(lchip, stats_num, p_statsid_rse->stats_ptr));
    }
    else
    {
        usw_flow_stats_master[lchip]->stats_type[p_statsid->dir][SYS_STATS_TYPE_ECMP].used_cnt--;
    }

    ctc_hash_remove(usw_flow_stats_master[lchip]->flow_stats_id_hash, p_statsid_rse);
    mem_free(p_statsid_rse);

    CTC_ERROR_RETURN(_sys_usw_flow_stats_free_statsid(lchip, p_statsid->stats_id));

    return CTC_E_NONE;
}

#define __7_STATS_WB__
STATIC int32
_sys_usw_flow_stats_wb_sync_statsid(sys_stats_statsid_t *p_statsid, void *user_data)
{
    uint16 max_entry_cnt = 0;
    sys_wb_stats_statsid_t* p_wb_statsid = NULL;
    sys_traverse_t *data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)(data->data);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_statsid = (sys_wb_stats_statsid_t *)wb_data->buffer + wb_data->valid_cnt;
    sal_memset(p_wb_statsid, 0, sizeof(sys_wb_stats_statsid_t));

    p_wb_statsid->stats_id = p_statsid->stats_id;
    p_wb_statsid->stats_ptr = p_statsid->stats_ptr;
    p_wb_statsid->hw_stats_ptr = p_statsid->hw_stats_ptr;
    p_wb_statsid->stats_id_type = p_statsid->stats_id_type;
    p_wb_statsid->dir = p_statsid->dir;
    p_wb_statsid->color_aware = p_statsid->color_aware;
    p_wb_statsid->acl_priority = p_statsid->u.acl_priority;
    p_wb_statsid->is_vc_label = p_statsid->u.is_vc_label;

    wb_data->valid_cnt++;

    if (wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
        sal_memset(wb_data->buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_flow_stats_wb_sync_statsptr(sys_stats_flow_stats_t *p_stats, void *user_data)
{
    uint16 max_entry_cnt = 0;
    sys_wb_stats_statsptr_t* p_wb_statsptr = NULL;
    sys_traverse_t *data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)(data->data);

    if ((!CTC_FLAG_ISSET(p_stats->flag, SYS_STATS_PTR_FLAG_HAVE_STATSID))
        && (!CTC_FLAG_ISSET(p_stats->flag, SYS_STATS_PTR_FLAG_CREATE_ON_INIT)))
    {
        max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

        p_wb_statsptr = (sys_wb_stats_statsptr_t *)wb_data->buffer + wb_data->valid_cnt;
        sal_memset(p_wb_statsptr, 0, sizeof(sys_wb_stats_statsptr_t));

        p_wb_statsptr->stats_ptr = p_stats->stats_ptr;

        wb_data->valid_cnt++;

        if (wb_data->valid_cnt == max_entry_cnt)
        {
            CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
            wb_data->valid_cnt = 0;
            sal_memset(wb_data->buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_flow_stats_wb_sync(uint8 lchip)
{
    uint32 type = 0;
    uint32 ram = 0;
    uint8 dir = CTC_INGRESS;
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_traverse_t user_data;
    sys_wb_stats_master_t  *p_wb_stats_master = NULL;
    uint8 work_status = 0;

    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(work_status == CTC_FTM_MEM_CHANGE_RECOVER)
	{
		return CTC_E_NONE;
	}
    /*syncup buffer*/
    wb_data.buffer = mem_malloc(MEM_STATS_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    STATS_LOCK();

    /*syncup master*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_stats_master_t, CTC_FEATURE_STATS, SYS_WB_APPID_STATS_SUBID_MASTER);
    p_wb_stats_master = (sys_wb_stats_master_t  *)wb_data.buffer;
    p_wb_stats_master->lchip = lchip;
    p_wb_stats_master->version = SYS_WB_VERSION_STATS;

    for (dir = 0; dir < CTC_BOTH_DIRECTION; dir++)
    {
        for ( type = 0; type < SYS_STATS_TYPE_MAX; type++)
        {
            for ( ram = 0; ram < SYS_STATS_RAM_MAX; ram++)
            {
                p_wb_stats_master->stats_type[dir][type].used_cnt_ram[ram] = usw_flow_stats_master[lchip]->stats_type[dir][type].used_cnt_ram[ram];
            }
            p_wb_stats_master->stats_type[dir][type].used_cnt = usw_flow_stats_master[lchip]->stats_type[dir][type].used_cnt;
            p_wb_stats_master->stats_type[dir][type].ram_bmp = usw_flow_stats_master[lchip]->stats_type[dir][type].ram_bmp;
        }
    }

    for ( ram = 0; ram < SYS_STATS_RAM_MAX; ram++)
    {
        p_wb_stats_master->stats_ram[ram].used_cnt = usw_flow_stats_master[lchip]->stats_ram[ram].used_cnt;
        p_wb_stats_master->stats_ram[ram].stats_bmp = usw_flow_stats_master[lchip]->stats_ram[ram].stats_bmp;
        p_wb_stats_master->stats_ram[ram].acl_priority = usw_flow_stats_master[lchip]->stats_ram[ram].acl_priority;
    }

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    /*syncup statsid*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_stats_statsid_t, CTC_FEATURE_STATS, SYS_WB_APPID_STATS_SUBID_STATSID);
    sal_memset(&user_data, 0, sizeof(user_data));
    user_data.data = &wb_data;
    user_data.value1 = lchip;
    wb_data.valid_cnt = 0;
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_ERROR_GOTO(ctc_hash_traverse(usw_flow_stats_master[lchip]->flow_stats_id_hash, (hash_traversal_fn) _sys_usw_flow_stats_wb_sync_statsid, (void *)&user_data), ret, done);
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
        wb_data.valid_cnt = 0;
    }

    /*syncup statsptr*/
    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_stats_statsptr_t, CTC_FEATURE_STATS, SYS_WB_APPID_STATS_SUBID_STATSPTR);
    sal_memset(&user_data, 0, sizeof(user_data));
    user_data.data = &wb_data;
    user_data.value1 = lchip;
    wb_data.valid_cnt = 0;
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_ERROR_GOTO(ctc_hash_traverse(usw_flow_stats_master[lchip]->flow_stats_hash, (hash_traversal_fn) _sys_usw_flow_stats_wb_sync_statsptr, (void *)&user_data), ret, done);
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

    STATS_UNLOCK();

    return ret;
}

STATIC int32
_sys_usw_flow_stats_wb_restore_create_statsid_ptr(uint8 lchip, sys_stats_statsid_t* p_sys_statsid)
{
    int32 ret = CTC_E_NONE;
    sys_usw_opf_t opf;
    uint16 stats_num = 0;
    uint32 offset = 0;

    stats_num = (p_sys_statsid->color_aware ? 3: 1);
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = usw_flow_stats_master[lchip]->opf_type_flow_stats;
    opf.pool_index = SYS_STATS_DECODE_PTR_RAM(p_sys_statsid->stats_ptr);
    offset = SYS_STATS_DECODE_PTR_OFFSET(p_sys_statsid->stats_ptr);

    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, stats_num, offset));

    ret = _sys_usw_flow_stats_entry_create(lchip, stats_num, p_sys_statsid->stats_ptr, SYS_STATS_PTR_FLAG_HAVE_STATSID);
    if (ret)
    {
        sys_usw_opf_free_offset(lchip, &opf, stats_num, offset);
    }
    else
    {
        usw_flow_stats_master[lchip]->alloc_count += stats_num;
    }

    return ret;
}

STATIC int32
_sys_usw_flow_stats_wb_restore_create_statsid(uint8 lchip, sys_stats_statsid_t* p_sys_statsid)
{
    int32 ret = CTC_E_NONE;
    sys_usw_opf_t opf;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = usw_flow_stats_master[lchip]->opf_type_stats_id;

    /*statsid maybe alloc*/
    if (usw_flow_stats_master[lchip]->stats_mode != CTC_STATS_MODE_USER)
    {
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, p_sys_statsid->stats_id));
    }

    /* stats_ptr from sys_usw_flow_stats_add_ecmp_stats which called by nexthop */
    if (SYS_STATS_TYPE_ECMP != p_sys_statsid->stats_id_type)
    {
        ret = _sys_usw_flow_stats_wb_restore_create_statsid_ptr(lchip, p_sys_statsid);
        if (ret != CTC_E_NONE)
        {
            if (usw_flow_stats_master[lchip]->stats_mode == CTC_STATS_MODE_DEFINE)
            {
                sys_usw_opf_free_offset(lchip, &opf, 1, p_sys_statsid->stats_id);
            }

            return ret;
        }
    }
    else
    {
        if (p_sys_statsid->stats_ptr)
        {
            CTC_ERROR_RETURN(_sys_usw_flow_stats_entry_create(lchip, 1, p_sys_statsid->stats_ptr, SYS_STATS_PTR_FLAG_HAVE_STATSID));
        }
    }

    ctc_hash_insert(usw_flow_stats_master[lchip]->flow_stats_id_hash, p_sys_statsid);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_flow_stats_wb_restore_statsid(uint8 lchip, ctc_wb_query_t* p_wb_query, uint32 version)
{
    int32 ret = CTC_E_NONE;
    uint16 entry_cnt = 0;
    sys_wb_stats_statsid_t wb_statsid;
    sys_stats_statsid_t* p_sys_statsid = NULL;

    /*set to default value*/
    sal_memset(&wb_statsid, 0, sizeof(sys_wb_stats_statsid_t));

    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_stats_statsid_t, CTC_FEATURE_STATS, SYS_WB_APPID_STATS_SUBID_STATSID);
    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);
    {
        sal_memcpy(&wb_statsid, (uint8*)(p_wb_query->buffer)+entry_cnt*(p_wb_query->key_len + p_wb_query->data_len), p_wb_query->key_len + p_wb_query->data_len);

        entry_cnt++;

        p_sys_statsid = mem_malloc(MEM_STATS_MODULE,  sizeof(sys_stats_statsid_t));
        if (p_sys_statsid == NULL)
        {
            SYS_STATS_DBG_ERROR(" [Stats] Stats wb restore statsid failed \n");
            continue;
        }
        sal_memset(p_sys_statsid, 0, sizeof(sys_stats_statsid_t));

        p_sys_statsid->stats_id = wb_statsid.stats_id;
        p_sys_statsid->stats_id_type = wb_statsid.stats_id_type;
        p_sys_statsid->stats_ptr = wb_statsid.stats_ptr;
        p_sys_statsid->hw_stats_ptr = wb_statsid.hw_stats_ptr;
        p_sys_statsid->dir = wb_statsid.dir;
        p_sys_statsid->color_aware = wb_statsid.color_aware;
        p_sys_statsid->u.acl_priority = wb_statsid.acl_priority;
        p_sys_statsid->u.is_vc_label = wb_statsid.is_vc_label;

        ret = _sys_usw_flow_stats_wb_restore_create_statsid(lchip, p_sys_statsid);
        if (ret)
        {
            mem_free(p_sys_statsid);
            SYS_STATS_DBG_ERROR(" [Stats] Stats wb restore statsid failed \n");
            continue;
        }
    }
    CTC_WB_QUERY_ENTRY_END(p_wb_query);

done:

    return ret;
}

STATIC int32
_sys_usw_flow_stats_wb_restore_statsptr(uint8 lchip, ctc_wb_query_t* p_wb_query, uint32 version)
{
    int32 ret = CTC_E_NONE;
    uint16 entry_cnt = 0;
    uint32 offset = 0;
    sys_usw_opf_t opf;
    sys_wb_stats_statsptr_t wb_statsptr;

    SYS_STATS_DBG_FUNC();
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = usw_flow_stats_master[lchip]->opf_type_flow_stats;

    /*set to default value*/
    sal_memset(&wb_statsptr, 0, sizeof(sys_wb_stats_statsptr_t));

    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_stats_statsptr_t, CTC_FEATURE_STATS, SYS_WB_APPID_STATS_SUBID_STATSPTR);
    CTC_WB_QUERY_ENTRY_BEGIN(p_wb_query);
    {
        sal_memcpy(&wb_statsptr, (uint8*)(p_wb_query->buffer)+entry_cnt*(p_wb_query->key_len + p_wb_query->data_len), p_wb_query->key_len + p_wb_query->data_len);

        entry_cnt++;

        opf.pool_index = SYS_STATS_DECODE_PTR_RAM(wb_statsptr.stats_ptr);
        offset = SYS_STATS_DECODE_PTR_OFFSET(wb_statsptr.stats_ptr);
        ret = sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, offset);
        if (ret != CTC_E_NONE)
        {
            SYS_STATS_DBG_ERROR(" [Stats] Stats wb restore statsptr failed \n");
            continue;
        }

        ret = _sys_usw_flow_stats_entry_create(lchip, 1, wb_statsptr.stats_ptr, 0);
        if (ret)
        {
            sys_usw_opf_free_offset(lchip, &opf, 1, offset);
            SYS_STATS_DBG_ERROR(" [Stats] Stats wb restore statsptr failed \n");
            continue;
        }
        usw_flow_stats_master[lchip]->alloc_count++;
    }
    CTC_WB_QUERY_ENTRY_END(p_wb_query);
done:

    return ret;
}

STATIC int32
_sys_usw_flow_stats_wb_restore_master(uint8 lchip, ctc_wb_query_t* p_wb_query, uint32* version)
{
    uint32 type = 0;
    uint32 ram = 0;
    uint8 dir = CTC_INGRESS;
    int32 ret = CTC_E_NONE;
    sys_wb_stats_master_t* p_wb_stats_master = NULL;

    p_wb_stats_master = (sys_wb_stats_master_t*)mem_malloc(MEM_STATS_MODULE, sizeof(sys_wb_stats_master_t));
    if (NULL == p_wb_stats_master)
    {
        return CTC_E_NO_MEMORY;
    }
    CTC_WB_INIT_QUERY_T(p_wb_query, sys_wb_stats_master_t, CTC_FEATURE_STATS, SYS_WB_APPID_STATS_SUBID_MASTER);
    CTC_ERROR_GOTO(ctc_wb_query_entry(p_wb_query), ret, done);
    if ((p_wb_query->valid_cnt != 1) || (p_wb_query->is_end != 1))
    {
        CTC_WB_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "query stats master error! valid_cnt: %d, is_end: %d.\n", p_wb_query->valid_cnt, p_wb_query->is_end);
        ret = CTC_E_NONE;
        goto done;
    }

    sal_memset(p_wb_stats_master, 0, sizeof(sys_wb_stats_master_t));
    sal_memcpy(p_wb_stats_master, p_wb_query->buffer, p_wb_query->key_len + p_wb_query->data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_STATS, p_wb_stats_master->version))
    {
        CTC_ERROR_GOTO(CTC_E_VERSION_MISMATCH, ret, done);
    }

    *version = p_wb_stats_master->version;

    for (dir = 0; dir < CTC_BOTH_DIRECTION; dir++)
    {
        for ( type = 0; type < SYS_STATS_TYPE_MAX; type++)
        {
            for ( ram = 0; ram < SYS_STATS_RAM_MAX; ram++)
            {
                usw_flow_stats_master[lchip]->stats_type[dir][type].used_cnt_ram[ram] = p_wb_stats_master->stats_type[dir][type].used_cnt_ram[ram];
            }
            usw_flow_stats_master[lchip]->stats_type[dir][type].used_cnt = p_wb_stats_master->stats_type[dir][type].used_cnt;
            usw_flow_stats_master[lchip]->stats_type[dir][type].ram_bmp = p_wb_stats_master->stats_type[dir][type].ram_bmp;
        }
    }

    for ( ram = 0; ram < SYS_STATS_RAM_MAX; ram++)
    {
        usw_flow_stats_master[lchip]->stats_ram[ram].used_cnt = p_wb_stats_master->stats_ram[ram].used_cnt;
        usw_flow_stats_master[lchip]->stats_ram[ram].stats_bmp = p_wb_stats_master->stats_ram[ram].stats_bmp;
        usw_flow_stats_master[lchip]->stats_ram[ram].acl_priority = p_wb_stats_master->stats_ram[ram].acl_priority;
    }

done:
    if (p_wb_stats_master)
    {
        mem_free(p_wb_stats_master);
    }

    return ret;
}

STATIC int32
_sys_usw_flow_stats_wb_restore(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t wb_query;
    uint32 version = 0;

    SYS_STATS_DBG_FUNC();
    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_STATS_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        CTC_ERROR_GOTO(CTC_E_NO_MEMORY, ret, done);
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    STATS_LOCK();

    /*restore  master*/
    CTC_ERROR_GOTO(_sys_usw_flow_stats_wb_restore_master(lchip, &wb_query, &version), ret, done);

    /*restore  statsid*/
    CTC_ERROR_GOTO(_sys_usw_flow_stats_wb_restore_statsid(lchip, &wb_query, version), ret, done);

    /*restore  statsptr*/
    CTC_ERROR_GOTO(_sys_usw_flow_stats_wb_restore_statsptr(lchip, &wb_query, version), ret, done);

    SYS_STATS_DBG_INFO( "restore stats ptr alloced num:[%u]\n",usw_flow_stats_master[lchip]->alloc_count);

done:
    if (wb_query.key)
    {
        mem_free(wb_query.key);
    }

    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    STATS_UNLOCK();

    return ret;
}

#define __8_STATS_INIT_FUNC__

STATIC int32
_sys_usw_flow_stats_statsid_init(uint8 lchip)
{
    sys_usw_opf_t opf;
    uint32 start_offset = 0;
    uint32 entry_num    = 0;

    usw_flow_stats_master[lchip]->flow_stats_id_hash = ctc_hash_create(1,
                                        SYS_STATS_FLOW_ENTRY_HASH_BLOCK_SIZE,
                                        (hash_key_fn) _sys_usw_flow_stats_id_hash_key_make,
                                        (hash_cmp_fn) _sys_usw_flow_stats_id_hash_key_cmp);

    if (usw_flow_stats_master[lchip]->stats_mode == CTC_STATS_MODE_DEFINE)
    {
        sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

        start_offset  = 1;
        entry_num     = CTC_STATS_MAX_STATSID;
        CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &usw_flow_stats_master[lchip]->opf_type_stats_id, 1, "opf-type-stats-id"));
        opf.pool_type = usw_flow_stats_master[lchip]->opf_type_stats_id;
        CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, start_offset, (entry_num-1)));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_flow_stats_global_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    ds_t   ds;

    field_val = 1;
    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_mcNexthopPtrAsStatsPtrEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    field_val = 1;
    cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_mcNexthopPtrAsStatsPtrEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /* After byte/packet reach the threshold, the ram data move to DsStats */

    SetStatsUpdateThrdCtl(V, byteThreshold_f,   &ds, SYS_STATS_CHACHE_UPDATE_BYTE_THRESHOLD);
    SetStatsUpdateThrdCtl(V, packetThreshold_f, &ds, SYS_STATS_CHACHE_UPDATE_PKT_THRESHOLD);

    cmd = DRV_IOW(StatsUpdateThrdCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds));

    SetGlobalStatsSatuInterruptThreshold(V, byteCntThresholdLo_f,         &ds, 0);
    SetGlobalStatsSatuInterruptThreshold(V, byteCntThresholdHi_f,         &ds, 0);
    SetGlobalStatsSatuInterruptThreshold(V, pktCntThresholdLo_f,          &ds, 0);
    SetGlobalStatsSatuInterruptThreshold(V, pktCntThresholdHi_f,          &ds, 0);
    SetGlobalStatsSatuInterruptThreshold(V, satuAddrFifoDepthThreshold_f, &ds, 0);

    cmd = DRV_IOW(GlobalStatsSatuInterruptThreshold_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ds));

    field_val = 1;
    cmd = DRV_IOW(GlobalStatsUpdateEnCtl_t, GlobalStatsUpdateEnCtl_updateEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    CTC_ERROR_RETURN(sys_usw_flow_stats_set_drop_packet_stats_en(lchip, CTC_STATS_FLOW_DISCARD_STATS, TRUE));
    CTC_ERROR_RETURN(sys_usw_flow_stats_set_saturate_en(lchip, CTC_STATS_TYPE_FWD, TRUE));
    CTC_ERROR_RETURN(sys_usw_flow_stats_set_clear_after_read_en(lchip, CTC_STATS_TYPE_FWD, TRUE));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_flow_stats_type_init(uint8 lchip)
{
    sys_stats_prop_t* p_prop = NULL;

    /*SYS_STATS_TYPE_QUEUE*/
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_QUEUE]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_QUEUE);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_QUEUE].stats_bmp, SYS_STATS_TYPE_QUEUE);

    /*SYS_STATS_TYPE_VLAN*/
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_VLAN]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL1);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL1].stats_bmp, SYS_STATS_TYPE_VLAN);
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_EGRESS][SYS_STATS_TYPE_VLAN]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_EPE_GLOBAL3);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL3].stats_bmp, SYS_STATS_TYPE_VLAN);

    /*SYS_STATS_TYPE_INTF*/
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_INTF]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL2);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL2].stats_bmp, SYS_STATS_TYPE_INTF);
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_EGRESS][SYS_STATS_TYPE_INTF]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_EPE_GLOBAL3);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL3].stats_bmp, SYS_STATS_TYPE_INTF);

    /*SYS_STATS_TYPE_VRF*/
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_VRF]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL1);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL1].stats_bmp, SYS_STATS_TYPE_VRF);

    /*SYS_STATS_TYPE_SCL*/
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_SCL]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL1);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL1].stats_bmp, SYS_STATS_TYPE_SCL);
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_EGRESS][SYS_STATS_TYPE_SCL]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_EPE_GLOBAL1);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL1].stats_bmp, SYS_STATS_TYPE_SCL);

    /*SYS_STATS_TYPE_TUNNEL*/
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_TUNNEL]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL1);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL1].stats_bmp, SYS_STATS_TYPE_TUNNEL);
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_EGRESS][SYS_STATS_TYPE_TUNNEL]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_EPE_GLOBAL1);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL1].stats_bmp, SYS_STATS_TYPE_TUNNEL);

    /*SYS_STATS_TYPE_LSP*/
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_LSP]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL1);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL1].stats_bmp, SYS_STATS_TYPE_LSP);
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_EGRESS][SYS_STATS_TYPE_LSP]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_EPE_GLOBAL1);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL1].stats_bmp, SYS_STATS_TYPE_LSP);

    /*SYS_STATS_TYPE_PW*/
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_PW]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL2);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL2].stats_bmp, SYS_STATS_TYPE_PW);
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_EGRESS][SYS_STATS_TYPE_PW]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_EPE_GLOBAL2);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL2].stats_bmp, SYS_STATS_TYPE_PW);

    /*SYS_STATS_TYPE_ACL*/
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_ACL]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_PRIVATE0);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_PRIVATE1);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_PRIVATE2);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_PRIVATE3);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL0);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL1);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL2);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL3);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_PRIVATE0].stats_bmp, SYS_STATS_TYPE_ACL);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_PRIVATE1].stats_bmp, SYS_STATS_TYPE_ACL);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_PRIVATE2].stats_bmp, SYS_STATS_TYPE_ACL);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_PRIVATE3].stats_bmp, SYS_STATS_TYPE_ACL);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL0].stats_bmp, SYS_STATS_TYPE_ACL);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL1].stats_bmp, SYS_STATS_TYPE_ACL);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL2].stats_bmp, SYS_STATS_TYPE_ACL);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL3].stats_bmp, SYS_STATS_TYPE_ACL);
    usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_PRIVATE0].acl_priority = 0;
    usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_PRIVATE1].acl_priority = 1;
    usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_PRIVATE2].acl_priority = 2;
    usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_PRIVATE3].acl_priority = 3;
    usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL0].acl_priority = 4;
    usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL1].acl_priority = 5;
    usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL2].acl_priority = 6;
    usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL3].acl_priority = 7;
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_EGRESS][SYS_STATS_TYPE_ACL]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_EPE_PRIVATE0);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_EPE_GLOBAL0);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_EPE_GLOBAL1);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_PRIVATE0].stats_bmp, SYS_STATS_TYPE_ACL);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL0].stats_bmp, SYS_STATS_TYPE_ACL);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL1].stats_bmp, SYS_STATS_TYPE_ACL);
    usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_PRIVATE0].acl_priority = 0;
    usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL0].acl_priority = 1;
    usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL1].acl_priority = 2;

    /*SYS_STATS_TYPE_FWD_NH*/
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_FWD_NH]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL3);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL3].stats_bmp, SYS_STATS_TYPE_FWD_NH);
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_EGRESS][SYS_STATS_TYPE_FWD_NH]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_EPE_GLOBAL2);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL2].stats_bmp, SYS_STATS_TYPE_FWD_NH);

    /*SYS_STATS_TYPE_FIB*/
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_FIB]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL1);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL1].stats_bmp, SYS_STATS_TYPE_FIB);

    /*SYS_STATS_TYPE_FLOW_HASH*/
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_FLOW_HASH]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL2);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL2].stats_bmp, SYS_STATS_TYPE_FLOW_HASH);

    /*SYS_STATS_TYPE_MAC*/
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_MAC]);
        CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL3);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL3].stats_bmp, SYS_STATS_TYPE_MAC);


    /*SYS_STATS_TYPE_POLICER0*/
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_POLICER0]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL0);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL0].stats_bmp, SYS_STATS_TYPE_POLICER0);

    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_EGRESS][SYS_STATS_TYPE_POLICER0]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_EPE_GLOBAL0);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL0].stats_bmp, SYS_STATS_TYPE_POLICER0);


    /*SYS_STATS_TYPE_POLICER1*/
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_POLICER1]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL1);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL1].stats_bmp, SYS_STATS_TYPE_POLICER1);
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_EGRESS][SYS_STATS_TYPE_POLICER1]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_EPE_GLOBAL1);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL1].stats_bmp, SYS_STATS_TYPE_POLICER1);

    if (SYS_STATS_TYPE_ENABLE(CTC_STATS_ECMP_STATS))
    {
        /*SYS_STATS_TYPE_ECMP*/
        p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_ECMP]);
        CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL3);
        CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL3].stats_bmp, SYS_STATS_TYPE_ECMP);
    }

    if (SYS_STATS_TYPE_ENABLE(CTC_STATS_PORT_STATS))
    {
        /*SYS_STATS_TYPE_PORT*/
        p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_PORT]);
        CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_IPE_GLOBAL3);
        CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL3].stats_bmp, SYS_STATS_TYPE_PORT);

        /*SYS_STATS_TYPE_PORT*/
        p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_EGRESS][SYS_STATS_TYPE_PORT]);
        CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_EPE_GLOBAL1);
        CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL1].stats_bmp, SYS_STATS_TYPE_PORT);
    }
    /*SYS_STATS_TYPE_SDC*/
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_SDC]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_SDC);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_SDC].stats_bmp, SYS_STATS_TYPE_SDC);
    p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_EGRESS][SYS_STATS_TYPE_SDC]);
    CTC_BIT_SET(p_prop->ram_bmp, SYS_STATS_RAM_SDC);
    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_SDC].stats_bmp, SYS_STATS_TYPE_SDC);

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_flow_stats_alloc_solid_type(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 r = 0;
    uint32 stats_ptr = 0;
    uint32 offset = 0;
    sys_stats_prop_t* p_prop = NULL;
    sys_stats_ram_t* p_ram = NULL;
    sys_usw_opf_t opf;
    uint16 entry_num = 0;
    uint8 dir = 0;
    uint32 value = 0;
    uint8 start_r = 0, end_r = 0;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = usw_flow_stats_master[lchip]->opf_type_flow_stats;
    if (SYS_STATS_TYPE_ENABLE(CTC_STATS_ECMP_STATS))
    {
        /* ecmp stats alloc */
        p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_ECMP]);
        entry_num = 0;
        CTC_ERROR_RETURN(sys_usw_nh_get_max_ecmp_group_num(lchip, &entry_num));
        for (r = 0; r < SYS_STATS_RAM_MAX; r++)
        {
            p_ram = &(usw_flow_stats_master[lchip]->stats_ram[r]);
            if (CTC_IS_BIT_SET(p_ram->stats_bmp, SYS_STATS_TYPE_ECMP))
            {
                opf.pool_index = r;
                CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, entry_num, &offset));

                cmd = DRV_IOW(IpeEcmpCtl_t, IpeEcmpCtl_ecmpStatsOffset_f);
                stats_ptr = SYS_STATS_ENCODE_CHIP_PTR(r, offset - 1);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &stats_ptr));
                usw_flow_stats_master[lchip]->ecmp_base_ptr = SYS_STATS_ENCODE_SYS_PTR(r, offset - 1);
                p_prop->used_cnt_ram[r] += entry_num;
                p_ram->used_cnt += entry_num;
                usw_flow_stats_master[lchip]->alloc_count += entry_num;
                break;
            }
        }
    }

    if (SYS_STATS_TYPE_ENABLE(CTC_STATS_PORT_STATS))
    {
        /* port stats alloc */
        for (dir = 0; dir < CTC_BOTH_DIRECTION; dir++)
        {
            p_prop = &(usw_flow_stats_master[lchip]->stats_type[dir][SYS_STATS_TYPE_PORT]);

            if (dir == CTC_INGRESS)
            {
                start_r = SYS_STATS_RAM_IPE_GLOBAL0;
                end_r = SYS_STATS_RAM_IPE_GLOBAL3;
            }
            else
            {
                start_r = SYS_STATS_RAM_EPE_GLOBAL0;
                end_r = SYS_STATS_RAM_EPE_GLOBAL3;
            }


            for (r = start_r; r <= end_r; r++)
            {
                p_ram = &(usw_flow_stats_master[lchip]->stats_ram[r]);

                if (CTC_IS_BIT_SET(p_ram->stats_bmp, SYS_STATS_TYPE_PORT))
                {
                    value = 1;
                    if (dir == CTC_INGRESS)
                    {
                        cmd = DRV_IOW(IpeFwdStatsCtl_t, IpeFwdStatsCtl_portStatsEn_f);
                        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                        cmd = DRV_IOW(IpeFwdStatsCtl_t, IpeFwdStatsCtl_portStatsPtrBase_f);
                    }
                    else
                    {
                        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_portStatsEn_f);
                        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
                        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_portStatsPtrBase_f);
                    }

                    opf.pool_index = r;
                    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, SYS_USW_MAX_PORT_NUM_PER_CHIP, &offset)); /* only one ram?*/

                    stats_ptr = SYS_STATS_ENCODE_CHIP_PTR(r, offset);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &stats_ptr));

                    stats_ptr = SYS_STATS_ENCODE_SYS_PTR(r, offset);
                    usw_flow_stats_master[lchip]->port_base_ptr[dir] = SYS_STATS_ENCODE_SYS_PTR(r, stats_ptr);
                    CTC_ERROR_RETURN(_sys_usw_flow_stats_entry_create(lchip, SYS_USW_MAX_PORT_NUM_PER_CHIP, stats_ptr, SYS_STATS_PTR_FLAG_CREATE_ON_INIT));

                    p_prop->used_cnt_ram[r] += SYS_USW_MAX_PORT_NUM_PER_CHIP;
                    p_ram->used_cnt += SYS_USW_MAX_PORT_NUM_PER_CHIP;
                    usw_flow_stats_master[lchip]->alloc_count += SYS_USW_MAX_PORT_NUM_PER_CHIP;
                    usw_flow_stats_master[lchip]->stats_type[dir][SYS_STATS_TYPE_PORT].used_cnt += SYS_USW_MAX_PORT_NUM_PER_CHIP;
                    break;
                }
            }
        }

    }
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_flow_stats_free_solid_type(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 r = 0;
    uint32 stats_ptr = 0;
    uint32 offset = 0;
    sys_stats_prop_t* p_prop = NULL;
    sys_stats_ram_t* p_ram = NULL;
    sys_usw_opf_t opf;
    uint16 entry_num = 0;
    uint8 dir = 0;
    uint32 value = 0;
    uint8 start_r = 0, end_r = 0;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = usw_flow_stats_master[lchip]->opf_type_flow_stats;
    /* ecmp stats free */
    if (SYS_STATS_TYPE_ENABLE(CTC_STATS_ECMP_STATS))
    {
        p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_ECMP]);
        entry_num = 0;
        CTC_ERROR_RETURN(sys_usw_nh_get_max_ecmp_group_num(lchip, &entry_num));
        for (r = 0; r < SYS_STATS_RAM_MAX; r++)
        {
            p_ram = &(usw_flow_stats_master[lchip]->stats_ram[r]);
            if (CTC_IS_BIT_SET(p_ram->stats_bmp, SYS_STATS_TYPE_ECMP))
            {
                opf.pool_index = r;
                offset = SYS_STATS_DECODE_PTR_OFFSET(usw_flow_stats_master[lchip]->ecmp_base_ptr) + 1;
                CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, entry_num, offset));

                cmd = DRV_IOW(IpeEcmpCtl_t, IpeEcmpCtl_ecmpStatsOffset_f);
                stats_ptr = 0;
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &stats_ptr));
                usw_flow_stats_master[lchip]->ecmp_base_ptr = 0;
                p_prop->used_cnt_ram[r] -= entry_num;
                p_ram->used_cnt -= entry_num;
                usw_flow_stats_master[lchip]->alloc_count -= entry_num;

                break;
            }
        }
    }

    if (SYS_STATS_TYPE_ENABLE(CTC_STATS_PORT_STATS))
    {
        /* port stats free */
        for (dir = 0; dir < CTC_BOTH_DIRECTION; dir++)
        {
            p_prop = &(usw_flow_stats_master[lchip]->stats_type[dir][SYS_STATS_TYPE_PORT]);
            value = 0;
            if (dir == CTC_INGRESS)
            {
                cmd = DRV_IOW(IpeFwdStatsCtl_t, IpeFwdStatsCtl_portStatsEn_f);
                start_r = SYS_STATS_RAM_IPE_GLOBAL0;
                end_r = SYS_STATS_RAM_IPE_GLOBAL3;
            }
            else
            {
                cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_portStatsEn_f);
                start_r = SYS_STATS_RAM_EPE_GLOBAL0;
                end_r = SYS_STATS_RAM_EPE_GLOBAL3;
            }
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

            for (r = start_r; r <= end_r; r++)
            {
                p_ram = &(usw_flow_stats_master[lchip]->stats_ram[r]);

                if (CTC_IS_BIT_SET(p_ram->stats_bmp, SYS_STATS_TYPE_PORT))
                {
                    if (dir == CTC_INGRESS)
                    {
                        cmd = DRV_IOW(IpeFwdStatsCtl_t, IpeFwdStatsCtl_portStatsPtrBase_f);
                    }
                    else
                    {
                        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_portStatsPtrBase_f);
                    }

                    opf.pool_index = r;
                    offset = SYS_STATS_DECODE_PTR_OFFSET(usw_flow_stats_master[lchip]->port_base_ptr[dir]);
                    CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, SYS_USW_MAX_PORT_NUM_PER_CHIP, offset));

                    stats_ptr = 0;
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &stats_ptr));

                    usw_flow_stats_master[lchip]->port_base_ptr[dir] = 0;

                    stats_ptr = SYS_STATS_ENCODE_SYS_PTR(r, offset);
                    CTC_ERROR_RETURN(_sys_usw_flow_stats_entry_delete(lchip, SYS_USW_MAX_PORT_NUM_PER_CHIP, stats_ptr));

                    p_prop->used_cnt_ram[r] -= SYS_USW_MAX_PORT_NUM_PER_CHIP;
                    p_prop->used_cnt -= SYS_USW_MAX_PORT_NUM_PER_CHIP;
                    p_ram->used_cnt -= SYS_USW_MAX_PORT_NUM_PER_CHIP;
                    usw_flow_stats_master[lchip]->alloc_count -= SYS_USW_MAX_PORT_NUM_PER_CHIP;
                    break;
                }
            }
        }
    }

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_flow_stats_ram_alloc(uint8 lchip)
{
    uint32 base = 0;
    uint32 cmd = 0;
    uint32 r = 0;
    uint32 stats_ptr = 0;
    uint32 offset = 0;
    uint32 total = 0;
    sys_stats_prop_t* p_prop = NULL;
    sys_stats_ram_t* p_ram = NULL;
    sys_usw_opf_t opf;
    uint32 entry_num = 0;

    SYS_STATS_DBG_FUNC();

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));

    /* global/private stats pool init */
    CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &usw_flow_stats_master[lchip]->opf_type_flow_stats,  SYS_STATS_RAM_MAX,"opf-type-flow-stats"));

    opf.pool_type = usw_flow_stats_master[lchip]->opf_type_flow_stats;

    for (r = 0; r < SYS_STATS_RAM_MAX; r++)
    {
        p_ram = &(usw_flow_stats_master[lchip]->stats_ram[r]);

        opf.pool_index = r;

        if (p_ram->total_cnt > 0)
        {
            /*SDC do not need cache*/
            if (r != SYS_STATS_RAM_SDC)
            {
                cmd = DRV_IOW(StatsCacheBasePtr_t, p_ram->base_field_id);
                base = (total>0 ? total-1: 0);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &base));
                p_ram->base_idx = base;
                p_ram->base_ptr = SYS_STATS_ENCODE_SYS_PTR(r, 0);

                SYS_STATS_DBG_INFO( "ram:%-2u, base:0x%08X\n", r, base);

                total += p_ram->total_cnt;
            }

            CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 1, p_ram->total_cnt - 1));
        }
    }

    /* queue stats alloc */
    if(DRV_IS_DUET2(lchip))
    {
        p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_QUEUE]);
        CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, DsStatsQueue_t, &entry_num));
        entry_num--;
        p_ram = &(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_QUEUE]);
        opf.pool_index = SYS_STATS_RAM_QUEUE;
        CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, entry_num, &offset));
        stats_ptr = SYS_STATS_ENCODE_SYS_PTR(SYS_STATS_RAM_QUEUE, 0);
        CTC_ERROR_RETURN(_sys_usw_flow_stats_entry_create(lchip, entry_num, stats_ptr, SYS_STATS_PTR_FLAG_CREATE_ON_INIT));
        p_prop->used_cnt_ram[SYS_STATS_RAM_QUEUE] += entry_num;
        p_prop->used_cnt += entry_num;
        p_ram->used_cnt += entry_num;
    }

    CTC_ERROR_RETURN(_sys_usw_flow_stats_alloc_solid_type(lchip));

    return CTC_E_NONE;
}


STATIC int32
_sys_usw_flow_stats_ram_init(uint8 lchip)
{
    sys_stats_ram_t* p_ram = NULL;

    /*SYS_STATS_RAM_IPE_GLOBAL0*/
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL0]);
    if(DRV_IS_DUET2(lchip))
    {
        p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsIngressGlobal0_t)/4*3;/*4096/4*4 = 3071 */
    }
    else
    {
        p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsIngressGlobal0_t)/4*4;/*4096/4*4 = 4096 */
    }
    p_ram->cache_id = DsStatsIngressGlobal0_t;
    p_ram->base_field_id = StatsCacheBasePtr_ipeGlobalStats0BasePtr_f;

    /*SYS_STATS_RAM_IPE_GLOBAL1*/
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL1]);
    p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsIngressGlobal1_t)/4*3;
    p_ram->cache_id = DsStatsIngressGlobal1_t;
    p_ram->base_field_id = StatsCacheBasePtr_ipeGlobalStats1BasePtr_f;

    /*SYS_STATS_RAM_IPE_GLOBAL2*/
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL2]);
    p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsIngressGlobal2_t)/4*3;
    p_ram->cache_id = DsStatsIngressGlobal2_t;
    p_ram->base_field_id = StatsCacheBasePtr_ipeGlobalStats2BasePtr_f;

    /*SYS_STATS_RAM_IPE_GLOBAL3*/
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_GLOBAL3]);
    if(DRV_IS_DUET2(lchip))
    {
        p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsIngressGlobal3_t)/4*3;
    }
    else
    {
        p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsIngressGlobal3_t)/4*2;
    }
    p_ram->cache_id = DsStatsIngressGlobal3_t;
    p_ram->base_field_id = StatsCacheBasePtr_ipeGlobalStats3BasePtr_f;

    /*SYS_STATS_RAM_EPE_GLOBAL0*/
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL0]);
    if(DRV_IS_DUET2(lchip))
    {
        p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsEgressGlobal0_t)/2;
    }
    else
    {
        p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsEgressGlobal0_t)/4*4;
    }
    p_ram->cache_id = DsStatsEgressGlobal0_t;
    p_ram->base_field_id = StatsCacheBasePtr_epeGlobalStats0BasePtr_f;

    /*SYS_STATS_RAM_EPE_GLOBAL1*/
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL1]);
    if(DRV_IS_DUET2(lchip))
    {
        p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsEgressGlobal1_t)/2;
    }
    else
    {
        p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsEgressGlobal1_t)/4*3;
    }
    p_ram->cache_id = DsStatsEgressGlobal1_t;
    p_ram->base_field_id = StatsCacheBasePtr_epeGlobalStats1BasePtr_f;

    /*SYS_STATS_RAM_EPE_GLOBAL2*/
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL2]);
    if(DRV_IS_DUET2(lchip))
    {
        p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsEgressGlobal2_t)/2;
    }
    else
    {
        p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsEgressGlobal2_t)/4*3;
    }
    p_ram->cache_id = DsStatsEgressGlobal2_t;
    p_ram->base_field_id = StatsCacheBasePtr_epeGlobalStats2BasePtr_f;

    /*SYS_STATS_RAM_EPE_GLOBAL3*/
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_GLOBAL3]);
    if(DRV_IS_DUET2(lchip))
    {
        p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsEgressGlobal3_t)/2;
    }
    else
    {
        p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsEgressGlobal3_t)/4*2;
    }
    p_ram->cache_id = DsStatsEgressGlobal3_t;
    p_ram->base_field_id = StatsCacheBasePtr_epeGlobalStats3BasePtr_f;

    /*SYS_STATS_RAM_IPE_PRIVATE0*/
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_PRIVATE0]);
    p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsIngressACL0_t);
    p_ram->cache_id = DsStatsIngressACL0_t;
    p_ram->base_field_id = StatsCacheBasePtr_ipePrivateStats0BasePtr_f;
    CTC_BIT_SET(p_ram->stats_bmp, SYS_STATS_TYPE_ACL);

    /*SYS_STATS_RAM_IPE_PRIVATE1*/
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_PRIVATE1]);
    p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsIngressACL1_t);
    p_ram->cache_id = DsStatsIngressACL1_t;
    p_ram->base_field_id = StatsCacheBasePtr_ipePrivateStats1BasePtr_f;
    CTC_BIT_SET(p_ram->stats_bmp, SYS_STATS_TYPE_ACL);

    /*SYS_STATS_RAM_IPE_PRIVATE2*/
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_PRIVATE2]);
    p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsIngressACL2_t);
    p_ram->cache_id = DsStatsIngressACL2_t;
    p_ram->base_field_id = StatsCacheBasePtr_ipePrivateStats2BasePtr_f;
    CTC_BIT_SET(p_ram->stats_bmp, SYS_STATS_TYPE_ACL);

    /*SYS_STATS_RAM_IPE_PRIVATE3*/
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_IPE_PRIVATE3]);
    p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsIngressACL3_t);
    p_ram->cache_id = DsStatsIngressACL3_t;
    p_ram->base_field_id = StatsCacheBasePtr_ipePrivateStats3BasePtr_f;
    CTC_BIT_SET(p_ram->stats_bmp, SYS_STATS_TYPE_ACL);

    /*SYS_STATS_RAM_EPE_PRIVATE0*/
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_EPE_PRIVATE0]);
    p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsEgressACL0_t);
    p_ram->cache_id = DsStatsEgressACL0_t;
    p_ram->base_field_id = StatsCacheBasePtr_epePrivateStats0BasePtr_f;
    CTC_BIT_SET(p_ram->stats_bmp, SYS_STATS_TYPE_ACL);

    /*SYS_STATS_RAM_QUEUE*/
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_QUEUE]);
    p_ram->total_cnt = TABLE_MAX_INDEX(lchip, DsStatsQueue_t);
    p_ram->cache_id = DsStatsQueue_t;
    p_ram->base_field_id = StatsCacheBasePtr_qMgrGlobalStatsBasePtr_f;

    /*SYS_STATS_RAM_SDC*/
    p_ram = &(usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_SDC]);
    p_ram->total_cnt = 16383;
    p_ram->cache_id = 0;

    CTC_ERROR_RETURN(_sys_usw_flow_stats_ram_alloc(lchip));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_flow_stats_free_node(void* node_data, void* user_data)
{
    mem_free(node_data);

    return CTC_E_NONE;
}

#define __9_STATS_API__

int32
sys_usw_flow_stats_ram_assign(uint8 lchip, uint16 ram_bmp[][CTC_BOTH_DIRECTION], uint16 acl_ram_bmp[][CTC_BOTH_DIRECTION])
{
    uint8 dir = 0;
    uint8 ctc_type = 0;
    uint8 acl_pri = 0;
    uint8 i = 0, ram = 0;
    sys_stats_prop_t* p_prop = NULL;
    uint8 sys_type = 0;
    uint32 count = 0;
    uint8 bit_cnt[16] = {0};

    for(i = 0; i < 16; i++)
    {
        if(CTC_IS_BIT_SET(ram_bmp[CTC_STATS_STATSID_TYPE_ECMP][CTC_INGRESS],i))
        {
            bit_cnt[0]++;
        }
        if(CTC_IS_BIT_SET(ram_bmp[CTC_STATS_STATSID_TYPE_PORT][CTC_INGRESS],i))
        {
            bit_cnt[1]++;
        }
        if(CTC_IS_BIT_SET(ram_bmp[CTC_STATS_STATSID_TYPE_PORT][CTC_EGRESS],i))
        {
            bit_cnt[2]++;
        }
    }
    if(bit_cnt[0] > 1 || bit_cnt[1] > 1 || bit_cnt[2] > 1)
    {
        SYS_STATS_DBG_ERROR("ecmp & port stats only one bit can be set\n");
        return CTC_E_INTR_INVALID_PARAM;
    }

    ctc_hash_get_count(usw_flow_stats_master[lchip]->flow_stats_id_hash, &count);
    if(count)
    {
        SYS_STATS_DBG_ERROR("must be configed before creat first stats_id\n");
        return CTC_E_IN_USE;
    }

    for(dir = 0; dir < CTC_BOTH_DIRECTION; dir++)
    {
        sal_memset(bit_cnt, 0, sizeof(bit_cnt));
        for (acl_pri = 0; acl_pri < 8; acl_pri++)
        {
            for (ram = 0; ram < 16; ram++)
            {
                if (CTC_IS_BIT_SET(acl_ram_bmp[acl_pri][dir], ram))
                {
                    bit_cnt[ram]++;
                }
                if (bit_cnt[ram] > 1)
                {
                    return CTC_E_INTR_INVALID_PARAM;
                }
            }
        }
    }

    CTC_ERROR_RETURN(_sys_usw_flow_stats_free_solid_type(lchip));

    for (ctc_type = 0; ctc_type < CTC_STATS_STATSID_TYPE_MAX; ctc_type++)
    {
        if(CTC_STATS_STATSID_TYPE_FID == ctc_type || CTC_STATS_STATSID_TYPE_SDC == ctc_type || CTC_STATS_STATSID_TYPE_ACL == ctc_type)
        {
            continue;
        }
        _sys_usw_flow_stats_map_type(lchip, ctc_type, 0, &sys_type, &dir);

        if(CTC_BOTH_DIRECTION == dir || CTC_INGRESS == dir)
        {
            for (i = 0; i < 4; i++)/*only set global ram*/
            {
                p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][sys_type]);
                ram = SYS_STATS_RAM_IPE_GLOBAL0 + i;
                if (CTC_IS_BIT_SET(ram_bmp[ctc_type][CTC_INGRESS], i))
                {
                    CTC_BIT_SET(p_prop->ram_bmp, ram);
                    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[ram].stats_bmp, sys_type);
                }
                else
                {
                    CTC_BIT_UNSET(p_prop->ram_bmp, ram);
                    CTC_BIT_UNSET(usw_flow_stats_master[lchip]->stats_ram[ram].stats_bmp, sys_type);
                }
            }

        }

        if(CTC_BOTH_DIRECTION == dir || CTC_EGRESS == dir)
        {
            for (i = 0; i < 4; i++)/*only set global ram*/
            {
                p_prop = &(usw_flow_stats_master[lchip]->stats_type[CTC_EGRESS][sys_type]);
                ram = SYS_STATS_RAM_EPE_GLOBAL0 + i;
                if (CTC_IS_BIT_SET(ram_bmp[ctc_type][CTC_EGRESS], i))
                {
                    CTC_BIT_SET(p_prop->ram_bmp, ram);
                    CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[ram].stats_bmp, sys_type);
                }
                else
                {
                    CTC_BIT_UNSET(p_prop->ram_bmp, ram);
                    CTC_BIT_UNSET(usw_flow_stats_master[lchip]->stats_ram[ram].stats_bmp, sys_type);
                }
            }
        }
    }

    usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_ACL].ram_bmp = 0;
    usw_flow_stats_master[lchip]->stats_type[CTC_EGRESS][SYS_STATS_TYPE_ACL].ram_bmp = 0;
    for(ram = 0; ram < SYS_STATS_RAM_QUEUE; ram++)
    {
        CTC_BIT_UNSET(usw_flow_stats_master[lchip]->stats_ram[ram].stats_bmp, SYS_STATS_TYPE_ACL);
    }

    for (acl_pri = 0; acl_pri < 8; acl_pri++)
    {
        for (i = 0; i < 8; i++)
        {
            ram = SYS_STATS_RAM_IPE_GLOBAL0 + i;
            if (CTC_IS_BIT_SET(acl_ram_bmp[acl_pri][CTC_INGRESS], i))
            {
                CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_type[CTC_INGRESS][SYS_STATS_TYPE_ACL].ram_bmp, ram);
                CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[ram].stats_bmp, SYS_STATS_TYPE_ACL);
                usw_flow_stats_master[lchip]->stats_ram[ram].acl_priority = acl_pri;
            }
        }
    }

    for (acl_pri = 0; acl_pri < 3; acl_pri++)
    {
        for (i = 0; i < 5; i++)
        {
            ram = SYS_STATS_RAM_EPE_GLOBAL0 + i;
            if (CTC_IS_BIT_SET(acl_ram_bmp[acl_pri][CTC_EGRESS], i))
            {
                CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_type[CTC_EGRESS][SYS_STATS_TYPE_ACL].ram_bmp, ram);
                CTC_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[ram].stats_bmp, SYS_STATS_TYPE_ACL);
                usw_flow_stats_master[lchip]->stats_ram[ram].acl_priority = acl_pri;
            }
        }
    }

    CTC_ERROR_RETURN(_sys_usw_flow_stats_alloc_solid_type(lchip));

    return CTC_E_NONE;
}

/* get stats ptr and check stats type and direction by stats id*/
int32
sys_usw_flow_stats_get_statsptr_with_check(uint8 lchip, sys_stats_statsid_t* p_statsid, uint32* p_cache_ptr)
{
    sys_stats_statsid_t statsid_lkp;

    CTC_PTR_VALID_CHECK(p_statsid);
    CTC_PTR_VALID_CHECK(p_cache_ptr);
    /*Check stats id type and direction*/
    sal_memset(&statsid_lkp, 0, sizeof(sys_stats_statsid_t));
    statsid_lkp.stats_id = p_statsid->stats_id;
    CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsid(lchip, &statsid_lkp));
    if (statsid_lkp.dir != p_statsid->dir || statsid_lkp.stats_id_type != p_statsid->stats_id_type)
    {
        SYS_STATS_DBG_ERROR(" Stats id is not valid, line:%u \n", __LINE__);
        CTC_ERROR_RETURN(CTC_E_INVALID_PARAM);
    }
    /*Get stats ptr*/
    CTC_ERROR_RETURN(sys_usw_flow_stats_get_statsptr(lchip, p_statsid->stats_id, p_cache_ptr));
    SYS_STATS_DBG_INFO("stats_id:%u, cache_ptr:0x%04X\n", p_statsid->stats_id, *p_cache_ptr);
    return CTC_E_NONE;
}

/* get stats property by stats id*/
int32
sys_usw_flow_stats_get_statsid(uint8 lchip, sys_stats_statsid_t* p_statsid)
{
    sys_stats_statsid_t* p_statsid_rse = NULL;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();

    STATS_LOCK();

    p_statsid_rse = ctc_hash_lookup(usw_flow_stats_master[lchip]->flow_stats_id_hash, p_statsid);
    if (p_statsid_rse == NULL)
    {
        SYS_STATS_DBG_ERROR(" Entry not exist \n");
        STATS_UNLOCK();
		return CTC_E_NOT_EXIST;
    }

    sal_memcpy(p_statsid, p_statsid_rse, sizeof(sys_stats_statsid_t));
    STATS_UNLOCK();

    return CTC_E_NONE;
}
int32
sys_usw_flow_stats_get_statsptr(uint8 lchip, uint32 stats_id, uint32* p_cache_ptr)
{
    sys_stats_statsid_t statsid_lkp;
    sys_stats_statsid_t* p_statsid_res = NULL;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_cache_ptr);

    STATS_LOCK();

    sal_memset(&statsid_lkp, 0, sizeof(statsid_lkp));
    statsid_lkp.stats_id = stats_id;
    p_statsid_res = ctc_hash_lookup(usw_flow_stats_master[lchip]->flow_stats_id_hash, &statsid_lkp);
    if (p_statsid_res == NULL)
    {
        SYS_STATS_DBG_ERROR(" Entry not exist \n");
        CTC_ERROR_RETURN_UNLOCK(CTC_E_NOT_EXIST);
    }

    *p_cache_ptr = (uint32)(SYS_STATS_ENCODE_CHIP_PTR((p_statsid_res->stats_ptr>>16), (p_statsid_res->stats_ptr&0xFFFF)));
    p_statsid_res->hw_stats_ptr = *p_cache_ptr;

    STATS_UNLOCK();

    SYS_STATS_DBG_INFO("stats_id:%u, stats_ptr:0x%08X, cache_ptr:0x%04X\n", stats_id, p_statsid_res->stats_ptr, *p_cache_ptr);

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_get_igs_port_log_stats(uint8 lchip, uint32 gport, ctc_stats_basic_t* p_stats)
{
    uint16 lport = 0;
    uint32 stats_ptr = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_stats);

    if (!SYS_STATS_TYPE_ENABLE(CTC_STATS_PORT_STATS))
    {
        SYS_STATS_DBG_ERROR(" Port stats not support \n");
        return CTC_E_NOT_SUPPORT;
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    sal_memset(p_stats, 0, sizeof(ctc_stats_basic_t));

    sys_usw_dma_stats_func(lchip, SYS_DMA_FLOW_STATS_CHAN_ID, 0);
    stats_ptr = usw_flow_stats_master[lchip]->port_base_ptr[CTC_INGRESS] + lport;

    STATS_LOCK();

    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_get_stats(lchip, stats_ptr, 1, p_stats));

    STATS_UNLOCK();

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_clear_igs_port_log_stats(uint8 lchip, uint32 gport)
{
    uint16 lport = 0;
    uint32 stats_ptr = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();

    if (!SYS_STATS_TYPE_ENABLE(CTC_STATS_PORT_STATS))
    {
        SYS_STATS_DBG_ERROR(" Port stats not support \n");
        return CTC_E_NOT_SUPPORT;
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);
    sys_usw_dma_stats_func(lchip, SYS_DMA_FLOW_STATS_CHAN_ID, 0);
    stats_ptr = usw_flow_stats_master[lchip]->port_base_ptr[CTC_INGRESS] + lport;

    STATS_LOCK();

    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_clear_stats(lchip, 1, stats_ptr));

    STATS_UNLOCK();

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_get_egs_port_log_stats(uint8 lchip, uint32 gport, ctc_stats_basic_t* p_stats)
{
    uint16 lport = 0;
    uint32 stats_ptr = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_stats);

    if (!SYS_STATS_TYPE_ENABLE(CTC_STATS_PORT_STATS))
    {
        SYS_STATS_DBG_ERROR(" Port stats not support \n");
        return CTC_E_NOT_SUPPORT;
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    sal_memset(p_stats, 0, sizeof(ctc_stats_basic_t));

    sys_usw_dma_stats_func(lchip, SYS_DMA_FLOW_STATS_CHAN_ID, 0);
    stats_ptr = usw_flow_stats_master[lchip]->port_base_ptr[CTC_EGRESS] + lport;

    STATS_LOCK();

    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_get_stats(lchip, stats_ptr, 1, p_stats));

    STATS_UNLOCK();

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_clear_egs_port_log_stats(uint8 lchip, uint32 gport)
{
    uint16 lport = 0;
    uint32 stats_ptr = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();

    if (!SYS_STATS_TYPE_ENABLE(CTC_STATS_PORT_STATS))
    {
        SYS_STATS_DBG_ERROR(" Port stats not support \n");
        return CTC_E_NOT_SUPPORT;
    }

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    sys_usw_dma_stats_func(lchip, SYS_DMA_FLOW_STATS_CHAN_ID, 0);
    stats_ptr = usw_flow_stats_master[lchip]->port_base_ptr[CTC_EGRESS] + lport;

    STATS_LOCK();

    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_clear_stats(lchip, 1, stats_ptr));

    STATS_UNLOCK();

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_enable_igs_port_discard_stats(uint8 lchip, uint8 enable)
{
    uint32 field_val = 0;
    uint32 cmd = 0;

    SYS_STATS_INIT_CHECK();

    STATS_LOCK();
    field_val = ((enable > 0) ? 1: 0);
    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_discardStatsByPort_f);
    CTC_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &field_val));
    field_val = ((enable > 0) ? 1: 0);
    cmd = DRV_IOW(IpeFwdDiscardStatsCtl_t, IpeFwdDiscardStatsCtl_clearOnRead_f);
    CTC_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &field_val));

    STATS_UNLOCK();
    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_get_igs_port_discard_stats(uint8 lchip, uint32 gport, ctc_stats_basic_t* p_stats)
{
    uint16 i = 0;
    uint16 lport = 0;
    uint32 cmd = 0;
    ds_t   ds;
    uint32 count[2] = {0};
    uint32 base = 0;
    uint16 idx = 0;
    drv_work_platform_type_t platform_type = 0;
    ctc_stats_basic_t* p_tmp_stats = NULL;

    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_stats);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    STATS_LOCK();
    CTC_ERROR_RETURN_UNLOCK(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_CHAN_ID, (void *)&base));
    base = base << 6;
    sal_memset(p_stats, 0, sizeof(ctc_stats_basic_t) * SYS_STATS_IPE_DISCARD_TYPE_MAX);

    for (i=SYS_STATS_IPE_DISCARD_TYPE_START; i<=SYS_STATS_IPE_DISCARD_TYPE_END; i++)
    {
        sal_memset(&ds, 0, sizeof(ds_t));
        p_tmp_stats = p_stats + idx;

        cmd = DRV_IOR(DsIngressDiscardStats_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, base+i, cmd, &ds));

        GetDsIngressDiscardStats(A, packetCount_f, &ds, count);
        p_tmp_stats->packet_count = GetDsIngressDiscardStats(V, packetCount_f, &ds);

        CTC_ERROR_RETURN_UNLOCK(drv_get_platform_type(lchip, &platform_type));
        if (SW_SIM_PLATFORM == platform_type)
        {
            sal_memset(ds, 0, sizeof(ds_t));
            cmd = DRV_IOW(DsIngressDiscardStats_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, base+i, cmd, &ds);
        }

        idx++;
    }
    STATS_UNLOCK();
    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_clear_igs_port_discard_stats(uint8 lchip, uint32 gport)
{
    uint16 i = 0;
    uint16 lport = 0;
    uint32 cmd = 0;
    ds_t   ds;
    uint32 base = 0;

    SYS_STATS_INIT_CHECK();

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    STATS_LOCK();
    CTC_ERROR_RETURN_UNLOCK(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_CHAN_ID, (void *)&base));
    base = base << 6;
    sal_memset(ds, 0, sizeof(ds_t));
    cmd = DRV_IOW(DsIngressDiscardStats_t, DRV_ENTRY_FLAG);

    for (i=SYS_STATS_IPE_DISCARD_TYPE_START; i<=SYS_STATS_IPE_DISCARD_TYPE_END; i++)
    {
        DRV_IOCTL(lchip, base+i, cmd, &ds);
    }
    STATS_UNLOCK();
    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_show_igs_port_discard_stats(uint8 lchip, uint32 gport)
{
    uint16 i = 0;
    uint16 cnt = 0;
    ctc_stats_basic_t stats[SYS_STATS_DISCARD_TYPE_MAX];

    sal_memset(stats, 0, sizeof(stats));
    CTC_ERROR_RETURN(sys_usw_flow_stats_get_igs_port_discard_stats(lchip, gport, stats));

    SYS_STATS_DBG_DUMP("Ingress discard stats:\n");
    SYS_STATS_DBG_DUMP("============================================================\n");
    SYS_STATS_DBG_DUMP("Type                   Packet\n");
    SYS_STATS_DBG_DUMP("------------------------------------------------------------\n");
    cnt = 0;
    for (i=0; i<SYS_STATS_IPE_DISCARD_TYPE_MAX; i++)
    {
        if (stats[i].packet_count == 0)
        {
            continue;
        }
        SYS_STATS_DBG_DUMP("%-4u                   %"PRIu64"\n", SYS_STATS_IPE_DISCARD_TYPE_START+i, stats[i].packet_count);
        cnt++;
    }
    if (cnt == 0)
    {
        SYS_STATS_DBG_DUMP("NA                     NA\n");
    }
    SYS_STATS_DBG_DUMP("============================================================\n");

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_enable_egs_port_discard_stats(uint8 lchip, uint8 enable)
{
    uint32 field_val = 0;
    uint32 cmd = 0;

    SYS_STATS_INIT_CHECK();
    STATS_LOCK();
    field_val = ((enable > 0) ? 1: 0);
    cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_discardStatsByPort_f);
    CTC_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &field_val));
    field_val = ((enable > 0) ? 1: 0);
    cmd = DRV_IOW(EpeHdrEditMiscCtl_t, EpeHdrEditMiscCtl_clearOnRead_f);
    CTC_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &field_val));
    STATS_UNLOCK();
    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_get_egs_port_discard_stats(uint8 lchip, uint32 gport, ctc_stats_basic_t* p_stats)
{
    uint16 i = 0;
    uint16 lport = 0;
    uint32 chan_id = 0;
    uint32 cmd = 0;
    ds_t   ds;
    uint32 count[2] = {0};
    uint32 base = 0;
    uint16 idx = 0;
    drv_work_platform_type_t platform_type = 0;
    ctc_stats_basic_t* p_tmp_stats = NULL;

    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_stats);

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    STATS_LOCK();
    CTC_ERROR_RETURN_UNLOCK(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_CHAN_ID, (void *)&chan_id));
    base = (chan_id << 5) + (chan_id << 4) + (chan_id << 3);
    sal_memset(p_stats, 0, sizeof(ctc_stats_basic_t) * SYS_STATS_EPE_DISCARD_TYPE_MAX);

    for (i = 0; i < SYS_STATS_EPE_DISCARD_TYPE_MAX; i++)
    {
        sal_memset(&ds, 0, sizeof(ds_t));
        p_tmp_stats = p_stats + idx;
        idx++;
        cmd = DRV_IOR(DsEgressDiscardStats_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN_UNLOCK(DRV_IOCTL(lchip, base + i, cmd, &ds));

        GetDsEgressDiscardStats(A, packetCount_f, &ds, count);
        p_tmp_stats->packet_count = GetDsEgressDiscardStats(V, packetCount_f, &ds);

        CTC_ERROR_RETURN_UNLOCK(drv_get_platform_type(lchip, &platform_type));
        if (SW_SIM_PLATFORM == platform_type)
        {
            sal_memset(ds, 0, sizeof(ds_t));
            cmd = DRV_IOW(DsEgressDiscardStats_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, base + i, cmd, &ds);
        }
    }
    STATS_UNLOCK();
    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_clear_egs_port_discard_stats(uint8 lchip, uint32 gport)
{
    uint16 i = 0;
    uint32 chan_id = 0;
    uint16 lport = 0;
    uint32 cmd = 0;
    ds_t   ds;
    uint32 base = 0;

    SYS_STATS_INIT_CHECK();

    SYS_MAP_CTC_GPORT_TO_DRV_LPORT_WITH_CHECK(gport, lchip, lport);

    CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, gport, SYS_DMPS_PORT_PROP_CHAN_ID, (void *)&chan_id));
    base = (chan_id << 5) + (chan_id << 4) + (chan_id << 3);
    sal_memset(ds, 0, sizeof(ds_t));
    cmd = DRV_IOW(DsEgressDiscardStats_t, DRV_ENTRY_FLAG);

    for (i=0; i<SYS_STATS_EPE_DISCARD_TYPE_MAX; i++)
    {
        DRV_IOCTL(lchip, base+i, cmd, &ds);
    }

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_show_egs_port_discard_stats(uint8 lchip, uint32 gport)
{
    uint16 i = 0;
    uint16 cnt = 0;
    ctc_stats_basic_t stats[SYS_STATS_DISCARD_TYPE_MAX];

    sal_memset(stats, 0, sizeof(stats));
    CTC_ERROR_RETURN(sys_usw_flow_stats_get_egs_port_discard_stats(lchip, gport, stats));

    SYS_STATS_DBG_DUMP("Egress discard stats:\n");
    SYS_STATS_DBG_DUMP("============================================================\n");
    SYS_STATS_DBG_DUMP("Type                   Packet\n");
    SYS_STATS_DBG_DUMP("------------------------------------------------------------\n");
    cnt = 0;
    for (i=0; i<SYS_STATS_EPE_DISCARD_TYPE_MAX; i++)
    {
        if (stats[i].packet_count == 0)
        {
            continue;
        }
        SYS_STATS_DBG_DUMP("%-4u                   %"PRIu64"\n", SYS_STATS_EPE_DISCARD_TYPE_START+i, stats[i].packet_count);
        cnt++;
    }
    if (cnt == 0)
    {
        SYS_STATS_DBG_DUMP("NA                     NA\n");
    }
    SYS_STATS_DBG_DUMP("============================================================\n");

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_alloc_statsptr(uint8 lchip, ctc_direction_t dir, uint8 stats_num, uint8 stats_type, uint32* p_stats_ptr)
{
    int32 ret = 0;
    sys_stats_statsid_t statsid;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_stats_ptr);

    sal_memset(&statsid, 0, sizeof(sys_stats_statsid_t));
    statsid.dir = dir;
    statsid.stats_id_type = stats_type;

    STATS_LOCK();

    ret = _sys_usw_flow_stats_alloc_statsptr(lchip, stats_num, &statsid);
    if (ret != CTC_E_NONE)
    {
        CTC_ERROR_RETURN_UNLOCK(ret);
    }

    ret = _sys_usw_flow_stats_entry_create(lchip, stats_num, statsid.stats_ptr, 0);
    if (ret != CTC_E_NONE)
    {
        _sys_usw_flow_stats_free_statsptr(lchip, stats_num, &statsid);
        CTC_ERROR_RETURN_UNLOCK(ret);
    }

    _sys_usw_flow_stats_clear_stats(lchip, stats_num, statsid.stats_ptr);

    STATS_UNLOCK();

    *p_stats_ptr = SYS_STATS_ENCODE_CHIP_PTR(statsid.stats_ptr >> 16, statsid.stats_ptr&0xFFFF);
    SYS_STATS_DBG_INFO("stats_ptr:0x%04X\n", *p_stats_ptr);

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_free_statsptr(uint8 lchip, ctc_direction_t dir, uint8 stats_num, uint8 stats_type, uint32* p_stats_ptr)
{
    uint8 ram = 0;
    sys_stats_statsid_t statsid;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_stats_ptr);

    sal_memset(&statsid, 0, sizeof(sys_stats_statsid_t));
    statsid.dir = dir;
    statsid.stats_id_type = stats_type;

    ram = SYS_STATS_PARSER_RAM_INDEX(stats_type, dir, *p_stats_ptr);

    statsid.stats_ptr = SYS_STATS_ENCODE_SYS_PTR(ram, (*p_stats_ptr)&0xFFF);

    STATS_LOCK();

    _sys_usw_flow_stats_clear_stats(lchip, stats_num, statsid.stats_ptr);

    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_free_statsptr(lchip, stats_num, &statsid));
    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_entry_delete(lchip, stats_num, statsid.stats_ptr));

    STATS_UNLOCK();

    SYS_STATS_DBG_INFO("stats_ptr:0x%08X\n", *p_stats_ptr);

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_get_policing_stats(uint8 lchip, ctc_direction_t dir, uint32 stats_ptr, uint8 stats_type,sys_stats_policing_t* p_stats)
{
    uint8 ram = 0;
    ctc_stats_basic_t count;
    uint32 tmp_stats_ptr = 9;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_stats);

    sal_memset(&count, 0, sizeof(ctc_stats_basic_t));
    sal_memset(p_stats, 0, sizeof(sys_stats_policing_t));

    sys_usw_dma_stats_func(lchip, SYS_DMA_FLOW_STATS_CHAN_ID, 0);

    ram = SYS_STATS_PARSER_RAM_INDEX(stats_type, dir, stats_ptr);
    tmp_stats_ptr = SYS_STATS_ENCODE_SYS_PTR(ram, stats_ptr&0xFFF);

    STATS_LOCK();

    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_get_stats(lchip, tmp_stats_ptr, 1, &count));

    STATS_UNLOCK();

    p_stats->policing_confirm_pkts = count.packet_count;
    p_stats->policing_confirm_bytes = count.byte_count;

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_clear_policing_stats(uint8 lchip, ctc_direction_t dir, uint32 stats_ptr, uint8 stats_type)
{
    uint8 ram = 0;
    uint32 tmp_stats_ptr = 9;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();

    sys_usw_dma_stats_func(lchip, SYS_DMA_FLOW_STATS_CHAN_ID, 0);

    ram = SYS_STATS_PARSER_RAM_INDEX(stats_type, dir, stats_ptr);

    tmp_stats_ptr = SYS_STATS_ENCODE_SYS_PTR(ram, stats_ptr&0xFFF);

    STATS_LOCK();

    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_clear_stats(lchip, 1, tmp_stats_ptr));

    STATS_UNLOCK();

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_get_queue_stats(uint8 lchip, uint32 stats_ptr, sys_stats_queue_t* p_stats)
{
    uint32 base_ptr = 0;
    ctc_stats_basic_t count;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();

    sal_memset(&count, 0, sizeof(ctc_stats_basic_t));

    sys_usw_dma_stats_func(lchip, SYS_DMA_FLOW_STATS_CHAN_ID, 0);
    STATS_LOCK();

    base_ptr = usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_QUEUE].base_ptr;
    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_get_stats(lchip, base_ptr + (stats_ptr&0xFFFF), 1, &count));

    STATS_UNLOCK();

    p_stats->packets  = count.packet_count;
    p_stats->bytes    = count.byte_count;

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_clear_queue_stats(uint8 lchip, uint32 stats_ptr)
{
    uint32 base_ptr = 0;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();

    sys_usw_dma_stats_func(lchip, SYS_DMA_FLOW_STATS_CHAN_ID, 0);
    STATS_LOCK();

    base_ptr = usw_flow_stats_master[lchip]->stats_ram[SYS_STATS_RAM_QUEUE].base_ptr;
    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_clear_stats(lchip, 1, base_ptr + (stats_ptr&0xFFFF)));

    STATS_UNLOCK();

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_add_ecmp_stats(uint8 lchip, uint32 stats_id, uint16 ecmp_group_id)
{
    sys_stats_statsid_t statsid_lkp;
    sys_stats_statsid_t* p_statsid_res = NULL;

    SYS_STATS_INIT_CHECK();

    if (!SYS_STATS_TYPE_ENABLE(CTC_STATS_ECMP_STATS))
    {
        SYS_STATS_DBG_ERROR(" Ecmp stats not support \n");
		return CTC_E_NOT_SUPPORT;
    }

    STATS_LOCK();

    sal_memset(&statsid_lkp, 0, sizeof(statsid_lkp));
    statsid_lkp.stats_id = stats_id;
    p_statsid_res = ctc_hash_lookup(usw_flow_stats_master[lchip]->flow_stats_id_hash, &statsid_lkp);
    if (p_statsid_res == NULL)
    {
        SYS_STATS_DBG_ERROR(" Entry not exist \n");
		CTC_ERROR_RETURN_UNLOCK(CTC_E_NOT_EXIST);
    }

    if (SYS_STATS_TYPE_ECMP != p_statsid_res->stats_id_type)
    {
        SYS_STATS_DBG_ERROR(" [Stats] Stats_id type is mismatch \n");
		CTC_ERROR_RETURN_UNLOCK(CTC_E_INVALID_CONFIG);
    }

    if (0 != p_statsid_res->stats_ptr)
    {
        SYS_STATS_DBG_ERROR(" [Stats] Statsid have been used \n");
		CTC_ERROR_RETURN_UNLOCK(CTC_E_IN_USE);
    }

    p_statsid_res->stats_ptr = usw_flow_stats_master[lchip]->ecmp_base_ptr + ecmp_group_id;

    SYS_STATS_DBG_INFO("add ecmp stats stats_ptr:%d\n", p_statsid_res->stats_ptr);

    /* add to fwd stats list */
    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_entry_create(lchip, 1, p_statsid_res->stats_ptr, SYS_STATS_PTR_FLAG_HAVE_STATSID));
    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_clear_stats(lchip, 1, p_statsid_res->stats_ptr));

    STATS_UNLOCK();

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_remove_ecmp_stats(uint8 lchip, uint32 stats_id, uint16 ecmp_group_id)
{
    uint32 stats_ptr = 0;
    sys_stats_statsid_t  statsid_lkp;
    sys_stats_statsid_t* p_statsid_res = NULL;

    SYS_STATS_INIT_CHECK();

    if (!SYS_STATS_TYPE_ENABLE(CTC_STATS_ECMP_STATS))
    {
        SYS_STATS_DBG_ERROR(" Ecmp stats not support \n");
		return CTC_E_NOT_SUPPORT;
    }

    sal_memset(&statsid_lkp, 0, sizeof(statsid_lkp));

    STATS_LOCK();

    statsid_lkp.stats_id = stats_id;
    p_statsid_res = ctc_hash_lookup(usw_flow_stats_master[lchip]->flow_stats_id_hash, &statsid_lkp);

    if (NULL == p_statsid_res)
    {
        SYS_STATS_DBG_ERROR(" Entry not exist \n");
		CTC_ERROR_RETURN_UNLOCK(CTC_E_NOT_EXIST);
    }

    if (SYS_STATS_TYPE_ECMP != p_statsid_res->stats_id_type)
    {
        SYS_STATS_DBG_ERROR(" [Stats] Stats_id type is mismatch \n");
		CTC_ERROR_RETURN_UNLOCK(CTC_E_INVALID_CONFIG);
    }

    p_statsid_res->stats_ptr = 0;
    stats_ptr = usw_flow_stats_master[lchip]->ecmp_base_ptr + ecmp_group_id;
    SYS_STATS_DBG_INFO("delete ecmp stats stats_ptr:%d\n", stats_ptr);

    /* remove to fwd stats list */
    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_clear_stats(lchip, 1, stats_ptr));
    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_entry_delete(lchip, 1, stats_ptr));

    STATS_UNLOCK();

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_set_drop_packet_stats_en(uint8 lchip, ctc_stats_discard_t bitmap, bool enable)
{
    uint32 cmd = 0;
    uint32 tmp = 0;
    uint32  care_discard = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();

    tmp = 0;
    if (0 != (bitmap & CTC_STATS_FLOW_DISCARD_STATS))
    {
        tmp = (enable) ? 1 : 0;

        care_discard = (enable) ? 0x3 : 0;
        cmd = DRV_IOW(IpeSclFlowCtl_t, IpeSclFlowCtl_sclStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &care_discard));

        cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_srcInterfacestatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        cmd = DRV_IOW(IpeIntfMapperCtl_t, IpeIntfMapperCtl_srcVlanStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        cmd = DRV_IOW(IpeTunnelDecapCtl_t, IpeTunnelDecapCtl_tunnelStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        cmd = DRV_IOW(IpeBridgeCtl_t, IpeBridgeCtl_macdaStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_fwdStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        care_discard = (enable) ? 0xF : 0;
        cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_userIdstatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &care_discard));

        care_discard = (enable) ? 0x7 : 0;
        cmd = DRV_IOW(IpeMplsCtl_t, IpeMplsCtl_statsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &care_discard));

        cmd = DRV_IOW(IpeFlowHashCtl_t, IpeFlowHashCtl_statsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        cmd = DRV_IOW(IpeRouteCtl_t, IpeRouteCtl_vrfStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        cmd = DRV_IOW(IpeRouteCtl_t, IpeRouteCtl_ipdaStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        cmd = DRV_IOW(IpeEcmpCtl_t, IpeEcmpCtl_ecmpStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        care_discard = (enable) ? 0xFF : 0;
        cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_aclStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &care_discard));
        cmd = DRV_IOW(IpeAclQosCtl_t, IpeAclQosCtl_aclStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &care_discard));

        /*EPE stats*/
        cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_nhpStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_xlateStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_vlanStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        cmd = DRV_IOW(EpeNextHopCtl_t, EpeNextHopCtl_destInfStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_l3edit0StatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_l3edit1StatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_l3edit2StatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        care_discard = (enable) ? 0x7 : 0;
        cmd = DRV_IOW(EpeAclQosCtl_t, EpeAclQosCtl_aclStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &care_discard));
    }

    if (0 != (bitmap & CTC_STATS_RANDOM_LOG_DISCARD_STATS))
    {
        tmp = (enable) ? 1 : 0;

        cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_portStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));

        cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_portStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));
    }

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_get_drop_packet_stats_en(uint8 lchip, ctc_stats_discard_t bitmap, bool* enable)
{
    uint32 cmd = 0, tmp = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(enable);

    *enable = FALSE;

    switch (bitmap & (CTC_STATS_RANDOM_LOG_DISCARD_STATS | CTC_STATS_FLOW_DISCARD_STATS))
    {
    case CTC_STATS_FLOW_DISCARD_STATS:
        cmd = DRV_IOR(IpeFwdCtl_t, IpeFwdCtl_fwdStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));
        *enable = (tmp) ? TRUE : FALSE;
        break;
    case CTC_STATS_RANDOM_LOG_DISCARD_STATS:
        cmd = DRV_IOR(IpeFwdCtl_t, IpeFwdCtl_portStatsCareDiscard_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));
        *enable = (tmp) ? TRUE : FALSE;
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_set_saturate_en(uint8 lchip, ctc_stats_type_t stats_type, bool enable)
{
    uint32 cmd = 0;
    uint32 tmp = 0;
    SYS_STATS_INIT_CHECK();
    SYS_STATS_TYPE_CHECK(stats_type);

    tmp = (FALSE == enable) ? 0 : 1;
    SYS_STATS_DBG_FUNC();
    SYS_STATS_DBG_INFO( "enable:%d\n", enable);

    switch (stats_type)
    {
    case CTC_STATS_TYPE_FWD:
        /*DS STATS*/
        cmd = DRV_IOW(GlobalStatsCtl_t, GlobalStatsCtl_saturateEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_get_saturate_en(uint8 lchip, ctc_stats_type_t stats_type, bool* p_enable)
{
    uint32 cmd = 0;
    uint32 tmp = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    SYS_STATS_TYPE_CHECK(stats_type);

    switch (stats_type)
    {
    case CTC_STATS_TYPE_FWD:
        /*DS STATS*/
        cmd = DRV_IOR(GlobalStatsCtl_t, GlobalStatsCtl_saturateEn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    *p_enable = (tmp>0 ? TRUE: FALSE);

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_set_hold_en(uint8 lchip, ctc_stats_type_t stats_type, bool enable)
{
    uint32 cmd = 0;
    uint32 tmp = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    SYS_STATS_TYPE_CHECK(stats_type);

    tmp = (FALSE == enable) ? 0 : 1;
    switch (stats_type)
    {
    case CTC_STATS_TYPE_FWD:
        /*DS STATS*/
        cmd = DRV_IOW(GlobalStatsCtl_t, GlobalStatsCtl_statsHold_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_get_hold_en(uint8 lchip, ctc_stats_type_t stats_type, bool* p_enable)
{
    uint32 cmd = 0;
    uint32 tmp = 0;

    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    SYS_STATS_TYPE_CHECK(stats_type);

    switch (stats_type)
    {
    case CTC_STATS_TYPE_FWD:
        /*DS STATS*/
        cmd = DRV_IOR(GlobalStatsCtl_t, GlobalStatsCtl_statsHold_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    *p_enable = (tmp>0 ? TRUE: FALSE);

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_set_clear_after_read_en(uint8 lchip, ctc_stats_type_t stats_type, bool enable)
{
    uint32 cmd = 0;
    uint32 tmp = 0;

    SYS_STATS_INIT_CHECK();
    SYS_STATS_TYPE_CHECK(stats_type);

    tmp = (FALSE == enable) ? 0 : 1;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_DBG_INFO( "enable:%d\n", enable);

    switch (stats_type)
    {
    case CTC_STATS_TYPE_FWD:
        /*DS STATS*/
        cmd = DRV_IOW(GlobalStatsCtl_t, GlobalStatsCtl_clearOnRead_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    usw_flow_stats_master[lchip]->clear_read_en = enable;

    return CTC_E_NONE;

}

int32
sys_usw_flow_stats_get_clear_after_read_en(uint8 lchip, ctc_stats_type_t stats_type, bool* p_enable)
{
    uint32 cmd = 0;
    uint32 tmp = 0;

    SYS_STATS_INIT_CHECK();
    SYS_STATS_TYPE_CHECK(stats_type);

    SYS_STATS_DBG_FUNC();

    switch (stats_type)
    {
    case CTC_STATS_TYPE_FWD:
        /*DS STATS*/
        cmd = DRV_IOR(GlobalStatsCtl_t, GlobalStatsCtl_clearOnRead_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &tmp));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    *p_enable = (tmp>0 ? TRUE: FALSE);

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_create_statsid(uint8 lchip, ctc_stats_statsid_t* p_ctc_statsid)
{
    uint8 type = 0;
    uint8 dir = 0;
    sys_stats_statsid_t  sys_statsid;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_ctc_statsid);

    if(CTC_STATS_STATSID_TYPE_ECMP == p_ctc_statsid->type && p_ctc_statsid->dir == CTC_EGRESS)
    {
        return CTC_E_INVALID_PARAM;
    }
    CTC_ERROR_RETURN(_sys_usw_flow_stats_map_type(lchip, p_ctc_statsid->type, p_ctc_statsid->statsid.is_vc_label, &type, &dir));
    sal_memset(&sys_statsid, 0, sizeof(sys_statsid));
    sys_statsid.stats_id = p_ctc_statsid->stats_id;
    sys_statsid.color_aware = p_ctc_statsid->color_aware;
    sys_statsid.stats_id_type = type;
    sys_statsid.dir = (dir == CTC_BOTH_DIRECTION) ? p_ctc_statsid->dir : dir;

    if (CTC_STATS_STATSID_TYPE_ACL == p_ctc_statsid->type)
    {
        sys_statsid.u.acl_priority = p_ctc_statsid->statsid.acl_priority;
        if (CTC_INGRESS == sys_statsid.dir)
        {
            CTC_MAX_VALUE_CHECK(sys_statsid.u.acl_priority, ACL_IGS_BLOCK_MAX_NUM-1);
        }
        else if (CTC_EGRESS == sys_statsid.dir)
        {
            CTC_MAX_VALUE_CHECK(sys_statsid.u.acl_priority, ACL_EGS_BLOCK_MAX_NUM-1);
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }
    }
    else if (CTC_STATS_STATSID_TYPE_MPLS == p_ctc_statsid->type)
    {
        sys_statsid.u.is_vc_label = p_ctc_statsid->statsid.is_vc_label;
        CTC_MAX_VALUE_CHECK(sys_statsid.u.is_vc_label, 1);
    }

    STATS_LOCK();

    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_create_statsid(lchip, &sys_statsid));

    STATS_UNLOCK();

    p_ctc_statsid->stats_id = sys_statsid.stats_id;

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_destroy_statsid(uint8 lchip, uint32 stats_id)
{
    sys_stats_statsid_t  statsid_lkp;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    CTC_MIN_VALUE_CHECK(stats_id, 1);

    sal_memset(&statsid_lkp, 0, sizeof(statsid_lkp));
    statsid_lkp.stats_id = stats_id;

    STATS_LOCK();

    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_destroy_statsid(lchip, &statsid_lkp));

    STATS_UNLOCK();

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_lookup_statsid(uint8 lchip, ctc_stats_statsid_type_t type, uint32 cache_ptr, uint32* p_statsid, uint8 dir)
{
    int32 ret = 0;
    sys_stats_statsid_t statsid;
    uint8 sys_type = SYS_STATS_TYPE_MAX;
    uint8 mapping_dir = CTC_BOTH_DIRECTION;
    SYS_STATS_DBG_FUNC();

    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_statsid);
    if (CTC_STATS_STATSID_TYPE_ECMP == type)
    {
        /* stats_ptr from sys_usw_flow_stats_add_ecmp_stats which called by nexthop */
        return CTC_E_NONE;
    }

    sal_memset(&statsid, 0, sizeof(sys_stats_statsid_t));
    statsid.hw_stats_ptr = cache_ptr;
    CTC_ERROR_RETURN(_sys_usw_flow_stats_map_type(lchip,  type,  0, &sys_type, &mapping_dir));
    statsid.stats_id_type = sys_type;
    statsid.dir = (mapping_dir == CTC_BOTH_DIRECTION ? dir : mapping_dir);
    STATS_LOCK();

    ret = ctc_hash_traverse_through(usw_flow_stats_master[lchip]->flow_stats_id_hash, (hash_traversal_fn)_sys_usw_flow_stats_id_hash_value_cmp, &statsid);
    if (ret == CTC_E_EXIST)
    {
        *p_statsid = statsid.stats_id;
        STATS_UNLOCK();
        return CTC_E_NONE;
    }

    STATS_UNLOCK();

    return CTC_E_NOT_EXIST;
}

int32
sys_usw_flow_stats_get_stats(uint8 lchip, uint32 stats_id, ctc_stats_basic_t* p_stats)
{
    uint16 i = 0;
    uint16 stats_num = 1;
    sys_stats_statsid_t sys_statsid_lkp;
    sys_stats_statsid_t* p_statsid_rse;
    ctc_stats_basic_t stats[3];

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    CTC_PTR_VALID_CHECK(p_stats);
    CTC_MIN_VALUE_CHECK(stats_id, 1);

    sal_memset(&sys_statsid_lkp, 0, sizeof(sys_statsid_lkp));

    sys_usw_dma_stats_func(lchip, SYS_DMA_FLOW_STATS_CHAN_ID, 0);
    STATS_LOCK();

    sys_statsid_lkp.stats_id = stats_id;
    p_statsid_rse = ctc_hash_lookup(usw_flow_stats_master[lchip]->flow_stats_id_hash, &sys_statsid_lkp);
    if (p_statsid_rse == NULL)
    {
        SYS_STATS_DBG_ERROR(" Entry not exist \n");
        CTC_ERROR_RETURN_UNLOCK(CTC_E_NOT_EXIST);
    }

    sal_memset(&stats, 0, sizeof(stats));
    stats_num = (p_statsid_rse->color_aware ? 3: 1);
    sal_memset(p_stats, 0, sizeof(ctc_stats_basic_t) * stats_num);

    if (p_statsid_rse->stats_ptr == 0)
    {
        CTC_ERROR_RETURN_UNLOCK(CTC_E_NOT_EXIST);
    }

    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_get_stats(lchip, p_statsid_rse->stats_ptr, stats_num, stats));

    STATS_UNLOCK();

    for (i=0; i<stats_num; i++)
    {
        p_stats[i].byte_count += stats[i].byte_count;
        p_stats[i].packet_count += stats[i].packet_count;
    }

    return CTC_E_NONE;
}
int32
sys_usw_flow_stats_get_stats_num(uint8 lchip, uint32 stats_id, uint32* p_stats_num)
{
    sys_stats_statsid_t sys_statsid_lkp;
    sys_stats_statsid_t* p_statsid_rse;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    CTC_MIN_VALUE_CHECK(stats_id, 1);
    CTC_PTR_VALID_CHECK(p_stats_num);

    STATS_LOCK();
    sys_statsid_lkp.stats_id = stats_id;
    p_statsid_rse = ctc_hash_lookup(usw_flow_stats_master[lchip]->flow_stats_id_hash, &sys_statsid_lkp);
    if (p_statsid_rse == NULL)
    {
        SYS_STATS_DBG_ERROR(" Entry not exist \n");
        CTC_ERROR_RETURN_UNLOCK(CTC_E_NOT_EXIST);
    }
    *p_stats_num = (p_statsid_rse->color_aware ? 3: 1);

    STATS_UNLOCK();
    return CTC_E_NONE;
}
int32
sys_usw_flow_stats_clear_stats(uint8 lchip, uint32 stats_id)
{
    uint16 stats_num = 1;
    sys_stats_statsid_t statsid_lkp;
    sys_stats_statsid_t* p_statsid_rse = NULL;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    CTC_MIN_VALUE_CHECK(stats_id, 1);

    sys_usw_dma_stats_func(lchip, SYS_DMA_FLOW_STATS_CHAN_ID, 0);
    STATS_LOCK();

    sal_memset(&statsid_lkp, 0, sizeof(statsid_lkp));
    statsid_lkp.stats_id = stats_id;
    p_statsid_rse = ctc_hash_lookup(usw_flow_stats_master[lchip]->flow_stats_id_hash, &statsid_lkp);
    if (p_statsid_rse == NULL)
    {
        SYS_STATS_DBG_ERROR(" Entry not exist \n");
        CTC_ERROR_RETURN_UNLOCK(CTC_E_NOT_EXIST);
    }

    if (p_statsid_rse->stats_ptr == 0)
    {
        CTC_ERROR_RETURN_UNLOCK(CTC_E_NOT_EXIST);
    }

    stats_num = (p_statsid_rse->color_aware ? 3: 1);
    CTC_ERROR_RETURN_UNLOCK(_sys_usw_flow_stats_clear_stats(lchip, stats_num, p_statsid_rse->stats_ptr));

    STATS_UNLOCK();

    return CTC_E_NONE;
}

bool
sys_usw_flow_stats_is_color_aware_en(uint8 lchip, uint32 stats_id)
{
    bool enable = FALSE;
    sys_stats_statsid_t statsid_lkp;
    sys_stats_statsid_t* p_statsid_rse = NULL;

    SYS_STATS_DBG_FUNC();
    SYS_STATS_INIT_CHECK();
    CTC_MIN_VALUE_CHECK(stats_id, 1);

    STATS_LOCK();

    sal_memset(&statsid_lkp, 0, sizeof(statsid_lkp));
    statsid_lkp.stats_id = stats_id;
    p_statsid_rse = ctc_hash_lookup(usw_flow_stats_master[lchip]->flow_stats_id_hash, &statsid_lkp);
    if (p_statsid_rse == NULL)
    {
        SYS_STATS_DBG_ERROR(" Entry not exist \n");
        STATS_UNLOCK();
		return FALSE;
    }

    enable = (p_statsid_rse->color_aware ? TRUE: FALSE);

    STATS_UNLOCK();

    return enable;
}

int32
sys_usw_flow_stats_show_status(uint8 lchip)
{
    uint16 i = 0, j = 0, r = 0;
    char*  igr_desc[SYS_STATS_TYPE_MAX] = {"", "Vlan", "L3if",  "Vrf", "Scl",   "TunnelDecap",  "MplsLspLabel",    "MplsVcLabel",   "Acl", "Forward", "Ip", "Policer0", "Policer1", "Ecmp", "Port", "FlowHash", "Mac"};
    char*  egr_desc[SYS_STATS_TYPE_MAX] = {"",      "Vlan", "L3if",  "Vrf", "Xlate", "TunnelEncap",  "NexthopMplsLsp",  "NexthopMplsPw", "Acl", "Nexthop", "",     "Policer0", "Policer1", "", "Port", "", ""};
    char*  igr_module[SYS_STATS_TYPE_MAX] = {"", "Ingress", "Ingress", "Ingress", "Ingress", "Ingress", "Ingress", "Ingress", "Ingress", "Ingress", "Ingress", "Ingress", "Ingress", "Ingress", "Ingress", "Ingress", "Ingress"};
    char*  egr_module[SYS_STATS_TYPE_MAX] = {"", "Egress", "Egress", "Egress", "Egress", "Egress", "Egress", "Egress", "Egress", "Egress", "Egress", "Egress", "Egress", "Egress", "Egress", "Egress", "Egress"};
    char** p_desc = NULL;
    char** p_module = NULL;
    sys_stats_prop_t* p_prop = NULL;
    sys_stats_ram_t* p_ram = usw_flow_stats_master[lchip]->stats_ram;
    LCHIP_CHECK(lchip);
    SYS_STATS_INIT_CHECK();

    SYS_STATS_DBG_DUMP("\n");
    SYS_STATS_DBG_DUMP("Flow Stats Resource Total num:%u, used:%u\n", TABLE_MAX_INDEX(lchip, DsStats_t),usw_flow_stats_master[lchip]->alloc_count);
    SYS_STATS_DBG_DUMP(" --------------------------------------------------------------------------------------------------------------------------------\n");

    for (i = 0; i < CTC_BOTH_DIRECTION; i++)
    {
        p_desc = (CTC_INGRESS == i) ? igr_desc : egr_desc;
        p_module = (CTC_INGRESS == i) ? igr_module : egr_module;

        STATS_LOCK();
        if(i == CTC_INGRESS)
        {
            SYS_STATS_DBG_DUMP(" %-17s%-10s%-9s%-9s%-9s%-9s%-9s%-9s%-9s%-9s%-9s\n",
                           "StatsType", "Module", "Global0", "Global1", "Global2", "Global3", "AclRam0", "AclRam1", "AclRam2", "AclRam3", "used-cnt");
            SYS_STATS_DBG_DUMP(" %-27s%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-9u\n",
                               "max cnt:",
                               p_ram[SYS_STATS_RAM_IPE_GLOBAL0].total_cnt-1,
                               p_ram[SYS_STATS_RAM_IPE_GLOBAL1].total_cnt-1,
                               p_ram[SYS_STATS_RAM_IPE_GLOBAL2].total_cnt-1,
                               p_ram[SYS_STATS_RAM_IPE_GLOBAL3].total_cnt-1,
                               p_ram[SYS_STATS_RAM_IPE_PRIVATE0].total_cnt-1,
                               p_ram[SYS_STATS_RAM_IPE_PRIVATE1].total_cnt-1,
                               p_ram[SYS_STATS_RAM_IPE_PRIVATE2].total_cnt-1,
                               p_ram[SYS_STATS_RAM_IPE_PRIVATE3].total_cnt-1);
            SYS_STATS_DBG_DUMP(" %-27s%-9u%-9u%-9u%-9u%-9u%-9u%-9u%-9u\n",
                               "used cnt:",
                               p_ram[SYS_STATS_RAM_IPE_GLOBAL0].used_cnt,
                               p_ram[SYS_STATS_RAM_IPE_GLOBAL1].used_cnt,
                               p_ram[SYS_STATS_RAM_IPE_GLOBAL2].used_cnt,
                               p_ram[SYS_STATS_RAM_IPE_GLOBAL3].used_cnt,
                               p_ram[SYS_STATS_RAM_IPE_PRIVATE0].used_cnt,
                               p_ram[SYS_STATS_RAM_IPE_PRIVATE1].used_cnt,
                               p_ram[SYS_STATS_RAM_IPE_PRIVATE2].used_cnt,
                               p_ram[SYS_STATS_RAM_IPE_PRIVATE3].used_cnt);

            for (j = SYS_STATS_TYPE_VLAN; j < SYS_STATS_TYPE_SDC; j++)
            {
                p_prop = &(usw_flow_stats_master[lchip]->stats_type[i][j]);

                SYS_STATS_DBG_DUMP(" %-17s%-10s", p_desc[j], p_module[j]);

                for (r = SYS_STATS_RAM_IPE_GLOBAL0; r < SYS_STATS_RAM_QUEUE; r++)
                {
                    if (!CTC_IS_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[r].stats_bmp, j))
                    {
                        SYS_STATS_DBG_DUMP("%-9s", "-");
                    }
                    else
                    {
                        SYS_STATS_DBG_DUMP("%-9u", p_prop->used_cnt_ram[r]);
                    }
                }

                SYS_STATS_DBG_DUMP("%-12u", p_prop->used_cnt);

                SYS_STATS_DBG_DUMP("\n");
            }
        }
        else
        {
            SYS_STATS_DBG_DUMP(" %-17s%-10s%-9s%-9s%-9s%-9s%-9s%-9s\n",
                           "StatsType", "Module", "Global0", "Global1", "Global2", "Global3", "AclRam0", "used-cnt");
            SYS_STATS_DBG_DUMP(" %-27s%-9u%-9u%-9u%-9u%-9u\n","max cnt:",
                               p_ram[SYS_STATS_RAM_EPE_GLOBAL0].total_cnt-1,
                               p_ram[SYS_STATS_RAM_EPE_GLOBAL1].total_cnt-1,
                               p_ram[SYS_STATS_RAM_EPE_GLOBAL2].total_cnt-1,
                               p_ram[SYS_STATS_RAM_EPE_GLOBAL3].total_cnt-1,
                               p_ram[SYS_STATS_RAM_EPE_PRIVATE0].total_cnt-1);
            SYS_STATS_DBG_DUMP(" %-27s%-9u%-9u%-9u%-9u%-9u\n","used cnt:",
                               p_ram[SYS_STATS_RAM_EPE_GLOBAL0].used_cnt,
                               p_ram[SYS_STATS_RAM_EPE_GLOBAL1].used_cnt,
                               p_ram[SYS_STATS_RAM_EPE_GLOBAL2].used_cnt,
                               p_ram[SYS_STATS_RAM_EPE_GLOBAL3].used_cnt,
                               p_ram[SYS_STATS_RAM_EPE_PRIVATE0].used_cnt);

            for (j = SYS_STATS_TYPE_VLAN; j < SYS_STATS_TYPE_SDC; j++)
            {
                if ((SYS_STATS_TYPE_FIB == j)
                    || (SYS_STATS_TYPE_VRF == j)
                || (SYS_STATS_TYPE_QUEUE == j)
                || (SYS_STATS_TYPE_MAC == j)
                || (SYS_STATS_TYPE_FLOW_HASH == j)
                || (SYS_STATS_TYPE_ECMP == j))
                {
                    continue;
                }
                p_prop = &(usw_flow_stats_master[lchip]->stats_type[i][j]);

                SYS_STATS_DBG_DUMP(" %-17s%-10s", p_desc[j], p_module[j]);

                for (r = SYS_STATS_RAM_EPE_GLOBAL0; r < SYS_STATS_RAM_IPE_GLOBAL0; r++)
                {
                    if (!CTC_IS_BIT_SET(usw_flow_stats_master[lchip]->stats_ram[r].stats_bmp, j))
                    {
                        SYS_STATS_DBG_DUMP("%-9s", "-");
                    }
                    else
                    {
                        SYS_STATS_DBG_DUMP("%-9u", p_prop->used_cnt_ram[r]);
                    }
                }

                SYS_STATS_DBG_DUMP("%-12u", p_prop->used_cnt);
                SYS_STATS_DBG_DUMP("\n");
            }

        }
        STATS_UNLOCK();
        SYS_STATS_DBG_DUMP(" --------------------------------------------------------------------------------------------------------------------------------\n");

    }
    SYS_STATS_DBG_DUMP("\n");

    return CTC_E_NONE;
}

int32
sys_usw_flow_stats_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;
    uint32 index = 0;
    uint32 ram_index = 0;
    uint32 type = 0;

    SYS_STATS_INIT_CHECK();
    STATS_LOCK();
    SYS_DUMP_DB_LOG(p_f, "%s\n", "# FLOW STATS");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-30s:0x%x\n", "Stats_bitmap", usw_flow_stats_master[lchip]->stats_bitmap);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n", "Stats_mode", usw_flow_stats_master[lchip]->stats_mode);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n", "Clear_read_on", usw_flow_stats_master[lchip]->clear_read_en);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n", "Ecmp_base_ptr", usw_flow_stats_master[lchip]->ecmp_base_ptr);
    SYS_DUMP_DB_LOG(p_f, "%-30s:%u\n", "Resource used num", usw_flow_stats_master[lchip]->alloc_count);

    SYS_DUMP_DB_LOG(p_f, "%-6s%-4s%-8s%-9s%-12s\n","Type","Dir","Ram_bmp","Used_cnt","used_cnt_ram");
    for(index = 0; index < CTC_BOTH_DIRECTION; index++)
    {
        for (type=0; type<SYS_STATS_TYPE_MAX; type++)
        {
            SYS_DUMP_DB_LOG(p_f, "%-6u%-4u0x%-6x%-9u", type,index,usw_flow_stats_master[lchip]->stats_type[index][type].ram_bmp,usw_flow_stats_master[lchip]->stats_type[index][type].used_cnt);
            SYS_DUMP_DB_LOG(p_f,"[");
            for (ram_index=0; ram_index<SYS_STATS_RAM_MAX-1; ram_index++)
            {
                SYS_DUMP_DB_LOG(p_f,"%u,", usw_flow_stats_master[lchip]->stats_type[index][type].used_cnt_ram[ram_index]);
            }
            SYS_DUMP_DB_LOG(p_f,"%u]\n", usw_flow_stats_master[lchip]->stats_type[index][type].used_cnt_ram[ram_index]);
        }
    }

    SYS_DUMP_DB_LOG(p_f, "%-30s:\n", "Stats_ram");
    SYS_DUMP_DB_LOG(p_f, " %-6s%-11s%-9s%-9s%-10s%-9s%-9s%-9s%-9s\n",
                       "Type", "Stats_bmp", "Cache_id", "Base_fid", "Total_cnt", "Used_cnt", "Base_ptr", "Base_idx", "Acl_pri");
    for(index = 0; index < SYS_STATS_RAM_MAX; index++)
    {
        SYS_DUMP_DB_LOG(p_f, " %-6u0x%-9x%-9u%-9u%-10u%-9u%-9u%-9u%-9u\n",
                       index, usw_flow_stats_master[lchip]->stats_ram[index].stats_bmp, usw_flow_stats_master[lchip]->stats_ram[index].cache_id, usw_flow_stats_master[lchip]->stats_ram[index].base_field_id, \
                       usw_flow_stats_master[lchip]->stats_ram[index].total_cnt, usw_flow_stats_master[lchip]->stats_ram[index].used_cnt, usw_flow_stats_master[lchip]->stats_ram[index].base_ptr, \
                       usw_flow_stats_master[lchip]->stats_ram[index].base_idx, usw_flow_stats_master[lchip]->stats_ram[index].acl_priority);
    }
    SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "\n");
    sys_usw_opf_fprint_alloc_used_info(lchip, usw_flow_stats_master[lchip]->opf_type_flow_stats, p_f);
    if(usw_flow_stats_master[lchip]->opf_type_stats_id != 0)
    {
        sys_usw_opf_fprint_alloc_used_info(lchip, usw_flow_stats_master[lchip]->opf_type_stats_id, p_f);
    }
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");

    STATS_UNLOCK();

    return ret;
}

int32
sys_usw_stats_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param)
{
    sys_usw_mac_stats_dump_db(lchip, p_f, p_dump_param);
    sys_usw_flow_stats_dump_db(lchip, p_f, p_dump_param);

    return CTC_E_NONE;
}
#define __10_STATS_INIT__

int32
sys_usw_flow_stats_init(uint8 lchip, ctc_stats_global_cfg_t* stats_global_cfg)
{
    int32 ret = 0;
    uint8 i = 0;
    uint32 entry_num = 0;
    uint32 cmd = 0;
    uint32 field_value = 0;
    uint16 loop = 0;
    uint32 stats_offet = 0;
    uint8 work_status = 0;

    LCHIP_CHECK(lchip);
    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
    }
    /*init global variable*/
    if (NULL != usw_flow_stats_master[lchip])
    {
        return CTC_E_NONE;
    }

    usw_flow_stats_master[lchip] = (sys_stats_master_t*)mem_malloc(MEM_STATS_MODULE, sizeof(sys_stats_master_t));
    if (NULL == usw_flow_stats_master[lchip])
    {
        SYS_STATS_DBG_ERROR(" No memory \n");
        CTC_ERROR_GOTO(CTC_E_NO_MEMORY, ret, error_roll);
    }

    sal_memset(usw_flow_stats_master[lchip], 0, sizeof(sys_stats_master_t));
    sal_mutex_create(&(usw_flow_stats_master[lchip]->p_stats_mutex));
    usw_flow_stats_master[lchip]->stats_mode = stats_global_cfg->stats_mode;
    usw_flow_stats_master[lchip]->stats_bitmap = stats_global_cfg->stats_bitmap;

    for (i=0; i<SYS_STATS_MAX_MEM_BLOCK; i++)
    {
        ctc_list_pointer_init(&(usw_flow_stats_master[lchip]->flow_stats_list[i]));
    }

    sys_usw_ftm_query_table_entry_num(lchip, DsStats_t, &entry_num);
    /* flow stats hash init */
    usw_flow_stats_master[lchip]->flow_stats_hash = ctc_hash_create(1, entry_num,
                                           (hash_key_fn)_sys_usw_flow_stats_hash_key_make,
                                           (hash_cmp_fn)_sys_usw_flow_stats_hash_key_cmp);
    if (NULL == usw_flow_stats_master[lchip]->flow_stats_hash)
    {
        SYS_STATS_DBG_ERROR(" No memory \n");
        CTC_ERROR_GOTO(CTC_E_NO_MEMORY, ret, error_roll);
    }

    /* sys stats resource init */
    CTC_ERROR_GOTO(_sys_usw_flow_stats_type_init(lchip), ret, error_roll);
    CTC_ERROR_GOTO(_sys_usw_flow_stats_ram_init(lchip), ret, error_roll);

    /* global cfg init */
    CTC_ERROR_GOTO(_sys_usw_flow_stats_global_init(lchip), ret, error_roll);
    CTC_ERROR_GOTO(_sys_usw_flow_stats_statsid_init(lchip), ret, error_roll);

    /* reg callback init */
    CTC_ERROR_GOTO(sys_usw_dma_register_cb(lchip, SYS_DMA_CB_TYPE_FLOW_STATS,_sys_usw_flow_stats_sync_flow_stats), ret, error_roll);
    CTC_ERROR_GOTO(sys_usw_dma_register_cb(lchip, SYS_DMA_CB_TYPE_SDC_STATS, _sys_usw_flow_stats_sync_sdc_stats), ret, error_roll);
    CTC_ERROR_GOTO(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_STATS, _sys_usw_flow_stats_wb_sync), ret, error_roll);
    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_STATS, sys_usw_stats_dump_db), ret, error_roll);

    /* wormboot data restore */
    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(_sys_usw_flow_stats_wb_restore(lchip));
    }

    /*init stats capability*/
    MCHIP_CAP(SYS_CAP_SPEC_TOTAL_STATS_NUM) =  TABLE_MAX_INDEX(lchip, DsStats_t);

    cmd = DRV_IOW(IpeFwdStatsCtl_t, IpeFwdStatsCtl_portStatsIncludeInternal_f);
    field_value = 0;
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error_roll);

    cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_portStatsIncludeInternal_f);
    field_value = 0;
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &field_value), ret, error_roll);

    /* init queue stats */
    if(!DRV_IS_DUET2(lchip) && (!(CTC_WB_ENABLE && (CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)) || (work_status == CTC_FTM_MEM_CHANGE_RECOVER)))
    {
        for(loop = 0; loop < MCHIP_CAP(SYS_CAP_QOS_QUEUE_BASE_TYPE_EXT); loop ++)
        {
            CTC_ERROR_GOTO(sys_usw_flow_stats_alloc_statsptr(lchip, CTC_INGRESS, 2, SYS_STATS_TYPE_QUEUE, &stats_offet),ret ,error_roll);
            cmd = DRV_IOW(DsErmQueueCfg_t, DsErmQueueCfg_statsPtr_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, loop, cmd, &stats_offet), ret ,error_roll);
            cmd = DRV_IOW(DsErmQueueCfg_t, DsErmQueueCfg_statsMode_f);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, loop, cmd, &field_value), ret ,error_roll);
        }
    }
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    return CTC_E_NONE;

error_roll:

    if (NULL != usw_flow_stats_master[lchip])
    {
        if (NULL != usw_flow_stats_master[lchip]->flow_stats_hash)
        {
            ctc_hash_free(usw_flow_stats_master[lchip]->flow_stats_hash);
        }

        if (NULL != usw_flow_stats_master[lchip]->flow_stats_id_hash)
        {
            ctc_hash_free(usw_flow_stats_master[lchip]->flow_stats_id_hash);
        }

        if (usw_flow_stats_master[lchip]->stats_mode == CTC_STATS_MODE_DEFINE)
        {
            sys_usw_opf_deinit(lchip, usw_flow_stats_master[lchip]->opf_type_stats_id);
        }

        sys_usw_opf_deinit(lchip, usw_flow_stats_master[lchip]->opf_type_flow_stats);

        if (usw_flow_stats_master[lchip]->p_stats_mutex)
        {
            sal_mutex_destroy(usw_flow_stats_master[lchip]->p_stats_mutex);
        }

        mem_free(usw_flow_stats_master[lchip]);
        usw_flow_stats_master[lchip] = NULL;
    }

    return ret;
}

int32
sys_usw_flow_stats_deinit(uint8 lchip)
{
    LCHIP_CHECK(lchip);

    if (NULL == usw_flow_stats_master[lchip])
    {
        return CTC_E_NONE;
    }

    if (usw_flow_stats_master[lchip]->p_stats_mutex)
    {
        sal_mutex_destroy(usw_flow_stats_master[lchip]->p_stats_mutex);
    }

    if (NULL != usw_flow_stats_master[lchip]->flow_stats_id_hash)
    {
        /*free stats statsid*/
        ctc_hash_traverse(usw_flow_stats_master[lchip]->flow_stats_id_hash, (hash_traversal_fn)_sys_usw_flow_stats_free_node, NULL);
        ctc_hash_free(usw_flow_stats_master[lchip]->flow_stats_id_hash);
    }

    if (NULL != usw_flow_stats_master[lchip]->flow_stats_hash)
    {
        /*free stats statsid*/
        ctc_hash_traverse(usw_flow_stats_master[lchip]->flow_stats_hash, (hash_traversal_fn)_sys_usw_flow_stats_free_node, NULL);
        ctc_hash_free(usw_flow_stats_master[lchip]->flow_stats_hash);
    }

    if (usw_flow_stats_master[lchip]->stats_mode == CTC_STATS_MODE_DEFINE)
    {
        sys_usw_opf_deinit(lchip, usw_flow_stats_master[lchip]->opf_type_stats_id);
    }

    sys_usw_opf_deinit(lchip, usw_flow_stats_master[lchip]->opf_type_flow_stats);

    mem_free(usw_flow_stats_master[lchip]);
    usw_flow_stats_master[lchip] = NULL;

    return CTC_E_NONE;
}

