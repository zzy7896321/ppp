#include <stdio.h>
#include "ppp.h"
#include <stdlib.h>
#include <time.h>

#include "../query/query.h"
#include "../parse/parse.h"
#include "../parse/interface.h"

#include "../common/mem_profile.h"
#include "../common/trace.h"
#include "../common/variables.h"


void calc_mean_var(pp_trace_store_t* traces) {
	float sum = 0.0, sqrsum = 0.0;
	float mean, variance;
	int n = traces->n;

	int i;
	for ( i = 0; i < n; ++i) {
		pp_variable_t* pp_var = pp_trace_find_variable(traces->trace[i], "bias");
		if (pp_var == 0 || pp_var->type != PP_VARIABLE_FLOAT) {
			printf("error: bias not found\n");
			return ;
		}
	
		float var = PP_VARIABLE_FLOAT_VALUE(pp_var);
		sum += var;
		sqrsum += var * var;
	}

	mean = sum / n;
	variance = sqrsum /n - mean * mean;
	printf("\nbias: mean = %f, variance = %f\n\n", mean, variance);
}

int main(int argc, char** argv)
{
#ifdef ENABLE_MEM_PROFILE
    mem_profile_init();
#endif

	int a = 2, b = 5;
	if (argc >= 3) {
		a = atoi(argv[1]);
		b = atoi(argv[2]);
	}

    /* use pointers to structs because the client doesn't need to know the struct sizes */
    struct pp_state_t* state;
    struct pp_instance_t* instance;
    struct pp_query_t* query;
    struct pp_trace_store_t* traces;
    float result;

    state = pp_new_state();
    printf("> state created\n");

    pp_load_file(state, "parse/models/biased-coin.model");
    printf("> file loaded\n");

	if (!state) return 1;
	
	ModelNode* model = model_map_find(state->model_map, state->symbol_table, "biased_coin");
	printf(dump_model(model));

    query = pp_compile_query("x[0] == 1 x[1] == 1 x[2] == 0 x[3] == 1 x[4] == 0");
    printf("> condition compiled\n");

	pp_variable_t* param[2] = {
		new_pp_int(a),
		new_pp_int(b),
	};

    traces = pp_sample(state, "biased_coin", param, query);
    printf("> traces sampled\n");

    pp_query_destroy(query);

	calc_mean_var(traces);
    
    //query = pp_compile_query("bias == 1");
    //printf("> query compiled\n");

    //pp_get_result(traces, query, &result);  // "get_result" may not be a good name 
    //printf("%f\n", result);
	
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

    pp_trace_store_destroy(traces);

    //pp_query_destroy(query);
	
	pp_variable_destroy(param[0]);
	pp_variable_destroy(param[1]);

#ifdef ENABLE_MEM_PROFILE
    mem_profile_print();
    mem_profile_destroy();
#endif

    return 0;
}
