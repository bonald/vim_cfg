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
#include <linux/mii.h>

#include <asm/io.h>

#include "ctcmac.h"
#include "ctcmac_reg.h"



struct ctc_mdio_priv {
	void __iomem *map;
};

static int ctc_mdio_write(struct mii_bus *bus, int mii_id, int reg,
		u16 value)
{    
    return 0;
}

static int ctc_mdio_read(struct mii_bus *bus, int mii_id, int reg)
{
    int value = 0;

    if(reg == 0)
        value = 0x1040;
    else if(reg == 1)
        value = 0x796d;
    else if ((reg == 0x9) || (reg == 0xa))
        value = 0;
    else if(reg == 4)
        value = 0x01e1;
    else if(reg == 5)
        value = 0x4de1;
    else if(reg == 0xf)
        value = 0x3000;
    else 
        value = 0x00ff;

    return value; //TODO:  emulation board mdio is abnormal
}

static int ctc_mdio_reset(struct mii_bus *bus)
{    
    return 0;
}

static const struct of_device_id ctc_mdio_match[] = {
	{
		.compatible = "ctc,dummy-mdio",
	},
	{},
};
MODULE_DEVICE_TABLE(of, ctc_mdio_match);

static int ctc_mdio_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct resource res;
	struct ctc_mdio_priv *priv;
	struct mii_bus *new_bus;
	int err;
	
	new_bus = mdiobus_alloc_size(sizeof(*priv));
	if (!new_bus)
		return -ENOMEM;

	priv = new_bus->priv;
	new_bus->name = "CTC MII Bus",
	new_bus->read = &ctc_mdio_read;
	new_bus->write = &ctc_mdio_write;
	new_bus->reset = &ctc_mdio_reset;

	err = of_address_to_resource(np, 0, &res);
	if (err < 0) {
		pr_err("Of address to resource fail %d!\n", err);
		goto error;
	}

	snprintf(new_bus->id, MII_BUS_ID_SIZE, "%s@%llx", np->name,
		(unsigned long long)res.start);

	priv->map = of_iomap(np, 0);
	if (!priv->map) {
		err = -ENOMEM;
		pr_err("of iomap fail %d!\n", err);
		goto error;
	}

	new_bus->parent = &pdev->dev;
	platform_set_drvdata(pdev, new_bus);

	err = of_mdiobus_register(new_bus, np);
	if (err) {
		pr_err("register mdio bus fail %d!\n", err);
		goto error;
	}

	return 0;

error:
	if (priv->map)
		iounmap(priv->map);

	kfree(new_bus);

	return err;
}


static int ctc_mdio_remove(struct platform_device *pdev)
{
	struct device *device = &pdev->dev;
	struct mii_bus *bus = dev_get_drvdata(device);
	struct ctc_mdio_priv *priv = bus->priv;

	mdiobus_unregister(bus);

	iounmap(priv->map);
	mdiobus_free(bus);

	return 0;
}

static struct platform_driver ctc_mdio_driver = {
	.driver = {
		.name = "ctc_dummy_mdio",
		.of_match_table = ctc_mdio_match,
	},
	.probe = ctc_mdio_probe,
	.remove = ctc_mdio_remove,
};

module_platform_driver(ctc_mdio_driver);

MODULE_LICENSE("GPL");
