
#include <dirent.h>
#include "proto.h"
#include "param_check.h"
#include "lib_image.h"
#include "lib_fs.h"
#include "ctclib_crc.h"

#define DPKG_FILE_NAME  "/tmp/dpkg_info"

char*
format_time_str(char *pszBuf, size_t nLen, char *pszFormat, time_t nTime)
{
    struct tm *tm = NULL;
    if (NULL == pszBuf || NULL == pszFormat)
    {
        return NULL;
    }

    tm = localtime(&nTime);
    sal_time_strf(pszBuf, nLen, pszFormat, tm);
    return pszBuf;
}

/*******************************************************************************
 * Name:    check_image_file
 * Purpose: check centec image file
 * Input: 
 *   filename: image file full name
 * Output: 
 * Return: 
 *   success: 0
 *   failed : -1
 * Note: 
 ******************************************************************************/
#ifndef O_BINARY
#define O_BINARY    0
#endif
#define M_IMAGE_OPEN_MODE   (O_RDONLY|O_BINARY)

#define M_IMAGE_SET_ERRMSG(...)   \
do \
{ \
    if (errmsg) \
    { \
        sal_snprintf(errmsg, size, __VA_ARGS__); \
    } \
} while (0)

/* crc 32 table */
const u_int32_t crc_table[256] = 
{
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

#define DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);

uint32
special_gen_crc32(uint32 crc, char *buf, size_t len)
{
    //crc = crc ^ 0xffffffffL;
    while (len >= 8)
    {
      DO8(buf);
      len -= 8;
    }
    if (len) do {
      DO1(buf);
    } while (--len);
    //return crc ^ 0xffffffffL;
    return crc ;
}

int32
_deb_lib_image_check_image_file(char *filename, char *errmsg, uint32 size, uint32 b_crc)
{

    return 0;
}

int32
_ctc_lib_image_check_fit_image_file(char *filename, char *errmsg, uint32 size, uint32 b_crc)
{
    int ret = -1;
    char cmd[M_FULLPATH_MAX_LEN]={0};
    char buf[256];
    FILE *fp;
    int crc_val = -1;
    int header_val = -1;
    sal_strcpy(cmd,SWITCH_PREFIX"/usr/sbin/check_fit_image ");
    sal_strcat(cmd,filename);
    sal_strcat(cmd," /tmp/image_info 1>/dev/null 2>&1 ");
    
    ret=system(cmd);
    if(ret != 0)
    {
        /* for bug 53362, by tongzb, optimize log info,2019-09-03 */
        M_IMAGE_SET_ERRMSG("Invalid image file: has corrupted data");
        return ret;
    }
    
    fp = sal_fopen("/tmp/image_info", "r");
    if(NULL == fp)
    {
        /* for bug 53362, by tongzb, optimize log info,2019-09-03 */
        M_IMAGE_SET_ERRMSG("Invalid image file: has corrupted data");
        return -1;
    }
    
    while(fgets(buf, 256, fp) != NULL)
    {
        uint8 tmp[256];
    
        if(!sal_strncmp(buf, "check_image", 11))
        {
            sal_sscanf(buf, "%s %x", tmp, &header_val);
        }
        if ((b_crc != 0)&&(header_val == 0))
        {
            if(!sal_strncmp(buf, "verifying", 9))
            {
                sal_sscanf(buf, "%s %x", tmp, &crc_val);
            }            
        }
    }
    sal_fclose(fp);
    if(b_crc != 0)
        ret = crc_val;
    else
        ret = header_val;

    if(ret != 0)
    {
        M_IMAGE_SET_ERRMSG("check image failed! please check the image!\n");
        return ret;
    }
    system("rm /tmp/image_info"); 

    return ret;
}

int32
_ctc_lib_image_check_image_file(char *filename, char *errmsg, uint32 size, uint32 b_crc)
{
    FILE *ifd;
    ctc_image_header_t header;
    ctc_image_header_t *hdr = &header;
    int ret = -1;
    int len = 0,count = 0;
    char *data = NULL;
    uint32_t checksum = 0l;
    uint32_t temp_crc = 0;
    uint32_t data_len = 0;
    uint32_t read_len = 0;
    char buf[BUFFER_COUNT];
    
    if (NULL == filename)
    {
        SAL_ASSERT(0);
        M_IMAGE_SET_ERRMSG("Error: internal error");
        return -1;
    }

    if ((ifd = sal_fopen(filename, SAL_OPEN_RO))  == NULL)
    {
        M_IMAGE_SET_ERRMSG("Can't open file: %s", sal_strerror(errno));
        goto error_out;
    }

    /*
     * create copy of header so that we can blank out the
     * checksum field for checking - this can't be done
     * on the PROT_READ mapped data.
     */
    if(sal_fread(hdr, sizeof(ctc_image_header_t), 1, ifd) != 1)
    {
        M_IMAGE_SET_ERRMSG("Invalid image file: length error");
        goto error_out;
    }

    if (sal_ntohl(hdr->ih_magic) != CTC_IH_MAGIC) 
    {
        M_IMAGE_SET_ERRMSG("Invalid image file: bad magic number");
        goto error_out;
    }

    data = (char *)hdr;
    len  = sizeof(ctc_image_header_t);
    checksum = sal_ntoh32(hdr->ih_hcrc);
    hdr->ih_hcrc = sal_hton32(0);  /* clear for re-calculation */
    temp_crc = temp_crc ^ 0xffffffffL;
    temp_crc = special_gen_crc32(temp_crc, data, len);
    
    temp_crc = temp_crc ^ 0xffffffffL;
    
    if (temp_crc != checksum) 
    {
        M_IMAGE_SET_ERRMSG("Invalid image file: bad header checksum");
        goto error_out;
    }
    
    temp_crc = 0;
    /* only check the date indicated by ih_size of header */
    data_len = sal_ntoh32(hdr->ih_size);
    if (b_crc != 0)
    {
        temp_crc = temp_crc ^ 0xffffffffL;
        read_len = BUFFER_COUNT;
        while((count = sal_fread(buf, sizeof(char), read_len, ifd)) > 0)
        {
            if(count == 0)
                break;
            temp_crc = special_gen_crc32(temp_crc, buf, count);  

            data_len -= read_len;
            read_len = data_len > BUFFER_COUNT ? BUFFER_COUNT : data_len;
            if(data_len<=0)
                break;
        }
        temp_crc = temp_crc ^ 0xffffffffL;
        if (temp_crc != sal_ntoh32(hdr->ih_dcrc)) 
        {
            M_IMAGE_SET_ERRMSG("Invalid image file: has corrupted data");
            goto error_out;
        }
                
    }
    ret = 0;

error_out:
    
    if (ifd > 0)
    {
        sal_fclose(ifd);
    }

#ifdef TODO
    /*modified by xgu for bug 14775, 2011-04-21*/
    /*Free cached memory*/
    memmgr_free_cached_mem();
#endif

    return ret;
}

int32
_ctc_get_packagename_from_fit_image(char *filename, char *pkgname, size_t size)
{
    char cmd[M_FULLPATH_MAX_LEN]={0};
    char buf[256];
    FILE *fp;
    char str_val[256];
    int ret = -1;
    sal_strcpy(cmd,SWITCH_PREFIX"/usr/sbin/check_fit_image ");
    sal_strcat(cmd,filename);
    sal_strcat(cmd," /tmp/image_info 1>/dev/null 2>&1");

    ret=system(cmd);
    if(ret != 0)
    {
        return ret;
    }

    fp = sal_fopen("/tmp/image_info", "r");
    if(NULL == fp)
    {
        return -1;
    }

    while(fgets(buf, 256, fp) != NULL)
    {
        uint8 tmp[256];

        if(!sal_strncmp(buf, "version", 7))
        {
            sal_sscanf(buf, "%s %s", tmp, str_val);
        }
    }
    sal_fclose(fp);

    system("rm /tmp/image_info"); 
    sal_snprintf(pkgname, size, "%s", str_val);

    return 0;
}

int32
_ctc_get_packagename_from_image(char *filename, char *pkgname, size_t size)
{
    ctc_image_header_t hdr;
    FILE *fp = NULL;
    
    if (NULL == filename || NULL == pkgname)
    {
        return -1;
    }
    
    if ((fp = sal_fopen(filename, SAL_OPEN_RO)) == NULL)
    {
        return -1;
    }

    if (sal_fread(&hdr, sizeof(ctc_image_header_t), 1, fp) != 1)
    {    
        sal_fclose(fp);
        return -1;
    }
    
    sal_fclose(fp);
    sal_snprintf(pkgname, size, hdr.ih_name);
    return 0;
}

int32
_deb_strip_newline(char *line)
{
    int32 line_strlen = 0;
    
    line_strlen = sal_strlen(line);
    if (line_strlen > 0)
    {
        if (line[line_strlen-1] == CMD_LINE_CHAR)
        {
            line[line_strlen-1] = CMD_ZERO_CHAR;
        }
    }

    return 0;
}
    
int32
_deb_get_info_from_image(char *filename, char *version, char *creatttime, size_t size)
{
    FILE *fp = NULL;
    char cmd[CMD_BUF_512];
    char line[CMD_BUF_512];
    int32 rc = 0;
    char *p_time = NULL;
    char *p_ver = NULL;

    version[0] = CMD_ZERO_CHAR;
    creatttime[0] = CMD_ZERO_CHAR;
    
    sal_snprintf(cmd, CMD_BUF_512, "dpkg -I %s > %s 2>&1", filename, DPKG_FILE_NAME);
    rc = sal_system(cmd);
    if (rc != 0)
    {
        return -1;
    }

    fp = sal_fopen(DPKG_FILE_NAME, SAL_OPEN_RO);
    if (NULL == fp)
    {
        return -1;
    }

    while (NULL != sal_fgets(line, CMD_BUF_512, fp))
    {
        p_time = sal_strstr(line, "Date: ");
        if (p_time)
        {
            p_time += sal_strlen("Date: ");
            sal_strncpy(creatttime, p_time, size);
            _deb_strip_newline(creatttime);
        }
        p_ver = sal_strstr(line, "Version: ");
        if (p_ver)
        {
            p_ver += sal_strlen("Version: ");
            sal_strncpy(version, p_ver, size);
            _deb_strip_newline(version);
        }
    }

    if (fp)
    {
        sal_fclose(fp);
    }
    sal_unlink(DPKG_FILE_NAME);
    
    return 0;
}

int32
_deb_get_createtime_from_image(char *filename, u_int32_t *ptime)
{
    ctc_image_header_t hdr;
    FILE *fp = NULL;
    
    if (NULL == filename || NULL == ptime)
    {
        return -1;
    }
    
    if ((fp = sal_fopen(filename, SAL_OPEN_RO)) == NULL)
    {
        return -1;
    }

    if (sal_fread(&hdr, sizeof(ctc_image_header_t), 1, fp) != 1)
    {    
        sal_fclose(fp);
        return -1;
    }
    
    sal_fclose(fp);
    *ptime = sal_ntoh32(hdr.ih_time);
    return 0;
}

int32
_ctc_get_createtime_from_fit_image(char *filename, u_int32_t *ptime)
{
    char cmd[M_FULLPATH_MAX_LEN]={0};
    char buf[256];
    FILE *fp;
    u_int32_t time_val;
    int ret = -1;

    sal_strcpy(cmd,SWITCH_PREFIX"/usr/sbin/check_fit_image ");
    sal_strcat(cmd,filename);
    sal_strcat(cmd," /tmp/image_info 1>/dev/null 2>&1");

    ret=system(cmd);
    if(ret != 0)
    {
        return ret;
    }

    fp = sal_fopen("/tmp/image_info", "r");
    if(NULL == fp)
    {
        return -1;
    }

    while(fgets(buf, 256, fp) != NULL)
    {
        uint8 tmp[256];

        if(!sal_strncmp(buf, "created_time", 12))
        {
            sal_sscanf(buf, "%s %d", tmp, &time_val);
        }
    }
    sal_fclose(fp);

    system("rm /tmp/image_info"); 
    /* for bug 47622, modify by tongzb, fix create time show error */
    *ptime = time_val;

    return 0;
}

int32
_ctc_get_createtime_from_image(char *filename, u_int32_t *ptime)
{
    ctc_image_header_t hdr;
    FILE *fp = NULL;
    
    if (NULL == filename || NULL == ptime)
    {
        return -1;
    }
    
    if ((fp = sal_fopen(filename, SAL_OPEN_RO)) == NULL)
    {
        return -1;
    }

    if (sal_fread(&hdr, sizeof(ctc_image_header_t), 1, fp) != 1)
    {    
        sal_fclose(fp);
        return -1;
    }
    
    sal_fclose(fp);
    *ptime = sal_ntoh32(hdr.ih_time);
    return 0;
}

uint32
image_multi_count(ctc_image_header_t *hdr, uint32 *image_size)
{
    uint32 i, count = 0;
    uint32 *size;

    /* get start of the image payload, which in case of multi
     * component images that points to a table of component sizes */
    size = (uint32 *)image_get_data(hdr);

    /* count non empty slots */
    for (i = 0; size[i]; ++i)
    {
        image_size[i] = size[i];
        count++;
    }

    return count;
}

int32
lib_image_update_ramfs()
{
    uint32 has_ramfs = FALSE;
    
    has_ramfs = syscmd_file_exist(RAMFS_FILENAME);
    if (has_ramfs)
    {
        /* has new ramfs, should remove /etc/cmd/ files */
        sal_system("rm -rf /etc/cmd/ 1>/dev/null 2>&1");
        sal_system("cd /; cpio -idmvu < "RAMFS_FILENAME" 1>/dev/null 2>&1");
        lib_image_remove_ramfs();
    }
    
    return 0;
}

int32
lib_image_remove_ramfs()
{
    sal_system("rm -rf "RAMFS_FILENAME"* 1>/dev/null 2>&1");

    return 0;
}

int32
lib_image_get_ramfs(char *filename)
{
    uint32 header_size = 0;
    uint32 size_size = 0;
    uint32 ramfs_offset = 0;
    uint32 ramfs_size = 0;
    int32 left_size = 0;
    int32 read_size = 0;
    uint32 ramfs_curr_size = 0;
    uint32 image_count = 0;
    int32 read_len = 0;
    uint32 image_size[MAX_IMAGE_NUM];
    uint8 header_buffer[IMAGE_BUFFER_SIZE];
    uint8 image_buffer[IMAGE_BUFFER_SIZE];
    ctc_image_header_t *p_hdr = (ctc_image_header_t *)header_buffer;
    FILE *fp = NULL;
    FILE *ramfs_fp = NULL;
    int32 rc = 0;
    
    if (NULL == filename)
    {
        return -1;
    }

    sal_memset(header_buffer, 0, sizeof(header_buffer));
    if ((fp = sal_fopen(filename, SAL_OPEN_RO)) == NULL)
    {
        goto ERROR_EXIT;
    }

    lib_image_remove_ramfs();
    
    if ((ramfs_fp = sal_fopen(RAMFS_LZMA_FILENAME, SAL_OPEN_RW)) == NULL)
    {
        goto ERROR_EXIT;
    }
    
    if (sal_fread(header_buffer, sizeof(ctc_image_header_t)+MAX_IMAGE_NUM*4, 1, fp) != 1)
    {
        goto ERROR_EXIT;
    }

    image_count = image_multi_count(p_hdr, image_size);
    header_size = image_get_header_size();
    size_size = (image_count+1)*sizeof(uint32);

    ramfs_offset = header_size + size_size + GLB_ALIGN(image_size[0]);
    ramfs_size = image_size[1];

    sal_fseek(fp, ramfs_offset, SEEK_SET);
    ramfs_curr_size = 0;
    
    while (ramfs_curr_size <= ramfs_size)
    {
        left_size = ramfs_size - ramfs_curr_size;
        read_size = (IMAGE_BUFFER_SIZE > left_size) ? left_size : IMAGE_BUFFER_SIZE;
        if (0 == read_size)
        {
            break;
        }
        read_len = sal_fread(image_buffer, 1, read_size, fp);
        if (read_len > 0)
        {
            ramfs_curr_size += read_len;
            sal_fwrite(image_buffer, 1, read_len, ramfs_fp); 
        }
        else
        {
            goto ERROR_EXIT;
        }
    }

    sal_fclose(fp);
    sal_fclose(ramfs_fp);

    rc = sal_system("lzma -d "RAMFS_LZMA_FILENAME" 1>/dev/null 2>&1");
    if (0 != rc)
    {
        return -1;
    }
    return 0;

ERROR_EXIT:
    if (fp)
    {
        sal_fclose(fp);
        fp = NULL;
    }
    
    if (ramfs_fp)
    {
        sal_fclose(ramfs_fp);
        ramfs_fp = NULL;
    }
    
    return -1;
}

int32
_deb_lib_image_show_boot_image()
{
    struct dirent *ent = NULL;
    DIR *dir = NULL;
    char szFullName[M_FULLPATH_MAX_LEN + 1];
    char szCurName[M_FULLPATH_MAX_LEN];
    char* pFileName = NULL;
    char szVersion[CMD_BUF_64];
    char szCreateTime[CMD_BUF_64];
    uint32 is_boot_image = FALSE;
    char errmsg[CMD_BUF_64];
    int32 rc = PM_E_NONE;

    sal_memset(errmsg, 0, sizeof(errmsg));
    errmsg[0] = CMD_ZERO_CHAR;
    szFullName[0] = CMD_ZERO_CHAR;
    szFullName[M_FULLPATH_MAX_LEN] = CMD_ZERO_CHAR;

    if ((rc = readlink(M_CUR_BOOT_IMAGE_LINK_FULLNAME, 
                        szFullName, M_FULLPATH_MAX_LEN)) < 0)
    {
        sal_printf("%% Can't read boot configuration file: %s\n",
                sal_strerror(errno));
        return PM_E_FAIL;
    }
    szFullName[rc] = CMD_ZERO_CHAR;
    if ((pFileName = sal_strrchr(szFullName, '/')) == NULL)
    {
        pFileName = szFullName;
    }
    else
    {
        pFileName++;
        if (CMD_ZERO_CHAR == pFileName)
        {
            sal_printf("%% Invalid boot configuration file.\n");
            return PM_E_FAIL;
        }
    }

    /* search dir */
    dir = opendir(M_BOOT_IMAGE_PATH);
    if (NULL == dir)
    {
        sal_printf("%% Can't open boot image directory: %s\n",
                sal_strerror(errno));
        return PM_E_FAIL;
    }

    sal_printf("System image files list:\n");

    sal_printf("%-2s%-22s%-22s%s\n", "", "Create Time", "Version", "File name");
    sal_printf("-----------------------+---------------------+----------------------------------\n");

    /* find the partial match item */
    while ((ent = readdir(dir)) != NULL)
    {
        is_boot_image = FALSE;

        /* skip . & .. */
        if (!sal_strcmp(ent->d_name, ".")
                || !sal_strcmp(ent->d_name, ".."))
        {
            continue;
        }
        if (DT_REG != ent->d_type)
        {
            continue;
        }

        /* ignore next boot image file */
        if (!sal_strcmp(ent->d_name, pFileName))
        {
            is_boot_image = TRUE;
        }

        sal_snprintf(szCurName, M_FULLPATH_MAX_LEN, "%s/%s",
                     M_BOOT_IMAGE_PATH, ent->d_name);

        rc = lib_image_check_image_file(szCurName, NULL, 0, 0);
        /* invaldate image file */
        if (rc != 0)
        {
            continue;
        }

        rc = _deb_get_info_from_image(szCurName, szVersion, szCreateTime, CMD_BUF_64);
        if (rc != 0)
        {
            continue;
        }
        
        /* backup boot image */
        sal_printf("%-2s%-22s%-22s%s\n", is_boot_image ? "*" : "", szCreateTime, szVersion, ent->d_name);
    }
    
    closedir(dir);
    return PM_E_NONE;
}

int32
_ctc_lib_image_show_boot_image()
{
    struct dirent *ent = NULL;
    DIR *dir = NULL;
    char szFullName[M_FULLPATH_MAX_LEN + 1];
    char szCurName[M_FULLPATH_MAX_LEN];
    char* pFileName = NULL;
    char szPkgName[CMD_BUF_64];
    char szCreateTime[CMD_BUF_32];
    uint32 nCreateTime = 0;
    char errmsg[CMD_BUF_64];
    int32 rc = PM_E_NONE;

    sal_memset(errmsg, 0, sizeof(errmsg));
    errmsg[0] = CMD_ZERO_CHAR;
    szFullName[0] = CMD_ZERO_CHAR;
    szFullName[M_FULLPATH_MAX_LEN] = CMD_ZERO_CHAR;
    if ((rc = readlink(M_CUR_BOOT_IMAGE_LINK_FULLNAME, 
                        szFullName, M_FULLPATH_MAX_LEN)) < 0)
    {
        sal_printf("%% Can't read boot configuration file: %s\n",
                sal_strerror(errno));
        return PM_E_FAIL;
    }
    szFullName[rc] = CMD_ZERO_CHAR;
    if ((pFileName = sal_strrchr(szFullName, '/')) == NULL)
    {
        pFileName = szFullName;
    }
    else
    {
        pFileName++;
        if (CMD_ZERO_CHAR == pFileName)
        {
            sal_printf("%% Invalid boot configuration file.\n");
            return PM_E_FAIL;
        }
    }
    
    /* search dir */
    dir = opendir(M_BOOT_IMAGE_PATH);
    if (NULL == dir)
    {
        sal_printf("%% Can't open boot image directory: %s\n",
                sal_strerror(errno));
        return PM_E_FAIL;
    }

    sal_printf("System image files list:\n");

    sal_printf("%-2s%-22s%-22s%s\n", "", "Create Time", "Version", "File name");
#ifdef HAVE_DEBIAN
    sal_printf("-----------------------+---------------------+---------------------------------\n");
#else
    sal_printf("-----------------------+---------------------+----------------------------------\n");
#endif

    /* find the partial match item */
    while ((ent = readdir(dir)) != NULL)
    {
        /* skip . & .. */
        if (!sal_strcmp(ent->d_name, ".")
                || !sal_strcmp(ent->d_name, ".."))
        {
            continue;
        }

        /* ignore next boot image file */
        if (!sal_strcmp(ent->d_name, pFileName))
        {
            continue;
        }

        sal_snprintf(szCurName, M_FULLPATH_MAX_LEN, "%s/%s",
                     M_BOOT_IMAGE_PATH, ent->d_name);

        rc = lib_image_check_image_file(szCurName, NULL, 0, 0);
        /* invaldate image file */
        if (rc != 0)
        {
            continue;
        }

        get_packagename_from_image(szCurName, szPkgName, CMD_BUF_64);
        get_createtime_from_image(szCurName, &nCreateTime);
        format_time_str(szCreateTime, CMD_BUF_32, 
                        "%Y-%m-%d %H:%M:%S", nCreateTime);
        if (!sal_strcmp(ent->d_name, M_CUR_BOOT_IMAGE_LINK_NAME))
        {
            /* next boot image */
            sal_printf("%-2s%-22s%-22s%s\n", "*", 
                    szCreateTime, szPkgName, pFileName);
        }
        else
        {
            /* backup boot image */
            sal_printf("%-2s%-22s%-22s%s\n", "", szCreateTime, szPkgName, ent->d_name);
        }
    }
    
    closedir(dir);
    return PM_E_NONE;
}

int32
lib_image_check_image_file(char *filename, char *errmsg, uint32 size, uint32 b_crc)
{
/* modified by shil temporarily, support boot flash on arm64 v550-24x8y2c, 2018-05-29 */
#if defined _CTC_X86_64_
    return _deb_lib_image_check_image_file(filename, errmsg, size, b_crc);
#elif defined _CTC_ARM_LS1023A_
    return _ctc_lib_image_check_fit_image_file(filename, errmsg, size, b_crc);
#elif defined _CTC_ARM_CTC5236_
    return _ctc_lib_image_check_fit_image_file(filename, errmsg, size, b_crc);
#else
    return _ctc_lib_image_check_image_file(filename, errmsg, size, b_crc);
#endif
}

int32
get_createtime_from_image(char *filename, u_int32_t *ptime)
{
#if defined _CTC_X86_64_
    return _deb_get_createtime_from_image(filename, ptime);
#elif defined _CTC_ARM_LS1023A_ 
    return _ctc_get_createtime_from_fit_image(filename, ptime);
#elif defined _CTC_ARM_CTC5236_ 
    return _ctc_get_createtime_from_fit_image(filename, ptime);
#else
    return _ctc_get_createtime_from_image(filename, ptime);
#endif
}

int32
get_packagename_from_image(char *filename, char *pkgname, size_t size)
{
#if defined _CTC_ARM_LS1023A_ 
    return _ctc_get_packagename_from_fit_image(filename, pkgname, size);
#elif defined _CTC_ARM_CTC5236_ 
    return _ctc_get_packagename_from_fit_image(filename, pkgname, size);
#else
    return _ctc_get_packagename_from_image(filename, pkgname, size);
#endif
}

int32
lib_image_show_boot_image()
{
#ifdef _CTC_X86_64_
    return _deb_lib_image_show_boot_image();
#else
    return _ctc_lib_image_show_boot_image();
#endif
}

int32
lib_parse_warmboot_file(uint32 *warmboot_type, uint32 *warmboot_time, uint32 *warmboot_count)
{
    FILE *fp = NULL;
    char buf[CMD_BUF_256];
    uint32 type = 0;
    uint32 seconds = 0;
    uint32 count = 0;

    *warmboot_type = GLB_WARMBOOT_NORMAL;
    *warmboot_time = 0;
    
    if ((fp = sal_fopen(BHM_WARMBOOT_FILE, "r")) == NULL)
    {
        return PM_E_FAIL;
    }

    while (NULL != sal_fgets(buf, CMD_BUF_256, fp))
    {
        if (0 == sal_strncmp(buf, "type=", sal_strlen("type=")))
        { 
            if (sal_sscanf(buf, "type=%u", &type) != 1)
            {
                sal_fclose(fp);
                return PM_E_FAIL;
            }
        }
        if (0 == sal_strncmp(buf, "time=", sal_strlen("time=")))
        { 
            if (sal_sscanf(buf, "time=%u", &seconds) != 1)
            {
                sal_fclose(fp);
                return PM_E_FAIL;
            }
        }
        if (0 == sal_strncmp(buf, "count=", sal_strlen("count=")))
        { 
            if (sal_sscanf(buf, "count=%u", &count) != 1)
            {
                sal_fclose(fp);
                return PM_E_FAIL;
            }
        }
    }

    *warmboot_type = type;
    *warmboot_time = seconds;
    *warmboot_count = count;
    sal_fclose(fp);

    return PM_E_NONE;
}

