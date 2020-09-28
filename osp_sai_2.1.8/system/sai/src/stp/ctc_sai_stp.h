#if !defined (__CTC_SAI_STP_H_)
#define __CTC_SAI_STP_H_

#include <sai.h>
#include <ctc_opf.h>
#include <ctc_vector.h>

sai_status_t
ctc_sai_create_stp_inst(
    _Out_ sai_object_id_t *,
    _In_  uint32_t,
    _In_  const sai_attribute_t *);

stbl_stp_t*
ctc_sai_stp_get_by_oid(
    _In_  const sai_object_id_t);

sai_status_t
ctc_sai_stp_add_vlan(
    _In_  stbl_stp_t*,
    _In_  const sai_vlan_id_t);

sai_status_t
ctc_sai_stp_remove_vlan(
    _In_  stbl_stp_t*,
    _In_  const sai_vlan_id_t);

sai_status_t
ctc_sai_stp_init(void);


#endif

