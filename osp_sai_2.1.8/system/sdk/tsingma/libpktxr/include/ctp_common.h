/****************************************************************************
 * ctp_common.h  provides all CTP common Data Structures and Macros define.
 *
 * Copyright: (c)2007 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       Anchor Xie.
 * Date:         2007-4-11.
 * Reason:       First Create.
*****************************************************************************/

#ifndef _CTP_COMMON_H_
#define _CTP_COMMON_H_

/****************************************************************************
 *
 * Header Files
 *
****************************************************************************/
#include <stdint.h>
#include <arpa/inet.h>
#include <linux/types.h>
#include "stdio.h"
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "sal.h"
#include "ctp_util.h"
#include "ctp_header_format.h"
#include "cts_xr_config.h"
#include "cts_xr_debug.h"
#include "ctp_queue.h"
#include "ctp_simv.h"

/****************************************************************************
 *
 * Data Structure and Defines
 *
****************************************************************************/
#define NULL_FIELD            0xFFFF
#ifndef NULL
#define NULL 0
#endif

#define M_MAXPKT_BUFFER_SIZE    (CTS_XR_PKT_MTU * 2 + 32)
#define M_BUFF_LEN 32
#define IPV6_HEADER_LEN 40
//#define IS_BIT_SET(flag, bit)   (((flag) & (1 << (bit))) ? 1: 0)
/*define memory function*/
#define XMALLOC(type,size)      mem_malloc(MEM_CLI_MODULE, size)
#define XCALLOC(type,size)      mem_calloc(size, MEM_CLI_MODULE, __FILE__,  __LINE__)
#define XFREE(type,ptr)         mem_free(ptr)
#define XREALLOC(type,ptr,size) mem_realloc(MEM_CLI_MODULE, ptr, size)
#define XSTRDUP(type,str)       mem_strdup(str, MEM_CLI_MODULE, __FILE__,  __LINE__)


/*protocols enum*/
enum memory_type
{
    MTYPE_TMP = 0,	/* Must always be first and should be zero. */

    /*for protocols*/
    MTYPE_ETHERNET,
    MTYPE_VLAN,
    MTYPE_ARP,
    MTYPE_IPV4,
    MTYPE_IPV6,
    MTYPE_TCP,
    MTYPE_UDP,
    MTYPE_BPDU,
    MTYPE_IGMP,
    MTYPE_OSPF,
    MTYPE_ICMP,
    MTYPE_ICMPV6,
    MTYPE_PIM_SM,
    MTYPE_PAYLOAD,
    MTYPE_EXT_HEADER,

    /*for streams*/
    MTYPE_STREAM,
    MTYPE_ADAPTER,
    MTYPE_CAPTURE,
    MTYPE_FILTER,

    MTYPE_PORT,    /*port and socket*/
    MTYPE_UTIL,    /*util tools*/
    MTYPE_CMODEL,    /*cmodel and platform*/

    MTYPE_TCL_CMDS,   /*register tcl cmds*/
    MTYPE_NEW_CTP,
    MTYPE_NEW_CTP_ENCODE,
    MTYPE_NEW_CTP_DECODE,
    MTYPE_MAX
};


typedef enum  tagSearchSeq
{
    E_SEARCH_NORMAL = 0,
    E_SEARCH_REVERSE = 1,
} E_SearchSeq;
void ctc_pktxr_set_search_sequence(E_SearchSeq eseq);
E_SearchSeq ctc_pktxr_get_search_sequence(void);

/* Added by Alexander Liu 2012-10-22 16:39 */
#define M_CTP_MODIFY_SCRIPT_OKAT_STR        "success"

typedef struct tag_ctp_pkt_template ctp_pkt_template_t;

/* translate field name from ASIC to CTP */
typedef struct tag_field_name_trans {
    TAILQ_ENTRY(tag_field_name_trans) entry;

    char *pszAsicName; /* primary key */
    char *pszCtpName;
    int offset;
    int bits;
    int findex; /* field index */
} field_name_trans_t;

typedef TAILQ_HEAD(tag_field_name_hdr, tag_field_name_trans) field_name_trans_hdr_t;

/*
 * return value of this callback function:
 *   0 for success
 *   -1 for failed
 *
 * the caller will supply buffer pRetValue, the buffer size is M_MAXPKT_BUFFER_SIZE bytes
 * pnRetLen should fill by the callback function (the unit is bits)
 */
typedef int (*pkt_modifier_cb)(char *pszTemplate, char *pszField,
    int offset, int bits, unsigned char *pRetValue, int *pnRetLen, void *pArg);
typedef int (*pkt_modifier_free_cb)(void *pArg);

/* packet modifier item */
typedef struct tag_pm_cb {
    TAILQ_ENTRY(tag_pm_cb) entry;

    field_name_trans_t *pTrans; /* translate info */

    pkt_modifier_cb cb; /* callback function for this modifier */
    pkt_modifier_free_cb free_cb; /* free pExtraArg when destroy pm_cb_t */
    void *pExtraArg; /* extra data for cb */
} pm_cb_t;

/* packet modifier list */
typedef TAILQ_HEAD(tag_pm_cb_list, tag_pm_cb) pm_cb_list_t;

int pkt_template_register_modifier(char *pszTemplate,
    char *pszAsicFieldName, char *pszProc);

/*
 * This data structure is used for stateless field store
 * in case of packet parsing and compose processing
 */
struct ctp_field_seg
{
    uint16               field_id;
    uint16               length;  /* bit length */
    uint16               old_len;
    uint8                *pszSpecial; /* for special field, eg: IPV4_LEN, CHKSUM */
    uint8                *value;

    pm_cb_list_t mod_list; /* Modifier for this field */

    TAILQ_ENTRY(ctp_field_seg) entry;
};
typedef struct ctp_field_seg ctp_field_seg_t;
typedef TAILQ_HEAD(tagctp_field_seg_hdr_t, ctp_field_seg) ctp_field_seg_hdr_t;

/*
 * This data structure is used for stateless header store
 * in case of packet parsing and compose processing
 */
struct ctp_header_info
{
    ctp_field_seg_hdr_t seg_list;
    uint32 next_type;

    TAILQ_ENTRY(ctp_header_info) entry;
};
typedef struct ctp_header_info ctp_header_info_t;
typedef TAILQ_HEAD(tagctp_header_info_hdr_t, ctp_header_info) ctp_header_info_hdr_t;

/*
 * This data structure is use for store field information which
 * is retrieve by packet compose and filtering processing.
 * This info input from a script file automatic , and store it
 * to a gloabal arraies.
 */

/*
 * This data structure is used for stateless CTP_FLAGS_EXIST store
 * in case of packet parsing and compose processing
 */

struct ctp_flags_exist
{
    uint8      next;
    uint8      op_flags;
#define OP_FLAG_STAND_ONLY       (1 << 0)
#define OP_FLAG_AND_NEXT         (1 << 1)
#define OP_FLAG_OR_NEXT          (1 << 2)
#define OP_FLAG_DEC_CNT          (1 << 3)
    union
    {
        uint16 field_id; /* faild jump ID, dest field Id for length */
        uint16 protocol;
    };
    char       *exp;
};
typedef struct ctp_flags_exist ctp_flags_exist_t;
typedef struct ctp_flags_exist ctp_flags_jump_t;
typedef struct ctp_flags_exist ctp_flags_nextp_t;
typedef struct ctp_flags_exist ctp_flags_repeat_t;
typedef struct ctp_flags_exist ctp_flags_len_t;


/*
 * This data structure is used for stateless field infomation
 * in case of packet parsing end compose processing
 */
struct ctp_field_info
{
    char                *descriptor;
    uint16             field_len;
    uint16             flags;
#define CTP_FLAG_EXIST        (1 << 0)
#define CTP_FLAG_JUMP         (1 << 1)
#define CTP_FLAG_DEFAULT      (1 << 2)
#define CTP_FLAG_NXT_PROT     (1 << 3)
#define CTP_FLAG_REPEAT       (1 << 4)
#define CTP_FLAG_LENGTH       (1 << 5)
#define CTP_FLAG_UDN_TYPE     (1 << 6)
    ctp_flags_exist_t  *exist;
    ctp_flags_jump_t   *jump;
    ctp_flags_nextp_t  *nextp;
    ctp_flags_repeat_t  *repeat;
    ctp_flags_len_t    *length;
    char             *def_value;
};
typedef struct ctp_field_info ctp_field_info_t;

/*
 * This data structure is use for store user inputted command
 * string list whitch used for parsing, compose and filtering.
 */
struct ctp_command
{
    char              *keyword;
    char              *value;

    TAILQ_ENTRY(ctp_command) entry;

    struct {
        struct ctp_command *tqh_first; /* first element */
        struct ctp_command **tqh_last; /* addr of last next element */
    } list;
};
typedef struct ctp_command ctp_cmd_t;
typedef TAILQ_HEAD(tagctp_command_hdr_t, ctp_command) ctp_command_hdr_t;

/*
 * This data structure is use for store packet infomaitons
 * when packet parsing, compose and filtering.
 */
struct ctp_packet_info
{
    uint16            length; /* buffer length: byte */
    uint16            type; /* first header type */
    ctp_header_info_hdr_t header;

    uint8             *packet;
    uint16            pktlen; /* current packet length: bit */
};
typedef struct ctp_packet_info ctp_pkt_info_t;

/* whole packet compare, check ip/tcp/udp checksum control */
struct chk_checksum_flag_s
{
    bool ignore_ip_checksum;
    bool ignore_tcp_checksum;
    bool ignore_udp_checksum;
};
typedef struct chk_checksum_flag_s chk_checksum_flag_t;

/* whole packet compare, check ipv4/ipv6/udp total length */
struct chk_total_length_flag_s
{
    bool ignore_ipv4_total_length;
    bool ignore_ipv6_total_length;
    bool ignore_udp_total_length;
};
typedef struct chk_total_length_flag_s chk_total_length_flag_t;

/* Added by Alexander 2012-10-18 19:19 */

#define _M_CTP_PKT_TEMPLATE_NAME_SIZE           32
#define _M_CTP_PKT_TEMPALTE_GLOBAL_NAME         "__GLOBAL_DEFAULT__"
#define _M_CTP_PKT_TEMPLATE_PAYLOAD_FIELD_STR   "PLD_STR"
#define _M_CTP_PKT_TEMPLATE_PKTCRC_FIELD_STR    "PKT_CRC"

struct tag_ctp_pkt_template {
    TAILQ_ENTRY(tag_ctp_pkt_template) entry;

    char name[_M_CTP_PKT_TEMPLATE_NAME_SIZE];
    ctp_pkt_info_t epkt;   /* encoded packet */
    void *pld_pattern; /* payload string 0x.... */
    payload_format_e pld_format; /* payload format (default is E_PKT_PLD_FMT_ZERO */
    pm_cb_list_t payload_mod_list; /* Modifier for payload */
    pm_cb_list_t crc_mod_list; /* Modifier for Crc of the whole packet */
#if 0
    size_t pld_offset; /* hold payload offset */
#endif
    uint16 enc_cw[CONTROL_WORD_NUM];
    /*
     * This Variablies used for store handled and to be handled command string
     */
    ctp_command_hdr_t enc_left_cmdl;
    ctp_command_hdr_t enc_right_cmdl;

    uint8* ip_da; /* for IPv4 & IPv6 */
    uint8* ip_sa;
    int ip_len;
    int pkt_base_bits; /* base packet bits length: not include payload and pkt crc */

    /* reference count to avoid free */
    int refcnt;

    /* hold packet */
    unsigned char opkt_buf[M_MAXPKT_BUFFER_SIZE];

    /* template status */
#define _M_CTP_PKT_TEMPLATE_STATUS_PKT_CFG      0x1
#define _M_CTP_PKT_TEMPLATE_STATUS_PRE_COMPOSE  0x2
    int status;

    field_name_trans_hdr_t transHdr;

    pm_cb_list_t *pModListArr[E_MOD_MAX]; /* fields need modify from simv, reference related segment */
};

typedef TAILQ_HEAD(tag_ctp_pkt_template_hdr, tag_ctp_pkt_template) ctp_pkt_template_hdr_t;

/* some field need delay process
 *
 * eg: IPv4 Length, check sum
 */

/* process delayed field */
typedef struct tagDelayProcParam DelayProcParam_t;
typedef int (*delay_field_proc_cb)(ctp_pkt_template_t *pTemplate,
    DelayProcParam_t *pParam, int nCurOffset);

typedef struct tagDelayProcField {
    TAILQ_ENTRY(tagDelayProcField) entry;

    char *pszName; /* field name */
    delay_field_proc_cb procCB; /* process callback function */
    int nStoreDelta;
    int bGlobal; /* is global field ? */
} DelayProcField_t;

typedef TAILQ_HEAD(tagDelayProcList, tagDelayProcField) DelayProcList_t;

struct tagDelayProcParam {
    TAILQ_ENTRY(tagDelayProcParam) entry;

    unsigned char *pOffset; /* field start offset */
    int nStart; /* start offset */
    int nStoreOffset; /* store offset */
    int nLen; /* field length */

    pm_cb_list_t *pModList; /* modifier for this field */

    DelayProcField_t *pField; /* reference, need not free */
};

typedef TAILQ_HEAD(tagDelayProcParamList, tagDelayProcParam) DelayProcParamList_t;

/****************************************************************************
 *
 * Global and Declarations
 *
****************************************************************************/

/****************************************************************************
 *
 * Functions
 *
 ****************************************************************************/

/*******************************************************************************
 * Name   : ctc_pktxr_logical_expression_calculator
 * Purpose: logical expression calculate.
 * Input  : .
 * Output : None.
 * Return : .
 * Note   : N/A
*******************************************************************************/
int
ctc_pktxr_logical_expression_calculator(char *exp, ctp_field_seg_hdr_t *fiedl_seg,uint16 *cw);
/*******************************************************************************
 * Name   : ctc_pktxr_math_expression_calculator
 * Purpose: maths expression calculate.
 * Input  : .
 * Output : None.
 * Return : stored header type string's pointer.
 * Note   : N/A
*******************************************************************************/
void
ctc_pktxr_math_expression_calculator(char *exp, ctp_field_seg_hdr_t *fiedl_seg,
                           uint16 *enc_cw, uint32 *result);

/*******************************************************************************
 * Name   : ctc_pktxr_get_header_type_string
 * Purpose: Get the pointer of stored header type string.
 * Input  : header type enum.
 * Output : None.
 * Return : stored header type string's pointer.
 * Note   : N/A
*******************************************************************************/
char *
ctc_pktxr_get_header_type_string(uint16 header_type);

/*******************************************************************************
 * Name   : ctc_pktxr_get_header_start_field
 * Purpose: Get the first field of header type.
 * Input  : header type enum.
 * Output : None.
 * Return : stored field ID.
 * Note   : N/A
*******************************************************************************/
uint16
ctc_pktxr_get_header_start_field(uint16 header_type);

/*******************************************************************************
* Name   : ctc_pktxr_get_field_info_pointer
 * Purpose: Get the pointer of stored.
 * Input  : None.
 * Output : None.
 * Return : stored header type string's pointer.
 * Note   : N/A
*******************************************************************************/
ctp_field_info_t *
ctc_pktxr_get_field_info_pointer(uint16 field_id);
/* Added by Alexander 2012-10-22 10:33 */
ctp_field_info_t *ctc_pktxr_find_field_by_name(char *pszName);
void ctc_pktxr_output_all_fields(int fd);

/******************************************************************************
 *
 * Name   : ctc_pktxr_control_word_handle
 * Purpose: Handle the control word command string.
 * Input  : .
 * Output : None.
 * Return : stored header type string's pointer.
 * Note   : N/A
*******************************************************************************/
ctp_error_e
ctc_pktxr_control_word_handle(ctp_command_hdr_t *l, ctp_command_hdr_t *r, uint16 *cw);

 /*******************************************************************************
 * Name   : cmmd_lookup
 * Purpose: look up the command string.
 * Input  : .
 * Output : None.
 * Return : stored header type string's pointer.
 * Note   : N/A
*******************************************************************************/
uint8
ctc_pktxr_cmd_lookup(ctp_command_hdr_t *l, ctp_command_hdr_t *r, char *key, ctp_cmd_t **f);

/*******************************************************************************
 * Name   : packet_decoder
 * Purpose: decode the packet and save the result to dec_pkt.
 * Input  : packet - input packet ponter.
 * Output : None.
 * Return : .
 * Note   : user need release its own memory.
*******************************************************************************/
ctp_error_e
ctc_pktxr_packet_decoder(uint8 *packet);

/*******************************************************************************
 * Name   : decode_command_handle
 * Purpose: handle decorder command.
 * Input  : keyw - keyword of field set or command set.
 *        : value- value of field set or command set
 * Output : None.
 * Return : .
 * Note   : user need release its own memory
*******************************************************************************/
ctp_error_e
ctc_pktxr_decode_command_set(const char *keyw, const char *value);

/*******************************************************************************
 * Name   : decoder_get_field
 * Purpose: get the the segment infomations.
 * Input  : field_id - input field ID.
 * Output : None.
 * Return : .
 * Note   : .
*******************************************************************************/
ctp_error_e
ctc_pktxr_decoder_get_field(uint16 field_id, ctp_field_seg_t **field_seg);

/*******************************************************************************
 * Name   : decoder_get_list_field
 * Purpose: according to the dec_pkt and field_id, and get the the segment infomations from dec_pkt.
 * Input  : field_id - input field ID.
 * Output : None.
 * Return : .
 * Note   : .
*******************************************************************************/
ctp_error_e
ctc_pktxr_decoder_get_list_field(uint16 field_id, ctp_field_seg_t **field_seg, int count);

/*******************************************************************************
 * Name   : ctc_pktxr_decoder_get_payload
 * Purpose: get the the segment infomations.
 * Input  : None.
 * Output : None.
 * Return : .
 * Note   : .
*******************************************************************************/
ctp_error_e
ctc_pktxr_decoder_get_payload(uint16 *len, uint8 **pld);


/*******************************************************************************
 * Name   : ctc_pktxr_ctp_encode_get_outpkt
 * Purpose: get the result packet.
 * Input  : None.
 * Output : outpacket pointer.
 * Return : .
 * Note   : N/A
*******************************************************************************/
ctp_error_e
ctc_pktxr_ctp_encode_get_outpkt(uint8 **packet, uint16 *pktlen);

#endif
