#ifndef __HAGT_OPENFLOW_PORT_H__
#define __HAGT_OPENFLOW_PORT_H__


/* Common flags to indicate behavior of the physical port.  These flags are
 * used in ofp_port to describe the current configuration.  They are used in
 * the ofp_port_mod message to configure the port's behavior.
 */
enum hagt_openflow_port_config {
    HAGT_OPENFLOW_PORT_DOWN    = 1 << 0,  /* Port is administratively down. */
    HAGT_OPENFLOW_NO_RECV      = 1 << 2,  /* Drop all packets received by port. */
    HAGT_OPENFLOW_NO_FWD       = 1 << 5,  /* Drop packets forwarded to port. */
    HAGT_OPENFLOW_NO_PACKET_IN = 1 << 6   /* Do not send packet-in msgs for port. */
};

/*
*  pdu l2pdu action
*/
enum hagt_openflow_port_l2pdu_action_e
{
    HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_REDIRECT_TO_CPU,         /* layer2 pdu action type: redirect to cpu*/
    HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_COPY_TO_CPU,             /* layer2 pdu action type: copy to cpu*/
    HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_FWD,                     /* layer2 pdu action type: normal forwarding*/
    HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_DISCARD,                 /* layer2 pdu action type: discard the pdu*/

    HAGT_OPENFLOW_PORT_L2PDU_ACTION_TYPE_MAX
};
typedef enum hagt_openflow_port_l2pdu_action_e hagt_openflow_port_l2pdu_action_t;

/*
*  pdu l2pdu action index
*/
enum hagt_openflow_port_l2pdu_action_indx_e
{
    HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_STP = 0,                 /* layer2 pdu action index: stp*/
    HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_SLOW_PROTO = 1,              /* layer2 pdu action index: slow protocol(lacp)*/
    HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_EAPOL     = 2,
    HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_LLDP      = 3,
    HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_ISIS       = 4,   /* ISIS action index*/
    HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_EFM_OAM    = 5,   /* Efm oam action index*/
    HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_FIP        = 6,   /* FIP(FCoE Initialization Protocol) action index*/

    /*SYSTEM MODIFIED, Modified by System to suport erps */
    HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_ERPS       = 7,   /* ERPS */
    HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_RSV_BEGIN = 8,    /* The begin of reserve action index*/
    HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_RSV_END   = 14,   /* The end of reserve action index*/
    HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_MACDA     = 63,    /* MACDA exception to cpu action index*/

    HAGT_OPENFLOW_PORT_L2PDU_ACTION_INDEX_MAX
};
typedef enum hagt_openflow_port_l2pdu_action_indx_e hagt_openflow_port_l2pdu_action_indx_t;

/* OpenFlow 1.0 specific flags to indicate behavior of the physical port.
 * These flags are used in ofp10_phy_port to describe the current
 * configuration.  They are used in the ofp10_port_mod message to configure the
 * port's behavior.
 */
enum ofp10_port_config {
    HAGT_OPENFLOW_PORT_NO_RECV_STP  = 1 << 3, /* Drop received 802.1D STP packets. */
    HAGT_OPENFLOW_PORT_NO_FLOOD     = 1 << 4, /* Do not include port when flooding. */
};

struct hagt_openflow_port_mac_s
{    
    uint32 gport;   /* global port index */
    mac_addr_t  mac; 
};
typedef struct hagt_openflow_port_mac_s hagt_openflow_port_mac_t;


int32
hagt_openflow_port_register_callback(void);


#endif /* !__HAGT_OPENFLOW_PORT_H__ */
