#include "hash_table_impl.h"
#include "hash_table.h"

#include <string.h>
#include <stdlib.h>

unsigned bkdr_hash(const char* str) {
	unsigned hash = 0;
	for ( ; *str != '\0'; ++str) {
		hash = (hash * 131 + *str);
	}
	return hash;
}

struct pp_variable_t;
void pp_variable_destroy(struct pp_variable_t* value);

#define HASH_TABLE_HASH_FUNCTION(key) (bkdr_hash(key))
#define HASH_TABLE_COMPARATOR(key1, key2) (!strcmp(key1, key2))
#define HASH_TABLE_DESTROY_KEY(key) 
#define HASH_TABLE_DESTROY_VALUE(value) pp_variable_destroy(value)
#define HASH_TABLE_KEY_NOT_FOUND_VALUE 0
#define HASH_TABLE_VALUE_DEFAULT 0
#define HASH_TABLE_DUMP_KEY(buffer, buf_size, key) snprintf(buffer, buf_size, key)
#define HASH_TABLE_DUMP_VALUE(buffer, buf_size, value) pp_variable_dump(value, buffer, buf_size)
#define HASH_TABLE_CLONE_KEY(key) key
/* memory leak */
#define HASH_TABLE_CLONE_VALUE(value) pp_variable_clone(value)
DEFINE_HASH_TABLE(variable, const char*, struct pp_variable_t*)
#undef HASH_TABLE_HASH_FUNCTION
#undef HASH_TABLE_COMPARATOR
#undef HASH_TABLE_DESTROY_KEY
#undef HASH_TABLE_DESTROY_VALUE
#undef HASH_TABLE_KEY_NOT_FOUND_VALUE
#undef HASH_TABLE_VALUE_DEFAULT
#undef HASH_TABLE_DUMP_KEY
#undef HASH_TABLE_DUMP_VALUE
#undef HASH_TABLE_CLONE_KEY
#undef HASH_TABLE_CLONE_VALUE
