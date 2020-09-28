
#ifndef __DS_DHCLIENT_IF_DEFINE_H__
#define __DS_DHCLIENT_IF_DEFINE_H__

/* DS_DHCLIENT_IF field defines */
typedef enum
{
    DS_DHCLIENT_IF_FLD_REQUESTED_FLAGS      = 0 ,  /* RW */
    DS_DHCLIENT_IF_FLD_LEASE                = 1 ,  /* RW */
    DS_DHCLIENT_IF_FLD_STATUS               = 2 ,  /* RW */
    DS_DHCLIENT_IF_FLD_OPTION               = 3 ,  /* RW */
    DS_DHCLIENT_IF_FLD_CLIENT_FLAGS         = 4 ,  /* RW */
    DS_DHCLIENT_IF_FLD_CLIENT_ID            = 5 ,  /* RW */
    DS_DHCLIENT_IF_FLD_CLASS_FLAGS          = 6 ,  /* RW */
    DS_DHCLIENT_IF_FLD_CLASS_ID             = 7 ,  /* RW */
    DS_DHCLIENT_IF_FLD_HOSTNAME             = 8 ,  /* RW */
    DS_DHCLIENT_IF_FLD_IANA_T1              = 9 ,  /* RW */
    DS_DHCLIENT_IF_FLD_IANA_T2              = 10,  /* RW */
    DS_DHCLIENT_IF_FLD_MAX                  = 11
} ds_dhclient_if_field_id_t;

/* DS_DHCLIENT_IF defines */
typedef struct
{
    uint32               requested_flags;     /* requested flags */
    uint32               lease;               /* lease time, seconds */
    uint32               status;              /* status */
    uint32               option;              /* request option */
    uint32               client_flags;
    char                 client_id[DHCP_NAME_SIZE]; /* client id */
    uint32               class_flags;
    char                 class_id[DHCP_NAME_SIZE]; /* class id */
    char                 hostname[COMM_NAME_SIZE+1]; /* hostname */
    uint32               iana_t1;             /* IA_NA  T1 */
    uint32               iana_t2;             /* IA_NA  T2 */
} ds_dhclient_if_t;

#endif /* !__DS_DHCLIENT_IF_DEFINE_H__ */

