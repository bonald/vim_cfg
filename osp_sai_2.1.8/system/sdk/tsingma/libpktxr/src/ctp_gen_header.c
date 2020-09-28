#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

static FILE *dest0, *dest1, *src, *src1;
#define is_valid_line(line)     (NULL != strstr(line, "$"))
#define is_a_letter(ch)        ((toupper(ch)>='A')&&(toupper(ch)<='Z'))
#define ignore_black(buff)    while(!is_a_letter(buff[0])){buff++;};
#define up_to_lower(ch)    (ch) = ((ch) + 'a' -'A')

#define MTYPE_TMP   0
#define XSTRDUP(a, b)       strdup((b))
#define XFREE(a, b)         free((b))
#define XMALLOC(a, b)       malloc((b))
#define XCALLOC(a, b)       calloc(1, (b))

int free_tokens(char ***pppTokArr)
{
    char **ppTokArr = NULL;
    int i = 0;

    if (NULL == pppTokArr || NULL == *pppTokArr)
    {
        return -1;
    }
    ppTokArr = *pppTokArr;

    i = 0;
    while (1)
    {
        if (NULL == ppTokArr[i])
        {
            break;
        }
        
        XFREE(MTYPE_TMP, ppTokArr[i]);
        i++;
    }

    XFREE(MTYPE_TMP, ppTokArr);
    return 0;
}

int 
split_tokens(char *pszBuf, size_t nLen, unsigned long nMaxToks, 
             char *pszDelimiters, unsigned long *pnToks, char ***pppTokArr)
{
    char *pTmp = NULL;
    char *pTok = NULL;
    char **ppTokArr = NULL;
    char *pSavePtr = NULL;
    unsigned long nArrSize = nMaxToks + 1;
    unsigned long nCurTok = 0;

    /* validate params */
    if (NULL == pszBuf || nLen <= 0 
        || nMaxToks <= 0 || nMaxToks > 100
        || NULL == pszDelimiters || NULL == pnToks
        || NULL == pppTokArr)
    {
        return -1;
    }
                  
    /* getting string copy */
    pTmp = XSTRDUP(MTYPE_TMP, pszBuf);
    if (NULL == pTmp)
    {
        errno = ENOMEM;
        return -1;
    }
    
    /* calloc token array */
    ppTokArr = XCALLOC(MTYPE_TMP, sizeof(char *) * nArrSize);
    if (NULL == ppTokArr)
    {
        errno = ENOMEM;
        goto error_out;
    }
    
    pTok = strtok_r(pTmp, pszDelimiters, &pSavePtr);
    while (NULL != pTok)
    {
        nCurTok++;
        if (nCurTok > nMaxToks)
        {
            errno = EINVAL;
            goto error_out;
        }

        /* save current token */
        ppTokArr[nCurTok - 1] = XSTRDUP(MTYPE_TMP, pTok);
        if (NULL == ppTokArr[nCurTok - 1])
        {
            errno = ENOMEM;
            goto error_out;
        }

        /* process next token */
        pTok = strtok_r(NULL, pszDelimiters, &pSavePtr);
    }

    /* success */
    *pnToks = nCurTok;
    *pppTokArr = ppTokArr;
                                                                                
    /* free memory */
    if (NULL != pTmp)
    {
        XFREE(MTYPE_TMP, pTmp);
    }
    return 0;

error_out:
    if (NULL != pTmp)
    {
        XFREE(MTYPE_TMP, pTmp);
    }

    free_tokens(&ppTokArr);
    return -1;
}


static int
open_file()
{
    dest0 = fopen("ctp_header_format.h", "w+");
    if (NULL == dest0)
    {
        printf("fopen ctp_header_format.h failed!\n");
        return -1;
    }

    dest1 = fopen("ctp_header_format.c", "w+");
    if (NULL == dest1)
    {
        printf("fopen ctp_header_format.c failed!\n");
        return -1;
    }

    src = fopen("header_format.txt", "r");
    if (NULL == src)
    {
        printf("fopen header_format.txt failed!\n");
        return -1;
    }

    src1 = fopen("header_flags.txt", "r");
    if (NULL == src1)
    {
        printf("fopen header_flags.txt failed!\n");
        return -1;
    }

    return 0;
}


static void
close_file(void)
{  
    fputs("\n\n#endif\n\n", dest0);
    fclose(dest0);

    fputs("\n/****************************************************************************", dest1);
    fputs("\n *", dest1);
    fputs("\n * Functions", dest1);
    fputs("\n *", dest1);
    fputs("\n ****************************************************************************/", dest1);
    fputs("\n", dest1);
    fputs("\n/*******************************************************************************", dest1);
    fputs("\n * Name   : ctc_pktxr_get_header_type_string", dest1);
    fputs("\n * Purpose: Get the pointer of stored header type string.", dest1);
    fputs("\n * Input  : header type enum.", dest1);
    fputs("\n * Output : None.", dest1);
    fputs("\n * Return : stored header type string.", dest1);
    fputs("\n * Note   : N/A", dest1);
    fputs("\n*******************************************************************************/", dest1);
    fputs("\nchar *\nctc_pktxr_get_header_type_string(uint16 header_type)", dest1);
    fputs("\n{", dest1);
    fputs("\n    return header_type_str[header_type];", dest1);
    fputs("\n}", dest1);
    fputs("\n", dest1);
    fputs("\n/*******************************************************************************", dest1);
    fputs("\n * Name   : ctc_pktxr_get_header_start_field", dest1);
    fputs("\n * Purpose: Get the first field of header type.", dest1);
    fputs("\n * Input  : header type enum.", dest1);
    fputs("\n * Output : None.", dest1);
    fputs("\n * Return : stored field ID.", dest1);
    fputs("\n * Note   : N/A", dest1);
    fputs("\n*******************************************************************************/", dest1);
    fputs("\nuint16 \nctc_pktxr_get_header_start_field(uint16 header_type)", dest1);
    fputs("\n{", dest1);
    fputs("\n    return header_start_field[header_type];", dest1);
    fputs("\n}", dest1);
    fputs("\n", dest1);
    fputs("\n/*******************************************************************************", dest1);
    fputs("\n * Name   : ctc_pktxr_get_field_info_pointer", dest1);
    fputs("\n * Purpose: Get the pointer of stored.", dest1);
    fputs("\n * Input  : None.", dest1);
    fputs("\n * Output : None.", dest1);
    fputs("\n * Return : stored header type string's pointer.", dest1);
    fputs("\n * Note   : N/A", dest1);
    fputs("\n*******************************************************************************/", dest1);
    fputs("\nctp_field_info_t *\nctc_pktxr_get_field_info_pointer(uint16 field_id)", dest1);
    fputs("\n{", dest1);
    fputs("\n    uint16 max_count = sizeof(field_info) / sizeof(ctp_field_info_t);", dest1);
    fputs("\n", dest1);
    fputs("\n    if (field_id >= max_count) {", dest1);
    fputs("\n        assert(0);", dest1);
    fputs("\n        return NULL;", dest1);
    fputs("\n    }", dest1);
    fputs("\n    return &field_info[field_id];", dest1);
    fputs("\n}\n", dest1);

    fputs("\n\n/* return 0 for failed", dest1);
    fputs("\n * return 1 for success", dest1);
    fputs("\n */", dest1);
    fputs("\nctp_field_info_t* ctc_pktxr_find_field_by_name(char *pszName)", dest1);
    fputs("\n{", dest1);
    fputs("\n   uint16 max_count = sizeof(field_info) / sizeof(ctp_field_info_t);", dest1);
    fputs("\n   int i;", dest1);
    fputs("\n", dest1);
    fputs("\n   for (i = 0; i < max_count; i++) {", dest1);
    fputs("\n       if (!strcasecmp(field_info[i].descriptor, pszName)) {", dest1);
    fputs("\n           return &field_info[i];", dest1);
    fputs("\n       }", dest1);
    fputs("\n   }\n", dest1);
    fputs("\n   return NULL;", dest1);
    fputs("\n}\n", dest1);

    fputs("\nvoid ctc_pktxr_output_all_fields(int fd)", dest1);
    fputs("\n{", dest1);
    fputs("\n   uint16 max_count = sizeof(field_info) / sizeof(ctp_field_info_t);", dest1);
    fputs("\n   int i;", dest1);
    fputs("\n", dest1);
    fputs("\n   for (i = 0; i < max_count; i++) {", dest1);
    fputs("\n       write(fd, field_info[i].descriptor, strlen(field_info[i].descriptor));", dest1);
    fputs("\n       write(fd, \"\\r\\n\", 2);", dest1);    
    fputs("\n   }\n", dest1);
    fputs("\n   return;", dest1);
    fputs("\n}\n", dest1);
      
    fclose(dest1);
    fclose(src);
    fclose(src1);
    return;
}

static void
valid_handle(char *line, int type)
{
#define M_STR_BUFSIZE   256 
    char *pch0, *pch1, str[M_STR_BUFSIZE], str1[M_STR_BUFSIZE];
    int  len;
    char **ppTokArr = NULL;
    unsigned long nToks = 0;
#define M_MAX_TOKENS        32    
    
    if (line[strlen(line) - 1] == '\n')
    {
        line[strlen(line) - 1] = '\0';
    }
    if (line[strlen(line) - 1] == '\r')
    {
        line[strlen(line) - 1] = '\0';
    }
    if (strlen(line) <= 0)
    {
        return;
    }
    if (split_tokens(line, strlen(line), M_MAX_TOKENS,
                     " ", &nToks, &ppTokArr) != 0)
    {
        printf("WARNING: Out of memory!!!\n");
        exit(1);
    }
    
    if(1 == type)
    {
        if(strncasecmp(ppTokArr[0], "$TYPES", 6)) 
        {
            goto err_out;
        }

        if (nToks != 2)
        {
            printf("line error: '%s'\n", line);
            goto err_out;
        }
        
        sprintf(str1, "\n    %s,", ppTokArr[1]);
        fputs(str1, dest0);

        sprintf(str1, "\n    \"%s\",", ppTokArr[1]);
        fputs(str1, dest1);
    }

    if(2 == type)
    {
        static int state = 0;

        if(0 == state)
        {
            if(!strcasecmp(ppTokArr[0], "$TYPES"))
            {
                state = 1;
            }
            goto err_out;
        }
        
        if (1 == state)
        {
            if (!strcasecmp(ppTokArr[0], "$FIELD"))
            {
                if (nToks < 4)
                {
                    printf("line error: '%s'\n", line);
                    goto err_out;
                }
                sprintf(str1, "\n    %s,", ppTokArr[1]);
                fputs(str1, dest1);
                state = 0;
            }
            goto err_out;
        }
    }

    if (3 == type)
    {
        static int bit = 0, next, flags;
        static int first   = 1;

        if (first)
        {
            if(NULL != strstr(line, "$EXIST"))
            {
                 bit = 1;  
            }
            else if (NULL != strstr(line, "$JUMP"))
            {
                 bit = 2;  
            }
            else if (NULL != strstr(line, "$NEXTP"))
            {
                 bit = 3;  
            }
            else if (NULL != strstr(line, "$LENGTH"))
            {
                 bit = 5;  
            }
            else if (NULL != strstr(line, "$REPEAT"))
            {
                 bit = 4;  
            }
            else
            {
                goto err_out;
            }
            /* get belongs field ID and store it to str */
            pch0 = strstr(line, "$") + 7;
            ignore_black(pch0);
            pch1 = strstr(pch0, "$");
            if(NULL == pch1)
            {
                goto err_out;
            }
            memset(str, 0, M_STR_BUFSIZE);
            len = (int) (pch1 - pch0);
            for(next =0; next < len; next ++) {str[next] = is_a_letter(pch0[next])? (pch0[next] -'A' + 'a'):pch0[next];}
            if(1 == bit)
            {
                sprintf(str1, "\nstatic ctp_flags_exist_t %s_exist[] =\n{", str);
            }
            else if (2 == bit)
            {
                sprintf(str1, "\nstatic ctp_flags_jump_t %s_jump[] =\n{", str);
            }
            else if (3 == bit)
            {
                sprintf(str1, "\nstatic ctp_flags_nextp_t %s_nextp[] =\n{", str);
            }
            else if (4 == bit)
            {
                sprintf(str1, "\nstatic ctp_flags_repeat_t %s_repeat[] =\n{", str);
            }
            else
            {
                sprintf(str1, "\nstatic ctp_flags_len_t %s_len[] =\n{", str);
            }
            fputs(str1, dest1);

            pch0 = pch1 + 1;
        }
        else
        {
            pch0 = line;
        }
        
        if (2 != sscanf(pch0, "%d%x", &next, &flags))
        {
                goto err_out;
        }

        
        pch1 = strstr(pch0, "$");
        if(NULL == pch1)
        {
                goto err_out;
        }
        pch0 = strstr(pch1 + 1, "$");
        memset(str, 0, M_STR_BUFSIZE);
        len = (int) (pch0 - pch1 -1);
        memcpy(str, pch1 + 1, len);
        memset(str1, 0, M_STR_BUFSIZE);
        pch0 = strstr(pch1, "[");
        memcpy(str1, pch0, strlen(pch0));
        fprintf(dest1, "\n    { %d, %d, {%s}, \"%s\" },", next, flags, str, str1);
        if(next)
        {
            first   = 0;
        }
        else
        {
            first   = 1;
            fprintf(dest1, "\n};\n");
        }
    }
    
    if(4 == type)
    {
        int idx = 0;
        int b = 0, e = 0, j = 0, d = 0, n = 0, l = 0, r = 0, dv = 0, u = 0;
        char output[M_STR_BUFSIZE];
        
        if(strcasecmp(ppTokArr[0], "$FIELD"))
        {
            goto err_out;
        }
        if (nToks < 4)
        {
            printf("line err: '%s'\n", line);
            goto err_out;
        }

        for (idx = 4; idx < nToks; idx++)
        {
            if (!strcasecmp(ppTokArr[idx], "default")) {
                d = 1;
            } else if (!strcasecmp(ppTokArr[idx], "jump")) {
                j = 1;
            } else if (!strcasecmp(ppTokArr[idx], "exist")) {
                e = 1;
            } else if (!strcasecmp(ppTokArr[idx], "repeat")) {
                r = 1;
            } else if (!strcasecmp(ppTokArr[idx], "udt")) {
                u = 1;
            } else if (!strcasecmp(ppTokArr[idx], "nextp")) {
                n = 1;
            } else if (!strcasecmp(ppTokArr[idx], "length")) {
                l = 1;
            }
        }
        
        strcpy(str, ppTokArr[1]);
        len = strlen(str);
        memset(str1, 0, M_STR_BUFSIZE);
        for(b =0; b < len; b ++) {
            str1[b] = is_a_letter(str[b])? (str[b] -'A' + 'a') : str[b];
        }
        sscanf(ppTokArr[2], "%d", &b);
        sscanf(ppTokArr[3], "%x", &dv);
        fprintf(dest0, "\n    %s,", str);
        sprintf(output, "\n    {\"%s\", %d, 0x%x", str, b, 
            (e | (j <<1) |(d << 2) |(n << 3) | (r << 4) | (l << 5)| (u << 6)));
        
        if (e) sprintf(str, ", %s_exist", str1);
        else  sprintf(str, ", NULL");
        strcat(output, str);
        
        if (j) sprintf(str, ", %s_jump", str1);
        else  sprintf(str, ", NULL");
        strcat(output, str);
        
        if (n) sprintf(str, ", %s_nextp", str1);
        else  sprintf(str, ", NULL");
        strcat(output, str);
        
        if (r) sprintf(str, ", %s_repeat", str1);
        else  sprintf(str, ", NULL");
        strcat(output, str);
        
        if (l) sprintf(str, ", %s_len", str1);
        else  sprintf(str, ", NULL");        
        strcat(output, str);
        
        sprintf(str, ", \"%x\"},", dv);
        strcat(output, str);
        fprintf(dest1, output);
    }
err_out:
    if (ppTokArr)
    {
        free_tokens(&ppTokArr);
    }
    return;
}

static void 
comment_string_output()
{
    fputs("\n/****************************************************************************",dest0);
    fputs("\n * ctp_header_format.h Provides all CTP Supportted header protocol enumeration.",dest0);
    fputs("\n *",dest0);
    fputs("\n * Copyright: (c)2007 Centec Networks Inc.  All rights reserved.",dest0);
    fputs("\n *",dest0);
    fputs("\n * This file is automatic generate by CTP_GEN_HEADER tools and use a script",dest0);
    fputs("\n * input file named ctp_header_format.txt",dest0);
    fputs("\n *",dest0);
    fputs("\n*****************************************************************************/",dest0);
    fputs("\n",dest0);
    fputs("\n#ifndef _CTP_HEADER_FOMAT_H_",dest0);
    fputs("\n#define _CTP_HEADER_FOMAT_H_",dest0);
    fputs("\n",dest0);
    fputs("\n/****************************************************************************",dest0);
    fputs("\n *",dest0);
    fputs("\n * Header Files",dest0);
    fputs("\n *",dest0);
    fputs("\n****************************************************************************/",dest0);
    fputs("\n#include \"ctp_common.h\"",dest0);
    fputs("\n",dest0);
    fputs("\n/****************************************************************************",dest0);
    fputs("\n *",dest0);
    fputs("\n * Data Structure and Defines ",dest0);
    fputs("\n *",dest0);
    fputs("\n****************************************************************************/",dest0);
    fputs("\n",dest0);

    fputs("\n/****************************************************************************",dest1);
    fputs("\n * ctp_header_format.c  provides all CTP Supportted header protocol infomation.",dest1);
    fputs("\n *",dest1);
    fputs("\n * Copyright: (c)2007 Centec Networks Inc.  All rights reserved.",dest1);
    fputs("\n *",dest1);
    fputs("\n * This file is automatic generate by CTP_GEN_HEADER tools and use a script",dest1);
    fputs("\n * input file named ctp_header_format.txt",dest1);
    fputs("\n *",dest1);
    fputs("\n*****************************************************************************/",dest1);
    fputs("\n",dest1);
    fputs("\n/****************************************************************************",dest1);
    fputs("\n *",dest1);
    fputs("\n * Header Files",dest1);
    fputs("\n *",dest1);
    fputs("\n****************************************************************************/",dest1);
    fputs("\n#include <unistd.h>",dest1);
    fputs("\n#include \"ctp_common.h\"",dest1);
    fputs("\n",dest1);
    fputs("\n/****************************************************************************",dest1);
    fputs("\n *",dest1);
    fputs("\n * Global and Declarations ",dest1);
    fputs("\n *",dest1);
    fputs("\n****************************************************************************/",dest1);
    fputs("\n",dest1);
}

int
main(void)
{
#define M_LINE_BUFSIZE    2048
    char line[M_LINE_BUFSIZE];
 
    if (open_file())
    {
     return 0;
    }
    
    comment_string_output();
    
    /* types handle */    
    fputs("\n/*", dest0);
    fputs("\n * This enumeration is use for list all header types", dest0);
    fputs("\n*/", dest0);
    fputs("\nenum", dest0);
    fputs("\n{", dest0);

    fputs("\n/*", dest1);
    fputs("\n * This array is use for header types ID and String inter-search", dest1);
    fputs("\n*/", dest1);
    fputs("\nstatic char *header_type_str[] =", dest1);
    fputs("\n{", dest1);
    while (NULL != fgets(line, M_LINE_BUFSIZE, src))
    {
    if(!is_valid_line(line))
    {
        continue;
    }
    valid_handle(line, 1);
    }
    fputs("\n    HEADER_TYPE_NUM,",dest0);
    fputs("\n};\n",dest0);
    fputs("\n};\n",dest1);

    /* header type's start field handle */
    fputs("\n/*", dest1);
    fputs("\n * This array is use for header types start field ID", dest1);
    fputs("\n*/", dest1);
    fputs("\nstatic uint16 header_start_field[] =", dest1);
    fputs("\n{", dest1);
    rewind(src);
    while (NULL != fgets(line, M_LINE_BUFSIZE, src))
    {
        if(!is_valid_line(line))
        {
            continue;
        }
        valid_handle(line, 2);
    }
    fputs("\n};\n",dest1);

    /* flags infomation  handle */
    fputs("\n/*", dest1);
    fputs("\n * These arraies is use for flags infomation start field ID", dest1);
    fputs("\n*/", dest1);
    while (NULL != fgets(line, M_LINE_BUFSIZE, src1))
    {
        if(!is_valid_line(line))
        {
            continue;
        }
        valid_handle(line, 3);
    }


    /* field information handle */
    fputs("\n/*", dest0);
    fputs("\n * This enumeration is use for list all header fields", dest0);
    fputs("\n*/", dest0);
    fputs("\nenum", dest0);
    fputs("\n{", dest0);

    fputs("\n/*", dest1);
    fputs("\n * This array is use for header fields ID and String inter-search", dest1);
    fputs("\n*/", dest1);
    fputs("\nstatic ctp_field_info_t field_info[] =", dest1);
    fputs("\n{", dest1);
    rewind(src);
    while (NULL != fgets(line, M_LINE_BUFSIZE, src))
    {
        if(!is_valid_line(line))
        {
            continue;
        }
        valid_handle(line, 4);
    }
    fputs("\n    FIELD_NUM,",dest0);
    fputs("\n};\n",dest0);
    fputs("\n};\n",dest1);

    close_file();
    return 0;
}
