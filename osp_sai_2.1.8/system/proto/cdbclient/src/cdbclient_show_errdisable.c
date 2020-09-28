
#include "proto.h"
#include "version.h"
#include "cdbclient.h"
#include "gen/tbl_sys_global_define.h"
#include "gen/tbl_interface_define.h"
#include "gen/tbl_interface.h"
#include "gen/tbl_errdisable_define.h"


static int32
__cdbctl_show_errdisable_detect_one(char *line, uint32 *p_banner)
{
    cfg_parse_get_info_t info;
    
    if (TRUE == *p_banner)
    {
        *p_banner = FALSE;
        sal_printf("%-24s %-16s\n", "ErrDisable Reason", "Detection status");
        sal_printf("------------------------+----------------\n");
    }

    cfg_get_parse(line, &info, 0);

    sal_printf("%-24s %-16s\n", 
        info.value[TBL_ERRDISABLE_FLD_KEY], 
        enabled_str(enable_value(info.value[TBL_ERRDISABLE_FLD_ERRDISABLE_EN])));

    return PM_E_NONE;
}


static int32 
_cdbctl_show_errdisable_detect(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    uint32 banner = TRUE;
    
    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        __cdbctl_show_errdisable_detect_one(line, &banner);
    }

    return PM_E_NONE;
}

static int32
_cdbctl_show_errdisable_recovery_one(char *line, uint32 *p_banner)
{
    cfg_parse_get_info_t info;
    
    if (TRUE == *p_banner)
    {
        *p_banner = FALSE;
        sal_printf("%-24s %-16s\n", "ErrDisable Reason", "Detection status");
        sal_printf("------------------------+----------------\n");
    }

    cfg_get_parse(line, &info, 0);

    sal_printf("%-24s %-16s\n", 
        info.value[TBL_ERRDISABLE_FLD_KEY], 
        enabled_str(enable_value(info.value[TBL_ERRDISABLE_FLD_RECOVERY_EN])));

    return PM_E_NONE;
}


static int32 
_cdbctl_show_errdisable_recovery(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];
    uint32 banner = TRUE;
    cfg_parse_get_info_t info;
    int32 rc = PM_E_NONE;

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbctl_show_errdisable_recovery_one(line, &banner);
    }

    PM_E_RETURN(cdbclt_get_tbl_line(TBL_SYS_GLOBAL, NULL, 0, line, FILE_READ_LINE_LEN));
    PM_E_RETURN(cfg_get_parse(line, &info, 0));

    sal_printf("\nTimer interval: %d seconds\n", sal_atoi(info.value[TBL_SYS_GLOBAL_FLD_ERRDISABLE_INTERVAL]));

    return PM_E_NONE;
}

int32 cdbctl_show_errdisable(void *p_data, uint32 data_len, FILE *fp, cfg_parse_info_t *p_info)
{
    char *cmd = (char*)p_data;

    if (sal_strstr(cmd, "detect"))
    {
        _cdbctl_show_errdisable_detect(p_data, data_len, fp, p_info);
    }
    else if(sal_strstr(cmd, "recovery"))
    {
        _cdbctl_show_errdisable_recovery(p_data, data_len, fp, p_info);
    }

    return PM_E_NONE;
}

