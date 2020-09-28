#if !defined (__SAI_ROUTER_H_)
#define __SAI_ROUTER_H_

#include <sairouter.h>
#include <ctc_vector.h>
#include <ctc_opf.h>
#include <ctc_mix.h>

sai_status_t
ctc_vr_add_router_intf(
	_In_  sai_object_id_t,
    _In_ stbl_rif_t*);

sai_status_t
ctc_vr_remove_router_intf(
	_In_  sai_object_id_t,
    _In_ stbl_rif_t*);

void
ctc_vr_release(
    _In_ stbl_vr_t*);

stbl_vr_t*
ctc_vr_get_by_oid(
    _In_ const sai_object_id_t);

sai_status_t
ctc_sai_vr_init();

sai_status_t
ctc_vr_alloc(
	_Out_ stbl_vr_t**);

sai_status_t
ctc_sai_remove_virtual_router(
    _In_ sai_object_id_t vr_id);


#endif

