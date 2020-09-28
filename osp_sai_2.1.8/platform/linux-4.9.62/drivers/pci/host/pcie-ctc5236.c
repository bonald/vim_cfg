/*
 * pcie-ctc5236.c Support for pcie rc on Centec TsingMa SoC's
 *
 * Copyright (C) 2004-2017 Centec Networks (suzhou) Co., LTD.
 *
 * Author: Jay Cao <caoj@centecnetworks.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/msi.h>
#include <linux/of_address.h>
#include <linux/of_pci.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/irqchip/chained_irq.h>
#include <soc/ctc/sysctl.h>
#include <linux/regmap.h>
#include <linux/mfd/syscon.h>

#define CTC5236_PCIE_BUS		20
#define CTC5236_PCIE_DEV		15
#define CTC5236_PCIE_FUNC	       12

#define LTSSM_STATE_MASK	0xff
#define LTSSM_L0		0x10

#define CTC5236_CTRL_PCI_IDS	0x9c

#define CTC5236_CTRL_ATR_W0	0x600
#define CTC5236_CTRL_ATR_W1	0x700
#define CTC5236_CTRL_ATR_S0	0x800
#define CTC5236_CTRL_ATR_S1	0x900
#define CTC5236_CTRL_ATR_S1_T0	0x00
#define CTC5236_CTRL_ATR_S1_T2	0x20
#define CTC5236_CTRL_ATR_S1_T3	0x40
#define CTC5236_CTRL_ATR_S1_T4	0x60

/* Number of MSI IRQs */
#define CTC5236_NUM_MSI_IRQS		32

#define CTL_MSI_ADDR		0x18000190
#define CTL_ISTATUS_LOCAL	0x00000184
#define CTL_MSI_STATUS		0x00000194

struct ctc5236_msi {			/* MSI information */
	struct irq_domain *msi_domain;
	unsigned long *bitmap;
	struct irq_domain *dev_domain;
	struct mutex lock;		/* protect bitmap variable */
	int irq_msi;
};


struct ctc5236_pcie {
	struct device *dev;
	void __iomem *cfg_base;
	void __iomem *ctrl_base;
	struct regmap	*sysctl_regmap_base;
	phys_addr_t msi_addr;
	struct ctc5236_msi msi;
	u8			root_bus_nr;
	struct list_head	resources;
};

static inline u32 ctc5236_ctrl_readl(struct ctc5236_pcie *pcie, u32 off)
{
	return readl(pcie->ctrl_base + off);
}

static inline void ctc5236_ctrl_writel(struct ctc5236_pcie *pcie, u32 val, u32 off)
{
	writel(val, pcie->ctrl_base + off);
}

int ctc5236_pcie_link_up(struct ctc5236_pcie *pcie)
{
	u32 val;

	regmap_read(pcie->sysctl_regmap_base, offsetof(struct SysCtl_regs, SysPcieStatus), &val);

	val = val &LTSSM_STATE_MASK;
	if(val < LTSSM_L0)
		return 0;

	return 1;
	
}

static int ctc5236_pcie_valid_device(struct ctc5236_pcie *pcie, struct pci_bus *bus,
				int dev)
{
	/* If there is no link, then there is no device */
	if (bus->number != pcie->root_bus_nr) {
		if (!ctc5236_pcie_link_up(pcie))
			return 0;
	}

	/* access only one slot on each root port */
	if (bus->number == pcie->root_bus_nr && dev > 0)
		return 0;

	return 1;
}

static void __iomem *ctc5236_pcie_map_bus(struct pci_bus *bus, unsigned int devfn,
				      int where)
{
	struct ctc5236_pcie *pcie = bus->sysdata;
	int busn;

	if (!ctc5236_pcie_valid_device(pcie,bus, devfn))
		return NULL;

	busn = (bus->number << CTC5236_PCIE_BUS) |
			(devfn << CTC5236_PCIE_DEV);

	return pcie->cfg_base + busn + where;
}

/* PCIe operations */
static struct pci_ops ctc5236_pcie_ops = {
       .map_bus = ctc5236_pcie_map_bus,
	.read  = pci_generic_config_read,
	.write = pci_generic_config_write,
};

static struct irq_chip ctc5236_msi_irq_chip = {
	.name = "CTC5236 MSI",
	.irq_enable = unmask_msi_irq,
	.irq_disable = mask_msi_irq,
	.irq_mask = mask_msi_irq,
	.irq_unmask = unmask_msi_irq,

};

static struct msi_domain_info ctc5236_msi_domain_info = {
	.flags = (MSI_FLAG_USE_DEF_DOM_OPS | MSI_FLAG_USE_DEF_CHIP_OPS ),
	.chip = &ctc5236_msi_irq_chip,
};

static void ctc5236_compose_msi_msg(struct irq_data *data, struct msi_msg *msg)
{
	struct ctc5236_pcie *pcie = irq_data_get_irq_chip_data(data);
	phys_addr_t msi_addr = pcie->msi_addr;
	//printk("ctc5236_compose_msi_msg:%x,hwirq:%d\n",msi_addr,data->hwirq);
	msg->address_lo = lower_32_bits(msi_addr);
	msg->address_hi = upper_32_bits(msi_addr);
	msg->data = data->hwirq;
}

static int ctc5236_msi_set_affinity(struct irq_data *irq_data,
				const struct cpumask *mask, bool force)
{
	return -EINVAL;
}

static struct irq_chip ctc5236_irq_chip = {
	.name = "CTC5236 MSI",
	.irq_compose_msi_msg = ctc5236_compose_msi_msg,
	.irq_set_affinity = ctc5236_msi_set_affinity,
};

static int ctc5236_irq_domain_alloc(struct irq_domain *domain, unsigned int virq,
				unsigned int nr_irqs, void *args)
{
	struct ctc5236_pcie *pcie = domain->host_data;
	struct ctc5236_msi *msi = &pcie->msi;
	int bit;
	int i;
	//printk("ctc5236_irq_domain_alloc: virq:%d,nr_irqs:%d\n",virq,nr_irqs);
	mutex_lock(&msi->lock);
	bit = bitmap_find_next_zero_area(msi->bitmap, CTC5236_NUM_MSI_IRQS, 0,
					 nr_irqs, 0);
	if (bit >= CTC5236_NUM_MSI_IRQS) {
		mutex_unlock(&msi->lock);
		return -ENOSPC;
	}

	bitmap_set(msi->bitmap, bit, nr_irqs);

	for (i = 0; i < nr_irqs; i++) {
		irq_domain_set_info(domain, virq + i, bit + i, &ctc5236_irq_chip,
				domain->host_data, handle_simple_irq,
				NULL, NULL);
	}
	mutex_unlock(&msi->lock);
	return 0;
}

static void ctc5236_irq_domain_free(struct irq_domain *domain, unsigned int virq,
					unsigned int nr_irqs)
{
	struct irq_data *data = irq_domain_get_irq_data(domain, virq);
	struct ctc5236_pcie *pcie = irq_data_get_irq_chip_data(data);
	struct ctc5236_msi *msi = &pcie->msi;

	mutex_lock(&msi->lock);
	bitmap_clear(msi->bitmap, data->hwirq, nr_irqs);
	mutex_unlock(&msi->lock);
}

static const struct irq_domain_ops ctc5236_msi_domain_ops = {
	.alloc  = ctc5236_irq_domain_alloc,
	.free   = ctc5236_irq_domain_free,
};

static void ctc5236_pcie_handle_msi_irq(struct ctc5236_pcie *pcie, u32 status_reg)
{
	struct ctc5236_msi *msi;
	unsigned long status;
	u32 bit;
	u32 virq;
	u32 temp = 0x184;
	msi = &pcie->msi;

	status = ctc5236_ctrl_readl(pcie, status_reg);
	//printk("1 ctc5236_pcie_handle_msi_irq status:0x%x\n",status);	

	status = ctc5236_ctrl_readl(pcie, temp);
	//printk("1 ctc5236_pcie_handle_msi_irq local:0x%x\n",status);

	ctc5236_ctrl_writel(pcie, 0xffffffff, temp);
	
	while ((status = ctc5236_ctrl_readl(pcie, status_reg)) != 0) {
		//printk("2 ctc5236_pcie_handle_msi_irq status:0x%x\n",status);
		for_each_set_bit(bit, &status, 32) {
			ctc5236_ctrl_writel(pcie, 1 << bit, status_reg);

			status = ctc5236_ctrl_readl(pcie, temp);
	              //printk("2 ctc5236_pcie_handle_msi_irq local:0x%x\n",status);
	
			status = ctc5236_ctrl_readl(pcie, status_reg);
			
			virq = irq_find_mapping(msi->dev_domain, bit);
			//printk("write ctc5236_pcie_handle_msi_irq status:0x%x,virq:%d\n",status,virq);
			if (virq)
				generic_handle_irq(virq);
		}
	}
}

static void ctc5236_pcie_msi_handler(struct irq_desc *desc)
{
	struct irq_chip *chip = irq_desc_get_chip(desc);
	struct ctc5236_pcie *pcie = irq_desc_get_handler_data(desc);

	//printk("ctc5236_pcie_msi_handler inerrupt handler\n");
	
	chained_irq_enter(chip, desc);
	ctc5236_pcie_handle_msi_irq(pcie, CTL_MSI_STATUS);
	chained_irq_exit(chip, desc);
}


static int ctc5236_pcie_init_irq_domain(struct ctc5236_pcie *pcie)
{
	struct device *dev = pcie->dev;
	struct fwnode_handle *fwnode = of_node_to_fwnode(dev->of_node);
	struct ctc5236_msi *msi = &pcie->msi;

	msi->dev_domain = irq_domain_add_linear(NULL, CTC5236_NUM_MSI_IRQS,
						&ctc5236_msi_domain_ops, pcie);
	if (!msi->dev_domain) {
		dev_err(dev, "failed to create dev IRQ domain\n");
		return -ENOMEM;
	}
	msi->msi_domain = pci_msi_create_irq_domain(fwnode,
						    &ctc5236_msi_domain_info,
						    msi->dev_domain);
	if (!msi->msi_domain) {
		dev_err(dev, "failed to create msi IRQ domain\n");
		irq_domain_remove(msi->dev_domain);
		return -ENOMEM;
	}
	
	return 0;
}

static int ctc5236_pcie_enable_msi(struct ctc5236_pcie *pcie, struct pci_bus *bus)
{
	struct device *dev = pcie->dev;
	struct platform_device *pdev = to_platform_device(dev);
	struct ctc5236_msi *msi = &pcie->msi;
	int ret;
	
	int size = BITS_TO_LONGS(CTC5236_NUM_MSI_IRQS) ;

	mutex_init(&msi->lock);

	pcie->msi_addr = CTL_MSI_ADDR;

	//printk("ctc5236_pcie_enable_msi: msi->bitmap size:%d\n",size);
	msi->bitmap = kzalloc(size, GFP_KERNEL);
	if (!msi->bitmap)
		return -ENOMEM;

	msi->irq_msi = platform_get_irq_byname(pdev, "msi");
	//printk("ctc5236_pcie_enable_msi: msi->irq_msi:%d\n",msi->irq_msi);
	if (msi->irq_msi < 0) {
		dev_err(dev, "failed to get IRQ#%d\n", msi->irq_msi);
		ret = -EINVAL;
		goto err;
	}

	irq_set_chained_handler_and_data(msi->irq_msi,
					 ctc5236_pcie_msi_handler, pcie);

	
	/* Enable MSI */

	/* setup AFI/FPCI range */


	/*
	 * For  MSI interrupts: disable, clear any pending,
	 * and enable
	 */


	return 0;
err:
	kfree(msi->bitmap);
	msi->bitmap = NULL;
	return ret;
}


static int ctc5236_pcie_hw_init(struct ctc5236_pcie *pcie)
{

	/* setup the correct class code for the host bridge */
	writel(PCI_CLASS_BRIDGE_PCI << 16, pcie->ctrl_base+ CTC5236_CTRL_PCI_IDS);
    
	/* setup the correct inbound window size*/
	writel(0x00000045, pcie->ctrl_base+ CTC5236_CTRL_ATR_W1);

	return 0;
}

static int ctc5236_pcie_parse_request_of_pci_ranges(struct ctc5236_pcie *pcie)
{
	int err;
	struct device *dev = pcie->dev;
	struct device_node *np = dev->of_node;
	struct resource_entry *win, *tmp;
	resource_size_t iobase;

	INIT_LIST_HEAD(&pcie->resources);

	err = of_pci_get_host_bridge_resources(np, 0, 0xff, &pcie->resources,
					       &iobase);
	if (err)
		return err;

	err = devm_request_pci_bus_resources(dev, &pcie->resources);
	if (err)
		goto out_release_res;

	resource_list_for_each_entry_safe(win, tmp, &pcie->resources) {
		struct resource *res = win->res;

		switch (resource_type(res)) {
		case IORESOURCE_IO:
			err = pci_remap_iospace(res, iobase);
			if (err) {
				dev_warn(dev, "error %d: failed to map resource %pR\n",
					 err, res);
				resource_list_destroy_entry(win);
			}
			break;
		case IORESOURCE_MEM:
			//res_valid |= !(res->flags & IORESOURCE_PREFETCH);
			break;
		case IORESOURCE_BUS:
			pcie->root_bus_nr = res->start;
			break;
		}
	}
	
	return 0;

out_release_res:
	pci_free_resource_list(&pcie->resources);
	return err;
}

static int ctc5236_pcie_parse_dt(struct ctc5236_pcie *pcie,
			     struct platform_device *pdev)
{
	struct device *dev = pcie->dev;
	struct device_node *node = dev->of_node;
	struct resource *res;
	const char *type;

	/* Check for device type */
	type = of_get_property(node, "device_type", NULL);
	if (!type || strcmp(type, "pci")) {
		dev_err(dev, "invalid \"device_type\" %s\n", type);
		return -EINVAL;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "cfg");
	pcie->cfg_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(pcie->cfg_base))
		return PTR_ERR(pcie->cfg_base);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "ctrl");
	pcie->ctrl_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(pcie->ctrl_base))
		return PTR_ERR(pcie->ctrl_base);
	
	pcie->sysctl_regmap_base =syscon_regmap_lookup_by_phandle(node,"ctc,sysctrl");;
	if (IS_ERR(pcie->sysctl_regmap_base))
		return PTR_ERR(pcie->sysctl_regmap_base);


	return 0;
}

static const struct of_device_id ctc5236_pcie_of_match[] = {
	{ .compatible = "centec,ctc5236-pcie", },
	{}
};

static int ctc5236_pcie_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ctc5236_pcie *pcie;
	struct pci_bus *bus;
	struct pci_bus *child;
	int err;
	
	//printk("#######ctc5236_pcie_probe#######\n");

	
	pcie = devm_kzalloc(dev, sizeof(*pcie), GFP_KERNEL);
	if (!pcie)
		return -ENOMEM;

	pcie->dev = dev;

	err = ctc5236_pcie_parse_dt(pcie, pdev);
	if (err) {
		dev_err(dev, "Parsing DT failed\n");
		return err;
	}

	INIT_LIST_HEAD(&pcie->resources);
	
	err = ctc5236_pcie_hw_init(pcie);
	if (err) {
		dev_err(dev, "HW Initialization failed\n");
		return err;
	}

	err = ctc5236_pcie_init_irq_domain(pcie);
	if (err) {
		dev_err(dev, "Failed creating IRQ Domain\n");
		return err;
	}
	
	err = ctc5236_pcie_parse_request_of_pci_ranges(pcie);
	if (err) {
		dev_err(dev, "Getting bridge resources failed\n");
		return err;
	}

	//printk("#######ctc5236_pcie_probe    1#######\n");
	bus = pci_scan_root_bus(dev, pcie->root_bus_nr, &ctc5236_pcie_ops,
				pcie, &pcie->resources);
	if (!bus)
		return -ENOMEM;

	if (IS_ENABLED(CONFIG_PCI_MSI)) {
	err = ctc5236_pcie_enable_msi(pcie, bus);
	if (err < 0) {
		dev_err(dev, "failed to enable MSI support: %d\n", err);
		goto error;
	}
	}
		
	pci_bus_size_bridges(bus);
	//printk("#######ctc5236_pcie_probe    2#######\n");
	pci_bus_assign_resources(bus);
	//printk("#######ctc5236_pcie_probe    3#######\n");
	list_for_each_entry(child, &bus->children, node)
		{
		//printk("#######ctc5236_pcie_probe    4#######\n");
		pcie_bus_configure_settings(child);
		}
	//printk("#######ctc5236_pcie_probe    5#######\n");
	pci_bus_add_devices(bus);
	//printk("#######ctc5236_pcie_probe    6#######\n");
	return 0;

error:
	pci_free_resource_list(&pcie->resources);
	return err;
	
}

static struct platform_driver ctc5236_pcie_driver = {
	.driver = {
		.name = "ctc5236-pcie",
		.suppress_bind_attrs = true,
		.of_match_table = ctc5236_pcie_of_match,
	},
	.probe = ctc5236_pcie_probe,
};
builtin_platform_driver(ctc5236_pcie_driver);
