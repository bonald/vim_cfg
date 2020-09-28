
#include "proto.h"
#include "gen/tbl_dot1x_port_define.h"
#include "gen/tbl_dot1x_port.h"
#include "cdb_data_cmp.h"

tbl_dot1x_port_master_t *_g_p_tbl_dot1x_port_master = NULL;

static uint32
_tbl_dot1x_port_hash_make(tbl_dot1x_port_t *p_dot1x_port)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_dot1x_port->key;
    for (index = 0; index < sizeof(p_dot1x_port->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_dot1x_port_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_dot1x_port_t *p_dot1x_port1 = (tbl_dot1x_port_t*)p_arg1;
    tbl_dot1x_port_t *p_dot1x_port2 = (tbl_dot1x_port_t*)p_arg2;

    if (0 == sal_memcmp(&p_dot1x_port1->key, &p_dot1x_port2->key, sizeof(tbl_dot1x_port_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
tbl_dot1x_port_add_dot1x_port_sync(tbl_dot1x_port_t *p_dot1x_port, uint32 sync)
{
    tbl_dot1x_port_master_t *p_master = _g_p_tbl_dot1x_port_master;
    tbl_dot1x_port_t *p_db_dot1x_port = NULL;
    void *p_ret = NULL;

    /* 1. lookup entry exist */
    if (tbl_dot1x_port_get_dot1x_port(&p_dot1x_port->key))
    {
        return PM_E_EXIST;
    }

    /* 2. alloc a new db entry */
    p_db_dot1x_port = XCALLOC(MEM_TBL_DOT1X_PORT, sizeof(tbl_dot1x_port_t));
    if (NULL == p_db_dot1x_port)
    {
        return PM_E_NO_MEMORY;
    }

    /* 3. evaluate db entry */
    sal_memcpy(p_db_dot1x_port, p_dot1x_port, sizeof(tbl_dot1x_port_t));

    /* 4. add to db */
    p_ret = ctclib_hash_get(p_master->dot1x_port_hash, (void*)p_db_dot1x_port, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        return PM_E_NO_MEMORY;
    }
    ctclib_slistnode_insert_sort(p_master->dot1x_port_list, p_db_dot1x_port);

    /* 5. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_add(TBL_DOT1X_PORT, p_db_dot1x_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_add(TBL_DOT1X_PORT, p_db_dot1x_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dot1x_port_del_dot1x_port_sync(tbl_dot1x_port_key_t *p_dot1x_port_key, uint32 sync)
{
    tbl_dot1x_port_master_t *p_master = _g_p_tbl_dot1x_port_master;
    tbl_dot1x_port_t *p_db_dot1x_port = NULL;

    /* 1. lookup entry exist */
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(p_dot1x_port_key);
    if (NULL == p_db_dot1x_port)
    {
        return PM_E_NONE;
    }

    /* 2. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_del(TBL_DOT1X_PORT, p_db_dot1x_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_del(TBL_DOT1X_PORT, p_db_dot1x_port);
        #endif 
    }

    /* 3. delete from db */
    ctclib_hash_release(p_master->dot1x_port_hash, (void*)p_db_dot1x_port);
    ctclib_slistnode_delete(p_master->dot1x_port_list, p_db_dot1x_port);

    /* 4. free db entry */
    XFREE(MEM_TBL_DOT1X_PORT, p_db_dot1x_port);

    return PM_E_NONE;
}

int32
tbl_dot1x_port_set_dot1x_port_field_sync(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_field_id_t field_id, uint32 sync)
{
    tbl_dot1x_port_t *p_db_dot1x_port = NULL;

    /* 1. lookup entry exist */
    p_db_dot1x_port = tbl_dot1x_port_get_dot1x_port(&p_dot1x_port->key);
    if (NULL == p_db_dot1x_port)
    {
        return PM_E_NOT_EXIST;
    }

    /* 2. update db entry field */
    switch (field_id)
    {
    case TBL_DOT1X_PORT_FLD_CURRENT_RADIUS:
        p_db_dot1x_port->current_radius = p_dot1x_port->current_radius;
        break;

    case TBL_DOT1X_PORT_FLD_RADIUS_MSG_ID:
        p_db_dot1x_port->radius_msg_id = p_dot1x_port->radius_msg_id;
        break;

    case TBL_DOT1X_PORT_FLD_RETRY:
        p_db_dot1x_port->retry = p_dot1x_port->retry;
        break;

    case TBL_DOT1X_PORT_FLD_T_WAIT:
        p_db_dot1x_port->t_wait = p_dot1x_port->t_wait;
        break;

    case TBL_DOT1X_PORT_FLD_SESSIONTYPE:
        p_db_dot1x_port->sessiontype = p_dot1x_port->sessiontype;
        break;

    case TBL_DOT1X_PORT_FLD_NAME:
        sal_memcpy(p_db_dot1x_port->name, p_dot1x_port->name, sizeof(p_dot1x_port->name));
        break;

    case TBL_DOT1X_PORT_FLD_BUF:
        sal_memcpy(p_db_dot1x_port->buf, p_dot1x_port->buf, sizeof(p_dot1x_port->buf));
        break;

    case TBL_DOT1X_PORT_FLD_DEST_MAC:
        sal_memcpy(p_db_dot1x_port->dest_mac, p_dot1x_port->dest_mac, sizeof(p_dot1x_port->dest_mac));
        break;

    case TBL_DOT1X_PORT_FLD_SRC_MAC:
        sal_memcpy(p_db_dot1x_port->src_mac, p_dot1x_port->src_mac, sizeof(p_dot1x_port->src_mac));
        break;

    case TBL_DOT1X_PORT_FLD_USER_NAME:
        sal_memcpy(p_db_dot1x_port->user_name, p_dot1x_port->user_name, sizeof(p_dot1x_port->user_name));
        break;

    case TBL_DOT1X_PORT_FLD_AUTHCONTROLLEDPORTCONTROL:
        p_db_dot1x_port->authControlledPortControl = p_dot1x_port->authControlledPortControl;
        break;

    case TBL_DOT1X_PORT_FLD_T_TIMER:
        p_db_dot1x_port->t_timer = p_dot1x_port->t_timer;
        break;

    case TBL_DOT1X_PORT_FLD_PROTOCOL_VERSION:
        p_db_dot1x_port->protocol_version = p_dot1x_port->protocol_version;
        break;

    case TBL_DOT1X_PORT_FLD_GUEST_VLAN_VALID:
        p_db_dot1x_port->guest_vlan_valid = p_dot1x_port->guest_vlan_valid;
        break;

    case TBL_DOT1X_PORT_FLD_GUEST_VLAN:
        p_db_dot1x_port->guest_vlan = p_dot1x_port->guest_vlan;
        break;

    case TBL_DOT1X_PORT_FLD_CURRENT_USER:
        p_db_dot1x_port->current_user = p_dot1x_port->current_user;
        break;

    case TBL_DOT1X_PORT_FLD_CURRENT_USER_COUNT_DB:
        p_db_dot1x_port->current_user_count_db = p_dot1x_port->current_user_count_db;
        break;

    case TBL_DOT1X_PORT_FLD_MAX_USER:
        p_db_dot1x_port->max_user = p_dot1x_port->max_user;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_MODE:
        p_db_dot1x_port->auth_mode = p_dot1x_port->auth_mode;
        break;

    case TBL_DOT1X_PORT_FLD_MAC_AUTH_BYPASS:
        p_db_dot1x_port->mac_auth_bypass = p_dot1x_port->mac_auth_bypass;
        break;

    case TBL_DOT1X_PORT_FLD_SHARE_REAUTH:
        p_db_dot1x_port->share_reauth = p_dot1x_port->share_reauth;
        break;

    case TBL_DOT1X_PORT_FLD_AUTHWHILE:
        p_db_dot1x_port->authWhile = p_dot1x_port->authWhile;
        break;

    case TBL_DOT1X_PORT_FLD_AWHILE:
        p_db_dot1x_port->aWhile = p_dot1x_port->aWhile;
        break;

    case TBL_DOT1X_PORT_FLD_QUIETWHILE:
        p_db_dot1x_port->quietWhile = p_dot1x_port->quietWhile;
        break;

    case TBL_DOT1X_PORT_FLD_REAUTHWHEN:
        p_db_dot1x_port->reAuthWhen = p_dot1x_port->reAuthWhen;
        break;

    case TBL_DOT1X_PORT_FLD_TXWHEN:
        p_db_dot1x_port->txWhen = p_dot1x_port->txWhen;
        break;

    case TBL_DOT1X_PORT_FLD_AUTHABORT:
        p_db_dot1x_port->authAbort = p_dot1x_port->authAbort;
        break;

    case TBL_DOT1X_PORT_FLD_AUTHFAIL:
        p_db_dot1x_port->authFail = p_dot1x_port->authFail;
        break;

    case TBL_DOT1X_PORT_FLD_AUTHSTART:
        p_db_dot1x_port->authStart = p_dot1x_port->authStart;
        break;

    case TBL_DOT1X_PORT_FLD_AUTHTIMEOUT:
        p_db_dot1x_port->authTimeout = p_dot1x_port->authTimeout;
        break;

    case TBL_DOT1X_PORT_FLD_AUTHSUCCESS:
        p_db_dot1x_port->authSuccess = p_dot1x_port->authSuccess;
        break;

    case TBL_DOT1X_PORT_FLD_INITIALIZE:
        p_db_dot1x_port->initialize = p_dot1x_port->initialize;
        break;

    case TBL_DOT1X_PORT_FLD_PORTENABLED:
        p_db_dot1x_port->portEnabled = p_dot1x_port->portEnabled;
        break;

    case TBL_DOT1X_PORT_FLD_REAUTHENTICATE:
        p_db_dot1x_port->reAuthenticate = p_dot1x_port->reAuthenticate;
        break;

    case TBL_DOT1X_PORT_FLD_EAPNOREQ:
        p_db_dot1x_port->eapNoReq = p_dot1x_port->eapNoReq;
        break;

    case TBL_DOT1X_PORT_FLD_EAPRESTART:
        p_db_dot1x_port->eaprestart = p_dot1x_port->eaprestart;
        break;

    case TBL_DOT1X_PORT_FLD_EAPRESP:
        p_db_dot1x_port->eapResp = p_dot1x_port->eapResp;
        break;

    case TBL_DOT1X_PORT_FLD_EAPREQ:
        p_db_dot1x_port->eapReq = p_dot1x_port->eapReq;
        break;

    case TBL_DOT1X_PORT_FLD_EAPSUCCESS:
        p_db_dot1x_port->eapSuccess = p_dot1x_port->eapSuccess;
        break;

    case TBL_DOT1X_PORT_FLD_EAPFAIL:
        p_db_dot1x_port->eapFail = p_dot1x_port->eapFail;
        break;

    case TBL_DOT1X_PORT_FLD_RETRANSMIT:
        p_db_dot1x_port->retransmit = p_dot1x_port->retransmit;
        break;

    case TBL_DOT1X_PORT_FLD_CURRENTID:
        p_db_dot1x_port->currentId = p_dot1x_port->currentId;
        break;

    case TBL_DOT1X_PORT_FLD_PORTCONTROL:
        p_db_dot1x_port->portControl = p_dot1x_port->portControl;
        break;

    case TBL_DOT1X_PORT_FLD_PORTSTATUS:
        p_db_dot1x_port->portStatus = p_dot1x_port->portStatus;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF:
        p_db_dot1x_port->auth_pae_eapLogoff = p_dot1x_port->auth_pae_eapLogoff;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART:
        p_db_dot1x_port->auth_pae_eapStart = p_dot1x_port->auth_pae_eapStart;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_PORTMODE:
        p_db_dot1x_port->auth_pae_portMode = p_dot1x_port->auth_pae_portMode;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHCOUNT:
        p_db_dot1x_port->auth_pae_reAuthCount = p_dot1x_port->auth_pae_reAuthCount;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_RXRESPID:
        p_db_dot1x_port->auth_pae_rxRespId = p_dot1x_port->auth_pae_rxRespId;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_STATE:
        p_db_dot1x_port->auth_pae_state = p_dot1x_port->auth_pae_state;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_QUIETPERIOD:
        p_db_dot1x_port->auth_pae_quietPeriod = p_dot1x_port->auth_pae_quietPeriod;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHMAX:
        p_db_dot1x_port->auth_pae_reAuthMax = p_dot1x_port->auth_pae_reAuthMax;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_TXPERIOD:
        p_db_dot1x_port->auth_pae_txPeriod = p_dot1x_port->auth_pae_txPeriod;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_ENTER_CONNECTING:
        p_db_dot1x_port->auth_pae_enter_connecting = p_dot1x_port->auth_pae_enter_connecting;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_CONNECTING:
        p_db_dot1x_port->auth_pae_eaplogoff_while_connecting = p_dot1x_port->auth_pae_eaplogoff_while_connecting;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_ENTER_AUTHENTICATING:
        p_db_dot1x_port->auth_pae_enter_authenticating = p_dot1x_port->auth_pae_enter_authenticating;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_SUCCESS_WHILE_AUTHENTICATING:
        p_db_dot1x_port->auth_pae_success_while_authenticating = p_dot1x_port->auth_pae_success_while_authenticating;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_TIMEOUT_WHILE_AUTHENTICATING:
        p_db_dot1x_port->auth_pae_timeout_while_authenticating = p_dot1x_port->auth_pae_timeout_while_authenticating;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_FAIL_WHILE_AUTHENTICATING:
        p_db_dot1x_port->auth_pae_fail_while_authenticating = p_dot1x_port->auth_pae_fail_while_authenticating;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART_WHILE_AUTHENTICATING:
        p_db_dot1x_port->auth_pae_eapstart_while_authenticating = p_dot1x_port->auth_pae_eapstart_while_authenticating;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_AUTHENTICATING:
        p_db_dot1x_port->auth_pae_eaplogoff_while_authenticating = p_dot1x_port->auth_pae_eaplogoff_while_authenticating;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHS_WHILE_AUTHENTICATED:
        p_db_dot1x_port->auth_pae_reauths_while_authenticated = p_dot1x_port->auth_pae_reauths_while_authenticated;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART_WHILE_AUTHENTICATED:
        p_db_dot1x_port->auth_pae_eapstart_while_authenticated = p_dot1x_port->auth_pae_eapstart_while_authenticated;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_AUTHENTICATED:
        p_db_dot1x_port->auth_pae_eaplogoff_while_authenticated = p_dot1x_port->auth_pae_eaplogoff_while_authenticated;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHS_WHILE_AUTHENTICATING:
        p_db_dot1x_port->auth_pae_reauths_while_authenticating = p_dot1x_port->auth_pae_reauths_while_authenticating;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYAVAILABLE:
        p_db_dot1x_port->auth_key_xmit_keyAvailable = p_dot1x_port->auth_key_xmit_keyAvailable;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYTXENABLED:
        p_db_dot1x_port->auth_key_xmit_keyTxEnabled = p_dot1x_port->auth_key_xmit_keyTxEnabled;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYRUN:
        p_db_dot1x_port->auth_key_xmit_KeyRun = p_dot1x_port->auth_key_xmit_KeyRun;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYDONE:
        p_db_dot1x_port->auth_key_xmit_KeyDone = p_dot1x_port->auth_key_xmit_KeyDone;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_STATE:
        p_db_dot1x_port->auth_key_xmit_state = p_dot1x_port->auth_key_xmit_state;
        break;

    case TBL_DOT1X_PORT_FLD_REAUTHPERIOD:
        p_db_dot1x_port->reAuthPeriod = p_dot1x_port->reAuthPeriod;
        break;

    case TBL_DOT1X_PORT_FLD_REAUTHENABLED:
        p_db_dot1x_port->reAuthEnabled = p_dot1x_port->reAuthEnabled;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_BE_STATE:
        p_db_dot1x_port->auth_be_state = p_dot1x_port->auth_be_state;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_BE_REQCOUNT:
        p_db_dot1x_port->auth_be_reqCount = p_dot1x_port->auth_be_reqCount;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_BE_IDFROMSERVER:
        p_db_dot1x_port->auth_be_idFromServer = p_dot1x_port->auth_be_idFromServer;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_BE_SUPPTIMEOUT:
        p_db_dot1x_port->auth_be_suppTimeout = p_dot1x_port->auth_be_suppTimeout;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_BE_SERVERTIMEOUT:
        p_db_dot1x_port->auth_be_serverTimeout = p_dot1x_port->auth_be_serverTimeout;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_BE_MAXREQ:
        p_db_dot1x_port->auth_be_maxReq = p_dot1x_port->auth_be_maxReq;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_RESPONSES:
        p_db_dot1x_port->auth_be_backend_responses = p_dot1x_port->auth_be_backend_responses;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_ACCESS_CHALLENGES:
        p_db_dot1x_port->auth_be_backend_access_challenges = p_dot1x_port->auth_be_backend_access_challenges;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_OTHERREQUEST_TO_SUPPLICANT:
        p_db_dot1x_port->auth_be_backend_otherrequest_to_supplicant = p_dot1x_port->auth_be_backend_otherrequest_to_supplicant;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_AUTH_SUCCESS:
        p_db_dot1x_port->auth_be_backend_auth_success = p_dot1x_port->auth_be_backend_auth_success;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_AUTH_FAILS:
        p_db_dot1x_port->auth_be_backend_auth_fails = p_dot1x_port->auth_be_backend_auth_fails;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_NONNAK_RESPONSES_FROM_SUPPLICANT:
        p_db_dot1x_port->auth_be_backend_nonnak_responses_from_supplicant = p_dot1x_port->auth_be_backend_nonnak_responses_from_supplicant;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_STATE:
        p_db_dot1x_port->auth_ctrl_dir_state = p_dot1x_port->auth_ctrl_dir_state;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_ADMINCONTROLLEDDIRECTIONS:
        p_db_dot1x_port->auth_ctrl_dir_adminControlledDirections = p_dot1x_port->auth_ctrl_dir_adminControlledDirections;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_OPERCONTROLLEDDIRECTIONS:
        p_db_dot1x_port->auth_ctrl_dir_operControlledDirections = p_dot1x_port->auth_ctrl_dir_operControlledDirections;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_BRIDGEDETECTED:
        p_db_dot1x_port->auth_ctrl_dir_bridgeDetected = p_dot1x_port->auth_ctrl_dir_bridgeDetected;
        break;

    case TBL_DOT1X_PORT_FLD_AUTH_KEY_RECV_RXKEY:
        p_db_dot1x_port->auth_key_recv_rxKey = p_dot1x_port->auth_key_recv_rxKey;
        break;

    case TBL_DOT1X_PORT_FLD_EAPOL_FRAMES_RX:
        p_db_dot1x_port->eapol_frames_rx = p_dot1x_port->eapol_frames_rx;
        break;

    case TBL_DOT1X_PORT_FLD_EAPOL_FRAMES_TX:
        p_db_dot1x_port->eapol_frames_tx = p_dot1x_port->eapol_frames_tx;
        break;

    case TBL_DOT1X_PORT_FLD_EAPOL_START_FRAMES_RX:
        p_db_dot1x_port->eapol_start_frames_rx = p_dot1x_port->eapol_start_frames_rx;
        break;

    case TBL_DOT1X_PORT_FLD_EAPOL_LOGOFF_FRAMES_RX:
        p_db_dot1x_port->eapol_logoff_frames_rx = p_dot1x_port->eapol_logoff_frames_rx;
        break;

    case TBL_DOT1X_PORT_FLD_EAP_RESPID_FRAMES_RX:
        p_db_dot1x_port->eap_respid_frames_rx = p_dot1x_port->eap_respid_frames_rx;
        break;

    case TBL_DOT1X_PORT_FLD_EAP_RESP_FRAMES_RX:
        p_db_dot1x_port->eap_resp_frames_rx = p_dot1x_port->eap_resp_frames_rx;
        break;

    case TBL_DOT1X_PORT_FLD_EAP_REQID_FRAMES_TX:
        p_db_dot1x_port->eap_reqid_frames_tx = p_dot1x_port->eap_reqid_frames_tx;
        break;

    case TBL_DOT1X_PORT_FLD_EAP_REQ_FRAMES_TX:
        p_db_dot1x_port->eap_req_frames_tx = p_dot1x_port->eap_req_frames_tx;
        break;

    case TBL_DOT1X_PORT_FLD_EAPOL_INVALID_FRAMES_RX:
        p_db_dot1x_port->eapol_invalid_frames_rx = p_dot1x_port->eapol_invalid_frames_rx;
        break;

    case TBL_DOT1X_PORT_FLD_EAP_LEN_ERROR_FRAMES_RX:
        p_db_dot1x_port->eap_len_error_frames_rx = p_dot1x_port->eap_len_error_frames_rx;
        break;

    case TBL_DOT1X_PORT_FLD_EAPOL_LAST_FRAME_VERSION:
        p_db_dot1x_port->eapol_last_frame_version = p_dot1x_port->eapol_last_frame_version;
        break;

    case TBL_DOT1X_PORT_FLD_EAPOL_LAST_FRAME_SOURCE:
        sal_memcpy(p_db_dot1x_port->eapol_last_frame_source, p_dot1x_port->eapol_last_frame_source, sizeof(p_dot1x_port->eapol_last_frame_source));
        break;

    case TBL_DOT1X_PORT_FLD_SESSION_TIME:
        p_db_dot1x_port->session_time = p_dot1x_port->session_time;
        break;

    case TBL_DOT1X_PORT_FLD_SESSION_AUTHENTICATION_METHOD:
        p_db_dot1x_port->session_authentication_method = p_dot1x_port->session_authentication_method;
        break;

    case TBL_DOT1X_PORT_FLD_SESSION_TERMINATE_CAUSE:
        p_db_dot1x_port->session_terminate_cause = p_dot1x_port->session_terminate_cause;
        break;

    case TBL_DOT1X_PORT_FLD_SESSION_USER_NAME:
        sal_memcpy(p_db_dot1x_port->session_user_name, p_dot1x_port->session_user_name, sizeof(p_dot1x_port->session_user_name));
        break;

    case TBL_DOT1X_PORT_FLD_CLASS_ID:
        p_db_dot1x_port->class_id = p_dot1x_port->class_id;
        break;

    case TBL_DOT1X_PORT_FLD_MAX:
        sal_memcpy(p_db_dot1x_port, p_dot1x_port, sizeof(tbl_dot1x_port_t));
        break;
    default:
        return PM_E_NOT_SUPPORT;
    }

    /* 3. sync to CDB */
    if (sync)
    {
        cdb_sync_tbl_set(TBL_DOT1X_PORT, field_id, p_db_dot1x_port);
        #ifdef LDB_SYNC_TO_RDB 
        cdb_sync_publish_tbl_set(TBL_DOT1X_PORT, field_id, p_db_dot1x_port);
        #endif 
    }

    return PM_E_NONE;
}

int32
tbl_dot1x_port_add_dot1x_port(tbl_dot1x_port_t *p_dot1x_port)
{
    return tbl_dot1x_port_add_dot1x_port_sync(p_dot1x_port, TRUE);
}

int32
tbl_dot1x_port_del_dot1x_port(tbl_dot1x_port_key_t *p_dot1x_port_key)
{
    return tbl_dot1x_port_del_dot1x_port_sync(p_dot1x_port_key, TRUE);
}

int32
tbl_dot1x_port_set_dot1x_port_field(tbl_dot1x_port_t *p_dot1x_port, tbl_dot1x_port_field_id_t field_id)
{
    return tbl_dot1x_port_set_dot1x_port_field_sync(p_dot1x_port, field_id, TRUE);
}

tbl_dot1x_port_t*
tbl_dot1x_port_get_dot1x_port(tbl_dot1x_port_key_t *p_dot1x_port_key)
{
    tbl_dot1x_port_master_t *p_master = _g_p_tbl_dot1x_port_master;
    tbl_dot1x_port_t lkp;

    sal_memcpy(&lkp.key, p_dot1x_port_key, sizeof(tbl_dot1x_port_key_t));
    return ctclib_hash_lookup(p_master->dot1x_port_hash, &lkp);
}

char*
tbl_dot1x_port_key_val2str(tbl_dot1x_port_t *p_dot1x_port, char *str, uint32 str_len)
{
    sal_memset(str, 0, str_len);
    sal_snprintf(str, str_len, "%u", p_dot1x_port->key.ifindex);
    return str;
}

int32
tbl_dot1x_port_key_str2val(char *str, tbl_dot1x_port_t *p_dot1x_port)
{
    int32 ret = 0;

    p_dot1x_port->key.ifindex = cdb_uint_str2val(str, &ret);
    return ret;
}

char*
tbl_dot1x_port_key_name_dump(tbl_dot1x_port_t *p_dot1x_port, char *str)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DOT1X_PORT);
    sal_sprintf(str, "%s"KEY_CONNECT_STR
          "%u", p_node->name, p_dot1x_port->key.ifindex);
    return str;
}

char*
tbl_dot1x_port_key_value_dump(tbl_dot1x_port_t *p_dot1x_port, char *str)
{
    sal_sprintf(str, "%u", p_dot1x_port->key.ifindex);
    return str;
}

char*
tbl_dot1x_port_field_name_dump(tbl_dot1x_port_t *p_dot1x_port, int32 field_id, char *str)
{

    cdb_node_t *p_node = cdb_get_tbl(TBL_DOT1X_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;
    if (field_id >= TBL_DOT1X_PORT_FLD_MAX)
    {
        return NULL;
    }

    sal_sprintf(str, "%s", p_tbl_info->field[field_id].name);

    return str;
}

char*
tbl_dot1x_port_field_value_dump(tbl_dot1x_port_t *p_dot1x_port, int32 field_id, char* str)
{
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_KEY, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->key.ifindex);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_CURRENT_RADIUS, field_id))
    {
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_RADIUS_MSG_ID, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->radius_msg_id);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_RETRY, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->retry);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_T_WAIT, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_dot1x_port->t_wait);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SESSIONTYPE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->sessiontype);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_NAME, field_id))
    {
        sal_sprintf(str, "%s", p_dot1x_port->name);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_BUF, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_dot1x_port->buf, sizeof(p_dot1x_port->buf));
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_DEST_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_dot1x_port->dest_mac);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SRC_MAC, field_id))
    {
        cdb_mac_addr_val2str(str, MAX_CMD_STR_LEN, p_dot1x_port->src_mac);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_USER_NAME, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_dot1x_port->user_name, sizeof(p_dot1x_port->user_name));
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHCONTROLLEDPORTCONTROL, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->authControlledPortControl);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_T_TIMER, field_id))
    {
        ctc_task_val2str(str, MAX_CMD_STR_LEN, p_dot1x_port->t_timer);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_PROTOCOL_VERSION, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->protocol_version);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_GUEST_VLAN_VALID, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->guest_vlan_valid);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_GUEST_VLAN, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->guest_vlan);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_CURRENT_USER, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->current_user);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_CURRENT_USER_COUNT_DB, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->current_user_count_db);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_MAX_USER, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->max_user);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_MODE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_mode);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_MAC_AUTH_BYPASS, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->mac_auth_bypass);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SHARE_REAUTH, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->share_reauth);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHWHILE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->authWhile);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AWHILE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->aWhile);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_QUIETWHILE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->quietWhile);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_REAUTHWHEN, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->reAuthWhen);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_TXWHEN, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->txWhen);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHABORT, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->authAbort);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHFAIL, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->authFail);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHSTART, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->authStart);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHTIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->authTimeout);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHSUCCESS, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->authSuccess);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_INITIALIZE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->initialize);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_PORTENABLED, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->portEnabled);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_REAUTHENTICATE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->reAuthenticate);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPNOREQ, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eapNoReq);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPRESTART, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eaprestart);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPRESP, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eapResp);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPREQ, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eapReq);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPSUCCESS, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eapSuccess);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPFAIL, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eapFail);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_RETRANSMIT, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->retransmit);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_CURRENTID, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->currentId);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_PORTCONTROL, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->portControl);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_PORTSTATUS, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->portStatus);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_eapLogoff);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_eapStart);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_PORTMODE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_portMode);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHCOUNT, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_reAuthCount);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_RXRESPID, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_rxRespId);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_state);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_QUIETPERIOD, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_quietPeriod);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHMAX, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_reAuthMax);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_TXPERIOD, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_txPeriod);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_ENTER_CONNECTING, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_enter_connecting);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_CONNECTING, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_eaplogoff_while_connecting);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_ENTER_AUTHENTICATING, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_enter_authenticating);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_SUCCESS_WHILE_AUTHENTICATING, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_success_while_authenticating);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_TIMEOUT_WHILE_AUTHENTICATING, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_timeout_while_authenticating);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_FAIL_WHILE_AUTHENTICATING, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_fail_while_authenticating);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART_WHILE_AUTHENTICATING, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_eapstart_while_authenticating);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_AUTHENTICATING, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_eaplogoff_while_authenticating);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHS_WHILE_AUTHENTICATED, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_reauths_while_authenticated);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART_WHILE_AUTHENTICATED, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_eapstart_while_authenticated);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_AUTHENTICATED, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_eaplogoff_while_authenticated);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHS_WHILE_AUTHENTICATING, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_pae_reauths_while_authenticating);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYAVAILABLE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_key_xmit_keyAvailable);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYTXENABLED, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_key_xmit_keyTxEnabled);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYRUN, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_key_xmit_KeyRun);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYDONE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_key_xmit_KeyDone);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_key_xmit_state);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_REAUTHPERIOD, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->reAuthPeriod);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_REAUTHENABLED, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->reAuthEnabled);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_be_state);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_REQCOUNT, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_be_reqCount);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_IDFROMSERVER, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_be_idFromServer);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_SUPPTIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_be_suppTimeout);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_SERVERTIMEOUT, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_be_serverTimeout);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_MAXREQ, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_be_maxReq);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_RESPONSES, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_be_backend_responses);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_ACCESS_CHALLENGES, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_be_backend_access_challenges);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_OTHERREQUEST_TO_SUPPLICANT, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_be_backend_otherrequest_to_supplicant);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_AUTH_SUCCESS, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_be_backend_auth_success);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_AUTH_FAILS, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_be_backend_auth_fails);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_NONNAK_RESPONSES_FROM_SUPPLICANT, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_be_backend_nonnak_responses_from_supplicant);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_STATE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_ctrl_dir_state);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_ADMINCONTROLLEDDIRECTIONS, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_ctrl_dir_adminControlledDirections);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_OPERCONTROLLEDDIRECTIONS, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_ctrl_dir_operControlledDirections);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_BRIDGEDETECTED, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_ctrl_dir_bridgeDetected);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_RECV_RXKEY, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->auth_key_recv_rxKey);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_FRAMES_RX, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eapol_frames_rx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_FRAMES_TX, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eapol_frames_tx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_START_FRAMES_RX, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eapol_start_frames_rx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_LOGOFF_FRAMES_RX, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eapol_logoff_frames_rx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAP_RESPID_FRAMES_RX, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eap_respid_frames_rx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAP_RESP_FRAMES_RX, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eap_resp_frames_rx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAP_REQID_FRAMES_TX, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eap_reqid_frames_tx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAP_REQ_FRAMES_TX, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eap_req_frames_tx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_INVALID_FRAMES_RX, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eapol_invalid_frames_rx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAP_LEN_ERROR_FRAMES_RX, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eap_len_error_frames_rx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_LAST_FRAME_VERSION, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->eapol_last_frame_version);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_LAST_FRAME_SOURCE, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_dot1x_port->eapol_last_frame_source, sizeof(p_dot1x_port->eapol_last_frame_source));
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SESSION_TIME, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->session_time);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SESSION_AUTHENTICATION_METHOD, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->session_authentication_method);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SESSION_TERMINATE_CAUSE, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->session_terminate_cause);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SESSION_USER_NAME, field_id))
    {
        cdb_uint8_array_val2str(str, MAX_CMD_STR_LEN, p_dot1x_port->session_user_name, sizeof(p_dot1x_port->session_user_name));
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_CLASS_ID, field_id))
    {
        sal_sprintf(str, "%u", p_dot1x_port->class_id);
    }
    return str;
}

char**
tbl_dot1x_port_table_dump(tbl_dot1x_port_t *p_dot1x_port, char **str)
{
    char buf[MAX_CMD_STR_LEN];
    int i = 0, j= 0;

    sal_sprintf(str[j++], "%s",
         tbl_dot1x_port_key_name_dump(p_dot1x_port, buf));
    for(i=1; i<TBL_DOT1X_PORT_FLD_MAX; i++)
    {
        sal_sprintf(str[j++], "%s",
            tbl_dot1x_port_field_name_dump(p_dot1x_port, i, buf));
        sal_sprintf(str[j++], "%s",
            tbl_dot1x_port_field_value_dump(p_dot1x_port, i, buf));
    }
    return str;
}

int32
tbl_dot1x_port_field_value_parser(char *str, int32 field_id, tbl_dot1x_port_t *p_dot1x_port)
{
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_KEY, field_id))
    {
        int32 ret;
        p_dot1x_port->key.ifindex = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_CURRENT_RADIUS, field_id))
    {
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_RADIUS_MSG_ID, field_id))
    {
        int32 ret;
        p_dot1x_port->radius_msg_id = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_RETRY, field_id))
    {
        int32 ret;
        p_dot1x_port->retry = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_T_WAIT, field_id))
    {
        ctc_task_str2val(str, p_dot1x_port->t_wait);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SESSIONTYPE, field_id))
    {
        int32 ret;
        p_dot1x_port->sessiontype = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_NAME, field_id))
    {
        sal_strcpy(p_dot1x_port->name, str);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_BUF, field_id))
    {
        cdb_uint8_array_str2val(str, p_dot1x_port->buf);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_DEST_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_dot1x_port->dest_mac, str);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SRC_MAC, field_id))
    {
        cdb_mac_addr_str2val(p_dot1x_port->src_mac, str);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_USER_NAME, field_id))
    {
        cdb_uint8_array_str2val(str, p_dot1x_port->user_name);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHCONTROLLEDPORTCONTROL, field_id))
    {
        int32 ret;
        p_dot1x_port->authControlledPortControl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_T_TIMER, field_id))
    {
        ctc_task_str2val(str, p_dot1x_port->t_timer);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_PROTOCOL_VERSION, field_id))
    {
        int32 ret;
        p_dot1x_port->protocol_version = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_GUEST_VLAN_VALID, field_id))
    {
        int32 ret;
        p_dot1x_port->guest_vlan_valid = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_GUEST_VLAN, field_id))
    {
        int32 ret;
        p_dot1x_port->guest_vlan = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_CURRENT_USER, field_id))
    {
        int32 ret;
        p_dot1x_port->current_user = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_CURRENT_USER_COUNT_DB, field_id))
    {
        int32 ret;
        p_dot1x_port->current_user_count_db = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_MAX_USER, field_id))
    {
        int32 ret;
        p_dot1x_port->max_user = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_MODE, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_mode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_MAC_AUTH_BYPASS, field_id))
    {
        int32 ret;
        p_dot1x_port->mac_auth_bypass = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SHARE_REAUTH, field_id))
    {
        int32 ret;
        p_dot1x_port->share_reauth = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHWHILE, field_id))
    {
        int32 ret;
        p_dot1x_port->authWhile = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AWHILE, field_id))
    {
        int32 ret;
        p_dot1x_port->aWhile = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_QUIETWHILE, field_id))
    {
        int32 ret;
        p_dot1x_port->quietWhile = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_REAUTHWHEN, field_id))
    {
        int32 ret;
        p_dot1x_port->reAuthWhen = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_TXWHEN, field_id))
    {
        int32 ret;
        p_dot1x_port->txWhen = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHABORT, field_id))
    {
        int32 ret;
        p_dot1x_port->authAbort = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHFAIL, field_id))
    {
        int32 ret;
        p_dot1x_port->authFail = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHSTART, field_id))
    {
        int32 ret;
        p_dot1x_port->authStart = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHTIMEOUT, field_id))
    {
        int32 ret;
        p_dot1x_port->authTimeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHSUCCESS, field_id))
    {
        int32 ret;
        p_dot1x_port->authSuccess = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_INITIALIZE, field_id))
    {
        int32 ret;
        p_dot1x_port->initialize = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_PORTENABLED, field_id))
    {
        int32 ret;
        p_dot1x_port->portEnabled = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_REAUTHENTICATE, field_id))
    {
        int32 ret;
        p_dot1x_port->reAuthenticate = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPNOREQ, field_id))
    {
        int32 ret;
        p_dot1x_port->eapNoReq = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPRESTART, field_id))
    {
        int32 ret;
        p_dot1x_port->eaprestart = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPRESP, field_id))
    {
        int32 ret;
        p_dot1x_port->eapResp = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPREQ, field_id))
    {
        int32 ret;
        p_dot1x_port->eapReq = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPSUCCESS, field_id))
    {
        int32 ret;
        p_dot1x_port->eapSuccess = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPFAIL, field_id))
    {
        int32 ret;
        p_dot1x_port->eapFail = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_RETRANSMIT, field_id))
    {
        int32 ret;
        p_dot1x_port->retransmit = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_CURRENTID, field_id))
    {
        int32 ret;
        p_dot1x_port->currentId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_PORTCONTROL, field_id))
    {
        int32 ret;
        p_dot1x_port->portControl = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_PORTSTATUS, field_id))
    {
        int32 ret;
        p_dot1x_port->portStatus = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_eapLogoff = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_eapStart = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_PORTMODE, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_portMode = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHCOUNT, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_reAuthCount = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_RXRESPID, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_rxRespId = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_STATE, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_QUIETPERIOD, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_quietPeriod = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHMAX, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_reAuthMax = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_TXPERIOD, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_txPeriod = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_ENTER_CONNECTING, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_enter_connecting = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_CONNECTING, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_eaplogoff_while_connecting = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_ENTER_AUTHENTICATING, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_enter_authenticating = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_SUCCESS_WHILE_AUTHENTICATING, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_success_while_authenticating = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_TIMEOUT_WHILE_AUTHENTICATING, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_timeout_while_authenticating = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_FAIL_WHILE_AUTHENTICATING, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_fail_while_authenticating = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART_WHILE_AUTHENTICATING, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_eapstart_while_authenticating = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_AUTHENTICATING, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_eaplogoff_while_authenticating = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHS_WHILE_AUTHENTICATED, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_reauths_while_authenticated = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART_WHILE_AUTHENTICATED, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_eapstart_while_authenticated = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_AUTHENTICATED, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_eaplogoff_while_authenticated = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHS_WHILE_AUTHENTICATING, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_pae_reauths_while_authenticating = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYAVAILABLE, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_key_xmit_keyAvailable = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYTXENABLED, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_key_xmit_keyTxEnabled = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYRUN, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_key_xmit_KeyRun = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYDONE, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_key_xmit_KeyDone = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_STATE, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_key_xmit_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_REAUTHPERIOD, field_id))
    {
        int32 ret;
        p_dot1x_port->reAuthPeriod = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_REAUTHENABLED, field_id))
    {
        int32 ret;
        p_dot1x_port->reAuthEnabled = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_STATE, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_be_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_REQCOUNT, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_be_reqCount = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_IDFROMSERVER, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_be_idFromServer = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_SUPPTIMEOUT, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_be_suppTimeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_SERVERTIMEOUT, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_be_serverTimeout = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_MAXREQ, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_be_maxReq = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_RESPONSES, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_be_backend_responses = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_ACCESS_CHALLENGES, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_be_backend_access_challenges = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_OTHERREQUEST_TO_SUPPLICANT, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_be_backend_otherrequest_to_supplicant = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_AUTH_SUCCESS, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_be_backend_auth_success = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_AUTH_FAILS, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_be_backend_auth_fails = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_NONNAK_RESPONSES_FROM_SUPPLICANT, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_be_backend_nonnak_responses_from_supplicant = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_STATE, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_ctrl_dir_state = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_ADMINCONTROLLEDDIRECTIONS, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_ctrl_dir_adminControlledDirections = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_OPERCONTROLLEDDIRECTIONS, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_ctrl_dir_operControlledDirections = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_BRIDGEDETECTED, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_ctrl_dir_bridgeDetected = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_RECV_RXKEY, field_id))
    {
        int32 ret;
        p_dot1x_port->auth_key_recv_rxKey = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_FRAMES_RX, field_id))
    {
        int32 ret;
        p_dot1x_port->eapol_frames_rx = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_FRAMES_TX, field_id))
    {
        int32 ret;
        p_dot1x_port->eapol_frames_tx = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_START_FRAMES_RX, field_id))
    {
        int32 ret;
        p_dot1x_port->eapol_start_frames_rx = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_LOGOFF_FRAMES_RX, field_id))
    {
        int32 ret;
        p_dot1x_port->eapol_logoff_frames_rx = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAP_RESPID_FRAMES_RX, field_id))
    {
        int32 ret;
        p_dot1x_port->eap_respid_frames_rx = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAP_RESP_FRAMES_RX, field_id))
    {
        int32 ret;
        p_dot1x_port->eap_resp_frames_rx = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAP_REQID_FRAMES_TX, field_id))
    {
        int32 ret;
        p_dot1x_port->eap_reqid_frames_tx = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAP_REQ_FRAMES_TX, field_id))
    {
        int32 ret;
        p_dot1x_port->eap_req_frames_tx = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_INVALID_FRAMES_RX, field_id))
    {
        int32 ret;
        p_dot1x_port->eapol_invalid_frames_rx = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAP_LEN_ERROR_FRAMES_RX, field_id))
    {
        int32 ret;
        p_dot1x_port->eap_len_error_frames_rx = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_LAST_FRAME_VERSION, field_id))
    {
        int32 ret;
        p_dot1x_port->eapol_last_frame_version = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_LAST_FRAME_SOURCE, field_id))
    {
        cdb_uint8_array_str2val(str, p_dot1x_port->eapol_last_frame_source);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SESSION_TIME, field_id))
    {
        int32 ret;
        p_dot1x_port->session_time = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SESSION_AUTHENTICATION_METHOD, field_id))
    {
        int32 ret;
        p_dot1x_port->session_authentication_method = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SESSION_TERMINATE_CAUSE, field_id))
    {
        int32 ret;
        p_dot1x_port->session_terminate_cause = cdb_uint_str2val(str, &ret);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SESSION_USER_NAME, field_id))
    {
        cdb_uint8_array_str2val(str, p_dot1x_port->session_user_name);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_CLASS_ID, field_id))
    {
        int32 ret;
        p_dot1x_port->class_id = cdb_uint_str2val(str, &ret);
    }
    return PM_E_NONE;
}

int32
tbl_dot1x_port_table_parser(char** array, char* key_value,tbl_dot1x_port_t *p_dot1x_port)
{
    int32 rc;
    int i = 0, j= 0;
    cdb_node_t *p_node = cdb_get_tbl(TBL_DOT1X_PORT);
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    PM_E_RETURN(tbl_dot1x_port_key_str2val(key_value, p_dot1x_port));

    for(i=1; i<TBL_DOT1X_PORT_FLD_MAX; i++)
    {
        if(sal_strncmp(array[j++], p_tbl_info->field[i].name, sal_strlen(p_tbl_info->field[i].name)))
        {
             return PM_E_INVALID_PARAM;
        }
        PM_E_RETURN(tbl_dot1x_port_field_value_parser( array[j++], i, p_dot1x_port));
    }

    return PM_E_NONE;
}

int32
tbl_dot1x_port_dump_one(tbl_dot1x_port_t *p_dot1x_port, tbl_iter_args_t *pargs)
{
    cdb_node_t *p_node = cdb_get_tbl(TBL_DOT1X_PORT);
    field_parse_t *p_field = pargs->argv[0];
    FILE *fp = pargs->argv[1];
    cdb_tbl_info_t *p_tbl_info = p_node->tbl_info;

    char buf[MAX_CMD_STR_LEN];

    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_KEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_KEY].name,
            p_dot1x_port->key.ifindex);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_CURRENT_RADIUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"",
            p_tbl_info->field[TBL_DOT1X_PORT_FLD_CURRENT_RADIUS].name);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_RADIUS_MSG_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_RADIUS_MSG_ID].name,
            p_dot1x_port->radius_msg_id);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_RETRY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_RETRY].name,
            p_dot1x_port->retry);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_T_WAIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_PORT_FLD_T_WAIT].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_port->t_wait));
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SESSIONTYPE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_SESSIONTYPE].name,
            p_dot1x_port->sessiontype);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_PORT_FLD_NAME].name,
            p_dot1x_port->name);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_BUF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_PORT_FLD_BUF].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_port->buf, sizeof(p_dot1x_port->buf)));
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_DEST_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_PORT_FLD_DEST_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_port->dest_mac));
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SRC_MAC, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_PORT_FLD_SRC_MAC].name, 
            cdb_mac_addr_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_port->src_mac));
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_USER_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_PORT_FLD_USER_NAME].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_port->user_name, sizeof(p_dot1x_port->user_name)));
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHCONTROLLEDPORTCONTROL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTHCONTROLLEDPORTCONTROL].name,
            p_dot1x_port->authControlledPortControl);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_T_TIMER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_PORT_FLD_T_TIMER].name, 
            ctc_task_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_port->t_timer));
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_PROTOCOL_VERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_PROTOCOL_VERSION].name,
            p_dot1x_port->protocol_version);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_GUEST_VLAN_VALID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_GUEST_VLAN_VALID].name,
            p_dot1x_port->guest_vlan_valid);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_GUEST_VLAN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_GUEST_VLAN].name,
            p_dot1x_port->guest_vlan);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_CURRENT_USER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_CURRENT_USER].name,
            p_dot1x_port->current_user);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_CURRENT_USER_COUNT_DB, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_CURRENT_USER_COUNT_DB].name,
            p_dot1x_port->current_user_count_db);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_MAX_USER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_MAX_USER].name,
            p_dot1x_port->max_user);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_MODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_MODE].name,
            p_dot1x_port->auth_mode);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_MAC_AUTH_BYPASS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_MAC_AUTH_BYPASS].name,
            p_dot1x_port->mac_auth_bypass);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SHARE_REAUTH, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_SHARE_REAUTH].name,
            p_dot1x_port->share_reauth);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHWHILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTHWHILE].name,
            p_dot1x_port->authWhile);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AWHILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AWHILE].name,
            p_dot1x_port->aWhile);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_QUIETWHILE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_QUIETWHILE].name,
            p_dot1x_port->quietWhile);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_REAUTHWHEN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_REAUTHWHEN].name,
            p_dot1x_port->reAuthWhen);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_TXWHEN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_TXWHEN].name,
            p_dot1x_port->txWhen);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHABORT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTHABORT].name,
            p_dot1x_port->authAbort);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHFAIL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTHFAIL].name,
            p_dot1x_port->authFail);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHSTART, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTHSTART].name,
            p_dot1x_port->authStart);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHTIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTHTIMEOUT].name,
            p_dot1x_port->authTimeout);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTHSUCCESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTHSUCCESS].name,
            p_dot1x_port->authSuccess);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_INITIALIZE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_INITIALIZE].name,
            p_dot1x_port->initialize);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_PORTENABLED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_PORTENABLED].name,
            p_dot1x_port->portEnabled);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_REAUTHENTICATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_REAUTHENTICATE].name,
            p_dot1x_port->reAuthenticate);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPNOREQ, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAPNOREQ].name,
            p_dot1x_port->eapNoReq);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPRESTART, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAPRESTART].name,
            p_dot1x_port->eaprestart);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPRESP, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAPRESP].name,
            p_dot1x_port->eapResp);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPREQ, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAPREQ].name,
            p_dot1x_port->eapReq);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPSUCCESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAPSUCCESS].name,
            p_dot1x_port->eapSuccess);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPFAIL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAPFAIL].name,
            p_dot1x_port->eapFail);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_RETRANSMIT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_RETRANSMIT].name,
            p_dot1x_port->retransmit);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_CURRENTID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_CURRENTID].name,
            p_dot1x_port->currentId);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_PORTCONTROL, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_PORTCONTROL].name,
            p_dot1x_port->portControl);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_PORTSTATUS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_PORTSTATUS].name,
            p_dot1x_port->portStatus);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF].name,
            p_dot1x_port->auth_pae_eapLogoff);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART].name,
            p_dot1x_port->auth_pae_eapStart);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_PORTMODE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_PORTMODE].name,
            p_dot1x_port->auth_pae_portMode);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHCOUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHCOUNT].name,
            p_dot1x_port->auth_pae_reAuthCount);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_RXRESPID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_RXRESPID].name,
            p_dot1x_port->auth_pae_rxRespId);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_STATE].name,
            p_dot1x_port->auth_pae_state);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_QUIETPERIOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_QUIETPERIOD].name,
            p_dot1x_port->auth_pae_quietPeriod);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHMAX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHMAX].name,
            p_dot1x_port->auth_pae_reAuthMax);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_TXPERIOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_TXPERIOD].name,
            p_dot1x_port->auth_pae_txPeriod);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_ENTER_CONNECTING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_ENTER_CONNECTING].name,
            p_dot1x_port->auth_pae_enter_connecting);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_CONNECTING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_CONNECTING].name,
            p_dot1x_port->auth_pae_eaplogoff_while_connecting);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_ENTER_AUTHENTICATING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_ENTER_AUTHENTICATING].name,
            p_dot1x_port->auth_pae_enter_authenticating);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_SUCCESS_WHILE_AUTHENTICATING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_SUCCESS_WHILE_AUTHENTICATING].name,
            p_dot1x_port->auth_pae_success_while_authenticating);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_TIMEOUT_WHILE_AUTHENTICATING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_TIMEOUT_WHILE_AUTHENTICATING].name,
            p_dot1x_port->auth_pae_timeout_while_authenticating);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_FAIL_WHILE_AUTHENTICATING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_FAIL_WHILE_AUTHENTICATING].name,
            p_dot1x_port->auth_pae_fail_while_authenticating);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART_WHILE_AUTHENTICATING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART_WHILE_AUTHENTICATING].name,
            p_dot1x_port->auth_pae_eapstart_while_authenticating);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_AUTHENTICATING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_AUTHENTICATING].name,
            p_dot1x_port->auth_pae_eaplogoff_while_authenticating);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHS_WHILE_AUTHENTICATED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHS_WHILE_AUTHENTICATED].name,
            p_dot1x_port->auth_pae_reauths_while_authenticated);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART_WHILE_AUTHENTICATED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPSTART_WHILE_AUTHENTICATED].name,
            p_dot1x_port->auth_pae_eapstart_while_authenticated);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_AUTHENTICATED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_EAPLOGOFF_WHILE_AUTHENTICATED].name,
            p_dot1x_port->auth_pae_eaplogoff_while_authenticated);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHS_WHILE_AUTHENTICATING, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_PAE_REAUTHS_WHILE_AUTHENTICATING].name,
            p_dot1x_port->auth_pae_reauths_while_authenticating);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYAVAILABLE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYAVAILABLE].name,
            p_dot1x_port->auth_key_xmit_keyAvailable);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYTXENABLED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYTXENABLED].name,
            p_dot1x_port->auth_key_xmit_keyTxEnabled);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYRUN, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYRUN].name,
            p_dot1x_port->auth_key_xmit_KeyRun);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYDONE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_KEYDONE].name,
            p_dot1x_port->auth_key_xmit_KeyDone);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_KEY_XMIT_STATE].name,
            p_dot1x_port->auth_key_xmit_state);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_REAUTHPERIOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_REAUTHPERIOD].name,
            p_dot1x_port->reAuthPeriod);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_REAUTHENABLED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_REAUTHENABLED].name,
            p_dot1x_port->reAuthEnabled);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_BE_STATE].name,
            p_dot1x_port->auth_be_state);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_REQCOUNT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_BE_REQCOUNT].name,
            p_dot1x_port->auth_be_reqCount);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_IDFROMSERVER, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_BE_IDFROMSERVER].name,
            p_dot1x_port->auth_be_idFromServer);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_SUPPTIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_BE_SUPPTIMEOUT].name,
            p_dot1x_port->auth_be_suppTimeout);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_SERVERTIMEOUT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_BE_SERVERTIMEOUT].name,
            p_dot1x_port->auth_be_serverTimeout);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_MAXREQ, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_BE_MAXREQ].name,
            p_dot1x_port->auth_be_maxReq);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_RESPONSES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_RESPONSES].name,
            p_dot1x_port->auth_be_backend_responses);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_ACCESS_CHALLENGES, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_ACCESS_CHALLENGES].name,
            p_dot1x_port->auth_be_backend_access_challenges);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_OTHERREQUEST_TO_SUPPLICANT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_OTHERREQUEST_TO_SUPPLICANT].name,
            p_dot1x_port->auth_be_backend_otherrequest_to_supplicant);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_AUTH_SUCCESS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_AUTH_SUCCESS].name,
            p_dot1x_port->auth_be_backend_auth_success);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_AUTH_FAILS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_AUTH_FAILS].name,
            p_dot1x_port->auth_be_backend_auth_fails);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_NONNAK_RESPONSES_FROM_SUPPLICANT, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_BE_BACKEND_NONNAK_RESPONSES_FROM_SUPPLICANT].name,
            p_dot1x_port->auth_be_backend_nonnak_responses_from_supplicant);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_STATE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_STATE].name,
            p_dot1x_port->auth_ctrl_dir_state);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_ADMINCONTROLLEDDIRECTIONS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_ADMINCONTROLLEDDIRECTIONS].name,
            p_dot1x_port->auth_ctrl_dir_adminControlledDirections);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_OPERCONTROLLEDDIRECTIONS, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_OPERCONTROLLEDDIRECTIONS].name,
            p_dot1x_port->auth_ctrl_dir_operControlledDirections);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_BRIDGEDETECTED, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_CTRL_DIR_BRIDGEDETECTED].name,
            p_dot1x_port->auth_ctrl_dir_bridgeDetected);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_AUTH_KEY_RECV_RXKEY, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_AUTH_KEY_RECV_RXKEY].name,
            p_dot1x_port->auth_key_recv_rxKey);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_FRAMES_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAPOL_FRAMES_RX].name,
            p_dot1x_port->eapol_frames_rx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_FRAMES_TX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAPOL_FRAMES_TX].name,
            p_dot1x_port->eapol_frames_tx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_START_FRAMES_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAPOL_START_FRAMES_RX].name,
            p_dot1x_port->eapol_start_frames_rx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_LOGOFF_FRAMES_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAPOL_LOGOFF_FRAMES_RX].name,
            p_dot1x_port->eapol_logoff_frames_rx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAP_RESPID_FRAMES_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAP_RESPID_FRAMES_RX].name,
            p_dot1x_port->eap_respid_frames_rx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAP_RESP_FRAMES_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAP_RESP_FRAMES_RX].name,
            p_dot1x_port->eap_resp_frames_rx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAP_REQID_FRAMES_TX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAP_REQID_FRAMES_TX].name,
            p_dot1x_port->eap_reqid_frames_tx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAP_REQ_FRAMES_TX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAP_REQ_FRAMES_TX].name,
            p_dot1x_port->eap_req_frames_tx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_INVALID_FRAMES_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAPOL_INVALID_FRAMES_RX].name,
            p_dot1x_port->eapol_invalid_frames_rx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAP_LEN_ERROR_FRAMES_RX, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAP_LEN_ERROR_FRAMES_RX].name,
            p_dot1x_port->eap_len_error_frames_rx);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_LAST_FRAME_VERSION, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAPOL_LAST_FRAME_VERSION].name,
            p_dot1x_port->eapol_last_frame_version);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_EAPOL_LAST_FRAME_SOURCE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_PORT_FLD_EAPOL_LAST_FRAME_SOURCE].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_port->eapol_last_frame_source, sizeof(p_dot1x_port->eapol_last_frame_source)));
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SESSION_TIME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_SESSION_TIME].name,
            p_dot1x_port->session_time);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SESSION_AUTHENTICATION_METHOD, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_SESSION_AUTHENTICATION_METHOD].name,
            p_dot1x_port->session_authentication_method);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SESSION_TERMINATE_CAUSE, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_SESSION_TERMINATE_CAUSE].name,
            p_dot1x_port->session_terminate_cause);
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_SESSION_USER_NAME, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%s", p_tbl_info->field[TBL_DOT1X_PORT_FLD_SESSION_USER_NAME].name, 
            cdb_uint8_array_val2str(buf, MAX_CMD_STR_LEN, p_dot1x_port->session_user_name, sizeof(p_dot1x_port->session_user_name)));
    }
    if (FLD_MATCH(TBL_DOT1X_PORT_FLD_CLASS_ID, p_field->field_id[0]))
    {
        sal_fprintf(fp, "/%s"CMD_EQUAL_STR"%u", p_tbl_info->field[TBL_DOT1X_PORT_FLD_CLASS_ID].name,
            p_dot1x_port->class_id);
    }

    sal_fprintf(fp, "\n");

    return PM_E_NONE;
}

int32
tbl_dot1x_port_iterate(TBL_ITER_CB_FUNC fn, tbl_iter_args_t *pargs)
{
    tbl_dot1x_port_master_t *p_master = _g_p_tbl_dot1x_port_master;
    ctclib_slistnode_t *listnode = NULL;
    ctclib_slistnode_t *next = NULL;
    tbl_dot1x_port_t *p_db_dot1x_port = NULL;
    int32 rc = PM_E_NONE;

    if (NULL == p_master)
    {
        return PM_E_NONE;
    }

    CTCLIB_SLIST_LOOP_DEL(p_master->dot1x_port_list, p_db_dot1x_port, listnode, next)
    {
        rc |= fn(p_db_dot1x_port, pargs);
    }

    return (rc) ? PM_E_FAIL : PM_E_NONE;
}

tbl_dot1x_port_master_t*
tbl_dot1x_port_get_master()
{
    return _g_p_tbl_dot1x_port_master;
}

tbl_dot1x_port_master_t*
tbl_dot1x_port_init_dot1x_port()
{
    _g_p_tbl_dot1x_port_master = XCALLOC(MEM_TBL_MASTER, sizeof(tbl_dot1x_port_master_t));
    _g_p_tbl_dot1x_port_master->dot1x_port_hash = ctclib_hash_create(_tbl_dot1x_port_hash_make, _tbl_dot1x_port_hash_cmp);
    _g_p_tbl_dot1x_port_master->dot1x_port_list = ctclib_slist_create(tbl_dot1x_port_cmp, NULL);
    return _g_p_tbl_dot1x_port_master;
}

