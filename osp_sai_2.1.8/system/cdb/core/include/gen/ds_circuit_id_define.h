
#ifndef __DS_CIRCUIT_ID_DEFINE_H__
#define __DS_CIRCUIT_ID_DEFINE_H__

/* DS_CIRCUIT_ID field defines */
typedef enum
{
    DS_CIRCUIT_ID_FLD_KEY                  = 0 ,  /* READ */
    DS_CIRCUIT_ID_FLD_CIRCUIT              = 1 ,  /* RW */
    DS_CIRCUIT_ID_FLD_MAX                  = 2 
} ds_circuit_id_field_id_t;

/* DS_CIRCUIT_ID defines */
typedef struct
{
    vid_t                vid;
} ds_circuit_id_key_t;

typedef struct
{
    ds_circuit_id_key_t  key;
    char                 circuit[DHCSNOOPING_MAX_CIRCUIT_ID_STRING]; /* circuit id string */
} ds_circuit_id_t;

#endif /* !__DS_CIRCUIT_ID_DEFINE_H__ */

