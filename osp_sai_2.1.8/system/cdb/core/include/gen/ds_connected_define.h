
#ifndef __DS_CONNECTED_DEFINE_H__
#define __DS_CONNECTED_DEFINE_H__

/* DS_CONNECTED field defines */
typedef enum
{
    DS_CONNECTED_FLD_KEY                  = 0 ,  /* READ */
    DS_CONNECTED_FLD_DESTINATION          = 1 ,  /* RW */
    DS_CONNECTED_FLD_FLAGS_SECONDARY      = 2 ,  /* READ */
    DS_CONNECTED_FLD_FLAGS_ANYCAST        = 3 ,  /* READ */
    DS_CONNECTED_FLD_FLAGS_VIRTUAL        = 4 ,  /* READ */
    DS_CONNECTED_FLD_FLAGS_DUPLICATE      = 5 ,  /* READ */
    DS_CONNECTED_FLD_INSTALLED            = 6 ,  /* READ */
    DS_CONNECTED_FLD_MAX                  = 7 
} ds_connected_field_id_t;

/* DS_CONNECTED defines */
typedef struct
{
    prefix_t             address;
} ds_connected_key_t;

typedef struct
{
    ds_connected_key_t   key;
    prefix_t             destination;
    uint8                flags;
    uint8                installed;
} ds_connected_t;

#endif /* !__DS_CONNECTED_DEFINE_H__ */

