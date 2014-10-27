#ifndef COMMON_MEM_PROFILE_H
#define COMMON_MEM_PROFILE_H

#include "macro_util.h"
#include <stddef.h>

#define PROFILE_MEM_ALLOC(type, count)	\
	(type*) mem_alloc(STRINGIFY(type), sizeof(type), count)

#define PROFILE_MEM_FREE(type, ptr, count)	\
	mem_free(STRINGIFY(type), ptr, sizeof(type), count)


void mem_profile_init();

void* mem_alloc(const char* type_name, size_t type_size, int count);

void mem_free(const char* type_name, void* ptr, size_t type_size, int count);

void mem_profile_print();

void mem_profile_destroy();

#endif
