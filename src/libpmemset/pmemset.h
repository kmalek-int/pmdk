/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * pmemset.h -- internal definitions for libpmemset
 */
#ifndef PMEMSET_H
#define PMEMSET_H

#include "libpmemset.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PMEMSET_MAJOR_VERSION 0
#define PMEMSET_MINOR_VERSION 0

#define PMEMSET_LOG_PREFIX "libpmemset"
#define PMEMSET_LOG_LEVEL_VAR "PMEMSET_LOG_LEVEL"
#define PMEMSET_LOG_FILE_VAR "PMEMSET_LOG_FILE"

struct pmemset {
        struct pmemset_config config;           /* configuration */
        struct ravl_interval *part_map_tree;    /* part_map RAVL tree */
};

struct pmemset_header {
	char stub;
};

struct ravl_interval;
struct ravl_interval_node;

typedef size_t ravl_interval_min(void *addr);
typedef size_t ravl_interval_max(void *addr);

struct ravl_interval *ravl_interval_new(ravl_interval_min *min,
		ravl_interval_min *max);

void
ravl_interval_delete(struct ravl_interval *ri);

void
pmemset_set_init(struct pmemset *set, struct pmemset_config *config);


#ifdef __cplusplus
}
#endif

#endif
