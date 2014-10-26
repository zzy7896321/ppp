#ifndef EXECUTE_H
#define EXECUTE_H

#include "infer.h"
#include "../common/variables.h"
#include "../common/trace.h"

int execute_add(pp_variable_t* left, pp_variable_t* right, pp_variable_t** result_ptr);
int execute_sub(pp_variable_t* left, pp_variable_t* right, pp_variable_t** result_ptr);
int execute_mul(pp_variable_t* left, pp_variable_t* right, pp_variable_t** result_ptr);
int execute_div(pp_variable_t* left, pp_variable_t* right, pp_variable_t** result_ptr);
int execute_if_expr(IfExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr);
int execute_new_expr(NewExprNode* expr, pp_trace_t* tarce, pp_variable_t** result_ptr);
int execute_binary_expr(BinaryExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr);
int execute_primary_expr(PrimaryExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr);
int execute_expr(ExprNode* expr, pp_trace_t* trace, pp_variable_t** result_ptr);
int get_parameters(ExprSeqNode* expr_seq, pp_trace_t* trace, size_t num_expected, pp_variable_t*** result_ptr);
int get_variable_ptr(VariableNode* variable, pp_trace_t* trace, pp_variable_t*** result_ptr);
int execute_draw_stmt(DrawStmtNode* stmt, pp_trace_t* trace);
int execute_let_stmt(LetStmtNode* stmt, pp_trace_t* trace);
int execute_for_stmt(ForStmtNode* stmt, pp_trace_t* trace);
int execute_stmt(StmtNode* stmt, pp_trace_t* trace);

#endif
