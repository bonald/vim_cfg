/*
 * Virtual terminal [aka TeletYpe] interface routine.
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "ctc_cmd.h"
#include "ctc_cli.h"
#ifdef SDK_IN_VXWORKS
#include "timers.h "
#else
#include "sal_timer.h"
#endif

/* Vty events */
enum event
{
    CTC_VTI_SERV,
    CTC_VTI_READ,
    CTC_VTI_WRITE,
    CTC_VTI_TIMEOUT_RESET,
#ifdef VTYSH
    VTYSH_SERV,
    VTYSH_READ
#endif /* VTYSH */
};

/* Vector which store each vti structure. */
static vector vtivec;

/* Vty timeout value. */
static unsigned long vti_timeout_val = CTC_VTI_TIMEOUT_DEFAULT;

/* Current directory. */
char* vti_cwd = NULL;

/* Configure lock. */
static int vti_config;

ctc_vti_t* g_ctc_vti = NULL;



ctc_vti_t*
ctc_vti_create(int mode);
char g_ctc_vti_out_buf[1024];

extern sal_file_t debug_fprintf_log_file;

extern int grep_cn;

int
ctc_vti_out(ctc_vti_t* vti, const char* format, ...)
{
    va_list args;
    int len = 0;
    int size = sizeof(g_ctc_vti_out_buf);
    char* p = NULL;

    va_start(args, format);

    /* Try to write to initial buffer.  */
#ifdef SDK_IN_VXWORKS
    len = vsprintf(g_ctc_vti_out_buf, format, args);
#else
    len = vsnprintf(g_ctc_vti_out_buf, sizeof(g_ctc_vti_out_buf), format, args);
#endif

    va_end(args);

    /* Initial buffer is not enough.  */
    if (len < 0 || len >= size)
    {
        while (1)
        {
            if (len > -1)
            {
                size = len + 1;
            }
            else
            {
                size = size * 2;
            }

            p = mem_realloc(MEM_LIBCTCCLI_MODULE, p, size);
            if (!p)
            {
                return -1;
            }

#ifdef SDK_IN_VXWORKS
            len = vsprintf(p, format, args);
#else
            len = vsnprintf(p, size, format, args);
#endif

            if (len > -1 && len < size)
            {
                break;
            }
        }
    }

    /* When initial buffer is enough to store all output.  */
    if (!p)
    {
        p = g_ctc_vti_out_buf;
    }

    /* Pointer p must point out buffer. */
    if (vti->output)
    {
        vti->output(p, len, NULL);
    }
    else if (vti->printf)
    {
        vti->printf(vti, p, len);
    }

    /* If p is not different with buf, it is allocated buffer.  */
    if (p != g_ctc_vti_out_buf)
    {
        mem_free(p);
    }

    return len;
}

/* Put out prompt and wait input from user. */
void
ctc_vti_prompt(ctc_vti_t* vti)
{
    ctc_vti_out(vti,"%s",ctc_cmd_prompt(vti->node));
}

/* Allocate new vti struct. */
ctc_vti_t*
ctc_vti_new()
{
    ctc_vti_t* new_vti = mem_malloc(MEM_LIBCTCCLI_MODULE, sizeof(ctc_vti_t));

    if (!new_vti)
    {
        return NULL;
    }
    sal_memset(new_vti, 0, sizeof(ctc_vti_t));

    new_vti->buf = mem_malloc(MEM_LIBCTCCLI_MODULE, CTC_VTI_BUFSIZ);
    if (!new_vti->buf)
    {
        mem_free(new_vti);
        return NULL;
    }
    sal_memset(new_vti->buf, 0, CTC_VTI_BUFSIZ);

    new_vti->max = CTC_VTI_BUFSIZ;

    return new_vti;
}

/* Command execution over the vti interface. */
int
ctc_vti_command(ctc_vti_t* vti, char* buf)
{
    int ret;
    vector vline;

    /* Split readline string up into the vector */
    vline = ctc_cmd_make_strvec(buf);

    if (vline == NULL)
    {
        return CMD_SUCCESS;
    }

    ret = ctc_cmd_execute_command(vline, vti, NULL);

    if (ret != CMD_SUCCESS)
    {
        switch (ret)
        {
        case CMD_WARNING: /* do nothing*/
            /*printf ("%% System warning...\n");
            */
            break;

        case CMD_ERR_AMBIGUOUS:
            ctc_vti_out(vti, "%% Ambiguous command\n\r");
            break;

        case CMD_ERR_NO_MATCH:
            ctc_vti_out(vti, "%% Unrecognized command\n\r");
            break;

        case CMD_ERR_INCOMPLETE:
            ctc_vti_out(vti, "%% Incomplete command\n\r");
            break;

        case CMD_SYS_ERROR:
            ctc_vti_out(vti, "%% System warning...\n\r");
            break;

        default:
            break;
        }
    }

    ctc_cmd_free_strvec(vline);

    return ret;
}

char ctc_telnet_backward_char = 0x08;
char ctc_telnet_space_char = ' ';

/* Basic function to write buffer to vti. */
void
ctc_vti_write(ctc_vti_t* vti, char* buf, uint32 nbytes)
{
    if (vti->output)
    {
        vti->output(buf, nbytes, NULL);
    }
    else if(vti->printf)
    {
        vti->printf(vti, buf, nbytes);
    }
}

/* Ensure length of input buffer.  Is buffer is short, double it. */
STATIC void
ctc_vti_ensure(ctc_vti_t* vti, int length)
{
    if (vti->max <= length)
    {
        vti->max *= 2;
        vti->buf = mem_realloc(MEM_LIBCTCCLI_MODULE, vti->buf, vti->max);
    }
}

/* Basic function to insert character into vti. */
STATIC void
ctc_vti_self_insert(ctc_vti_t* vti, char c)
{
    int length;

    ctc_vti_ensure(vti, vti->length + 1);
    length = vti->length - vti->cp;
    sal_memmove(&vti->buf[vti->cp + 1], &vti->buf[vti->cp], length);
    vti->buf[vti->cp] = c;

    /*ctc_vti_write (vti, &vti->buf[vti->cp], length + 1);
    for (i = 0; i < length; i++)
      ctc_vti_write (vti, &ctc_telnet_backward_char, 1);*/

    vti->cp++;
    vti->length++;
}

/* Self insert character 'c' in overwrite mode. */
STATIC void
ctc_vti_self_insert_overwrite(ctc_vti_t* vti, char c)
{
    ctc_vti_ensure(vti, vti->length + 1);
    vti->buf[vti->cp++] = c;

    if (vti->cp > vti->length)
    {
        vti->length++;
    }

/*
  if ((vti->node == AUTH_NODE) || (vti->node == AUTH_ENABLE_NODE))
    return;
*/
    ctc_vti_write(vti, &c, 1);
}

/* Insert a word into vti interface with overwrite mode. */
STATIC void
ctc_vti_insert_word_overwrite(ctc_vti_t* vti, char* str)
{
    int len = sal_strlen(str);

    ctc_vti_write(vti, str, len);
    sal_strcpy(&vti->buf[vti->cp], str);
    vti->cp += len;
    vti->length = vti->cp;
}

/* Forward character. */
STATIC void
ctc_vti_forward_char(ctc_vti_t* vti)
{
    if (vti->cp < vti->length)
    {
        ctc_vti_write(vti, &vti->buf[vti->cp], 1);
        vti->cp++;
    }
}

/* Backward character. */
STATIC void
ctc_vti_backward_char(ctc_vti_t* vti)
{
    if (vti->cp > 0)
    {
        vti->cp--;
        ctc_vti_write(vti, &ctc_telnet_backward_char, 1);
    }
}

/* Move to the beginning of the line. */
STATIC void
ctc_vti_beginning_of_line(ctc_vti_t* vti)
{
    while (vti->cp)
    {
        ctc_vti_backward_char(vti);
    }
}

/* Move to the end of the line. */
STATIC void
ctc_vti_end_of_line(ctc_vti_t* vti)
{
    while (vti->cp < vti->length)
    {
        ctc_vti_forward_char(vti);
    }
}

STATIC void ctc_vti_kill_line_from_beginning(ctc_vti_t*);
STATIC void ctc_vti_redraw_line(ctc_vti_t*);

/* Print command line history.  This function is called from
   ctc_vti_next_line and ctc_vti_previous_line. */
STATIC void
ctc_vti_history_print(ctc_vti_t* vti)
{
    int length;

    ctc_vti_kill_line_from_beginning(vti);

    if (vti->hist[vti->hp] != NULL)
    {
        /* Get previous line from history buffer */
        length = sal_strlen(vti->hist[vti->hp]);
        sal_memcpy(vti->buf, vti->hist[vti->hp], length);
        vti->cp = vti->length = length;

        /* Redraw current line */
        ctc_vti_redraw_line(vti);
    }
}

/* Show next command line history. */
void
ctc_vti_next_line(ctc_vti_t* vti)
{
    int try_index;
    int try_count = 0;

    if (vti->hp == vti->hindex)
    {
        ctc_vti_kill_line_from_beginning(vti);
        return;
    }

    /* Try is there history exist or not. */
    try_index = vti->hp;
    if (try_index == (CTC_VTI_MAXHIST - 1))
    {
        try_index = 0;
    }
    else
    {
        try_index++;
    }

    while ((vti->hist[try_index] == NULL) && (try_count < CTC_VTI_MAXHIST))
    {
        if (try_index == (CTC_VTI_MAXHIST - 1))
        {
            try_index = 0;
        }
        else
        {
            try_index++;
        }

        try_count++;
    }

    /* If there is not history return. */
    if (vti->hist[try_index] == NULL)
    {
        ctc_vti_kill_line_from_beginning(vti);
        return;
    }
    else
    {
        vti->hp = try_index;
    }

    ctc_vti_history_print(vti);
}

/* Show previous command line history. */
void
ctc_vti_previous_line(ctc_vti_t* vti)
{
    int try_index;
    int try_count = 0;

    try_index = vti->hp;

    if (try_index == 0)
    {
        try_index = CTC_VTI_MAXHIST - 1;
    }
    else
    {
        try_index--;
    }

    while (vti->hist[try_index] == NULL && try_count < CTC_VTI_MAXHIST)
    {
        if (try_index == 0)
        {
            try_index = CTC_VTI_MAXHIST - 1;
        }
        else
        {
            try_index--;
        }

        try_count++;
    }

    if (vti->hist[try_index] == NULL)
    {
        ctc_vti_kill_line_from_beginning(vti);
        return;
    }
    else
    {
        vti->hp = try_index;
    }

    ctc_vti_history_print(vti);
}

/* This function redraw all of the command line character. */
STATIC void
ctc_vti_redraw_line(ctc_vti_t* vti)
{
    ctc_vti_write(vti, vti->buf, vti->length);
    vti->cp = vti->length;
}

/* Forward word. */
void
ctc_vti_forward_word(ctc_vti_t* vti)
{
    while (vti->cp != vti->length && vti->buf[vti->cp] != ' ')
    {
        ctc_vti_forward_char(vti);
    }

    while (vti->cp != vti->length && vti->buf[vti->cp] == ' ')
    {
        ctc_vti_forward_char(vti);
    }
}

/* Backward word without skipping training space. */
void
ctc_vti_backward_pure_word(ctc_vti_t* vti)
{
    while (vti->cp > 0 && vti->buf[vti->cp - 1] != ' ')
    {
        ctc_vti_backward_char(vti);
    }
}

/* Backward word. */
void
ctc_vti_backward_word(ctc_vti_t* vti)
{
    while (vti->cp > 0 && vti->buf[vti->cp - 1] == ' ')
    {
        ctc_vti_backward_char(vti);
    }

    while (vti->cp > 0 && vti->buf[vti->cp - 1] != ' ')
    {
        ctc_vti_backward_char(vti);
    }
}

/* When '^D' is typed at the beginning of the line we move to the down
   level. */
STATIC void
ctc_vti_down_level(ctc_vti_t* vti)
{
    ctc_vti_out(vti, "%s", CTC_VTI_NEWLINE);
    ctc_vti_prompt(vti);
    vti->cp = 0;
}

/* When '^Z' is received from vti, move down to the enable mode. */
void
ctc_vti_end_config(ctc_vti_t* vti)
{
    ctc_vti_out(vti, "%s", CTC_VTI_NEWLINE);

    switch (vti->node)
    {
    case CTC_VIEW_NODE:
    case CTC_ENABLE_NODE:
        /* Nothing to do. */
        break;

    case CTC_CONFIG_NODE:
    case CTC_INTERFACE_NODE:
    case CTC_KEYCHAIN_NODE:
    case CTC_KEYCHAIN_KEY_NODE:
    case CTC_MASC_NODE:
    case CTC_VTI_NODE:
        ctc_vti_config_unlock(vti);
        vti->node = CTC_ENABLE_NODE;
        break;

    default:
        /* Unknown node, we have to ignore it. */
        break;
    }

    ctc_vti_prompt(vti);
    vti->cp = 0;
}

/* Delete a charcter at the current point. */
STATIC void
ctc_vti_delete_char(ctc_vti_t* vti)
{
    int i;
    int size;

    if (vti->length == 0)
    {
        ctc_vti_down_level(vti);
        return;
    }

    if (vti->cp == vti->length)
    {
        return; /* completion need here? */

    }

    size = vti->length - vti->cp;

    vti->length--;
    sal_memmove(&vti->buf[vti->cp], &vti->buf[vti->cp + 1], size - 1);
    vti->buf[vti->length] = '\0';

    ctc_vti_write(vti, &vti->buf[vti->cp], size - 1);
    ctc_vti_write(vti, &ctc_telnet_space_char, 1);

    for (i = 0; i < size; i++)
    {
        ctc_vti_write(vti, &ctc_telnet_backward_char, 1);
    }
}

/* Delete a character before the point. */
STATIC void
ctc_vti_delete_backward_char(ctc_vti_t* vti)
{
    if (vti->cp == 0)
    {
        return;
    }

    ctc_vti_backward_char(vti);
    ctc_vti_delete_char(vti);
}

/* Kill rest of line from current point. */
STATIC void
ctc_vti_kill_line(ctc_vti_t* vti)
{
    int i;
    int size;

    size = vti->length - vti->cp;

    if (size == 0)
    {
        return;
    }

    for (i = 0; i < size; i++)
    {
        ctc_vti_write(vti, &ctc_telnet_space_char, 1);
    }

    for (i = 0; i < size; i++)
    {
        ctc_vti_write(vti, &ctc_telnet_backward_char, 1);
    }

    sal_memset(&vti->buf[vti->cp], 0, size);
    vti->length = vti->cp;
}

/* Kill line from the beginning. */
STATIC void
ctc_vti_kill_line_from_beginning(ctc_vti_t* vti)
{
    ctc_vti_beginning_of_line(vti);
    ctc_vti_kill_line(vti);
}

/* Delete a word before the point. */
void
ctc_vti_forward_kill_word(ctc_vti_t* vti)
{
    while (vti->cp != vti->length && vti->buf[vti->cp] == ' ')
    {
        ctc_vti_delete_char(vti);
    }

    while (vti->cp != vti->length && vti->buf[vti->cp] != ' ')
    {
        ctc_vti_delete_char(vti);
    }
}

/* Delete a word before the point. */
STATIC void
ctc_vti_backward_kill_word(ctc_vti_t* vti)
{
    while (vti->cp > 0 && vti->buf[vti->cp - 1] == ' ')
    {
        ctc_vti_delete_backward_char(vti);
    }

    while (vti->cp > 0 && vti->buf[vti->cp - 1] != ' ')
    {
        ctc_vti_delete_backward_char(vti);
    }
}

void
ctc_vti_clear_buf(ctc_vti_t* vti)
{
    sal_memset(vti->buf, 0, vti->max);
}

/* Transpose chars before or at the point. */
STATIC void
ctc_vti_transpose_chars(ctc_vti_t* vti)
{
    char c1, c2;

    /* If length is short or point is near by the beginning of line then
       return. */
    if (vti->length < 2 || vti->cp < 1)
    {
        return;
    }

    /* In case of point is located at the end of the line. */
    if (vti->cp == vti->length)
    {
        c1 = vti->buf[vti->cp - 1];
        c2 = vti->buf[vti->cp - 2];

        ctc_vti_backward_char(vti);
        ctc_vti_backward_char(vti);
        ctc_vti_self_insert_overwrite(vti, c1);
        ctc_vti_self_insert_overwrite(vti, c2);
    }
    else
    {
        c1 = vti->buf[vti->cp];
        c2 = vti->buf[vti->cp - 1];

        ctc_vti_backward_char(vti);
        ctc_vti_self_insert_overwrite(vti, c1);
        ctc_vti_self_insert_overwrite(vti, c2);
    }
}

/* Do completion at vti interface. */
STATIC void
ctc_vti_complete_command(ctc_vti_t* vti)
{
    int i;
    int ret;
    char** matched = NULL;
    vector vline;
    char match_list[256] = {'\0'};

    vline = ctc_cmd_make_strvec(vti->buf);
    if (vline == NULL)
    {
        return;
    }

    /* In case of 'help \t'. */
    if (sal_isspace((int)vti->buf[vti->length - 1]))
    {
        ctc_vti_vec_set(vline, '\0');
    }

    matched = ctc_cmd_complete_command(vline, vti, &ret);
    if(!matched)
    {
        return;
    }

    ctc_cmd_free_strvec(vline);

    /*printf( "%s", CTC_VTI_NEWLINE);
    */
    switch (ret)
    {
    case CMD_ERR_AMBIGUOUS:

        break;

    case CMD_ERR_NO_MATCH:

        break;

    case CMD_COMPLETE_FULL_MATCH:
        ctc_vti_backward_pure_word(vti);
        ctc_vti_insert_word_overwrite(vti, matched[0]);
        ctc_vti_self_insert(vti, ' ');
        ctc_vti_out(vti, " ");
        mem_free(matched[0]);
        break;

    case CMD_COMPLETE_MATCH:
        ctc_vti_backward_pure_word(vti);
        ctc_vti_insert_word_overwrite(vti, matched[0]);
        mem_free(matched[0]);
        ctc_vti_vec_only_index_free(matched);
        return;
        break;

    case CMD_COMPLETE_LIST_MATCH:
        ctc_vti_out(vti, "\n\r");

        for (i = 0; matched[i] != NULL; i++)
        {
            if (i != 0 && ((i % 6) == 0))
            {
                /*printf( "%s", CTC_VTI_NEWLINE);
                */
                ctc_vti_out(vti, "\n\r");
            }

            /*printf( "%-10s ", matched[i]);
            */
            sal_sprintf(match_list, "%-18s ", matched[i]);
            ctc_vti_write(vti, match_list, sal_strlen(match_list));
            if (sal_strcmp(matched[i], "<cr>") != 0)
            {
                mem_free(matched[i]);
            }
        }

        /*printf( "%s", CTC_VTI_NEWLINE);
        */
        ctc_vti_out(vti, "\n\r");
        ctc_vti_prompt(vti);
        ctc_vti_redraw_line(vti);
        break;

    case CMD_ERR_NOTHING_TODO:
        /*ctc_vti_prompt (vti);
        ctc_vti_redraw_line (vti);
        */
        break;

    default:
        break;
    }

    if (matched)
    {
        ctc_vti_vec_only_index_free(matched);
    }
}

void
ctc_vti_describe_fold(ctc_vti_t* vti, int cmd_width, int desc_width, ctc_cmd_desc_t* desc)
{
    char* buf, * cmd, * p;
    int pos;

    cmd = desc->cmd[0] == '.' ? desc->cmd + 1 : desc->cmd;

    if (desc_width <= 0)
    {
        ctc_vti_out(vti, "  %-*s  %s%s", cmd_width, cmd, desc->str, CTC_VTI_NEWLINE);
        return;
    }

    buf = mem_malloc(MEM_LIBCTCCLI_MODULE, sal_strlen(desc->str) + 1);
    if (!buf)
    {
        return;
    }
    sal_memset(buf, 0, sal_strlen(desc->str) + 1);

    for (p = desc->str; sal_strlen(p) > desc_width; p += pos + 1)
    {
        for (pos = desc_width; pos > 0; pos--)
        {
            if (*(p + pos) == ' ')
            {
                break;
            }
        }

        if (pos == 0)
        {
            break;
        }

        sal_strncpy(buf, p, pos);
        buf[pos] = '\0';
        ctc_vti_out(vti, "  %-*s  %s%s", cmd_width, cmd, buf, CTC_VTI_NEWLINE);

        cmd = "";
    }

    ctc_vti_out(vti, "  %-*s  %s%s", cmd_width, cmd, p, CTC_VTI_NEWLINE);

    mem_free(buf);
}

/* Describe matched command function. */
STATIC void
ctc_vti_describe_command(ctc_vti_t* vti)
{
    int ret;
    vector vline;
    vector describe;
    int i, width, desc_width;
    ctc_cmd_desc_t* desc, * desc_cr = NULL;

    vline = ctc_cmd_make_strvec(vti->buf);

    /* In case of '> ?'. */
    if (vline == NULL)
    {
        vline = ctc_vti_vec_init(1);
        if(vline == NULL)
        {
            return;
        }
        ctc_vti_vec_set(vline, '\0');
    }
    else if (sal_isspace((int)vti->buf[vti->length - 1]))
    {
        ctc_vti_vec_set(vline, '\0');
    }

    describe = ctc_cmd_describe_command(vline, vti, &ret);

    ctc_vti_out(vti, "%s", CTC_VTI_NEWLINE);

    /* Ambiguous error. */
    switch (ret)
    {
    case CMD_ERR_AMBIGUOUS:
        ctc_cmd_free_strvec(vline);
        ctc_vti_out(vti, "%% Ambiguous command%s", CTC_VTI_NEWLINE);
        ctc_vti_prompt(vti);
        ctc_vti_redraw_line(vti);
        return;
        break;

    case CMD_ERR_NO_MATCH:
        ctc_cmd_free_strvec(vline);
        ctc_vti_out(vti, "%% Unrecognized command%s", CTC_VTI_NEWLINE);
        ctc_vti_prompt(vti);
        ctc_vti_redraw_line(vti);
        return;
        break;
    }

    /* Get width of command string. */
    width = 0;

    for (i = 0; i < vector_max(describe); i++)
    {
        if ((desc = vector_slot(describe, i)) != NULL)
        {
            int len;

            if (desc->cmd[0] == '\0')
            {
                continue;
            }

            len = sal_strlen(desc->cmd);
            if (desc->cmd[0] == '.')
            {
                len--;
            }

            if (width < len)
            {
                width = len;
            }
        }
    }

    /* Get width of description string. */
    desc_width = vti->width - (width + 6);

    /* Print out description. */
    for (i = 0; i < vector_max(describe); i++)
    {
        if ((desc = vector_slot(describe, i)) != NULL)
        {
            if (desc->cmd[0] == '\0')
            {
                continue;
            }

            if (sal_strcmp(desc->cmd, "<cr>") == 0)
            {
                desc_cr = desc;
                continue;
            }

            if (!desc->str)
            {
                ctc_vti_out(vti, "  %-s%s",
                            desc->cmd[0] == '.' ? desc->cmd + 1 : desc->cmd,
                            CTC_VTI_NEWLINE);
            }
            else if (desc_width >= sal_strlen(desc->str))
            {
                ctc_vti_out(vti, "  %-*s  %s%s", width,
                            desc->cmd[0] == '.' ? desc->cmd + 1 : desc->cmd,
                            desc->str, CTC_VTI_NEWLINE);
            }
            else
            {
                ctc_vti_describe_fold(vti, width, desc_width, desc);
            }
        }
    }

    if ((desc = desc_cr))
    {
        if (!desc->str)
        {
            ctc_vti_out(vti, "  %-s%s",
                        desc->cmd[0] == '.' ? desc->cmd + 1 : desc->cmd,
                        CTC_VTI_NEWLINE);
        }
        else if (desc_width >= sal_strlen(desc->str))
        {
            ctc_vti_out(vti, "  %-*s  %s%s", width,
                        desc->cmd[0] == '.' ? desc->cmd + 1 : desc->cmd,
                        desc->str, CTC_VTI_NEWLINE);
        }
        else
        {
            ctc_vti_describe_fold(vti, width, desc_width, desc);
        }
    }

    ctc_cmd_free_strvec(vline);
    ctc_vti_vec_free(describe);

    ctc_vti_prompt(vti);
    ctc_vti_redraw_line(vti);
}

/* ^C stop current input and do not add command line to the history. */
STATIC void
ctc_vti_stop_input(ctc_vti_t* vti)
{
    vti->cp = vti->length = 0;
    ctc_vti_clear_buf(vti);
    ctc_vti_out(vti, "%s", CTC_VTI_NEWLINE);

    switch (vti->node)
    {
    case CTC_VIEW_NODE:
    case CTC_ENABLE_NODE:
        /* Nothing to do. */
        break;

    case CTC_CONFIG_NODE:
    case CTC_INTERFACE_NODE:
    case CTC_KEYCHAIN_NODE:
    case CTC_KEYCHAIN_KEY_NODE:
    case CTC_MASC_NODE:
    case CTC_VTI_NODE:
        ctc_vti_config_unlock(vti);
        vti->node = CTC_ENABLE_NODE;
        break;

    default:
        /* Unknown node, we have to ignore it. */
        break;
    }

    ctc_vti_prompt(vti);

    /* Set history pointer to the latest one. */
    vti->hp = vti->hindex;
}

void
ctc_vti_append_history_command(char* cmd)
{
#ifdef ISGCOV
    sal_file_t p_history_file = NULL;

    p_history_file = sal_fopen("history", "a+");
    if (!p_history_file)
    {
        return;
    }

    sal_fprintf(p_history_file, "%s\n", cmd);

    sal_fclose(p_history_file);
    p_history_file = NULL;
#else

#endif
    return ;
}


/* Add current command line to the history buffer. */
STATIC void
ctc_vti_hist_add(ctc_vti_t* vti)
{
    int index;

    if (vti->length == 0)
    {
        return;
    }

    index = vti->hindex ? vti->hindex - 1 : CTC_VTI_MAXHIST - 1;

    ctc_vti_append_history_command(vti->buf);

    /* Ignore the same string as previous one. */
    if (vti->hist[index])
    {
        if (sal_strcmp(vti->buf, vti->hist[index]) == 0)
        {
            vti->hp = vti->hindex;
            return;
        }
    }

    /* Insert history entry. */
    if (vti->hist[vti->hindex])
    {
        mem_free(vti->hist[vti->hindex]);
    }

    vti->hist[vti->hindex] = XSTRDUP(MTYPE_VTI_HIST, vti->buf);

    /* History index rotation. */
    vti->hindex++;
    if (vti->hindex == CTC_VTI_MAXHIST)
    {
        vti->hindex = 0;
    }

    vti->hp = vti->hindex;
}

/* Execute current command line. */
STATIC int
ctc_vti_execute(ctc_vti_t* vti)
{
    int ret;
    int ret1;

    ret = CMD_SUCCESS;
    ret1 = CMD_SUCCESS;

    ctc_vti_out(vti, "\n\r");
    vti->for_en = 0;
    vti->cmd_usec = 0;
    grep_cn = 0;
    ctc_cmd_grep_command(vti);
    ret1 = ctc_cmd_for_command(vti);
    if (ret1 != 0)
    {
        ret = ctc_vti_command(vti, vti->buf);
    }
    if(vti->fprintf == TRUE)
    {
        sal_fclose(debug_fprintf_log_file);
        vti->fprintf = FALSE;
    
        if(vti->sort_lineth > 0)
        {
            ctc_cmd_sort(vti);
        }
        ctc_cmd_grep(vti);       
    }
    ctc_vti_hist_add(vti);
    vti->cp = vti->length = 0;
    ctc_vti_clear_buf(vti);

    ctc_vti_prompt(vti);

    return ret;
}

#define CONTROL(X)  ((X) - '@')
#define VTI_NORMAL     0
#define VTI_PRE_ESCAPE 1
#define VTI_ESCAPE     2

/* Escape character command map. */
void
ctc_vti_escape_map(unsigned char c, ctc_vti_t* vti)
{
    switch (c)
    {
    case ('A'):
        ctc_vti_previous_line(vti);
        break;

    case ('B'):
        ctc_vti_next_line(vti);
        break;

    case ('C'):
        ctc_vti_forward_char(vti);
        break;

    case ('D'):
        ctc_vti_backward_char(vti);
        break;

    default:
        break;
    }

    /* Go back to normal mode. */
    vti->escape = CTC_VTI_NORMAL;
}

STATIC int
is_char_visible(unsigned char c)
{
    if (c >= 32 && c < 127)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static char g_ctc_vti_read_buf[CTC_VTI_READ_BUFSIZ] = {0};
static char g_vti_out_buf[CTC_VTI_READ_BUFSIZ] = {0};
int
ctc_vti_read_cmd(ctc_vti_t* vty, const int8* szbuf, const uint32 buf_len)
{
    int i = 0;
    int nbytes;

    sal_memcpy(g_ctc_vti_read_buf,szbuf,buf_len < CTC_VTI_READ_BUFSIZ ? buf_len : CTC_VTI_READ_BUFSIZ);
    nbytes = buf_len < CTC_VTI_READ_BUFSIZ ? buf_len : CTC_VTI_READ_BUFSIZ;
    /*read(ioTaskStdGet(0,0), &buf[i], 1) *//*for vxworks*/
    for (i = 0; i < nbytes; i++)
    {
#ifdef SDK_IN_VXWORKS
        /*delay_a_while();*/
#else
        /*usleep(1);*/
        /*1 ms */
#endif

        if (is_char_visible(g_ctc_vti_read_buf[i]) && (vty->escape == VTI_NORMAL))
        {
            uint8   out_buf_len = 0;
            sal_memset(g_vti_out_buf,'\0',sizeof(g_vti_out_buf));
            g_vti_out_buf[0] = g_ctc_vti_read_buf[i];
            out_buf_len++;
            if (vty->cp != vty->length)
            {
                uint8 dist = 0;
                uint8 j = 0;

                /*move forward*/
                dist = vty->length - vty->cp;
                sal_memcpy(&g_vti_out_buf[1],&vty->buf[vty->cp], dist);
                out_buf_len += dist;
                /*move backward*/
                for (j=0; j< dist; j++)
                {
                    g_vti_out_buf[1 + dist + j] = ctc_telnet_backward_char;
                }
                out_buf_len += dist;
            }
            if (vty->output)
            {
                vty->output(g_vti_out_buf, out_buf_len, NULL);
            }
            else
            {
                vty->printf(vty, g_vti_out_buf, out_buf_len);
            }

            /*sal_memcpy(&g_ctc_vti->buf[g_ctc_vti->cp], &buf[i], 1); */
        }

        /* Escape character. */
        if (vty->escape == VTI_ESCAPE)
        {
            ctc_vti_escape_map (g_ctc_vti_read_buf[i], vty);
            continue;
        }

        /* Pre-escape status. */
        if (vty->escape == VTI_PRE_ESCAPE)
        {
            switch (g_ctc_vti_read_buf[i])
            {
                case '[':
                    vty->escape = VTI_ESCAPE;
                    break;
                case 'b':
                    ctc_vti_backward_word (vty);
                    vty->escape = VTI_NORMAL;
                    break;
                case 'f':
                    ctc_vti_forward_word (vty);
                    vty->escape = VTI_NORMAL;
                    break;
                case 'd':
                    ctc_vti_forward_kill_word (vty);
                    vty->escape = VTI_NORMAL;
                    break;
                case CONTROL('H'):
                case 0x7f:
                    ctc_vti_backward_kill_word (vty);
                    vty->escape = VTI_NORMAL;
                    break;
                default:
                    vty->escape = VTI_NORMAL;
                    break;
            }
            continue;

        }

        switch (g_ctc_vti_read_buf[i])
        {
            case CONTROL('A'):
                ctc_vti_beginning_of_line(vty);
                break;

            case CONTROL('B'):
                ctc_vti_backward_char(vty);
                break;

            case CONTROL('C'):
                ctc_vti_stop_input(vty);
                break;

            case CONTROL('D'):
                ctc_vti_delete_char(vty);
                break;

            case CONTROL('E'):
                ctc_vti_end_of_line(vty);
                break;

            case CONTROL('F'):
                ctc_vti_forward_char(vty);
                break;

            case CONTROL('H'):
            case 0x7f:
                ctc_vti_delete_backward_char(vty);
                break;
            case 'C':
                if(grep_cn)
                {
                    ctc_cli_out("\n");
                    ctc_cmd_grep(vty);
                }
                else
                {
                    ctc_vti_self_insert(vty, g_ctc_vti_read_buf[i]);
                }
                break;

            case CONTROL('K'):
                ctc_vti_kill_line(vty);
                break;

            case CONTROL('N'):
                ctc_vti_next_line(vty);
                break;

            case CONTROL('P'):
                ctc_vti_previous_line(vty);
                break;

            case CONTROL('T'):
                ctc_vti_transpose_chars(vty);
                break;

            case CONTROL('U'):
                ctc_vti_kill_line_from_beginning(vty);
                break;

            case CONTROL('W'):
                ctc_vti_backward_kill_word(vty);
                break;

            case CONTROL('Z'):
                ctc_vti_end_config(vty);
                break;

            case '\n':
            case '\r':
                ctc_vti_execute(vty);
                break;

            case '\t':
                ctc_vti_complete_command(vty);
                break;

            case '?':
                ctc_vti_describe_command(vty);
                break;

            case '\033':
                if (i + 1 < nbytes && g_ctc_vti_read_buf[i + 1] == '[')
                {
                    vty->escape = VTI_ESCAPE;
                    i++;
                }
                else
                {
                    vty->escape = VTI_PRE_ESCAPE;
                }
                break;

                break;


            default:
                if (g_ctc_vti_read_buf[i] > 31 && g_ctc_vti_read_buf[i] < 127)
                {
                    ctc_vti_self_insert(vty, g_ctc_vti_read_buf[i]);
                }

                break;
        }
    }

    return 0;
}


int32 ctc_vti_cmd_input(const int8* buf, const uint32 buf_len)
{
    if (g_ctc_vti)/*init g_ctc_vti at ctc_vti_init()*/
    {
        ctc_vti_read_cmd(g_ctc_vti, buf, buf_len);
    }
    else
    {
        return -1;
    }
    return 0;
}

int32 ctc_vti_cmd_output_register(CTC_CB_VTI_CMD_OUT_FUN_P print)
{
    if (g_ctc_vti)/*init g_ctc_vti at ctc_vti_init()*/
    {
        g_ctc_vti->output = print;
    }
    else
    {
        return -1;
    }
    return 0;
}

/* Read data via vti socket. */
int
ctc_vti_read(int8* buf, uint32 buf_size,uint32 mode)
{
    int nbytes = 0;

    if (CTC_VTI_SHELL_MODE_DEFAULT == mode)
    {
        nbytes = sal_read(0, buf, buf_size);
    }
    else
    {
        /* write your code here */
    }

    return nbytes;
}

/* Create new g_ctc_vti structure. */
ctc_vti_t*
ctc_vti_create(int mode)
{
    ctc_vti_t* vti;

    /* Allocate new vti structure and set up default values. */
    vti = ctc_vti_new();
    if (!vti)
    {
        return NULL;
    }

    vti->fd = 0;
    vti->type = CTC_VTI_TERM;
    vti->address = "";

    vti->node = mode;

    vti->fail = 0;
    vti->cp = 0;
    ctc_vti_clear_buf(vti);
    vti->length = 0;
    sal_memset(vti->hist, 0, sizeof(vti->hist));
    vti->hp = 0;
    vti->hindex = 0;
    ctc_vti_vec_set_index(vtivec, 0, vti);
    vti->status = VTI_NORMAL;
    vti->v_timeout = vti_timeout_val;

    vti->lines = -1;
    vti->iac = 0;
    vti->iac_sb_in_progress = 0;
    vti->width = 0;

    ctc_vti_prompt(vti);

    return vti;
}

int
ctc_vti_config_lock(ctc_vti_t* vti)
{
    if (vti_config == 0)
    {
        vti->config = 1;
        vti_config = 1;
    }

    return vti->config;
}

int
ctc_vti_config_unlock(ctc_vti_t* vti)
{
    if (vti_config == 1 && vti->config == 1)
    {
        vti->config = 0;
        vti_config = 0;
    }

    return vti->config;
}

char*
ctc_vti_get_cwd()
{
    return vti_cwd;
}

int
ctc_vti_shell(ctc_vti_t* vti)
{
    return vti->type == CTC_VTI_SHELL ? 1 : 0;
}

int
ctc_vti_shell_serv(ctc_vti_t* vti)
{
    return vti->type == CTC_VTI_SHELL_SERV ? 1 : 0;
}

void
ctc_vti_init_vtish()
{
    vtivec = ctc_vti_vec_init(VECTOR_MIN_SIZE);
}

/* Install vti's own commands like `who' command. */
void
ctc_vti_init(int mode)
{
    /* For further configuration read, preserve current directory. */
    vtivec = ctc_vti_vec_init(VECTOR_MIN_SIZE);

    /* Initilize server thread vector. */
    /*Vctc_vti_serv_thread = ctc_vti_vec_init (VECTOR_MIN_SIZE);*/

    if (!g_ctc_vti)
    {
        g_ctc_vti = ctc_vti_create(mode);
    }
}

