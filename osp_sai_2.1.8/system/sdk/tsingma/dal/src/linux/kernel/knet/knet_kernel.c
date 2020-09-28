/**
 @file knet_kernel.c

 @date 2012-10-18

 @version v2.0


*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/types.h>
#include <asm/io.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <asm/irq.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/semaphore.h>
#include <linux/dma-mapping.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/skbuff.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
#include <linux/irqdomain.h>
#endif
#include "../dal_kernel.h"
#include "dal_common.h"
#include "dal_mpool.h"
#include "knet_kernel.h"
MODULE_AUTHOR("Centec Networks Inc.");
MODULE_DESCRIPTION("KNET kernel module");
MODULE_LICENSE("GPL");

/* DMA memory pool size */
static char* dma_pool_size;
module_param(dma_pool_size, charp, 0);

/*****************************************************************************
 * defines
 *****************************************************************************/
#define SUP_CTL_INTR_VEC_ADDR   0x00011040
#define DMA_CTL_INTR_FUNC_ADDR    0x00028ea0
#define DMA_CTL_TAB_ADDR    0x00028b80

#define DAL_MAX_CHIP_NUM   8
#define SUP_INTR_DMA   27

#define INTR_INDEX_VAL_SET      0
#define INTR_INDEX_VAL_RESET    1
#define INTR_INDEX_MASK_SET     2
#define INTR_INDEX_MASK_RESET   3
#define INTR_INDEX_MAX          4

#define PACKET_HEADER_LEN  40

#define PORT_RSV_PORT_ILOOP_ID    252
#define NEXTHOP_RES_BYPASS_NH_PTR   0xFFFE

#define PACKET_OTHER_FROMCPU_BIT_OFFSET (4*32 + 8)     /*(4, 8, 1)*/
#define PACKET_TTL_BIT_OFFSET (4*32 + 19)     /*(4, 19, 8)*/

#define PACKET_MACKNOWN_BIT_OFFSET (5*32 + 24)     /*(5, 24, 1)*/

#define PACKET_FROMCPUOROAM_BIT_OFFSET (6*32 + 17)     /*(6, 17, 1)*/

#define PACKET_BYPASSINGRESSEDIT_BIT_OFFSET (6*32 + 22)     /*(6, 22, 1)*/
#define PACKET_NEXTHOPEXT_BIT_OFFSET (6*32 + 28)     /*(6, 28, 1)*/
#define PACKET_NEXTHOPPTR_BIT_OFFSET     (7*32 + 1)     /*(7, 1, 18)*/

#define PACKET_COLOR_BIT_OFFSET (7*32 + 19)     /*(7, 19, 2)*/

#define PACKET_STAGACTION_BIT_OFFSET (5*32 + 25)     /*(5, 25, 2)*/
#define PACKET_SVLANTAG_OPVALID_BIT_OFFSET (6*32 + 25)     /*(6, 25, 1)*/
#define PACKET_SVLAN_BIT_OFFSET     (7*32 + 21)     /*(7, 21, 11), (8, 0, 1)*/

#define PACKET_CRITICALPACKET_BIT_OFFSET (6*32 + 31)     /*(6, 31, 1)*/
#define PACKET_SRCPORT_BIT_OFFSET     (8*32 + 1)     /*(8, 1, 16)*/
#define PACKET_DESTMAP_BIT_OFFSET     (9*32 + 5)     /*(9, 5, 19)*/

#define PACKET_HEADERHASH_BIT_OFFSET     (8*32 + 18)     /*(8, 18, 8)*/

#define PACKET_PRIO_BIT_OFFSET     (9*32 + 0)     /*(9, 0, 4)*/

#define PACKET_GPORT_TO_GCHIP(gport)         (((gport) >> 8)&0x7F)
#define PACKET_ENCODE_LPORT_TO_DESTMAP(gchip,lport)        ((((gchip)&0x7F) << 9) | ((lport)&0x1FF))
#define PACKET_ENCODE_GPORT_TO_DESTMAP(gport)        ((PACKET_GPORT_TO_GCHIP(gport) << 9) | (((((gport) >> (8 + 7) & 7) << 8) | ((gport) & 0xFF)) &0x1FF))
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
#include <linux/slab.h>
#define virt_to_bus virt_to_phys
#define bus_to_virt phys_to_virt
#endif
/*****************************************************************************
 * typedef
 *****************************************************************************/
struct net_device_priv_s {
    struct list_head list;
    struct net_device_stats stats;
    struct net_device *dev;
    u8 netif_id;
    u8 lchip;
    u8 type;
    u16 vlan;
    u32 gport;
    u32 flags;
} ;
typedef struct net_device_priv_s net_device_priv_t;

/***************************************************************************
 *declared
 ***************************************************************************/

/*****************************************************************************
 * global variables
 *****************************************************************************/
static int knet_version = VERSION_1DOT0;
static dal_dma_info_t knet_dma_info[DAL_MAX_CHIP_NUM];
static dal_dma_chan_t *knet_dma_chan[DAL_MAX_CHIP_NUM][KNET_MAX_RX_CHAN_NUM+KNET_MAX_TX_CHAN_NUM];
spinlock_t knet_dma_chan_lock[DAL_MAX_CHIP_NUM][KNET_MAX_RX_CHAN_NUM+KNET_MAX_TX_CHAN_NUM];
struct list_head knet_dev_list;
spinlock_t knet_dev_lock;
static struct task_struct *knet_rx_thread = NULL;
static wait_queue_head_t poll_desc;
static u8 knet_rx_start = 0;
static u8 knet_dma_done[DAL_MAX_CHIP_NUM][KNET_MAX_RX_CHAN_NUM+KNET_MAX_TX_CHAN_NUM];
struct semaphore rx_sem;
static struct class *knet_class;
static dal_mpool_mem_t* knet_dma_pool[DAL_MAX_CHIP_NUM];

static int knet_debug = 0;
module_param(knet_debug, int, 0);
MODULE_PARM_DESC(knet_debug, "Set debug level (default 0)");

/*****************************************************************************
 * macros
 *****************************************************************************/
STATIC int
_knet_packet_dump(u8* data, u32 len)
{
    u32 cnt = 0;
    char line[256] = {'\0'};
    char tmp[32] = {'\0'};

    if (0 == len)
    {
        return 0;
    }

    for (cnt = 0; cnt < len; cnt++)
    {
        if ((cnt % 16) == 0)
        {
            if (cnt != 0)
            {
                printk("%s", line);
            }

            memset(line, 0, sizeof(line));
            if (cnt == 0)
            {
                sprintf(tmp, "0x%04x:  ", cnt);
            }
            else
            {
                sprintf(tmp, "\n0x%04x:  ", cnt);
            }
            strcat(line, tmp);
        }

        sprintf(tmp, "%02x", data[cnt]);
        strcat(line, tmp);

        if ((cnt % 2) == 1)
        {
            strcat(line, " ");
        }
    }

    printk("%s", line);
    printk("\n");

    return 0;
}

u8
_knet_data_calc_crc8(u8* data, u32 len, u32 init_crc)
{
    u32 crc = 0;
    u32 i = 0;
    u32 msb_bit = 0;
    u32 poly_mask = 0;
    u32 topone=0;
    u32 poly = 0x107;
    u32 poly_len = 8;
    u32 bit_len = len * 8;

    if (NULL == data)
    {
        return 0;
    }

    poly_mask = (1<<poly_len) -1;
    topone = (1 << (poly_len-1));
    crc = init_crc & poly_mask;

    for (i = 0; i < bit_len; i++)
    {
        msb_bit = ((data[i / 8] >> (7 - (i % 8))) & 0x1) << (poly_len - 1);
        crc ^= msb_bit;
        crc = (crc << 1) ^ ((crc & topone) ? poly : 0);
    }

    crc &= poly_mask;
    return crc;
}

STATIC int
_knet_convert_header(u32* dest, u32* src, u32 int_len)
{
    int i = 0;

    for (i = 0; i < int_len; i++)
    {
        *(dest + i) = *(src + (int_len - i - 1));
    }

    for (i = 0; i < int_len; i++)
    {
        dest[i] = ntohl(dest[i]);
    }

    return 0;
}

STATIC net_device_priv_t *
_knet_netif_lookup(dal_netif_t *netif)
{
    struct list_head *list;
    net_device_priv_t *priv = NULL;
    unsigned long flags;
    u8 find = 0;

    spin_lock_irqsave(&knet_dev_lock, flags);
    list_for_each(list, &knet_dev_list)
    {
        priv = (net_device_priv_t *)list;
        if (netif->type != priv->type)
        {
            continue;
        }

        if ((netif->type == DAL_NETIF_T_PORT) &&
            (netif->gport == priv->gport))
        {
            find = 1;
            break;
        }
        else if ((netif->type == DAL_NETIF_T_VLAN) &&
            (netif->vlan == priv->vlan))
        {
            find = 1;
            break;
        }
    }
    spin_unlock_irqrestore(&knet_dev_lock, flags);

    return find ? priv : NULL;
}

STATIC int
_knet_packet_rx(u32 lchip, void *logic_addr, u32 pkt_len)
{
    net_device_priv_t *priv = NULL;
    struct sk_buff *skb;
    u32 bheader[PACKET_HEADER_LEN/4] = {0};
    u32 *p_raw_hdr = bheader;
    dal_netif_t tmp_netif = {0};
    int ret = 0;

    _knet_convert_header(p_raw_hdr, (u32*)logic_addr, PACKET_HEADER_LEN / 4);

    tmp_netif.vlan = ((*(p_raw_hdr + PACKET_SVLAN_BIT_OFFSET/32 + 1) & 0x1) << 11) | ((*(p_raw_hdr + PACKET_SVLAN_BIT_OFFSET/32) >> 21) & 0x7FF);
    tmp_netif.gport = (*(p_raw_hdr + PACKET_SRCPORT_BIT_OFFSET/32) >> 1) & 0xFFFF;

    //printk("KNET: recieve packet, src gport: %u, src vlan: %u.\n", netif_rx.src_port, netif_rx.src_vlan);

    //_knet_packet_dump(logic_addr, pkt_len);

    tmp_netif.type = DAL_NETIF_T_PORT;
    priv = _knet_netif_lookup(&tmp_netif);
    if (!priv)
    {
        tmp_netif.type = DAL_NETIF_T_VLAN;
        priv = _knet_netif_lookup(&tmp_netif);
        if (!priv)
        {
            return -1;
        }
    }
    //printk("KNET: recieve packet from netif %s, gport: 0x%x, vlan: %u.\n", priv->dev->name, priv->gport, priv->vlan);

    priv->stats.rx_packets++;
    priv->stats.rx_bytes += pkt_len - PACKET_HEADER_LEN;

    skb = dev_alloc_skb(pkt_len - PACKET_HEADER_LEN + 2);
    if (!skb)
    {
        return -1;
    }
    skb_reserve(skb, 2);

    memcpy(skb->data, logic_addr + PACKET_HEADER_LEN, pkt_len - PACKET_HEADER_LEN);
    skb_put(skb, pkt_len - PACKET_HEADER_LEN);

    skb->protocol = eth_type_trans(skb, priv->dev);

    //printk("start netif_rx from dev: %s, ifindex: %d, mtu: %d!\n", skb->dev->name, skb->dev->ifindex, skb->dev->mtu);
    ret = netif_rx(skb);
    if (ret)
    {
        printk("send packet to network stack error.\n");
    }

    return 0;
}

int _knet_dma_pkt_rx_func(u8 lchip, u8 chan_id)
{
    int ret = 0;
    DsDesc_t* p_desc = NULL;
    u32 cur_index = 0;
    u32 sop_index = 0;
    u32 buf_count = 0;
    u64 phy_addr = 0;
    #ifndef DUET2
    u64 temp_addr = 0;
    #endif
    void *logic_addr = 0;
    u32 process_cnt = 0;
    u32 is_sop = 0;
    u32 is_eop = 0;
    u8 need_eop = 0;
    u32 wait_cnt = 0;
    u32 pkt_len = 0;
    u32 desc_done = 0;
    u32 knet_done = 0;
    knet_dma_desc_t* p_base_desc = (knet_dma_desc_t*)knet_dma_chan[lchip][chan_id]->virt_base;

    cur_index = knet_dma_chan[lchip][chan_id]->current_index;
    for (;; cur_index++)
    {
        if (cur_index >= knet_dma_chan[lchip][chan_id]->desc_depth)
        {
            cur_index = 0;
        }

        p_desc = &(p_base_desc[cur_index].desc_info);
        dal_cache_inval(virt_to_bus(p_desc), sizeof(DsDesc_t));

        desc_done = p_desc->done;
        knet_done = p_desc->rsv0;

        if (0 == desc_done)
        {
            if (need_eop)
            {
                printk("Desc not done, But need eop!!cur_index %d\n", cur_index);

                while(wait_cnt < 0xffff)
                {
                    dal_cache_inval(virt_to_bus(p_desc), sizeof(DsDesc_t));
                    desc_done = p_desc->done;
                    knet_done = p_desc->rsv0;
                    if (desc_done)
                    {
                        break;
                    }
                    wait_cnt++;
                }

                /* Cannot get EOP, means no EOP packet error, just clear desc*/
                if (wait_cnt >= 0xffff)
                {
                   printk("No EOP, cur_index %d, buf_count %d\n", cur_index, buf_count);
                   buf_count = 0;
                   need_eop = 0;
                   break;
                }
                wait_cnt = 0;
            }
            else
            {
                printk("No desc is not done, process count %d cur_index %d\n", process_cnt, cur_index);
                break;
            }
        }

        if (knet_done)
        {
            printk("Wait user process cur_index %d.\n", cur_index);
            break;
        }

        process_cnt++;

        is_sop = p_desc->u1_pkt_sop;
        is_eop = p_desc->u1_pkt_eop;

        /*Before get EOP, next packet SOP come, no EOP packet error, drop error packet */
        if (need_eop && is_sop)
        {
            printk("PKT RX error, chan_id %d need eop, sop_index %d, buf_count %d\n", chan_id, sop_index, buf_count);
            buf_count = 0;
            need_eop = 0;
        }

        /* Cannot get SOP, means no SOP packet error, just clear desc*/
        if (0 == buf_count)
        {
            if (0 == is_sop)
            {
                printk("PKT RX error, chan_id %d index %d first is not sop\n", chan_id, cur_index);
                goto error;
            }
        }

        if (is_sop)
        {
            sop_index = cur_index;
            pkt_len = 0;
            need_eop = 1;
        }

        phy_addr = knet_dma_info[lchip].phy_base_hi;
        #ifdef DUET2
        phy_addr = (phy_addr << 32) | ntohl(p_desc->memAddr);
        pkt_len += ntohs(p_desc->realSize);   
        #else
        temp_addr = p_desc->memAddr;
        temp_addr = (temp_addr<<4);
        phy_addr = (phy_addr << 32) | temp_addr;
        pkt_len += p_desc->realSize;
        #endif
        logic_addr = bus_to_virt(phy_addr);
        buf_count++;

        if (is_eop)
        {
            //printk("[DMA] PKT RX lchip %d dma_chan %d buf_count %d pkt_len %d curr_index %d\n",
                            //lchip, chan_id, buf_count, pkt_len, cur_index);

            _knet_packet_rx(lchip, logic_addr, pkt_len);

            buf_count = 0;
            need_eop = 0;
        }

error:
        p_desc->rsv0 = 1;
        dal_cache_flush(virt_to_bus(p_desc), sizeof(DsDesc_t));

        /* one interrupt process desc_depth max, for other channel using same sync channel to be processed in time */
        if ((process_cnt >= knet_dma_chan[lchip][chan_id]->desc_depth) && (!need_eop))
        {
            cur_index++;
            break;
        }
    }

    knet_dma_chan[lchip][chan_id]->current_index = (cur_index>=knet_dma_chan[lchip][chan_id]->desc_depth) ?
        (cur_index % knet_dma_chan[lchip][chan_id]->desc_depth) : cur_index;

    return ret;
}

STATIC int
knet_rx_thread_func(void *arg)
{
    DsDesc_t* p_desc = NULL;
    u32 cur_index = 0;
    knet_dma_desc_t* p_base_desc = NULL;
    u32 chan_id = 0;
    u8 lchip = 0;
    int ret = 0;

    while (1)
    {
        ret = down_interruptible(&rx_sem);
        if (ret)
        {
            continue;
        }

        for (lchip = 0; lchip < DAL_MAX_CHIP_NUM; lchip++)
        {
            for (chan_id = 0; chan_id < KNET_MAX_RX_CHAN_NUM; chan_id++)
            {
                if (!knet_dma_chan[lchip][chan_id] || !knet_dma_chan[lchip][chan_id]->active)
                {
                    continue;
                }

                p_base_desc = (knet_dma_desc_t*)knet_dma_chan[lchip][chan_id]->virt_base;
                cur_index = knet_dma_chan[lchip][chan_id]->current_index;
                p_desc = &(p_base_desc[cur_index].desc_info);
                dal_cache_inval(virt_to_bus(p_desc), sizeof(DsDesc_t));

                if (!p_desc->done)
                {
                    continue;
                }

                //printk("KNET: receive packet, dma chan_id: %d, desc index: %d, realsize: %u, cfgsize: %u, memaddr: 0x%x.\n", chan_id, cur_index, p_desc->realSize, p_desc->cfgSize, p_desc->memAddr);
                spin_lock(&knet_dma_chan_lock[lchip][chan_id]);
                _knet_dma_pkt_rx_func(lchip, chan_id);
                spin_unlock(&knet_dma_chan_lock[lchip][chan_id]);
            }
        }

        if (!knet_rx_start)
        {
            break;
        }
    }

    return 0;
}

void intr_handle(void* data)
{
    up(&rx_sem);

    return;
}

STATIC int
linux_connect_irq(unsigned long arg)
{
    dal_ops_t * dal_ops = NULL;
    u32 irq = 0;
    int ret = 0;

    dal_get_dal_ops(&dal_ops);

    if (copy_from_user(&irq, (void*)arg, sizeof(u32)))
    {
        return -EFAULT;
    }

    ret = dal_ops->interrupt_connect(irq, 0, intr_handle, NULL);
    if (ret)
    {
        printk("KNET: connect irq %d to dal fail, ret = %d.\n", irq, ret);
        return ret;
    }

    return 0;
}

STATIC int
linux_disconnect_irq(unsigned long arg)
{
    dal_ops_t * dal_ops = NULL;
    u32 irq = 0;
    int ret = 0;

    dal_get_dal_ops(&dal_ops);

    if (copy_from_user(&irq, (void*)arg, sizeof(u32)))
    {
        return -EFAULT;
    }

    ret = dal_ops->interrupt_disconnect(irq);
    if (ret)
    {
        printk("KNET: disconnect irq %d to dal fail, ret = %d.\n", irq, ret);
        return ret;
    }

    return 0;
}

/* set knet version, copy to user */
STATIC int
linux_get_knet_version(unsigned long arg)
{
    if (copy_to_user((int*)arg, (void*)&knet_version, sizeof(knet_version)))
    {
        return -EFAULT;
    }

    return 0;
}

STATIC int
linux_set_dma_info(unsigned long arg)
{
    dal_dma_info_t dma_info;
    void *tx_base = NULL;

    if (copy_from_user(&dma_info, (void*)arg, sizeof(dal_dma_info_t)))
    {
        return -EFAULT;
    }

    memcpy(&knet_dma_info[dma_info.lchip], &dma_info, sizeof(dal_dma_info_t));

    tx_base = (void *)knet_dma_info[dma_info.lchip].virt_base + knet_dma_info[dma_info.lchip].knet_tx_offset;

    if (knet_dma_pool[dma_info.lchip])
    {
        dal_mpool_destroy(dma_info.lchip, knet_dma_pool[dma_info.lchip]);
    }

    knet_dma_pool[dma_info.lchip] = dal_mpool_create(dma_info.lchip, tx_base, dma_info.knet_tx_size);
    if (!knet_dma_pool[dma_info.lchip])
    {
        printk("KNET: create tx mpool fail!\n");
    }

    printk("KNET: set dma info on chip %d, phy addr: %d, virt addr: %p, tx base: %p, tx size: 0x%x.\n", dma_info.lchip,
        knet_dma_info[dma_info.lchip].phy_base, knet_dma_info[dma_info.lchip].virt_base, tx_base, dma_info.knet_tx_size);

    return 0;
}

STATIC int
linux_reg_dma_chan(unsigned long arg)
{
    dal_dma_chan_t dma_chan;
    u64 phy_addr = 0;

    if (copy_from_user(&dma_chan, (void*)arg, sizeof(dal_dma_chan_t)))
    {
        return -EFAULT;
    }

    if (!dma_chan.active && knet_dma_chan[dma_chan.lchip][dma_chan.channel_id])
    {
        knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->active = 0;
        msleep(100);

        printk("KNET: unregister lchip %d dma chan[%d].\n", dma_chan.lchip, dma_chan.channel_id);
    }
    else
    {
        if (!knet_dma_chan[dma_chan.lchip][dma_chan.channel_id])
        {
            knet_dma_chan[dma_chan.lchip][dma_chan.channel_id] = kzalloc(sizeof(dal_dma_chan_t), GFP_KERNEL);
            if (!knet_dma_chan[dma_chan.lchip][dma_chan.channel_id])
            {
                return -1;
            }
        }

        if (knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->p_desc_used &&
            (dma_chan.desc_num != knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->desc_num))
        {
            kfree(knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->p_desc_used);
            knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->p_desc_used = NULL;
        }

        if (!knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->p_desc_used)
        {
            knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->p_desc_used = kmalloc(sizeof(char) * dma_chan.desc_num, GFP_KERNEL);
            if (!knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->p_desc_used)
            {
                kfree(knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]);
                return -1;
            }
        }
        memset(knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->p_desc_used, 0, sizeof(char) * dma_chan.desc_num);

        knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->lchip = dma_chan.lchip;
        knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->channel_id = dma_chan.channel_id;
        knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->dmasel = dma_chan.dmasel;
        knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->desc_num = dma_chan.desc_num;
        knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->desc_depth = dma_chan.desc_depth;
        knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->data_size = dma_chan.data_size;
        knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->mem_base = dma_chan.mem_base;
        knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->current_index = 0;

        phy_addr = knet_dma_info[dma_chan.lchip].phy_base_hi;
        phy_addr = (phy_addr << 32) | knet_dma_info[dma_chan.lchip].phy_base;

        knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->virt_base = ((void*)knet_dma_info[dma_chan.lchip].virt_base + (dma_chan.mem_base - phy_addr));
        spin_lock_init(&knet_dma_chan_lock[dma_chan.lchip][dma_chan.channel_id]);
        knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->active = dma_chan.active;

        printk("KNET: register lchip %d dma chan[%d], desc virt base %p\n", knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->lchip,
            knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->channel_id, (void*)knet_dma_chan[dma_chan.lchip][dma_chan.channel_id]->virt_base);

        if (knet_rx_start == 0)
        {
            knet_rx_start = 1;
            wake_up_process(knet_rx_thread);
        }
    }

    return 0;
}

STATIC int
knet_device_open(struct net_device *dev)
{
    netif_start_queue(dev);

    return 0;
}

STATIC int
knet_device_stop(struct net_device *dev)
{
    netif_stop_queue(dev);

    return 0;
}

STATIC int
_knet_wait_desc_finish_tx(DsDesc_t* p_desc, dal_dma_chan_t* p_dma_chan)
{
    unsigned int  cnt = 0;
    int   ret = 0;

    /* check last transmit is done */
    if (p_dma_chan->p_desc_used[p_dma_chan->current_index])
    {
        while(cnt < 100)
        {
            dal_cache_inval(virt_to_bus(p_desc), sizeof(DsDesc_t));
            if (p_desc->done)
            {
                break;
                /* last transmit is done */
            }

            msleep(1);
            cnt++;
        }

        if (!p_desc->done)
        {
            printk("KNET: last transmit is not done,%d\n", p_dma_chan->current_index);
            p_dma_chan->p_desc_used[p_dma_chan->current_index] = 0;
            ret = 0;
        }
    }

    return ret;
}

STATIC int
_knet_packet_tx_rawhdr(u32 *p_raw_hdr, struct sk_buff *skb)
{
    net_device_priv_t *priv = netdev_priv(skb->dev);
    u32 dest_map = 0;
    u8 gchip = 0;
    u8 hash = 0;

    /* Must be inited */
    *(p_raw_hdr + PACKET_OTHER_FROMCPU_BIT_OFFSET/32) |= 1 << 8;
    *(p_raw_hdr + PACKET_MACKNOWN_BIT_OFFSET/32) |= 1 << 24;
    *(p_raw_hdr + PACKET_FROMCPUOROAM_BIT_OFFSET/32) |= 1 << 17;

    /*default set ttl to 1*/
    *(p_raw_hdr + PACKET_TTL_BIT_OFFSET/32) |= 1 << 19;

    /*default set color to green*/
    *(p_raw_hdr + PACKET_COLOR_BIT_OFFSET/32) |= 3 << 19;

    /*default set prio to 15*/
    *(p_raw_hdr + PACKET_PRIO_BIT_OFFSET/32) |= 0xF;

    if (priv->vlan)
    {
        *(p_raw_hdr + PACKET_STAGACTION_BIT_OFFSET/32) |= 2 << 25;
        *(p_raw_hdr + PACKET_SVLANTAG_OPVALID_BIT_OFFSET/32) |= 1 << 25;
        *(p_raw_hdr + PACKET_SVLAN_BIT_OFFSET/32 + 1) |= (priv->vlan >> 11) & 0x1;
        *(p_raw_hdr + PACKET_SVLAN_BIT_OFFSET/32) |= (priv->vlan & 0x7FF) << 21;
    }

    if (priv->type == DAL_NETIF_T_PORT)
    {
        dest_map = PACKET_ENCODE_GPORT_TO_DESTMAP(priv->gport);

        /*cfg bypass*/
        *(p_raw_hdr + PACKET_BYPASSINGRESSEDIT_BIT_OFFSET/32) |= 1 << 22;
        *(p_raw_hdr + PACKET_NEXTHOPEXT_BIT_OFFSET/32) |= 1 << 28;
        *(p_raw_hdr + PACKET_NEXTHOPPTR_BIT_OFFSET/32) |= (NEXTHOP_RES_BYPASS_NH_PTR & 0x3FFFF) << 1;
    }
    else
    {
        gchip = PACKET_GPORT_TO_GCHIP(priv->gport);
        dest_map = PACKET_ENCODE_LPORT_TO_DESTMAP(gchip, PORT_RSV_PORT_ILOOP_ID);
    }
    *(p_raw_hdr + PACKET_DESTMAP_BIT_OFFSET/32) |= (dest_map & 0x7FFFF) << 5;

    /*cfg critical*/
    *(p_raw_hdr + PACKET_CRITICALPACKET_BIT_OFFSET/32) |= 1 << 31;

    hash = _knet_data_calc_crc8(skb->data, 12, 0);
    *(p_raw_hdr + PACKET_HEADERHASH_BIT_OFFSET/32) |= (hash & 0xFF) << 18;

    return 0;
}

STATIC int
_knet_packet_tx(u8 lchip, dal_dma_chan_t* p_dma_chan, DsDesc_t* p_desc, void *logic_addr, u32 pkt_len)
{
    int ret = 0;
    unsigned int phy_tx_addr = 0;

    p_desc->u1_pkt_sop = 1;
    p_desc->u1_pkt_eop = 1;
    p_desc->rsv0 = 1;
    phy_tx_addr = (unsigned int)virt_to_bus(logic_addr);
    #ifdef DUET2
    p_desc->cfgSize = htons(pkt_len);
    p_desc->memAddr = htonl(phy_tx_addr);
    #else
    p_desc->memAddr = (phy_tx_addr>>4);
    p_desc->cfgSize = pkt_len;
    #endif
    dal_cache_flush(virt_to_bus(logic_addr), pkt_len);
    dal_cache_flush(virt_to_bus(p_desc), sizeof(DsDesc_t));

    p_dma_chan->p_desc_used[p_dma_chan->current_index] = 1;

    /* next descriptor, tx_desc_index: 0~tx_desc_num-1*/
    p_dma_chan->current_index =
        (p_dma_chan->current_index == (p_dma_chan->desc_depth - 1)) ? 0 : (p_dma_chan->current_index + 1);

    return ret;
}

STATIC int
knet_device_tx(struct sk_buff *skb, struct net_device *dev)
{
    net_device_priv_t *priv = netdev_priv(dev);
    knet_dma_desc_t* p_base_desc = NULL;
    u32 bheader[PACKET_HEADER_LEN/4] = {0};
    u32 *p_raw_hdr = bheader;
    DsDesc_t* p_desc = NULL;
    u64 phy_addr = 0;
    void *logic_addr = 0;
    u32 cur_index = 0;
    u32 chan_id = 0;
    int ret = 0;

    for (chan_id = KNET_MAX_RX_CHAN_NUM; chan_id < KNET_MAX_RX_CHAN_NUM + KNET_MAX_TX_CHAN_NUM; chan_id++)
    {
        if (knet_dma_chan[priv->lchip][chan_id] && knet_dma_chan[priv->lchip][chan_id]->active)
        {
            break;
        }
    }

    if (chan_id == KNET_MAX_RX_CHAN_NUM + KNET_MAX_TX_CHAN_NUM)
    {
        return -1;
    }

    if (skb->len > KNET_PKT_MTU)
    {
        printk("KNET: packet too long, length: %d.\n", skb->len);
        return -1;
    }

    spin_lock(&knet_dma_chan_lock[priv->lchip][chan_id]);
    if (knet_dma_done[priv->lchip][chan_id])
    {
        spin_unlock(&knet_dma_chan_lock[priv->lchip][chan_id]);
        printk("last transmit is not done!\n");
        return -1;
    }

    knet_dma_done[priv->lchip][chan_id] = 1;
    wake_up(&poll_desc);

    p_base_desc = (knet_dma_desc_t*)knet_dma_chan[priv->lchip][chan_id]->virt_base;
    cur_index = knet_dma_chan[priv->lchip][chan_id]->current_index;

    p_desc = &p_base_desc[cur_index].desc_info;
    dal_cache_inval(virt_to_bus(p_desc), sizeof(DsDesc_t));

    ret = _knet_wait_desc_finish_tx(p_desc, knet_dma_chan[priv->lchip][chan_id]);
    if (ret)
    {
        return ret;
    }

    phy_addr = knet_dma_info[priv->lchip].phy_base_hi;
    phy_addr = (phy_addr << 32) | ntohl(p_desc->memAddr);
    if (phy_addr)
    {
        logic_addr = bus_to_virt(phy_addr);
        dal_mpool_free(priv->lchip, knet_dma_pool[priv->lchip], logic_addr);
    }

    logic_addr = dal_mpool_alloc(priv->lchip, knet_dma_pool[priv->lchip], skb->len + PACKET_HEADER_LEN, 0);
    if (!logic_addr)
    {
        printk("KNET: alloc tx mem fail, size: %d.\n", skb->len + PACKET_HEADER_LEN);
        return -1;
    }

    _knet_packet_tx_rawhdr(p_raw_hdr, skb);
    _knet_convert_header((u32*)logic_addr, p_raw_hdr, PACKET_HEADER_LEN / 4);

    memcpy((char*)logic_addr + PACKET_HEADER_LEN, skb->data, skb->len);

    printk("KNET: send packet to netif %s, vlan: %d, gport: 0x%x, packet len: %d.\n", skb->dev->name, priv->vlan, priv->gport, skb->len);
    _knet_packet_dump((void*)logic_addr, skb->len + PACKET_HEADER_LEN);

    _knet_packet_tx(priv->lchip, knet_dma_chan[priv->lchip][chan_id], p_desc, logic_addr, skb->len + PACKET_HEADER_LEN);

    spin_unlock(&knet_dma_chan_lock[priv->lchip][chan_id]);

    return ret;
}

/*
 * Network Device Statistics.
 * Cleared at init time.
 */
STATIC struct net_device_stats *
knet_device_get_stats(struct net_device *dev)
{
    net_device_priv_t *priv = netdev_priv(dev);

    return &priv->stats;
}

STATIC int
knet_device_change_mtu(struct net_device *dev, int new_mtu)
{
    if (new_mtu < 68 || new_mtu > 9664) {
        return -EINVAL;
    }
    dev->mtu = new_mtu;
    return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29))
static const struct net_device_ops knet_netdev_ops = {
	.ndo_open		= knet_device_open,
	.ndo_stop		= knet_device_stop,
	.ndo_start_xmit		= knet_device_tx,
	.ndo_get_stats		= knet_device_get_stats,
	.ndo_validate_addr	= NULL,
	.ndo_set_rx_mode	= NULL,
	.ndo_set_mac_address	= NULL,
	.ndo_do_ioctl		= NULL,
	.ndo_tx_timeout		= NULL,
	.ndo_change_mtu		= knet_device_change_mtu,
};
#endif

STATIC struct net_device *
knet_init_dev(u8 *mac, char *name)
{
    struct net_device *dev;

    /* Create Ethernet device */
    dev = alloc_etherdev(sizeof(net_device_priv_t));

    if (dev == NULL) {
        printk("Error allocating Ethernet device.\n");
        return NULL;
    }
#ifdef SET_MODULE_OWNER
    SET_MODULE_OWNER(dev);
#endif

    /* Set the device MAC address */
    memcpy(dev->dev_addr, mac, 6);

    /* Device information -- not available right now */
    dev->irq = 0;
    dev->base_addr = 0;

    /* Default MTU should not exceed MTU of switch front-panel ports */
    dev->mtu = 9664;

    /* Device vectors */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
    dev->netdev_ops = &knet_netdev_ops;
#else
    dev->open = knet_device_open;
    dev->hard_start_xmit = knet_device_tx;
    dev->stop = knet_device_stop;
    dev->get_stats = knet_device_get_stats;
    dev->change_mtu = knet_device_change_mtu;
#endif

    if (name && *name)
    {
        strncpy(dev->name, name, IFNAMSIZ-1);
    }

    /* Register the kernel Ethernet device */
    if (register_netdev(dev))
    {
        printk("Error registering Ethernet device.\n");
        free_netdev(dev);
        return NULL;
    }
    printk("Created Ethernet device %s, ifindex: %d.\n", dev->name, dev->ifindex);

    return dev;
}

STATIC int
knet_netif_create(dal_netif_t *netif)
{
    struct net_device *dev;
    struct list_head *list;
    net_device_priv_t *priv, *cur_priv;
    char mac_0[6] = {0};
    unsigned long flags;
    int found = 0;
    int ret = 0;

    if ((netif->type != DAL_NETIF_T_VLAN) && (netif->type != DAL_NETIF_T_PORT))
    {
        return -1;
    }

    if (netif->netif_id >= DAL_MAX_KNET_NETIF)
    {
        return -1;
    }

    if (!memcmp(netif->mac, mac_0, 6))
    {
        return -1;
    }

    priv = _knet_netif_lookup(netif);
    if (priv)
    {
        return -1;
    }

    if ((dev = knet_init_dev(netif->mac, netif->name)) == NULL)
    {
        return -1;
    }
    priv = netdev_priv(dev);
    priv->dev = dev;
    priv->lchip = netif->lchip;
    priv->type = netif->type;
    priv->vlan = (netif->type == DAL_NETIF_T_VLAN) ? netif->vlan : 0;
    priv->gport = netif->gport;

    spin_lock_irqsave(&knet_dev_lock, flags);

    /*
     * We insert network interfaces sorted by ID.
     * In case an interface is destroyed, we reuse the ID
     * the next time an interface is created.
     */
    list_for_each(list, &knet_dev_list)
    {
        cur_priv = (net_device_priv_t *)list;
        if (dev->ifindex < cur_priv->netif_id)
        {
            found = 1;
            break;
        }
    }
    priv->netif_id = dev->ifindex;
    if (found)
    {
        /* Replace previously removed interface */
        list_add_tail(&priv->list, &cur_priv->list);
    }
    else
    {
        /* No holes - add to end of list */
        list_add_tail(&priv->list, &knet_dev_list);
    }

    spin_unlock_irqrestore(&knet_dev_lock, flags);

    printk("Assigned ID %d to Ethernet device %s, mac: %x%x.%x%x.%x%x\n", priv->netif_id, dev->name,
        dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2], dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);

    netif->netif_id = priv->netif_id;

    ret = dev_open(dev);
    if (ret)
    {
        printk("device %s start fail, ret = %d.\n", dev->name, ret);
    }

    return 0;
}

STATIC int
knet_netif_destroy(dal_netif_t *netif)
{
    struct net_device *dev;
    net_device_priv_t *priv = NULL;
    unsigned long flags;

    priv = _knet_netif_lookup(netif);
    if (!priv)
    {
        return -1;
    }

    spin_lock_irqsave(&knet_dev_lock, flags);
    list_del(&priv->list);
    spin_unlock_irqrestore(&knet_dev_lock, flags);

    dev = priv->dev;
    printk("Removing virtual Ethernet device %s, netif_id: %d.\n", dev->name, priv->netif_id);
    unregister_netdev(dev);
    free_netdev(dev);

    return 0;
}

STATIC int
knet_netif_get(dal_netif_t *netif)
{
    net_device_priv_t *priv = NULL;

    priv = _knet_netif_lookup(netif);
    if (!priv)
    {
        return -1;
    }

    netif->gport = priv->gport;
    netif->netif_id = priv->netif_id;
    memcpy(netif->mac, priv->dev->dev_addr, 6);
    memcpy(netif->name, priv->dev->name, DAL_MAX_KNET_NAME_LEN - 1);

    return 0;
}

STATIC int
linux_handle_netif(unsigned long arg)
{
    dal_netif_t netif = {0};
    int ret = 0;

    if (copy_from_user(&netif, (void*)arg, sizeof(dal_netif_t)))
    {
        return -EFAULT;
    }

    switch (netif.op_type)
    {
        case DAL_OP_CREATE:
            ret = knet_netif_create(&netif);
            break;
        case DAL_OP_DESTORY:
            ret = knet_netif_destroy(&netif);
            break;
        case DAL_OP_GET:
            ret = knet_netif_get(&netif);
            break;
        default:
            break;
    }

    if ((netif.op_type == DAL_OP_CREATE) ||
        (netif.op_type == DAL_OP_GET))
    {
        if (copy_to_user((void*)arg, &netif, sizeof (dal_netif_t)))
        {
            return -EFAULT;
        }
    }

    return ret;
}

STATIC u32
linux_knet_poll(struct file* filp, struct poll_table_struct* p)
{
    u8 lchip = 0;
    u8 chan_id = 0;
    u32 mask = 0;
    unsigned long flags;

    poll_wait(filp, &poll_desc, p);
    local_irq_save(flags);
    for (lchip = 0; lchip < DAL_MAX_CHIP_NUM; lchip++)
    {
        for (chan_id = KNET_MAX_RX_CHAN_NUM; chan_id < KNET_MAX_RX_CHAN_NUM + KNET_MAX_TX_CHAN_NUM; chan_id++)
        {
            if (!knet_dma_chan[lchip][chan_id] || !knet_dma_chan[lchip][chan_id]->active)
            {
                continue;
            }
            
            spin_lock(&knet_dma_chan_lock[lchip][chan_id]);
            if (knet_dma_done[lchip][chan_id])
            {
                mask |= POLLIN | POLLRDNORM;
                knet_dma_done[lchip][chan_id] = 0;
            }
            spin_unlock(&knet_dma_chan_lock[lchip][chan_id]);
        }
    }
    local_irq_restore(flags);

    return mask;
}

#ifdef CONFIG_COMPAT
STATIC long
linux_knet_ioctl(struct file* file,
                u32 cmd, unsigned long arg)
#else

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
STATIC long
linux_knet_ioctl(struct file* file,
                u32 cmd, unsigned long arg)
#else
STATIC int
linux_knet_ioctl(struct inode* inode, struct file* file,
                u32 cmd, unsigned long arg)
#endif

#endif
{
    switch (cmd)
    {
    case CMD_GET_KNET_VERSION:
        return linux_get_knet_version(arg);

    case CMD_CONNECT_INTERRUPTS:
        return linux_connect_irq(arg);

    case CMD_DISCONNECT_INTERRUPTS:
        return linux_disconnect_irq(arg);

    case CMD_SET_DMA_INFO:
        return linux_set_dma_info(arg);

    case CMD_REG_DMA_CHAN:
        return linux_reg_dma_chan(arg);

    case CMD_HANDLE_NETIF:
        return linux_handle_netif(arg);

    default:
        break;
    }

    return 0;
}

static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .poll = linux_knet_poll,
#ifdef CONFIG_COMPAT
    .compat_ioctl = linux_knet_ioctl,
    .unlocked_ioctl = linux_knet_ioctl,
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
    .unlocked_ioctl = linux_knet_ioctl,
#else
    .ioctl = linux_knet_ioctl,
#endif
#endif
};

STATIC int __init
linux_knet_init(void)
{
    int ret = 0;
    u8 lchip = 0;

    memset(knet_dma_info, 0, DAL_MAX_CHIP_NUM * sizeof(dal_dma_info_t));
    memset(knet_dma_chan, 0, DAL_MAX_CHIP_NUM * (KNET_MAX_RX_CHAN_NUM+KNET_MAX_TX_CHAN_NUM) * sizeof(dal_dma_chan_t *));
    memset(knet_dma_done, 0, DAL_MAX_CHIP_NUM * (KNET_MAX_RX_CHAN_NUM+KNET_MAX_TX_CHAN_NUM) * sizeof(u8));

    ret = register_chrdev(KNET_DEV_MAJOR, KNET_NAME, &fops);
    if (ret < 0)
    {
        printk(KERN_WARNING "Register linux_knet device, ret %d\n", ret);
        return ret;
    }

    /* alloc /dev/linux_knet node */
    knet_class = class_create(THIS_MODULE, KNET_NAME);
    device_create(knet_class, NULL, MKDEV(KNET_DEV_MAJOR, 0), NULL, KNET_NAME);

    INIT_LIST_HEAD(&knet_dev_list);
    spin_lock_init(&knet_dev_lock);
    init_waitqueue_head(&poll_desc);
    sema_init(&rx_sem, 0);

    knet_rx_thread = kthread_create(knet_rx_thread_func, NULL, "knet_rx_thread");
    if (!knet_rx_thread)
    {
        printk("create knet rx thread error!\n");
        return -1;
    }

    dal_mpool_init(lchip);

    printk("Register linux_knet device!\n");

    return ret;
}

STATIC void __exit
linux_knet_exit(void)
{
    net_device_priv_t *priv;
    struct list_head *list;
    unsigned long flags;
    u8 lchip = 0;
    u8 chan_id = 0;

    printk("start unregister linux_knet device!\n");

    device_destroy(knet_class, MKDEV(KNET_DEV_MAJOR, 0));
    class_destroy(knet_class);
    unregister_chrdev(KNET_DEV_MAJOR, "linux_knet");

    if (knet_rx_thread)
    {
        knet_rx_start = 0;
        msleep(500);
    }

    for (lchip = 0; lchip < DAL_MAX_CHIP_NUM; lchip++)
    {
        for (chan_id = 0; chan_id < KNET_MAX_RX_CHAN_NUM+KNET_MAX_TX_CHAN_NUM; chan_id++)
        {
            if (knet_dma_chan[lchip][chan_id])
            {
                kfree(knet_dma_chan[lchip][chan_id]);
                knet_dma_chan[lchip][chan_id] = NULL;
            }
        }

        if (knet_dma_pool[lchip])
        {
            dal_mpool_destroy(lchip, knet_dma_pool[lchip]);
            knet_dma_pool[lchip] = NULL;
        }
    }

    printk("destory all netif!\n");
    spin_lock_irqsave(&knet_dev_lock, flags);
    list_for_each(list, &knet_dev_list)
    {
        priv = (net_device_priv_t *)list;
        unregister_netdev(priv->dev);
        free_netdev(priv->dev);
    }
    spin_unlock_irqrestore(&knet_dev_lock, flags);

    printk("Unregister linux_knet device!\n");
}

module_init(linux_knet_init);
module_exit(linux_knet_exit);

