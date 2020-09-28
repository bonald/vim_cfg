
#ifndef __TBL_TAP_DPI_GLOBAL_DEFINE_H__
#define __TBL_TAP_DPI_GLOBAL_DEFINE_H__

/* TBL_TAP_DPI_GLOBAL field defines */
typedef enum
{
    TBL_TAP_DPI_GLOBAL_FLD_DPI_ACTION           = 0 ,  /* RW */
    TBL_TAP_DPI_GLOBAL_FLD_REDIR_IF             = 1 ,  /* RW */
    TBL_TAP_DPI_GLOBAL_FLD_MAX                  = 2 
} tbl_tap_dpi_global_field_id_t;

/* TBL_TAP_DPI_GLOBAL defines */
typedef struct
{
    uint32               dpi_action;
    char                 redir_if[IFNAME_SIZE+1]; /* tap dpi redirect interface */
} tbl_tap_dpi_global_t;

#endif /* !__TBL_TAP_DPI_GLOBAL_DEFINE_H__ */

