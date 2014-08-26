#include "infer.h"
#include "variables.h"
#include "trace.h"

int rejection_sampling_execute_add(pp_variable_t* left, pp_variable_t* right, pp_variable_t** result_ptr);
int rejection_sampling_execute_sub(pp_variable_t* left, pp_variable_t* right, pp_variable_t** result_ptr);
int rejection_sampling_execute_mul(pp_variable_t* left, pp_variable_t* right, pp_variable_t** result_ptr);
int rejection_sampling_execute_div(pp_variable_t* left, pp_variable_t* right, pp_variable_t** result_ptr);
int rejection_sampling_execute_if_expr(IfExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr);
int rejection_sampling_execute_new_expr(NewExprNode* expr, pp_trace_t* tarce, pp_variable_t** result_ptr);
int rejection_sampling_execute_binary_expr(BinaryExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr);
int rejection_sampling_execute_primary_expr(PrimaryExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr);
int rejection_sampling_execute_expr(ExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr);
int rejection_sampling_get_parameters(ExprSeqNode* expr_seq, pp_trace_t* trace, size_t num_expected, pp_variable_t*** result_ptr);
int rejection_sampling_get_variable_ptr(VariableNode* variable, pp_trace_t* trace, pp_variable_t*** result_ptr);
int rejection_sampling_execute_draw_stmt(DrawStmtNode* stmt, pp_trace_t* trace);
int rejection_sampling_execute_let_stmt(LetStmtNode* stmt, pp_trace_t* trace);
int rejection_sampling_execute_for_stmt(ForStmtNode* stmt, pp_trace_t* trace);
int rejection_sampling_execute_stmt(StmtNode* stmt, pp_trace_t* trace);

int rejection_sampling(struct pp_state_t* state, const char* model_name, pp_variable_t* param[], pp_query_t* query, void** internal_data_ptr, pp_trace_t** trace_ptr);
