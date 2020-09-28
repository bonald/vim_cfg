#include "proto.h"
#include "cdbclient.h"

int32
cdbclient_execvp(char **argv)
{
    char cmd_str[CMD_BUF_256];
    int32 i = 0;
    int32 rc = 0;

    cmd_str[0] = CMD_ZERO_CHAR;
    
    while (argv[i])
    {
        sal_strncat(cmd_str, argv[i], CMD_BUF_256);
        sal_strncat(cmd_str, " ", CMD_BUF_256);
        i++;
    }

    rc = system(cmd_str);
//    sal_printf("cmd: %s, rc: %d\n", cmd_str, rc);
    if (0 != rc)
    {
        rc = -1;
    }

    return rc;
}

