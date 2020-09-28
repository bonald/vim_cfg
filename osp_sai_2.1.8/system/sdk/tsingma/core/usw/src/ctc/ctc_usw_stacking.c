#if (FEATURE_MODE == 0)
/**
 @file ctc_usw_stacking.c

 @date 2012-3-14

 @version v2.0
*/

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "ctc_error.h"
#include "ctc_usw_stacking.h"
#include "sys_usw_stacking.h"
#include "sys_usw_common.h"
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

int32
ctc_usw_stacking_create_trunk(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trunk);
    if (p_trunk->extend.enable)
    {
        SYS_MAP_GCHIP_TO_LCHIP(p_trunk->extend.gchip, lchip)
    }

    CTC_ERROR_RETURN(sys_usw_stacking_create_trunk(lchip, p_trunk));

    return CTC_E_NONE;
}

int32
ctc_usw_stacking_destroy_trunk(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trunk);
    if (p_trunk->extend.enable)
    {
        SYS_MAP_GCHIP_TO_LCHIP(p_trunk->extend.gchip, lchip)
    }

    CTC_ERROR_RETURN(sys_usw_stacking_destroy_trunk(lchip, p_trunk));

    return CTC_E_NONE;
}

int32
ctc_usw_stacking_add_trunk_port(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trunk);
    if (p_trunk->extend.enable)
    {
        SYS_MAP_GCHIP_TO_LCHIP(p_trunk->extend.gchip, lchip)
    }

    CTC_ERROR_RETURN(sys_usw_stacking_add_trunk_port(lchip, p_trunk));

    return CTC_E_NONE;
}

int32
ctc_usw_stacking_remove_trunk_port(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trunk);
    if (p_trunk->extend.enable)
    {
        SYS_MAP_GCHIP_TO_LCHIP(p_trunk->extend.gchip, lchip)
    }

    CTC_ERROR_RETURN(sys_usw_stacking_remove_trunk_port(lchip, p_trunk));

    return CTC_E_NONE;
}

int32
ctc_usw_stacking_replace_trunk_ports(uint8 lchip, ctc_stacking_trunk_t* p_trunk, uint32* gports, uint16 mem_ports)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trunk);
    if (p_trunk->extend.enable)
    {
        SYS_MAP_GCHIP_TO_LCHIP(p_trunk->extend.gchip, lchip)
    }

    CTC_ERROR_RETURN(sys_usw_stacking_replace_trunk_ports(lchip, p_trunk->trunk_id, gports, mem_ports));

    return CTC_E_NONE;
}


extern int32
ctc_usw_stacking_get_member_ports(uint8 lchip_id, uint8 trunk_id, uint32* p_gports, uint8* cnt)
{
    uint8 lchip = lchip_id;
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    LCHIP_CHECK(lchip_id);

    CTC_ERROR_RETURN(sys_usw_stacking_get_member_ports(lchip_id, trunk_id, p_gports, cnt));

    return CTC_E_NONE;
}


int32
ctc_usw_stacking_add_trunk_rchip(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trunk);
    if (p_trunk->extend.enable)
    {
        SYS_MAP_GCHIP_TO_LCHIP(p_trunk->extend.gchip, lchip)
    }

    CTC_ERROR_RETURN(sys_usw_stacking_add_trunk_rchip(lchip, p_trunk));

    return CTC_E_NONE;
}

int32
ctc_usw_stacking_remove_trunk_rchip(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trunk);
    if (p_trunk->extend.enable)
    {
        SYS_MAP_GCHIP_TO_LCHIP(p_trunk->extend.gchip, lchip)
    }

    CTC_ERROR_RETURN(sys_usw_stacking_remove_trunk_rchip(lchip, p_trunk));

    return CTC_E_NONE;
}

int32
ctc_usw_stacking_get_trunk_rchip(uint8 lchip, ctc_stacking_trunk_t* p_trunk)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_trunk);
    if (p_trunk->extend.enable)
    {
        SYS_MAP_GCHIP_TO_LCHIP(p_trunk->extend.gchip, lchip)
    }

    CTC_ERROR_RETURN(sys_usw_stacking_get_trunk_rchip(lchip, p_trunk));

    return CTC_E_NONE;
}


int32
ctc_usw_stacking_set_property(uint8 lchip, ctc_stacking_property_t* p_prop)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    LCHIP_CHECK(lchip);

    if (p_prop->extend.enable)
    {
        SYS_MAP_GCHIP_TO_LCHIP(p_prop->extend.gchip, lchip)
    }
    CTC_ERROR_RETURN(sys_usw_stacking_set_property(lchip, p_prop));

    return CTC_E_NONE;
}

int32
ctc_usw_stacking_get_property(uint8 lchip, ctc_stacking_property_t* p_prop)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    LCHIP_CHECK(lchip);
    if (p_prop->extend.enable)
    {
        SYS_MAP_GCHIP_TO_LCHIP(p_prop->extend.gchip, lchip)
    }

    CTC_ERROR_RETURN(sys_usw_stacking_get_property(lchip, p_prop));

    return CTC_E_NONE;
}

int32
ctc_usw_stacking_create_keeplive_group(uint8 lchip, uint16 group_id)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_EXIST,
                                 ret,
                                 sys_usw_stacking_create_keeplive_group(lchip, group_id));

    }
    return ret;
}

int32
ctc_usw_stacking_destroy_keeplive_group(uint8 lchip, uint16 group_id)
{
    uint8 lchip_start              = 0;
    uint8 lchip_end                = 0;
    int32 ret                      = CTC_E_NONE;
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_FOREACH_ERROR_RETURN(CTC_E_NOT_EXIST,
                                 ret,
                                 sys_usw_stacking_destroy_keeplive_group(lchip, group_id));
    }
    return ret;
}

int32
ctc_usw_stacking_keeplive_add_member(uint8 lchip, ctc_stacking_keeplive_t* p_keeplive)
{
	uint8 loop1 = 0;
    int32 ret   = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_keeplive);
    if (p_keeplive->extend.enable)
    {
        SYS_MAP_GCHIP_TO_LCHIP(p_keeplive->extend.gchip, lchip)
    }
	if (p_keeplive->trunk_bmp_en)
    {
        if (p_keeplive->mem_type == CTC_STK_KEEPLIVE_MEMBER_PORT)
        {
            return CTC_E_NOT_SUPPORT;
        }
        for (loop1 = 0; loop1 < CTC_STK_TRUNK_BMP_NUM * 32; loop1++)
        {
            if (!CTC_IS_BIT_SET(p_keeplive->trunk_bitmap[loop1 / 32], loop1 % 32))
            {
                continue;
            }
            p_keeplive->trunk_id = loop1;
            ret = sys_usw_stacking_keeplive_add_member(lchip, p_keeplive);
            if ((ret < 0) && (ret != CTC_E_MEMBER_EXIST))
            {
                goto Error;
            }
        }
    }
	else
	{
		CTC_ERROR_RETURN(sys_usw_stacking_keeplive_add_member(lchip, p_keeplive));
	}
    return CTC_E_NONE;
Error:
    ctc_usw_stacking_keeplive_remove_member(lchip, p_keeplive);
    return ret;
}

int32
ctc_usw_stacking_keeplive_remove_member(uint8 lchip, ctc_stacking_keeplive_t* p_keeplive)
{
	uint8 loop1 = 0;
    int32 ret   = 0;
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_keeplive);
    if (p_keeplive->extend.enable)
    {
        SYS_MAP_GCHIP_TO_LCHIP(p_keeplive->extend.gchip, lchip)
    }

	if (p_keeplive->trunk_bmp_en)
    {
        if (p_keeplive->mem_type == CTC_STK_KEEPLIVE_MEMBER_PORT)
        {
            return CTC_E_NOT_SUPPORT;
        }
        for (loop1 = 0; loop1 < CTC_STK_TRUNK_BMP_NUM * 32; loop1++)
        {
            if (!CTC_IS_BIT_SET(p_keeplive->trunk_bitmap[loop1 / 32], loop1 % 32))
            {
                continue;
            }
            p_keeplive->trunk_id = loop1;
            ret = sys_usw_stacking_keeplive_remove_member(lchip, p_keeplive);
            if ((ret < 0) && (ret != CTC_E_MEMBER_NOT_EXIST))
            {
                return ret;
            }
        }
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_stacking_keeplive_remove_member(lchip, p_keeplive));
    }

    return CTC_E_NONE;
}

int32
ctc_usw_stacking_keeplive_get_members(uint8 lchip, ctc_stacking_keeplive_t* p_keeplive)
{

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_keeplive);
    if (p_keeplive->extend.enable)
    {
        SYS_MAP_GCHIP_TO_LCHIP(p_keeplive->extend.gchip, lchip)
    }

    CTC_ERROR_RETURN(sys_usw_stacking_keeplive_get_members(lchip, p_keeplive));
    return CTC_E_NONE;
}


int32
ctc_usw_stacking_set_trunk_mcast_profile(uint8 lchip, ctc_stacking_trunk_mcast_profile_t* p_mcast_profile)
{
    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_mcast_profile);
    if (p_mcast_profile->extend.enable)
    {
        SYS_MAP_GCHIP_TO_LCHIP(p_mcast_profile->extend.gchip, lchip)
    }
    CTC_ERROR_RETURN(sys_usw_stacking_set_trunk_mcast_profile(lchip, p_mcast_profile));

    return CTC_E_NONE;
}


int32
ctc_usw_stacking_get_trunk_mcast_profile(uint8 lchip, ctc_stacking_trunk_mcast_profile_t* p_mcast_profile)
{
    LCHIP_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_mcast_profile);
    if (p_mcast_profile->extend.enable)
    {
        SYS_MAP_GCHIP_TO_LCHIP(p_mcast_profile->extend.gchip, lchip)
    }
    CTC_ERROR_RETURN(sys_usw_stacking_get_trunk_mcast_profile(lchip, p_mcast_profile));

    return CTC_E_NONE;
}


int32
ctc_usw_stacking_init(uint8 lchip, void* p_cfg)
{
    ctc_stacking_glb_cfg_t stacking_glb_cfg;
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    LCHIP_CHECK(lchip);
    /* init stacking*/
    if (NULL == p_cfg)
    {
        sal_memset(&stacking_glb_cfg, 0 , sizeof(stacking_glb_cfg));
        stacking_glb_cfg.hdr_glb.mac_da_chk_en     = 0;
        stacking_glb_cfg.hdr_glb.ether_type_chk_en = 0;
        stacking_glb_cfg.hdr_glb.vlan_tpid         = 0x8100;
        stacking_glb_cfg.hdr_glb.ether_type        = 0x55bb;
        stacking_glb_cfg.hdr_glb.ip_protocol       = 255;
        stacking_glb_cfg.hdr_glb.udp_dest_port     = 0x1234;
        stacking_glb_cfg.hdr_glb.udp_src_port      = 0x5678;
        stacking_glb_cfg.hdr_glb.udp_en            = 0;
        stacking_glb_cfg.hdr_glb.ip_ttl            = 255;
        stacking_glb_cfg.hdr_glb.ip_dscp           = 63;
        stacking_glb_cfg.hdr_glb.cos               = 7;
        stacking_glb_cfg.hdr_glb.is_ipv4           = 1;
        stacking_glb_cfg.hdr_glb.ipsa.ipv4         = 0x11223344;
        stacking_glb_cfg.fabric_mode               = 0;
        stacking_glb_cfg.version                   = CTC_STACKING_VERSION_1_0;
        p_cfg = &stacking_glb_cfg;
    }

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_stacking_init(lchip, p_cfg));
    }

    return CTC_E_NONE;
}


int32
ctc_usw_stacking_deinit(uint8 lchip)
{
    uint8 lchip_start = 0;
    uint8 lchip_end = 0;

    FEATURE_SUPPORT_CHECK(CTC_FEATURE_STACKING);
    LCHIP_CHECK(lchip);

    CTC_FOREACH_LCHIP(lchip_start, lchip_end, 1)
    {
        CTC_ERROR_RETURN(sys_usw_stacking_deinit(lchip));
    }

    return CTC_E_NONE;
}

#endif

