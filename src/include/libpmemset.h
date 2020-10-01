/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2020, Intel Corporation */

/*
 * libpmemset.h -- definitions of libpmemset entry points (EXPERIMENTAL)
 *
 * See libpmemset(7) for details.
 */

#ifndef LIBPMEMSET_H
#define LIBPMEMSET_H 1

#include <stddef.h>

#ifdef _WIN32
#include <pmemcompat.h>

#ifndef PMDK_UTF8_API
#define pmemset_config_set_layout_name pmemset_config_set_layout_nameW
#define pmemset_header_init pmemset_header_initW
#define pmemset_source_from_file pmemset_source_from_fileW
#define pmemset_source_from_file_params pmemset_source_from_file_paramsW
#define pmemset_source_from_temporary pmemset_source_from_temporaryW
#else
#define pmemset_config_set_layout_name pmemset_config_set_layout_nameU
#define pmemset_header_init pmemset_header_initU
#define pmemset_source_from_file pmemset_source_from_fileU
#define pmemset_source_from_file_params pmemset_source_from_file_paramsU
#define pmemset_source_from_temporary pmemset_source_from_temporaryU
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PMEMSET_E_UNKNOWN			(-200000)
#define PMEMSET_E_NOSUPP			(-200001)

struct pmemset;
struct pmemset_header;

/* event setup */

struct pmemset_event_context;

typedef int pmemset_event_callback(struct pmemset *set,
		struct pmemset_event_context *context, void *arg);

/* config setup */

struct pmem2_vm_reservation;
struct pmemset_config;

int pmemset_config_new(struct pmemset_config **config);

int pmemset_config_set_create_if_none(struct pmemset_config *config,
		int value);

int pmemset_config_set_create_if_invalid(struct pmemset_config *config,
		int value);

int pmemset_config_set_event_callback(struct pmemset_config *config,
		pmemset_event_callback *callback, void *arg);

int pmemset_config_set_reservation(struct pmemset_config *config,
		struct pmem2_vm_reservation *rsv);

int pmemset_config_set_contiguous_part_coalescing(
		struct pmemset_config *config, int value);

#ifndef _WIN32
int pmemset_config_set_layout_name(struct pmemset_config *config,
		const char *layout);
#else
int pmemset_config_set_layout_nameU(struct pmemset_config *config,
		const char *layout);

int pmemset_config_set_layout_nameW(struct pmemset_config *config,
		const wchar_t *layout);
#endif

int pmemset_config_set_version(struct pmemset_config *config,
		int major, int minor);

int pmemset_config_delete(struct pmemset_config **config);

/* source setup */

struct pmem2_source;
struct pmemset_source;

int pmemset_source_from_external(struct pmemset_source **source,
	struct pmem2_source *ext_source);

#ifndef WIN32
int pmemset_source_from_file(struct pmemset_source **source, const char *file);

int pmemset_source_from_temporary(struct pmemset_source **source,
		const char *dir);
#else
int pmemset_source_from_fileU(struct pmemset_source **source, const char *file);

int pmemset_source_from_fileW(struct pmemset_source **source,
		const wchar_t *file);

int pmemset_source_from_file_paramsU(struct pmemset_source **source,
		const wchar_t *file);

int pmemset_source_from_file_paramsW(struct pmemset_source **source,
		const wchar_t *file);

int pmemset_source_from_temporaryU(struct pmemset_source **source,
		const char *dir);

int pmemset_source_from_temporaryW(struct pmemset_source **source,
		const wchar_t *dir);
#endif

int pmemset_source_fallocate(struct pmemset_source *source, int flags,
		size_t offset, size_t size);

int pmemset_source_delete(struct pmemset_source **source);

/* part setup */

struct pmemset_part;
struct pmemset_part_descriptor;
struct pmemset_part_map;
struct pmemset_part_shutdown_state_data;

enum pmemset_part_state {
	/*
	 * The pool state cannot be determined because of errors during
	 * retrieval of device information.
	 */
	PMEMSET_PART_STATE_INDETERMINATE,

	/*
	 * The pool is internally consistent and was closed cleanly.
	 * Application can assume that no custom recovery is needed.
	 */
	PMEMSET_PART_STATE_OK,

	/*
	 * The pool is internally consistent, but it was not closed cleanly.
	 * Application must perform consistency checking and custom recovery
	 * on user data.
	 */
	PMEMSET_PART_STATE_OK_BUT_INTERRUPTED,

	/*
	 * The pool can contain invalid data as a result of hardware failure.
	 * Reading the pool is unsafe.
	 */
	PMEMSET_PART_STATE_CORRUPTED,
};

int pmemset_part_descriptor_new(struct pmemset_part_descriptor **part,
		struct pmemset *set, struct pmemset_source *source,
		size_t offset, size_t length);

int pmemset_part_descriptor_pread_mcsafe(struct pmemset_part_descriptor *part,
		void *dst, size_t size, size_t offset);

int pmemset_part_descriptor_pwrite_mcsafe(struct pmemset_part_descriptor *part,
		void *dst, size_t size, size_t offset);

int pmemset_part_finalize(struct pmemset *set,
		struct pmemset_part_descriptor *descriptor,
		const struct pmemset_header *header_in,
		struct pmemset_header *header_out,
		const struct pmemset_part_shutdown_state_data *sds_in,
		struct pmemset_part_shutdown_state_data *sds_out,
		enum pmemset_part_state *state);

int pmemset_part_address(struct pmemset_part *part);

int pmemset_part_length(struct pmemset_part *part);

int pmemset_part_first(struct pmemset *set, struct pmemset_part **part);

int pmemset_part_next(struct pmemset *set, struct pmemset_part **part);

int pmemset_part_by_address(struct pmemset *set, struct pmemset_part **part,
		void *addr);

int pmemset_part_drop(struct pmemset_part *part);

int pmemset_part_map_new(struct pmemset_part_map **pmap,
		struct pmemset_part **part,
		const struct pmemset_header *header_in,
		struct pmemset_header *header_out,
		const struct pmemset_part_shutdown_state_data *sds_in,
		struct pmemset_part_shutdown_state_data *sds_out,
		enum pmemset_part_state *state);

int pmemset_part_map_drop(struct pmemset_part_map **pmap);

int pmemset_part_map_address(struct pmemset_part_map *pmap);

int pmemset_part_map_length(struct pmemset_part_map *pmap);

int pmemset_part_map_first(struct pmemset *set, struct pmemset_part_map **pmap);

int pmemset_part_map_next(struct pmemset *set, struct pmemset_part_map **pmap);

/* pmemset setup */

#define PMEMSET_F_MEM_NODRAIN	(1U << 0)

#define PMEMSET_F_MEM_NONTEMPORAL	(1U << 1)
#define PMEMSET_F_MEM_TEMPORAL	(1U << 2)

#define PMEMSET_F_MEM_WC		(1U << 3)
#define PMEMSET_F_MEM_WB		(1U << 4)

#define PMEMSET_F_MEM_NOFLUSH	(1U << 5)

#define PMEMSET_F_MEM_VALID_FLAGS (PMEMSET_F_MEM_NODRAIN | \
		PMEMSET_F_MEM_NONTEMPORAL | \
		PMEMSET_F_MEM_TEMPORAL | \
		PMEMSET_F_MEM_WC | \
		PMEMSET_F_MEM_WB | \
		PMEMSET_F_MEM_NOFLUSH)

int pmemset_new(struct pmemset **set, struct pmemset_config *config);

int pmemset_delete(struct pmemset **set);

#ifndef WIN32
int pmemset_header_init(struct pmemset_header *header, const char *layout,
		int major, int minor);
#else
int pmemset_header_initU(struct pmemset_header *header, const char *layout,
		int major, int minor);

int pmemset_header_initW(struct pmemset_header *header, const wchar_t *layout,
		int major, int minor);
#endif

int pmemset_remove_part(struct pmemset *set, struct pmemset_part **part);

int pmemset_remove_part_map(struct pmemset *set,
		struct pmemset_part_map **part);

int pmemset_remove_range(struct pmemset *set, void *addr, size_t len);

int pmemset_persist(struct pmemset *set, const void *ptr, size_t size);

int pmemset_flush(struct pmemset *set, const void *ptr, size_t size);

int pmemset_drain(struct pmemset *set);

int pmemset_memmove(struct pmemset *set, void *pmemdest, const void *src,
		size_t len, unsigned flags);

int pmemset_memcpy(struct pmemset *set, void *pmemdest, const void *src,
		size_t len, unsigned flags);

int pmemset_memset(struct pmemset *set, void *pmemdest, int c, size_t len,
		unsigned flags);

int pmemset_deep_flush(struct pmemset *set, void *ptr, size_t size);

#ifdef __cplusplus
}
#endif
#endif	/* libpmemset.h */
