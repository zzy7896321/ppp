#include "execute.h"
#include "../debug.h"

int execute_add(pp_variable_t* left, pp_variable_t* right, pp_variable_t** result_ptr) {
	switch (left->type) {
	case PP_VARIABLE_INT:
		switch (right->type) {
		case PP_VARIABLE_INT:
			{
				*result_ptr = new_pp_int(PP_VARIABLE_INT_VALUE(left) + PP_VARIABLE_INT_VALUE(right));
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}			
			break;
		case PP_VARIABLE_FLOAT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_INT_VALUE(left) + PP_VARIABLE_FLOAT_VALUE(right));
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		case PP_VARIABLE_VECTOR:
			{
				pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_OPERAND_TYPE, ": INT and VECTOR");
			}
			break;
		}
		break;
	case PP_VARIABLE_FLOAT:
		switch (right->type) {
		case PP_VARIABLE_INT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_FLOAT_VALUE(left) + PP_VARIABLE_INT_VALUE(right));
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}			
			break;
		case PP_VARIABLE_FLOAT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_FLOAT_VALUE(left) + PP_VARIABLE_FLOAT_VALUE(right));
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		case PP_VARIABLE_VECTOR:
			{
				pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_OPERAND_TYPE, ": FLOAT and VECTOR");
			}
			break;
		}
		break;
	case PP_VARIABLE_VECTOR:
		switch (right->type) {
		case PP_VARIABLE_INT:
			{
				pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_OPERAND_TYPE, ": VECTOR and INT");
			}			
			break;
		case PP_VARIABLE_FLOAT:
			{
				pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_OPERAND_TYPE, ": VECTOR and FLOAT");
			}
			break;
		case PP_VARIABLE_VECTOR:
			{	
				size_t l_length = PP_VARIABLE_VECTOR_LENGTH(left);
				size_t r_length = PP_VARIABLE_VECTOR_LENGTH(right);

				if (l_length != r_length) {
					pp_sample_error_return(PP_SAMPLE_FUNCTION_VECTOR_LENGTH_MISMATCH, ": %u and %u", l_length, r_length);
				}

				pp_variable_t* result = new_pp_vector(PP_VARIABLE_VECTOR_CAPACITY(left));
				for (l_length = 0; l_length != r_length; ++l_length) {
					int status = 
						execute_add(	PP_VARIABLE_VECTOR_VALUE(left)[l_length],
														PP_VARIABLE_VECTOR_VALUE(right)[l_length],
														&(PP_VARIABLE_VECTOR_VALUE(result)[l_length]));
					if (status != PP_SAMPLE_FUNCTION_NORMAL) {
						ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
						pp_variable_destroy(result);
						pp_sample_error_return(status, "");
					}
					else {
						++PP_VARIABLE_VECTOR_LENGTH(result);
					}
				}

				*result_ptr = result;
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		}
		break;
	}

	pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
}

int execute_sub(pp_variable_t* left, pp_variable_t* right, pp_variable_t** result_ptr) {
	switch (left->type) {
	case PP_VARIABLE_INT:
		switch (right->type) {
		case PP_VARIABLE_INT:
			{
				*result_ptr = new_pp_int(PP_VARIABLE_INT_VALUE(left) - PP_VARIABLE_INT_VALUE(right));
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}			
			break;
		case PP_VARIABLE_FLOAT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_INT_VALUE(left) - PP_VARIABLE_FLOAT_VALUE(right));
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		case PP_VARIABLE_VECTOR:
			{
				pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_OPERAND_TYPE, ": INT and VECTOR");
			}
			break;
		}
		break;
	case PP_VARIABLE_FLOAT:
		switch (right->type) {
		case PP_VARIABLE_INT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_FLOAT_VALUE(left) - PP_VARIABLE_INT_VALUE(right));
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}			
			break;
		case PP_VARIABLE_FLOAT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_FLOAT_VALUE(left) - PP_VARIABLE_FLOAT_VALUE(right));
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		case PP_VARIABLE_VECTOR:
			{
				pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_OPERAND_TYPE, ": FLOAT and VECTOR");
			}
			break;
		}
		break;
	case PP_VARIABLE_VECTOR:
		switch (right->type) {
		case PP_VARIABLE_INT:
			{
				pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_OPERAND_TYPE, ": VECTOR and INT");
			}			
			break;
		case PP_VARIABLE_FLOAT:
			{
				pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_OPERAND_TYPE, ": VECTOR and FLOAT");
			}
			break;
		case PP_VARIABLE_VECTOR:
			{	
				size_t l_length = PP_VARIABLE_VECTOR_LENGTH(left);
				size_t r_length = PP_VARIABLE_VECTOR_LENGTH(right);

				if (l_length != r_length) {
					pp_sample_error_return(PP_SAMPLE_FUNCTION_VECTOR_LENGTH_MISMATCH, "");
				}

				pp_variable_t* result = new_pp_vector(PP_VARIABLE_VECTOR_CAPACITY(left));
				for (l_length = 0; l_length != r_length; ++l_length) {
					int status = 
						execute_sub(	PP_VARIABLE_VECTOR_VALUE(left)[l_length],
														PP_VARIABLE_VECTOR_VALUE(right)[l_length],
														&(PP_VARIABLE_VECTOR_VALUE(result)[l_length]));
					if (status != PP_SAMPLE_FUNCTION_NORMAL) {
						ERR_OUTPUT("%s\n", pp_sample_get_error_string(status)); 
						pp_variable_destroy(result);
						pp_sample_error_return(status, "");
					}
					else {
						++PP_VARIABLE_VECTOR_LENGTH(result);
					}
				}

				*result_ptr = result;
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		}
		break;
	}

	pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
}
int execute_mul(pp_variable_t* left, pp_variable_t* right, pp_variable_t** result_ptr) {
	switch (left->type) {
	case PP_VARIABLE_INT:
		switch (right->type) {
		case PP_VARIABLE_INT:
			{
				*result_ptr = new_pp_int(PP_VARIABLE_INT_VALUE(left) * PP_VARIABLE_INT_VALUE(right));
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}			
			break;
		case PP_VARIABLE_FLOAT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_INT_VALUE(left) * PP_VARIABLE_FLOAT_VALUE(right));
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		case PP_VARIABLE_VECTOR:
			{
				pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_OPERAND_TYPE, ": INT and VECTOR");
			}
			break;
		}
		break;
	case PP_VARIABLE_FLOAT:
		switch (right->type) {
		case PP_VARIABLE_INT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_FLOAT_VALUE(left) * PP_VARIABLE_INT_VALUE(right));
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}			
			break;
		case PP_VARIABLE_FLOAT:
			{
				*result_ptr = new_pp_float(PP_VARIABLE_FLOAT_VALUE(left) * PP_VARIABLE_FLOAT_VALUE(right));
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}
		case PP_VARIABLE_VECTOR:
			{
				pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_OPERAND_TYPE, ": FLOAT and VECTOR");
			}
			break;
		}
		break;
	case PP_VARIABLE_VECTOR:
		pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, ": * is not defined for vectors");
		break;
	}

	pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
}

int execute_div(pp_variable_t* left, pp_variable_t* right, pp_variable_t** result_ptr) {
	switch (left->type) {
	case PP_VARIABLE_INT:
		switch (right->type) {
		case PP_VARIABLE_INT:
			{
				if (!PP_VARIABLE_INT_VALUE(right)) {
					pp_sample_error_return(PP_SAMPLE_FUNCTION_DIVISION_BY_ZERO, "");
				}
				*result_ptr = new_pp_int(PP_VARIABLE_INT_VALUE(left) / PP_VARIABLE_INT_VALUE(right));
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}			
			break;
		case PP_VARIABLE_FLOAT:
			{
				if (!PP_VARIABLE_INT_VALUE(right)) {
					pp_sample_error_return(PP_SAMPLE_FUNCTION_DIVISION_BY_ZERO, "");
				}
				*result_ptr = new_pp_float(PP_VARIABLE_INT_VALUE(left) / PP_VARIABLE_FLOAT_VALUE(right));
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		case PP_VARIABLE_VECTOR:
			{
				pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_OPERAND_TYPE, ": INT and VECTOR");
			}
			break;
		}
		break;
	case PP_VARIABLE_FLOAT:
		switch (right->type) {
		case PP_VARIABLE_INT:
			{
				if (!PP_VARIABLE_INT_VALUE(right)) {
					pp_sample_error_return(PP_SAMPLE_FUNCTION_DIVISION_BY_ZERO, "");
				}
				*result_ptr = new_pp_float(PP_VARIABLE_FLOAT_VALUE(left) / PP_VARIABLE_INT_VALUE(right));
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}			
			break;
		case PP_VARIABLE_FLOAT:
			{
				if (!PP_VARIABLE_INT_VALUE(right)) {
					pp_sample_error_return(PP_SAMPLE_FUNCTION_DIVISION_BY_ZERO, "");
				}
				*result_ptr = new_pp_float(PP_VARIABLE_FLOAT_VALUE(left) / PP_VARIABLE_FLOAT_VALUE(right));
				pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
			}
			break;
		case PP_VARIABLE_VECTOR:
			{
				pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_OPERAND_TYPE, ": FLOAT and VECTOR");
			}
			break;
		}
		break;
	case PP_VARIABLE_VECTOR:
		pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, ": / is not defined for vectors");
		break;
	}

	pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
}


int execute_if_expr(IfExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr) {
	pp_variable_t* condition = 0;
	{
		int status = execute_expr((ExprNode*) expr->condition, trace, &condition);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			pp_sample_error_return(status, "");
		}
	}

	int condition_true = 0;
	switch (condition->type) {
	case PP_VARIABLE_INT:
		condition_true = !!PP_VARIABLE_INT_VALUE(condition);
		break;
	case PP_VARIABLE_FLOAT:
		condition_true = !!PP_VARIABLE_FLOAT_VALUE(condition);
		break;
	case PP_VARIABLE_VECTOR:
		pp_sample_error_return(PP_SAMPLE_FUNCTION_NON_SCALAR_TYPE_AS_CONDITION, "");
	}
	pp_variable_destroy(condition);

	ExprNode* taken_branch = (condition_true) ? expr->consequent : expr->alternative;
	pp_sample_return(execute_expr(taken_branch, trace, result_ptr));
}

int execute_new_expr(NewExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr) {
	pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
}

int execute_binary_expr(BinaryExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr) {
	pp_variable_t* left = 0;
	pp_variable_t* right = 0;

	{
		int status = execute_expr((ExprNode*) expr->left, trace, &left);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			pp_sample_error_return(status, "");
		}
	}
	{
		int status = execute_expr((ExprNode*) expr->right, trace, &right);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			pp_variable_destroy(left);
			pp_sample_error_return(status, "");
		}
	}

	#define SAFE_RETURN(op)	\
		do {	\
			int status = execute_##op(left, right, result_ptr);	\
			pp_variable_destroy(left);	\
			pp_variable_destroy(right);	\
			pp_sample_return(status);\
		} while(0)

	switch (expr->op) {
	case OP_ADD:
		SAFE_RETURN(add);
	case OP_SUB:
		SAFE_RETURN(sub);
	case OP_MUL:
		SAFE_RETURN(mul);
	case OP_DIV:
		SAFE_RETURN(div);
	}	

	#undef SAFE_RETURN

	pp_variable_destroy(left);
	pp_variable_destroy(right);
	pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
}

int execute_primary_expr(PrimaryExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr) {
	switch (expr->type) {
	case NUM_EXPR:
		{
			NumericalValueNode* numerical_value = ((NumExprNode*) expr)->numerical_value;
			switch (numerical_value->type) {
			case REAL_VALUE:
				{
					*result_ptr = new_pp_float(((RealValueNode*) numerical_value)->value);
					pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
				}
			case INTEGER_VALUE:
				{
					*result_ptr = new_pp_int(((IntegerValueNode*) numerical_value)->value);
					pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
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
						pp_sample_error_return(PP_SAMPLE_FUNCTION_VARIABLE_NOT_FOUND, ": %s", name);
					}

					*result_ptr = pp_variable_clone(var); 
					pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
				}
			case FIELD_VAR:
				{
					pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
				}
			case INDEX_VAR:
				{
					IndexVarNode* index_var = (IndexVarNode*) variable;
					const char* name = symbol_to_string(node_symbol_table(index_var), index_var->name);
					pp_variable_t* var = pp_trace_find_variable(trace, name);
					if (!var) {
						pp_sample_error_return(PP_SAMPLE_FUNCTION_VARIABLE_NOT_FOUND, ": %s", name);
					}

					ExprSeqNode* expr_seq = index_var->expr_seq;
					while (expr_seq) {
						if (!var || var->type != PP_VARIABLE_VECTOR) {
							pp_sample_error_return(PP_SAMPLE_FUNCTION_SUBSCRIPTING_TO_NON_VECTOR, "%s", name);
						}
						ExprNode* expr = expr_seq->expr;
						pp_variable_t* sub = 0;
						int status = execute_expr(expr, trace, &sub);
						if (status != PP_SAMPLE_FUNCTION_NORMAL) {
							pp_sample_error_return(status, "");
						}

						if (sub->type != PP_VARIABLE_INT) {
							pp_variable_destroy(sub);
							pp_sample_error_return(PP_SAMPLE_FUNCTION_NON_INTEGER_SUBSCRIPTION, "");
						}
						int index = PP_VARIABLE_INT_VALUE(sub);
						pp_variable_destroy(sub);
						if (index < 0 || index >= PP_VARIABLE_VECTOR_LENGTH(var)) {
							pp_sample_error_return(PP_SAMPLE_FUNCTION_INDEX_OUT_OF_BOUND,
									": %s, index = %d, length = %d", name, index, PP_VARIABLE_VECTOR_LENGTH(var));
						}

						var = PP_VARIABLE_VECTOR_VALUE(var)[index];
						expr_seq = expr_seq->expr_seq;
					}

					*result_ptr = pp_variable_clone(var);
					pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
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
						int status = execute_primary_expr(unary->primary, trace, &var);
						if (status != PP_SAMPLE_FUNCTION_NORMAL) {
							pp_sample_error_return(status, "");
						}
					}
					switch (var->type) {
					case PP_VARIABLE_INT:
						PP_VARIABLE_INT_VALUE(var) = -PP_VARIABLE_INT_VALUE(var);
						*result_ptr = var;
						pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
					case PP_VARIABLE_FLOAT:
						PP_VARIABLE_FLOAT_VALUE(var) = -PP_VARIABLE_FLOAT_VALUE(var);
						*result_ptr = var;
						pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
					case PP_VARIABLE_VECTOR:
						pp_variable_destroy(var);
						pp_sample_error_return(	PP_SAMPLE_FUNCTION_INVALID_OPERAND_TYPE, "");
					}
				}
			}
		}
		break;
	case GROUP_EXPR:
		{
			pp_sample_return(execute_expr(((GroupExprNode*) expr)->expr, trace, result_ptr));
		}
		//break;
	case FUNC_EXPR:
		{
			pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
		}
		//break;
	}
	
	pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
}

int execute_expr(ExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr) {
	if (!expr) {
		pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_EXPRESSION, "");
	}	

	switch (expr->type) {
	case IF_EXPR:
		pp_sample_return(execute_if_expr((IfExprNode*) expr, trace, result_ptr));
	case NEW_EXPR:
		pp_sample_return(execute_new_expr((NewExprNode*) expr, trace, result_ptr));
	case BINARY_EXPR:
		pp_sample_return(execute_binary_expr((BinaryExprNode*) expr, trace, result_ptr));
	case PRIMARY_EXPR:
		pp_sample_return(execute_primary_expr((PrimaryExprNode*) expr, trace, result_ptr));
	}

	pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
}

int get_parameters(ExprSeqNode* expr_seq, pp_trace_t* trace, size_t num_expected, pp_variable_t*** result_ptr) {
	#define REJECTION_SAMPLING_GET_PARAM_CLEAR_RETURN(status)	\
			for (size_t j = 0; j < i; ++j) {	\
				pp_variable_destroy(result[i]);	\
			}	\
			free(result);	\
			pp_sample_error_return(status, "")


	pp_variable_t** result = malloc(sizeof(pp_variable_t*) * num_expected);
	size_t i;

	for (i = 0; i != num_expected && expr_seq; ++i, expr_seq = expr_seq->expr_seq) {
		int status = execute_expr(expr_seq->expr, trace, &(result[i]));
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			REJECTION_SAMPLING_GET_PARAM_CLEAR_RETURN(status);
		}
	}

	if (i != num_expected || expr_seq) {
		REJECTION_SAMPLING_GET_PARAM_CLEAR_RETURN(PP_SAMPLE_FUNCTION_NUMBER_OF_PARAMETER_MISMATCH);
	}

	#undef REJECTION_SAMPLING_GET_PARAM_CLEAR_RETURN

	*result_ptr = result;
	pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
}

int get_variable_ptr(VariableNode* variable, pp_trace_t* trace, pp_variable_t*** result_ptr) {

	switch (variable->type) {
	case NAME_VAR:
		*result_ptr = &(pp_trace_get_variable(trace, symbol_to_string(node_symbol_table(variable), ((NameVarNode*) variable)->name)));
		pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
	case FIELD_VAR:
		pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
		//break;
	case INDEX_VAR:
		{
			const char* name = symbol_to_string(node_symbol_table(variable), ((IndexVarNode*) variable)->name);
			pp_variable_t** vec_ptr = &pp_trace_get_variable(trace, name);
			pp_variable_t* vec = *vec_ptr;

			ExprSeqNode* expr_seq = ((IndexVarNode*) variable)->expr_seq;
			if (!expr_seq) {
				pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_EXPRESSION, "");
			}
			while (expr_seq) {
				if (!vec) {
					vec = new_pp_vector(16);
					*vec_ptr = vec;
				}
				else if (vec->type != PP_VARIABLE_VECTOR) {
					pp_sample_error_return(PP_SAMPLE_FUNCTION_SUBSCRIPTING_TO_NON_VECTOR, ": %s", name);
				}

				ExprNode* expr = expr_seq->expr;
				pp_variable_t* sub = 0;
				int status = execute_expr(expr, trace, &sub);
				if (status != PP_SAMPLE_FUNCTION_NORMAL) {
					pp_sample_error_return(status, "");
				}
				if (sub->type != PP_VARIABLE_INT) {
					pp_sample_error_return(PP_SAMPLE_FUNCTION_NON_INTEGER_SUBSCRIPTION, "");
				}
				int index = PP_VARIABLE_INT_VALUE(sub);
				pp_variable_destroy(sub);
				if (index < 0) {
					pp_sample_error_return(PP_SAMPLE_FUNCTION_INDEX_OUT_OF_BOUND,
							": %s, index %d, length %d", name, index, PP_VARIABLE_VECTOR_LENGTH(vec));
				}
				if (index >= PP_VARIABLE_VECTOR_LENGTH(vec)) {
					pp_variable_vector_resize((pp_vector_t*) vec, index + 1);
				}

				vec_ptr = &(PP_VARIABLE_VECTOR_VALUE(vec)[index]);
				vec = *vec_ptr;

				expr_seq = expr_seq->expr_seq;
			}

			*result_ptr = vec_ptr;
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
	}

	pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
}

int execute_draw_stmt(DrawStmtNode* stmt, pp_trace_t* trace) {

	pp_variable_t** result_ptr = 0;
	{
		int status = get_variable_ptr(stmt->variable, trace, &result_ptr);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			pp_sample_error_return(status, "");
		}
	}

	#define EXECUTE_DRAW_STMT_GET_PARAM(n)	\
		pp_variable_t** param = 0;	\
	do {	\
		int status = get_parameters(stmt->expr_seq, trace, n, &param);	\
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {	\
			pp_sample_error_return(status, "");	\
		}	\
	} while(0) 

	#define EXECUTE_DRAW_STMT_CONVERT_PARAM(i, type, name, pp_type)	\
		type name = pp_variable_to_##pp_type (param[ i ])

	#define EXECUTE_DRAW_STMT_CLEAR(n)	\
	do {	\
		for (size_t i = 0; i < n; ++i) {	\
			pp_variable_destroy(param[i]);	\
		}	\
		free(param);	\
	} while(0)

	switch (stmt->dist_type) {
	case ERP_FLIP:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(1);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, p, float);
			EXECUTE_DRAW_STMT_CLEAR(1);

			int sample = flip(p);
			float logprob = flip_logprob(sample, p);
			if (*result_ptr) {
				pp_variable_destroy(*result_ptr);
			}
			*result_ptr = new_pp_int(sample);
			trace->logprob += logprob;
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
	case ERP_MULTINOMIAL:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(1);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float*, theta, float_vector);
			int n = PP_VARIABLE_VECTOR_LENGTH(param[0]);
			EXECUTE_DRAW_STMT_CLEAR(1);

			int sample = multinomial(theta, n);
			float logprob = multinomial_logprob(sample, theta, n);
			if (*result_ptr) {
				pp_variable_destroy(*result_ptr);
			}
			*result_ptr = new_pp_int(sample);
			trace->logprob += logprob;
			free(theta);
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
	case ERP_UNIFORM:
		{
			pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
		}
	case ERP_GAUSSIAN:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(2);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, mu, float);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(1, float, sigma, float);
			EXECUTE_DRAW_STMT_CLEAR(2);

			float sample = gaussian(mu, sigma);
			float logprob = gaussian_logprob(sample, mu, sigma);
			if (*result_ptr) {
				pp_variable_destroy(*result_ptr);
			}
			*result_ptr = new_pp_float(sample);
			trace->logprob += logprob;
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
	case ERP_GAMMA:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(2);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, a, float);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(1, float, b, float);
			EXECUTE_DRAW_STMT_CLEAR(2);

			float sample = gamma1(a, b);
			float logprob = gamma_logprob(sample, a, b);
			if (*result_ptr) {
				pp_variable_destroy(*result_ptr);
			}
			*result_ptr = new_pp_float(sample);
			trace->logprob += logprob;
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
		break;
	case ERP_BETA:
		{
			pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
		}
	case ERP_BINOMIAL:
		{
			pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
		}
	case ERP_POISSON:
		{
			pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
		}
	case ERP_DIRICHLET:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(2);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, alpha, float);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(1, int, n, int);
			EXECUTE_DRAW_STMT_CLEAR(2);

			float* alphas = malloc(sizeof(float) * n);
			for (int i = 0; i != n; ++i) {
				alphas[i] = alpha;
			}
			float* sample = dirichlet(alphas, n);
			float logprob = dirichlet_logprob(sample, alphas, n);
			if (*result_ptr) {
				pp_variable_destroy(*result_ptr);
			}
			*result_ptr = pp_variable_float_array_to_vector(sample, n);
			trace->logprob += logprob;
			free(sample);
			free(alphas);
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
	}

	#undef EXECUTE_DRAW_STMT_GET_PARAM
	#undef EXECUTE_DRAW_STMT_CONVERT_PARAM
	#undef EXECUTE_DRAW_STMT_CLEAR

	pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
}

int execute_let_stmt(LetStmtNode* stmt, pp_trace_t* trace) {
	pp_variable_t** result_ptr = 0;
	{
		int status = get_variable_ptr(stmt->variable, trace, &result_ptr);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			pp_sample_error_return(status, "");
		}
	}

	pp_variable_t* result = 0;
	{
		int status = execute_expr(stmt->expr, trace, &result);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			pp_sample_error_return(status, "");
		}
	}

	if (*result_ptr) {
		pp_variable_destroy(*result_ptr);
	}
	*result_ptr = result;
	pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
}

int execute_for_stmt(ForStmtNode* stmt, pp_trace_t* trace) {
	pp_variable_t** loop_var_ptr = &(pp_trace_get_variable(trace, symbol_to_string(node_symbol_table(stmt), stmt->name)));

	pp_variable_t* loop_var = 0;
	int status = execute_expr(stmt->start_expr, trace, &loop_var);
	if (status != PP_SAMPLE_FUNCTION_NORMAL) {
		pp_sample_error_return(status, "");
	}
	if (loop_var->type != PP_VARIABLE_INT) {
		pp_sample_error_return(PP_SAMPLE_FUNCTION_NON_INTEGER_LOOP_VARIABLE, "");
	}

	pp_variable_t* end_var = 0;
	status = execute_expr(stmt->end_expr, trace, &end_var);
	if (status != PP_SAMPLE_FUNCTION_NORMAL) {
		pp_variable_destroy(loop_var);
		pp_sample_error_return(status, "");
	}
	if (end_var->type != PP_VARIABLE_INT) {
		pp_sample_error_return(PP_SAMPLE_FUNCTION_NON_INTEGER_LOOP_VARIABLE, "");
	}

	/* FIXME end_var could be modified during the loop, 
	   which means it should actually be reevaluated at the end of each loop */
	*loop_var_ptr = loop_var;
	for (; PP_VARIABLE_INT_VALUE(*loop_var_ptr) <= PP_VARIABLE_INT_VALUE(end_var); ++PP_VARIABLE_INT_VALUE(*loop_var_ptr)) {
		StmtsNode* stmts = stmt->stmts;
		while (stmts) {
			status = execute_stmt(stmts->stmt, trace);
			if (status != PP_SAMPLE_FUNCTION_NORMAL) {
				pp_variable_destroy(end_var);
				pp_sample_error_return(status, "");
			}
			stmts = stmts->stmts;
		}
	}

	pp_variable_destroy(end_var);
	pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);	
}

int execute_stmt(StmtNode* stmt, pp_trace_t* trace) {
	if (!stmt) {
		pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_STATEMENT, "");
	}

//	ERR_OUTPUT("executing stmt:\n%s", dump_stmt(stmt));
	switch (stmt->type) {
	case DRAW_STMT:
		pp_sample_return(execute_draw_stmt((DrawStmtNode*) stmt, trace));
	case LET_STMT:
		pp_sample_return(execute_let_stmt((LetStmtNode*) stmt, trace));
	case FOR_STMT:
		pp_sample_return(execute_for_stmt((ForStmtNode*) stmt, trace));
	}

	pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
}
