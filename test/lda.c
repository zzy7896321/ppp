#include <stdio.h>
#include "ppp.h"
#include <stdlib.h>
#include <time.h>

#include "query/query.h"
#include "parse/parse.h"
#include "parse/interface.h"
#include "infer/variables.h"

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

    pp_load_file(state, "parse/models/lda.model");
    printf("> file loaded\n");
	
	ModelNode* model = model_map_find(state->model_map, state->symbol_table, "latent_dirichlet_allocation");
	printf(dump_model(model));

    //query = pp_compile_query("");
    //printf("> condition compiled\n");

	pp_variable_t** param = malloc(sizeof(pp_variable_t*) * 4);
	param[0] = new_pp_int(2);
	param[1] = new_pp_int(2);
	param[2] = new_pp_vector(2);
	PP_VARIABLE_VECTOR_LENGTH(param[2]) = 2;
	PP_VARIABLE_VECTOR_VALUE(param[2])[0] = new_pp_int(2);
	PP_VARIABLE_VECTOR_VALUE(param[2])[1] = new_pp_int(2);
	param[3] = new_pp_int(3);	

    traces = pp_sample(state, "latent_dirichlet_allocation", param, 0);
    printf("> traces sampled\n");

    //query = pp_compile_query("f== 1");
    //printf("> query compiled\n");

    //pp_get_result(traces, query, &result);  // "get_result" may not be a good name 
    //printf("%f\n", result);
    if (!traces) {
        printf("ERROR encountered!!\n");
        return 1;
    }

	printf("last sample:\n");
	char buffer[8096];
	pp_trace_dump(traces->trace[traces->n-1], buffer, 8096);
	printf(buffer); 

    FILE* trace_dump_file = fopen("trace_dump.txt", "w");
    for (size_t i = 0; i != traces->n; ++i) {
        pp_trace_dump(traces->trace[i], buffer, 8096);
        fprintf(trace_dump_file, "[trace %u]\n", i);
        fprintf(trace_dump_file, buffer);
    }
    fclose(trace_dump_file);

	// pp_free is broken
    pp_free(state);  /* free memory, associated models, instances, queries, and trace stores are deallocated */

    return 0;
}