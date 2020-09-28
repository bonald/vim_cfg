#ifndef _CTC_GPIO_H_
#define _CTC_GPIO_H_




#define CTC_GPIO_DEV_NAME     "/dev/ctc_gpio"
/*
 * Macros to help debugging
 */

#undef PDEBUG             /* undef it, just in case */
#undef CTC_GPIO_DEBUG
#ifdef CTC_GPIO_DEBUG
#ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#define PDEBUG(fmt, args...) printk( KERN_DEBUG "ctc_gpio: " fmt, ## args)
#else
     /* This one for user space */
#define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#endif
#else
#define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

#define CTC_GPIO_MAJOR       106
#define CTC_GPIO_NR_DEVS     1

#define CTC_OCTEON_GPIO_ADDR  ((uint32_t)0x1070000000800)
#define CTC_OCTEON_GPIO_SIZE  0x1000

/*
 * Split minors in two parts
 */
#define TYPE(minor)	(((minor) >> 4) & 0xf)	/* high nibble */
#define NUM(minor)	((minor) & 0xf)		/* low  nibble */




#define CTC_GPIO_READ        _IO(CTC_GPIO_MAJOR, 15)
#define CTC_GPIO_WRITE       _IO(CTC_GPIO_MAJOR, 16)
#define CTC_GPIO_GPIO_INIT       _IO(CTC_GPIO_MAJOR, 17)

#endif /* _CTC_GPIO_H_ */

