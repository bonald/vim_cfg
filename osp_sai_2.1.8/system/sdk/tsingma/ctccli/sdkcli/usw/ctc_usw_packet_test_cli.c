/**
 @file ctc_libpkt_test_cli.c

 @date 2018-12-13

 @version v1.0

---file comments----
*/

#if 0/*SYSTEM MODIFIED by yejl for delete the packet cli, 2019-03-05*/
/****************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#ifdef _SAL_LINUX_UM
#include "ctc_cli.h"
#include "ctc_cli_common.h"
#include "ctp_api.h"
#include "ctc_api.h"
#include "ctcs_api.h"
#include "ctc_packet.h"
#include "ctc_error.h"
#include "ctc_debug.h"

extern int32
sys_usw_packet_get_rx_buffer(uint8 lchip,
                                      uint16 lport,
                                      int32 (*filter)(uint8 *packet, uint16 *match, uint8 **got_value),
                                      void** p_pkt_test_rx,
                                      uint8* p_match_num);
extern ctp_error_e
ctc_pktxr_filter_reset(void);

#define CTC_CLI_PACKET "packet"
#define CTC_CLI_PACKET_TEST_M_STR "packet key string, for example $ETH_MAC_DA"
#define CTC_CLI_PACKET_TEST_M_VAL "packet value, for example 060504030201"
#define CTC_CLI_PKT_BUF_PKT_LEN 256
#define CTC_CLI_PACKET_TEST_RETURN(op) \
    { \
        int32 rv = (op); \
        if (rv != CTP_E_NONE) \
        { \
            if (g_error_on) \
            sal_printf("Error Happened!! Fun:%s()  Line:%d ret:%d\n",__FUNCTION__,__LINE__, rv); \
            return CLI_ERROR; \
        } \
    }
struct ctc_cli_pkt_buf_s
{
   uint16 pkt_len;
   uint8  mode;
   uint8  rsv;
   uint8  pkt_data[CTC_CLI_PKT_BUF_PKT_LEN];
   sal_time_t tm;
   uint32 hash_seed;
   uint32  seconds;    /* for packet rx timestamp dump */
   uint32  nanoseconds;    /* for packet rx timestamp dump */
};
typedef struct ctc_cli_pkt_buf_s ctc_cli_pkt_buf_t;

CTC_CLI(ctc_cli_usw_packet_initialize,
    ctc_cli_usw_packet_initialize_cmd,
    "packet initialize",
    CTC_CLI_PACKET,
    "initialize")
    {
        int32 ret;
        ret = ctc_pktxr_initialize_ctp_encoder();
        if (ret < 0)
        {
            ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
            return CLI_ERROR;
        }
        return ret;
    }

CTC_CLI(ctc_cli_usw_packet_encode,
        ctc_cli_usw_packet_encode_cmd,
        "packet encode KEY VALUE",
        CTC_CLI_PACKET,
        "encode packet",
        CTC_CLI_PACKET_TEST_M_STR,
        CTC_CLI_PACKET_TEST_M_VAL)
{
    char* keyw = NULL;
    char* value = NULL;
    int32 ret = 0;

    keyw = argv[0];
    value = argv[1];

    CTC_CLI_PACKET_TEST_RETURN(ctc_pktxr_ctp_encode_command_set(keyw, value, NULL));

    return ret;

}

CTC_CLI(ctc_cli_usw_packet_send,
        ctc_cli_usw_packet_send_cmd,
        "packet send port PORT (count CNT|)",
        CTC_CLI_PACKET,
        "send packet",
        "source port",
        CTC_CLI_GPORT_ID_DESC,
        "packet count",
        "COUNT")
{
    static ctc_pkt_tx_t pkt_tx;
    ctc_pkt_tx_t* p_pkt_tx = &pkt_tx;
    ctc_pkt_info_t* p_tx_info = &(p_pkt_tx->tx_info);
    ctc_pkt_skb_t* p_skb = &(p_pkt_tx->skb);
    uint8* pkt_buf = NULL;
    uint16 pkt_len = 0;
    int32 ret = 0;
    int32 idx = 0;
    uint32 count = 1;
    uint32 cnt = 0;
    uint16 line = 0;
    uint16 num = 0;

    sal_memset(&pkt_tx, 0, sizeof(pkt_tx));
    pkt_tx.mode = 0;
    p_tx_info->flags = CTC_PKT_FLAG_INGRESS_MODE;
    CTC_CLI_GET_UINT32("port", p_tx_info->dest_gport, argv[0]);

    idx = CTC_CLI_GET_ARGC_INDEX("count");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("count", count, argv[idx + 1]);
    }

    CTC_CLI_PACKET_TEST_RETURN(ctc_pktxr_ctp_encode_get_outpkt(&pkt_buf, &pkt_len));

    p_skb->data = pkt_buf;
    p_skb->len = pkt_len;

    while (cnt < count)
    {
        if (g_ctcs_api_en)
        {
            ret = ctcs_packet_tx(g_api_lchip, p_pkt_tx);
        }
        else
        {
            ret = ctc_packet_tx(p_pkt_tx);
        }
        if (ret < 0)
        {
            ctc_cli_out("%% ctc_packet_tx,ret = %d, %s, count: %5d\n", ret, ctc_get_error_desc(ret), cnt);
            return CLI_ERROR;
        }
        cnt++;
    }

    if (pkt_buf && pkt_len)
    {
        sal_printf("packet: \n");
        for (num = 0; num < pkt_len; num++)
        {
            line++;
            if ((line % 16) == 0)
            {
                sal_printf("%02x\n", pkt_buf[num]);
            }
            else
            {
                sal_printf("%02x", pkt_buf[num]);
            }
        }
        sal_printf("\n");
    }
    return ret;

}

CTC_CLI(ctc_cli_usw_packet_filter,
        ctc_cli_usw_packet_filter_cmd,
        "packet filter (set KEY VALUE | reset)",
        CTC_CLI_PACKET,
        "filter packet",
        "set filter"
        CTC_CLI_PACKET_TEST_M_STR,
        CTC_CLI_PACKET_TEST_M_VAL,
        "reset filter")
{
    const char *keyw = NULL;
    const char *value = NULL;
    int32 idx = 0;
    int8 is_set = 0;

    idx = CTC_CLI_GET_ARGC_INDEX("set");
    if (0xFF != idx)
    {
        is_set = 1;
        keyw = argv[0];
        value = argv[1];
    }

    idx = CTC_CLI_GET_ARGC_INDEX("reset");
    if (0xFF != idx)
    {
        is_set = 0;
    }
    if (is_set)
    {
        CTC_CLI_PACKET_TEST_RETURN(ctc_pktxr_filter_command_set(keyw, value));
    }
    else
    {
        CTC_CLI_PACKET_TEST_RETURN(ctc_pktxr_filter_reset());
    }
    return 0;
}


CTC_CLI(ctc_cli_usw_packet_show_packet,
        ctc_cli_usw_packet_show_packet_cmd,
        "show packet port PORT (id ID|) (cnt|frame|)",
        "show",
        CTC_CLI_PACKET,
        "source port",
        CTC_CLI_GPORT_ID_DESC,
        "packt-id",
        "PACKET ID",
        "count",
        "frame")
{
    uint8* pkt_buf = NULL;
    uint16 pkt_len = 0;
    int32 ret = 0;
    uint8 lchip = 0;
    uint16  lport;
    int32 idx = 0;
    uint8 num = 0xFF;
    uint8 id = 0;
    uint8 match_num = 0;
    /*bool assign_id = FALSE;*/
    uint16 line = 0;
    ctc_cli_pkt_buf_t* p_pkt_test_buf = NULL;

    CTC_CLI_GET_UINT32("port", lport, argv[0]);

    idx =  CTC_CLI_GET_ARGC_INDEX("id");
    if (0xFF != idx)
    {
        CTC_CLI_GET_UINT32("id", id, argv[idx + 1]);
        /*assign_id = TRUE;*/
    }

    p_pkt_test_buf = (ctc_cli_pkt_buf_t*)sal_malloc(num*sizeof(ctc_cli_pkt_buf_t));
    if (p_pkt_test_buf == NULL)
    {
        ctc_cli_out("Memory alloc error ! \n");
        return CLI_ERROR;
    }
    sal_memset(p_pkt_test_buf, 0, sizeof(ctc_cli_pkt_buf_t));

    ret = sys_usw_packet_get_rx_buffer(lchip,
                                       lport,
                                       ctc_pktxr_packet_filter_value,
                                       (void** )&p_pkt_test_buf,
                                       &match_num);

    if (ret < 0)
    {
        ctc_cli_out("%% %s \n", ctc_get_error_desc(ret));
        return CLI_ERROR;
    }

    idx =  CTC_CLI_GET_ARGC_INDEX("cnt");
    if (0xFF != idx)
    {
        ctc_cli_out("count:%u \n", match_num);
    }
    else
    {
        if (id == 0)
        {
            id = 1;
        }

        if (id > match_num)
        {
            return 0;
        }

        p_pkt_test_buf = p_pkt_test_buf + id - 1;

        pkt_buf = p_pkt_test_buf->pkt_data + 40;
        pkt_len = p_pkt_test_buf->pkt_len - 40;


        idx =  CTC_CLI_GET_ARGC_INDEX("frame");
        if (0xFF != idx)
        {
               sal_printf("packet:");
            for (num = 0; num < pkt_len; num++)
            {
                sal_printf("%02x ", pkt_buf[num]);
            }
        }
        else
        {
            sal_printf("packet(len %d): \n", pkt_len);
            for (num = 0; num < pkt_len; num++)
            {
                line++;
                if ((line % 16) == 0)
                {
                    sal_printf("%02x\n", pkt_buf[num]);
                }
                else
                {
                    sal_printf("%02x", pkt_buf[num]);
                }
            }
        }
        sal_printf("\n");
    }

    return ret;

}
#endif

int32
ctc_usw_packet_test_cli_init(void)
{
#ifdef _SAL_LINUX_UM
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_initialize_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_encode_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_send_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_filter_cmd);
    install_element(CTC_SDK_MODE, &ctc_cli_usw_packet_show_packet_cmd);
#endif
    return 0;
}

int32
ctc_usw_packet_test_cli_deinit(void)
{
#ifdef _SAL_LINUX_UM
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_initialize_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_encode_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_send_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_filter_cmd);
    uninstall_element(CTC_SDK_MODE, &ctc_cli_usw_packet_show_packet_cmd);
#endif
    return 0;
}
#endif
