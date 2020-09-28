#include "sal_types.h"
#include "drv_api.h"
#include "usw/include/drv_chip_agent.h"

extern drv_chip_agent_mode_t g_usw_chip_agent_mode;
drv_chip_agent_t g_drv_agent;
bool g_eadp_drv_debug_en = FALSE;

drv_chip_agent_mode_t g_usw_chip_agent_mode = DRV_CHIP_AGT_MODE_NONE;



int32
drv_usw_agent_sram_tbl_write(uint8 lchip_offset, tbls_id_t tbl_id, uint32 index, uint32* data)
{
    chip_io_para_t io_para;

    sal_memset(&io_para, 0, sizeof(io_para));

    io_para.op = CHIP_IO_OP_IOCTL_SRAM_WRITE;
    io_para.lchip = lchip_offset;

    io_para.u.ioctl.tbl_id = tbl_id;
    io_para.u.ioctl.index = index;
    io_para.u.ioctl.val = data;

    return g_drv_agent.chip_agent_cb(lchip_offset, &io_para);
}

int32
drv_usw_agent_sram_tbl_read(uint8 lchip_offset, tbls_id_t tbl_id, uint32 index, uint32* data)
{
    chip_io_para_t io_para;

    sal_memset(&io_para, 0, sizeof(io_para));

    io_para.op = CHIP_IO_OP_IOCTL_SRAM_READ;
    io_para.lchip = lchip_offset;

    io_para.u.ioctl.tbl_id = tbl_id;
    io_para.u.ioctl.index = index;
    io_para.u.ioctl.val = data;

    return g_drv_agent.chip_agent_cb(lchip_offset, &io_para);
}


int32
drv_usw_agent_tcam_tbl_write(uint8 lchip_offset, tbls_id_t tbl_id, uint32 index, tbl_entry_t* entry)
{
    chip_io_para_t io_para;

    sal_memset(&io_para, 0, sizeof(io_para));

    io_para.op = CHIP_IO_OP_IOCTL_TCAM_WRITE;
    io_para.lchip = lchip_offset;

    io_para.u.ioctl.tbl_id = tbl_id;
    io_para.u.ioctl.index = index;
    io_para.u.ioctl.val = entry;

    return g_drv_agent.chip_agent_cb(lchip_offset, &io_para);
}

int32
drv_usw_agent_tcam_tbl_read(uint8 lchip_offset, tbls_id_t tbl_id, uint32 index, tbl_entry_t* entry)
{
    chip_io_para_t io_para;

    sal_memset(&io_para, 0, sizeof(io_para));

    io_para.op = CHIP_IO_OP_IOCTL_TCAM_READ;
    io_para.lchip = lchip_offset;

    io_para.u.ioctl.tbl_id = tbl_id;
    io_para.u.ioctl.index = index;
    io_para.u.ioctl.val = entry;

    return g_drv_agent.chip_agent_cb(lchip_offset, &io_para);
}

int32
drv_usw_agent_tcam_tbl_remove(uint8 lchip_offset, tbls_id_t tbl_id, uint32 index)
{
    chip_io_para_t io_para;

    sal_memset(&io_para, 0, sizeof(io_para));

    io_para.op = CHIP_IO_OP_IOCTL_TCAM_REMVOE;
    io_para.lchip = lchip_offset;

    io_para.u.ioctl.tbl_id = tbl_id;
    io_para.u.ioctl.index = index;

    return g_drv_agent.chip_agent_cb(lchip_offset, &io_para);
}

int32
drv_usw_agent_dma_dump(uint8 lchip_offset, uint16* entry_num, void *data, uint16 threshold)
{
    int32 ret = 0;
    chip_io_para_t io_para;

    sal_memset(&io_para, 0, sizeof(io_para));

    io_para.op = CHIP_IO_OP_DMA_DUMP;
    io_para.lchip = lchip_offset;

    io_para.u.dma_dump.threshold = threshold;
    io_para.u.dma_dump.entry_num = 0;
    io_para.u.dma_dump.val = data;

    ret =  g_drv_agent.chip_agent_cb(lchip_offset, &io_para);
    *entry_num = io_para.u.dma_dump.entry_num;

    return ret;
}




int32
drv_usw_agent_io_callback(chip_agent_msg_io_para_t *p_msg_io)
{
    uint8 lchip = 0;
    drv_io_callback_fun_t *drv_io_api = NULL;
    chip_io_para_t *p_io_para = NULL;
    int32 ret = 0;


     /*-lchip = p_msg_io->hdr.lchip;*/
    drv_io_api = &(p_drv_master[lchip]->drv_io_api);

    p_io_para = &p_msg_io->para;

    switch(p_msg_io->hdr.op)
    {
    case CHIP_IO_OP_IOCTL_SRAM_WRITE:
        ret = drv_io_api->drv_sram_tbl_write(lchip, p_io_para->u.ioctl.tbl_id, p_io_para->u.ioctl.index, (uint32*)(p_io_para->u.ioctl.val));
        break;

    case CHIP_IO_OP_IOCTL_SRAM_READ:
        ret = drv_io_api->drv_sram_tbl_read(lchip, p_io_para->u.ioctl.tbl_id, p_io_para->u.ioctl.index, (uint32*)(p_io_para->u.ioctl.val));
        break;

    case CHIP_IO_OP_IOCTL_TCAM_WRITE:
        ret = drv_io_api->drv_tcam_tbl_write(lchip, p_io_para->u.ioctl.tbl_id, p_io_para->u.ioctl.index, (tbl_entry_t *)(p_io_para->u.ioctl.val));
        break;

    case CHIP_IO_OP_IOCTL_TCAM_READ:
        ret = drv_io_api->drv_tcam_tbl_read(lchip, p_io_para->u.ioctl.tbl_id, p_io_para->u.ioctl.index, (tbl_entry_t *)(p_io_para->u.ioctl.val));
        break;

    case CHIP_IO_OP_IOCTL_TCAM_REMVOE:
        ret = drv_io_api->drv_tcam_tbl_remove(lchip, p_io_para->u.ioctl.tbl_id, p_io_para->u.ioctl.index);
        break;

    case CHIP_IO_OP_DMA_DUMP:
    {
        DRV_AGENT_DMA_DUMP_FUNC p_dma_dump_cb = NULL;
        void* user_data = NULL;

        g_drv_agent.get_dma_dump_cb(lchip, (void**)&p_dma_dump_cb, &user_data);
        if (p_dma_dump_cb)
        {
            ret = (*p_dma_dump_cb)(lchip, &p_io_para->u.dma_dump.entry_num, p_io_para->u.dma_dump.val, p_io_para->u.dma_dump.threshold);
        }
    }
        break;

    }

    p_msg_io->hdr.ret = ret;

    return 0;
}


int32
drv_usw_agent_register_cb(uint8 type, void* cb)
{
    switch(type)
    {
    case DRV_AGENT_CB_SET_OAM_DEFECT:
        g_drv_agent.set_oam_defect_cb = cb;
        break;

    case DRV_AGENT_CB_SET_PKT_RX:
        g_drv_agent.set_packet_rx_cb  = cb;
        break;

    case DRV_AGENT_CB_SET_PKT_TX:
        g_drv_agent.set_packet_tx_cb  = cb;
        break;

    case DRV_AGENT_CB_SET_DMA_RX:
        g_drv_agent.set_dma_cb        = cb;
        break;

    case DRV_AGENT_CB_SET_DMA_DUMP:
        g_drv_agent.set_dma_dump_cb   = cb;
        break;

    case DRV_AGENT_CB_GET_LEARN:
        g_drv_agent.get_learn_cb = cb;
        break;

    case DRV_AGENT_CB_GET_AGING:
        g_drv_agent.get_aging_cb = cb;
        break;

    case DRV_AGENT_CB_GET_IPFIX:
        g_drv_agent.get_ipfix_cb = cb;
        break;

    case DRV_AGENT_CB_GET_MONITOR:
        g_drv_agent.get_monitor_cb = cb;
        break;

    case DRV_AGENT_CB_GET_OAM_DEFECT:
        g_drv_agent.get_oam_defect_cb = cb;
        break;

    case DRV_AGENT_CB_GET_DMA_DUMP:
        g_drv_agent.get_dma_dump_cb = cb;
        break;

    case DRV_AGENT_CB_IPFIX_EXPORT:
        g_drv_agent.ipfix_export_stats = cb;
        break;

    case DRV_AGENT_CB_PKT_TX:
        g_drv_agent.dma_pkt_tx = cb;
        break;

    case DRV_AGENT_CB_PKT_RX:
        g_drv_agent.packet_rx = cb;
        break;
    }

    return 0;
}


int32
drv_usw_register_chip_agent_cb(DRV_IO_AGENT_CALLBACK cb)
{
    g_drv_agent.chip_agent_cb = cb;
    return DRV_E_NONE;
}

int32
drv_usw_agent_init(uint8 lchip)
{
    drv_io_callback_fun_t io_func =
    {
        drv_sram_tbl_read:drv_usw_agent_sram_tbl_read,
        drv_sram_tbl_write:drv_usw_agent_sram_tbl_write,
        drv_tcam_tbl_read:drv_usw_agent_tcam_tbl_read,
        drv_tcam_tbl_write:drv_usw_agent_tcam_tbl_write,
        drv_tcam_tbl_remove:drv_usw_agent_tcam_tbl_remove
     } ;

     if (DRV_CHIP_AGT_MODE_CLIENT == g_usw_chip_agent_mode)
     {
         drv_install_api(lchip, &io_func);
         g_drv_agent.set_dma_dump_cb(lchip, drv_usw_agent_dma_dump);
     }

    return DRV_E_NONE;
}




char*
drv_usw_agent_dma_op_str(uint32 op)
{
    switch (op)
    {
    case CHIP_AGENT_DMA_INFO_TYPE_LEARN:
        return "FDB Learning";

    case CHIP_AGENT_DMA_INFO_TYPE_HASHDUMP:
        return "HASH DUMP";

    case CHIP_AGENT_DMA_INFO_TYPE_IPFIX:
        return "IPFIX";

    case CHIP_AGENT_DMA_INFO_TYPE_SDC:
        return "SDC";

    case CHIP_AGENT_DMA_INFO_TYPE_MONITOR:
        return "MONITOR";

    default:
        return "invalid";
    }
}


int32
drv_usw_chip_agent_mode(void)
{
    return g_usw_chip_agent_mode;
}


#define  _CHIP_AGENT_API


#define  _CHIP_AGENT_ENCODE_DECODE
int32
drv_usw_agent_encode_init(uint8* buf, chip_agent_msg_init_para_t* para, uint32* req_len)
{
    int32 pkt_size_left = CHIP_AGT_BUF_SIZE;
    int32* size = NULL;
    uint8** pnt = NULL;
    uint8* start = NULL;
    uint8* end = NULL;

    size = &pkt_size_left;
    start = buf;
    pnt = &buf;

    ENCODE_PUTC(para->profile_type);
    ENCODE_PUTC(para->endian);
    ENCODE_PUTC(para->ver_len);
    ENCODE_PUTC(para->date_len);
    ENCODE_PUTL(para->is_asic);
    ENCODE_PUTL(para->sim_debug_action);
    ENCODE_PUTL(para->sim_packet_debug);
    ENCODE_PUTL(para->ret);

    if (para->ver_len)
    {
        ENCODE_PUT(para->ver, para->ver_len);
    }

    if (para->date_len)
    {
        ENCODE_PUT(para->date, para->date_len);
    }

    end = *pnt;
    *req_len = end - start;

    DRV_EADP_DBG_INFO(g_eadp_drv_debug_en, "encode Init,profile_type %d,LE %d,is_asic %d,debug_action %d,packet_debug %d,ret %d,ver %s,date %s,ver_len %d,date_len %d\n",
                    para->profile_type, para->endian, para->is_asic, para->sim_debug_action, para->sim_packet_debug, para->ret, para->ver, para->date, para->ver_len, para->date_len);

    return DRV_CHIP_AGT_E_NONE;
}

int32
drv_usw_agent_decode_init(uint8* buf, chip_agent_msg_init_para_t* para, uint32 len)
{
    int32 pkt_size_left = len;
    int32* size = NULL;
    uint8** pnt = NULL;

    size = &pkt_size_left;
    pnt = &buf;

    if (len == 0)
    {
        return DRV_CHIP_AGT_E_DECODE_ZERO_LEN;
    }

    /* encode IO header */
    DECODE_GETC(para->profile_type);
    DECODE_GETC(para->endian);
    DECODE_GETC(para->ver_len);
    DECODE_GETC(para->date_len);
    DECODE_GETL(para->is_asic);
    DECODE_GETL(para->sim_debug_action);
    DECODE_GETL(para->sim_packet_debug);
    DECODE_GETL(para->ret);

    if (para->ver_len)
    {
        DECODE_GET(para->ver, para->ver_len);
    }

    if (para->date_len)
    {
        DECODE_GET(para->date, para->date_len);
    }

    if (*size != 0)
    {
        return DRV_CHIP_AGT_E_MSG_LEN;
    }

    DRV_EADP_DBG_INFO(g_eadp_drv_debug_en, "decode Init,profile_type %d,LE %d,is_asic %d,debug_action %d,packet_debug %d,ret %d,ver %s,date %s,ver_len %d,date_len %d\n",
                  para->profile_type, para->endian, para->is_asic, para->sim_debug_action, para->sim_packet_debug, para->ret, para->ver, para->date, para->ver_len, para->date_len);

    return DRV_CHIP_AGT_E_NONE;
}

int32
drv_usw_agent_encode_model(uint8* buf, chip_agent_msg_model_para_t* para, uint32* req_len)
{
    int32 pkt_size_left = CHIP_AGT_BUF_SIZE;
    int32* size = NULL;
    uint8** pnt = NULL;
    uint8* start = NULL;
    uint8* end = NULL;

    size = &pkt_size_left;
    start = buf;
    pnt = &buf;

    ENCODE_PUTL(para->type);
    ENCODE_PUTL(para->param_count);
    ENCODE_PUTL(para->param[0]);
    ENCODE_PUTL(para->param[1]);
    ENCODE_PUTL(para->param[2]);
    ENCODE_PUTL(para->param[3]);
    ENCODE_PUTL(para->param[4]);
    ENCODE_PUTL(para->ret);

    end = *pnt;
    *req_len = end - start;

    DRV_EADP_DBG_INFO(g_eadp_drv_debug_en, "encode Model,type %d,count %d,param %d,%d,%d,%d,%d\n",
                  para->type, para->param_count, para->param[0], para->param[1], para->param[2], para->param[3], para->param[4]);

    return DRV_CHIP_AGT_E_NONE;
}

int32
drv_usw_agent_decode_model(uint8* buf, chip_agent_msg_model_para_t* para, uint32 len)
{
    int32 pkt_size_left = len;
    int32* size = NULL;
    uint8** pnt = NULL;

    size = &pkt_size_left;
    pnt = &buf;

    if (len == 0)
    {
        return DRV_CHIP_AGT_E_DECODE_ZERO_LEN;
    }

    DECODE_GETL(para->type);
    DECODE_GETL(para->param_count);
    DECODE_GETL(para->param[0]);
    DECODE_GETL(para->param[1]);
    DECODE_GETL(para->param[2]);
    DECODE_GETL(para->param[3]);
    DECODE_GETL(para->param[4]);
    DECODE_GETL(para->ret);

    if (*size != 0)
    {
        return DRV_CHIP_AGT_E_MSG_LEN;
    }

    DRV_EADP_DBG_INFO(g_eadp_drv_debug_en, "decode Model,type %d,count %d,param %d,%d,%d,%d,%d\n",
                  para->type, para->param_count, para->param[0], para->param[1], para->param[2], para->param[3], para->param[4]);

    return DRV_CHIP_AGT_E_NONE;
}

int32
drv_usw_agent_encode_ctrl(uint8* buf, chip_agent_msg_ctrl_para_t* para, uint32* req_len)
{
    int32 pkt_size_left = CHIP_AGT_BUF_SIZE;
    int32* size = NULL;
    uint8** pnt = NULL;
    uint8* start = NULL;
    uint8* end = NULL;

    size = &pkt_size_left;
    start = buf;
    pnt = &buf;

    ENCODE_PUTL(para->action);
    ENCODE_PUTL(para->type);
    ENCODE_PUTL(para->flag);
    ENCODE_PUTL(para->enable);
    ENCODE_PUTL(para->ret);

    end = *pnt;
    *req_len = end - start;

    DRV_EADP_DBG_INFO(g_eadp_drv_debug_en, "encode Control,action %d,type %d,flag 0x%04x,enable %d,ret %d\n",
                  para->action, para->type, para->flag, para->enable, para->ret);

    return DRV_CHIP_AGT_E_NONE;
}

int32
drv_usw_agent_decode_ctrl(uint8* buf, chip_agent_msg_ctrl_para_t* para, uint32 len)
{
    int32 pkt_size_left = len;
    int32* size = NULL;
    uint8** pnt = NULL;

    size = &pkt_size_left;
    pnt = &buf;

    if (len == 0)
    {
        return DRV_CHIP_AGT_E_DECODE_ZERO_LEN;
    }

    DECODE_GETL(para->action);
    DECODE_GETL(para->type);
    DECODE_GETL(para->flag);
    DECODE_GETL(para->enable);
    DECODE_GETL(para->ret);

    if (*size != 0)
    {
        return DRV_CHIP_AGT_E_MSG_LEN;
    }

    DRV_EADP_DBG_INFO(g_eadp_drv_debug_en, "decode Control,action %d,type %d,flag 0x%04x,enable %d,ret %d\n",
                  para->action, para->type, para->flag, para->enable, para->ret);

    return DRV_CHIP_AGT_E_NONE;
}

int32
drv_usw_agent_encode_pkt(uint8* buf, chip_agent_msg_pkt_para_t* para, uint32* req_len)
{
    int32 pkt_size_left = CHIP_AGT_BUF_SIZE;
    int32* size = NULL;
    uint8** pnt = NULL;
    uint8* start = NULL;
    uint8* end = NULL;

    size = &pkt_size_left;
    start = buf;
    pnt = &buf;

    ENCODE_PUTL(para->pkt_len);
    ENCODE_PUTL(para->pkt_mode);

    if (para->pkt_len)
    {
        ENCODE_PUT(para->pkt, para->pkt_len);
    }

    end = *pnt;
    *req_len = end - start;

    DRV_EADP_DBG_INFO(g_eadp_drv_debug_en, "encode Packet,len %d,mode %d\n", para->pkt_len, para->pkt_mode);

    return DRV_CHIP_AGT_E_NONE;
}

int32
drv_usw_agent_decode_pkt(uint8* buf, chip_agent_msg_pkt_para_t* para, uint32 len)
{
    int32 pkt_size_left = len;
    int32* size = NULL;
    uint8** pnt = NULL;

    size = &pkt_size_left;
    pnt = &buf;

    if (len == 0)
    {
        return DRV_CHIP_AGT_E_DECODE_ZERO_LEN;
    }

    DECODE_GETL(para->pkt_len);
    DECODE_GETL(para->pkt_mode);

    if (para->pkt_len)
    {
        DECODE_GET(para->pkt, para->pkt_len);
    }

    if (*size != 0)
    {
        return DRV_CHIP_AGT_E_MSG_LEN;
    }

    DRV_EADP_DBG_INFO(g_eadp_drv_debug_en, "decode Packet,len %d,mode %d\n", para->pkt_len, para->pkt_mode);

    return DRV_CHIP_AGT_E_NONE;
}

int32
drv_usw_agent_encode_rx_pkt(uint8* buf, chip_agent_msg_pkt_rx_para_t* para, uint32* req_len)
{
    int32 pkt_size_left = CHIP_AGT_BUF_SIZE;
    int32* size = NULL;
    uint8** pnt = NULL;
    uint8* start = NULL;
    uint8* end = NULL;

    size = &pkt_size_left;
    start = buf;
    pnt = &buf;

    ENCODE_PUTL(para->pkt_len);
    ENCODE_PUTL(para->pkt_mode);
    ENCODE_PUTL(para->dma_chan);
    ENCODE_PUTL(para->buf_count);
    ENCODE_PUTL(para->pkt_buf_len);

    if (para->pkt_len)
    {
        ENCODE_PUT(para->pkt, para->pkt_len);
    }

    end = *pnt;
    *req_len = end - start;

    DRV_EADP_DBG_INFO(g_eadp_drv_debug_en, "encode Packet,len %d,mode %d\n", para->pkt_len, para->pkt_mode);

    return DRV_CHIP_AGT_E_NONE;
}

int32
drv_usw_agent_decode_rx_pkt(uint8* buf, chip_agent_msg_pkt_rx_para_t* para, uint32 len)
{
    int32 pkt_size_left = len;
    int32* size = NULL;
    uint8** pnt = NULL;

    size = &pkt_size_left;
    pnt = &buf;

    if (len == 0)
    {
        return DRV_CHIP_AGT_E_DECODE_ZERO_LEN;
    }

    DECODE_GETL(para->pkt_len);
    DECODE_GETL(para->pkt_mode);
    DECODE_GETL(para->dma_chan);
    DECODE_GETL(para->buf_count);
    DECODE_GETL(para->pkt_buf_len);

    if (para->pkt_len)
    {
        DECODE_GET(para->pkt, para->pkt_len);
    }

    if (*size != 0)
    {
        return DRV_CHIP_AGT_E_MSG_LEN;
    }

    DRV_EADP_DBG_INFO(g_eadp_drv_debug_en, "decode Packet,len %d,mode %d\n", para->pkt_len, para->pkt_mode);

    return DRV_CHIP_AGT_E_NONE;
}


int32
drv_usw_agent_encode_log(uint8* buf, chip_agent_msg_log_para_t* para, uint32* req_len)
{
    int32 pkt_size_left = CHIP_AGT_BUF_SIZE;
    int32* size = NULL;
    uint8** pnt = NULL;
    uint8* start = NULL;
    uint8* end = NULL;

    size = &pkt_size_left;
    start = buf;
    pnt = &buf;

    ENCODE_PUTL(para->len);

    if (para->len)
    {
        ENCODE_PUT(para->data, para->len);
    }

    end = *pnt;
    *req_len = end - start;

    DRV_EADP_DBG_INFO(g_eadp_drv_debug_en, "encode Log,len %d\n", para->len);

    return DRV_CHIP_AGT_E_NONE;
}

int32
drv_usw_agent_decode_log(uint8* buf, chip_agent_msg_log_para_t* para, uint32 len)
{
    int32 pkt_size_left = len;
    int32* size = NULL;
    uint8** pnt = NULL;

    size = &pkt_size_left;
    pnt = &buf;

    if (len == 0)
    {
        return DRV_CHIP_AGT_E_DECODE_ZERO_LEN;
    }

    DECODE_GETL(para->len);

    if (para->len)
    {
        DECODE_GET(para->data, para->len);
    }

    if (*size != 0)
    {
        return DRV_CHIP_AGT_E_MSG_LEN;
    }

    DRV_EADP_DBG_INFO(g_eadp_drv_debug_en, "decode Log,len %d\n", para->len);

    return DRV_CHIP_AGT_E_NONE;
}


int32
drv_usw_agent_encode_io_ioctl(uint8* buf, chip_agent_msg_io_para_t* para, uint32* req_len)
{
    int32 pkt_size_left = CHIP_AGT_BUF_SIZE;
    int32* size = NULL;
    uint8** pnt = NULL;
    uint8* start = NULL;
    uint8* end = NULL;
	uint8 lchip = 0;

    size = &pkt_size_left;
    start = buf;
    pnt = &buf;

    /* encode IO header */

    switch(para->para.op)
    {
    case CHIP_IO_OP_IOCTL_SRAM_READ:
        {
            uint32 val_len = 0;

            val_len = TABLE_ENTRY_SIZE(lchip, para->para.u.ioctl.tbl_id);
            /*msgHdr(4B) +  tblId(4B) + Index(4B) + Val($(tblSize))+ Mask($(tblSize))*/
            para->hdr.len = sizeof(chip_agent_msg_io_hdr_t) + sizeof(uint32)*2 + TABLE_ENTRY_SIZE(lchip, para->para.u.ioctl.tbl_id)*2;

            ENCODE_PUTC(para->hdr.op);
            ENCODE_PUTC(para->hdr.lchip);
            ENCODE_PUTW(para->hdr.len);
            ENCODE_PUTL(para->hdr.ret);

            ENCODE_PUTL(para->para.u.ioctl.tbl_id);
            ENCODE_PUTL(para->para.u.ioctl.index);
            ENCODE_PUT(para->para.u.ioctl.val, val_len);

        }
    break;

    case CHIP_IO_OP_IOCTL_SRAM_WRITE:
        {
            uint32 val_len = 0;

            val_len = TABLE_ENTRY_SIZE(lchip, para->para.u.ioctl.tbl_id);
            /*msgHdr(4B) +  tblId(4B) + Index(4B) + Val($(tblSize))+ MaskVal($(tblSize))*/
            para->hdr.len = sizeof(chip_agent_msg_io_hdr_t) + sizeof(uint32)*2 + TABLE_ENTRY_SIZE(lchip, para->para.u.ioctl.tbl_id)*2;

            ENCODE_PUTC(para->hdr.op);
            ENCODE_PUTC(para->hdr.lchip);
            ENCODE_PUTW(para->hdr.len);
            ENCODE_PUTL(para->hdr.ret);

            ENCODE_PUTL(para->para.u.ioctl.tbl_id);
            ENCODE_PUTL(para->para.u.ioctl.index);
            ENCODE_PUT(para->para.u.ioctl.val, val_len);
        }
    break;

    case CHIP_IO_OP_IOCTL_TCAM_READ:
        {
            uint32 val_len                 = 0;
            tbl_entry_t* p_entry           = NULL;

            val_len = TABLE_ENTRY_SIZE(lchip, para->para.u.ioctl.tbl_id);
            p_entry = (tbl_entry_t*)para->para.u.ioctl.val;

            /*msgHdr(4B) +  tblId(4B) + Index(4B) + Val($(tblSize))+ MaskVal($(tblSize))*/
            para->hdr.len = sizeof(chip_agent_msg_io_hdr_t) + sizeof(uint32)*2 + TABLE_ENTRY_SIZE(lchip, para->para.u.ioctl.tbl_id)*2;

            ENCODE_PUTC(para->hdr.op);
            ENCODE_PUTC(para->hdr.lchip);
            ENCODE_PUTW(para->hdr.len);
            ENCODE_PUTL(para->hdr.ret);

            ENCODE_PUTL(para->para.u.ioctl.tbl_id);
            ENCODE_PUTL(para->para.u.ioctl.index);
            ENCODE_PUT(p_entry->data_entry, val_len);
            ENCODE_PUT(p_entry->mask_entry, val_len);

        }
    break;

    case CHIP_IO_OP_IOCTL_TCAM_WRITE:
        {
            uint32 val_len                 = 0;
            tbl_entry_t* p_entry           = NULL;

            val_len = TABLE_ENTRY_SIZE(lchip, para->para.u.ioctl.tbl_id);
            p_entry = (tbl_entry_t*)para->para.u.ioctl.val;

            /*msgHdr(4B) +  tblId(4B) + Index(4B) + Val($(tblSize))+ MaskVal($(tblSize))*/
            para->hdr.len = sizeof(chip_agent_msg_io_hdr_t) + sizeof(uint32)*2 + TABLE_ENTRY_SIZE(lchip, para->para.u.ioctl.tbl_id)*2;

            ENCODE_PUTC(para->hdr.op);
            ENCODE_PUTC(para->hdr.lchip);
            ENCODE_PUTW(para->hdr.len);
            ENCODE_PUTL(para->hdr.ret);

            ENCODE_PUTL(para->para.u.ioctl.tbl_id);
            ENCODE_PUTL(para->para.u.ioctl.index);
            ENCODE_PUT(p_entry->data_entry, val_len);
            ENCODE_PUT(p_entry->mask_entry, val_len);
        }
    break;

    case CHIP_IO_OP_IOCTL_TCAM_REMVOE:
        {
            /*uint32 val_len = 0;

            val_len = TABLE_ENTRY_SIZE(lchip, para->para.u.ioctl.tbl_id);*/

            /*msgHdr(4B) +  tblId(4B) + Index(4B) +  Val($(tblSize))+ MaskVal($(tblSize))*/
            para->hdr.len = sizeof(chip_agent_msg_io_hdr_t) + sizeof(uint32)*2 + TABLE_ENTRY_SIZE(lchip, para->para.u.ioctl.tbl_id)*2;

            ENCODE_PUTC(para->hdr.op);
            ENCODE_PUTC(para->hdr.lchip);
            ENCODE_PUTW(para->hdr.len);
            ENCODE_PUTL(para->hdr.ret);

            ENCODE_PUTL(para->para.u.ioctl.tbl_id);
            ENCODE_PUTL(para->para.u.ioctl.index);

        }
        break;


    case CHIP_IO_OP_DMA_DUMP:
        {
            uint32 val_len = 0;

            val_len= MAX_DMA_NUM*4;

            /*msgHdr(4B) +  threshold(2B)+ entryNum + MAX_DMA_NUM*4*/
            para->hdr.len = sizeof(chip_agent_msg_io_hdr_t) + sizeof(uint32) +val_len;

            ENCODE_PUTC(para->hdr.op);
            ENCODE_PUTC(para->hdr.lchip);
            ENCODE_PUTW(para->hdr.len);
            ENCODE_PUTL(para->hdr.ret);

            ENCODE_PUTW(para->para.u.dma_dump.threshold);
            ENCODE_PUTW(para->para.u.dma_dump.entry_num);
            ENCODE_PUT(para->para.u.dma_dump.val, val_len);
        }
        break;


    default:
        return DRV_CHIP_AGT_E_GEN;
    }

    end = *pnt;
    *req_len = end - start;

    return DRV_CHIP_AGT_E_NONE;
}

int32
drv_usw_agent_decode_io_ioctl(uint8* buf, chip_agent_msg_io_para_t* para, uint32 len)
{
    int32 pkt_size_left = len;
    int32* size = NULL;
    uint8** pnt = NULL;
	uint8 lchip = 0;

    size = &pkt_size_left;
    pnt = &buf;

    if (len == 0)
    {
        return DRV_CHIP_AGT_E_DECODE_ZERO_LEN;
    }


    /* decode IO header */
    DECODE_GETC(para->hdr.op);
    DECODE_GETC(para->hdr.lchip);
    DECODE_GETW(para->hdr.len);
    DECODE_GETL(para->hdr.ret);

    para->para.op = para->hdr.op;

    switch(para->hdr.op)
    {
    case CHIP_IO_OP_IOCTL_SRAM_READ:
        {
            uint32 val_len = 0;

            /*msgHdr(4B) +  tblId(4B) + Index(4B) + Val($(tblSize))+ MaskVal($(tblSize))*/
            DECODE_GETL(para->para.u.ioctl.tbl_id);
            DECODE_GETL(para->para.u.ioctl.index);

            val_len = TABLE_ENTRY_SIZE(lchip, para->para.u.ioctl.tbl_id);
            CHIP_AGT_DECODE_BUF(para->para.u.ioctl.val, val_len);
        }
    break;

    case CHIP_IO_OP_IOCTL_SRAM_WRITE:
        {
            uint32 val_len = 0;

            /*msgHdr(4B) +  tblId(4B) + Index(4B) + Val($(tblSize))+ MaskVal($(tblSize))*/
            DECODE_GETL(para->para.u.ioctl.tbl_id);
            DECODE_GETL(para->para.u.ioctl.index);

            val_len = TABLE_ENTRY_SIZE(lchip, para->para.u.ioctl.tbl_id);
            CHIP_AGT_DECODE_BUF(para->para.u.ioctl.val, val_len);
        }

    break;

    case CHIP_IO_OP_IOCTL_TCAM_READ:
        {
            uint32 val_len                 = 0;
            tbl_entry_t* p_entry           = NULL;

            p_entry = (tbl_entry_t*)para->para.u.ioctl.val;

            /*msgHdr(4B) +  tblId(4B) + Index(4B) + Val($(tblSize))+ MaskVal($(tblSize))*/
            DECODE_GETL(para->para.u.ioctl.tbl_id);
            DECODE_GETL(para->para.u.ioctl.index);

            val_len = TABLE_ENTRY_SIZE(lchip, para->para.u.ioctl.tbl_id);
            CHIP_AGT_DECODE_BUF(p_entry->data_entry, val_len);
            CHIP_AGT_DECODE_BUF(p_entry->mask_entry, val_len);

        }
    break;

    case CHIP_IO_OP_IOCTL_TCAM_WRITE:
        {
            uint32 val_len                 = 0;
            tbl_entry_t* p_entry           = NULL;

            p_entry = (tbl_entry_t*)para->para.u.ioctl.val;

            /*msgHdr(4B) +  tblId(4B) + Index(4B) + Val($(tblSize))+ MaskVal($(tblSize))*/
            DECODE_GETL(para->para.u.ioctl.tbl_id);
            DECODE_GETL(para->para.u.ioctl.index);

            val_len = TABLE_ENTRY_SIZE(lchip, para->para.u.ioctl.tbl_id);
            CHIP_AGT_DECODE_BUF(p_entry->data_entry, val_len);
            CHIP_AGT_DECODE_BUF(p_entry->mask_entry, val_len);
        }
    break;

    case CHIP_IO_OP_IOCTL_TCAM_REMVOE:
        {
            /*uint32 val_len = 0;*/

            /*msgHdr(4B) +  tblId(4B) + Index(4B) + Val($(tblSize))+ MaskVal($(tblSize))*/
            DECODE_GETL(para->para.u.ioctl.tbl_id);
            DECODE_GETL(para->para.u.ioctl.index);

            /*val_len = TABLE_ENTRY_SIZE(lchip, para->para.u.ioctl.tbl_id);*/

        }
        break;


    case CHIP_IO_OP_DMA_DUMP:
        {
            uint32 val_len = 0;

            val_len= MAX_DMA_NUM*4;

            /*msgHdr(4B) +  threshold(2B) + entryNum(2B) + MAX_DMA_NUM*4*/
            DECODE_GETW(para->para.u.dma_dump.threshold);
            DECODE_GETW(para->para.u.dma_dump.entry_num);
            CHIP_AGT_DECODE_BUF(para->para.u.dma_dump.val, val_len);
        }
        break;

    default:
        return DRV_CHIP_AGT_E_GEN;
    }


    if (*size != 0)
    {
        sal_printf("DRV_CHIP_AGT_E_MSG_LEN, size:%d\n", *size);
        return DRV_CHIP_AGT_E_MSG_LEN;
    }

    return DRV_CHIP_AGT_E_NONE;
}



int32
drv_usw_agent_decode_io_header(uint8* buf, chip_agent_msg_io_hdr_t* hdr, uint32 len)
{
    int32 pkt_size_left = len;
    int32* size = NULL;
    uint8** pnt = NULL;

    size = &pkt_size_left;
    pnt = &buf;

    if (len == 0)
    {
        return DRV_CHIP_AGT_E_DECODE_ZERO_LEN;
    }

    DECODE_GETC(hdr->op);
    DECODE_GETC(hdr->lchip);
    DECODE_GETW(hdr->len);
    return DRV_CHIP_AGT_E_NONE;
}



int32
drv_usw_agent_encode_dma(uint8* buf, chip_agent_msg_dma_para_t* para, uint32* req_len)
{
    int32 pkt_size_left = CHIP_AGT_BUF_SIZE;
    int32* size = NULL;
    uint8** pnt = NULL;
    uint8* start = NULL;
    uint8* end = NULL;

    size = &pkt_size_left;
    start = buf;
    pnt = &buf;

    /* encode DMA header */
    ENCODE_PUTC(para->op);
    ENCODE_PUTC(para->lchip);
    ENCODE_PUTW(para->num);

    ENCODE_PUT(para->data_buf, MAX_DMA_NUM*4);
    ENCODE_PUT(para->data_ext, MAX_DMA_EXT_NUM*4);


    DRV_EADP_DBG_INFO(g_eadp_drv_debug_en, "encode DMA %s,chip %d,num %d\n",
                  drv_usw_agent_dma_op_str(para->op), para->lchip, para->num);
    end = *pnt;
    *req_len = end - start;

    return DRV_CHIP_AGT_E_NONE;
}

int32
drv_usw_agent_decode_dma(uint8* buf, chip_agent_msg_dma_para_t* para, uint32 len)
{
    int32 pkt_size_left = len;
    int32* size = NULL;
    uint8** pnt = NULL;

    size = &pkt_size_left;
    pnt = &buf;

    if (len == 0)
    {
        return DRV_CHIP_AGT_E_DECODE_ZERO_LEN;
    }

    /* encode DMA header */
    DECODE_GETC(para->op);
    DECODE_GETC(para->lchip);
    DECODE_GETW(para->num);

    CHIP_AGT_DECODE_BUF(para->data_buf, MAX_DMA_NUM*4);
    CHIP_AGT_DECODE_BUF(para->data_ext, MAX_DMA_EXT_NUM*4);

    DRV_EADP_DBG_INFO(g_eadp_drv_debug_en, "decode DMA %s,chip %d,num %d\n",
                 drv_usw_agent_dma_op_str(para->op), para->lchip, para->num);

    if (*size != 0)
    {
        return DRV_CHIP_AGT_E_MSG_LEN;
    }

    return DRV_CHIP_AGT_E_NONE;
}



