#include "defs.h"
#include <stdio.h>

#ifdef DEBUG
	#define ERR_OUTPUT(...) \
		fprintf(stderr, "%s(%d): ", __FILE__, __LINE__); \
		fprintf(stderr,  __VA_ARGS__)
#else
	#define ERR_OUTPUT(...) 
#endif

void dump_vertex(struct BNVertex* vertex);
void dump_vertexDraw(struct BNVertexDraw* vertexDraw);
void dump_vertexDrawBern(struct BNVertexDrawBern* vertexBern);
void dump_vertexDrawNorm(struct BNVertexDrawNorm* vertexNorm);
void dump_vertexDrawGamma(struct BNVertexDrawGamma* vertexGamma);
void dump_vertexCompute(struct BNVertexCompute* vertexCompute);
void dump_vertexComputeFunc(struct BNVertexComputeFunc* vertexComputeFunc);
void dump_vertexComputeUnary(struct BNVertexComputeUnary* vertexComputeUnary);
void dump_vertexComputeBinop(struct BNVertexComputeBinop* vertexComputeBinop);
void dump_vertexComputeIf(struct BNVertexComputeIf* vertexComputeIf);

/* dump facilities */

/**
 * DUMP_START_VAR_NAME should be called before any call to the subsequent DUMP facilities.
 * all calls should specify the same num_written variable.
 */
#define DUMP_START_VAR_NAME(num_written)	\
	int num_written = 0

/**
 * func should have the prototype:
 * int func(char* buffer, int buf_size, <at least one parameter here> );
 *
 * func should return the number of characters written on success (not including the terminating null-byte),
 * or negative value if an error occured. If the written content gets truncated due to buf_size limit, func
 * should return the number of characters that would be written if no limit was imposed.
 *
 * If func fails, this macro returns immediately with the same error number.
 */
#define DUMP_CALL_VAR_NAME(num_written, func, buffer, buf_size, ...)	\
	do {	\
		int ret = func(buffer + num_written, (buf_size >= num_written) ? (buf_size - num_written) : 0, __VA_ARGS__);	\
		if (ret < 0) return ret;	\
		num_written += ret;	\
	} while (0)


/**
 * DUMP with snprintf. Equivalent to DUMP_CALL_VAR_NAME(num_written, snprintf, buffer, buf_size, ...).
 */
#define DUMP_VAR_NAME(num_written, buffer, buf_size, ...)	\
	DUMP_CALL_VAR_NAME(num_written, snprintf, buffer, buf_size, __VA_ARGS__)

/**
 * Returns num_written.
 */
#define DUMP_SUCCESS_VAR_NAME(num_written)	\
	return num_written


/**
 * Returns with error.
 */
#define DUMP_ERROR_VAR_NAME(num_written, error_value)	\
	return error_value


/* the following DUMP uses the default name of num_written */
#define DUMP_START() DUMP_START_VAR_NAME(num_written)
#define DUMP_CALL(func, buffer, buf_size, ...) DUMP_CALL_VAR_NAME(num_written, func, buffer, buf_size, __VA_ARGS__)
#define DUMP(buffer, buf_size, ...) DUMP_VAR_NAME(num_written, buffer, buf_size, __VA_ARGS__)
#define DUMP_SUCCESS() DUMP_SUCCESS_VAR_NAME(num_written)
#define DUMP_ERROR(error_value) DUMP_ERROR_VAR_NAME(num_written, error_value)

