#include <stdio.h>
#include "ppp.h"
#include <stdlib.h>
#include <time.h>

#include "../query/observation.h"
#include "../query/string_query.h"
#include "../parse/parse.h"
#include "../parse/interface.h"
#include "../common/variables.h"
#include "../common/trace.h"

int main() {
	set_sample_method("Metropolis-hastings");
	set_sample_iterations(1000);
	set_mh_burn_in(200);
	set_mh_lag(10);
	set_mh_max_initial_round(2000);
	set_prompt_per_round(50);

	pp_state_t* state;
	struct pp_instance_t* instance;
	pp_query_t* query;
	pp_trace_store_t* traces;

	state = pp_new_state();

	pp_load_file(state, "parse/models/hmm.model");

	ModelNode* model = model_map_find(state->model_map, state->symbol_table, "hidden_markov_model");
	printf(dump_model(model));
	
	

	return 0;
}

