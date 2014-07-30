#ifndef QUERY_H_
#define QUERY_H_

/* Define your own stuff here */

#define PPPL_COMPARE_EQ     0
#define PPPL_COMPARE_NEQ    1
#define PPPL_COMPARE_LT     2
#define PPPL_COMPARE_GT     3
#define PPPL_COMPARE_NGT    4
#define PPPL_COMPARE_NLT    5

struct pp_query_t {
    /* FIXME */
	char varname[100];
	float threshold;
	int compare;
	struct pp_query_t *next;
};
struct pp_query_t* pp_compile_query(struct pp_instance_t* instance, const char* query_string);
int pp_query_acceptor(struct pp_instance_t* instance, name_to_value_t F, void* raw_data);

#endif
