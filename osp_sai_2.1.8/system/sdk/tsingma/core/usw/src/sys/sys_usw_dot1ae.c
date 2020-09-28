#if (FEATURE_MODE == 0)
/**
 @file sys_usw_dot1ae.c

 @date 2010-2-26

 @version v2.0

---file comments----
*/

/***************************************************************
 *
 * Header Files
 *
 ***************************************************************/
#include "sal.h"
#include "ctc_error.h"
#include "ctc_hash.h"
#include "ctc_port.h"
#include "ctc_vector.h"
#include "ctc_linklist.h"
#include "ctc_spool.h"

#include "sys_usw_opf.h"
#include "sys_usw_dot1ae.h"
#include "sys_usw_port.h"
#include "sys_usw_vlan.h"
#include "sys_usw_scl_api.h"
#include "sys_usw_chip.h"
#include "sys_usw_common.h"
#include "sys_usw_ftm.h"
#include "sys_usw_internal_port.h"
#include "sys_usw_qos_api.h"
#include "sys_usw_internal_port.h"
#include "sys_usw_nexthop_api.h"

#include "drv_api.h"
#include "sys_usw_wb_common.h"
#include "sys_usw_register.h"
#include "sys_usw_dmps.h"
#include "sys_usw_dma.h"
#include "sys_usw_nexthop.h"

/***************************************************************
 *
 *  Defines and Macros
 *
 ***************************************************************/


#define SYS_DOT1AE_TX_CHAN_NUM  32
#define SYS_DOT1AE_RX_CHAN_NUM  32

#define SYS_DOT1AE_SEC_CHAN_NUM      (SYS_DOT1AE_TX_CHAN_NUM + SYS_DOT1AE_RX_CHAN_NUM)



/****************************************************************************
 *
* Global and Declaration
*
*****************************************************************************/
struct sys_dot1ae_chan_bind_node_s
{
    ctc_slistnode_t head;
    uint32 value;
};
typedef struct sys_dot1ae_chan_bind_node_s sys_dot1ae_chan_bind_node_t;

struct sys_dot1ae_chan_s
{
    uint32 chan_id;
    uint32  dir:2,
            sc_index:5, /*0~32*/
            rsv1:1,
            valid:1,
            include_sci:1,
            an_en:4,/*rx inuse*/
            next_an:4,/**/
            an_valid:4,/*valid an cfg*/
            binding_type:2, /*0:port 1:nexthop 2:logicport*/
            rsv:8;
    ctc_slist_t* bind_mem_list;
    uint8  *key;   /*only for duet2*/
};
typedef struct sys_dot1ae_chan_s sys_dot1ae_chan_t;

struct sys_dot1ae_global_stats_s
{
    uint64 in_pkts_no_sci;                 /**< [TM] Dot1AE global stats*/
    uint64 in_pkts_unknown_sci;            /**< [TM] Dot1AE global stats*/
};
typedef struct sys_dot1ae_global_stats_s sys_dot1ae_global_stats_t;

struct sys_dot1ae_master_s
{
    sys_dot1ae_chan_t dot1ae_sec_chan[SYS_DOT1AE_SEC_CHAN_NUM];
    uint8  dot1ae_opf_type;
    uint8  rsv[3];
    sal_mutex_t* sec_mutex;
    ctc_dot1ae_an_stats_t an_stats[SYS_DOT1AE_SEC_CHAN_NUM][4];
    sys_dot1ae_global_stats_t global_stats;
};
typedef struct sys_dot1ae_master_s sys_dot1ae_master_t;

sys_dot1ae_master_t* usw_dot1ae_master[CTC_MAX_LOCAL_CHIP_NUM] = {NULL};

#define SYS_DOT1AE_INIT_CHECK(lchip) \
    do \
    { \
        SYS_LCHIP_CHECK_ACTIVE(lchip); \
        if (usw_dot1ae_master[lchip] == NULL){ \
            SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " Feature not initialized \n");\
            return CTC_E_NOT_INIT;} \
    } while (0)


#define SYS_DOT1AE_CREATE_LOCK(lchip)                   \
    do                                            \
    {                                             \
        sal_mutex_create(&usw_dot1ae_master[lchip]->sec_mutex); \
        if (NULL == usw_dot1ae_master[lchip]->sec_mutex)  \
        { \
            CTC_ERROR_RETURN(CTC_E_NO_MEMORY); \
        } \
    } while (0)

#define SYS_DOT1AE_LOCK(lchip) \
    if (usw_dot1ae_master[lchip]->sec_mutex) sal_mutex_lock(usw_dot1ae_master[lchip]->sec_mutex)

#define SYS_DOT1AE_UNLOCK(lchip) \
    if (usw_dot1ae_master[lchip]->sec_mutex) sal_mutex_unlock(usw_dot1ae_master[lchip]->sec_mutex)

#define SYS_DOT1AE_REVERSE(dest, src, len)      \
{                                            \
    int32 i = 0;                             \
    for (i=0;i<(len);i++)                    \
    {                                        \
        (dest)[(len)-1-i] = (src)[i];          \
    }                                        \
}
#define SYS_DOT1AE_32var_TO_64var(dest, high32, low32)    \
{\
    sal_memcpy(&(dest), &(high32), sizeof(uint32));\
    sal_memcpy((uint8 *)&(dest) + sizeof(uint32), &(low32), sizeof(uint32));\
}



/***************************************************************
 *
 *  Functions
 *
 ***************************************************************/
#if 0
STATIC int32
_sys_usw_dot1ae_map_drv_window_size(uint8 lchip, uint32 window_size, uint8* o_drv_size, uint8* o_drv_shift)
{
    int8   idx1      = 0;
    uint16 drv_size  = 0;
    uint8  drv_shift = 0;

    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    for(idx1 = 31; idx1 > 0; idx1--)
    {
        if(CTC_IS_BIT_SET(window_size, idx1)) /*get index of highest bit*/
        {
            break;
        }
    }
    if(idx1 <= 7)  /*window_size < 256*/
    {
        idx1 = 7;
    }
    drv_size  = (window_size >> (idx1 - 7)) & 0xFF;
    drv_shift = idx1 - 7;
    if(drv_shift > 0)
    {
        if(CTC_IS_BIT_SET(window_size, idx1 - 8))
        {
            drv_size += 1;
            if(drv_size == 0x100)
            {
                drv_size = drv_size >> 1;
                drv_shift += 1;
            }
        }
    }
    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "window-size: %u, drv_size: %u, drv_shift: %u\n", window_size, drv_size, drv_shift);

    if(o_drv_size)
    {
        *o_drv_size = drv_size;
    }
    if(o_drv_shift)
    {
        *o_drv_shift = drv_shift;
    }

    return CTC_E_NONE;
}
#endif
STATIC sys_dot1ae_chan_t *
_sys_usw_dot1ae_get_chan_by_id(uint8 lchip, uint32 chan_id)
{
    uint8 idx = 0;
    sys_dot1ae_chan_t* p_sys_chan = NULL;

    for(idx = 0; idx < SYS_DOT1AE_SEC_CHAN_NUM; idx++)
    {
        p_sys_chan = &usw_dot1ae_master[lchip]->dot1ae_sec_chan[idx];
        if(p_sys_chan->valid && (chan_id == p_sys_chan->chan_id))
        {
          return p_sys_chan;
        }
    }
   return NULL;
}
int32
sys_usw_dot1ae_get_bind_sec_chan(uint8 lchip, sys_usw_register_dot1ae_bind_sc_t* p_bind_sc)
{
    uint8 idx = 0;
    uint8 end_idx = 0;
    uint8 found = 0;
    sys_dot1ae_chan_t* p_sys_chan = NULL;
    sys_dot1ae_chan_bind_node_t* p_sec_chan_bind_node = NULL;
    ctc_slistnode_t        *node = NULL;
    SYS_DOT1AE_INIT_CHECK(lchip);

    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "get bind_sec_chan,sc_index:%d: gport:0x%x dir:%d\n", p_bind_sc->sc_index, p_bind_sc->gport,p_bind_sc->dir);

    p_bind_sc->chan_id = 0;
    idx = (p_bind_sc->dir ==CTC_DOT1AE_SC_DIR_RX)?MCHIP_CAP(SYS_CAP_DOT1AE_TX_CHAN_NUM):0;
    end_idx = (p_bind_sc->dir ==CTC_DOT1AE_SC_DIR_RX)?SYS_DOT1AE_SEC_CHAN_NUM:MCHIP_CAP(SYS_CAP_DOT1AE_TX_CHAN_NUM);
    for( ; idx < end_idx; idx++)
    {
        p_sys_chan = &usw_dot1ae_master[lchip]->dot1ae_sec_chan[idx];
        if(p_sys_chan->valid
                && (p_bind_sc->sc_index== p_sys_chan->sc_index)
                && (0 != CTC_SLISTCOUNT(p_sys_chan->bind_mem_list)))
        {
            CTC_SLIST_LOOP(p_sys_chan->bind_mem_list, node)
            {
                p_sec_chan_bind_node = _ctc_container_of(node, sys_dot1ae_chan_bind_node_t, head);
                if (p_sec_chan_bind_node->value == p_bind_sc->gport)
                {
                    found = 1;
                    break;
                }
            }
            if (1 == found)
            {
                break;
            }
        }
    }
    if(found)
    {
        p_bind_sc->chan_id = p_sys_chan->chan_id;
    }
    return CTC_E_NONE;
}
int32
sys_usw_dot1ae_bind_sec_chan(uint8 lchip, sys_usw_register_dot1ae_bind_sc_t* p_bind_sc)
{
    uint8 idx = 0;
    uint8 max_channel = 0;
    uint8 found = 0;
    uint16 array_index = 0;
    sys_dot1ae_chan_t* p_sys_chan = NULL;
    sys_dot1ae_chan_bind_node_t* p_sec_chan_bind_node = NULL;
    ctc_slistnode_t        *node = NULL;

    SYS_DOT1AE_INIT_CHECK(lchip);

    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "bind_sec_chan,chan_id:%d: gport:0x%x dir:%d\n", p_bind_sc->chan_id, p_bind_sc->gport,p_bind_sc->dir);

    idx = (p_bind_sc->dir ==CTC_DOT1AE_SC_DIR_RX)?MCHIP_CAP(SYS_CAP_DOT1AE_TX_CHAN_NUM):0;
    max_channel = (p_bind_sc->dir ==CTC_DOT1AE_SC_DIR_RX)?SYS_DOT1AE_SEC_CHAN_NUM : MCHIP_CAP(SYS_CAP_DOT1AE_TX_CHAN_NUM);
    for(; idx < max_channel ; idx++)
    {
        p_sys_chan = &usw_dot1ae_master[lchip]->dot1ae_sec_chan[idx];

        if(p_sys_chan->valid
                && (p_bind_sc->chan_id ==  p_sys_chan->chan_id))
        {
            if ((p_sys_chan->binding_type != p_bind_sc->type) && (0 != CTC_SLISTCOUNT(p_sys_chan->bind_mem_list)))
            {
                return CTC_E_IN_USE;
            }
            CTC_SLIST_LOOP(p_sys_chan->bind_mem_list, node)
            {
                p_sec_chan_bind_node = _ctc_container_of(node, sys_dot1ae_chan_bind_node_t, head);
                if (p_sec_chan_bind_node->value == p_bind_sc->gport)
                {
                    return CTC_E_EXIST;
                }
            }
            found = 1;
            array_index = idx;
        }
    }
    if(!found)
    {
        return CTC_E_NOT_READY;
    }
    p_sys_chan = &usw_dot1ae_master[lchip]->dot1ae_sec_chan[array_index];
    if(!DRV_IS_DUET2(lchip) && p_bind_sc->dir == CTC_DOT1AE_SC_DIR_TX && p_bind_sc->type == 1)
    {
        uint32  aes_key_index = 0;
        uint32  cipher_mode = 0;
        uint32 cmd = 0;
        for (idx = 0; idx < MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM); idx++)
        {
            if (CTC_IS_BIT_SET(p_sys_chan->an_valid, idx))
            {
                aes_key_index = (p_sys_chan->sc_index << 2) + (idx & 0x3);
                cmd = DRV_IOR(DsDot1AeAesKey_t, DsDot1AeAesKey_cipherMode_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, aes_key_index, cmd, &cipher_mode));
                if (cipher_mode == 1 || cipher_mode == 2)
                {
                    return CTC_E_INVALID_CONFIG;
                }
            }
        }
    }
    p_sys_chan->binding_type = p_bind_sc->type;
    p_sec_chan_bind_node = mem_malloc(MEM_DOT1AE_MODULE, sizeof(sys_dot1ae_chan_bind_node_t));
    if (NULL == p_sec_chan_bind_node)
    {
        return CTC_E_NO_MEMORY;
    }
    sal_memset(p_sec_chan_bind_node, 0, sizeof(sys_dot1ae_chan_bind_node_t));
    p_sec_chan_bind_node->value = p_bind_sc->gport;
    ctc_slist_add_head(p_sys_chan->bind_mem_list, &(p_sec_chan_bind_node->head));
    p_bind_sc->sc_index = p_sys_chan->sc_index;
    p_bind_sc->include_sci = p_sys_chan->include_sci;
    return CTC_E_NONE;
}
int32
sys_usw_dot1ae_unbind_sec_chan(uint8 lchip, sys_usw_register_dot1ae_bind_sc_t* p_bind_sc)
{
    uint8 idx = 0;
    uint8 found = 0;
    sys_dot1ae_chan_t* p_sys_chan = NULL;
    sys_dot1ae_chan_bind_node_t* p_sec_chan_bind_node = NULL;
    ctc_slistnode_t        *node = NULL;

    SYS_DOT1AE_INIT_CHECK(lchip);
    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "unbind_sec_chan,sc_index:%d: gport:0x%x dir:%d\n", p_bind_sc->sc_index, p_bind_sc->gport,p_bind_sc->dir);

    idx = (p_bind_sc->dir ==CTC_DOT1AE_SC_DIR_RX)?MCHIP_CAP(SYS_CAP_DOT1AE_TX_CHAN_NUM):0;
    for( ;idx < SYS_DOT1AE_SEC_CHAN_NUM; idx++)
    {
        p_sys_chan = &usw_dot1ae_master[lchip]->dot1ae_sec_chan[idx];
        if(p_sys_chan->valid
                && (p_bind_sc->sc_index== p_sys_chan->sc_index)
                && (p_sys_chan->binding_type == p_bind_sc->type))
        {
            CTC_SLIST_LOOP(p_sys_chan->bind_mem_list, node)
            {
                p_sec_chan_bind_node = _ctc_container_of(node, sys_dot1ae_chan_bind_node_t, head);
                if (p_sec_chan_bind_node->value == p_bind_sc->gport)
                {
                    found = 1;
                    break;
                }
            }
            if (1 == found)
            {
                break;
            }
        }
    }
    if(found)
    {
        ctc_slist_delete_node(p_sys_chan->bind_mem_list, &(p_sec_chan_bind_node->head));
        mem_free(p_sec_chan_bind_node);
    }

    return CTC_E_NONE;
}

int32
_sys_usw_dot1ae_reset_sec_chan_cfg(uint8 lchip, sys_dot1ae_chan_t* p_sc_info)
{
    uint32 cmd = 0;
    uint64 value = 0;
    int32 ret = 0;
    uint8  key_index   = 0,aes_key_index = 0;
    uint8 loop = 0;
    uint8 step = 0;
    uint32 ebit_cbit_all[4] = {0};
    uint8 sci_hw[8] = {0};
    uint32 sc_array_idx = 0;
    DsDot1AePnCheck_m rx_pn_cfg;
    DsEgressDot1AePn_m tx_pn_cfg;
    DsDot1AeAesKey_m aes_key;
    DsDot1AeDecryptConfig_m Decrypt_cfg;
    sal_memset(&aes_key,0,sizeof(aes_key));
    sal_memset(&rx_pn_cfg,0,sizeof(rx_pn_cfg));
    sal_memset(&tx_pn_cfg,0,sizeof(tx_pn_cfg));
    sal_memset(&Decrypt_cfg,0,sizeof(Decrypt_cfg));

    for(loop = 0;loop < MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM);loop++)
    {
        key_index = (p_sc_info->sc_index << 2) + (loop & 0x3);
        /* tx key_index: 0-127;
        rx key_index: 128 - 255 */
        aes_key_index = key_index + ((p_sc_info->dir == CTC_DOT1AE_SC_DIR_TX)?0:128);

        /*reset aes key*/
        cmd = DRV_IOW(DsDot1AeAesKey_t, DRV_ENTRY_FLAG);
        DRV_IOCTL(lchip, aes_key_index, cmd, &aes_key);

        /*next pn/windown size*/
        if (p_sc_info->dir == CTC_DOT1AE_SC_DIR_TX)
        {
            cmd = DRV_IOW(DsEgressDot1AePn_t, DRV_ENTRY_FLAG);
            value = 1;
            SetDsEgressDot1AePn(A, pn_f, &tx_pn_cfg, &value);
            DRV_IOCTL(lchip, key_index, cmd, &tx_pn_cfg);

            /*an/next an */
            value = 0;
            step = EpeDot1AeAnCtl1_array_1_nextAn_f - EpeDot1AeAnCtl1_array_0_nextAn_f;
            cmd = DRV_IOW(EpeDot1AeAnCtl1_t, EpeDot1AeAnCtl1_array_0_nextAn_f + step * key_index);
            DRV_IOCTL(lchip, 0, cmd, &value);

            step = EpeDot1AeAnCtl0_array_1_currentAn_f - EpeDot1AeAnCtl0_array_0_currentAn_f;
            cmd = DRV_IOW(EpeDot1AeAnCtl0_t, EpeDot1AeAnCtl0_array_0_currentAn_f + step * key_index);
            DRV_IOCTL(lchip, 0, cmd, &value);

            /* e&c bit*/
            cmd = DRV_IOR(EpePktProcCtl_t, EpePktProcCtl_dot1AeTciEbitCbit_f);
            DRV_IOCTL(lchip, 0, cmd, &ebit_cbit_all);
            CTC_BIT_UNSET(ebit_cbit_all[key_index / 32], key_index % 32);
            cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_dot1AeTciEbitCbit_f);
            DRV_IOCTL(lchip, 0, cmd, &ebit_cbit_all);

            /*aepnMode*/
            cmd = DRV_IOR(EpePktProcCtl_t, EpePktProcCtl_dot1AePnMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ebit_cbit_all));
            CTC_BIT_UNSET(ebit_cbit_all[key_index/32], (key_index % 32));
            cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_dot1AePnMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, ebit_cbit_all));

            cmd = DRV_IOW(DsEgressDot1AeSci_t, DsEgressDot1AeSci_sci_f);
            DRV_IOCTL(lchip, key_index, cmd, &sci_hw);
        }
        else
        {
            cmd = DRV_IOW(DsDot1AePnCheck_t, DRV_ENTRY_FLAG);
            value = 1;
            SetDsDot1AePnCheck(A, nextPn_f, &rx_pn_cfg, &value);
            DRV_IOCTL(lchip, key_index, cmd, &rx_pn_cfg);

            cmd = DRV_IOW(DsDot1AeSci_t, DsDot1AeSci_sci_f);
            DRV_IOCTL(lchip, key_index, cmd, &sci_hw);

            cmd = DRV_IOW(DsDot1AeDecryptConfig_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, key_index, cmd, &Decrypt_cfg);
        }

    }

    sc_array_idx = p_sc_info->sc_index  + ((p_sc_info->dir ==CTC_DOT1AE_SC_DIR_RX)?MCHIP_CAP(SYS_CAP_DOT1AE_TX_CHAN_NUM):0);
    sal_memset(usw_dot1ae_master[lchip]->an_stats[sc_array_idx], 0 , sizeof(ctc_dot1ae_an_stats_t)*4);

    return ret;
}

int32
sys_usw_dot1ae_add_sec_chan(uint8 lchip, ctc_dot1ae_sc_t* p_sc)
{
    int32  ret = CTC_E_NONE;
    sys_usw_opf_t opf;
    uint32   sc_array_idx = 0;
    uint32  offset=0;
    sys_dot1ae_chan_t* p_sys_sec_chan = NULL;
    uint32 key_len = CTC_DOT1AE_KEY_LEN;


    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_DOT1AE_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_sc);
    CTC_MAX_VALUE_CHECK(p_sc->dir, CTC_DOT1AE_SC_DIR_RX);
    CTC_MIN_VALUE_CHECK(p_sc->chan_id,1);
    SYS_DOT1AE_LOCK(lchip);
    p_sys_sec_chan = _sys_usw_dot1ae_get_chan_by_id(lchip,  p_sc->chan_id);
    if(p_sys_sec_chan)
    {
       SYS_DOT1AE_UNLOCK(lchip);
       return CTC_E_EXIST;
    }

    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = usw_dot1ae_master[lchip]->dot1ae_opf_type;
    opf.pool_index = (p_sc->dir ==CTC_DOT1AE_SC_DIR_TX) ?0 :1;
    CTC_ERROR_GOTO(sys_usw_opf_alloc_offset(lchip, &opf, 1, &offset),ret,error);
    /*array idx:tx:0~31:rx:32~63*/
    sc_array_idx = offset  + ((p_sc->dir ==CTC_DOT1AE_SC_DIR_RX)?MCHIP_CAP(SYS_CAP_DOT1AE_TX_CHAN_NUM):0);

    p_sys_sec_chan = &usw_dot1ae_master[lchip]->dot1ae_sec_chan[sc_array_idx];
    p_sys_sec_chan->sc_index = offset & 0x1F ;
    p_sys_sec_chan->chan_id = p_sc->chan_id;
    p_sys_sec_chan->dir = p_sc->dir;
    p_sys_sec_chan->valid = 1;
    p_sys_sec_chan->bind_mem_list = ctc_slist_new();

    if(DRV_IS_DUET2(lchip))
    {
        key_len = CTC_DOT1AE_KEY_LEN/2;
    }
    if(p_sc->dir == CTC_DOT1AE_SC_DIR_RX)
    {

        p_sys_sec_chan->key = mem_malloc(MEM_DOT1AE_MODULE, key_len * MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM) * sizeof(uint8));
        if(!p_sys_sec_chan->key)
        {
            ctc_slist_free(p_sys_sec_chan->bind_mem_list);
            sys_usw_opf_free_offset(lchip, &opf, 1, offset);
            SYS_DOT1AE_UNLOCK(lchip);
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_sys_sec_chan->key, 0, key_len * MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM) * sizeof(uint8));
    }

    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "add sec_chan,chan_id:%d (sc_idx:%d) dir:%d\n", p_sys_sec_chan->chan_id, p_sys_sec_chan->sc_index,p_sys_sec_chan->dir);

    _sys_usw_dot1ae_reset_sec_chan_cfg( lchip,p_sys_sec_chan);
    SYS_DOT1AE_UNLOCK(lchip);
    return CTC_E_NONE;

error:

    SYS_DOT1AE_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_dot1ae_remove_sec_chan(uint8 lchip,  ctc_dot1ae_sc_t* p_sc)
{
    sys_usw_opf_t opf;
    uint32   sc_array_idx = 0;
    uint32  offset=0;
    sys_dot1ae_chan_t* p_sys_sec_chan = NULL;
    sys_dot1ae_chan_bind_node_t* p_sec_chan_bind_node = NULL;
    ctc_slistnode_t        *node = NULL, *next_node = NULL;

    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_DOT1AE_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_sc);

    SYS_DOT1AE_LOCK(lchip);
    p_sys_sec_chan = _sys_usw_dot1ae_get_chan_by_id(lchip,  p_sc->chan_id);
    if(!p_sys_sec_chan)
    {
        SYS_DOT1AE_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }

    if(CTC_SLISTCOUNT(p_sys_sec_chan->bind_mem_list))
    {
        SYS_DOT1AE_UNLOCK(lchip);
        return CTC_E_IN_USE;
    }
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    opf.pool_type = usw_dot1ae_master[lchip]->dot1ae_opf_type;
    opf.pool_index = (p_sys_sec_chan->dir ==CTC_DOT1AE_SC_DIR_TX) ?0 :1;

    offset = p_sys_sec_chan->sc_index;
    sys_usw_opf_free_offset(lchip, &opf, 1, offset);
    /*array idx:tx:0~31:rx:32~63*/
    sc_array_idx = offset + ((p_sys_sec_chan->dir ==CTC_DOT1AE_SC_DIR_RX)?MCHIP_CAP(SYS_CAP_DOT1AE_TX_CHAN_NUM):0);
    p_sys_sec_chan = &usw_dot1ae_master[lchip]->dot1ae_sec_chan[sc_array_idx];

    CTC_SLIST_LOOP_DEL(p_sys_sec_chan->bind_mem_list, node, next_node)
    {
        p_sec_chan_bind_node = (sys_dot1ae_chan_bind_node_t*)node;
        mem_free(p_sec_chan_bind_node);
    }
    ctc_slist_free(p_sys_sec_chan->bind_mem_list);
    p_sys_sec_chan->valid = 0;
    p_sys_sec_chan->an_valid = 0;
    if(p_sys_sec_chan->key)
    {
        mem_free(p_sys_sec_chan->key);
    }
    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "add sec_chan,chan_id:%d (sc_idx:%d) dir:%d\n", p_sys_sec_chan->chan_id, p_sys_sec_chan->sc_index,p_sys_sec_chan->dir);

    SYS_DOT1AE_UNLOCK(lchip);
    return CTC_E_NONE;
}


int32
_sys_usw_dot1ae_set_sa_cfg(uint8 lchip, sys_dot1ae_chan_t *p_sc_info,ctc_dot1ae_sa_t* p_sa)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint8  key_index   = 0,aes_key_index = 0;
    uint32  cipher_mode = 0;
    uint8  ebit_cbit = 0;
    uint32 ebit_cbit_all[4] = {0};
    uint32 key_len = CTC_DOT1AE_KEY_LEN;
    ctc_dot1ae_glb_cfg_t glb_cfg;
    int32 ret = CTC_E_NONE;
    uint8* p_key_hw = NULL;
    uint8  salt_hw[12];
    uint32 pn_mode[4] = {0};
    uint32 validateFrames = 0;
    uint32 icv_discard = 0;
    uint32 icv_exception = 0;

    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    CTC_PTR_VALID_CHECK(p_sa);
    CTC_MAX_VALUE_CHECK(p_sa->an, MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM) - 1);
    CTC_MAX_VALUE_CHECK(p_sa->cipher_suite, CTC_DOT1AE_CIPHER_SUITE_GCM_AES_XPN_256);
    CTC_MAX_VALUE_CHECK(p_sa->validateframes, CTC_DOT1AE_VALIDFRAMES_STRICT);

    if(DRV_IS_DUET2(lchip))
    {
        key_len = CTC_DOT1AE_KEY_LEN/2;
        CTC_MAX_VALUE_CHECK(p_sa->next_pn, 0xffffffff);
    }

    key_index = (p_sc_info->sc_index<< 2) + (p_sa->an & 0x3);
        /* tx key_index: 0-127; rx key_index: 128-255 */
    aes_key_index = key_index + ((p_sc_info->dir == CTC_DOT1AE_SC_DIR_TX)?0:128);

    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "key_index:(%u)  aes_key_index:(%u)    sc_index(%u)    an(%u)\n", key_index,aes_key_index,p_sc_info->sc_index, p_sa->an);

    if(CTC_FLAG_ISSET(p_sa->upd_flag,CTC_DOT1AE_SA_FLAG_NEXT_PN))
    {
        if(CTC_FLAG_ISSET(p_sa->upd_flag,CTC_DOT1AE_SA_FLAG_KEY))
        {
            value = p_sa->cipher_suite;
        }
        else
        {
            cmd = DRV_IOR(DsDot1AeAesKey_t, DsDot1AeAesKey_cipherSuite_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, aes_key_index, cmd, &value));
        }
        if(value < 2)
        {
            CTC_MAX_VALUE_CHECK(p_sa->next_pn, 0xffffffff);
        }
        sys_usw_dot1ae_get_global_cfg(lchip,&glb_cfg);
        if(p_sc_info->dir == CTC_DOT1AE_SC_DIR_TX && glb_cfg.tx_pn_thrd > 0)
        {
            CTC_MAX_VALUE_CHECK(p_sa->next_pn, glb_cfg.tx_pn_thrd);
        }
        if(p_sc_info->dir == CTC_DOT1AE_SC_DIR_RX && glb_cfg.rx_pn_thrd > 0)
        {
            CTC_MAX_VALUE_CHECK(p_sa->next_pn, glb_cfg.rx_pn_thrd);
        }

        cmd = (p_sc_info->dir == CTC_DOT1AE_SC_DIR_TX) ? DRV_IOW(DsEgressDot1AePn_t, DsEgressDot1AePn_pn_f)
                                    : DRV_IOW(DsDot1AePnCheck_t, DsDot1AePnCheck_nextPn_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &p_sa->next_pn));
    }

    if(CTC_FLAG_ISSET(p_sa->upd_flag,CTC_DOT1AE_SA_FLAG_KEY))
    {
        p_key_hw = mem_malloc(MEM_DOT1AE_MODULE, key_len);
        if(NULL == p_key_hw)
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_key_hw, 0 , key_len);
        if(p_sc_info->dir == CTC_DOT1AE_SC_DIR_RX)
        {
            if(!p_sc_info->key)
            {
                mem_free(p_key_hw);
                return CTC_E_NO_MEMORY;
            }
            if(DRV_IS_DUET2(lchip)&&(p_sa->key[0] != '\0'))
            {
                sal_memcpy((uint8 *)p_sc_info->key + p_sa->an *key_len,(uint8 *)p_sa->key,key_len*sizeof(uint8));
                SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "rx_key:(%s) \n",(p_sc_info->key + p_sa->an *key_len));
            }
        }
        SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "key:(%s) \n",(p_sa->key));

        if(!DRV_IS_DUET2(lchip) && (CTC_DOT1AE_CIPHER_SUITE_GCM_AES_128 == p_sa->cipher_suite || CTC_DOT1AE_CIPHER_SUITE_GCM_AES_XPN_128 == p_sa->cipher_suite))
        {
            SYS_DOT1AE_REVERSE(p_key_hw,p_sa->key, key_len/2);
        }
        else
        {
            SYS_DOT1AE_REVERSE(p_key_hw,p_sa->key, key_len);
        }
        if(!(DRV_IS_DUET2(lchip) && !CTC_IS_BIT_SET(p_sc_info->an_en, p_sa->an)))
        {
            SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "tx_hw_key:(%s) \n", (p_key_hw));
            cmd = DRV_IOW(DsDot1AeAesKey_t, DsDot1AeAesKey_aesKey_f);
            ret = DRV_IOCTL(lchip, aes_key_index, cmd, p_key_hw);
        }
        mem_free(p_key_hw);
        if(ret < 0)
        {
            return ret;
        }

        sal_memset(salt_hw, 0, sizeof(salt_hw));
        SYS_DOT1AE_REVERSE(salt_hw,p_sa->salt, sizeof(salt_hw));
        cmd = DRV_IOW(DsDot1AeAesKey_t, DsDot1AeAesKey_salt_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, aes_key_index, cmd, salt_hw));

        cmd = DRV_IOW(DsDot1AeAesKey_t, DsDot1AeAesKey_ssci_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, aes_key_index, cmd, &p_sa->ssci));

        value = p_sa->cipher_suite;
        cmd = DRV_IOW(DsDot1AeAesKey_t, DsDot1AeAesKey_cipherSuite_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, aes_key_index, cmd, &value));

        if(p_sa->cipher_suite > CTC_DOT1AE_CIPHER_SUITE_GCM_AES_256 && p_sc_info->dir == CTC_DOT1AE_SC_DIR_TX)
        {
            cmd = DRV_IOR(EpePktProcCtl_t, EpePktProcCtl_dot1AePnMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, pn_mode));
            CTC_BIT_SET(pn_mode[key_index/32], key_index%32);
            cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_dot1AePnMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, pn_mode));
        }
        else
        {
            cmd = DRV_IOR(EpePktProcCtl_t, EpePktProcCtl_dot1AePnMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, pn_mode));
            CTC_BIT_UNSET(pn_mode[key_index/32], key_index%32);
            cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_dot1AePnMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, pn_mode));
        }
    }

    if(CTC_FLAG_ISSET(p_sa->upd_flag,CTC_DOT1AE_SA_FLAG_ENCRYPTION))
    {
        if(!DRV_IS_DUET2(lchip))
        {
            if(CTC_SLISTCOUNT(p_sc_info->bind_mem_list) && (1 == p_sc_info->binding_type) /*cloud sec*/
                && (CTC_DOT1AE_ENCRYPTION_OFFSET_30 == p_sa->confid_offset || CTC_DOT1AE_ENCRYPTION_OFFSET_50 == p_sa->confid_offset))
            {
                return CTC_E_INVALID_CONFIG;
            }
        }

        if(p_sa->no_encryption)
        {
            cipher_mode = 3;
            ebit_cbit = 0;
        }
        else if(p_sa->confid_offset == CTC_DOT1AE_ENCRYPTION_OFFSET_0)
        {
            cipher_mode = 0;
            ebit_cbit = 1;
        }
        else if(p_sa->confid_offset == CTC_DOT1AE_ENCRYPTION_OFFSET_30)
        {
            cipher_mode = 1;
            ebit_cbit = 1;
        }
        else if(p_sa->confid_offset == CTC_DOT1AE_ENCRYPTION_OFFSET_50)
        {
            cipher_mode = 2;
            ebit_cbit = 1;
        }
        else
        {
            return CTC_E_INVALID_PARAM;
        }

        cmd = DRV_IOW(DsDot1AeAesKey_t, DsDot1AeAesKey_cipherMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, aes_key_index, cmd, &cipher_mode));

        cmd = DRV_IOR(EpePktProcCtl_t, EpePktProcCtl_dot1AeTciEbitCbit_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ebit_cbit_all));
        if(ebit_cbit)
        {
            CTC_BIT_SET(ebit_cbit_all[key_index/32], key_index%32);
        }
        else
        {
            CTC_BIT_UNSET(ebit_cbit_all[key_index/32], key_index%32);
        }
        cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_dot1AeTciEbitCbit_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ebit_cbit_all));
    }

       if ((!DRV_IS_DUET2(lchip)) && p_sc_info->dir == CTC_DOT1AE_SC_DIR_RX && CTC_FLAG_ISSET(p_sa->upd_flag, CTC_DOT1AE_SA_FLAG_VALIDATEFRAMES))
       {
           icv_discard = (p_sa->validateframes != CTC_DOT1AE_VALIDFRAMES_DISABLE);
           validateFrames = p_sa->validateframes;
           /*modify for validateframes equal zero, must check error*/
           if (CTC_DOT1AE_VALIDFRAMES_DISABLE == p_sa->validateframes)
           {
                validateFrames = 3;
           }
       }
       else if (CTC_FLAG_ISSET(p_sa->upd_flag, CTC_DOT1AE_SA_FLAG_ICV_ERROR_ACTION)
           && p_sc_info->dir == CTC_DOT1AE_SC_DIR_RX)
       {
           /*icv check mode*/
           switch (p_sa->icv_error_action)
           {
               case CTC_EXCP_NORMAL_FWD:
                   icv_discard = 0;
                   icv_exception = 0;
                   validateFrames = 1;
                   break;
               case CTC_EXCP_FWD_AND_TO_CPU:
                   icv_discard = 0;
                   icv_exception = 1;
                   validateFrames = 1;
                   break;
               case CTC_EXCP_DISCARD_AND_TO_CPU:
                   icv_discard = 1;
                   icv_exception = 1;
                   validateFrames = 2;
                   break;
               case CTC_EXCP_DISCARD:
                   icv_discard = 1;
                   icv_exception = 0;
                   validateFrames = 2;
                   break;
               default:
                   return CTC_E_INVALID_PARAM;
           }


       }

       if (p_sc_info->dir == CTC_DOT1AE_SC_DIR_RX && ( CTC_FLAG_ISSET(p_sa->upd_flag, CTC_DOT1AE_SA_FLAG_VALIDATEFRAMES)
           || CTC_FLAG_ISSET(p_sa->upd_flag, CTC_DOT1AE_SA_FLAG_ICV_ERROR_ACTION)))
       {
           cmd = DRV_IOW(DsDot1AeDecryptConfig_t, DsDot1AeDecryptConfig_validateFrames_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &validateFrames));
           cmd = DRV_IOW(DsDot1AeDecryptConfig_t, DsDot1AeDecryptConfig_icvCheckFailDiscard_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &icv_discard));
           cmd = DRV_IOW(DsDot1AeDecryptConfig_t, DsDot1AeDecryptConfig_icvCheckFailException_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &icv_exception));

           cmd = DRV_IOW(DsDot1AePnCheck_t, DsDot1AePnCheck_icvCheckFailDiscard_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &icv_discard));
           cmd = DRV_IOW(DsDot1AePnCheck_t, DsDot1AePnCheck_icvCheckFailException_f);
           CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &icv_exception));
       }

    return CTC_E_NONE;
}
int32
_sys_usw_dot1ae_update_sec_chan_an_en(uint8 lchip,sys_dot1ae_chan_t* p_sys_sec_chan, uint32 an_bitmap)
{
    uint8 an = 0;
    uint32 enable = 0;
    uint32 enable_old = 0;
    uint8 an_count = 0;
    int32 ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 value = 0;
    uint8 step = 0;
    uint32 key_len = CTC_DOT1AE_KEY_LEN;
    uint8  key_index   = 0;

    if(DRV_IS_DUET2(lchip))
    {
        key_len = CTC_DOT1AE_KEY_LEN/2;
    }

    if(an_bitmap > ((1 << MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM)) - 1))
    {
        return CTC_E_INVALID_PARAM;
    }

    for(an = 0; an < MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM);an++)
    {
        if( CTC_IS_BIT_SET(an_bitmap,an))
        {
            an_count++;
        }
    }

    if(p_sys_sec_chan->dir == CTC_DOT1AE_SC_DIR_TX && an_count != 1)
    {
        return CTC_E_INVALID_PARAM;
    }

    for(an = 0; an < MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM);an++)
    {
        if(CTC_IS_BIT_SET(an_bitmap,an) && !CTC_IS_BIT_SET(p_sys_sec_chan->an_valid,an))
        {
            return CTC_E_NOT_READY;
        }
    }
    for(an = 0; an < MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM);an++)
    {
        if(!CTC_IS_BIT_SET(an_bitmap,an)
            && (p_sys_sec_chan->dir == CTC_DOT1AE_SC_DIR_TX))
        {
            /*rx need clear key when an disable*/
            continue;
        }
        enable_old = CTC_IS_BIT_SET(p_sys_sec_chan->an_en,an);
        enable = CTC_IS_BIT_SET(an_bitmap,an);

        if(p_sys_sec_chan->dir == CTC_DOT1AE_SC_DIR_TX )
        {
            value = an;
            step = EpeDot1AeAnCtl0_array_1_currentAn_f - EpeDot1AeAnCtl0_array_0_currentAn_f;
            cmd = DRV_IOW(EpeDot1AeAnCtl0_t, EpeDot1AeAnCtl0_array_0_currentAn_f + step * (p_sys_sec_chan->sc_index << 2));
            ret = DRV_IOCTL(lchip, 0, cmd, &value);
        }
        else
        {
            if(DRV_IS_DUET2(lchip))
            {
                ctc_dot1ae_sa_t sa;
                sal_memset(&sa, 0, sizeof(ctc_dot1ae_sa_t));
                sa.an = an;
                CTC_SET_FLAG(sa.upd_flag, CTC_DOT1AE_SA_FLAG_KEY);
                if (enable)
                {
                    CTC_BIT_SET(p_sys_sec_chan->an_en, an);
                    sal_memcpy(sa.key, p_sys_sec_chan->key + an * key_len, key_len);
                }
                if (enable != enable_old)
                {
                    ret = _sys_usw_dot1ae_set_sa_cfg(lchip, p_sys_sec_chan, &sa);
                }

                if (!enable || (enable && ret != CTC_E_NONE) )
                {
                    CTC_BIT_UNSET(p_sys_sec_chan->an_en, an);
                }
            }
            else
            {
                if (enable)
                {
                    CTC_BIT_SET(p_sys_sec_chan->an_en, an);
                }
                else
                {
                    CTC_BIT_UNSET(p_sys_sec_chan->an_en, an);
                }
                key_index = (p_sys_sec_chan->sc_index<< 2) + (an & 0x3);
                cmd = DRV_IOW(DsDot1AeDecryptConfig_t, DsDot1AeDecryptConfig_inUse_f);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &enable));
            }
        }
    }

    return ret;
}
int32
sys_usw_dot1ae_update_sec_chan(uint8 lchip, ctc_dot1ae_sc_t* p_sc)
{
    int32  ret = CTC_E_NONE;
    uint32 cmd = 0;
    sys_dot1ae_chan_t* p_sys_sec_chan = NULL;
    uint8 sci_hw[8] = {0};
    uint32 key_index = 0;
    uint8 index = 0 ;
    uint16 tmp1 = 0,tmp2 = 0;
    uint32 value =0;

    DsDot1AePnCheck_m rx_pn;
    DsDot1AeDecryptConfig_m rx_cfg;
    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_DOT1AE_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_sc);
    SYS_DOT1AE_LOCK(lchip);
    p_sys_sec_chan = _sys_usw_dot1ae_get_chan_by_id(lchip,  p_sc->chan_id);
    if(!p_sys_sec_chan)
    {
        SYS_DOT1AE_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }
    ret = CTC_E_NONE;
    key_index = p_sys_sec_chan->sc_index  <<2;
    switch(p_sc->property)
    {
    case CTC_DOT1AE_SC_PROP_AN_EN:              /**< [D2][tx/rx] Enable AN to channel  */
        ret = _sys_usw_dot1ae_update_sec_chan_an_en(lchip,p_sys_sec_chan, p_sc->data);
        break;
    case CTC_DOT1AE_SC_PROP_NEXT_AN:
    {
        uint8 step = 0;
        if(p_sys_sec_chan->dir == CTC_DOT1AE_SC_DIR_RX || p_sc->data >= MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM) )
        {
            SYS_DOT1AE_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
        if(!CTC_IS_BIT_SET(p_sys_sec_chan->an_valid,p_sc->data))
        {
            SYS_DOT1AE_UNLOCK(lchip);
            return CTC_E_NOT_READY;
        }
        value =  p_sc->data;
        step = EpeDot1AeAnCtl1_array_1_nextAn_f - EpeDot1AeAnCtl1_array_0_nextAn_f;
        cmd = DRV_IOW(EpeDot1AeAnCtl1_t, EpeDot1AeAnCtl1_array_0_nextAn_f + step * key_index);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret,error);
        p_sys_sec_chan->next_an = p_sc->data;
    }
        break;
    case CTC_DOT1AE_SC_PROP_INCLUDE_SCI:          /**< [D2]  [tx] Transmit SCI in MacSec Tag*/
        if(p_sys_sec_chan->dir == CTC_DOT1AE_SC_DIR_RX)
        {
            SYS_DOT1AE_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
        p_sc->data =  p_sc->data ? 1: 0;

        if (CTC_SLISTCOUNT(p_sys_sec_chan->bind_mem_list))
        {
            sys_dot1ae_chan_bind_node_t* p_sec_chan_bind_node = NULL;
            ctc_slistnode_t        *node = NULL;
            if (1 == p_sys_sec_chan->binding_type)
            {
                sys_usw_register_dot1ae_bind_sc_t bind_sc;
                CTC_SLIST_LOOP(p_sys_sec_chan->bind_mem_list, node)
                {
                    p_sec_chan_bind_node = _ctc_container_of(node, sys_dot1ae_chan_bind_node_t, head);
                    sal_memset(&bind_sc, 0, sizeof(bind_sc));
                    bind_sc.include_sci = p_sc->data;
                    bind_sc.gport = p_sec_chan_bind_node->value;
                    CTC_ERROR_GOTO(sys_usw_nh_update_dot1ae(lchip, (void*)(&bind_sc)), ret, error);
                }
            }
            else
            {
                CTC_SLIST_LOOP(p_sys_sec_chan->bind_mem_list, node)
                {
                    p_sec_chan_bind_node = _ctc_container_of(node, sys_dot1ae_chan_bind_node_t, head);
                    CTC_ERROR_GOTO(sys_usw_port_set_internal_property(lchip, p_sec_chan_bind_node->value, SYS_PORT_PROP_DOT1AE_TX_SCI_EN, p_sc->data), ret, error);
                }
            }
        }
        p_sys_sec_chan->include_sci = p_sc->data;
        break;
    case CTC_DOT1AE_SC_PROP_SCI:                /**< [D2]  [tx] Secure Channel Identifier,64 bit. uint32 value[2]*/
        if(!p_sc->ext_data)
        {
            SYS_DOT1AE_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
        SYS_DOT1AE_REVERSE(sci_hw, (uint8*)p_sc->ext_data, sizeof(sci_hw));
        cmd = (p_sys_sec_chan->dir ==CTC_DOT1AE_SC_DIR_RX) ? DRV_IOW(DsDot1AeSci_t, DsDot1AeSci_sci_f)
                                    :DRV_IOW(DsEgressDot1AeSci_t, DsEgressDot1AeSci_sci_f);
        for(index = 0; index < MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM); index++)
        {
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index + index , cmd, &sci_hw),ret,error);
        }
        break;
    case CTC_DOT1AE_SC_PROP_REPLAY_WINDOW_SIZE:  /**< [D2] [rx] Replay protect windows size*/
        if(p_sys_sec_chan->dir == CTC_DOT1AE_SC_DIR_TX)
        {
            SYS_DOT1AE_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
        if(DRV_IS_DUET2(lchip))
        {
            sys_usw_common_get_compress_near_division_value(lchip, p_sc->data,
                                    MCHIP_CAP(SYS_CAP_DOT1AE_DIVISION_WIDE), MCHIP_CAP(SYS_CAP_DOT1AE_SHIFT_WIDE), &tmp1, &tmp2, 0);
            for(index = 0; index < MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM); index++)
            {
                cmd = DRV_IOR(DsDot1AePnCheck_t, DRV_ENTRY_FLAG);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index + index, cmd, &rx_pn),ret,error);
                SetDsDot1AePnCheck(V, replayWindow_f, &rx_pn, tmp1);
                SetDsDot1AePnCheck(V, dot1AeReplayWindowShift_f, &rx_pn, tmp2);
                cmd = DRV_IOW(DsDot1AePnCheck_t, DRV_ENTRY_FLAG);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index + index, cmd, &rx_pn),ret,error);
            }
        }
        else
        {
            for(index = 0; index < MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM); index++)
            {
                cmd = DRV_IOR(DsDot1AeDecryptConfig_t, DRV_ENTRY_FLAG);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index + index, cmd, &rx_cfg),ret,error);
                SetDsDot1AeDecryptConfig(A, replayWindow_f, &rx_cfg, &p_sc->data);
                cmd = DRV_IOW(DsDot1AeDecryptConfig_t, DRV_ENTRY_FLAG);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index + index, cmd, &rx_cfg),ret,error);
            }

        }
        break;
    case CTC_DOT1AE_SC_PROP_REPLAY_PROTECT_EN:
        if(p_sys_sec_chan->dir == CTC_DOT1AE_SC_DIR_TX)
        {
            SYS_DOT1AE_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
        for(index = 0; index < MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM); index++)
        {
            if(DRV_IS_DUET2(lchip))
            {
                cmd = DRV_IOR(DsDot1AePnCheck_t, DRV_ENTRY_FLAG);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index + index, cmd, &rx_pn),ret,error);
                SetDsDot1AePnCheck(V, replayProtect_f, &rx_pn,p_sc->data?1:0);
                cmd = DRV_IOW(DsDot1AePnCheck_t, DRV_ENTRY_FLAG);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index + index, cmd, &rx_pn),ret,error);
            }
            else
            {
                cmd = DRV_IOR(DsDot1AeDecryptConfig_t, DRV_ENTRY_FLAG);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index + index, cmd, &rx_cfg),ret,error);
                SetDsDot1AeDecryptConfig(V, replayProtect_f, &rx_cfg, p_sc->data?1:0);
                cmd = DRV_IOW(DsDot1AeDecryptConfig_t, DRV_ENTRY_FLAG);
                CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index + index, cmd, &rx_cfg),ret,error);
            }
        }
        break;
    case CTC_DOT1AE_SC_PROP_SA_CFG:/**< [D2] [tx/rx] configure SA Property, ctc_dot1ae_sa_t*/
        CTC_ERROR_GOTO(_sys_usw_dot1ae_set_sa_cfg(lchip,p_sys_sec_chan,(ctc_dot1ae_sa_t*)p_sc->ext_data),ret,error);
        CTC_BIT_SET(p_sys_sec_chan->an_valid,((ctc_dot1ae_sa_t*)p_sc->ext_data)->an);
        break;
    default :
        break;
     }
error:
    SYS_DOT1AE_UNLOCK(lchip);
    return ret;
}

int32
_sys_usw_dot1ae_get_sa_cfg(uint8 lchip, sys_dot1ae_chan_t *p_sc_info,ctc_dot1ae_sa_t* p_sa)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint32 mask = 1;
    uint32  key_index = 0;
    uint32  aes_key_index = 0;
    uint32  cipher_mode = 0;
    uint8 step = 0;
    uint8* p_key_hw = NULL;
    uint32 key_len = CTC_DOT1AE_KEY_LEN;
    int32 ret = CTC_E_NONE;
    uint8  salt_tmp[12] = {0};

    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    if(DRV_IS_DUET2(lchip))
    {
        key_len = CTC_DOT1AE_KEY_LEN/2;
    }

    CTC_PTR_VALID_CHECK(p_sa);
    CTC_MAX_VALUE_CHECK(p_sa->an, (MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM)-1));
    mask = mask << p_sa->an;
    if(!CTC_FLAG_ISSET(p_sc_info->an_valid,mask))
    {
        SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "The an has not beed configured\n");
        return CTC_E_NOT_READY;
    }
    key_index = (p_sc_info->sc_index << 2) + (p_sa->an & 0x3);
    /* tx key_index: 0-127; rx key_index: 128-255 */
    aes_key_index = key_index + ((p_sc_info->dir == CTC_DOT1AE_SC_DIR_TX)?0:128);

    /*next_pn*/
    cmd = (p_sc_info->dir == CTC_DOT1AE_SC_DIR_TX) ? DRV_IOR(DsEgressDot1AePn_t, DsEgressDot1AePn_pn_f)
                               : DRV_IOR(DsDot1AePnCheck_t, DsDot1AePnCheck_nextPn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &p_sa->next_pn));
    /*RX CFG*/
    if(p_sc_info->dir == CTC_DOT1AE_SC_DIR_RX)
    {
        uint64 window_size = 0;
        uint32 validateFrames = 0;
        uint32 replayProtect = 0;
        uint32 discard = 0, expection = 0;
        uint32 tmp1 = 0,tmp2 = 0;
        if(DRV_IS_DUET2(lchip))
        {
            DsDot1AePnCheck_m rx_pn_cfg;
            cmd = DRV_IOR(DsDot1AePnCheck_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, key_index, cmd, &rx_pn_cfg);
            replayProtect = GetDsDot1AePnCheck(V, replayProtect_f, &rx_pn_cfg);
            tmp1 = GetDsDot1AePnCheck(V, replayWindow_f, &rx_pn_cfg);
            tmp2 = GetDsDot1AePnCheck(V, dot1AeReplayWindowShift_f, &rx_pn_cfg);
            window_size = tmp1 << tmp2;

            cmd = DRV_IOR(DsDot1AePnCheck_t, DsDot1AePnCheck_icvCheckFailDiscard_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &discard));
            cmd = DRV_IOR(DsDot1AePnCheck_t, DsDot1AePnCheck_icvCheckFailException_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, key_index, cmd, &expection));
        }
        else
        {
            DsDot1AeDecryptConfig_m rx_pn_cfg;
            cmd = DRV_IOR(DsDot1AeDecryptConfig_t, DRV_ENTRY_FLAG);
            DRV_IOCTL(lchip, key_index, cmd, &rx_pn_cfg);
            replayProtect = GetDsDot1AeDecryptConfig(V, replayProtect_f, &rx_pn_cfg);
            GetDsDot1AeDecryptConfig(A, replayWindow_f, &rx_pn_cfg, &window_size);
            p_sa->icv_error_action = 0;
            /*SYS_DOT1AE_32var_TO_64var(window_size, value_array[1], value_array[0]);*/

            validateFrames = GetDsDot1AeDecryptConfig(V, validateFrames_f, &rx_pn_cfg);
            /*modify for validateframes equal zero, must check error*/
            if (3 == validateFrames)
            {
                validateFrames = CTC_DOT1AE_VALIDFRAMES_DISABLE;
            }
            p_sa->validateframes= validateFrames;
        }

        if(replayProtect)
        {
            p_sa->lowest_pn = (p_sa->next_pn > window_size)?(p_sa->next_pn-window_size):1;
        }
        else
        {
            p_sa->lowest_pn =  p_sa->next_pn ;
        }

        if (DRV_IS_DUET2(lchip))
        {
            if (discard && expection)
            {
                p_sa->icv_error_action = CTC_EXCP_DISCARD_AND_TO_CPU;
            }
            else if(!discard && expection)
            {
                p_sa->icv_error_action = CTC_EXCP_FWD_AND_TO_CPU;
            }
            else if(!discard && !expection)
            {
                p_sa->icv_error_action = CTC_EXCP_NORMAL_FWD;
            }
            else
            {
                p_sa->icv_error_action  = CTC_EXCP_DISCARD;
            }
        }
    }

    /*AN in_use*/
    if(p_sc_info->dir == CTC_DOT1AE_SC_DIR_TX)
    {
        step = EpeDot1AeAnCtl0_array_1_currentAn_f - EpeDot1AeAnCtl0_array_0_currentAn_f;
        cmd = DRV_IOR(EpeDot1AeAnCtl0_t, EpeDot1AeAnCtl0_array_0_currentAn_f + step*(p_sc_info->sc_index<< 2));
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
        p_sa->in_use = (value == p_sa->an);
    }
    else
    {
        p_sa->in_use = CTC_IS_BIT_SET(p_sc_info->an_en,p_sa->an);
    }

    cmd = DRV_IOR(DsDot1AeAesKey_t, DsDot1AeAesKey_salt_f);
    ret = DRV_IOCTL(lchip, aes_key_index, cmd, salt_tmp);
    SYS_DOT1AE_REVERSE(p_sa->salt, salt_tmp, sizeof(salt_tmp));
    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "p_sa->salt:(%s) \n", (p_sa->salt));

    cmd = DRV_IOR(DsDot1AeAesKey_t, DsDot1AeAesKey_ssci_f);
    ret = DRV_IOCTL(lchip, aes_key_index, cmd, &p_sa->ssci);

    cmd = DRV_IOR(DsDot1AeAesKey_t, DsDot1AeAesKey_cipherSuite_f);
    ret = DRV_IOCTL(lchip, aes_key_index, cmd, &value);
    p_sa->cipher_suite = value;


     /*aes key*/
    if(DRV_IS_DUET2(lchip) && p_sc_info->dir == CTC_DOT1AE_SC_DIR_RX)
    {
        sal_memcpy((uint8 *)p_sa->key,(uint8 *)p_sc_info->key + p_sa->an *key_len,key_len);
    }
    else
    {
        p_key_hw = mem_malloc(MEM_DOT1AE_MODULE, key_len);
        if (NULL == p_key_hw)
        {
            return CTC_E_NO_MEMORY;
        }
        sal_memset(p_key_hw, 0 , key_len);
        cmd = DRV_IOR(DsDot1AeAesKey_t, DsDot1AeAesKey_aesKey_f);
        ret = DRV_IOCTL(lchip, aes_key_index, cmd, p_key_hw);
        if (!DRV_IS_DUET2(lchip) && (CTC_DOT1AE_CIPHER_SUITE_GCM_AES_128 == p_sa->cipher_suite || CTC_DOT1AE_CIPHER_SUITE_GCM_AES_XPN_128 == p_sa->cipher_suite))
        {
            SYS_DOT1AE_REVERSE(p_sa->key, p_key_hw, key_len / 2);
        }
        else
        {
            SYS_DOT1AE_REVERSE(p_sa->key, p_key_hw, key_len);
        }
        mem_free(p_key_hw);
        if (ret < 0)
        {
            return ret;
        }
    }

    /*ENCRYPTION_OFFSET*/
    if(p_sc_info->dir == CTC_DOT1AE_SC_DIR_TX)
    {
        cmd = DRV_IOR(DsDot1AeAesKey_t, DsDot1AeAesKey_cipherMode_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, aes_key_index, cmd, &cipher_mode));
        if (cipher_mode != 3)
        {
            if (cipher_mode == 0)
            {
                p_sa->confid_offset = CTC_DOT1AE_ENCRYPTION_OFFSET_0;
            }
            else if(cipher_mode == 1)
            {
                p_sa->confid_offset = CTC_DOT1AE_ENCRYPTION_OFFSET_30;
            }
            else if(cipher_mode == 2)
            {
                p_sa->confid_offset = CTC_DOT1AE_ENCRYPTION_OFFSET_50;
            }
            p_sa->no_encryption = 0;
        }
        else
        {
            p_sa->no_encryption = 1;
        }
    }

    return CTC_E_NONE;
}
int32
sys_usw_dot1ae_get_sec_chan(uint8 lchip, ctc_dot1ae_sc_t* p_sc)
{
    int32  ret = CTC_E_NONE;
    uint8 step = 0;
    uint32 value = 0;
    uint32 cmd = 0;
    sys_dot1ae_chan_t* p_sys_sec_chan = NULL;
    uint8 sci_hw[8] = {0};
    uint32 key_index = 0;
    DsDot1AePnCheck_m rx_pn;
    DsDot1AeDecryptConfig_m rx_cfg;
    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_DOT1AE_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_sc);
    SYS_DOT1AE_LOCK(lchip);
    p_sys_sec_chan = _sys_usw_dot1ae_get_chan_by_id(lchip,  p_sc->chan_id);
    if(!p_sys_sec_chan)
    {
        SYS_DOT1AE_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }
    ret = CTC_E_NONE;
    p_sc->dir = p_sys_sec_chan->dir;
    switch(p_sc->property)
    {
    case CTC_DOT1AE_SC_PROP_AN_EN:              /**< [D2][tx/rx] Enable AN to channel  (0~3) */
    {
        if(p_sys_sec_chan->dir == CTC_DOT1AE_SC_DIR_TX)
        {
            step = EpeDot1AeAnCtl0_array_1_currentAn_f - EpeDot1AeAnCtl0_array_0_currentAn_f;
            cmd = DRV_IOR(EpeDot1AeAnCtl0_t, EpeDot1AeAnCtl0_array_0_currentAn_f + step*(p_sys_sec_chan->sc_index<< 2));
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &value),ret,error);
            CTC_BIT_SET(p_sc->data,value);  /*in use*/
        }
        else
        {
            p_sc->data = p_sys_sec_chan->an_en;
        }
    }
        break;
    case CTC_DOT1AE_SC_PROP_INCLUDE_SCI:          /**< [D2]  [tx] Transmit SCI in MacSec Tag*/
        p_sc->data = p_sys_sec_chan->include_sci ;
        break;
    case CTC_DOT1AE_SC_PROP_SCI:               /**< [D2]  [tx] Secure Channel Identifier,64 bit. uint32 value[2]*/
        if(!p_sc->ext_data)
        {
            SYS_DOT1AE_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
        key_index = p_sys_sec_chan->sc_index  << 2;
        cmd = (p_sys_sec_chan->dir ==CTC_DOT1AE_SC_DIR_RX) ? DRV_IOR(DsDot1AeSci_t, DsDot1AeSci_sci_f)
                                                   :DRV_IOR(DsEgressDot1AeSci_t, DsEgressDot1AeSci_sci_f);
        CTC_ERROR_GOTO(DRV_IOCTL(lchip, key_index, cmd, &sci_hw),ret,error);
        SYS_DOT1AE_REVERSE((uint8*)p_sc->ext_data,sci_hw, sizeof(sci_hw));
        break;
    case CTC_DOT1AE_SC_PROP_REPLAY_WINDOW_SIZE:  /**< [D2] [rx] Replay protect windows size*/
    {
        if (DRV_IS_DUET2(lchip))
        {
            uint8 drv_size = 0;
            uint8 drv_shift = 0;
            cmd = DRV_IOR(DsDot1AePnCheck_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, p_sys_sec_chan->sc_index << 2, cmd, &rx_pn), ret, error);
            if (GetDsDot1AePnCheck(V, replayProtect_f, &rx_pn))
            {
                drv_size = GetDsDot1AePnCheck(V, replayWindow_f, &rx_pn);
                drv_shift = GetDsDot1AePnCheck(V, dot1AeReplayWindowShift_f, &rx_pn);
                p_sc->data  = drv_size << drv_shift;
            }
            else
            {
                p_sc->data = 0;
            }
        }
        else
        {
            cmd = DRV_IOR(DsDot1AeDecryptConfig_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, p_sys_sec_chan->sc_index << 2, cmd, &rx_cfg), ret, error);
            if(GetDsDot1AeDecryptConfig(V, replayProtect_f, &rx_cfg))
            {
                GetDsDot1AeDecryptConfig(A, replayWindow_f, &rx_cfg, &p_sc->data);
            }
            else
            {
                p_sc->data = 0;
            }
        }
    }
        break;
    case CTC_DOT1AE_SC_PROP_REPLAY_PROTECT_EN:
        if (DRV_IS_DUET2(lchip))
        {
            cmd = DRV_IOR(DsDot1AePnCheck_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, p_sys_sec_chan->sc_index << 2, cmd, &rx_pn), ret, error);
            p_sc->data = GetDsDot1AePnCheck(V, replayProtect_f, &rx_pn);
        }
        else
        {
            cmd = DRV_IOR(DsDot1AeDecryptConfig_t, DRV_ENTRY_FLAG);
            CTC_ERROR_GOTO(DRV_IOCTL(lchip, p_sys_sec_chan->sc_index << 2, cmd, &rx_cfg), ret, error);
            p_sc->data = GetDsDot1AeDecryptConfig(V, replayProtect_f, &rx_cfg);
        }
        break;
    case CTC_DOT1AE_SC_PROP_SA_CFG:              /**< [D2] [tx/rx] configure SA Property, ctc_dot1ae_sa_t*/
        if(!p_sc->ext_data)
        {
            SYS_DOT1AE_UNLOCK(lchip);
            return CTC_E_INVALID_PARAM;
        }
        ret = _sys_usw_dot1ae_get_sa_cfg(lchip,p_sys_sec_chan,p_sc->ext_data);
        break;
    default :
        break;
    }
error:
    SYS_DOT1AE_UNLOCK(lchip);
    return ret;
}

int32
sys_usw_dot1ae_set_global_cfg(uint8 lchip, ctc_dot1ae_glb_cfg_t* p_glb_cfg)
{
    uint32 value = 0;
    uint32 cmd   = 0;
    uint32 overflow_discard = 0;
    uint32 overflow_exception = 0;

    SYS_DOT1AE_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_glb_cfg);

    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);
    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Dot1AE Encrypt-pn-thrd: %"PRIu64", Decrypt-pn-thrd: %"PRIu64", port_mirror_pkt_type: %u\n",\
                          p_glb_cfg->tx_pn_thrd, p_glb_cfg->rx_pn_thrd,p_glb_cfg->dot1ae_port_mirror_mode);

    if(DRV_IS_DUET2(lchip))
    {
        CTC_MAX_VALUE_CHECK(p_glb_cfg->tx_pn_thrd, 0xffffffff);
        CTC_MAX_VALUE_CHECK(p_glb_cfg->rx_pn_thrd, 0xffffffff);
    }

    cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_dot1AePnThreshold_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &p_glb_cfg->tx_pn_thrd));
    cmd = DRV_IOW(Dot1AeDecryptCtl_t, Dot1AeDecryptCtl_dot1AePnThreshold_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &p_glb_cfg->rx_pn_thrd));

    switch (p_glb_cfg->tx_pn_overflow_action)
    {
        case CTC_EXCP_NORMAL_FWD:
            overflow_discard = 0;
            overflow_exception = 0;
            break;
        case CTC_EXCP_DISCARD_AND_TO_CPU:
            overflow_discard = 1;
            overflow_exception = 1;
            break;
        case CTC_EXCP_DISCARD:
            overflow_discard = 1;
            overflow_exception = 0;
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }
    cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_dot1AePnOverflowDiscard_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &overflow_discard));
    cmd = DRV_IOW(EpePktProcCtl_t, EpePktProcCtl_dot1AePnOverflowExceptionEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &overflow_exception));

    switch(p_glb_cfg->dot1ae_port_mirror_mode)
    {
        case CTC_DOT1AE_PORT_MIRRORED_MODE_PLAIN_TEXT:
            value = 1;
            cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_portLogSelect_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            value = 0;
            cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_dot1AeLogMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            break;
        case CTC_DOT1AE_PORT_MIRRORED_MODE_ENCRYPTED:
            value = 3;
            cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_portLogSelect_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            value = 1;
            cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_dot1AeLogMode_f);
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
            break;
        default:
            return CTC_E_INVALID_PARAM;
    }

    return CTC_E_NONE;
}

int32
sys_usw_dot1ae_get_global_cfg(uint8 lchip, ctc_dot1ae_glb_cfg_t* p_glb_cfg)
{
    uint32 value = 0;
    uint32 value1 = 0;
    uint32 cmd   = 0;
    uint32 overflow_discard = 0;
    uint32 overflow_exception = 0;

    SYS_DOT1AE_INIT_CHECK(lchip);
    CTC_PTR_VALID_CHECK(p_glb_cfg);

    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n",__FUNCTION__);

    cmd = DRV_IOR(EpePktProcCtl_t, EpePktProcCtl_dot1AePnThreshold_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &p_glb_cfg->tx_pn_thrd));

    cmd = DRV_IOR(Dot1AeDecryptCtl_t, Dot1AeDecryptCtl_dot1AePnThreshold_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &p_glb_cfg->rx_pn_thrd));

    cmd = DRV_IOR(EpePktProcCtl_t, EpePktProcCtl_dot1AePnOverflowDiscard_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &overflow_discard));
    cmd = DRV_IOR(EpePktProcCtl_t, EpePktProcCtl_dot1AePnOverflowExceptionEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &overflow_exception));
    if (overflow_discard && overflow_exception)
    {
        p_glb_cfg->tx_pn_overflow_action = CTC_EXCP_DISCARD_AND_TO_CPU;
    }
    else if(!overflow_discard && !overflow_exception)
    {
        p_glb_cfg->tx_pn_overflow_action = CTC_EXCP_NORMAL_FWD;
    }
    else
    {
        p_glb_cfg->tx_pn_overflow_action = CTC_EXCP_DISCARD;
    }

    cmd = DRV_IOR(IpeUserIdCtl_t, IpeUserIdCtl_portLogSelect_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOR(EpeHeaderEditCtl_t, EpeHeaderEditCtl_dot1AeLogMode_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value1));

    if(value == 1 && value1 == 0)
    {
        p_glb_cfg->dot1ae_port_mirror_mode = CTC_DOT1AE_PORT_MIRRORED_MODE_PLAIN_TEXT;
    }
    else if(value == 3 && value1 == 1)
    {
        p_glb_cfg->dot1ae_port_mirror_mode = CTC_DOT1AE_PORT_MIRRORED_MODE_ENCRYPTED;
    }
    else
    {
        return CTC_E_INVALID_PARAM;
    }


    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_INFO, "Dot1AE Encrypt-pn-thrd: %"PRIu64", Decrypt-pn-thrd: %"PRIu64",port_mirror_pkt_type: %u\n",\
                          p_glb_cfg->tx_pn_thrd, p_glb_cfg->rx_pn_thrd, p_glb_cfg->dot1ae_port_mirror_mode);

    return CTC_E_NONE;
}

int32
sys_usw_dot1ae_show_status(uint8 lchip)
{
    uint32 cmd = 0;
    uint32 value = 0;
    uint8 idx = 0;
    uint8 step = 0;
    uint8 rx_chan_count = 0,tx_chan_count = 0;
    sys_dot1ae_chan_t* p_sys_sec_chan = NULL;
    uint8 an_en = 0;
    sys_dot1ae_chan_bind_node_t* p_sec_chan_bind_node = NULL;
    ctc_slistnode_t        *node = NULL;

    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    SYS_DOT1AE_INIT_CHECK(lchip);
    SYS_DOT1AE_LOCK(lchip);


    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "--------------Dot1AE Overall Status ---------------\n");
    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%7s %6s %6s %6s %6s %6s\n","chan_id","Dir","sc-idx","Bound","AN-CFG","AN-InUse");
    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "---------------------------------------------------\n");
    for(idx = 0; idx < SYS_DOT1AE_SEC_CHAN_NUM; idx++)
    {
        p_sys_sec_chan = &usw_dot1ae_master[lchip]->dot1ae_sec_chan[idx];
        if(!p_sys_sec_chan->valid)
        {
            continue;
        }
        if(p_sys_sec_chan->dir == CTC_DOT1AE_SC_DIR_TX )
        {

            step = EpeDot1AeAnCtl0_array_1_currentAn_f - EpeDot1AeAnCtl0_array_0_currentAn_f;
            cmd = DRV_IOR(EpeDot1AeAnCtl0_t, EpeDot1AeAnCtl0_array_0_currentAn_f + step*(p_sys_sec_chan->sc_index<< 2));
            CTC_ERROR_RETURN_WITH_UNLOCK(DRV_IOCTL(lchip, 0, cmd, &value), usw_dot1ae_master[lchip]->sec_mutex);
            CTC_BIT_SET(an_en,value);
            tx_chan_count++;
        }
        else
        {
            an_en = p_sys_sec_chan->an_en;
            rx_chan_count++;
        }
        if (CTC_SLISTCOUNT(p_sys_sec_chan->bind_mem_list))
        {
            CTC_SLIST_LOOP(p_sys_sec_chan->bind_mem_list, node)
            {
                p_sec_chan_bind_node = _ctc_container_of(node, sys_dot1ae_chan_bind_node_t, head);
                SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7u %-6s %-6u 0x%-4x %-6u %-6u\n",\
                p_sys_sec_chan->chan_id,p_sys_sec_chan->dir?"rx":"tx",p_sys_sec_chan->sc_index,\
                p_sec_chan_bind_node->value,p_sys_sec_chan->an_valid,an_en);
            }
        }
        else
        {
            SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "%-7u %-6s %-6u 0x%-4x %-6u %-6u\n",\
            p_sys_sec_chan->chan_id,p_sys_sec_chan->dir?"rx":"tx",p_sys_sec_chan->sc_index,\
            0xFFFF,p_sys_sec_chan->an_valid,an_en);
        }
        an_en = 0;

    }
    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Tx channel number:%d \n",tx_chan_count);
    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_DUMP,"Rx channel number:%d \n",rx_chan_count);
    SYS_DOT1AE_UNLOCK(lchip);
    return CTC_E_NONE;
}
int32
sys_usw_dot1ae_wb_sync(uint8 lchip)
{

    int32 ret = CTC_E_NONE;
    ctc_wb_data_t wb_data;
    sys_wb_dot1ae_master_t*p_wb_dot1ae_master = NULL;
    sys_wb_dot1ae_channel_t  *p_wb_dot1ae_channel = NULL;
    sys_wb_dot1ae_channel_bind_node_t  *p_wb_dot1ae_chan_bind_node = NULL;
    uint8 idx = 0;
    sys_dot1ae_chan_t* p_sys_chan = NULL;
    uint16 max_entry_cnt = 0;
    sys_dot1ae_chan_bind_node_t* p_sec_chan_bind_node = NULL;
    ctc_slistnode_t        *node = NULL;

    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    SYS_USW_FTM_CHECK_NEED_SYNC(lchip);
    /*syncup  dot1ae_matser*/
    wb_data.buffer = mem_malloc(MEM_DOT1AE_MODULE, CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_data.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_data.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);


    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_dot1ae_master_t, CTC_FEATURE_DOT1AE, SYS_WB_APPID_DOT1AE_SUBID_MASTER);

    p_wb_dot1ae_master = (sys_wb_dot1ae_master_t  *)wb_data.buffer;
    p_wb_dot1ae_master->lchip = lchip;
    p_wb_dot1ae_master->version = SYS_WB_VERSION_DOT1AE;

    wb_data.valid_cnt = 1;
    CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);

    /*syncup  dot1ae_channel*/
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_dot1ae_channel_t, CTC_FEATURE_DOT1AE, SYS_WB_APPID_DOT1AE_SUBID_CHANNEL);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data.key_len + wb_data.data_len);
    p_wb_dot1ae_channel = (sys_wb_dot1ae_channel_t  *)wb_data.buffer;
    p_sys_chan = &(usw_dot1ae_master[lchip]->dot1ae_sec_chan[0]);
    for (idx = 0; idx < SYS_DOT1AE_SEC_CHAN_NUM; idx++)
    {
        if (p_sys_chan[idx].valid)
        {
            p_wb_dot1ae_channel->chan_id = p_sys_chan[idx].chan_id;
            p_wb_dot1ae_channel->dir = p_sys_chan[idx].dir;
            p_wb_dot1ae_channel->sc_index = p_sys_chan[idx].sc_index;
            //p_wb_dot1ae_channel->bound = p_sys_chan[idx].bound;
            p_wb_dot1ae_channel->valid = p_sys_chan[idx].valid;
            p_wb_dot1ae_channel->include_sci = p_sys_chan[idx].include_sci;
            p_wb_dot1ae_channel->an_en = p_sys_chan[idx].an_en;
            p_wb_dot1ae_channel->an_valid = p_sys_chan[idx].an_valid;
            p_wb_dot1ae_channel->next_an = p_sys_chan[idx].next_an;
            p_wb_dot1ae_channel->binding_type = p_sys_chan[idx].binding_type;
            //p_wb_dot1ae_channel->gport = p_sys_chan[idx].gport;
            if (p_sys_chan[idx].dir == CTC_DOT1AE_SC_DIR_RX)
            {
                sal_memcpy(p_wb_dot1ae_channel->key, p_sys_chan[idx].key, sizeof(p_wb_dot1ae_channel->key));
            }

            if (++wb_data.valid_cnt == max_entry_cnt)
            {
                CTC_ERROR_RETURN(ctc_wb_add_entry(&wb_data));
                wb_data.valid_cnt = 0;
            }
            p_wb_dot1ae_channel++;
        }
    }
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
    }

    /*syncup  dot1ae_chan_member*/
    CTC_WB_INIT_DATA_T((&wb_data), sys_wb_dot1ae_channel_bind_node_t, CTC_FEATURE_DOT1AE, SYS_WB_APPID_DOT1AE_SUBID_CHAN_BIND_NODE);

    max_entry_cnt = CTC_WB_DATA_BUFFER_LENGTH / (wb_data.key_len + wb_data.data_len);
    p_wb_dot1ae_chan_bind_node = (sys_wb_dot1ae_channel_bind_node_t  *)wb_data.buffer;
    p_sys_chan = &(usw_dot1ae_master[lchip]->dot1ae_sec_chan[0]);
    for (idx = 0; idx < SYS_DOT1AE_SEC_CHAN_NUM; idx++)
    {
        if (p_sys_chan[idx].valid)
        {
            CTC_SLIST_LOOP(p_sys_chan->bind_mem_list, node)
            {
                p_sec_chan_bind_node = _ctc_container_of(node, sys_dot1ae_chan_bind_node_t, head);
                p_wb_dot1ae_chan_bind_node->value = p_sec_chan_bind_node->value;
                p_wb_dot1ae_chan_bind_node->sc_index = p_sys_chan[idx].sc_index;
                p_wb_dot1ae_chan_bind_node->dir = p_sys_chan[idx].dir;
                if (++wb_data.valid_cnt == max_entry_cnt)
                {
                    CTC_ERROR_RETURN(ctc_wb_add_entry(&wb_data));
                    wb_data.valid_cnt = 0;
                }
                p_wb_dot1ae_chan_bind_node++;

            }
        }
    }
    if (wb_data.valid_cnt > 0)
    {
        CTC_ERROR_GOTO(ctc_wb_add_entry(&wb_data), ret, done);
    }

done:
    if (wb_data.buffer)
    {
        mem_free(wb_data.buffer);
    }

    return CTC_E_NONE;
}

int32
sys_usw_dot1ae_wb_restore(uint8 lchip)
{

    uint16 entry_cnt = 0;
    int32 ret = CTC_E_NONE;
    ctc_wb_query_t wb_query;
    sys_wb_dot1ae_master_t  dot1ae_master ;
    sys_dot1ae_chan_t* p_sys_chan = NULL;
    sys_wb_dot1ae_channel_t   wb_dot1ae_channel;
    sys_wb_dot1ae_channel_bind_node_t wb_dot1ae_chan_bind_node;
    sys_usw_opf_t opf;
    uint32 sc_array_idx;
    uint32 key_len = CTC_DOT1AE_KEY_LEN;
    sys_dot1ae_chan_bind_node_t* p_sec_chan_bind_node = NULL;

    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    if(DRV_IS_DUET2(lchip))
    {
        key_len = CTC_DOT1AE_KEY_LEN/2;
    }
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));
    wb_query.buffer = mem_malloc(MEM_DOT1AE_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);
    if (NULL == wb_query.buffer)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);
    /*restore  dot1ae_master*/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_dot1ae_master_t, CTC_FEATURE_DOT1AE, SYS_WB_APPID_DOT1AE_SUBID_MASTER);
    sal_memset(&dot1ae_master, 0, sizeof(sys_wb_dot1ae_master_t));

    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, done);

    sal_memcpy(&dot1ae_master, wb_query.buffer,  wb_query.key_len + wb_query.data_len);

    if (CTC_WB_VERSION_CHECK(SYS_WB_VERSION_DOT1AE, dot1ae_master.version))
    {
        ret = CTC_E_VERSION_MISMATCH;
        goto done;
    }


    /*restore  dot1ae channel entry*/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_dot1ae_channel_t, CTC_FEATURE_DOT1AE, SYS_WB_APPID_DOT1AE_SUBID_CHANNEL);
    sal_memset(&wb_dot1ae_channel, 0, sizeof(sys_wb_dot1ae_channel_t));

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
    sal_memcpy(&wb_dot1ae_channel, (sys_wb_dot1ae_channel_t *)wb_query.buffer+entry_cnt,  wb_query.key_len + wb_query.data_len);
    entry_cnt++;

    sc_array_idx = wb_dot1ae_channel.sc_index + ((wb_dot1ae_channel.dir == CTC_DOT1AE_SC_DIR_RX)?MCHIP_CAP(SYS_CAP_DOT1AE_TX_CHAN_NUM):0);
    p_sys_chan = &(usw_dot1ae_master[lchip]->dot1ae_sec_chan[0]);

    p_sys_chan[sc_array_idx].chan_id = wb_dot1ae_channel.chan_id;
    p_sys_chan[sc_array_idx].dir = wb_dot1ae_channel.dir;
    p_sys_chan[sc_array_idx].sc_index = wb_dot1ae_channel.sc_index;
    p_sys_chan[sc_array_idx].valid = wb_dot1ae_channel.valid;
    p_sys_chan[sc_array_idx].include_sci = wb_dot1ae_channel.include_sci;
    p_sys_chan[sc_array_idx].an_en = wb_dot1ae_channel.an_en;
    p_sys_chan[sc_array_idx].an_valid = wb_dot1ae_channel.an_valid;
    p_sys_chan[sc_array_idx].next_an = wb_dot1ae_channel.next_an;
    p_sys_chan[sc_array_idx].binding_type = wb_dot1ae_channel.binding_type;
    p_sys_chan[sc_array_idx].bind_mem_list = ctc_slist_new();

    if (p_sys_chan[sc_array_idx].dir == CTC_DOT1AE_SC_DIR_RX)
    {
        p_sys_chan[sc_array_idx].key = mem_malloc(MEM_DOT1AE_MODULE, key_len * MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM) * sizeof(uint8));
        sal_memcpy( p_sys_chan[sc_array_idx].key, wb_dot1ae_channel.key, sizeof(wb_dot1ae_channel.key));
    }


    /*recover opf*/
    opf.pool_type = usw_dot1ae_master[lchip]->dot1ae_opf_type;
    if (sc_array_idx < MCHIP_CAP(SYS_CAP_DOT1AE_TX_CHAN_NUM))
    {
        opf.pool_index = 0;
    }
    else
    {
        opf.pool_index = 1;
    }
    CTC_ERROR_RETURN(sys_usw_opf_alloc_offset_from_position(lchip, &opf, 1, wb_dot1ae_channel.sc_index));

    CTC_WB_QUERY_ENTRY_END((&wb_query));

    /*restore  dot1ae channel bind node*/
    CTC_WB_INIT_QUERY_T((&wb_query), sys_wb_dot1ae_channel_bind_node_t, CTC_FEATURE_DOT1AE, SYS_WB_APPID_DOT1AE_SUBID_CHAN_BIND_NODE);
    sal_memset(&wb_dot1ae_chan_bind_node, 0, sizeof(sys_wb_dot1ae_channel_bind_node_t));

    CTC_WB_QUERY_ENTRY_BEGIN((&wb_query));
    sal_memcpy(&wb_dot1ae_chan_bind_node, (sys_wb_dot1ae_channel_bind_node_t *)wb_query.buffer+entry_cnt,  wb_query.key_len + wb_query.data_len);
    entry_cnt++;

    sc_array_idx = wb_dot1ae_chan_bind_node.sc_index + ((wb_dot1ae_chan_bind_node.dir == CTC_DOT1AE_SC_DIR_RX)?MCHIP_CAP(SYS_CAP_DOT1AE_TX_CHAN_NUM):0);
    p_sys_chan = &(usw_dot1ae_master[lchip]->dot1ae_sec_chan[0]);

    p_sec_chan_bind_node = mem_malloc(MEM_DOT1AE_MODULE, sizeof(sys_dot1ae_chan_bind_node_t));
    if (NULL == p_sec_chan_bind_node)
    {
        ret = CTC_E_NO_MEMORY;
        goto done;
    }
    p_sec_chan_bind_node->value = wb_dot1ae_chan_bind_node.value;
    ctc_slist_add_head(p_sys_chan->bind_mem_list, &(p_sec_chan_bind_node->head));

    CTC_WB_QUERY_ENTRY_END((&wb_query));

done:
    if (wb_query.key)
    {
        mem_free(wb_query.key);
    }

    if (wb_query.buffer)
    {
        mem_free(wb_query.buffer);
    }

    return CTC_E_NONE;
}

int32
_sys_usw_dot1ae_get_stats(uint8 lchip, uint32 chan_id, ctc_dot1ae_stats_t* p_stats)
{
    uint32  cmd = 0;
    uint8   an = 0;
    uint32 idx = 0;
    uint64 count = 0;
    uint8 an_stats_idx = 0;
    sys_dot1ae_chan_t* p_chan = NULL;
    DsDot1AeDecryptGlobalStats_m rx_glb;
    DsDot1AeDecryptStats_m  rx_stats;
    DsDot1AeEncryptStats_m  tx_stats;

    sal_memset(&rx_glb, 0, sizeof(rx_glb));
    cmd = DRV_IOR(DsDot1AeDecryptGlobalStats_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_glb));

    p_stats->in_pkts_unknown_sci = GetDsDot1AeDecryptGlobalStats(V, array_0_packetCount_f, &rx_glb);
    p_stats->in_pkts_no_sci = GetDsDot1AeDecryptGlobalStats(V, array_1_packetCount_f, &rx_glb);

    if (1 == SDK_WORK_PLATFORM)
    {
        sal_memset(&rx_glb, 0, sizeof(rx_glb));
        cmd = DRV_IOW(DsDot1AeDecryptGlobalStats_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &rx_glb));
    }

    p_chan = _sys_usw_dot1ae_get_chan_by_id(lchip, chan_id);
    if (p_chan == NULL)
    {
        return CTC_E_NONE;
    }
    for (an = 0; an < MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM); an++)
    {
        if (!CTC_IS_BIT_SET(p_chan->an_valid, an))
        {
            continue;
        }
        if (p_chan->dir == CTC_DOT1AE_SC_DIR_RX )
        {
            for (an_stats_idx = 0; an_stats_idx < 8; an_stats_idx++)
            {
                idx = (((p_chan->sc_index << 2) + (an & 0x3)) << 3) + an_stats_idx;
                sal_memset(&rx_stats, 0, sizeof(rx_stats));
                cmd = DRV_IOR(DsDot1AeDecryptStats_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, idx, cmd, &rx_stats));
                count = GetDsDot1AeDecryptStats(V, packetCount_f, &rx_stats);

                switch(an_stats_idx)
                {
                case 0:
                    p_stats->an_stats[an].in_pkts_unused_sa = count;
                    break;
                case 1:
                    p_stats->an_stats[an].in_pkts_not_using_sa = count;
                    break;
                case 2:
                    p_stats->an_stats[an].in_pkts_late = count;
                    break;
                case 3:
                    p_stats->an_stats[an].in_pkts_not_valid = count;
                    break;
                case 4:
                    p_stats->an_stats[an].in_pkts_invalid = count;
                    break;
                case 5:
                    p_stats->an_stats[an].in_pkts_delayed = count;
                    break;
                case 6:
                    p_stats->an_stats[an].in_pkts_unchecked = count;
                    break;
                case 7:
                    p_stats->an_stats[an].in_pkts_ok = count;
                    break;
                default:
                    break;

                }

                if (1 == SDK_WORK_PLATFORM)
                {
                    sal_memset(&rx_stats, 0, sizeof(rx_stats));
                    cmd = DRV_IOW(DsDot1AeDecryptStats_t, DRV_ENTRY_FLAG);
                    CTC_ERROR_RETURN(DRV_IOCTL(lchip, idx, cmd, &rx_stats));
                }
            }
        }
        else
        {
            idx = (((p_chan->sc_index << 2) + (an & 0x3)) << 1) + 0;
            cmd = DRV_IOR(DsDot1AeEncryptStats_t, DRV_ENTRY_FLAG);

            sal_memset(&tx_stats, 0, sizeof(tx_stats));
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, idx, cmd, &tx_stats));
            p_stats->an_stats[an].out_pkts_protected = GetDsDot1AeEncryptStats(V, packetCount_f, &tx_stats);
            idx++;
            sal_memset(&tx_stats, 0, sizeof(tx_stats));
            CTC_ERROR_RETURN(DRV_IOCTL(lchip, idx, cmd, &tx_stats));
            p_stats->an_stats[an].out_pkts_encrypted = GetDsDot1AeEncryptStats(V, packetCount_f, &tx_stats);

            if (1 == SDK_WORK_PLATFORM)
            {
                sal_memset(&tx_stats, 0, sizeof(tx_stats));
                cmd = DRV_IOW(DsDot1AeEncryptStats_t, DRV_ENTRY_FLAG);
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, idx, cmd, &tx_stats));
                idx--;
                sal_memset(&tx_stats, 0, sizeof(tx_stats));
                CTC_ERROR_RETURN(DRV_IOCTL(lchip, idx, cmd, &tx_stats));
            }
        }
    }


    return CTC_E_NONE;
}

int32
sys_usw_dot1ae_get_stats(uint8 lchip, uint32 chan_id, ctc_dot1ae_stats_t* p_stats)
{
    uint8   an = 0;
    sys_dot1ae_chan_t* p_chan = NULL;
    uint32 sc_array_idx = 0;

    if (DRV_IS_DUET2(lchip))
    {
        return CTC_E_NOT_SUPPORT;
    }
    if (NULL == usw_dot1ae_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }

    SYS_DOT1AE_LOCK(lchip);

    p_chan = _sys_usw_dot1ae_get_chan_by_id(lchip, chan_id);
    if(NULL == p_chan)
    {
        SYS_DOT1AE_UNLOCK(lchip);
        return CTC_E_NOT_EXIST;
    }
    sc_array_idx = p_chan->sc_index  + ((p_chan->dir ==CTC_DOT1AE_SC_DIR_RX)?MCHIP_CAP(SYS_CAP_DOT1AE_TX_CHAN_NUM):0);
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_dot1ae_get_stats(lchip,  chan_id,  p_stats),usw_dot1ae_master[lchip]->sec_mutex);

    for (an = 0; an < MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM); an++)
    {
        p_stats->an_stats[an].out_pkts_protected += usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].out_pkts_protected;
        p_stats->an_stats[an].out_pkts_encrypted += usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].out_pkts_encrypted;
        p_stats->an_stats[an].in_pkts_ok += usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].in_pkts_ok;
        p_stats->an_stats[an].in_pkts_unchecked += usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].in_pkts_unchecked;
        p_stats->an_stats[an].in_pkts_delayed += usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].in_pkts_delayed;
        p_stats->an_stats[an].in_pkts_invalid += usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].in_pkts_invalid;
        p_stats->an_stats[an].in_pkts_not_valid += usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].in_pkts_not_valid;
        p_stats->an_stats[an].in_pkts_late += usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].in_pkts_late;
        p_stats->an_stats[an].in_pkts_not_using_sa += usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].in_pkts_not_using_sa;
        p_stats->an_stats[an].in_pkts_unused_sa += usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].in_pkts_unused_sa;

        usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].out_pkts_protected = p_stats->an_stats[an].out_pkts_protected;
        usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].out_pkts_encrypted = p_stats->an_stats[an].out_pkts_encrypted;
        usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].in_pkts_ok = p_stats->an_stats[an].in_pkts_ok;
        usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].in_pkts_unchecked = p_stats->an_stats[an].in_pkts_unchecked;
        usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].in_pkts_delayed = p_stats->an_stats[an].in_pkts_delayed;
        usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].in_pkts_invalid = p_stats->an_stats[an].in_pkts_invalid;
        usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].in_pkts_not_valid = p_stats->an_stats[an].in_pkts_not_valid;
        usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].in_pkts_late = p_stats->an_stats[an].in_pkts_late;
        usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].in_pkts_not_using_sa = p_stats->an_stats[an].in_pkts_not_using_sa;
        usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an].in_pkts_unused_sa = p_stats->an_stats[an].in_pkts_unused_sa;
    }
    p_stats->in_pkts_no_sci += usw_dot1ae_master[lchip]->global_stats.in_pkts_no_sci;
    p_stats->in_pkts_unknown_sci += usw_dot1ae_master[lchip]->global_stats.in_pkts_unknown_sci;

    usw_dot1ae_master[lchip]->global_stats.in_pkts_no_sci = p_stats->in_pkts_no_sci;
    usw_dot1ae_master[lchip]->global_stats.in_pkts_unknown_sci = p_stats->in_pkts_unknown_sci;

    SYS_DOT1AE_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_dot1ae_clear_stats(uint8 lchip, uint32 chan_id, uint8 an)
{
    ctc_dot1ae_stats_t stats;
    sys_dot1ae_chan_t* p_chan = NULL;
    uint32 sc_array_idx = 0;

    if (DRV_IS_DUET2(lchip))
    {
        return CTC_E_NOT_SUPPORT;
    }
    if (NULL == usw_dot1ae_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }

    /*clear hw*/
    SYS_DOT1AE_LOCK(lchip);
    CTC_ERROR_RETURN_WITH_UNLOCK(_sys_usw_dot1ae_get_stats(lchip,  chan_id,  &stats),usw_dot1ae_master[lchip]->sec_mutex);

    /*clear db*/
    p_chan = _sys_usw_dot1ae_get_chan_by_id(lchip, chan_id);
    if(p_chan)
    {
        sc_array_idx = p_chan->sc_index  + ((p_chan->dir == CTC_DOT1AE_SC_DIR_RX)?MCHIP_CAP(SYS_CAP_DOT1AE_TX_CHAN_NUM):0);
        if (an != 0xFF)
        {
            if (an >= MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM))
            {
                SYS_DOT1AE_UNLOCK(lchip);
                return CTC_E_INVALID_PARAM;
            }
            sal_memset(&usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an], 0 , sizeof(ctc_dot1ae_an_stats_t));
        }
        else
        {
            for (an = 0; an < MCHIP_CAP(SYS_CAP_DOT1AE_AN_NUM); an++)
            {
                sal_memset(&usw_dot1ae_master[lchip]->an_stats[sc_array_idx][an], 0 , sizeof(ctc_dot1ae_an_stats_t));
            }
        }
    }
    else
    {
        usw_dot1ae_master[lchip]->global_stats.in_pkts_no_sci = 0;
        usw_dot1ae_master[lchip]->global_stats.in_pkts_unknown_sci = 0;
    }
    SYS_DOT1AE_UNLOCK(lchip);
    return CTC_E_NONE;
}

int32
sys_usw_dot1ae_sync_dma_stats(uint8 lchip, void* p_data)
{
    uint8 i = 0;
    uint8 an = 0;
    uint8 type = 0;
    uint32* p_addr = NULL;
    sys_dma_reg_t* p_dma_reg = (sys_dma_reg_t*)p_data;

    CTC_PTR_VALID_CHECK(p_data);
    if (DRV_IS_DUET2(lchip))
    {
        return CTC_E_NOT_SUPPORT;
    }
    if (NULL == usw_dot1ae_master[lchip])
    {
        return CTC_E_NOT_INIT;
    }

    type = *((uint8*)p_dma_reg->p_ext);
    p_addr = p_dma_reg->p_data;

    SYS_DOT1AE_LOCK(lchip);
    if(0 == type) /*global*/
    {
        usw_dot1ae_master[lchip]->global_stats.in_pkts_unknown_sci += *(p_addr);
        usw_dot1ae_master[lchip]->global_stats.in_pkts_no_sci += *(p_addr + 1);
    }
    else if(1 == type)/*tx*/
    {
        for (i = 0; i < SYS_DOT1AE_TX_CHAN_NUM; i++)
        {
            if(!usw_dot1ae_master[lchip]->dot1ae_sec_chan[i].valid)
            {
                continue;
            }
            for (an = 0; an < 4; an++)
            {
                usw_dot1ae_master[lchip]->an_stats[i][an].out_pkts_protected += *(p_addr + i*4*2 + an);
                usw_dot1ae_master[lchip]->an_stats[i][an].out_pkts_encrypted += *(p_addr + i*4*2 + an + 1);
            }
        }

    }
    else/*rx*/
    {
        for (i = MCHIP_CAP(SYS_CAP_DOT1AE_TX_CHAN_NUM); i < SYS_DOT1AE_SEC_CHAN_NUM; i++)
        {
            if(!usw_dot1ae_master[lchip]->dot1ae_sec_chan[i].valid)
            {
                continue;
            }
            for (an = 0; an < 4; an++)
            {
                usw_dot1ae_master[lchip]->an_stats[i][an].in_pkts_ok += *(p_addr + i*4*8 + an + 7);
                usw_dot1ae_master[lchip]->an_stats[i][an].in_pkts_unchecked += *(p_addr + i*4*8 + an + 6);
                usw_dot1ae_master[lchip]->an_stats[i][an].in_pkts_delayed += *(p_addr + i*4*8 + an + 5);
                usw_dot1ae_master[lchip]->an_stats[i][an].in_pkts_invalid += *(p_addr + i*4*8 + an + 4);
                usw_dot1ae_master[lchip]->an_stats[i][an].in_pkts_not_valid += *(p_addr + i*4*8 + an + 3);
                usw_dot1ae_master[lchip]->an_stats[i][an].in_pkts_late += *(p_addr + i*4*8 + an + 2);
                usw_dot1ae_master[lchip]->an_stats[i][an].in_pkts_not_using_sa += *(p_addr + i*4*8 + an + 1);
                usw_dot1ae_master[lchip]->an_stats[i][an].in_pkts_unused_sa += *(p_addr + i*4*8 + an);
            }
        }
    }

    SYS_DOT1AE_UNLOCK(lchip);

    return CTC_E_NONE;
}

int32
sys_usw_dot1ae_dump_db(uint8 lchip, sal_file_t p_f,ctc_global_dump_db_t* p_dump_param)
{
    int32 ret = CTC_E_NONE;
    uint32 index = 0;
    uint32 node_idx = 0;
    uint32 an_index = 0;
    sys_dot1ae_chan_bind_node_t* p_sec_chan_bind_node = NULL;
    ctc_slistnode_t        *node = NULL;

    SYS_DOT1AE_INIT_CHECK(lchip);
    SYS_DOT1AE_LOCK(lchip);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "# DOT1AE");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "{");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "Master config:");
    SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "%-30s:\n","dot1ae_sec_chan");
    SYS_DUMP_DB_LOG(p_f, "%-7s%-7s%-4s%-6s%-6s%-8s%-6s%-8s%-9s%-7s\n","Index","Chan","Dir","Sc_id","Valid","Inc_sci","An_en","Next_en","An_valid","B_type");
    for(index = 0; index < SYS_DOT1AE_SEC_CHAN_NUM; index++)
    {
        SYS_DUMP_DB_LOG(p_f, "%-7u%-7u%-4u%-6u%-6u%-8u%-6u%-8u%-9u%-7u\n", index, usw_dot1ae_master[lchip]->dot1ae_sec_chan[index].chan_id, usw_dot1ae_master[lchip]->dot1ae_sec_chan[index].dir, \
        usw_dot1ae_master[lchip]->dot1ae_sec_chan[index].sc_index, usw_dot1ae_master[lchip]->dot1ae_sec_chan[index].valid, \
        usw_dot1ae_master[lchip]->dot1ae_sec_chan[index].include_sci, usw_dot1ae_master[lchip]->dot1ae_sec_chan[index].an_en, usw_dot1ae_master[lchip]->dot1ae_sec_chan[index].next_an, \
        usw_dot1ae_master[lchip]->dot1ae_sec_chan[index].an_valid, usw_dot1ae_master[lchip]->dot1ae_sec_chan[index].binding_type);
    }

    SYS_DUMP_DB_LOG(p_f, "%-30s:\n","dot1ae_sec_chan_bind_node");
    SYS_DUMP_DB_LOG(p_f, "%-7s%-7s%-4s%-6s%-12s\n","Index","Chan","Dir","Sc_id","bound");
    node_idx=0;
    for(index = 0; index < SYS_DOT1AE_SEC_CHAN_NUM; index++)
    {
        CTC_SLIST_LOOP(usw_dot1ae_master[lchip]->dot1ae_sec_chan[index].bind_mem_list, node)
        {
            p_sec_chan_bind_node = _ctc_container_of(node, sys_dot1ae_chan_bind_node_t, head);
            SYS_DUMP_DB_LOG(p_f, "%-7u%-7u%-4u%-6u%-12u\n", node_idx, usw_dot1ae_master[lchip]->dot1ae_sec_chan[index].chan_id, usw_dot1ae_master[lchip]->dot1ae_sec_chan[index].dir, \
                usw_dot1ae_master[lchip]->dot1ae_sec_chan[index].sc_index, p_sec_chan_bind_node->value);
            node_idx++;
        }
    }

    SYS_DUMP_DB_LOG(p_f, "%-30s:\n","an_stats");
    SYS_DUMP_DB_LOG(p_f, "%-6s%-3s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n","Chan","An","out_ptc","Out_ery","In_ok","Inuncheck","Indelay","Invalid","Notvalid","Inlate","Not_sa","Unusedsa");
    for(index = 0; index < SYS_DOT1AE_SEC_CHAN_NUM; index++)
    {
        for (an_index = 0; an_index<4; an_index++)
        {
            SYS_DUMP_DB_LOG(p_f, "%-6u%-3u0x%-8"PRIx64"0x%-8"PRIx64"0x%-8"PRIx64"0x%-8"PRIx64"0x%-8"PRIx64"0x%-8"PRIx64"0x%-8"PRIx64"0x%-8"PRIx64"0x%-8"PRIx64"0x%-8"PRIx64"\n", index, an_index, usw_dot1ae_master[lchip]->an_stats[index][an_index].out_pkts_protected, \
                usw_dot1ae_master[lchip]->an_stats[index][an_index].out_pkts_encrypted, usw_dot1ae_master[lchip]->an_stats[index][an_index].in_pkts_ok, usw_dot1ae_master[lchip]->an_stats[index][an_index].in_pkts_unchecked, \
                usw_dot1ae_master[lchip]->an_stats[index][an_index].in_pkts_delayed, usw_dot1ae_master[lchip]->an_stats[index][an_index].in_pkts_invalid, usw_dot1ae_master[lchip]->an_stats[index][an_index].in_pkts_not_valid, \
                usw_dot1ae_master[lchip]->an_stats[index][an_index].in_pkts_late, usw_dot1ae_master[lchip]->an_stats[index][an_index].in_pkts_not_using_sa, usw_dot1ae_master[lchip]->an_stats[index][an_index].in_pkts_unused_sa);
        }
    }
    SYS_DUMP_DB_LOG(p_f, "%-30s:\n","global_stats");
    SYS_DUMP_DB_LOG(p_f, "%-30s:0x%"PRIx64"\n","in pkts no sci", usw_dot1ae_master[lchip]->global_stats.in_pkts_no_sci);
    SYS_DUMP_DB_LOG(p_f, "%-30s:0x%"PRIx64"\n","in pkts unknown sci", usw_dot1ae_master[lchip]->global_stats.in_pkts_unknown_sci);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "-----------------------------------------------------------------------------------");
    SYS_DUMP_DB_LOG(p_f, "\n");
    sys_usw_opf_fprint_alloc_used_info(lchip, usw_dot1ae_master[lchip]->dot1ae_opf_type, p_f);
    SYS_DUMP_DB_LOG(p_f, "%s\n", "}");

    SYS_DOT1AE_UNLOCK(lchip);

    return ret;
}

STATIC int32
_sys_usw_dot1ae_opf_init(uint8 lchip)
{
    sys_usw_opf_t opf;

    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    /*Dot1AE chan num (encrypt chan and decrypt chan) chan:0 Reserved for do nothing  */
    sal_memset(&opf, 0, sizeof(sys_usw_opf_t));
    CTC_ERROR_RETURN(sys_usw_opf_init(lchip, &usw_dot1ae_master[lchip]->dot1ae_opf_type, 2, "opf-dot1ae-chan"));
    opf.pool_type = usw_dot1ae_master[lchip]->dot1ae_opf_type;
    opf.pool_index = 0;      /*tx chan*/
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 0, MCHIP_CAP(SYS_CAP_DOT1AE_TX_CHAN_NUM)));
    opf.pool_index = 1;      /*rx chan*/
    CTC_ERROR_RETURN(sys_usw_opf_init_offset(lchip, &opf, 0, MCHIP_CAP(SYS_CAP_DOT1AE_RX_CHAN_NUM)));

    return CTC_E_NONE;
}

int32
sys_usw_dot1ae_init(uint8 lchip)
{
    int32  ret = CTC_E_NONE;
    uint32 cmd = 0;
    uint32 value = 0;
    uint16 lport = 0;
    uint32 dot1ae_enable = 0;
    uint8  gchip_id = 0;
    uint32 bypass_nh_offset = 0;
    uint32 fwd_offset = 0;
    EpePktProcCtl_m    epe_pkt_ctl;
    Dot1AeDecryptCtl_m decrypt_ctl;
    IpeIntfMapperCtl_m intf_ctl;
    BufRetrvEpeChanSwapCtl_m epe_chan_swap_ctl;
    sys_nh_param_dsfwd_t dsfwd_param;
    uint8 dot1ae_en = 0;
    uint8 unused_chan = 0;
    uint8 work_status = 0;

    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);

    sys_usw_ftm_get_working_status(lchip, &work_status);
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_DONE);
    }

    CTC_ERROR_RETURN(sys_usw_dmps_get_port_property(lchip, 0, SYS_DMPS_PORT_PROP_DOT1AE_ENABLE, (void *)&dot1ae_enable));
    if(0 == dot1ae_enable)
    {
        return CTC_E_NONE;
    }


    if (NULL != usw_dot1ae_master[lchip])
    {
        return CTC_E_NONE;
    }


    usw_dot1ae_master[lchip] = (sys_dot1ae_master_t*)mem_malloc(MEM_DOT1AE_MODULE, sizeof(sys_dot1ae_master_t));
    if (NULL == usw_dot1ae_master[lchip])
    {
        SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_ERROR, " No memory \n");
        return CTC_E_NO_MEMORY;

    }

    sal_memset(usw_dot1ae_master[lchip], 0, sizeof(sys_dot1ae_master_t));

    sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));
    sal_memset(&epe_pkt_ctl, 0, sizeof(EpePktProcCtl_m));
    sal_memset(&decrypt_ctl, 0, sizeof(Dot1AeDecryptCtl_m));
    sal_memset(&intf_ctl,    0, sizeof(IpeIntfMapperCtl_m));
    sal_memset(&epe_chan_swap_ctl, 0, sizeof(BufRetrvEpeChanSwapCtl_m));

    /* set register cb api */
    CTC_ERROR_GOTO(sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_DOT1AE_BIND_SEC_CHAN, sys_usw_dot1ae_bind_sec_chan), ret, ERROR_FREE);
    CTC_ERROR_GOTO(sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_DOT1AE_UNBIND_SEC_CHAN, sys_usw_dot1ae_unbind_sec_chan), ret, ERROR_FREE);
    CTC_ERROR_GOTO(sys_usw_register_api_cb(lchip, SYS_REGISTER_CB_TYPE_DOT1AE_GET_BIND_SEC_CHAN, sys_usw_dot1ae_get_bind_sec_chan), ret, ERROR_FREE);

    CTC_ERROR_GOTO(_sys_usw_dot1ae_opf_init(lchip), ret, ERROR_FREE);
#if 0
    IpeChanCfgMsg_m ipe_chan_cfg;
    sal_memset(&ipe_chan_cfg, 0, sizeof(IpeChanCfgMsg_m));
    cmd = DRV_IOR(IpeChanCfgMsg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_chan_cfg));
    SetIpeChanCfgMsg(V, cfgDot1AeDecryptChanId_f, &ipe_chan_cfg, MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT));
    SetIpeChanCfgMsg(V, cfgDot1AeEncryptChanId_f, &ipe_chan_cfg, MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT));
    cmd = DRV_IOW(IpeChanCfgMsg_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &ipe_chan_cfg));
#endif

    /* IpeUserIdCtl */
/*
portLogEligible = (IpeUserIdCtl.portLogSelect(1,0) == CBit(2, 'b', "00", 2))
              || ((IpeUserIdCtl.portLogSelect(1,0) == CBit(2, 'b', "01", 2)) && (ParserResult.layer3Type(3,0) != L3TYPE_DOT1AE))
              || ((IpeUserIdCtl.portLogSelect(1,0) == CBit(2, 'b', "10", 2)) && PacketInfo.dot1AeEn && (!PacketInfo.dot1AeEncrypted) && (ParserResult.layer3Type(3,0) == L3TYPE_DOT1AE))
              || ((IpeUserIdCtl.portLogSelect(1,0) == CBit(2, 'b', "11", 2)) && ((!PacketInfo.dot1AeEn) || (!PacketInfo.dot1AeEncrypted && (ParserResult.layer3Type(3,0) == L3TYPE_DOT1AE))));
*/
    /* NetRxMiscChanPktLenChkCtl */

    if (DRV_IS_DUET2(lchip))
    {
        value = 7;
        cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMinLenSelId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 9, cmd, &value));
        cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMinLenSelId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 10, cmd, &value));

        cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMaxLenSelId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 9, cmd, &value));
        cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMaxLenSelId_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 10, cmd, &value));
    }
    else
    {
        value = SYS_USW_ILOOP_MIN_FRAMESIZE_DEFAULT_VALUE;
        cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMinLen_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 9, cmd, &value));
        cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMinLen_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 10, cmd, &value));

        value = SYS_USW_MAX_FRAMESIZE_MAX_VALUE;
        cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMaxLen_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 9, cmd, &value));
        cmd = DRV_IOW(NetRxMiscChanPktLenChkCtl_t, NetRxMiscChanPktLenChkCtl_cfgMiscMaxLen_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 10, cmd, &value));
    }

    /* IpeUserIdCtl */
    value = 1;
    cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_unknownDot1AeSciDiscard_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    value = 1;
    cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_unknownDot1AePacketDiscard_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    value = 1;
    cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_portLogSelect_f);   /*rx, only mirror plain pkt*/
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    /* EpeHeaderEditCtl */
    value = 0;
    cmd = DRV_IOW(EpeHeaderEditCtl_t, EpeHeaderEditCtl_dot1AeLogMode_f);   /*tx, only mirror plain pkt*/
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    /*MacSecDecryptCtl*/
    value = 1;
    cmd = DRV_IOW(MacSecDecryptCtl_t, MacSecDecryptCtl_cipherModeMismatchDiscard_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    value = 1;
    cmd = DRV_IOW(MacSecDecryptCtl_t, MacSecDecryptCtl_hostSciLowBits_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    /* IpeIntfMapperCtl */
    cmd = DRV_IOR(IpeIntfMapperCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &intf_ctl));

    SetIpeIntfMapperCtl(V, dot1AeDisableLearning_f, &intf_ctl, 1);
    SetIpeIntfMapperCtl(V, bypassPortCrossConnectDisable_f, &intf_ctl, 1);

    cmd = DRV_IOW(IpeIntfMapperCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &intf_ctl));

    /* EpePktProcCtl */
    cmd = DRV_IOR(EpePktProcCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_pkt_ctl));
    SetEpePktProcCtl(V, dot1AePnThreshold_f, &epe_pkt_ctl, 0xc0000000);
    SetEpePktProcCtl(V, dot1AePnMatchSwitchEn_f,    &epe_pkt_ctl, 1);
    SetEpePktProcCtl(V, dot1AePnMatchExceptionEn_f, &epe_pkt_ctl, 1);
    SetEpePktProcCtl(V, dot1AePnOverflowDiscard_f, &epe_pkt_ctl, 1);
    SetEpePktProcCtl(V, dot1AePnOverflowExceptionEn_f, &epe_pkt_ctl, 1);
    SetEpePktProcCtl(V, dot1AeXpnEn_f, &epe_pkt_ctl, 1);

    cmd = DRV_IOW(EpePktProcCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_pkt_ctl));

    /* Dot1AeDecryptCtl */
    cmd = DRV_IOR(Dot1AeDecryptCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &decrypt_ctl));
    SetDot1AeDecryptCtl(V, dot1AePnThreshold_f, &decrypt_ctl, 0xc0000000);
    SetDot1AeDecryptCtl(V, dot1AePnThresholdException_f, &decrypt_ctl, 1);

    cmd = DRV_IOW(Dot1AeDecryptCtl_t, DRV_ENTRY_FLAG);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &decrypt_ctl));

    /* IpeE2iLoopCtl */
    value = 0;
    cmd = DRV_IOW(IpeE2iLoopCtl_t, IpeE2iLoopCtl_dot1AeDecryptPortControlEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    value = 1;
    cmd = DRV_IOW(IpeE2iLoopCtl_t, IpeE2iLoopCtl_dot1AeEncryptPortControlEn_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    cmd = DRV_IOW(IpeE2iLoopCtl_t, IpeE2iLoopCtl_dot1AeEncryptedLoopbackPort_f);
    CTC_ERROR_RETURN(sys_usw_internal_port_get_rsv_port(lchip, SYS_INTERNAL_PORT_TYPE_WLAN_ENCAP, &lport));
    value = lport;
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    /* QWriteDot1AeCtl */
    value = MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT);
    cmd = DRV_IOW(QWriteDot1AeCtl_t, QWriteDot1AeCtl_dot1AeEncryptChannel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));
    value = MCHIP_CAP(SYS_CAP_CHANID_MAC_DECRYPT);
    cmd = DRV_IOW(QWriteDot1AeCtl_t, QWriteDot1AeCtl_dot1AeDecryptChannel_f);
    CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && (work_status != CTC_FTM_MEM_CHANGE_RECOVER))
    {
        cmd = DRV_IOR(IpeUserIdCtl_t, IpeUserIdCtl_dot1AeLoopbackPtr_f);
        DRV_IOCTL(lchip, 0, cmd, &fwd_offset);
        CTC_ERROR_RETURN(sys_usw_nh_alloc_from_position(lchip, SYS_NH_ENTRY_TYPE_FWD, 2, fwd_offset));
    }
    else
    {
        CTC_ERROR_RETURN(sys_usw_nh_alloc(lchip, SYS_NH_ENTRY_TYPE_FWD, 2, &fwd_offset));
    }
    /* write dsfwd */
    CTC_ERROR_GOTO(sys_usw_get_gchip_id(lchip, &gchip_id), ret, DSFWD_ERROR);
    CTC_ERROR_GOTO(sys_usw_internal_port_get_rsv_port(lchip,
                                                SYS_INTERNAL_PORT_TYPE_WLAN_E2ILOOP, &lport), ret, DSFWD_ERROR);

    dsfwd_param.dest_chipid = gchip_id;
    dsfwd_param.dest_id = lport;
    dsfwd_param.dsfwd_offset = fwd_offset;
    dsfwd_param.dsnh_offset = SYS_DSNH_INDEX_FOR_NONE;

    CTC_ERROR_GOTO(sys_usw_nh_add_dsfwd(lchip, &dsfwd_param), ret, DSFWD_ERROR);

    CTC_ERROR_RETURN(sys_usw_nh_get_resolved_offset(lchip, SYS_NH_RES_OFFSET_TYPE_BYPASS_NH, &bypass_nh_offset));

    sal_memset(&dsfwd_param, 0, sizeof(sys_nh_param_dsfwd_t));

    dsfwd_param.dest_chipid = gchip_id;
    dsfwd_param.dsfwd_offset = fwd_offset + 1;
    dsfwd_param.dsnh_offset = bypass_nh_offset;

    CTC_ERROR_GOTO(sys_usw_nh_add_dsfwd(lchip, &dsfwd_param), ret, DSFWD_ERROR);

    cmd = DRV_IOW(IpeUserIdCtl_t, IpeUserIdCtl_dot1AeLoopbackPtr_f);
    CTC_ERROR_GOTO(DRV_IOCTL(lchip, 0, cmd, &fwd_offset), ret, DSFWD_ERROR);

    /*init sw*/
    sal_memset(usw_dot1ae_master[lchip]->dot1ae_sec_chan, 0, SYS_DOT1AE_SEC_CHAN_NUM * sizeof(sys_dot1ae_chan_t));
    CTC_ERROR_GOTO(sys_usw_dma_register_cb(lchip, SYS_DMA_CB_TYPE_DOT1AE_STATS, sys_usw_dot1ae_sync_dma_stats), ret, DSFWD_ERROR);

    SYS_DOT1AE_CREATE_LOCK(lchip);
    CTC_ERROR_GOTO(sys_usw_wb_sync_register_cb(lchip, CTC_FEATURE_DOT1AE, sys_usw_dot1ae_wb_sync), ret, DSFWD_ERROR);
    CTC_ERROR_GOTO(sys_usw_dump_db_register_cb(lchip, CTC_FEATURE_DOT1AE, sys_usw_dot1ae_dump_db), ret, DSFWD_ERROR);

    if (CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING)
    {
        CTC_ERROR_RETURN(sys_usw_dot1ae_wb_restore(lchip));
    }

    /*CTC_ERROR_RETURN(sys_usw_datapath_get_dot1ae_en(lchip, &dot1ae_en));*/
    if (dot1ae_en && DRV_IS_DUET2(lchip))
    {
        /*TBD-CTC_ERROR_RETURN(sys_usw_datapath_get_dot1ae_enq_chan(lchip, &unused_chan));*/

        for (lport = 0; lport < MCHIP_CAP(SYS_CAP_NETWORK_CHANNEL_NUM); lport++)
        {
            CTC_ERROR_RETURN(sys_usw_qos_add_extend_port_to_channel(lchip, lport, unused_chan));
        }

        value = MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT);
        cmd = DRV_IOW(EpeGlobalChannelMap_t, EpeGlobalChannelMap_macEncryptChannel_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        value = unused_chan;
        cmd = DRV_IOW(QWriteDot1AeCtl_t, QWriteDot1AeCtl_dot1AeEncryptChannel_f);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &value));

        /* BufRetrvEpeChanSwapCtl */
        cmd = DRV_IOR(BufRetrvEpeChanSwapCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_chan_swap_ctl));
        SetBufRetrvEpeChanSwapCtl(V, epeChanSwapEn_f, &epe_chan_swap_ctl, 1);
        SetBufRetrvEpeChanSwapCtl(V, epeOriginalChan_f, &epe_chan_swap_ctl, unused_chan);
        SetBufRetrvEpeChanSwapCtl(V, epeSwapedChan_f, &epe_chan_swap_ctl, MCHIP_CAP(SYS_CAP_CHANID_MAC_ENCRYPT));
        cmd = DRV_IOW(BufRetrvEpeChanSwapCtl_t, DRV_ENTRY_FLAG);
        CTC_ERROR_RETURN(DRV_IOCTL(lchip, 0, cmd, &epe_chan_swap_ctl));

    }
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    return CTC_E_NONE;

DSFWD_ERROR:
    sys_usw_nh_free(lchip, SYS_NH_ENTRY_TYPE_FWD, 2, fwd_offset);

ERROR_FREE :

    if (NULL != usw_dot1ae_master[lchip])
    {
        mem_free(usw_dot1ae_master[lchip]);
    }
    if(CTC_WB_ENABLE && CTC_WB_STATUS(lchip) == CTC_WB_STATUS_RELOADING && work_status == CTC_FTM_MEM_CHANGE_RECOVER)
    {
    	drv_set_warmboot_status(lchip, CTC_WB_STATUS_RELOADING);
    }
    return ret;
}

int32
sys_usw_dot1ae_deinit(uint8 lchip)
{

    SYS_DOT1AE_DBG_OUT(CTC_DEBUG_LEVEL_FUNC, "%s()\n", __FUNCTION__);
    LCHIP_CHECK(lchip);
    if (NULL == usw_dot1ae_master[lchip])
    {
        return CTC_E_NONE;
    }

    sal_mutex_destroy(usw_dot1ae_master[lchip]->sec_mutex);

    /*free opf*/
    sys_usw_opf_free(lchip, usw_dot1ae_master[lchip]->dot1ae_opf_type, 0);
    sys_usw_opf_free(lchip, usw_dot1ae_master[lchip]->dot1ae_opf_type, 1);
    sys_usw_opf_deinit(lchip, usw_dot1ae_master[lchip]->dot1ae_opf_type);
    /*deinit master*/

    mem_free(usw_dot1ae_master[lchip]);

    return CTC_E_NONE;
}


#endif

