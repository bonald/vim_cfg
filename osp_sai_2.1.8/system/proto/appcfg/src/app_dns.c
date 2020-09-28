
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "proto.h"
#include "gen/tbl_static_dns_define.h"
#include "gen/tbl_static_dns.h"
#include "gen/tbl_dynamic_dns_domain_define.h"
#include "gen/tbl_dynamic_dns_domain.h"
#include "gen/tbl_dynamic_dns_server_define.h"
#include "gen/tbl_dynamic_dns_server.h"
#include "appcfg.h"
#include "app_dns.h"


int32
_app_static_dns_add_cfg(char *hostname, char *ip_addr)
{
    tbl_static_dns_key_t static_dns_key;
    tbl_static_dns_t *p_db_static_dns = NULL;
    tbl_static_dns_t  p_static_cfg;
    int32 rc = PM_E_NONE;

    sal_memset(&p_static_cfg, 0, sizeof(p_static_cfg));
    sal_strncpy(static_dns_key.name, hostname, DNS_HOSTNAME_SIZE);
    p_db_static_dns = tbl_static_dns_get_static_dns(&static_dns_key);
    
    if(p_db_static_dns != NULL)
    {
        cdb_addr_str2val(&p_static_cfg.ip_addr, ip_addr, IPADDR_BUF_SIZE);
        p_db_static_dns->ip_addr = p_static_cfg.ip_addr;
        rc = tbl_static_dns_set_static_dns_field(p_db_static_dns, TBL_STATIC_DNS_FLD_IP_ADDR);
        
    }
    else
    {
        sal_strncpy(p_static_cfg.key.name, hostname, DNS_HOSTNAME_SIZE);
        cdb_addr_str2val(&p_static_cfg.ip_addr, ip_addr, IPADDR_BUF_SIZE);
        rc = tbl_static_dns_add_static_dns(&p_static_cfg);
    }
    
    if(rc < 0)
    {
        return rc;
    }
    
    rc = tbl_static_dns_save_cfg_file();
    
    
    return rc;
}

int32
_app_static_dns_del_cfg(char *hostname)
{
    tbl_static_dns_t  p_static_cfg;
    int32 rc = PM_E_NONE;

    sal_memset(&p_static_cfg, 0, sizeof(p_static_cfg));
    sal_strncpy( p_static_cfg.key.name, hostname, DNS_HOSTNAME_SIZE);
    rc = tbl_static_dns_del_static_dns(&p_static_cfg.key);
    
    
    if(rc < 0)
    {
        return rc;
    }
    
    rc = tbl_static_dns_save_cfg_file();
    
    
    return rc;
}

int32
_app_static_dns_get_ip_by_hostname(char *hostname, char *ip_addr)
{
    tbl_static_dns_key_t static_dns_key;
    tbl_static_dns_t *p_db_static_dns = NULL;
    int32 rc = PM_E_NONE;

    sal_memset(&static_dns_key, 0, sizeof(static_dns_key));
    sal_strncpy(static_dns_key.name, hostname, DNS_HOSTNAME_SIZE);
    p_db_static_dns = tbl_static_dns_get_static_dns(&static_dns_key);
    
    if(p_db_static_dns != NULL)
    {
      cdb_addr_val2str(ip_addr, IPADDR_BUF_SIZE, &p_db_static_dns->ip_addr);     
    }
    else
    {
        return PM_E_FAIL;
    }
    
    return rc;
}


int32
_app_dynamic_dns_domain_add_cfg(char *domain_name)
{
    tbl_dynamic_dns_domain_t  p_domain_cfg;
    int32 rc = PM_E_NONE;
    
    sal_memset(&p_domain_cfg, 0, sizeof(p_domain_cfg));
    sal_strncpy(p_domain_cfg.key.name, domain_name, DNS_HOSTNAME_SIZE);
    
    rc = tbl_dynamic_dns_domain_add_dynamic_dns_domain(&p_domain_cfg);
      
    if(rc < 0)
    {
        return rc;
    }
    
    rc = tbl_dynamic_dns_save_cfg_file();
        
    return rc;
}

int32
_app_dynamic_dns_domain_del_cfg(char *domain_name)
{
    tbl_dynamic_dns_domain_t  p_domain_cfg;
    int32 rc = PM_E_NONE;
    
    sal_memset(&p_domain_cfg, 0, sizeof(p_domain_cfg));
    sal_strncpy(p_domain_cfg.key.name, domain_name, DNS_HOSTNAME_SIZE);
    
    rc = tbl_dynamic_dns_domain_del_dynamic_dns_domain(&p_domain_cfg.key);
      
    if(rc < 0)
    {
        return rc;
    }
    
    rc = tbl_dynamic_dns_save_cfg_file();
        
    return rc;
}

int32
_app_dynamic_dns_server_add_cfg(tbl_dynamic_dns_server_t *p_dynamic_server)
{
    int32 rc = PM_E_NONE;
    
    rc = tbl_dynamic_dns_server_add_dynamic_dns_server(p_dynamic_server);
      
    if(rc < 0)
    {
        return rc;
    }
    
    rc = tbl_dynamic_dns_save_cfg_file();
        
    return rc;
}

int32
_app_dynamic_dns_server_del_cfg(tbl_dynamic_dns_server_key_t *p_dynamic_server_key)
{
    int32 rc = PM_E_NONE;
    
    rc = tbl_dynamic_dns_server_del_dynamic_dns_server(p_dynamic_server_key);
      
    if(rc < 0)
    {
        return rc;
    }
    
    rc = tbl_dynamic_dns_save_cfg_file();
        
    return rc;
}


int32
tbl_static_dns_save_cfg_file()
{
    char szBuf[BLOCK_SIZE] = {0};
    FILE *fp = NULL;
    FILE *fp_tmp = NULL;
    bool skip_flag = FALSE;
    tbl_iter_args_t iter_args;

    /* open host file */
    if ((fp = fopen(STATIC_HOST_FILE, "a+")) == NULL)
    {
        return PM_E_FAIL;
    }
    
    fseek(fp, 0, SEEK_SET);

    /* open tmp file */
    if ((fp_tmp = fopen(STATIC_HOST_TMPFILE, "w+")) == NULL)
    {
        fclose(fp);
        return PM_E_FAIL;
    }

    while (fgets(szBuf, BLOCK_SIZE, fp) != NULL)
    {
        /* skip rewrite old host file data */
        if (TRUE == skip_flag)
        {
            if (!sal_strncmp(szBuf, STR_ADDED_END, sal_strlen(STR_ADDED_END)))
            {
                skip_flag = FALSE;
            }
            continue;
        }
        else if (!sal_strncmp(szBuf, STR_ADDED_BEGIN, sal_strlen(STR_ADDED_BEGIN)))
        {
            skip_flag = TRUE;
            continue;
        }
        
        fprintf(fp_tmp, "%s", szBuf);
    }

    /* write our item to the tail of the host file */
    fprintf(fp_tmp, STR_ADDED_BEGIN);

    iter_args.argv[0] = fp_tmp;
    tbl_static_dns_iterate((TBL_ITER_CB_FUNC)tbl_static_dns_save_cfg, &iter_args);

    fprintf(fp_tmp, STR_ADDED_END);

    fclose(fp);
    fclose(fp_tmp);

    rename(STATIC_HOST_TMPFILE, STATIC_HOST_FILE);
    return PM_E_NONE;
}

int32 
tbl_dynamic_dns_save_cfg_file()
{
    FILE * savefile = fopen(DYNAMIC_HOST_FILE, "w+");
    char szBuf[BLOCK_SIZE] = {0};
    //char szIpBuf[47] = {0};
    tbl_iter_args_t iter_args;
    tbl_dynamic_dns_domain_master_t *p_dynamic_domain_master = tbl_dynamic_dns_domain_get_master();
    tbl_dynamic_dns_server_master_t *p_dynamic_server_master = tbl_dynamic_dns_server_get_master();
    if (NULL == savefile)
    {
        return PM_E_FAIL;
    }
    if (p_dynamic_server_master->dynamic_server_list->count > 0)
    {
        iter_args.argv[1] = savefile;
        tbl_dynamic_dns_server_iterate((TBL_ITER_CB_FUNC)tbl_dynamic_dns_server_save_cfg, &iter_args);
        savefile = iter_args.argv[1];
    }
    sal_memset(szBuf, 0, BLOCK_SIZE);
    
    if (p_dynamic_domain_master->dynamic_domain_list->count == 1)
    {   
        iter_args.argv[1] = savefile;
        tbl_dynamic_dns_domain_iterate((TBL_ITER_CB_FUNC)tbl_dynamic_dns_domain_save_cfg, &iter_args);

    }
    else if (p_dynamic_domain_master->dynamic_domain_list->count > 1)
    {
        char *location = szBuf;
        const char *tag = STR_SEARCH_TAG;

        sal_snprintf(location, BLOCK_SIZE, "%s", tag);
        location = location + sal_strlen(tag);

        iter_args.argv[0] = location;
        tbl_dynamic_dns_domain_iterate((TBL_ITER_CB_FUNC)tbl_dynamic_dns_domain_save_cfg, &iter_args);

        location = iter_args.argv[0];
        *location = '\n';
        sal_fputs(szBuf, savefile);
    }
    

    sal_fclose(savefile);

    return PM_E_NONE;
}

int32
tbl_static_dns_save_cfg(void *data, tbl_iter_args_t *pargs)
{
    tbl_static_dns_t *p_static_dns  = (tbl_static_dns_t *)data;
    FILE *fp_tmp = pargs->argv[0];
    char szIpBuf[47] = {0};

    if (NULL != p_static_dns)
    {
        cdb_addr_val2str(szIpBuf, 47, &p_static_dns->ip_addr);
        sal_fprintf(fp_tmp, "%s %s\n", szIpBuf, p_static_dns->key.name);
    }
    
    return PM_E_NONE;
}

int32
tbl_dynamic_dns_domain_save_cfg(void *data, tbl_iter_args_t *pargs)
{
    tbl_dynamic_dns_domain_t *p_dynamic_domain  = (tbl_dynamic_dns_domain_t *)data;
    char *location = pargs->argv[0]; 
    FILE *savefile = pargs->argv[1];
    char szBuf[BLOCK_SIZE] = {0};
    tbl_dynamic_dns_domain_master_t *p_dynamic_domain_master = tbl_dynamic_dns_domain_get_master();
    
    if (NULL != p_dynamic_domain)
    {       
        if (p_dynamic_domain_master->dynamic_domain_list->count == 1)
        {   
            const char *tag = STR_DOMAIN_TAG;
            sal_snprintf(szBuf, BLOCK_SIZE, "%s\t%s\n", tag, p_dynamic_domain->key.name);
            fputs(szBuf, savefile);

        }
        else if (p_dynamic_domain_master->dynamic_domain_list->count > 1)
        {
            sal_snprintf(location, BLOCK_SIZE, "\t%s", p_dynamic_domain->key.name);
            location = location + sal_strlen(p_dynamic_domain->key.name) + 1; 
            pargs->argv[0] = location;
        }

    }
    
    return PM_E_NONE;
}

int32
tbl_dynamic_dns_server_save_cfg(void *data, tbl_iter_args_t *pargs)
{
    tbl_dynamic_dns_server_t *p_dynamic_server  = (tbl_dynamic_dns_server_t *)data;
    FILE *savefile = pargs->argv[1];
    char szBuf[BLOCK_SIZE] = {0};
    char szIpBuf[47] = {0};

    if (NULL != p_dynamic_server)
    {
         cdb_addr_val2str(szIpBuf, 47, &p_dynamic_server->key.ip_addr);
         sal_snprintf(szBuf, BLOCK_SIZE, "%s\t%s\n", STR_NAME_SERVER_TAG, szIpBuf);
         sal_fputs(szBuf, savefile);
         pargs->argv[1] = savefile;
    }
    
    return PM_E_NONE;
}

