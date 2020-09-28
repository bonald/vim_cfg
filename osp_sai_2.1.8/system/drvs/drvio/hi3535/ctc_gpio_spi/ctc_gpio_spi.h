#ifndef _CTC_GPIO_SPI_H_
#define _CTC_GPIO_SPI_H_

#define CTC_GPIO_SPI_DEV_NAME     "/dev/ctc_gpio_spi"
/*
 * Macros to help debugging
 */

#undef PDEBUG             /* undef it, just in case */
#undef CTC_GPIO_SPI_DEBUG
//#define CTC_GPIO_SPI_DEBUG
#ifdef CTC_GPIO_SPI_DEBUG
/* This one if debugging is on, and kernel space */
#define PDEBUG(fmt, args...) printk( KERN_INFO "\nctc_gpio_spi: " fmt, ## args)
#else
#define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

#define CTC_GPIO_SPI_MAJOR       115

/*
 * Split minors in two parts
 */
#define TYPE(minor)	(((minor) >> 4) & 0xf)	/* high nibble */
#define NUM(minor)	((minor) & 0xf)		/* low  nibble */




#define CTC_GPIO_SPI_RW        _IO(CTC_GPIO_SPI_MAJOR, 15)


#endif /* _CTC_GPIO_H_ */

