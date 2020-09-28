
#ifndef __TBL_NTP_SYNCSTATUS_DEFINE_H__
#define __TBL_NTP_SYNCSTATUS_DEFINE_H__

/* TBL_NTP_SYNCSTATUS field defines */
typedef enum
{
    TBL_NTP_SYNCSTATUS_FLD_STATUS               = 0 ,  /* RW */
    TBL_NTP_SYNCSTATUS_FLD_STRATUM              = 1 ,  /* RW */
    TBL_NTP_SYNCSTATUS_FLD_FREQUENCY            = 2 ,  /* RW */
    TBL_NTP_SYNCSTATUS_FLD_PRECISION            = 3 ,  /* RW */
    TBL_NTP_SYNCSTATUS_FLD_REFERENCE_TIME       = 4 ,  /* RW */
    TBL_NTP_SYNCSTATUS_FLD_ROOT_DELAY           = 5 ,  /* RW */
    TBL_NTP_SYNCSTATUS_FLD_ROOT_DISPERSION      = 6 ,  /* RW */
    TBL_NTP_SYNCSTATUS_FLD_STABILITY            = 7 ,  /* RW */
    TBL_NTP_SYNCSTATUS_FLD_MAX                  = 8 
} tbl_ntp_syncstatus_field_id_t;

/* TBL_NTP_SYNCSTATUS defines */
typedef struct
{
    char                 status[NTP_MAX_KEY_LEN];
    char                 stratum[NTP_MAX_KEY_LEN];
    char                 frequency[NTP_MAX_KEY_LEN];
    char                 precision[NTP_MAX_KEY_LEN];
    char                 reference_time[NTP_MAX_KEY_LEN];
    char                 root_delay[NTP_MAX_KEY_LEN];
    char                 root_dispersion[NTP_MAX_KEY_LEN];
    char                 stability[NTP_MAX_KEY_LEN];
} tbl_ntp_syncstatus_t;

#endif /* !__TBL_NTP_SYNCSTATUS_DEFINE_H__ */

