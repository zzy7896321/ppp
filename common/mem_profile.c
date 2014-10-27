#include <stdlib.h>

#include "mem_profile.h"

typedef struct mem_rec {
	size_t type_size;
	int count;
} mem_rec;

mem_rec* new_mem_rec() {
	mem_rec* rec = malloc(sizeof(mem_rec));
	rec->type_size = 0;
	rec->count = 0;
	return rec;
}

void mem_rec_destroy(mem_rec* rec) {
	free(rec);
}

unsigned bkdr_hash(const char*);

#define HASH_TABLE_PREFIX mem
#define HASH_TABLE_KEY_TYPE const char*
#define HASH_TABLE_VALUE_TYPE mem_rec*
#define HASH_TABLE_VALUE_DEFAULT_VALUE new_mem_rec()
#define HASH_TABLE_HASH_FUNCTION(key) bkdr_hash(key)
#define HASH_TABLE_KEY_COMPARATOR(key1, key2) (!strcmp(key1, key2))
#define HASH_TABLE_KEY_DUMP(buffer, buf_size, key) snprintf(buffer, buf_size, "%s", key)
#define HASH_TABLE_VALUE_DUMP(buffer, buf_size, value) snprintf(buffer, buf_size, "%d (%d bytes)", value->count, value->type_size * value->count)
#define HASH_TABLE_VALUE_DESTROY(value) mem_rec_destroy(value)
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

void* mem_alloc(const char* type_name, size_t type_size, int count) {
	mem_rec* rec = mem_get(mem_record, type_name);
	rec->type_size = type_size;
	rec->count += count;
	return malloc(type_size * count);
}

void mem_free(const char* type_name, void* ptr, size_t type_size, int count) {
	free(ptr);
	mem_rec* rec = mem_get(mem_record, type_name);
	rec->type_size = type_size;
	rec->count -= count;
}
