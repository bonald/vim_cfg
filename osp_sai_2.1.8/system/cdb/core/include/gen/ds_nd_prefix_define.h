
#ifndef __DS_ND_PREFIX_DEFINE_H__
#define __DS_ND_PREFIX_DEFINE_H__

/* DS_ND_PREFIX field defines */
typedef enum
{
    DS_ND_PREFIX_FLD_KEY                  = 0 ,  /* READ */
    DS_ND_PREFIX_FLD_CONFIG               = 1 ,  /* RW */
    DS_ND_PREFIX_FLD_VLIFETIME            = 2 ,  /* RW */
    DS_ND_PREFIX_FLD_PLIFETIME            = 3 ,  /* RW */
    DS_ND_PREFIX_FLD_ONLINK               = 4 ,  /* RW */
    DS_ND_PREFIX_FLD_AUTOCONFIG           = 5 ,  /* RW */
    DS_ND_PREFIX_FLD_MAX                  = 6 
} ds_nd_prefix_field_id_t;

/* DS_ND_PREFIX defines */
typedef nd_prefix_key_t ds_nd_prefix_key_t;

typedef struct
{
    ds_nd_prefix_key_t   key;
    uint16               config;              /* config parameters */
    uint32               vlifetime;           /* valid lifetime. */
    uint32               plifetime;           /* preferred lifetime */
    uint16               onlink;              /* onlink determination */
    uint16               autoconfig;          /* auto configuration */
} ds_nd_prefix_t;

#endif /* !__DS_ND_PREFIX_DEFINE_H__ */

