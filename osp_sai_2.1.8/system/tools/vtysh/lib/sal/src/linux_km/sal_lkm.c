#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include "sal.h"

static int sal_lkm_init()
{
    return 0;
}

static void sal_lkm_exit()
{
}

module_init(sal_lkm_init);
module_exit(sal_lkm_exit);
