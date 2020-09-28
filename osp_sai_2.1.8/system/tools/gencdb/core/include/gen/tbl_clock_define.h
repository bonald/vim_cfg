
#ifndef __TBL_CLOCK_DEFINE_H__
#define __TBL_CLOCK_DEFINE_H__

/* TBL_CLOCK field defines */
typedef enum
{
    TBL_CLOCK_FLD_TIMEZONE_POSITIVE    = 0 ,  /* RW */
    TBL_CLOCK_FLD_TIMEZONE_HOUR        = 1 ,  /* RW */
    TBL_CLOCK_FLD_TIMEZONE_MINUTE      = 2 ,  /* RW */
    TBL_CLOCK_FLD_TIMEZONE_SECOND      = 3 ,  /* RW */
    TBL_CLOCK_FLD_TIMEZONE_NAME        = 4 ,  /* RW */
    TBL_CLOCK_FLD_CP_TIMEZONE_POSITIVE = 5 ,  /* RW */
    TBL_CLOCK_FLD_CP_TIMEZONE_HOUR     = 6 ,  /* RW */
    TBL_CLOCK_FLD_CP_TIMEZONE_MINUTE   = 7 ,  /* RW */
    TBL_CLOCK_FLD_CP_TIMEZONE_SECOND   = 8 ,  /* RW */
    TBL_CLOCK_FLD_CP_TIMEZONE_NAME     = 9 ,  /* RW */
    TBL_CLOCK_FLD_MAX                  = 10
} tbl_clock_field_id_t;

/* TBL_CLOCK defines */
typedef struct
{
    uint8                timezone_positive;
    uint8                timezone_hour;
    uint8                timezone_minute;
    uint8                timezone_second;
    char                 timezone_name[MAX_ZONE_NAME_LEN+1];
    uint8                cp_timezone_positive;
    uint8                cp_timezone_hour;
    uint8                cp_timezone_minute;
    uint8                cp_timezone_second;
    char                 cp_timezone_name[MAX_ZONE_NAME_LEN+1];
} tbl_clock_t;

#endif /* !__TBL_CLOCK_DEFINE_H__ */

