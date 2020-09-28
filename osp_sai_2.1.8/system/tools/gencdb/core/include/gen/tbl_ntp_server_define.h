
#ifndef __TBL_NTP_SERVER_DEFINE_H__
#define __TBL_NTP_SERVER_DEFINE_H__

/* TBL_NTP_SERVER field defines */
typedef enum
{
    TBL_NTP_SERVER_FLD_KEY                  = 0 ,  /* RW */
    TBL_NTP_SERVER_FLD_KEYID                = 1 ,  /* RW */
    TBL_NTP_SERVER_FLD_VERSION              = 2 ,  /* RW */
    TBL_NTP_SERVER_FLD_PREFER               = 3 ,  /* RW */
    TBL_NTP_SERVER_FLD_TYPE                 = 4 ,  /* RW */
    TBL_NTP_SERVER_FLD_IS_INBAND            = 5 ,  /* RW */
    TBL_NTP_SERVER_FLD_MAX                  = 6 
} tbl_ntp_server_field_id_t;

/* TBL_NTP_SERVER defines */
typedef struct
{
    char                 host[NTP_MAX_HOST_IP_SIZE];
} tbl_ntp_server_key_t;

typedef struct
{
    tbl_ntp_server_key_t key;
    uint32               keyid;
    uint32               version;
    uint32               prefer;
    uint32               type;
    uint32               is_inband;
} tbl_ntp_server_t;

typedef struct
{
    ctclib_slist_t       *ntp_server_list;
} tbl_ntp_server_master_t;

#endif /* !__TBL_NTP_SERVER_DEFINE_H__ */

