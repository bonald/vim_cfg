#if !defined (__CTC_SAI_COMMON_ATTR_H_)
#define __CTC_SAI_COMMON_ATTR_H_

#include "sai.h"
#include "saitypes.h"
#include "saistatus.h"
#include "sdb.h"
#include "gen/stbl_sai_glb_define.h"
#include "gen/stbl_sai_glb.h"
#include "gen/stbl_port_glb_define.h"
#include "gen/stbl_port_glb.h"
#include "gen/stbl_port_define.h"
#include "gen/stbl_port.h"
#include "gen/stbl_lag_memport_define.h"
#include "gen/stbl_lag_memport.h"
#include "gen/stbl_hostif_define.h"
#include "gen/stbl_hostif.h"
#include "gen/stbl_vlan_define.h"
#include "gen/stbl_vlan.h"
#include "gen/stbl_vlan_class_define.h"
#include "gen/stbl_vlan_class.h"
#include "gen/stbl_stp_define.h"
#include "gen/stbl_stp.h"
#include "gen/stbl_vr_define.h"
#include "gen/stbl_vr.h"
#include "gen/stbl_rif_define.h"
#include "gen/stbl_rif.h"
#include "gen/stbl_neighbor_define.h"
#include "gen/stbl_neighbor.h"
#include "gen/stbl_nexthop_define.h"
#include "gen/stbl_nexthop.h"
#include "gen/stbl_nexthop_group_define.h"
#include "gen/stbl_nexthop_group.h"
#include "gen/stbl_vrrp_vip_define.h"
#include "gen/stbl_vrrp_vip.h"
#include "gen/stbl_route_define.h"
#include "gen/stbl_route.h"
#include "gen/stbl_l2mc_define.h"
#include "gen/stbl_l2mc.h"
#include "gen/stbl_ipmc_define.h"
#include "gen/stbl_ipmc.h"
#include "gen/stbl_copp_group_define.h"
#include "gen/stbl_copp_group.h"
#include "gen/stbl_copp_trap_define.h"
#include "gen/stbl_copp_trap.h"
#include "gen/stbl_copp_reason_define.h"
#include "gen/stbl_copp_reason.h"
#include "gen/stbl_copp_entry_define.h"
#include "gen/stbl_copp_entry.h"
#include "gen/stbl_acl_glb_define.h"
#include "gen/stbl_acl_glb.h"
#include "gen/stbl_acl_port_define.h"
#include "gen/stbl_acl_port.h"
#include "gen/stbl_acl_tbl_define.h"
#include "gen/stbl_acl_tbl.h"
#include "gen/stbl_acl_entry_define.h"
#include "gen/stbl_acl_entry.h"
#include "gen/stbl_acl_tunnel_define.h"
#include "gen/stbl_acl_tunnel.h"
#include "gen/stbl_acl_udf_entry_define.h"
#include "gen/stbl_acl_udf_entry.h"
#include "gen/stbl_nat_forward_define.h"
#include "gen/stbl_nat_forward.h"


typedef enum
{
    SAI_ATTR_FALG_READ      = 0x0001,
    SAI_ATTR_FALG_WRITE     = 0x0002,
    SAI_ATTR_FALG_CREATE    = 0x0004,           /* MANDATORY_ON_CREATE */
    SAI_ATTR_FLAG_DEFAULT   = 0x0008,
    SAI_ATTR_FLAG_UNKONW    = 0x0010,
//    SAL_ATTR_FALG_CREATE_AND_SET              /* SAL_ATTR_FALG_CREATE | SAL_ATTR_FALG_WRITE */
}sai_attr_entry_flag_t;

typedef enum
{
    SAI_ATTR_CHECK_READ     = SAI_ATTR_FALG_READ,
    SAI_ATTR_CHECK_WRITE    = SAI_ATTR_FALG_WRITE,
    SAI_ATTR_CHECK_CREATE   = SAI_ATTR_FALG_CREATE,
}sai_attr_check_t;

#define SAI_ATTR_ID_END 0xFFFFFFFF

typedef struct ctc_sai_attr_entry_list_s
{
    sai_attribute_value_t   value;
    int32_t                 attr_index;     /* -1 -> default entry */
    bool                    valid;
}ctc_sai_attr_entry_list_t;


typedef struct ctc_sai_attr_entry_info_s
{
    sai_attr_id_t           id;
    uint32_t                type;

    sai_attribute_value_t   default_value;

    struct {
        sai_status_t (*func_fn)(ctc_sai_attr_entry_list_t*,void*);
        void*   func_parameter;
    }check_fn;
}ctc_sai_attr_entry_info_t;


#define ctc_sai_attr_foreach_entry_info(pentry,pentry_list)  \
    for(pentry = pentry_list;                           \
        pentry && pentry->id != SAI_ATTR_ID_END;        \
        pentry++)

const sai_attribute_t const *
ctc_sai_attr_get_attr_by_id(    const sai_attr_id_t,
                                    const sai_attribute_t*,
                                    uint32_t,
                                    uint32_t*);

sai_status_t
ctc_sai_attr_alloc_attr_entry_list(const ctc_sai_attr_entry_info_t const * ,
                                 const sai_attribute_t*,
                                 uint32_t attr_count,
                                 _Out_ ctc_sai_attr_entry_list_t **);

#define ctc_sai_attr_free_attr_entry_list(pattr_entry_list) \
    if(pattr_entry_list)    \
        XFREE(MEM_FEA_TEMP_TODO, pattr_entry_list)

sai_status_t
ctc_sai_attr_check_attr_entry_list(const ctc_sai_attr_entry_info_t const *,
                                            ctc_sai_attr_entry_list_t*);

sai_status_t
ctc_sai_attr_check_read_attr(const ctc_sai_attr_entry_info_t const *,
                                 const sai_attribute_t*);

sai_status_t
ctc_sai_attr_check_write_attr(const ctc_sai_attr_entry_info_t const *,
                                 const sai_attribute_t*);


typedef struct ctc_sai_check_u32_range_s
{
    uint32_t    min;
    uint32_t    max;
}ctc_sai_check_u32_range_t,ctc_sai_check_object_type_range_t;

/* extern attr check parameter */
extern ctc_sai_check_u32_range_t g_packet_action_range;

sai_status_t
ctc_sai_check_i32_range_fn(ctc_sai_attr_entry_list_t *pattr, void *data);

sai_status_t
ctc_sai_check_u32_range_fn(ctc_sai_attr_entry_list_t *pattr, void *data);

sai_status_t
ctc_sai_check_object_type_range_fn(ctc_sai_attr_entry_list_t *pattr, void *data);


#endif

