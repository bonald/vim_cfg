
#include "proto.h"
#include "gen/tbl_clock_define.h"
#include "gen/tbl_clock.h"
#include "gen/tbl_tag_summer_clock_define.h"
#include "gen/tbl_tag_summer_clock.h"
#include "gen/tbl_chassis_define.h"
#include "gen/tbl_chassis.h"
#include "app_clock.h"
#include "glb_phy_define.h"
#include <time.h>

int _app_clock_check_timezone_name(const char *pszZonename)
{
    int nLen = 0;
    char *p = NULL;
    
    if (NULL == pszZonename)
    {
        return -1;
    }

    /* check the string length */
    nLen = sal_strlen(pszZonename);
    if (nLen < M_TIMEZONE_NAME_MINSIZE || nLen >= M_TIMEZONE_NAME_MAXSIZE)
    {
        return -1;
    }

    /*check the first character*/    
    if(!sal_char_isalpha(*pszZonename))
    {
        return -1;
    }

    /* check the chars */
    p = (char *)(pszZonename + 1);
    while ('\0' != *p)
    {
        if ((sal_isupper(*p)) || (sal_islower(*p)) || ('_' == *p))
        {
            p++;
            continue;
        }
        
        return -1;
    }

    /*check the last character*/
    p--;
    if(!sal_char_isalpha(*p))
    {
        return -1;
    }
    
    return PM_E_NONE;
}


int32
_app_summer_clock_get_which_from_str(char *tm_which_str, int32 *tm_which)
{
    if (NULL == tm_which_str || NULL == tm_which)
    {
        return PM_E_FAIL;
    }

    if (0 == sal_strcmp("first", tm_which_str))
    {
        *tm_which = FIRST_WDAY;
    }
    else if (0 == sal_strcmp("second", tm_which_str))
    {
        *tm_which = SECOND_WDAY;
    }
    else if (0 == sal_strcmp("third", tm_which_str))
    {
        *tm_which = THIRD_WDAY;
    }
    else if (0 == sal_strcmp("fourth", tm_which_str))
    {
        *tm_which = FOURTH_WDAY;
    }
    else if (0 == sal_strcmp("last", tm_which_str))
    {
        *tm_which = LAST_WDAY;
    }
    else
    {
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}
int32
_app_summer_clock_get_weekday_from_str(char *tm_wday_str, int32 *tm_wday)
{
    int32 i = 0;
    char wday_str_tmp[MAX_ZONE_NAME_LEN] = {0};
    
    if (NULL == tm_wday_str || NULL == tm_wday)
    {
        return PM_E_FAIL;
    }

    while(tm_wday_str[i] && i < MAX_ZONE_NAME_LEN)
    {
        wday_str_tmp[i] = tolower(tm_wday_str[i]);
        i++;
    }

    if (sal_strstr("monday", wday_str_tmp) && 0 == sal_strncmp("mon", wday_str_tmp, 3))
    {
        *tm_wday = MONDAY;
    }
    else if (sal_strstr("tuesday", wday_str_tmp) && 0 == sal_strncmp("tue", wday_str_tmp, 3))
    {
        *tm_wday = TUESDAY;
    }
    else if (sal_strstr("wednesday", wday_str_tmp) && 0 == sal_strncmp("wed", wday_str_tmp, 3))
    {
        *tm_wday = WEDNESDAY;
    }
    else if (sal_strstr("thursday", wday_str_tmp) && 0 == sal_strncmp("thu", wday_str_tmp, 3))
    {
        *tm_wday = THURSDAY;
    }
    else if (sal_strstr("friday", wday_str_tmp) && 0 == sal_strncmp("fri", wday_str_tmp, 3))
    {
        *tm_wday = FRIDAY;
    }
    else if (sal_strstr("saturday", wday_str_tmp) && 0 == sal_strncmp("sat", wday_str_tmp, 3))
    {
        *tm_wday = SATURDAY;
    }
    else if (sal_strstr("sunday", wday_str_tmp) && 0 == sal_strncmp("sun", wday_str_tmp, 3))
    {
        *tm_wday = SUNDAY;
    }
    else
    {
        return PM_E_FAIL;
    }

    return PM_E_NONE;
}

int
_app_summer_clock_make_time(summer_time_t *ptm, time_t *tmSec)
{
    struct tm tmCur;  /*current date*/
    
    if((NULL == ptm) || (NULL == tmSec))
    {
        return -1;   
    }    

    /*make time*/
    tmCur.tm_year = ptm->tm_year - 1900;
    tmCur.tm_mon = ptm->tm_mon - 1;
    tmCur.tm_mday = ptm->tm_mday;
    tmCur.tm_hour = ptm->tm_hour;
    tmCur.tm_min = ptm->tm_min;
    tmCur.tm_sec = ptm->tm_sec;

    *tmSec = sal_time_mk(&tmCur);

    return 0;
}

int 
_app_summer_clock_check_date(int *day, const int nMon, const int nYear, tbl_tag_summer_clock_t *tmCur, int flag)
{
    int nDay;
    int ret = 0;
    if (NULL == tmCur)
    {
       sal_printf("Can not find summer clock table");
       return -1;
    }
    if(NULL == day)
    {
        return -1;
    }

    /*get argment*/
    nDay = *day;  

    switch(nMon)
    {
        case 2:
        {
            if((flag == FLAG_DST)&& tmCur->is_recurring)
            {
                if((tmCur->nStartFebDay == 29) ||(tmCur->nEndFebDay == 29))                    
                {
                    if(ISLEAP(nYear))
                        nDay = 29;
                    else
                        nDay = 28;
                    break;
                }
            }
           
            if(ISLEAP(nYear))
            {
                if(nDay > 29)
                {
                    ret = -1;
                    nDay = 29;
                    break;
                }
            }
            else
            {
                if(nDay > 28)
                {
                    ret = -1;
                    nDay = 28;
                    break;
                }
                else
                    break;
            }
           
        }
        case 4:
        case 6:
        case 9:
        case 11:
        {
            if(nDay > 30)
            {
                ret = -1;
                nDay = 30;
                break;
            }
            else
                break;
        }
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
        if (nDay > 31)
        {
            ret = -1;
            nDay = 31;
        }
        break;
        default :
            break;
    }

    *day = nDay;   

    return ret;
}
int32
_app_summmer_trans_wday_time_to_localtime(which_summer_time_t *wday_tm, summer_time_t*tm)
{
    struct tm *ptmCur;  /* the first of month date in this year */
    summer_time_t tm_temp;
    time_t tmSec; 
    int32 month_first_day_wady = 0; /* day of the week int the first day of month in this year */
    int32 nYear = wday_tm->wday_tm_year;
    int32 nMon = wday_tm->wday_tm_mon;
    int32 nDay = 0; // get the day later
    int32 nWhich = wday_tm->wday_tm_which;
    int32 nWeekday = wday_tm->wday_tm_wday;
    int32 nHour = wday_tm->wday_tm_hour;
    int32 nMin = wday_tm->wday_tm_min;
    int32 nSec = wday_tm->wday_tm_sec;
    int32 nDay_temp;
    int32 ret;
    tbl_tag_summer_clock_t *tmCur = NULL;
    sal_memset(&tm_temp, 0, sizeof(tm_temp));
    /* set the 1st day of specify month */
    tm_temp.tm_year = nYear;
    tm_temp.tm_mon = nMon;
    tm_temp.tm_mday = 1;    // the first day of this month
    tm_temp.tm_hour = 0;
    tm_temp.tm_min = 0;
    tm_temp.tm_sec = 0; 
    _app_summer_clock_make_time(&tm_temp, &tmSec);
    ptmCur = sal_localtime(&tmSec);
    /* day of the week <1-7> 1:Monday 2:Tuesday ... */
    month_first_day_wady = ptmCur->tm_wday;
    nDay = 7 * (nWhich - (nWeekday < month_first_day_wady ? 0:1)) + (nWeekday - month_first_day_wady) + 1;
    /* get the current day */
    nDay_temp = nDay;
    tmCur = tbl_tag_summer_clock_get_tag_summer_clock();
    ret = _app_summer_clock_check_date(&nDay_temp, nMon, nYear,tmCur, FLAG_NO_DST);
    if(ret < 0)
    {
        nDay -= 7;
    }
    tm->tm_year = nYear;
    tm->tm_mon = nMon;
    tm->tm_mday = nDay;
    tm->tm_hour = nHour;
    tm->tm_min = nMin;
    tm->tm_sec = nSec; 
    return PM_E_NONE;
}

int32
_app_summer_clock_cal_dst_timer_interval(long int *interval)
{
    time_t date;
    time_t tmSec;  /*time make*/
    char strZoneInfo[MAX_TIME_LEN + 1];
    char strZone[MAX_ZONE_NAME_LEN + 1];    
    long int nlOffset;
    tbl_tag_summer_clock_t *p_db_summer_clk = NULL;
    if(NULL == interval)
    {
        return -1;
    }
    p_db_summer_clk = tbl_tag_summer_clock_get_tag_summer_clock();
    /*get current time and timezone*/
    date = sal_time_sys_current(NULL);
    /*get current timezone from TZ*/
    sal_memset(strZoneInfo, 0, sizeof(strZoneInfo));
    sal_memset(strZone, 0, sizeof(strZone));
    if(NULL == getenv("TZ"))
    {
        sal_strncpy(strZone, "UTC", sizeof(strZone));
        strZone[MAX_ZONE_NAME_LEN] = '\0';
    }
    else
    {
        sal_strncpy(strZoneInfo, getenv("TZ"), sizeof(strZoneInfo));
        strZoneInfo[MAX_TIME_LEN] = '\0'; 
        sal_sscanf(strZoneInfo, "%s", strZone);  
        strZone[MAX_ZONE_NAME_LEN] = '\0';  
    }
   
    /*judge whether is summer time*/
    if(!sal_strncmp(strZone, p_db_summer_clk->strZoneName, sal_strlen(p_db_summer_clk->strZoneName)))
    {
        /*current time is summer time, cal the time interval from current time to end summer time*/
        _app_summer_clock_make_time(&p_db_summer_clk->tmend, &tmSec);        
    }
    else
    {
        /*current time is NOT summer time, cal the time interval from current time to start summer time*/    
        if(!p_db_summer_clk->is_recurring)
        {
            /*if summer time is not recurring, judge whether end summer time is arrived*/
            _app_summer_clock_make_time(&p_db_summer_clk->tmend, &tmSec);
            if((tmSec - (date + p_db_summer_clk->iDstOffset * 60)) < 0)
            {
                *interval = 0;
                return PM_E_NONE;
            }
        }
        _app_summer_clock_make_time(&p_db_summer_clk->tmstart, &tmSec);        
    } 

    /*calculate interval*/
    nlOffset = tmSec - date;
    if(nlOffset > 24 * 3600)
    {
        *interval = 24 *3600;
    }
    else if(nlOffset > 3600)
    {
        *interval = 3600;
    }
    else if(nlOffset > 60)
    {
        *interval = 60;
    }
    else
    {
        *interval = 1;
    }
   
    return PM_E_NONE;
}
int32
 _app_summer_clock_change_dst_offset(int *minute, int *hour)
{
    long int nlTotalSec = 0;  
    tbl_tag_summer_clock_t *summer_clk = tbl_tag_summer_clock_get_tag_summer_clock();
    tbl_clock_t *p_db_clk = tbl_clock_get_clock();
    if((NULL == minute) || (NULL == hour))
    {
        return -1;
    }
    
    /*in tzset, west is positive, east is negistive, but in our command, 
     west in negtive and east is positive, so here we  need change it*/
    if( OP_POSITIVE == p_db_clk->timezone_positive)
    {
        nlTotalSec = p_db_clk->timezone_hour * 3600 + p_db_clk->timezone_minute * 60 + p_db_clk->timezone_second
                  + summer_clk->iDstOffset * 60;  

        if (nlTotalSec >= 24 * 3600)
        {
             /*should change from east to west*/
             nlTotalSec -= 24 *3600;
             *hour = -(nlTotalSec / 3600);
             *minute = (nlTotalSec / 60)%60;
        }
        else
        {
            *hour = (nlTotalSec / 3600);  
            *minute = (nlTotalSec / 60)%60;
        }
    }
    else
    {
        nlTotalSec = (-p_db_clk->timezone_hour) * 3600 + (-p_db_clk->timezone_minute) * 60 + (-p_db_clk->timezone_second)
                  + summer_clk->iDstOffset * 60; 
        *hour = nlTotalSec / 3600;  
        *minute = (nlTotalSec > 0) ? ((nlTotalSec / 60)%60) : ((-nlTotalSec / 60)%60);
    } 

    return PM_E_NONE;
}

int
_app_summer_clock_set_timezone_dst()
{
    char strZone[MAX_TIME_LEN + 1]; 
    int nHour, nMin;
    int ret;
    tbl_tag_summer_clock_t *p_db_summer_clk = NULL;
    tbl_clock_t *p_db_clk = tbl_clock_get_clock();
    sal_memset(strZone, 0, sizeof(strZone));  
    _app_summer_clock_change_dst_offset(&nMin, &nHour);            
    p_db_summer_clk = tbl_tag_summer_clock_get_tag_summer_clock();
    p_db_clk->cp_timezone_positive = p_db_clk->timezone_positive;
    /*for bug 24023 accord glibc limit for TZ modified by liuyang 2013-10-23*/
    if (nHour >= 0)
    {
        sal_sprintf(strZone, "%s-%02u:%02u:%u%u", p_db_summer_clk->strZoneName, nHour, nMin,0,0);
        p_db_clk->timezone_positive = 1;
    }
    else
    {
        sal_sprintf(strZone, "%s+%02u:%02u:%u%u", p_db_summer_clk->strZoneName, (-nHour), nMin,0,0);
        p_db_clk->timezone_positive = 0;
    }
    p_db_clk->cp_timezone_hour = p_db_clk->timezone_hour;
    p_db_clk->cp_timezone_minute = p_db_clk->timezone_minute;
    p_db_clk->cp_timezone_second = p_db_clk->timezone_second;
    sal_strncpy(p_db_clk->cp_timezone_name,p_db_clk->timezone_name,strlen(p_db_clk->timezone_name));
    tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_CP_TIMEZONE_POSITIVE);
    tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_CP_TIMEZONE_HOUR);
    tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_CP_TIMEZONE_MINUTE);
    tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_CP_TIMEZONE_SECOND);
    tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_CP_TIMEZONE_NAME);
    strZone[MAX_TIME_LEN] = '\0';
    ret = setenv("TZ", strZone, 1);
    if(ret < 0)
    {
        log_sys(M_MOD_LIB, E_ERROR, "Put env failed! ");
        return -1;
    }
    tzset();   
    /*write to zone config*/
    app_clock_write_profile_timezone(strZone);
     /*inform other process*/
    app_clock_notify_other_application();
    return PM_E_NONE;   
}
int32
app_set_tag_summer_clock(tbl_tag_summer_clock_t *p_summer_clk)
{
    if (NULL == p_summer_clk)
    {
        return PM_E_NONE;
    }
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_STRZONENAME);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_IDSTOFFSET);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_WDAY_TM_START);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_WDAY_TM_END);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_TMSTART);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_TMEND);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_IS_RECURRING);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_IS_WDAY_TM);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_NSTARTFEBDAY);
    tbl_tag_summer_clock_set_tag_summer_clock_field(p_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_NENDFEBDAY);
    return PM_E_NONE;
}

void
_app_summer_clock_dst_timer_alarm()
{
    long int interval;
    sal_time_t date;
    sal_time_t tmSec;  /*time make*/ 
    tbl_tag_summer_clock_t *p_db_summer_clk = NULL;
    tbl_clock_t *p_clk = NULL;
    p_db_summer_clk = tbl_tag_summer_clock_get_tag_summer_clock();
    /*get current time*/
    date = sal_time_sys_current(NULL);
    if(date == (sal_time_t) - 1)
    {
        log_sys(M_MOD_LIB, E_ERROR, "Unable to get time!errno = %s. ", sal_strerror(errno));
   
    }    
    /*when end current time is great than summer time, then change TZ*/
    _app_summer_clock_make_time(&p_db_summer_clk->tmend, &tmSec);
    if(((date - tmSec) >= 0) || ((date + TIME_ERROR_RANGE - tmSec) >= 0))
    {   
        p_clk = tbl_clock_get_clock();
        if (0 == sal_strlen(p_clk->timezone_name))
        {
            _app_clock_set_timezone(p_clk, FALSE);
        }
        else
        {
            _app_clock_set_timezone(p_clk, TRUE);
        }
        if(p_db_summer_clk->is_recurring)
        {
            if (p_db_summer_clk->is_wday_tm)
            {
                p_db_summer_clk->wday_tm_start.wday_tm_year += 1;
                p_db_summer_clk->wday_tm_end.wday_tm_year += 1;
                _app_summmer_trans_wday_time_to_localtime(&p_db_summer_clk->wday_tm_start, &p_db_summer_clk->tmstart);
                _app_summmer_trans_wday_time_to_localtime(&p_db_summer_clk->wday_tm_end, &p_db_summer_clk->tmend);
                p_db_summer_clk->nStartFebDay = p_db_summer_clk->tmstart.tm_mday;
                p_db_summer_clk->nEndFebDay = p_db_summer_clk->tmend.tm_mday;
            }
            else
            {
                p_db_summer_clk->tmstart.tm_year += 1;
                p_db_summer_clk->tmend.tm_year += 1;
            }
            _app_summer_clock_check_date(&p_db_summer_clk->tmstart.tm_mday, p_db_summer_clk->tmstart.tm_mon, 
                p_db_summer_clk->tmstart.tm_year, p_db_summer_clk, FLAG_DST);
            _app_summer_clock_check_date(&p_db_summer_clk->tmend.tm_mday, p_db_summer_clk->tmend.tm_mon, 
                p_db_summer_clk->tmend.tm_year, p_db_summer_clk, FLAG_DST); 
             app_set_tag_summer_clock(p_db_summer_clk);
            /*add summer time alarm*/
            _app_summer_clock_cal_dst_timer_interval(&interval);
            p_db_summer_clk->t_summer_time_timer = CTC_TASK_ADD_TIME(_app_summer_clock_dst_timer_alarm, NULL,interval);
            tbl_tag_summer_clock_set_tag_summer_clock_field(p_db_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_T_SUMMER_TIME_TIMER);
        }
        else
        {
            CTC_TASK_STOP_TIMER(p_db_summer_clk->t_summer_time_timer);   
        }
    }  
    else
    {
        /*when current time is great than start summer time, then change TZ*/
        _app_summer_clock_make_time(&p_db_summer_clk->tmstart, &tmSec);
        if(((date - tmSec) >= 0) || ((date + TIME_ERROR_RANGE - tmSec) >= 0))
        {
            _app_summer_clock_set_timezone_dst();   
        }

        /*add summer time alarm*/
        _app_summer_clock_cal_dst_timer_interval(&interval);
        p_db_summer_clk->t_summer_time_timer = CTC_TASK_ADD_TIME(_app_summer_clock_dst_timer_alarm, NULL, interval);
        tbl_tag_summer_clock_set_tag_summer_clock_field(p_db_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_T_SUMMER_TIME_TIMER);
    }   
}
int
_app_summer_clock_dst_timer_adjust()
{
    long int interval = 0;   /*timer alarm inteval*/    
    /*adjust summer time alarm*/ 
    tbl_tag_summer_clock_t *p_db_summer_clk = NULL;
    p_db_summer_clk = tbl_tag_summer_clock_get_tag_summer_clock();
    _app_summer_clock_cal_dst_timer_interval(&interval);
    if(interval != 0)
    {
        p_db_summer_clk->t_summer_time_timer = CTC_TASK_ADD_TIME(_app_summer_clock_dst_timer_alarm,NULL,interval);  
        tbl_tag_summer_clock_set_tag_summer_clock_field(p_db_summer_clk, TBL_TAG_SUMMER_CLOCK_FLD_T_SUMMER_TIME_TIMER);
    }
    
    return PM_E_NONE;
}
static int32
_app_summer_clock_cmd_get(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    tbl_tag_summer_clock_t *p_summer_db_clk =  NULL;
    FILE *fp = NULL;

    p_summer_db_clk = tbl_tag_summer_clock_get_tag_summer_clock();
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");
    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_tag_summer_clock_dump_one(p_summer_db_clk, &args);
    sal_fclose(fp);
    return PM_E_NONE;
}

static int32
_app_clock_cmd_get(cfg_cmd_para_t *para, cdb_node_t *p_node, field_parse_t *p_field)
{
    tbl_iter_args_t args;
    tbl_clock_t *p_db_clk = tbl_clock_get_clock();
    FILE *fp = NULL;
    
    CFG_BUILD_SHOW_PATH(para->p_show->path, para->pid, para->p_msg->hdr.seq);
    fp = sal_fopen(para->p_show->path, "w+");

    sal_memset(&args, 0 ,sizeof(args));
    args.argv[0] = p_field;
    args.argv[1] = fp;
    tbl_clock_dump_one(p_db_clk, &args);
    sal_fclose(fp);

    return PM_E_NONE;
}

static int32
_app_clock_cmd_add_timezone(tbl_clock_t *p_clk, cfg_cmd_para_t *para)
{
    tbl_chassis_t *p_chassis = tbl_chassis_get_chassis();
    int32 rc = PM_E_NONE;
    if (p_chassis->schedule_reboot)
    {
        CFG_CONFLICT_RET("Schedule reboot has been configured");
    }

    rc = app_clock_add_timezone(p_clk);
    return rc;
}

static int32
_app_clock_cmd_del_timezone(tbl_clock_t *p_clk, cfg_cmd_para_t *para)
{
    tbl_chassis_t *p_chassis = tbl_chassis_get_chassis();
    int32 rc = PM_E_NONE;
    if (p_chassis->schedule_reboot)
    {
        CFG_CONFLICT_RET("Schedule reboot has been configured");
    }
    
    rc = app_clock_del_timezone(p_clk, FALSE);   
    return rc;
}
int32
_app_summer_clock_cmd_del_timezone(int32 flag)
{
    tbl_clock_t *p_db_clk = tbl_clock_get_clock();
    if (NULL == p_db_clk)
    {
        return PM_E_NONE;
    }
    if (flag)
    {
        p_db_clk->timezone_hour = p_db_clk->cp_timezone_hour;
        p_db_clk->timezone_minute = p_db_clk->cp_timezone_minute ;
        p_db_clk->timezone_second = p_db_clk->cp_timezone_second ;
        p_db_clk->timezone_positive = p_db_clk->cp_timezone_positive ;
        sal_strncpy(p_db_clk->timezone_name,p_db_clk->cp_timezone_name,strlen(p_db_clk->cp_timezone_name));
        tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_TIMEZONE_POSITIVE);
        tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_TIMEZONE_HOUR);
        tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_TIMEZONE_MINUTE);
        tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_TIMEZONE_SECOND);
        tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_TIMEZONE_NAME);
        _app_clock_set_timezone(p_db_clk, TRUE);
    }
    else
    {   
        sal_memset(p_db_clk, 0, sizeof(*p_db_clk));
        tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_TIMEZONE_POSITIVE);
        tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_TIMEZONE_HOUR);
        tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_TIMEZONE_MINUTE);
        tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_TIMEZONE_SECOND);
        tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_TIMEZONE_NAME);
        tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_CP_TIMEZONE_POSITIVE);
        tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_CP_TIMEZONE_HOUR);
        tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_CP_TIMEZONE_MINUTE);
        tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_CP_TIMEZONE_SECOND);
        tbl_clock_set_clock_field(p_db_clk, TBL_CLOCK_FLD_CP_TIMEZONE_NAME);
        _app_clock_set_timezone(p_db_clk, FALSE);
     }
     return PM_E_NONE;
}
int32
app_check_timezone_name(const char *name, int32 min_len, int32 max_len, char *err_desc)
{
    char *p = NULL;
    
    /* check the string length */
    if (sal_strlen(name) >= max_len || sal_strlen(name) < min_len)
    {
        sal_sprintf(err_desc, "Zone name length range is [%u, %u)", min_len, max_len);
        return PM_E_INVALID_PARAM;
    }
    
    /*check the first character*/    
    if (!sal_isalpha(*name))
    {
        sal_sprintf(err_desc, "The first character should be a-z or A-Z");
        return PM_E_INVALID_PARAM;
    }
    
    /* check the chars */
    p = (char *)(name + 1);
    while ('\0' != *p)
    {
        if (sal_isalpha(*p))
        {
            p++;
            continue;
        }
        sal_sprintf(err_desc, "Character should be \"A-Za-z\"");
        return PM_E_INVALID_PARAM;
    }
    /*check the last character*/
    p--;
    if (!sal_isalpha(*p))
    {
        sal_sprintf(err_desc, "The last character should be a-z or A-Z");
        return PM_E_INVALID_PARAM;
    }
    return PM_E_NONE;
}

int32
app_clock_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    tbl_clock_t clk;
    int32 rc = PM_E_NONE;
    tbl_tag_summer_clock_t *p_summer_db_clk = NULL;

    CFG_DBG_FUNC();

    p_node = cdb_get_tbl(TBL_CLOCK);
    p_summer_db_clk =  tbl_tag_summer_clock_get_tag_summer_clock();
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        if (0 == sal_strcmp(argv[0], "timezone"))
        {
            sal_memset(&clk, 0, sizeof(clk));
            rc = app_check_timezone_name(argv[1], MIN_ZONE_NAME_LEN, MAX_ZONE_NAME_LEN, para->p_rs->desc);
            if (rc < 0) 
            {
                para->p_rs->rc = PM_E_CFG_INVALID_PARAM;
                return rc;
            }
            PM_CFG_CHECK_EXPECT_PARAMS(argc, 6, para->p_rs);

            sal_strncpy(clk.timezone_name, argv[1], MAX_ZONE_NAME_LEN);
            if (0 == sal_strcmp(argv[2], "add"))
            {
                clk.timezone_positive = TRUE;
            }
            else
            {
                clk.timezone_positive = FALSE;
            }

            PM_CFG_GET_UINT(clk.timezone_hour, argv[3], para->p_rs);
            if (0 != sal_strcmp(argv[4], "none"))
            {
                PM_CFG_GET_UINT(clk.timezone_minute, argv[4], para->p_rs);
            }
            if (0 != sal_strcmp(argv[5], "none"))
            {
                PM_CFG_GET_UINT(clk.timezone_second, argv[5], para->p_rs);
            }
            rc = _app_clock_cmd_add_timezone(&clk, para);
            if (p_summer_db_clk->strZoneName)
            {
                 CTC_TASK_STOP_TIMER(p_summer_db_clk->t_summer_time_timer); 
                _app_summer_clock_dst_timer_adjust();
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
        break;

    case CDB_OPER_DEL:
        if (0 == sal_strcmp(argv[0], "timezone"))
        {
            sal_memset(&clk, 0, sizeof(clk));
            rc = _app_clock_cmd_del_timezone(&clk, para);
            if (p_summer_db_clk->strZoneName)
            {
                 CTC_TASK_STOP_TIMER(p_summer_db_clk->t_summer_time_timer); 
                _app_summer_clock_dst_timer_adjust();
            }
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid parameters");
        }
        break;

    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _app_clock_cmd_get(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }  
    return rc;
}
int32
app_summer_clock_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para)
{
    cdb_node_t* p_node = NULL;
    field_parse_t field;
    tbl_tag_summer_clock_t summer_clk;
    tbl_tag_summer_clock_t *p_summer_db_clk;
    int32 nWhich;
    int32 nWeekday;
    int32 nMon;
    int32 nDay;
    int32 nYeay;
    int32 nHour;
    int32 nMin;
    int32 nSec;
    int32 rc = PM_E_NONE;
    time_t date;
    struct tm *ptmCur = NULL;
    tbl_clock_t *p_db_clk = tbl_clock_get_clock();
    CFG_DBG_FUNC();
    p_node = cdb_get_tbl(TBL_CLOCK);
    p_summer_db_clk =  tbl_tag_summer_clock_get_tag_summer_clock();
    switch (para->oper)
    {
    case CDB_OPER_ADD:
        
        break;

    case CDB_OPER_DEL:
        if (!sal_strlen(p_summer_db_clk->strZoneName))
        {
            return PM_E_NONE;
        }
        if (sal_strlen(p_db_clk->cp_timezone_name))
        {
            _app_summer_clock_cmd_del_timezone(TRUE); 
        }
        else
        {
            _app_summer_clock_cmd_del_timezone(FALSE);
        }
        sal_memset(p_summer_db_clk, 0, sizeof(*p_summer_db_clk));
        app_set_tag_summer_clock(p_summer_db_clk);
        break;
    case CDB_OPER_SET:
        sal_memset(&summer_clk, 0, sizeof(summer_clk));
        if ((0 == sal_strcmp(argv[1], "recurring"))&&(0 == sal_strcmp(argv[2], "rec_which"))&&(12 == argc))
        {
            if (_app_clock_check_timezone_name(argv[0]) < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid zone name, must be character A-Za-z and not be the same as standard zone name. Its length range is [3,31]");
            }
            if (!sal_strcmp(argv[0], p_db_clk->timezone_name))
            {
                CFG_INVALID_PARAM_RET("Invalid zone name, must be character A-Za-z and not be the same as standard zone name. Its length range is [3,31]");
            }
            sal_strncpy(summer_clk.strZoneName, argv[0], MAX_ZONE_NAME_LEN);
            summer_clk.strZoneName[MAX_ZONE_NAME_LEN] = '\0'; 
            
            if (_app_summer_clock_get_which_from_str(argv[3], &nWhich))
            {
                CFG_INVALID_PARAM_RET("Invalid which week");
            }
            if (_app_summer_clock_get_weekday_from_str(argv[4], &nWeekday))
            {
                CFG_INVALID_PARAM_RET("Invalid start weekday");
            }
            nMon = sal_atoi(argv[5]);
            if (sal_sscanf(argv[6], "%2d:%2d:%2d", &nHour, &nMin, &nSec) != 3)
            {
                CFG_INVALID_PARAM_RET("Unable to translate clock time: %s", argv[6]);
            }  
            summer_clk.wday_tm_start.wday_tm_which = nWhich;
            summer_clk.wday_tm_start.wday_tm_wday = nWeekday;
            summer_clk.wday_tm_start.wday_tm_mon = nMon;
            summer_clk.wday_tm_start.wday_tm_hour = nHour;
            summer_clk.wday_tm_start.wday_tm_min = nMin;
            summer_clk.wday_tm_start.wday_tm_sec = nSec;
            
            if (_app_summer_clock_get_which_from_str(argv[7], &nWhich))
            {
                CFG_INVALID_PARAM_RET("Invalid which week");
            }
            if (_app_summer_clock_get_weekday_from_str(argv[8], &nWeekday))
            {
                CFG_INVALID_PARAM_RET("Invalid end weekday");
            }
            nMon = sal_atoi(argv[9]);
            if(sal_sscanf(argv[10], "%2d:%2d:%2d", &nHour, &nMin, &nSec) != 3)
            {
                CFG_INVALID_PARAM_RET("Unable to translate clock time: %s", argv[10]);
            }  
            summer_clk.wday_tm_end.wday_tm_which = nWhich;
            summer_clk.wday_tm_end.wday_tm_wday = nWeekday;
            summer_clk.wday_tm_end.wday_tm_mon = nMon;
            summer_clk.wday_tm_end.wday_tm_hour = nHour;
            summer_clk.wday_tm_end.wday_tm_min = nMin;
            summer_clk.wday_tm_end.wday_tm_sec = nSec; 
            summer_clk.iDstOffset = sal_atoi(argv[11]);
            
            date = sal_time_sys_current(NULL);
            if(date == (sal_time_t) - 1)
            {
                CFG_INVALID_PARAM_RET("Unable to get time:%s", sal_strerror(errno));
            }
            ptmCur = sal_localtime(&date);
            summer_clk.wday_tm_start.wday_tm_year = ptmCur->tm_year + 1900;
            summer_clk.wday_tm_end.wday_tm_year = ptmCur->tm_year + 1900;
            
            if (summer_clk.wday_tm_start.wday_tm_mon == summer_clk.wday_tm_end.wday_tm_mon)
            {
                CFG_INVALID_PARAM_RET ("The end time and the start time cannot be in the same month");
            }
            if (summer_clk.wday_tm_start.wday_tm_mon > summer_clk.wday_tm_end.wday_tm_mon)
            {
                summer_clk.wday_tm_end.wday_tm_year++;
            }
            _app_summmer_trans_wday_time_to_localtime (&summer_clk.wday_tm_start, &summer_clk.tmstart);
            _app_summmer_trans_wday_time_to_localtime (&summer_clk.wday_tm_end, &summer_clk.tmend);
            
            summer_clk.is_recurring = TRUE;
            summer_clk.is_wday_tm = TRUE;          
            /*when year changed, check month and day*/
            /*when recurring, and feb has 29 day, we should store it for leap year*/
            summer_clk.nStartFebDay = summer_clk.tmstart.tm_mday;
            summer_clk.nEndFebDay = summer_clk.tmend.tm_mday; 
            app_set_tag_summer_clock(&summer_clk);
            if (p_summer_db_clk->t_summer_time_timer)
            {
                if (sal_strlen(p_db_clk->cp_timezone_name))
                {
                    _app_summer_clock_cmd_del_timezone(TRUE); 
                }
                else
                {
                    _app_summer_clock_cmd_del_timezone(FALSE);
                } 
                CTC_TASK_STOP_TIMER(p_summer_db_clk->t_summer_time_timer);
            }
            rc = _app_summer_clock_dst_timer_adjust(NULL);
            if(rc < 0)
            {
                CFG_INVALID_PARAM_RET("Summer time adjust failed");
            }
    
            return PM_E_NONE;
        }
        else if ((0 == sal_strcmp(argv[1], "recurring"))&&(0 == sal_strcmp(argv[2], "rec_date"))&&(10 == argc))
        {
            sal_memset(&summer_clk, 0, sizeof(summer_clk));
            if (_app_clock_check_timezone_name(argv[0]) < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid zone name, must be character A-Za-z and not be the same as standard zone name. Its length range is [3,31]");
            }
            if (!sal_strcmp(argv[0], p_db_clk->timezone_name))
            {
                CFG_INVALID_PARAM_RET("Invalid zone name, must be character A-Za-z and not be the same as standard zone name. Its length range is [3,31]");
            }
            sal_strncpy(summer_clk.strZoneName, argv[0], MAX_ZONE_NAME_LEN);
            summer_clk.strZoneName[MAX_ZONE_NAME_LEN] = '\0'; 
            nMon = sal_atoi(argv[3]);
            nDay = sal_atoi(argv[4]);
            if(sal_sscanf(argv[5], "%2d:%2d:%2d", &nHour, &nMin, &nSec) != 3)
            {
                CFG_INVALID_PARAM_RET("Unable to translate clock time: %s", argv[5]);
            }
            summer_clk.tmstart.tm_mon = nMon;
            summer_clk.tmstart.tm_mday = nDay;
            summer_clk.tmstart.tm_hour = nHour;
            summer_clk.tmstart.tm_min = nMin;
            summer_clk.tmstart.tm_sec = nSec;
            nMon = sal_atoi(argv[6]);
            nDay = sal_atoi(argv[7]);
            if(sal_sscanf(argv[8], "%2d:%2d:%2d", &nHour, &nMin, &nSec) != 3)
            {
                CFG_INVALID_PARAM_RET("Unable to translate clock time: %s", argv[8]);
            }
            summer_clk.tmend.tm_mon = nMon;
            summer_clk.tmend.tm_mday = nDay;
            summer_clk.tmend.tm_hour = nHour;
            summer_clk.tmend.tm_min = nMin;
            summer_clk.tmend.tm_sec = nSec;
            summer_clk.iDstOffset = sal_atoi(argv[9]);
            summer_clk.is_recurring = TRUE;
            summer_clk.is_wday_tm = FALSE;  
            summer_clk.nStartFebDay = summer_clk.tmstart.tm_mday;
            summer_clk.nEndFebDay = summer_clk.tmend.tm_mday; 
            date = sal_time_sys_current(NULL);
            if (date == (sal_time_t) - 1)
            {
                CFG_INVALID_PARAM_RET("Unable to get time:%s", sal_strerror(errno));
            }
            ptmCur = sal_localtime(&date);
            summer_clk.tmstart.tm_year= ptmCur->tm_year + 1900;
            summer_clk.tmend.tm_year= ptmCur->tm_year + 1900;
            if (summer_clk.tmend.tm_mon == summer_clk.tmstart.tm_mon)
            {
                CFG_INVALID_PARAM_RET ("The end time and the start time cannot be in the same month");
            }
            if (summer_clk.tmstart.tm_mon> summer_clk.tmend.tm_mon)
            {
                summer_clk.tmend.tm_year++;
            }
            rc = _app_summer_clock_check_date(&summer_clk.tmstart.tm_mday, summer_clk.tmstart.tm_mon, 
                                         summer_clk.tmstart.tm_year, &summer_clk, FLAG_DST);
            if (rc)
            {
                CFG_INVALID_PARAM_RET("Invalid start time. The start year,start month and start day do not match");
            }
            rc =_app_summer_clock_check_date(&summer_clk.tmend.tm_mday, summer_clk.tmend.tm_mon, 
                                          summer_clk.tmend.tm_year, &summer_clk, FLAG_DST);
            if (rc)
            {
                CFG_INVALID_PARAM_RET("Invalid end time. The end year,end month and end day do not match");
            }
            app_set_tag_summer_clock(&summer_clk);
            if (p_summer_db_clk->t_summer_time_timer)
            {
                if (sal_strlen(p_db_clk->cp_timezone_name))
                {
                    _app_summer_clock_cmd_del_timezone(TRUE); 
                }
                else
                {
                    _app_summer_clock_cmd_del_timezone(FALSE);
                } 
                CTC_TASK_STOP_TIMER(p_summer_db_clk->t_summer_time_timer);
            }
            rc = _app_summer_clock_dst_timer_adjust(NULL);
            if(rc < 0)
            {
                CFG_INVALID_PARAM_RET("Summer time adjust failed");
            }
            return PM_E_NONE;     
        }
        else if ((0 == sal_strcmp(argv[1], "date"))&&(11 == argc))
        {
            
            sal_memset(&summer_clk, 0, sizeof(summer_clk));
            if (_app_clock_check_timezone_name(argv[0]) < 0)
            {
                CFG_INVALID_PARAM_RET("Invalid zone name, must be character A-Za-z and not be the same as standard zone name. Its length range is [3,31]");
            }
            if (!sal_strcmp(argv[0], p_db_clk->timezone_name))
            {
                CFG_INVALID_PARAM_RET("Invalid zone name, must be character A-Za-z and not be the same as standard zone name. Its length range is [3,31]");
            }
            sal_strncpy(summer_clk.strZoneName, argv[0], MAX_ZONE_NAME_LEN);
            summer_clk.strZoneName[MAX_ZONE_NAME_LEN] = '\0'; 
            nMon = sal_atoi(argv[2]);
            nDay = sal_atoi(argv[3]);
            nYeay = sal_atoi(argv[4]);
            if(sal_sscanf(argv[5], "%2d:%2d:%2d", &nHour, &nMin, &nSec) != 3)
            {
                CFG_INVALID_PARAM_RET("Unable to translate clock time: %s", argv[5]);
            }
            summer_clk.tmstart.tm_mon = nMon;
            summer_clk.tmstart.tm_mday = nDay;
            summer_clk.tmstart.tm_year = nYeay;
            summer_clk.tmstart.tm_hour = nHour;
            summer_clk.tmstart.tm_min = nMin;
            summer_clk.tmstart.tm_sec = nSec;
            nMon = sal_atoi(argv[6]);
            nDay = sal_atoi(argv[7]);
            nYeay = sal_atoi(argv[8]);
            if(sal_sscanf(argv[9], "%2d:%2d:%2d", &nHour, &nMin, &nSec) != 3)
            {
                CFG_INVALID_PARAM_RET("Unable to translate clock time: %s", argv[9]);
            }
            summer_clk.tmend.tm_mon = nMon;
            summer_clk.tmend.tm_mday = nDay;
            summer_clk.tmend.tm_year = nYeay;
            summer_clk.tmend.tm_hour = nHour;
            summer_clk.tmend.tm_min = nMin;
            summer_clk.tmend.tm_sec = nSec;
            summer_clk.iDstOffset = sal_atoi(argv[10]);
            summer_clk.is_recurring = FALSE;
            summer_clk.is_wday_tm = FALSE;  
            summer_clk.nStartFebDay = summer_clk.tmstart.tm_mday;
            summer_clk.nEndFebDay = summer_clk.tmend.tm_mday; 
            if (summer_clk.tmstart.tm_year > summer_clk.tmend.tm_year)
            {
                CFG_INVALID_PARAM_RET("The end year can not be less than start year");
            }
            if ((summer_clk.tmstart.tm_year == summer_clk.tmend.tm_year)&&(summer_clk.tmstart.tm_mon== summer_clk.tmend.tm_mon))
            {
                CFG_INVALID_PARAM_RET("In the same year, the end time and the start time cannot be in the same month");                
            }
            if ((summer_clk.tmstart.tm_year == summer_clk.tmend.tm_year)&&(summer_clk.tmstart.tm_mon> summer_clk.tmend.tm_mon))
            {
                CFG_INVALID_PARAM_RET("In the same year the end time can not be less than the start time");                
            }

            rc = _app_summer_clock_check_date(&summer_clk.tmstart.tm_mday, summer_clk.tmstart.tm_mon, 
                                         summer_clk.tmstart.tm_year, &summer_clk, FLAG_DST);
            if (rc)
            {
                CFG_INVALID_PARAM_RET("Invalid start time.The start year,start month and start day do not match");
            }
            rc = _app_summer_clock_check_date(&summer_clk.tmend.tm_mday, summer_clk.tmend.tm_mon, 
                                          summer_clk.tmend.tm_year, &summer_clk, FLAG_DST);
            if (rc)
            {
                CFG_INVALID_PARAM_RET("Invalid end time.The end year,end month and end day do not match");
            }
            app_set_tag_summer_clock(&summer_clk);
            
            if (p_summer_db_clk->t_summer_time_timer)
            {
                if (sal_strlen(p_db_clk->cp_timezone_name))
                {
                    _app_summer_clock_cmd_del_timezone(TRUE); 
                }
                else
                {
                    _app_summer_clock_cmd_del_timezone(FALSE);
                } 
                CTC_TASK_STOP_TIMER(p_summer_db_clk->t_summer_time_timer);
            }
            rc = _app_summer_clock_dst_timer_adjust(NULL);
            if(rc < 0)
            {
                CFG_INVALID_PARAM_RET("Summer time adjust failed");
            }
            return PM_E_NONE;
        }
        else
        {
            CFG_INVALID_PARAM_RET("Invalid summer clock set");
        }
        break;
    
    case CDB_OPER_GET:
        rc = cfg_cmd_parse_field(argv, argc, p_node, &field, FALSE);
        if (rc < 0)
        {
            CFG_INVALID_PARAM_RET("Invalid field");
        }

        rc = _app_summer_clock_cmd_get(para, p_node, &field);
        break;

    default:
        CFG_INVALID_PARAM_RET("Invalid action");
        break;
    }
    
    return rc;
}
int32
app_clock_init()
{
    cdb_register_cfg_tbl_cb(TBL_CLOCK, app_clock_cmd_process);
    cdb_register_cfg_tbl_cb(TBL_TAG_SUMMER_CLOCK, app_summer_clock_cmd_process);
    return PM_E_NONE;
}

