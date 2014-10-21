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

