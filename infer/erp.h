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

#endif
