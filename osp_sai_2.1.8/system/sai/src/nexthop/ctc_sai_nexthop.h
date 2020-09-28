#if !defined (__SAI_NEXTHOP_H_)
#define __SAI_NEXTHOP_H_

#include <sainexthop.h>
#include <ctc_vector.h>
#include <ctc_opf.h>

void
ctc_nexthop_release(
	_In_  stbl_nexthop_t *p_sdb_nh);

sai_status_t
ctc_sai_nexthop_init();

stbl_nexthop_t*
ctc_nexthop_get_by_oid(
	_In_  const sai_object_id_t nh_id);

sai_status_t
ctc_sai_nexthop_alloc_offset(uint32_t *p_index);

sai_status_t
ctc_sai_nexthop_free_offset(uint32_t index);

int32
stbl_nexthop_add_array(stbl_nexthop_t *p_nexthop);
int32
stbl_nexthop_del_array(uint32 nh_id);

#endif

