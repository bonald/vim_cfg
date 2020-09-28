/*
 * (C) 2013 by Ana Rey Botello <anarey@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <linux/netfilter/nf_tables.h>
#include <libmnl/libmnl.h>
#include <libnftnl/rule.h>
#include <libnftnl/expr.h>

static int test_ok = 1;

static void print_err(const char *msg)
{
	printf("\033[31mERROR:\e[0m %s\n", msg);
	test_ok = 0;
}

static void cmp_nftnl_expr(struct nftnl_expr *rule_a,
			      struct nftnl_expr *rule_b)
{
	if (nftnl_expr_get_u32(rule_a, NFTNL_EXPR_LOG_SNAPLEN) !=
	    nftnl_expr_get_u32(rule_b, NFTNL_EXPR_LOG_SNAPLEN))
		print_err("Expr NFTNL_EXPR_LOG_SNAPLEN mismatches");
	if (nftnl_expr_get_u16(rule_a, NFTNL_EXPR_LOG_GROUP) !=
	    nftnl_expr_get_u16(rule_b, NFTNL_EXPR_LOG_GROUP))
		print_err("Expr NFTNL_EXPR_LOG_GROUP mismatches");
	if (nftnl_expr_get_u16(rule_a, NFTNL_EXPR_LOG_QTHRESHOLD) !=
	    nftnl_expr_get_u16(rule_b, NFTNL_EXPR_LOG_QTHRESHOLD))
		print_err("Expr NFTNL_EXPR_LOG_QTHRESHOLD mismatches");
	if(strcmp(nftnl_expr_get_str(rule_a, NFTNL_EXPR_LOG_PREFIX),
		  nftnl_expr_get_str(rule_b, NFTNL_EXPR_LOG_PREFIX)) != 0)
		print_err("Expr NFTNL_EXPR_LOG_PREFIX mismatches");
}

int main(int argc, char *argv[])
{
	struct nftnl_rule *a, *b;
	struct nftnl_expr *ex;
	struct nlmsghdr *nlh;
	char buf[4096];
	struct nftnl_expr_iter *iter_a, *iter_b;
	struct nftnl_expr *rule_a, *rule_b;

	a = nftnl_rule_alloc();
	b = nftnl_rule_alloc();
	if (a == NULL || b == NULL)
		print_err("OOM");
	ex = nftnl_expr_alloc("log");
	if (ex == NULL)
		print_err("OOM");

	nftnl_expr_set_u32(ex, NFTNL_EXPR_LOG_SNAPLEN, 0x12345678);
	nftnl_expr_set_u16(ex, NFTNL_EXPR_LOG_GROUP, 0x1234);
	nftnl_expr_set_u16(ex, NFTNL_EXPR_LOG_QTHRESHOLD, 0x3412);
	nftnl_expr_set_str(ex, NFTNL_EXPR_LOG_PREFIX, "test");

	nftnl_rule_add_expr(a, ex);

	nlh = nftnl_rule_nlmsg_build_hdr(buf, NFT_MSG_NEWRULE, AF_INET, 0, 1234);
	nftnl_rule_nlmsg_build_payload(nlh, a);
	if (nftnl_rule_nlmsg_parse(nlh, b) < 0)
		print_err("parsing problems");

	iter_a = nftnl_expr_iter_create(a);
	iter_b = nftnl_expr_iter_create(b);
	if (iter_a == NULL || iter_b == NULL)
		print_err("OOM");
	rule_a = nftnl_expr_iter_next(iter_a);
	rule_b = nftnl_expr_iter_next(iter_b);
	if (rule_a == NULL || rule_b == NULL)
		print_err("OOM");

	cmp_nftnl_expr(rule_a, rule_b);

	if (nftnl_expr_iter_next(iter_a) != NULL ||
	    nftnl_expr_iter_next(iter_b) != NULL)
		print_err("More 1 expr.");

	nftnl_expr_iter_destroy(iter_a);
	nftnl_expr_iter_destroy(iter_b);
	nftnl_rule_free(a);
	nftnl_rule_free(b);

	if (!test_ok)
		exit(EXIT_FAILURE);

	printf("%s: \033[32mOK\e[0m\n", argv[0]);
	return EXIT_SUCCESS;
}
