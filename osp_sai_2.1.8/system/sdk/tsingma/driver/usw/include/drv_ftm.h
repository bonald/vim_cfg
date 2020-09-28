/**
 @file drv_ftm.h

 @date 2011-11-16

 @version v2.0

 alloc memory and offset
*/

#ifndef _SYS_GOLDENGATE_FTM_H
#define _SYS_GOLDENGATE_FTM_H
#ifdef __cplusplus
extern "C" {
#endif

#define DRV_FTM_MEM_MAX (32)

#define DRV_FTM_INIT_CHECK(lchip) \
    do { \
        if (NULL == g_usw_ftm_master[lchip]){ \
            return DRV_E_NOT_INIT; } \
    } while (0)



enum drv_ftm_cam_type_e
{
    DRV_FTM_CAM_TYPE_INVALID        = 0,
    DRV_FTM_CAM_TYPE_FIB_HOST0,
    DRV_FTM_CAM_TYPE_FIB_HOST1,
    DRV_FTM_CAM_TYPE_SCL,
    DRV_FTM_CAM_TYPE_XC,
    DRV_FTM_CAM_TYPE_FLOW,
    DRV_FTM_CAM_TYPE_MAX
};
typedef enum drv_ftm_cam_type_e drv_ftm_cam_type_t;


#define DRV_FTM_TCAM_SIZE_80  0
#define DRV_FTM_TCAM_SIZE_160 1
#define DRV_FTM_TCAM_SIZE_320 2
#define DRV_FTM_TCAM_SIZE_640 3

enum drv_ftm_tcam_key_type_e
{
    DRV_FTM_TCAM_TYPE_IGS_ACL0,
    DRV_FTM_TCAM_TYPE_IGS_ACL1,
    DRV_FTM_TCAM_TYPE_IGS_ACL2,
    DRV_FTM_TCAM_TYPE_IGS_ACL3,
    DRV_FTM_TCAM_TYPE_IGS_ACL4,
    DRV_FTM_TCAM_TYPE_IGS_ACL5,
    DRV_FTM_TCAM_TYPE_IGS_ACL6,
    DRV_FTM_TCAM_TYPE_IGS_ACL7,

    DRV_FTM_TCAM_TYPE_EGS_ACL0,
    DRV_FTM_TCAM_TYPE_EGS_ACL1,
    DRV_FTM_TCAM_TYPE_EGS_ACL2,
    DRV_FTM_TCAM_TYPE_IGS_USERID0,
    DRV_FTM_TCAM_TYPE_IGS_USERID1,
    DRV_FTM_TCAM_TYPE_IGS_USERID2,
    DRV_FTM_TCAM_TYPE_IGS_USERID3,

    DRV_FTM_TCAM_TYPE_IGS_LPM_ALL,          /* just for cmode use 13*/

     /*can not change start*/
    DRV_FTM_TCAM_TYPE_IGS_LPM0DA,           /* private ipda */
    DRV_FTM_TCAM_TYPE_IGS_LPM0SA,           /* private ipsa */
    DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB,        /* public ipda, include lookup1 & lookup 2 */
    DRV_FTM_TCAM_TYPE_IGS_LPM0SAPUB,        /* public ipsa include lookup1 & lookup 2 */
#if 0
    DRV_FTM_TCAM_TYPE_IGS_LPM0DA_V6_HALF,
    DRV_FTM_TCAM_TYPE_IGS_LPM0SA_V6_HALF,           /* private ipsa */
    DRV_FTM_TCAM_TYPE_IGS_LPM0DAPUB_V6_HALF,        /* public ipda, include*/
    DRV_FTM_TCAM_TYPE_IGS_LPM0SAPUB_V6_HALF,        /* public ipsa include*/
#endif

    DRV_FTM_TCAM_TYPE_IGS_LPM1,

     /*can not change end*/
    DRV_FTM_TCAM_TYPE_STATIC_TCAM,

    DRV_FTM_KEY_TYPE_IPV4_UCAST,
    DRV_FTM_KEY_TYPE_IPV4_NAT,
    DRV_FTM_KEY_TYPE_IPV6_UCAST,
    DRV_FTM_KEY_TYPE_IPV6_UCAST_HALF,

    DRV_FTM_TCAM_TYPE_MAX
};
typedef enum drv_ftm_tcam_key_type_e drv_ftm_tcam_key_type_t;


enum drv_ftm_sram_tbl_type_e
{
    DRV_FTM_SRAM_TBL_MAC_HASH_KEY,
    DRV_FTM_SRAM_TBL_FIB0_HASH_KEY,
    DRV_FTM_SRAM_TBL_FIB1_HASH_KEY,
    DRV_FTM_SRAM_TBL_FLOW_HASH_KEY,
    DRV_FTM_SRAM_TBL_IPFIX_HASH_KEY,
    DRV_FTM_SRAM_TBL_USERID_HASH_KEY,
    DRV_FTM_SRAM_TBL_USERID1_HASH_KEY,
    DRV_FTM_SRAM_TBL_XCOAM_HASH_KEY,

    DRV_FTM_SRAM_TBL_LPM_LKP_KEY0,
    DRV_FTM_SRAM_TBL_LPM_LKP_KEY1,

    DRV_FTM_SRAM_TBL_LPM_LKP_KEY2,
    DRV_FTM_SRAM_TBL_LPM_LKP_KEY3,

    DRV_FTM_SRAM_TBL_DSMAC_AD,
    DRV_FTM_SRAM_TBL_DSIP_AD,
    DRV_FTM_SRAM_TBL_FLOW_AD,
    DRV_FTM_SRAM_TBL_IPFIX_AD,
    DRV_FTM_SRAM_TBL_USERID_AD,
    DRV_FTM_SRAM_TBL_USERID_AD1,

    DRV_FTM_SRAM_TBL_NEXTHOP,
    DRV_FTM_SRAM_TBL_FWD,
    DRV_FTM_SRAM_TBL_MET,
    DRV_FTM_SRAM_TBL_EDIT,

    DRV_FTM_SRAM_TBL_OAM_APS,
    DRV_FTM_SRAM_TBL_OAM_LM,
    DRV_FTM_SRAM_TBL_OAM_MEP,
    DRV_FTM_SRAM_TBL_OAM_MA,
    DRV_FTM_SRAM_TBL_OAM_MA_NAME,

    DRV_FTM_SRAM_TBL_MPLS_HASH_KEY,
    DRV_FTM_SRAM_TBL_MPLS_HASH_AD,
    DRV_FTM_SRAM_TBL_GEM_PORT,
    DRV_FTM_SRAM_TBL_MAX
};
typedef enum drv_ftm_sram_tbl_type_e drv_ftm_sram_tbl_type_t;


struct drv_ftm_tbl_info_s
{
    uint32  tbl_id;                             /**[GB]ctc_ftm_tbl_type_t*/
    uint32 mem_bitmap;                          /**[GB]Table allocation in which SRAM*/
    uint32 mem_start_offset[DRV_FTM_MEM_MAX];  /**[GB]Start Offset of SRAM*/
    uint32 mem_entry_num[DRV_FTM_MEM_MAX];     /**[GB]Entry number in SRAM*/
};
typedef struct drv_ftm_tbl_info_s drv_ftm_tbl_info_t;

/**
 @brief Profile key information
*/
struct drv_ftm_key_info_s
{
    uint8  key_size;                    /**< [HB.GB.GG]Value = {1,2,4,8}, indicates {80b,160b,320b,640b}. */
    uint32 max_key_index;               /**< [HB.GB]Key total number. key_max_index * key_size = consumed 80b tcam entry. */
    uint8  key_media;                   /**< [HB.GB]ctc_ftm_key_location_t*/
    uint8  key_id;                      /**< [HB.GB.GG] drv_ftm_key_type_t*/

    uint32 tcam_bitmap;                         /**< [GG]Tcam Key tcam bitmap*/
    uint32 tcam_start_offset[DRV_FTM_MEM_MAX]; /**< [GG]Start Offset of TCAM*/
    uint32 tcam_entry_num[DRV_FTM_MEM_MAX];    /**< [GG]Entry number in TCAM*/
};
typedef struct drv_ftm_key_info_s drv_ftm_key_info_t;


struct drv_ftm_cam_s
{
    uint8 conflict_cam_num[DRV_FTM_CAM_TYPE_MAX];
};
typedef struct drv_ftm_cam_s drv_ftm_cam_t;


struct drv_ftm_profile_info_s
{
    drv_ftm_key_info_t* key_info;   /**< [HB.GB]Profile key information*/
    uint16 key_info_size;           /**< [HB.GB]Size of key_info, multiple of sizeof(ctc_ftm_key_info_t) */
    uint8 profile_type;             /**< [GB]Profile type, refer to ctc_ftm_profile_type_t*/
    uint8 lpm_mode;

    uint32 couple_mode:1;
    uint32 napt_enable:1;
    uint32 mpls_mode:2;             /* 0,disable; 1:not couple; 2,couple mode*/
    uint32 scl_mode:1;
    uint32 rsv:27;
    drv_ftm_tbl_info_t* tbl_info;               /**< [GB]table information  */
    uint16 tbl_info_size;                       /**< [GB]Size of tbl_info, multiple of sizeof(ctc_ftm_tbl_info_t) */

    drv_ftm_cam_t cam_info;
};
typedef struct drv_ftm_profile_info_s drv_ftm_profile_info_t;

enum drv_ftm_info_type_e
{
    DRV_FTM_INFO_TYPE_CAM,
    DRV_FTM_INFO_TYPE_TCAM,
    DRV_FTM_INFO_TYPE_EDRAM,
    DRV_FTM_INFO_TYPE_LPM_MODEL,        /* ftm lpm model */
    DRV_FTM_INFO_TYPE_NAT_PBR_EN,       /* ftm nat/pbr enable */
    DRV_FTM_INFO_TYPE_LPM_TCAM_INIT_SIZE,
    DRV_FTM_INFO_TYPE_PIPELINE_INFO,
    DRV_FTM_INFO_TYPE_SCL_MODE,
    DRV_FTM_INFO_TYPE_MAX,
};
typedef enum drv_ftm_info_type_e drv_ftm_info_type_t;

enum drv_ftm_lpm_model_type_e
{
    LPM_MODEL_PUBLIC_IPDA,                              /* only public Ipda */
    LPM_MODEL_PRIVATE_IPDA,                             /* only private ipda */
    LPM_MODEL_PUBLIC_IPDA_IPSA_HALF,            /* public ipda+ipsa, half size, performance */
    LPM_MODEL_PUBLIC_IPDA_IPSA_FULL,             /* public ipda+ipsa, full size, no performance */
    LPM_MODEL_PUBLIC_IPDA_PRIVATE_IPDA_HALF, /* public and private ipda, half size, performance */
    LPM_MODEL_PRIVATE_IPDA_IPSA_HALF,           /* private ipda+ipsa, half size, performance */
    LPM_MODEL_PRIVATE_IPDA_IPSA_FULL,           /* private ipda+ipsa, full size, no performance, default Mode */
    LPM_MODEL_PUBLIC_IPDA_IPSA_PRIVATE_IPDA_IPSA_HALF /* public ipda+ipsa, private ipda+ipsa, half size, no performance */
};
typedef enum drv_ftm_lpm_model_type_e drv_ftm_lpm_model_type_t;

#define DRV_FTM_FTM_MAX_EDRAM_MEM_NUM       20
#define DRV_FTM_FTM_MAX_TCAM_TBL_NUM        20

struct drv_ftm_info_detail_s
{
    uint8 info_type;                /*< [GG]drv_ftm_info_type_t */
    uint8 tbl_type;                 /*< [GG]drv_ftm_key_type_t or drv_ftm_tbl_type_t or drv_ftm_cam_type_t*/
    uint8 tbl_entry_num;

    /*for edram*/
    uint8 mem_id[DRV_FTM_FTM_MAX_EDRAM_MEM_NUM];
    uint32 entry_num[DRV_FTM_FTM_MAX_EDRAM_MEM_NUM];
    uint32 offset[DRV_FTM_FTM_MAX_EDRAM_MEM_NUM];

    /*for tcam*/
    uint32 tbl_id[DRV_FTM_FTM_MAX_TCAM_TBL_NUM];
    uint32 key_size[DRV_FTM_FTM_MAX_TCAM_TBL_NUM];
    uint32 max_idx[DRV_FTM_FTM_MAX_TCAM_TBL_NUM];

    /* for cam */
    uint8 max_size;
    uint8 used_size;
    uint8 scl_mode;

    union {
        uint32 lpm_model;
        uint32 nat_pbr_en;
        uint32 lpm_tcam_init_size[2][3];
        struct
        {
            uint32 size0;
            uint32 size1;
        }pipeline_info;
    }l3;


    char str[64];
};
typedef struct drv_ftm_info_detail_s drv_ftm_info_detail_t;




enum drv_ftm_tbl_detail_type_e
{
    DRV_FTM_TBL_TYPE,
};
typedef enum drv_ftm_tbl_detail_type_e drv_ftm_tbl_detail_type_t;

enum drv_ftm_table_type_e
{
    DRV_FTM_TABLE_STATIC,
    DRV_FTM_TABLE_DYNAMIC,
    DRV_FTM_TABLE_TCAM_KEY,
};
typedef enum drv_ftm_table_type_e drv_ftm_table_type_t;

struct drv_ftm_tbl_detail_s
{
    uint8 type;                /*< [GG]drv_ftm_tbl_detail_type_t */
    uint32 tbl_id;

    uint8 tbl_type;
};
typedef struct drv_ftm_tbl_detail_s drv_ftm_tbl_detail_t;


enum drv_ftm_mem_info_type_e
{
    DRV_FTM_MEM_INFO_MEM_TYPE,
    DRV_FTM_MEM_INFO_MEM_TYPE_DETAIL,
    DRV_FTM_MEM_INFO_ENTRY_ENUM,
    DRV_FTM_MEM_INFO_TBL_INDEX,
};
typedef enum drv_ftm_mem_info_type_e drv_ftm_mem_info_type_t;


enum drv_ftm_mem_type_e
{
    DRV_FTM_MEM_DYNAMIC,
    DRV_FTM_MEM_TCAM,

    DRV_FTM_MEM_DYNAMIC_KEY,
    DRV_FTM_MEM_DYNAMIC_AD,
    DRV_FTM_MEM_DYNAMIC_EDIT,

    DRV_FTM_MEM_TCAM_FLOW_KEY,
    DRV_FTM_MEM_TCAM_LPM_KEY,
    DRV_FTM_MEM_TCAM_CID_KEY,
    DRV_FTM_MEM_TCAM_QUEUE_KEY,

    DRV_FTM_MEM_TCAM_FLOW_AD,
    DRV_FTM_MEM_TCAM_LPM_AD,
    DRV_FTM_MEM_MIXED_KEY,
    DRV_FTM_MEM_MIXED_AD,
    DRV_FTM_MEM_MIXED_CAM,

    DRV_FTM_MEM_SHARE_MEM,
};
typedef enum drv_ftm_mem_type_e drv_ftm_mem_type_t;

struct drv_ftm_mem_info_s
{
    uint8 info_type;                /*< drv_ftm_mem_info_type_t */
    uint32 mem_id;
    uint32 mem_offset;

    uint8 mem_type;
    uint8 mem_sub_id;

    uint32 entry_num;

    uint32 table_id;
    uint32 index;
};
typedef struct drv_ftm_mem_info_s drv_ftm_mem_info_t;

#define DRV_USW_FTM_EXTERN_MEM_ID_BASE  8

extern  int32
drv_ftm_get_tbl_detail(uint8 lchip, drv_ftm_tbl_detail_t* p_tbl_info);
extern  int32
drv_ftm_get_mem_info(uint8 lchip, drv_ftm_mem_info_t* p_mem_info);



extern int32
drv_usw_ftm_map_tcam_index(uint8 lchip,
                       uint32 tbl_id,
                       uint32 old_index,
                       uint32* new_index);
extern int32
drv_usw_ftm_lookup_ctl_init(uint8 lchip);

extern int32
drv_usw_ftm_mem_alloc(uint8 lchip, void* profile_info);
extern int32
drv_usw_ftm_mem_free(uint8 lchip);
extern int32
drv_usw_ftm_get_info_detail(uint8 lchip, drv_ftm_info_detail_t* p_ftm_info);
extern int32
drv_usw_ftm_map_compress_idx_real(uint8 lchip, uint32 tbl_id, uint32 old_index,uint32* new_index);

extern int32
drv_usw_ftm_get_host1_poly_type(uint32* poly, uint32* poly_len, uint32* type);


extern int32
drv_usw_ftm_get_sram_type(uint8 lchip, uint32 mem_id, uint8* p_sram_type);

extern int32
drv_usw_ftm_get_hash_poly(uint8 lchip, uint32 mem_id, uint8 sram_type, uint32* drv_poly);

extern int32
drv_usw_ftm_set_hash_poly(uint8 lchip, uint32 mem_id, uint8 sram_type, uint32 drv_poly);

extern int32
drv_usw_ftm_table_id_2_mem_id(uint8 lchip, uint32 tbl_id, uint32 tbl_idx, uint32* p_mem_id, uint32* p_offset);

extern int32
drv_usw_get_memory_size(uint8 lchip, uint32 mem_id, uint32*p_mem_size);

extern int32
drv_usw_ftm_get_cam_by_tbl_id(uint32 tbl_id, uint8* cam_type, uint8* cam_num);
extern int32 drv_usw_ftm_misc_config(uint8 lchip);
extern int32 drv_usw_ftm_adjust_flow_tcam(uint8 lchip, uint8 expand_key, uint8 compress_key);
extern int32 drv_usw_ftm_adjust_mac_table(uint8 lchip, drv_ftm_profile_info_t* p_profile);
extern int32 drv_usw_ftm_reset_tcam_table(uint8 lchip);
#ifdef __cplusplus
}
#endif

#endif

