
#include "proto.h"
#include "version.h"
#include <dirent.h>
#include "gen/tbl_version_define.h"
#include "gen/tbl_version.h"
#ifndef _GLB_UML_SYSTEM_
#include "ctc_watchdog.h"
#endif

static lib_master_t* _g_lib_master = NULL;

lib_master_t*
lib_init(pm_id_t pm_id)
{
    uint32 multi_thread = FALSE;
    int32 rc = PM_E_NONE;

    if (PM_ID_FEA == pm_id)
    {
        multi_thread = TRUE;
    }
    
    rc = memory_init(multi_thread);
    if (rc)
    {
        return NULL;
    }
    
    _g_lib_master = (lib_master_t*)XCALLOC(MEM_PM_LIB_MASTER, sizeof(lib_master_t));
    if (!_g_lib_master)
    {
        return NULL;
    }

    _g_lib_master->cfg_sock = INVALID_SOCK_ID;
    _g_lib_master->data_sock = INVALID_SOCK_ID;
    _g_lib_master->sub_cdb_sock = INVALID_SOCK_ID;
    _g_lib_master->sub_tbl_sock = INVALID_SOCK_ID;
    _g_lib_master->trap_sock = INVALID_SOCK_ID;
    _g_lib_master->pm_id = pm_id;

    cdb_init(pm_id);
    ipc_sock_init();
    ipc_register_rx_cb(ipc_pm_process_msg);
    if (PM_ID_CTL == pm_id)
    {
        return _g_lib_master;
    }

    lib_cmd_init(pm_id);

    return _g_lib_master;
}

int32
lib_add_sync_tbl(uint32 tbl_id)
{
    _g_lib_master->sync_tbl_ids[_g_lib_master->sync_tbl_num] = tbl_id;
    _g_lib_master->sync_tbl_num++;

    return PM_E_NONE;
}

int32
lib_set_sync_enable()
{
    _g_lib_master->sync_enable = TRUE;
    _g_lib_master->sync_state = LIB_SYNCING;

    return PM_E_NONE;
}

int32
lib_set_synced()
{
    _g_lib_master->sync_state = LIB_SYNCED;

    return PM_E_NONE;
}

lib_master_t*
lib_get_master()
{
    return _g_lib_master;
}

const char*
lib_version(char *buf, int32 len)
{
    /* TODO .*/
    //if (lib_fs_is_exist(OEM_INFO_FILE))
    if (0)
    {
        // oem info
    }
    else
    {
        sal_snprintf(buf, len, "%s", OSP_VERSION);
    }
    
    return buf;
}

/* Utility function to print out version for main() for PMs. */
void
lib_print_version(char *progname)
{
    char buf[50];

    sal_printf("%s version %s\n", progname, lib_version(buf, 50));
}

const char*
yes_no_str(uint32 enable)
{
    return (enable) ? "Yes" : "No";
}

const char*
on_off_str(uint32 enable)
{
    return (enable) ? "On" : "Off";
}

const char*
on_off_str_lower(uint32 enable)
{
    return (enable) ? "on" : "off";
}

const char*
enable_str(uint32 enable)
{
    return (enable) ? "Enable" : "Disable";
}

const char*
enable_str_lower(uint32 enable)
{
    return (enable) ? "enable" : "disable";
}

const char*
enabled_str(uint32 enable)
{
    return (enable) ? "Enabled" : "Disabled";
}

const char*
enabled_str_lower(uint32 enable)
{
    return (enable) ? "enabled" : "disabled";
}

const char*
active_str(uint32 active)
{
    return (active) ? "Active" : "Suspend";
}

const char*
restart_str(uint32 restart)
{
    if (restart)
    {
        return "restart";
    }
    else
    {
        return "start";
    }
}

uint32
enable_value(const char *str)
{
    if ('0' == str[0])
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

int
check_line_str(char *pszStr, int len)
{
    char *p = pszStr;

    if (NULL == p)
    {
        return -1;
    }

    if (sal_strlen(p) >= len)
    {
        return -2;
    }
    
    /* check the chars */
    while ('\0' != *p) {
        if ((sal_isdigit(*p)) 
            || (sal_isupper(*p))
            || (sal_islower(*p))
            || (sal_isspace(*p))
            || ('.' == *p)
            || ('@' == *p)
            || (',' == *p)
            || ('-' == *p)
            || ('_' == *p)) {
            p++;
            continue;
        }

        return -1;
    }

    return 0;
}

int
check_if_name_str(char *pszName, int len)
{
    char *p = pszName;

    if (NULL == p)
    {
        return -4;
    }

    if ((0 == sal_strncmp("loopback", p, sal_strlen("loopback"))) && sal_strlen(p) >= KERNEL_IFNAME_MAX_SIZE)
    {
        return -5;
    }

    if ((0 == sal_strncmp("loopback", p, sal_strlen("loopback"))) && p[sal_strlen("loopback")] == '0' )
    {
        if(0 != sal_strcmp("loopback0", p))
        {
            return -5;
        }
    }

    if ((0 == sal_strncmp(GLB_IFNAME_VLAN_PREFIX, p, sal_strlen(GLB_IFNAME_VLAN_PREFIX))) && p[sal_strlen(GLB_IFNAME_VLAN_PREFIX)] == '0' )
    {
        if(0 != sal_strcmp(GLB_IFNAME_VLAN_PREFIX"0", p))
        {
            return -5;
        }
    }

    if ((0 == sal_strncmp(GLB_IFNAME_L2GRE_PREFIX, p, sal_strlen(GLB_IFNAME_L2GRE_PREFIX))) && p[sal_strlen(GLB_IFNAME_L2GRE_PREFIX)] == '0' )
    {
        if(0 != sal_strcmp(GLB_IFNAME_L2GRE_PREFIX"0", p))
        {
            return -5;
        }
    }

    if ((0 == sal_strncmp(GLB_IFNAME_L3GRE_PREFIX, p, sal_strlen(GLB_IFNAME_L3GRE_PREFIX))) && p[sal_strlen(GLB_IFNAME_L3GRE_PREFIX)] == '0' )
    {
        if(0 != sal_strcmp(GLB_IFNAME_L3GRE_PREFIX"0", p))
        {
            return -5;
        }
    }

    if ((0 == sal_strncmp(GLB_IFNAME_NVGRE_PREFIX, p, sal_strlen(GLB_IFNAME_NVGRE_PREFIX))) && p[sal_strlen(GLB_IFNAME_NVGRE_PREFIX)] == '0' )
    {
        if(0 != sal_strcmp(GLB_IFNAME_NVGRE_PREFIX"0", p))
        {
            return -5;
        }
    }

    if ((0 == sal_strncmp(GLB_IFNAME_VXLAN_PREFIX, p, sal_strlen(GLB_IFNAME_VXLAN_PREFIX))) && p[sal_strlen(GLB_IFNAME_VXLAN_PREFIX)] == '0' )
    {
        if(0 != sal_strcmp(GLB_IFNAME_VXLAN_PREFIX"0", p))
        {
            return -5;
        }
    }
    
    if (!((p[0]>= 'a' && p[0] <= 'z') || (p[0]>= 'A' && p[0] <= 'Z') || (p[0]>= '0' && p[0] <= '9')))
    {
        return -1;
    }

    if (sal_strlen(p) > len)
    {
        return -2;
    }
    
    /* check the chars */
    while ('\0' != *p)
    {
        if ((sal_isdigit(*p)) 
            || (sal_isupper(*p))
            || (sal_islower(*p))
            || ('/' == *p)
            || ('-' == *p)
            || ('_' == *p)
            || (':' == *p))
        {
            p++;
            continue;
        }

        return -3;
    }

    return 0;
}


int
check_name_character_and_len(char *pszName, int len)
{
    char *p = pszName;

    if (NULL == p)
    {
        return -4;
    }

    if (!((p[0]>= 'a' && p[0] <= 'z') || (p[0]>= 'A' && p[0] <= 'Z') || (p[0]>= '0' && p[0] <= '9')))
    {
        return -1;
    }

    if (sal_strlen(p) > len)
    {
        return -2;
    }
    
    /* check the chars */
    while ('\0' != *p)
    {
        if ((sal_isdigit(*p)) 
            || (sal_isupper(*p))
            || (sal_islower(*p))
            || ('.' == *p)
            || ('-' == *p)
            || ('_' == *p))
        {
            p++;
            continue;
        }

        return -3;
    }

    return 0;
}

int32
lib_bmp_alloc_id(uint32 *bmp, uint32 max)
{
    int32 id = 0;
    
    for (id = 0; id < max; id++)
    {
        if (GLB_BMP_ISSET(bmp, id))
        {
            continue;
        }

        GLB_BMP_SET(bmp, id);
        return id;
    }

    return PM_E_FAIL; 
}

int32
lib_bmp_release_id(uint32 *bmp, int32 id, uint32 max)
{
    if (id >= max)
    {
        return PM_E_FAIL;
    }
    
    if (!GLB_BMP_ISSET(bmp, id))
    {
        return PM_E_FAIL;
    }

    GLB_BMP_UNSET(bmp, id);
    return PM_E_NONE; 
}

int32
lib_reboot()
{
#if 0
    int32 rc = 0;
    rc = reboot(RB_AUTOBOOT);
    if (-1 == rc)
    {
        return errno;
    }
#else
    /* reboot system by busybox reboot command 
     * to execute restart.sh before call kernel,
     * restart.sh is define in /etc/inittab*/
#ifdef _CTC_X86_64_
    /* temporarily, for x86_64 e582-48x6q only, added by shil. 2017-11-27 */
    system("i2cset -y 0 0x58 0x23 0x00");
    system("i2cset -y 0 0x58 0x23 0x03");
#endif
#ifdef HAVE_DEBIAN
    system("reboot -f");
#else
    system("reboot");
#endif
#endif
    return 0;
}

void
_lib_reboot_wait(void *p_data)
{
    lib_reboot();
}

int32
lib_reboot_system_trap()
{
    char str_time[50];
    sal_time_t date;

    sal_memset(str_time, 0, sizeof(str_time));
    date = sal_time(NULL);   
    strftime(str_time, sizeof(str_time), "%T %Z %a %b %d %Y", sal_localtime(&date));
    /*log reboot info*/
    logid_sys(LOG_SYSTEM_4_REBOOT, str_time);
    sleep(1);
    return 0;
}
/*modified by jqiu for bug46425, 2018-02-12*/
#ifndef _GLB_UML_SYSTEM_
#if (defined _CTC_FSCL_P1010_) || (defined _CTC_FSCL_P1014_)
int32
_lib_dying_watchdog()    
{
    int wd_fd;
    int ret;
    wd_fd = sal_open(CTC_WD_DEV_NAME, O_RDWR|O_SYNC);
    if (wd_fd < 0)
    {
        return -1;
    }
    ret = ioctl(wd_fd, CTC_DYING_WATCHDOG, 0);
    if(ret < 0)
        return -1;
    else
        return 0;
    return 0;
}
#endif
#endif

int32
lib_reboot_system()
{
#ifdef _GLB_UML_SYSTEM_
    system("/centec_switch/sbin/logsync.sh 1>/dev/null 2>&1");
    /*add by chenzh for bug 16508, 2011-09-27*/
    system("/centec_switch/sbin/diaglogsync.sh 1>/dev/null 2>&1");
#else
    system(SWITCH_PREFIX"logsync.sh 1>/dev/null 2>&1");
    /*add by chenzh for bug 16508, 2011-09-27*/
    system(SWITCH_PREFIX"diaglogsync.sh 1>/dev/null 2>&1");
#if (defined _CTC_FSCL_P1010_) || (defined _CTC_FSCL_P1014_)
    _lib_dying_watchdog();
#endif
#endif

    /* kick off the shell */
    system("killall klish");

    ctc_task_add_timer(TASK_PRI_HIGH, _lib_reboot_wait, NULL, 1);
    return PM_E_NONE;
}

int32
lib_print_to_all_tty(char *out)
{
    static DIR *dir;
    struct dirent *entry;
    char fullname[CMD_BUF_256];
    int32 fd = 0;

    if (NULL == out)
    {
        return PM_E_NONE;
    }

    /* print to console */
    sprintf(fullname, "/dev/console");
    fd = sal_open(fullname, O_WRONLY);
    if (fd != -1)
    {
        sal_write(fd, out, sal_strlen(out));
        sal_close(fd);
    }

    /* print to pts */    
    dir = opendir("/dev/pts");
    do 
    {
        if (NULL == (entry = readdir(dir)))
        {
            closedir(dir);
            return PM_E_NONE;
        }
        
        if (!sal_strcmp(entry->d_name, ".") || !sal_strcmp(entry->d_name, ".."))
        {
            continue;
        }        
        
        sprintf(fullname, "/dev/pts/%s", entry->d_name);
        fd = sal_open(fullname, O_WRONLY);
        if (fd == -1)
        {
            continue;
        }
        sal_write(fd, out, sal_strlen(out));
        sal_close(fd);
    } while(1);

    return PM_E_NONE;
}

char*
lib_get_product_name()
{
    tbl_version_t *p_db_version = tbl_version_get_version();

    if (p_db_version)
    {
        return p_db_version->product;
    }
    
    return "Unknown";
}

char*
lib_strip_newline(char *str)
{
    uint32 str_len = 0;
    
    if (NULL == str)
    {
        return 0;
    }
    
    str_len = sal_strlen(str);

    if (str_len > 0)
    {
        if (str[str_len-1] == CMD_LINE_CHAR)
        {
            str[str_len-1] = CMD_ZERO_CHAR;
        }
    }
    return 0;
}

