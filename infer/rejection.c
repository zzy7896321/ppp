#include "infer.h"
#include "rejection.h"
#include "../debug.h"

/* cannot destroy temporaries in op functions */
#define BINARY_OP_RETURN(status)	\
	return status

int rejection_sampling_execute_add(pp_variable_t* left, pp_variable_t* right, pp_variable_t** result_ptr) {
	switch (left->type) {
	case INT:
		switch (right->type) {
		case INT:
			{
				*result_ptr = new_pp_int(PP_VARIABLE_INT_VALUE(left) + PP_VARIABLE_INT_VALUE(right));
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}			
			break;
		case FLOAT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_INT_VALUE(left) + PP_VARIABLE_FLOAT_VALUE(right));
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		case VECTOR:
			{
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_INVALID_OPEARND_TYPE);
			}
			break;
		}
		break;
	case FLOAT:
		switch (right->type) {
		case INT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_FLOAT_VALUE(left) + PP_VARIABLE_INT_VALUE(right));
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}			
			break;
		case FLOAT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_FLOAT_VALUE(left) + PP_VARIABLE_FLOAT_VALUE(right));
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		case VECTOR:
			{
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_INVALID_OPEARND_TYPE);
			}
			break;
		}
		break;
	case VECTOR:
		switch (right->type) {
		case INT:
			{
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_INVALID_OPEARND_TYPE);
			}			
			break;
		case FLOAT:
			{
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_INVALID_OPEARND_TYPE);
			}
			break;
		case VECTOR:
			{	
				size_t l_length = PP_VARIABLE_VECTOR_LENGTH(left);
				size_t r_length = PP_VARIABLE_VECTOR_LENGTH(right);

				if (l_length != r_length) {
					BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_VECTOR_LENGTH_MISMATCH);
				}

				pp_variable_t* result = new_pp_vector(PP_VARIABLE_VECTOR_CAPACITY(left));
				for (l_length = 0; l_length != r_length; ++l_length) {
					int status = 
						rejection_sampling_execute_add(	PP_VARIABLE_VECTOR_VALUE(left)[l_length],
														PP_VARIABLE_VECTOR_VALUE(right)[l_length],
														&(PP_VARIABLE_VECTOR_VALUE(result)[l_length]));
					if (status != PP_SAMPLE_FUNCTION_NORMAL) {
						ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
						pp_variable_destroy(result);
						BINARY_OP_RETURN(status);
					}
					else {
						++PP_VARIABLE_VECTOR_LENGTH(result);
					}
				}

				*result_ptr = result;
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		}
		break;
	}

	BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_UNHANDLED);
}

int rejection_sampling_execute_sub(pp_variable_t* left, pp_variable_t* right, pp_variable_t** result_ptr) {
	switch (left->type) {
	case INT:
		switch (right->type) {
		case INT:
			{
				*result_ptr = new_pp_int(PP_VARIABLE_INT_VALUE(left) - PP_VARIABLE_INT_VALUE(right));
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}			
			break;
		case FLOAT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_INT_VALUE(left) - PP_VARIABLE_FLOAT_VALUE(right));
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		case VECTOR:
			{
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_INVALID_OPEARND_TYPE);
			}
			break;
		}
		break;
	case FLOAT:
		switch (right->type) {
		case INT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_FLOAT_VALUE(left) - PP_VARIABLE_INT_VALUE(right));
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}			
			break;
		case FLOAT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_FLOAT_VALUE(left) - PP_VARIABLE_FLOAT_VALUE(right));
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		case VECTOR:
			{
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_INVALID_OPEARND_TYPE);
			}
			break;
		}
		break;
	case VECTOR:
		switch (right->type) {
		case INT:
			{
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_INVALID_OPEARND_TYPE);
			}			
			break;
		case FLOAT:
			{
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_INVALID_OPEARND_TYPE);
			}
			break;
		case VECTOR:
			{	
				size_t l_length = PP_VARIABLE_VECTOR_LENGTH(left);
				size_t r_length = PP_VARIABLE_VECTOR_LENGTH(right);

				if (l_length != r_length) {
					BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_VECTOR_LENGTH_MISMATCH);
				}

				pp_variable_t* result = new_pp_vector(PP_VARIABLE_VECTOR_CAPACITY(left));
				for (l_length = 0; l_length != r_length; ++l_length) {
					int status = 
						rejection_sampling_execute_sub(	PP_VARIABLE_VECTOR_VALUE(left)[l_length],
														PP_VARIABLE_VECTOR_VALUE(right)[l_length],
														&(PP_VARIABLE_VECTOR_VALUE(result)[l_length]));
					if (status != PP_SAMPLE_FUNCTION_NORMAL) {
						ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
						pp_variable_destroy(result);
						BINARY_OP_RETURN(status);
					}
					else {
						++PP_VARIABLE_VECTOR_LENGTH(result);
					}
				}

				*result_ptr = result;
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		}
		break;
	}

	BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_UNHANDLED);
}
int rejection_sampling_execute_mul(pp_variable_t* left, pp_variable_t* right, pp_variable_t** result_ptr) {
	switch (left->type) {
	case INT:
		switch (right->type) {
		case INT:
			{
				*result_ptr = new_pp_int(PP_VARIABLE_INT_VALUE(left) * PP_VARIABLE_INT_VALUE(right));
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}			
			break;
		case FLOAT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_INT_VALUE(left) * PP_VARIABLE_FLOAT_VALUE(right));
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		case VECTOR:
			{
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_INVALID_OPEARND_TYPE);
			}
			break;
		}
		break;
	case FLOAT:
		switch (right->type) {
		case INT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_FLOAT_VALUE(left) * PP_VARIABLE_INT_VALUE(right));
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}			
			break;
		case FLOAT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_FLOAT_VALUE(left) * PP_VARIABLE_FLOAT_VALUE(right));
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		case VECTOR:
			{
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_INVALID_OPEARND_TYPE);
			}
			break;
		}
		break;
	case VECTOR:
		BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_UNHANDLED);
		break;
	}

	BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_UNHANDLED);
}

int rejection_sampling_execute_div(pp_variable_t* left, pp_variable_t* right, pp_variable_t** result_ptr) {
	switch (left->type) {
	case INT:
		switch (right->type) {
		case INT:
			{
				if (!PP_VARIABLE_INT_VALUE(right)) {
					BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_DIVISION_BY_ZERO);
				}
				*result_ptr = new_pp_int(PP_VARIABLE_INT_VALUE(left) / PP_VARIABLE_INT_VALUE(right));
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}			
			break;
		case FLOAT:
			{
				if (!PP_VARIABLE_INT_VALUE(right)) {
					BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_DIVISION_BY_ZERO);
				}
				*result_ptr = new_pp_float(PP_VARIABLE_INT_VALUE(left) / PP_VARIABLE_FLOAT_VALUE(right));
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		case VECTOR:
			{
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_INVALID_OPEARND_TYPE);
			}
			break;
		}
		break;
	case FLOAT:
		switch (right->type) {
		case INT:
			{
				if (!PP_VARIABLE_INT_VALUE(right)) {
					BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_DIVISION_BY_ZERO);
				}
				*result_ptr = new_pp_float(PP_VARIABLE_FLOAT_VALUE(left) / PP_VARIABLE_INT_VALUE(right));
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}			
			break;
		case FLOAT:
			{
				if (!PP_VARIABLE_INT_VALUE(right)) {
					BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_DIVISION_BY_ZERO);
				}
				*result_ptr = new_pp_float(PP_VARIABLE_FLOAT_VALUE(left) / PP_VARIABLE_FLOAT_VALUE(right));
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		case VECTOR:
			{
				BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_INVALID_OPEARND_TYPE);
			}
			break;
		}
		break;
	case VECTOR:
		BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_UNHANDLED);
		break;
	}

	BINARY_OP_RETURN(PP_SAMPLE_FUNCTION_UNHANDLED);
}

#undef BINARY_OP_RETURN

int rejection_sampling_execute_if_expr(IfExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr) {
	pp_variable_t* condition = 0;
	{
		int status = rejection_sampling_execute_expr((ExprNode*) expr->condition, trace, &condition);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
			return status;
		}
	}

	int condition_true = 0;
	switch (condition->type) {
	case INT:
		condition_true = !!PP_VARIABLE_INT_VALUE(condition);
		break;
	case FLOAT:
		condition_true = !!PP_VARIABLE_FLOAT_VALUE(condition);
		break;
	case VECTOR:
		return PP_SAMPLE_FUNCTION_NON_SCALAR_TYPE_AS_CONDITION;
	}
	pp_variable_destroy(condition);

	ExprNode* taken_branch = (condition_true) ? expr->consequent : expr->alternative;
	return rejection_sampling_execute_expr(taken_branch, trace, result_ptr);
}

int rejection_sampling_execute_new_expr(NewExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr) {
	return PP_SAMPLE_FUNCTION_UNHANDLED;	
}

int rejection_sampling_execute_binary_expr(BinaryExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr) {
	pp_variable_t* left = 0;
	pp_variable_t* right = 0;

	{
		int status = rejection_sampling_execute_expr((ExprNode*) expr->left, trace, &left);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
			return status;
		}
	}
	{
		int status = rejection_sampling_execute_expr((ExprNode*) expr->right, trace, &right);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
			pp_variable_destroy(left);
			return status;
		}
	}

	#define SAFE_RETURN(op)	\
		{	\
			int status = rejection_sampling_execute_##op(left, right, result_ptr);	\
			pp_variable_destroy(left);	\
			pp_variable_destroy(right);	\
			return status;	\
		}

	switch (expr->op) {
	case OP_ADD:
		SAFE_RETURN(add)
	case OP_SUB:
		SAFE_RETURN(sub)
	case OP_MUL:
		SAFE_RETURN(mul)
	case OP_DIV:
		SAFE_RETURN(div)
	}	

	#undef SAFE_RETURN

	pp_variable_destroy(left);
	pp_variable_destroy(right);
	return PP_SAMPLE_FUNCTION_UNHANDLED;
}

int rejection_sampling_execute_primary_expr(PrimaryExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr) {
	switch (expr->type) {
	case NUM_EXPR:
		{
			NumericalValueNode* numerical_value = ((NumExprNode*) expr)->numerical_value;
			switch (numerical_value->type) {
			case REAL_VALUE:
				{
					*result_ptr = new_pp_float(((RealValueNode*) numerical_value)->value);
					return PP_SAMPLE_FUNCTION_NORMAL;
				}
			case INTEGER_VALUE:
				{
					*result_ptr = new_pp_int(((IntegerValueNode*) numerical_value)->value);
					return PP_SAMPLE_FUNCTION_NORMAL;
				}
			}
		}
		break;
	case VAR_EXPR:
		{
			VariableNode* variable = ((VarExprNode*) expr)->variable;
			switch (variable->type) {
			case NAME_VAR:
				{
					NameVarNode* name_var = (NameVarNode*) variable;
					const char* name = symbol_to_string(node_symbol_table(name_var), name_var->name);
					pp_variable_t* var = pp_trace_find_variable(trace, name);
					if (!var) {
						return PP_SAMPLE_FUNCTION_VARIABLE_NOT_FOUND;
					}

					*result_ptr = pp_variable_clone(var); 
					return PP_SAMPLE_FUNCTION_NORMAL;
				}
			case FIELD_VAR:
				{
					return PP_SAMPLE_FUNCTION_UNHANDLED;
				}
			case INDEX_VAR:
				{
					IndexVarNode* index_var = (IndexVarNode*) expr;
					const char* name = symbol_to_string(node_symbol_table(index_var), index_var->name);
					pp_variable_t* var = pp_trace_find_variable(trace, name);
					if (!var) {
						return PP_SAMPLE_FUNCTION_VARIABLE_NOT_FOUND;
					}

					ExprSeqNode* expr_seq = index_var->expr_seq;
					while (expr_seq) {
						if (!var || var->type != VECTOR) {
							return PP_SAMPLE_FUNCTION_SUBSCRIPTING_TO_NON_VECTOR;
						}
						ExprNode* expr = expr_seq->expr;
						pp_variable_t* sub = 0;
						int status = rejection_sampling_execute_expr(expr, trace, &sub);
						if (status != PP_SAMPLE_FUNCTION_NORMAL) {
							ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
							return status;
						}

						if (sub->type != INT) {
							pp_variable_destroy(sub);
							return PP_SAMPLE_FUNCTION_NON_INTEGER_SUBSCRIPTION;
						}
						var = PP_VARIABLE_VECTOR_VALUE(var)[PP_VARIABLE_INT_VALUE(sub)];
						pp_variable_destroy(sub);

						expr_seq = expr_seq->expr_seq;
					}

					*result_ptr = pp_variable_clone(var);
					return PP_SAMPLE_FUNCTION_NORMAL;
				}
			}
		}
		break;
	case UNARY_EXPR:
		{
			UnaryExprNode* unary = (UnaryExprNode*) expr;
			switch (unary->op) {
			case OP_NEG:
				{
					pp_variable_t* var = 0;
					{
						int status = rejection_sampling_execute_primary_expr(unary->primary, trace, &var);
						if (status != PP_SAMPLE_FUNCTION_NORMAL) {
							ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
							return status;
						}
					}
					switch (var->type) {
					case INT:
						PP_VARIABLE_INT_VALUE(var) = -PP_VARIABLE_INT_VALUE(var);
						*result_ptr = var;
						return PP_SAMPLE_FUNCTION_NORMAL;
					case FLOAT:
						PP_VARIABLE_FLOAT_VALUE(var) = -PP_VARIABLE_FLOAT_VALUE(var);
						*result_ptr = var;
						return PP_SAMPLE_FUNCTION_NORMAL;
					case VECTOR:
						pp_variable_destroy(var);
						return PP_SAMPLE_FUNCTION_INVALID_OPEARND_TYPE;
					}
				}
			}
		}
		break;
	case GROUP_EXPR:
		{
			return rejection_sampling_execute_expr(((GroupExprNode*) expr)->expr, trace, result_ptr);
		}
		//break;
	case FUNC_EXPR:
		{
			return PP_SAMPLE_FUNCTION_UNHANDLED;
		}
		//break;
	}
	ERR_OUTPUT("\n");
	return PP_SAMPLE_FUNCTION_UNHANDLED;
}

int rejection_sampling_execute_expr(ExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr) {
	if (!expr) {
		return PP_SAMPLE_FUNCTION_INVALID_EXPRESSION;
	}	

	switch (expr->type) {
	case IF_EXPR:
		return rejection_sampling_execute_if_expr((IfExprNode*) expr, trace, result_ptr);
	case NEW_EXPR:
		return rejection_sampling_execute_new_expr((NewExprNode*) expr, trace, result_ptr);
	case BINARY_EXPR:
		return rejection_sampling_execute_binary_expr((BinaryExprNode*) expr, trace, result_ptr);
	case PRIMARY_EXPR:
		return rejection_sampling_execute_primary_expr((PrimaryExprNode*) expr, trace, result_ptr);
	}
	ERR_OUTPUT("\n");
	return PP_SAMPLE_FUNCTION_UNHANDLED;
}

int rejection_sampling_get_parameters(ExprSeqNode* expr_seq, pp_trace_t* trace, size_t num_expected, pp_variable_t*** result_ptr) {
	#define REJECTION_SAMPLING_GET_PARAM_CLEAR_RETURN(status)	\
			for (size_t j = 0; j < i; ++j) {	\
				pp_variable_destroy(result[i]);	\
			}	\
			free(result);	\
			return status


	pp_variable_t** result = malloc(sizeof(pp_variable_t*) * num_expected);
	size_t i;

	for (i = 0; i != num_expected && expr_seq; ++i, expr_seq = expr_seq->expr_seq) {
		int status = rejection_sampling_execute_expr(expr_seq->expr, trace, &(result[i]));
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
			REJECTION_SAMPLING_GET_PARAM_CLEAR_RETURN(status);
		}
	}

	if (i != num_expected || expr_seq) {
		REJECTION_SAMPLING_GET_PARAM_CLEAR_RETURN(PP_SAMPLE_FUNCTION_NUMBER_OF_PARAMETER_MISMATCH);
	}

	#undef REJECTION_SAMPLING_GET_PARAM_CLEAR_RETURN

	*result_ptr = result;
	return PP_SAMPLE_FUNCTION_NORMAL;
}

int rejection_sampling_get_variable_ptr(VariableNode* variable, pp_trace_t* trace, pp_variable_t*** result_ptr) {

	switch (variable->type) {
	case NAME_VAR:
		*result_ptr = &(pp_trace_get_variable(trace, symbol_to_string(node_symbol_table(variable), ((NameVarNode*) variable)->name)));
		return PP_SAMPLE_FUNCTION_NORMAL;
	case FIELD_VAR:
		return PP_SAMPLE_FUNCTION_UNHANDLED;
		//break;
	case INDEX_VAR:
		{
			pp_variable_t* vec = pp_trace_find_variable(trace, symbol_to_string(node_symbol_table(variable), ((IndexVarNode*) variable)->name));
			if (!vec) {
				return PP_SAMPLE_FUNCTION_VARIABLE_NOT_FOUND;
			}

			ExprSeqNode* expr_seq = ((IndexVarNode*) variable)->expr_seq;
			if (!expr_seq) {
				return PP_SAMPLE_FUNCTION_INVALID_EXPRESSION;
			}
			while (expr_seq->expr_seq) {
				if (!vec || vec->type != VECTOR) {
					return PP_SAMPLE_FUNCTION_SUBSCRIPTING_TO_NON_VECTOR;
				}

				ExprNode* expr = expr_seq->expr;
				pp_variable_t* sub = 0;
				int status = rejection_sampling_execute_expr(expr, trace, &sub);
				if (status != PP_SAMPLE_FUNCTION_NORMAL) {
					ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
					return status;
				}
				if (sub->type != INT) {
					return PP_SAMPLE_FUNCTION_NON_INTEGER_SUBSCRIPTION;
				}

				vec = PP_VARIABLE_VECTOR_VALUE(vec)[PP_VARIABLE_INT_VALUE(sub)];
				pp_variable_destroy(sub);

				expr_seq = expr_seq->expr_seq;
			}

			{
				if (!vec || vec->type != VECTOR) {
					return PP_SAMPLE_FUNCTION_SUBSCRIPTING_TO_NON_VECTOR;
				}

				ExprNode* expr = expr_seq->expr;
				pp_variable_t* sub = 0;
				int status = rejection_sampling_execute_expr(expr, trace, &sub);
				if (status != PP_SAMPLE_FUNCTION_NORMAL) {
					ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
					return status;
				}
				if (sub->type != INT) {
					return PP_SAMPLE_FUNCTION_NON_INTEGER_SUBSCRIPTION;
				}

				*result_ptr = &(PP_VARIABLE_VECTOR_VALUE(vec)[PP_VARIABLE_INT_VALUE(sub)]);
				return PP_SAMPLE_FUNCTION_NORMAL;
			}
		}
	}

	return PP_SAMPLE_FUNCTION_UNHANDLED;
}

int rejection_sampling_execute_draw_stmt(DrawStmtNode* stmt, pp_trace_t* trace) {

	pp_variable_t** result_ptr = 0;
	{
		int status = rejection_sampling_get_variable_ptr(stmt->variable, trace, &result_ptr);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
			return status;
		}
	}

	#define EXECUTE_DRAW_STMT_GET_PARAM(n)	\
		pp_variable_t** param = 0;	\
		int status = rejection_sampling_get_parameters(stmt->expr_seq, trace, n, &param);	\
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {	\
			ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 	\
			return status;	\
		}

	#define EXECUTE_DRAW_STMT_CONVERT_PARAM(i, type, name, pp_type)	\
		type name = pp_variable_to_##pp_type (param[ i ]);

	#define EXECUTE_DRAW_STMT_CLEAR(n)	\
		for (size_t i = 0; i < n; ++i) {	\
			pp_variable_destroy(param[i]);	\
		}	\
		free(param);	

	switch (stmt->dist_type) {
	case ERP_FLIP:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(1)
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, p, float)
			EXECUTE_DRAW_STMT_CLEAR(1)

			int sample = flip(p);
			float logprob = flip_logprob(sample, p);
			if (*result_ptr) {
				pp_variable_destroy(*result_ptr);
			}
			*result_ptr = new_pp_int(sample);
			trace->logprob += logprob;
			return PP_SAMPLE_FUNCTION_NORMAL;
		}
	case ERP_MULTINOMIAL:
		{
			return PP_SAMPLE_FUNCTION_UNHANDLED;
		}
	case ERP_UNIFORM:
		{
			return PP_SAMPLE_FUNCTION_UNHANDLED;
		}
	case ERP_GAUSSIAN:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(2)
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, mu, float)
			EXECUTE_DRAW_STMT_CONVERT_PARAM(1, float, sigma, float)
			EXECUTE_DRAW_STMT_CLEAR(2)

			float sample = gaussian(mu, sigma);
			float logprob = gaussian_logprob(sample, mu, sigma);
			if (*result_ptr) {
				pp_variable_destroy(*result_ptr);
			}
			*result_ptr = new_pp_float(sample);
			trace->logprob += logprob;
			return PP_SAMPLE_FUNCTION_NORMAL;
		}
	case ERP_GAMMA:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(2)
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, a, float)
			EXECUTE_DRAW_STMT_CONVERT_PARAM(1, float, b, float)
			EXECUTE_DRAW_STMT_CLEAR(2)

			float sample = gamma1(a, b);
			float logprob = gamma_logprob(sample, a, b);
			if (*result_ptr) {
				pp_variable_destroy(*result_ptr);
			}
			*result_ptr = new_pp_float(sample);
			trace->logprob += logprob;
			return PP_SAMPLE_FUNCTION_NORMAL;
		}
		break;
	case ERP_BETA:
		{
			return PP_SAMPLE_FUNCTION_UNHANDLED;
		}
	case ERP_BINOMIAL:
		{
			return PP_SAMPLE_FUNCTION_UNHANDLED;
		}
	case ERP_POISSON:
		{
			return PP_SAMPLE_FUNCTION_UNHANDLED;
		}
	case ERP_DIRICHLET:
		{
			return PP_SAMPLE_FUNCTION_UNHANDLED;
		}
	}

	#undef EXECUTE_DRAW_STMT_GET_PARAM
	#undef EXECUTE_DRAW_STMT_CONVERT_PARAM
	#undef EXECUTE_DRAW_STMT_CLEAR

	return PP_SAMPLE_FUNCTION_UNHANDLED;
}

int rejection_sampling_execute_let_stmt(LetStmtNode* stmt, pp_trace_t* trace) {
	pp_variable_t** result_ptr = 0;
	{
		int status = rejection_sampling_get_variable_ptr(stmt->variable, trace, &result_ptr);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
			return status;
		}
	}

	pp_variable_t* result = 0;
	{
		int status = rejection_sampling_execute_expr(stmt->expr, trace, &result);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
			return status;
		}
	}

	if (*result_ptr) {
		pp_variable_destroy(*result_ptr);
	}
	*result_ptr = result;
	return PP_SAMPLE_FUNCTION_NORMAL;
}

int rejection_sampling_execute_for_stmt(ForStmtNode* stmt, pp_trace_t* trace) {
	pp_variable_t** loop_var_ptr = &(pp_trace_get_variable(trace, symbol_to_string(node_symbol_table(stmt), stmt->name)));

	pp_variable_t* loop_var = 0;
	int status = rejection_sampling_execute_expr(stmt->start_expr, trace, &loop_var);
	if (status != PP_SAMPLE_FUNCTION_NORMAL) {
		ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
		return status;
	}
	if (loop_var->type != INT) {
		return PP_SAMPLE_FUNCTION_NON_INTEGER_LOOP_VARIABLE;
	}

	pp_variable_t* end_var = 0;
	status = rejection_sampling_execute_expr(stmt->end_expr, trace, &end_var);
	if (status != PP_SAMPLE_FUNCTION_NORMAL) {
		ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
		pp_variable_destroy(loop_var);
		return status;
	}
	if (end_var->type != INT) {
		return PP_SAMPLE_FUNCTION_NON_INTEGER_LOOP_VARIABLE;
	}

	*loop_var_ptr = loop_var;
	for (; PP_VARIABLE_INT_VALUE(*loop_var_ptr) < PP_VARIABLE_INT_VALUE(end_var); ++PP_VARIABLE_INT_VALUE(*loop_var_ptr)) {
		StmtsNode* stmts = stmt->stmts;
		while (stmts) {
			status = rejection_sampling_execute_stmt(stmts->stmt, trace);
			if (status != PP_SAMPLE_FUNCTION_NORMAL) {
				ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
				pp_variable_destroy(end_var);
				return status;
			}
			stmts = stmts->stmts;
		}
	}

	pp_variable_destroy(end_var);
	return PP_SAMPLE_FUNCTION_NORMAL;	
}

int rejection_sampling_execute_stmt(StmtNode* stmt, pp_trace_t* trace) {
	if (!stmt) {
		return PP_SAMPLE_FUNCTION_INVALID_STATEMENT;
	}

	switch (stmt->type) {
	case DRAW_STMT:
		return rejection_sampling_execute_draw_stmt((DrawStmtNode*) stmt, trace);
	case LET_STMT:
		return rejection_sampling_execute_let_stmt((LetStmtNode*) stmt, trace);
	case FOR_STMT:
		return rejection_sampling_execute_for_stmt((ForStmtNode*) stmt, trace);
	}

	return PP_SAMPLE_FUNCTION_UNHANDLED;
}

int rejection_sampling(struct pp_state_t* state, const char* model_name, pp_variable_t* param[], pp_query_t* query, void** internal_data_ptr, pp_trace_t** trace_ptr) {
	/* nothing to do with internal_data_ptr */
	if (!state) {
		//ERR_OUTPUT("clean up internal data\n");
		return PP_SAMPLE_FUNCTION_NORMAL;
	}

	/* find model */
	ModelNode* model = model_map_find(state->model_map, state->symbol_table, model_name);
	if (!model) {
		return PP_SAMPLE_FUNCTION_MODEL_NOT_FOUND;	
	}

	pp_trace_t* trace = 0;

	while (1) {
		/* initialize trace */
		trace = new_pp_trace();

		/* set up parameters */
		ModelParamsNode* param_node = model->params;
		while (param_node) {
			const char* varname = symbol_to_string(node_symbol_table(param_node), param_node->name);
			pp_trace_set_variable(trace, varname, *(param++));	
			param_node = param_node->model_params;
		}

		/* ignore the declarations */

		/* execute statements */
		StmtsNode* stmts = model->stmts;
		while (stmts) {
			StmtNode* stmt = stmts->stmt;
			//ERR_OUTPUT("executing statement: %s", dump_stmt(stmt));

			int status = rejection_sampling_execute_stmt(stmt, trace);
			if (status != PP_SAMPLE_FUNCTION_NORMAL) {
				ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
				pp_trace_destroy(trace);
				return status;
			}
			stmts = stmts->stmts;
		}

		/* check condition */
		if (pp_query_acceptor(trace, query)) {
			break;
		}
		else {
			pp_trace_destroy(trace);
		}
	}

	*trace_ptr = trace;
	return PP_SAMPLE_FUNCTION_NORMAL;
}

