#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h> 
#include <unistd.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <mtd/mtd-user.h>


/* crc 32 table */
const long ctclib_crc_table[256] = 
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

#define DO1(buf) crc = ctclib_crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);
/*******************************************************************************
 * Name:    ctclib_gen_crc32
 * Purpose:   calculate crc32
 * Input: 
 *   crc: crc
 *   buf: buffer
 *   len: buffer length
 * Output: 
 * Return: crc
 * Note: 
 ******************************************************************************/
long
ctclib_gen_crc32(long crc, char *buf, size_t len)
{
    crc = crc ^ 0xffffffffL;
    while (len >= 8)
    {
      DO8(buf);
      len -= 8;
    }
    if (len) do {
      DO1(buf);
    } while (--len);
    return crc ^ 0xffffffffL;
}

/* DEBUG must be out, the board_type_detect will use the out info for check bootimage */
#if 1
#define DEBUG(x,args...)  printf(x,##args)
#endif

#define SYSENV_BASE             0x310000
#define UBOOTENV_BASE           0x300000
#define OEM_OFFSET              0x1300
#define PORTINFO_OFFSET         0x1400
#define STMCOF_OFFSET           0x1260

int main(int argc, char *argv[])
{
    enum {
        OPT_NONE,
	    OPT_ERASE,	   
        OPT_READ_IMAGE,
        OPT_WRITE_IMAGE,	   
        OPT_READ_CMD,
        OPT_WRITE_CMD,
	    OPT_READ_SERVERIP,
	    OPT_WRITE_SERVERIP,
	    OPT_READ_IPADDR,
	    OPT_WRITE_IPADDR,
	    OPT_WRITE_OEM,
	    OPT_READ_OEM,
	    OPT_WRITE_PORTINFO,
	    OPT_READ_PORTINFO	,
	    OPT_WRITE_STMINFO,
	    OPT_READ_STMINFO	    
    }option = OPT_NONE;
    char value[256];/* value for write */
    memset(value,0,256);
    
    if(argc == 3 && !strcmp(argv[1],"read"))
    {
        if(!strcmp(argv[2],"image"))
            option = OPT_READ_IMAGE;
        else if(!strcmp(argv[2],"cmd"))
            option = OPT_READ_CMD;
        else if(!strcmp(argv[2],"serverip"))
            option = OPT_READ_SERVERIP;
        else if(!strcmp(argv[2],"ipaddr"))
            option = OPT_READ_IPADDR;	     
    }
    else if(argc == 4 && !strcmp(argv[1],"read"))
    {
        if(!strcmp(argv[2],"oem"))
            option = OPT_READ_OEM;
        else if(!strcmp(argv[2],"portinfo"))
            option = OPT_READ_PORTINFO;
        else if(!strcmp(argv[2],"stminfo"))
            option = OPT_READ_STMINFO;        
        memcpy(value,argv[3],strlen(argv[3]));
    }
    else if(argc == 4 && !strcmp(argv[1],"write"))
    {
        if(!strcmp(argv[2],"image"))
            option = OPT_WRITE_IMAGE;
        else if(!strcmp(argv[2],"cmd"))
            option = OPT_WRITE_CMD;
        else if(!strcmp(argv[2],"serverip"))
            option = OPT_WRITE_SERVERIP;
        else if(!strcmp(argv[2],"ipaddr"))
            option = OPT_WRITE_IPADDR; 
        else if(!strcmp(argv[2],"oem"))
            option = OPT_WRITE_OEM;
        else if(!strcmp(argv[2],"portinfo"))
            option = OPT_WRITE_PORTINFO;
        else if(!strcmp(argv[2],"stminfo"))
            option = OPT_WRITE_STMINFO;        
        memcpy(value,argv[3],strlen(argv[3]));
    }   
    else if(argc == 2 && !strcmp(argv[1],"erase"))
    {
	 	option = OPT_ERASE;
    }
    else
    {
	 	DEBUG("<erase>\n<read [cmd|image|serverip|ipaddr|oem|portinfo|stminfo] <value>>\n<write [cmd|image|serverip|ipaddr|oem|protinfo|stminfo] value>\n");
	 	return -1;
    }
    
    if(option == OPT_NONE)
        return 0;
    
    int fd = open("/dev/mtd0",O_SYNC|O_RDWR);
    if(fd<0)
    {
        DEBUG("open /dev/mtd0 failed!");
        return -1;
    }
    
    off_t offset;
    if((option == OPT_WRITE_OEM)||(option == OPT_READ_OEM)
      ||(option == OPT_WRITE_PORTINFO)||(option == OPT_READ_PORTINFO)
      ||(option == OPT_WRITE_STMINFO)||(option == OPT_READ_STMINFO))    
        offset = SYSENV_BASE;/* from 0x310000 to end is for system */
    else
        offset = UBOOTENV_BASE;/* from 0x300000 to 0x310000 is for uboot */
    if (offset != lseek(fd, offset, SEEK_SET))
    {
        DEBUG("lseek set failed");
        if(close(fd)<0)
        {
            DEBUG("close fd failed");
            return -1;
        }
        return -1;
    }
    
    char buf[0x2000];
    long len = 0;
    long i = 0;
    long j = 0;
    long t = 0;
    long index = 0;
    char strbuf[256];
    char flag = 0;
    struct erase_info_user erase;
    int err;
    len = read(fd, buf,0x2000);
    DEBUG("Read data len = %ld\n",len);
    //only env space keep to this, OEM/PORTINFO/STMINFO is not in the env space
    if((option != OPT_WRITE_OEM)&&(option != OPT_WRITE_PORTINFO)&&(option != OPT_WRITE_STMINFO)) 
    {
        for(i=0;i<len;i++)
        {
            if(i<len-4)
            {
                if((buf[i]==0x00)&&(buf[i+1]==0x00)&&(buf[i+2]==0x00)&&(buf[i+3]==0x00))
                {
                    len = i;
                    break;
                }
            }
        }
        DEBUG("Valid data len = %ld\n",len);
    }

    if(option == OPT_ERASE)
    {
        erase.start = offset;
        erase.length = 0x10000;
        int err = ioctl(fd, MEMERASE, &erase);
        if(err != 0)
        {
            DEBUG("erase flash failed\n ");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }
            return -1;
        }        
    }
    else if((option == OPT_READ_IMAGE)||(option == OPT_WRITE_IMAGE))
    {
        for(i=0;i<len;i++)
        {
            if((i<len-9)&&(flag == 0))
            {
                t = 0;
                if((buf[i]=='b')&&(buf[i+1]=='o')&&(buf[i+2]=='o')&&(buf[i+3]=='t')
                    &&(buf[i+4]=='i')&&(buf[i+5]=='m')&&(buf[i+6]=='a')&&(buf[i+7]=='g')&&(buf[i+8]=='e'))
                {
                    flag = 1;
                    index = i;
                    for(j=i;j<len;j++)
                    {
                        if((buf[j]>0x20)&&(buf[j]<0x7f)) //do not mach space
                        {
                            strbuf[t++] = buf[j];
                        }
                        else
                        {
                            strbuf[t] = '\0';
                            break;
                        }
                    }
                    if(strbuf[t] == '\0')
                    {
                        DEBUG("bootimage %s\n\n", strbuf);
                        break;
                    }
                }
            }
        }    
    }
    else if((option == OPT_READ_CMD)||(option == OPT_WRITE_CMD))
    {
         for(i=0;i<len;i++)
         {
            if((i<len-7)&&(flag == 0))
            {
                t = 0;
                if((buf[i]=='b')&&(buf[i+1]=='o')&&(buf[i+2]=='o')&&(buf[i+3]=='t')
                    &&(buf[i+4]=='c')&&(buf[i+5]=='m')&&(buf[i+6]=='d'))
                {
                    flag = 1;
                    index = i;
                    for(j=i;j<len;j++)
                    {
                        if((buf[j]>=0x20)&&(buf[j]<0x7f))
                        {
                            strbuf[t++] = buf[j];
                        }
                        else
                        {
                            strbuf[t] = '\0';
                            break;
                        }
                    }
                    if(strbuf[t] == '\0')
                    {
                        DEBUG("bootcmd %s\n", strbuf);
                        break;
                    }
                }
            }
         }
    }
    else if((option == OPT_READ_SERVERIP)||(option == OPT_WRITE_SERVERIP))
    {
         for(i=0;i<len;i++)
         {
            if((i<len-8)&&(flag == 0))
            {
                t = 0;
                if((buf[i]=='s')&&(buf[i+1]=='e')&&(buf[i+2]=='r')&&(buf[i+3]=='v')
                    &&(buf[i+4]=='e')&&(buf[i+5]=='r')&&(buf[i+6]=='i')&&(buf[i+7]=='p'))
                {
                    flag = 1;
                    index = i;
                    for(j=i;j<len;j++)
                    {
                        if((buf[j]>=0x20)&&(buf[j]<0x7f))
                        {
                            strbuf[t++] = buf[j];
                        }
                        else
                        {
                            strbuf[t] = '\0';
                            break;
                        }
                    }
                    if(strbuf[t] == '\0')
                    {
                        DEBUG("serverip %s\n", strbuf);
                        break;
                    }
                }
            }
         }
    }
    else if((option == OPT_READ_IPADDR)||(option == OPT_WRITE_IPADDR))
    {
         for(i=0;i<len;i++)
         {
            if((i<len-6)&&(flag == 0))
            {
                t = 0;
                if((buf[i]=='i')&&(buf[i+1]=='p')&&(buf[i+2]=='a')&&(buf[i+3]=='d')&&(buf[i+4]=='d')&&(buf[i+5]=='r'))
                {
                    flag = 1;
                    index = i;
                    for(j=i;j<len;j++)
                    {
                        if((buf[j]>=0x20)&&(buf[j]<0x7f))
                        {
                            strbuf[t++] = buf[j];
                        }
                        else
                        {
                            strbuf[t] = '\0';
                            break;
                        }
                    }
                    if(strbuf[t] == '\0')
                    {
                        DEBUG("ipaddr %s\n", strbuf);
                        break;
                    }
                }
            }
         }
    }    

    if((option == OPT_WRITE_CMD)||(option == OPT_WRITE_SERVERIP)||(option == OPT_WRITE_IPADDR))
    {
		int strbuf_len = strlen(strbuf);
		int value_len = strlen(value);
        int match_len = 0;//match str len
		DEBUG("Set value: %s len: %d\n",value,value_len);
		char *p = (char *)malloc(0x2000);
        for(i=0;i<0x2000;i++)
        {
           if(i>=(strbuf_len+index))
        	   p[i-strbuf_len-index] = buf[i];
        }
	    if(option == OPT_WRITE_CMD)
	    {
            for(i=0;i<value_len;i++)
            {
            	if(value[i] == '@')
            		value[i] = 0x20;
            }
            value[value_len] = 0x20;//add space
            value[value_len+1] = '\0';
            value_len += 1;
            match_len = 8;//bootcmd=
	    }
        else if(option == OPT_WRITE_SERVERIP)
        {
            match_len = 9;//serverip=
        }
        else if(option == OPT_WRITE_IPADDR)
        {
            match_len = 7;//ipaddr=
        }        
        for(i=0;i<0x2000;i++)
        {
            if(i>=(index+match_len)&&i<(index+match_len+value_len))
            {
                buf[i] = value[i-index-match_len];
            }
            else if(i>=(index+match_len+value_len))
            {
                buf[i] = p[i-index-match_len-value_len];
            }
        }        
        long crc=0;
        crc = ctclib_gen_crc32(0,&buf[4],0x2000-4);
        buf[0] = ((char *)&crc)[0];
        buf[1] = ((char *)&crc)[1];
        buf[2] = ((char *)&crc)[2];
        buf[3] = ((char *)&crc)[3];
        
        erase.start = offset;
        erase.length = 0x10000;
        int err = ioctl(fd, MEMERASE, &erase);
        if(err != 0)
        {
            DEBUG("erase flash failed\n ");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }
            return -1;
        }
        if (offset != lseek(fd, offset, SEEK_SET))
        {
            DEBUG("lseek set failed");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }
            return -1;
        }

        err = write(fd, buf,0x2000);
        if(err < 0)
        {
            DEBUG("set value failed");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }
            return -1;
        }		
    }
    else if(option == OPT_WRITE_OEM)
    {
        char oem_buf[256];
		int oemfd = open(value,O_SYNC|O_RDWR);
		if(oemfd<0)
		{
			DEBUG("open oem file %s failed!\n",value);
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }            
			return -1;
		}
		len = read(oemfd, oem_buf,256);
		if(close(oemfd)<0)
		{
			DEBUG("close oem failed\n");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }            
			return -1;
		}

        for(i=0;i<0xff;i++)/* oem 0x1300-0x13ff */
        {
            buf[OEM_OFFSET+i] = oem_buf[i];
        }
        erase.start = offset;
        erase.length = 0x10000;
        err = ioctl(fd, MEMERASE, &erase);
        if(err != 0)
        {
            DEBUG("erase flash failed\n ");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }
            return -1;
        }
        if (offset != lseek(fd, offset, SEEK_SET))
        {
            DEBUG("lseek set failed");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }
            return -1;
        }

        err = write(fd, buf,0x2000);
        if(err < 0)
        {
            DEBUG("set value failed");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }
            return -1;
        }	
    }
    else if(option == OPT_READ_OEM)
    {
        char oem_buf[256];
		int oemfd = open(value,O_SYNC|O_RDWR|O_CREAT|O_TRUNC);
		if(oemfd<0)
		{
			DEBUG("open oem file %s failed!\n",value);
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }            
			return -1;
		}
        for(i=0;i<256;i++)
        {
            oem_buf[i] = buf[OEM_OFFSET+i];
        }
		len = write(oemfd, oem_buf,256);
		if(close(oemfd)<0)
		{
			DEBUG("close oem failed\n");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }            
			return -1;
		}        
    }
    else if(option == OPT_WRITE_PORTINFO)
    {
        char portinfo_buf[256];
		int portinfofd = open(value,O_SYNC|O_RDWR);
		if(portinfofd<0)
		{
			DEBUG("open portinfo file %s failed!\n",value);
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }            
			return -1;
		}
		len = read(portinfofd, portinfo_buf,256);
		if(close(portinfofd)<0)
		{
			DEBUG("close portinfo failed\n");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }            
			return -1;
		}

        for(i=0;i<0xff;i++)/* portinfo 0x1400-0x14ff */
        {
            buf[PORTINFO_OFFSET+i] = portinfo_buf[i];
        }
        erase.start = offset;
        erase.length = 0x10000;
        err = ioctl(fd, MEMERASE, &erase);
        if(err != 0)
        {
            DEBUG("erase flash failed\n ");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }
            return -1;
        }
        if (offset != lseek(fd, offset, SEEK_SET))
        {
            DEBUG("lseek set failed");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }
            return -1;
        }

        err = write(fd, buf,0x2000);
        if(err < 0)
        {
            DEBUG("set value failed");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }
            return -1;
        }	
    }
    else if(option == OPT_READ_PORTINFO)
    {
        char portinfo_buf[256];
		int portinfofd = open(value,O_SYNC|O_RDWR|O_CREAT|O_TRUNC);
		if(portinfofd<0)
		{
			DEBUG("open portinfo file %s failed!\n",value);
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }            
			return -1;
		}
        for(i=0;i<256;i++)
        {
            portinfo_buf[i] = buf[PORTINFO_OFFSET+i];
        }
		len = write(portinfofd, portinfo_buf,256);
		if(close(portinfofd)<0)
		{
			DEBUG("close portinfo failed\n");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }            
			return -1;
		}        
    }    
    else if(option == OPT_WRITE_STMINFO)
    {
        char stminfo_buf[256];
		int stminfofd = open(value,O_SYNC|O_RDWR);
		if(stminfofd<0)
		{
			DEBUG("open stminfo file %s failed!\n",value);
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }            
			return -1;
		}
		len = read(stminfofd, stminfo_buf,256);
		if(close(stminfofd)<0)
		{
			DEBUG("close stm failed\n");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }            
			return -1;
		}

        for(i=0;i<0x5;i++)/* stminfo 0x1260-0x1264 */
        {
            buf[STMCOF_OFFSET+i] = stminfo_buf[i];
        }
        erase.start = offset;
        erase.length = 0x10000;
        err = ioctl(fd, MEMERASE, &erase);
        if(err != 0)
        {
            DEBUG("erase flash failed\n ");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }
            return -1;
        }
        if (offset != lseek(fd, offset, SEEK_SET))
        {
            DEBUG("lseek set failed");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }
            return -1;
        }

        err = write(fd, buf,0x2000);
        if(err < 0)
        {
            DEBUG("set value failed");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }
            return -1;
        }	
    }
    else if(option == OPT_READ_STMINFO)
    {
        char stminfo_buf[256];
		int stminfofd = open(value,O_SYNC|O_RDWR|O_CREAT|O_TRUNC);
		if(stminfofd<0)
		{
			DEBUG("open stminfo file %s failed!\n",value);
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }            
			return -1;
		}
        for(i=0;i<5;i++)/* stminfo 0x1260-0x1264 */
        {
            stminfo_buf[i] = buf[STMCOF_OFFSET+i];
        }
		len = write(stminfofd, stminfo_buf,256);
		if(close(stminfofd)<0)
		{
			DEBUG("close stminfo failed\n");
            if(close(fd)<0)
            {
                DEBUG("close fd failed");
                return -1;
            }            
			return -1;
		}        
    }        
    if(close(fd)<0)
    {
        DEBUG("close fd failed");
        return -1;
    }
    return 0;
}
