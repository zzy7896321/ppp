#include <stdio.h>
#include "ppp.h"
#include <stdlib.h>
#include <time.h>

int main()
{
    /* use pointers to structs because the client doesn't need to know the struct sizes */
    struct pp_state_t* state;
    struct pp_instance_t* instance;
    struct pp_query_t* query;
    struct pp_trace_store_t* traces;
    float result;

    state = pp_new_state();
    printf("> state created\n");

    pp_load_file(state, "../parse/models/flip.model");
    printf("> file loaded\n");

    instance = pp_new_instance(state, "flip_example", 0);
    printf("> instance created\n");

    query = pp_compile_query(instance, "x>2, x<3");
    printf("> condition compiled\n");

    traces = pp_sample(instance, query);
    printf("> traces sampled\n");

    query = pp_compile_query(instance, "flip == 1");
    printf("> query compiled\n");

    pp_get_result(traces, query, &result);  /* "get_result" may not be a good name */
    printf("%f\n", result);

    pp_infer(instance, "flip == 1", "x > 2, x < 3", &result);  /* alternative way to get a result */
    printf("%f\n", result);

    pp_free(state);  /* free memory, associated models, instances, queries, and trace stores are deallocated */

    return 0;
}
