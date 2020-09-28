/**
 @file ctc_usw_port.c

 @author  Copyright (C) 2011 Centec Networks Inc.  All rights reserved.

 @date 2011-11-9

 @version v2.0

 The file provide all port related APIs of GreatBelt SDK.
*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "ctc_error.h"
#include "ctc_debug.h"

#include "ctc_usw_port.h"
#include "sys_usw_port.h"
#include "sys_usw_mac.h"
#include "sys_usw_common.h"
/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
/****************************************************************************
 *
* Function
*
*****************************************************************************/
/**
 @brief Initialize the port module

 @param[in]  port_global_cfg

 @return CTC_E_XXX

*/
int32
ctc_usw_port_init(uint8 lchip, void* port_global_cfg)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    ctc_port_global_cfg_t global_cfg;


    LCHIP_CHECK(lchip);

    if (NULL == port_global_cfg)
    {
        sal_memset(&global_cfg, 0, sizeof(ctc_port_global_cfg_t));
    }
    else
    {
        sal_memcpy(&global_cfg, port_global_cfg, sizeof(ctc_port_global_cfg_t));
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_port_init(lchip, &global_cfg));
        CTC_ERROR_RETURN(sys_usw_mac_init(lchip, &global_cfg));
    }

    return CTC_E_NONE;
}

/**
 @brief De-Initialize port module

 @param[in] lchip    local chip id

 @return CTC_E_XXX
*/
extern int32
ctc_usw_port_deinit(uint8 lchip)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_mac_deinit(lchip));
        CTC_ERROR_RETURN(sys_usw_port_deinit(lchip));
    }

    return CTC_E_NONE;
}

/**
 @brief set port default configure

 @param[in]  gport global physical port

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_default_cfg(uint8 lchip, uint32 gport)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_set_default_cfg(lchip, gport));

    return CTC_E_NONE;
}

/**
 @brief Set port mac enable

 @param[in] gport   global physical port

 @param[in] enable  a boolean value denote port mac enable or not

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_mac_en(uint8 lchip, uint32 gport, bool enable)
{
    uint32 value = enable;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_MAC_EN, value));

    return CTC_E_NONE;
}

/**
 @brief Get port mac enable

 @param[in] gport   global physical port

 @param[out] enable  a boolean value denote port mac enable or not

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_mac_en(uint8 lchip, uint32 gport, bool* enable)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);

    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_MAC_EN, &value));

    *enable = (1 == value) ? TRUE : FALSE;
    return CTC_E_NONE;
}

/**
 @brief Set port enable/disable, include Rx,Tx

 @param[in] gport global physical port

 @param[in] enable a boolean value wanted to set

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_port_en(uint8 lchip, uint32 gport, bool enable)
{
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,
							sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_PORT_EN, enable));
    }
    return ret;
}

/**
 @brief Get port enable/disable, include Rx,Tx

 @param[in] gport  global physical port

 @param[out] enable  a boolean value to get(it is return TRUE when Rx, Tx are all enable)

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_port_en(uint8 lchip, uint32 gport, bool* enable)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);

    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_PORT_EN, &value));

    *enable = (1 == value) ? TRUE : FALSE;
    return CTC_E_NONE;
}

/**
 @brief Set port's vlan tag control mode

 @param[in] gport global physical port

 @param[in] mode  refer to ctc_vlantag_ctl_t

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_vlan_ctl(uint8 lchip, uint32 gport, ctc_vlantag_ctl_t mode)
{
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,
			sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_VLAN_CTL, mode));
    }
    return ret;
}

/**
 @brief Get port's vlan tag control mode

 @param[in] gport global physical port

 @param[out] mode  refer to ctc_vlantag_ctl_t

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_vlan_ctl(uint8 lchip, uint32 gport, ctc_vlantag_ctl_t* mode)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_VLAN_CTL, (uint32*)mode));

    return CTC_E_NONE;
}

/**
 @brief Set default vlan id of packet which receive from this port

 @param[in] gport global physical port

 @param[in] vid default vlan id of the packet

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_default_vlan(uint8 lchip, uint32 gport, uint16 vid)
{
    uint32 value = vid;
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,
			sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_DEFAULT_VLAN, value));
    }
    return ret;
}

/**
 @brief Get default vlan id of packet which receive from this port

 @param[in] gport global physical port

 @param[out] vid default vlan id of the packet

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_default_vlan(uint8 lchip, uint32 gport, uint16* vid)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(vid);
    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_DEFAULT_VLAN, &value));

    *vid = value;
    return CTC_E_NONE;
}

/**
 @brief Set vlan filtering enable/disable on the port

 @param[in] gport global physical port

 @param[in] dir  flow direction, refer to ctc_direction_t

 @param[in] enable a boolean value denote whether the function is enable

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_vlan_filter_en(uint8 lchip, uint32 gport, ctc_direction_t dir, bool enable)
{
    uint32 value = enable;
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

	FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,sys_usw_port_set_direction_property(lchip, gport, CTC_PORT_DIR_PROP_VLAN_FILTER_EN, dir, value));
    }
    return ret;
}

/**
 @brief Get vlan filtering enable/disable on the port

 @param[in] gport global physical port

 @param[in] dir  flow direction, refer to ctc_direction_t

 @param[out] enable a boolean value denote whether the function is enable

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_vlan_filter_en(uint8 lchip, uint32 gport, ctc_direction_t dir, bool* enable)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);

    CTC_ERROR_RETURN(sys_usw_port_get_direction_property(lchip, gport, CTC_PORT_DIR_PROP_VLAN_FILTER_EN, dir, &value));

    *enable = (1 == value) ? TRUE : FALSE;
    return CTC_E_NONE;
}

/**
 @brief Set vlan domain when packet singgle tagged with tpid 0x8100

 @param[in] gport global physical port

 @param[in] vlan_domain  cvlan or svlan

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_vlan_domain(uint8 lchip, uint32 gport, ctc_port_vlan_domain_type_t vlan_domain)
{
    uint32 value = vlan_domain;
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;
    uint8 lchip_tmp = lchip;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret, sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_VLAN_DOMAIN, value));
    }

    lchip = lchip_tmp;
    if (!vlan_domain) /* svlan domain */
    {
	    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
		{
			CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,
				sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_DOT1Q_TYPE, CTC_DOT1Q_TYPE_BOTH));
		}
    }
    else    /* cvlan domain */
    {
        CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
		{
			CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,
				sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_DOT1Q_TYPE, CTC_DOT1Q_TYPE_CVLAN));

		}
    }
    return ret;
}

/**
 @brief Get vlan domain

 @param[in] gport global physical port

 @param[out] vlan_domain  cvlan or svlan

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_vlan_domain(uint8 lchip, uint32 gport, ctc_port_vlan_domain_type_t* vlan_domain)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(vlan_domain);
    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_VLAN_DOMAIN, &value));

    *vlan_domain = (1 == value) ? CTC_PORT_VLAN_DOMAIN_CVLAN : CTC_PORT_VLAN_DOMAIN_SVLAN;
    return CTC_E_NONE;
}

/**
 @brief Set what tag the packet with transmit from the port

 @param[in] gport global physical port

 @param[in] type dot1q type of port untagged/ctagged/stagged/double-tagged

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_dot1q_type(uint8 lchip, uint32 gport, ctc_dot1q_type_t type)
{
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret, sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_DOT1Q_TYPE, type));
    }
    return ret;
}

/**
 @brief  Get what tag the packet with transmit from the port

 @param[in] gport global physical port

 @param[out] type dot1q type of port untagged/ctagged/stagged/double-tagged

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_dot1q_type(uint8 lchip, uint32 gport, ctc_dot1q_type_t* type)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_DOT1Q_TYPE, (uint32*)type));

    return CTC_E_NONE;
}

/**
 @brief Set port whether the transmit is enable

 @param[in] gport global physical port

 @param[in] enable a boolean value wanted to set

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_transmit_en(uint8 lchip, uint32 gport, bool enable)
{
    uint32 value = enable;
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_TRANSMIT_EN, value));
    }
    return ret;
}

/**
 @brief Get port whether the transmit is enable

 @param[in] gport global physical port

 @param[out] enable a boolean value wanted to set

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_transmit_en(uint8 lchip, uint32 gport, bool* enable)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);
    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_TRANSMIT_EN, &value));

    *enable = (1 == value) ? TRUE : FALSE;
    return CTC_E_NONE;
}

/**
 @brief Set port whether the receive is enable

 @param[in] gport global physical port

 @param[out] enable a boolean value to get

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_receive_en(uint8 lchip, uint32 gport, bool enable)
{
    uint32 value = enable;
	uint8 lchip_start = 0;
    uint8 lchip_end = 0;
	int32 ret = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
	{
     	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT,
						ret, sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_RECEIVE_EN, value));
    }
    return ret;
}

/**
 @brief Get port whether the receive is enable

 @param[in] gport global physical port

 @param[in] enable a boolean value wanted to set

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_receive_en(uint8 lchip, uint32 gport, bool* enable)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);

    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_RECEIVE_EN, &value));

    *enable = (1 == value) ? TRUE : FALSE;
    return CTC_E_NONE;
}

/**
 @brief Set port whehter layer2 bridge function is enable

 @param[in] gport global physical port

 @param[out] enable a boolean value to get

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_bridge_en(uint8 lchip, uint32 gport, bool enable)
{
    uint32 value = enable;
	uint8 lchip_start = 0;
    uint8 lchip_end = 0;
	int32 ret = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
	{
     	CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_SUPPORT,
				ret, sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_BRIDGE_EN, value));
   	}

    return ret;
}

/**
 @brief Get port whehter layer2 bridge function is enable

 @param[in] gport global physical port

 @param[out] enable a boolean value wanted to set

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_bridge_en(uint8 lchip, uint32 gport, bool* enable)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);

    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_BRIDGE_EN, &value));

    *enable = (1 == value) ? TRUE : FALSE;
    return CTC_E_NONE;
}

/**
 @brief Set the port as physical interface

 @param[in] gport global physical port

 @param[in] enable   enable/disable the port as physical interface

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_phy_if_en(uint8 lchip, uint32 gport, bool enable)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_set_phy_if_en(lchip, gport, enable));

    return CTC_E_NONE;
}

/**
 @brief Get whether route function on port is enable

 @param[in]  gport   global physical port

 @param[out] l3if_id the id of associated l3 interface

 @param[out] enable  enable/disable the port as physical interface

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_phy_if_en(uint8 lchip, uint32 gport, uint16* l3if_id, bool* enable)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_get_phy_if_en(lchip, gport, l3if_id, enable));

    return CTC_E_NONE;
}

/**
 @brief Set the port as sub interface

 @param[in] gport global physical port

 @param[in] enable   enable/disable the port as sub interface

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_sub_if_en(uint8 lchip, uint32 gport, bool enable)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_set_sub_if_en(lchip, gport, enable));

    return CTC_E_NONE;
}

/**
 @brief Get whether sub-route function on port is enable

 @param[in]  gport   global physical port


 @param[out] enable  enable/disable the port as sub interface

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_sub_if_en(uint8 lchip, uint32 gport, bool* p_enable)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_get_sub_if_en(lchip, gport, p_enable));

    return CTC_E_NONE;
}


/**
 @brief The function is to set stag tpid index

 @param[in] gport global physical port

 @param[in] dir  flow direction, refer to ctc_direction_t

 @param[in] index stag tpid index, can be configed in ethernet ctl register

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_stag_tpid_index(uint8 lchip, uint32 gport, ctc_direction_t dir, uint8 index)
{
    uint32 value = index;
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

	FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,sys_usw_port_set_direction_property(lchip, gport, CTC_PORT_DIR_PROP_STAG_TPID_INDEX, dir, value));
    }
    return ret;
}

/**
 @brief The function is to get stag tpid index

 @param[in] gport global physical port

 @param[in] dir  flow direction, refer to ctc_direction_t

 @param[out] index stag tpid index, the index point to stag tpid value

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_stag_tpid_index(uint8 lchip, uint32 gport, ctc_direction_t dir, uint8* index)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(index);
    CTC_ERROR_RETURN(sys_usw_port_get_direction_property(lchip, gport, CTC_PORT_DIR_PROP_STAG_TPID_INDEX, dir, &value));

    *index = value;
    return CTC_E_NONE;
}

/**
 @brief Set port cross connect forwarding by port if enable

 @param[in] gport global physical port

 @param[in] nhid gport's cross connect forward nhid, nhid value 0xFFFFFFFF for disable cross connect on the port

 @remark  Set port cross connect nhid, the packet will be forwarding by nhid specified without looking up mac or routing
          when nhid is valid

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_cross_connect(uint8 lchip, uint32 gport, uint32 nhid)
{
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_CROSS_CONNECT_EN, nhid));
    }
    return ret;
}

/**
 @brief Get port cross connect

 @param[in] gport global physical port

 @param[out] p_value gport's cross connect forward nhid, value 0xFFFFFFFF means port cross connect disabled on the port

 @remark  Get port cross connect nhid value

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_cross_connect(uint8 lchip, uint32 gport, uint32* p_value)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_CROSS_CONNECT_EN, p_value));

    return CTC_E_NONE;
}

/**
 @brief Set learning enable/disable on this port

 @param[in] gport global physical port

 @param[in] enable mac learning is enable on the port if enable set

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_learning_en(uint8 lchip, uint32 gport, bool enable)
{
    uint32 value = enable;
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_LEARNING_EN, value));
    }
    return ret;
}

/**
 @brief Get learning enable/disable on the port

 @param[in] gport global physical port

 @param[out] enable mac learning is enable on the port if enable set

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_learning_en(uint8 lchip, uint32 gport, bool* enable)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);

    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_LEARNING_EN, &value));

    *enable = (1 == value) ? TRUE : FALSE;
    return CTC_E_NONE;
}

/**
 @brief Set use outer ttl in case of tunnel

 @param[in] gport global physical port

 @param[in] enable a boolen vaule denote the function is enable/disable

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_use_outer_ttl(uint8 lchip, uint32 gport, bool enable)
{
    uint32 value = enable;
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_USE_OUTER_TTL, value));
    }
    return ret;
}

/**
 @brief Get use outer ttl in case of tunnel

 @param[in] gport global physical port

 @param[out] enable a boolen vaule denote the function is enable/disable

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_use_outer_ttl(uint8 lchip, uint32 gport, bool* enable)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);

    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_USE_OUTER_TTL, &value));

    *enable = (1 == value) ? TRUE : FALSE;
    return CTC_E_NONE;
}

int32
ctc_usw_port_set_untag_dft_vid(uint8 lchip, uint32 gport, bool enable, bool untag_svlan)
{
    uint32 value = 0;
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    if (enable && untag_svlan)
    {
        value = CTC_PORT_UNTAG_PVID_TYPE_SVLAN;
    }
    else if (enable && !untag_svlan)
    {
        value = CTC_PORT_UNTAG_PVID_TYPE_CVLAN;
    }
    else
    {
        value = CTC_PORT_UNTAG_PVID_TYPE_NONE;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_UNTAG_PVID_TYPE, value));
    }
    return ret;
}

int32
ctc_usw_port_get_untag_dft_vid(uint8 lchip, uint32 gport, bool* enable, bool* untag_svlan)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);
    CTC_PTR_VALID_CHECK(untag_svlan);

    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_UNTAG_PVID_TYPE, &value));

    switch(value)
    {
    case CTC_PORT_UNTAG_PVID_TYPE_NONE:
        *enable = 0;
        break;
    case CTC_PORT_UNTAG_PVID_TYPE_SVLAN:
        *enable = 1;
        *untag_svlan = 1;
        break;
    case CTC_PORT_UNTAG_PVID_TYPE_CVLAN:
        *enable = 1;
        *untag_svlan = 0;
        break;
    }

    return CTC_E_NONE;
}

/**
 @brief Set protocol based vlan class enable/disable on the port

 @param[in] gport global physical port

 @param[in] enable a boolen vaule denote the function is enable/disable

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_protocol_vlan_en(uint8 lchip, uint32 gport, bool enable)
{
    uint32 value = enable;
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

	FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,
			sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_PROTOCOL_VLAN_EN, value));
    }
    return ret;
}

/**
 @brief Get protocol vlan enable/disable of the port

 @param[in] gport global physical port

 @param[out] enable A boolen vaule denote the function is enable/disable

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_protocol_vlan_en(uint8 lchip, uint32 gport, bool* enable)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);

    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_PROTOCOL_VLAN_EN, &value));

    *enable = (1 == value) ? TRUE : FALSE;
    return CTC_E_NONE;
}

/**
 @brief Set random log function of port

 @param[in] gport global physical port

 @param[in] dir   flow direction, refer to ctc_direction_t

 @param[in] enable A boolen vaule denote the function is enable/disable

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_random_log_en(uint8 lchip, uint32 gport, ctc_direction_t dir, bool enable)
{
    uint32 value = enable;
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

	FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,sys_usw_port_set_direction_property(lchip, gport, CTC_PORT_DIR_PROP_RANDOM_LOG_EN, dir, value));
    }
    return ret;
}

/**
 @brief Get random log function of port

 @param[in] gport global physical port

 @param[in] dir   flow direction, refer to ctc_direction_t

 @param[out] enable A boolen vaule denote the function is enable/disable

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_random_log_en(uint8 lchip, uint32 gport, ctc_direction_t dir, bool* enable)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);

    CTC_ERROR_RETURN(sys_usw_port_get_direction_property(lchip, gport, CTC_PORT_DIR_PROP_RANDOM_LOG_EN, dir, &value));

    *enable = (1 == value) ? TRUE : FALSE;
    return CTC_E_NONE;
}

/**
 @brief Set the percentage of received packet for random log

 @param[in] gport global physical port

 @param[in] dir  flow direction, refer to ctc_direction_t

 @param[in] percent The percentage of received packet for random log,and the range of percent is 0-100

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_random_log_percent(uint8 lchip, uint32 gport, ctc_direction_t dir, uint8 percent)
{
    uint32 value = percent;
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

	FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,sys_usw_port_set_direction_property(lchip, gport, CTC_PORT_DIR_PROP_RANDOM_LOG_PERCENT, dir, value));
    }
    return ret;
}

/**
 @brief Get the percentage of received packet for random log

 @param[in] gport global physical port

 @param[in] dir  flow direction, refer to ctc_direction_t

 @param[out] percent The percentage of received packet for random log,and the range of percent is 0-100

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_random_log_percent(uint8 lchip, uint32 gport, ctc_direction_t dir, uint8* percent)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(percent);

    CTC_ERROR_RETURN(sys_usw_port_get_direction_property(lchip, gport, CTC_PORT_DIR_PROP_RANDOM_LOG_PERCENT, dir, &value));

    *percent = value;
    return CTC_E_NONE;
}

/**
 @brief The function is to set vlan range on the port

 @param[in] gport global physical port

 @param[in] vrange_info vlan range info, refer to ctc_vlan_range_info_t

 @param[in] enable enable or disable vlan range

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_vlan_range(uint8 lchip, uint32 gport, ctc_vlan_range_info_t* vrange_info, bool enable)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_set_vlan_range(lchip, gport, vrange_info, enable));

    return CTC_E_NONE;
}

/**
 @brief The function is to get vlan range on the port

 @param[in] gport global physical port

 @param[in|out] vrange_info vlan range info, refer to ctc_vlan_range_info_t

 @param[out] enable enable or disable vlan range

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_vlan_range(uint8 lchip, uint32 gport, ctc_vlan_range_info_t* vrange_info, bool* enable)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_get_vlan_range(lchip, gport, vrange_info, enable));

    return CTC_E_NONE;
}

/**
 @brief Set port speed mode

 @param[in] gport       global physical port

 @param[in] speed_mode  Speed at 10M 100M 1G of Gmac

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_speed(uint8 lchip, uint32 gport, ctc_port_speed_t speed_mode)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_SPEED, speed_mode));

    return CTC_E_NONE;
}

/**
 @brief Get port speed mode

 @param[in] gport       global physical port

 @param[out] speed_mode Speed at 10M 100M 1G of Gmac

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_speed(uint8 lchip, uint32 gport, ctc_port_speed_t* speed_mode)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_SPEED, (uint32*)speed_mode));

    return CTC_E_NONE;
}

/**
 @brief Set port Auto-neg mode

 @param[in] gport       global physical port

 @param[in] enable   enable or disable this function

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_auto_neg(uint8 lchip, uint32 gport, uint32 value)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_AUTO_NEG_EN, value));

    return CTC_E_NONE;
}

/**
 @brief Set port min frame size

 @param[in] gport       global phyical port

 @param[in] size   min frame size

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_min_frame_size(uint8 lchip, uint32 gport, uint8 size)
{
    uint32 value = size;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_MIN_FRAME_SIZE, value));

    return CTC_E_NONE;
}

/**
 @brief Get port min frame size

 @param[in] gport   global phyical port

 @param[out] size   min frame size

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_min_frame_size(uint8 lchip, uint32 gport, uint8* size)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(size);

    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_MIN_FRAME_SIZE, &value));

    *size = value;
    return CTC_E_NONE;
}

/**
 @brief Set max frame size per port

 @param[in] gport   global phyical port

 @param[in] value   max frame size

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_max_frame(uint8 lchip, uint32 gport, uint32 value)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_MAX_FRAME_SIZE, value));

    return CTC_E_NONE;
}

/**
 @brief Get max frame size per port

 @param[in]  gport    global phyical port

 @param[out] p_value  max frame size

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_max_frame(uint8 lchip, uint32 gport, uint32* p_value)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_MAX_FRAME_SIZE, p_value));

    return CTC_E_NONE;
}

/**
 @brief Set flow control of port

 @param[in] Point to flow control property

 @remark  Enable/disable flow control or priority flow control

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_flow_ctl_en(uint8 lchip, ctc_port_fc_prop_t *p_fc_prop)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_fc_prop);
    SYS_MAP_GPORT_TO_LCHIP(p_fc_prop->gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_set_flow_ctl_en(lchip, p_fc_prop));

    return CTC_E_NONE;
}


/**
 @brief Get flow control of port

 @param[in|out] Point to flow control property

 @remark  Get flow control or priority flow control enable/disable

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_flow_ctl_en(uint8 lchip, ctc_port_fc_prop_t *p_fc_prop)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_fc_prop);
    SYS_MAP_GPORT_TO_LCHIP(p_fc_prop->gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_get_flow_ctl_en(lchip, p_fc_prop));

    return CTC_E_NONE;
}

/**
 @brief Set port preamble length

 @param[in] gport   global phyical port

 @param[in] pre_bytes   preamble length value

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_preamble(uint8 lchip, uint32 gport, uint8 pre_bytes)
{
    uint32 value = pre_bytes;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_PREAMBLE, value));

    return CTC_E_NONE;
}

/**
 @brief Get port preamble length

 @param[in] gport       global phyical port

 @param[out] pre_bytes   preamble length value

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_preamble(uint8 lchip, uint32 gport, uint8* pre_bytes)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(pre_bytes);

    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_PREAMBLE, &value));

    *pre_bytes = value;
    return CTC_E_NONE;
}


/**
 @brief Set port pading

 @param[in] gport       global phyical port

 @param[in] enable      a boolean value denote the function enable or not

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_pading_en(uint8 lchip, uint32 gport, bool enable)
{
    uint32 value = enable;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_PADING_EN, value));

    return CTC_E_NONE;
}

/**
 @brief Get port pading

 @param[in] gport       global phyical port

 @param[in] enable      a boolean value denote the function enable or not

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_pading_en(uint8 lchip, uint32 gport, bool* enable)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);

    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_PADING_EN, &value));

    *enable = (1 == value) ? TRUE : FALSE;
    return CTC_E_NONE;
}

/**
 @brief Set port whether the srcdiscard is enable

 @param[in] gport global physical port

 @param[in] enable a boolean value wanted to set

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_srcdiscard_en(uint8 lchip, uint32 gport, bool enable)
{
    uint32 value = enable;
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

	FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,
			sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_SRC_DISCARD_EN, value));
    }
    return ret;
}

/**
 @brief Get port whether the srcdiscard is enable

 @param[in] gport global physical port

 @param[out] enable a boolean value denote the function enable or not

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_srcdiscard_en(uint8 lchip, uint32 gport, bool* enable)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);

    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_SRC_DISCARD_EN, &value));

    *enable = (1 == value) ? TRUE : FALSE;
    return CTC_E_NONE;
}

/**
 @brief Set port loopback

 @param[in] p_port_lbk point to  ctc_port_lbk_param_t

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_loopback(uint8 lchip, ctc_port_lbk_param_t* p_port_lbk)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_port_lbk);
    SYS_MAP_GPORT_TO_LCHIP(p_port_lbk->src_gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_set_loopback(lchip, p_port_lbk));

    return CTC_E_NONE;
}

/**
 @brief Set port port check enable

 @param[in] gport  global physical port

 @param[in] enable  a boolean value wanted to set

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_port_check_en(uint8 lchip, uint32 gport, bool enable)
{
    uint32 value = enable;
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

	FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,
			sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_PORT_CHECK_EN, value));
    }
    return ret;
}

/**
 @brief Get port port check enable

 @param[in] gport  global physical port

 @param[in] enable  a boolean value wanted to get

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_port_check_en(uint8 lchip, uint32 gport, bool* enable)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);

    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_PORT_CHECK_EN, &value));

    *enable = (1 == value) ? TRUE : FALSE;
    return CTC_E_NONE;
}

/**
 @brief Set ipg size of system

 @param[in] index   indicate index of ipg to be set

 @param[in] size    ipg size

 @return CTC_E_XXX

*/
int32
ctc_usw_set_ipg_size(uint8 lchip, ctc_ipg_size_t index, uint8 size)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_set_ipg_size(lchip, index, size));
    }

    return CTC_E_NONE;
}

/**
 @brief Get ipg size of system

 @param[in] index   indicate index of ipg to be set

 @param[out] size   ipg size

 @return CTC_E_XXX

*/
int32
ctc_usw_get_ipg_size(uint8 lchip, ctc_ipg_size_t index, uint8* size)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_get_ipg_size(lchip, index, size));

    return CTC_E_NONE;
}

/**
 @brief Set ipg index per port

 @param[in] gport   global phyical port

 @param[in] index   size index

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_ipg(uint8 lchip, uint32 gport, ctc_ipg_size_t index)
{
    uint32 value = index;
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

	FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,
			sys_usw_port_set_property(lchip, gport, CTC_PORT_PROP_IPG, value));
    }
    return ret;
}

/**
 @brief Get ipg index per port

 @param[in] gport   global phyical port

 @param[out] index  size index

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_ipg(uint8 lchip, uint32 gport, ctc_ipg_size_t* index)
{
    uint32 value = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(index);

    CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, CTC_PORT_PROP_IPG, &value));

    *index = value;
    return CTC_E_NONE;
}

/**
 @brief  Set isolated id with port isolation

 @param[in] gport            global port

 @param[in] p_restriction    port restriction param

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_restriction(uint8 lchip, uint32 gport, ctc_port_restriction_t* p_restriction)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_restriction);

    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_set_restriction(lchip, gport, p_restriction));

    return CTC_E_NONE;
}

/**
 @brief  Get isolated id with port isolation

 @param[in] gport            global port

 @param[in] p_restriction    port restriction param

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_restriction(uint8 lchip, uint32 gport, ctc_port_restriction_t* p_restriction)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_get_restriction(lchip, gport, p_restriction));

    return CTC_E_NONE;
}

/**
 @brief  Set the isolation group of port or isolated id

 @param[in]  lchip    local chip id

 @param[in] p_restriction    restriction group param

 @remark
    Set configuration info about restriction on port or isolation id.

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_isolation(uint8 lchip, ctc_port_isolation_t* p_port_isolation)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);

    CTC_ERROR_RETURN(sys_usw_port_set_isolation(lchip, p_port_isolation));

    return CTC_E_NONE;
}

/**
 @brief  Get the isolation group of port or isolated id

 @param[in]  lchip    local chip id

 @param[out] p_restriction    restriction group info

 @remark
    Get configuration info about restriction on port or isolation id.

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_isolation(uint8 lchip, ctc_port_isolation_t* p_port_isolation)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);

    CTC_ERROR_RETURN(sys_usw_port_get_isolation(lchip, p_port_isolation));

    return CTC_E_NONE;
}

/**
 @brief Set port property

 @param[in] gport, global physical port

 @param[in] port_prop, port property

 @param[in] value

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32 value)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);

    if ((CTC_PORT_PROP_MAC_EN == port_prop)
        || (CTC_PORT_PROP_FEC_EN == port_prop)
        || (CTC_PORT_PROP_SIGNAL_DETECT == port_prop)
        || (CTC_PORT_PROP_LINK_INTRRUPT_EN == port_prop)
        || (CTC_PORT_PROP_AUTO_NEG_EN == port_prop)
        || (CTC_PORT_PROP_AUTO_NEG_MODE == port_prop)
        || (CTC_PORT_PROP_SPEED == port_prop)
        || (CTC_PORT_PROP_MAX_FRAME_SIZE == port_prop)
        || (CTC_PORT_PROP_PREAMBLE == port_prop)
        || (CTC_PORT_PROP_MIN_FRAME_SIZE == port_prop)
        || (CTC_PORT_PROP_ERROR_CHECK == port_prop)
        || (CTC_PORT_PROP_CRC_RECALC_EN == port_prop)
        || (CTC_PORT_PROP_PADING_EN == port_prop)
        || (CTC_PORT_PROP_RX_PAUSE_TYPE == port_prop)
        || (CTC_PORT_PROP_LINKSCAN_EN == port_prop)
        || (CTC_PORT_PROP_UNIDIR_EN == port_prop)
        || (CTC_PORT_PROP_MAC_TX_IPG == port_prop)
        || (CTC_PORT_PROP_CL73_ABILITY == port_prop)
        || (CTC_PORT_PROP_CHK_CRC_EN == port_prop)
        || (CTC_PORT_PROP_STRIP_CRC_EN == port_prop)
        || (CTC_PORT_PROP_APPEND_CRC_EN == port_prop)
        || (CTC_PORT_PROP_APPEND_TOD_EN == port_prop)
        || (CTC_PORT_PROP_AUTO_NEG_FEC == port_prop))
    {
        SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
        CTC_ERROR_RETURN(sys_usw_mac_set_property(lchip, gport, port_prop, value));
    }
    else if ((port_prop == CTC_PORT_PROP_STATION_MOVE_PRIORITY)
        || (port_prop == CTC_PORT_PROP_STATION_MOVE_ACTION))
    {
        CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
        {
            CTC_FOREACH_ERROR_RETURN(CTC_E_NONE,
                                     ret,
                                     sys_usw_port_set_station_move(lchip, gport, port_prop, value));
        }
    }
    else
    {
    	CTC_FOREACH_LCHIP(lchip_start,lchip_end, 1)
    	{
        	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,
							sys_usw_port_set_property(lchip, gport, port_prop, value));
		}
    }

    return ret;
}

/**
 @brief Set port various properties at a time

 @param[in] gport, global physical port

 @param[in] port_prop, port properties array

 @param[in|out] array_num, num of port properies

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_property_array(uint8 lchip, uint32 gport, ctc_property_array_t* port_prop, uint16* array_num)
{
    uint8 lchip_start  = 0;
    uint8 lchip_end    = 0;
    int32 ret          = CTC_E_NONE;
    uint16 loop        = 0;
    uint16 loop_num    = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(port_prop);
    CTC_PTR_VALID_CHECK(array_num);
    loop_num = *array_num;

    for (loop = 0; loop < loop_num; loop++)
    {

        if ((CTC_PORT_PROP_MAC_EN == port_prop[loop].property)
        || (CTC_PORT_PROP_FEC_EN == port_prop[loop].property)
        || (CTC_PORT_PROP_SIGNAL_DETECT == port_prop[loop].property)
        || (CTC_PORT_PROP_LINK_INTRRUPT_EN == port_prop[loop].property)
        || (CTC_PORT_PROP_AUTO_NEG_EN == port_prop[loop].property)
        || (CTC_PORT_PROP_AUTO_NEG_MODE == port_prop[loop].property)
        || (CTC_PORT_PROP_SPEED == port_prop[loop].property)
        || (CTC_PORT_PROP_MAX_FRAME_SIZE == port_prop[loop].property)
        || (CTC_PORT_PROP_PREAMBLE == port_prop[loop].property)
        || (CTC_PORT_PROP_MIN_FRAME_SIZE == port_prop[loop].property)
        || (CTC_PORT_PROP_ERROR_CHECK == port_prop[loop].property)
        || (CTC_PORT_PROP_PADING_EN == port_prop[loop].property)
        || (CTC_PORT_PROP_RX_PAUSE_TYPE == port_prop[loop].property)
        || (CTC_PORT_PROP_LINKSCAN_EN == port_prop[loop].property)
        || (CTC_PORT_PROP_UNIDIR_EN == port_prop[loop].property)
        || (CTC_PORT_PROP_CL73_ABILITY == port_prop[loop].property)
        || (CTC_PORT_PROP_CHK_CRC_EN == port_prop[loop].property)
        || (CTC_PORT_PROP_STRIP_CRC_EN == port_prop[loop].property)
        || (CTC_PORT_PROP_APPEND_CRC_EN == port_prop[loop].property)
        || (CTC_PORT_PROP_APPEND_TOD_EN == port_prop[loop].property)
        || (CTC_PORT_PROP_AUTO_NEG_FEC == port_prop[loop].property))
        {
            SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
            ret = sys_usw_mac_set_property(lchip, gport, port_prop[loop].property, port_prop[loop].data);
            if(ret && ret != CTC_E_NOT_SUPPORT)
            {
                *array_num = loop;
                return ret;
            }
        }
        else if ((port_prop[loop].property == CTC_PORT_PROP_STATION_MOVE_PRIORITY)
            || (port_prop[loop].property == CTC_PORT_PROP_STATION_MOVE_ACTION))
        {
            CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
            {
                ret = sys_usw_port_set_station_move(lchip, gport, port_prop[loop].property, port_prop[loop].data);
                if (ret && ret != CTC_E_NOT_SUPPORT)
                {
                    *array_num = loop;
                    return ret;
                }
            }
        }
        else
        {
 		    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
		    {
		    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,
					sys_usw_port_set_property(lchip, gport, port_prop[loop].property, port_prop[loop].data));

	        }
			*array_num = loop;
        }
    }

    /*if some properties do not support, sdk will still return error none*/
    return ret;
}

/**
 @brief Get port property

 @param[in] gport, global physical port

 @param[in] port_prop, port property

 @param[in] value

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_property(uint8 lchip, uint32 gport, ctc_port_property_t port_prop, uint32* value)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);

    if ((CTC_PORT_PROP_MAC_EN == port_prop)
        || (CTC_PORT_PROP_LINK_UP == port_prop)
        || (CTC_PORT_PROP_FEC_EN == port_prop)
        || (CTC_PORT_PROP_SIGNAL_DETECT == port_prop)
        || (CTC_PORT_PROP_LINK_INTRRUPT_EN == port_prop)
        || (CTC_PORT_PROP_AUTO_NEG_EN == port_prop)
        || (CTC_PORT_PROP_AUTO_NEG_MODE == port_prop)
        || (CTC_PORT_PROP_SPEED == port_prop)
        || (CTC_PORT_PROP_MAX_FRAME_SIZE == port_prop)
        || (CTC_PORT_PROP_PREAMBLE == port_prop)
        || (CTC_PORT_PROP_MIN_FRAME_SIZE == port_prop)
        || (CTC_PORT_PROP_ERROR_CHECK == port_prop)
        || (CTC_PORT_PROP_PADING_EN == port_prop)
        || (CTC_PORT_PROP_RX_PAUSE_TYPE == port_prop)
        || (CTC_PORT_PROP_LINKSCAN_EN == port_prop)
        || (CTC_PORT_PROP_UNIDIR_EN == port_prop)
        || (CTC_PORT_PROP_MAC_TX_IPG == port_prop)
        || (CTC_PORT_PROP_CL73_ABILITY == port_prop)
        || (CTC_PORT_PROP_CHK_CRC_EN == port_prop)
        || (CTC_PORT_PROP_STRIP_CRC_EN == port_prop)
        || (CTC_PORT_PROP_APPEND_CRC_EN == port_prop)
        || (CTC_PORT_PROP_APPEND_TOD_EN == port_prop)
        || (CTC_PORT_PROP_AUTO_NEG_FEC == port_prop))
    {
        CTC_ERROR_RETURN(sys_usw_mac_get_property(lchip, gport, port_prop, value));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_port_get_property(lchip, gport, port_prop, value));
    }

    return CTC_E_NONE;
}

/**
 @brief Set port property with direction

 @param[in] gport, global physical port

 @param[in] port_prop, port property

 @param[out] value

 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_direction_property(uint8 lchip, uint32 gport, ctc_port_direction_property_t port_prop, ctc_direction_t dir, uint32 value)
{
	int32 ret = CTC_E_NONE;
 	uint8 lchip_start = 0;
	uint8 lchip_end = 0;

	FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
    	CTC_FOREACH_ERROR_RETURN( CTC_E_NOT_SUPPORT, ret,
			sys_usw_port_set_direction_property(lchip, gport, port_prop, dir, value));
    }
    return ret;
}

/**
 @brief Get port property with direction

 @param[in] gport, global physical port

 @param[in] port_prop, port property

 @param[out] value

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_direction_property(uint8 lchip, uint32 gport, ctc_port_direction_property_t port_prop, ctc_direction_t dir, uint32* value)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_port_get_direction_property(lchip, gport, port_prop, dir, value));

    return CTC_E_NONE;
}

/**
 @brief Get port mac link up status

 @param[in] gport, global port of the system

 @param[out] is_up, 0 means not link up, 1 means link up

 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_mac_link_up(uint8 lchip, uint32 gport, bool* is_up)
{
    uint32 link_status = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_get_link_up(lchip, gport, &link_status, 0));
    *is_up = (link_status)?TRUE:FALSE;

    return CTC_E_NONE;
}

/**
 @brief Set port mac entire 48bits

 @param[in] gport, global port of the system
 @param[in] port_mac, port entire 48bits

 @remark  Set port mac extire 48bits. Per port to set, and prefix mac should indicated which to use.
 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_port_mac_postfix(uint8 lchip, uint32 gport, ctc_port_mac_postfix_t* port_mac)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_set_port_mac_postfix(lchip, gport, port_mac));

    return CTC_E_NONE;
}

/**
 @brief Get port mac

 @param[in] gport, global port of the system
 @param[out] port_mac, port mac 48bits

 @remark  Get port mac 48bits.
 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_port_mac(uint8 lchip, uint32 gport, mac_addr_t* port_mac)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_get_port_mac(lchip, gport, port_mac));

    return CTC_E_NONE;
}

/**
 @brief Set port reflective bridge enable

 @param[in] gport, global port of the system
 @param[in] enable, enable port reflective bridge

 @remark  Set port reflective bridge enable, l2 bridge source port and dest port are the same.
 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_reflective_bridge_en(uint8 lchip, uint32 gport, bool enable)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_set_reflective_bridge_en(lchip, gport, enable));

    return CTC_E_NONE;
}

/**
 @brief Get port reflective bridge enable

 @param[in] gport, global port of the system
 @param[out] enable, port reflective bridge enable status

 @remark  Get port reflective bridge enable status, 0. means reflective bridge disable, 1.means reflective bridge disable.
 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_reflective_bridge_en(uint8 lchip, uint32 gport, bool* enable)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_get_reflective_bridge_en(lchip, gport, enable));

    return CTC_E_NONE;
}

int32
ctc_usw_port_set_scl_property(uint8 lchip, uint32 gport, ctc_port_scl_property_t* prop)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_set_scl_property(lchip, gport, prop));

    return CTC_E_NONE;
}

int32
ctc_usw_port_get_scl_property(uint8 lchip, uint32 gport, ctc_port_scl_property_t* prop)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_get_scl_property(lchip, gport, prop));

    return CTC_E_NONE;
}

int32
ctc_usw_set_min_frame_size(uint8 lchip, ctc_frame_size_t index, uint16 value)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_set_min_frame_size(lchip, index, value));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_get_min_frame_size(uint8 lchip, ctc_frame_size_t index, uint16* p_value)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_get_min_frame_size(lchip, index, p_value));

    return CTC_E_NONE;
}

int32
ctc_usw_set_max_frame_size(uint8 lchip, ctc_frame_size_t index, uint16 value)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_set_max_frame_size(lchip, index, value));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_get_max_frame_size(uint8 lchip, ctc_frame_size_t index, uint16* p_value)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(sys_usw_get_max_frame_size(lchip, index, p_value));

    return CTC_E_NONE;
}

int32
ctc_usw_port_set_acl_property(uint8 lchip, uint32 gport, ctc_acl_property_t* p_prop)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_set_acl_property(lchip, gport, p_prop));

    return CTC_E_NONE;
}

int32
ctc_usw_port_get_acl_property(uint8 lchip, uint32 gport, ctc_acl_property_t* p_prop)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_get_acl_property(lchip, gport, p_prop));

    return CTC_E_NONE;
}

int32
ctc_usw_port_set_bpe_property(uint8 lchip, uint32 gport, ctc_port_bpe_property_t* p_prop)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_set_bpe_property(lchip, gport, p_prop));

    return CTC_E_NONE;
}

int32
ctc_usw_port_get_bpe_property(uint8 lchip, uint32 gport, ctc_port_bpe_property_t* p_prop)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_get_bpe_property(lchip, gport, p_prop));

    return CTC_E_NONE;
}


/**
 @brief Set port mac authen enable

 @param[in] gport, global port of the system
 @param[in] enable, enable port mac authen

 @remark  Set port mac authen enable, for src port. Used with CTC_L2_DFT_VLAN_FLAG_PORT_MAC_AUTHEN.
 @return CTC_E_XXX

*/
int32
ctc_usw_port_set_mac_auth(uint8 lchip, uint32 gport, bool enable)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);

    CTC_ERROR_RETURN(sys_usw_port_set_mac_auth(lchip, gport, enable));
    return CTC_E_NONE;
}

/**
 @brief Get port mac authen enable

 @param[in] gport, global port of the system
 @param[out] enable, port mac authen enable

 @remark  Get port mac authen enable, Used with CTC_L2_DFT_VLAN_FLAG_PORT_MAC_AUTHEN.
 @return CTC_E_XXX

*/
int32
ctc_usw_port_get_mac_auth(uint8 lchip, uint32 gport, bool* enable)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);

    CTC_ERROR_RETURN(sys_usw_port_get_mac_auth(lchip, gport, enable));
    return CTC_E_NONE;
}

int32
ctc_usw_port_set_interface_mode(uint8 lchip, uint32 gport, ctc_port_if_mode_t* if_mode)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_mac_set_interface_mode(lchip, gport, if_mode));
    return CTC_E_NONE;
}

int32
ctc_usw_port_get_capability(uint8 lchip, uint32 gport, ctc_port_capability_type_t type, void* p_value)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_PORT);
    LCHIP_CHECK(lchip);
    SYS_MAP_GPORT_TO_LCHIP(gport, lchip);
    CTC_ERROR_RETURN(sys_usw_port_get_capability(lchip, gport, type, p_value));

    return CTC_E_NONE;
}
