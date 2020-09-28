/*
 * Copyright (C) 2012 Altera Corporation
 * Copyright (c) 2011 Picochip Ltd., Jamie Iles
 *
 * Modified from mach-picoxcell/time.c
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
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <linux/delay.h>
#include <linux/dw_apb_timer.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/clk.h>
#include <linux/sched_clock.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>

/* guy add 2019/03/13 */
#define APBTMR_N_EOI			0x0c
#define APBTMR_N_CONTROL		0x08
#define APBTMR_N_LOAD_COUNT		0x00


static void __init timer_get_base_and_rate(struct device_node *np,
				    void __iomem **base, u32 *rate)
{
	struct clk *timer_clk;
	struct clk *pclk;
	u32 DivNum = 0x3c;

	*base = of_iomap(np, 0);

	if (!*base)
		panic("Unable to map regs for %s", np->name);

	/*
	 * Not all implementations use a periphal clock, so don't panic
	 * if it's not present
	 */
	pclk = of_clk_get_by_name(np, "pclk");
	if (!IS_ERR(pclk))
		if (clk_prepare_enable(pclk))
			pr_warn("pclk for %s is present, but could not be activated\n",
				np->name);

	timer_clk = of_clk_get_by_name(np, "timer");
	if (IS_ERR(timer_clk))
		goto try_clock_freq;

	/* Centec TsingMa SoC Timer, Modified by wangyb 2018/12/19 */
	if (!of_property_read_u32(np, "DivNum", &DivNum)){
		writel(DivNum, *base + APBTMR_N_DivNum);
	}
	
	if (!clk_prepare_enable(timer_clk)) {
		*rate = clk_get_rate(timer_clk) / DivNum;
		printk("CTC_DEBUG, timer_get_base_and_rate, iobase is %p, rate is 0x%x\n", *base, *rate);
		return;
	}

try_clock_freq:
	if (of_property_read_u32(np, "clock-freq", rate) &&
	    of_property_read_u32(np, "clock-frequency", rate))
		panic("No clock nor clock-frequency property for %s", np->name);
}

static void __init add_clockevent(struct device_node *event_timer)
{
	void __iomem *iobase;
	struct dw_apb_clock_event_device *ced;
	u32 irq, rate;

	irq = irq_of_parse_and_map(event_timer, 0);
	if (irq == 0)
		panic("No IRQ for clock event timer");

	timer_get_base_and_rate(event_timer, &iobase, &rate);

	ced = dw_apb_clockevent_init(0, event_timer->name, 300, iobase, irq,
				     rate);
	if (!ced)
		panic("Unable to initialise clockevent device");

	dw_apb_clockevent_register(ced);
}

static void __iomem *sched_io_base;
static u32 sched_rate;

static void __init add_clocksource(struct device_node *source_timer)
{
	void __iomem *iobase;
	struct dw_apb_clocksource *cs;
	u32 rate;

	timer_get_base_and_rate(source_timer, &iobase, &rate);

	cs = dw_apb_clocksource_init(300, source_timer->name, iobase, rate);
	if (!cs)
		panic("Unable to initialise clocksource device");

	dw_apb_clocksource_start(cs);
	dw_apb_clocksource_register(cs);

	/*
	 * Fallback to use the clocksource as sched_clock if no separate
	 * timer is found. sched_io_base then points to the current_value
	 * register of the clocksource timer.
	 */
	sched_io_base = iobase + 0x04;
	sched_rate = rate;
}

static u64 notrace read_sched_clock(void)
{
	return ~readl_relaxed(sched_io_base);
}

static const struct of_device_id sptimer_ids[] __initconst = {
	{ .compatible = "picochip,pc3x2-rtc" },
	{ /* Sentinel */ },
};

static void __init init_sched_clock(void)
{
	struct device_node *sched_timer;

	sched_timer = of_find_matching_node(NULL, sptimer_ids);
	if (sched_timer) {
		timer_get_base_and_rate(sched_timer, &sched_io_base,
					&sched_rate);
		of_node_put(sched_timer);
	}

	sched_clock_register(read_sched_clock, 32, sched_rate);
}

#ifdef CONFIG_ARM
static unsigned long dw_apb_delay_timer_read(void)
{
	return ~readl_relaxed(sched_io_base);
}

static struct delay_timer dw_apb_delay_timer = {
	.read_current_timer	= dw_apb_delay_timer_read,
};
#endif

static int num_called;
static int __init dw_apb_timer_init(struct device_node *timer)
{
	switch (num_called) {
	case 0:
		pr_debug("%s: found clockevent timer\n", __func__);
		add_clockevent(timer);
		break;
	case 1:
		pr_debug("%s: found clocksource timer\n", __func__);
		add_clocksource(timer);
		init_sched_clock();
#ifdef CONFIG_ARM
		dw_apb_delay_timer.freq = sched_rate;
		register_current_timer_delay(&dw_apb_delay_timer);
#endif
		break;
	default:
		break;
	}

	num_called++;

	return 0;
}

/*
 * Centec TsingMa SoC Timer Test, Modified by wangyb 2018/12/19
 */

struct timer_test_t{	
	void __iomem *iobase;
};

static int count = 0;
static irqreturn_t dw_apb_timer_irq(int irq, void *data)
{	
	struct timex txc;	
	struct rtc_time tm;	
	struct timer_test_t *timer_test = data;		

	do_gettimeofday(&(txc.time));	
	rtc_time_to_tm(txc.time.tv_sec, &tm);
	
	count ++;	
	printk("CTC_DEBUG, dw_apb_timer_irq, irq is %d, UTC time :%d-%d-%d %d:%d:%d, count is %d\n", 		
		irq, tm.tm_year+1900,tm.tm_mon, tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec, count);		

	//printk("CTC_DEBUG, timer_test->iobase is %p\n", timer_test->iobase);	
	readl(timer_test->iobase + APBTMR_N_EOI);	

	if(count == 3){
		/* Immediately_load mode */		
		//writel(0x0,  timer_test->iobase + APBTMR_N_CONTROL);		
		writel(0x2FAF080,  timer_test->iobase + APBTMR_N_Ext_Load);
		writel(0x2,  timer_test->iobase + APBTMR_N_Ext_Ctl);				
	}
	if(count == 6){
		printk("CTC_DEBUG, Stop Timer Tests\n");
		writel(0x6,  timer_test->iobase + APBTMR_N_CONTROL);	
	}
	
	return 0;
}

static int __init dw_apb_timer_test(struct device_node *timer)
{	
	u32 irq = 0, rate;	
	int ret, timerMode;	
	int DivNum = 0x3c;	
	int LoadCount = 0x0;	
	struct timer_test_t *timer_test = kzalloc(sizeof(*timer_test), GFP_KERNEL);	
	
	printk("CTC_DEBUG, dw_apb_timer_test, START\n");	
	timer_get_base_and_rate(timer, &(timer_test->iobase), &rate);	
	
	irq = irq_of_parse_and_map(timer, 0);
	ret = request_irq(irq, dw_apb_timer_irq, 0, "dw_apb_timer_irq", timer_test);	
	if (ret <0){
		printk("dw_apb_timer_irq failed\n");
		return -1;
	}		
			
	of_property_read_u32(timer, "timerMode", &timerMode);
	printk("CTC_DEBUG, timerMode is %d, irq is %d\n", timerMode, irq);	
	
	DivNum = readl(timer_test->iobase + APBTMR_N_DivNum);	
	if (DivNum == 0x3c)		
		LoadCount = 0x17D7840;         //25_000_000 times
	if (DivNum == 0x2)		
		LoadCount = 0x1DCD6500;	     //500_000_000 times
	if (DivNum == 0xfe)		
		LoadCount = 0x1E8480;	     //2_000_000 times
	
	if (timerMode == 2){		
		/* User-define mode */		
		printk("CTC_DEBUG, Timer into User-define mode, load count is %d\n", LoadCount);		
		writel(0x0,  timer_test->iobase + APBTMR_N_CONTROL);		
		writel(0x0,  timer_test->iobase + APBTMR_N_Ext_Ctl);		
		writel(LoadCount,  timer_test->iobase + APBTMR_N_LOAD_COUNT);		
		writel(0x3,  timer_test->iobase + APBTMR_N_CONTROL);	
		
	}else if (timerMode == 1){		
		/* Free-running mode */		
		printk("CTC_DEBUG, Timer into Free-running mode, load count is 0xFFFF_FFFF\n");		
		writel(0x0,  timer_test->iobase + APBTMR_N_CONTROL);		
		writel(0x0,  timer_test->iobase + APBTMR_N_Ext_Ctl);		
		writel(0x1,  timer_test->iobase + APBTMR_N_CONTROL);	
		
	}else if (timerMode == 3){	
		/* Once mode */		
		printk("CTC_DEBUG, Timer into Once mode, load count is %d\n", LoadCount);			
		writel(0x0,  timer_test->iobase + APBTMR_N_CONTROL);		
		writel(0x0,  timer_test->iobase + APBTMR_N_Ext_Ctl);		
		writel(LoadCount,  timer_test->iobase + APBTMR_N_LOAD_COUNT);		
		writel(0x3,  timer_test->iobase + APBTMR_N_CONTROL);		
		writel(0x1,  timer_test->iobase + APBTMR_N_Ext_Ctl);	
	}	
	
	return 0;
}

CLOCKSOURCE_OF_DECLARE(pc3x2_timer, "picochip,pc3x2-timer", dw_apb_timer_init);
CLOCKSOURCE_OF_DECLARE(apb_timer_osc, "snps,dw-apb-timer-osc", dw_apb_timer_init);
CLOCKSOURCE_OF_DECLARE(apb_timer_sp, "snps,dw-apb-timer-sp", dw_apb_timer_init);
CLOCKSOURCE_OF_DECLARE(apb_timer, "snps,dw-apb-timer", dw_apb_timer_init);
CLOCKSOURCE_OF_DECLARE(apb_timer_test, "snps,dw-apb-timer-test", dw_apb_timer_test);
