#include "hash_table_impl.h"
#include <stdio.h>

#define HASH_TABLE_HASH_FUNCTION(key) (key)
#define HASH_TABLE_COMPARATOR(key1, key2) (key1 == key2)
#define HASH_TABLE_DESTROY_KEY(key) 
#define HASH_TABLE_DESTROY_VALUE(value) 
#define HASH_TABLE_KEY_NOT_FOUND_VALUE 0
#define HASH_TABLE_VALUE_DEFAULT 0
DEFINE_HASH_TABLE(int, int, int);
#undef HASH_TABLE_HASH_FUNCTION
#undef HASH_TABLE_COMPARATOR
#undef HASH_TABLE_DESTROY_KEY
#undef HASH_TABLE_DESTROY_VALUE
#undef HASH_TABLE_KEY_NOT_FOUND_VALUE

unsigned bkdr_hash(char* str) {
	unsigned hash = 0;
	for ( ; *str != '\0'; ++str) {
		hash = (hash * 131 + *str);
	}
	return hash;
}

#define HASH_TABLE_HASH_FUNCTION(key) (bkdr_hash(key))
#define HASH_TABLE_COMPARATOR(key1, key2) (!strcmp(key1, key2))
#define HASH_TABLE_DESTROY_KEY(key) 
#define HASH_TABLE_DESTROY_VALUE(value) 
#define HASH_TABLE_KEY_NOT_FOUND_VALUE -1
#define HASH_TABLE_VALUE_DEFAULT 0
DEFINE_HASH_TABLE(str, char*, int);
#undef HASH_TABLE_HASH_FUNCTION
#undef HASH_TABLE_COMPARATOR
#undef HASH_TABLE_DESTROY_KEY
#undef HASH_TABLE_DESTROY_VALUE
#undef HASH_TABLE_KEY_NOT_FOUND_VALUE

int main() {
	printf("int_hash_table\n");
	int_hash_table_t* hash_table = new_int_hash_table(0xFFFF);

	int_hash_table_put(hash_table, 1, 100);
	int_hash_table_put(hash_table, 2, 200);
	int_hash_table_put(hash_table, 123, 12300);

	printf("%d %d %d %d\n", int_hash_table_get(hash_table, 1),  int_hash_table_get(hash_table, 2),
			int_hash_table_get(hash_table, 123), int_hash_table_get(hash_table, 65537));

	int_hash_table_remove(hash_table, 123);
	int_hash_table_put(hash_table, 65537, -1);

	printf("%d %d %d %d\n", int_hash_table_get(hash_table, 1),  int_hash_table_get(hash_table, 2),
			int_hash_table_get(hash_table, 123), int_hash_table_get(hash_table, 65537));

	printf("%d\n", int_hash_table_size(hash_table));

	int_hash_table_destroy(hash_table);

	printf("str_hash_table\n");
	str_hash_table_t* ht = new_str_hash_table(0xFFFF);

	str_hash_table_put(ht, "x", 1);
	str_hash_table_put(ht, "y", 2);
	str_hash_table_put(ht, "flip", 0);

	printf("%d %d %d %d\n", str_hash_table_get(ht, "x"),
							str_hash_table_get(ht, "y"),
							str_hash_table_get(ht, "flip"),
							str_hash_table_get(ht, "z"));

	str_hash_table_remove(ht, "x");
	str_hash_table_put(ht, "z", 123);
	printf("%d %d %d %d\n", str_hash_table_get(ht, "x"),
							str_hash_table_get(ht, "y"),
							str_hash_table_get(ht, "flip"),
							str_hash_table_get(ht, "z"));

	printf("%d\n", str_hash_table_size(ht));
	str_hash_table_destroy(ht);

	return 0;
}