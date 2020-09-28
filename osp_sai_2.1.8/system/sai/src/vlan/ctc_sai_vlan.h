#if !defined (__SAI_VLAN_H_)
#define __SAI_VLAN_H_

#include <saivlan.h>

enum{
    CTC_SAI_LAG_NF_NEW_VLAN = 0,
    CTC_SAI_LAG_NF_DEL_VLAN,
    CTC_SAI_LAG_NF_NEW_VLAN_MEMBER,
    CTC_SAI_LAG_NF_DEL_VLAN_MEMBER,
};

typedef struct ctc_sai_vlan_notifier
{
    uint32_t                            op;
    sai_vlan_id_t                     	vlan_id;
    sai_object_id_t                     vlan_member_port_id;      /* member port oid */
}ctc_sai_vlan_notifier_t;


sai_status_t ctc_sai_vlan_init();

sai_status_t
ctc_sai_vlan_set_proto_excep(
    _In_ sai_vlan_id_t          vlan_id,
    _In_ bool                       enable);

#endif

