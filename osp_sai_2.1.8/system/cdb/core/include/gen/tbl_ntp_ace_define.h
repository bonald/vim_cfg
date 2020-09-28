
#ifndef __TBL_NTP_ACE_DEFINE_H__
#define __TBL_NTP_ACE_DEFINE_H__

/* TBL_NTP_ACE field defines */
typedef enum
{
    TBL_NTP_ACE_FLD_KEY                  = 0 ,  /* RW */
    TBL_NTP_ACE_FLD_OPTION               = 1 ,  /* RW */
    TBL_NTP_ACE_FLD_FAMILY               = 2 ,  /* RW */
    TBL_NTP_ACE_FLD_MASKLEN              = 3 ,  /* RW */
    TBL_NTP_ACE_FLD_MAX                  = 4 
} tbl_ntp_ace_field_id_t;

/* TBL_NTP_ACE defines */
typedef struct
{
    char                 host[NTP_MAX_HOST_IP_SIZE];
    char                 mask[NTP_MAX_HOST_IP_SIZE];
} tbl_ntp_ace_key_t;

typedef struct
{
    tbl_ntp_ace_key_t    key;
    uint32               option;
    uint32               family;
    uint32               masklen;
} tbl_ntp_ace_t;

typedef struct
{
    ctclib_slist_t       *ntp_ace_list;
} tbl_ntp_ace_master_t;

#endif /* !__TBL_NTP_ACE_DEFINE_H__ */

