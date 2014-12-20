#ifndef MH_SAMPLER_H
#define MH_SAMPLER_H

#include "infer.h"
#include "../common/trace.h"
#include "../common/variables.h"
#include "../common/stack.h"

extern unsigned g_mh_sampler_burn_in_iterations;
extern unsigned g_mh_sampler_lag;
extern unsigned g_mh_sampler_maximum_initial_round;

typedef struct mh_sampling_trace_t mh_sampling_trace_t;
typedef struct mh_sampler_t mh_sampler_t;

typedef struct mh_sampling_sample_t {
	pp_variable_t* value;
	float logprob;
	size_t num_param;
	pp_variable_t* param[];
} mh_sampling_sample_t;

mh_sampling_sample_t* new_mh_sampling_sample(pp_variable_t* value, float logprob, size_t num_param, ...);
int mh_sampling_get_new_sample(DrawStmtNode* stmt, pp_trace_t* trace, mh_sampling_sample_t** sample_ptr);
int mh_sampling_reuse_old_sample(DrawStmtNode* stmt, mh_sampling_trace_t* m_trace, mh_sampling_sample_t* old_sample, mh_sampling_sample_t** sample_ptr);
int mh_sampling_use_observed_value(DrawStmtNode* stmt, mh_sampling_trace_t* m_trace, pp_variable_t* variable);
int mh_sampling_rescore(DrawStmtNode* stmt, mh_sampling_trace_t* m_trace, pp_variable_t* variable, mh_sampling_sample_t** sample_ptr);
int mh_sampling_sample_has_same_param(mh_sampling_sample_t* lhs, mh_sampling_sample_t* rhs);
mh_sampling_sample_t* mh_sampling_sample_clone(mh_sampling_sample_t* sample);
void mh_sampling_sample_destroy(mh_sampling_sample_t* sample);

typedef int (*mh_sampling_kernel_t)(DrawStmtNode* node, mh_sampling_sample_t* sample, mh_sampling_sample_t** result_ptr, float* F, float* R);
int mh_sampling_random_walk(DrawStmtNode* node, mh_sampling_sample_t* sample, mh_sampling_sample_t** result_ptr,float* F, float* R);

typedef char* mh_sampling_name_t;

DECLARE_STACK(loop_index, unsigned)

struct mh_sampler_t {
	pp_state_t* state;
	char* model_name;
	ModelNode* model;	// model node is cached on initialization for performance consideration
	pp_variable_t** param;
	pp_query_t* query;

	loop_index_stack_t* loop_index;
	mh_sampling_trace_t* current_trace;
	float ll_stale;
	float ll_fresh;

	int n_accepted;
	int n_steps;
};

mh_sampler_t* new_mh_sampler(pp_state_t* state, const char* model_name, pp_variable_t* param[], pp_query_t* query);
int mh_sampler_init_trace(mh_sampler_t* mh_sampler);
int mh_sampler_has_same_model(mh_sampler_t* mh_sampler, pp_state_t* state, const char* model_name, pp_variable_t* param[], pp_query_t* query);
int mh_sampler_execute_draw_stmt(mh_sampler_t* mh_sampler, DrawStmtNode* stmt, pp_trace_t* trace);
int mh_sampler_execute_for_stmt(mh_sampler_t* mh_sampler, ForStmtNode* stmt, pp_trace_t* trace);
int mh_sampler_execute_stmt(mh_sampler_t* mh_sampler, StmtNode* stmt, pp_trace_t* trace);
int mh_sampler_step(mh_sampler_t* mh_sampler);
void mh_sampler_destroy(mh_sampler_t* mh_sampler);

int mh_sampler_observe_value(mh_sampler_t* sampler, VariableNode* variable, pp_trace_t* trace, pp_variable_t** result_ptr);

const mh_sampling_name_t mh_sampling_get_name(void* node, loop_index_stack_t* loop_index);

void* mh_sampling_name_to_node(const char* name);

//void mh_sampling_destroy_name(mh_sampling_name_t* name);
#define mh_sampling_destroy_name(name) free(name)


//DECLARE_HASH_TABLE(mh_sampling_erp, char*, mh_sampling_sample_t*);
typedef struct mh_sampling_erp_hash_table_t mh_sampling_erp_hash_table_t;
typedef struct mh_sampling_erp_hash_table_node_t mh_sampling_erp_hash_table_node_t;

struct mh_sampling_trace_t {
	pp_trace_t super;

	float observed_logprob;
	mh_sampling_erp_hash_table_t* erp_hash_table;
};

mh_sampling_trace_t* new_mh_sampling_trace(symbol_table_t* symbol_table);

mh_sampling_erp_hash_table_node_t* mh_sampling_trace_randomly_pick_one_erp(mh_sampling_trace_t* trace);

void mh_sampling_trace_destroy(mh_sampling_trace_t* trace);

#endif
