#if !defined (__SAI_UDF_H_)
#define __SAI_UDF_H_

#include <saiudf.h>
#include <ctc_sai_common.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_vector.h>
#include <ctc_opf.h>
#include <ctc_parser.h>



#define SAI_UDF_L2_ETHER_TYPE 0x0800

#define CTC_SAI_UDF_GROUP_MAX                   4
#define CTC_SAI_UDF_GROUP_UDF_OID_MAX           8


typedef struct ctc_sai_udf_match_s
{
    uint16_t        udf_match_l2_type;
    uint8_t         udf_match_l3_type;
    uint16_t        udf_match_gre_type;
    uint8_t         udf_match_priority;
    
}ctc_sai_udf_match_t;

typedef struct ctc_sai_udf_s
{
    sai_object_id_t udf_match_id;
    sai_object_id_t udf_group_id;
    sai_udf_base_t udf_base_select;
    uint16_t udf_offset;
    sai_u8_list_t udf_hash_mask_list;
    
}ctc_sai_udf_t;

typedef struct ctc_sai_udf_group_s
{
    sai_udf_group_type_t udf_group_type;
    sai_object_id_t udf_oid[CTC_SAI_UDF_GROUP_UDF_OID_MAX];
    uint16_t          udf_length;
    
}ctc_sai_udf_group_t;


enum{
    CTC_SAI_UDF_INFO_TYPE_MATCH = 0,
    CTC_SAI_UDF_INFO_TYPE_UDF,
    CTC_SAI_UDF_INFO_TYPE_GROUP,
    CTC_SAI_UDF_INFO_TYPE_MAX,
};


typedef struct ctc_sai_udf_info_s
{
	uint32_t            info_type;		/* 0->udf_match,1->udf_id,2->udf_group */
    sai_object_id_t     oid;
	
	union
	{
        ctc_sai_udf_match_t         udf_match;
        ctc_sai_udf_t               udf;
        ctc_sai_udf_group_t         udf_group;
	}u;

    uint32              max_count;
    uint32              total_count;
    uint32              udf_match_cnt; 
    uint32              udf_cnt;
    uint32              udf_group_cnt;
    
    ctc_vector_t*       pudf_vector; 
	
}ctc_sai_udf_info_t;



sai_status_t 
ctc_sai_udf_init();

ctc_sai_udf_group_t*
ctc_sai_get_udf_group(sai_object_id_t udf_group_oid);

#endif

