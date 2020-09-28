
#ifndef __IGMP_PDU_H__
#define __IGMP_PDU_H__

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
    
    uint32  src_ip_addr;
    
    uint32  dest_ip_addr;
    
    uint8   option[3];
    uint8   padding;
} ip_header_t;

typedef struct
{
    uint8   type;
    uint8   max_resp_time;
    uint16  checksum;
    
    uint32  group_address;
} igmp_header_t;

typedef struct
{
    uint8   version_type;
    uint8   reserved_byte;
    uint16  checksum;
} pim_hello_header_t;

typedef struct
{
    uint16   type;
    uint16   length;
    uint16  holdtime;
} pim_hello_option1_t;
typedef struct
{
    uint8   type;
    uint8   reserved;
    uint16  checksum;
    
    uint16  reserved1;
    uint16  number_of_group;
} igmp_v3_header_t;

typedef struct
{
    uint8   record_type;
    uint8   aux_data_len;
    uint16  number_of_sources;
    uint32  multicast_address;
    uint32  *source_address;
} igmp_group_record_t;

typedef struct
{
    igmp_v3_header_t        header;
    igmp_group_record_t     *group_record;
} igmp_v3_pdu_info_t;

typedef struct
{
    uint8   *mac_addr;
    vid_t   vid;
    uint32  querier_max_response_time;
    addr_ipv4_t src_ip_addr;
    addr_ipv4_t dest_ip_addr;
    addr_ipv4_t group_addr;
    uint8               v3_qrv;                /* for V3 */
    uint8               v3_qqic;               /* for V3 */
    uint32              v3_group_number;       /* for V3 */
    igmp_group_record_t v3_group_record[1];    /* for V3 */
} igmp_encode_info_t;

char*
igmp_pdu_type_str(uint8 type);

int32
igmp_pdu_encode_v1_v2(uint8 *p_buf, uint32 len, uint8 type, uint8 version, igmp_encode_info_t *p_info);

int32
igmp_pdu_encode_v3(uint8 *p_buf, uint32 len, uint8 type, igmp_encode_info_t *p_info);

#endif /* __IGMP_PDU_H__ */
