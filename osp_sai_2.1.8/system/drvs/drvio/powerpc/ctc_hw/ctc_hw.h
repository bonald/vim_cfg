#ifndef __CTC_HW_H__
#define __CTC_HW_H__

#define CTC_HW_DEV_NAME            "/dev/ctc_hw"
#define CTC_HW_DEV_MAJOR           100

#define EN_ASIC_NORMAL_INT          _IO(CTC_HW_DEV_MAJOR, 6)
#define EN_ASIC_FATAL_INT           _IO(CTC_HW_DEV_MAJOR, 7)
#define REG_ASIC_NORMAL_INT         _IO(CTC_HW_DEV_MAJOR, 8) 
#define REG_ASIC_FATAL_INT          _IO(CTC_HW_DEV_MAJOR, 9)
#define CTC_DIS_ALL_IRQS            _IO(CTC_HW_DEV_MAJOR, 10)
#define EN_FOAM_NORMAL_INT          _IO(CTC_HW_DEV_MAJOR, 11)
#define REG_FOAM_NORMAL_INT         _IO(CTC_HW_DEV_MAJOR, 12) 
#define CTC_GET_MEM_FREE            _IO(CTC_HW_DEV_MAJOR, 13)
#define CTC_CPU_MDIO_READ           _IO(CTC_HW_DEV_MAJOR, 14)
#define CTC_CPU_MDIO_WRITE          _IO(CTC_HW_DEV_MAJOR, 15)

#define CTC_HW_GET_PLATFORM_TYPE    _IO(CTC_HW_DEV_MAJOR, 16)

struct hw_cpu_mdio_cfg_ioctl_s
{
    unsigned int phy_addr;
    unsigned int reg;
    unsigned int value;
}; 
typedef struct hw_cpu_mdio_cfg_ioctl_s  hw_cpu_mdio_cfg_ioctl_t;                                                           \

#endif

