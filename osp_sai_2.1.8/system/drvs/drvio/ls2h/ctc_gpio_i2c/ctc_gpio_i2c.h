#ifndef _CTC_GPIO_I2C_H_
#define _CTC_GPIO_I2C_H_

#define CTC_GPIO_I2C_DEV_NAME     "/dev/ctc_gpio_i2c"
#define CTC_GPIO_I2C_MAJOR       115

#undef PDEBUG             /* undef it, just in case */
#undef CTC_GPIO_DEBUG
#ifdef CTC_GPIO_DEBUG
#ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#define PDEBUG(fmt, args...) printk( KERN_INFO "ctc_gpio_i2c: " fmt, ## args)
#else
     /* This one for user space */
#define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#endif
#else
#define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#define I2C_RDWR        0x0707
#define CTC_GPIO_I2C_RDWR   _IO(CTC_GPIO_I2C_MAJOR, 0)

/*  CTC_GPIO_I2C_RDWR  =  (0<<30 ) | (108 <<8) | (0<<0) |(0<<16)*/

#endif /* _CTC_GPIO_I2C_H_ */

