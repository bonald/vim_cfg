
#include <openssl/bn.h>
#include <assert.h>

#include "proto.h"
#include "gen/tbl_cem_define.h"
#include "gen/tbl_cem.h"
#include "appcfg.h"
#include "app_cem.h"
#include "lib_fs.h"


static int32
_app_cem_cmd_get_cem(cfg_cmd_para_t *para, cdb_node_t* p_node, field_parse_t *p_field)
{
    FILE *fp = NULL;
    tbl_iter_args_t args;

    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_cem_iterate((TBL_ITER_CB_FUNC)tbl_cem_dump_one, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_app_cem_cmd_set_event(char **argv, tbl_cem_t *p_cem, tbl_cem_t *p_db_cem, cfg_cmd_para_t *para)
{
    int32 rc = 0;
    /* 1. config check */
    if(0 == sal_strcmp(p_cem->event, "cpu") || 0 == sal_strcmp(p_cem->event, "memory"))
    {
        p_cem->threshold = cdb_int_str2val(argv[3], &rc);
        if(NULL != p_db_cem)
        {
            if(sal_strcmp(p_cem->event,p_db_cem->event))
            {
                if(0 == p_cem->threshold)
                {
                    return rc;
                }
            }
        }
        if(rc < 0)
        {
            return rc;
        }
    }
    else
    {
        if(0 == sal_strcmp(argv[3], "0") )
        {
            if(NULL != p_db_cem)
            {
                if(sal_strcmp(p_cem->event,p_db_cem->event))
                {
                    if(0 == p_cem->threshold)
                    {
                        return rc;
                    }
                } 
            }
           sal_memset(p_cem->event,0,CEM_NAME_SIZE); 
        }
    }
    /* 2. call API */
    PM_E_RETURN(app_cem_set_event(p_cem));
    return rc;
}

static int32
_app_cem_cmd_set_loadpath(char **argv, tbl_cem_t *p_cem, tbl_cem_t *p_db_cem, cfg_cmd_para_t *para)
{
    int32 rc = 0;
    /* 1. config check */
    if(NULL != p_db_cem)
    {
        if(0 == sal_memcmp(p_cem->loadpath, p_db_cem->loadpath, COMM_NAME_SIZE))
        {
            return rc;
        }
        
        if(0 == sal_strcmp(p_cem->loadpath,"none")) 
        {
            sal_memset(p_cem->loadpath, 0, COMM_NAME_SIZE);
        }
    }
    /* 2. call API */
    PM_E_RETURN(app_cem_set_loadpath(p_cem));
    return rc;
}

static int32
_app_cem_cmd_set_snmptrap(char **argv, tbl_cem_t *p_cem, tbl_cem_t *p_db_cem, cfg_cmd_para_t *para)
{
    int32 rc = 0;
    /* 1. config */

     p_cem->snmptrap = cdb_int_str2val(argv[2], &rc);

    /* 2. call API */
    PM_E_RETURN(app_cem_set_snmptrap(p_cem));
    return rc;
}
static int32
_app_cem_cmd_set_mail(char **argv, tbl_cem_t *p_cem, tbl_cem_t *p_db_cem, cfg_cmd_para_t *para)
{
    int32 rc = 0;
    int i = 2;

     /* 1. config check */
    if(0 == sal_strcmp(argv[i], "none")) 
    {
        PM_E_RETURN(app_cem_set_mail(p_cem));
    }
    else
    {
        if(0 == sal_strcmp(argv[i], "from"))
        {   
            i++;
            if(sal_strcmp(argv[i], ""))
            {
                sal_strcpy(p_cem->mail.from, argv[i]);
            }
            i++;
        }
        if(0 == sal_strcmp(argv[i], "to"))
        {   
            i++;
            if(sal_strcmp(argv[i], ""))
            {
                sal_strcpy(p_cem->mail.to, argv[i]);
            }
            i++;
        }
        if(0 == sal_strcmp(argv[i], "subject"))
        {   
            i++;
            if(sal_strcmp(argv[i], ""))
            {
                sal_strcpy(p_cem->mail.subject, argv[i]);
            }
            i++;
        }
        if(0 == sal_strcmp(argv[i], "cc"))
        {   
            i++;
            if(sal_strcmp(argv[i], ""))
            {
                sal_strcpy(p_cem->mail.cc, argv[i]);
            }
            i++;
        }
        if(0 == sal_strcmp(argv[i], "body"))
        {   
            i++;
            if(sal_strcmp(argv[i], ""))
            {
                sal_strcpy(p_cem->mail.body, argv[i]);
            }
            i++;
        }
        if(0 == sal_strcmp(argv[i], "server"))
        {   
            i++;
            if(sal_strcmp(argv[i], ""))
            {
                sal_strcpy(p_cem->mail.server, argv[i]);
            }
            i++;
        }
        if(0 == sal_strcmp(argv[i], "attach"))
        {   
            i++;
            if(sal_strcmp(argv[i], "none"))
            {
                sal_strcpy(p_cem->mail.attach, argv[i]);
            }
            i++;
        } 
        /* 2. call API */
        PM_E_RETURN(app_cem_set_mail(p_cem));
    }

    return rc;
}
int32
app_event_manager_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    tbl_cem_t event_manager;
    tbl_cem_t *p_db_event_manager = NULL;
    int32 field_id = 0;
    cdb_field_t *p_field = NULL;
    int32 rc = PM_E_NONE;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_CEM);
    
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        
        sal_memset(&event_manager, 0, sizeof(event_manager));
        sal_strncpy(event_manager.key.name, argv[0], COMM_NAME_SIZE);
        
        rc = cdb_check_name(argv[0], COMM_NAME_SIZE - 1, para->p_rs->desc);

        if (rc < 0) 
        {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
        /* check exist */
        p_db_event_manager = tbl_cem_get_cem(&event_manager.key);

        if(p_db_event_manager)
        {
           return PM_E_NONE;
        }

        PM_CFG_CHECK_EXPECT_PARAMS(argc, 1, para->p_rs);

        rc = _app_event_manager_add_cfg(&event_manager);
        break;

    case CDB_OPER_DEL:
        
        sal_memset(&event_manager, 0, sizeof(event_manager));
        sal_strncpy(event_manager.key.name, argv[0], COMM_NAME_SIZE);
        
        rc = cdb_check_name(argv[0], COMM_NAME_SIZE - 1, para->p_rs->desc);

        if (rc < 0) 
        {
            para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
            return rc;
        }
        /* check exist */
        p_db_event_manager = tbl_cem_get_cem(&event_manager.key);

        if(NULL == p_db_event_manager)
        {
           CFG_CONFLICT_RET("Event Manager Entry %s not found", event_manager.key.name);
        }
        rc = _app_event_manager_del_cfg(&event_manager.key);
        break;

    case CDB_OPER_SET:       
        sal_memset(&event_manager, 0, sizeof(event_manager));
        sal_strncpy(event_manager.key.name, argv[0], COMM_NAME_SIZE);

        /* check exist */
        p_db_event_manager = tbl_cem_get_cem(&event_manager.key);
        if (!p_db_event_manager)
        {
            CFG_CONFLICT_RET("Event Manager Entry %s not found", event_manager.key.name);
        }
        
        field_id = cdb_get_field_id(argv[1], p_node, &p_field);
        if (field_id < 0)
        {
            CFG_INVALID_PARAM_RET("Unknown field %s", argv[1]);
        }
        if (CDB_ACCESS_RD == p_field->access)
        {
            CFG_INVALID_PARAM_RET("Field %s is read-only", p_field->name);
        }
        switch (field_id)
        {
            case TBL_CEM_FLD_EVENT:
                rc = cdb_check_name(argv[2], CEM_NAME_SIZE - 1, para->p_rs->desc);

                if (rc < 0) 
                {
                    para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
                    return rc;
                }
                sal_strncpy(event_manager.event, argv[2], CEM_NAME_SIZE);
                
                rc = _app_cem_cmd_set_event(argv, &event_manager, p_db_event_manager, para);
                break;

            case TBL_CEM_FLD_LOADPATH:
                
                sal_strncpy(event_manager.loadpath, argv[2], CEM_NAME_SIZE);
                rc = _app_cem_cmd_set_loadpath(argv, &event_manager, p_db_event_manager, para);
                break;
                
            case TBL_CEM_FLD_MAIL:
                if(sal_strcmp(argv[2], "none")) 
                {
                    PM_CFG_CHECK_EXPECT_PARAMS(argc, 16, para->p_rs);
                }
                rc = _app_cem_cmd_set_mail(argv, &event_manager, p_db_event_manager, para);
                break;
                
            case TBL_CEM_FLD_SNMPTRAP:
                
                rc = _app_cem_cmd_set_snmptrap(argv, &event_manager, p_db_event_manager, para);
                break;   
            
        }
        break;
    case CDB_OPER_GET:
        #if 1
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _app_cem_cmd_get_cem(para, p_node, &field);
        #endif
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}


