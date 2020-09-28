
#ifndef __TBL_NTP_CFG_DEFINE_H__
#define __TBL_NTP_CFG_DEFINE_H__

/* TBL_NTP_CFG field defines */
typedef enum
{
    TBL_NTP_CFG_FLD_ACLCOUNT             = 0 ,  /* RW */
    TBL_NTP_CFG_FLD_UCASTSERVERCOUNT     = 1 ,  /* RW */
    TBL_NTP_CFG_FLD_UCASTCLIENTCOUNT     = 2 ,  /* RW */
    TBL_NTP_CFG_FLD_BCASTSERVERCOUNT     = 3 ,  /* RW */
    TBL_NTP_CFG_FLD_MCASTCLIENTCOUNT     = 4 ,  /* RW */
    TBL_NTP_CFG_FLD_MCASTSERVERCOUNT     = 5 ,  /* RW */
    TBL_NTP_CFG_FLD_BCASTDELAY           = 6 ,  /* RW */
    TBL_NTP_CFG_FLD_MINIMUMDISTANCE      = 7 ,  /* RW */
    TBL_NTP_CFG_FLD_AUTHENTICATION       = 8 ,  /* RW */
    TBL_NTP_CFG_FLD_BREFCLOCK            = 9 ,  /* RW */
    TBL_NTP_CFG_FLD_REFCLOCK_STRATUM     = 10,  /* RW */
    TBL_NTP_CFG_FLD_IFRELOADCOUNT        = 11,  /* RW */
    TBL_NTP_CFG_FLD_CLEARSTATS           = 12,  /* RW */
    TBL_NTP_CFG_FLD_DEBUG_LEVEL          = 13,  /* RW */
    TBL_NTP_CFG_FLD_UCASTSERVERCOUNT_INBAND = 14,  /* RW */
    TBL_NTP_CFG_FLD_UCASTSERVERCOUNT_OUTBAND = 15,  /* RW */
    TBL_NTP_CFG_FLD_MAX                  = 16
} tbl_ntp_cfg_field_id_t;

/* TBL_NTP_CFG defines */
typedef struct
{
    int32                aclCount;
    int32                ucastServerCount;
    int32                ucastClientCount;
    int32                bcastServerCount;
    int32                mcastClientCount;
    int32                mcastServerCount;
    int32                bcastDelay;
    int32                minimumDistance;
    int32                authentication;      /* authentication mode, 1 enable, 0 disable */
    int32                brefclock;
    int32                refclock_stratum;
    int32                ifreloadCount;
    int32                clearStats;
    int32                debug_level;         /* default is 2(crit) */
    int32                ucastServerCount_inband;
    int32                ucastServerCount_outband;
} tbl_ntp_cfg_t;

#endif /* !__TBL_NTP_CFG_DEFINE_H__ */

