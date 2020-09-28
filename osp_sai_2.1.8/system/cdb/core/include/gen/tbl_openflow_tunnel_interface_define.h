
#ifndef __TBL_OPENFLOW_TUNNEL_INTERFACE_DEFINE_H__
#define __TBL_OPENFLOW_TUNNEL_INTERFACE_DEFINE_H__

/* TBL_OPENFLOW_TUNNEL_INTERFACE field defines */
typedef enum
{
    TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_KEY                  = 0 ,  /* READ */
    TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_OPENFLOW_ENABLE      = 1 ,  /* RW */
    TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_SOURCE_IP            = 2 ,  /* RW */
    TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_REMOTE_IP            = 3 ,  /* RW */
    TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_PORT_NAME       = 4 ,  /* RW */
    TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_MAC             = 5 ,  /* RW */
    TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_BIND_VLAN            = 6 ,  /* RW */
    TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_FLAG                 = 7 ,  /* RW */
    TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_TUNNEL_STATUS        = 8 ,  /* RW */
    TBL_OPENFLOW_TUNNEL_INTERFACE_FLD_MAX                  = 9 
} tbl_openflow_tunnel_interface_field_id_t;

/* TBL_OPENFLOW_TUNNEL_INTERFACE defines */
typedef struct
{
    char                 name[IFNAME_SIZE];
} tbl_openflow_tunnel_interface_key_t;

typedef struct
{
    tbl_openflow_tunnel_interface_key_t key;
    uint8                openflow_enable;     /* openflow enable, default:0 */
    char                 source_ip[IPADDR_BUF_SIZE]; /* tunnel src ip */
    char                 remote_ip[IPADDR_BUF_SIZE]; /* tunnel dst ip */
    char                 bind_port_name[IFNAME_SIZE]; /* bind port name */
    char                 bind_mac[MACADDR_BUF_SIZE]; /*mac address*/
    uint32               bind_vlan;           /*vlan id*/
    uint32               flag;                /*configure flag*/
    uint32               tunnel_status;
} tbl_openflow_tunnel_interface_t;

typedef struct
{
    ctclib_hash_t        *tunnel_if_hash;
    ctclib_slist_t       *tunnel_if_list;
} tbl_openflow_tunnel_interface_master_t;

#endif /* !__TBL_OPENFLOW_TUNNEL_INTERFACE_DEFINE_H__ */

