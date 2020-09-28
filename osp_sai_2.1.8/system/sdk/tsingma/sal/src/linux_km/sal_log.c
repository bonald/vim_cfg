#include "sal.h"

sal_loglevel_t debug_level_flag;

char sal_print_buf[1024];
char sal_print_line[512];

static const char* ll2str[] =
{
    KERN_CRIT    "FATAL %s(%d): %s\n",
    KERN_ERR     "ERROR %s(%d): %s\n",
    KERN_WARNING "WARN  %s(%d): %s\n",
    KERN_INFO    "INFO  %s(%d): %s\n",
    KERN_DEBUG   "DEBUG %s(%d): %s\n",
};

int ctc_sal_log(int log_level,
            const char* file, int32 line, const char* fmt, ...)
{
    char fmtbuf[0x100];
    va_list ap;

    if (log_level < debug_level_flag)
    {
        return 0;
    }

    if (log_level < SAL_LL_FATAL)
    {
        log_level = SAL_LL_FATAL;
    }

    if (log_level < SAL_LL_INFO)
    {
        snprintf(fmtbuf, 0x100, ll2str[log_level], file, line, fmt);
        fmtbuf[0xFF] = 0;
    }
    else
    {
        strncpy(fmtbuf, fmt, 0x100);
        fmtbuf[0xFF] = 0;
    }

    va_start(ap, fmt);
    vprintk(fmtbuf, ap);
    va_end(ap);
    return 0;
}

int
ctc_sal_print_func(const char* fmt,  ...)
{
    int i = 0;
    int j = 0;
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(sal_print_buf, 1023, fmt, ap);
    va_end(ap);

    i = 0;

    while (i < 1024 && j < 512 && sal_print_buf[i] != '\0')
    {
        switch (sal_print_buf[i])
        {
        case '\n':
            sal_print_line[j] = '\0';
            printk("%s\r\n", sal_print_line);
            j = 0;
            break;

        default:
            sal_print_line[j] = sal_print_buf[i];
            j++;
            break;
        }

        i++;
    }

    if (sal_print_buf[i] == '\0')
    {
        sal_print_line[j] = '\0';
        printk("%s", sal_print_line);
    }

    return 0;
}

static int8 glog_printf_bug[1024] = "";
int ctc_sal_fprintf(sal_file_t pf, const char* fmt,  ...)
{
    va_list args;
	int i;

    sal_memset(glog_printf_bug,0,sizeof(glog_printf_bug));

	va_start(args, fmt);
	i = vsnprintf(glog_printf_bug, sizeof(glog_printf_bug), fmt, args);
	va_end(args);

    sal_write(pf,glog_printf_bug,i);

	return i;
}

EXPORT_SYMBOL(ctc_sal_log);

