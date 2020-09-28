#ifdef SDK_IN_VXWORKS
#include "sal.h"
#include "ctc_shell_server.h"

int ctc_vty_socket()
{
    return 0;
}

void ctc_vty_close()
{
    return;
}
#endif
