
#include "proto.h"
#include "snmpagt_define.h"
#include "snmpagt.h"

/*--------------------------------------------------------- */
/* Print help message */
static void 
usage(int32 status, const char *argv0)
{
    const char *name = NULL;

    if (!argv0)
    {
        return;
    }

    /* Find the basename */
    name = sal_strrchr(argv0, '/');
    if (name)
        name++;
    else
        name = argv0;

    if (status != 0) {
        sal_printf("Try `%s -h' for more information.\n", name);
    } else {
        sal_printf("Usage: %s d ...\n", name);
        sal_printf("SNMP agent process\n");
        sal_printf("Options:\n");
        sal_printf("\t-d, --debug\tPrint debug\n");
    }
}

int32
main(int32 argc, char **argv)
{
    char *p;
    char *progname;
    uint32 debug = 0;
    static const char *shortopts = "d";

/* 
Command line options. */
#ifdef HAVE_GETOPT_H
struct option longopts[] =
{
 { "debug",      no_argument,       NULL, 'd'},
 { 0 }
};
#endif /* HAVE_GETOPT_H */

    /* preserve my name */
    progname = ((p = sal_strrchr(argv[0], '/')) ? ++p : argv[0]);

    while (1)
    {   
         int32 opt;
   
#ifdef HAVE_GETOPT_H
         opt = getopt_long(argc, argv, shortopts, longopts, 0);
#else
         opt = getopt(argc, argv, shortopts);
#endif /* HAVE_GETOPT_H */
   
         if (EOF == opt)
         {   
             break;
         }
     
         switch (opt)
         {
         case 'd':
             debug = TRUE;
             break;
         default:
             usage(TRUE, progname);
             break;
         }
    }
 
    snmpagt_init(debug);
    exit(0);
}

