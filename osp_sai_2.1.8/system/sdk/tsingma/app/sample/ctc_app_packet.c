/**
 @file ctc_app_packet.c

 @date 2012-11-30

 @version v2.0

*/

/****************************************************************************
 *
* Header Files
*
****************************************************************************/
#include "sal.h"
#include "api/include/ctc_api.h"
#include "ctc_app.h"
#include "ctc_app_packet.h"

/****************************************************************************
 *
* Defines and Macros
*
*****************************************************************************/

#define PKT_SAMPLE_STR      "Packet Sample "
#if 0

struct app_pkt_stats_s
{
    uint32 tx;
    uint32 rx;
};
typedef struct app_pkt_stats_s app_pkt_stats_t;

struct app_pkt_eth_s
{
    int32 sock;
    char  ifname[10];
    sal_task_t* t_rx;
    app_pkt_stats_t stats;
};
typedef struct app_pkt_eth_s app_pkt_eth_t;


struct app_pkt_sample_master_s
{
    uint32 init_done;
    uint32 rx_print_en;
    CTC_PKT_RX_CALLBACK internal_rx_cb;  /* for SDK's internal process */
    CTC_PKT_RX_CALLBACK rx_cb;
    app_pkt_eth_t eth;
};
typedef struct app_pkt_sample_master_s app_pkt_sample_master_t;
#endif

/****************************************************************************
*
* Global and Declaration
*
*****************************************************************************/

app_pkt_sample_master_t* g_app_pkt_sample_master = NULL;

/****************************************************************************
*
* Function
*
*****************************************************************************/
STATIC int32
_ctc_app_packet_sample_dump8(uint8* data, uint32 len)
{
    uint32 cnt = 0;
    char line[256];
    char tmp[256];

    if (0 == len)
    {
        return CTC_E_NONE;
    }

    for (cnt = 0; cnt < len; cnt++)
    {
        if ((cnt % 16) == 0)
        {
            if (cnt != 0)
            {
                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s", line);
            }

            sal_memset(line, 0, sizeof(line));
            sal_sprintf(tmp, "\n0x%04x:  ", cnt);
            if (sal_strlen(tmp) > 32)
            {
                tmp[32] = '\0';
            }
            sal_strcat(line, tmp);
        }

        sal_sprintf(tmp, "%02x", data[cnt]);
        if (sal_strlen(tmp) > 32)
        {
            tmp[32] = '\0';
        }
        sal_strcat(line, tmp);

        if ((cnt % 2) == 1)
        {
            sal_strcat(line, " ");
        }
    }

    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%s", line);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\n");

    return CTC_E_NONE;
}

int32
_ctc_app_packet_sample_dump_info(ctc_pkt_info_t* p_info, uint32 is_tx)
{
    if (is_tx)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nTx Information\n");
    }
    else
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nRx Information\n");
    }

    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------\n");
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "flags              :   0x%08X\n", p_info->flags);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "oper_type          :   %d\n", p_info->oper_type);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "priority           :   %d\n", p_info->priority);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "color              :   %d\n", p_info->color);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "src_cos            :   %d\n", p_info->src_cos);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "TTL                :   %d\n", p_info->ttl);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "is_critical        :   %d\n", p_info->is_critical);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "dest_gport         :   0x%04x\n", p_info->dest_gport);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "dest_group_id      :   %d\n", p_info->dest_group_id);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "src_svid           :   %d\n", p_info->src_svid);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "src_cvid           :   %d\n", p_info->src_cvid);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "src_gport          :   0x%04x\n", p_info->src_port);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "logic src_gport    :   %d\n", p_info->logic_src_port);

    if (CTC_PKT_OPER_OAM == p_info->oper_type)
    {
        /* OAM */
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "oam.type           :   %d\n", p_info->oam.type);
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "oam.flags          :   0x%08X\n", p_info->oam.flags);
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "oam.mep_index      :   %d\n", p_info->oam.mep_index);
        if (CTC_FLAG_ISSET(p_info->oam.flags, CTC_PKT_OAM_FLAG_IS_DM))
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "oam.dm_ts_offset   :   %d\n", p_info->oam.dm_ts_offset);
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "oam.dm_ts.sec      :   %d\n", p_info->oam.dm_ts.seconds);
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "oam.dm_ts.ns       :   %d\n", p_info->oam.dm_ts.nanoseconds);
        }
    }

    if (CTC_PKT_OPER_PTP == p_info->oper_type)
    {
        /* PTP */
        if (is_tx)
        {
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ptp.oper           :   %d\n", p_info->ptp.oper);
            CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ptp.seq_id         :   %d\n", p_info->ptp.seq_id);
        }

        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ptp.ts.sec         :   %d\n", p_info->ptp.ts.seconds);
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ptp.ts.ns          :   %d\n", p_info->ptp.ts.nanoseconds);
    }

    if (is_tx)
    {
        /* TX */
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "nh_offset          :   %d\n", p_info->nh_offset);
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "hash               :   %d\n", p_info->hash);
    }
    else
    {
        /* RX */
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "reason             :   %d\n", p_info->reason);
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "vrfid              :   %d\n", p_info->vrfid);
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "packet_type        :   %d\n", p_info->packet_type);
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "src_chip           :   %d\n", p_info->src_chip);
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "payload_offset     :   %d\n", p_info->payload_offset);
    }

    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------\n");
    return CTC_E_NONE;
}

int32
_ctc_app_packet_sample_dump_pkt_buf(ctc_pkt_buf_t* p_pkt_data, uint32 buf_count)
{
    ctc_pkt_buf_t* p_data = NULL;
    uint32 i = 0;

    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------\n");

    for (i = 0; i < buf_count; i++)
    {
        p_data = &(p_pkt_data[i]);
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Buffer %d : Length %d\n", i, p_data->len);
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------");
        _ctc_app_packet_sample_dump8(p_data->data, p_data->len);
    }

    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------------------\n");
    return CTC_E_NONE;
}

int32
_ctc_app_packet_sample_build_raw_pkt(uint8* p_pkt, ctc_pkt_rx_t* p_pkt_rx)
{
    uint32 i = 0;
    uint8* p = p_pkt;

    for (i = 0; i < p_pkt_rx->buf_count; i++)
    {
        sal_memcpy(p, p_pkt_rx->pkt_buf[i].data, p_pkt_rx->pkt_buf[i].len);
        p += p_pkt_rx->pkt_buf[i].len;
    }

    return CTC_E_NONE;
}

int32
_ctc_app_packet_sample_dump_raw_pkt(ctc_pkt_rx_t* p_pkt_rx)
{
    uint8* p_pkt = NULL;
    uint8* p = NULL;
    uint16 data_len = 0;

    p_pkt = sal_malloc(p_pkt_rx->pkt_len);
    if(NULL == p_pkt)
    {
        return CTC_E_NO_MEMORY;
    }

    _ctc_app_packet_sample_build_raw_pkt(p_pkt, p_pkt_rx);

    p = p_pkt;
    if (p_pkt_rx->eth_hdr_len)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ETH Header Length : %d\n", p_pkt_rx->eth_hdr_len);
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------");
        _ctc_app_packet_sample_dump8(p, p_pkt_rx->eth_hdr_len);
        p += p_pkt_rx->eth_hdr_len;
    }

    if (p_pkt_rx->pkt_hdr_len)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet Header Length : %d\n", p_pkt_rx->pkt_hdr_len);
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------");
        _ctc_app_packet_sample_dump8(p, p_pkt_rx->pkt_hdr_len);
        p += p_pkt_rx->pkt_hdr_len;
    }

    if (p_pkt_rx->stk_hdr_len)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Stacking Header Length : %d\n", p_pkt_rx->stk_hdr_len);
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------");
        _ctc_app_packet_sample_dump8(p, p_pkt_rx->stk_hdr_len);
        p += p_pkt_rx->stk_hdr_len;
    }

    p = p_pkt + CTC_PKT_TOTAL_HDR_LEN(p_pkt_rx);
    data_len = p_pkt_rx->pkt_len - CTC_PKT_TOTAL_HDR_LEN(p_pkt_rx);

    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Packet Data Length : %d\n", data_len);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "------------------------------------------------");
    _ctc_app_packet_sample_dump8(p, data_len);

    sal_free(p_pkt);
    p_pkt = NULL;

    return CTC_E_NONE;
}

int32
_ctc_app_packet_sample_dump_rx(ctc_pkt_rx_t* p_pkt_rx)
{
    if (CTC_PKT_MODE_ETH == p_pkt_rx->mode)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nRX Packet by ETH \n");
    }
    else
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nRX Packet by DMA channel %d \n", p_pkt_rx->dma_chan);
    }

    CTC_ERROR_RETURN(_ctc_app_packet_sample_dump_info(&p_pkt_rx->rx_info, FALSE));

    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nPacket in %d buffers : Length %d, Header Length (ETH %d, Packet %d, Stacking %d)\n", p_pkt_rx->buf_count,
                p_pkt_rx->pkt_len, p_pkt_rx->eth_hdr_len, p_pkt_rx->pkt_hdr_len, p_pkt_rx->stk_hdr_len);
    CTC_ERROR_RETURN(_ctc_app_packet_sample_dump_pkt_buf(p_pkt_rx->pkt_buf, p_pkt_rx->buf_count));

    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "\nPacket in one consistent buffer : Length %d, Header Length (ETH %d, Packet %d)\n",
                p_pkt_rx->pkt_len, p_pkt_rx->eth_hdr_len, p_pkt_rx->pkt_hdr_len);
    CTC_ERROR_RETURN(_ctc_app_packet_sample_dump_raw_pkt(p_pkt_rx));

    return CTC_E_NONE;
}

int32
ctc_app_packet_sample_tx(ctc_pkt_tx_t* p_pkt_tx)
{
    app_pkt_sample_master_t* p_pkt_master = g_app_pkt_sample_master;
    int32 ret = CTC_E_NONE;

    if (!p_pkt_master->init_done)
    {
        return CTC_E_NOT_INIT;
    }

#ifdef _SAL_LINUX_UM
    /* call socket send */
    ret = send(p_pkt_master->eth.sock, p_pkt_tx->skb.data, p_pkt_tx->skb.len, 0);
    if (ret < 0)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,PKT_SAMPLE_STR "socket send failed!\n");
        return CTC_E_INVALID_PARAM;
    }

    if (ret != p_pkt_tx->skb.len)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,PKT_SAMPLE_STR "socket send len %d actual send len %d!\n", p_pkt_tx->skb.len, ret);
        return CTC_E_INVALID_PARAM;
    }
#endif

    p_pkt_master->eth.stats.tx++;

    return ret;
}

/* set promisc mode */
int32
_ctc_app_packet_sample_eth_set_promisc(char* ifname, int32 fd, uint32 enable)
{
#ifdef SDK_IN_USERMODE
    struct ifreq ifr;

    sal_strcpy(ifr.ifr_name, (char*)ifname);
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) == -1)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,PKT_SAMPLE_STR "socket ioctl failed!\n");
        return CTC_E_INVALID_PARAM;
    }

    if (enable)
    {
        ifr.ifr_flags |= IFF_PROMISC;
    }
    else
    {
        ifr.ifr_flags &= ~IFF_PROMISC;
    }

    if (ioctl(fd, SIOCSIFFLAGS, &ifr) == -1)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,PKT_SAMPLE_STR "socket ioctl failed!\n");
        return CTC_E_INVALID_PARAM;
    }
#endif
    return CTC_E_NONE;
}

/* open raw socket */
int32
ctc_app_packet_sample_eth_open_raw_socket(uint16 cpu_ifid)
{
    int32 ret = CTC_E_NONE;

#ifdef SDK_IN_USERMODE
    app_pkt_sample_master_t* p_pkt_master = g_app_pkt_sample_master;
    app_pkt_eth_t* p_eth = &p_pkt_master->eth;
    int32 if_idx = 0;
    struct ifreq ifr;
    struct sockaddr_ll sll;
    int32 sock = -1;

    sal_sprintf((char*)p_eth->ifname, "%s%d", "eth", cpu_ifid);

    p_eth->sock = -1;

    /* create socket */
    if ((sock = sal_socket(PF_PACKET, SOCK_RAW, sal_htons(ETH_P_ALL))) < 0)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,PKT_SAMPLE_STR "socket create failed!\n");
        return CTC_E_INVALID_PARAM;
    }

    ret = _ctc_app_packet_sample_eth_set_promisc(p_eth->ifname, sock, TRUE);
    if (ret < 0)
    {
        sal_close(sock);
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(&ifr, 0, sizeof(ifr));
    sal_strncpy(ifr.ifr_name, p_eth->ifname, sizeof(ifr.ifr_name));

    ret = ioctl(sock, SIOCGIFINDEX, &ifr);
    if (ret < 0)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,PKT_SAMPLE_STR "socket ioctl failed!\n");
        sal_close(sock);
        return CTC_E_INVALID_PARAM;
    }

    if_idx = ifr.ifr_ifindex;
    sal_memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = if_idx;
    sll.sll_protocol = sal_htons(ETH_P_ALL);
    ret = sal_bind(sock, (struct sockaddr*)&sll, sizeof(sll));
    if (ret < 0)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,PKT_SAMPLE_STR "socket bind failed!\n");
        sal_close(sock);
        return CTC_E_INVALID_PARAM;
    }

    p_eth->sock = sock;
    p_pkt_master->init_done = TRUE;
#endif
    return ret;
}

/* close raw socket */
int32
ctc_app_packet_sample_eth_close_raw_socket(void)
{
#ifdef SDK_IN_USERMODE

    app_pkt_sample_master_t* p_pkt_master = g_app_pkt_sample_master;
    app_pkt_eth_t* p_eth = &p_pkt_master->eth;

    if (p_pkt_master->init_done != TRUE)
    {
        return CTC_E_NOT_INIT;
    }

    _ctc_app_packet_sample_eth_set_promisc(p_eth->ifname, p_eth->sock, FALSE);
    sal_close(p_eth->sock);
    p_eth->sock = -1;
    p_pkt_master->init_done = FALSE;
#endif
    return CTC_E_NONE;
}

STATIC int32
_ctc_app_packet_sample_rx_handle(app_pkt_sample_master_t* p_pkt_master, ctc_pkt_rx_t* p_pkt_rx)
{
    CTC_ERROR_RETURN(ctc_packet_decap(p_pkt_rx));

    if (p_pkt_master->internal_rx_cb)
    {
        p_pkt_master->internal_rx_cb(p_pkt_rx);
    }

    if (p_pkt_master->rx_cb)
    {
        CTC_ERROR_RETURN(p_pkt_master->rx_cb(p_pkt_rx));
    }

    return CTC_E_NONE;
}

/* receive packet from socket */
STATIC int32
_ctc_app_packet_sample_eth_rx(app_pkt_sample_master_t* p_pkt_master, ctc_pkt_rx_t* p_pkt_rx)
{
    int32 n = 0;

    if (p_pkt_master->init_done != TRUE)
    {
        return CTC_E_NOT_INIT;
    }
#ifdef _SAL_LINUX_UM
    n = sal_recv(p_pkt_master->eth.sock, p_pkt_rx->pkt_buf[0].data, CTC_PKT_MTU, 0);
    if (n < 0)
    {
        return CTC_E_INVALID_PARAM;
    }
#endif
    p_pkt_rx->pkt_buf[0].len = n;
    p_pkt_rx->pkt_len = n;
    p_pkt_master->eth.stats.rx++;
    p_pkt_rx->mode = CTC_PKT_MODE_ETH;
    CTC_ERROR_RETURN(_ctc_app_packet_sample_rx_handle(p_pkt_master, p_pkt_rx));

    return CTC_E_NONE;
}

/* receive packet*/
STATIC void
_ctc_app_packet_sample_eth_rx_task(void* user_param)
{
    ctc_pkt_rx_t pkt_rx;
    ctc_pkt_rx_t* p_pkt_rx = &pkt_rx;
    ctc_pkt_buf_t pkt_buf[1];
    uint8* rx_buf = NULL;

    rx_buf = (uint8 *)sal_malloc(CTC_PKT_MTU);
    if(rx_buf == NULL)
    {
        return;

}

    sal_memset(&pkt_rx, 0, sizeof(pkt_rx));
    pkt_buf[0].data = rx_buf;
    p_pkt_rx->pkt_buf = pkt_buf;
    p_pkt_rx->buf_count = 1;

    while (1)
    {
        p_pkt_rx->pkt_buf[0].len = 0;
        p_pkt_rx->pkt_len = 0;
        _ctc_app_packet_sample_eth_rx(g_app_pkt_sample_master, p_pkt_rx);

        /* error processing */
        sal_task_sleep(50);
    }

    sal_free(rx_buf);

    return;
}

STATIC int32
_ctc_app_packet_sample_eth_init(app_pkt_sample_master_t* p_pkt_master)
{
    app_pkt_eth_t* p_eth = &p_pkt_master->eth;
    int32 ret = CTC_E_NONE;

    if (0 != sal_task_create(&p_eth->t_rx,
                             "AppPktRx",
                             32 * 1024, SAL_TASK_PRIO_DEF, _ctc_app_packet_sample_eth_rx_task, NULL))
    {
        sal_task_destroy(p_eth->t_rx);
        return CTC_E_NOT_INIT;
    }

    return ret;
}

int32
ctc_app_packet_sample_rx(ctc_pkt_rx_t* p_pkt_rx)
{
    if (NULL == g_app_pkt_sample_master)
    {
        return CTC_E_NOT_INIT;
    }

    /* dump packet */
    if (g_app_pkt_sample_master->rx_print_en)
    {
        CTC_ERROR_RETURN(_ctc_app_packet_sample_dump_rx(p_pkt_rx));
    }

    return CTC_E_NONE;
}

int32
ctc_app_packet_sample_show()
{
    app_pkt_sample_master_t* p_pkt_master = g_app_pkt_sample_master;
    app_pkt_eth_t* p_eth = NULL;

    if (NULL == p_pkt_master)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, PKT_SAMPLE_STR "does not initialized!\n");
        return CTC_E_NOT_INIT;
    }

    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Init Done  : %s\n", (p_pkt_master->init_done) ? "Yes" : "No");
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "RX Print   : %s\n", (p_pkt_master->rx_print_en) ? "On" : "Off");

    p_eth = &p_pkt_master->eth;
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Socket     : %d\n", p_eth->sock);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "ETH-ifname : %s\n", p_eth->ifname);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "TX Packet  : %d\n", p_eth->stats.tx);
    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "RX Packet  : %d\n", p_eth->stats.rx);

    return CTC_E_NONE;
}

int32
ctc_app_packet_sample_set_rx_print_en(uint32 enable)
{
    if (NULL == g_app_pkt_sample_master)
    {
        return CTC_E_NOT_INIT;
    }

    g_app_pkt_sample_master->rx_print_en = enable;
    return CTC_E_NONE;
}

int32
ctc_app_packet_eth_init(void)
{
    int32 ret = CTC_E_NONE;

    /* Init global pointer variable */
    if (NULL != g_app_pkt_sample_master)
    {
        return CTC_E_NONE;
    }

    MALLOC_POINTER(app_pkt_sample_master_t, g_app_pkt_sample_master);
    if (NULL == g_app_pkt_sample_master)
    {
        return CTC_E_INVALID_PARAM;
    }

    sal_memset(g_app_pkt_sample_master, 0, sizeof(app_pkt_sample_master_t));

    /* register socket TX/RX callback */
    g_app_pkt_sample_master->rx_cb = ctc_app_packet_sample_rx;

#ifdef _SAL_LINUX_UM
    ret = _ctc_app_packet_sample_eth_init(g_app_pkt_sample_master);
    if (ret)
    {
        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Eth init fail, ret:%d \n", ret);
    }
#endif
    return ret;
}

#ifdef _SAL_LINUX_UM
STATIC int32 _ctc_app_packet_netif_create(char* eth_name, uint8 rebind)
{
    uint32 sock_fd = 0;
    int32 ret;
    uint8 find = 0;
    uint8 netif_idx = 0;
    app_pkt_netif_info_t  *pkt_netif = NULL;
    app_pkt_sample_master_t* p_pkt_master = g_app_pkt_sample_master;

    for (netif_idx = 0; netif_idx < p_pkt_master->netif_count; netif_idx++)
    {
        if (!sal_strcmp(p_pkt_master->pkt_netif[netif_idx]->ifr.ifr_name, eth_name))
        {
            if (rebind)
            {
                find = 1;
                break;
            }
            else
            {
                return netif_idx;
            }
        }
    }

    if (find)
    {
        pkt_netif = p_pkt_master->pkt_netif[netif_idx];
        sal_close(pkt_netif->sock_fd);
    }
    else
    {
        netif_idx = p_pkt_master->netif_count++;
        pkt_netif = sal_malloc(sizeof(app_pkt_netif_info_t));
        if (!pkt_netif)
        {
            ret = -1;
            goto error;
        }
    }
    sal_memset(pkt_netif, 0, sizeof(app_pkt_netif_info_t));

    sock_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock_fd <= 0)
    {
        sal_printf("create socket error: %s.\n", strerror(errno));
        ret = -1;
        goto error;
    }

    sal_strcpy(pkt_netif->ifr.ifr_name, eth_name);
    ret = ioctl(sock_fd, SIOCGIFINDEX, &pkt_netif->ifr);
    if (ret)
    {
        sal_printf("get netif %s ifindex fail. %s.\n", pkt_netif->ifr.ifr_name, strerror(errno));
        ret =  -1;
        goto error;
    }
    sal_printf("create socket fd %d on netif %s ifindex %d.\n", sock_fd, eth_name, pkt_netif->ifr.ifr_ifindex);

    pkt_netif->sock_fd = sock_fd;

    pkt_netif->sl.sll_family = AF_PACKET;
    pkt_netif->sl.sll_ifindex = pkt_netif->ifr.ifr_ifindex;
    pkt_netif->sl.sll_protocol = htons(ETH_P_ALL);
    pkt_netif->sl.sll_halen = 6;

    if (bind(sock_fd, (struct sockaddr *)&pkt_netif->sl, sizeof(struct sockaddr_ll)))
    {
        sal_printf("bind socket error: %s.\n", strerror(errno));
        ret = -1;
        goto error;
    }
    p_pkt_master->pkt_netif[netif_idx] = pkt_netif;

    return netif_idx;

error:
    sal_close(sock_fd);
    sal_free(pkt_netif);
    p_pkt_master->netif_count--;
    return ret;
}

STATIC void
_ctc_app_packet_netif_rx_thread(void *param)
{
#define MAX_NETIF_RX_BUF_LEN     2048
    struct sockaddr_ll sl;
    uint16 index, rcv_len, max_fd = 0;
    uint32 fromlen = sizeof(struct sockaddr_ll);
    int32 rv = 0;
    uint8 * rx_pkt;
    fd_set fds;
    struct timeval timeout;
    app_pkt_sample_master_t* p_pkt_master = g_app_pkt_sample_master;

    if ((rx_pkt = sal_malloc(MAX_NETIF_RX_BUF_LEN)) == NULL)
    {
        return;
    }
    sal_memset(rx_pkt, 0, MAX_NETIF_RX_BUF_LEN);

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    while (1)
    {
        FD_ZERO(&fds);
        for (index = 0; index < MAX_APP_NETIF_NUM; index++)
        {
            if (!p_pkt_master->pkt_netif[index] || (p_pkt_master->pkt_netif[index]->sock_fd <= 0))
            {
                continue;
            }

            FD_SET(p_pkt_master->pkt_netif[index]->sock_fd, &fds);
            max_fd = (p_pkt_master->pkt_netif[index]->sock_fd > max_fd) ? p_pkt_master->pkt_netif[index]->sock_fd : max_fd;
        }

        rv = select(max_fd + 1, &fds, NULL, NULL, &timeout);
        if (rv < 0)
        {
            /* error case */
            break;
        }
        else if (rv == 0)
        {
            /* timeout */
            continue;
        }
        else
        {
            for (index = 0; index < MAX_APP_NETIF_NUM; index++)
            {
                if (!p_pkt_master->pkt_netif[index] || (p_pkt_master->pkt_netif[index]->sock_fd <= 0))
                {
                    continue;
                }

                if (FD_ISSET(p_pkt_master->pkt_netif[index]->sock_fd, &fds))
                {
                    rcv_len = recvfrom(p_pkt_master->pkt_netif[index]->sock_fd, rx_pkt, MAX_NETIF_RX_BUF_LEN, 0, (struct sockaddr *)&sl, &fromlen);
                    if (p_pkt_master->rx_print_en)
                    {
                        sal_printf("netif %s recv packet len %d\n", p_pkt_master->pkt_netif[index]->ifr.ifr_name, rcv_len);
                        rcv_len = (rcv_len <= MAX_NETIF_RX_BUF_LEN) ? rcv_len : 256;
                        _ctc_app_packet_sample_dump8(rx_pkt, rcv_len);
                    }
                }
            }
        }
    }

    if (rx_pkt)
    {
        sal_free(rx_pkt);
    }
}

int32
ctc_app_packet_netif_rx(char* eth_name, uint8 rebind)
{
    int8 netif_idx = 0;
    app_pkt_sample_master_t* p_pkt_master = g_app_pkt_sample_master;

    netif_idx = _ctc_app_packet_netif_create(eth_name, rebind);
    if (netif_idx < 0)
    {
        return -1;
    }

    if (!p_pkt_master->netif_rx_task)
    {
        sal_task_create(&p_pkt_master->netif_rx_task, "app_packet_netif_rx_thread",
                                  SAL_DEF_TASK_STACK_SIZE, SAL_TASK_PRIO_LOW, _ctc_app_packet_netif_rx_thread, NULL);
    }

    return CTC_E_NONE;
}

int32
ctc_app_packet_netif_tx(char* eth_name, uint8 rebind, uint8 *tx_pkt, uint16 pkt_len)
{
    int8 netif_idx = 0;
    uint16 send_len = 0;
    app_pkt_sample_master_t* p_pkt_master = g_app_pkt_sample_master;

    netif_idx = _ctc_app_packet_netif_create(eth_name, rebind);
    if (netif_idx < 0)
    {
        return -1;
    }

    send_len = send(p_pkt_master->pkt_netif[netif_idx]->sock_fd, tx_pkt, pkt_len, 0);
    if (send_len != pkt_len)
    {
        sal_printf("send len: %d, packet len: %d, %s.\n", send_len, pkt_len, strerror(errno));
        return -1;
    }

    return 0;
}
#endif
