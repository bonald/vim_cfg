/**
 @file ctc_cli.c

 @date 2010-8-03

 @version v2.0

  This file contains ctc cli api implementation
*/

#include "ctc_cli.h"

#ifdef SDK_IN_VXWORKS
#include "ioLib.h"
#include "types/vxTypesBase.h"
#endif

#include "stdarg.h"

#define DECIMAL_STRLEN_MIN 1
#define DECIMAL_STRLEN_MAX 10

#define strtou32(x,y,z) strtoul((char*)x,(char**)y,z)

CTC_CLI_PORT_MAP_FUNC cli_port_map_func_ptr;

CTC_CLI_OUT_FUNC cli_print_func_ptr = printf;

unsigned char
ctc_cli_get_prefix_item (char **argv ,unsigned char argc ,char *prefix,unsigned char prefix_len)
{
    unsigned char index =0;

    while(index <argc )
    {
        if (!strncmp(argv[index], prefix, prefix_len))
        {
            return index;
        }
        index++;
    }
    return 0xFF;
}

void ctc_cli_register_print_fun(CTC_CLI_OUT_FUNC func)
{
     cli_print_func_ptr = func;
}

char print_buf[1024];
char print_line[512];
int ctc_cli_out(const char *fmt, ...)
{
    int i = 0;
    int j = 0;

    va_list ap;

    va_start(ap, fmt);
#ifdef SDK_IN_VXWORKS
    vsprintf(print_buf, fmt, ap);
#else
    vsnprintf(print_buf, 1023, fmt, ap);
#endif
    va_end(ap);
    i = 0;
    while(i<1024 &&  j<512 && print_buf[i] != '\0')
    {
        switch(print_buf[i])
        {
            case '\n':
                print_line[j] = '\0';
                (*cli_print_func_ptr)("%s\r\n", print_line);
                j = 0;
                break;

            /*Added by xgu, when print '%' should follow '%', otherwise will crash, 2013-2-28 */
            case '%':
                print_line[j] = '%';
                j++;
                print_line[j] = '%';
                j++;
                break;

            default:
                print_line[j] = print_buf[i];
                j++;
                break;
        }
        i++;
    }
    if(print_buf[i] == '\0')
    {
        print_line[j] = '\0';
        (*cli_print_func_ptr)("%s", print_line);
    }

    return 0;
}

int32
ctc_cmd_str2int (char *str, int32 *ret)
{
  uint32 i;
  uint32 len;
  uint32 digit;
  uint32 limit, remain;
  uint32 minus = 0;
  uint32 max = 0xFFFFFFFF;
  uint32 total = 0;

  /* Sanify check. */
  if (str == NULL || ret == NULL)
    return -1;

  /* First set return value as error. */
  *ret = -1;

  len = strlen (str);
  if (*str == '+')
    {
      str++;
      len--;
    }
  else if (*str == '-')
    {
      str++;
      len--;
      minus = 1;
      max = max / 2 + 1;

    }
  /*add for suport parser hex format*/
    if (len >= 2 && !memcmp(str, "0x", 2))
    {

     if(len == 2)
      {
              *ret = -1;
              return 0xFFFFFFFF;
      }
      else if(len > 10)
      {
              *ret = -1;
              return 0xFFFFFFFF;
      }

      for(i = 2; i < len; i++)
      {
            if ((*(str+i) <= '9' && *(str+i) >= '0')
              || (*(str+i) <= 'f' && *(str+i) >=  'a')
              || (*(str+i) <= 'F' && *(str+i) >= 'A'))
            {
                /*do nothing*/
            }
            else
            {
              return -1;
            }

      }
      total = strtou32(str, NULL, 16);
    }
    else
    {

        limit = max / 10;
        remain = max % 10;

        if (len < DECIMAL_STRLEN_MIN || len > DECIMAL_STRLEN_MAX)
            return -1;

        for (i = 0; i < len; i++)
        {
            if (*str < '0' || *str > '9')
                return -1;

            digit = *str++ - '0';

            if (total  > limit || (total == limit && digit > remain))
                return -1;

            total = total * 10 + digit;
        }
    }

  *ret = 0;
  if (minus && (total == 0))
      return -1;
  if (minus)
    return - total;
  else
    return total;
}

uint32
ctc_cmd_str2uint (char *str, int32 *ret)
{
  uint32 i;
  uint32 len;
  uint32 digit;
  uint32 limit, remain;
  uint32 max = 0xFFFFFFFF;
  uint32 total = 0;

  /* Sanify check. */
  if (str == NULL || ret == NULL)
    return 0xFFFFFFFF;

  /* First set return value as error. */
  *ret = -1;

  len = strlen (str);

  /*add for suport parser hex format*/
    if (len >= 2 && !memcmp(str, "0x", 2))
    {
      if(len == 2)
      {
              *ret = -1;
              return 0xFFFFFFFF;
      }
      else if(len > 10)
      {
              *ret = -1;
              return 0xFFFFFFFF;
      }
      for(i = 2; i < len; i++)
      {
            if ((*(str+i) <= '9' && *(str+i) >= '0')
              || (*(str+i) <= 'f' && *(str+i) >=  'a')
              || (*(str+i) <= 'F' && *(str+i) >= 'A'))
            {
                /*do nothing*/
            }
            else
            {
              *ret = -1;
              return 0xFFFFFFFF;
            }

      }
      total = strtou32(str, NULL, 16);
    }
    else
    {

        limit = max / 10;
        remain = max % 10;

        if (len < DECIMAL_STRLEN_MIN || len > DECIMAL_STRLEN_MAX)
          {
            *ret = -1;
            return 0xFFFFFFFF;
          }

        for (i = 0; i < len; i++)
        {
            if (*str < '0' || *str > '9')
              {
                *ret = -1;
                return 0xFFFFFFFF;
              }

            digit = *str++ - '0';

            if (total  > limit || (total == limit && digit > remain))
              {
                 *ret = -1;
                return 0xFFFFFFFF;
              }

            total = total * 10 + digit;
        }
    }

  *ret = 0;
  return total;
}

int32
ctc_cmd_judge_is_num(char *str)
{
    uint32 i;
    uint32 len;

    /* Sanify check. */
    if (NULL == str)
        return -1;

    len = strlen (str);

    /*add for suport parser hex format*/
    if (len >= 2 && !memcmp(str, "0x", 2))
    {
        if(len == 2)
        {
            return -1;
        }
        for(i = 2; i < len; i++)
        {
            if ((*(str+i) <= '9' && *(str+i) >= '0')
              || (*(str+i) <= 'f' && *(str+i) >= 'a')
              || (*(str+i) <= 'F' && *(str+i) >= 'A'))
            {
                /*do nothing*/
            }
            else
            {
                return -1;
            }
        }
    }
    else
    {
        if (len < DECIMAL_STRLEN_MIN || len > DECIMAL_STRLEN_MAX)
        {
            return -1;
        }

        for (i = 0; i < len; i++)
        {
            if (*str < '0' || *str > '9')
            {
                return -1;
            }
        }
    }

    return 0;
}



void
ctc_uint64_to_str(uint64 src, char dest[UINT64_STR_LEN])
{
    int8 i = UINT64_STR_LEN - 1, j = 0;
    uint64 value, sum;

    memset(dest, 0, UINT64_STR_LEN);
    if (0 == src)
    {
        dest[0] = 48;
        return ;
    }

    sum = src;
    while (sum)
    {
        value =  sum % 10;
        dest[(uint8)i--] = value + 48;
        if (i < 0)
            break;

        sum = sum / 10;
    }

    /*move the string to the front*/
    for (j = 0; j < (UINT64_STR_LEN - 1 - i); j++)
        dest[(uint8)j] = dest[(uint8)i + (uint8)j + 1];

    for (; j <= UINT64_STR_LEN - 1; j++)
        dest[(uint8)j] = 0;
}

#ifndef SDK_IN_VXWORKS
struct termios termios_old;
#else
static int termios_fd;
static int termios_old;
static int termios_new;
#endif
void set_terminal_raw_mode(void)
{
#ifdef SDK_IN_VXWORKS
    termios_fd = ioTaskStdGet(0,STD_IN);
    termios_old = ioctl(ioTaskStdGet(0,STD_IN), FIOGETOPTIONS, 0);
    termios_new = termios_old & ~(OPT_LINE | OPT_ECHO);
    ioctl(ioTaskStdGet(0,STD_IN), FIOSETOPTIONS, termios_new);
#elif SDK_IN_USERMODE
    /*system("stty raw -echo");*/
    struct termios terminal_new;
    tcgetattr(0, &terminal_new);
    memcpy(&termios_old, &terminal_new, sizeof(struct termios));
    terminal_new.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                               | INLCR | IGNCR | ICRNL | IXON);
    /*
      OPOST (output post-processing) & ISIG (Input character signal generating enabled) need to be set
      terminal_new.c_oflag &= ~OPOST;
      terminal_new.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
      */
    terminal_new.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN);
    terminal_new.c_cflag &= ~(CSIZE | PARENB);
    terminal_new.c_cflag |= CS8;

    tcsetattr(0, TCSANOW, &terminal_new);
#endif
}
void restore_terminal_mode(void)
{
#ifdef SDK_IN_VXWORKS
    ioctl(ioTaskStdGet(0,STD_IN), FIOSETOPTIONS, termios_old);
#elif SDK_IN_USERMODE
    /*system("stty cooked echo");*/
    tcsetattr(0, TCSANOW, &termios_old);
#endif
    printf("\n");

}
extern char *
ctc_cli_get_debug_desc (unsigned char level)
{
    static char debug_level_desc[64];
    memset(debug_level_desc, 0, 63);
    if (level & 0x01)
    {
        strcat(debug_level_desc, "Func/");
    }
    if (level & 0x02)
    {
        strcat(debug_level_desc, "Param/");
    }
    if (level & 0x04)
    {
        strcat(debug_level_desc, "Info/");
    }
    if (level & 0x08)
    {
        strcat(debug_level_desc, "Error/");
    }
    if (strlen(debug_level_desc) != 0)
    {
        debug_level_desc[strlen(debug_level_desc) -1] = '\0';
    }
    else
    {
          strcpy(debug_level_desc, "None");
    }
    return debug_level_desc;
}

