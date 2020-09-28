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



CTC_CLI_OUT_FUNC cli_print_func_ptr = sal_printf;
uint8 g_cli_parser_param_en = 0;

/* prefix_len == 0: match whole world. */
unsigned char
ctc_cli_get_prefix_item(char** argv, unsigned char argc, char* prefix, unsigned char prefix_len)
{
    unsigned char index = 0;

    while (index < argc)
    {
        if ((prefix_len && !sal_strncmp(argv[index], prefix, prefix_len)) ||
            (!sal_strncmp(argv[index], prefix, sal_strlen(argv[index])) &&
             (sal_strlen(argv[index]) == sal_strlen(prefix))) )
        {
            return index;
        }

        index++;
    }

    return 0xFF;
}

void
ctc_cli_register_print_fun(CTC_CLI_OUT_FUNC func)
{
    cli_print_func_ptr = func;
}

char print_buf[1024];
char g_ctc_cli_out_print_buf[1026];
char print_line[1024];
int
ctc_cli_out(const char* fmt, ...)
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

    while (i < 1023 && j < 512 && print_buf[i] != '\0')
    {
        switch (print_buf[i])
        {
        case '\n':
            print_line[j] = '\0';
            if(g_ctc_vti)
            {
                sal_strcpy(g_ctc_cli_out_print_buf,print_line);
                sal_strcat(g_ctc_cli_out_print_buf,"\r\n");

                if (g_ctc_vti->output)
                {
                    g_ctc_vti->output(g_ctc_cli_out_print_buf, sal_strlen(g_ctc_cli_out_print_buf), NULL);
                }
                else if (g_ctc_vti->grep && g_ctc_vti->fprintf)
                {
                    g_ctc_vti->grep(g_ctc_vti, g_ctc_cli_out_print_buf, sal_strlen(g_ctc_cli_out_print_buf));
                }
                else if (g_ctc_vti->printf)
                {
                    g_ctc_vti->printf(g_ctc_vti, g_ctc_cli_out_print_buf, sal_strlen(g_ctc_cli_out_print_buf));
                }
            }
            else
            {
                (* cli_print_func_ptr)("%s\r\n", print_line);
            }
            j = 0;
            break;

        default:
            print_line[j] = print_buf[i];
            j++;
            break;
        }

        i++;
    }

    if (print_buf[i] == '\0')
    {
        print_line[j] = '\0';
        if(g_ctc_vti )
        {
            if (g_ctc_vti->output)
            {
                g_ctc_vti->output(print_line, sal_strlen(print_line), NULL);
            }
            else if (g_ctc_vti->grep && g_ctc_vti->fprintf)
            {
                g_ctc_vti->grep(g_ctc_vti, print_line, sal_strlen(g_ctc_cli_out_print_buf));
            }
            else if (g_ctc_vti->printf)
            {
                g_ctc_vti->printf(g_ctc_vti, print_line, sal_strlen(print_line) );
            }
        }
        else
        {
            (* cli_print_func_ptr)("%s", print_line);
        }
    }

    return 0;
}

int32
ctc_cmd_str2int(char* str, int32* ret)
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
    {
        return -1;
    }

    /* First set return value as error. */
    *ret = -1;

    len = sal_strlen(str);
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
    if (len >= 2 && !sal_memcmp(str, "0x", 2))
    {

        if (len == 2)
        {
            *ret = -1;
            return 0xFFFFFFFF;
        }
        else if (len > 10)
        {
            *ret = -1;
            return 0xFFFFFFFF;
        }

        for (i = 2; i < len; i++)
        {
            if ((*(str + i) <= '9' && *(str + i) >= '0')
                || (*(str + i) <= 'f' && *(str + i) >= 'a')
                || (*(str + i) <= 'F' && *(str + i) >= 'A'))
            {
                /*do nothing*/
            }
            else
            {
                return -1;
            }
        }

        total = sal_strtou32(str, NULL, 16);
    }
    else
    {

        limit = max / 10;
        remain = max % 10;

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

            digit = *str++ - '0';

            if (total > limit || (total == limit && digit > remain))
            {
                return -1;
            }

            total = total * 10 + digit;
        }
    }

    *ret = 0;
    if (minus && (total == 0))
    {
        return -1;
    }

    if (minus)
    {
        return -total;
    }
    else
    {
        return total;
    }
}

uint32
ctc_cmd_str2uint(char* str, int32* ret)
{
    uint32 i;
    uint32 len;
    uint32 digit;
    uint32 limit, remain;
    uint32 max = 0xFFFFFFFF;
    uint32 total = 0;

    /* Sanify check. */
    if (str == NULL || ret == NULL)
    {
        return 0xFFFFFFFF;
    }

    /* First set return value as error. */
    *ret = -1;

    len = sal_strlen(str);

    /*add for suport parser hex format*/
    if (len >= 2 && !sal_memcmp(str, "0x", 2))
    {
        if (len == 2)
        {
            *ret = -1;
            return 0xFFFFFFFF;
        }
        else if (len > 10)
        {
            *ret = -1;
            return 0xFFFFFFFF;
        }

        for (i = 2; i < len; i++)
        {
            if ((*(str + i) <= '9' && *(str + i) >= '0')
                || (*(str + i) <= 'f' && *(str + i) >= 'a')
                || (*(str + i) <= 'F' && *(str + i) >= 'A'))
            {
                /*do nothing*/
            }
            else
            {
                *ret = -1;
                return 0xFFFFFFFF;
            }
        }

        total = sal_strtou32(str, NULL, 16);
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

            if (total > limit || (total == limit && digit > remain))
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

uint64
ctc_cmd_str2uint64(char* str, int32* ret)
{
#define UINT64_DECIMAL_STRLEN_MAX 20
    uint32 i;
    uint32 len;
    uint32 digit;
    uint64 limit, remain;
    uint64 max = 0xFFFFFFFFFFFFFFFFULL;
    uint64 total = 0;

    /* Sanify check. */
    if (str == NULL || ret == NULL)
    {
        return 0xFFFFFFFFFFFFFFFFULL;
    }

    /* First set return value as error. */
    *ret = -1;

    len = sal_strlen(str);

    /*add for suport parser hex format*/
    if (len >= 2 && !sal_memcmp(str, "0x", 2))
    {
        if (len == 2)
        {
            *ret = -1;
            return 0xFFFFFFFFFFFFFFFFULL;
        }
        else if (len > 18)
        {
            *ret = -1;
            return 0xFFFFFFFFFFFFFFFFULL;
        }

        for (i = 2; i < len; i++)
        {
            if ((*(str + i) <= '9' && *(str + i) >= '0')
                || (*(str + i) <= 'f' && *(str + i) >= 'a')
                || (*(str + i) <= 'F' && *(str + i) >= 'A'))
            {
                /*do nothing*/
            }
            else
            {
                *ret = -1;
                return 0xFFFFFFFFFFFFFFFFULL;
            }
        }

        total = sal_strtou64(str, NULL, 16);
    }
    else
    {

        limit = max / 10;
        remain = max % 10;

        if (len < DECIMAL_STRLEN_MIN || len > UINT64_DECIMAL_STRLEN_MAX)
        {
            *ret = -1;
            return 0xFFFFFFFFFFFFFFFFULL;
        }

        for (i = 0; i < len; i++)
        {
            if (*str < '0' || *str > '9')
            {
                *ret = -1;
                return 0xFFFFFFFFFFFFFFFFULL;
            }

            digit = *str++ - '0';

            if (total > limit || (total == limit && digit > remain))
            {
                *ret = -1;
                return 0xFFFFFFFFFFFFFFFFULL;
            }

            total = total * 10 + digit;
        }
    }

    *ret = 0;
    return total;
}

int32
ctc_cmd_judge_is_num(char* str)
{
    uint32 i;
    uint32 len;

    /* Sanify check. */
    if (NULL == str)
    {
        return -1;
    }

    len = sal_strlen(str);

    /*add for suport parser hex format*/
    if (len >= 2 && !sal_memcmp(str, "0x", 2))
    {
        if (len == 2)
        {
            return -1;
        }

        for (i = 2; i < len; i++)
        {
            if ((*(str + i) <= '9' && *(str + i) >= '0')
                || (*(str + i) <= 'f' && *(str + i) >= 'a')
                || (*(str + i) <= 'F' && *(str + i) >= 'A'))
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

    sal_memset(dest, 0, UINT64_STR_LEN);
    if (0 == src)
    {
        dest[0] = 48;
        return;
    }

    sum = src;

    while (sum)
    {
        value = sum % 10;
        dest[(uint8)i--] = value + 48;
        if (i < 0)
        {
            break;
        }

        sum = sum / 10;
    }

    /*move the string to the front*/
    for (j = 0; j < (UINT64_STR_LEN - 1 - i); j++)
    {
        dest[(uint8)j] = dest[(uint8)i + (uint8)j + 1];
    }

    for (; j <= UINT64_STR_LEN - 1; j++)
    {
        dest[(uint8)j] = 0;
    }
}

int32
ctc_cmd_str2nint (char *str, int32 *ret, uint32 *val)
{
  uint32 i;
  int32 j;
  uint32 len;
  uint32 digit;
  uint32 limit, remain;
  uint32 minus = 0;
  uint32 max = 0xFFFFFFFF;
   /*uint32 total = 0;*/
  char tmpstr[10] = "\0";


  /* Sanify check. */
  if (str == NULL || ret == NULL)
    return -1;

  /* First set return value as error. */
  *ret = -1;

  len = sal_strlen (str);
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
     /*don't suport parser negative integer format by hezc 20091208*/
     return -1;
    }
  /*add by hezc for suport parser hex format*/
    if (len >= 2 && !sal_memcmp(str, "0x", 2))
    {

     if(len == 2)
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
      for(j = len-2, i=0; j > 0; j-=8, i++)
      {
            if (j<8)
            {
                sal_strncpy(tmpstr, str+2, j);
                tmpstr[j] = '\0';
            }
            else
            {
                sal_strncpy(tmpstr, str+len-(i+1)*8, 8);
                tmpstr[8] = '\0';
            }
            val[i] = sal_strtou32(tmpstr, NULL, 16);
      }
       /*total = sal_strtou32(str, NULL, 16);*/
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

            if (val[0]  > limit || (val[0] == limit && digit > remain))
                return -1;

            val[0] = val[0] * 10 + digit;
        }
    }

  *ret = 0;
  if (minus && (val[0] == 0))
      return -1;
  if (minus)
      val[0] = -val[0];

    return 0;
}



#ifdef SDK_IN_USERMODE
struct termios termios_old;
#elif defined SDK_IN_VXWORKS
static int termios_fd;
static int termios_old;
static int termios_new;
#endif
void
set_terminal_raw_mode(uint32 mode)
{
    if(CTC_VTI_SHELL_MODE_DEFAULT == mode)
    {
#ifdef SDK_IN_VXWORKS
        termios_fd = ioTaskStdGet(0, STD_IN);
        termios_old = ioctl(ioTaskStdGet(0, STD_IN), FIOGETOPTIONS, 0);
        termios_new = termios_old & ~(OPT_LINE | OPT_ECHO);
        ioctl(ioTaskStdGet(0, STD_IN), FIOSETOPTIONS, termios_new);
#elif defined SDK_IN_USERMODE
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
    else
    {
        /* write your code here */
    }
}

void
restore_terminal_mode(uint32 mode)
{
    if(CTC_VTI_SHELL_MODE_DEFAULT == mode)
    {
    
#ifdef SDK_IN_VXWORKS
        ioctl(ioTaskStdGet(0, STD_IN), FIOSETOPTIONS, termios_old);
#elif defined SDK_IN_USERMODE
        /*system("stty cooked echo");*/
        tcsetattr(0, TCSANOW, &termios_old);
#endif
        ctc_cli_out("\n");
    }
    else
    {
        /* write your code here */
    }
}

extern char*
ctc_cli_get_debug_desc(unsigned char level)
{
    static char debug_level_desc[64];

    sal_memset(debug_level_desc, 0, 63);
    if (level & 0x01)
    {
        sal_strcat(debug_level_desc, "Func/");
    }

    if (level & 0x02)
    {
        sal_strcat(debug_level_desc, "Param/");
    }

    if (level & 0x04)
    {
        sal_strcat(debug_level_desc, "Info/");
    }

    if (level & 0x08)
    {
        sal_strcat(debug_level_desc, "Error/");
    }

    if (sal_strlen(debug_level_desc) != 0)
    {
        debug_level_desc[sal_strlen(debug_level_desc) - 1] = '\0';
    }
    else
    {
        sal_strcpy(debug_level_desc, "None");
    }

    return debug_level_desc;
}

uint32
ctc_cmd_get_value(int32* ret, char* name, char* str, uint32 min, uint32 max, uint8 type)
{
    int32 retv = 0;
    uint32 tmp = 0;
    uint8 j = 0;
    char string[100] = {0};

    sal_memcpy(string, name, sal_strlen(name));
    tmp = ctc_cmd_str2uint((str), &retv);

    if(0 == type)   /*get uint8*/
    {
        if (retv < 0 || tmp > CTC_MAX_UINT8_VALUE || tmp < min || tmp > max)
        {
            ctc_cli_out("%% Invalid %s value\n", name);
            *ret = CLI_ERROR;
            return 0;
        }
    }
    else if(1 == type)  /*get uint16*/
    {
        if (retv < 0 || tmp > CTC_MAX_UINT16_VALUE || tmp < min || tmp > max)
        {
            ctc_cli_out("%% Invalid %s value\n", name);
            *ret = CLI_ERROR;
            return 0;
        }
    }
    else    /*get uint32*/
    {
        if (retv < 0 || tmp > CTC_MAX_UINT32_VALUE || tmp < min || tmp > max)
        {
            ctc_cli_out("%% Invalid %s value\n", name);
            *ret = CLI_ERROR;
            return 0;
        }
    }

    for (j = 0; j < sal_strlen(string); j++)
    {
        (string[j]) = sal_tolower((string[j]));
    }

    *ret = retv;
    return tmp;
}


