#include "syslog-ng.h"
#include "logmsg.h"

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define TEST_ASSERT(x, format, value, expected)		\
  do 				\
    { 				\
        if (!(x))		\
          {			\
            fprintf(stderr, "Testcase failed; msg='%s', cond='%s', value=" format ", expected=" format "\n", msg, __STRING(x), value, expected); \
            exit(1);		\
          }			\
    }				\
  while (0)

void 
msg_event(gint prio, const char *desc, void *tag1, ...)
{
  ;
}
  
unsigned long 
absolute_value(signed long diff)
{
  if (diff < 0)
    return -diff;
  else
    return diff;
}

int 
testcase(gchar *msg, 
         gint parse_flags,
         gchar *bad_hostname_re,
         gint expected_pri, 
         unsigned long expected_stamp_sec, 
         unsigned long expected_stamp_usec, 
         unsigned long expected_stamp_ofs,
         const gchar *expected_date, 
         const gchar *expected_host, 
         const gchar *expected_program, 
         const gchar *expected_msg)
{
  LogMessage *logmsg;
  time_t now;
  regex_t bad_hostname;
  
  if (bad_hostname_re)
    TEST_ASSERT(regcomp(&bad_hostname, bad_hostname_re, REG_NOSUB | REG_EXTENDED) == 0, "%d", 0, 0);

  logmsg = log_msg_new(msg, strlen(msg), NULL, parse_flags, bad_hostname_re ? &bad_hostname : NULL);
  TEST_ASSERT(logmsg->pri == expected_pri, "%d", logmsg->pri, expected_pri);
  if (expected_stamp_sec)
    {
      TEST_ASSERT(logmsg->stamp.time.tv_sec == expected_stamp_sec, "%d", (int) logmsg->stamp.time.tv_sec, (int) expected_stamp_sec);
      TEST_ASSERT(logmsg->stamp.time.tv_usec == expected_stamp_usec, "%d", (int) logmsg->stamp.time.tv_usec, (int) expected_stamp_usec);
      TEST_ASSERT(logmsg->stamp.zone_offset == expected_stamp_ofs, "%d", (int) logmsg->stamp.zone_offset, (int) expected_stamp_ofs);
    }
  else
    {
      time(&now);
      TEST_ASSERT(absolute_value(logmsg->stamp.time.tv_sec - now) < 1, "%d", 0, 0);
    }
  if (expected_date)
    {
      TEST_ASSERT(strcmp(logmsg->date->str, expected_date) == 0, "%s", logmsg->date->str, expected_date);
    }
  TEST_ASSERT(strcmp(logmsg->host->str, expected_host) == 0, "%s", logmsg->host->str, expected_host);
  TEST_ASSERT(strcmp(logmsg->program->str, expected_program) == 0, "%s", logmsg->program->str, expected_program);
  TEST_ASSERT(strcmp(logmsg->msg->str, expected_msg) == 0, "%s", logmsg->msg->str, expected_msg);
  log_msg_unref(logmsg);
  return 0;
}

int 
main(int argc G_GNUC_UNUSED, char *argv[] G_GNUC_UNUSED)
{
  putenv("TZ=CET");
  tzset();
  testcase("<15> openvpn[2499]: PTHREAD support initialized", 0, NULL,
           15, 			// pri
           0, 0, 0,		// timestamp (sec/usec/zone)
           NULL,                // originally formatted timestamp
           "",		// host
           "openvpn",		// openvpn
           "openvpn[2499]: PTHREAD support initialized" // msg
           );

  testcase("<7>2004-09-07T10:43:21.156+02:00 bzorp openvpn[2499]: PTHREAD support initialized", 0, NULL,
           7, 			// pri
           1094546601, 156000, 7200,	// timestamp (sec/usec/zone)
           "2004-09-07T10:43:21.156+02:00",   // originally formatted timestamp
           "bzorp",		// host
           "openvpn",		// openvpn
           "openvpn[2499]: PTHREAD support initialized" // msg
           );

  testcase("<7>2004-09-07T10:43:21.156+01:00 bzorp openvpn[2499]: PTHREAD support initialized", 0, NULL,
           7, 			// pri
           1094543001, 156000, 3600,	// timestamp (sec/usec/zone)
           "2004-09-07T10:43:21.156+01:00",   // originally formatted timestamp
           "bzorp",		// host
           "openvpn",		// openvpn
           "openvpn[2499]: PTHREAD support initialized" // msg
           );

  testcase("<7>2006-03-26T01:59:59.156+01:00 bzorp openvpn[2499]: PTHREAD support initialized", 0, NULL,
           7, 			// pri
           1143334799, 156000, 3600,	// timestamp (sec/usec/zone)
           NULL,                // originally formatted timestamp
           "bzorp",		// host
           "openvpn",		// openvpn
           "openvpn[2499]: PTHREAD support initialized" // msg
           );

  testcase("<7>2006-03-26T02:00:00.156+01:00 bzorp openvpn[2499]: PTHREAD support initialized", 0, NULL,
           7, 			// pri
           1143334800, 156000, 3600,	// timestamp (sec/usec/zone)
           NULL,                // originally formatted timestamp
           "bzorp",		// host
           "openvpn",		// openvpn
           "openvpn[2499]: PTHREAD support initialized" // msg
           );
  testcase("<7>2006-03-26T03:00:00.156+02:00 bzorp openvpn[2499]: PTHREAD support initialized", 0, NULL,
           7, 			// pri
           1143334800, 156000, 7200,	// timestamp (sec/usec/zone)
           NULL,                // originally formatted timestamp
           "bzorp",		// host
           "openvpn",		// openvpn
           "openvpn[2499]: PTHREAD support initialized" // msg
           );
  testcase("<7>2006-10-29T01:00:00.156+02:00 bzorp openvpn[2499]: PTHREAD support initialized", 0, NULL,
           7, 			// pri
           1162076400, 156000, 7200,	// timestamp (sec/usec/zone)
           NULL,                // originally formatted timestamp
           "bzorp",		// host
           "openvpn",		// openvpn
           "openvpn[2499]: PTHREAD support initialized" // msg
           );
  testcase("<7>2006-10-29T01:59:59.156+02:00 bzorp openvpn[2499]: PTHREAD support initialized", 0, NULL,
           7, 			// pri
           1162079999, 156000, 7200,	// timestamp (sec/usec/zone)
           NULL,                // originally formatted timestamp
           "bzorp",		// host
           "openvpn",		// openvpn
           "openvpn[2499]: PTHREAD support initialized" // msg
           );
  testcase("<7>2006-10-29T02:00:00.156+02:00 bzorp openvpn[2499]: PTHREAD support initialized", 0, NULL,
           7, 			// pri
           1162080000, 156000, 7200,	// timestamp (sec/usec/zone)
           NULL,                // originally formatted timestamp
           "bzorp",		// host
           "openvpn",		// openvpn
           "openvpn[2499]: PTHREAD support initialized" // msg
           );
  
  /* the same in a foreign timezone */
  testcase("<7>2006-10-29T01:00:00.156+01:00 bzorp openvpn[2499]: PTHREAD support initialized", 0, NULL,
           7, 			// pri
           1162072800, 156000, 3600,	// timestamp (sec/usec/zone)
           NULL,                // originally formatted timestamp
           "bzorp",		// host
           "openvpn",		// openvpn
           "openvpn[2499]: PTHREAD support initialized" // msg
           );
  testcase("<7>2006-10-29T01:59:59.156+01:00 bzorp openvpn[2499]: PTHREAD support initialized", 0, NULL,
           7, 			// pri
           1162076399, 156000, 3600,	// timestamp (sec/usec/zone)
           NULL,                // originally formatted timestamp
           "bzorp",		// host
           "openvpn",		// openvpn
           "openvpn[2499]: PTHREAD support initialized" // msg
           );
  testcase("<7>2006-10-29T02:00:00.156+01:00 bzorp openvpn[2499]: PTHREAD support initialized", 0, NULL,
           7, 			// pri
           1162076400, 156000, 3600,	// timestamp (sec/usec/zone)
           NULL,                // originally formatted timestamp
           "bzorp",		// host
           "openvpn",		// openvpn
           "openvpn[2499]: PTHREAD support initialized" // msg
           );
  
  /* check hostname */

  testcase("<7>2006-10-29T02:00:00.156+01:00 %bzorp openvpn[2499]: PTHREAD support initialized", LP_CHECK_HOSTNAME, NULL,
           7, 			// pri
           1162076400, 156000, 3600,	// timestamp (sec/usec/zone)
           NULL,                // originally formatted timestamp
           "",		// host
           "%bzorp openvpn",		// openvpn
           "%bzorp openvpn[2499]: PTHREAD support initialized" // msg
           );

  testcase("<7>2006-10-29T02:00:00.156+01:00 ctld snmpd[2499]: PTHREAD support initialized", 0, "^ctld",
           7, 			// pri
           1162076400, 156000, 3600,	// timestamp (sec/usec/zone)
           NULL,                // originally formatted timestamp
           "",		// host
           "ctld snmpd",		// openvpn
           "ctld snmpd[2499]: PTHREAD support initialized" // msg
           );
  return 0;
}


