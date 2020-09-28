#ifndef __CTC_SPI_H__
#define __CTC_SPI_H__
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/types.h>

#include "sal_common.h"

#define OCTEON_SPI_CLK_GPIO      18                  /*GPIO 18*/
#define OCTEON_SPI_DO_GPIO       20                  /*GPIO 20*/
#define OCTEON_SPI_DI_GPIO       19                   /*GPIO 19*/
#define OCTEON_SPI_CS_GPIO       22                   /*GPIO 22*/

struct spi_op_para_s
{
    uint32 addr;           /*  address of the spi component */    
    uint32 *val;           /* pointer of the value buffur */
};
typedef struct spi_op_para_s spi_op_para_t;

typedef enum
{
    E_SPI_GPIO = 0,        /* the SPI interface is implemented by GPIO pins */
    E_SPI_FPGA,             /* the SPI interface is implemented by ctrl fpga */
    E_SPI_EPLD,             /* the SPI interface is implemented by EPLD */
    E_SPI_CPU,           /* the SPI interface is implemented by CPU */
} spi_type_e;

typedef enum
{
    CMD_UINT16_VAL_UINT32,  /*cmd is uint16, and value is uint32*/
    CMD_UINT32_VAL_UINT8,  /*cmd is uint32, and value is uint8*/   
}spi_fpga_type_e;

struct spi_pin_info_s
{
   uint8 prot_index;     /*index == 0,means port a) */
   uint32 bit;            /* pin bit */
};
typedef struct spi_pin_info_s spi_pin_info_t;

enum spi_cpu_chip_sel_e
{
    SPI_CPU_CHIP_SEL_0,
    SPI_CPU_CHIP_SEL_1,
    SPI_CPU_CHIP_SEL_2,
    SPI_CPU_CHIP_SEL_3
};
typedef enum spi_cpu_chip_sel_e spi_cpu_chip_sel_t;

struct spi_epld_info_s
{
    uint8* baseaddr; 
    uint32 busy;
};
typedef struct spi_epld_info_s spi_epld_info_t;


struct spi_fpga_info_s
{
    uint32* baseaddr; 
    uint32 busy;
    spi_fpga_type_e fpga_type;
};
typedef struct spi_fpga_info_s spi_fpga_info_t;

struct spi_gpio_info_s
{
    int32 fd; 
};
typedef struct spi_gpio_info_s spi_gpio_info_t;

struct spi_cpu_info_s
{
    int32 fd;
    spi_cpu_chip_sel_t chip_sel;
};
typedef struct spi_cpu_info_s spi_cpu_info_t;

union spi_info_union
{    
    spi_fpga_info_t spi_fpga_info;
    spi_gpio_info_t spi_gpio_info;
    spi_epld_info_t spi_epld_info;
    spi_cpu_info_t spi_cpu_info;
};
typedef union spi_info_union spi_info_union_t;

struct spi_gen_s
{
    spi_type_e spi_type;     
    uint32 len;              /* spi data length (byte)*/    
    uint32 alen;             /* spi address length (byte)*/ 
    spi_info_union_t spi_info; 
};
typedef struct spi_gen_s spi_gen_t;

typedef struct spi_handle_s spi_handle_t;
struct spi_handle_s
{
    int32 (*close)(spi_handle_t *);
    int32 (*read)(const spi_handle_t *, spi_op_para_t *);
    int32 (*write)(const spi_handle_t *, spi_op_para_t *);
    void *data;
};

spi_handle_t *spi_create_handle(const spi_gen_t *spi_pgen);

#endif

