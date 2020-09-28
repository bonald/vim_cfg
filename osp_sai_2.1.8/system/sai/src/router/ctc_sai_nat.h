#if !defined (__SAI_NAT_H_)
#define __SAI_NAT_H_

#include <sainat.h>

#if 0
#define CTC_SAI_VRRP_MAX        32

typedef enum
{
    SAI_VRRP_VIP_CREATE,
    SAI_VRRP_VIP_REMOVE,
    SAI_VRRP_VIP_GET
}ctc_sai_vrrp_update_type_t;

typedef struct ctc_icmp_message_header_s
{
    uint8   dst_mac[6];
    uint8   src_mac[6];
    uint16  eth_type;
} ctc_icmp_message_header_t;

typedef struct
{
#if _GLB_HOST_IS_LE
    uint8   ihl:4;                  /* header length */
    uint8   version:4;              /* version */
#else
    uint8   version:4;              /* version */
    uint8   ihl:4;                  /* header length */
#endif
    uint8   tos;                    /* type of service */
    uint16  total_length;           /* total length */
    
    uint16  identification;         /* identification */
    uint16  offset;                 /* fragment offset field */
    
    uint8   ttl;                    /* time to live */
    uint8   protocol;               /* protocol */
    uint16  checksum;               /* checksum */
    
    uint32  src_ip_addr;            /* src ip */    
    uint32  dest_ip_addr;           /* dest ip */
} ctc_icmp_ip_header_t;

stbl_vrrp_vip_t*
ctc_vrrp_vip_get_by_key(
    _In_  const sai_vrrp_vip_entry_t* );

#define GLB_DEFAULT_TPID                0x8100
#define ETH_P_IPV4                      0x0800
#define MAC_ADDR_LEN                    6
#define ETH_HEADER_LEN                  14  /* MAC_ADDR_LEN * 2 + 2 */
#define VLAN_TAG_LEN                    4
#define PROTO_ICMP                      1
#endif
sai_status_t ctc_sai_nat_init(void);

#endif

