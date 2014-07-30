#ifndef INFER_H_
#define INFER_H_

#include "../defs.h"

struct pp_trace_store_t {
	struct pp_instance_t* instance;
    int num_iters;
    int num_verts;
    int num_accepts;
    float t[1];
};

int rejection_sampling(struct pp_instance_t* instance, acceptor_t accept,
                       name_to_value_t F, void* raw_data, 
                       vertices_handler_t add_trace, void* add_trace_data);

int trace_store_insert(struct pp_instance_t* instance, void* raw_data);

float getsample(struct BNVertexDraw* vertexDraw);
float getcomp(struct BNVertexCompute* vertexComp);

/************************************** Sample Functions ******************************************/

float flip(float p);
float flipD();
float log_flip(float p);

float multinomial(float theta[], int n);
float multinomial_logprob(float m, float theta[], int n);

float uniform(float low, float high);
float uniformDiscrete(int low, int high);

float gaussian(float mu, float sigma);
float guassian_logprob(float x, float mu, float sigma);

float gamma1(float a, float b);
float gamma_logprob(float x, float a, float b);

float beta(float a, float b);
float beta_logprob(float x, float a, float b);

float binomial(float p, int n);
float binomial_lgoprob(float s, float p, int n);

float poisson(int mu);
float poisson_logprob(float k, int mu);

float* dirichlet(float alpha[], int n);
float dirchlet_lgoprob(float theta[], float alpha[]);

#endif
