/****************************************************************************
 *
 * (C) Copyright Centec Networks Inc.  All rights reserved.
 *
 * Modify History:
 * Revision      : R0.01
 * Author        : Justin
 * Date          : 2007-011-14
 * Reason        : First Create.
 ****************************************************************************/
#include <unistd.h>
#include <sys/termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "proto.h"
#include "cdb_const.h"
#include "glb_tempfile_define.h"

int serial_speed_arr[] = {B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B600};
int serial_name_arr[]  = {115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200, 600};

/* success: 0, others -1 */
int check_serial_baudrate(int value)
{
    int nCnt = sizeof(serial_name_arr) / sizeof(serial_name_arr[0]);
    int i;

    for (i = 0; i < nCnt; i++) {
        if (value == serial_name_arr[i]) {
            return 0;
        }
    }
    return -1;
}

int serial_store_attr(char attr[], int val)
{
    FILE *fp = NULL;
    fp = fopen(GLB_SERIAL_ATTR, "w");
    if (NULL == fp)
        return -1;
    fprintf(fp, "%s %d\n", attr, val);
    fclose(fp);
    return 0;
}

 /*************************************************************
 * Name: imi_set_serial_baudrate
 * Purpose: set serial baudrate
 * Input: speed
 * Output: N/A
 * Return: 1: fail  0:success
 * Note: N/A
 ***********************************************************/
int 
set_serial_baudrate(int speed)
{
    int fd;
    int i;
    struct termios Opt;
    struct termios opt_shadow;

    fd = open("/dev/console", O_RDWR);
    if(-1 == fd)
    {
        return 1;
    }
    
    tcgetattr(fd, &Opt);
 
    for(i = 0; i < sizeof(serial_speed_arr)/sizeof(int); i ++)
    {
        if(speed == serial_name_arr[i])
        {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&Opt, serial_speed_arr[i]);
            cfsetospeed(&Opt, serial_speed_arr[i]);
            tcsetattr(fd, TCSANOW, & Opt);

            tcgetattr(fd, &opt_shadow);
            if( ( cfgetispeed(&Opt) !=  cfgetispeed(&opt_shadow) )  \
                    ||  ( cfgetospeed(&Opt) != cfgetospeed(&opt_shadow) )) {
                goto ERR_BND_SET;
            }
            
            tcflush(fd, TCIOFLUSH);
            break;
        }
        else
        {
            if(i == (sizeof(serial_speed_arr)/sizeof(int) - 1))
            {
                goto ERR_BND_SET;
            }
        }
    }

    serial_store_attr("speed", speed);
    
    close(fd);
    return 0;

ERR_BND_SET:
    close(fd);
    return 1;
}
    
 /*************************************************************
 * Name: imi_set_serial_parity
 * Purpose: set serial parity
 * Input: parity
 * Output: N/A
 * Return: 1: fail  0:success
 * Note: N/A
 ***********************************************************/
int 
set_serial_parity(int parity)
{
    int fd;
    struct termios Opt;
     
    fd = open("/dev/console", O_RDWR);    
    
    if(-1 == fd)
    {
        return 1;
    }
    
    if(tcgetattr(fd, &Opt) != 0)
    {
        close(fd);
        return -1;
    }
    
    switch(parity)
    {
        case PARITY_NONE:
            Opt.c_cflag &= ~PARENB;             /*clear parity enable */
            Opt.c_iflag &= ~INPCK;              /*Enable parity checking*/
            break;
        case PARITY_ODD:
            Opt.c_cflag |= (PARODD | PARENB);   /* odd check*/
            break;
        case PARITY_EVEN:
            Opt.c_cflag |= PARENB;              /* Enable parity */
            Opt.c_cflag &= ~PARODD;             /* even check */
            break;
        default:
            close(fd);
            return -1;
    }
    
    /* Set input parity option */
    if(parity != 'n')
        Opt.c_iflag |= INPCK;
    tcflush(fd, TCIFLUSH);

    if(tcsetattr(fd, TCSANOW, &Opt) != 0)
    {
        close(fd);
        return 1;
    }

    serial_store_attr("parity", parity);

    close(fd);
    return 0;
}

 /*************************************************************
 * Name: imi_set_serial_databits
 * Purpose: set serial databis
 * Input: databits
 * Output: N/A
 * Return: 1: fail  0:success
 * Note: N/A
 ***********************************************************/
int 
set_serial_databits(int databits)
{
    int fd;
    struct termios Opt;
    
    fd = open("/dev/console", O_RDWR);
    if(-1 == fd)
    {
        return 1;
    }
    
    if(tcgetattr(fd, &Opt) != 0)
    {
        close(fd);
        return -1;
    }
    
    Opt.c_cflag &= ~CSIZE;

    /* set data bits */
    switch(databits)
    {
        case 7:
            Opt.c_cflag |= CS7;
            break;
        case 8:
            Opt.c_cflag |= CS8;
            break;
        default:
            close(fd);
            return -1;
    }

    tcflush(fd, TCIFLUSH);
    
    if(tcsetattr(fd, TCSANOW, &Opt) != 0)
    {
        close(fd);
        return 1;
    }

    serial_store_attr("databits", databits);

    close(fd);
    return 0;
}

 /*************************************************************
 * Name: imi_set_serial_stopbits
 * Purpose: set serial stopbits
 * Input: stopbits
 * Output: N/A
 * Return: 1: fail  0:success
 * Note: N/A
 ***********************************************************/
int 
set_serial_stopbits(int stopbits)
{
    int fd;
    struct termios Opt;
    
    fd = open("/dev/console", O_RDWR);
    if(-1 == fd)
    {
        return 1;
    }
    
    if(tcgetattr(fd, &Opt) != 0)
    {
        close(fd);
        return -1;
    }
    
    switch(stopbits)
    {
        case 1:
            Opt.c_cflag &= ~CSTOPB;
            break;
        case 2:
            Opt.c_cflag |= CSTOPB;
            break;
        default:
            close(fd);
            return -1;
    }

    tcflush(fd, TCIFLUSH);
    
    if(tcsetattr(fd, TCSANOW, &Opt) != 0)
    {
        close(fd);
        return 1;
    }

    serial_store_attr("stopbits", stopbits);
    
    close(fd);
    return 0;
}

 /*************************************************************
 * Name: imi_get_serial_speed
 * Purpose: get serial speed
 * Input: N/A
 * Output: N/A
 * Return: buadrate
 * Note: N/A
 ***********************************************************/
int
get_serial_buadrate()
{
    int fd;
    int i;
    int ary_len;
    speed_t bd_rate;
    struct termios opt;

    fd = open("/dev/console", O_RDWR );
    if ( -1 == fd ) 
    {
        return -1;
    }

    tcgetattr(fd, &opt);
    bd_rate = cfgetispeed( &opt);

    ary_len =  sizeof( serial_speed_arr ) / sizeof( int );
    for ( i = 0; i < ary_len; ++i ) 
    {
        if ( bd_rate == serial_speed_arr[i] ) 
        {
            close(fd);
            return serial_name_arr[i];
        }
    }

    close(fd);
    return -1;
}

 /*************************************************************
 * Name: imi_get_serial_parity
 * Purpose: get serial parity
 * Input: N/A
 * Output: N/A
 * Return: parity
 * Note: N/A
 ***********************************************************/ 
int 
get_serial_parity( void )
{
    int fd;
    int return_val;
    struct termios opt;

    fd = open("/dev/console", O_RDWR );
    if ( -1 == fd ) 
    {
        return -1;
    }

    tcgetattr(fd, &opt);

    return_val = -1;
    if ( !( opt.c_cflag & PARENB ) ) 
    {

        return_val = 0;
        goto RET;

    }
    else 
    {
        if ( opt.c_cflag & PARODD )
        {
            return_val = 3;
            goto RET;
        }
        else 
        {
            return_val = 6;
            goto RET;
        }
    }

    RET:
    close(fd);
    return return_val;
}

 /*************************************************************
 * Name: imi_get_serial_databits
 * Purpose: get serial databits
 * Input: N/A
 * Output: N/A
 * Return: databits
 * Note: N/A
 ***********************************************************/ 
int 
get_serial_databits( void )
{
    int fd;
    int return_val;
    struct termios opt;

    fd = open("/dev/console", O_RDWR );
    if( -1 == fd ) {
        return -1;
    }

    tcgetattr(fd, &opt);

    return_val = -1;
    switch( opt.c_cflag & CSIZE ) 
    {
        case CS5:
            return_val = 5;
            break;
        case CS6:
            return_val = 6;
            break;
        case CS7:
            return_val = 7;
            break;
        case CS8:
            return_val = 8;
            break;
        default:
            return_val = 1;
    }

    close(fd);
    return return_val;
}

 /*************************************************************
 * Name: imi_get_serial_stopbits
 * Purpose: get serial stopbits
 * Input: N/A
 * Output: N/A
 * Return: stopbits
 * Note: N/A
 ***********************************************************/ 
int 
get_serial_stopbits( void )
{
    int fd;
    int return_val;
    struct termios opt;

    fd = open("/dev/console", O_RDWR );
    if ( -1 == fd ) 
    {
        perror("open /dev/console failed.\n");
        return -1;
    }

    return_val = -1;
    tcgetattr(fd, &opt);
    if ( opt.c_cflag & CSTOPB ) 
    {
        return_val = 2; /* 2 bit stop */
    }
    else 
    {
        return_val = 1; /* 1 bit stop */
    }

    close(fd);
    return return_val;
}

