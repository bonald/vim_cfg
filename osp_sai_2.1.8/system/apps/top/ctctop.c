
#include <ctype.h>  
#include <dirent.h>  
#include <grp.h>  
#include <pwd.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <sys/types.h>  
#include <unistd.h>  

#define CTCTOP_PRINT

//#include <cutils/sched_policy.h>  
/** 
typedef enum { 
    SP_BACKGROUND = 0, 
    SP_FOREGROUND = 1, 
} SchedPolicy; 
*/  
//extern int get_sched_policy(int tid, SchedPolicy *policy);  
  
struct cpu_info {  
    long long unsigned utime, ntime, stime, itime;  
    long long unsigned iowtime, irqtime, sirqtime;  
};  
  
#define PROC_NAME_LEN 64  
#define THREAD_NAME_LEN 32  
  
struct proc_info {  
    struct proc_info *next;  
    pid_t pid;  
    pid_t tid;  
    uid_t uid;  
    gid_t gid;  
    char name[PROC_NAME_LEN];  
    char tname[THREAD_NAME_LEN];  
    char state;  
    long unsigned utime;  
    long unsigned stime;  
    long unsigned delta_utime;  
    long unsigned delta_stime;  
    long unsigned delta_time;  
    long vss;  
    long rss;  
    int num_threads;  
    char policy[32];  
};  
  
struct proc_list {  
    struct proc_info **array;  
    int size;  
};  
  
#define die(...) { fprintf(stderr, __VA_ARGS__); exit(EXIT_FAILURE); }  
  
#define INIT_PROCS 50  
#define THREAD_MULT 8  
static struct proc_info **old_procs, **new_procs;  
static int num_old_procs, num_new_procs;  
static struct proc_info *free_procs;  
static int num_used_procs, num_free_procs;  
  
static int max_procs, delay, iterations, threads;  
static int debug;

static struct cpu_info old_cpu, new_cpu;  
  
static struct proc_info *alloc_proc(void);  
static void free_proc(struct proc_info *proc);  
static void read_procs(void);  
static int read_stat(char *filename, struct proc_info *proc);  
static void read_policy(int pid, struct proc_info *proc);  
static void add_proc(int proc_num, struct proc_info *proc);  
static int read_cmdline(char *filename, struct proc_info *proc);  
static int read_status(char *filename, struct proc_info *proc);  
static void print_procs(void);  
static struct proc_info *find_old_proc(pid_t pid, pid_t tid);  
static void free_old_procs(void);  
static int (*proc_cmp)(const void *a, const void *b);  
static int proc_cpu_cmp(const void *a, const void *b);  
static int proc_vss_cmp(const void *a, const void *b);  
static int proc_rss_cmp(const void *a, const void *b);  
static int proc_thr_cmp(const void *a, const void *b);  
static int numcmp(long long a, long long b);  
static void usage(char *cmd);  
  
int main(int argc, char *argv[]) {  
    int i;  
  
    num_used_procs = num_free_procs = 0;  
  
    max_procs = 0;  
    delay = 30;
    iterations = -1;  
    proc_cmp = &proc_cpu_cmp;  
    for (i = 1; i < argc; i++) {  
        if (!strcmp(argv[i], "-m")) {  
            if (i + 1 >= argc) {  
                fprintf(stderr, "Option -m expects an argument.\n");  
                usage(argv[0]);  
                exit(EXIT_FAILURE);  
            }  
            max_procs = atoi(argv[++i]);  
            continue;  
        }  
        if (!strcmp(argv[i], "-n")) {  
            if (i + 1 >= argc) {  
                fprintf(stderr, "Option -n expects an argument.\n");  
                usage(argv[0]);  
                exit(EXIT_FAILURE);  
            }  
            iterations = atoi(argv[++i]);  
            continue;  
        }  
        if (!strcmp(argv[i], "-d")) {
#if 0            
            if (i + 1 >= argc) {  
                fprintf(stderr, "Option -d expects an argument.\n");  
                usage(argv[0]);  
                exit(EXIT_FAILURE);  
            }  
            delay = atoi(argv[++i]);  
#else
            debug = 1;
#endif
            continue;  
        }  
        if (!strcmp(argv[i], "-s")) {  
            if (i + 1 >= argc) {  
                fprintf(stderr, "Option -s expects an argument.\n");  
                usage(argv[0]);  
                exit(EXIT_FAILURE);  
            }  
            ++i;  
            if (!strcmp(argv[i], "cpu")) { proc_cmp = &proc_cpu_cmp; continue; }  
            if (!strcmp(argv[i], "vss")) { proc_cmp = &proc_vss_cmp; continue; }  
            if (!strcmp(argv[i], "rss")) { proc_cmp = &proc_rss_cmp; continue; }  
            if (!strcmp(argv[i], "thr")) { proc_cmp = &proc_thr_cmp; continue; }  
            fprintf(stderr, "Invalid argument \"%s\" for option -s.\n", argv[i]);  
            exit(EXIT_FAILURE);  
        }  
        if (!strcmp(argv[i], "-t")) { threads = 1; continue; }  
        if (!strcmp(argv[i], "-h")) {  
            usage(argv[0]);  
            exit(EXIT_SUCCESS);  
        }  
        fprintf(stderr, "Invalid argument \"%s\".\n", argv[i]);  
        usage(argv[0]);  
        exit(EXIT_FAILURE);  
    }  
  
    if (threads && proc_cmp == &proc_thr_cmp) {  
        fprintf(stderr, "Sorting by threads per thread makes no sense!\n");  
        exit(EXIT_FAILURE);  
    }  
  
    free_procs = NULL;  
  
    num_new_procs = num_old_procs = 0;  
    new_procs = old_procs = NULL;  
  
    read_procs();  
    while ((iterations == -1) || (iterations-- > 0)) {  
        old_procs = new_procs;  
        num_old_procs = num_new_procs;  
        memcpy(&old_cpu, &new_cpu, sizeof(old_cpu));  
        sleep(delay);  
        read_procs();  
        print_procs();  
        free_old_procs();  
    }  
  
    return 0;  
}  
  
static struct proc_info *alloc_proc(void) {  
    struct proc_info *proc;  
  
    if (free_procs) {  
        proc = free_procs;  
        free_procs = free_procs->next;  
        num_free_procs--;  
    } else {  
        proc = malloc(sizeof(*proc));  
        if (!proc) die("Could not allocate struct process_info.\n");  
    }  
  
    num_used_procs++;  
  
    return proc;  
}  
  
static void free_proc(struct proc_info *proc) {  
    proc->next = free_procs;  
    free_procs = proc;  
  
    num_used_procs--;  
    num_free_procs++;  
}  
  
#define MAX_LINE 256  
  
static void read_procs(void) {  
    DIR *proc_dir, *task_dir;  
    struct dirent *pid_dir, *tid_dir;  
    char filename[64];  
    FILE *file;  
    int proc_num;  
    struct proc_info *proc;  
    pid_t pid, tid;  
  
    int i;  
  
    proc_dir = opendir("/proc");  
    if (!proc_dir) die("Could not open /proc.\n");  
  
    new_procs = calloc(INIT_PROCS * (threads ? THREAD_MULT : 1), sizeof(struct proc_info *));  
    num_new_procs = INIT_PROCS * (threads ? THREAD_MULT : 1);  
  
    file = fopen("/proc/stat", "r");  
    if (!file) die("Could not open /proc/stat.\n");  
    fscanf(file, "cpu  %llu %llu %llu %llu %llu %llu %llu", &new_cpu.utime, &new_cpu.ntime, &new_cpu.stime,  
            &new_cpu.itime, &new_cpu.iowtime, &new_cpu.irqtime, &new_cpu.sirqtime);  
    fclose(file);
    if (debug)
    {
        printf("[CPU] %llu %llu %llu %llu %llu %llu %llu\n", new_cpu.utime, new_cpu.ntime, new_cpu.stime,  
                new_cpu.itime, new_cpu.iowtime, new_cpu.irqtime, new_cpu.sirqtime);
    }

    proc_num = 0;  
    while ((pid_dir = readdir(proc_dir))) {  
        if (!isdigit(pid_dir->d_name[0]))  
            continue;  
  
        pid = atoi(pid_dir->d_name);  
          
        struct proc_info cur_proc;  
          
        if (!threads) {  
            proc = alloc_proc();  
  
            proc->pid = proc->tid = pid;  
  
            sprintf(filename, "/proc/%d/stat", pid);  
            read_stat(filename, proc);  
  
            sprintf(filename, "/proc/%d/cmdline", pid);  
            read_cmdline(filename, proc);  
  
            sprintf(filename, "/proc/%d/status", pid);  
            read_status(filename, proc);  
  
            read_policy(pid, proc);  
  
            proc->num_threads = 0;  
        } else {  
            sprintf(filename, "/proc/%d/cmdline", pid);  
            read_cmdline(filename, &cur_proc);  
  
            sprintf(filename, "/proc/%d/status", pid);  
            read_status(filename, &cur_proc);  
              
            proc = NULL;  
        }  
  
        sprintf(filename, "/proc/%d/task", pid);  
        task_dir = opendir(filename);  
        if (!task_dir) continue;  
  
        while ((tid_dir = readdir(task_dir))) {  
            if (!isdigit(tid_dir->d_name[0]))  
                continue;  
  
            if (threads) {  
                tid = atoi(tid_dir->d_name);  
  
                proc = alloc_proc();  
  
                proc->pid = pid; proc->tid = tid;  
  
                sprintf(filename, "/proc/%d/task/%d/stat", pid, tid);  
                read_stat(filename, proc);  
  
                read_policy(tid, proc);  
  
                strcpy(proc->name, cur_proc.name);  
                proc->uid = cur_proc.uid;  
                proc->gid = cur_proc.gid;  
  
                add_proc(proc_num++, proc);  
            } else {  
                proc->num_threads++;  
            }  
        }  
  
        closedir(task_dir);  
          
        if (!threads)  
            add_proc(proc_num++, proc);  
    }  
  
    for (i = proc_num; i < num_new_procs; i++)  
        new_procs[i] = NULL;  
  
    closedir(proc_dir);  
}  
  
static int read_stat(char *filename, struct proc_info *proc) {  
    FILE *file;  
    char buf[MAX_LINE], *open_paren, *close_paren;  
  
    file = fopen(filename, "r");  
    if (!file) return 1;  
    fgets(buf, MAX_LINE, file);  
    fclose(file);  
  
    /* Split at first '(' and last ')' to get process name. */  
    open_paren = strchr(buf, '(');  
    close_paren = strrchr(buf, ')');  
    if (!open_paren || !close_paren) return 1;  
  
    *open_paren = *close_paren = '\0';  
    strncpy(proc->tname, open_paren + 1, THREAD_NAME_LEN);  
    proc->tname[THREAD_NAME_LEN-1] = 0;  
      
    /* Scan rest of string. */  
    sscanf(close_paren + 1, " %c %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d "  
                 "%lu %lu %*d %*d %*d %*d %*d %*d %*d %ld %ld",  
                 &proc->state, &proc->utime, &proc->stime, &proc->vss, &proc->rss);
    if (debug)
    {
        if (proc->utime)
        {
            printf("[PROC] %s %s\n", filename, close_paren + 1);
            printf("[PROC] %s utime %lu, stime %lu, vss %ld, rss %ld\n", filename, proc->utime, proc->stime, proc->vss, proc->rss);
        }
    }
    
    return 0;  
}  
  
static void add_proc(int proc_num, struct proc_info *proc) {  
    int i;  
  
    if (proc_num >= num_new_procs) {  
        new_procs = realloc(new_procs, 2 * num_new_procs * sizeof(struct proc_info *));  
        if (!new_procs) die("Could not expand procs array.\n");  
        for (i = num_new_procs; i < 2 * num_new_procs; i++)  
            new_procs[i] = NULL;  
        num_new_procs = 2 * num_new_procs;  
    }  
    new_procs[proc_num] = proc;  
}  
  
static int read_cmdline(char *filename, struct proc_info *proc) {  
    FILE *file;  
    char line[MAX_LINE];  
  
    line[0] = '\0';  
    file = fopen(filename, "r");  
    if (!file) return 1;  
    fgets(line, MAX_LINE, file);  
    fclose(file);  
    if (strlen(line) > 0) {  
        strncpy(proc->name, line, PROC_NAME_LEN);  
        proc->name[PROC_NAME_LEN-1] = 0;  
    } else  
        proc->name[0] = 0;  
    return 0;  
}  
  
static void read_policy(int pid, struct proc_info *proc) {  
    /** 
    SchedPolicy p; 
    if (get_sched_policy(pid, &p) < 0) 
        strcpy(proc->policy, "unk"); 
    else { 
        if (p == SP_BACKGROUND) 
            strcpy(proc->policy, "bg"); 
        else if (p == SP_FOREGROUND) 
            strcpy(proc->policy, "fg"); 
        else 
            strcpy(proc->policy, "er"); 
    }*/  
}  
  
static int read_status(char *filename, struct proc_info *proc) {  
    FILE *file;  
    char line[MAX_LINE];  
    unsigned int uid, gid;  
  
    file = fopen(filename, "r");  
    if (!file) return 1;  
    while (fgets(line, MAX_LINE, file)) {  
        sscanf(line, "Uid: %u", &uid);  
        sscanf(line, "Gid: %u", &gid);  
    }  
    fclose(file);  
    proc->uid = uid; proc->gid = gid;  
    return 0;  
}  
  
static void print_procs(void) {  
    int i;  
    struct proc_info *old_proc, *proc;  
    long long total_delta_time;  
    double cpu_usage;
    struct passwd *user;  
    struct group *group;  
    char *user_str, user_buf[20];  
    char *group_str, group_buf[20];
    FILE *fp = NULL;

    (void)user_str;
    (void)group_str;
    
    for (i = 0; i < num_new_procs; i++) {  
        if (new_procs[i]) {  
            old_proc = find_old_proc(new_procs[i]->pid, new_procs[i]->tid);  
            if (old_proc) {  
                new_procs[i]->delta_utime = new_procs[i]->utime - old_proc->utime;  
                new_procs[i]->delta_stime = new_procs[i]->stime - old_proc->stime;  
            } else {  
                new_procs[i]->delta_utime = 0;  
                new_procs[i]->delta_stime = 0;  
            }  
            new_procs[i]->delta_time = new_procs[i]->delta_utime + new_procs[i]->delta_stime;  
        }  
    }  
  
    total_delta_time = (new_cpu.utime + new_cpu.ntime + new_cpu.stime + new_cpu.itime  
                        + new_cpu.iowtime + new_cpu.irqtime + new_cpu.sirqtime)  
                     - (old_cpu.utime + old_cpu.ntime + old_cpu.stime + old_cpu.itime  
                        + old_cpu.iowtime + old_cpu.irqtime + old_cpu.sirqtime);  
  
    qsort(new_procs, num_new_procs, sizeof(struct proc_info *), proc_cmp);  

    fp = fopen("/tmp/cpu_usage", "w+");
    if (NULL == fp)
    {
        return;
    }
    
#ifdef CTCTOP_PRINT
    fprintf(fp, "%-5s %-5s %-5s %-5s %-5s %-5s %-5s %-5s\n", "User", "Nice", "Sys", "Idle", "IOW", "IRQ", "SIRQ", "Total");
    fprintf(fp, "%-5lld %-5lld %-5lld %-5lld %-5lld %-5lld %-5lld %-5lld\n",
            new_cpu.utime - old_cpu.utime,  
            new_cpu.ntime - old_cpu.ntime,  
            new_cpu.stime - old_cpu.stime,  
            new_cpu.itime - old_cpu.itime,  
            new_cpu.iowtime - old_cpu.iowtime,  
            new_cpu.irqtime - old_cpu.irqtime,  
            new_cpu.sirqtime - old_cpu.sirqtime,  
            total_delta_time);
#else
    fprintf(fp, "User %ld%%, System %ld%%, IOW %ld%%, IRQ %ld%%\n",  
            ((new_cpu.utime + new_cpu.ntime) - (old_cpu.utime + old_cpu.ntime)) * 100  / total_delta_time,  
            ((new_cpu.stime ) - (old_cpu.stime)) * 100 / total_delta_time,  
            ((new_cpu.iowtime) - (old_cpu.iowtime)) * 100 / total_delta_time,  
            ((new_cpu.irqtime + new_cpu.sirqtime)  
                    - (old_cpu.irqtime + old_cpu.sirqtime)) * 100 / total_delta_time);  
    fprintf(fp, "User %ld + Nice %ld + Sys %ld + Idle %ld + IOW %ld + IRQ %ld + SIRQ %ld = %ld\n",  
            new_cpu.utime - old_cpu.utime,  
            new_cpu.ntime - old_cpu.ntime,  
            new_cpu.stime - old_cpu.stime,  
            new_cpu.itime - old_cpu.itime,  
            new_cpu.iowtime - old_cpu.iowtime,  
            new_cpu.irqtime - old_cpu.irqtime,  
            new_cpu.sirqtime - old_cpu.sirqtime,  
            total_delta_time);
#endif /* !CTCTOP_PRINT */

    fprintf(fp, "\n");  
#ifdef CTCTOP_PRINT
        fprintf(fp, "%-5s %-6s %s\n", "PID", "CPU", "Name");  
#else
    if (!threads)   
        fprintf(fp, "%5s %4s %1s %5s %7s %7s %3s %-8s %s\n", "PID", "CPU%", "S", "#THR", "VSS", "RSS", "PCY", "UID", "Name");  
    else  
        fprintf(fp, "%5s %5s %4s %1s %7s %7s %3s %-8s %-15s %s\n", "PID", "TID", "CPU%", "S", "VSS", "RSS", "PCY", "UID", "Thread", "Proc");  
#endif /* !CTCTOP_PRINT */

    for (i = 0; i < num_new_procs; i++) {  
        proc = new_procs[i];  
  
        if (!proc || (max_procs && (i >= max_procs)))  
            break;  
        user  = getpwuid(proc->uid);  
        group = getgrgid(proc->gid);  
        if (user && user->pw_name) {  
            user_str = user->pw_name;  
        } else {  
            snprintf(user_buf, 20, "%d", proc->uid);  
            user_str = user_buf;  
        }  
        if (group && group->gr_name) {  
            group_str = group->gr_name;  
        } else {  
            snprintf(group_buf, 20, "%d", proc->gid);  
            group_str = group_buf;  
        }  
#ifdef CTCTOP_PRINT
        cpu_usage = proc->delta_time * 100.0 / total_delta_time;
        if (0 != proc->delta_time)
        {
            fprintf(fp, "%-5d %-6.2f %s\n", proc->pid, cpu_usage, proc->tname[0] != 0 ? proc->tname : proc->name);
        }
#else
        if (!threads)   
            fprintf(fp, "%5d %3ld%% %c %5d %6ldK %6ldK %3s %-8.8s %s\n", proc->pid, proc->delta_time * 100 / total_delta_time, proc->state, proc->num_threads,  
                proc->vss / 1024, proc->rss * getpagesize() / 1024, proc->policy, user_str, proc->name[0] != 0 ? proc->name : proc->tname);  
        else  
            fprintf(fp, "%5d %5d %3ld%% %c %6ldK %6ldK %3s %-8.8s %-15s %s\n", proc->pid, proc->tid, proc->delta_time * 100 / total_delta_time, proc->state,  
                proc->vss / 1024, proc->rss * getpagesize() / 1024, proc->policy, user_str, proc->tname, proc->name);  
#endif /* !CTCTOP_PRINT */
    }

    fclose(fp);
    fp = NULL;

    return;
}  
  
static struct proc_info *find_old_proc(pid_t pid, pid_t tid) {  
    int i;  
  
    for (i = 0; i < num_old_procs; i++)  
        if (old_procs[i] && (old_procs[i]->pid == pid) && (old_procs[i]->tid == tid))  
            return old_procs[i];  
  
    return NULL;  
}  
  
static void free_old_procs(void) {  
    int i;  
  
    for (i = 0; i < num_old_procs; i++)  
        if (old_procs[i])  
            free_proc(old_procs[i]);  
  
    free(old_procs);  
}  
  
static int proc_cpu_cmp(const void *a, const void *b) {  
    struct proc_info *pa, *pb;  
  
    pa = *((struct proc_info **)a); pb = *((struct proc_info **)b);  
  
    if (!pa && !pb) return 0;  
    if (!pa) return 1;  
    if (!pb) return -1;  
  
    return -numcmp(pa->delta_time, pb->delta_time);  
}  
  
static int proc_vss_cmp(const void *a, const void *b) {  
    struct proc_info *pa, *pb;  
  
    pa = *((struct proc_info **)a); pb = *((struct proc_info **)b);  
  
    if (!pa && !pb) return 0;  
    if (!pa) return 1;  
    if (!pb) return -1;  
  
    return -numcmp(pa->vss, pb->vss);  
}  
  
static int proc_rss_cmp(const void *a, const void *b) {  
    struct proc_info *pa, *pb;  
  
    pa = *((struct proc_info **)a); pb = *((struct proc_info **)b);  
  
    if (!pa && !pb) return 0;  
    if (!pa) return 1;  
    if (!pb) return -1;  
  
    return -numcmp(pa->rss, pb->rss);  
}  
  
static int proc_thr_cmp(const void *a, const void *b) {  
    struct proc_info *pa, *pb;  
  
    pa = *((struct proc_info **)a); pb = *((struct proc_info **)b);  
  
    if (!pa && !pb) return 0;  
    if (!pa) return 1;  
    if (!pb) return -1;  
  
    return -numcmp(pa->num_threads, pb->num_threads);  
}  
  
static int numcmp(long long a, long long b) {  
    if (a < b) return -1;  
    if (a > b) return 1;  
    return 0;  
}  
  
static void usage(char *cmd) {  
    fprintf(stderr, "Usage: %s [ -m max_procs ] [ -n iterations ] [ -d delay ] [ -s sort_column ] [ -t ] [ -h ]\n"  
                    "    -m num  Maximum number of processes to display.\n"  
                    "    -n num  Updates to show before exiting.\n"  
                    "    -d num  Seconds to wait between updates.\n"  
                    "    -s col  Column to sort by (cpu,vss,rss,thr).\n"  
                    "    -t      Show threads instead of processes.\n"  
                    "    -h      Display this help screen.\n",  
        cmd);  
}  

