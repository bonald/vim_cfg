
#ifndef __APP_CLOCK_H__
#define __APP_CLOCK_H__
enum whichWday
{
    FIRST_WDAY = 1,
    SECOND_WDAY = 2,
    THIRD_WDAY = 3,
    FOURTH_WDAY = 4,
    LAST_WDAY = 5,
};
typedef enum tagZoneOP
{
   OP_NEGATIVE = 0,
   OP_POSITIVE
}E_ZoneOP;
#define FLAG_DST     1
#define FLAG_NO_DST  0
#define TIME_DST                0x02
#define ISLEAP(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#define MAX_TIME_LEN 200
#define sal_time_sys_current(z) time(z)
#define M_TIMEZONE_NAME_MAXSIZE      32
#define M_TIMEZONE_NAME_MINSIZE      3 
#define TIME_ZONE               0x01
#define TIME_ERROR_RANGE 1  /*seconds*/
#if HAVE___SECURE_GETENV && !defined(__LCLINT__)
#define	getenv(_s)	__secure_getenv(_s)
#endif
int32
app_clock_add_timezone(tbl_clock_t *p_clk);

int32
app_clock_del_timezone(tbl_clock_t *p_clk, uint32 init_done);
int 
app_clock_notify_other_application();
int32
app_clock_write_profile_timezone(char *strZoneInfo);
int32
_app_clock_set_timezone(tbl_clock_t *p_clk, uint32 is_add);
int32
app_clock_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_clock_init();

int32
app_clock_add_bj_timezone();

int32
app_clock_proc_startup_done();

#endif /* !__APP_CLOCK_H__ */
