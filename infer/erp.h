#ifndef ERP_H
#define ERP_H

#include <stddef.h>

typedef enum erp_enum_t {
	ERP_UNKNOWN = 0,
    ERP_FLIP,
    ERP_MULTINOMIAL,
    ERP_UNIFORM,
    ERP_GAUSSIAN,
    ERP_GAMMA,
    ERP_BETA,
    ERP_BINOMIAL,
    ERP_POISSON,
    ERP_DIRICHLET,

    NUMBER_OF_ERP
} erp_enum_t;

extern char* ERP_NAME[NUMBER_OF_ERP];
#define erp_name(erp_type) ERP_NAME[erp_type]

extern size_t ERP_NUM_OF_PARAM[NUMBER_OF_ERP];
#define erp_num_of_param(erp_type) ERP_NUM_OF_PARAM[erp_type]

erp_enum_t erp_type(const char* erp_name);

/************************************** Sample Functions ******************************************/

float randomC();
float randomR();
float randomL();

int flip(float p);
float flip_logprob(int value, float p);

float flipD();
float flipD_logprob(float value);

float log_flip(float p);
float log_flip_logprob(float value, float p);

int multinomial(float theta[], int n);
float multinomial_logprob(int m, float theta[], int n);

float uniform(float low, float high);
float uniformDiscrete(int low, int high);

float gaussian(float mu, float sigma);
float gaussian_logprob(float x, float mu, float sigma);

float gamma1(float a, float b); // avoid name conflict with gcc built-in function gamma
float gamma_logprob(float x, float a, float b);

float beta(float a, float b);
float beta_logprob(float x, float a, float b);

float binomial(float p, int n);
float binomial_lgoprob(float s, float p, int n);

float poisson(int mu);
float poisson_logprob(float k, int mu);

float* dirichlet(float alpha[], int n);
float dirichlet_logprob(float theta[], float alpha[], int n);

#endif
