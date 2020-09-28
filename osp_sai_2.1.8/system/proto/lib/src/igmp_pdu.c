
#include "proto.h"
#include "igmp_define.h"
#include "igmp_pdu.h"

char*
igmp_pdu_type_str(uint8 type)
{
    switch (type)
    {
    case IGMP_MSG_MEMBERSHIP_QUERY:
        return "query";

    case IGMP_MSG_V1_MEMBERSHIP_REPORT:
        return "v1 report";

    case IGMP_MSG_V2_MEMBERSHIP_REPORT:
        return "v2 report";
        
    case IGMP_MSG_V3_MEMBERSHIP_REPORT:
        return "v3 report";

    case IGMP_MSG_V2_LEAVE_GROUP:
        return "v2 leave";

    default:
        return "invalid";
    }
}

/*

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      Type     | Max Resp Time |           Checksum            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         Group Address                         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

 */
uint32
igmp_calc_checksum (uint16 *ptr, uint32 length)
{
    uint32 sum = 0;
    uint16 oddbyte = 0;
    uint16 result = 0;
    uint8 *p = NULL;
    uint8 *q = NULL;

    sum = 0;
    while (length > 1)
    {
        sum += *ptr++;
        length -= 2;
    }

    if (length == 1)
    {
        p = (uint8 *)&oddbyte;
        q = (uint8 *)ptr;
        *p = *q;
        sum += oddbyte;
    }

    sum  = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;

    return result;
}

int32
igmp_pdu_encode_v1_v2(uint8 *p_buf, uint32 len, uint8 type, uint8 version, igmp_encode_info_t *p_info)
{
    uint8 macda[MAC_ADDR_LEN];
    uint8 max_resp_time = 0;
    uint16 checksum = 0;
    uint32 group_address = p_info->group_addr.s_addr;
    int32   len_left = len;
    int32   igmp_len = 0;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    uint8 *igmp_start = NULL;
    uint8 *igmp_end = NULL;
    ip_header_t *ip_header = NULL;
    igmp_header_t *igmp_header = NULL;

    if (IGMP_VERSION_2 == version && IGMP_MSG_MEMBERSHIP_QUERY == type && (sal_hton32(INADDR_ALLHOSTS_GROUP) != sal_hton32(group_address)))
    {
        max_resp_time = p_info->querier_max_response_time * IGMP_ONE_SEC_UNIT;
    }
    
    size = &len_left;
    ppnt = &p_buf;

    /* 1. encode L2 header */
    IGMP_IPV4_ADDR_TO_MAC(p_info->dest_ip_addr.s_addr, macda);
    MSG_ENCODE_PUT(macda, MAC_ADDR_LEN);
    MSG_ENCODE_PUT(p_info->mac_addr, MAC_ADDR_LEN);
    if (p_info->vid)
    {
        MSG_ENCODE_PUTW(GLB_ETH_P_8021Q);
        MSG_ENCODE_PUTW(p_info->vid);
    }
    MSG_ENCODE_PUTW(ETH_P_IPV4);
    
    /* 2. bypass IP header */
    ip_header = (ip_header_t*)*ppnt;
    MSG_ENCODE_PUT_EMPTY(sizeof(ip_header_t));

    /* 3. encode IGMP header */
    igmp_start = *ppnt;
    igmp_header = (igmp_header_t*)*ppnt;
    MSG_ENCODE_PUT_EMPTY(sizeof(igmp_header_t));
    igmp_header->type = type;
    igmp_header->max_resp_time = max_resp_time;
    igmp_header->checksum = 0;
    igmp_header->group_address = group_address;

    igmp_end = *ppnt;
    igmp_len = igmp_end - igmp_start;
    checksum = igmp_calc_checksum((uint16*)igmp_header, igmp_len);
    igmp_header->checksum = checksum;

    /* 4. encode IP header */
    ip_header->ihl = 6;
    ip_header->version = 4;
    ip_header->tos = 0xC0;
    ip_header->total_length = sal_hton16(ip_header->ihl * 4 + igmp_len);

    ip_header->identification = 0;
    ip_header->offset = 0;

    ip_header->ttl = 1;
    ip_header->protocol = IPPROTO_IGMP;
    ip_header->checksum = 0;
    
    ip_header->src_ip_addr = (p_info->src_ip_addr.s_addr);
    ip_header->dest_ip_addr = (p_info->dest_ip_addr.s_addr);
    ip_header->option[0] = IGMP_IPOPT_RA;
    ip_header->option[1] = IGMP_RA_SIZE;
    checksum = igmp_calc_checksum((uint16*)ip_header, sizeof(*ip_header));
    ip_header->checksum = checksum;

    return (len - len_left);
}

int32
igmp_pdu_encode_v3(uint8 *p_buf, uint32 len, uint8 type, igmp_encode_info_t *p_info)
{
    uint8 macda[MAC_ADDR_LEN];
    uint32 group_address = p_info->group_addr.s_addr;
    uint8 max_resp_time = 0;
    uint16 checksum = 0;
    int32   len_left = len;
    int32   igmp_len = 0;
    int32   i = 0;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    uint8 *igmp_start = NULL;
    uint8 *igmp_end = NULL;
    ip_header_t *ip_header = NULL;
    igmp_header_t *igmp_header = NULL;
    igmp_v3_header_t *igmp_v3_header = NULL;

    if (IGMP_MSG_MEMBERSHIP_QUERY == type && (sal_hton32(INADDR_ALLHOSTS_GROUP) != sal_hton32(group_address)))
    {
        max_resp_time = p_info->querier_max_response_time * IGMP_ONE_SEC_UNIT;
    }
    
    size = &len_left;
    ppnt = &p_buf;

    /* 1. encode L2 header */
    IGMP_IPV4_ADDR_TO_MAC(p_info->dest_ip_addr.s_addr, macda);
    MSG_ENCODE_PUT(macda, MAC_ADDR_LEN);
    MSG_ENCODE_PUT(p_info->mac_addr, MAC_ADDR_LEN);
    if (p_info->vid)
    {
        MSG_ENCODE_PUTW(GLB_ETH_P_8021Q);
        MSG_ENCODE_PUTW(p_info->vid);
    }
    MSG_ENCODE_PUTW(ETH_P_IPV4);
    
    /* 2. bypass IP header */
    ip_header = (ip_header_t*)*ppnt;
    MSG_ENCODE_PUT_EMPTY(sizeof(ip_header_t));

    /* 3. encode IGMP header */
    igmp_start = *ppnt;

    if (IGMP_MSG_MEMBERSHIP_QUERY == type)
    {
        igmp_header = (igmp_header_t*)*ppnt;
        MSG_ENCODE_PUT_EMPTY(sizeof(igmp_header_t));
        igmp_header->type = type;
        igmp_header->max_resp_time = max_resp_time;
        igmp_header->checksum = 0;
        igmp_header->group_address = group_address;
        MSG_ENCODE_PUTC(p_info->v3_qrv);
        MSG_ENCODE_PUTC(p_info->v3_qqic);
        MSG_ENCODE_PUTW(p_info->v3_group_number);       // number_of_group
    }
    else
    {
        igmp_v3_header = (igmp_v3_header_t*)*ppnt;
        MSG_ENCODE_PUTC(type);                          // type
        MSG_ENCODE_PUTC(0);                             // reserved
        MSG_ENCODE_PUTW(0);                             // checksum
        MSG_ENCODE_PUTW(0);                             // reserved1
        MSG_ENCODE_PUTW(p_info->v3_group_number);       // number_of_group
    }

    for (i = 0; i < p_info->v3_group_number; i++)
    {
        MSG_ENCODE_PUTC(p_info->v3_group_record[i].record_type);
        MSG_ENCODE_PUTC(p_info->v3_group_record[i].aux_data_len);
        MSG_ENCODE_PUTW(p_info->v3_group_record[i].number_of_sources);
        MSG_ENCODE_PUTL(sal_ntohl(p_info->v3_group_record[i].multicast_address));
    }

    igmp_end = *ppnt;
    igmp_len = igmp_end - igmp_start;
    if (IGMP_MSG_MEMBERSHIP_QUERY == type)
    {
        checksum = igmp_calc_checksum((uint16*)igmp_header, igmp_len);
        igmp_header->checksum = checksum;
    }
    else
    {
        checksum = igmp_calc_checksum((uint16*)igmp_v3_header, igmp_len);
        igmp_v3_header->checksum = checksum;
    }

    /* 4. encode IP header */
    ip_header->ihl = 6;
    ip_header->version = 4;
    ip_header->tos = 0xC0;
    ip_header->total_length = sal_hton16(ip_header->ihl * 4 + igmp_len);

    ip_header->identification = 0;
    ip_header->offset = 0;

    ip_header->ttl = 1;
    ip_header->protocol = IPPROTO_IGMP;
    ip_header->checksum = 0;
    
    ip_header->src_ip_addr = (p_info->src_ip_addr.s_addr);
    ip_header->dest_ip_addr = (p_info->dest_ip_addr.s_addr);
    ip_header->option[0] = IGMP_IPOPT_RA;
    ip_header->option[1] = IGMP_RA_SIZE;
    checksum = igmp_calc_checksum((uint16*)ip_header, sizeof(*ip_header));
    ip_header->checksum = checksum;

    return (len - len_left);
}
