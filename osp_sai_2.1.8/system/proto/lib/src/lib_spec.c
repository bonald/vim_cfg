#include "proto.h"
#include "lib_spec.h"
#include "lib_tblinc.h"
#include "oem_info.h"

oem_info_t g_st_oem_info;
int g_oem_init = 0;

uint32
lib_spec_get_field(uint32 field_id)
{
    tbl_chassis_t* p_chassis = NULL;
    tbl_sys_spec_t* p_spec = NULL;
    int32 type = 0;

    p_chassis = tbl_chassis_get_chassis();
    if(p_chassis)
    {
        type = p_chassis->cur_stm_mode;
    }
    else
    {
        return PM_E_NOT_INITED;
    }
    
    p_spec = tbl_sys_spec_get_sys_spec_by_type(type);
    if (NULL == p_spec)
    {
        return 0;
    }

    return tbl_sys_spec_get_sys_spec_field_by_type(type, field_id);
}

void
oem_info_init(void)
{
    FILE *fp;
    char buf[OEM_INFO_BUFSIZ];

    sal_memset(&g_st_oem_info, 0, sizeof(oem_info_t));
    sal_strncpy(g_st_oem_info.company_name, "Unknown", OEM_INFO_BUFSIZ);
    sal_strncpy(g_st_oem_info.package_name, "Unknown", OEM_INFO_BUFSIZ);
    sal_strncpy(g_st_oem_info.product_name, "Unknown", OEM_INFO_BUFSIZ);
    sal_strncpy(g_st_oem_info.hardware_type, "Unknown", OEM_INFO_BUFSIZ);
    sal_strncpy(g_st_oem_info.snmp_enterprise_oid, "27975", OEM_INFO_BUFSIZ);
    
    fp = sal_fopen(OEM_INFO_FILE, "r");
    if (NULL == fp)
    {
        log_sys(M_MOD_LIB, E_ERROR, "OEM information file is not exist.");
        return;
    }

    while (sal_fgets(buf, OEM_INFO_BUFSIZ, fp) != NULL)
    {
        buf[sal_strlen(buf)-1]= '\0';
        if(!sal_strncmp(buf, OEM_COMPANY_FULL_NAME, sal_strlen(OEM_COMPANY_FULL_NAME)))
        {
            sal_memcpy(g_st_oem_info.company_name, buf+sizeof(OEM_COMPANY_FULL_NAME), 
                    OEM_INFO_BUFSIZ-sizeof(OEM_COMPANY_FULL_NAME));
        }
        else if(!sal_strncmp(buf, OEM_PACKAGE_NAME, sal_strlen(OEM_PACKAGE_NAME)))
        {
            sal_memcpy(g_st_oem_info.package_name, buf+sizeof(OEM_PACKAGE_NAME), 
                    OEM_INFO_BUFSIZ-sizeof(OEM_PACKAGE_NAME));
        }
        else if(!sal_strncmp(buf, OEM_PRODUCT_NAME, sal_strlen(OEM_PRODUCT_NAME)))
        {
            sal_memcpy(g_st_oem_info.product_name, buf+sizeof(OEM_PRODUCT_NAME), 
                    OEM_INFO_BUFSIZ-sizeof(OEM_PRODUCT_NAME));
        }
        else if(!sal_strncmp(buf, OEM_HARDWARE_TYPE, sal_strlen(OEM_HARDWARE_TYPE)))
        {
            sal_memcpy(g_st_oem_info.hardware_type, buf+sizeof(OEM_HARDWARE_TYPE), 
                    OEM_INFO_BUFSIZ-sizeof(OEM_HARDWARE_TYPE));
        }
        else if(!sal_strncmp(buf, OEM_SNMP_ENTERPRISE_OID, sal_strlen(OEM_SNMP_ENTERPRISE_OID)))
        {
            sal_memcpy(g_st_oem_info.snmp_enterprise_oid, buf+sizeof(OEM_SNMP_ENTERPRISE_OID), 
                    OEM_INFO_BUFSIZ-sizeof(OEM_SNMP_ENTERPRISE_OID));
        }
        else if(!sal_strncmp(buf, OEM_CHASSIS_TYPE, sal_strlen(OEM_CHASSIS_TYPE)))
        {
            sal_memcpy(g_st_oem_info.chassis_type, buf+sizeof(OEM_CHASSIS_TYPE), 
                    OEM_INFO_BUFSIZ-sizeof(OEM_CHASSIS_TYPE));
        }
    }    

    sal_fclose(fp);
  
    return;
}


char* oem_info_get_info_by_type(int type, char *buf)
{
    if (g_oem_init == 0)
    {
        oem_info_init();
        g_oem_init = 1;
    }

    switch (type)
    {
        case OEM_TYPE_COMPANY_NAME:
            if (buf)
                sal_snprintf(buf, OEM_INFO_BUFSIZ, g_st_oem_info.company_name);
            return g_st_oem_info.company_name;    
            
        case OEM_TYPE_PACKAGE_NAME:
            if (buf)
                sal_snprintf(buf, OEM_INFO_BUFSIZ, g_st_oem_info.package_name);
            return g_st_oem_info.package_name;    

        case OEM_TYPE_PRODUCT_NAME:
            if (buf)
                sal_snprintf(buf, OEM_INFO_BUFSIZ, g_st_oem_info.product_name);
            return g_st_oem_info.product_name;    

        case OEM_TYPE_HARDWARE_TYPE:
            if (buf)
                sal_snprintf(buf, OEM_INFO_BUFSIZ, g_st_oem_info.hardware_type);
            return g_st_oem_info.hardware_type;    

        case OEM_TYPE_SNMP_ENTERPRISE_OID:
            if (buf)
                sal_snprintf(buf, OEM_INFO_BUFSIZ, g_st_oem_info.snmp_enterprise_oid);
            return g_st_oem_info.snmp_enterprise_oid; 
  
        default:
            break;
    }    

    return NULL;
}

char* oem_info_get_company_name(char *buf)
{
    return oem_info_get_info_by_type(OEM_TYPE_COMPANY_NAME, buf);
}

char* oem_info_get_package_name(char *buf)
{
    return oem_info_get_info_by_type(OEM_TYPE_PACKAGE_NAME, buf);
}

char* oem_info_get_product_name(char *buf)
{
    return oem_info_get_info_by_type(OEM_TYPE_PRODUCT_NAME, buf);
}

char* oem_info_get_hardware_type(char *buf)
{
    return oem_info_get_info_by_type(OEM_TYPE_HARDWARE_TYPE, buf);
}

char* oem_info_get_chassis_type(char *buf)
{
    return oem_info_get_info_by_type(OEM_TYPE_CHASSIS_TYPE, buf);
}

char* oem_info_get_snmp_enterprise_oid(char *buf)
{
    return oem_info_get_info_by_type(OEM_TYPE_SNMP_ENTERPRISE_OID, buf);
}

int oem_info_get_single_snmp_enterprise_oid()
{
    char enterprise_oid[OEM_INFO_BUFSIZ];
    
    oem_info_get_snmp_enterprise_oid(enterprise_oid);
    return sal_atoi(enterprise_oid);
}

int
oem_info_get_oem_vendor_type()
{
    char package[OEM_INFO_BUFSIZ];
    
    oem_info_get_package_name(package);

    return OEM_VENDOR_TYPE_CENTEC;
}


