#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <inttypes.h>

#define SOC_REG_PRINT(fmt, args...) \
do { \
    FILE * fp = NULL; \
    fp = fopen("/tmp/.soc_out", "w+"); \
    fprintf(fp, fmt, ##args); \
    fclose(fp); \
    printf(fmt, ##args); \
} while (0)

#define SOC_REG_PRINT_APP(fmt, args...) \
do { \
    FILE * fp = NULL; \
    fp = fopen("/tmp/.soc_out", "a"); \
    fprintf(fp, fmt, ##args); \
    fclose(fp); \
    printf(fmt, ##args); \
} while (0)

static uint64_t reg_str2uint64_t(char *str)
{
    uint64_t ret = 0;
    if (strlen(str) > 2 &&  str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        sscanf(str, "%" PRIx64 "\n", &ret);
    }
    else
    {
        sscanf(str, "%" PRIu64 "\n", &ret);
    }
    return ret;
}

int main(int argc, char *argv[])
{
    int  opt = 0;
    unsigned int  mem_fd = 0;
    unsigned char* ccsr_virt_base = NULL;
    uint32_t  opt_flag = 0;
    uint32_t  baseaddr = 0;
    uint32_t  offset = 0;
    uint32_t  width=0;
    uint32_t  count = 0;
    uint32_t  pos = 0;
    uint64_t  writeval = 0;
    uint64_t  value = 0;

    while ((opt = getopt(argc, argv, "rwh")) != -1)
    {
        switch (opt)
        {
        case 'r':
            opt_flag = 2;
            break;
        case 'w':
            opt_flag = 1;
            break;
        case 'h':
            opt_flag = 0;
            break;
        default:
            opt_flag = 0;
        }
    }

    if(opt_flag == 2)
    {
        baseaddr = reg_str2uint64_t(argv[2]);
        offset = reg_str2uint64_t(argv[3]);
        width = reg_str2uint64_t(argv[4]);
        count = reg_str2uint64_t(argv[5]);

        if (offset + width*count > 3*1024*1024)
        {
            SOC_REG_PRINT("out of range!\n");
            return -1;
        }
        
        mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
        if(mem_fd < 0) {
            SOC_REG_PRINT("cannot open /dev/mem file!\n");
            return -1;
        }
        ccsr_virt_base = (unsigned char *)mmap(NULL, 3*1024*1024, 
            PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, baseaddr);
        if (ccsr_virt_base == (unsigned char*)(-1))
        {
            SOC_REG_PRINT("baseaddr must be a multiple of the page size!\n");
            return -1;
        }

        for (pos = 0; pos < count; pos++)
        {
            switch (width)
            {
                case 1:
                    value = *(uint8_t *)((ccsr_virt_base +  offset) + 1*pos);
                    if (pos == 0)
                        SOC_REG_PRINT("0x%x : " "0x%02" PRIx64 "\n", offset + 1*pos, value);
                    else
                        SOC_REG_PRINT_APP("0x%x : " "0x%02" PRIx64 "\n", offset + 1*pos, value);
                    break;
                case 2:
                    value = *(uint16_t *)((ccsr_virt_base +  offset) + 2*pos);
                    if (pos == 0)
                        SOC_REG_PRINT("0x%x : " "0x%04" PRIx64 "\n", offset + 2*pos, value);
                    else
                        SOC_REG_PRINT_APP("0x%x : " "0x%04" PRIx64 "\n", offset + 2*pos, value);
                    break;
                case 4:
                    value = *(uint32_t *)((ccsr_virt_base +  offset) + 4*pos);
                    if (pos == 0)
                        SOC_REG_PRINT("0x%x : " "0x%08" PRIx64 "\n", offset + 4*pos, value);
                    else
                        SOC_REG_PRINT_APP("0x%x : " "0x%08" PRIx64 "\n", offset + 4*pos, value);
                    break;
                case 8:
                    value = *(uint64_t *)((ccsr_virt_base +  offset) + 8*pos);
                    if (pos == 0)
                        SOC_REG_PRINT("0x%x : " "0x%016" PRIx64 "\n", offset + 8*pos, value);
                    else
                        SOC_REG_PRINT_APP("0x%x : " "0x%016" PRIx64 "\n", offset + 8*pos, value);
                    break; 
                default:
                    value = *(uint8_t *)((ccsr_virt_base +  offset) + 1*pos);
                    if (pos == 0)
                        SOC_REG_PRINT("0x%x : " "0x%02" PRIx64 "\n", offset + 1*pos, value);
                    else
                        SOC_REG_PRINT_APP("0x%x : " "0x%02" PRIx64 "\n", offset + 1*pos, value);
                    break;              
            }
        }
            
        munmap(ccsr_virt_base, 3*1024*1024);
    }
    else if (opt_flag == 1)
    {
        baseaddr = reg_str2uint64_t(argv[2]);
        offset = reg_str2uint64_t(argv[3]);
        width = reg_str2uint64_t(argv[4]);
        writeval =  reg_str2uint64_t(argv[5]);
        
        if (offset + width > 3*1024*1024)
        {
            SOC_REG_PRINT("out of range!\n");
            return -1;
        }
        
        mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
        if(mem_fd < 0) {
            SOC_REG_PRINT("cannot open /dev/mem file!\n");
            return -1;
        }
        ccsr_virt_base = (unsigned char *)mmap(NULL, 3*1024*1024, 
            PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, baseaddr);
        if (ccsr_virt_base == (unsigned char*)(-1))
        {
            SOC_REG_PRINT("baseaddr must be a multiple of the page size!\n");
            return -1;
        }

        switch(width)
        {
            case 1:
                *(uint8_t *)(ccsr_virt_base +  offset) = (uint8_t)writeval;
                value = *(uint8_t *)(ccsr_virt_base +  offset);
                SOC_REG_PRINT("0x%x : " "0x%02" PRIx64 "\n", offset, value);
                break;
            case 2:
                *(uint16_t *)(ccsr_virt_base +  offset) = (uint16_t)writeval;
                value = *(uint16_t *)(ccsr_virt_base +  offset);
                SOC_REG_PRINT("0x%x : " "0x%04" PRIx64 "\n", offset, value);
                break;
            case 4:
                *(uint32_t *)(ccsr_virt_base +  offset) = (uint32_t)writeval;
                value = *(uint32_t *)(ccsr_virt_base +  offset);
                SOC_REG_PRINT("0x%x : " "0x%08" PRIx64 "\n", offset, value);
                break;
            case 8:
                *(uint64_t *)(ccsr_virt_base +  offset) = (uint64_t)writeval;
                value = *(uint64_t *)(ccsr_virt_base +  offset);
                SOC_REG_PRINT("0x%x : " "0x%016" PRIx64 "\n", offset, value);
                break;
            default:
                *(uint8_t *)(ccsr_virt_base +  offset) = (uint8_t)writeval;
                value = *(uint8_t *)(ccsr_virt_base +  offset);
                SOC_REG_PRINT("0x%x : " "0x%02" PRIx64 "\n", offset, value);
                break;
        }

        munmap(ccsr_virt_base, 3*1024*1024);
    }
    else
    {
        SOC_REG_PRINT("NAME\n"
                                   "\treg_ops - read or write soc regs\n"
                                   "USAGE\n"
                                   "\treg_ops -h : get help\n"
                                   "\treg_ops -r baseaddr offset width count : read reg\n"
                                   "\treg_ops -w baseaddr offset width value : write reg\n");
    }

    return 0;
}

