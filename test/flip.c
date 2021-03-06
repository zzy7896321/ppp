#include <stdio.h>
#include "ppp.h"
#include <stdlib.h>
#include <time.h>

#include "../query/string_query.h"
#include "../parse/parse.h"
#include "../parse/interface.h"

#include "../common/mem_profile.h"

int main()
{
#ifdef ENABLE_MEM_PROFILE
    mem_profile_init();
#endif

    /* use pointers to structs because the client doesn't need to know the struct sizes */
    struct pp_state_t* state;
    struct pp_instance_t* instance;
    struct pp_query_t* query;
    struct pp_trace_store_t* traces;
    float result;

    state = pp_new_state();
    printf("> state created\n");

    pp_load_file(state, "parse/models/flip.model");
    printf("> file loaded\n");

	if (!state) return 1;
	
	ModelNode* model = model_map_find(state->model_map, state->symbol_table, "flip_example");
	printf(dump_model(model));

    query = pp_compile_string_query("x>2 x<3");
    printf("> condition compiled\n");


    traces = pp_sample(state, "flip_example", 0, query);
    printf("> traces sampled\n");

    pp_compiled_query_destroy(query);
    
    query = pp_compile_string_query("f== 1");
    printf("> query compiled\n");

    pp_get_result(traces, query, &result);  // "get_result" may not be a good name 
    printf("%f\n", result);
	
	printf("last sample:\n");
	char buffer[8096];
	pp_trace_dump(buffer, 8096, traces->trace[traces->n-1]);
	printf(buffer); 

    FILE* trace_dump_file = fopen("trace_dump.txt", "w");
    for (size_t i = 0; i != traces->n; ++i) {
        pp_trace_dump(buffer, 8096, traces->trace[i]);
        fprintf(trace_dump_file, "[trace %u]\n", i);
        fprintf(trace_dump_file, buffer);
    }
    fclose(trace_dump_file);

	// pp_free is broken
    pp_free(state);  /* free memory, associated models, instances, queries, and trace stores are deallocated */

    pp_trace_store_destroy(traces);

    pp_compiled_query_destroy(query);

#ifdef ENABLE_MEM_PROFILE
    mem_profile_print();
    mem_profile_destroy();
#endif

    return 0;
}
