

#define HASH_TABLE_PREFIX ihtab
#define HASH_TABLE_KEY_TYPE int
#define HASH_TABLE_VALUE_TYPE int
#define HASH_TABLE_DECLARE_DUMP 1
#define HASH_TABLE_DECLARE_ONLY 1
#include "hash_table.h"

unsigned int_hash(int x) {
	return (((unsigned) x) << 2) +  (((unsigned) x) >> 2);
}

#define HASH_TABLE_PREFIX ihtab
#define HASH_TABLE_KEY_TYPE int
#define HASH_TABLE_VALUE_TYPE int
#define HASH_TABLE_VALUE_DEFAULT_VALUE -1
#define HASH_TABLE_HASH_FUNCTION(key) int_hash(key)
#define HASH_TABLE_KEY_DUMP(buffer, buf_size, key) snprintf(buffer, buf_size, "%d", key)
#define HASH_TABLE_VALUE_DUMP(buffer, buf_size, value) snprintf(buffer, buf_size, "%d", value)
#include "hash_table.h"

struct A {
	int x;
};

unsigned bkdr_hash(const char* str) {
	unsigned hash = 0;
	for ( ; *str != '\0'; ++str) {
		hash = (hash * 131 + *str);
	}
	return hash;
}

#define HASH_TABLE_PREFIX shtab
#define HASH_TABLE_KEY_TYPE char*
#define HASH_TABLE_VALUE_TYPE struct A*
#define HASH_TABLE_HASH_FUNCTION(key) bkdr_hash(key)
#define HASH_TABLE_KEY_COMPARATOR(key1, key2) !strcmp(key1, key2)
#define HASH_TABLE_VALUE_DEFAULT_VALUE 0
#define HASH_TABLE_KEY_DUMP(buffer, buf_size, str) snprintf(buffer, buf_size, "%s", str)
#define HASH_TABLE_VALUE_DUMP(buffer, buf_size, value) snprintf(buffer, buf_size, "%d", (value) ? value->x : -1)
#define HASH_TABLE_KEY_CLONE(var, key) var = strdup(key)
#define HASH_TABLE_VALUE_CLONE(var, value) if (value) {var = malloc(sizeof(struct A)); var->x = value->x; } else var = 0
#define HASH_TABLE_KEY_DESTRUCTOR(key) free(key)
#define HASH_TABLE_VALUE_DESTRUCTOR(value) if (value) free(value)
#include "hash_table.h"

#define BUF_SIZE 8096
char buffer[BUF_SIZE];

int main() {
	size_t i;	

	ihtab_t* ihtab = new_ihtab(2, 0.8);
	for (i = 0; i < 10; ++i) {
		ihtab_put(ihtab, i, i + 100);
	}
	ihtab_dump(buffer, BUF_SIZE, ihtab);
	printf("ihtab_empty(ihtab) = %d\n", ihtab_empty(ihtab));
	printf("%s\n", buffer);

	printf("ihtab_empty(ihtab) = %d\n", ihtab_empty(ihtab));
	printf("ihtab[%d] = %d\n", 5, ihtab_find(ihtab, 5));
	printf("ihtab[%d] = %d\n", 7, ihtab_find(ihtab, 7));
	printf("ihtab[%d] = %d\n", 0, ihtab_find(ihtab, 0));

	printf("ihtab_remove(ihtab, %d) = %d\n", 5, ihtab_remove(ihtab, 5));

	printf("ihtab[%d] = %d\n", 5, ihtab_find(ihtab, 5));
	printf("ihtab[%d] = %d\n", 7, ihtab_find(ihtab, 7));
	printf("ihtab[%d] = %d\n", 0, ihtab_find(ihtab, 0));

	ihtab_dump(buffer, BUF_SIZE, ihtab);
	printf("%s\n", buffer);

	printf("ihtab_get(5) = %d\n", ihtab_get(ihtab, 5));
	ihtab_dump(buffer, BUF_SIZE, ihtab);
	printf("%s\n", buffer);

	ihtab_clear(ihtab);
	
	printf("ihtab_empty(ihtab) = %d\n", ihtab_empty(ihtab));
	ihtab_dump(buffer, BUF_SIZE, ihtab);
	printf("%s\n", buffer);

	ihtab_destroy(ihtab);

	struct A a;
	char name[100];
	shtab_t* shtab = new_shtab(2, 0.8);
	for (i = 0; i < 10; ++i) {
		snprintf(name, 100, "node %d", i);
		a.x = i + 100;
		shtab_put(shtab, name, &a);
	}
	shtab_dump(buffer, BUF_SIZE, shtab);
	printf("shtab_empty(shtab) = %d\n", shtab_empty(shtab));
	printf("%s\n", buffer);

	printf("shtab_empty(shtab) = %d\n", shtab_empty(shtab));
	printf("shtab[%s] = %d\n", "node 5", shtab_find(shtab, "node 5"));
	printf("shtab[%s] = %d\n", "node 7", shtab_find(shtab, "node 7"));
	printf("shtab[%s] = %d\n", "node 0", shtab_find(shtab, "node 0"));

	printf("shtab_remove(shtab, %s) = %d\n", "node 5", shtab_remove(shtab, "node 5"));

	printf("shtab[%s] = %d\n", "node 5", shtab_find(shtab, "node 5"));
	printf("shtab[%s] = %d\n", "node 7", shtab_find(shtab, "node 7"));
	printf("shtab[%s] = %d\n", "node 0", shtab_find(shtab, "node 0"));

	shtab_dump(buffer, BUF_SIZE, shtab);
	printf("%s\n", buffer);

	printf("shtab_get(\"node 5\") = %d\n", shtab_get(shtab, "node 5"));
	shtab_dump(buffer, BUF_SIZE, shtab);
	printf("%s\n", buffer);

	shtab_clear(shtab);
	
	printf("shtab_empty(shtab) = %d\n", shtab_empty(shtab));
	shtab_dump(buffer, BUF_SIZE, shtab);
	printf("%s\n", buffer);

	shtab_destroy(shtab);

	return 0;
}


