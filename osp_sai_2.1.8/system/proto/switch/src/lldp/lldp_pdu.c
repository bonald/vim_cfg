#include "proto.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_lldp_global_define.h"
#include "gen/tbl_lldp_global.h"
#include "gen/tbl_lldp_if_define.h"
#include "gen/tbl_lldp_if.h"
#include "switch.h"
#include "lldp_define.h"
#include "lldp.h"
#include "mstp_define.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "lldp_api.h"
#include "gen/tbl_version_define.h"
#include "gen/tbl_version.h"
#include "gen/tbl_card_define.h"
#include "gen/tbl_card.h"
#include "gen/tbl_manage_if_define.h"
#include "gen/tbl_manage_if.h"
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_global.h"

static mac_addr_t lldp_mac_address      = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E};

int32
lldp_pdu_encode(uint8 *p_buf, uint32 len, tbl_interface_t *p_db_if, tbl_lldp_if_t *p_db_lldp_if)
{
    char ifname_ext[IFNAME_SIZE];
    tbl_lldp_global_t *p_db_glb = tbl_lldp_global_get_lldp_global();
    tbl_version_t *p_db_version = tbl_version_get_version();
    tbl_card_t *p_db_card = tbl_card_get_default_card();
    tbl_manage_if_t *p_db_mgtif = tbl_manage_if_get_manage_if();
    tbl_mstp_global_t *p_glb = tbl_mstp_global_get_mstp_global();
    
    uint32 length = 0;
    int32  len_left = len;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    char *port_desc = NULL;
    char *system_desc = NULL;
    char *system_name = NULL;
    char system_name_buf[LLDP_NAME_SIZE+1];
    uint32 sys_cap = 0;
    uint32 mant_length = 0;
    uint16 pvid = 0;
    uint32 i = 0;

    uint16 vid = 0 ;
    char *vlan_name = NULL;
    char vlan_name_buf[LLDP_NAME_SIZE];
    uint32 vlan_length = 0;
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;

    lldp_protocol_identity_t* proto = NULL;
    uint32 proto_id = 0 ;

    uint32 linkagg_status = 0;
    uint32 linkagg_id = 0;

    uint32 autonego_support = 0;
    uint32 autonego_cap = 0;
    uint32 oper_mau_type = 0;

    uint32 max_frame_size = 0;
        
    size = &len_left;
    ppnt = &p_buf;

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);

    /* L2 header */
    MSG_ENCODE_PUT(lldp_mac_address, MAC_ADDR_LEN);
    MSG_ENCODE_PUT(p_db_if->mac_addr, MAC_ADDR_LEN);
    MSG_ENCODE_PUTW(ETH_P_LLDP);                        /* Table 8-2¡ªLLDP Ethertype */
    
    LLDP_DEBUG(LLDP_PACKET_TX, "TX: Encoding LLDP PDU for interface %s\n", ifname_ext);

    /* 1) Chassis ID TLV */
    LLDP_ENCODE_TL(LLDP_TLV_TYPE_CHASSIS_ID, (MAC_ADDR_LEN + 1));
    MSG_ENCODE_PUTC(LLDP_CHASSIS_ID_SUBTYPE_MAC_ADDRESS);
    if (p_db_glb->system_mac_en)
    {
        MSG_ENCODE_PUT(p_db_glb->system_mac, MAC_ADDR_LEN);
    }
    else
    {
        MSG_ENCODE_PUT(p_db_if->hw_mac_addr, MAC_ADDR_LEN);
    }
    
    LLDP_DEBUG(LLDP_PACKET_TX, "TX: Encoding CHASSIS_ID_TLV");
    
    /* 2) Port ID TLV */
    length = sal_strlen(ifname_ext);
    LLDP_ENCODE_TL(LLDP_TLV_TYPE_PORT_ID, length + 1);
    MSG_ENCODE_PUTC(LLDP_PORT_ID_SUBTYPE_IF_NAME);
    MSG_ENCODE_PUT(ifname_ext, length);
    
    LLDP_DEBUG(LLDP_PACKET_TX, "TX: Encoding PORT_ID_TLV");
    
    /* 3) Time To Live TLV */
    LLDP_ENCODE_TL(LLDP_TLV_TYPE_TTL, 2);
    MSG_ENCODE_PUTW(p_db_lldp_if->tx_ttl);
    
    LLDP_DEBUG(LLDP_PACKET_TX, "TX: Encoding TTL_TLV of TTL value %d\n", p_db_lldp_if->tx_ttl);

    /* LLDP_TLV_TYPE_PORT_DESCRIPTION */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_basic, BASIC_TLV_EN_PORT_DESCRIPTION))
    {
        port_desc = sal_strlen(p_db_if->desc) ? p_db_if->desc : ifname_ext;
        length = sal_strlen(port_desc);
        LLDP_ENCODE_TL(LLDP_TLV_TYPE_PORT_DESCRIPTION, length);
        MSG_ENCODE_PUT(port_desc, length);
        
        LLDP_DEBUG(LLDP_PACKET_TX, "TX: Encoding PORT_DESCRIPTION_TLV with the description %s\n", port_desc);
    }

    /* LLDP_TLV_TYPE_SYSTEM_NAME */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_basic, BASIC_TLV_EN_SYSTEM_NAME))
    {
        length = sal_strlen(p_db_glb->system_name);
        if (0 == length)
        {
            sal_memset(system_name_buf, 0, sizeof(system_name_buf));
            gethostname(system_name_buf, sizeof(system_name_buf));
            system_name = system_name_buf;
            length = sal_strlen(system_name);
        }
        else
        {
            system_name = p_db_glb->system_name;
        }
        LLDP_ENCODE_TL(LLDP_TLV_TYPE_SYSTEM_NAME, length);
        MSG_ENCODE_PUT(system_name, length);
        
        LLDP_DEBUG(LLDP_PACKET_TX, "TX: Encoding SYSTEM_NAME_TLV with the name %s\n", system_name);
    }
    
    /* LLDP_TLV_TYPE_SYSTEM_DESCRIPTION */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_basic, BASIC_TLV_EN_SYSTEM_DESCRIPTION))
    {
        system_desc = sal_strlen(p_db_glb->system_desc) ? p_db_glb->system_desc : p_db_glb->system_desc_default;
        length = sal_strlen(system_desc);
        LLDP_ENCODE_TL(LLDP_TLV_TYPE_SYSTEM_DESCRIPTION, length);
        MSG_ENCODE_PUT(system_desc, length);
        
        LLDP_DEBUG(LLDP_PACKET_TX, "TX: Encoding SYSTEM_DESCRIPTION_TLV with the description %s\n", system_desc);
    }

    /* LLDP_TLV_TYPE_SYSTEM_CAPABILITIES */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_basic, BASIC_TLV_EN_SYSTEM_CAPABILITIES))
    {
         LLDP_ENCODE_TL(LLDP_TLV_TYPE_SYSTEM_CAPABILITIES, 4);
         MSG_ENCODE_PUTW(LLDP_SYS_CAP_DEFAULT);
         if (GLB_IS_BIT_SET(p_db_glb->system_capability, LLDP_SYS_CAP_ROUTING))
         {
             GLB_SET_BIT(sys_cap, LLDP_SYS_CAP_ROUTING);
             LLDP_DEBUG(LLDP_PACKET_TX, "TX: Encoding SYSTEM_CAPABILITIES_TLV with the capability: routing\n");
         }
         if (GLB_IS_BIT_SET(p_db_glb->system_capability, LLDP_SYS_CAP_L2_SWITCHING))
         {
             GLB_SET_BIT(sys_cap, LLDP_SYS_CAP_L2_SWITCHING);
             LLDP_DEBUG(LLDP_PACKET_TX, "TX: Encoding SYSTEM_CAPABILITIES_TLV with the capability: bridge\n");
         }
         MSG_ENCODE_PUTW(sys_cap);
    }

    /* LLDP_TLV_TYPE_MANAGEMENT_ADDRESS */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_basic, BASIC_TLV_EN_MANAGEMENT_ADDRESS))
    {
        if (GLB_IS_BIT_SET(p_db_glb->config_flag, LLDP_CONF_MGMT_IP_ADDRESS))
        {
            LLDP_ENCODE_TL(LLDP_TLV_TYPE_MANAGEMENT_ADDRESS, LLDP_MANAGEMENT_ADDRESS_IPV4_TLV_LENGTH);
            mant_length = sizeof(struct sal_in4_addr) + 1;
            MSG_ENCODE_PUTC(mant_length);
            MSG_ENCODE_PUTC(LLDP_MANAGEMENT_ADDRESS_SUBTYPE_IPV4);
            MSG_ENCODE_PUTL(sal_htonl(p_db_glb->management_ip.u.prefix4.s_addr));
            
            LLDP_DEBUG(LLDP_PACKET_TX, "TX: Use configured ip address for MANAGEMENT_ADDRESS_TLV\n"); 
        }
        else if (p_db_mgtif->addr.prefix.s_addr)
        {
            LLDP_ENCODE_TL(LLDP_TLV_TYPE_MANAGEMENT_ADDRESS, LLDP_MANAGEMENT_ADDRESS_IPV4_TLV_LENGTH);
            mant_length = sizeof(struct sal_in4_addr) + 1;
            MSG_ENCODE_PUTC(mant_length);
            MSG_ENCODE_PUTC(LLDP_MANAGEMENT_ADDRESS_SUBTYPE_IPV4); 
            MSG_ENCODE_PUTL(sal_htonl(p_db_mgtif->addr.prefix.s_addr));

            LLDP_DEBUG(LLDP_PACKET_TX, "TX: Use mgmt-if ip address for MANAGEMENT_ADDRESS_TLV\n");
        }
        else 
        {
            LLDP_ENCODE_TL(LLDP_TLV_TYPE_MANAGEMENT_ADDRESS, LLDP_MANAGEMENT_ADDRESS_TLV_LENGTH);
            mant_length = MAC_ADDR_LEN + 1;
            MSG_ENCODE_PUTC(mant_length);
            MSG_ENCODE_PUTC(LLDP_MANAGEMENT_ADDRESS_SUBTYPE_ALL802); 
            MSG_ENCODE_PUT(p_db_if->hw_mac_addr, MAC_ADDR_LEN);

            LLDP_DEBUG(LLDP_PACKET_TX, "TX: Use MAC address for MANAGEMENT_ADDRESS_TLV\n");
        }
        MSG_ENCODE_PUTC(LLDP_IF_NUMBERING_IFINDEX);
        MSG_ENCODE_PUTL(p_db_if->ifindex);
        MSG_ENCODE_PUTC(MANAGEMENT_ADDRESS_OID_STRING_LENGTH);
    }

    if (p_db_lldp_if->tlv_enable_8021)
    {
        LLDP_DEBUG(LLDP_PACKET_TX, "TX: Encoding IEEE_8021_ORG_SPECIFIC_TLV\n");
    }
    /* IEEE_8021_TLV_EN_PORT_VLAN_ID */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_PORT_VLAN_ID))
    {
        LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, 6);
        MSG_ENCODE_PUTC(IEEE_8021_OUI_1);
        MSG_ENCODE_PUTC(IEEE_8021_OUI_2);
        MSG_ENCODE_PUTC(IEEE_8021_OUI_3);
        MSG_ENCODE_PUTC(PORT_VLAN_ID_TLV_SUBTYPE);
        if (GLB_IF_MODE_L3 != p_db_if->mode)
        {
            pvid = p_db_if->brgif->pvid;
        }
        else
        {
            pvid = 0;
        }
        MSG_ENCODE_PUTW(pvid);
    }
  
    /* IEEE_8021_TLV_EN_PROTO_VLAN_ID */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_PROTO_VLAN_ID))
    {
        /*not support*/
        LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, 7);
        MSG_ENCODE_PUTC(IEEE_8021_OUI_1);
        MSG_ENCODE_PUTC(IEEE_8021_OUI_2);
        MSG_ENCODE_PUTC(IEEE_8021_OUI_3);
        MSG_ENCODE_PUTC(PORT_AND_PROTOCOL_VLAN_ID_TLV_SUBTYPE);
        MSG_ENCODE_PUTC(p_db_lldp_if->ppvid_flag);
        MSG_ENCODE_PUTW(p_db_lldp_if->ppvid);
    } 

    /* IEEE_8021_TLV_EN_PORT_VLAN_NAME */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_PORT_VLAN_NAME))
    {   
        if (p_db_lldp_if->vlan_name_id)
        {
            sal_memset(vlan_name_buf, 0, sizeof(vlan_name_buf));
            sal_snprintf (vlan_name_buf, LLDP_NAME_MAX_SIZE, "VLAN%u", p_db_lldp_if->vlan_name_id);
            vlan_name = vlan_name_buf;
            vid = p_db_lldp_if->vlan_name_id;
            vlan_length = sal_strlen(vlan_name);
        }
        else
        {
            if (p_db_if->brgif)
            {
                vlan_key.vid = p_db_if->brgif->pvid;
                p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
                if (!p_db_vlan)
                {
                    return PM_E_NOT_EXIST;
                }
                vlan_name = p_db_vlan->name;
                vid = p_db_if->brgif->pvid;
                vlan_length = sal_strlen(p_db_vlan->name);
            }
        }        
        length = vlan_length + 7;
        LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, length);
        MSG_ENCODE_PUTC(IEEE_8021_OUI_1);
        MSG_ENCODE_PUTC(IEEE_8021_OUI_2);
        MSG_ENCODE_PUTC(IEEE_8021_OUI_3);
        MSG_ENCODE_PUTC(VLAN_NAME_TLV_SUBTYPE);
        MSG_ENCODE_PUTW(vid);
        MSG_ENCODE_PUTC(vlan_length);
        MSG_ENCODE_PUT(vlan_name, vlan_length);
    }

    /* IEEE_8021_TLV_EN_PROTOCOL_ID */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_PROTOCOL_ID))
    {
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
        
        for (proto_id = LLDP_PROTO_STP; proto_id < LLDP_PROTO_MAX; proto_id++)
        {
            
            if (GLB_IS_BIT_SET (p_db_lldp_if->protocol_id, proto_id))
            {
                proto = lldp_get_protocol_identity (proto_id);

                if (proto == NULL)
                {
                    continue;
                }
                LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, 5 + proto->protocol_id_len);
                MSG_ENCODE_PUTC(IEEE_8021_OUI_1);
                MSG_ENCODE_PUTC(IEEE_8021_OUI_2);
                MSG_ENCODE_PUTC(IEEE_8021_OUI_3);
                MSG_ENCODE_PUTC(PROTOCOL_IDENTITY_TLV_SUBTYPE);
                MSG_ENCODE_PUTC(proto->protocol_id_len);
                MSG_ENCODE_PUT(proto->protocol_payload, proto->protocol_id_len);
            }
        }
    }

    /* IEEE_8021_TLV_EN_LINK_AGG_8021 */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8021, IEEE_8021_TLV_EN_LINK_AGG_8021))
    {
         LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, 9);
         MSG_ENCODE_PUTC(IEEE_8021_OUI_1);
         MSG_ENCODE_PUTC(IEEE_8021_OUI_2);
         MSG_ENCODE_PUTC(IEEE_8021_OUI_3);
         MSG_ENCODE_PUTC(LINK_AGGREGATION_8021_TLV_SUBTYPE);

         GLB_SET_BIT(linkagg_status, LLDP_8021_AGGREGATION_CAPABLE); 
         if (INTERFACE_IS_LAG_GROUP(p_db_if) || INTERFACE_IS_LAG_MEMBER(p_db_if))
         {
             GLB_SET_BIT(linkagg_status, LLDP_8021_AGGREGATION_ENABLE);
             if (NULL != p_db_if->lag)
             {
                  linkagg_id = p_db_if->lag->lag_id; 
             }
             else
             {
                  linkagg_id = 0;
             }
         }
         MSG_ENCODE_PUTC(linkagg_status);
         MSG_ENCODE_PUTL(linkagg_id);
    }

    if (p_db_lldp_if->tlv_enable_8023)
    {
        LLDP_DEBUG(LLDP_PACKET_TX, "TX: Encoding IEEE_8023_ORG_SPECIFIC_TLV\n");
    }
    /* IEEE_8023_TLV_EN_MAC_PHY_CONFIG_STATUS */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8023, IEEE_8023_TLV_EN_MAC_PHY_CONFIG_STATUS))
    {
         LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, 9);
         MSG_ENCODE_PUTC(IEEE_8023_OUI_1);
         MSG_ENCODE_PUTC(IEEE_8023_OUI_2);
         MSG_ENCODE_PUTC(IEEE_8023_OUI_3);
         MSG_ENCODE_PUTC(MAC_PHY_CONFIG_STATUS_TLV_SUBTYPE);
         autonego_support = lldp_autoneg_support_get(p_db_if);
         autonego_cap = lldp_autoneg_capability_get(p_db_if);
         oper_mau_type = lldp_oper_mau_type_get(p_db_if);
         MSG_ENCODE_PUTC(autonego_support);
         MSG_ENCODE_PUTW(autonego_cap);
         MSG_ENCODE_PUTW(oper_mau_type);
    }
    
    /* IEEE_8023_TLV_EN_POWER_VIA_MDI */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8023, IEEE_8023_TLV_EN_POWER_VIA_MDI))
    {
         LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, 7);
         MSG_ENCODE_PUTC(IEEE_8023_OUI_1);
         MSG_ENCODE_PUTC(IEEE_8023_OUI_2);
         MSG_ENCODE_PUTC(IEEE_8023_OUI_3);
         MSG_ENCODE_PUTC(POWER_VIA_MDI_TLV_SUBTYPE);
         MSG_ENCODE_PUTC(p_db_lldp_if->mdi_support);
         MSG_ENCODE_PUTC(p_db_lldp_if->pse_power_pair);
         MSG_ENCODE_PUTC(p_db_lldp_if->power_class);  
    }
    
    /* IEEE_8023_TLV_EN_LINK_AGG_8023 */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8023, IEEE_8023_TLV_EN_LINK_AGG_8023))
    {
         LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, 9);
         MSG_ENCODE_PUTC(IEEE_8023_OUI_1);
         MSG_ENCODE_PUTC(IEEE_8023_OUI_2);
         MSG_ENCODE_PUTC(IEEE_8023_OUI_3);
         MSG_ENCODE_PUTC(LINK_AGGREGATION_TLV_SUBTYPE);

         GLB_SET_BIT(linkagg_status, LLDP_8023_AGGREGATION_CAPABLE); 
         if (INTERFACE_IS_LAG_GROUP(p_db_if) || INTERFACE_IS_LAG_MEMBER(p_db_if))
         {
             GLB_SET_BIT(linkagg_status, LLDP_8023_AGGREGATION_ENABLE);
             if (NULL != p_db_if->lag)
             {
                  linkagg_id = p_db_if->lag->lag_id; 
             }
             else
             {
                  linkagg_id = 0;
             }
         }
         MSG_ENCODE_PUTC(linkagg_status);
         MSG_ENCODE_PUTL(linkagg_id); 
    }

    /* IEEE_8023_TLV_EN_MAX_FRAME_SIZE */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_8023, IEEE_8023_TLV_EN_MAX_FRAME_SIZE))
    {
         LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, 6);
         MSG_ENCODE_PUTC(IEEE_8023_OUI_1);
         MSG_ENCODE_PUTC(IEEE_8023_OUI_2);
         MSG_ENCODE_PUTC(IEEE_8023_OUI_3);
         MSG_ENCODE_PUTC(MAXIMUM_FRAME_SIZE_TLV_SUBTYPE);
         if (GLB_IF_MODE_L2 == p_db_if->mode)
         {
             if (p_db_if->jumboframe_en)
             {
                max_frame_size = GLB_DFT_MAX_FRAME_SIZE_1;
             }
             else
             {
                max_frame_size = GLB_DFT_MAX_FRAME_SIZE_0;
             }
         }
         else if (GLB_IF_MODE_L3 == p_db_if->mode)
         {
            max_frame_size = p_db_if->mtu;
         }
         
         MSG_ENCODE_PUTW(max_frame_size);  
    }

    if (p_db_lldp_if->tlv_enable_med)
    {
        LLDP_DEBUG(LLDP_PACKET_TX, "TX: Encoding LLDP_MED_TLV\n");
    }
    /* MED_TLV_EN_CAP */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_med, MED_TLV_EN_CAP))
    {
         LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, 7);
         MSG_ENCODE_PUTC(TIA_OUI_1);
         MSG_ENCODE_PUTC(TIA_OUI_2);
         MSG_ENCODE_PUTC(TIA_OUI_3);
         MSG_ENCODE_PUTC(MED_TYPE_CAPABILITIES_SUBTYPE);
         MSG_ENCODE_PUTW(p_db_lldp_if->tlv_enable_med); 
         MSG_ENCODE_PUTC(p_db_lldp_if->med_dev_type);  
    }

    /* MED_TLV_EN_NETWORK_POLICY */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_med, MED_TLV_EN_NETWORK_POLICY))
    {
         LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, 8);
         MSG_ENCODE_PUTC(TIA_OUI_1);
         MSG_ENCODE_PUTC(TIA_OUI_2);
         MSG_ENCODE_PUTC(TIA_OUI_3);
         MSG_ENCODE_PUTC(MED_TYPE_NETWORK_POLICY_SUBTYPE);
         MSG_ENCODE_PUTL(p_db_lldp_if->med_policy_flag);  
    }

    /* MED_TLV_EN_LOCATION_ID */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_med, MED_TLV_EN_LOCATION_ID))
    {
         if (MED_LCI_FORMAT_CIVIC == p_db_lldp_if->med_lci_format)
         {
             length = p_db_lldp_if->med_lci_civic.len;
             LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, 6 + length);
             MSG_ENCODE_PUTC(TIA_OUI_1);
             MSG_ENCODE_PUTC(TIA_OUI_2);
             MSG_ENCODE_PUTC(TIA_OUI_3);
             MSG_ENCODE_PUTC(MED_TYPE_LOCATION_ID_SUBTYPE);
             MSG_ENCODE_PUTC(MED_LCI_FORMAT_CIVIC);
             MSG_ENCODE_PUTC(p_db_lldp_if->med_lci_civic.len);
             MSG_ENCODE_PUTC(p_db_lldp_if->med_lci_civic.what);
             MSG_ENCODE_PUT(p_db_lldp_if->med_lci_civic.countrycode, LLDP_MED_LCI_CIVIC_COUNTRYCODE_SIZE);

             for (i = 0; i < p_db_lldp_if->med_lci_civic.ca_num; i++)
             {  
                  MSG_ENCODE_PUTC(p_db_lldp_if->med_lci_civic.ca_type[i]);
                  MSG_ENCODE_PUTC(p_db_lldp_if->med_lci_civic.ca_length[i]);
                  MSG_ENCODE_PUT(p_db_lldp_if->med_lci_civic.ca_value[i], p_db_lldp_if->med_lci_civic.ca_length[i]);
             }
         }
         else if (MED_LCI_FORMAT_ECS_ELIN == p_db_lldp_if->med_lci_format)
         {
             length = sal_strlen(p_db_lldp_if->med_lci_elin);
             LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, 5 + length);
             MSG_ENCODE_PUTC(TIA_OUI_1);
             MSG_ENCODE_PUTC(TIA_OUI_2);
             MSG_ENCODE_PUTC(TIA_OUI_3);
             MSG_ENCODE_PUTC(MED_TYPE_LOCATION_ID_SUBTYPE);
             MSG_ENCODE_PUTC(MED_LCI_FORMAT_ECS_ELIN);
             MSG_ENCODE_PUT(p_db_lldp_if->med_lci_elin, length);
         }
    }

    /* MED_TLV_EN_EXT_POWER_VIA_MDI */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_med, MED_TLV_EN_EXT_POWER_VIA_MDI))
    {
         LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, 7);
         MSG_ENCODE_PUTC(TIA_OUI_1);
         MSG_ENCODE_PUTC(TIA_OUI_2);
         MSG_ENCODE_PUTC(TIA_OUI_3);
         MSG_ENCODE_PUTC(MED_TYPE_EXT_POWER_VIA_MDI_SUBTYPE);
         MSG_ENCODE_PUTC(p_db_lldp_if->med_power_flag); 
         MSG_ENCODE_PUTW(p_db_lldp_if->med_power_value); 
    }

    /* MED_TLV_EN_INVENTORY */
    if (GLB_IS_BIT_SET(p_db_lldp_if->tlv_enable_med, MED_TLV_EN_INVENTORY))
    {
        /*hardware version*/
        length = sal_strlen(p_db_version->hw_type);
        LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, length + 4);
        MSG_ENCODE_PUTC(TIA_OUI_1);
        MSG_ENCODE_PUTC(TIA_OUI_2);
        MSG_ENCODE_PUTC(TIA_OUI_3);
        MSG_ENCODE_PUTC(MED_TYPE_INVENTORY_SUBTYPE_HARDWARE);
        MSG_ENCODE_PUT(p_db_version->hw_type, length);

        /*firm version*/
        length = sal_strlen(p_db_card->hardware_ver);
        LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, length + 4);
        MSG_ENCODE_PUTC(TIA_OUI_1);
        MSG_ENCODE_PUTC(TIA_OUI_2);
        MSG_ENCODE_PUTC(TIA_OUI_3);
        MSG_ENCODE_PUTC(MED_TYPE_INVENTORY_SUBTYPE_FIRMWARE);
        MSG_ENCODE_PUT(p_db_card->hardware_ver, length);

        /*software version*/
        length = sal_strlen(p_db_version->version);
        LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, length + 4);
        MSG_ENCODE_PUTC(TIA_OUI_1);
        MSG_ENCODE_PUTC(TIA_OUI_2);
        MSG_ENCODE_PUTC(TIA_OUI_3);
        MSG_ENCODE_PUTC(MED_TYPE_INVENTORY_SUBTYPE_SOFTWARE);
        MSG_ENCODE_PUT(p_db_version->version, length);
        
        /*serial version*/
        length = sal_strlen(p_db_card->serial_no);
        LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, length + 4);
        MSG_ENCODE_PUTC(TIA_OUI_1);
        MSG_ENCODE_PUTC(TIA_OUI_2);
        MSG_ENCODE_PUTC(TIA_OUI_3);
        MSG_ENCODE_PUTC(MED_TYPE_INVENTORY_SUBTYPE_SERIAL);
        MSG_ENCODE_PUT(p_db_card->serial_no, length);
        
        /*manufacturer_name version*/
        length = sal_strlen(p_db_version->company);
        LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, length + 4);
        MSG_ENCODE_PUTC(TIA_OUI_1);
        MSG_ENCODE_PUTC(TIA_OUI_2);
        MSG_ENCODE_PUTC(TIA_OUI_3);
        MSG_ENCODE_PUTC(MED_TYPE_INVENTORY_SUBTYPE_MANUFACTURER_NAME);
        MSG_ENCODE_PUT(p_db_version->company, length);

        /*model_name version*/
        length = sal_strlen(p_db_version->product);
        LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, length + 4);
        MSG_ENCODE_PUTC(TIA_OUI_1);
        MSG_ENCODE_PUTC(TIA_OUI_2);
        MSG_ENCODE_PUTC(TIA_OUI_3);
        MSG_ENCODE_PUTC(MED_TYPE_INVENTORY_SUBTYPE_MODEL_NAME);
        MSG_ENCODE_PUT(p_db_version->product, length);
        
        /*asset version*/
        length = 0;
        LLDP_ENCODE_TL(LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC, length + 4);
        MSG_ENCODE_PUTC(TIA_OUI_1);
        MSG_ENCODE_PUTC(TIA_OUI_2);
        MSG_ENCODE_PUTC(TIA_OUI_3);
        MSG_ENCODE_PUTC(MED_TYPE_INVENTORY_SUBTYPE_ASSET_ID);
    }
    
    /* End Of LLDPDU TLV */
    LLDP_ENCODE_TL(LLDP_TLV_TYPE_END_OF_LLDPDU, 0);

    return (len - len_left);
}

int32
lldp_pdu_tx(tbl_lldp_if_t *p_db_lldp_if)
{
    tbl_interface_t *p_db_if = NULL;
    tbl_lldp_global_t *p_db_glb = tbl_lldp_global_get_lldp_global();
    uint8 pkt[MSG_SIZE];
    int32 pkt_len = 0;
    char ifname_ext[IFNAME_SIZE];
    int32 rc = PM_E_NONE;

    /* check global enable */
    if (!p_db_glb->enable)
    {
        /*LLDP debug*/
        LLDP_DEBUG(LLDP_PACKET_TX, "LLDP packet transmit fail because lldp is not enable globally\n");
        return PM_E_NONE;
    }
    
    p_db_if = (tbl_interface_t*)p_db_lldp_if->intf;
    if (p_db_if == NULL)
    {
        return PM_E_NONE;
    }
    if (!tbl_interface_is_running(p_db_if))
    {
        /*LLDP debug*/
        LLDP_DEBUG(LLDP_PACKET_TX, "LLDP packet transmit fail because interface %s is not up\n", 
                   IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return PM_E_NONE;
    }
    /*check the interface is mirror dest interface*/
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG))
    {
        /*LLDP debug*/
        LLDP_DEBUG(LLDP_PACKET_TX, "LLDP packet transmit fail because interface %s is mirror dest port\n", 
                   IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return PM_E_NONE;
    }
    /*add by yejl, check the interface is dot1x unauthed in egress*/
    if (p_db_if->dot1x_enable)
    {
        if (GLB_FLAG_ISSET(p_db_if->dot1x_flag, GLB_IF_DOT1X_UNAUTHED) && GLB_FLAG_ISSET(p_db_if->dot1x_flag, GLB_IF_DOT1X_DIR_OUT))
        {
             /*LLDP debug*/
             LLDP_DEBUG(LLDP_PACKET_TX, "LLDP packet transmit fail because interface %s is dot1x unauthed in egress\n", 
                   IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
             return PM_E_NONE;
        }   
    }
    pkt_len = lldp_pdu_encode(pkt, MSG_SIZE, p_db_if, p_db_lldp_if);
    if (pkt_len < 0)
    {
        return PM_E_FAIL;
    }
    PM_E_RETURN(ipc_pkt_tx_lldp(pkt, pkt_len, p_db_lldp_if->key.ifindex));
    p_db_lldp_if->tx_count++;

    return PM_E_NONE;
}

int32
lldp_pdu_decode(tbl_interface_t *p_db_if, tbl_lldp_if_t *p_db_lldp_if, ds_lldp_neighbour_t *p_neigh,
    uint8 *p_buf, uint32 len)
{
    lldp_msap_id_t *p_msap_id = &p_neigh->key;
    uint32 type = 0;
    uint32 subnet_type = 0;
    uint32 length = 0;
    int32   len_left = len;
    int32*  size = NULL;
    uint8** ppnt = NULL;

    uint32 mant_addr_len = 0;
    uint32  oid_len = 0;

    uint32 subtype_oui[3];
    uint32 subtype = 0;

    uint32 vlan_name_len = 0;
    uint8  proto_id_len = 0;
    uint8  proto_payload[LLDP_802_1_PROTO_ID_LEN];
    uint8  proto_id = 0;

    uint32 med_cap_find = 0;
    
    size = &len_left;
    ppnt = &p_buf;

    sal_memset(p_neigh, 0, sizeof(ds_lldp_neighbour_t));
    p_neigh->rx_ifindex = p_db_lldp_if->key.ifindex;

    /* L2 header */
    MSG_DECODE_GET_EMPTY(MAC_ADDR_LEN);                 /* MACDA */
    MSG_DECODE_GET(p_neigh->mac_addr, MAC_ADDR_LEN);    /* MACSA */
    MSG_DECODE_GET_EMPTY(2);                            /* EtherType */
    
    /* Chassis ID TLV */
    LLDP_DECODE_TL(type, length);
    if (LLDP_TLV_TYPE_CHASSIS_ID != type)
    {
        /*LLDP debug*/
        LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP Frame Validation- First TLV is not Chassis ID TLV\n");
        p_db_lldp_if->rx_error_count++;
        p_db_lldp_if->rx_discard_count++;
        return PM_E_FAIL;
    }

    if (length < CHASSIS_ID_TLV_MIN_SIZE || length > CHASSIS_ID_TLV_MAX_SIZE)
    {
        /*LLDP debug*/
        LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP Frame Validation- Chassis ID TLV length %d is not in the range[2,256]\n", length);
        p_db_lldp_if->rx_error_count++;
        p_db_lldp_if->rx_discard_count++;
        return PM_E_FAIL;
    }

    MSG_DECODE_GETC(p_msap_id->chassis_id.type);
    p_msap_id->chassis_id.len = length - 1;
    switch (p_msap_id->chassis_id.type)
    {
    case LLDP_CHASSIS_ID_SUBTYPE_MAC_ADDRESS:
        if (length != (MAC_ADDR_LEN + 1))
        {
            /*LLDP debug*/
            LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP Frame Validation- Chassis ID TLV length %d is not enough for MAC address\n", length);
            p_db_lldp_if->rx_error_count++;
            p_db_lldp_if->rx_discard_count++;
            return PM_E_FAIL;
        }
        sal_memset(p_msap_id->chassis_id.id, 0, sizeof(p_msap_id->chassis_id.id));
        MSG_DECODE_GET(p_msap_id->chassis_id.id, length - 1);
        break;

    case LLDP_CHASSIS_ID_SUBTYPE_NET_ADDRESS:
        MSG_DECODE_GETC(subnet_type);
        if (AF_INET == subnet_type)
        {
            if (length != (sizeof(addr_ipv4_t) + 2))
            {
                 /*LLDP debug*/
                 LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP Frame Validation- Chassis ID TLV length %d is mismatch for IPv4 address\n", length);
                 p_db_lldp_if->rx_error_count++;
                 p_db_lldp_if->rx_discard_count++;
                return PM_E_FAIL;
            }
        }
        else if (AF_INET6 == subnet_type)
        {
            if (length != (sizeof(addr_ipv6_t) + 2))
            {
                /*LLDP debug*/
                LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP Frame Validation- Chassis ID TLV length %d is mismatch for IPv6 address\n", length);
                p_db_lldp_if->rx_error_count++;
                p_db_lldp_if->rx_discard_count++;
                return PM_E_FAIL;
            }
        }
        sal_memset(p_msap_id->chassis_id.id, 0, sizeof(p_msap_id->chassis_id.id));
        p_msap_id->chassis_id.id[0] = subnet_type;
        MSG_DECODE_GET(&p_msap_id->chassis_id.id[1], length - 2);
        break;

    case LLDP_CHASSIS_ID_SUBTYPE_IF_ALIAS:
    case LLDP_CHASSIS_ID_SUBTYPE_IF_NAME:
    case LLDP_CHASSIS_ID_SUBTYPE_LOCALLY_ASSINGED:
    case LLDP_CHASSIS_ID_SUBTYPE_CHASSIS_COM:
    case LLDP_CHASSIS_ID_SUBTYPE_PORT_COM:
        sal_memset(p_msap_id->chassis_id.id, 0, sizeof(p_msap_id->chassis_id.id));
        MSG_DECODE_GET(p_msap_id->chassis_id.id, length - 1);
        break;

    default:
        /*LLDP debug*/
        LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP Frame Validation- Chassis ID TLV subtype %d is unknown\n", p_msap_id->chassis_id.type);
        return PM_E_FAIL;
    }

    /* Port ID TLV */
    LLDP_DECODE_TL(type, length);
    if (type != LLDP_TLV_TYPE_PORT_ID)
    {
        /*LLDP debug*/
        LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP Frame Validation- Second TLV is not Port ID TLV\n");
        p_db_lldp_if->rx_error_count++;
        p_db_lldp_if->rx_discard_count++;
        return PM_E_FAIL;
    }
    if (length < PORT_ID_TLV_MIN_SIZE || length > PORT_ID_TLV_MAX_SIZE)
    {
        /*LLDP debug*/
        LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP Frame Validation- Port ID TLV length %d is not in the range[2,256]\n", length);
        p_db_lldp_if->rx_error_count++;
        p_db_lldp_if->rx_discard_count++;
        return PM_E_FAIL;
    }
    
    MSG_DECODE_GETC(p_msap_id->port_id.type);
    p_msap_id->port_id.len = length - 1;
    switch (p_msap_id->port_id.type)
    {
    case LLDP_PORT_ID_SUBTYPE_MAC_ADDRESS:
        if (length != (MAC_ADDR_LEN + 1))
        {
            /*LLDP debug*/
            LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP Frame Validation- Port ID TLV length %d is not enough for MAC address\n", length);
            p_db_lldp_if->rx_error_count++;
            p_db_lldp_if->rx_discard_count++;
            return PM_E_FAIL;
        }
        sal_memset(p_msap_id->port_id.id, 0, sizeof(p_msap_id->port_id.id));
        MSG_DECODE_GET(p_msap_id->port_id.id, length - 1);
        break;

    case LLDP_PORT_ID_SUBTYPE_NET_ADDRESS:
        MSG_DECODE_GETC(subnet_type);
        if (AF_INET == subnet_type)
        {
            if (length != (sizeof(addr_ipv4_t) + 2))
            {
                /*LLDP debug*/
                LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP Frame Validation- Port ID TLV length %d is mismatch for IPv4 address\n", length);
                p_db_lldp_if->rx_error_count++;
                p_db_lldp_if->rx_discard_count++;
                return PM_E_FAIL;
            }
        }
        else if (AF_INET6 == subnet_type)
        {
            if (length != (sizeof(addr_ipv6_t) + 2))
            {
                /*LLDP debug*/
                LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP Frame Validation- Port ID TLV length %d is mismatch for IPv6 address\n", length);
                p_db_lldp_if->rx_error_count++;
                p_db_lldp_if->rx_discard_count++;
                return PM_E_FAIL;
            }
        }
        sal_memset(p_msap_id->port_id.id, 0, sizeof(p_msap_id->port_id.id));
        p_msap_id->port_id.id[0] = subnet_type;
        MSG_DECODE_GET(&p_msap_id->port_id.id[1], length - 2);
        break;

    case LLDP_PORT_ID_SUBTYPE_IF_ALIAS:
    case LLDP_PORT_ID_SUBTYPE_IF_NAME:
    case LLDP_PORT_ID_SUBTYPE_LOCALLY_ASSINGED:
    case LLDP_PORT_ID_SUBTYPE_AGENT_CIRCUITID:
    case LLDP_PORT_ID_SUBTYPE_PORT_COM:
        sal_memset(p_msap_id->port_id.id, 0, sizeof(p_msap_id->port_id.id));
        MSG_DECODE_GET(p_msap_id->port_id.id, length - 1);
        break;    

    default:
        /*LLDP debug*/
        LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP Frame Validation- Port ID TLV subtype %d is unknown\n", p_msap_id->port_id.type);
        return PM_E_FAIL;
    }

    /* TTL TLV */
    LLDP_DECODE_TL(type, length);
    if (LLDP_TLV_TYPE_TTL != type)
    {
        /*LLDP debug*/
        LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP Frame Validation- Third TLV is not TTL TLV\n");
        p_db_lldp_if->rx_error_count++;
        p_db_lldp_if->rx_discard_count++;
        return PM_E_FAIL;
    }
    if (length < TTL_TLV_MIN_SIZE)
    {
        /*LLDP debug*/
        LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP Frame Validation- TTL TLV length %d is less than 2\n", length);
        p_db_lldp_if->rx_error_count++;
        p_db_lldp_if->rx_discard_count++;
        return PM_E_FAIL;
    }

    MSG_DECODE_GETW(p_neigh->rx_ttl);
    if (p_neigh->rx_ttl == 0)
    {
        /*LLDP debug*/
        LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP TTL is zero\n");
    }
    /*LLDP debug*/
    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP recieve TTL value is %d\n", p_neigh->rx_ttl);
    
    /* optional TLV */
    LLDP_DECODE_TL(type, length);
    while (type != LLDP_TLV_TYPE_END_OF_LLDPDU)
    {
        switch (type)
        {
        case LLDP_TLV_TYPE_PORT_DESCRIPTION:
            MSG_DECODE_GET(p_neigh->port_desc, length);
            LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP PORT DESCRIPTION TLV- value %s\n", p_neigh->port_desc);
            break;
        case LLDP_TLV_TYPE_SYSTEM_NAME:
            MSG_DECODE_GET(p_neigh->system_name, length);
            LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP SYSTEM NAME TLV- value %s\n", p_neigh->system_name);
            break;
        case LLDP_TLV_TYPE_SYSTEM_DESCRIPTION:
            MSG_DECODE_GET(p_neigh->system_desc, length);
            LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP SYSTEM DESCRIPTION TLV- value %s\n", p_neigh->system_desc);
            break;
        case LLDP_TLV_TYPE_SYSTEM_CAPABILITIES:
            MSG_DECODE_GETW(p_neigh->sys_cap);
            MSG_DECODE_GETW(p_neigh->sys_cap_en);
            LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP SYSTEM CAPABILITIES TLV- cap: %d, enabled: %d\n", 
                             p_neigh->sys_cap, p_neigh->sys_cap_en);
            break;
        case LLDP_TLV_TYPE_MANAGEMENT_ADDRESS:
            MSG_DECODE_GETC(mant_addr_len);
            MSG_DECODE_GETC(p_neigh->mgmt_addr_sub_type);
            switch (p_neigh->mgmt_addr_sub_type)
            {
            case LLDP_MANAGEMENT_ADDRESS_SUBTYPE_IPV4:
                if (mant_addr_len != sizeof(addr_ipv4_t) + 1)
                {
                    p_db_lldp_if->rx_error_count++;
                    p_db_lldp_if->rx_discard_count++;
                    return PM_E_FAIL;
                }
                break;
            case LLDP_MANAGEMENT_ADDRESS_SUBTYPE_IPV6:
                if (mant_addr_len != sizeof(addr_ipv6_t) + 1)
                {
                    p_db_lldp_if->rx_error_count++;
                    p_db_lldp_if->rx_discard_count++;
                    return PM_E_FAIL;
                }
                break;
            case LLDP_MANAGEMENT_ADDRESS_SUBTYPE_ALL802:
                if (mant_addr_len != MAC_ADDR_LEN + 1)
                {
                    p_db_lldp_if->rx_error_count++;
                    p_db_lldp_if->rx_discard_count++;
                    return PM_E_FAIL;
                }
                break;
            default:
                return PM_E_FAIL;
            }
            MSG_DECODE_GET(p_neigh->mant_addr, mant_addr_len - 1);
            MSG_DECODE_GETC(p_neigh->if_numbering);
            MSG_DECODE_GETL(p_neigh->if_number);
            MSG_DECODE_GETC(oid_len);
            p_neigh->oid_len = oid_len;
            MSG_DECODE_GET(p_neigh->oid, p_neigh->oid_len);    
            LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP MANAGEMENT ADDRESS TLV- subtype: %d, address_length: %d, if numbering: %d, if number: %d\n", 
                             p_neigh->mgmt_addr_sub_type, mant_addr_len, p_neigh->if_numbering, p_neigh->if_number);
            break;
        case LLDP_TLV_TYPE_ORGANIZATION_SPECIFIC:
            MSG_DECODE_GETC(subtype_oui[0]);
            MSG_DECODE_GETC(subtype_oui[1]);
            MSG_DECODE_GETC(subtype_oui[2]);
            if (subtype_oui[0] == IEEE_8021_OUI_1 && subtype_oui[1] == IEEE_8021_OUI_2 && subtype_oui[2] == IEEE_8021_OUI_3)
            {
                 MSG_DECODE_GETC(subtype);
                 switch (subtype)
                 {
                 case PORT_VLAN_ID_TLV_SUBTYPE:
                    MSG_DECODE_GETW(p_neigh->pvid);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP PORT VLAN ID TLV- value: %d\n", p_neigh->pvid);
                    break;
                 case PORT_AND_PROTOCOL_VLAN_ID_TLV_SUBTYPE:
                    MSG_DECODE_GETC(p_neigh->ppvid_flag);
                    MSG_DECODE_GETW(p_neigh->ppvid);
                    break;
                 case VLAN_NAME_TLV_SUBTYPE:
                    MSG_DECODE_GETW(p_neigh->vlan_id);
                    MSG_DECODE_GETC(vlan_name_len);
                    MSG_DECODE_GET(p_neigh->vlan_name, vlan_name_len);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP VLAN NAME TLV- vlan_id: %d, vlan-name: %s\n", 
                                p_neigh->vlan_id, p_neigh->vlan_name);
                    break;

                 case PROTOCOL_IDENTITY_TLV_SUBTYPE:
                    sal_memset(proto_payload, 0, sizeof(proto_payload));
                    MSG_DECODE_GETC(proto_id_len);
                    MSG_DECODE_GET(proto_payload, proto_id_len);
                    proto_id = lldp_get_protocol_id(proto_payload, proto_id_len);
                    GLB_SET_BIT(p_neigh->protocol_id, proto_id);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP PROTOCOL IDENTITY TLV- proto: %d\n", proto_id);
                    break;

                 case LINK_AGGREGATION_8021_TLV_SUBTYPE:
                    MSG_DECODE_GETC(p_neigh->link_aggr_status);
                    MSG_DECODE_GETL(p_neigh->link_aggr_id);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP LINK AGGREGATION 8021 TLV- status: %d, agg_id: %d\n", 
                                p_neigh->link_aggr_status, p_neigh->link_aggr_id);
                    break;
                 default:
                    return PM_E_FAIL;
                 }
            }
            else if (subtype_oui[0] == IEEE_8023_OUI_1 && subtype_oui[1] == IEEE_8023_OUI_2 && subtype_oui[2] == IEEE_8023_OUI_3)
            {
                MSG_DECODE_GETC(subtype);
                switch (subtype)
                {
                 case MAC_PHY_CONFIG_STATUS_TLV_SUBTYPE:
                    MSG_DECODE_GETC(p_neigh->autonego_support);
                    MSG_DECODE_GETW(p_neigh->autonego_cap);
                    MSG_DECODE_GETW(p_neigh->oper_mau_type);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP MAC PHY CONFIG STATUS TLV- status: %d, cap: %d, mau: %d\n", 
                               p_neigh->autonego_support, p_neigh->autonego_cap, p_neigh->oper_mau_type);
                    break;
                 case POWER_VIA_MDI_TLV_SUBTYPE:
                    MSG_DECODE_GETC(p_neigh->power_flag);
                    MSG_DECODE_GETC(p_neigh->power_pair);
                    MSG_DECODE_GETC(p_neigh->power_class);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP POWER VIA MDI TLV- flag: %d, pair: %d, class: %d\n", 
                               p_neigh->power_flag, p_neigh->power_pair, p_neigh->power_class);
                    break;
                 case LINK_AGGREGATION_TLV_SUBTYPE:
                    if (p_neigh->link_aggr_status)
                    {
                        break;
                    }
                    MSG_DECODE_GETC(p_neigh->link_aggr_status);
                    MSG_DECODE_GETL(p_neigh->link_aggr_id);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP LINK AGGREGATION 8023 TLV- status: %d, agg_id: %d\n", 
                                p_neigh->link_aggr_status, p_neigh->link_aggr_id);
                    break;
                 case MAXIMUM_FRAME_SIZE_TLV_SUBTYPE:
                    MSG_DECODE_GETW(p_neigh->max_frame_size);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP MAXIMUM FRAME SIZE TLV- value: %d\n", p_neigh->max_frame_size);
                    break;
                 default:
                    return PM_E_FAIL;
                }
            }
            else if (subtype_oui[0] == TIA_OUI_1 && subtype_oui[1] == TIA_OUI_2 && subtype_oui[2] == TIA_OUI_3)
            {
                MSG_DECODE_GETC(subtype);
                switch (subtype)
                {
                case MED_TYPE_CAPABILITIES_SUBTYPE:
                    if (med_cap_find)
                    {
                         LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- Duplicate med_cap_tlv\n");
                         p_db_lldp_if->rx_error_count++;
                         p_db_lldp_if->rx_discard_count++;
                         return PM_E_FAIL;
                    }
                    med_cap_find = 1;
                    MSG_DECODE_GETW(p_neigh->med_capbility); 
                    MSG_DECODE_GETC(p_neigh->med_dev_type);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP MED CAPABILITIES TLV- cap: %d, type: %d\n", 
                                p_neigh->med_capbility, p_neigh->med_dev_type);
                    break;
                case MED_TYPE_NETWORK_POLICY_SUBTYPE:
                    if (!med_cap_find)
                    {
                         LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- med_cap_tlv should at first for med tlv set\n");
                         p_db_lldp_if->rx_error_count++;
                         p_db_lldp_if->rx_discard_count++;
                         return PM_E_FAIL;
                    }
                    MSG_DECODE_GETL(p_neigh->med_policy_flag);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP MED NETWORK POLICY TLV- value: %d\n", p_neigh->med_policy_flag);
                    break;
                case MED_TYPE_LOCATION_ID_SUBTYPE:
                    if (!med_cap_find)
                    {
                         LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- Med_cap_tlv should at first for med tlv set\n");
                         p_db_lldp_if->rx_error_count++;
                         p_db_lldp_if->rx_discard_count++;
                         return PM_E_FAIL;
                    }
                    if (length < 5)
                    {
                         LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- The length %d of LCI TLV is wrong\n", length);
                         p_db_lldp_if->rx_error_count++;
                         p_db_lldp_if->rx_discard_count++;
                         return PM_E_FAIL;
                    }
                    MSG_DECODE_GETC(p_neigh->med_lci_format);
                    if (MED_LCI_FORMAT_CIVIC == p_neigh->med_lci_format)
                    {
                        uint32 ca_len = 0;
                        uint32 ca_num = 0;
                        uint32 cli_len = 0;
                        MSG_DECODE_GETC(p_neigh->med_lci_civic.len);
                        if (p_neigh->med_lci_civic.len < 5)
                        {
                             p_db_lldp_if->tlv_discard_count++;
                             break;
                        }
                        MSG_DECODE_GETC(p_neigh->med_lci_civic.what);
                        MSG_DECODE_GET(p_neigh->med_lci_civic.countrycode, LLDP_MED_LCI_CIVIC_COUNTRYCODE_SIZE);
                        
                        cli_len = p_neigh->med_lci_civic.len - 3;
                        while ((ca_len + 2 * ca_num)  < cli_len)
                        {
                            MSG_DECODE_GETC(p_neigh->med_lci_civic.ca_type[ca_num]);
                            MSG_DECODE_GETC(p_neigh->med_lci_civic.ca_length[ca_num]);
                            MSG_DECODE_GET(p_neigh->med_lci_civic.ca_value[ca_num], p_neigh->med_lci_civic.ca_length[ca_num]);
                            
                            ca_len = ca_len + p_neigh->med_lci_civic.ca_length[ca_num];
                            p_neigh->med_lci_civic.ca_num ++;
                            ca_num++;
                        } 
                        if ((ca_len + 2 * ca_num) != cli_len)
                        {
                            LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP Frame Validation- LLDP med Location Id Civic type length is wrong\n");
                            p_db_lldp_if->rx_error_count++;
                            p_db_lldp_if->rx_discard_count++;
                            return PM_E_FAIL;   
                        }
                        LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP MED LCI TLV- civic:len %d, what %d, country %s, ca_num %d\n",
                                    p_neigh->med_lci_civic.len, p_neigh->med_lci_civic.what, p_neigh->med_lci_civic.countrycode,
                                    p_neigh->med_lci_civic.ca_num);
                    }
                    else if (MED_LCI_FORMAT_ECS_ELIN == p_neigh->med_lci_format)
                    {
                        if (length < (10 + 5) || length > (25 + 5))
                        {
                             LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- LLDP Frame Validation- LLDP med Location Id ECS-ELIN type length is wrong\n");
                             p_db_lldp_if->tlv_discard_count++;
                             length = length - 5;
                             break;
                        }
                        MSG_DECODE_GET(p_neigh->med_lci_elin, length - 5);
                        LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP MED LCI TLV- ecs_elin: %d\n", p_neigh->med_lci_elin);
                    }
                    else if (MED_LCI_FORMAT_COORDINATE == p_neigh->med_lci_format)
                    {
                        if (length != (5 + 16))
                        {
                            return PM_E_FAIL;
                        }
                        MSG_DECODE_GET(p_neigh->med_lci_coordinate, LLDP_MED_LCI_COORDINATE_SIZE);
                    }
                    else
                    {
                        p_db_lldp_if->rx_discard_count++;
                        return PM_E_FAIL;
                    }
                    break;
                case MED_TYPE_EXT_POWER_VIA_MDI_SUBTYPE:
                    if (!med_cap_find)
                    {
                         LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- Med_cap_tlv should at first for med tlv set\n");
                         p_db_lldp_if->rx_error_count++;
                         p_db_lldp_if->rx_discard_count++;
                         return PM_E_FAIL;                           
                    }
                    MSG_DECODE_GETC(p_neigh->med_power_flag); 
                    MSG_DECODE_GETW(p_neigh->med_power_value); 
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP MED EXT POWER VIA MDI TLV- flag: %d, value: %d\n",
                                p_neigh->med_power_flag, p_neigh->med_power_value);
                    break;
                case MED_TYPE_INVENTORY_SUBTYPE_HARDWARE:
                    if (!med_cap_find)
                    {
                         LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- Med_cap_tlv should at first for med tlv set\n");
                         p_db_lldp_if->rx_error_count++;
                         p_db_lldp_if->rx_discard_count++;
                         return PM_E_FAIL;                           
                    }
                    MSG_DECODE_GET(p_neigh->med_hard_ver, length - 4);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP MED INVENTORY TLV- type is hardware, value is %s\n", 
                                p_neigh->med_hard_ver);
                    break;
                case MED_TYPE_INVENTORY_SUBTYPE_FIRMWARE:
                    if (!med_cap_find)
                    {
                         LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- Med_cap_tlv should at first for med tlv set\n");
                         p_db_lldp_if->rx_error_count++;
                         p_db_lldp_if->rx_discard_count++;
                         return PM_E_FAIL;                           
                    }
                    MSG_DECODE_GET(p_neigh->med_firm_ver, length - 4);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP MED INVENTORY TLV- type is firmware, value is %s\n", 
                                p_neigh->med_firm_ver);
                    break;
                case MED_TYPE_INVENTORY_SUBTYPE_SOFTWARE:
                    if (!med_cap_find)
                    {
                         LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- Med_cap_tlv should at first for med tlv set\n");
                         p_db_lldp_if->rx_error_count++;
                         p_db_lldp_if->rx_discard_count++;
                         return PM_E_FAIL;                           
                    }
                    MSG_DECODE_GET(p_neigh->med_soft_ver, length - 4);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP MED INVENTORY TLV- type is software, value is %s\n", 
                                p_neigh->med_soft_ver);
                    break;
                case MED_TYPE_INVENTORY_SUBTYPE_SERIAL:
                    if (!med_cap_find)
                    {
                         LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- Med_cap_tlv should at first for med tlv set\n");
                         p_db_lldp_if->rx_error_count++;
                         p_db_lldp_if->rx_discard_count++;
                         return PM_E_FAIL;                           
                    }
                    MSG_DECODE_GET(p_neigh->med_serial_ver, length - 4);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP MED INVENTORY TLV- type is serial, value is %s\n", 
                                p_neigh->med_serial_ver);
                    break;
                case MED_TYPE_INVENTORY_SUBTYPE_MANUFACTURER_NAME:
                    if (!med_cap_find)
                    {
                         LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- Med_cap_tlv should at first for med tlv set\n");
                         p_db_lldp_if->rx_error_count++;
                         p_db_lldp_if->rx_discard_count++;
                         return PM_E_FAIL;                           
                    }
                    MSG_DECODE_GET(p_neigh->med_manufac_ver, length - 4);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP MED INVENTORY TLV- type is manufacturer name, value is %s\n", 
                                p_neigh->med_manufac_ver);
                    break;
                case MED_TYPE_INVENTORY_SUBTYPE_MODEL_NAME:
                    if (!med_cap_find)
                    {
                         LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- Med_cap_tlv should at first for med tlv set\n");
                         p_db_lldp_if->rx_error_count++;
                         p_db_lldp_if->rx_discard_count++;
                         return PM_E_FAIL;                           
                    }
                    MSG_DECODE_GET(p_neigh->med_moname_ver, length - 4);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP MED INVENTORY TLV- type is model name, value is %s\n", 
                                p_neigh->med_moname_ver);
                    break;
                case MED_TYPE_INVENTORY_SUBTYPE_ASSET_ID:
                    if (!med_cap_find)
                    {
                         LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- Med_cap_tlv should at first for med tlv set\n");
                         p_db_lldp_if->rx_error_count++;
                         p_db_lldp_if->rx_discard_count++;
                         return PM_E_FAIL;                           
                    }
                    MSG_DECODE_GET(p_neigh->med_asset_ver, length - 4);
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: LLDP MED INVENTORY TLV- type is asset id, value is %s\n", 
                                p_neigh->med_asset_ver);
                    break;
                default:
                    LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- Unknown MED TLV sub type %d\n", subtype);
                    p_db_lldp_if->tlv_discard_count++;
                    p_db_lldp_if->tlv_unrecognized_count++;
                    break;
                }
            }
            else
            {
                 LLDP_DEBUG(LLDP_PACKET_RX, "RX: Error- Unknown TLV type %d\n", type);
                 p_db_lldp_if->tlv_discard_count++;
                 p_db_lldp_if->tlv_unrecognized_count++;
                 break;
            }
        }
        LLDP_DECODE_TL(type, length);
    }

    return PM_E_NONE;
}

int32
lldp_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    tbl_lldp_global_t *p_db_glb = tbl_lldp_global_get_lldp_global();
    tbl_lldp_if_key_t key;
    tbl_interface_t *p_db_if = NULL;
    tbl_lldp_if_t *p_db_lldp_if = NULL;
    ds_lldp_neighbour_t neigh;
    char ifname_ext[IFNAME_SIZE];
    uint32 ifindex = 0;
    int32 rc = PM_E_NONE;

    /* check global enable */
    if (!p_db_glb->enable)
    {
        /*LLDP debug*/
        LLDP_DEBUG(LLDP_PACKET_RX, "LLDP packet recieve fail because lldp is not enable globally\n");
        return PM_E_NONE;
    }

    ifindex = p_msg->u_hdr.pkt.ifindex;
    key.ifindex = ifindex;
    p_db_lldp_if = tbl_lldp_if_get_lldp_if(&key);
    if (NULL == p_db_lldp_if || NULL == p_db_lldp_if->intf)
    {
        return PM_E_NONE;
    }
    p_db_if = (tbl_interface_t*)p_db_lldp_if->intf;
    if (!tbl_interface_is_running(p_db_if))
    {
        /*LLDP debug*/
        LLDP_DEBUG(LLDP_PACKET_RX, "LLDP packet recieve fail because interface %s is not up\n", 
                   IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return PM_E_NONE;
    }
    /*check the interface is mirror dest interface*/
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG))
    {
        /*LLDP debug*/
        LLDP_DEBUG(LLDP_PACKET_RX, "LLDP packet recieve fail because interface %s is mirror dest port\n", 
                   IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return PM_E_NONE;
    }
    /*add by yejl for bug38388, check the interface is dot1x unauthed in ingress*/
    if (p_db_if->dot1x_enable)
    {
        if (GLB_FLAG_ISSET(p_db_if->dot1x_flag, GLB_IF_DOT1X_UNAUTHED))
        {
            /*LLDP debug*/
            LLDP_DEBUG(LLDP_PACKET_RX, "LLDP packet recieve fail because interface %s is dot1x unauthed in ingress\n", 
                   IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
            return PM_E_NONE;
        }
    }
    /* check port enable */
    if ((LLDP_IF_TXRX != p_db_lldp_if->mode) && (LLDP_IF_RXONLY != p_db_lldp_if->mode))
    {
        /*LLDP debug*/
        LLDP_DEBUG(LLDP_PACKET_RX, "LLDP packet recieve fail because interface %s is not rx or txrx\n", 
                   IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext));
        return PM_E_NONE;
    }
    rc = lldp_pdu_decode(p_db_if, p_db_lldp_if, &neigh, p_msg->data, p_msg->data_len);
    if (PM_E_NONE == rc)
    {
        p_db_lldp_if->rx_count++;
        lldp_if_update_neigh(p_db_lldp_if, &neigh);
    }
    else
    {
        p_db_lldp_if->rx_error_count++;
    }

    return PM_E_NONE;
}

int32
lldp_pdu_tx_disable_encode(uint8 *p_buf, uint32 len, tbl_interface_t *p_db_if, tbl_lldp_if_t *p_db_lldp_if)
{
    tbl_lldp_global_t *p_db_glb = tbl_lldp_global_get_lldp_global();
    char ifname_ext[IFNAME_SIZE];
    int32  len_left = len;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    uint32 length = 0;
    
    size = &len_left;
    ppnt = &p_buf;

    /* L2 header */
    MSG_ENCODE_PUT(lldp_mac_address, MAC_ADDR_LEN);
    MSG_ENCODE_PUT(p_db_if->mac_addr, MAC_ADDR_LEN);
    MSG_ENCODE_PUTW(ETH_P_LLDP);                        /* Table 8-2¡ªLLDP Ethertype */

    /* 1) Chassis ID TLV */
    LLDP_ENCODE_TL(LLDP_TLV_TYPE_CHASSIS_ID, (MAC_ADDR_LEN + 1));
    MSG_ENCODE_PUTC(LLDP_CHASSIS_ID_SUBTYPE_MAC_ADDRESS);
    if (p_db_glb->system_mac_en)
    {
        MSG_ENCODE_PUT(p_db_glb->system_mac, MAC_ADDR_LEN);
    }
    else
    {
        MSG_ENCODE_PUT(p_db_if->hw_mac_addr, MAC_ADDR_LEN);
    }
    
    /* 2) Port ID TLV */
    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    length = sal_strlen(ifname_ext);
    LLDP_ENCODE_TL(LLDP_TLV_TYPE_PORT_ID, length + 1);
    MSG_ENCODE_PUTC(LLDP_PORT_ID_SUBTYPE_IF_NAME);
    MSG_ENCODE_PUT(ifname_ext, length);
    
    /* 3) Time To Live TLV */
    LLDP_ENCODE_TL(LLDP_TLV_TYPE_TTL, 2);
    MSG_ENCODE_PUTW(0);

    /* End Of LLDPDU TLV */
    LLDP_ENCODE_TL(LLDP_TLV_TYPE_END_OF_LLDPDU, 0);

    return (len - len_left);
}

/*modified by yejl to fix bug40072, 2016-08-10*/
int32
lldp_pdu_tx_disable(tbl_lldp_if_t *p_db_lldp_if)
{
    tbl_interface_t *p_db_if = NULL;
    uint8 pkt[MSG_SIZE];
    int32 pkt_len = 0;
    int32 rc = PM_E_NONE;
    
    p_db_if = (tbl_interface_t*)p_db_lldp_if->intf;
    if (p_db_if == NULL)
    {
        return PM_E_NONE;
    }
    if (!tbl_interface_is_running(p_db_if))
    {
        return PM_E_NONE;
    }
    /*check the interface is mirror dest interface*/
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG))
    {
        return PM_E_NONE;
    }
    /*add by yejl, check the interface is dot1x unauthed in egress*/
    if (p_db_if->dot1x_enable)
    {
        if (GLB_FLAG_ISSET(p_db_if->dot1x_flag, GLB_IF_DOT1X_UNAUTHED) && GLB_FLAG_ISSET(p_db_if->dot1x_flag, GLB_IF_DOT1X_DIR_OUT))
        {
             return PM_E_NONE;
        }   
    }
    pkt_len = lldp_pdu_tx_disable_encode(pkt, MSG_SIZE, p_db_if, p_db_lldp_if);
    if (pkt_len < 0)
    {
        return PM_E_FAIL;
    }
    PM_E_RETURN(ipc_pkt_tx_lldp(pkt, pkt_len, p_db_lldp_if->key.ifindex));
    p_db_lldp_if->tx_count++;

    return PM_E_NONE;
}

