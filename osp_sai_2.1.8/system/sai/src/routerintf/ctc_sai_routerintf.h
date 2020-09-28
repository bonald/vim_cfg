#if !defined (__SAI_ROUTERINTF_H_)
#define __SAI_ROUTERINTF_H_

#include <sairouterintf.h>
#include <ctc_vector.h>

stbl_rif_t*
ctc_routerintf_get_by_vlan(
	_In_  const sai_vlan_id_t);

stbl_rif_t*
ctc_routerintf_get_by_port_oid(
	_In_  const sai_object_id_t port_oid);

void
ctc_routerintf_release(
    _In_  stbl_rif_t*);

stbl_rif_t*
ctc_routerintf_get_by_oid(
	_In_  const sai_object_id_t rif_id);

stbl_rif_t*
ctc_routerintf_get_by_oid_no_ref(
	_In_  const sai_object_id_t rif_id);

sai_status_t
ctc_routerintf_update_src_mac(
	_In_  const stbl_rif_t *,
	_In_  const uint8_t *);

sai_status_t
ctc_routerintf_set_v4_admin(
	_In_  stbl_rif_t *,
	_In_  const sai_attribute_t *);

sai_status_t
ctc_routerintf_set_v6_admin(
	_In_  stbl_rif_t *,
	_In_  const sai_attribute_t *);

sai_status_t
ctc_sai_set_router_interface_attribute(
    _In_ sai_object_id_t ,
    _In_ const sai_attribute_t *);

sai_status_t
ctc_sai_routerintf_init(void);

sai_status_t
ctc_routerintf_alloc(
    _Out_ stbl_rif_t**,
    _In_ uint32_t);

sai_status_t
ctc_routerintf_add_entry(
	_In_ stbl_rif_t*);

sai_status_t
ctc_routerintf_remove_entry(
	_In_ stbl_rif_t*);

int32
ctc_routerintf_alloc_l3if_id(uint32_t *l3if_id);

int32
ctc_routerintf_release_l3if_id(uint32_t l3if_id);

#endif

