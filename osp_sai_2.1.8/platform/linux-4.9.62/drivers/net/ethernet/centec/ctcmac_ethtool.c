/*
 * Centec CpuMac Ethernet Driver -- CpuMac controller implementation
 * Provides Bus interface for MIIM regs
 *
 * Author: liuht <liuht@centecnetworks.com>
 *
 * Copyright 2002-2018, Centec Networks (Suzhou) Co., Ltd.
 *
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_mdio.h>
#include <linux/of_platform.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/in.h>
#include <linux/net_tstamp.h>
#include <linux/ethtool.h>
#include <asm/io.h>

#include "ctcmac.h"
#include "ctcmac_reg.h"

struct ctcmac_pkt_stats{
	u64 rx_good_ucast_bytes;
	u64 rx_good_ucast_pkt  ;
	u64 rx_good_mcast_bytes;
	u64 rx_good_mcast_pkt  ;
	u64 rx_good_bcast_bytes;
	u64 rx_good_bcast_pkt  ;
	u64 rx_good_pause_bytes;
	u64 rx_good_pause_pkt  ;
	u64 rx_good_pfc_bytes  ;
	u64 rx_good_pfc_pkt    ;
	u64 rx_good_control_bytes;
	u64 rx_good_control_pkt;
	u64 rx_fcs_error_bytes ;
	u64 rx_fcs_error_pkt   ;
	u64 rx_mac_overrun_bytes;
	u64 rx_mac_overrun_pkt ;
	u64 rx_good_63B_bytes  ;
	u64 rx_good_63B_pkt    ;
	u64 rx_bad_63B_bytes   ;
	u64 rx_bad_63B_pkt     ;
	u64 rx_good_mtu2B_bytes;
	u64 rx_good_mtu2B_pkt  ;
	u64 rx_bad_mtu2B_bytes ;
	u64 rx_bad_mtu2B_pkt   ;
	u64 rx_good_jumbo_bytes;
	u64 rx_good_jumbo_pkt  ;
	u64 rx_bad_jumbo_bytes ;
	u64 rx_bad_jumbo_pkt   ;
	u64 rx_64B_bytes       ;
	u64 rx_64B_pkt         ;
	u64 rx_127B_bytes      ;
	u64 rx_127B_pkt        ;
	u64 rx_255B_bytes      ;
	u64 rx_255B_pkt        ;
	u64 rx_511B_bytes      ;
	u64 rx_511B_pkt        ;
	u64 rx_1023B_bytes     ;
	u64 rx_1023B_pkt       ;
	u64 rx_mtu1B_bytes     ;
	u64 rx_mtu1B_pkt       ;
	u64 tx_ucast_bytes     ;
	u64 tx_ucast_pkt       ;
	u64 tx_mcast_bytes     ;
	u64 tx_mcast_pkt       ;
	u64 tx_bcast_bytes     ;
	u64 tx_bcast_pkt       ;
	u64 tx_pause_bytes     ;
	u64 tx_pause_pkt       ;
	u64 tx_control_bytes   ;
	u64 tx_control_pkt     ;
	u64 tx_fcs_error_bytes ;
	u64 tx_fcs_error_pkt   ;
	u64 tx_underrun_bytes  ;
	u64 tx_underrun_pkt    ;
	u64 tx_63B_bytes       ;
	u64 tx_63B_pkt         ;
	u64 tx_64B_bytes       ;
	u64 tx_64B_pkt         ;
	u64 tx_127B_bytes      ;
	u64 tx_127B_pkt        ;
	u64 tx_255B_bytes      ;
	u64 tx_255B_pkt        ;
	u64 tx_511B_bytes      ;
	u64 tx_511B_pkt        ;
	u64 tx_1023B_bytes     ;
	u64 tx_1023B_pkt       ;
	u64 tx_mtu1_bytes     ;
	u64 tx_mtu1_pkt       ;
	u64 tx_mtu2_bytes     ;
	u64 tx_mtu2_pkt       ;
	u64 tx_jumbo_bytes     ;
	u64 tx_jumbo_pkt       ;
	u64 mtu1;
	u64 mtu2;
};

#define CTCMAC_STATS_LEN  (sizeof(struct ctcmac_pkt_stats)/sizeof(u64))

extern void stop_ctcmac(struct net_device *ndev);
extern int startup_ctcmac(struct net_device *ndev);

static const char ctc_stat_gstrings[][ETH_GSTRING_LEN] = {
	"RX-bytes-good-ucast",
	"RX-frame-good-ucast",
	"RX-bytes-good-mcast",
	"RX-frame-good-mcast",
	"RX-bytes-good-bcast",
	"RX-frame-good-bcast",
	"RX-bytes-good-pause",
	"RX-frame-good-pause",
	"RX-bytes-good-pfc",
	"RX-frame-good-pfc",  
	"RX-bytes-good-control",
	"RX-frame-good-control",
	"RX-bytes-fcs-error",
	"RX-frame-fcs-error",
	"RX-bytes-mac-overrun",
	"RX-frame-mac-overrun",
	"RX-bytes-good-63B",
	"RX-frame-good-63B",
	"RX-bytes-bad-63B", 
	"RX-frame-bad-63B",
	"RX-bytes",
	"RX-frame",
	"RX-bytes-bad",
	"RX-frame-bad",
	"RX-bytes-good-jumbo",
	"RX-frame-good-jumbo",
	"RX-bytes-bad-jumbo", 
	"RX-frame-bad-jumbo", 
	"RX-bytes-64B",       
	"RX-frame-64B",       
	"RX-bytes-127B",      
	"RX-frame-127B",      
	"RX-bytes-255B",      
	"RX-frame-255B",      
	"RX-bytes-511B",      
	"RX-frame-511B",      
	"RX-bytes-1023B",     
	"RX-frame-1023B",     
	"RX-bytes",       
	"RX-frame",
	"TX-bytes-ucast",    
	"TX-frame-ucast",    
	"TX-bytes-mcast",    
	"TX-frame-mcast",   
	"TX-bytes-bcast",    
	"TX-frame-bcast",    
	"TX-bytes-pause",    
	"TX-frame-pause",   
	"TX-bytes-control",  
	"TX-frame-control",  
	"TX-bytes-fcs-error",
	"TX-frame-fcs-error",
	"TX-bytes-underrun",
	"TX-frame-underrun",
	"TX-bytes-63B",      
	"TX-frame-63B",      
	"TX-bytes-64B",      
	"TX-frame-64B",      
	"TX-bytes-127B",     
	"TX-frame-127B",     
	"TX-bytes-255B",     
	"TX-frame-255B",     
	"TX-bytes-511B",     
	"TX-frame-511B",     
	"TX-bytes-1023B",   
	"TX-frame-1023B",   
	"TX-bytes-mtu1",
	"TX-frame-mtu1",
	"TX-bytes-mtu2",
	"TX-frame-mtu2",
	"TX-bytes-jumbo",   
	"TX-frame-jumbo",
	"mtu1",
	"mtu2",   
};

static struct ctcmac_pkt_stats g_pkt_stats[2];

static inline u32 ctcmac_regr(unsigned __iomem *addr)
{
	u32 val;
	val = readl(addr);
	return val;
}

static inline void ctcmac_regw(unsigned __iomem *addr, u32 val)
{
	writel(val, addr);
}

static void ctcmac_gdrvinfo(struct net_device *dev,
			  struct ethtool_drvinfo *drvinfo)
{
	strlcpy(drvinfo->driver, "ctcmac", sizeof(drvinfo->driver));
	strlcpy(drvinfo->version, "v1.0",	sizeof(drvinfo->version));
	strlcpy(drvinfo->fw_version, "N/A", sizeof(drvinfo->fw_version));
	strlcpy(drvinfo->bus_info, "N/A", sizeof(drvinfo->bus_info));
}

/* Return the length of the register structure */
static int ctcmac_reglen(struct net_device *dev)
{
	return sizeof (struct CpuMac_regs);
}

/* Return a dump of the GFAR register space */
static void ctcmac_get_regs(struct net_device *dev, struct ethtool_regs *regs,
			  void *regbuf)
{
	int i;
	struct ctcmac_private *priv = netdev_priv(dev);
	u32 __iomem *theregs = (u32 __iomem *) priv->cpumac_reg;
	u32 *buf = (u32 *) regbuf;

	for (i = 0; i < sizeof (struct CpuMac_regs) / sizeof (u32); i++)
		buf[i] = ctcmac_regr(&theregs[i]);
}

/* Fills in rvals with the current ring parameters.  Currently,
 * rx, rx_mini, and rx_jumbo rings are the same size, as mini and
 * jumbo are ignored by the driver */
static void ctcmac_gringparam(struct net_device *dev,
			    struct ethtool_ringparam *rvals)
{
	struct ctcmac_private *priv = netdev_priv(dev);
	struct ctcmac_priv_tx_q *tx_queue = NULL;
	struct ctcmac_priv_rx_q *rx_queue = NULL;

	tx_queue = priv->tx_queue[0];
	rx_queue = priv->rx_queue[0];

	rvals->rx_max_pending = CTCMAC_MAX_RING_SIZE;
	rvals->rx_mini_max_pending = CTCMAC_MAX_RING_SIZE;
	rvals->rx_jumbo_max_pending = CTCMAC_MAX_RING_SIZE;
	rvals->tx_max_pending = CTCMAC_MAX_RING_SIZE;

	/* Values changeable by the user.  The valid values are
	 * in the range 1 to the "*_max_pending" counterpart above.
	 */
	rvals->rx_pending = rx_queue->rx_ring_size;
	rvals->rx_mini_pending = rx_queue->rx_ring_size;
	rvals->rx_jumbo_pending = rx_queue->rx_ring_size;
	rvals->tx_pending = tx_queue->tx_ring_size;
}

/* Change the current ring parameters, stopping the controller if
 * necessary so that we don't mess things up while we're in motion.
 */
static int ctcmac_sringparam(struct net_device *dev,
			   struct ethtool_ringparam *rvals)
{
	struct ctcmac_private *priv = netdev_priv(dev);
	int err = 0, i;

	if (rvals->rx_pending > CTCMAC_MAX_RING_SIZE)
		return -EINVAL;

	if (rvals->tx_pending > CTCMAC_MAX_RING_SIZE)
		return -EINVAL;

	while (test_and_set_bit_lock(CTCMAC_RESETTING, &priv->state))
		cpu_relax();

	if (dev->flags & IFF_UP)
		stop_ctcmac(dev);

	/* Change the sizes */
	for (i = 0; i < priv->num_rx_queues; i++)
		priv->rx_queue[i]->rx_ring_size = rvals->rx_pending;

	for (i = 0; i < priv->num_tx_queues; i++)
		priv->tx_queue[i]->tx_ring_size = rvals->tx_pending;

	/* Rebuild the rings with the new size */
	if (dev->flags & IFF_UP)
		err = startup_ctcmac(dev);

	clear_bit_unlock(CTCMAC_RESETTING, &priv->state);

	return err;
}

static void ctcmac_gpauseparam(struct net_device *dev,
			     struct ethtool_pauseparam *epause)
{
#if 0
	struct ctcmac_private *priv = netdev_priv(dev);

	epause->autoneg = !!priv->pause_aneg_en;
	epause->rx_pause = !!priv->rx_pause_en;
	epause->tx_pause = !!priv->tx_pause_en;
#endif
}

static int ctcmac_spauseparam(struct net_device *dev,
			    struct ethtool_pauseparam *epause)
{
#if 0
	struct gfar_private *priv = netdev_priv(dev);
	struct phy_device *phydev = dev->phydev;
	struct gfar __iomem *regs = priv->gfargrp[0].regs;
	u32 oldadv, newadv;

	if (!phydev)
		return -ENODEV;

	if (!(phydev->supported & SUPPORTED_Pause) ||
	    (!(phydev->supported & SUPPORTED_Asym_Pause) &&
	     (epause->rx_pause != epause->tx_pause)))
		return -EINVAL;

	priv->rx_pause_en = priv->tx_pause_en = 0;
	if (epause->rx_pause) {
		priv->rx_pause_en = 1;

		if (epause->tx_pause) {
			priv->tx_pause_en = 1;
			/* FLOW_CTRL_RX & TX */
			newadv = ADVERTISED_Pause;
		} else  /* FLOW_CTLR_RX */
			newadv = ADVERTISED_Pause | ADVERTISED_Asym_Pause;
	} else if (epause->tx_pause) {
		priv->tx_pause_en = 1;
		/* FLOW_CTLR_TX */
		newadv = ADVERTISED_Asym_Pause;
	} else
		newadv = 0;

	if (epause->autoneg)
		priv->pause_aneg_en = 1;
	else
		priv->pause_aneg_en = 0;

	oldadv = phydev->advertising &
		(ADVERTISED_Pause | ADVERTISED_Asym_Pause);
	if (oldadv != newadv) {
		phydev->advertising &=
			~(ADVERTISED_Pause | ADVERTISED_Asym_Pause);
		phydev->advertising |= newadv;
		if (phydev->autoneg)
			/* inform link partner of our
			 * new flow ctrl settings
			 */
			return phy_start_aneg(phydev);

		if (!epause->autoneg) {
			u32 tempval;
			tempval = gfar_read(&regs->maccfg1);
			tempval &= ~(MACCFG1_TX_FLOW | MACCFG1_RX_FLOW);

			priv->tx_actual_en = 0;
			if (priv->tx_pause_en) {
				priv->tx_actual_en = 1;
				tempval |= MACCFG1_TX_FLOW;
			}

			if (priv->rx_pause_en)
				tempval |= MACCFG1_RX_FLOW;
			gfar_write(&regs->maccfg1, tempval);
		}
	}
#endif
	return 0;
}

static void ctcmac_gstrings(struct net_device *dev, u32 stringset, u8 * buf)
{
	memcpy(buf, ctc_stat_gstrings, CTCMAC_STATS_LEN * ETH_GSTRING_LEN);
}

static int ctcmac_sset_count(struct net_device *dev, int sset)
{
	return CTCMAC_STATS_LEN;
}

static void ctcmac_fill_stats(struct net_device *netdev, struct ethtool_stats *dummy,
			    u64 *buf)
{
	u32 mtu;
	unsigned long flags;
	struct ctcmac_pkt_stats *stats;
	struct ctcmac_private *priv =  netdev_priv(netdev);

	spin_lock_irqsave(&priv->reglock, flags);
	stats = &g_pkt_stats[priv->index];
	stats->rx_good_ucast_bytes      +=   readq(&priv->cpumac_mem->CpuMacStatsRam0[0]);
	stats->rx_good_ucast_pkt        +=   readq(&priv->cpumac_mem->CpuMacStatsRam0[2]);
	stats->rx_good_mcast_bytes      +=   readq(&priv->cpumac_mem->CpuMacStatsRam1[0]);
	stats->rx_good_mcast_pkt        +=   readq(&priv->cpumac_mem->CpuMacStatsRam1[2]);
	stats->rx_good_bcast_bytes      +=   readq(&priv->cpumac_mem->CpuMacStatsRam2[0]);
	stats->rx_good_bcast_pkt        +=   readq(&priv->cpumac_mem->CpuMacStatsRam2[2]);
	stats->rx_good_pause_bytes      +=   readq(&priv->cpumac_mem->CpuMacStatsRam3[0]) ;
	stats->rx_good_pause_pkt        +=   readq(&priv->cpumac_mem->CpuMacStatsRam3[2]) ;
	stats->rx_good_pfc_bytes        +=   readq(&priv->cpumac_mem->CpuMacStatsRam4[0]) ;
	stats->rx_good_pfc_pkt          +=   readq(&priv->cpumac_mem->CpuMacStatsRam4[2]) ;
	stats->rx_good_control_bytes    +=   readq(&priv->cpumac_mem->CpuMacStatsRam5[0]) ;
	stats->rx_good_control_pkt      +=   readq(&priv->cpumac_mem->CpuMacStatsRam5[2]) ;
	stats->rx_fcs_error_bytes       +=   readq(&priv->cpumac_mem->CpuMacStatsRam6[0]) ;
	stats->rx_fcs_error_pkt         +=   readq(&priv->cpumac_mem->CpuMacStatsRam6[2]) ;
	stats->rx_mac_overrun_bytes     +=   readq(&priv->cpumac_mem->CpuMacStatsRam7[0]) ;
	stats->rx_mac_overrun_pkt       +=   readq(&priv->cpumac_mem->CpuMacStatsRam7[2]) ;
	stats->rx_good_63B_bytes        +=   readq(&priv->cpumac_mem->CpuMacStatsRam8[0]) ;
	stats->rx_good_63B_pkt          +=   readq(&priv->cpumac_mem->CpuMacStatsRam8[2]) ;
	stats->rx_bad_63B_bytes         +=   readq(&priv->cpumac_mem->CpuMacStatsRam9[0]) ;
	stats->rx_bad_63B_pkt           +=   readq(&priv->cpumac_mem->CpuMacStatsRam9[2]) ;
	stats->rx_good_mtu2B_bytes      +=   readq(&priv->cpumac_mem->CpuMacStatsRam10[0]);
	stats->rx_good_mtu2B_pkt        +=   readq(&priv->cpumac_mem->CpuMacStatsRam10[2]);
	stats->rx_bad_mtu2B_bytes       +=   readq(&priv->cpumac_mem->CpuMacStatsRam11[0]);
	stats->rx_bad_mtu2B_pkt         +=   readq(&priv->cpumac_mem->CpuMacStatsRam11[2]);
	stats->rx_good_jumbo_bytes      +=   readq(&priv->cpumac_mem->CpuMacStatsRam12[0]);
	stats->rx_good_jumbo_pkt        +=   readq(&priv->cpumac_mem->CpuMacStatsRam12[2]);
	stats->rx_bad_jumbo_bytes       +=   readq(&priv->cpumac_mem->CpuMacStatsRam13[0]);
	stats->rx_bad_jumbo_pkt         +=   readq(&priv->cpumac_mem->CpuMacStatsRam13[2]);
	stats->rx_64B_bytes             +=   readq(&priv->cpumac_mem->CpuMacStatsRam14[0]);
	stats->rx_64B_pkt               +=   readq(&priv->cpumac_mem->CpuMacStatsRam14[2]);
	stats->rx_127B_bytes            +=   readq(&priv->cpumac_mem->CpuMacStatsRam15[0]);
	stats->rx_127B_pkt              +=   readq(&priv->cpumac_mem->CpuMacStatsRam15[2]);
	stats->rx_255B_bytes            +=   readq(&priv->cpumac_mem->CpuMacStatsRam16[0]);
	stats->rx_255B_pkt              +=   readq(&priv->cpumac_mem->CpuMacStatsRam16[2]);
	stats->rx_511B_bytes            +=   readq(&priv->cpumac_mem->CpuMacStatsRam17[0]);
	stats->rx_511B_pkt              +=   readq(&priv->cpumac_mem->CpuMacStatsRam17[2]);
	stats->rx_1023B_bytes           +=   readq(&priv->cpumac_mem->CpuMacStatsRam18[0]);
	stats->rx_1023B_pkt             +=   readq(&priv->cpumac_mem->CpuMacStatsRam18[2]);
	stats->rx_mtu1B_bytes           +=   readq(&priv->cpumac_mem->CpuMacStatsRam19[0]);
	stats->rx_mtu1B_pkt             +=   readq(&priv->cpumac_mem->CpuMacStatsRam19[2]);
	stats->tx_ucast_bytes           +=   readq(&priv->cpumac_mem->CpuMacStatsRam20[0]);
	stats->tx_ucast_pkt             +=   readq(&priv->cpumac_mem->CpuMacStatsRam20[2]);
	stats->tx_mcast_bytes           +=   readq(&priv->cpumac_mem->CpuMacStatsRam21[0]);
	stats->tx_mcast_pkt             +=   readq(&priv->cpumac_mem->CpuMacStatsRam21[2]);
	stats->tx_bcast_bytes           +=   readq(&priv->cpumac_mem->CpuMacStatsRam22[0]);
	stats->tx_bcast_pkt             +=   readq(&priv->cpumac_mem->CpuMacStatsRam22[2]);
	stats->tx_pause_bytes           +=   readq(&priv->cpumac_mem->CpuMacStatsRam23[0]);
	stats->tx_pause_pkt             +=   readq(&priv->cpumac_mem->CpuMacStatsRam23[2]);
	stats->tx_control_bytes         +=   readq(&priv->cpumac_mem->CpuMacStatsRam24[0]);
	stats->tx_control_pkt           +=   readq(&priv->cpumac_mem->CpuMacStatsRam24[2]);
	stats->tx_fcs_error_bytes       +=   readq(&priv->cpumac_mem->CpuMacStatsRam25[0]);
	stats->tx_fcs_error_pkt         +=   readq(&priv->cpumac_mem->CpuMacStatsRam25[2]);
	stats->tx_underrun_bytes        +=   readq(&priv->cpumac_mem->CpuMacStatsRam26[0]);
	stats->tx_underrun_pkt          +=   readq(&priv->cpumac_mem->CpuMacStatsRam26[2]);
	stats->tx_63B_bytes             +=   readq(&priv->cpumac_mem->CpuMacStatsRam27[0]);
	stats->tx_63B_pkt               +=   readq(&priv->cpumac_mem->CpuMacStatsRam27[2]);
	stats->tx_64B_bytes             +=   readq(&priv->cpumac_mem->CpuMacStatsRam28[0]);
	stats->tx_64B_pkt               +=   readq(&priv->cpumac_mem->CpuMacStatsRam28[2]);
	stats->tx_127B_bytes            +=   readq(&priv->cpumac_mem->CpuMacStatsRam29[0]);
	stats->tx_127B_pkt              +=   readq(&priv->cpumac_mem->CpuMacStatsRam29[2]);
	stats->tx_255B_bytes            +=   readq(&priv->cpumac_mem->CpuMacStatsRam30[0]);
	stats->tx_255B_pkt              +=   readq(&priv->cpumac_mem->CpuMacStatsRam30[2]);
	stats->tx_511B_bytes            +=   readq(&priv->cpumac_mem->CpuMacStatsRam31[0]);
	stats->tx_511B_pkt              +=   readq(&priv->cpumac_mem->CpuMacStatsRam31[2]);
	stats->tx_1023B_bytes           +=   readq(&priv->cpumac_mem->CpuMacStatsRam32[0]);
	stats->tx_1023B_pkt             +=   readq(&priv->cpumac_mem->CpuMacStatsRam32[2]);
	stats->tx_mtu1_bytes           +=   readq(&priv->cpumac_mem->CpuMacStatsRam33[0]);
	stats->tx_mtu1_pkt             +=   readq(&priv->cpumac_mem->CpuMacStatsRam33[2]);
	stats->tx_mtu2_bytes           +=   readq(&priv->cpumac_mem->CpuMacStatsRam34[0]);
	stats->tx_mtu2_pkt             +=   readq(&priv->cpumac_mem->CpuMacStatsRam34[2]);
	stats->tx_jumbo_bytes           +=   readq(&priv->cpumac_mem->CpuMacStatsRam35[0]);
	stats->tx_jumbo_pkt             +=   readq(&priv->cpumac_mem->CpuMacStatsRam35[2]);
	mtu                                      =   readl(&priv->cpumac_reg->CpuMacStatsCfg[1]);
	stats->mtu1                         =  mtu & 0x3fff;
	stats->mtu2                         =  (mtu>>16) & 0x3fff;
	spin_unlock_irqrestore(&priv->reglock, flags);
	
	memcpy(buf, (void *)stats, sizeof(struct ctcmac_pkt_stats));
}

static uint32_t ctcmac_get_msglevel(struct net_device *dev)
{
	struct ctcmac_private *priv = netdev_priv(dev);

	return priv->msg_enable;
}

static void ctcmac_set_msglevel(struct net_device *dev, uint32_t data)
{
	struct ctcmac_private *priv = netdev_priv(dev);

	priv->msg_enable = data;
}

static int ctcmac_get_ts_info(struct net_device *dev,
			    struct ethtool_ts_info *info)
{
#if 0
	struct gfar_private *priv = netdev_priv(dev);

	if (!(priv->device_flags & FSL_GIANFAR_DEV_HAS_TIMER)) {
		info->so_timestamping = SOF_TIMESTAMPING_RX_SOFTWARE |
					SOF_TIMESTAMPING_SOFTWARE;
		info->phc_index = -1;
		return 0;
	}
	info->so_timestamping = SOF_TIMESTAMPING_TX_HARDWARE |
				SOF_TIMESTAMPING_RX_HARDWARE |
				SOF_TIMESTAMPING_RAW_HARDWARE;
	info->phc_index = gfar_phc_index;
	info->tx_types = (1 << HWTSTAMP_TX_OFF) |
			 (1 << HWTSTAMP_TX_ON);
	info->rx_filters = (1 << HWTSTAMP_FILTER_NONE) |
			   (1 << HWTSTAMP_FILTER_ALL);
#endif
	return 0;
}

const struct ethtool_ops ctcmac_ethtool_ops = {
	.get_drvinfo = ctcmac_gdrvinfo,
	.get_regs_len = ctcmac_reglen,
	.get_regs = ctcmac_get_regs,
	.get_link = ethtool_op_get_link,
	.get_ringparam = ctcmac_gringparam,
	.set_ringparam = ctcmac_sringparam,
	.get_pauseparam = ctcmac_gpauseparam,
	.set_pauseparam = ctcmac_spauseparam,
	.get_strings = ctcmac_gstrings,
	.get_sset_count = ctcmac_sset_count,
	.get_ethtool_stats = ctcmac_fill_stats,
	.get_msglevel = ctcmac_get_msglevel,
	.set_msglevel = ctcmac_set_msglevel,
	.get_link_ksettings = phy_ethtool_get_link_ksettings,
	.set_link_ksettings = phy_ethtool_set_link_ksettings,
	.get_ts_info = ctcmac_get_ts_info,
};

