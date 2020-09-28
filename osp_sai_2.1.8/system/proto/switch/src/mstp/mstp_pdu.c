
#include "proto.h"
#include "gen/tbl_lag_global_define.h"
#include "gen/tbl_lag_global.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_mstp_port_define.h"
#include "gen/tbl_mstp_port.h"
#include "gen/tbl_msti_port_define.h"
#include "gen/tbl_msti_port.h"
#include "gen/tbl_mstp_global_define.h"
#include "gen/tbl_mstp_global.h"
#include "gen/tbl_mstp_instance_define.h"
#include "gen/tbl_mstp_instance.h"
#include "gen/tbl_errdisable_define.h"
#include "gen/tbl_errdisable.h"
#include "switch.h"
#include "mstp_define.h"
#include "mstp_pdu.h"
#include "mstp_api.h"
#include "mstp_sm.h"
#include "mstp.h"
#include "errdisable.h"

#ifdef CONFIG_MSTP

static mac_addr_t mstp_mac_address      = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x00};
static mac_addr_t mstp_zero_mac_address = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

char*
bpdu_version_val2str(uint8 version)
{
    switch (version)
    {
    case BPDU_VERSION_STP:
        return "stp";
    case BPDU_VERSION_RSTP:
        return "rstp";
    case BPDU_VERSION_MSTP:
        return "mstp";
    default:
        return "invalid";
    }
}

char*
bpdu_type_val2str(uint8 type)
{
    switch (type)
    {
    case BPDU_TYPE_CONFIG:
        return "config";
    case BPDU_TYPE_RST:
        return "rstp";
    case BPDU_TYPE_MST:
        return "mstp";
    case BPDU_TYPE_TCN:
        return "tcn";
    default:
        return "invalid";
    }
}

int32
mstp_pdu_encode_tcn(uint8 *p_buf, uint32 len, tbl_interface_t *p_db_if, tbl_mstp_port_t *p_db_port)
{
    int32   len_left = len;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    
    size = &len_left;
    ppnt = &p_buf;

    /* L2 header */
    MSG_ENCODE_PUT (mstp_mac_address, MAC_ADDR_LEN);
    MSG_ENCODE_PUT (p_db_if->mac_addr, MAC_ADDR_LEN);
    MSG_ENCODE_PUTW(BPDU_STP_TCN_SIZE);
    
    MSG_ENCODE_PUTC(LLC_DEFAULT_ADDR);
    MSG_ENCODE_PUTC(LLC_DEFAULT_ADDR);
    MSG_ENCODE_PUTC(LLC_DEFAULT_CTRL);

    MSG_ENCODE_PUTW(0);                     /* Protocol Identifier */ 
    MSG_ENCODE_PUTC(BPDU_VERSION_STP);      /* Protocol Version Identifier */
    MSG_ENCODE_PUTC(BPDU_TYPE_TCN);         /* BPDU Type */

    return (len - len_left);
}

mstp_bpdu_role_t
mstp_pdu_get_bpdu_role(tbl_mstp_port_t *p_db_port)
{
    /*
    9.2.9 Encoding of Port Role values
    a) A value of 0 indicates Unknown.
    b) A value of 1 indicates Alternate or Backup.
    c) A value of 2 indicates Root.
    d) A value of 3 indicates Designated.
    */
    switch (p_db_port->role)
    {
    case ROLE_ALTERNATE:
    case ROLE_BACKUP:
        return BPDU_ROLE_ALTERNATE_BACKUP;
    
    case ROLE_ROOTPORT:
        return BPDU_ROLE_ROOT;

    case ROLE_DESIGNATED:
        return BPDU_ROLE_DESIGNATED;

    default:
        return BPDU_ROLE_UNKNOWN;
    }
}

static mstp_bpdu_role_t
_mstp_pdu_msti_get_bpdu_role(tbl_msti_port_t *p_db_msti)
{
    /*
    9.2.9 Encoding of Port Role values
    a) A value of 0 indicates Unknown.
    b) A value of 1 indicates Alternate or Backup.
    c) A value of 2 indicates Root.
    d) A value of 3 indicates Designated.
    */
    switch (p_db_msti->role)
    {
    case ROLE_ALTERNATE:
    case ROLE_BACKUP:
        return BPDU_ROLE_ALTERNATE_BACKUP;
    
    case ROLE_ROOTPORT:
        return BPDU_ROLE_ROOT;

    case ROLE_DESIGNATED:
        return BPDU_ROLE_DESIGNATED;

    default:
        return BPDU_ROLE_UNKNOWN;
    }
}

int32
mstp_pdu_encode_msti(tbl_mstp_port_t *p_db_port, tbl_msti_port_t *p_db_msti, int32 *size, uint8 **ppnt)
{
    tbl_mstp_instance_key_t inst_key;
    tbl_mstp_instance_t *p_db_inst = NULL;
    uint8 flags = 0;
    uint8 bpdu_role = 0;
    uint8 agree = 0;
    char ifname_ext[IFNAME_EXT_SIZE];

    IFNAME_ETH2FULL(p_db_port->key.name, ifname_ext);

    inst_key.instance = p_db_msti->key.instance;
    p_db_inst = tbl_mstp_instance_get_mstp_instance(&inst_key);
    if (NULL == p_db_inst)
    {
        return PM_E_NONE;
    }
    
    /* Merged from E580 by kcao for bug 37813 2016-06-13, liwh for bug 31643, 2015-01-20 */
    if (ROLE_DESIGNATED == p_db_msti->role)
    {
        agree = TRUE;
    }
    else
    {
        agree = p_db_msti->agree;
    }
    
    bpdu_role = _mstp_pdu_msti_get_bpdu_role(p_db_msti);
    flags =   (CTC_TASK_IS_RUNNING(p_db_msti->tcWhile_timer) ? 0x01 : 0)
            | ((p_db_msti->proposing) ? 0x02 : 0)
            | ((bpdu_role & 0x03) << 2)
            | ((PST_LEARNING == p_db_msti->pst_state || PST_FORWARDING == p_db_msti->pst_state) ? 0x10 : 0)
            | ((PST_FORWARDING == p_db_msti->pst_state) ? 0x20 : 0)
            | ((agree) ? 0x40 : 0)
            | ((p_db_msti->tcAck) ? 0x80 : 0);  /* MSTPTODO 0x80 should be master */

    
    MSTP_DEBUG(STP_PACKET_TX, "Interface %s instance %u TX RSTP flags 0x%02X (TC %u, TCAck %u, proposal %u, learn %u, forward %u, agree %u, role %s)",
        ifname_ext,
        p_db_msti->key.instance,
        flags,
        GLB_FLAG_ISSET(flags, 0x01),
        GLB_FLAG_ISSET(flags, 0x80),
        GLB_FLAG_ISSET(flags, 0x02),
        GLB_FLAG_ISSET(flags, 0x10),
        GLB_FLAG_ISSET(flags, 0x20),
        GLB_FLAG_ISSET(flags, 0x40),
        cdb_enum_val2str(mstp_bpdu_role_strs, BPDU_ROLE_MAX, bpdu_role));

    MSG_ENCODE_PUTC(flags);

    MSG_ENCODE_PUTW(p_db_msti->designated_regionRootId.priority);
    MSG_ENCODE_PUT(p_db_msti->designated_regionRootId.address, MAC_ADDR_LEN);
    MSG_ENCODE_PUTL(p_db_msti->designated_intRootPathCost);
    MSG_ENCODE_PUTC((p_db_inst->bridge_priority>>8)&0xFF);
    MSG_ENCODE_PUTC(((p_db_msti->portId>>8) & 0xF0));
   
    MSG_ENCODE_PUTC(p_db_msti->hopCount);

    return PM_E_NONE;
}

int32
mstp_pdu_encode_mst(tbl_mstp_port_t *p_db_port, int32 *size, uint8 **ppnt)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_msti_port_t *p_db_msti = NULL;
    ctclib_slistnode_t *listnode = NULL;
    uint16 version3_len = 0;
    uint8 *p_version3 = *ppnt;
    uint8 *p_msti_start = NULL;
    
    /* skip the len field as we will only know later */ 
    MSG_ENCODE_PUT_EMPTY(2);
    p_msti_start = *ppnt;

    MSG_ENCODE_PUTC(p_db_glb->mstp_format_id);     /* cfg_format_id Format specified by IEEE 802.1s */

    MSG_ENCODE_PUT(p_db_glb->mstp_region_name, MSTP_REGION_NAME_LEN);
    MSG_ENCODE_PUTW(p_db_glb->mstp_revision_level);
    MSG_ENCODE_PUT(p_db_glb->mstp_digest.data, MSTP_DIGEST_LEN);

    MSG_ENCODE_PUTL(p_db_port->designated_intRootPathCost);
    MSG_ENCODE_PUTW(p_db_port->designated_designatedBridgeId.priority);
    MSG_ENCODE_PUT(p_db_port->designated_designatedBridgeId.address, MAC_ADDR_LEN);

    MSG_ENCODE_PUTC(p_db_port->hopCount);

    CTCLIB_SLIST_LOOP(p_db_port->msti_list.obj_list, p_db_msti, listnode)
    {
        mstp_pdu_encode_msti(p_db_port, p_db_msti, size, ppnt);
    }

    /* encode version3 length */
    version3_len = (*ppnt) - p_msti_start;
    *(p_version3) = ((version3_len) >> 8) & 0xFF;
    *(p_version3 + 1) = (version3_len) & 0xFF;
    
    return PM_E_NONE;
}

int32
mstp_pdu_encode_bpdu(uint8 *p_buf, uint32 len, tbl_interface_t *p_db_if, tbl_mstp_port_t *p_db_port, uint32 is_rstp)
{
#define BPDU_L2HDR_LEN  14
    mstp_bpdu_role_t bpdu_role = 0;
    uint16 bpdu_len = 0;
    uint32  value = 0;
    uint8   flags = 0;
    int32   len_left = len;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    uint8 agree = FALSE;
    char ifname_ext[IFNAME_EXT_SIZE];
    uint8 *p_bpdu_len = NULL;

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    
    size = &len_left;
    ppnt = &p_buf;

    /* L2 header */
    MSG_ENCODE_PUT(mstp_mac_address, MAC_ADDR_LEN);
    MSG_ENCODE_PUT(p_db_if->mac_addr, MAC_ADDR_LEN);
    /* bypass uint16 length field */
    p_bpdu_len = *ppnt;
    MSG_ENCODE_PUT_EMPTY(2);
    
    MSG_ENCODE_PUTC(LLC_DEFAULT_ADDR);
    MSG_ENCODE_PUTC(LLC_DEFAULT_ADDR);
    MSG_ENCODE_PUTC(LLC_DEFAULT_CTRL);
    
    MSG_ENCODE_PUTW(0);                         /* Protocol Identifier */ 
    if (is_rstp)
    {
        if (p_db_port->forceVersion >= BPDU_VERSION_MSTP)
        {
            MSG_ENCODE_PUTC(BPDU_VERSION_MSTP);     /* Protocol Version Identifier */
            MSG_ENCODE_PUTC(BPDU_TYPE_RST);         /* BPDU Type */
        }
        else
        {
            MSG_ENCODE_PUTC(BPDU_VERSION_RSTP);     /* Protocol Version Identifier */
            MSG_ENCODE_PUTC(BPDU_TYPE_RST);         /* BPDU Type */
        }
    }
    else
    {
        MSG_ENCODE_PUTC(BPDU_VERSION_STP);      /* Protocol Version Identifier */
        MSG_ENCODE_PUTC(BPDU_TYPE_CONFIG);      /* BPDU Type */
    }

    /* 2. Set flags
                                0 1 2 3 4 5 6 7 
                               +-+-+-+-+---+-+-+
                                | | | |  |  | |
    topology_change_ack(0x80) --+ | | |  |  | +- topology_change  (0x01)
          agreement    (0x40) ----+ | |  |  +--- proposal         (0x02)
            forwarding (0x20) ------+ |  +------ port_role        (0x0C)
                                      +--------- learning         (0x10)
    */

    if (is_rstp)
    {    
        /* Merged from E580 by kcao for bug 37813 2016-06-13, liwh for bug 31643, 2015-01-20 */
        if (ROLE_DESIGNATED == p_db_port->role)
        {
            agree = TRUE;
        }
        else
        {
            agree = p_db_port->agree;
        }
        
        bpdu_role = mstp_pdu_get_bpdu_role(p_db_port);
        flags =   (CTC_TASK_IS_RUNNING(p_db_port->tcWhile_timer) ? 0x01 : 0)
                | ((p_db_port->proposing) ? 0x02 : 0)
                | ((bpdu_role & 0x03) << 2)
                | ((PST_LEARNING == p_db_port->pst_state || PST_FORWARDING == p_db_port->pst_state) ? 0x10 : 0)
                | ((PST_FORWARDING == p_db_port->pst_state) ? 0x20 : 0)
                | ((agree) ? 0x40 : 0)
                | ((p_db_port->tcAck) ? 0x80 : 0);

        MSTP_DEBUG(STP_PACKET_TX, "Interface %s TX RSTP flags 0x%02X (TC %u, TCAck %u, proposal %u, learn %u, forward %u, agree %u, role %s)",
            ifname_ext,
            flags,
            GLB_FLAG_ISSET(flags, 0x01),
            GLB_FLAG_ISSET(flags, 0x80),
            GLB_FLAG_ISSET(flags, 0x02),
            GLB_FLAG_ISSET(flags, 0x10),
            GLB_FLAG_ISSET(flags, 0x20),
            GLB_FLAG_ISSET(flags, 0x40),
            cdb_enum_val2str(mstp_bpdu_role_strs, BPDU_ROLE_MAX, bpdu_role));
    }
    else
    {
        flags = (CTC_TASK_IS_RUNNING(p_db_port->tcWhile_timer) ? 0x01 : 0)
                | ((p_db_port->tcAck) ? 0x80 : 0);

        MSTP_DEBUG(STP_PACKET_TX, "Interface %s TX STP flags 0x%02X (TC %u, TCAck %u)",
            ifname_ext,
            flags,
            GLB_FLAG_ISSET(flags, 0x01),
            GLB_FLAG_ISSET(flags, 0x80));
    }
    MSG_ENCODE_PUTC(flags);
    MSG_ENCODE_PUTW(p_db_port->designated_rootId.priority);
    MSG_ENCODE_PUT(p_db_port->designated_rootId.address, MAC_ADDR_LEN);
    MSG_ENCODE_PUTL(p_db_port->designated_rootPathCost);
    if (p_db_port->forceVersion >= BPDU_VERSION_MSTP)
    {
        MSG_ENCODE_PUTW(p_db_port->designated_regionRootId.priority);
        MSG_ENCODE_PUT(p_db_port->designated_regionRootId.address, MAC_ADDR_LEN);
    }
    else
    {
        MSG_ENCODE_PUTW(p_db_port->designated_designatedBridgeId.priority);
        MSG_ENCODE_PUT(p_db_port->designated_designatedBridgeId.address, MAC_ADDR_LEN);
    }
    MSG_ENCODE_PUTW(p_db_port->portId);
    value = p_db_port->port_messageAge * MSTP_ONE_SEC_UNIT;
    MSG_ENCODE_PUTW(value);
    value = p_db_port->port_maxAge * MSTP_ONE_SEC_UNIT;
    MSG_ENCODE_PUTW(value);
    value = p_db_port->port_helloTime * MSTP_ONE_SEC_UNIT;
    MSG_ENCODE_PUTW(value);
    value = p_db_port->port_fwdDelay * MSTP_ONE_SEC_UNIT;
    MSG_ENCODE_PUTW(value);

    if (is_rstp)
    {
        MSG_ENCODE_PUTC(BPDU_VERSION_1_LEN);
    }
    
    MSTP_DEBUG(STP_PACKET_TX, "Interface %s TX rootId (%04X-%02X%02X.%02X%02X.%02X%02X) rootPathCost (%u) designatedBridgeId (%04X-%02X%02X.%02X%02X.%02X%02X)",
        ifname_ext,
        p_db_port->designated_rootId.priority,
        p_db_port->designated_rootId.address[0], p_db_port->designated_rootId.address[1], p_db_port->designated_rootId.address[2],
        p_db_port->designated_rootId.address[3], p_db_port->designated_rootId.address[4], p_db_port->designated_rootId.address[5],
        p_db_port->designated_rootPathCost,
        p_db_port->designated_designatedBridgeId.priority,
        p_db_port->designated_designatedBridgeId.address[0], p_db_port->designated_designatedBridgeId.address[1], p_db_port->designated_designatedBridgeId.address[2],
        p_db_port->designated_designatedBridgeId.address[3], p_db_port->designated_designatedBridgeId.address[4], p_db_port->designated_designatedBridgeId.address[5]
        );

    if (is_rstp)
    {
        if (p_db_port->forceVersion >= BPDU_VERSION_MSTP)
        {
            /* Encode MSTP Parameters */
            len += mstp_pdu_encode_mst(p_db_port, size, ppnt);
        }
    }

    bpdu_len = (len - len_left) - BPDU_L2HDR_LEN;
    *(p_bpdu_len) = ((bpdu_len) >> 8) & 0xFF;
    *(p_bpdu_len + 1) = (bpdu_len) & 0xFF;
    
    return (len - len_left);
}

int32
mstp_pdu_tx_tcn(tbl_mstp_port_t *p_db_port)
{
    char ifname_ext[IFNAME_EXT_SIZE];
    tbl_interface_t *p_db_if = NULL;
    uint8 pkt[BPDU_PKT_LEN];
    int32 pkt_len = 0;
    int32 rc = PM_E_NONE;
    
    if (p_db_port->operEdge && p_db_port->oper_bpdufilter)
    {
        return PM_E_FAIL;
    }

    p_db_if = (tbl_interface_t*)p_db_port->p_if;
    pkt_len = mstp_pdu_encode_tcn(pkt, BPDU_PKT_LEN, p_db_if, p_db_port);
    if (pkt_len < 0)
    {
        return PM_E_FAIL;
    }
    PM_E_RETURN(ipc_pkt_tx_bpdu(pkt, pkt_len, p_db_port->ifindex));

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
    MSTP_DEBUG(STP_PACKET_TX, "Interface %s TX TCN", ifname_ext);

    return PM_E_NONE;
}

int32
mstp_pdu_tx_bpdu(tbl_mstp_port_t *p_db_port, uint32 is_rstp)
{
    tbl_interface_t *p_db_if = NULL;
    uint8 pkt[BPDU_PKT_LEN];
    int32 pkt_len = 0;
    int32 rc = PM_E_NONE;

    if (p_db_port->operEdge && p_db_port->oper_bpdufilter)
    {
        return PM_E_FAIL;
    }
  
    p_db_if = (tbl_interface_t*)p_db_port->p_if;
    pkt_len = mstp_pdu_encode_bpdu(pkt, BPDU_PKT_LEN, p_db_if, p_db_port, is_rstp);
    if (pkt_len < 0)
    {
        return PM_E_FAIL;
    }
    PM_E_RETURN(ipc_pkt_tx_bpdu(pkt, pkt_len, p_db_if->ifindex));
    
    return PM_E_NONE;
}

static int32 
_mstp_pdu_decode_bpdu(tbl_interface_t *p_db_if, tbl_mstp_port_t *p_db_port, bpdu_t *p_bpdu,
    uint8 *p_buf, int32 bpdu_len, char *p_errstr, uint32 errstr_len)
{
    uint32  i = 0;
    uint8   version1_len = 0;
    uint8   flags = 0;
    int32   len_left = bpdu_len;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    char ifname_ext[IFNAME_EXT_SIZE];

    IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);

    size = &len_left;
    ppnt = &p_buf;

    sal_memset(p_bpdu, 0, sizeof(bpdu_t));

    /* 1. Parse header */
    MSG_DECODE_GETW(p_bpdu->proto_id);
    MSG_DECODE_GETC(p_bpdu->version);
    MSG_DECODE_GETC(p_bpdu->type);
    
    if (BPDU_TYPE_TCN == p_bpdu->type)
    {
        MSTP_DEBUG(STP_PACKET_RX, "Interface %s RX TCN", ifname_ext);
        return PM_E_NONE;
    }

    /* 2. Parse flags
                                0 1 2 3 4 5 6 7 
                               +-+-+-+-+---+-+-+
                                | | | |  |  | |
    topology_change_ack(0x80) --+ | | |  |  | +- topology_change  (0x01)
          agreement    (0x40) ----+ | |  |  +--- proposal         (0x02)
            forwarding (0x20) ------+ |  +------ port_role        (0x0C)
                                      +--------- learning         (0x10)
    */
    MSG_DECODE_GETC(flags);

    switch (p_bpdu->version)
    {
    case BPDU_VERSION_STP:
        if (BPDU_STP_LEN != bpdu_len)
        {
            sal_snprintf(p_errstr, errstr_len, "STP BPDU length is %u, should be %u", bpdu_len, BPDU_STP_LEN);
            return PM_E_FAIL;
        }
        break;
    case BPDU_VERSION_RSTP:
        if (BPDU_RSTP_LEN != bpdu_len)
        {
            sal_snprintf(p_errstr, errstr_len, "RSTP BPDU length is %u, should be %u", bpdu_len, BPDU_RSTP_LEN);
            return PM_E_FAIL;
        }
        break;
    case BPDU_VERSION_MSTP:
        if (BPDU_RSTP_LEN > bpdu_len)
        {
            sal_snprintf(p_errstr, errstr_len, "RSTP BPDU length is %u, less than minimum %u", bpdu_len, BPDU_RSTP_LEN);
            return PM_E_FAIL;
        }
        break;
        
    default:
        sal_snprintf(p_errstr, errstr_len, "RSTP BPDU version is %u", p_bpdu->version);
        return PM_E_FAIL;
    }
    
    p_bpdu->topology_change = (flags & 0x01);
    if (p_bpdu->version >= BPDU_VERSION_RSTP)
    {            
        p_bpdu->proposal   = (flags & 0x02) ? TRUE : FALSE;
        p_bpdu->port_role  = (flags & 0x0C) >> 2;
        p_bpdu->learning   = (flags & 0x10) ? TRUE : FALSE;
        p_bpdu->forwarding = (flags & 0x20) ? TRUE : FALSE;
        p_bpdu->agreement  = (flags & 0x40) ? TRUE : FALSE;
    }
    p_bpdu->topology_change_ack = (flags & 0x80) ? TRUE : FALSE;

    MSTP_DEBUG(STP_PACKET_RX, "Interface %s RX %s, flags 0x%02X (TC %u, TCAck %u, proposal %u, learn %u, forward %u, agree %u, role %s)",
        ifname_ext, bpdu_version_val2str(p_bpdu->version),
        flags,
        p_bpdu->topology_change,
        p_bpdu->topology_change_ack,
        p_bpdu->proposal,
        p_bpdu->learning,
        p_bpdu->forwarding,
        p_bpdu->agreement,
        cdb_enum_val2str(mstp_bpdu_role_strs, BPDU_ROLE_MAX, p_bpdu->port_role));

    /* 3. Parse STP/RSTP */
    MSG_DECODE_GETW(p_bpdu->root_id.priority);
    MSG_DECODE_GET(p_bpdu->root_id.address, MAC_ADDR_LEN);
    MSG_DECODE_GETL(p_bpdu->external_rpc);
    
    MSG_DECODE_GETW(p_bpdu->reg_root_id.priority);
    MSG_DECODE_GET(p_bpdu->reg_root_id.address, MAC_ADDR_LEN);
    if (BPDU_VERSION_MSTP != p_bpdu->version)
    {
        p_bpdu->bridge_id.priority = p_bpdu->reg_root_id.priority;
        sal_memcpy(p_bpdu->bridge_id.address, p_bpdu->reg_root_id.address, MAC_ADDR_LEN);
    }
    MSG_DECODE_GETW(p_bpdu->port_id);

    
    if (0 == sal_memcmp(mstp_zero_mac_address, p_bpdu->root_id.address, MAC_ADDR_LEN))
    {
        sal_snprintf(p_errstr, errstr_len, "BPDU rootId address is all ZERO");
        return PM_E_INVALID_PARAM;
    }
    
    if (BPDU_VERSION_MSTP == p_bpdu->version)
    {
    }
    else
    {
        if (0 == sal_memcmp(mstp_zero_mac_address, p_bpdu->bridge_id.address, MAC_ADDR_LEN))
        {
            sal_snprintf(p_errstr, errstr_len, "BPDU bridgeId address is all ZERO");
            return PM_E_INVALID_PARAM;
        }
    }

    MSG_DECODE_GETW(p_bpdu->messageAge);
    MSG_DECODE_GETW(p_bpdu->maxAge);
    MSG_DECODE_GETW(p_bpdu->helloTime);
    MSG_DECODE_GETW(p_bpdu->fwdDelay);

    p_bpdu->messageAge  = p_bpdu->messageAge / MSTP_ONE_SEC_UNIT;
    p_bpdu->maxAge      = p_bpdu->maxAge / MSTP_ONE_SEC_UNIT;
    p_bpdu->helloTime   = p_bpdu->helloTime / MSTP_ONE_SEC_UNIT;
    p_bpdu->fwdDelay    = p_bpdu->fwdDelay / MSTP_ONE_SEC_UNIT;

    /* Added by kcao for bug 37833 */
    if ((p_bpdu->maxAge < MSTP_TIMER_MIN_MAX_AGE) || (p_bpdu->maxAge > MSTP_TIMER_MAX_MAX_AGE))
    {
        sal_snprintf(p_errstr, errstr_len, "BPDU maxAge is %u, should be [%u, %u]", p_bpdu->maxAge, MSTP_TIMER_MIN_MAX_AGE, MSTP_TIMER_MAX_MAX_AGE);
        return PM_E_INVALID_PARAM;
    }
    if ((p_bpdu->helloTime < MSTP_TIMER_MIN_HELLO_TIME) || (p_bpdu->helloTime > MSTP_TIMER_MAX_HELLO_TIME))
    {
        sal_snprintf(p_errstr, errstr_len, "BPDU helloTime is %u, should be [%u, %u]", p_bpdu->helloTime, MSTP_TIMER_MIN_HELLO_TIME, MSTP_TIMER_MAX_HELLO_TIME);
        return PM_E_INVALID_PARAM;
    }
    if ((p_bpdu->fwdDelay < MSTP_TIMER_MIN_FWD_DELAY) || (p_bpdu->helloTime > MSTP_TIMER_MAX_FWD_DELAY))
    {
        sal_snprintf(p_errstr, errstr_len, "BPDU fwdDelay is %u, should be [%u, %u]", p_bpdu->fwdDelay, MSTP_TIMER_MIN_FWD_DELAY, MSTP_TIMER_MAX_FWD_DELAY);
        return PM_E_INVALID_PARAM;
    }

    /* 
     * 9.3.1 Configuration BPDUs
     * The Message Age (Octets 28 and 29) shall be less than Max Age (Octets 30 and 31).
     */
    if ((p_bpdu->messageAge >= p_bpdu->maxAge))
    {
        sal_snprintf(p_errstr, errstr_len, "BPDU messageAge %u >= maxAge %u", p_bpdu->messageAge, p_bpdu->maxAge);
        return PM_E_INVALID_PARAM;
    }

    MSG_DECODE_GETC(version1_len);
    (void)version1_len;

    /* 4. Parse MSTP Instance */
    if (p_bpdu->version >= BPDU_VERSION_MSTP)
    {
        MSG_DECODE_GETW(p_bpdu->version3_len);
        if (0 == p_bpdu->version3_len)
        {
            sal_snprintf(p_errstr, errstr_len, "BPDU MSTP version3 length is ZERO");
            return PM_E_INVALID_PARAM;
        }

        MSG_DECODE_GETC(p_bpdu->mstp_cfg.format_id);
        MSG_DECODE_GET(p_bpdu->mstp_cfg.region_name, MSTP_REGION_NAME_LEN);
        p_bpdu->mstp_cfg.region_name[MSTP_REGION_NAME_LEN] = '\0';
        MSG_DECODE_GETW(p_bpdu->mstp_cfg.revision_level);
        MSG_DECODE_GET(p_bpdu->mstp_cfg.digest, MSTP_DIGEST_LEN);
        
        MSG_DECODE_GETL(p_bpdu->internal_rpc);
        MSG_DECODE_GETW(p_bpdu->bridge_id.priority);
        MSG_DECODE_GET(p_bpdu->bridge_id.address, MAC_ADDR_LEN);
        MSG_DECODE_GETC(p_bpdu->mstp_hops);
        if (0 == sal_memcmp(mstp_zero_mac_address, p_bpdu->bridge_id.address, MAC_ADDR_LEN))
        {
            sal_snprintf(p_errstr, errstr_len, "BPDU MSTP bridgeId address is all ZERO");
            return PM_E_INVALID_PARAM;
        }
        p_bpdu->msti_num = 0;
        p_bpdu->msti_num = (p_bpdu->version3_len - 64) / BPDU_MSTI_LEN;

        for (i = 0; i < p_bpdu->msti_num; i++)
        {
            flags = 0;
            MSG_DECODE_GETC(flags);
            p_bpdu->msti[i].topology_change = (flags & 0x01) ? TRUE : FALSE;
            p_bpdu->msti[i].proposal   = (flags & 0x02) ? TRUE : FALSE;
            p_bpdu->msti[i].port_role  = (flags & 0x0C) >> 2;
            p_bpdu->msti[i].learning   = (flags & 0x10) ? TRUE : FALSE;
            p_bpdu->msti[i].forwarding = (flags & 0x20) ? TRUE : FALSE;
            p_bpdu->msti[i].agreement  = (flags & 0x40) ? TRUE : FALSE;
            p_bpdu->msti[i].master = (flags & 0x80) ? TRUE : FALSE;
            
            MSG_DECODE_GETW(p_bpdu->msti[i].reg_root_id.priority);
            MSG_DECODE_GET(p_bpdu->msti[i].reg_root_id.address, MAC_ADDR_LEN);
            MSG_DECODE_GETL(p_bpdu->msti[i].internal_rpc);
            MSG_DECODE_GETC(p_bpdu->msti[i].bridge_priority);
            MSG_DECODE_GETC(p_bpdu->msti[i].port_priority);
            MSG_DECODE_GETC(p_bpdu->msti[i].hops);

            p_bpdu->msti[i].instance_id = (p_bpdu->msti[i].reg_root_id.priority & 0xFFF);
            mstp_make_bridge_id(&p_bpdu->msti[i].bridge_id, p_bpdu->bridge_id.address, 
                            ((p_bpdu->msti[i].bridge_priority<<8)&0xFF00), p_bpdu->msti[i].instance_id);
            p_bpdu->msti[i].port_id = (((p_bpdu->msti[i].port_priority << 8) & 0xF000) | (p_bpdu->port_id & 0xFFF));
            p_bpdu->msti[i].cist = p_bpdu;
        }
    }
    else
    {
        p_bpdu->internal_rpc = p_bpdu->external_rpc;
    }

    if (p_bpdu->version >= BPDU_VERSION_MSTP)
    {
        MSTP_DEBUG(STP_PACKET_RX, "Interface %s msti_num %u, RX rootId (%04X-%02X%02X.%02X%02X.%02X%02X) externalRootPathCost (%u) regRootId (%04X-%02X%02X.%02X%02X.%02X%02X) internalRootPathCost (%u) designatedBridgeId (%04X-%02X%02X.%02X%02X.%02X%02X)",
            ifname_ext,
            p_bpdu->msti_num,
            p_bpdu->root_id.priority,
            p_bpdu->root_id.address[0], p_bpdu->root_id.address[1], p_bpdu->root_id.address[2],
            p_bpdu->root_id.address[3], p_bpdu->root_id.address[4], p_bpdu->root_id.address[5],
            p_bpdu->external_rpc,
            p_bpdu->reg_root_id.priority,
            p_bpdu->reg_root_id.address[0], p_bpdu->reg_root_id.address[1], p_bpdu->reg_root_id.address[2],
            p_bpdu->reg_root_id.address[3], p_bpdu->reg_root_id.address[4], p_bpdu->reg_root_id.address[5],
            p_bpdu->internal_rpc,
            p_bpdu->bridge_id.priority,
            p_bpdu->bridge_id.address[0], p_bpdu->bridge_id.address[1], p_bpdu->bridge_id.address[2],
            p_bpdu->bridge_id.address[3], p_bpdu->bridge_id.address[4], p_bpdu->bridge_id.address[5]
            );
    }
    else
    {
        MSTP_DEBUG(STP_PACKET_RX, "Interface %s RX rootId (%04X-%02X%02X.%02X%02X.%02X%02X) rootPathCost (%u) designatedBridgeId (%04X-%02X%02X.%02X%02X.%02X%02X)",
            ifname_ext,
            p_bpdu->root_id.priority,
            p_bpdu->root_id.address[0], p_bpdu->root_id.address[1], p_bpdu->root_id.address[2],
            p_bpdu->root_id.address[3], p_bpdu->root_id.address[4], p_bpdu->root_id.address[5],
            p_bpdu->external_rpc,
            p_bpdu->bridge_id.priority,
            p_bpdu->bridge_id.address[0], p_bpdu->bridge_id.address[1], p_bpdu->bridge_id.address[2],
            p_bpdu->bridge_id.address[3], p_bpdu->bridge_id.address[4], p_bpdu->bridge_id.address[5]
            );
    }

    return PM_E_NONE;
}

int32
mstp_pdu_decode_bpdu(tbl_interface_t *p_db_if, tbl_mstp_port_t *p_db_port, bpdu_t *p_bpdu, mstp_llc_hdr_t *llc_hdr,
    uint8 *p_buf, uint32 len, char *p_errstr, uint32 errstr_len)
{
    int32   len_left = len;
    int32*  size = NULL;
    uint8** ppnt = NULL;
    int32   bpdu_len = 0;
    int32   rc = PM_E_NONE;
    
    size = &len_left;
    ppnt = &p_buf;

    /*
        0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
      +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
0x0000|           MACDA       |        MACSA          |  LEN  |  LLC  |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
0x0010| LLC |                                                         |
      +-----+                                                         +
      |                              BPDU                             |
      |                                                               |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                                LLC Header Format
    */
    
    /* 1. Decode LLC Header */
    MSG_DECODE_GET(llc_hdr->macda, MAC_ADDR_LEN);
    MSG_DECODE_GET(llc_hdr->macsa, MAC_ADDR_LEN);
    MSG_DECODE_GETW(llc_hdr->len);
    MSG_DECODE_GETC(llc_hdr->dsap);
    MSG_DECODE_GETC(llc_hdr->ssap);
    MSG_DECODE_GETC(llc_hdr->ctrl);

    bpdu_len = llc_hdr->len - LLC_LEN;

#if 0
    /* 2. Check loopback BPDU */
    if (0 == sal_memcmp(p_db_if->mac_addr, llc_hdr.macsa, MAC_ADDR_LEN))
    {
        p_db_port->bpdu_loop_count++;
        if (p_db_port->bpdu_loop_count >= 3)
        {
            /* errdisable */
            errdisable_handle_exception(ERRDIS_BPDU_LOOP, p_db_if, TRUE);
            p_db_port->bpdu_loop_count = 0;
        }
        return PM_E_FAIL;
    }
#endif

    /* 3. Check LLC */
    if (LLC_DEFAULT_ADDR != llc_hdr->dsap
     || LLC_DEFAULT_ADDR != llc_hdr->ssap
     || LLC_DEFAULT_CTRL != llc_hdr->ctrl)
    {
        sal_snprintf(p_errstr, errstr_len, "LLC field is not 0x420303");
        return PM_E_FAIL;
    }
    
    rc = _mstp_pdu_decode_bpdu(p_db_if, p_db_port, p_bpdu,
        p_buf, bpdu_len, p_errstr, errstr_len);
    
    return rc;
}

int32
mstp_pdu_rx(int32 sock, int32 sess_fd, ctc_msg_t *p_msg)
{
    tbl_mstp_global_t *p_db_glb = tbl_mstp_global_get_mstp_global();
    tbl_interface_t *p_db_if = NULL;
    tbl_mstp_port_t *p_db_port = NULL;
    mstp_llc_hdr_t llc_hdr;
    bpdu_t *p_bpdu = NULL;
    uint32 ifindex = 0;
    char errstr[CMD_BUF_64];
    char ifname_ext[IFNAME_EXT_SIZE];
    int32 rc = PM_E_NONE;

    errstr[0] = CMD_ZERO_CHAR;
    
    /* check global enable */
    if (!p_db_glb->enable)
    {
        return PM_E_NONE;
    }
    
    ifindex = p_msg->u_hdr.pkt.ifindex;
    p_db_if = tbl_interface_get_interface_by_ifindex(ifindex);
    if (!p_db_if || !p_db_if->brgif || !p_db_if->mstp_port)
    {
        return PM_E_FAIL;
    }
    if (!tbl_interface_is_running(p_db_if))
    {
        return PM_E_NONE;
    }
    
    p_db_port = p_db_if->mstp_port;
    p_bpdu = p_db_port->bpdu;

    /* check port enable */
    if (!p_db_port->port_enable)
    {
        return PM_E_NONE;
    }

    if (p_db_port->portEnabled)
    {
        if (p_db_port->operEdge)
        {
            if (p_db_port->oper_bpdufilter)
            {
                /* drop */
                rc = PM_E_FAIL;
            }
            else
            {
                if (p_db_port->oper_bpduguard)
                {
                    /* errdisable */
                    errdisable_handle_exception(ERRDIS_BPDU_GUARD, p_db_if, TRUE);
                    rc = PM_E_FAIL;
                }
            }
        }
    }
    else
    {
        rc = PM_E_NONE;
    }

    if (0 == rc)
    {
        sal_memset(&llc_hdr, 0, sizeof(llc_hdr));
        rc = mstp_pdu_decode_bpdu(p_db_if, p_db_port, p_bpdu, &llc_hdr, p_msg->data, p_msg->data_len, errstr, CMD_BUF_64);
        if (!sal_memcmp(llc_hdr.macsa, p_db_if->mac_addr, MAC_ADDR_LEN))
        {
            p_db_port->bpdu_loop_count++;
            if (p_db_port->bpdu_loop_count >= MSTP_BPDU_LOOP_THRESHOLD)
            {
                errdisable_handle_exception(ERRDIS_BPDU_LOOP, p_db_if, TRUE);
                p_db_port->bpdu_loop_count = 0;
            }
        }
    }
        
#if 0
    /* delete by kcao for bug 38829 */
    if (0 == sal_memcmp(p_db_glb->bridge_id.address, p_bpdu->root_id.address, MAC_ADDR_LEN))
    {
        /* If RX root_id.address is equal to self bridge_id.address, not update rx bpdu */
        return PM_E_NONE;
    }
#endif

    if (PM_E_NONE == rc)
    {
        p_db_port->rx_bpdu_count++;
        mstp_sm_rx_bpdu(p_db_port);
        mstp_api_tx(p_db_port);
    }
    else
    {
        IFNAME_ETH2FULL(p_db_if->key.name, ifname_ext);
        MSTP_DEBUG(STP_PACKET_RX, "Interface %s received invalid BPDU for %s", ifname_ext, errstr);
        p_db_port->rx_error_count++;
    }

    return PM_E_NONE;
}

#endif /* CONFIG_MSTP */

