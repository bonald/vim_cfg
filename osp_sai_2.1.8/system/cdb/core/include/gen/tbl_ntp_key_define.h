
#ifndef __TBL_NTP_KEY_DEFINE_H__
#define __TBL_NTP_KEY_DEFINE_H__

/* TBL_NTP_KEY field defines */
typedef enum
{
    TBL_NTP_KEY_FLD_KEY                  = 0 ,  /* RW */
    TBL_NTP_KEY_FLD_VALUE                = 1 ,  /* RW */
    TBL_NTP_KEY_FLD_TRUSTED              = 2 ,  /* RW */
    TBL_NTP_KEY_FLD_MAX                  = 3 
} tbl_ntp_key_field_id_t;

/* TBL_NTP_KEY defines */
typedef struct
{
    uint32               keyid;
} tbl_ntp_key_key_t;

typedef struct
{
    tbl_ntp_key_key_t    key;
    char                 value[NTP_MAX_KEY_LEN];
    uint32               trusted;
} tbl_ntp_key_t;

typedef struct
{
    ctclib_slist_t       *ntp_key_list;
} tbl_ntp_key_master_t;

#endif /* !__TBL_NTP_KEY_DEFINE_H__ */

