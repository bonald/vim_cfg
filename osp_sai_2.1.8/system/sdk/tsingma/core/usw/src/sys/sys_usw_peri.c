/**
 @file sys_usw_peri.c

 @date 2009-10-19

 @version v2.0

 The file define APIs of chip of sys layer
*/
/****************************************************************************
 *
 * Header Files
 *
 ****************************************************************************/
#include "sal.h"
#include "dal.h"
#include "ctc_error.h"
#include "ctc_register.h"
#include "sys_usw_peri.h"
#include "sys_usw_chip_global.h"
#include "sys_usw_mac.h"
#include "sys_usw_dmps.h"
#include "sys_usw_chip.h"
#include "sys_usw_datapath.h"
#include "sys_usw_common.h"
#include "sys_usw_interrupt.h"
#include "sys_usw_dma.h"
#include "sys_usw_port.h"

#include "drv_api.h"

/****************************************************************************
 *
 * Global and static
 *
 *****************************************************************************/
sys_peri_master_t* p_usw_peri_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

extern dal_op_t g_dal_op;
int32
sys_usw_peri_set_phy_scan_cfg(uint8 lchip);
extern int32
_sys_usw_mac_get_cl37_an_remote_status(uint8 lchip, uint32 gport, uint32 auto_neg_mode, uint32* p_speed, uint32* p_link);
/****************************************************************************
 *
 * Function
 *
 *****************************************************************************/
/**
 @brief The function is to initialize the chip module and set the local chip number of the linecard
*/
int32
sys_usw_peri_init(uint8 lchip, uint8 lchip_num)
{
    int32 ret;
#if 0
    if (lchip_num > SYS_USW_MAX_LOCAL_CHIP_NUM)
    {
        return CTC_E_INVALID_CHIP_NUM;
    }
#endif
    p_usw_peri_master[lchip] = (sys_peri_master_t*)mem_malloc(MEM_SYSTEM_MODULE, sizeof(sys_peri_master_t));

    if (NULL == p_usw_peri_master[lchip])
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
		return CTC_E_NO_MEMORY;
    }

    sal_memset(p_usw_peri_master[lchip], 0, sizeof(sys_peri_master_t));

    /* create mutex for chip module */
    ret = sal_mutex_create(&(p_usw_peri_master[lchip]->p_peri_mutex));
    if (ret || (!p_usw_peri_master[lchip]->p_peri_mutex))
    {
        ret = CTC_E_NO_MEMORY;
        goto error0;
    }

    /* create mutex for dynamic switch */
    ret = sal_mutex_create(&(p_usw_peri_master[lchip]->p_switch_mutex));
    if (ret || (!p_usw_peri_master[lchip]->p_switch_mutex))
    {
        ret = CTC_E_NO_MEMORY;
        goto error1;
    }

    /* create mutex for I2C */
    ret = sal_mutex_create(&(p_usw_peri_master[lchip]->p_i2c_mutex));
    if (ret || (!p_usw_peri_master[lchip]->p_i2c_mutex))
    {
        ret = CTC_E_NO_MEMORY;
        goto error2;
    }

    /* create mutex for SMI */
    ret = sal_mutex_create(&(p_usw_peri_master[lchip]->p_smi_mutex));
    if (ret || (!p_usw_peri_master[lchip]->p_smi_mutex))
    {
        ret = CTC_E_NO_MEMORY;
        goto error3;
    }

    p_usw_peri_master[lchip]->phy_list = ctc_slist_new();
    if (NULL == p_usw_peri_master[lchip]->phy_list)
    {
        ret = CTC_E_NO_MEMORY;
        goto error4;
    }

    p_usw_peri_master[lchip]->phy_register = mem_malloc(MEM_PORT_MODULE, SYS_USW_MAX_PHY_PORT * sizeof(void*));
    if (NULL == p_usw_peri_master[lchip]->phy_register)
    {
        ret = CTC_E_NO_MEMORY;
        goto error5;
    }
    sal_memset(p_usw_peri_master[lchip]->phy_register, 0, SYS_USW_MAX_PHY_PORT * sizeof(void*));

    /* peri io init */
    CTC_ERROR_GOTO(MCHIP_API(lchip)->peri_init(lchip), ret, error6);

    return CTC_E_NONE;
error6:
    mem_free(p_usw_peri_master[lchip]->phy_register);
error5:
    ctc_slist_delete(p_usw_peri_master[lchip]->phy_list);
error4:
    ctc_sal_mutex_destroy(p_usw_peri_master[lchip]->p_smi_mutex);
error3:
    ctc_sal_mutex_destroy(p_usw_peri_master[lchip]->p_i2c_mutex);
error2:
    ctc_sal_mutex_destroy(p_usw_peri_master[lchip]->p_switch_mutex);
error1:
    ctc_sal_mutex_destroy(p_usw_peri_master[lchip]->p_peri_mutex);
error0:
    mem_free(p_usw_peri_master[lchip]);

    return ret;
}

int32
sys_usw_peri_deinit(uint8 lchip)
{
    ctc_slistnode_t* node = NULL;
    ctc_slistnode_t* next_node = NULL;

    if (NULL == p_usw_peri_master[lchip])
    {
        return CTC_E_NONE;
    }

    mem_free(p_usw_peri_master[lchip]->phy_register);

    /*free phy slist*/
    CTC_SLIST_LOOP_DEL(p_usw_peri_master[lchip]->phy_list, node, next_node)
    {
        ctc_slist_delete_node(p_usw_peri_master[lchip]->phy_list, node);
        mem_free(node);
    }
    ctc_slist_delete(p_usw_peri_master[lchip]->phy_list);

    if (p_usw_peri_master[lchip]->p_peri_mutex)
    {
        ctc_sal_mutex_destroy(p_usw_peri_master[lchip]->p_peri_mutex);
    }

    if (p_usw_peri_master[lchip]->p_switch_mutex)
    {
        ctc_sal_mutex_destroy(p_usw_peri_master[lchip]->p_switch_mutex);
    }

    if (p_usw_peri_master[lchip]->p_i2c_mutex)
    {
        ctc_sal_mutex_destroy(p_usw_peri_master[lchip]->p_i2c_mutex);
    }

    if (p_usw_peri_master[lchip]->p_smi_mutex)
    {
        ctc_sal_mutex_destroy(p_usw_peri_master[lchip]->p_smi_mutex);
    }

    mem_free(p_usw_peri_master[lchip]);

    return CTC_E_NONE;
}

int32
sys_usw_mdio_init(uint8 lchip)
{
    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_mdio_init(lchip));
    return CTC_E_NONE;
}

#define __MDIO_INTERFACE__
/**
 @brief set phy to port mapping
*/
STATIC  int32
_sys_usw_peri_phy_driver_probe(uint8 lchip, uint16 lport, uint8 type)
{
    uint8 found = 0;
    uint32 phy_id;
    ctc_chip_mdio_para_t para;
    ctc_slistnode_t        *node = NULL;
    sys_chip_phy_shim_t* p_phy_shim = NULL;

    sal_memset(&para, 0, sizeof(ctc_chip_mdio_para_t));

    para.bus = p_usw_peri_master[lchip]->port_mdio_mapping_tbl[lport];
    para.phy_addr = p_usw_peri_master[lchip]->port_phy_mapping_tbl[lport];
    para.dev_no = p_usw_peri_master[lchip]->port_dev_no_mapping_tbl[lport];

    if (para.bus == 0xFF)
    {
        phy_id = CTC_CHIP_PHY_NULL_PHY_ID;
    }
    else
    {
        para.reg = 2;
        CTC_ERROR_RETURN(sys_usw_peri_mdio_read(lchip, type, &para));
        phy_id = para.value;
        para.reg = 3;
        CTC_ERROR_RETURN(sys_usw_peri_mdio_read(lchip, type, &para));
        phy_id = para.value<<16|phy_id;
    }

    CTC_SLIST_LOOP(p_usw_peri_master[lchip]->phy_list, node)
    {
        p_phy_shim = _ctc_container_of(node, sys_chip_phy_shim_t, head);
        if (p_phy_shim->phy_shim.phy_id == phy_id)
        {
            found = 1;
            break;
        }
    }

    if (1 == found)
    {
        p_usw_peri_master[lchip]->phy_register[lport] = (ctc_chip_phy_shim_t*)&(p_phy_shim->phy_shim);
        return CTC_E_NONE;
    }

    return CTC_E_NOT_EXIST;
}

int32
sys_usw_peri_set_phy_prop(uint8 lchip, uint16 lport, uint16 phy_prop_type, uint32 value)
{
    ctc_chip_mdio_type_t type = CTC_CHIP_MDIO_GE;
    ctc_chip_mdio_para_t para;
    sys_datapath_lport_attr_t* port_attr = NULL;

    sal_memset(&para, 0, sizeof(ctc_chip_mdio_para_t));

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " set phy driver lport %d is not used \n", lport);
        return CTC_E_INVALID_LOCAL_PORT;
    }
    if ((SYS_DMPS_NETWORK_PORT == port_attr->port_type) && (CTC_PORT_SPEED_10G == port_attr->speed_mode))
    {
        type = CTC_CHIP_MDIO_XG;
    }

    PERI_LOCK(lchip);
    para.bus = p_usw_peri_master[lchip]->port_mdio_mapping_tbl[lport];
    para.phy_addr = p_usw_peri_master[lchip]->port_phy_mapping_tbl[lport];
    para.dev_no = p_usw_peri_master[lchip]->port_dev_no_mapping_tbl[lport];

    switch(phy_prop_type)
    {
        case CTC_PORT_PROP_PHY_INIT:
            CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_peri_phy_driver_probe(lchip, lport, type), p_usw_peri_master[lchip]->p_peri_mutex);
            if (p_usw_peri_master[lchip]->phy_register[lport]->driver.init && value)
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.init(lchip, &para), p_usw_peri_master[lchip]->p_peri_mutex);
            }
            else if ((p_usw_peri_master[lchip]->phy_register[lport]->driver.deinit) && (0 == value))
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.deinit(lchip, &para), p_usw_peri_master[lchip]->p_peri_mutex);
                p_usw_peri_master[lchip]->phy_register[lport] = NULL;
            }
            else
            {
                PERI_UNLOCK(lchip);
                return CTC_E_INVALID_PARAM;
            }
            break;
        case CTC_PORT_PROP_PHY_EN:
            if (p_usw_peri_master[lchip]->phy_register[lport]->driver.set_port_en)
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.set_port_en(lchip, &para, value), p_usw_peri_master[lchip]->p_peri_mutex);
            }
            break;
        case CTC_PORT_PROP_PHY_DUPLEX:
            if (p_usw_peri_master[lchip]->phy_register[lport]->driver.set_port_duplex_en)
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.set_port_duplex_en(lchip, &para, value), p_usw_peri_master[lchip]->p_peri_mutex);
            }
            break;
        case CTC_PORT_PROP_PHY_MEDIUM:
            if (p_usw_peri_master[lchip]->phy_register[lport]->driver.set_port_medium)
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.set_port_medium(lchip, &para, value), p_usw_peri_master[lchip]->p_peri_mutex);
            }
            break;
        case CTC_PORT_PROP_PHY_LOOPBACK:
            if (p_usw_peri_master[lchip]->phy_register[lport]->driver.set_port_loopback)
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.set_port_loopback(lchip, &para, value), p_usw_peri_master[lchip]->p_peri_mutex);
            }
            break;

        case CTC_PORT_PROP_AUTO_NEG_EN:
            if (p_usw_peri_master[lchip]->phy_register[lport]->driver.set_port_auto_neg_en)
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.set_port_auto_neg_en(lchip, &para, value), p_usw_peri_master[lchip]->p_peri_mutex);
            }
            break;
        case CTC_PORT_PROP_SPEED:
            if (p_usw_peri_master[lchip]->phy_register[lport]->driver.set_port_speed)
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.set_port_speed(lchip, &para, value), p_usw_peri_master[lchip]->p_peri_mutex);
            }
            break;
        case CTC_PORT_PROP_UNIDIR_EN:
            if (p_usw_peri_master[lchip]->phy_register[lport]->driver.set_port_unidir_en)
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.set_port_unidir_en(lchip, &para, value), p_usw_peri_master[lchip]->p_peri_mutex);
            }
            break;
        default:
            if ((phy_prop_type >= CTC_PORT_PROP_PHY_CUSTOM_BASE) && (phy_prop_type <= CTC_PORT_PROP_PHY_CUSTOM_MAX_TYPE))
            {
                if (p_usw_peri_master[lchip]->phy_register[lport]->driver.set_ext_attr)
                {
                    CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.set_ext_attr(lchip, &para, phy_prop_type, value), p_usw_peri_master[lchip]->p_peri_mutex);
                }
            }
            else
            {
                PERI_UNLOCK(lchip);
                return CTC_E_INVALID_PARAM;
            }

    }
    PERI_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_peri_get_phy_prop(uint8 lchip, uint16 lport, uint16 phy_prop_type, uint32* p_value)
{
    ctc_chip_mdio_para_t para;
    sys_datapath_lport_attr_t* port_attr = NULL;

    sal_memset(&para, 0, sizeof(ctc_chip_mdio_para_t));

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " get phy driver lport %d is not used \n", lport);
        return CTC_E_INVALID_LOCAL_PORT;
    }

    PERI_LOCK(lchip);
    if (NULL == p_usw_peri_master[lchip]->phy_register[lport])
    {
        PERI_UNLOCK(lchip);
        return CTC_E_NOT_INIT;
    }
    para.bus = p_usw_peri_master[lchip]->port_mdio_mapping_tbl[lport];
    para.phy_addr = p_usw_peri_master[lchip]->port_phy_mapping_tbl[lport];
    para.dev_no = p_usw_peri_master[lchip]->port_dev_no_mapping_tbl[lport];

    switch(phy_prop_type)
    {
        case CTC_PORT_PROP_PHY_EN:
            if (p_usw_peri_master[lchip]->phy_register[lport]->driver.get_port_en)
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.get_port_en(lchip, &para, p_value), p_usw_peri_master[lchip]->p_peri_mutex);
            }
            break;
        case CTC_PORT_PROP_PHY_DUPLEX:
            if (p_usw_peri_master[lchip]->phy_register[lport]->driver.get_port_duplex_en)
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.get_port_duplex_en(lchip, &para, p_value), p_usw_peri_master[lchip]->p_peri_mutex);
            }
            break;
        case CTC_PORT_PROP_PHY_MEDIUM:
            if (p_usw_peri_master[lchip]->phy_register[lport]->driver.get_port_medium)
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.get_port_medium(lchip, &para, p_value), p_usw_peri_master[lchip]->p_peri_mutex);
            }
            break;
        case CTC_PORT_PROP_PHY_LOOPBACK:
            if (p_usw_peri_master[lchip]->phy_register[lport]->driver.get_port_loopback)
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.get_port_loopback(lchip, &para, p_value), p_usw_peri_master[lchip]->p_peri_mutex);
            }
            break;

        case CTC_PORT_PROP_AUTO_NEG_EN:
            if (p_usw_peri_master[lchip]->phy_register[lport]->driver.get_port_auto_neg_en)
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.get_port_auto_neg_en(lchip, &para, p_value), p_usw_peri_master[lchip]->p_peri_mutex);
            }
            break;
        case CTC_PORT_PROP_SPEED:
            if (p_usw_peri_master[lchip]->phy_register[lport]->driver.get_port_speed)
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.get_port_speed(lchip, &para, p_value), p_usw_peri_master[lchip]->p_peri_mutex);
            }
            break;
        case CTC_PORT_PROP_LINK_UP:
            if (p_usw_peri_master[lchip]->phy_register[lport]->driver.get_link_up_status)
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.get_link_up_status(lchip, &para, p_value), p_usw_peri_master[lchip]->p_peri_mutex);
            }
            break;
        case CTC_PORT_PROP_UNIDIR_EN:
            if (p_usw_peri_master[lchip]->phy_register[lport]->driver.get_port_unidir_en)
            {
                CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.get_port_unidir_en(lchip, &para, p_value), p_usw_peri_master[lchip]->p_peri_mutex);
            }
            break;
        default:
            if ((phy_prop_type >= CTC_PORT_PROP_PHY_CUSTOM_BASE) && (phy_prop_type <= CTC_PORT_PROP_PHY_CUSTOM_MAX_TYPE))
            {
                if (p_usw_peri_master[lchip]->phy_register[lport]->driver.get_ext_attr)
                {
                    CTC_ERROR_RETURN_WITH_UNLOCK(p_usw_peri_master[lchip]->phy_register[lport]->driver.get_ext_attr(lchip, &para, phy_prop_type, p_value), p_usw_peri_master[lchip]->p_peri_mutex);
                }
            }
            else
            {
                PERI_UNLOCK(lchip);
                return CTC_E_INVALID_PARAM;
            }

    }
    PERI_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_peri_get_phy_id(uint8 lchip, uint16 lport, uint32* phy_id)
{
    sys_datapath_lport_attr_t* port_attr = NULL;

    SYS_MAX_PHY_PORT_CHECK(lport);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " get phy id lport %d is not used \n", lport);
        return CTC_E_INVALID_LOCAL_PORT;
    }

    PERI_LOCK(lchip);
    if (NULL != p_usw_peri_master[lchip]->phy_register[lport])
    {
        *phy_id = p_usw_peri_master[lchip]->phy_register[lport]->phy_id;
    }
    else
    {
        PERI_UNLOCK(lchip);
        return CTC_E_NOT_INIT;
    }
    PERI_UNLOCK(lchip);

    return CTC_E_NONE;

}

int32
sys_usw_peri_get_phy_register_exist(uint8 lchip, uint16 lport)
{
    sys_datapath_lport_attr_t* port_attr = NULL;

    SYS_MAX_PHY_PORT_CHECK(lport);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type != SYS_DMPS_NETWORK_PORT)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " get phy register exist lport %d is not used \n", lport);
        return CTC_E_INVALID_PORT;
    }

    PERI_LOCK(lchip);
    if (NULL != p_usw_peri_master[lchip]->phy_register[lport])
    {
        PERI_UNLOCK(lchip);
        return CTC_E_NONE;
    }
    PERI_UNLOCK(lchip);

    return CTC_E_NOT_INIT;
}
int32
sys_usw_peri_set_phy_mapping(uint8 lchip, ctc_chip_phy_mapping_para_t* phy_mapping_para)
{
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (NULL == phy_mapping_para)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid pointer \n");
        return CTC_E_INVALID_PTR;

    }

    sal_memcpy(p_usw_peri_master[lchip]->port_mdio_mapping_tbl, phy_mapping_para->port_mdio_mapping_tbl,
               SYS_USW_MAX_PHY_PORT);

    sal_memcpy(p_usw_peri_master[lchip]->port_phy_mapping_tbl, phy_mapping_para->port_phy_mapping_tbl,
               SYS_USW_MAX_PHY_PORT);

    return CTC_E_NONE;
}

/**
 @brief get phy to port mapping
*/
int32
sys_usw_peri_get_phy_mapping(uint8 lchip, ctc_chip_phy_mapping_para_t* p_phy_mapping_para)
{
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (NULL == p_phy_mapping_para)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid pointer \n");
        return CTC_E_INVALID_PTR;

    }

    sal_memcpy(p_phy_mapping_para->port_mdio_mapping_tbl,  p_usw_peri_master[lchip]->port_mdio_mapping_tbl,
               SYS_USW_MAX_PHY_PORT);

    sal_memcpy(p_phy_mapping_para->port_phy_mapping_tbl,  p_usw_peri_master[lchip]->port_phy_mapping_tbl,
               SYS_USW_MAX_PHY_PORT);


    return CTC_E_NONE;
}

/* called by MDIO scan init and dynamic switch */
int32
sys_usw_peri_set_phy_scan_cfg(uint8 lchip)
{
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_set_phy_scan_cfg(lchip));

    return CTC_E_NONE;
}

/**
 @brief smi interface for set auto scan para
*/
int32
sys_usw_peri_set_phy_scan_para(uint8 lchip, ctc_chip_phy_scan_ctrl_t* p_scan_para)
{
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_set_phy_scan_para(lchip, p_scan_para));

    return CTC_E_NONE;
}

/**
 @brief smi interface for sgt auto scan para
*/
int32
sys_usw_peri_get_phy_scan_para(uint8 lchip, ctc_chip_phy_scan_ctrl_t* p_scan_para)
{
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_get_phy_scan_para(lchip, p_scan_para));

    return CTC_E_NONE;
}

/**
 @brief smi interface for set auto scan enable or not
*/
int32
sys_usw_peri_set_phy_scan_en(uint8 lchip, bool enable)
{
    uint32 cmd0 = 0, cmd1 = 0;
    uint32 field_value = (TRUE == enable) ? 1 : 0;
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set Scan Enable enable:%d\n", ((TRUE == enable) ? 1 : 0));

    if (DRV_IS_DUET2(lchip))
    {
        uint32 cmd2 = 0, cmd3 = 0;
        cmd0 = DRV_IOW(MdioScanCtl00_t, MdioScanCtl00_scanStartLane0_f);
        cmd1 = DRV_IOW(MdioScanCtl01_t, MdioScanCtl01_scanStartLane0_f);
        cmd2 = DRV_IOW(MdioScanCtl10_t, MdioScanCtl10_scanStartLane1_f);
        cmd3 = DRV_IOW(MdioScanCtl11_t, MdioScanCtl11_scanStartLane1_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd0, &field_value));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd1, &field_value));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd2, &field_value));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd3, &field_value));
    }
    else
    {
        cmd0 = DRV_IOW(MdioScanCtl0_t, MdioScanCtl0_scanStartLane0_f);
        cmd1 = DRV_IOW(MdioScanCtl1_t, MdioScanCtl1_scanStartLane1_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd0, &field_value));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd1, &field_value));
    }

    return CTC_E_NONE;
}

/**
 @brief smi interface for get auto scan enable or not
*/
int32
sys_usw_peri_get_phy_scan_en(uint8 lchip, bool* enable)
{
    uint32 cmd = 0;
    uint32 field_value = 0;

    SYS_PERI_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (DRV_IS_DUET2(lchip))
    {
        cmd = DRV_IOR(MdioScanCtl00_t, MdioScanCtl00_scanStartLane0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    }
    else
    {
        cmd = DRV_IOR(MdioScanCtl0_t, MdioScanCtl0_scanStartLane0_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    }

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get Scan Enable enable:%d\n", field_value);
    *enable = field_value;

    return CTC_E_NONE;
}

/**
 @brief mdio interface
*/
int32
sys_usw_peri_mdio_read(uint8 lchip, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para)
{
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_para);

    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_read_phy_reg(lchip, type, p_para));

    return CTC_E_NONE;
}

/**
 @brief mdio interface
*/
int32
sys_usw_peri_mdio_write(uint8 lchip, ctc_chip_mdio_type_t type, ctc_chip_mdio_para_t* p_para)
{
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_para);
    LCHIP_CHECK(lchip);
    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_write_phy_reg(lchip, type, p_para));

    return CTC_E_NONE;
}

/**
 @brief mdio interface to set mdio clock frequency
*/
int32
sys_usw_peri_set_mdio_clock(uint8 lchip, ctc_chip_mdio_type_t type, uint16 freq)
{
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if ((type != CTC_CHIP_MDIO_GE) && (type != CTC_CHIP_MDIO_XG))
    {
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_set_mdio_clock(lchip, type, freq));

    return CTC_E_NONE;
}

/**
 @brief mdio interface to get mdio clock frequency
*/
int32
sys_usw_peri_get_mdio_clock(uint8 lchip, ctc_chip_mdio_type_t type, uint16* freq)
{
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(freq);

    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_get_mdio_clock(lchip, type, freq));

    return CTC_E_NONE;
}

#define __I2C_MASTER_INTERFACE__

STATIC int32
_sys_usw_peri_i2c_set_switch(uint8 lchip, uint8 ctl_id, uint8 i2c_switch_id)
{
    uint32 cmd = 0;
    uint32 field_value = 0;

    field_value = (SYS_CHIP_INVALID_SWITCH_ID != i2c_switch_id)? 1 : 0;
    cmd = DRV_IOW(I2CMasterCfg0_t + ctl_id, I2CMasterCfg0_switchEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    if (i2c_switch_id)
    {
        field_value = 0xe ;/* pca9548a device fixed address */
        cmd = DRV_IOW(I2CMasterCfg0_t + ctl_id, I2CMasterCfg0_switchAddr_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_peri_set_i2c_byte_access(uint8 lchip, uint8 ctl_id, bool enable)
{

    uint32 cmd = 0;
    uint32 field_value = 0;
    uint32 tbl_id = 0;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set byte access enable:%d\n", (enable ? 1 : 0));

    tbl_id = ctl_id ? I2CMasterCfg1_t : I2CMasterCfg0_t ;

    field_value = enable ? 1 : 0;
    cmd = DRV_IOW(tbl_id, I2CMasterCfg0_currentByteAcc_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_peri_i2c_set_bitmap_ctl(uint8 lchip, ctc_chip_i2c_read_t* p_i2c_para)
{
    sys_chip_i2c_slave_bitmap_t slave_bitmap = {0};
    I2CMasterBmpCfg0_m bitmap_ctl;
    uint32 cmd = 0;
    int32 ret = 0;

    sal_memset(&bitmap_ctl, 0, sizeof(I2CMasterBmpCfg0_m));

    if (SYS_CHIP_I2C_32BIT_DEV_ID > p_i2c_para->slave_dev_id)
    {
        slave_bitmap.slave_bitmap1 = (0x1 << p_i2c_para->slave_dev_id);
    }
    else if(SYS_CHIP_I2C_MAX_BITAMP > p_i2c_para->slave_dev_id)
    {
        slave_bitmap.slave_bitmap2 = (0x1 << (p_i2c_para->slave_dev_id - SYS_CHIP_I2C_32BIT_DEV_ID));
    }
    else
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_set_bitmap_ctl: slave_dev_id(%d) out of bound!\n", p_i2c_para->slave_dev_id);
        return CTC_E_INVALID_PARAM;
    }

    SetI2CMasterBmpCfg(p_i2c_para->ctl_id, &bitmap_ctl, &slave_bitmap);
    cmd = DRV_IOW(I2CMasterBmpCfg0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &bitmap_ctl);
    if (ret < 0)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_set_bitmap_ctl: write table fail! ctl_id %d, slave_bitmap1 %d, slave_bitmap2 %d\n",
                         p_i2c_para->ctl_id, slave_bitmap.slave_bitmap1, slave_bitmap.slave_bitmap2);
        return CTC_E_INVALID_CONFIG;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_peri_i2c_set_master_read(uint8 lchip, ctc_chip_i2c_read_t* p_i2c_para)
{
    int32 ret = 0;
    uint32 cmd = 0;
    I2CMasterReadCfg0_m master_rd;
    I2CMasterReadCtl0_m read_ctl;
    I2CMasterReadStatus0_m read_status;

    sal_memset(&master_rd, 0, sizeof(I2CMasterReadCfg0_m));
    sal_memset(&read_ctl, 0, sizeof(I2CMasterReadCtl0_m));

    /*Before trigger read, clear old status*/
    cmd = DRV_IOR(I2CMasterReadStatus0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &read_status);

    SetI2CMasterReadCfg(p_i2c_para->ctl_id, slaveAddr_f, &master_rd, p_i2c_para->dev_addr);
    SetI2CMasterReadCfg(p_i2c_para->ctl_id, offset_f, &master_rd, p_i2c_para->offset);
    SetI2CMasterReadCfg(p_i2c_para->ctl_id, length_f, &master_rd, p_i2c_para->length);
    cmd = DRV_IOW(I2CMasterReadCfg0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &master_rd);
    if (ret < 0)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_set_master_read: I2CMasterReadCfg error! %d\n", ret);
        return CTC_E_INVALID_CONFIG;
    }

    SetI2CMasterReadCtl(p_i2c_para->ctl_id, pollingSel_f, &read_ctl, 0);
    SetI2CMasterReadCtl(p_i2c_para->ctl_id, readEn_f, &read_ctl, 1);
    cmd = DRV_IOW(I2CMasterReadCtl0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &read_ctl);
    if (ret < 0)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_set_master_read: I2CMasterReadCtl error! %d\n", ret);
        return CTC_E_INVALID_CONFIG;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_peri_i2c_check_master_status(uint8 lchip, ctc_chip_i2c_read_t* p_i2c_para)
{
    uint32 cmd = 0;
    int32 ret = 0;
    uint32 timeout = 0;
    uint32 field_value = 0;
    I2CMasterStatus0_m master_status;

    sal_memset(&master_status, 0, sizeof(I2CMasterStatus0_m));
    timeout = 4 * p_i2c_para->length;

    cmd = DRV_IOR(I2CMasterStatus0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);

    do
    {
        sal_task_sleep(1);
        ret = DRV_IOCTL(lchip, 0, cmd, &master_status);
        if (ret < 0)
        {
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_check_master_status: I2CMasterStatus error! %d\n", ret);
            return CTC_E_INVALID_CONFIG;
        }
        GetI2CMasterStatus(p_i2c_para->ctl_id, triggerReadValid_f,
                        &master_status, field_value);
    }while ((field_value == 0) && (timeout--));

    if (0 == field_value)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "i2c master read cmd not done! line = %d\n",  __LINE__);
        return CTC_E_INVALID_CONFIG;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_peri_i2c_read_data(uint8 lchip, ctc_chip_i2c_read_t* p_i2c_para)
{
    uint8 index = 0;
    uint32 cmd = 0;
    int32 ret = 0;
    uint32 field_value = 0;
    I2CMasterDataMem0_m data_buf;

    sal_memset(&data_buf, 0, sizeof(I2CMasterDataMem0_m));

    cmd = DRV_IOR(I2CMasterDataMem0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);

    for (index = 0; index < p_i2c_para->length; index++)
    {
        ret = DRV_IOCTL(lchip, index, cmd, &data_buf);
        if (ret < 0)
        {
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_read_data: I2CMasterDataMem error! %d %d\n", ret, p_i2c_para->ctl_id);
            return CTC_E_INVALID_CONFIG;
        }

        if (p_i2c_para->ctl_id)
        {
            p_i2c_para->p_buf[index] = GetI2CMasterDataMem1(V, data_f, &data_buf);
        }
        else
        {
            p_i2c_para->p_buf[index] = GetI2CMasterDataMem0(V, data_f, &data_buf);
        }
    }

    /* clear read cmd */
    field_value = 0;
    cmd = DRV_IOW(I2CMasterReadCtl0_t + p_i2c_para->ctl_id, I2CMasterReadCtl0_readEn_f);
    ret = DRV_IOCTL(lchip, 0, cmd, &field_value);
    if (ret < 0)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_read_data: I2CMasterReadCtl error! %d %d\n", ret, p_i2c_para->ctl_id);
        return CTC_E_INVALID_CONFIG;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_peri_i2c_check_data(uint8 lchip, ctc_chip_i2c_read_t* p_i2c_para)

{
    uint32 cmd = 0;
    int32 ret = 0;
    uint32 arr_read_status[2] = {0};
    I2CMasterReadStatus0_m read_status;

    sal_memset(&read_status, 0, sizeof(I2CMasterReadStatus0_m));

    /*check read status */
    cmd = DRV_IOR(I2CMasterReadStatus0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &read_status);
    if (ret < 0)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_check_data: I2CMasterReadStatus error! %d %d\n", ret, p_i2c_para->ctl_id);
        return CTC_E_INVALID_CONFIG;
    }
    GetI2CMasterReadStatus(p_i2c_para->ctl_id, &read_status, arr_read_status);

    if ((0 != arr_read_status[0]) || (0 != arr_read_status[1]))
    {
        /* SYSTEM MODIFIED by tongzb for fiber access on TM, 2019-01-24 */
        /* p_i2c_para->p_buf[0] == 0xff : there is no ack from device, judge present info on system call */
        /* p_i2c_para->p_buf[0] == 0x0 : there is error ack from device */
        if (p_i2c_para->p_buf[0] == 0xff)
        {
            //SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_check_data: p_buf[0] == 0xff\n");
            //return CTC_E_INVALID_CONFIG;
        }
        else if (p_i2c_para->p_buf[0] == 0)
        {
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_check_data: p_buf[0] == 0\n");
            return CTC_E_INVALID_CONFIG;
        }
        else
        {
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_check_data: p_buf[0] %d\n", p_i2c_para->p_buf[0]);
            return CTC_E_INVALID_CONFIG;
        }
    }

    return CTC_E_NONE;
}

/**
 @brief i2c master for read sfp
*/
int32
sys_usw_peri_i2c_read(uint8 lchip, ctc_chip_i2c_read_t* p_i2c_para)
{
    int32 ret = 0;
    uint32 cmd = 0;
    uint32 field_value = 0;
    uint32 field_value1 = 0;
    I2CMasterReadCtl0_m read_ctl;

    CTC_PTR_VALID_CHECK(p_i2c_para);
    CTC_PTR_VALID_CHECK(p_i2c_para->p_buf);
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "read sfp, bitmap:0x%x addr:0x%x  offset:0x%x length:0x%x \n",
                     p_i2c_para->slave_bitmap, p_i2c_para->dev_addr, p_i2c_para->offset, p_i2c_para->length);

    if(0x3ff < p_i2c_para->dev_addr)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_usw_peri_i2c_read: out of bound!\n");
        return CTC_E_INTR_INVALID_PARAM;
    }

    if ((p_i2c_para->buf_length < p_i2c_para->length) || (0 == p_i2c_para->length)
        || ((SYS_CHIP_CONTROL0_ID != p_i2c_para->ctl_id)
            && (SYS_CHIP_CONTROL1_ID != p_i2c_para->ctl_id)))
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&read_ctl, 0, sizeof(I2CMasterReadCtl0_m));
    cmd = DRV_IOR(I2CMasterReadCtl0_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &read_ctl);
    DRV_IOR_FIELD(lchip, I2CMasterReadCtl0_t, I2CMasterReadCtl0_readEn_f, &field_value, &read_ctl);
    sal_memset(&read_ctl, 0, sizeof(I2CMasterReadCtl0_m));
    cmd = DRV_IOR(I2CMasterReadCtl1_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &read_ctl);
    DRV_IOR_FIELD(lchip, I2CMasterReadCtl1_t, I2CMasterReadCtl1_readEn_f, &field_value1, &read_ctl);
    if(field_value || field_value1)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_usw_peri_i2c_read: i2c-scan is running!\n");
        return CTC_E_INVALID_PARAM;
    }

    SYS_PERI_I2C_LOCK(lchip);

    if (p_i2c_para->access_switch)
    {
        p_i2c_para->i2c_switch_id = 0;
        p_i2c_para->length = 1;
        CTC_ERROR_GOTO(_sys_usw_peri_set_i2c_byte_access(lchip, p_i2c_para->ctl_id, TRUE), ret, EXIT);
    }

    CTC_ERROR_GOTO(_sys_usw_peri_i2c_set_switch(lchip, p_i2c_para->ctl_id, p_i2c_para->i2c_switch_id),
        ret, error_0);

    CTC_ERROR_GOTO(_sys_usw_peri_i2c_set_bitmap_ctl(lchip, p_i2c_para),
        ret, error_0);

    CTC_ERROR_GOTO(_sys_usw_peri_i2c_set_master_read(lchip, p_i2c_para),
        ret, error_0);

    CTC_ERROR_GOTO(_sys_usw_peri_i2c_check_master_status(lchip, p_i2c_para),
        ret, error_0);

    CTC_ERROR_GOTO(_sys_usw_peri_i2c_read_data(lchip, p_i2c_para),
        ret, error_0);

    /*check read status */
    CTC_ERROR_GOTO( _sys_usw_peri_i2c_check_data(lchip, p_i2c_para),
        ret, error_0);

error_0:
    if (p_i2c_para->access_switch)
    {
        CTC_ERROR_GOTO(_sys_usw_peri_set_i2c_byte_access(lchip, p_i2c_para->ctl_id, FALSE), ret, EXIT);
    }
EXIT:
    SYS_PERI_I2C_UNLOCK(lchip);
    return ret;
}

STATIC int32
_sys_usw_peri_i2c_set_write_bitmap_ctl(uint8 lchip, ctc_chip_i2c_write_t* p_i2c_para)
{
    sys_chip_i2c_slave_bitmap_t slave_bitmap = {0};
    I2CMasterBmpCfg0_m bitmap_ctl;
    uint32 cmd = 0;
    int32 ret = 0;

    sal_memset(&bitmap_ctl, 0, sizeof(I2CMasterBmpCfg0_m));

    if (SYS_CHIP_I2C_32BIT_DEV_ID > p_i2c_para->slave_id)
    {
        slave_bitmap.slave_bitmap1 = (0x1 << p_i2c_para->slave_id);
    }
    else if(SYS_CHIP_I2C_MAX_BITAMP > p_i2c_para->slave_id)
    {
        slave_bitmap.slave_bitmap2 = (0x1 << (p_i2c_para->slave_id - SYS_CHIP_I2C_32BIT_DEV_ID));
    }
    else
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_set_bitmap_ctl: slave_id(%d) out of bound!\n", p_i2c_para->slave_id);
        return CTC_E_INVALID_PARAM;
    }

    SetI2CMasterBmpCfg(p_i2c_para->ctl_id, &bitmap_ctl, &slave_bitmap);

    cmd = DRV_IOW(I2CMasterBmpCfg0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &bitmap_ctl);
    if (ret < 0)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_set_write_bitmap_ctl: I2CMasterBmpCfg error! %d %d\n",
                         p_i2c_para->ctl_id, ret);
        return CTC_E_INVALID_CONFIG;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_peri_i2c_set_write_master(uint8 lchip, ctc_chip_i2c_write_t* p_i2c_para)
{
    int32 ret = 0;
    uint32 cmd = 0;
    uint32 field_value = 0;
    I2CMasterReadCfg0_m master_rd;
    I2CMasterStatus0_m master_status;

    sal_memset(&master_rd, 0, sizeof(I2CMasterReadCfg0_m));
    sal_memset(&master_status, 0, sizeof(I2CMasterStatus0_m));

    /* check status */
    if (p_i2c_para->ctl_id)
    {
        cmd = DRV_IOR(I2CMasterReadCtl1_t, I2CMasterReadCtl1_readEn_f);
    }
    else
    {
        cmd = DRV_IOR(I2CMasterReadCtl0_t, I2CMasterReadCtl0_readEn_f);
    }

    ret = DRV_IOCTL(lchip, 0, cmd, &field_value);
    if (ret < 0)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_set_write_master: I2CMasterReadCtl error! %d %d\n",
                         p_i2c_para->ctl_id, ret);
        return CTC_E_INVALID_CONFIG;
    }

    if (field_value != 0)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_set_write_master: field_value %d \n", field_value);
        return CTC_E_INVALID_CONFIG;
    }

#if 0
    /*
        hardware is not sure to set pollingDone_f 1
    */
    cmd = DRV_IOR(I2CMasterStatus0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(p_i2c_para->lchip, 0, cmd, &master_status);
    if (ret < 0)
    {
        return DRV_E_I2C_MASTER_CMD_ERROR;
    }

    GetI2CMasterStatus(p_i2c_para->ctl_id, pollingDone_f, &master_status, polling_done);

    if (polling_done != 1)
    {
        /* chip init status polling_done is set */
        DRV_DBG_INFO("i2c master write check polling fail! line = %d\n",  __LINE__);
        return DRV_E_I2C_MASTER_POLLING_NOT_DONE;
    }
#endif

    SetI2CMasterReadCfg(p_i2c_para->ctl_id, slaveAddr_f, &master_rd, p_i2c_para->dev_addr);
    SetI2CMasterReadCfg(p_i2c_para->ctl_id, offset_f, &master_rd, p_i2c_para->offset);
    cmd = DRV_IOW(I2CMasterReadCfg0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &master_rd);
    if (ret < 0)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_set_write_master: I2CMasterReadCfg error! %d %d\n", p_i2c_para->ctl_id, ret);
        return CTC_E_INVALID_CONFIG;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_peri_i2c_write_data(uint8 lchip, ctc_chip_i2c_write_t* p_i2c_para)
{
    uint32 cmd = 0;
    int32 ret = 0;
    I2CMasterWrCfg0_m master_wr;

    sal_memset(&master_wr, 0, sizeof(I2CMasterWrCfg0_m));

    SetI2CMasterWrCfg(p_i2c_para->ctl_id, wrData_f, &master_wr, p_i2c_para->data);
    SetI2CMasterWrCfg(p_i2c_para->ctl_id, wrEn_f, &master_wr, 1);
    cmd = DRV_IOW(I2CMasterWrCfg0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);
    ret = DRV_IOCTL(lchip, 0, cmd, &master_wr);
    if (ret < 0)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_write_data: I2CMasterWrCfg error! %d %d\n", p_i2c_para->ctl_id, ret);
        return CTC_E_INVALID_CONFIG;
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_peri_i2c_check_write_finish(uint8 lchip, ctc_chip_i2c_write_t* p_i2c_para)
{
    uint32 cmd_cfg = 0;
    uint32 cmd_dbg = 0;
    int32 ret = 0;
    uint32 timeout = 4;
    I2CMasterWrCfg0_m master_wr;
    I2CMasterDebugState0_m master_dbg;
    uint8 i2c_wr_en = 0;
    uint32 i2c_status = 0;

    sal_memset(&master_wr, 0, sizeof(I2CMasterWrCfg0_m));
    sal_memset(&master_dbg, 0, sizeof(I2CMasterDebugState0_m));

    /* wait write op done */
    cmd_cfg = DRV_IOR(I2CMasterWrCfg0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);
    cmd_dbg = DRV_IOR(I2CMasterDebugState0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);

    do
    {
        sal_task_sleep(1);
        ret = DRV_IOCTL(lchip, 0, cmd_cfg, &master_wr);
        if (ret < 0)
        {
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_check_write_finish: I2CMasterWrCfg error! %d %d\n", p_i2c_para->ctl_id, ret);
            return CTC_E_INVALID_CONFIG;
        }
        GetI2CMasterWrCfg(p_i2c_para->ctl_id, wrEn_f, &master_wr, i2c_wr_en );

        ret = DRV_IOCTL(lchip, 0, cmd_dbg, &master_dbg);
        if (ret < 0)
        {
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_i2c_check_write_finish: I2CMasterDebugState error! %d %d\n", p_i2c_para->ctl_id, ret);
            return CTC_E_INVALID_CONFIG;
        }
        GetI2CMasterDebugState(p_i2c_para->ctl_id, i2cAccState_f, &master_dbg, i2c_status);
    }while (((0 != i2c_wr_en) || (0 != i2c_status)) && (--timeout));

    if (((0 != i2c_wr_en) || (0 != i2c_status)))
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "i2c master write cmd not done! line = %d\n \
                    wrEn:%u, i2cAccState:%u \n",  __LINE__, i2c_wr_en, i2c_status);
        return CTC_E_INVALID_CONFIG;
    }
    return CTC_E_NONE;
}

/**
 @brief i2c master for write sfp
*/
int32
sys_usw_peri_i2c_write(uint8 lchip, ctc_chip_i2c_write_t* p_i2c_para)
{
    int32 ret = 0;

    CTC_PTR_VALID_CHECK(p_i2c_para);
    SYS_PERI_INIT_CHECK(lchip);
    if (((SYS_CHIP_CONTROL0_ID != p_i2c_para->ctl_id)
            && (SYS_CHIP_CONTROL1_ID != p_i2c_para->ctl_id)))
    {
        return CTC_E_INVALID_PARAM;
    }

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "write sfp, slave_id:0x%x addr:0x%x  offset:0x%x data:0x%x \n",
                     p_i2c_para->slave_id, p_i2c_para->dev_addr, p_i2c_para->offset, p_i2c_para->data);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "chip-id:%u \n",
                     p_i2c_para->lchip);

    CTC_PTR_VALID_CHECK(p_i2c_para);

    SYS_PERI_I2C_LOCK(lchip);
    if (p_i2c_para->access_switch)
    {
        p_i2c_para->i2c_switch_id = 0;
        CTC_ERROR_GOTO(_sys_usw_peri_set_i2c_byte_access(lchip, p_i2c_para->ctl_id, TRUE), ret, EXIT);
    }

    CTC_ERROR_GOTO(_sys_usw_peri_i2c_set_switch(lchip, p_i2c_para->ctl_id, p_i2c_para->i2c_switch_id),
        ret, error_0);

    CTC_ERROR_GOTO(_sys_usw_peri_i2c_set_write_bitmap_ctl(lchip, p_i2c_para),
        ret, error_0);

    CTC_ERROR_GOTO(_sys_usw_peri_i2c_set_write_master(lchip, p_i2c_para),
        ret, error_0);

    CTC_ERROR_GOTO(_sys_usw_peri_i2c_write_data(lchip, p_i2c_para),
        ret, error_0);

    CTC_ERROR_GOTO(_sys_usw_peri_i2c_check_write_finish(lchip, p_i2c_para),
        ret, error_0);

error_0:
    if (p_i2c_para->access_switch)
    {
        CTC_ERROR_GOTO(_sys_usw_peri_set_i2c_byte_access(lchip, p_i2c_para->ctl_id, FALSE), ret, EXIT);
    }
EXIT:
    SYS_PERI_I2C_UNLOCK(lchip);
    return CTC_E_NONE;
}


/**
 @brief set i2c polling read para
*/
STATIC int32
_sys_usw_peri_set_i2c_scan_para(uint8 lchip, ctc_chip_i2c_scan_t* p_i2c_para)
{
    uint32 cmd = 0;
    uint32 field_value = 0;
    I2CMasterReadCfg0_m master_rd;
    I2CMasterReadCtl0_m read_ctl;
    I2CMasterBmpCfg0_m bitmap_ctl;
    I2CMasterPollingCfg0_m polling_cfg;  /*check*/
    uint8 slave_num = 0;
    uint8 index = 0;

    CTC_PTR_VALID_CHECK(p_i2c_para);

    sal_memset(&master_rd, 0, sizeof(I2CMasterReadCfg0_m));
    sal_memset(&read_ctl, 0, sizeof(I2CMasterReadCtl0_m));
    sal_memset(&bitmap_ctl, 0, sizeof(I2CMasterBmpCfg0_m));
    sal_memset(&polling_cfg, 0, sizeof(I2CMasterPollingCfg0_m));

    if((1 < p_i2c_para->ctl_id) ||
       (0x3ff < p_i2c_para->dev_addr) ||
       (0xffff < p_i2c_para->slave_bitmap[1]))
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_set_i2c_scan_para: out of bound! %d %d %d %d\n",
                         p_i2c_para->ctl_id, p_i2c_para->dev_addr, p_i2c_para->length, p_i2c_para->offset);
        return CTC_E_INVALID_PARAM;
    }

    CTC_ERROR_RETURN(_sys_usw_peri_i2c_set_switch(lchip, p_i2c_para->ctl_id, p_i2c_para->i2c_switch_id));

    if (SYS_CHIP_FLAG_ISSET(p_i2c_para->op_flag, SYS_CHIP_SFP_BITMAP_OP)
        || SYS_CHIP_FLAG_ISZERO(p_i2c_para->op_flag))
    {
        SetI2CMasterBmpCfg(p_i2c_para->ctl_id, &bitmap_ctl, p_i2c_para->slave_bitmap);
        cmd = DRV_IOW(I2CMasterBmpCfg0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &bitmap_ctl));
        for (index = 0; index < 32; index++)
        {
            if (CTC_IS_BIT_SET(p_i2c_para->slave_bitmap[p_i2c_para->ctl_id], index))
            {
                slave_num++;
            }
        }
    }

    if (SYS_CHIP_FLAG_ISSET(p_i2c_para->op_flag, SYS_CHIP_SFP_SCAN_REG_OP)
        || SYS_CHIP_FLAG_ISZERO(p_i2c_para->op_flag))
    {
        if ((slave_num*p_i2c_para->length) > SYS_CHIP_I2C_READ_MAX_LENGTH)
        {
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_set_i2c_scan_para: length over 384, salve_num %d length %d\n",
                     slave_num, p_i2c_para->length);

            return CTC_E_INVALID_PARAM;
        }

        SetI2CMasterReadCfg(p_i2c_para->ctl_id, slaveAddr_f,
                        &master_rd, p_i2c_para->dev_addr);
        SetI2CMasterReadCfg(p_i2c_para->ctl_id, length_f,
                        &master_rd, p_i2c_para->length);
        SetI2CMasterReadCfg(p_i2c_para->ctl_id, offset_f,
                        &master_rd, p_i2c_para->offset);
        cmd = DRV_IOW(I2CMasterReadCfg0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &master_rd));
    }

    if (SYS_CHIP_FLAG_ISSET(p_i2c_para->op_flag, SYS_CHIP_SFP_INTERVAL_OP)
        || SYS_CHIP_FLAG_ISZERO(p_i2c_para->op_flag))
    {
        SetI2CMasterPollingCfg(p_i2c_para->ctl_id, pollingInterval_f,
                            &polling_cfg, p_i2c_para->interval);
        cmd = DRV_IOW(I2CMasterPollingCfg0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &polling_cfg));
    }

    field_value = 1;
    if (p_i2c_para->ctl_id)
    {
        cmd = DRV_IOW(I2CMasterReadCtl1_t, I2CMasterReadCtl1_pollingSel_f);
    }
    else
    {
        cmd = DRV_IOW(I2CMasterReadCtl0_t, I2CMasterReadCtl0_pollingSel_f);
    }
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    return CTC_E_NONE;
}

/**
 @brief get i2c polling read para
*/
STATIC int32
_sys_usw_peri_get_i2c_scan_para(uint8 lchip, ctc_chip_i2c_scan_t* p_i2c_para)
{
    uint32 cmd = 0;
    uint32 field_value = 0;
    I2CMasterReadCfg0_m master_rd;
    I2CMasterPollingCfg0_m polling_cfg;  /*check*/

    CTC_PTR_VALID_CHECK(p_i2c_para);

    sal_memset(&master_rd, 0, sizeof(I2CMasterReadCfg0_m));
    sal_memset(&polling_cfg, 0, sizeof(I2CMasterPollingCfg0_m));

    if(1 < p_i2c_para->ctl_id)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "_sys_usw_peri_get_i2c_scan_para: out of bound! %d \n",
                         p_i2c_para->ctl_id);
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOW(I2CMasterCfg0_t + p_i2c_para->ctl_id, I2CMasterCfg0_switchEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    p_i2c_para->i2c_switch_id = field_value;

    if (SYS_CHIP_FLAG_ISSET(p_i2c_para->op_flag, SYS_CHIP_SFP_BITMAP_OP)
        || SYS_CHIP_FLAG_ISZERO(p_i2c_para->op_flag))
    {
        cmd = DRV_IOR(I2CMasterBmpCfg0_t + p_i2c_para->ctl_id, I2CMasterBmpCfg0_slaveBitmap_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, p_i2c_para->slave_bitmap));
    }

    if (SYS_CHIP_FLAG_ISSET(p_i2c_para->op_flag, SYS_CHIP_SFP_SCAN_REG_OP)
        || SYS_CHIP_FLAG_ISZERO(p_i2c_para->op_flag))
    {
        cmd = DRV_IOR(I2CMasterReadCfg0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &master_rd));
        DRV_IOR_FIELD(lchip, I2CMasterReadCfg0_t + p_i2c_para->ctl_id, I2CMasterReadCfg0_slaveAddr_f,
                        &field_value, &master_rd);
        p_i2c_para->dev_addr = field_value;
        DRV_IOR_FIELD(lchip, I2CMasterReadCfg0_t + p_i2c_para->ctl_id, I2CMasterReadCfg0_length_f,
                        &field_value, &master_rd);
        p_i2c_para->length = field_value;
        DRV_IOR_FIELD(lchip, I2CMasterReadCfg0_t + p_i2c_para->ctl_id, I2CMasterReadCfg0_offset_f,
                        &field_value, &master_rd);
        p_i2c_para->offset = field_value;
    }

    if (SYS_CHIP_FLAG_ISSET(p_i2c_para->op_flag, SYS_CHIP_SFP_INTERVAL_OP)
        || SYS_CHIP_FLAG_ISZERO(p_i2c_para->op_flag))
    {
        cmd = DRV_IOR(I2CMasterPollingCfg0_t + p_i2c_para->ctl_id, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &polling_cfg));
        DRV_IOR_FIELD(lchip, I2CMasterPollingCfg0_t + p_i2c_para->ctl_id, I2CMasterPollingCfg0_pollingInterval_f,
                        &field_value, &polling_cfg);
        p_i2c_para->interval = field_value;
    }

    return CTC_E_NONE;
}

/**
 @brief i2c master for polling read
*/
int32
sys_usw_peri_set_i2c_scan_para(uint8 lchip, ctc_chip_i2c_scan_t* p_i2c_para)
{
    SYS_PERI_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_i2c_para);

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "scan sfp, op_flag:0x%x, bitmap:0x%x addr:0x%x  offset:0x%x length:0x%x intreval:0x%x\n",
                     p_i2c_para->op_flag, p_i2c_para->slave_bitmap[0], p_i2c_para->dev_addr, p_i2c_para->offset, p_i2c_para->length, p_i2c_para->interval);

    CTC_ERROR_RETURN(_sys_usw_peri_set_i2c_scan_para(lchip, p_i2c_para));

    return CTC_E_NONE;
}

/**
 @brief get i2c master for polling read
*/
int32
sys_usw_peri_get_i2c_scan_para(uint8 lchip, ctc_chip_i2c_scan_t* p_i2c_para)
{
    SYS_PERI_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_i2c_para);

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(_sys_usw_peri_get_i2c_scan_para(lchip, p_i2c_para));

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "scan sfp, op_flag:0x%x, bitmap:0x%x addr:0x%x  offset:0x%x length:0x%x intreval:0x%x\n",
                     p_i2c_para->op_flag, p_i2c_para->slave_bitmap[0], p_i2c_para->dev_addr, p_i2c_para->offset, p_i2c_para->length, p_i2c_para->interval);

    return CTC_E_NONE;
}

/**
 @brief i2c master for polling read start
*/
int32
sys_usw_peri_set_i2c_scan_en(uint8 lchip, bool enable)
{
    uint32 cmd = 0;
    uint32 field_value = (TRUE == enable) ? 1 : 0;
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Set Scan Enable enable:%d\n", ((TRUE == enable) ? 1 : 0));

    cmd = DRV_IOW(I2CMasterReadCtl0_t, I2CMasterReadCtl0_readEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    cmd = DRV_IOW(I2CMasterReadCtl1_t, I2CMasterReadCtl1_readEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    return CTC_E_NONE;
}

/**
 @brief get i2c master for polling read start
*/
int32
sys_usw_peri_get_i2c_scan_en(uint8 lchip, bool* enable)
{
    uint32 cmd = 0;
    uint32 field_value = 0;

    SYS_PERI_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    cmd = DRV_IOR(I2CMasterReadCtl0_t, I2CMasterReadCtl0_readEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Get Scan Enable enable:%d\n", field_value);

    *enable = field_value;

    return CTC_E_NONE;
}

/**
 @brief interface for read i2c databuf, usual used for i2c master for polling read
*/
int32
sys_usw_peri_read_i2c_buf(uint8 lchip, ctc_chip_i2c_scan_read_t *p_i2c_scan_read)
{
    uint32 index = 0;
    uint32 cmd = 0;
    I2CMasterDataMem0_m data_buf;
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "read sfp buf, lchip:%d len:%d\n",
                                                lchip, p_i2c_scan_read->len);

    CTC_PTR_VALID_CHECK(p_i2c_scan_read);
    CTC_PTR_VALID_CHECK(p_i2c_scan_read->p_buf);

    if (p_i2c_scan_read->len > SYS_CHIP_I2C_READ_MAX_LENGTH)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_usw_peri_read_i2c_buf: p_i2c_scan_read->len (%d) out of bound!\n", p_i2c_scan_read->len);
        return CTC_E_INVALID_PARAM;
    }

    if (1 < p_i2c_scan_read->ctl_id)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "sys_usw_peri_read_i2c_buf: ctl_id (%d) out of bound!\n", p_i2c_scan_read->ctl_id);
        return CTC_E_INVALID_PARAM;
    }

    if (TRUE != sys_usw_chip_is_local(lchip, p_i2c_scan_read->gchip))
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Invalid Local chip id \n");
			return CTC_E_INVALID_CHIP_ID;

    }

    sal_memset(&data_buf, 0, sizeof(I2CMasterDataMem0_m));

    cmd = DRV_IOR(I2CMasterDataMem0_t + p_i2c_scan_read->ctl_id, DRV_ENTRY_FLAG);

    for (index = 0; index < p_i2c_scan_read->len; index++)
    {
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, index, cmd, &data_buf));

        p_i2c_scan_read->p_buf[index] = GetI2CMasterDataMem0(V, data_f, &data_buf);
    }


    return CTC_E_NONE;
}

int32
sys_usw_peri_set_i2c_clock(uint8 lchip, uint8 ctl_id, uint16 freq)
{
    uint32 cmd = 0;
    uint32 field_value = 0;

    SYS_PERI_INIT_CHECK(lchip);
    CTC_MAX_VALUE_CHECK(ctl_id, 1);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if ((SDK_WORK_PLATFORM != 0) || (DRV_IS_DUET2(lchip)))
    {
        return CTC_E_NOT_SUPPORT;
    }

    field_value = 500*1000/freq;
    if (0 == ctl_id)
    {
        cmd = DRV_IOW(I2CMasterCfg0_t, I2CMasterCfg0_clkDiv_f);
    }
    else
    {
        cmd = DRV_IOW(I2CMasterCfg1_t, I2CMasterCfg0_clkDiv_f);
    }
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    return CTC_E_NONE;
}

int32
sys_usw_peri_get_i2c_clock(uint8 lchip, uint8 ctl_id, uint16* freq)
{
    uint32 cmd = 0;
    uint32 field_val = 0;

    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if ((SDK_WORK_PLATFORM != 0) || (DRV_IS_DUET2(lchip)))
    {
        return CTC_E_NOT_SUPPORT;
    }

    if (0 == ctl_id)
    {
        cmd = DRV_IOR(I2CMasterCfg0_t, I2CMasterCfg0_clkDiv_f);
    }
    else
    {
        cmd = DRV_IOR(I2CMasterCfg1_t, I2CMasterCfg0_clkDiv_f);
    }
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    *freq = 500*1000/field_val;

    return CTC_E_NONE;
}

#define __MAC_LED_INTERFACE__

/**
 @brief mac led interface
*/
int32
sys_usw_peri_set_mac_led_mode(uint8 lchip, ctc_chip_led_para_t* p_led_para, ctc_chip_mac_led_type_t led_type, uint8 inner)
{
    CTC_PTR_VALID_CHECK(p_led_para);
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_set_mac_led_mode(lchip, p_led_para, led_type, inner));

    return CTC_E_NONE;
}

/**
 @brief mac led interface for mac and led mapping
*/
int32
sys_usw_peri_set_mac_led_mapping(uint8 lchip, ctc_chip_mac_led_mapping_t* p_led_map)
{
    SYS_PERI_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_led_map);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "mac num:%d \n", p_led_map->mac_led_num);

    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_set_mac_led_mapping(lchip, p_led_map));

    return CTC_E_NONE;
}

/**
 @brief begin mac led function
*/
int32
sys_usw_peri_set_mac_led_en(uint8 lchip, bool enable)
{
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "mac led Enable enable:%d\n", ((TRUE == enable) ? 1 : 0));

    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_set_mac_led_en(lchip, enable));

    return CTC_E_NONE;
}

/**
 @brief get mac led function
*/
int32
sys_usw_peri_get_mac_led_en(uint8 lchip, bool* enable)
{
    SYS_PERI_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(enable);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_get_mac_led_en(lchip, enable));

    return CTC_E_NONE;
}

int32
sys_usw_peri_set_mac_led_clock(uint8 lchip, uint16 freq)
{
    uint32 cmd = 0;
    uint32 field_value = 0;
    uint32 core_freq = 0;

    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if ((SDK_WORK_PLATFORM != 0) || (DRV_IS_DUET2(lchip)))
    {
        return CTC_E_NOT_SUPPORT;
    }

    core_freq = sys_usw_get_core_freq(lchip, 0);

    field_value = (core_freq*1000/freq);
    if(0xffff < field_value)
    {
        return CTC_E_INVALID_PARAM;
    }

    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgClkLedDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    /*
        #2, Toggle the clockLed divider reset
            cfg SupDskCfg.cfgResetClkLedDiv = 1'b1;
            cfg SupDskCfg.cfgResetClkLedDiv = 1'b0;
    */
    field_value = 1;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgResetClkLedDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));
    field_value = 0;
    cmd = DRV_IOW(SupDskCfg_t, SupDskCfg_cfgResetClkLedDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_value));

    return CTC_E_NONE;
}

int32
sys_usw_peri_get_mac_led_clock(uint8 lchip, uint16* freq)
{
    uint32 cmd = 0;
    uint32 field_val = 0;
    uint32 core_freq = 0;

    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if ((SDK_WORK_PLATFORM != 0) || (DRV_IS_DUET2(lchip)))
    {
        return CTC_E_NOT_SUPPORT;
    }

    core_freq = sys_usw_get_core_freq(lchip, 0);

    cmd = DRV_IOR(SupDskCfg_t, SupDskCfg_cfgClkLedDiv_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_val));

    *freq = (core_freq*1000)/field_val;

    return CTC_E_NONE;
}

STATIC int32
sys_usw_peri_set_macled_blink_interval(uint8 lchip, uint32* interval)
{

    uint32 cmd = 0;
    uint32 value = 0;

    if(SDK_WORK_PLATFORM != 0)
    {
        return CTC_E_NOT_SUPPORT;
    }
    CTC_MAX_VALUE_CHECK(*interval,SYS_USW_MACLED_BLINK_INTERVAL_MAX);
    value = *interval;
    /* sampleInterval is 6.5 times as refreshInterval,blinkOffInterval is 5.2 times as refreshInterval */
    if(DRV_IS_DUET2(lchip))
    {
        cmd = DRV_IOW(LedBlinkCfg0_t, LedBlinkCfg0_blinkOffInterval_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOW(LedBlinkCfg1_t, LedBlinkCfg1_blinkOffInterval_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOW(LedBlinkCfg0_t, LedBlinkCfg0_blinkOnInterval_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOW(LedBlinkCfg1_t, LedBlinkCfg1_blinkOnInterval_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        value = *interval / 5;
        cmd = DRV_IOW(LedRefreshInterval0_t, LedRefreshInterval0_refreshInterval_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOW(LedRefreshInterval1_t, LedRefreshInterval1_refreshInterval_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        value = value * 6 + value / 2;
        cmd = DRV_IOW(LedSampleInterval0_t, LedSampleInterval0_sampleInterval_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOW(LedSampleInterval1_t, LedSampleInterval1_sampleInterval_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    }
    else
    {
        cmd = DRV_IOW(LedBlinkCfg_t, LedBlinkCfg_blinkOffInterval_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        cmd = DRV_IOW(LedBlinkCfg_t, LedBlinkCfg_blinkOnInterval_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        value = *interval / 5;
        cmd = DRV_IOW(LedRefreshInterval_t, LedRefreshInterval_refreshInterval_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        value = value * 6 + value / 2;
        cmd = DRV_IOW(LedSampleInterval_t, LedSampleInterval_sampleInterval_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    }

    return CTC_E_NONE;
}

STATIC int32
sys_usw_peri_get_macled_blink_interval(uint8 lchip, uint32* interval)
{
    uint32 cmd = 0;
    uint32 table_id = 0;

    if(SDK_WORK_PLATFORM != 0)
    {
        return CTC_E_NOT_SUPPORT;
    }
    table_id = DRV_IS_DUET2(lchip)?LedBlinkCfg0_t:LedBlinkCfg_t;
    cmd = DRV_IOR(table_id, LedBlinkCfg0_blinkOffInterval_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, interval));

    return CTC_E_NONE;
}

#define __GPIO_INTERFACE__
/**
 @brief gpio interface
*/
int32
sys_usw_peri_set_gpio_mode(uint8 lchip, uint8 gpio_id, ctc_chip_gpio_mode_t mode)
{
    SYS_PERI_INIT_CHECK(lchip);

    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_set_gpio_mode(lchip, gpio_id, mode));

    return CTC_E_NONE;
}

/**
 @brief gpio output
*/
int32
sys_usw_peri_set_gpio_output(uint8 lchip, uint8 gpio_id, uint8 out_para)
{
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "set gpio out, gpio_id:%d out_para:%d\n", gpio_id, out_para);

    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_set_gpio_output(lchip, gpio_id, out_para));

    return CTC_E_NONE;
}


/**
 @brief gpio input
*/
int32
sys_usw_peri_get_gpio_input(uint8 lchip, uint8 gpio_id, uint8* in_value)
{
    SYS_PERI_INIT_CHECK(lchip);

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_get_gpio_input(lchip, gpio_id, in_value));

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "get gpio in, gpio_id:%d in_para:%d\n", gpio_id, *in_value);

    return CTC_E_NONE;
}

int32
sys_usw_peri_set_gpio_cfg(uint8 lchip, ctc_chip_gpio_cfg_t* p_cfg)
{
    uint32 cmd = 0;
    uint32 tbl_debce_id = 0;
    uint32 field_debce_id = 0;
    uint32 field_debce_val = 0;
    uint32 tbl_level_id = 0;
    uint32 field_level_id = 0;
    uint32 field_level_val = 0;
    uint32 gpio_act_id = 0;
    uint8 gpio_id = 0;

    SYS_PERI_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_cfg);

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "set gpio cfg, gpio_id:%d debounce:%d level_edge_val: %d\n", gpio_id, p_cfg->debounce_en, p_cfg->level_edge_val);

    if (DRV_IS_DUET2(lchip))
    {
        return CTC_E_NOT_SUPPORT;
    }

    if (gpio_act_id >= SYS_PERI_HS_MAX_GPIO_ID)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "gpio_id:%d over max gpio id\n", gpio_id);
        return CTC_E_INVALID_PARAM;
    }

    gpio_id = p_cfg->gpio_id;
    if (SYS_PERI_GPIO_IS_HS(gpio_id))
    {
        gpio_act_id = gpio_id-SYS_PERI_MAX_GPIO_ID;
        tbl_debce_id = GpioHsDebCtl_t;
        field_debce_id = GpioHsDebCtl_cfgHsDebEn_f;
        tbl_level_id = GpioHsIntrLevel_t;
        field_level_id = GpioHsIntrLevel_cfgHsIntrLevel_f;
    }
    else
    {
        gpio_act_id = gpio_id;
        tbl_debce_id = GpioDebCtl_t;
        field_debce_id = GpioDebCtl_cfgDebEn_f;
        tbl_level_id = GpioIntrLevel_t;
        field_level_id = GpioIntrLevel_cfgIntrLevel_f;
    }

    cmd = DRV_IOR(tbl_debce_id, field_debce_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_debce_val));
    cmd = DRV_IOR(tbl_level_id, field_level_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_level_val));

    if (p_cfg->debounce_en)
    {
        CTC_BIT_SET(field_debce_val, gpio_act_id);
    }
    else
    {
        CTC_BIT_UNSET(field_debce_val, gpio_act_id);
    }

    if (p_cfg->level_edge_val)
    {
        CTC_BIT_SET(field_level_val, gpio_act_id);
    }
    else
    {
        CTC_BIT_UNSET(field_level_val, gpio_act_id);
    }

    cmd = DRV_IOW(tbl_debce_id, field_debce_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_debce_val));
    cmd = DRV_IOW(tbl_level_id, field_level_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_level_val));

    return CTC_E_NONE;
}

int32
sys_usw_peri_get_gpio_cfg(uint8 lchip, ctc_chip_gpio_cfg_t* p_cfg)
{
    uint32 cmd = 0;
    uint32 tbl_debce_id = 0;
    uint32 field_debce_id = 0;
    uint32 field_debce_val = 0;
    uint32 tbl_level_id = 0;
    uint32 field_level_id = 0;
    uint32 field_level_val = 0;
    uint32 gpio_act_id = 0;
    uint8 gpio_id = 0;

    SYS_PERI_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_cfg);

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if (DRV_IS_DUET2(lchip))
    {
        return CTC_E_NOT_SUPPORT;
    }

    if (gpio_act_id >= SYS_PERI_HS_MAX_GPIO_ID)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "gpio_id:%d over max gpio id\n", gpio_id);
        return CTC_E_INVALID_PARAM;
    }

    gpio_id = p_cfg->gpio_id;
    if (SYS_PERI_GPIO_IS_HS(gpio_id))
    {
        gpio_act_id = gpio_id-SYS_PERI_MAX_GPIO_ID;
        tbl_debce_id = GpioHsDebCtl_t;
        field_debce_id = GpioHsDebCtl_cfgHsDebEn_f;
        tbl_level_id = GpioHsIntrLevel_t;
        field_level_id = GpioHsIntrLevel_cfgHsIntrLevel_f;
    }
    else
    {
        gpio_act_id = gpio_id;
        tbl_debce_id = GpioDebCtl_t;
        field_debce_id = GpioDebCtl_cfgDebEn_f;
        tbl_level_id = GpioIntrLevel_t;
        field_level_id = GpioIntrLevel_cfgIntrLevel_f;
    }

    cmd = DRV_IOR(tbl_debce_id, field_debce_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_debce_val));
    cmd = DRV_IOR(tbl_level_id, field_level_id);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &field_level_val));

    if (CTC_IS_BIT_SET(field_debce_val, gpio_act_id))
    {
        p_cfg->debounce_en = 1;
    }
    else
    {
        p_cfg->debounce_en = 0;;
    }

    if (CTC_IS_BIT_SET(field_level_val, gpio_act_id))
    {
        p_cfg->level_edge_val = 1;
    }
    else
    {
        p_cfg->level_edge_val = 0;
    }

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "get gpio cfg, gpio_id:%d debounce:%d level_edge_val: %d\n", gpio_id, p_cfg->debounce_en, p_cfg->level_edge_val);

    return CTC_E_NONE;
}

#define __OTHER_INTERFACE__

/**
 @brief get serdes info from lport
*/
int32
sys_usw_peri_get_serdes_info(uint8 lchip, uint8 lport, uint8* macro_idx, uint8* link_idx)
{
    SYS_PERI_INIT_CHECK(lchip);
#ifdef NEVER
    if (0 == SDK_WORK_PLATFORM)
    {
        uint8 sgmac_id = 0;
        uint8 serdes_id = 0;

        if (lport >= 60)
        {
            return CTC_E_INVALID_PARAM;
        }

        if (lport < 48)
        {
            /* gmac serdes info */
            CTC_ERROR_RETURN(drv_usw_get_gmac_info(lchip, lport, DRV_CHIP_MAC_SERDES_INFO, (void*)&serdes_id));
        }
        else
        {
            /* sgmac serdes info */
            sgmac_id = lport - 48;
            CTC_ERROR_RETURN(drv_usw_get_sgmac_info(lchip, sgmac_id, DRV_CHIP_MAC_SERDES_INFO, (void*)&serdes_id));
        }

        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "serdes_id:%d \n",serdes_id);

        /* get macro idx */
        CTC_ERROR_RETURN(drv_usw_get_serdes_info(lchip, serdes_id, DRV_CHIP_SERDES_HSSID_INFO, (void*)macro_idx));

        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "macro_idx:%d \n",*macro_idx);

        /* get link idx */
        CTC_ERROR_RETURN(drv_usw_get_serdes_info(lchip, serdes_id, DRV_CHIP_SERDES_LANE_INFO, (void*)link_idx));
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "link_idx:%d \n",*link_idx);

    }

    return CTC_E_NONE;
#endif /* never */
    return CTC_E_NONE;
}

int32
sys_usw_peri_set_dlb_chan_type(uint8 lchip, uint8 chan_id)
{
    uint32 value = 0;
    uint32 cmd = 0;
    uint16 lport = 0;
    sys_datapath_lport_attr_t* port_attr = NULL;
    SYS_PERI_INIT_CHECK(lchip);
    /* get local phy port by chan_id */
    lport = SYS_GET_LPORT_ID_WITH_CHAN(lchip, chan_id);

    CTC_ERROR_RETURN(sys_usw_mac_get_port_capability(lchip, lport, &port_attr));
    if (port_attr->port_type == SYS_DMPS_NETWORK_PORT)
    {
        switch (port_attr->speed_mode)
        {
            case CTC_PORT_SPEED_10M:
            case CTC_PORT_SPEED_100M:
            case CTC_PORT_SPEED_1G:
            case CTC_PORT_SPEED_2G5:
                value = 0;  /* 1G, get channel type by datapath */
                break;

            case CTC_PORT_SPEED_5G:
                value = 1;  /* 5G, get channel type by datapath */
                break;

            case CTC_PORT_SPEED_10G:
                value = 2;  /* 10G, get channel type by datapath */
                break;

            case CTC_PORT_SPEED_20G:
                value = 3;  /* 20G, get channel type by datapath */
                break;

            case CTC_PORT_SPEED_25G:
                value = 4;  /* 25G, get channel type by datapath */
                break;

            case CTC_PORT_SPEED_40G:
                value = 5;  /* 40G, get channel type by datapath */
                break;

            case CTC_PORT_SPEED_50G:
                value = 6;  /* 50G, get channel type by datapath */
                break;

            case CTC_PORT_SPEED_100G:
                value = 7;  /* 100G, get channel type by datapath */
                break;
            default:
                break;
        }

        cmd = DRV_IOW(DlbChanMaxLoadType_t, DlbChanMaxLoadType_loadType_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, chan_id, cmd, &value));
    }

    return CTC_E_NONE;
}

/* dynamic switch */
int32
sys_usw_peri_set_serdes_mode(uint8 lchip, ctc_chip_serdes_info_t* p_serdes_info)
{
    uint16 drv_port = 0;
    uint32 gport = 0;
    uint8 gchip = 0;
    uint8 slice_id = 0;
    uint8 chan_id = 0;
    sys_datapath_serdes_info_t* p_serdes = NULL;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_serdes_info);

    if (p_serdes_info->serdes_id >= SYS_USW_DATAPATH_SERDES_NUM)
    {
        return CTC_E_INVALID_PARAM;
    }

    if (p_serdes_info->serdes_mode >= CTC_CHIP_MAX_SERDES_MODE)
    {
        return CTC_E_INVALID_PARAM;
    }

    SWITCH_LOCK(lchip);

#if 0
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_datapath_set_serdes_dynamic_switch(lchip, p_serdes_info->serdes_id,
        p_serdes_info->serdes_mode, p_serdes_info->overclocking_speed), p_usw_peri_master[lchip]->p_switch_mutex);
#else
    CTC_ERROR_RETURN_WITH_UNLOCK(sys_usw_serdes_set_mode(lchip, p_serdes_info), p_usw_peri_master[lchip]->p_switch_mutex);
#endif

    SWITCH_UNLOCK(lchip);

    if (CTC_CHIP_SERDES_NONE_MODE != p_serdes_info->serdes_mode)
    {
        CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_info(lchip, p_serdes_info->serdes_id, &p_serdes));

        slice_id = (p_serdes_info->serdes_id >= SERDES_NUM_PER_SLICE) ? 1 : 0;
        drv_port = p_serdes->lport + MCHIP_CAP(SYS_CAP_PER_SLICE_PORT_NUM) * slice_id;
        CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
        gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, drv_port);
        chan_id = SYS_GET_CHANNEL_ID(lchip, gport);
        CTC_ERROR_RETURN(sys_usw_peri_set_dlb_chan_type(lchip, chan_id));
    }

    return CTC_E_NONE;
}

STATIC int32
_sys_usw_peri_set_global_eee_en(uint8 lchip, uint32* enable)
{
#if 0
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 tbl_id = 0;

    /* cfg NetTxMiscCtl */
    value = enable?1:0;

    tbl_id = NetTxMiscCtl0_t;
    cmd = DRV_IOW(tbl_id, NetTxMiscCtl0_eeeCtlEnable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    tbl_id = NetTxMiscCtl1_t;
    cmd = DRV_IOW(tbl_id, NetTxMiscCtl0_eeeCtlEnable_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
#endif
    return CTC_E_NONE;
}

STATIC int32
_sys_usw_peri_set_mem_scan_mode(uint8 lchip, ctc_chip_mem_scan_cfg_t* p_cfg)
{

    drv_ser_scan_info_t ecc_scan_info;

    if(drv_ser_get_cfg(lchip, DRV_SER_CFG_TYPE_SCAN_MODE, NULL))
    {
        CTC_ERROR_RETURN(sys_usw_dma_set_tcam_scan_mode(lchip, p_cfg->tcam_scan_mode, p_cfg->tcam_scan_interval));
    }

    ecc_scan_info.type = DRV_SER_SCAN_TYPE_TCAM;
    ecc_scan_info.mode =p_cfg->tcam_scan_mode;
    ecc_scan_info.scan_interval = p_cfg->tcam_scan_interval;
    drv_ser_set_cfg(lchip, DRV_SER_CFG_TYPE_SCAN_MODE,&ecc_scan_info);

    ecc_scan_info.type = DRV_SER_SCAN_TYPE_SBE;
    ecc_scan_info.mode = p_cfg->sbe_scan_mode;
    ecc_scan_info.scan_interval = p_cfg->sbe_scan_interval;
    drv_ser_set_cfg(lchip, DRV_SER_CFG_TYPE_SCAN_MODE,&ecc_scan_info);

    return CTC_E_NONE;

}

STATIC int32
_sys_usw_peri_get_mem_scan_mode(uint8 lchip, ctc_chip_mem_scan_cfg_t* p_cfg)
{


    drv_ser_scan_info_t ecc_scan_info;

    ecc_scan_info.type = DRV_SER_SCAN_TYPE_TCAM;
    drv_ser_get_cfg(lchip, DRV_SER_CFG_TYPE_SCAN_MODE,&ecc_scan_info);
    p_cfg->tcam_scan_mode =  ecc_scan_info.mode;
    p_cfg->tcam_scan_interval =  ecc_scan_info.scan_interval;

    ecc_scan_info.type = DRV_SER_SCAN_TYPE_SBE;
    drv_ser_get_cfg(lchip, DRV_SER_CFG_TYPE_SCAN_MODE,&ecc_scan_info);
    p_cfg->sbe_scan_mode =  ecc_scan_info.mode;
    p_cfg->sbe_scan_interval =  ecc_scan_info.scan_interval;


    return CTC_E_NONE;
}

STATIC int32
_sys_usw_peri_reset_hw(uint8 lchip, void* p_data)
{
    drv_ser_set_cfg(lchip, DRV_SER_CFG_TYPE_HW_RESER_EN, NULL);

    return CTC_E_NONE;
}

/**
 @brief begin memory BIST function
*/
STATIC int32
_sys_usw_chip_set_mem_bist(uint8 lchip, ctc_chip_mem_bist_t* p_value)
{
    int32 ret = 0;
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_value);

    if (NULL == MCHIP_API(lchip)->mem_bist_set)
    {
        return CTC_E_NOT_SUPPORT;
    }

    if (1 == SDK_WORK_PLATFORM)
    {
        return CTC_E_NONE;
    }

    CTC_ERROR_RETURN(sys_usw_chip_set_active(lchip, FALSE));
    drv_set_warmboot_status(lchip, 1);

     /*cb*/
    CTC_ERROR_GOTO(MCHIP_API(lchip)->mem_bist_set(lchip, (void*)p_value), ret, error_return);

error_return:
    drv_set_warmboot_status(lchip, 0);
    CTC_ERROR_RETURN(sys_usw_chip_set_active(lchip, TRUE));

    return ret;

}

STATIC
void _sys_usw_chip_phy_link_event_polling_cb(uint8 lchip)
{
    uint8 gchip = 0;
    uint8 link_status_change = 0;
    uint16 lport =0;
    uint32 value = 0;
    uint32 gport = 0;
    uint32 auto_neg_en = 0;
    uint32 auto_neg_mode = 0;
    uint32 speed = 0;
    ctc_chip_mdio_para_t para;
    sys_usw_get_gchip_id(lchip, &gchip);

    for (lport = 0; lport < SYS_USW_MAX_PHY_PORT; lport++)
    {
        SYS_LCHIP_CHECK_ACTIVE_IN_THREAD(lchip);
        link_status_change = 0;
        sys_usw_peri_get_phy_prop(lchip, lport, CTC_PORT_PROP_LINK_UP, &value);
        PERI_LOCK(lchip);
        if (p_usw_peri_master[lchip]->phy_register[lport])
        {
            if (value != p_usw_port_master[lchip]->igs_port_prop[lport].link_status)
            {
                link_status_change = 1;
                SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lport %d old_link_status %d new_link_status %d\n", lport, p_usw_port_master[lchip]->igs_port_prop[lport].link_status, value);
            }
        }
        PERI_UNLOCK(lchip);
        if (0 == link_status_change)
        {
            continue;
        }
        sal_memset(&para, 0, sizeof(ctc_chip_mdio_para_t));

        para.bus = p_usw_peri_master[lchip]->port_mdio_mapping_tbl[lport];
        para.phy_addr = p_usw_peri_master[lchip]->port_phy_mapping_tbl[lport];
        para.dev_no = p_usw_peri_master[lchip]->port_dev_no_mapping_tbl[lport];

        p_usw_port_master[lchip]->igs_port_prop[lport].link_status = value;
        gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, lport);
        sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_MAC_EN, value);

        /*auto neg enable and linkup, set mac speed*/
        if (value)
        {
            sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_SPEED, &speed);
            if ((CTC_PORT_SPEED_1G == speed) || (CTC_PORT_SPEED_100M == speed) || (CTC_PORT_SPEED_10M == speed))
            {
                sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_AUTO_NEG_EN, &auto_neg_en);
                if (auto_neg_en)
                {
                    sys_usw_mac_get_property(lchip, gport, CTC_PORT_PROP_AUTO_NEG_MODE, &auto_neg_mode);
                    if (CTC_PORT_AUTO_NEG_MODE_SGMII_SLAVER == auto_neg_mode)
                    {
                         sys_usw_mac_set_property(lchip, gport, CTC_PORT_PROP_SPEED, speed);
                    }
                 }
            }
        }
        PERI_LOCK(lchip);
        if (value)
        {
            if (p_usw_peri_master[lchip]->phy_register[lport] && p_usw_peri_master[lchip]->phy_register[lport]->driver.linkup_event)
            {
                p_usw_peri_master[lchip]->phy_register[lport]->driver.linkup_event(lchip, &para, p_usw_peri_master[lchip]->phy_register[lport]->user_data);
            }
        }
        else
        {
            if (p_usw_peri_master[lchip]->phy_register[lport] && p_usw_peri_master[lchip]->phy_register[lport]->driver.linkdown_event)
            {
                p_usw_peri_master[lchip]->phy_register[lport]->driver.linkdown_event(lchip, &para, p_usw_peri_master[lchip]->phy_register[lport]->user_data);
            }
        }
        PERI_UNLOCK(lchip);
    }
}

STATIC void
_sys_usw_chip_phy_link_event_polling(void* p_data)
{
    uint32 value = 0;
    uint8 lchip = 0;
    uint16 interval = 0;

    value = (uintptr)p_data;
    lchip = value&0xFF;
    interval = (value>>8)&0xFFFF;
    while (1)
    {
        _sys_usw_chip_phy_link_event_polling_cb(lchip);
        sal_task_sleep(interval);
    }

    return;
}

STATIC int32
_sys_usw_chip_phy_link_monitor(uint8 lchip, uint16 interval)
{
    drv_work_platform_type_t platform_type;
    int32 ret = 0;
    char buffer[SAL_TASK_MAX_NAME_LEN]={0};
    uint32 value = 0;

    CTC_ERROR_RETURN(drv_get_platform_type(lchip, &platform_type));
    if (platform_type != HW_PLATFORM)
    {
        return CTC_E_NONE;
    }

    sal_sprintf(buffer, "ctcphyMon-%d", lchip);
    value = (interval<<8|lchip);
    if (NULL == p_usw_peri_master[lchip]->p_polling_scan)
    {
        ret = sal_task_create(&(p_usw_peri_master[lchip]->p_polling_scan), buffer,
                              SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_DEF, _sys_usw_chip_phy_link_event_polling, (void*)(uintptr)value);
        if (ret < 0)
        {
            return CTC_E_NOT_INIT;
        }
    }
    return CTC_E_NONE;
}


STATIC void
_sys_usw_chip_phy_link_event_interrupt_cb(void* p_data)
{
    uint8 lchip = (uintptr)p_data;

     _sys_usw_chip_phy_link_event_polling_cb(lchip);

    return;
}

STATIC int32
sys_usw_peri_set_phy_register(uint8 lchip, ctc_chip_phy_shim_t* p_register)
{
    sys_chip_phy_shim_t* p_register_tmp = NULL;
    int32 ret = CTC_E_NONE;

    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_register);

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_PARAM, "registe phy name %s phy id 0x%x\n", p_register->phy_name, p_register->phy_id);

    p_register_tmp = mem_malloc(MEM_SYSTEM_MODULE, sizeof(sys_chip_phy_shim_t));
    if (NULL == p_register_tmp)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_register_tmp, 0, sizeof(sys_chip_phy_shim_t));
    sal_memcpy(&(p_register_tmp->phy_shim), p_register, sizeof(ctc_chip_phy_shim_t));

    ctc_slist_add_tail(p_usw_peri_master[lchip]->phy_list, &(p_register_tmp->head));

    if ((0 != p_register->irq) && (CTC_CHIP_PHY_SHIM_EVENT_MODE_IRQ == p_register->event_mode))
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "register phy with mdio scan, irq:%d \n", p_register->irq);
        ret = g_dal_op.interrupt_register(p_register->irq, SAL_TASK_PRIO_DEF, _sys_usw_chip_phy_link_event_interrupt_cb, NULL);
        if (ret < 0)
        {
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, "register phy interrupt failed!! irq:%d \n", p_register->irq);
             return CTC_E_NOT_EXIST;
        }
    }
    else if (CTC_CHIP_PHY_SHIM_EVENT_MODE_SCAN == p_register->event_mode)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "register phy with mdio scan\n");
        sys_usw_peri_set_phy_scan_en(lchip, TRUE);
    }
    else if (CTC_CHIP_PHY_SHIM_EVENT_MODE_POLLING== p_register->event_mode)
    {
        SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "register phy with polling, timer:%d \n", p_register->poll_interval);
        _sys_usw_chip_phy_link_monitor(lchip, p_register->poll_interval);
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_peri_set_clock(uint8 lchip, ctc_chip_peri_clock_t* p_clock)
{
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    switch(p_clock->type)
    {
        case CTC_CHIP_PERI_MDIO_TYPE:
            CTC_ERROR_RETURN(sys_usw_peri_set_mdio_clock(lchip, p_clock->mdio_type, p_clock->clock_val));
            break;
        case CTC_CHIP_PERI_I2C_TYPE:
            CTC_ERROR_RETURN(sys_usw_peri_set_i2c_clock(lchip, p_clock->ctl_id, p_clock->clock_val));
            break;
        case CTC_CHIP_PERI_MAC_LED_TYPE:
            CTC_ERROR_RETURN(sys_usw_peri_set_mac_led_clock(lchip, p_clock->clock_val));
            break;
        default:
            return CTC_E_INVALID_PARAM;
            break;
    }

    return CTC_E_NONE;
}

int32
sys_usw_peri_get_clock(uint8 lchip, ctc_chip_peri_clock_t* p_clock)
{
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    switch(p_clock->type)
    {
        case CTC_CHIP_PERI_MDIO_TYPE:
            CTC_ERROR_RETURN(sys_usw_peri_get_mdio_clock(lchip, p_clock->mdio_type, &p_clock->clock_val));
            break;
        case CTC_CHIP_PERI_I2C_TYPE:
            CTC_ERROR_RETURN(sys_usw_peri_get_i2c_clock(lchip, p_clock->ctl_id, &p_clock->clock_val));
            break;
        case CTC_CHIP_PERI_MAC_LED_TYPE:
            CTC_ERROR_RETURN(sys_usw_peri_get_mac_led_clock(lchip, &p_clock->clock_val));
            break;
        default:
            return CTC_E_INVALID_PARAM;
            break;
    }

    return CTC_E_NONE;
}

int32
sys_usw_peri_set_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value)
{
    ctc_chip_gpio_params_t* p_gpio_param = NULL;
    ctc_chip_gpio_cfg_t* p_gpio_cfg = NULL;
    ctc_chip_phy_mapping_para_t* p_phy_mapping = NULL;

    CTC_PTR_VALID_CHECK(p_value);
    SYS_PERI_INIT_CHECK(lchip);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "lchip:%d chip_prop:0x%x \n", lchip, chip_prop);


    switch(chip_prop)
    {
        case CTC_CHIP_PROP_EEE_EN:
            CTC_ERROR_RETURN(_sys_usw_peri_set_global_eee_en(lchip, (uint32*)p_value));
            break;
#if 0
        case CTC_CHIP_PROP_SERDES_PRBS:
            CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_prbs(lchip, p_value));
            break;
        case CTC_CHIP_PROP_SERDES_FFE:
            CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_ffe(lchip, p_value));
            break;
        case CTC_CHIP_PEOP_SERDES_POLARITY:
            CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_polarity(lchip, p_value));
            break;
        case CTC_CHIP_PROP_SERDES_LOOPBACK:
            CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_loopback(lchip, p_value));
            break;
#endif
        case CTC_CHIP_MAC_LED_EN:
            CTC_ERROR_RETURN(sys_usw_peri_set_mac_led_en(lchip, *((bool *)p_value)));
            break;
        case CTC_CHIP_PROP_MEM_SCAN:
            CTC_ERROR_RETURN(_sys_usw_peri_set_mem_scan_mode(lchip, p_value));
            break;
        case CTC_CHIP_PROP_RESET_HW:
            CTC_ERROR_RETURN(_sys_usw_peri_reset_hw(lchip, p_value));
            break;
#if 0
        case CTC_CHIP_PROP_SERDES_P_FLAG:
        case CTC_CHIP_PROP_SERDES_PEAK:
        case CTC_CHIP_PROP_SERDES_DPC:
        case CTC_CHIP_PROP_SERDES_SLEW_RATE:
            CTC_ERROR_RETURN(sys_usw_datapath_set_serdes_cfg(lchip, chip_prop, p_value));
            break;
#endif
        case CTC_CHIP_PROP_GPIO_MODE:
            p_gpio_param = (ctc_chip_gpio_params_t*)p_value;
            CTC_ERROR_RETURN(sys_usw_peri_set_gpio_mode(lchip,
                                    p_gpio_param->gpio_id, p_gpio_param->value));
            break;
        case CTC_CHIP_PROP_GPIO_OUT:
            p_gpio_param = (ctc_chip_gpio_params_t*)p_value;
            CTC_ERROR_RETURN(sys_usw_peri_set_gpio_output(lchip,
                                    p_gpio_param->gpio_id, p_gpio_param->value));
            break;
        case CTC_CHIP_PROP_GPIO_CFG:
            p_gpio_cfg = (ctc_chip_gpio_cfg_t*)p_value;
            CTC_ERROR_RETURN(sys_usw_peri_set_gpio_cfg(lchip, p_gpio_cfg));
            break;
        case CTC_CHIP_PROP_PHY_MAPPING:
            p_phy_mapping= (ctc_chip_phy_mapping_para_t*)p_value;
            CTC_ERROR_RETURN(sys_usw_peri_set_phy_mapping(lchip, p_phy_mapping));
            break;
        case CTC_CHIP_PROP_MEM_BIST:
            CTC_ERROR_RETURN(_sys_usw_chip_set_mem_bist(lchip, (ctc_chip_mem_bist_t*)p_value));
            break;
        case CTC_CHIP_PHY_SCAN_EN:
            CTC_ERROR_RETURN(sys_usw_peri_set_phy_scan_en(lchip, *((bool*)p_value)));
            break;
        case CTC_CHIP_PHY_SCAN_PARA:
            CTC_ERROR_RETURN(sys_usw_peri_set_phy_scan_para(lchip, (ctc_chip_phy_scan_ctrl_t*)p_value));
            break;
        case CTC_CHIP_I2C_SCAN_EN:
            CTC_ERROR_RETURN(sys_usw_peri_set_i2c_scan_en(lchip, *((bool*)p_value)));
            break;
        case CTC_CHIP_I2C_SCAN_PARA:
            CTC_ERROR_RETURN(sys_usw_peri_set_i2c_scan_para(lchip, (ctc_chip_i2c_scan_t*)p_value));
            break;
        case CTC_CHIP_PROP_SERDES_PRBS:
        case CTC_CHIP_PROP_SERDES_FFE:
        case CTC_CHIP_PEOP_SERDES_POLARITY:
        case CTC_CHIP_PROP_SERDES_LOOPBACK:
        case CTC_CHIP_PROP_SERDES_P_FLAG:
        case CTC_CHIP_PROP_SERDES_PEAK:
        case CTC_CHIP_PROP_SERDES_DPC:
        case CTC_CHIP_PROP_SERDES_SLEW_RATE:
        case CTC_CHIP_PROP_SERDES_CTLE:
            CTC_ERROR_RETURN(sys_usw_serdes_set_property(lchip, chip_prop, p_value));
            break;
        case CTC_CHIP_PROP_PERI_CLOCK:
            CTC_ERROR_RETURN(sys_usw_peri_set_clock(lchip, (ctc_chip_peri_clock_t*)p_value));
            break;
        case CTC_CHIP_PROP_MAC_LED_BLINK_INTERVAL:
            CTC_ERROR_RETURN(sys_usw_peri_set_macled_blink_interval(lchip, (uint32*)p_value));
            break;
        case CTC_CHIP_PROP_REGISTER_PHY:
            CTC_ERROR_RETURN(sys_usw_peri_set_phy_register(lchip, (ctc_chip_phy_shim_t*)p_value));
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_peri_get_property(uint8 lchip, ctc_chip_property_t chip_prop, void* p_value)
{
    ctc_port_serdes_info_t* p_serdes_port = NULL;
    uint8 slice_id = 0;
    uint8 gchip = 0;
    uint16 drv_port = 0;
    ctc_chip_gpio_params_t* p_gpio_param = NULL;
    sys_datapath_serdes_info_t* p_serdes = NULL;
    ctc_chip_gpio_cfg_t* p_gpio_cfg = NULL;
    CTC_PTR_VALID_CHECK(p_value);
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    switch(chip_prop)
    {
#if 0
        case CTC_CHIP_PROP_SERDES_FFE:
            p_ffe = (ctc_chip_serdes_ffe_t*)p_value;
            CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_ffe(lchip, p_ffe->serdes_id, coefficient, p_ffe->mode, &value));
            for (index = 0; index < SYS_USW_CHIP_FFE_PARAM_NUM; index++)
            {
                p_ffe->coefficient[index] = coefficient[index];
            }
            p_ffe->status = value;
            break;
        case CTC_CHIP_PEOP_SERDES_POLARITY:
            CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_polarity(lchip, p_value));
            break;
        case CTC_CHIP_PROP_SERDES_P_FLAG:
        case CTC_CHIP_PROP_SERDES_PEAK:
        case CTC_CHIP_PROP_SERDES_DPC:
        case CTC_CHIP_PROP_SERDES_SLEW_RATE:
            CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_cfg(lchip, chip_prop, p_value));
            break;
        case CTC_CHIP_PROP_SERDES_LOOPBACK:
            CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_loopback(lchip, p_value));
            break;
#else
        case CTC_CHIP_PROP_SERDES_FFE:
        case CTC_CHIP_PEOP_SERDES_POLARITY:
        case CTC_CHIP_PROP_SERDES_LOOPBACK:
        case CTC_CHIP_PROP_SERDES_P_FLAG:
        case CTC_CHIP_PROP_SERDES_PEAK:
        case CTC_CHIP_PROP_SERDES_DPC:
        case CTC_CHIP_PROP_SERDES_SLEW_RATE:
        case CTC_CHIP_PROP_SERDES_CTLE:
            CTC_ERROR_RETURN(sys_usw_serdes_get_property(lchip, chip_prop, p_value));
            break;
#endif
        case CTC_CHIP_PROP_DEVICE_INFO:
            CTC_ERROR_RETURN(sys_usw_chip_get_device_info(lchip, (ctc_chip_device_info_t*)p_value));
            break;
        case CTC_CHIP_MAC_LED_EN:
            CTC_ERROR_RETURN(sys_usw_peri_get_mac_led_en(lchip, (bool*)p_value));
            break;
        case CTC_CHIP_PROP_MEM_SCAN:
            CTC_ERROR_RETURN(_sys_usw_peri_get_mem_scan_mode(lchip, p_value));
            break;
        case CTC_CHIP_PROP_GPIO_IN:
            p_gpio_param = (ctc_chip_gpio_params_t*)p_value;
            CTC_ERROR_RETURN(sys_usw_peri_get_gpio_input(lchip,
                                    p_gpio_param->gpio_id, &(p_gpio_param->value)));
            break;
        case CTC_CHIP_PROP_GPIO_CFG:
            p_gpio_cfg = (ctc_chip_gpio_cfg_t*)p_value;
            CTC_ERROR_RETURN(sys_usw_peri_get_gpio_cfg(lchip, p_gpio_cfg));
            break;
        case CTC_CHIP_PROP_SERDES_ID_TO_GPORT:
            p_serdes_port = (ctc_port_serdes_info_t*)p_value;
            CTC_ERROR_RETURN(sys_usw_datapath_get_serdes_info(lchip, p_serdes_port->serdes_id, &p_serdes));
            drv_port = p_serdes->lport + 256*slice_id;
            CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip));
            p_serdes_port->gport = SYS_MAP_DRV_LPORT_TO_CTC_GPORT(gchip, drv_port);
            p_serdes_port->overclocking_speed = p_serdes->overclocking_speed;
            p_serdes_port->serdes_mode = p_serdes->mode;
            break;
        case CTC_CHIP_PHY_SCAN_EN:
            CTC_ERROR_RETURN(sys_usw_peri_get_phy_scan_en(lchip, (bool*)p_value));
            break;
        case CTC_CHIP_PHY_SCAN_PARA:
            CTC_ERROR_RETURN(sys_usw_peri_get_phy_scan_para(lchip, (ctc_chip_phy_scan_ctrl_t*)p_value));
            break;
        case CTC_CHIP_I2C_SCAN_EN:
            CTC_ERROR_RETURN(sys_usw_peri_get_i2c_scan_en(lchip, (bool*)p_value));
            break;
        case CTC_CHIP_I2C_SCAN_PARA:
            CTC_ERROR_RETURN(sys_usw_peri_get_i2c_scan_para(lchip, (ctc_chip_i2c_scan_t*)p_value));
            break;
        case CTC_CHIP_PROP_PERI_CLOCK:
            CTC_ERROR_RETURN(sys_usw_peri_get_clock(lchip, (ctc_chip_peri_clock_t*)p_value));
            break;
        case CTC_CHIP_PROP_MAC_LED_BLINK_INTERVAL:
            CTC_ERROR_RETURN(sys_usw_peri_get_macled_blink_interval(lchip, (uint32*)p_value));
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_get_chip_sensor(uint8 lchip, ctc_chip_sensor_type_t type, uint32* p_value)
{
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_value);
    SYS_PERI_INIT_CHECK(lchip);

    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_get_chip_sensor(lchip, type, p_value));
    return CTC_E_NONE;
}

int32
sys_usw_peri_get_gport_by_mdio_para(uint8 lchip, uint8 bus, uint8 phy_addr, uint32* gport)
{
    uint8  gchip_id = 0;
    uint16 lport = 0;
    ctc_chip_phy_mapping_para_t* p_phy_mapping = NULL;
    int32 ret = CTC_E_NONE;

    p_phy_mapping = mem_malloc(MEM_SYSTEM_MODULE, sizeof(ctc_chip_phy_mapping_para_t));
    if (NULL == p_phy_mapping)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_phy_mapping, 0, sizeof(ctc_chip_phy_mapping_para_t));
    CTC_ERROR_GOTO(sys_usw_peri_get_phy_mapping(lchip, p_phy_mapping), ret, out);
    CTC_ERROR_GOTO(sys_usw_get_gchip_id(lchip, &gchip_id), ret, out);

    for(lport=0; lport < SYS_USW_MAX_PHY_PORT; lport++)
    {
        if ((bus == p_phy_mapping->port_mdio_mapping_tbl[lport])
                    && (phy_addr == p_phy_mapping->port_phy_mapping_tbl[lport]))
        {
            *gport = CTC_MAP_LPORT_TO_GPORT(gchip_id, lport);
            break;
        }
    }

    if(lport == SYS_USW_MAX_PHY_PORT)
    {
        ret = CTC_E_INVALID_PARAM;
    }

out:
    if (p_phy_mapping)
    {
        mem_free(p_phy_mapping);
    }

    return ret;
}

int32
sys_usw_peri_phy_link_change_isr(uint8 lchip, uint32 intr, void * p_data)
{
    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_PTR_VALID_CHECK(p_data);
    SYS_PERI_INIT_CHECK(lchip);

    CTC_ERROR_RETURN(MCHIP_API(lchip)->peri_phy_link_change_isr(lchip, intr, p_data));
    return CTC_E_NONE;
}

int32
sys_usw_peri_gpio_isr(uint8 lchip, uint32 intr, void* p_data)
{
    CTC_INTERRUPT_EVENT_FUNC event_cb;
    uint32 cmd;
    uint32 gpio_status = 0;
    uint32 hs_gpio_status = 0;
    uint8  loop = 0;
    uint8  gchip_id = 0;
    uint8  gpio_isr_en = 0;
    uint8  gpio_id = 0;
    int32 ret = 0;
    ctc_chip_gpio_event_t   gpio_event_status;
    sys_intr_type_t    intr_type;
    uint32 value = 0;

    SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    CTC_ERROR_RETURN(sys_usw_interrupt_get_event_cb(lchip, CTC_EVENT_EXTRAL_GPIO_CHANGE, &event_cb));
    CTC_ERROR_RETURN(sys_usw_get_gchip_id(lchip, &gchip_id));

    sal_memset(&gpio_event_status, 0, sizeof(ctc_chip_gpio_event_t));
    sal_memset(&intr_type, 0, sizeof(intr_type));
    intr_type.intr = intr;
    intr_type.sub_intr = INVG;
    intr_type.low_intr = INVG;
    CTC_ERROR_RETURN(sys_usw_interrupt_set_en(lchip, &intr_type, 0));

    cmd = DRV_IOR(GpioIntrStatus_t, GpioIntrStatus_gpioIntrStatus_f);
    DRV_IOCTL(lchip, 0, cmd, &gpio_status);

    cmd = DRV_IOR(GpioHsIntrStatus_t, GpioHsIntrStatus_gpioHsIntrStatus_f);
    DRV_IOCTL(lchip, 0, cmd, &hs_gpio_status);

    for(loop=0; loop < SYS_PERI_HS_MAX_GPIO_ID; loop++)
    {
        if (SYS_PERI_GPIO_IS_HS(loop))
        {
            gpio_id = loop-SYS_PERI_MAX_GPIO_ID;
            gpio_isr_en = CTC_IS_BIT_SET(hs_gpio_status, gpio_id);
        }
        else
        {
            gpio_id = loop;
            gpio_isr_en = CTC_IS_BIT_SET(gpio_status, gpio_id);
        }

        if(gpio_isr_en)
        {
            /*clear status*/
            value = 0;
            CTC_BIT_SET(value, gpio_id);
            if (SYS_PERI_GPIO_IS_HS(loop))
            {
                cmd = DRV_IOW(GpioHsEoiCtl_t, GpioHsEoiCtl_cfgHsIntrClear_f);
                DRV_IOCTL(lchip, loop, cmd, &value);
            }
            else
            {
                cmd = DRV_IOW(GpioEoiCtl_t, GpioEoiCtl_cfgIntrClear_f);
                DRV_IOCTL(lchip, loop, cmd, &value);
            }

            gpio_event_status.gpio_id = loop;
            SYS_PERI_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "gpio_id %d occur interrupt!\n",
                                                   loop);
            if(event_cb)
            {
                event_cb(gchip_id, &gpio_event_status);
            }
        }
    }

    sys_usw_interrupt_set_en(lchip, &intr_type, 1);
    return ret;
}

