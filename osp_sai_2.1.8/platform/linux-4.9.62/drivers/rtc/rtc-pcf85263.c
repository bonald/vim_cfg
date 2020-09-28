/*
 * rtc-pcf85263 Driver for the NXP PCF85263 RTC
 * Copyright 2016 Parkeon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/rtc.h>
#include <linux/i2c.h>
#include <linux/bcd.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regmap.h>


#define DRV_NAME "rtc-pcf85263"

/* Quartz capacitance */
#define PCF85263_QUARTZCAP_7pF		0
#define PCF85263_QUARTZCAP_6pF		1
#define PCF85263_QUARTZCAP_12p5pF	2

/* Quartz drive strength */
#define PCF85263_QUARTZDRIVE_NORMAL	0
#define PCF85263_QUARTZDRIVE_LOW	1
#define PCF85263_QUARTZDRIVE_HIGH	2


#define PCF85263_REG_RTC_SC	0x01	/* Seconds */
#define PCF85263_REG_RTC_SC_OS		BIT(7)	/* Oscilator stopped flag */

#define PCF85263_REG_RTC_MN	0x02	/* Minutes */
#define PCF85263_REG_RTC_HR	0x03	/* Hours */
#define PCF85263_REG_RTC_DT	0x04	/* Day of month 1-31 */
#define PCF85263_REG_RTC_DW	0x05	/* Day of week 0-6 */
#define PCF85263_REG_RTC_MO	0x06	/* Month 1-12 */
#define PCF85263_REG_RTC_YR	0x07	/* Year 0-99 */

#define PCF85263_REG_ALM1_SC	0x08	/* Seconds */
#define PCF85263_REG_ALM1_MN	0x09	/* Minutes */
#define PCF85263_REG_ALM1_HR	0x0a	/* Hours */
#define PCF85263_REG_ALM1_DT	0x0b	/* Day of month 1-31 */
#define PCF85263_REG_ALM1_MO	0x0c	/* Month 1-12 */

#define PCF85263_REG_ALM_CTL	0x10
#define PCF85263_REG_ALM_CTL_ALL_A1E	0x1f /* sec,min,hr,day,mon alarm 1 */

#define PCF85263_REG_OSC	0x25
#define PCF85263_REG_OSC_CL_MASK	(BIT(0) | BIT(1))
#define PCF85263_REG_OSC_CL_SHIFT	0
#define PCF85263_REG_OSC_OSCD_MASK	(BIT(2) | BIT(3))
#define PCF85263_REG_OSC_OSCD_SHIFT	2
#define PCF85263_REG_OSC_LOWJ		BIT(4)
#define PCF85263_REG_OSC_12H		BIT(5)

#define PCF85263_REG_PINIO	0x27
#define PCF85263_REG_PINIO_INTAPM_MASK	(BIT(0) | BIT(1))
#define PCF85263_REG_PINIO_INTAPM_SHIFT	0
#define PCF85263_INTAPM_INTA	(0x2 << PCF85263_REG_PINIO_INTAPM_SHIFT)
#define PCF85263_INTAPM_HIGHZ	(0x3 << PCF85263_REG_PINIO_INTAPM_SHIFT)
#define PCF85263_REG_PINIO_TSPM_MASK	(BIT(2) | BIT(3))
#define PCF85263_REG_PINIO_TSPM_SHIFT	2
#define PCF85263_TSPM_DISABLED		(0x0 << PCF85263_REG_PINIO_TSPM_SHIFT)
#define PCF85263_TSPM_INTB		(0x1 << PCF85263_REG_PINIO_TSPM_SHIFT)
#define PCF85263_REG_PINIO_CLKDISABLE	BIT(7)

#define PCF85263_REG_FUNCTION	0x28
#define PCF85263_REG_FUNCTION_COF_MASK	0x7
#define PCF85263_REG_FUNCTION_COF_OFF	0x7	/* No clock output */

#define PCF85263_REG_INTA_CTL	0x29
#define PCF85263_REG_INTB_CTL	0x2A
#define PCF85263_REG_INTx_CTL_A1E	BIT(4)	/* Alarm 1 */
#define PCF85263_REG_INTx_CTL_ILP	BIT(7)	/* 0=pulse, 1=level */

#define PCF85263_REG_FLAGS	0x2B
#define PCF85263_REG_FLAGS_A1F		BIT(5)

#define PCF85263_REG_RAM_BYTE	0x2c

#define PCF85263_REG_STOPENABLE 0x2e
#define PCF85263_REG_STOPENABLE_STOP	BIT(0)

#define PCF85263_REG_RESET	0x2f	/* Reset command */
#define PCF85263_REG_RESET_CMD_CPR	0xa4	/* Clear prescaler */

#define PCF85263_MAX_REG 0x2f

#define PCF85263_HR_PM		BIT(5)

enum pcf85263_irqpin {
	PCF85263_IRQPIN_NONE,
	PCF85263_IRQPIN_INTA,
	PCF85263_IRQPIN_INTB
};

static const char *const pcf85263_irqpin_names[] = {
	[PCF85263_IRQPIN_NONE] = "None",
	[PCF85263_IRQPIN_INTA] = "INTA",
	[PCF85263_IRQPIN_INTB] = "INTB"
};

struct pcf85263 {
	struct device *dev;
	struct rtc_device *rtc;
	struct regmap *regmap;
	enum pcf85263_irqpin irq_pin;
	int irq;
	bool mode_12h;
};

/*
 * Helpers to convert 12h to 24h and vice versa.
 * Values in register are stored in BCD with a PM flag in bit 5
 *
 * 23:00 <=> 11PM <=> 0x31
 * 00:00 <=> 12AM <=> 0x12
 * 01:00 <=> 1AM <=> 0x01
 * 12:00 <=> 12PM <=> 0x32
 * 13:00 <=> 1PM <=> 0x21
 */
static int pcf85263_bcd12h_to_bin24h(int regval)
{
	int hr = bcd2bin(regval & 0x1f);
	bool pm = regval & PCF85263_HR_PM;

	if (hr == 12)
		return pm ? 12 : 0;

	return pm ? hr + 12 : hr;
}

static int pcf85263_bin24h_to_bcd12h(int hr24)
{
	bool pm = hr24 >= 12;
	int hr12 = hr24 % 12;

	if (!hr12)
		hr12++;

	return bin2bcd(hr12) | pm ? 0 : PCF85263_HR_PM;
}

static int pcf85263_read_time(struct device *dev, struct rtc_time *tm)
{
	struct pcf85263 *pcf85263 = dev_get_drvdata(dev);
	const int first = PCF85263_REG_RTC_SC;
	const int last = PCF85263_REG_RTC_YR;
	const int len = last - first + 1;
	u8 regs[len];
	u8 hr_reg;
	int ret;

	ret = regmap_bulk_read(pcf85263->regmap, first, regs, len);
	if (ret)
		return ret;

	if (regs[PCF85263_REG_RTC_SC - first] & PCF85263_REG_RTC_SC_OS) {
		dev_warn(dev, "Oscillator stop detected, date/time is not reliable.\n");
		return -EINVAL;
	}

	tm->tm_sec = bcd2bin(regs[PCF85263_REG_RTC_SC - first] & 0x7f);
	tm->tm_min = bcd2bin(regs[PCF85263_REG_RTC_MN - first] & 0x7f);

	hr_reg = regs[PCF85263_REG_RTC_HR - first];
	if (pcf85263->mode_12h)
		tm->tm_hour = pcf85263_bcd12h_to_bin24h(hr_reg);
	else
		tm->tm_hour = bcd2bin(hr_reg & 0x3f);

	tm->tm_mday = bcd2bin(regs[PCF85263_REG_RTC_DT - first]);
	tm->tm_wday = bcd2bin(regs[PCF85263_REG_RTC_DW - first]);
	tm->tm_mon  = bcd2bin(regs[PCF85263_REG_RTC_MO - first]) - 1;
	tm->tm_year = bcd2bin(regs[PCF85263_REG_RTC_YR - first]);

	tm->tm_year += 100;  /* Assume 21st century */

	return 0;
}

static int pcf85263_set_time(struct device *dev, struct rtc_time *tm)
{
	struct pcf85263 *pcf85263 = dev_get_drvdata(dev);

	/*
	 * Before setting time need to stop RTC and disable prescaler
	 * Do this all in a single I2C transaction exploiting wraparound
	 * as described in data sheet.
	 * This means that the array below must be in register order
	 */
	u8 regs[] = {
		PCF85263_REG_STOPENABLE_STOP,	/* STOP */
		PCF85263_REG_RESET_CMD_CPR,	/* Disable prescaler */
		/* Wrap around to register 0 (1/100s) */
		0,				/* 1/100s always zero. */
		bin2bcd(tm->tm_sec),
		bin2bcd(tm->tm_min),
		bin2bcd(tm->tm_hour),		/* 24-hour */
		bin2bcd(tm->tm_mday),
		bin2bcd(tm->tm_wday + 1),
		bin2bcd(tm->tm_mon + 1),
		bin2bcd(tm->tm_year % 100)
	};
	int ret;

	ret = regmap_bulk_write(pcf85263->regmap, PCF85263_REG_STOPENABLE,
				regs, sizeof(regs));
	if (ret)
		return ret;

	/* As we have set the time in 24H update the hardware for that */
	if (pcf85263->mode_12h) {
		pcf85263->mode_12h = false;
		ret = regmap_update_bits(pcf85263->regmap, PCF85263_REG_OSC,
					 PCF85263_REG_OSC_12H, 0);
		if (ret)
			return ret;
	}

	/* Start it again */
	return regmap_write(pcf85263->regmap, PCF85263_REG_STOPENABLE, 0);
}

static int pcf85263_enable_alarm(struct pcf85263 *pcf85263, bool enable)
{
	int reg;
	int ret;

	ret = regmap_update_bits(pcf85263->regmap, PCF85263_REG_ALM_CTL,
				 PCF85263_REG_ALM_CTL_ALL_A1E,
				 enable ? PCF85263_REG_ALM_CTL_ALL_A1E : 0);
	if (ret)
		return ret;

	switch (pcf85263->irq_pin) {
	case PCF85263_IRQPIN_NONE:
		return 0;

	case PCF85263_IRQPIN_INTA:
		reg = PCF85263_REG_INTA_CTL;
		break;

	case PCF85263_IRQPIN_INTB:
		reg = PCF85263_REG_INTB_CTL;
		break;

	default:
		return -EINVAL;
	}

	return regmap_update_bits(pcf85263->regmap, reg,
				  PCF85263_REG_INTx_CTL_A1E,
				  enable ? PCF85263_REG_INTx_CTL_A1E : 0);
}

static int pcf85263_read_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
	struct pcf85263 *pcf85263 = dev_get_drvdata(dev);
	struct rtc_time *tm = &alarm->time;
	const int first = PCF85263_REG_ALM1_SC;
	const int last = PCF85263_REG_ALM1_MO;
	const int len = last - first + 1;
	u8 regs[len];
	u8 hr_reg;
	unsigned int regval;
	int ret;

	ret = regmap_bulk_read(pcf85263->regmap, first, regs, len);
	if (ret)
		return ret;

	tm->tm_sec = bcd2bin(regs[PCF85263_REG_ALM1_SC - first] & 0x7f);
	tm->tm_min = bcd2bin(regs[PCF85263_REG_ALM1_MN - first] & 0x7f);

	hr_reg = regs[PCF85263_REG_ALM1_HR - first];
	if (pcf85263->mode_12h)
		tm->tm_hour = pcf85263_bcd12h_to_bin24h(hr_reg);
	else
		tm->tm_hour = bcd2bin(hr_reg & 0x3f);

	tm->tm_mday = bcd2bin(regs[PCF85263_REG_ALM1_DT - first]);
	tm->tm_mon  = bcd2bin(regs[PCF85263_REG_ALM1_MO - first]) - 1;
	tm->tm_year = -1;
	tm->tm_wday = -1;

	ret = regmap_read(pcf85263->regmap, PCF85263_REG_ALM_CTL, &regval);
	if (ret)
		return ret;
	alarm->enabled = !!(regval & PCF85263_REG_ALM_CTL_ALL_A1E);

	ret = regmap_read(pcf85263->regmap, PCF85263_REG_FLAGS, &regval);
	if (ret)
		return ret;
	alarm->pending = !!(regval & PCF85263_REG_FLAGS_A1F);

	return 0;
}

static int pcf85263_set_alarm(struct device *dev, struct rtc_wkalrm *alarm)
{
	struct pcf85263 *pcf85263 = dev_get_drvdata(dev);
	struct rtc_time *tm = &alarm->time;
	const int first = PCF85263_REG_ALM1_SC;
	const int last = PCF85263_REG_ALM1_MO;
	const int len = last - first + 1;
	u8 regs[len];
	int ret;

	/* Disable alarm comparison during update */
	ret = pcf85263_enable_alarm(pcf85263, false);
	if (ret)
		return ret;

	/* Clear any pending alarm (write 0=>clr, 1=>no change) */
	ret = regmap_write(pcf85263->regmap, PCF85263_REG_FLAGS,
			   (unsigned int)(~PCF85263_REG_FLAGS_A1F));
	if (ret)
		return ret;

	/* Set the alarm time registers */
	regs[PCF85263_REG_ALM1_SC - first] = bin2bcd(tm->tm_sec);
	regs[PCF85263_REG_ALM1_MN - first] = bin2bcd(tm->tm_min);
	regs[PCF85263_REG_ALM1_HR - first] = pcf85263->mode_12h ?
			pcf85263_bin24h_to_bcd12h(tm->tm_hour) :
			bin2bcd(tm->tm_hour);
	regs[PCF85263_REG_ALM1_DT - first] = bin2bcd(tm->tm_mday);
	regs[PCF85263_REG_ALM1_MO - first] = bin2bcd(tm->tm_mon + 1);

	ret = regmap_bulk_write(pcf85263->regmap, first, regs, sizeof(regs));
	if (ret)
		return ret;

	if (alarm->enabled)
		ret = pcf85263_enable_alarm(pcf85263, true);

	return ret;
}

static int pcf85263_alarm_irq_enable(struct device *dev, unsigned int enable)
{
	struct pcf85263 *pcf85263 = dev_get_drvdata(dev);

	return pcf85263_enable_alarm(pcf85263, !!enable);
}

static irqreturn_t pcf85263_irq(int irq, void *data)
{
	struct pcf85263 *pcf85263 = data;
	unsigned int regval;
	int ret;

	ret = regmap_read(pcf85263->regmap, PCF85263_REG_FLAGS, &regval);
	if (ret)
		return IRQ_NONE;

	if (regval & PCF85263_REG_FLAGS_A1F) {
		regmap_write(pcf85263->regmap, PCF85263_REG_FLAGS,
			     (unsigned int)(~PCF85263_REG_FLAGS_A1F));

		rtc_update_irq(pcf85263->rtc, 1, RTC_IRQF | RTC_AF);

		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

static int pcf85263_check_osc_stopped(struct pcf85263 *pcf85263)
{
	unsigned int regval;
	int ret;

	ret = regmap_read(pcf85263->regmap, PCF85263_REG_RTC_SC, &regval);
	if (ret)
		return ret;

	ret = regval & PCF85263_REG_RTC_SC_OS ? 1 : 0;
	if (ret)
		dev_warn(pcf85263->dev, "Oscillator stop detected, date/time is not reliable.\n");

	return ret;
}

#ifdef CONFIG_RTC_INTF_DEV
static int pcf85263_ioctl(struct device *dev,
			  unsigned int cmd, unsigned long arg)
{
	struct pcf85263 *pcf85263 = dev_get_drvdata(dev);
	int ret;

	switch (cmd) {
	case RTC_VL_READ:
		ret = pcf85263_check_osc_stopped(pcf85263);
		if (ret < 0)
			return ret;

		if (copy_to_user((void __user *)arg, &ret, sizeof(int)))
			return -EFAULT;
		return 0;

	case RTC_VL_CLR:
		return regmap_update_bits(pcf85263->regmap,
					  PCF85263_REG_RTC_SC,
					  PCF85263_REG_RTC_SC_OS, 0);
	default:
		return -ENOIOCTLCMD;
	}
}
#else
#define pcf85263_ioctl NULL
#endif

static int pcf85263_init_hw(struct pcf85263 *pcf85263)
{
	struct device_node *np = pcf85263->dev->of_node;
	unsigned int regval;
	u32 propval;
	int ret;

	/* Determine if oscilator has been stopped (probably low power) */
	ret = pcf85263_check_osc_stopped(pcf85263);
	if (ret < 0) {
		/* Log here since this is the first hw access on probe */
		dev_err(pcf85263->dev, "Unable to read register\n");

		return ret;
	}

	/* Determine 12/24H mode */
	ret = regmap_read(pcf85263->regmap, PCF85263_REG_OSC, &regval);
	if (ret)
		return ret;
	pcf85263->mode_12h = !!(regval & PCF85263_REG_OSC_12H);

	/* Set oscilator register */
	regval &= ~PCF85263_REG_OSC_12H; /* keep current 12/24 h setting */

	propval = PCF85263_QUARTZCAP_12p5pF;
	of_property_read_u32(np, "quartz-load-capacitance", &propval);
	regval |= ((propval << PCF85263_REG_OSC_CL_SHIFT)
		    & PCF85263_REG_OSC_CL_MASK);

	propval = PCF85263_QUARTZDRIVE_NORMAL;
	of_property_read_u32(np, "quartz-drive-strength", &propval);
	regval |= ((propval << PCF85263_REG_OSC_OSCD_SHIFT)
		    & PCF85263_REG_OSC_OSCD_MASK);

	if (of_property_read_bool(np, "quartz-low-jitter"))
		regval |= PCF85263_REG_OSC_LOWJ;

	ret = regmap_write(pcf85263->regmap, PCF85263_REG_OSC, regval);
	if (ret)
		return ret;

	/* Set function register (RTC mode, 1s tick, clock output static) */
	ret = regmap_write(pcf85263->regmap, PCF85263_REG_FUNCTION,
			   PCF85263_REG_FUNCTION_COF_OFF);
	if (ret)
		return ret;

	/* Set all interrupts to disabled, level mode */
	ret = regmap_write(pcf85263->regmap, PCF85263_REG_INTA_CTL,
			   PCF85263_REG_INTx_CTL_ILP);
	if (ret)
		return ret;
	ret = regmap_write(pcf85263->regmap, PCF85263_REG_INTB_CTL,
			   PCF85263_REG_INTx_CTL_ILP);
	if (ret)
		return ret;

	/* Setup IO pin config register */
	regval = PCF85263_REG_PINIO_CLKDISABLE;
	switch (pcf85263->irq_pin) {
	case PCF85263_IRQPIN_INTA:
		regval |= (PCF85263_INTAPM_INTA | PCF85263_TSPM_DISABLED);
		break;
	case PCF85263_IRQPIN_INTB:
		regval |= (PCF85263_INTAPM_HIGHZ | PCF85263_TSPM_INTB);
		break;
	case PCF85263_IRQPIN_NONE:
		regval |= (PCF85263_INTAPM_HIGHZ | PCF85263_TSPM_DISABLED);
		break;
	}
	ret = regmap_write(pcf85263->regmap, PCF85263_REG_PINIO, regval);

	return ret;
}

static const struct rtc_class_ops rtc_ops = {
	.ioctl = pcf85263_ioctl,
	.read_time = pcf85263_read_time,
	.set_time = pcf85263_set_time,
	.read_alarm = pcf85263_read_alarm,
	.set_alarm = pcf85263_set_alarm,
	.alarm_irq_enable = pcf85263_alarm_irq_enable,
};

static const struct regmap_config pcf85263_regmap_cfg = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = PCF85263_MAX_REG,
};

/*
 * On some boards the interrupt line may not be wired to the CPU but only to
 * a power supply circuit.
 * In that case no interrupt will be specified in the device tree but the
 * wakeup-source DT property may be used to enable wakeup programming in
 * sysfs
 */
static bool pcf85263_can_wakeup_machine(struct pcf85263 *pcf85263)
{
	return pcf85263->irq ||
		of_property_read_bool(pcf85263->dev->of_node, "wakeup-source");
}

static int pcf85263_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct pcf85263 *pcf85263;
	int ret;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C |
				     I2C_FUNC_SMBUS_BYTE_DATA |
				     I2C_FUNC_SMBUS_I2C_BLOCK))
		return -ENODEV;

	pcf85263 = devm_kzalloc(dev, sizeof(*pcf85263), GFP_KERNEL);
	if (!pcf85263)
		return -ENOMEM;

	pcf85263->dev = dev;
	pcf85263->irq = client->irq;
	dev_set_drvdata(dev, pcf85263);

	pcf85263->regmap = devm_regmap_init_i2c(client, &pcf85263_regmap_cfg);
	if (IS_ERR(pcf85263->regmap)) {
		ret = PTR_ERR(pcf85263->regmap);
		dev_err(dev, "regmap allocation failed (%d)\n", ret);

		return ret;
	}

	/* Determine which interrupt pin the board uses */
	if (pcf85263_can_wakeup_machine(pcf85263)) {
		if (of_property_match_string(dev->of_node,
					     "interrupt-names", "INTB") >= 0)
			pcf85263->irq_pin = PCF85263_IRQPIN_INTB;
		else
			pcf85263->irq_pin = PCF85263_IRQPIN_INTA;
	} else {
		pcf85263->irq_pin = PCF85263_IRQPIN_NONE;
	}

	ret = pcf85263_init_hw(pcf85263);
	if (ret)
		return ret;

	if (pcf85263->irq) {
		ret = devm_request_threaded_irq(dev, pcf85263->irq, NULL,
						pcf85263_irq,
						IRQF_ONESHOT,
						dev->driver->name, pcf85263);
		if (ret) {
			dev_err(dev, "irq %d unavailable (%d)\n",
				pcf85263->irq, ret);
			pcf85263->irq = 0;
		}
	}

	if (pcf85263_can_wakeup_machine(pcf85263))
		device_init_wakeup(dev, true);

	pcf85263->rtc = devm_rtc_device_register(dev, dev->driver->name,
						 &rtc_ops, THIS_MODULE);
	ret = PTR_ERR_OR_ZERO(pcf85263->rtc);
	if (ret)
		return ret;

	/* We cannot support UIE mode if we do not have an IRQ line */
	if (!pcf85263->irq)
		pcf85263->rtc->uie_unsupported = 1;

	dev_info(pcf85263->dev,
		 "PCF85263 RTC (irqpin=%s irq=%d)\n",
		 pcf85263_irqpin_names[pcf85263->irq_pin],
		 pcf85263->irq);

	return 0;
}

static int pcf85263_remove(struct i2c_client *client)
{
	struct pcf85263 *pcf85263 = i2c_get_clientdata(client);

	if (pcf85263_can_wakeup_machine(pcf85263))
		device_init_wakeup(pcf85263->dev, false);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int pcf85263_suspend(struct device *dev)
{
	struct pcf85263 *pcf85263 = dev_get_drvdata(dev);
	int ret = 0;

	if (device_may_wakeup(dev))
		ret = enable_irq_wake(pcf85263->irq);

	return ret;
}

static int pcf85263_resume(struct device *dev)
{
	struct pcf85263 *pcf85263 = dev_get_drvdata(dev);
	int ret = 0;

	if (device_may_wakeup(dev))
		ret = disable_irq_wake(pcf85263->irq);

	return ret;
}

#endif

static const struct i2c_device_id pcf85263_id[] = {
	{ "pcf85263", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, pcf85263_id);

#ifdef CONFIG_OF
static const struct of_device_id pcf85263_of_match[] = {
	{ .compatible = "nxp,pcf85263" },
	{}
};
MODULE_DEVICE_TABLE(of, pcf85263_of_match);
#endif

static SIMPLE_DEV_PM_OPS(pcf85263_pm_ops, pcf85263_suspend,  pcf85263_resume);

static struct i2c_driver pcf85263_driver = {
	.driver		= {
		.name	= "rtc-pcf85263",
		.of_match_table = of_match_ptr(pcf85263_of_match),
		.pm = &pcf85263_pm_ops,
	},
	.probe		= pcf85263_probe,
	.remove		= pcf85263_remove,
	.id_table	= pcf85263_id,
};

module_i2c_driver(pcf85263_driver);

MODULE_AUTHOR("Martin Fuzzey <mfuzzey@parkeon.com>");
MODULE_DESCRIPTION("PCF85263 RTC Driver");
MODULE_LICENSE("GPL");

