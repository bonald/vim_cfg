#ifndef __COLD_SW_ITEM_H__
#define __COLD_SW_ITEM_H__

#define COLD_PS_INFO_FILE_NAME         "ps_info.log"
#define COLD_NET_INFO_FILE_NAME        "net_info.log"

#define COLD_CPU_INFO_PATH    "/proc/stat"
#define COLD_MEM_INFO_PATH    "/proc/meminfo"

typedef struct cold_sw_item_mem_info_s
{
  uint32    mem_total;
  uint32    mem_free;
  uint32    mem_buffers;
  uint32    mem_cached; 
  
} cold_sw_item_mem_info_t;

typedef struct cold_sw_item_sys_s
{
   uint32 cpu_usage;
   uint32 mem_usage;
   cold_sw_item_mem_info_t mem_info;
} cold_sw_item_sys_t;

void cold_item_cpu_usage_init(void);
#endif /*__COLD_SW_ITEM_H__*/
