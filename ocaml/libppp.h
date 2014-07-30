struct pp_state_t;
struct pp_instance_t;
struct pp_query_t;
struct pp_trace_store_t;

struct pp_state_t* pp_new_state();
int pp_free(struct pp_state_t* state);

int pp_load_file(struct pp_state_t* state, const char* filename);
struct pp_instance_t* pp_new_instance(struct pp_state_t* state, const char* model_name, float* model_params);

struct pp_query_t* pp_compile_query(struct pp_instance_t* instance, const char* query_string);
struct pp_trace_store_t* pp_sample(struct pp_instance_t* instance, struct pp_query_t* query);
int pp_get_result(struct pp_trace_store_t* traces, struct pp_query_t* query, float* result);

int pp_infer(struct pp_instance_t* instance, const char* X, const char* Y, float* result);

