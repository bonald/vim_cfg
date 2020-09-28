
#include "glb_tempfile_define.h"
#include "hsrv_inc.h"
#include "hsrv_wb.h"
#include "sdb_defines.h"
#include "dbclient.h"

#define CONFIG_WARMBOOT

extern int32
ctc_sai_set_wb_en(uint32 en);
extern int32
ctc_sai_set_wb_restoring();
extern uint32
ctc_sai_get_wb_restoring();
extern int32
ctc_sdk_deinit(void);
extern void
ctc_vty_close();

typedef struct
{
    char       *argv[DB_MAX_DEPTH];
    char       *value[DB_MAX_DEPTH];
    uint32      argc;
} wb_parse_info_t;

/* bypass {} sub string */
char*
wb_bra_strchr(char *s, char c)
{
    uint32 bracket_depth = 0;
    
    char *p = s;
    
    while (CMD_ZERO_CHAR != *p) 
    {
        /* bypass '/' in the pair of {} */
        if (0 == bracket_depth)
        {
            if (c == *p)  
            {
                return p;
            }
        }
        p++;
    }

    return NULL;
}

int32
wb_get_parse(char *line, wb_parse_info_t *p_info)
{
    uint32          line_strlen = 0;
    char           *pos = NULL;
    char           *value = NULL;
    char            div_char = 0;

    div_char = CMD_DIVISION_CHAR;

    sal_memset(p_info, 0, sizeof(wb_parse_info_t));
    if (!line)
    {
        return PM_E_NONE;
    }
    
    line_strlen = sal_strlen(line);
    if (0 == line_strlen)
    {
        return PM_E_NONE;
    }
    if (line[line_strlen-1] == CMD_LINE_CHAR)
    {
        line[line_strlen-1] = CMD_ZERO_CHAR;
    }
    line_strlen = sal_strlen(line);
    if (line[line_strlen-1] == div_char)
    {
        line[line_strlen-1] = CMD_ZERO_CHAR;
    }

    pos = wb_bra_strchr(line, div_char);
    if (!pos)
    {
        return PM_E_NONE;
    }
    
    while (pos)
    {
        *pos = 0;
        p_info->argv[p_info->argc] = pos + 1;
        value = wb_bra_strchr(p_info->argv[p_info->argc], CMD_EQUAL_CHAR);
        if (NULL == value)
        {
            p_info->value[p_info->argc] = NULL;
        }
        p_info->value[p_info->argc] = value+1;
        p_info->argc++;
        pos = wb_bra_strchr(pos + 1, div_char);
    }
    
    return PM_E_NONE;
}

#define _WB_TBL_RIF_

ctclib_hash_t        *hsrv_wb_rif_hash = NULL;

static uint32
_tbl_rif_hash_make(tbl_rif_t *p_rif)
{
    uint32 index = 0;
    uint32 key = 0;
    uint8 *pval = NULL;

    pval = (uint8*)&p_rif->key;
    for (index = 0; index < sizeof(p_rif->key); index++)
    {
        key += pval[index];
    }

    return key;
}

static bool
_tbl_rif_hash_cmp(void *p_arg1, void *p_arg2)
{
    tbl_rif_t *p_rif1 = (tbl_rif_t*)p_arg1;
    tbl_rif_t *p_rif2 = (tbl_rif_t*)p_arg2;

    if (0 == sal_memcmp(&p_rif1->key, &p_rif2->key, sizeof(tbl_rif_key_t)))
    {
        return TRUE;
    }

    return FALSE;
}

int32
_hsrv_wb_sync_tbl_rif_fn(void *obj, tbl_iter_args_t *pargs)
{
    tbl_rif_t *p_db_rif = (tbl_rif_t*)obj;
    char *tbl_name = (char*)pargs->argv[0];
    char data[CMD_BUF_1024];
    sal_snprintf(data, CMD_BUF_1024, "/%s"CMD_EQUAL_STR"%s/%s"CMD_EQUAL_STR"%"PRIu64"/%s"CMD_EQUAL_STR"%"PRIu64,
        "key", p_db_rif->key.name, "rif_id", p_db_rif->rif_id,"hostif_id", p_db_rif->hostif_id);
    dbclient_lpush_binary(tbl_name, (uint8*)data, sal_strlen(data));

    return PM_E_NONE;
}

int32
hsrv_wb_sync_tbl_rif()
{
    cdb_node_t *p_db_node = cdb_get_tbl(TBL_RIF);
    tbl_iter_args_t args;
    char tbl_name[CMD_BUF_64];

    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = tbl_name;
    sal_snprintf(tbl_name, CMD_BUF_64, "CDB#%s", p_db_node->name);
    tbl_rif_iterate(_hsrv_wb_sync_tbl_rif_fn, &args);

    return PM_E_NONE;
}

int32
_hsrv_wb_restore_tbl_rif_fn(void *obj, uint32 len, void *aux)
{
    tbl_rif_t *p_db_rif = NULL;
    ctclib_hash_t *p_hash = (ctclib_hash_t *)aux;
    char *str = (char*)obj;
    void *p_ret = NULL;
    wb_parse_info_t parse;

    p_db_rif = XCALLOC(MEM_TBL_RIF, sizeof(tbl_rif_t));
    if (NULL == p_db_rif)
    {
        return PM_E_FAIL;
    }
    wb_get_parse(str, &parse);

    sal_strcpy(p_db_rif->key.name, parse.value[0]);
    sal_sscanf(parse.value[1], "%"PRIu64, &p_db_rif->rif_id);
    sal_sscanf(parse.value[2], "%"PRIu64, &p_db_rif->hostif_id);
    p_ret = ctclib_hash_get(p_hash, (void*)p_db_rif, ctclib_hash_alloc_intern);
    if (NULL == p_ret)
    {
        XFREE(MEM_TBL_RIF, p_db_rif);
        return PM_E_NO_MEMORY;
    }
    
    return PM_E_NONE;
}

int32
hsrv_wb_restore_tbl_rif()
{
    cdb_node_t *p_db_node = cdb_get_tbl(TBL_RIF);
    tbl_iter_args_t args;
    char tbl_name[CMD_BUF_64];

    if (NULL == hsrv_wb_rif_hash)
    {
        hsrv_wb_rif_hash = ctclib_hash_create(_tbl_rif_hash_make, _tbl_rif_hash_cmp);
    }
    
    sal_memset(&args, 0, sizeof(args));
    args.argv[0] = tbl_name;
    sal_snprintf(tbl_name, CMD_BUF_64, "CDB#%s", p_db_node->name);
    dbclient_lrange_handle(tbl_name, LRANGE_MIN_INDEX, LRANGE_MAX_INDEX, _hsrv_wb_restore_tbl_rif_fn, hsrv_wb_rif_hash);

    return PM_E_NONE;
}

tbl_rif_t*
hsrv_wb_tbl_rif_get_rif(tbl_rif_key_t *p_rif_key)
{
    tbl_rif_t lkp;

    if (NULL == hsrv_wb_rif_hash)
    {
        return NULL;
    }
    
    sal_memcpy(&lkp.key, p_rif_key, sizeof(tbl_rif_key_t));
    return ctclib_hash_lookup(hsrv_wb_rif_hash, &lkp);
}

int32
hsrv_wb_sync()
{
#ifdef USW
    uint8 lchip = 0;
#endif
    log_sys(M_MOD_SAI, E_ERROR, "hsrv_wb_sync()");
    
    /* 1. connect DB */
    dbclient_init();
    dbclient_flushdb();

    /* 2. CDB/SDB sync */
    hsrv_wb_sync_tbl_rif();

    /* 3. SDK Sync */
#ifdef USW
    ctc_wb_sync(lchip);
    ctc_wb_sync_done(lchip, 0);
#else
    ctc_wb_sync();
    ctc_wb_sync_done();
#endif
    return PM_E_NONE;
}

int32
hsrv_wb_sync_done()
{
    log_sys(M_MOD_SAI, E_ERROR, "hsrv_wb_sync_done()");
    
    dbclient_save();
    dbclient_deinit();
    return PM_E_NONE;
}

int32
hsrv_wb_sdb_restore()
{
    log_sys(M_MOD_HSRV, E_ERROR, "hsrv_wb_sdb_restore()");
    
    /* 1. get data */
    dbclient_init();
    
    hsrv_wb_restore_tbl_rif();
    
    return PM_E_NONE;
}

int32
hsrv_wb_sdb_restore_done()
{
    log_sys(M_MOD_HSRV, E_ERROR, "hsrv_wb_sdb_restore_done()");
    
    /* 1. free data */
    if (NULL != hsrv_wb_rif_hash)
    {
        ctclib_hash_free(hsrv_wb_rif_hash);
        hsrv_wb_rif_hash = NULL;
    }

    ctc_sai_set_wb_restoring(FALSE);
    
    dbclient_deinit();
    return PM_E_NONE;
}


static int32
_hsrv_wb_restoring_file_exist()
{
    struct stat stat_buf;
    
    if (!stat(WB_RESTORING_FILE, &stat_buf))
    {
        return 1;
    }
    return 0;
}

int32
hsrv_wb_do_reboot()
{
    hsrv_wb_sync();
    hsrv_wb_sync_done();

    log_sys(M_MOD_HSRV, E_ERROR, "ctc_sdk_deinit()");
    ctc_sdk_deinit();
    log_sys(M_MOD_HSRV, E_ERROR, "ctc_vty_close()"); 
    ctc_vty_close();
    log_sys(M_MOD_HSRV, E_ERROR, "exit()");
    exit(0);

    return 0;
}

int32
hsrv_wb_init()
{
#ifdef CONFIG_WARMBOOT
    uint32 restoring = FALSE;
    
    hsrv_wb_cmd_init();

    ctc_sai_set_wb_en(TRUE);
    restoring = _hsrv_wb_restoring_file_exist();
    if (restoring)
    {
        log_sys(M_MOD_HSRV, E_ERROR, "Warmboot restoring is true");
        ctc_sai_set_wb_restoring(restoring);

        hsrv_wb_sdb_restore();
    }
#endif

    return PM_E_NONE;
}

