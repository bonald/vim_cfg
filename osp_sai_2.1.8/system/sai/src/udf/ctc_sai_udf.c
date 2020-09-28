#include <sal.h>
#include "ctc_api.h"
#include <sai.h>
#include <saitypes.h>
#include <saistatus.h>
#include <ctc_sai_common.h>
#include <ctc_sai_common_attr.h>
#include <ctc_sai_sai.h>
#include <ctc_sai_debug.h>
#include <ctc_sai_udf.h>


#if 0

#define 	CTC_SAI_UDF_RES				64
#define 	CTC_SAI_UDF_BLOCK_SIZE		32

static ctc_sai_udf_info_t  g_sai_udf_info;


static sai_status_t
__udf_create(ctc_sai_udf_info_t **ppudf_info_entry)
{
    stbl_sai_glb_t          *p_sai_glb = stbl_sai_glb_get_sai_glb();
    sai_status_t            ret 		        = SAI_STATUS_SUCCESS;
    ctc_sai_udf_info_t    	*pudf_info_entry 	= NULL;
    uint32_t                index;

    /* 1: check res */
    if (g_sai_udf_info.total_count >= g_sai_udf_info.max_count)
    {
        ret = SAI_STATUS_TABLE_FULL;
        goto out;
    }

    /* 2: alloc index */
    CTC_SAI_ERROR_GOTO_MAPSDKERR2SAI(ctclib_opb_alloc_offset(&p_sai_glb->udf_opb, &index),ret,out);

    /* 3: malloc mem */
    pudf_info_entry = XCALLOC(MEM_FEA_TEMP_TODO, sizeof(ctc_sai_udf_info_t));
    if (NULL == pudf_info_entry)
    {
        ret = SAI_STATUS_NO_MEMORY;
        ctclib_opb_free_offset(&p_sai_glb->udf_opb, index);
        goto out;
    }
    
    sal_memset(pudf_info_entry, 0, sizeof(ctc_sai_udf_info_t));

    /* 4: set OID */
    *ppudf_info_entry = pudf_info_entry;
    pudf_info_entry->oid = CTC_SAI_OBJECT_TYPE_SET(SAI_OBJECT_TYPE_UDF, index);

    /* 5: save to vector */
    ctc_vector_add(g_sai_udf_info.pudf_vector, index, pudf_info_entry);

    g_sai_udf_info.total_count++;

    CTC_SAI_DEBUG("__udf_create:udf_oid = 0x%llx\n",pudf_info_entry->oid);

out:
    return ret;
}

void
__udf_remove(
    _In_  ctc_sai_udf_info_t* pudf_info_entry)
{
    stbl_sai_glb_t          *p_sai_glb = stbl_sai_glb_get_sai_glb();
    
    if(NULL == pudf_info_entry)
    {
        return ;
    }

    CTC_SAI_DEBUG("__udf_remove:udf_oid = 0x%llx\n",pudf_info_entry->oid);

    /* 1: remove vector */
    ctc_vector_del( g_sai_udf_info.pudf_vector, 
                    CTC_SAI_OBJECT_INDEX_GET(pudf_info_entry->oid));

    /* 2: free index */
    ctclib_opb_free_offset(&p_sai_glb->udf_opb, CTC_SAI_OBJECT_INDEX_GET(pudf_info_entry->oid));

    /* 3: free mem */
    XFREE(MEM_FEA_TEMP_TODO, pudf_info_entry);

    g_sai_udf_info.total_count--;
}

ctc_sai_udf_info_t*
ctc_sai_udf_get_by_oid(sai_object_id_t  udf_oid)
{
    return ctc_vector_get( g_sai_udf_info.pudf_vector, 
                    CTC_SAI_OBJECT_INDEX_GET(udf_oid));
}


static sai_status_t ctc_sai_create_udf(
    _Out_ sai_object_id_t* udf_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
    )
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    sai_attribute_t const *pattr_entry = NULL;
    ctc_sai_udf_info_t         *pudf_info = NULL;
    uint32_t index = 0;

    CTC_SAI_DEBUG_FUNC();
    
    CTC_SAI_PTR_VALID_CHECK(udf_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    ret = __udf_create(&pudf_info);
    if(SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }
    
    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_ATTR_MATCH_ID, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        pudf_info->u.udf.udf_match_id = pattr_entry->value.oid;
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_ATTR_GROUP_ID, attr_list, attr_count, &index);
    if (pattr_entry)
    {
        pudf_info->u.udf.udf_group_id = pattr_entry->value.oid;
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_ATTR_BASE, attr_list, attr_count, &index);
    if (pattr_entry)
    {
        pudf_info->u.udf.udf_base_select = pattr_entry->value.s32;
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_ATTR_OFFSET, attr_list, attr_count, &index);
    if (pattr_entry)
    {
        pudf_info->u.udf.udf_offset = pattr_entry->value.u16;
    }

    pudf_info->info_type = CTC_SAI_UDF_INFO_TYPE_UDF;
    g_sai_udf_info.udf_cnt++;

    *udf_id = pudf_info->oid;
    
    return SAI_STATUS_SUCCESS;
}


static sai_status_t ctc_sai_remove_udf(
    _In_ sai_object_id_t udf_id
    )
{
    ctc_sai_udf_info_t      *pudf_info_entry    = NULL;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_DEBUG("ctc_sai_remove_udf:udf_id = 0x%llx\n",udf_id);

    pudf_info_entry = ctc_sai_udf_get_by_oid(udf_id);
    if(NULL == pudf_info_entry)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    __udf_remove(pudf_info_entry);
    g_sai_udf_info.udf_cnt--;

    return SAI_STATUS_SUCCESS;
}


static sai_status_t ctc_sai_set_udf_attribute(
    _In_ sai_object_id_t udf_id,
    _In_ const sai_attribute_t *attr
    )
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t ctc_sai_get_udf_attribute(
    _In_ sai_object_id_t udf_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    )
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t ctc_sai_create_udf_match(
    _Out_ sai_object_id_t* udf_match_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
    )
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    sai_attribute_t const *pattr_entry = NULL;
    ctc_sai_udf_info_t         *pudf_info = NULL;
    uint32_t index = 0;

    CTC_SAI_DEBUG_FUNC();
    
    CTC_SAI_PTR_VALID_CHECK(udf_match_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    ret = __udf_create(&pudf_info);
    if(SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }
    
    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_MATCH_ATTR_L2_TYPE, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        pudf_info->u.udf_match.udf_match_l2_type = pattr_entry->value.u16;
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_MATCH_ATTR_L3_TYPE, attr_list, attr_count, &index);
    if (pattr_entry)
    {
        pudf_info->u.udf_match.udf_match_l3_type = pattr_entry->value.u8;
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_MATCH_ATTR_GRE_TYPE, attr_list, attr_count, &index);
    if (pattr_entry)
    {
        pudf_info->u.udf_match.udf_match_gre_type = pattr_entry->value.u16;
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_MATCH_ATTR_PRIORITY, attr_list, attr_count, &index);
    if (pattr_entry)
    {
        pudf_info->u.udf_match.udf_match_priority = pattr_entry->value.u8;
    }

    pudf_info->info_type = CTC_SAI_UDF_INFO_TYPE_MATCH;
    g_sai_udf_info.udf_match_cnt++;

    *udf_match_id = pudf_info->oid;
    
    return SAI_STATUS_SUCCESS;
}

static sai_status_t ctc_sai_remove_udf_match(
    _In_ sai_object_id_t udf_match_id
    )
{
    ctc_sai_udf_info_t      *pudf_info_entry    = NULL;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_DEBUG("ctc_sai_remove_udf_match:udf_id = 0x%llx\n",udf_match_id);

    pudf_info_entry = ctc_sai_udf_get_by_oid(udf_match_id);
    if(NULL == pudf_info_entry)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    __udf_remove(pudf_info_entry);
    g_sai_udf_info.udf_match_cnt--;
    
    return SAI_STATUS_SUCCESS;
}

static sai_status_t ctc_sai_set_udf_match_attribute(
    _In_ sai_object_id_t udf_match_id,
    _In_ const sai_attribute_t *attr
    )
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t ctc_sai_get_udf_match_attribute(
    _In_ sai_object_id_t udf_match_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    )
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t ctc_sai_create_udf_group(
    _Out_ sai_object_id_t* udf_group_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
    )
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    sai_attribute_t const *pattr_entry = NULL;
    ctc_sai_udf_info_t *pudf_info = NULL;
    ctc_sai_udf_info_t *search_udf_info_entry = NULL;
    ctc_sai_udf_info_t *search_udf_match_entry = NULL;
    uint32_t index = 0;
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t udf_index = 0xFFFFFFFF;
    ctc_parser_udf_t udf;

    CTC_SAI_DEBUG_FUNC();
    
    CTC_SAI_PTR_VALID_CHECK(udf_group_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);
    sal_memset(&udf, 0, sizeof(ctc_parser_udf_t));

    ret = __udf_create(&pudf_info);
    if(SAI_STATUS_SUCCESS != ret)
    {
        return ret;
    }
    
    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_GROUP_ATTR_UDF_LIST, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        for (i = 0; i < pattr_entry->value.objlist.count; i++)
        {
            pudf_info->u.udf_group.udf_oid[i] = pattr_entry->value.objlist.list[i];
        }
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_GROUP_ATTR_TYPE, attr_list, attr_count, &index);
    if (pattr_entry)
    {
        pudf_info->u.udf_group.udf_group_type = pattr_entry->value.s32;
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_ATTR_LENGTH, attr_list, attr_count, &index);
    if (pattr_entry)
    {
        pudf_info->u.udf_group.udf_length = pattr_entry->value.u16;
        udf.udf_num = pudf_info->u.udf_group.udf_length & 0xFF;  //  4 
    }
   
    for (i = 0; i < CTC_SAI_UDF_GROUP_UDF_OID_MAX && 0 != pudf_info->u.udf_group.udf_oid[i]; i++)
    {
        search_udf_info_entry = ctc_sai_udf_get_by_oid(pudf_info->u.udf_group.udf_oid[i]);
        if (search_udf_info_entry && search_udf_info_entry->info_type == CTC_SAI_UDF_INFO_TYPE_UDF)
        {
            if (search_udf_info_entry->u.udf.udf_base_select == SAI_UDF_BASE_L3)
            {
                udf.type = CTC_PARSER_UDF_TYPE_L3_UDF;
            }
            else if (search_udf_info_entry->u.udf.udf_base_select == SAI_UDF_BASE_L4)
            {
                udf.type = CTC_PARSER_UDF_TYPE_L4_UDF;
            }
            else
            {
                return SAI_STATUS_NOT_SUPPORTED;
            }

            if(search_udf_info_entry->u.udf.udf_offset & 0x80)
            {
                j = search_udf_info_entry->u.udf.udf_offset & 0x7F;
                udf.udf_offset[0] = j + 5;
                udf.udf_offset[1] = j + 4;
                udf.udf_offset[2] = j + 1;
                udf.udf_offset[3] = j;
            }
            else
            {
                for (j = 0; j < udf.udf_num && j < CTC_PARSER_UDF_FIELD_NUM; j++)
                {
                    udf.udf_offset[CTC_PARSER_UDF_FIELD_NUM - j - 1] = (search_udf_info_entry->u.udf.udf_offset & 0xFF) + j;
                }
            }
            
            search_udf_match_entry = ctc_sai_udf_get_by_oid(search_udf_info_entry->u.udf.udf_match_id);
            if (search_udf_match_entry && search_udf_match_entry->info_type == CTC_SAI_UDF_INFO_TYPE_MATCH)
            {
                if (CTC_PARSER_UDF_TYPE_L3_UDF == udf.type)
                {
                    udf.ether_type = search_udf_match_entry->u.udf_match.udf_match_l2_type;
                }
                if (CTC_PARSER_UDF_TYPE_L4_UDF == udf.type)
                {
                    udf.ip_version = CTC_IP_VER_4;
                    udf.l3_header_protocol = search_udf_match_entry->u.udf_match.udf_match_l3_type;
                }

                CTC_SAI_DEBUG("ctc_sai_create_udf_group:udf_l3_header_protocol = %d\n",udf.l3_header_protocol);
		  		udf.no_sort = 1;
                ret = ctc_parser_set_udf(udf_index, &udf);
                if (ret)
                {
                    return ret;
                }
            }
        }
    }

    pudf_info->info_type = CTC_SAI_UDF_INFO_TYPE_GROUP;
    g_sai_udf_info.udf_group_cnt++;

    *udf_group_id = pudf_info->oid;
    
    return SAI_STATUS_SUCCESS;
}

static sai_status_t ctc_sai_remove_udf_group(
    _In_ sai_object_id_t udf_group_id
    )
{
    sai_status_t ret = SAI_STATUS_SUCCESS;
    ctc_sai_udf_info_t *pudf_info = NULL;
    uint32_t i = 0;
    uint32_t udf_index = 0xFFFFFFFF;
    ctc_parser_udf_t udf;
    ctc_sai_udf_info_t *search_udf_info_entry = NULL;
    ctc_sai_udf_info_t *search_udf_match_entry = NULL;

    CTC_SAI_DEBUG_FUNC();

    CTC_SAI_DEBUG("ctc_sai_remove_udf_group:udf_id = 0x%llx\n",udf_group_id);
    sal_memset(&udf, 0, sizeof(ctc_parser_udf_t));
    
    pudf_info = ctc_sai_udf_get_by_oid(udf_group_id);
    if(NULL == pudf_info)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    for (i = 0; i < CTC_SAI_UDF_GROUP_UDF_OID_MAX && 0 != pudf_info->u.udf_group.udf_oid[i]; i++)
    {
        search_udf_info_entry = ctc_sai_udf_get_by_oid(pudf_info->u.udf_group.udf_oid[i]);
        if (search_udf_info_entry && search_udf_info_entry->info_type == CTC_SAI_UDF_INFO_TYPE_UDF)
        {
            if (search_udf_info_entry->u.udf.udf_base_select == SAI_UDF_BASE_L3)
            {
                udf.type = CTC_PARSER_UDF_TYPE_L3_UDF;
            }
            else if (search_udf_info_entry->u.udf.udf_base_select == SAI_UDF_BASE_L4)
            {
                udf.type = CTC_PARSER_UDF_TYPE_L4_UDF;
            }
            else
            {
                return SAI_STATUS_NOT_SUPPORTED;
            }

            search_udf_match_entry = ctc_sai_udf_get_by_oid(search_udf_info_entry->u.udf.udf_match_id);
            if (search_udf_match_entry && search_udf_match_entry->info_type == CTC_SAI_UDF_INFO_TYPE_MATCH)
            {
                if (CTC_PARSER_UDF_TYPE_L3_UDF == udf.type)
                {
                    udf.ether_type = search_udf_match_entry->u.udf_match.udf_match_l2_type;
                }
                if (CTC_PARSER_UDF_TYPE_L4_UDF == udf.type)
                {
                    udf.ip_version = CTC_IP_VER_4;
                    udf.l3_header_protocol = search_udf_match_entry->u.udf_match.udf_match_l3_type;
                }
                ret = ctc_parser_set_udf(udf_index, &udf);
                if (ret)
                {
                    return ret;
                }
            }
        }
    }

    __udf_remove(pudf_info);
    g_sai_udf_info.udf_group_cnt--;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t ctc_sai_set_udf_group_attribute(
    _In_ sai_object_id_t udf_group_id,
    _In_ const sai_attribute_t *attr
    )
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t ctc_sai_get_udf_group_attribute(
    _In_ sai_object_id_t udf_group_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    )
{
    return SAI_STATUS_SUCCESS;
}
#endif


#define ____SAI_API____

/* duet2 & tsingma udf byte length is 4 bytes, goldengate udf byte length is 1 byte */
#ifdef GOLDENGATE
#define CTC_SAI_UDF_GROUP_LENGTH  1
#else
#define CTC_SAI_UDF_GROUP_LENGTH  4
#endif

/* duet2 & tsingma have 16 udf entry, goldengate have 4 udf entry */
#ifdef GOLDENGATE
#define CTC_SAI_UDF_GROUP_MAX_NUM  4
#else
#define CTC_SAI_UDF_GROUP_MAX_NUM  16
#endif


#define ____SAI_API_UDF_GROUP____

static sai_status_t ctc_sai_create_udf_group(
    _Out_ sai_object_id_t* udf_group_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
    )
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    sai_attribute_t const *pattr_entry = NULL;
    stbl_acl_udf_entry_t   sdb_udf;
    stbl_acl_udf_entry_t  *p_sdb_udf   = NULL;
    uint32_t index = 0;

    CTC_SAI_DEBUG_FUNC();
    
    CTC_SAI_PTR_VALID_CHECK(udf_group_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    sal_memset(&sdb_udf, 0, sizeof(sdb_udf));

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_GROUP_ATTR_UDF_ID, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        sdb_udf.key.id = pattr_entry->value.u32;
        p_sdb_udf = stbl_acl_udf_entry_get_acl_udf_entry(&sdb_udf.key);
        if (NULL != p_sdb_udf)
        {
            ret = SAI_STATUS_ITEM_ALREADY_EXISTS;
            goto out;
        }
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_GROUP_ATTR_TYPE, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        sdb_udf.group_type = pattr_entry->value.s32;
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_GROUP_ATTR_LENGTH, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        sdb_udf.group_length = pattr_entry->value.u16;
         /* dt2 group length must be 4 byte, gg group length must be 1 byte */
        if (CTC_SAI_UDF_GROUP_LENGTH != pattr_entry->value.u16)
        {
            ret = SAI_STATUS_INVALID_PARAMETER;
            goto out;
        }
    }

    CTC_SAI_ERROR_GOTO(stbl_acl_udf_entry_add_acl_udf_entry(&sdb_udf), ret, out);
    p_sdb_udf = stbl_acl_udf_entry_get_acl_udf_entry(&sdb_udf.key);
    if (NULL == p_sdb_udf)
    {
        ret = SAI_STATUS_NO_MEMORY;
        goto out;
    }

    *udf_group_id = sdb_udf.key.id;

out:
    return ret;
}

static sai_status_t ctc_sai_remove_udf_group(
    _In_ sai_object_id_t udf_group_id
    )
{
    sai_status_t     ret = SAI_STATUS_SUCCESS;
    stbl_acl_udf_entry_t   sdb_udf;
    stbl_acl_udf_entry_t  *p_sdb_udf   = NULL;
    
#ifdef GOLDENGATE
    uint32_t   udf_index = 0xFFFFFFFF;
    ctc_parser_udf_t udf;
#endif

#ifdef USW
    ctc_acl_classify_udf_t udf_entry;
#endif

    CTC_SAI_DEBUG_FUNC();
    
    sal_memset(&sdb_udf, 0, sizeof(sdb_udf));
    sdb_udf.key.id = udf_group_id;
    p_sdb_udf = stbl_acl_udf_entry_get_acl_udf_entry(&sdb_udf.key);
    if (NULL == p_sdb_udf)
    {
        ret = SAI_STATUS_ITEM_NOT_FOUND;
        return ret;
    }

#ifdef GOLDENGATE
    /* fix bug 51084 not call sdk api when udf not installed on GG */
    if (0 == p_sdb_udf->ethtype[0] && 0 == p_sdb_udf->ipproto[0])
    {
        goto del;
    }

    sal_memset(&udf, 0, sizeof(udf));
    if (p_sdb_udf->offset_type == SAI_UDF_BASE_L3)
    {
        udf.type = CTC_PARSER_UDF_TYPE_L3_UDF;
    }
    else if (p_sdb_udf->offset_type == SAI_UDF_BASE_L4)
    {
        udf.type = CTC_PARSER_UDF_TYPE_L4_UDF;
    }
    else
    {
        return SAI_STATUS_NOT_SUPPORTED;
    }

    if (CTC_PARSER_UDF_TYPE_L3_UDF == udf.type)
    {
        udf.ether_type = p_sdb_udf->ethtype[0];
    }
    if (CTC_PARSER_UDF_TYPE_L4_UDF == udf.type)
    {
        udf.ip_version = CTC_IP_VER_4;
        udf.l3_header_protocol = p_sdb_udf->ipproto[0];
    }

    ret = ctc_parser_set_udf(udf_index, &udf);
#endif

#ifdef USW
    sal_memset(&udf_entry,0,sizeof(udf_entry));
#ifdef DUET2
    udf_entry.udf_id = udf_group_id;
#else
    /* BUG53886 optimize udf_id priority behaviour as DUET2 while spec changed */
    udf_entry.udf_id = CTC_SAI_UDF_GROUP_MAX_NUM-1 - udf_group_id;
#endif
    ret = ctc_acl_remove_udf_entry(&udf_entry);
#endif

#ifdef GOLDENGATE
del:
#endif
    CTC_SAI_ERROR_GOTO(stbl_acl_udf_entry_del_acl_udf_entry(&sdb_udf.key), ret, out);

out:

    return ret;
}

static sai_status_t ctc_sai_set_udf_group_attribute(
    _In_ sai_object_id_t udf_group_id,
    _In_ const sai_attribute_t *attr
    )
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t ctc_sai_get_udf_group_attribute(
    _In_ sai_object_id_t udf_group_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    )
{
    return SAI_STATUS_SUCCESS;
}


#define ____SAI_API_UDF_MATCH____

static sai_status_t ctc_sai_create_udf_match(
    _Out_ sai_object_id_t* udf_match_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
    )
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    sai_attribute_t const *pattr_entry = NULL;
    stbl_acl_udf_entry_t   sdb_udf;
    stbl_acl_udf_entry_t  *p_sdb_udf   = NULL;
    uint32_t index = 0;

    CTC_SAI_DEBUG_FUNC();
    
    CTC_SAI_PTR_VALID_CHECK(udf_match_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    sal_memset(&sdb_udf, 0, sizeof(sdb_udf));

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_MATCH_ATTR_PRIORITY, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        sdb_udf.key.id = pattr_entry->value.u32;
        p_sdb_udf = stbl_acl_udf_entry_get_acl_udf_entry(&sdb_udf.key);
        if (NULL == p_sdb_udf)
        {
            ret = SAI_STATUS_ITEM_NOT_FOUND;
            goto out;
        }
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_MATCH_ATTR_L2_TYPE, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        p_sdb_udf->ethtype[0] = pattr_entry->value.aclfield.data.u16;
        p_sdb_udf->ethtype[1] = pattr_entry->value.aclfield.mask.u16;
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_MATCH_ATTR_L3_TYPE, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        p_sdb_udf->ipproto[0] = pattr_entry->value.aclfield.data.u8;
        p_sdb_udf->ipproto[1] = pattr_entry->value.aclfield.mask.u8;
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_MATCH_ATTR_VLANNUM, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        p_sdb_udf->vlannum[0] = pattr_entry->value.aclfield.data.u8;
        p_sdb_udf->vlannum[1] = pattr_entry->value.aclfield.mask.u8;
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_MATCH_ATTR_L4_TYPE, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        p_sdb_udf->l4_type[0] = pattr_entry->value.aclfield.data.u8;
        p_sdb_udf->l4_type[1] = pattr_entry->value.aclfield.mask.u8;
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_MATCH_ATTR_SRCPORT, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        p_sdb_udf->srcport[0] = pattr_entry->value.aclfield.data.u16;
        p_sdb_udf->srcport[1] = pattr_entry->value.aclfield.mask.u16;
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_MATCH_ATTR_DSTPORT, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        p_sdb_udf->dstport[0] = pattr_entry->value.aclfield.data.u16;
        p_sdb_udf->dstport[1] = pattr_entry->value.aclfield.mask.u16;
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_MATCH_ATTR_MPLSNUM, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        p_sdb_udf->mplsnum[0] = pattr_entry->value.aclfield.data.u8;
        p_sdb_udf->mplsnum[1] = pattr_entry->value.aclfield.mask.u8;
    }

    *udf_match_id = p_sdb_udf->key.id;

out:
    return ret;
}

static sai_status_t ctc_sai_remove_udf_match(
    _In_ sai_object_id_t udf_match_id
    )
{
    sai_status_t   ret = SAI_STATUS_SUCCESS;
    CTC_SAI_DEBUG_FUNC();
    
    return ret;
}

static sai_status_t ctc_sai_set_udf_match_attribute(
    _In_ sai_object_id_t udf_match_id,
    _In_ const sai_attribute_t *attr
    )
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    stbl_acl_udf_entry_t   sdb_udf;
    stbl_acl_udf_entry_t  *p_sdb_udf   = NULL;
#ifdef USW
    ctc_field_key_t        udf_key_field;
    sal_memset(&udf_key_field, 0, sizeof(udf_key_field));
    uint32  udf_id = 0;
#endif

#ifdef GOLDENGATE
    uint32           udf_index  = 0xFFFFFFFF;
    ctc_parser_udf_t udf;
    sal_memset(&udf, 0, sizeof(udf));
#endif

    CTC_SAI_DEBUG_FUNC();
    CTC_SAI_PTR_VALID_CHECK(attr);

    sal_memset(&sdb_udf, 0, sizeof(sdb_udf));

    sdb_udf.key.id = udf_match_id;
    p_sdb_udf = stbl_acl_udf_entry_get_acl_udf_entry(&sdb_udf.key);
    if (NULL == p_sdb_udf)
    {
        ret = SAI_STATUS_ITEM_NOT_FOUND;
        goto out;
    }

#ifdef USW
    if (!p_sdb_udf->installed)
    {
        ret = SAI_STATUS_FAILURE;
        goto out;
    }
#endif

    switch (attr->id)
    {
    case SAI_UDF_MATCH_ATTR_L2_TYPE:
#ifdef GOLDENGATE
        udf.type = CTC_PARSER_UDF_TYPE_L3_UDF;
        udf.ether_type = p_sdb_udf->ethtype[0];
        ctc_parser_set_udf(udf_index, &udf);
#endif        
        p_sdb_udf->ethtype[0] = attr->value.aclfield.data.u16;
        p_sdb_udf->ethtype[1] = attr->value.aclfield.mask.u16;
#ifdef USW
        udf_key_field.type = CTC_FIELD_KEY_ETHER_TYPE;
        udf_key_field.data = p_sdb_udf->ethtype[0];
        udf_key_field.mask = p_sdb_udf->ethtype[1];
#endif        
        break;
    case SAI_UDF_MATCH_ATTR_L3_TYPE:
#ifdef GOLDENGATE
        udf.type = CTC_PARSER_UDF_TYPE_L4_UDF;
        udf.ip_version = CTC_IP_VER_4;
        udf.l3_header_protocol = p_sdb_udf->ipproto[0];
        ctc_parser_set_udf(udf_index, &udf);
#endif
        p_sdb_udf->ipproto[0] = attr->value.aclfield.data.u8;
        p_sdb_udf->ipproto[1] = attr->value.aclfield.mask.u8;
#ifdef USW        
        udf_key_field.type = CTC_FIELD_KEY_IP_PROTOCOL;
        udf_key_field.data = p_sdb_udf->ipproto[0];
        udf_key_field.mask = p_sdb_udf->ipproto[1];
#endif        
        break;
    case SAI_UDF_MATCH_ATTR_VLANNUM:
        p_sdb_udf->vlannum[0] = attr->value.aclfield.data.u8;
        p_sdb_udf->vlannum[1] = attr->value.aclfield.mask.u8;
#ifdef USW
        udf_key_field.type = CTC_FIELD_KEY_VLAN_NUM;
        udf_key_field.data = p_sdb_udf->vlannum[0];
        udf_key_field.mask = p_sdb_udf->vlannum[1];
#endif        
        break;
    case SAI_UDF_MATCH_ATTR_L4_TYPE:
        p_sdb_udf->l4_type[0] = attr->value.aclfield.data.u8;
        p_sdb_udf->l4_type[1] = attr->value.aclfield.mask.u8;
#ifdef USW        
        udf_key_field.type = CTC_FIELD_KEY_L4_TYPE;
        udf_key_field.data = p_sdb_udf->l4_type[0];
        udf_key_field.mask = p_sdb_udf->l4_type[1];
#endif        
        break;
    case SAI_UDF_MATCH_ATTR_SRCPORT:
        p_sdb_udf->srcport[0] = attr->value.aclfield.data.u16;
        p_sdb_udf->srcport[1] = attr->value.aclfield.mask.u16;
#ifdef USW        
        udf_key_field.type = CTC_FIELD_KEY_L4_SRC_PORT;
        udf_key_field.data = p_sdb_udf->srcport[0];
        udf_key_field.mask = p_sdb_udf->srcport[1];
#endif        
        break;
    case SAI_UDF_MATCH_ATTR_DSTPORT:
        p_sdb_udf->dstport[0] = attr->value.aclfield.data.u16;
        p_sdb_udf->dstport[1] = attr->value.aclfield.mask.u16;
#ifdef USW        
        udf_key_field.type = CTC_FIELD_KEY_L4_DST_PORT;
        udf_key_field.data = p_sdb_udf->dstport[0];
        udf_key_field.mask = p_sdb_udf->dstport[1];
#endif        
        break;
    case SAI_UDF_MATCH_ATTR_MPLSNUM:
        p_sdb_udf->mplsnum[0] = attr->value.aclfield.data.u8;
        p_sdb_udf->mplsnum[1] = attr->value.aclfield.mask.u8;
#ifdef USW
        udf_key_field.type = CTC_FIELD_KEY_LABEL_NUM;
        udf_key_field.data = p_sdb_udf->mplsnum[0];
        udf_key_field.mask = p_sdb_udf->mplsnum[1];
#endif        
        break;
    default:
        ret = SAI_STATUS_NOT_SUPPORTED;
        goto out;
        break;
    }

#ifdef USW
#ifdef DUET2
    udf_id = p_sdb_udf->key.id;
#else
    /* BUG53886 optimize udf_id priority behaviour as DUET2 while spec changed */
    udf_id = CTC_SAI_UDF_GROUP_MAX_NUM-1 - p_sdb_udf->key.id;
#endif
    if (attr->value.aclfield.enable)
    {

        CTC_SAI_ERROR_GOTO(ctc_acl_add_udf_entry_key_field(udf_id, &udf_key_field), ret, out);
    }
    else
    {
        CTC_SAI_ERROR_GOTO(ctc_acl_remove_udf_entry_key_field(udf_id, &udf_key_field), ret, out);
    }
#endif

out:
    return ret;
}

static sai_status_t ctc_sai_get_udf_match_attribute(
    _In_ sai_object_id_t udf_match_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    )
{
    return SAI_STATUS_SUCCESS;
}


#define ____SAI_API_UDF____


#ifdef GOLDENGATE
static sai_status_t _ctc_sai_udf_create_udf_entry_gg(
    stbl_acl_udf_entry_t  *p_sdb_udf
    )
{
    sai_status_t  ret = SAI_STATUS_SUCCESS;
    uint32        offset_tmp = 0;
    uint32        udf_index  = 0xFFFFFFFF;
    ctc_parser_udf_t udf;

    CTC_SAI_DEBUG_FUNC();
    sal_memset(&udf, 0, sizeof(udf));

    if (p_sdb_udf->offset_type == SAI_UDF_BASE_L3)
    {
        udf.type = CTC_PARSER_UDF_TYPE_L3_UDF;
    }
    else if (p_sdb_udf->offset_type == SAI_UDF_BASE_L4)
    {
        udf.type = CTC_PARSER_UDF_TYPE_L4_UDF;
    }
    else
    {
        return SAI_STATUS_NOT_SUPPORTED;
    }

    if (CTC_PARSER_UDF_TYPE_L3_UDF == udf.type)
    {
        udf.ether_type = p_sdb_udf->ethtype[0];
    }
    if (CTC_PARSER_UDF_TYPE_L4_UDF == udf.type)
    {
        udf.ip_version = CTC_IP_VER_4;
        udf.l3_header_protocol = p_sdb_udf->ipproto[0];
    }

    /* remove udf configuration to support update udf offset */
    ctc_parser_set_udf(udf_index, &udf);

    if(p_sdb_udf->offset[0] & 0x80)
    {
        offset_tmp = p_sdb_udf->offset[0] & 0x7F;
        udf.udf_offset[0] = offset_tmp + 5;
        udf.udf_offset[1] = offset_tmp + 4;
        udf.udf_offset[2] = offset_tmp + 1;
        udf.udf_offset[3] = offset_tmp;
    }
    else
    {
        for (offset_tmp = 0; offset_tmp < CTC_PARSER_UDF_FIELD_NUM; offset_tmp++)
        {
            udf.udf_offset[offset_tmp] = p_sdb_udf->offset[offset_tmp];
        }
    }

    udf.udf_num = CTC_PARSER_UDF_FIELD_NUM;
    udf.no_sort = 1;

    ret = ctc_parser_set_udf(udf_index, &udf);

    return ret;
}

#endif

#ifdef USW
static sai_status_t _ctc_sai_udf_create_udf_entry(
    stbl_acl_udf_entry_t  *p_sdb_udf
    )
{
    sai_status_t     ret = SAI_STATUS_SUCCESS;
    ctc_acl_classify_udf_t udf_entry;
    ctc_field_key_t        udf_key_field;
    uint8    loop_i = 0;
    uint32   udf_id = 0;

#ifdef DUET2
    udf_id = p_sdb_udf->key.id;
#else
    udf_id = CTC_SAI_UDF_GROUP_MAX_NUM-1 - p_sdb_udf->key.id;
#endif
    sal_memset(&udf_entry, 0, sizeof(udf_entry));
    udf_entry.udf_id   = udf_id; //p_sdb_udf->key.id;
    udf_entry.priority = udf_id;
    udf_entry.offset_type = p_sdb_udf->offset_type;
    
#ifdef TSINGMA
    /* SDK tsingma sys_usw_acl_map_ctc_to_sys_offset_type unCompatible with D2 */
    udf_entry.offset_type++;
#endif
    for (loop_i = 0; loop_i < GLB_ACL_UDF_FIELD_NUM_MAX; loop_i++) /* get udf in group */
    {
        udf_entry.offset[udf_entry.offset_num++] = p_sdb_udf->offset[loop_i];
    }

    if (p_sdb_udf->installed)
    {
        ret = ctc_acl_remove_udf_entry(&udf_entry);
    }

    CTC_SAI_ERROR_GOTO(ctc_acl_add_udf_entry(&udf_entry), ret, out);
    p_sdb_udf->installed = 1;

    if (p_sdb_udf->ethtype[1])
    {
        sal_memset(&udf_key_field, 0, sizeof(udf_key_field));
        udf_key_field.type = CTC_FIELD_KEY_ETHER_TYPE;
        udf_key_field.data = p_sdb_udf->ethtype[0];
        udf_key_field.mask = p_sdb_udf->ethtype[1];
        CTC_SAI_ERROR_GOTO(ctc_acl_add_udf_entry_key_field(udf_id, &udf_key_field), ret, out);
    }
    if (p_sdb_udf->ipproto[1])
    {
        sal_memset(&udf_key_field, 0, sizeof(udf_key_field));
        udf_key_field.type = CTC_FIELD_KEY_IP_PROTOCOL;
        udf_key_field.data = p_sdb_udf->ipproto[0];
        udf_key_field.mask = p_sdb_udf->ipproto[1];
        CTC_SAI_ERROR_GOTO(ctc_acl_add_udf_entry_key_field(udf_id, &udf_key_field), ret, out);
    }
    if (p_sdb_udf->vlannum[1])
    {
        sal_memset(&udf_key_field, 0, sizeof(udf_key_field));
        udf_key_field.type = CTC_FIELD_KEY_VLAN_NUM;
        udf_key_field.data = p_sdb_udf->vlannum[0];
        udf_key_field.mask = p_sdb_udf->vlannum[1];
        CTC_SAI_ERROR_GOTO(ctc_acl_add_udf_entry_key_field(udf_id, &udf_key_field), ret, out);
    }
    if (p_sdb_udf->l4_type[1])
    {
        sal_memset(&udf_key_field, 0, sizeof(udf_key_field));
        udf_key_field.type = CTC_FIELD_KEY_L4_TYPE;
        udf_key_field.data = p_sdb_udf->l4_type[0];
        udf_key_field.mask = p_sdb_udf->l4_type[1];
        CTC_SAI_ERROR_GOTO(ctc_acl_add_udf_entry_key_field(udf_id, &udf_key_field), ret, out);
    }
    if (p_sdb_udf->srcport[1])
    {
        sal_memset(&udf_key_field, 0, sizeof(udf_key_field));
        udf_key_field.type = CTC_FIELD_KEY_L4_SRC_PORT;
        udf_key_field.data = p_sdb_udf->srcport[0];
        udf_key_field.mask = p_sdb_udf->srcport[1];
        CTC_SAI_ERROR_GOTO(ctc_acl_add_udf_entry_key_field(udf_id, &udf_key_field), ret, out);
    }
    if (p_sdb_udf->dstport[1])
    {
        sal_memset(&udf_key_field, 0, sizeof(udf_key_field));
        udf_key_field.type = CTC_FIELD_KEY_L4_DST_PORT;
        udf_key_field.data = p_sdb_udf->dstport[0];
        udf_key_field.mask = p_sdb_udf->dstport[1];
        CTC_SAI_ERROR_GOTO(ctc_acl_add_udf_entry_key_field(udf_id, &udf_key_field), ret, out);
    }

    sal_memset(&udf_key_field, 0, sizeof(udf_key_field));
    udf_key_field.type = CTC_FIELD_KEY_UDF_ENTRY_VALID;
    CTC_SAI_ERROR_GOTO(ctc_acl_add_udf_entry_key_field(udf_id, &udf_key_field), ret, out);

out:
    return ret;
}
#endif

static sai_status_t ctc_sai_create_udf(
    _Out_ sai_object_id_t* udf_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list
    )
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    sai_attribute_t const *pattr_entry = NULL;
    stbl_acl_udf_entry_t   sdb_udf;
    stbl_acl_udf_entry_t  *p_sdb_udf   = NULL;
    uint32_t index = 0;

    CTC_SAI_DEBUG_FUNC();
    
    CTC_SAI_PTR_VALID_CHECK(udf_id);
    CTC_SAI_PTR_VALID_CHECK(attr_list);

    sal_memset(&sdb_udf, 0, sizeof(sdb_udf));

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_ATTR_GROUP_ID, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        sdb_udf.key.id = pattr_entry->value.u32;
        p_sdb_udf = stbl_acl_udf_entry_get_acl_udf_entry(&sdb_udf.key);
        if (NULL == p_sdb_udf)
        {
            ret = SAI_STATUS_ITEM_NOT_FOUND;
            goto out;
        }
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_ATTR_BASE, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        p_sdb_udf->offset_type = pattr_entry->value.u32;
    }

    pattr_entry = ctc_sai_attr_get_attr_by_id(SAI_UDF_ATTR_OFFSET, attr_list, attr_count, &index);
    if (NULL != pattr_entry)
    {
        p_sdb_udf->offset[0] = pattr_entry->value.u32 & 0xff;
        p_sdb_udf->offset[1] = (pattr_entry->value.u32 >> 8) & 0xff;
        p_sdb_udf->offset[2] = (pattr_entry->value.u32 >> 16) & 0xff;
        p_sdb_udf->offset[3] = (pattr_entry->value.u32 >> 24) & 0xff;
    }

#ifdef GOLDENGATE
    CTC_SAI_ERROR_GOTO(_ctc_sai_udf_create_udf_entry_gg(p_sdb_udf), ret, out);
#endif

#ifdef USW
    CTC_SAI_ERROR_GOTO(_ctc_sai_udf_create_udf_entry(p_sdb_udf), ret, out);
#endif
    
    *udf_id = sdb_udf.key.id;

out:
    return ret;
}


static sai_status_t ctc_sai_remove_udf(
    _In_ sai_object_id_t udf_id
    )
{
    sai_status_t   ret = SAI_STATUS_SUCCESS;
    CTC_SAI_DEBUG_FUNC();
    
    return ret;
}


static sai_status_t ctc_sai_set_udf_attribute(
    _In_ sai_object_id_t udf_id,
    _In_ const sai_attribute_t *attr
    )
{
    return SAI_STATUS_SUCCESS;
}

static sai_status_t ctc_sai_get_udf_attribute(
    _In_ sai_object_id_t udf_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list
    )
{
    return SAI_STATUS_SUCCESS;
}


#define ____SAI_UDF_INNER_FUNC____

sai_status_t
ctc_sai_udf_db_init()
{
    return SAI_STATUS_SUCCESS;
}


sai_status_t __udf_init_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    sai_status_t    ret = SAI_STATUS_SUCCESS;
    ret = ctc_sai_udf_db_init();

    if(SAI_STATUS_SUCCESS != ret)
    {
        goto out;
    }

    preg->init_status =  INITIALIZED;

out:
    return ret;
}

sai_status_t __udf_exit_mode_fn(ctc_sai_api_reg_info_t* preg, void* private_data)
{
    preg->init_status = UNINITIALIZED;
    return SAI_STATUS_SUCCESS;
}

/* define sai 0.9.2 */
static sai_udf_api_t      g_sai_udf_api_func = {

    .create_udf                 = ctc_sai_create_udf,
    .remove_udf                 = ctc_sai_remove_udf,
    .set_udf_attribute          = ctc_sai_set_udf_attribute,
    .get_udf_attribute          = ctc_sai_get_udf_attribute,
    .create_udf_match           = ctc_sai_create_udf_match,
    .remove_udf_match           = ctc_sai_remove_udf_match,
    .set_udf_match_attribute    = ctc_sai_set_udf_match_attribute,
    .get_udf_match_attribute    = ctc_sai_get_udf_match_attribute,
    .create_udf_group           = ctc_sai_create_udf_group,
    .remove_udf_group           = ctc_sai_remove_udf_group,
    .set_udf_group_attribute    = ctc_sai_set_udf_group_attribute,
    .get_udf_group_attribute    = ctc_sai_get_udf_group_attribute
};


static ctc_sai_api_reg_info_t g_udf_api_reg_info = {
        .id  = SAI_API_UDF,
        .init_func = __udf_init_mode_fn,
        .exit_func = __udf_exit_mode_fn,
        .api_method_table = &g_sai_udf_api_func,
        .private_data     = NULL,
};

#define ________SAI_UDF_OUTER_FUNC
sai_status_t ctc_sai_udf_init()
{
    api_reg_register_fn(&g_udf_api_reg_info);

    return SAI_STATUS_SUCCESS;
}



