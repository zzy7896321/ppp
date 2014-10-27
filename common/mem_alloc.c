#include <stdlib.h>

#include "mem_alloc.h"


unsigned bkdr_hash(const char*);

#define HASH_TABLE_PREFIX mem
#define HASH_TABLE_KEY_TYPE const char*
#define HASH_TABLE_VALUE_TYPE int
#define HASH_TABLE_VALUE_DEFAULT_VALUE 0
#define HASH_TABLE_HASH_FUNCTION(key) bkdr_hash(key)
#define HASH_TABLE_KEY_COMPARATOR(key1, key2) (!strcmp(key1, key2))
#define HASH_TABLE_KEY_DUMP(buffer, buf_size, key) snprintf(buffer, buf_size, "%s", key)
#define HASH_TABLE_VALUE_DUMP(buffer, buf_size, value) snprintf(buffer, buf_size, "%d", value)
#include "hash_table.h"

static mem_t* mem_record = 0;

void mem_profile_init() {
	mem_record = new_mem(4, 0.8);
}

void mem_profile_print() {
	char buffer[8096];
	mem_dump(buffer, 8096, mem_record);
	printf("%s\n", buffer);
}

void mem_profile_destroy() {
	mem_destroy(mem_record);
	mem_record = 0;
}

void* mem_alloc(const char* type_name, size_t size) {
	mem_put(mem_record, type_name, mem_get(mem_record, type_name) + size);
	return malloc(count);
}

void mem_free(const char* type_name, void* ptr, size_t size) {
	free(ptr);
	mem_put(mem_record, type_name, mem_get(mem_record, type_name) - size);
}
