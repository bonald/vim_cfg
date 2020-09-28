
#include <zebra.h>
#include <sys/un.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <pwd.h>
#include <sys/file.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <lib/version.h>
#include "getopt.h"
#include "command.h"
#include "memory.h"
#include "linklist.h"
#include "memory_vty.h"
#include "libfrr.h"
#include "vty.h"
#include "vtysh.h"

extern void vtysh_init_cmd(void);

static char quagga_config_default[MAXPATHLEN] = SYSCONFDIR FRR_DEFAULT_CONFIG;
char *quagga_config = quagga_config_default;
int execute_flag = 0;

/* VTY Socket prefix */
const char *vty_sock_path = NULL;

int
vtyclient_init()
{
	if (!vty_sock_path)
		vty_sock_path = frr_vtydir;

	/* Make vty structure and register commands. */
	vtysh_init_vty();
	vtysh_init_cmd();
	vtysh_user_init();
	vtysh_config_init();

	vty_init_vtysh();

	/* Do not connect until we have passed authentication. */
	if (vtysh_connect_all(NULL) <= 0) {
		fprintf(stderr, "Exiting: failed to connect to any daemons.\n");
	}

//	vtysh_pager_init();
	vtysh_readline_init();

	/* Enter into enable node. */
	vtysh_execute("enable");
    return 0;
}

