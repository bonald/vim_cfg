#if !defined (__SAITAP_GROUP_H_)
#define __SAITAP_GROUP_H_

#include <saitypes.h>

typedef enum _sai_tap_group_member_stage_t
{
    /** Ingress Stage */
    SAI_TAP_GROUP_MEMBER_STAGE_INGRESS,

    /** Egress Stage */
    SAI_TAP_GROUP_MEMBER_STAGE_EGRESS,
} sai_tap_group_member_stage_t;

typedef enum _sai_tap_group_attr_t
{

    SAI_TAP_GROUP_ATTR_START,    

    /** READ-ONLY */

    /** List of ingress members in a TAP_GROUP [sai_object_list_t]*/
    SAI_TAP_GROUP_ATTR_INGRESS_MEMBER_LIST = SAI_TAP_GROUP_ATTR_START,
	
	/** List of egress members in a TAP_GROUP [sai_object_list_t]*/
    SAI_TAP_GROUP_ATTR_EGRESS_MEMBER_LIST,

    /** READ-WRITE */
	
	/* TAP_GROUP name string(max-length 32) (MANDATORY_ON_CREATE | CREATE_ONLY)  */
    SAI_TAP_GROUP_ATTR_NAME,

    SAI_TAP_GROUP_ATTR_TRUNCTION_LEN,
	
	/** Custom range base value */
    SAI_TAP_GROUP_ATTR_CUSTOM_RANGE_START = 0x10000000,

    /* --*/
    SAI_TAP_GROUP_ATTR_CUSTOM_RANGE_END

} sai_tap_group_attr_t;

typedef enum _sai_tap_group_member_untag_mode_t
{
    SAI_TAP_GROUP_MEMBER_STAGE_UNTAG_MODE_DISABLE,
    SAI_TAP_GROUP_MEMBER_STAGE_UNTAG_MODE_DOUBLE_VLAN,
    SAI_TAP_GROUP_MEMBER_STAGE_UNTAG_MODE_SVLAN,
    SAI_TAP_GROUP_MEMBER_STAGE_UNTAG_MODE_CVLAN,
} sai_tap_group_member_untag_mode_t;

typedef enum _sai_tap_group_member_attr_t {

    SAI_TAP_GROUP_MEMBER_ATTR_START,

    /** READ_WRITE */
	
	/** TAP_GROUP stage [sai_tap_group_member_stage_t]
     * (MANDATORY_ON_CREATE|CREATE_ONLY) */
    SAI_TAP_GROUP_MEMBER_ATTR_STAGE = SAI_TAP_GROUP_MEMBER_ATTR_START,

    /** The TAP_GROUP oid
	 * (MANDATORY_ON_CREATE|CREATE_ONLY) */
    SAI_TAP_GROUP_MEMBER_ENTRY_ATTR_TAP_GROUP_ID,

    /** The port id here can refer to a generic port object such as SAI port object id,
     * SAI LAG object id and etc. 
	 * (MANDATORY_ON_CREATE|CREATE_ONLY) */
    SAI_TAP_GROUP_MEMBER_ENTRY_ATTR_PORT_ID,

    /** timestamp [bool]
     * (SAI_TAP_GROUP_MEMBER_ATTR_STAGE = SAI_TAP_GROUP_MEMBER_STAGE_EGRESS) */
    SAI_TAP_GROUP_MEMBER_ATTR_TIMESTAMP,
	
	/** truncation [bool]
     * (SAI_TAP_GROUP_MEMBER_ATTR_STAGE = SAI_TAP_GROUP_MEMBER_STAGE_INGRESS) */
    SAI_TAP_GROUP_MEMBER_ATTR_TRUNCATION,
	
	/** mark-source [bool]
     * (SAI_TAP_GROUP_MEMBER_ATTR_STAGE = SAI_TAP_GROUP_MEMBER_STAGE_INGRESS) */
    SAI_TAP_GROUP_MEMBER_ATTR_MARK_SOURCE,
	
	/** mark-source vid [u16]
     * (SAI_VLAN_MEMBER_ATTR_MARK_SOURCE = true) */
    SAI_TAP_GROUP_MEMBER_ATTR_MARK_SOURCE_VID,
	
	/** untag [sai_tap_group_member_untag_mode_t]
     * (SAI_TAP_GROUP_MEMBER_ATTR_STAGE = SAI_TAP_GROUP_MEMBER_STAGE_INGRESS) */
    SAI_TAP_GROUP_MEMBER_ATTR_UNTAG,

    /** ingress action edit-macda [sai_mac_t] */
    SAI_TAP_GROUP_MEMBER_ATTR_ACTION_EDIT_MACDA,

    /** ingress action edit-macsa [sai_mac_t] */
    SAI_TAP_GROUP_MEMBER_ATTR_ACTION_EDIT_MACSA,

    /** ingress action edit-ipda [sai_ip_address_t] */
    SAI_TAP_GROUP_MEMBER_ATTR_ACTION_EDIT_IPDA,

    /** ingress action edit-ipsa [sai_ip_address_t] */
    SAI_TAP_GROUP_MEMBER_ATTR_ACTION_EDIT_IPSA,

    /** ingress action edit-ipsa [uint16_t] */
    SAI_TAP_GROUP_MEMBER_ATTR_ACTION_EDIT_VLAN,

    SAI_TAP_GROUP_MEMBER_ATTR_END,

    /** custom range base value */
    SAI_TAP_GROUP_MEMBER_ATTR_CUSTOM_RANGE_START = 0x10000000,

    /* --*/
    SAI_TAP_GROUP_MEMBER_ATTR_CUSTOM_RANGE_END


} sai_vlan_member_attr_t;



typedef sai_status_t (*sai_create_tap_group_fn)(
	_Out_ sai_object_id_t		*tap_group_id,
    _In_ const sai_attribute_t  *attr,
	_In_ uint32_t 				attr_count
    );

typedef sai_status_t (*sai_remove_tap_group_fn)(
    _In_ sai_object_id_t 		tap_group_id
    );

typedef sai_status_t (*sai_set_tap_group_attribute_fn)(
    _In_ sai_object_id_t 		tap_group_id,
    _In_ const sai_attribute_t  *attr
    );

typedef sai_status_t (*sai_get_tap_group_attribute_fn)(
    _In_ sai_object_id_t 		tap_group_id,
    _In_ uint32_t 				attr_count,
    _Inout_ sai_attribute_t 	*attr_list
    );

typedef sai_status_t (*sai_create_tap_group_member_fn)(
    _Out_ sai_object_id_t		*tap_group_ingress_id,
    _In_ const sai_attribute_t 	*attr_list,
    _In_ uint32_t 				attr_count
    );


typedef sai_status_t (*sai_remove_tap_group_member_fn)(
    _In_ sai_object_id_t 		tap_group_ingress_id
    );


typedef sai_status_t (*sai_set_tap_group_member_attribute_fn)(
    _In_ sai_object_id_t 		vlan_member_id,
    _In_ const sai_attribute_t *attr_list,
    _In_ uint32_t 				attr_count
    );


typedef sai_status_t (*sai_get_tap_group_member_attribute_fn)(
    _In_ sai_object_id_t 		vlan_member_id,
    _In_ const uint32_t 		attr_count,
    _Inout_ sai_attribute_t 	*attr_list
    );

/**
 * @brief TAP_GROUP methods table retrieved with sai_api_query()
 */
typedef struct _sai_tap_group_api_t
{
    sai_create_tap_group_fn                  		create_tap_group;
    sai_remove_tap_group_fn                  		remove_tap_group;
    sai_set_tap_group_attribute_fn           		set_tap_group_attribute;
    sai_get_tap_group_attribute_fn           		get_tap_group_attribute;
	
    sai_create_tap_group_member_fn          		create_tap_group_member;
    sai_remove_tap_group_member_fn          		remove_tap_group_member;
    sai_set_tap_group_member_attribute_fn   		set_tap_group_member_attribute;
    sai_get_tap_group_member_attribute_fn   		get_tap_group_member_attribute;
} sai_tap_group_api_t;

/**
 *\}
 */
#endif // __SAITAP_GROUP_H_
