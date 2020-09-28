#include "sal.h"
#include "ctc_cli.h"
#include "drv_api.h"
#include "usw/include/drv_common.h"
#include "usw/include/drv_error.h"
#include "ctc_dkit.h"
#include "ctc_usw_dkit_memory.h"
#include "ctc_usw_dkit.h"
#include "ctc_dkit_api.h"

/* refer to hash_db*/
enum ctc_dkit_memory_hash_key_type_e
{
    CTC_DKIT_MEMORY_HASH_KEY_TPYE_XC_OAM = 0,
    CTC_DKIT_MEMORY_HASH_KEY_TPYE_HOST0 = 1,
    CTC_DKIT_MEMORY_HASH_KEY_TPYE_HOST1 = 2,
    CTC_DKIT_MEMORY_HASH_KEY_TPYE_FLOW = 3,
    CTC_DKIT_MEMORY_HASH_KEY_TPYE_IP_FIX = 4,
    CTC_DKIT_MEMORY_HASH_KEY_TPYE_USER_ID = 5,
    CTC_DKIT_MEMORY_HASH_KEY_TPYE_MAX
};
typedef enum ctc_dkit_memory_hash_key_type_e ctc_dkit_memory_hash_key_type_t;

#define CTC_DKITS_DUT2_TBL_ID_CHECK(tbl_id)  do{\
    if((tbl_id) <= 0 || (tbl_id) >= MaxTblId_t) {return CLI_ERROR;} \
    } while(0)

extern int32
drv_usw_ftm_map_tcam_index(uint8 lchip,
                       uint32 tbl_id,
                       uint32 old_index,
                       uint32* new_index);
extern fields_t*
drv_usw_find_field(uint8 lchip, tbls_id_t tbl_id, fld_id_t field_id);
STATIC int32
_ctc_usw_dkit_memory_is_hash_key(uint8 lchip, uint32 tbl_id, uint32* hash_key_type);
STATIC int32
_ctc_usw_ftm_tcam_io(uint8 lchip, uint32 index, uint32 cmd, void* val)
{
    uint32 tbl_id = 0;
    uint32 new_index = 0;

    tbl_id = DRV_IOC_MEMID(cmd);

    new_index = index;

    drv_usw_ftm_map_tcam_index(lchip, tbl_id, index, &new_index);
    return DRV_IOCTL(lchip, new_index, cmd, val);
}

#define FTM_TCAM_IOCTL(lchip, index, cmd, val) _ctc_usw_ftm_tcam_io(lchip, index, cmd, val)

extern
int32 sram_model_get_sw_address_by_tbl_id_and_index(uint8 lchip_offset, tbls_id_t tbl_id,
                                                    uint32 index, uintptr *start_data_addr,
                                                    uint32 is_dump_cfg);

STATIC char*
_ctc_usw_dkit_memory_tbl_type_to_str(ext_content_type_t type)
{
    switch (type)
    {
        case EXT_INFO_TYPE_NORMAL:
            return "NORMAL";

        case EXT_INFO_TYPE_TCAM:
        case EXT_INFO_TYPE_LPM_TCAM_IP:
        case EXT_INFO_TYPE_LPM_TCAM_NAT:
        case EXT_INFO_TYPE_STATIC_TCAM_KEY:
            return "TCAM";

        default:
            return "DYNAMIC";
    }
}

STATIC char*
_ctc_usw_dkit_memory_tbl_slice_type_to_str(ds_slice_type_t type)
{
    switch (type)
    {
        case SLICE_Share:
            return "SHARE";

        case SLICE_Duplicated:
            return "DUPLICATE";

        case SLICE_Cascade:
            return "CASCADE";

        case SLICE_PreCascade:
            return "PreCASCADE";

        case SLICE_Default:
            return "Default";

        default:
            return "Invalid";
    }
}


STATIC char*
_ctc_usw_dkit_memory_tblreg_dump_list_one(uint8 lchip, uint32 tbl_id, uint32* first_flag, uint8 detail)
{

    tables_info_t* tbl_ptr = NULL;
    ext_content_type_t ext_type = 0;
    char tmp[8];
    char tbl_name[CTC_DKITS_RW_BUF_SIZE];
    uint32 addr = 0;
    int32  ret = CLI_SUCCESS;
    uint32 entry = 0;
    uint32 word = 0;
    uint32 field_num = 0;

    if (!(DRV_TABLE_TYPE_TCAM == drv_usw_get_table_type(lchip, tbl_id)
        || DRV_TABLE_TYPE_LPM_TCAM_IP == drv_usw_get_table_type(lchip, tbl_id)
        || DRV_TABLE_TYPE_STATIC_TCAM_KEY == drv_usw_get_table_type(lchip, tbl_id)))
    {
        ret = drv_get_table_property(lchip, DRV_TABLE_PROP_HW_ADDR, tbl_id, 0, (uint32*)&addr);
    }

    if (ret != CLI_SUCCESS)
    {
        CTC_DKIT_PRINT("Get table addr error!\n");
    }

    tbl_ptr = TABLE_INFO_PTR(lchip, tbl_id);

    entry = TABLE_MAX_INDEX(lchip, tbl_id);
    entry = ((tbl_id == DsFwd_t) || (tbl_id == DsFwdHalf_t))? entry*2: entry;
    word = tbl_ptr->byte;
    field_num = tbl_ptr->field_num;


    if (TABLE_EXT_INFO_PTR(lchip, tbl_id))
    {
        ext_type = TABLE_EXT_INFO_PTR(lchip, tbl_id)->ext_content_type;
    }

    if (*first_flag)
    {
        if(detail)
        {
            CTC_DKIT_PRINT("%-5s %-10s %-6s %-7s %-8s %-15s %-9s %-20s\n",
                        "TblID", "Address",  "Number", "EntrySZ", "Fields", "Type", "Slice", "TblName");
            CTC_DKIT_PRINT("----------------------------------------------------------------------------------\n");

        }
        else
        {

        }
        *first_flag = FALSE;
    }

    sal_memset(tmp, 0, sizeof(tmp));

    if (DRV_TABLE_TYPE_TCAM == drv_usw_get_table_type(lchip, tbl_id))
    {
        if (TABLE_ENTRY_SIZE(lchip, tbl_id) == TCAM_KEY_SIZE(lchip, tbl_id))
        {
            sal_sprintf(tmp, "%d", TCAM_KEY_SIZE(lchip, tbl_id));
        }
        else
        {
            sal_sprintf(tmp, "*%d", TCAM_KEY_SIZE(lchip, tbl_id));
        }
    }

    drv_get_table_property(lchip, DRV_TABLE_PROP_GET_NAME, tbl_id, 0, (char*)tbl_name);
    if(detail)
    {
        CTC_DKIT_PRINT("%-5d 0x%08x %-6d %-7d %-8d %-15s %-9s %-20s\n",
                    tbl_id, addr,  entry,
                    word, field_num, _ctc_usw_dkit_memory_tbl_type_to_str(ext_type),
                    _ctc_usw_dkit_memory_tbl_slice_type_to_str(tbl_ptr->slicetype), tbl_name);
    }
    else
    {
        CTC_DKIT_PRINT(" %s\n",tbl_name);
    }

    return CLI_SUCCESS;
}



STATIC int32
_ctc_usw_dkit_memory_tblreg_dump_chip_read_tbl_db_by_id(uint8 lchip, char* tbl_name, uint32 tbl_id, uint32 detail)
{

    tables_info_t* tbl_ptr = NULL;
    fields_t* fld_ptr = NULL;
    uint32 fld_idx = 0;
    uint32 first_flag = TRUE;
    uint8 seg_id = 0;
    char* ptr_field_name = NULL;

    tbl_ptr = TABLE_INFO_PTR(lchip, tbl_id);
    _ctc_usw_dkit_memory_tblreg_dump_list_one(lchip, tbl_id, &first_flag, 1);

    if (detail)
    {
        /* descrption */
        CTC_DKIT_PRINT("==================================================================================\n");
        CTC_DKIT_PRINT("%-8s %-8s %-8s %-8s %-8s %-30s\n",
                "FldID","Word", "Bit", "SegLen","TotalLen", "Name");
        CTC_DKIT_PRINT("----------------------------------------------------------------------------------\n");

        /* value */
        for (fld_idx = 0; fld_idx < tbl_ptr->field_num; fld_idx++)
        {
            fld_ptr = &(tbl_ptr->ptr_fields[fld_idx]);
            ptr_field_name = fld_ptr->ptr_field_name;

            CTC_DKIT_PRINT("%-8d %-8d %-8d %-8d %-8d %-30s\n",
                fld_ptr->field_id,
                fld_ptr->ptr_seg[0].word_offset,
                fld_ptr->ptr_seg[0].start,
                fld_ptr->ptr_seg[0].bits,
                fld_ptr->bits,
                ptr_field_name);
            for (seg_id = 1; seg_id < fld_ptr->seg_num; seg_id ++)
            {
                CTC_DKIT_PRINT("%-8s %-8d %-8d %-8d \n",
                " ",
                fld_ptr->ptr_seg[seg_id].word_offset,
                fld_ptr->ptr_seg[seg_id].start,
                fld_ptr->ptr_seg[seg_id].bits);
            }
        }

    }
    CTC_DKIT_PRINT("----------------------------------------------------------------------------------\n\n");

    return CLI_SUCCESS;
}


STATIC int32
_ctc_usw_dkit_memory_str_to_upper(char* str)
{
    uint32  i = 0;

    /* convert the alphabets to upper case before comparison */
    for (i = 0; i < sal_strlen((char*)str); i++)
    {
        if (sal_isalpha((int)str[i]) && sal_islower((int)str[i]))
        {
            str[i] = sal_toupper(str[i]);
        }
    }

    return CLI_SUCCESS;
}


int32
_ctc_usw_dkit_memory_show_ram_by_data_tbl_id(uint8 lchip,  uint32 tbl_id, uint32 index, uint32* data_entry, char* regex, uint8 acc_io)
{
    uint32 value[MAX_ENTRY_WORD] = {0};
    uint8 uint32_id = 0;
    fields_t* fld_ptr = NULL;
    int32 fld_idx = 0;
    int32 cur_fld_idx = 0;
    char str[35] = {0};
    char format[10] = {0};
    tables_info_t* tbl_ptr = NULL;
    uint32 addr = 0;
    uint32 field_num = 0;
    char* ptr_field_name = NULL;
    uint8 print_fld = 0;
    char match_str[100] = {0};
    uint32* p_temp_data = NULL;
    int32 ret = 0;

    DRV_INIT_CHECK(lchip);

    if (DsL2Edit3WOuter_t == tbl_id)
    {
        tbl_id = DsL2EditEth3W_t;
        p_temp_data = (index&0x01)? ((uint32*)data_entry+3):data_entry;
    }
    else if (DsL3Edit3W3rd_t == tbl_id)
    {
        tbl_id = DsL3EditMpls3W_t;
        p_temp_data = (index&0x01)? ((uint32*)data_entry+3):data_entry;
    }
    else
    {
        p_temp_data = data_entry;
    }

    tbl_ptr = TABLE_INFO_PTR(lchip, tbl_id);
    field_num = tbl_ptr->field_num;


    for (fld_idx = fld_idx; fld_idx < field_num; fld_idx++)
    {
        sal_memset(value, 0 , sizeof(value));
        fld_ptr = &(tbl_ptr->ptr_fields[fld_idx]);

        drv_get_field(lchip, tbl_id, fld_ptr->field_id, p_temp_data, value);

        cur_fld_idx = fld_ptr->field_id;
        ptr_field_name = fld_ptr->ptr_field_name;

        if (NULL == regex)
        {
            if (0 == fld_idx)
            {
                sal_sprintf(match_str, "%-8u%-8u%-13s%-9u%-40s", index, cur_fld_idx, CTC_DKITS_HEX_FORMAT(str, format, value[0], 8), fld_ptr->bits, ptr_field_name);
            }
            else
            {
                sal_sprintf(match_str, "%-8s%-8u%-13s%-9u%-40s", "", cur_fld_idx, CTC_DKITS_HEX_FORMAT(str, format, value[0], 8), fld_ptr->bits, ptr_field_name);
            }

            print_fld = 1;
        }
        else
        {
            sal_sprintf(match_str, "%-8u%-8u%-13s%-9u%-40s", index, cur_fld_idx, CTC_DKITS_HEX_FORMAT(str, format, value[0], 8), fld_ptr->bits, ptr_field_name);
            print_fld = ctc_re_exec(match_str);
        }

        if (print_fld)
        {
            CTC_DKIT_PRINT("%s\n", match_str);

            for (uint32_id = 1; uint32_id < ((fld_ptr->bits + 31) / 32); uint32_id++)
            {
                CTC_DKIT_PRINT("%-16s%s\n", " ", CTC_DKITS_HEX_FORMAT(str, format, value[uint32_id], 8));
            }
        }

    }

    if (NULL == regex)
    {
        CTC_DKIT_PRINT("-------------------------------------------------------------\n");
    }


    /*Detail addr/data information*/
    if ((DRV_TABLE_TYPE_TCAM != drv_usw_get_table_type(lchip, tbl_id))
        && (DRV_TABLE_TYPE_LPM_TCAM_IP != drv_usw_get_table_type(lchip, tbl_id))
        && (DRV_TABLE_TYPE_LPM_TCAM_NAT != drv_usw_get_table_type(lchip, tbl_id))
        && (DRV_TABLE_TYPE_STATIC_TCAM_KEY != drv_usw_get_table_type(lchip, tbl_id))
        && (PacketHeaderOuter_t != tbl_id))
    {
        uint32 hash_key_type = 0;

        if ((_ctc_usw_dkit_memory_is_hash_key(lchip, tbl_id, &hash_key_type)) && acc_io)
        {
            uint8 step = 0;
            uint8 cam_num = 0;
            uint8 hash_module = 0;
            uint32 cam_tbl_id = 0;

            drv_usw_acc_get_hash_module_from_tblid(lchip, tbl_id, &hash_module);
            drv_usw_get_cam_info(lchip, hash_module, &cam_tbl_id, &cam_num);
            if (index < cam_num)
            {
                step =  TABLE_ENTRY_SIZE(lchip, cam_tbl_id)/TABLE_ENTRY_SIZE(lchip, tbl_id);
                ret = drv_get_table_property(lchip, DRV_TABLE_PROP_HW_ADDR, cam_tbl_id, (index/step), (uint32*)&addr);
                addr += (index%step)*TABLE_ENTRY_SIZE(lchip, tbl_id);
            }
            else
            {
                ret = drv_get_table_property(lchip, DRV_TABLE_PROP_HW_ADDR, tbl_id, (index - cam_num), (uint32*)&addr);
            }

        }
        else if (DsFwd_t == tbl_id || DsFwdHalf_t == tbl_id || DsIpfixSessionRecord_t == tbl_id)
        {
            ret = drv_get_table_property(lchip, DRV_TABLE_PROP_HW_ADDR, tbl_id, index/2, (uint32*)&addr);
        }
        else
        {
            ret = drv_get_table_property(lchip, DRV_TABLE_PROP_HW_ADDR, tbl_id, index, (uint32*)&addr);
        }

        if (ret)
        {
            return CLI_ERROR;
        }
    }
    CTC_DKIT_PRINT_DEBUG("%-6s %-10s  %-10s \n", "Word",  "Addr", "Value");
    for (uint32_id = 0; uint32_id < TABLE_ENTRY_SIZE(lchip, tbl_id) / DRV_BYTES_PER_WORD; uint32_id ++)
    {
        CTC_DKIT_PRINT_DEBUG("%-6u ", uint32_id);
        CTC_DKIT_PRINT_DEBUG("%-10s  ",   CTC_DKITS_HEX_FORMAT(str, format, addr + uint32_id * 4, 8));
        CTC_DKIT_PRINT_DEBUG("%-10s\n", CTC_DKITS_HEX_FORMAT(str, format, *(p_temp_data + uint32_id), 8));
    }

    return CLI_SUCCESS;
}

int32
_ctc_usw_dkit_memory_check_ram_by_data_tbl_id(uint8 lchip,  uint32 tbl_id, uint32 index, uint32* data_entry, char* regex, uint8 acc_io)
{
    uint32 value_wr[MAX_ENTRY_WORD] = {0};
    uint32 value_rd[MAX_ENTRY_WORD] = {0};
    uint32 value[MAX_ENTRY_WORD] = {0};
    uint8 uint32_id = 0;
    fields_t* fld_ptr = NULL;
    int32 fld_idx = 0;
    int32 cur_fld_idx = 0;
    char str_wr[35] = {0};
    char format_wr[10] = {0};
    char str_rd[35] = {0};
    char format_rd[10] = {0};
    tables_info_t* tbl_ptr = NULL;
    uint32 cmd = 0;
    uint32 field_num = 0;
    char* ptr_field_name = NULL;
    char match_str[110] = {0};
    uint32 result = 0;
    uint8 num = 0;

    DRV_INIT_CHECK(lchip);

    if (DsL2Edit3WOuter_t == tbl_id)
    {
        tbl_id = DsL2EditEth3W_t;
    }
    else if (DsL3Edit3W3rd_t == tbl_id)
    {
        tbl_id = DsL3EditMpls3W_t;
    }

    tbl_ptr = TABLE_INFO_PTR(lchip, tbl_id);
    field_num = tbl_ptr->field_num;


    for (fld_idx = fld_idx; fld_idx < field_num; fld_idx++)
    {
        for (num=0; num<2; num++)
        {
            sal_memset(value, 0 , sizeof(value));
            sal_memset(value_wr, 0 , sizeof(value_wr));
            sal_memset(value_rd, 0 , sizeof(value_rd));
            fld_ptr = &(tbl_ptr->ptr_fields[fld_idx]);

            if (0 == fld_idx%2 )
            {
                if (0 == num)
                {
                    sal_memset(value, 0x5a , sizeof(value));
                }
                else
                {
                    sal_memset(value, 0xa5 , sizeof(value));
                }
            }
            else
            {
                if (0 == num)
                {
                    sal_memset(value, 0xa5 , sizeof(value));
                }
                else
                {
                    sal_memset(value, 0x5a , sizeof(value));
                }
            }
            drv_get_field(lchip, tbl_id, fld_ptr->field_id, value, value_wr);
            cmd = DRV_IOW(tbl_id, fld_ptr->field_id);
            DRV_IOCTL(lchip, index, cmd, value_wr);

            cmd = DRV_IOR(tbl_id, fld_ptr->field_id);
            DRV_IOCTL(lchip, index, cmd, value_rd);

            result = 1;
            for (uint32_id = 0; uint32_id < ((fld_ptr->bits + 31) / 32); uint32_id++)
            {
                if ((value_wr[uint32_id]) != (value_rd[uint32_id]))
                {
                    result = 0;
                    break;
                }
            }

            cur_fld_idx = fld_ptr->field_id;
            ptr_field_name = fld_ptr->ptr_field_name;

            if (0 == fld_idx)
            {
                sal_sprintf(match_str, "%-8u%-8u%-13s%-13s%-9u%-9s%-40s", index, cur_fld_idx, CTC_DKITS_HEX_FORMAT(str_wr, format_wr, value_wr[0], 8), CTC_DKITS_HEX_FORMAT(str_rd, format_rd, value_rd[0], 8), fld_ptr->bits, (result?"Sucs":"FAIL"), ptr_field_name);
            }
            else
            {
                sal_sprintf(match_str, "%-8s%-8u%-13s%-13s%-9u%-9s%-40s", "", cur_fld_idx, CTC_DKITS_HEX_FORMAT(str_wr, format_wr, value_wr[0], 8), CTC_DKITS_HEX_FORMAT(str_rd, format_rd, value_rd[0], 8), fld_ptr->bits, (result?"Sucs":"FAIL"), ptr_field_name);
            }

            CTC_DKIT_PRINT("%s\n", match_str);

            for (uint32_id = 1; uint32_id < ((fld_ptr->bits + 31) / 32); uint32_id++)
            {
                CTC_DKIT_PRINT("%-16s%13s%13s\n", " ", CTC_DKITS_HEX_FORMAT(str_wr, format_wr, value_wr[uint32_id], 8), CTC_DKITS_HEX_FORMAT(str_rd, format_rd, value_rd[uint32_id], 8));
            }
        }
    }

    if (NULL == regex)
    {
        CTC_DKIT_PRINT("-------------------------------------------------------------\n");
    }

    return CLI_SUCCESS;
}


int32
ctc_usw_dkit_print_table(uint8 lchip,  uint32 tbl_id, uint32* data_entry)
{
    _ctc_usw_dkit_memory_show_ram_by_data_tbl_id(lchip,  tbl_id,  0, data_entry, NULL, 0);
    return 0;
}



STATIC int32
_ctc_usw_dkit_memory_is_hash_key(uint8 lchip, uint32 tbl_id, uint32* hash_key_type)
{
    uint8 is_hash_key = 0;

    switch (tbl_id)
    {
    case DsFibHost0MacHashKey_t:
        is_hash_key = 1;
        break;

    case DsFibHost0FcoeHashKey_t:
    case DsFibHost0Ipv4HashKey_t:
    case DsFibHost0Ipv6McastHashKey_t:
    case DsFibHost0Ipv6UcastHashKey_t:
    case DsFibHost0MacIpv6McastHashKey_t:
    case DsFibHost0TrillHashKey_t:
        is_hash_key = 1;
        break;

    case DsFibHost1FcoeRpfHashKey_t:
    case DsFibHost1Ipv4McastHashKey_t:
    case DsFibHost1Ipv4NatDaPortHashKey_t:
    case DsFibHost1Ipv4NatSaPortHashKey_t:
    case DsFibHost1Ipv6McastHashKey_t:
    case DsFibHost1Ipv6NatDaPortHashKey_t:
    case DsFibHost1Ipv6NatSaPortHashKey_t:
    case DsFibHost1MacIpv4McastHashKey_t:
    case DsFibHost1MacIpv6McastHashKey_t:
    case DsFibHost1TrillMcastVlanHashKey_t:
        is_hash_key = 1;
        break;

    case DsEgressXcOamBfdHashKey_t:
    case DsEgressXcOamCvlanCosPortHashKey_t:
    case DsEgressXcOamCvlanPortHashKey_t:
    case DsEgressXcOamDoubleVlanPortHashKey_t:
    case DsEgressXcOamEthHashKey_t:
    case DsEgressXcOamMplsLabelHashKey_t:
    case DsEgressXcOamMplsSectionHashKey_t:
    case DsEgressXcOamPortCrossHashKey_t:
    case DsEgressXcOamPortHashKey_t:
    case DsEgressXcOamPortVlanCrossHashKey_t:
    case DsEgressXcOamRmepHashKey_t:
    case DsEgressXcOamSvlanCosPortHashKey_t:
    case DsEgressXcOamSvlanPortHashKey_t:
    case DsEgressXcOamSvlanPortMacHashKey_t:
    case DsEgressXcOamTunnelPbbHashKey_t:
        is_hash_key = 1;
        break;

    case DsFlowL2HashKey_t:
    case DsFlowL2L3HashKey_t:
    case DsFlowL3Ipv4HashKey_t:
    case DsFlowL3Ipv6HashKey_t:
    case DsFlowL3MplsHashKey_t:
        is_hash_key = 1;
        break;

    case DsUserIdCapwapMacDaForwardHashKey_t:
    case DsUserIdCapwapStaStatusHashKey_t:
    case DsUserIdCapwapStaStatusMcHashKey_t:
    case DsUserIdCapwapVlanForwardHashKey_t:
    case DsUserIdCvlanCosPortHashKey_t:
    case DsUserIdCvlanPortHashKey_t:
    case DsUserIdDoubleVlanPortHashKey_t:
    case DsUserIdEcidNameSpaceHashKey_t:
    case DsUserIdIngEcidNameSpaceHashKey_t:
    case DsUserIdIpv4PortHashKey_t:
    case DsUserIdIpv4SaHashKey_t:
    case DsUserIdIpv6PortHashKey_t:
    case DsUserIdIpv6SaHashKey_t:
    case DsUserIdMacHashKey_t:
    case DsUserIdMacPortHashKey_t:
    case DsUserIdPortHashKey_t:
    case DsUserIdSclFlowL2HashKey_t:
    case DsUserIdSvlanCosPortHashKey_t:
    case DsUserIdSvlanHashKey_t:
    case DsUserIdSvlanMacSaHashKey_t:
    case DsUserIdSvlanPortHashKey_t:
    case DsUserIdTunnelCapwapRmacHashKey_t:
    case DsUserIdTunnelCapwapRmacRidHashKey_t:
    case DsUserIdTunnelIpv4CapwapHashKey_t:
    case DsUserIdTunnelIpv4DaHashKey_t:
    case DsUserIdTunnelIpv4GreKeyHashKey_t:
    case DsUserIdTunnelIpv4HashKey_t:
    case DsUserIdTunnelIpv4McNvgreMode0HashKey_t:
    case DsUserIdTunnelIpv4McVxlanMode0HashKey_t:
    case DsUserIdTunnelIpv4NvgreMode1HashKey_t:
    case DsUserIdTunnelIpv4RpfHashKey_t:
    case DsUserIdTunnelIpv4UcNvgreMode0HashKey_t:
    case DsUserIdTunnelIpv4UcNvgreMode1HashKey_t:
    case DsUserIdTunnelIpv4UcVxlanMode0HashKey_t:
    case DsUserIdTunnelIpv4UcVxlanMode1HashKey_t:
    case DsUserIdTunnelIpv4UdpHashKey_t:
    case DsUserIdTunnelIpv4VxlanMode1HashKey_t:
    case DsUserIdTunnelIpv6CapwapHashKey_t:
    case DsUserIdTunnelIpv6McNvgreMode0HashKey_t:
    case DsUserIdTunnelIpv6McNvgreMode1HashKey_t:
    case DsUserIdTunnelIpv6McVxlanMode0HashKey_t:
    case DsUserIdTunnelIpv6McVxlanMode1HashKey_t:
    case DsUserIdTunnelIpv6UcNvgreMode0HashKey_t:
    case DsUserIdTunnelIpv6UcNvgreMode1HashKey_t:
    case DsUserIdTunnelIpv6UcVxlanMode0HashKey_t:
    case DsUserIdTunnelIpv6UcVxlanMode1HashKey_t:
    case DsUserIdTunnelIpv6DaHashKey_t:
    case DsUserIdTunnelIpv6GreKeyHashKey_t:
    case DsUserIdTunnelIpv6HashKey_t:
    case DsUserIdTunnelIpv6UdpHashKey_t:
    case DsUserIdTunnelMplsHashKey_t:
    case DsUserIdTunnelPbbHashKey_t:
    case DsUserIdTunnelTrillMcAdjHashKey_t:
    case DsUserIdTunnelTrillMcDecapHashKey_t:
    case DsUserIdTunnelTrillMcRpfHashKey_t:
    case DsUserIdTunnelTrillUcDecapHashKey_t:
    case DsUserIdTunnelTrillUcRpfHashKey_t:
        /**< [TM] */
    case DsUserId1CapwapMacDaForwardHashKey_t:
    case DsUserId1CapwapStaStatusHashKey_t:
    case DsUserId1CapwapStaStatusMcHashKey_t:
    case DsUserId1CapwapVlanForwardHashKey_t:
    case DsUserId1CvlanCosPortHashKey_t:
    case DsUserId1CvlanPortHashKey_t:
    case DsUserId1DoubleVlanPortHashKey_t:
    case DsUserId1EcidNameSpaceHashKey_t:
    case DsUserId1IngEcidNameSpaceHashKey_t:
    case DsUserId1Ipv4PortHashKey_t:
    case DsUserId1Ipv4SaHashKey_t:
    case DsUserId1Ipv6PortHashKey_t:
    case DsUserId1Ipv6SaHashKey_t:
    case DsUserId1MacHashKey_t:
    case DsUserId1MacPortHashKey_t:
    case DsUserId1PortHashKey_t:
    case DsUserId1SclFlowL2HashKey_t:
    case DsUserId1SvlanCosPortHashKey_t:
    case DsUserId1SvlanHashKey_t:
    case DsUserId1SvlanMacSaHashKey_t:
    case DsUserId1SvlanPortHashKey_t:
    case DsUserId1TunnelCapwapRmacHashKey_t:
    case DsUserId1TunnelCapwapRmacRidHashKey_t:
    case DsUserId1TunnelIpv4CapwapHashKey_t:
    case DsUserId1TunnelIpv4DaHashKey_t:
    case DsUserId1TunnelIpv4GreKeyHashKey_t:
    case DsUserId1TunnelIpv4HashKey_t:
    case DsUserId1TunnelIpv4McNvgreMode0HashKey_t:
    case DsUserId1TunnelIpv4McVxlanMode0HashKey_t:
    case DsUserId1TunnelIpv4NvgreMode1HashKey_t:
    case DsUserId1TunnelIpv4RpfHashKey_t:
    case DsUserId1TunnelIpv4UcNvgreMode0HashKey_t:
    case DsUserId1TunnelIpv4UcNvgreMode1HashKey_t:
    case DsUserId1TunnelIpv4UcVxlanMode0HashKey_t:
    case DsUserId1TunnelIpv4UcVxlanMode1HashKey_t:
    case DsUserId1TunnelIpv4UdpHashKey_t:
    case DsUserId1TunnelIpv4VxlanMode1HashKey_t:
    case DsUserId1TunnelIpv6CapwapHashKey_t:
    case DsUserId1TunnelIpv6McNvgreMode0HashKey_t:
    case DsUserId1TunnelIpv6McNvgreMode1HashKey_t:
    case DsUserId1TunnelIpv6McVxlanMode0HashKey_t:
    case DsUserId1TunnelIpv6McVxlanMode1HashKey_t:
    case DsUserId1TunnelIpv6UcNvgreMode0HashKey_t:
    case DsUserId1TunnelIpv6UcNvgreMode1HashKey_t:
    case DsUserId1TunnelIpv6UcVxlanMode0HashKey_t:
    case DsUserId1TunnelIpv6UcVxlanMode1HashKey_t:
    case DsUserId1TunnelIpv6DaHashKey_t:
    case DsUserId1TunnelIpv6GreKeyHashKey_t:
    case DsUserId1TunnelIpv6HashKey_t:
    case DsUserId1TunnelIpv6UdpHashKey_t:
    case DsUserId1TunnelTrillMcAdjHashKey_t:
    case DsUserId1TunnelTrillMcDecapHashKey_t:
    case DsUserId1TunnelTrillMcRpfHashKey_t:
    case DsUserId1TunnelTrillUcDecapHashKey_t:
    case DsUserId1TunnelTrillUcRpfHashKey_t:
        is_hash_key = 1;
        break;

    case DsIpfixL2HashKey_t:
    case DsIpfixL2L3HashKey_t:
    case DsIpfixL3Ipv4HashKey_t:
    case DsIpfixL3Ipv6HashKey_t:
    case DsIpfixL3MplsHashKey_t:
    case DsIpfixSessionRecord_t:
    case DsIpfixL2HashKey0_t:
    case DsIpfixL2L3HashKey0_t:
    case DsIpfixL3Ipv4HashKey0_t:
    case DsIpfixL3Ipv6HashKey0_t:
    case DsIpfixL3MplsHashKey0_t:
    case DsIpfixSessionRecord0_t:
    case DsIpfixL2HashKey1_t:
    case DsIpfixL2L3HashKey1_t:
    case DsIpfixL3Ipv4HashKey1_t:
    case DsIpfixL3Ipv6HashKey1_t:
    case DsIpfixL3MplsHashKey1_t:
    case DsIpfixSessionRecord1_t:
        is_hash_key = 1;
        break;

    case DsMplsLabelHashKey_t:
        is_hash_key = 1;
        break;

    case DsGemPortHashKey_t:
        is_hash_key = 1;
        break;

    default:
        is_hash_key = 0;
    }

    return is_hash_key;

}

STATIC int
_ctc_usw_dkit_memory_acc_read(uint8 lchip, uint32 tbl_id, uint32 index, uint32 hash_key_type, void *p_data)
{
    int ret = CLI_SUCCESS;

    drv_acc_in_t  in;
    drv_acc_out_t out;

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));

    in.type = DRV_ACC_TYPE_LOOKUP;
    in.tbl_id = tbl_id;
    in.index = index;
    in.op_type = DRV_ACC_OP_BY_INDEX;
    ret = drv_acc_api(lchip, &in, &out);

    sal_memcpy((uint8*)p_data, (uint8*)(out.data), TABLE_ENTRY_SIZE(lchip, tbl_id));

    return ret;


}

STATIC int32
_ctc_usw_dkit_memory_acc_write(uint8 lchip, uint32 tbl_id, uint32 index, uint32 hash_key_type, void *p_data)
{
    int ret = CLI_SUCCESS;

    drv_acc_in_t  in;
    drv_acc_out_t out;

    sal_memset(&in, 0, sizeof(in));
    sal_memset(&out, 0, sizeof(out));

    in.type = DRV_ACC_TYPE_ADD;
    in.tbl_id = tbl_id;
    in.data = p_data;
    in.index = index;
    in.op_type = DRV_ACC_OP_BY_INDEX;
    ret = drv_acc_api(lchip, &in, &out);


    return ret;


}

STATIC int32
_ctc_usw_dkit_memory_show_tcam_by_data_tbl_id(uint8 lchip, uint32 tbl_id, uint32 index, uint32* data_entry, uint32* mask_entry, char* regex)
{
    uint32* value = NULL;
    uint32* mask = NULL;
    uint8 uint32_id = 0;
    fields_t* fld_ptr = NULL;
    int32 fld_idx = 0;
    tables_info_t* tbl_ptr = NULL;
    uint32 addr = 0;
    uint8 print_fld = 0;
    char match_str[100] = {0};

    tbl_ptr = TABLE_INFO_PTR(lchip, tbl_id);

    value = (uint32*)sal_malloc(MAX_ENTRY_WORD * sizeof(uint32));
    if(NULL == value)
    {
        return CLI_ERROR;
    }
    mask = (uint32*)sal_malloc(MAX_ENTRY_WORD * sizeof(uint32));
    if(NULL == mask)
    {
        sal_free(value);
        return CLI_ERROR;
    }


    for (fld_idx = 0; fld_idx < tbl_ptr->field_num; fld_idx++)
    {
        sal_memset(value, 0 , MAX_ENTRY_WORD * sizeof(uint32));
        sal_memset(mask, 0 , MAX_ENTRY_WORD * sizeof(uint32));
        fld_ptr = &(tbl_ptr->ptr_fields[fld_idx]);
        drv_get_field(lchip, tbl_id, fld_ptr->field_id, data_entry, value);
        drv_get_field(lchip, tbl_id, fld_ptr->field_id, mask_entry, mask);

        if (NULL == regex)
        {
            if (0 == fld_idx)
            {
                sal_sprintf(match_str, "%-6d %-6d 0x%08x 0x%08x %-6d %-10s", index,  fld_ptr->field_id, value[0], mask[0], fld_ptr->bits, fld_ptr->ptr_field_name);
            }
            else
            {
                sal_sprintf(match_str, "%-6s %-6d 0x%08x 0x%08x %-6d %-10s", "", fld_ptr->field_id, value[0], mask[0], fld_ptr->bits, fld_ptr->ptr_field_name);
            }

            print_fld = 1;
        }
        else
        {
            sal_sprintf(match_str, "%-6d %-6d 0x%08x 0x%08x %-6d %-10s", index,  fld_ptr->field_id, value[0], mask[0], fld_ptr->bits, fld_ptr->ptr_field_name);

            print_fld = ctc_re_exec(match_str);
        }


        if (print_fld)
        {
            CTC_DKIT_PRINT("%s\n", match_str);

            for (uint32_id = 1; uint32_id < ((fld_ptr->bits + 31) / 32); uint32_id ++)
            {
                CTC_DKIT_PRINT("%-13s 0x%08x 0x%08x\n", " ", value[uint32_id], mask[uint32_id]);
            }
        }

    }

    if (NULL == regex)
    {
        CTC_DKIT_PRINT("----------------------------------------------------------------------------------\n");
    }

    CTC_DKIT_PRINT_DEBUG("%-6s %-10s  %-10s \n", "Word",  "Addr", "Value");
    for (uint32_id = 0; uint32_id < TABLE_ENTRY_SIZE(lchip, tbl_id) / DRV_BYTES_PER_WORD; uint32_id ++)
    {
        CTC_DKIT_PRINT_DEBUG("%-6d 0x%08x 0x%08x\n", uint32_id, addr + uint32_id*4, *(data_entry + uint32_id));
    }

    CTC_DKIT_PRINT("\n");

    if(NULL != value)
    {
        sal_free(value);
    }
    if(NULL != mask)
    {
        sal_free(mask);
    }

    return CLI_SUCCESS;

}


STATIC int32
_ctc_usw_dkit_memory_list_table(uint8 lchip, ctc_dkit_memory_para_t* p_memory_para)
{
#define CTC_GOLDENGEAT_LIST_MAX_NUM   9600
    uint32  *ids = NULL;
    char*    upper_name = NULL;
    char*    upper_name1 = NULL;
    char*    upper_name_grep = NULL;
    uint32  i = 0;
    uint32  id_index = 0;
    uint32  first_flag = TRUE;
    char* p_char = NULL;
    int32 ret = CLI_SUCCESS;

    DKITS_PTR_VALID_CHECK(p_memory_para);
    DRV_INIT_CHECK(lchip);

    ids = (uint32 *)sal_malloc(CTC_GOLDENGEAT_LIST_MAX_NUM * sizeof(uint32));
    if(ids == NULL)
    {
        CTC_DKIT_PRINT("%% No memory\n");
        return CLI_ERROR;
    }
    upper_name = (char*)sal_malloc(CTC_DKITS_RW_BUF_SIZE * sizeof(char));
    if(upper_name == NULL)
    {
        CTC_DKIT_PRINT("%% No memory\n");
        ret = CLI_ERROR;
        goto clean;
    }
    upper_name1 = (char*)sal_malloc(CTC_DKITS_RW_BUF_SIZE * sizeof(char));
    if(upper_name1 == NULL)
    {
        CTC_DKIT_PRINT("%% No memory\n");
        ret = CLI_ERROR;
        goto clean;
    }
    upper_name_grep = (char*)sal_malloc(CTC_DKITS_RW_BUF_SIZE * sizeof(char));
    if(upper_name_grep == NULL)
    {
        CTC_DKIT_PRINT("%% No memory\n");
        ret = CLI_ERROR;
        goto clean;
    }

    sal_memset(ids, 0, CTC_GOLDENGEAT_LIST_MAX_NUM * sizeof(uint32));
    sal_memset(upper_name, 0, CTC_DKITS_RW_BUF_SIZE * sizeof(char));
    sal_memset(upper_name1, 0, CTC_DKITS_RW_BUF_SIZE * sizeof(char));
    sal_memset(upper_name_grep, 0, CTC_DKITS_RW_BUF_SIZE * sizeof(char));

    if (0 == sal_strlen((char*)p_memory_para->buf))
    {
        /* list all */
        CTC_DKIT_PRINT("List all %d entries\n", MaxTblId_t);

        for (i = 0; i < MaxTblId_t; i++)
        {
            _ctc_usw_dkit_memory_tblreg_dump_list_one(lchip, i, &first_flag, p_memory_para->detail);
        }
        goto clean;
    }

    sal_strncpy((char*)upper_name, (char*)p_memory_para->buf, CTC_DKITS_RW_BUF_SIZE);
    _ctc_usw_dkit_memory_str_to_upper(upper_name);

    if (p_memory_para->grep)
    {
        sal_strncpy((char*)upper_name_grep, (char*)p_memory_para->buf2, CTC_DKITS_RW_BUF_SIZE);
        _ctc_usw_dkit_memory_str_to_upper(upper_name_grep);
    }

    for (i = 0; i < MaxTblId_t; i++)
    {
        drv_get_table_property(lchip, DRV_TABLE_PROP_GET_NAME, i, 0, (char*)upper_name1);
        _ctc_usw_dkit_memory_str_to_upper(upper_name1);

        p_char = (char*)sal_strstr((char*)upper_name1, (char*)upper_name);
        if (p_char)
        {
            if (0 == TABLE_MAX_INDEX(lchip, i))
            {
                continue;
            }

            if (sal_strlen((char*)upper_name1) == sal_strlen((char*)upper_name))
            {
                /* list full matched */
                _ctc_usw_dkit_memory_tblreg_dump_chip_read_tbl_db_by_id(lchip, upper_name1, i, p_memory_para->detail);
                goto clean;
            }

            if (p_memory_para->grep)
            {
                p_char = NULL;
                p_char = (char*)sal_strstr((char*)upper_name1, (char*)upper_name_grep);
                if (p_char)
                {
                    ids[id_index] = i;
                    id_index++;
                }
            }
            else
            {
                ids[id_index] = i;
                id_index++;
            }

            if (id_index >= CTC_GOLDENGEAT_LIST_MAX_NUM)
            {
                break;
            }
        }
    }

    if (id_index == 0)
    {
        CTC_DKIT_PRINT("%% Not found %s \n", p_memory_para->buf);
        goto clean;
    }

    for (i = 0; i < id_index; i++)
    {
        /* list matched */
        _ctc_usw_dkit_memory_tblreg_dump_list_one(lchip, ids[i], &first_flag, p_memory_para->detail);
    }

    if (p_memory_para->detail)
    {
        CTC_DKIT_PRINT("----------------------------------------------------------------------------------\n");
    }
    if (id_index >= CTC_GOLDENGEAT_LIST_MAX_NUM)
    {
        CTC_DKIT_PRINT("Too much matched, only display first %d entries. \n", CTC_GOLDENGEAT_LIST_MAX_NUM);
    }
    CTC_DKIT_PRINT("\n");

clean:
    if(NULL != ids)
    {
        sal_free(ids);
    }
    if(NULL != upper_name)
    {
        sal_free(upper_name);
    }
    if(NULL != upper_name1)
    {
        sal_free(upper_name1);
    }
    if(NULL != upper_name_grep)
    {
        sal_free(upper_name_grep);
    }

    return ret;
}

int32
_ctc_usw_dkit_memory_read_table(uint8 lchip, ctc_dkit_memory_para_t* p_memory_para)
{
    char* tbl_name = NULL;
    int32 start_index = 0;
    uint32 count = 0;
    uint8 step = 0;
    uint32 tbl_id = 0;
    int32 ret = CLI_SUCCESS;
    uint32 cmd = 0;
    tbl_entry_t entry;
    uint32* data_entry = NULL;
    uint32* mask_entry = NULL;
    int32 i = 0;
    uint32 index = 0;
    uint32 index_temp = 0;
    uint32 hash_key_type = CTC_DKIT_MEMORY_HASH_KEY_TPYE_MAX;
    char* regex = NULL;
    uint32 is_half = 0;

    DKITS_PTR_VALID_CHECK(p_memory_para);
    DRV_INIT_CHECK(lchip);

    data_entry = (uint32*)sal_malloc(MAX_ENTRY_WORD * sizeof(uint32));
    if(NULL == data_entry)
    {
        return CLI_ERROR;
    }

    mask_entry = (uint32*)sal_malloc(MAX_ENTRY_WORD * sizeof(uint32));
    if(NULL == mask_entry)
    {
        sal_free(data_entry);
        return CLI_ERROR;
    }

    sal_memset(data_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));
    sal_memset(mask_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));

    lchip = p_memory_para->param[1];
    start_index = p_memory_para->param[2];
    count = p_memory_para->param[4];
    step = p_memory_para->param[5];
    tbl_name = (char*)p_memory_para->buf;



    if (p_memory_para->grep)
    {
        regex = (char*)p_memory_para->buf2;
    }

    if (NULL != regex)
    {
        char* p = NULL;

        p = ctc_re_comp(regex);

        if (NULL != p)
        {
            CTC_DKIT_PRINT("Not match regex  %s: %s\n", p, regex);
            ret = 0;
            goto clean;
        }
    }



    if (CTC_DKITS_INVALID_TBL_ID != p_memory_para->param[3])
    {
        tbl_id = p_memory_para->param[3];
        if((tbl_id) <= 0 || (tbl_id) >= MaxTblId_t)
        {
            ret = CLI_ERROR;
            goto clean;
        }
        drv_get_table_property(lchip, DRV_TABLE_PROP_GET_NAME, tbl_id, 0, (char*)tbl_name);
        CTC_DKIT_PRINT_DEBUG("read %s %d count %d step %d\n", tbl_name, start_index, count, step);
    }
    else
    {
        if (drv_usw_get_tbl_id_by_string(lchip, &tbl_id, tbl_name)
            || (0 == TABLE_MAX_INDEX(lchip, tbl_id) && ((DsL2Edit3WOuter_t != tbl_id && DsL3Edit3W3rd_t != tbl_id))))
        {
            CTC_DKIT_PRINT("%% Not found %s\n", tbl_name);
            ret = CLI_ERROR;
            goto clean;
        }
    }
    if (dkits_log_file)
    {
        CTC_DKIT_PRINT("\n%-15s %-15s\n", "Tbl-name:", tbl_name);
    }

    if ((DRV_TABLE_TYPE_TCAM == drv_usw_get_table_type(lchip, tbl_id))
        || (DRV_TABLE_TYPE_LPM_TCAM_IP == drv_usw_get_table_type(lchip, tbl_id))
    || (DRV_TABLE_TYPE_LPM_TCAM_NAT == drv_usw_get_table_type(lchip, tbl_id))
    || (DRV_TABLE_TYPE_STATIC_TCAM_KEY == drv_usw_get_table_type(lchip, tbl_id)))
    {
        if (dkits_log_file)
        {
            CTC_DKIT_PRINT("----------------------------------------------------------------------------------");
        }
        /* TCAM */
        CTC_DKIT_PRINT("\n%-6s %-6s %-10s %-10s %-6s %-10s\n", "Index", "FldID", "Value", "Mask", "BitLen", "Name");
        CTC_DKIT_PRINT("----------------------------------------------------------------------------------\n");

    }
    else
    {
        if (dkits_log_file)
        {
            CTC_DKIT_PRINT("-------------------------------------------------------------");
        }
        /* non-TCAM */
        CTC_DKIT_PRINT("\n%-8s%-8s%-13s%-9s%-40s\n", "Index", "FldID", "Value", "BitLen", "FldName");
        CTC_DKIT_PRINT("-------------------------------------------------------------\n");
    }


    for (i = 0; i < count; i++)
    {
        index = (start_index + (i * step));
        if ((DRV_TABLE_TYPE_TCAM == drv_usw_get_table_type(lchip, tbl_id))
        || (DRV_TABLE_TYPE_LPM_TCAM_IP == drv_usw_get_table_type(lchip, tbl_id))
        || (DRV_TABLE_TYPE_LPM_TCAM_NAT == drv_usw_get_table_type(lchip, tbl_id))
        || (DRV_TABLE_TYPE_STATIC_TCAM_KEY == drv_usw_get_table_type(lchip, tbl_id)))
        {
            entry.data_entry = data_entry;
            entry.mask_entry = mask_entry;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            if((DRV_TABLE_TYPE_LPM_TCAM_IP == drv_usw_get_table_type(lchip, tbl_id)) ||
                (DRV_TABLE_TYPE_LPM_TCAM_NAT == drv_usw_get_table_type(lchip, tbl_id)))
            {
                FTM_TCAM_IOCTL(lchip, index, cmd, &entry);
            }
            else
            {
                DRV_IOCTL(lchip, index, cmd, &entry);
            }
            _ctc_usw_dkit_memory_show_tcam_by_data_tbl_id(lchip, tbl_id, index, data_entry, mask_entry, regex);
        }
        else
        {
            if ((_ctc_usw_dkit_memory_is_hash_key(lchip, tbl_id, &hash_key_type))&&(!p_memory_para->direct_io))
            {
                ret =  _ctc_usw_dkit_memory_acc_read(lchip, tbl_id, index, hash_key_type, (void*)(data_entry));/*merge table and cam*/
            }
            else
            {
                uint32 tbl_id_tmp = tbl_id;

                if (DsFwdHalf_t == tbl_id_tmp)
                {
                    tbl_id_tmp = DsFwd_t;
                }

                if ((DsIpfixSessionRecord_t == tbl_id_tmp) || (DsFwd_t == tbl_id_tmp) )
                {
                    index_temp = index / 2;
                }
                else
                {
                    index_temp = index;
                }

                if (DsL2Edit3WOuter_t == tbl_id)
                {
                    tbl_id_tmp = DsL2Edit6WOuter_t;
                    index_temp = index/2;
                }

                if (DsL3Edit3W3rd_t == tbl_id)
                {
                    tbl_id_tmp = DsL3Edit6W3rd_t;
                    index_temp = index/2;
                }

                cmd = DRV_IOR(tbl_id_tmp, DRV_ENTRY_FLAG);
                ret = DRV_IOCTL(lchip, index_temp, cmd, data_entry);
            }
            if (CLI_SUCCESS == ret)
            {
                if (DsFwd_t == tbl_id)
                {
                    drv_get_field(lchip, DsFwd_t, DsFwd_isHalf_f, data_entry, (uint32*)&is_half);
                }
                if ((DsFwdHalf_t == tbl_id) || ((DsFwd_t == tbl_id)&&is_half))
                {
                    DsFwdHalf_m dsfwd_half;

                    if (index % 2 == 0)
                    {
                        drv_get_field(lchip, DsFwdDualHalf_t, DsFwdDualHalf_g_0_dsFwdHalf_f,   data_entry,   (uint32*)&dsfwd_half);
                    }
                    else
                    {
                        drv_get_field(lchip, DsFwdDualHalf_t, DsFwdDualHalf_g_1_dsFwdHalf_f,   data_entry,   (uint32*)&dsfwd_half);
                    }

                    sal_memcpy(data_entry, &dsfwd_half, sizeof(dsfwd_half));
                }
                _ctc_usw_dkit_memory_show_ram_by_data_tbl_id(lchip, tbl_id, index, data_entry, regex, !p_memory_para->direct_io);
            }
        }
    }
    if (ret < 0)
    {
        CTC_DKIT_PRINT("%% Read index %d failed %d\n", index, ret);
    }
clean:
    sal_free(data_entry);
    sal_free(mask_entry);

    return ret;
}

STATIC int32
_ctc_usw_dkit_memory_write_table(uint8 lchip, ctc_dkit_memory_para_t* p_memory_para)
{
    char* reg_name = NULL;
    char* fld_name = NULL;
    int32 index = 0;
    uint32 index_temp = 0;
    uint32 reg_tbl_id = 0;
    uint32 input_tbl_id = 0;
    uint32 tmp_reg_tbl_id = 0;
    uint32* p_value = NULL;
    uint32* p_mask = NULL;
    uint32* mask = NULL;
    uint32* value = NULL;
    uint32 cmd = 0;
    int32 ret = 0;
    fld_id_t fld_id = 0;
    tbl_entry_t field_entry;
    uint32 hash_key_type = CTC_DKIT_MEMORY_HASH_KEY_TPYE_MAX;
    char* fld_name_temp = NULL;
    DsL2EditEth3W_m ds_l2_edit_eth3_w;
    DsL2Edit6WOuter_m ds_l2_edit6_w_outer;
    DsL3Edit3W3rd_m ds_mpls_3w;
    DsL3Edit6W3rd_m ds_mpls_6w;
    DsFwdHalf_m dsfwd_half;
    uint32 is_half = 0;

    DKITS_PTR_VALID_CHECK(p_memory_para);
    DRV_INIT_CHECK(lchip);

    mask = (uint32*)sal_malloc(MAX_ENTRY_WORD * sizeof(uint32));
    value = (uint32*)sal_malloc(MAX_ENTRY_WORD * sizeof(uint32));
    fld_name_temp = (char*)sal_malloc(MAX_ENTRY_WORD * sizeof(char));

    if (!mask || !value || !fld_name_temp)
    {
        ret = CLI_ERROR;
        goto WRITE_TABLE_ERROR;
    }

    sal_memset(mask, 0, MAX_ENTRY_WORD * sizeof(uint32));
    sal_memset(value, 0, MAX_ENTRY_WORD * sizeof(uint32));
    sal_memset(fld_name_temp, 0, MAX_ENTRY_WORD * sizeof(char));

    lchip = p_memory_para->param[1];
    index_temp = p_memory_para->param[2];
    p_value =(uint32*)p_memory_para->value;
    p_mask = (uint32*)p_memory_para->mask;
    reg_name = (char*)p_memory_para->buf;
    fld_name = (char*)p_memory_para->buf2;


    if (CTC_DKITS_INVALID_TBL_ID != p_memory_para->param[3])
    {
        reg_tbl_id = p_memory_para->param[3];
        drv_get_table_property(lchip, DRV_TABLE_PROP_GET_NAME, reg_tbl_id, 0, (char*)reg_name);
    }
    else
    {
        if (DRV_E_NONE != drv_usw_get_tbl_id_by_string(lchip, &reg_tbl_id, reg_name)
            || (0 == TABLE_MAX_INDEX(lchip, reg_tbl_id) && ((DsL2Edit3WOuter_t != reg_tbl_id && DsL3Edit3W3rd_t != reg_tbl_id))))
        {
            CTC_DKIT_PRINT("%% Not found %s\n", reg_name);
            ret = CLI_ERROR;
            goto WRITE_TABLE_ERROR;
        }
    }

    if ((reg_tbl_id) <= 0 || (reg_tbl_id) >= MaxTblId_t)
    {
        ret = CLI_ERROR;
        goto WRITE_TABLE_ERROR;
    }

    if (p_memory_para->param[4] != 0xFFFFFFFF)
    {
        fld_id = p_memory_para->param[4];

        drv_usw_get_field_string_by_id(lchip, reg_tbl_id , fld_id, fld_name);
    }
    else
    {
        if (DsL2Edit3WOuter_t == reg_tbl_id)
        {
            tmp_reg_tbl_id = DsL2EditEth3W_t;
        }
        else if (DsL3Edit3W3rd_t == reg_tbl_id)
        {
            tmp_reg_tbl_id = DsL3EditMpls3W_t;
        }
        else
        {
            tmp_reg_tbl_id = reg_tbl_id;
        }
        if (DRV_E_NONE != drv_usw_get_field_id_by_string(lchip, tmp_reg_tbl_id, &fld_id, fld_name))
        {
            CTC_DKIT_PRINT("%% Not found %s\n", fld_name);
            ret = CLI_ERROR;
            goto WRITE_TABLE_ERROR;
        }
    }

    CTC_DKIT_PRINT_DEBUG("Write %s %d %s\n", reg_name, index_temp, fld_name);
    if ((DsIpfixSessionRecord_t == reg_tbl_id))
    {
        index = index_temp / 2;
    }
    else if ((DsFwd_t == reg_tbl_id) || (DsFwdHalf_t == reg_tbl_id))
    {
        index = index_temp / 2;
        reg_tbl_id = DsFwd_t;
    }
    else if (DsL2Edit3WOuter_t == reg_tbl_id)
    {
        index = index_temp / 2;
        reg_tbl_id = DsL2Edit6WOuter_t;
        input_tbl_id = DsL2Edit3WOuter_t;
    }
    else if (DsL3Edit3W3rd_t == reg_tbl_id)
    {
        index = index_temp / 2;
        reg_tbl_id = DsL3Edit6W3rd_t;
        input_tbl_id = DsL3Edit3W3rd_t;
    }
    else
    {
        index = index_temp;
    }
    /*1.read firstly*/
    DRV_TBL_EMPTY_CHECK(0, reg_tbl_id);


    cmd = DRV_IOR(reg_tbl_id, DRV_ENTRY_FLAG);
    if (DRV_TABLE_TYPE_TCAM == drv_usw_get_table_type(lchip, reg_tbl_id)
        || DRV_TABLE_TYPE_LPM_TCAM_IP == drv_usw_get_table_type(lchip, reg_tbl_id)
        || DRV_TABLE_TYPE_STATIC_TCAM_KEY == drv_usw_get_table_type(lchip, reg_tbl_id)
        || DRV_TABLE_TYPE_LPM_TCAM_NAT == drv_usw_get_table_type(lchip, reg_tbl_id))
    {
        field_entry.data_entry = value;
        field_entry.mask_entry = mask;
        if((DRV_TABLE_TYPE_LPM_TCAM_IP == drv_usw_get_table_type(lchip, reg_tbl_id)) ||
                (DRV_TABLE_TYPE_LPM_TCAM_NAT == drv_usw_get_table_type(lchip, reg_tbl_id)))
        {
            ret = FTM_TCAM_IOCTL(lchip, index, cmd, &field_entry);
        }
        else
        {
            ret = DRV_IOCTL(lchip, index, cmd, &field_entry);
        }
    }
    else
    {
        if ((_ctc_usw_dkit_memory_is_hash_key(lchip, reg_tbl_id, &hash_key_type))&&(!p_memory_para->direct_io))
        {
            _ctc_usw_dkit_memory_acc_read(lchip, reg_tbl_id, index_temp, hash_key_type, (void*)(value));
        }
        else
        {
            ret = DRV_IOCTL(lchip, index, cmd, value);
            if (DsFwd_t == reg_tbl_id)
            {
                drv_get_field(lchip, DsFwd_t, DsFwd_isHalf_f, value, (uint32*)&is_half);
            }
            if (DsL2Edit3WOuter_t == input_tbl_id)
            {
                sal_memset(&ds_l2_edit_eth3_w, 0, sizeof(ds_l2_edit_eth3_w));
                sal_memset(&ds_l2_edit6_w_outer, 0, sizeof(ds_l2_edit6_w_outer));
                sal_memcpy((uint8*)&ds_l2_edit6_w_outer, (uint8*)value, sizeof(ds_l2_edit6_w_outer));
                if ((index_temp&0x01))
                {
                    sal_memcpy((uint8*)&ds_l2_edit_eth3_w, (uint8*)&ds_l2_edit6_w_outer + sizeof(ds_l2_edit_eth3_w), sizeof(ds_l2_edit_eth3_w));
                }
                else
                {
                    sal_memcpy((uint8*)&ds_l2_edit_eth3_w, (uint8*)&ds_l2_edit6_w_outer, sizeof(ds_l2_edit_eth3_w));
                }
            }
            else if (DsL3Edit3W3rd_t == input_tbl_id)
            {
                sal_memset(&ds_mpls_3w, 0, sizeof(ds_mpls_3w));
                sal_memset(&ds_mpls_6w, 0, sizeof(ds_mpls_6w));
                sal_memcpy((uint8*)&ds_mpls_6w, (uint8*)value, sizeof(ds_mpls_6w));
                if ((index_temp&0x01))
                {
                    sal_memcpy((uint8*)&ds_mpls_3w, (uint8*)&ds_mpls_6w + sizeof(ds_mpls_3w), sizeof(ds_mpls_3w));
                }
                else
                {
                    sal_memcpy((uint8*)&ds_mpls_3w, (uint8*)&ds_mpls_6w, sizeof(ds_mpls_3w));
                }
            }
            else if ((DsFwd_t == reg_tbl_id) && is_half)
            {
                if (index_temp % 2 == 0)
                {
                    drv_get_field(lchip, DsFwdDualHalf_t, DsFwdDualHalf_g_0_dsFwdHalf_f, value, (uint32*)&dsfwd_half);
                }
                else
                {
                    drv_get_field(lchip, DsFwdDualHalf_t, DsFwdDualHalf_g_1_dsFwdHalf_f, value, (uint32*)&dsfwd_half);
                }
            }
        }
    }
    /*2.write secondly*/
    cmd = DRV_IOW(reg_tbl_id, DRV_ENTRY_FLAG);
    if ((DRV_TABLE_TYPE_TCAM == drv_usw_get_table_type(lchip, reg_tbl_id)
        || DRV_TABLE_TYPE_LPM_TCAM_IP == drv_usw_get_table_type(lchip, reg_tbl_id)
        || DRV_TABLE_TYPE_LPM_TCAM_NAT == drv_usw_get_table_type(lchip, reg_tbl_id)
        || DRV_TABLE_TYPE_STATIC_TCAM_KEY == drv_usw_get_table_type(lchip, reg_tbl_id)))
    {
        drv_set_field(lchip, reg_tbl_id, fld_id, field_entry.data_entry, p_value);
        drv_set_field(lchip, reg_tbl_id, fld_id, field_entry.mask_entry, p_mask);
        if((DRV_TABLE_TYPE_LPM_TCAM_IP == drv_usw_get_table_type(lchip, reg_tbl_id)) ||
                (DRV_TABLE_TYPE_LPM_TCAM_NAT == drv_usw_get_table_type(lchip, reg_tbl_id)))
        {
            ret = FTM_TCAM_IOCTL(lchip, index, cmd, &field_entry);
        }
        else
        {
            ret = DRV_IOCTL(lchip, index, cmd, &field_entry);
        }
    }
    else
    {
        if (DsL2Edit3WOuter_t == input_tbl_id)
        {
            input_tbl_id = DsL2EditEth3W_t; /*drv only parse DsL2EditEth3W_t, other than DsL2Edit3WOuter_t*/
            drv_set_field(lchip, input_tbl_id, fld_id, (uint8*)&ds_l2_edit_eth3_w, p_value);
            if ((index_temp&0x01))
            {
                sal_memcpy((uint8*)&ds_l2_edit6_w_outer + sizeof(ds_l2_edit_eth3_w), (uint8*)&ds_l2_edit_eth3_w, sizeof(ds_l2_edit_eth3_w));
            }
            else
            {
                sal_memcpy((uint8*)&ds_l2_edit6_w_outer, (uint8*)&ds_l2_edit_eth3_w, sizeof(ds_l2_edit_eth3_w));
            }
            sal_memcpy((uint8*)value, (uint8*)&ds_l2_edit6_w_outer, sizeof(ds_l2_edit6_w_outer));
        }
        else if (DsL3Edit3W3rd_t == input_tbl_id)
        {
            input_tbl_id = DsL3EditMpls3W_t;
            drv_set_field(lchip, input_tbl_id, fld_id, (uint8*)&ds_mpls_3w, p_value);
            if ((index_temp&0x01))
            {
                sal_memcpy((uint8*)&ds_mpls_6w + sizeof(ds_mpls_3w), (uint8*)&ds_mpls_3w, sizeof(ds_mpls_3w));
            }
            else
            {
                sal_memcpy((uint8*)&ds_mpls_6w, (uint8*)&ds_mpls_3w, sizeof(ds_mpls_3w));
            }
            sal_memcpy((uint8*)value, (uint8*)&ds_mpls_6w, sizeof(ds_mpls_6w));
        }
        else if (DsFwd_t == reg_tbl_id && is_half)
        {
            drv_set_field(lchip, DsFwdHalf_t, fld_id, (uint8*)&dsfwd_half, p_value);
            if (index_temp % 2 == 0)
            {
                drv_set_field(lchip, DsFwdDualHalf_t, DsFwdDualHalf_g_0_dsFwdHalf_f, (uint8*)value, (uint32*)&dsfwd_half);
            }
            else
            {
                drv_set_field(lchip, DsFwdDualHalf_t, DsFwdDualHalf_g_1_dsFwdHalf_f, (uint8*)value, (uint32*)&dsfwd_half);
            }
        }
        else
        {
            drv_set_field(lchip, reg_tbl_id, fld_id, value, p_value);
        }
        if ((_ctc_usw_dkit_memory_is_hash_key(lchip, reg_tbl_id, &hash_key_type))&&(!p_memory_para->direct_io))
        {
            _ctc_usw_dkit_memory_acc_write(lchip, reg_tbl_id, index_temp, hash_key_type, (void*)(value));
        }
        else
        {
            if (((DsEthMep_t == reg_tbl_id) || (DsEthRmep_t == reg_tbl_id) || (DsBfdMep_t == reg_tbl_id) || (DsBfdRmep_t == reg_tbl_id)))
            {
                sal_memset(mask, 0xFF, MAX_ENTRY_WORD * sizeof(uint32));
                *p_value = 0;
                drv_set_field(lchip, reg_tbl_id, fld_id, mask, p_value);
                field_entry.data_entry = value;
                field_entry.mask_entry = mask;
                ret = DRV_IOCTL(lchip, index, cmd, &field_entry);
            }
            else
            {
                ret = DRV_IOCTL(lchip, index, cmd, value);
            }
        }
    }

    if (ret < 0)
    {
        CTC_DKIT_PRINT("%% Write %s index %d failed %d\n",  "tbl-reg", index_temp, ret);
    }

WRITE_TABLE_ERROR:
    if (mask)
    {
        sal_free(mask);
    }
    if (value)
    {
        sal_free(value);
    }
    if (fld_name_temp)
    {
        sal_free(fld_name_temp);
    }

    return ret;
}


STATIC int32
_ctc_usw_dkit_memory_reset_table(uint8 lchip, ctc_dkit_memory_para_t* p_memory_para)
{
     return CLI_SUCCESS;
}

int32
ctc_usw_dkit_memory_is_invalid_table(uint8 lchip, uint32 tbl_id)
{
#if 0
    if ((DsL2L3EditRam12W_t == tbl_id) || (DsL2L3EditRam6W_t == tbl_id) || (DsL2L3EditRam3W_t == tbl_id)
        || (DynamicDsHashEdram12W_t == tbl_id) || (DynamicDsHashEdram6W_t == tbl_id) || (DynamicDsHashEdram3W_t == tbl_id)
        || (DynamicDsAdEdram12W_t == tbl_id) || (DynamicDsAdEdram6W_t == tbl_id) || (DynamicDsAdEdram3W_t == tbl_id))
    {
        return 1;
    }
    #endif
    return 0;
}
STATIC int32
_ctc_usw_dkit_memory_show_tbl_by_addr(uint8 lchip, ctc_dkit_memory_para_t* p_memory_para)
{
    uint32 addr = 0;
    uint32 tbl_id = 0;
    uint32 tbl_addr_max = 0;
    uint32 tbl_entry_size = 0;
    uint32 tbl_entry_num = 0;
    uint32 is_match = 0;
    uint32 tbl_index = 0;
    uint32 dynamic_mem_bitmap = 0;
    uint32 dynamic_mem_hw_data_base = 0;
    uint32 dynamic_mem_entry_num = 0;
    uint32 i = 0;
    uint32 temp = 0;
    tables_info_t* tbl_ptr = NULL;
    char    buf[CTC_DKITS_RW_BUF_SIZE];

    DKITS_PTR_VALID_CHECK(p_memory_para);
    DRV_INIT_CHECK(lchip);

    addr = p_memory_para->param[0];

    for (tbl_id = 0; tbl_id < MaxTblId_t ; tbl_id++)
    {

        if (ctc_usw_dkit_memory_is_invalid_table(lchip, tbl_id))
        {
            continue;
        }

        tbl_ptr = TABLE_INFO_PTR(lchip, tbl_id);
        if ((tbl_ptr != NULL) && (TABLE_MAX_INDEX(lchip, tbl_id) != 0))
        {

            tbl_entry_size = TABLE_ENTRY_SIZE(lchip, tbl_id);
            tbl_entry_num = TABLE_MAX_INDEX(lchip, tbl_id);

            temp = 4;
            if (tbl_entry_size > 4)
            {
                temp = 8;
            }
            if (tbl_entry_size > 8)
            {
                temp = 16;
            }
            if (tbl_entry_size > 16)
            {
                temp = 32;
            }
            if (tbl_entry_size > 32)
            {
                temp = 64;
            }
            if (tbl_entry_size > 64)
            {
                temp = 128;
            }

            tbl_entry_size = temp;

            if (DRV_TABLE_TYPE_DYNAMIC == drv_usw_get_table_type(lchip, tbl_id))
            {
                dynamic_mem_bitmap = DYNAMIC_BITMAP(lchip, tbl_id);

                for (i = 0; i < 32; i++)
                {
                    if (IS_BIT_SET(dynamic_mem_bitmap, i))
                    {
                        dynamic_mem_hw_data_base = DYNAMIC_DATA_BASE(lchip, tbl_id, i, 0);
                        dynamic_mem_entry_num = DYNAMIC_ENTRY_NUM(lchip, tbl_id, i);
                        tbl_addr_max = tbl_entry_size*dynamic_mem_entry_num;
                        tbl_index = DYNAMIC_START_INDEX(lchip, tbl_id, i);
                        if ((addr >= dynamic_mem_hw_data_base) && (addr < dynamic_mem_hw_data_base + tbl_addr_max))
                        {
                            is_match = 1;
                            tbl_index +=  (addr-dynamic_mem_hw_data_base)/tbl_entry_size;

                            break;
                        }
                    }
                }
            }
            else if (DRV_TABLE_TYPE_TCAM == drv_usw_get_table_type(lchip, tbl_id))
            {
                /*no need*/
            }
            else
            {
                if (SLICE_Cascade == tbl_ptr->slicetype)
                {
                    tbl_entry_num /= 2;
                }
                tbl_addr_max = tbl_entry_size*tbl_entry_num;

                /*for slice0 or not Cascade*/
                if ((addr >= tbl_ptr->addrs[0]) && (addr < tbl_ptr->addrs[0] + tbl_addr_max))
                {
                    is_match = 1;
                    tbl_index = (addr - tbl_ptr->addrs[0]) / tbl_entry_size;
                }

                if ((SLICE_Cascade == tbl_ptr->slicetype) && (is_match != 1))
                {
                    if ((addr >= tbl_ptr->addrs[1]) && (addr < tbl_ptr->addrs[1] + tbl_addr_max))
                    {
                        is_match = 1;
                        tbl_index = (addr - tbl_ptr->addrs[1]) / tbl_entry_size + tbl_entry_num;
                    }
                }
            }

            if (is_match)
            {
                drv_get_table_property(lchip, DRV_TABLE_PROP_GET_NAME, tbl_id, 0, (char*)buf);
                CTC_DKIT_PRINT("------------------------------------------------\n");
                CTC_DKIT_PRINT("ID:%d -> %s, index:%d\n", tbl_id, buf, tbl_index);
                CTC_DKIT_PRINT("------------------------------------------------\n");
                break;
            }
        }
    }

    if (0 == is_match)
    {
        CTC_DKIT_PRINT("%% Not found table in addr 0x%x\n", addr);
    }

    return CLI_SUCCESS;
}

STATIC int32
_ctc_usw_dkit_memory_show_tbl_by_data(uint8 lchip, ctc_dkit_memory_para_t* p_memory_para)
{
    char* reg_name = NULL;
    uint32 reg_tbl_id = 0;
    uint32* p_value = NULL;
    int32 ret = 0;

    DRV_INIT_CHECK(lchip);

    p_value =(uint32*)p_memory_para->value;
    reg_name = (char*)p_memory_para->buf;


    if (CTC_DKITS_INVALID_TBL_ID != p_memory_para->param[3])
    {
        reg_tbl_id = p_memory_para->param[3];
        CTC_DKITS_DUT2_TBL_ID_CHECK(reg_tbl_id);
        drv_get_table_property(lchip, DRV_TABLE_PROP_GET_NAME, reg_tbl_id, 0, (void*)reg_name);
    }
    else
    {
        if (DRV_E_NONE != drv_usw_get_tbl_id_by_string(lchip, &reg_tbl_id, reg_name))
        {
            CTC_DKIT_PRINT("%% Not found %s\n", reg_name);
            return CLI_ERROR;
        }
    }

    ret = _ctc_usw_dkit_memory_show_ram_by_data_tbl_id(lchip, reg_tbl_id, 0, p_value, NULL, 0);

    if (ret < 0)
    {
        CTC_DKIT_PRINT("%% show %s data failed %d\n", reg_name, ret);
    }

    return CLI_SUCCESS;
}
int32
_ctc_usw_dkit_memory_check_table_field_id(uint8 lchip, ctc_dkit_tbl_reg_wr_t* tbl_info, uint32* p_table_id, uint16* p_field_id)
{
    tbls_id_t table_id = 0;
    fld_id_t field_id = 0;
    if (DRV_E_NONE != drv_usw_get_tbl_id_by_string(lchip, &table_id, (char*)tbl_info->table_name))
    {
        CTC_DKIT_PRINT_DEBUG("invalid table name %s\n", tbl_info->table_name);
        return DKIT_E_INVALID_PARAM;
    }
    if (DRV_E_NONE != drv_usw_get_field_id_by_string(lchip, table_id, &field_id, (char*)tbl_info->field_name))
    {
        CTC_DKIT_PRINT_DEBUG("invalid field name %s\n", tbl_info->field_name);
        return DKIT_E_INVALID_PARAM;
    }
    if (NULL == drv_usw_find_field(lchip, table_id, field_id))
    {
        return DKIT_E_INVALID_PARAM;
    }
    *p_table_id = table_id;
    *p_field_id = field_id;
    return DKIT_E_NONE;
}
int32
ctc_usw_dkit_memory_read_table(uint8 lchip, void* p_param)
{
    uint32 cmd = 0;
    tbl_entry_t entry;
    uint32* data_entry = NULL;
    uint32 index_temp = 0;
    uint32 hash_key_type = CTC_DKIT_MEMORY_HASH_KEY_TPYE_MAX;
    tables_info_t* tbl_ptr = NULL;
    uint32 table_id_temp = 0;
    uint16 field_id_temp = 0;
    ctc_dkit_tbl_reg_wr_t* tbl_info = (ctc_dkit_tbl_reg_wr_t*)p_param;

    CTC_DKIT_PTR_VALID_CHECK(tbl_info);
    CTC_DKIT_PTR_VALID_CHECK(tbl_info->table_name);
    CTC_DKIT_PTR_VALID_CHECK(tbl_info->field_name);
    CTC_DKIT_LCHIP_CHECK(lchip);
    DRV_INIT_CHECK(lchip);

    CTC_DKIT_PRINT_DEBUG("%s()\n", __FUNCTION__);

    CTC_DKIT_ERROR_RETURN(_ctc_usw_dkit_memory_check_table_field_id(lchip, tbl_info, &table_id_temp, &field_id_temp));

    CTC_DKIT_PRINT_DEBUG("lchip:%d tbl_id:%d index:%d field_id:%d\n", lchip, table_id_temp,
                                           tbl_info->table_index, field_id_temp);

    tbl_ptr = TABLE_INFO_PTR(lchip, table_id_temp);
   if ((DRV_TABLE_TYPE_TCAM == drv_usw_get_table_type(lchip, table_id_temp))
        || (DRV_TABLE_TYPE_LPM_TCAM_IP == drv_usw_get_table_type(lchip, table_id_temp))
    || (DRV_TABLE_TYPE_LPM_TCAM_NAT == drv_usw_get_table_type(lchip, table_id_temp))
    || (DRV_TABLE_TYPE_STATIC_TCAM_KEY == drv_usw_get_table_type(lchip, table_id_temp)))
    {
        entry.data_entry = tbl_info->value;
        entry.mask_entry = tbl_info->mask;
        cmd = DRV_IOR(table_id_temp, field_id_temp);
        CTC_DKIT_ERROR_RETURN(DRV_IOCTL(lchip, tbl_info->table_index, cmd, &entry));
    }
    else
    {
        data_entry = tbl_info->value;
        if (_ctc_usw_dkit_memory_is_hash_key(lchip, table_id_temp, &hash_key_type))
        {
            CTC_DKIT_ERROR_RETURN(_ctc_usw_dkit_memory_acc_read(lchip, table_id_temp, tbl_info->table_index, hash_key_type, (void*)(data_entry)));
        }
        else
        {
            if ((DsIpfixSessionRecord_t == table_id_temp)||(DsFwd_t == table_id_temp))
            {
                index_temp = tbl_info->table_index/2;
                field_id_temp = field_id_temp;
                if (DsFwd_t == table_id_temp)
                {
                    if ((tbl_info->table_index) % 2)
                    {
                        field_id_temp = (field_id_temp==DRV_ENTRY_FLAG)?DRV_ENTRY_FLAG:field_id_temp+ (tbl_ptr->field_num / 2);
                    }
                }
            }
            else
            {
                index_temp = tbl_info->table_index;
                field_id_temp = field_id_temp;
            }
            cmd = DRV_IOR(table_id_temp, field_id_temp);
            CTC_DKIT_ERROR_RETURN(DRV_IOCTL(lchip, index_temp, cmd, data_entry));
        }
    }

    return DKIT_E_NONE;
}

int32
ctc_usw_dkit_memory_write_table(uint8 lchip,  void* p_param)
{
    uint32 cmd = 0;
    tbl_entry_t entry;
    uint32* p_value = NULL;
    uint32* p_mask = NULL;
    uint32 hash_key_type = CTC_DKIT_MEMORY_HASH_KEY_TPYE_MAX;
    uint32 index_temp = 0;
    tables_info_t* tbl_ptr = NULL;
    uint32 table_id_temp = 0;
    uint16 field_id_temp = 0;
    ctc_dkit_tbl_reg_wr_t* tbl_info = (ctc_dkit_tbl_reg_wr_t*)p_param;
    int32 ret = DKIT_E_NONE;

    CTC_DKIT_PTR_VALID_CHECK(tbl_info);
    CTC_DKIT_PTR_VALID_CHECK(tbl_info->table_name);
    CTC_DKIT_PTR_VALID_CHECK(tbl_info->field_name);
    CTC_DKIT_LCHIP_CHECK(lchip);
    DRV_INIT_CHECK(lchip);
    CTC_DKIT_PRINT_DEBUG("%s()\n", __FUNCTION__);


    CTC_DKIT_ERROR_RETURN(_ctc_usw_dkit_memory_check_table_field_id(lchip, tbl_info, &table_id_temp, &field_id_temp));

    CTC_DKIT_PRINT_DEBUG("lchip:%d tbl_id:%d index:%d field_id:%d\n", lchip, table_id_temp,
                                           tbl_info->table_index, field_id_temp);

    tbl_ptr = TABLE_INFO_PTR(lchip, table_id_temp);
    if ((DsIpfixSessionRecord_t == table_id_temp)||(DsFwd_t == table_id_temp))
    {
        index_temp = tbl_info->table_index/2;
        if (DsFwd_t == table_id_temp)
        {
            if ((tbl_info->table_index) % 2)
            {
                field_id_temp = (field_id_temp==DRV_ENTRY_FLAG)?DRV_ENTRY_FLAG:(field_id_temp+(tbl_ptr->field_num/2));
            }
        }
    }
    else
    {
        index_temp = tbl_info->table_index;
        field_id_temp = field_id_temp;
    }

    p_value = mem_malloc(MEM_SYSTEM_MODULE, sizeof(uint32)*CTC_MAX_TABLE_DATA_LEN);
    p_mask = mem_malloc(MEM_SYSTEM_MODULE, sizeof(uint32)*CTC_MAX_TABLE_DATA_LEN);
    if ((NULL == p_value) || (NULL == p_mask))
    {
        ret = DKIT_E_NO_MEMORY;
        goto out;
    }
    sal_memset(p_value, 0, CTC_MAX_TABLE_DATA_LEN * sizeof(uint32));
    sal_memset(p_mask, 0, CTC_MAX_TABLE_DATA_LEN * sizeof(uint32));

    cmd = DRV_IOR(table_id_temp, DRV_ENTRY_FLAG);
    if ((DRV_TABLE_TYPE_TCAM == drv_usw_get_table_type(lchip, table_id_temp))
        || (DRV_TABLE_TYPE_LPM_TCAM_IP == drv_usw_get_table_type(lchip, table_id_temp))
    || (DRV_TABLE_TYPE_LPM_TCAM_NAT == drv_usw_get_table_type(lchip, table_id_temp))
    || (DRV_TABLE_TYPE_STATIC_TCAM_KEY == drv_usw_get_table_type(lchip, table_id_temp)))
    {
        entry.data_entry = p_value;
        entry.mask_entry = p_mask;
        CTC_DKIT_ERROR_GOTO(DRV_IOCTL(lchip, index_temp, cmd, &entry), ret, out);
    }
    else
    {
        if (_ctc_usw_dkit_memory_is_hash_key(lchip, table_id_temp, &hash_key_type))
        {
            _ctc_usw_dkit_memory_acc_read(lchip, table_id_temp, index_temp, hash_key_type, (void*)(p_value));
        }
        else
        {
            CTC_DKIT_ERROR_GOTO(DRV_IOCTL(lchip, index_temp, cmd, p_value), ret, out);
        }
    }

    cmd = DRV_IOW(table_id_temp, DRV_ENTRY_FLAG);
    if ((DRV_TABLE_TYPE_TCAM == drv_usw_get_table_type(lchip, table_id_temp))
        || (DRV_TABLE_TYPE_LPM_TCAM_IP == drv_usw_get_table_type(lchip, table_id_temp))
    || (DRV_TABLE_TYPE_LPM_TCAM_NAT == drv_usw_get_table_type(lchip, table_id_temp))
    || (DRV_TABLE_TYPE_STATIC_TCAM_KEY == drv_usw_get_table_type(lchip, table_id_temp)))
    {
        if (DRV_ENTRY_FLAG != field_id_temp)
        {
            drv_set_field(lchip, table_id_temp, field_id_temp, entry.data_entry, tbl_info->value);
            drv_set_field(lchip, table_id_temp, field_id_temp, entry.mask_entry, tbl_info->mask);
        }
        else
        {
            entry.data_entry = tbl_info->value;
            entry.mask_entry = tbl_info->mask;
        }
        CTC_DKIT_ERROR_GOTO(DRV_IOCTL(lchip, index_temp, cmd, &entry), ret, out);
    }
    else
    {
        if (DRV_ENTRY_FLAG != field_id_temp)
        {
            drv_set_field(lchip, table_id_temp, field_id_temp, p_value, tbl_info->value);
        }
        else
        {
            sal_memcpy(p_value, tbl_info->value, sizeof(CTC_MAX_TABLE_DATA_LEN*sizeof(uint32)));
        }
        if (_ctc_usw_dkit_memory_is_hash_key(lchip, table_id_temp, &hash_key_type))
        {
            _ctc_usw_dkit_memory_acc_write(lchip, table_id_temp, index_temp, hash_key_type, (void*)(p_value));
        }
        else
        {
            if (((DsEthMep_t == table_id_temp) || (DsEthRmep_t == table_id_temp) || (DsBfdMep_t == table_id_temp) || (DsBfdRmep_t == table_id_temp)))
            {
                if (DRV_ENTRY_FLAG != field_id_temp)
                {
                    sal_memset(p_mask, 0xFF, CTC_MAX_TABLE_DATA_LEN * sizeof(uint32));
                    *tbl_info->value = 0;
                    drv_set_field(lchip, table_id_temp, field_id_temp, p_mask, tbl_info->value);
                }
                else
                {
                    sal_memset(p_mask, 0x0, CTC_MAX_TABLE_DATA_LEN * sizeof(uint32));
                }
                entry.data_entry = p_value;
                entry.mask_entry = p_mask;
                CTC_DKIT_ERROR_GOTO(DRV_IOCTL(lchip, index_temp, cmd, &entry), ret, out);
            }
            else
            {
                CTC_DKIT_ERROR_GOTO(DRV_IOCTL(lchip, index_temp, cmd, p_value), ret, out);
            }
        }
    }

out:
    if (p_value)
    {
        mem_free(p_value);
    }
    if (p_mask)

{
        mem_free(p_mask);
    }

    return ret;
}

int32
_ctc_usw_dkit_memory_check_table(uint8 lchip, ctc_dkit_memory_para_t* p_memory_para)
{
    char* tbl_name = NULL;
    int32 start_index = 0;
    uint32 count = 0;
    uint8 step = 0;
    uint32 tbl_id = 0;
    int32 ret = CLI_SUCCESS;
    uint32 cmd = 0;
    tbl_entry_t entry;
    uint32* data_entry = NULL;
    uint32* mask_entry = NULL;
    int32 i = 0;
    uint32 index = 0;
    uint32 index_temp = 0;
    uint32 hash_key_type = CTC_DKIT_MEMORY_HASH_KEY_TPYE_MAX;
    char* regex = NULL;
    uint32 is_half = 0;

    DKITS_PTR_VALID_CHECK(p_memory_para);
    DRV_INIT_CHECK(lchip);

    data_entry = (uint32*)sal_malloc(MAX_ENTRY_WORD * sizeof(uint32));
    if(NULL == data_entry)
    {
        return CLI_ERROR;
    }

    mask_entry = (uint32*)sal_malloc(MAX_ENTRY_WORD * sizeof(uint32));
    if(NULL == mask_entry)
    {
        sal_free(data_entry);
        return CLI_ERROR;
    }

    sal_memset(data_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));
    sal_memset(mask_entry, 0, MAX_ENTRY_WORD * sizeof(uint32));

    lchip = p_memory_para->param[1];
    start_index = p_memory_para->param[2];
    count = p_memory_para->param[4];
    step = p_memory_para->param[5];
    tbl_name = (char*)p_memory_para->buf;

    if (CTC_DKITS_INVALID_TBL_ID != p_memory_para->param[3])
    {
        tbl_id = p_memory_para->param[3];
        if((tbl_id) <= 0 || (tbl_id) >= MaxTblId_t)
        {
            ret = CLI_ERROR;
            goto clean;
        }
        drv_get_table_property(lchip, DRV_TABLE_PROP_GET_NAME, tbl_id, 0, (char*)tbl_name);
        CTC_DKIT_PRINT_DEBUG("read %s %d count %d step %d\n", tbl_name, start_index, count, step);
    }
    else
    {
        if (drv_usw_get_tbl_id_by_string(lchip, &tbl_id, tbl_name)
            || (0 == TABLE_MAX_INDEX(lchip, tbl_id)))
        {
            CTC_DKIT_PRINT("%% Not found %s\n", tbl_name);
            ret = CLI_ERROR;
            goto clean;
        }
    }
    if (dkits_log_file)
    {
        CTC_DKIT_PRINT("\n%-15s %-15s\n", "Tbl-name:", tbl_name);
    }

    if ((DRV_TABLE_TYPE_TCAM == drv_usw_get_table_type(lchip, tbl_id))
        || (DRV_TABLE_TYPE_LPM_TCAM_IP == drv_usw_get_table_type(lchip, tbl_id))
    || (DRV_TABLE_TYPE_LPM_TCAM_NAT == drv_usw_get_table_type(lchip, tbl_id))
    || (DRV_TABLE_TYPE_STATIC_TCAM_KEY == drv_usw_get_table_type(lchip, tbl_id)))
    {
        if (dkits_log_file)
        {
            CTC_DKIT_PRINT("----------------------------------------------------------------------------------");
        }
        /* TCAM */
        CTC_DKIT_PRINT("\n%-6s %-6s %-10s %-10s %-6s %-10s\n", "Index", "FldID", "Value", "Mask", "BitLen", "Name");
        CTC_DKIT_PRINT("----------------------------------------------------------------------------------\n");

    }
    else
    {
        if (dkits_log_file)
        {
            CTC_DKIT_PRINT("-------------------------------------------------------------");
        }
        /* non-TCAM */
        CTC_DKIT_PRINT("\n%-8s%-8s%-13s%-13s%-9s%-9s%-40s\n", "Index", "FldID", "Wr Value", "Rd Value", "BitLen", "Result", "FldName");
        CTC_DKIT_PRINT("-------------------------------------------------------------\n");
    }


    for (i = 0; i < count; i++)
    {
        index = (start_index + (i * step));
        if ((DRV_TABLE_TYPE_TCAM == drv_usw_get_table_type(lchip, tbl_id))
        || (DRV_TABLE_TYPE_LPM_TCAM_IP == drv_usw_get_table_type(lchip, tbl_id))
        || (DRV_TABLE_TYPE_LPM_TCAM_NAT == drv_usw_get_table_type(lchip, tbl_id))
        || (DRV_TABLE_TYPE_STATIC_TCAM_KEY == drv_usw_get_table_type(lchip, tbl_id)))
        {
            entry.data_entry = data_entry;
            entry.mask_entry = mask_entry;
            cmd = DRV_IOR(tbl_id, DRV_ENTRY_FLAG);
            if((DRV_TABLE_TYPE_LPM_TCAM_IP == drv_usw_get_table_type(lchip, tbl_id)) ||
                (DRV_TABLE_TYPE_LPM_TCAM_NAT == drv_usw_get_table_type(lchip, tbl_id)))
            {
                FTM_TCAM_IOCTL(lchip, index, cmd, &entry);
            }
            else
            {
                DRV_IOCTL(lchip, index, cmd, &entry);
            }
        }
        else
        {
            if ((_ctc_usw_dkit_memory_is_hash_key(lchip, tbl_id, &hash_key_type))&&(!p_memory_para->direct_io))
            {
                ret = DKIT_E_NOT_SUPPORT;
                goto clean;
                /*ret =  _ctc_usw_dkit_memory_acc_read(lchip, tbl_id, index, hash_key_type, (void*)(data_entry));*//*merge table and cam*/
            }
            else
            {
                uint32 tbl_id_tmp = tbl_id;

                if (DsFwdHalf_t == tbl_id_tmp)
                {
                    tbl_id_tmp = DsFwd_t;
                }

                if ((DsIpfixSessionRecord_t == tbl_id_tmp) || (DsFwd_t == tbl_id_tmp) )
                {
                    index_temp = index / 2;
                }
                else
                {
                    index_temp = index;
                }

                if (DsL2Edit3WOuter_t == tbl_id)
                {
                    tbl_id_tmp = DsL2Edit6WOuter_t;
                    index_temp = index/2;
                }

                if (DsL3Edit3W3rd_t == tbl_id)
                {
                    tbl_id_tmp = DsL3Edit6W3rd_t;
                    index_temp = index/2;
                }

                cmd = DRV_IOR(tbl_id_tmp, DRV_ENTRY_FLAG);
                ret = DRV_IOCTL(lchip, index_temp, cmd, data_entry);
            }
            if (CLI_SUCCESS == ret)
            {
                if (DsFwd_t == tbl_id)
                {
                    drv_get_field(lchip, DsFwd_t, DsFwd_isHalf_f, data_entry, (uint32*)&is_half);
                }
                if ((DsFwdHalf_t == tbl_id) || ((DsFwd_t == tbl_id)&&is_half))
                {
                    DsFwdHalf_m dsfwd_half;

                    if (index % 2 == 0)
                    {
                        drv_get_field(lchip, DsFwdDualHalf_t, DsFwdDualHalf_g_0_dsFwdHalf_f,   data_entry,   (uint32*)&dsfwd_half);
                    }
                    else
                    {
                        drv_get_field(lchip, DsFwdDualHalf_t, DsFwdDualHalf_g_1_dsFwdHalf_f,   data_entry,   (uint32*)&dsfwd_half);
                    }

                    sal_memcpy(data_entry, &dsfwd_half, sizeof(dsfwd_half));
                }
                _ctc_usw_dkit_memory_check_ram_by_data_tbl_id(lchip, tbl_id, index, data_entry, regex, !p_memory_para->direct_io);
            }
        }
    }
    if (ret < 0)
    {
        CTC_DKIT_PRINT("%% Check index %d failed %d\n", index, ret);
    }
clean:
    sal_free(data_entry);
    sal_free(mask_entry);

    return ret;
}

int32
ctc_usw_dkit_memory_process(void* p_para)
{
    ctc_dkit_memory_para_t* p_memory_para = (ctc_dkit_memory_para_t*)p_para;
    uint8 lchip = 0;

    DKITS_PTR_VALID_CHECK(p_para);
    DRV_INIT_CHECK(lchip);
    CTC_DKIT_LCHIP_CHECK(p_memory_para->param[1]);

    CTC_DKIT_PRINT_DEBUG("%s()\n", __FUNCTION__);
    switch (p_memory_para->type)
    {
        case CTC_DKIT_MEMORY_LIST:
            _ctc_usw_dkit_memory_list_table(lchip, p_memory_para);
            break;
        case CTC_DKIT_MEMORY_READ:
            _ctc_usw_dkit_memory_read_table(lchip, p_memory_para);
            break;
        case CTC_DKIT_MEMORY_WRITE:
            _ctc_usw_dkit_memory_write_table(lchip, p_memory_para);
            break;
        case CTC_DKIT_MEMORY_RESET:
            _ctc_usw_dkit_memory_reset_table(lchip, p_memory_para);
            break;
        case CTC_DKIT_MEMORY_SHOW_TBL_BY_ADDR:
            _ctc_usw_dkit_memory_show_tbl_by_addr(lchip, p_memory_para);
            break;

        case CTC_DKIT_MEMORY_SHOW_TBL_BY_DATA:
            _ctc_usw_dkit_memory_show_tbl_by_data(lchip, p_memory_para);
            break;
        case CTC_DKIT_MEMORY_CHECK:
            _ctc_usw_dkit_memory_check_table(lchip, p_memory_para);
            break;
        default:
            break;
    }

    return CLI_SUCCESS;
}


int32
ctc_usw_dkit_device_info(void* p_para)
{
    ctc_dkit_memory_para_t* p_memory_para = (ctc_dkit_memory_para_t*)p_para;
    uint8 lchip = 0;
    uint32 cmd = 0;
    uint32 capwap_decrypt_0_chan = 0;
    uint32 capwap_decrypt_1_chan = 0;
    uint32 capwap_decrypt_2_chan = 0;
    uint32 capwap_decrypt_3_chan = 0;
    uint32 capwap_encrypt_0_chan = 0;
    uint32 capwap_encrypt_1_chan = 0;
    uint32 capwap_encrypt_2_chan = 0;
    uint32 capwap_encrypt_3_chan = 0;
    uint32 capwap_reassem_chan = 0;
    uint32 mac_decrypt_chan = 0;
    uint32 mac_encrypt_chan = 0;
    uint32 iloop_chan = 0;
    uint32 eloop_chan = 0;
    uint32 oam_chan = 0;
    uint32 dma_0_chan = 0;
    uint32 dma_1_chan = 0;
    uint32 dma_2_chan = 0;
    uint32 dma_3_chan = 0;

    QMgrDeqChanIdCfg_m qmgr_deq_chan_id_cfg;

    DKITS_PTR_VALID_CHECK(p_para);
    CTC_DKIT_LCHIP_CHECK(p_memory_para->param[1]);

    CTC_DKIT_PRINT_DEBUG("%s()\n", __FUNCTION__);

    lchip = p_memory_para->param[1];
    DRV_INIT_CHECK(lchip);
    sal_memset(&qmgr_deq_chan_id_cfg, 0, sizeof(qmgr_deq_chan_id_cfg));
    cmd = DRV_IOR(QMgrDeqChanIdCfg_t, DRV_ENTRY_FLAG);
    DRV_IOCTL(lchip, 0, cmd, &qmgr_deq_chan_id_cfg);
    GetQMgrDeqChanIdCfg(A, cfgCapwapDecrypt0ChanId_f, &qmgr_deq_chan_id_cfg, &capwap_decrypt_0_chan);
    GetQMgrDeqChanIdCfg(A, cfgCapwapDecrypt1ChanId_f, &qmgr_deq_chan_id_cfg, &capwap_decrypt_1_chan);
    GetQMgrDeqChanIdCfg(A, cfgCapwapDecrypt2ChanId_f, &qmgr_deq_chan_id_cfg, &capwap_decrypt_2_chan);
    GetQMgrDeqChanIdCfg(A, cfgCapwapDecrypt3ChanId_f, &qmgr_deq_chan_id_cfg, &capwap_decrypt_3_chan);
    GetQMgrDeqChanIdCfg(A, cfgCapwapEncrypt0ChanId_f, &qmgr_deq_chan_id_cfg, &capwap_encrypt_0_chan);
    GetQMgrDeqChanIdCfg(A, cfgCapwapEncrypt1ChanId_f, &qmgr_deq_chan_id_cfg, &capwap_encrypt_1_chan);
    GetQMgrDeqChanIdCfg(A, cfgCapwapEncrypt2ChanId_f, &qmgr_deq_chan_id_cfg, &capwap_encrypt_2_chan);
    GetQMgrDeqChanIdCfg(A, cfgCapwapEncrypt3ChanId_f, &qmgr_deq_chan_id_cfg, &capwap_encrypt_3_chan);
    GetQMgrDeqChanIdCfg(A, cfgCapwapReassembleChanId_f, &qmgr_deq_chan_id_cfg, &capwap_reassem_chan);
    GetQMgrDeqChanIdCfg(A, cfgMacDecryptChanId_f, &qmgr_deq_chan_id_cfg, &mac_decrypt_chan);
    GetQMgrDeqChanIdCfg(A, cfgMacEncryptChanId_f, &qmgr_deq_chan_id_cfg, &mac_encrypt_chan);
    GetQMgrDeqChanIdCfg(A, cfgILoopChanId_f, &qmgr_deq_chan_id_cfg, &iloop_chan);
    GetQMgrDeqChanIdCfg(A, cfgELoopChanId_f, &qmgr_deq_chan_id_cfg, &eloop_chan);
    GetQMgrDeqChanIdCfg(A, cfgOamChanId_f, &qmgr_deq_chan_id_cfg, &oam_chan);
    GetQMgrDeqChanIdCfg(A, cfgDma0ChanId_f, &qmgr_deq_chan_id_cfg, &dma_0_chan);
    GetQMgrDeqChanIdCfg(A, cfgDma1ChanId_f, &qmgr_deq_chan_id_cfg, &dma_1_chan);
    GetQMgrDeqChanIdCfg(A, cfgDma2ChanId_f, &qmgr_deq_chan_id_cfg, &dma_2_chan);
    GetQMgrDeqChanIdCfg(A, cfgDma3ChanId_f, &qmgr_deq_chan_id_cfg, &dma_3_chan);

    CTC_DKIT_PRINT("%-20s\n", "Internal Port Status");
    CTC_DKIT_PRINT("%-20s %-8s\n", "Type", "ChanId");

    CTC_DKIT_PRINT("-----------------------------\n");
    CTC_DKIT_PRINT("%-20s %-8d\n", "OAM", oam_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "ILOOP", iloop_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "ELOOP", eloop_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "DMA_0", dma_0_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "DMA_1", dma_1_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "DMA_2", dma_2_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "DMA_3", dma_3_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "MAC_DECTYPT", mac_decrypt_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "MAC_ENCTYPT", mac_encrypt_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "CAPWAP_DECTYPT_0", capwap_decrypt_0_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "CAPWAP_DECTYPT_1", capwap_decrypt_1_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "CAPWAP_DECTYPT_2", capwap_decrypt_2_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "CAPWAP_DECTYPT_3", capwap_decrypt_3_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "CAPWAP_ENCTYPT_0", capwap_encrypt_0_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "CAPWAP_ENCTYPT_1", capwap_encrypt_1_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "CAPWAP_ENCTYPT_2", capwap_encrypt_2_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "CAPWAP_ENCTYPT_3", capwap_encrypt_3_chan);
    CTC_DKIT_PRINT("%-20s %-8d\n", "CAPWAP_REASEMBLE", capwap_reassem_chan);


    return CLI_SUCCESS;
}



