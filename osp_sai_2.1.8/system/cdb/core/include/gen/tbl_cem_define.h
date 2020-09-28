
#ifndef __TBL_CEM_DEFINE_H__
#define __TBL_CEM_DEFINE_H__

/* TBL_CEM field defines */
typedef enum
{
    TBL_CEM_FLD_KEY                  = 0 ,  /* READ */
    TBL_CEM_FLD_EVENT                = 1 ,  /* RW */
    TBL_CEM_FLD_THRESHOLD            = 2 ,  /* RW */
    TBL_CEM_FLD_SNMPTRAP             = 3 ,  /* RW */
    TBL_CEM_FLD_LOADPATH             = 4 ,  /* RW */
    TBL_CEM_FLD_MAIL                 = 5 ,  /* RW */
    TBL_CEM_FLD_MAX                  = 6 
} tbl_cem_field_id_t;

/* TBL_CEM defines */
typedef struct
{
    char                 name[COMM_NAME_SIZE+1];
} tbl_cem_key_t;

typedef struct
{
    tbl_cem_key_t        key;
    char                 event[CEM_NAME_SIZE]; /*event*/
    uint32               threshold;
    uint32               snmptrap;            /*action send trap*/
    char                 loadpath[COMM_NAME_SIZE]; /*action load config file*/
    mail_t               mail;                /*action send email*/
} tbl_cem_t;

typedef struct
{
    ctclib_hash_t        *cem_hash;
    ctclib_slist_t       *cem_list;
} tbl_cem_master_t;

#endif /* !__TBL_CEM_DEFINE_H__ */

