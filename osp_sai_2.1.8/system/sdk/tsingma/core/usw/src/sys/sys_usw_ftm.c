/*
 @file sys_usw_ftm.c

 @date 2011-11-16

 @version v2.0

 This file provides all sys alloc function
*/

#include "sal.h"
#include "ctc_const.h"
#include "ctc_error.h"
#include "ctc_debug.h"
#include "ctc_register.h"
#include "sys_usw_opf.h"
#include "sys_usw_register.h"
#include "sys_usw_ftm.h"
#include "sys_usw_chip.h"
#include "sys_usw_common.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_nexthop_api.h"

#include "drv_api.h"
#include "usw/include/drv_common.h"

#include "usw/include/drv_ftm.h"

struct sys_ftm_reset_register_s
{
    uint32 tbl_id;
    uint32 max_index;
    uint32 *p_value;
};
typedef struct sys_ftm_reset_register_s sys_ftm_reset_register_t;

struct sys_ftm_master_s
{
    uint32 glb_met_size;             /*means mcast group number*/
    uint32 glb_nexthop_size;

    uint8 profile_type;
    uint8 ftm_opf_type;
    uint32 profile_specs[CTC_FTM_SPEC_MAX];     /**[GG]record the max entry num for different modules*/
    sys_ftm_callback_t ftm_callback[CTC_FTM_SPEC_MAX];  /**[GG] for ftm show*/
    sys_ftm_rsv_hash_key_cb_t ftm_rsv_hash_key_cb[SYS_FTM_HASH_KEY_MAX];  /**[GG] add or remove reserved key*/
    uint16   sram_type[DRV_FTM_SRAM_MAX];  /**[GG] add or remove reserved key*/
    uint8  status;
    uint8  buffer_mode;/*00: 4.5M->4.5M, 01: 4.5M->9M, 10:9M->4.5M, 11:9M->9M*/
    sys_ftm_change_mem_cb_t fdb_cb;
};
typedef struct sys_ftm_master_s sys_ftm_master_t;

sys_ftm_master_t* g_usw_sys_ftm[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};


enum sys_ftm_opf_type_e
{
    SYS_FTM_OPF_TYPE_FIB_AD,
    SYS_FTM_OPF_TYPE_IPDA_AD,
    SYS_FTM_OPF_TYPE_FLOW_AD,
    SYS_FTM_OPF_TYPE_SCL_AD,
    SYS_FTM_OPF_TYPE_SCL_AD1,
    SYS_FTM_OPF_TYPE_MAX
};
typedef enum sys_ftm_opf_type_e sys_ftm_opf_type_t;
extern int32
drv_usw_get_dynamic_ram_couple_mode(uint8 lchip, uint16 sram_type, uint32* couple_mode);
extern int32 drv_usw_ftm_reset_dynamic_table(uint8 lchip);
int32 sys_usw_ftm_reset_bufferstore_register(uint8 lchip, uint8 mode);
#define NETWORK_CFG_FILE            "dut_topo.txt"

#if (SDK_WORK_PLATFORM == 1)
enum cm_mem_profile_e
{
    CM_MEM_PROFILE_DEFAULT = 0,

    CM_MEM_PROFILE_MACHOST0_FULL_LEVEL = 1,
    CM_MEM_PROFILE_MACHOST1_FULL_LEVEL = 2,
    CM_MEM_PROFILE_FLOW_FULL_LEVEL = 3,
    CM_MEM_PROFILE_USERID_MODE1_FULL_LEVEL = 5,
    CM_MEM_PROFILE_EGRESSXC_MODE1_FULL_LEVEL = 6,
    CM_MEM_PROFILE_IPFIX_FULL_LEVEL = 7,
    CM_MEM_PROFILE_EGRESSXC_MODE0_FULL_LEVEL = 8,
    CM_MEM_PROFILE_LPM_FULL_LEVEL = 10,
    CM_MEM_PROFILE_MET_FULL_LEVEL = 11,
    CM_MEM_PROFILE_USERID_MODE0_FULL_LEVEL = 12,

    CM_MEM_PROFILE_FIB_HOST0_4_LEVEL = 17,

    CM_MEM_PROFILE_OAM_PERF = 20,

    CM_MEM_PROFILE_UNKOWN,
};
typedef enum cm_mem_profile_e cm_mem_profile_t;

enum cm_tcam_profile_e
{
    CM_TCAM_PROFILE_DEFAULT = 0,
    CM_TCAM_PROFILE_IPEACL0 = 1,
    CM_TCAM_PROFILE_IPEACL1 = 2,
    CM_TCAM_PROFILE_EPEACL0 = 3,
    CM_TCAM_PROFILE_FLEX = 4,
    CM_TCAM_PROFILE_INVALID = 5,
};
typedef enum cm_tcam_profile_e cm_tcam_profile_t;

enum cm_lpm_tcam_profile_e
{
    CM_LPM_TCAM_PROFILE_DEFAULT = 0,   /*LPM Tcam lookup case 7*/
    CM_LPM_TCAM_PROFILE_PRI_DA_SA = 1,  /*LPM Tcam lookup case 6*/

    CM_LPM_TCAM_PROFILE_INVALID = 1,
};
typedef enum cm_lpm_tcam_profile_e cm_lpm_tcam_profile_t;

extern int32 swemu_environment_setting(char* filename);
extern int32 sim_model_init(void);
extern int32 sram_model_initialize(uint8 chip_id);
extern int32 tcam_model_initialize(uint8 chip_id);
extern int32 cm_sim_cfg_kit_allocate_sram_tbl_info(void);
extern int32 sram_model_allocate_sram_tbl_info(void);
extern int32 cm_sim_cfg_kit_init_mem_allocation_profile(cm_mem_profile_t mem_profile_index, cm_tcam_profile_t tcam_profile_index,
            cm_lpm_tcam_profile_t lpm_tcam_profile_index);
extern int32 cm_sim_cfg_kit_allocate_tcam_key_process();
extern int32 cm_sim_cfg_kit_allocate_tcam_ad_process();
extern int32 cm_sim_cfg_kit_allocate_lpm_tcam_key_process();
extern int32 cm_sim_cfg_kit_allocate_lpm_tcam_ad_process();
extern int32 cm_sim_cfg_kit_allocate_process();
extern int32 ctc_cmodel_deinit();
extern int32 model_drv_set_dynamic_special_ram_couple_mode(uint32 mem_id ,uint32 couple_mode);
extern uint32 _drv_usw_ftm_memid_2_table(uint8 lchip, uint32 mem_id);
#endif
#define SYS_FTM_INIT_CHECK(lchip)                                 \
    do {                                                            \
        SYS_LCHIP_CHECK_ACTIVE(lchip);                            \
        if (NULL == g_usw_sys_ftm[lchip])                          \
        {                                                          \
            return CTC_E_NOT_INIT;                                 \
        }                                                          \
       }while(0)

int32
_sys_usw_ftm_init_sim_module(uint8 lchip)
{
    int32 ret = 0;
    uint32 cmd = 0;
    uint32 device_info = 0;


    device_info = device_info;
    cmd = cmd;

    /*process only to avoid warning*/
    ret = ret;

#ifndef SDK_IN_VXWORKS
    /* software simulation platform,init  testing swemu environment*/
#if (SDK_WORK_PLATFORM == 1)
    {
        uint32 tbl_id;
        uint32 mem_id;
        uint32 couple_mode;
         /*-ret = swemu_environment_setting(NETWORK_CFG_FILE);*/
        if (ret != 0)
        {
            SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"swemu_environment_setting failed:%s@%d \n",  __FUNCTION__, __LINE__);
            return ret;
        }


        for(mem_id=DRV_FTM_SRAM0; mem_id < DRV_FTM_SRAM_MAX; mem_id++)
        {
            tbl_id = _drv_usw_ftm_memid_2_table(lchip, mem_id);

            drv_usw_get_dynamic_ram_couple_mode(lchip, tbl_id, &couple_mode);
            if(!DRV_IS_DUET2(lchip) && (mem_id == DRV_FTM_SRAM6 || mem_id == DRV_FTM_SRAM7 || mem_id == DRV_FTM_SRAM8 ||
                mem_id == DRV_FTM_SRAM9 || mem_id == DRV_FTM_SRAM10 || mem_id == DRV_FTM_SRAM17))
            {
                couple_mode = 0;
            }

            model_drv_set_dynamic_special_ram_couple_mode(mem_id, couple_mode);
        }
        if(!DRV_IS_DUET2(lchip))
        {
            couple_mode = 1;
            model_drv_set_dynamic_special_ram_couple_mode(DRV_FTM_MIXED0, couple_mode);
            model_drv_set_dynamic_special_ram_couple_mode(DRV_FTM_MIXED1, couple_mode);
            model_drv_set_dynamic_special_ram_couple_mode(DRV_FTM_MIXED2, couple_mode);
        }
        ret = sram_model_initialize(lchip);
        if (ret != 0)
        {
            SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"sram_model_initialize failed:%s@%d \n",  __FUNCTION__, __LINE__);
            return ret;
        }

        ret = tcam_model_initialize(lchip);
        if (ret != 0)
        {
            SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"tcam_model_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
            return ret;
        }

        ret  = cm_sim_cfg_kit_allocate_sram_tbl_info();
        if (ret != 0)
        {
            SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"sram_model_allocate_sram_tbl_info failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
        /* um emu mode sdk */
        ret = sim_model_init();
        if (ret != 0)
        {
            SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"sim_model_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
            return ret;
        }

        /* init cmodel device version ID */
        cmd = DRV_IOR(DevId_t, DevId_deviceId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &device_info));
        device_info |= (DRV_IS_DUET2(lchip) ? 0x271:0x281);
        cmd = DRV_IOW(DevId_t, DevId_deviceId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &device_info));
    }
#endif
#else
    /* software simulation platform,init memmodel*/
#if (SDK_WORK_PLATFORM == 1)
    {
        ret = sram_model_initialize(lchip);
        if (ret != 0)
        {
            SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"sram_model_initialize failed:%s@%d \n",  __FUNCTION__, __LINE__);
            return ret;
        }

        ret = tcam_model_initialize(lchip);
        if (ret != 0)
        {
            SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"tcam_model_init failed:%s@%d \n",  __FUNCTION__, __LINE__);
            return ret;
        }

        ret  = sram_model_allocate_sram_tbl_info();
        if (ret != 0)
        {
            SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"sram_model_allocate_sram_tbl_info failed:%s@%d %s\n",  __FUNCTION__, __LINE__, ctc_get_error_desc(ret));
            return ret;
        }
    }
#endif
#endif

    return CTC_E_NONE;
}
extern int32 sys_usw_ftm_wb_sync(uint8 lchip);
extern int32 sys_usw_ftm_wb_restore(uint8 lchip);

int32
sys_usw_ftm_lkp_register_init(uint8 lchip)
{
    int32 ret = 0;
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && g_usw_sys_ftm[lchip]->status == 3)
	{
		drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
	}
    ret = drv_usw_ftm_lookup_ctl_init(lchip);

    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && g_usw_sys_ftm[lchip]->status == 3)
	{
		drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
	}
    return ret;
}

int32
sys_usw_ftm_get_spec(uint8 lchip, uint32 specs_type)
{
    SYS_FTM_INIT_CHECK(lchip);
    if (CTC_FTM_SPEC_MAX <= specs_type)
    {
        return CTC_E_INVALID_PARAM;
    }

    return g_usw_sys_ftm[lchip]->profile_specs[specs_type];
}

STATIC int32
_sys_usw_ftm_get_resource(uint8 lchip, uint32 specs_type, uint32 tbl_id, uint8 cam)
{
    uint8 index = 0;
    uint32 num = 0;
    uint8 count = 0;
    uint8 start = 0;
    uint32 cam_num = 0;
    uint32 entry_num = 0;
    uint32 entry_num1 = 0;

    switch(specs_type)
    {
        case CTC_FTM_SPEC_ACL:
            count = MCHIP_CAP(SYS_CAP_ACL_INGRESS_ACL_LKUP) + MCHIP_CAP(SYS_CAP_ACL_EGRESS_LKUP_NUM);
            count = ((DRV_IS_DUET2(lchip) ||tbl_id == DsAclQosKey80Egr0_t || cam == 0 ) ?count:MCHIP_CAP(SYS_CAP_ACL_INGRESS_ACL_LKUP));
            start = ((DRV_IS_DUET2(lchip) ||tbl_id == DsAclQosKey80Ing0_t || cam == 0) ?0:MCHIP_CAP(SYS_CAP_ACL_INGRESS_ACL_LKUP));
            for(index = start; index < count; index++)
            {
                drv_usw_ftm_get_entry_num(lchip, tbl_id+index, &num);
                entry_num = entry_num + num;
            }
            break;
        case CTC_FTM_SPEC_IPUC_LPM:
            drv_usw_ftm_get_entry_num(lchip, (DRV_IS_DUET2(lchip)?DsLpmLookupKey_t:tbl_id), &entry_num);
            entry_num = (DRV_IS_DUET2(lchip)?entry_num*2:entry_num*12/4);
            break;
        case CTC_FTM_SPEC_MPLS:
            drv_usw_ftm_get_entry_num(lchip, (DRV_IS_DUET2(lchip)?DsUserIdTunnelMplsHashKey_t:tbl_id), &entry_num);
            cam_num = DRV_IS_DUET2(lchip)?USER_ID_HASH_CAM_NUM:64;
            break;
        case CTC_FTM_SPEC_IPMC:
        case CTC_FTM_SPEC_L2MC:
        case CTC_FTM_SPEC_NAPT:
            entry_num = g_usw_sys_ftm[lchip]->profile_specs[specs_type];
            break;
        case CTC_FTM_SPEC_ACL_FLOW:
            drv_usw_ftm_get_entry_num(lchip, tbl_id, &entry_num);
            cam_num = FLOW_HASH_CAM_NUM;
            break;
        case CTC_FTM_SPEC_IPUC_HOST:
        case CTC_FTM_SPEC_MAC:
            drv_usw_ftm_get_entry_num(lchip, tbl_id, &entry_num);
            cam_num = FIB_HOST0_CAM_NUM;
            break;
        case CTC_FTM_SPEC_TUNNEL:
        case CTC_FTM_SPEC_SCL_FLOW:
        case CTC_FTM_SPEC_SCL:
            drv_usw_ftm_get_entry_num(lchip, tbl_id, &entry_num);
            if(!DRV_IS_DUET2(lchip))
            {
                drv_usw_ftm_get_entry_num(lchip, DsUserId1MacHashKey_t, &entry_num1);
                entry_num += entry_num1;
            }
            cam_num = DRV_IS_DUET2(lchip)?USER_ID_HASH_CAM_NUM:0;
            break;
        case CTC_FTM_SPEC_OAM:
            drv_usw_ftm_get_entry_num(lchip, tbl_id, &entry_num);
            entry_num = entry_num /2;
            break;
        default:
            drv_usw_ftm_get_entry_num(lchip, tbl_id, &entry_num);
            cam_num = 0;
    }

    if(entry_num != 0 && cam !=0)
    {
        entry_num += cam_num;
    }

    return entry_num;

}


int32
sys_usw_ftm_query_table_entry_num(uint8 lchip, uint32 table_id, uint32* entry_num)
{
    drv_usw_ftm_get_entry_num(lchip, table_id, entry_num);
    return CTC_E_NONE;
}

int32
sys_usw_ftm_query_table_entry_size(uint8 lchip, uint32 table_id, uint32* entry_size)
{
    CTC_DEBUG_ADD_STUB_CTC(NULL);
    if (table_id >= MaxTblId_t)
    {
        SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"unexpect tbl id:%d\r\n", table_id);
        return CTC_E_INVALID_PARAM;
    }

    drv_usw_ftm_get_entry_size(lchip, table_id, entry_size);

    return CTC_E_NONE;

}
#if 0
int32
sys_usw_ftm_query_lpm_model(uint8 lchip, uint32* lpm_model)
{
    drv_ftm_info_detail_t ftm_info;

    SYS_FTM_INIT_CHECK(lchip);
    sal_memset(&ftm_info,0,sizeof(drv_ftm_info_detail_t));

    ftm_info.info_type = DRV_FTM_INFO_TYPE_LPM_MODEL;
    drv_usw_ftm_get_info_detail(lchip, &ftm_info);

    *lpm_model = ftm_info.l3.lpm_model;

    return CTC_E_NONE;
}
#endif
int32
sys_usw_ftm_get_lpm_pipeline_info(uint8 lchip, sys_ftm_lpm_pipeline_info_t* p_pipeline_info)
{
    drv_ftm_info_detail_t ftm_info;

    SYS_FTM_INIT_CHECK(lchip);
    DRV_PTR_VALID_CHECK(p_pipeline_info);
    sal_memset(&ftm_info,0,sizeof(drv_ftm_info_detail_t));
    ftm_info.info_type = DRV_FTM_INFO_TYPE_PIPELINE_INFO;
    drv_usw_ftm_get_info_detail(lchip, &ftm_info);

    sal_memcpy(p_pipeline_info, &(ftm_info.l3.pipeline_info), sizeof(sys_ftm_lpm_pipeline_info_t));

    return CTC_E_NONE;
}
int32
sys_usw_ftm_query_lpm_tcam_init_size(uint8 lchip, uint32** init_size)
{
    drv_ftm_info_detail_t ftm_info;

    SYS_FTM_INIT_CHECK(lchip);
    DRV_PTR_VALID_CHECK(init_size);
    sal_memset(&ftm_info,0,sizeof(drv_ftm_info_detail_t));

    ftm_info.info_type = DRV_FTM_INFO_TYPE_LPM_TCAM_INIT_SIZE;
    drv_usw_ftm_get_info_detail(lchip, &ftm_info);

    sal_memcpy(init_size, &(ftm_info.l3.lpm_tcam_init_size), sizeof(ftm_info.l3.lpm_tcam_init_size));

    return CTC_E_NONE;
}
#if 0
extern int32
sys_usw_ftm_get_tcam_ad_index(uint8 lchip, uint32 key_tbl_id,  uint32 key_index)
{
#ifdef NEVER
    uint8 key_size = 0;
    uint8  per_entry_bytes = 0;
    bool is_lpm_key = FALSE;
    uint8 shift = 0;

    is_lpm_key = ((key_tbl_id == DsLpmTcamIpv440Key_t)|| (key_tbl_id == DsLpmTcamIpv6160Key0_t));
    per_entry_bytes =          (is_lpm_key)? DRV_LPM_KEY_BYTES_PER_ENTRY: DRV_BYTES_PER_ENTRY;
    key_size =  (TABLE_ENTRY_SIZE(key_tbl_id) / per_entry_bytes);
    shift =  (is_lpm_key)? 1:2;

    return (key_index*key_size) / shift;
#endif /* never */
    return CTC_E_NONE;
}
#endif
int32
sys_usw_ftm_get_opf_type(uint8 lchip, uint32 table_id, uint32* p_opf_type)
{
    SYS_FTM_INIT_CHECK(lchip);

    switch (table_id)
    {
    case DsMac_t:
        *p_opf_type = SYS_FTM_OPF_TYPE_FIB_AD;
        break;

    case DsIpDa_t:
    case DsTrillDa_t:
    case DsFcoeDa_t:
        *p_opf_type = SYS_FTM_OPF_TYPE_IPDA_AD;
        break;

    case DsAcl_t:
        *p_opf_type = SYS_FTM_OPF_TYPE_FLOW_AD;
        break;

    case DsMpls_t:
    case DsTunnelId_t:
    case DsUserId_t:
    case DsSclFlow_t:
    case DsTunnelIdRpf_t:
        *p_opf_type = SYS_FTM_OPF_TYPE_SCL_AD;
        break;

    case DsTunnelId1_t:
    case DsUserId1_t:
    case DsSclFlow1_t:
    case DsTunnelIdRpf1_t:
        *p_opf_type = SYS_FTM_OPF_TYPE_SCL_AD1;
        break;


    default:
        SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Not suport tbl id:%d now!!\r\n", table_id);
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_ftm_alloc_table_offset(uint8 lchip, uint32 table_id, uint8 dir, uint32 block_size, uint8 multi, uint32* offset)
{
    uint32 opf_type = 0;
    sys_usw_opf_t opf;

    CTC_PTR_VALID_CHECK(offset);
    SYS_FTM_INIT_CHECK(lchip);

    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && (0 == g_usw_sys_ftm[lchip]->status))
    {
        return CTC_E_NONE;
    }
    if (table_id >= MaxTblId_t)
    {
        SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"unexpect tbl id:%d\r\n", table_id);
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_ftm_get_opf_type(lchip, table_id, &opf_type));

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = g_usw_sys_ftm[lchip]->ftm_opf_type;
    opf.pool_index = opf_type;
    opf.multiple  = multi;
    opf.reverse = dir;

    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset(lchip, &opf, block_size, offset));

     /* SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"FTM: alloc tableId:%d block_size:%d, offset:%d\n", table_id, block_size, *offset);*/
    return CTC_E_NONE;
}

int32
sys_usw_ftm_free_table_offset(uint8 lchip, uint32 table_id, uint8 dir, uint32 block_size, uint32 offset)
{
    uint32 opf_type = 0;
    sys_usw_opf_t opf;

    CTC_DEBUG_ADD_STUB_CTC(NULL);
    SYS_FTM_INIT_CHECK(lchip);
    if (table_id >= MaxTblId_t)
    {
        SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"unexpect tbl id:%d\r\n", table_id);
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_ftm_get_opf_type(lchip, table_id, &opf_type));

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = g_usw_sys_ftm[lchip]->ftm_opf_type;
    opf.pool_index = opf_type;

    if (0 == dir)
    {
        CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, block_size, offset));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_opf_free_offset(lchip, &opf, block_size, offset));
    }

    return CTC_E_NONE;
}
int32
sys_usw_ftm_alloc_table_offset_from_position(uint8 lchip, uint32 table_id, uint8 dir, uint32 block_size, uint32 offset)
{
    uint32 opf_type = 0;
    uint8  multi_alloc = 0;
    sys_usw_opf_t opf;

    SYS_FTM_INIT_CHECK(lchip);
    if (table_id >= MaxTblId_t)
    {
        SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"unexpect tbl id:%d\r\n", table_id);
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_ftm_get_opf_type(lchip, table_id, &opf_type));

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = g_usw_sys_ftm[lchip]->ftm_opf_type;
    opf.pool_index = opf_type;
    opf.multiple  = multi_alloc;
    opf.reverse = dir;

    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, block_size, offset));

     /* SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"FTM: alloc tableId:%d block_size:%d, offset:%d\n", table_id, block_size, *offset);*/
    return CTC_E_NONE;
}

int32
sys_usw_ftm_get_alloced_table_count(uint8 lchip, uint32 table_id, uint32* count)
{
    uint32 opf_type = 0;
    sys_usw_opf_t opf;

    CTC_DEBUG_ADD_STUB_CTC(NULL);
    SYS_FTM_INIT_CHECK(lchip);

    if (table_id >= MaxTblId_t)
    {
        SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"unexpect tbl id:%d\r\n", table_id);
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(sys_usw_ftm_get_opf_type(lchip, table_id, &opf_type));

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = g_usw_sys_ftm[lchip]->ftm_opf_type;
    opf.pool_index = opf_type;

    *count = sys_usw_opf_get_alloced_cnt(lchip, &opf);

    return CTC_E_NONE;

}

int32
sys_usw_ftm_get_dyn_entry_size(uint8 lchip, sys_ftm_dyn_entry_type_t type,
                                      uint32* p_size)
{
    SYS_FTM_INIT_CHECK(lchip);

    switch(type)
    {
    case SYS_FTM_DYN_ENTRY_GLB_MET:
        *p_size =  g_usw_sys_ftm[lchip]->glb_met_size;
        break;
    default:
        return CTC_E_INVALID_PARAM;
    }
    return CTC_E_NONE;
}

int32
sys_usw_ftm_get_dynamic_table_info(uint8 lchip, uint32 tbl_id,
                                          uint8* dyn_tbl_idx,
                                          uint32* entry_enum,
                                          uint32* nh_glb_entry_num)
{
    CTC_PTR_VALID_CHECK(dyn_tbl_idx);
    CTC_PTR_VALID_CHECK(entry_enum);
    CTC_PTR_VALID_CHECK(nh_glb_entry_num);
    SYS_FTM_INIT_CHECK(lchip);

    *entry_enum = 0;
    *nh_glb_entry_num = 0;

    switch (tbl_id)
    {
        case DsFwd_t:
            *dyn_tbl_idx = SYS_FTM_DYN_NH0;
            break;

        case DsL2EditEth3W_t:
            *dyn_tbl_idx = SYS_FTM_DYN_NH1;
            break;

        case DsMetEntry3W_t:
            *dyn_tbl_idx = SYS_FTM_DYN_NH2;
            *nh_glb_entry_num = g_usw_sys_ftm[lchip]->glb_met_size;
            break;

        case DsNextHop4W_t:
            *dyn_tbl_idx = SYS_FTM_DYN_NH3;
            *nh_glb_entry_num = g_usw_sys_ftm[lchip]->glb_nexthop_size;
            break;

        default:
            return CTC_E_INVALID_PARAM;
        }

    drv_usw_ftm_get_entry_num(lchip, tbl_id, entry_enum);

    if (DsFwd_t == tbl_id)
    {
        *entry_enum = *entry_enum*2;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ftm_get_opf_size(uint8 lchip, uint32 opf_type, uint32* entry_num)
{
    uint32 table_id = 0;

    switch (opf_type)
    {
        case SYS_FTM_OPF_TYPE_FIB_AD:
            table_id = DsMac_t;
            break;

        case SYS_FTM_OPF_TYPE_IPDA_AD:
            table_id = DsIpDa_t;
            break;

        case SYS_FTM_OPF_TYPE_FLOW_AD:
            table_id = DsAcl_t;
            break;

        case SYS_FTM_OPF_TYPE_SCL_AD:
            table_id = DsUserId_t;
            break;

        case SYS_FTM_OPF_TYPE_SCL_AD1:
            table_id = DsUserId1_t;
            break;


        default:
            return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(sys_usw_ftm_query_table_entry_num(lchip, table_id, entry_num));
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ftm_opf_init_table_share(uint8 lchip)
{
    uint8 i = 0;
    uint8 opf_type = 0;
    uint32 entry_num = 0;
    sys_usw_opf_t opf;

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &opf_type, SYS_FTM_OPF_TYPE_MAX, "opf-ftm"));
    g_usw_sys_ftm[lchip]->ftm_opf_type = opf_type;

    for (i = SYS_FTM_OPF_TYPE_FIB_AD; i < SYS_FTM_OPF_TYPE_MAX; i++)
    {
        opf.pool_type = g_usw_sys_ftm[lchip]->ftm_opf_type;
        opf.pool_index = i;
        CTC_ERROR_RETURN(_sys_usw_ftm_get_opf_size(lchip, i, &entry_num));
        CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 0, entry_num));
    }

    return CTC_E_NONE;
}

int32
sys_usw_ftm_set_misc(uint8 lchip, ctc_ftm_profile_info_t* profile_info)
{

    if(CTC_FTM_PROFILE_USER_DEFINE == profile_info->profile_type)
    {
        g_usw_sys_ftm[lchip]->glb_met_size     = profile_info->misc_info.mcast_group_number;
        g_usw_sys_ftm[lchip]->glb_nexthop_size = profile_info->misc_info.glb_nexthop_number;

        sal_memcpy(&g_usw_sys_ftm[lchip]->profile_specs[0], &profile_info->misc_info.profile_specs[0],
                    sizeof(uint32) * CTC_FTM_SPEC_MAX);
    }
    else
    {
        if(DRV_IS_DUET2(lchip))
        {
            g_usw_sys_ftm[lchip]->glb_met_size     = 8 * 1024;
            g_usw_sys_ftm[lchip]->glb_nexthop_size = 16016;

            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_MAC]          = 16 * 1024;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_IPUC_HOST]    = 8 * 1024;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_IPUC_LPM]     = 8 * 1024;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_IPMC]         = 1024;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_ACL]          = 7680;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_SCL_FLOW]     = 1024;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_ACL_FLOW]     = 16 * 1024;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_OAM]          = 2 * 1024;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_SCL]          = 4 * 1024 + 512;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_TUNNEL]       = 3 * 1024;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_MPLS]         = 4 * 1024;
            /*bug39900 g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_VRF]          = 256;*/
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_L2MC]         = 1024;
            /*bug39900 g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_FID]          = 5 * 1024;*/
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_IPFIX]        = 8 * 1024;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_NAPT]        = 1024;
        }
        else 
        {
            g_usw_sys_ftm[lchip]->glb_met_size     = 8 * 1024;
            g_usw_sys_ftm[lchip]->glb_nexthop_size = 16016;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_IPUC_HOST]    = 16 * 1024;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_IPMC]         = 1024;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_SCL_FLOW]     = 1024;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_SCL]          = 4 * 1024;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_TUNNEL]       = 3 * 1024;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_L2MC]         = 1024;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_NAPT]        = 1024;
            g_usw_sys_ftm[lchip]->profile_specs[CTC_FTM_SPEC_MPLS]         = 8 * 1024;
        }
    }

    return CTC_E_NONE;
}

int32
sys_tsingma_ftm_reset_spec_value(uint8 lchip)
{
    uint8 specs_name[] = 
        {
            CTC_FTM_SPEC_MAC,
            CTC_FTM_SPEC_IPUC_HOST,
            CTC_FTM_SPEC_IPUC_LPM,
            CTC_FTM_SPEC_ACL,
            CTC_FTM_SPEC_ACL_FLOW,
            CTC_FTM_SPEC_SCL,
            CTC_FTM_SPEC_SCL_FLOW,
            CTC_FTM_SPEC_TUNNEL,
            CTC_FTM_SPEC_IPFIX,
            CTC_FTM_SPEC_OAM,
            CTC_FTM_SPEC_MPLS,
            CTC_FTM_SPEC_L2MC,
            CTC_FTM_SPEC_IPMC,
            CTC_FTM_SPEC_NAPT,
        };
    uint32 tabl_name[] =
       {
            DsFibHost0MacHashKey_t,
            DsFibHost0Ipv4HashKey_t,
            DsNeoLpmIpv4Bit32Snake_t,
            DsAclQosKey80Egr0_t,
            DsFlowL2HashKey_t,
            DsUserIdMacHashKey_t,
            0,
            0,
            DsIpfixL2HashKey_t,
            DsBfdMep_t,
            DsMplsLabelHashKey_t,
            0,
            0,        
            0,
       };
    uint8 i = 0;

    for(i = 0; i < sizeof(specs_name)/sizeof(uint8); i++)
    {
        if(g_usw_sys_ftm[lchip]->profile_specs[specs_name[i]] == 0)
        {
            g_usw_sys_ftm[lchip]->profile_specs[specs_name[i]] = _sys_usw_ftm_get_resource(lchip, specs_name[i], tabl_name[i], 0);
        }
    }
    return CTC_E_NONE;
}
/*
   this function use to user define memprofile map
 */
STATIC int32
_sys_usw_ftm_map_key_id_info(uint8 key_id,uint8 key_size, uint8* drv_key_id, uint8* drv_key_size)
{

    /* convert ctc size to drv size*/
    switch(key_size)
    {
        case CTC_FTM_KEY_SIZE_160_BIT:
            *drv_key_size = DRV_FTM_TCAM_SIZE_160;
            break;
        case CTC_FTM_KEY_SIZE_320_BIT:
            *drv_key_size = DRV_FTM_TCAM_SIZE_320;
            break;
        case CTC_FTM_KEY_SIZE_640_BIT:
            *drv_key_size = DRV_FTM_TCAM_SIZE_640;
            break;
        default:
            break;
    }
    /*convert ctc id to drv id*/
    switch(key_id)
    {
        case CTC_FTM_KEY_TYPE_SCL0:
            *drv_key_id = DRV_FTM_TCAM_TYPE_IGS_USERID0;
            break;
        case CTC_FTM_KEY_TYPE_SCL1:
            *drv_key_id = DRV_FTM_TCAM_TYPE_IGS_USERID1;
            break;

        case CTC_FTM_KEY_TYPE_SCL2:
            *drv_key_id = DRV_FTM_TCAM_TYPE_IGS_USERID2;
            break;
        case CTC_FTM_KEY_TYPE_SCL3:
            *drv_key_id = DRV_FTM_TCAM_TYPE_IGS_USERID3;
            break;

        case CTC_FTM_KEY_TYPE_ACL0:
            *drv_key_id = DRV_FTM_TCAM_TYPE_IGS_ACL0;
            break;

        case CTC_FTM_KEY_TYPE_ACL1:
            *drv_key_id = DRV_FTM_TCAM_TYPE_IGS_ACL1;
            break;

        case CTC_FTM_KEY_TYPE_ACL2:
            *drv_key_id = DRV_FTM_TCAM_TYPE_IGS_ACL2;
            break;

        case CTC_FTM_KEY_TYPE_ACL3:
            *drv_key_id = DRV_FTM_TCAM_TYPE_IGS_ACL3;
            break;

        case CTC_FTM_KEY_TYPE_ACL4:
            *drv_key_id = DRV_FTM_TCAM_TYPE_IGS_ACL4;
            break;

        case CTC_FTM_KEY_TYPE_ACL5:
            *drv_key_id = DRV_FTM_TCAM_TYPE_IGS_ACL5;
            break;

        case CTC_FTM_KEY_TYPE_ACL6:
            *drv_key_id = DRV_FTM_TCAM_TYPE_IGS_ACL6;
            break;

        case CTC_FTM_KEY_TYPE_ACL7:
            *drv_key_id = DRV_FTM_TCAM_TYPE_IGS_ACL7;
            break;

        case CTC_FTM_KEY_TYPE_ACL0_EGRESS:
            *drv_key_id = DRV_FTM_TCAM_TYPE_EGS_ACL0;
            break;

        case CTC_FTM_KEY_TYPE_ACL1_EGRESS:
            *drv_key_id = DRV_FTM_TCAM_TYPE_EGS_ACL1;
            break;

        case CTC_FTM_KEY_TYPE_ACL2_EGRESS:
            *drv_key_id = DRV_FTM_TCAM_TYPE_EGS_ACL2;
            break;

        case CTC_FTM_KEY_TYPE_IPV4_UCAST:
            *drv_key_id = DRV_FTM_KEY_TYPE_IPV4_UCAST;
            break;

        case CTC_FTM_KEY_TYPE_IPV4_NAT:
            *drv_key_id = DRV_FTM_KEY_TYPE_IPV4_NAT;
            break;


        case CTC_FTM_KEY_TYPE_IPV6_UCAST:
            *drv_key_id = DRV_FTM_KEY_TYPE_IPV6_UCAST;
            break;


        case CTC_FTM_KEY_TYPE_IPV6_UCAST_HALF:
            *drv_key_id = DRV_FTM_KEY_TYPE_IPV6_UCAST_HALF;
            break;



        default:
            *drv_key_id = 0xFF;
            break;

    }

    return DRV_E_NONE;
}


int32
_sys_usw_ftm_map_tbl_id(uint8 tbl_id,uint8* drv_tbl_id)
{
    switch(tbl_id)
    {
        case CTC_FTM_TBL_TYPE_LPM_PIPE0:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_LPM_LKP_KEY0;
            break;

        case CTC_FTM_TBL_TYPE_LPM_PIPE1:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_LPM_LKP_KEY1;
            break;

        case CTC_FTM_TBL_TYPE_NEXTHOP:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_NEXTHOP;
            break;

        case CTC_FTM_TBL_TYPE_FWD:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_FWD;
            break;

        case CTC_FTM_TBL_TYPE_MET:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_MET;
            break;

        case CTC_FTM_TBL_TYPE_EDIT:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_EDIT;
            break;

        case CTC_FTM_TBL_TYPE_OAM_MEP:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_OAM_MEP;
            break;

        case CTC_FTM_TBL_TYPE_LM:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_OAM_LM;
            break;

        case CTC_FTM_TBL_TYPE_SCL_HASH_KEY:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_USERID_HASH_KEY;
            break;

        case CTC_FTM_TBL_TYPE_SCL_HASH_AD:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_USERID_AD;
            break;

        case CTC_FTM_TBL_TYPE_FIB_HASH_KEY:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_MAC_HASH_KEY;
            break;

        case CTC_FTM_TBL_TYPE_FIB0_HASH_KEY:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_FIB0_HASH_KEY;
            break;
        case CTC_FTM_TBL_TYPE_DSMAC_AD:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_DSMAC_AD;
            break;

        case CTC_FTM_TBL_TYPE_FIB1_HASH_KEY:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_FIB1_HASH_KEY;
            break;

        case CTC_FTM_TBL_TYPE_DSIP_AD:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_DSIP_AD;
            break;

        case CTC_FTM_TBL_TYPE_XCOAM_HASH_KEY:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY;
            break;
        case CTC_FTM_TBL_TYPE_FLOW_HASH_KEY:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_FLOW_HASH_KEY;
            break;
        case CTC_FTM_TBL_TYPE_FLOW_AD:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_FLOW_AD;
            break;
        case CTC_FTM_TBL_TYPE_IPFIX_HASH_KEY:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY;
            break;

        case CTC_FTM_TBL_TYPE_IPFIX_AD:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_IPFIX_AD;
            break;
        case CTC_FTM_TBL_TYPE_OAM_APS:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_OAM_APS;
            break;


        case CTC_FTM_TBL_TYPE_SCL1_HASH_KEY:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_USERID1_HASH_KEY;
            break;
        case CTC_FTM_TBL_TYPE_SCL1_HASH_AD:
            *drv_tbl_id = DRV_FTM_SRAM_TBL_USERID_AD1;
            break;

        default:
            *drv_tbl_id = 0xFF;
            break;
    }
    return DRV_E_NONE;
}

STATIC int32 _sys_usw_ftm_map_profile(uint8 lchip, ctc_ftm_profile_info_t* profile_info,
                                                                        drv_ftm_profile_info_t* drv_profile,
                                                                        uint8* tbl_id_map, uint8* key_id_map)
{
    uint16 tbl_index = 0;
    uint8 drv_key_id = 0;
    uint8 drv_key_size = 0;
    uint8 drv_tbl_id = 0;

    drv_profile->profile_type    = profile_info->profile_type;

    switch(profile_info->misc_info.ip_route_mode)
    {
        case CTC_FTM_ROUTE_MODE_DEFAULT:
            drv_profile->lpm_mode = LPM_MODEL_PRIVATE_IPDA_IPSA_FULL;
            break;
        case CTC_FTM_ROUTE_MODE_WITHOUT_RPF_LINERATE:
            drv_profile->lpm_mode = LPM_MODEL_PRIVATE_IPDA;
            break;
        case CTC_FTM_ROUTE_MODE_WITH_RPF_LINERATE:
            drv_profile->lpm_mode = LPM_MODEL_PRIVATE_IPDA_IPSA_HALF;
            break;
        case CTC_FTM_ROUTE_MODE_WITH_PUB_LINERATE:
            drv_profile->lpm_mode = LPM_MODEL_PUBLIC_IPDA_PRIVATE_IPDA_HALF;
            break;
        case CTC_FTM_ROUTE_MODE_WITH_PUB_AND_RPF:
            drv_profile->lpm_mode = LPM_MODEL_PUBLIC_IPDA_IPSA_PRIVATE_IPDA_IPSA_HALF;
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    drv_profile->scl_mode = profile_info->misc_info.scl_mode;

    drv_profile->key_info_size   = profile_info->key_info_size;
    drv_profile->tbl_info_size   = profile_info->tbl_info_size;

    if (drv_profile->key_info_size && profile_info->key_info)
    {
        for (tbl_index = 0; tbl_index < drv_profile->key_info_size; tbl_index++)
        {

            _sys_usw_ftm_map_key_id_info(profile_info->key_info[tbl_index].key_id,
                                         profile_info->key_info[tbl_index].key_size,
                                         &drv_key_id, &drv_key_size);
            sal_memcpy(&drv_profile->key_info[tbl_index], &profile_info->key_info[tbl_index], sizeof(drv_profile->key_info[tbl_index]));
            drv_profile->key_info[tbl_index].key_id = drv_key_id;
            drv_profile->key_info[tbl_index].key_size = drv_key_size;
            if(key_id_map)
            {
                key_id_map[drv_key_id] = tbl_index;
            }
        }
    }

    if(drv_profile->tbl_info_size && profile_info->tbl_info)
    {
        for (tbl_index = 0; tbl_index < profile_info->tbl_info_size; tbl_index++)
        {
            _sys_usw_ftm_map_tbl_id(profile_info->tbl_info[tbl_index].tbl_id, &drv_tbl_id);
            sal_memcpy(&drv_profile->tbl_info[tbl_index], &profile_info->tbl_info[tbl_index], sizeof(drv_profile->tbl_info[tbl_index]));
            drv_profile->tbl_info[tbl_index].tbl_id = drv_tbl_id;
            if(tbl_id_map)
            {
                tbl_id_map[drv_tbl_id] = tbl_index;
            }
        }
    }

    /*init couple mode*/
    drv_profile->couple_mode = CTC_FLAG_ISSET(profile_info->flag, CTC_FTM_FLAG_COUPLE_EN)?1:0;
	drv_profile->napt_enable =SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_NAPT)?1:0;
    if(!DRV_IS_DUET2(lchip))
    {
        drv_profile->mpls_mode = SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_MPLS)?((SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_MPLS)>8192)?2:1):0;
    }

    return CTC_E_NONE;
}
int32
sys_usw_ftm_mem_alloc(uint8 lchip, ctc_ftm_profile_info_t* profile_info)
{
    int32  ret = 0;
    uint8 realloc  = 0;
    drv_ftm_profile_info_t drv_profile;
    uint8  tmp_status = 0;
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(profile_info);

    if (NULL != g_usw_sys_ftm[lchip])
    {
        tmp_status = g_usw_sys_ftm[lchip]->status;
        if(0 == g_usw_sys_ftm[lchip]->status)
        {
            realloc = 1;
            if (0 == SDK_WORK_PLATFORM)
            {
                CTC_ERROR_RETURN(drv_usw_ftm_reset_dynamic_table(lchip));
                CTC_ERROR_RETURN(drv_usw_ftm_reset_tcam_table(lchip));
            }
#if (1 == SDK_WORK_PLATFORM)
            ctc_cmodel_deinit();
#endif
            CTC_ERROR_RETURN(drv_usw_ftm_mem_free(lchip));
            CTC_ERROR_RETURN(sys_usw_opf_deinit(lchip, g_usw_sys_ftm[lchip]->ftm_opf_type));

        }
        else
        {
            if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && tmp_status == 3)
            {
                drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
            }
            if (0 == SDK_WORK_PLATFORM)
            {
                CTC_ERROR_RETURN(drv_usw_ftm_reset_dynamic_table(lchip));
                CTC_ERROR_RETURN(drv_usw_ftm_reset_tcam_table(lchip));
            }
            if(g_usw_sys_ftm[lchip]->buffer_mode == 1 || g_usw_sys_ftm[lchip]->buffer_mode == 2)/*buffer changed*/
            {
                CTC_ERROR_RETURN(sys_usw_ftm_reset_bufferstore_register(lchip, g_usw_sys_ftm[lchip]->buffer_mode));
            }

            if (0 == SDK_WORK_PLATFORM)
            {
                CTC_ERROR_RETURN(drv_usw_ftm_reset_dynamic_table(lchip));
            }

            if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && tmp_status == 3)
            {
                drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
            }

        }
    }
    else
    {
        g_usw_sys_ftm[lchip] = mem_malloc(MEM_FTM_MODULE, sizeof(sys_ftm_master_t));

        if (NULL == g_usw_sys_ftm[lchip])
        {
            return DRV_E_NO_MEMORY;
        }
    }
    sal_memset(g_usw_sys_ftm[lchip], 0, sizeof(sys_ftm_master_t));
    sal_memset(&drv_profile, 0, sizeof(drv_ftm_profile_info_t));
    g_usw_sys_ftm[lchip]->profile_type = profile_info->profile_type;
    g_usw_sys_ftm[lchip]->status = tmp_status;
    if (profile_info->key_info_size && profile_info->key_info)
    {
        drv_profile.key_info = mem_malloc(MEM_FTM_MODULE, sizeof(drv_ftm_key_info_t)*profile_info->key_info_size);
        if(NULL == drv_profile.key_info)
        {
            ret =  DRV_E_NO_MEMORY;
            goto end;
        }
    }

    if(profile_info->tbl_info_size&& profile_info->tbl_info)
    {
        drv_profile.tbl_info = mem_malloc(MEM_FTM_MODULE, sizeof(ctc_ftm_tbl_info_t)*profile_info->tbl_info_size);
        if(NULL == drv_profile.tbl_info)
        {
            ret = DRV_E_NO_MEMORY;
            goto end;
        }
    }
    CTC_ERROR_GOTO(sys_usw_ftm_set_misc(lchip, profile_info), ret, end);
    CTC_ERROR_GOTO(_sys_usw_ftm_map_profile(lchip, profile_info, &drv_profile, NULL, NULL), ret, end);

    /*init ftm profile */
    CTC_ERROR_GOTO(drv_usw_ftm_mem_alloc(lchip, &drv_profile), ret, end);

    /*init sim module, now only uml emulation mode need(ctc-sim mode) */
#if (1 == SDK_WORK_PLATFORM)
    {
        /* just server need to init, and one server just have one cm */
        CTC_ERROR_GOTO(_sys_usw_ftm_init_sim_module(lchip), ret, end);
    }
#endif

    /*init table share */
    CTC_ERROR_GOTO(_sys_usw_ftm_opf_init_table_share(lchip), ret, end);

    if(!DRV_IS_DUET2(lchip))
    {
        CTC_ERROR_GOTO(sys_tsingma_ftm_reset_spec_value(lchip), ret, end);
    }
    MCHIP_CAP(SYS_CAP_SPEC_TUNNEL_ENTRY_NUM) = SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_TUNNEL);
    MCHIP_CAP(SYS_CAP_SPEC_MPLS_ENTRY_NUM) = SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_MPLS);
    MCHIP_CAP(SYS_CAP_SPEC_MAC_ENTRY_NUM) = SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_MAC);
    MCHIP_CAP(SYS_CAP_SPEC_IPMC_ENTRY_NUM) = SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_IPMC);
    MCHIP_CAP(SYS_CAP_SPEC_HOST_ROUTE_ENTRY_NUM) = SYS_FTM_SPEC(lchip, CTC_FTM_SPEC_IPUC_HOST);
    MCHIP_CAP(SYS_CAP_SPEC_MCAST_GROUP_NUM) = g_usw_sys_ftm[lchip]->glb_met_size;

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_GOTO(sys_usw_ftm_wb_restore(lchip), ret, end);
    }
    if(realloc)
    {
        CTC_ERROR_RETURN(sys_usw_ftm_lkp_register_init(lchip));
    }
end:
    if(NULL != drv_profile.tbl_info)
    {
        mem_free(drv_profile.tbl_info);
    }

    if(NULL != drv_profile.key_info)
    {
        mem_free(drv_profile.key_info);
    }
    return ret;
}

int32
sys_usw_ftm_mem_free(uint8 lchip)
{
    LCHIP_CHECK(lchip);

#if (1 == SDK_WORK_PLATFORM)
    ctc_cmodel_deinit();
#endif
    /*ecc deinit before ftm ,because some data relying on ftm.*/

    drv_usw_ftm_mem_free(lchip);

    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (NULL == g_usw_sys_ftm[lchip])
    {
        return CTC_E_NONE;
    }
    sys_usw_opf_deinit(lchip, g_usw_sys_ftm[lchip]->ftm_opf_type);

    /*Do not mem free for store current work status*/
    if(g_usw_sys_ftm[lchip]->status == 0)
    {
        mem_free(g_usw_sys_ftm[lchip]);
    }

    sys_usw_mchip_deinit(lchip);

    drv_deinit(lchip, 0);
    return CTC_E_NONE;
}

int32
sys_usw_ftm_register_callback(uint8 lchip, uint32 spec_type, sys_ftm_callback_t func)
{
    CTC_PTR_VALID_CHECK(g_usw_sys_ftm[lchip]);
    CTC_PTR_VALID_CHECK(func);
    SYS_FTM_INIT_CHECK(lchip);

    if (spec_type >= CTC_FTM_SPEC_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }

    g_usw_sys_ftm[lchip]->ftm_callback[spec_type] = func;

    return CTC_E_NONE;
}

int32
sys_usw_ftm_register_rsv_key_cb(uint8 lchip, uint32 hash_key_type, sys_ftm_rsv_hash_key_cb_t func)
{
    CTC_PTR_VALID_CHECK(g_usw_sys_ftm[lchip]);
    CTC_PTR_VALID_CHECK(func);
    SYS_FTM_INIT_CHECK(lchip);

    if (hash_key_type >= SYS_FTM_HASH_KEY_MAX)
    {
        return CTC_E_INVALID_PARAM;
    }

    g_usw_sys_ftm[lchip]->ftm_rsv_hash_key_cb[hash_key_type] = func;

    return CTC_E_NONE;
}
int32 sys_usw_ftm_adjust_flow_tcam(uint8 lchip, uint8 expand_blocks, uint8 compress_blocks)
{
    SYS_FTM_INIT_CHECK(lchip);
    CTC_ERROR_RETURN(drv_usw_ftm_adjust_flow_tcam(lchip, expand_blocks, compress_blocks));

    return CTC_E_NONE;
}

int32 sys_usw_ftm_reset_bufferstore_register(uint8 lchip, uint8  mode)
{
    sys_ftm_reset_register_t *p_register = NULL;
    uint32 cmd = 0;
    uint32 value = 0;
    int32 ret = 0;
    uint16 loop = 0;
    uint16 loop1 = 0;
    uint32 table_id[SYS_FTM_CHANGE_TABLE_NUM] = {DsIrmPortCfg_t,DsIrmPortFlowControlThrdProfile_t, DsIrmPortLimitedThrdProfile_t,
            DsIrmPortStallCfg_t, DsIrmPortStallEn_t, DsIrmPortTcCfg_t,DsIrmPortTcFlowControlThrdProfile_t,DsIrmPortTcLimitedThrdProfile_t,
             DsSrcSgmacGroup_t,DsIrmColorDpMap_t,DsIrmPrioScTcMap_t,DsIrmChannel_t,BufStorePktSizeChkCtl_t,BufferStoreCpuRxLogChannelMap_t,
             BufferStoreCtl_t,IrmMiscCtl_t,DsIrmScThrd_t,IrmResrcMonScanCtl_t};
    uint32 table_size = sizeof(ds_t);
    ShareBufferCtl_m ShareBufferCtl;
    SYS_FTM_INIT_CHECK(lchip);

    sal_memset(&ShareBufferCtl, 0, sizeof(ShareBufferCtl_m));
    p_register = mem_malloc(MEM_FTM_MODULE, sizeof(sys_ftm_reset_register_t) * SYS_FTM_CHANGE_TABLE_NUM);
    if (NULL == p_register)
    {
        ret = CTC_E_NO_MEMORY;
        goto error;
    }
    sal_memset(p_register, 0, sizeof(sys_ftm_reset_register_t) * SYS_FTM_CHANGE_TABLE_NUM);
    /* read old table value */
    for(loop = 0; loop < SYS_FTM_CHANGE_TABLE_NUM; loop ++)
    {
        p_register[loop].tbl_id = table_id[loop];
        sys_usw_ftm_query_table_entry_num(lchip, table_id[loop], &value);
        p_register[loop].max_index = value;
        p_register[loop].p_value = mem_malloc(MEM_FTM_MODULE, table_size * value);
        if (NULL == p_register[loop].p_value)
        {
            ret = CTC_E_NO_MEMORY;
            goto error;
        }
        sal_memset(p_register[loop].p_value, 0, table_size * value);
        for(loop1 = 0; loop1 < value; loop1 ++)
        {
            cmd = DRV_IOR(table_id[loop], DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, loop1, cmd, (uint8*)p_register[loop].p_value + table_size*loop1),ret, error);
        }
    }
    /* reset buffer store */
    value = 1;
    cmd = DRV_IOW(RlmBsrCtlReset_t, RlmBsrCtlReset_resetCoreBufStore_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret, error);
    value = 0;
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret, error);
    /* change profile table value */
    value = (mode==2) ? 0xFFF : 0x1FFF;
    cmd = DRV_IOW(BufStoreFreeListCtl_t, BufStoreFreeListCtl_freeListTailPtrCfg_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret, error);

    value = 0;
    cmd = DRV_IOW(BufStoreInit_t, BufStoreInit_init_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret, error);
    value = 1;
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret, error);

    value = 0;
    cmd = DRV_IOW(ShareBufferInit_t, ShareBufferInit_init_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret, error);
    value = 1;
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret, error);

#if (0 == SDK_WORK_PLATFORM)
    cmd = DRV_IOR(BufStoreInitDone_t, BufStoreInitDone_initDone_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret, error);
    if (!value)
    {
        SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        ret = CTC_E_NOT_INIT;
        goto error;
    }
    cmd = DRV_IOR(ShareBufferInitDone_t, ShareBufferInitDone_initDone_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret, error);
    if (!value)
    {
        SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");
        ret = CTC_E_NOT_INIT;
        goto error;
    }
#endif

    /* write old table value */
    for(loop = 0; loop < SYS_FTM_CHANGE_TABLE_NUM; loop ++)
    {
        for(loop1 = 0; loop1 < p_register[loop].max_index; loop1 ++)
        {
            cmd = DRV_IOW(table_id[loop], DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, loop1, cmd, (uint8*)p_register[loop].p_value + table_size*loop1),ret, error);
        }
    }
    value = 20;
    cmd = DRV_IOW(DsIrmPortGuaranteedThrdProfile_t, DsIrmPortGuaranteedThrdProfile_portGuaranteedThrd_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret, error);

    for (loop = 0; loop < 3; loop++)
    {
        value =  0x3FF;
        cmd = DRV_IOW(DsIrmScCngThrdProfile_t, DsIrmScCngThrdProfile_g_0_scCngThrd_f + loop);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret, error);
    }
    value =  0x500;
    cmd = DRV_IOW(DsIrmMiscThrd_t, DsIrmMiscThrd_c2cThrd_f );
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret, error);
    cmd = DRV_IOW(DsIrmMiscThrd_t, DsIrmMiscThrd_criticalThrd_f );
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret, error);
    value =  0x7fff;
    cmd = DRV_IOW(DsIrmMiscThrd_t, DsIrmMiscThrd_totalThrd_f );
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret, error);

    value =  (mode == 2) ? 0x3200 : 0x7200;
    cmd = DRV_IOW(DsErmMiscThrd_t, DsErmMiscThrd_totalThrd_f );
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret, error);
    value =  (mode == 2) ? 0x540 : 0xD40;
    cmd = DRV_IOW(DsErmScThrd_t, DsErmScThrd_g_0_scThrd_f );
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret, error);
error:
    for(loop = 0; loop < SYS_FTM_CHANGE_TABLE_NUM; loop ++)
    {
        if (p_register[loop].p_value)
        {
            mem_free(p_register[loop].p_value);
        }
    }
    if (p_register)
    {
        mem_free(p_register);
    }

    return ret;
}
#define ____hash_poly____

int32
sys_usw_ftm_get_current_hash_poly_type(uint8 lchip, uint32 mem_id, uint8 sram_type, uint32 *poly_type)
{
    uint32 type = 0;
    drv_usw_ftm_get_hash_poly(lchip, mem_id, sram_type, &type);
    switch (sram_type)
    {
        case DRV_FTM_SRAM_TBL_FIB0_HASH_KEY:

            if (0 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_1;
            }
            else if (1 == type)
            {
               *poly_type = CTC_FTM_HASH_POLY_TYPE_15;
            }
            else if (2 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_16;
            }
            else if (3 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_4;
            }
            else if (4 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_17;
            }
            else if (5 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_3;
            }
            else if (6 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_5;
            }
            else if (7 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_6;
            }
            else if (8 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_7;
            }
            break;

        case DRV_FTM_SRAM_TBL_FIB1_HASH_KEY:
        case DRV_FTM_SRAM_TBL_USERID_HASH_KEY:
            if (0 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_14;
            }
            else if (1 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_18;
            }
            else if (2 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_19;
            }
            else if (3 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_1;
            }
            else if (4 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_15;
            }
            else if (5 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_16;
            }
            else if (6 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_4;
            }
            else if (7 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_17;
            }
            else if (8 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_3;
            }
            break;

        case DRV_FTM_SRAM_TBL_FLOW_HASH_KEY:
            if (0 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_1;
            }
            else if (1 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_15;
            }
            else if (2 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_16;
            }
            else if (3 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_4;
            }
            else if (4 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_17;
            }
            else if (5 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_3;
            }
            else if (6 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_5;
            }
            else if (7 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_6;
            }
            else if (8 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_7;
            }
            break;
        case DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY:
            if (0 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_1;
            }
            else if (1 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_4;
            }
            else if (2 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_5;
            }
            break;

        default :
            break;
    }

    return DRV_E_NONE;

}

int32
sys_usw_ftm_get_hash_poly_cap(uint8 lchip, uint8 sram_type, ctc_ftm_hash_poly_t* hash_poly)
{
    uint32 couple = 0;


    CTC_PTR_VALID_CHECK(hash_poly);

    /*get hash poly capablity*/
    DRV_IF_ERROR_RETURN(drv_usw_get_dynamic_ram_couple_mode(lchip, sram_type, &couple));

    switch (sram_type)/*refer to _sys_usw_ftm_get_hash_type*/
    {
        case DRV_FTM_SRAM_TBL_FIB0_HASH_KEY:
            if(hash_poly->mem_id < DRV_FTM_SRAM3)
            {
                hash_poly->poly_num = 3;
                hash_poly->poly_type[0] = couple ? CTC_FTM_HASH_POLY_TYPE_5 : CTC_FTM_HASH_POLY_TYPE_4;
                hash_poly->poly_type[1] = couple ? CTC_FTM_HASH_POLY_TYPE_6 : CTC_FTM_HASH_POLY_TYPE_17;
                hash_poly->poly_type[2] = couple ? CTC_FTM_HASH_POLY_TYPE_7 : CTC_FTM_HASH_POLY_TYPE_3;
            }
            else if(hash_poly->mem_id < DRV_FTM_SRAM6)
            {
                hash_poly->poly_num = 3;
                hash_poly->poly_type[0] = couple ? CTC_FTM_HASH_POLY_TYPE_4 : CTC_FTM_HASH_POLY_TYPE_1;
                hash_poly->poly_type[1] = couple ? CTC_FTM_HASH_POLY_TYPE_17 : CTC_FTM_HASH_POLY_TYPE_15;
                hash_poly->poly_type[2] = couple ? CTC_FTM_HASH_POLY_TYPE_3 : CTC_FTM_HASH_POLY_TYPE_16;
            }
            else
            {
                return DRV_E_INVALID_MEM;
            }
            break;

        case DRV_FTM_SRAM_TBL_FIB1_HASH_KEY:
        case DRV_FTM_SRAM_TBL_USERID_HASH_KEY:
            if(hash_poly->mem_id < DRV_FTM_SRAM6 || hash_poly->mem_id > DRV_FTM_SRAM10)
            {
                return DRV_E_INVALID_MEM;
            }

            if(hash_poly->mem_id < DRV_FTM_SRAM8)
            {
                hash_poly->poly_num = 3;
                hash_poly->poly_type[0] = couple ? CTC_FTM_HASH_POLY_TYPE_4 : CTC_FTM_HASH_POLY_TYPE_1;
                hash_poly->poly_type[1] = couple ? CTC_FTM_HASH_POLY_TYPE_17 : CTC_FTM_HASH_POLY_TYPE_15;
                hash_poly->poly_type[2] = couple ? CTC_FTM_HASH_POLY_TYPE_3 : CTC_FTM_HASH_POLY_TYPE_16;
            }
            else if(hash_poly->mem_id < DRV_FTM_SRAM10)
            {
                hash_poly->poly_num = couple ? 3: 2;
                hash_poly->poly_type[0] = couple ?  CTC_FTM_HASH_POLY_TYPE_1 : CTC_FTM_HASH_POLY_TYPE_18;
                hash_poly->poly_type[1] = couple ?  CTC_FTM_HASH_POLY_TYPE_15 : CTC_FTM_HASH_POLY_TYPE_19;
                hash_poly->poly_type[2] = CTC_FTM_HASH_POLY_TYPE_16;
            }
            else
            {
                hash_poly->poly_num = couple ? 2 : 1;
                hash_poly->poly_type[0] = couple ?  CTC_FTM_HASH_POLY_TYPE_18 : CTC_FTM_HASH_POLY_TYPE_14;
                hash_poly->poly_type[1] = CTC_FTM_HASH_POLY_TYPE_19;
            }
            break;

        case DRV_FTM_SRAM_TBL_FLOW_HASH_KEY:
            if(hash_poly->mem_id < DRV_FTM_SRAM3)
            {
                hash_poly->poly_num = 3;
                hash_poly->poly_type[0] = couple ? CTC_FTM_HASH_POLY_TYPE_5 : CTC_FTM_HASH_POLY_TYPE_4;
                hash_poly->poly_type[1] = couple ? CTC_FTM_HASH_POLY_TYPE_6 : CTC_FTM_HASH_POLY_TYPE_17;
                hash_poly->poly_type[2] = couple ? CTC_FTM_HASH_POLY_TYPE_7 : CTC_FTM_HASH_POLY_TYPE_3;
            }
            else if(hash_poly->mem_id < DRV_FTM_SRAM5)
            {
                hash_poly->poly_num = 3;
                hash_poly->poly_type[0] = couple ? CTC_FTM_HASH_POLY_TYPE_4 : CTC_FTM_HASH_POLY_TYPE_1;
                hash_poly->poly_type[1] = couple ? CTC_FTM_HASH_POLY_TYPE_17 : CTC_FTM_HASH_POLY_TYPE_15;
                hash_poly->poly_type[2] = couple ? CTC_FTM_HASH_POLY_TYPE_3 : CTC_FTM_HASH_POLY_TYPE_16;
            }
            else
            {
                return DRV_E_INVALID_MEM;
            }
            break;

        case DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY:
            hash_poly->poly_num = 1;
            if(hash_poly->mem_id == DRV_FTM_SRAM2)
            {
                hash_poly->poly_type[0] = couple? CTC_FTM_HASH_POLY_TYPE_5 : CTC_FTM_HASH_POLY_TYPE_4;
            }
            else if(hash_poly->mem_id == DRV_FTM_SRAM5)
            {
                hash_poly->poly_type[0] = couple? CTC_FTM_HASH_POLY_TYPE_4 : CTC_FTM_HASH_POLY_TYPE_1;
            }
            else
            {
                return DRV_E_INVALID_MEM;
            }
            break;


        case DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY:
            hash_poly->poly_num = 2;

            if(hash_poly->mem_id == DRV_FTM_SRAM0)
            {
                hash_poly->poly_type[0] = couple? CTC_FTM_HASH_POLY_TYPE_3 : CTC_FTM_HASH_POLY_TYPE_1;
                hash_poly->poly_type[1] = couple? CTC_FTM_HASH_POLY_TYPE_4 : CTC_FTM_HASH_POLY_TYPE_15;
            }
            else if(hash_poly->mem_id == DRV_FTM_SRAM5)
            {
                hash_poly->poly_type[0] = couple? CTC_FTM_HASH_POLY_TYPE_1 : CTC_FTM_HASH_POLY_TYPE_18;
                hash_poly->poly_type[1] = couple? CTC_FTM_HASH_POLY_TYPE_15 : CTC_FTM_HASH_POLY_TYPE_19;
            }
            else
            {
                return DRV_E_INVALID_MEM;
            }
            break;

        default:
            break;
    }

    return CTC_E_NONE;
}


int32
sys_usw_ftm_mapping_drv_hash_poly_type(uint8 lchip, uint8 sram_type, uint32 type, uint32* p_poly)
{
    switch (sram_type)
    {
        case DRV_FTM_SRAM_TBL_FIB0_HASH_KEY:

            if (CTC_FTM_HASH_POLY_TYPE_1 == type)
            {
                *p_poly = 0;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_15 == type)
            {
                *p_poly = 1;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_16 == type)
            {
                *p_poly = 2;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_4 == type)
            {
                *p_poly = 3;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_17 == type)
            {
                *p_poly = 4;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_3 == type)
            {
                *p_poly = 5;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_5 == type)
            {
                *p_poly = 6;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_6 == type)
            {
                *p_poly = 7;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_7 == type)
            {
                *p_poly = 8;
            }
            break;

        case DRV_FTM_SRAM_TBL_FIB1_HASH_KEY:
            if (CTC_FTM_HASH_POLY_TYPE_14 == type)
            {
                *p_poly = 0;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_18 == type)
            {
                *p_poly = 1;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_19 == type)
            {
                *p_poly = 2;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_1 == type)
            {
                *p_poly = 3;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_15 == type)
            {
                *p_poly = 4;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_16 == type)
            {
                *p_poly = 5;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_4 == type)
            {
                *p_poly = 6;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_17 == type)
            {
                *p_poly = 7;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_3 == type)
            {
                *p_poly = 8;
            }
            break;

        case DRV_FTM_SRAM_TBL_FLOW_HASH_KEY:
            if (CTC_FTM_HASH_POLY_TYPE_1 == type)
            {
                *p_poly = 0;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_15 == type)
            {
                *p_poly = 1;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_16 == type)
            {
                *p_poly = 2;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_4 == type)
            {
                *p_poly = 3;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_17 == type)
            {
                *p_poly = 4;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_3 == type)
            {
                *p_poly = 5;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_5 == type)
            {
                *p_poly = 6;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_6 == type)
            {
                *p_poly = 7;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_7 == type)
            {
                *p_poly = 8;
            }
            break;

        case DRV_FTM_SRAM_TBL_USERID_HASH_KEY:
            if (CTC_FTM_HASH_POLY_TYPE_14 == type)
            {
                *p_poly = 0;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_18 == type)
            {
                *p_poly = 1;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_19 == type)
            {
                *p_poly = 2;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_1 == type)
            {
                *p_poly = 3;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_15 == type)
            {
                *p_poly = 4;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_16 == type)
            {
                *p_poly = 5;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_4 == type)
            {
                *p_poly = 6;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_17 == type)
            {
                *p_poly = 7;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_3 == type)
            {
                *p_poly = 8;
            }
            break;

        case DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY:
            if (CTC_FTM_HASH_POLY_TYPE_1 == type)
            {
                *p_poly = 0;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_4 == type)
            {
                *p_poly = 1;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_5 == type)
            {
                *p_poly = 2;
            }

            break;

        case DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY:
            if (CTC_FTM_HASH_POLY_TYPE_18 == type)
            {
                *p_poly = 0;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_19 == type)
            {
                *p_poly = 1;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_1 == type)
            {
                *p_poly = 2;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_15 == type)
            {
                *p_poly = 3;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_4 == type)
            {
                *p_poly = 4;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_3 == type)
            {
                *p_poly = 5;
            }
            break;

        default :
            break;
    }

    return DRV_E_NONE;
}



int32
sys_tsingma_ftm_mapping_drv_hash_poly_type(uint8 lchip, uint8 sram_type, uint32 type, uint32* p_poly)
{
    switch (sram_type)
    {
        case DRV_FTM_SRAM_TBL_MAC_HASH_KEY:
        case DRV_FTM_SRAM_TBL_FIB0_HASH_KEY:

            if (CTC_FTM_HASH_POLY_TYPE_18 == type)
            {    /**< [D2.TM]x10+x4+x3+x1+1*/
                *p_poly = 0;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_15 == type)
            {   /**< [D2.TM]x11+x4+x2+x+1*/
                *p_poly = 1;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_16 == type)
            {   /**< [D2.TM]x11+x5+x3+x+1*/
                *p_poly = 2;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_4 == type)
            {   /**< [GG.D2.TM]x12+x6+x4+x+1*/
                *p_poly = 3;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_17 == type)
            {   /**< [D2.TM]x12+x7+x4+x3+1*/
                *p_poly = 4;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_3 == type)
            {   /**< [GG.D2.TM]x12+x8+x2+x+1*/
                *p_poly = 5;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_5 == type)
            {   /**< [GG.D2.TM]x13+x4+x3+x+1*/
                *p_poly = 6;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_6 == type)
            {   /**< [GG.D2.TM]x13+x5+x2+x+1*/
                *p_poly = 7;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_22 == type)
            {  /**< [TM]x13+x6+x4+x+1*/
                *p_poly = 8;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_7 == type)
            {  /**< [GG.D2.TM]x13+x7+x6+x+1*/
                *p_poly = 9;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_23 == type)
            {  /**< [TM]x14+x6+x4+x3+x2+x1+1*/
                *p_poly = 10;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_24 == type)
            {  /**< [TM]x14+x6+x5+x4+x3+x1+1*/
                *p_poly = 11;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_25 == type)
            {  /**< [TM]x14+x7+x5+x3+x2+x1+1*/
                *p_poly = 12;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_26 == type)
            {  /**< [TM]x14+x7+x5+x4+x3+x1+1*/
                *p_poly = 13;
            }


            break;

        case DRV_FTM_SRAM_TBL_FIB1_HASH_KEY:
            if (CTC_FTM_HASH_POLY_TYPE_15 == type)
            {   /**< [D2.TM]x11+x4+x2+x+1*/
                *p_poly = 0;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_16 == type)
            {    /**< [D2.TM]x11+x5+x3+x+1*/
                *p_poly = 1;
            }

            break;

        case DRV_FTM_SRAM_TBL_FLOW_HASH_KEY:
            if (CTC_FTM_HASH_POLY_TYPE_4 == type)
            {/**< [GG.D2.TM]x12+x6+x4+x+1*/
                *p_poly = 0;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_3 == type)
            {/**< [GG.D2.TM]x12+x8+x2+x+1*/
                *p_poly = 1;
            }

            break;

        case DRV_FTM_SRAM_TBL_USERID_HASH_KEY:
            if (CTC_FTM_HASH_POLY_TYPE_5 == type)
            {/**< [GG.D2.TM]x13+x4+x3+x+1*/
                *p_poly = 0;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_3 == type)
            { /**< [GG.D2.TM]x12+x8+x2+x+1*/
                *p_poly = 1;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_15 == type)
            { /**< [D2.TM]x11+x4+x2+x+1*/
                *p_poly = 2;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_16 == type)
            {/**< [D2.TM]x11+x5+x3+x+1*/
                *p_poly = 3;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_20 == type)
            {/**< [TM]x11+x6+x2+x+1*/
                *p_poly = 4;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_21 == type)
            { /**< [TM]x11+x6+x5+x+1*/
                *p_poly = 5;
            }

            break;

        case DRV_FTM_SRAM_TBL_USERID1_HASH_KEY:
            if (CTC_FTM_HASH_POLY_TYPE_18 == type)
            { /**< [D2.TM]x10+x4+x3+x1+1*/
                *p_poly = 0;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_15 == type)
            { /**< [D2.TM]x11+x4+x2+x+1*/
                *p_poly = 1;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_16 == type)
            {/**< [D2.TM]x11+x5+x3+x+1*/
                *p_poly = 2;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_20 == type)
            {/**< [TM]x11+x6+x2+x+1*/
                *p_poly = 3;
            }

            break;


        case DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY:
            if (CTC_FTM_HASH_POLY_TYPE_15 == type)
            {/**< [D2.TM]x11+x4+x2+x+1*/
                *p_poly = 0;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_1 == type)
            {    /**< [GG.D2.TM]x11+x2+1*/
                *p_poly = 1;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_4 == type)
            {/**< [GG.D2.TM]x12+x6+x4+x+1*/
                *p_poly = 2;
            }
            else if (CTC_FTM_HASH_POLY_TYPE_3 == type)
            {/**< [GG.D2.TM]x12+x8+x2+x+1*/
                *p_poly = 3;
            }

            break;

        default :
            break;
    }

    return DRV_E_NONE;
}

int32
sys_tsingma_ftm_get_current_hash_poly_type(uint8 lchip, uint32 mem_id, uint8 sram_type, uint32 *poly_type)
{
    uint32 type = 0;
    drv_usw_ftm_get_hash_poly(lchip, mem_id, sram_type, &type);

    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "mem_id:%d, type:%d, drv_poly:%d\n", mem_id, sram_type, type);
    switch (sram_type)
    {
        case DRV_FTM_SRAM_TBL_FIB0_HASH_KEY:
        case DRV_FTM_SRAM_TBL_MAC_HASH_KEY:
            if (0 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_18;
            }
            else if (1 == type)
            {
               *poly_type = CTC_FTM_HASH_POLY_TYPE_15;
            }
            else if (2 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_16;
            }
            else if (3 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_4;
            }
            else if (4 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_17;
            }
            else if (5 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_3;
            }
            else if (6 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_5;
            }
            else if (7 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_6;
            }
            else if (8 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_22;
            }
            else if (9 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_7;
            }
            else if (10 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_23;
            }
            else if (11 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_24;
            }
            else if (12 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_25;
            }
            else if (13 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_26;
            }


            break;

        case DRV_FTM_SRAM_TBL_FIB1_HASH_KEY:

            if (0 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_15;
            }
            else if (1 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_16;
            }

            break;


        case DRV_FTM_SRAM_TBL_FLOW_HASH_KEY:

            if (0 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_4;
            }
            else if (1 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_3;
            }
            break;


        case DRV_FTM_SRAM_TBL_USERID_HASH_KEY:

            if (0 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_5;
            }
            else if (1 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_3;
            }
            else if (2 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_15;
            }
            else if (3 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_16;
            }
            else if (4 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_20;
            }
            else if (5 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_21;
            }

            break;

        case DRV_FTM_SRAM_TBL_USERID1_HASH_KEY:

            if (0 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_18;
            }
            else if (1 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_15;
            }
            else if (2 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_16;
            }
            else if (3 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_20;
            }

            break;

        case DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY:
            if (0 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_15;
            }
            else if (1 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_1;
            }
            else if (2 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_4;
            }
            else if (3 == type)
            {
                *poly_type = CTC_FTM_HASH_POLY_TYPE_3;
            }

            break;

        default :
            break;
    }

    return DRV_E_NONE;

}


int32
sys_tsingma_ftm_get_hash_poly_cap(uint8 lchip, uint8 sram_type, ctc_ftm_hash_poly_t* hash_poly)
{
    uint32 size = 0;
    drv_ftm_mem_info_t mem_info;

    CTC_PTR_VALID_CHECK(hash_poly);

    /*get hash poly capablity*/
    sal_memset(&mem_info, 0, sizeof(mem_info));

    mem_info.info_type = DRV_FTM_MEM_INFO_ENTRY_ENUM;
    mem_info.mem_id =  hash_poly->mem_id;

    drv_ftm_get_mem_info(lchip, &mem_info);

    size = mem_info.entry_num;

    switch (sram_type)/*refer to _sys_usw_ftm_get_hash_type*/
    {
        case DRV_FTM_SRAM_TBL_FIB0_HASH_KEY:
        case DRV_FTM_SRAM_TBL_MAC_HASH_KEY:
            {
                switch(size)
                {
                case 4*1024:
                    hash_poly->poly_num = 1;
                    hash_poly->poly_type[0] = CTC_FTM_HASH_POLY_TYPE_18;
                    break;

                case 8*1024:
                    hash_poly->poly_num = 2;
                    hash_poly->poly_type[0] = CTC_FTM_HASH_POLY_TYPE_15;
                    hash_poly->poly_type[1] = CTC_FTM_HASH_POLY_TYPE_16;
                    break;

                case 16*1024:
                    hash_poly->poly_num = 3;
                    hash_poly->poly_type[0] = CTC_FTM_HASH_POLY_TYPE_4;
                    hash_poly->poly_type[1] = CTC_FTM_HASH_POLY_TYPE_17;
                    hash_poly->poly_type[2] = CTC_FTM_HASH_POLY_TYPE_3;
                    break;

                case 32*1024:
                    hash_poly->poly_num = 4;
                    hash_poly->poly_type[0] = CTC_FTM_HASH_POLY_TYPE_5;
                    hash_poly->poly_type[1] = CTC_FTM_HASH_POLY_TYPE_6;
                    hash_poly->poly_type[2] = CTC_FTM_HASH_POLY_TYPE_22;
                    hash_poly->poly_type[3] = CTC_FTM_HASH_POLY_TYPE_7;
                    break;

                case 64*1024:
                    hash_poly->poly_num = 4;
                    hash_poly->poly_type[0] = CTC_FTM_HASH_POLY_TYPE_23;
                    hash_poly->poly_type[1] = CTC_FTM_HASH_POLY_TYPE_24;
                    hash_poly->poly_type[2] = CTC_FTM_HASH_POLY_TYPE_25;
                    hash_poly->poly_type[3] = CTC_FTM_HASH_POLY_TYPE_26;
                    break;

                default:
                    return DRV_E_INVALID_MEM;
                }
            }

            break;

        case DRV_FTM_SRAM_TBL_FIB1_HASH_KEY:
            {
                switch(size)
                {
                case 8*1024:
                    hash_poly->poly_num = 2;
                    hash_poly->poly_type[0] = CTC_FTM_HASH_POLY_TYPE_15;
                    hash_poly->poly_type[1] = CTC_FTM_HASH_POLY_TYPE_16;
                    break;

                default:
                    return DRV_E_INVALID_MEM;
                }
            }

            break;

        case DRV_FTM_SRAM_TBL_USERID_HASH_KEY:
            {
                switch(size)
                {
                case 8*1024:
                    hash_poly->poly_num = 4;
                    hash_poly->poly_type[0] = CTC_FTM_HASH_POLY_TYPE_15;
                    hash_poly->poly_type[1] = CTC_FTM_HASH_POLY_TYPE_16;
                    hash_poly->poly_type[2] = CTC_FTM_HASH_POLY_TYPE_20;
                    hash_poly->poly_type[3] = CTC_FTM_HASH_POLY_TYPE_21;
                    break;

                case 16*1024:
                    hash_poly->poly_num = 1;
                    hash_poly->poly_type[0] = CTC_FTM_HASH_POLY_TYPE_3;

                    break;

                case 32*1024:
                    hash_poly->poly_num = 1;
                    hash_poly->poly_type[0] = CTC_FTM_HASH_POLY_TYPE_5;
                    break;

                default:
                    return DRV_E_INVALID_MEM;
                }
            }

            break;

        case DRV_FTM_SRAM_TBL_USERID1_HASH_KEY:
            {
                switch(size)
                {
                case 4*1024:
                    hash_poly->poly_num = 1;
                    hash_poly->poly_type[0] = CTC_FTM_HASH_POLY_TYPE_18;
                    break;

                case 8*1024:
                    hash_poly->poly_num = 3;
                    hash_poly->poly_type[0] = CTC_FTM_HASH_POLY_TYPE_15;
                    hash_poly->poly_type[1] = CTC_FTM_HASH_POLY_TYPE_16;
                    hash_poly->poly_type[2] = CTC_FTM_HASH_POLY_TYPE_20;
                    break;

                default:
                    return DRV_E_INVALID_MEM;
                }
            }

            break;


        case DRV_FTM_SRAM_TBL_FLOW_HASH_KEY:
            {
                switch(size)
                {
                case 16*1024:
                    hash_poly->poly_num = 2;
                    hash_poly->poly_type[0] = CTC_FTM_HASH_POLY_TYPE_4;
                    hash_poly->poly_type[1] = CTC_FTM_HASH_POLY_TYPE_3;
                    break;

                default:
                    return DRV_E_INVALID_MEM;
                }
            }

            break;

        case DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY:
            {
                switch(size)
                {
                case 8*1024:
                    hash_poly->poly_num = 2;
                    hash_poly->poly_type[0] = CTC_FTM_HASH_POLY_TYPE_15;
                    hash_poly->poly_type[1] = CTC_FTM_HASH_POLY_TYPE_1;
                    break;

                case 16*1024:
                    hash_poly->poly_num = 2;
                    hash_poly->poly_type[0] = CTC_FTM_HASH_POLY_TYPE_4;
                    hash_poly->poly_type[1] = CTC_FTM_HASH_POLY_TYPE_3;
                    break;

                default:
                    return DRV_E_INVALID_MEM;
                }
            }

            break;

        default:
            break;
    }

    return CTC_E_NONE;
}



int32
_sys_usw_ftm_check_hash_mem_empty(uint8 lchip, uint8 sram_type)
{
    uint8 step = 0;
    uint32 value = 0;
    uint32 cmd = 0;
    uint32 mem_entry_num = 0;
    uint32 loop =0 ;
    uint32 tbl_id = 0;
    uint32 fld_id = 0;

    switch(sram_type)
    {
        case DRV_FTM_SRAM_TBL_MAC_HASH_KEY:
        tbl_id = DsFibHost0MacHashKey_t;
        fld_id = DsFibHost0MacHashKey_valid_f;
        step = 1;
        break;


        case DRV_FTM_SRAM_TBL_FIB0_HASH_KEY:
        tbl_id = DsFibHost0FcoeHashKey_t;
        fld_id = DsFibHost0FcoeHashKey_valid_f;
        step = 1;
        break;

        case DRV_FTM_SRAM_TBL_FIB1_HASH_KEY:
        tbl_id = DsFibHost1FcoeRpfHashKey_t;
        fld_id = DsFibHost1FcoeRpfHashKey_valid_f;
        step = 1;
        break;

        case DRV_FTM_SRAM_TBL_FLOW_HASH_KEY:
        tbl_id = DsFlowL2HashKey_t;
        fld_id = DsFlowL2HashKey_hashKeyType_f;
        step = 2;
        break;

        case DRV_FTM_SRAM_TBL_USERID_HASH_KEY:
        tbl_id = DsUserIdCvlanCosPortHashKey_t;
        fld_id = DsUserIdCvlanCosPortHashKey_valid_f;
        step = 1;
        break;


        case DRV_FTM_SRAM_TBL_USERID1_HASH_KEY:
        tbl_id = DsUserId1CvlanCosPortHashKey_t;
        fld_id = DsUserId1CvlanCosPortHashKey_valid_f;
        step = 1;
        break;

        case DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY:
        tbl_id = DsEgressXcOamBfdHashKey_t;
        fld_id = DsEgressXcOamBfdHashKey_valid_f;
        step = 2;
        break;
        default :
        break;
    }
    mem_entry_num = TABLE_MAX_INDEX(lchip, tbl_id);
    cmd = DRV_IOR(tbl_id, fld_id);
    for (loop = 0; loop < mem_entry_num; loop += step)
    {
        DRV_IOCTL(lchip, loop, cmd, &value);
        if (value)
        {
            SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Entry already exist \n");
			return CTC_E_EXIST;

        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_ftm_set_rsv_hash_key(uint8 lchip, uint8 is_add)
{
    uint8 i = 0;
    for (i = 0; i < SYS_FTM_HASH_KEY_MAX; i++)
    {
        if (NULL != g_usw_sys_ftm[lchip]->ftm_rsv_hash_key_cb[i])
        {
            g_usw_sys_ftm[lchip]->ftm_rsv_hash_key_cb[i](lchip, is_add);
        }
    }
    return CTC_E_NONE;
}

int32
sys_usw_ftm_get_hash_poly(uint8 lchip, ctc_ftm_hash_poly_t* hash_poly)
{

    uint8 sram_type = 0;

    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(hash_poly);
    SYS_FTM_INIT_CHECK(lchip);


    CTC_MAX_VALUE_CHECK(hash_poly->mem_id, DRV_FTM_SRAM_MAX);

    CTC_ERROR_RETURN(drv_usw_ftm_get_sram_type(lchip, hash_poly->mem_id, &sram_type));
    if((DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY == sram_type) || (DRV_FTM_SRAM_TBL_MAX == sram_type))
    {
        SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;

    }

    /*get current hash poly*/
     /*sys_usw_ftm_get_current_hash_poly_type(lchip, hash_poly->mem_id, sram_type, &hash_poly->cur_poly_type);//get enum type from asic type*/
    MCHIP_API(lchip)->ftm_mapping_drv_to_ctc_hash_poly(lchip, hash_poly->mem_id, sram_type, &hash_poly->cur_poly_type);

     /*_sys_usw_ftm_get_hash_poly(lchip, sram_type, hash_poly);*/
    MCHIP_API(lchip)->ftm_get_hash_poly_cap(lchip, sram_type, hash_poly);
    return CTC_E_NONE;
}


int32
sys_usw_ftm_set_hash_poly(uint8 lchip, ctc_ftm_hash_poly_t* hash_poly)
{

    uint32 poly_type = 0;
    uint8 sram_type = 0;

    uint32 loop = 0;
    int32 ret = CTC_E_NONE;
    ctc_ftm_hash_poly_t poly_capabilty;

    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_FTM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(hash_poly);

    CTC_MAX_VALUE_CHECK(hash_poly->mem_id, DRV_FTM_SRAM_MAX);

    sal_memset(&poly_capabilty, 0, sizeof(ctc_ftm_hash_poly_t));
    poly_capabilty.mem_id = hash_poly->mem_id;


    CTC_ERROR_RETURN(drv_usw_ftm_get_sram_type(lchip, hash_poly->mem_id, &sram_type));
    if((DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY == sram_type) || (DRV_FTM_SRAM_TBL_MAX == sram_type))
    {
        SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " API or some feature is not supported \n");
        return CTC_E_NOT_SUPPORT;

    }
    /*get hash poly capabilty*/
    /* _sys_usw_ftm_get_hash_poly(lchip, sram_type, &poly_capabilty);*/
    MCHIP_API(lchip)->ftm_get_hash_poly_cap(lchip, sram_type, &poly_capabilty);

    for (loop = 0; loop < poly_capabilty.poly_num; loop++)
    {
        if (hash_poly->cur_poly_type == poly_capabilty.poly_type[loop])
        {
            break;
        }
    }
    if (loop >= poly_capabilty.poly_num)
    {
        return CTC_E_INVALID_PARAM;
    }

    _sys_usw_ftm_set_rsv_hash_key(lchip, 0);/*remove reserved hash key*/
    ret = _sys_usw_ftm_check_hash_mem_empty(lchip, sram_type);
    if(ret < 0)
    {
        goto End;
    }
     /*sys_usw_ftm_mapping_drv_hash_poly_type(sram_type, hash_poly->cur_poly_type, &poly_type);*/
    ret = MCHIP_API(lchip)->ftm_mapping_ctc_to_drv_hash_poly(lchip, sram_type, hash_poly->cur_poly_type, &poly_type);
    if(ret < 0)
    {
        goto End;
    }

    ret = drv_usw_ftm_set_hash_poly(lchip, hash_poly->mem_id, sram_type, poly_type);
    if(ret < 0)
    {
        goto End;
    }


End:
    _sys_usw_ftm_set_rsv_hash_key(lchip, 1);/*add reserved hash key*/
    return ret;
}

int32
sys_usw_ftm_set_profile_specs(uint8 lchip, uint32 spec_type, uint32 value)
{
    uint32 cmd = 0;
    uint32 spec_fdb = 0;
    uint32 global_capability_type = SYS_CAP_MAX;

    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(g_usw_sys_ftm[lchip]);
    SYS_FTM_INIT_CHECK(lchip);

    if (spec_type >= CTC_FTM_SPEC_MAX || spec_type == CTC_FTM_SPEC_INVALID)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (!DRV_IS_DUET2(lchip) && !((spec_type == CTC_FTM_SPEC_IPUC_HOST) || (spec_type == CTC_FTM_SPEC_IPMC) || (spec_type == CTC_FTM_SPEC_SCL_FLOW) || 
        (spec_type == CTC_FTM_SPEC_SCL) || (spec_type == CTC_FTM_SPEC_TUNNEL) || (spec_type == CTC_FTM_SPEC_L2MC) || (spec_type == CTC_FTM_SPEC_NAPT)))
    {
        return CTC_E_NONE;
    }
    
    if (spec_type == CTC_FTM_SPEC_MAC)
    {
        value = (value > 0x7FFFF) ? 0x7FFFF : value;
        spec_fdb = value;
        cmd = DRV_IOW(MacLimitSystem_t, MacLimitSystem_profileMaxCount_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &spec_fdb));
    }
    
    g_usw_sys_ftm[lchip]->profile_specs[spec_type] = value;

    switch (spec_type)
    {
        case CTC_FTM_SPEC_TUNNEL:
            global_capability_type = SYS_CAP_SPEC_TUNNEL_ENTRY_NUM;
            break;
        case CTC_FTM_SPEC_MPLS:
            global_capability_type = SYS_CAP_SPEC_MPLS_ENTRY_NUM;
            break;

        case CTC_FTM_SPEC_MAC:
            global_capability_type = SYS_CAP_SPEC_MAC_ENTRY_NUM;
            break;

        case CTC_FTM_SPEC_IPMC:
            global_capability_type = SYS_CAP_SPEC_IPMC_ENTRY_NUM;
            break;

        case CTC_FTM_SPEC_IPUC_HOST:
            global_capability_type = SYS_CAP_SPEC_HOST_ROUTE_ENTRY_NUM;
            break;

        default:
            break;
    }

    if (SYS_CAP_MAX != global_capability_type)
    {
        MCHIP_CAP(global_capability_type) = g_usw_sys_ftm[lchip]->profile_specs[spec_type];
    }


    return CTC_E_NONE;
}

int32
sys_usw_ftm_get_profile_specs(uint8 lchip, uint32 spec_type, uint32* value)
{
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_FTM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(value);

    if (spec_type >= CTC_FTM_SPEC_MAX || spec_type == CTC_FTM_SPEC_INVALID)
    {
        return CTC_E_INVALID_PARAM;
    }

    *value = g_usw_sys_ftm[lchip]->profile_specs[spec_type];

    return CTC_E_NONE;
}

/*only used before datapath moduli init for share buffer memory config*/
int32 sys_usw_ftm_misc_config(uint8 lchip)
{
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    return drv_usw_ftm_misc_config(lchip);
}


#define _____Show_____

STATIC int32
_sys_usw_ftm_show_specs(uint8 lchip)
{
    sys_ftm_specs_info_t specs_info;
    int32 ret = CTC_E_NONE;
    uint8 specs_name[] = 
        {
            CTC_FTM_SPEC_MAC,
            CTC_FTM_SPEC_IPUC_HOST,
            CTC_FTM_SPEC_IPUC_LPM,
            CTC_FTM_SPEC_ACL,
            CTC_FTM_SPEC_ACL_FLOW,
            CTC_FTM_SPEC_SCL,
            CTC_FTM_SPEC_SCL_FLOW,
            CTC_FTM_SPEC_TUNNEL,
            CTC_FTM_SPEC_IPFIX,
            CTC_FTM_SPEC_OAM,
            CTC_FTM_SPEC_MPLS,
            CTC_FTM_SPEC_L2MC,
            CTC_FTM_SPEC_IPMC,
            CTC_FTM_SPEC_NAPT,
        };
    char* arr_name[][2] =
       {
            {"MAC","80"},
            {"IP Host","80"},
            {"IP LPM", "80"},
            {"ACL","80"},
            {"Acl Flow","160"},
            {"Scl","80"},
            {"Scl Flow","80"},
            {"Tunnel Decp","80"},
            {"Ipfix","160"},
            {"Oam","160"},
            {"MPLS","80"},
            {"L2Mcast Group",""},
            {"IPMC","80"},
            {"NAPT","80"},
       };
    uint32 tabl_name[] =
        {
            DsFibHost0MacHashKey_t,
            DsFibHost0Ipv4HashKey_t,
            DsNeoLpmIpv4Bit32Snake_t,
            DsAclQosKey80Egr0_t,
            DsFlowL2HashKey_t,
            DsUserIdMacHashKey_t,
            DsUserIdSclFlowL2HashKey_t,
            DsUserIdTunnelIpv4HashKey_t,
            DsIpfixL2HashKey_t,
            DsBfdMep_t,
            DsMplsLabelHashKey_t,
            DsFibHost0MacHashKey_t,
            DsFibHost1Ipv4McastHashKey_t,
            DsFibHost1Ipv4NatSaPortHashKey_t,
        };
    uint32 entry_num = 0;
    uint8 i = 0;

    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n-------------------------------------------------------------\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"FEATURE SPEC  (H-Hash/T-Tcam)                  \n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-17s%-10s%-10s%-10s%-10s%-10s\n",
                     "FEATURE_NAME","RESOURCE", "SPEC", "USED", "RAM_TYPE","KEY_SIZE");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");

    for (i = 0; i < sizeof(specs_name)/sizeof(uint8); i++)
    {
        if (NULL != g_usw_sys_ftm[lchip]->ftm_callback[specs_name[i]])
        {
            /*
                for CTC_FTM_SPEC_IPFIX, it can learn by hardware, so
                just show the max-entry-number which it can reach
            */
            entry_num = _sys_usw_ftm_get_resource(lchip, specs_name[i], tabl_name[i], 1);
            specs_info.used_size = 0;
            ret = g_usw_sys_ftm[lchip]->ftm_callback[specs_name[i]](lchip, &specs_info);
            if (ret < 0)
            {
                ret = CTC_E_NONE;
                continue;
            }
            if (CTC_FTM_SPEC_ACL != specs_name[i])
            {
                SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-17s%-10d%-10d%-10d%-10s%-10s\n",
                    arr_name[i][0], entry_num,
                    g_usw_sys_ftm[lchip]->profile_specs[i],
                    specs_info.used_size, "H", arr_name[i][1]);
            }
            else
            {
                SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-17s%-10d%-10d%-10d%-10s%-10s\n",
                    arr_name[i][0], entry_num,
                    g_usw_sys_ftm[lchip]->profile_specs[i],
                    specs_info.used_size, "T", arr_name[i][1]);
            }
        }
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_tsingma_ftm_show_specs(uint8 lchip)
{
    sys_ftm_specs_info_t specs_info;
    int32 ret = CTC_E_NONE;
    char *str = " ";
    uint8 specs_name[] = 
        {
            CTC_FTM_SPEC_MAC,
            CTC_FTM_SPEC_IPUC_HOST,
            CTC_FTM_SPEC_IPUC_LPM,
            CTC_FTM_SPEC_ACL,
            CTC_FTM_SPEC_ACL_FLOW,
            CTC_FTM_SPEC_SCL,
            CTC_FTM_SPEC_SCL_FLOW,
            CTC_FTM_SPEC_TUNNEL,
            CTC_FTM_SPEC_IPFIX,
            CTC_FTM_SPEC_OAM,
            CTC_FTM_SPEC_MPLS,
            CTC_FTM_SPEC_L2MC,
            CTC_FTM_SPEC_IPMC,
            CTC_FTM_SPEC_NAPT,
        };
    char* arr_name[][2] =
       {
            {"MAC","80"},
            {"IP Host","80"},
            {"IP LPM", "56"},
            {"Igs ACL","80"},
            {"Acl Flow","160"},
            {"Scl","80"},
            {"Scl Flow","80"},
            {"Tunnel Decp","80"},
            {"Ipfix","160"},
            {"Oam",""},
            {"MPLS",""},
            {"L2MC",""},
            {"IPMC",""},
            {"NAPT",""},
       };
       uint32 tabl_name[] =
       {
            DsFibHost0MacHashKey_t,
            DsFibHost0Ipv4HashKey_t,
            DsNeoLpmIpv4Bit32Snake_t,
            DsAclQosKey80Ing0_t,
            DsFlowL2HashKey_t,
            DsUserIdMacHashKey_t,
            0,
            0,
            DsIpfixL2HashKey_t,
            DsBfdMep_t,
            DsMplsLabelHashKey_t,
            0,
            0,        
            0,
       };
    uint32 entry_num = 0;
    uint8 i = 0;
    extern uint32 sys_usw_aps_ftm_cb(uint8 lchip, uint32 *entry_num);

    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n-------------------------------------------------------------\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"FEATURE SPEC  (H-Hash/T-Tcam)                  \n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-17s%-10s%-10s%-10s%-10s%-10s\n",
                     "FEATURE_NAME","RESOURCE", "ENTRYS", "USED", "RAM_TYPE","KEY_SIZE");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");

    for (i = 0; i < sizeof(specs_name)/sizeof(uint8); i++)
    {
        if (NULL != g_usw_sys_ftm[lchip]->ftm_callback[specs_name[i]])
        {
            /*
                for CTC_FTM_SPEC_IPFIX, it can learn by hardware, so
                just show the max-entry-number which it can reach
            */
            entry_num = _sys_usw_ftm_get_resource(lchip, specs_name[i], tabl_name[i], 1);
            specs_info.used_size = 0;
            specs_info.type = 0;
            ret = g_usw_sys_ftm[lchip]->ftm_callback[specs_name[i]](lchip, &specs_info);
            if (ret < 0)
            {
                ret = CTC_E_NONE;
                continue;
            }
            if (CTC_FTM_SPEC_ACL != specs_name[i])
            {
                if(specs_name[i] == CTC_FTM_SPEC_SCL_FLOW || specs_name[i] == CTC_FTM_SPEC_TUNNEL)
                {
                    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-17s%-10s%-10d%-10d%-10s%-10s\n",
                        arr_name[i][0], "  -  ",g_usw_sys_ftm[lchip]->profile_specs[specs_name[i]],
                        specs_info.used_size, "H", arr_name[i][1]);
                }
                else if(specs_name[i] == CTC_FTM_SPEC_IPUC_HOST || specs_name[i] == CTC_FTM_SPEC_IPUC_LPM)
                {
                    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-17s%-10d%-10s%-10s%-10s%-10s\n",
                        arr_name[i][0], entry_num,"  -  ","-", "H", arr_name[i][1]);
                    if(specs_name[i] == CTC_FTM_SPEC_IPUC_LPM)
                    {
                        SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-17s%-7s%d%s%-7d%-10d%-10s%-10s\n",
                            "   -v4", "  -  ", entry_num*6/10, "-", entry_num*8/10, specs_info.used_size, " ", arr_name[i][1]);
                        str = "112";
                    }
                    else
                    {
                        SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-17s%-10s%-10d%-10d%-10s%-10s\n",
                            "   -v4", "  -  ",g_usw_sys_ftm[lchip]->profile_specs[specs_name[i]], specs_info.used_size, " ", arr_name[i][1]); 
                        str = "160";
                    }
                    specs_info.used_size = 0;
                    specs_info.type = 1;
                    g_usw_sys_ftm[lchip]->ftm_callback[specs_name[i]](lchip, &specs_info);
                    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-17s%-10s%-10s%-10d%-10s%-10s\n",
                        "   -v6", "  -  ", "  -  ", specs_info.used_size, " ", str);
                }
                else
                {
                    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-17s%-10d%-10d%-10d%-10s%-10s\n",
                    arr_name[i][0], entry_num,
                    g_usw_sys_ftm[lchip]->profile_specs[specs_name[i]],
                    specs_info.used_size, "H", arr_name[i][1]);
                }
            }
            else
            {
                SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-17s%-10d%-10d%-10d%-10s%-10s\n",
                    arr_name[i][0], entry_num, entry_num, specs_info.used_size, "T", arr_name[i][1]);
                entry_num = _sys_usw_ftm_get_resource(lchip, specs_name[i], DsAclQosKey80Egr0_t, 1);
                specs_info.used_size = 0;
                specs_info.type = 1;
                g_usw_sys_ftm[lchip]->ftm_callback[specs_name[i]](lchip, &specs_info);
                SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-17s%-10d%-10d%-10d%-10s%-10s\n",
                    "Egs ACL", entry_num, entry_num, specs_info.used_size, "T", arr_name[i][1]);
            }
        }
    }

    specs_info.used_size = 0;
    sys_usw_aps_ftm_cb(lchip, &specs_info.used_size);
    entry_num = _sys_usw_ftm_get_resource(lchip, 0, DsApsBridge_t, 1);
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-17s%-10d%-10d%-10d%-10s%-10s\n",
                    "APS", entry_num, entry_num, specs_info.used_size, "H", " ");

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_ftm_show_nh_info(uint8 lchip)
{
    uint32 specs_type = 0;
    uint32 entry_num = 0;
    char* nh_name[] =
        {
            "DsFwd",
            "DsMetEntry",
            "DsNexthop",
            "DsL2Edit",
            "DsL3Edit",
            "DsL3Edit3W_SPME",
            "DsL2EditEth_Outer",
        };
    uint32 edit_name[][2] =
        {
            {DsFwd_t,SYS_NH_ENTRY_TYPE_FWD},
            {DsMetEntry3W_t,SYS_NH_ENTRY_TYPE_MET},
            {DsNextHop4W_t,SYS_NH_ENTRY_TYPE_NEXTHOP_4W},
            {DsL2EditEth3W_t,SYS_NH_ENTRY_TYPE_L2EDIT_INNER_3W},
            {DsL2EditEth3W_t,SYS_NH_ENTRY_TYPE_L3EDIT_FLEX},
            {0,SYS_NH_ENTRY_TYPE_L3EDIT_SPME},
            {0,SYS_NH_ENTRY_TYPE_L2EDIT_OUTER_4W},
        };
    uint32 nh_used = 0;
    extern int32 sys_usw_nh_get_nh_resource(uint8 lchip, uint32 type, uint32* used_count);
    entry_num = 0;
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n-------------------------------------------------------------\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"NEXTHOP                     \n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-26s%-10s%-10s\n",
                     "EDIT_NAME","RESOURCE","USED");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");
    for (specs_type = 0; specs_type < 7; ++ specs_type)
    {
        if(specs_type < 5)
        {
            drv_usw_ftm_get_entry_num(lchip, edit_name[specs_type][0], &entry_num);
            if(edit_name[specs_type][0] == DsFwd_t)
            {
                entry_num = entry_num*2;
            }
        }
        else
        {
            entry_num = 1024;
        }
        sys_usw_nh_get_nh_resource(lchip, edit_name[specs_type][1],&nh_used);
        SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-26s%-10d%-10d\n",
            nh_name[specs_type], entry_num, nh_used);
    }


    return CTC_E_NONE;
}


STATIC int32
_sys_usw_ftm_show_global_info(uint8 lchip)
{
    uint32 cmd = 0;
    uint8 lpm_mode = 0;
    uint8 nat_enable = 0;

    LpmTcamCtl_m lpmtcam;
    cmd = DRV_IOR(LpmTcamCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lpmtcam));
    lpm_mode = GetLpmTcamCtl(V,privatePublicLookupMode_f, &lpmtcam);

    if (DRV_IS_DUET2(lchip))
    {
        nat_enable = GetLpmTcamCtl(V, natPbrTcamLookupEn_f, &lpmtcam);
    }
    else
    {
        nat_enable = GetLpmTcamCtl(V, natTcamLookupEn_f, &lpmtcam);
    }

	switch(lpm_mode)
    {
        case LPM_MODEL_PRIVATE_IPDA_IPSA_FULL:
            lpm_mode = CTC_FTM_ROUTE_MODE_DEFAULT;
            break;
        case LPM_MODEL_PRIVATE_IPDA:
            lpm_mode = CTC_FTM_ROUTE_MODE_WITHOUT_RPF_LINERATE;
            break;
        case LPM_MODEL_PRIVATE_IPDA_IPSA_HALF:
            lpm_mode = CTC_FTM_ROUTE_MODE_WITH_RPF_LINERATE;
            break;
        case LPM_MODEL_PUBLIC_IPDA_PRIVATE_IPDA_HALF:
            lpm_mode = CTC_FTM_ROUTE_MODE_WITH_PUB_LINERATE;
            break;
        case LPM_MODEL_PUBLIC_IPDA_IPSA_PRIVATE_IPDA_IPSA_HALF:
            lpm_mode = CTC_FTM_ROUTE_MODE_WITH_PUB_AND_RPF;
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n-------------------------------------------------------------\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  Global Config Information                                     \n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  Global DsNexthop                : %d\n", g_usw_sys_ftm[lchip]->glb_nexthop_size);
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  Mcast Group Number              : %d\n", g_usw_sys_ftm[lchip]->glb_met_size);
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  IP Route MODE                   : %d \n",lpm_mode);
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"  NAT/PBR ENABLE                  : %d \n",nat_enable);
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");
    return CTC_E_NONE;
}

int32
sys_usw_ftm_show_alloc_info(uint8 lchip)
{
    SYS_FTM_INIT_CHECK(lchip);
    if(DRV_IS_DUET2(lchip))
    {
        CTC_ERROR_RETURN(_sys_usw_ftm_show_specs(lchip));
    }
    else
    {
        CTC_ERROR_RETURN(_sys_tsingma_ftm_show_specs(lchip));
    }
    CTC_ERROR_RETURN(_sys_usw_ftm_show_nh_info(lchip));
    CTC_ERROR_RETURN(_sys_usw_ftm_show_global_info(lchip));

    return CTC_E_NONE;
}


int32
sys_usw_ftm_show_cam_info(uint8 lchip)
{
    uint8 cam_type = 0;
    drv_ftm_info_detail_t ftm_info;
    char *arr_cam_name[] =
    {
        "Invalid",
        "FibHost0",
        "FibHost1",
        "Scl",
        "Xc",
        "Flow"
    };

    SYS_FTM_INIT_CHECK(lchip);
    sal_memset(&ftm_info, 0, sizeof(drv_ftm_info_detail_t));

    /* Show SRAM table alloc info */
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n-------------------------------------------------------------\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Internal Cam alloc info                                     \n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-26s  %-10s  %-10s \n",
                     "CAM NAME", "TOTAL_SIZE", "USED_SIZE");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");

    ftm_info.info_type = DRV_FTM_INFO_TYPE_CAM;

    for (cam_type = DRV_FTM_CAM_TYPE_FIB_HOST0; cam_type < DRV_FTM_CAM_TYPE_MAX; ++ cam_type)
    {
        ftm_info.tbl_type = cam_type;
		ftm_info.used_size = 0;
        drv_usw_ftm_get_info_detail(lchip, &ftm_info);
        SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-26s  %-10d  %-10d\n",
                arr_cam_name[cam_type], ftm_info.max_size, ftm_info.used_size);
    }

    return CTC_E_NONE;
}


int32
sys_usw_ftm_show_edram_info(uint8 lchip)
{
    uint32 sram_type            = 0;
    drv_ftm_info_detail_t ftm_info;
    uint32 loop = 0;

    SYS_FTM_INIT_CHECK(lchip);

    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");

    /* Show SRAM table alloc info */
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n-------------------------------------------------------------\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Internal eDram table alloc info                                     \n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s%-10s%-10s%-10s\n",
                     "DYNAMIC_TABLE", "MEMORY", "ALLOCED", "OFFSET");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");

    for (sram_type = 0; sram_type < DRV_FTM_SRAM_TBL_MAX; sram_type++)
    {

        sal_memset(&ftm_info, 0, sizeof(drv_ftm_info_detail_t));
        ftm_info.info_type = DRV_FTM_INFO_TYPE_EDRAM;

        ftm_info.tbl_type = sram_type;
        drv_usw_ftm_get_info_detail(lchip, &ftm_info);
        for (loop = 0; loop < ftm_info.tbl_entry_num; ++loop)
        {
            SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s%-10d%-10d%-10d\n",
                                 ftm_info.str, ftm_info.mem_id[loop], ftm_info.entry_num[loop],
                                 ftm_info.offset[loop]);
            sal_strcpy(ftm_info.str, "");
        }

    }

    return CTC_E_NONE;
}

int32
sys_usw_ftm_show_tcam_info(uint8 lchip)
{
    uint8 tcam_key_type     = 0;
    drv_ftm_info_detail_t ftm_info;
    uint8 loop = 0;

    SYS_FTM_INIT_CHECK(lchip);
    sal_memset(&ftm_info, 0, sizeof(drv_ftm_info_detail_t));

    /* Show internal Tcam alloc info */
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"TCAM alloc info\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-15s%-35s%-12s%-12s\n",
                    "TCAM_TBL", "KEY_NAME", "KEY_SIZE", "MAX_INDEX");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");

    ftm_info.info_type = DRV_FTM_INFO_TYPE_TCAM;

    for (tcam_key_type = 0; tcam_key_type < DRV_FTM_TCAM_TYPE_STATIC_TCAM; tcam_key_type++)
    {

        ftm_info.tbl_type = tcam_key_type;

        if (CTC_E_NONE != drv_usw_ftm_get_info_detail(lchip, &ftm_info))
        {
          continue;
        }

        for (loop = 0; loop < ftm_info.tbl_entry_num; ++loop)
        {
            if (0 != ftm_info.key_size[loop])
            {
                SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-15s%-35s%-12d%-12d\n", ftm_info.str,TABLE_NAME(lchip, ftm_info.tbl_id[loop]),
                    ftm_info.key_size[loop], ftm_info.max_idx[loop]);

            }
            else
            {
                SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-15s%-35s%-12s%-12s\n", ftm_info.str,TABLE_NAME(lchip, ftm_info.tbl_id[loop]),
                    "", "");
            }
            sal_strcpy(ftm_info.str, "");
        }
    }
    return CTC_E_NONE;
}
int32
sys_usw_ftm_show_hash_poly_info(uint8 lchip)
{
    uint32 sram_type = 0;
    drv_ftm_info_detail_t ftm_info;
    ctc_ftm_hash_poly_t hash_poly;
    uint32 loop = 0;
    uint32 loop1 = 0;

    SYS_FTM_INIT_CHECK(lchip);

    sal_memset(&ftm_info, 0, sizeof(drv_ftm_info_detail_t));
    sal_memset(&hash_poly, 0, sizeof(ctc_ftm_hash_poly_t));

    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");

    /* Show hash poly alloc info */
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"\n-------------------------------------------------------------\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Hash poly alloc info\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"%-20s%-10s%-15s%-10s\n",
                     "DYNAMIC_TABLE", "MEMORY", "CURRENT_TYPE", "HASH_CAPABILITY");
    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"-------------------------------------------------------------\n");

    ftm_info.info_type = DRV_FTM_INFO_TYPE_EDRAM;

    for (sram_type = 0; sram_type < DRV_FTM_SRAM_TBL_MAX; sram_type++)            /*loop for function*/
    {

        ftm_info.tbl_type = sram_type;
        drv_usw_ftm_get_info_detail(lchip, &ftm_info);
        for (loop = 0; loop < ftm_info.tbl_entry_num; ++loop)        /*loop for mem_id*/
        {
            sal_memset(&hash_poly, 0, sizeof(ctc_ftm_hash_poly_t));
            hash_poly.mem_id = ftm_info.mem_id[loop];

            if (hash_poly.mem_id  > DRV_FTM_SRAM_MAX)
            {
                continue;
            }

            sys_usw_ftm_get_hash_poly(lchip, &hash_poly);
            SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-20s%-10d%-15d", ftm_info.str, ftm_info.mem_id[loop], hash_poly.cur_poly_type);
            for (loop1 = 0; loop1 < hash_poly.poly_num; loop1++)             /*loop for showing hash ability*/
            {
                SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%d ", hash_poly.poly_type[loop1]);
            }
            SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");
            sal_strcpy(ftm_info.str, "");
        }

    }

    return CTC_E_NONE;
}

int32
sys_usw_ftm_show_detail(uint8 lchip)
{
    SYS_FTM_INIT_CHECK(lchip);

    sys_usw_ftm_show_edram_info(lchip);
    sys_usw_ftm_show_hash_poly_info(lchip);
    sys_usw_ftm_show_tcam_info(lchip);
    sys_usw_ftm_show_cam_info(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_ftm_show_alloc_info_detail(uint8 lchip)
{
    SYS_FTM_INIT_CHECK(lchip);
    CTC_ERROR_RETURN(_sys_usw_ftm_show_specs(lchip));
    CTC_ERROR_RETURN(sys_usw_ftm_show_detail(lchip));
    CTC_ERROR_RETURN(_sys_usw_ftm_show_global_info(lchip));

    return CTC_E_NONE;
}

#define ____WarmBoot___
STATIC int32
_sys_usw_ftm_traverse_opf_func(uint8 lchip, sys_usw_opf_used_node_t* p_node, void* user_data)
{
    sys_traverse_t *traversal_data = (sys_traverse_t *)user_data;
    ctc_wb_data_t *wb_data = (ctc_wb_data_t *)(traversal_data->data);
    sys_wb_ftm_ad_t  *p_wb_ftm_ad;
    uint16 max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data->key_len + wb_data->data_len);

    p_wb_ftm_ad = (sys_wb_ftm_ad_t*)wb_data->buffer + wb_data->valid_cnt;
    sal_memset(p_wb_ftm_ad, 0, sizeof(sys_wb_ftm_ad_t));

    p_wb_ftm_ad->opf_pool_type = traversal_data->value1;
    p_wb_ftm_ad->opf_pool_index = traversal_data->value2;
    p_wb_ftm_ad->sub_id = p_node->node_idx;

    p_wb_ftm_ad->start = p_node->start;
    p_wb_ftm_ad->end = p_node->end;
    p_wb_ftm_ad->reverse = p_node->reverse;

    if (++wb_data->valid_cnt == max_entry_cnt)
    {
        CTC_ERROR_RETURN(ctc_wb_add_entry(wb_data));
        wb_data->valid_cnt = 0;
    }

    return CTC_E_NONE;
}

int32 sys_usw_ftm_wb_sync(uint8 lchip)
{
    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_traverse_t user_data;
    sys_usw_opf_t    opf;
    uint8       loop;

    if(g_usw_sys_ftm[lchip]->status == 3)
    {
        return CTC_E_NONE;
    }
    /*syncup fdb_matser*/
    wb_data.buffer = mem_malloc(MEM_FTM_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_DATA_T((&wb_data),sys_wb_ftm_ad_t, CTC_FEATURE_FTM, SYS_WB_APPID_FTM_SUBID_AD);

    sal_memset(&opf, 0, sizeof(opf));
    opf.pool_type = g_usw_sys_ftm[lchip]->ftm_opf_type;
    user_data.data = &wb_data;
    user_data.value1 = opf.pool_type;
    for (loop = SYS_FTM_OPF_TYPE_FIB_AD; loop < SYS_FTM_OPF_TYPE_MAX; loop++)
    {
        opf.pool_index = loop;
        user_data.value2 = loop;

        CTC_ERROR_GOTO(sys_usw_opf_traverse(lchip, &opf, _sys_usw_ftm_traverse_opf_func, &user_data, 0), ret, done);
        if (wb_data.valid_cnt > 0)
        {
            CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
            wb_data.valid_cnt = 0;
        }
    }
done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    return ret;
}

int32 sys_usw_ftm_wb_restore(uint8 lchip)
{
    uint16 entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t    wb_query;
    sys_wb_ftm_ad_t   wb_ftm_ad;
    sys_wb_ftm_ad_t*  p_wb_ftm_ad = &wb_ftm_ad;
    sys_usw_opf_t     opf;

    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_FTM_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);

    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_ftm_ad_t, CTC_FEATURE_FTM, SYS_WB_APPID_FTM_SUBID_AD);
    /* set default value*/
    sal_memset(p_wb_ftm_ad, 0, sizeof(sys_wb_ftm_ad_t));
    sal_memset(&opf, 0, sizeof(opf));

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
        sal_memcpy((uint8*)p_wb_ftm_ad, (uint8*)(wb_query.buffer) + entry_cnt * (wb_query.key_len + wb_query.data_len), wb_query.key_len + wb_query.data_len);
        entry_cnt++;
        opf.pool_type = p_wb_ftm_ad->opf_pool_type;
        opf.pool_index = p_wb_ftm_ad->opf_pool_index;
        opf.reverse = p_wb_ftm_ad->reverse;

        CTC_ERROR_GOTO(sys_usw_opf_alloc_offset_from_position(lchip, &opf, p_wb_ftm_ad->end-p_wb_ftm_ad->start+1, p_wb_ftm_ad->start), ret, done);
    CTC_WB_QUERY_ENTRY_END((&wb_query));

done:
    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    return ret;
}
#define __change_profile__
int32
sys_usw_ftm_mem_change(uint8 lchip, ctc_ftm_change_profile_t* p_profile)
{
    int32 ret = 0;
    drv_ftm_profile_info_t old_drv_pfl;
    drv_ftm_profile_info_t new_drv_pfl;
    uint8  old_tbl_id_map[DRV_FTM_SRAM_TBL_MAX] = {0xFF};
    uint8  new_tbl_id_map[DRV_FTM_SRAM_TBL_MAX] = {0xFF};
    uint8  old_key_id_map[DRV_FTM_TCAM_TYPE_MAX] = {0xFF};
    uint8  new_key_id_map[DRV_FTM_TCAM_TYPE_MAX] = {0xFF};
    uint16 tmp_tbl_id;
    uint8  sys_mode = CTC_FTM_MEM_CHANGE_NONE;

    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if(!DRV_IS_TSINGMA(lchip))
    {
        return CTC_E_NOT_SUPPORT;
    }
    CTC_PTR_VALID_CHECK(p_profile);
    CTC_PTR_VALID_CHECK(p_profile->old_profile);
    CTC_PTR_VALID_CHECK(p_profile->new_profile);
    SYS_FTM_INIT_CHECK(lchip);

    /*mapping ctc profile to drv profile and create drv_tbl_id to tbl_idx mapping*/
    sal_memset(&old_drv_pfl, 0, sizeof(drv_ftm_profile_info_t));
    sal_memset(&new_drv_pfl, 0, sizeof(drv_ftm_profile_info_t));
    if (p_profile->old_profile->key_info_size && p_profile->old_profile->key_info)
    {
        old_drv_pfl.key_info = mem_malloc(MEM_FTM_MODULE, sizeof(drv_ftm_key_info_t)*p_profile->old_profile->key_info_size);
        if(NULL == old_drv_pfl.key_info)
        {
            ret =  DRV_E_NO_MEMORY;
            goto end0;
        }
    }
    if(p_profile->old_profile->tbl_info_size && p_profile->old_profile->tbl_info)
    {
        old_drv_pfl.tbl_info = mem_malloc(MEM_FTM_MODULE, sizeof(ctc_ftm_tbl_info_t)*p_profile->old_profile->tbl_info_size);
        if(NULL == old_drv_pfl.tbl_info)
        {
            ret = DRV_E_NO_MEMORY;
            goto end0;
        }
    }
    CTC_ERROR_GOTO(_sys_usw_ftm_map_profile(lchip, p_profile->old_profile, &old_drv_pfl, (uint8*)old_tbl_id_map, (uint8*)old_key_id_map), ret, end0);

    if (p_profile->new_profile->key_info_size && p_profile->new_profile->key_info)
    {
        new_drv_pfl.key_info = mem_malloc(MEM_FTM_MODULE, sizeof(drv_ftm_key_info_t)*p_profile->new_profile->key_info_size);
        if(NULL == new_drv_pfl.key_info)
        {
            ret =  DRV_E_NO_MEMORY;
            goto end0;
        }
    }

    if(p_profile->new_profile->tbl_info_size && p_profile->new_profile->tbl_info)
    {
        new_drv_pfl.tbl_info = mem_malloc(MEM_FTM_MODULE, sizeof(ctc_ftm_tbl_info_t)*p_profile->new_profile->tbl_info_size);
        if(NULL == new_drv_pfl.tbl_info)
        {
            ret = DRV_E_NO_MEMORY;
            goto end0;
        }
    }
    CTC_ERROR_GOTO(_sys_usw_ftm_map_profile(lchip, p_profile->new_profile, &new_drv_pfl, (uint8*)new_tbl_id_map, (uint8*)new_key_id_map), ret, end0);

    for(tmp_tbl_id=0; tmp_tbl_id < DRV_FTM_SRAM_TBL_MAX; ++tmp_tbl_id)
    {
        if((old_tbl_id_map[tmp_tbl_id] == 0xFF && new_tbl_id_map[tmp_tbl_id] != 0xFF) || \
            (new_tbl_id_map[tmp_tbl_id] == 0xFF && old_tbl_id_map[tmp_tbl_id] != 0xFF))
        {
            if(tmp_tbl_id == DRV_FTM_SRAM_TBL_MAC_HASH_KEY)
            {
                sys_mode = CTC_FTM_MEM_CHANGE_MAC;
            }
            else
            {
                sys_mode = p_profile->recover_en ? CTC_FTM_MEM_CHANGE_RECOVER : CTC_FTM_MEM_CHANGE_REBOOT;
                break;
            }
        }

        if(old_drv_pfl.tbl_info[old_tbl_id_map[tmp_tbl_id]].mem_bitmap == new_drv_pfl.tbl_info[new_tbl_id_map[tmp_tbl_id]].mem_bitmap)
        {
            continue;
        }


        if((tmp_tbl_id == DRV_FTM_SRAM_TBL_MAC_HASH_KEY) && \
            (old_drv_pfl.tbl_info[old_tbl_id_map[tmp_tbl_id]].mem_bitmap^new_drv_pfl.tbl_info[new_tbl_id_map[tmp_tbl_id]].mem_bitmap))
        {
            sys_mode = CTC_FTM_MEM_CHANGE_MAC;
        }
        else
        {
            sys_mode = p_profile->recover_en ? CTC_FTM_MEM_CHANGE_RECOVER : CTC_FTM_MEM_CHANGE_REBOOT;
            break;
        }
    }

    for(tmp_tbl_id=0; tmp_tbl_id < DRV_FTM_TCAM_TYPE_MAX; ++tmp_tbl_id)
    {
        if((old_key_id_map[tmp_tbl_id] == 0xFF && new_key_id_map[tmp_tbl_id] != 0xFF) || \
            (new_key_id_map[tmp_tbl_id] == 0xFF && old_key_id_map[tmp_tbl_id] != 0xFF))
        {
            sys_mode = p_profile->recover_en ? CTC_FTM_MEM_CHANGE_RECOVER : CTC_FTM_MEM_CHANGE_REBOOT;
            break;
        }

        if(old_drv_pfl.key_info[old_key_id_map[tmp_tbl_id]].tcam_bitmap != new_drv_pfl.key_info[new_key_id_map[tmp_tbl_id]].tcam_bitmap)
        {
            sys_mode = p_profile->recover_en ? CTC_FTM_MEM_CHANGE_RECOVER : CTC_FTM_MEM_CHANGE_REBOOT;
            break;
        }
    }
    g_usw_sys_ftm[lchip]->buffer_mode = 0;
    if(!((old_drv_pfl.tbl_info[old_tbl_id_map[DRV_FTM_SRAM_TBL_MAC_HASH_KEY]].mem_bitmap >> 23)|\
        (old_drv_pfl.tbl_info[old_tbl_id_map[DRV_FTM_SRAM_TBL_DSMAC_AD]].mem_bitmap >> 27)|\
        (old_drv_pfl.tbl_info[old_tbl_id_map[DRV_FTM_SRAM_TBL_DSIP_AD]].mem_bitmap >> 27)|\
        (old_drv_pfl.tbl_info[old_tbl_id_map[DRV_FTM_SRAM_TBL_NEXTHOP]].mem_bitmap >> 29)))
    {
        g_usw_sys_ftm[lchip]->buffer_mode |= 2;
    }

    if(!((new_drv_pfl.tbl_info[new_tbl_id_map[DRV_FTM_SRAM_TBL_MAC_HASH_KEY]].mem_bitmap >> 23)|\
        (new_drv_pfl.tbl_info[new_tbl_id_map[DRV_FTM_SRAM_TBL_DSMAC_AD]].mem_bitmap >> 27)|\
        (new_drv_pfl.tbl_info[new_tbl_id_map[DRV_FTM_SRAM_TBL_DSIP_AD]].mem_bitmap >> 27)|\
        (new_drv_pfl.tbl_info[new_tbl_id_map[DRV_FTM_SRAM_TBL_NEXTHOP]].mem_bitmap >> 29)))
    {
        g_usw_sys_ftm[lchip]->buffer_mode |= 1;
    }

    /*if change buffer and buffer is used in ad/nexthop table, can not only adjust mac table*/
    if((sys_mode == 1) && \
        (g_usw_sys_ftm[lchip]->buffer_mode == 1 || g_usw_sys_ftm[lchip]->buffer_mode == 2) && \
            ((old_drv_pfl.tbl_info[old_tbl_id_map[DRV_FTM_SRAM_TBL_DSMAC_AD]].mem_bitmap >> 27)|\
            (old_drv_pfl.tbl_info[old_tbl_id_map[DRV_FTM_SRAM_TBL_DSIP_AD]].mem_bitmap >> 27)|\
            (old_drv_pfl.tbl_info[old_tbl_id_map[DRV_FTM_SRAM_TBL_NEXTHOP]].mem_bitmap >> 29)))
    {
        sys_mode = p_profile->recover_en ? CTC_FTM_MEM_CHANGE_RECOVER : CTC_FTM_MEM_CHANGE_REBOOT;
    }
    g_usw_sys_ftm[lchip]->status = sys_mode;

    SYS_FTM_DBG_OUT(CTC_DEBUG_LEVEL_INFO, " change mode %u, buffer mode %u\n", g_usw_sys_ftm[lchip]->status, g_usw_sys_ftm[lchip]->buffer_mode);

    if(g_usw_sys_ftm[lchip]->fdb_cb && (g_usw_sys_ftm[lchip]->status == CTC_FTM_MEM_CHANGE_MAC || g_usw_sys_ftm[lchip]->status == CTC_FTM_MEM_CHANGE_RECOVER))
    {
        CTC_ERROR_GOTO(g_usw_sys_ftm[lchip]->fdb_cb(lchip, NULL), ret, end0);
    }
    /*change mac table*/
    if(g_usw_sys_ftm[lchip]->status == CTC_FTM_MEM_CHANGE_MAC)
    {
        uint8 loop = 0;

        if(g_usw_sys_ftm[lchip]->buffer_mode == 1 || g_usw_sys_ftm[lchip]->buffer_mode == 2)
        {
            CTC_ERROR_GOTO(sys_usw_ftm_reset_bufferstore_register(lchip, g_usw_sys_ftm[lchip]->buffer_mode), ret, end0);
        }
        CTC_ERROR_GOTO(drv_usw_ftm_adjust_mac_table(lchip, &new_drv_pfl), ret, end0);
        for(loop=CTC_FTM_SPEC_MAC; loop < CTC_FTM_SPEC_MAX; loop++)
        {
            CTC_ERROR_GOTO(sys_usw_ftm_set_profile_specs(lchip, loop, p_profile->new_profile->misc_info.profile_specs[loop]), ret, end0);
        }
        g_usw_sys_ftm[lchip]->buffer_mode = 0;
        g_usw_sys_ftm[lchip]->status = CTC_FTM_MEM_CHANGE_NONE;
    }
    p_profile->change_mode = sys_mode;
    if(old_drv_pfl.key_info)
    {
        mem_free(old_drv_pfl.key_info);
    }
    if(old_drv_pfl.tbl_info)
    {
        mem_free(old_drv_pfl.tbl_info);
    }
    if(new_drv_pfl.key_info)
    {
        mem_free(new_drv_pfl.key_info);
    }
    if(new_drv_pfl.tbl_info)
    {
        mem_free(new_drv_pfl.tbl_info);
    }
    return CTC_E_NONE;
end0:
    if(old_drv_pfl.key_info)
    {
        mem_free(old_drv_pfl.key_info);
    }
    if(old_drv_pfl.tbl_info)
    {
        mem_free(old_drv_pfl.tbl_info);
    }
    if(new_drv_pfl.key_info)
    {
        mem_free(new_drv_pfl.key_info);
    }
    if(new_drv_pfl.tbl_info)
    {
        mem_free(new_drv_pfl.tbl_info);
    }
    g_usw_sys_ftm[lchip]->buffer_mode = 0;
    g_usw_sys_ftm[lchip]->status = CTC_FTM_MEM_CHANGE_NONE;
    return ret;
}
int32 sys_usw_ftm_set_working_status(uint8 lchip, uint8 status)
{
    SYS_FTM_INIT_CHECK(lchip);

    g_usw_sys_ftm[lchip]->status = status;

    return CTC_E_NONE;
}
int32 sys_usw_ftm_get_working_status(uint8 lchip, uint8* p_status)
{
    SYS_FTM_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_status);

    *p_status = g_usw_sys_ftm[lchip]->status;

    return CTC_E_NONE;
}
int32 sys_usw_ftm_register_change_mem_cb(uint8 lchip, sys_ftm_change_mem_cb_t cb)
{
    SYS_FTM_INIT_CHECK(lchip);
    g_usw_sys_ftm[lchip]->fdb_cb = cb;
    return CTC_E_NONE;
}
