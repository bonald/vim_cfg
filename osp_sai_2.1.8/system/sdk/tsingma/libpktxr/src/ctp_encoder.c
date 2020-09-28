/****************************************************************************
 * ctp_encoder.c  provides all CTP Packet Encoder Fucntion and Macros define.
 *
 * Copyright: (c)2007 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       Anchor Xie.
 * Date:         2007-4-16.
 * Reason:       First Create.
*****************************************************************************/

/****************************************************************************
 *
 * Header Files
 *
****************************************************************************/
#include <stdio.h>
#include "cts_xr_config.h"
#include "ctp_common.h"
#include "ctp_util.h"
#include <netinet/in.h>
#include <openssl/md5.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "ctp_queue.h"

/****************************************************************************
 *
 * Data Structure and Defines
 *
****************************************************************************/
#ifndef min
#define min(a, b) ((a) > (b) ? (b) : (a))
#endif

#define __M_PKT_MODIFIER_VAR_NAME   "::__PKT_MODIFIER_RETVAR"

/****************************************************************************
 *
 * Global and Declarations
 *
 ****************************************************************************/
/* Added by Alexander 2012-10-18 19:27 */
ctp_pkt_template_hdr_t g_pkt_template_hdr;

DelayProcList_t g_delayProcList = TAILQ_HEAD_INITIALIZER(g_delayProcList);


/* current packet decoder var */
ctp_pkt_template_t *g_global_pkt_template = NULL;

#define IPV4_ADDR_LEN 4
#define IPV6_ADDR_LEN 16

#define BAY_HEADER_LEN 32

static uint8 CRC8_TABLE[]= {
    0x00,   0x07,   0x0E,   0x09,   0x1C,   0x1B,   0x12,   0x15,
    0x38,   0x3F,   0x36,   0x31,   0x24,   0x23,   0x2A,   0x2D,
    0x70,   0x77,   0x7E,   0x79,   0x6C,   0x6B,   0x62,   0x65,
    0x48,   0x4F,   0x46,   0x41,   0x54,   0x53,   0x5A,   0x5D,
    0xE0,   0xE7,   0xEE,   0xE9,   0xFC,   0xFB,   0xF2,   0xF5,
    0xD8,   0xDF,   0xD6,   0xD1,   0xC4,   0xC3,   0xCA,   0xCD,
    0x90,   0x97,   0x9E,   0x99,   0x8C,   0x8B,   0x82,   0x85,
    0xA8,   0xAF,   0xA6,   0xA1,   0xB4,   0xB3,   0xBA,   0xBD,
    0xC7,   0xC0,   0xC9,   0xCE,   0xDB,   0xDC,   0xD5,   0xD2,
    0xFF,   0xF8,   0xF1,   0xF6,   0xE3,   0xE4,   0xED,   0xEA,
    0xB7,   0xB0,   0xB9,   0xBE,   0xAB,   0xAC,   0xA5,   0xA2,
    0x8F,   0x88,   0x81,   0x86,   0x93,   0x94,   0x9D,   0x9A,
    0x27,   0x20,   0x29,   0x2E,   0x3B,   0x3C,   0x35,   0x32,
    0x1F,   0x18,   0x11,   0x16,   0x03,   0x04,   0x0D,   0x0A,
    0x57,   0x50,   0x59,   0x5E,   0x4B,   0x4C,   0x45,   0x42,
    0x6F,   0x68,   0x61,   0x66,   0x73,   0x74,   0x7D,   0x7A,
    0x89,   0x8E,   0x87,   0x80,   0x95,   0x92,   0x9B,   0x9C,
    0xB1,   0xB6,   0xBF,   0xB8,   0xAD,   0xAA,   0xA3,   0xA4,
    0xF9,   0xFE,   0xF7,   0xF0,   0xE5,   0xE2,   0xEB,   0xEC,
    0xC1,   0xC6,   0xCF,   0xC8,   0xDD,   0xDA,   0xD3,   0xD4,
    0x69,   0x6E,   0x67,   0x60,   0x75,   0x72,   0x7B,   0x7C,
    0x51,   0x56,   0x5F,   0x58,   0x4D,   0x4A,   0x43,   0x44,
    0x19,   0x1E,   0x17,   0x10,   0x05,   0x02,   0x0B,   0x0C,
    0x21,   0x26,   0x2F,   0x28,   0x3D,   0x3A,   0x33,   0x34,
    0x4E,   0x49,   0x40,   0x47,   0x52,   0x55,   0x5C,   0x5B,
    0x76,   0x71,   0x78,   0x7F,   0x6A,   0x6D,   0x64,   0x63,
    0x3E,   0x39,   0x30,   0x37,   0x22,   0x25,   0x2C,   0x2B,
    0x06,   0x01,   0x08,   0x0F,   0x1A,   0x1D,   0x14,   0x13,
    0xAE,   0xA9,   0xA0,   0xA7,   0xB2,   0xB5,   0xBC,   0xBB,
    0x96,   0x91,   0x98,   0x9F,   0x8A,   0x8D,   0x84,   0x83,
    0xDE,   0xD9,   0xD0,   0xD7,   0xC2,   0xC5,   0xCC,   0xCB,
    0xE6,   0xE1,   0xE8,   0xEF,   0xFA,   0xFD,   0xF4,   0xF3};




static uint8*
 binary_dup (enum memory_type type,uint8 *src,int len)
{
    uint8*pmem=NULL;
    pmem=(uint8 *)XMALLOC(type,len);
    if (pmem !=NULL) {
        memcpy(pmem,src,len);
    } else {
        printf("Memory allocation failed!\n");
        assert(0);
    }
    return pmem;
}

#define  XBINDUP(a,b,len)       binary_dup((a),(b),(len));



static ctp_pkt_template_t *pkt_template_lookup(char *pszName);
static void encode_free_header(ctp_pkt_template_t *pTemplate);
static void encode_free_cmdlist(ctp_pkt_template_t *pTemplate);
static ctp_error_e packet_compose(ctp_pkt_template_t *pTemplate, int frame_size);
static int __pkt_template_get_modified_data(char *pszTemplate, pm_cb_t *pItem,
    int *pmodoffset, int *pmodlen, uint8 *pmodbuf);
static ctp_error_e packet_encoder(int bCompose, ctp_pkt_template_t *ptr);


static int reg_delay_proc_field(char *pszFieldName, delay_field_proc_cb procCB,
    int nStoreDelta, int bGlobal)
{
    DelayProcField_t *pField = NULL;

    TAILQ_FOREACH(pField, &g_delayProcList, entry) {
        if (!strcmp(pField->pszName, pszFieldName)) {
            fprintf(stderr, "register_delayed_process_field: field %s exist\n",
                    pszFieldName);
            return 0;
        }
    }

    pField = XCALLOC(MTYPE_NEW_CTP, sizeof(DelayProcField_t));
    if (NULL == pField) {
        fprintf(stderr, "register_delayed_process_field: Out of memory\n");
        return -1;
    }

    TAILQ_INSERT_TAIL(&g_delayProcList, pField, entry);
    pField->pszName = XSTRDUP(MTYPE_NEW_CTP, pszFieldName);
    pField->procCB = procCB;
    pField->nStoreDelta = nStoreDelta;
    pField->bGlobal = bGlobal;
    return 0;
}

static DelayProcField_t *find_delay_proc_field(char *pszFieldName)
{
    DelayProcField_t *pField = NULL;

    TAILQ_FOREACH(pField, &g_delayProcList, entry) {
        if (strstr(pszFieldName, pField->pszName)) {
            return pField;
        }
    }

    return NULL;
}

static DelayProcParam_t *find_delay_proc_param(DelayProcParamList_t *pList, char *pszFieldName)
{
    DelayProcParam_t *pItem = NULL;

    TAILQ_FOREACH(pItem, pList, entry) {
        if (!strcmp(pItem->pField->pszName, pszFieldName)) {
            return pItem;
        }
    }

    return NULL;
}

static int add_delay_proc_param(DelayProcParamList_t *pList, DelayProcField_t *pDelayProcField,
    unsigned char *pDelayOffset, int nDelayProcLen, int nStoreOffset, int nStart,
    pm_cb_list_t *pModList)
{
    DelayProcParam_t *pItem = XCALLOC(MTYPE_NEW_CTP, sizeof(DelayProcParam_t));
    if (NULL == pItem) {
        return -1;
    }

    pItem->pOffset = pDelayOffset;
    pItem->nLen = nDelayProcLen;
    pItem->pModList = pModList;
    pItem->pField = pDelayProcField;
    pItem->nStoreOffset = nStoreOffset + pDelayProcField->nStoreDelta;
    pItem->nStart = nStart;
    TAILQ_INSERT_TAIL(pList, pItem, entry);
    return 0;
}

/*
 * nType:
 *   0: none global item
 *   1: all item
 *
 */
static void destroy_delay_proc_param_list(DelayProcParamList_t *pList, int nType)
{
    DelayProcParam_t *pItem = NULL;
    DelayProcParam_t *pOldItem = NULL;

    TAILQ_FOREACH(pItem, pList, entry) {
        if (0 == nType && pItem->pField->bGlobal) {
            continue;
        }

        if (pOldItem) {
            TAILQ_REMOVE(pList, pOldItem, entry);
            XFREE(MTYPE_NEW_CTP, pOldItem);
        }
        pOldItem = pItem;
    }

    if (pOldItem) {
        TAILQ_REMOVE(pList, pOldItem, entry);
        XFREE(MTYPE_NEW_CTP, pOldItem);
    }
    return;
}

#define M_SPECIAL_GET_MODIFIED_DATA(szField) \
do { \
    if (0 != __pkt_template_get_modified_data(pTemplate->name, \
                TAILQ_FIRST(pParam->pModList), &modoffset, &modlen, szModBuf)) { \
        CTP_OUT("Packet template execute modifier for field %s failed\n", szField); \
        return -1; \
    } \
\
    if (pParam->nLen != (modlen / 8)) { \
        CTP_OUT("Modifier returned value for field %s invalid\n", szField); \
        return -1; \
    } \
\
    data16 = szModBuf[0]; \
} while (0)


static int chksm_delay_proc(ctp_pkt_template_t *pTemplate,
    DelayProcParam_t *pParam, int nCurOffset)
{
    int modoffset = 0, modlen = 0;
    unsigned char szModBuf[M_MAXPKT_BUFFER_SIZE];
    u_int16_t data16 = 0;
    u_int16_t *p_start = NULL;
    u_int32_t chksm_len = 0;

#if 0
    printf("start %d store_offset %d chksm_byte_len %d\n",
        nCurOffset, pParam->nStoreOffset, pParam->nLen);
#endif
    memset(pParam->pOffset, 0, pParam->nLen);
    if (TAILQ_EMPTY(pParam->pModList)) {
        p_start = (u_int16_t*)(&(pTemplate->epkt.packet[(pParam->nStoreOffset+7)/8]));
        chksm_len = (nCurOffset - pParam->nStoreOffset)/8;
        data16 = ctc_pktxr_calculate_chksum(p_start, chksm_len, 0);
    } else {
        M_SPECIAL_GET_MODIFIED_DATA("CHKSM");
    }

    ctc_pktxr_memcpy_chksum(&pParam->pOffset, &data16, pParam->nLen);
    return 0;
}

static int ipv4len_delay_proc(ctp_pkt_template_t *pTemplate,
    DelayProcParam_t *pParam, int nCurOffset)
{
    int modoffset = 0, modlen = 0;
    unsigned char szModBuf[M_MAXPKT_BUFFER_SIZE];
    u_int16_t data16 = 0;

    memset(pParam->pOffset, 0x00, pParam->nLen);

    if (TAILQ_EMPTY(pParam->pModList)) {
        data16 = (pTemplate->pkt_base_bits - pParam->nStoreOffset + 32)/8;
        if (pTemplate->epkt.length > (pTemplate->pkt_base_bits / 8) + 4) {
            data16 +=  pTemplate->epkt.length - (pTemplate->pkt_base_bits / 8) - 4;
        }
    } else {
        M_SPECIAL_GET_MODIFIED_DATA("IPV4_LEN");
    }

    ctc_pktxr_memcpy_chksum(&pParam->pOffset, &data16, pParam->nLen);
    return 0;
}

static int icmpv6_chksum_delay_proc(ctp_pkt_template_t *pTemplate,
    DelayProcParam_t *pParam, int nCurOffset)
{
    int modoffset = 0, modlen = 0;
    unsigned char szModBuf[M_MAXPKT_BUFFER_SIZE];
    u_int16_t data16 = 0;
    u_int16_t *p_start = NULL;
    u_int32_t chksm_len = 0;
    u_int32_t pseudo_header_checksum = 0;

    memset(pParam->pOffset, 0x00, pParam->nLen);
    if (TAILQ_EMPTY(pParam->pModList)) {
        int i;
        for (i = 0; i < pTemplate->ip_len; i+=2)
        {
            pseudo_header_checksum += ntohs(*(uint16*)(pTemplate->ip_da + i));
        }

        for (i = 0; i < pTemplate->ip_len; i+=2)
        {
            pseudo_header_checksum += ntohs(*(uint16*)(pTemplate->ip_sa + i));
        }
        p_start = (uint16*)(&(pTemplate->epkt.packet[(pParam->nStoreOffset+7)/8]));
        chksm_len = (nCurOffset - pParam->nStoreOffset)/8;

        pseudo_header_checksum += 58;
        pseudo_header_checksum += (nCurOffset - pParam->nStoreOffset)/8;
        data16 = ctc_pktxr_calculate_chksum(p_start, chksm_len, pseudo_header_checksum);
    } else {
         M_SPECIAL_GET_MODIFIED_DATA("ICMPV6_CHECKSUM");
    }

    ctc_pktxr_memcpy_chksum(&pParam->pOffset, &data16, pParam->nLen);
    return 0;
}

static int icmp_chksum_delay_proc(ctp_pkt_template_t *pTemplate,
    DelayProcParam_t *pParam, int nCurOffset)
{
    int modoffset = 0, modlen = 0;
    unsigned char szModBuf[M_MAXPKT_BUFFER_SIZE];
    u_int16_t data16 = 0;
    u_int16_t *p_start = NULL;
    u_int32_t chksm_len = 0;

    memset(pParam->pOffset, 0x00, pParam->nLen);
    if (TAILQ_EMPTY(pParam->pModList)) {
        p_start = (uint16*)(&(pTemplate->epkt.packet[(pParam->nStoreOffset+7)/8]));
        chksm_len = (nCurOffset - pParam->nStoreOffset)/8;
        data16 = ctc_pktxr_calculate_chksum(p_start, chksm_len, 0);
    } else {
        M_SPECIAL_GET_MODIFIED_DATA("ICMP_CHECKSUM");
    }

    ctc_pktxr_memcpy_chksum(&pParam->pOffset, &data16, pParam->nLen);
    return 0;
}

static int tcp_checksum_delay_proc(ctp_pkt_template_t *pTemplate,
    DelayProcParam_t *pParam, int nCurOffset)
{
    int modoffset = 0, modlen = 0;
    unsigned char szModBuf[M_MAXPKT_BUFFER_SIZE];
    u_int16_t data16 = 0;
    u_int16_t *p_start = NULL;
    u_int32_t chksm_len = 0;
    uint32 pseudo_header_checksum = 0;

    if (TAILQ_EMPTY(pParam->pModList)) {
        int i;
        for (i = 0; i < pTemplate->ip_len; i+=2)
        {
            pseudo_header_checksum += ntohs(*(uint16*)(pTemplate->ip_da + i));
        }

        for (i = 0; i < pTemplate->ip_len; i+=2)
        {
            pseudo_header_checksum += ntohs(*(uint16*)(pTemplate->ip_sa + i));
        }
        p_start = (uint16*)(&(pTemplate->epkt.packet[(pParam->nStoreOffset+7)/8]));// should notice not to minus 1
        chksm_len = pTemplate->epkt.length - pParam->nStoreOffset/8 - 4;
        pseudo_header_checksum += 6;
        pseudo_header_checksum += chksm_len;
        data16 = ctc_pktxr_calculate_chksum(p_start, chksm_len, pseudo_header_checksum);
    } else {
        M_SPECIAL_GET_MODIFIED_DATA("TCP_CHECKSUM");
    }

    ctc_pktxr_memcpy_chksum(&pParam->pOffset, &data16, pParam->nLen);
    return 0;
}

static int hm_header_crc_delay_proc(ctp_pkt_template_t *pTemplate,
    DelayProcParam_t *pParam, int nCurOffset)
{
    int modoffset = 0, modlen = 0;
    unsigned char szModBuf[M_MAXPKT_BUFFER_SIZE];
    u_int16_t data16 = 0;
    uint8 crc = 0;
    uint32 humber_crc_offset = (nCurOffset - 136 + 7)/8;

    if (TAILQ_EMPTY(pParam->pModList)) {
        int32 cnt;
        uint32 humber_head_start = (nCurOffset - 256 + 7)/8;

        for (cnt = 0; cnt < BAY_HEADER_LEN; cnt++)
        {
            crc = CRC8_TABLE[(crc ^ pTemplate->epkt.packet[humber_head_start + cnt])];
        }
    } else {
        M_SPECIAL_GET_MODIFIED_DATA("CPUPKT_HM_HEADER_CRC");
        crc = (data16 & 0xFF);
    }

    pTemplate->epkt.packet[humber_crc_offset] = crc;
    return 0;
}

static int gb_header_crc_delay_proc(ctp_pkt_template_t *pTemplate,
    DelayProcParam_t *pParam, int nCurOffset)
{
    int modoffset = 0, modlen = 0;
    unsigned char szModBuf[M_MAXPKT_BUFFER_SIZE];
    u_int16_t data16 = 0;
    uint8 crc = 0;
    uint32 greatbelt_crc_offset = (nCurOffset - 136 + 7) / 8;

    if (TAILQ_EMPTY(pParam->pModList)) {
        uint32 greatbelt_head_start = (nCurOffset - 256 + 7) / 8;

        crc = ctc_pktxr_calculate_crc4((uint8 *)&pTemplate->epkt.packet[greatbelt_head_start], 32, 0);
    } else {
        M_SPECIAL_GET_MODIFIED_DATA("CPUPKT_GB_HEADER_CRC");
        crc = (data16 & 0xFF);
    }
    pTemplate->epkt.packet[greatbelt_crc_offset] &= 0xF0;
    pTemplate->epkt.packet[greatbelt_crc_offset] |= crc;
    return 0;
}

static int udp_len_delay_proc(ctp_pkt_template_t *pTemplate,
    DelayProcParam_t *pParam, int nCurOffset)
{
    int modoffset = 0, modlen = 0;
    unsigned char szModBuf[M_MAXPKT_BUFFER_SIZE];
    u_int16_t data16 = 0;

    memset(pParam->pOffset, 0x00, pParam->nLen);
    if (TAILQ_EMPTY(pParam->pModList)) {
        data16 = pTemplate->epkt.length - pParam->nStoreOffset/8 - 4;
    } else {
        M_SPECIAL_GET_MODIFIED_DATA("UDP_LENGTH");
    }

    ctc_pktxr_memcpy_chksum(&pParam->pOffset, &data16, pParam->nLen);
    return 0;
}

static int udp_chksum_delay_proc(ctp_pkt_template_t *pTemplate,
    DelayProcParam_t *pParam, int nCurOffset)
{
    int modoffset = 0, modlen = 0;
    unsigned char szModBuf[M_MAXPKT_BUFFER_SIZE];
    u_int16_t data16 = 0;
    u_int16_t *p_start = NULL;
    u_int32_t chksm_len = 0;
    uint32 pseudo_header_checksum = 0;

    memset(pParam->pOffset, 0x00, pParam->nLen);
    if (TAILQ_EMPTY(pParam->pModList)) {
        int i;

        for (i = 0; i < pTemplate->ip_len; i+=2)
        {
            pseudo_header_checksum += ntohs(*(uint16*)(pTemplate->ip_da + i));
        }

        for (i = 0; i < pTemplate->ip_len; i+=2)
        {
            pseudo_header_checksum += ntohs(*(uint16*)(pTemplate->ip_sa + i));
        }
        p_start = (uint16*)(&(pTemplate->epkt.packet[(pParam->nStoreOffset+7)/8]));
        chksm_len = pTemplate->epkt.length - pParam->nStoreOffset/8 - 4;

        pseudo_header_checksum += 17;
        pseudo_header_checksum += chksm_len;
        data16 = ctc_pktxr_calculate_chksum(p_start, chksm_len, pseudo_header_checksum);
    } else {
        M_SPECIAL_GET_MODIFIED_DATA("UDP_CHECKSUM");
    }

    ctc_pktxr_memcpy_chksum(&pParam->pOffset, &data16, pParam->nLen);
    return 0;
}

static int ipv6_payl_len_delay_proc(ctp_pkt_template_t *pTemplate,
    DelayProcParam_t *pParam, int nCurOffset)
{
    int modoffset = 0, modlen = 0;
    unsigned char szModBuf[M_MAXPKT_BUFFER_SIZE];
    u_int16_t data16 = 0;

    memset(pParam->pOffset, 0, pParam->nLen);

    if (TAILQ_EMPTY(pParam->pModList)) {
        data16 = pTemplate->epkt.length - pParam->nStoreOffset / 8 - 4;
    } else {
        M_SPECIAL_GET_MODIFIED_DATA("IPV6_PAYL_LEN");
    }

    ctc_pktxr_memcpy_chksum(&pParam->pOffset, &data16, pParam->nLen);
    return 0;
}

static int ext_len_delay_proc(ctp_pkt_template_t *pTemplate,
    DelayProcParam_t *pParam, int nCurOffset)
{
    int modoffset = 0, modlen = 0;
    unsigned char szModBuf[M_MAXPKT_BUFFER_SIZE];
    u_int16_t data16 = 0;

    memset(pParam->pOffset, 0, pParam->nLen);
    if (TAILQ_EMPTY(pParam->pModList)) {
        *pParam->pOffset = (nCurOffset - pParam->nStoreOffset)/64 - 1;
    } else {
        M_SPECIAL_GET_MODIFIED_DATA("IPV6_PAYL_LEN");
        *pParam->pOffset = (data16 & 0xFF);
    }
    return 0;
}

static void register_special_fields(void)
{
    reg_delay_proc_field("HEAD_PAYLOAD_LEN", NULL, 0, 0);
    reg_delay_proc_field("CHKSM", chksm_delay_proc, 0, 0);
    reg_delay_proc_field("IPV4_LEN", ipv4len_delay_proc, 32, 0);
    reg_delay_proc_field("ICMPV6_CHECKSUM", icmpv6_chksum_delay_proc, 0, 0);
    reg_delay_proc_field("ICMP_CHECKSUM", icmp_chksum_delay_proc, 0, 0);
    reg_delay_proc_field("CPUPKT_HM_HEADER_CRC", hm_header_crc_delay_proc, 0, 0);
    reg_delay_proc_field("CPUPKT_GB_HEADER_CRC", gb_header_crc_delay_proc, 0, 0);
    reg_delay_proc_field("UDP_LENGTH", udp_len_delay_proc, 0, 0);
    reg_delay_proc_field("IPV6_PAYL_LEN", ipv6_payl_len_delay_proc, IPV6_HEADER_LEN * 8, 0);
    reg_delay_proc_field("EXT_LENGTHER", ext_len_delay_proc, 0, 0);

    reg_delay_proc_field("TCP_CHECKSUM", tcp_checksum_delay_proc, 0, 1);
    reg_delay_proc_field("UDP_CHECKSUM", udp_chksum_delay_proc, 0, 1);
    return;
}

/****************************************************************************
 *
 * Functions
 *
 ****************************************************************************/
#if 0
static field_name_trans_t *field_name_trans_lookup(ctp_pkt_template_t *pTemplate, char *pszAsicName)
{
    field_name_trans_t *pItem = NULL;

    TAILQ_FOREACH(pItem, &pTemplate->transHdr, entry) {
        if (!strcasecmp(pszAsicName, pItem->pszAsicName)) {
            return pItem;
        }
    }
    return NULL;
}

static int field_name_trans_add(char *pszTemplate, char *pszAsicName, char *pszCtpName,
    int offset, int bits, int findex)
{
    ctp_field_seg_t *pSegment = NULL;
    ctp_pkt_template_t *pTemplate = NULL;
    field_name_trans_t *pItem = NULL;
    int nFieldLen = 0;
    char *ppSimvFieldArr[] = {
        M_FLD_CRC,
        M_FLD_CHIP_ID,
        M_FLD_PKT_ID,
        M_FLD_SRC_CHANNAL,
        M_FLD_DST_CHANNAL,
        "Unknown"
    };
    int i;
    int bPayload = 0, bCrc = 0;

    if (NULL == pszAsicName || NULL == pszCtpName) {
        return -1;
    }

    pTemplate = pkt_template_lookup(pszTemplate);
    if (NULL == pTemplate) {
        printf("No such template: %s\n", pszTemplate);
        return -1;
    }

    pItem = field_name_trans_lookup(pTemplate, pszAsicName);
    if (NULL != pItem) {
        printf("Field translate paris exists: %s\n", pszAsicName);
        return -1;
    }

    bPayload = !strcasecmp(pszCtpName, _M_CTP_PKT_TEMPLATE_PAYLOAD_FIELD_STR);
    bCrc = !strcasecmp(pszCtpName, _M_CTP_PKT_TEMPLATE_PKTCRC_FIELD_STR);
    if (bPayload || bCrc) {
        if (bPayload) {
            nFieldLen = CTS_XR_PKT_MTU;
        } else if (bCrc) {
            nFieldLen = 32;
        }
    } else {
        pSegment = __get_pkt_seg_by_field_name(pTemplate, pszCtpName, findex);
        if (NULL == pSegment) {
            printf("No such CTP based field name: %s\n", pszCtpName);
            return -1;
        }
        nFieldLen = pSegment->length;
    }

    if (bits > nFieldLen) {
        printf("Bits overflow for field %s (%d vs %d)\n",
                pszCtpName, nFieldLen, bits);
        return -1;
    }

    pItem = XCALLOC(MTYPE_NEW_CTP_ENCODE, sizeof(field_name_trans_t));
    if (NULL == pItem) {
        return -1;
    }

    pItem->pszAsicName = XSTRDUP(MTYPE_NEW_CTP_ENCODE, pszAsicName);
    pItem->pszCtpName = XSTRDUP(MTYPE_NEW_CTP_ENCODE, pszCtpName);
    pItem->offset = offset;
    if (bits <= 0) {
        pItem->bits = nFieldLen;
    } else {
        pItem->bits = bits;
    }

    pItem->findex = findex;
    TAILQ_INSERT_TAIL(&pTemplate->transHdr, pItem, entry);

    /* update simv modified fields */
    if (bPayload || bCrc) {
        if (bCrc) {
            pTemplate->pModListArr[E_MOD_FLD_CRC] = &pTemplate->crc_mod_list;
        }
        return 0;
    }

    for (i = E_MOD_FLD_CHIP_ID; i < E_MOD_MAX; i++) {
        if (strcmp(pszAsicName, ppSimvFieldArr[i])) {
            continue;
        }

        pTemplate->pModListArr[i] = &pSegment->mod_list;
        break;
    }
    return 0;
}


/* list all CTP based field names */
static void print_ctp_based_field_names(void)
{
    printf("CTP based field names:\n");
    printf("%s\n", _M_CTP_PKT_TEMPLATE_PAYLOAD_FIELD_STR);
    printf("%s\n", _M_CTP_PKT_TEMPLATE_PKTCRC_FIELD_STR);
    ctc_pktxr_output_all_fields(fileno(stdout));
    fflush(stdout);
    return;
}
#endif


/* get current length of the packet */
static inline int
__get_pkt_cur_bytes(ctp_pkt_template_t *pTemplate)
{
    return (pTemplate->epkt.pktlen + 7) / 8;
}

ctp_pkt_template_t *pkt_template_calloc(char *pszName)
{
    ctp_pkt_template_t *pTemplate = NULL;
    pTemplate = (ctp_pkt_template_t *)XCALLOC(MTYPE_NEW_CTP_ENCODE, sizeof(ctp_pkt_template_t));

    if (NULL != pTemplate) {
        snprintf(pTemplate->name, _M_CTP_PKT_TEMPLATE_NAME_SIZE, "%s", pszName);
        pTemplate->epkt.packet = pTemplate->opkt_buf;
        TAILQ_INIT(&pTemplate->epkt.header);
        pTemplate->refcnt = 1;
        TAILQ_INIT(&pTemplate->enc_left_cmdl);
        TAILQ_INIT(&pTemplate->enc_right_cmdl);
        pTemplate->status = _M_CTP_PKT_TEMPLATE_STATUS_PKT_CFG;
        pTemplate->pld_format = E_PKT_PLD_FMT_ZERO;
        TAILQ_INIT(&pTemplate->transHdr);

        TAILQ_INIT(&pTemplate->crc_mod_list);
        TAILQ_INIT(&pTemplate->payload_mod_list);
    }

    return pTemplate;
}

#if 0
static void pkt_template_reference(ctp_pkt_template_t *pTemplate)
{
    if (NULL != pTemplate) {
        pTemplate->refcnt++;
    }
    return;
}

static pm_cb_t *_find_pm_cb(pm_cb_list_t *pList, char *pszAsciName)
{
    pm_cb_t *pItem = NULL;

    TAILQ_FOREACH(pItem, pList, entry) {
        if (!strcmp(pszAsciName, pItem->pTrans->pszAsicName)) {
            return pItem;
        }
    }

    return NULL;
}
#endif

static void _del_pm_cb_item(pm_cb_list_t *pList, pm_cb_t *pItem)
{
    TAILQ_REMOVE(pList, pItem, entry);
    if (pItem->pExtraArg && pItem->free_cb) {
        pItem->free_cb(pItem->pExtraArg);
    }

    XFREE(MTYPE_NEW_CTP_ENCODE, pItem);
    return;
}

#if 0
static void _del_pm_cb_list_bycb(pm_cb_list_t *pList, pkt_modifier_cb pCb)
{
    pm_cb_t *pItem = NULL;
    pm_cb_t *pItemDel = NULL;

    TAILQ_FOREACH(pItem, pList, entry) {
        if (pItem->cb != pCb) {
            continue;
        }

        if (pItemDel) {
            _del_pm_cb_item(pList, pItemDel);
        }

        pItemDel = pItem;
    }

    if (pItemDel) {
        _del_pm_cb_item(pList, pItemDel);
    }
    return;
}

static void _del_pm_cb_list_byname(pm_cb_list_t *pList, char *pszAsciName)
{
    pm_cb_t *pItem = NULL;

    TAILQ_FOREACH(pItem, pList, entry) {
        if (strcmp(pszAsciName, pItem->pTrans->pszAsicName)) {
            continue;
        }

        _del_pm_cb_item(pList, pItem);
        return;
    }

    return;
}
#endif

static void _destroy_pm_cb_list(pm_cb_list_t *pList)
{
    pm_cb_t *pItem = NULL;

    while (!TAILQ_EMPTY(pList)) {
        pItem = TAILQ_FIRST(pList);
        _del_pm_cb_item(pList, pItem);
    }

    return;
}

static void __pkt_modifier_free(ctp_pkt_template_t *pTemplate)
{
    ctp_header_info_t *cur_header = NULL;
    ctp_field_seg_t *cur_seg = NULL;

    TAILQ_FOREACH(cur_header, &pTemplate->epkt.header, entry) {
        TAILQ_FOREACH(cur_seg, &cur_header->seg_list, entry) {
            _destroy_pm_cb_list(&cur_seg->mod_list);
        }
    }

    _destroy_pm_cb_list(&pTemplate->crc_mod_list);
    _destroy_pm_cb_list(&pTemplate->payload_mod_list);
    return;
}
#if 0
static void __pkt_template_delete(ctp_pkt_template_t *pTemplate)
{
    field_name_trans_t *pItem = NULL;

    if (NULL == pTemplate) {
        return;
    }

    pTemplate->refcnt--;
    if (pTemplate->refcnt > 0) {
        return;
    }

    encoder_reset(pTemplate);
    if (NULL != pTemplate->entry.tqe_prev || NULL != pTemplate->entry.tqe_next) {
        TAILQ_REMOVE(&g_pkt_template_hdr, pTemplate, entry);
    }

    while (!TAILQ_EMPTY(&pTemplate->transHdr)) {
        pItem = TAILQ_FIRST(&pTemplate->transHdr);
        TAILQ_REMOVE(&pTemplate->transHdr, pItem, entry);
        XFREE(MTYPE_NEW_CTP_ENCODE, pItem->pszAsicName);
        XFREE(MTYPE_NEW_CTP_ENCODE, pItem->pszCtpName);
        XFREE(MTYPE_NEW_CTP_ENCODE, pItem);
    }
    XFREE(MTYPE_NEW_CTP_ENCODE, pTemplate);
    return;
}
#endif

static ctp_pkt_template_t *pkt_template_lookup(char *pszName)
{
    ctp_pkt_template_t *pTemplate = NULL;

    TAILQ_FOREACH(pTemplate, &g_pkt_template_hdr, entry) {
        if (!strcasecmp(pTemplate->name, pszName)) {
            return pTemplate;
        }
    }

    return NULL;
}

#if 0
static ctp_pkt_template_t *pkt_template_add(char *pszName)
{
    ctp_pkt_template_t *pTemplate = NULL;

    if (NULL != (pTemplate = pkt_template_lookup(pszName))) {
        return pTemplate;
    }

    pTemplate = pkt_template_calloc(pszName);
    if (NULL != pTemplate) {
        TAILQ_INSERT_TAIL(&g_pkt_template_hdr, pTemplate, entry);
    }
    return pTemplate;
}

static int pkt_template_delete(char *pszName)
{
    ctp_pkt_template_t *pTemplate = pkt_template_lookup(pszName);

    if (NULL == pTemplate) {
        return CTP_E_NONE;
    }

    __pkt_template_delete(pTemplate);
    return CTP_E_NONE;
}

static int pkt_template_reset(ctp_pkt_template_t *pTemplate)
{
    encoder_reset(pTemplate);
    return CTP_E_NONE;
}
#endif

/* save current global packet template configuration as template
 * and reset global packet template
 */
static int save_pkt_configuration_as_template(char *pszName)
{
    ctp_pkt_template_t *pTemplate = NULL;
    int ret = CTP_E_NONE;

    if (NULL == g_global_pkt_template || NULL == pszName
            || TAILQ_EMPTY(&g_global_pkt_template->enc_left_cmdl)) {
        CTP_OUT("Save Packet Configuration as template failed: source packet configuration is empty!\n");
        return CTP_E_EMPTY;
    }

    pTemplate = pkt_template_lookup(pszName);
    if (NULL != pTemplate) {
        CTP_OUT("Save Packet Configuration as template failed: item exists!\n");
        return CTP_E_EXISTS;
    }

    if (CTP_E_NONE != (ret = packet_encoder(0, g_global_pkt_template))) {
        CTP_OUT("Save Packet Configuration as template failed: Pre encode packet failed!\n");
        return ret;
    }

    /* rename template and insert to list */
    pTemplate = g_global_pkt_template;
    assert(_M_CTP_PKT_TEMPLATE_STATUS_PKT_CFG == pTemplate->status);
    pTemplate->status = _M_CTP_PKT_TEMPLATE_STATUS_PRE_COMPOSE;
    snprintf(pTemplate->name, _M_CTP_PKT_TEMPLATE_NAME_SIZE, "%s", pszName);
    TAILQ_INSERT_TAIL(&g_pkt_template_hdr, pTemplate, entry);

    /* Create a new global template */
    g_global_pkt_template = pkt_template_calloc(_M_CTP_PKT_TEMPALTE_GLOBAL_NAME);
    if (NULL == g_global_pkt_template) {
        CTP_OUT("WARNING: Out of memory!!!\n");
        return CTP_E_MEMORY_ERROR;
    }
    return CTP_E_NONE;
}

static char *__get_field_name_by_id(uint16_t field_id)
{
    return ctc_pktxr_get_field_info_pointer(field_id)->descriptor;
}

#if 0
static ctp_field_seg_t *
__get_pkt_seg_by_field_name(ctp_pkt_template_t *pTemplate, char *pszField, int field_idx)
{
    ctp_header_info_t *cur_header = NULL;
    ctp_field_seg_t *cur_seg = NULL;
    int field_cnt = 0;

    TAILQ_FOREACH(cur_header, &pTemplate->epkt.header, entry) {
        TAILQ_FOREACH(cur_seg, &cur_header->seg_list, entry) {
            if (!strcasecmp(pszField, __get_field_name_by_id(cur_seg->field_id))) {
                field_cnt++;
                if (field_cnt == field_idx) {
                    /* find it */
                    return cur_seg;
                }
            }
        }
    }

    return NULL;
}

/*
 * delete all modifier items
 *
 * return values:
 *   0 for success
 *   -1 for error
 *
 */
static int pkt_template_clear_modifier(char *pszTemplate)
{
    ctp_pkt_template_t *pTemplate = pkt_template_lookup(pszTemplate);

    if (NULL == pTemplate) {
        fprintf(stderr, "Register modifier failed: No such packet template\n");
        goto err_out;
    }

    __pkt_modifier_free(pTemplate);
    return 0;

err_out:
    fflush(stderr);
    return -1;
}

/*
 * delete one modifier
 *
 * return values:
 *   0 for success
 *   -1 for error
 *
 */
static int pkt_template_unregister_modifier(char *pszTemplate, char *pszAsicFieldName)
{
    ctp_pkt_template_t *pTemplate = NULL;
    field_name_trans_t *pTrans = NULL;
    ctp_field_seg_t *pSegment = NULL;
    pm_cb_list_t *pModList = NULL;
    int ret = -1;
    int bPayload = 0;
    int bPktCrc = 0;

    assert(NULL != pszTemplate);
    if (NULL == pszAsicFieldName || !strcasecmp(pszAsicFieldName, "")) {
        /* clear all modifiers for this template */
        return pkt_template_clear_modifier(pszTemplate);
    }

    pTemplate = pkt_template_lookup(pszTemplate);
    if (NULL == pTemplate) {
        fprintf(stderr, "Unregister modifier failed: No such packet template\n");
        goto err_out;
    }

    pTrans = field_name_trans_lookup(pTemplate, pszAsicFieldName);
    if (NULL == pTrans) {
        fprintf(stderr, "Unregister modifier failed: Can't find CTP field name for %s\n",
                pszAsicFieldName);
        goto err_out;
    }

    bPayload = !strcasecmp(pTrans->pszCtpName, _M_CTP_PKT_TEMPLATE_PAYLOAD_FIELD_STR);
    bPktCrc = !strcasecmp(pTrans->pszCtpName, _M_CTP_PKT_TEMPLATE_PKTCRC_FIELD_STR);
    if (bPayload || bPktCrc) {
        if (bPayload) {
            pModList = &pTemplate->payload_mod_list;
        } else {
            pModList = &pTemplate->crc_mod_list;
        }
    } else {
        pSegment = __get_pkt_seg_by_field_name(pTemplate, pTrans->pszCtpName, pTrans->findex);
        if (NULL == pSegment) {
            fprintf(stderr, "Unregister modifier failed: Can't find field '%s' index %d in this packet template\n",
                    pTrans->pszAsicName, pTrans->findex);
            goto err_out;
        }

        pModList = &pSegment->mod_list;
    }

    _del_pm_cb_list_byname(pModList, pszAsicFieldName);

#if 0
    fprintf(stdout, "Unregister modifier for field %s -> %s success\n",
            pTrans->pszAsicName, pTrans->pszCtpName);
#endif
    ret = 0;

err_out:
    fflush(stderr);
    return ret;
}
#endif



static uint32 __pkt_template_rand(void)
{
    return (uint32)((float)RAND_MAX * rand() / (RAND_MAX + 1.0));
}

/* return 0 for success
 * return -1 for fail
 */
static int __pkt_template_fill_payload(ctp_pkt_template_t *pTemplate, int frame_size)
{
    int payload_bytes;
    int payload_offset;
    int copy_len = 0;
    int pattern_len = 0;
    char *pStart = NULL;
    char *pPayload = NULL;
    int ret = -1;
    int idx;

    assert(frame_size > 4);
    assert(NULL != pTemplate);

    payload_offset = __get_pkt_cur_bytes(pTemplate) - 2;
    payload_bytes = frame_size - 4 - payload_offset;
    if (payload_bytes <= 0) {
        goto err_out;
    }

    if (E_PKT_PLD_FMT_FIXED == pTemplate->pld_format
        || E_PKT_PLD_FMT_REPEAT == pTemplate->pld_format) {
        if (pTemplate->pld_pattern) {
            pattern_len = (strlen(pTemplate->pld_pattern) + 1) / 2;
            pPayload = XMALLOC(MTYPE_NEW_CTP_ENCODE, pattern_len);
            ctc_pktxr_string2char(pTemplate->pld_pattern, (uint8*)pPayload);
        }
    }
    pStart = (char*)pTemplate->epkt.packet + payload_offset;
    switch (pTemplate->pld_format) {
    case E_PKT_PLD_FMT_ZERO:
        memset(pStart, 0x00, payload_bytes);
        break;
    case E_PKT_PLD_FMT_FULL:
        memset(pStart, 0xFF, payload_bytes);
        break;
    case E_PKT_PLD_FMT_FIXED:
        if (pPayload) {
            copy_len = min(payload_bytes, pattern_len);
            memcpy(pStart, pPayload, copy_len);
        }
        break;
    case E_PKT_PLD_FMT_REPEAT:
        if (pPayload) {
            int left_len = payload_bytes;
            idx = 0;
            while (left_len > 0) {
                copy_len = min(left_len, pattern_len);
                memcpy(pStart + idx, pPayload, copy_len);
                left_len -= copy_len;
                if (left_len <= 0) {
                    break;
                }
                idx += pattern_len;
            }
        }
        break;
    case E_PKT_PLD_FMT_INC_BYTE:
        {
            uint8 byteVal = 0;
            for (idx = 0; idx < payload_bytes; idx++, byteVal++) {
                *(pStart + idx) = byteVal;
            }
        }
        break;
    case E_PKT_PLD_FMT_INC_WORD:
        {
            uint16 wordVal = 0;
            if (payload_bytes >= 2) {
                for (idx = 0; idx < payload_bytes; idx += sizeof(uint16), wordVal++) {
                    *((uint16 *)(pStart + idx)) = htons(wordVal);
                }

                if (payload_bytes % 2) {
                    *((uint16 *)(pStart + idx - 1)) = htons(wordVal);
                }
            } else {
                /* only 1 byte */
                *pStart = 0x00;
            }
        }
        break;
    case E_PKT_PLD_FMT_RANDOM:
        {
            uint32 dwordVal = 0;
            int left_bytes = payload_bytes % 4;
            struct timeval rtv;
            gettimeofday(&rtv, NULL);
            srand(rtv.tv_usec); /* call this to change random numbers */
            if (payload_bytes >= 4) {
                for (idx = 0; idx < payload_bytes; idx += sizeof(uint32)) {
                    dwordVal = __pkt_template_rand();
                    *((uint32 *)(pStart + idx)) = dwordVal;
                }

                if (left_bytes) {
                    int left_offset = idx - sizeof(uint32) + 1;
                    dwordVal = __pkt_template_rand();
                    for (idx = 0; idx < left_bytes; idx++) {
                        *(pStart + left_offset + idx) = *((uint8 *)(&dwordVal) + idx);
                    }
                }
            } else {
                dwordVal = __pkt_template_rand();
                for (idx = 0; idx < left_bytes; idx++) {
                    *(pStart + idx) = *((uint8 *)(&dwordVal) + idx);
                }
            }
        }
        break;
    default:
        assert(0);
        goto err_out;
    }

    if (!TAILQ_EMPTY(&pTemplate->payload_mod_list)) {
        unsigned char szModBuf[M_MAXPKT_BUFFER_SIZE];
        int modoffset = 0;
        int modlen = 0;
        pm_cb_t *pCb = NULL;

        TAILQ_FOREACH(pCb, &pTemplate->payload_mod_list, entry) {
            if (0 != __pkt_template_get_modified_data(pTemplate->name,
                        pCb, &modoffset, &modlen, szModBuf)) {
                CTP_OUT("Packet template execute packet payload modifier failed\n");
                goto err_out;
            }

            if ((modoffset + modlen) <= (payload_bytes * 8)) {
                copy_len = modlen - 1;
            } else {
                /* overflow */
                copy_len = (payload_bytes * 8) - modoffset - 1;
            }
            ctc_pktxr_set_field_bit((uint8*)pStart, modoffset, modoffset + copy_len, (uint8*)szModBuf);
        }
    }

    pTemplate->epkt.pktlen += payload_bytes * 8;
    ret = 0;

err_out:
    if (NULL != pPayload) {
        XFREE(MTYPE_NEW_CTP_ENCODE, pPayload);
    }
    return ret;
}

/* return 0 for success
 * return -1 for fail
 */
static int pkt_template_set_payload_pattern(char *pszTemplate, char *pszPattern)
{
    ctp_pkt_template_t *pTemplate = NULL;

    if (NULL == pszTemplate) {
        pTemplate = g_global_pkt_template;
    } else {
        pTemplate = pkt_template_lookup(pszTemplate);
        if (NULL == pTemplate) {
            fprintf(stderr, "pkt_template_set_payload_pattern: No such template '%s'\n",
                    pszTemplate);
            goto err_out;
        }
    }

    if (pTemplate->pld_pattern) {
        XFREE(MTYPE_NEW_CTP_ENCODE, pTemplate->pld_pattern);
    }
    pTemplate->pld_pattern = XSTRDUP(MTYPE_NEW_CTP_ENCODE, pszPattern);
    if (NULL == pTemplate->pld_pattern) {
        fprintf(stderr, "Save payload pattern failed: Out of memory!\n");
        goto err_out;
    }

    return 0;

err_out:
    fflush(stderr);
    return -1;
}

#if 0
/* calc crc or not */
static int pkt_template_set_crc_option(char *pszTemplate, int bEnable)
{
    ctp_pkt_template_t *pTemplate = NULL;

    if (NULL == pszTemplate) {
        pTemplate = g_global_pkt_template;
    } else {
        pTemplate = pkt_template_lookup(pszTemplate);
        if (NULL == pTemplate) {
            fprintf(stderr, "No such template '%s'\n",
                    pszTemplate);
            goto err_out;
        }
    }

    pTemplate->enc_cw[PKT_CRC] = bEnable;
    return 0;

err_out:
    fflush(stderr);
    return -1;
}
#endif
/* return 0 for success
 * return -1 for fail
 */
static int pkt_template_set_payload_format(char *pszTemplate, payload_format_e eFormat)
{
    ctp_pkt_template_t *pTemplate = NULL;

    if (NULL == pszTemplate) {
        pTemplate = g_global_pkt_template;
    } else {
        pTemplate = pkt_template_lookup(pszTemplate);
        if (NULL == pTemplate) {
            fprintf(stderr, "No such template '%s'\n",
                    pszTemplate);
            goto err_out;
        }
    }

    if (eFormat > E_PKT_PLD_FMT_RANDOM) {
        eFormat = E_PKT_PLD_FMT_ZERO;
    }
    pTemplate->pld_format = eFormat;
    return 0;

err_out:
    fflush(stderr);
    return -1;
}

#if 0
/*
 * compose to get a packet from template
 * return values:
 * 0  for success
 * -1 for failed
 *
 * frame_size:
 *   modify the whole packet length,
 *   0 for not modify packet length
 *   will ignore it if this value is less than packet body
 */
static int pkt_template_compose(char *pszTemplate, unsigned char **opkt_buf, int *opkt_len, int frame_size)
{
    ctp_pkt_template_t *pTemplate = NULL;
    int ret = 0;

    pTemplate = pkt_template_lookup(pszTemplate);
    if (NULL == pTemplate) {
        fprintf(stderr, "Compose packet from template failed: No such template '%s'\n",
                pszTemplate);
        goto err_out;
    }

    ret = packet_compose(pTemplate, frame_size);
    if (CTP_E_NONE != ret) {
        fprintf(stderr, "Compose packet from template failed.\n");
        goto err_out;
    }

    *opkt_buf = pTemplate->opkt_buf;
    *opkt_len = (pTemplate->epkt.pktlen  + 1) / 8;
    return 0;

err_out:
    fflush(stderr);
    return -1;
}

static int save_mod_related_vars(ctp_pkt_template_t *pTemplate,
    pkt_mod_param_t *pMod, pkt_mod_saved_data_t *pSaved)
{
    pSaved->pszPayloadPattern = pTemplate->pld_pattern;
    pSaved->ePayloadType = pTemplate->pld_format;
    return 0;
}

/* callback function for fields in pkt_mod_field_e */
static int field_mod_callback(char *pszTemplate, char *pszField, int offset, int bits,
    unsigned char *pRetValue, int *pnRetLen, void *pArg)
{
    char szBuf[128];
    int32_t *pnValue = (int32_t *)pArg;

#if 0
    printf("field_mod_callback: %s %d %d 0x%X\n",
        pszField, offset, bits, *pnValue);
#endif
    if (bits > 32) {
        fprintf(stderr,
            "WARNING: field_mod_callback may fail for field %s bits %d > 32\n",
            pszField, bits);
    }

    snprintf(szBuf, 128, "%x", *pnValue);
    if (strlen(szBuf) % 2) {
        snprintf(szBuf, 128, "0%x", *pnValue);
    }

    memset(pRetValue, 0x00, (bits + 7) / 8); /* XXX: erase some more bits, but who care */
    ctc_pktxr_string2char(szBuf, pRetValue + ((bits + 7)/ 8) - (strlen(szBuf) + 1)/2);
    *pnRetLen = bits;
    return 0;
}

static void __try_delete_tmp_modifier(pm_cb_list_t *pList)
{
    if (NULL == pList) {
        return;
    }

    _del_pm_cb_list_bycb(pList, field_mod_callback);
    return;
}

static int __create_tmp_modifier(ctp_pkt_template_t *pTemplate,
    pkt_mod_field_e eType, char *pszAsicName, int32_t *pnValue)
{
    static field_name_trans_t stFakeCrcTrans = {
        .pszAsicName = M_FLD_CRC,
        .pszCtpName = _M_CTP_PKT_TEMPLATE_PKTCRC_FIELD_STR,
        .offset = 0,
        .bits = 32,
        .findex = 1
    };
    pm_cb_t *pCb = NULL;
    field_name_trans_t *pTrans = NULL;

    if (*pnValue >= 0) {
        /* need modify chip id */
        if (!pTemplate->pModListArr[eType]) {
            fprintf(stderr, "generate_pkt: '%s' undefined in template '%s'\n",
                    pszAsicName, pTemplate->name);
            return -1;
        }

        pTrans = field_name_trans_lookup(pTemplate, pszAsicName);
        if (NULL == pTrans && E_MOD_FLD_CRC == eType){
            pTrans = &stFakeCrcTrans;
        }

        pCb = XCALLOC(MTYPE_NEW_CTP_ENCODE, sizeof(pm_cb_t));
        pCb->pTrans = pTrans;
        pCb->cb = field_mod_callback;
        pCb->pExtraArg = pnValue;
        TAILQ_INSERT_TAIL(pTemplate->pModListArr[eType], pCb, entry);
    }

    return 0;
}

static int generate_tmp_modifiers(ctp_pkt_template_t *pTemplate,
    pkt_mod_param_t *pMod, pkt_mod_saved_data_t *pSaved)
{
    int nRet = -1;

    if (pMod->nCrc > 0
            && __create_tmp_modifier(pTemplate, E_MOD_FLD_CRC,
                M_FLD_CRC, &pMod->nCrc) != 0) {
        goto err_out;
    }

    if (0 != __create_tmp_modifier(pTemplate, E_MOD_FLD_CHIP_ID,
                    M_FLD_CHIP_ID, &pMod->nChipId)) {
        goto err_out;
    }

    if (0 != __create_tmp_modifier(pTemplate, E_MOD_FLD_PKT_ID,
                    M_FLD_PKT_ID, &pMod->nPktId)) {
        goto err_out;
    }

    if (0 != __create_tmp_modifier(pTemplate, E_MOD_FLD_SRC_CHANNAL,
                    M_FLD_SRC_CHANNAL, &pMod->nSrcChannal)) {
        goto err_out;
    }

    if (0 != __create_tmp_modifier(pTemplate, E_MOD_FLD_DST_CHANNAL,
                    M_FLD_DST_CHANNAL, &pMod->nDstChannal)) {
        goto err_out;
    }

    pTemplate->pld_pattern = pMod->pszPayload;
    pTemplate->pld_format = pMod->ePayloadType;
    nRet = 0;

err_out:

    return nRet;
}

static int restore_mod_related_vars(ctp_pkt_template_t *pTemplate,
    pkt_mod_param_t *pMod, pkt_mod_saved_data_t *pSaved)
{
    int i;

    for (i = E_MOD_FLD_CRC; i < E_MOD_MAX; i++) {
        if (pTemplate->pModListArr[i]) {
            __try_delete_tmp_modifier(pTemplate->pModListArr[i]);
        }
    }

    pTemplate->pld_pattern = pSaved->pszPayloadPattern;
    pTemplate->pld_format = pSaved->ePayloadType;
    return 0;
}

static int generate_pkt(pkt_mod_param_t *pMod, unsigned char *opkt_buf, int *opkt_len)
{
    ctp_pkt_template_t *pTemplate = NULL;
    pkt_mod_saved_data_t stSavedData;
    int nRes = -1;
    int nLen = 0;
    int nRet = -1;

    pTemplate = pkt_template_lookup(pMod->pszTemplate);
    if (NULL == pTemplate) {
        fprintf(stderr, "generate_pkt: No such template '%s'\n",
                pMod->pszTemplate);
        goto err_out;
    }

    memset(&stSavedData, 0x00, sizeof(pkt_mod_saved_data_t));
    save_mod_related_vars(pTemplate, pMod, &stSavedData);
    if (generate_tmp_modifiers(pTemplate, pMod, &stSavedData) != 0) {
        goto err_out;
    }

    nRes = packet_compose(pTemplate, pMod->nFrameSize);
    if (CTP_E_NONE != nRes) {
        fprintf(stderr, "generate_pkt: perform encode failed\n");
        goto err_out;
    }

    nLen = (pTemplate->epkt.pktlen  + 1) / 8;
    memcpy(opkt_buf, pTemplate->opkt_buf, nLen);
    *opkt_len = nLen;
    nRet = 0;

err_out:
    fflush(stderr);
    restore_mod_related_vars(pTemplate, pMod, &stSavedData);
    return nRet;
}
#endif

/*
 * compose to get a packet from template
 * return values:
 * 0  for success
 * -1 for failed
 * the call will supply pmodbuf buffer
 */
static int __pkt_template_get_modified_data(char *pszTemplate, pm_cb_t *pItem,
    int *pmodoffset, int *pmodlen, uint8 *pmodbuf)
{
    int ret = -1;
    int len = 0;

    assert(NULL != pItem->cb);
    if (0 != pItem->cb(pszTemplate, pItem->pTrans->pszAsicName,
                        pItem->pTrans->offset, pItem->pTrans->bits,
                        pmodbuf, &len, pItem->pExtraArg)) {
        fprintf(stderr, "Execute callback function for template %s field %s failed\n",
                pszTemplate, pItem->pTrans->pszAsicName);
        goto err_out;
    }

    /* check returned values */
    if (len < 1 || len < pItem->pTrans->bits) {
        fprintf(stderr, "Return value from callback too short: return len %d, modifier bits %d\n",
                len, pItem->pTrans->bits);
        goto err_out;
    }

    *pmodlen = len; /* bits length */
    *pmodoffset = pItem->pTrans->offset;
    ret = 0;

err_out:
    fflush(stderr);
    return ret;
}

/*******************************************************************************
 * Name   : encoder_next_protocol_handle
 * Purpose: hand the set next protocol flags.
 * Input  : None.
 * Output : None.
 * Return : .
 * Note   : N/A
*******************************************************************************/
static void
encoder_next_protocol_handle(ctp_field_info_t  *field_info,
                            ctp_field_seg_hdr_t *field_seg,
                            ctp_pkt_template_t *pTemplate)
{
    int idx = 0;

    for(;;)
    {
        int               logic = 0; 
		int               logic_store = 0;
        ctp_flags_nextp_t *nextp = NULL;
        char              *exp = NULL;
        uint16            flag_store = 0;

        nextp = &field_info->nextp[idx];
        exp   = nextp->exp;
        logic = ctc_pktxr_logical_expression_calculator(exp, field_seg, pTemplate->enc_cw);

        if(0 == idx)
        {
            logic_store = logic;
        }
        else
        {
            if(flag_store & OP_FLAG_AND_NEXT)
            {
                logic_store = logic_store && logic;
            }
            else
            {
                logic_store = logic_store || logic;
            }
        }

        if(nextp->op_flags & OP_FLAG_STAND_ONLY)
        {
            if(logic_store)
            {
                pTemplate->enc_cw[NEXT_TYPE] = nextp->protocol;
                break;
            }
            else
            {
                if(!nextp->next)
                {
                    break;
                }

            idx ++;
            flag_store = nextp->op_flags;
            }
        }

    }

}

/*******************************************************************************
 * Name   : encoder_jump_handle
 * Purpose: hand the set jump flags.
 * Input  : None.
 * Output : None.
 * Return : .
 * Note   : N/A
*******************************************************************************/
static uint8
encoder_jump_handle(ctp_field_info_t  *field_info,
                               ctp_field_seg_hdr_t *field_seg, uint16 *jump_id,
                               ctp_pkt_template_t *pTemplate)
{
    int               idx = 0;

    for(;;)
    {
        int               logic = 0; 
		int               logic_store = 0;
        ctp_flags_jump_t  *jump = NULL;
        char              *exp = NULL;
        uint16            flag_store = 0;

        jump = &field_info->jump[idx];
        exp   = jump->exp;
        logic = ctc_pktxr_logical_expression_calculator(exp, field_seg, pTemplate->enc_cw);

        if(0 == idx)
        {
            logic_store = logic;
        }
        else
        {
            if(flag_store & OP_FLAG_AND_NEXT)
            {
                logic_store = logic_store && logic;
            }
            else
            {
                logic_store = logic_store || logic;
            }
        }

        if(jump->op_flags & OP_FLAG_STAND_ONLY)
        {
            if(logic_store)
            {
                *jump_id = jump->field_id;
                if(jump->op_flags & OP_FLAG_DEC_CNT)
                {
                    pTemplate->enc_cw[REPEAT] --;
                }
                return 1;
            }
            else
            {
                if(!jump->next)
                {
                    break;
                }

            idx ++;
            flag_store = jump->op_flags;
            }

        }
    }

    return 0;
}


/*******************************************************************************
 * Name   : encoder_length_handle
 * Purpose: hand the set length flags.
 * Input  : None.
 * Output : None.
 * Return : stored header type string's pointer.
 * Note   : N/A
*******************************************************************************/
static void
encoder_length_handle(ctp_field_info_t  *field_info, ctp_field_seg_hdr_t *fiedl_seg,
        ctp_pkt_template_t *pTemplate)
{
    ctp_flags_len_t    *length;
    char               *exp;
    uint32             len;

    length = field_info->length;

    if(NULL == length)
    {
        CTP_OUT("LENGTH_HANDLE_POINTER NOT EXIST, FIELD = %s\n",
                field_info->descriptor);
        return;
    }

    exp = length->exp;
    if(NULL == exp)
    {
        CTP_OUT("LENGTH_EXPRESSION_POINTER NOT EXIST, FIELD = %s\n",
                field_info->descriptor);
        return;
    }

    ctc_pktxr_math_expression_calculator(exp, fiedl_seg, pTemplate->enc_cw, &len);
    pTemplate->enc_cw[FLEN_FID] = length->field_id;
    pTemplate->enc_cw[FIEL_LEN] = (uint16)len;
}

 /*******************************************************************************
 * Name   : encoder_repeat_handle
 * Purpose: hand the set repeat counter flags.
 * Input  : None.
 * Output : None.
 * Return : stored header type string's pointer.
 * Note   : N/A
*******************************************************************************/
static void
encoder_repeat_handle(ctp_field_info_t  *field_info, ctp_field_seg_hdr_t *fiedl_seg,
        ctp_pkt_template_t *pTemplate)
{
    ctp_flags_repeat_t *repeat;
    char               *exp;
    uint32             rep_cnt;

    repeat = field_info->repeat;

    if(NULL == repeat)
    {
        CTP_OUT("REPEAT_HANDLE_POINTER NOT EXIST, FIELD = %s\n",
                field_info->descriptor);
        return;
    }

    exp = repeat->exp;
    if(NULL == exp)
    {
        CTP_OUT("REPEAT_EXPRESSION_POINTER NOT EXIST, FIELD = %s\n",
                field_info->descriptor);
        return;
    }

    ctc_pktxr_math_expression_calculator(exp, fiedl_seg, pTemplate->enc_cw, &rep_cnt);
    pTemplate->enc_cw[REPEAT] = (uint16)rep_cnt ;
    #if 0
    printf("======FIELD=%s======REPCOUNT=%d===========\n",field_info->descriptor,pTemplate->enc_cw[REPEAT]);
    #endif
}

 /*******************************************************************************
 * Name   : encoder_special_field_handle
 * Purpose: hand the special field such as checksum or payload_length.
 * Input  : None.
 * Output : None.
 * Return : stored header type string's pointer.
 * Note   : N/A
*******************************************************************************/
ctp_error_e
encoder_special_field_handle(ctp_field_info_t *field_info, char *str, uint16 num, uint16 len, uint16 oldlen ,ctp_field_seg_hdr_t *field_seg)
 {
     ctp_field_seg_t *temp_seg = NULL;

     temp_seg = (ctp_field_seg_t *) XCALLOC(MTYPE_NEW_CTP_ENCODE,sizeof(ctp_field_seg_t));
     if (NULL == temp_seg)
     {
         return CTP_E_MEMORY_ERROR;
     }

     temp_seg->field_id = num;
     temp_seg->length = len;
     temp_seg->old_len = oldlen;
     temp_seg->pszSpecial = (uint8*)XSTRDUP(MTYPE_NEW_CTP_ENCODE, str);
     temp_seg->value = (uint8*)XSTRDUP(MTYPE_NEW_CTP_ENCODE,str);
     TAILQ_INIT(&temp_seg->mod_list);
     TAILQ_INSERT_TAIL(field_seg, temp_seg, entry);

     return CTP_E_NONE;
 }

 /*******************************************************************************
 * Name   : packet_compose_special_field_handle
 * Purpose: hand the special field such as checksum or payload_length.
 * Input  : None.
 * Output : None.
 * Return : stored header type string's pointer.
 * Note   : N/A
*******************************************************************************/
static ctp_error_e
packet_compose_special_field_handle(int *exist, uint8 **pos, uint16 *byte_len,
        ctp_field_seg_t *field_seg, int start,
        ctp_pkt_template_t *pTemplate)
{
    uint8 *value = NULL;
    char *pszDefault = "00";

    *exist = 1;
    *pos = &(pTemplate->epkt.packet[(start +7)/8]);
    value = (uint8 *) XMALLOC (MTYPE_NEW_CTP_ENCODE,(field_seg->length + 7)/ 8);
    if (NULL == value)
    {
        return CTP_E_MEMORY_ERROR;
    }

    memset(value, 0, ((field_seg->length + 7)/ 8));
    ctc_pktxr_string2char(pszDefault, value + ((field_seg->length + 7)/ 8)
                          - (strlen(pszDefault) + 1)/2);
    *byte_len = (field_seg->length)/8;
    if (NULL != field_seg->value)
    {
        XFREE(MTYPE_NEW_CTP_ENCODE,field_seg->value);
    }
    field_seg->value = value;

    return CTP_E_NONE;

 }
/*******************************************************************************
 * Name   : packet_compose
 * Purpose: compose packet to UIBT8 forat.
 * Input  : None.
 * Output : None.
 * Return : .
 * Note   : N/A
*******************************************************************************/
static ctp_error_e
packet_compose(ctp_pkt_template_t *pTemplate, int frame_size)
{
    pm_cb_t *pCb = NULL;
    int start = 0, end = 0;
    ctp_header_info_t *curr_header = NULL;
    /*uint16 type = 0;*/
    ctp_field_seg_t *curr_seg = NULL;
    uint32 crc = 0;
    int modlen = 0;
    int modoffset = 0;

    unsigned char szModBuf[M_MAXPKT_BUFFER_SIZE];
    DelayProcParamList_t delayProcParamList = TAILQ_HEAD_INITIALIZER(delayProcParamList);
    DelayProcParam_t *pDPPItem = NULL;
    unsigned char *pDelayOffset = NULL;
    u_int16_t nDelayProcLen = 0;
    int bDelayExist = 0;
    int nRet = CTP_E_NONE;

    //add by qiuping, clear the entire packet, avoid affect next packet. :fix udp checksum bug.
    memset(pTemplate->epkt.packet,0,M_MAXPKT_BUFFER_SIZE);

    /* per field segment handle */
    /*type = pTemplate->epkt.type;*/
    TAILQ_FOREACH(curr_header, &pTemplate->epkt.header, entry)
    {
        int store_offset = 0;
        DelayProcField_t *pDelayProcField = NULL;

        store_offset = start;

        TAILQ_FOREACH(curr_seg, &curr_header->seg_list, entry)
        {
            if (NULL != curr_seg->pszSpecial) {
                pDelayProcField = find_delay_proc_field((char*)curr_seg->pszSpecial);
                if (NULL != pDelayProcField) {
                    nRet = packet_compose_special_field_handle(&bDelayExist, &pDelayOffset,
                                &nDelayProcLen, curr_seg, start, pTemplate);
                    if (CTP_E_NONE != nRet) {
                        printf("Handle the field %s failed\n", curr_seg->pszSpecial);
                        goto err_out;
                    }

                    if (!find_delay_proc_param(&delayProcParamList, (char*)curr_seg->pszSpecial)) {
                        //printf("Too many %s field\n", curr_seg->pszSpecial);
                        //nRet = CTP_E_MODIFY_FAILED;
                        //goto err_out;
                        if (add_delay_proc_param(&delayProcParamList, pDelayProcField,
                            pDelayOffset, nDelayProcLen, store_offset, start,
                            &curr_seg->mod_list) != 0) {
                        printf("Proc %s field failed: Out of memory\n", curr_seg->pszSpecial);
                        nRet = CTP_E_MODIFY_FAILED;
                        goto err_out;
                    }
                  }

                }
            }

            /* since the field length is 0, like MAX_VNUM, not set to the packet */
            if (0 == curr_seg->length)
            {
                continue;
            }

            end  = start + curr_seg->length - 1;

            if((end/8) >= pTemplate->epkt.length)
            {
                printf("Packet too large when perform encode\n");
                nRet = CTP_E_LESS_LEN;
                goto err_out;
            }

            ctc_pktxr_set_field_bit(pTemplate->epkt.packet, start, end, curr_seg->value);
            if (!TAILQ_EMPTY(&curr_seg->mod_list)) {
                TAILQ_FOREACH(pCb, &curr_seg->mod_list, entry) {
                    if (0 != __pkt_template_get_modified_data(pTemplate->name,
                                pCb, &modoffset, &modlen, szModBuf)) {
                        CTP_OUT("Packet template execute modifier for field %s failed\n",
                            __get_field_name_by_id(curr_seg->field_id));
                        nRet = CTP_E_MODIFY_FAILED;
                        goto err_out;
                    }

                    if ((modoffset + modlen) >= curr_seg->length) {
                        /* overflow field length */
                        ctc_pktxr_set_field_bit(pTemplate->epkt.packet, start + modoffset,
                                      end, szModBuf);
                    } else {
                        ctc_pktxr_set_field_bit(pTemplate->epkt.packet, start + modoffset,
                                      start + modoffset + modlen - 1, szModBuf);
                    }
                }
            }
            pTemplate->epkt.pktlen = end;

            /* for next segment */
            start = end + 1;
        }

        TAILQ_FOREACH(pDPPItem, &delayProcParamList, entry) {
            if (pDPPItem->pField->bGlobal || !pDPPItem->pField->procCB) {
                continue;
            }

            if (pDPPItem->pField->procCB(pTemplate, pDPPItem, start) != 0) {
                printf("Process field %s failed\n", pDPPItem->pField->pszName);
                nRet = CTP_E_MODIFY_FAILED;
                goto err_out;
            }
        }
        destroy_delay_proc_param_list(&delayProcParamList, 0); /* delete none global item */

        /* assign new type of next header */
        /*type  = curr_header->next_type;*/
    }

    TAILQ_FOREACH(pDPPItem, &delayProcParamList, entry) {
        if (!pDPPItem->pField->bGlobal || !pDPPItem->pField->procCB) {
            continue;
        }

        if (pDPPItem->pField->procCB(pTemplate, pDPPItem, start) != 0) {
            printf("Process field %s failed\n", pDPPItem->pField->pszName);
            nRet = CTP_E_MODIFY_FAILED;
            goto err_out;
        }
    }

    if (0 == frame_size) {
        frame_size = pTemplate->enc_cw[PKT_LEN];
    }
    if (frame_size >= M_MAXPKT_BUFFER_SIZE) {
        CTP_OUT("Frame size %d too large error!\n", frame_size);
        nRet = CTP_E_MODIFY_FAILED;
        goto err_out;
    }

    /* 4 bytes is CRC field length */
    if ((frame_size - 4) > __get_pkt_cur_bytes(pTemplate)) {
        /* let's fill payload string */
        __pkt_template_fill_payload(pTemplate, frame_size);
    }

    /* Modified by Alexander Liu 2012-10-23 09:05 */
    if (pTemplate->enc_cw[PKT_CRC]) {
        if ((__get_pkt_cur_bytes(pTemplate) + 4) >= M_MAXPKT_BUFFER_SIZE) {
            CTP_OUT("Packet too large error: Can't hold the packet CRC field.\n");
            nRet = CTP_E_MODIFY_FAILED;
            goto err_out;
        }

        /* need calc the whole packet's CRC */
        if (TAILQ_EMPTY(&pTemplate->crc_mod_list)) {
            crc = htonl(ctc_pktxr_ctp_crc32(0xFFFFFFFF, pTemplate->epkt.packet,
                        __get_pkt_cur_bytes(pTemplate)));
            memcpy(pTemplate->epkt.packet + __get_pkt_cur_bytes(pTemplate),
                   &crc, sizeof(uint32));
        } else {
            TAILQ_FOREACH(pCb, &pTemplate->crc_mod_list, entry) {
                if (0 != __pkt_template_get_modified_data(pTemplate->name,
                            pCb, &modoffset, &modlen, szModBuf)) {
                    CTP_OUT("Packet template execute packet CRC modifier failed\n");
                    nRet = CTP_E_MODIFY_FAILED;
                    goto err_out;
                }

                start = pTemplate->epkt.pktlen + 1;
                end = start + 31; /* 4 bytes crc field length */
                if (0 == modoffset && modlen == 32) {
                    /* replace the whole field */
                    ctc_pktxr_set_field_bit(pTemplate->epkt.packet, start, end, szModBuf);
                } else {
                    /* XXX: replace some bits of the field. FIXME */
                    crc = htonl(ctc_pktxr_ctp_crc32(0xFFFFFFFF, pTemplate->epkt.packet,
                                __get_pkt_cur_bytes(pTemplate)));
                    memcpy(pTemplate->epkt.packet + __get_pkt_cur_bytes(pTemplate),
                           &crc, sizeof(uint32));
                    if ((modoffset + modlen) >= 32) {
                        /* overflow field length */
                        ctc_pktxr_set_field_bit(pTemplate->epkt.packet, start + modoffset,
                                      end, szModBuf);
                    } else {
                        ctc_pktxr_set_field_bit(pTemplate->epkt.packet, start + modoffset,
                                      start + modoffset + modlen - 1, szModBuf);
                    }
                }
            }
        }

        /* packet crc field at the end is 4 bytes */
        pTemplate->epkt.pktlen += 32;
    }

    nRet = CTP_E_NONE;

err_out:
    destroy_delay_proc_param_list(&delayProcParamList, 1);
    return nRet;
}

/*******************************************************************************
 * Name   : packet_encoder
 * Purpose: Get the pointer of stored header type string.
 * Input  : None.
 * Output : None.
 * Return : stored header type string's pointer.
 * Note   : N/A
*******************************************************************************/
static ctp_error_e
packet_encoder(int bCompose, ctp_pkt_template_t *ptr)
{
    ctp_pkt_template_t *pTemplate = ptr ? ptr : g_global_pkt_template;
    int               ret, found;
    uint16            field_id, flen,flen_old, jump_id; // jump_id is the jump repeat place
    ctp_field_info_t  *field_info = NULL;
    ctp_cmd_t         *cmd = NULL;
    ctp_header_info_t *curr_header = NULL;
    /*ctp_field_seg_t   *curr_seg = NULL;*/
    uint8             *value = NULL;
    uint16            nOldCrc = 0;

    ctc_pktxr_set_search_sequence(E_SEARCH_REVERSE);
    nOldCrc = pTemplate->enc_cw[PKT_CRC];
    memset(pTemplate->enc_cw, 0, CONTROL_WORD_NUM * sizeof(uint16));
    pTemplate->enc_cw[PKT_CRC] = nOldCrc;
    pTemplate->enc_cw[NEXT_TYPE] = HEADER_TYPE_NUM;
    pTemplate->enc_cw[FLEN_FID]  = FIELD_NUM;
    CTP_ERR_RETURN(ctc_pktxr_control_word_handle(&pTemplate->enc_left_cmdl, &pTemplate->enc_right_cmdl,
                                        pTemplate->enc_cw), "encoder control_word_handle");

    memset(&pTemplate->epkt, 0, sizeof(ctp_pkt_info_t));
    pTemplate->epkt.packet = pTemplate->opkt_buf;
    pTemplate->epkt.type = pTemplate->enc_cw[PKT_TYPE];
    pTemplate->epkt.length = pTemplate->enc_cw[PKT_LEN];
    pTemplate->pkt_base_bits = 0;
    /* free old header */
    encode_free_header(pTemplate);
    TAILQ_INIT(&pTemplate->epkt.header);
    curr_header = XCALLOC(MTYPE_NEW_CTP_ENCODE, sizeof(ctp_header_info_t));
    if (NULL == curr_header)
    {
        return CTP_E_MEMORY_ERROR;
    }
    TAILQ_INSERT_TAIL(&pTemplate->epkt.header, curr_header, entry);
    TAILQ_INIT(&curr_header->seg_list);
    curr_header->next_type = HEADER_TYPE_NUM;

    /* get rthe first field  */
    field_id = ctc_pktxr_get_header_start_field(pTemplate->enc_cw[PKT_TYPE]);

    /* per field handle */
    while (NULL_FIELD != field_id)
    {
        field_info = ctc_pktxr_get_field_info_pointer(field_id);

        /* detemind field length */
        found = ctc_pktxr_length_lookup(pTemplate->enc_cw, field_id, &flen);
        flen = found? flen : field_info->field_len;
        flen_old = field_info->field_len;

        /* Exist in command list handle */
        found = ctc_pktxr_cmd_lookup(&pTemplate->enc_left_cmdl, &pTemplate->enc_right_cmdl,
                                     field_info->descriptor, &cmd);

#if 1
       if(field_info)
        {
        //printf("Field->descriptor:%s\n",field_info->descriptor);
       // printf("Field->descriptor:%s   keyword:%s     value:%s\n",field_info->descriptor, cmd->keyword,cmd->value);
        }
#endif

        if (strstr(field_info->descriptor, "HEAD_PAYLOAD_LEN") != NULL)
        {
            encoder_special_field_handle(field_info, "HEAD_PAYLOAD_LEN", field_id, flen,flen_old, &curr_header->seg_list);
        }

        if (strstr(field_info->descriptor, "CHKSM") != NULL && !found)
        {
            int rs = 0;
            pTemplate->pkt_base_bits += flen;
            rs = encoder_special_field_handle(field_info, "CHKSM", field_id, flen,flen_old,&curr_header->seg_list);
            if (CTP_E_NONE != rs)
            {
                printf("handle the checksum failed\n");
                return rs;
            }

            jump_id = field_id + 1;
            goto NEXT_FIELD;

        }

        if (strstr(field_info->descriptor, "IPV4_LEN") != NULL && !found)
        {
            int rs = 0;
            pTemplate->pkt_base_bits += flen;
            encoder_special_field_handle(field_info, "IPV4_LEN", field_id, flen, flen_old,&curr_header->seg_list);
            if (CTP_E_NONE != rs)
            {
                printf("handle the ipv4 length failed\n");
                return rs;
            }

            jump_id = field_id + 1;
            goto NEXT_FIELD;
        }

        if (strstr(field_info->descriptor, "ICMPV6_CHECKSUM") != NULL)
        {
            int rs = 0;
            pTemplate->pkt_base_bits += flen;
            encoder_special_field_handle(field_info, "ICMPV6_CHECKSUM", field_id, flen,flen_old, &curr_header->seg_list);
            if (CTP_E_NONE != rs)
            {
                printf("handle the ICMPV6 checksum failed\n");
                return rs;
            }

            /* jump operation handle handle */
            if(field_info->flags & CTP_FLAG_JUMP)
            {
                if (encoder_jump_handle(field_info, &curr_header->seg_list, &jump_id,
                            pTemplate))
                {
                    goto NEXT_FIELD;
                }
            }

            jump_id = field_id + 1;
            goto NEXT_FIELD;
        }

        if (strstr(field_info->descriptor, "ICMP_CHECKSUM") != NULL)
        {
            int rs = 0;
            pTemplate->pkt_base_bits += flen;
            encoder_special_field_handle(field_info, "ICMP_CHECKSUM", field_id, flen,flen_old, &curr_header->seg_list);
            if (CTP_E_NONE != rs)
            {
                printf("handle the ICMP checksum failed\n");
                return rs;
            }

            /* jump operation handle handle */
            if(field_info->flags & CTP_FLAG_JUMP)
            {
                if (encoder_jump_handle(field_info, &curr_header->seg_list, &jump_id, pTemplate))
                {
                    goto NEXT_FIELD;
                }
            }

            jump_id = field_id + 1;
            goto NEXT_FIELD;
        }


        if (strstr(field_info->descriptor, "TCP_CHECKSUM") != NULL)
        {
            int rs = 0;
            pTemplate->pkt_base_bits += flen;
            encoder_special_field_handle(field_info, "TCP_CHECKSUM", field_id, flen, flen_old,&curr_header->seg_list);
            if (CTP_E_NONE != rs)
            {
                printf("handle the tcp checksum failed\n");
                return rs;
            }

            jump_id = field_id + 1;
            goto NEXT_FIELD;
        }

        if (strstr(field_info->descriptor, "CPUPKT_HM_HEADER_CRC") != NULL)
        {
            pTemplate->pkt_base_bits += flen;
            encoder_special_field_handle(field_info, "CPUPKT_HM_HEADER_CRC", field_id, flen,flen_old, &curr_header->seg_list);

            jump_id = field_id + 1;
            goto NEXT_FIELD;
        }

        if (strstr(field_info->descriptor, "CPUPKT_GB_HEADER_CRC") != NULL)
        {
            pTemplate->pkt_base_bits += flen;
            encoder_special_field_handle(field_info, "CPUPKT_GB_HEADER_CRC", field_id, flen, flen_old, &curr_header->seg_list);
            jump_id = field_id + 1;
            goto NEXT_FIELD;
        }
        if (strstr(field_info->descriptor, "UDP_LENGTH") != NULL)
        {
            int rs = 0;
            pTemplate->pkt_base_bits += flen;
            encoder_special_field_handle(field_info, "UDP_LENGTH", field_id, flen, flen_old, &curr_header->seg_list);
            if (CTP_E_NONE != rs)
            {
                printf("handle the udp length failed\n");
                return rs;
            }

            jump_id = field_id + 1;
            goto NEXT_FIELD;
        }

        if (strstr(field_info->descriptor, "UDP_CHECKSUM") != NULL)
        {
            int rs = 0;
            pTemplate->pkt_base_bits += flen;
            encoder_special_field_handle(field_info, "UDP_CHECKSUM", field_id, flen,flen_old, &curr_header->seg_list);
            if (CTP_E_NONE != rs)
            {
                printf("handle the udp checksum failed\n");
                return rs;
            }

            /* jump operation handle handle */
            if(field_info->flags & CTP_FLAG_JUMP)
            {
                if (encoder_jump_handle(field_info, &curr_header->seg_list, &jump_id, pTemplate))
                {
                    goto NEXT_FIELD;
                }
            }

            jump_id = field_id + 1;
            goto NEXT_FIELD;
        }

        if (strstr(field_info->descriptor, "IPV6_PAYL_LEN") != NULL)
        {
            int rs = 0;
            pTemplate->pkt_base_bits += flen;
            encoder_special_field_handle(field_info, "IPV6_PAYL_LEN", field_id, flen, flen_old,&curr_header->seg_list);
            if (CTP_E_NONE != rs)
            {
                printf("handle the ipv6 payload length failed\n");
                return rs;
            }

            jump_id = field_id + 1;
            goto NEXT_FIELD;
        }

        if (strstr(field_info->descriptor, "EXT_LENGTHER") != NULL)
        {
            int rs = 0;
            pTemplate->pkt_base_bits += flen;
            encoder_special_field_handle(field_info, "EXT_LENGTHER", field_id, flen, flen_old,&curr_header->seg_list);
            if (CTP_E_NONE != rs)
            {
                printf("handle the extension length failed\n");
                return rs;
            }

            jump_id = field_id + 1;
            goto NEXT_FIELD;
        }

        if (NULL != value)
        {
            XFREE(MTYPE_NEW_CTP_ENCODE,value);
            value = NULL;
        }
        if (found)
        {
            //printf("Field->descriptor:%s   keyword:%s     value:%s\n",field_info->descriptor, cmd->keyword,cmd->value);

            if (flen)
            {
                value = (uint8 *) XMALLOC (MTYPE_NEW_CTP_ENCODE,(flen + 7)/ 8);
                if (NULL == value)
                {
                    return CTP_E_MEMORY_ERROR;
                }

                memset(value, 0, ((flen + 7)/ 8));
                ctc_pktxr_string2char(cmd->value, value + ((flen + 7)/ 8)
                                        - (strlen(cmd->value) + 1)/2);

                if (strstr(field_info->descriptor, "IPV4_SA") != NULL)
                {
                    pTemplate->ip_len = IPV4_ADDR_LEN;
                    if (NULL != pTemplate->ip_sa) {
                        XFREE(MTYPE_NEW_CTP_ENCODE, pTemplate->ip_sa);
                    }
                    pTemplate->ip_sa = XBINDUP(MTYPE_NEW_CTP_ENCODE, &value[0], pTemplate->ip_len);
                    if (NULL == pTemplate->ip_sa) {
                        return CTP_E_MEMORY_ERROR;
                    }
                }
                else if (strstr(field_info->descriptor, "IPV4_DA") != NULL)
                {
                    pTemplate->ip_len = IPV4_ADDR_LEN;
                    if (NULL != pTemplate->ip_da) {
                        XFREE(MTYPE_NEW_CTP_ENCODE, pTemplate->ip_da);
                    }
                    pTemplate->ip_da = XBINDUP(MTYPE_NEW_CTP_ENCODE, &value[0], pTemplate->ip_len);
                    if (NULL == pTemplate->ip_da) {
                        return CTP_E_MEMORY_ERROR;
                    }
                }
                else if (strstr(field_info->descriptor, "IPV6_SA") != NULL)
                {
                    pTemplate->ip_len = IPV6_ADDR_LEN;
                    if (NULL != pTemplate->ip_sa) {
                        XFREE(MTYPE_NEW_CTP_ENCODE, pTemplate->ip_sa);
                    }
                    pTemplate->ip_sa = XBINDUP(MTYPE_NEW_CTP_ENCODE, &value[0],pTemplate->ip_len);
                    if (NULL == pTemplate->ip_sa) {
                        return CTP_E_MEMORY_ERROR;
                    }
                }
                else if (strstr(field_info->descriptor, "IPV6_DA") != NULL)
                {
                    pTemplate->ip_len = IPV6_ADDR_LEN;
                    if (NULL != pTemplate->ip_da) {
                        XFREE(MTYPE_NEW_CTP_ENCODE, pTemplate->ip_da);
                    }
                    pTemplate->ip_da = XBINDUP(MTYPE_NEW_CTP_ENCODE, &value[0],pTemplate->ip_len);
                    if (NULL == pTemplate->ip_da) {
                        return CTP_E_MEMORY_ERROR;
                    }
                }
            }
            else
            {
                uint16 pseudo_flen = flen_old ? flen_old : 8;
                value = (uint8 *) XMALLOC (MTYPE_NEW_CTP_ENCODE,(pseudo_flen + 7)/ 8);
                if (NULL == value)
                {
                    return CTP_E_MEMORY_ERROR;
                }
                memset(value, 0, ((pseudo_flen + 7)/ 8));
                ctc_pktxr_string2char(cmd->value, value+ ((pseudo_flen + 7)/ 8)
                                        - (strlen(cmd->value) + 1)/2);
            }
        }

        // if not set the value, the default value will be used
        if ((field_info->flags & CTP_FLAG_DEFAULT) && !found)
        {
            if (flen)
            {
                value = (uint8 *) XMALLOC (MTYPE_NEW_CTP_ENCODE,(flen + 7)/ 8);
                if (NULL == value)
                {
                    return CTP_E_MEMORY_ERROR;
                }

                memset(value, 0, (flen + 7)/ 8);
                ctc_pktxr_string2char(field_info->def_value, value+ ((flen + 7)/ 8)
                                        - (strlen(field_info->def_value) + 1)/2);
            }
            else
            {
                uint16 pseudo_flen = flen_old ? flen_old :8;
                value = (uint8 *) XMALLOC (MTYPE_NEW_CTP_ENCODE,(pseudo_flen + 7)/ 8);
                if (NULL == value)
                {
                    return CTP_E_MEMORY_ERROR;
                }

                memset(value, 0, ((pseudo_flen + 7)/ 8));
                ctc_pktxr_string2char(field_info->def_value, value);
            }
        }

        /* add to field segment list and default handle */
        if(found || (field_info->flags & CTP_FLAG_DEFAULT))
        {
            ctp_field_seg_t *temp_seg = NULL;
            pTemplate->pkt_base_bits += flen;
            temp_seg = (ctp_field_seg_t *) XCALLOC (MTYPE_NEW_CTP_ENCODE,sizeof(ctp_field_seg_t));
            if (NULL == temp_seg)
            {
                if (NULL != value)
                {
                    XFREE(MTYPE_NEW_CTP_ENCODE,value);
                    value = NULL;
                }
                return CTP_E_MEMORY_ERROR;
            }
            temp_seg->field_id = field_id;
            temp_seg->length   = flen;
            temp_seg->old_len  = flen_old;
            temp_seg->value    = value;
            TAILQ_INIT(&temp_seg->mod_list);
            value = NULL;
            TAILQ_INSERT_TAIL(&curr_header->seg_list, temp_seg, entry);
        }
        else
        {
            if (NULL != value)
            {
                XFREE(MTYPE_NEW_CTP_ENCODE,value);
                value = NULL;
            }
            if(field_info->flags & CTP_FLAG_EXIST)
            {
                jump_id               = field_info->exist->field_id;
                goto NEXT_FIELD;
            }
            else
            {
                CTP_ERR_RETURN(CTP_E_MIS_FIELD, field_info->descriptor);
            }
        }

        /* user define next protocol handle */
        if(field_info->flags & CTP_FLAG_UDN_TYPE)
        {
            uint16 t;

            if (found)
            {
                for (t =0; t < HEADER_TYPE_NUM; t++)
                {
                    if(strstr(cmd->value, ctc_pktxr_get_header_type_string(t)))
                    {
                        pTemplate->enc_cw[NEXT_TYPE] = t;
                        break;
                    }

                }
            }
            jump_id = FIELD_NUM;
            goto NEXT_FIELD;
        }

        /* no need EXIST FLAG Handle because if set or not exist but nust handle in decode*/

        /* set next protocol handle */
        if(field_info->flags & CTP_FLAG_NXT_PROT)
        {
            encoder_next_protocol_handle(field_info, &curr_header->seg_list, pTemplate);
            curr_header->next_type = pTemplate->enc_cw[NEXT_TYPE];
        }

        /* set repeat counter handle */
        if(field_info->flags & CTP_FLAG_REPEAT)
        {
            encoder_repeat_handle(field_info, &curr_header->seg_list, pTemplate);
        }

        /* set other field length handle handle */
        if(field_info->flags & CTP_FLAG_LENGTH)
        {
            encoder_length_handle(field_info, &curr_header->seg_list, pTemplate);
        }

        /* jump operation handle handle */
        if(field_info->flags & CTP_FLAG_JUMP)
        {
            if (encoder_jump_handle(field_info, &curr_header->seg_list, &jump_id, pTemplate))
            {
                goto NEXT_FIELD;
            }
        }

        jump_id = field_id + 1;

NEXT_FIELD:
        if(FIELD_NUM == jump_id)
        {
            if(HEADER_TYPE_NUM == pTemplate->enc_cw[NEXT_TYPE])
            {
                field_id = NULL_FIELD;
            }
            else /* add a new header */
            {
                ctp_header_info_t *temp = NULL;
                temp = (ctp_header_info_t *)XCALLOC(MTYPE_NEW_CTP_ENCODE, sizeof(ctp_header_info_t));
                if (NULL == temp)
                {
                    return CTP_E_MEMORY_ERROR;
                }
                TAILQ_INSERT_TAIL(&pTemplate->epkt.header, temp, entry);
                curr_header            = temp;
                curr_header->next_type = HEADER_TYPE_NUM;
                TAILQ_INIT(&curr_header->seg_list);
                field_id               = ctc_pktxr_get_header_start_field(pTemplate->enc_cw[NEXT_TYPE]);
                pTemplate->enc_cw[NEXT_TYPE]     = HEADER_TYPE_NUM;
                /*curr_seg = NULL;*/     // add this to ensure the seg list can be set
            }
        }
        else
        {
            field_id = jump_id;  //continue to execute while
        }
    }

    if (bCompose) {
        CTP_ERR_RETURN(packet_compose(pTemplate, pTemplate->enc_cw[PKT_LEN]), "packet_compose");
    }
    return CTP_E_NONE;
}

/* free pTemplate->epkt.header */
static void
encode_free_header(ctp_pkt_template_t *pTemplate)
{
    ctp_header_info_t *curr_header = NULL;
    ctp_field_seg_t   *curr_seg = NULL;

    /* free memory of header */
    while(!TAILQ_EMPTY(&pTemplate->epkt.header))
    {
        curr_header = TAILQ_FIRST(&pTemplate->epkt.header);
        TAILQ_REMOVE(&pTemplate->epkt.header, curr_header, entry);
        while(!TAILQ_EMPTY(&curr_header->seg_list))
        {
            curr_seg = TAILQ_FIRST(&curr_header->seg_list);
            TAILQ_REMOVE(&curr_header->seg_list, curr_seg, entry);
            if (NULL != curr_seg->pszSpecial)
            {
                XFREE(MTYPE_NEW_CTP_ENCODE, curr_seg->pszSpecial);
                curr_seg->pszSpecial = NULL;
            }
            if (NULL != curr_seg->value)
            {
                XFREE(MTYPE_NEW_CTP_ENCODE,curr_seg->value);
                curr_seg->value = NULL;
            }
            if (NULL != curr_seg)
            {
                XFREE(MTYPE_NEW_CTP_ENCODE,curr_seg);
                curr_seg = NULL ;
            }
        }


        if (NULL != curr_header)
        {
            XFREE(MTYPE_NEW_CTP_ENCODE,curr_header);
            curr_header = NULL;
        }
    }

    TAILQ_INIT(&pTemplate->epkt.header);
    return ;
}

/* free command list */
static void
encode_free_cmdlist(ctp_pkt_template_t *pTemplate)
{
    ctp_cmd_t *temp = NULL;
    ctp_cmd_t *psub = NULL;

    /* free left command list */
    while (!TAILQ_EMPTY(&pTemplate->enc_left_cmdl))
    {
        temp = TAILQ_FIRST(&pTemplate->enc_left_cmdl);
        TAILQ_REMOVE(&pTemplate->enc_left_cmdl, temp, entry);
        while (!TAILQ_EMPTY(&temp->list))
        {
            psub = TAILQ_FIRST(&temp->list);
            TAILQ_REMOVE(&temp->list, psub, entry);
            if (NULL != psub->keyword)
            {
                XFREE(MTYPE_NEW_CTP_ENCODE,psub->keyword);
                psub->keyword = NULL;
            }
            if (NULL != psub->value)
            {
                XFREE(MTYPE_NEW_CTP_ENCODE,psub->value);
                psub->value = NULL;
            }
            if (NULL != psub)
            {
                XFREE(MTYPE_NEW_CTP_ENCODE,psub);
                psub = NULL;
            }

        }
        if (NULL != temp->keyword)
        {
            XFREE(MTYPE_NEW_CTP_ENCODE,temp->keyword);
            temp->keyword = NULL;
        }
        if (NULL != temp->value)
        {
            XFREE(MTYPE_NEW_CTP_ENCODE,temp->value);
            temp->value = NULL;
        }
        if (NULL != temp)
        {
            XFREE(MTYPE_NEW_CTP_ENCODE,temp);
            temp = NULL;
        }
    }
    TAILQ_INIT(&pTemplate->enc_left_cmdl);

    /* free right command list */
    while (!TAILQ_EMPTY(&pTemplate->enc_right_cmdl))
    {
        temp = TAILQ_FIRST(&pTemplate->enc_right_cmdl);
        TAILQ_REMOVE(&pTemplate->enc_right_cmdl, temp, entry);
        while (!TAILQ_EMPTY(&temp->list))
        {
            psub = TAILQ_FIRST(&temp->list);
            TAILQ_REMOVE(&temp->list, psub, entry);
            if (NULL != psub->keyword)
            {
                XFREE(MTYPE_NEW_CTP_ENCODE,psub->keyword);
                psub->keyword = NULL;
            }
            if (NULL != psub->value)
            {
                XFREE(MTYPE_NEW_CTP_ENCODE,psub->value);
                psub->value = NULL;
            }
            if (NULL != psub)
            {
                XFREE(MTYPE_NEW_CTP_ENCODE,psub);
                psub = NULL;
            }
        }
        if (NULL != temp->keyword)
        {
            XFREE(MTYPE_NEW_CTP_ENCODE,temp->keyword);
            temp->keyword = NULL;
        }
        if (NULL != temp->value)
        {
            XFREE(MTYPE_NEW_CTP_ENCODE,temp->value);
            temp->value = NULL;
        }
        if (NULL != temp)
        {
            XFREE(MTYPE_NEW_CTP_ENCODE,temp);
            temp = NULL;
        }
    }
    TAILQ_INIT(&pTemplate->enc_right_cmdl);
    return;
}

/*******************************************************************************
 * Name   : encoder_reset
 * Purpose: reset encorder.
 * Input  : None.
 * Output : None.
 * Return : .
 * Note   : N/A
*******************************************************************************/
static ctp_error_e
encoder_reset(ctp_pkt_template_t *pTemplate)
{
    if (NULL == pTemplate) {
        pTemplate = g_global_pkt_template;
    }
    encode_free_cmdlist(pTemplate);
    encode_free_header(pTemplate);
    __pkt_modifier_free(pTemplate);

    /* free payload memory */
    if(NULL != pTemplate->pld_pattern)
    {
        XFREE(MTYPE_NEW_CTP_ENCODE, pTemplate->pld_pattern);
    }
    pTemplate->pld_pattern = NULL;
    if (NULL != pTemplate->ip_sa) {
        XFREE(MTYPE_NEW_CTP_ENCODE, pTemplate->ip_sa);
        pTemplate->ip_sa = NULL;
    }

    if (NULL != pTemplate->ip_da) {
        XFREE(MTYPE_NEW_CTP_ENCODE, pTemplate->ip_da);
        pTemplate->ip_da = NULL;
    }
    pTemplate->ip_len = 0;
    pTemplate->pkt_base_bits = 0;

    memset(&pTemplate->enc_cw, 0x00, sizeof(uint16) * CONTROL_WORD_NUM);
    memset(&pTemplate->epkt, 0, sizeof(ctp_pkt_info_t));
    pTemplate->epkt.packet = pTemplate->opkt_buf;
    return CTP_E_NONE;
}

/*******************************************************************************
 * Name   : ctc_pktxr_ctp_encode_command_set
 * Purpose: handle encorder command.
 * Input  : keyw - keyword of field set or command set.
 *        : value- value of field set or command set
 * Output : None.
 * Return : .
 * Note   : N/A
*******************************************************************************/
ctp_error_e
ctc_pktxr_ctp_encode_command_set(const char *keyw, const char *value, ctp_pkt_template_t *pTemplate)
{
    char      *cmd_str;
    ctp_cmd_t *cmd_list;
    int       ret;
    int pld_format=0;

    if (NULL == pTemplate) {
        pTemplate = g_global_pkt_template;
    }

    cmd_str = (char *)XMALLOC(MTYPE_NEW_CTP_ENCODE,strlen(keyw) + 1);
    if (NULL == cmd_str)
    {
        return CTP_E_MEMORY_ERROR;
    }

    memset(cmd_str, 0, strlen(keyw) + 1);
    ctc_pktxr_convert_to_upper(cmd_str, keyw);

    if(strstr(cmd_str, "@PKT_SEND"))
    {
        XFREE(MTYPE_NEW_CTP_ENCODE,cmd_str);
        /* kick off CTP send packet proecessing */
        CTP_ERR_RETURN(packet_encoder(1, g_global_pkt_template), "packet_encoder");
        pTemplate->pkt_base_bits = 0;
        return CTP_E_NONE;
    }
    else if(strstr(cmd_str, "@RESET_ENCORDER"))
    {
        XFREE(MTYPE_NEW_CTP_ENCODE,cmd_str);
        /* kick off CTP send packet proecessing */
        CTP_ERR_RETURN(encoder_reset(pTemplate), "encoder_reset");
        return CTP_E_NONE;
    }
    else if(strstr(cmd_str, "@PLD_STR"))
    {
        XFREE(MTYPE_NEW_CTP_ENCODE,cmd_str);
        if (0 == pkt_template_set_payload_pattern(NULL, (char *)value)) {
            return CTP_E_NONE;
        }
        return CTP_E_MEMORY_ERROR;
    }
    else if(strstr(cmd_str, "@PLD_FMT"))
    {
        XFREE(MTYPE_NEW_CTP_ENCODE,cmd_str);
        pld_format=atoi(value);
        //printf("===============pld_format====%d===========",pld_format);

        if (0 == pkt_template_set_payload_format(NULL, (payload_format_e)pld_format)) {
            return CTP_E_NONE;
        }
        return CTP_E_MEMORY_ERROR;
    }
    else if (strstr(cmd_str, "@SAVE_AS_TEMPLATE"))
    {
        if (TAILQ_EMPTY(&pTemplate->enc_left_cmdl)) {
            CTP_OUT("Current Packet Configuration empty error\n");
            return CTP_E_EMPTY;
        }
        ret = save_pkt_configuration_as_template((char *)value);
        XFREE(MTYPE_NEW_CTP_ENCODE, cmd_str);
        return ret;
    }
    else
    {
        cmd_list =  (ctp_cmd_t *) XCALLOC(MTYPE_NEW_CTP_ENCODE, sizeof(ctp_cmd_t));
        if (NULL == cmd_list)
        {
            XFREE(MTYPE_NEW_CTP_ENCODE,cmd_str);
            return CTP_E_MEMORY_ERROR;
        }
        TAILQ_INIT(&cmd_list->list);
        cmd_list->keyword = cmd_str;
        cmd_str = (char *)XMALLOC(MTYPE_NEW_CTP_ENCODE,strlen(value) + 1);
        if (NULL == cmd_str)
        {
            XFREE(MTYPE_NEW_CTP_ENCODE,cmd_list->keyword);
            XFREE(MTYPE_NEW_CTP_ENCODE,cmd_list);
            return CTP_E_MEMORY_ERROR;
        }

        strcpy(cmd_str, value);
        cmd_list->value   = cmd_str;

        TAILQ_INSERT_TAIL(&pTemplate->enc_left_cmdl, cmd_list, entry);
    }

    return CTP_E_NONE;
}

/*******************************************************************************
 * Name   : ctc_pktxr_ctp_encode_get_outpkt
 * Purpose: get the result packet.
 * Input  : None.
 * Output : outpacket pointer.
 * Return : .
 * Note   : N/A
*******************************************************************************/
ctp_error_e
ctc_pktxr_ctp_encode_get_outpkt(uint8 **packet, uint16 *pktlen)
{
    *packet = g_global_pkt_template->epkt.packet;
    *pktlen = (g_global_pkt_template->epkt.pktlen + 1) / 8;
    //Move from cts_xr_send_engine, by shanz, 2011/01/05
    encoder_reset(g_global_pkt_template);
    return CTP_E_NONE;
}

#if 0
/*******************************************************************************
 * Name   : get_md5_string
 * Purpose: get the MD5 string.
 * Input  : None.
 * Output : output md5 string pointer.
 * Return : .
 * Note   : N/A
*******************************************************************************/
static ctp_error_e
get_md5_string(uint8 *tohash,int len, unsigned char *final)
{
    uint8 result_str[16] ;
    //MD5(tohash,len,result_str);
    ctc_pktxr_uint2str((char*)final, result_str, 32);
    return CTP_E_NONE;
}

/*******************************************************************************
 * Name   : ctp_encode_ixia_get_outpkt
 * Purpose: get the result packet in Ixia.
 * Input  : None.
 * Output : outpacket pointer.
 * Return : .
 * Note   : N/A
*******************************************************************************/
static ctp_error_e
ctp_encode_ixia_get_outpkt(uint8 **packet, uint16 *pktlen)
{
    *packet = (g_global_pkt_template->epkt.packet + 12);
    if (((g_global_pkt_template->epkt.pktlen + 1) / 8 - 12) > 0)
    {
        *pktlen = ((g_global_pkt_template->epkt.pktlen + 1) / 8 - 12);
    }
    else
    {
        *pktlen = 0;
    }
    return CTP_E_NONE;
}


/*******************************************************************************
 * Name   : ctp_encode_get_outpkt_len
 * Purpose: get the result packet length.
 * Input  : None.
 * Output : outpacket pointer.
 * Return : .
 * Note   : N/A
*******************************************************************************/

static ctp_error_e
ctp_encode_get_outpkt_len(uint8 *packet_len)
{
    *packet_len = (g_global_pkt_template->epkt.pktlen  + 1) / 8;
    return CTP_E_NONE;
}
#endif


int ctc_pktxr_initialize_ctp_encoder()
{
    TAILQ_INIT(&g_pkt_template_hdr);

    g_global_pkt_template = pkt_template_calloc(_M_CTP_PKT_TEMPALTE_GLOBAL_NAME);
    if (NULL == g_global_pkt_template) {
        CTP_OUT("WARNING: Out of memory!!!\n");
        return CTP_E_MEMORY_ERROR;
    }

    register_special_fields();

    return CTP_E_NONE;
}
