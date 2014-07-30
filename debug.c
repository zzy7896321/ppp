#include "debug.h"
#include <stdio.h>

void dump_vertex(struct BNVertex* vertex) {
	printf("vertex 0x%08x:", vertex);
	
	switch (vertex->type) {
	case BNV_CONST:
		printf(" CONST, sample = %f", vertex->sample);
		break;
	case BNV_DRAW:
		printf(" DRAW, sample = %f", vertex->sample);
		dump_vertexDraw((struct BNVertexDraw*) vertex);
		break;
	case BNV_COMPU:
		printf(" COMPUTE, sample = %f", vertex->sample);
		dump_vertexCompute((struct BNVertexCompute*) vertex);
		break;
	default:
		printf(" UNKNOWN");
	}

	printf("\n");
}

void dump_vertexDraw(struct BNVertexDraw* vertexDraw) {
	switch (vertexDraw->type) {
	case FLIP:
		printf(", FLIP");
		dump_vertexDrawBern((struct BNVertexDrawBern*) vertexDraw);
		break;
	case LOG_FLIP:
		printf(", LOG_FLIP");
		dump_vertexDrawBern((struct BNVertexDrawBern*) vertexDraw);
		break;
	case GAUSSIAN:
		printf(", GAUSSIAN");
		dump_vertexDrawNorm((struct BNVertexDrawNorm*) vertexDraw);
		break;
	case GAMMA:
		printf(", GAMMA");
		dump_vertexDrawGamma((struct BNVertexDrawGamma*) vertexDraw);
		break;
	default:
		printf(", UNKNOWN");
	}
}

void dump_vertexDrawBern(struct BNVertexDrawBern* vertexBern) {
	printf(", p_node = 0x%08x, p = %f", vertexBern->p, vertexBern->p->sample);
}

void dump_vertexDrawNorm(struct BNVertexDrawNorm* vertexNorm) {
	printf(", mu_node = 0x%08x, mu = %f, var_node = 0x%08x, var = %f",
			vertexNorm->mean, vertexNorm->mean->sample,
			vertexNorm->variance, vertexNorm->variance->sample);
}

void dump_vertexDrawGamma(struct BNVertexDrawGamma* vertexGamma) {
	printf(", a_node = 0x%08x, a = %f, b_node = 0x%08x, b = %f",
			vertexGamma->a, vertexGamma->a->sample,
			vertexGamma->b, vertexGamma->b->sample);
}

void dump_vertexCompute(struct BNVertexCompute* vertexCompute) {
	
	switch (vertexCompute->type) {
	case BNVC_IF:
		printf(", IF");
		dump_vertexComputeIf((struct BNVertexComputeIf*) vertexCompute);
		break;
	case BNVC_BINOP:
		printf(", BINOP");
		dump_vertexComputeBinop((struct BNVertexComputeBinop*) vertexCompute);
		break;
	case BNVC_UNARY:
		printf(", UNARY");
		dump_vertexComputeUnary((struct BNVertexComputeUnary*) vertexCompute);
		break;
	case BNVC_FUNC:
		printf(", FUNC");
		dump_vertexComputeFunc((struct BNVertexComputeFunc*) vertexCompute);
		break;
	default:
		printf(", UNKNOWN");
	}
}

void dump_vertexComputeFunc(struct BNVertexComputeFunc* vertexComputeFunc) {
	switch (vertexComputeFunc->func) {
	case FUNC_LOG:
		printf(", LOG, arg_node = 0x%08x, arg = %f", vertexComputeFunc->args[0], vertexComputeFunc->args[0]->sample);
		break;
	case FUNC_EXP:
		printf(", EXP, arg_node = 0x%08x, arg = %f", vertexComputeFunc->args[0], vertexComputeFunc->args[0]->sample);
		break;
	default:
		printf(", UNKNOWN");
	}
}


void dump_vertexComputeUnary(struct BNVertexComputeUnary* vertexComputeUnary) {
	switch (vertexComputeUnary->op) {
	case UNARY_NEG:
		printf(", NEG, operand_node = 0x%08x, operand = %f", vertexComputeUnary->primary, vertexComputeUnary->primary->sample);
		break;
	case UNARY_POS:
		printf(", POS, operand_node = 0x%08x, opearnd = %f", vertexComputeUnary->primary, vertexComputeUnary->primary->sample);
		break;
	default:
		printf(", UNKNOWN");
	}

}
void dump_vertexComputeBinop(struct BNVertexComputeBinop* vertexComputeBinop) {
	switch (vertexComputeBinop->binop) {
	case BINOP_PLUS:
		printf(", PLUS, left_node = 0x%08x, left = %f, right_node = 0x%08x, right = %f",
				vertexComputeBinop->left, vertexComputeBinop->left->sample,
				vertexComputeBinop->right, vertexComputeBinop->right->sample);
		break;
	case BINOP_SUB:
		printf(", SUB, left_node = 0x%08x, left = %f, right_node = 0x%08x, right = %f",
				vertexComputeBinop->left, vertexComputeBinop->left->sample,
				vertexComputeBinop->right, vertexComputeBinop->right->sample);
		break;
	case BINOP_MULTI:
		printf(", MULTI, left_node = 0x%08x, left = %f, right_node = 0x%08x, right = %f",
				vertexComputeBinop->left, vertexComputeBinop->left->sample,
				vertexComputeBinop->right, vertexComputeBinop->right->sample);
		break;
	case BINOP_DIV:
		printf(", DIV, left_node = 0x%08x, left = %f, right_node = 0x%08x, right = %f",
				vertexComputeBinop->left, vertexComputeBinop->left->sample,
				vertexComputeBinop->right, vertexComputeBinop->right->sample);
		break;
	default:
		printf(", UNKNOWN");
	}
}

void dump_vertexComputeIf(struct BNVertexComputeIf* vertexComputeIf) {

	printf(", IF, condition_node = 0x%08x, condition = %f, consequence_node = 0x%08x, consequence  = %f, alternative_node = 0x%08x, alternative = %f",
			vertexComputeIf->condition, vertexComputeIf->condition->sample,
			vertexComputeIf->consequent, vertexComputeIf->consequent->sample,
			vertexComputeIf->alternative, vertexComputeIf->alternative->sample);
}

