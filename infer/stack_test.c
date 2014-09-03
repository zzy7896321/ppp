#include "stack.h"

#include <stdio.h>
#include <string.h>

DECLARE_STACK(int, int)
#define STACK_DEFAULT_VALUE 0
#define STACK_DESTROY_VALUE(value)
DEFINE_STACK(int, int)
#undef STACK_DEFAULT_VALUE
#undef STACK_DESTROY_VALUE

int main() {

	int_stack_t* istack = new_int_stack(2);

	printf("istack->capacity = %u, istack->size = %u\n", int_stack_capacity(istack), int_stack_size(istack));

	int n = 100000;
	printf("pushing 0..%d to istack\n", n-1);
	for (int i = 0; i < n; ++i) {
		int_stack_push(istack, i);
	}

	printf("istack->capacity = %u, istack->size = %u\n", int_stack_capacity(istack), int_stack_size(istack));

	printf("istack->top() = %d\n", int_stack_top(istack));
	printf("istack->pop(), istack->top() = %d, istack->size() = %u\n", (int_stack_pop(istack), int_stack_top(istack)), int_stack_size(istack));

	printf("destroying istack\n");
	int_stack_destroy(istack);
	return 0;
}