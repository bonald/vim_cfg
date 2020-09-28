
#ifndef __LIB_H__
#define __LIB_H__

#define YES_NO_FROM_1_0(_str)   yes_no_str(enable_value(_str))
#define ON_OFF_FROM_1_0(_str)   on_off_str_lower(enable_value(_str))

typedef enum
{
    LIB_SYNCED = 0,
    LIB_SYNCING,
} lib_sync_state;

typedef struct lib_master_s
{
    /* Protocol Module ID. */
    pm_id_t             pm_id;
    ctc_task_master_t  *task_mst;
    ctc_task_t         *t_bhm_client;   /* PM_ID_BHM client init */
    ctc_task_t         *t_bhm_server;   /* PM_ID_BHM init */
    void               *proto_mst;
    
    int32               cfg_sock;   /* sock ID for CDS config client/server */
    int32               nc_sock;    /* sock ID for NCS config client/server */
    int32               data_sock;  /* sock ID for CDS data client/server */
    int32               sub_cdb_sock; /* sock ID for CDS subscribe cdb server */
    int32               sub_tbl_sock; /* sock ID for CDS subscribe table server */
    int32               bhm_sock;   /* sock ID for BHM */
    int32               trap_sock;  /* sock ID for send snmp trap */

    uint32              sync_enable;    /* Only for PM, if set means PM needs sync CDB data from cds */
    lib_sync_state      sync_state;     /* Only for PM, if set means CDB data has synced */
    uint32              sync_tbl_num;
    uint32              sync_tbl_ids[TBL_MAX];

} lib_master_t;

lib_master_t*
lib_init(pm_id_t pm_id);

int32
lib_add_sync_tbl(uint32 tbl_id);

int32
lib_set_sync_enable();

int32
lib_set_synced();

lib_master_t*
lib_get_master();

const char*
lib_version(char *buf, int32 len);

void
lib_print_version(char *progname);

const char*
yes_no_str(uint32 enable);

const char*
on_off_str(uint32 enable);

const char*
on_off_str_lower(uint32 enable);

const char*
enable_str(uint32 enable);

const char*
enable_str_lower(uint32 enable);

const char*
enabled_str(uint32 enable);

const char*
enabled_str_lower(uint32 enable);

const char*
active_str(uint32 active);

const char*
restart_str(uint32 active);

uint32
enable_value(const char *str);

int
check_line_str(char *pszName, int len);
#define M_LINE_STR_LIMIT    "can only include [0-9A-Fa-z\\,\\@._ -]."

int
check_name_character_and_len(char *pszName, int len);

int32
lib_bmp_alloc_id(uint32 *bmp, uint32 max);

int32
lib_bmp_release_id(uint32 *bmp, int32 id, uint32 max);

int32
lib_reboot();

int32
lib_reboot_system_trap();

int32
lib_reboot_system();

int32
lib_print_to_all_tty(char *out);

char*
lib_get_product_name();

char*
lib_strip_newline(char *str);

#endif /* !__LIB_H__ */
