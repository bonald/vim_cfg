
#ifndef __APPCFG_H__
#define __APPCFG_H__

#define M_TRIM_CR(szLine) \
do { \
    char *_p = szLine + strlen(szLine) - 1; \
    if (*_p == '\n') { \
        *_p = '\0'; \
        _p--; \
    } \
    if (*_p == '\r') { \
        *_p = '\0'; \
    } \
} while (0); \

#endif /* !__APPCFG_H__ */

