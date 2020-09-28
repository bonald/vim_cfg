#if !defined (__SAI_HOSTINTERFACE_H_)
#define __SAI_HOSTINTERFACE_H_
#include <sys/epoll.h>
#include <linux/socket.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_tun.h>
#include "ctc_const.h"

typedef struct ctc_sai_hostif_info_s
{
    sal_task_t *recv_task;
    int32 epoll_sock;
    struct epoll_event evl;
    char pkt_buff[9600];

    int32 sock;
    int32 seq;
    struct sockaddr_nl snl;
    char name[20];
}ctc_sai_hostif_info_t;

enum ctc_sai_l2pdu_key_index_by_mac_da_e
{
    SAI_L2PDU_KEY_INDEX_ERPS_COMMON         = 0,
    SAI_L2PDU_KEY_INDEX_ERPS_FLUSH          = 1,
    SAI_L2PDU_KEY_INDEX_L2PROTO_PROTOCOL_MAC = 2,
    SAI_L2PDU_KEY_INDEX_L2PROTO_GROUP_MAC    = 3
 };
typedef enum ctc_sai_l2pdu_key_index_by_mac_da_e ctc_sai_l2pdu_key_index_by_mac_da_t;

enum ctc_sai_l2pdu_key_index_by_mac_da_hw_e
{
    SAI_L2PDU_KEY_INDEX_ERPS_HW_ADDR1       = 0,
    SAI_L2PDU_KEY_INDEX_ERPS_HW_ADDR2       = 1,
    SAI_L2PDU_KEY_INDEX_ERPS_HW_ADDR3       = 2,
    SAI_L2PDU_KEY_INDEX_ERPS_HW_ADDR4       = 3
 };
typedef enum ctc_sai_l2pdu_key_index_by_mac_da_hw_e ctc_sai_l2pdu_key_index_by_mac_da_hw_t;

#define CTC_SAI_CPU_TRAFFIC_SET_MAC(mac, byte0, byte1, byte2, byte3, byte4, byte5)         \
do{                                                                                     \
    mac[0] = byte0;                                                                     \
    mac[1] = byte1;                                                                     \
    mac[2] = byte2;                                                                     \
    mac[3] = byte3;                                                                     \
    mac[4] = byte4;                                                                     \
    mac[5] = byte5;                                                                     \
}while(0)

sai_status_t ctc_sai_hostinterface_init();

sai_status_t ctc_sai_hostif_db_dhcp_sys_syn(uint32 enable);

int32
ctc_packet_send_to_kernel(int fd, uint8 *buf, uint32 length);

int32 ctc_fdb_get_gport(uint8 *mac_address, uint32 vlanid, uint32 *gport);

int32 ctc_fdb_get_portid(uint8 *mac_address, uint32 vlanid, sai_object_id_t *portid);

uint32_t sai_trap_action_to_ctc_action(sai_hostif_trap_id_t trapid, sai_packet_action_t action);

uint32_t sai_trap_id_to_ctc_reason_id(sai_hostif_trap_id_t trapid);

#endif

