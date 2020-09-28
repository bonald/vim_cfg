
#ifndef __DS_STORM_CONTROL_DEFINE_H__
#define __DS_STORM_CONTROL_DEFINE_H__

/* DS_STORM_CONTROL field defines */
typedef enum
{
    DS_STORM_CONTROL_FLD_BCAST_MODE           = 0 ,  /* RW */
    DS_STORM_CONTROL_FLD_MCAST_MODE           = 1 ,  /* RW */
    DS_STORM_CONTROL_FLD_UCAST_MODE           = 2 ,  /* RW */
    DS_STORM_CONTROL_FLD_BCAST_RATE           = 3 ,  /* RW */
    DS_STORM_CONTROL_FLD_MCAST_RATE           = 4 ,  /* RW */
    DS_STORM_CONTROL_FLD_UCAST_RATE           = 5 ,  /* RW */
    DS_STORM_CONTROL_FLD_MAX                  = 6 
} ds_storm_control_field_id_t;

/* DS_STORM_CONTROL defines */
typedef struct
{
    uint32               bcast_mode;          /* glb_ratelimit_mode_t */
    uint32               mcast_mode;          /* glb_ratelimit_mode_t */
    uint32               ucast_mode;          /* glb_ratelimit_mode_t */
    uint32               bcast_rate;
    uint32               mcast_rate;
    uint32               ucast_rate;
} ds_storm_control_t;

#endif /* !__DS_STORM_CONTROL_DEFINE_H__ */

