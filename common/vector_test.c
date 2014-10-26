#include <stdio.h>
#include <string.h>

#include "macro_util.h"

int dump_string(char* buffer, int buf_size, char* str) {
	if (!str) return snprintf(buffer, buf_size, "NULL");
	return snprintf(buffer, buf_size, "%s", str);
}

#define VECTOR_PREFIX ivec
#define VECTOR_VALUE_TYPE int
#define VECTOR_VALUE_DEFAULT_VALUE 0
#define VECTOR_VALUE_DUMP(buffer, buf_size, value) snprintf(buffer, buf_size, "%d", value)
#include "vector.h"

#define VECTOR_PREFIX svec
#define VECTOR_VALUE_TYPE char*
#define VECTOR_VALUE_DEFAULT_VALUE 0
#define VECTOR_VALUE_CLONE(var, value) var = strdup(value)
#define VECTOR_VALUE_DESTRUCTOR(var) free(var)
#define VECTOR_VALUE_DUMP(buffer, buf_size, value) dump_string(buffer, buf_size, value)
#include "vector.h"

#include <assert.h>

int main() {
	int i;
	ivec_t* vec = new_ivec(100);
	for (i = 0; i < 10000; ++i) {
		assert(ivec_push_back(vec, i));
	}

	printf("size = %u, empty = %d, capacity = %u\n", ivec_size(vec), ivec_empty(vec), ivec_capacity(vec));
	printf("popback = %d\n", ivec_pop_back(vec));
	printf("size = %u, empty = %d, capacity = %u\n", ivec_size(vec), ivec_empty(vec), ivec_capacity(vec));

	int* d = ivec_data(vec);
	for (i = 0; i < 9999; ++i) {
		assert(*(d+i) == i && i == ivec_at(vec, i));
	}

	printf("resize to 10, and shrink_to_fit\n");
	ivec_resize_default(vec, 10);
	ivec_shrink_to_fit(vec);
	printf("size = %u, empty = %d, capacity = %u\n", ivec_size(vec), ivec_empty(vec), ivec_capacity(vec));
	
	char buf[1000];
	ivec_dump(buf, 1000, vec);
	printf("%s\n", buf);

	printf("resize to 15\n", ivec_resize_default(vec, 15));
	printf("size = %u, empty = %d, capacity = %u\n", ivec_size(vec), ivec_empty(vec), ivec_capacity(vec));
	ivec_dump(buf, 1000, vec);
	printf("%s\n", buf);

	ivec_destroy(vec);

	svec_t* svec = new_svec(100);
	for (i = 0; i < 100; ++i) {
		snprintf(buf, 1000, "%d", i);
		assert(svec_push_back(svec, buf));
	}

	printf("size = %u, empty = %d, capacity = %u\n", svec_size(svec), svec_empty(svec), svec_capacity(svec));
	char* ret = svec_pop_back(svec);
	printf("popback = %s\n", ret);
	free(ret);
	printf("size = %u, empty = %d, capacity = %u\n", svec_size(svec), svec_empty(svec), svec_capacity(svec));

	char** s = svec_data(svec);
	for (i = 0; i < 99; ++i) {
		snprintf(buf, 1000, "%d", i);
		assert(!strcmp(*(s+i), buf) && !strcmp(buf, svec_at(svec, i)));
	}

	printf("resize to 10, and shrink to fit\n");
	svec_resize_default(svec, 10);
	svec_shrink_to_fit(svec);
	printf("size = %u, empty = %d, capacity = %u\n", svec_size(svec), svec_empty(svec), svec_capacity(svec));
	
	svec_dump(buf, 1000, svec);
	printf("%s\n", buf);

	printf("resize to 15\n", svec_resize_default(svec, 15));
	printf("size = %u, empty = %d, capacity = %u\n", svec_size(svec), svec_empty(svec), svec_capacity(svec));
	svec_dump(buf, 1000, svec);
	printf("%s\n", buf);

	svec_destroy(svec);

	return 0;
}
