#ifndef DEFS_H_
#define DEFS_H_

/* Define common data structures here */

struct pp_instance_t;

/**
Get total number of vertices.
*/
int pp_instance_num_vertices(struct pp_instance_t*);

/**
Get the i-th vertex.
*/
struct BNVertex* pp_instance_vertex(struct pp_instance_t*, int i);

/**
Get the name of the i-th vertex.
*/
const char* pp_instance_vertex_name(struct pp_instance_t*, int i);

/**
 *	Get the number of the vertex. -1 if not found.
 */
int pp_instance_find_num_of_vertex(struct pp_instance_t*, struct BNVertex* vertex);

/* Define common types here */

typedef float (*name_to_value_t)(struct pp_instance_t* instance, const char* name);

typedef int (*acceptor_t)(struct pp_instance_t* instance, name_to_value_t F, void* raw_data);

typedef int (*vertices_handler_t)(struct pp_instance_t* instance, void* raw_data);

/* Define internal interfaces here */

float pp_name_to_value(struct pp_instance_t* instance, const char* name);

struct pp_trace_t;
struct pp_query_t;
int pp_query_acceptor(struct pp_trace_t* trace, struct pp_query_t* query);

/* BNVertex */

struct BNVertex
{
    enum {
        BNV_CONST,
    	BNV_DRAW,
    	BNV_COMPU
	} type;
    float sample;  /* last sampled value */
};

struct BNVertexDraw /* extends BNVertex */
{
    struct BNVertex super;
    enum {
		FLIP, LOG_FLIP,
		MULTINOMIAL,
		UNIFORM,
		UNIFORM_DISCRETE,
		GAUSSIAN,
		GAMMA,
		BETA,
		BINOMIAL,
		POISSON,
		DIRICHLET,
		DISTR_TYPE_LIMIT
	} type;
};

struct BNVertexDrawBern /* extends BNVertexDraw */
{
    struct BNVertexDraw super;
    struct BNVertex* p;
};

struct BNVertexDrawNorm /* extends BNVertexDraw */
{
    struct BNVertexDraw super;
    struct BNVertex* mean;
    struct BNVertex* variance;
};

struct BNVertexDrawGamma /* extends BNVertexDraw */
{
    struct BNVertexDraw super;
    struct BNVertex* a;
    struct BNVertex* b;
};

struct BNVertexDrawBinomial {
	struct BNVertexDraw super;
	float p;
	int n;
};

struct BNVertexDrawMultinomial {
    struct BNVertexDraw super;
    float* theta;
    int n;
};

struct BNVertexDrawUniform {
    struct BNVertexDraw super;
    float low;
    float high;
};

struct BNVertexDrawUniformDiscrete {
	struct BNVertexDraw super;
	int low;
	int high;
};

struct BNVertexDrawBeta {
    struct BNVertexDraw super;
    float a;
    float b;
};

struct BNVertexDrawPoisson {
    struct BNVertexDraw super;
    int mu;
};

struct BNVertexDrawDrichlet {
    struct BNVertexDraw super;
    float* alpha;
    int n;
};

struct BNVertexCompute /* extends BNVertex */
{
    struct BNVertex super;
    enum {
    	BNVC_IF,
   		BNVC_BINOP,
        BNVC_UNARY,
        BNVC_FUNC
	} type;
};

struct BNVertexComputeFunc /* extends BNVertexCompute */
{
    struct BNVertexCompute super;
    enum { FUNC_LOG, FUNC_EXP } func;
    struct BNVertex* args[1];
};

struct BNVertexComputeUnary /* extends BNVertexCompute */
{
    struct BNVertexCompute super;
    struct BNVertex* primary;
    enum {
        UNARY_NEG,
		UNARY_POS
    } op;
};

struct BNVertexComputeBinop /* extends BNVertexCompute */
{
    struct BNVertexCompute super;
    struct BNVertex* left;
    struct BNVertex* right;
    enum {
        BINOP_PLUS,
        BINOP_SUB,
        BINOP_MULTI,
        BINOP_DIV
    } binop;
};

struct BNVertexComputeIf /* extends BNVertexCompute */
{
    struct BNVertexCompute super;
    struct BNVertex* condition;
    struct BNVertex* consequent;
    struct BNVertex* alternative;
};

#endif
