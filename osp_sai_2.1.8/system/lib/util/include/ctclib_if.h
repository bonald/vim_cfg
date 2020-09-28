#ifndef __CTCLIB_IF_H__
#define __CTCLIB_IF_H__

#define CLI_ARGV_MAX_LEN                256

#define CLI_DEF_PHY_PORT_PREFIX         GLB_DEF_PHY_PORT_PREFIX
#define CLI_DEF_PHY_PORT_FULL_PREFIX    GLB_DEF_PHY_PORT_FULL_PREFIX

#define CLI_PHY_PORT_NUM_MAX            256  
#define CLI_PHT_PORT_TYPE_MAX           GLB_PHT_PORT_TYPE_MAX

#define IF_DEF_PORT_CONN_CHAR          '-'
#define IF_DEF_SUBPORT_CONN_CHAR       '/'
    
struct cli_phy_port_prefix_s
{
    bool valid;
    char prefix[GLB_PHY_PORT_PREFIX_SZ];
    char full_prefix[GLB_PHY_PORT_FULL_PREFIX_SZ];
    char desc[CLI_ARGV_MAX_LEN];
};
typedef struct cli_phy_port_prefix_s cli_phy_port_prefix_t;

struct cli_phy_port_s
{
    uint16 slot_id;
    uint16 port_id;
    uint16 sub_id;
    uint16 port_type;
    uint32 ifindex;
};
typedef struct cli_phy_port_s cli_phy_port_t;

struct cli_phy_port_master_s
{
    uint32 is_default;
    char port_connection;
    char sub_port_connection;
    uint16 valid_num;  /*valid port number in cli_phy_port_t*/
    cli_phy_port_t port[CLI_PHY_PORT_NUM_MAX];
    cli_phy_port_prefix_t port_prefix[CLI_PHT_PORT_TYPE_MAX];
    int split_type[GLB_SPEED_MAX]; /*the real speed type*/
    uint16 match_port_type[CLI_PHT_PORT_TYPE_MAX]; /*save tmp info while parse port*/
    uint16 match_port_id; /*save tmp info while parse port*/
    uint16 match_sub_port_id; /*save tmp info while parse port*/
    char match_name[GLB_PHY_PORT_FULL_NAME_SZ]; /*same as INTERFACE_NAMSIZ*/
};
typedef struct cli_phy_port_master_s cli_phy_port_master_t;

int32
if_get_port_num_by_slot_port(uint8 slot_id, uint8 port_id, uint8 sub_port_id, uint8 *p_port_num);

int32
if_build_port_shortname_by_slot_port(char *ifname, uint16 slot_id, uint16 port_id, uint16 sub_port_id);

int32
if_build_port_fullname_by_slot_port(char *ifname, uint16 slot_id, uint16 port_id, uint16 sub_port_id);

int32
if_check_phy_port_by_shortname(char *ifname);

int32
if_check_phy_port_by_fullname(char *ifname);

int32
if_check_phy_port_by_name(char *ifname);

int32
if_get_slot_port_by_fullname(char *ifname, uint8 *slot_id, uint8 *port_id, uint8 *sub_port_id);

int32
if_get_slot_port_by_shortname(char *ifname, uint8 *slot_id, uint8 *port_id, uint8 *sub_port_id);

int32
if_get_slot_port_by_name(char *ifname, uint8 *slot_id, uint8 *port_id, uint8 *sub_port_id);

char*
if_get_port_fullname_name(char *ifname);

char*
if_get_port_shortname_name(char *ifname);

int32
if_build_ethname_by_slot_port(char *ifname, uint8 slot_no, uint8 port_no, uint8 sub_port_no);

int32
if_get_slot_port_by_ethname(char *ifname, uint8 *slot_no, uint8 *port_no, uint8 *sub_port_no);

int32
if_get_ifindex_by_name(const char *ifname);

int32
if_get_name_by_ifindex(uint32 ifindex, char *name, uint32 len);

char*
if_ethname_to_shortname(char *ethname, char *shortname, uint32 buf_len);

char*
if_ethname_to_fullname(char *ethname, char *fullname, uint32 buf_len);

char*
if_exname_to_ethname(char *exname, char *name, uint32 buf_len);
    
#define IFNAME_ETH2SHORT(_ETHNAME, _BUF) if_ethname_to_shortname(_ETHNAME, _BUF, GLB_IFNAME_SZ)
#define IFNAME_ETH2FULL(_ETHNAME, _BUF) if_ethname_to_fullname(_ETHNAME, _BUF, GLB_IFNAME_SZ)
#define IFNAME_EX2ETH(_EXNAME, _BUF) if_exname_to_ethname(_EXNAME, _BUF, GLB_IFNAME_SZ)

cli_phy_port_master_t*
if_get_phy_port_master();

int32
ctclib_if_init();

#endif /* !__CTCLIB_IF_H__ */

