// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/*
 * basic.c -- simple example for the libpmemset
 */

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif
#include <libpmem2.h>
#include <libpmemset.h>

#define USAGE_STR_INFO "usage: %s <file> [<granularity>]\n" \
	"Where available granualrity options are:\n" \
	"\tpage - page granularity (default)\n" \
	"\tcacheline - cacheline granularity\n" \
	"\tbyte - byte granularity\n"

#define CFG_GRANULARITY_DEFAULT		PMEM2_GRANULARITY_PAGE
#define PART_SIZE			4096
#define PMEMSET_FILE_OPEN		1

enum user_config_granularity_type {
	USER_CFG_GRANULARITY_PAGE,
	USER_CFG_GRANULARITY_CACHE_LINE,
	USER_CFG_GRANULARITY_BYTE,

	USER_CFG_GRANULARITY_MAX
};

static struct user_granularity_config {
	const char *string;
	enum pmem2_granularity granularity;
} user_granularity_configs[USER_CFG_GRANULARITY_MAX] = {
	[USER_CFG_GRANULARITY_PAGE] = { .string = "page",
		.granularity = PMEM2_GRANULARITY_PAGE },
	[USER_CFG_GRANULARITY_CACHE_LINE] = { .string = "cacheline",
		.granularity = PMEM2_GRANULARITY_CACHE_LINE },
	[USER_CFG_GRANULARITY_BYTE] = { .string = "byte",
		.granularity = PMEM2_GRANULARITY_BYTE },
};

/*
 * user_data_operation_parse -- parses the user data operation
 */
static enum pmem2_granularity
user_granularity_config_parse(char *arg_str)
{
	for (int i = 0; i < USER_CFG_GRANULARITY_MAX; ++i) {
		if (strcmp(user_granularity_configs[i].string, arg_str) == 0)
			return user_granularity_configs[i].granularity;
	}
	fprintf(stderr,
		"Incorrect granularity option. Setting to default.\n");
	return CFG_GRANULARITY_DEFAULT;
}

int
main(int argc, char *argv[])
{
	struct pmemset_source *src;
	struct pmemset_config *cfg;
	struct pmemset *set;
	struct pmemset_part *part;
	struct pmemset_part_descriptor desc;

	/* parse and validate arguments */
	if (argc < 2) {
		fprintf(stderr, USAGE_STR_INFO, argv[0]);
		exit(1);
	}
	
	char *file = argv[1];
	int ret;

#ifdef PMEMSET_FILE_OPEN
	/* create source*/
	ret = pmemset_source_from_file(&src, file);
	if (ret) {
		pmemset_perror("pmemset_source_from_file");
		exit(1);
	}
#else
	int fd;
	struct pmem2_source *pmem2_src;
	
	if ((fd = open(file, O_RDWR)) < 0) {
		perror("open");
		exit(1);
	}
	
	if (pmem2_source_from_fd(&pmem2_src, fd)) {
		pmem2_perror("pmem2_source_from_fd");
		exit(1);
	}

	if (pmemset_source_from_pmem2(&src, pmem2_src)) {
		pmemset_perror("pmemset_source_from_pmem2");
		exit(1);
	}
#endif
	/* create config */
	ret = pmemset_config_new(&cfg);
	if (ret) {
		pmemset_perror("pmemset_config_new");
		goto exit_src_del;
	}

	/* config granularity*/
	enum pmem2_granularity cfg_granularity;
	if (argc > 2) {
		
		cfg_granularity =  user_granularity_config_parse(argv[2]);
	}
	else {
		cfg_granularity = CFG_GRANULARITY_DEFAULT;
	}

	ret = pmemset_config_set_required_store_granularity(cfg,
							cfg_granularity);

	if (ret) {
		pmemset_perror("pmemset_config_set_required_store_granularity");
		goto exit_cfg_del;
	}
	
	/* create set */
	ret = pmemset_new(&set, cfg);
	if (ret) {
		pmemset_perror("pmemset_new");
		goto exit_cfg_del;
	}

	/* create part */
	ret = pmemset_part_new(&part, set, src, 0, PART_SIZE);
	if (ret) {
		pmemset_perror("pmemset_part_new");
		goto exit;
	}

	/* map the part */
	ret = pmemset_part_map(&part, NULL, &desc);
	if (ret) {
		pmemset_perror("pmemset_part_map");
		goto exit;
	}

	strcpy(desc.addr, "PMDK libpmemset usage example application");

	pmemset_persist(set, desc.addr, desc.size);
	
exit:
	pmemset_delete(&set);
exit_cfg_del:
	pmemset_config_delete(&cfg);
exit_src_del:
	pmemset_source_delete(&src);

	return ret;
}