#include "hsrv_inc.h"
#include "hsrv_openflow.h"
#include "ctc_ttp_drv_port.h"

int32
ctc_ttp_drv_port_enable(uint32 ifindex)
{
    ctc_port_scl_property_t port_scl_property;
    hsrv_openflow_ttp_port_info_t* p_port = NULL;

    sal_memset(&port_scl_property, 0, sizeof(ctc_port_scl_property_t));

    /* get hsrv port info */
    p_port = hsrv_openflow_port_get_port_info_by_ifindex(ifindex);
    if (!p_port)
    {
        return HSRV_E_PORT_NOT_EXIST;
    }
        
    /* 1. set mac learning disable */
    HSRV_IF_ERROR_RETURN(ctc_port_set_learning_en(p_port->gport, FALSE));

    /* 2. set vlan tag:allow all packet  */
    HSRV_IF_ERROR_RETURN(ctc_port_set_vlan_ctl(p_port->gport, CTC_VLANCTL_ALLOW_ALL_PACKETS));

    /* 3. set vlan filter disable */
    HSRV_IF_ERROR_RETURN(ctc_port_set_vlan_filter_en(p_port->gport, CTC_BOTH_DIRECTION, FALSE));
    HSRV_IF_ERROR_RETURN(ctc_port_set_property(p_port->gport, CTC_PORT_PROP_DOT1Q_TYPE, CTC_DOT1Q_TYPE_SVLAN));
    HSRV_IF_ERROR_RETURN(ctc_port_set_property(p_port->gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 1));

    /* 4. set scl 0 */
    port_scl_property.scl_id = 0;
    port_scl_property.tcam_type = CTC_PORT_IGS_SCL_TCAM_TYPE_MAC;
    port_scl_property.action_type = CTC_PORT_SCL_ACTION_TYPE_FLOW;
    port_scl_property.direction = CTC_INGRESS;
    port_scl_property.class_id_en = 1;
    port_scl_property.class_id = p_port->class_id;
    HSRV_IF_ERROR_RETURN(ctc_port_set_scl_property(p_port->gport, &port_scl_property));

    
    sal_memset(&port_scl_property, 0, sizeof(ctc_port_scl_property_t));
    
    /* 5. set scl 1 */
    port_scl_property.scl_id = 1;
    port_scl_property.hash_type = CTC_PORT_IGS_SCL_HASH_TYPE_PORT_SVLAN;
    port_scl_property.action_type = CTC_PORT_SCL_ACTION_TYPE_SCL;
    port_scl_property.direction = CTC_INGRESS;
    port_scl_property.class_id_en = 1;
    port_scl_property.class_id = p_port->class_id;
    HSRV_IF_ERROR_RETURN(ctc_port_set_scl_property(p_port->gport, &port_scl_property));

    return 0;
}

int32
ctc_ttp_drv_port_disable(uint32 ifindex)
{
    hsrv_openflow_ttp_port_info_t* p_port = NULL;

    /* get hsrv port info */
    p_port = hsrv_openflow_port_get_port_info_by_ifindex(ifindex);
    if (!p_port)
    {
        return HSRV_E_PORT_NOT_EXIST;
    }
    HSRV_IF_ERROR_RETURN(ctc_port_set_learning_en(p_port->gport, TRUE));

    HSRV_IF_ERROR_RETURN(ctc_port_set_vlan_ctl(p_port->gport, CTC_VLANCTL_DROP_ALL_TAGGED));

    HSRV_IF_ERROR_RETURN(ctc_port_set_vlan_filter_en(p_port->gport, CTC_BOTH_DIRECTION, TRUE));
    HSRV_IF_ERROR_RETURN(ctc_port_set_property(p_port->gport, CTC_PORT_PROP_ADD_DEFAULT_VLAN_DIS, 0));
    
    return 0;
}

