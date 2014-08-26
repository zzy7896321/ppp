#ifndef ERP_H
#define ERP_H

typedef enum erp_enum_t {
	ERP_UNKNOWN,
    ERP_FLIP,
    ERP_MULTINOMIAL,
    ERP_UNIFORM,
    ERP_GAUSSIAN,
    ERP_GAMMA,
    ERP_BETA,
    ERP_BINOMIAL,
    ERP_POISSON,
    ERP_DIRICHLET,
} erp_enum_t;

const char* erp_name(erp_enum_t erp_type);

erp_enum_t erp_type(const char* erp_name);

#endif
