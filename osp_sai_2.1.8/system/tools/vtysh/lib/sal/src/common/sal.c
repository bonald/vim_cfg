#include"sal.h"

sal_err_t sal_init(void)
{
    sal_err_t ret;
    ret = sal_timer_init();
    return ret;
}

void sal_deinit(void)
{
    sal_timer_fini();
}


