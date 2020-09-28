#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#define TMPFILE "/tmp/startup-config.tmp"
#define WANTMATCH 0
#define UNWANTMATCH 1
#define COMMANDLEN 64


int main(int argc, char *argv[])
{
    FILE *fp;
    FILE *fp_tmp;
    char line_buf[LINE_MAX];
    unsigned int match_flag;
    char command[COMMANDLEN] = { 0 };
    int privilege_level_haved = 0; // have no  privilege level
    int line_vty = 0;
    int has_login = 0;

    if(argc != 2) {
        printf("Please in put the startup config file path\n");
    }
    match_flag = UNWANTMATCH;
    if((fp = fopen(argv[1], "r")) == NULL) {
        perror("open configure file failed\n");
        return 1;
    }
    if( !( fp_tmp = fopen(TMPFILE, "w") )) {
        perror("creat a temp file failed\n");
        return 1;
    }
    
    while (fgets(line_buf, LINE_MAX, fp) != NULL) {

        if(strstr(line_buf, "aaa new-model") != NULL) {
            continue;
        }
        if(strstr(line_buf, "aaa authentication") != NULL) {
            continue;
        }
        if(strstr(line_buf, "aaa authorization") != NULL) {
            continue;
        }
        if(strstr(line_buf, "aaa accounting") != NULL) {
            continue;
        }
        if(strstr(line_buf, "aaa privilege") != NULL) {
            continue;
        }

        if(strstr(line_buf, "line console 0") != NULL) {
            match_flag = WANTMATCH;
        }
        
        if(strstr(line_buf, "line vty") != NULL) {
            line_vty = 1;
        }

        if((match_flag == WANTMATCH) && (strstr(line_buf, "line vty") != NULL)) {
           
           if(privilege_level_haved == 0) 
           {
               fputs(" privilege level 4\n", fp_tmp);
           }
           /* Deleted by kcao for bug 39633 2016-07-19 */
           // match_flag = UNWANTMATCH;
        }

        if(match_flag == WANTMATCH) {
            if(strstr(line_buf,"login") != NULL) {
                strcpy(line_buf," no login\n");
                if (line_vty)
                {
                    has_login = 1;
                }
            }

            if(strstr(line_buf, "privilege level") != NULL) {
                privilege_level_haved = 1;
                strcpy(line_buf," privilege level 4\n");
            } 
        }

        fputs(line_buf, fp_tmp);
    }

    if (!has_login)
    {
        strcpy(line_buf," no login\n");
        fputs(line_buf, fp_tmp);
    }

    fclose(fp);
    fclose(fp_tmp);
    unlink(argv[1]);
    sprintf(command,"mv %s %s", TMPFILE, argv[1]);
    system(command);

    return 0;

}
