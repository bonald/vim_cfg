
#ifndef __DS_LLDP_NEIGHBOUR_DEFINE_H__
#define __DS_LLDP_NEIGHBOUR_DEFINE_H__

/* DS_LLDP_NEIGHBOUR field defines */
typedef enum
{
    DS_LLDP_NEIGHBOUR_FLD_KEY                  = 0 ,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_RX_IFINDEX           = 1 ,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MAC_ADDR             = 2 ,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_RX_TTL               = 3 ,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_CHASSIS_ID_SUB_TYPE  = 4 ,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_PORT_ID_SUB_TYPE     = 5 ,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_SYSTEM_NAME          = 6 ,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_SYSTEM_DESC          = 7 ,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_PORT_DESC            = 8 ,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_IFNAME               = 9 ,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_VLAN_NAME            = 10,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_VLAN_ID              = 11,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_SYS_CAP              = 12,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_SYS_CAP_EN           = 13,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MANT_ADDR            = 14,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_OID                  = 15,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_OID_LEN              = 16,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MGMT_ADDR_SUB_TYPE   = 17,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_IF_NUMBERING         = 18,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_IF_NUMBER            = 19,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_PVID                 = 20,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_PPVID_FLAG           = 21,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_PPVID                = 22,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_PROTOCOL_ID          = 23,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_AUTONEGO_SUPPORT     = 24,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_AUTONEGO_CAP         = 25,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_OPER_MAU_TYPE        = 26,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_LINK_AGGR_STATUS     = 27,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_LINK_AGGR_ID         = 28,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MAX_FRAME_SIZE       = 29,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_POWER_FLAG           = 30,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_POWER_PAIR           = 31,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_POWER_CLASS          = 32,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MED_CAPBILITY        = 33,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MED_DEV_TYPE         = 34,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MED_POLICY_FLAG      = 35,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MED_POWER_FLAG       = 36,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MED_POWER_VALUE      = 37,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MED_LCI_FORMAT       = 38,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MED_HARD_VER         = 39,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MED_FIRM_VER         = 40,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MED_SOFT_VER         = 41,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MED_SERIAL_VER       = 42,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MED_MANUFAC_VER      = 43,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MED_MONAME_VER       = 44,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MED_ASSET_VER        = 45,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MED_LCI_COORDINATE   = 46,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MED_LCI_ELIN         = 47,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MED_LCI_CIVIC        = 48,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_RX_TTL_TIMER         = 49,  /* READ */
    DS_LLDP_NEIGHBOUR_FLD_MAX                  = 50
} ds_lldp_neighbour_field_id_t;

/* DS_LLDP_NEIGHBOUR defines */
typedef lldp_msap_id_t ds_lldp_neighbour_key_t;

typedef struct
{
    ds_lldp_neighbour_key_t key;
    uint32               rx_ifindex;
    mac_addr_t           mac_addr;
    uint32               rx_ttl;
    uint32               chassis_id_sub_type;
    uint32               port_id_sub_type;
    char                 system_name[LLDP_NAME_SIZE+1];
    char                 system_desc[LLDP_DESCRIPTION_SIZE+1];
    char                 port_desc[LLDP_DESCRIPTION_SIZE+1];
    char                 ifname[IFNAME_SIZE+1];
    char                 vlan_name[LLDP_VLAN_NAME_SIZE+1]; /*8021 vlan name*/
    uint32               vlan_id;             /*8021 vlan id*/
    uint32               sys_cap;             /*basic system capability*/
    uint32               sys_cap_en;          /*basic system capability enable*/
    char                 mant_addr[LLDP_MGMT_ADDR_LENGTH]; /*basic management addr*/
    char                 oid[LLDP_OID_LEN_MAX];
    uint32               oid_len;
    uint32               mgmt_addr_sub_type;
    uint32               if_numbering;        /*basic management ip,if_numbering*/
    uint32               if_number;
    uint32               pvid;                /*8021 port vlan id*/
    uint32               ppvid_flag;          /*8021 port protocol vlan id flag*/
    uint32               ppvid;               /*8021 port protocol vlan id*/
    uint32               protocol_id;         /*8021 protocol id*/
    uint32               autonego_support;    /*8023 support autonego or not*/
    uint32               autonego_cap;        /*8023 autonego capability*/
    uint32               oper_mau_type;       /*8023 autonego mau type*/
    uint32               link_aggr_status;    /*8021 or 8023 surpport linkagg or not*/
    uint32               link_aggr_id;        /*8021 or 8023 linkagg id */
    uint32               max_frame_size;      /*8023 max_frame_size*/
    uint32               power_flag;          /*8023 PSE or PD*/
    uint32               power_pair;          /*8023 Signal or Spare or Unknown*/
    uint32               power_class;         /*8023 power class 0,1,2 ...*/
    uint32               med_capbility;       /*med capability*/
    uint32               med_dev_type;        /*med device type:Network Connectivity or Endpoint or Unknown */
    uint32               med_policy_flag;     /*med network policy*/
    uint32               med_power_flag;      /*med power flag*/
    uint32               med_power_value;     /*med power value*/
    uint32               med_lci_format;      /*med lci TLV£ºlocation_id format*/
    char                 med_hard_ver[LLDP_MED_INVENTORY_VALUE_SIZE+1]; /*med Inventory TLV£ºhardware version*/
    char                 med_firm_ver[LLDP_MED_INVENTORY_VALUE_SIZE+1]; /*med Inventory TLV£ºfirmware version*/
    char                 med_soft_ver[LLDP_MED_INVENTORY_VALUE_SIZE+1]; /*med Inventory TLV£ºsoftware version*/
    char                 med_serial_ver[LLDP_MED_INVENTORY_VALUE_SIZE+1]; /*med Inventory TLV£ºserial version*/
    char                 med_manufac_ver[LLDP_MED_INVENTORY_VALUE_SIZE+1]; /*med Inventory TLV£ºmanufacturer version*/
    char                 med_moname_ver[LLDP_MED_INVENTORY_VALUE_SIZE+1]; /*med Inventory TLV£ºmodel name version*/
    char                 med_asset_ver[LLDP_MED_INVENTORY_VALUE_SIZE+1]; /*med Inventory TLV£ºasset version*/
    char                 med_lci_coordinate[LLDP_MED_LCI_COORDINATE_SIZE+1]; /*med lci TLV£ºlocation_id format: coordinate*/
    char                 med_lci_elin[LLDP_MED_LCI_ELIN_MAX_SIZE+1]; /*med lci TLV£ºlocation_id format: ecs-elin*/
    lldp_civic_address_t med_lci_civic;       /*med lci TLV£ºlocation_id format: civic*/
    ctc_task_t           *rx_ttl_timer;
} ds_lldp_neighbour_t;

#endif /* !__DS_LLDP_NEIGHBOUR_DEFINE_H__ */

