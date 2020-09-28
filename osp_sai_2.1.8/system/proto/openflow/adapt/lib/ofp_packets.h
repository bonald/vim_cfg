#ifndef __OFP_PACKETS_H__
#define __OFP_PACKETS_H__

#define ETH_TYPE_VLAN_8021Q_9100    0x9100
#define ETH_TYPE_VLAN_8021Q_9200    0x9200

typedef struct 
{
    uint32_ofp type;
    
    uint16_ofp in_port;
    uint16_ofp rsv;
    uint32_ofp tunnel_id;
    uint32_ofp flow_id;
    // Not used now. bool is_table_miss;
    struct rule_ctc* p_rule;

    uint8_ofp  table_id;
    uint8_ofp  rsv2;
    uint16_ofp metadata;
    uint16_ofp slave_port;
    uint16_ofp rsv3;
    
} ofp_to_ovs_packet_info_t;

#endif /* !__OFP_PACKETS_H__ */
