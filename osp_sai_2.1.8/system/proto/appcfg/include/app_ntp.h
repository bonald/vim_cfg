#ifndef __APP_NTP_H__
#define __APP_NTP_H__

enum
{
    BITOFFSET_VERSION = 0,
    BITOFFSET_KOD,
    BITOFFSET_IGNORE,
    BITOFFSET_NOQUERY,
    BITOFFSET_NOMODIFY,
    BITOFFSET_NOTRAP,
    BITOFFSET_LOWPRIOTRAP,
    BITOFFSET_NOSERVE,
    BITOFFSET_NOPEER,
    BITOFFSET_NOTRUST,
    BITOFFSET_LIMITED,
    BITOFFSET_NTPPORT,
    BITOFFSET_END
};

/* delete mulitcast support
 * if you turn this on, please don't forgot to 
 * add code to finish this feature
 * current code can't working well for multicast
 */
#undef HAVE_NTP_MCAST_CLI

#define     NTP_SERVER_MAX			5
#define     NTP_MAX_LINE_SIZE           512
#define     NTP_UCAST_SERVER            "server"
#define     NTP_UCAST_PEER              "peer"
#define     NTP_MCAST_SERVER            "broadcast"
#define     NTP_MCAST_CLIENT            "multicastclient"
#define     NTP_BCAST_SERVER            "broadcast"
#define     NTP_BCAST_CLIENT            "broadcastclient"
#define     NTP_CONFIG_FILE             "/etc/ntp.conf"
#define     NTP_KEY_PATH                "/etc/ntp"
#define     NTP_KEY_FILE                NTP_KEY_PATH"/keys"
#define     NTP_DRIFT_FILE              "/etc/ntp/drift"

/* if you change this macro, please don't forgot to change it in the ntpdc */
#define NTP_DEFAULT_REQUEST_KEY_NO  64567
#define NTP_DEFAULT_REQUEST_KEY     "m6~6^b?z$}HTC<b'"
#define M_NTP_CMD_BUFSZ             1024  
#define M_NTP_LOCALHOST_IP          "127.0.0.1"
#define M_NTP_CONFIG_FILE_PATH      "/etc/ntp.conf"
#define M_NTP_LOCALHOST_IP6          "::1"
#define M_NTP_FULLMASK_IP          "255.255.255.255"
#define M_NTP_FULLMASK_IP6          "FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF"


#define NTP_MAX_KEY_VALUE       64000

#define NTP_OUT_SPLIT(pcli)   \
cli_out(pcli, "============================================================\n")

#define M_NTP_DEFAULT_REFCLOCK_STRATUM  10
#define M_NTP_MIN_REFCLOCK_STRATUM  1
#define M_NTP_MAX_REFCLOCK_STRATUM  15
#define M_NTP_INVALID_REFCLOCK_STRATUM  16

#define M_NTP_REFCLOCK_ADDR             "127.127.1.0"

#define M_NTP_DEFAULT_BCASTDELAY    3000
#define M_NTP_MINIMUM_DISTANCE      1

#define     NTP_PID_FILE                "/var/run/ntpd.pid"

/* hold ntp server & peer & ... */
#define M_NTP_TYPE_UCAST_SERVER     1
#define M_NTP_TYPE_UCAST_CLIENT     2
#define M_NTP_TYPE_MCAST_SERVER     10
#define M_NTP_TYPE_MCAST_CLIENT     11
#define M_NTP_TYPE_BCAST_SERVER     20
#define M_NTP_TYPE_UCAST_SERVER_INBAND     21
#define M_NTP_TYPE_UCAST_SERVER_OUTBAND   22

/* NTP Config Structure */
#define M_NTP_MAX_REFCLOCK_CNT      3
#define M_NTP_MAX_ACL_CNT           128
#define M_NTP_MAX_UCASTSERVER_CNT   16
#define M_NTP_MAX_UCASTCLIENT_CNT   16
#define M_NTP_MAX_MCASTCLIENT_CNT   16
#define M_NTP_MAX_MCASTSERVER_CNT   16
#define M_NTP_MAX_BCASTSERVER_CNT   16

/*NTP interface number*/
#define M_NTP_MAX_ETH_IF_CNT_ONE_SLOT   48
#define M_NTP_MAX_ETH_IF_CNT   600   /*  > 12(slot) * 24(port)  */
#define M_NTP_MAX_LOOPBACK_IF_CNT   256
#define M_NTP_MAX_VLAN_IF_CNT   4096
#define M_NTP_MAX_INTERFACE_CNT   (M_NTP_MAX_ETH_IF_CNT + \
                M_NTP_MAX_LOOPBACK_IF_CNT + M_NTP_MAX_VLAN_IF_CNT)

/*interface status, used in NTPConfig.ifstatus*/
#define M_NTP_LOCK_INTERFACE  (1<<0)
#define M_NTP_BROADCAST_CLIENT  (1<<1)


/*add by huwx for bug 17931, 2012.3.2*/
#define NTP_RESTRICT_ADDRESS_LEN_MAX  16 
#define NTP_RESTRICT_FLAGS_LEN_MAX    80


int32
app_ntp_init();

int32 
ntp_save_keypair();
int32 
ntp_add_ucast_server(cfg_cmd_para_t *para, char *hostname, uint32 keyid, int version, int prefer);

int32 
ntp_del_ucast_server(cfg_cmd_para_t *para, char *hostname, char *inband);
int32
ntp_get_ucast_server(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field);
int32
ntp_add_ucast_peer(cfg_cmd_para_t *para, char *hostname, uint32 keyid, int version, int prefer);
int32 
ntp_del_ucast_peer(cfg_cmd_para_t *para, char *hostname, char *inband);
int32 
ntp_add_trustedkey(cfg_cmd_para_t *para, uint32 keyid);
int32 
ntp_del_trustedkey(cfg_cmd_para_t *para, uint32 keyid);
int32
ntp_get_trustedkey(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field);
int32
ntp_add_keypair(cfg_cmd_para_t *para, uint32 keyid, char* value);
int32
ntp_del_keypair(cfg_cmd_para_t *para, uint32 keyid, int32 need_restart);
int32
ntp_if_reload(cfg_cmd_para_t *para);
int32 
ntp_enable_refclock(cfg_cmd_para_t *para, int32 stratum);
int32 
ntp_set_broadcast_delay(cfg_cmd_para_t *para, int32 delay);
int32
ntp_set_minimum_distance(cfg_cmd_para_t *para, int32 minimum_distance);
int32 
ntp_set_auth_enable(cfg_cmd_para_t *para, char* enable);
int32 
ntp_clear_stats(cfg_cmd_para_t *para);
int32 
ntp_set_debug_level(cfg_cmd_para_t *para, char* level_str);
int32
ntp_config_exec(char *host, char *pszcmd, uint8 bShowCmd);
int32
ntp_show_exec(cfg_cmd_para_t *para, char *host, char *pszcmd);
int32 
ntp_interface_disable(cfg_cmd_para_t *para, char* ifname, int32 disable);
int32 
ntp_interface_broadcast_client(cfg_cmd_para_t *para, char* ifname, int32 bclient);
int32
ntp_get_global_cfg(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field);
int32 
ntp_del_ace(cfg_cmd_para_t *para, int32 argc, char* argv[]);
int32 
ntp_del_ace_option(cfg_cmd_para_t *para, int32 argc, char* argv[]);
int32
ntp_add_ace(cfg_cmd_para_t *para, int32 argc, char* argv[]);
int32
ntp_get_ace(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field);

int32
app_ntp_normal_cfg_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);
int32
app_ntp_server_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);
int32
app_ntp_trustkey_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);
int32
app_ntp_interface_cfg_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);
int32
app_ntp_ace_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);
int32
app_cmd_process_ntp_syncstatus(char **argv, int32 argc, cfg_cmd_para_t *para);
int32
ntp_server_set_namespace(cfg_cmd_para_t *para, const char *host, const char *is_inband);

int32
app_get_ip_str2val(char *str, addr_t *addr);
#endif /* !__APP_NTP_H__ */

