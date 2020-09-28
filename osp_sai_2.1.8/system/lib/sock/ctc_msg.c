
#include <stddef.h>
#include "sal.h"
#include "sal_socket.h"
#include "ctc_task.h"
#include "ctc_msg.h"
#include "ctc_sock.h"
#include "ctclib_memory.h"


extern ctc_sock_master_t *g_p_sock_master;

static uint32 g_ipc_debug_flag = FALSE;

#define IPC_DEBUG_OUT(fmt, args...) \
    if (g_ipc_debug_flag) \
        sal_printf(fmt, ##args);

const char *g_ipc_dest_desc[] = 
{
    "invalid",
    "ctl",
    "ccs",
    "cds",
    "chsm",
    "switch",
    "routed",
    "opm",
    "authd",
    "appcfg",
    "fea",
    "ovs-vswitchd",
    "dhcrelay",
    "dhcsnooping",
    "dhclient",
    "ptp",
    "bhm",
    "eventmgr",
    "ssm",
    "subscriber",
    "ncs",
    "dhcpd6",
    "dhclient6",
};
    

uint32
ctc_msg_get_uhdr_len(msg_oper_t oper)
{
    switch (oper)
    {
    case MSG_OPER_CONNECT:
        return MSG_HDR_CONNECT_SIZE;

    case MSG_OPER_SYNC:
        return MSG_HDR_SYNC_SIZE;

    case MSG_OPER_CFG:
        return MSG_HDR_CFG_SIZE;

    case MSG_OPER_DATA:
        return MSG_HDR_DATA_SIZE;

    case MSG_OPER_FEI:
        return MSG_HDR_FEI_SIZE;

    case MSG_OPER_PKT:
        return MSG_HDR_PKT_SIZE;

    case MSG_OPER_LCM:
        return MSG_HDR_LCM_SIZE;

    case MSG_OPER_SUBSCRIBE:
        return MSG_HDR_SUBSCRIBE_SIZE;
        
    case MSG_OPER_SUB_REQ:
        return MSG_HDR_SUB_REQ_SIZE;

    case MSG_OPER_SUB_RESP:
        return MSG_HDR_SUB_RESP_SIZE;

    case MSG_OPER_BHM:
        return MSG_HDR_BHM_SIZE;

    default:
        return 0;
    }
}

msg_dec_ret_t
ctc_msg_decode_hdr(uint8* data, uint32 length, ctc_msg_t* p_msg)
{
    msg_dec_ret_t ret;
    uint32  dec_length = length;
    int32   data_length = 0;
    uint8  *dec_data = data;
    uint32 *size = &dec_length;
    uint8 **ppnt = &dec_data;

    if (length < MSG_HDR_SIZE)
    {
        ret.consumed = 0;
        ret.code = MSG_DEC_MORE;
        return ret;
    }

    MSG_DECODE_GETC(p_msg->hdr.version);
    MSG_DECODE_GETC(p_msg->hdr.operation);
    MSG_DECODE_GETW(p_msg->hdr.length);
    MSG_DECODE_GETL(p_msg->hdr.seq);

    if (length < MSG_HDR_SIZE + ctc_msg_get_uhdr_len(p_msg->hdr.operation))
    {
        ret.consumed = 0;
        ret.code = MSG_DEC_MORE;
        return ret;
    }
    
    switch (p_msg->hdr.operation)
    {
    case MSG_OPER_CONNECT:
        MSG_DECODE_GETW(p_msg->u_hdr.connect.src);
        MSG_DECODE_GETW(p_msg->u_hdr.connect.pid);
        break;

    case MSG_OPER_SYNC:
        MSG_DECODE_GETW(p_msg->u_hdr.sync.oper);
        MSG_DECODE_GETC(p_msg->u_hdr.sync.dest);
        MSG_DECODE_GETC(p_msg->u_hdr.sync.src);
        MSG_DECODE_GETL(p_msg->u_hdr.sync.seq);
        break;

    case MSG_OPER_CFG:
        MSG_DECODE_GETC(p_msg->u_hdr.cfg.dest);
        MSG_DECODE_GETC(p_msg->u_hdr.cfg.src);
        MSG_DECODE_GETW(p_msg->u_hdr.cfg.pid);
        MSG_DECODE_GETC(p_msg->u_hdr.cfg.oper);
        MSG_DECODE_GETC(p_msg->u_hdr.cfg.type);
        MSG_DECODE_GETW(p_msg->u_hdr.cfg.id);
        MSG_DECODE_GETL(p_msg->u_hdr.cfg.cfg);
        break;

    case MSG_OPER_DATA:
        MSG_DECODE_GETC(p_msg->u_hdr.data.oper);
        MSG_DECODE_GETC(p_msg->u_hdr.data.type);
        MSG_DECODE_GETW(p_msg->u_hdr.data.tbl_id);
        MSG_DECODE_GETW(p_msg->u_hdr.data.ds_id);
        MSG_DECODE_GETW(p_msg->u_hdr.data.ds2_id);
        MSG_DECODE_GETL(p_msg->u_hdr.data.fields[0]);
        MSG_DECODE_GETL(p_msg->u_hdr.data.fields[1]);
        break;

    case MSG_OPER_FEI:
        MSG_DECODE_GETL(p_msg->u_hdr.fei.type);
        MSG_DECODE_GETL(p_msg->u_hdr.fei.flags);
        break;

    case MSG_OPER_PKT:
        MSG_DECODE_GETW(p_msg->u_hdr.pkt.ifindex);
        MSG_DECODE_GETW(p_msg->u_hdr.pkt.vid);
        MSG_DECODE_GETL(p_msg->u_hdr.pkt.type);
        MSG_DECODE_GETL(p_msg->u_hdr.pkt.sub_type);
        MSG_DECODE_GETL(p_msg->u_hdr.pkt.flags);
        break;

    case MSG_OPER_LCM:
        MSG_DECODE_GETL(p_msg->u_hdr.lcm.slot);
        break;

    case MSG_OPER_SUBSCRIBE:
        MSG_DECODE_GETC(p_msg->u_hdr.sub.type);
        MSG_DECODE_GETC(p_msg->u_hdr.sub.node_type);
        MSG_DECODE_GETW(p_msg->u_hdr.sub.tbl_id);
        MSG_DECODE_GETW(p_msg->u_hdr.sub.ds_id);
        MSG_DECODE_GETW(p_msg->u_hdr.sub.tbl_num);
        MSG_DECODE_GETL(p_msg->u_hdr.sub.field);
        MSG_DECODE_GETW(p_msg->u_hdr.sub.pid);
        MSG_DECODE_GETW(p_msg->u_hdr.sub.src);
        break;

    case MSG_OPER_SUB_REQ:
    case MSG_OPER_SUB_RESP:
        break;

    case MSG_OPER_BHM:
        MSG_DECODE_GETW(p_msg->u_hdr.bhm.src);
        MSG_DECODE_GETW(p_msg->u_hdr.bhm.pid);
        break;

    default:
        ret.consumed = 0;
        ret.code = MSG_DEC_FAIL;
    }

    if (p_msg->hdr.length > length)
    {
        ret.consumed = 0;
        ret.code = MSG_DEC_MORE;
    }
    else
    {
        ret.consumed = p_msg->hdr.length;
        ret.code = MSG_DEC_OK;
        data_length = p_msg->hdr.length - MSG_HDR_SIZE - ctc_msg_get_uhdr_len(p_msg->hdr.operation);
        p_msg->data = NULL;
        p_msg->data_len = 0;
        if (data_length > 0)
        {
            p_msg->data = XCALLOC(MEM_LIB_SOCK_DATA, data_length);
            p_msg->data_len = data_length;
            MSG_DECODE_GET(p_msg->data, p_msg->data_len);
        }
    }

    return ret;
}

const char*
ctc_msg_oper_desc(msg_oper_t oper)
{
    switch (oper)
    {
    case MSG_OPER_CONNECT:
        return "connect";
    case MSG_OPER_SYNC:
        return "sync";
    case MSG_OPER_CFG:
        return "cfg";
    case MSG_OPER_DATA:
        return "data";
    case MSG_OPER_FEI:
        return "fei";
    case MSG_OPER_PKT:
        return "pkt";
    case MSG_OPER_LCM:
        return "lcm";
    case MSG_OPER_SUBSCRIBE:
        return "subscribe";
    case MSG_OPER_SUB_REQ:
        return "sub_req";
    case MSG_OPER_SUB_RESP:
        return "sub_resp";
    case MSG_OPER_BHM:
        return "bhm";
    default:
        return "Unknown";
    }
}

int32
ctc_msg_dump(ctc_msg_t *p_msg, uint32 send)
{
    IPC_DEBUG_OUT("###### IPC %s msg %s ######\n", send ? "TX" : "RX",
        ctc_msg_oper_desc(p_msg->hdr.operation));
    IPC_DEBUG_OUT("version      %u\n", p_msg->hdr.version);
    IPC_DEBUG_OUT("length       %u\n", p_msg->hdr.length);
    IPC_DEBUG_OUT("seq          %u\n", p_msg->hdr.seq);

    switch (p_msg->hdr.operation)
    {
    case MSG_OPER_CONNECT:
        IPC_DEBUG_OUT("src          0x%X (%s)\n", p_msg->u_hdr.connect.src, IPC_DEST_DESC(p_msg->u_hdr.connect.src));
        IPC_DEBUG_OUT("pid          %u\n", p_msg->u_hdr.connect.pid);
        break;

    case MSG_OPER_SYNC:
        IPC_DEBUG_OUT("oper         %u\n", p_msg->u_hdr.sync.oper);
        IPC_DEBUG_OUT("dest         0x%X (%s)\n", p_msg->u_hdr.sync.dest, IPC_DEST_DESC(p_msg->u_hdr.sync.dest));
        IPC_DEBUG_OUT("src          0x%X (%s)\n", p_msg->u_hdr.sync.src, IPC_DEST_DESC(p_msg->u_hdr.sync.src));
        IPC_DEBUG_OUT("seq          %u\n", p_msg->u_hdr.sync.seq);
        break;

    case MSG_OPER_CFG:
        IPC_DEBUG_OUT("dest         0x%X (%s)\n", p_msg->u_hdr.cfg.dest, IPC_DEST_DESC(p_msg->u_hdr.cfg.dest));
        IPC_DEBUG_OUT("src          0x%X (%s)\n", p_msg->u_hdr.cfg.src, IPC_DEST_DESC(p_msg->u_hdr.cfg.src));
        IPC_DEBUG_OUT("pid          %u\n", p_msg->u_hdr.cfg.pid);
        IPC_DEBUG_OUT("oper         %u\n", p_msg->u_hdr.cfg.oper);
        IPC_DEBUG_OUT("type         %u\n", p_msg->u_hdr.cfg.type);
        IPC_DEBUG_OUT("id           %u\n", p_msg->u_hdr.cfg.id);
        IPC_DEBUG_OUT("cfg          %u\n", p_msg->u_hdr.cfg.cfg);
        break;
        
    case MSG_OPER_DATA:
        IPC_DEBUG_OUT("oper         %u\n", p_msg->u_hdr.data.oper);
        IPC_DEBUG_OUT("type         %u\n", p_msg->u_hdr.data.type);
        IPC_DEBUG_OUT("tbl_id       %u\n", p_msg->u_hdr.data.tbl_id);
        IPC_DEBUG_OUT("ds_id        %u\n", p_msg->u_hdr.data.ds_id);
        IPC_DEBUG_OUT("ds2_id       %u\n", p_msg->u_hdr.data.ds2_id);
        IPC_DEBUG_OUT("fields[0]    0x%08X\n", p_msg->u_hdr.data.fields[0]);
        IPC_DEBUG_OUT("fields[1]    0x%08X\n", p_msg->u_hdr.data.fields[1]);
        break;

    case MSG_OPER_FEI:
        IPC_DEBUG_OUT("type         %u\n", p_msg->u_hdr.fei.type);
        IPC_DEBUG_OUT("flags        0x%X\n", p_msg->u_hdr.fei.flags);
        break;

    case MSG_OPER_PKT:
        IPC_DEBUG_OUT("ifindex      0x%X\n", p_msg->u_hdr.pkt.ifindex);
        IPC_DEBUG_OUT("vid          %u\n", p_msg->u_hdr.pkt.vid);
        IPC_DEBUG_OUT("type         %u\n", p_msg->u_hdr.pkt.type);
        IPC_DEBUG_OUT("sub type     %u\n", p_msg->u_hdr.pkt.sub_type);
        IPC_DEBUG_OUT("flags        0x%X\n", p_msg->u_hdr.pkt.flags);
        break;

    case MSG_OPER_LCM:
        IPC_DEBUG_OUT("slot         %u\n", p_msg->u_hdr.lcm.slot);
        break;

    case MSG_OPER_SUBSCRIBE:
        IPC_DEBUG_OUT("type         %u\n", p_msg->u_hdr.sub.type);
        IPC_DEBUG_OUT("node_type    %u\n", p_msg->u_hdr.sub.node_type);
        IPC_DEBUG_OUT("tbl_id       %u\n", p_msg->u_hdr.sub.tbl_id);
        IPC_DEBUG_OUT("ds_id        %u\n", p_msg->u_hdr.sub.ds_id);
        IPC_DEBUG_OUT("tbl_num      %u\n", p_msg->u_hdr.sub.tbl_num);
        IPC_DEBUG_OUT("field        %u\n", p_msg->u_hdr.sub.field);
        IPC_DEBUG_OUT("pid          %u\n", p_msg->u_hdr.sub.pid);
        IPC_DEBUG_OUT("src          %u\n", p_msg->u_hdr.sub.src);
        break;

    case MSG_OPER_SUB_REQ:
    case MSG_OPER_SUB_RESP:
        break;

    case MSG_OPER_BHM:
        IPC_DEBUG_OUT("src          0x%X (%s)\n", p_msg->u_hdr.bhm.src, IPC_DEST_DESC(p_msg->u_hdr.bhm.src));
        IPC_DEBUG_OUT("pid          %u\n", p_msg->u_hdr.bhm.pid);
        break;

    default:
        return -1;
    }
    
    IPC_DEBUG_OUT("###### IPC msg end ######\n");
    
    return 0;
}

static int32
ctc_msg_send_connect_resp(ctc_sock_t *p_sock, uint32 peer)
{
    ctc_msg_t   req_msg;
    ctc_msg_t  *p_req_msg = &req_msg;
    int32 rc = 0;

    /* not resp to PM_ID_CTL */
    if (1 == (peer & 0xFFFF))
    {
        return 0;
    }
    
    if (!p_sock->is_server)
    {
        return 0;
    }
    
    sal_memset(p_req_msg, 0, sizeof(ctc_msg_t));
    MSG_BUILD_HDR(p_req_msg, MSG_OPER_CONNECT, 0);
    p_req_msg->u_hdr.connect.src = p_sock->pmid;
    p_req_msg->u_hdr.connect.pid = 0;    
    rc = ctc_sock_send(p_sock->sock, peer, p_req_msg);
    
    return rc;
}

int32
_ctc_msg_handle(ctc_sock_session_t* p_session, ctc_msg_t *p_msg)
{
    ctc_sock_t *p_sock = (ctc_sock_t*)p_session->p_sock;

    if (!p_sock)
    {
        return -1;
    }

    ctc_msg_dump(p_msg, FALSE);

    if (MSG_OPER_CONNECT != p_msg->hdr.operation)
    {
        /* check seq */
        if ((p_session->rx_msg_seq + 1) != p_msg->hdr.seq)
        {
            p_session->seq_mismatch++;
        }
        p_session->rx_msg_seq = p_msg->hdr.seq;
    }
    
    switch (p_msg->hdr.operation)
    {
    case MSG_OPER_CONNECT:
        p_session->peer = SOCK_BUILD_PEER(p_msg->u_hdr.connect.pid, p_msg->u_hdr.connect.src);
        ctc_msg_send_connect_resp(p_sock, p_session->peer);
        break;

    case MSG_OPER_SYNC:
        break;

    case MSG_OPER_CFG:
        break;

    case MSG_OPER_DATA:
        break;

    case MSG_OPER_FEI:
        break;

    case MSG_OPER_PKT:
        break;

    case MSG_OPER_LCM:
        break;

    case MSG_OPER_SUBSCRIBE:
        break;

    case MSG_OPER_SUB_REQ:
        break;

    case MSG_OPER_SUB_RESP:
        break;

    case MSG_OPER_BHM:
        break;

    default:
        return -1;
    }
    
    if (g_p_sock_master->rx_fn)
    {
        g_p_sock_master->rx_fn(p_sock->sock, p_session->fd, p_msg);
    }
    
    return 0;
}

int32
ctc_msg_decode(void* p_sess, int32 read_len)
{
    ctc_sock_session_t* p_session = p_sess;
    ctc_msg_t msg;
    int32 left = 0;
    msg_dec_ret_t dec_rc;
    uint8* p = NULL;
    
    p_session->rx_buf_len += read_len;
    p = p_session->rx_buf;
    left = p_session->rx_buf_len;

    msg.data_len = 0;
    msg.data1_len = 0;
    msg.data2_len = 0;
    
    for (;;)
    {
        dec_rc = ctc_msg_decode_hdr(p, left, &msg);
        switch (dec_rc.code)
        {
        case MSG_DEC_OK:
            msg.peer = p_session->peer;
            _ctc_msg_handle(p_session, &msg);
            ctc_msg_free(&msg);
            p += dec_rc.consumed;
            left -= dec_rc.consumed;
            if (left == 0)
            {
                p_session->rx_buf_len = 0;
                return 0;
            }
            break;

        case MSG_DEC_MORE:
            if (left > 0 && p_session->rx_buf != p)
            {
                sal_memmove(p_session->rx_buf, p, left);
            }

            p_session->rx_buf_len = left;
            return 0;

        case MSG_DEC_FAIL:
            return -1;
        }
    }
}

int32
ctc_msg_free(ctc_msg_t *p_msg)
{
    if (p_msg->data)
    {
        XFREE(MEM_LIB_SOCK_DATA, p_msg->data);
        p_msg->data = NULL;
    }
    return 0;
}

