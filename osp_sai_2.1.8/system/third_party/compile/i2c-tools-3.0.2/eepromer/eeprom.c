/*
This program is hereby placed into the public domain.
Of course the program is provided without warranty of any kind.
*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <linux/i2c-dev.h>

/*
  this program can read 24C16 (and probably smaller ones, too)
  I wrote it as a quick and dirty hack because my satellite receiver
  hung again... so I had to reprogram the eeprom where is stores it's
  settings.
 */

#define DEFAULT_I2C_BUS      "/dev/i2c-0"
#define DEFAULT_EEPROM_ADDR  0x50         /* the 24C16 sits on i2c address 0x50 */
#define MAX_BYTES            8            /* max number of bytes to write in one chunk */
       /* ... note: 24C02 and 24C01 only allow 8 bytes to be written in one chunk.   *
        *  if you are going to write 24C04,8,16 you can change this to 16            */

/* write len bytes (stored in buf) to eeprom at address addr, page-offset offset */
/* if len=0 (buf may be NULL in this case) you can reposition the eeprom's read-pointer */
/* return 0 on success, -1 on failure */
int eeprom_write(int fd,
		 unsigned int addr,
		 unsigned int offset,
		 unsigned char *buf,
		 unsigned char len
){
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg             i2cmsg;
	int i;
	unsigned char _buf[MAX_BYTES + 2];

	if(len>MAX_BYTES){
	    fprintf(stderr,"I can only write max %d bytes at a time!\n", MAX_BYTES);
	    return -1;
	}

	if(len+offset >256){
	    fprintf(stderr,"Sorry, len(%d)+offset(%d) > 256 (page boundary)\n",
			len,offset);
	    return -1;
	}

	_buf[0]=0x0ff & (offset>>8);    /* _buf[0] is the offset into the eeprom page! */
	_buf[1]=0x0ff & offset;    /* _buf[1] is the offset into the eeprom page! */
	for(i=0;i<len;i++) /* copy buf[0..n] -> _buf[2..n+2] */
	    _buf[2+i]=buf[i];

	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;

	i2cmsg.addr  = addr;
	i2cmsg.flags = 0;
	i2cmsg.len   = 2+len;
	i2cmsg.buf   = _buf;

	if((i=ioctl(fd,I2C_RDWR,&msg_rdwr))<0){
	    perror("ioctl()");
	    fprintf(stderr,"ioctl returned %d\n",i);
	    return -1;
	}

#if 0
	if(len>0) {
	    fprintf(stderr,"Wrote %d bytes to eeprom at 0x%02x, offset %08x\n",
		    len,addr,offset);
	} else {
	    fprintf(stderr,"Positioned pointer in eeprom at 0x%02x to offset %08x\n",
		    addr,offset);
    }
#endif

	return 0;
}

/* read len bytes stored in eeprom at address addr, offset offset in array buf */
/* return -1 on error, 0 on success */
int eeprom_read(int fd,
		 unsigned int addr,
		 unsigned int offset,
		 unsigned char *buf,
		 unsigned short len
){
	struct i2c_rdwr_ioctl_data msg_rdwr;
	struct i2c_msg             i2cmsg;
	int i;

	if(eeprom_write(fd,addr,offset,NULL,0)<0)
	    return -1;

	msg_rdwr.msgs = &i2cmsg;
	msg_rdwr.nmsgs = 1;

	i2cmsg.addr  = addr;
	i2cmsg.flags = I2C_M_RD;
	i2cmsg.len   = len;
	i2cmsg.buf   = buf;

	if((i=ioctl(fd,I2C_RDWR,&msg_rdwr))<0){
	    perror("ioctl()");
	    fprintf(stderr,"ioctl returned %d\n",i);
	    return -1;
	}

	fprintf(stderr,"Read %d bytes from eeprom at 0x%02x, offset %08x\n",
		len,addr,offset);

	return 0;
}



int main(int argc, char **argv){
    int i,j;

    /* filedescriptor and name of device */
    int d; 
    char *dn=DEFAULT_I2C_BUS;

    /* filedescriptor and name of data file */
    int f=-1;
    char *fn=NULL;

    unsigned int addr=DEFAULT_EEPROM_ADDR;
    int rwmode=0;
    int bytes = -1;

    unsigned char buf[MAX_BYTES];
    unsigned char *rbuf = NULL;
    int offset = 0;
    
    while((i=getopt(argc,argv,"d:a:b:wf:h"))>=0){
        switch(i){
        case 'h':
            fprintf(stderr,"%s [-d dev] [-a adr] [-b bytes] [-w] [-f file]\n",argv[0]);
            fprintf(stderr,"\tdev: device, e.g. /dev/i2c-0    (def)\n");
            fprintf(stderr,"\tadr: base address of eeprom, eg 0xA0 (def)\n");
            fprintf(stderr,"\tbytes: bytes\n");
            fprintf(stderr,"\t-w : write to eeprom (default is reading!)\n");
            fprintf(stderr,"\t-f file: copy eeprom contents to/from file\n");
            exit(1);
            break;
        case 'd':
            dn=optarg;
            break;
        case 'a':
            addr = strtol(optarg, NULL, 0);
            break;
        case 'b':
            bytes = strtol(optarg, NULL, 0);
            break;
        case 'w':
            rwmode++;
            break;
        case 'f':
            fn=optarg;
            break;
        }
    }
   
    fprintf(stderr,"base-address of eeproms       : 0x%02x\n",addr);

    if(fn){
        if(!rwmode) /* if we are reading, *WRITE* to file */
            f=open(fn,O_WRONLY|O_CREAT,0666);
        else /* if we are writing to eeprom, *READ* from file */
            f=open(fn,O_RDONLY);
        if(f<0){
            fprintf(stderr,"Could not open data-file %s for reading or writing\n",fn);
            perror(fn);
            exit(1);
        }
        fprintf(stderr,"file opened for %7s       : %s\n",rwmode?"reading":"writing",fn);
        fprintf(stderr,"            on filedescriptor : %d\n",f);
    }

    if((d=open(dn,O_RDWR))<0){
        fprintf(stderr,"Could not open i2c at %s\n",dn);
        perror(dn);
        exit(1);
    }

    fprintf(stderr,"i2c-devicenode is             : %s\n",dn);
    fprintf(stderr,"            on filedescriptor : %d\n\n",d);


    offset = 0;
    if(rwmode){

        j=read(f,buf,sizeof(buf));
        if(j<0){
            fprintf(stderr,"Cannot read from file '%s'\n",fn);
            perror(fn);
            exit(1);
        }
        while (j > 0) {
            if(eeprom_write(d,addr,offset,buf,j)<0) {
                perror("");
                exit(1);
            }
            offset += j;
            j=read(f,buf,sizeof(buf));
            if(j<0){
                fprintf(stderr,"Cannot read from file '%s'\n",fn);
                perror(fn);
                exit(1);
            }
        }
    } else {
        if (bytes <= 0) {
            fprintf(stderr,"Cannot read %d bytes from eeprom\n", bytes);
        } else {
            rbuf = malloc(bytes);
            j = eeprom_read(d,addr,offset,rbuf,bytes);
            if(j<0){
                fprintf(stderr,"Cannot read from eeprom\n");
                perror("");
                exit(1);
            } else {
                j=write(f,rbuf,bytes);
                if (j != bytes) {
                    fprintf(stderr, "write %d\%d bytes\n", j, bytes);
                }
            }
            if (rbuf) {
                free(rbuf);
                rbuf = NULL;
            }
        }
    }

    if(f>=0) {
        close(f);
    }

    close(d);

    exit(0);

}
