#include "sal.h"

sal_err_t
ctc_sal_init(void)
{
    sal_err_t ret;

    ret = sal_timer_init();
    return ret;
}

void
ctc_sal_deinit(void)
{
    sal_timer_fini();
}

#if defined(_SAL_LINUX_KM)

static int sort_of_seed;

int ctc_sal_get_rand()
{
    int rand_value = 0;
    get_random_bytes(&rand_value, sizeof(rand_value));

    return ((rand_value ^ sort_of_seed) & 0x7fffffff);
}

void
ctc_sal_srand(unsigned int seed)
{
    sort_of_seed = (seed << 19) ^ (seed << 13) ^ seed;
}

#include <asm/div64.h>

unsigned long long __udivdi3 (unsigned long long a, unsigned long long b) {
    unsigned long long x = a, y = b, result;
    unsigned long mod;

    mod = do_div(x, y);
    result = x;
    return result;
}


long long __divdi3 (long long a, long long b) {
    unsigned long long x = a, y = b, result;
    unsigned long mod;

    mod = do_div(x, y);
    result = x;
    return result;
}

long long __umoddi3(long long a, long long b) {
    unsigned long long x = a, y = b;
    unsigned long mod;

    mod = do_div(x, y);
    return mod;
}
/*
double __subdf3(double a, double b)
{
	return a-b;
}*/
#endif

