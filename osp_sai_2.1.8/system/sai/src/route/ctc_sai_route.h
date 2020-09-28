#if !defined (__CTC_ROUTE_H_)
#define __CTC_ROUTE_H_

#include <ctc_sai_router.h>

#define CTC_SAI_ROUTE_MAX_ENTRY  32768

sai_status_t ctc_sai_route_init();

sai_status_t
_route_check_nexthop_exist(sai_object_id_t nexthop_id);

#endif

