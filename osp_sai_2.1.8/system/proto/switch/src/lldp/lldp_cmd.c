#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_lldp_global_define.h"
#include "gen/tbl_lldp_global.h"
#include "gen/tbl_lldp_if_define.h"
#include "gen/tbl_lldp_if.h"
#include "gen/tbl_card_define.h"
#include "gen/tbl_card.h"
#include "gen/tbl_version_define.h"
#include "gen/tbl_version.h"
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_global.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "switch.h"
#include "lldp_define.h"
#include "lldp.h"
#include "glb_macro.h"
#include "lldp_api.h"
#include "mstp_define.h"


static char *lldp_basic_str[BASIC_TLV_EN_ALL] = 
{
    "port-description",
    "system-name",
    "system-description",
    "system-capabilities",
    "management-address",
};

static char *lldp_8021_str[IEEE_8021_TLV_EN_ALL] = 
{
    "port-vlan",
    "protocol-vlan",
    "vlan-name",
    "protocol-id",
    "link-aggregation",
    "dcbx",
};

static char *lldp_8023_str[IEEE_8023_TLV_EN_ALL] = 
{
    "mac-phy-cfg",
    "power",
    "link-aggregation",
    "max-frame-size",
};

static char *lldp_med_str[MED_TLV_EN_ALL] = 
{
    "capability",
    "network-policy",
    "location-id",
    "ext-power",
    "inventory",
};

const static char *lldp_if_mode_cli_strs[LLDP_IF_MODE_MAX] =
{
    "Disabled",
    "TXRX",
    "TX",
    "RX"
};

const static char *lldp_sys_cap_strs[LLDP_SYS_CAP_MAX] =
{
    "Other",
    "Repeater",
    "Bridge",
    "WLAN Access Point",
    "Router",
    "Telephone",
    "DOCSIS cable device",
    "Station",
};

const static char *proto_id_strs[LLDP_PROTO_MAX] =
{
    "Spanning Tree Protocol",
    "Rapid Spanning Tree Protocol",
    "Multiple Spanning Tree Protocol",
    "GARP Multicast Registration Protocol",
    "MRP Multicast Registration Protocol",
    "GARP VLAN Registration Protocol",
    "MRP VLAN Registration Protocol",
    "Link Aggregation",
    "Port Authentication",
    "Ethernet OAM",
};

const static char *lldp_med_np_app_str[LLDP_MED_NP_TYPE_MAX] = 
{
    "Unknown",
    "Voice",
    "Voice Signaling",
    "Guest Voice",
    "Guest Voice Signaling",
    "Softphone Voice",
    "Video Conference",
    "Streaming Video",
    "Video Signaling"
};

static char *lldp_chassis_id_type[LLDP_CHASSIS_ID_SUBTYPE_MAX] = 
{
    "",
    "Chassis Component",
    "Interface alias",
    "Port Component",
    "Mac address",
    "Net address",
    "Interface Name",
    "Locally assigned"
};

static char *lldp_port_id_type[LLDP_PORT_ID_SUBTYPE_MAX] = 
{
    "",
    "Interface alias",
    "Port Component",
    "Mac address",
    "Net address",
    "Interface Name",
    "Agent circuitid",
    "Locally assigned"
};

static char *lldp_med_cap_tlv_type [MED_TYPE_CAPABILITIES_SUB_RESERVED] =  
{
    "LLDP-MED Capabiliies",
    "Network Policy",
    "Location Identification",
    "Extended Power via MDI - PSE",
    "Extended Power via MDI - PD",
    "Inventory",
};

int32
lldp_if_update_all_tx_ttl()
{
    tbl_lldp_if_master_t *p_master = tbl_lldp_if_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_lldp_if_t *p_db_lldp_if = NULL;

    CTCLIB_SLIST_LOOP(p_master->if_list, p_db_lldp_if, listnode)
    {
        lldp_if_update_tx_ttl(p_db_lldp_if);
        if (p_db_lldp_if->mode == LLDP_IF_TXRX || p_db_lldp_if->mode == LLDP_IF_TXONLY)
        {
            lldp_pdu_tx(p_db_lldp_if);
        } 
    }
    return PM_E_NONE;
}

int32
lldp_set_enable(uint32 enable)
{
    tbl_lldp_global_t* p_lldp_glb = tbl_lldp_global_get_lldp_global();
    tbl_lldp_if_master_t *p_master = tbl_lldp_if_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_lldp_if_t *p_db_lldp_if = NULL;
    
    p_lldp_glb->enable = enable;
    tbl_lldp_global_set_lldp_global_field(p_lldp_glb, TBL_LLDP_GLOBAL_FLD_ENABLE);

    /*modified by yejl to fix bug40072, 2016-08-10*/
    if (!enable)
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_lldp_if, listnode)
        {
            if (p_db_lldp_if->mode == LLDP_IF_TXRX || p_db_lldp_if->mode == LLDP_IF_TXONLY)
            {
                /*If lldp global disable, send a sample lldp packet(tx-ttl is 0) to remote*/
                lldp_pdu_tx_disable(p_db_lldp_if);
            }
            /*If lldp global disable, delete all the neigh information*/
            _lldp_if_del_all_neigh(p_db_lldp_if);
        } 
    }
    return PM_E_NONE;
}

int32
lldp_set_timer_tx_hold(uint32 tx_hold)
{
    tbl_lldp_global_t* p_lldp_glb = tbl_lldp_global_get_lldp_global();

    p_lldp_glb->tx_hold = tx_hold;
    tbl_lldp_global_set_lldp_global_field(p_lldp_glb, TBL_LLDP_GLOBAL_FLD_TX_HOLD);

    lldp_if_update_all_tx_ttl();

    /*LLDP debug*/
    LLDP_DEBUG(LLDP_EVENT, "LLDP set tx hold to value %d and update tx TTL and tx init\n", tx_hold);
        
    return PM_E_NONE;
}

int32
lldp_set_timer_tx_interval(uint32 tx_interval)
{
    tbl_lldp_global_t* p_lldp_glb = tbl_lldp_global_get_lldp_global();

    p_lldp_glb->tx_interval = tx_interval;
    tbl_lldp_global_set_lldp_global_field(p_lldp_glb, TBL_LLDP_GLOBAL_FLD_TX_INTERVAL);

    lldp_if_update_all_tx_ttl();

    /*LLDP debug*/
    LLDP_DEBUG(LLDP_EVENT, "LLDP set tx interval to value %d and update tx TTL and tx init\n", tx_interval);
    
    return PM_E_NONE;
}

int32
lldp_set_timer_tx_delay(uint32 tx_delay)
{
    tbl_lldp_global_t* p_lldp_glb = tbl_lldp_global_get_lldp_global();

    p_lldp_glb->tx_delay = tx_delay;
    tbl_lldp_global_set_lldp_global_field(p_lldp_glb, TBL_LLDP_GLOBAL_FLD_TX_DELAY);

    return PM_E_NONE;
}

int32
lldp_set_timer_reinit_delay(uint32 reinit_delay)
{
    tbl_lldp_global_t* p_lldp_glb = tbl_lldp_global_get_lldp_global();

    p_lldp_glb->reinit_delay = reinit_delay;
    tbl_lldp_global_set_lldp_global_field(p_lldp_glb, TBL_LLDP_GLOBAL_FLD_REINIT_DELAY);

    return PM_E_NONE;
}

int32
lldp_set_system_name(char *system_name)
{
    tbl_lldp_global_t* p_lldp_glb = tbl_lldp_global_get_lldp_global();

    sal_memset(p_lldp_glb->system_name, 0, sizeof(p_lldp_glb->system_name));
    sal_strcpy(p_lldp_glb->system_name, system_name);
    tbl_lldp_global_set_lldp_global_field(p_lldp_glb, TBL_LLDP_GLOBAL_FLD_SYSTEM_NAME);

    return PM_E_NONE;
}

int32
lldp_set_system_desc(char *system_desc)
{
    tbl_lldp_global_t* p_lldp_glb = tbl_lldp_global_get_lldp_global();

    sal_memset(p_lldp_glb->system_desc, 0, sizeof(p_lldp_glb->system_desc));
    sal_strcpy(p_lldp_glb->system_desc, system_desc);
    tbl_lldp_global_set_lldp_global_field(p_lldp_glb, TBL_LLDP_GLOBAL_FLD_SYSTEM_DESC);

    return PM_E_NONE;
}

int32
lldp_set_system_mgmt_ip(addr_t *addr)
{
    tbl_lldp_global_t* p_lldp_glb = tbl_lldp_global_get_lldp_global();

    sal_memcpy(&p_lldp_glb->management_ip, addr, sizeof(addr_t));
    tbl_lldp_global_set_lldp_global_field(p_lldp_glb, TBL_LLDP_GLOBAL_FLD_MANAGEMENT_IP);
    tbl_lldp_global_set_lldp_global_field(p_lldp_glb, TBL_LLDP_GLOBAL_FLD_CONFIG_FLAG);

    return PM_E_NONE;
}

int32
lldp_set_if_set_basic_tvl(tbl_lldp_if_t* p_lldp_if)
{
    int32 rc = PM_E_NONE;
    
     /*lookup the interface exist*/
    if (NULL == p_lldp_if)
    {
        return PM_E_FAIL;
    }
    
    /* 2. update local CDB */ 
    PM_E_RETURN(tbl_lldp_if_set_lldp_if_field(p_lldp_if, TBL_LLDP_IF_FLD_TLV_ENABLE_BASIC));
    
    return rc;
}

int32
lldp_set_if_set_med_tvl(tbl_lldp_if_t* p_lldp_if)
{
    int32 rc = PM_E_NONE;
    
    /*lookup the interface exist*/
    if (NULL == p_lldp_if)
    {
        return PM_E_FAIL;
    }
    
    /* 2. update local CDB */ 
    PM_E_RETURN(tbl_lldp_if_set_lldp_if_field(p_lldp_if, TBL_LLDP_IF_FLD_TLV_ENABLE_MED));
    
    return rc;
}

int32
lldp_set_if_set_8021_org_specific_tvl(tbl_lldp_if_t* p_lldp_if)
{
    int32 rc = PM_E_NONE;
    
    /*lookup the interface exist*/
    if (NULL == p_lldp_if)
    {
        return PM_E_FAIL;
    }
    
    /* 2. update local CDB */ 
    PM_E_RETURN(tbl_lldp_if_set_lldp_if_field(p_lldp_if, TBL_LLDP_IF_FLD_TLV_ENABLE_8021));
    
    return rc;
}

int32
lldp_set_if_set_8021_org_specific_vlan_id(tbl_lldp_if_t* p_lldp_if, uint32 vlan_id)
{
    int32 rc = PM_E_NONE;
    
    /*lookup the interface exist*/
    if (NULL == p_lldp_if)
    {
        return PM_E_FAIL;
    }
    p_lldp_if->vlan_name_id = vlan_id;
    
    /* 2. update local CDB */ 
    PM_E_RETURN(tbl_lldp_if_set_lldp_if_field(p_lldp_if, TBL_LLDP_IF_FLD_VLAN_NAME_ID));
    return rc;
}

int32
lldp_set_if_set_8023_org_specific_tvl(tbl_lldp_if_t* p_lldp_if)
{
    int32 rc = PM_E_NONE;
    
    /*lookup the interface exist*/
    if (NULL == p_lldp_if)
    {
        return PM_E_FAIL;
    }
    
    /* 2. update local CDB */ 
    PM_E_RETURN(tbl_lldp_if_set_lldp_if_field(p_lldp_if, TBL_LLDP_IF_FLD_TLV_ENABLE_8023));
    
    return rc;
}

int32
lldp_set_if_set_med_location_id_tvl(tbl_lldp_if_t* p_lldp_if)
{
    int32 rc = PM_E_NONE;
    
    /*lookup the interface exist*/
    if (NULL == p_lldp_if)
    {
        return PM_E_FAIL;
    }
    
    /* 2. update local CDB */ 
    PM_E_RETURN(tbl_lldp_if_set_lldp_if_field(p_lldp_if, TBL_LLDP_IF_FLD_MED_LCI_FORMAT));
    PM_E_RETURN(tbl_lldp_if_set_lldp_if_field(p_lldp_if, TBL_LLDP_IF_FLD_MED_LCI_ELIN));
    PM_E_RETURN(tbl_lldp_if_set_lldp_if_field(p_lldp_if, TBL_LLDP_IF_FLD_MED_LCI_CIVIC));

    /*If the location id tlv is modified, send the lldp packect to remote immediately*/
    if (p_lldp_if->mode == LLDP_IF_TXRX || p_lldp_if->mode == LLDP_IF_TXONLY)
    {
        lldp_pdu_tx(p_lldp_if);
    }
    
    return rc;
}

static int32
_lldp_cmd_get_lldp_global(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_lldp_global_t* p_lldp_glb = tbl_lldp_global_get_lldp_global();
    tbl_iter_args_t args;
        
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    tbl_lldp_global_dump_one(p_lldp_glb, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_lldp_cmd_set_enable(uint32 enable, tbl_lldp_global_t* p_lldp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (enable == p_lldp_glb->enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(lldp_set_enable(enable));
    return rc;
}

static int32
_lldp_cmd_set_timer_tx_hold(uint32 tx_hold, tbl_lldp_global_t* p_lldp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (tx_hold == p_lldp_glb->tx_hold)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(lldp_set_timer_tx_hold(tx_hold));

    return PM_E_NONE;
}


static int32
_lldp_cmd_set_timer_tx_interval(uint32 tx_interval, tbl_lldp_global_t* p_lldp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 tx_delay = 0;
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();

    /* 1. config check */
    
    tx_delay = p_lldp_glb->tx_delay;
    if (p_sys_glb->startup_done && tx_delay > (LLDP_TX_DELAY_CONSTANT * tx_interval))
    {
        CFG_INVALID_PARAM_RET("Invalid TxInterval value, TxInterval value should be >= 4*txdelay and txdelay is %u now", tx_delay);
    }

    /* 2. check duplicate config */
    if (tx_interval == p_lldp_glb->tx_interval)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(lldp_set_timer_tx_interval(tx_interval));

    return PM_E_NONE;
}

static int32
_lldp_cmd_set_timer_tx_delay(uint32 tx_delay, tbl_lldp_global_t* p_lldp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    uint32 tx_interval = 0;

    /* 1. config check */
    tx_interval = p_lldp_glb->tx_interval;
    if (tx_delay > (LLDP_TX_DELAY_CONSTANT * tx_interval))
    {
        CFG_INVALID_PARAM_RET("Invalid TxDelay value, TxDelay value should be <=0.25*txinterval and txinterval is %u now", tx_interval);
    }
    /* 2. check duplicate config */
    if (tx_delay == p_lldp_glb->tx_delay)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(lldp_set_timer_tx_delay(tx_delay));

    return PM_E_NONE;
}

static int32
_lldp_cmd_set_timer_reinit_delay(uint32 reinit_delay, tbl_lldp_global_t* p_lldp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */

    /* 2. check duplicate config */
    if (reinit_delay == p_lldp_glb->reinit_delay)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(lldp_set_timer_reinit_delay(reinit_delay));

    return PM_E_NONE;
}

static int32
_lldp_cmd_set_system_name(char *system_name, tbl_lldp_global_t* p_lldp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
   
    /*call API */
    PM_E_RETURN(lldp_set_system_name(system_name));

    return PM_E_NONE;
}

static int32
_lldp_cmd_set_system_desc(char *system_desc, tbl_lldp_global_t* p_lldp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    
    /*call API */
    PM_E_RETURN(lldp_set_system_desc(system_desc));

    return PM_E_NONE;
}

static int32
_lldp_cmd_set_mgmt_ip(addr_t *addr, tbl_lldp_global_t* p_lldp_glb, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    /* 1. config check */
    
    /* 2. check duplicate config */
    if (0 == sal_memcmp(addr, &p_lldp_glb->management_ip, sizeof(addr_t)))
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(lldp_set_system_mgmt_ip(addr));

    return PM_E_NONE;
}

static int32
_lldp_cmd_if_set_basic_tvl(tbl_lldp_if_t* p_lldp_if, tbl_lldp_if_t* p_db_lldp_if, uint32 value, uint32 is_add, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];
    char ifname[IFNAME_EXT_SIZE];
    tbl_interface_t *p_db_if = NULL;
    uint32 old_is_add = FALSE;
    
    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_lldp_if->key.ifindex);
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }

    /* 1. config check */
    tbl_interface_get_name_by_ifindex(p_lldp_if->key.ifindex, ifname, IFNAME_SIZE);
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(ifname, ifname_ext));
    }
    IF_MGR_CHECK_LAG_GROUP(p_db_if);
    
    /* 2. check duplicate config */

    old_is_add = GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_basic, value);
    if (is_add)
    {
        if (old_is_add == is_add)
        {
            return PM_E_NONE;
        }
        else
        {
            GLB_SET_BIT(p_db_lldp_if->tlv_enable_basic, value);
        }
    }
    else
    {
        if (old_is_add == is_add)
        {
            CFG_CONFLICT_RET("The basic tlv %s is not exist already", lldp_basic_str[value]);
        }
        else
        {
            GLB_CLEAR_BIT(p_db_lldp_if->tlv_enable_basic, value);
        }
    }
    
    /* 3. call API */
    PM_E_RETURN(lldp_set_if_set_basic_tvl(p_db_lldp_if));

    return rc;
}

static int32
_lldp_cmd_if_set_med_tvl(tbl_lldp_if_t* p_lldp_if, tbl_lldp_if_t* p_db_lldp_if, uint32 value, uint32 is_add, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];
    char ifname[IFNAME_EXT_SIZE];
    tbl_interface_t *p_db_if = NULL;
    uint32 old_is_add = FALSE;
    
    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_lldp_if->key.ifindex);
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }

    /* 1. config check */
    tbl_interface_get_name_by_ifindex(p_lldp_if->key.ifindex, ifname, IFNAME_SIZE);
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(ifname, ifname_ext));
    }
    IF_MGR_CHECK_LAG_GROUP(p_db_if);
    
    /* 2. check duplicate config */
    old_is_add = GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_med, value);
    if (is_add)
    {
        if (old_is_add == is_add && (value != MED_TLV_EN_LOCATION_ID))
        {
            CFG_CONFLICT_RET("The med tlv %s is exist now", lldp_med_str[value]);
        }
        else
        {
            GLB_SET_BIT(p_db_lldp_if->tlv_enable_med, value);
        }
        
    }
    else
    {
        if (old_is_add == is_add)
        {
            CFG_CONFLICT_RET("The med tlv %s is not exist already", lldp_med_str[value]);
        }
        else
        {
            GLB_CLEAR_BIT(p_db_lldp_if->tlv_enable_med, value);
        }
    }

    /* 3. call API */
    PM_E_RETURN(lldp_set_if_set_med_tvl(p_db_lldp_if));

    return rc;
}

static int32
_lldp_cmd_if_set_8021_org_specific_tvl(tbl_lldp_if_t* p_lldp_if, tbl_lldp_if_t* p_db_lldp_if, uint32 value, uint32 is_add, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];
    char ifname[IFNAME_EXT_SIZE];
    tbl_interface_t *p_db_if = NULL;
    uint32 old_is_add = FALSE;
    
    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_lldp_if->key.ifindex);
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }

    /* 1. config check */
    tbl_interface_get_name_by_ifindex(p_lldp_if->key.ifindex, ifname, IFNAME_SIZE);
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(ifname, ifname_ext));
    }
    IF_MGR_CHECK_LAG_GROUP(p_db_if);
    
    /* 2. check duplicate config */

    old_is_add = GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, value);
    if (is_add)
    {
        if (value == IEEE_8021_TLV_EN_LINK_AGG_8021)
        {
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8023, IEEE_8023_TLV_EN_LINK_AGG_8023))
            {
                 CFG_CONFLICT_RET("Link aggregation TLV is now enabled in Other tlv set, please cancel it before");
            }
        }
        if (old_is_add == is_add)
        {
            return PM_E_NONE;
        }
        else
        {
            GLB_SET_BIT(p_db_lldp_if->tlv_enable_8021, value);
        }
    }
    else
    {
        if (old_is_add == is_add)
        {
            CFG_CONFLICT_RET("The 8021_org_specific tlv %s is not exist already", lldp_8021_str[value]);
        }
        else
        {
            GLB_CLEAR_BIT(p_db_lldp_if->tlv_enable_8021, value);
        }
    }
    
    
    /* 3. call API */
    PM_E_RETURN(lldp_set_if_set_8021_org_specific_tvl(p_db_lldp_if));

    return rc;
}

static int32
_lldp_cmd_if_set_8021_org_specific_vlan_id(tbl_lldp_if_t* p_lldp_if, tbl_lldp_if_t* p_db_lldp_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];
    char ifname[IFNAME_EXT_SIZE];
    tbl_interface_t *p_db_if = NULL;
    
    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_lldp_if->key.ifindex);
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }

    /* 1. config check */
    tbl_interface_get_name_by_ifindex(p_lldp_if->key.ifindex, ifname, IFNAME_SIZE);
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(ifname, ifname_ext));
    }
    IF_MGR_CHECK_LAG_GROUP(p_db_if);
    
    /* 2. check duplicate config */
    if (p_lldp_if->vlan_name_id == p_db_lldp_if->vlan_name_id)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    PM_E_RETURN(lldp_set_if_set_8021_org_specific_vlan_id(p_lldp_if, p_lldp_if->vlan_name_id));

    return rc;
    
}

static int32
_lldp_cmd_if_set_8023_org_specific_tvl(tbl_lldp_if_t* p_lldp_if, tbl_lldp_if_t* p_db_lldp_if, uint32 value, uint32 is_add, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];
    char ifname[IFNAME_EXT_SIZE];
    tbl_interface_t *p_db_if = NULL;
    uint32 old_is_add = FALSE;
    
    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_lldp_if->key.ifindex);
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }

    /* 1. config check */
    tbl_interface_get_name_by_ifindex(p_lldp_if->key.ifindex, ifname, IFNAME_SIZE);
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(ifname, ifname_ext));
    }
    IF_MGR_CHECK_LAG_GROUP(p_db_if);
    
    /* 2. check duplicate config */

    old_is_add = GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8023, value);
    if (is_add)
    {
        if (value == IEEE_8023_TLV_EN_LINK_AGG_8023)
        {
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_LINK_AGG_8021))
            {
                 CFG_CONFLICT_RET("Link aggregation TLV is now enabled in Other tlv set, please cancel it before");
            }
        }
        if (old_is_add == is_add)
        {
            return PM_E_NONE;
        }
        else
        {
            GLB_SET_BIT(p_db_lldp_if->tlv_enable_8023, value);
        }
    }
    else
    {
        if (old_is_add == is_add)
        {
            CFG_CONFLICT_RET("The 8023_org_specific tlv %s is not exist already", lldp_8023_str[value]);
        }
        else
        {
            GLB_CLEAR_BIT(p_db_lldp_if->tlv_enable_8023, value);
        }
    }
    
    
    /* 3. call API */
    PM_E_RETURN(lldp_set_if_set_8023_org_specific_tvl(p_db_lldp_if));

    return rc;
}

static int32
 _lldp_cmd_if_set_med_location_id_tvl(tbl_lldp_if_t* p_lldp_if, tbl_lldp_if_t* p_db_lldp_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
    char ifname_ext[IFNAME_EXT_SIZE];
    char ifname[IFNAME_EXT_SIZE];
    tbl_interface_t *p_db_if = NULL;
    
    p_db_if = tbl_interface_get_interface_by_ifindex(p_db_lldp_if->key.ifindex);
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }

    /* 1. config check */
    tbl_interface_get_name_by_ifindex(p_lldp_if->key.ifindex, ifname, IFNAME_SIZE);
    if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("Interface %s is not physical port", IFNAME_ETH2FULL(ifname, ifname_ext));
    }
    IF_MGR_CHECK_LAG_GROUP(p_db_if);

    /* 2. check duplicate config */
    
    /* 3. call API */
    PM_E_RETURN(lldp_set_if_set_med_location_id_tvl(p_lldp_if));
    
    return rc;
}


int32
lldp_cmd_process_lldp_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    tbl_lldp_global_t* p_lldp_glb = NULL;
    cdb_node_t* p_node = NULL;
    int32 value = 0;
    uint32 parse_field = FALSE;
    uint32 i = 0;
    addr_ipv4_t addr_ipv4;
    addr_t addr;
    char sys_name[LLDP_NAME_SIZE + 1];
    char sys_desc[LLDP_DESCRIPTION_SIZE + 1];/*modified by yejl to fix bug40017*/
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_LLDP_GLOBAL);
    p_lldp_glb = tbl_lldp_global_get_lldp_global();
    if (NULL == p_lldp_glb)
    {
        return PM_E_NONE;
    }
    sal_memset(sys_name, 0, sizeof(sys_name));
    sal_memset(sys_desc, 0, sizeof(sys_desc));
    sal_memset(&addr, 0, sizeof(addr));
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }
        
        switch (field_id)
        {
        case TBL_LLDP_GLOBAL_FLD_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            rc = _lldp_cmd_set_enable(value, p_lldp_glb, para);
            break;

        case TBL_LLDP_GLOBAL_FLD_TX_HOLD:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            rc = _lldp_cmd_set_timer_tx_hold(value, p_lldp_glb, para);
            break;

        case TBL_LLDP_GLOBAL_FLD_TX_INTERVAL:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            rc = _lldp_cmd_set_timer_tx_interval(value, p_lldp_glb, para);
            break;

        case TBL_LLDP_GLOBAL_FLD_TX_DELAY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            rc = _lldp_cmd_set_timer_tx_delay(value, p_lldp_glb, para);
            break;

        case TBL_LLDP_GLOBAL_FLD_REINIT_DELAY:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_UINT(value, argv[1], para->p_rs);
            rc = _lldp_cmd_set_timer_reinit_delay(value, p_lldp_glb, para);
            break;

        case TBL_LLDP_GLOBAL_FLD_SYSTEM_NAME:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 2, para->p_rs);
            if (argc == 2)
            {
                if (check_name_character_and_len(argv[i+1], LLDP_NAME_SIZE) != 0)
                {
                    CFG_INVALID_PARAM_RET("The first character should be a-z or A-Z or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 64.");
                }
                sal_strncpy(sys_name, argv[i+1], LLDP_NAME_SIZE + 1);
            }
            rc = _lldp_cmd_set_system_name(sys_name, p_lldp_glb, para);
            break;

        case TBL_LLDP_GLOBAL_FLD_SYSTEM_DESC:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 2, para->p_rs);
            if (argc == 2)
            {
                if (check_name_character_and_len(argv[i+1], LLDP_DESCRIPTION_SIZE) != 0)
                {
                    CFG_INVALID_PARAM_RET("The first character should be a-z or A-Z or 0-9, character only can be 0-9A-Za-z.-_ and the max len is 255.");
                }
                sal_strncpy(sys_desc, argv[i+1], LLDP_DESCRIPTION_SIZE + 1);/*modified by yejl to fix bug40017*/
            }
            rc = _lldp_cmd_set_system_desc(sys_desc, p_lldp_glb, para);
            break;

        case TBL_LLDP_GLOBAL_FLD_MANAGEMENT_IP:   
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 2, para->p_rs);
            if (argc == 2)
            {
                CFG_PARSER_IP_ADDR(argv[1], &addr_ipv4, para->p_rs);
                addr.family = AF_INET;
                addr.u.prefix4 = addr_ipv4;
                GLB_SET_BIT(p_lldp_glb->config_flag, LLDP_CONF_MGMT_IP_ADDRESS);
            }
            else
            {
                GLB_CLEAR_BIT(p_lldp_glb->config_flag, LLDP_CONF_MGMT_IP_ADDRESS);
            }
            rc = _lldp_cmd_set_mgmt_ip(&addr, p_lldp_glb, para);
            break;
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        
        break;

    case CDB_OPER_GET:
        parse_field = FALSE;
        if (parse_field)
        {
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }
        else
        {
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        }
        rc = _lldp_cmd_get_lldp_global(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_lldp_cmd_if_create(tbl_lldp_if_t *p_lldp_if, tbl_lldp_if_t *p_db_lldp_if, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    if (NULL == p_db_lldp_if)
    {
        rc = lldp_if_create(p_lldp_if);
    }
    else
    {
        rc = lldp_if_set_mode(p_lldp_if, p_db_lldp_if);
    }

    return rc;
}

static int32
_lldp_cmd_if_destory(tbl_lldp_if_t *p_db_lldp_if, tbl_interface_t *p_db_if, cfg_cmd_para_t *para)
{
    tbl_lldp_global_t* p_lldp_glb = tbl_lldp_global_get_lldp_global();
    int32 rc = PM_E_NONE;
    
    if (NULL == p_db_if)
    {
        return PM_E_NOT_EXIST;
    }
    if (p_db_if->l2pro_uplink)
    {
        CFG_INVALID_PARAM_RET("Can't disable lldp on this port because l2protocol uplink is enabled");
    }

    /*modified by yejl to fix bug40072, 2016-08-10*/
    if (p_db_lldp_if->mode == LLDP_IF_TXRX || p_db_lldp_if->mode == LLDP_IF_TXONLY)
    {
        if (p_lldp_glb->enable)
        {
            lldp_pdu_tx_disable(p_db_lldp_if);
        }
    }
    
    rc = lldp_if_destory(p_db_lldp_if);
    
    return rc;
}

static int32
_lldp_cmd_if_get(uint32 ifindex, field_parse_t *p_field, cfg_cmd_para_t *para)
{
    tbl_lldp_if_master_t *p_master = tbl_lldp_if_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_lldp_if_t *p_db_lldp_if = NULL;
    tbl_lldp_if_key_t key;
    tbl_iter_args_t args;
    FILE *fp = NULL;
    int32 rc = PM_E_NONE;


    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    if (ifindex)
    {
        p_db_lldp_if = tbl_lldp_if_get_lldp_if(&key);
        if (p_db_lldp_if)
        {
            tbl_lldp_if_dump_one(p_db_lldp_if, &args);
        }
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_lldp_if, listnode)
        {
            tbl_lldp_if_dump_one(p_db_lldp_if, &args);
        }
    }
    sal_fclose(fp);
    
    return rc;
}

int32
lldp_cmd_process_lldp_if(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    field_parse_t field;
    tbl_lldp_if_t lldp_if;
    tbl_lldp_if_t* p_lldp_if = &lldp_if;
    tbl_lldp_if_t* p_db_lldp_if = NULL;
    tbl_interface_t *p_db_if = NULL;
    int32 ifindex = 0;
    uint32 length = 0;
    int32 value = 0;
    uint32 i = 0;
    uint32 ca_len = 0;
    uint32 is_add = FALSE;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    sal_memset(&lldp_if, 0, sizeof(lldp_if));

    if (CDB_OPER_GET != para->oper)
    {
        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
        p_db_if = tbl_interface_get_interface_by_name(argv[0]);
        if (NULL == p_db_if)
        {
            CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[0]);
        }
        if (GLB_IF_TYPE_PORT_IF != p_db_if->hw_type)
        {
            CFG_INVALID_PARAM_RET("LLDP can only work on physical interface");
        }
    }
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        p_lldp_if->key.ifindex = p_db_if->ifindex;
        p_db_lldp_if = tbl_lldp_if_get_lldp_if(&p_lldp_if->key);
        
        value = cdb_enum_str2val(lldp_if_mode_strs, LLDP_IF_MODE_MAX, argv[1]);
        if (value < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid lldp port mode %s", argv[1]);
        }
        p_lldp_if->mode = value;
        _lldp_cmd_if_create(p_lldp_if, p_db_lldp_if, para);
        break;

    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        p_lldp_if->key.ifindex = p_db_if->ifindex;
        p_db_lldp_if = tbl_lldp_if_get_lldp_if(&p_lldp_if->key);
        if (NULL == p_db_lldp_if)
        {
            return PM_E_NONE;
        }
        _lldp_cmd_if_destory(p_db_lldp_if, p_db_if, para);
        break;
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 4, para->p_rs);
        ifindex = tbl_interface_get_ifindex_by_name(argv[0]);
        if (ifindex <= 0)
        {
            CFG_INVALID_PARAM_RET("Invalid interface name %s", argv[0]);
        }
        p_lldp_if->key.ifindex = ifindex;
        p_db_lldp_if = tbl_lldp_if_get_lldp_if(&p_lldp_if->key);
        if (NULL == p_db_lldp_if)
        {
            CFG_INVALID_PARAM_RET("The LLDP of %s is not enable!", argv[0]);
        }
        if (0 == sal_strcmp(argv[1], "add"))
        {
            is_add = TRUE;
        }
        else if (0 == sal_strcmp(argv[1], "del"))
        {
            is_add = FALSE;
        }
        if (0 == sal_strcmp(argv[2], "basic"))
        { 
            if (0 == sal_strcmp(argv[3], "all"))
            {
                p_lldp_if->tlv_enable_basic = BASIC_TLV_EN_ALL;
                for (i = BASIC_TLV_EN_PORT_DESCRIPTION; i < BASIC_TLV_EN_ALL; i++)
                {
                    if (GLB_IS_BIT_SET(p_lldp_if->tlv_enable_basic, i))
                    {
                        value = i;
                        _lldp_cmd_if_set_basic_tvl(p_lldp_if, p_db_lldp_if, value, is_add, para);
                    }
                }
                return PM_E_NONE;
            }
            else if (0 == sal_strcmp(argv[3], "management-address"))
            {
                value = BASIC_TLV_EN_MANAGEMENT_ADDRESS;
            }
            else if (0 == sal_strcmp(argv[3], "port-description"))
            {
                value = BASIC_TLV_EN_PORT_DESCRIPTION;
            }
            else if (0 == sal_strcmp(argv[3], "system-capabilities"))
            {
                value = BASIC_TLV_EN_SYSTEM_CAPABILITIES;
            }
            else if (0 == sal_strcmp(argv[3], "system-description"))
            {
                value = BASIC_TLV_EN_SYSTEM_DESCRIPTION;
            }
            else if (0 == sal_strcmp(argv[3], "system-name"))
            {
                value = BASIC_TLV_EN_SYSTEM_NAME;
            }
            _lldp_cmd_if_set_basic_tvl(p_lldp_if, p_db_lldp_if, value, is_add, para);
        }
        else if (0 == sal_strcmp(argv[2], "med"))
        {
            if (0 == sal_strcmp(argv[3], "all"))
            {
                p_lldp_if->tlv_enable_med = MED_TLV_EN_ALL;
                for (i = MED_TLV_EN_NETWORK_POLICY; i < MED_TLV_EN_ALL; i++)
                {
                    if (GLB_IS_BIT_SET(p_lldp_if->tlv_enable_med, i))
                    {
                        value = i;
                        _lldp_cmd_if_set_med_tvl(p_lldp_if, p_db_lldp_if, value, is_add, para);
                    }
                }
                return PM_E_NONE;
            }
            else if (0 == sal_strcmp(argv[3], "ext-power"))
            {
                value = MED_TLV_EN_EXT_POWER_VIA_MDI;
            }
            else if (0 == sal_strcmp(argv[3], "capbility"))
            {
                value = MED_TLV_EN_CAP;
            }
            else if (0 == sal_strcmp(argv[3], "network-policy"))
            {
                value = MED_TLV_EN_NETWORK_POLICY;
            }
            else if (0 == sal_strcmp(argv[3], "inventory"))
            {
                value = MED_TLV_EN_INVENTORY;
            }
            else if (0 == sal_strcmp(argv[3], "location-id"))
            {
                value = MED_TLV_EN_LOCATION_ID;
                if(argc <= 4)
                {
                    p_lldp_if->med_lci_format = 0;
                    sal_memset(p_lldp_if->med_lci_elin, 0, sizeof(p_lldp_if->med_lci_elin));
                    sal_memset(&p_lldp_if->med_lci_civic, 0, sizeof(p_lldp_if->med_lci_civic));
                }
                else
                {
                    if (0 == sal_strcmp(argv[4], "civic"))
                    {
                        uint32 what = 0;
                        uint32 type_value = 0;
                        uint32 a = 7;
                        char buf[LLDP_MED_LCI_CIVIC_CA];
                        
                        sal_memset(p_lldp_if->med_lci_elin, 0, sizeof(p_lldp_if->med_lci_elin));
                        sal_memset(&p_lldp_if->med_lci_civic, 0, sizeof(p_lldp_if->med_lci_civic));
                        
                        p_lldp_if->med_lci_format = MED_LCI_FORMAT_CIVIC;
                        PM_CFG_GET_UINT(what, argv[5], para->p_rs);
                        p_lldp_if->med_lci_civic.what = what;

                        if (sal_strlen(argv[6]) != LLDP_MED_LCI_CIVIC_COUNTRYCODE_SIZE && 
                            check_name_character_and_len(argv[6], LLDP_MED_LCI_CIVIC_COUNTRYCODE_SIZE) != 0)
                        {
                             CFG_INVALID_PARAM_RET("Wrong Civic country code");
                        }
                        sal_strncpy(p_lldp_if->med_lci_civic.countrycode, argv[6], LLDP_MED_LCI_CIVIC_COUNTRYCODE_SIZE + 1);

                        for (i = 0; i< 10; i++)
                        {
                            sal_memset(buf, 0, sizeof(buf));
                            sal_snprintf (buf, LLDP_MED_LCI_CIVIC_CA, "ca%u", i+1);
                            if (0 == sal_strcmp(argv[a], buf))
                            {
                                a++;
                                if (0 != sal_strcmp(argv[a], "none"))
                                {
                                    PM_CFG_GET_UINT(type_value, argv[a], para->p_rs);
                                    p_lldp_if->med_lci_civic.ca_type[i] = type_value;
                                    a++;
                                    if (check_name_character_and_len(argv[a], LLDP_MED_LCI_CIVIC_VALUE_SIZE) != 0)
                                    {
                                        CFG_INVALID_PARAM_RET("Invalid string!");
                                    }
                                    sal_strncpy(p_lldp_if->med_lci_civic.ca_value[i], argv[a], LLDP_MED_LCI_CIVIC_VALUE_SIZE + 1);
                                    p_lldp_if->med_lci_civic.ca_num ++;
                                    p_lldp_if->med_lci_civic.ca_length[i] = strlen(argv[a]);
                                    a++;
                                }
                                else
                                {
                                    a = a + 2;
                                }
                            } 
                        }
                        
                        for(i = 0; i < p_lldp_if->med_lci_civic.ca_num; i++)
                        {
                            ca_len = ca_len + p_lldp_if->med_lci_civic.ca_length[i];
                        }
                        if (ca_len > LLDP_MED_LCI_CIVIC_VALUE_SIZE)
                        {
                            CFG_INVALID_PARAM_RET("The length of all CA-VALUE can't exceed %d", LLDP_MED_LCI_CIVIC_VALUE_SIZE);
                        }
                        p_lldp_if->med_lci_civic.len = 3 + ca_len + p_lldp_if->med_lci_civic.ca_num * 2;
                        
                    }
                    else if (0 == sal_strcmp(argv[4], "ecs-elin"))
                    {
                        sal_memset(p_lldp_if->med_lci_elin, 0, sizeof(p_lldp_if->med_lci_elin));
                        sal_memset(&p_lldp_if->med_lci_civic, 0, sizeof(p_lldp_if->med_lci_civic));
                        
                        p_lldp_if->med_lci_format = MED_LCI_FORMAT_ECS_ELIN; 
                        length = sal_strlen(argv[5]);
                        if (length < 10 || length > 25)
                        {
                             CFG_INVALID_PARAM_RET("The valid length of ECS ELIN address should be in [10-25]");
                        }
                        sal_strncpy(p_lldp_if->med_lci_elin, argv[5], LLDP_MED_LCI_ELIN_MAX_SIZE);
                    }
                }
                _lldp_cmd_if_set_med_location_id_tvl(p_lldp_if, p_db_lldp_if, para); 
            }
            
            _lldp_cmd_if_set_med_tvl(p_lldp_if, p_db_lldp_if, value, is_add, para);
        }
        else if (0 == sal_strcmp(argv[2], "8021-org-specific"))
        {
            if (0 == sal_strcmp(argv[3], "all"))
            {   
                p_lldp_if->tlv_enable_8021 = IEEE_8021_TLV_EN_ALL;
                for (i = IEEE_8021_TLV_EN_PORT_VLAN_ID; i< IEEE_8021_TLV_EN_ALL; i++)
                {
                    if (GLB_IS_BIT_SET(p_lldp_if->tlv_enable_8021, i))
                    {
                        value = i;
                        _lldp_cmd_if_set_8021_org_specific_tvl(p_lldp_if, p_db_lldp_if, value, is_add, para);
                    }
                }
                return PM_E_NONE;
            }
            else if (0 == sal_strcmp(argv[3], "port-vlan"))
            {
                value = IEEE_8021_TLV_EN_PORT_VLAN_ID;
            }
            else if (0 == sal_strcmp(argv[3], "protocol-id"))
            {
                value = IEEE_8021_TLV_EN_PROTOCOL_ID;
            }
            else if (0 == sal_strcmp(argv[3], "protocol-vlan"))
            {
                value = IEEE_8021_TLV_EN_PROTO_VLAN_ID;
            }
            else if (0 == sal_strcmp(argv[3], "vlan-name"))
            {
                value = IEEE_8021_TLV_EN_PORT_VLAN_NAME;
            }
            else if (0 == sal_strcmp(argv[3], "link-aggregation"))
            {
                value = IEEE_8021_TLV_EN_LINK_AGG_8021;
            }
            else if (0 == sal_strcmp(argv[3], "vlan-name-value"))
            {
                PM_CFG_GET_UINT(value, argv[4], para->p_rs);
                p_lldp_if->vlan_name_id = value;
                _lldp_cmd_if_set_8021_org_specific_vlan_id(p_lldp_if, p_db_lldp_if, para);
                return PM_E_NONE;
            }
            _lldp_cmd_if_set_8021_org_specific_tvl(p_lldp_if, p_db_lldp_if, value, is_add, para);
        }
        else if (0 == sal_strcmp(argv[2], "8023-org-specific"))
        {
            if (0 == sal_strcmp(argv[3], "all"))
            {
                p_lldp_if->tlv_enable_8023 = IEEE_8023_TLV_EN_ALL;
                for (i = IEEE_8023_TLV_EN_MAC_PHY_CONFIG_STATUS; i< IEEE_8023_TLV_EN_ALL; i++)
                {
                    if (GLB_IS_BIT_SET(p_lldp_if->tlv_enable_8023, i))
                    {
                        value = i;
                        _lldp_cmd_if_set_8023_org_specific_tvl(p_lldp_if, p_db_lldp_if, value, is_add, para);
                    }
                }
                return PM_E_NONE;
            }
            else if (0 == sal_strcmp(argv[3], "link-aggregation"))
            {
                value = IEEE_8023_TLV_EN_LINK_AGG_8023;
            }
            else if (0 == sal_strcmp(argv[3], "mac-phy-cfg"))
            {
                value = IEEE_8023_TLV_EN_MAC_PHY_CONFIG_STATUS;
            }
            else if (0 == sal_strcmp(argv[3], "max-frame-size"))
            {
                value = IEEE_8023_TLV_EN_MAX_FRAME_SIZE;
            }
            else if (0 == sal_strcmp(argv[3], "power"))
            {
                value = IEEE_8023_TLV_EN_POWER_VIA_MDI;
            }
           _lldp_cmd_if_set_8023_org_specific_tvl(p_lldp_if, p_db_lldp_if, value, is_add, para);
        }
        break;

    case CDB_OPER_GET:
        if (0 == argc)
        {
            ifindex = GLB_INVALID_IFINDEX;
        }
        else
        {
            PM_CFG_GET_UINT(ifindex, argv[0], para->p_rs);
        }
        sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        rc = _lldp_cmd_if_get(ifindex, &field, para);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_lldp_cmd_show_local_config_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_lldp_global_t *p_db_glb = tbl_lldp_global_get_lldp_global();
    FILE *fp = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    sal_fprintf(fp, "LLDP global configuration:\n");
    sal_fprintf(fp, "-------------------------+-------------------\n");
    sal_fprintf(fp, "%-25s: %-s\n", "LLDP function global enabled", (p_db_glb->enable ? "YES" : "NO"));
    sal_fprintf(fp, "%-25s: %-u\n", "LLDP TxHold", p_db_glb->tx_hold);
    sal_fprintf(fp, "%-25s: %-us\n", "LLDP TxInterval", p_db_glb->tx_interval);
    sal_fprintf(fp, "%-25s: %-us\n", "LLDP ReinitDelay", p_db_glb->reinit_delay);
    sal_fprintf(fp, "%-25s: %-us\n", "LLDP TxDelay", p_db_glb->tx_delay);
    sal_fprintf(fp, "\n");

    sal_fclose(fp);
    
    return PM_E_NONE;
}

static int32
_lldp_cmd_show_local_config_interface(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_lldp_if_key_t key;
    tbl_interface_t *p_db_if = NULL;
    tbl_lldp_if_t *p_db_lldp_if = NULL;
    FILE *fp = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];

    if (NULL == argv[1])
    {
        return PM_E_NONE;
    }
    p_db_if = tbl_interface_get_interface_by_name(argv[1]);
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }        
    key.ifindex = p_db_if->ifindex;
    p_db_lldp_if = tbl_lldp_if_get_lldp_if(&key);
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_fprintf(fp, "LLDP configuration on interface %s:\n", IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
    sal_fprintf(fp, "-------------------------+-------------------\n");
    if (NULL == p_db_lldp_if)
    {
        sal_fprintf(fp, "%-25s: %-s\n", "LLDP admin status", "Disabled");
    }
    else
    {
        sal_fprintf(fp, "%-25s: %-s\n", "LLDP admin status",
            cdb_enum_val2str(lldp_if_mode_cli_strs, LLDP_IF_MODE_MAX, p_db_lldp_if->mode));

        if (p_db_lldp_if->tlv_enable_basic)
        {
            sal_fprintf(fp, "Basic optional TLV Enabled:\n");
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_basic, BASIC_TLV_EN_PORT_DESCRIPTION))
            {
                sal_fprintf(fp, "    Port Description TLV\n");            
            }
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_basic, BASIC_TLV_EN_SYSTEM_NAME))
            {
                sal_fprintf(fp, "    System Name TLV\n");            
            }
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_basic, BASIC_TLV_EN_SYSTEM_DESCRIPTION))
            {
                sal_fprintf(fp, "    System Description TLV\n");            
            }
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_basic, BASIC_TLV_EN_SYSTEM_CAPABILITIES))
            {
                sal_fprintf(fp, "    System Capabilities TLV\n");            
            }
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_basic, BASIC_TLV_EN_MANAGEMENT_ADDRESS))
            {
                sal_fprintf(fp, "    Management Address TLV\n");            
            }
            sal_fprintf(fp, "\n");        
        }

        if (p_db_lldp_if->tlv_enable_8021)
        {
            sal_fprintf(fp, "IEEE 802.1 TLV Enabled:\n");
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_PORT_VLAN_ID))
            {
                sal_fprintf(fp, "    Port Vlan ID TLV\n");            
            }
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_PROTO_VLAN_ID))
            {
                sal_fprintf(fp, "    Port and Protocol Vlan ID TLV\n");            
            }
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_PORT_VLAN_NAME))
            {
                sal_fprintf(fp, "    Vlan Name TLV\n");            
            }
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_PROTOCOL_ID))
            {
                sal_fprintf(fp, "    Protocol Identity TLV\n");            
            }
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_LINK_AGG_8021))
            {
                sal_fprintf(fp, "    Link Aggregation TLV\n");            
            }
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_DCBX))
            {
                sal_fprintf(fp, "    DCBX TLV\n");            
            }
            sal_fprintf(fp, "\n");
        }

        if (p_db_lldp_if->tlv_enable_8023)
        {
            sal_fprintf(fp,  "IEEE 802.3 TLV Enabled:\n");
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8023, IEEE_8023_TLV_EN_MAC_PHY_CONFIG_STATUS))
            {
                sal_fprintf(fp, "    MAC/PHY Configuration/Status TLV\n");            
            }
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8023, IEEE_8023_TLV_EN_POWER_VIA_MDI))
            {
                sal_fprintf(fp, "    Power Via MDI TLV\n");            
            }
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8023, IEEE_8023_TLV_EN_LINK_AGG_8023))
            {
                sal_fprintf(fp, "    Link Aggregation TLV\n");             
            }
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8023, IEEE_8023_TLV_EN_MAX_FRAME_SIZE))
            {
                sal_fprintf(fp, "    Maximum Frame Size TLV\n");            
            }
            sal_fprintf(fp, "\n");
        }

        if (p_db_lldp_if->tlv_enable_med)
        {
            sal_fprintf(fp,  "LLDP-MED TLV Enabled:\n");
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_med, MED_TLV_EN_CAP))
            {
                sal_fprintf(fp, "    Med Capabilities TLV\n");            
            }
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_med, MED_TLV_EN_NETWORK_POLICY))
            {
                sal_fprintf(fp, "    Network Policy TLV\n");            
            }
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_med, MED_TLV_EN_LOCATION_ID))
            {
                sal_fprintf(fp, "    Location Identification TLV\n");             
            }
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_med, MED_TLV_EN_EXT_POWER_VIA_MDI))
            {
                sal_fprintf(fp, "    Extended Power-via-MDI TLV\n");            
            }
            if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_med, MED_TLV_EN_INVENTORY))
            {
                sal_fprintf(fp, "    Inventory TLV\n");            
            }
            sal_fprintf(fp, "\n");
        }
    }
    sal_fprintf(fp, "\n");

    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_lldp_cmd_show_local_tlv_info_global(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_version_t *p_db_version = tbl_version_get_version();
    tbl_card_t *p_db_card = tbl_card_get_default_card();
    tbl_lldp_global_t *p_db_glb = tbl_lldp_global_get_lldp_global();
    char management_ip_str[CMD_BUF_32];
    char system_name[LLDP_NAME_SIZE+1];
    char *system_desc = NULL;
    FILE *fp = NULL;
    uint32 i = 0;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (p_db_card == NULL)
    {   
        return PM_E_NONE;
    }
    if (p_db_version == NULL)
    {   
        return PM_E_NONE;
    }
    if (p_db_glb == NULL)
    {   
        return PM_E_NONE;
    }
    
    system_name[0] = CMD_ZERO_CHAR;
    if (sal_strlen(p_db_glb->system_name))
    {
        sal_strcpy(system_name, p_db_glb->system_name);
    }
    else
    {
        gethostname(system_name, sizeof(system_name));
    }

    if (sal_strlen(p_db_glb->system_desc))
    {
        system_desc = p_db_glb->system_desc;
    }
    else
    {
        system_desc = p_db_glb->system_desc_default;
    }

    sal_fprintf(fp, "LLDP global TLV information:\n");
    sal_fprintf(fp, "-------------------------+-------------------\n");
    sal_fprintf(fp, "%-25s: %-s\n", "System Name", system_name);
    sal_fprintf(fp, "\n");
    sal_fprintf(fp, "%-25s: %-s\n", "System Description",system_desc);
    sal_fprintf(fp, "\n");
    sal_fprintf(fp, "%-25s:\n", "System Capabilities");
    for (i = 0; i < LLDP_SYS_CAP_MAX; i++)
    {
        if (GLB_IS_BIT_SET(p_db_glb->system_capability, i))
        {
            sal_fprintf(fp, "    %s     :  ", lldp_sys_cap_strs[i]);
            if (p_db_glb->enable)
            {
                sal_fprintf(fp, "%s\n", "Enabled");
            }
            else
            {
                sal_fprintf(fp, "\n");
            }
        }
    }
    sal_fprintf(fp, "\n");
    
    if (GLB_IS_BIT_SET(p_db_glb->config_flag, LLDP_CONF_MGMT_IP_ADDRESS))
    {
        cdb_addr_val2str(management_ip_str, CMD_BUF_32, &p_db_glb->management_ip);
        sal_fprintf(fp, "%-25s: %-s\n", "Configured Management IP Address", management_ip_str);
    }
    else
    {
         sal_fprintf(fp, "%-25s: %-s\n", "Configured Management IP Address", "");
    }
    sal_fprintf(fp, "\n");

    sal_fprintf(fp, "%-25s:\n", "LLDP MED Inventory Information");
    sal_fprintf(fp, "%-25s: %-s\n", "  Hardware Rivision", p_db_version->hw_type);
    sal_fprintf(fp, "%-25s: %-s\n", "  Firmware Rivision", p_db_card->hardware_ver);
    sal_fprintf(fp, "%-25s: %-s\n", "  Software Rivision", p_db_version->version);
    sal_fprintf(fp, "%-25s: %-s\n", "  Serial Number", p_db_card->serial_no);
    sal_fprintf(fp, "%-25s: %-s\n", "  Manufacturer Name", p_db_version->company);
    sal_fprintf(fp, "%-25s: %-s\n", "  Model Name", p_db_version->product);
    sal_fprintf(fp, "%-25s: %-s\n", "  Asset ID", "");
    sal_fprintf(fp, "\n");

    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_lldp_cmd_show_local_tlv_info_interface(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_lldp_if_key_t key;
    tbl_interface_t *p_db_if = NULL;
    tbl_lldp_if_t *p_db_lldp_if = NULL;
    vid_t pvid = 0;
    uint32 i = 0;
    uint32 type = 0;
    char ifname_ext[IFNAME_SIZE];
    tbl_mstp_global_t *p_glb = tbl_mstp_global_get_mstp_global();
    FILE *fp = NULL;

    if (NULL == argv[1])
    {
        return PM_E_NONE;
    }
    p_db_if = tbl_interface_get_interface_by_name(argv[1]);
    if (NULL == p_db_if)
    {
        return PM_E_NONE;
    }        
    key.ifindex = p_db_if->ifindex;
    p_db_lldp_if = tbl_lldp_if_get_lldp_if(&key);

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);

    pvid = (p_db_if->brgif) ? p_db_if->brgif->pvid : 0;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (NULL == p_db_lldp_if)
    {
        sal_fprintf(fp, "The lldp of %s is not enable\n", ifname_ext);
    }
    else
    {
        sal_fprintf(fp, "LLDP TLV information on interface %s:\n", p_db_if->key.name);
        sal_fprintf(fp, "-------------------------+-------------------\n");
        
        sal_fprintf(fp, "%-25s: %-u\n", "Port Vlan ID", pvid);
        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "%-25s:\n", "Protocol identities");
        p_db_lldp_if->protocol_id = 0;
        switch (p_glb->type)
        {
        case MSTP_TYPE_STP:
            GLB_SET_BIT(p_db_lldp_if->protocol_id, LLDP_PROTO_STP);
            break;
        case MSTP_TYPE_RSTP:
            GLB_SET_BIT(p_db_lldp_if->protocol_id, LLDP_PROTO_RSTP);
            break;
        case MSTP_TYPE_MSTP:
            GLB_SET_BIT(p_db_lldp_if->protocol_id, LLDP_PROTO_MSTP);
            break;
        }
        if (INTERFACE_IS_LAG_GROUP(p_db_if) || INTERFACE_IS_LAG_MEMBER(p_db_if))
        {
            GLB_SET_BIT(p_db_lldp_if->protocol_id, LLDP_PROTO_LACP);
        }
        if (p_db_if->dot1x_enable)
        {
            GLB_SET_BIT(p_db_lldp_if->protocol_id, LLDP_PROTO_DOT1X);
        }
        for (i = LLDP_PROTO_STP; i < LLDP_PROTO_MAX; i++)
        {
            if (GLB_IS_BIT_SET(p_db_lldp_if->protocol_id, i))
            {
                sal_fprintf(fp, "    %s\n", proto_id_strs[i]);
            }            
        }
        sal_fprintf(fp, "\n");

        if (p_db_if->lag)
        {
            sal_fprintf(fp, "%-25s: %-s", "Link Aggregation status", "Supported");
            sal_fprintf(fp, ", %s\n", "Enabled");
            sal_fprintf(fp, "%-25s: %-u\n", "Link Aggregation Port ID", p_db_if->lag->lag_id + 2048);
        }
        else
        {
             sal_fprintf(fp, "%-25s: %-s\n", "Link Aggregation status", "Supported");
        }
        sal_fprintf(fp, "\n");

        /*802.3*/
        sal_fprintf(fp, "%-25s:\n", "MAC/PHY Configuration/Status");
        if (p_db_if->speed == GLB_SPEED_AUTO || p_db_if->duplex == GLB_DUPLEX_AUTO)
        {
            sal_fprintf(fp, "%-25s: %-s", "AutoNego Support","Supported");
            sal_fprintf(fp, ", %s\n", "Enabled");
        }
        else 
        {
            sal_fprintf(fp, "%-25s: %-s\n", "AutoNego Support","Supported");
        }
        sal_fprintf(fp, "%-25s: %-u\n", "AutoNego Capability",lldp_autoneg_capability_get(p_db_if));
        sal_fprintf(fp, "%-25s: %-u\n", "Operational MAU Type",lldp_oper_mau_type_get(p_db_if));
        sal_fprintf(fp, "\n");
        
        sal_fprintf(fp, "%-25s:\n", "Power via MDI");
        sal_fprintf(fp, "%-25s: %-s\n", "Port class", (p_db_lldp_if->mdi_support & 0x01) ? "PSE" : "PD");
        sal_fprintf(fp, "%-25s: %-s\n", "PSE MDI power support", (p_db_lldp_if->mdi_support & 0x02) ? "YES" : "NO");
        sal_fprintf(fp, "%-25s: %-s\n", "PSE MDI power state", (p_db_lldp_if->mdi_support & 0x04) ? "Enabled" : "Disabled");
        sal_fprintf(fp, "%-25s: %-s\n", "PSE pairs control ability", (p_db_lldp_if->mdi_support & 0x08) ? "YES" : "NO");
        if (LLDP_PSE_POWER_PAIR_SIGNAL == p_db_lldp_if->pse_power_pair)
        {
            sal_fprintf(fp, "%-25s: %-s\n", "PSE power pair", "Signal");
        }
        else if (LLDP_PSE_POWER_PAIR_SPARE == p_db_lldp_if->pse_power_pair)
        {
            sal_fprintf(fp, "%-25s: %-s\n", "PSE power pair", "Spare");
        }
        else
        {
            sal_fprintf(fp, "%-25s: %-s\n", "PSE power pair", "Unknown");
        }

        if (p_db_lldp_if->power_class > LLDP_POWER_CLASS_UN && p_db_lldp_if->power_class < LLDP_POWER_CLASS_MAX)
        {
            sal_fprintf(fp, "%-25s: %-u\n", "Power class", p_db_lldp_if->power_class - 1);
        }
        else
        {
            sal_fprintf(fp, "%-25s: %-s\n", "Power class", "Unknown");
        }
        sal_fprintf(fp, "\n");
        
        /*Maximum Frame Size*/
        if (p_db_if->jumboframe_en)
        {
            sal_fprintf(fp, "%-25s: %-u\n", "Maximum Frame Size", GLB_DFT_MAX_FRAME_SIZE_1);
        }
        else
        {
            sal_fprintf(fp, "%-25s: %-u\n", "Maximum Frame Size", GLB_DFT_MAX_FRAME_SIZE_0);
        }
        sal_fprintf(fp, "\n");
        
        /*med*/
        sal_fprintf(fp, "%-25s\n", "LLDP-MED TLV");
        /*1. Network Policy*/
        sal_fprintf(fp, "%-25s:\n", "Network Policy");
        type = LLDP_MED_NP_GET_APP_TYPE(p_db_lldp_if->med_policy_flag);
        if (type >= LLDP_MED_NP_TYPE_MAX)
        {
            type = 0;
        }
        sal_fprintf(fp, "%-25s: %-s\n", "Application Type", lldp_med_np_app_str[type]);
        
        if (LLDP_MED_NP_GET_U_VALUE(p_db_lldp_if->med_policy_flag))
        {
            sal_fprintf(fp, "%-25s: %-s\n", "Status", "Unknown");
        }
        else
        {
            sal_fprintf(fp, "%-25s: %-s\n", "Status", "Configured");
            if (LLDP_MED_NP_GET_T_VALUE(p_db_lldp_if->med_policy_flag))
            {
                sal_fprintf(fp, "%-25s: %-u\n", "Vlan ID", LLDP_MED_NP_GET_VLAN_ID(p_db_lldp_if->med_policy_flag));
                sal_fprintf(fp, "%-25s: %-u\n", "L2 Priority", LLDP_MED_NP_GET_L2_PRIORITY(p_db_lldp_if->med_policy_flag));
            }
            sal_fprintf(fp, "%-25s: %-u\n", "DSCP", LLDP_MED_NP_GET_DSCP(p_db_lldp_if->med_policy_flag));          
        }
        sal_fprintf(fp, "\n");
        
        /*2.Extended Power-via-MDI*/
        sal_fprintf(fp, "%-25s:\n", "Extended Power-via-MDI");
        if (LLDP_MED_POW_GET_TYPE(p_db_lldp_if->med_power_flag) == MED_TYPE_POWER_FLAG_TYPE_PD)
        {
            sal_fprintf(fp, "%-25s: %-s\n", "type", "PD");
            if (LLDP_MED_POW_GET_SRC(p_db_lldp_if->med_power_flag) == MED_TYPE_POWER_FLAG_TYPE_PD_SRC_PSE)
            {
                sal_fprintf(fp, "%-25s: %-s\n", "Power Source", "PSE");
            }
            else if (LLDP_MED_POW_GET_SRC(p_db_lldp_if->med_power_flag) == MED_TYPE_POWER_FLAG_TYPE_PD_SRC_LOCAL)
            {
                 sal_fprintf(fp, "%-25s: %-s\n", "Power Source", "Local");
            }
            else if (LLDP_MED_POW_GET_SRC(p_db_lldp_if->med_power_flag) == MED_TYPE_POWER_FLAG_TYPE_PD_SRC_ALL)
            {
                 sal_fprintf(fp, "%-25s: %-s\n", "Power Source", "PSE and Local");
            }
            else
            {
                 sal_fprintf(fp, "%-25s: %-s\n", "Power Source", "Unknown");
            }   
            sal_fprintf(fp, "%-25s: %-d\n", "Power Priority", LLDP_MED_POW_GET_PRIORITY(p_db_lldp_if->med_power_flag));
            sal_fprintf(fp, "%-25s: %0.1f%s\n", "Power Value", (float)p_db_lldp_if->med_power_value/(float)10, "W");
        }
        else if (LLDP_MED_POW_GET_TYPE(p_db_lldp_if->med_power_flag) == MED_TYPE_POWER_FLAG_TYPE_PSE)
        {
            sal_fprintf(fp, "%-25s: %-s\n", "type", "PSE");
            if (LLDP_MED_POW_GET_SRC(p_db_lldp_if->med_power_flag) == MED_TYPE_POWER_FLAG_TYPE_PSE_SRC_PRI)
            {
                sal_fprintf(fp, "%-25s: %-s\n", "Power Source", "Primary");
            }
            else if (LLDP_MED_POW_GET_SRC(p_db_lldp_if->med_power_flag) == MED_TYPE_POWER_FLAG_TYPE_PSE_SRC_BAK)
            {
                 sal_fprintf(fp, "%-25s: %-s\n", "Power Source", "Backup");
            }
            else
            {
                 sal_fprintf(fp, "%-25s: %-s\n", "Power Source", "Unknown");
            }   
            sal_fprintf(fp, "%-25s: %-d\n", "Power Priority", LLDP_MED_POW_GET_PRIORITY(p_db_lldp_if->med_power_flag));
            sal_fprintf(fp, "%-25s: %0.1f%s\n", "Power Value", (float)p_db_lldp_if->med_power_value/(float)10, "W"); 
        }
        else
        {
            sal_fprintf(fp, "%-25s: %-s\n", "type", "Unknown");
        }
        sal_fprintf(fp, "\n");

        /*3.location id*/
        sal_fprintf(fp, "%-25s:\n", "Location Identification");
        if (MED_LCI_FORMAT_CIVIC == p_db_lldp_if->med_lci_format)
        {
            sal_fprintf(fp, "%-25s:\n", "  Civic Address");
            sal_fprintf(fp, "  %s: %-d\n", "Address reference", p_db_lldp_if->med_lci_civic.what);
            sal_fprintf(fp, "  %s: %-s\n", "Country code", p_db_lldp_if->med_lci_civic.countrycode);
            for (i = 0; i < p_db_lldp_if->med_lci_civic.ca_num; i++)
            {
                sal_fprintf(fp, "  %s: %-d,", "CA-Type", p_db_lldp_if->med_lci_civic.ca_type[i]);
                sal_fprintf(fp, "  %s: %-s\n", "CA-Value", p_db_lldp_if->med_lci_civic.ca_value[i]);
            }
        }
        else if (MED_LCI_FORMAT_ECS_ELIN == p_db_lldp_if->med_lci_format)
        {
            sal_fprintf(fp, "%-25s: %-s\n", "  ECS ELIN", p_db_lldp_if->med_lci_elin);
        }
        sal_fprintf(fp, "\n");
    }

    sal_fclose(fp);

    return PM_E_NONE;
}

int32
lldp_cmd_process_show_local(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (0 == argc)
        {
            CFG_INVALID_PARAM_RET("Need show type parameter");
        }
        else if (1 <= argc)
        {
            if (0 == sal_strcmp(argv[0], "config"))
            {
                if (1 == argc)
                {
                    rc = _lldp_cmd_show_local_config_global(argv, argc, para);
                }
                else
                {
                    rc = _lldp_cmd_show_local_config_interface(argv, argc, para);
                }
            }
            else if (0 == sal_strcmp(argv[0], "tlv_info"))
            {
                if (1 == argc)
                {
                    rc = _lldp_cmd_show_local_tlv_info_global(argv, argc, para);
                }
                else
                {
                    rc = _lldp_cmd_show_local_tlv_info_interface(argv, argc, para);
                }
            }
            else
            {
                CFG_INVALID_PARAM_RET("Invalid show type %s", argv[0]);
            }
        }
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

char*
lldp_chassis_id_val2str(char *str, uint32 str_len, lldp_chassis_id_t *p_chassis_id)
{
    uint8 *mac = NULL;

    str[0] = CMD_ZERO_CHAR;
    if (LLDP_CHASSIS_ID_SUBTYPE_MAC_ADDRESS == p_chassis_id->type)
    {
        mac = (uint8*)p_chassis_id->id;
        sal_sprintf(str, "%.04hX.%.04hX.%.04hX",
            sal_ntoh16(((uint16*)mac)[0]),
            sal_ntoh16(((uint16*)mac)[1]), 
            sal_ntoh16(((uint16*)mac)[2]));
    }
    
    return str;
}

char*
lldp_port_id_val2str(char *str, uint32 str_len, lldp_port_id_t *p_port_id)
{
    char *name = NULL;

    str[0] = CMD_ZERO_CHAR;
    if (LLDP_PORT_ID_SUBTYPE_IF_NAME == p_port_id->type)
    {
        name = (char*)p_port_id->id;
        sal_sprintf(str, "%s", name);
    }

    return str;
}

static int32
_lldp_cmd_process_show_neighbor_brief(tbl_interface_t *p_db_if, ds_lldp_neighbour_t *p_db_neigh, FILE *fp)
{
    char ifname_ext[IFNAME_SIZE];
    char chassis_id_str[CMD_BUF_32];
    char port_id_str[CMD_BUF_32];
    int32 expire_time = 0;
    char *system_name = NULL;
    uint32 first_neigh = TRUE;

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    if (p_db_neigh->rx_ttl_timer)
    {
        expire_time = ctc_timer_get_remain_sec(p_db_neigh->rx_ttl_timer);
    }

    lldp_chassis_id_val2str(chassis_id_str, CMD_BUF_32, &p_db_neigh->key.chassis_id);
    lldp_port_id_val2str(port_id_str, CMD_BUF_32, &p_db_neigh->key.port_id);
    system_name = (p_db_neigh->system_name) ? p_db_neigh->system_name : "-";

    sal_fprintf(fp, "%-25s: %-s\n", "Local Port", ifname_ext);
    sal_fprintf(fp, "%-25s: %-s\n", "ChassisID", chassis_id_str);
    sal_fprintf(fp, "%-25s: %-s\n", "Remote Port", port_id_str);
    sal_fprintf(fp, "%-25s: %-u\n", "HoldTime", p_db_neigh->rx_ttl);
    sal_fprintf(fp, "%-25s: %-u\n", "ExpireTime", expire_time);
    sal_fprintf(fp, "%-25s: %-s\n", "System Name", system_name);

    sal_fprintf(fp, "\n");
    if (first_neigh)
    {
        ifname_ext[0] = CMD_ZERO_CHAR;
    }
    return PM_E_NONE;
}


static int32
_lldp_cmd_process_show_neighbor_interface(tbl_interface_t *p_db_if, ds_lldp_neighbour_t *p_db_neigh, FILE *fp)
{
    char ip_addr[INET_NTOP_BUFSIZ];
    char chassis_id_str[CMD_BUF_32];
    char port_id_str[CMD_BUF_32];
    int32 expire_time = 0;
    uint8 proto_id = 0;
    lldp_protocol_identity_t* proto = NULL;
    uint32 i = 0;
    
    sal_fprintf(fp, "Neighbor LLDP Information\n");
    
    /* chassis id*/
    sal_fprintf(fp, "%-25s: %-s\n", "Chassis ID type", lldp_chassis_id_type[p_db_neigh->key.chassis_id.type]);
    sal_fprintf(fp, "%-25s: %-s\n", "Chassis ID", lldp_chassis_id_val2str(chassis_id_str, CMD_BUF_32, &p_db_neigh->key.chassis_id));

    /* port id*/
    sal_fprintf(fp, "%-25s: %-s\n", "Port ID type", lldp_port_id_type[p_db_neigh->key.port_id.type]);
    sal_fprintf(fp, "%-25s: %-s\n", "Port ID", lldp_port_id_val2str(port_id_str, CMD_BUF_32, &p_db_neigh->key.port_id));
    sal_fprintf(fp, "\n");
    
    /* ttl */
    sal_fprintf(fp, "%-25s: %-u\n", "TTL", p_db_neigh->rx_ttl);
    if (p_db_neigh->rx_ttl_timer)
    {
        expire_time = ctc_timer_get_remain_sec(p_db_neigh->rx_ttl_timer);
    }
    sal_fprintf(fp, "%-25s: %-u\n", "ExpireTime", expire_time);
    sal_fprintf(fp, "\n");
    
    /* basic*/
    if (p_db_neigh->system_name)
    {
        sal_fprintf(fp, "%-25s: %-s\n", "System Name", p_db_neigh->system_name);
    }
    if (p_db_neigh->system_desc)
    {
        sal_fprintf(fp, "%-25s: %-s\n", "System Description", p_db_neigh->system_desc);
    }
    if (p_db_neigh->port_desc)
    {
        sal_fprintf(fp, "%-25s: %-s\n", "Port Description", p_db_neigh->port_desc);
    }
    sal_fprintf(fp, "\n");
    
    if (p_db_neigh->sys_cap)
    {
        sal_fprintf(fp, "%-25s:\n", "System Capabilities");
        for (i = 0; i < LLDP_SYS_CAP_MAX; i++)
        {
            if (GLB_IS_BIT_SET(p_db_neigh->sys_cap, i))
            {
                sal_fprintf(fp, "      %-s  :  ", lldp_sys_cap_strs[i]);
                if (p_db_neigh->sys_cap_en)
                {
                    sal_fprintf(fp, "%-s\n", "Enabled");
                }
                else
                {
                    sal_fprintf(fp, "\n");
                }
            }     
        }
        sal_fprintf(fp, "\n");
    }
    
    if (p_db_neigh->mgmt_addr_sub_type)
    {
        sal_fprintf(fp, "%-25s:", "Management Address");
        if (LLDP_MANAGEMENT_ADDRESS_SUBTYPE_IPV4 == p_db_neigh->mgmt_addr_sub_type)
        {
             sal_inet_ntop (AF_INET, (void *) &p_db_neigh->mant_addr, ip_addr, INET_NTOP_BUFSIZ);
             sal_fprintf(fp, " %-s\n", ip_addr);
        }
        else if (LLDP_MANAGEMENT_ADDRESS_SUBTYPE_ALL802 == p_db_neigh->mgmt_addr_sub_type)
        {
              sal_fprintf (fp, " %.04hx.%.04hx.%.04hx\n",
                sal_ntoh16(((unsigned short *)
                p_db_neigh->mant_addr)[0]),
                sal_ntoh16(((unsigned short *)
                p_db_neigh->mant_addr)[1]),
                sal_ntoh16(((unsigned short *)
                p_db_neigh->mant_addr)[2]));   
        }
        else if (LLDP_MANAGEMENT_ADDRESS_SUBTYPE_IPV6 == p_db_neigh->mgmt_addr_sub_type)
        {
             sal_inet_ntop (AF_INET6, (void *) &p_db_neigh->mant_addr, ip_addr, INET_NTOP_BUFSIZ);
             sal_fprintf(fp, " %-s\n", ip_addr);
        }

        if (p_db_neigh->if_numbering)
        {
            sal_fprintf(fp, "%-25s: %-u\n", "Interface Numbering", p_db_neigh->if_numbering);
            sal_fprintf(fp, "%-25s: %-u\n", "Interface Number", p_db_neigh->if_number);
        }
        if (p_db_neigh->oid)
        {
            sal_fprintf(fp, "%-25s: ", "OID");
            for (i = 0; i < p_db_neigh->oid_len; i++)
            {
                sal_fprintf(fp, "%02x", p_db_neigh->oid[i]); 
            }
            sal_fprintf(fp, "\n");
        }
        sal_fprintf(fp, "\n");
    }
    
    /* 802.1 */
    if (p_db_neigh->pvid)
    {
        sal_fprintf(fp, "%-25s: %-u\n", "Port Vlan ID", p_db_neigh->pvid);
        sal_fprintf(fp, "\n");
    }
    
    if (p_db_neigh->vlan_id)
    {
        sal_fprintf(fp, "%-25s: %-u\n", "VLAN ID", p_db_neigh->vlan_id);
        sal_fprintf(fp, "%-25s: %-s\n", "VLAN Name", p_db_neigh->vlan_name);
        sal_fprintf(fp, "\n");
    }

    /*not support*/
    if (p_db_neigh->ppvid)
    {
        if (p_db_neigh->ppvid_flag)
        {
            sal_fprintf(fp, "%-25s: %-s\n", "Protocol Port VLAN", "Supported");
            sal_fprintf(fp, "%-25s: %-u\n", "Protocol Port VLAN", p_db_neigh->ppvid);
            sal_fprintf(fp, "\n");
        }
    }
    
    if (p_db_neigh->protocol_id)
    {
        sal_fprintf(fp, "%-25s:\n", "Protocols Identity");
        for (proto_id = LLDP_PROTO_STP; proto_id < LLDP_PROTO_MAX; proto_id++)
        {
            proto = lldp_get_protocol_identity (proto_id);
            if (GLB_IS_BIT_SET (p_db_neigh->protocol_id, proto_id)
                && (proto != NULL))
            {
                sal_fprintf(fp, "   %s\n", proto->protocol_str);
            }           
        }
        sal_fprintf(fp, "\n");
    }
    
    /*some problem*/
    if (GLB_IS_BIT_SET(p_db_neigh->link_aggr_status, LLDP_8021_AGGREGATION_CAPABLE))
    {
         sal_fprintf(fp, "%-25s: %-s\n", "Link Aggregation Capability", "Support");
         if (GLB_IS_BIT_SET(p_db_neigh->link_aggr_status, LLDP_8021_AGGREGATION_ENABLE))
         {
              sal_fprintf(fp, "%-25s: %-s\n", "Link Aggregation Status", "Enable");
              sal_fprintf(fp, "%-25s: %-u\n", "Link Aggregation Port ID", p_db_neigh->link_aggr_id + 2048);
         }
         else
         {
              sal_fprintf(fp, "%-25s: %-s\n", "Link Aggregation Status", "Disabled");
              sal_fprintf(fp, "%-25s: %-u\n", "Link Aggregation Port ID", 0);
         }
         sal_fprintf(fp, "\n");
    }  

    /* 802.3 */
    sal_fprintf(fp, "%-25s: \n", "MAC/PHY Configuration/Status");
    sal_fprintf(fp, "%-25s:", "AutoNego Support");
    if (GLB_IS_BIT_SET(p_db_neigh->autonego_support, LLDP_MAU_AUTONEG_SUPPORTED))
    {
         sal_fprintf(fp, " %s", "Support");
         if (GLB_IS_BIT_SET(p_db_neigh->autonego_support, LLDP_MAU_AUTONEG_ENABLED))
         {
             sal_fprintf(fp, ", %s\n", "Enabled");
         }
         else
         {
             sal_fprintf(fp, "\n"); 
         }
    }
    else
    {
         sal_fprintf(fp, ", %s\n", "Not Support");
    }
    if (p_db_neigh->autonego_cap)
    {
         sal_fprintf(fp, "%-25s: \n", " AutoNego Capability");
         for (i = 0; i < LLDP_AUTONEGO_CAP_MAX;  i++)
         {  
            if (p_db_neigh->autonego_cap & (0x8000 >> i))
            {
                sal_fprintf(fp, "  %s  %s\n", lldp_auto_neg_cap_to_name(i), lldp_auto_neg_cap_to_descr(i));
            }
         }
    }
    else
    {
        sal_fprintf(fp, "%-25s: %-s\n", "AutoNego Capability", "Unknown");
    }
    if (p_db_neigh->oper_mau_type)
    {
         sal_fprintf(fp, "%-25s: %-s %-s\n", "Operational MAU Type",  
            lldp_mau_type_to_name(p_db_neigh->oper_mau_type),
            lldp_mau_type_to_descr(p_db_neigh->oper_mau_type));
    }
    else
    {
        sal_fprintf(fp, "%-25s: %-s - %-s\n", "Operational MAU Type", "Unknown", "Unknown" );
    }
    sal_fprintf(fp, "\n"); 
    
    sal_fprintf(fp, "%-25s:\n", "Power via MDI");
    sal_fprintf(fp, "%-25s: %-s\n", "Port class", (p_db_neigh->power_flag & 0x01) ? "PSE" : "PD");
    sal_fprintf(fp, "%-25s: %-s\n", "PSE MDI power support", (p_db_neigh->power_flag & 0x02) ? "YES" : "NO");
    sal_fprintf(fp, "%-25s: %-s\n", "PSE MDI power state", (p_db_neigh->power_flag & 0x04) ? "Enabled" : "Disabled");
    sal_fprintf(fp, "%-25s: %-s\n", "PSE pairs control ability", (p_db_neigh->power_flag & 0x08) ? "YES" : "NO");
    if (LLDP_PSE_POWER_PAIR_SIGNAL == p_db_neigh->power_pair)
    {
        sal_fprintf(fp, "%-25s: %-s\n", "PSE power pair", "Signal");
    }
    else if (LLDP_PSE_POWER_PAIR_SPARE == p_db_neigh->power_pair)
    {
        sal_fprintf(fp, "%-25s: %-s\n", "PSE power pair", "Spare");
    }
    else
    {
        sal_fprintf(fp, "%-25s: %-s\n", "PSE power pair", "Unknown");
    }

    if (p_db_neigh->power_class > LLDP_POWER_CLASS_UN && p_db_neigh->power_class < LLDP_POWER_CLASS_MAX)
    {
        sal_fprintf(fp, "%-25s: %-d\n", "Power class", p_db_neigh->power_class - 1);
    }
    else
    {
        sal_fprintf(fp, "%-25s: %-s\n", "Power class", "Unknown");
    }
    sal_fprintf(fp, "\n");
    
    sal_fprintf(fp, "%-25s: %-u\n", "Maximum Frame Size", p_db_neigh->max_frame_size);
    sal_fprintf(fp, "\n");
    
    /* med */
    sal_fprintf(fp, "%-25s\n", "LLDP MED Information");
    if(p_db_neigh->med_dev_type == 4)
    {
        sal_fprintf(fp, "%-25s: %-s\n", "Device type", "Network Connectivity");
    }
    else if(p_db_neigh->med_dev_type <= 3 || p_db_neigh->med_dev_type >= 1)
    {
        sal_fprintf(fp, "%-25s: %-d\n", "Endpoint class", p_db_neigh->med_dev_type);
    }
    else
    {
        sal_fprintf(fp, "%-25s: %-s\n", "Device type", "Unknown");
    }
    
    sal_fprintf(fp, "%-25s:\n", "Med capabilities");
    for (i = 0; i < MED_TYPE_CAPABILITIES_SUB_RESERVED; i++)
    {
        if (GLB_IS_BIT_SET(p_db_neigh->med_capbility, i))
        {
            sal_fprintf(fp, "  %s\n", lldp_med_cap_tlv_type[i]);
        }
    }
    sal_fprintf(fp, "\n");
    
    /*1. Network Policy*/
    sal_fprintf(fp, "%-25s:\n", "Network Policy");
    sal_fprintf(fp, "%-25s: %-s\n", "Application Type", lldp_med_np_app_str[LLDP_MED_NP_GET_APP_TYPE(p_db_neigh->med_policy_flag)]);
    
    if (LLDP_MED_NP_GET_U_VALUE(p_db_neigh->med_policy_flag))
    {
        sal_fprintf(fp, "%-25s: %-s\n", "Status", "Unknown");
    }
    else
    {
        sal_fprintf(fp, "%-25s: %-s\n", "Status", "Configured");
        if (LLDP_MED_NP_GET_T_VALUE(p_db_neigh->med_policy_flag))
        {
            sal_fprintf(fp, "%-25s: %-s\n", "Tagged", "YES");
            if (LLDP_MED_NP_GET_VLAN_ID (p_db_neigh->med_policy_flag))
            {
                sal_fprintf(fp, "%-25s: %-u\n", "Vlan ID", LLDP_MED_NP_GET_VLAN_ID(p_db_neigh->med_policy_flag));
            }
            sal_fprintf(fp, "%-25s: %-u\n", "L2 Priority", LLDP_MED_NP_GET_L2_PRIORITY(p_db_neigh->med_policy_flag));
        }
        sal_fprintf(fp, "%-25s: %-u\n", "DSCP", LLDP_MED_NP_GET_DSCP(p_db_neigh->med_policy_flag));          
    }
    sal_fprintf(fp, "\n");

    /*2. Extended Power-via-MDI*/
    sal_fprintf(fp, "%-25s:\n", "Extended Power-via-MDI");
    if (LLDP_MED_POW_GET_TYPE(p_db_neigh->med_power_flag) == MED_TYPE_POWER_FLAG_TYPE_PD)
    {
        sal_fprintf(fp, "%-25s: %-s\n", "type", "PD");
        if (LLDP_MED_POW_GET_SRC(p_db_neigh->med_power_flag) == MED_TYPE_POWER_FLAG_TYPE_PD_SRC_PSE)
        {
            sal_fprintf(fp, "%-25s: %-s\n", "Power Source", "PSE");
        }
        else if (LLDP_MED_POW_GET_SRC(p_db_neigh->med_power_flag) == MED_TYPE_POWER_FLAG_TYPE_PD_SRC_LOCAL)
        {
             sal_fprintf(fp, "%-25s: %-s\n", "Power Source", "Local");
        }
        else if (LLDP_MED_POW_GET_SRC(p_db_neigh->med_power_flag) == MED_TYPE_POWER_FLAG_TYPE_PD_SRC_ALL)
        {
             sal_fprintf(fp, "%-25s: %-s\n", "Power Source", "PSE and Local");
        }
        else
        {
             sal_fprintf(fp, "%-25s: %-s\n", "Power Source", "Unknown");
        } 
        sal_fprintf(fp, "%-25s: %-d\n", "Power Priority", LLDP_MED_POW_GET_PRIORITY(p_db_neigh->med_power_flag));
        sal_fprintf(fp, "%-25s: %0.1f%s\n", "Power Value", (float)p_db_neigh->med_power_value/(float)10, "W"); 
    }
    else if (LLDP_MED_POW_GET_TYPE(p_db_neigh->med_power_flag) == MED_TYPE_POWER_FLAG_TYPE_PSE)
    {
        sal_fprintf(fp, "%-25s: %-s\n", "type", "PSE");
        if (LLDP_MED_POW_GET_SRC(p_db_neigh->med_power_flag) == MED_TYPE_POWER_FLAG_TYPE_PSE_SRC_PRI)
        {
            sal_fprintf(fp, "%-25s: %-s\n", "Power Source", "Primary");
        }
        else if (LLDP_MED_POW_GET_SRC(p_db_neigh->med_power_flag) == MED_TYPE_POWER_FLAG_TYPE_PSE_SRC_BAK)
        {
             sal_fprintf(fp, "%-25s: %-s\n", "Power Source", "Backup");
        }
        else
        {
             sal_fprintf(fp, "%-25s: %-s\n", "Power Source", "Unknown");
        }
        sal_fprintf(fp, "%-25s: %-d\n", "Power Priority", LLDP_MED_POW_GET_PRIORITY(p_db_neigh->med_power_flag));
        sal_fprintf(fp, "%-25s: %0.1f%s\n", "Power Value", (float)p_db_neigh->med_power_value/(float)10, "W"); 
    }
    else
    {
        sal_fprintf(fp, "%-25s: %-s\n", "type", "Unknown");
    }
    sal_fprintf(fp, "\n");

    /*3. Inventory Information*/
    sal_fprintf(fp, "%-25s:\n", "Inventory Information"); 
    if (p_db_neigh->med_hard_ver)
    {
        sal_fprintf(fp, "%-25s: %-s\n", "  Hardware Rivision", p_db_neigh->med_hard_ver);
    }
    if (p_db_neigh->med_firm_ver)
    {
        sal_fprintf(fp, "%-25s: %-s\n", "  Firmware Rivision", p_db_neigh->med_firm_ver);
    }
    if (p_db_neigh->med_soft_ver)
    {
        sal_fprintf(fp, "%-25s: %-s\n", "  Software Rivision", p_db_neigh->med_soft_ver);
    }
    if (p_db_neigh->med_serial_ver)
    {
        sal_fprintf(fp, "%-25s: %-s\n", "  Serial Number", p_db_neigh->med_serial_ver);
    }
    if (p_db_neigh->med_manufac_ver)
    {
        sal_fprintf(fp, "%-25s: %-s\n", "  Manufacturer Name", p_db_neigh->med_manufac_ver);
    }
    if (p_db_neigh->med_moname_ver)
    {
        sal_fprintf(fp, "%-25s: %-s\n", "  Model Name", p_db_neigh->med_moname_ver);
    }
    if (p_db_neigh->med_asset_ver)
    {
         sal_fprintf(fp, "%-25s: %-s\n", "  Asset ID", "");
    }
    sal_fprintf(fp, "\n");

    /*4.location id*/
    sal_fprintf(fp, "%-25s:\n", "Location Identification");
    if (MED_LCI_FORMAT_CIVIC == p_db_neigh->med_lci_format)
    {
        sal_fprintf(fp, "%-25s:\n", "  Civic Address");
        sal_fprintf(fp, "  %s: %-d\n", "Address reference", p_db_neigh->med_lci_civic.what);
        sal_fprintf(fp, "  %s: %-s\n", "Country code", p_db_neigh->med_lci_civic.countrycode);
        for (i = 0; i < p_db_neigh->med_lci_civic.ca_num; i++)
        {
             sal_fprintf(fp, "  %s: %-d, ", "CA-Type", p_db_neigh->med_lci_civic.ca_type[i]);
             sal_fprintf(fp, "%s: %-s\n", "CA-Value", p_db_neigh->med_lci_civic.ca_value[i]);
        }
    }
    else if (MED_LCI_FORMAT_ECS_ELIN == p_db_neigh->med_lci_format)
    {
        sal_fprintf(fp, "%-25s: %-s\n", "  ECS ELIN", p_db_neigh->med_lci_elin);
    }
    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

static int32
_lldp_cmd_process_show_neighbor_one(uint32 brief, tbl_interface_t *p_db_if, tbl_lldp_if_t *p_db_lldp_if, FILE *fp)
{
    ds_lldp_neighbour_t *p_db_neigh = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    if (CTCLIB_SLIST_ISEMPTY(p_db_lldp_if->neigh_list.obj_list))
    {
        return PM_E_NONE;
    }    
    CTCLIB_SLIST_LOOP(p_db_lldp_if->neigh_list.obj_list, p_db_neigh, listnode)
    {
        if (brief)
        {
            _lldp_cmd_process_show_neighbor_brief(p_db_if, p_db_neigh, fp);
        }
        else
        {
            _lldp_cmd_process_show_neighbor_interface(p_db_if, p_db_neigh, fp);
        }
    }
    return PM_E_NONE;
}

static int32
_lldp_cmd_process_show_neighbor(uint32 brief, char *ifname, cfg_cmd_para_t *para)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    tbl_lldp_if_key_t key;
    FILE *fp = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_lldp_if_t *p_db_lldp_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (NULL == ifname)
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
        {
            key.ifindex = p_db_if->ifindex;
            p_db_lldp_if = tbl_lldp_if_get_lldp_if(&key);
            if (p_db_lldp_if)
            {
                _lldp_cmd_process_show_neighbor_one(brief, p_db_if, p_db_lldp_if, fp);
            }
        }
    }
    else
    {
        p_db_if = tbl_interface_get_interface_by_name(ifname);
        if (p_db_if)
        {
            key.ifindex = p_db_if->ifindex;
            p_db_lldp_if = tbl_lldp_if_get_lldp_if(&key);
            if (p_db_lldp_if)
            {
                _lldp_cmd_process_show_neighbor_one(brief, p_db_if, p_db_lldp_if, fp);
            }
        }
    }
    sal_fclose(fp);

    return PM_E_NONE;
}
  
int32
lldp_cmd_process_show_neighbor(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char *ifname = NULL;
    uint32 brief = FALSE;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 2, para->p_rs);
        if (0 == sal_strcmp(argv[0], "brief"))
        {
            brief = TRUE;
        }
        else
        {
            brief = FALSE;
        }
        if (2 == argc)
        {
            ifname = argv[1];
        }
        _lldp_cmd_process_show_neighbor(brief, ifname, para);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_lldp_cmd_show_statistics_one(uint32 *banner, tbl_interface_t *p_db_if, tbl_lldp_if_t *p_db_lldp_if, FILE *fp)
{
    char ifname_ext[IFNAME_SIZE];
    
    if (*banner)
    {
        sal_fprintf(fp, "LLDP statistics information:\n");
        *banner = FALSE;
    }

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    sal_fprintf(fp, "-------------------------+-------------------\n");
    sal_fprintf(fp, "%-25s: %-s\n", "LLDP Port statistics for ", ifname_ext);
    sal_fprintf(fp, "%-25s: %-u\n", "Frames Transmitted", p_db_lldp_if->tx_count);
    sal_fprintf(fp, "%-25s: %-u\n", "Frames Aged", p_db_lldp_if->aged_count);
    sal_fprintf(fp, "%-25s: %-u\n", "Frames Discarded", p_db_lldp_if->rx_discard_count);
    sal_fprintf(fp, "%-25s: %-u\n", "Frames with Error", p_db_lldp_if->rx_error_count);
    sal_fprintf(fp, "%-25s: %-u\n", "Frames Recieved", p_db_lldp_if->rx_count);
    sal_fprintf(fp, "%-25s: %-u\n", "TLVs Discarded", p_db_lldp_if->tlv_discard_count);
    sal_fprintf(fp, "%-25s: %-u\n", "TLVs Unrecognized", p_db_lldp_if->tlv_unrecognized_count);
    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

static int32
_lldp_cmd_clear_statistics_one(tbl_lldp_if_t *p_db_lldp_if)
{
    p_db_lldp_if->tx_count = 0;
    p_db_lldp_if->aged_count = 0;
    p_db_lldp_if->rx_discard_count = 0;
    p_db_lldp_if->rx_error_count = 0;
    p_db_lldp_if->rx_count = 0;
    p_db_lldp_if->tlv_discard_count = 0;
    p_db_lldp_if->tlv_unrecognized_count = 0;
    
    return PM_E_NONE;
}

static int32
_lldp_cmd_show_statistics(char *ifname, cfg_cmd_para_t *para)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    tbl_lldp_if_key_t key;
    FILE *fp = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_lldp_if_t *p_db_lldp_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint32 banner = TRUE;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (NULL == ifname)
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
        {
            key.ifindex = p_db_if->ifindex;
            p_db_lldp_if = tbl_lldp_if_get_lldp_if(&key);
            if (p_db_lldp_if)
            {
                _lldp_cmd_show_statistics_one(&banner, p_db_if, p_db_lldp_if, fp);
            }
        }
    }
    else
    {
        p_db_if = tbl_interface_get_interface_by_name(ifname);
        if (p_db_if)
        {
            key.ifindex = p_db_if->ifindex;
            p_db_lldp_if = tbl_lldp_if_get_lldp_if(&key);
            if (p_db_lldp_if)
            {
                _lldp_cmd_show_statistics_one(&banner, p_db_if, p_db_lldp_if, fp);
            }
        }
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_lldp_cmd_clear_statistics(char *ifname, cfg_cmd_para_t *para)
{
    tbl_interface_master_t *p_master = tbl_interface_get_master();
    tbl_lldp_if_key_t key;
    FILE *fp = NULL;
    tbl_interface_t *p_db_if = NULL;
    tbl_lldp_if_t *p_db_lldp_if = NULL;
    ctclib_slistnode_t *listnode = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    if (NULL == ifname)
    {
        CTCLIB_SLIST_LOOP(p_master->if_list, p_db_if, listnode)
        {
            key.ifindex = p_db_if->ifindex;
            p_db_lldp_if = tbl_lldp_if_get_lldp_if(&key);
            if (p_db_lldp_if)
            {
                _lldp_cmd_clear_statistics_one(p_db_lldp_if);
            }
        }
    }
    else
    {
        p_db_if = tbl_interface_get_interface_by_name(ifname);
        if (p_db_if)
        {
            key.ifindex = p_db_if->ifindex;
            p_db_lldp_if = tbl_lldp_if_get_lldp_if(&key);
            if (p_db_lldp_if)
            {
                _lldp_cmd_clear_statistics_one(p_db_lldp_if);
            }
        }
    }
    sal_fclose(fp);

    return PM_E_NONE;
}


int32
lldp_cmd_process_show_statistics(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char *ifname = NULL;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_GET:
        if (1 <= argc)
        {
            ifname = argv[0];
        }
        _lldp_cmd_show_statistics(ifname, para);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return PM_E_NONE;
}

int32
lldp_cmd_process_clear_statistics(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char *ifname = NULL;

    CFG_DBG_FUNC();
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        if (1 <= argc)
        {
            ifname = argv[0];
        }
        _lldp_cmd_clear_statistics(ifname, para);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return PM_E_NONE;
}
