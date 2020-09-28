/****************************************************************************
 * 
 * Head files
 * 
 ****************************************************************************/
#include "sal.h"
#include "ctc_task.h"
#include "genlog.h"
#include "genlog_inner.h"
#include "genlog_usr.h"
#include "genlog_id_priv.h"
 
/***************************************************************************
 *
 * Globals and declarations
 * 
 ***************************************************************************/
static struct genlog_data s_log_data = SYSLOG_DATA_INIT; /* common log envirionment */
static struct genlog_data s_osp_log_data = SYSLOG_OSP_DATA_INIT; /* common log envirionment */
static struct extra_log_info s_extra_info = EXTRA_LOG_INFO_INIT; /* extra info, eg, the user name and ip */

/*******************************************************************************
 * Name:    _log_strlcpy
 * Purpose: security string copy
 * Input: 
 *   dst: destination 
 *   src: source 
 *   size: length 
 * Output: 
 * Return: 
 *   success: the length of src 
 *   failed : the length of src
 * Note: used for string copy 
 ******************************************************************************/
static size_t
_log_strlcpy(char *dst, const char *src, size_t size)
{
    if (size)
    {
        sal_strncpy(dst, src, size - 1);
        dst[size - 1] = '\0';
    }
    else
    {
        dst[0] = '\0';
    }
    return sal_strlen(src);
}

/*******************************************************************************
 * Name:    _copy_meta_str
 * Purpose: copy string with meta charachter disable
 * Input: 
 *   src: the source string
 * Output: 
 * Return:
 *   dest string, 0 for NULL
 * Note: the client response to free the dest memory
 ******************************************************************************/
static char*
_copy_meta_str(const char *src)
{
    int length = 0;
    char *dest = NULL;
    const char *p1 = src;
    char *p2 = NULL;

    if (!src)
    {
        return NULL;
    }

    length = sal_strlen(src);
    p2 = dest = (char *)malloc(2*length + 1);
    if (!p2)
    {
        return NULL;
    }
    sal_memset(dest, 0, 2*length + 1);

    while (*p1)
    {
        if ('%' == *p1)
        {
            *p2++ = *p1;
        }
        *p2++ = *p1++;
    }
    dest[2*length] = '\0';
    return dest;
}

/*******************************************************************************
 * Name:    _connectlog_r
 * Purpose: connecting to syslog-ng daemon
 * Input: 
 *   data: the log environment
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
static void
_connectlog_r(struct genlog_data *data)
{
    struct sal_sockaddr_un syslog_addr;

    if (-1 == data->log_file)
    {
        if (-1 == (data->log_file = sal_socket(AF_UNIX, SOCK_DGRAM, 0)))
        {
            return;
        }
        (void)sal_fcntl(data->log_file, F_SETFD, 1);
        /*set mode to block*/
        sal_sock_set_nonblocking(data->log_file, TRUE);
    }

    if (-1 != data->log_file && !data->connected)
    {
        sal_memset(&syslog_addr, 0, sizeof(syslog_addr));
        syslog_addr.sun_family = AF_UNIX;
        _log_strlcpy(syslog_addr.sun_path, _PATH_LOG,
                    sizeof(syslog_addr.sun_path));

        errno = 0;
        if (connect(data->log_file, (struct sal_sockaddr *)&syslog_addr,
                    sizeof(struct sal_sockaddr)) < 0)
        {
            if ((errno != EWOULDBLOCK) && (errno != EINPROGRESS))
            {
                (void)sal_close(data->log_file);
                data->log_file = -1;
            }
            else
            {
                data->connected = 1;
            }
        }
        else
        {
            data->connected = 1;
        }
    }
}

/*******************************************************************************
 * Name:    _openlog_r
 * Purpose: open log
 * Input: 
 *   ident: the pragram name
 *   logstat: log option
 *   logfac: facility 
 *   data: the log environment
 * Output: 
 * Return: N/A
 * Note: reentrant
 ******************************************************************************/
static void
_openlog_r(const char *ident, int logstat, int logfac, struct genlog_data *data)
{
    if (ident != NULL)
    {
        data->log_tag = ident;
    }
    data->log_stat = logstat;
    if (logfac != 0 && (logfac & ~LOG_FACMASK) == 0)
    {
        data->log_fac = logfac;
    }

    if (data->log_stat & LOG_NDELAY) /* open immediately */
    {
        _connectlog_r(data);
    }
    data->opened = 1;           /* ident and facility has been set */
}

/*******************************************************************************
 * Name:    _closelog_r
 * Purpose: clear log environment
 * Input: 
 *   data: the log environment 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
static void
_closelog_r(struct genlog_data *data)
{
    if (data->log_file != -1)
    {
        (void)sal_close(data->log_file);
    }
    data->log_file = -1;
    data->connected = 0;
    data->log_tag = NULL;
}

/*******************************************************************************
 * Name:    _format_oprt_log
 * Purpose: formatting operating log
 * Input: 
 *   pszOutput: result
 *   pLen: result length 
 * Output: pszOutput
 * Return: 
 *   success: FMT_SUCCESS 
 *   failed : FMT_FAIL 
 * Note: 
 ******************************************************************************/
static E_FMT_RTN
_format_oprt_log(char **pszOutput, size_t *pLen)
{
    SAL_ASSERT(pszOutput && pLen);
    if (NULL == s_extra_info.ip_addr)
    {
        return FMT_FAIL;
    }
#define LEFT_LEN 100
    *pLen = sal_strlen(s_extra_info.ip_addr) + sal_strlen(s_extra_info.usr_name) + LEFT_LEN;
    *pszOutput = (char *) malloc(*pLen + 1);

    if (!(*pszOutput))
    {
        return FMT_FAIL;
    }

    sal_snprintf(*pszOutput, *pLen, "user=%s;ip=%s;cmdlevel=%d;opresult=%d;",  
        s_extra_info.usr_name, s_extra_info.ip_addr, s_extra_info.cmd_level, s_extra_info.oprt_rtn);
    
    return FMT_SUCCESS;
}

/*******************************************************************************
 * Name:    _format_alrm_log
 * Purpose: formatting device alarm log
 * Input: 
 *   pszOutput: result
 *   pLen: result length 
 * Output: pszOutput
 * Return: 
 *   success: FMT_SUCCESS 
 *   failed : FMT_FAIL 
 * Note: 
 ******************************************************************************/
static E_FMT_RTN
_format_alrm_log(char **pszOutput, size_t *pLen)
{
    if (NULL == s_extra_info.ip_addr)
    {
        return FMT_FAIL;
    }
#define ALRM_PREFIX_LEN 10
    *pLen = ALRM_PREFIX_LEN;
    *pszOutput = (char*)malloc(*pLen + 1);
    sal_memset(*pszOutput, 0, *pLen + 1);

    if (!(*pszOutput))
    {
        return FMT_FAIL;
    }

    sal_snprintf(*pszOutput, *pLen, ALRM_LOG_PREFIX);
    
    return FMT_SUCCESS;
}

/*******************************************************************************
 * Name:    _vsyslog_r
 * Purpose: generating one log
 * Input: 
 *   pri: priority
 *   data: the global log environment
 *   fmt: the message
 *   ap: first variable parameter
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
static void
_vsyslog_r(int logid, int pri, struct genlog_data *data, const char *fmt, va_list ap)
{
    int cnt;
    char ch, *p, *t;
    time_t now;
    int saved_errno, error;
    int severity = pri;

#define TBUF_LEN    2048
#define FMT_LEN     1024
    char *stdp = NULL;
    char tbuf[TBUF_LEN];
    char fmt_cpy[FMT_LEN];
    int tbuf_left, fmt_left, prlen;
    struct timeval tz;
    static int sec = 0;

    signal(SIGPIPE, SIG_IGN);

    /* Check for invalid bits. */
    if (pri & ~(LOG_PRIMASK | LOG_FACMASK))
    {
        pri &= LOG_PRIMASK | LOG_FACMASK;
    }

    /* Check priority against setlogmask values. */
    if (!(LOG_MASK(LOG_PRI(pri)) & data->log_mask))
    {
        return;
    }

    saved_errno = errno;

    /* Set default facility if none specified. */
    if ((pri & LOG_FACMASK) == 0)
    {
        pri |= data->log_fac;
    }

    /* If we have been called through syslog(), no need for reentrancy. */
    ctc_task_boottime(&tz);
    if (tz.tv_sec != sec)
    {
        now = tz.tv_sec;
        sec = tz.tv_sec;
        strftime(s_log_data.log_time_str, MAX_LOG_TIME_LEN, "%h %e %T ", localtime(&now));
    }

    /*modified by weij for bug 18671, 2012-03-20*/
    if (s_log_data.rate_limit_type)
    {
        /*outof bound*/
        if (s_log_data.rate_limit_type >= LOG_RATE_LIMIT_TYPE_MAX)
        {
            return;
        }
        if (g_log_rate_limit_arr[s_log_data.rate_limit_type].next_sec > sec)
        {
            g_log_rate_limit_arr[s_log_data.rate_limit_type].suppress_cnt++;
            return;
        }
        g_log_rate_limit_arr[s_log_data.rate_limit_type].next_sec = 
                                        sec + g_log_rate_limit_arr[s_log_data.rate_limit_type].log_interval;
    }

    p = tbuf;
    tbuf_left = TBUF_LEN;

#define DEC()                         \
    do                                \
    {                                 \
        if (prlen < 0)                \
        {                             \
            prlen = 0;                \
        }                             \
        if (prlen >= tbuf_left)       \
        {                             \
            prlen = tbuf_left - 1;    \
        }                             \
        p += prlen;                   \
        tbuf_left -= prlen;           \
    }                                 \
    while (0);

    prlen = sal_snprintf(p, tbuf_left, "<%d>", pri);
    DEC();

    /* 
     * syslog-ng-d will expand time automagically for reentrant case, and
     * for normal case, just do like before
     */
    prlen = sal_snprintf(p, tbuf_left, "%s", s_log_data.log_time_str);
    DEC();

    if (data->log_stat & LOG_PERROR)
    {
        stdp = p;
    }
    if (data->log_tag == NULL)
    {
        data->log_tag = "genlog";
    }
    if (data->log_tag != NULL)
    {
        if (data->is_percent)
        {
            prlen = sal_snprintf(p, tbuf_left, "%%%s", data->log_tag);
        }
        else
        {
            prlen = sal_snprintf(p, tbuf_left, "%s", data->log_tag);
        }
        DEC();
    }
    if (data->log_stat & LOG_PID)
    {
        prlen = sal_snprintf(p, tbuf_left, "[%ld]", (long) sal_getpid());
        DEC();
    }
    if (data->log_tag != NULL)
    {
        if (tbuf_left > 1)
        {
            *p++ = ':';
            tbuf_left--;
        }
        if (tbuf_left > 1)
        {
            *p++ = ' ';
            tbuf_left--;
        }
    }

    /* strerror() is not reentrant */
    for (t = fmt_cpy, fmt_left = FMT_LEN; (ch = *fmt); ++fmt)
    {
        if (ch == '%' && fmt[1] == 'm')
        {
            ++fmt;
            if (data == &s_log_data)
            {
                prlen = sal_snprintf(t, fmt_left, "%s", sal_strerror(saved_errno));
            }
            else
            {
                prlen = sal_snprintf(t, fmt_left, "Error %d", saved_errno);
            }
            if (prlen < 0)
            {
                prlen = 0;
            }
            if (prlen >= fmt_left)
            {
                prlen = fmt_left - 1;
            }
            t += prlen;
            fmt_left -= prlen;
        }
        else if (ch == '%' && fmt[1] == '%' && fmt_left > 2)
        {
            *t++ = '%';
            *t++ = '%';
            fmt++;
            fmt_left -= 2;
        }
        else
        {
            if (fmt_left > 1)
            {
                *t++ = ch;
                fmt_left--;
            }
        }
    }
    *t = '\0';

    if (_format_oprt_log == s_extra_info.fmt_func)
    {
        E_FMT_RTN fmt_rtn = FMT_FAIL;
        char *pszOutput = NULL;
        size_t len = 0;
        
        /* Format extra log info if required */
        fmt_rtn = s_extra_info.fmt_func(&pszOutput, &len);
        prlen = sal_snprintf(p, tbuf_left, pszOutput);
        DEC();
        if (FMT_FAIL != fmt_rtn)
        {
            free(pszOutput);
            pszOutput = NULL;
        }
    }

    if (_format_alrm_log == s_extra_info.fmt_func)
    {
        E_FMT_RTN fmt_rtn = FMT_FAIL;
        char *pszOutput = NULL;
        size_t len = 0;

        prlen = sal_vsnprintf(p, tbuf_left, fmt_cpy, ap);
        DEC();
        
        /* Format extra log info if required */
        fmt_rtn = s_extra_info.fmt_func(&pszOutput, &len);
        prlen = sal_snprintf(p, tbuf_left, pszOutput);
        DEC();
        if (FMT_FAIL != fmt_rtn)
        {
            free(pszOutput);
            pszOutput = NULL;
        }
    }

    else if (_format_oprt_log == s_extra_info.fmt_func)
    {
        prlen = sal_snprintf(p, tbuf_left, fmt_cpy);
        DEC();
    }

    else
    {
        prlen = sal_vsnprintf(p, tbuf_left, fmt_cpy, ap);
        DEC();
    }

    prlen = sal_snprintf(p, tbuf_left, "\n");
    DEC();
    
    cnt = p - tbuf;

    /* Output to stderr if required */
    if (data->log_stat & LOG_PERROR)
    {
        struct iovec iov[2];

        iov[0].iov_base = stdp;
        iov[0].iov_len = cnt - (stdp - tbuf);
        iov[1].iov_base = "\n";
        iov[1].iov_len = 1;
        (void)sal_writev(STDERR_FILENO, iov, 2);
    }

    /* Get connected, output the message to the local logger. */
    if (!data->opened)
    {
        _openlog_r(data->log_tag, data->log_stat, 0, data);
    }
    _connectlog_r(data);

    errno = 0;
    if ((error = sal_send(data->log_file, tbuf, cnt, 0)) < 0)
    {
        if ((errno != EWOULDBLOCK) && (errno != EINPROGRESS))
        {
            /* syslog-ng maybe restart, so we reopen the connection,
               then send this message again */
            if (errno != ENOBUFS) 
            {
                _closelog_r(data);
                _openlog_r("", LOG_CONS | LOG_NDELAY, LOG_USER, &s_log_data);
                error = sal_send(data->log_file, tbuf, cnt, 0);
            }
        }
    }

    if (INVALID_LOGID != logid)
    {
        if (s_extra_info.cb_func)
        {
            s_extra_info.cb_func(logid, severity, tbuf);
        }
    }
    
    if (data != &s_log_data)
    {
        _closelog_r(data);
    }
}

/*******************************************************************************
 * Name:    genlog_open
 * Purpose: open genlog log
 * Input: N/A
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void
genlog_open(void)
{
    _openlog_r("", LOG_CONS | LOG_NDELAY, LOG_USER, &s_log_data);
}

/*******************************************************************************
 * Name:    genlog_close
 * Purpose: close genlog
 * Input: N/A
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void
genlog_close(void)
{
    _closelog_r(&s_log_data);
}

/*******************************************************************************
 * Name:    log_init_line
 * Purpose: initialize access information
 * Input: 
 *   ip_addr: the client ip address
 *   usr_name: the access user name 
 * Output: 
 * Return: N/A
 * Note: 
 ******************************************************************************/
void 
log_init_line(const char *ip_addr, const char *usr_name)
{
    SAL_ASSERT(NULL != ip_addr && NULL != usr_name);
    _log_strlcpy(s_extra_info.ip_addr, ip_addr, MAX_IP_LEN+1);
    _log_strlcpy(s_extra_info.usr_name, usr_name, MAX_USR_NAME_LEN+1);
}

/*******************************************************************************
 * Name:    log_sys
 * Purpose: add one system log
 * Input: 
 *   eModule: module id
 *   eSeverity: log severity level
 *   fmt: the log content
 * Output: 
 * Return: N/A
 * Note: log length should be limited in 1024 bytes 
 ******************************************************************************/
void
log_sys(E_MODULE eModule, E_SEVERITY eSeverity, const char *fmt, ...)
{
    va_list ap;

    SAL_ASSERT(eModule > M_MOD_UNKNOWN && eModule < M_MOD_MAX 
        && eSeverity >= E_EMERGENCY && eSeverity <= E_DEBUG);

    s_log_data.log_tag = (const char *)gModuleNameArr[eModule].pszName;
    if (!s_log_data.log_tag)
    {
        return;
    }
    /*modified by weij for bug 18671, 2012-03-20*/
    s_log_data.rate_limit_type = LOG_RATE_LIMIT_TYPE_DEF;
    s_extra_info.fmt_func = NULL;
    va_start(ap, fmt);
    _vsyslog_r(INVALID_LOGID, eSeverity, &s_log_data, fmt, ap);
    va_end(ap);
}

/*modified by weij for bug 18671, 2012-03-20*/
/*******************************************************************************
 * Name:    log_sys_rate_limit
 * Purpose: add one system log while may drop by ratelimit
 * Input: 
 *   eModule: module id
 *   eSeverity: log severity level
 *   uint16: rate limit type
 *   fmt: the log content
 * Output: 
 * Return: N/A
 * Note: log length should be limited in 1024 bytes 
 ******************************************************************************/
void
log_sys_rate_limit(E_MODULE eModule, E_SEVERITY eSeverity, int rateLimitType, const char *fmt, ...)
{
    va_list ap;

    SAL_ASSERT(eModule > M_MOD_UNKNOWN && eModule < M_MOD_MAX 
        && eSeverity >= E_EMERGENCY && eSeverity <= E_DEBUG);

    s_log_data.log_tag = (const char *)gModuleNameArr[eModule].pszName;
    if (!s_log_data.log_tag)
    {
        return;
    }
    s_log_data.rate_limit_type = rateLimitType;
    s_extra_info.fmt_func = NULL;
    va_start(ap, fmt);
    _vsyslog_r(INVALID_LOGID, eSeverity, &s_log_data, fmt, ap);
    va_end(ap);
}

/*******************************************************************************
 * Name:    log_oper
 * Purpose: add on operating log
 * Input: 
 *   eModule: module id
 *   eSeverity: log severity level 
 *   cl: command level
 *   rlt: operating result
 *   fmt: the log content
 * Output: 
 * Return: N/A
 * Note: log length should be limited in 1024 bytes
 ******************************************************************************/
void
log_oper(E_MODULE eModule, E_SEVERITY eSeverity, CMD_LEVEL cl, E_OPRT_RLT rlt,
         const char *fmt, ...)
{
    va_list ap;
    int old_facility;
    char *real_oper = NULL;

    SAL_ASSERT(eModule > M_MOD_UNKNOWN && eModule < M_MOD_MAX 
        && eSeverity >= E_EMERGENCY && eSeverity <= E_DEBUG);
    
    s_log_data.log_tag = (const char *) gModuleNameArr[eModule].pszName;
    if (!s_log_data.log_tag)
    {
        return;
    }

    old_facility = s_log_data.log_fac;
    s_log_data.log_fac = LOG_LOCAL5;
    /*modified by weij for bug 18671, 2012-03-20*/
    s_log_data.rate_limit_type = LOG_RATE_LIMIT_TYPE_DEF;

    s_extra_info.fmt_func = &_format_oprt_log;
    s_extra_info.oprt_rtn = rlt;
    s_extra_info.cmd_level = cl;
    
    real_oper = _copy_meta_str(fmt);
    if (!real_oper)
    {
        return;
    }

    va_start(ap, fmt);
    _vsyslog_r(INVALID_LOGID, eSeverity, &s_log_data, real_oper, ap);
    va_end(ap);

    free(real_oper);
    real_oper = NULL;
    s_log_data.log_fac = old_facility;
}

/* add by chenzh for add diag log, 2011-09-26*/
/*******************************************************************************
 * Name:    log_diag
 * Purpose: add an diag log
 * Input: 
 *   eModule: module id
 *   eSeverity: log severity level 
 *   cl: command level
 *   rlt: operating result
 *   fmt: the log content
 * Output: 
 * Return: N/A
 * Note: log length should be limited in 1024 bytes
 ******************************************************************************/
void
log_diag(E_MODULE eModule, E_SEVERITY eSeverity, const char *fmt, ...)
{
    va_list ap;
    E_SEVERITY sev;
    int old_facility;

    SAL_ASSERT(eModule > M_MOD_UNKNOWN && eModule < M_MOD_MAX 
        && eSeverity >= E_EMERGENCY && eSeverity <= E_DEBUG);

    old_facility = s_log_data.log_fac;
    s_log_data.log_fac = LOG_LOCAL7;
    /*modified by weij for bug 18671, 2012-03-20*/
    s_log_data.rate_limit_type = LOG_RATE_LIMIT_TYPE_DEF;
    
    sev = (E_SEVERITY)eSeverity;
    s_log_data.log_tag = (const char *) gModuleNameArr[eModule].pszName;
    if (!s_log_data.log_tag)
    {
        return;
    }
    s_extra_info.fmt_func = NULL;
       
    va_start(ap, fmt);
    _vsyslog_r(INVALID_LOGID, sev, &s_log_data, fmt, ap);
    va_end(ap);

    s_log_data.log_fac = old_facility;
}


/*modified by weij for bug 18671, 2012-03-20*/
/*******************************************************************************
 * Name:    log_diag_rate_limit
 * Purpose: add one system log while may drop by ratelimit
 * Input: 
 *   eModule: module id
 *   eSeverity: log severity level
 *   uint16: rate limit type
 *   fmt: the log content
 * Output: 
 * Return: N/A
 * Note: log length should be limited in 1024 bytes 
 ******************************************************************************/
void
log_diag_rate_limit(E_MODULE eModule, E_SEVERITY eSeverity, int rateLimitType, const char *fmt, ...)
{
    va_list ap;
    E_SEVERITY sev;
    int old_facility;

    SAL_ASSERT(eModule > M_MOD_UNKNOWN && eModule < M_MOD_MAX 
        && eSeverity >= E_EMERGENCY && eSeverity <= E_DEBUG);

    old_facility = s_log_data.log_fac;
    s_log_data.log_fac = LOG_LOCAL7;

    sev = (E_SEVERITY)eSeverity;
    s_log_data.log_tag = (const char *) gModuleNameArr[eModule].pszName;
    if (!s_log_data.log_tag)
    {
        return;
    }
    s_log_data.rate_limit_type = rateLimitType;
    s_extra_info.fmt_func = NULL;

    va_start(ap, fmt);
    _vsyslog_r(INVALID_LOGID, sev, &s_log_data, fmt, ap);
    va_end(ap);

    s_log_data.log_fac = old_facility;
}

/*******************************************************************************
 * Name:    log_get_rate_limit_arr
 * Purpose: return glb log data
 * Input: None
 * Output: 
 * Return: glb_log_rate_limit_arr
 ******************************************************************************/
log_message_rate_limit_t *
log_get_rate_limit_arr(void)
{
    return g_log_rate_limit_arr;
}

void
log_set_nonblocking(int set)
{
    if (-1 == s_log_data.log_file)
    {
        return;
    }
    if (set)
    {
        sal_sock_set_nonblocking(s_log_data.log_file, TRUE);
    }
    else
    {
        sal_sock_set_nonblocking(s_log_data.log_file, FALSE);
    }
    return;
}

void
logid_sys(E_LOGID logid, ...)
{
    const char *fmt = NULL;
    E_SEVERITY eSeverity = 0;
    va_list ap;

    SAL_ASSERT(logid < LOG_ID_MAX);

    s_osp_log_data.log_tag = (const char *)gLogInfoArr[logid].logid_str;
    if (!s_osp_log_data.log_tag)
    {
        return;
    }
    fmt = (const char *)gLogInfoArr[logid].format_str;

    eSeverity = gLogInfoArr[logid].severity;

    s_osp_log_data.rate_limit_type = LOG_RATE_LIMIT_TYPE_DEF;
    s_extra_info.fmt_func = NULL;

    va_start(ap, logid);
    _vsyslog_r(logid, eSeverity, &s_osp_log_data, fmt, ap);
    va_end(ap);
}

const char*
logid_get_desc(E_LOGID logid)
{
    if (logid >= LOG_ID_MAX)
    {
        return "Invalid";
    }

    return gLogInfoArr[logid].logid_str;    
}

void
logid_register_cb_func(LOG_CB_FUNC func)
{
    s_extra_info.cb_func = func;
}

void logit(int eSeverity, int syserr, const char *fmt, ...)
{
    va_list ap;

    SAL_ASSERT( eSeverity >= E_EMERGENCY && eSeverity <= E_DEBUG);

    s_log_data.log_tag = (const char *)gModuleNameArr[M_MOD_PIMSM].pszName;
    if (!s_log_data.log_tag)
    {
        return;
    }
    s_log_data.rate_limit_type = LOG_RATE_LIMIT_TYPE_DEF;
    s_extra_info.fmt_func = NULL;
    va_start(ap, fmt);
    _vsyslog_r(INVALID_LOGID, eSeverity, &s_log_data, fmt, ap);
    va_end(ap);	

}
