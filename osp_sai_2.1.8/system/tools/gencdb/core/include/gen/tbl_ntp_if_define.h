
#ifndef __TBL_NTP_IF_DEFINE_H__
#define __TBL_NTP_IF_DEFINE_H__

/* TBL_NTP_IF field defines */
typedef enum
{
    TBL_NTP_IF_FLD_DISABLE              = 0 ,  /* RW */
    TBL_NTP_IF_FLD_BROADCASTCLIENT      = 1 ,  /* RW */
    TBL_NTP_IF_FLD_MAX                  = 2 
} tbl_ntp_if_field_id_t;

/* TBL_NTP_IF defines */
typedef struct
{
    int32                disable;
    int32                broadcastClient;
} tbl_ntp_if_t;

#endif /* !__TBL_NTP_IF_DEFINE_H__ */

