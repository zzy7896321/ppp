#include "../ppp.h"
#include "../defs.h"
#include "query.h"
#include <stdio.h>

int main() {
       struct pp_query_t* query = pp_compile_query(0, "x < 3, x > 2");
       while (query) {
        printf(" %s %f %d \n", query->varname, query->threshold, query->compare);
        query = query->next;
       }
       return 0;
}
