
#ifndef __DHCPCFG_H__
#define __DHCPCFG_H__

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


int32
dhclient_tbl_init();

int32
dhclient_proc_init_done();

int32
dhclient_proc_startup_done();

int32
dhclient_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
dhclient_port_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
dhclient_dbg_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);


#endif /* !__DHCPCFG_H__ */

