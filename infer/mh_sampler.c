#include "mh_sampler.h"
#include "infer.h"
#include "../debug.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

#ifdef DEBUG
	#define ERR_OUTPUT(...) \
		fprintf(stderr, "%s(%d): ", __FILE__, __LINE__); \
		fprintf(stderr,  __VA_ARGS__)
#else
	#define ERR_OUTPUT(...) 
#endif

int g_burn_in_iterations = 0;//2000; temporarily disabled
int g_sample_round = 100;

int mh_sampling(struct pp_instance_t* instance, acceptor_t condition_acceptor, 
				name_to_value_t name_to_value_F, void* raw_data, 
				vertices_handler_t add_trace, void* add_trace_data) 
{
	/* debug output */
	FILE* transition_out = fopen("transitions.txt", "w");
	/* end debug output */

	ERR_OUTPUT("initialing mh_sampler\n");
	mh_sampler_t* sampler = mh_sampler_init(instance);	
	
	ERR_OUTPUT("initializing trace\n");
	mh_sampler_trace_update(sampler);

	ERR_OUTPUT("accepting_update\n");
	mh_sampler_accept_update(sampler);
	
	ERR_OUTPUT("INITIAL SAMPLE: \n");
	for (int i = 0; i < sampler->num_of_vertices; ++i) {
		ERR_OUTPUT("vertex %d = %f\n", i,  sampler->value[i]);
	}
	ERR_OUTPUT("\n");

	for (int k = 1; k <= g_sample_iterations + g_burn_in_iterations; ++k) {

		int erp_id = mh_sampler_get_random_erp(sampler);
		struct BNVertexDraw* vertexDraw = (struct BNVertexDraw*) pp_instance_vertex(instance, erp_id);
				
		ERR_OUTPUT("round %d, vertex_addr = 0x%08x, erp_id = %d, vertexType = %d, value = %f\n", k, vertexDraw, erp_id, vertexDraw->type, vertexDraw->super.sample);

		float new_sample;
		float new_ll; 
		float F, R;
		float acceptance_rate;

		switch (vertexDraw->type) {
		case FLIP:
		case LOG_FLIP:
			
			if (rand() < (RAND_MAX >> 1)) {
				new_sample = sampler->value[erp_id];
				new_ll = sampler->log_likelihood[erp_id];
				F = 0.5;
				R = 0.5;
			}

			else {
				new_sample = 1 - sampler->value[erp_id];
				if (vertexDraw->type == FLIP) {
					new_ll = flip_logprob(new_sample, sampler->param[erp_id][0]);
				}

				else {
					new_ll = log_flip_logprob(new_sample, sampler->param[erp_id][0]);
				}
				F = 0.5;
				R = 0.5;
			}

			break;

		default:
		
			new_sample = uniform(sampler->value[erp_id] - 1, sampler->value[erp_id] + 1);
			F = R = 0;

			//new_sample = uniform(sampler->value[erp_id] - fabs(sampler->value[erp_id]) / 2 - 1, 
				//	sampler->value[erp_id] + fabs(sampler->value[erp_id] / 2)) + 1;
			//F = -log(fabs(sampler->value[erp_id]) + 2);
			//R = -log(fabs(new_sample) + 2);
			
			
			//new_sample = gaussian(sampler->value[erp_id], 1 + sampler->value[erp_id] * sampler->value[erp_id]);
			//F = gaussian_logprob(new_sample, sampler->value[erp_id], 1 + sampler->value[erp_id] * sampler->value[erp_id]);
			//R = gaussian_logprob(sampler->value[erp_id], new_sample, 1 + new_sample * new_sample);

			if (vertexDraw->type == GAUSSIAN) {
				assert(sampler->param[erp_id]);
				new_ll = gaussian_logprob(new_sample, sampler->param[erp_id][0], sampler->param[erp_id][1]);
			}

			else {
				assert(sampler->param[erp_id]);
				new_ll = gamma_logprob(new_sample, sampler->param[erp_id][0], sampler->param[erp_id][1]);
			}

			
			break;
		}
		
		ERR_OUTPUT("trace update with new_value = %f, ll = %f\n", new_sample, new_ll);
		mh_sampler_trace_update_with_new_value(sampler, mh_get_name(sampler->instance, erp_id), new_sample, new_ll, sampler->param[erp_id]);


		ERR_OUTPUT("new_ll = %f, ll = %f, R = %f, F = %f, size_of_db = %d, new_size_of_db = %d, ll_stale = %f, ll_fresh = %f\n",
				sampler->new_ll, sampler->ll, R, F, sampler->size_of_db, sampler->new_size_of_db, sampler->ll_stale, sampler->ll_fresh);
		acceptance_rate = sampler->new_ll - sampler->ll + R - F + log((float)(sampler->size_of_db) / (float)(sampler->new_size_of_db)) + sampler->ll_stale - sampler->ll_fresh;

		float random_value = log(randomR());
		ERR_OUTPUT("log(randomR()) = %f, acceptance_rate = %f\n", random_value, acceptance_rate);

		/* debug output */
			for (int i = 0; i < sampler->num_of_erps; ++i) {
				fprintf(transition_out, "%f ", sampler->value[sampler->erps[i]]);
			}
		/* debug output */

		if (random_value < acceptance_rate) {
			mh_sampler_accept_update(sampler);
		}

		else {
			mh_sampler_discard_update(sampler);
		}

		/* debug output 
			for (int i = 0; i < sampler->num_of_erps; ++i) {
				fprintf(transition_out, "%f ", sampler->value[sampler->erps[i]]);
			}
			fprintf(transition_out, "\n");
		/* debug output */

		ERR_OUTPUT("SAMPLE: \n");
		for (int i = 0; i < sampler->num_of_erps; ++i) {
			ERR_OUTPUT("vertex %d = %f\n", sampler->erps[i], sampler->value[sampler->erps[i]]);
		}

		ERR_OUTPUT("\n");

		if (k >= g_burn_in_iterations) {
			if (condition_acceptor(instance, name_to_value_F, raw_data)) {
				add_trace(instance, add_trace_data);
			}
		}

	}

		/* debug output */
			fclose(transition_out);
		/* debug output */
	return 0;
}

mh_sampler_t* mh_sampler_init(struct pp_instance_t* instance) {
	mh_sampler_t *sampler = malloc(sizeof(mh_sampler_t));
	int n = pp_instance_num_vertices(instance);	// number of vertices
	
	sampler->num_of_vertices = n;
	sampler->instance = instance;

	sampler->value = malloc(sizeof(float) * n);
	sampler->log_likelihood = malloc(sizeof(float) * n);
	sampler->param = calloc(n, sizeof(float*));	// NULL indicates not in the DB
	//sampler->ll
	sampler->size_of_db = 0;
	
	int num_of_erps = 0; // just go through the vertices twice. 
	for (int i = 0; i < n; ++i){
		if (pp_instance_vertex(instance, i)->type == BNV_DRAW){
			++num_of_erps;
		}
	}

	sampler->num_of_erps = num_of_erps;
	sampler->erps = malloc(sizeof(int) * num_of_erps);

	num_of_erps = 0;
	for (int i = 0; i < n; ++i){
		//printf("%d: ", i);
		//dump_vertex(pp_instance_vertex(instance, i));

		if (pp_instance_vertex(instance, i)->type == BNV_DRAW){
			sampler->erps[num_of_erps++] = i;
		}
	}

	/*printf("erps:\n");	
	for (int i = 0; i < num_of_erps; ++i) {
		printf("%d: ", sampler->erps[i]);
		dump_vertex(pp_instance_vertex(instance, sampler->erps[i]));
	}*/

	sampler->new_value = malloc(sizeof(float) * n);
	sampler->new_log_likelihood = malloc(sizeof(float) * n);
	sampler->new_param = calloc(n, sizeof(float*));
	//sampler->new_ll, sampler->ll_stale, sampler->ll_fresh
	sampler->new_size_of_db = 0;

	return sampler;
}

void mh_sampler_free(mh_sampler_t* sampler){
	free(sampler->value);
	free(sampler->log_likelihood);
	
	for (int i = 0; i < sampler->num_of_vertices; ++i){
		if (sampler->param[i])
			free(sampler->param[i]);

		if (sampler->new_param[i])
			free(sampler->new_param[i]);
	}

	free(sampler->param);
	free(sampler->erps);

	free(sampler->new_value);
	free(sampler->new_log_likelihood);
	free(sampler->new_param);	// new_param[i] was freed above

	free(sampler);
}

void mh_sampler_trace_update(mh_sampler_t* sampler){
	sampler->new_ll = 0.0;
	sampler->ll_stale = 0.0;
	sampler->ll_fresh = 0.0;

	for (int i = 0; i < sampler->num_of_vertices; ++i) {
		struct BNVertex* vertex = pp_instance_vertex(sampler->instance, i);

		switch (vertex->type){
		case BNV_CONST:
			// do nothing			
			break;

		case BNV_DRAW:
			vertex->sample = mh_sampler_get_sample(sampler, i, (struct BNVertexDraw*) vertex);	
			break;

		case BNV_COMPU:
			vertex->sample = mh_sampler_get_compute(sampler, i, (struct BNVertexCompute*) vertex);
			sampler->new_value[i] = vertex->sample;
			break;

		default:
			ERR_OUTPUT("Unknown BNVertex type: %d\n", vertex->type);
			return ;
		}
	}
}

void mh_sampler_trace_update_with_new_value(mh_sampler_t* sampler, mh_name_t name, float value, float log_likelihood, float* param){
	if (!sampler->new_param[name]){
		++sampler->new_size_of_db;
	}

	sampler->new_value[name] = value;
	sampler->new_log_likelihood[name] = log_likelihood;
	
	mh_update_param( &(sampler->new_param[name]), param, mh_number_of_param((struct BNVertexDraw*) pp_instance_vertex(sampler->instance, name)) );

	mh_sampler_trace_update(sampler);
}

float mh_sampler_get_sample(mh_sampler_t* sampler, int vertex_index, struct BNVertexDraw* vertexDraw) {

	int num_of_param = mh_number_of_param(vertexDraw);
	float current_param[num_of_param];

	mh_sampler_get_current_param(sampler, current_param, vertexDraw);

	if (sampler->new_param[vertex_index]){
		// a previously sampled value is found
		
		if ( ! mh_param_equal(sampler->new_param[vertex_index], current_param, num_of_param)) {
			mh_update_param( &(sampler->new_param[vertex_index]), current_param, num_of_param);				
			sampler->new_log_likelihood[vertex_index] = mh_log_likelihood(vertexDraw, sampler->new_value[vertex_index], current_param);
		}
	}

	else {
		// no previously sampled value
		
		sampler->new_value[vertex_index] = getsample(vertexDraw);
		mh_update_param( &(sampler->new_param[vertex_index]), current_param, num_of_param);
		sampler->new_log_likelihood[vertex_index] = mh_log_likelihood(vertexDraw, sampler->new_value[vertex_index], current_param);

		sampler->ll_fresh += sampler->new_log_likelihood[vertex_index];
		++sampler->new_size_of_db;
	}

	sampler->new_ll += sampler->new_log_likelihood[vertex_index];
	return sampler->new_value[vertex_index];
}

float mh_sampler_get_compute(mh_sampler_t* sampler, int vertex_index, struct BNVertexCompute* vertexComp) {

    switch (vertexComp->type) {
        case BNVC_BINOP: {
            struct BNVertexComputeBinop* vertexBinop = (struct BNVertexComputeBinop*)vertexComp;
            switch (vertexBinop->binop) {
                case BINOP_PLUS:
                    return vertexBinop->left->sample + vertexBinop->right->sample;
                case BINOP_SUB:
                    return vertexBinop->left->sample - vertexBinop->right->sample;
                case BINOP_MULTI:
                    return vertexBinop->left->sample * vertexBinop->right->sample;
                case BINOP_DIV:
                    return vertexBinop->left->sample / vertexBinop->right->sample;
            }
            return 0;
        }
        case BNVC_IF: {
            struct BNVertexComputeIf* vertexIf = (struct BNVertexComputeIf*)vertexComp;
			struct BNVertex *taken, *not_taken;
            if (vertexIf->condition->sample) {
				taken = vertexIf->consequent;
				not_taken = vertexIf->alternative;
			}
            else {
				taken = vertexIf->alternative;
				not_taken = vertexIf->consequent;
			}
			
			if (not_taken->type == BNV_DRAW) {
				int not_taken_index = pp_instance_find_num_of_vertex(sampler->instance, not_taken);

				// remove from db
				free(sampler->new_param[not_taken_index]);
				sampler->new_param[not_taken_index] = 0;
				sampler->ll_stale += sampler->new_log_likelihood[not_taken_index];
				sampler->new_ll -= sampler->new_log_likelihood[not_taken_index];
				--sampler->new_size_of_db;
			}

			return taken->sample;
        }
        case BNVC_UNARY: {
            struct BNVertexComputeUnary* vertex = (struct BNVertexComputeUnary*)vertexComp;
            if (vertex->op == UNARY_NEG)
                return -(vertex->primary->sample);
			if (vertex->op == UNARY_POS)
				return vertex->primary->sample;
            return 0;
        }
        case BNVC_FUNC: {
            struct BNVertexComputeFunc* vertex = (struct BNVertexComputeFunc*)vertexComp;
            if (vertex->func == FUNC_LOG)
                return log(vertex->args[0]->sample);
            if (vertex->func == FUNC_EXP)
                return exp(vertex->args[0]->sample);
            return 0;
        }
        default: return 0;
    }
}

void mh_sampler_get_current_param(mh_sampler_t* sampler, float* current_param, struct BNVertexDraw* vertexDraw) {

	switch (vertexDraw->type) {
	case FLIP:
	case LOG_FLIP:
		current_param[0] = ((struct BNVertexDrawBern*) vertexDraw)->p->sample;
		break;

	case GAUSSIAN:
		{
			struct BNVertexDrawNorm* vertexNorm = (struct BNVertexDrawNorm*) vertexDraw;
			current_param[0] = vertexNorm->mean->sample;
			current_param[1] = vertexNorm->variance->sample;
		}
		break;

	case GAMMA:
		{
			struct BNVertexDrawGamma* vertexGamma = (struct BNVertexDrawGamma*) vertexDraw;
			current_param[0] = vertexGamma->a->sample;
			current_param[1] = vertexGamma->b->sample;
		}
		break;
	/* FIXME support for more distributions */
	
	default:
		ERR_OUTPUT("Unknown distribution %d\n", vertexDraw->type);
	}
}

int mh_sampler_get_random_erp(mh_sampler_t* sampler) {
	int id = rand() % sampler->num_of_erps;

	while (!sampler->param[sampler->erps[id]]) {
		id = rand() % sampler->num_of_erps;
	}

	return sampler->erps[id];	
}

void mh_sampler_accept_update(mh_sampler_t* sampler) {
	ERR_OUTPUT("accept update\n");
	int n = sampler->num_of_vertices;

	memcpy(sampler->value, sampler->new_value, sizeof(float) * n);
	memcpy(sampler->log_likelihood, sampler->new_log_likelihood, sizeof(float) * n);

	for (int i = 0; i < n; ++i) {
		struct BNVertex* vertex = (struct BNVertex*) pp_instance_vertex(sampler->instance, i);
		if (vertex->type == BNV_DRAW) {
			mh_update_param(&(sampler->param[i]), sampler->new_param[i], mh_number_of_param((struct BNVertexDraw*) vertex));
		}
	}

	sampler->ll = sampler->new_ll;
	sampler->size_of_db = sampler->new_size_of_db;
}

void mh_sampler_discard_update(mh_sampler_t* sampler) {
	ERR_OUTPUT("discard update\n");
	int n = sampler->num_of_vertices;

	memcpy(sampler->new_value, sampler->value, sizeof(float) * n);
	memcpy(sampler->new_log_likelihood, sampler->log_likelihood, sizeof(float) * n);

	for (int i = 0; i < n; ++i) {
		struct BNVertex* vertex = (struct BNVertex*) pp_instance_vertex(sampler->instance, i);
		if (vertex->type == BNV_DRAW) {
			mh_update_param(&(sampler->new_param[i]), sampler->param[i], mh_number_of_param((struct BNVertexDraw*) vertex));
			vertex->sample = sampler->value[i];
		}
		else if (vertex->type == BNV_COMPU) {
			vertex->sample = sampler->value[i];
		}
	}

	sampler->new_size_of_db = sampler->size_of_db;
}

/* miscs */
mh_name_t mh_get_name(struct pp_instance_t* instance, int i) {
	return i;
}

int mh_number_of_param(struct BNVertexDraw* vertexDraw){

	switch (vertexDraw->type){
	case FLIP:
	case LOG_FLIP:
		return 1;

	case GAUSSIAN:
	case GAMMA:
		return 2;

	default:
		ERR_OUTPUT("Unknown vertexDraw->type %d\n", vertexDraw->type);	
	}

	// FIXME support more distributions
	
	return 0;
}

void mh_update_param(float** param, float* new_param, int num_of_param) {
	
	if (new_param) {
		if (!*param){
			*param = malloc(num_of_param * sizeof(float));
		}
		memcpy(*param, new_param, num_of_param * sizeof(float));
	}

	else {
		
		if (*param) {
			free(*param);
			*param = 0;
		}
	}
}

int mh_param_equal(float* param1, float* param2, int num_of_param){
	
	for ( ; --num_of_param >= 0 && param1[num_of_param] == param2[num_of_param]; );
	return num_of_param < 0;
}

float mh_log_likelihood(struct BNVertexDraw* vertexDraw, float value, float* param) {
	
	switch (vertexDraw->type){
	case FLIP:
		return flip_logprob(value, param[0]);

	case LOG_FLIP:
		return log_flip_logprob(value, param[0]);

	case GAUSSIAN:
		return gaussian_logprob(value, param[0], param[1]);

	case GAMMA:
		return gamma_logprob(value, param[0], param[1]);

	default:
		ERR_OUTPUT("Unknown vertexDraw->type %d\n", vertexDraw->type);	
	}

	// FIXME support more distributions
	
	return -INFINITY;
}

