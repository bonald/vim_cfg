
#include "proto.h"
#include "gen/tbl_clock_define.h"
#include "gen/tbl_clock.h"
#include "app_clock.h"
#include "param_check.h"

#define MAX_PROC_LEN            128

/*process control structure*/
typedef struct tagProcCtrl
{
    char strPIDFile[MAX_PROC_LEN + 1];
    int signal;
}S_ProcCtrl;

S_ProcCtrl  g_stProcCtrl[] =
{
    /*{"/var/run/ntpd.pid", SIGHUP},*/
    {"/var/run/syslog-ng.pid", SIGUSR2}
};

int 
app_clock_notify_other_application()
{
    int nPid;   /*process index*/ 

    for (nPid = 0; nPid < (sizeof(g_stProcCtrl)/sizeof(g_stProcCtrl[0])); nPid++)
    {
        signal_app((const char*)g_stProcCtrl[nPid].strPIDFile, g_stProcCtrl[nPid].signal);
    }

    return 0;
}

int32
app_clock_write_profile_timezone(char *strZoneInfo)
{
    FILE *fp = NULL;
    
    if (NULL == strZoneInfo)
    {
        return PM_E_FAIL;
    }

    /*open timezone profile*/
    fp = sal_fopen(ZONE_CONFIG_PATH, SAL_OPEN_RW);
    if (NULL == fp)
    {
        return -1;
    }

    /*write timezone profile*/
    sal_fputs("#!/bin/bash\n", fp);
    sal_fputs("TZ=", fp);
    sal_fputs(strZoneInfo, fp);
    sal_fputs("\n", fp);
    sal_fputs("export TZ", fp);

    /*close timezone profile*/
    sal_fclose(fp);

    chmod(ZONE_CONFIG_PATH, (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH));

    return 0;
}

int32
_app_clock_set_timezone(tbl_clock_t *p_clk, uint32 is_add)
{
    char strZone[CMD_BUF_256];
    int32 rc = PM_E_NONE;
    
    sal_memset(strZone, 0, sizeof(strZone));
    
    if (is_add)
    {
        if (p_clk->timezone_positive) 
        {
            sal_sprintf(strZone, "%s-%02u:%02u:%02u", p_clk->timezone_name, 
                p_clk->timezone_hour, p_clk->timezone_minute, p_clk->timezone_second);
        }
        else
        {
            /*negative time zone from UTC*/
            sal_sprintf(strZone, "%s+%02u:%02u:%02u", p_clk->timezone_name, 
                p_clk->timezone_hour, p_clk->timezone_minute, p_clk->timezone_second);
        }
    }
    else
    {
        sal_sprintf(strZone, "%s", "UTC");         
    }  

    rc = setenv("TZ", strZone, 1);    
    if (rc < 0)
    {
        log_sys(M_MOD_LIB, E_ERROR, "Put env failed! ");
        return -1;
    }
    tzset();

    /*syschoronize system time to hardware*/            
    //synchro_time_to_harware ();      
    
    /*must be put before inform other process, write to zone config*/
    app_clock_write_profile_timezone(strZone);

    app_clock_notify_other_application();

    return rc;
}

int32
app_clock_add_timezone(tbl_clock_t *p_clk)
{
    char offset_str[CMD_BUF_32];

    sal_memset(offset_str, 0, sizeof(offset_str));
    sal_snprintf(offset_str, CMD_BUF_32, "%02u:%02u:%02u", p_clk->timezone_hour, p_clk->timezone_minute, p_clk->timezone_second);
    logid_sys(LOG_SYSTEM_4_TIMEZONE_SET, p_clk->timezone_name, p_clk->timezone_positive ? "add" : "minus", offset_str);
    sal_task_sleep(100);
    
    _app_clock_set_timezone(p_clk, TRUE);

    /* store DB */
    tbl_clock_set_clock_field(p_clk, TBL_CLOCK_FLD_TIMEZONE_POSITIVE);
    tbl_clock_set_clock_field(p_clk, TBL_CLOCK_FLD_TIMEZONE_HOUR);
    tbl_clock_set_clock_field(p_clk, TBL_CLOCK_FLD_TIMEZONE_MINUTE);
    tbl_clock_set_clock_field(p_clk, TBL_CLOCK_FLD_TIMEZONE_SECOND);
    /* Should be last because we want to use this field as a notification of timezone field update finished. */
    tbl_clock_set_clock_field(p_clk, TBL_CLOCK_FLD_TIMEZONE_NAME);

    return PM_E_NONE;
}

int32
app_clock_del_timezone(tbl_clock_t *p_clk, uint32 init_done)
{
    tbl_clock_t *p_db_clk = tbl_clock_get_clock();
    char offset_str[CMD_BUF_32];

    /* Added by kcao for bug 50567, clear timezone to UTC when system init down */
    if (!init_done)
    {
        if (0 == sal_strlen(p_db_clk->timezone_name))
        {
            return PM_E_NONE;
        }

        sal_memset(offset_str, 0, sizeof(offset_str));
        sal_snprintf(offset_str, CMD_BUF_32, "%02u:%02u:%02u", p_db_clk->timezone_hour, p_db_clk->timezone_minute, p_db_clk->timezone_second);
        logid_sys(LOG_SYSTEM_4_TIMEZONE_CLEAR, p_db_clk->timezone_name, p_db_clk->timezone_positive ? "add" : "minus", offset_str);

    }
    
    sal_task_sleep(100);
    
    _app_clock_set_timezone(p_clk, FALSE);
            
    /* store DB */
    tbl_clock_set_clock_field(p_clk, TBL_CLOCK_FLD_TIMEZONE_POSITIVE);
    tbl_clock_set_clock_field(p_clk, TBL_CLOCK_FLD_TIMEZONE_HOUR);
    tbl_clock_set_clock_field(p_clk, TBL_CLOCK_FLD_TIMEZONE_MINUTE);
    tbl_clock_set_clock_field(p_clk, TBL_CLOCK_FLD_TIMEZONE_SECOND);
    /* Should be last because we want to use this field as a notification of timezone field update finished. */
    tbl_clock_set_clock_field(p_clk, TBL_CLOCK_FLD_CP_TIMEZONE_NAME);
    tbl_clock_set_clock_field(p_clk, TBL_CLOCK_FLD_CP_TIMEZONE_POSITIVE);
    tbl_clock_set_clock_field(p_clk, TBL_CLOCK_FLD_CP_TIMEZONE_HOUR);
    tbl_clock_set_clock_field(p_clk, TBL_CLOCK_FLD_CP_TIMEZONE_MINUTE);
    tbl_clock_set_clock_field(p_clk, TBL_CLOCK_FLD_CP_TIMEZONE_SECOND);
    /* Should be last because we want to use this field as a notification of timezone field update finished. */
    tbl_clock_set_clock_field(p_clk, TBL_CLOCK_FLD_TIMEZONE_NAME);
    
    return PM_E_NONE;
}

int32
app_clock_add_bj_timezone()
{
    tbl_clock_t clk;

    sal_memset(&clk, 0, sizeof(clk));
    sal_snprintf(clk.timezone_name, MAX_ZONE_NAME_LEN, "Beijing");
    clk.timezone_positive   = TRUE;
    clk.timezone_hour       = 8;
    clk.timezone_minute     = 0;
    clk.timezone_second     = 0;
    app_clock_add_timezone(&clk);

    return PM_E_NONE;
}

int32
app_clock_proc_init_done()
{
    tbl_clock_t clk;

    sal_memset(&clk, 0, sizeof(clk));
    app_clock_del_timezone(&clk, TRUE);
    return PM_E_NONE;
}
    
int32
app_clock_proc_startup_done()
{
    int32 rc = PM_E_NONE;
    tbl_clock_t  clk;
    tbl_clock_t *p_db_clk = tbl_clock_get_clock();
    /* modified by liwh for bug 49670, 2018-11-16 */
    sal_time_t date;
    struct tm *p_local_time = NULL;
    /* liwh end */

    if (0 == sal_strlen(p_db_clk->timezone_name))
    {
        sal_memset(&clk, 0, sizeof(clk));
        rc = _app_clock_set_timezone(&clk, FALSE);
    }      

    /* modified by liwh for bug 49670, 2018-11-16 */        
    date = sal_time(NULL);
    p_local_time = sal_localtime(&date);
    if ((p_local_time->tm_year + 1900) < 2000) 
    {
        system("cdbctl update/cdb/sys/clock/00:00:00/1/1/2000");
        system("cdbctl update/cdb/sys/clock/switch");
    }
    /* liwh end */
    
    return rc;
}


