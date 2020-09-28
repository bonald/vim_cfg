/**
 @file ctc_usw_vlan.c

 @date 2009-10-17

 @version v2.0

 The file contains all vlan APIs
*/

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_const.h"

#include "ctc_usw_vlan.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_common.h"
#include "sys_usw_vlan.h"
#include "sys_usw_vlan_classification.h"
#include "sys_usw_vlan_mapping.h"
#include "sys_usw_nexthop_api.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/
/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/

/**
 @brief init the vlan module
*/
int32
ctc_usw_vlan_init(uint8 lchip, ctc_vlan_global_cfg_t* vlan_global_cfg)
{
    uint8 lchip_start               = 0;
    uint8 lchip_end                 = 0;
    ctc_vlan_global_cfg_t vlan_cfg;

    LCHIP_CHECK(lchip);

    if (NULL == vlan_global_cfg)
    {
        /*set default value*/
        sal_memset(&vlan_cfg, 0, sizeof(ctc_vlan_global_cfg_t));
        vlan_cfg.vlanptr_mode = CTC_VLANPTR_MODE_USER_DEFINE1;
        vlan_global_cfg = &vlan_cfg;
    }


    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_init(lchip, vlan_global_cfg));
        CTC_ERROR_RETURN(sys_usw_vlan_mapping_init(lchip));
        CTC_ERROR_RETURN(sys_usw_vlan_class_init(lchip, vlan_global_cfg));
    }
    return CTC_E_NONE;
}

int32
ctc_usw_vlan_deinit(uint8 lchip)
{
    uint8 lchip_start               = 0;
    uint8 lchip_end                 = 0;

    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_deinit(lchip));
        CTC_ERROR_RETURN(sys_usw_vlan_class_deinit(lchip));
        CTC_ERROR_RETURN(sys_usw_vlan_mapping_deinit(lchip));
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to create a vlan
*/
int32
ctc_usw_vlan_create_vlan(uint8 lchip, uint16 vlan_id)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,ret,
                            sys_usw_vlan_create_vlan(lchip, vlan_id));
    }

    /*roll back if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_vlan_destory_vlan(lchip,vlan_id);
    }

    return ret;
}

/**
 @brief The function is to create a vlan with uservlanptr
*/
int32
ctc_usw_vlan_create_uservlan(uint8 lchip, ctc_vlan_uservlan_t* user_vlan)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 vlan_id                  = 0;
    int32 ret                      = 0;
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    CTC_PTR_VALID_CHECK(user_vlan);
    vlan_id = user_vlan->vlan_id;
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,ret,
                                 sys_usw_vlan_create_uservlan(lchip, user_vlan));
    }

    /*roll back if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_vlan_destory_vlan(lchip,vlan_id);
    }
    return ret;
}

/**
 @brief The function is to remove the vlan
*/
int32
ctc_usw_vlan_destroy_vlan(uint8 lchip, uint16 vlan_id)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,ret,
                                        sys_usw_vlan_destory_vlan(lchip, vlan_id));
    }

    return ret;
}

/**
 @brief The function is to add member ports to a vlan
*/
int32
ctc_usw_vlan_add_ports(uint8 lchip, uint16 vlan_id, ctc_port_bitmap_t port_bitmap)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_vlan_add_ports(lchip, vlan_id, port_bitmap));

    return CTC_E_NONE;
}

/**
 @brief The function is to add member port to a vlan
*/
int32
ctc_usw_vlan_add_port(uint8 lchip, uint16 vlan_id, uint32 gport)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_vlan_add_port(lchip, vlan_id, gport));

    return CTC_E_NONE;
}

/**
 @brief The function is to show vlan's member port
*/
int32
ctc_usw_vlan_get_ports(uint8 lchip, uint16 vlan_id, uint8 gchip, ctc_port_bitmap_t port_bitmap)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    SYS_MAP_GCHIP_TO_LCHIP(gchip, lchip);
    CTC_ERROR_RETURN(sys_usw_vlan_get_ports(lchip, vlan_id, gchip, port_bitmap));

    return CTC_E_NONE;
}

/**
 @brief The function is to remove member ports to a vlan
*/
int32
ctc_usw_vlan_remove_ports(uint8 lchip, uint16 vlan_id, ctc_port_bitmap_t port_bitmap)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_vlan_remove_ports(lchip, vlan_id, port_bitmap));

    return CTC_E_NONE;
}

/**
 @brief The function is to remove member port to a vlan
*/
int32
ctc_usw_vlan_remove_port(uint8 lchip, uint16 vlan_id, uint32 gport)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_vlan_remove_port(lchip, vlan_id, gport));

    return CTC_E_NONE;
}

int32
ctc_usw_vlan_set_tagged_port(uint8 lchip, uint16 vlan_id, uint32 gport, bool tagged)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_vlan_set_tagged_port(lchip, vlan_id, gport, tagged));

    return CTC_E_NONE;
}

int32
ctc_usw_vlan_set_tagged_ports(uint8 lchip, uint16 vlan_id, ctc_port_bitmap_t port_bitmap)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_vlan_set_tagged_ports(lchip, vlan_id, port_bitmap));

    return CTC_E_NONE;
}

int32
ctc_usw_vlan_get_tagged_ports(uint8 lchip, uint16 vlan_id, uint8 gchip, ctc_port_bitmap_t port_bitmap)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    SYS_MAP_GCHIP_TO_LCHIP(gchip, lchip);
    CTC_ERROR_RETURN(sys_usw_vlan_get_tagged_ports(lchip, vlan_id, gchip, port_bitmap));

    return CTC_E_NONE;
}

/**
 @brief The function is to set receive enable on vlan
*/
int32
ctc_usw_vlan_set_receive_en(uint8 lchip, uint16 vlan_id, bool enable)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_set_property(lchip, vlan_id, CTC_VLAN_PROP_RECEIVE_EN, enable));
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to get receive on vlan
*/
int32
ctc_usw_vlan_get_receive_en(uint8 lchip, uint16 vlan_id, bool* enable)
{
    uint32 value                   = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);
    CTC_ERROR_RETURN(sys_usw_vlan_get_property(lchip, vlan_id, CTC_VLAN_PROP_RECEIVE_EN, &value));
    *enable                        = value ? TRUE : FALSE;
    return CTC_E_NONE;
}

/**
 @brief The function is to set tranmit enable on vlan
*/
int32
ctc_usw_vlan_set_transmit_en(uint8 lchip, uint16 vlan_id, bool enable)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_set_property(lchip, vlan_id, CTC_VLAN_PROP_TRANSMIT_EN, enable));
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to get tranmit on vlan
*/
int32
ctc_usw_vlan_get_transmit_en(uint8 lchip, uint16 vlan_id, bool* enable)
{
    uint32 value                   = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);
    CTC_ERROR_RETURN(sys_usw_vlan_get_property(lchip, vlan_id, CTC_VLAN_PROP_TRANSMIT_EN, &value));
    *enable                        = value ? TRUE : FALSE;
    return CTC_E_NONE;
}

/**
 @brief The function is to set bridge enable on vlan
*/
int32
ctc_usw_vlan_set_bridge_en(uint8 lchip, uint16 vlan_id, bool enable)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_set_property(lchip, vlan_id, CTC_VLAN_PROP_BRIDGE_EN, enable));
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to get bridge on vlan
*/
int32
ctc_usw_vlan_get_bridge_en(uint8 lchip, uint16 vlan_id, bool* enable)
{
    uint32 value                   = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);
    CTC_ERROR_RETURN(sys_usw_vlan_get_property(lchip, vlan_id, CTC_VLAN_PROP_BRIDGE_EN, &value));
    *enable                        = value ? TRUE : FALSE;
    return CTC_E_NONE;
}

/**
 @brief The fucntion is to set fid of vlan
*/
int32
ctc_usw_vlan_set_fid(uint8 lchip, uint16 vlan_id, uint16 fid)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_set_property(lchip, vlan_id, CTC_VLAN_PROP_FID, fid));
    }

    return CTC_E_NONE;
}

/**
 @brief The fucntion is to get vrfid of vlan
*/
int32
ctc_usw_vlan_get_fid(uint8 lchip, uint16 vlan_id, uint16* fid)
{
    uint32 value_32                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(fid);
    CTC_ERROR_RETURN(sys_usw_vlan_get_property(lchip, vlan_id, CTC_VLAN_PROP_FID, &value_32));
    *fid                           = value_32 & 0xffff;
    return CTC_E_NONE;

}

/**
 @brief The function is set mac learning enable/disable on the vlan
*/
int32
ctc_usw_vlan_set_learning_en(uint8 lchip, uint16 vlan_id, bool enable)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_set_property(lchip, vlan_id, CTC_VLAN_PROP_LEARNING_EN, enable));
    }

    return CTC_E_NONE;
}

/**
 @brief The function is get mac learning enable/disable on the vlan
*/
int32
ctc_usw_vlan_get_learning_en(uint8 lchip, uint16 vlan_id, bool* enable)
{
    uint32 value                   = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);
    CTC_ERROR_RETURN(sys_usw_vlan_get_property(lchip, vlan_id, CTC_VLAN_PROP_LEARNING_EN, &value));
    *enable                        = value ? TRUE : FALSE;

    return CTC_E_NONE;
}

/**
 @brief The function is to set igmp snooping enable on the vlan
*/
int32
ctc_usw_vlan_set_igmp_snoop_en(uint8 lchip, uint16 vlan_id, bool enable)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_set_property(lchip, vlan_id, CTC_VLAN_PROP_IGMP_SNOOP_EN, enable));
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to get igmp snooping enable of the vlan
*/
int32
ctc_usw_vlan_get_igmp_snoop_en(uint8 lchip, uint16 vlan_id, bool* enable)
{
    uint32 value                   = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);
    CTC_ERROR_RETURN(sys_usw_vlan_get_property(lchip, vlan_id, CTC_VLAN_PROP_IGMP_SNOOP_EN, &value));
    *enable                        = value ? TRUE : FALSE;
    return CTC_E_NONE;
}

/**
 @brief The function is to set dhcp exception action of the vlan
*/
int32
ctc_usw_vlan_set_dhcp_excp_type(uint8 lchip, uint16 vlan_id, ctc_exception_type_t type)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_set_property(lchip, vlan_id, CTC_VLAN_PROP_DHCP_EXCP_TYPE, type));
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to get dhcp exception action of the vlan
*/
int32
ctc_usw_vlan_get_dhcp_excp_type(uint8 lchip, uint16 vlan_id, ctc_exception_type_t* type)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_vlan_get_property(lchip, vlan_id, CTC_VLAN_PROP_DHCP_EXCP_TYPE, (uint32*)type));

    return CTC_E_NONE;
}

/**
 @brief The function is to set arp exception action of the vlan
*/
int32
ctc_usw_vlan_set_arp_excp_type(uint8 lchip, uint16 vlan_id, ctc_exception_type_t type)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_set_property(lchip, vlan_id, CTC_VLAN_PROP_ARP_EXCP_TYPE, type));
    }

    return CTC_E_NONE;
}

/**
 @brief The function is to get arp exception action of the vlan
*/
int32
ctc_usw_vlan_get_arp_excp_type(uint8 lchip, uint16 vlan_id, ctc_exception_type_t* type)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_vlan_get_property(lchip, vlan_id, CTC_VLAN_PROP_ARP_EXCP_TYPE, (uint32*)type));

    return CTC_E_NONE;
}

int32
ctc_usw_vlan_set_property(uint8 lchip, uint16 vlan_id, ctc_vlan_property_t vlan_prop, uint32 value)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_set_property(lchip, vlan_id, vlan_prop, value));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_vlan_set_property_array(uint8 lchip, uint16 vlan_id, ctc_property_array_t* vlan_prop, uint16* array_num)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint16 loop                    = 0;
    int32 ret                      = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(vlan_prop);
    CTC_PTR_VALID_CHECK(array_num);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        for(loop = 0; loop < *array_num; loop++)
        {
            ret = sys_usw_vlan_set_property(lchip, vlan_id, vlan_prop[loop].property, vlan_prop[loop].data);
            if (ret && ret != CTC_E_NOT_SUPPORT)
            {
                *array_num = loop;
                return ret;
            }
        }
    }

    return CTC_E_NONE;
}

int32
ctc_usw_vlan_get_property(uint8 lchip, uint16 vlan_id, ctc_vlan_property_t vlan_prop, uint32* value)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_vlan_get_property(lchip, vlan_id, vlan_prop, value));

    return CTC_E_NONE;
}

int32
ctc_usw_vlan_set_direction_property(uint8 lchip, uint16 vlan_id, ctc_vlan_direction_property_t vlan_prop, ctc_direction_t dir, uint32 value)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_set_direction_property(lchip, vlan_id, vlan_prop, dir, value));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_vlan_get_direction_property(uint8 lchip, uint16 vlan_id, ctc_vlan_direction_property_t vlan_prop, ctc_direction_t dir, uint32* value)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_vlan_get_direction_property(lchip, vlan_id, vlan_prop, dir, value));

    return CTC_E_NONE;
}
int32
ctc_usw_vlan_set_acl_property(uint8 lchip, uint16 vlan_id, ctc_acl_property_t* p_prop)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_set_acl_property(lchip, vlan_id, p_prop));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_vlan_get_acl_property(uint8 lchip, uint16 vlan_id, ctc_acl_property_t* p_prop)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_vlan_get_acl_property(lchip, vlan_id, p_prop));

    return CTC_E_NONE;
}
/***************************************************************
 *
 *  Vlan Classification Begin
 *
 ***************************************************************/

/**
 @brief The function is to add one vlan classification rule
*/
int32
ctc_usw_vlan_add_vlan_class(uint8 lchip, ctc_vlan_class_t* p_vlan_class)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,ret,
                                sys_usw_vlan_add_vlan_class(lchip, p_vlan_class));
    }

    /*roll back if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_vlan_remove_vlan_class(lchip,p_vlan_class);
    }
    return ret;
}

/**
 @brief The fucntion is to remove on vlan classification rule
*/
int32
ctc_usw_vlan_remove_vlan_class(uint8 lchip, ctc_vlan_class_t* p_vlan_class)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,ret,
                                        sys_usw_vlan_remove_vlan_class(lchip, p_vlan_class));
    }

    return ret;
}

/**
 @brief The fucntion is to flush vlan classification by type
*/
int32
ctc_usw_vlan_remove_all_vlan_class(uint8 lchip, ctc_vlan_class_type_t type)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,ret,
                                        sys_usw_vlan_remove_all_vlan_class(lchip, type));
    }

    return ret;
}

/**
 @brief Add vlan classification default entry per label
*/
int32
ctc_usw_vlan_add_default_vlan_class(uint8 lchip, ctc_vlan_class_type_t type, ctc_vlan_miss_t* p_action)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,ret,
                            sys_usw_vlan_add_default_vlan_class(lchip, type, p_action));
    }

    /*roll back if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_vlan_remove_default_vlan_class(lchip,type);
    }

    return ret;
}

/**
 @brief Remove vlan classification default entry per label
*/
int32
ctc_usw_vlan_remove_default_vlan_class(uint8 lchip, ctc_vlan_class_type_t type)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,ret,
                                    sys_usw_vlan_remove_default_vlan_class(lchip, type));
    }

    return ret;
}

/**
 @brief The function is to create a vlan range
*/
int32
ctc_usw_vlan_create_vlan_range_group(uint8 lchip, ctc_vlan_range_info_t* vrange_info, bool is_svlan)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,ret,
                                sys_usw_vlan_create_vlan_range(lchip, vrange_info, is_svlan));
    }

    /*roll back if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_vlan_destroy_vlan_range(lchip, vrange_info);
    }

    return ret;
}

int32
ctc_usw_vlan_get_vlan_range_type(uint8 lchip, ctc_vlan_range_info_t* vrange_info, bool* is_svlan)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_vlan_get_vlan_range_type(lchip, vrange_info, is_svlan));

    return CTC_E_NONE;
}

int32
ctc_usw_vlan_destroy_vlan_range_group(uint8 lchip, ctc_vlan_range_info_t* vrange_info)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,ret,
                                       sys_usw_vlan_destroy_vlan_range(lchip, vrange_info));
    }

    return ret;
}

int32
ctc_usw_vlan_add_vlan_range(uint8 lchip, ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_t* vlan_range)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,ret,
                        sys_usw_vlan_add_vlan_range_member(lchip, vrange_info, vlan_range));
    }

    /*roll back if error exist*/
    CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
    {
        sys_usw_vlan_remove_vlan_range_member(lchip, vrange_info, vlan_range);
    }

    return ret;
}

/**
 @brief remove vlan range from a vrange_group
*/

int32
ctc_usw_vlan_remove_vlan_range(uint8 lchip, ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_t* vlan_range)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,ret,
                        sys_usw_vlan_remove_vlan_range_member(lchip, vrange_info, vlan_range));
    }

    return ret;
}

int32
ctc_usw_vlan_get_vlan_range(uint8 lchip, ctc_vlan_range_info_t* vrange_info, ctc_vlan_range_group_t* vrange_group, uint8* count)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_vlan_get_vlan_range_member(lchip, vrange_info, vrange_group, count));

    return CTC_E_NONE;

}

/**
 @brief The function is to add one vlan mapping entry on the port in IPE
*/
int32
ctc_usw_vlan_add_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_vlan_mapping);

    if (CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT)
            || CTC_IS_LINKAGG_PORT(gport))
    {
        all_lchip = 1;
    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(gport,lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,ret,
                    sys_usw_vlan_add_vlan_mapping(lchip, gport, p_vlan_mapping));
    }

    /*roll back if error exist*/
    if(1 == all_lchip)
    {
        CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
        {
            sys_usw_vlan_remove_vlan_mapping(lchip,gport,p_vlan_mapping);
        }
    }

    return ret;

}

/**
 @brief The function is to get one vlan mapping entry
*/
int32
ctc_usw_vlan_get_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_vlan_mapping);

    if (CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT)
            || CTC_IS_LINKAGG_PORT(gport))
    {
        lchip_start = 0;
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(gport,lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_get_vlan_mapping(lchip, gport, p_vlan_mapping));
    }

    return ret;
}

/**
 @brief The function is to update one vlan mapping entry on the port in IPE
*/
int32
ctc_usw_vlan_update_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_vlan_mapping);

    if (CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT)
            || CTC_IS_LINKAGG_PORT(gport))
    {
        all_lchip = 1;
    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(gport,lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;

    }
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_update_vlan_mapping(lchip, gport, p_vlan_mapping));
    }
    return ret;
}

/**
 @brief The function is to remove one vlan mapping entry on the port in IPE
*/
int32
ctc_usw_vlan_remove_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_mapping_t* p_vlan_mapping)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_vlan_mapping);

    if (CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT)
            || CTC_IS_LINKAGG_PORT(gport))
    {
        all_lchip = 1;
    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(gport, lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,ret,
                        sys_usw_vlan_remove_vlan_mapping(lchip, gport, p_vlan_mapping));
    }
    return ret;

}

/**
 @brief The function is to add vlan mapping default entry on the port
*/
int32
ctc_usw_vlan_add_default_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_miss_t* p_action)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    if(CTC_IS_LINKAGG_PORT(gport))
    {
        all_lchip = 1;

    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(gport, lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,ret,
            sys_usw_vlan_add_default_vlan_mapping(lchip, gport, p_action));
    }

    /*roll back if error exist*/
    if(1 == all_lchip)
    {
        CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
        {
            sys_usw_vlan_remove_default_vlan_mapping(lchip, gport);
        }
    }
    return ret;
}

/**
 @brief The function is to remove vlan mapping miss match default entry of port
*/
int32
ctc_usw_vlan_remove_default_vlan_mapping(uint8 lchip, uint32 gport)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    if(CTC_IS_LINKAGG_PORT(gport))
    {
        all_lchip = 1;

    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(gport, lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,ret,
                        sys_usw_vlan_remove_default_vlan_mapping(lchip, gport));
    }
    return ret;
}

/**
 @brief The function is to remove all vlan mapping entries on port
*/
int32
ctc_usw_vlan_remove_all_vlan_mapping_by_port(uint8 lchip, uint32 gport)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    if(CTC_IS_LINKAGG_PORT(gport))
    {
        all_lchip = 1;

    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(gport, lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,ret,
                        sys_usw_vlan_remove_all_vlan_mapping_by_port(lchip, gport));
    }
    return ret;
}

/**
 @brief The function is to add one egress vlan mapping entry on the port in EPE
*/
int32
ctc_usw_vlan_add_egress_vlan_mapping(uint8 lchip, uint32 gport, ctc_egress_vlan_mapping_t* p_vlan_mapping)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_vlan_mapping);

    if (CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT)
            || CTC_IS_LINKAGG_PORT(gport))
    {
        all_lchip = 1;

    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(gport, lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,ret,
                        sys_usw_vlan_add_egress_vlan_mapping(lchip, gport, p_vlan_mapping));
    }

    /*roll back if error exist*/
    if(1 == all_lchip)
    {
        CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
        {
            sys_usw_vlan_remove_egress_vlan_mapping(lchip, gport, p_vlan_mapping);
        }
    }
    return ret;
}

/**
 @brief The function is to remove one egress vlan mapping entry on the port in EPE
*/
int32
ctc_usw_vlan_remove_egress_vlan_mapping(uint8 lchip, uint32 gport, ctc_egress_vlan_mapping_t* p_vlan_mapping)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_vlan_mapping);

    if (CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT)
            || CTC_IS_LINKAGG_PORT(gport))
    {
        all_lchip = 1;
    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(gport, lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,ret,
                        sys_usw_vlan_remove_egress_vlan_mapping(lchip, gport, p_vlan_mapping));
    }
    return ret;
}
/**
 @brief The function is to get one egress vlan mapping entry
*/
int32
ctc_usw_vlan_get_egress_vlan_mapping(uint8 lchip, uint32 gport, ctc_egress_vlan_mapping_t* p_vlan_mapping)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_vlan_mapping);

    if (CTC_FLAG_ISSET(p_vlan_mapping->flag, CTC_VLAN_MAPPING_FLAG_USE_LOGIC_PORT)
            || CTC_IS_LINKAGG_PORT(gport))
    {
        lchip_start = 0;
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(gport,lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_get_egress_vlan_mapping(lchip, gport, p_vlan_mapping));
    }

    return CTC_E_NONE;
}
/**
 @brief The function is to add egress vlan mapping default entry on the port
*/
int32
ctc_usw_vlan_add_default_egress_vlan_mapping(uint8 lchip, uint32 gport, ctc_vlan_miss_t* p_action)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    if(CTC_IS_LINKAGG_PORT(gport))
    {
        all_lchip =1;
    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(gport, lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }
    CTC_FOREACH_LCHIP(lchip_start, lchip_end,all_lchip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,ret,
                        sys_usw_vlan_add_default_egress_vlan_mapping(lchip, gport, p_action));
    }

    /*roll back if error exist*/
    if(1 == all_lchip)
    {
        CTC_FOREACH_ROLLBACK(lchip_start,lchip_end)
        {
            sys_usw_vlan_remove_default_egress_vlan_mapping(lchip, gport);
        }
    }
    return ret;
}

/**
 @brief The function is to remove egress vlan mapping default entry of port
*/
int32
ctc_usw_vlan_remove_default_egress_vlan_mapping(uint8 lchip, uint32 gport)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    if(CTC_IS_LINKAGG_PORT(gport))
    {
        all_lchip = 1;

    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(gport, lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,ret,
                        sys_usw_vlan_remove_default_egress_vlan_mapping(lchip, gport));
    }

    return ret;

}

/**
 @brief The function is to remove all egress vlan mapping entries on port
*/
int32
ctc_usw_vlan_remove_all_egress_vlan_mapping_by_port(uint8 lchip, uint32 gport)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    uint8 all_lchip                = 0;
    int32 ret                      = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);

    if(CTC_IS_LINKAGG_PORT(gport))
    {
        all_lchip = 1;
    }
    else
    {
        SYS_MAP_GPORT_TO_LCHIP(gport, lchip_start);
        lchip_end = lchip_start + 1;
        all_lchip = 0;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, all_lchip)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,ret,
                        sys_usw_vlan_remove_all_egress_vlan_mapping_by_port(lchip, gport));
    }
    return ret;
}

int32
ctc_usw_vlan_set_mac_auth(uint8 lchip, uint16 vlan_id, bool enable)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_set_mac_auth(lchip, vlan_id, enable));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_vlan_get_mac_auth(uint8 lchip, uint16 vlan_id, bool* enable)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_VLAN);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_vlan_get_mac_auth(lchip, vlan_id, enable));
    }

    return CTC_E_NONE;
}


