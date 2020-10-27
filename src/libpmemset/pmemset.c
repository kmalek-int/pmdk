// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * pmemset.c -- implementation of common pmemset API
 */

#include "pmemset.h"
#include "libpmemset.h"
#include "part.h"
#include "pmemset_utils.h"
#include "ravl.h"
#include "alloc.h"

/*
 * mapping_min
 */
static size_t
mapping_min(void *addr)
{
	struct pmemset_part_map *pmap = (struct pmemset_part_map *)addr;
	return (size_t)pmap->addr;
}

/*
 * mapping_max
 */
static size_t
mapping_max(void *addr)
{
	struct pmemset_part_map *pmap = (struct pmemset_part_map *)addr;
	return (size_t)pmap->addr + pmap->length;
}

/*
 * ravl_interval
 */
struct ravl_interval {
	struct ravl *tree;
	ravl_interval_min *get_min;
	ravl_interval_max *get_max;
};

/*
 * ravl_interval_node
 */
struct ravl_interval_node {
	void *addr;
	ravl_interval_min *get_min;
	ravl_interval_max *get_max;
};

/*
 * ravl_interval_compare
 */
static int
ravl_interval_compare(const void *lhs, const void *rhs){
	const struct ravl_interval_node *left = lhs;
	const struct ravl_interval_node *right = rhs;

	if (left->get_max(left->addr) <= right->get_min(right->addr))
		return -1;
	if (left->get_min(left->addr) >= right->get_max(right->addr))
		return 1;
	return 0;
}

/*
 * ravl_interval_new -- allocate and initialize RAVL tree for pmemset_part_map
 */
struct ravl_interval *
ravl_interval_new(ravl_interval_min *get_min, ravl_interval_max *get_max)
{
	int ret;
	struct ravl_interval *interval = pmemset_malloc(sizeof(*interval), 
                                                        &ret);

	if (ret)
		goto ret_null;

	interval->tree = ravl_new_sized(ravl_interval_compare,
			sizeof(struct ravl_interval_node));
	if (!(interval->tree))
		goto free_alloc;

	interval->get_min = get_min;
	interval->get_max = get_max;

	return interval;

free_alloc:
	Free(interval);
ret_null:
	return NULL;
}

/*
 * ravl_interval_delete
 */
void
ravl_interval_delete(struct ravl_interval *ri)
{
	ravl_delete(ri->tree);
	ri->tree = NULL;
	Free(ri);
}

/*
 * pmemset_set_init -- initialize set structure.
 */
void
pmemset_set_init(struct pmemset *set, struct pmemset_config *config)
{
	ASSERTne(config, NULL);

        /* copy config */
	memcpy(&set->config, config, sizeof(*config));

        /* intialize RAVL */
        set->part_map_tree = ravl_interval_new(mapping_min, mapping_max);        
}

/*
 * pmemset_new -- allocates and initialize pmemset structure.
 */
int
pmemset_new(struct pmemset **set, struct pmemset_config *config)
{
	PMEMSET_ERR_CLR();

	int ret;

        /* allocate set structure */
        *set = pmemset_malloc(sizeof(**set), &ret);

	if (ret)
		return ret;

	ASSERTne(set, NULL);

        /* initialize set */
	pmemset_set_init(*set, config);

	return 0;
}

/*
 * pmemset_delete -- de-allocate set structure
 */
int
pmemset_delete(struct pmemset **set)
{
        /* delete RAVL tree with part_map nodes */
        ravl_interval_delete((*set)->part_map_tree);

	Free(*set);

	*set = NULL;

	return 0;
}

#ifndef _WIN32
/*
 * pmemset_header_init -- not supported
 */
int
pmemset_header_init(struct pmemset_header *header, const char *layout,
		int major, int minor)
{
	return PMEMSET_E_NOSUPP;
}
#else
/*
 * pmemset_header_initU -- not supported
 */
int
pmemset_header_initU(struct pmemset_header *header, const char *layout,
		int major, int minor)
{
	return PMEMSET_E_NOSUPP;
}

/*
 * pmemset_header_initW -- not supported
 */
int
pmemset_header_initW(struct pmemset_header *header, const wchar_t *layout,
		int major, int minor)
{
	return PMEMSET_E_NOSUPP;
}
#endif

/*
 * pmemset_remove_part -- not supported
 */
int
pmemset_remove_part(struct pmemset *set, struct pmemset_part **part)
{
	return PMEMSET_E_NOSUPP;
}

/*
 * pmemset_remove_part_map -- not supported
 */
int
pmemset_remove_part_map(struct pmemset *set, struct pmemset_part_map **part)
{
	return PMEMSET_E_NOSUPP;
}

/*
 * pmemset_remove_range -- not supported
 */
int
pmemset_remove_range(struct pmemset *set, void *addr, size_t len)
{
	return PMEMSET_E_NOSUPP;
}

/*
 * pmemset_persist -- not supported
 */
int
pmemset_persist(struct pmemset *set, const void *ptr, size_t size)
{
	return PMEMSET_E_NOSUPP;
}

/*
 * pmemset_flush -- not supported
 */
int
pmemset_flush(struct pmemset *set, const void *ptr, size_t size)
{
	return PMEMSET_E_NOSUPP;
}

/*
 * pmemset_drain -- not supported
 */
int
pmemset_drain(struct pmemset *set)
{
	return PMEMSET_E_NOSUPP;
}

/*
 * pmemset_memmove -- not supported
 */
int
pmemset_memmove(struct pmemset *set, void *pmemdest, const void *src,
		size_t len, unsigned flags)
{
	return PMEMSET_E_NOSUPP;
}

/*
 * pmemset_memcpy -- not supported
 */
int
pmemset_memcpy(struct pmemset *set, void *pmemdest, const void *src, size_t len,
		unsigned flags)
{
	return PMEMSET_E_NOSUPP;
}

/*
 * pmemset_memset -- not supported
 */
int
pmemset_memset(struct pmemset *set, void *pmemdest, int c, size_t len,
		unsigned flags)
{
	return PMEMSET_E_NOSUPP;
}

/*
 * pmemset_deep_flush -- not supported
 */
int
pmemset_deep_flush(struct pmemset *set, void *ptr, size_t size)
{
	return PMEMSET_E_NOSUPP;
}
