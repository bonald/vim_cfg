#ifndef _CLISH_CLI_H
#define _CLISH_CLI_H

/* Output modifier function return value.  */
#define CLISH_MODIFIER_BEGIN            0
#define CLISH_MODIFIER_INCLUDE          1
#define CLISH_MODIFIER_EXCLUDE          2
#define CLISH_MODIFIER_GREP             3
#define CLISH_MODIFIER_REDIRECT         4
#define CLISH_MODIFIER_FILENAME_ERROR   5
#define CLISH_MODIFIER_CANCELLED        6

#define CLISH_PARSE_GEN_ERROR       (-1)
#define CLISH_PARSE_EXIT_IF_RANGE   (-2)
#define CLISH_PARSE_EXEC_FAIL       (-3)
#define CLISH_PARSE_IF_NOT_EXIST    (-4)

#define MAX_ZONE_NAME_LEN      32
#define MIN_ZONE_NAME_LEN      3 /*for bug 28442 modifed by liuyang 2014-5-12*/
#define M_TIMEZONE_NAME_LIMIT_STR    "can only include [a-zA-Z_]. The begin and end character must be alphabet. Its length range is [3, 32)."
#define MAX_TIME_LEN      200

/* Skip white space. */
#define SKIP_WHITE_SPACE(P) \
  do { \
      if (NULL != P) \
       while (*P != '\0' && isspace ((int) * P)) \
         P++; \
   } while (0)

   typedef enum tagZoneOP
   {
       OP_NEGATIVE = 0,
       OP_POSITIVE
   }E_ZoneOP;

   /*timezone info struct*/
   typedef struct tagZoneInfo
   {
       char strZoneName[MAX_ZONE_NAME_LEN + 1]; /*zone name*/    
       int iHourOffset;  /*zone hour offset*/
       int iMinOffset;  /*zone min offset*/
       int iSecOffset;  /*zone sec offset*/
       E_ZoneOP operator; /*positive or negative from UTC*/
   }S_ZoneInfo;

typedef struct system_file_s
{
    char *filename; /* file name */
    char *usage; /* usage string */
} system_file_t;

int clish_confirm_overwrite_check(struct cli *cli, char *action,
        char *file, char *origfile);

#endif /*_CLISH_CLI_H*/
