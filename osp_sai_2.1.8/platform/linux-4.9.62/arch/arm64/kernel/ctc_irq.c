#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of.h>
#include <linux/of_device.h>

#ifdef CONFIG_CTC_PLATFORM
#ifndef CONFIG_ARCH_CTC5236

int xirq_gpio_1_23 = 0;
int xirq_gpio_1_24 = 0;
int xirq_gpio_1_25 = 0;

static int ctc_irq_init(void)
{
    struct device_node *xnp;
    for_each_node_by_type(xnp, "irq") {         
        if (of_device_is_compatible(xnp, "fsl,ctc-irq"))
        {
            xirq_gpio_1_23 = irq_of_parse_and_map(xnp, 0); 
            printk(KERN_INFO "ctc-irq GPIO1_23 IRQ3 is %d\n", xirq_gpio_1_23);
            xirq_gpio_1_24 = irq_of_parse_and_map(xnp, 1); 
            printk(KERN_INFO "ctc-irq GPIO1_24 IRQ4 is %d\n", xirq_gpio_1_24);
            xirq_gpio_1_25 = irq_of_parse_and_map(xnp, 2); 
            printk(KERN_INFO "ctc-irq GPIO1_25 IRQ5 is %d\n", xirq_gpio_1_25);
        }
    } 
    return 0;
}

EXPORT_SYMBOL(xirq_gpio_1_23);
EXPORT_SYMBOL(xirq_gpio_1_24);
EXPORT_SYMBOL(xirq_gpio_1_25);

device_initcall(ctc_irq_init);

#endif
#endif
