#ifndef __CHSM_HOTFIX_H__
#define __CHSM_HOTFIX_H__

#ifdef HAVE_HOTFIX

#include "sal.h"
#include "param_check.h"

#define HOTFIX_BUF_SIZE      512

typedef struct hotfix_info_s
{
    char name[M_FULLPATH_MAX_LEN];
    int id;
    int active;
    int armed;
    char synopsis[HOTFIX_BUF_SIZE];
    char desc[HOTFIX_BUF_SIZE];
    char module[HOTFIX_BUF_SIZE];
    struct list *depend;
}hotfix_info_t;

int chsm_hotfix_config_init (struct cli_tree *pctree);
int chsm_hotfix_config_write(struct cli *pcli);
void chsm_hotfix_show_version_hotfix(struct cli *cli);
#endif
#endif
