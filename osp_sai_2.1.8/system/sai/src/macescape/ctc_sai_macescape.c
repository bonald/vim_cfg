#include <sai.h>
#include <sal.h>
#include "ctc_api.h"
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_debug.h>
#include <ctc_sai_macescape.h>

static struct ctc_sai_macescape_info_s ctc_sai_macescape_info;

sai_status_t
ctc_sai_create_mac_escape_entry(
    _In_ const sai_mac_escape_entry_t *macescape_entry)
{
    int32  ret = SAI_STATUS_SUCCESS;
    uint32 entry_index = ctc_sai_macescape_info.entry_count;
    ctc_mirror_rspan_escape_t mac_escape;
    
    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(macescape_entry);

    if (CTC_SAI_MAC_ESCAPE_NUM_MAX == entry_index)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    sal_memset(mac_escape.mac0, 0x0, sizeof(mac_addr_t));
    sal_memset(mac_escape.mac1, 0x0, sizeof(mac_addr_t));
    sal_memset(mac_escape.mac_mask0, 0xFF, sizeof(mac_addr_t));
    sal_memset(mac_escape.mac_mask1, 0xFF, sizeof(mac_addr_t));

    if (ctc_sai_macescape_info.macescape[0].is_valid)
    {
        sal_memcpy(mac_escape.mac0, ctc_sai_macescape_info.macescape[0].macescape_key.mac, sizeof(mac_addr_t));
        sal_memcpy(mac_escape.mac_mask0, ctc_sai_macescape_info.macescape[0].macescape_key.mask, sizeof(mac_addr_t));
    
        sal_memcpy(mac_escape.mac1, macescape_entry->mac, sizeof(mac_addr_t));
        sal_memcpy(mac_escape.mac_mask1, macescape_entry->mask, sizeof(mac_addr_t));
    }
    else
    {
        sal_memcpy(mac_escape.mac0, macescape_entry->mac, sizeof(mac_addr_t));
        sal_memcpy(mac_escape.mac_mask0, macescape_entry->mask, sizeof(mac_addr_t));
        
        sal_memcpy(mac_escape.mac1, macescape_entry->mac, sizeof(mac_addr_t));
        sal_memcpy(mac_escape.mac_mask1, macescape_entry->mask, sizeof(mac_addr_t));
    }

    CTC_SAI_ERROR_GOTO(ctc_mirror_set_escape_mac(&mac_escape), ret, out);

    if (ctc_sai_macescape_info.entry_count == 0)
    {
        CTC_SAI_ERROR_GOTO(ctc_mirror_set_escape_en(TRUE), ret, out);
    }

    ctc_sai_macescape_info.macescape[entry_index].is_valid = TRUE;
    sal_memcpy(ctc_sai_macescape_info.macescape[entry_index].macescape_key.mac, macescape_entry->mac, sizeof(mac_addr_t));
    sal_memcpy(ctc_sai_macescape_info.macescape[entry_index].macescape_key.mask, macescape_entry->mask, sizeof(mac_addr_t));
    ctc_sai_macescape_info.entry_count++;
    
 out:
    return ret;
    
}

sai_status_t
ctc_sai_remove_mac_eacape_entry(
    _In_ const sai_mac_escape_entry_t *macescape_entry)
{
    int32  ret = SAI_STATUS_SUCCESS;
    uint32 entry_index = 0;
    ctc_mirror_rspan_escape_t mac_escape;
    
    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(macescape_entry);

    if (0 == ctc_sai_macescape_info.entry_count)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }
    
    sal_memset(mac_escape.mac0, 0x0, sizeof(mac_addr_t));
    sal_memset(mac_escape.mac1, 0x0, sizeof(mac_addr_t));
    sal_memset(mac_escape.mac_mask0, 0xFF, sizeof(mac_addr_t));
    sal_memset(mac_escape.mac_mask1, 0xFF, sizeof(mac_addr_t));

    if (ctc_sai_macescape_info.macescape[0].is_valid)
    {
        if (ctc_sai_macescape_info.macescape[1].is_valid)
        {
            if ((0 == sal_memcmp(ctc_sai_macescape_info.macescape[0].macescape_key.mac, macescape_entry->mac, sizeof(mac_addr_t))) &&
                (0 == sal_memcmp(ctc_sai_macescape_info.macescape[0].macescape_key.mask, macescape_entry->mask, sizeof(mac_addr_t))))
            {
                entry_index = 0;
                sal_memcpy(mac_escape.mac0, ctc_sai_macescape_info.macescape[1].macescape_key.mac, sizeof(mac_addr_t));
                sal_memcpy(mac_escape.mac_mask0, ctc_sai_macescape_info.macescape[1].macescape_key.mask, sizeof(mac_addr_t));
            
                sal_memcpy(mac_escape.mac1, ctc_sai_macescape_info.macescape[1].macescape_key.mac, sizeof(mac_addr_t));
                sal_memcpy(mac_escape.mac_mask1, ctc_sai_macescape_info.macescape[1].macescape_key.mask, sizeof(mac_addr_t));
            }
            else if ((0 == sal_memcmp(ctc_sai_macescape_info.macescape[1].macescape_key.mac, macescape_entry->mac, sizeof(mac_addr_t))) &&
                     (0 == sal_memcmp(ctc_sai_macescape_info.macescape[1].macescape_key.mask, macescape_entry->mask, sizeof(mac_addr_t))))
            {
                entry_index = 1;
                sal_memcpy(mac_escape.mac0, ctc_sai_macescape_info.macescape[0].macescape_key.mac, sizeof(mac_addr_t));
                sal_memcpy(mac_escape.mac_mask0, ctc_sai_macescape_info.macescape[0].macescape_key.mask, sizeof(mac_addr_t));
            
                sal_memcpy(mac_escape.mac1, ctc_sai_macescape_info.macescape[0].macescape_key.mac, sizeof(mac_addr_t));
                sal_memcpy(mac_escape.mac_mask1, ctc_sai_macescape_info.macescape[0].macescape_key.mask, sizeof(mac_addr_t));
            }
            else
            {
                return SAI_STATUS_INVALID_PARAMETER;
            }
        }
    }
    
    CTC_SAI_ERROR_GOTO(ctc_mirror_set_escape_mac(&mac_escape), ret, out);

    if (ctc_sai_macescape_info.entry_count == 1)
    {
        CTC_SAI_ERROR_GOTO(ctc_mirror_set_escape_en(FALSE), ret, out);
    }
    ctc_sai_macescape_info.macescape[entry_index].is_valid = FALSE;
    sal_memset(ctc_sai_macescape_info.macescape[entry_index].macescape_key.mac, 0x0, sizeof(sai_mac_t));
    sal_memset(ctc_sai_macescape_info.macescape[entry_index].macescape_key.mask, 0x0, sizeof(sai_mac_t));
    ctc_sai_macescape_info.entry_count--;
    
 out:
    return ret;
    
}

#define ________SAI_MACESCAPE_OUTER_FUNC
static sai_status_t
__init_mode_fn(
    _In_  ctc_sai_api_reg_info_t    *preg,
    _In_  void                      *private_data)
{
    preg->init_status = INITIALIZED;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t
__exit_mode_fn(
    _In_  ctc_sai_api_reg_info_t    *preg,
    _In_  void                      *private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.2 */
static sai_mac_escape_api_t      g_sai_mac_escape_api_func = {
    .create_mac_escape_entry           = ctc_sai_create_mac_escape_entry,
    .delete_mac_escape_entry           = ctc_sai_remove_mac_eacape_entry,  
};

static ctc_sai_api_reg_info_t g_mac_escape_api_reg_info = {
        .id         = SAI_API_MAC_ESCAPE,
        .init_func  = __init_mode_fn,
        .exit_func  = __exit_mode_fn,
        .api_method_table = &g_sai_mac_escape_api_func,
        .private_data     = NULL,
};

#define ________SAI_MACESCAPE_OUTER_FUNC
sai_status_t
ctc_sai_macescape_init()
{
    api_reg_register_fn(&g_mac_escape_api_reg_info);

    return SAI_STATUS_SUCCESS;
}

