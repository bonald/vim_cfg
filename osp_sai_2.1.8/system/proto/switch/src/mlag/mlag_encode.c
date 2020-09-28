
#include "proto.h"
#include "switch.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_lag_global_define.h"
#include "gen/tbl_lag_global.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_sys_global.h"
#include "gen/tbl_mlag_define.h"
#include "gen/tbl_mlag.h"
#include "gen/tbl_mlag_peer_define.h"
#include "gen/tbl_mlag_peer.h"
#include "gen/tbl_mlag_port_define.h"
#include "gen/tbl_mlag_port.h"
#include "gen/tbl_errdisable_define.h"
#include "gen/tbl_errdisable.h"
#include "mlag.h"
#include "mlag_api.h"
#include "mlag_sock.h"

typedef struct
{
    int32   data_length;
    int32   size_left;
    int32  *size;
    uint8  *pnt;
    uint8  data[MLAG_MAX_PACKET_SIZE];
} mlag_buf_t;

static int32
_mpe_buf_init(mlag_buf_t *buf, uint32 length)
{
    buf->size_left = MLAG_MAX_PACKET_SIZE;
    buf->size = &buf->size_left;
    buf->pnt = buf->data;
    buf->data_length = length;
    
    return PM_E_NONE;
}

static int32
_mlag_peer_get_connected_sock(tbl_mlag_peer_t *peer)
{
    mlag_sock_connected_t connected = mpn_sock_check_connected(peer);
    
    switch (connected)
    {
    case MLAG_SOCK_CONNECTED_BOTH:
    case MLAG_SOCK_CONNECTED_SERVER:
        return peer->server_sock;
    case MLAG_SOCK_CONNECTED_CLIENT:
        return peer->client_sock;
    default:
        return INVALID_SOCK_ID;
    }
}

static int32
_mpe_msg_hdr(mlag_buf_t *buf, tbl_mlag_peer_t *peer, uint8 type)
{
    int32 **size = &buf->size;
    uint8 **ppnt = &buf->pnt;
    
    MSG_ENCODE_PUTC(MLAG_VERSION_1);
    MSG_ENCODE_PUTC(type);
    MSG_ENCODE_PUTW(buf->data_length);
    
    return PM_E_NONE;
}

static int32
_mpe_msg_open(mlag_buf_t *buf, tbl_mlag_peer_t *peer)
{
    tbl_mlag_t *mlag = tbl_mlag_get_mlag();
    int32 **size = &buf->size;
    uint8 **ppnt = &buf->pnt;
    
    MSG_ENCODE_PUT(mlag->local_sysid, MAC_ADDR_LEN);
    MSG_ENCODE_PUTW(peer->holdtime);

    return PM_E_NONE;
}

static int32
_mpe_msg_failover(mlag_buf_t *buf, tbl_mlag_peer_t *peer, uint32 mlagid, uint32 is_recover)
{
    int32 **size = &buf->size;
    uint8 **ppnt = &buf->pnt;
    
    MSG_ENCODE_PUTW(mlagid);
    MSG_ENCODE_PUTC(is_recover);
    MSG_ENCODE_PUTC(MLAG_MSG_RESERVED);

    return PM_E_NONE;
}

static int32
_mpe_msg_conf(mlag_buf_t *buf, tbl_mlag_peer_t *peer, uint32 mlagid, uint32 is_conf, uint32 is_up)
{
    int32 **size = &buf->size;
    uint8 **ppnt = &buf->pnt;
    
    MSG_ENCODE_PUTW(mlagid);
    MSG_ENCODE_PUTC(is_conf);
    MSG_ENCODE_PUTC(is_up);

    return PM_E_NONE;
}

static int32
_mpe_msg_fdbsync(mlag_buf_t *buf, tbl_mlag_peer_t *peer, glb_mlag_fdb_sync_t *sync)
{
    uint32 i = 0;
    int32 **size = &buf->size;
    uint8 **ppnt = &buf->pnt;
    
    MSG_ENCODE_PUTC(sync->count);

    for (i = 0; i < sync->count; i++)
    {
        MSG_ENCODE_PUTW(sync->array[i].fid);
        MSG_ENCODE_PUT(sync->array[i].mac, MAC_ADDR_LEN);
        MSG_ENCODE_PUTW(sync->array[i].mlag_id);
        if (GLB_FLAG_ISSET(sync->array[i].flag, GLB_FDB_FLAG_STATIC))
        {
            MSG_ENCODE_PUTC(MLAG_MSG_FDBSYNC_TYPE_STATIC);
        }
        else
        {
            MSG_ENCODE_PUTC(MLAG_MSG_FDBSYNC_TYPE_DYNAMIC);
        }
        MSG_ENCODE_PUTC(sync->array[i].act);

        switch (sync->array[i].act)
        {
        case GLB_MLAG_FDB_SYNC_ADD:
        case GLB_MLAG_FDB_SYNC_DEL:
            MLAG_DEBUG(MLAG_PACKET, "Encode FDB-sync, action: %s, vid: %u, mac: %02x%02x.%02x%02x.%02x%02x, type: %s",
                MLAG_SYNC_FDB_ACT_STR(sync->array[i].act),
                sync->array[i].fid, 
                sync->array[i].mac[0], sync->array[i].mac[1], sync->array[i].mac[2],
                sync->array[i].mac[3], sync->array[i].mac[4], sync->array[i].mac[5], 
                GLB_FLAG_ISSET(sync->array[i].flag, GLB_FDB_FLAG_STATIC) ? "static" : "dynamic");
            break;

        case GLB_MLAG_FDB_SYNC_FLUSH_ALL:
            MLAG_DEBUG(MLAG_PACKET, "Encode FDB-sync, action: %s",
                MLAG_SYNC_FDB_ACT_STR(sync->array[i].act));
            break;

        case GLB_MLAG_FDB_SYNC_FLUSH_PORT:
            MLAG_DEBUG(MLAG_PACKET, "Encode FDB-sync, action: %s, mlag_id: %u",
                MLAG_SYNC_FDB_ACT_STR(sync->array[i].act), sync->array[i].mlag_id);
            break;
            
        case GLB_MLAG_FDB_SYNC_FLUSH_VLAN:
            MLAG_DEBUG(MLAG_PACKET, "Encode FDB-sync, action: %s, vid: %u",
                MLAG_SYNC_FDB_ACT_STR(sync->array[i].act), sync->array[i].fid);
            break;
            
        case GLB_MLAG_FDB_SYNC_FLUSH_MAC:
            MLAG_DEBUG(MLAG_PACKET, "Encode FDB-sync, action: %s, mac: %02x%02x.%02x%02x.%02x%02x",
                MLAG_SYNC_FDB_ACT_STR(sync->array[i].act), 
                sync->array[i].mac[0], sync->array[i].mac[1], sync->array[i].mac[2],
                sync->array[i].mac[3], sync->array[i].mac[4], sync->array[i].mac[5]);
            break;

        default:
            break;
        }
    }

    return PM_E_NONE;
}

static int32
_mpe_msg_stp_global_info(mlag_buf_t *buf, tbl_mlag_peer_t *peer, 
        uint8 enable, uint8 type, uint16 prio, uint8 *addr)
{
    /* Message format:
    +----------------+----------------+---------------------------------+
    |   Version      |   Type Field   |          Packet Length          |
    +----------------+----------------+---------------------------------+
    |   Sub Type 1   |      enable    |            propriety            |
    +----------------+----------------+---------------------------------+
    |                   MAC Address (high 32 bit)                       |
    +---------------------------------+----------------+----------------+
    |     MAC Address (low 16 bit)    |    stp type    |
    +---------------------------------+----------------+
    */
    
    int32 **size = &buf->size;
    uint8 **ppnt = &buf->pnt;
    
    MSG_ENCODE_PUTC(MLAG_MSG_STP_GLOBAL);
    MSG_ENCODE_PUTC(enable);
    MSG_ENCODE_PUTW(prio);
    MSG_ENCODE_PUT(addr, MAC_ADDR_LEN);
    MSG_ENCODE_PUTC(type);

    return PM_E_NONE;
}

static int32
_mpe_msg_stp_port_info(mlag_buf_t *buf, tbl_mlag_peer_t *peer, uint8 enable, uint16 mlagid)
{
    /* Message format:
    +----------------+----------------+---------------------------------+
    |   Version      |   Type Field   |          Packet Length          |
    +----------------+----------------+---------------------------------+
    |   Sub Type 1   |      enable    |            mlagid               |
    +----------------+----------------+---------------------------------+
    */

    int32 **size = &buf->size;
    uint8 **ppnt = &buf->pnt;
    
    MSG_ENCODE_PUTC(MLAG_MSG_STP_PORT);
    MSG_ENCODE_PUTC(enable);
    MSG_ENCODE_PUTW(mlagid);

    return PM_E_NONE;
}

static int32
_mpe_msg_stp_bpdu_packet(mlag_buf_t *buf, tbl_mlag_peer_t *peer, uint16 mlagid, 
    l2_socket_addr_t *addr, uint32 length, uint8 *pkt)
{
    /* Message format:
    +----------------+----------------+----------------+----------------+
    |   Version      |   Type Field   |          Packet Length          |
    +----------------+----------------+----------------+----------------+
    |   Sub Type     |              mlag id            |  
    +-------------------------------------------------------------------+
    |                          struct sockaddr_vlan                     |
    |                                 ~                                 |
    +-------------------------------------------------------------------+
    |                     bpdu payload lenth                            |
    +-------------------------------------------------------------------+
    |                            bpdu payload                           |
    |                                 ~                                 |
    |                                                                   
    |                                                   
    +-------------------------------------------------------------------+
    */

    int32 **size = &buf->size;
    uint8 **ppnt = &buf->pnt;
    
    MSG_ENCODE_PUTC(MLAG_MSG_STP_PACKET);
    MSG_ENCODE_PUTW(mlagid);
    MSG_ENCODE_PUT(addr->dest_mac, MAC_ADDR_LEN);
    MSG_ENCODE_PUT(addr->src_mac, MAC_ADDR_LEN);
    MSG_ENCODE_PUTL(addr->port);
    MSG_ENCODE_PUTW(addr->vlanid);
    MSG_ENCODE_PUTW(addr->svlanid);
    MSG_ENCODE_PUTW(addr->type);
    MSG_ENCODE_PUTW(addr->reserve);
    MSG_ENCODE_PUTL(length);
    MSG_ENCODE_PUT(pkt, length);

    return PM_E_NONE;
}

static int32
_mpe_msg_stp_state(mlag_buf_t *buf, tbl_mlag_peer_t *peer, uint16 mlagid, uint16 instance, uint8 state)
{
    /* Message format:
    +----------------+----------------+----------------+----------------+
    |   Version      |   Type Field   |          Packet Length          |
    +----------------+----------------+----------------+----------------+
    |   Sub Type     |              mlag id            |   instance id  |
    +----------------+----------------+----------------+----------------+
    |  instance id   | state          | 
    +---------------------------------+
    */
    
    int32 **size = &buf->size;
    uint8 **ppnt = &buf->pnt;
    
    MSG_ENCODE_PUTC(MLAG_MSG_STP_STATE);
    MSG_ENCODE_PUTW(mlagid);
    MSG_ENCODE_PUTW(instance);
    MSG_ENCODE_PUTC(state);

    return PM_E_NONE;
}

static int32
_mpe_msg_stp_instance(mlag_buf_t *buf, tbl_mlag_peer_t *peer, uint16 instance)
{
    /* Message format:
    +----------------+----------------+----------------+----------------+
    |   Version      |   Type Field   |          Packet Length          |
    +----------------+----------------+----------------+----------------+
    |   Sub Type     |              instance id        |
    +----------------+---------------------------------+
    */
    
    int32 **size = &buf->size;
    uint8 **ppnt = &buf->pnt;
    
    MSG_ENCODE_PUTC(MLAG_MSG_STP_INST);
    MSG_ENCODE_PUTW(instance);

    return PM_E_NONE;
}

static int32
_mpe_msg_syspri(mlag_buf_t *buf, tbl_mlag_peer_t *peer)
{
    tbl_mlag_t *mlag = tbl_mlag_get_mlag();
    int32 **size = &buf->size;
    uint8 **ppnt = &buf->pnt;
    
    MSG_ENCODE_PUTW(mlag->local_syspri);

    return PM_E_NONE;
}

static int32
_mlag_send(tbl_mlag_peer_t *peer, mlag_buf_t *buf)
{
    int32 sock = 0;
    int32 rc = PM_E_NONE;
    sock = _mlag_peer_get_connected_sock(peer);
    if (sock < 0)
    {
        return PM_E_FAIL;
    }
    
    rc = ctc_sock_send_raw(sock, IGNORE_SOCK_PEER, buf->data, buf->data_length);
    if (rc < 0)
    {
        /* Added by kcao for 41194 2016-10-26, close MLAG connection when send fail */
        log_sys(M_MOD_INTERNAL, E_ERROR, "MLAG socket transmit failed, close the current connection");
        mpf_process_event(MPF_EVENT_SOCK_TX_FAIL);
    }

    return rc;
}

int32
mlag_send_open(tbl_mlag_peer_t *peer)
{
    mlag_buf_t buf;
    int32 rc = PM_E_NONE;

    _mpe_buf_init(&buf, MLAG_MSG_OPEN_MAX_SIZE);

    /* Encode the Message Header */
    _mpe_msg_hdr(&buf, peer, MLAG_MSG_OPEN);

    /* Encode Open Message */
    _mpe_msg_open(&buf, peer);

    rc = (_mlag_send(peer, &buf));
    if (rc < 0)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "send_open fail, connected is %u", mpn_sock_check_connected(peer));
        return rc;
    }

    /* Count this Keepalive */
    peer->open_out++;
    
    MLAG_DEBUG(MLAG_PACKET, "Send Open, count is %u", peer->open_out);

    return rc;
}

int32
mlag_send_keepalive(tbl_mlag_peer_t *peer)
{
    mlag_buf_t buf;
    int32 rc = PM_E_NONE;

    _mpe_buf_init(&buf, MLAG_MSG_KEEPALIVE_MAX_SIZE);

    /* Encode the Message Header */
    _mpe_msg_hdr(&buf, peer, MLAG_MSG_KEEPALIVE);

    PM_E_RETURN(_mlag_send(peer, &buf));

    /* Count this Keepalive */
    peer->keepalive_out++;
    
    MLAG_DEBUG(MLAG_PACKET, "Send Keepalive, count is %u", peer->keepalive_out);
    
    return rc;
}

int32
mlag_send_disconnect(tbl_mlag_peer_t *peer)
{
    mlag_buf_t buf;
    int32 rc = PM_E_NONE;

    _mpe_buf_init(&buf, MLAG_MSG_DISCONNECT_MAX_SIZE);

    /* Encode the Message Header */
    _mpe_msg_hdr(&buf, peer, MLAG_MSG_DISCONNECT);

    PM_E_RETURN(_mlag_send(peer, &buf));

    /* Count this Keepalive */
    peer->disconnect_out++;
    
    MLAG_DEBUG(MLAG_PACKET, "Send Disconnect, count is %u", peer->disconnect_out);
    
    return rc;
}

int32
mlag_send_failover(tbl_mlag_peer_t *peer, uint32 mlagid, uint32 is_recover)
{
    mlag_buf_t buf;
    int32 rc = PM_E_NONE;

    _mpe_buf_init(&buf, MLAG_MSG_FAILOVER_MAX_SIZE);

    /* Encode the Message Header */
    _mpe_msg_hdr(&buf, peer, MLAG_MSG_FAILOVER);

    /* Encode Open Message */
    _mpe_msg_failover(&buf, peer, mlagid, is_recover);

    PM_E_RETURN(_mlag_send(peer, &buf));

    /* Count this Keepalive */
    peer->failover_out++;
    
    MLAG_DEBUG(MLAG_PACKET, "Send Fail-over mlagid: %u, is_recover: %u, count is %u", 
        mlagid, is_recover, peer->failover_out);
    
    return rc;
}

int32
mlag_send_conf(tbl_mlag_peer_t *peer, uint32 mlagid, uint32 is_conf, uint32 is_up)
{
    mlag_buf_t buf;
    int32 rc = PM_E_NONE;

    _mpe_buf_init(&buf, MLAG_MSG_CONF_MAX_SIZE);

    /* Encode the Message Header */
    _mpe_msg_hdr(&buf, peer, MLAG_MSG_CONF);

    /* Encode Open Message */
    _mpe_msg_conf(&buf, peer, mlagid, is_conf, is_up);

    PM_E_RETURN(_mlag_send(peer, &buf));

    /* Count Conf */
    peer->conf_out++;
    
    MLAG_DEBUG(MLAG_PACKET, "Send Conf, mlagid: %u, is_conf: %u, is_up: %u, count is %u", 
        mlagid, is_conf, is_up, peer->conf_out);
    
    return rc;
}

int32
mlag_send_fdbsync(tbl_mlag_peer_t *peer, glb_mlag_fdb_sync_t *sync)
{
    mlag_buf_t buf;
    int32 rc = PM_E_NONE;

    _mpe_buf_init(&buf, MLAG_MSG_FDBSYNC_HDR_SIZE + MLAG_MSG_FDBSYNC_BLK_SIZE * sync->count);

    /* Encode the Message Header */
    _mpe_msg_hdr(&buf, peer, MLAG_MSG_FDBSYNC);

    /* Encode Open Message */
    _mpe_msg_fdbsync(&buf, peer, sync);

    PM_E_RETURN(_mlag_send(peer, &buf));

    /* Count this FDBSYNC Message */
    peer->fdbsync_out++;
    
    MLAG_DEBUG(MLAG_PACKET, "Send FDB-sync, count is %u", peer->fdbsync_out);
    
    return rc;
}

int32
mlag_send_stp_global_info(tbl_mlag_peer_t *peer, uint8 enable, uint8 type, uint16 prio, uint8 *addr)
{
    mlag_buf_t buf;
    int32 rc = PM_E_NONE;

    _mpe_buf_init(&buf, MLAG_MSG_STP_GLOBAL_SIZE);

    /* Encode the Message Header */
    _mpe_msg_hdr(&buf, peer, MLAG_MSG_STP);

    /* Encode Open Message */
    _mpe_msg_stp_global_info(&buf, peer, enable, type, prio, addr);

    PM_E_RETURN(_mlag_send(peer, &buf));

    /* Count this Keepalive */
    peer->stp_out++;
    peer->stp_global_out++;   
    
    MLAG_DEBUG(MLAG_PACKET, "Send STP-global-info, count is %u", peer->stp_global_out);
    
    return rc;
}

int32
mlag_send_stp_port_info(tbl_mlag_peer_t *peer, uint8 enable, uint16 mlagid)
{
    mlag_buf_t buf;
    int32 rc = PM_E_NONE;

    _mpe_buf_init(&buf, MLAG_MSG_STP_PORT_SIZE);

    /* Encode the Message Header */
    _mpe_msg_hdr(&buf, peer, MLAG_MSG_STP);

    /* Encode Open Message */
    _mpe_msg_stp_port_info(&buf, peer, enable, mlagid);

    PM_E_RETURN(_mlag_send(peer, &buf));

    /* Count this Keepalive */
    peer->stp_out++;
    peer->stp_port_out++;   

    MLAG_DEBUG(MLAG_PACKET, "Send STP-port-info, mlagid: %u, enable: %u, count is %u", 
        mlagid, enable, peer->stp_port_out);

    return rc;
}

int32
mlag_send_stp_state(tbl_mlag_peer_t *peer, uint16 mlagid, uint16 instance, uint8 state)
{
    mlag_buf_t buf;
    int32 rc = PM_E_NONE;

    _mpe_buf_init(&buf, MLAG_MSG_STP_STATE_SIZE);

    /* Encode the Message Header */
    _mpe_msg_hdr(&buf, peer, MLAG_MSG_STP);

    /* Encode Open Message */
    _mpe_msg_stp_state(&buf, peer, mlagid, instance, state);

    PM_E_RETURN(_mlag_send(peer, &buf));

    /* Count this Keepalive */
    peer->stp_out++;
    peer->stp_state_out++;

    MLAG_DEBUG(MLAG_PACKET, "Send STP-state-info, mlagid: %u, instance: %u, state: %u, count is %u", mlagid, instance, state, peer->stp_state_out);

    return rc;
}

int32
mlag_send_stp_bpdu_packet(tbl_mlag_peer_t *peer, uint16 mlagid, l2_socket_addr_t *addr, uint32 length, uint8 *pkt)
{
    mlag_buf_t buf;
    int32 rc = PM_E_NONE;

    _mpe_buf_init(&buf, MLAG_MSG_STP_BPDU_PKT_HEADER_SIZE+length);

    /* Encode the Message Header */
    _mpe_msg_hdr(&buf, peer, MLAG_MSG_STP);

    /* Encode Open Message */
    _mpe_msg_stp_bpdu_packet(&buf, peer, mlagid, addr, length, pkt);

    PM_E_RETURN(_mlag_send(peer, &buf));

    /* Count this Keepalive */
    peer->stp_out++;
    peer->stp_packet_out++;   
    
    MLAG_DEBUG(MLAG_PACKET, "Send STP-packet, mlagid: %u, count is %u", mlagid, peer->stp_packet_out);
    
    return rc;
}

int32
mlag_send_stp_instance(tbl_mlag_peer_t *peer, uint16 instance)
{
    mlag_buf_t buf;
    int32 rc = PM_E_NONE;

    _mpe_buf_init(&buf, MLAG_MSG_STP_INSTANCE_SIZE);

    /* Encode the Message Header */
    _mpe_msg_hdr(&buf, peer, MLAG_MSG_STP);

    /* Encode Open Message */
    _mpe_msg_stp_instance(&buf, peer, instance);

    PM_E_RETURN(_mlag_send(peer, &buf));

    /* Count this Keepalive */
    peer->stp_out++;
    peer->stp_instance_out++;   

    MLAG_DEBUG(MLAG_PACKET, "Send STP-instance-info, instance: %u, count is %u", instance, peer->stp_instance_out);

    return rc;
}

int32
mlag_send_syspri(tbl_mlag_peer_t *peer)
{
    mlag_buf_t buf;
    int32 rc = PM_E_NONE;

    _mpe_buf_init(&buf, MLAG_MSG_SYSPRI_MAX_SIZE);

    /* Encode the Message Header */
    _mpe_msg_hdr(&buf, peer, MLAG_MSG_SYSPRI);

    /* Encode syspri Message */
    _mpe_msg_syspri(&buf, peer);

    PM_E_RETURN(_mlag_send(peer, &buf));

    /* Count this Keepalive */
    peer->syspri_out++;
    
    MLAG_DEBUG(MLAG_PACKET, "Send Sys-pri, count is %u", peer->syspri_out);
    
    return rc;
}

static int32
_mpe_msg_peer_fdb(mlag_buf_t *buf, tbl_mlag_peer_t *peer, uint8 op, vid_t *vlan, uint16 vlan_num)
{
    tbl_sys_global_t *p_sys_glb = tbl_sys_global_get_sys_global();
//    tbl_mlag_t *mlag = tbl_mlag_get_mlag();
    uint32 i = 0;
    int32 **size = &buf->size;
    uint8 **ppnt = &buf->pnt;
    
    MSG_ENCODE_PUTC(op);
    MSG_ENCODE_PUT(p_sys_glb->route_mac, MAC_ADDR_LEN);
    MSG_ENCODE_PUTW(vlan_num);
    for (i = 0; i < vlan_num; i++)
    {
        MSG_ENCODE_PUTW(vlan[i]);
    }

    return PM_E_NONE;
}

int32
mlag_send_peer_fdb(tbl_mlag_peer_t *peer, uint8 op, vid_t *vlan, uint16 vlan_num)
{
    mlag_buf_t buf;
    int32 rc = PM_E_NONE;
    
    _mpe_buf_init(&buf, MLAG_HEADER_SIZE + 1 + MAC_ADDR_LEN + 2 + vlan_num * sizeof(uint16));

    /* Encode the Message Header */
    _mpe_msg_hdr(&buf, peer, MLAG_MSG_PEER_FDB);

    /* Encode syspri Message */
    _mpe_msg_peer_fdb(&buf, peer, op, vlan, vlan_num);
    
    PM_E_RETURN(_mlag_send(peer, &buf));

    /* Count this Message */
    peer->peer_fdb_out++;

    MLAG_DEBUG(MLAG_PACKET, "Send Peer-FDB, count is %u", peer->peer_fdb_out);
    
    return PM_E_NONE;
}
