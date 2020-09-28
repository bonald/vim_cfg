
#ifndef __TBL_LLDP_IF_DEFINE_H__
#define __TBL_LLDP_IF_DEFINE_H__

#include "gen/ds_lldp_neighbour_define.h"

/* TBL_LLDP_IF field defines */
typedef enum
{
    TBL_LLDP_IF_FLD_KEY                  = 0 ,  /* RW */
    TBL_LLDP_IF_FLD_MODE                 = 1 ,  /* RW */
    TBL_LLDP_IF_FLD_TX_TTL               = 2 ,  /* RW */
    TBL_LLDP_IF_FLD_TX_COUNT             = 3 ,  /* RW */
    TBL_LLDP_IF_FLD_RX_COUNT             = 4 ,  /* READ */
    TBL_LLDP_IF_FLD_RX_ERROR_COUNT       = 5 ,  /* READ */
    TBL_LLDP_IF_FLD_RX_DISCARD_COUNT     = 6 ,  /* READ */
    TBL_LLDP_IF_FLD_TLV_DISCARD_COUNT    = 7 ,  /* READ */
    TBL_LLDP_IF_FLD_TLV_UNRECOGNIZED_COUNT = 8 ,  /* READ */
    TBL_LLDP_IF_FLD_AGED_COUNT           = 9 ,  /* READ */
    TBL_LLDP_IF_FLD_TLV_ENABLE_BASIC     = 10,  /* RW */
    TBL_LLDP_IF_FLD_TLV_ENABLE_8021      = 11,  /* RW */
    TBL_LLDP_IF_FLD_TLV_ENABLE_8023      = 12,  /* RW */
    TBL_LLDP_IF_FLD_TLV_ENABLE_MED       = 13,  /* RW */
    TBL_LLDP_IF_FLD_PPVID_FLAG           = 14,  /* RW */
    TBL_LLDP_IF_FLD_PPVID                = 15,  /* RW */
    TBL_LLDP_IF_FLD_PROTOCOL_ID          = 16,  /* RW */
    TBL_LLDP_IF_FLD_VLAN_NAME            = 17,  /* RW */
    TBL_LLDP_IF_FLD_VLAN_NAME_ID         = 18,  /* RW */
    TBL_LLDP_IF_FLD_MDI_SUPPORT          = 19,  /* READ */
    TBL_LLDP_IF_FLD_PSE_POWER_PAIR       = 20,  /* READ */
    TBL_LLDP_IF_FLD_POWER_CLASS          = 21,  /* READ */
    TBL_LLDP_IF_FLD_MED_CAPBILITY        = 22,  /* READ */
    TBL_LLDP_IF_FLD_MED_DEV_TYPE         = 23,  /* READ */
    TBL_LLDP_IF_FLD_MED_POWER_FLAG       = 24,  /* READ */
    TBL_LLDP_IF_FLD_MED_POWER_VALUE      = 25,  /* READ */
    TBL_LLDP_IF_FLD_MED_POLICY_FLAG      = 26,  /* READ */
    TBL_LLDP_IF_FLD_MED_LCI_FORMAT       = 27,  /* RW */
    TBL_LLDP_IF_FLD_MED_LCI_COORDINATE   = 28,  /* RW */
    TBL_LLDP_IF_FLD_MED_LCI_ELIN         = 29,  /* RW */
    TBL_LLDP_IF_FLD_MED_LCI_CIVIC        = 30,  /* RW */
    TBL_LLDP_IF_FLD_TX_TIMER             = 31,  /* RW */
    TBL_LLDP_IF_FLD_NEIGH_LIST           = 32,  /* SUB */
    TBL_LLDP_IF_FLD_MAX                  = 33
} tbl_lldp_if_field_id_t;

/* TBL_LLDP_IF defines */
typedef struct
{
    uint32               ifindex;
} tbl_lldp_if_key_t;

typedef struct
{
    tbl_lldp_if_key_t    key;
    uint32               mode;                /* lldp_if_mode_t */
    uint32               tx_ttl;
    uint32               tx_count;
    uint32               rx_count;
    uint32               rx_error_count;
    uint32               rx_discard_count;
    uint32               tlv_discard_count;
    uint32               tlv_unrecognized_count;
    uint32               aged_count;
    uint32               tlv_enable_basic;    /*basic tvl*/
    uint32               tlv_enable_8021;     /*8021 tvl*/
    uint32               tlv_enable_8023;     /*8023 tvl*/
    uint32               tlv_enable_med;      /*med tvl*/
    uint32               ppvid_flag;          /*8021 prot-protocol-vlan-id TLV:flag*/
    uint32               ppvid;               /*8021 prot-protocol-vlan-id TLV:value*/
    uint32               protocol_id;         /*8021 protocol-id TLV*/   /* bitmap of lldp_proto_t */
    char                 vlan_name[VLANNAME_SIZE+1]; /*8021 vlan-name TLV: vlan-name*/
    uint32               vlan_name_id;        /*8021 vlan-name TLV: vlan-name-id*/
    uint32               mdi_support;         /*8023 Power Via MDI TLV :mdi_support*/
    uint32               pse_power_pair;      /*8023 Power Via MDI TLV :pse_power_pair*/
    uint32               power_class;         /*8023 Power Via MDI TLV :power_class*/
    uint32               med_capbility;       /*med LLDP-MED Capabilities TLV :med_capbility*/
    uint32               med_dev_type;        /*med LLDP-MED Capabilities TLV :med_dev_type*/
    uint32               med_power_flag;      /*med Extended Power-via-MDI TLV :med_power_flag*/
    uint32               med_power_value;     /*med Extended Power-via-MDI TLV :med_power_value*/
    uint32               med_policy_flag;     /*med Network Policy TLV£ºmed_policy_flag*/
    uint32               med_lci_format;      /*med Location id TLV£ºlocation_id format*/
    char                 med_lci_coordinate[LLDP_MED_LCI_COORDINATE_SIZE+1]; /*med Location id TLV£ºlocation_id format: coordinate*/
    char                 med_lci_elin[LLDP_MED_LCI_ELIN_MAX_SIZE+1]; /*med Location id TLV£ºlocation_id format: ecs-elin*/
    lldp_civic_address_t med_lci_civic;       /*med Location id TLV£ºlocation_id format: civic*/
    ctc_task_t           *tx_timer;
    void                 *intf;               /* refer to tbl_interface_t */
    cdb_reference_list_t neigh_list;
} tbl_lldp_if_t;

typedef struct
{
    ctclib_slist_t       *if_list;
} tbl_lldp_if_master_t;

#endif /* !__TBL_LLDP_IF_DEFINE_H__ */

