
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include "proto.h"
#include "dhclient_func.h"
#include "gen/tbl_dhclient_define.h"
#include "gen/tbl_dhclient.h"
#include "gen/tbl_dhcsrvgrp_define.h"
#include "gen/tbl_dhcsrvgrp.h"
#include "gen/tbl_dhcp_debug_define.h"
#include "gen/tbl_dhcp_debug.h"
#include "gen/tbl_route_if_define.h"
#include "gen/tbl_route_if.h"
#include "param_check.h"
#include "lib_fs.h"

extern int add_agent_options;
extern int g_dhcprelay_trustall;
extern int agent_relay_mode;
extern int drop_agent_mismatches;

extern void reset_dhclient_statistics();
extern void output_dhclient_statistics(FILE *fp);

#define CONSOLE_DEBUG_OUT(fmt, args...)                      \
{                                                            \
    FILE * fp_console = NULL;                                \
    fp_console = fopen("/dev/console", "w+");                \
    fprintf(fp_console, fmt"\n", ##args);                    \
    fclose(fp_console);                                      \
}

DEBUG_DECLAR(dhcp_debug);

/* --- dhclient --- */
static int
_check_dhclient_field(char *pszkey, char *pszValue, tbl_dhclient_t *pitem, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = cdb_get_tbl(TBL_DHCLIENT);
    cdb_field_t *p_field = NULL;
    int32 field_id;

    field_id = cdb_get_field_id(pszkey, p_node, &p_field);
    if (field_id < 0) {
        snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Unknown field %s", pszkey); 
        goto err_out;
    }

    switch ((tbl_dhclient_field_id_t)field_id) {
    case TBL_DHCLIENT_FLD_BROADCAST:
         if (strcmp(pszValue, "0") && strcmp(pszValue, "1")) {
            snprintf(para->p_rs->desc, MAX_CMD_RET_LEN, "Invalid enable value"); 
            goto err_out;
        }
        pitem->broadcast = atoi(pszValue);
        break;
    case TBL_DHCLIENT_FLD_DISTANCE:
        pitem->distance = atoi(pszValue);
        break;
    default:
        break;
    }
    
    return field_id;

err_out:
    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
    return -1;
}

static int32
_dhclient_cmd_get(tbl_dhclient_t *pitem, cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_dhclient_dump_one(pitem, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

int32
dhclient_set(tbl_dhclient_field_id_t field_id, tbl_dhclient_t *pitem)
{
    int32 rc = PM_E_NONE;
    
    /* 1. process */
    switch (field_id) {
    case TBL_DHCLIENT_FLD_DISTANCE:
    case TBL_DHCLIENT_FLD_BROADCAST:
        break;
    default:
        return PM_E_NONE;
    }

    /* 2. update local CDB */
    PM_E_RETURN(tbl_dhclient_set_dhclient_field(pitem, field_id));


    return rc;
}


static int32
_dhclient_cmd_set(tbl_dhclient_field_id_t field_id, tbl_dhclient_t *pitem, 
    tbl_dhclient_t *p_db_item, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    switch (field_id) {
    case TBL_DHCLIENT_FLD_DISTANCE:
        if (pitem->distance == p_db_item->distance) {
            return PM_E_NONE;
        }

        g_dhc_ctl_blk.distance = pitem->distance;
        break;
    case TBL_DHCLIENT_FLD_BROADCAST:
        if (pitem->broadcast == p_db_item->broadcast) {
            return PM_E_NONE;
        }

        if (pitem->broadcast) {
            g_dhc_ctl_blk.flags |= DHCLIENT_BROADCAST_FLAG;
        } else {
            g_dhc_ctl_blk.flags &= ~DHCLIENT_BROADCAST_FLAG;
        }
        break;
    default:
        break;
    }

    PM_E_RETURN(dhclient_set(field_id, pitem));

    return rc;
}

int32
_dhclient_interface_cfg(tbl_route_if_t *p_intf, tbl_iter_args_t *pargs)
{
#if 0
    FILE *fp = (FILE *)pargs;

    if (NULL == p_intf) {
        return -1;
    }

    if (p_intf->dhcp_server_group || p_intf->dhcp_relay_option_trust) {
        fprintf(fp, "/key=%s/dhcp_relay_option_trust=%u/dhcp_server_group=%u\r\n", 
            p_intf->key.name, p_intf->dhcp_relay_option_trust,
            p_intf->dhcp_server_group);
    }
#endif
    return PM_E_NONE;
}

static int32
_dhclient_get_config(cfg_cmd_para_t *para)
{
#if 0
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    /* output interface config */
    tbl_route_if_iterate((TBL_ITER_CB_FUNC)_dhclient_interface_cfg, (tbl_iter_args_t *)fp);
    sal_fclose(fp);
#endif
    return PM_E_NONE;
}

extern void output_dhcrelay_statistics(FILE *fp);
static int32
_dhclient_get_statistics(cfg_cmd_para_t *para)
{
    FILE *fp = NULL;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    output_dhclient_statistics(fp);
    sal_fclose(fp);
    return PM_E_NONE;
}

int32
dhclient_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;

    cdb_node_t* p_node = NULL;
    tbl_dhclient_field_id_t field_id = TBL_DHCLIENT_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    tbl_dhclient_t item;
    tbl_dhclient_t *p_db_item = NULL;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_DHCLIENT);    
    p_db_item = tbl_dhclient_get_dhclient();
    switch (para->oper) {
    case CDB_OPER_ADD:
        break;
    case CDB_OPER_DEL:
        if (!sal_strcmp(argv[0], "statistics") && 1 == argc) {
            reset_dhclient_statistics();
        } 
        break;

    case CDB_OPER_SET:
        if ((argc % 2) != 0) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        for (i = 0; i < argc; i += 2) {
            if (_check_dhclient_field(argv[i], argv[i + 1], &item, para) == -1) {
                return PM_E_CFG_INVALID_PARAM;
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access) {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 0; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            rc |= _dhclient_cmd_set(field_id, &item, p_db_item, para);
        }
        break;

    case CDB_OPER_GET:
        if (1 == argc && (!strcasecmp(argv[0], "interfaces") 
                           || !strcasecmp(argv[0], "trusted"))) {
            rc = _dhclient_get_config(para);
            break;
        } else if (1 == argc && !strcasecmp(argv[0], "statistics")) {
            rc = _dhclient_get_statistics(para);
            break;
        }

        if (1 == argc && !strcasecmp(argv[0], "config")) {
            sal_memset(&field, FLD_ID_ALL, sizeof(field_parse_t));
        } else {    
            rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
            if (rc < 0) {
                CFG_INVALID_PARAM_RET("Invalid field");
            }
        }

        rc = _dhclient_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }

    return rc;
}

int32
dhclient_port_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    int32 rc = PM_E_NONE;
#if 0
    cdb_node_t* p_node = NULL;
    tbl_dhcsrvgrp_field_id_t field_id = TBL_DHCSRVGRP_FLD_MAX;
    cdb_field_t *p_field = NULL;
    int32 i = 0;
    field_parse_t field;
    tbl_dhcsrvgrp_t item;
    tbl_dhcsrvgrp_t *p_db_item = NULL;
    int nId;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_DHCSRVGRP);  

    if (0 == argc) {
        if (CDB_OPER_GET != para->oper) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
    } else {
        nId = atoi(argv[0]);
        if (strspn(argv[0], "0123456789") != strlen(argv[0]) 
                || (nId < 0 || nId >= GLB_MAX_DHCSRVGRP_NUM)) {
            CFG_INVALID_PARAM_RET("Invalid dhcp server group index: %s", argv[0]);
        }
    
        sal_memset(&item, 0, sizeof(tbl_dhcsrvgrp_t));
        item.key.id = nId;
        p_db_item = tbl_dhcsrvgrp_get_dhcsrvgrp(&item.key);
        if (NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Internal error: can't found dhcp server group %s", argv[0]);
        }
    }
    
    switch (para->oper) {
    case CDB_OPER_ADD:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    case CDB_OPER_DEL:
    case CDB_OPER_SET:
        if (2 == argc) {
            if (!strcasecmp(argv[1], "addrs")) { 
                field_id = TBL_DHCSRVGRP_FLD_ADDRS;
                memset(item.addrs, 0x00, DHCP_SRV_ADDRS_BUF_SIZE); 
                item.cnt = 0;
            } else {
                CFG_INVALID_PARAM_RET("Invalid parameters");
            }
            rc = _dhcsrvgrp_cmd_set(field_id, &item, p_db_item, para);
            return rc;
        }
    
        if ((argc % 2) != 1) {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }

        for (i = 1; i < argc; i += 2) {
            if (_check_dhcsrvgrp_field(argv[i], argv[i + 1], 
                    &item, para, p_db_item, (CDB_OPER_DEL == para->oper)) == -1) {
                return PM_E_CFG_INVALID_PARAM;
            }

            cdb_get_field_id(argv[i], p_node, &p_field);
            if (CDB_ACCESS_RD == p_field->access)
            {
                CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
            }
        }

        for (i = 1; i < argc; i += 2) {
            field_id = cdb_get_field_id(argv[i], p_node, &p_field);
            rc |= _dhcsrvgrp_cmd_set(field_id, &item, p_db_item, para);
        }
        break;

    case CDB_OPER_GET:
        if (argc > 0 && NULL == p_db_item) {
            CFG_INVALID_PARAM_RET("Dhcp server group %s not found", argv[0]);
        }
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, TRUE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _dhcsrvgrp_cmd_get(p_db_item, para, p_node, &field);    
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
#endif    
    return rc;
}

int32
dhclient_tbl_init()
{
    tbl_dhclient_t dhclient;
    
    /* -- dhclient -- */
    memset(&dhclient, 0x00, sizeof(tbl_dhclient_t));
    dhclient.distance = DHCLIENT_DEFAULT_ROUTER_DISTANCE;
    dhclient.broadcast = DHCLIENT_DEFAULT_FLAGS;
    
    tbl_dhclient_set_dhclient_field(&dhclient, TBL_DHCLIENT_FLD_DISTANCE);
    tbl_dhclient_set_dhclient_field(&dhclient, TBL_DHCLIENT_FLD_BROADCAST);

    return PM_E_NONE;
}

int32
dhclient_proc_startup_done(void)
{
    return PM_E_NONE;
}

int32
dhclient_proc_init_done(void)
{
    /* Modified by kcao for bug 40062, should init CDB table after connection to CDS is ready */
    dhclient_tbl_init();
    
    return PM_E_NONE;
}

