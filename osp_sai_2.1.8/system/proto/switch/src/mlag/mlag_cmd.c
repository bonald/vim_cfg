
#include "proto.h"
#include "switch.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_mlag_define.h"
#include "gen/tbl_mlag.h"
#include "gen/tbl_mlag_peer_define.h"
#include "gen/tbl_mlag_peer.h"
#include "gen/tbl_mlag_port_define.h"
#include "gen/tbl_mlag_port.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_fdb_define.h"
#include "gen/tbl_fdb.h"
#include "gen/tbl_mcfdb_define.h"
#include "gen/tbl_mcfdb.h"
#include "gen/tbl_macfilter_define.h"
#include "gen/tbl_macfilter.h"
#include "gen/tbl_l2_action_define.h"
#include "gen/tbl_l2_action.h"
#include "gen/tbl_psfdb_define.h"
#include "gen/tbl_psfdb.h"
#include "mlag.h"
#include "mlag_fsm.h"
#include "mlag_api.h"
#include "brg_fdb.h"

static int32
_mlag_cmd_get_mlag(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_mlag_t *p_mlag = tbl_mlag_get_mlag();
    FILE *fp = NULL;
    tbl_iter_args_t args;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_mlag_dump_one(p_mlag, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_mlag_cmd_get_mlag_peer(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_mlag_peer_t *p_peer = tbl_mlag_peer_get_mlag_peer();
    FILE *fp = NULL;
    tbl_iter_args_t args;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_mlag_peer_dump_one(p_peer, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_mlag_cmd_get_mlag_port(cfg_cmd_para_t *para, cdb_node_t *p_node, tbl_mlag_port_t *p_db_port, field_parse_t *p_field)
{
    tbl_mlag_port_master_t *p_master = tbl_mlag_port_get_master();
    ctclib_slistnode_t *listnode = NULL;
    tbl_mlag_port_t *p_iter_port = NULL;
    FILE *fp = NULL;
    tbl_iter_args_t args;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;

    if (p_db_port)
    {
        tbl_mlag_port_dump_one(p_db_port, &args);
    }
    else
    {
        CTCLIB_SLIST_LOOP(p_master->port_list, p_iter_port, listnode)
        {
            tbl_mlag_port_dump_one(p_iter_port, &args);
        }
    }
    
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_mlag_cmd_set_enable(tbl_mlag_t *p_mlag, tbl_mlag_t *p_db_mlag, cfg_cmd_para_t *para)
{
    /* 1. config check */
    
    /* 2. check duplicate config */
    if (p_mlag->enable == p_db_mlag->enable)
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    if (p_mlag->enable)
    {
        return mlag_api_create(p_mlag);
    }
    else
    {
        return mlag_api_destory();
    }
}

static int32
_mlag_cmd_set_peer_addr(tbl_mlag_peer_t *p_peer, tbl_mlag_peer_t *p_db_peer, cfg_cmd_para_t *para)
{
    uint32 ntoh_s_addr = 0;
    uint32 old_peer_valid = FALSE;
    uint32 new_peer_valid = FALSE;
    
    /* 1. config check */
    ntoh_s_addr = sal_ntoh32(p_peer->peer_addr.u.prefix4.s_addr);
    if (IN_LOOPBACK(ntoh_s_addr))
    {
        CFG_CONFLICT_RET("Cannot uses loopback address");
    }

    /* 2. check duplicate config */
    if (0 == sal_memcmp(&p_peer->peer_addr, &p_db_peer->peer_addr, sizeof(p_peer->peer_addr)))
    {
        return PM_E_NONE;
    }

    old_peer_valid = mlag_is_peer_addr_valid(p_db_peer);
    new_peer_valid = mlag_is_peer_addr_valid(p_peer);

    /* 3. call API */
    if (new_peer_valid)
    {
        if (old_peer_valid)
        {
            mlag_api_destory_peer();
        }
        return mlag_api_create_peer(p_peer);
    }
    else
    {
        return mlag_api_destory_peer();
    }
}

static int32
_mlag_cmd_set_peer_timers(tbl_mlag_peer_t *p_peer, tbl_mlag_peer_t *p_db_peer, cfg_cmd_para_t *para)
{
    /* 1. config check */

    if (!p_peer->set_timers)
    {
        p_peer->holdtime = MLAG_DEFAULT_HOLDTIME;
        p_peer->keepalive = MLAG_DEFAULT_KEEPALIVE;
    }
    else
    {
        if (p_peer->holdtime < p_peer->keepalive * 4)
        {
            CFG_CONFLICT_RET("Hold time should be no less than 4 times of the keepalive time");
        }
    }
    
    /* 2. check duplicate config */
    if (p_peer->set_timers == p_db_peer->set_timers
     && p_peer->holdtime == p_db_peer->holdtime
     && p_peer->keepalive == p_db_peer->keepalive)
    {
        return PM_E_NONE;
    }

    /* 3. call API */
    return mlag_api_set_peer_timers(p_peer);
}

static int32
_mlag_cmd_set_reload_delay_interval(tbl_mlag_t *p_mlag, tbl_mlag_t *p_db_mlag, cfg_cmd_para_t *para)
{
    /* 1. config check */
    
    /* 2. check duplicate config */
    if ((p_mlag->reload_delay_interval == p_db_mlag->reload_delay_interval)
     && (p_mlag->reload_delay_auto == p_db_mlag->reload_delay_auto))
    {
        return PM_E_NONE;
    }
    
    /* 3. call API */
    mlag_api_set_reload_delay_interval(p_mlag);

    return PM_E_NONE;
}

static int32
_mlag_cmd_set_peer_link_if(char *ifname, tbl_mlag_t *p_db_mlag, cfg_cmd_para_t *para)
{
    tbl_mstp_port_key_t mstp_key;
    tbl_mstp_port_t *p_mstp_port = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if = NULL;
    ds_brgif_t *p_db_brgif = NULL;
    tbl_interface_t *p_old_if = NULL;
    uint32 new_peerlink_valid = FALSE;
    
    /* 1. config check */
    if (ifname)
    {
        sal_memset(&if_key, 0, sizeof(if_key));
        sal_strcpy(if_key.name, ifname);
        p_db_if = tbl_interface_get_interface(&if_key);
        if (NULL == p_db_if)
        {
            CFG_CONFLICT_RET("Interface %s not exist", ifname);
        }

        IF_MGR_CHECK_LAG_MEMBER(p_db_if);
            
        if (GLB_IF_MODE_L2 != p_db_if->mode)
        {
            CFG_CONFLICT_RET("Peer-link can only be configured on layer2 interface");
        }
        if (GLB_VLAN_PORT_TYPE_TRUNK != p_db_if->vlan_type)
        {
            CFG_CONFLICT_RET("Peer-link can only be configured on trunk port");
        }
        if (p_db_if->isolate_group_id)
        {
            CFG_CONFLICT_RET("Port isolate group configured on this interface, remove it first");
        }
        if (GLB_IF_PVLAN_TYPE_NONE != p_db_if->pvlan_type)
        {
            CFG_CONFLICT_RET("Private vlan configured on this interface, remove it first");
        }
        if (p_db_if->mlag_id)
        {
            CFG_CONFLICT_RET("Interface has been configured MLAG %u, remove it first", p_db_if->mlag_id);
        }
        if ((GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG)))
        {
            CFG_CONFLICT_RET("Peer-link cannot be configured on mirror destination interface");
        }
        
        /* Added by kcao 2017-01-11 for bug 41372, check port-security and MLAG peer-link */
        p_db_brgif = p_db_if->brgif;
        if (p_db_brgif)
        {
            if (p_db_brgif->port_security_en)
            {
                CFG_CONFLICT_RET("Peer-link cannot be configured on security port");
            }
        }
  
        sal_memset(&mstp_key, 0, sizeof(mstp_key));
        sal_strcpy(mstp_key.name, ifname);
        p_mstp_port = tbl_mstp_port_get_mstp_port(&mstp_key);
        if (p_mstp_port && p_mstp_port->port_enable)
        {
            CFG_CONFLICT_RET("Peer-link can only be configured on spanning-tree disabled port");
        }

        if (p_db_mlag->peer_link_if)
        {
            /* 2. check duplicate config */
            if (p_db_mlag->peer_link_if == p_db_if)
            {
                return PM_E_NONE;
            }
            p_old_if = (tbl_interface_t*)p_db_mlag->peer_link_if;
            CFG_CONFLICT_RET("Peer-link has already been configured on %s, remove it first", 
                IFNAME_ETH2FULL(p_old_if->key.name, ifname_ext));
        }

        new_peerlink_valid = TRUE;
    }
    else
    {
        new_peerlink_valid = FALSE;
        p_db_if = NULL;
    }
    
    /* 3. call API */
    if (new_peerlink_valid)
    {
        mlag_api_create_peer_link(p_db_if);
    }
    else
    {
        mlag_api_destory_peer_link();
    }

    return PM_E_NONE;
}

int32
mlag_cmd_process_mlag(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    char *ifname = NULL;
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    cdb_node_t* p_node = NULL;
    tbl_mlag_t mlag;
    field_parse_t field;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    int32 value = 0;
#if 0    
    uint32 parse_field = TRUE;
#endif
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_MLAG);
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }

        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }
        
        switch (field_id)
        {
        case TBL_MLAG_FLD_ENABLE:
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
            PM_CFG_GET_BOOL(value, argv[i+1], para->p_rs);
            mlag.enable = value;
            rc = _mlag_cmd_set_enable(&mlag, p_db_mlag, para);
            break;

        case TBL_MLAG_FLD_RELOAD_DELAY_INTERVAL:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 2, para->p_rs);
            if (1 == argc)
            {
                mlag.reload_delay_auto = TRUE;
                value = MLAG_DEFAULT_RELOAD_DELAY;
            }
            else
            {
                mlag.reload_delay_auto = FALSE;
                PM_CFG_GET_UINT_RANGE(value, MLAG_MIN_RELOAD_DELAY, MLAG_MAX_RELOAD_DELAY, argv[i+1], para->p_rs);
            }
            mlag.reload_delay_interval = value;
            rc = _mlag_cmd_set_reload_delay_interval(&mlag, p_db_mlag, para);
            break;

        case TBL_MLAG_FLD_PEER_LINK_IFINDEX:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 2, para->p_rs);
            ifname = argv[i+1];
            rc = _mlag_cmd_set_peer_link_if(ifname, p_db_mlag, para);
            break;

        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }
        
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field));
        rc = _mlag_cmd_get_mlag(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
mlag_cmd_process_mlag_peer(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    tbl_mlag_peer_t *p_db_peer = tbl_mlag_peer_get_mlag_peer();
    cdb_node_t* p_node = NULL;
    tbl_mlag_peer_t peer;
    field_parse_t field;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    int32 value = 0;
#if 0    
    uint32 parse_field = TRUE;
#endif
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_MLAG_PEER);
    
    switch (para->oper)
    {
    case CDB_OPER_SET:
        PM_CFG_CHECK_MIN_PARAMS(argc, 1, para->p_rs);
        field_id = cdb_get_field_id(argv[i], p_node, &p_field);
        if (field_id < 0)
        {    
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[i]);
        }

        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }

        sal_memset(&peer, 0, sizeof(peer));
        switch (field_id)
        {
        case TBL_MLAG_PEER_FLD_PEER_ADDR:
            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 2, para->p_rs);
            if (2 == argc)
            {
                rc = cdb_addr_str2val(&peer.peer_addr, argv[i+1], IPADDR_BUF_SIZE);
                if (rc < 0)
                {
                    CFG_INVALID_PARAM_RET("Invalid peer address %s", argv[i+1]);
                }
            }
            _mlag_cmd_set_peer_addr(&peer, p_db_peer, para);
            break;

        case TBL_MLAG_PEER_FLD_SET_TIMERS:

            PM_CFG_CHECK_EXPECT_2PARAMS(argc, 1, 3, para->p_rs);
            if (3 == argc)
            {
                peer.set_timers = TRUE;
                PM_CFG_GET_UINT_RANGE(value, 1, 65535, argv[i+1], para->p_rs);
                peer.keepalive = value;
                PM_CFG_GET_UINT_RANGE(value, 4, 65535, argv[i+2], para->p_rs);
                peer.holdtime = value;
            }
            else
            {
                peer.set_timers = FALSE;
            }
            _mlag_cmd_set_peer_timers(&peer, p_db_peer, para);
            break;
    
        default:
            CFG_INVALID_PARAM_RET("Unknown support changed field %s", argv[i]);
            break;
        }        
        break;

    case CDB_OPER_GET:
        sal_memset(&field, FLD_ID_ALL, sizeof(field));
        rc = _mlag_cmd_get_mlag_peer(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

int32
_mlag_cmd_add_mlag_port(char *ifname, uint32 mlag_id, cfg_cmd_para_t *para)
{
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    tbl_mlag_port_key_t port_key;
    tbl_mlag_port_t *p_db_port = NULL;
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if = NULL;
    ds_brgif_t *p_db_brgif = NULL;
    int32 has_static_fdb = FALSE;
    int32 rc = PM_E_NONE;

    sal_memset(&if_key, 0, sizeof(if_key));
    sal_strcpy(if_key.name, ifname);

    /* 1. config check */    
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s not exist", ifname);
    }
    if (GLB_IF_TYPE_LINKAGG_IF != p_db_if->hw_type)
    {
        CFG_CONFLICT_RET("MLAG can only be applied on LAG interface");
    }
    if (GLB_IF_MODE_L3 == p_db_if->mode)
    {
        CFG_CONFLICT_RET("Mlag can only be applied on l2 LAG interface");
    }
    if (p_db_if->isolate_group_id)
    {
        CFG_CONFLICT_RET("Port isolate group configured on this interface, remove it first");
    }
    if (GLB_IF_PVLAN_TYPE_NONE != p_db_if->pvlan_type)
    {
        CFG_CONFLICT_RET("Private vlan configured on this interface, remove it first");
    }
    if (MLAG_ID_INVALID != p_db_if->mlag_id)
    {
        CFG_CONFLICT_RET("Interface has already bound with MLAG %u", p_db_if->mlag_id);
    }
    if (GLB_FLAG_ISSET(p_db_if->mirror_enable, MIRROR_DEST_FLAG)) 
    {
        CFG_CONFLICT_RET("Interface has configured monitor destination");
    }    
    if (p_db_mlag->peer_link_if == p_db_if)
    {
        CFG_CONFLICT_RET("Interface has used for peer-link");
    }
    has_static_fdb = brg_fdb_is_static_fdb_on_port(p_db_if->ifindex);
    if (has_static_fdb)
    {
        CFG_CONFLICT_RET("Interface has static FDB on it, clear static FDB first");
    }
    p_db_brgif = ds_brgif_get_brgif(p_db_if);
    if (NULL == p_db_brgif)
    {
        CFG_CONFLICT_RET("Interface is not switch port");
    }
    if (p_db_brgif->port_security_en)
    {
        CFG_CONFLICT_RET("Interface has been enabled port security");
    }
    if (!p_db_brgif->mac_learning_en)
    {
        CFG_CONFLICT_RET("Interface should enable mac learning at first");
    }
    if (p_db_if->static_security_num)
    {
        CFG_CONFLICT_RET("Interface has been configured security MAC");
    }
    
    port_key.id = mlag_id;
    p_db_port = tbl_mlag_port_get_mlag_port(&port_key);
    if (p_db_port && (GLB_INVALID_IFINDEX != p_db_port->ifindex))
    {
        CFG_CONFLICT_RET("MLAG id has already been used");
    }
    
    /* 2. check duplicate config */
    
    /* 3. call API */
    PM_E_RETURN(mlag_api_add_mlag_port(p_db_if, mlag_id));

    return PM_E_NONE;
}

int32
_mlag_cmd_del_mlag_port(char *ifname, cfg_cmd_para_t *para)
{
    tbl_interface_key_t if_key;
    tbl_interface_t *p_db_if = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&if_key, 0, sizeof(if_key));
    sal_strcpy(if_key.name, ifname);

    /* 1. config check */    
    p_db_if = tbl_interface_get_interface(&if_key);
    if (NULL == p_db_if)
    {
        CFG_CONFLICT_RET("Interface %s not exist", ifname);
    }
    if (MLAG_ID_INVALID == p_db_if->mlag_id)
    {
        CFG_CONFLICT_RET("Interface has not bound with MLAG");
    }
    
    /* 2. check duplicate config */
    
    
    /* 3. call API */
    PM_E_RETURN(mlag_api_del_mlag_port(p_db_if));

    return PM_E_NONE;
}

int32
mlag_cmd_process_mlag_port(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    tbl_mlag_port_key_t key;
    tbl_mlag_port_t *p_db_port = NULL;
    field_parse_t field;
    uint32 mlag_id = 0;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_MLAG_PORT);
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 2, para->p_rs);
        PM_CFG_GET_UINT(mlag_id, argv[0], para->p_rs);
        if (mlag_id > GLB_LAG_GROUP_NUM)
        {
            CFG_CONFLICT_RET("Max MLAG id should be less than %d", GLB_LAG_GROUP_NUM+1);
        }
        _mlag_cmd_add_mlag_port(argv[1], mlag_id, para);
        break;
        
    case CDB_OPER_DEL:
        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);
        _mlag_cmd_del_mlag_port(argv[0], para);
        break;

    case CDB_OPER_GET:
        PM_CFG_CHECK_EXPECT_2PARAMS(argc, 0, 1, para->p_rs);
        if (1 == argc)
        {
            PM_CFG_GET_UINT(mlag_id, argv[0], para->p_rs);
            if (mlag_id > GLB_LAG_GROUP_NUM)
            {
                CFG_CONFLICT_RET("Max MLAG id should be less than %d", GLB_LAG_GROUP_NUM+1);
            }
            key.id = mlag_id;
            p_db_port = tbl_mlag_port_get_mlag_port(&key);
            if (NULL == p_db_port)
            {
                CFG_CONFLICT_RET("MLAG group %u not exist", mlag_id);
            }
        }
        sal_memset(&field, FLD_ID_ALL, sizeof(field));
        rc = _mlag_cmd_get_mlag_port(para, p_node, p_db_port, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}

static int32
_mlag_cmd_show_mlag(cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    char buf[CMD_BUF_32];
    tbl_mlag_t *p_db_mlag = tbl_mlag_get_mlag();
    tbl_mlag_peer_t *p_db_peer = tbl_mlag_peer_get_mlag_peer();
    FILE *fp = NULL;
    uint32 remain_sec = 0;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (p_db_mlag->enable)
    {
        sal_memset(ifname_ext, 0, sizeof(ifname_ext));
        if (p_db_mlag->peer_link_if)
        {
            p_db_if = (tbl_interface_t*)p_db_mlag->peer_link_if;
            IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
        }
        else
        {
            sal_strcpy(ifname_ext, "-");
        }
        if (mlag_is_peer_addr_valid(p_db_peer))
        {
            cdb_addr_val2str(buf, CMD_BUF_64, &p_db_peer->peer_addr);
        }
        else
        {
            sal_strcpy(buf, "-");
        }
        
        sal_fprintf(fp, "MLAG Configuration:\n");
        sal_fprintf(fp, "%-22s: %s\n", "peer-link",     ifname_ext);
        sal_fprintf(fp, "%-22s: %s\n", "peer-address",  buf);
        if (p_db_mlag->reload_delay_auto)
        {
            sal_fprintf(fp, "%-22s: Auto\n", "reload-delay");
        }
        else
        {
            sal_fprintf(fp, "%-22s: %us\n", "reload-delay",  p_db_mlag->reload_delay_interval);
        }
        sal_fprintf(fp, "\n");
        sal_fprintf(fp, "MLAG Status:\n");
        sal_fprintf(fp, "%-22s: %s\n", "role",          (MLAG_ROLE_MASTER == p_db_mlag->role) ? "Master" : "Slave");
        sal_fprintf(fp, "%-22s: %s\n", "state",         MLAG_PEER_FSM_STATE_STR(p_db_peer->mpf_state));
        sal_fprintf(fp, "%-22s: %s\n", "local system-id",   cdb_mac_addr_val2str(buf, CMD_BUF_32, p_db_mlag->local_sysid));
        sal_fprintf(fp, "%-22s: %s\n", "remote system-id",  cdb_mac_addr_val2str(buf, CMD_BUF_32, p_db_mlag->remote_sysid));
        sal_fprintf(fp, "%-22s: %s\n", "mlag system-id",    cdb_mac_addr_val2str(buf, CMD_BUF_32, p_db_mlag->mlag_sysid));
        sal_fprintf(fp, "%-22s: %u\n", "local lacp-priority",  p_db_mlag->local_syspri);
        sal_fprintf(fp, "%-22s: %u\n", "remote lacp-priority", p_db_mlag->remote_syspri);
        sal_fprintf(fp, "%-22s: %u\n", "mlag lacp-priority",   p_db_mlag->mlag_syspri);
        sal_fprintf(fp, "%-22s: %s\n", "peer-conf",     mlag_is_peer_addr_valid(p_db_peer) ? "Yes" : "No");

        if (p_db_mlag->reload_delay_timer)
        {
            remain_sec = ctc_timer_get_remain_sec(p_db_mlag->reload_delay_timer);
            sal_fprintf(fp, "%-22s: %u\n", "reload-delay time", remain_sec);
        }        
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

#define ONE_WEEK_SECOND     (60*60*24*7)
#define ONE_DAY_SECOND      (60*60*24)

char*
mlag_time_t2wdhms_str(sal_time_t time, char *str, uint32 str_len)
{
    sal_time_t now;
    sal_time_t delta;
    struct sal_tm *tm = NULL;

    str[0] = CMD_ZERO_CHAR;

    /* If uptime is ZERO, just return blank field */
    if (time == 0)
    {
        sal_snprintf(str, str_len, "%s", "N/A");
        return str;
    }

    /* Get current time */
    now = ctc_time_boottime_sec(NULL);
  
    delta = now - time;

    tm = sal_localtime(&delta);

    /* Making formatted timer strings */
    if (delta < ONE_DAY_SECOND)
    {
        sal_snprintf(str, str_len, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
    }
    else if (delta < ONE_WEEK_SECOND)
    {
        sal_snprintf(str, str_len, "%02dd%dh%02dm", tm->tm_yday, tm->tm_hour, tm->tm_min);
    }
    else
    {
        sal_snprintf(str, str_len, "%02dw%dd%02dh", (tm->tm_yday/7), (tm->tm_yday%7), tm->tm_hour);
    }

    return str;
}


static int32
_mlag_cmd_show_mlag_peer(cfg_cmd_para_t *para)
{
    char uptime_str[CMD_BUF_32];
    char lastread_str[CMD_BUF_32];
    char buf[CMD_BUF_64];
    uint32 rx_total = 0;
    uint32 tx_total = 0;
    tbl_mlag_peer_t *p = tbl_mlag_peer_get_mlag_peer();
    FILE *fp = NULL;
    int32 remain_sec = 0;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    if (mlag_is_peer_addr_valid(p))
    {
        sal_fprintf(fp, "MLAG neighbor is %s, MLAG version 1\n", 
            cdb_addr_val2str(buf, CMD_BUF_64, &p->peer_addr));
        if (p->set_timers)
        {
            sal_fprintf(fp, "Configured hold time is %u, keepalive interval is %u seconds\n", 
                p->holdtime, p->keepalive);
        }
        sal_fprintf(fp, "Operation hold time is %u, keepalive interval is %u seconds\n", 
                p->v_holdtime, p->v_keepalive);

        mlag_time_t2wdhms_str(p->uptime, uptime_str, CMD_BUF_32);
        mlag_time_t2wdhms_str(p->lastread, lastread_str, CMD_BUF_32);
        sal_fprintf(fp, "Uptime %s, Last read time %s\n", uptime_str, lastread_str);

        if (MPF_STATE_ACTIVE == p->mpf_state)
        {
            if (p->t_auto_start)
            {
                remain_sec = ctc_timer_get_remain_sec(p->t_auto_start);
                if (remain_sec >= 0)
                {
                    sal_fprintf(fp, "Auto start timer due in %d seconds\n", remain_sec);
                }
            }
            else
            {
                remain_sec = ctc_sock_get_connect_remain_second(p->client_sock);
                if (remain_sec >= 0)
                {
                    sal_fprintf(fp, "Next connect timer due in %d seconds\n", remain_sec);
                }
            }
        }
        sal_fprintf(fp, "%s = %s\n\n", "MLAG state", MLAG_PEER_FSM_STATE_STR(p->mpf_state));

        /* Packet counts. */
        rx_total = p->open_in + p->keepalive_in + p->fdbsync_in + p->failover_in + p->conf_in + p->syspri_in + p->peer_fdb_in + p->disconnect_in;
        tx_total = p->open_out + p->keepalive_out + p->fdbsync_out + p->failover_out + p->conf_out + p->syspri_out + p->peer_fdb_out + p->disconnect_out;

        sal_fprintf(fp, "%-12s : received %u, sent %u\n", "Total", rx_total, tx_total);
        sal_fprintf(fp, "%-12s : received %u, sent %u\n", "Open", p->open_in, p->open_out);
        sal_fprintf(fp, "%-12s : received %u, sent %u\n", "KAlive", p->keepalive_in, p->keepalive_out);
        sal_fprintf(fp, "%-12s : received %u, sent %u\n", "Disconnect", p->disconnect_in, p->disconnect_out);
        sal_fprintf(fp, "%-12s : received %u, sent %u\n", "Fdb sync", p->fdbsync_in, p->fdbsync_out);
        sal_fprintf(fp, "%-12s : received %u, sent %u\n", "Failover", p->failover_in, p->failover_out);
        sal_fprintf(fp, "%-12s : received %u, sent %u\n", "Conf", p->conf_in, p->conf_out);
        sal_fprintf(fp, "%-12s : received %u, sent %u\n", "Syspri", p->syspri_in, p->syspri_out);
        sal_fprintf(fp, "%-12s : received %u, sent %u\n", "Peer fdb", p->peer_fdb_in, p->peer_fdb_out);
#if 0   /* disable MLAG xSTP */
        sal_fprintf(fp, "%-12s : received %u, sent %u\n", "STP Total", p->stp_in, p->stp_out);
        sal_fprintf(fp, "%-12s : received %u, sent %u\n", " Global", p->stp_global_in, p->stp_global_out);
        sal_fprintf(fp, "%-12s : received %u, sent %u\n", " Packet", p->stp_packet_in, p->stp_packet_out);
        sal_fprintf(fp, "%-12s : received %u, sent %u\n", " Instance", p->stp_instance_in, p->stp_state_out);
        sal_fprintf(fp, "%-12s : received %u, sent %u\n", " State", p->stp_state_in, p->stp_state_out);
        if (p->stp_in != p->stp_global_in + p->stp_packet_in + p->stp_instance_in + p->stp_state_in)
        {
            sal_fprintf(fp, "STP input count error!\n");
        }
        if(p->stp_out!= p->stp_global_out + p->stp_packet_out + p->stp_instance_out + p->stp_state_out)
        {
            sal_fprintf(fp, "STP output count error!\n");
        }
#endif
        sal_fprintf(fp, "\n");

        sal_fprintf(fp, "Connections established %u; dropped %u\n", p->established, p->dropped);
   
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_mlag_cmd_show_mlag_interface(cfg_cmd_para_t *para)
{
    tbl_interface_t *p_db_if = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];
    char local_state[IFNAME_EXT_SIZE];
    tbl_mlag_port_t *p_db_port = NULL;
    ctclib_slistnode_t *listnode = NULL;
    tbl_mlag_port_master_t *p_master = tbl_mlag_port_get_master();
    FILE *fp = NULL;
    uint32 banner = TRUE;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    
    CTCLIB_SLIST_LOOP(p_master->port_list, p_db_port, listnode)
    {
        if (banner)
        {
            sal_fprintf(fp, "%-7s %-9s %-14s %s\n", "mlagid", "local-if", "local-state", "remote-state");
            sal_fprintf(fp, "-------+---------+--------------+--------------\n");
            banner = FALSE;
        }
        
        p_db_if = (tbl_interface_t*)p_db_port->p_if;
        if (p_db_if)
        {
            IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
            if (tbl_interface_is_running(p_db_if))
            {
                sal_strcpy(local_state, "up");
            }
            else
            {
                sal_strcpy(local_state, "down");
            }
        }
        else
        {
            sal_strcpy(ifname_ext, "-");
            sal_strcpy(local_state, "-");
        }
        sal_fprintf(fp, "%-7u %-9s %-14s %s\n", 
            p_db_port->key.id,
            ifname_ext, 
            local_state,
            ((p_db_port->peer_conf) ? ((p_db_port->peer_if_up) ? "up":"down") : "-"));
    }
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
mlag_cmd_process_show_mlag(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();
    
    if (0 == argc)
    {
        _mlag_cmd_show_mlag(para);
    }
    else if (1 == argc)
    {
        if (0 == sal_strcmp("peer", argv[0]))
        {
            _mlag_cmd_show_mlag_peer(para);
        }
        else if (0 == sal_strcmp("interface", argv[0]))
        {
            _mlag_cmd_show_mlag_interface(para);
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameter number");
        }
    }
    else
    {
        CFG_INVALID_PARAM_RET("Invalid parameter number");
    }
    
    return rc;
}

int32
mlag_cmd_process_clear_counters(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    return mlag_api_clear_counters(para);
}

