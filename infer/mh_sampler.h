#ifndef MH_SAMPLER_H_
#define MH_SAMPLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../defs.h"

extern int g_burn_in_iterations;
extern int g_sample_round;

int mh_sampling(struct pp_instance_t* instance, acceptor_t condition_accept, 
				name_to_value_t F, void* raw_data,
				vertices_handler_t add_trace, void* add_trace_data);

/* the type of names of vertices. The name is currently the no. of the vertex. */
typedef int mh_name_t;

/* metropolis-hastings sampler */
typedef struct mh_sampler_t{
	struct pp_instance_t* instance;
	int num_of_vertices;

	float* value;
	float* log_likelihood;
	float** param;
	float ll;	// likelihood of X
	int size_of_db;		
	// the types of erps are fixed in the instance

	int num_of_erps;	// the number of erps
	int* erps;		// the no. of vertices of erps

	/* pending updates */
	float* new_value;
	float* new_log_likelihood;
	float** new_param;
	float new_ll;
	float ll_stale, ll_fresh;
	int new_size_of_db;

} mh_sampler_t;

mh_sampler_t* mh_sampler_init(struct pp_instance_t* instance);

void mh_sampler_free(mh_sampler_t* sampler);

void mh_sampler_trace_update(mh_sampler_t* sampler);

void mh_sampler_trace_update_with_new_value(mh_sampler_t* sampler, mh_name_t name, float value, float log_likelihood, float* param);

void mh_sampler_get_current_param(mh_sampler_t* sampler, float* current_param, struct BNVertexDraw* vertexDraw);

/* may update likelihood and database */
float mh_sampler_get_sample(mh_sampler_t* sampler, int vertex_index, struct BNVertexDraw* vertexDraw);

float mh_sampler_get_compute(mh_sampler_t* sampler, int vertex_index, struct BNVertexCompute* vertexComp);

int mh_sampler_get_random_erp(mh_sampler_t* sampler);

void mh_sampler_accept_update(mh_sampler_t* sampler);

void mh_sampler_discard_update(mh_sampler_t* sampler);

/* Miscs*/
mh_name_t mh_get_name(struct pp_instance_t* instance, int i);

int mh_number_of_param(struct BNVertexDraw* vertexDraw);

void mh_update_param(float** param, float* new_param, int num_of_param);

int mh_param_equal(float* param1, float* param2, int num_of_param);

float mh_log_likelihood(struct BNVertexDraw* vertexDraw, float value, float* param);

#endif	// MH_SAMPLER_H_
