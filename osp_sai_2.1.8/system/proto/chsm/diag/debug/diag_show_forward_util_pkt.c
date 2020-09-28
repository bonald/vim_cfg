/*****************************************************************************
 * ctcutil_pkt.c    packet generator
 *
 * Copyright (C) 2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:    R0.01
 * Author:      Hai Sun
 * Date:        2005-11-23
 * Reason:      Initial version
 *****************************************************************************/

#include "sal.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>

#include "diag_show_forward_util_rand.h"
#include "diag_show_forward_util_pkt.h"
#include "chsm_error.h"

#define CRCPOLY_LE                      0xedb88320
#define CRCPOLY_BE                      0x04c11db7

#define LINE_SIZE                       128

extern uint8 calculate_crc8(uint8 *data, int32 len, uint8 init_crc);

#define BITS_CHECK_ERROR(value, bit_num)  \
        if (!((32 == (bit_num)) || (0 == ((value) & (~((1<<(bit_num)) - 1)))))) \
        { \
            printf("%x is beyond %d bits range limit\n", (int32)(value), (int8)(bit_num)); \
            return -1; \
        }

/****************************************************************************
 * Name:       _diag_show_forward_icmp6_cksum
 * Purpose:    compute the icmp6 checksum
 * Input:     ipv6_addr_src
 *              ipv6_addr_dst
 *              icp data point
 *             len   -- data length.
 * Output:     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_icmp6_cksum(const uint32 *ip6_src, const uint32 *ip6_dst, uint8 *icp, uint32 len)
{
    int32 i = 0;
    register const uint16 *sp = NULL;
    uint32 sum = 0;
    uint32 data32 = 0;
    union {
        struct
        {
            uint32 ph_src[4];
            uint32 ph_dst[4];
            uint32 ph_len;
            uint8 ph_zero[3];
            uint8 ph_nxt;
        } ph;
        uint16 pa[20];
    } phu;

    /* pseudo-header */
    sal_memset(&phu, 0, sizeof(phu));
    data32 = sal_htonl(ip6_src[0]);
    sal_memcpy(phu.ph.ph_src, &data32, 4);

    data32 = sal_htonl(ip6_src[1]);
    sal_memcpy(phu.ph.ph_src + 1, &data32, 4);

    data32 = sal_htonl(ip6_src[2]);
    sal_memcpy(phu.ph.ph_src + 2, &data32, 4);

    data32 = sal_htonl(ip6_src[3]);
    sal_memcpy(phu.ph.ph_src + 3, &data32, 4);

    data32 = sal_htonl(ip6_dst[0]);
    sal_memcpy(phu.ph.ph_dst, &data32, 4);

    data32 = sal_htonl(ip6_dst[1]);
    sal_memcpy(phu.ph.ph_dst + 1, &data32, 4);

    data32 = sal_htonl(ip6_dst[2]);
    sal_memcpy(phu.ph.ph_dst + 2, &data32, 4);

    data32 = sal_htonl(ip6_dst[3]);
    sal_memcpy(phu.ph.ph_dst + 3, &data32, 4);
    phu.ph.ph_len = sal_htonl(len);
    phu.ph.ph_nxt = IPPROTO_ICMPV6;

    sum = 0;
    for (i = 0; i < sizeof(phu.pa) / sizeof(phu.pa[0]); i++)
    {
        sum += phu.pa[i];
    }

    sp = (const uint16 *)icp;

    for (i = 0; i < (len & ~1); i += 2)
    {
        sum += *sp++;
    }

    if (len & 1)
    {
        sum += sal_htons((*(const uint8 *)sp) << 8);
    }

    while (sum > 0xffff)
    sum = (sum & 0xffff) + (sum >> 16);
    sum = ~sum & 0xffff;

    return (sum);
}

/****************************************************************************
 * Name:       _diag_show_forward_swap32
 * Purpose:    swap uint32 data.
 * Input:      data  -- pointer to uint32 data
 *             len   -- data length.
 *             direction -- swap direction.
 * Output:     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_swap32(uint32 *data, int32 len, uint8 direction)
{
    int32 cnt = 0;

    for (cnt = 0; cnt < len; cnt ++)
    {
        if (DIAG_HOST_TO_NETWORK == direction)
        {
            data[cnt] = sal_htonl(data[cnt]);
        }
        else
        {
            data[cnt] = sal_ntohl(data[cnt]);
        }
    }

    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name:       _diag_show_forward_crc32_le
 * Purpose:    calculate 32 bit crc le.
 * Input:      data  -- pointer to uint32 data
 *              len   -- data length.
 * Output:     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/

static inline int32
_diag_show_forward_crc32_le(uint32 seed,  uint8 *data, uint32 data_len, uint32 *p_crc)
{
    uint32 crc = 0;
    int32 i = 0;

    crc = seed;

    while (data_len)
    {
        crc ^= *data;
        for (i = 0; i < 8; i++)
        {
            crc = (crc >> 1) ^ ((crc & 1) ? CRCPOLY_LE : 0);
        }
        data++;
        data_len--;
    }

    *p_crc = ~crc;

    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name     :   _diag_show_forward_crc32_be
 * Purpose  :   calculate 32 bit crc be.
 * Input    :      data  -- pointer to uint32 data
 *                  len   -- data length.
 *                  direction -- swap direction.
 * Output   :     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static inline int32
_diag_show_forward_crc32_be(uint32 seed,  uint8 *data, uint32 data_len, uint32 *p_crc)
{
    uint32 crc = 0;
    int32 i = 0;

    crc = seed;

    while (data_len)
    {
        crc ^= (*data << 24);
        for (i = 0; i < 8; i++)
        {
            crc = (crc << 1) ^ ((crc & 0x80000000) ? CRCPOLY_BE : 0);
        }
        data++;
        data_len--;
    }

    *p_crc = ~crc;

     return CHSM_E_SUCCESS;
}


/*****************************************************************************
 * Name         : _diag_show_forward_ctcutil_crc32
 * Purpose      : calculate crc32
 * Input        : seed: initial crc seed
 *                data: data buffer to be calculated
 *                data_len: data length
 * Output       : crc: calculated crc
 * Return   : CHSM_E_XX
 * Note     : N/A
 *****************************************************************************/
static int32
_diag_show_forward_ctcutil_crc32(uint32 seed,  uint8 *data, uint32 data_len, uint32 *p_crc)
{
    int32 ret = 0;

    if (!data || data_len == 0 || !p_crc)
    {
        return CHSM_E_INVALID_PARAM;
    }
    ret = _diag_show_forward_crc32_le(seed, data, data_len, p_crc);

    return ret;
}

/****************************************************************************
 * Name    : diag_show_forward_ip_chksum
 * Purpose :calculate ip checksum.
 * Parameters:
 * Input    : p points to ip header
 *              length is the header length
 *              init is the initial seed value for calculation
 * Output:     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_ip_chksum (uint16 *p, uint16 length, uint16 init)
{
    uint32 sum = init;
    uint16 len = length >> 1;

    while (len-- > 0)
    {
        sum += sal_ntoh16(*p++);
    }

    if (length & 1)
    {
        sum += (*p & 0xFF00);
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    len = sum & 0xFFFF;
    return(~len);
}
/****************************************************************************
 * Name    : _diag_show_forward_igmp_checksum
 * Purpose : calculate igmp checksum.
 * Parameters:
 * Input    : p points to ip header
 *              length is the header length
 *              init is the initial seed value for calculation
 * Output  : N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_igmp_checksum (uint16 *ptr, uint16 nbytes)
{
    uint32 sum = 0;
    uint16 oddbyte = 0;
    uint16 result = 0;
    uint8 *p = NULL;
    uint8 *q = NULL;

    sum = 0;
    while (nbytes > 1)
    {
        sum += *ptr++;
        nbytes -= 2;
    }

    if (nbytes == 1)
    {
        p = (uint8 *)&oddbyte;
        q = (uint8 *)ptr;
        *p = *q;
        sum += oddbyte;
    }

    sum  = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    result = ~sum;

    return result;
}

/****************************************************************************
 * Name    : diag_show_forward_tcp_chksum
 * Purpose : calculate tcp checksum.
 * Input    : is_ipv6 indicates whether it's a ipv6 packet
 *              p_l3 points to layer 3 header
 *              p_l4 points to layer 4 header
 *              length is the header length
 *              init is the initial seed value for calculation
 * Output : N/A.
 * Return  : CHSM_E_XX
 * Note    : N/A
 ****************************************************************************/
static int32
_diag_show_forward_tcp_chksum(diag_config_t *config, int32 is_ipv6, uint16 *p_l3, 
                                                                        uint16 *p_l4, uint16 length, uint16 init)
{
    uint32 sum = 0;
    int32 i = 0;
    uint16 len =0;

    sum = init;
    len = length >> 1;
    
    if (is_ipv6)
    {
        for (i = 0; i < 16; i++)
        {
            sum += sal_ntoh16(*p_l3++);
        }
        sum += length;
        sum += config->ipv6.ipv6_nxt_hdr;
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
            sum += sal_ntoh16(p_l3[6 + i]);
        }
        sum += config->ipv4.ipv4_proto;
        sum += length;
    }

    while (len-- > 0)
    {
        sum += sal_ntoh16(*p_l4++);
    }

    if (length & 1)
    {
        sum += (*p_l4 & 0xFF00);
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    len = sum & 0xFFFF;

    return (~len);
}
/****************************************************************************
 * Name     :  _diag_show_forward_generate_random_pkt
 * Purpose  :generate fixed length packet randomly
 * Input     : *pkt -- the random pkt will be added there
 *               length -- payload length
 * Output  :     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_random_pkt (uint8 **pkt, uint32 length)
{
    uint32 random = 0;
    uint32 i = 0;
    uint32 min = 0;
    uint32 max = 255;

    for (i = 0; i < length; i ++)
    {
        diag_show_forward_ctcutil_rand(min, max, (uint32 *)&random);
        **pkt = (uint8)random;
        (*pkt) ++;
    }

    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name     : strtoul
 * Purpose  : The function implement converting a string to long integer.
 * Inputs    : the source string to be converted;
 *                endptr : if endptr is not null, these functions set
 *                *endptr (the "next character pointer") to
 *                point to the character that stopped the
 *                scan (*endptr = &stopper).
 *                endptr is useful for error detection.
 *                radix : determine the base of the value being converted.
 * Output   :
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static __inline__ int32
_diag_show_forward_pkt_str2ul(uint8* s_start, uint8* s_end)
{
    char *str_temp = NULL;
    bool hex_bool = FALSE;
    bool hex_temp1 = FALSE;
    bool hex_temp2 = FALSE;
    bool hex_temp3 = FALSE;
    int32 i = 0;
    int32 k = 1;
    uint32 sum = 0;
    bool str_bool = FALSE;

    if (!s_start)
    {
        return CHSM_E_SUCCESS;
    }
    
    str_temp = (char *)s_start;
    if (!s_end)
    {
        s_end = s_start + sal_strlen((char *)s_start);
    }
    if ('0' == *str_temp && 'x' == *(str_temp+1))
    {
        str_temp += 2;
        while (str_temp <= (char *)s_end && '\0' != *str_temp)
        {
            hex_temp1 = *str_temp >= 'a' && *str_temp <= 'f';
            hex_temp2 = *str_temp >= 'A' && *str_temp <= 'F';
            hex_temp3 = *str_temp >= '0' && *str_temp <= '9';
            hex_bool = hex_temp1 || hex_temp2 || hex_temp3;

            if (!hex_bool)
            {
                return 0xFFFFFFFF;
            }
            ++str_temp;
        }
        --str_temp;

        while (str_temp >= (char *)s_start && 'x'!= *str_temp)
        {
            hex_temp1 = *str_temp >= 'a' && *str_temp <= 'f';
            hex_temp2 = *str_temp >= 'A' && *str_temp <= 'F';
            hex_temp3 = *str_temp >= '0' && *str_temp <= '9';

            if (hex_temp1)
            {
                i = (int32)(*str_temp - 'a') + 10;
            }
            if (hex_temp2)
            {
                i = (int32)(*str_temp - 'A') + 10;
            }
            if (hex_temp3)
            {
                i = (int32)(*str_temp - '0');
            }

            sum = sum + i * k;
            k *= 16;
            --str_temp;
     }
    return sum;
    }

    while (str_temp <= (char *)s_end && '\0' != *str_temp)
    {
        str_bool = *str_temp >= '0' && *str_temp <= '9';
        if (str_bool)
        {
            str_temp++;
        }
        else
        {
            return 0xFFFFFFFF;
        }
    }
    --str_temp;
    while (str_temp >= (char *)s_start)
    {
        i = (int32)(*str_temp - '0');
        sum = sum + i * k;
        k *= 10;
        --str_temp;
    }

    return sum;
}

/****************************************************************************
 * Name     : _diag_show_forward_ipv4addr_to_uint
 * Purpose  : gets the ipv4 address value from argument vector
 * Input     : ipv4 address
 * Output   : unsigned int
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static __inline__ int32
_diag_show_forward_ipv4addr_to_uint(uint8* address)
{
    uint8* str = NULL;
    uint8 addr_tmp[4][6];
    uint16 i = 0;
    uint16 j = 0;
    uint32 value = 0;
    uint32 value_tmp = 0;

    if (!address)
    {
        return CHSM_E_SUCCESS;
    }

    for (i = 0; i < 4; i++)
    {
        sal_memset(addr_tmp[i], 0, 6);
    }

    str = address;
    i = 0;
    /*save the address to the string array addr_tmp*/
    while (0 != *str && '/' != *str)
    {
        if ('.' != *str)
        {
            j = 0;
            while (0 != *str && '/' != *str && '.' != *str)
            {
                if (i < 4 && j < 6)
                {
                    addr_tmp[i][j++] = *str++;
                }
                else
                {
                    ++str;
                }
            }

            ++i;
        }

        if (0 == *str || '/' == *str)
        {
            break;
        }

        ++str;
    }

    /*convert the addr_tmp string to unsigned long data type*/
    for (i = 0; i < 4; i++)
    {
        value_tmp = _diag_show_forward_pkt_str2ul(addr_tmp[i], NULL);
        value = value | (value_tmp << (3-i)*8);
    }

    return value;
}


/******************************************************************************
 * Name   : _diag_show_forward_memcpy_data
 * Purpose: copy data to memory location according to edian
 * Input  : p_pkt: address of memory pointer
 *            data:  points to data to be copied
 *            size:  data size
 * Output : p_pkt: memory pointer gets updated based on size
 * Return   : CHSM_E_XX
 * Note     : N/A
 *****************************************************************************/
static int32
_diag_show_forward_memcpy_data(uint8 **p_pkt, void *data, int32 size)
{
    void *pkt = *p_pkt;

#ifdef _GLB_UML_SYSTEM_
    if ( 2 == size)
    {
        *(uint16*)data = SHOW_FORWARD_SWAP16(*(uint16*)data);
    }
    else if ( 3 == size)
    {
        *(uint32*)data = SHOW_FORWARD_SWAP32(*(uint32*)data) >> 8;
    }
    else if ( 4 == size)
    {
         *(uint32*)data = SHOW_FORWARD_SWAP32(*(uint32*)data);
    }
    sal_memcpy(pkt, data, size);
    pkt += size;

#else
    if ( 3 == size)
    {
        *(uint32*)data >>= 8;
    }
    sal_memcpy(pkt, data, size);
    pkt += size;

#endif
    
    *p_pkt = pkt;

    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name     : _diag_show_forward_generate_bay_header
 * Purpose  :    
 * Input:      bay_hdr : bay header
 *               bay_hdr_config: bay header config
 * Output   : N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_bay_header(diag_bay_packet_header_t *bay_hdr, 
                                                                                        diag_bay_hdr_config_t *bay_hdr_config)
{
    bay_hdr->hdr_type = bay_hdr_config->hdr_type;
    bay_hdr->dest_id_discard = bay_hdr_config->dest_id_discard;
    bay_hdr->pkt_offset = bay_hdr_config->pkt_offset;
    bay_hdr->multi_cast = bay_hdr_config->multi_cast;
    bay_hdr->dest_chip_id = bay_hdr_config->dest_chip_id;
    bay_hdr->dest_id = bay_hdr_config->dest_id;
    bay_hdr->priority = bay_hdr_config->priority;
    bay_hdr->pkt_type = bay_hdr_config->pkt_type;
    bay_hdr->src_cos = bay_hdr_config->src_cos;
    bay_hdr->header_hash = bay_hdr_config->header_hash;
    bay_hdr->src_port = bay_hdr_config->src_port;
    bay_hdr->src_vlan_id = bay_hdr_config->src_vlan_id;
    bay_hdr->color = bay_hdr_config->color;
    bay_hdr->nxt_hop_ptr = bay_hdr_config->nxt_hop_ptr;
    bay_hdr->len_adj_type = bay_hdr_config->len_adj_type;
    bay_hdr->critical_pkt = bay_hdr_config->critical_pkt;
    bay_hdr->pkt_len = bay_hdr_config->pkt_len;
    bay_hdr->ttl = bay_hdr_config->ttl;
    bay_hdr->hdr_crc = bay_hdr_config->hdr_crc;
    bay_hdr->src_queue_select = bay_hdr_config->src_que_select;
    bay_hdr->vpls_port_type = bay_hdr_config->vpls_port_type;
    bay_hdr->untagged_pkt = bay_hdr_config->untagged_pkt;
    if(TRUE != bay_hdr_config->hdr_crc_config)
    {
        bay_hdr->hdr_crc = calculate_crc8((uint8 *)bay_hdr, sizeof(bay_hdr), bay_hdr->hdr_crc);
    }
    return CHSM_E_SUCCESS;

}

/****************************************************************************
 * Name     : _diag_show_forward_generate_exception_header
 * Purpose  : generate exception header   
 * Input:      ecptn:bay exception
 *               ecptn_config:bay exception config.
 * Output   : N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_exception_header(diag_humber_exception_info_t *ecptn, 
                                                                                                diag_humber_exception_config_t *ecptn_config)
{
    ecptn->exceptions_vect_11_0 = ecptn_config->exceptions_vect_11_0;
    ecptn->exp_vect_12= ecptn_config->exp_vect_12;
    ecptn->exp_vect_13= ecptn_config->exp_vect_13;
    ecptn->exp_vect_14= ecptn_config->exp_vect_14;
    ecptn->exp_vect_15= ecptn_config->exp_vect_15;
    
    ecptn->qos_log_id = ecptn_config->qos_log_id;
    ecptn->acl_log_id = ecptn_config->acl_log_id;
    ecptn->l3_span_id = ecptn_config->l3_span_id;
    ecptn->l2_span_id = ecptn_config->l2_span_id;
    ecptn->exception_pkt_type = ecptn_config->exception_pkt_type;
    ecptn->egress_exception = ecptn_config->egress_exception;
    ecptn->mac_valid = ecptn_config->mac_valid;
    ecptn->loopback_en= ecptn_config->loopback_en;
    ecptn->exception_sub_idx= ecptn_config->exception_sub_idx;

    return CHSM_E_SUCCESS;
}
/****************************************************************************
 * Name     : _diag_show_forward_generate_l2_header
 * Purpose  : generate l2 header
 * Input:      pkt: pointer of pkt. ctrl_var:struct ctl var
 *               l2_config: ethernet ctr struct. pkt_start_ptr:point of start 
 * Output   :     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_l2_header(uint8**pkt,diag_ethernet_config_t *l2_config, 
                                                                                diag_ethernet_ctr_t *ctrl_var, uint8 *pkt_start_ptr)
{
    uint16  data16 = 0;
    uint32  data32 = 0;

    /* mac da and sa */
    if (L2_TYPE_PPP != l2_config->l2_type)
    {
        sal_memcpy(*pkt, l2_config->mac_da, 6);
        ctrl_var->mac_da_offset = *pkt - pkt_start_ptr + 2;
        (*pkt) += 6;
        sal_memcpy(*pkt, l2_config->mac_sa, 6);
        ctrl_var->mac_sa_offset = *pkt - pkt_start_ptr + 2 ;
        (*pkt) += 6;
    }
    /* vlan */
    if ( 1 == l2_config->num_vlan )
    {
        data16 =  l2_config->vlan_tag1;
        _diag_show_forward_memcpy_data(pkt, &data16, 2);
        data16 = (((uint16) l2_config->cos1) << 13 | ((uint16) l2_config->cfi1 << 12)
              |  l2_config->vid1);
        ctrl_var->vid_offset = *pkt - pkt_start_ptr;
        _diag_show_forward_memcpy_data(pkt, &data16, 2);
    }
    else if ( 2 == l2_config->num_vlan )
    {
        data16 =  l2_config->vlan_tag1;
        _diag_show_forward_memcpy_data(pkt, &data16, 2);
        data16 = (((uint16) l2_config->cos1) << 13 | ((uint16) l2_config->cfi1 << 12)
              |  l2_config->vid1);
        _diag_show_forward_memcpy_data(pkt, &data16, 2);

        data16 = l2_config->vlan_tag2;
        _diag_show_forward_memcpy_data(pkt, &data16, 2);
        data16 = (((uint16) l2_config->cos2) << 13 | ((uint16) l2_config->cfi2 << 12)
              |  l2_config->vid2);
        _diag_show_forward_memcpy_data(pkt, &data16, 2);
    }
    /* l2 type specific */
    switch ( l2_config->l2_type)
    {
        case L2_TYPE_ETH_V2:
            data16 =  l2_config->type;
            ctrl_var->type_offset = *pkt - pkt_start_ptr;
            _diag_show_forward_memcpy_data(pkt, &data16, 2);
            break;

        case L2_TYPE_ETH_SAP:
            data16 =  l2_config->len;
            _diag_show_forward_memcpy_data(pkt, &data16, 2);

            *(uint8*)*pkt =  l2_config->dsap;
            (*pkt)++;
            *(uint8*)*pkt =  l2_config->ssap;
            (*pkt)++;
            *(uint8*)*pkt =  l2_config->ctl0;
            (*pkt)++;
            break;

        case L2_TYPE_ETH_SAP2:
            data16 =  l2_config->len;
            _diag_show_forward_memcpy_data(pkt, &data16, 2);

            *(uint8*)*pkt =  l2_config->dsap;
            (*pkt)++;
            *(uint8*)*pkt =  l2_config->ssap;
            (*pkt)++;
            *(uint8*)*pkt =  l2_config->ctl0;
            (*pkt)++;
            (*pkt)++;
            break;

        case L2_TYPE_ETH_SNAP:
            data16 =  l2_config->len;
            _diag_show_forward_memcpy_data(pkt, &data16, 2);

            *(uint8*)*pkt =  l2_config->dsap;
            (*pkt)++;
            *(uint8*)*pkt =  l2_config->ssap;
            (*pkt)++;
            *(uint8*)*pkt =  l2_config->ctl0;
            (*pkt)++;

            data32 =  l2_config->oui;
            _diag_show_forward_memcpy_data(pkt, &data32, 3);

            data16 =  l2_config->type;
            _diag_show_forward_memcpy_data(pkt, &data16, 2);

            break;

        case L2_TYPE_RAW_SNAP:
        /* todo */
        break;

        case L2_TYPE_PPP:
        /* todo */
        break;
    }
    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name     : _diag_show_forward_generate_l3_ipv4
 * Purpose  : generate l3 ipv4  
 * Input:      pkt: pointer of pkt.           l3_config:ipvt config
 *               p_total_len: total len.        ctrl_var: ctrl var struct
 *               p_ip_chksum: check sum.   pkt_start_ptr:start pointer of pkt
 * Output   :     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_l3_ipv4(uint8 **pkt,uint8 **p_total_len,uint8 **p_ip_chksum,
                                                                            diag_ipv4_config_t *l3_config,diag_ipv4_ctr_t *ctrl_var, uint8 *pkt_start_ptr)
{
    uint16  data16 = 0;
    uint32  data32 = 0;
    uint8    data8 = 0;

    if( 0 == l3_config->ipv4_ihl)
    {
        **pkt= 0x45;            /* version and header length */
    }/* todo: ip option */
    else
    {
        **pkt= (0x4<<4)|(l3_config->ipv4_ihl);
    }
    (*pkt)++;
    data8 = l3_config->ipv4_type_serv;/* todo: type of service */
    **pkt = data8;
    (*pkt)++;

    *p_total_len = *pkt;
    (*pkt) += 2;                /*2 byte will fill later*/

    data16 = l3_config->ipv4_id;
    ctrl_var->ip_id_offset = *pkt - pkt_start_ptr;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);
    data16 = ((l3_config->ipv4_flags << 13) | l3_config->ipv4_frag_offset);
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

    **pkt = l3_config->ipv4_ttl;
    (*pkt)++;
    **pkt = l3_config->ipv4_proto;
    (*pkt)++;

    *p_ip_chksum = *pkt;
    (*pkt) += 2;         /*2 byte will fill later*/

    data32 = l3_config->ipv4_sa;
    ctrl_var->ipsa_offset = *pkt - pkt_start_ptr;
    _diag_show_forward_memcpy_data(pkt, &data32, 4);
    data32 = l3_config->ipv4_da;
    ctrl_var->ipda_offset = *pkt - pkt_start_ptr;
    _diag_show_forward_memcpy_data(pkt, &data32, 4);  /* todo: ip options */

    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name     : _diag_show_forward_generate_l3_ipv6
 * Purpose  : generate l3 ipv6   
 * Input:      pkt: pointer of pkt.           l3_config:ipvt config
 *               p_total_len: total len.        ctrl_var: ctrl var struct
 *               p_ip_chksum: check sum.   pkt_start_ptr:start pointer of pkt
 * Output   : N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_l3_ipv6(uint8 **pkt, uint8 **p_pld_len,diag_ipv6_config_t *l3_config,
                                                                                diag_ipv6_ctr_t *ctrl_var, uint8 *pkt_start_ptr)
{
    uint16  data16 = 0;
    uint32  data32 = 0;

    **pkt = 0x60 | l3_config->ipv6_traf_class >> 4;
    (*pkt)++;
    **pkt = (l3_config->ipv6_traf_class << 4) | ((l3_config->ipv6_flow_label >> 16) & 0xf);
    (*pkt) ++;
    data16 = l3_config->ipv6_flow_label & 0xffff;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);
    *p_pld_len = *pkt;
    (*pkt) += 2;

    **pkt = l3_config->ipv6_nxt_hdr;
    (*pkt)++;

    **pkt = l3_config->ipv6_hp_limit;
    (*pkt)++;

    ctrl_var->ipsa_offset = *pkt - pkt_start_ptr;

    data32 = l3_config->ipv6_sa[0];
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    data32 = l3_config->ipv6_sa[1];
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    data32 = l3_config->ipv6_sa[2];
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    data32 = l3_config->ipv6_sa[3];
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    ctrl_var->ipda_offset = *pkt - pkt_start_ptr;

    data32 = l3_config->ipv6_da[0];
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    data32 = l3_config->ipv6_da[1];
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    data32 = l3_config->ipv6_da[2];
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    data32 = l3_config->ipv6_da[3];
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    return CHSM_E_SUCCESS;
}
/****************************************************************************
 * Name     : _diag_show_forward_generate_l4_tcp
 * Purpose  : generate l4 tcp   
 * Input:      pkt: pointer of pkt.           l3_config:ipvt config
 *               p_total_len: total len.        ctrl_var: ctrl var struct
 *               p_ip_chksum: check sum.   pkt_start_ptr:start pointer of pkt
 * Output   :     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_l4_tcp(uint8 **pkt, uint8 **p_tcp_chksum,diag_tcp_config_t *l4_config,
                                                                                diag_tcp_ctr_t *ctrl_var, uint8 *pkt_start_ptr)
{
    uint16  data16 = 0;
    uint32  data32 = 0;

    data16 = l4_config->sport;
    ctrl_var->sport_offset = *pkt - pkt_start_ptr;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

    data16 = l4_config->dport;
    ctrl_var->dport_offset = *pkt - pkt_start_ptr;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

    data32 = l4_config->seq;
    ctrl_var->seq_offset = *pkt - pkt_start_ptr;
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    data32 = l4_config->ack_seq;
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    data16 = l4_config->data_offset << 12 | l4_config->flag;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

    data16 = l4_config->window;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

    *p_tcp_chksum = *pkt;
    ctrl_var->check_sum_offset = *pkt - pkt_start_ptr;
    (*pkt) += 2;

    data16 = l4_config->urg_ptr;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name     : _diag_show_forward_generate_l4_udp
 * Purpose  : generate l4 udp   
 * Input:      pkt: pointer of pkt.           l3_config:ipvt config
 *               p_total_len: total len.        ctrl_var: ctrl var struct
 *               p_ip_chksum: check sum.   pkt_start_ptr:start pointer of pkt
 * Output   :     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_l4_udp(uint8 **pkt, uint8 **p_udp_len,diag_udp_config_t *l4_config,
                                                                            diag_udp_ctr_t *ctrl_var, uint8 *pkt_start_ptr)
{
    uint16  data16 = 0;

    data16 = l4_config->sport;
    ctrl_var->sport_offset = *pkt -pkt_start_ptr;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

    data16 = l4_config->dport;
    ctrl_var->dport_offset = *pkt - pkt_start_ptr;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

    *p_udp_len = *pkt;
    (*pkt) += 2;

    if(l4_config->chksum)
    {
        data16 = l4_config->chksum;
        _diag_show_forward_memcpy_data(pkt, &data16, 2);
    }
    else
    {
        (*pkt) += 2;
    }
    if(l4_config->mip_type)
    {
        **pkt = 4;
        (*pkt)++;
        **pkt = l4_config->mip_type;
        *pkt += 3;
    }

    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name     : _diag_show_forward_generate_l3_mpls
 * Purpose  : generate l3 mpls   
 * Input:      pkt: pointer of pkt.           l3_config:ipvt config
 *               p_total_len: total len.        ctrl_var: ctrl var struct
 *               p_ip_chksum: check sum.   pkt_start_ptr:start pointer of pkt
 * Output   :     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_l3_mpls(uint8 **pkt, diag_mpls_ctr_t *ctrl_var, 
                                                                                diag_mpls_config_t *mpls_config,  uint8 *pkt_start_ptr)
{
    uint32  data32 = 0;
    int32 i = 0;

    if (0 != mpls_config->mpls_label_num)
    {
        for(i = 0; i < mpls_config->mpls_label_num; i++)
        {
            BITS_CHECK_ERROR(mpls_config->mpls[i].mpls_label, 20);
            BITS_CHECK_ERROR(mpls_config->mpls[i].mpls_exp, 3);
            BITS_CHECK_ERROR(mpls_config->mpls[i].mpls_s, 1);
            BITS_CHECK_ERROR(mpls_config->mpls[i].mpls_ttl, 8);
        }

        for(i = mpls_config->mpls_label_num - 1; i>=0; i--)
        {
            data32 = ((mpls_config->mpls[i].mpls_label<< 12) | (mpls_config->mpls[i].mpls_exp << 9)
                      | (mpls_config->mpls[i].mpls_s << 8) | (mpls_config->mpls[i].mpls_ttl));
            ctrl_var[i].label_offset = *pkt - pkt_start_ptr;
            _diag_show_forward_memcpy_data(pkt, &data32, 4);
        }
    }

    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name     : _diag_show_forward_generate_pad
 * Purpose  : generate pad
 * Input:      pkt: pointer of pkt
 *               pad: pad config struct.
 * Output   : N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_pad(uint8 **pkt, diag_pad_config_t *pad)
{
    int32 i = 0;

    for(i = 0; i < pad->pad_length; i++)
    {
        **pkt = 0;
        (*pkt)++;
    }

    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name     : _diag_show_forward_generate_l3_arp
 * Purpose  : generate l3 arp   
 * Input:      pkt: pointer of pkt
 *               l3_config: lay 3 cofnig struct.
 * Output   :     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_l3_arp(uint8 **pkt, diag_arp_config_t *l3_config)
{
    uint32   data32 = 0;

    BITS_CHECK_ERROR(l3_config->arp_hdw_type, 16);
    BITS_CHECK_ERROR(l3_config->arp_protocol_type, 16);
    BITS_CHECK_ERROR(l3_config->arp_hdw_addr_len, 8);
    BITS_CHECK_ERROR(l3_config->arp_protocol_addr_len, 8);
    BITS_CHECK_ERROR(l3_config->arp_operation_code, 16);

    data32 = (l3_config->arp_hdw_type << 16) | (l3_config->arp_protocol_type);
    _diag_show_forward_memcpy_data(pkt, &data32, 4);
    data32 = ((l3_config->arp_hdw_addr_len << 24) | (l3_config->arp_protocol_addr_len << 16)
              | (l3_config->arp_operation_code));
    _diag_show_forward_memcpy_data(pkt, &data32, 4);
    sal_memcpy(*pkt, l3_config->arp_sender_hdw_add, 6);
    (*pkt) += 6;
    data32 = l3_config->arp_sender_protocol_add;
    _diag_show_forward_memcpy_data(pkt, &data32, 4);
    sal_memcpy(*pkt, l3_config->arp_dst_hdw_add, 6);
    (*pkt) += 6;
    data32 = l3_config->arp_dst_protocol_add;
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name     : _diag_show_forward_generate_l3_gre
 * Purpose  : generate l3 gre   
 * Input:      pkt: pointer of pkt
 *               diag_gre_config_t: gre config struct.
 * Output   :     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_l3_gre(uint8 **pkt, diag_gre_config_t *gre_config)
{
    uint16  data16 = 0;
    uint32   data32 = 0;

    BITS_CHECK_ERROR(gre_config->gre_c, 1);
    BITS_CHECK_ERROR(gre_config->gre_r, 1);
    BITS_CHECK_ERROR(gre_config->gre_k, 1);
    BITS_CHECK_ERROR(gre_config->gre_s, 1);
    BITS_CHECK_ERROR(gre_config->gre_reserved0,9);
    BITS_CHECK_ERROR(gre_config->gre_ver, 3);
    BITS_CHECK_ERROR(gre_config->gre_protocal_type, 16);
    BITS_CHECK_ERROR(gre_config->gre_checksum, 16);
    BITS_CHECK_ERROR(gre_config->gre_reserved1, 16);


    data16 = ((gre_config->gre_c<<15)|(gre_config->gre_r<<14)|(gre_config->gre_k<<13)|
    (gre_config->gre_s<<12)|(gre_config->gre_reserved0<<3)|(gre_config->gre_ver));
    _diag_show_forward_memcpy_data(pkt, &data16, 2);
    data16 = gre_config->gre_protocal_type;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);
    if(gre_config->gre_c)
    {
        data16 = gre_config->gre_checksum;
        _diag_show_forward_memcpy_data(pkt, &data16, 2);
        data16 = gre_config->gre_reserved1;
        _diag_show_forward_memcpy_data(pkt, &data16, 2);
    }
    if(gre_config->gre_k)
    {
        data32 = gre_config->gre_key;
        _diag_show_forward_memcpy_data(pkt, &data32, 4);
    }
    if(gre_config->gre_s)
    {
        data32 = gre_config->gre_sq_num;
        _diag_show_forward_memcpy_data(pkt, &data32, 4);
    }
    if(gre_config->gre_r)
    {
        data32 = gre_config->gre_routing;
        _diag_show_forward_memcpy_data(pkt, &data32, 4);
    }

    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name     : _diag_show_forward_generate_bpdu
 * Purpose  : generate bpdu   
 * Input:      pkt: pointer of pkt
 *               bpdu_config: bpdu config struct.
 * Output   :     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_bpdu(uint8 **pkt, diag_bpdu_config_t *bpdu_config)
{
    uint16 data16 = 0;
    uint32 data32 = 0;

    **pkt = bpdu_config->llc_dsap;
    (*pkt)++;

    **pkt = bpdu_config->llc_ssap;
    (*pkt)++;

    **pkt = bpdu_config->llc_ctlo;
    (*pkt)++;

    data16 = bpdu_config->protocol_id;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

    **pkt = bpdu_config->ver;
    (*pkt)++;
    **pkt = bpdu_config->type;
    (*pkt)++;
    **pkt = bpdu_config->flag;
    (*pkt)++;

    data16 = bpdu_config->root_id_pri;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

    sal_memcpy(*pkt, bpdu_config->root_id_mac, 6);
    (*pkt) += 6;

    data32 = bpdu_config->root_pathcost;
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    data16 = bpdu_config->bridge_id_pri;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

    sal_memcpy(*pkt, bpdu_config->bridge_id_mac, 6);
    (*pkt) += 6;

    data16 = bpdu_config->port_id;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

    data16 = bpdu_config->msg_age;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

    data16 = bpdu_config->max_age;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

    data16 = bpdu_config->hello_time;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

    data16 = bpdu_config->fwd_delay;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

    data32 = bpdu_config->dlc_value1;
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    data32 = bpdu_config->dlc_value2;
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name     : _diag_show_forward_generate_igmp_v1
 * Purpose  : generate igmp v1   
 * Input:      pkt: pointer of pkt
 *               igmp: igmp config struct.
 *               igmp_chk_sum : chk sum
 * Output   :     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_igmp_v1(uint8 **pkt, diag_igmp_v1_config_t *igmp, uint8 **igmp_chk_sum)
{
    uint32 data32 = 0;

    **pkt = ((igmp->version << 4)|(igmp->type));
    (*pkt)++;

    **pkt = 0;
    (*pkt)++;

    *igmp_chk_sum = *pkt;
    (*pkt) += 2;

    data32 = igmp->group_address;
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    return CHSM_E_SUCCESS;

}

/****************************************************************************
 * Name     : _diag_show_forward_generate_igmp_v2
 * Purpose  : generate igmp v2   
 * Input:      pkt: pointer of pkt
 *               igmp: igmp config struct.
 *               igmp_chk_sum : chk sum
 * Output   : N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_igmp_v2(uint8 **pkt, diag_igmp_v2_config_t *igmp,  uint8 **igmp_chk_sum)
{
    uint32 data32 = 0;

    **pkt = igmp->type;
    (*pkt)++;

    **pkt = igmp->max_resp_time;
    (*pkt)++;

    *igmp_chk_sum = *pkt;
    (*pkt) += 2;

    data32 = igmp->group_address;
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    return CHSM_E_SUCCESS;

}

/****************************************************************************
 * Name     : _diag_show_forward_generate_igmp_v3
 * Purpose  : generate igmp v1   
 * Input:      pkt: pointer of pkt
 *               igmp: igmp config struct.
 *               igmp_chk_sum : chk sum
 * Output   : N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_igmp_v3(uint8 **pkt, diag_igmp_v3_config_t *igmp, uint8 **igmp_chk_sum)
{
    uint32 data32 = 0;
    uint16 data16 = 0;
    int32 i = 0;

    **pkt = igmp->type;
    (*pkt)++;

    **pkt = igmp->max_resp_time;
    (*pkt)++;

    *igmp_chk_sum = *pkt;
    (*pkt) += 2;

    data32 = igmp->group_address;
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    **pkt = (igmp->resv << 4)|(igmp->s << 3)|(igmp->qrv);
    (*pkt)++;

    **pkt = igmp->qqic;
    (*pkt)++;

    data16 = igmp->num_of_src;
    _diag_show_forward_memcpy_data(pkt, &data16, 2);

   for(i = 0; i< igmp->num_of_src; i++)
   {
        data32 = igmp->sr_address[i];
        _diag_show_forward_memcpy_data(pkt, &data32, 4);
   }
    return CHSM_E_SUCCESS;

}

/****************************************************************************
 * Name     : _diag_show_forward_generate_icmpv6_nsmf
 * Purpose  : generate icmpv6 nsmf  
 * Input:      pkt: pointer of pkt
 *               nsmf: icmpv6 nsmf config struct.
 *               icmpv6_chksum : chk sum
 * Output   :     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_icmpv6_nsmf(uint8 **pkt, diag_icmpv6_nsmf_config_t *nsmf, uint8 **icmpv6_chksum)
{
    uint32 data32 = 0;
    uint16 data16 = 0;

    **pkt = nsmf->type;
    (*pkt)++;

    **pkt = nsmf->code;
    (*pkt)++;

    *icmpv6_chksum = *pkt;
    (*pkt) += 2;

    data32 = nsmf->reserved;
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    data32 = nsmf->target_addr[0];
    _diag_show_forward_memcpy_data(pkt, &nsmf->target_addr[0], 4);

    data32 = nsmf->target_addr[1];
    _diag_show_forward_memcpy_data(pkt, &nsmf->target_addr[1], 4);

    data32 = nsmf->target_addr[2];
    _diag_show_forward_memcpy_data(pkt, &nsmf->target_addr[2], 4);

    data32 = nsmf->target_addr[3];
    _diag_show_forward_memcpy_data(pkt, &nsmf->target_addr[3], 4);

    if(nsmf->len)
    {
        data16 = nsmf->len;
        _diag_show_forward_memcpy_data(pkt, &data16, 2);
        sal_memcpy(*pkt, nsmf->targ_link_lay_addr, 6);
        (*pkt) += 6;
    }
    return CHSM_E_SUCCESS;

}

/****************************************************************************
 * Name     : _diag_show_forward_generate_icmpv6_namf
 * Purpose  : generate icmpv6 namf  
 * Input:      pkt: pointer of pkt
 *               namf: icmpv6 namf config struct.
 *               icmpv6_chksum : chk sum
 * Output   :     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_generate_icmpv6_namf(uint8 **pkt, diag_icmpv6_namf_config_t *namf, uint8 **icmpv6_chksum)
{
    uint32 data32 = 0;
    uint16 data16 = 0;

    **pkt = namf->type;
    (*pkt)++;

    **pkt = namf->code;
    (*pkt)++;

    *icmpv6_chksum = *pkt;
    (*pkt) += 2;

    data32 = ((namf->r << 31)|(namf->s <<30)|(namf->o << 29)|(namf->reserved));
    _diag_show_forward_memcpy_data(pkt, &data32, 4);

    data32 = namf->target_addr[0];
    _diag_show_forward_memcpy_data(pkt, &namf->target_addr[0], 4);

    data32 = namf->target_addr[1];
    _diag_show_forward_memcpy_data(pkt, &namf->target_addr[1], 4);

    data32 = namf->target_addr[2];
    _diag_show_forward_memcpy_data(pkt, &namf->target_addr[2], 4);

    data32 = namf->target_addr[3];
    _diag_show_forward_memcpy_data(pkt, &namf->target_addr[3], 4);

    if(namf->len)
    {
        data16 = namf->len;
        _diag_show_forward_memcpy_data(pkt, &data16, 2);
        sal_memcpy(*pkt, namf->targ_link_lay_addr, 6);
        (*pkt) += 6;
    }
    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name     : _diag_show_forward_fill_the_payload
 * Purpose  : fill payload 
 * Input:      pkt: pointer of pkt
 *               line: payload.
 *               config : payload config
 * Output   : N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
static int32
_diag_show_forward_fill_the_payload(FILE *fp,uint8 *line,diag_pld_config_t *config,uint8 **pkt)
{
    char   *ch = NULL;
    uint32 data32 = 0;
    uint32 static_part_len = 0;

    ch = sal_strstr((char *)line, "=");

    if (NULL != ch)
    {
        ch ++;
        /* skip white space */
        while (sal_isspace(*ch))
        {
            ch++;
        }

        while (TRUE)
        {
            while (sal_sscanf(ch, "%x", &data32) == 1)
            {
                **pkt = (uint8)data32;
                (*pkt)++;
                static_part_len ++;

                /* skip white space */
                ch += 2;
                while (sal_isspace(*ch))
                {
                    ch++;
                }
            }

            if (NULL == sal_fgets((char *)line, LINE_SIZE, fp))
            {
                break;
            }
            ch = (char *)line;
        }
    }

    if (config->pld_len > static_part_len)
    {
        _diag_show_forward_generate_random_pkt(pkt, (config->pld_len - static_part_len));
    }
    return CHSM_E_SUCCESS;
}

/****************************************************************************
 * Name     : diag_show_forward_construct_pkt
 * Purpose  : construct pkt   
 * Input:      fp: pointer of input file.     line: promt infomation
 *               fpkt :pointer of pkt .         ctrl_var:ctrl val struct 
 *               config: point of confit struct .
 * Output   :     N/A.
 * Return   : CHSM_E_XX
 * Note     : N/A
 ****************************************************************************/
int32
diag_show_forward_construct_pkt(FILE *fp, uint8 *line, diag_pkt_t *fpkt, diag_ctrl_var_t *ctrl_var, diag_config_t *config)
{ 
    uint16  data16 = 0;
    uint32  data32 = 0;
    uint8 *pkt= NULL;
    uint8 *pkt_start_ptr = NULL;
    uint8 *p_l3 = NULL;
    uint8 *p_total_len = NULL;
    uint8 *p_ip_chksum = NULL;
    uint8 *p_pld_len = NULL;
    uint8 *igmp_chksum = NULL;
    uint8 *p_l4 = NULL;
    uint8 *p_tcp_chksum = NULL;
    uint8 *p_udp_len = NULL;
    uint8 *icmpv6_chksum = NULL;
    uint32 icmp_pkt_len = 0;
    uint8 *igmp_start = NULL;
    uint32 igmp_len = 0;
    diag_bay_packet_header_t  bay_hdr; /*here need optimize  to humber header,later*/
    uint32 index = 0;
    uint8  bay_header_crc_config_flag=FALSE;
    bool is_igmp = FALSE;
    uint32 igmp_index = 0;
    diag_humber_exception_info_t ecptn;
    uint8 *bayhdr_start = NULL;
    uint8 *humber_exception_start = NULL;

    sal_memset(&ecptn, 0, sizeof (diag_humber_exception_info_t));
    sal_memset(&bay_hdr, 0, sizeof(diag_bay_packet_header_t));
    /*
    * generate packet headers according to config data
    */
    pkt_start_ptr = fpkt->pkt;
    pkt = fpkt->pkt;

    while (DIAG_PAYLOAD!= config[index].header_type)
    {
        switch ( config[index].header_type )
        {
            case DIAG_BAY_HEADER_HDR:
                if ( fpkt->has_hdr )
                {
                    bayhdr_start = pkt;
                    ctrl_var[index].bay_hdr.bay_hdr_start_offset = pkt - pkt_start_ptr;
                    pkt += sizeof(diag_bay_packet_header_t);
                    _diag_show_forward_generate_bay_header(&bay_hdr, &config[index].bay_hdr);
                    if (TRUE == config[index].bay_hdr.hdr_crc_config)
                    {
                        bay_header_crc_config_flag = TRUE;
                    }
                    if (FORWARD_IS_BIT_SET(bay_hdr.nxt_hop_ptr,16))
                    {
                        data32 = config[index].bay_hdr.customer_label;
                        _diag_show_forward_memcpy_data(&pkt, &data32, 4);
                        if (FORWARD_IS_BIT_SET(bay_hdr.nxt_hop_ptr,17))
                        {
                            data32 = config[index].bay_hdr.martini_encap;
                            _diag_show_forward_memcpy_data(&pkt, &data32, 4);
                        }
                    }
                }
                
                break;
            case DIAG_BAY_EXCEPTION_HDR:
                _diag_show_forward_generate_exception_header(&ecptn, &config[index].exception);
                humber_exception_start = pkt;
                pkt += sizeof(diag_humber_exception_info_t);
                sal_memcpy(humber_exception_start, &ecptn, sizeof(diag_humber_exception_info_t));
                _diag_show_forward_swap32((uint32 *)humber_exception_start, 1, DIAG_HOST_TO_NETWORK);
                
                break;
            case DIAG_ETHERNET_HDR:
                _diag_show_forward_generate_l2_header(&pkt, &config[index].ethernet, &ctrl_var[index].ethernet, pkt_start_ptr);
                
                break;
            case DIAG_IPV4_HDR:
                ctrl_var[index].ipv4.ipv4_start = pkt;
                _diag_show_forward_generate_l3_ipv4(&pkt,&p_total_len,&p_ip_chksum, 
                                                                                &config[index].ipv4, &ctrl_var[index].ipv4,pkt_start_ptr);

                ctrl_var[index].ipv4.total_len_offset = p_total_len - pkt_start_ptr;
                ctrl_var[index].ipv4.check_sum_offset = p_ip_chksum - pkt_start_ptr;
                
                break;
            case DIAG_IPV6_HDR:
                ctrl_var[index].ipv6.ipv6_start = pkt;
                _diag_show_forward_generate_l3_ipv6(&pkt,&p_pld_len,&config[index].ipv6,&ctrl_var[index].ipv6,pkt_start_ptr);

                ctrl_var[index].ipv6.total_len_offset = p_pld_len - pkt_start_ptr;
                
                break;
            case DIAG_ARP_HDR:
                _diag_show_forward_generate_l3_arp(&pkt, &config[index].arp);
                
                break;
            case DIAG_MPLS_HDR:
                _diag_show_forward_generate_l3_mpls(&pkt, ctrl_var[index].mpls,&config[index].mpls, pkt_start_ptr);
                
                break;
            case DIAG_GRE_HDR:
                _diag_show_forward_generate_l3_gre(&pkt,&config[index].gre);
                
                break;
            case DIAG_BPDU_HDR:
               _diag_show_forward_generate_bpdu(&pkt, &config[index].bpdu);
               
                break;
            case DIAG_ICMPV6_NSMF:
               _diag_show_forward_generate_icmpv6_nsmf(&pkt, &config[index].icmpv6_nsmf, &icmpv6_chksum);
               
                break;
            case DIAG_ICMPV6_NAMF:
               _diag_show_forward_generate_icmpv6_namf(&pkt, &config[index].icmpv6_namf, &icmpv6_chksum);
               
                break;
            case DIAG_IGMP_V1_HDR:
                igmp_start = pkt;
                _diag_show_forward_generate_igmp_v1(&pkt, &config[index].igmp_v1, &igmp_chksum);
                
                break;
            case DIAG_IGMP_V2_HDR:
                igmp_start = pkt;
                _diag_show_forward_generate_igmp_v2(&pkt, &config[index].igmp_v2, &igmp_chksum);
                
                break;
            case DIAG_IGMP_V3_HDR:
                igmp_start = pkt;
                _diag_show_forward_generate_igmp_v3(&pkt, &config[index].igmp_v3, &igmp_chksum);
                igmp_len = pkt - igmp_start;
                
                break;
            case DIAG_TCP_HDR:
                ctrl_var[index].tcp.tcp_start = pkt;
                _diag_show_forward_generate_l4_tcp(&pkt,&p_tcp_chksum,&config[index].tcp,&ctrl_var[index].tcp,pkt_start_ptr);
                ctrl_var[index].tcp.check_sum_offset = p_tcp_chksum - pkt_start_ptr;
                
                break;
            case DIAG_UDP_HDR:
                ctrl_var[index].udp.udp_start = pkt;
                _diag_show_forward_generate_l4_udp(&pkt,&p_udp_len,&config[index].udp,&ctrl_var[index].udp,pkt_start_ptr);
                ctrl_var[index].udp.udp_len_offset = p_udp_len - pkt_start_ptr;
                
                break;
            case DIAG_PAD_HDR:
                ctrl_var[index].pad.pad_offset = pkt - pkt_start_ptr;
                _diag_show_forward_generate_pad(&pkt, &config[index].pad);
                
                break;
            default:
                break;
        }

        index++;
    }

    if( DIAG_PAYLOAD == config[index].header_type)
    {
        _diag_show_forward_fill_the_payload(fp, line, &config[index].payload, &pkt);
    }

    index = 0;

    while (DIAG_PAYLOAD!= config[index].header_type)
    {
        switch ( config[index].header_type )
        {
            case DIAG_IPV4_HDR:
                 /* fill ipv4 total length */
                if (config[index].ipv4.ipv4_total_len)
                {
                    data16 = config[index].ipv4.ipv4_total_len;
                }
                else
                {
                    data16 = (uint16)(pkt - ctrl_var[index].ipv4.ipv4_start);
                }

                p_pld_len = pkt_start_ptr + ctrl_var[index].ipv4.total_len_offset;

                _diag_show_forward_memcpy_data(&p_pld_len, &data16, 2);

                if (config[index].ipv4.ipv4_hdr_checksum)
                {
                    data16 = config[index].ipv4.ipv4_hdr_checksum;
                }
                else
                {
                    data16 = _diag_show_forward_ip_chksum((uint16*)(ctrl_var[index].ipv4.ipv4_start), 20, 0);  /* no options for now */
                }

                p_ip_chksum =  pkt_start_ptr + ctrl_var[index].ipv4.check_sum_offset;
                _diag_show_forward_memcpy_data(&p_ip_chksum, &data16, 2);
                
                break;
            case DIAG_IPV6_HDR:
                if (config[index].ipv6.ipv6_pld_len)
                {
                    data16 = config[index].ipv6.ipv6_pld_len;
                }
                else
                {
                    data16 = (uint16)(pkt - ctrl_var[index].ipv6.ipv6_start) - 40;
                }
                p_pld_len = pkt_start_ptr + ctrl_var[index].ipv6.total_len_offset;
                _diag_show_forward_memcpy_data(&p_pld_len, &data16, 2);
                
                break;
            case DIAG_ICMPV6_NSMF:
               if(config[index].icmpv6_nsmf.chksum)
                   data16 = config[index].icmpv6_nsmf.chksum;
               else
               {
                   if(config[index].icmpv6_nsmf.len)
                   {
                        icmp_pkt_len = 32;
                   }
                   else
                    {
                       icmp_pkt_len = 24;
                    }

                   data16 = (uint16)_diag_show_forward_icmp6_cksum(config[index - 1].ipv6.ipv6_sa, config[index - 1].ipv6.ipv6_da,
                                                 (ctrl_var[index - 1].ipv6.ipv6_start + 40), icmp_pkt_len);
               }
               sal_memcpy(icmpv6_chksum, &data16, 2);
                
                break;
            case DIAG_ICMPV6_NAMF:
                if(config[index].icmpv6_namf.chksum)
                {
                   data16 = config[index].icmpv6_namf.chksum;
                }
                else
                {
                   if(config[index].icmpv6_namf.len)
                   {
                        icmp_pkt_len = 32;
                   }
                   else
                    {
                       icmp_pkt_len = 24;
                    }

                   data16 = (uint16)_diag_show_forward_icmp6_cksum(config[index - 1].ipv6.ipv6_sa, config[index - 1].ipv6.ipv6_da,
                                                 (ctrl_var[index - 1].ipv6.ipv6_start + 40), icmp_pkt_len);
                }
                sal_memcpy(icmpv6_chksum, &data16, 2);
                
                break;
            case DIAG_IGMP_V1_HDR:
                is_igmp = TRUE;
                igmp_index = index;
                
                break;
            case DIAG_IGMP_V2_HDR:
                is_igmp = TRUE;
                igmp_index = index;
                
                break;
            case DIAG_IGMP_V3_HDR:
                is_igmp = TRUE;
                igmp_index = index;
                
                break;
            case DIAG_TCP_HDR:
                if ( 0 != config[index].tcp.seq)
                {
                    if(config[index].tcp.chksum)
                    {
                      data16 = config[index].tcp.chksum;
                    }
                    else
                    {
                      p_l4 = ctrl_var[index].tcp.tcp_start;
                      if (DIAG_IPV6_HDR== config[index-1].header_type)
                      {
                          p_l3 = ctrl_var[index-1].ipv6.ipv6_start;
                          data16 = _diag_show_forward_tcp_chksum(&config[index-1], 1, (uint16*)p_l3, (uint16*)p_l4, (uint16)(pkt - p_l4), 0);
                      }
                      else if(DIAG_IPV4_HDR== config[index-1].header_type)
                      {
                          p_l3 = ctrl_var[index-1].ipv4.ipv4_start;
                          data16 = _diag_show_forward_tcp_chksum(&config[index-1], 0, (uint16*)p_l3, (uint16*)p_l4, (uint16)(pkt - p_l4), 0);
                      }
                    }
                    p_tcp_chksum = pkt_start_ptr + ctrl_var[index].tcp.check_sum_offset;
                    _diag_show_forward_memcpy_data(&p_tcp_chksum, &data16, 2);

                }

                break;
            case DIAG_UDP_HDR:
                p_l4 = ctrl_var[index].udp.udp_start;
                p_udp_len = pkt_start_ptr + ctrl_var[index].udp.udp_len_offset;
                if (0 != config[index].udp.dport)
                {
                    if(config[index].udp.len)
                    {
                        data16 = config[index].udp.len;
                    }
                    else
                    {
                        data16 = (uint16)(pkt - p_l4);
                    }
                    _diag_show_forward_memcpy_data(&p_udp_len, &data16, 2);
                }
                
                break;
            default:
                break;
        }
        index++;
    }

    /*add bay header before packet*/
    fpkt->pkt_len = (uint16)(pkt - fpkt->pkt);

    if(fpkt->has_hdr)
    {
        //pkt len not include crc len, so here
        if(bay_hdr.pkt_len == 0)
        {
            bay_hdr.pkt_len = pkt -bayhdr_start -sizeof(diag_bay_packet_header_t)+4;
        }
       sal_memcpy(bayhdr_start, &bay_hdr, sizeof(diag_bay_packet_header_t));
       _diag_show_forward_swap32((uint32 *)bayhdr_start, sizeof(diag_bay_packet_header_t) / 4, DIAG_HOST_TO_NETWORK);

        if(TRUE != bay_header_crc_config_flag)
        {
            *(bayhdr_start+sizeof(diag_bay_packet_header_t)-1) = 0;
            *(bayhdr_start+sizeof(diag_bay_packet_header_t)-1) = calculate_crc8(bayhdr_start, sizeof(diag_bay_packet_header_t), 0);
        }
    }

    if(is_igmp)
    {
        if(config[igmp_index].igmp_v3.checksum)
        {
            data16 = config[igmp_index].igmp_v3.checksum;
        }
        else
        {
            data16 = _diag_show_forward_igmp_checksum((uint16*)igmp_start, (pkt - igmp_start));
        }

        sal_memcpy(igmp_chksum, &data16, 2);
    }

    _diag_show_forward_ctcutil_crc32(0xFFFFFFFF, fpkt->pkt, fpkt->pkt_len, &data32);
    _diag_show_forward_swap32(&data32, 1, DIAG_HOST_TO_NETWORK);
    _diag_show_forward_memcpy_data(&pkt, &data32, 4);
    fpkt->pkt_len += 4;

    return CHSM_E_SUCCESS;

}


