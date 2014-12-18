#ifndef PPP_H_
#define PPP_H_

struct pp_state_t;
struct pp_instance_t;
struct pp_query_t;
struct pp_trace_t;
struct pp_trace_store_t;
struct pp_variable_t;

struct pp_state_t* pp_new_state();
int pp_free(struct pp_state_t* state);

int pp_load_file(struct pp_state_t* state, const char* filename);
//struct pp_instance_t* pp_new_instance(struct pp_state_t* state, const char* model_name, float* model_params);

typedef void (*sample_acceptor)(void* data, struct pp_trace_t* trace);

struct pp_query_t* pp_compile_query(const char* query_string);
struct pp_trace_store_t* pp_sample(struct pp_state_t* state, const char* model_name, struct pp_variable_t* param[], struct pp_query_t* query);
struct pp_trace_store_t* pp_sample_v(struct pp_state_t* state, const char* model_name, struct pp_variable_t* param[], struct pp_query_t* query, int num_output_vars, ...);
int pp_sample_f(struct pp_state_t* state, const char* model_name, struct pp_variable_t* param[], struct pp_query_t* query, sample_acceptor sa, void* sa_data);
int pp_get_result(struct pp_trace_store_t* traces, struct pp_query_t* query, float* result);

//int pp_infer(struct pp_instance_t* instance, const char* X, const char* Y, float* result);

void set_sample_iterations(int sample_iterations);

void set_sample_method(const char* sample_method);

void set_mh_burn_in(int burn_in);

void set_mh_lag(int lag);

void set_mh_max_initial_round(int initial_round);

void set_prompt_per_round(int round);


#endif
