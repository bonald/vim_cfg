#if !defined (__CTC_SAI_TAP_GROUP_H_)
#define __CTC_SAI_TAP_GROUP_H_

#include <sai.h>
#include <ctc_opf.h>
#include <ctclib_opf.h>
#include <ctc_vector.h>
#include <ctc_linklist.h>

enum __ctc_sai_tap_group_edit_packet_flag
{
    CTC_SAI_TAP_GROUP_EDIT_NONE = 0,
    CTC_SAI_TAP_GROUP_EDIT_DMAC,
    CTC_SAI_TAP_GROUP_EDIT_SMAC, 
    CTC_SAI_TAP_GROUP_EDIT_SIP,
    CTC_SAI_TAP_GROUP_EDIT_DIP,
    CTC_SAI_TAP_GROUP_EDIT_VLAN,
    CTC_SAI_TAP_GROUP_EDIT_MAX,
};

enum __ctc_sai_tap_group_edit_type
{
    CTC_SAI_TAP_GROUP_EDIT_TYPE_NORMAL    = 16,
    CTC_SAI_TAP_GROUP_EDIT_TYPE_TRUNCTION,
    CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM,
    CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_GRE_L2_HDR,
    CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_VLAN_REP_OR_ADD,
    CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_VLAN_ADD,
    CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_L2GRE_HDR,
    CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_L3GRE_HDR,
    /*added by guoxd for vxlan*/
    CTC_SAI_TAP_GROUP_EDIT_TYPE_CUSTOM_ADD_VXLAN_HDR,
    /*ended by guoxd*/
    CTC_SAI_TAP_GROUP_EDIT_TYPE_MAX,
};

typedef struct __ctc_sai_tap_group_edit_group_key{
    uint32_t            edit_flag;
    sai_ip_address_t    ip_sip;
    sai_ip_address_t    ip_dip;
    uint16_t            vlan;
    sai_mac_t           mac_dest;
    sai_mac_t           mac_src;

    /*added by yejl for bug 53475, 2019-09-10*/
    sai_ip_address_t    l2gre_ip_sip;
    sai_ip_address_t    l2gre_ip_dip;
    sai_mac_t           l2gre_mac_da;
    uint32_t            l2gre_key_num;
    uint32_t            l2gre_key_len;

    sai_ip_address_t    l3gre_ip_sip;
    sai_ip_address_t    l3gre_ip_dip;
    sai_mac_t           l3gre_mac_da;
    /*ended by yejl*/

    /*added by guoxd for vxlan*/
    sai_mac_t              vxlan_mac_da;
    sai_ip_address_t    vxlan_ip_sa;
    sai_ip_address_t    vxlan_ip_da;
    uint32_t                vxlan_port_src;
    uint32_t                vxlan_port_dst;
    uint32_t                vxlan_vni_set;
    /*ended by guoxd*/
    
}ctc_sai_tap_group_edit_group_key;

typedef struct __ctc_sai_tap_group_edit_group{
    ctc_slistnode_t                     list_node;
    ctc_sai_tap_group_edit_group_key    key;
    
    uint32_t			                nexthop_id;
	uint32_t			                nexthop_group_id;

    uint32_t			                session_id;
	uint32_t			                illop_nexthop_id;

    int32_t                             count;

    uint32_t			                trunction_count;                /* if trunction_count != 0; update mirror & iloop res */
    uint32_t			                trunction_len;                  /* [D2] The length of packet after truncate */
    ctc_hash_t                          *port_res_hash;
}ctc_sai_tap_group_edit_group;

typedef struct __ctc_sai_tap_group_egress_misc_key{
    ctc_sai_tap_group_edit_group_key    edit_key;
    sai_object_id_t                     poid;
}ctc_sai_tap_group_egress_misc_key;

typedef struct __ctc_sai_tap_group_egress_misc{
    ctc_sai_tap_group_egress_misc_key    key;
    
    uint32_t			                nexthop_id;
    int32_t                             count;
}ctc_sai_tap_group_egress_misc;

typedef struct __ctc_sai_tap_group_port_res{
    sai_object_id_t                     poid;
    ctc_sai_tap_group_egress_misc       *egress_misc;
}ctc_sai_tap_group_port_res;

typedef struct ctc_sai_tap_group_s
{
	char 				name[32];
    uint32_t			trunction_len;
    ctc_slist_t         *edit_header;
}ctc_sai_tap_group_t;

enum{
    CTC_SAI_TAP_GROUP_MEMBER_TYPE_INGRESS = 0,
    CTC_SAI_TAP_GROUP_MEMBER_TYPE_EGRESS,
    CTC_SAI_TAP_GROUP_MEMBER_TYPE_MAX,
};

typedef struct ctc_sai_tap_group_member_s
{
    uint32_t            tap_member_stage;		/* 0->ingress,1->egress */
    sai_object_id_t     tap_group_oid;
    sai_object_id_t     port_oid;
	
	union
	{
		struct {
            ctc_sai_tap_group_edit_group_key    edit_key;               /* ctc_sai_tap_group_edit_group_key */
            ctc_sai_tap_group_edit_group        *pcustom_edit_group;    /* if custom key, then save edit_group to free */
			uint32_t	mark_source;			/* 0->disable */
			uint32_t	untag;					/* 0->disable,1->enable,2-svlan,3-cvlan */
            uint32_t	trunction;				/* 0->disable,1->enable */
		}ingress;
		
		struct {
			uint32_t	timestamp;				/* 0->diable,1->enable */
            uint32_t    timestamp_nhid;         /* if timestamp enable, then set timestamp_nhid, use to free mcast_member */
		}egress;
	}u;
    
}ctc_sai_tap_group_member_t;

enum{
    CTC_SAI_TAP_INFO_TYPE_GROUP = 0,
    CTC_SAI_TAP_INFO_TYPE_MEMBER,
    CTC_SAI_TAP_INFO_TYPE_MAX,
};

typedef struct ctc_sai_tap_info_s
{
	uint32_t            tap_info_type;		/* 0->tap_group,1->tap_group_member */
    sai_object_id_t     tap_oid;
	
	union
	{
		ctc_sai_tap_group_t			tap_group;
		ctc_sai_tap_group_member_t	tap_group_member;
	}u;
	
}ctc_sai_tap_info_t;

typedef struct ctc_sai_tap_group_master_s
{
    uint32_t        max_count;
    uint32          tap_cnt;                
    uint32          tap_group_cnt;
    uint32          tap_member_ing_cnt;
    uint32          tap_member_egr_cnt;
    
    ctc_vector_t    *ptap_group_vector; /* save all ctc_sai_tap_info_t struct info */
    
    ctc_hash_t*     egress_misc_hash;
    /*added by guoxd for vxlan*/
    ctclib_opf_t       *tap_group_vxlan_opf;
    /*ended by guoxd*/
}ctc_sai_tap_group_master_t;

sai_status_t
ctc_sai_tap_group_init(void);

extern ctc_sai_tap_info_t*
ctc_sai_tap_get_by_oid(sai_object_id_t  tap_oid);

sai_status_t
ctc_sai_tap_group_get_session_id(sai_object_id_t tap_group_id, uint32_t  *psession_id);

sai_status_t
ctc_sai_tap_group_put_session_id(sai_object_id_t tap_group_id);

sai_status_t
ctc_sai_tap_group_get_nexthop_id(sai_object_id_t tap_group_id, uint32_t  *pnexthop_id);

ctc_sai_tap_group_edit_group *
ctc_sai_tap_group_get_edit_group(ctc_sai_tap_group_edit_group_key    key, ctc_sai_tap_info_t *ptap_info_entry);

void
ctc_sai_tap_group_release_edit_group(ctc_sai_tap_group_edit_group *edit_group, ctc_sai_tap_info_t *ptap_info_entry);

sai_status_t
ctc_sai_tap_group_set_trunction_len(_In_  const sai_attribute_t *attr);

#endif

