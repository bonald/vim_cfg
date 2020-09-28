
#ifndef __TBL_SYS_GLOBAL_DEFINE_H__
#define __TBL_SYS_GLOBAL_DEFINE_H__

/* TBL_SYS_GLOBAL field defines */
typedef enum
{
    TBL_SYS_GLOBAL_FLD_HOSTNAME             = 0 ,  /* RW */
    TBL_SYS_GLOBAL_FLD_ROUTE_MAC            = 1 ,  /* READ */
    TBL_SYS_GLOBAL_FLD_MAC_NUM              = 2 ,  /* READ */
    TBL_SYS_GLOBAL_FLD_CURR_VLANIF_COUNT    = 3 ,  /* READ */
    TBL_SYS_GLOBAL_FLD_INIT_DONE            = 4 ,  /* READ */
    TBL_SYS_GLOBAL_FLD_STARTUP_DONE         = 5 ,  /* READ */
    TBL_SYS_GLOBAL_FLD_MAX_FRAME_SIZE       = 6 ,  /* READ */
    TBL_SYS_GLOBAL_FLD_JUMBO_FRAME_SIZE     = 7 ,  /* READ */
    TBL_SYS_GLOBAL_FLD_REBOOT_TYPE          = 8 ,  /* READ */
    TBL_SYS_GLOBAL_FLD_IPG_SHAPING_ENABLE   = 9 ,  /* RW */
    TBL_SYS_GLOBAL_FLD_IPG_POLICING_ENABLE  = 10,  /* RW */
    TBL_SYS_GLOBAL_FLD_IPG_STORM_CONTROL_ENABLE = 11,  /* RW */
    TBL_SYS_GLOBAL_FLD_AAA_NEW_MODEL        = 12,  /* RW */
    TBL_SYS_GLOBAL_FLD_SERVICE_PASSWORD_ENCRYPTION = 13,  /* RW */
    TBL_SYS_GLOBAL_FLD_MAX_VTY              = 14,  /* RW */
    TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE1       = 15,  /* RW */
    TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE2       = 16,  /* RW */
    TBL_SYS_GLOBAL_FLD_AAA_PRIVILEGE3       = 17,  /* RW */
    TBL_SYS_GLOBAL_FLD_DHCP_SERVICE_ENABLE  = 18,  /* RW */
    TBL_SYS_GLOBAL_FLD_DHCP_RELAY_ENABLE    = 19,  /* RW */
    TBL_SYS_GLOBAL_FLD_DHCP_SNOOPING_ENABLE = 20,  /* RW */
    TBL_SYS_GLOBAL_FLD_HTTP_SERVICE_ENABLE  = 21,  /* RW */
    TBL_SYS_GLOBAL_FLD_HTTP_IMAGE_NAME      = 22,  /* RW */
    TBL_SYS_GLOBAL_FLD_ERRDISABLE_INTERVAL  = 23,  /* RW */
    TBL_SYS_GLOBAL_FLD_TRUNCTION_LENGTH     = 24,  /* RW */
    TBL_SYS_GLOBAL_FLD_TAP_TS_MACDA         = 25,  /* RW */
    TBL_SYS_GLOBAL_FLD_TAP_TS_MACSA         = 26,  /* RW */
    TBL_SYS_GLOBAL_FLD_ETHER_TYPE           = 27,  /* RW */
    TBL_SYS_GLOBAL_FLD_PTF_PORT             = 28,  /* RW */
    TBL_SYS_GLOBAL_FLD_TELNET_DISABLE       = 29,  /* RW */
    TBL_SYS_GLOBAL_FLD_TELNET_PORT          = 30,  /* RW */
    TBL_SYS_GLOBAL_FLD_TELNET_ACL           = 31,  /* RW */
    TBL_SYS_GLOBAL_FLD_SSH_ACL              = 32,  /* RW */
    TBL_SYS_GLOBAL_FLD_HTTP_DISABLE         = 33,  /* RW */
    TBL_SYS_GLOBAL_FLD_HTTP_PORT            = 34,  /* RW */
    TBL_SYS_GLOBAL_FLD_HTTP_TIMEOUT         = 35,  /* RW */
    TBL_SYS_GLOBAL_FLD_HTTPS_DISABLE        = 36,  /* RW */
    TBL_SYS_GLOBAL_FLD_HTTPS_PORT           = 37,  /* RW */
    TBL_SYS_GLOBAL_FLD_RPCAPI_DISABLE       = 38,  /* RW */
    TBL_SYS_GLOBAL_FLD_RPCAPI_PORT          = 39,  /* RW */
    TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_EN        = 40,  /* RW */
    TBL_SYS_GLOBAL_FLD_RPCAPI_SSL_PORT      = 41,  /* RW */
    TBL_SYS_GLOBAL_FLD_RPCAPI_AUTH_MODE     = 42,  /* RW */
    TBL_SYS_GLOBAL_FLD_OPER_RPCAPI_AUTH_MODE = 43,  /* RW */
    TBL_SYS_GLOBAL_FLD_CUT_THROUGH_ENABLE   = 44,  /* RW */
    TBL_SYS_GLOBAL_FLD_PROTECTED_VLAN_CNT   = 45,  /* RW */
    TBL_SYS_GLOBAL_FLD_VLAN_STATS_CNT       = 46,  /* RW */
    TBL_SYS_GLOBAL_FLD_MIB_PORT_STATS_READ_NUM = 47,  /* RW */
    TBL_SYS_GLOBAL_FLD_MIB_FLOW_STATS_READ_NUM = 48,  /* RW */
    TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD1    = 49,  /* RW */
    TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD2    = 50,  /* RW */
    TBL_SYS_GLOBAL_FLD_MEMORY_THRESHOLD3    = 51,  /* RW */
    TBL_SYS_GLOBAL_FLD_SYSTEM_MEMCHK_STATE  = 52,  /* RW */
    TBL_SYS_GLOBAL_FLD_MGMT_IF_RUNNING      = 53,  /* READ */
    TBL_SYS_GLOBAL_FLD_MGMT_IF_TIMER        = 54,  /* READ */
    TBL_SYS_GLOBAL_FLD_WARMBOOT_TYPE        = 55,  /* READ */
    TBL_SYS_GLOBAL_FLD_WARMBOOT_TIME        = 56,  /* READ */
    TBL_SYS_GLOBAL_FLD_WARMBOOT_COUNT       = 57,  /* READ */
    TBL_SYS_GLOBAL_FLD_SYSTEM_TIME_UPDATE   = 58,  /* READ */
    TBL_SYS_GLOBAL_FLD_SFLOW_COUNTER_IFINDEX = 59,  /* READ */
    TBL_SYS_GLOBAL_FLD_DHCP_SERVER_ENABLE   = 60,  /* RW */
    TBL_SYS_GLOBAL_FLD_ADD_GRE_ACE_ENTRY_NUM = 61,  /* RW */
    TBL_SYS_GLOBAL_FLD_ADD_VXLAN_ENTRY_NUM  = 62,  /* RW */
    TBL_SYS_GLOBAL_FLD_SYSTEM_PRODUCT       = 63,  /* RW */
    TBL_SYS_GLOBAL_FLD_MAX                  = 64
} tbl_sys_global_field_id_t;

/* TBL_SYS_GLOBAL defines */
typedef struct
{
    char                 hostname[HOSTNAME_SIZE+1];
    mac_addr_t           route_mac;
    uint32               mac_num;
    int32                curr_vlanif_count;
    uint32               init_done;
    uint32               startup_done;
    uint32               max_frame_size;
    uint32               jumbo_frame_size;
    uint8                reboot_type;         /* glb_reboot_type_t, this boot reason */
    uint8                ipg_shaping_enable;
    uint8                ipg_policing_enable;
    uint8                ipg_storm_control_enable;
    uint8                aaa_new_model;       /* AAA new model or not, default not */
    uint8                service_password_encryption; /* encrypt password or not */
    uint8                max_vty;             /* MAX vty numbers */
    uint8                aaa_privilege1;
    uint8                aaa_privilege2;
    uint8                aaa_privilege3;
    uint8                dhcp_service_enable; /* enable DHCP service or not */
    uint8                dhcp_relay_enable;   /* enable DHCP relay or not */
    uint8                dhcp_snooping_enable; /* enable DHCP snooping or not */
    uint8                http_service_enable; /* TODO, enable RPC HTTP service */
    char                 http_image_name[HTTP_IMAGE_NAME_SIZE]; /* TODO, HTTP image file name */
    uint32               errdisable_interval; /* errdisable recovery interval*/
    uint32               trunction_length;    /* Truncation length */
    mac_addr_t           tap_ts_macda;        /* TAP Timestamp macda */
    mac_addr_t           tap_ts_macsa;        /* TAP Timestamp macsa */
    uint16               ether_type;          /* TAP Timestamp ethernet type */
    uint32               ptf_port;            /* PTF test TCP port, 0 means PTF disable */
    uint32               telnet_disable;      /* disable telnet service */
    uint32               telnet_port;         /* modify telnet service port */
    char                 telnet_acl[ACL_NAME_FULL_NAME_SIZE+1]; /* Telnet ACL name*/
    char                 ssh_acl[ACL_NAME_FULL_NAME_SIZE+1]; /* SSH ACL name*/
    uint32               http_disable;        /* disable http service */
    uint32               http_port;           /* modify http service port*/
    uint32               http_timeout;        /* modify http timeout interval*/
    uint32               https_disable;       /* disable https service */
    uint32               https_port;          /* modify https service port*/
    uint32               rpcapi_disable;      /* disable rpcapi service */
    uint32               rpcapi_port;         /* modify rpcapi service port*/
    uint32               rpcapi_ssl_en;       /* rpcapi ssl enable */
    uint32               rpcapi_ssl_port;     /* rpcapi ssl port, default is 443 */
    uint32               rpcapi_auth_mode;    /* refer to glb_rpcapi_auth_mode_t */
    uint32               oper_rpcapi_auth_mode; /* refer to glb_rpcapi_auth_mode_t, effective auth_mode */
    uint8                cut_through_enable;  /* Enable cut through or not */
    uint32               protected_vlan_cnt;  /*protected vlan count*/
    uint32               vlan_stats_cnt;      /*vlan statistics count*/
    uint32               mib_port_stats_read_num; /* MIB port stats read number per round */
    uint32               mib_flow_stats_read_num; /* MIB flow stats read number per round */
    uint32               memory_threshold1;   /*memory threshold one*/
    uint32               memory_threshold2;   /*memory threshold two*/
    uint32               memory_threshold3;   /*memory threshold three*/
    uint32               system_memchk_state; /*syslimit_memchk_state_t*/
    uint32               mgmt_if_running;     /* mgmt if up/down state */
    ctc_task_t           *mgmt_if_timer;      /* monitor mgmt if up/down timer */
    uint32               warmboot_type;       /* fastboot/warmboot type */
    uint32               warmboot_time;       /* fastboot/warmboot time */
    uint32               warmboot_count;      /* fastboot/warmboot count */
    uint32               system_time_update;  /* ntp sync, after update system time */
    uint32               sflow_counter_ifindex; /* update counters */
    uint8                dhcp_server_enable;  /* enable DHCP server or not */
    uint32               add_gre_ace_entry_num; /*add-l2gre or add-l3gre ace entry number*/
    uint32               add_vxlan_entry_num; /*add-vxlan ace entry number*/
    uint32               system_product;
} tbl_sys_global_t;

#endif /* !__TBL_SYS_GLOBAL_DEFINE_H__ */

