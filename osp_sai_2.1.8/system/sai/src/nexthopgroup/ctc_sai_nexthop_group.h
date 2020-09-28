#if !defined (__SAI_NEXTHOP_GROUP_H_)
#define __SAI_NEXTHOP_GROUP_H_

#include <saifdb.h>
#include <ctc_vector.h>
#include <ctc_opf.h>
#include <ctc_mix.h>
#include <ctc_sai_common.h>

#define CTC_SAI_NEXTHOP_GROUP_MAX_ENTRY     16384//1024
#define CTC_SAI_NH_GROUP_OFFSET             12000//8K + 3000
//#define CTC_SAI_DEFAULT_MAX_ECMP_NUM        64

stbl_nexthop_group_t*
ctc_nexthop_group_get_by_oid(
    _In_  const sai_object_id_t);

sai_status_t
ctc_sai_nexthop_group_init();


#endif

