#ifndef PARSE_H_
#define PARSE_H_

#include "../common/symbol_table.h"
#include "../common/list.h"
#include "../common/ilist.h"
#include "../infer/erp.h"
#include <stdlib.h>
#include <stddef.h>

/* FIXME should allow arbitrary length */
#define MAX_INTEGER_VALUE_SIZE 10

/* TYPES */

typedef struct ParserState
{
    symbol_table_t* symbol_table;
    int pointer;
    int program_length;
    const char* program;
} ParserState;

int peek1(ParserState* ps, int offset);
int peek(ParserState* ps);
void skip_spaces(ParserState* ps);
int accept(ParserState* ps, const char* token);

/* Base class of Node. */
typedef struct CommonNode
{
    symbol_table_t* symbol_table;
} CommonNode;

symbol_table_t* node_symbol_table(void* any);
void* new_node(size_t size, ParserState* ps);

typedef struct ModelsNode /* extends CommonNode */
{
    struct CommonNode super;
    struct ModelNode* model;
    struct ModelsNode* models;
} ModelsNode;

ModelsNode* parse_models(ParserState* ps);
const char* dump_models(ModelsNode* models);
int dump_models_impl(char* buffer, int buf_size, ModelsNode* models);

typedef struct ModelNode /* extends CommonNode */
{
    struct CommonNode super;
    symbol_t name;
    struct ModelParamsNode* params;
    struct DeclsNode* decls;
    struct StmtsNode* stmts;
} ModelNode;

ModelNode* parse_model(ParserState* ps);
const char* dump_model(ModelNode* model);
int dump_model_impl(char* buffer, int buf_size, ModelNode* model);

typedef struct ModelParamsNode /* extends CommonNode */
{
    struct CommonNode super;
    symbol_t name;
    struct ModelParamsNode* model_params;
} ModelParamsNode;

ModelParamsNode* parse_model_params(ParserState* ps);
const char* dump_model_params(ModelParamsNode* model_params);
int dump_model_params_impl(char* buffer, int buf_size, ModelParamsNode* model_params);

typedef struct DeclsNode /* extends CommonNode */
{
    struct CommonNode super;
    struct DeclNode* decl;
    struct DeclsNode* decls;
} DeclsNode;

DeclsNode* parse_decls(ParserState* ps);
const char* dump_decls(DeclsNode* decls);
int dump_decls_impl(char* buffer, int buf_size, DeclsNode* decls);

typedef struct DeclNode /* extends CommonNode */
{
    struct CommonNode super;
    enum { PUBLIC_DECL, PRIVATE_DECL } type;
} DeclNode;

DeclNode* parse_decl(ParserState* ps);
const char* dump_decl(DeclNode* decl);
int dump_decl_impl(char* buffer, int buf_size, DeclNode* decl);

typedef struct PublicDeclNode /* extends DeclNode */
{
    struct DeclNode super;
    struct VariableNode* variable;
} PublicDeclNode;

PublicDeclNode* parse_public_decl(ParserState* ps);
const char* dump_public_decl(PublicDeclNode* public_decl);
int dump_public_decl_impl(char* buffer, int buf_size, PublicDeclNode* public_decl);

typedef struct PrivateDeclNode /* extends DeclNode */
{
    struct DeclNode super;
    struct VariableNode* variable;
} PrivateDeclNode;

PrivateDeclNode* parse_private_decl(ParserState* ps);
const char* dump_private_decl(PrivateDeclNode* private_decl);
int dump_private_decl_impl(char* buffer, int buf_size, PrivateDeclNode* private_decl);

typedef struct StmtsNode /* extends CommonNode */
{
    struct CommonNode super;
    struct StmtNode* stmt;
    struct StmtsNode* stmts;
} StmtsNode;

StmtsNode* parse_stmts(ParserState* ps);
const char* dump_stmts(StmtsNode* stmts);
int dump_stmts_impl(char* buffer, int buf_size, StmtsNode* stmts);

typedef struct StmtNode /* extends CommonNode */
{
    struct CommonNode super;
    enum { DRAW_STMT, LET_STMT, FOR_STMT, WHILE_STMT } type;
} StmtNode;

StmtNode* parse_stmt(ParserState* ps);
const char* dump_stmt(StmtNode* stmt);
int dump_stmt_impl(char* buffer, int buf_size, StmtNode* stmt);

typedef struct DrawStmtNode /* extends StmtNode */
{
    struct StmtNode super;
    struct VariableNode* variable;
    /* symbol_t dist; */
    erp_enum_t dist_type;
    struct ExprSeqNode* expr_seq;
} DrawStmtNode;

const char* dump_draw_stmt(DrawStmtNode* draw_stmt);
int dump_draw_stmt_impl(char* buffer, int buf_size, DrawStmtNode* draw_stmt);

typedef struct LetStmtNode /* extends StmtNode */
{
    struct StmtNode super;
    struct VariableNode* variable;
    struct ExprNode* expr;
} LetStmtNode;

const char* dump_let_stmt(LetStmtNode* let_stmt);
int dump_let_stmt_impl(char* buffer, int buf_size, LetStmtNode* let_stmt);

typedef struct ForStmtNode /* extends StmtNode */
{
    struct StmtNode super;
    symbol_t name;
    struct ExprNode* start_expr;
    struct ExprNode* end_expr;
    struct StmtsNode* stmts;
} ForStmtNode;

ForStmtNode* parse_for_stmt(ParserState* ps);
const char* dump_for_stmt(ForStmtNode* for_stmt);
int dump_for_stmt_impl(char* buffer, int buf_size, ForStmtNode* for_stmt);

typedef struct WhileStmtNode { /* extends StmtNode */
    struct StmtNode super;
    struct ExprNode* condition;
    struct StmtsNode* stmts;
} WhileStmtNode;

WhileStmtNode* parse_while_stmt(ParserState* ps);
const char* dump_while_stmt(WhileStmtNode* while_stmt);
int dump_while_stmt_impl(char* buffer, int buf_size, WhileStmtNode* while_stmt);

typedef struct ExprSeqNode /* extends CommonNode */
{
    struct CommonNode super;
    struct ExprNode* expr;
    struct ExprSeqNode* expr_seq;
} ExprSeqNode;

ExprSeqNode* parse_expr_seq(ParserState* ps);
const char* dump_expr_seq(ExprSeqNode* expr_seq);
int dump_expr_seq_impl(char* buffer, int buf_size, ExprSeqNode* expr_seq);
size_t expr_seq_length(ExprSeqNode* expr_seq);

typedef struct ExprNode /* extends CommonNode */
{
    struct CommonNode super;
    enum { IF_EXPR, NEW_EXPR, BINARY_EXPR, PRIMARY_EXPR } type;
} ExprNode;

ExprNode* parse_expr(ParserState* ps);
ExprNode* parse_add_expr(ParserState* ps);
ExprNode* parse_term(ParserState* ps);
const char* dump_expr(ExprNode* expr);
int dump_expr_impl(char* buffer, int buf_size, ExprNode* expr);

typedef struct IfExprNode /* extends ExprNode */
{
    struct ExprNode super;
    struct ExprNode* condition;
    struct ExprNode* consequent;
    struct ExprNode* alternative;
} IfExprNode;

const char* dump_if_expr(IfExprNode* if_expr);
int dump_if_expr_impl(char* buffer, int buf_size, IfExprNode* if_expr);

typedef struct NewExprNode /* extends ExprNode */
{
    struct ExprNode super;
    symbol_t name;
} NewExprNode;

const char* dump_new_expr(NewExprNode* new_expr);
int dump_new_expr_impl(char* buffer, int buf_size, NewExprNode* new_expr);

typedef struct BinaryExprNode /* extends ExprNode */
{
    struct ExprNode super;
    enum { OP_ADD, OP_SUB, OP_MUL, OP_DIV } op;
    struct ExprNode* left;
    struct ExprNode* right;
} BinaryExprNode;

const char* dump_binary_expr(BinaryExprNode* binary_expr);
int dump_binary_expr_impl(char* buffer, int buf_size, BinaryExprNode* binary_expr);

typedef struct PrimaryExprNode /* extends ExprNode */
{
    struct ExprNode super;
    enum {
        NUM_EXPR,
        VAR_EXPR,
        UNARY_EXPR,
        GROUP_EXPR,
        FUNC_EXPR
    } type;
} PrimaryExprNode;

PrimaryExprNode* parse_primary(ParserState* ps);
const char* dump_primary(PrimaryExprNode* primary);
int dump_primary_impl(char* buffer, int buf_size, PrimaryExprNode* primary);

typedef struct NumExprNode /* extends PrimaryExprNode */
{
    struct PrimaryExprNode super;
    struct NumericalValueNode* numerical_value;
} NumExprNode;

typedef struct VarExprNode /* extends PrimaryExprNode */
{
    struct PrimaryExprNode super;
    struct VariableNode* variable;
} VarExprNode;

typedef struct UnaryExprNode /* extends PrimaryExprNode */
{
    struct PrimaryExprNode super;
    enum { OP_NEG } op;
    struct PrimaryExprNode* primary;
} UnaryExprNode;

const char* dump_unary_expr(UnaryExprNode* unary);
int dump_unary_expr_impl(char* buffer, int buf_size, UnaryExprNode* unary);

typedef struct GroupExprNode /* extends PrimaryExprNode */
{
    struct PrimaryExprNode super;
    struct ExprNode* expr;
} GroupExprNode;

typedef struct FuncExprNode /* extends PrimrayNode */
{
    struct PrimaryExprNode super;
    symbol_t name;
    struct ExprSeqNode* expr_seq;
} FuncExprNode;

typedef struct VariableNode /* extends CommonNode */
{
    struct CommonNode super;
    enum { NAME_VAR, FIELD_VAR, INDEX_VAR } type;
} VariableNode;

VariableNode* parse_variable(ParserState* ps);
VariableNode* parse_variable1(ParserState* ps, symbol_t name);
const char* dump_variable(VariableNode* variable);
int dump_variable_impl(char* buffer, int buf_size, VariableNode* variable);

typedef struct NameVarNode /* extends VariableNode */
{
    struct VariableNode super;
    symbol_t name;
} NameVarNode;

const char* dump_name_var(struct NameVarNode* name_var);
int dump_name_var_impl(char* buffer, int buf_size, struct NameVarNode* name_var);

typedef struct FieldVarNode /* extends VariableNode */
{
    struct VariableNode super;
    symbol_t name;
    symbol_t field_name;
} FieldVarNode;

const char* dump_field_var(FieldVarNode* field_var);
int dump_field_var_impl(char* buffer, int buf_size, FieldVarNode* field_var);

typedef struct IndexVarNode /* extends VariableNode */
{
    struct VariableNode super;
    symbol_t name;
    struct ExprSeqNode* expr_seq;
} IndexVarNode;

const char* dump_index_var(IndexVarNode* index_var);
int dump_index_var_impl(char* buffer, int buf_size, IndexVarNode* index_var);

typedef struct NumericalValueNode /* extends CommonNode */
{
    struct CommonNode super;
    enum { REAL_VALUE, INTEGER_VALUE } type;
} NumericalValueNode;

NumericalValueNode* parse_numerical_value(ParserState* ps);
const char* dump_numerical_value(NumericalValueNode* numerical_value);
int dump_numerical_value_impl(char* buffer, int buf_size, NumericalValueNode* numerical_value);

typedef struct RealValueNode /* extends NumericalValueNode */
{
    struct NumericalValueNode super;
    /*struct IntegerValueNode* integer_part;
    struct IntegerValueNode* fractional;*/
    float value;
} RealValueNode;

const char* dump_real_value(RealValueNode* real_value);
int dump_real_value_impl(char* buffer, int buf_size, RealValueNode* real_value);

typedef struct IntegerValueNode /* extends NumericalValueNode */
{
    struct NumericalValueNode super;
    /*int length;
    char digits[MAX_INTEGER_VALUE_SIZE];*/
    int value;
} IntegerValueNode;

IntegerValueNode* parse_integer_value(ParserState* ps);
const char* dump_integer_value(IntegerValueNode* integer_value);
int dump_integer_value_impl(char* buffer, int buf_size, IntegerValueNode* integer_value);

/* FUNCTIONS */

ModelsNode* parse_file(const char* file_name);
ModelsNode* parse_file1(const char* filename, symbol_table_t* symbol_table);

symbol_t parse_name(ParserState* ps);
const char* dump_name(symbol_t name, symbol_table_t* symbol_table);
int dump_name_impl(char* buffer, int buf_size, symbol_t name, symbol_table_t* symbol_table);

#endif
