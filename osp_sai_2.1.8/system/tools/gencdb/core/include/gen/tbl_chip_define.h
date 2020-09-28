
#ifndef __TBL_CHIP_DEFINE_H__
#define __TBL_CHIP_DEFINE_H__

/* TBL_CHIP field defines */
typedef enum
{
    TBL_CHIP_FLD_TYPE                 = 0 ,  /* READ */
    TBL_CHIP_FLD_MAX                  = 1 
} tbl_chip_field_id_t;

/* TBL_CHIP defines */
typedef struct
{
    uint32               type;                /* refer to glb_chip_type_t */
} tbl_chip_t;

#endif /* !__TBL_CHIP_DEFINE_H__ */

