/****************************************************************************
 * epld_drv.c    Control epld access interface
 *
 * Copyright:    (c)2005 Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision:     R0.01.
 * Author:       Zhu Jian
 * Date:         2010-08-26.
 * Reason:       First Create.
 ****************************************************************************/

/****************************************************************************
 *
* Header Files 
*
****************************************************************************/
#include "sal_common.h"
#include "ctc_pci.h"
#include "drv_debug.h"
#include "epld_drv.h"
#include "ctc_spi.h"
#include "ctc_i2c.h"

/****************************************************************************
 *
* Defines and Macros 
*
****************************************************************************/

/****************************************************************************
 *
* Global and Declarations  
*
****************************************************************************/
static epld_handle_t* epld_create_handle(void* epld_dev_info);

/****************************************************************************
 *
* Functions  
*
****************************************************************************/

static void gpio_delay()
{ 
    volatile int32 loop = 0x10;
    while(loop--);
}

int32
epld_drv_write(const epld_handle_t* phdl, epld_access_t* paccess)
{
    epld_info_t* p_epld_info = NULL;
    
    if(NULL == phdl || NULL == paccess || NULL == phdl->info)
    {
        DRV_LOG_ERR("EPLD pci write: invalid pointer\n");
        return  -1;
    }

    p_epld_info = (epld_info_t*)phdl->info;
    PCIIO_WR_8(p_epld_info->base_addr + paccess->addr, paccess->val);

    return 0;
}

int32
epld_drv_read(const epld_handle_t* phdl, epld_access_t* paccess)
{
    epld_info_t* p_epld_info = NULL;
    
    if(NULL == phdl || NULL == paccess || NULL == phdl->info)
    {
        DRV_LOG_ERR("EPLD pci read: invalid pointer\n");
        return -1;
    }
    
    p_epld_info = (epld_info_t*)phdl->info;
    PCIIO_RD_8(p_epld_info->base_addr + paccess->addr, paccess->val);

    return 0;
}

/*****************************************************************************
 * Name         :   epld_drv_item_read
 * Purpose      :   read value from epld's register 
 * Input        :   epld handler
 *              :   op_reg  define in pci_drv.h
 * Output       :   value
 * Return       :   Success/Error Number 
 * Note         :   Function read value from epld's responed bit of item_no
 *              :   and adjust the value to the lowest bit in parameter 
 *****************************************************************************/
int32 
epld_drv_item_read(const epld_handle_t* phdl, uint32 op_reg, int32* value)
{
    uint8 offset;
    int32 start_bit;
    int32 item_bitwidth;
    int8 epld_reg_value;
    epld_info_t* p_epld_info = NULL; 
    
    if(NULL == phdl || NULL == phdl->info)
    {
        DRV_LOG_ERR("epld read item: invalid pointer\n");
        return -1;
    }

    p_epld_info= phdl->info;
    if(0 == p_epld_info->reg_desc[op_reg].reg_valid)
    {
        *value = 0;
        return -1;
    }
    offset = p_epld_info->reg_desc[op_reg].reg_offset;
    start_bit = p_epld_info->reg_desc[op_reg].start_bit;
    item_bitwidth = p_epld_info->reg_desc[op_reg].item_bitwidth;
    epld_reg_value = 0;
    
    PCIIO_RD_8(p_epld_info->base_addr + offset*4, epld_reg_value);
    
    epld_reg_value >>= start_bit;

    epld_reg_value &= ((1 << item_bitwidth) - 1);

    *value = (int32)epld_reg_value;
   
    return 0;
}

/*****************************************************************************
 * Name         :   epld_drv_item_write
 * Purpose      :   write value into epld's register 
 * Input        :   epld epld handler        
 *              :   op_reg  defined in epld_drv.h
 *              :   value   the value to be write
 * Output       :   
 * Return       :   Success/Error number 
 * Note         :   The value to be write is stored from the bit0 
 *              :   function will adjust to the responed bit in epld
 * *****************************************************************************/
int32 
epld_drv_item_write(const epld_handle_t *phdl, uint32 op_reg, int32 value)
{
    uint8 offset;
    int32 start_bit;
    int32 item_bitwidth;
    int8 epld_reg_value;    
    uint32 temp_value = 0;
    epld_info_t *p_epld_info = NULL;
    
    if(NULL == phdl || NULL == phdl->info)
    {
        DRV_LOG_ERR("epld write item: invalid pointer\n");
        return -1;
    }

    p_epld_info = phdl->info;
    if(0 == p_epld_info->reg_desc[op_reg].reg_valid)
    {
        return -1;
    }
    offset = p_epld_info->reg_desc[op_reg].reg_offset;
    start_bit = p_epld_info->reg_desc[op_reg].start_bit;
    item_bitwidth = p_epld_info->reg_desc[op_reg].item_bitwidth;
    epld_reg_value = 0;
    
    PCIIO_RD_8(p_epld_info->base_addr + offset*4, epld_reg_value);
   
    value <<= start_bit;
    temp_value = (((1 << item_bitwidth) - 1) << start_bit);
    epld_reg_value &= (~temp_value);
    epld_reg_value |= value;

    PCIIO_WR_8(p_epld_info->base_addr + offset*4, epld_reg_value);

    return 0;
}

int32
localbus_epld_read(const epld_handle_t *phdl, epld_access_t *paccess)
{   
    epld_info_t *p_epld_info = NULL;
    
    if(NULL == phdl || NULL == paccess || NULL == phdl->info)
    {
        DRV_LOG_ERR("EPLD localbus read: invalid pointer\n");
        return -1;
    }

    p_epld_info = (epld_info_t *)phdl->info;
    paccess->val = *((uint8_t *)(p_epld_info->base_addr)+ paccess->addr);
    return 0;
}

int32
localbus_epld_write(const epld_handle_t *phdl, epld_access_t *paccess)
{   
    epld_info_t *p_epld_info = NULL;
    
    if(NULL == phdl || NULL == paccess || NULL == phdl->info)
    {
        DRV_LOG_ERR("EPLD localbus write: invalid pointer\n");
        return  -1;
    }

    p_epld_info = (epld_info_t *)phdl->info;
    *((uint8_t *)(p_epld_info->base_addr) + paccess->addr) = paccess->val;
    return 0;
}


int32 
localbus_epld_item_read(const epld_handle_t *phdl, uint32_t op_reg, int32 * value)
{
    int32 offset;
    int32 start_bit;

    int32 item_bitwidth;
    uint8_t epld_reg_value;
    epld_info_t *p_epld_info = NULL; 
    
    if(NULL == phdl || NULL == phdl->info)
    {
        printf("EPLD read item: invalid pointer\n");
        return -1;
    }
    
    p_epld_info = (epld_info_t *)phdl->info;
    if(0 == p_epld_info->reg_desc[op_reg].reg_valid)
    {
        *value = 0;
        return -1;
    }
    offset = p_epld_info->reg_desc[op_reg].reg_offset;
    start_bit = p_epld_info->reg_desc[op_reg].start_bit;

    item_bitwidth = p_epld_info->reg_desc[op_reg].item_bitwidth;
    epld_reg_value = 0;
    
    epld_reg_value = *((uint8_t *)(p_epld_info->base_addr) + offset);
    

    
    epld_reg_value >>= start_bit ;

    epld_reg_value &= ((1 << item_bitwidth) - 1);
    
    *value = (int32)epld_reg_value;
   
    return 0;
}


int32 
localbus_epld_item_write(const epld_handle_t *phdl, uint32_t op_reg,  int32 value)
{
    int32 offset;
    int32 start_bit;
    int32 item_bitwidth;
    uint8_t epld_reg_value;    
    uint8_t temp_value = 0;
    epld_info_t *p_epld_info = NULL; 

    if(NULL == phdl || NULL == phdl->info)
    {
        printf("EPLD write item: invalid pointer\n");
        return -1;
    }

    p_epld_info= phdl->info;
    if(0 == p_epld_info->reg_desc[op_reg].reg_valid)
    {
        return -1;
    }
    offset = p_epld_info->reg_desc[op_reg].reg_offset;
    start_bit = p_epld_info->reg_desc[op_reg].start_bit;
    item_bitwidth = p_epld_info->reg_desc[op_reg].item_bitwidth;
    epld_reg_value = 0;
    
    epld_reg_value = *((uint8_t *)(p_epld_info->base_addr) + offset);
    

    value <<= start_bit ;

    temp_value =( ((1 << item_bitwidth) - 1) << start_bit );
    value &= temp_value;
    
    epld_reg_value &= (~temp_value);
    epld_reg_value |= value;
    *((uint8_t *)(p_epld_info->base_addr)  + offset) = epld_reg_value;
    return 0;
}

int32
gpio_epld_read(const epld_handle_t *phdl, epld_access_t *paccess)
{   
    spi_handle_t* spi_phdl = NULL;
    spi_op_para_t spi_para;  
    epld_info_t *p_epld_info = NULL; 
    
    if(NULL == phdl || NULL == paccess || NULL == phdl->info || NULL == phdl->handle)
    {
        DRV_LOG_ERR("EPLD gpio spi read: invalid pointer\n");
        return -1;
    }

    p_epld_info = (epld_info_t *)phdl->info;

    /*jcao first switch to EPLD before you do real spi operation*/
    *((uint8 *)(p_epld_info ->base_addr + EPLD_SPI_SWITCH)) = p_epld_info->switch_value;
    gpio_delay();
    *((uint8 *)(p_epld_info ->base_addr + 0xa)) = 0x1;
    *((uint8 *)(p_epld_info ->base_addr + 0xa)) = 0x0;
    
    spi_phdl = (spi_handle_t *)phdl->handle;
      
    spi_para.addr= (paccess->addr) | (EPLD_SPI_READ<<7);//from hardware team    
    spi_para.val = (uint32 *)(&(paccess->val));
      
    return spi_phdl->read(spi_phdl, &spi_para);
}

int32
gpio_epld_write(const epld_handle_t *phdl, epld_access_t *paccess)
{   
    spi_handle_t* spi_phdl = NULL;
    spi_op_para_t spi_para;
    epld_info_t *p_epld_info = NULL; 
    
    if(NULL == phdl || NULL == paccess || NULL == phdl->info || NULL == phdl->handle)
    {
        DRV_LOG_ERR("EPLD gpio spi write: invalid pointer\n");
        return  -1;
    }

    p_epld_info = (epld_info_t *)phdl->info;

    /*jcao first switch to EPLD before you do real spi operation*/
    *((uint8 *)(p_epld_info ->base_addr + EPLD_SPI_SWITCH)) = p_epld_info->switch_value;
    gpio_delay();
    *((uint8 *)(p_epld_info ->base_addr + 0xa)) = 0x1;
    *((uint8 *)(p_epld_info ->base_addr + 0xa)) = 0x0;
    
    spi_phdl = (spi_handle_t *)phdl->handle;
    
    spi_para.addr= (paccess->addr) | (EPLD_SPI_WRITE<<7);;//from hardware team    
    spi_para.val = (uint32 *)(&(paccess->val));  
    
    return spi_phdl->write(spi_phdl, &spi_para);
}


int32 
gpio_epld_item_read(const epld_handle_t *phdl, uint32_t op_reg, int32 * value)
{
    int32 offset;
    int32 start_bit;

    int32 item_bitwidth;
    uint32 epld_reg_value;
    epld_info_t *p_epld_info = NULL; 

    spi_handle_t* spi_phdl = NULL;
    spi_op_para_t spi_para; 
    int32 ret = 0;
    
    if(NULL == phdl || NULL == phdl->info || NULL == phdl->handle)
    {
        printf("EPLD gpio spi read item: invalid pointer\n");
        return -1;
    }
    
    p_epld_info = (epld_info_t *)phdl->info;
    if(0 == p_epld_info->reg_desc[op_reg].reg_valid)
    {
        *value = 0;
        return -1;
    }
    offset = p_epld_info->reg_desc[op_reg].reg_offset;
    start_bit = p_epld_info->reg_desc[op_reg].start_bit;

    item_bitwidth = p_epld_info->reg_desc[op_reg].item_bitwidth;
    epld_reg_value = 0;

    /*jcao first switch to EPLD before you do real spi operation*/
    *((uint8 *)(p_epld_info ->base_addr + EPLD_SPI_SWITCH)) = p_epld_info->switch_value;
    gpio_delay();
    *((uint8 *)(p_epld_info ->base_addr + 0xa)) = 0x1;
    *((uint8 *)(p_epld_info ->base_addr + 0xa)) = 0x0;
    
    spi_phdl = (spi_handle_t *)phdl->handle;
      
    spi_para.addr= (offset) | (EPLD_SPI_READ<<7);//from hardware team    
    spi_para.val = (uint32 *)(&epld_reg_value);

    ret = spi_phdl->read(spi_phdl, &spi_para);
    
    epld_reg_value >>= start_bit ;

    epld_reg_value &= ((1 << item_bitwidth) - 1);
    
    *value = (int32)epld_reg_value;
   
    return ret;
}


int32 
gpio_epld_item_write(const epld_handle_t *phdl, uint32_t op_reg,  int32 value)
{
    int32 offset;
    int32 start_bit;
    int32 item_bitwidth;
    uint32 epld_reg_value;    
    uint32 temp_value = 0;
    epld_info_t *p_epld_info = NULL; 

    spi_handle_t* spi_phdl = NULL;
    spi_op_para_t spi_para; 
    int32 ret = 0;

    if(NULL == phdl || NULL == phdl->info || NULL == phdl->handle)
    {
        printf("EPLD gpio spi write item: invalid pointer\n");
        return -1;
    }

    p_epld_info= phdl->info;
    if(0 == p_epld_info->reg_desc[op_reg].reg_valid)
    {
        return -1;
    }
    offset = p_epld_info->reg_desc[op_reg].reg_offset;
    start_bit = p_epld_info->reg_desc[op_reg].start_bit;
    item_bitwidth = p_epld_info->reg_desc[op_reg].item_bitwidth;
    epld_reg_value = 0;
    
    /*jcao first switch to EPLD before you do real spi operation*/
    *((uint8 *)(p_epld_info ->base_addr + EPLD_SPI_SWITCH)) = p_epld_info->switch_value;
    gpio_delay();
    *((uint8 *)(p_epld_info ->base_addr + 0xa)) = 0x1;
    *((uint8 *)(p_epld_info ->base_addr + 0xa)) = 0x0;
    
    spi_phdl = (spi_handle_t *)phdl->handle;
      
    spi_para.addr= (offset) | (EPLD_SPI_READ<<7);//from hardware team        
    spi_para.val = (uint32 *)(&epld_reg_value);
    

    ret += spi_phdl->read(spi_phdl, &spi_para);
    

    value <<= start_bit ;

    temp_value =( ((1 << item_bitwidth) - 1) << start_bit );
    
    epld_reg_value &= (~temp_value);
    epld_reg_value |= value;
    
    spi_para.addr= (offset) | (EPLD_SPI_WRITE<<7);//from hardware team    
    spi_para.val = (uint32 *)(&epld_reg_value);

    ret += spi_phdl->write(spi_phdl, &spi_para);
    return ret ;
}


int32
i2c_epld_read(const epld_handle_t *phdl, epld_access_t *paccess)
{   
    i2c_handle_t* i2c_phdl = NULL;
    i2c_op_para_t i2c_para;  
    epld_info_t *p_epld_info = NULL; 
    int32 ret = 0;
    uint8 value;
    
    if(NULL == phdl || NULL == paccess || NULL == phdl->info || NULL == phdl->handle)
    {
        DRV_LOG_ERR("EPLD gpio spi read: invalid pointer\n");
        return -1;
    }
    p_epld_info = (epld_info_t *)phdl->info;
    i2c_phdl = (i2c_handle_t *)phdl->handle;
    
    i2c_para.offset = paccess->addr;
    i2c_para.len = p_epld_info->i2c_gen.alen;
    i2c_para.p_val = &value;
  
    ret = i2c_phdl->read(i2c_phdl, &i2c_para);
    
    paccess->val = (uint32)value;
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

int32
i2c_epld_write(const epld_handle_t *phdl, epld_access_t *paccess)
{   
    i2c_handle_t* i2c_phdl = NULL;
    i2c_op_para_t i2c_para;
    epld_info_t *p_epld_info = NULL; 
    int32 ret = 0;
    uint8 value;
    
    if(NULL == phdl || NULL == paccess || NULL == phdl->info || NULL == phdl->handle)
    {
        DRV_LOG_ERR("EPLD gpio spi write: invalid pointer\n");
        return  -1;
    }
    p_epld_info = (epld_info_t *)phdl->info;
    i2c_phdl = (i2c_handle_t *)phdl->handle;
    i2c_para.offset = paccess->addr;
    i2c_para.len = p_epld_info->i2c_gen.alen;
    value = (uint8)paccess->val;
    i2c_para.p_val = &value;
    ret = i2c_phdl->write(i2c_phdl, &i2c_para);
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}


int32 
i2c_epld_item_read(const epld_handle_t *phdl, uint32_t op_reg, int32 * value)
{
    int32 offset;
    int32 start_bit;

    int32 item_bitwidth;
    uint8 epld_reg_value = 0;
    epld_info_t *p_epld_info = NULL; 

    i2c_handle_t* i2c_phdl = NULL;
    i2c_op_para_t i2c_para; 
    int32 ret = 0;

    if(NULL == phdl || NULL == phdl->info || NULL == phdl->handle)
    {
        printf("EPLD i2c read item: invalid pointer\n");
        return -1;
    }

    p_epld_info = (epld_info_t *)phdl->info;
    if(0 == p_epld_info->reg_desc[op_reg].reg_valid)
    {
        *value = 0;
        return -1;
    }
    offset = p_epld_info->reg_desc[op_reg].reg_offset;
    start_bit = p_epld_info->reg_desc[op_reg].start_bit;

    item_bitwidth = p_epld_info->reg_desc[op_reg].item_bitwidth;
    i2c_phdl = (i2c_handle_t *)phdl->handle;
    
    i2c_para.offset = offset;
    i2c_para.len = p_epld_info->i2c_gen.alen;
    i2c_para.p_val = (uint8 *)(&epld_reg_value);
     
    ret = i2c_phdl->read(i2c_phdl, &i2c_para); 
     
    epld_reg_value >>= start_bit ;

    epld_reg_value &= ((1 << item_bitwidth) - 1);


    
    *value = (int32)epld_reg_value;
   
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

int32 
i2c_epld_item_write(const epld_handle_t *phdl, uint32_t op_reg,  int32 value)
{
    int32 offset;
    int32 start_bit;
    int32 item_bitwidth;
    uint8 epld_reg_value;    
    uint8 temp_value = 0;
    epld_info_t *p_epld_info = NULL; 

    i2c_handle_t* i2c_phdl = NULL;
    i2c_op_para_t i2c_para; 
    int32 ret = 0;
  
    if(NULL == phdl || NULL == phdl->info || NULL == phdl->handle)
    {
        printf("EPLD gpio spi write item: invalid pointer\n");
        return -1;
    }
    p_epld_info= phdl->info;
    if(0 == p_epld_info->reg_desc[op_reg].reg_valid)
    {
        return -1;
    }
    
    offset = p_epld_info->reg_desc[op_reg].reg_offset;
    start_bit = p_epld_info->reg_desc[op_reg].start_bit;
    item_bitwidth = p_epld_info->reg_desc[op_reg].item_bitwidth;
    i2c_phdl = (i2c_handle_t *)phdl->handle;
    i2c_para.offset = offset;
    i2c_para.len = p_epld_info->i2c_gen.alen;
    i2c_para.p_val = (uint8 *)(&epld_reg_value);
     
    ret = i2c_phdl->read(i2c_phdl, &i2c_para);
 
    value <<= start_bit ;
    temp_value =( ((1 << item_bitwidth) - 1) << start_bit );
    
    epld_reg_value &= (~temp_value);
    epld_reg_value |= value;    
    
    ret += i2c_phdl->write(i2c_phdl, &i2c_para);
    if(ret < 0)
        return RESULT_ERROR;
    else
        return RESULT_OK;
}

int32 epld_drv_get_item_width(const epld_handle_t *phdl, uint32_t op_reg)
{
    int32 item_bitwidth;
    epld_info_t *p_epld_info = NULL; 
    
    if(NULL == phdl || NULL == phdl->info)
    {
        printf("EPLD read item: invalid pointer\n");
        return -1;
    }    
    p_epld_info = (epld_info_t *)phdl->info;
    if(0 == p_epld_info->reg_desc[op_reg].reg_valid)
    {        
        return -1;
    }
    item_bitwidth = p_epld_info->reg_desc[op_reg].item_bitwidth;
    return item_bitwidth;
}

static epld_handle_t*
epld_create_handle(void *epld_dev_info)
{
    epld_handle_t *phdl = NULL; 
    epld_info_t *p_epld_info = NULL; 
    
    if (NULL == epld_dev_info)
    {
        return NULL;
    }

    phdl = (epld_handle_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_EPLD_INFO, sizeof(epld_handle_t));
    if (NULL == phdl)
    {
        goto err_out;
    }
    
    phdl->info = (epld_info_t *)DRV_MALLOC(CTCLIB_MEM_DRIVER_EPLD_INFO, sizeof(epld_info_t));
    if (NULL == phdl->info)
    {
        goto err_out;
    }

    sal_memcpy((int8 *)phdl->info, (int8 *)epld_dev_info, sizeof(epld_info_t));

    p_epld_info = (epld_info_t *)epld_dev_info;
    phdl->get_item_width = epld_drv_get_item_width;
    if(EPLD_PCI_TYPE == p_epld_info->epld_bus_type)
    {
        phdl->item_read = epld_drv_item_read;
        phdl->item_write = epld_drv_item_write;
        phdl->read = epld_drv_read;
        phdl->write = epld_drv_write;
    }
    else if (EPLD_LOCALBUS_TYPE == p_epld_info->epld_bus_type)
    {
        phdl->item_read = localbus_epld_item_read;
        phdl->item_write = localbus_epld_item_write;
        phdl->read = localbus_epld_read;
        phdl->write = localbus_epld_write;
    }
    else if (EPLD_SPI_TYPE == p_epld_info->epld_bus_type)
    {
        spi_handle_t* spi_phdl = NULL;
     
        p_epld_info->spi_gen.alen = EPLD_ADDRESS_LENTH;
        p_epld_info->spi_gen.len = EPLD_DATA_LENTH;
    
        spi_phdl = spi_create_handle(&(p_epld_info->spi_gen));
        phdl->handle = (void *)spi_phdl;
        phdl->item_read = gpio_epld_item_read;
        phdl->item_write = gpio_epld_item_write;
        phdl->read = gpio_epld_read;
        phdl->write = gpio_epld_write;
    }
    else if (EPLD_I2C_TYPE == p_epld_info->epld_bus_type)
    {
        i2c_handle_t *i2c_phdl = NULL;
        i2c_phdl = i2c_create_handle(&p_epld_info->i2c_gen);
        phdl->handle = (void *)i2c_phdl;
        phdl->item_read = i2c_epld_item_read;
        phdl->item_write = i2c_epld_item_write;
        phdl->read = i2c_epld_read;
        phdl->write = i2c_epld_write;
    }
    else
    {
        DRV_LOG_ERR("Unsupport epld type.");
        goto err_out;       
    }
    
    return phdl;
    
err_out:
    if (NULL != phdl && NULL != phdl->info)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_EPLD_INFO, phdl->info);
        phdl->info = NULL;
    }

    if (NULL != phdl)
    {
        DRV_FREE(CTCLIB_MEM_DRIVER_EPLD_INFO, phdl);
        phdl = NULL;
    }

    return NULL;
}
    
/****************************************************************************
 * Name : epld_dev_register
 * Purpose: register epld dev driver, initialize the handler
 * Input    : epld access type, the pointer of epld memory base addr
 * Output:  
 * Return:  the handler of the epld dev
 * Note :
****************************************************************************/   
epld_handle_t *
epld_dev_register(void *info)
{    
    return epld_create_handle(info);
}

