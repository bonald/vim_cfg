
#ifndef __TBL_DHCSNOOPING_DEFINE_H__
#define __TBL_DHCSNOOPING_DEFINE_H__

/* TBL_DHCSNOOPING field defines */
typedef enum
{
    TBL_DHCSNOOPING_FLD_VERIFY_MAC_ADDRESS   = 0 ,  /* RW */
    TBL_DHCSNOOPING_FLD_ADD_AGENT_OPTIONS    = 1 ,  /* RW */
    TBL_DHCSNOOPING_FLD_ALLOW_UNTRUSTED      = 2 ,  /* RW */
    TBL_DHCSNOOPING_FLD_HOSTNAME_AS_REMOTE_ID = 3 ,  /* RW */
    TBL_DHCSNOOPING_FLD_REMOTE_ID_STRING     = 4 ,  /* RW */
    TBL_DHCSNOOPING_FLD_DATABASE_SAVE_INTERVAL = 5 ,  /* RW */
    TBL_DHCSNOOPING_FLD_VLANS                = 6 ,  /* RW */
    TBL_DHCSNOOPING_FLD_SNOOPING_ACL_APPLIED = 7 ,  /* READ */
    TBL_DHCSNOOPING_FLD_MAX                  = 8 
} tbl_dhcsnooping_field_id_t;

/* TBL_DHCSNOOPING defines */
typedef struct
{
    uint8                verify_mac_address;  /* verify mac address */
    uint8                add_agent_options;   /* add agent options or not */
    uint8                allow_untrusted;     /* Allow untrusted ports to accept incoming DHCP packets with relay agent information */
    uint8                hostname_as_remote_id; /* use hostname as remote id or not */
    char                 remote_id_string[DHCSNOOPING_MAX_REMOTE_ID_STRING]; /* remote id string */
    uint16               database_save_interval; /* binding database save interval */
    vlan_bmp_t           vlans;               /* snooping vlan bit map */
    vlan_bmp_t           snooping_acl_applied; /* is snooping acl applied */
} tbl_dhcsnooping_t;

#endif /* !__TBL_DHCSNOOPING_DEFINE_H__ */

