#ifndef __HSRV_SYNC_MIRROR_H__
#define __HSRV_SYNC_MIRROR_H__

#define MIRROR_TO_CPU_PACKET_BUFF_SIZE 1000
#define MIRROR_TO_CPU_ADMIN_PACKET_BUFF_SIZE 99999
#define HSRV_MIRROR_SOURCE_CPU_PORT 0xFFF0
#define MIRROR_TO_CPU_PACKET_SIZE 9601
#define MTYPE_TMP 0
#define MIRROR_TO_CPU_PACKET_FILE_DIR "/mnt/flash/mirror"
#define MIRROR_TO_CPU_FLASH_DIR "/mnt/flash"
#define MIRROR_TO_CPU_PACKET_FILE_NAME "MirCpuPkt"
#define FULL_NAME_FLASH              "/mnt/flash"
#define HSRV_MIRROR_SOURCE_CPU_GLPORT 0xFFF0

#define CONVERT_PACKET_MAX_LENGTH 10624
#define DLT_EN10MB  1               
#define LINKTYPE_ETHERNET   DLT_EN10MB  /* also for 100Mb and up */
#define TCPDUMP_MAGIC       0xa1b2c3d4
#define PCAP_VERSION_MAJOR 2
#define PCAP_VERSION_MINOR 4

typedef struct
{
    uint8   data[MIRROR_TO_CPU_PACKET_SIZE];
    uint32  data_len;
    uint32  ifindex;
    char    ifname[IFNAME_SIZE];
    uint8   packet_is_written;      /*check whether packets in buffer have been written*/
    struct timeval tv;              /*the time of packet being received*/
} hsrv_cpu_mirror_node_t;

int32
hsrv_mirror_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
hsrv_mirror_mac_escape_sync(cdb_node_t* p_tbl_node, cdb_node_t* p_ds_node, uint32 oper, uint32 field_id, 
      void *p_tbl, void *p_ds);
int32
hsrv_show_cpu_mirror_capture(cfg_cmd_para_t *para);
int32
hsrv_show_cpu_mirror_buf_size(cfg_cmd_para_t *para);
int32
hsrv_show_cpu_mirror_packet_all(cfg_cmd_para_t *para);
int32
hsrv_show_cpu_mirror_buf_size(cfg_cmd_para_t *para);
hsrv_cpu_mirror_node_t*
hsrv_mirror_get_node_by_id(uint32 packet_id);
int32
hsrv_mirror_txt_pkt_to_pcap(char *src_file, char *dst_file);
int32
hsrv_mirror_to_cpu_rx_packet(uint8 * h_data, uint32 h_data_len, tbl_interface_t *p_db_if);
int32
hsrv_show_cpu_mirror_packet_add(uint32 packet_id, hsrv_cpu_mirror_node_t* p_node, FILE *fp);
int32
hsrv_mirror_get_disk_free_size();
int32
hsrv_cpu_mirror_clear_cpu_mirror_buffer();
int
syscmd_file_exist(char *filename);
int32
hsrv_mirror_start();

#endif /* !__HSRV_SYNC_MIRROR_H__ */


