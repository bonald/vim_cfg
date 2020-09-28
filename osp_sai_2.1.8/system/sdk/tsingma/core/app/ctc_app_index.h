/**
 @file ctc_app_index.h

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2009-11-26

 @version v2.0

   This file contains all L2 related data structure, enum, macro and proto.
*/

#ifndef _CTC_APP_INDEX_H
#define _CTC_APP_INDEX_H
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/

enum ctc_app_index_type
{
    CTC_APP_INDEX_TYPE_MPLS_TUNEL_ID,
    CTC_APP_INDEX_TYPE_NHID,
    CTC_APP_INDEX_TYPE_L3IF_ID,
    CTC_APP_INDEX_TYPE_MCAST_GROUP_ID,
    CTC_APP_INDEX_TYPE_GLOBAL_DSNH_OFFSET,
    CTC_APP_INDEX_TYPE_POLICER_ID,
    CTC_APP_INDEX_TYPE_STATSID_ID,
    CTC_APP_INDEX_TYPE_LOGIC_PORT,
    CTC_APP_INDEX_TYPE_APS_GROUP_ID,
    CTC_APP_INDEX_TYPE_ARP_ID,
    CTC_APP_INDEX_TYPE_ACL_ENTRY_ID,
    CTC_APP_INDEX_TYPE_ACL_GROUP_ID,
    CTC_APP_INDEX_TYPE_SCL_ENTRY_ID,
    CTC_APP_INDEX_TYPE_SCL_GROUP_ID,
    CTC_APP_INDEX_TYPE_MAX
};
typedef enum ctc_app_index_type ctc_app_index_type_t;


struct ctc_app_index_s
{
    uint8   index_type; /**<  refer to ctc_app_index_type_t*/
    uint8   gchip ;    /**< */
    uint8   rsv;  /**< */

    /* used for CTC_APP_INDEX_TYPE_GLOBAL_DSNH_OFFSET*/
    uint8  nh_type;             /* refer to ctc_nh_type_t  */
    void *p_nh_param ;  /* */

    uint32  index;
    uint32  entry_num;
};
typedef struct ctc_app_index_s ctc_app_index_t;


enum ctc_app_chipset_type_e
{
 CTC_APP_CHIPSET_CTC5160,
 CTC_APP_CHIPSET_CTC8096,
 CTC_APP_CHIPSET_CTC7148,
 CTC_APP_CHIPSET_CTC7132
};

extern int32
ctc_app_index_init(uint8 gchip);

extern int32
ctc_app_index_alloc(ctc_app_index_t* app_index);

extern int32
ctc_app_index_free(ctc_app_index_t* app_index);

extern int32
ctc_app_index_alloc_dsnh_offset(uint8 gchip ,uint8 nh_type ,void* nh_param,
                                uint32 *dsnh_offset,uint32 *entry_num);
extern int32
ctc_app_index_set_rchip_dsnh_offset_range(uint8 gchip,uint32 min_index,uint32 max_index);

extern int32
ctc_app_index_set_chipset_type(uint8 gchip,uint8 type);

extern int32
ctc_app_index_get_lchip_id(uint8 gchip, uint8* lchip);

extern int32
ctc_app_index_alloc_from_position(ctc_app_index_t* app_index);

#ifdef __cplusplus
}
#endif

#endif  /*_CTC_APP_INDEX_H*/

