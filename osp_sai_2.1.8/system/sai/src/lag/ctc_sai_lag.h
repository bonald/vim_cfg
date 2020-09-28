#if !defined (__CTC_SAI_LAG_H_)
#define __CTC_SAI_LAG_H_

#include <sai.h>
#include <ctc_opf.h>
#include "ctc_sai_port.h"

#define CTC_SAI_MAX_LINKAGG_DLB_GROUP_NUM   8
#define CTC_SAI_MAX_LINKAGG_RR_GROUP_NUM    16
#if defined GOLDENGATE
#ifdef TAPPRODUCT
#define CTC_SAI_MAX_LINKAGG_GROUP_NUM       16
#else
#define CTC_SAI_MAX_LINKAGG_GROUP_NUM       55
#endif
#elif defined USW
#ifdef TAPPRODUCT
#define CTC_SAI_MAX_LINKAGG_GROUP_NUM       16
#else
#define CTC_SAI_MAX_LINKAGG_GROUP_NUM       55
#endif
#else
#define CTC_SAI_MAX_LINKAGG_GROUP_NUM       31
#endif

enum{
    CTC_SAI_LAG_NF_NEW_LAG = 0,
    CTC_SAI_LAG_NF_DEL_LAG,
    CTC_SAI_LAG_NF_NEW_LAG_MEMBER,
    CTC_SAI_LAG_NF_DEL_LAG_MEMBER,
};

typedef struct ctc_sai_lag_notifier
{
    uint32_t                            op;
    sai_object_id_t                     lag_id;
    sai_object_id_t                     lag_member_port_id;      /* member port oid */
}ctc_sai_lag_notifier_t;

sai_status_t
ctc_sai_lag_get_ports(
    _In_    sai_object_id_t      lag_id,
    _Inout_ sai_attribute_t*     attr);

sai_status_t
ctc_sai_lag_init(void);

int32_t
ctc_sai_lag_alloc_timestamp (sai_object_id_t port_id, uint32_t *ptimestamp_nhid);

int32_t
ctc_sai_lag_free_timestamp (sai_object_id_t port_id);

#endif

