
#include "proto.h"
#include "switch.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_vlan_define.h"
#include "gen/tbl_vlan.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_fdb.h"
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
#include "lacp_define.h"
#include "lacp_pdu.h"
#include "lacp.h"
#include "mlag.h"
#include "mlag_api.h"
#include "mlag_fsm.h"
#include "mlag_sock.h"
#include "fei_brg.h"
#include "genlog.h"
#include "lib_syslimit.h"

static int32
_mpd_check_need_add_peer_fdb(uint8 *mac, vid_t vid, tbl_interface_t *peer_link_if)
{
    tbl_vlan_key_t vlan_key;
    tbl_vlan_t *p_db_vlan = NULL;
    tbl_fdb_key_t fdb_key;
    tbl_fdb_t *p_db_fdb = NULL;

    vlan_key.vid = vid;
    p_db_vlan = tbl_vlan_get_vlan(&vlan_key);
    if (NULL == p_db_vlan)
    {
        return FALSE;
    }

    if (!GLB_BMP_ISSET(p_db_vlan->member_port, peer_link_if->ifindex))
    {
        return FALSE;
    }

    fdb_key.fid = vid;
    sal_memcpy(fdb_key.mac, mac, MAC_ADDR_LEN);
    p_db_fdb = tbl_fdb_get_fdb(&fdb_key);
    if (p_db_fdb)
    {
        return FALSE;
    }
    
    return TRUE;
}

int32
mpd_msg_open(tbl_mlag_peer_t *peer, mlag_decode_t *p_msg)
{
    tbl_mlag_t *mlag = tbl_mlag_get_mlag();
    uint16 holdtime = 0;
    uint16 conf_keepalive = 0;
    uint16 conf_holdtime = 0;
    int32 **size = &p_msg->p_size_left;
    uint8 **ppnt = &p_msg->pnt;
    int32 rc = PM_E_NONE;

    peer->open_in++;

    MLAG_DEBUG(MLAG_PACKET, "Receive Open, count is %u", peer->open_in);
    
    MSG_DECODE_GET(mlag->remote_sysid, MAC_ADDR_LEN);
    MSG_DECODE_GETW(holdtime);
    tbl_mlag_set_mlag_field(mlag, TBL_MLAG_FLD_REMOTE_SYSID);

    conf_holdtime = peer->holdtime;
    conf_keepalive = peer->keepalive;
  
    if (holdtime < conf_holdtime)
    {
        peer->v_holdtime = holdtime;
    }
    else
    {
        peer->v_holdtime = conf_holdtime;
    }

    /*
    * When localy configured keepalive is smaller than holdtime / 4,
    * use localy configured value as keepalive.  Otherwise use
    * calculated value
    */
    if (conf_keepalive <= (peer->v_holdtime/4))
    {
        peer->v_keepalive = conf_keepalive;
    }
    else
    {
        peer->v_keepalive = (peer->v_holdtime/4);
    }

    mlag_api_role_select();
  
    rc = mpf_process_event(MPF_EVENT_OPEN_VALID);
    return rc;
}

int32
mpd_msg_keepalive(tbl_mlag_peer_t *peer, mlag_decode_t *p_msg)
{
    int32 rc = PM_E_NONE;

    peer->keepalive_in++;
    /* Generate MLAG Peer FSM Valid KEEP-ALIVE Event */

    MLAG_DEBUG(MLAG_PACKET, "Receive Keepalive, count is %u", peer->keepalive_in);
    
    rc = mpf_process_event(MPF_EVENT_KEEPALIVE_VALID);
    return rc;
}

int32
mpd_msg_fdbsync(tbl_mlag_peer_t *peer, mlag_decode_t *p_msg)
{
    uint16 mlag_id = 0;
    uint8 fdb_num = 0;
    uint8 fdb_type = 0;
    uint8 fdb_act = 0;
    uint32 i = 0;
    int32 **size = &p_msg->p_size_left;
    uint8 **ppnt = &p_msg->pnt;
    glb_mlag_fdb_sync_t sync;
    int32 rc = PM_E_NONE;
    
    peer->fdbsync_in++;
    
    MSG_DECODE_GETC(fdb_num);

    MLAG_DEBUG(MLAG_PACKET, "Receive FDB-Sync, count is %u", fdb_num);
    
    sync.count = fdb_num;
    for(i = 0; i < fdb_num; i++)
    {
        /* Decode 'Vlan ID' value */
        MSG_DECODE_GETW(sync.array[i].fid);
        MSG_DECODE_GET(sync.array[i].mac, MAC_ADDR_LEN);
        MSG_DECODE_GETW(mlag_id);
        MSG_DECODE_GETC(fdb_type);
        MSG_DECODE_GETC(fdb_act);
        
        sync.array[i].mlag_id = mlag_id;
        sync.array[i].act = fdb_act;
        sync.array[i].flag = 0;
        GLB_SET_FLAG(sync.array[i].flag, GLB_FDB_FLAG_MLAG_SYNC_IN);
        if (MLAG_MSG_FDBSYNC_TYPE_STATIC == fdb_type)
        {
            GLB_SET_FLAG(sync.array[i].flag, GLB_FDB_FLAG_STATIC);
        }

        switch (sync.array[i].act)
        {
        case GLB_MLAG_FDB_SYNC_ADD:
        case GLB_MLAG_FDB_SYNC_DEL:
            MLAG_DEBUG(MLAG_PACKET, "Decode FDB-sync, action: %s, vid: %u, mac: %02x%02x.%02x%02x.%02x%02x, type: %s",
                MLAG_SYNC_FDB_ACT_STR(sync.array[i].act),
                sync.array[i].fid, 
                sync.array[i].mac[0], sync.array[i].mac[1], sync.array[i].mac[2],
                sync.array[i].mac[3], sync.array[i].mac[4], sync.array[i].mac[5],
                GLB_FLAG_ISSET(sync.array[i].flag, GLB_FDB_FLAG_STATIC) ? "static" : "dynamic");
            break;

        case GLB_MLAG_FDB_SYNC_FLUSH_ALL:
            MLAG_DEBUG(MLAG_PACKET, "Decode FDB-sync, action: %s",
                MLAG_SYNC_FDB_ACT_STR(sync.array[i].act));
            break;

        case GLB_MLAG_FDB_SYNC_FLUSH_PORT:
            MLAG_DEBUG(MLAG_PACKET, "Decode FDB-sync, action: %s, mlag_id: %u",
                MLAG_SYNC_FDB_ACT_STR(sync.array[i].act), sync.array[i].mlag_id);
            break;
            
        case GLB_MLAG_FDB_SYNC_FLUSH_VLAN:
            MLAG_DEBUG(MLAG_PACKET, "Decode FDB-sync, action: %s, vid: %u",
                MLAG_SYNC_FDB_ACT_STR(sync.array[i].act), sync.array[i].fid);
            break;
            
        case GLB_MLAG_FDB_SYNC_FLUSH_MAC:
            MLAG_DEBUG(MLAG_PACKET, "Decode FDB-sync, action: %s, mac: %02x%02x.%02x%02x.%02x%02x",
                MLAG_SYNC_FDB_ACT_STR(sync.array[i].act), 
                sync.array[i].mac[0], sync.array[i].mac[1], sync.array[i].mac[2],
                sync.array[i].mac[3], sync.array[i].mac[4], sync.array[i].mac[5]);
            break;

        default:
            break;
        }
    }

    rc = fei_mlag_sync_fdb(&sync);

    return rc;
}

int32
mpd_msg_failover(tbl_mlag_peer_t *peer, mlag_decode_t *p_msg)
{
    uint32 mlagid = 0;
    uint8 is_recover = 0;
    uint8 reserved = 0;
    int32 **size = &p_msg->p_size_left;
    uint8 **ppnt = &p_msg->pnt;
    int32 rc = PM_E_NONE;
    
    peer->failover_in++;

    MSG_DECODE_GETW(mlagid);
    MSG_DECODE_GETC(is_recover);
    MSG_DECODE_GETC(reserved);
    (void)reserved;
    
    MLAG_DEBUG(MLAG_PACKET, "Receive Failover, mlagid: %u, is_recover: %u, count is %u", mlagid, is_recover, peer->failover_in);
    
    mlag_api_peer_failover(mlagid, is_recover);
    
    return rc;
}

int32
mpd_msg_conf(tbl_mlag_peer_t *peer, mlag_decode_t *p_msg)
{
    uint32 mlagid = 0;
    uint8 is_conf = 0;
    uint8 is_up = 0;
    int32 **size = &p_msg->p_size_left;
    uint8 **ppnt = &p_msg->pnt;
    int32 rc = PM_E_NONE;

    peer->conf_in++;

    MSG_DECODE_GETW(mlagid);
    MSG_DECODE_GETC(is_conf);
    MSG_DECODE_GETC(is_up);

    MLAG_DEBUG(MLAG_PACKET, "Receive Config, mlagid: %u, is_conf: %u, is_up: %u, count is %u", mlagid, is_conf, is_up, peer->conf_in);
    
    if (is_conf)
    {
        rc = mlag_api_port_peer_conf(mlagid, is_up);
    }
    else
    {
        rc = mlag_api_port_peer_unconf(mlagid);
    }

    return rc;
}

int32
mpd_msg_stp(tbl_mlag_peer_t *peer, mlag_decode_t *p_msg)
{
    uint8 addr[MAC_ADDR_LEN];
    uint8 sub_type = 0;
    uint8 stp_type = 0;
    uint8 enable = 0;
    uint16 prio = 0;
    uint16 mlagid = 0;
    uint16 instance = 0;
    uint32 state = 0;
    uint32 length = 0;
    uint8 pkt_buff[MLAG_MSG_STP_PKT_BUFF_SIZE];
    l2_socket_addr_t l2_skaddr;
    int32 **size = &p_msg->p_size_left;
    uint8 **ppnt = &p_msg->pnt;
    int32 rc = PM_E_NONE;

    peer->stp_in++;
    
    MLAG_DEBUG(MLAG_PACKET, "Receive STP, count is %u", peer->stp_in);
    
    MSG_DECODE_GETC(sub_type);

    switch(sub_type)
    {
    case MLAG_MSG_STP_GLOBAL:
        peer->stp_global_in++;
        MSG_DECODE_GETC(enable);
        MSG_DECODE_GETW(prio);
        MSG_DECODE_GET(addr, MAC_ADDR_LEN);
        MSG_DECODE_GETC(stp_type);
#if 0
        MLAG_DEBUG(MLAG_PACKET, "%s-%s [DECODE] Stp global: enable %u, type %u, prio 0x%02x%02x, addr %02x%02x.%02x%02x.%02x%02x",
            peer ? peer->host : (u_int8_t *) "?",
            peer ? MLAG_PEER_DIR_STR (peer) : "?",
            enable, stp_type, prio[0], prio[1],
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

        /*send to mstpd*/
        sal_memset(&msg_global, 0, sizeof(nsm_msg_mlag_stp_global_t));
        nsm_mlag_send_global_info_msg(&msg_global, enable, stp_type, prio, addr);
        ret = SSOCK_ERR_NONE;
#endif   
        break;

    case MLAG_MSG_STP_PORT:
        peer->stp_port_in++;
        MSG_DECODE_GETC(enable);
        MSG_DECODE_GETW(mlagid);
#if 0
        group_list = mlag_group_list_lookup_default();
        if(!group_list)
        {
            MLAG_LOG_ERR("group_list not found!");
            return -1;
        }
        group = mlag_group_lookup(group_list, mlagid);
        if(!group)
        {
            MLAG_LOG_ERR("group %d not found!", mlagid);
            return -1;
        }

        if(NULL == group->ifp)
        {
            return -1;
        }

        sal_memset(&msg_port, 0, sizeof(nsm_msg_mlag_stp_port_t));

        group->stp_port_enable = enable;
        nsm_mlag_send_port_info_msg(&msg_port, mlagid, group->ifp->ifindex,
            group->peer_conf, group->peer_if_up, enable);
#endif
        break;

    case MLAG_MSG_STP_INST:
        peer->stp_instance_in++;
        MSG_DECODE_GETW(instance);
#if 0
        nsm_mlag_recv_add_instance(instance);            
        MLAG_DEBUG(MLAG_PACKET, "%s-%s [DECODE] Stp instance: instance %d",
            peer ? peer->host : (u_int8_t *) "?",
            peer ? MLAG_PEER_DIR_STR (peer) : "?",
            instance);
#endif
        break;

    case MLAG_MSG_STP_PACKET:
        peer->stp_packet_in++;
        MSG_DECODE_GETW(mlagid);
        MSG_DECODE_GET(l2_skaddr.dest_mac, MAC_ADDR_LEN);
        MSG_DECODE_GET(l2_skaddr.src_mac, MAC_ADDR_LEN);
        MSG_DECODE_GETL(l2_skaddr.port);
        MSG_DECODE_GETW(l2_skaddr.vlanid);
        MSG_DECODE_GETW(l2_skaddr.svlanid);
        MSG_DECODE_GETW(l2_skaddr.type);
        MSG_DECODE_GETW(l2_skaddr.reserve);

        MSG_DECODE_GETL(length);
        MSG_DECODE_GET(pkt_buff, length);
#if 0
        if (MLAG_DEBUG (events, EVENTS))
        {
            char tmp_buff[1024] = "";
            char * p = tmp_buff;
            
            for(i = 0; i < lenth; i++)
            {
                sal_sprintf(p, "%02X", pkt_buff[i]);
                p += 2;
        
                if(0 != i && 0 == (i+1)%4)
                {
                    sal_sprintf(p, " ");
                    p++;
                }
            }
            MLAG_DEBUG(MLAG_PACKET, "%s-%s [DECODE] Stp bpdu packet: mlag id %d, packet lenth %d",
                   peer->host, MLAG_PEER_DIR_STR (peer), 
                   mlagid, lenth);
            MLAG_DEBUG(MLAG_PACKET, "%s-%s [DECODE] Stp bpdu packet:"
                    "src addr (%.02x%.02x.%.02x%.02x.%.02x%.02x) "
                    "dst addr (%.02x%.02x.%.02x%.02x.%.02x%.02x) ",
                    peer->host, MLAG_PEER_DIR_STR (peer), 
                    l2_skaddr.src_mac[0],
                    l2_skaddr.src_mac[1],
                    l2_skaddr.src_mac[2],
                    l2_skaddr.src_mac[3],
                    l2_skaddr.src_mac[4],
                    l2_skaddr.src_mac[5],
                    l2_skaddr.dest_mac[0],
                    l2_skaddr.dest_mac[1],
                    l2_skaddr.dest_mac[2],
                    l2_skaddr.dest_mac[3],
                    l2_skaddr.dest_mac[4],
                    l2_skaddr.dest_mac[5]);
            MLAG_DEBUG(MLAG_PACKET, "%s-%s [DECODE] Stp bpdu packet: payload %s",
                    peer->host, MLAG_PEER_DIR_STR (peer), 
                    tmp_buff);
        }
        
        nsm_mlag_send_bpdu_packet_msg(&msg_pkt, mlagid, l2_skaddr, lenth, pkt_buff);
#endif
        break;

    case MLAG_MSG_STP_STATE:
        peer->stp_state_in++;
        MSG_DECODE_GETW(mlagid);
        MSG_DECODE_GETW(instance);
        MSG_DECODE_GETC(state);
#if 0
        nsm_mlag_bridge_set_port_state(mlagid, state, instance);
        MLAG_DEBUG(MLAG_PACKET, "%s-%s [DECODE] Stp State: mlag %d, instance %d, state %d",
            peer ? peer->host : (u_int8_t *) "?",
            peer ? MLAG_PEER_DIR_STR (peer) : "?",
            mlagid, instance, state);
#endif
        break;

    default:
        break;
    }
    
    (void)stp_type;
    (void)enable;
    (void)prio;
    (void)mlagid;
    (void)instance;
    (void)state;
    
    return rc;
}

int32
mpd_msg_syspri(tbl_mlag_peer_t *peer, mlag_decode_t *p_msg)
{
    tbl_mlag_t *mlag = tbl_mlag_get_mlag();
    uint16 syspri = 0;
    int32 **size = &p_msg->p_size_left;
    uint8 **ppnt = &p_msg->pnt;
    int32 rc = PM_E_NONE;

    peer->syspri_in++;

    MSG_DECODE_GETW(syspri);
    mlag->remote_syspri = syspri;
    mlag_api_syspri_select(mlag);

    MLAG_DEBUG(MLAG_PACKET, "Receive Syspri, syspri: %u, count is %u", syspri, peer->syspri_in);
    
    return rc;
}

int32
mpd_msg_peer_fdb(tbl_mlag_peer_t *peer, mlag_decode_t *p_msg)
{
    tbl_mlag_t tmp_mlag;
    tbl_mlag_t *mlag = tbl_mlag_get_mlag();
    mac_addr_t mac;
    uint32 mac_change = FALSE;
    uint32 peer_vlanif_bmp_change = FALSE;
    uint32 peer_fdb_bmp_change = FALSE;
    uint32 i = 0;
    uint16 vlan_num = 0;
    vid_t vid = 0;
    uint8  op = 0;
    int32 **size = &p_msg->p_size_left;
    uint8 **ppnt = &p_msg->pnt;
    int32 rc = PM_E_NONE;

    peer->peer_fdb_in++;

    MLAG_DEBUG(MLAG_PACKET, "Receive Peer FDB, count is %u", peer->peer_fdb_in);
    
    if (NULL == mlag->peer_link_if)
    {
        return PM_E_NONE;
    }
    
    MSG_DECODE_GETC(op);
    MSG_DECODE_GET(mac, MAC_ADDR_LEN);
    MSG_DECODE_GETW(vlan_num);

    if (sal_memcmp(mac, mlag->peer_routemac, MAC_ADDR_LEN))
    {
        mlag_api_peer_fdb_flush();
        mac_change = TRUE;
        sal_memcpy(mlag->peer_routemac, mac, MAC_ADDR_LEN);
        tbl_mlag_set_mlag_field(mlag, TBL_MLAG_FLD_PEER_ROUTEMAC);
    }
    (void)mac_change;
    
    switch (op)
    {
    case MLAG_MSG_PEER_FDB_OP_ADD:
        sal_memcpy(tmp_mlag.peer_vlanif_bmp, mlag->peer_vlanif_bmp, sizeof(vlan_bmp_t));
        sal_memcpy(tmp_mlag.peer_fdb_bmp, mlag->peer_fdb_bmp, sizeof(vlan_bmp_t));
        
        for (i = 0; i < vlan_num; i++)
        {
            MSG_DECODE_GETW(vid);  
            if (vid && vid < GLB_MAX_VLAN_ID)
            {
                GLB_BMP_SET(tmp_mlag.peer_vlanif_bmp, vid);
                peer_vlanif_bmp_change = TRUE;
                if (_mpd_check_need_add_peer_fdb(mlag->peer_routemac, vid, mlag->peer_link_if))
                {
                    peer_fdb_bmp_change = TRUE;
                    GLB_BMP_SET(tmp_mlag.peer_fdb_bmp, vid);
                }
            }
        }
        if (peer_vlanif_bmp_change)
        {
            tbl_mlag_set_mlag_field(&tmp_mlag, TBL_MLAG_FLD_PEER_VLANIF_BMP);
        }
        if (peer_fdb_bmp_change)
        {
            tbl_mlag_set_mlag_field(&tmp_mlag, TBL_MLAG_FLD_PEER_FDB_BMP);
        }
        break;

    case MLAG_MSG_PEER_FDB_OP_DEL:
        sal_memcpy(tmp_mlag.peer_vlanif_bmp, mlag->peer_vlanif_bmp, sizeof(vlan_bmp_t));
        sal_memcpy(tmp_mlag.peer_fdb_bmp, mlag->peer_fdb_bmp, sizeof(vlan_bmp_t));
        
        for (i = 0; i < vlan_num; i++)
        {
            MSG_DECODE_GETW(vid);  
            if (vid && vid < GLB_MAX_VLAN_ID)
            {
                peer_vlanif_bmp_change = TRUE;
                GLB_BMP_UNSET(tmp_mlag.peer_vlanif_bmp, vid);
                if (_mpd_check_need_add_peer_fdb(mlag->peer_routemac, vid, mlag->peer_link_if))
                {
                    peer_fdb_bmp_change = TRUE;
                    GLB_BMP_UNSET(tmp_mlag.peer_fdb_bmp, vid);
                }
            }
        }
        if (peer_vlanif_bmp_change)
        {
            tbl_mlag_set_mlag_field(&tmp_mlag, TBL_MLAG_FLD_PEER_VLANIF_BMP);
        }
        if (peer_fdb_bmp_change)
        {
            tbl_mlag_set_mlag_field(&tmp_mlag, TBL_MLAG_FLD_PEER_FDB_BMP);
        }
        break;

    case MLAG_MSG_PEER_FDB_OP_SYNC:
        sal_memcpy(tmp_mlag.peer_vlanif_bmp, mlag->peer_vlanif_bmp, sizeof(vlan_bmp_t));
        sal_memcpy(tmp_mlag.peer_fdb_bmp, mlag->peer_fdb_bmp, sizeof(vlan_bmp_t));
        
        for (i = 0; i < vlan_num; i++)
        {
            MSG_DECODE_GETW(vid);  
            if (vid && vid < GLB_MAX_VLAN_ID)
            {
                peer_vlanif_bmp_change = TRUE;
                GLB_BMP_SET(tmp_mlag.peer_vlanif_bmp, vid);
                if (_mpd_check_need_add_peer_fdb(mlag->peer_routemac, vid, mlag->peer_link_if))
                {
                    peer_fdb_bmp_change = TRUE;
                    GLB_BMP_SET(tmp_mlag.peer_fdb_bmp, vid);
                }
            }
        }
        if (peer_vlanif_bmp_change)
        {
            tbl_mlag_set_mlag_field(&tmp_mlag, TBL_MLAG_FLD_PEER_VLANIF_BMP);
        }
        if (peer_fdb_bmp_change)
        {
            tbl_mlag_set_mlag_field(&tmp_mlag, TBL_MLAG_FLD_PEER_FDB_BMP);
        }
        break;

    default:
        break;
    }
    return rc;
}

int32
mpd_msg_disconnect(tbl_mlag_peer_t *peer, mlag_decode_t *p_msg)
{
    int32 rc = PM_E_NONE;

    peer->disconnect_in++;

    MLAG_DEBUG(MLAG_PACKET, "Receive Disconnect, count is %u", peer->disconnect_in);

    mpf_process_event(MPF_EVENT_RX_DISCONNECT);
    
    return rc;
}

int32
mpd_msg_handle(ctc_sock_session_t* p_session, mlag_decode_t *p_msg)
{
    tbl_mlag_peer_t *peer = tbl_mlag_peer_get_mlag_peer();
    ctc_sock_t *p_sock = NULL;
    uint16 msg_size_min = 0;
    uint16 msg_size_max = 0;
    char buf[256];
    int32 rc = PM_E_NONE;
    uint32 is_server = FALSE;

    peer->lastread = ctc_time_boottime_sec(NULL);
    p_sock = (ctc_sock_t*)p_session->p_sock;
    if (peer->server_sock == p_sock->sock)
    {
        is_server = TRUE;
    }
    else if (peer->client_sock == p_sock->sock)
    {
        is_server = FALSE;
    }
    else
    {
        rc = PM_E_FAIL;
        goto EXIT;
    }

    /*check the system memory state when recieve MLAG packet*/
    if (syslimit_memory_threshold_check(SYSLIMIT_MEMCHECK_STATE_WARNING) != 0)
    {
        rc = PM_E_FAIL;
        goto EXIT;
    }
    
    sal_memset(buf, 0, sizeof(buf));
    sal_snprintf(buf, 256, "mpd_msg_handle is_server %u, ver %u, type %s(%u), length %u",
        is_server, p_msg->version, mlag_msg_type_str(p_msg->type), p_msg->type, p_msg->length);
    MLAG_DEBUG(MLAG_PACKET, "%s", buf);

    /* Validata MLAG Message Type */
    switch (p_msg->type)
    {
    case MLAG_MSG_OPEN:
        msg_size_min = MLAG_MSG_OPEN_MIN_SIZE;
        msg_size_max = MLAG_MSG_OPEN_MAX_SIZE;
        break;

    case MLAG_MSG_KEEPALIVE:
        msg_size_min = MLAG_MSG_KEEPALIVE_MIN_SIZE;
        msg_size_max = MLAG_MSG_KEEPALIVE_MAX_SIZE;
        break;

    case MLAG_MSG_FDBSYNC:
        msg_size_min = MLAG_MSG_FDBSYNC_MIN_SIZE;
        msg_size_max = MLAG_MAX_PACKET_SIZE;
        break;

    case MLAG_MSG_FAILOVER:
        msg_size_min = MLAG_MSG_FAILOVER_MIN_SIZE;
        msg_size_max = MLAG_MSG_FAILOVER_MAX_SIZE;
        break;

    case MLAG_MSG_CONF:
        msg_size_min = MLAG_MSG_CONF_MIN_SIZE;
        msg_size_max = MLAG_MSG_CONF_MAX_SIZE;
        break;

    /*add by huangxt for mlag-stp. 2014.09.29*/
    case MLAG_MSG_STP:
        msg_size_min = MLAG_MSG_STP_MIN_SIZE;
        msg_size_max = MLAG_MSG_STP_MAX_SIZE;
        break;

    case MLAG_MSG_SYSPRI:
        msg_size_min = MLAG_MSG_SYSPRI_MIN_SIZE;
        msg_size_max = MLAG_MSG_SYSPRI_MAX_SIZE;
        break;

    case MLAG_MSG_PEER_FDB:
        msg_size_min = MLAG_MSG_PEER_FDB_MIN_SIZE;
        msg_size_max = MLAG_MSG_PEER_FDB_MAX_SIZE;
        break;

    case MLAG_MSG_DISCONNECT:
        msg_size_min = MLAG_MSG_DISCONNECT_MIN_SIZE;
        msg_size_max = MLAG_MSG_DISCONNECT_MAX_SIZE;
        break;

    default:
        log_sys(M_MOD_INTERNAL, E_ERROR, "Receive invalid msg type %u", p_msg->type);
        goto EXIT;
    }
    
    /* Validata MLAG Message Length */
    if (p_msg->length < msg_size_min || p_msg->length > msg_size_max)
    {
        log_sys(M_MOD_INTERNAL, E_ERROR, "Receive invalid msg header length %u for type %u", p_msg->length, p_msg->type);
        goto EXIT;
    }

    switch (p_msg->type)
    {
    case MLAG_MSG_OPEN:
        rc = mpd_msg_open(peer, p_msg);
        break;

    case MLAG_MSG_KEEPALIVE:
        rc = mpd_msg_keepalive(peer, p_msg);
        break;

    case MLAG_MSG_FDBSYNC:
        rc = mpd_msg_fdbsync(peer, p_msg);
        break;

    case MLAG_MSG_FAILOVER:
        rc = mpd_msg_failover(peer, p_msg);
        break;

    case MLAG_MSG_CONF:
        rc = mpd_msg_conf(peer, p_msg);
        break;

    case MLAG_MSG_STP:
        rc = mpd_msg_stp(peer, p_msg);
        break;

    case MLAG_MSG_SYSPRI:
        rc = mpd_msg_syspri(peer, p_msg);
        break;

    case MLAG_MSG_PEER_FDB:
        rc = mpd_msg_peer_fdb(peer, p_msg);
        break;

    case MLAG_MSG_DISCONNECT:
        rc = mpd_msg_disconnect(peer, p_msg);
        break;

    default:
        log_sys(M_MOD_INTERNAL, E_ERROR, "Receive invalid msg type %u", p_msg->type);
        goto EXIT;
    }
    
EXIT:
    
    return rc;
}
