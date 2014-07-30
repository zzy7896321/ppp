#include "../ppp.h"
#include "../defs.h"
#include "infer.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_sample_iterations = 10000;

static void init_sample_iterations()
{
    char* sample_iterations = getenv("SAMPLE_ITERATIONS");
    if (sample_iterations) {
        g_sample_iterations = atoi(sample_iterations);
        fprintf(stderr, "init_sample_iterations (%d): g_sample_iterations=%d\n", __LINE__, g_sample_iterations);
    }
}
char * dump_vertex(struct BNVertex* vertex){
    char vertex_type[16],*retstr;
    retstr=calloc(128,sizeof(char));
    switch(vertex->type){
    case BNV_CONST:
        strcpy(vertex_type,"BNV_CONST");
        break;
    case BNV_DRAW:
        strcpy(vertex_type,"BNV_DRAW");
        break;
    case BNV_COMPU:
        strcpy(vertex_type,"BNV_COMPU");
        break;
    }
    sprintf(retstr,"[type]%s [value]%f",vertex_type,vertex->sample);
    return retstr;
}
int gibbs_sampling(struct pp_instance_t* instance, acceptor_t condition_accept,
        name_to_value_t F, void* raw_data,
        vertices_handler_t add_trace, void* add_trace_data){
    int k, i, n;
    n = pp_instance_num_vertices(instance);

    /*INIT init the t0 values*/
    for(k=0;k<1000;k++){
        for(i=0;i<n;i++){
            struct BNVertex* vertex = pp_instance_vertex(instance, i);
            const char *vertex_name=pp_instance_vertex_name(instance, i);
            if (vertex->type == BNV_CONST)
                vertex->sample = vertex->sample;  /* do nothing */
            else if (vertex->type == BNV_DRAW)
                vertex->sample = getsample((struct BNVertexDraw*)vertex);
            else if (vertex->type == BNV_COMPU)
                vertex->sample = getcomp((struct BNVertexCompute*)vertex);
            fprintf(stdout,"Vertex %d : [name]%s %s\n",i,vertex_name,dump_vertex(vertex));
        }
        add_trace(instance, add_trace_data);
    }
    /*ITERATION*/
    for (k = 0; k < g_sample_iterations; ++k) {
        for (i = 0; i < n; ++i) {
            struct BNVertex* vertex = pp_instance_vertex(instance, i);
            if (vertex->type == BNV_CONST)
                vertex->sample = vertex->sample;  /* do nothing */
            else if (vertex->type == BNV_DRAW)
                vertex->sample = getsample((struct BNVertexDraw*)vertex);
            else if (vertex->type == BNV_COMPU)
                vertex->sample = getcomp((struct BNVertexCompute*)vertex);
        }
        if (condition_accept(instance, F, raw_data)) {
            add_trace(instance, add_trace_data);
        }
    }
    return 0;
}
struct pp_trace_store_t* pp_sample(struct pp_instance_t* instance, struct pp_query_t* query)
{
    char infer_type='g';
    int n;
    struct pp_trace_store_t* traces;

    fprintf(stdout,"1\n");
    /*init_sample_iterations();*/

    n = pp_instance_num_vertices(instance);
    fprintf(stdout,"2\n");
    traces = malloc(sizeof(struct pp_trace_store_t) + (g_sample_iterations * n * sizeof(float)));
    fprintf(stdout,"3\n");
    traces->instance = instance;
    traces->num_iters = g_sample_iterations;
    traces->num_verts = n;
    traces->num_accepts = 0;
    fprintf(stdout,"4\n");
    if(infer_type=='g'){
        gibbs_sampling(instance, pp_query_acceptor, pp_name_to_value, query, trace_store_insert, traces);
    }
    else if(infer_type=='m'){
        gibbs_sampling(instance, pp_query_acceptor, pp_name_to_value, query, trace_store_insert, traces);
    }
    else{
        rejection_sampling(instance, pp_query_acceptor, pp_name_to_value, query, trace_store_insert, traces);
    }

    return traces;
}

int pp_get_result(struct pp_trace_store_t* traces, struct pp_query_t* query, float* result)
{
    int i, j, n;
    float count = 0;
    struct pp_instance_t* instance = traces->instance;
    n = pp_instance_num_vertices(instance);
    for (i = 0; i < traces->num_accepts; ++i) {
        for (j = 0; j < n; ++j) {
            pp_instance_vertex(instance, j)->sample = traces->t[i * traces->num_verts + j];
        }
        if (pp_query_acceptor(instance, pp_name_to_value, query)) ++count;
    }
    *result = count / traces->num_accepts;
    return 0;
}

/* P(X|Y) */
int pp_infer(struct pp_instance_t* instance, const char* X, const char* Y, float* result)
{
    struct pp_query_t* query;
    struct pp_trace_store_t* traces;

    query = pp_compile_query(instance, Y);
    traces = pp_sample(instance, query);
    query = pp_compile_query(instance, X);
    pp_get_result(traces, query, result);

    return 0;
}

int rejection_sampling(struct pp_instance_t* instance, acceptor_t condition_accept, 
                       name_to_value_t F, void* raw_data,
                       vertices_handler_t add_trace, void* add_trace_data)
{
    int k, i, n;
    n = pp_instance_num_vertices(instance);
    for (k = 0; k < g_sample_iterations; ++k) {
        for (i = 0; i < n; ++i) {
            struct BNVertex* vertex = pp_instance_vertex(instance, i);
            if (vertex->type == BNV_CONST)
                vertex->sample = vertex->sample;  /* do nothing */
            else if (vertex->type == BNV_DRAW)
                vertex->sample = getsample((struct BNVertexDraw*)vertex);
            else if (vertex->type == BNV_COMPU)
                vertex->sample = getcomp((struct BNVertexCompute*)vertex);
        }
        if (condition_accept(instance, F, raw_data)) {
            add_trace(instance, add_trace_data);
        }
    }
    return 0;
}

int mh_sampling(struct pp_instance_t* instance, acceptor_t condition_accept, 
            name_to_value_t F, void* raw_data,
            vertices_handler_t add_trace, void* add_trace_data)
{
    return 0;
}
int trace_store_insert(struct pp_instance_t* instance, void* raw_data)
{
    struct pp_trace_store_t* traces;
    int i;
    int n;

    n = pp_instance_num_vertices(instance);
    traces = (struct pp_trace_store_t*)raw_data;
    for (i = 0; i < n; ++i) {
        traces->t[traces->num_accepts * n + i] = pp_instance_vertex(instance, i)->sample;
    }
    ++traces->num_accepts;
    return 0;
}

float getsample(struct BNVertexDraw* vertexDraw) 
{
    switch (vertexDraw->type) {
        case FLIP: return flip(((struct BNVertexDrawBern*)vertexDraw)->p->sample);
        case LOG_FLIP: return log_flip(((struct BNVertexDrawBern*)vertexDraw)->p->sample);
        case GAUSSIAN: {
            struct BNVertexDrawNorm* vertexNorm = (struct BNVertexDrawNorm*)vertexDraw;
            return gaussian(vertexNorm->mean->sample, vertexNorm->variance->sample);
        }
        case GAMMA: {
            struct BNVertexDrawGamma* vertexGamma = (struct BNVertexDrawGamma*)vertexDraw;
            return gamma1(vertexGamma->a->sample, vertexGamma->b->sample);
        }
        /* FIXME getsample for more distributions */
        default: return 0;
    }
}

float getcomp(struct BNVertexCompute* vertexComp) 
{
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
            if (vertexIf->condition->sample)
                return vertexIf->consequent->sample;
            else
                return vertexIf->alternative->sample;
        }
        case BNVC_UNARY: {
            struct BNVertexComputeUnary* vertex = (struct BNVertexComputeUnary*)vertexComp;
            if (vertex->op == UNARY_NEG)
                return -(vertex->primary->sample);
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

