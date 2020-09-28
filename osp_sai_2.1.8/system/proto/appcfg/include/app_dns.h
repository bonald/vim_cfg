#ifndef __APP_DNS_H__
#define __APP_DNS_H__

#define   STR_ADDED_BEGIN        "#{{############################################\n"
#define   STR_ADDED_END          "#}}############################################\n"
#define   STATIC_HOST_FILE       "/etc/hosts"
#define   STATIC_HOST_TMPFILE    "/etc/hosts.tmp"
#define   DYNAMIC_HOST_FILE      "/etc/resolv.conf"
#define   BLOCK_SIZE    512

#define   MAX_STATIC_HOST_ENTRY  1024
#define   MAX_DYNAMIC_DNS_DOMAIN_NUM  6
#define   MAX_DYNAMIC_DNS_SERVER_NUM  3

#define   STR_NAME_SERVER_TAG         "nameserver"
#define   STR_DOMAIN_TAG             "domain"
#define   STR_SEARCH_TAG             "search"



int32
app_static_dns_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_dynamic_dns_domain_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
app_dynamic_dns_server_cmd_process(char **argv, int32 argc, cfg_cmd_para_t *para);

int32
_app_dynamic_dns_domain_add_cfg(char *domain_name);

int32
_app_dynamic_dns_domain_del_cfg(char *domain_name);

int32
_app_static_dns_add_cfg(char *hostname, char *ip_addr);

int32
_app_static_dns_del_cfg(char *hostname);


int32
_app_static_dns_get_ip_by_hostname(char *hostname, char *ip_addr);


int32
_app_dynamic_dns_server_add_cfg(tbl_dynamic_dns_server_t *p_dynamic_server);

int32
_app_dynamic_dns_server_del_cfg(tbl_dynamic_dns_server_key_t *p_dynamic_server_key);

int32
tbl_static_dns_save_cfg_file();

int32
tbl_static_dns_save_cfg(void *data, tbl_iter_args_t *pargs);

int32 
tbl_dynamic_dns_save_cfg_file();

int32
tbl_dynamic_dns_domain_save_cfg(void *data, tbl_iter_args_t *pargs);

int32
tbl_dynamic_dns_server_save_cfg(void *data, tbl_iter_args_t *pargs);

#endif /* !__APP_DNS_H__ */

