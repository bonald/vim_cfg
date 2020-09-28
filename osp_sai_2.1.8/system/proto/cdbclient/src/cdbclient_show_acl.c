
#include "proto.h"
#include "cdbclient.h"

#include "gen/tbl_time_range_define.h"



static int32
_cdbctl_show_time_range_one(char *line)
{
    cfg_parse_get_info_t info;

    cfg_get_parse(line, &info, 0);

    sal_printf("time-range %s\n", info.value[TBL_TIME_RANGE_FLD_KEY]);
    sal_printf("%s\n", info.value[TBL_TIME_RANGE_FLD_TIME]);        

    return PM_E_NONE;
}

int32
cdbctl_show_time_range(FILE *fp, cfg_parse_info_t *p_info)
{
    char  line[FILE_READ_LINE_LEN];

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        _cdbctl_show_time_range_one(line);
    }

    return PM_E_NONE;
}

    int32
cdbctl_show_pmap_if_stats(FILE *fp, cfg_parse_info_t *p_info)
{
#if 0
    char  line[FILE_READ_LINE_LEN];

    while (NULL != sal_fgets(line, FILE_READ_LINE_LEN, fp))
    {
        sal_printf("%s\n", line);;
    }

#endif
    return PM_E_NONE;
}

