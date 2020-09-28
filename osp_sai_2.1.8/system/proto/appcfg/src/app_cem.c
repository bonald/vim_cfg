
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "proto.h"

#include "gen/tbl_cem_define.h"
#include "gen/tbl_cem.h"
#include "appcfg.h"
#include "app_cem.h"



int32
_app_event_manager_add_cfg(tbl_cem_t *p_event_manager)
{
    int32 rc = PM_E_NONE;
   
    rc = tbl_cem_add_cem(p_event_manager);
        
    return rc;
}

int32
_app_event_manager_del_cfg(tbl_cem_key_t *p_event_manager_key)
{
    int32 rc = PM_E_NONE;
    
    rc = tbl_cem_del_cem(p_event_manager_key);
     
    return rc;
}

int32
app_cem_set_event(tbl_cem_t *p_event_manager)
{
    tbl_cem_key_t event_manager_key;
    tbl_cem_t *p_db_event_manager = NULL;
    int32 rc = PM_E_NONE;

    sal_strncpy(event_manager_key.name, p_event_manager->key.name, COMM_NAME_SIZE);
    p_db_event_manager = tbl_cem_get_cem(&event_manager_key);
    if(NULL != p_db_event_manager)
    {
        if(sal_strcmp(p_event_manager->event, p_db_event_manager->event))
        {
            rc = tbl_cem_set_cem_field(p_event_manager,TBL_CEM_FLD_EVENT);
            
            if(rc < 0)
            {
                return rc;
            }
        }

    }
    else
    {
        rc = tbl_cem_set_cem_field(p_event_manager,TBL_CEM_FLD_EVENT);
        if(rc < 0)
        {
            return rc;
        }
    }
    if(0 == sal_strcmp(p_event_manager->event, "cpu") ||
        0 == sal_strcmp(p_event_manager->event, "memory")) 
    {
        if(p_event_manager->threshold != p_db_event_manager->threshold)
        {
            rc = tbl_cem_set_cem_field(p_event_manager,TBL_CEM_FLD_THRESHOLD);
            if(rc < 0)
            {
                return rc;
            }
        }
        
        if(0 == p_event_manager->threshold)
        {            
            sal_memset(p_event_manager->event, 0, CEM_NAME_SIZE);
            rc = tbl_cem_set_cem_field(p_event_manager,TBL_CEM_FLD_EVENT);
            if(rc < 0)
            {
                return rc;
            }
        }
    }
    
    return rc;
}

int32
app_cem_set_snmptrap(tbl_cem_t *p_event_manager)
{
    tbl_cem_key_t event_manager_key;
    tbl_cem_t *p_db_event_manager = NULL;
    int32 rc = PM_E_NONE;

    sal_strncpy(event_manager_key.name, p_event_manager->key.name, COMM_NAME_SIZE);
    p_db_event_manager = tbl_cem_get_cem(&event_manager_key);
    if(NULL != p_db_event_manager)
    {
        if(p_event_manager->snmptrap != p_db_event_manager->snmptrap)
        {
            rc = tbl_cem_set_cem_field(p_event_manager,TBL_CEM_FLD_SNMPTRAP);
            
            if(rc < 0)
            {
                return rc;
            }
        }

    }
    else
    {
        rc = tbl_cem_set_cem_field(p_event_manager,TBL_CEM_FLD_SNMPTRAP);
        if(rc < 0)
        {
            return rc;
        }
    }    
    return rc;
}

int32
app_cem_set_loadpath(tbl_cem_t *p_event_manager)
{
    int32 rc = PM_E_NONE;

    rc = tbl_cem_set_cem_field(p_event_manager,TBL_CEM_FLD_LOADPATH);
      
    return rc;
}

int32
app_cem_set_mail(tbl_cem_t *p_event_manager)
{
    tbl_cem_key_t event_manager_key;
    tbl_cem_t *p_db_event_manager = NULL;
    int32 rc = PM_E_NONE;

    sal_strncpy(event_manager_key.name, p_event_manager->key.name, COMM_NAME_SIZE);
    p_db_event_manager = tbl_cem_get_cem(&event_manager_key);
    if(NULL != p_db_event_manager)
    {
        if(sal_memcmp(&p_event_manager->mail, &p_db_event_manager->mail, sizeof(mail_t)))
        {
            rc = tbl_cem_set_cem_field(p_event_manager,TBL_CEM_FLD_MAIL);
            
            if(rc < 0)
            {
                return rc;
            }
        }

    }
    else
    {
        rc = tbl_cem_set_cem_field(p_event_manager,TBL_CEM_FLD_MAIL);
        if(rc < 0)
        {
            return rc;
        }
    }    
    return rc;
}
int32
app_cem_init()
{
    system("reactorctl -t cem");
    return PM_E_NONE;
}

