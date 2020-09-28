/*
 * (C) Copyright 2004-2017 Centec Networks (suzhou) Co., LTD.
 * Wangyb <wangyb@centecnetworks.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <soc/ctc/ctc5236_switch.h>

ctc_access_t *access;

#define 	SWITCH_DTS_OFFSET 0x1000

int ctc5236_switch_read(u32 offset, u32 len, u32* p_value)
{
	ctc_switch_cmd_status_u_t cmd_status_u;
	u32 timeout = 0x6400;
	u32 cmd_len = 0;
	u8 index = 0;

	if(!p_value){
		pr_err("switch read:value buffer is NULL!\n");
		return -1;
	}

	/* switch only have 16 databuf, len must not exceed 16 */
	if ((16 < len) || (0 == len))
	{
		pr_err("switch read: length error! len = %d \n", len);
		return -1;
	}
	/* cmdDataLen must be power of 2 */
	if ((len & (len - 1)) == 0)
	{
		cmd_len = len;
	}
	else
	{
		cmd_len = len;
		do
		{
			cmd_len++;
		}
		while ((cmd_len <= 16) && (cmd_len & (cmd_len - 1)));
	}

	/* 1. write CmdStatusReg */
	memset(&cmd_status_u, 0, sizeof(ctc_switch_cmd_status_u_t));
	cmd_status_u.cmd_status.cmdReadType = 1;
	cmd_status_u.cmd_status.cmdEntryWords = (len==16)?0:len;   /* normal operate only support 1 entry */
	cmd_status_u.cmd_status.cmdDataLen = len;
	writel(cmd_status_u.val, &access->cmd_status);
	/* 2. write AddrReg */
	writel(offset, &access->addr);
	/* 3. polling status and check */
	cmd_status_u.val = readl(&access->cmd_status);
	while (!(cmd_status_u.cmd_status.reqProcDone) && (--timeout))
	{
		cmd_status_u.val = readl(&access->cmd_status);
	}
	/* 4. check cmd done */
	if (!(cmd_status_u.cmd_status.reqProcDone))
	{
		pr_err("switch read error! cmd_status = %x\n", cmd_status_u.val);
		return -1;
	}
	/* 5. check pcie read status */
	if (cmd_status_u.cmd_status.reqProcError != 0)
	{
		pr_err("pci read error! cmd_status = %x, offset = 0x%x\n", cmd_status_u.val, offset);
		return -1;
	}

	/* 6. read data from buffer */
	for (index = 0; index < len; index++)
	{
		p_value[index] = readl(&access->data[index]);
	}
	
	return 0;
}


int ctc5236_switch_write(u32 offset, u32 len, u32* p_value)
{
	ctc_switch_cmd_status_u_t cmd_status_u;
	u32 timeout = 0x6400;  /* need to be confirmed */
	u32 cmd_len = 0;
	u8 index = 0;

	if(!p_value){
		pr_err("switch write:value buffer is NULL!\n");
		return -1;
	}

	/* switch only have 16 databuf, len must not exceed 16 */
	if ((16 < len) || (0 == len))
	{
		pr_err("switch write length error! len = %d \n", len);
		return -1;
	}

	/* cmdDataLen must be power of 2 */
	if ((len & (len - 1)) == 0)
	{
		cmd_len = len;
	}
	else
	{
		cmd_len = len;
		do
		{
			cmd_len++;
		}
		while ((cmd_len <= 16) && (cmd_len & (cmd_len - 1)));
	}

	/* 1. write CmdStatusReg */
	memset(&cmd_status_u, 0, sizeof(ctc_switch_cmd_status_t));
	cmd_status_u.cmd_status.cmdReadType = 0;
	cmd_status_u.cmd_status.cmdEntryWords = (len==16)?0:len;
	cmd_status_u.cmd_status.cmdDataLen = len; /* Notice: for 1 entry op, cmdDatalen eq cmdEntryWords, but for mutil entry, len = cmd_len */
	writel(cmd_status_u.val, &access->cmd_status);
	/* 2. write AddrReg */
	writel(offset, &access->addr);
	/* 3. write data into databuffer */
	for (index = 0; index < len; index++)
	{
		writel(p_value[index], &access->data[index]);
	}

	/* 4. polling status and check */
	cmd_status_u.val  = readl(&access->cmd_status);
	while (!(cmd_status_u.cmd_status.reqProcDone) && (--timeout))
	{
		cmd_status_u.val  = readl(&access->cmd_status);
	}

	/* 5. check cmd done */
	if (!(cmd_status_u.cmd_status.reqProcDone))
	{
		pr_err("switch write error! cmd_status = %x\n", cmd_status_u.val);
		return -1;
	}

	/* 6. check switch read status */
	if (cmd_status_u.cmd_status.reqProcError != 0)
	{
		pr_err("switch write error! cmd_status = %x, offset=0x%x\n", cmd_status_u.val, offset);
		return -1;
	}

	return 0;
}


static int ctc_switch_probe(struct platform_device *pdev)
{
	struct resource *iomem;
	void __iomem *ioaddr;
	resource_size_t start;
	uint val;
	
	iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0) ;

	start = iomem->start-0x1000;
	ioaddr = devm_ioremap(&pdev->dev, start, resource_size(iomem));
	if (IS_ERR(ioaddr)) {
		return -1;
	}
	access = (ctc_access_t *)ioaddr;

	return 0;
}


static const struct of_device_id ctc_switch_of_match[] = {
	{ .compatible = "centec,switch" },
	{ }
};

MODULE_DEVICE_TABLE(of, ctc_switch_of_match);

static struct platform_driver ctc_switch_driver = {
	.driver = {
		.name = "ctc-switch",
		.of_match_table = ctc_switch_of_match,
	},
	.probe = ctc_switch_probe,
};

module_platform_driver(ctc_switch_driver);

MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:switch");
