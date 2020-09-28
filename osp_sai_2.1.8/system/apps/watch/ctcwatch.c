
#include <ctype.h>  
#include <dirent.h>  
#include <grp.h>  
#include <pwd.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <sys/types.h>  
#include <unistd.h>  
#include <signal.h>  

#define CMD_ZERO_CHAR           '\0'
#define CMD_QUOTATION_CHAR      '"'
#define CMD_DIVISION_SPACE      ' '
#define MAX_DEPTH           20

#define MATCH_CMD_SUCCESS    1
#define MATCH_CMD_FAIL   0

#define SHOW_IF_CMD_WORDS_THREE          3
#define SHOW_IF_CMD_WORDS_FOUR            4
#define SHOW_IF_CMD_WORDS_FIVE              5
#define SHOW_IF_CMD_WORDS_SIX             6

struct watch_opt_info
{  
    int n_flag;
    int t_flag;  
    char period[32];
}; 

typedef struct watch_cmd_s
{
    char *exec_cmd; /* command name in exec view */
    char *trans_cmd; /* transferred command in shell */
} watch_cmd_t;

typedef struct
{
    char       *argv[MAX_DEPTH];
    int      argc;
} cmd_parse_info_t;

watch_cmd_t g_watch_cmd_arr[] = 
{
    {"show environment", "cdbctl show/cdb/sys/show_environment"},
    {"show memory summary total", "cdbctl show/cdb/sys/mem_summary"},
    {"show memory process ccs", "cdbctl show/cdb/sys/show_common/ccs/memory"},
    {"show memory process cds", "cdbctl show/cdb/sys/show_common/cds/memory"},
    {"show memory process switch", "cdbctl show/cdb/sys/show_common/switch/memory"},
    {"show memory process opm", "cdbctl show/cdb/sys/show_common/opm/memory"},
    {"show memory process routed", "cdbctl show/cdb/sys/show_common/routed/memory"},
    {"show memory process chsm", "cdbctl show/cdb/sys/show_common/chsm/memory"},
    {"show memory process appcfg", "cdbctl show/cdb/sys/show_common/appcfg/memory"},
    {"show memory process fea", "cdbctl show/cdb/sys/show_common/fea/memory"},
    {"show memory process dhcsnooping", "cdbctl show/cdb/sys/show_common/dhcsnooping/memory"},
    {"show memory process dhcrelay", "cdbctl show/cdb/sys/show_common/dhcrelay/memory"},
    {"show memory process authd", "cdbctl show/cdb/sys/show_common/authd/memory"},
    {"show memory process ptp", "cdbctl show/cdb/sys/show_common/ptp/memory"},
    {"show memory process bhm", "cdbctl show/cdb/sys/show_common/bhm/memory"},
    {"show memory process ssm", "cdbctl show/cdb/sys/show_common/ssm/memory"},    
    {"show memory history", "cdbctl show/cdb/sys/show_cpu_mem_history/mem/1hour/60"},
    {"show memory history per-1-min", "cdbctl show/cdb/sys/show_cpu_mem_history/mem/1hour/60"},
    {"show memory history per-10-mins", "cdbctl show/cdb/sys/show_cpu_mem_history/mem/12hour/72"},
    {"show cpu utilization", "cdbctl show/cdb/app/cpu_utilization"},
    {"show cpu packets", "cdbctl show/cdb/sys/cpu_packets"},
    {"show cpu history", "cdbctl show/cdb/sys/show_cpu_mem_history/cpu/1hour/60"},
    {"show cpu history per-1-min", "cdbctl show/cdb/sys/show_cpu_mem_history/cpu/1hour/60"},
    {"show cpu history per-10-mins", "cdbctl show/cdb/sys/show_cpu_mem_history/cpu/12hour/72"},        
    {"show transceiver detail", "cdbctl show/cdb/sys/show_fiber/all/detail"},        
    {"show interface null", "cdbctl show/cdb/l2/show_interface/null0"},
    {"show interface status", "cdbctl show/cdb/interface/status"},
    {"show interface description", "cdbctl show/cdb/interface/description"},
    {"show interface summary", "cdbctl show/cdb/interface//summary"},
    {"show interface switchport", "cdbctl show/cdb/interface/switchport"},
    {"show interface trunk", "cdbctl show/cdb/interface/trunk"},
    {"show interface tunnel", "cdbctl show/cdb/debug/show_hsrv/openflow/tunnel/default"},
    {"show interface bandwidth-in-use", "cdbctl show/cdb/sys/show_bandwidth_usage"},              
    {"ovs-ofctl dump-flows br0", "ovs-ofctl dump-flows br0"},
    {"ovs-ofctl dump-meters br0", "ovs-ofctl dump-meters br0"},
    {"ovs-ofctl dump-groups br0", "ovs-ofctl dump-groups br0"},
    {"ovs-ofctl dump-flows br0 -O openflow13", "ovs-ofctl dump-flows br0 -O openflow13"},
    {"ovs-ofctl dump-meters br0 -O openflow13", "ovs-ofctl dump-meters br0 -O openflow13"},
    {"ovs-ofctl dump-groups br0 -O openflow13", "ovs-ofctl dump-groups br0 -O openflow13"}        
};

static int match_show_interface_bandwidth(char *cmd);

static int match_show_interface_summary(char *cmd);

static int match_show_interface_switchport_interface (char *cmd);

static int match_show_interface(char *cmd);

static int match_show_transceiver(char *cmd);

static void transfer_show_interface_bandwidth_cmd(char *cmd_str, char *transfer_cmd, cmd_parse_info_t *parse_info);

static void transfer_show_interface_summary_cmd(char *cmd_str, char *transfer_cmd, cmd_parse_info_t *parse_info);

static void transfer_show_interface_switchport_cmd(char *cmd_str, char *transfer_cmd, cmd_parse_info_t *parse_info);

static void transfer_show_interface_cmd(char *cmd_str, char *transfer_cmd, cmd_parse_info_t *parse_info);

static void transfer_show_transceiver_cmd(char *cmd_str, char *transfer_cmd, cmd_parse_info_t *parse_info);

static int watch_cmd_transfer(char *cmd_str, char *transfer_cmd);
static int watch_system_process(struct watch_opt_info *watch_opt, char *cmd_str) ;
static void usage(void);  


/* SIGHUP handler. */
void 
sighup (int sig)
{
    
}

/* SIGINT handler. */
void
sigint (int sig)
{
    exit (0);
}

/* Initialization of signal handles.  */
void
signal_init ()
{
    signal(SIGHUP, sighup);
    signal(SIGINT, sigint);
    signal(SIGTERM, sigint);
}


int main(int argc, char *argv[]) {  
    int i = 1;  
    char cmd_str[100] = {0};
    struct watch_opt_info watch_opt = {0};

    for (i = 1; i < argc; i++) 
    {   
        if (!strcmp(argv[i], "-n")) 
        {  
            if (i + 1 >= argc) 
            {  
                fprintf(stderr, "Option -n expects an argument.\n");  
                usage();  
                exit(EXIT_FAILURE);  
            }  
            watch_opt.n_flag = 1;
            strcpy(watch_opt.period, argv[++i]);
            
            continue;  
        }  
        else if (!strcmp(argv[i], "-t")) 
        {  
            if (i + 1 >= argc) 
            {  
                usage();  
                exit(EXIT_FAILURE);  
            }

            watch_opt.t_flag = 1;
            continue;
        }  
        else if (!strcmp(argv[i], "-h")) 
        {  
            usage();  
            exit(EXIT_SUCCESS);  
        } 
        else if(argv[i])
        {
            strcat(cmd_str, argv[i]);
            if (i < argc -1)
            {
                strcat(cmd_str, " ");
            }
            continue;
        }
        else
        {
            fprintf(stderr, "Invalid argument \"%s\".\n", argv[i]);  
            usage();  
            exit(EXIT_FAILURE);  
        }
    } 
    
    /* Initialize signal.  */
    signal_init ();

    /* transfer cmd to shell cmd and run*/
    watch_system_process(&watch_opt, cmd_str);
    
    return 0;  
}  

int
watch_cmd_parse_para(char *para_str, cmd_parse_info_t *p_info)
{
    int          cmdstrlen = 0;
    char           *pos = NULL;

    memset(p_info, 0, sizeof(cmd_parse_info_t));

    if (NULL == para_str)
    {
        return 0;
    }
   
    /* fix cdbctl crash problem */
    cmdstrlen = strlen(para_str);
    if (0 == cmdstrlen)
    {
        return 0;
    }
       
    if (para_str[cmdstrlen-1] == CMD_DIVISION_SPACE)
    {
        para_str[cmdstrlen-1] = 0;
    }

    pos = para_str;
    p_info->argv[p_info->argc++] = pos;
    pos = strchr(para_str, CMD_DIVISION_SPACE);
    
    while (pos)
    {
        *pos = 0;
        p_info->argv[p_info->argc++] = pos + 1;
        pos = strchr(pos + 1, CMD_DIVISION_SPACE);
    }
    
    return 0;
}

static int match_show_interface_bandwidth(char *cmd)
{
    int size = strlen("show interface bandwidth-in-use ");
    if (!strncmp(cmd, "show interface bandwidth-in-use ",size))
    {
        return MATCH_CMD_SUCCESS;
    }
    
    return 0;
}

static int match_show_interface_summary(char *cmd)
{
    int size = strlen("show interface summary ");
    if (!strncmp(cmd, "show interface summary ",size))
    {
        return MATCH_CMD_SUCCESS;
    }
    return 0;
}

static int match_show_interface_switchport_interface (char *cmd)
{
    int size = strlen("show interface switchport interface ");
    if (!strncmp(cmd, "show interface switchport interface ",size))
    {
        return MATCH_CMD_SUCCESS;
    }
    
    return 0;
}


static int match_show_interface(char *cmd)
{
    int size = strlen("show interface ");
    if (!strncmp(cmd, "show interface ",size))
    {
        return MATCH_CMD_SUCCESS;
    }
    
    return 0;
}

static int match_show_transceiver(char *cmd)
{
    int size = strlen("show transceiver ");
    if (!strncmp(cmd, "show transceiver ",size))
    {
        return MATCH_CMD_SUCCESS;
    }
    
    return 0;
}

static int match_show_cpu_history(char *cmd)
{
    int size1 = strlen("show cpu history per-1-min ");
    int size2 = strlen("show cpu history per-10-mins ");
    if (!strncmp(cmd, "show cpu history per-1-min ",size1))
    {
        return MATCH_CMD_SUCCESS;
    }
    else if (!strncmp(cmd, "show cpu history per-10-mins ",size2))
    {
        return MATCH_CMD_SUCCESS;
    }
    
    return 0;
}

static int match_show_mem_history(char *cmd)
{
    int size1 = strlen("show memory history per-1-min ");
    int size2 = strlen("show memory history per-10-mins ");
    if (!strncmp(cmd, "show memory history per-1-min ",size1))
    {
        return MATCH_CMD_SUCCESS;
    }
    else if (!strncmp(cmd, "show memory history per-10-mins ",size2))
    {
        return MATCH_CMD_SUCCESS;
    }

    return 0;
}

static void transfer_show_interface_bandwidth_cmd(char *cmd_str, char *transfer_cmd, cmd_parse_info_t *parse_info)
{
        strcat(transfer_cmd, "cdbctl show/cdb/sys/show_bandwidth_usage/");
        if (SHOW_IF_CMD_WORDS_FOUR == parse_info->argc)
        {
            strcat(transfer_cmd, parse_info->argv[SHOW_IF_CMD_WORDS_FOUR - 1]);  
        }
        else if (SHOW_IF_CMD_WORDS_FIVE == parse_info->argc)
        {
            strcat(transfer_cmd, parse_info->argv[SHOW_IF_CMD_WORDS_FIVE - 2]);
            strcat(transfer_cmd, "/");
            strcat(transfer_cmd, parse_info->argv[SHOW_IF_CMD_WORDS_FIVE - 1]);  
        }
}

static void transfer_show_interface_summary_cmd(char *cmd_str, char *transfer_cmd, cmd_parse_info_t *parse_info)
{
        strcat(transfer_cmd, "cdbctl show/cdb/interface/");
        strcat(transfer_cmd, parse_info->argv[SHOW_IF_CMD_WORDS_FOUR - 1]);
        strcat(transfer_cmd, "/summary");
}

static void transfer_show_interface_switchport_cmd(char *cmd_str, char *transfer_cmd, cmd_parse_info_t *parse_info)
{
        strcat(transfer_cmd, "cdbctl show/cdb/interface/");
        strcat(transfer_cmd, parse_info->argv[SHOW_IF_CMD_WORDS_FIVE - 1]);
        strcat(transfer_cmd, "/switchport");
}

static void transfer_show_interface_cmd(char *cmd_str, char *transfer_cmd, cmd_parse_info_t *parse_info)
{
        strcat(transfer_cmd, "cdbctl show/cdb/l2/show_interface/");
        strcat(transfer_cmd, parse_info->argv[SHOW_IF_CMD_WORDS_THREE - 1]);    
}

static void transfer_show_cpu_history_cmd(char *cmd_str, char *transfer_cmd, cmd_parse_info_t *parse_info)
{
        if (strstr(cmd_str, "per-1-min"))
        {
            strcat(transfer_cmd, "cdbctl show/cdb/sys/show_cpu_mem_history/cpu/1hour/");
        }
        else if (strstr(cmd_str, "per-10-mins"))
        {
            strcat(transfer_cmd, "cdbctl show/cdb/sys/show_cpu_mem_history/cpu/12hour/");
        }        
        strcat(transfer_cmd, parse_info->argv[SHOW_IF_CMD_WORDS_FIVE - 1]); 
}

static void transfer_show_mem_history_cmd(char *cmd_str, char *transfer_cmd, cmd_parse_info_t *parse_info)
{
        if (strstr(cmd_str, "per-1-min"))
        {
            strcat(transfer_cmd, "cdbctl show/cdb/sys/show_cpu_mem_history/mem/1hour/");
        }
        else if (strstr(cmd_str, "per-10-mins"))
        {
            strcat(transfer_cmd, "cdbctl show/cdb/sys/show_cpu_mem_history/mem/12hour/");
        }        
        strcat(transfer_cmd, parse_info->argv[SHOW_IF_CMD_WORDS_FIVE - 1]); 
}

static void transfer_show_transceiver_cmd(char *cmd_str, char *transfer_cmd, cmd_parse_info_t *parse_info)
{
        int i = 0;
        
        strcat(transfer_cmd, "cdbctl show/cdb/sys/show_fiber/");
        
        while(i < parse_info->argc)
        {
            if (strstr(parse_info->argv[i], "eth-"))
            {
                break;
            }
            else
            {
                i++;
            }
        }
        
        if (strstr(cmd_str, "eth-"))
        {
            strcat(transfer_cmd, parse_info->argv[i]); 
        }
        else
        {
            strcat(transfer_cmd, "all"); 
        }
        
        if (strstr(cmd_str, "detail"))
        {
            strcat(transfer_cmd, "/detail");
        }

}

static int watch_cmd_match_transfer(char *cmd_str, char *transfer_cmd, cmd_parse_info_t *parse_info)
{
    /* match and transfer cmd*/
     if (match_show_interface_bandwidth(cmd_str) 
         && (parse_info->argc == SHOW_IF_CMD_WORDS_FOUR || parse_info->argc == SHOW_IF_CMD_WORDS_FIVE))
     {
         transfer_show_interface_bandwidth_cmd(cmd_str, transfer_cmd, parse_info);
         return MATCH_CMD_SUCCESS;
     }
     
     if (match_show_transceiver(cmd_str) 
         && (parse_info->argc == SHOW_IF_CMD_WORDS_FOUR || parse_info->argc == SHOW_IF_CMD_WORDS_THREE))
     {
         transfer_show_transceiver_cmd(cmd_str, transfer_cmd, parse_info);
         return MATCH_CMD_SUCCESS;
     } 
     
     if (match_show_interface_summary(cmd_str) && parse_info->argc == SHOW_IF_CMD_WORDS_FOUR)
     {
         transfer_show_interface_summary_cmd(cmd_str, transfer_cmd, parse_info);
         return MATCH_CMD_SUCCESS;
     }
     
     if (match_show_interface_switchport_interface(cmd_str) && parse_info->argc == SHOW_IF_CMD_WORDS_FIVE)
     {
         transfer_show_interface_switchport_cmd(cmd_str, transfer_cmd, parse_info);
         return MATCH_CMD_SUCCESS;
     } 
     
     if (match_show_interface(cmd_str) && parse_info->argc == SHOW_IF_CMD_WORDS_THREE)
     {
         transfer_show_interface_cmd(cmd_str, transfer_cmd, parse_info);
         return MATCH_CMD_SUCCESS;
     }
    
     if (match_show_cpu_history(cmd_str) && parse_info->argc == SHOW_IF_CMD_WORDS_FIVE)
     {
         transfer_show_cpu_history_cmd(cmd_str, transfer_cmd, parse_info);
         return MATCH_CMD_SUCCESS;
     }
    
     if (match_show_mem_history(cmd_str) && parse_info->argc == SHOW_IF_CMD_WORDS_FIVE)
     {
         transfer_show_mem_history_cmd(cmd_str, transfer_cmd, parse_info);
         return MATCH_CMD_SUCCESS;
     }
    return MATCH_CMD_FAIL;
}

static int watch_cmd_transfer(char *cmd_str, char *transfer_cmd)
{
    char tmp_cmd_str[100] = {0}; 
    int         cmdstrlen = 0;
    int         ret = 0;
    cmd_parse_info_t parse_info;

    memset(&parse_info, 0, sizeof(parse_info));

    if (NULL == cmd_str)
    {
        return MATCH_CMD_FAIL;
    }
    /*if has more than " " return, but xml STRING type can trip extra indent */
    if (cmd_str && strstr(cmd_str, "  "))
    {
        return MATCH_CMD_FAIL;
    }
  
    cmdstrlen = strlen(cmd_str);
    if (0 == cmdstrlen)
    {
        return MATCH_CMD_FAIL;
    }
    
    strcpy(tmp_cmd_str, cmd_str);
    
    /* get each word in cmd and the number of words*/
    watch_cmd_parse_para(tmp_cmd_str, &parse_info);

    /* match and transfer cmd*/
    ret = watch_cmd_match_transfer(cmd_str, transfer_cmd, &parse_info);
    
    return ret;
    
}

static int watch_system_process(struct watch_opt_info *watch_opt, char *cmd_str) 
{ 
    char watch_str[200] = {0};
    int arr_size = 0;
    int i = 0;

    strcpy(watch_str, "watch ");
    if(watch_opt->n_flag)
    {
        strcat(watch_str, "-n ");
        strcat(watch_str, watch_opt->period);
        strcat(watch_str, " ");
    }
    
    if(watch_opt->t_flag)
    {
        strcat(watch_str, "-t ");
    } 

    arr_size = sizeof(g_watch_cmd_arr) / sizeof(g_watch_cmd_arr[0]);

    /*search cmd in array absolutely*/
    while(cmd_str && i < arr_size && g_watch_cmd_arr[i].exec_cmd)
    {
        if (!strcmp(cmd_str, g_watch_cmd_arr[i].exec_cmd))
        {
            strcat(watch_str, g_watch_cmd_arr[i].trans_cmd);
            break;
        }
        else
        {
            i++;
        }
    
        if (i == arr_size || NULL == g_watch_cmd_arr[i].exec_cmd)
        {
            break;
        }
    }

    /*cmd can not match in array absolutely*/
    if (i != arr_size)
    {
#if 0 /*for debug*/
        fprintf(stdout, "watch_str_arr (%s)\n",  watch_str);
        sleep(5);
#endif
        system(watch_str);
        return 0;
    }

    /*search and transfer cmd with ifname or other variable*/
    if (watch_cmd_transfer(cmd_str, watch_str))
    {

#if 0 /*for debug*/
        fprintf(stdout, "watch_str_trans (%s)\n",  watch_str);
        sleep(5);
#endif        
        system(watch_str);
        return 0; 
    }

    /*can not find cmd by above two method*/
    printf("This command (%s) is not supported to watch!\n", cmd_str);
    usage();
    exit(EXIT_FAILURE); 
}  

static void usage(void) {  
    fprintf(stderr, "Usage: watch [ -n sec ] [ -t ] [ -h ] COMMAND\n"  
                    "    -n sec  Loop period in seconds (default 2).\n"  
                    "    -t      Don't print header.\n"  
                    "    -h      Display this help screen.\n"
                    "    Supported COMMANDs:\n"
                    "            show environment\n"        
                    "            show memory (history|process|summary)\n"
                    "            show cpu (history|packets|utilization)\n"
                    "            show transceiver (eth- | detail)\n"
                    "            show interface xxx\n"
                    "            ovs-ofctl dump-flows br0 -O openflow13\n"
                    "            ovs-ofctl dump-meters br0 -O openflow13\n"
                    "            ovs-ofctl dump-groups br0 -O openflow13\n" );  
}  





