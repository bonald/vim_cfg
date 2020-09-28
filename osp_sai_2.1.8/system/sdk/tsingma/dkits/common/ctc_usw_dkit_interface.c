#include "drv_api.h"
#include "usw/include/drv_common.h"
#include "ctc_cli.h"
#include "ctc_dkit.h"
#include "ctc_usw_dkit.h"
#include "ctc_usw_dkit_interface.h"

extern int32
_ctc_usw_dkit_memory_read_table(uint8 lchip, ctc_dkit_memory_para_t* p_memory_para);

uint32 usw_xqmac_tbl_offset[32] =
{
      0,   1,   2,   3, /* XQMac0~3 */
      4,   5,   6,   7, /* XQMac0~3 */
      8,   9,   0,   0, /* XQMac0~1 */
    10, 11,   0,   0, /* XQMac0~1 */
    12, 13, 14, 15, /* XQMac0~3 */
    16, 17, 18, 19, /* XQMac0~3 */
    20, 21,   0,   0, /* XQMac0~1 */
    22, 23,   0,   0   /* XQMac0~1 */
};

int32
ctc_usw_dkit_interface_print_pcs_table(uint8 lchip, uint32 tbl_id, uint32 index)
{

    char tbl_name[32] = {0};
    ctc_dkit_memory_para_t* ctc_dkit_memory_para = NULL;

    ctc_dkit_memory_para = (ctc_dkit_memory_para_t*)sal_malloc(sizeof(ctc_dkit_memory_para_t));
    if(NULL == ctc_dkit_memory_para)
    {
        return CLI_ERROR;
    }
    sal_memset(ctc_dkit_memory_para, 0 , sizeof(ctc_dkit_memory_para_t));

    ctc_dkit_memory_para->param[1] = lchip;
    ctc_dkit_memory_para->param[2] = index;
    ctc_dkit_memory_para->param[3] = tbl_id;
    ctc_dkit_memory_para->param[4] = 1;

    drv_usw_get_tbl_string_by_id(lchip, tbl_id , tbl_name);
    CTC_DKIT_PRINT("Table name:%s\n", tbl_name);
    CTC_DKIT_PRINT("-------------------------------------------------------------\n");

    _ctc_usw_dkit_memory_read_table(lchip, ctc_dkit_memory_para);

    if(NULL != ctc_dkit_memory_para)
    {
        sal_free(ctc_dkit_memory_para);
    }
    return CLI_SUCCESS;
}


