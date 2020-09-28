
#ifndef __TBL_DHCLIENT_DEFINE_H__
#define __TBL_DHCLIENT_DEFINE_H__

/* TBL_DHCLIENT field defines */
typedef enum
{
    TBL_DHCLIENT_FLD_ENABLE               = 0 ,  /* RW */
    TBL_DHCLIENT_FLD_DISTANCE             = 1 ,  /* RW */
    TBL_DHCLIENT_FLD_BROADCAST            = 2 ,  /* RW */
    TBL_DHCLIENT_FLD_MAX                  = 3 
} tbl_dhclient_field_id_t;

/* TBL_DHCLIENT defines */
typedef struct
{
    uint8                enable;              /* enable / disable */
    uint8                distance;            /* default-router distance 254 */
    uint8                broadcast;           /* client can¡¯t receive unicast IP datagrams until been configured with an IP address */
} tbl_dhclient_t;

#endif /* !__TBL_DHCLIENT_DEFINE_H__ */

