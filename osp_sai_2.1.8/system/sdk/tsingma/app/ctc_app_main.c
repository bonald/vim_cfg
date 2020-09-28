#include "sal.h"
#include "api/include/ctc_api.h"
#include "api/include/ctcs_api.h"
#include "ctc_app.h"

extern int32 userinit(uint8 ctc_shell_mode, uint8 ctcs_api_en, void* p_init_config);
extern int32 ctc_cli_start(int32 ctc_shell_mode);

extern ctc_app_master_t g_ctc_app_master;

#if !defined SDK_IN_VXWORKS && !defined _SAL_LINUX_KM
void
ctc_app_main_usage(void)
{
    sal_printf("Usage: ctcsdk [OPTIONS...]\r\n");
    sal_printf("Options:\n");
    sal_printf("  cli       CLI mode\n");
    sal_printf("  ctcs      CTCS mode\n");
    sal_printf("  warmboot  Warm boot mode\n");
    sal_printf("  reloading Warm boot reloading process\n");
#if defined CHIP_AGENT && defined _SAL_LINUX_UM && (1 == SDK_WORK_PLATFORM)
    sal_printf("  agent SDK-NAME SOCK-PATH-PREFIX TOPO-CFG Running SDK in chip agent env\n");
#endif
    return;
}

int32
main(const int argc,const char* argv[])
{
    int32 ret = 0;
    uint8 loop = 0;
    uint8 ctc_shell_mode = 1;
    uint8 ctcs_api_en = 0;
#if defined CHIP_AGENT && defined _SAL_LINUX_UM && (0 == SDK_WORK_PLATFORM)
    char agent_name[32] = {0};
    char agent_path[PATH_MAX] = {0};
    char agent_file[PATH_MAX] = {0};
#endif

    while(++loop < argc)
    {
        if (!sal_strcmp(argv[loop], "-h"))
        {
            ctc_app_main_usage();
            return 0;
        }

        if (!sal_strcmp(argv[loop], "cli"))
        {
            ctc_shell_mode =  0;
        }
        else if (!sal_strcmp(argv[loop], "ctcs"))
        {
            ctcs_api_en = 1;
        }
        else if (!sal_strcmp(argv[loop], "wb") || !sal_strcmp(argv[loop], "warmboot"))
        {
            g_ctc_app_master.wb_enable = 1;
        }
        else if (!sal_strcmp(argv[loop], "reloading"))
        {
            g_ctc_app_master.wb_reloading = 1;
        }
        else if (!sal_strcmp(argv[loop], "rchain"))
        {
            ctcs_api_en = 2;
        }
#if defined CHIP_AGENT && defined _SAL_LINUX_UM && (0 == SDK_WORK_PLATFORM)
        else if (!sal_strcmp(argv[loop], "agent"))
        {
            if (loop >= (argc - 1)) {
                sal_printf("%% Please specify SDK name\n");
                return -1;
            }
            sal_snprintf(agent_name, 32, "%s", (char *)argv[loop + 1]);
            loop++;

            if (loop >= (argc - 1)) {
                sal_printf("%% Please specify socket path prefix\n");
                return -1;
            }
            sal_snprintf(agent_path, PATH_MAX, "%s", (char *)argv[loop + 1]);
            loop++;

            if (loop >= (argc - 1)) {
                sal_printf("%% Please specify topo cofiguration file\n");
                return -1;
            }
            sal_snprintf(agent_file, PATH_MAX, "%s",  (char *)argv[loop + 1]);
            loop++;

            extern int ctc_chip_agent_set_env(char *pszName, char *pszPathPrefix, char *pszCfgFile);
            ctc_chip_agent_set_env(agent_name, agent_path, agent_file);
        }
#endif
    }

    ret = userinit(ctc_shell_mode, ctcs_api_en, NULL);
    if (0 != ret)
    {
        return ret;
    }
    ret = ctc_cli_start(ctc_shell_mode);

    return ret;
}
#endif

