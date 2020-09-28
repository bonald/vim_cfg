/**
 @file sys_usw_aclqos_policer.c

 @date 2009-10-16

 @version v2.0

*/

/****************************************************************************
  *
  * Header Files
  *
  ****************************************************************************/
#include "ctc_error.h"
#include "ctc_const.h"
#include "ctc_macro.h"
#include "ctc_debug.h"
#include "ctc_warmboot.h"

#include "sys_usw_chip.h"
#include "sys_usw_qos.h"
#include "sys_usw_common.h"

#include "drv_api.h"


/****************************************************************************
  *
  * Defines and Macros
  *
  ****************************************************************************/
#define SYS_QOS_CLASS_COLOR_MAX (MAX_CTC_QOS_COLOR - 1)
#define SYS_QOS_CLASS_COS_MAX 7
#define SYS_QOS_CLASS_CFI_MAX 1
#define SYS_QOS_CLASS_DSCP_MAX 63
#define SYS_QOS_CLASS_ECN_MAX 3
#define SYS_QOS_CLASS_EXP_MAX 7

#define SYS_QOS_CLASS_DBG_OUT(level, FMT, ...) \
    do { \
        CTC_DEBUG_OUT(qos, class, QOS_CLASS_SYS, level, FMT, ##__VA_ARGS__); \
    } while (0);

/****************************************************************************
  *
  * Functions
  *
  ****************************************************************************/

/**
 @brief set cos + cfi -> priority + color mapping table for the given domain
*/
int32
sys_usw_qos_set_igs_cos_map_table(uint8 lchip, uint8 domain,
                                        uint8 cos,
                                        uint8 cfi,
                                        uint8 priority,
                                        uint8 color)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    IpePhbDot1pTaggedMap_m cos_phb_map;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    
    CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_COS_DOMAIN_MAX));
    CTC_MAX_VALUE_CHECK(priority, MCHIP_CAP(SYS_CAP_QOS_CLASS_PRIORITY_MAX));
    CTC_MAX_VALUE_CHECK(color, SYS_QOS_CLASS_COLOR_MAX);
    CTC_MAX_VALUE_CHECK(cos, SYS_QOS_CLASS_COS_MAX);
    CTC_MAX_VALUE_CHECK(cfi, SYS_QOS_CLASS_CFI_MAX);

    sal_memset(&cos_phb_map, 0, sizeof(IpePhbDot1pTaggedMap_m));
        
    /*config IpePhbDot1pTaggedMap*/
    tbl_index = (domain << 4) | (cos << 1) | cfi;

    cmd = DRV_IOR(IpePhbDot1pTaggedMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &cos_phb_map));
    
    SetIpePhbDot1pTaggedMap(V, prio_f, &cos_phb_map, priority);
    SetIpePhbDot1pTaggedMap(V, color_f, &cos_phb_map, color);
    cmd = DRV_IOW(IpePhbDot1pTaggedMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &cos_phb_map));
        
    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "cos + cfi -> priority + color map, domain = %d, cos = %d, cfi = %d, priority = %d, color = %d\n",
                           domain, cos, cfi, priority, color);

    return CTC_E_NONE;
}

/**
 @brief get cos -> priority + color mapping table for the given domain
*/
int32
sys_usw_qos_get_igs_cos_map_table(uint8 lchip, uint8 domain,
                                        uint8 cos,
                                        uint8 cfi,
                                        uint8* priority,
                                        uint8* color)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    IpePhbDot1pTaggedMap_m cos_phb_map;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_COS_DOMAIN_MAX));
    CTC_MAX_VALUE_CHECK(cos, SYS_QOS_CLASS_COS_MAX);
    CTC_MAX_VALUE_CHECK(cfi, SYS_QOS_CLASS_CFI_MAX);

    sal_memset(&cos_phb_map, 0, sizeof(IpePhbDot1pTaggedMap_m));
    
    tbl_index = (domain << 4) | (cos << 1) | cfi;
    
    cmd = DRV_IOR(IpePhbDot1pTaggedMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &cos_phb_map));
    
    *priority = GetIpePhbDot1pTaggedMap(V, prio_f, &cos_phb_map);
    *color = GetIpePhbDot1pTaggedMap(V, color_f, &cos_phb_map);
    
    return CTC_E_NONE;
}

/**
 @brief set dscp -> priority + color mapping table for the given domain
*/
int32
sys_usw_qos_set_igs_dscp_map_table(uint8 lchip, uint8 domain,
                                         uint8 dscp,
                                         uint8 priority,
                                         uint8 color)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    IpePhbDscpMap_m dscp_phb_map;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_DSCP_DOMAIN_MAX));
    CTC_MAX_VALUE_CHECK(dscp, SYS_QOS_CLASS_DSCP_MAX);
    CTC_MAX_VALUE_CHECK(priority, MCHIP_CAP(SYS_CAP_QOS_CLASS_PRIORITY_MAX));
    CTC_MAX_VALUE_CHECK(color, SYS_QOS_CLASS_COLOR_MAX);

    sal_memset(&dscp_phb_map, 0, sizeof(IpePhbDscpMap_m));

    tbl_index = (domain << 6) | dscp;

    cmd = DRV_IOR(IpePhbDscpMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &dscp_phb_map));
    
    SetIpePhbDscpMap(V, prio_f, &dscp_phb_map, priority);
    SetIpePhbDscpMap(V, color_f, &dscp_phb_map, color);
    SetIpePhbDscpMap(V, newDscp_f, &dscp_phb_map, dscp);
    cmd = DRV_IOW(IpePhbDscpMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &dscp_phb_map));

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "dscp -> priority + color map, domain = %d, dscp = %d, priority = %d, color = %d\n",
                           domain, dscp, priority, color);

    return CTC_E_NONE;
}

/**
 @brief get dscp -> priority + color mapping table for the given domain
*/
int32
sys_usw_qos_get_igs_dscp_map_table(uint8 lchip, uint8 domain,
                                         uint8 dscp,
                                         uint8* priority,
                                         uint8* color)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    IpePhbDscpMap_m dscp_phb_map;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_DSCP_DOMAIN_MAX));
    CTC_MAX_VALUE_CHECK(dscp, SYS_QOS_CLASS_DSCP_MAX);

    sal_memset(&dscp_phb_map, 0, sizeof(IpePhbDscpMap_m));

    tbl_index = (domain << 6) | dscp;

    cmd = DRV_IOR(IpePhbDscpMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &dscp_phb_map));
    
    *priority = GetIpePhbDscpMap(V, prio_f, &dscp_phb_map);
    *color = GetIpePhbDscpMap(V, color_f, &dscp_phb_map);

    return CTC_E_NONE;
}

/**
 @brief set mpls exp -> priority + color mapping table for the given domain
*/
int32
sys_usw_qos_set_igs_exp_map_table(uint8 lchip, uint8 domain,
                                        uint8 exp,
                                        uint8 priority,
                                        uint8 color)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    IpePhbTcMap_m exp_phb_map;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_EXP_DOMAIN_MAX));
    CTC_MAX_VALUE_CHECK(exp, SYS_QOS_CLASS_EXP_MAX);
    CTC_MAX_VALUE_CHECK(priority, MCHIP_CAP(SYS_CAP_QOS_CLASS_PRIORITY_MAX));
    CTC_MAX_VALUE_CHECK(color, SYS_QOS_CLASS_COLOR_MAX);

    tbl_index = (domain << 3) | exp;

    sal_memset(&exp_phb_map, 0, sizeof(IpePhbTcMap_m));

    cmd = DRV_IOR(IpePhbTcMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &exp_phb_map));
    
    SetIpePhbTcMap(V, prio_f, &exp_phb_map, priority);
    SetIpePhbTcMap(V, color_f, &exp_phb_map, color);
    SetIpePhbTcMap(V, newLabelTc_f, &exp_phb_map, exp);
    cmd = DRV_IOW(IpePhbTcMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &exp_phb_map));

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "exp -> priority + color map, domain = %d, exp = %d, priority = %d, color = %d\n",
                           domain, exp, priority, color);

    return CTC_E_NONE;
}

/**
 @brief get mpls exp -> priority + color mapping table for the given domain
*/
int32
sys_usw_qos_get_igs_exp_map_table(uint8 lchip, uint8 domain,
                                        uint8 exp,
                                        uint8* priority,
                                        uint8* color)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    IpePhbTcMap_m exp_phb_map;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_EXP_DOMAIN_MAX));
    CTC_MAX_VALUE_CHECK(exp, SYS_QOS_CLASS_EXP_MAX);

    tbl_index = (domain << 3) | exp;

    sal_memset(&exp_phb_map, 0, sizeof(IpePhbTcMap_m));

    cmd = DRV_IOR(IpePhbTcMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &exp_phb_map));
    
    *priority = GetIpePhbTcMap(V, prio_f, &exp_phb_map);
    *color = GetIpePhbTcMap(V, color_f, &exp_phb_map);

    return CTC_E_NONE;
}

/**
 @brief set priority + color -> cos mapping table for the given domain
*/
int32
sys_usw_qos_set_egs_cos_map_table(uint8 lchip, uint8 domain,
                                        uint8 priority,
                                        uint8 color,
                                        uint8 cos,
                                        uint8 cfi)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    EpePhbDot1pTaggedMap0_m cos_phb_map0;
    EpePhbDot1pTaggedMap1_m cos_phb_map1;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_COS_DOMAIN_MAX));
    CTC_MAX_VALUE_CHECK(cos, SYS_QOS_CLASS_COS_MAX);
    CTC_MAX_VALUE_CHECK(cfi, SYS_QOS_CLASS_CFI_MAX);
    CTC_MAX_VALUE_CHECK(priority, MCHIP_CAP(SYS_CAP_QOS_CLASS_PRIORITY_MAX));
    CTC_MAX_VALUE_CHECK(color, SYS_QOS_CLASS_COLOR_MAX);

    sal_memset(&cos_phb_map0, 0, sizeof(EpePhbDot1pTaggedMap0_m));
    sal_memset(&cos_phb_map1, 0, sizeof(EpePhbDot1pTaggedMap1_m));

    /*config EpePhbDot1pTaggedMap0*/
    tbl_index = (domain << 4) | priority;

    cmd = DRV_IOR(EpePhbDot1pTaggedMap0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &cos_phb_map0));

    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_RED == color))
    {
        SetEpePhbDot1pTaggedMap0(V, mappedCosR_f, &cos_phb_map0, cos);
        SetEpePhbDot1pTaggedMap0(V, mappedCfiR_f, &cos_phb_map0, cfi);
    }
    
    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_YELLOW == color))
    {
        SetEpePhbDot1pTaggedMap0(V, mappedCosY_f, &cos_phb_map0, cos);
        SetEpePhbDot1pTaggedMap0(V, mappedCfiY_f, &cos_phb_map0, cfi);
    }
    
    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_GREEN == color))
    {
        SetEpePhbDot1pTaggedMap0(V, mappedCosG_f, &cos_phb_map0, cos);
        SetEpePhbDot1pTaggedMap0(V, mappedCfiG_f, &cos_phb_map0, cfi);
    }

    cmd = DRV_IOW(EpePhbDot1pTaggedMap0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &cos_phb_map0));

    /*config EpePhbDot1pTaggedMap1*/
    cmd = DRV_IOR(EpePhbDot1pTaggedMap1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &cos_phb_map1));

    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_RED == color))
    {
        SetEpePhbDot1pTaggedMap1(V, mappedCosR_f, &cos_phb_map1, cos);
        SetEpePhbDot1pTaggedMap1(V, mappedCfiR_f, &cos_phb_map1, cfi);
    }
    
    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_YELLOW == color))
    {
        SetEpePhbDot1pTaggedMap1(V, mappedCosY_f, &cos_phb_map1, cos);
        SetEpePhbDot1pTaggedMap1(V, mappedCfiY_f, &cos_phb_map1, cfi);
    }
    
    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_GREEN == color))
    {
        SetEpePhbDot1pTaggedMap1(V, mappedCosG_f, &cos_phb_map1, cos);
        SetEpePhbDot1pTaggedMap1(V, mappedCfiG_f, &cos_phb_map1, cfi);
    }

    cmd = DRV_IOW(EpePhbDot1pTaggedMap1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &cos_phb_map1));

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "priority + color -> cos map, domain = %d, priority = %d, color = %d, cos = %d, cfi = %d\n",
                           domain, priority, color, cos, cfi);

    return CTC_E_NONE;
}

/**
 @brief get priority + color -> cos mapping table for the given domain
*/
int32
sys_usw_qos_get_egs_cos_map_table(uint8 lchip, uint8 domain,
                                        uint8 priority,
                                        uint8 color,
                                        uint8* cos,
                                        uint8* cfi)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    EpePhbDot1pTaggedMap0_m cos_phb_map0;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_COS_DOMAIN_MAX));
    CTC_MAX_VALUE_CHECK(priority, MCHIP_CAP(SYS_CAP_QOS_CLASS_PRIORITY_MAX));
    CTC_MAX_VALUE_CHECK(color, SYS_QOS_CLASS_COLOR_MAX);

    sal_memset(&cos_phb_map0, 0, sizeof(EpePhbDot1pTaggedMap0_m));

    tbl_index = (domain << 4) | priority;

    cmd = DRV_IOR(EpePhbDot1pTaggedMap0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &cos_phb_map0));

    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_RED == color))
    {
        *cos = GetEpePhbDot1pTaggedMap0(V, mappedCosR_f, &cos_phb_map0);
        *cfi = GetEpePhbDot1pTaggedMap0(V, mappedCfiR_f, &cos_phb_map0);
    }
    else if (CTC_QOS_COLOR_YELLOW == color)
    {
        *cos = GetEpePhbDot1pTaggedMap0(V, mappedCosY_f, &cos_phb_map0);
        *cfi = GetEpePhbDot1pTaggedMap0(V, mappedCfiY_f, &cos_phb_map0);
    }
    else if (CTC_QOS_COLOR_GREEN == color)
    {
        *cos = GetEpePhbDot1pTaggedMap0(V, mappedCosG_f, &cos_phb_map0);
        *cfi = GetEpePhbDot1pTaggedMap0(V, mappedCfiG_f, &cos_phb_map0);
    }

    return CTC_E_NONE;
}

/**
 @brief set priority + color -> dscp mapping table for the given domain
*/
int32
sys_usw_qos_set_egs_dscp_map_table(uint8 lchip, uint8 domain,
                                         uint8 priority,
                                         uint8 color,
                                         uint8 dscp)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    EpePhbDscpMap0_m dscp_phb_map0;
    EpePhbDscpMap1_m dscp_phb_map1;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_DSCP_DOMAIN_MAX));
    CTC_MAX_VALUE_CHECK(priority, MCHIP_CAP(SYS_CAP_QOS_CLASS_PRIORITY_MAX));
    CTC_MAX_VALUE_CHECK(color, SYS_QOS_CLASS_COLOR_MAX);
    CTC_MAX_VALUE_CHECK(dscp, SYS_QOS_CLASS_DSCP_MAX);

    sal_memset(&dscp_phb_map0, 0, sizeof(EpePhbDscpMap0_m));
    sal_memset(&dscp_phb_map1, 0, sizeof(EpePhbDscpMap1_m));

    tbl_index = (domain << 4) | priority;

    /*config EpePhbDscpMap0*/
    cmd = DRV_IOR(EpePhbDscpMap0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &dscp_phb_map0));

    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_RED == color))
    {
        SetEpePhbDscpMap0(V, mappedDscpR_f, &dscp_phb_map0, dscp);
    }
    
    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_YELLOW == color))
    {
        SetEpePhbDscpMap0(V, mappedDscpY_f, &dscp_phb_map0, dscp);
    }
    
    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_GREEN == color))
    {
        SetEpePhbDscpMap0(V, mappedDscpG_f, &dscp_phb_map0, dscp);
    }

    cmd = DRV_IOW(EpePhbDscpMap0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &dscp_phb_map0));

    /*config EpePhbDscpMap1*/
    cmd = DRV_IOR(EpePhbDscpMap1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &dscp_phb_map1));

    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_RED == color))
    {
        SetEpePhbDscpMap1(V, mappedDscpR_f, &dscp_phb_map1, dscp);
    }
    
    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_YELLOW == color))
    {
        SetEpePhbDscpMap1(V, mappedDscpY_f, &dscp_phb_map1, dscp);
    }
    
    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_GREEN == color))
    {
        SetEpePhbDscpMap1(V, mappedDscpG_f, &dscp_phb_map1, dscp);
    }

    cmd = DRV_IOW(EpePhbDscpMap1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &dscp_phb_map1));

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "priority + color -> dscp map, domain = %d, priority = %d, color = %d, dscp = %d\n",
                           domain, priority, color, dscp);

    return CTC_E_NONE;
}

/**
 @brief get priority + color -> dscp mapping table for the given domain
*/
int32
sys_usw_qos_get_egs_dscp_map_table(uint8 lchip, uint8 domain,
                                         uint8 priority,
                                         uint8 color,
                                         uint8* dscp)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    EpePhbDscpMap0_m dscp_phb_map0;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_DSCP_DOMAIN_MAX));
    CTC_MAX_VALUE_CHECK(priority, MCHIP_CAP(SYS_CAP_QOS_CLASS_PRIORITY_MAX));
    CTC_MAX_VALUE_CHECK(color, SYS_QOS_CLASS_COLOR_MAX);

    sal_memset(&dscp_phb_map0, 0, sizeof(EpePhbDscpMap0_m));

    tbl_index = (domain << 4) | priority;

    cmd = DRV_IOR(EpePhbDscpMap0_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &dscp_phb_map0));

    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_RED == color))
    {
        *dscp = GetEpePhbDscpMap0(V, mappedDscpR_f, &dscp_phb_map0);
    }
    else if (CTC_QOS_COLOR_YELLOW == color)
    {
        *dscp = GetEpePhbDscpMap0(V, mappedDscpY_f, &dscp_phb_map0);
    }
    else if (CTC_QOS_COLOR_GREEN == color)
    {
        *dscp = GetEpePhbDscpMap0(V, mappedDscpG_f, &dscp_phb_map0);
    }

    return CTC_E_NONE;
}

/**
 @brief set priority + color -> mpls exp mapping table for the given domain
*/
int32
sys_usw_qos_set_egs_exp_map_table(uint8 lchip, uint8 domain,
                                        uint8 priority,
                                        uint8 color,
                                        uint8 exp)
{
    uint32 tbl_index = 0;
    uint32 cmd       = 0;
    EpePhbTcMap0_m exp_phb_map0;
    EpePhbTcMap1_m exp_phb_map1;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_EXP_DOMAIN_MAX));
    CTC_MAX_VALUE_CHECK(priority, MCHIP_CAP(SYS_CAP_QOS_CLASS_PRIORITY_MAX));
    CTC_MAX_VALUE_CHECK(color, SYS_QOS_CLASS_COLOR_MAX);
    CTC_MAX_VALUE_CHECK(exp, SYS_QOS_CLASS_EXP_MAX);

    tbl_index = (domain << 4) | priority;

    sal_memset(&exp_phb_map0, 0, sizeof(EpePhbTcMap0_m));
    sal_memset(&exp_phb_map1, 0, sizeof(EpePhbTcMap1_m));

    /*config EpePhbDscpMap0*/
    if (0 == domain)
    {
        cmd = DRV_IOR(EpePhbTcMap0_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &exp_phb_map0));

        if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_RED == color))
        {
            SetEpePhbTcMap0(V, mappedMplsTcR_f, &exp_phb_map0, exp);
        }
        
        if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_YELLOW == color))
        {
            SetEpePhbTcMap0(V, mappedMplsTcY_f, &exp_phb_map0, exp);
        }
        
        if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_GREEN == color))
        {
            SetEpePhbTcMap0(V, mappedMplsTcG_f, &exp_phb_map0, exp);
        }

        cmd = DRV_IOW(EpePhbTcMap0_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &exp_phb_map0));
    }

    /*config EpePhbDscpMap1*/
    cmd = DRV_IOR(EpePhbTcMap1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &exp_phb_map1));

    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_RED == color))
    {
        SetEpePhbTcMap1(V, mappedMplsTcR_f, &exp_phb_map1, exp);
    }
    
    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_YELLOW == color))
    {
        SetEpePhbTcMap1(V, mappedMplsTcY_f, &exp_phb_map1, exp);
    }
    
    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_GREEN == color))
    {
        SetEpePhbTcMap1(V, mappedMplsTcG_f, &exp_phb_map1, exp);
    }

    cmd = DRV_IOW(EpePhbTcMap1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &exp_phb_map1));

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "priority + color -> exp map, domain = %d, priority = %d, color = %d, exp = %d\n",
                           domain, priority, color, exp);

    return CTC_E_NONE;
}

/**
 @brief get priority + color -> mpls exp mapping table for the given domain
*/
int32
sys_usw_qos_get_egs_exp_map_table(uint8 lchip, uint8 domain,
                                        uint8 priority,
                                        uint8 color,
                                        uint8* exp)
{
    uint32 tbl_index = 0;
    uint32 cmd       = 0;
    EpePhbTcMap1_m exp_phb_map1;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_EXP_DOMAIN_MAX));
    CTC_MAX_VALUE_CHECK(priority, MCHIP_CAP(SYS_CAP_QOS_CLASS_PRIORITY_MAX));
    CTC_MAX_VALUE_CHECK(color, SYS_QOS_CLASS_COLOR_MAX);

    tbl_index = (domain << 4) | priority;

    cmd = DRV_IOR(EpePhbTcMap1_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &exp_phb_map1));

    if ((CTC_QOS_COLOR_NONE == color) || (CTC_QOS_COLOR_RED == color))
    {
        *exp = GetEpePhbTcMap1(V, mappedMplsTcR_f, &exp_phb_map1);
    }
    else if (CTC_QOS_COLOR_YELLOW == color)
    {
        *exp = GetEpePhbTcMap1(V, mappedMplsTcY_f, &exp_phb_map1);
    }
    else if (CTC_QOS_COLOR_GREEN == color)
    {
        *exp = GetEpePhbTcMap1(V, mappedMplsTcG_f, &exp_phb_map1);
    }

    return CTC_E_NONE;
}

/**
 @brief set cos -> cos mapping table for the given table_map_id
*/
int32
sys_usw_qos_set_igs_cos2cos_map_table(uint8 lchip, uint8 table_map_id, uint8 cos_in,
                                                         uint8 cos_out, uint8 action_type)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    DsIpePhbMutationCosMap_m mutation_cos_map;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(table_map_id, MCHIP_CAP(SYS_CAP_QOS_CLASS_TABLE_MAP_ID_MAX));
    CTC_MAX_VALUE_CHECK(cos_in, SYS_QOS_CLASS_COS_MAX);
    CTC_MAX_VALUE_CHECK(cos_out, SYS_QOS_CLASS_COS_MAX);
    CTC_MAX_VALUE_CHECK(action_type, CTC_QOS_TABLE_MAP_ACTION_MAX - 1);

    sal_memset(&mutation_cos_map, 0, sizeof(DsIpePhbMutationCosMap_m));

    /*config DsIpePhbMutationCosMap*/
    tbl_index = (table_map_id << 3) | cos_in;

    cmd = DRV_IOR(DsIpePhbMutationCosMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &mutation_cos_map));
    
    SetDsIpePhbMutationCosMap(V, toCos_newCosR_f, &mutation_cos_map, cos_out);     
    SetDsIpePhbMutationCosMap(V, toCos_newCosY_f, &mutation_cos_map, cos_out); 
    SetDsIpePhbMutationCosMap(V, toCos_newCosG_f, &mutation_cos_map, cos_out);   
    SetDsIpePhbMutationCosMap(V, toCos_mtActionType_f, &mutation_cos_map, action_type);
    cmd = DRV_IOW(DsIpePhbMutationCosMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &mutation_cos_map));

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "cos -> cos map, table_map_id = %d, cos_in = %d, cos_out = %d, mutation_action_type = %d\n",
                           table_map_id, cos_in, cos_out, action_type);

    return CTC_E_NONE;
}

/**
 @brief get cos-> cos mapping table for the given table_map_id
*/
int32
sys_usw_qos_get_igs_cos2cos_map_table(uint8 lchip, uint8 table_map_id, uint8 cos_in, 
                                                         uint8* cos_out, uint8* action_type)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    DsIpePhbMutationCosMap_m mutation_cos_map;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(table_map_id, MCHIP_CAP(SYS_CAP_QOS_CLASS_TABLE_MAP_ID_MAX));
    CTC_MAX_VALUE_CHECK(cos_in, SYS_QOS_CLASS_COS_MAX);
    
    sal_memset(&mutation_cos_map, 0, sizeof(DsIpePhbMutationCosMap_m));

    /*config DsIpePhbMutationCosMap*/
    tbl_index = (table_map_id << 3) | cos_in;

    cmd = DRV_IOR(DsIpePhbMutationCosMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &mutation_cos_map));
    
    *cos_out = GetDsIpePhbMutationCosMap(V, toCos_newCosG_f, &mutation_cos_map);   
    *action_type = GetDsIpePhbMutationCosMap(V, toCos_mtActionType_f, &mutation_cos_map);
    
    return CTC_E_NONE;
}

/**
 @brief set cos -> dscp mapping table for the given table_map_id
*/
int32
sys_usw_qos_set_igs_cos2dscp_map_table(uint8 lchip, uint8 table_map_id, uint8 cos_in,
                                                           uint8 dscp_out, uint8 action_type)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    DsIpePhbMutationCosMap_m mutation_cos_map;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(table_map_id, MCHIP_CAP(SYS_CAP_QOS_CLASS_TABLE_MAP_ID_MAX));
    CTC_MAX_VALUE_CHECK(cos_in, SYS_QOS_CLASS_COS_MAX);
    CTC_MAX_VALUE_CHECK(dscp_out, SYS_QOS_CLASS_DSCP_MAX);
    CTC_MAX_VALUE_CHECK(action_type, CTC_QOS_TABLE_MAP_ACTION_MAX - 1);

    sal_memset(&mutation_cos_map, 0, sizeof(DsIpePhbMutationCosMap_m));

    /*config DsIpePhbMutationCosMap*/
    tbl_index = (table_map_id << 3) | cos_in;

    cmd = DRV_IOR(DsIpePhbMutationCosMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &mutation_cos_map));
    
    SetDsIpePhbMutationCosMap(V, toDscp_newDscpR_f, &mutation_cos_map, dscp_out);
    SetDsIpePhbMutationCosMap(V, toDscp_newDscpY_f, &mutation_cos_map, dscp_out);
    SetDsIpePhbMutationCosMap(V, toDscp_newDscpG_f, &mutation_cos_map, dscp_out);   
    SetDsIpePhbMutationCosMap(V, toDscp_mtActionType_f, &mutation_cos_map, action_type);
    cmd = DRV_IOW(DsIpePhbMutationCosMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &mutation_cos_map));

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "cos -> dscp map, table_map_id = %d, cos_in = %d, dscp_out = %d, mutation_action_type = %d\n",
                           table_map_id, cos_in, dscp_out, action_type);

    return CTC_E_NONE;
}

/**
 @brief get cos-> dscp mapping table for the given table_map_id
*/
int32
sys_usw_qos_get_igs_cos2dscp_map_table(uint8 lchip, uint8 table_map_id, uint8 cos_in, 
                                                           uint8* dscp_out, uint8* action_type)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    DsIpePhbMutationCosMap_m mutation_cos_map;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(table_map_id, MCHIP_CAP(SYS_CAP_QOS_CLASS_TABLE_MAP_ID_MAX));
    CTC_MAX_VALUE_CHECK(cos_in, SYS_QOS_CLASS_COS_MAX);

    sal_memset(&mutation_cos_map, 0, sizeof(DsIpePhbMutationCosMap_m));

    /*get DsIpePhbMutationCosMap*/
    tbl_index = (table_map_id << 3) | cos_in;

    cmd = DRV_IOR(DsIpePhbMutationCosMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &mutation_cos_map));
    
    *dscp_out = GetDsIpePhbMutationCosMap(V, toDscp_newDscpG_f, &mutation_cos_map);   
    *action_type = GetDsIpePhbMutationCosMap(V, toDscp_mtActionType_f, &mutation_cos_map);
    
    return CTC_E_NONE;
}

/**
 @brief set dscp -> cos mapping table for the given table_map_id
*/
int32
sys_usw_qos_set_igs_dscp2cos_map_table(uint8 lchip, uint8 table_map_id, uint8 dscp_in,
                                                           uint8 cos_out, uint8 action_type)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    DsIpePhbMutationDscpMap_m mutation_dscp_map;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(table_map_id, MCHIP_CAP(SYS_CAP_QOS_CLASS_TABLE_MAP_ID_MAX));
    CTC_MAX_VALUE_CHECK(cos_out, SYS_QOS_CLASS_COS_MAX);
    CTC_MAX_VALUE_CHECK(dscp_in, SYS_QOS_CLASS_DSCP_MAX);
    CTC_MAX_VALUE_CHECK(action_type, CTC_QOS_TABLE_MAP_ACTION_MAX - 1);

    sal_memset(&mutation_dscp_map, 0, sizeof(DsIpePhbMutationDscpMap_m));

    /*config DsIpePhbMutationDscpMap*/
    tbl_index = (table_map_id << 6) | dscp_in;

    cmd = DRV_IOR(DsIpePhbMutationDscpMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &mutation_dscp_map));
    
    SetDsIpePhbMutationDscpMap(V, toCos_newCosR_f, &mutation_dscp_map, cos_out);
    SetDsIpePhbMutationDscpMap(V, toCos_newCosY_f, &mutation_dscp_map, cos_out);
    SetDsIpePhbMutationDscpMap(V, toCos_newCosG_f, &mutation_dscp_map, cos_out);    
    SetDsIpePhbMutationDscpMap(V, toCos_mtActionType_f, &mutation_dscp_map, action_type);
    cmd = DRV_IOW(DsIpePhbMutationDscpMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &mutation_dscp_map));

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "dscp -> cos map, table_map_id = %d, dscp_in = %d, cos_out = %d, mutation_action_type = %d\n",
                           table_map_id, dscp_in, cos_out, action_type);

    return CTC_E_NONE;
}

/**
 @brief get dscp -> cos mapping table for the given table_map_id
*/
int32
sys_usw_qos_get_igs_dscp2cos_map_table(uint8 lchip, uint8 table_map_id, uint8 dscp_in,
                                                           uint8* cos_out, uint8* action_type)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    DsIpePhbMutationDscpMap_m mutation_dscp_map;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(table_map_id, MCHIP_CAP(SYS_CAP_QOS_CLASS_TABLE_MAP_ID_MAX));
    CTC_MAX_VALUE_CHECK(dscp_in, SYS_QOS_CLASS_DSCP_MAX);

    sal_memset(&mutation_dscp_map, 0, sizeof(DsIpePhbMutationDscpMap_m));

    /*get DsIpePhbMutationDscpMap*/
    tbl_index = (table_map_id << 6) | dscp_in;

    cmd = DRV_IOR(DsIpePhbMutationDscpMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &mutation_dscp_map));
    
    *cos_out = GetDsIpePhbMutationDscpMap(V, toCos_newCosG_f, &mutation_dscp_map);   
    *action_type = GetDsIpePhbMutationDscpMap(V, toCos_mtActionType_f, &mutation_dscp_map);
    
    return CTC_E_NONE;
}


/**
 @brief set dscp -> dscp mapping table for the given table_map_id
*/
int32
sys_usw_qos_set_igs_dscp2dscp_map_table(uint8 lchip, uint8 table_map_id, uint8 dscp_in,
                                                            uint8 dscp_out, uint8 action_type)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    DsIpePhbMutationDscpMap_m mutation_dscp_map;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(table_map_id, MCHIP_CAP(SYS_CAP_QOS_CLASS_TABLE_MAP_ID_MAX));
    CTC_MAX_VALUE_CHECK(dscp_in, SYS_QOS_CLASS_DSCP_MAX);
    CTC_MAX_VALUE_CHECK(dscp_out, SYS_QOS_CLASS_DSCP_MAX);
    CTC_MAX_VALUE_CHECK(action_type, CTC_QOS_TABLE_MAP_ACTION_MAX - 1);

    sal_memset(&mutation_dscp_map, 0, sizeof(DsIpePhbMutationDscpMap_m));

    /*config DsIpePhbMutationCosMap*/
    tbl_index = (table_map_id << 6) | dscp_in;

    cmd = DRV_IOR(DsIpePhbMutationDscpMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &mutation_dscp_map)); 
    
    SetDsIpePhbMutationDscpMap(V, toDscp_newDscpR_f, &mutation_dscp_map, dscp_out);
    SetDsIpePhbMutationDscpMap(V, toDscp_newDscpY_f, &mutation_dscp_map, dscp_out);
    SetDsIpePhbMutationDscpMap(V, toDscp_newDscpG_f, &mutation_dscp_map, dscp_out);  
    SetDsIpePhbMutationDscpMap(V, toDscp_mtActionType_f, &mutation_dscp_map, action_type);
    cmd = DRV_IOW(DsIpePhbMutationDscpMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &mutation_dscp_map));

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "dscp -> dscp map, table_map_id = %d, dscp_in = %d, dscp_out = %d, mutation_action_type = %d\n",
                           table_map_id, dscp_in, dscp_out, action_type);

    return CTC_E_NONE;
}

/**
 @brief get dscp -> dscp mapping table for the given table_map_id
*/
int32
sys_usw_qos_get_igs_dscp2dscp_map_table(uint8 lchip, uint8 table_map_id, uint8 dscp_in, 
                                                             uint8* dscp_out, uint8* action_type)
{
    uint32 tbl_index         = 0;
    uint32 cmd               = 0;
    DsIpePhbMutationDscpMap_m mutation_dscp_map;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_MAX_VALUE_CHECK(table_map_id, MCHIP_CAP(SYS_CAP_QOS_CLASS_TABLE_MAP_ID_MAX));
    CTC_MAX_VALUE_CHECK(dscp_in, SYS_QOS_CLASS_DSCP_MAX);

    sal_memset(&mutation_dscp_map, 0, sizeof(DsIpePhbMutationDscpMap_m));

    /*get DsIpePhbMutationDscpMap*/
    tbl_index = (table_map_id << 6) | dscp_in;

    cmd = DRV_IOR(DsIpePhbMutationDscpMap_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, tbl_index, cmd, &mutation_dscp_map));
    
    *dscp_out = GetDsIpePhbMutationDscpMap(V, toDscp_newDscpG_f, &mutation_dscp_map);  
    *action_type = GetDsIpePhbMutationDscpMap(V, toDscp_mtActionType_f, &mutation_dscp_map);
    
    return CTC_E_NONE;
}

extern int32
sys_usw_qos_domain_map_set(uint8 lchip, ctc_qos_domain_map_t* p_domain_map)
{
    uint8 domain   = 0;
    uint8 cos      = 0;
    uint8 dscp     = 0;
    uint8 exp      = 0;
    uint8 cfi      = 0;
    uint8 priority = 0;
    uint8 color    = 0;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    domain   = p_domain_map->domain_id;
    priority = p_domain_map->priority;
    color    = p_domain_map->color;

    switch (p_domain_map->type)
    {
    case CTC_QOS_DOMAIN_MAP_IGS_COS_TO_PRI_COLOR:
        cos = p_domain_map->hdr_pri.dot1p.cos;
        cfi = p_domain_map->hdr_pri.dot1p.dei;
        CTC_ERROR_RETURN(sys_usw_qos_set_igs_cos_map_table(lchip, domain,
                                                                 cos,
                                                                 cfi,
                                                                 priority,
                                                                 color));
        break;

    case CTC_QOS_DOMAIN_MAP_IGS_DSCP_TO_PRI_COLOR:
        dscp = p_domain_map->hdr_pri.tos.dscp;
        CTC_ERROR_RETURN(sys_usw_qos_set_igs_dscp_map_table(lchip, domain,
                                                                  dscp,
                                                                  priority,
                                                                  color));
        break;

    case CTC_QOS_DOMAIN_MAP_IGS_IP_PREC_TO_PRI_COLOR:
        return CTC_E_NOT_SUPPORT;
        break;

    case CTC_QOS_DOMAIN_MAP_IGS_EXP_TO_PRI_COLOR:
        exp = p_domain_map->hdr_pri.exp;
        CTC_ERROR_RETURN(sys_usw_qos_set_igs_exp_map_table(lchip, domain,
                                                                 exp,
                                                                 priority,
                                                                 color));
        break;

    case CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_COS:
        cos = p_domain_map->hdr_pri.dot1p.cos;
        cfi = p_domain_map->hdr_pri.dot1p.dei;
        CTC_ERROR_RETURN(sys_usw_qos_set_egs_cos_map_table(lchip, domain,
                                                                 priority,
                                                                 color,
                                                                 cos,
                                                                 cfi));
        break;

    case CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_DSCP:
        dscp = p_domain_map->hdr_pri.tos.dscp;
        CTC_ERROR_RETURN(sys_usw_qos_set_egs_dscp_map_table(lchip, domain,
                                                                  priority,
                                                                  color,
                                                                  dscp));
        break;

    case CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_EXP:
        exp = p_domain_map->hdr_pri.exp;
        CTC_ERROR_RETURN(sys_usw_qos_set_egs_exp_map_table(lchip, domain,
                                                                 priority,
                                                                 color,
                                                                 exp));

        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_domain_map_get(uint8 lchip, ctc_qos_domain_map_t* p_domain_map)
{
    uint8 domain   = 0;
    uint8 cos      = 0;
    uint8 dscp     = 0;
    uint8 exp      = 0;
    uint8 cfi      = 0;
    uint8 priority = 0;
    uint8 color    = 0;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    domain = p_domain_map->domain_id;

    switch (p_domain_map->type)
    {
    case CTC_QOS_DOMAIN_MAP_IGS_COS_TO_PRI_COLOR:
        cos = p_domain_map->hdr_pri.dot1p.cos;
        cfi = p_domain_map->hdr_pri.dot1p.dei;
        CTC_ERROR_RETURN(sys_usw_qos_get_igs_cos_map_table(lchip, domain,
                                                                 cos,
                                                                 cfi,
                                                                 &priority,
                                                                 &color));
        p_domain_map->priority = priority;
        p_domain_map->color    = color;
        break;

    case CTC_QOS_DOMAIN_MAP_IGS_DSCP_TO_PRI_COLOR:
        dscp = p_domain_map->hdr_pri.tos.dscp;
        CTC_ERROR_RETURN(sys_usw_qos_get_igs_dscp_map_table(lchip, domain,
                                                                  dscp,
                                                                  &priority,
                                                                  &color));
        p_domain_map->priority = priority;
        p_domain_map->color    = color;
        break;

    case CTC_QOS_DOMAIN_MAP_IGS_IP_PREC_TO_PRI_COLOR:
        return CTC_E_NOT_SUPPORT;
        break;

    case CTC_QOS_DOMAIN_MAP_IGS_EXP_TO_PRI_COLOR:
        exp = p_domain_map->hdr_pri.exp;
        CTC_ERROR_RETURN(sys_usw_qos_get_igs_exp_map_table(lchip, domain,
                                                                 exp,
                                                                 &priority,
                                                                 &color));
        p_domain_map->priority = priority;
        p_domain_map->color    = color;
        break;

    case CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_COS:

        priority = p_domain_map->priority;
        color    = p_domain_map->color;
        CTC_ERROR_RETURN(sys_usw_qos_get_egs_cos_map_table(lchip, domain,
                                                                 priority,
                                                                 color,
                                                                 &cos,
                                                                 &cfi));
        p_domain_map->hdr_pri.dot1p.cos = cos;
        p_domain_map->hdr_pri.dot1p.dei = cfi;
        break;

    case CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_DSCP:
        priority = p_domain_map->priority;
        color    = p_domain_map->color;
        CTC_ERROR_RETURN(sys_usw_qos_get_egs_dscp_map_table(lchip, domain,
                                                                  priority,
                                                                  color,
                                                                  &dscp));
        p_domain_map->hdr_pri.tos.dscp = dscp;
        break;

    case CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_EXP:
        priority = p_domain_map->priority;
        color    = p_domain_map->color;
        CTC_ERROR_RETURN(sys_usw_qos_get_egs_exp_map_table(lchip, domain,
                                                                 priority,
                                                                 color,
                                                                 &exp));
        p_domain_map->hdr_pri.exp = exp;
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_table_map_set(uint8 lchip, ctc_qos_table_map_t* p_table_map)
{
    uint8 table_map_id   = 0;
    uint8 in      = 0;
    uint8 out     = 0;
    uint8 action_type = 0;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    table_map_id  = p_table_map->table_map_id;
    action_type   = p_table_map->action_type;
    in = p_table_map->in;
    out = p_table_map->out;

    switch (p_table_map->type)
    {
    case CTC_QOS_TABLE_MAP_IGS_COS_TO_COS:
        CTC_ERROR_RETURN(sys_usw_qos_set_igs_cos2cos_map_table(lchip, table_map_id,
                                                                 in,
                                                                 out, 
                                                                 action_type));
        break;

    case CTC_QOS_TABLE_MAP_IGS_COS_TO_DSCP:
        CTC_ERROR_RETURN(sys_usw_qos_set_igs_cos2dscp_map_table(lchip, table_map_id,
                                                                  in,
                                                                  out,
                                                                  action_type));
        break;

    case CTC_QOS_TABLE_MAP_IGS_DSCP_TO_COS:
        CTC_ERROR_RETURN(sys_usw_qos_set_igs_dscp2cos_map_table(lchip, table_map_id,
                                                                 in,
                                                                 out, 
                                                                 action_type));
        break;

    case CTC_QOS_TABLE_MAP_IGS_DSCP_TO_DSCP:
        CTC_ERROR_RETURN(sys_usw_qos_set_igs_dscp2dscp_map_table(lchip, table_map_id,
                                                                  in,
                                                                  out,
                                                                  action_type));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_table_map_get(uint8 lchip, ctc_qos_table_map_t* p_table_map)
{
    uint8 table_map_id   = 0;
    uint8 in      = 0;
    uint8 out     = 0;
    uint8 action_type = 0;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    table_map_id  = p_table_map->table_map_id;
    in            = p_table_map->in;

    switch (p_table_map->type)
    {
    case CTC_QOS_TABLE_MAP_IGS_COS_TO_COS:
        CTC_ERROR_RETURN(sys_usw_qos_get_igs_cos2cos_map_table(lchip, table_map_id,
                                                                 in,
                                                                 &out,
                                                                 &action_type));
        break;

    case CTC_QOS_TABLE_MAP_IGS_COS_TO_DSCP:
        CTC_ERROR_RETURN(sys_usw_qos_get_igs_cos2dscp_map_table(lchip, table_map_id,
                                                                  in,
                                                                  &out,
                                                                  &action_type));
        break;

    case CTC_QOS_TABLE_MAP_IGS_DSCP_TO_COS:
        CTC_ERROR_RETURN(sys_usw_qos_get_igs_dscp2cos_map_table(lchip, table_map_id,
                                                                 in,
                                                                 &out,
                                                                 &action_type));
        break;

    case CTC_QOS_TABLE_MAP_IGS_DSCP_TO_DSCP:
        CTC_ERROR_RETURN(sys_usw_qos_get_igs_dscp2dscp_map_table(lchip, table_map_id,
                                                                  in,
                                                                  &out,
                                                                  &action_type));
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    p_table_map->out = out;
    p_table_map->action_type = action_type;

    return CTC_E_NONE;
}

int32
sys_usw_qos_domain_map_default(uint8 lchip)
{
    uint8 color    = 0;
    uint8 priority = 0;
    uint8 value    = 0;
    uint8 domain   = 0;

    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    LCHIP_CHECK(lchip);

    /* init ingress cos + cfi -> priority + color mapping table */
    for (domain = 0; domain < 8; domain++)
    {
        color = CTC_QOS_COLOR_GREEN;

        for (value = 0; value < 8; value++)
        {
            priority = value * 2;
            CTC_ERROR_RETURN(sys_usw_qos_set_igs_cos_map_table(lchip, domain,
                                                                 value,
                                                                 0,
                                                                 priority,
                                                                 color));

            priority = value * 2;
            CTC_ERROR_RETURN(sys_usw_qos_set_igs_cos_map_table(lchip, domain, 
                                                                 value,
                                                                 1,
                                                                 priority,
                                                                 color));
        }
    }

    /* init ingress dscp -> priority + color mapping table */
    for (domain = 0; domain < 16; domain++)
    {
        color = CTC_QOS_COLOR_GREEN;

        for (value = 0; value < 64; value++)
        {
            priority = value / 4;

            CTC_ERROR_RETURN(sys_usw_qos_set_igs_dscp_map_table(lchip, domain,
                                                                          value,
                                                                          priority,
                                                                          color));
        }
    }

    /* init ingress mpls exp -> priority + color mapping table */
    for (domain = 0; domain < 16; domain++)
    {
        color = CTC_QOS_COLOR_GREEN;

        for (value = 0; value < 8; value++)
        {
            priority = value * 2;
            CTC_ERROR_RETURN(sys_usw_qos_set_igs_exp_map_table(lchip, domain,
                                                                      value,
                                                                      priority,
                                                                      color));
        }
    }

    /* init egress priority + color -> cos mapping table */
    for (domain = 0; domain < 8; domain++)
    {
        for (priority = 0; priority < 16; priority++)
        {
            for (color = CTC_QOS_COLOR_NONE; color < MAX_CTC_QOS_COLOR; color++)
            {
                /* init egress priority + color -> cos mapping table */
                value = priority / 2;
                CTC_ERROR_RETURN(sys_usw_qos_set_egs_cos_map_table(lchip, domain,
                                                                          priority,
                                                                          color,
                                                                          value,
                                                                          0));
            }
        }
    }


    /* init egress priority + color -> dscp mapping table */
    for (domain = 0; domain < 16; domain++)
    {
        for (priority = 0; priority < 16; priority++)
        {
            for (color = CTC_QOS_COLOR_NONE; color < MAX_CTC_QOS_COLOR; color++)
            {
                value = priority * 4;
                CTC_ERROR_RETURN(sys_usw_qos_set_egs_dscp_map_table(lchip, domain,
                                                                           priority,
                                                                           color,
                                                                           value));
            }
        }
    }

    /* init egress priority + color -> exp mapping table */
    for (domain = 0; domain < 16; domain++)
    {
        for (priority = 0; priority < 16; priority++)
        {
            for (color = CTC_QOS_COLOR_NONE; color < MAX_CTC_QOS_COLOR; color++)
            {
                value = priority / 2;
                CTC_ERROR_RETURN(sys_usw_qos_set_egs_exp_map_table(lchip, domain,
                                                                          priority,
                                                                          color,
                                                                          value));
            }
        }
    }

    return CTC_E_NONE;
}

int32
_sys_usw_qos_domain_map_dump(uint8 lchip, uint8 domain, uint8 type)
{
    uint8 color    = 0;
    uint8 priority = 0;
    uint8 value    = 0;
    uint8 cfi      = 0;

    LCHIP_CHECK(lchip);

    switch (type)
    {
    case CTC_QOS_DOMAIN_MAP_IGS_COS_TO_PRI_COLOR:
        {
            CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_COS_DOMAIN_MAX));

            /* ingress cos + cfi -> priority + color mapping table */
            SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ingress cos + dei -> priority + color mapping table\n");
            SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------\n");

            for (value = 0; value < 8; value++)
            {
                CTC_ERROR_RETURN(sys_usw_qos_get_igs_cos_map_table(lchip, domain,
                                                                         value,
                                                                         0,
                                                                         &priority,
                                                                         &color));

                SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "   cos(%d)  +  dei(%d) -> priority(%d)  + color(%d)\n",
                                       value, 0, priority, color);

                CTC_ERROR_RETURN(sys_usw_qos_get_igs_cos_map_table(lchip, domain,
                                                                         value,
                                                                         1,
                                                                         &priority,
                                                                         &color));

                SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "   cos(%d)  +  dei(%d) -> priority(%d)  + color(%d)\n",
                                       value, 1, priority, color);
            }
        }
        break;

    case CTC_QOS_DOMAIN_MAP_IGS_IP_PREC_TO_PRI_COLOR:
        return CTC_E_NOT_SUPPORT;
        break;

    case CTC_QOS_DOMAIN_MAP_IGS_DSCP_TO_PRI_COLOR:
        {
            CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_DSCP_DOMAIN_MAX));

            /* ingress dscp -> priority + color mapping table */
            SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ingress dscp -> priority + color mapping table\n");
            SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------\n");

            for (value = 0; value < 64; value++)
            {
                CTC_ERROR_RETURN(sys_usw_qos_get_igs_dscp_map_table(lchip, domain,
                                                                           value,
                                                                           &priority,
                                                                           &color));

                SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "    dscp(%d) -> priority(%d)  + color(%d)\n",
                                       value, priority, color);
            }
        }

        break;

    case CTC_QOS_DOMAIN_MAP_IGS_EXP_TO_PRI_COLOR:
        {
            CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_EXP_DOMAIN_MAX));

            /* ingress mpls exp -> priority + color mapping table */
            SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ingress exp -> priority + color mapping table\n");
            SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------\n");

            for (value = 0; value < 8; value++)
            {
                CTC_ERROR_RETURN(sys_usw_qos_get_igs_exp_map_table(lchip, domain,
                                                                         value,
                                                                         &priority,
                                                                         &color));

                SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "    exp(%d) -> priority(%d)  + color(%d)\n",
                                       value, priority, color);
            }
        }

        break;

    case CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_COS:
        {
            CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_COS_DOMAIN_MAX));

            /* egress priority + color -> cos mapping table */
            SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "egress  priority + color -> cos  mapping table\n");
            SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------\n");

            for (priority = 0; priority < 16; priority++)
            {
                for (color = CTC_QOS_COLOR_NONE; color < MAX_CTC_QOS_COLOR; color++)
                {
                    CTC_ERROR_RETURN(sys_usw_qos_get_egs_cos_map_table(lchip, domain,
                                                                             priority,
                                                                             color,
                                                                             &value,
                                                                             &cfi));

                    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " priority(%d)  + color(%d)  -> cos(%d) + dei(%d)\n",
                                           priority, color, value, cfi);

                }
            }
        }

        break;

    case CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_DSCP:
        {
            CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_DSCP_DOMAIN_MAX));

            /* init egress priority + color -> dscp mapping table */
            SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "egress  priority + color -> dscp  mapping table\n");
            SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------\n");

            for (priority = 0; priority < 16; priority++)
            {
                for (color = CTC_QOS_COLOR_NONE; color < MAX_CTC_QOS_COLOR; color++)
                {
                    CTC_ERROR_RETURN(sys_usw_qos_get_egs_dscp_map_table(lchip, domain,
                                                                              priority,
                                                                              color,
                                                                              &value));

                    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " priority(%d)  + color(%d)  -> dscp(%d)\n",
                                           priority, color, value);
                }
            }
        }

        break;

    case CTC_QOS_DOMAIN_MAP_EGS_PRI_COLOR_TO_EXP:
        {
            CTC_MAX_VALUE_CHECK(domain, MCHIP_CAP(SYS_CAP_QOS_CLASS_EXP_DOMAIN_MAX));

            /* init egress priority + color -> exp mapping table */
            SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "egress  priority + color -> exp  mapping table\n");
            SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "-----------------------------------------------------\n");

            for (priority = 0; priority < 16; priority++)
            {
                for (color = CTC_QOS_COLOR_NONE; color < MAX_CTC_QOS_COLOR; color++)
                {
                    CTC_ERROR_RETURN(sys_usw_qos_get_egs_exp_map_table(lchip, domain,
                                                                             priority,
                                                                             color,
                                                                             &value));

                    SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, " priority(%d)  + color(%d)  -> exp(%d)\n",
                                           priority, color, value);
                }
            }
        }
        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
_sys_usw_qos_table_map_dump(uint8 lchip, uint8 table_map_id, uint8 type)
{
    uint8 in = 0;
    uint8 out = 0;
    uint8 action_type = 0;
    char*  str_action[3] = {"None", "Copy", "Map"};

    LCHIP_CHECK(lchip);

    switch (type)
    {
    case CTC_QOS_TABLE_MAP_IGS_COS_TO_COS:
        
        CTC_MAX_VALUE_CHECK(table_map_id, MCHIP_CAP(SYS_CAP_QOS_CLASS_TABLE_MAP_ID_MAX));

        /* ingress cos -> cos mapping table */
        SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ingress cos -> cos mapping table\n");
        SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------\n");

        for (in = 0; in <= SYS_QOS_CLASS_COS_MAX; in++)         
        {
            CTC_ERROR_RETURN(sys_usw_qos_get_igs_cos2cos_map_table(lchip, table_map_id,
                                                                     in,
                                                                     &out,
                                                                     &action_type));

            SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "   cos(%d) -> cos(%d) + action(%s)\n",
                                  in, out, str_action[action_type]);
        }
        
        break;

    case CTC_QOS_TABLE_MAP_IGS_COS_TO_DSCP:
        CTC_MAX_VALUE_CHECK(table_map_id, MCHIP_CAP(SYS_CAP_QOS_CLASS_TABLE_MAP_ID_MAX));

        /* ingress cos -> dscp mapping table */
        SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ingress cos -> dscp mapping table\n");
        SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------\n");

        for (in = 0; in <= SYS_QOS_CLASS_COS_MAX; in++)         
        {
            CTC_ERROR_RETURN(sys_usw_qos_get_igs_cos2dscp_map_table(lchip, table_map_id,
                                                                      in,
                                                                      &out,
                                                                      &action_type));

            SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "   cos(%d) -> dscp(%d) + action(%s)\n",
                                  in, out, str_action[action_type]);
        }
  
    break;

    case CTC_QOS_TABLE_MAP_IGS_DSCP_TO_COS:
        CTC_MAX_VALUE_CHECK(table_map_id, MCHIP_CAP(SYS_CAP_QOS_CLASS_TABLE_MAP_ID_MAX));

        /* ingress cos -> dscp mapping table */
        SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ingress dscp -> cos mapping table\n");
        SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------\n");

        for (in = 0; in <= SYS_QOS_CLASS_DSCP_MAX; in++)         
        {
            CTC_ERROR_RETURN(sys_usw_qos_get_igs_dscp2cos_map_table(lchip, table_map_id,
                                                                      in,
                                                                      &out,
                                                                      &action_type));

            SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "   dscp(%d) -> cos(%d) + action(%s)\n",
                                  in, out, str_action[action_type]);
        }

        break;

    case CTC_QOS_TABLE_MAP_IGS_DSCP_TO_DSCP:
        CTC_MAX_VALUE_CHECK(table_map_id, MCHIP_CAP(SYS_CAP_QOS_CLASS_TABLE_MAP_ID_MAX));

        /* ingress cos -> dscp mapping table */
        SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ingress dscp -> dscp mapping table\n");
        SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "----------------------------------------\n");

        for (in = 0; in <= SYS_QOS_CLASS_DSCP_MAX; in++)         
        {
            CTC_ERROR_RETURN(sys_usw_qos_get_igs_dscp2dscp_map_table(lchip, table_map_id,
                                                                       in,
                                                                       &out,
                                                                       &action_type));

            SYS_QOS_CLASS_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "   dscp(%d) -> dscp(%d) + action(%s)\n",
                                  in, out, str_action[action_type]);
        }

        break;

    default:
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

/**
 @brief init qos classification mapping tables
*/
int32
sys_usw_qos_class_init(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint8  domain_id = 0;
    IpePktProcPhbCtl_m ipe_phb_ctl;
    IpePhbDot1pUntaggedMap_m untag_phb_map;
    
    /*********************************************************************************************************
    *  Trust cos default cfg
    
    *  cosPhbUseInner  srcOuterVlanIsSvlan  phbCvlanIdValid   phbSvlanIdValid   classifiedCos  classifiedCfi 
    *  --------------  -------------------  ---------------   ---------------   -------------  -------------
    *        0                  0                  0                 0            defaultPcp     defaultDei
    *        0                  0                  0                 1            defaultPcp     defaultDei
    *        0                  0                  1                 0            defaultPcp     defaultDei   
    *        0                  0                  1                 1            defaultPcp     defaultDei  
    *        0                  1                  0                 0            defaultPcp     defaultDei
    *        0                  1                  0                 1             stagCos        stagCfi
    *        0                  1                  1                 0            defaultPcp     defaultDei
    *        0                  1                  1                 1             stagCos        stagCfi
    *        1                  0                  0                 0            defaultPcp     defaultDei
    *        1                  0                  0                 1            defaultPcp     defaultDei
    *        1                  0                  1                 0             ctagCos        ctagCfi
    *        1                  0                  1                 1            defaultPcp     defaultPcp        
    *        1                  1                  0                 0            defaultPcp     defaultDei
    *        1                  1                  0                 1            defaultPcp     defaultDei
    *        1                  1                  1                 0            defaultPcp     defaultDei
    *        1                  1                  1                 1             ctagCos        ctagCfi      
    **********************************************************************************************************/
    
    /* IpePktProcPhbCtl.g_i_cosType_f, i=(cosPhbUseInner,srcOuterVlanIsSvlan,phbCvlanIdValid,phbSvlanIdValid) 
       cosType=0 : defaultPcp; cosType=1 : stagCos; cosType=2 : ctagCos;
    */
    sal_memset(&ipe_phb_ctl, 0, sizeof(IpePktProcPhbCtl_m));
    
    cmd = DRV_IOR(IpePktProcPhbCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_phb_ctl));
    SetIpePktProcPhbCtl(V, g_0_cosType_f, &ipe_phb_ctl, 0);
    SetIpePktProcPhbCtl(V, g_1_cosType_f, &ipe_phb_ctl, 0);
    SetIpePktProcPhbCtl(V, g_2_cosType_f, &ipe_phb_ctl, 0);
    SetIpePktProcPhbCtl(V, g_3_cosType_f, &ipe_phb_ctl, 0);
    SetIpePktProcPhbCtl(V, g_4_cosType_f, &ipe_phb_ctl, 0);
    SetIpePktProcPhbCtl(V, g_5_cosType_f, &ipe_phb_ctl, 1);
    SetIpePktProcPhbCtl(V, g_6_cosType_f, &ipe_phb_ctl, 0);
    SetIpePktProcPhbCtl(V, g_7_cosType_f, &ipe_phb_ctl, 1);
    SetIpePktProcPhbCtl(V, g_8_cosType_f, &ipe_phb_ctl, 0);
    SetIpePktProcPhbCtl(V, g_9_cosType_f, &ipe_phb_ctl, 0);
    SetIpePktProcPhbCtl(V, g_10_cosType_f, &ipe_phb_ctl, 2);
    SetIpePktProcPhbCtl(V, g_11_cosType_f, &ipe_phb_ctl, 0);
    SetIpePktProcPhbCtl(V, g_12_cosType_f, &ipe_phb_ctl, 0);
    SetIpePktProcPhbCtl(V, g_13_cosType_f, &ipe_phb_ctl, 0);
    SetIpePktProcPhbCtl(V, g_14_cosType_f, &ipe_phb_ctl, 0);
    SetIpePktProcPhbCtl(V, g_15_cosType_f, &ipe_phb_ctl, 2);
    cmd = DRV_IOW(IpePktProcPhbCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_phb_ctl));

    field_val = 1;
    cmd = DRV_IOW(IpeFwdCtl_t, IpeFwdCtl_mutationTableTypeEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    CTC_ERROR_RETURN(sys_usw_qos_domain_map_default(lchip));

    sal_memset(&untag_phb_map, 0, sizeof(IpePhbDot1pUntaggedMap_m));

    /*config IpePhbDot1pUntaggedMap*/ 
    for (domain_id = 0; domain_id <= MCHIP_CAP(SYS_CAP_QOS_CLASS_COS_DOMAIN_MAX); domain_id++)         
    {        
        cmd = DRV_IOR(IpePhbDot1pUntaggedMap_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, domain_id, cmd, &untag_phb_map));
        SetIpePhbDot1pTaggedMap(V, prio_f, &untag_phb_map, 0);
        SetIpePhbDot1pTaggedMap(V, color_f, &untag_phb_map, CTC_QOS_COLOR_GREEN);
        cmd = DRV_IOW(IpePhbDot1pUntaggedMap_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, domain_id, cmd, &untag_phb_map));     
    }

    return CTC_E_NONE;
}

int32
sys_usw_qos_class_deinit(uint8 lchip)
{
    return CTC_E_NONE;
}

