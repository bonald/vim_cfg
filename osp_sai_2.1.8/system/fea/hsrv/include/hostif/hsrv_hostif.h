#ifndef __HSRV_HOSTINTF_H__
#define __HSRV_HOSTINTF_H__

#define SLOW_PROTO_CONTROL_LACP 0x01
#define SLOW_PROTO_MARKER_LACP  0x02
#define SLOW_PROTO_OAM          0x03
#define SLOW_PROTO_SSM          0x0a

typedef struct
{
    glb_packet_tocpu_reason_t reason_id;
    sai_object_id_t trap_group_oid;
    sai_object_id_t trap_group_policer_oid;
    sai_object_id_t copp_entry_oid[8];
} hsrv_hostintf_trap_group_t;

typedef struct
{
    int32 pm_pkt_sock;    /* packet sock to PMs */

    hsrv_hostintf_trap_group_t reason_info[GLB_CPU_TRAFFIC_MAX];

    sai_object_id_t group_queue[GLB_CPU_TRAFFIC_GROUP_MAX];
    sai_object_id_t scheduler_id[GLB_CPU_TRAFFIC_GROUP_MAX];
    
    sai_hostif_api_t *hostintf_api;
    sai_copp_api_t   *copp_api;
} hsrv_hostintf_master_t;

int32
hsrv_hostif_create(char *ifname, uint64 portid, uint64 *hif_id);

int32
hsrv_hostif_remove(uint64 hif_id);

int32
hsrv_packet_event_init();

int32 
hsrv_cpu_traffic_update_ssh_entry(uint16 dst_port);

int32 
hsrv_cpu_traffic_update_telnet_entry(uint16 dst_port);

void 
hsrv_packet_event_notification(
    void *buffer, 
    sai_size_t buffer_size, 
    uint32_t attr_count,
    const sai_attribute_t *attr_list);

void 
hsrv_cpu_packet_debug(
    void *buffer, 
    sai_size_t buffer_size, 
    uint32_t attr_count,
    const sai_attribute_t *attr_list);

int32
hsrv_packet_event_send(int32 sock, ctc_msg_t *p_msg);

int32
hsrv_hostif_start();


#endif /* !__HSRV_HOSTINTF_H__ */
