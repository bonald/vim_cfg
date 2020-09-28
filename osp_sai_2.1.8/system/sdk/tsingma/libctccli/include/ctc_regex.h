#ifndef REGEX_H
#define REGEX_H

#include "sal.h"

extern char *ctc_re_comp(char *);
extern int32 ctc_re_exec(char *);
extern void ctc_re_modw(char *);
extern int32 ctc_re_subs(char *, char *);

#endif /* REGEX_H */
