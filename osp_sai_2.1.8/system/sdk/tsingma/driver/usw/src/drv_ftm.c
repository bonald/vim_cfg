/*
 @file drv_ftm.c

 @date 2011-11-16

 @version v2.0

 This file provides all sys alloc function
*/

#include "sal.h"
#include "usw/include/drv_enum.h"
#include "usw/include/drv_ftm.h"
#include "usw/include/drv_common.h"
#include "drv_api.h"



#define DRV_FTM_REALLOC_TCAM 1


#define DYNAMIC_SLICE0_ADDR_BASE_OFFSET_TO_DUP 0x60000000
#define DYNAMIC_SLICE1_ADDR_BASE_OFFSET_TO_DUP 0x40000000

int32 drv_set_tcam_lookup_bitmap(uint32 mem_id, uint32 bitmap);

#define DRV_FTM_DBG_OUT(fmt, args...)       sal_printf(fmt, ##args)
#define DRV_FTM_DBG_DUMP(fmt, args...)

#define   DRV_FTM_TBL_INFO(tbl_id)   g_usw_ftm_master[lchip]->p_sram_tbl_info[tbl_id]
#define   DRV_FTM_KEY_INFO(tbl_id)   g_usw_ftm_master[lchip]->p_tcam_key_info[tbl_id]


#define DRV_FTM_TBL_ACCESS_FLAG(tbl_id)               DRV_FTM_TBL_INFO(tbl_id).access_flag
#define DRV_FTM_TBL_MAX_ENTRY_NUM(tbl_id)             DRV_FTM_TBL_INFO(tbl_id).max_entry_num
#define DRV_FTM_TBL_MEM_BITMAP(tbl_id)                DRV_FTM_TBL_INFO(tbl_id).mem_bitmap
#define DRV_FTM_TBL_COUPLE_MODE(tbl_id)                DRV_FTM_TBL_INFO(tbl_id).couple_mode
#define DRV_FTM_TBL_MEM_START_OFFSET(tbl_id, mem_id)  DRV_FTM_TBL_INFO(tbl_id).mem_start_offset[mem_id]
#define DRV_FTM_TBL_MEM_ENTRY_NUM(tbl_id, mem_id)     DRV_FTM_TBL_INFO(tbl_id).mem_entry_num[mem_id]

#define DRV_FTM_TBL_BASE_BITMAP(tbl_id)      DRV_FTM_TBL_INFO(tbl_id).dyn_ds_base.bitmap
#define DRV_FTM_TBL_BASE_VAL(tbl_id, i)   DRV_FTM_TBL_INFO(tbl_id).dyn_ds_base.ds_dynamic_base[i]
#define DRV_FTM_TBL_BASE_MIN(tbl_id, i)   DRV_FTM_TBL_INFO(tbl_id).dyn_ds_base.ds_dynamic_min_index[i]
#define DRV_FTM_TBL_BASE_MAX(tbl_id, i)   DRV_FTM_TBL_INFO(tbl_id).dyn_ds_base.ds_dynamic_max_index[i]


#define DRV_MEM_ENTRY_NUM(lchip, mem_id)   (DRV_MEM_INFO(lchip, mem_id).entry_num)
#define DRV_MEM_ADD3W(lchip, mem_id)   (DRV_MEM_INFO(lchip, mem_id).addr_3w)
#define DRV_MEM_ADD6W(lchip, mem_id)   (DRV_MEM_INFO(lchip, mem_id).addr_6w)
#define DRV_MEM_ADD12W(lchip, mem_id)   (DRV_MEM_INFO(lchip, mem_id).addr_12w)



#define DRV_FTM_MEM_MAX_ENTRY_NUM(mem_id)     DRV_MEM_ENTRY_NUM(lchip, mem_id)
#define DRV_FTM_MEM_HW_DATA_BASE(mem_id)      DRV_MEM_ADD3W(lchip, mem_id)
#define DRV_FTM_MEM_HW_DATA_BASE6W(mem_id)    DRV_MEM_ADD6W(lchip, mem_id)
#define DRV_FTM_MEM_HW_DATA_BASE12W(mem_id)   DRV_MEM_ADD12W(lchip, mem_id)
#define DRV_FTM_MEM_HW_MASK_BASE(mem_id)      DRV_MEM_ADD6W(lchip, mem_id)

#define DRV_FTM_KEY_MAX_INDEX(tbl_id)                  DRV_FTM_KEY_INFO(tbl_id).max_key_index
#define DRV_FTM_KEY_MEM_BITMAP(tbl_id)                 DRV_FTM_KEY_INFO(tbl_id).mem_bitmap
#define DRV_FTM_KEY_ENTRY_NUM(tbl_id, type, mem_id)                 DRV_FTM_KEY_INFO(tbl_id).entry_num[type][mem_id]
#define DRV_FTM_KEY_START_OFFSET(tbl_id, type, mem_id)                 DRV_FTM_KEY_INFO(tbl_id).start_offset[type][mem_id]



#define DRV_FTM_ADD_TABLE(mem_id, table, start, size) \
    do { \
        if (size>0) \
        { \
            DRV_FTM_TBL_MEM_BITMAP(table)               |= (1 << mem_id); \
            DRV_FTM_TBL_MAX_ENTRY_NUM(table)            += size; \
            DRV_FTM_TBL_MEM_START_OFFSET(table, mem_id) = start; \
            DRV_FTM_TBL_MEM_ENTRY_NUM(table, mem_id)    = size; \
        } \
    } while (0)

#define DRV_FTM_REMOVE_TABLE(mem_id, table, start, size) \
    do { \
        if (size>0) \
        { \
            DRV_FTM_TBL_MEM_BITMAP(table)               &= ~(1 << mem_id); \
            DRV_FTM_TBL_MAX_ENTRY_NUM(table)            -= size; \
            DRV_FTM_TBL_MEM_START_OFFSET(table, mem_id) = 0; \
            DRV_FTM_TBL_MEM_ENTRY_NUM(table, mem_id)    = 0; \
        } \
    } while (0)
#define DRV_FTM_ADD_KEY(mem_id, table, start, size) \
    do { \
        if (size>0) \
        { \
            DRV_FTM_KEY_MEM_BITMAP(table)               |=  (1 << (mem_id-DRV_FTM_TCAM_KEY0)); \
            DRV_FTM_KEY_MAX_INDEX(table)                += size; \
        } \
    } while (0)
#define DRV_FTM_REMOVE_KEY(mem_id, table, start, size) \
    do { \
        if (size) \
        { \
            DRV_FTM_KEY_MEM_BITMAP(table)               &=  ~(1 << (mem_id-DRV_FTM_TCAM_KEY0)); \
            DRV_FTM_KEY_MAX_INDEX(table)                -= size; \
        } \
    } while (0)

#define DRV_FTM_ADD_LPM_KEY(mem_id, table, start, size) \
    do { \
        if (size>0) \
        { \
            DRV_FTM_KEY_MEM_BITMAP(table)               |=  (1 << (mem_id-DRV_FTM_LPM_TCAM_KEY0)); \
            DRV_FTM_KEY_MAX_INDEX(table)                += size; \
        } \
    } while (0)

#define DRV_FTM_ADD_KEY_BYTYPE(mem_id, table, type, start, size) \
    do { \
        if (size>0) \
        { \
           DRV_FTM_KEY_START_OFFSET(table, type, mem_id - DRV_FTM_TCAM_KEY0)       = start; \
           DRV_FTM_KEY_ENTRY_NUM(table, type, mem_id - DRV_FTM_TCAM_KEY0)          = size; \
        } \
    } while (0)

#define DRV_FTM_REMOVE_KEY_BYTYPE(mem_id, table, type) \
    do { \
       DRV_FTM_KEY_START_OFFSET(table, type, mem_id - DRV_FTM_TCAM_KEY0)       = 0; \
       DRV_FTM_KEY_ENTRY_NUM(table, type, mem_id - DRV_FTM_TCAM_KEY0)          = 0; \
    } while (0)

#define DRV_FTM_ADD_LPM_KEY_BYTYPE(mem_id, table, type, start, size) \
    do { \
        if (size>0) \
        { \
           DRV_FTM_KEY_START_OFFSET(table, type, mem_id - DRV_FTM_LPM_TCAM_KEY0)       = start; \
           DRV_FTM_KEY_ENTRY_NUM(table, type, mem_id - DRV_FTM_LPM_TCAM_KEY0)          = size; \
        } \
    } while (0)


#define DRV_FTM_ADD_FULL_TABLE(mem_id, table, couple) \
        do {\
            if (couple>0) \
            {\
                DRV_FTM_ADD_TABLE(mem_id, table, 0, (DRV_FTM_MEM_MAX_ENTRY_NUM(mem_id)*2));\
                DRV_FTM_TBL_COUPLE_MODE(table) = 1;\
            } else {\
                DRV_FTM_ADD_TABLE(mem_id, table, 0, DRV_FTM_MEM_MAX_ENTRY_NUM(mem_id));    \
            }\
        } while (0)

#define DRV_FTM_ADD_FULL_KEY(mem_id, table) \
        do { \
            uint32 entry_num = DRV_FTM_MEM_MAX_ENTRY_NUM(mem_id);\
            DRV_FTM_ADD_KEY(mem_id, table, 0, entry_num);\
            DRV_FTM_ADD_KEY_BYTYPE(mem_id, table, DRV_FTM_TCAM_SIZE_80,  0,                 1 * (entry_num/4)); \
            DRV_FTM_ADD_KEY_BYTYPE(mem_id, table, DRV_FTM_TCAM_SIZE_160, 1 * (entry_num/4), 1 * (entry_num/4)); \
            DRV_FTM_ADD_KEY_BYTYPE(mem_id, table, DRV_FTM_TCAM_SIZE_320, 2 * (entry_num/4), 1 * (entry_num/4)); \
            DRV_FTM_ADD_KEY_BYTYPE(mem_id, table, DRV_FTM_TCAM_SIZE_640, 3 * (entry_num/4), 1 * (entry_num/4)); \
       } while (0)

/* single/double 2 chose 1 */
#define DRV_FTM_ADD_LPM_FULL_KEY(mem_id, table)                                                             \
    do {                                                                                                \
        uint32 entry_num = DRV_FTM_MEM_MAX_ENTRY_NUM(mem_id);                                           \
        DRV_FTM_ADD_LPM_KEY(mem_id, table, 0, entry_num);                                               \
        DRV_FTM_ADD_LPM_KEY_BYTYPE(mem_id, table, 0,   0,              entry_num); \
     } while (0)

enum drv_ftm_module_s
{
    TCAM_MODULE_NONE = 0,
    TCAM_MODULE_ACL = 1,
    TCAM_MODULE_SCL = 2,
    TCAM_MODULE_LPM = 3,
};




enum drv_ftm_dyn_cam_type_e
{
    DRV_FTM_DYN_CAM_TYPE_LPM_KEY0,
    DRV_FTM_DYN_CAM_TYPE_LPM_KEY1,
    DRV_FTM_DYN_CAM_TYPE_LPM_KEY2,
    DRV_FTM_DYN_CAM_TYPE_LPM_KEY3,
    DRV_FTM_DYN_CAM_TYPE_MAC_KEY,
    DRV_FTM_DYN_CAM_TYPE_FIB_MACHOST0_KEY,
    DRV_FTM_DYN_CAM_TYPE_FIB_MACHOST1_KEY,
    DRV_FTM_DYN_CAM_TYPE_FIB_FLOW_KEY,
    DRV_FTM_DYN_CAM_TYPE_FIB_FLOW_AD,
    DRV_FTM_DYN_CAM_TYPE_USERID_HASH_KEY,
    DRV_FTM_DYN_CAM_TYPE_USERID_HASH_AD,
    DRV_FTM_DYN_CAM_TYPE_USERID_HASH_AD1,
    DRV_FTM_DYN_CAM_TYPE_EGRESS_OAM_HASH_KEY,

    DRV_FTM_DYN_CAM_TYPE_IP_FIX_HASH_KEY,
    DRV_FTM_DYN_CAM_TYPE_IP_FIX_HASH_AD,
    DRV_FTM_DYN_CAM_TYPE_FIB_L2_AD,
    DRV_FTM_DYN_CAM_TYPE_FIB_L3_AD,
    DRV_FTM_DYN_CAM_TYPE_DS_LM,
    DRV_FTM_DYN_CAM_TYPE_DS_APS,
    DRV_FTM_DYN_CAM_TYPE_DS_MEP,
    DRV_FTM_DYN_CAM_TYPE_DS_MA,
    DRV_FTM_DYN_CAM_TYPE_DS_MANAME,
    DRV_FTM_DYN_CAM_TYPE_DS_EDIT,
    DRV_FTM_DYN_CAM_TYPE_DS_NEXTHOP,
    DRV_FTM_DYN_CAM_TYPE_DS_MET,
    DRV_FTM_DYN_CAM_TYPE_DS_FWD,
    DRV_FTM_DYN_CAM_TYPE_MAX
};
typedef enum drv_ftm_dyn_cam_type_e drv_ftm_dyn_cam_type_t;

#define DRV_FTM_UNIT_1K 10
#define DRV_FTM_UNIT_8K 13
#define DRV_FTM_FLD_INVALID 0xFFFFFFFF

#define DRV_FTM_MAX_MEM_NUM 8

struct drv_ftm_dyn_ds_base_s
{
    uint32 bitmap;
    uint32 ds_dynamic_base[16];
    uint32 ds_dynamic_max_index[16];
    uint32 ds_dynamic_min_index[16];
};
typedef struct drv_ftm_dyn_ds_base_s drv_ftm_dyn_ds_base_t;

struct drv_ftm_sram_tbl_info_s
{
    uint32 access_flag; /*judge weather tbl share alloc*/
    uint32 mem_bitmap;
    uint32 max_entry_num;
    uint32 mem_start_offset[DRV_FTM_EDRAM_MAX];
    uint32 mem_entry_num[DRV_FTM_EDRAM_MAX];
    drv_ftm_dyn_ds_base_t dyn_ds_base;
    uint8  couple_mode;
    uint8  rsv[3];
};
typedef struct drv_ftm_sram_tbl_info_s drv_ftm_sram_tbl_info_t;

#define BYTE_TO_4W  12



#define DRV_FTM_TCAM_SIZE_ipv4_pbr 0
#define DRV_FTM_TCAM_SIZE_ipv6_pbr 1
#define DRV_FTM_TCAM_SIZE_ipv4_nat 2
#define DRV_FTM_TCAM_SIZE_ipv6_nat 3

#define DRV_FTM_TCAM_SIZE_halfkey   0
#define DRV_FTM_TCAM_SIZE_singlekey 1
#define DRV_FTM_TCAM_SIZE_doublekey 2

#define DRV_FTM_TCAM_SIZE_LPMhalf   0
#define DRV_FTM_TCAM_SIZE_LPMSingle 1
#define DRV_FTM_TCAM_SIZE_LPMDouble 2

#define DRV_FTM_TCAM_SIZE_MAX 4



#define DRV_FTM_CAM_CHECK_TBL_VALID(cam_type, tbl_id)\
    if ((DRV_FTM_CAM_TYPE_FIB_HOST1_NAT == cam_type)\
        && (DsFibHost1Ipv4NatDaPortHashKey_t != tbl_id))\
    {\
        return DRV_E_HASH_CONFLICT;\
    }

struct drv_ftm_tcam_key_info_s
{
    uint32 mem_bitmap;
    uint32  max_key_index;
    uint32 entry_num[DRV_FTM_TCAM_SIZE_MAX][DRV_FTM_MAX_ID];
    uint32 start_offset[DRV_FTM_TCAM_SIZE_MAX][DRV_FTM_MAX_ID];
};
typedef struct drv_ftm_tcam_key_info_s drv_ftm_tcam_key_info_t;
typedef struct g_ftm_master_s g_ftm_master_t;


struct g_ftm_master_s
{
    drv_ftm_sram_tbl_info_t* p_sram_tbl_info;
    drv_ftm_tcam_key_info_t* p_tcam_key_info;

    uint16  int_tcam_used_entry;
    /*cam*/
    uint8 cfg_max_cam_num[DRV_FTM_CAM_TYPE_MAX];
    uint8 conflict_cam_num[DRV_FTM_CAM_TYPE_MAX];

    /*cam bitmap*/
    uint32 fib1_cam_bitmap0[1];     /*single*/
    uint32 fib1_cam_bitmap1[1];     /*dual*/
    uint32 fib1_cam_bitmap2[1];     /*quad*/

    uint32 userid_cam_bitmap0[1];   /*single*/
    uint32 userid_cam_bitmap1[1];   /*dual*/
    uint32 userid_cam_bitmap2[1];   /*quad*/

    uint32 xcoam_cam_bitmap0[4];    /*single*/
    uint32 xcoam_cam_bitmap1[4];    /*dual*/
    uint32 xcoam_cam_bitmap2[4];    /*quad*/

    uint32 cam_bitmap;
    uint32 host1_poly_type;

    uint8   couple_mode;
    uint8   lpm_model;
    uint8   nat_pbr_enable;
    uint8   napt_enable;
    uint8   mpls_mode;
    uint8   scl_mode;

    uint16 tcam_specs[DRV_FTM_TCAM_TYPE_MAX][4];
    uint16 tcam_offset[DRV_FTM_TCAM_TYPE_MAX][4];
    uint32 lpm_tcam_init_bmp[2][3];   /*0 private, 1 public; 0 ipv6 and ipv6 half boundary, 1 ipv6 half and ipv4 boundary*/
};

g_ftm_master_t* g_usw_ftm_master[30] = {NULL};

#define DRV_BIT_SET(flag, bit)      ((flag) = (flag) | (1 << (bit)))


static uint32 parser_tbl_id_list[] =
{
   ParserUdfCam_t                           ,
   ParserUdfCamResult_t                     ,
   ParserDebugStats_t                       ,
   ParserEthernetCtl_t                      ,
   ParserIpChecksumCtl_t                    ,
   ParserIpCtl_t                            ,
   ParserL3Ctl_t                            ,
   ParserLayer2ProtocolCam_t                ,
   ParserLayer2ProtocolCamValid_t           ,
   ParserLayer3FlexCtl_t                    ,
   ParserLayer3HashCtl_t                    ,
   ParserLayer3ProtocolCam_t                ,
   ParserLayer3ProtocolCamValid_t           ,
   ParserLayer3ProtocolCtl_t                ,
   ParserLayer4AchCtl_t                     ,
   ParserLayer4AppCtl_t                     ,
   ParserLayer4FlexCtl_t                    ,
   ParserMplsCtl_t                          ,
   ParserPacketTypeMap_t                    ,
   ParserPbbCtl_t                           ,
   ParserTrillCtl_t                         ,
   ParserReserved_t                         ,
   ParserRangeOpCtl_t
};

static uint32 acl_ad_80bits_tbl[] =
{
   DsEgrAcl0IpfixEnableTcamAd_t                    ,
   DsEgrAcl1IpfixEnableTcamAd_t                    ,
   DsEgrAcl2IpfixEnableTcamAd_t                    ,
   DsIngAcl0IpfixEnableTcamAd_t                    ,
   DsIngAcl1IpfixEnableTcamAd_t                    ,
   DsIngAcl2IpfixEnableTcamAd_t                    ,
   DsIngAcl3IpfixEnableTcamAd_t                    ,
   DsIngAcl4IpfixEnableTcamAd_t                    ,
   DsIngAcl5IpfixEnableTcamAd_t                    ,
   DsIngAcl6IpfixEnableTcamAd_t                    ,
   DsIngAcl7IpfixEnableTcamAd_t
};
extern /* sram type refer to drv_ftm_sram_tbl_type_t*/
int32
drv_usw_get_dynamic_ram_couple_mode(uint8 lchip, uint16 sram_type, uint32* couple_mode);


int32 drv_usw_ftm_get_mplshash_mode(uint8 lchip, uint8* is_mpls)
{
    if (g_usw_ftm_master[lchip] == NULL)
    {
        return DRV_E_INVALID_PTR;
    }
    *is_mpls = g_usw_ftm_master[lchip]->mpls_mode;

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_ftm_set_profile_mpls_gem_port(uint8 lchip)
{
    uint16 sram_type = 0;
    uint8 multi = 1;


    if (g_usw_ftm_master[lchip]->mpls_mode)
    {

        sram_type = DRV_FTM_SRAM_TBL_MPLS_HASH_KEY;

        if (g_usw_ftm_master[lchip]->couple_mode || (g_usw_ftm_master[lchip]->mpls_mode == 2))
        {
            multi = 2;
            DRV_FTM_TBL_COUPLE_MODE(sram_type) = 1;
            DRV_FTM_TBL_COUPLE_MODE(DRV_FTM_SRAM_TBL_MPLS_HASH_AD) = 1;
        }

        DRV_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(sram_type) , DRV_FTM_MIXED0- DRV_FTM_MIXED0);  /*Moidfy to 4K*/
        DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, DRV_FTM_MIXED0)    = DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_MIXED0)*multi;
        DRV_FTM_TBL_MAX_ENTRY_NUM(sram_type)   += DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, DRV_FTM_MIXED0);

        DRV_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(sram_type) , DRV_FTM_MIXED1- DRV_FTM_MIXED0); /*Moidfy to 4K*/
        DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, DRV_FTM_MIXED1)    = DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_MIXED1)*multi;
        DRV_FTM_TBL_MAX_ENTRY_NUM(sram_type)   += DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, DRV_FTM_MIXED1);

        sram_type = DRV_FTM_SRAM_TBL_MPLS_HASH_AD; /*Moidfy to 8K*/
        DRV_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(sram_type) , DRV_FTM_MIXED2- DRV_FTM_MIXED0);
        DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, DRV_FTM_MIXED2)    = DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_MIXED2)*multi;
        DRV_FTM_TBL_MAX_ENTRY_NUM(sram_type)   += DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, DRV_FTM_MIXED2);
    }
    else
    {
        sram_type = DRV_FTM_SRAM_TBL_GEM_PORT;
        DRV_FTM_TBL_COUPLE_MODE(sram_type) = 1;

        DRV_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(sram_type) , DRV_FTM_MIXED2- DRV_FTM_MIXED0);  /*Moidfy to 16K*2*4, couple*/
        DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, DRV_FTM_MIXED2)    = (DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_MIXED2))*2*4;
        DRV_FTM_TBL_MAX_ENTRY_NUM(sram_type)   += DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, DRV_FTM_MIXED2);

        DRV_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(sram_type) , DRV_FTM_MIXED0- DRV_FTM_MIXED0); /*Moidfy to 4K*2/8*6 = 1K*6 */
        DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, DRV_FTM_MIXED0)    = ((DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_MIXED0)*2)/8)*6;
        DRV_FTM_TBL_MAX_ENTRY_NUM(sram_type)   += DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, DRV_FTM_MIXED0);

        DRV_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(sram_type) , DRV_FTM_MIXED1- DRV_FTM_MIXED0); /*Moidfy to 4K*2/8*6 = 1K*6 */
        DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, DRV_FTM_MIXED1)    = ((DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_MIXED1)*2)/8)*6;
        DRV_FTM_TBL_MAX_ENTRY_NUM(sram_type)   += DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, DRV_FTM_MIXED1);
    }

    return 0;
}


STATIC int32
_drv_usw_ftm_set_profile_default(uint8 lchip)
{

    uint8  table         = 0;
    /********************************************
    dynamic edram table
    *********************************************/

    /*FIB Host0 Hash Key*/
    /* table 表示RAM的集合,比如FibHost0Hash 可以选择5块RAM */
    table = DRV_FTM_SRAM_TBL_FIB0_HASH_KEY;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM0, table, 0);   /*Moidfy to 16K*/  /* 将RAM1 整块大小都加到table中 */
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM3, table, 0);   /*Moidfy to 8K*/

    /*FIB Host1 Hash Key*/
    table = DRV_FTM_SRAM_TBL_FIB1_HASH_KEY;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM9, table, 1);   /*Moidfy to 4K*/

    /*FLOW Hash Key*/
    table = DRV_FTM_SRAM_TBL_FLOW_HASH_KEY;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM4, table, 1);   /*Moidfy to 8K*/

    /*FLOW Hash AD*/
    table = DRV_FTM_SRAM_TBL_FLOW_AD;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM14, table, 1);   /*Moidfy to 8K*/

    /*Lpm lookup Key*/
    table = DRV_FTM_SRAM_TBL_LPM_LKP_KEY0;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM6, table, 0);   /*Moidfy to 8K*/

    /*DsIpDa*/
    table = DRV_FTM_SRAM_TBL_DSIP_AD;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM13, table, 1);   /*Moidfy to 8K*/

    /*DsIpMac*/
    table = DRV_FTM_SRAM_TBL_DSMAC_AD;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM11, table, 0);  /*Moidfy to 16K*/

    /*UserIdTunnelMpls Hash Key*/
    table = DRV_FTM_SRAM_TBL_USERID_HASH_KEY;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM7, table, 0);  /*Moidfy to 8K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM8, table, 0);  /*Moidfy to 4K*/

    /*UserIdTunnelMpls AD*/
    table = DRV_FTM_SRAM_TBL_USERID_AD;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM15, table, 0);  /*Moidfy to 8K*/
    /*Oam MaName*/
    table = DRV_FTM_SRAM_TBL_OAM_MA_NAME;
    DRV_FTM_ADD_TABLE(DRV_FTM_SRAM1, table, 0, 4 * 1024);  /*Moidfy to 4K*/
    /*Oam Mep*/
    table = DRV_FTM_SRAM_TBL_OAM_MEP;
    DRV_FTM_ADD_TABLE(DRV_FTM_SRAM1, table, 4 * 1024, 8 * 1024);          /*Moidfy to 8K*/ /* 将SRAM14的前4K ~ 12K放到TABLE OAM_MEP 中 */

    /*Oam Ma*/
    table = DRV_FTM_SRAM_TBL_OAM_MA;
    DRV_FTM_ADD_TABLE(DRV_FTM_SRAM1, table, 12 * 1024, 2 * 1024);   /*Moidfy to 2K*//* 将SRAM14的10K ~ 12K放到TABLE OAM_MA 中 */

    /*Oam Lm*/
    table = DRV_FTM_SRAM_TBL_OAM_LM;
    DRV_FTM_ADD_TABLE(DRV_FTM_SRAM1, table, 14 * 1024, 2 * 1024);  /*Moidfy to 2K*/

    /*Oam APS*/
    table = DRV_FTM_SRAM_TBL_OAM_APS;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM10, table, 0);  /*Moidfy to 4K*/

    /*DsEdit*/
    table = DRV_FTM_SRAM_TBL_EDIT;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM20, table, 0);  /*Moidfy to 8K*/


    /*DsNexthop*/
    table = DRV_FTM_SRAM_TBL_NEXTHOP;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM16, table, 1);  /*Moidfy to 8K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM19, table, 1);  /*Moidfy to 8K*/
    /*DsMet*/
    table = DRV_FTM_SRAM_TBL_MET;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM18, table, 1);  /*Moidfy to 8K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM21, table, 1);  /*Moidfy to 4K*/


    /*XcOamHash */
    table = DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM2, table, 0);   /*Moidfy to 32K*/

    /*IpFix */
    table = DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY;
     /*DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM3, table);*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM5, table, 1);   /*Moidfy to 8K*/

    /*IpFixAd */
    table = DRV_FTM_SRAM_TBL_IPFIX_AD;
     /*DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM7, table);*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM12, table, 1);   /*Moidfy to 8K*/

    /*DsFwd */
    table = DRV_FTM_SRAM_TBL_FWD;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM17, table, 0);   /*Moidfy to 8K*/

    /********************************************
    tcam table
    *********************************************/

    table = DRV_FTM_TCAM_TYPE_IGS_ACL0;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY2, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL1;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY3, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL2;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY4, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL3;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY5, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL4;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY6, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL5;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY7, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL6;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY8, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL7;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY9, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_EGS_ACL0;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY10, table);/* OK */

    table = DRV_FTM_TCAM_TYPE_EGS_ACL1;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY11, table);/* OK */

    table = DRV_FTM_TCAM_TYPE_EGS_ACL2;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY12, table);/* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_USERID0;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY0, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_USERID1;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY1, table); /* OK */

    /*default tcam mode is private ipda+ipsa, full size, no performance*/
    g_usw_ftm_master[lchip]->lpm_model = LPM_MODEL_PRIVATE_IPDA_IPSA_FULL;
    g_usw_ftm_master[lchip]->nat_pbr_enable = 1;
	g_usw_ftm_master[lchip]->napt_enable = 1;
    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][0] = (1<<(DRV_FTM_LPM_TCAM_KEY0-DRV_FTM_LPM_TCAM_KEY0)) | (1<<(DRV_FTM_LPM_TCAM_KEY1-DRV_FTM_LPM_TCAM_KEY0))\
      | (1<<(DRV_FTM_LPM_TCAM_KEY2-DRV_FTM_LPM_TCAM_KEY0)) | (1<<(DRV_FTM_LPM_TCAM_KEY3-DRV_FTM_LPM_TCAM_KEY0));
	g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][2] = (1<<(DRV_FTM_LPM_TCAM_KEY4-DRV_FTM_LPM_TCAM_KEY0)) | (1<<(DRV_FTM_LPM_TCAM_KEY5-DRV_FTM_LPM_TCAM_KEY0))\
      | (1<<(DRV_FTM_LPM_TCAM_KEY6-DRV_FTM_LPM_TCAM_KEY0)) | (1<<(DRV_FTM_LPM_TCAM_KEY7-DRV_FTM_LPM_TCAM_KEY0));

    table = DRV_FTM_TCAM_TYPE_IGS_LPM0DA;
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY0, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY1, table);

    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY2, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY3, table);

    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY4, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY5, table);

    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY6, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY7, table);

    table = DRV_FTM_TCAM_TYPE_IGS_LPM1;
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY8, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY9, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY10, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY11, table);

    return DRV_E_NONE;
}

STATIC int32
_drv_tsingma_ftm_set_profile_default(uint8 lchip)
{

    uint8  table         = 0;
    /********************************************
    dynamic edram table
    *********************************************/

    /*FIB Host0 Hash Key*/
    /* table 表示RAM的集合,比如FibHost0Hash 可以选择5块RAM */
    table = DRV_FTM_SRAM_TBL_FIB0_HASH_KEY;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM1, table, 0);   /*Moidfy to 16K*/


    table = DRV_FTM_SRAM_TBL_MAC_HASH_KEY;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM23, table, 0);   /*Moidfy to 32K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM24, table, 0);   /*Moidfy to 32K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM25, table, 0);   /*Moidfy to 32K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM26, table, 0);   /*Moidfy to 32K*/




    /*FIB Host1 Hash Key*/
    table = DRV_FTM_SRAM_TBL_FIB1_HASH_KEY;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM10, table, 0);   /*Moidfy to 8K*/


    /*FLOW Hash Key*/
    table = DRV_FTM_SRAM_TBL_FLOW_HASH_KEY;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM0, table, 1);   /*Moidfy to 32K (couple)*/


    /*FLOW Hash AD*/
    table = DRV_FTM_SRAM_TBL_FLOW_AD;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM11, table, 0);   /*Moidfy to 16K*/


    /*Lpm lookup Key*/
    table = DRV_FTM_SRAM_TBL_LPM_LKP_KEY0;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM6, table, 0);   /*Moidfy to 8K*/

    /*Lpm lookup Key*/
    table = DRV_FTM_SRAM_TBL_LPM_LKP_KEY1;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM7, table, 0);   /*Moidfy to 8K*/

    /*DsIpDa*/
    table = DRV_FTM_SRAM_TBL_DSIP_AD;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM14, table, 0);   /*Moidfy to 16K*/

    /*DsIpMac*/
    table = DRV_FTM_SRAM_TBL_DSMAC_AD;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM12, table, 0);  /*Moidfy to 8K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM13, table, 0);  /*Moidfy to 8K*/


    /*UserIdTunnel Hash Key*/
    table = DRV_FTM_SRAM_TBL_USERID_HASH_KEY;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM8, table, 0);  /*Moidfy to 8K*/

    /*UserIdTunnel Hash Key*/
    table = DRV_FTM_SRAM_TBL_USERID1_HASH_KEY;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM9, table, 0);  /*Moidfy to 8K*/

	g_usw_ftm_master[lchip]->scl_mode = 1;


    /*UserIdTunnel AD*/
    table = DRV_FTM_SRAM_TBL_USERID_AD;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM15, table, 0);  /*Moidfy to 8K*/


    /*UserIdTunnel AD*/
    table = DRV_FTM_SRAM_TBL_USERID_AD1;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM16, table, 0);  /*Moidfy to 8K*/


    /*Oam MaName*/
    table = DRV_FTM_SRAM_TBL_OAM_MA_NAME;
    DRV_FTM_ADD_TABLE(DRV_FTM_SRAM2, table, 0, 4 * 1024);  /*Moidfy to 4K*/
    /*Oam Mep*/
    table = DRV_FTM_SRAM_TBL_OAM_MEP;
    DRV_FTM_ADD_TABLE(DRV_FTM_SRAM2, table, 4 * 1024, 8 * 1024);          /*Moidfy to 8K*/ /* 将SRAM14的前8K放到TABLE OAM_MEP 中 */
    /*Oam Ma*/
    table = DRV_FTM_SRAM_TBL_OAM_MA;
    DRV_FTM_ADD_TABLE(DRV_FTM_SRAM2, table, 12 * 1024, 2 * 1024);   /*Moidfy to 2K*//* put SRAM14 8K ~ 10K into TABLE OAM_MA */
    /*Oam Lm*/
    table = DRV_FTM_SRAM_TBL_OAM_LM;
    DRV_FTM_ADD_TABLE(DRV_FTM_SRAM2, table, 14 * 1024, 2 * 1024);  /*Moidfy to 2K*/


    /*XcOamHash */
    table = DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM3, table, 0);   /*Moidfy to 8K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM4, table, 0);   /*Moidfy to 8K*/


    /*DsNexthop*/
    table = DRV_FTM_SRAM_TBL_NEXTHOP;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM17, table, 0);  /*Moidfy to 32K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM29, table, 0);  /*Moidfy to 32K*/

    /*DsMet*/
    table = DRV_FTM_SRAM_TBL_MET;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM18, table, 0);  /*Moidfy to 8K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM19, table, 0);  /*Moidfy to 8K*/

    /*DsEdit*/
    table = DRV_FTM_SRAM_TBL_EDIT;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM21, table, 0);  /*Moidfy to 4K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM22, table, 0);  /*Moidfy to 4K*/

    /*DsFwd */
    table = DRV_FTM_SRAM_TBL_FWD;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM20, table, 0);   /*Moidfy to 4K*/


    /********************************************
    tcam table
    *********************************************/


    table = DRV_FTM_TCAM_TYPE_IGS_USERID0;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY0, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_USERID1;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY1, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_USERID2;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY2, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_USERID3;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY3, table); /* OK */


    table = DRV_FTM_TCAM_TYPE_IGS_ACL0;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY4, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL1;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY5, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL2;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY6, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL3;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY7, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL4;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY8, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL5;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY9, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL6;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY10, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL7;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY11, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_EGS_ACL0;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY12, table);/* OK */

    table = DRV_FTM_TCAM_TYPE_EGS_ACL1;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY13, table);/* OK */

    table = DRV_FTM_TCAM_TYPE_EGS_ACL2;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY14, table);/* OK */

    /*default tcam mode is private ipda+ipsa, full size, no performance*/
    g_usw_ftm_master[lchip]->lpm_model = LPM_MODEL_PRIVATE_IPDA_IPSA_FULL;
    g_usw_ftm_master[lchip]->nat_pbr_enable = 1;
	g_usw_ftm_master[lchip]->napt_enable = 1;
    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][0] = (1<<(DRV_FTM_LPM_TCAM_KEY0-DRV_FTM_LPM_TCAM_KEY0)) | (1<<(DRV_FTM_LPM_TCAM_KEY1-DRV_FTM_LPM_TCAM_KEY0))\
      | (1<<(DRV_FTM_LPM_TCAM_KEY2-DRV_FTM_LPM_TCAM_KEY0)) | (1<<(DRV_FTM_LPM_TCAM_KEY3-DRV_FTM_LPM_TCAM_KEY0));
	g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][2] = (1<<(DRV_FTM_LPM_TCAM_KEY4-DRV_FTM_LPM_TCAM_KEY0)) | (1<<(DRV_FTM_LPM_TCAM_KEY5-DRV_FTM_LPM_TCAM_KEY0))\
      | (1<<(DRV_FTM_LPM_TCAM_KEY6-DRV_FTM_LPM_TCAM_KEY0)) | (1<<(DRV_FTM_LPM_TCAM_KEY7-DRV_FTM_LPM_TCAM_KEY0));

    table = DRV_FTM_TCAM_TYPE_IGS_LPM0DA;
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY0, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY1, table);

    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY2, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY3, table);

    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY4, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY5, table);

    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY6, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY7, table);

    table = DRV_FTM_TCAM_TYPE_IGS_LPM1;
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY8, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY9, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY10, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY11, table);

    return DRV_E_NONE;
}


#if 0
STATIC int32
_drv_tsingma_ftm_set_profile_1(uint8 lchip)
{

    uint8  table         = 0;
    /********************************************
    dynamic edram table
    *********************************************/

    /*FIB Host0 Hash Key*/


    table = DRV_FTM_SRAM_TBL_MAC_HASH_KEY;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM0, table, 0);
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM1, table, 0);
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM2, table, 0);
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM3, table, 0);
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM4, table, 0);
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM5, table, 0);

    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM23, table, 0);   /*Moidfy to 32K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM24, table, 0);   /*Moidfy to 32K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM25, table, 0);   /*Moidfy to 32K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM26, table, 0);   /*Moidfy to 32K*/



    /*FIB Host1 Hash Key*/
    table = DRV_FTM_SRAM_TBL_FIB1_HASH_KEY;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM10, table, 0);   /*Moidfy to 8K*/

    /*FLOW Hash AD*/
    table = DRV_FTM_SRAM_TBL_FLOW_AD;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM11, table, 0);   /*Moidfy to 16K*/


    /*Lpm lookup Key*/
    table = DRV_FTM_SRAM_TBL_LPM_LKP_KEY0;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM6, table, 0);   /*Moidfy to 8K*/

    /*Lpm lookup Key*/
    table = DRV_FTM_SRAM_TBL_LPM_LKP_KEY1;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM7, table, 0);   /*Moidfy to 8K*/

    /*DsIpDa*/
    table = DRV_FTM_SRAM_TBL_DSIP_AD;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM14, table, 0);   /*Moidfy to 16K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM27, table, 0);   /*Moidfy to 32K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM28, table, 0);   /*Moidfy to 64K*/

    /*DsIpMac*/
    table = DRV_FTM_SRAM_TBL_DSMAC_AD;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM12, table, 0);  /*Moidfy to 8K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM13, table, 0);  /*Moidfy to 8K*/


    /*UserIdTunnel Hash Key*/
    table = DRV_FTM_SRAM_TBL_USERID_HASH_KEY;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM8, table, 0);  /*Moidfy to 8K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM9, table, 0);  /*Moidfy to 8K*/


    /*UserIdTunnel AD*/
    table = DRV_FTM_SRAM_TBL_USERID_AD;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM15, table, 0);  /*Moidfy to 8K*/


    /*UserIdTunnel AD*/
    table = DRV_FTM_SRAM_TBL_USERID_AD1;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM16, table, 0);  /*Moidfy to 8K*/


    /*DsNexthop*/
    table = DRV_FTM_SRAM_TBL_NEXTHOP;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM17, table, 0);  /*Moidfy to 32K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM18, table, 0);  /*Moidfy to 8K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM29, table, 0);  /*Moidfy to 32K*/

    /*DsMet*/
    table = DRV_FTM_SRAM_TBL_MET;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM19, table, 0);  /*Moidfy to 8K*/

    /*DsEdit*/
    table = DRV_FTM_SRAM_TBL_EDIT;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM21, table, 0);  /*Moidfy to 4K*/
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM22, table, 0);  /*Moidfy to 4K*/

    /*DsFwd */
    table = DRV_FTM_SRAM_TBL_FWD;
    DRV_FTM_ADD_FULL_TABLE(DRV_FTM_SRAM20, table, 0);   /*Moidfy to 4K*/

    /********************************************
    tcam table
    *********************************************/


    table = DRV_FTM_TCAM_TYPE_IGS_USERID0;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY0, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_USERID1;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY1, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_USERID2;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY2, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_USERID3;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY3, table); /* OK */


    table = DRV_FTM_TCAM_TYPE_IGS_ACL0;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY4, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL1;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY5, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL2;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY6, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL3;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY7, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL4;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY8, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL5;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY9, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL6;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY10, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_IGS_ACL7;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY11, table); /* OK */

    table = DRV_FTM_TCAM_TYPE_EGS_ACL0;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY12, table);/* OK */

    table = DRV_FTM_TCAM_TYPE_EGS_ACL1;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY13, table);/* OK */

    table = DRV_FTM_TCAM_TYPE_EGS_ACL2;
    DRV_FTM_ADD_FULL_KEY(DRV_FTM_TCAM_KEY14, table);/* OK */

    /*default tcam mode is private ipda+ipsa, full size, no performance*/
    g_usw_ftm_master[lchip]->lpm_model = LPM_MODEL_PRIVATE_IPDA_IPSA_FULL;
    g_usw_ftm_master[lchip]->nat_pbr_enable = 1;
	g_usw_ftm_master[lchip]->napt_enable = 1;
    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][0] = (1<<(DRV_FTM_LPM_TCAM_KEY0-DRV_FTM_LPM_TCAM_KEY0)) | (1<<(DRV_FTM_LPM_TCAM_KEY1-DRV_FTM_LPM_TCAM_KEY0))\
      | (1<<(DRV_FTM_LPM_TCAM_KEY2-DRV_FTM_LPM_TCAM_KEY0)) | (1<<(DRV_FTM_LPM_TCAM_KEY3-DRV_FTM_LPM_TCAM_KEY0));
	g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][2] = (1<<(DRV_FTM_LPM_TCAM_KEY4-DRV_FTM_LPM_TCAM_KEY0)) | (1<<(DRV_FTM_LPM_TCAM_KEY5-DRV_FTM_LPM_TCAM_KEY0))\
      | (1<<(DRV_FTM_LPM_TCAM_KEY6-DRV_FTM_LPM_TCAM_KEY0)) | (1<<(DRV_FTM_LPM_TCAM_KEY7-DRV_FTM_LPM_TCAM_KEY0));

    table = DRV_FTM_TCAM_TYPE_IGS_LPM0DA;
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY0, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY1, table);

    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY2, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY3, table);

    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY4, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY5, table);

    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY6, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY7, table);

    table = DRV_FTM_TCAM_TYPE_IGS_LPM1;
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY8, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY9, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY10, table);
    DRV_FTM_ADD_LPM_FULL_KEY(DRV_FTM_LPM_TCAM_KEY11, table);

    return DRV_E_NONE;
}
#endif




STATIC int32
_drv_usw_ftm_process_oam_tbl(uint8 lchip, drv_ftm_tbl_info_t *p_oam_tbl_info)
{

    uint32 bitmap = 0;
    uint8 bit_offset = 0;
    uint32 total_size = 0;

    uint32 tbl_ids[] = {DRV_FTM_SRAM_TBL_OAM_MA_NAME, DRV_FTM_SRAM_TBL_OAM_MEP, DRV_FTM_SRAM_TBL_OAM_MA, DRV_FTM_SRAM_TBL_OAM_LM};
    uint32 tbl_size[] = {0, 0, 0, 0};
    uint8  idx  = 0;

    uint32 mem_entry_num[DRV_FTM_MEM_MAX]    = {0};
    uint32 mem_start_offset[DRV_FTM_MEM_MAX] = {0};

    bitmap = p_oam_tbl_info->mem_bitmap;
    for (bit_offset = 0 ; bit_offset < 32; bit_offset++)
    {
        if(IS_BIT_SET(bitmap, bit_offset))
        {
            #if 0
            if(couple_mode)
            {
                total_size += (p_oam_tbl_info->mem_entry_num[bit_offset]*2);
            }
            else
            {
                total_size += p_oam_tbl_info->mem_entry_num[bit_offset];
            }
            #endif
            if(p_oam_tbl_info->mem_entry_num[bit_offset] > DRV_FTM_MEM_MAX_ENTRY_NUM(bit_offset))
            {
                DRV_FTM_TBL_COUPLE_MODE(DRV_FTM_SRAM_TBL_OAM_MA_NAME) = 1;
                DRV_FTM_TBL_COUPLE_MODE(DRV_FTM_SRAM_TBL_OAM_MEP) = 1;
                DRV_FTM_TBL_COUPLE_MODE(DRV_FTM_SRAM_TBL_OAM_MA) = 1;
                DRV_FTM_TBL_COUPLE_MODE(DRV_FTM_SRAM_TBL_OAM_LM) = 1;
            }
            mem_entry_num[bit_offset] = p_oam_tbl_info->mem_entry_num[bit_offset];
            mem_start_offset[bit_offset] = p_oam_tbl_info->mem_start_offset[bit_offset];
            total_size += p_oam_tbl_info->mem_entry_num[bit_offset];
        }
    }

    /*SYSTEM MODIFIED by yangl for oam profile bug 53741, 2019-9-29*/
    tbl_size[0] = total_size/8*1;   /*MANAME*/
    tbl_size[1] = total_size/8*4;   /*MEP*/
    tbl_size[2] = total_size/8*1;   /*MA*/
    tbl_size[3] = total_size/8*2;   /*LM*/
    /* yangl end */
    bit_offset = 0;
    while (bit_offset < DRV_FTM_MEM_MAX)
    {
        if(IS_BIT_SET(bitmap, bit_offset))
        {
            if (tbl_size[idx] == mem_entry_num[bit_offset])
            {
                DRV_FTM_ADD_TABLE(bit_offset, tbl_ids[idx], mem_start_offset[bit_offset], mem_entry_num[bit_offset]);
                idx++;
                bit_offset ++;
            }
            else if (tbl_size[idx] > mem_entry_num[bit_offset])
            {
                DRV_FTM_ADD_TABLE(bit_offset, tbl_ids[idx], mem_start_offset[bit_offset], mem_entry_num[bit_offset]);
                tbl_size[idx] -= mem_entry_num[bit_offset];
                bit_offset ++;
            }
            else
            {
                DRV_FTM_ADD_TABLE(bit_offset, tbl_ids[idx], mem_start_offset[bit_offset], tbl_size[idx]);
                mem_start_offset[bit_offset] += tbl_size[idx];
                idx++;
            }

            if(idx >= 4)
            {
                break;
            }
        }
        else
        {
            bit_offset ++;
        }
    }

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_ftm_get_sram_tbl_name(uint32 sram_type, char* sram_tbl_id_str)
{
    char* str[DRV_FTM_SRAM_TBL_MAX]={
        "MacHashKey",
        "FibHost0HashKey",
        "FibHost1HashKey",
        "FlowHashKey",
        "IpfixHashKey",
        "SclHashKey",
        "Scl1HashKey",
        "XoamHashKey",
        "LpmLookupKey0",
        "LpmLookupKey1",
        "LpmLookupKey2",
        "LpmLookupKey3",
        "DsMacAd",
        "DsIpAd",
        "DsFlowAd",
        "DsIpfixAd",
        "DsSclAd",
        "DsScl1Ad",
        "DsNexthop",
        "DsFwd",
        "DsMet",
        "DsEdit",
        "DsAps",
        "DsOamLm",
        "DsOamMep",
        "DsOamMa",
        "DsOamMaName",
        "MplsHashKey",
        "MplsHashAd",
        "XgponGemPort",
    };

    DRV_PTR_VALID_CHECK(sram_tbl_id_str);
    if (sram_type >= DRV_FTM_SRAM_TBL_MAX)
    {
        return DRV_E_EXCEED_MAX_SIZE;
    }

    sal_strcpy(sram_tbl_id_str, str[sram_type]);

    return DRV_E_NONE;
}


STATIC int32
_drv_usw_ftm_set_profile_user_define_edram(uint8 lchip, drv_ftm_profile_info_t *profile_info)
{
    uint8 tbl_index     = 0;
    uint8 tbl_info_num  = 0;
    uint8 tbl_id        = 0;

    uint32 bitmap       = 0;
    uint8  mem_id       = 0;
	uint8  mem_cnt      = 0;
    uint32 offset       = 0;
    uint32 entry_size   = 0;


    /********************************************
    dynamic edram table
    *********************************************/
    tbl_info_num = profile_info->tbl_info_size;
    for(tbl_index = 0; tbl_index < tbl_info_num; tbl_index++)
    {
        tbl_id = profile_info->tbl_info[tbl_index].tbl_id;

        if (0xFF == tbl_id)
        {
            continue;
        }

        bitmap = profile_info->tbl_info[tbl_index].mem_bitmap;
        if (DRV_FTM_SRAM_TBL_OAM_MEP == tbl_id)
        {/*oam mep*/
            _drv_usw_ftm_process_oam_tbl(lchip, &profile_info->tbl_info[tbl_index]);
        }
        else
        {

			mem_cnt = 0;
            for (mem_id = 0; mem_id < DRV_FTM_SRAM_MAX; mem_id++)
            {
            char str[20];
                if(!IS_BIT_SET(bitmap, mem_id))
                {
                    continue;
                }
				mem_cnt++;
                offset       = profile_info->tbl_info[tbl_index].mem_start_offset[mem_id];
                entry_size   = profile_info->tbl_info[tbl_index].mem_entry_num[mem_id];
                /* if entry size is larger than mac entry num, means couple mode*/
                if(entry_size > DRV_FTM_MEM_MAX_ENTRY_NUM(mem_id))
                {
                    DRV_FTM_TBL_COUPLE_MODE(tbl_id) = 1;
                }

                DRV_FTM_ADD_TABLE(mem_id, tbl_id, offset, entry_size);

                _drv_usw_ftm_get_sram_tbl_name(tbl_id, str);

            }
			/*add check for ipda & ipsa sperate mode */
			if(DRV_IS_DUET2(lchip) && (tbl_id == DRV_FTM_SRAM_TBL_LPM_LKP_KEY0) && (g_usw_ftm_master[lchip]->lpm_model == LPM_MODEL_PRIVATE_IPDA_IPSA_HALF))
			{
				uint32 entry_num_half = DRV_FTM_TBL_MAX_ENTRY_NUM(tbl_id)/2;
				uint32 tmp_enry_num = 0;
				uint32 bitmap_da = 0;
				uint32 bitmap_sa = bitmap;

				if(bitmap && mem_cnt < 2)
				{
					DRV_FTM_DBG_OUT("Error: alloc lpm memory error, line:%d\n", __LINE__);
					return DRV_E_INVALID_DYNIC_TBL_ALLOC;
				}
				for (mem_id = 0; mem_id < DRV_FTM_SRAM_MAX; mem_id++)
	            {
	                if(!IS_BIT_SET(bitmap, mem_id))
	                {
	                    continue;
	                }
					DRV_BIT_SET(bitmap_da, mem_id);
					DRV_BIT_UNSET(bitmap_sa, mem_id);
					tmp_enry_num += DRV_FTM_TBL_MEM_ENTRY_NUM(tbl_id, mem_id);
					if(tmp_enry_num > entry_num_half)
					{
						DRV_FTM_DBG_OUT("Error: alloc lpm memory error, line:%d\n", __LINE__);
						return DRV_E_INVALID_DYNIC_TBL_ALLOC;
					}
					else if(tmp_enry_num == entry_num_half)
					{

						break;
					}
				}

				/* check pass, realloc IpDa/IpSa lpm memory */
				sal_memset(&(DRV_FTM_TBL_INFO(DRV_FTM_SRAM_TBL_LPM_LKP_KEY0)), 0, sizeof(drv_ftm_sram_tbl_info_t));
				for(tbl_id=DRV_FTM_SRAM_TBL_LPM_LKP_KEY0; tbl_id<=DRV_FTM_SRAM_TBL_LPM_LKP_KEY1;tbl_id++)
				{
					if(tbl_id == DRV_FTM_SRAM_TBL_LPM_LKP_KEY0)
					{
						bitmap = bitmap_da;
					}
					else
					{
						bitmap = bitmap_sa;
					}

					for (mem_id = 0; mem_id < DRV_FTM_SRAM_MAX; mem_id++)
		            {
		                if(!IS_BIT_SET(bitmap, mem_id))
		                {
		                    continue;
		                }
		                offset       = profile_info->tbl_info[tbl_index].mem_start_offset[mem_id];
		                entry_size   = profile_info->tbl_info[tbl_index].mem_entry_num[mem_id];
		                /* if entry size is larger than mac entry num, means couple mode*/
		                if(entry_size > DRV_FTM_MEM_MAX_ENTRY_NUM(mem_id))
		                {
		                    DRV_FTM_TBL_COUPLE_MODE(tbl_id) = 1;
		                }

		                DRV_FTM_ADD_TABLE(mem_id, tbl_id, offset, entry_size);
		            }
				}
			}
            else if(DRV_IS_TSINGMA(lchip) && (tbl_id == DRV_FTM_SRAM_TBL_LPM_LKP_KEY0))
            {
                uint32 temp_tbl_id[2] = {0};
                uint8  tbl_num = 0;
                sal_memset(&(DRV_FTM_TBL_INFO(DRV_FTM_SRAM_TBL_LPM_LKP_KEY0)), 0, sizeof(drv_ftm_sram_tbl_info_t));
                for (mem_id = 0; mem_id < DRV_FTM_SRAM_MAX; mem_id++)
	            {
	                if(!IS_BIT_SET(bitmap, mem_id))
	                {
	                    continue;
	                }
                    tbl_num = 0;
                    offset       = profile_info->tbl_info[tbl_index].mem_start_offset[mem_id];
    		        entry_size   = profile_info->tbl_info[tbl_index].mem_entry_num[mem_id];
                    switch(mem_id)
                    {
                        case DRV_FTM_SRAM0:
                        case DRV_FTM_SRAM3:
                            temp_tbl_id[tbl_num++] = DRV_FTM_SRAM_TBL_LPM_LKP_KEY0;
                            if(g_usw_ftm_master[lchip]->lpm_model == LPM_MODEL_PRIVATE_IPDA_IPSA_HALF)
                            {
                                temp_tbl_id[tbl_num++] = DRV_FTM_SRAM_TBL_LPM_LKP_KEY2;
                            }
                            break;
                        case DRV_FTM_SRAM1:
                        case DRV_FTM_SRAM4:
                            temp_tbl_id[tbl_num++] = DRV_FTM_SRAM_TBL_LPM_LKP_KEY1;
                            if(g_usw_ftm_master[lchip]->lpm_model == LPM_MODEL_PRIVATE_IPDA_IPSA_HALF)
                            {
                                temp_tbl_id[tbl_num++] = DRV_FTM_SRAM_TBL_LPM_LKP_KEY3;
                            }
                            break;
                        case DRV_FTM_SRAM6:
                            temp_tbl_id[tbl_num++] = DRV_FTM_SRAM_TBL_LPM_LKP_KEY0;
                            break;
                        case DRV_FTM_SRAM7:
                            temp_tbl_id[tbl_num++] = DRV_FTM_SRAM_TBL_LPM_LKP_KEY1;
                            break;
                        case DRV_FTM_SRAM8:
                            if(g_usw_ftm_master[lchip]->lpm_model == LPM_MODEL_PRIVATE_IPDA_IPSA_HALF)
                            {
                                temp_tbl_id[tbl_num++] = DRV_FTM_SRAM_TBL_LPM_LKP_KEY2;
                            }
                            else
                            {
                                temp_tbl_id[tbl_num++] = DRV_FTM_SRAM_TBL_LPM_LKP_KEY0;
                            }
                            break;
                        case DRV_FTM_SRAM10:
                            if(g_usw_ftm_master[lchip]->lpm_model == LPM_MODEL_PRIVATE_IPDA_IPSA_HALF)
                            {
                                temp_tbl_id[tbl_num++] = DRV_FTM_SRAM_TBL_LPM_LKP_KEY3;
                            }
                            else
                            {
                                temp_tbl_id[tbl_num++] = DRV_FTM_SRAM_TBL_LPM_LKP_KEY1;
                            }
                            break;
                       default:
                            return DRV_E_INVALID_ALLOC_INFO;
                    }

                    offset       = profile_info->tbl_info[tbl_index].mem_start_offset[mem_id];
		            entry_size   = profile_info->tbl_info[tbl_index].mem_entry_num[mem_id];

                    if(tbl_num == 2)
                    {
                        /*one physical memory have two logic memory*/
                        tbl_id = temp_tbl_id[0];
		                if(entry_size > DRV_FTM_MEM_MAX_ENTRY_NUM(mem_id))
		                {
		                    DRV_FTM_TBL_COUPLE_MODE(tbl_id) = 1;
		                }
		                DRV_FTM_ADD_TABLE(mem_id, tbl_id, offset, entry_size/2);

                        tbl_id = temp_tbl_id[1];
		                if(entry_size > DRV_FTM_MEM_MAX_ENTRY_NUM(mem_id))
		                {
		                    DRV_FTM_TBL_COUPLE_MODE(tbl_id) = 1;
		                }
		                DRV_FTM_ADD_TABLE(mem_id, tbl_id, offset+entry_size/2, entry_size/2);
                    }
                    else if(tbl_num == 1)
                    {
                        tbl_id = temp_tbl_id[0];
		                if(entry_size > DRV_FTM_MEM_MAX_ENTRY_NUM(mem_id))
		                {
		                    DRV_FTM_TBL_COUPLE_MODE(tbl_id) = 1;
		                }
		                DRV_FTM_ADD_TABLE(mem_id, tbl_id, offset, entry_size);
                    }
                }
            }

        }

    }
    return DRV_E_NONE;
}

#define IS_LPM_KEY_TYPE(key_type) \
  ((((key_type) == DRV_FTM_KEY_TYPE_IPV4_UCAST) || ((key_type) == DRV_FTM_KEY_TYPE_IPV6_UCAST) ||\
  ((key_type) == DRV_FTM_KEY_TYPE_IPV4_NAT) || ((key_type) == DRV_FTM_KEY_TYPE_IPV6_UCAST_HALF)) ? 1:0)
STATIC int32
_drv_usw_ftm_set_profile_user_define_lpm_tcam(uint8 lchip, drv_ftm_profile_info_t *profile_info)
{
    uint8  key_idx       = 0;
    uint8  mem_id        = 0;

    uint32 lpm_entry_size = 0;
    uint32 lpm_entry_size_v6 = 0;
    uint32 nat_entry_size = 0;
    /*uint32 total_lookup1 = 0;*/
    uint32 bitmap       = 0;
    uint32 low_bmp;
    uint32 high_bmp;

    uint32 lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM1-DRV_FTM_TCAM_TYPE_IGS_LPM0DA+1] = {0};
    /********************************************
    tcam table
    *********************************************/
    for(key_idx = 0; key_idx < profile_info->key_info_size; key_idx++)
    {

        bitmap = profile_info->key_info[key_idx].tcam_bitmap;

        if(!IS_LPM_KEY_TYPE(profile_info->key_info[key_idx].key_id))
        {
            continue;
        }
        low_bmp = bitmap & 0x030F;   /*bit  0011,0000,1111*/
        high_bmp = bitmap & 0x0CF0;  /*bit  1100,1111,0000*/
        if(DRV_FTM_KEY_TYPE_IPV4_NAT == profile_info->key_info[key_idx].key_id)
        {
            lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM1-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= bitmap;
            for(mem_id = DRV_FTM_LPM_TCAM_KEY0; mem_id < DRV_FTM_LPM_TCAM_KEYM; mem_id++)
            {
                if(!IS_BIT_SET(bitmap, (mem_id - DRV_FTM_LPM_TCAM_KEY0)))
                {
                    continue;
                }
                nat_entry_size  += profile_info->key_info[key_idx].tcam_entry_num[mem_id - DRV_FTM_LPM_TCAM_KEY0];
            }
        }
        else if(DRV_FTM_KEY_TYPE_IPV6_UCAST == profile_info->key_info[key_idx].key_id)
        {
            for(mem_id = DRV_FTM_LPM_TCAM_KEY0; mem_id < DRV_FTM_LPM_TCAM_KEYM; mem_id++)
            {
                if(!IS_BIT_SET(bitmap, (mem_id - DRV_FTM_LPM_TCAM_KEY0)))
                {
                    continue;
                }
                lpm_entry_size   += profile_info->key_info[key_idx].tcam_entry_num[mem_id - DRV_FTM_LPM_TCAM_KEY0];
            }

            switch(g_usw_ftm_master[lchip]->lpm_model)
            {
                case LPM_MODEL_PRIVATE_IPDA:
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= bitmap;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][0] |= bitmap;
                    break;
                case LPM_MODEL_PUBLIC_IPDA_PRIVATE_IPDA_HALF:
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= low_bmp;
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= high_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][0] |= low_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][0] |= high_bmp;
                    break;
                case LPM_MODEL_PRIVATE_IPDA_IPSA_HALF:
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= low_bmp;
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0SA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= high_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][0] |= low_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][0] |= high_bmp;
                    break;
                case LPM_MODEL_PRIVATE_IPDA_IPSA_FULL:
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= bitmap;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][0] |= bitmap;
                    break;
                case LPM_MODEL_PUBLIC_IPDA_IPSA_PRIVATE_IPDA_IPSA_HALF:
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= low_bmp;

                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= high_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][0] |= low_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][0] |= high_bmp;

                    break;
                default:
                    break;
            }

        }
        else if(DRV_FTM_KEY_TYPE_IPV6_UCAST_HALF == profile_info->key_info[key_idx].key_id)
        {
            for(mem_id = DRV_FTM_LPM_TCAM_KEY0; mem_id < DRV_FTM_LPM_TCAM_KEYM; mem_id++)
            {
                if(!IS_BIT_SET(bitmap, (mem_id - DRV_FTM_LPM_TCAM_KEY0)))
                {
                    continue;
                }
                lpm_entry_size_v6  += profile_info->key_info[key_idx].tcam_entry_num[mem_id - DRV_FTM_LPM_TCAM_KEY0];
            }
            switch(g_usw_ftm_master[lchip]->lpm_model)
            {
                case LPM_MODEL_PRIVATE_IPDA:
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= bitmap;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][1] |= bitmap;
                    break;
                case LPM_MODEL_PUBLIC_IPDA_PRIVATE_IPDA_HALF:
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= low_bmp;
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= high_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][1] |= low_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][1] |= high_bmp;
                    break;
                case LPM_MODEL_PRIVATE_IPDA_IPSA_HALF:
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= low_bmp;
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0SA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= high_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][1] |= low_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][1] |= high_bmp;
                    break;
                case LPM_MODEL_PRIVATE_IPDA_IPSA_FULL:
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= bitmap;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][1] |= bitmap;
                    break;
                case LPM_MODEL_PUBLIC_IPDA_IPSA_PRIVATE_IPDA_IPSA_HALF:
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= low_bmp;

                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= high_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][1] |= low_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][1] |= high_bmp;
                    break;
                default:
                    break;
            }
        }
        else if(DRV_FTM_KEY_TYPE_IPV4_UCAST == profile_info->key_info[key_idx].key_id)
        {
            for(mem_id = DRV_FTM_LPM_TCAM_KEY0; mem_id < DRV_FTM_LPM_TCAM_KEYM; mem_id++)
            {
                if(!IS_BIT_SET(bitmap, (mem_id - DRV_FTM_LPM_TCAM_KEY0)))
                {
                    continue;
                }
                lpm_entry_size   += profile_info->key_info[key_idx].tcam_entry_num[mem_id - DRV_FTM_LPM_TCAM_KEY0];
            }

            switch(g_usw_ftm_master[lchip]->lpm_model)
            {
                case LPM_MODEL_PRIVATE_IPDA:
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= bitmap;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][2] |= bitmap;
                    break;
                case LPM_MODEL_PUBLIC_IPDA_PRIVATE_IPDA_HALF:
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= low_bmp;
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= high_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][2] |= low_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][2] |= high_bmp;
                    break;
                case LPM_MODEL_PRIVATE_IPDA_IPSA_HALF:
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= low_bmp;
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0SA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= high_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][2] |= low_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][2] |= high_bmp;
                    break;
                case LPM_MODEL_PRIVATE_IPDA_IPSA_FULL:
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= bitmap;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][2] |= bitmap;
                    break;
                case LPM_MODEL_PUBLIC_IPDA_IPSA_PRIVATE_IPDA_IPSA_HALF:
                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= low_bmp;

                    lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |= high_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][2] |= low_bmp;
                    g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][2] |= high_bmp;

                    break;
                default:
                    break;
            }

        }

    }

    /*check division*/
    /*total_lookup1 = lpm_entry_size+lpm_entry_size_v6;*/
    if(nat_entry_size)
    {
        g_usw_ftm_master[lchip]->nat_pbr_enable = 1;
    }
    else
    {
        g_usw_ftm_master[lchip]->nat_pbr_enable = 0;
    }


    /*check alloced bitmap*/
    switch(g_usw_ftm_master[lchip]->lpm_model)
    {
        case LPM_MODEL_PRIVATE_IPDA:
            if(lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0SA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA] |\
               lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0SAPUB-DRV_FTM_TCAM_TYPE_IGS_LPM0DA])
            {
                DRV_FTM_DBG_OUT("Error: In private ipda mode,can not alloc ipSa resource line:%d\n", __LINE__);
                return DRV_E_INVALID_TCAM_ALLOC;
            }
            break;
        case LPM_MODEL_PRIVATE_IPDA_IPSA_HALF:
            if((lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0SA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA]) == 0 )
            {
                DRV_FTM_DBG_OUT("Error: In ipda and ipsa half key mode, ipSa have no resource, line:%d\n", __LINE__);
                return DRV_E_INVALID_TCAM_ALLOC;
            }

            if((lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA]) == 0)
            {
                DRV_FTM_DBG_OUT("Error: In ipda and ipsa half key mode, ipDa have no resource, line:%d\n", __LINE__);
                return DRV_E_INVALID_TCAM_ALLOC;
            }
            break;
        case LPM_MODEL_PUBLIC_IPDA_PRIVATE_IPDA_HALF:
        case LPM_MODEL_PUBLIC_IPDA_IPSA_PRIVATE_IPDA_IPSA_HALF:
            if((lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB-DRV_FTM_TCAM_TYPE_IGS_LPM0DA]) == 0)
            {
                DRV_FTM_DBG_OUT("Error:In public and private mode, public route have no resource, line:%d\n", __LINE__);
                return DRV_E_INVALID_TCAM_ALLOC;
            }

            if((lpm_mem_bmp[DRV_FTM_TCAM_TYPE_IGS_LPM0DA-DRV_FTM_TCAM_TYPE_IGS_LPM0DA])== 0)
            {
                DRV_FTM_DBG_OUT("Error:In public and private mode, private route have no resource, line:%d\n", __LINE__);
                return DRV_E_INVALID_TCAM_ALLOC;
            }
            break;
        default:
            break;
    }

     /*alloc*/
    for(key_idx = DRV_FTM_TCAM_TYPE_IGS_LPM0DA; key_idx <= DRV_FTM_TCAM_TYPE_IGS_LPM1; key_idx++)
    {
        bitmap = lpm_mem_bmp[key_idx-DRV_FTM_TCAM_TYPE_IGS_LPM0DA];
        for(mem_id=DRV_FTM_LPM_TCAM_KEY0; mem_id < DRV_FTM_LPM_TCAM_KEYM; mem_id++)
        {
            if(!IS_BIT_SET(bitmap, (mem_id-DRV_FTM_LPM_TCAM_KEY0)))
            {
                continue;
            }
            DRV_FTM_ADD_LPM_FULL_KEY(mem_id, key_idx);
        }
    }
#if 0
     /*just for cmodel*/
    key_idx = DRV_FTM_TCAM_TYPE_IGS_LPM_ALL;
    for(mem_id=DRV_FTM_LPM_TCAM_KEY0; mem_id < DRV_FTM_LPM_TCAM_KEYM; mem_id++)
    {
        DRV_FTM_ADD_LPM_FULL_KEY(mem_id, key_idx);
    }
#endif
     /*calculate boundary*/

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_ftm_set_profile_user_define_tcam(uint8 lchip, drv_ftm_profile_info_t *profile_info)
{
    uint8 key_idx       = 0;
    uint8 key_info_num  = 0;
    uint32 bitmap       = 0;
    uint8 mem_id        = 0;

    uint8  key_id       = 0;
    uint8  key_type      = 0;

    uint32 offset       = 0;
    uint32 entry_size   = 0;

    /********************************************
    tcam table
    *********************************************/
    key_info_num = profile_info->key_info_size;
    for(key_idx = 0; key_idx < key_info_num; key_idx++)
    {
        key_id = profile_info->key_info[key_idx].key_id;

        if (0xFF == key_id
            || IS_LPM_KEY_TYPE(key_id))
        {
            continue;
        }

        bitmap = profile_info->key_info[key_idx].tcam_bitmap;
        for(mem_id = DRV_FTM_TCAM_KEY0; mem_id < DRV_FTM_TCAM_KEYM; mem_id++)
        {
            if(IS_BIT_SET(bitmap, (mem_id - DRV_FTM_TCAM_KEY0)))
            {
                offset       = profile_info->key_info[key_idx].tcam_start_offset[mem_id - DRV_FTM_TCAM_KEY0];
                entry_size   = profile_info->key_info[key_idx].tcam_entry_num[mem_id - DRV_FTM_TCAM_KEY0];

                DRV_FTM_ADD_KEY(mem_id, key_id, 0, entry_size);
                DRV_FTM_ADD_KEY_BYTYPE(mem_id, key_id, key_type, offset, entry_size);
            }
        }
    }

    return DRV_E_NONE;

}

STATIC int32
_drv_usw_ftm_set_profile_user_define(uint8 lchip, drv_ftm_profile_info_t *profile_info)
{

    _drv_usw_ftm_set_profile_user_define_edram(lchip, profile_info);

    _drv_usw_ftm_set_profile_user_define_tcam(lchip, profile_info);
    _drv_usw_ftm_set_profile_user_define_lpm_tcam(lchip, profile_info);

    return DRV_E_NONE;
}



STATIC int32
_drv_usw_ftm_get_sram_tbl_id(uint8 lchip, uint32 sram_type,
                                   uint32* p_sram_tbl_id,
                                   uint8* p_share_num)
{
    uint8 idx = 0;

    DRV_PTR_VALID_CHECK(p_sram_tbl_id);
    DRV_PTR_VALID_CHECK(p_share_num);

    switch (sram_type)
    {

    case DRV_FTM_SRAM_TBL_MAC_HASH_KEY:

        if (DRV_IS_DUET2(lchip))
        {
            break;
        }
        p_sram_tbl_id[idx++] = DsFibHost0MacHashKey_t;
        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;
        break;

    case DRV_FTM_SRAM_TBL_FIB0_HASH_KEY:
        if (DRV_IS_DUET2(lchip))
        {
            p_sram_tbl_id[idx++] = DsFibHost0MacHashKey_t;
        }
        p_sram_tbl_id[idx++] = DsFibHost0Ipv4HashKey_t,
        p_sram_tbl_id[idx++] = DsFibHost0Ipv6UcastHashKey_t,
        p_sram_tbl_id[idx++] = DsFibHost0Ipv6McastHashKey_t,
        p_sram_tbl_id[idx++] = DsFibHost0MacIpv6McastHashKey_t,
        p_sram_tbl_id[idx++] = DsFibHost0TrillHashKey_t,
        p_sram_tbl_id[idx++] = DsFibHost0FcoeHashKey_t,
        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;
        break;

    case DRV_FTM_SRAM_TBL_FIB1_HASH_KEY:
		if(g_usw_ftm_master[lchip]->napt_enable)
		{
        	p_sram_tbl_id[idx++] = DsFibHost1Ipv4NatDaPortHashKey_t;
        	p_sram_tbl_id[idx++] = DsFibHost1Ipv4NatSaPortHashKey_t;
        	p_sram_tbl_id[idx++] = DsFibHost1Ipv6NatDaPortHashKey_t;
        	p_sram_tbl_id[idx++] = DsFibHost1Ipv6NatSaPortHashKey_t;
        }

        p_sram_tbl_id[idx++] = DsFibHost1Ipv4McastHashKey_t,
		p_sram_tbl_id[idx++] = DsFibHost1Ipv6McastHashKey_t,
        p_sram_tbl_id[idx++] = DsFibHost1MacIpv4McastHashKey_t,
        p_sram_tbl_id[idx++] = DsFibHost1FcoeRpfHashKey_t,
        p_sram_tbl_id[idx++] = DsFibHost1TrillMcastVlanHashKey_t,
        p_sram_tbl_id[idx++] = DsFibHost1MacIpv6McastHashKey_t,
        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;
        break;

    case DRV_FTM_SRAM_TBL_FLOW_HASH_KEY:
        p_sram_tbl_id[idx++] = DsFlowL2HashKey_t,
        p_sram_tbl_id[idx++] = DsFlowL2L3HashKey_t,
        p_sram_tbl_id[idx++] = DsFlowL3Ipv4HashKey_t,
        p_sram_tbl_id[idx++] = DsFlowL3Ipv6HashKey_t,
        p_sram_tbl_id[idx++] = DsFlowL3MplsHashKey_t,
        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;
        break;

    case DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY:
        p_sram_tbl_id[idx++] = DsIpfixL2HashKey_t,
        p_sram_tbl_id[idx++] = DsIpfixL2L3HashKey_t,
        p_sram_tbl_id[idx++] = DsIpfixL3Ipv4HashKey_t,
        p_sram_tbl_id[idx++] = DsIpfixL3Ipv6HashKey_t,
        p_sram_tbl_id[idx++] = DsIpfixL3MplsHashKey_t,
        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;
        break;

    case DRV_FTM_SRAM_TBL_USERID_HASH_KEY:
        p_sram_tbl_id[idx++] = DsUserIdCvlanCosPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdCvlanPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdDoubleVlanPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdIpv4PortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdIpv4SaHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdIpv6PortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdIpv6SaHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdMacHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdMacPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdSvlanCosPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdSvlanHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdSvlanMacSaHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdSvlanPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdSclFlowL2HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv4DaHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv4GreKeyHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv4HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv4RpfHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv4UdpHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv4McNvgreMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv4NvgreMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv4McVxlanMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv4VxlanMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv4UcNvgreMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv4UcNvgreMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv4UcVxlanMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv4UcVxlanMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv6DaHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv6GreKeyHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv6HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv6UdpHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv6McNvgreMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv6McNvgreMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv6McVxlanMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv6McVxlanMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv6UcNvgreMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv6UcNvgreMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv6UcVxlanMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv6UcVxlanMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelMplsHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelPbbHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelTrillMcAdjHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelTrillMcDecapHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelTrillMcRpfHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelTrillUcDecapHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelTrillUcRpfHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdCapwapMacDaForwardHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdCapwapStaStatusHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdCapwapStaStatusMcHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdCapwapVlanForwardHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv4CapwapHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelIpv6CapwapHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelCapwapRmacHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdTunnelCapwapRmacRidHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdEcidNameSpaceHashKey_t,
        p_sram_tbl_id[idx++] = DsUserIdIngEcidNameSpaceHashKey_t,


        p_sram_tbl_id[idx++] = DsUserId0CvlanCosPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0CvlanPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0DoubleVlanPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0Ipv4PortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0Ipv4SaHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0Ipv6PortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0Ipv6SaHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0MacHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0MacPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0PortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0SvlanCosPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0SvlanHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0SvlanMacSaHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0SvlanPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0SclFlowL2HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv4DaHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv4GreKeyHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv4HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv4RpfHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv4UdpHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv4McNvgreMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv4NvgreMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv4McVxlanMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv4VxlanMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv4UcNvgreMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv4UcNvgreMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv4UcVxlanMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv4UcVxlanMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv6DaHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv6GreKeyHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv6HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv6UdpHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv6McNvgreMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv6McNvgreMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv6McVxlanMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv6McVxlanMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv6UcNvgreMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv6UcNvgreMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv6UcVxlanMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv6UcVxlanMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelTrillMcAdjHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelTrillMcDecapHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelTrillMcRpfHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelTrillUcDecapHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelTrillUcRpfHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0CapwapMacDaForwardHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0CapwapStaStatusHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0CapwapStaStatusMcHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0CapwapVlanForwardHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv4CapwapHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelIpv6CapwapHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelCapwapRmacHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0TunnelCapwapRmacRidHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0EcidNameSpaceHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId0IngEcidNameSpaceHashKey_t,

        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;
        break;


    case DRV_FTM_SRAM_TBL_USERID1_HASH_KEY:

        if (DRV_IS_DUET2(lchip))
        {
            break;
        }

        p_sram_tbl_id[idx++] = DsUserId1CvlanCosPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1CvlanPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1DoubleVlanPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1Ipv4PortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1Ipv4SaHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1Ipv6PortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1Ipv6SaHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1MacHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1MacPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1PortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1SvlanCosPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1SvlanHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1SvlanMacSaHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1SvlanPortHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1SclFlowL2HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv4DaHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv4GreKeyHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv4HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv4RpfHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv4UdpHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv4McNvgreMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv4NvgreMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv4McVxlanMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv4VxlanMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv4UcNvgreMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv4UcNvgreMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv4UcVxlanMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv4UcVxlanMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv6DaHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv6GreKeyHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv6HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv6UdpHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv6McNvgreMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv6McNvgreMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv6McVxlanMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv6McVxlanMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv6UcNvgreMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv6UcNvgreMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv6UcVxlanMode0HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv6UcVxlanMode1HashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelTrillMcAdjHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelTrillMcDecapHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelTrillMcRpfHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelTrillUcDecapHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelTrillUcRpfHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1CapwapMacDaForwardHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1CapwapStaStatusHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1CapwapStaStatusMcHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1CapwapVlanForwardHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv4CapwapHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelIpv6CapwapHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelCapwapRmacHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1TunnelCapwapRmacRidHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1EcidNameSpaceHashKey_t,
        p_sram_tbl_id[idx++] = DsUserId1IngEcidNameSpaceHashKey_t,

        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;
        break;


    case DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY:
        p_sram_tbl_id[idx++] = DsEgressXcOamPortCrossHashKey_t,
        p_sram_tbl_id[idx++] = DsEgressXcOamPortHashKey_t,
        p_sram_tbl_id[idx++] = DsEgressXcOamPortVlanCrossHashKey_t,
        p_sram_tbl_id[idx++] = DsEgressXcOamCvlanCosPortHashKey_t,
        p_sram_tbl_id[idx++] = DsEgressXcOamCvlanPortHashKey_t,
        p_sram_tbl_id[idx++] = DsEgressXcOamDoubleVlanPortHashKey_t,
        p_sram_tbl_id[idx++] = DsEgressXcOamSvlanCosPortHashKey_t,
        p_sram_tbl_id[idx++] = DsEgressXcOamSvlanPortHashKey_t,
        p_sram_tbl_id[idx++] = DsEgressXcOamSvlanPortMacHashKey_t,
        p_sram_tbl_id[idx++] = DsEgressXcOamTunnelPbbHashKey_t,
        p_sram_tbl_id[idx++] = DsEgressXcOamBfdHashKey_t,
        p_sram_tbl_id[idx++] = DsEgressXcOamEthHashKey_t,
        p_sram_tbl_id[idx++] = DsEgressXcOamMplsLabelHashKey_t,
        p_sram_tbl_id[idx++] = DsEgressXcOamMplsSectionHashKey_t,
        p_sram_tbl_id[idx++] = DsEgressXcOamRmepHashKey_t,

        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;
        break;

    case DRV_FTM_SRAM_TBL_LPM_LKP_KEY0:
        p_sram_tbl_id[idx++] = DsLpmLookupKey_t;
        p_sram_tbl_id[idx++] = DsNeoLpmIpv4Bit32Snake_t;
        p_sram_tbl_id[idx++] = DsNeoLpmIpv6Bit64Snake_t;
        p_sram_tbl_id[idx++] = DsNeoLpmIpv6Bit128Snake_t;

        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;

        break;
    case DRV_FTM_SRAM_TBL_LPM_LKP_KEY1:
        p_sram_tbl_id[idx++] = DsLpmLookupKey1_t;
        p_sram_tbl_id[idx++] = DsNeoLpmIpv4Bit32Snake1_t;
        p_sram_tbl_id[idx++] = DsNeoLpmIpv6Bit64Snake1_t;
        p_sram_tbl_id[idx++] = DsNeoLpmIpv6Bit128Snake1_t;

        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;

        break;
    case DRV_FTM_SRAM_TBL_LPM_LKP_KEY2:
        p_sram_tbl_id[idx++] = DsNeoLpmIpv4Bit32Snake2_t;
        p_sram_tbl_id[idx++] = DsNeoLpmIpv6Bit64Snake2_t;
        p_sram_tbl_id[idx++] = DsNeoLpmIpv6Bit128Snake2_t;

        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;

        break;
    case DRV_FTM_SRAM_TBL_LPM_LKP_KEY3:
        p_sram_tbl_id[idx++] = DsNeoLpmIpv4Bit32Snake3_t;
        p_sram_tbl_id[idx++] = DsNeoLpmIpv6Bit64Snake3_t;
        p_sram_tbl_id[idx++] = DsNeoLpmIpv6Bit128Snake3_t;

        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;

        break;
    case DRV_FTM_SRAM_TBL_DSMAC_AD:
        p_sram_tbl_id[idx++] = DsMac_t;
        break;

    case DRV_FTM_SRAM_TBL_DSIP_AD:
        p_sram_tbl_id[idx++] = DsIpDa_t;
        p_sram_tbl_id[idx++] = DsIpSaNat_t;
        p_sram_tbl_id[idx++] = DsFcoeDa_t;
        p_sram_tbl_id[idx++] = DsFcoeSa_t;
        p_sram_tbl_id[idx++] = DsTrillDa_t;
        break;

    case DRV_FTM_SRAM_TBL_FLOW_AD:
        p_sram_tbl_id[idx++] = DsFlow_t;
        break;

    case DRV_FTM_SRAM_TBL_IPFIX_AD:
        p_sram_tbl_id[idx++] = DsIpfixSessionRecord_t;
        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 0;
        break;

    case DRV_FTM_SRAM_TBL_USERID_AD:
        p_sram_tbl_id[idx++] = DsTunnelId_t;
        p_sram_tbl_id[idx++] = DsTunnelIdHalf_t;
        p_sram_tbl_id[idx++] = DsUserId_t;
        p_sram_tbl_id[idx++] = DsUserIdHalf_t;
        p_sram_tbl_id[idx++] = DsSclFlow_t;
        p_sram_tbl_id[idx++] = DsTunnelIdRpf_t;

        p_sram_tbl_id[idx++] = DsTunnelId0_t;
        p_sram_tbl_id[idx++] = DsTunnelIdHalf0_t;
        p_sram_tbl_id[idx++] = DsUserId0_t;
        p_sram_tbl_id[idx++] = DsUserIdHalf0_t;
        p_sram_tbl_id[idx++] = DsSclFlow0_t;
        p_sram_tbl_id[idx++] = DsTunnelIdRpf0_t;


        if (DRV_IS_DUET2(lchip))
        {
            p_sram_tbl_id[idx++] = DsMpls_t;
            p_sram_tbl_id[idx++] = DsMplsHalf_t;
        }



        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;
        break;

    case DRV_FTM_SRAM_TBL_USERID_AD1:

        if (DRV_IS_DUET2(lchip))
        {
            break;
        }

        p_sram_tbl_id[idx++] = DsTunnelId1_t;
        p_sram_tbl_id[idx++] = DsTunnelIdHalf1_t;
        p_sram_tbl_id[idx++] = DsUserId1_t;
        p_sram_tbl_id[idx++] = DsUserIdHalf1_t;
        p_sram_tbl_id[idx++] = DsSclFlow1_t;
        p_sram_tbl_id[idx++] = DsTunnelIdRpf1_t;
        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;
        break;


    case DRV_FTM_SRAM_TBL_NEXTHOP:
        p_sram_tbl_id[idx++] = DsNextHop4W_t;
        p_sram_tbl_id[idx++] = DsNextHop8W_t;
        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;
        break;

    case DRV_FTM_SRAM_TBL_FWD:
        p_sram_tbl_id[idx++] = DsFwd_t;
        p_sram_tbl_id[idx++] = DsFwdDualHalf_t;
        p_sram_tbl_id[idx++] = DsFwdHalf_t;
        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;
        break;

    case DRV_FTM_SRAM_TBL_MET:
        p_sram_tbl_id[idx++] = DsMetEntry3W_t;
        p_sram_tbl_id[idx++] = DsMetEntry6W_t;
        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;
        break;

    case DRV_FTM_SRAM_TBL_EDIT:
        p_sram_tbl_id[idx++] = DsL23Edit3W_t;
        p_sram_tbl_id[idx++] = DsL23Edit12W_t;
        p_sram_tbl_id[idx++] = DsL23Edit6W_t;
        p_sram_tbl_id[idx++] = DsL2EditEth3W_t;
        p_sram_tbl_id[idx++] = DsL2EditEth6W_t;
        p_sram_tbl_id[idx++] = DsL2EditInnerSwap_t;
        p_sram_tbl_id[idx++] = DsL2EditFlex_t;
        p_sram_tbl_id[idx++] = DsL2EditLoopback_t;
        p_sram_tbl_id[idx++] = DsL2EditPbb4W_t;
        p_sram_tbl_id[idx++] = DsL2EditPbb8W_t;
        p_sram_tbl_id[idx++] = DsL2EditSwap_t;
        p_sram_tbl_id[idx++] = DsL2EditOf_t;
        p_sram_tbl_id[idx++] = DsL3EditFlex_t;
        p_sram_tbl_id[idx++] = DsL3EditLoopback_t;
        p_sram_tbl_id[idx++] = DsL3EditMpls3W_t;
        p_sram_tbl_id[idx++] = DsL3EditMpls12W_t;
        p_sram_tbl_id[idx++] = DsL3EditNat3W_t;
        p_sram_tbl_id[idx++] = DsL3EditNat6W_t;
        p_sram_tbl_id[idx++] = DsL3EditOf6W_t;
        p_sram_tbl_id[idx++] = DsL3EditOf12W_t;
        p_sram_tbl_id[idx++] = DsL3EditOf12WIpv6Only_t;
        p_sram_tbl_id[idx++] = DsL3EditOf12WArpData_t;
        p_sram_tbl_id[idx++] = DsL3EditOf12WIpv6L4_t;
        p_sram_tbl_id[idx++] = DsL3EditTrill_t;
        p_sram_tbl_id[idx++] = DsL3EditTunnelV4_t;
        p_sram_tbl_id[idx++] = DsL3EditTunnelV6_t;
        p_sram_tbl_id[idx++] = DsL3Edit12W1st_t;
        p_sram_tbl_id[idx++] = DsL3Edit12W2nd_t;
        p_sram_tbl_id[idx++] = DsL2Edit12WInner_t;
        p_sram_tbl_id[idx++] = DsL2Edit12WShare_t;
        p_sram_tbl_id[idx++] = DsL2Edit6WShare_t;
        p_sram_tbl_id[idx++] = DsL2EditDsLite_t;
        p_sram_tbl_id[idx++] = TempInnerEdit12W_t;
        p_sram_tbl_id[idx++] = TempOuterEdit12W_t;
        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;
    break;

    case DRV_FTM_SRAM_TBL_OAM_APS:
        p_sram_tbl_id[idx++] = DsApsBridge_t;
        break;

    case DRV_FTM_SRAM_TBL_OAM_LM:
        p_sram_tbl_id[idx++] = DsOamLmStats_t;
        break;

    case DRV_FTM_SRAM_TBL_OAM_MEP:
        p_sram_tbl_id[idx++] = DsBfdMep_t;
        p_sram_tbl_id[idx++] = DsBfdRmep_t;
        p_sram_tbl_id[idx++] = DsEthMep_t;
        p_sram_tbl_id[idx++] = DsEthRmep_t;
        break;

    case DRV_FTM_SRAM_TBL_OAM_MA:
        p_sram_tbl_id[idx++] = DsMa_t;
        break;

    case DRV_FTM_SRAM_TBL_OAM_MA_NAME:
        p_sram_tbl_id[idx++] = DsMaName_t;
        break;


    case DRV_FTM_SRAM_TBL_MPLS_HASH_KEY:
        p_sram_tbl_id[idx++] = DsMplsLabelHashKey_t;
        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;
        break;

    case DRV_FTM_SRAM_TBL_MPLS_HASH_AD:
        p_sram_tbl_id[idx++] = DsMpls_t;
        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 2;
        break;


    case DRV_FTM_SRAM_TBL_GEM_PORT:
        p_sram_tbl_id[idx++] = DsGemPortHashKey_t;
        DRV_FTM_TBL_ACCESS_FLAG(sram_type) = 1;
        break;

    default:
        return DRV_E_INVAILD_TYPE;

    }

    *p_share_num = idx;

    return DRV_E_NONE;

}



/*Diff between Tsingma and D2*/
STATIC int32
_sys_usw_ftm_get_edram_bitmap(uint8 lchip, uint8 sram_type,
                                     uint32* bitmap,
                                     uint32* sram_array)
{
    uint32 bit_map_temp = 0;
    uint32 mem_bit_map  = 0;
    uint8 mem_id        = 0;
    uint8 idx           = 0;
    uint32 bit[DRV_FTM_EDRAM_MAX]     = {0};
    uint8 bit_idx = 0;

    DRV_MEM_GET_EDRAM_BITMAP_FUNC(lchip, sram_type, bit);

    mem_bit_map = DRV_FTM_TBL_MEM_BITMAP(sram_type);

    for (mem_id = DRV_FTM_SRAM0; mem_id < DRV_FTM_EDRAM_MAX; mem_id++)
    {
        bit_idx = (mem_id >= DRV_FTM_MIXED0)? (mem_id - DRV_FTM_MIXED0): mem_id;

        if (IS_BIT_SET(mem_bit_map, bit_idx) && DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, mem_id))
        {
            idx = bit[mem_id];
            SET_BIT(bit_map_temp, idx);
            sram_array[idx] = mem_id;
        }
    }

    *bitmap = bit_map_temp;

    return DRV_E_NONE;
}


/*Diff between Tsingma and D2*/
STATIC int32
_sys_usw_ftm_get_hash_type(uint8 lchip,
                           uint8 sram_type,
                           uint32 mem_id,
                           uint8 couple,
                           uint32 *poly)
{

   DRV_MEM_GET_HASH_TYPE_FUNC(lchip, sram_type, mem_id, couple, poly);

    return DRV_E_NONE;
}


tbls_ext_info_t*
drv_usw_ftm_build_dynamic_tbl_info()
{
    tbls_ext_info_t* p_tbl_ext_info = NULL;
    dynamic_mem_ext_content_t* p_dyn_mem_ext_info = NULL;

    p_tbl_ext_info = mem_malloc(MEM_FTM_MODULE, sizeof(tbls_ext_info_t));
    if (NULL == p_tbl_ext_info)
    {
        return NULL;
    }

    sal_memset(p_tbl_ext_info, 0, sizeof(tbls_ext_info_t));

    p_dyn_mem_ext_info = mem_malloc(MEM_FTM_MODULE, sizeof(dynamic_mem_ext_content_t));
    if (NULL == p_dyn_mem_ext_info)
    {
        mem_free(p_tbl_ext_info);
        return NULL;
    }

    sal_memset(p_dyn_mem_ext_info, 0, sizeof(dynamic_mem_ext_content_t));

    p_tbl_ext_info->ptr_ext_content = p_dyn_mem_ext_info;

    return p_tbl_ext_info;

}


STATIC int32
drv_usw_ftm_alloc_dyn_share_tbl(uint8 lchip, uint8 sram_type,
                                       uint32 tbl_id, uint8 force_realloc)
{
    uint32 bitmap = 0;
    uint32 sram_array[16] = {0};
    uint8   bit_idx = 0;

    uint32 mem_entry_num = 0;
    uint32 max_index_num = 0;
    uint32 start_index = 0;
    uint32 end_index = 0;

    uint32 start_base = 0;
    uint32 end_base = 0;
    uint32 entry_num = 0;

    uint32 hw_base = 0;
    uint32 tbl_offset = 0;
    int8 mem_id = 0;

#if (0 == SDK_WORK_PLATFORM)
    uint8 is_mep = 0;
#endif
    uint32 hw_addr_base_offset_slice0 = 0, hw_addr_base_offset_slice1 = 0;

    dynamic_mem_ext_content_t* p_dyn_mem_ext_info = NULL;
    tbls_ext_info_t* p_tbl_ext_info = NULL;

#if (0 == SDK_WORK_PLATFORM)
    is_mep      = (DRV_FTM_SRAM_TBL_OAM_MEP == sram_type);
#endif
    if (0 == DRV_FTM_TBL_MAX_ENTRY_NUM(sram_type))
    {
        return DRV_E_NONE;
    }

    if(!TABLE_EXT_INFO_PTR(lchip, tbl_id) )
    {
        p_tbl_ext_info = drv_usw_ftm_build_dynamic_tbl_info();
        if (NULL == p_tbl_ext_info)
        {
            return DRV_E_NO_MEMORY;
        }
        TABLE_EXT_INFO_PTR(lchip, tbl_id) = p_tbl_ext_info;
        TABLE_EXT_INFO_PTR(lchip, tbl_id) = p_tbl_ext_info;
    }
	else
	{
		max_index_num = force_realloc ? 0 : TABLE_MAX_INDEX(lchip, tbl_id);
	}


    p_dyn_mem_ext_info = DYNAMIC_EXT_INFO_PTR(lchip, tbl_id);
    if (sram_type == DRV_FTM_SRAM_TBL_MPLS_HASH_KEY ||
        sram_type == DRV_FTM_SRAM_TBL_MPLS_HASH_AD ||
        sram_type == DRV_FTM_SRAM_TBL_GEM_PORT)
    {
        TABLE_EXT_INFO_TYPE(lchip, tbl_id) = EXT_INFO_TYPE_MIXED;
    }
    else
    {
        TABLE_EXT_INFO_TYPE(lchip, tbl_id) = EXT_INFO_TYPE_DYNAMIC;
    }

    p_dyn_mem_ext_info->dynamic_access_mode = DYNAMIC_DEFAULT;

    if (DRV_FTM_TBL_ACCESS_FLAG(sram_type) == 1)
    {
        if (TABLE_ENTRY_SIZE(lchip, tbl_id) == (4 * DRV_BYTES_PER_ENTRY))
        {
            p_dyn_mem_ext_info->dynamic_access_mode = DYNAMIC_16W_MODE;
        }
        else if (TABLE_ENTRY_SIZE(lchip, tbl_id) == (2 * DRV_BYTES_PER_ENTRY))
        {
            p_dyn_mem_ext_info->dynamic_access_mode = DYNAMIC_8W_MODE;
        }
        else if (TABLE_ENTRY_SIZE(lchip, tbl_id) == DRV_BYTES_PER_ENTRY)
        {
            p_dyn_mem_ext_info->dynamic_access_mode = DYNAMIC_4W_MODE;
        }
        else if (TABLE_ENTRY_SIZE(lchip, tbl_id) == 2*DRV_BYTES_PER_WORD)
        {
            p_dyn_mem_ext_info->dynamic_access_mode = DYNAMIC_2W_MODE;
        }
        else if (TABLE_ENTRY_SIZE(lchip, tbl_id) == DRV_BYTES_PER_WORD)
        {
            p_dyn_mem_ext_info->dynamic_access_mode = DYNAMIC_1W_MODE;
        }

    }

    DYNAMIC_BITMAP(lchip, tbl_id) |= DRV_FTM_TBL_MEM_BITMAP(sram_type);
    /* SDK的memid和芯片的不一样顺序,进行调整; */
    _sys_usw_ftm_get_edram_bitmap(lchip, sram_type,   &bitmap,   sram_array);
     /*-bitmap = DYNAMIC_BITMAP(lchip, tbl_id);*/

    for (bit_idx = 0; bit_idx < 16; bit_idx++)
    {
        if (!IS_BIT_SET(bitmap, bit_idx))
        {
            continue;
        }



        mem_id = sram_array[bit_idx];
        mem_entry_num = DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, mem_id);

        if (TABLE_ENTRY_SIZE(lchip, tbl_id) == (4 * DRV_BYTES_PER_ENTRY))
        {
            hw_base = DRV_FTM_MEM_HW_DATA_BASE12W(mem_id);
        }
        else if (TABLE_ENTRY_SIZE(lchip, tbl_id) == (2 * DRV_BYTES_PER_ENTRY))
        {
            hw_base = DRV_FTM_MEM_HW_DATA_BASE6W(mem_id);
        }
        else if (TABLE_ENTRY_SIZE(lchip, tbl_id) == DRV_BYTES_PER_ENTRY)
        {
            hw_base = DRV_FTM_MEM_HW_DATA_BASE(mem_id);
        }
        else if (TABLE_ENTRY_SIZE(lchip, tbl_id) == 2*DRV_BYTES_PER_WORD)
        {
            hw_base = DRV_FTM_MEM_HW_DATA_BASE(mem_id); /*2w base*/
        }

#if (SDK_WORK_PLATFORM == 1)
        hw_base = DRV_FTM_MEM_HW_DATA_BASE(mem_id);
#endif


        hw_addr_base_offset_slice0 = hw_base - DYNAMIC_SLICE0_ADDR_BASE_OFFSET_TO_DUP;
        hw_addr_base_offset_slice1 = hw_base - DYNAMIC_SLICE1_ADDR_BASE_OFFSET_TO_DUP;


        DYNAMIC_ENTRY_NUM(lchip, tbl_id, mem_id) = mem_entry_num;
        tbl_offset = DRV_FTM_TBL_MEM_START_OFFSET(sram_type, mem_id);


#if (0 == SDK_WORK_PLATFORM)
        /*
CPU indirect access to mep tables */
        if (is_mep)
        {
            DYNAMIC_DATA_BASE(lchip, tbl_id, mem_id, 0) = TABLE_DATA_BASE(lchip, OamDsMpData_t, 0) ;
        }
        else if (DRV_FTM_SRAM_TBL_OAM_MA == sram_type)
        {
            DYNAMIC_DATA_BASE(lchip, tbl_id, mem_id, 0) = TABLE_DATA_BASE(lchip, OamDsMaData_t, 0);
        }
        else if (DRV_FTM_SRAM_TBL_OAM_MA_NAME == sram_type)
        {
            DYNAMIC_DATA_BASE(lchip, tbl_id, mem_id, 0) = TABLE_DATA_BASE(lchip, OamDsMaNameData_t, 0);
        }
        else if (DRV_FTM_SRAM_TBL_OAM_LM == sram_type)/*TBD-Need test LM on TM board*/
        {
            DYNAMIC_DATA_BASE(lchip, tbl_id, mem_id, 0) = TABLE_DATA_BASE(lchip, DsOamLmStatsRtl_t, 0);
        }
        else if(DRV_IS_TSINGMA(lchip) && (DRV_FTM_SRAM_TBL_GEM_PORT == sram_type))
        {
            if(mem_id == DRV_FTM_MIXED2)
            {
                DYNAMIC_DATA_BASE(lchip, tbl_id, mem_id, 0) = TABLE_DATA_BASE(lchip, MplsHashAd2w_t, 0);
            }
            else if(mem_id == DRV_FTM_MIXED1)
            {
                DYNAMIC_DATA_BASE(lchip, tbl_id, mem_id, 0) = 0x01D2C000;
            }
            else if(mem_id == DRV_FTM_MIXED0)
            {
                DYNAMIC_DATA_BASE(lchip, tbl_id, mem_id, 0) = TABLE_DATA_BASE(lchip, MplsHashGemPortKey2w_t, 0);
            }
        }
        else
#endif
       {
            DYNAMIC_DATA_BASE(lchip, tbl_id, mem_id, 0) = hw_base + tbl_offset* DRV_ADDR_BYTES_PER_ENTRY;
            DYNAMIC_DATA_BASE(lchip, tbl_id, mem_id, 1) = hw_addr_base_offset_slice0 + tbl_offset* DRV_ADDR_BYTES_PER_ENTRY;
            DYNAMIC_DATA_BASE(lchip, tbl_id, mem_id, 2) = hw_addr_base_offset_slice1 + tbl_offset* DRV_ADDR_BYTES_PER_ENTRY;
        }

        start_index = max_index_num; /*per key index*/
        start_base = entry_num; /*per 80bit base*/
        entry_num += mem_entry_num;

        if (DRV_FTM_TBL_ACCESS_FLAG(sram_type))
        {
            max_index_num += mem_entry_num;
        }
        else
        {
            max_index_num += mem_entry_num * DRV_BYTES_PER_ENTRY / TABLE_ENTRY_SIZE(lchip, tbl_id);
        }

        end_index = (max_index_num - 1);
        end_base = entry_num - 1;

        DYNAMIC_START_INDEX(lchip, tbl_id, mem_id) = start_index;
        DYNAMIC_END_INDEX(lchip, tbl_id, mem_id) = end_index;
        DYNAMIC_MEM_OFFSET(lchip, tbl_id, mem_id) = tbl_offset;

        DRV_FTM_TBL_BASE_VAL(sram_type, bit_idx) = tbl_offset;
        DRV_FTM_TBL_BASE_MIN(sram_type, bit_idx) = start_base;
        DRV_FTM_TBL_BASE_MAX(sram_type, bit_idx) = end_base;


    }

    TABLE_EXT_INFO(lchip, tbl_id).entry = max_index_num;
    DRV_FTM_TBL_BASE_BITMAP(sram_type) = bitmap;


    for (bit_idx = 0; bit_idx < DRV_FTM_MAX_MEM_NUM; bit_idx++)
    {
        if (IS_BIT_SET(bitmap, bit_idx))
        {
            mem_id = sram_array[bit_idx];
            TABLE_EXT_INFO(lchip, tbl_id).addrs[0] = DYNAMIC_DATA_BASE(lchip, tbl_id, mem_id, 0);
            break;
        }
    }


    return DRV_E_NONE;
}



STATIC int32
drv_usw_ftm_alloc_sram(uint8 lchip, uint32 start_type, uint32 end_type, uint8 force_realloc)
{
    uint8 index = 0;
    uint32 sram_type = 0;
    uint32 sram_tbl_id = MaxTblId_t;
    uint32 sram_tbl_a[200] = {0};
    uint8 share_tbl_num  = 0;
    uint32 max_entry_num = 0;

    for (sram_type = start_type; sram_type < end_type; sram_type++)
    {
        max_entry_num = DRV_FTM_TBL_MAX_ENTRY_NUM(sram_type);
        if (0 == max_entry_num)
        {
            continue;
        }

        _drv_usw_ftm_get_sram_tbl_id(lchip, sram_type,
                                           sram_tbl_a,
                                           &share_tbl_num);

        for (index = 0; index < share_tbl_num; index++)
        {
            sram_tbl_id = sram_tbl_a[index];

            if (sram_tbl_id >= MaxTblId_t)
            {
                DRV_FTM_DBG_DUMP("unexpect tbl id:%d\r\n", sram_tbl_id);
                return DRV_E_INVAILD_TYPE;
            }

            if ( (NULL == TABLE_INFO_PTR(lchip, sram_tbl_id)) || 0 == TABLE_FIELD_NUM(lchip, sram_tbl_id))
            {
                continue;
            }
            DRV_IF_ERROR_RETURN(drv_usw_ftm_alloc_dyn_share_tbl(lchip, sram_type, sram_tbl_id, force_realloc));
        }

        index = 0;

    }

    return DRV_E_NONE;
}

#define ___change_table_config
STATIC int32
_drv_usw_ftm_get_tcam_info(uint8 lchip,
                                 uint32 tcam_key_type,
                                 uint32* p_tcam_key_id,
                                 uint32* p_tcam_ad_tbl,
                                 uint8* p_key_share_num,
                                 uint8* p_ad_share_num)
{
    uint8 idx0 = 0;
    uint8 idx1 = 0;

    DRV_PTR_VALID_CHECK(p_tcam_key_id);
    DRV_PTR_VALID_CHECK(p_tcam_ad_tbl);
    DRV_PTR_VALID_CHECK(p_key_share_num);
    DRV_PTR_VALID_CHECK(p_ad_share_num);

    switch (tcam_key_type)
    {
    case DRV_FTM_TCAM_TYPE_IGS_ACL0:
        p_tcam_key_id[idx0++] = DsAclQosForwardKey320Ing0_t;
        p_tcam_key_id[idx0++] = DsAclQosForwardKey640Ing0_t;
        p_tcam_key_id[idx0++] = DsAclQosKey80Ing0_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key320Ing0_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key640Ing0_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key160Ing0_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key320Ing0_t;
        p_tcam_key_id[idx0++] = DsAclQosMacIpv6Key640Ing0_t;
        p_tcam_key_id[idx0++] = DsAclQosMacKey160Ing0_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key320Ing0_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key640Ing0_t;
        p_tcam_key_id[idx0++] = DsAclQosCidKey160Ing0_t;
        p_tcam_key_id[idx0++] = DsAclQosCoppKey320Ing0_t;
        p_tcam_key_id[idx0++] = DsAclQosCoppKey640Ing0_t;
        p_tcam_key_id[idx0++] = DsAclQosUdfKey320Ing0_t;

        p_tcam_ad_tbl[idx1++] = DsAcl0Ingress_t;
        p_tcam_ad_tbl[idx1++] = DsAcl0HalfIngress_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl0ForwardBasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl0ForwardExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl0IpfixEnableTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl0Ipv6BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl0Ipv6ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl0L3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl0L3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl0MacIpv6TcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl0MacL3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl0MacL3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl0MacTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl0SgtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl0CoppBasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl0CoppExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl0UdfTcamAd_t;

        /*- p_tcam_ad_tbl[idx1++] = DsTcam0_t;*/
        break;

    case DRV_FTM_TCAM_TYPE_IGS_ACL1:
        p_tcam_key_id[idx0++] = DsAclQosForwardKey320Ing1_t;
        p_tcam_key_id[idx0++] = DsAclQosForwardKey640Ing1_t;
        p_tcam_key_id[idx0++] = DsAclQosKey80Ing1_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key320Ing1_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key640Ing1_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key160Ing1_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key320Ing1_t;
        p_tcam_key_id[idx0++] = DsAclQosMacIpv6Key640Ing1_t;
        p_tcam_key_id[idx0++] = DsAclQosMacKey160Ing1_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key320Ing1_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key640Ing1_t;
        p_tcam_key_id[idx0++] = DsAclQosCidKey160Ing1_t;
        p_tcam_key_id[idx0++] = DsAclQosCoppKey320Ing1_t;
        p_tcam_key_id[idx0++] = DsAclQosCoppKey640Ing1_t;
        p_tcam_key_id[idx0++] = DsAclQosUdfKey320Ing1_t;

        p_tcam_ad_tbl[idx1++] = DsAcl1Ingress_t;
        p_tcam_ad_tbl[idx1++] = DsAcl1HalfIngress_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl1ForwardBasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl1ForwardExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl1IpfixEnableTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl1Ipv6BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl1Ipv6ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl1L3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl1L3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl1MacIpv6TcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl1MacL3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl1MacL3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl1MacTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl1SgtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl1CoppBasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl1CoppExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl1UdfTcamAd_t;
        break;

    case DRV_FTM_TCAM_TYPE_IGS_ACL2:
        p_tcam_key_id[idx0++] = DsAclQosForwardKey320Ing2_t;
        p_tcam_key_id[idx0++] = DsAclQosForwardKey640Ing2_t;
        p_tcam_key_id[idx0++] = DsAclQosKey80Ing2_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key320Ing2_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key640Ing2_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key160Ing2_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key320Ing2_t;
        p_tcam_key_id[idx0++] = DsAclQosMacIpv6Key640Ing2_t;
        p_tcam_key_id[idx0++] = DsAclQosMacKey160Ing2_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key320Ing2_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key640Ing2_t;
        p_tcam_key_id[idx0++] = DsAclQosCidKey160Ing2_t;
        p_tcam_key_id[idx0++] = DsAclQosCoppKey320Ing2_t;
        p_tcam_key_id[idx0++] = DsAclQosCoppKey640Ing2_t;
        p_tcam_key_id[idx0++] = DsAclQosUdfKey320Ing2_t;

        p_tcam_ad_tbl[idx1++] = DsAcl2Ingress_t;
        p_tcam_ad_tbl[idx1++] = DsAcl2HalfIngress_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl2ForwardBasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl2ForwardExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl2IpfixEnableTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl2Ipv6BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl2Ipv6ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl2L3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl2L3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl2MacIpv6TcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl2MacL3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl2MacL3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl2MacTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl2SgtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl2CoppBasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl2CoppExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl2UdfTcamAd_t;
        break;

    case DRV_FTM_TCAM_TYPE_IGS_ACL3:
        p_tcam_key_id[idx0++] = DsAclQosForwardKey320Ing3_t;
        p_tcam_key_id[idx0++] = DsAclQosForwardKey640Ing3_t;
        p_tcam_key_id[idx0++] = DsAclQosKey80Ing3_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key320Ing3_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key640Ing3_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key160Ing3_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key320Ing3_t;
        p_tcam_key_id[idx0++] = DsAclQosMacIpv6Key640Ing3_t;
        p_tcam_key_id[idx0++] = DsAclQosMacKey160Ing3_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key320Ing3_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key640Ing3_t;
        p_tcam_key_id[idx0++] = DsAclQosCidKey160Ing3_t;
        p_tcam_key_id[idx0++] = DsAclQosCoppKey320Ing3_t;
        p_tcam_key_id[idx0++] = DsAclQosCoppKey640Ing3_t;
        p_tcam_key_id[idx0++] = DsAclQosUdfKey320Ing3_t;

        p_tcam_ad_tbl[idx1++] = DsAcl3Ingress_t;
        p_tcam_ad_tbl[idx1++] = DsAcl3HalfIngress_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl3ForwardBasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl3ForwardExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl3IpfixEnableTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl3Ipv6BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl3Ipv6ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl3L3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl3L3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl3MacIpv6TcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl3MacL3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl3MacL3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl3MacTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl3SgtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl3CoppBasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl3CoppExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl3UdfTcamAd_t;
        break;

    case DRV_FTM_TCAM_TYPE_IGS_ACL4:
        p_tcam_key_id[idx0++] = DsAclQosForwardKey320Ing4_t;
        p_tcam_key_id[idx0++] = DsAclQosForwardKey640Ing4_t;
        p_tcam_key_id[idx0++] = DsAclQosKey80Ing4_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key320Ing4_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key640Ing4_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key160Ing4_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key320Ing4_t;
        p_tcam_key_id[idx0++] = DsAclQosMacIpv6Key640Ing4_t;
        p_tcam_key_id[idx0++] = DsAclQosMacKey160Ing4_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key320Ing4_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key640Ing4_t;
        p_tcam_key_id[idx0++] = DsAclQosCidKey160Ing4_t;
        p_tcam_key_id[idx0++] = DsAclQosCoppKey320Ing4_t;
        p_tcam_key_id[idx0++] = DsAclQosCoppKey640Ing4_t;
        p_tcam_key_id[idx0++] = DsAclQosUdfKey320Ing4_t;

        p_tcam_ad_tbl[idx1++] = DsAcl4Ingress_t;
        p_tcam_ad_tbl[idx1++] = DsAcl4HalfIngress_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl4ForwardBasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl4ForwardExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl4IpfixEnableTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl4Ipv6BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl4Ipv6ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl4L3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl4L3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl4MacIpv6TcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl4MacL3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl4MacL3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl4MacTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl4SgtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl4CoppBasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl4CoppExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl4UdfTcamAd_t;
        break;

    case DRV_FTM_TCAM_TYPE_IGS_ACL5:
        p_tcam_key_id[idx0++] = DsAclQosForwardKey320Ing5_t;
        p_tcam_key_id[idx0++] = DsAclQosForwardKey640Ing5_t;
        p_tcam_key_id[idx0++] = DsAclQosKey80Ing5_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key320Ing5_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key640Ing5_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key160Ing5_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key320Ing5_t;
        p_tcam_key_id[idx0++] = DsAclQosMacIpv6Key640Ing5_t;
        p_tcam_key_id[idx0++] = DsAclQosMacKey160Ing5_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key320Ing5_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key640Ing5_t;
        p_tcam_key_id[idx0++] = DsAclQosCidKey160Ing5_t;
        p_tcam_key_id[idx0++] = DsAclQosCoppKey320Ing5_t;
        p_tcam_key_id[idx0++] = DsAclQosCoppKey640Ing5_t;
        p_tcam_key_id[idx0++] = DsAclQosUdfKey320Ing5_t;

        p_tcam_ad_tbl[idx1++] = DsAcl5Ingress_t;
        p_tcam_ad_tbl[idx1++] = DsAcl5HalfIngress_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl5ForwardBasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl5ForwardExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl5IpfixEnableTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl5Ipv6BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl5Ipv6ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl5L3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl5L3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl5MacIpv6TcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl5MacL3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl5MacL3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl5MacTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl5SgtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl5CoppBasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl5CoppExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl5UdfTcamAd_t;
        break;
    case DRV_FTM_TCAM_TYPE_IGS_ACL6:
        p_tcam_key_id[idx0++] = DsAclQosForwardKey320Ing6_t;
        p_tcam_key_id[idx0++] = DsAclQosForwardKey640Ing6_t;
        p_tcam_key_id[idx0++] = DsAclQosKey80Ing6_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key320Ing6_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key640Ing6_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key160Ing6_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key320Ing6_t;
        p_tcam_key_id[idx0++] = DsAclQosMacIpv6Key640Ing6_t;
        p_tcam_key_id[idx0++] = DsAclQosMacKey160Ing6_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key320Ing6_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key640Ing6_t;
        p_tcam_key_id[idx0++] = DsAclQosCidKey160Ing6_t;
        p_tcam_key_id[idx0++] = DsAclQosCoppKey320Ing6_t;
        p_tcam_key_id[idx0++] = DsAclQosCoppKey640Ing6_t;
        p_tcam_key_id[idx0++] = DsAclQosUdfKey320Ing6_t;

        p_tcam_ad_tbl[idx1++] = DsAcl6Ingress_t;
        p_tcam_ad_tbl[idx1++] = DsAcl6HalfIngress_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl6ForwardBasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl6ForwardExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl6IpfixEnableTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl6Ipv6BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl6Ipv6ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl6L3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl6L3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl6MacIpv6TcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl6MacL3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl6MacL3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl6MacTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl6SgtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl6CoppBasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl6CoppExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl6UdfTcamAd_t;
        break;
    case DRV_FTM_TCAM_TYPE_IGS_ACL7:
        p_tcam_key_id[idx0++] = DsAclQosForwardKey320Ing7_t;
        p_tcam_key_id[idx0++] = DsAclQosForwardKey640Ing7_t;
        p_tcam_key_id[idx0++] = DsAclQosKey80Ing7_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key320Ing7_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key640Ing7_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key160Ing7_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key320Ing7_t;
        p_tcam_key_id[idx0++] = DsAclQosMacIpv6Key640Ing7_t;
        p_tcam_key_id[idx0++] = DsAclQosMacKey160Ing7_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key320Ing7_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key640Ing7_t;
        p_tcam_key_id[idx0++] = DsAclQosCidKey160Ing7_t;
        p_tcam_key_id[idx0++] = DsAclQosCoppKey320Ing7_t;
        p_tcam_key_id[idx0++] = DsAclQosCoppKey640Ing7_t;
        p_tcam_key_id[idx0++] = DsAclQosUdfKey320Ing7_t;

        p_tcam_ad_tbl[idx1++] = DsAcl7Ingress_t;
        p_tcam_ad_tbl[idx1++] = DsAcl7HalfIngress_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl7ForwardBasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl7ForwardExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl7IpfixEnableTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl7Ipv6BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl7Ipv6ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl7L3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl7L3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl7MacIpv6TcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl7MacL3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl7MacL3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl7MacTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl7SgtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl7CoppBasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl7CoppExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsIngAcl7UdfTcamAd_t;

        break;

    case DRV_FTM_TCAM_TYPE_EGS_ACL0:
        p_tcam_key_id[idx0++] = DsAclQosKey80Egr0_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key320Egr0_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key640Egr0_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key160Egr0_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key320Egr0_t;
        p_tcam_key_id[idx0++] = DsAclQosMacIpv6Key640Egr0_t;
        p_tcam_key_id[idx0++] = DsAclQosMacKey160Egr0_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key320Egr0_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key640Egr0_t;
        p_tcam_key_id[idx0++] = DsAclQosUdfKey320Egr0_t;

        p_tcam_ad_tbl[idx1++] = DsAcl0Egress_t;
        p_tcam_ad_tbl[idx1++] = DsAcl0HalfEgress_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl0IpfixEnableTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl0Ipv6BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl0Ipv6ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl0L3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl0L3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl0MacIpv6TcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl0MacTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl0MacL3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl0MacL3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl0UdfTcamAd_t;
        break;
    case DRV_FTM_TCAM_TYPE_EGS_ACL1:
        p_tcam_key_id[idx0++] = DsAclQosKey80Egr1_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key320Egr1_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key640Egr1_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key160Egr1_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key320Egr1_t;
        p_tcam_key_id[idx0++] = DsAclQosMacIpv6Key640Egr1_t;
        p_tcam_key_id[idx0++] = DsAclQosMacKey160Egr1_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key320Egr1_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key640Egr1_t;
        p_tcam_key_id[idx0++] = DsAclQosUdfKey320Egr1_t;

        p_tcam_ad_tbl[idx1++] = DsAcl1Egress_t;
        p_tcam_ad_tbl[idx1++] = DsAcl1HalfEgress_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl1IpfixEnableTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl1Ipv6BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl1Ipv6ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl1L3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl1L3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl1MacIpv6TcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl1MacTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl1MacL3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl1MacL3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl1UdfTcamAd_t;
        break;
    case DRV_FTM_TCAM_TYPE_EGS_ACL2:
        p_tcam_key_id[idx0++] = DsAclQosKey80Egr2_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key320Egr2_t;
        p_tcam_key_id[idx0++] = DsAclQosIpv6Key640Egr2_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key160Egr2_t;
        p_tcam_key_id[idx0++] = DsAclQosL3Key320Egr2_t;
        p_tcam_key_id[idx0++] = DsAclQosMacIpv6Key640Egr2_t;
        p_tcam_key_id[idx0++] = DsAclQosMacKey160Egr2_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key320Egr2_t;
        p_tcam_key_id[idx0++] = DsAclQosMacL3Key640Egr2_t;
        p_tcam_key_id[idx0++] = DsAclQosUdfKey320Egr2_t;

        p_tcam_ad_tbl[idx1++] = DsAcl2Egress_t;
        p_tcam_ad_tbl[idx1++] = DsAcl2HalfEgress_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl2IpfixEnableTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl2Ipv6BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl2Ipv6ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl2L3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl2L3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl2MacIpv6TcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl2MacTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl2MacL3BasicTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl2MacL3ExtTcamAd_t;
        p_tcam_ad_tbl[idx1++] = DsEgrAcl2UdfTcamAd_t;
        break;


    case DRV_FTM_TCAM_TYPE_IGS_USERID0:
        p_tcam_key_id[idx0++] = DsUserId0TcamKey80_t;
        p_tcam_key_id[idx0++] = DsScl0MacKey160_t;
        p_tcam_key_id[idx0++] = DsScl0L3Key160_t;
        p_tcam_key_id[idx0++] = DsScl0Ipv6Key320_t;
        p_tcam_key_id[idx0++] = DsScl0MacL3Key320_t;
        p_tcam_key_id[idx0++] = DsScl0MacIpv6Key640_t;
        p_tcam_key_id[idx0++] = DsUserId0TcamKey160_t;
        p_tcam_key_id[idx0++] = DsUserId0TcamKey320_t;
        p_tcam_key_id[idx0++] = DsUserId0TcamKey640_t;
        p_tcam_key_id[idx0++] = DsScl0UdfKey160_t;
        p_tcam_key_id[idx0++] = DsScl0UdfKey320_t;

        p_tcam_ad_tbl[idx1++] = DsUserId0Tcam_t;
        p_tcam_ad_tbl[idx1++] = DsTunnelId0Tcam_t;
        p_tcam_ad_tbl[idx1++] = DsSclFlow0Tcam_t;
        p_tcam_ad_tbl[idx1++] = DsUserIdTcam160Ad0_t;
         /*-p_tcam_ad_tbl[idx1++] = DsUserId0Mac160TcamAd_t;*/
        p_tcam_ad_tbl[idx1++] = DsUserIdTcam320Ad0_t;
         /*-p_tcam_ad_tbl[idx1++] = DsUserIdTcam640Ad0_t;*/
        p_tcam_ad_tbl[idx1++] = DsMpls0Tcam_t;
        break;

    case DRV_FTM_TCAM_TYPE_IGS_USERID1:
        p_tcam_key_id[idx0++] = DsUserId1TcamKey80_t;
        p_tcam_key_id[idx0++] = DsScl1MacKey160_t;
        p_tcam_key_id[idx0++] = DsScl1L3Key160_t;
        p_tcam_key_id[idx0++] = DsScl1Ipv6Key320_t;
        p_tcam_key_id[idx0++] = DsScl1MacL3Key320_t;
        p_tcam_key_id[idx0++] = DsScl1MacIpv6Key640_t;
        p_tcam_key_id[idx0++] = DsUserId1TcamKey160_t;
        p_tcam_key_id[idx0++] = DsUserId1TcamKey320_t;
        p_tcam_key_id[idx0++] = DsUserId1TcamKey640_t;
        p_tcam_key_id[idx0++] = DsScl1UdfKey160_t;
        p_tcam_key_id[idx0++] = DsScl1UdfKey320_t;

        p_tcam_ad_tbl[idx1++] = DsUserId1Tcam_t;
        p_tcam_ad_tbl[idx1++] = DsTunnelId1Tcam_t;
        p_tcam_ad_tbl[idx1++] = DsSclFlow1Tcam_t;
        p_tcam_ad_tbl[idx1++] = DsUserIdTcam160Ad1_t;
         /*-p_tcam_ad_tbl[idx1++] = DsUserId1Mac160TcamAd_t;*/
        p_tcam_ad_tbl[idx1++] = DsUserIdTcam320Ad1_t;
         /*-p_tcam_ad_tbl[idx1++] = DsUserIdTcam640Ad1_t;*/
        p_tcam_ad_tbl[idx1++] = DsMpls1Tcam_t;
        break;


    case DRV_FTM_TCAM_TYPE_IGS_USERID2:
        p_tcam_key_id[idx0++] = DsScl2MacKey160_t;
        p_tcam_key_id[idx0++] = DsScl2L3Key160_t;
        p_tcam_key_id[idx0++] = DsScl2Ipv6Key320_t;
        p_tcam_key_id[idx0++] = DsScl2MacL3Key320_t            ;
        p_tcam_key_id[idx0++] = DsScl2MacIpv6Key640_t;
        p_tcam_key_id[idx0++] = DsScl2UdfKey160_t;
        p_tcam_key_id[idx0++] = DsScl2UdfKey320_t;

        p_tcam_ad_tbl[idx1++] = DsUserId2_t;
        p_tcam_ad_tbl[idx1++] = DsSclFlow2Tcam_t;

        break;


    case DRV_FTM_TCAM_TYPE_IGS_USERID3:
        p_tcam_key_id[idx0++] = DsScl3MacKey160_t;
        p_tcam_key_id[idx0++] = DsScl3L3Key160_t;
        p_tcam_key_id[idx0++] = DsScl3Ipv6Key320_t;
        p_tcam_key_id[idx0++] = DsScl3MacL3Key320_t            ;
        p_tcam_key_id[idx0++] = DsScl3MacIpv6Key640_t;
        p_tcam_key_id[idx0++] = DsScl3UdfKey160_t;
        p_tcam_key_id[idx0++] = DsScl3UdfKey320_t;

        p_tcam_ad_tbl[idx1++] = DsUserId3_t;
        p_tcam_ad_tbl[idx1++] = DsSclFlow3Tcam_t;
        break;


    case DRV_FTM_TCAM_TYPE_IGS_LPM_ALL: /*just for cmodel*/
    #if 0
        p_tcam_key_id[idx0++] = DsLpmTcamIpv4HalfKey_t;
        p_tcam_key_id[idx0++] = DsLpmTcamIpv6DoubleKey0_t;
        p_tcam_key_id[idx0++] = DsLpmTcamIpv6QuadKey_t;
        p_tcam_key_id[idx0++] = DsLpmTcamIpv6DoubleKey0_t;
        p_tcam_key_id[idx0++] = DsLpmTcamIpv6DoubleKey1_t;
         /*p_tcam_key_id[idx0++] = DsLpmTcamIpv4DoubleKey_t;*/
        p_tcam_key_id[idx0++] = DsLpmTcamIpv6SingleKey_t;
        p_tcam_key_id[idx0++] = DsLpmTcamIpv4SaHalfKey_t;
        p_tcam_key_id[idx0++] = DsLpmTcamIpv6SaSingleKey_t;
        p_tcam_key_id[idx0++] = DsLpmTcamIpv6SaDoubleKey0_t;



        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv6SingleKeyAd_t;
        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv4HalfKeyAd_t;
        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv6DoubleKey0Ad_t;
        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv4SaHalfKeyAd_t;
        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv6SaSingleKeyAd_t;
        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv6SaDoubleKey0Ad_t;
         /*p_tcam_ad_tbl[idx1++] = DsLpmTcamAd_t;*/
     #endif
        break;
    case DRV_FTM_TCAM_TYPE_IGS_LPM0DA:
        p_tcam_key_id[idx0++] = DsLpmTcamIpv4HalfKey_t;
        p_tcam_key_id[idx0++] = DsLpmTcamIpv6DoubleKey0_t;

        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv6DoubleKey0Ad_t;
        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv4HalfKeyAd_t;

       if (g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][1])
        {
            p_tcam_key_id[idx0++] = DsLpmTcamIpv6SingleKey_t;
            p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv6SingleKeyAd_t;
        }

        break;

    case DRV_FTM_TCAM_TYPE_IGS_LPM0SA:
        p_tcam_key_id[idx0++] = DsLpmTcamIpv4SaHalfKey_t;
        p_tcam_key_id[idx0++] = DsLpmTcamIpv6SaDoubleKey0_t;
        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv4SaHalfKeyAd_t;
        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv6SaDoubleKey0Ad_t;
        if(g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][1])
        {
            p_tcam_key_id[idx0++] = DsLpmTcamIpv6SaSingleKey_t;
            p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv6SaSingleKeyAd_t;
        }
        break;

    case DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB:
        p_tcam_key_id[idx0++] = DsLpmTcamIpv4DaPubHalfKey_t;
        p_tcam_key_id[idx0++] = DsLpmTcamIpv6DaPubDoubleKey0_t;

        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv4DaPubHalfKeyAd_t;
        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv6DaPubDoubleKey0Ad_t;
        if(g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][1])
        {
            p_tcam_key_id[idx0++] = DsLpmTcamIpv6DaPubSingleKey_t;
            p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv6DaPubSingleKeyAd_t;
        }
        p_tcam_ad_tbl[idx1++] =  DsLpmTcamIpv4DaPubRouteKeyAd_t;
        break;

    case DRV_FTM_TCAM_TYPE_IGS_LPM0SAPUB:
        p_tcam_key_id[idx0++] = DsLpmTcamIpv4SaPubHalfKey_t;
        p_tcam_key_id[idx0++] = DsLpmTcamIpv6SaPubDoubleKey0_t;

        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv4SaPubHalfKeyAd_t;
        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv6SaPubDoubleKey0Ad_t;
        if(g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][1])
        {
            p_tcam_key_id[idx0++] = DsLpmTcamIpv6SaPubSingleKey_t;
            p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv6SaPubSingleKeyAd_t;
        }
        p_tcam_ad_tbl[idx1++] =  DsLpmTcamIpv4SaPubRouteKeyAd_t;
        break;
#if 0
    /*single key*/
    case DRV_FTM_TCAM_TYPE_IGS_LPM0DA_V6_HALF:
        p_tcam_key_id[idx0++] = DsLpmTcamIpv6SingleKeyLookup1_t;

        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv6SingleKeyAdLookup1_t;
        p_tcam_key_id[idx0++] = DsLpmTcamIpv6SingleKey_t;
        break;

    case DRV_FTM_TCAM_TYPE_IGS_LPM0SA_V6_HALF:
        p_tcam_key_id[idx0++] = DsLpmTcamIpv6SaSingleKeyLookup1_t;

        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv6SaSingleKeyAdLookup1_t;
        break;

    case DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB_V6_HALF:
        p_tcam_key_id[idx0++] = DsLpmTcamIpv6DaPubSingleKey_t;

        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv6DaPubSingleKeyAd_t;
        break;
    case DRV_FTM_TCAM_TYPE_IGS_LPM0SAPUB_V6_HALF:
        p_tcam_key_id[idx0++] = DsLpmTcamIpv6SaPubSingleKey_t;

        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv6SaPubSingleKeyAd_t;
        break;
#endif
    case DRV_FTM_TCAM_TYPE_IGS_LPM1:
        p_tcam_key_id[idx0++] = DsLpmTcamIpv4NatDoubleKey_t;
        p_tcam_key_id[idx0++] = DsLpmTcamIpv4DoubleKey_t;

        p_tcam_ad_tbl[idx1++] = DsLpmTcamIpv4NatDoubleKeyAd_t;
        p_tcam_ad_tbl[idx1++] = DsLpmTcamAd_t;   /*special for DsLpmTcamIpv4DoubleKey_t */
        break;

    case DRV_FTM_TCAM_TYPE_STATIC_TCAM:
        p_tcam_key_id[idx0++] = DsCategoryIdPairTcamKey_t;
        p_tcam_key_id[idx0++] = DsQueueMapTcamKey_t;
        break;

    default:
       break;
    }

    *p_key_share_num = idx0;
    *p_ad_share_num  = idx1;
    return DRV_E_NONE;
}
#if 0
uint32
drv_usw_ftm_alloc_tcam_key_map_szietype(uint8 lchip, uint32 tbl_id)
{
    /* Lpm NAT 表项都是从TCAM中切割出来的,这里根据TABLEID得到切割的数组的索引 */
    uint8 new_type = 0;
    switch (tbl_id)
    {
        case DsLpmTcamIpv4DoubleKey_t:
            new_type = DRV_FTM_TCAM_SIZE_ipv4_nat;
            break;
        case DsLpmTcamIpv4PbrDoubleKey_t:
            new_type = DRV_FTM_TCAM_SIZE_ipv4_pbr;
            break;
        case DsLpmTcamIpv4NatDoubleKey_t:
            new_type = DRV_FTM_TCAM_SIZE_ipv4_nat;
            break;
        case DsLpmTcamIpv6QuadKey_t:
            new_type = DRV_FTM_TCAM_SIZE_ipv6_pbr;
            break;
        case DsLpmTcamIpv6DoubleKey1_t:
            new_type = DRV_FTM_TCAM_SIZE_ipv6_nat;
            break;

        case DsLpmTcamIpv4HalfKeyLookup2_t:
        case DsLpmTcamIpv4HalfKeyLookup1_t:                /* for v4 lpm + tcam + prefix */
        case DsLpmTcamIpv4SaHalfKeyLookup2_t:
        case DsLpmTcamIpv4SaHalfKeyLookup1_t:
        case DsLpmTcamIpv4DaPubHalfKeyLookup2_t:
        case DsLpmTcamIpv4DaPubHalfKeyLookup1_t:
        case DsLpmTcamIpv4SaPubHalfKeyLookup2_t:
        case DsLpmTcamIpv4SaPubHalfKeyLookup1_t:
            new_type = DRV_FTM_TCAM_SIZE_halfkey;
            break;

        case DsLpmTcamIpv6DoubleKey0Lookup2_t:             /* for ipv6 tcam + chip bug */
        case DsLpmTcamIpv6DoubleKey0Lookup1_t:
        case DsLpmTcamIpv6SaDoubleKey0Lookup2_t:
        case DsLpmTcamIpv6SaDoubleKey0Lookup1_t:
        case DsLpmTcamIpv6DaPubDoubleKey0Lookup2_t:
        case DsLpmTcamIpv6DaPubDoubleKey0Lookup1_t:
        case DsLpmTcamIpv6SaPubDoubleKey0Lookup2_t:
        case DsLpmTcamIpv6SaPubDoubleKey0Lookup1_t:
            new_type = DRV_FTM_TCAM_SIZE_doublekey;
            break;

        case DsLpmTcamIpv6SingleKey_t:              /* for ipv6 prefix only */
        case DsLpmTcamIpv6SaSingleKey_t:
        case DsLpmTcamIpv6DaPubSingleKey_t:
        case DsLpmTcamIpv6SaPubSingleKey_t:
            new_type = DRV_FTM_TCAM_SIZE_singlekey;
            break;
        default:
            break;
    }
    return new_type;
}

#endif
/*
 * 由于TCAM_AD的大小固定的,但是KEY是不同的,需要修正下AD的大小,来产生偏移修正
 */
STATIC void
drv_usw_ftm_alloc_tcam_ad_map_to_new_word(uint8 lchip, uint32 tbl_id)
{
#define CASE_ACL_ING_AD_FIXSIZE_2_KEYSIZE_BREAK(ad,key)         \
    case DsIngAcl0##ad##TcamAd_t:                               \
    case DsIngAcl1##ad##TcamAd_t:                               \
    case DsIngAcl2##ad##TcamAd_t:                               \
    case DsIngAcl3##ad##TcamAd_t:                               \
    case DsIngAcl4##ad##TcamAd_t:                               \
    case DsIngAcl5##ad##TcamAd_t:                               \
    case DsIngAcl6##ad##TcamAd_t:                               \
    case DsIngAcl7##ad##TcamAd_t:                               \
        TCAM_KEY_SIZE(lchip, tbl_id) =                              \
            TABLE_ENTRY_SIZE(lchip, DsAclQos##key##Ing0_t);            \
        break;

#define CASE_ACL_EGR_AD_FIXSIZE_2_KEYSIZE_BREAK(ad,key)         \
    case DsEgrAcl0##ad##TcamAd_t:                               \
    case DsEgrAcl1##ad##TcamAd_t:                               \
    case DsEgrAcl2##ad##TcamAd_t:                               \
        TCAM_KEY_SIZE(lchip, tbl_id) =                              \
            TABLE_ENTRY_SIZE(lchip, DsAclQos##key##Egr0_t);            \
        break;

    switch (tbl_id)
    {

        case DsLpmTcamIpv4HalfKeyAd_t:
        case DsLpmTcamIpv4HalfKeyAdLookup1_t:
        case DsLpmTcamIpv4SaHalfKeyAdLookup1_t:
        case DsLpmTcamIpv4DaPubHalfKeyAdLookup1_t:
        case DsLpmTcamIpv4SaPubHalfKeyAdLookup1_t:
        case DsLpmTcamIpv4HalfKeyAdLookup2_t:
        case DsLpmTcamIpv4SaHalfKeyAdLookup2_t:
        case DsLpmTcamIpv4DaPubHalfKeyAdLookup2_t:
            TCAM_KEY_SIZE(lchip, tbl_id) =
                TABLE_ENTRY_SIZE(lchip, DsLpmTcamIpv4HalfKeyLookup1_t);
           break;

        case DsLpmTcamIpv6SaPubDoubleKey0Ad_t:
        case DsLpmTcamIpv6SaPubDoubleKey0AdLookup1_t:
        case DsLpmTcamIpv6DaPubDoubleKey0Ad_t:
        case DsLpmTcamIpv6DaPubDoubleKey0AdLookup1_t:
        case DsLpmTcamIpv6SaDoubleKey0Ad_t:
        case DsLpmTcamIpv6SaDoubleKey0AdLookup1_t:
        case DsLpmTcamIpv6DoubleKey0Ad_t:
        case DsLpmTcamIpv6DoubleKey0AdLookup1_t:
        case DsLpmTcamIpv6SaPubDoubleKey0AdLookup2_t:
        case DsLpmTcamIpv6DaPubDoubleKey0AdLookup2_t:
        case DsLpmTcamIpv6SaDoubleKey0AdLookup2_t:
        case DsLpmTcamIpv6DoubleKey0AdLookup2_t:
            TCAM_KEY_SIZE(lchip, tbl_id) =
                TABLE_ENTRY_SIZE(lchip, DsLpmTcamIpv6DoubleKey0Lookup1_t);
            break;

        case DsLpmTcamIpv6SingleKeyAd_t:
        case DsLpmTcamIpv6SaSingleKeyAd_t:
        case DsLpmTcamIpv6DaPubSingleKeyAd_t:
        case DsLpmTcamIpv6SaPubSingleKeyAd_t:
        case DsLpmTcamIpv4SaPubHalfKeyAdLookup2_t:
            TCAM_KEY_SIZE(lchip, tbl_id) =
                TABLE_ENTRY_SIZE(lchip, DsLpmTcamIpv6SingleKey_t);
            break;

        case DsLpmTcamAd_t:
            TCAM_KEY_SIZE(lchip, tbl_id) =
                TABLE_ENTRY_SIZE(lchip, DsLpmTcamIpv4NatDoubleKey_t);
            break;
        case DsLpmTcamIpv4NatDoubleKeyAd_t:
            TCAM_KEY_SIZE(lchip, tbl_id) =
                TABLE_ENTRY_SIZE(lchip, DsLpmTcamIpv4NatDoubleKey_t);
            break;

        case DsLpmTcamIpv4PbrDoubleKeyAd_t:
            TCAM_KEY_SIZE(lchip, tbl_id) =
                TABLE_ENTRY_SIZE(lchip, DsLpmTcamIpv4PbrDoubleKey_t);
            break;

        case DsLpmTcamIpv6NatDoubleKeyAd_t:
            TCAM_KEY_SIZE(lchip, tbl_id) =
                TABLE_ENTRY_SIZE(lchip, DsLpmTcamIpv6DoubleKey1_t);
            break;

        case DsLpmTcamIpv6PbrQuadKeyAd_t:
            TCAM_KEY_SIZE(lchip, tbl_id) =
                TABLE_ENTRY_SIZE(lchip, DsLpmTcamIpv6QuadKey_t);
            break;

        CASE_ACL_ING_AD_FIXSIZE_2_KEYSIZE_BREAK(ForwardExt,ForwardKey640)
        CASE_ACL_ING_AD_FIXSIZE_2_KEYSIZE_BREAK(ForwardBasic,ForwardKey320)
        CASE_ACL_ING_AD_FIXSIZE_2_KEYSIZE_BREAK(IpfixEnable,Key80)
        CASE_ACL_ING_AD_FIXSIZE_2_KEYSIZE_BREAK(Ipv6Basic,Ipv6Key320)
        CASE_ACL_ING_AD_FIXSIZE_2_KEYSIZE_BREAK(Ipv6Ext,Ipv6Key640)
        CASE_ACL_ING_AD_FIXSIZE_2_KEYSIZE_BREAK(L3Basic,L3Key160)
        CASE_ACL_ING_AD_FIXSIZE_2_KEYSIZE_BREAK(L3Ext,L3Key320)
        CASE_ACL_ING_AD_FIXSIZE_2_KEYSIZE_BREAK(MacIpv6,MacIpv6Key640)
        CASE_ACL_ING_AD_FIXSIZE_2_KEYSIZE_BREAK(MacL3Basic,MacL3Key320)
        CASE_ACL_ING_AD_FIXSIZE_2_KEYSIZE_BREAK(MacL3Ext,MacL3Key640)
        CASE_ACL_ING_AD_FIXSIZE_2_KEYSIZE_BREAK(Mac,MacKey160)
        CASE_ACL_ING_AD_FIXSIZE_2_KEYSIZE_BREAK(Sgt,CidKey160)

        CASE_ACL_EGR_AD_FIXSIZE_2_KEYSIZE_BREAK(IpfixEnable,Key80)
        CASE_ACL_EGR_AD_FIXSIZE_2_KEYSIZE_BREAK(Ipv6Basic,Ipv6Key320)
        CASE_ACL_EGR_AD_FIXSIZE_2_KEYSIZE_BREAK(Ipv6Ext,Ipv6Key640)
        CASE_ACL_EGR_AD_FIXSIZE_2_KEYSIZE_BREAK(L3Basic,L3Key160)
        CASE_ACL_EGR_AD_FIXSIZE_2_KEYSIZE_BREAK(L3Ext,L3Key320)
        CASE_ACL_EGR_AD_FIXSIZE_2_KEYSIZE_BREAK(MacIpv6,MacIpv6Key640)
        CASE_ACL_EGR_AD_FIXSIZE_2_KEYSIZE_BREAK(Mac,MacKey160)
        CASE_ACL_EGR_AD_FIXSIZE_2_KEYSIZE_BREAK(MacL3Basic,MacL3Key320)
        CASE_ACL_EGR_AD_FIXSIZE_2_KEYSIZE_BREAK(MacL3Ext,MacL3Key640)
        default:
            break;
    }
#undef CASE_ACL_ING_AD_FIXSIZE_2_KEYSIZE_BREAK
#undef CASE_ACL_EGR_AD_FIXSIZE_2_KEYSIZE_BREAK
}
#if 0
STATIC int32
drv_usw_ftm_get_tcam_ad_by_size_type(uint8 tcam_key_type,
                                        uint8 size_type,
                                        uint32 *p_ad_share_tbl,
                                        uint8* p_ad_share_num)
{
#define DRV_FTM_TCAM_ACL_ING_AD_MAP(acl_id)                 \
        {                                                   \
            if (DRV_FTM_TCAM_SIZE_80 == size_type)          \
            {                                               \
                p_ad_share_tbl[idx0++] =                    \
                    DsIngAcl##acl_id##SgtTcamAd_t;          \
                p_ad_share_tbl[idx0++] =                    \
                    DsIngAcl##acl_id##IpfixEnableTcamAd_t;  \
            }                                               \
            else if (DRV_FTM_TCAM_SIZE_160 == size_type)    \
            {                                               \
                p_ad_share_tbl[idx0++] =                    \
                    DsIngAcl##acl_id##L3BasicTcamAd_t;      \
                p_ad_share_tbl[idx0++] =                    \
                    DsIngAcl##acl_id##MacTcamAd_t;          \
            }                                               \
            else if(DRV_FTM_TCAM_SIZE_320 == size_type)     \
            {                                               \
                p_ad_share_tbl[idx0++] =                    \
                    DsIngAcl##acl_id##Ipv6BasicTcamAd_t;    \
                p_ad_share_tbl[idx0++] =                    \
                    DsIngAcl##acl_id##L3ExtTcamAd_t;        \
                p_ad_share_tbl[idx0++] =                    \
                    DsIngAcl##acl_id##MacL3BasicTcamAd_t;   \
                p_ad_share_tbl[idx0++] =                    \
                    DsIngAcl##acl_id##ForwardBasicTcamAd_t;    \
            }                                               \
            else if(DRV_FTM_TCAM_SIZE_640 == size_type)     \
            {                                               \
                p_ad_share_tbl[idx0++] =                    \
                    DsIngAcl##acl_id##ForwardExtTcamAd_t;  \
                p_ad_share_tbl[idx0++] =                    \
                    DsIngAcl##acl_id##Ipv6ExtTcamAd_t;      \
                p_ad_share_tbl[idx0++] =                    \
                    DsIngAcl##acl_id##MacIpv6TcamAd_t;      \
                p_ad_share_tbl[idx0++] =                    \
                    DsIngAcl##acl_id##MacL3ExtTcamAd_t;     \
            }                                               \
       }

#define DRV_FTM_TCAM_ACL_EGR_AD_MAP(acl_id)                 \
        {                                                   \
            if (DRV_FTM_TCAM_SIZE_80 == size_type)          \
            {                                               \
                p_ad_share_tbl[idx0++] =                    \
                    DsEgrAcl##acl_id##IpfixEnableTcamAd_t;  \
            }                                               \
            else if (DRV_FTM_TCAM_SIZE_160 == size_type)    \
            {                                               \
                p_ad_share_tbl[idx0++] =                    \
                    DsEgrAcl##acl_id##L3BasicTcamAd_t;      \
                p_ad_share_tbl[idx0++] =                    \
                    DsEgrAcl##acl_id##MacTcamAd_t;          \
            }                                               \
            else if(DRV_FTM_TCAM_SIZE_320 == size_type)     \
            {                                               \
                p_ad_share_tbl[idx0++] =                    \
                    DsEgrAcl##acl_id##Ipv6BasicTcamAd_t;    \
                p_ad_share_tbl[idx0++] =                    \
                    DsEgrAcl##acl_id##L3ExtTcamAd_t;        \
                p_ad_share_tbl[idx0++] =                    \
                    DsEgrAcl##acl_id##MacL3BasicTcamAd_t;   \
            }                                               \
            else if(DRV_FTM_TCAM_SIZE_640 == size_type)     \
            {                                               \
                p_ad_share_tbl[idx0++] =                    \
                    DsEgrAcl##acl_id##Ipv6ExtTcamAd_t;      \
                p_ad_share_tbl[idx0++] =                    \
                    DsEgrAcl##acl_id##MacIpv6TcamAd_t;      \
                p_ad_share_tbl[idx0++] =                    \
                    DsEgrAcl##acl_id##MacL3ExtTcamAd_t;     \
            }                                               \
       }

    uint8 idx0 = 0;

    DRV_PTR_VALID_CHECK(p_ad_share_tbl);
    DRV_PTR_VALID_CHECK(p_ad_share_num);

    switch (tcam_key_type)
    {

    case DRV_FTM_TCAM_TYPE_IGS_ACL0:
        DRV_FTM_TCAM_ACL_ING_AD_MAP(0);
        break;

    case DRV_FTM_TCAM_TYPE_IGS_ACL1:
        DRV_FTM_TCAM_ACL_ING_AD_MAP(1);
        break;

    case DRV_FTM_TCAM_TYPE_IGS_ACL2:
        DRV_FTM_TCAM_ACL_ING_AD_MAP(2);
        break;

    case DRV_FTM_TCAM_TYPE_IGS_ACL3:
        DRV_FTM_TCAM_ACL_ING_AD_MAP(3);
        break;

    case DRV_FTM_TCAM_TYPE_IGS_ACL4:
        DRV_FTM_TCAM_ACL_ING_AD_MAP(4);
        break;

    case DRV_FTM_TCAM_TYPE_IGS_ACL5:
        DRV_FTM_TCAM_ACL_ING_AD_MAP(5);
        break;

    case DRV_FTM_TCAM_TYPE_IGS_ACL6:
        DRV_FTM_TCAM_ACL_ING_AD_MAP(6);
        break;

    case DRV_FTM_TCAM_TYPE_IGS_ACL7:
        DRV_FTM_TCAM_ACL_ING_AD_MAP(7);
        break;

    case DRV_FTM_TCAM_TYPE_EGS_ACL0:
        DRV_FTM_TCAM_ACL_EGR_AD_MAP(0);
        break;

    case DRV_FTM_TCAM_TYPE_EGS_ACL1:
        DRV_FTM_TCAM_ACL_EGR_AD_MAP(1);
        break;

    case DRV_FTM_TCAM_TYPE_EGS_ACL2:
        DRV_FTM_TCAM_ACL_EGR_AD_MAP(2);
        break;

    case DRV_FTM_TCAM_TYPE_IGS_USERID0:
        if (DRV_FTM_TCAM_SIZE_160 == size_type)
        {
            p_ad_share_tbl[idx0++] = DsUserIdTcam160Ad0_t;
            /*- p_ad_share_tbl[idx0++] = DsUserId0Mac160TcamAd_t;*/
        }
        else if(DRV_FTM_TCAM_SIZE_320 == size_type)
        {
            p_ad_share_tbl[idx0++] = DsUserIdTcam320Ad0_t;
        }
        else if(DRV_FTM_TCAM_SIZE_640 == size_type)
        {
            p_ad_share_tbl[idx0++] = DsUserIdTcam320Ad0_t;
        }
        break;

    case DRV_FTM_TCAM_TYPE_IGS_USERID1:
        if (DRV_FTM_TCAM_SIZE_160 == size_type)
        {
            p_ad_share_tbl[idx0++] = DsUserIdTcam160Ad1_t;
             /*-p_ad_share_tbl[idx0++] = DsUserId1Mac160TcamAd_t;*/
        }
        else if(DRV_FTM_TCAM_SIZE_320 == size_type)
        {
            p_ad_share_tbl[idx0++] = DsUserIdTcam320Ad1_t;
        }
        else if(DRV_FTM_TCAM_SIZE_640 == size_type)
        {
            p_ad_share_tbl[idx0++] = DsUserIdTcam320Ad1_t;
        }
        break;
    case DRV_FTM_TCAM_TYPE_IGS_LPM0:
        if (DRV_FTM_TCAM_SIZE_halfkey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv4HalfKeyAdLookup1_t;
        }
        else if(DRV_FTM_TCAM_SIZE_doublekey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6DoubleKey0AdLookup1_t;
        }
        else if(DRV_FTM_TCAM_SIZE_singlekey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6SingleKeyAd_t;
        }
        break;
    case DRV_FTM_TCAM_TYPE_IGS_LPM0SAPRI:
        if (DRV_FTM_TCAM_SIZE_halfkey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv4SaHalfKeyAdLookup1_t;
        }
        else if(DRV_FTM_TCAM_SIZE_doublekey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6SaDoubleKey0AdLookup1_t;
        }
        else if(DRV_FTM_TCAM_SIZE_singlekey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6SaSingleKeyAd_t;
        }
        break;
    case DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB:
        if (DRV_FTM_TCAM_SIZE_halfkey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv4DaPubHalfKeyAdLookup1_t;
        }
        else if(DRV_FTM_TCAM_SIZE_doublekey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6DaPubDoubleKey0AdLookup1_t;
        }
        else if(DRV_FTM_TCAM_SIZE_singlekey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6DaPubSingleKeyAd_t;
        }
        break;
    case DRV_FTM_TCAM_TYPE_IGS_LPM0SAPUB:
        if (DRV_FTM_TCAM_SIZE_halfkey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv4SaPubHalfKeyAdLookup1_t;
        }
        else if(DRV_FTM_TCAM_SIZE_doublekey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6SaPubDoubleKey0AdLookup1_t;
        }
        else if(DRV_FTM_TCAM_SIZE_singlekey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6SaPubSingleKeyAd_t;
        }
        break;
    case DRV_FTM_TCAM_TYPE_IGS_LPM0_LK2:
        if (DRV_FTM_TCAM_SIZE_halfkey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv4HalfKeyAdLookup2_t;
        }
        else if(DRV_FTM_TCAM_SIZE_doublekey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6DoubleKey0AdLookup2_t;
        }
        break;
    case DRV_FTM_TCAM_TYPE_IGS_LPM0SAPRI_LK2:
        if (DRV_FTM_TCAM_SIZE_halfkey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv4SaHalfKeyAdLookup2_t;
        }
        else if(DRV_FTM_TCAM_SIZE_doublekey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6SaDoubleKey0AdLookup2_t;
        }
        break;
    case DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB_LK2:
        if (DRV_FTM_TCAM_SIZE_halfkey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv4DaPubHalfKeyAdLookup2_t;
        }
        else if(DRV_FTM_TCAM_SIZE_doublekey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6DaPubDoubleKey0AdLookup2_t;
        }
        break;
    case DRV_FTM_TCAM_TYPE_IGS_LPM0SAPUB_LK2:
        if (DRV_FTM_TCAM_SIZE_halfkey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv4SaPubHalfKeyAdLookup2_t;
        }
        else if(DRV_FTM_TCAM_SIZE_doublekey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6SaPubDoubleKey0AdLookup2_t;
        }
        break;

    case DRV_FTM_TCAM_TYPE_IGS_LPM0_ALL:
        if (DRV_FTM_TCAM_SIZE_halfkey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv4HalfKeyAd_t;
        }
        else if(DRV_FTM_TCAM_SIZE_doublekey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6DoubleKey0Ad_t;
        }
        else if(DRV_FTM_TCAM_SIZE_singlekey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6SingleKeyAd_t;
        }
        break;

    case DRV_FTM_TCAM_TYPE_IGS_LPM0_PUB_ALL:
        if (DRV_FTM_TCAM_SIZE_halfkey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv4HalfKeyAd_t;
        }
        else if(DRV_FTM_TCAM_SIZE_doublekey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6DoubleKey0Ad_t;
        }
        else if(DRV_FTM_TCAM_SIZE_singlekey == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6SingleKeyAd_t;
        }
        break;


    case DRV_FTM_TCAM_TYPE_IGS_LPM1:
        if(DRV_FTM_TCAM_SIZE_ipv4_nat == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv4NatDoubleKeyAd_t;
            p_ad_share_tbl[idx0++] = DsLpmTcamAd_t;
        }else if(DRV_FTM_TCAM_SIZE_ipv4_pbr == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6PbrQuadKeyAd_t;
        }
        else if(DRV_FTM_TCAM_SIZE_ipv6_nat == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6NatDoubleKeyAd_t;
        }else if(DRV_FTM_TCAM_SIZE_ipv6_pbr == size_type)
        {
            p_ad_share_tbl[idx0++] = DsLpmTcamIpv6PbrQuadKeyAd_t;
        }

        break;

    default:
        return DRV_E_INVAILD_TYPE;
    }

    *p_ad_share_num  = idx0;

    return DRV_E_NONE;

#undef DRV_FTM_TCAM_ACL_ING_AD_MAP
#undef DRV_FTM_TCAM_ACL_EGR_AD_MAP
}
#endif
tbls_ext_info_t*
drv_usw_ftm_build_tcam_tbl_info()
{
    tbls_ext_info_t* p_tbl_ext_info = NULL;
    tcam_mem_ext_content_t* p_tcam_mem_ext_info = NULL;

    p_tbl_ext_info = mem_malloc(MEM_FTM_MODULE, sizeof(tbls_ext_info_t));
    if (NULL == p_tbl_ext_info)
    {
        return NULL;
    }

    sal_memset(p_tbl_ext_info, 0, sizeof(tbls_ext_info_t));

    p_tcam_mem_ext_info = mem_malloc(MEM_FTM_MODULE, sizeof(tcam_mem_ext_content_t));
    if (NULL == p_tcam_mem_ext_info)
    {
        mem_free(p_tbl_ext_info);
        return NULL;
    }

    sal_memset(p_tcam_mem_ext_info, 0, sizeof(tcam_mem_ext_content_t));
    p_tcam_mem_ext_info->hw_mask_base =  mem_malloc(MEM_FTM_MODULE, sizeof(addrs_t)*3);
    if (NULL == p_tcam_mem_ext_info->hw_mask_base )
    {
        mem_free(p_tcam_mem_ext_info);
        mem_free(p_tbl_ext_info);
        return NULL;
    }
    p_tbl_ext_info->ptr_ext_content = p_tcam_mem_ext_info;

    return p_tbl_ext_info;

}


STATIC int32
drv_usw_ftm_alloc_tcam_key_share_tbl(uint8 lchip, uint8 tcam_key_type,
                                 uint32 tbl_id, uint8 force_realloc)
{
    uint32 max_index_num = 0;
    uint32 mem_id = 0;
    uint32 mem_idx = 0;
    tbls_ext_info_t* p_tbl_ext_info = NULL;
    uint32  is_lpm = 0;
    int32   per_entry_bytes = 0;
    uint32  mem_entry      = 0;
    is_lpm = 0;

    if(!TABLE_EXT_INFO_PTR(lchip, tbl_id))
    {
        p_tbl_ext_info = drv_usw_ftm_build_tcam_tbl_info();
        if (NULL == p_tbl_ext_info)
        {
            return DRV_E_NO_MEMORY;
        }
        TABLE_EXT_INFO_PTR(lchip, tbl_id) = p_tbl_ext_info;
    }
	else
	{
		max_index_num = force_realloc ? 0 : TABLE_MAX_INDEX(lchip, tbl_id);
	}
    per_entry_bytes            = DRV_BYTES_PER_ENTRY;
    switch(tcam_key_type)
    {
        case DRV_FTM_TCAM_TYPE_IGS_LPM_ALL:

        case DRV_FTM_TCAM_TYPE_IGS_LPM0DA:
        case DRV_FTM_TCAM_TYPE_IGS_LPM0SA:
        case DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB:
        case DRV_FTM_TCAM_TYPE_IGS_LPM0SAPUB:
         /*case DRV_FTM_TCAM_TYPE_IGS_LPM0DA_V6_HALF:*/
         /*case DRV_FTM_TCAM_TYPE_IGS_LPM0SA_V6_HALF:*/
        /* case DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB_V6_HALF:*/
        /* case DRV_FTM_TCAM_TYPE_IGS_LPM0SAPUB_V6_HALF:*/
            TABLE_EXT_INFO_TYPE(lchip, tbl_id) = EXT_INFO_TYPE_LPM_TCAM_IP;
            TCAM_KEY_MODULE(lchip, tbl_id) = TCAM_MODULE_LPM;
            per_entry_bytes             = DRV_LPM_KEY_BYTES_PER_ENTRY;
            is_lpm                      = 1;
            break;
        case DRV_FTM_TCAM_TYPE_IGS_LPM1:
        TABLE_EXT_INFO_TYPE(lchip, tbl_id) = EXT_INFO_TYPE_LPM_TCAM_NAT;
        TCAM_KEY_MODULE(lchip, tbl_id) = TCAM_MODULE_LPM;
            per_entry_bytes             = DRV_LPM_KEY_BYTES_PER_ENTRY;
            is_lpm                      = 1;
            break;

        case DRV_FTM_TCAM_TYPE_IGS_USERID0:
        case DRV_FTM_TCAM_TYPE_IGS_USERID1:
        case DRV_FTM_TCAM_TYPE_IGS_USERID2:
        case DRV_FTM_TCAM_TYPE_IGS_USERID3:
            TABLE_EXT_INFO_TYPE(lchip, tbl_id) = EXT_INFO_TYPE_TCAM;
            TCAM_KEY_MODULE(lchip, tbl_id) = TCAM_MODULE_SCL;
            break;

        default:
            TABLE_EXT_INFO_TYPE(lchip, tbl_id) = EXT_INFO_TYPE_TCAM;
            TCAM_KEY_MODULE(lchip, tbl_id) = TCAM_MODULE_ACL;
            break;
    }

    if (TABLE_ENTRY_SIZE(lchip, tbl_id) == 0)
    {
        DRV_FTM_DBG_DUMP("tbl_id:%d size is zero!!!\n", tbl_id);
        return DRV_E_NONE;
    }

    TCAM_KEY_TYPE(lchip, tbl_id) = tcam_key_type;

    TCAM_KEY_SIZE(lchip, tbl_id) = TABLE_ENTRY_SIZE(lchip, tbl_id);

    if(force_realloc)
    {
        TCAM_BITMAP(lchip, tbl_id) = DRV_FTM_KEY_MEM_BITMAP(tcam_key_type);
    }
    else
    {
        TCAM_BITMAP(lchip, tbl_id) |= DRV_FTM_KEY_MEM_BITMAP(tcam_key_type);
    }
    if(tcam_key_type <= DRV_FTM_TCAM_TYPE_IGS_USERID1)
    {
    #if (SDK_WORK_PLATFORM == 1)
        drv_set_tcam_lookup_bitmap(tcam_key_type, TCAM_BITMAP(lchip, tbl_id));
    #endif
    }

    if(is_lpm)
    {
        for (mem_idx = DRV_FTM_LPM_TCAM_KEY0; mem_idx < DRV_FTM_LPM_TCAM_KEYM; mem_idx++)
        {
            mem_id = mem_idx - DRV_FTM_LPM_TCAM_KEY0;

            /* if share list is no info in the block ,continue */
            if (!IS_BIT_SET(TCAM_BITMAP(lchip, (tbl_id)), mem_id))
            {
                continue;
            }

            mem_entry = DRV_FTM_MEM_MAX_ENTRY_NUM(mem_idx);

            TCAM_DATA_BASE(lchip, tbl_id, mem_id, 0) =
                DRV_FTM_MEM_HW_DATA_BASE(mem_idx);
            TCAM_MASK_BASE(lchip, tbl_id, mem_id, 0) =
                DRV_FTM_MEM_HW_MASK_BASE(mem_idx);

            TCAM_ENTRY_NUM(lchip, tbl_id, mem_id)      = mem_entry;
            TCAM_START_INDEX(lchip, tbl_id, mem_id)    = max_index_num;

            /* lpm per 46bit 分, 因此 最大值 / (一次需要几个46bit) == 最大多少个index */
            /* NAT /PBR 96bit 分, 因此 最大值 / (一次需要几个46bit) == 最大多少个index */
            max_index_num +=
                (mem_entry / (TABLE_ENTRY_SIZE(lchip, tbl_id) / per_entry_bytes));
            TCAM_END_INDEX(lchip, tbl_id, mem_id) = (max_index_num - 1);
        }
    }
    else
    {
        for (mem_idx = DRV_FTM_TCAM_KEY0; mem_idx < DRV_FTM_TCAM_KEYM; mem_idx++)
        {
            mem_id = mem_idx - DRV_FTM_TCAM_KEY0;

            /* if share list is no info in the block ,continue */
            if (!IS_BIT_SET(TCAM_BITMAP(lchip, (tbl_id)), mem_id))
            {
                continue;
            }

            mem_entry = DRV_FTM_MEM_MAX_ENTRY_NUM(mem_idx);

            TCAM_DATA_BASE(lchip, tbl_id, mem_id, 0) =
                DRV_FTM_MEM_HW_DATA_BASE(mem_idx);
            TCAM_MASK_BASE(lchip, tbl_id, mem_id, 0) =
                DRV_FTM_MEM_HW_MASK_BASE(mem_idx);

            TCAM_ENTRY_NUM(lchip, tbl_id, mem_id)      =  mem_entry;
            TCAM_START_INDEX(lchip, tbl_id, mem_id)    = max_index_num;

            /* flowhash per 96bit 分,因此 最大值 / (一次需要几个80bit) */
            max_index_num +=
                (mem_entry / (TABLE_ENTRY_SIZE(lchip, tbl_id) / per_entry_bytes));
            TCAM_END_INDEX(lchip, tbl_id, mem_id) = (max_index_num - 1);
        }
    }

    TABLE_EXT_INFO(lchip, tbl_id).entry = max_index_num;
    DRV_FTM_DBG_DUMP("Key TABLE_MAX_INDEX(lchip, %20s) = %d\n", TABLE_NAME(lchip, tbl_id), max_index_num);

    return DRV_E_NONE;

}

int32
drv_usw_ftm_get_tcam_tbl_info_detail(uint8 lchip, char* P_tbl_name)
{
    uint8 index = 0;
    uint32 tcam_key_type = 0;
    uint32 tcam_key_id = MaxTblId_t;
    uint32 tcam_key_a[20] = {0};
    uint32 tcam_ad_a[20] = {0};
    uint8 key_share_num  = 0;
    uint8 ad_share_num  = 0;
    uint8 mem_idx = 0;
    uint8 mem_id = 0;
     /*uint32 mem_entry = 0;*/
    uint32 tbl_id = 0;
    uint8 size_type = 0;

    DRV_INIT_CHECK(lchip);
    if (DRV_E_NONE != drv_usw_get_tbl_id_by_string(lchip, &tbl_id, (char*)P_tbl_name))
    {

        DRV_FTM_DBG_OUT("%% Not found %s\n", P_tbl_name);
        return DRV_E_INVALID_TBL;
    }

    for (tcam_key_type = DRV_FTM_TCAM_TYPE_IGS_ACL0; tcam_key_type < DRV_FTM_TCAM_TYPE_MAX; tcam_key_type++)
    {

        DRV_IF_ERROR_RETURN(_drv_usw_ftm_get_tcam_info(lchip, tcam_key_type,
                                                          tcam_key_a,
                                                          tcam_ad_a,
                                                          &key_share_num,
                                                          &ad_share_num));

        if (0 == key_share_num || 0 == ad_share_num)
        {
            continue;
        }

        /*tcam key alloc*/
        for (index = 0; index < key_share_num; index++)
        {
            tcam_key_id = tcam_key_a[index];

            if (tcam_key_id >= MaxTblId_t)
            {
                return DRV_E_NONE;
            }

            if ( (NULL == TABLE_INFO_PTR(lchip, tcam_key_id)) || 0 == TABLE_FIELD_NUM(lchip, tcam_key_id))
            {
                continue;
            }

            if (tcam_key_id != tbl_id)
            {
                continue;
            }

            size_type = TCAM_KEY_SIZE(lchip, tbl_id) / (DRV_LPM_KEY_BYTES_PER_ENTRY);
            if (size_type == 2)
            {
                size_type = DRV_FTM_TCAM_SIZE_LPMSingle;
            }
            else if(size_type == 4)
            {
                size_type = DRV_FTM_TCAM_SIZE_LPMDouble;
            }
            else
            {
                size_type = DRV_FTM_TCAM_SIZE_LPMhalf;
            }

            for (mem_idx = DRV_FTM_LPM_TCAM_KEY0; mem_idx < DRV_FTM_LPM_TCAM_KEYM; mem_idx++)
            {
                mem_id = mem_idx - DRV_FTM_LPM_TCAM_KEY0;

                /* if share list is no info in the block ,continue */
                if (!IS_BIT_SET(TCAM_BITMAP(lchip, (tcam_key_id)), mem_id))
                {
                    continue;
                }

                 /*mem_entry = DRV_FTM_MEM_MAX_ENTRY_NUM(mem_idx);*/

                DRV_FTM_DBG_OUT("%-30s :%s\n", "Tcam Table ID", TABLE_NAME(lchip, tcam_key_id));
                DRV_FTM_DBG_OUT("%-30s :%d\n", "Tcam Memory ID", mem_id);
                DRV_FTM_DBG_OUT("%-30s :0x%x\n", "Memory max entry", TCAM_ENTRY_NUM(lchip, tcam_key_id, mem_id));
                DRV_FTM_DBG_OUT("%-30s :0x%x\n", "Table Start Idx In Mem", TCAM_START_INDEX(lchip, tcam_key_id, mem_id));
                DRV_FTM_DBG_OUT("%-30s :0x%x\n", "Table End Idx In Mem", TCAM_END_INDEX(lchip, tcam_key_id, mem_id));

                 /*DRV_FTM_DBG_OUT("%-30s :0x%x\n", "Spec max entry num", g_usw_ftm_master[lchip]->tcam_specs[tcam_key_type][size_type]);*/
                /* DRV_FTM_DBG_OUT("%-30s :0x%x\n", "Table Start Idx In Spec", g_usw_ftm_master[lchip]->tcam_offset[tcam_key_type][size_type]);*/
                DRV_FTM_DBG_OUT("\n");
            }
        }

        for (index = 0; index < ad_share_num; index++)
        {
            tcam_key_id = tcam_ad_a[index];

            if (tcam_key_id >= MaxTblId_t)
            {
                return DRV_E_NONE;
            }

            if ( (NULL == TABLE_INFO_PTR(lchip, tcam_key_id)) || 0 == TABLE_FIELD_NUM(lchip, tcam_key_id))
            {
                continue;
            }

            if (tcam_key_id != tbl_id)
            {
                continue;
            }

            size_type = TCAM_KEY_SIZE(lchip, tbl_id) / (DRV_LPM_KEY_BYTES_PER_ENTRY);
            if (size_type == 2)
            {
                size_type = DRV_FTM_TCAM_SIZE_LPMSingle;
            }
            else if(size_type == 4)
            {
                size_type = DRV_FTM_TCAM_SIZE_LPMDouble;
            }
            else
            {
                size_type = DRV_FTM_TCAM_SIZE_LPMhalf;
            }

            for (mem_idx = DRV_FTM_LPM_TCAM_KEY0; mem_idx < DRV_FTM_LPM_TCAM_KEYM; mem_idx++)
            {
                mem_id = mem_idx - DRV_FTM_LPM_TCAM_KEY0;

                /* if share list is no info in the block ,continue */
                if (!IS_BIT_SET(TCAM_BITMAP(lchip, (tcam_key_id)), mem_id))
                {
                    continue;
                }

                 /*mem_entry = DRV_FTM_MEM_MAX_ENTRY_NUM(mem_idx);*/

                DRV_FTM_DBG_OUT("%-30s :%s\n", "Tcam Table ID", TABLE_NAME(lchip, tcam_key_id));
                DRV_FTM_DBG_OUT("%-30s :%d\n", "Tcam Memory ID", mem_id);
                DRV_FTM_DBG_OUT("%-30s :0x%x\n", "Memory max entry", TCAM_ENTRY_NUM(lchip, tcam_key_id, mem_id));
                DRV_FTM_DBG_OUT("%-30s :0x%x\n", "Table Start Idx In Mem", TCAM_START_INDEX(lchip, tcam_key_id, mem_id));
                DRV_FTM_DBG_OUT("%-30s :0x%x\n", "Table End Idx In Mem", TCAM_END_INDEX(lchip, tcam_key_id, mem_id));

                 /*DRV_FTM_DBG_OUT("%-30s :0x%x\n", "Spec max entry num", g_usw_ftm_master[lchip]->tcam_specs[tcam_key_type][size_type]);*/
                /* DRV_FTM_DBG_OUT("%-30s :0x%x\n", "Table Start Idx In Spec", g_usw_ftm_master[lchip]->tcam_offset[tcam_key_type][size_type]);*/
                DRV_FTM_DBG_OUT("\n");
            }
        }

    }

    return DRV_E_NONE;
}

STATIC int32
drv_usw_ftm_alloc_tcam_ad_share_tbl(uint8 lchip, uint8 tcam_key_type,
                                 uint32 tbl_id, uint8 force_realloc)
{
    uint32 max_index_num = 0;
    int8 mem_id = 0;
    uint8 mem_idx = 0;
    tbls_ext_info_t* p_tbl_ext_info = NULL;
    bool is_lpm = 0;
    uint8 per_entry_bytes = 0;
    uint32 mem_entry      = 0;

    if(!TABLE_EXT_INFO_PTR(lchip, tbl_id))
    {
        p_tbl_ext_info = drv_usw_ftm_build_tcam_tbl_info();
        if (NULL == p_tbl_ext_info)
        {
          return DRV_E_NO_MEMORY;
        }

        TABLE_EXT_INFO_PTR(lchip, tbl_id) = p_tbl_ext_info;
    }
	else
	{
		max_index_num = force_realloc ? 0 : TABLE_MAX_INDEX(lchip, tbl_id);
	}
    switch(tcam_key_type)
    {
    case DRV_FTM_TCAM_TYPE_IGS_LPM_ALL:

    case DRV_FTM_TCAM_TYPE_IGS_LPM0DA:
    case DRV_FTM_TCAM_TYPE_IGS_LPM0SA:
    case DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB:
    case DRV_FTM_TCAM_TYPE_IGS_LPM0SAPUB:
     /*case DRV_FTM_TCAM_TYPE_IGS_LPM0DA_V6_HALF:*/
     /*case DRV_FTM_TCAM_TYPE_IGS_LPM0SA_V6_HALF:*/
    /* case DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB_V6_HALF:*/
     /*case DRV_FTM_TCAM_TYPE_IGS_LPM0SAPUB_V6_HALF:*/
        TABLE_EXT_INFO_TYPE(lchip, tbl_id) = EXT_INFO_TYPE_TCAM_LPM_AD;
        per_entry_bytes             = DRV_LPM_AD0_BYTE_PER_ENTRY;
        is_lpm                      = 1;
        TCAM_KEY_MODULE(lchip, tbl_id) = TCAM_MODULE_LPM;
        break;
    case DRV_FTM_TCAM_TYPE_IGS_LPM1:
        TABLE_EXT_INFO_TYPE(lchip, tbl_id) = EXT_INFO_TYPE_TCAM_NAT_AD;
        per_entry_bytes             = DRV_LPM_AD1_BYTE_PER_ENTRY;
        is_lpm                      = 1;
        TCAM_KEY_MODULE(lchip, tbl_id) = TCAM_MODULE_LPM;
        break;

    case DRV_FTM_TCAM_TYPE_IGS_USERID0:
    case DRV_FTM_TCAM_TYPE_IGS_USERID1:
    case DRV_FTM_TCAM_TYPE_IGS_USERID2:
    case DRV_FTM_TCAM_TYPE_IGS_USERID3:
        TABLE_EXT_INFO_TYPE(lchip, tbl_id) = EXT_INFO_TYPE_TCAM_AD;
        TCAM_KEY_MODULE(lchip, tbl_id) = TCAM_MODULE_SCL;
        break;

    default:
        TABLE_EXT_INFO_TYPE(lchip, tbl_id) = EXT_INFO_TYPE_TCAM_AD;
        TCAM_KEY_MODULE(lchip, tbl_id) = TCAM_MODULE_ACL;
        break;
    }

    TCAM_KEY_TYPE(lchip, tbl_id) = tcam_key_type;

     /*- if(DsTcam0_t == tbl_id)*/
     /*- {*/
     /*-     TCAM_BITMAP(lchip, DsTcam0_t) = 0x1FFFF;*/
     /*-  }*/
     /*-  else*/
    if(force_realloc)
    {
        TCAM_BITMAP(lchip, tbl_id) = DRV_FTM_KEY_MEM_BITMAP(tcam_key_type);
    }
    else
    {
        TCAM_BITMAP(lchip, tbl_id) |= DRV_FTM_KEY_MEM_BITMAP(tcam_key_type);
    }

    if(is_lpm)
    {
        for (mem_idx = DRV_FTM_LPM_TCAM_AD0; mem_idx < DRV_FTM_LPM_TCAM_ADM; mem_idx++)
        {
             mem_id = mem_idx - DRV_FTM_LPM_TCAM_AD0;

            /* if share list is no info in the block ,continue */
            if (!IS_BIT_SET(TCAM_BITMAP(lchip, tbl_id),mem_id))
            {
                continue;
            }

            mem_entry = DRV_FTM_MEM_MAX_ENTRY_NUM(mem_idx);

            TCAM_DATA_BASE(lchip, tbl_id, mem_id, 0)   =
                DRV_FTM_MEM_HW_DATA_BASE(mem_idx);

            TCAM_ENTRY_NUM(lchip, tbl_id, mem_id)      = mem_entry;
            TCAM_START_INDEX(lchip, tbl_id, mem_id)    = max_index_num;

            /*
                46Bit
             */

            max_index_num +=
                (mem_entry / (TABLE_ENTRY_SIZE(lchip, tbl_id) / per_entry_bytes));
            TCAM_END_INDEX(lchip, tbl_id, mem_id) = (max_index_num - 1);


        }
    }
    else
    {
        for (mem_idx = DRV_FTM_TCAM_AD0; mem_idx < DRV_FTM_TCAM_ADM; mem_idx++)
        {
             mem_id = mem_idx - DRV_FTM_TCAM_AD0;

            /* if share list is no info in the block ,continue */
            if (!IS_BIT_SET(TCAM_BITMAP(lchip, tbl_id),mem_id))
            {
                continue;
            }

            mem_entry = DRV_FTM_MEM_MAX_ENTRY_NUM(mem_idx);

            TCAM_DATA_BASE(lchip, tbl_id, mem_id, 0) =
                DRV_FTM_MEM_HW_DATA_BASE(mem_idx);

            TCAM_ENTRY_NUM(lchip, tbl_id, mem_id)      = mem_entry;
            TCAM_START_INDEX(lchip, tbl_id, mem_id)    = max_index_num;

            max_index_num +=
                (mem_entry / ( TABLE_ENTRY_SIZE(lchip, tbl_id) / DRV_BYTES_PER_ENTRY));
            TCAM_END_INDEX(lchip, tbl_id, mem_id) = (max_index_num - 1);

            DRV_FTM_DBG_DUMP("Ad TCAM_START_INDEX(lchip, %20s, %d) = %d\n", TABLE_NAME(lchip, tbl_id), mem_id, TCAM_START_INDEX(lchip, tbl_id, mem_id));
            DRV_FTM_DBG_DUMP("Ad TCAM_END_INDEX(lchip, %20s, %d) = %d\n", TABLE_NAME(lchip, tbl_id), mem_id, TCAM_END_INDEX(lchip, tbl_id, mem_id));
        }
    }

    TABLE_EXT_INFO(lchip, tbl_id).entry = max_index_num;
    drv_usw_ftm_alloc_tcam_ad_map_to_new_word(lchip, tbl_id);

    DRV_FTM_DBG_DUMP("Ad TCAM_BITMAP(lchip, %20s) = %d\n", TABLE_NAME(lchip, tbl_id), TCAM_BITMAP(lchip, tbl_id));
    DRV_FTM_DBG_DUMP("Ad TABLE_MAX_INDEX(lchip, %20s) = %d\n", TABLE_NAME(lchip, tbl_id), TABLE_MAX_INDEX(lchip, tbl_id));
    DRV_FTM_DBG_DUMP("----------------------------------------\n");

    return DRV_E_NONE;

}

STATIC int32
drv_usw_ftm_alloc_tcam(uint8 lchip, uint8 start_key_type, uint8 end_key_type, uint8 force_realloc)
{
    uint8 index = 0;
    uint32 tcam_key_type = 0;
    uint32 tcam_key_id = MaxTblId_t;
    uint32 tcam_ad_tbl = MaxTblId_t;
    uint32 tcam_key_a[20] = {0};
    uint32 tcam_ad_a[20] = {0};
    uint8 key_share_num  = 0;
    uint8 ad_share_num  = 0;

    for (tcam_key_type = start_key_type; tcam_key_type < end_key_type; tcam_key_type++)
    {

        DRV_IF_ERROR_RETURN(_drv_usw_ftm_get_tcam_info(lchip,
                                                          tcam_key_type,
                                                          tcam_key_a,
                                                          tcam_ad_a,
                                                          &key_share_num,
                                                          &ad_share_num));

        if (0 == key_share_num || 0 == ad_share_num)
        {
            continue;
        }

        /*tcam key alloc*/
        for (index = 0; index < key_share_num; index++)
        {
            tcam_key_id = tcam_key_a[index];

            if (tcam_key_id >= MaxTblId_t)
            {
                DRV_FTM_DBG_DUMP("unexpect tbl id:%d\r\n", tcam_key_id);
                return DRV_E_INVAILD_TYPE;
            }

            if ( (NULL == TABLE_INFO_PTR(lchip, tcam_key_id)) || 0 == TABLE_FIELD_NUM(lchip, tcam_key_id))
            {
                continue;
            }
            DRV_IF_ERROR_RETURN(drv_usw_ftm_alloc_tcam_key_share_tbl(lchip, tcam_key_type, tcam_key_id, force_realloc));
        }

        /*tcam ad alloc*/
        for (index = 0; index < ad_share_num; index++)
        {
            tcam_ad_tbl = tcam_ad_a[index];

            if (tcam_ad_tbl >= MaxTblId_t)
            {
                DRV_FTM_DBG_DUMP("unexpect tbl id:%d\r\n", tcam_ad_tbl);
                return DRV_E_INVAILD_TYPE;
            }


            if ( (NULL == TABLE_INFO_PTR(lchip, tcam_ad_tbl)) || 0 == TABLE_FIELD_NUM(lchip, tcam_ad_tbl))
            {
                continue;
            }
            DRV_IF_ERROR_RETURN(drv_usw_ftm_alloc_tcam_ad_share_tbl(lchip, tcam_key_type, tcam_ad_tbl, force_realloc));
        }

    }

    return DRV_E_NONE;

}

STATIC int32
drv_usw_ftm_alloc_static(uint8 lchip)
{
    uint8 index = 0;
    uint32 tcam_key_type = 0;
    uint32 tbl_id = MaxTblId_t;
    uint32 tcam_key_a[20] = {0};
    uint32 tcam_ad_a[20] = {0};
    uint8 key_share_num  = 0;
    uint8 ad_share_num  = 0;
    tbls_ext_info_t* p_tbl_ext_info = NULL;


    tcam_key_type = DRV_FTM_TCAM_TYPE_STATIC_TCAM;

    DRV_IF_ERROR_RETURN(_drv_usw_ftm_get_tcam_info(lchip,tcam_key_type,
                                              tcam_key_a,
                                              tcam_ad_a,
                                              &key_share_num,
                                              &ad_share_num));

    /*tcam key alloc*/
    for (index = 0; index < key_share_num; index++)
    {
        tbl_id = tcam_key_a[index];

        if (tbl_id >= MaxTblId_t)
        {
            DRV_FTM_DBG_DUMP("unexpect tbl id:%d\r\n", tbl_id);
            return DRV_E_INVAILD_TYPE;
        }


        if ( (NULL == TABLE_INFO_PTR(lchip, tbl_id)) || 0 == TABLE_FIELD_NUM(lchip, tbl_id))
        {
             continue;
        }

        p_tbl_ext_info = drv_usw_ftm_build_tcam_tbl_info();
        if (NULL == p_tbl_ext_info)
        {
            return DRV_E_NO_MEMORY;
        }

        TABLE_EXT_INFO_PTR(lchip, tbl_id) = p_tbl_ext_info;


        TABLE_EXT_INFO_TYPE(lchip, tbl_id) = EXT_INFO_TYPE_STATIC_TCAM_KEY;
        TCAM_KEY_MODULE(lchip, tbl_id) = TCAM_MODULE_NONE;
        TCAM_KEY_TYPE(lchip, tbl_id) = tcam_key_type;

        TCAM_KEY_SIZE(lchip, tbl_id) = TABLE_ENTRY_SIZE(lchip, tbl_id);/* table key size are in bytes */
        TCAM_BITMAP(lchip, tbl_id) = 0;

        if (tbl_id == DsCategoryIdPairTcamKey_t)
        {
            TCAM_DATA_BASE(lchip, tbl_id, 0, 0) = DRV_FTM_MEM_HW_DATA_BASE(DRV_FTM_CID_TCAM);
            TCAM_MASK_BASE(lchip, tbl_id, 0, 0) = DRV_FTM_MEM_HW_MASK_BASE(DRV_FTM_CID_TCAM);
        }
        else
        {

            TCAM_DATA_BASE(lchip, tbl_id, 0, 0) = DRV_FTM_MEM_HW_DATA_BASE(DRV_FTM_QUEUE_TCAM);
            TCAM_MASK_BASE(lchip, tbl_id, 0, 0) =  DRV_FTM_MEM_HW_MASK_BASE(DRV_FTM_QUEUE_TCAM);;
        }

        TCAM_ENTRY_NUM(lchip, tbl_id, 0) = TABLE_MAX_INDEX(lchip, tbl_id);
        TCAM_TABLE_SW_BASE(lchip, tbl_id, 0) = 0;
        TCAM_START_INDEX(lchip, tbl_id, 0) = 0;
        TCAM_END_INDEX(lchip, tbl_id, 0) = TABLE_MAX_INDEX(lchip, tbl_id) - 1;

    }

  return DRV_E_NONE;
}


int32
drv_usw_ftm_alloc_cam(uint8 lchip, drv_ftm_cam_t *p_cam_profile)
{
    #if 0
    uint32 bitmap = 0;

    /*fib acc cam*/
    /*bit
    bit[0]: mac
    bit[1]: ipv4 uc host
    bit[2]: ipv4 l2 mc
    bit[3]: ipv4 mc
    bit[4]: ipv6 uc hot
    bit[5]: ipv6 l2 mc
    bit[6]: ipv6 mc
    bit[7]: fcoe
    bit[8]: trill
    */
    SET_BIT(bitmap, 1);
    SET_BIT(bitmap, 4);
    SET_BIT(bitmap, 7);
    SET_BIT(bitmap, 8);
    if((0 == p_cam_profile->conflict_cam_num[DRV_FTM_CAM_TYPE_FIB_HOST0_FDB])
       && (0 == p_cam_profile->conflict_cam_num[DRV_FTM_CAM_TYPE_FIB_HOST0_MC]))
    {/*share*/

    }
    else if(FIB_HOST0_CAM_NUM == p_cam_profile->conflict_cam_num[DRV_FTM_CAM_TYPE_FIB_HOST0_FDB])
    {
        SET_BIT(bitmap, 2);
        SET_BIT(bitmap, 3);
        SET_BIT(bitmap, 5);
        SET_BIT(bitmap, 6);
    }
    else if(FIB_HOST0_CAM_NUM == p_cam_profile->conflict_cam_num[DRV_FTM_CAM_TYPE_FIB_HOST0_MC])
    {
        SET_BIT(bitmap, 0);
    }
    /*
    else if(FIB_HOST0_CAM_NUM == (p_cam_profile->fdb_conflict_num + p_cam_profile->ipmc_g_conflict_num))
    {
        //write valid bit
        drv_fib_acc_in_t* fib_acc_in;
        drv_fib_acc_out_t* fib_acc_out;
        uint8 index = 0;
        ds_fib_host0_ipv4_hash_key_m ds_fib_host0_ipv4_hash_key;

        sal_memset(&fib_acc_in, 0, sizeof(fib_acc_in));
        sal_memset(&fib_acc_out, 0, sizeof(drv_fib_acc_out_t));
        sal_memset(&ds_fib_host0_ipv4_hash_key, 0, sizeof(ds_fib_host0_ipv4_hash_key));

        SetDsFibHost0Ipv4HashKey(V, valid_f, &ds_fib_host0_ipv4_hash_key, 1);
        for (index = 0; index < p_cam_profile->ipmc_g_conflict_num; index ++)
        {
            fib_acc_in.rw.tbl_id = DsFibHost0Ipv4HashKey_t;
            fib_acc_in.rw.key_index = index;
            fib_acc_in.rw.data = (void*)&ds_fib_host0_ipv4_hash_key;
            drv_fib_acc(0, DRV_FIB_ACC_WRITE_FIB0_BY_IDX, &fib_acc_in, &fib_acc_out);
        }

        SET_BIT(bitmap, 2);
        SET_BIT(bitmap, 3);
        SET_BIT(bitmap, 5);
        SET_BIT(bitmap, 6);
    }
    */
    /* fib host0 acc bitmap */
    g_usw_ftm_master[lchip]->cam_bitmap = bitmap;

    sal_memcpy(&g_usw_ftm_master[lchip]->conflict_cam_num[0], &p_cam_profile->conflict_cam_num[0],
                sizeof(p_cam_profile->conflict_cam_num[0])*DRV_FTM_CAM_TYPE_MAX);
    sal_memcpy(&g_usw_ftm_master[lchip]->cfg_max_cam_num[0], &p_cam_profile->conflict_cam_num[0],
                sizeof(p_cam_profile->conflict_cam_num[0])*DRV_FTM_CAM_TYPE_MAX);


     /*if (p_cam_profile->conflict_cam_num[DRV_FTM_CAM_TYPE_OAM])*/
    {
        SET_BIT(g_usw_ftm_master[lchip]->xcoam_cam_bitmap0[3], 31); /*single*/
        SET_BIT(g_usw_ftm_master[lchip]->xcoam_cam_bitmap1[1], 31); /*dual*/
        SET_BIT(g_usw_ftm_master[lchip]->xcoam_cam_bitmap2[0], 31); /*quad*/

        if(g_usw_ftm_master[lchip]->conflict_cam_num[DRV_FTM_CAM_TYPE_OAM])
        {
            g_usw_ftm_master[lchip]->conflict_cam_num[DRV_FTM_CAM_TYPE_OAM] -= 1;
        }
        else
        {
            g_usw_ftm_master[lchip]->conflict_cam_num[DRV_FTM_CAM_TYPE_XC] -= 1;
        }
    }
    #endif
    return DRV_E_NONE;
}



int32
drv_usw_ftm_get_cam_by_tbl_id(uint32 tbl_id, uint8* cam_type, uint8* cam_num)
{
    uint8 type = DRV_FTM_CAM_TYPE_INVALID;

    switch(tbl_id)
    {
        case DsFibHost1Ipv6NatDaPortHashKey_t:
        case DsFibHost1Ipv6NatSaPortHashKey_t:
        case DsFibHost1Ipv4NatDaPortHashKey_t:
        case DsFibHost1Ipv4NatSaPortHashKey_t:
        case DsFibHost1Ipv4McastHashKey_t:
        case DsFibHost1Ipv6McastHashKey_t:
        case DsFibHost1MacIpv4McastHashKey_t:
        case DsFibHost1MacIpv6McastHashKey_t:
            type =DRV_FTM_CAM_TYPE_FIB_HOST1 ;
            *cam_num = 32;
            break;

        case DsEgressXcOamPortVlanCrossHashKey_t:
        case DsEgressXcOamCvlanCosPortHashKey_t:
        case DsEgressXcOamPortCrossHashKey_t:
        case DsEgressXcOamCvlanPortHashKey_t:
        case DsEgressXcOamDoubleVlanPortHashKey_t:
        case DsEgressXcOamSvlanCosPortHashKey_t:
        case DsEgressXcOamSvlanPortHashKey_t:
        case DsEgressXcOamPortHashKey_t:
        case DsEgressXcOamSvlanPortMacHashKey_t:
        case DsEgressXcOamTunnelPbbHashKey_t:
        case DsEgressXcOamBfdHashKey_t:
        case DsEgressXcOamEthHashKey_t:
        case DsEgressXcOamMplsLabelHashKey_t:
        case DsEgressXcOamMplsSectionHashKey_t:
        case DsEgressXcOamRmepHashKey_t:
            type = DRV_FTM_CAM_TYPE_XC;
            *cam_num = 128;
            break;

        case DsUserIdTunnelMplsHashKey_t:
        case DsUserIdCvlanCosPortHashKey_t:
        case DsUserIdCvlanPortHashKey_t:
        case DsUserIdDoubleVlanPortHashKey_t:
        case DsUserIdIpv4PortHashKey_t:
        case DsUserIdIpv4SaHashKey_t:
        case DsUserIdIpv6PortHashKey_t:
        case DsUserIdIpv6SaHashKey_t:
        case DsUserIdMacHashKey_t:
        case DsUserIdMacPortHashKey_t:
        case DsUserIdPortHashKey_t:
        case DsUserIdSvlanCosPortHashKey_t:
        case DsUserIdSvlanHashKey_t:
        case DsUserIdSvlanMacSaHashKey_t:
        case DsUserIdSvlanPortHashKey_t:
        case DsUserIdSclFlowL2HashKey_t:
        case DsUserIdTunnelIpv4DaHashKey_t:
        case DsUserIdTunnelIpv4GreKeyHashKey_t:
        case DsUserIdTunnelIpv4HashKey_t:
        case DsUserIdTunnelIpv4RpfHashKey_t:
        case DsUserIdTunnelIpv4UdpHashKey_t:
        case DsUserIdTunnelIpv4McNvgreMode0HashKey_t:
        case DsUserIdTunnelIpv4McVxlanMode0HashKey_t:
        case DsUserIdTunnelIpv4NvgreMode1HashKey_t:
        case DsUserIdTunnelIpv4VxlanMode1HashKey_t:
        case DsUserIdTunnelIpv4UcNvgreMode0HashKey_t:
        case DsUserIdTunnelIpv4UcNvgreMode1HashKey_t:
        case DsUserIdTunnelIpv4UcVxlanMode0HashKey_t:
        case DsUserIdTunnelIpv4UcVxlanMode1HashKey_t:
        case DsUserIdTunnelIpv6McNvgreMode0HashKey_t:
        case DsUserIdTunnelIpv6McNvgreMode1HashKey_t:
        case DsUserIdTunnelIpv6McVxlanMode0HashKey_t:
        case DsUserIdTunnelIpv6McVxlanMode1HashKey_t:
        case DsUserIdTunnelIpv6UcNvgreMode0HashKey_t:
        case DsUserIdTunnelIpv6UcNvgreMode1HashKey_t:
        case DsUserIdTunnelIpv6UcVxlanMode0HashKey_t:
        case DsUserIdTunnelIpv6UcVxlanMode1HashKey_t:
            type = DRV_FTM_CAM_TYPE_SCL;
            *cam_num = 32;
            break;
        case DsFlowL2HashKey_t:
        case DsFlowL2L3HashKey_t:
        case DsFlowL3Ipv4HashKey_t:
        case DsFlowL3Ipv6HashKey_t:
        case DsFlowL3MplsHashKey_t:
            type = DRV_FTM_CAM_TYPE_FLOW;
            *cam_num = 32;
            break;
        case  DsFibHost0FcoeHashKey_t:
        case DsFibHost0Ipv4HashKey_t:
        case DsFibHost0Ipv6McastHashKey_t:
        case DsFibHost0Ipv6UcastHashKey_t:
        case DsFibHost0MacHashKey_t:
        case DsFibHost0MacIpv6McastHashKey_t:
        case DsFibHost0TrillHashKey_t:
            type = DRV_FTM_CAM_TYPE_FIB_HOST0;
            *cam_num = 32;
            break;
        default:
            type = DRV_FTM_CAM_TYPE_INVALID;
            break;

    }

    *cam_type = type;

    return DRV_E_NONE;
}


STATIC int32
drv_usw_ftm_set_profile(uint8 lchip, drv_ftm_profile_info_t *profile_info)
{
    uint8 profile_index    = 0;

    profile_index = profile_info->profile_type;
    switch (profile_index)
    {
    case 0:
        if (DRV_IS_DUET2(lchip))
        {
            _drv_usw_ftm_set_profile_default(lchip);
        }
        else
        {
            /*_drv_tsingma_ftm_set_profile_1(lchip);*/
            _drv_tsingma_ftm_set_profile_default(lchip);
        }
        break;

    case 12:
        _drv_usw_ftm_set_profile_user_define(lchip, profile_info);
        break;

    default:
        return DRV_E_NONE;
    }
    if(DRV_IS_TSINGMA(lchip))
    {
        _drv_usw_ftm_set_profile_mpls_gem_port(lchip);
    }
    return DRV_E_NONE;

}



STATIC int32
drv_usw_ftm_init(uint8 lchip, drv_ftm_profile_info_t *profile_info)
{
    if (NULL != g_usw_ftm_master[lchip])
    {
        return DRV_E_NONE;
    }

    g_usw_ftm_master[lchip] = mem_malloc(MEM_FTM_MODULE, sizeof(g_ftm_master_t));

    if (NULL == g_usw_ftm_master[lchip])
    {
        return DRV_E_NO_MEMORY;
    }

    sal_memset(g_usw_ftm_master[lchip], 0, sizeof(g_ftm_master_t));

    g_usw_ftm_master[lchip]->p_sram_tbl_info =
        mem_malloc(MEM_FTM_MODULE, sizeof(drv_ftm_sram_tbl_info_t) * DRV_FTM_SRAM_TBL_MAX);
    if (NULL == g_usw_ftm_master[lchip]->p_sram_tbl_info)
    {
        mem_free(g_usw_ftm_master[lchip]);
        return DRV_E_NO_MEMORY;
    }

    sal_memset(g_usw_ftm_master[lchip]->p_sram_tbl_info, 0, sizeof(drv_ftm_sram_tbl_info_t) * DRV_FTM_SRAM_TBL_MAX);

    g_usw_ftm_master[lchip]->p_tcam_key_info =
        mem_malloc(MEM_FTM_MODULE, sizeof(drv_ftm_tcam_key_info_t) * DRV_FTM_TCAM_TYPE_MAX);
    if (NULL == g_usw_ftm_master[lchip]->p_tcam_key_info)
    {
        mem_free(g_usw_ftm_master[lchip]->p_sram_tbl_info);
        mem_free(g_usw_ftm_master[lchip]);
        return DRV_E_NO_MEMORY;
    }

    sal_memset(g_usw_ftm_master[lchip]->p_tcam_key_info, 0, sizeof(drv_ftm_tcam_key_info_t) * DRV_FTM_TCAM_TYPE_MAX);

    g_usw_ftm_master[lchip]->lpm_model = profile_info->lpm_mode;
	g_usw_ftm_master[lchip]->napt_enable= profile_info->napt_enable;

	g_usw_ftm_master[lchip]->mpls_mode = profile_info->mpls_mode;
    g_usw_ftm_master[lchip]->scl_mode = profile_info->scl_mode;

    return DRV_E_NONE;
}




STATIC int32
drv_usw_tcam_ctl_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 sel = 0;
    uint8  mem_id = 0;
    uint8  field_idx;
    uint32 single_key_bmp = 0;
    FlowTcamLookupCtl_m FlowTcamLookupCtl;
    LpmTcamCtl_m lpmtcam;

    sal_memset(&FlowTcamLookupCtl, 0, sizeof(FlowTcamLookupCtl));
    cmd = DRV_IOR(LpmTcamCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lpmtcam));
    SetLpmTcamCtl(V, privatePublicLookupMode_f, &lpmtcam, g_usw_ftm_master[lchip]->lpm_model);
    SetLpmTcamCtl(V, natPbrTcamLookupEn_f, &lpmtcam, g_usw_ftm_master[lchip]->nat_pbr_enable);
    SetLpmTcamCtl(V, natTcamLookupEn_f, &lpmtcam, g_usw_ftm_master[lchip]->nat_pbr_enable);
    SetLpmTcamCtl(V, lpmTcamLookup2Division_f, &lpmtcam, 0);

    /*clear table value for change memory*/
    for(field_idx=0; field_idx < 6;field_idx++)
    {
        SetLpmTcamCtl(V,tcamGroupConfig_0_ipv6Lookup64Prefix_f+field_idx,&lpmtcam, 0);
    }

    /*enable single key, single & v4 share the same ram*/
    if((g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][1] | g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][1]) != 0)
    {
        single_key_bmp = g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][1] | g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][1]\
                     | g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][2] | g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][2];
        for(field_idx=0; field_idx < 6;field_idx++)
        {
            if(IS_BIT_SET(single_key_bmp, 2*field_idx) && IS_BIT_SET(single_key_bmp,(2*field_idx+1)))
            {
                SetLpmTcamCtl(V,tcamGroupConfig_0_ipv6Lookup64Prefix_f+field_idx,&lpmtcam, 1);
            }
        }
    }

    cmd = DRV_IOW(LpmTcamCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &lpmtcam));

    cmd = DRV_IOR(FlowTcamLookupCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &FlowTcamLookupCtl));

    if (DRV_IS_DUET2(lchip))
    {
        /*User1/ACL1/ACl4/ACl7 sel*/
        mem_id = DRV_FTM_TCAM_KEY13 - DRV_FTM_TCAM_KEY0;
        if (IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_USERID1), mem_id))
        {
            sel = 0;
        }
        else if(IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_ACL1), mem_id))
        {
            sel = 1;
        }
        else if(IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_ACL4), mem_id))
        {
            sel = 2;
        }
        else if(IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_ACL7), mem_id))
        {
            sel = 3;
        }
        SetFlowTcamLookupCtl(V, share0IntfSel_f, &FlowTcamLookupCtl, sel);


        /*User0/ACL0/ACL2/ACl5 sel*/
        mem_id = DRV_FTM_TCAM_KEY14 - DRV_FTM_TCAM_KEY0;
        if (IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_USERID0), mem_id))
        {
            sel = 0;
        }
        else if(IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_ACL0), mem_id))
        {
            sel = 1;
        }
        else if(IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_ACL2), mem_id))
        {
            sel = 2;
        }
        else if(IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_ACL5), mem_id))
        {
            sel = 3;
        }
        SetFlowTcamLookupCtl(V, share1IntfSel_f, &FlowTcamLookupCtl, sel);


        /*User0/ACL3/ACl6 sel*/
        mem_id = DRV_FTM_TCAM_KEY15 - DRV_FTM_TCAM_KEY0;
        if (IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_USERID0), mem_id))
        {
            sel = 0;
        }
        else if(IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_ACL3), mem_id))
        {
            sel = 1;
        }
        else if(IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_ACL6), mem_id))
        {
            sel = 2;
        }
        SetFlowTcamLookupCtl(V, share2IntfSel_f, &FlowTcamLookupCtl, sel);
    }
    else
    {
        /*User2/ACL0/ACl4 sel*/
        mem_id = DRV_FTM_TCAM_KEY15 - DRV_FTM_TCAM_KEY0;
        if (IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_USERID2), mem_id))
        {
            sel = 0;
        }
        else if(IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_ACL0), mem_id))
        {
            sel = 1;
        }
        else if(IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_ACL4), mem_id))
        {
            sel = 2;
        }
        SetFlowTcamLookupCtl(V, share0IntfSel_f, &FlowTcamLookupCtl, sel);


        /*User3/ACL1/ACl5 sel*/
        mem_id = DRV_FTM_TCAM_KEY16 - DRV_FTM_TCAM_KEY0;
        if (IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_USERID3), mem_id))
        {
            sel = 0;
        }
        else if(IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_ACL1), mem_id))
        {
            sel = 1;
        }
        else if(IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_ACL5), mem_id))
        {
            sel = 2;
        }
        SetFlowTcamLookupCtl(V, share1IntfSel_f, &FlowTcamLookupCtl, sel);


        /*ACL2/ACl6 sel*/
        mem_id = DRV_FTM_TCAM_KEY17 - DRV_FTM_TCAM_KEY0;
        if(IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_ACL2), mem_id))
        {
            sel = 0;
        }
        else if(IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_ACL6), mem_id))
        {
            sel = 1;
        }
        SetFlowTcamLookupCtl(V, share2IntfSel_f, &FlowTcamLookupCtl, sel);

        /*ACL3/ACl7 sel*/
        mem_id = DRV_FTM_TCAM_KEY18 - DRV_FTM_TCAM_KEY0;
        if(IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_ACL3), mem_id))
        {
            sel = 0;
        }
        else if(IS_BIT_SET(DRV_FTM_KEY_MEM_BITMAP(DRV_FTM_TCAM_TYPE_IGS_ACL7), mem_id))
        {
            sel = 1;
        }
        SetFlowTcamLookupCtl(V, share3IntfSel_f, &FlowTcamLookupCtl, sel);

    }

    cmd = DRV_IOW(FlowTcamLookupCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &FlowTcamLookupCtl));


    return DRV_E_NONE;
}


#define DRV_FTM_MEM_COUPLE
int32
drv_usw_dynamic_cam_init(uint32 lchip)
{
    uint32 cmd                = 0;
    uint32 field_id        = 0;
    uint32 field_id_en      = 0;
    uint32 field_id_base      = 0;
    uint32 field_id_max_index = 0;
    uint32 field_id_min_index = 0;

    uint32 ds[32] = {0};
    uint8 i = 0;
    uint8 idx = 0;
    uint32 field_val          = 0;
    uint32 tbl_id             = 0;
    uint32 tbl_id_array[5]   = {0};

    uint32 sum               = 0;
    uint8 step                  = 1;
    uint32 bitmap            = 0;
    uint8 cam_type            = 0;
    uint8 sram_type           = 0;
    uint8 unit                    = DRV_FTM_UNIT_8K;

    LpmPipelineIfCtl_m lpm_pipe_line;

    for (cam_type = 0; cam_type < DRV_FTM_DYN_CAM_TYPE_MAX; cam_type++)
    {
        idx = 0;
        field_id        = DRV_FTM_FLD_INVALID;
        field_id_en      = DRV_FTM_FLD_INVALID;
        field_id_base      = DRV_FTM_FLD_INVALID;
        field_id_max_index = DRV_FTM_FLD_INVALID;
        field_id_min_index = DRV_FTM_FLD_INVALID;

        switch (cam_type)
        {
        case DRV_FTM_DYN_CAM_TYPE_LPM_KEY0:
            sram_type                    =  DRV_FTM_SRAM_TBL_LPM_LKP_KEY0;
            if(DRV_IS_DUET2(lchip))
            {
                tbl_id_array[idx++]      = DynamicKeyLpmPipe0IndexCam_t;
                field_id_en                   = DynamicKeyLpmPipe0IndexCam_lpmPipe0CamEnable0_f;
                field_id_base               = DynamicKeyLpmPipe0IndexCam_lpmPipe0BaseAddr0_f;
                field_id_min_index      = DynamicKeyLpmPipe0IndexCam_lpmPipe0MinIndex0_f;
                field_id_max_index     = DynamicKeyLpmPipe0IndexCam_lpmPipe0MaxIndex0_f;
            }
            else
            {
                tbl_id_array[idx++]      = DynamicKeyLpmPipeIf0IndexCam_t;
                field_id_en                   = DynamicKeyLpmPipeIf0IndexCam_lpmPipeIf0CamEnable0_f;
                field_id_base               = DynamicKeyLpmPipeIf0IndexCam_lpmPipeIf0BaseAddr0_f;
                field_id_min_index      = DynamicKeyLpmPipeIf0IndexCam_lpmPipeIf0MinIndex0_f;
                field_id_max_index     = DynamicKeyLpmPipeIf0IndexCam_lpmPipeIf0MaxIndex0_f;
            }
            unit                               = DRV_FTM_UNIT_1K;
            break;
        case DRV_FTM_DYN_CAM_TYPE_LPM_KEY1:
            sram_type                    =  DRV_FTM_SRAM_TBL_LPM_LKP_KEY1;
            if(DRV_IS_DUET2(lchip))
            {
                tbl_id_array[idx++]      = DynamicKeyLpmPipe1IndexCam_t;
                field_id_en                   = DynamicKeyLpmPipe1IndexCam_lpmPipe1CamEnable0_f;
                field_id_base               = DynamicKeyLpmPipe1IndexCam_lpmPipe1BaseAddr0_f;
                field_id_min_index      = DynamicKeyLpmPipe1IndexCam_lpmPipe1MinIndex0_f;
                field_id_max_index     = DynamicKeyLpmPipe1IndexCam_lpmPipe1MaxIndex0_f;
            }
            else
            {
                tbl_id_array[idx++]      = DynamicKeyLpmPipeIf1IndexCam_t;
                field_id_en                   = DynamicKeyLpmPipeIf0IndexCam_lpmPipeIf0CamEnable0_f;
                field_id_base               = DynamicKeyLpmPipeIf0IndexCam_lpmPipeIf0BaseAddr0_f;
                field_id_min_index      = DynamicKeyLpmPipeIf0IndexCam_lpmPipeIf0MinIndex0_f;
                field_id_max_index     = DynamicKeyLpmPipeIf0IndexCam_lpmPipeIf0MaxIndex0_f;
            }
            unit                               = DRV_FTM_UNIT_1K;
            break;
        case DRV_FTM_DYN_CAM_TYPE_LPM_KEY2:
            if(DRV_IS_TSINGMA(lchip))
            {
                sram_type                    =  DRV_FTM_SRAM_TBL_LPM_LKP_KEY2;
                tbl_id_array[idx++]      = DynamicKeyLpmPipeIf2IndexCam_t;
                field_id_en                   = DynamicKeyLpmPipeIf2IndexCam_lpmPipeIf2CamEnable0_f;
                field_id_base               = DynamicKeyLpmPipeIf2IndexCam_lpmPipeIf2BaseAddr0_f;
                field_id_min_index      = DynamicKeyLpmPipeIf2IndexCam_lpmPipeIf2MinIndex0_f;
                field_id_max_index     = DynamicKeyLpmPipeIf2IndexCam_lpmPipeIf2MaxIndex0_f;
                unit                               = DRV_FTM_UNIT_1K;
            }
            break;
        case DRV_FTM_DYN_CAM_TYPE_LPM_KEY3:
            if(DRV_IS_TSINGMA(lchip))
            {
                sram_type                    =  DRV_FTM_SRAM_TBL_LPM_LKP_KEY3;
                tbl_id_array[idx++]      = DynamicKeyLpmPipeIf3IndexCam_t;
                field_id_en                   = DynamicKeyLpmPipeIf3IndexCam_lpmPipeIf3CamEnable0_f;
                field_id_base               = DynamicKeyLpmPipeIf3IndexCam_lpmPipeIf3BaseAddr0_f;
                field_id_min_index      = DynamicKeyLpmPipeIf3IndexCam_lpmPipeIf3MinIndex0_f;
                field_id_max_index     = DynamicKeyLpmPipeIf3IndexCam_lpmPipeIf3MaxIndex0_f;
                unit                               = DRV_FTM_UNIT_1K;
            }

            break;
        case DRV_FTM_DYN_CAM_TYPE_FIB_MACHOST0_KEY:
            sram_type                    =  DRV_IS_DUET2(lchip)? DRV_FTM_SRAM_TBL_FIB0_HASH_KEY:
				                                                 DRV_FTM_SRAM_TBL_MAC_HASH_KEY;
            tbl_id_array[idx++]      =  DynamicKeyHost0KeyIndexCam_t;
            field_id_en                   = DynamicKeyHost0KeyIndexCam_host0KeyCamEnable0_f;
            field_id_base               = DynamicKeyHost0KeyIndexCam_host0KeyBaseAddr0_f;
            field_id_min_index      = DynamicKeyHost0KeyIndexCam_host0KeyMinIndex0_f;
            field_id_max_index     = DynamicKeyHost0KeyIndexCam_host0KeyMaxIndex0_f;
            unit                               = DRV_FTM_UNIT_1K;
            break;

        case DRV_FTM_DYN_CAM_TYPE_FIB_FLOW_AD:
            sram_type                    =  DRV_FTM_SRAM_TBL_FLOW_AD;
            tbl_id_array[idx++]      = DynamicAdDsFlowIndexCam_t;
            field_id_en                   = DynamicAdDsFlowIndexCam_dsFlowCamEnable0_f;
            field_id_base               = DynamicAdDsFlowIndexCam_dsFlowBaseAddr0_f;
            field_id_min_index      = DynamicAdDsFlowIndexCam_dsFlowMinIndex0_f;
            field_id_max_index     = DynamicAdDsFlowIndexCam_dsFlowMaxIndex0_f;
            unit                               = DRV_FTM_UNIT_1K;
            break;


        case DRV_FTM_DYN_CAM_TYPE_IP_FIX_HASH_KEY:
            sram_type                    =  DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY;
            tbl_id_array[idx++]      = DynamicKeyIpfixKeyIndexCam_t;
            field_id_en                   = DynamicKeyIpfixKeyIndexCam_ipfixKeyCamEnable0_f;
            field_id_base               = DynamicKeyIpfixKeyIndexCam_ipfixKeyBaseAddr0_f;
            field_id_min_index      = DynamicKeyIpfixKeyIndexCam_ipfixKeyMinIndex0_f;
            field_id_max_index     = DynamicKeyIpfixKeyIndexCam_ipfixKeyMaxIndex0_f;
            unit                               = DRV_FTM_UNIT_1K;
            break;

        case DRV_FTM_DYN_CAM_TYPE_IP_FIX_HASH_AD:
            sram_type                    =  DRV_FTM_SRAM_TBL_IPFIX_AD;
            tbl_id_array[idx++]      = DynamicAdDsIpfixIndexCam_t;
            field_id_en                   = DynamicAdDsIpfixIndexCam_dsIpfixCamEnable0_f;
            field_id_base               = DynamicAdDsIpfixIndexCam_dsIpfixBaseAddr0_f;
            field_id_min_index      = DynamicAdDsIpfixIndexCam_dsIpfixMinIndex0_f;
            field_id_max_index     = DynamicAdDsIpfixIndexCam_dsIpfixMaxIndex0_f;
            unit                               = DRV_FTM_UNIT_1K;
            break;

        case DRV_FTM_DYN_CAM_TYPE_FIB_L2_AD:
            sram_type                    =  DRV_FTM_SRAM_TBL_DSMAC_AD;
            tbl_id_array[idx++]      = DynamicAdDsMacIndexCam_t;
            field_id_en                   = DynamicAdDsMacIndexCam_dsMacCamEnable0_f;
            field_id_base               = DynamicAdDsMacIndexCam_dsMacBaseAddr0_f;
            field_id_min_index      = DynamicAdDsMacIndexCam_dsMacMinIndex0_f;
            field_id_max_index     = DynamicAdDsMacIndexCam_dsMacMaxIndex0_f;
            unit                               = DRV_FTM_UNIT_1K;
            break;

        case DRV_FTM_DYN_CAM_TYPE_FIB_L3_AD:
            sram_type                    =  DRV_FTM_SRAM_TBL_DSIP_AD;
            tbl_id_array[idx++]      = DynamicAdDsIpIndexCam_t;
            field_id_en                   = DynamicAdDsIpIndexCam_dsIpCamEnable0_f;
            field_id_base               = DynamicAdDsIpIndexCam_dsIpBaseAddr0_f;
            field_id_min_index      = DynamicAdDsIpIndexCam_dsIpMinIndex0_f;
            field_id_max_index     = DynamicAdDsIpIndexCam_dsIpMaxIndex0_f;
            unit                               = DRV_FTM_UNIT_1K;
            break;

        case DRV_FTM_DYN_CAM_TYPE_DS_EDIT:
            sram_type                    =  DRV_FTM_SRAM_TBL_EDIT;
            tbl_id_array[idx++]      = DynamicEditDsEditIndexCam_t;
            field_id_en                   = DynamicEditDsEditIndexCam_dsEditCamEnable0_f;
            field_id_base               = DynamicEditDsEditIndexCam_dsEditBaseAddr0_f;
            field_id_min_index      = DynamicEditDsEditIndexCam_dsEditMinIndex0_f;
            field_id_max_index     = DynamicEditDsEditIndexCam_dsEditMaxIndex0_f;
            unit                               = DRV_FTM_UNIT_1K;
            break;


        case DRV_FTM_DYN_CAM_TYPE_DS_NEXTHOP:
            sram_type                    =  DRV_FTM_SRAM_TBL_NEXTHOP;
            tbl_id_array[idx++]      = DynamicEditDsNextHopIndexCam_t;
            field_id_en                   = DynamicEditDsNextHopIndexCam_dsNextHopCamEnable0_f;
            field_id_base               = DynamicEditDsNextHopIndexCam_dsNextHopBaseAddr0_f;
            field_id_min_index      = DynamicEditDsNextHopIndexCam_dsNextHopMinIndex0_f;
            field_id_max_index     = DynamicEditDsNextHopIndexCam_dsNextHopMaxIndex0_f;
            unit                               = DRV_FTM_UNIT_1K;
            break;

        case DRV_FTM_DYN_CAM_TYPE_DS_MET:
            sram_type                    =  DRV_FTM_SRAM_TBL_MET;
            tbl_id_array[idx++]      = DynamicEditDsMetIndexCam_t;
            field_id_en                   = DynamicEditDsMetIndexCam_dsMetCamEnable0_f;
            field_id_base               = DynamicEditDsMetIndexCam_dsMetBaseAddr0_f;
            field_id_min_index      = DynamicEditDsMetIndexCam_dsMetMinIndex0_f;
            field_id_max_index     = DynamicEditDsMetIndexCam_dsMetMaxIndex0_f;
            unit                               = DRV_FTM_UNIT_1K;
            break;

        case DRV_FTM_DYN_CAM_TYPE_DS_FWD:
            sram_type                    =  DRV_FTM_SRAM_TBL_FWD;
            tbl_id_array[idx++]      = DynamicEditDsFwdIndexCam_t;
            field_id_en                   = DynamicEditDsFwdIndexCam_dsFwdCamEnable0_f;
            field_id_base               = DynamicEditDsFwdIndexCam_dsFwdBaseAddr0_f;
            field_id_min_index      = DynamicEditDsFwdIndexCam_dsFwdMinIndex0_f;
            field_id_max_index     = DynamicEditDsFwdIndexCam_dsFwdMaxIndex0_f;
            unit                               = DRV_FTM_UNIT_1K;
            break;


        case DRV_FTM_DYN_CAM_TYPE_USERID_HASH_AD:
            sram_type                    =  DRV_FTM_SRAM_TBL_USERID_AD;
            tbl_id_array[idx++]      = (DRV_IS_DUET2(lchip))?DynamicAdUserIdAdIndexCam_t:
				                                             DynamicAdUserIdAd0IndexCam_t;
            field_id_en                   = DynamicAdUserIdAdIndexCam_userIdAdCamEnable0_f;
            field_id_base               = DynamicAdUserIdAdIndexCam_userIdAdBaseAddr0_f;
            field_id_min_index      = DynamicAdUserIdAdIndexCam_userIdAdMinIndex0_f;
            field_id_max_index     = DynamicAdUserIdAd0IndexCam_userIdAd0MaxIndex0_f;
            unit                               = DRV_FTM_UNIT_1K;
            break;

        case DRV_FTM_DYN_CAM_TYPE_USERID_HASH_AD1:
            sram_type                    =  DRV_FTM_SRAM_TBL_USERID_AD1;
            tbl_id_array[idx++]      = DynamicAdUserIdAd1IndexCam_t;
            field_id_en                   = DynamicAdUserIdAd1IndexCam_userIdAd1CamEnable0_f;
            field_id_base               = DynamicAdUserIdAd1IndexCam_userIdAd1BaseAddr0_f;
            field_id_min_index      = DynamicAdUserIdAd1IndexCam_userIdAd1MinIndex0_f;
            field_id_max_index     = DynamicAdUserIdAd1IndexCam_userIdAd1MaxIndex0_f;
            unit                               = DRV_FTM_UNIT_1K;
            break;

        case DRV_FTM_DYN_CAM_TYPE_DS_MANAME:
            sram_type                    =  DRV_FTM_SRAM_TBL_OAM_MA_NAME;
            tbl_id_array[idx++]      = DynamicKeyDsMepIndexCam_t;
            field_id_en                   = DynamicKeyDsMepIndexCam_dsMepCamEnable0_f;
            field_id_base               = DynamicKeyDsMepIndexCam_dsMepBaseAddr0_f;
            field_id_min_index      = DynamicKeyDsMepIndexCam_dsMepMinIndex0_f;
            field_id_max_index     = DynamicKeyDsMepIndexCam_dsMepMaxIndex0_f;
            unit                               = DRV_FTM_UNIT_1K;
            break;

        case DRV_FTM_DYN_CAM_TYPE_DS_LM:
            sram_type                    =  DRV_FTM_SRAM_TBL_OAM_LM;
            tbl_id_array[idx++]      = DynamicKeyDsLmIndexCam_t;
            field_id_en                   = DynamicKeyDsLmIndexCam_dsLmCamEnable0_f;
            field_id_base               = DynamicKeyDsLmIndexCam_dsLmBaseAddr0_f;
            field_id_min_index      = DynamicKeyDsLmIndexCam_dsLmMinIndex0_f;
            field_id_max_index     = DynamicKeyDsLmIndexCam_dsLmMaxIndex0_f;
            unit                               = DRV_FTM_UNIT_1K;
            break;

        case DRV_FTM_DYN_CAM_TYPE_DS_APS:
        case DRV_FTM_DYN_CAM_TYPE_EGRESS_OAM_HASH_KEY:
        case DRV_FTM_DYN_CAM_TYPE_USERID_HASH_KEY:
        default:
           continue;
        }

        bitmap = DRV_FTM_TBL_BASE_BITMAP(sram_type);
        tbl_id = tbl_id_array[0];

        sal_memset(ds, 0, sizeof(ds));
        tbl_id = tbl_id_array[0];

        for (i = 0; i < DRV_FTM_MAX_MEM_NUM; i++)
        {
            if (!IS_BIT_SET(bitmap, i))
            {
                continue;
            }
            step = i;
            if (field_id_en != DRV_FTM_FLD_INVALID)
            {
                /*write ds cam enable value*/
                field_id = field_id_en + step;
                field_val = 1;
                DRV_SET_FIELD_V(lchip, tbl_id, field_id,   ds,   field_val );
            }

            if (field_id_base != DRV_FTM_FLD_INVALID)
            {
                /*write ds cam base value*/
                field_id = field_id_base + step;
                field_val = DRV_FTM_TBL_BASE_VAL(sram_type, i);
                field_val = (field_val >> unit);
                DRV_SET_FIELD_V(lchip, tbl_id, field_id,   ds,   field_val );
            }

            if (field_id_min_index != DRV_FTM_FLD_INVALID)
            {
                /*write ds cam min value*/
                field_id = field_id_min_index + step;
                field_val = DRV_FTM_TBL_BASE_MIN(sram_type, i);
                field_val = (field_val >> unit);

                if  (DRV_FTM_SRAM_TBL_USERID_HASH_KEY == sram_type)
                {
                    field_val = 0;
                }


                DRV_SET_FIELD_V(lchip, tbl_id, field_id,   ds,   field_val );

            }

            if (field_id_max_index != DRV_FTM_FLD_INVALID)
            {
                /*write ds cam max value*/
                field_id = field_id_max_index + step;

                /*write ds cam base max index*/
                if (DRV_FTM_SRAM_TBL_OAM_MA_NAME== sram_type)
                {
                    field_val = DRV_FTM_TBL_BASE_MAX(DRV_FTM_SRAM_TBL_OAM_MEP,    i) +
                    DRV_FTM_TBL_BASE_MAX(DRV_FTM_SRAM_TBL_OAM_MA,      i) +
                    DRV_FTM_TBL_BASE_MAX(DRV_FTM_SRAM_TBL_OAM_MA_NAME, i) +
                    DRV_FTM_TBL_BASE_MAX(DRV_FTM_SRAM_TBL_OAM_LM, i);   /*RTL Bug 110501*/
                }
                else
                {
                    field_val = DRV_FTM_TBL_BASE_MAX(sram_type, i);
                }

                field_val = (field_val >> unit);
                DRV_SET_FIELD_V(lchip, tbl_id, field_id,   ds,   field_val );
            }

        }

        for (i = 0; i < idx; i++)
        {
            tbl_id = tbl_id_array[i];
            cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
            DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));
        }

    }

    /*init oam base*/
    field_val = 0;
    field_id_base = OamTblAddrCtl_maNameBaseAddr_f;
    cmd = DRV_IOW(OamTblAddrCtl_t, field_id_base);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    sum += DRV_FTM_TBL_MAX_ENTRY_NUM(DRV_FTM_SRAM_TBL_OAM_MA_NAME);
    field_val = (sum >> 6);
    field_id_base = OamTblAddrCtl_mpBaseAddr_f;
    cmd = DRV_IOW(OamTblAddrCtl_t, field_id_base);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    sum += DRV_FTM_TBL_MAX_ENTRY_NUM(DRV_FTM_SRAM_TBL_OAM_MEP);
    field_val = (sum >> 6);
    field_id_base = OamTblAddrCtl_maBaseAddr_f;
    cmd = DRV_IOW(OamTblAddrCtl_t, field_id_base);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    /*init fib host0 cam bitmap*/
    cmd = DRV_IOW(FibAccelerationCtl_t, FibAccelerationCtl_camDisableBitmap_f);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &g_usw_ftm_master[lchip]->cam_bitmap));

    {
        /*Duet2 force enable couple mode, TM force 100G, share buffer*/
        field_val = (DRV_IS_DUET2(lchip)) ? 1 : 7;
        cmd = DRV_IOW(ResvForce_t, ResvForce_resvForce_f);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));
    }

    /*init TM lpm pipeline*/
    cmd = DRV_IOR(LpmPipelineIfCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &lpm_pipe_line);

    if(g_usw_ftm_master[lchip]->lpm_model == LPM_MODEL_PRIVATE_IPDA_IPSA_HALF)
    {
        SetLpmPipelineIfCtl(V, partitionMode_f, &lpm_pipe_line, 2);
        SetLpmPipelineIfCtl(V, neoLpmCompleteModeEn_f, &lpm_pipe_line, 0);
        SetLpmPipelineIfCtl(V, neoLpmSnakeGroup0En_f, &lpm_pipe_line, DRV_IS_BIT_SET(DRV_FTM_TBL_BASE_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY2),0));
        SetLpmPipelineIfCtl(V, neoLpmSnakeGroup1En_f, &lpm_pipe_line, DRV_IS_BIT_SET(DRV_FTM_TBL_BASE_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY2),1));
        SetLpmPipelineIfCtl(V, neoLpmSnakeGroup2En_f, &lpm_pipe_line, DRV_IS_BIT_SET(DRV_FTM_TBL_BASE_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY2),2));
    }
    else
    {
        SetLpmPipelineIfCtl(V, partitionMode_f, &lpm_pipe_line, 0);
        SetLpmPipelineIfCtl(V, neoLpmCompleteModeEn_f, &lpm_pipe_line, 1);
        SetLpmPipelineIfCtl(V, neoLpmSnakeGroup0En_f, &lpm_pipe_line, DRV_IS_BIT_SET(DRV_FTM_TBL_BASE_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY2),0));
        SetLpmPipelineIfCtl(V, neoLpmSnakeGroup1En_f, &lpm_pipe_line, DRV_IS_BIT_SET(DRV_FTM_TBL_BASE_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY2),1));
        SetLpmPipelineIfCtl(V, neoLpmSnakeGroup2En_f, &lpm_pipe_line, DRV_IS_BIT_SET(DRV_FTM_TBL_BASE_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY2),2));
        SetLpmPipelineIfCtl(V, neoLpmSnakeGroup3En_f, &lpm_pipe_line, DRV_IS_BIT_SET(DRV_FTM_TBL_BASE_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY2),3));
    }

    cmd = DRV_IOW(LpmPipelineIfCtl_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &lpm_pipe_line);
    return DRV_E_NONE;
}

#define DRV_FTM_MEM_COUPLE
int32
drv_duet2_dynamic_arb_init(uint32 lchip)
{
    tbls_id_t table_id = MaxTblId_t;
    uint32 cmd = 0;
    uint32 ds[MAX_ENTRY_WORD] = {0};
    uint32 bit_set = 0;
    uint32 couple_mode      = 0;
    uint32 couple_mode_value = 2;
	uint32 value_1 = 1;
    uint32 value_0 = 0;
#define ArbTable(i) DynamicKeyShareRam##i##Ctl_t
#define ArbTblFld(i, fld) DynamicKeyShareRam##i##Ctl_shareKeyRam##i##fld##_f
#define ArbTblFldCouple(i, fld) DynamicKeyShareRam##i##Ctl_shareKeyRam##i##Local##fld##_f
#define ArbTblFldCoupleMode(i) DynamicKeyShareRam##i##Ctl_shareKeyRam##i##CoupleMode_f


#define GetCoupleModePerMemory(bit_set, tbl_id,couple_mode) \
    do{\
        if(bit_set)\
        {\
        	couple_mode = 0;\
            DRV_IF_ERROR_RETURN(drv_usw_get_dynamic_ram_couple_mode(lchip, tbl_id, &couple_mode));\
        }\
    }while(0)
    /* Share RAM 0 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(0);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(0),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY), DRV_FTM_SRAM0);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(0, IpfixKeyEn),   &bit_set, ds);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(0),   &couple_mode_value, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB0_HASH_KEY), DRV_FTM_SRAM0);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB0_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(0, Host0KeyEn),   &bit_set, ds);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(0),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FLOW_HASH_KEY), DRV_FTM_SRAM0);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(0, FlowKeyEn),   &bit_set, ds);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FLOW_HASH_KEY, couple_mode);
    if(!DRV_IS_DUET2(lchip) && bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(0),   &couple_mode_value, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY0), DRV_FTM_SRAM0);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(0, LpmGrp0En),   &bit_set, ds);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_LPM_LKP_KEY0, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY1), DRV_FTM_SRAM0);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(0, LpmGrp1En),   &bit_set, ds);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_LPM_LKP_KEY1, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(0, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 1 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(1);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(1),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB0_HASH_KEY), DRV_FTM_SRAM1);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB0_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(1, Host0KeyEn),   &bit_set, ds);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(1),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FLOW_HASH_KEY), DRV_FTM_SRAM1);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FLOW_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(1, FlowKeyEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_OAM_MEP), DRV_FTM_SRAM1);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_OAM_MEP, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(1, DsOamEn),   &bit_set, ds);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(1, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 2 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(2);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(2),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB0_HASH_KEY), DRV_FTM_SRAM2);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB0_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(2, Host0KeyEn),   &bit_set, ds);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(2),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FLOW_HASH_KEY), DRV_FTM_SRAM2);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FLOW_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(2, FlowKeyEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY), DRV_FTM_SRAM2);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(2, EgrXcOamEn),   &bit_set, ds);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(2),   &value_0, ds);
    }

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(2, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 3 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(3);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(3),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB0_HASH_KEY), DRV_FTM_SRAM3);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB0_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(3, Host0KeyEn),   &bit_set, ds);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(3),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FLOW_HASH_KEY), DRV_FTM_SRAM3);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FLOW_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(3, FlowKeyEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY0), DRV_FTM_SRAM3);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(3, LpmGrp0En),   &bit_set, ds);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_LPM_LKP_KEY0, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY1), DRV_FTM_SRAM3);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(3, LpmGrp1En),   &bit_set, ds);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_LPM_LKP_KEY1, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(3, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 4 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(4);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(4),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB0_HASH_KEY), DRV_FTM_SRAM4);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB0_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(4, Host0KeyEn),   &bit_set, ds);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(4),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FLOW_HASH_KEY), DRV_FTM_SRAM4);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FLOW_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(4, FlowKeyEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_OAM_MEP), DRV_FTM_SRAM4);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_OAM_MEP, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(4, DsOamEn),   &bit_set, ds);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(4, Couple),   &couple_mode, ds);
    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 5 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(5);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(5),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY), DRV_FTM_SRAM5);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(5, IpfixKeyEn),   &bit_set, ds);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(5),   &couple_mode_value, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB0_HASH_KEY), DRV_FTM_SRAM5);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB0_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(5, Host0KeyEn),   &bit_set, ds);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(5),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY), DRV_FTM_SRAM5);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(5, EgrXcOamEn),   &bit_set, ds);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(5),   &value_0, ds);
    }

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(5, Couple),   &couple_mode, ds);
    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 6 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(6);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(6),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY0), DRV_FTM_SRAM6);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(6, LpmGrp0En),   &bit_set, ds);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_LPM_LKP_KEY0, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY1), DRV_FTM_SRAM6);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(6, LpmGrp1En),   &bit_set, ds);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_LPM_LKP_KEY1, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB1_HASH_KEY), DRV_FTM_SRAM6);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB1_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(6, Host1KeyEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_HASH_KEY), DRV_FTM_SRAM6);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(6, UserIdKeyEn),   &bit_set, ds);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(6),   &value_0, ds);
    }

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(6, Couple),   &couple_mode, ds);
    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 7 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(7);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(7),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY0), DRV_FTM_SRAM7);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(7, LpmGrp0En),   &bit_set, ds);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_LPM_LKP_KEY0, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY1), DRV_FTM_SRAM7);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(7, LpmGrp1En),   &bit_set, ds);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_LPM_LKP_KEY1, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB1_HASH_KEY), DRV_FTM_SRAM7);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB1_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(7, Host1KeyEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_HASH_KEY), DRV_FTM_SRAM7);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(7, UserIdKeyEn),   &bit_set, ds);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(7),   &value_0, ds);
    }

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(7, Couple),   &couple_mode, ds);
    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 8 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(8);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(8),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB1_HASH_KEY), DRV_FTM_SRAM8);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB1_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(8, Host1KeyEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_HASH_KEY), DRV_FTM_SRAM8);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(8, UserIdKeyEn),   &bit_set, ds);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(8),   &value_0, ds);
    }

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(8, Couple),   &couple_mode, ds);
    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 9 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(9);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(9),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB1_HASH_KEY), DRV_FTM_SRAM9);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB1_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(9, Host1KeyEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_HASH_KEY), DRV_FTM_SRAM9);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(9, UserIdKeyEn),   &bit_set, ds);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(9),   &value_0, ds);
    }

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(9, Couple),   &couple_mode, ds);
    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 10 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(10);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(10),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB1_HASH_KEY), DRV_FTM_SRAM10);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB1_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(10, Host1KeyEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_HASH_KEY), DRV_FTM_SRAM10);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_HASH_KEY, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(10, UserIdKeyEn),   &bit_set, ds);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(10),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_OAM_APS), DRV_FTM_SRAM10);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_OAM_APS, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(10, DsApsEn),   &bit_set, ds);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(10, Couple),   &couple_mode, ds);
    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


#undef ArbTable
#undef ArbTblFld
#undef ArbTblFldCouple
#undef ArbTblFldCoupleMode
#define ArbTable(i) DynamicAdShareRam##i##Ctl_t
#define ArbTblFld(i, fld) DynamicAdShareRam##i##Ctl_shareAdRam##i##fld##_f
#define ArbTblFldCouple(i, fld) DynamicAdShareRam##i##Ctl_shareAdRam##i##Local##fld##_f
#define ArbTblFldCoupleMode(i) DynamicAdShareRam##i##Ctl_shareAdRam##i##CoupleMode_f

    /* Share RAM 11 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(0);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(0),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_IPFIX_AD), DRV_FTM_SRAM11);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_IPFIX_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(0, DsIpfixEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSMAC_AD), DRV_FTM_SRAM11);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSMAC_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(0, DsMacEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSIP_AD), DRV_FTM_SRAM11);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSIP_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(0, DsIpEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FLOW_AD), DRV_FTM_SRAM11);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FLOW_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(0, DsFlowEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_AD), DRV_FTM_SRAM11);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(0, UserIdAdEn),   &bit_set, ds);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(0, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 12 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(1);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(1),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_IPFIX_AD), DRV_FTM_SRAM12);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_IPFIX_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(1, DsIpfixEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSMAC_AD), DRV_FTM_SRAM12);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSMAC_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(1, DsMacEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSIP_AD), DRV_FTM_SRAM12);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSIP_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(1, DsIpEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FLOW_AD), DRV_FTM_SRAM12);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FLOW_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(1, DsFlowEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_AD), DRV_FTM_SRAM12);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(1, UserIdAdEn),   &bit_set, ds);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(1, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));



    /* Share RAM 13 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(2);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(2),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSMAC_AD), DRV_FTM_SRAM13);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSMAC_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(2, DsMacEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSIP_AD), DRV_FTM_SRAM13);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSIP_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(2, DsIpEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FLOW_AD), DRV_FTM_SRAM13);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FLOW_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(2, DsFlowEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_AD), DRV_FTM_SRAM13);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(2, UserIdAdEn),   &bit_set, ds);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(2, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));



    /* Share RAM 14 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(3);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(3),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSMAC_AD), DRV_FTM_SRAM14);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSMAC_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(3, DsMacEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSIP_AD), DRV_FTM_SRAM14);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSIP_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(3, DsIpEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FLOW_AD), DRV_FTM_SRAM14);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FLOW_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(3, DsFlowEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_AD), DRV_FTM_SRAM14);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(3, UserIdAdEn),   &bit_set, ds);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(3, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 15 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(4);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(4),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSMAC_AD), DRV_FTM_SRAM15);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSMAC_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(4, DsMacEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSIP_AD), DRV_FTM_SRAM15);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSIP_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(4, DsIpEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FLOW_AD), DRV_FTM_SRAM15);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FLOW_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(4, DsFlowEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_AD), DRV_FTM_SRAM15);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_AD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(4, UserIdAdEn),   &bit_set, ds);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(4, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));



#undef ArbTable
#undef ArbTblFld
#undef ArbTblFldCouple
#undef ArbTblFldCoupleMode
#define ArbTable(i) DynamicEditShareRam##i##Ctl_t
#define ArbTblFld(i, fld) DynamicEditShareRam##i##Ctl_shareEditRam##i##fld##_f
#define ArbTblFldCouple(i, fld) DynamicEditShareRam##i##Ctl_shareEditRam##i##Local##fld##_f
#define ArbTblFldCoupleMode(i) DynamicEditShareRam##i##Ctl_shareEditRam##i##CoupleMode_f


    /* Share RAM 16 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(0);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(0),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_NEXTHOP), DRV_FTM_SRAM16);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_NEXTHOP, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(0, DsNextHopEn),   &bit_set, ds);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_NEXTHOP, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_MET), DRV_FTM_SRAM16);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(0, DsMetEn),   &bit_set, ds);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_MET, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(0, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 17 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(1);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(1),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FWD), DRV_FTM_SRAM17);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FWD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(1, DsFwdEn),   &bit_set, ds);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(1, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 18 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(2);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(2),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_NEXTHOP), DRV_FTM_SRAM18);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_NEXTHOP, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(2, DsNextHopEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_MET), DRV_FTM_SRAM18);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_MET, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(2, DsMetEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FWD), DRV_FTM_SRAM18);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FWD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(2, DsFwdEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_EDIT), DRV_FTM_SRAM18);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_EDIT, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(2, DsEditEn),   &bit_set, ds);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(2, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 19 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(3);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(3),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_NEXTHOP), DRV_FTM_SRAM19);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_NEXTHOP, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(3, DsNextHopEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_MET), DRV_FTM_SRAM19);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_MET, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(3, DsMetEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FWD), DRV_FTM_SRAM19);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FWD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(3, DsFwdEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_EDIT), DRV_FTM_SRAM19);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_EDIT, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(3, DsEditEn),   &bit_set, ds);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(3, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 20 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(4);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(4),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_NEXTHOP), DRV_FTM_SRAM20);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_NEXTHOP, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(4, DsNextHopEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_MET), DRV_FTM_SRAM20);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_MET, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(4, DsMetEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FWD), DRV_FTM_SRAM20);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FWD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(4, DsFwdEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_EDIT), DRV_FTM_SRAM20);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_EDIT, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(4, DsEditEn),   &bit_set, ds);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(4, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));



    /* Share RAM 21 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(5);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(5),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_NEXTHOP), DRV_FTM_SRAM21);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_NEXTHOP, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(5, DsNextHopEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_MET), DRV_FTM_SRAM21);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_MET, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(5, DsMetEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FWD), DRV_FTM_SRAM21);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FWD, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(5, DsFwdEn),   &bit_set, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_EDIT), DRV_FTM_SRAM21);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_EDIT, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFld(5, DsEditEn),   &bit_set, ds);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(5, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    return DRV_E_NONE;
}
int32 drv_tsingma_dynamic_arb_init(uint8 lchip)
{
    tbls_id_t table_id = MaxTblId_t;
    uint32 cmd = 0;
    uint32 ds[MAX_ENTRY_WORD] = {0};
    uint32 bit_set = 0;
    uint32 couple_mode      = 0;
    uint32 value_2 = 2;
	uint32 value_1 = 1;
    uint32 value_0 = 0;

#undef ArbTable
#undef ArbTblFld
#undef ArbTblFldCouple
#undef ArbTblFldCoupleMode
#define ArbTable(i) DynamicKeyShareRam##i##Ctl_t
#define ArbTblFld(i, fld) DynamicKeyShareRam##i##Ctl_shareKeyRam##i##fld##_f
#define ArbTblFldCouple(i, fld) DynamicKeyShareRam##i##Ctl_shareKeyRam##i##Local##fld##_f
#define ArbTblFldCoupleMode(i) DynamicKeyShareRam##i##Ctl_shareKeyRam##i##CoupleMode_f


#define GetCoupleModePerMemory(bit_set, tbl_id,couple_mode) \
    do{\
        if(bit_set)\
        {\
        	couple_mode = 0;\
            DRV_IF_ERROR_RETURN(drv_usw_get_dynamic_ram_couple_mode(lchip, tbl_id, &couple_mode));\
        }\
    }while(0)
    /* Share RAM 0 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(0);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(0),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_MAC_HASH_KEY), DRV_FTM_SRAM0);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_MAC_HASH_KEY, couple_mode);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(0),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB0_HASH_KEY), DRV_FTM_SRAM0);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB0_HASH_KEY, couple_mode);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(0),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FLOW_HASH_KEY), DRV_FTM_SRAM0);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FLOW_HASH_KEY, couple_mode);
    if(!DRV_IS_DUET2(lchip) && bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(0),   &value_2, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY0), DRV_FTM_SRAM0);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_LPM_LKP_KEY0, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY1), DRV_FTM_SRAM0);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_LPM_LKP_KEY1, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(0, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 1 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(1);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(1),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_MAC_HASH_KEY), DRV_FTM_SRAM1);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_MAC_HASH_KEY, couple_mode);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(1),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB0_HASH_KEY), DRV_FTM_SRAM1);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB0_HASH_KEY, couple_mode);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(1),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY0), DRV_FTM_SRAM1);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_LPM_LKP_KEY0, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY1), DRV_FTM_SRAM1);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_LPM_LKP_KEY1, couple_mode);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(1, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 2 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(2);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(2),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_MAC_HASH_KEY), DRV_FTM_SRAM2);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_MAC_HASH_KEY, couple_mode);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(2),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB0_HASH_KEY), DRV_FTM_SRAM2);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB0_HASH_KEY, couple_mode);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(2),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_OAM_MEP), DRV_FTM_SRAM2);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_OAM_MEP, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_HASH_KEY), DRV_FTM_SRAM2);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_HASH_KEY, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(2, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 3 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(3);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(3),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_MAC_HASH_KEY), DRV_FTM_SRAM3);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_MAC_HASH_KEY, couple_mode);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(3),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB0_HASH_KEY), DRV_FTM_SRAM3);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB0_HASH_KEY, couple_mode);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(3),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY), DRV_FTM_SRAM3);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY, couple_mode);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(3),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY0), DRV_FTM_SRAM3);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_LPM_LKP_KEY0, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY1), DRV_FTM_SRAM3);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_LPM_LKP_KEY1, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(3, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 4 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(4);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(4),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_MAC_HASH_KEY), DRV_FTM_SRAM4);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_MAC_HASH_KEY, couple_mode);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(4),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB0_HASH_KEY), DRV_FTM_SRAM4);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB0_HASH_KEY, couple_mode);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(4),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY), DRV_FTM_SRAM4);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY, couple_mode);
    if(bit_set && couple_mode)
    {
        DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(4),   &value_0, ds);
    }

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY0), DRV_FTM_SRAM4);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_LPM_LKP_KEY0, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY1), DRV_FTM_SRAM4);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_LPM_LKP_KEY1, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(4, Couple),   &couple_mode, ds);
    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 5 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(5);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(5),   &value_0, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_MAC_HASH_KEY), DRV_FTM_SRAM5);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_MAC_HASH_KEY, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FIB0_HASH_KEY), DRV_FTM_SRAM5);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FIB0_HASH_KEY, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID1_HASH_KEY), DRV_FTM_SRAM5);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID1_HASH_KEY, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(5, Couple),   &couple_mode, ds);
    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    /* Share RAM 6,7,8,9,10 can not be couple, so do nothing*/

#undef ArbTable
#undef ArbTblFld
#undef ArbTblFldCouple
#undef ArbTblFldCoupleMode
#define ArbTable(i) DynamicAdShareRam##i##Ctl_t
#define ArbTblFld(i, fld) DynamicAdShareRam##i##Ctl_shareAdRam##i##fld##_f
#define ArbTblFldCouple(i, fld) DynamicAdShareRam##i##Ctl_shareAdRam##i##Local##fld##_f
#define ArbTblFldCoupleMode(i) DynamicAdShareRam##i##Ctl_shareAdRam##i##CoupleMode_f

    /* Share RAM 11 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(0);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(0),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSMAC_AD), DRV_FTM_SRAM11);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSMAC_AD, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSIP_AD), DRV_FTM_SRAM11);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSIP_AD, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FLOW_AD), DRV_FTM_SRAM11);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FLOW_AD, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_AD1), DRV_FTM_SRAM11);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_AD1, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(0, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 12 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(1);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(1),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSMAC_AD), DRV_FTM_SRAM12);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSMAC_AD, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSIP_AD), DRV_FTM_SRAM12);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSIP_AD, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_AD), DRV_FTM_SRAM12);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_AD, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(1, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));



    /* Share RAM 13 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(2);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(2),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSMAC_AD), DRV_FTM_SRAM13);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSMAC_AD, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSIP_AD), DRV_FTM_SRAM13);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSIP_AD, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_AD), DRV_FTM_SRAM13);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_AD, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(2, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));



    /* Share RAM 14 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(3);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(3),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSMAC_AD), DRV_FTM_SRAM14);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSMAC_AD, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_DSIP_AD), DRV_FTM_SRAM14);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_DSIP_AD, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_AD), DRV_FTM_SRAM14);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_AD, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(3, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 15 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(4);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(4),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_AD), DRV_FTM_SRAM15);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_AD, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_AD1), DRV_FTM_SRAM15);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_AD1, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(4, Couple),   &couple_mode, ds);
    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    /* Share RAM 16 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(5);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(5),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_AD), DRV_FTM_SRAM16);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_AD, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_USERID_AD1), DRV_FTM_SRAM16);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_USERID_AD1, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(5, Couple),   &couple_mode, ds);
    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

#undef ArbTable
#undef ArbTblFld
#undef ArbTblFldCouple
#undef ArbTblFldCoupleMode
#define ArbTable(i) DynamicEditShareRam##i##Ctl_t
#define ArbTblFld(i, fld) DynamicEditShareRam##i##Ctl_shareEditRam##i##fld##_f
#define ArbTblFldCouple(i, fld) DynamicEditShareRam##i##Ctl_shareEditRam##i##Local##fld##_f
#define ArbTblFldCoupleMode(i) DynamicEditShareRam##i##Ctl_shareEditRam##i##CoupleMode_f


    /* Share RAM 17 can not be couple*/

    /* Share RAM 18 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(1);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(1),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_NEXTHOP), DRV_FTM_SRAM18);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_NEXTHOP, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_MET), DRV_FTM_SRAM18);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_MET, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FWD), DRV_FTM_SRAM18);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FWD, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(1, Couple),   &couple_mode, ds);
    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 19 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(2);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(2),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_NEXTHOP), DRV_FTM_SRAM19);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_NEXTHOP, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_MET), DRV_FTM_SRAM19);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_MET, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FWD), DRV_FTM_SRAM19);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FWD, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_EDIT), DRV_FTM_SRAM19);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_EDIT, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(2, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 20 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(3);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(3),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FWD), DRV_FTM_SRAM20);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FWD, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(3, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 21 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(4);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(4),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_NEXTHOP), DRV_FTM_SRAM21);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_NEXTHOP, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FWD), DRV_FTM_SRAM21);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FWD, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_EDIT), DRV_FTM_SRAM21);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_EDIT, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(4, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /* Share RAM 22 */
    sal_memset(ds, 0, sizeof(ds));
    table_id = ArbTable(5);
    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCoupleMode(5),   &value_1, ds);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_MET), DRV_FTM_SRAM22);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_MET, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_FWD), DRV_FTM_SRAM22);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_FWD, couple_mode);

    bit_set = IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_EDIT), DRV_FTM_SRAM22);
    GetCoupleModePerMemory(bit_set, DRV_FTM_SRAM_TBL_EDIT, couple_mode);

    DRV_IOW_FIELD(lchip, table_id, ArbTblFldCouple(5, Couple),   &couple_mode, ds);

    cmd = DRV_IOW(table_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    return DRV_E_NONE;
}

STATIC int32
drv_usw_dynamic_grp_init(uint8 lchip)
{
    uint32 module_id = 0;
    uint32 ram_id = 0;
    uint32 allocation_num = 0;
    uint32 allocation_id[8] = {0};
    uint32 i, j;
    uint32 ctl_register_id = 0;
    uint32 ctl_field_id_base = 0;
    uint32 ctl_field_id = 0;
    uint32 GroupModeEn = 1;
    uint32 ram_id_offset = 0;
    uint32 ctl_GroupMemIdMax = 0;
    uint32 share_mem_bitmap = 0;
    uint32 cmd = 0;
    uint32 dynamic_cam_entry[MAX_ENTRY_WORD] = {0};

    uint32 ram_id_mapping_register[] = {
        DynamicKeyShareRam0Ctl_t,
        DynamicKeyShareRam1Ctl_t,
        DynamicKeyShareRam2Ctl_t,
        DynamicKeyShareRam3Ctl_t,
        DynamicKeyShareRam4Ctl_t,
        DynamicKeyShareRam5Ctl_t,
        DynamicKeyShareRam6Ctl_t,
        DynamicKeyShareRam7Ctl_t,
        DynamicKeyShareRam8Ctl_t,
        DynamicKeyShareRam9Ctl_t,
        DynamicKeyShareRam10Ctl_t,
        DynamicAdShareRam0Ctl_t,
        DynamicAdShareRam1Ctl_t,
        DynamicAdShareRam2Ctl_t,
        DynamicAdShareRam3Ctl_t,
        DynamicAdShareRam4Ctl_t,
        DynamicAdShareRam5Ctl_t,
        DynamicEditShareRam0Ctl_t,
        DynamicEditShareRam1Ctl_t,
        DynamicEditShareRam2Ctl_t,
        DynamicEditShareRam3Ctl_t,
        DynamicEditShareRam4Ctl_t,
        DynamicEditShareRam5Ctl_t,
    };

    uint32 GroupMemId0[] = {
        DynamicKeyShareRam0Ctl_shareKeyRam0GroupMemId0_f,
        DynamicKeyShareRam1Ctl_shareKeyRam1GroupMemId0_f,
        DynamicKeyShareRam2Ctl_shareKeyRam2GroupMemId0_f,
        DynamicKeyShareRam3Ctl_shareKeyRam3GroupMemId0_f,
        DynamicKeyShareRam4Ctl_shareKeyRam4GroupMemId0_f,
        DynamicKeyShareRam5Ctl_shareKeyRam5GroupMemId0_f,
        DynamicKeyShareRam6Ctl_shareKeyRam6GroupMemId0_f,
        DynamicKeyShareRam7Ctl_shareKeyRam7GroupMemId0_f,
        DynamicKeyShareRam8Ctl_shareKeyRam8GroupMemId0_f,
        DynamicKeyShareRam9Ctl_shareKeyRam9GroupMemId0_f,
        DynamicKeyShareRam10Ctl_shareKeyRam10GroupMemId0_f,
        DynamicAdShareRam0Ctl_shareAdRam0GroupMemId0_f,
        DynamicAdShareRam1Ctl_shareAdRam1GroupMemId0_f,
        DynamicAdShareRam2Ctl_shareAdRam2GroupMemId0_f,
        DynamicAdShareRam3Ctl_shareAdRam3GroupMemId0_f,
        DynamicAdShareRam4Ctl_shareAdRam4GroupMemId0_f,
        DynamicAdShareRam5Ctl_shareAdRam5GroupMemId0_f,
        DynamicEditShareRam0Ctl_shareEditRam0GroupMemId0_f,
        DynamicEditShareRam1Ctl_shareEditRam1GroupMemId0_f,
        DynamicEditShareRam2Ctl_shareEditRam2GroupMemId0_f,
        DynamicEditShareRam3Ctl_shareEditRam3GroupMemId0_f,
        DynamicEditShareRam4Ctl_shareEditRam4GroupMemId0_f,
        DynamicEditShareRam5Ctl_shareEditRam5GroupMemId0_f,
    };

    uint32 GroupEn[] = {
        DynamicKeyShareRam0Ctl_shareKeyRam0GroupModeEn_f,
        DynamicKeyShareRam1Ctl_shareKeyRam1GroupModeEn_f,
        DynamicKeyShareRam2Ctl_shareKeyRam2GroupModeEn_f,
        DynamicKeyShareRam3Ctl_shareKeyRam3GroupModeEn_f,
        DynamicKeyShareRam4Ctl_shareKeyRam4GroupModeEn_f,
        DynamicKeyShareRam5Ctl_shareKeyRam5GroupModeEn_f,
        DynamicKeyShareRam6Ctl_shareKeyRam6GroupModeEn_f,
        DynamicKeyShareRam7Ctl_shareKeyRam7GroupModeEn_f,
        DynamicKeyShareRam8Ctl_shareKeyRam8GroupModeEn_f,
        DynamicKeyShareRam9Ctl_shareKeyRam9GroupModeEn_f,
        DynamicKeyShareRam10Ctl_shareKeyRam10GroupModeEn_f,
        DynamicAdShareRam0Ctl_shareAdRam0GroupModeEn_f,
        DynamicAdShareRam1Ctl_shareAdRam1GroupModeEn_f,
        DynamicAdShareRam2Ctl_shareAdRam2GroupModeEn_f,
        DynamicAdShareRam3Ctl_shareAdRam3GroupModeEn_f,
        DynamicAdShareRam4Ctl_shareAdRam4GroupModeEn_f,
        DynamicAdShareRam5Ctl_shareAdRam5GroupModeEn_f,
        DynamicEditShareRam0Ctl_shareEditRam0GroupModeEn_f,
        DynamicEditShareRam1Ctl_shareEditRam1GroupModeEn_f,
        DynamicEditShareRam2Ctl_shareEditRam2GroupModeEn_f,
        DynamicEditShareRam3Ctl_shareEditRam3GroupModeEn_f,
        DynamicEditShareRam4Ctl_shareEditRam4GroupModeEn_f,
        DynamicEditShareRam5Ctl_shareEditRam5GroupModeEn_f,
    };


     /* init all _shareKeyRam0GroupMemIdx_f to 0xF*/
    for(i = 0; i < 23; i++)
    {
        if(i <= 10)  /* DynamicKeyShareRamXCtl*/
        {
            ctl_GroupMemIdMax = 5;
        }
        else if(i >= 11 && i <= 16)  /* DynamicAdShareRamXCtl*/
        {
            ctl_GroupMemIdMax = 5;
        }
        else  /* DynamicEditShareRamXCtl*/
        {
            ctl_GroupMemIdMax = 4;
        }

        ctl_register_id = ram_id_mapping_register[i];
        ctl_field_id_base = GroupMemId0[i];

        cmd = DRV_IOR(ctl_register_id, DRV_ENTRY_FLAG);
        sal_memset(dynamic_cam_entry, 0, sizeof(dynamic_cam_entry));
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, dynamic_cam_entry));

        for(j = 0; j <= ctl_GroupMemIdMax; j++)
        {
            ctl_field_id = ctl_field_id_base + j;
            ram_id = 0xF;
            DRV_IOW_FIELD(lchip, ctl_register_id, ctl_field_id, &ram_id, dynamic_cam_entry);
        }

        GroupModeEn = 0;

        DRV_IOW_FIELD(lchip, ctl_register_id, GroupEn[i], &GroupModeEn, dynamic_cam_entry);

        cmd = DRV_IOW(ctl_register_id, DRV_ENTRY_FLAG);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, dynamic_cam_entry));

    }

    GroupModeEn = 1;

    for(module_id = DRV_FTM_SRAM_TBL_MAC_HASH_KEY; module_id < DRV_FTM_SRAM_TBL_MAX; module_id++)
    {
        if (module_id == DRV_FTM_SRAM_TBL_MPLS_HASH_KEY ||
            module_id == DRV_FTM_SRAM_TBL_MPLS_HASH_AD ||
        module_id == DRV_FTM_SRAM_TBL_GEM_PORT)
        {
            continue;
        }

        share_mem_bitmap = DRV_FTM_TBL_MEM_BITMAP(module_id);

        allocation_num = 0;
        sal_memset(allocation_id, 0, sizeof(allocation_id));

        for(ram_id = 0; ram_id < 23; ram_id++)
        {
            if(IS_BIT_SET(share_mem_bitmap, ram_id))
            {
                allocation_id[allocation_num] = ram_id;
                allocation_num++;
            }
        }

         /* current module has at least one ram*/
        if(allocation_num > 0)
        {
            if(allocation_id[0] <= 10)  /* DynamicKeyShareRamXCtl*/
            {
                ram_id_offset = 0;
                ctl_GroupMemIdMax = 5;
            }
            else if(allocation_id[0] >= 11 && allocation_id[0] <= 16)  /* DynamicAdShareRamXCtl*/
            {
                ram_id_offset = 11;
                ctl_GroupMemIdMax = 5;
            }
            else if(allocation_id[0] >= 17 && allocation_id[0] <= 22)  /* DynamicEditShareRamXCtl*/
            {
                ram_id_offset = 17;
                ctl_GroupMemIdMax = 4;
            }
            else
            {
                return DRV_E_INVAILD_TYPE;
            }

            for(i = 0; i < allocation_num; i++)
            {
                ctl_register_id = ram_id_mapping_register[allocation_id[i]];
                ctl_field_id_base = GroupMemId0[allocation_id[i]];

                cmd = DRV_IOR(ctl_register_id, DRV_ENTRY_FLAG);
                sal_memset(dynamic_cam_entry, 0, sizeof(dynamic_cam_entry));
                DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, dynamic_cam_entry));

                for (j = 0; j < allocation_num; j++)
                {
                    ctl_field_id = ctl_field_id_base + j;
                    ram_id = allocation_id[j] - ram_id_offset;
                    DRV_IOW_FIELD(lchip, ctl_register_id, ctl_field_id, &ram_id, dynamic_cam_entry);
                }

                DRV_IOW_FIELD(lchip, ctl_register_id, GroupEn[allocation_id[i]], &GroupModeEn, dynamic_cam_entry);

                cmd = DRV_IOW(ctl_register_id, DRV_ENTRY_FLAG);
                DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, dynamic_cam_entry));
            }

        }
    }

    return DRV_E_NONE;
}

STATIC int32
drv_usw_flow_lkup_ctl_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 ds[MAX_ENTRY_WORD] = {0};
    uint32 tbl_id = 0;
    uint32 en = 0;
    uint32 mem_alloced = 0;
    uint32 mac_alloced = 0;
    uint32 mem_size = 0;
    uint8 extern_en = 0;
    uint8 is_mac_key = 0;
    uint8 i  = 0;
    uint32 bitmap = 0;
    uint32 bitmap0 = 0;
    uint32 bitmap1 = 0;
    uint32 sram_array[6] = {0};
    uint32 sram_array0[16] = {0};
    uint8 sram_type = 0;
    uint32 couple_mode = 0;
    uint32 couple_mode0 = 0;
    uint32 couple_mode1 = 0;
    uint8 multi = 1;
    uint32 poly = 0;
    uint8 extern_ad_en = 0;
    FibAccelerationCtl_m fib_acc_ctl;
    FibHost0ExternalHashLookupCtl_m fib_ext_hash_ctl;
    FibEngineLookupCtl_m fib_lkup_ctl;
    ShareBufferCtl_m share_ctl;
    FibEngineExtAddrCfg_m ext_cfg;

    /****************************************************
    Flow lkup ctl
    *****************************************************/
    sal_memset(ds, 0, sizeof(ds));

    mem_alloced = 0;
    tbl_id = FlowHashLookupCtl_t;
    sram_type = DRV_FTM_SRAM_TBL_FLOW_HASH_KEY;
    sal_memset(sram_array, 0, sizeof(sram_array));
    _sys_usw_ftm_get_edram_bitmap(lchip, sram_type,   &bitmap,   sram_array);
    DRV_IF_ERROR_RETURN(drv_usw_get_dynamic_ram_couple_mode(lchip, sram_type, &couple_mode));

    i  = 0;
    en = IS_BIT_SET(bitmap, i);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel0HashEn_f,     ds, en);

    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel0HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel0Couple_f,     ds, couple_mode);
    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    i  = 1;
    en = IS_BIT_SET(bitmap, i);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel1HashEn_f,     ds, en);

    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel1HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel1IndexBase_f,   ds, mem_alloced);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel1Couple_f,     ds, couple_mode);
    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    i  = 2;
    en = IS_BIT_SET(bitmap, i);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel2HashEn_f,     ds, en);

    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel2HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel2IndexBase_f,   ds, mem_alloced);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel2Couple_f,     ds, couple_mode);
    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    i  = 3;
    en = IS_BIT_SET(bitmap, i);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel3HashEn_f,     ds, en);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel3HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel3IndexBase_f,   ds, mem_alloced);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel3Couple_f,     ds, couple_mode);
    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    i  = 4;
    en = IS_BIT_SET(bitmap, i);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel4HashEn_f,     ds, en);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel4HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel4IndexBase_f,   ds, mem_alloced);
    DRV_SET_FIELD_V(lchip, tbl_id,   FlowHashLookupCtl_flowLevel4Couple_f,     ds, couple_mode);
    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    /*userid  lkup ctl*/
    sal_memset(ds, 0, sizeof(ds));
    mem_alloced = 0;
    tbl_id = UserIdHashLookupCtl_t;
    sram_type = DRV_FTM_SRAM_TBL_USERID_HASH_KEY;
    sal_memset(sram_array, 0, sizeof(sram_array));
    _sys_usw_ftm_get_edram_bitmap(lchip, sram_type,   &bitmap,   sram_array);
    DRV_IF_ERROR_RETURN(drv_usw_get_dynamic_ram_couple_mode(lchip, sram_type, &couple_mode));

    i  = 0;
    en = IS_BIT_SET(bitmap, i);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel0HashEn_f,     ds, en);

    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel0HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel0Couple_f,     ds, couple_mode);

    /*TsingMa*/
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl0Level0HashEn_f,     ds, en);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl0Level0HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl0Level0Couple_f,     ds, couple_mode);

    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    i  = 1;
    en = IS_BIT_SET(bitmap, i);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel1Couple_f,     ds, couple_mode);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel1HashEn_f,     ds, en);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel1HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel1IndexBase_f,   ds, mem_alloced);

    /*TsingMa*/
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl0Level1HashEn_f,     ds, en);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl0Level1HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl0Level1IndexBase_f,   ds, mem_alloced);

    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    i  = 2;
    en = IS_BIT_SET(bitmap, i);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel2Couple_f,     ds, couple_mode);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel2HashEn_f,     ds, en);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel2HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel2IndexBase_f,   ds, mem_alloced);

    /*TsingMa*/
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl0Level2HashEn_f,     ds, en);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl0Level2HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl0Level2IndexBase_f,   ds, mem_alloced);

    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    i  = 3;
    en = IS_BIT_SET(bitmap, i);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel3Couple_f,     ds, couple_mode);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel3HashEn_f,     ds, en);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel3HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel3IndexBase_f,   ds, mem_alloced);

    /*TsingMa*/
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl0Level3HashEn_f,     ds, en);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl0Level3HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl0Level3IndexBase_f,   ds, mem_alloced);

    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    i  = 4;
    en = IS_BIT_SET(bitmap, i);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel4Couple_f,     ds, couple_mode);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel4HashEn_f,     ds, en);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel4HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_userIdLevel4IndexBase_f,   ds, mem_alloced);

    /*TsingMa*/
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl0Level4HashEn_f,     ds, en);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl0Level4HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl0Level4IndexBase_f,   ds, mem_alloced);


   /*TsingMa lookup 1*/
    mem_alloced = 0;
    sram_type = DRV_FTM_SRAM_TBL_USERID1_HASH_KEY;
    sal_memset(sram_array, 0, sizeof(sram_array));
    _sys_usw_ftm_get_edram_bitmap(lchip, sram_type,   &bitmap,   sram_array);
    DRV_IF_ERROR_RETURN(drv_usw_get_dynamic_ram_couple_mode(lchip, sram_type, &couple_mode));

    i  = 0;
    en = IS_BIT_SET(bitmap, i);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl1Level0HashEn_f,     ds, en);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl1Level0HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl1Level0Couple_f,     ds, couple_mode);
    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    i  = 1;
    en = IS_BIT_SET(bitmap, i);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl1Level1HashEn_f,     ds, en);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl1Level1HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl1Level1IndexBase_f,   ds, mem_alloced);
    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    i  = 2;
    en = IS_BIT_SET(bitmap, i);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl1Level2HashEn_f,     ds, en);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl1Level2HashType_f,   ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id,   UserIdHashLookupCtl_scl1Level2IndexBase_f,   ds, mem_alloced);
    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;


    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    /*XcOamUserid  lkup ctl*/
    sal_memset(ds, 0, sizeof(ds));
    mem_alloced = 0;
    tbl_id = EgressXcOamHashLookupCtl_t;
    sram_type = DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY;
    sal_memset(sram_array, 0, sizeof(sram_array));
    _sys_usw_ftm_get_edram_bitmap(lchip, sram_type,   &bitmap,   sram_array);
    DRV_IF_ERROR_RETURN(drv_usw_get_dynamic_ram_couple_mode(lchip, sram_type, &couple_mode));

    i = 0;
    en = IS_BIT_SET(bitmap, i);
    DRV_SET_FIELD_V(lchip, tbl_id, EgressXcOamHashLookupCtl_egressXcOamLevel0HashEn_f, ds, en);

    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id, EgressXcOamHashLookupCtl_egressXcOamLevel0HashType_f, ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id, EgressXcOamHashLookupCtl_egressXcOamLevel0Couple_f, ds, couple_mode);
    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    i = 1;
    en = IS_BIT_SET(bitmap, 1);
    DRV_SET_FIELD_V(lchip, tbl_id, EgressXcOamHashLookupCtl_egressXcOamLevel1Couple_f, ds, couple_mode);
    DRV_SET_FIELD_V(lchip, tbl_id, EgressXcOamHashLookupCtl_egressXcOamLevel1HashEn_f, ds, en);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id, EgressXcOamHashLookupCtl_egressXcOamLevel1HashType_f, ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id, EgressXcOamHashLookupCtl_egressXcOamLevel1IndexBase_f, ds, mem_alloced);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /*Host0lkup ctl*/
    sal_memset(ds, 0, sizeof(ds));
    mem_alloced = 0;
    tbl_id = FibHost0HashLookupCtl_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    cmd = DRV_IOR(FibAccelerationCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_acc_ctl));

    sram_type = DRV_FTM_SRAM_TBL_MAC_HASH_KEY;
    sal_memset(sram_array0, 0, sizeof(sram_array0));
    _sys_usw_ftm_get_edram_bitmap(lchip, sram_type,   &bitmap0,   sram_array0);
    DRV_IF_ERROR_RETURN(drv_usw_get_dynamic_ram_couple_mode(lchip, sram_type, &couple_mode0));

    sram_type = DRV_FTM_SRAM_TBL_FIB0_HASH_KEY;
    sal_memset(sram_array, 0, sizeof(sram_array));
    _sys_usw_ftm_get_edram_bitmap(lchip, sram_type,   &bitmap1,   sram_array);
    DRV_IF_ERROR_RETURN(drv_usw_get_dynamic_ram_couple_mode(lchip, sram_type, &couple_mode1));

    bitmap = (bitmap0 | bitmap1);

    i  = 0;
    en = IS_BIT_SET(bitmap, i);
    is_mac_key = IS_BIT_SET(bitmap0, i);
    couple_mode = is_mac_key?couple_mode0:couple_mode1;
    multi = (couple_mode?2:1);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level0HashEn_f, ds, en);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, DRV_FTM_SRAM0, couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level0HashType_f, ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0IpLookupAgingIndexBase_f, ds, (32+136*1024));
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level0Couple_f, ds, couple_mode);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level0MacKeyEn_f, ds, is_mac_key);

    mem_size += (DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*2;
    mem_alloced += en? (!is_mac_key?((DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*multi):0):0;
    mac_alloced += en? (is_mac_key?((DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*multi):0):0;
    SetFibAccelerationCtl(V, demarcationIndex0_f, &fib_acc_ctl, mac_alloced + 32); /*level 0 max*/
    SetFibAccelerationCtl(V, host0Level0MacHashEn_f, &fib_acc_ctl, is_mac_key);
    SetFibAccelerationCtl(V, host0Level0SizeMode_f, &fib_acc_ctl, couple_mode);
    i  = 1;
    en = IS_BIT_SET(bitmap, i);
    is_mac_key = IS_BIT_SET(bitmap0, i);
    couple_mode = is_mac_key?couple_mode0:couple_mode1;
    multi = (couple_mode?2:1);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level1HashEn_f, ds, en);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, DRV_FTM_SRAM0 + i, couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level1HashType_f, ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level1IndexBase_f, ds, is_mac_key?mac_alloced:mem_alloced);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level1AgingIndexBase_f, ds, mem_size);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level1Couple_f, ds, couple_mode);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level1MacKeyEn_f, ds, is_mac_key);

    mem_size += (DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*2;
    mem_alloced += en? (!is_mac_key?((DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*multi):0):0;
    mac_alloced += en? (is_mac_key?((DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*multi):0):0;
    SetFibAccelerationCtl(V, host0Level1MacHashEn_f, &fib_acc_ctl, is_mac_key);
    SetFibAccelerationCtl(V, demarcationIndex1_f, &fib_acc_ctl, mac_alloced + 32);
    SetFibAccelerationCtl(V, host0Level1SizeMode_f, &fib_acc_ctl, couple_mode);

    i  = 2;
    en = IS_BIT_SET(bitmap, i);
    is_mac_key = IS_BIT_SET(bitmap0, i);
    couple_mode = is_mac_key?couple_mode0:couple_mode1;
    multi = (couple_mode?2:1);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level2HashEn_f, ds, en);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, DRV_FTM_SRAM0 + i, couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level2HashType_f, ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level2IndexBase_f, ds, is_mac_key?mac_alloced:mem_alloced);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level2AgingIndexBase_f, ds, mem_size);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level2Couple_f, ds, couple_mode);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level2MacKeyEn_f, ds, is_mac_key);

    mem_size += (DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*2;
    mem_alloced += en? (!is_mac_key?((DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*multi):0):0;
    mac_alloced += en? (is_mac_key?((DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*multi):0):0;
    SetFibAccelerationCtl(V, host0Level2MacHashEn_f, &fib_acc_ctl, is_mac_key);
    SetFibAccelerationCtl(V, demarcationIndex2_f, &fib_acc_ctl, mac_alloced + 32);
    SetFibAccelerationCtl(V, host0Level2SizeMode_f, &fib_acc_ctl, couple_mode);

    i  = 3;
    en = IS_BIT_SET(bitmap, i);
    is_mac_key = IS_BIT_SET(bitmap0, i);
    couple_mode = is_mac_key?couple_mode0:couple_mode1;
    multi = (couple_mode?2:1);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level3HashEn_f, ds, en);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, DRV_FTM_SRAM0 + i, couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level3HashType_f, ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level3IndexBase_f, ds, is_mac_key?mac_alloced:mem_alloced);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level3AgingIndexBase_f, ds, mem_size);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level3Couple_f, ds, couple_mode);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level3MacKeyEn_f, ds, is_mac_key);

    mem_size += (DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*2;
    mem_alloced += en? (!is_mac_key?((DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*multi):0):0;
    mac_alloced += en? (is_mac_key?((DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*multi):0):0;
    SetFibAccelerationCtl(V, host0Level3MacHashEn_f, &fib_acc_ctl, is_mac_key);
    SetFibAccelerationCtl(V, demarcationIndex3_f, &fib_acc_ctl, mac_alloced + 32);
    SetFibAccelerationCtl(V, host0Level3SizeMode_f, &fib_acc_ctl, couple_mode);

    i  = 4;
    en = IS_BIT_SET(bitmap, i);
    is_mac_key = IS_BIT_SET(bitmap0, i);
    couple_mode = is_mac_key?couple_mode0:couple_mode1;
    multi = (couple_mode?2:1);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level4HashEn_f, ds, en);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, DRV_FTM_SRAM0 + i, couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level4HashType_f, ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level4IndexBase_f, ds, is_mac_key?mac_alloced:mem_alloced);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level4AgingIndexBase_f, ds,  mem_size);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level4Couple_f, ds, couple_mode);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level4MacKeyEn_f, ds, is_mac_key);

    mem_size += (DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*2;
    mem_alloced += en? (!is_mac_key?((DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*multi):0):0;
    mac_alloced += en? (is_mac_key?((DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*multi):0):0;
    SetFibAccelerationCtl(V, host0Level4MacHashEn_f, &fib_acc_ctl, is_mac_key);
    SetFibAccelerationCtl(V, demarcationIndex4_f, &fib_acc_ctl, mac_alloced + 32);
    SetFibAccelerationCtl(V, host0Level4SizeMode_f, &fib_acc_ctl, couple_mode);

    i  = 5;
    en = IS_BIT_SET(bitmap, i);
    is_mac_key = IS_BIT_SET(bitmap0, i);
    couple_mode = is_mac_key?couple_mode0:couple_mode1;
    multi = (couple_mode?2:1);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level5HashEn_f, ds, en);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, DRV_FTM_SRAM0 + i, couple_mode, &poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level5HashType_f, ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level5IndexBase_f, ds, is_mac_key?mac_alloced:mem_alloced );
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level5AgingIndexBase_f, ds, mem_size);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level5Couple_f, ds, couple_mode);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost0HashLookupCtl_fibHost0Level5MacKeyEn_f, ds, is_mac_key);

    mem_size += (DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*2;
    mem_alloced += en? (!is_mac_key?((DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*multi):0):0;
    mac_alloced += en? (is_mac_key?((DRV_FTM_MEM_MAX_ENTRY_NUM(DRV_FTM_SRAM0 + i))*multi):0):0;
    SetFibAccelerationCtl(V, host0Level5MacHashEn_f, &fib_acc_ctl, is_mac_key);
    SetFibAccelerationCtl(V, demarcationIndex5_f, &fib_acc_ctl, mac_alloced + 32);
    SetFibAccelerationCtl(V, host0Level5SizeMode_f, &fib_acc_ctl, couple_mode);

    i  = DRV_USW_FTM_EXTERN_MEM_ID_BASE;
    extern_en = IS_BIT_SET(bitmap0, i);
    couple_mode = couple_mode0;
    SetFibAccelerationCtl(V, host0HashSizeMode_f, &fib_acc_ctl, couple_mode);
    /*TsingMa*/
    SetFibAccelerationCtl(V, externalMacKeyIndexBase_f, &fib_acc_ctl,  mac_alloced + 32); /*include cam, AgingInex To KeyIndex*/
    SetFibAccelerationCtl(V, externalMacAgingIndexBase_f, &fib_acc_ctl,  0x26080);
    SetFibAccelerationCtl(V, externalMacIndexBase_f, &fib_acc_ctl, mac_alloced); /*not include cam, keyIndex To AgingInex*/
    SetFibAccelerationCtl(V, host0ExternalHashEn_f, &fib_acc_ctl, extern_en);
    SetFibAccelerationCtl(V, host0ExternalHashSizeMode_f, &fib_acc_ctl, couple_mode);

    SetFibAccelerationCtl(V, macNum_f, &fib_acc_ctl, DRV_FTM_TBL_MAX_ENTRY_NUM((DRV_IS_DUET2(lchip)?DRV_FTM_SRAM_TBL_FIB0_HASH_KEY:DRV_FTM_SRAM_TBL_MAC_HASH_KEY))+32);
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    cmd = DRV_IOW(FibAccelerationCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_acc_ctl));


    cmd = DRV_IOR(FibHost0ExternalHashLookupCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_ext_hash_ctl));
    SetFibHost0ExternalHashLookupCtl(V, externalAgingIndexBase_f, &fib_ext_hash_ctl, 0x26080); /*Static value*/
    SetFibHost0ExternalHashLookupCtl(V, externalKeyIndexBase_f, &fib_ext_hash_ctl, mac_alloced + 32);

    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array0[i], couple_mode, &poly);
    SetFibHost0ExternalHashLookupCtl(V, externalLevel0HashType_f, &fib_ext_hash_ctl, poly);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array0[i+1], couple_mode, &poly);
    SetFibHost0ExternalHashLookupCtl(V, externalLevel1HashType_f, &fib_ext_hash_ctl, poly);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array0[i+2], couple_mode, &poly);
    SetFibHost0ExternalHashLookupCtl(V, externalLevel2HashType_f, &fib_ext_hash_ctl, poly);
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array0[i+3], couple_mode, &poly);
    SetFibHost0ExternalHashLookupCtl(V, externalLevel3HashType_f, &fib_ext_hash_ctl, poly);

    SetFibHost0ExternalHashLookupCtl(V, externalMemoryCouple_f, &fib_ext_hash_ctl, couple_mode);
    cmd = DRV_IOW(FibHost0ExternalHashLookupCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_ext_hash_ctl));


    /*TsingMa*/
    cmd = DRV_IOR(FibEngineLookupCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_lkup_ctl));
    SetFibEngineLookupCtl(V, internalLookupEn_f, &fib_lkup_ctl, (bitmap0&0x3F)?1:0);
    SetFibEngineLookupCtl(V, externalLookupEn_f, &fib_lkup_ctl, extern_en);
    cmd = DRV_IOW(FibEngineLookupCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_lkup_ctl));

    cmd = DRV_IOR(ShareBufferCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &share_ctl));
    SetShareBufferCtl(V, cfgShareTableEn_f, &share_ctl, extern_en);

    /*ShareBuffer coupleMode MUST set to 1*/
    couple_mode = (extern_en == 0)? 1: couple_mode;

    SetShareBufferCtl(V, shareBufRam0Couple_f, &share_ctl, couple_mode);
    SetShareBufferCtl(V, shareBufRam1Couple_f, &share_ctl, couple_mode);
    SetShareBufferCtl(V, shareBufRam2Couple_f, &share_ctl, couple_mode);
    SetShareBufferCtl(V, shareBufRam3Couple_f, &share_ctl, couple_mode);

    if (extern_en == 0)
    {
        SetShareBufferCtl(V, shareBufRam4Couple_f, &share_ctl, couple_mode);
        SetShareBufferCtl(V, shareBufRam5Couple_f, &share_ctl, couple_mode);
    }

    cmd = DRV_IOR(FibEngineExtAddrCfg_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ext_cfg));

    /*DsMac ShareBuffer*/
    sal_memset(sram_array0, 0, sizeof(sram_array0));
    sram_type = DRV_FTM_SRAM_TBL_DSMAC_AD;
    _sys_usw_ftm_get_edram_bitmap(lchip, sram_type,   &bitmap0,   sram_array0);
    DRV_IF_ERROR_RETURN(drv_usw_get_dynamic_ram_couple_mode(lchip, sram_type, &couple_mode));

    if (IS_BIT_SET(bitmap0, DRV_USW_FTM_EXTERN_MEM_ID_BASE))
    {
        extern_ad_en = 1;
        SetFibEngineExtAddrCfg(V, cfgDsExtIsMac_f, &ext_cfg, 1);

        mem_alloced = 0;
        for (i = 0; i < 6; i++)
        {
            mem_alloced   += DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array0[i]);
        }

        SetFibEngineExtAddrCfg(V, cfgDsMacExtBaseIndex_f, &ext_cfg, mem_alloced);
        SetShareBufferCtl(V, shareBufRam4Couple_f, &share_ctl, couple_mode);
        SetShareBufferCtl(V, shareBufRam5Couple_f, &share_ctl, couple_mode);
    }

    /*DsIp ShareBuffer*/
    sal_memset(sram_array0, 0, sizeof(sram_array0));
    sram_type = DRV_FTM_SRAM_TBL_DSIP_AD;
    _sys_usw_ftm_get_edram_bitmap(lchip, sram_type,   &bitmap0,   sram_array0);
    DRV_IF_ERROR_RETURN(drv_usw_get_dynamic_ram_couple_mode(lchip, sram_type, &couple_mode));

    if (IS_BIT_SET(bitmap0, DRV_USW_FTM_EXTERN_MEM_ID_BASE))
    {
        extern_ad_en = 1;
        SetFibEngineExtAddrCfg(V, cfgDsExtIsMac_f, &ext_cfg, 0);

        mem_alloced = 0;
        for (i = 0; i < 6; i++)
        {
            mem_alloced   += DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array0[i]);
        }
        SetFibEngineExtAddrCfg(V, cfgDsIpExtBaseIndex_f, &ext_cfg, mem_alloced);
        SetShareBufferCtl(V, shareBufRam4Couple_f, &share_ctl, couple_mode);
        SetShareBufferCtl(V, shareBufRam5Couple_f, &share_ctl, couple_mode);
    }

    SetFibEngineExtAddrCfg(V, cfgDsExtLkupEn_f, &ext_cfg, extern_ad_en);
    SetFibEngineExtAddrCfg(V, cfgHost0MacExtKeyBaseIndex_f, &ext_cfg, mac_alloced + 32);

    cmd = DRV_IOW(ShareBufferCtl_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &share_ctl));
    cmd = DRV_IOW(FibEngineExtAddrCfg_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ext_cfg));

    /*DsNexthop ShareBuffer*/
    sal_memset(sram_array0, 0, sizeof(sram_array0));
    sram_type = DRV_FTM_SRAM_TBL_NEXTHOP;
    _sys_usw_ftm_get_edram_bitmap(lchip, sram_type,   &bitmap0,   sram_array0);

    cmd = DRV_IOR(FibEngineExtAddrCfg_t, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ext_cfg));
    if (IS_BIT_SET(bitmap0, DRV_USW_FTM_EXTERN_MEM_ID_BASE))
    {
        uint32 enable = 1;
        cmd = DRV_IOW(EpeHdrAdjustMiscCtl_t, EpeHdrAdjustMiscCtl_cfgDsNextHopExtLkupEn_f);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &enable));

        mem_alloced = 0;
        for (i = 0; i < 6; i++)
        {
            mem_alloced += DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array0[i]);
        }
        cmd = DRV_IOW(EpeHdrAdjNextHopExtBaseCtl_t, EpeHdrAdjNextHopExtBaseCtl_dsNextHopExtBase_f);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &mem_alloced));
    }


    /*Host1lkup ctl*/
    sal_memset(ds, 0, sizeof(ds));
    mem_alloced = 0;
    tbl_id = FibHost1HashLookupCtl_t;
    sram_type = DRV_FTM_SRAM_TBL_FIB1_HASH_KEY;
    sal_memset(sram_array, 0, sizeof(sram_array));
    _sys_usw_ftm_get_edram_bitmap(lchip, sram_type,   &bitmap,   sram_array);
    DRV_IF_ERROR_RETURN(drv_usw_get_dynamic_ram_couple_mode(lchip, sram_type, &couple_mode));


    i  = 0;
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);

    en = IS_BIT_SET(bitmap, i);

    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1AgingIndexOffset_f, ds, 139392);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level0HashEn_f, ds, en);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level0HashType_f, ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level0Couple_f, ds, couple_mode);
    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    i  = 1;
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    en = IS_BIT_SET(bitmap, i);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level1HashEn_f, ds, en);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level1HashType_f, ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level1Couple_f, ds, couple_mode);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level1IndexBase_f, ds, mem_alloced);

    /*TsingMa*/
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level1AgingIndexBase_f, ds, mem_alloced);
    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    i  = 2;
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    en = IS_BIT_SET(bitmap, i);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level2HashEn_f, ds, en);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level2HashType_f, ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level2Couple_f, ds, couple_mode);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level2IndexBase_f, ds, mem_alloced);
    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    i  = 3;
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    en = IS_BIT_SET(bitmap, i);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level3HashEn_f, ds, en);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level3HashType_f, ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level3Couple_f, ds, couple_mode);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level3IndexBase_f, ds, mem_alloced);
    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    i  = 4;
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    en = IS_BIT_SET(bitmap, i);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level4HashEn_f, ds, en);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level4HashType_f, ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level4Couple_f, ds, couple_mode);
    DRV_SET_FIELD_V(lchip, tbl_id, FibHost1HashLookupCtl_fibHost1Level4IndexBase_f, ds, mem_alloced);
    mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /*IPFix hash ctl*/
    sal_memset(ds, 0, sizeof(ds));
    tbl_id = IpfixHashLookupCtl_t;
    sram_type = DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY;
    _sys_usw_ftm_get_edram_bitmap(lchip, sram_type,   &bitmap,   sram_array);
    DRV_IF_ERROR_RETURN(drv_usw_get_dynamic_ram_couple_mode(lchip, sram_type, &couple_mode));

    DRV_SET_FIELD_V(lchip, tbl_id, IpfixHashLookupCtl_ipfixMemCouple_f,   ds,   couple_mode );
    i  = 0;
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    en = IS_BIT_SET(bitmap, i);

    DRV_SET_FIELD_V(lchip, tbl_id, IpfixHashLookupCtl_ipfixLevel0HashEn_f,   ds,   en );
    DRV_SET_FIELD_V(lchip, tbl_id, IpfixHashLookupCtl_ipfixLevel0HashType_f, ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id, IpfixHashLookupCtl_ipfixLevel1HashEn_f,   ds,   en );
    DRV_SET_FIELD_V(lchip, tbl_id, IpfixHashLookupCtl_ipfixLevel1HashType_f, ds, poly);

    i  = 1;
    _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
    en = IS_BIT_SET(bitmap, i);
    DRV_SET_FIELD_V(lchip, tbl_id, IpfixHashLookupCtl_ipfixLevel2HashEn_f,   ds,   en );
    DRV_SET_FIELD_V(lchip, tbl_id, IpfixHashLookupCtl_ipfixLevel2HashType_f, ds, poly);
    DRV_SET_FIELD_V(lchip, tbl_id, IpfixHashLookupCtl_ipfixLevel3HashEn_f,   ds,   en );
    DRV_SET_FIELD_V(lchip, tbl_id, IpfixHashLookupCtl_ipfixLevel3HashType_f, ds, poly);
    if(IS_BIT_SET(bitmap, 0) && IS_BIT_SET(bitmap, 1))
    {
        DRV_SET_FIELD_V(lchip, tbl_id, IpfixHashLookupCtl_ipfixKeyIndexBase_f,   ds,  DRV_FTM_TBL_MEM_ENTRY_NUM(DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY, DRV_FTM_SRAM0));
    }
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));



    /*Mpls hash ctl*/
    sal_memset(ds, 0, sizeof(ds));
    tbl_id = MplsHashLookupCtl_t;
    if (g_usw_ftm_master[lchip]->mpls_mode)
    {
        mem_alloced = 0;
        sram_type = DRV_FTM_SRAM_TBL_MPLS_HASH_KEY;
        _sys_usw_ftm_get_edram_bitmap(lchip, sram_type,   &bitmap,   sram_array);
        DRV_IF_ERROR_RETURN(drv_usw_get_dynamic_ram_couple_mode(lchip, sram_type, &couple_mode));

        i  = 0;
        _sys_usw_ftm_get_hash_type(lchip, sram_type, sram_array[i], couple_mode, &poly);
        en = IS_BIT_SET(bitmap, i);

        mem_alloced  += en?DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, sram_array[i]):0;

        DRV_SET_FIELD_V(lchip, tbl_id, MplsHashLookupCtl_mplsLevel0HashEn_f,   ds,   en );
        DRV_SET_FIELD_V(lchip, tbl_id, MplsHashLookupCtl_mplsLevel1HashEn_f,   ds,   en );
        DRV_SET_FIELD_V(lchip, tbl_id, MplsHashLookupCtl_mplsLevel1IndexBase_f, ds, mem_alloced);
        DRV_SET_FIELD_V(lchip, tbl_id, MplsHashLookupCtl_mplsHashLookupCoupleEn_f, ds, couple_mode);
        DRV_SET_FIELD_V(lchip, tbl_id, MplsHashLookupCtl_mplsAdMode_f, ds, 0);
    }
    else
    {
        DRV_SET_FIELD_V(lchip, tbl_id, MplsHashLookupCtl_mplsLevel0HashEn_f,   ds,   0 );
        DRV_SET_FIELD_V(lchip, tbl_id, MplsHashLookupCtl_mplsLevel1HashEn_f,   ds,   0 );
        DRV_SET_FIELD_V(lchip, tbl_id, MplsHashLookupCtl_mplsLevel1IndexBase_f, ds, 0);
        DRV_SET_FIELD_V(lchip, tbl_id, MplsHashLookupCtl_mplsHashLookupCoupleEn_f, ds, 1);
        DRV_SET_FIELD_V(lchip, tbl_id, MplsHashLookupCtl_mplsAdMode_f, ds, 1);
    }

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));


    /*lpm ctl*/
    sal_memset(ds, 0, sizeof(ds));
    tbl_id = FibEngineLookupCtl_t;
    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

	if(g_usw_ftm_master[lchip]->napt_enable)
	{
		DRV_SET_FIELD_V(lchip, tbl_id, FibEngineLookupCtl_ipv4NatSaPortLookupEn_f, ds, 1);
	    DRV_SET_FIELD_V(lchip, tbl_id, FibEngineLookupCtl_ipv4NatDaPortLookupEn_f, ds, 1);
	}
    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ds));

    return DRV_E_NONE;
}

#define NUM_16K                         (16 * 1024)
#define NUM_32K                         (32 * 1024)
#define NUM_64K                         (64 * 1024)

STATIC int32
drv_usw_fib_lkup_ctl_init(uint8 lchip)
{
    uint32 cmd = 0;
    FibAccelerationCtl_m fib_acc_ctl;
    uint32 tbl_id = 0;
    uint8  i  = 0;
    uint32 bitmap = 0;
    uint32 sram_array[6];
    uint8  sram_type = 0;
    uint32 sum = 0;
    uint32 mac_max_index[6] = {0};
    uint32 level_couple[6] = {0};
    uint32 couple_mode = 0;
    uint8  step = 0;

    /*Host0lkup ctl*/
    sal_memset(&fib_acc_ctl, 0, sizeof(FibAccelerationCtl_m));


    tbl_id = FibAccelerationCtl_t;
    sram_type = DRV_FTM_SRAM_TBL_FIB0_HASH_KEY;
    _sys_usw_ftm_get_edram_bitmap(lchip, sram_type, &bitmap, sram_array);
     /*-bitmap = DRV_FTM_TBL_MEM_BITMAP(tbl_id);*/
    DRV_IF_ERROR_RETURN(drv_usw_get_dynamic_ram_couple_mode(lchip, sram_type, &couple_mode));

    cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_acc_ctl));

    /*Support 6 level fibhostAcc*/
    sum = 32;  /*cam size*/
    step = FibAccelerationCtl_host0Level1HashEn_f-FibAccelerationCtl_host0Level0HashEn_f;
    for (i = 0; i < 6; i++)
    {
        if (IS_BIT_SET(bitmap, i))
        {
            DRV_SET_FIELD_V(lchip, tbl_id, FibAccelerationCtl_host0Level0HashEn_f + i*step, &fib_acc_ctl, 1);
            sum += (i < 3?NUM_32K:NUM_16K) >> !couple_mode;
            level_couple[i] = couple_mode;
        }
        mac_max_index[i] = sum;
    }

    SetFibAccelerationCtl(V, demarcationIndex0_f, &fib_acc_ctl, mac_max_index[0]);
    SetFibAccelerationCtl(V, demarcationIndex1_f, &fib_acc_ctl, mac_max_index[1]);
    SetFibAccelerationCtl(V, demarcationIndex2_f, &fib_acc_ctl, mac_max_index[2]);
    SetFibAccelerationCtl(V, demarcationIndex3_f, &fib_acc_ctl, mac_max_index[3]);
    SetFibAccelerationCtl(V, demarcationIndex4_f, &fib_acc_ctl, mac_max_index[4]);
    SetFibAccelerationCtl(V, demarcationIndex5_f, &fib_acc_ctl, mac_max_index[5]);
    SetFibAccelerationCtl(V, host0Level0SizeMode_f, &fib_acc_ctl, level_couple[0]);
    SetFibAccelerationCtl(V, host0Level1SizeMode_f, &fib_acc_ctl, level_couple[1]);
    SetFibAccelerationCtl(V, host0Level2SizeMode_f, &fib_acc_ctl, level_couple[2]);
    SetFibAccelerationCtl(V, host0Level3SizeMode_f, &fib_acc_ctl, level_couple[3]);
    SetFibAccelerationCtl(V, host0Level4SizeMode_f, &fib_acc_ctl, level_couple[4]);
    SetFibAccelerationCtl(V, host0Level5SizeMode_f, &fib_acc_ctl, level_couple[5]);
    SetFibAccelerationCtl(V, host0HashSizeMode_f, &fib_acc_ctl, couple_mode);

    cmd = DRV_IOW(tbl_id, DRV_ENTRY_FLAG);
    DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &fib_acc_ctl));

    return DRV_E_NONE;
}
static INLINE int32
_drv_usw_ftm_get_cam_info_by_type(uint8 lchip, uint32 cam_type, uint32* table_id, uint32* cam_num)
{
    switch(cam_type)
    {
        case DRV_FTM_CAM_TYPE_FIB_HOST0:
            *table_id = DsFibHost0MacHashKey_t;
            *cam_num = 32;
            break;
        case DRV_FTM_CAM_TYPE_FIB_HOST1:
            *table_id = DsFibHost1Ipv4McastHashKey_t;
            *cam_num = 32;
            break;
        case DRV_FTM_CAM_TYPE_SCL:
            *table_id = DsUserIdIpv4PortHashKey_t;
            *cam_num = (DRV_IS_DUET2(lchip)) ?32:0;
            break;
        case DRV_FTM_CAM_TYPE_XC:
            *table_id = DsEgressXcOamSvlanPortHashKey_t;
            *cam_num = 128;
            break;
        case DRV_FTM_CAM_TYPE_FLOW:
            *table_id = DsFlowL2HashKey_t;
            *cam_num = 32;
            break;
        default:
            return DRV_E_INVALID_ACCESS_TYPE;
    }
    return DRV_E_NONE;
}
STATIC int32
_drv_usw_ftm_get_cam_info(uint8 lchip, drv_ftm_info_detail_t* p_ftm_info)
{
    uint16 index;
    uint32 table_id = 0;
    uint32 cam_num = 0;
    uint8  hash_type = 0;

    drv_acc_in_t  in;
    drv_acc_out_t out;

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));

    in.type = DRV_ACC_TYPE_LOOKUP;
    in.op_type = DRV_ACC_OP_BY_INDEX;

    _drv_usw_ftm_get_cam_info_by_type(lchip, p_ftm_info->tbl_type, &table_id, &cam_num);

    for(index=0; index < cam_num; index++)
    {
        switch(p_ftm_info->tbl_type)
        {
            case DRV_FTM_CAM_TYPE_FIB_HOST0:
            case DRV_FTM_CAM_TYPE_FIB_HOST1:
            case DRV_FTM_CAM_TYPE_SCL:
                in.tbl_id = table_id;
                in.index = index;
                drv_acc_api(lchip, &in, &out);
                /* for all table, valid bit is in {0,0,1}*/
                if(out.data[0] && 0x01)
                {
                    p_ftm_info->used_size++;
                }
                break;
            case DRV_FTM_CAM_TYPE_XC:
                if(index % 2)
                {
                    continue;
                }
                in.tbl_id = table_id;
                in.index = index;
                drv_acc_api(lchip, &in, &out);
                hash_type = GetDsEgressXcOamSvlanPortHashKey(V, hashType_f, &out.data);
                if(hash_type != 0 )
                {
                    p_ftm_info->used_size += 2;
                }
                break;
            case DRV_FTM_CAM_TYPE_FLOW:
                if(index % 2)
                {
                    continue;
                }
                in.tbl_id = table_id;
                in.index = index;
                drv_acc_api(lchip, &in, &out);
                hash_type = GetDsFlowL2HashKey(V, hashKeyType_f, &out.data);
                if(hash_type != 0)
                {
                    p_ftm_info->used_size += 2;
                }
                break;
            default:
                break;
        }
    }
    p_ftm_info->max_size = cam_num;
    return DRV_E_NONE;
}

STATIC int32
_drv_usw_ftm_get_tcam_info_detail(uint8 lchip, drv_ftm_info_detail_t* p_ftm_info)
{
    uint8 tcam_key_type        = 0;
    uint32 tcam_key_id          = MaxTblId_t;
    uint32 tcam_ad_a[20]        = {0};
    uint32 old_key_size = 0;
    uint8 key_share_num         = 0;
    uint8 ad_share_num          = 0;
    uint32 key_size             = 0;
    uint8 index                 = 0;
    char* arr_tcam_tbl[] = {
        "IpeAcl0",
        "IpeAcl1",
        "IpeAcl2",
        "IpeAcl3",
        "IpeAcl4",
        "IpeAcl5",
        "IpeAcl6",
        "IpeAcl7",
        "EpeAcl0",
        "EpeAcl1",
        "EpeAcl2",
        "SCL0",
        "SCL1",
        "SCL2",
        "SCL3",
        "LPM0All",
        "LPM0",
        "LPM0SaPrivate",
        "LPM0DaPublic",
        "LPM0SaPublic",
        "LPM0Lk2",
        "LPM0SaPrivateLk2",
        "LPM0DaPublicLk2",
        "LPM0SaPublicLk2",
        "LPM1",
        "Static"
    };

    tcam_key_type = p_ftm_info->tbl_type;

    _drv_usw_ftm_get_tcam_info(lchip, tcam_key_type,
                             p_ftm_info->tbl_id,
                             tcam_ad_a,
                             &key_share_num,
                             &ad_share_num);
    if (0 == key_share_num || 0 == ad_share_num)
    {
        return DRV_E_NONE;
    }

    p_ftm_info->tbl_entry_num = key_share_num;

    /*tcam key alloc*/
    for (index = 0; index < key_share_num; index++)
    {
        tcam_key_id = p_ftm_info->tbl_id[index];

        if (tcam_key_id >= MaxTblId_t)
        {
            DRV_FTM_DBG_OUT("unexpect tbl id:%d\r\n", tcam_key_id);
            return DRV_E_INVAILD_TYPE;
        }

        if (0 == TABLE_FIELD_NUM(lchip, tcam_key_id))
        {
            continue;
        }

        p_ftm_info->max_idx[index]  = TABLE_MAX_INDEX(lchip, tcam_key_id);
        key_size            = TCAM_KEY_SIZE(lchip, tcam_key_id);
        if (tcam_key_type >= DRV_FTM_TCAM_TYPE_IGS_LPM0DA && tcam_key_type <= DRV_FTM_TCAM_TYPE_IGS_LPM1)
        {
            key_size            = (key_size / DRV_LPM_KEY_BYTES_PER_ENTRY) * 40;
        }
        else
        {
            key_size            = (key_size / DRV_BYTES_PER_ENTRY) * 80;
        }

        if (tcam_key_type != DRV_FTM_TCAM_TYPE_IGS_LPM1)
        {
            if (old_key_size != key_size)
            {
                p_ftm_info->key_size[index] = key_size;
            }
            else
            {
                p_ftm_info->key_size[index] = 0;
            }
        }
        else
        {
            p_ftm_info->key_size[index] = key_size;
        }

        old_key_size = key_size;

    }

    sal_strcpy(p_ftm_info->str, arr_tcam_tbl[tcam_key_type]);

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_ftm_get_edram_info_detail(uint8 lchip, drv_ftm_info_detail_t* p_ftm_info)
{
    uint8 mem_id = 0;
    uint8 mem_id_bit = 0;
    uint8 mem_num = 0;


    _drv_usw_ftm_get_sram_tbl_name(p_ftm_info->tbl_type, p_ftm_info->str);

    for (mem_id = DRV_FTM_SRAM0; mem_id < DRV_FTM_EDRAM_MAX; mem_id++)
    {
        mem_id_bit = (mem_id >= DRV_FTM_MIXED0)?(mem_id - DRV_FTM_MIXED0):mem_id;

        if (IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(p_ftm_info->tbl_type), mem_id_bit) &&
            DRV_FTM_TBL_MEM_ENTRY_NUM(p_ftm_info->tbl_type, mem_id))
        {
            p_ftm_info->mem_id[mem_num] = mem_id;
            p_ftm_info->entry_num[mem_num] = DRV_FTM_TBL_MEM_ENTRY_NUM(p_ftm_info->tbl_type, mem_id);
            p_ftm_info->offset[mem_num] = DRV_FTM_TBL_MEM_START_OFFSET(p_ftm_info->tbl_type, mem_id);

            ++ mem_num;
        }
    }

    p_ftm_info->tbl_entry_num = mem_num;

    return DRV_E_NONE;
}

STATIC int32
_drv_usw_ftm_get_lpm_tcam_init_size(uint8 lchip, drv_ftm_info_detail_t* p_ftm_info)
{
    uint16 mem_idx;
    uint16 mem_id;
    uint32 mem_bmp = g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][0];
    uint16 max_mem_id = 0;

    max_mem_id = 0xFFFF;
    for (mem_idx = DRV_FTM_LPM_TCAM_KEY0; mem_idx < DRV_FTM_LPM_TCAM_KEYM; mem_idx++)
    {
        mem_id = mem_idx - DRV_FTM_LPM_TCAM_KEY0;
        if (!IS_BIT_SET(mem_bmp, mem_id))
        {
            continue;
        }
        max_mem_id = mem_id;
    }
    p_ftm_info->l3.lpm_tcam_init_size[0][0] = (max_mem_id==0xFFFF)?0:TCAM_END_INDEX(lchip, DsLpmTcamIpv4HalfKey_t, max_mem_id)+1;

    max_mem_id = 0xFFFF;
    mem_bmp = g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][1];
    for (mem_idx = DRV_FTM_LPM_TCAM_KEY0; mem_idx < DRV_FTM_LPM_TCAM_KEYM; mem_idx++)
    {
        mem_id = mem_idx - DRV_FTM_LPM_TCAM_KEY0;
        if (!IS_BIT_SET(mem_bmp, mem_id))
        {
            continue;
        }
        max_mem_id = mem_id;
    }
    p_ftm_info->l3.lpm_tcam_init_size[0][1] = (max_mem_id==0xFFFF)?0:TCAM_END_INDEX(lchip, DsLpmTcamIpv4HalfKey_t, max_mem_id)+1;

    max_mem_id = 0xFFFF;
    mem_bmp = g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[0][2];
    for (mem_idx = DRV_FTM_LPM_TCAM_KEY0; mem_idx < DRV_FTM_LPM_TCAM_KEYM; mem_idx++)
    {
        mem_id = mem_idx - DRV_FTM_LPM_TCAM_KEY0;
        if (!IS_BIT_SET(mem_bmp, mem_id))
        {
            continue;
        }
        max_mem_id = mem_id;
    }
    p_ftm_info->l3.lpm_tcam_init_size[0][2] = (max_mem_id==0xFFFF)?0:TCAM_END_INDEX(lchip, DsLpmTcamIpv4HalfKey_t, max_mem_id)+1;

    if(g_usw_ftm_master[lchip]->lpm_model == LPM_MODEL_PUBLIC_IPDA_PRIVATE_IPDA_HALF || \
        g_usw_ftm_master[lchip]->lpm_model ==LPM_MODEL_PUBLIC_IPDA_IPSA_PRIVATE_IPDA_IPSA_HALF)
    {
        max_mem_id = 0xFFFF;
        mem_bmp = g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][0];
        for (mem_idx = DRV_FTM_LPM_TCAM_KEY0; mem_idx < DRV_FTM_LPM_TCAM_KEYM; mem_idx++)
        {
            mem_id = mem_idx - DRV_FTM_LPM_TCAM_KEY0;
            if (!IS_BIT_SET(mem_bmp, mem_id))
            {
                continue;
            }
            max_mem_id = mem_id;
        }
        p_ftm_info->l3.lpm_tcam_init_size[1][0] = (max_mem_id==0xFFFF)?0:TCAM_END_INDEX(lchip, DsLpmTcamIpv4DaPubHalfKey_t, max_mem_id)+1;

        max_mem_id = 0xFFFF;
        mem_bmp = g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][1];
        for (mem_idx = DRV_FTM_LPM_TCAM_KEY0; mem_idx < DRV_FTM_LPM_TCAM_KEYM; mem_idx++)
        {
            mem_id = mem_idx - DRV_FTM_LPM_TCAM_KEY0;
            if (!IS_BIT_SET(mem_bmp, mem_id))
            {
                continue;
            }
            max_mem_id = mem_id;
        }
        p_ftm_info->l3.lpm_tcam_init_size[1][1] = (max_mem_id==0xFFFF)?0:TCAM_END_INDEX(lchip, DsLpmTcamIpv4DaPubHalfKey_t, max_mem_id)+1;

        max_mem_id = 0xFFFF;
        mem_bmp = g_usw_ftm_master[lchip]->lpm_tcam_init_bmp[1][2];
        for (mem_idx = DRV_FTM_LPM_TCAM_KEY0; mem_idx < DRV_FTM_LPM_TCAM_KEYM; mem_idx++)
        {
            mem_id = mem_idx - DRV_FTM_LPM_TCAM_KEY0;
            if (!IS_BIT_SET(mem_bmp, mem_id))
            {
                continue;
            }
            max_mem_id = mem_id;
        }
        p_ftm_info->l3.lpm_tcam_init_size[1][2] = (max_mem_id==0xFFFF)?0:TCAM_END_INDEX(lchip, DsLpmTcamIpv4DaPubHalfKey_t, max_mem_id)+1;
    }
	return DRV_E_NONE;
}
STATIC int32 _drv_usw_ftm_get_pipeline_info(uint8 lchip, drv_ftm_info_detail_t* p_ftm_info)
{
    uint32 bitmap = DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_LPM_LKP_KEY0);
    uint8  mem_count = 0;
    uint8  mem_idx;

    if(!bitmap)
    {
        return DRV_E_NONE;
    }
#if 0
	/*ipda & ipsa half key, must NOT use couple mode*/
	if(g_usw_ftm_master[lchip]->lpm_model == LPM_MODEL_PRIVATE_IPDA_IPSA_HALF)
	{
		p_ftm_info->l3.pipeline_info.size0 =  TABLE_MAX_INDEX(lchip, DsLpmLookupKey_t);
        p_ftm_info->l3.pipeline_info.size1 = 0;
		return DRV_E_NONE;
	}
#endif
    for(mem_idx=0; mem_idx < DRV_FTM_SRAM_MAX; mem_idx++)
    {
        if(!DRV_IS_BIT_SET(bitmap, mem_idx))
        {
            continue;
        }
        mem_count++;
    }

    if(mem_count >= 2)
    {
        uint32 total_size = TABLE_MAX_INDEX(lchip, DsLpmLookupKey_t);
        uint32 temp_size = 0;
        for(mem_idx=0; mem_idx < DRV_FTM_SRAM_MAX; mem_idx++)
        {
            if(!DRV_IS_BIT_SET(bitmap, mem_idx))
            {
                continue;
            }
            temp_size += DYNAMIC_ENTRY_NUM(lchip, DsLpmLookupKey_t, mem_idx);
            if(temp_size >= total_size/2)
            {
                if(temp_size == total_size/2)
                {
                    p_ftm_info->l3.pipeline_info.size0 = temp_size;
                    p_ftm_info->l3.pipeline_info.size1 = temp_size;
                }
                else
                {
                    p_ftm_info->l3.pipeline_info.size0 = TABLE_MAX_INDEX(lchip, DsLpmLookupKey_t);
                    p_ftm_info->l3.pipeline_info.size1 = 0;
                }
                break;
            }

        }
    }
    else
    {
        p_ftm_info->l3.pipeline_info.size0 =  TABLE_MAX_INDEX(lchip, DsLpmLookupKey_t);
        p_ftm_info->l3.pipeline_info.size1 = 0;
    }
	return DRV_E_NONE;
}
int32
drv_usw_ftm_set_tcam_spec(uint8 lchip, void* profile_info)
{
    uint32 entry_num = 0;
    uint8 tcam_key_type = 0;

    for (tcam_key_type = DRV_FTM_TCAM_TYPE_IGS_ACL0; tcam_key_type <= DRV_FTM_TCAM_TYPE_IGS_USERID1; tcam_key_type++)
    {

        entry_num =  DRV_FTM_KEY_MAX_INDEX(tcam_key_type);

        g_usw_ftm_master[lchip]->tcam_specs[tcam_key_type][DRV_FTM_TCAM_SIZE_80] = (entry_num / 4) / 1;
        g_usw_ftm_master[lchip]->tcam_specs[tcam_key_type][DRV_FTM_TCAM_SIZE_160]  = (entry_num / 4) / 2;
        g_usw_ftm_master[lchip]->tcam_specs[tcam_key_type][DRV_FTM_TCAM_SIZE_320]  = (entry_num / 4) / 4;
        g_usw_ftm_master[lchip]->tcam_specs[tcam_key_type][DRV_FTM_TCAM_SIZE_640] = (entry_num / 4) / 8;


        g_usw_ftm_master[lchip]->tcam_offset[tcam_key_type][DRV_FTM_TCAM_SIZE_80] = 0;
        g_usw_ftm_master[lchip]->tcam_offset[tcam_key_type][DRV_FTM_TCAM_SIZE_160] = (g_usw_ftm_master[lchip]->tcam_specs[tcam_key_type][DRV_FTM_TCAM_SIZE_80])/2;

        g_usw_ftm_master[lchip]->tcam_offset[tcam_key_type][DRV_FTM_TCAM_SIZE_320] = (g_usw_ftm_master[lchip]->tcam_specs[tcam_key_type][DRV_FTM_TCAM_SIZE_80]
                                                                            + g_usw_ftm_master[lchip]->tcam_specs[tcam_key_type][DRV_FTM_TCAM_SIZE_160]*2)/4;

        g_usw_ftm_master[lchip]->tcam_offset[tcam_key_type][DRV_FTM_TCAM_SIZE_640] = (g_usw_ftm_master[lchip]->tcam_specs[tcam_key_type][DRV_FTM_TCAM_SIZE_80]
                                                                            +g_usw_ftm_master[lchip]->tcam_specs[tcam_key_type][DRV_FTM_TCAM_SIZE_160]*2
                                                                            +g_usw_ftm_master[lchip]->tcam_specs[tcam_key_type][DRV_FTM_TCAM_SIZE_320]*4)/8;
    }

    return 0;
}
int32
drv_usw_ftm_map_tcam_index(uint8 lchip,
                       uint32 tbl_id,
                       uint32 old_index,
                       uint32* new_index)
{
    *new_index = old_index;
    #if 0
    uint8 size = 0;
    uint8 tcam_key_type = 0;
    uint8 size_type = 0;

    if (NULL == TABLE_EXT_INFO_PTR(lchip, tbl_id))
    {
        return 0;
    }

    if (TCAM_KEY_MODULE(lchip, tbl_id) != TCAM_MODULE_ACL
        && TCAM_KEY_MODULE(lchip, tbl_id) != TCAM_MODULE_SCL
        && TCAM_KEY_MODULE(lchip, tbl_id) != TCAM_MODULE_LPM )
    {
        return 0;
    }

    tcam_key_type = TCAM_KEY_TYPE(lchip, tbl_id);

    if (TCAM_KEY_MODULE(lchip, tbl_id) == TCAM_MODULE_ACL)
    {

        /*
        80  bits / (2 * 80bits) = 0 取 type == 0
        160 bits / (2 * 80bits) = 1 取 type == 1
        320 bits / (2 * 80bits) = 2 取 type == 2
        640 bits / (2 * 80bits) = 4 取 type == 3
        */
        size_type = TCAM_KEY_SIZE(lchip, tbl_id) / (DRV_BYTES_PER_ENTRY * 2);
        size_type = (4 == size_type) ? 3 : size_type;

        *new_index = g_usw_ftm_master[lchip]->tcam_offset[tcam_key_type][size_type] + old_index;

        if (TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_TCAM_AD)
        {
            if (size_type == 0)
            {
                *new_index = (*new_index) *(TCAM_KEY_SIZE(lchip, tbl_id) / (DRV_BYTES_PER_ENTRY*1));
            }
            else
            {
                *new_index = (*new_index) *(TCAM_KEY_SIZE(lchip, tbl_id) / (DRV_BYTES_PER_ENTRY*2));
            }
        }

    }
    else if (TCAM_KEY_MODULE(lchip, tbl_id) == TCAM_MODULE_SCL)
    {
        if (TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_TCAM_AD)
        {
           return 0;
        }

        if (tcam_key_type == DRV_FTM_TCAM_TYPE_IGS_USERID1)
        {
            return 0;
        }

        size_type = TCAM_KEY_SIZE(lchip, tbl_id) / (DRV_BYTES_PER_ENTRY * 2);
        size_type = (4 == size_type) ? 3 : size_type;

        *new_index = g_usw_ftm_master[lchip]->tcam_offset[tcam_key_type][size_type] + old_index;
    }
    else if(TCAM_KEY_MODULE(lchip, tbl_id) == TCAM_MODULE_LPM )
    {

        size_type = TCAM_KEY_SIZE(lchip, tbl_id) / (DRV_LPM_KEY_BYTES_PER_ENTRY);
        if (size_type == 2)
        {
            size_type = DRV_FTM_TCAM_SIZE_LPMSingle;
        }
        else if(size_type == 4)
        {
            size_type = DRV_FTM_TCAM_SIZE_LPMDouble;
        }
        else
        {
            size_type = DRV_FTM_TCAM_SIZE_LPMhalf;
        }

        *new_index = g_usw_ftm_master[lchip]->tcam_offset[tcam_key_type][size_type] + old_index;

        if (TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_TCAM_LPM_AD || TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_TCAM_NAT_AD)
        {
            *new_index = (*new_index) *(TCAM_KEY_SIZE(lchip, tbl_id) / DRV_LPM_KEY_BYTES_PER_ENTRY);
        }
    }


    #endif
    return 0;

}

 /*drv_ftm_mem_id_t*/
uint32
_drv_usw_ftm_memid_2_table(uint8 lchip, uint32 mem_id)
{
    uint32 table_id;

    if(mem_id <= DRV_FTM_SRAM_MAX)
    {
        for(table_id=0; table_id < DRV_FTM_SRAM_TBL_MAX; table_id++)
        {
            if(DRV_FTM_TBL_MEM_BITMAP(table_id) & (1<<mem_id))
            {
                break;
            }
        }
    }
    else if(mem_id <=DRV_FTM_TCAM_KEYM)
    {
        mem_id = mem_id - DRV_FTM_TCAM_KEY0;
        for(table_id=0; table_id < DRV_FTM_TCAM_TYPE_MAX; table_id++)
        {
            if(DRV_FTM_KEY_MEM_BITMAP(table_id) & (1<<mem_id))
            {
                break;
            }
        }
    }
    else
    {
        mem_id = mem_id - DRV_FTM_LPM_TCAM_KEY0;
        for(table_id=0; table_id < DRV_FTM_TCAM_TYPE_MAX; table_id++)
        {
            if(DRV_FTM_KEY_MEM_BITMAP(table_id) & (1<<mem_id))
            {
                break;
            }
        }
    }

    return table_id;
}

#define _____APIs_____
int32
drv_usw_ftm_get_entry_num(uint8 lchip, uint32 tbl_id, uint32* entry_num)
{
    DRV_INIT_CHECK(lchip);
    *entry_num = TABLE_MAX_INDEX(lchip, tbl_id);

    return DRV_E_NONE;
}

int32
drv_usw_ftm_lookup_ctl_init(uint8 lchip)
{

    DRV_IF_ERROR_RETURN(drv_usw_tcam_ctl_init(lchip));
    DRV_IF_ERROR_RETURN(drv_usw_dynamic_cam_init(lchip));


    if (DRV_IS_DUET2(lchip))
    {
        DRV_IF_ERROR_RETURN(drv_duet2_dynamic_arb_init(lchip));
    }
    else if(DRV_IS_TSINGMA(lchip))
    {
        DRV_IF_ERROR_RETURN(drv_tsingma_dynamic_arb_init(lchip));
    }

    DRV_IF_ERROR_RETURN(drv_usw_dynamic_grp_init(lchip));
    DRV_IF_ERROR_RETURN(drv_usw_flow_lkup_ctl_init(lchip));
    if (DRV_IS_DUET2(lchip))
    {
        DRV_IF_ERROR_RETURN(drv_usw_fib_lkup_ctl_init(lchip));
    }

    /*when OAM enable, need to config reserved 1 for performance*/
    if(DRV_IS_TSINGMA(lchip))
    {
        uint32 cmd = 0;
        uint32 tmp_val32 = DRV_FTM_TBL_MAX_ENTRY_NUM(DRV_FTM_SRAM_TBL_OAM_MEP) ? 1:0;
        cmd = DRV_IOW(DynamicKeyReserved_t, DynamicKeyReserved_reserved_f);
        DRV_IOCTL(lchip, 0, cmd, &tmp_val32);
    }



    return DRV_E_NONE;
}

int32
drv_usw_ftm_ext_table_type_init(uint8 lchip)
{
    uint32 index =0;
    uint32 tbl_id = 0;
    tbls_ext_info_t* p_tbl_ext_info = NULL;

    /*1. init parser table ext type*/
    for(index =0; index < sizeof(parser_tbl_id_list)/sizeof(parser_tbl_id_list[0]); index++)
    {
        tbl_id = parser_tbl_id_list[index];
        if (TABLE_EXT_INFO_PTR(lchip, tbl_id) == NULL)
        {
            p_tbl_ext_info = mem_malloc(MEM_FTM_MODULE, sizeof(tbls_ext_info_t));
            if (NULL == p_tbl_ext_info)
            {
                return DRV_E_NO_MEMORY;
            }

            sal_memset(p_tbl_ext_info, 0, sizeof(tbls_ext_info_t));
            p_tbl_ext_info->ext_type = 1;

            TABLE_EXT_INFO_PTR(lchip, tbl_id) = p_tbl_ext_info;
        }
        else
        {
            p_tbl_ext_info = TABLE_EXT_INFO_PTR(lchip, tbl_id);
            p_tbl_ext_info->ext_type = 1;
        }
    }

    /*2.init 80bits flow ad table */
    for(index =0; index < sizeof(acl_ad_80bits_tbl)/sizeof(acl_ad_80bits_tbl[0]); index++)
    {
        tbl_id = acl_ad_80bits_tbl[index];
        if (TABLE_EXT_INFO_PTR(lchip, tbl_id) == NULL)
        {
            p_tbl_ext_info = mem_malloc(MEM_FTM_MODULE, sizeof(tbls_ext_info_t));
            if (NULL == p_tbl_ext_info)
            {
                return DRV_E_NO_MEMORY;
            }

            sal_memset(p_tbl_ext_info, 0, sizeof(tbls_ext_info_t));
            p_tbl_ext_info->ext_type = 2;

            TABLE_EXT_INFO_PTR(lchip, tbl_id) = p_tbl_ext_info;
        }
        else
        {
            p_tbl_ext_info = TABLE_EXT_INFO_PTR(lchip, tbl_id);
            p_tbl_ext_info->ext_type = 2;
        }
    }

    return DRV_E_NONE;
}

int32 drv_usw_ftm_alloc_ipfix(uint8 lchip)
{

#if 1
    uint16 tbl_num = 0;
    uint32 table_id = 0;

    uint32 drv_ingress_ipfix_hash_key[] =
    {
        DsIpfixL2HashKey_t,
        DsIpfixL2L3HashKey_t,
        DsIpfixL3Ipv4HashKey_t,
        DsIpfixL3Ipv6HashKey_t,
        DsIpfixL3MplsHashKey_t,
        DsIpfixL2HashKey0_t,
        DsIpfixL2L3HashKey0_t,
        DsIpfixL3Ipv4HashKey0_t,
        DsIpfixL3Ipv6HashKey0_t,
        DsIpfixL3MplsHashKey0_t,
    };

    uint32 drv_egress_ipfix_hash_key[] =
    {
        DsIpfixL2HashKey1_t,
        DsIpfixL2L3HashKey1_t,
        DsIpfixL3Ipv4HashKey1_t,
        DsIpfixL3Ipv6HashKey1_t,
        DsIpfixL3MplsHashKey1_t,
    };


#define DS_IPFIX_HASH_KEY_TABLE12_W0_ADDR                            0x01700000
#define DS_IPFIX_HASH_KEY_TABLE12_W1_ADDR                            0x01300000
#define DS_IPFIX_HASH_KEY_TABLE6_W0_ADDR                             0x01740000
#define DS_IPFIX_HASH_KEY_TABLE6_W1_ADDR                             0x01340000
#define FLOW_ACC_AD_RAM0_ADDR                                        0x01600000
#define FLOW_ACC_AD_RAM1_ADDR                                        0x01200000


    for(tbl_num = 0; tbl_num< 10; tbl_num++)
    {
        table_id = drv_ingress_ipfix_hash_key[tbl_num];

        if(TABLE_ENTRY_SIZE(0, table_id) == (DRV_BYTES_PER_ENTRY*2))
        {
            TABLE_INFO(lchip, table_id).addrs[0] = DS_IPFIX_HASH_KEY_TABLE6_W0_ADDR;
            TABLE_INFO(lchip, table_id).entry = 2*TABLE_MAX_INDEX(0, DsIpfixHashKey0_t);
        }
        else
        {
            TABLE_INFO(lchip, table_id).addrs[0] = DS_IPFIX_HASH_KEY_TABLE12_W0_ADDR;
            TABLE_INFO(lchip, table_id).entry = TABLE_MAX_INDEX(0, DsIpfixHashKey0_t);
        }
    }

    for(tbl_num = 0; tbl_num< 5; tbl_num++)
    {
        table_id = drv_egress_ipfix_hash_key[tbl_num];

        if(TABLE_ENTRY_SIZE(0, table_id) == (DRV_BYTES_PER_ENTRY*2))
        {
            TABLE_INFO(lchip, table_id).addrs[0] = DS_IPFIX_HASH_KEY_TABLE6_W1_ADDR;
            TABLE_INFO(lchip, table_id).entry = 2*TABLE_MAX_INDEX(0, DsIpfixHashKey0_t);
        }
        else
        {
            TABLE_INFO(lchip, table_id).addrs[0] = DS_IPFIX_HASH_KEY_TABLE12_W1_ADDR;
            TABLE_INFO(lchip, table_id).entry = TABLE_MAX_INDEX(0, DsIpfixHashKey0_t);
        }
    }

    TABLE_INFO(lchip, DsIpfixSessionRecord0_t).addrs[0] = FLOW_ACC_AD_RAM0_ADDR;
    TABLE_INFO(lchip, DsIpfixSessionRecord1_t).addrs[0] = FLOW_ACC_AD_RAM1_ADDR;
    TABLE_INFO(lchip, DsIpfixSessionRecord0_t).entry =  TABLE_MAX_INDEX(0, DsIpfixSessionRecord_t);
    TABLE_INFO(lchip, DsIpfixSessionRecord1_t).entry =  TABLE_MAX_INDEX(0, DsIpfixSessionRecord_t);

    #endif
    return 0;
}

int32
drv_usw_ftm_mem_alloc(uint8 lchip, void* p_profile_info)
{
    drv_ftm_profile_info_t *profile_info = (drv_ftm_profile_info_t *)p_profile_info;

    DRV_PTR_VALID_CHECK(profile_info);

    if ((profile_info->profile_type >= 1)
        && ((profile_info->profile_type != 12)))
    {
        return DRV_E_INVAILD_TYPE;
    }

    /*init global param*/
    DRV_IF_ERROR_RETURN(drv_usw_ftm_init(lchip, profile_info));


    DRV_IF_ERROR_RETURN(drv_usw_set_dynamic_ram_couple_mode(lchip, profile_info->couple_mode));

    /*set profile*/
    DRV_IF_ERROR_RETURN(drv_usw_ftm_set_profile(lchip, profile_info));

    /*alloc hash table*/
    DRV_IF_ERROR_RETURN(drv_usw_ftm_alloc_sram(lchip, 0, DRV_FTM_SRAM_TBL_MAX, 0));

    /* alloc tcam table*/
    DRV_IF_ERROR_RETURN(drv_usw_ftm_alloc_tcam(lchip, 0, DRV_FTM_TCAM_TYPE_MAX, 0));

    /* alloc static table*/
    DRV_IF_ERROR_RETURN(drv_usw_ftm_alloc_static(lchip));

    if (DRV_IS_TSINGMA(lchip))
    {
        DRV_IF_ERROR_RETURN(drv_usw_ftm_alloc_ipfix(lchip));
    }

    DRV_IF_ERROR_RETURN(drv_usw_ftm_alloc_cam(lchip, &profile_info->cam_info));

    DRV_IF_ERROR_RETURN(drv_usw_ftm_ext_table_type_init(lchip));

    DRV_IF_ERROR_RETURN(drv_usw_ftm_set_tcam_spec(lchip, profile_info));


    return DRV_E_NONE;


}
int32
drv_usw_ftm_mem_free(uint8 lchip)
{
    tbls_id_t tbl_id = 0;

    for(tbl_id=0; tbl_id < MaxTblId_t; tbl_id++)
    {
        if(!TABLE_EXT_INFO_PTR(lchip, tbl_id))
        {
            continue;
        }

        if(TABLE_EXT_INFO_PTR(lchip, tbl_id)->ptr_ext_content)
        {
            if((TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_TCAM) ||
                (TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_TCAM_AD) ||
                (TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_TCAM_LPM_AD) ||
                (TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_TCAM_NAT_AD) ||
                (TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_LPM_TCAM_IP) ||
                (TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_LPM_TCAM_NAT) ||
                (TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_STATIC_TCAM_KEY) ||
                (TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_DYNAMIC))
            {
                if((TABLE_EXT_INFO_TYPE(lchip, tbl_id) != EXT_INFO_TYPE_DYNAMIC) && TCAM_EXT_INFO_PTR(lchip, tbl_id)->hw_mask_base)
                {
                    mem_free(TCAM_EXT_INFO_PTR(lchip, tbl_id)->hw_mask_base);
                }
            }
            mem_free(TABLE_EXT_INFO_PTR(lchip, tbl_id)->ptr_ext_content);
        }
        mem_free(TABLE_EXT_INFO_PTR(lchip, tbl_id));
        TABLE_EXT_INFO_PTR(lchip, tbl_id) = NULL;
    }



    if(!g_usw_ftm_master[lchip])
    {
        return DRV_E_NONE;
    }
    mem_free(g_usw_ftm_master[lchip]->p_tcam_key_info);
    mem_free(g_usw_ftm_master[lchip]->p_sram_tbl_info);
    mem_free(g_usw_ftm_master[lchip]);

    return DRV_E_NONE;
}
 #if 0
int32
drv_usw_ftm_alloc_cam_offset(uint8 lchip, uint32 tbl_id, uint32 *offset)
{

    uint8 bit_len = 0;
    uint32 *bitmap0  = 0;
    uint32 *bitmap1  = 0;
    uint32 *bitmap2  = 0;

    uint32 *bitmap    = 0;
    uint32 loop_cnt  = 0;
    uint32 bit_offset = 0;

    uint8 cam_type = 0;
    uint32 alloc_offset = 0;
    uint8 cam_num = 0;

    /*1. get clear bit*/
    bit_len  = TABLE_ENTRY_SIZE(lchip, tbl_id) / BYTE_TO_4W;

    drv_usw_ftm_get_cam_by_tbl_id(tbl_id, &cam_type, &cam_num);
    if (DRV_FTM_CAM_TYPE_INVALID == cam_type)
    {
        return DRV_E_INVAILD_TYPE;
    }

    DRV_FTM_CAM_CHECK_TBL_VALID(cam_type, tbl_id);

    if ((DRV_FTM_CAM_TYPE_FIB_HOST1_NAT == cam_type)
        || (DRV_FTM_CAM_TYPE_FIB_HOST1_MC == cam_type))
    {
        bitmap0 = &g_usw_ftm_master[lchip]->fib1_cam_bitmap0[0];
        bitmap1 = &g_usw_ftm_master[lchip]->fib1_cam_bitmap1[0];
        bitmap2 = &g_usw_ftm_master[lchip]->fib1_cam_bitmap2[0];

        loop_cnt = cam_num/bit_len;
    }
    else if ((DRV_FTM_CAM_TYPE_SCL == cam_type)
        || (DRV_FTM_CAM_TYPE_DCN == cam_type)
        || (DRV_FTM_CAM_TYPE_MPLS == cam_type))
    {
        bitmap0 = &g_usw_ftm_master[lchip]->userid_cam_bitmap0[0];
        bitmap1 = &g_usw_ftm_master[lchip]->userid_cam_bitmap1[0];
        bitmap2 = &g_usw_ftm_master[lchip]->userid_cam_bitmap2[0];

        loop_cnt = cam_num/bit_len;
    }
    else if ((DRV_FTM_CAM_TYPE_XC == cam_type)
            || (DRV_FTM_CAM_TYPE_OAM == cam_type))
    {
        bitmap0 = &g_usw_ftm_master[lchip]->xcoam_cam_bitmap0[0];
        bitmap1 = &g_usw_ftm_master[lchip]->xcoam_cam_bitmap1[0];
        bitmap2 = &g_usw_ftm_master[lchip]->xcoam_cam_bitmap2[0];

        loop_cnt = cam_num/bit_len;
    }

    /*judge allocate or not*/
    if(g_usw_ftm_master[lchip]->conflict_cam_num[cam_type] < bit_len)
    {
        return DRV_E_HASH_CONFLICT;
    }

    bitmap  = (1 == bit_len) ? bitmap0 : ((2 == bit_len) ? bitmap1 : bitmap2);

    for(bit_offset = 0; bit_offset < loop_cnt; bit_offset++)
    {
        if(!IS_BIT_SET(bitmap[(bit_offset/32)], (bit_offset%32)))
        {
            break;
        }
    }

    if(bit_offset == loop_cnt)
    {
        return DRV_E_HASH_CONFLICT;
    }

    /*alloc num*/
    g_usw_ftm_master[lchip]->conflict_cam_num[cam_type] -= bit_len;

    alloc_offset  = bit_offset*bit_len;

    /*set bitmap*/
    if (1 == bit_len)
    {
        SET_BIT(bitmap0[alloc_offset/32], (alloc_offset%32));
        SET_BIT(bitmap1[(alloc_offset/2)/32], ((alloc_offset/2)%32));
        SET_BIT(bitmap2[(alloc_offset/4)/32], ((alloc_offset/4)%32));
    }
    else if (2 == bit_len)
    {
        SET_BIT(bitmap0[alloc_offset/32], (alloc_offset%32));
        SET_BIT(bitmap0[(alloc_offset+1)/32], ((alloc_offset+1)%32));

        SET_BIT(bitmap1[(alloc_offset/2)/32], ((alloc_offset/2)%32));
        SET_BIT(bitmap2[(alloc_offset/4)/32], ((alloc_offset/4)%32));
    }
    else if (4 == bit_len)
    {
        SET_BIT(bitmap0[alloc_offset/32], (alloc_offset%32));
        SET_BIT(bitmap0[(alloc_offset+1)/32], ((alloc_offset+1)%32));
        SET_BIT(bitmap0[(alloc_offset+2)/32], ((alloc_offset+2)%32));
        SET_BIT(bitmap0[(alloc_offset+3)/32], ((alloc_offset+3)%32));

        SET_BIT(bitmap1[(alloc_offset/2)/32], ((alloc_offset/2)%32));
        SET_BIT(bitmap1[(alloc_offset/2+1)/32], ((alloc_offset/2+1)%32));

        SET_BIT(bitmap2[(alloc_offset/4)/32], ((alloc_offset/4)%32));
    }

    *offset = alloc_offset;

    return DRV_E_NONE;
}

int32
drv_usw_ftm_free_cam_offset(uint8 lchip, uint32 tbl_id, uint32 offset)
{
    uint8 bit_len = 0;
    uint32 *bitmap0  = 0;
    uint32 *bitmap1  = 0;
    uint32 *bitmap2  = 0;

    uint8 bit_offset0 = 0;
    uint8 bit_offset1 = 0;
    uint8 bit_offset2 = 0;

    uint8 cam_type = 0;
    uint8 is_alloc = 0;
    uint8 cam_num = 0;

    /*1. get clear bit*/
    bit_len  = TABLE_ENTRY_SIZE(lchip, tbl_id) / 12;


    drv_usw_ftm_get_cam_by_tbl_id(tbl_id, &cam_type, &cam_num);
    if (DRV_FTM_CAM_TYPE_INVALID == cam_type)
    {
        return DRV_E_INVAILD_TYPE;
    }

    if ((DRV_FTM_CAM_TYPE_FIB_HOST1_NAT == cam_type)
        || (DRV_FTM_CAM_TYPE_FIB_HOST1_MC == cam_type))
    {
        bitmap0 = &g_usw_ftm_master[lchip]->fib1_cam_bitmap0[0];
        bitmap1 = &g_usw_ftm_master[lchip]->fib1_cam_bitmap1[0];
        bitmap2 = &g_usw_ftm_master[lchip]->fib1_cam_bitmap2[0];
        bit_offset0 = offset;
        bit_offset1 = offset/2;
        bit_offset2 = offset/4;
    }
    else if ((DRV_FTM_CAM_TYPE_SCL == cam_type)
        || (DRV_FTM_CAM_TYPE_DCN == cam_type)
        || (DRV_FTM_CAM_TYPE_MPLS == cam_type))
    {
        bitmap0 = &g_usw_ftm_master[lchip]->userid_cam_bitmap0[0];
        bitmap1 = &g_usw_ftm_master[lchip]->userid_cam_bitmap1[0];
        bitmap2 = &g_usw_ftm_master[lchip]->userid_cam_bitmap2[0];
        bit_offset0 = offset;
        bit_offset1 = offset/2;
        bit_offset2 = offset/4;
    }
    else if ((DRV_FTM_CAM_TYPE_XC == cam_type)
            || (DRV_FTM_CAM_TYPE_OAM == cam_type))
    {
        bitmap0 = &g_usw_ftm_master[lchip]->xcoam_cam_bitmap0[offset/32];
        bitmap1 = &g_usw_ftm_master[lchip]->xcoam_cam_bitmap1[offset/64];
        bitmap2 = &g_usw_ftm_master[lchip]->xcoam_cam_bitmap2[offset/128];

        bit_offset0 = offset%32;
        bit_offset1 = (offset/2)%32;
        bit_offset2 = (offset/4)%32;
    }

    /*judge allocate or not*/
    if (1 == bit_len)
    {
        is_alloc = IS_BIT_SET((*bitmap0), bit_offset0);
    }
    else if (2 == bit_len)
    {
        is_alloc = IS_BIT_SET((*bitmap1), bit_offset1);
    }
    else if (4 == bit_len)
    {
        is_alloc = IS_BIT_SET((*bitmap2), bit_offset2);
    }

    if(!is_alloc)
    {
        return DRV_E_INVALID_ALLOC_INFO;
    }

    /*free num*/
    g_usw_ftm_master[lchip]->conflict_cam_num[cam_type] += bit_len;

    /*free bitmap*/
    if (1 == bit_len)
    {
        CLEAR_BIT((*bitmap0), bit_offset0);


        if((!IS_BIT_SET((*bitmap0), (bit_offset0/2*2))
            &&(!IS_BIT_SET((*bitmap0), (bit_offset0/2*2 + 1)))))
        {
            CLEAR_BIT((*bitmap1), bit_offset1);
        }

        if(!IS_BIT_SET((*bitmap0), (bit_offset0/4*4))
            &&(!IS_BIT_SET((*bitmap0), (bit_offset0/4*4 + 1)))
            && (!IS_BIT_SET((*bitmap0), (bit_offset0/4*4 + 2)))
            && (!IS_BIT_SET((*bitmap0), (bit_offset0/4*4 + 3))))
        {
            CLEAR_BIT((*bitmap2), bit_offset2);
        }

    }
    else if (2 == bit_len)
    {
        CLEAR_BIT((*bitmap1), bit_offset1);

        CLEAR_BIT((*bitmap0), ((bit_offset0/2)*2));
        CLEAR_BIT((*bitmap0), ((bit_offset0/2)*2 + 1));

        if(!IS_BIT_SET((*bitmap1), (bit_offset1/2*2))
            &&(!IS_BIT_SET((*bitmap1), (bit_offset1/2*2 + 1))))
        {
            CLEAR_BIT((*bitmap2), bit_offset2);
        }

    }
    else if (4 == bit_len)
    {
        CLEAR_BIT((*bitmap2), bit_offset2);

        CLEAR_BIT((*bitmap1), ((bit_offset1/2)*2));
        CLEAR_BIT((*bitmap1), ((bit_offset1/2)*2 + 1));

        CLEAR_BIT((*bitmap0), ((bit_offset0/4)*4));
        CLEAR_BIT((*bitmap0), ((bit_offset0/4*4) + 1));
        CLEAR_BIT((*bitmap0), ((bit_offset0/4*4) + 2));
        CLEAR_BIT((*bitmap0), ((bit_offset0/4*4) + 3));
    }
    return DRV_E_NONE;
}
#endif
int32
drv_usw_ftm_get_info_detail(uint8 lchip, drv_ftm_info_detail_t* p_ftm_info)
{
    DRV_INIT_CHECK(lchip);
    switch (p_ftm_info->info_type)
    {
        case DRV_FTM_INFO_TYPE_CAM:
            DRV_IF_ERROR_RETURN(_drv_usw_ftm_get_cam_info(lchip, p_ftm_info));
            break;
        case DRV_FTM_INFO_TYPE_TCAM:
            DRV_IF_ERROR_RETURN(_drv_usw_ftm_get_tcam_info_detail(lchip, p_ftm_info));
            break;
        case DRV_FTM_INFO_TYPE_EDRAM:
            DRV_IF_ERROR_RETURN(_drv_usw_ftm_get_edram_info_detail(lchip, p_ftm_info));
            break;
        case DRV_FTM_INFO_TYPE_LPM_MODEL:
            p_ftm_info->l3.lpm_model = g_usw_ftm_master[lchip]->lpm_model;
            break;
        case DRV_FTM_INFO_TYPE_NAT_PBR_EN:
            p_ftm_info->l3.nat_pbr_en = g_usw_ftm_master[lchip]->nat_pbr_enable;
            break;
        case DRV_FTM_INFO_TYPE_LPM_TCAM_INIT_SIZE:
			DRV_IF_ERROR_RETURN(_drv_usw_ftm_get_lpm_tcam_init_size(lchip, p_ftm_info));
            break;
        case DRV_FTM_INFO_TYPE_PIPELINE_INFO:
			DRV_IF_ERROR_RETURN(_drv_usw_ftm_get_pipeline_info(lchip, p_ftm_info));
            break;

        case DRV_FTM_INFO_TYPE_SCL_MODE:
            p_ftm_info->scl_mode = g_usw_ftm_master[lchip]->scl_mode;

        default:
            break;
    }

    return DRV_E_NONE;
}
#if 0
int32
drv_usw_ftm_check_tbl_recover(uint8 lchip, uint8 mem_id, uint32 ram_offset, uint8* p_recover, uint32* p_tblid)
{

    uint32 sram_type       = 0, tbl_id = 0;
    uint32 mem_entry_num   = 0;
    uint32 start_offset    = 0, entry_num = 0;
    uint32 sram_tbl_a[50]  = {0};
    uint32 tcam_ad_a[20]   = {0};
    uint8  share_tbl_num   = 0, size_type = 0, ad_share_num = 0, tcam_key_type = 0;
    uint8  is_lpm = 0, ad_id = 0;

    if ((NULL == p_recover) || (NULL == p_tblid))
    {
        return DRV_E_INVALID_POINT;
    }
    *p_tblid = MaxTblId_t;

    if (mem_id < DRV_FTM_SRAM_MAX)
    {
        for (sram_type = 0; sram_type < DRV_FTM_SRAM_TBL_MAX; sram_type++)
        {
            mem_entry_num = DRV_FTM_TBL_MEM_ENTRY_NUM(sram_type, mem_id);
            if (!IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(sram_type), mem_id) || 0 == mem_entry_num)
            {
                continue;
            }
            else
            {
                start_offset = DRV_FTM_TBL_MEM_START_OFFSET(sram_type, mem_id);
                if ((ram_offset >= start_offset) && (ram_offset < (start_offset+ mem_entry_num)))
                {

                    break;
                }
                else
                {
                    continue;
                }
            }
        }

        if (sram_type == DRV_FTM_SRAM_TBL_MAX)
        {
            *p_recover = 0;
            return DRV_E_NONE;
        }
        else
        {
            /*
                DsOamLmStats DsBfdMep DsBfdRmep DsEthMep DsEthRmep can't recover
            */
            if ((DRV_FTM_SRAM_TBL_OAM_MEP == sram_type)
                || (DRV_FTM_SRAM_TBL_OAM_LM == sram_type)
                || (DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY == sram_type)
                || (DRV_FTM_SRAM_TBL_IPFIX_AD == sram_type))
            {
                *p_recover = 0;
            }
            else
            {
                *p_recover = 1;
            }

            _drv_usw_ftm_get_sram_tbl_id(lchip, sram_type, sram_tbl_a, &share_tbl_num);
            *p_tblid = sram_tbl_a[0];
        }
    }
    else if ((mem_id >= DRV_FTM_TCAM_AD0) && (mem_id < DRV_FTM_TCAM_ADM))
    {
        ad_id = mem_id - DRV_FTM_TCAM_AD0;

        for (tcam_key_type = DRV_FTM_TCAM_TYPE_IGS_ACL0; tcam_key_type < DRV_FTM_TCAM_TYPE_MAX; tcam_key_type++)
        {
            if(tcam_key_type >= DRV_FTM_TCAM_TYPE_IGS_LPM0 && tcam_key_type <= DRV_FTM_TCAM_TYPE_IGS_LPM1)
            {
                is_lpm = 1;
            }

            for (size_type = DRV_FTM_TCAM_SIZE_160; size_type < DRV_FTM_TCAM_SIZE_MAX; size_type++)
            {
                drv_usw_ftm_get_tcam_ad_by_size_type(tcam_key_type, size_type, tcam_ad_a,  &ad_share_num);

                entry_num = 0;

                tbl_id = tcam_ad_a[0];
                entry_num =  DRV_FTM_KEY_ENTRY_NUM(tcam_key_type, size_type, ad_id);
                if (0 == entry_num)
                {
                    continue;
                }

                if (is_lpm)
                {
                    if (DRV_FTM_TCAM_AD7 == mem_id)
                    {
                        if ((ram_offset >= DRV_FTM_KEY_START_OFFSET(tcam_key_type, size_type, ad_id))
                           && (ram_offset < (DRV_FTM_KEY_START_OFFSET(tcam_key_type, size_type, ad_id) + entry_num)))
                        {
                            *p_tblid = tbl_id;
                            *p_recover = 1;
                            return DRV_E_NONE;
                        }
                    }
                    else
                    {
                        if ((ram_offset >= (DRV_FTM_KEY_START_OFFSET(tcam_key_type, size_type, ad_id) / 2))
                            && (ram_offset < (DRV_FTM_KEY_START_OFFSET(tcam_key_type, size_type, ad_id) / 2) + ((entry_num / 2))))
                        {
                            *p_tblid = tbl_id;
                            *p_recover = 1;
                            return DRV_E_NONE;
                        }
                    }
                }
                else
                {
                    if ((ram_offset >= DRV_FTM_KEY_START_OFFSET(tcam_key_type, size_type, ad_id))
                        && (ram_offset < (DRV_FTM_KEY_START_OFFSET(tcam_key_type, size_type, ad_id) + entry_num)))
                    {
                        *p_tblid = tbl_id;
                        *p_recover = 1;
                        return DRV_E_NONE;
                    }
                }
            }
        }
    }
    else
    {
        return DRV_E_INVALID_PARAMETER;
    }

    return DRV_E_NONE;
}
#endif
int32
drv_usw_ftm_get_sram_type(uint8 lchip, uint32 mem_id, uint8* p_sram_type)
{
    uint8 sram_type = 0;
    for (sram_type = DRV_FTM_SRAM_TBL_MAC_HASH_KEY; sram_type <= DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY; sram_type++)
    {
        if (IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(sram_type), mem_id))
        {
            break;
        }
    }

    *p_sram_type = sram_type;

    return DRV_E_NONE;
}


int32
drv_usw_ftm_get_hash_poly(uint8 lchip, uint32 mem_id, uint8 sram_type, uint32* drv_poly)
{
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 fld_id = 0;

    switch (sram_type)
    {
        case DRV_FTM_SRAM_TBL_FLOW_HASH_KEY:
            tbl_id = FlowHashLookupCtl_t;
            if (DRV_FTM_SRAM0 == mem_id)
            {   /*TsingMa*/
                fld_id = FlowHashLookupCtl_flowLevel0HashType_f;
            }
            else if (DRV_FTM_SRAM1 == mem_id)
            {
                fld_id = FlowHashLookupCtl_flowLevel1HashType_f;
            }
            else if (DRV_FTM_SRAM2 == mem_id)
            {
                fld_id = FlowHashLookupCtl_flowLevel2HashType_f;
            }
            else if (DRV_FTM_SRAM3 == mem_id)
            {
                fld_id = FlowHashLookupCtl_flowLevel3HashType_f;
            }
            else if (DRV_FTM_SRAM4 == mem_id)
            {
                fld_id = FlowHashLookupCtl_flowLevel4HashType_f;
            }
            cmd = DRV_IOR(tbl_id, fld_id);
            DRV_IOCTL(lchip, 0, cmd, drv_poly);
            break;

        case DRV_FTM_SRAM_TBL_USERID_HASH_KEY:
            tbl_id = UserIdHashLookupCtl_t;
            switch(mem_id)
            {
                case DRV_FTM_SRAM2:
                    fld_id = UserIdHashLookupCtl_scl0Level0HashType_f;
                    break;

                case DRV_FTM_SRAM6:
                    fld_id = DRV_IS_DUET2(lchip)? UserIdHashLookupCtl_userIdLevel0HashType_f:
                                              UserIdHashLookupCtl_scl0Level1HashType_f;
                    break;
                case DRV_FTM_SRAM7:
                    fld_id = DRV_IS_DUET2(lchip)? UserIdHashLookupCtl_userIdLevel1HashType_f:
                                              UserIdHashLookupCtl_scl0Level2HashType_f;

                    break;
                case DRV_FTM_SRAM8:
                    fld_id = DRV_IS_DUET2(lchip)? UserIdHashLookupCtl_userIdLevel2HashType_f:
                                              UserIdHashLookupCtl_scl0Level3HashType_f;

                    break;
                case DRV_FTM_SRAM9:
                    fld_id = DRV_IS_DUET2(lchip)? UserIdHashLookupCtl_userIdLevel3HashType_f:
                                              UserIdHashLookupCtl_scl0Level4HashType_f;

                    break;
                case DRV_FTM_SRAM10:
                    fld_id = UserIdHashLookupCtl_userIdLevel4HashType_f;

                    break;
                default:
                    return DRV_E_INVALID_MEM;
            }
            cmd = DRV_IOR(tbl_id, fld_id);
            DRV_IOCTL(lchip, 0, cmd, drv_poly);
            break;


        case DRV_FTM_SRAM_TBL_USERID1_HASH_KEY:
            tbl_id = UserIdHashLookupCtl_t;
            switch(mem_id)
            {
                case DRV_FTM_SRAM5:
                    fld_id = UserIdHashLookupCtl_scl1Level0HashType_f;
                    break;


                case DRV_FTM_SRAM8:
                    fld_id = UserIdHashLookupCtl_scl1Level1HashType_f;
                    break;


                case DRV_FTM_SRAM9:
                    fld_id = UserIdHashLookupCtl_scl1Level2HashType_f;
                    break;


                default:
                    return DRV_E_INVALID_MEM;
            }
            cmd = DRV_IOR(tbl_id, fld_id);
            DRV_IOCTL(lchip, 0, cmd, drv_poly);
            break;


        case DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY:
            tbl_id = EgressXcOamHashLookupCtl_t;
            switch(mem_id)
            {
                case DRV_FTM_SRAM2:
                    fld_id = EgressXcOamHashLookupCtl_egressXcOamLevel0HashType_f;
                    break;
                case DRV_FTM_SRAM5:
                    fld_id = EgressXcOamHashLookupCtl_egressXcOamLevel1HashType_f;
                    break;
                /*TsingMa*/
                case DRV_FTM_SRAM3:
                    fld_id = EgressXcOamHashLookupCtl_egressXcOamLevel0HashType_f;
                    break;
                case DRV_FTM_SRAM4:
                    fld_id = EgressXcOamHashLookupCtl_egressXcOamLevel1HashType_f;
                    break;

                default:
                    return DRV_E_INVALID_MEM;
            }
            cmd = DRV_IOR(tbl_id, fld_id);
            DRV_IOCTL(lchip, 0, cmd, drv_poly);
            break;
        case DRV_FTM_SRAM_TBL_MAC_HASH_KEY:
        case DRV_FTM_SRAM_TBL_FIB0_HASH_KEY:
            tbl_id = FibHost0HashLookupCtl_t;
            if (DRV_FTM_SRAM0 == mem_id)
            {
                fld_id = FibHost0HashLookupCtl_fibHost0Level0HashType_f;
            }
            else if (DRV_FTM_SRAM1 == mem_id)
            {
                fld_id = FibHost0HashLookupCtl_fibHost0Level1HashType_f;
            }
            else if (DRV_FTM_SRAM2 == mem_id)
            {
                fld_id = FibHost0HashLookupCtl_fibHost0Level2HashType_f;
            }
            else if (DRV_FTM_SRAM3 == mem_id)
            {
                fld_id = FibHost0HashLookupCtl_fibHost0Level3HashType_f;
            }
            else if (DRV_FTM_SRAM4 == mem_id)
            {
                fld_id = FibHost0HashLookupCtl_fibHost0Level4HashType_f;
            }
            else if (DRV_FTM_SRAM5 == mem_id)
            {
                fld_id = FibHost0HashLookupCtl_fibHost0Level5HashType_f;
            }
            else if (DRV_FTM_SRAM23 == mem_id)
            {
                tbl_id = FibHost0ExternalHashLookupCtl_t;
                fld_id = FibHost0ExternalHashLookupCtl_externalLevel0HashType_f;
            }
            else if (DRV_FTM_SRAM24 == mem_id)
            {
                tbl_id = FibHost0ExternalHashLookupCtl_t;
                fld_id = FibHost0ExternalHashLookupCtl_externalLevel1HashType_f;
            }
            else if (DRV_FTM_SRAM25 == mem_id)
            {
                tbl_id = FibHost0ExternalHashLookupCtl_t;
                fld_id = FibHost0ExternalHashLookupCtl_externalLevel2HashType_f;
            }
            else if (DRV_FTM_SRAM26 == mem_id)
            {
                tbl_id = FibHost0ExternalHashLookupCtl_t;
                fld_id = FibHost0ExternalHashLookupCtl_externalLevel3HashType_f;
            }
            else
            {
                return DRV_E_INVALID_MEM;
            }

            cmd = DRV_IOR(tbl_id, fld_id);
            DRV_IOCTL(lchip, 0, cmd, drv_poly);
            break;

        case DRV_FTM_SRAM_TBL_FIB1_HASH_KEY:
            tbl_id = FibHost1HashLookupCtl_t;
            if (DRV_FTM_SRAM6 == mem_id)
            {
                fld_id = FibHost1HashLookupCtl_fibHost1Level0HashType_f;
            }
            else if (DRV_FTM_SRAM7 == mem_id)
            {
                fld_id = FibHost1HashLookupCtl_fibHost1Level1HashType_f;
            }
            else if (DRV_FTM_SRAM8 == mem_id)
            {
                fld_id =  DRV_IS_DUET2(lchip)? FibHost1HashLookupCtl_fibHost1Level2HashType_f:
                           FibHost1HashLookupCtl_fibHost1Level0HashType_f;
            }
            else if (DRV_FTM_SRAM9 == mem_id)
            {
                fld_id = DRV_IS_DUET2(lchip)? FibHost1HashLookupCtl_fibHost1Level3HashType_f:
                           FibHost1HashLookupCtl_fibHost1Level1HashType_f;
            }
            else if (DRV_FTM_SRAM10 == mem_id)
            {
                fld_id = FibHost1HashLookupCtl_fibHost1Level4HashType_f;
            }
            else
            {
                return DRV_E_INVALID_MEM;
            }
            cmd = DRV_IOR(tbl_id, fld_id);
            DRV_IOCTL(lchip, 0, cmd, drv_poly);
            break;

        default :
            break;

    }
    return DRV_E_NONE;
}


int32
drv_usw_ftm_set_hash_poly(uint8 lchip, uint32 mem_id, uint8 sram_type, uint32 drv_poly)
{
    uint32 cmd = 0;
    uint32 tbl_id = 0;
    uint32 fld_id = 0;

    switch (sram_type)
    {
        case DRV_FTM_SRAM_TBL_FLOW_HASH_KEY:
            tbl_id = FlowHashLookupCtl_t;
            if (DRV_FTM_SRAM0 == mem_id)
            {
                fld_id = FlowHashLookupCtl_flowLevel0HashType_f;
            }
            else if (DRV_FTM_SRAM1 == mem_id)
            {
                fld_id = FlowHashLookupCtl_flowLevel1HashType_f;
            }
            else if (DRV_FTM_SRAM2 == mem_id)
            {
                fld_id = FlowHashLookupCtl_flowLevel2HashType_f;
            }
            else if (DRV_FTM_SRAM3 == mem_id)
            {
                fld_id = FlowHashLookupCtl_flowLevel3HashType_f;
            }
            else if (DRV_FTM_SRAM4 == mem_id)
            {
                fld_id = FlowHashLookupCtl_flowLevel4HashType_f;
            }
            cmd = DRV_IOW(tbl_id, fld_id);
            DRV_IOCTL(lchip, 0, cmd, &drv_poly);
            break;

        case DRV_FTM_SRAM_TBL_USERID_HASH_KEY:
            tbl_id = UserIdHashLookupCtl_t;
            switch(mem_id)
            {
                case DRV_FTM_SRAM2:
                    fld_id = UserIdHashLookupCtl_scl0Level0HashType_f;
                    break;

                case DRV_FTM_SRAM6:
                    fld_id = DRV_IS_DUET2(lchip)? UserIdHashLookupCtl_userIdLevel0HashType_f:
                                              UserIdHashLookupCtl_scl0Level1HashType_f;
                    break;
                case DRV_FTM_SRAM7:
                    fld_id = DRV_IS_DUET2(lchip)? UserIdHashLookupCtl_userIdLevel1HashType_f:
                                              UserIdHashLookupCtl_scl0Level2HashType_f;

                    break;
                case DRV_FTM_SRAM8:
                    fld_id = DRV_IS_DUET2(lchip)? UserIdHashLookupCtl_userIdLevel2HashType_f:
                                              UserIdHashLookupCtl_scl0Level3HashType_f;

                    break;
                case DRV_FTM_SRAM9:
                    fld_id = DRV_IS_DUET2(lchip)? UserIdHashLookupCtl_userIdLevel3HashType_f:
                                              UserIdHashLookupCtl_scl0Level4HashType_f;

                    break;
                case DRV_FTM_SRAM10:
                    fld_id = UserIdHashLookupCtl_userIdLevel4HashType_f;

                    break;
                default:
                    return DRV_E_INVALID_MEM;
            }
            cmd = DRV_IOW(tbl_id, fld_id);
            DRV_IOCTL(lchip, 0, cmd, &drv_poly);
            break;

        case DRV_FTM_SRAM_TBL_USERID1_HASH_KEY:
            tbl_id = UserIdHashLookupCtl_t;
            switch(mem_id)
            {
                case DRV_FTM_SRAM5:
                    fld_id = UserIdHashLookupCtl_scl1Level0HashType_f;
                    break;


                case DRV_FTM_SRAM8:
                    fld_id = UserIdHashLookupCtl_scl1Level1HashType_f;
                    break;


                case DRV_FTM_SRAM9:
                    fld_id = UserIdHashLookupCtl_scl1Level2HashType_f;
                    break;


                default:
                    return DRV_E_INVALID_MEM;
            }
            cmd = DRV_IOW(tbl_id, fld_id);
            DRV_IOCTL(lchip, 0, cmd, &drv_poly);
            break;



        case DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY:
            tbl_id = EgressXcOamHashLookupCtl_t;
            switch(mem_id)
            {
                case DRV_FTM_SRAM2:
                    fld_id = EgressXcOamHashLookupCtl_egressXcOamLevel0HashType_f;
                    break;
                case DRV_FTM_SRAM5:
                    fld_id = EgressXcOamHashLookupCtl_egressXcOamLevel1HashType_f;
                    break;
                /*TsingMa*/
                case DRV_FTM_SRAM3:
                    fld_id = EgressXcOamHashLookupCtl_egressXcOamLevel0HashType_f;
                    break;
                case DRV_FTM_SRAM4:
                    fld_id = EgressXcOamHashLookupCtl_egressXcOamLevel1HashType_f;
                    break;

                default:
                    return DRV_E_INVALID_MEM;
            }
            cmd = DRV_IOW(tbl_id, fld_id);
            DRV_IOCTL(lchip, 0, cmd, &drv_poly);
            break;

        case DRV_FTM_SRAM_TBL_FIB0_HASH_KEY:
        case DRV_FTM_SRAM_TBL_MAC_HASH_KEY:
            tbl_id = FibHost0HashLookupCtl_t;
            if (DRV_FTM_SRAM0 == mem_id)
            {
                fld_id = FibHost0HashLookupCtl_fibHost0Level0HashType_f;
            }
            else if (DRV_FTM_SRAM1 == mem_id)
            {
                fld_id = FibHost0HashLookupCtl_fibHost0Level1HashType_f;
            }
            else if (DRV_FTM_SRAM2 == mem_id)
            {
                fld_id = FibHost0HashLookupCtl_fibHost0Level2HashType_f;
            }
            else if (DRV_FTM_SRAM3 == mem_id)
            {
                fld_id = FibHost0HashLookupCtl_fibHost0Level3HashType_f;
            }
            else if (DRV_FTM_SRAM4 == mem_id)
            {
                fld_id = FibHost0HashLookupCtl_fibHost0Level4HashType_f;
            }
            else if(DRV_FTM_SRAM5 == mem_id)
            {
                fld_id = FibHost0HashLookupCtl_fibHost0Level5HashType_f;
            }
            else if (DRV_FTM_SRAM23 == mem_id)
            {
                tbl_id = FibHost0ExternalHashLookupCtl_t;
                fld_id = FibHost0ExternalHashLookupCtl_externalLevel0HashType_f;
            }
            else if (DRV_FTM_SRAM24 == mem_id)
            {
                tbl_id = FibHost0ExternalHashLookupCtl_t;
                fld_id = FibHost0ExternalHashLookupCtl_externalLevel1HashType_f;
            }
            else if (DRV_FTM_SRAM25 == mem_id)
            {
                tbl_id = FibHost0ExternalHashLookupCtl_t;
                fld_id = FibHost0ExternalHashLookupCtl_externalLevel2HashType_f;
            }
            else if (DRV_FTM_SRAM26 == mem_id)
            {
                tbl_id = FibHost0ExternalHashLookupCtl_t;
                fld_id = FibHost0ExternalHashLookupCtl_externalLevel3HashType_f;
            }
            else
            {
                return DRV_E_INVALID_MEM;
            }
            cmd = DRV_IOW(tbl_id, fld_id);
            DRV_IOCTL(lchip, 0, cmd, &drv_poly);
            break;

        case DRV_FTM_SRAM_TBL_FIB1_HASH_KEY:
            tbl_id = FibHost1HashLookupCtl_t;
            if (DRV_FTM_SRAM6 == mem_id)
            {
                fld_id = FibHost1HashLookupCtl_fibHost1Level0HashType_f;
            }
            else if (DRV_FTM_SRAM7 == mem_id)
            {
                fld_id = FibHost1HashLookupCtl_fibHost1Level1HashType_f;
            }
            else if (DRV_FTM_SRAM8 == mem_id)
            {
                fld_id =  DRV_IS_DUET2(lchip)? FibHost1HashLookupCtl_fibHost1Level2HashType_f:
                           FibHost1HashLookupCtl_fibHost1Level0HashType_f;
            }
            else if (DRV_FTM_SRAM9 == mem_id)
            {
                fld_id = DRV_IS_DUET2(lchip)? FibHost1HashLookupCtl_fibHost1Level3HashType_f:
                           FibHost1HashLookupCtl_fibHost1Level1HashType_f;
            }
            else if (DRV_FTM_SRAM10 == mem_id)
            {
                fld_id = FibHost1HashLookupCtl_fibHost1Level4HashType_f;
            }
            else
            {
                return DRV_E_INVALID_MEM;
            }
            cmd = DRV_IOW(tbl_id, fld_id);
            DRV_IOCTL(lchip, 0, cmd, &drv_poly);
            g_usw_ftm_master[lchip]->host1_poly_type = drv_poly;
            break;

        default :
            break;

    }
    return DRV_E_NONE;
}

/* sram type refer to drv_ftm_sram_tbl_type_t*/
int32
drv_usw_get_dynamic_ram_couple_mode(uint8 lchip, uint16 sram_type, uint32* couple_mode)
{
    *couple_mode = g_usw_ftm_master[lchip]->couple_mode | DRV_FTM_TBL_COUPLE_MODE(sram_type);

    return DRV_E_NONE;
}

int32
drv_usw_set_dynamic_ram_couple_mode(uint8 lchip, uint32 couple_mode)
{
    g_usw_ftm_master[lchip]->couple_mode = couple_mode;
    return DRV_E_NONE;
}

int32
drv_usw_get_memory_size(uint8 lchip, uint32 mem_id, uint32*p_mem_size)
{
    uint32 table = _drv_usw_ftm_memid_2_table(lchip, mem_id);
    uint32 couple_mode = 0;
    uint8  per_entry_bytes = 0;

    DRV_PTR_VALID_CHECK(p_mem_size);

    if(mem_id <= DRV_FTM_SRAM_MAX)
    {
         couple_mode = DRV_FTM_TBL_COUPLE_MODE(table);
         per_entry_bytes = DRV_BYTES_PER_ENTRY;
    }
    else if(mem_id < DRV_FTM_EDRAM_MAX)
    {/*TM mpls & xgpon*/
        if (g_usw_ftm_master[lchip]->mpls_mode)
        {
            *p_mem_size = (mem_id<DRV_FTM_MIXED2) ? ((DRV_FTM_TBL_MEM_ENTRY_NUM(DRV_FTM_SRAM_TBL_MPLS_HASH_KEY, mem_id))*8):
                ((DRV_FTM_TBL_MEM_ENTRY_NUM(DRV_FTM_SRAM_TBL_MPLS_HASH_AD, mem_id))*24);
        }
        else
        {
            *p_mem_size = (DRV_FTM_TBL_MEM_ENTRY_NUM(DRV_FTM_SRAM_TBL_GEM_PORT, mem_id))*8;
        }
        return DRV_E_NONE;
    }
    else if(mem_id <= DRV_FTM_TCAM_ADM)
    {
        per_entry_bytes = DRV_BYTES_PER_ENTRY;
    }
    else if (mem_id <= DRV_FTM_LPM_TCAM_ADM)
    {
        per_entry_bytes = DRV_LPM_KEY_BYTES_PER_ENTRY;
    }
    else if (mem_id == DRV_FTM_QUEUE_TCAM)
    {
        per_entry_bytes = DRV_BYTES_PER_WORD*2;
    }
    else if (mem_id == DRV_FTM_CID_TCAM)
    {
        per_entry_bytes = DRV_BYTES_PER_WORD;
    }

    *p_mem_size = DRV_FTM_MEM_MAX_ENTRY_NUM(mem_id) * per_entry_bytes * (couple_mode ? 2 : 1);

    return DRV_E_NONE;

 }

int32
drv_usw_ftm_table_id_2_mem_id(uint8 lchip, uint32 tbl_id, uint32 tbl_idx, uint32* p_mem_id, uint32* p_offset)
{
    uint32 mem_bmp = 0;
    uint8  mem_idx;
    uint8 blk_id = 0;

    DRV_INIT_CHECK(lchip);
    DRV_PTR_VALID_CHECK(p_mem_id);
    DRV_PTR_VALID_CHECK(p_offset);

    if(!TABLE_EXT_INFO_PTR(lchip,tbl_id) || (TABLE_EXT_TYPE(lchip, tbl_id) == 1))
    {
        return DRV_E_INVAILD_TYPE;
    }
    if(TABLE_EXT_INFO_TYPE(lchip,tbl_id) == EXT_INFO_TYPE_DYNAMIC ||
        TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_MIXED)
    {
        mem_bmp = DYNAMIC_BITMAP(lchip,tbl_id);
        for(blk_id=0; blk_id < 32; blk_id++)
        {
            if(!IS_BIT_SET(mem_bmp, blk_id))
            {
                continue;
            }

            mem_idx = (TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_MIXED)? blk_id + DRV_FTM_MIXED0:blk_id;

            if((DYNAMIC_START_INDEX(lchip,tbl_id, mem_idx) <= tbl_idx) && (tbl_idx <= DYNAMIC_END_INDEX(lchip, tbl_id, mem_idx)))
            {
                *p_mem_id = mem_idx;
                if (DYNAMIC_ACCESS_MODE(lchip, tbl_id) != DYNAMIC_DEFAULT && (TABLE_EXT_INFO_TYPE(lchip, tbl_id) != EXT_INFO_TYPE_MIXED))
                {
                    *p_offset = (tbl_idx - DYNAMIC_START_INDEX(lchip, tbl_id, mem_idx)+DYNAMIC_MEM_OFFSET(lchip, tbl_id, mem_idx))*DRV_BYTES_PER_ENTRY;
                }
                else
                {
                    *p_offset = (tbl_idx - DYNAMIC_START_INDEX(lchip, tbl_id, mem_idx)+DYNAMIC_MEM_OFFSET(lchip, tbl_id, mem_idx))*TABLE_ENTRY_SIZE(lchip, tbl_id);
                }
                return DRV_E_NONE;
            }
        }
    }
    else
    {
        uint32 mem_id_max = 0;
        uint32 mem_id_base = 0;
        switch(TABLE_EXT_INFO_TYPE(lchip,tbl_id))
        {
            case EXT_INFO_TYPE_TCAM:
                mem_id_max = DRV_FTM_TCAM_KEYM-DRV_FTM_TCAM_KEY0;
                mem_id_base = DRV_FTM_TCAM_KEY0;
                break;
            case EXT_INFO_TYPE_TCAM_AD:
                mem_id_max = DRV_FTM_TCAM_ADM-DRV_FTM_TCAM_AD0;
                mem_id_base = DRV_FTM_TCAM_AD0;
                break;
            case EXT_INFO_TYPE_LPM_TCAM_IP:
            case EXT_INFO_TYPE_LPM_TCAM_NAT:
                mem_id_max = DRV_FTM_LPM_TCAM_KEYM -DRV_FTM_LPM_TCAM_KEY0;
                mem_id_base = DRV_FTM_LPM_TCAM_KEY0;
                break;
            case EXT_INFO_TYPE_TCAM_LPM_AD:
            case EXT_INFO_TYPE_TCAM_NAT_AD:
                mem_id_max = DRV_FTM_TCAM_ADM-DRV_FTM_TCAM_AD0;
                mem_id_base = DRV_FTM_LPM_TCAM_AD0;
                break;
            default:
                DRV_FTM_DBG_OUT("Invalid table id:%d,line:%d\n",tbl_id,__LINE__);
                return DRV_E_INVAILD_TYPE;
        }

        mem_bmp = TCAM_BITMAP(lchip,tbl_id);
        for(mem_idx=0; mem_idx < mem_id_max; mem_idx++)
        {
            if(!IS_BIT_SET(mem_bmp, mem_idx))
            {
                continue;
            }

            if((TCAM_START_INDEX(lchip,tbl_id, mem_idx) <= tbl_idx) && (tbl_idx <= TCAM_END_INDEX(lchip,tbl_id, mem_idx)))
            {
                *p_mem_id = mem_idx + mem_id_base;
                *p_offset = (tbl_idx-TCAM_START_INDEX(lchip, tbl_id, mem_idx))*TABLE_ENTRY_SIZE(lchip,tbl_id);
                return DRV_E_NONE;
            }
        }
    }

    DRV_FTM_DBG_OUT("Invalid table id, line:%d\n",__LINE__);
    return DRV_E_INVAILD_TYPE;
}


int32
drv_usw_ftm_dump_table_info(uint8 lchip, uint32 tbl_id)
{
    uint8 blk_id = 0;

    if ( (NULL == TABLE_INFO_PTR(lchip, tbl_id)) || 0 == TABLE_FIELD_NUM(lchip, tbl_id))
    {
        DRV_FTM_DBG_OUT("Empty table:%d\r\n", tbl_id);
        return 0;
    }

    DRV_FTM_DBG_OUT("%-20s:%10s\n", "Name", TABLE_NAME(lchip, tbl_id));
    DRV_FTM_DBG_OUT("%-20s:0x%x\n", "DataBase", TABLE_DATA_BASE(lchip, tbl_id, 0));
    DRV_FTM_DBG_OUT("%-20s:%d\n", "MaxIndex", TABLE_MAX_INDEX(lchip, tbl_id));
    DRV_FTM_DBG_OUT("%-20s:%d\n", "EntrySize", TABLE_ENTRY_SIZE(lchip, tbl_id));
    DRV_FTM_DBG_OUT("%-20s:%d\n", "MaxIndex", TABLE_MAX_INDEX(lchip, tbl_id));

    if (NULL == TABLE_EXT_INFO_PTR(lchip, tbl_id))
    {
        return 0;
    }

    if (TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_DYNAMIC ||
        TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_MIXED)
    {
        DRV_FTM_DBG_OUT("%-20s:%d\n", "ExtType", TABLE_EXT_INFO_TYPE(lchip, tbl_id));
        DRV_FTM_DBG_OUT("%-20s:0x%x\n", "Bitmap", DYNAMIC_BITMAP(lchip, tbl_id));
        for (blk_id = 0; blk_id < 32; blk_id++)
        {
            uint32 mem_id = 0;
            if (!IS_BIT_SET(DYNAMIC_BITMAP(lchip, tbl_id), blk_id))
            {
                continue;
            }

            mem_id = (TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_MIXED)? blk_id+DRV_FTM_MIXED0:blk_id;

            DRV_FTM_DBG_OUT("\n==================================================\n");
            DRV_FTM_DBG_OUT("%-20s:%d\n", "BlkId", mem_id);
            DRV_FTM_DBG_OUT("%-20s:0x%x\n", "DataBase", DYNAMIC_DATA_BASE(lchip, tbl_id, mem_id, 0));
            DRV_FTM_DBG_OUT("%-20s:%d\n", "EntryNumber", DYNAMIC_ENTRY_NUM(lchip, tbl_id, mem_id));
            DRV_FTM_DBG_OUT("%-20s:%d\n", "StartIndex", DYNAMIC_START_INDEX(lchip, tbl_id, mem_id));
            DRV_FTM_DBG_OUT("%-20s:%d\n", "EndIndex", DYNAMIC_END_INDEX(lchip, tbl_id, mem_id));
        }
    }
    else if(TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_TCAM ||
        TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_TCAM_AD ||
    TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_TCAM_LPM_AD ||
    TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_TCAM_NAT_AD ||
    TABLE_EXT_INFO_TYPE(lchip, tbl_id) == EXT_INFO_TYPE_LPM_TCAM_IP)
    {

        DRV_FTM_DBG_OUT("%-20s:%x\n", "keySize", TCAM_KEY_SIZE(lchip, tbl_id));
        DRV_FTM_DBG_OUT("%-20s:%x\n", "KeyType", TCAM_KEY_TYPE(lchip, tbl_id));
        DRV_FTM_DBG_OUT("%-20s:%x\n", "KeyModule", TCAM_KEY_MODULE(lchip, tbl_id));
        DRV_FTM_DBG_OUT("%-20s:%x\n", "Bitmap", TCAM_BITMAP(lchip, tbl_id));
        for (blk_id = 0; blk_id < 32; blk_id++)
        {
            if (!IS_BIT_SET(TCAM_BITMAP(lchip, tbl_id), blk_id))
            {
                continue;
            }
            DRV_FTM_DBG_OUT("\n==================================================\n");
            DRV_FTM_DBG_OUT("%-20s:%d\n", "BlkId", blk_id);
            DRV_FTM_DBG_OUT("%-20s:0x%x\n", "DataBase", TCAM_DATA_BASE(lchip, tbl_id, blk_id, 0));
            DRV_FTM_DBG_OUT("%-20s:0x%x\n", "MaskBase", TCAM_MASK_BASE(lchip, tbl_id, blk_id, 0));

            DRV_FTM_DBG_OUT("%-20s:%d\n", "EntryNumber", TCAM_ENTRY_NUM(lchip, tbl_id, blk_id));
            DRV_FTM_DBG_OUT("%-20s:%d\n", "StartIndex", TCAM_START_INDEX(lchip, tbl_id, blk_id));
            DRV_FTM_DBG_OUT("%-20s:%d\n", "EndIndex", TCAM_END_INDEX(lchip, tbl_id, blk_id));
        }
    }


  return 0;
}


/*==========================================================
*        ECC recovery
============================================================*/


int32
drv_ftm_get_tbl_detail(uint8 lchip, drv_ftm_tbl_detail_t* p_tbl_info)
{
    DRV_PTR_VALID_CHECK(p_tbl_info);

    switch(p_tbl_info->type)
    {
    case DRV_FTM_TBL_TYPE:
        DRV_TBL_ID_VALID_CHECK(lchip, p_tbl_info->tbl_id);
        if (NULL == TABLE_EXT_INFO_PTR(lchip, p_tbl_info->tbl_id) || (TABLE_EXT_TYPE(lchip, p_tbl_info->tbl_id) == 1))
        {
            p_tbl_info->tbl_type = DRV_FTM_TABLE_STATIC;
            return 0;
        }

        switch(TABLE_EXT_INFO_TYPE(lchip, p_tbl_info->tbl_id))
        {
        case EXT_INFO_TYPE_TCAM:
        case EXT_INFO_TYPE_LPM_TCAM_IP:
        case EXT_INFO_TYPE_LPM_TCAM_NAT:
        case EXT_INFO_TYPE_STATIC_TCAM_KEY:
            p_tbl_info->tbl_type = DRV_FTM_TABLE_TCAM_KEY;

            break;

        default:
            p_tbl_info->tbl_type = DRV_FTM_TABLE_DYNAMIC;
        }

        break;

    default:
        return DRV_E_INVAILD_TYPE;
    }

    return 0;
}

int32
_drv_ftm_get_mem_type(uint8 lchip, drv_ftm_mem_info_t* p_mem_info)
{
   uint16 mem_id = 0;
   uint8 detail_type = 0;
   uint8 mem_type = 0;

   mem_id = p_mem_info->mem_id;

   mem_type = DRV_FTM_MEM_DYNAMIC;

   if ( mem_id  <= DRV_FTM_SRAM10)
   {
       detail_type = DRV_FTM_MEM_DYNAMIC_KEY;
       p_mem_info->mem_sub_id = mem_id - DRV_FTM_SRAM0;
   }
   else if((mem_id >= DRV_FTM_SRAM11) && (mem_id  <= (DRV_IS_DUET2(lchip) ? DRV_FTM_SRAM15 : DRV_FTM_SRAM16)))
   {
       detail_type = DRV_FTM_MEM_DYNAMIC_AD;
       p_mem_info->mem_sub_id = mem_id - DRV_FTM_SRAM11;
   }
   else if((mem_id >= (DRV_IS_DUET2(lchip) ? DRV_FTM_SRAM16 : DRV_FTM_SRAM17)) && (mem_id  <= DRV_FTM_SRAM22))
   {
       detail_type = DRV_FTM_MEM_DYNAMIC_EDIT;
       p_mem_info->mem_sub_id = mem_id - (DRV_IS_DUET2(lchip) ? DRV_FTM_SRAM16 : DRV_FTM_SRAM17);
   }
   else if(mem_id >= DRV_FTM_TCAM_KEY0 && mem_id  <= DRV_FTM_TCAM_KEY18)
   {
       mem_type = DRV_FTM_MEM_TCAM;
       detail_type = DRV_FTM_MEM_TCAM_FLOW_KEY;
       p_mem_info->mem_sub_id = mem_id - DRV_FTM_TCAM_KEY0;

   }
   else if(mem_id >= DRV_FTM_LPM_TCAM_KEY0 && mem_id  <= DRV_FTM_LPM_TCAM_KEY11)
   {
       mem_type = DRV_FTM_MEM_TCAM;
       detail_type = DRV_FTM_MEM_TCAM_LPM_KEY;
       p_mem_info->mem_sub_id = mem_id - DRV_FTM_LPM_TCAM_KEY0;
   }
   else if(mem_id == DRV_FTM_CID_TCAM)
   {
       mem_type = DRV_FTM_MEM_TCAM;
       detail_type = DRV_FTM_MEM_TCAM_CID_KEY;
       p_mem_info->mem_sub_id = mem_id - DRV_FTM_CID_TCAM;
   }
   else if(mem_id == DRV_FTM_QUEUE_TCAM)
   {
       mem_type = DRV_FTM_MEM_TCAM;
       detail_type = DRV_FTM_MEM_TCAM_QUEUE_KEY;
       p_mem_info->mem_sub_id = mem_id - DRV_FTM_QUEUE_TCAM;
   }
   else if(mem_id >= DRV_FTM_TCAM_AD0 && mem_id  <= DRV_FTM_TCAM_AD18)
   {
       detail_type = DRV_FTM_MEM_TCAM_FLOW_AD;
       p_mem_info->mem_sub_id = mem_id - DRV_FTM_TCAM_AD0;
   }
   else if(mem_id >= DRV_FTM_LPM_TCAM_AD0 && mem_id  <= DRV_FTM_LPM_TCAM_AD11)
   {
       detail_type = DRV_FTM_MEM_TCAM_LPM_AD;
       p_mem_info->mem_sub_id = mem_id - DRV_FTM_LPM_TCAM_AD0;
   }
   else if(mem_id >= DRV_FTM_MIXED0 && mem_id  <= DRV_FTM_MIXED1)
   {
       detail_type = DRV_FTM_MEM_MIXED_KEY;
       p_mem_info->mem_sub_id = mem_id - DRV_FTM_MIXED0;
   }
   else if(mem_id == DRV_FTM_MIXED2)
   {
       detail_type = DRV_FTM_MEM_MIXED_AD;
       p_mem_info->mem_sub_id = mem_id - DRV_FTM_MIXED2;
   }
   else if(mem_id == DRV_FTM_MIXED2)
   {
       detail_type = DRV_FTM_MEM_MIXED_CAM;
       p_mem_info->mem_sub_id = mem_id - DRV_FTM_MIXED2;
   }
   else if(mem_id >= DRV_FTM_SRAM23 && mem_id  <= DRV_FTM_SRAM29)
   {
       detail_type = DRV_FTM_MEM_SHARE_MEM;
       p_mem_info->mem_sub_id = mem_id - DRV_FTM_SRAM23;
   }
   else
   {
       return DRV_E_INVAILD_TYPE;
   }

   if (p_mem_info->info_type == DRV_FTM_MEM_INFO_MEM_TYPE_DETAIL)
   {
       p_mem_info->mem_type = detail_type;
   }
   else
   {
       p_mem_info->mem_type = mem_type;
   }

   return 0;
}

int32
_drv_ftm_get_mem_tbl_index(uint8 lchip, drv_ftm_mem_info_t* p_mem_info)
{
   uint32 tbl_id = 0;
   uint32 index = 0;
   uint32 mem_id = 0;

   mem_id = p_mem_info->mem_id;

   tbl_id = DsFibHost0MacHashKey_t;

   if (!IS_BIT_SET(DYNAMIC_BITMAP(lchip, tbl_id), mem_id))
   {
       return DRV_E_INVAILD_TYPE;
   }

   index = DYNAMIC_START_INDEX(lchip, tbl_id, mem_id) + (p_mem_info->mem_offset)/TABLE_ENTRY_SIZE(lchip, tbl_id);

   if (index >  DYNAMIC_END_INDEX(lchip, tbl_id, mem_id))
   {
       return DRV_E_INVAILD_TYPE;
   }

   if (DRV_IS_DUET2(lchip))
   {
       uint32 cmd = 0;
       uint32 hash_type = 0;
       uint8 size = 0;

       cmd = DRV_IOR(DsFibHost0MacHashKey_t, DsFibHost0MacHashKey_hashType_f);
       DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &hash_type));

       switch(hash_type)
       {
       case FIBHOST0PRIMARYHASHTYPE_FCOE:
           p_mem_info->table_id = DsFibHost0FcoeHashKey_t;
           break;

       case FIBHOST0PRIMARYHASHTYPE_IPV4:
           p_mem_info->table_id = DsFibHost0Ipv4HashKey_t;
           break;

       case FIBHOST0PRIMARYHASHTYPE_IPV6MCAST:
           p_mem_info->table_id = DsFibHost0Ipv6McastHashKey_t;
           break;

       case FIBHOST0PRIMARYHASHTYPE_IPV6UCAST:
           p_mem_info->table_id = DsFibHost0Ipv6UcastHashKey_t;
           break;

       case FIBHOST0PRIMARYHASHTYPE_MAC:
           p_mem_info->table_id = DsFibHost0MacHashKey_t;
           break;

       case FIBHOST0PRIMARYHASHTYPE_MACIPV6MCAST:
           p_mem_info->table_id = DsFibHost0MacIpv6McastHashKey_t;
           break;

       case FIBHOST0PRIMARYHASHTYPE_TRILL:
           p_mem_info->table_id = DsFibHost0TrillHashKey_t;
           break;

       default :
          return DRV_E_INVAILD_TYPE;
       }

       size = TABLE_ENTRY_SIZE(lchip, tbl_id) / DRV_BYTES_PER_ENTRY;

       if (p_mem_info->index % size == 0)
       {
           p_mem_info->index = index;
       }
       else
       {
           p_mem_info->index = 0xFFFFFFFF;
       }

   }
   else
   {
       p_mem_info->table_id = tbl_id;
       p_mem_info->index = index;
   }

   return 0;
}
int32
drv_usw_ftm_get_tcam_memory_info(uint8 lchip, uint32 mem_id, uint32* p_mem_addr, uint32* p_entry_num, uint32* p_entry_size)
{
    DRV_PTR_VALID_CHECK(p_entry_size);
    DRV_PTR_VALID_CHECK(p_mem_addr);
    DRV_PTR_VALID_CHECK(p_entry_num);

    if(mem_id <= DRV_FTM_TCAM_KEYM)
    {
        *p_entry_size = 7;
    }
    else if(mem_id <= DRV_FTM_LPM_TCAM_KEYM)
    {
        *p_entry_size = 4;
    }
    else if(mem_id == DRV_FTM_QUEUE_TCAM)
    {
        *p_entry_size = 5;
    }
    else
    {
        *p_entry_size = 2;
    }

    *p_entry_num = DRV_FTM_MEM_MAX_ENTRY_NUM(mem_id);
    *p_mem_addr = DRV_FTM_MEM_HW_DATA_BASE(mem_id);

    return DRV_E_NONE;
}
int32
drv_ftm_get_mem_info(uint8 lchip, drv_ftm_mem_info_t* p_mem_info)
{
    DRV_PTR_VALID_CHECK(p_mem_info);

    switch(p_mem_info->info_type)
    {
    case DRV_FTM_MEM_INFO_MEM_TYPE:
    case DRV_FTM_MEM_INFO_MEM_TYPE_DETAIL:
        _drv_ftm_get_mem_type(lchip, p_mem_info);
        break;

    case DRV_FTM_MEM_INFO_TBL_INDEX:
        _drv_ftm_get_mem_tbl_index(lchip, p_mem_info);
        break;

    case DRV_FTM_MEM_INFO_ENTRY_ENUM:
        {
            uint32 table = _drv_usw_ftm_memid_2_table(lchip, p_mem_info->mem_id);
            p_mem_info->entry_num = DRV_FTM_TBL_MEM_ENTRY_NUM(table, p_mem_info->mem_id);
        }
        break;

    default:
        break;
    }

    return 0;
}

int32 drv_usw_ftm_misc_config(uint8 lchip)
{
    if(DRV_IS_TSINGMA(lchip))
    {
        uint8 extern_en = 0;
        uint32 cmd = 0;
        uint32 bitmap0 = 0;
        BufStoreFreeListCtl_m  buf_free_list_ctl;
        uint32 sram_array0[16] = {0};

        cmd = DRV_IOR(BufStoreFreeListCtl_t, DRV_ENTRY_FLAG);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buf_free_list_ctl));

        _sys_usw_ftm_get_edram_bitmap(lchip, DRV_FTM_SRAM_TBL_MAC_HASH_KEY,   &bitmap0,   sram_array0);
        extern_en = IS_BIT_SET(bitmap0, DRV_USW_FTM_EXTERN_MEM_ID_BASE);

        SetBufStoreFreeListCtl(V, freeListTailPtrCfg_f, &buf_free_list_ctl, extern_en ? 0xfff:0x1fff);

        cmd = DRV_IOW(BufStoreFreeListCtl_t, DRV_ENTRY_FLAG);
        DRV_IF_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &buf_free_list_ctl));
    }

    return 0;
}
int32 drv_usw_ftm_adjust_mac_table(uint8 lchip, drv_ftm_profile_info_t* profile)
{
    uint8 tbl_idx;
    drv_ftm_tbl_info_t tmp_mac_info;
    drv_ftm_profile_info_t tmp_profile;

    DRV_PTR_VALID_CHECK(profile);

    /*clear old fdb table*/
    for(tbl_idx=0; tbl_idx < 32; tbl_idx++)
    {
        if(DRV_IS_BIT_SET(DRV_FTM_TBL_MEM_BITMAP(DRV_FTM_SRAM_TBL_MAC_HASH_KEY), tbl_idx))
        {
            DRV_FTM_REMOVE_TABLE(tbl_idx, DRV_FTM_SRAM_TBL_MAC_HASH_KEY, 0, DRV_FTM_TBL_MEM_ENTRY_NUM(DRV_FTM_SRAM_TBL_MAC_HASH_KEY, tbl_idx));
        }
    }
    sal_memset(&tmp_mac_info, 0, sizeof(tmp_mac_info));
    for(tbl_idx=0; tbl_idx < profile->tbl_info_size; tbl_idx++)
    {
        if(profile->tbl_info[tbl_idx].tbl_id == DRV_FTM_SRAM_TBL_MAC_HASH_KEY)
        {
            sal_memcpy(&tmp_mac_info, profile->tbl_info+tbl_idx, sizeof(tmp_mac_info));
            break;
        }
    }
    tmp_profile.tbl_info_size = 1;
    tmp_profile.tbl_info = &tmp_mac_info;
    DRV_IF_ERROR_RETURN(_drv_usw_ftm_set_profile_user_define_edram(lchip, &tmp_profile));
    DRV_IF_ERROR_RETURN(drv_usw_ftm_alloc_sram(lchip, DRV_FTM_SRAM_TBL_MAC_HASH_KEY, DRV_FTM_SRAM_TBL_MAC_HASH_KEY+1, 1));
    DRV_IF_ERROR_RETURN(drv_usw_ftm_lookup_ctl_init(lchip));

    return DRV_E_NONE;
}
int32 drv_usw_ftm_adjust_flow_tcam(uint8 lchip, uint8 expand_key, uint8 compress_key)
{
    uint8 key_id;
    uint8 mem_id[8] = {DRV_FTM_TCAM_KEY15, DRV_FTM_TCAM_KEY16, DRV_FTM_TCAM_KEY17, DRV_FTM_TCAM_KEY18};
    uint32 entry_size = 2048;

    for(key_id=0; key_id < 8; key_id++)
    {
        if(DRV_IS_BIT_SET(expand_key, key_id))
        {
            DRV_FTM_ADD_KEY(mem_id[key_id%4], key_id+DRV_FTM_TCAM_TYPE_IGS_ACL0, 0, entry_size);
            DRV_FTM_ADD_KEY_BYTYPE(mem_id[key_id%4], key_id+DRV_FTM_TCAM_TYPE_IGS_ACL0, 0, 0, entry_size);
        }

        if(DRV_IS_BIT_SET(compress_key, key_id))
        {
            DRV_FTM_REMOVE_KEY(mem_id[key_id%4], key_id+DRV_FTM_TCAM_TYPE_IGS_ACL0, 0, entry_size);
            DRV_FTM_REMOVE_KEY_BYTYPE(mem_id[key_id%4], key_id+DRV_FTM_TCAM_TYPE_IGS_ACL0, 0);
        }
    }

    DRV_IF_ERROR_RETURN(drv_usw_ftm_alloc_tcam(lchip, DRV_FTM_TCAM_TYPE_IGS_ACL0, DRV_FTM_TCAM_TYPE_EGS_ACL0, 1));
    DRV_IF_ERROR_RETURN(drv_usw_tcam_ctl_init(lchip));

    return DRV_E_NONE;
}
int32 drv_usw_ftm_reset_tcam_table(uint8 lchip)
{
    uint16 loop;
    uint32 cmd = 0;
    uint32 value_0 = 0;
    uint32 value_1 = 1;

    cmd = DRV_IOW(LpmTcamInit_t, LpmTcamInit_init_f);
    DRV_IOCTL(lchip, 0, cmd, &value_0);
    DRV_IOCTL(lchip, 0, cmd, &value_1);

    cmd = DRV_IOW(FlowTcamInit_t, FlowTcamInit_init_f);
    DRV_IOCTL(lchip, 0, cmd, &value_0);
    DRV_IOCTL(lchip, 0, cmd, &value_1);

    cmd = DRV_IOR(LpmTcamInitDone_t, LpmTcamInitDone_initDone_f);
    DRV_IOCTL(lchip, 0, cmd, &value_0);
    if (!value_0)
    {
        return DRV_E_INIT_FAILED;
    }

    cmd = DRV_IOR(FlowTcamInitDone_t, FlowTcamInitDone_initDone_f);
    DRV_IOCTL(lchip, 0, cmd, &value_0);
    if (!value_0)
    {
        return DRV_E_INIT_FAILED;
    }

    cmd = DRV_IOD(DsCategoryIdPairTcamKey_t, DRV_ENTRY_FLAG);
    for(loop=0; loop < TABLE_MAX_INDEX(lchip, DsCategoryIdPairTcamKey_t); loop++)
    {
        DRV_IOCTL(lchip, 0, cmd, &value_0);
    }

    cmd = DRV_IOD(DsQueueMapTcamKey_t, DRV_ENTRY_FLAG);
    for(loop=0; loop < TABLE_MAX_INDEX(lchip, DsQueueMapTcamKey_t); loop++)
    {
        DRV_IOCTL(lchip, 0, cmd, &value_0);
    }

    return DRV_E_NONE;

}
int32 drv_usw_ftm_reset_dynamic_table(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 value_0 = 0;
    uint32 value_1 = 1;

    cmd = DRV_IOW(DynamicKeyInit_t, DynamicKeyInit_init_f);
    DRV_IOCTL(lchip, 0, cmd, &value_0);
    DRV_IOCTL(lchip, 0, cmd, &value_1);

    cmd = DRV_IOW(DynamicEditInit_t, DynamicEditInit_init_f);
    DRV_IOCTL(lchip, 0, cmd, &value_0);
    DRV_IOCTL(lchip, 0, cmd, &value_1);

    cmd = DRV_IOW(DynamicAdInit_t, DynamicAdInit_init_f);
    DRV_IOCTL(lchip, 0, cmd, &value_0);
    DRV_IOCTL(lchip, 0, cmd, &value_1);

    cmd = DRV_IOR(DynamicKeyInitDone_t, DynamicKeyInitDone_initDone_f);
    DRV_IOCTL(lchip, 0, cmd, &value_0);
    if (!value_0)
    {
        return DRV_E_INIT_FAILED;
    }

    cmd = DRV_IOR(DynamicEditInitDone_t, DynamicEditInitDone_initDone_f);
    DRV_IOCTL(lchip, 0, cmd, &value_0);
    if (!value_0)
    {
        return DRV_E_INIT_FAILED;
    }

    cmd = DRV_IOR(DynamicAdInitDone_t, DynamicAdInitDone_initDone_f);
    DRV_IOCTL(lchip, 0, cmd, &value_0);
    if (!value_0)
    {
        return DRV_E_INIT_FAILED;
    }

    return DRV_E_NONE;
}

