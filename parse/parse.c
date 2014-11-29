#include "../ppp.h"
#include "../defs.h"
#include "../debug.h"
#include "../config.h"
#include "parse.h"
#include "interface.h"
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define MAX_NAME_SIZE 256

#define DUMP_BUFFER_SIZE 8096
static char dump_buffer[DUMP_BUFFER_SIZE];

symbol_table_t* node_symbol_table(void* any)
{
    return ((struct CommonNode*)any)->symbol_table;
}

void* new_node(size_t size, ParserState* ps)
{
    struct CommonNode* node;
    node = malloc(size);
    node->symbol_table = ps->symbol_table;
    return node;
}


/******************************************************************************
Below is the parser
******************************************************************************/

//#define debug(fmt, arg) fprintf(stderr, fmt, arg)
#define debug(fmt, arg) do{}while(0) 

int peek1(struct ParserState* ps, int offset)
{
    if (ps->pointer + offset >= ps->program_length) return -1;
    return ps->program[ps->pointer + offset];
}

int peek(struct ParserState* ps)
{
    return peek1(ps, 0);
}

static int isnewline(int c)
{
    return c == '\n' || c == '\r';
}

void skip_spaces(struct ParserState* ps)
{
    while (1) {
        while (isspace(peek(ps))) {
            ps->pointer++;
        }

        /* skip comments: */
        if (peek(ps) == '/' && peek1(ps, 1) == '/') {
            ps->pointer += 2;
            while (peek(ps) > 0 && !isnewline(peek(ps))) {
                ps->pointer++;
            }
        } else if (peek(ps) == '/' && peek1(ps, 1) == '*') {
            ps->pointer += 2;
            while (peek(ps) > 0 && peek1(ps, 1) > 0 && !(peek(ps) == '*' && peek1(ps, 1) == '/')) {
                ps->pointer++;
            }
        } else break;
    }
}

int accept(struct ParserState* ps, const char* token)
{
    int token_length;
    int i;

    skip_spaces(ps);

    token_length = strlen(token);

    if (ps->pointer + token_length > ps->program_length)
        return 0;

    for (i = 0; i < token_length; i++)
        if (token[i] != ps->program[ps->pointer + i])
            return 0;

    ps->pointer += token_length;
    return 1;}


static void* failure(const char* message)
{
    debug("%s\n", message);
    return 0;
}

const char* dump_name(symbol_t name, struct symbol_table_t* symbol_table) {
    dump_name_impl(dump_buffer, DUMP_BUFFER_SIZE, name, symbol_table);
    return dump_buffer;
}

int dump_name_impl(char* buffer, int buf_size, symbol_t name, struct symbol_table_t* symbol_table)
{   
    return snprintf(buffer, buf_size, "%s", symbol_to_string(symbol_table, name));
}

static int is_name_first_character(int c)
{
    return isalpha(c) || c == '_';
}

static int is_name_character(int c)
{
    return isalnum(c) || c == '_';
}

symbol_t parse_name(struct ParserState* ps)
{
    int length;
    char characters[MAX_NAME_SIZE];

    skip_spaces(ps);
    if (!is_name_first_character(peek(ps))) return SYMBOL_NULL;

    length = 0;
    while (is_name_character(peek(ps))) {
        characters[length++] = peek(ps);
        ps->pointer++;
    }
    characters[length] = '\0';

    return symbol_table_insert(ps->symbol_table, characters);
}

const char* dump_models(struct ModelsNode* models) {
    dump_models_impl(dump_buffer, DUMP_BUFFER_SIZE, models);
    return dump_buffer;
}

int dump_models_impl(char* buffer, int buf_size, struct ModelsNode* models)
{
    int num_written = 0;
    DUMP_CALL(dump_model_impl, buffer, buf_size, models->model);
    if (models->models)
        DUMP_CALL(dump_models_impl, buffer, buf_size, models->models);
    return num_written;
}

ModelsNode* parse_models(ParserState* ps)
{
    struct ModelsNode* models;
    struct ModelNode* model;
    struct ModelsNode* models1;

    model = parse_model(ps);
    if (!model) return failure("parse_models: model expected");
    models1 = parse_models(ps);

    models = new_node(sizeof(struct ModelsNode), ps);
    models->model = model;
    models->models = models1;

    return models;
}

const char* dump_model(struct ModelNode* model) {
    dump_model_impl(dump_buffer, DUMP_BUFFER_SIZE, model);
    return dump_buffer;
}

int dump_model_impl(char* buffer, int buf_size, struct ModelNode* model)
{
    int num_written = 0;
    DUMP_CALL(snprintf, buffer, buf_size, "model ");
    
    DUMP_CALL(dump_name_impl, buffer, buf_size, model->name, node_symbol_table(model));
    DUMP_CALL(snprintf, buffer, buf_size, "(");
    if (model->params) {
      DUMP_CALL(dump_model_params_impl, buffer, buf_size, model->params);
    }
    DUMP_CALL(snprintf, buffer, buf_size, ") {\n");
    //DUMP_CALL(dump_decls_impl, buffer, buf_size, model->decls);
    DUMP_CALL(dump_stmts_impl, buffer, buf_size, model->stmts);
    DUMP_CALL(snprintf, buffer, buf_size, "}\n");
    return num_written;
}

struct ModelNode* parse_model(struct ParserState* ps)
{
    struct ModelNode* model;
    symbol_t name;
    struct ModelParamsNode* params;
    struct DeclsNode* decls = 0;
    struct StmtsNode* stmts;

    if (!accept(ps, "model")) return failure("parse_model: model expected");
    name = parse_name(ps);
    if (name == SYMBOL_NULL) return failure("parse_model: name expected");
    debug("%s\n", "parse_model: name recognized");
    if (!accept(ps, "(")) return failure("parse_model: ( expected");
    debug("%s\n", "parse_model: ( recognized");
    params = parse_model_params(ps);
    if (params)
        debug("%s\n", "parse_model: params recognized");
    else
        debug("%s\n", "parse_model: params not recognized");
    if (!accept(ps, ")")) return failure("parse_model: ) expected");
    debug("%s\n", "parse_model: ) recognized");
    if (!accept(ps, "{")) return failure("parse_model: { expected");
    debug("%s\n", "parse_model: { recognized");
    //decls = parse_decls(ps);
    //if (!decls) return failure("parse_model: decls expected");
    debug("%s\n", "parse_model: decls recognized");
    stmts = parse_stmts(ps);
    if (!stmts) return failure("parse_model: stmts expected");
    debug("%s\n", "parse_model: stmts recognized");
    if (!accept(ps, "}")) return failure("parse_model: } expected");
    debug("%s\n", "parse_model: } recognized");

    model = new_node(sizeof(struct ModelNode), ps);
    model->name = name;
    model->params = params;
    model->decls = decls;
    model->stmts = stmts;

    return model;
}


const char* dump_model_params(struct ModelParamsNode* model_params) {
    dump_model_params_impl(dump_buffer, DUMP_BUFFER_SIZE, model_params);
    return dump_buffer;
}

int dump_model_params_impl(char* buffer, int buf_size, struct ModelParamsNode* model_params)
{
    int num_written = 0;
    DUMP_CALL(dump_name_impl, buffer, buf_size, model_params->name, node_symbol_table(model_params));
    if (model_params->model_params) {
        DUMP_CALL(snprintf, buffer, buf_size, ", ");
        DUMP_CALL(dump_model_params_impl, buffer, buf_size, model_params->model_params);
    }
  return num_written;
}

struct ModelParamsNode* parse_model_params(struct ParserState* ps)
{
    symbol_t name;
    struct ModelParamsNode* model_params1;
    struct ModelParamsNode* model_params;

    name = parse_name(ps);
    if (name == SYMBOL_NULL) return failure("parse_model_params: name expected");
    if (accept(ps, ",")) {
        model_params1 = parse_model_params(ps);
        if (!model_params1) return failure("parse_model_params: model_params expected");
    } else {
        model_params1 = 0;
    }

    model_params = new_node(sizeof(struct ModelParamsNode), ps);
    model_params->name = name;
    model_params->model_params = model_params1;

    return model_params;
}


const char* dump_decls(struct DeclsNode* decls) {
    dump_decls_impl(dump_buffer, DUMP_BUFFER_SIZE, decls);
    return dump_buffer;
}

int dump_decls_impl(char* buffer, int buf_size, struct DeclsNode* decls)
{
    int num_written = 0;
    DUMP_CALL(dump_decl_impl, buffer, buf_size, decls->decl);
    if (decls->decls)
        DUMP_CALL(dump_decls_impl, buffer, buf_size, decls->decls);
    return num_written;
}

struct DeclsNode* parse_decls(struct ParserState* ps)
{
    struct DeclsNode* decls;
    struct DeclNode* decl;
    struct DeclsNode* decls1;

    decl = parse_decl(ps);
    if (!decl) return failure("parse_decls: decl expected");
    decls1 = parse_decls(ps);

    decls = new_node(sizeof(struct DeclsNode), ps);
    decls->decl = decl;
    decls->decls = decls1;

    return decls;
}


const char* dump_stmts(struct StmtsNode* stmts) {
    dump_stmts_impl(dump_buffer, DUMP_BUFFER_SIZE, stmts);
    return dump_buffer;
}

int dump_stmts_impl(char* buffer, int buf_size, struct StmtsNode* stmts)
{
    int num_written = 0;
    DUMP_CALL(dump_stmt_impl, buffer, buf_size, stmts->stmt);
    if (stmts->stmts)
        DUMP_CALL(dump_stmts_impl, buffer, buf_size, stmts->stmts);
    return num_written;
}

struct StmtsNode* parse_stmts(struct ParserState* ps)
{
    struct StmtsNode* stmts;
    struct StmtNode* stmt;
    struct StmtsNode* stmts1;

    stmt = parse_stmt(ps);
    if (!stmt) return failure("parse_stmts: stmt expected");
    stmts1 = parse_stmts(ps);

    stmts = new_node(sizeof(struct StmtsNode), ps);
    stmts->stmt = stmt;
    stmts->stmts = stmts1;

    return stmts;
}


const char* dump_decl(struct DeclNode* decl) {
    dump_decl_impl(dump_buffer, DUMP_BUFFER_SIZE, decl);
    return dump_buffer;
}

int dump_decl_impl(char* buffer, int buf_size, struct DeclNode* decl)
{
    if (decl->type == PUBLIC_DECL)
        return dump_public_decl_impl(buffer, buf_size, (struct PublicDeclNode*)decl);
    else
        return dump_private_decl_impl(buffer, buf_size, (struct PrivateDeclNode*)decl);
}

struct DeclNode* parse_decl(struct ParserState* ps)
{
    struct PublicDeclNode* public_decl;
    struct PrivateDeclNode* private_decl;

    public_decl = parse_public_decl(ps);
    if (public_decl) {
      return (struct DeclNode*)public_decl;
    }

    private_decl = parse_private_decl(ps);
    if (private_decl) {
      return (struct DeclNode*)private_decl;
    }

    return 0;
}


const char* dump_public_decl(struct PublicDeclNode* public_decl) {
    dump_public_decl_impl(dump_buffer, DUMP_BUFFER_SIZE, public_decl);
    return dump_buffer;
}

int dump_public_decl_impl(char* buffer, int buf_size, struct PublicDeclNode* public_decl)
{
    int num_written = 0;
    DUMP_CALL(snprintf, buffer, buf_size, "public ");
    DUMP_CALL(dump_variable_impl, buffer, buf_size, public_decl->variable);
    DUMP_CALL(snprintf, buffer, buf_size, "\n");
    return num_written;
}

struct PublicDeclNode* parse_public_decl(struct ParserState* ps)
{
    struct VariableNode* variable;
    struct PublicDeclNode* public_decl;

    if (!accept(ps, "public")) return failure("parse_public_decl: public expected");
    variable = parse_variable(ps);
    if (!variable) return failure("parse_public_decl: variable expected");
    debug("%s\n", "parse_public_decl: recognized");

    public_decl = new_node(sizeof(struct PublicDeclNode), ps);
    public_decl->super.type = PUBLIC_DECL;
    public_decl->variable = variable;

    return public_decl;
}


const char* dump_private_decl(struct PrivateDeclNode* private_decl) {
    dump_private_decl_impl(dump_buffer, DUMP_BUFFER_SIZE, private_decl);
    return dump_buffer;
}

int dump_private_decl_impl(char* buffer, int buf_size, struct PrivateDeclNode* private_decl)
{
    int num_written = 0;
    DUMP_CALL(snprintf, buffer, buf_size, "private ");
    DUMP_CALL(dump_variable_impl, buffer, buf_size, private_decl->variable);
    DUMP_CALL(snprintf, buffer, buf_size, "\n");
    return num_written;
}

struct PrivateDeclNode* parse_private_decl(struct ParserState* ps)
{
    struct VariableNode* variable;
    struct PrivateDeclNode* private_decl;

    if (!accept(ps, "private")) return failure("parse_private_decl: private expected");
    variable = parse_variable(ps);
    if (!variable) return failure("parse_private_decl: variable expected");
    debug("%s\n", "parse_private_decl: recognized");

    private_decl = new_node(sizeof(struct PrivateDeclNode), ps);
    private_decl->super.type = PRIVATE_DECL;
    private_decl->variable = variable;

    return private_decl;
}


const char* dump_stmt(struct StmtNode* stmt) {
    dump_stmt_impl(dump_buffer, DUMP_BUFFER_SIZE, stmt);
    return dump_buffer;
}

int dump_stmt_impl(char* buffer, int buf_size, struct StmtNode* stmt)
{
    if (stmt->type == DRAW_STMT)
        return dump_draw_stmt_impl(buffer, buf_size, (struct DrawStmtNode*)stmt);
    else if (stmt->type == LET_STMT)
        return dump_let_stmt_impl(buffer, buf_size, (struct LetStmtNode*)stmt);
    else if (stmt->type == FOR_STMT)
        return dump_for_stmt_impl(buffer, buf_size, (struct ForStmtNode*)stmt);
    else if (stmt->type == WHILE_STMT)
        return dump_while_stmt_impl(buffer, buf_size, (struct WhileStmtNode*) stmt);
    return snprintf(buffer, buf_size, "unrecognized stmt type\n");
}


const char* dump_draw_stmt(struct DrawStmtNode* draw_stmt) {
    dump_draw_stmt_impl(dump_buffer, DUMP_BUFFER_SIZE, draw_stmt);
    return dump_buffer;
}

int dump_draw_stmt_impl(char* buffer, int buf_size, struct DrawStmtNode* draw_stmt)
{
    int num_written = 0;
    DUMP_CALL(dump_variable_impl, buffer, buf_size, draw_stmt->variable);
    DUMP_CALL(snprintf, buffer, buf_size, " ~ ");
    //dump_name(draw_stmt->dist, node_symbol_table(draw_stmt));
    DUMP_CALL(snprintf, buffer, buf_size, "%s", erp_name(draw_stmt->dist_type));
    DUMP_CALL(snprintf, buffer, buf_size, "(");
    DUMP_CALL(dump_expr_seq_impl, buffer, buf_size, draw_stmt->expr_seq);
    DUMP_CALL(snprintf, buffer, buf_size, ")\n");
    return num_written;
}


const char* dump_let_stmt(struct LetStmtNode* let_stmt) {
    dump_let_stmt_impl(dump_buffer, DUMP_BUFFER_SIZE, let_stmt);
    return dump_buffer;
}

int dump_let_stmt_impl(char* buffer, int buf_size, struct LetStmtNode* let_stmt)
{
    int num_written = 0;
    DUMP_CALL(dump_variable_impl, buffer, buf_size, let_stmt->variable);
    DUMP_CALL(snprintf, buffer, buf_size, " = ");
    DUMP_CALL(dump_expr_impl, buffer, buf_size, let_stmt->expr);
    DUMP_CALL(snprintf, buffer, buf_size, "\n");
    return num_written;
}

struct StmtNode* parse_stmt(struct ParserState* ps)
{
    struct ForStmtNode* for_stmt;
    struct WhileStmtNode* while_stmt;
    struct VariableNode* variable;
    symbol_t dist;
    struct ExprSeqNode* expr_seq;
    struct DrawStmtNode* draw_stmt;
    struct ExprNode* expr;
    struct LetStmtNode* let_stmt;

    for_stmt = parse_for_stmt(ps);
    if (for_stmt) {
        debug("%s\n", "parse_stmt: for_stmt recognized");
        return (struct StmtNode*)for_stmt;
    }

    while_stmt = parse_while_stmt(ps);
    if (while_stmt) {
        debug("%s\n", "parse_stmt: while_stmt recognized");
        return (struct StmtNode*) while_stmt;
    }

    variable = parse_variable(ps);
    if (!variable) return failure("parse_stmt: variable expected");

    if (accept(ps, "~")) {
        dist = parse_name(ps);
        if (dist == SYMBOL_NULL) return failure("parse_stmt: dist expected");
        if (!accept(ps, "(")) return failure("parse_stmt: ( expected");
        expr_seq = parse_expr_seq(ps);
        if (!expr_seq) return failure("parse_stmt: expr_seq expected");
        if (!accept(ps, ")")) return failure("parse_stmt: ) expected");
        debug("%s\n", "parse_stmt: draw_stmt recognized");

        erp_enum_t dist_type = erp_type(symbol_to_string(ps->symbol_table, dist)); 

        draw_stmt = new_node(sizeof(struct DrawStmtNode), ps);
        draw_stmt->super.type = DRAW_STMT;
        draw_stmt->variable = variable;
        //draw_stmt->dist = dist;
        draw_stmt->dist_type = dist_type;
        draw_stmt->expr_seq = expr_seq;

        return (struct StmtNode*)draw_stmt;
    }
    
    if (accept(ps, "=")) {
        expr = parse_expr(ps);
        if (!expr) return failure("parse_stmt: expr expected");
        debug("%s\n", "parse_stmt: let_stmt recognized");

        let_stmt = new_node(sizeof(struct LetStmtNode), ps);
        let_stmt->super.type = LET_STMT;
        let_stmt->variable = variable;
        let_stmt->expr = expr;

        return (struct StmtNode*)let_stmt;
    }

    return 0;
}

const char* dump_for_stmt(struct ForStmtNode* for_stmt) {
    dump_for_stmt_impl(dump_buffer, DUMP_BUFFER_SIZE, for_stmt);
    return dump_buffer;
}

int dump_for_stmt_impl(char* buffer, int buf_size, struct ForStmtNode* for_stmt)
{
    int num_written = 0;
    DUMP_CALL(snprintf, buffer, buf_size, "for ");
    DUMP_CALL(dump_name_impl, buffer, buf_size, for_stmt->name, node_symbol_table(for_stmt));
    DUMP_CALL(snprintf, buffer, buf_size, " = ");
    DUMP_CALL(dump_expr_impl, buffer, buf_size, for_stmt->start_expr);
    DUMP_CALL(snprintf, buffer, buf_size, " to ");
    DUMP_CALL(dump_expr_impl, buffer, buf_size, for_stmt->end_expr);
    DUMP_CALL(snprintf, buffer, buf_size, " {\n");
    DUMP_CALL(dump_stmts_impl, buffer, buf_size, for_stmt->stmts);
    DUMP_CALL(snprintf, buffer, buf_size, "}\n");
    return num_written;
}

struct ForStmtNode* parse_for_stmt(struct ParserState* ps)
{
    symbol_t name;
    struct ExprNode* expr;
    struct ExprNode* expr1;
    struct StmtsNode* stmts;
    struct ForStmtNode* for_stmt;

    if (!accept(ps, "for")) return failure("parse_for_stmt: for expected");
    debug("%s\n", "parse_for_stmt: for recognized");
    name = parse_name(ps);
    if (name == SYMBOL_NULL) return failure("parse_for_stmt: name expected");
    debug("%s\n", "parse_for_stmt: name recognized");
    if (!accept(ps, "=")) return failure("parse_for_stmt: = expected");
    debug("%s\n", "parse_for_stmt: = recognized");
    debug("parse_for_stmt: %d\n", ps->pointer);
    debug("%c\n", ps->program[ps->pointer]);
    expr = parse_expr(ps);
    if (!expr) return failure("parse_for_stmt: expr expected");
    debug("%s\n", "parse_for_stmt: expr recognized");
    debug("parse_for_stmt: %d\n", ps->pointer);
    debug("%c\n", ps->program[ps->pointer]);
    if (!accept(ps, "to")) return failure("parse_for_stmt: to expected");
    debug("%s\n", "parse_for_stmt: to recognized");
    debug("parse_for_stmt: %d\n", ps->pointer);
    debug("%c\n", ps->program[ps->pointer]);
    expr1 = parse_expr(ps);
    if (!expr1) return failure("parse_for_stmt: expr1 expected");
    debug("%s\n", "parse_for_stmt: expr1 recognized");
    debug("parse_for_stmt: %d\n", ps->pointer);
    debug("%c\n", ps->program[ps->pointer]);
    if (!accept(ps, "{")) return failure("parse_for_stmt: { expected");
    debug("%s\n", "parse_for_stmt: { recognized");
    stmts = parse_stmts(ps);
    if (!stmts) return failure("parse_for_stmt: stmts expected");
    debug("%s\n", "parse_for_stmt: stmts recognized");
    if (!accept(ps, "}")) return failure("parse_for_stmt: } expected");
    debug("%s\n", "parse_for_stmt: } recognized");

    for_stmt = new_node(sizeof(struct ForStmtNode), ps);
    for_stmt->super.type = FOR_STMT;
    for_stmt->name = name;
    for_stmt->start_expr = expr;
    for_stmt->end_expr = expr1;
    for_stmt->stmts = stmts;

    return for_stmt;
}

WhileStmtNode* parse_while_stmt(ParserState* ps) {
    if (!accept(ps, "while")) {
        return failure("while stmt: while expected");
    }
    if (!accept(ps, "(")) {
        return failure("while stmt: ( expected");
    }
    ExprNode* condition = parse_expr(ps);
    if (!condition) {
        return failure("while stmt: expression expected");
    }
    if (!accept(ps, ")")) {
        return failure("while stmt: ) expected");
    }
    if (!accept(ps, "{")){
        return failure("while stmt: { expected");
    }
    StmtsNode* stmts = parse_stmts(ps);
    if (!stmts) {
        return failure("while stmt: stmts expected");
    }
    if (!accept(ps, "}")) {
        return failure("while stmt: } expected");
    }

    WhileStmtNode* while_stmt = malloc(sizeof(WhileStmtNode));
    while_stmt->super.type = WHILE_STMT;
    while_stmt->condition = condition;
    while_stmt->stmts = stmts;

    return while_stmt;
}

const char* dump_while_stmt(WhileStmtNode* while_stmt) {
    dump_while_stmt_impl(dump_buffer, DUMP_BUFFER_SIZE, while_stmt);
    return dump_buffer;
}

int dump_while_stmt_impl(char* buffer, int buf_size, WhileStmtNode* while_stmt) {
    int num_written = 0;
    DUMP_CALL(snprintf, buffer, buf_size, "while (");
    DUMP_CALL(dump_expr_impl, buffer, buf_size, while_stmt->condition);
    DUMP_CALL(snprintf, buffer, buf_size, ") {\n");
    DUMP_CALL(dump_stmts_impl, buffer, buf_size, while_stmt->stmts);
    DUMP_CALL(snprintf, buffer, buf_size, "}\n");
    return num_written; 
}

size_t expr_seq_length(struct ExprSeqNode* expr_seq)
{
    if (expr_seq)
        return 1 + expr_seq_length(expr_seq->expr_seq);
    else
        return 0;
}

const char* dump_expr_seq(struct ExprSeqNode* expr_seq) {
    dump_expr_seq_impl(dump_buffer, DUMP_BUFFER_SIZE, expr_seq);
    return dump_buffer;
}

int dump_expr_seq_impl(char* buffer, int buf_size, struct ExprSeqNode* expr_seq)
{
    int num_written = 0;
    DUMP_CALL(dump_expr_impl, buffer, buf_size, expr_seq->expr);
    if (expr_seq->expr_seq) {
        DUMP_CALL(snprintf, buffer, buf_size, ", ");
        DUMP_CALL(dump_expr_seq_impl, buffer, buf_size, expr_seq->expr_seq);
    }
    return num_written;
}

struct ExprSeqNode* parse_expr_seq(struct ParserState* ps)
{
    struct ExprNode* expr;
    struct ExprSeqNode* expr_seq1;
    struct ExprSeqNode* expr_seq;

    expr = parse_expr(ps);
    if (!expr) return failure("parse_expr_seq: expr expected");

    if (accept(ps, ",")) {
        expr_seq1 = parse_expr_seq(ps);
        if (!expr_seq1) return failure("parse_expr_seq: expr_seq1 expected");

        expr_seq = new_node(sizeof(struct ExprSeqNode), ps);
        expr_seq->expr = expr;
        expr_seq->expr_seq = expr_seq1;

        return expr_seq;
    }

    expr_seq = new_node(sizeof(struct ExprSeqNode), ps);
    expr_seq->expr = expr;
    expr_seq->expr_seq = 0;

    return expr_seq;
}


const char* dump_expr(struct ExprNode* expr) {
    dump_expr_impl(dump_buffer, DUMP_BUFFER_SIZE, expr);
    return dump_buffer;
}

int dump_expr_impl(char* buffer, int buf_size, struct ExprNode* expr)
{
    if (expr->type == IF_EXPR)
        return dump_if_expr_impl(buffer, buf_size, (struct IfExprNode*)expr);
    if (expr->type == NEW_EXPR)
        return dump_new_expr_impl(buffer, buf_size, (struct NewExprNode*)expr);
    if (expr->type == BINARY_EXPR)
        return dump_binary_expr_impl(buffer, buf_size, (struct BinaryExprNode*)expr);
    if (expr->type == PRIMARY_EXPR)
        return dump_primary_impl(buffer, buf_size, (struct PrimaryExprNode*)expr);
    return snprintf(buffer, buf_size, "unrecognized expr type\n");
}


const char* dump_if_expr(struct IfExprNode* if_expr) {
    dump_if_expr_impl(dump_buffer, DUMP_BUFFER_SIZE, if_expr);
    return dump_buffer;
}

int dump_if_expr_impl(char* buffer, int buf_size, struct IfExprNode* if_expr)
{
    int num_written = 0;
    DUMP_CALL(snprintf, buffer, buf_size, "if ");
    DUMP_CALL(dump_expr_impl, buffer, buf_size, if_expr->condition);
    DUMP_CALL(snprintf, buffer, buf_size, " then ");
    DUMP_CALL(dump_expr_impl, buffer, buf_size, if_expr->consequent);
    DUMP_CALL(snprintf, buffer, buf_size, " else ");
    DUMP_CALL(dump_expr_impl, buffer, buf_size, if_expr->alternative);
    return num_written;
}


const char* dump_new_expr(struct NewExprNode* new_expr) {
    dump_new_expr_impl(dump_buffer, DUMP_BUFFER_SIZE, new_expr);
    return dump_buffer;
}

int dump_new_expr_impl(char* buffer, int buf_size, struct NewExprNode* new_expr)
{
    int num_written = 0;
    DUMP_CALL(snprintf, buffer, buf_size, "new ");
    DUMP_CALL(dump_name_impl, buffer, buf_size, new_expr->name, node_symbol_table(new_expr));
    DUMP_CALL(snprintf, buffer, buf_size, "()");
    return num_written;
}

struct ExprNode* parse_expr(struct ParserState* ps)
{
    struct ExprNode* expr;
    struct ExprNode* expr1;
    struct ExprNode* expr2;
    struct IfExprNode* if_expr;
    symbol_t name;
    struct NewExprNode* new_expr;

    if (accept(ps, "if")) {
        expr = parse_expr(ps);
        if (!expr) return failure("parse_expr: expr expected");
        if (!accept(ps, "then")) return failure("parse_expr: then expected");
        expr1 = parse_expr(ps);
        if (!expr1) return failure("parse_expr: expr1 expected");
        if (!accept(ps, "else")) return failure("parse_expr: else expected");
        expr2 = parse_expr(ps);
        if (!expr2) return failure("parse_expr: expr2 expected");

        if_expr = new_node(sizeof(struct IfExprNode), ps);
        if_expr->super.type = IF_EXPR;
        if_expr->condition = expr;
        if_expr->consequent = expr1;
        if_expr->alternative = expr2;

        return (struct ExprNode*)if_expr;
    }

    if (accept(ps, "new")) {
        name = parse_name(ps);
        if (name == SYMBOL_NULL) return failure("parse_expr: name expected");
        if (!accept(ps, "(")) return failure("parse_expr: ( expected");
        if (!accept(ps, ")")) return failure("parse_expr: ) expected");

        new_expr = new_node(sizeof(struct NewExprNode), ps);
        new_expr->super.type = NEW_EXPR;
        new_expr->name = name;

        return (struct ExprNode*)new_expr;
    }

    return parse_add_expr(ps);
}


const char* dump_binary_expr(struct BinaryExprNode* expr) {
    dump_binary_expr_impl(dump_buffer, DUMP_BUFFER_SIZE, expr);
    return dump_buffer;
}

int dump_binary_expr_impl(char* buffer, int buf_size, struct BinaryExprNode* expr)
{
    int num_written = 0;
    DUMP_CALL(dump_expr_impl, buffer, buf_size, expr->left);
    if (expr->op == OP_ADD)
        DUMP_CALL(snprintf, buffer, buf_size, " + ");
    else if (expr->op == OP_SUB)
        DUMP_CALL(snprintf, buffer, buf_size, " - ");
    else if (expr->op == OP_MUL)
        DUMP_CALL(snprintf, buffer, buf_size, " * ");
    else if (expr->op == OP_DIV)
        DUMP_CALL(snprintf, buffer, buf_size, " / ");
    else
        DUMP_CALL(snprintf, buffer, buf_size, " ? ");
    DUMP_CALL(dump_expr_impl, buffer, buf_size, expr->right);
    return num_written;
}

struct ExprNode* parse_add_expr(struct ParserState* ps)
{
    struct ExprNode* term;
    struct ExprNode* add_expr1;
    struct BinaryExprNode* add_expr;

    term = parse_term(ps);
    if (!term) return failure("parse_add_expr: term expected");

    if (accept(ps, "+")) {
        add_expr1 = parse_add_expr(ps);
        if (!add_expr1) return failure("parse_add_expr: add_expr1 expected");

        add_expr = new_node(sizeof(struct BinaryExprNode), ps);
        add_expr->super.type = BINARY_EXPR;
        add_expr->op = OP_ADD;
        add_expr->left = term;
        add_expr->right = add_expr1;

        return (struct ExprNode*)add_expr;
    }

    if (accept(ps, "-")) {
        add_expr1 = parse_add_expr(ps);
        if (!add_expr1) return failure("parse_add_expr: add_expr1 expected");

        add_expr = new_node(sizeof(struct BinaryExprNode), ps);
        add_expr->super.type = BINARY_EXPR;
        add_expr->op = OP_SUB;
        add_expr->left = term;
        add_expr->right = add_expr1;

        return (struct ExprNode*)add_expr;
    }

    return term;
}

struct ExprNode* parse_term(struct ParserState* ps)
{
    struct PrimaryExprNode* primary;
    struct ExprNode* term1;
    struct BinaryExprNode* term;

    primary = parse_primary(ps);
    if (!primary) return failure("parse_term: primary expected");

    if (accept(ps, "*")) {
        term1 = parse_term(ps);
        if (!term1) return failure("parse_term: term1 expected");

        term = new_node(sizeof(struct BinaryExprNode), ps);
        term->super.type = BINARY_EXPR;
        term->op = OP_MUL;
        term->left = (struct ExprNode*)primary;
        term->right = term1;

        return (struct ExprNode*)term;
    }

    if (accept(ps, "/")) {
        term1 = parse_term(ps);
        if (!term1) return failure("parse_term: term1 expected");

        term = new_node(sizeof(struct BinaryExprNode), ps);
        term->super.type = BINARY_EXPR;
        term->op = OP_DIV;
        term->left = (struct ExprNode*)primary;
        term->right = term1;

        return (struct ExprNode*)term;
    }

    return (struct ExprNode*)primary;
}

const char* dump_unary_expr(struct UnaryExprNode* unary) {
    dump_unary_expr_impl(dump_buffer, DUMP_BUFFER_SIZE, unary);
    return dump_buffer;
}

int dump_unary_expr_impl(char* buffer, int buf_size, struct UnaryExprNode* unary)
{
    int num_written = 0;
    if (unary->op == OP_NEG)
        DUMP_CALL(snprintf, buffer, buf_size, "-");
    DUMP_CALL(dump_primary_impl, buffer, buf_size, unary->primary);
    return num_written;
}

const char* dump_primary(struct PrimaryExprNode* primary) {
    dump_primary_impl(dump_buffer, DUMP_BUFFER_SIZE, primary);
    return dump_buffer;
}

int dump_primary_impl(char* buffer, int buf_size, struct PrimaryExprNode* primary)
{
    if (primary->type == NUM_EXPR)
        return dump_numerical_value_impl(buffer, buf_size, ((struct NumExprNode*)primary)->numerical_value);

    if (primary->type == VAR_EXPR)
        return dump_variable_impl(buffer, buf_size, ((struct VarExprNode*)primary)->variable);

    if (primary->type == UNARY_EXPR)
        return dump_unary_expr_impl(buffer, buf_size, (struct UnaryExprNode*)primary);

    if (primary->type == GROUP_EXPR) {
        int num_written = 0;
        DUMP_CALL(snprintf, buffer, buf_size, "(");
        DUMP_CALL(dump_expr_impl, buffer, buf_size, ((struct GroupExprNode*)primary)->expr);
        DUMP_CALL(snprintf, buffer, buf_size, ")");
        return num_written;
    }

    if (primary->type == FUNC_EXPR) {
        int num_written = 0;
        DUMP_CALL(dump_name_impl, buffer, buf_size, ((struct FuncExprNode*)primary)->name, node_symbol_table(primary));
        DUMP_CALL(snprintf, buffer, buf_size, "(");
        DUMP_CALL(dump_expr_seq_impl, buffer, buf_size, ((struct FuncExprNode*)primary)->expr_seq);
        DUMP_CALL(snprintf, buffer, buf_size, ")");
        return num_written;
    }

    return snprintf(buffer, buf_size, "unrecognized primary type\n");
}

struct PrimaryExprNode* parse_primary(struct ParserState* ps)
{
    struct PrimaryExprNode* primary;
    struct UnaryExprNode* unary;
    struct ExprNode* expr;
    struct GroupExprNode* group;
    struct NumericalValueNode* numerical_value;
    struct NumExprNode* num_expr;
    symbol_t name;
    struct ExprSeqNode* expr_seq;
    struct FuncExprNode* func_expr;
    struct VariableNode* variable;
    struct VarExprNode* var_expr;

    if (accept(ps, "-")) {
        primary = parse_primary(ps);
        if (!primary) return failure("parse_primary: primary expected");

        unary = new_node(sizeof(struct UnaryExprNode), ps);
        unary->super.super.type = PRIMARY_EXPR;
        unary->super.type = UNARY_EXPR;
        unary->op = OP_NEG;
        unary->primary = primary;

        return (struct PrimaryExprNode*)unary;
    }

    if (accept(ps, "(")) {
        expr = parse_expr(ps);
        if (!expr) return failure("parse_primary: expr expected");
        if (!accept(ps, ")")) return failure("parse_primary: ) expected");

        group = new_node(sizeof(struct GroupExprNode), ps);
        group->super.super.type = PRIMARY_EXPR;
        group->super.type = GROUP_EXPR;
        group->expr = expr;

        return (struct PrimaryExprNode*)group;
    }

    numerical_value = parse_numerical_value(ps);
    if (numerical_value) {
        num_expr = new_node(sizeof(struct NumExprNode), ps);
        num_expr->super.super.type = PRIMARY_EXPR;
        num_expr->super.type = NUM_EXPR;
        num_expr->numerical_value = numerical_value;

        return (struct PrimaryExprNode*)num_expr;
    }

    name = parse_name(ps);
    if (name == SYMBOL_NULL) return failure("parse_primary: name expected");

    if (accept(ps, "(")) {
        expr_seq = parse_expr_seq(ps);
        if (!expr_seq) return failure("parse_primary: expr_seq expected");
        if (!accept(ps, ")")) return failure("parse_primary: ) expected");

        func_expr = new_node(sizeof(struct FuncExprNode), ps);
        func_expr->super.super.type = PRIMARY_EXPR;
        func_expr->super.type = FUNC_EXPR;
        func_expr->name = name;
        func_expr->expr_seq = expr_seq;

        return (struct PrimaryExprNode*)func_expr;
    }

    variable = parse_variable1(ps, name);
    if (variable) {
        var_expr = new_node(sizeof(struct VarExprNode), ps);
        var_expr->super.super.type = PRIMARY_EXPR;
        var_expr->super.type = VAR_EXPR;
        var_expr->variable = variable;

        return (struct PrimaryExprNode*)var_expr;
    }

    return 0;
}


const char* dump_variable(struct VariableNode* variable) {
    dump_variable_impl(dump_buffer, DUMP_BUFFER_SIZE, variable);
    return dump_buffer;
}

int dump_variable_impl(char* buffer, int buf_size, struct VariableNode* variable)
{
    if (variable->type == NAME_VAR)
        return dump_name_var_impl(buffer, buf_size, (struct NameVarNode*)variable);
    if (variable->type == FIELD_VAR)
        return dump_field_var_impl(buffer, buf_size, (struct FieldVarNode*)variable);
    if (variable->type == INDEX_VAR)
        return dump_index_var_impl(buffer, buf_size, (struct IndexVarNode*)variable);
    return snprintf(buffer, buf_size, "unrecognized variable type\n");
}

const char* dump_name_var(struct NameVarNode* name_var) {
    dump_name_var_impl(dump_buffer, DUMP_BUFFER_SIZE, name_var);
    return dump_buffer;
}

int dump_name_var_impl(char* buffer, int buf_size, struct NameVarNode* name_var)
{
    return dump_name_impl(buffer, buf_size, name_var->name, node_symbol_table(name_var));
}


const char* dump_field_var(struct FieldVarNode* field_var) {
    dump_field_var_impl(dump_buffer, DUMP_BUFFER_SIZE, field_var);
    return dump_buffer;
}

int dump_field_var_impl(char* buffer, int buf_size, struct FieldVarNode* field_var)
{
    int num_written = 0;
    DUMP_CALL(dump_name_impl, buffer, buf_size, field_var->name, node_symbol_table(field_var));
    DUMP_CALL(snprintf, buffer, buf_size, ".");
    DUMP_CALL(dump_name_impl, buffer, buf_size, field_var->field_name, node_symbol_table(field_var));
    return num_written;
}


const char* dump_index_var(struct IndexVarNode* index_var) {
    dump_index_var_impl(dump_buffer, DUMP_BUFFER_SIZE, index_var);
    return dump_buffer;
}

int dump_index_var_impl(char* buffer, int buf_size, struct IndexVarNode* index_var)
{
    int num_written = 0;
    DUMP_CALL(dump_name_impl, buffer, buf_size, index_var->name, node_symbol_table(index_var));
    DUMP_CALL(snprintf, buffer, buf_size, "[");
    DUMP_CALL(dump_expr_seq_impl, buffer, buf_size, index_var->expr_seq);
    DUMP_CALL(snprintf, buffer, buf_size, "]");
    return num_written;
}

struct VariableNode* parse_variable(struct ParserState* ps)
{
    symbol_t name;

    name = parse_name(ps);
    if (name == SYMBOL_NULL) return failure("parse_variable: name expected");
    return parse_variable1(ps, name);
}

struct VariableNode* parse_variable1(struct ParserState* ps, symbol_t name)
{
    symbol_t field_name;
    struct ExprSeqNode* expr_seq;
    struct FieldVarNode* field_var;
    struct IndexVarNode* index_var;
    struct NameVarNode* name_var;

    if (accept(ps, ".")) {
        field_name = parse_name(ps);
        if (field_name == SYMBOL_NULL) return failure("parse_variable1: field_name expected");

        field_var = new_node(sizeof(struct FieldVarNode), ps);
        field_var->super.type = FIELD_VAR;
        field_var->name = name;
        field_var->field_name = field_name;

        return (struct VariableNode*)field_var;
    }

    if (accept(ps, "[")) {
        expr_seq = parse_expr_seq(ps);
        if (!expr_seq) return failure("parse_variable1: expr_seq expected");
        if (!accept(ps, "]")) return failure("parse_variable1: ] expected");

        index_var = new_node(sizeof(struct IndexVarNode), ps);
        index_var->super.type = INDEX_VAR;
        index_var->name = name;
        index_var->expr_seq = expr_seq;

        return (struct VariableNode*)index_var;
    }

    name_var = new_node(sizeof(struct NameVarNode), ps);
    name_var->super.type = NAME_VAR;
    name_var->name = name;

    return (struct VariableNode*)name_var;
}


const char* dump_numerical_value(struct NumericalValueNode* numerical_value) {
    dump_numerical_value_impl(dump_buffer, DUMP_BUFFER_SIZE, numerical_value);
    return dump_buffer;
}

int dump_numerical_value_impl(char* buffer, int buf_size, struct NumericalValueNode* numerical_value)
{
    if (numerical_value->type == REAL_VALUE)
        return dump_real_value_impl(buffer, buf_size, (struct RealValueNode*)numerical_value);
    if (numerical_value->type == INTEGER_VALUE)
        return dump_integer_value_impl(buffer, buf_size, (struct IntegerValueNode*)numerical_value);
    return snprintf(buffer, buf_size, "unrecognized numerical value type\n");
}


const char* dump_real_value(struct RealValueNode* real_value) {
    dump_real_value_impl(dump_buffer, DUMP_BUFFER_SIZE, real_value);
    return dump_buffer;
}

int dump_real_value_impl(char* buffer, int buf_size, struct RealValueNode* real_value)
{
    /*dump_integer_value(real_value->integer_part);
    printf(".");
    return dump_integer_value(real_value->fractional);*/
    return snprintf(buffer, buf_size, "%f", real_value->value);
}

struct NumericalValueNode* parse_numerical_value(struct ParserState* ps)
{
    struct IntegerValueNode* integer_value;
//    struct IntegerValueNode* fractional;
    struct RealValueNode* real_value;

    integer_value = parse_integer_value(ps);
    if (!integer_value) return failure("parse_numerical_value: integer_value expected");

    if (accept(ps, ".")) {
        char digits[MAX_INTEGER_VALUE_SIZE+3];
        int length = 0;
        digits[length++] = '.';
        while (length < MAX_INTEGER_VALUE_SIZE && isdigit(peek(ps))) {
            digits[length++] = peek(ps);
            ps->pointer++;
        }

        if (isdigit(peek(ps))) {
            failure("parse_numerical_value: precision loss");
            do {ps->pointer++;} while (isdigit(peek(ps))); 
        }

		digits[length++] = '\0';

        real_value = new_node(sizeof(struct RealValueNode), ps);
        real_value->super.type = REAL_VALUE;
        real_value->value = integer_value->value + atof(digits);

        free(integer_value);
       // real_value->integer_part = integer_value;
       // real_value->fractional = fractional;

        return (struct NumericalValueNode*)real_value;
    }

    return (struct NumericalValueNode*)integer_value;
}


const char* dump_integer_value(struct IntegerValueNode* integer_value) {
    dump_integer_value_impl(dump_buffer, DUMP_BUFFER_SIZE, integer_value);
    return dump_buffer;
}

int dump_integer_value_impl(char* buffer, int buf_size, struct IntegerValueNode* integer_value)
{
    return snprintf(buffer, buf_size, "%d", integer_value->value);
    //return printf("%s", integer_value->digits);
}

struct IntegerValueNode* parse_integer_value(struct ParserState* ps)
{
    struct IntegerValueNode* integer_value;

    skip_spaces(ps);

    if (!isdigit(peek(ps))) return failure("parse_integer_value: digit expected");

    char digits[MAX_INTEGER_VALUE_SIZE + 1];
    int length = 0;
    while (isdigit(peek(ps))) {
        digits[length++] = peek(ps);
        ps->pointer++;
    }
    digits[length++] = '\0';

    if (isdigit(peek(ps))) {
        failure("parse_integer_value: overflow");
        do { ps->pointer++; } while (isdigit(peek(ps)));
    }

    integer_value = new_node(sizeof(struct IntegerValueNode), ps);
    integer_value->super.type = INTEGER_VALUE;
    integer_value->value = atoi(digits);

    return integer_value;
}

struct ModelsNode* parse_file1(const char* filename, struct symbol_table_t* symbol_table)
{
    int fd;
    int len;
    void* data;
    struct ParserState* ps;
    struct ModelsNode* models;

    fd = open(filename, O_RDONLY);
    len = lseek(fd, 0, SEEK_END);
    data = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);

    ps = malloc(sizeof(struct ParserState));
    ps->symbol_table = symbol_table;
    ps->pointer = 0;
    ps->program_length = len;
    ps->program = data;

    models = parse_models(ps);
    return models;
}

struct ModelsNode* parse_file(const char* filename)
{
    struct symbol_table_t* symbol_table;

    symbol_table = new_symbol_table();
    return parse_file1(filename, symbol_table);
}

/******************************************************************************
Module interfaces
******************************************************************************/

// struct model_param_map_entry_t
// {
//     symbol_t param_name;
//     float value;
//     struct model_param_map_entry_t* next;
// };

// struct model_param_map_t
// {
//     struct model_param_map_entry_t* entry;
// };

// static struct model_param_map_entry_t* model_param_map_find_entry(struct model_param_map_t* map, symbol_t symbol)
// {
//     struct model_param_map_entry_t* table_entry;

//     table_entry = map->entry;
//     while (table_entry)
//         if (table_entry->param_name == symbol)
//             return table_entry;
//         else
//             table_entry = table_entry->next;

//     return 0;
// }

// static void model_param_map_put(struct model_param_map_t* model_param_map, symbol_t param_name, float value)
// {
//     struct model_param_map_entry_t* table_entry;

//     table_entry = model_param_map_find_entry(model_param_map, param_name);
//     if (table_entry) {
//         table_entry->value = value;
//     } else {
//         table_entry = malloc(sizeof(struct model_param_map_entry_t));
//         table_entry->param_name = param_name;
//         table_entry->value = value;
//         table_entry->next = model_param_map->entry;
//         model_param_map->entry = table_entry;
//     }
// }

// static float model_param_map_get(struct model_param_map_t* model_param_map, symbol_t param_name)
// {
//     struct model_param_map_entry_t* table_entry;

//     table_entry = model_param_map_find_entry(model_param_map, param_name);
//     if (table_entry)
//         return table_entry->value;
//     else
//         return 0;
// }

// struct variable_vertex_map_entry_t
// {
//     symbol_t variable;
//     struct BNVertex* vertex;
//     struct variable_vertex_map_entry_t* next;
// };

// struct variable_vertex_map_t
// {
//     struct variable_vertex_map_entry_t* entry;
// };

// static struct variable_vertex_map_entry_t* variable_vertex_map_find_entry(struct variable_vertex_map_t* map, symbol_t variable)
// {
//     struct variable_vertex_map_entry_t* table_entry;

//     table_entry = map->entry;
//     while (table_entry)
//         if (table_entry->variable == variable)
//             return table_entry;
//         else
//             table_entry = table_entry->next;

//     return 0;
// }

// static void variable_vertex_map_put(struct variable_vertex_map_t* map, symbol_t variable, struct BNVertex* vertex)
// {
//     struct variable_vertex_map_entry_t* table_entry;

//     table_entry = variable_vertex_map_find_entry(map, variable);
//     if (table_entry) {
//         table_entry->vertex = vertex;
//     } else {
//         table_entry = malloc(sizeof(struct variable_vertex_map_entry_t));
//         table_entry->variable = variable;
//         table_entry->vertex = vertex;
//         table_entry->next = map->entry;
//         map->entry = table_entry;
//     }
// }

// static struct BNVertex* variable_vertex_map_get(struct variable_vertex_map_t* map, symbol_t variable)
// {
//     struct variable_vertex_map_entry_t* table_entry;

//     table_entry = variable_vertex_map_find_entry(map, variable);
//     if (table_entry)
//         return table_entry->vertex;
//     else
//         return 0;
// }

// static void instance_append_vertex(struct pp_instance_t* instance, struct BNVertex* vertex, const char* vertex_name)
// {
//     list_append(instance->vertices, vertex);
//     if (vertex_name)
//         list_append(instance->vertex_names, (void*)vertex_name);
//     else
//         /* FIXME should generate unique special names for intermediate results */
//         list_append(instance->vertex_names, "");
//     instance->n++;
// }

// /******************************************************************************
// Compute expressions
// ******************************************************************************/

// static struct BNVertex* compute_expr(struct ExprNode* expr, struct model_param_map_t* model_param_map,
//                                      struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance);

// static struct BNVertex* compute_if_expr(struct IfExprNode* expr, struct model_param_map_t* model_param_map,
//                                         struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
// {
//     struct BNVertex* condition;
//     struct BNVertex* consequent;
//     struct BNVertex* alternative;
//     struct BNVertexComputeIf* vertex;

//     condition = compute_expr(expr->condition, model_param_map, variable_vertex_map, instance);
//     consequent = compute_expr(expr->consequent, model_param_map, variable_vertex_map, instance);
//     alternative = compute_expr(expr->alternative, model_param_map, variable_vertex_map, instance);

//     /* FIXME generate special names for intermediate results */
//     instance_append_vertex(instance, condition, 0);
//     instance_append_vertex(instance, consequent, 0);
//     instance_append_vertex(instance, alternative, 0);

//     vertex = malloc(sizeof(struct BNVertexComputeIf));
//     vertex->super.super.type = BNV_COMPU;
//     vertex->super.type = BNVC_IF;
//     vertex->condition = condition;
//     vertex->consequent = consequent;
//     vertex->alternative = alternative;
//     return (struct BNVertex*)vertex;
// }

// static struct BNVertex* compute_binary_expr(struct BinaryExprNode* expr, struct model_param_map_t* model_param_map,
//                                             struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
// {
//     struct BNVertex* left;
//     struct BNVertex* right;
//     struct BNVertexComputeBinop* vertex;

//     left = compute_expr(expr->left, model_param_map, variable_vertex_map, instance);
//     right = compute_expr(expr->right, model_param_map, variable_vertex_map, instance);

//     instance_append_vertex(instance, left, 0);
//     instance_append_vertex(instance, right, 0);

//     vertex = malloc(sizeof(struct BNVertexComputeBinop));
//     vertex->super.super.type = BNV_COMPU;
//     vertex->super.type = BNVC_BINOP;
//     vertex->left = left;
//     vertex->right = right;

//     if (expr->op == OP_ADD)
//         vertex->binop = BINOP_PLUS;
//     else if (expr->op == OP_SUB)
//         vertex->binop = BINOP_SUB;
//     else if (expr->op == OP_MUL)
//         vertex->binop = BINOP_MULTI;
//     else if (expr->op == OP_DIV)
//         vertex->binop = BINOP_DIV;

//     return (struct BNVertex*)vertex;
// }

// static float evaluate_numerical_value(struct NumericalValueNode* numerical_value);

// static struct BNVertex* compute_numerical_value(struct NumericalValueNode* numerical_value)
// {
//     struct BNVertex* vertex;

//     vertex = malloc(sizeof(struct BNVertex));
//     vertex->type = BNV_CONST;
//     vertex->sample = evaluate_numerical_value(numerical_value);
//     return vertex;
// }

// static const char* variable_to_string(struct VariableNode* variable, struct model_param_map_t* model_param_map);

// static struct BNVertex* compute_variable(struct VariableNode* variable, struct model_param_map_t* model_param_map,
//                                          struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
// {

//     const char* variable_string;
//     symbol_t symbol;

//     struct BNVertex* variable_vertex;
//     struct BNVertexComputeUnary* vertex;

//     variable_string = variable_to_string(variable, model_param_map);
//     symbol = symbol_table_lookup_symbol(node_symbol_table(variable), variable_string);
//     variable_vertex = variable_vertex_map_get(variable_vertex_map, symbol);

//     /* copy the value in case of multiple samples drawn from the same distribution */
//     vertex = malloc(sizeof(struct BNVertexComputeUnary));
//     vertex->super.super.type = BNV_COMPU;
//     vertex->super.type = BNVC_UNARY;
//     vertex->primary = variable_vertex;
//     vertex->op = UNARY_POS;

//     return (struct BNVertex*)vertex;
// }

// static struct BNVertex* compute_primary(struct PrimaryExprNode* primary, struct model_param_map_t* model_param_map,
//                                         struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance);

// static struct BNVertex* compute_unary_expr(struct UnaryExprNode* expr, struct model_param_map_t* model_param_map,
//                                            struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
// {
//     struct BNVertex* primary;
//     struct BNVertexComputeUnary* vertex;

//     primary = compute_primary(expr->primary, model_param_map, variable_vertex_map, instance);

//     instance_append_vertex(instance, primary, 0);

//     vertex = malloc(sizeof(struct BNVertexComputeUnary));
//     vertex->super.super.type = BNV_COMPU;
//     vertex->super.type = BNVC_UNARY;
//     vertex->primary = primary;

//     if (expr->op == OP_NEG)
//         vertex->op = UNARY_NEG;

//     return (struct BNVertex*)vertex;
// }

// static struct BNVertex* compute_func_expr(struct FuncExprNode* expr, struct model_param_map_t* model_param_map,
//                                           struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
// {
//     symbol_t sym_exp;
//     symbol_t sym_log;
//     struct BNVertex* value;
//     struct BNVertexComputeFunc* vertex;

//     sym_exp = symbol_table_lookup_symbol(node_symbol_table(expr), "exp");
//     sym_log = symbol_table_lookup_symbol(node_symbol_table(expr), "log");

//     if (expr->name == sym_exp) {
//         assert(expr_seq_length(expr->expr_seq) == 1);

//         value = compute_expr(expr->expr_seq->expr, model_param_map, variable_vertex_map, instance);
//         instance_append_vertex(instance, value, 0);

//         vertex = malloc(sizeof(struct BNVertexComputeFunc));
//         vertex->super.super.type = BNV_COMPU;
//         vertex->super.type = BNVC_FUNC;
//         vertex->func = FUNC_EXP;
//         vertex->args[0] = value;
//         return (struct BNVertex*)vertex;
//     }

//     if (expr->name == sym_log) {
//         assert(expr_seq_length(expr->expr_seq) == 1);

//         value = compute_expr(expr->expr_seq->expr, model_param_map, variable_vertex_map, instance);
//         instance_append_vertex(instance, value, 0);

//         vertex = malloc(sizeof(struct BNVertexComputeFunc));
//         vertex->super.super.type = BNV_COMPU;
//         vertex->super.type = BNVC_FUNC;
//         vertex->func = FUNC_LOG;
//         vertex->args[0] = value;
//         return (struct BNVertex*)vertex;
//     }

//     fprintf(stderr, "compute_func_expr (%d): unrecognized function %s\n", __LINE__, symbol_to_string(node_symbol_table(expr), expr->name));
//     return 0;
// }

// static struct BNVertex* compute_primary(struct PrimaryExprNode* primary, struct model_param_map_t* model_param_map,
//                                         struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
// {
//     if (primary->type == NUM_EXPR)
//         return compute_numerical_value(((struct NumExprNode*)primary)->numerical_value);
//     if (primary->type == VAR_EXPR)
//         return compute_variable(((struct VarExprNode*)primary)->variable, model_param_map, variable_vertex_map, instance);
//     if (primary->type == UNARY_EXPR)
//         return compute_unary_expr((struct UnaryExprNode*)primary, model_param_map, variable_vertex_map, instance);
//     if (primary->type == GROUP_EXPR)
//         return compute_expr(((struct GroupExprNode*)primary)->expr, model_param_map, variable_vertex_map, instance);
//     if (primary->type == FUNC_EXPR)
//         return compute_func_expr((struct FuncExprNode*)primary, model_param_map, variable_vertex_map, instance);

//     fprintf(stderr, "evaluate_primary (%d): unrecognized type %d\n", __LINE__, primary->type);
//     return 0;
// }

// static struct BNVertex* compute_expr(struct ExprNode* expr, struct model_param_map_t* model_param_map,
//                                      struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
// {
//     if (expr->type == IF_EXPR)
//         return compute_if_expr((struct IfExprNode*)expr, model_param_map, variable_vertex_map, instance);
//     if (expr->type == BINARY_EXPR)
//         return compute_binary_expr((struct BinaryExprNode*)expr, model_param_map, variable_vertex_map, instance);
//     if (expr->type == PRIMARY_EXPR)
//         return compute_primary((struct PrimaryExprNode*)expr, model_param_map, variable_vertex_map, instance);

//     fprintf(stderr, "compute_expr (%d): unrecognized type %d\n", __LINE__, expr->type);
//     return 0;
// }

// /******************************************************************************
// Evaluate expressions
// ******************************************************************************/

// static float evaluate_expr(struct ExprNode* expr, struct model_param_map_t* model_param_map);

// static float evaluate_if_expr(struct IfExprNode* expr, struct model_param_map_t* model_param_map)
// {
//     if (evaluate_expr(expr->condition, model_param_map))
//         return evaluate_expr(expr->consequent, model_param_map);
//     else
//         return evaluate_expr(expr->alternative, model_param_map);
// }

// static float evaluate_binary_expr(struct BinaryExprNode* expr, struct model_param_map_t* model_param_map)
// {
//     float left;
//     float right;

//     left = evaluate_expr(expr->left, model_param_map);
//     right = evaluate_expr(expr->right, model_param_map);

//     if (expr->op == OP_ADD)
//         return left + right;
//     if (expr->op == OP_SUB)
//         return left - right;
//     if (expr->op == OP_MUL)
//         return left * right;
//     if (expr->op == OP_DIV)
//         return left / right;

//     fprintf(stderr, "evaluate_binary_expr (%d): unrecognized operator %d\n", __LINE__, expr->op);
//     return 0;
// }

// static float evaluate_primary(struct PrimaryExprNode* primary, struct model_param_map_t* model_param_map);

// static float evaluate_unary_expr(struct UnaryExprNode* expr, struct model_param_map_t* model_param_map)
// {
//     float value;

//     value = evaluate_primary(expr->primary, model_param_map);
//     if (expr->op == OP_NEG)
//         return -value;

//     fprintf(stderr, "evaluate_unary_expr (%d): unrecognized operator %d\n", __LINE__, expr->op);
//     return 0;
// }

// static float evaluate_func_expr(struct FuncExprNode* expr, struct model_param_map_t* model_param_map)
// {
//     symbol_t sym_exp;
//     symbol_t sym_log;
//     float value;

//     sym_exp = symbol_table_lookup_symbol(node_symbol_table(expr), "exp");
//     sym_log = symbol_table_lookup_symbol(node_symbol_table(expr), "log");

//     if (expr->name == sym_exp) {
//         assert(expr_seq_length(expr->expr_seq) == 1);
//         value = evaluate_expr(expr->expr_seq->expr, model_param_map);
//         return exp(value);
//     }

//     if (expr->name == sym_log) {
//         assert(expr_seq_length(expr->expr_seq) == 1);
//         value = evaluate_expr(expr->expr_seq->expr, model_param_map);
//         return log(value);
//     }

//     fprintf(stderr, "evaluate_func_expr (%d): unrecognized function %s\n", __LINE__, symbol_to_string(node_symbol_table(expr), expr->name));
//     return 0;
// }

// static float evaluate_integer_value(struct IntegerValueNode* integer_value)
// {
//     float value;
// /*    int i;
 
//     value = 0;
//     for (i = 0; i < integer_value->length; i++) {
//         value *= 10;
//         value += integer_value->digits[i] - '0';
//     } */
//     value = integer_value->value;
//     return value;
// }

// static float evaluate_real_value(struct RealValueNode* real_value)
// {
//     float value;
//     float base;
//     int i;

//     value = 0;
//     for (i = 0; i < real_value->integer_part->length; i++) {
//         value *= 10;
//         value += real_value->integer_part->digits[i] - '0';
//     }
//     base = 1;
//     for (i = 0; i < real_value->fractional->length; i++) {
//         base /= 10;
//         value += base * (real_value->fractional->digits[i] - '0');
//     }
//     value = real_value->value;
//     return value;
// }

// static float evaluate_numerical_value(struct NumericalValueNode* numerical_value)
// {
//     if (numerical_value->type == REAL_VALUE)
//         return evaluate_real_value((struct RealValueNode*)numerical_value);

//     if (numerical_value->type == INTEGER_VALUE)
//         return evaluate_integer_value((struct IntegerValueNode*)numerical_value);

//     fprintf(stderr, "evaluate_numerical_value (%d): unrecognized type %d\n", __LINE__, numerical_value->type);
//     return 0;
// }

// static const char* variable_to_string(struct VariableNode* variable, struct model_param_map_t* model_param_map);

// static float evaluate_variable(struct VariableNode* variable, struct model_param_map_t* model_param_map)
// {
//     const char* variable_string;
//     symbol_t symbol;

//     variable_string = variable_to_string(variable, model_param_map);
//     symbol = symbol_table_lookup_symbol(node_symbol_table(variable), variable_string);
//     return model_param_map_get(model_param_map, symbol);
// }

// static float evaluate_primary(struct PrimaryExprNode* primary, struct model_param_map_t* model_param_map)
// {
//     if (primary->type == NUM_EXPR)
//         return evaluate_numerical_value(((struct NumExprNode*)primary)->numerical_value);
//     if (primary->type == VAR_EXPR)
//         return evaluate_variable(((struct VarExprNode*)primary)->variable, model_param_map);
//     if (primary->type == UNARY_EXPR)
//         return evaluate_unary_expr((struct UnaryExprNode*)primary, model_param_map);
//     if (primary->type == GROUP_EXPR)
//         return evaluate_expr(((struct GroupExprNode*)primary)->expr, model_param_map);
//     if (primary->type == FUNC_EXPR)
//         return evaluate_func_expr((struct FuncExprNode*)primary, model_param_map);

//     fprintf(stderr, "evaluate_primary (%d): unrecognized type %d\n", __LINE__, primary->type);
//     return 0;
// }

// static float evaluate_expr(struct ExprNode* expr, struct model_param_map_t* model_param_map)
// {
//     if (expr->type == IF_EXPR)
//         return evaluate_if_expr((struct IfExprNode*)expr, model_param_map);
//     if (expr->type == BINARY_EXPR)
//         return evaluate_binary_expr((struct BinaryExprNode*)expr, model_param_map);
//     if (expr->type == PRIMARY_EXPR)
//         return evaluate_primary((struct PrimaryExprNode*)expr, model_param_map);

//     fprintf(stderr, "evaluate_expr (%d): unrecognized type %d\n", __LINE__, expr->type);
//     return 0;
// }

// static const char* name_var_to_string(struct NameVarNode* name_var)
// {
//     return symbol_to_string(node_symbol_table(name_var), name_var->name);
// }

// static const char* field_var_to_string(struct FieldVarNode* field_var)
// {
//     const char* name;
//     const char* field_name;
//     char* string;

//     name = symbol_to_string(node_symbol_table(field_var), field_var->name);
//     field_name = symbol_to_string(node_symbol_table(field_var), field_var->field_name);

//     string = malloc(strlen(name) + 1 + strlen(field_name) + 1);
//     strcpy(string, name);
//     strcat(string, ".");
//     strcat(string, field_name);
//     return string;
// }

// static const char* index_var_to_string(struct IndexVarNode* index_var, struct model_param_map_t* model_param_map)
// {
//     struct ilist_t* indices;
//     struct ExprSeqNode* expr_seq;
//     const char* name;
//     size_t len;
//     struct ilist_entry_t* index;
//     char index_string[MAX_INTEGER_VALUE_SIZE];
//     char* string;

//     /* Step 1. Evaluate index expressions */
//     indices = new_ilist();
//     expr_seq = index_var->expr_seq;
//     while (expr_seq) {
//         ilist_append(indices, (int)evaluate_expr(expr_seq->expr, model_param_map));
//         expr_seq = expr_seq->expr_seq;
//     }

//     /* Step 2. Calculate index_var string representation length */
//     name = symbol_to_string(node_symbol_table(index_var), index_var->name);
//     len = strlen(name) + 1; /* name[ */
//     index = indices->entry;
//     while (index) {
//         sprintf(index_string, "%d", index->data);
//         len += strlen(index_string);
//         if (index->next)
//             len += 1; /* , */
//         index = index->next;
//     }
//     len += 1; /* ] */

//     /* Step 3. Concatenate index_var string components */
//     string = malloc(len + 1);
//     strcpy(string, name);
//     strcat(string, "[");
//     index = indices->entry;
//     while (index) {
//         sprintf(index_string, "%d", index->data);
//         strcat(string, index_string);
//         if (index->next)
//             strcat(string, ",");
//         index = index->next;
//     }
//     strcat(string, "]");

//     return string;
// }

// static const char* variable_to_string(struct VariableNode* variable, struct model_param_map_t* model_param_map)
// {
//     if (variable->type == NAME_VAR)
//         return name_var_to_string((struct NameVarNode*)variable);
//     if (variable->type == FIELD_VAR)
//         return field_var_to_string((struct FieldVarNode*)variable);
//     if (variable->type == INDEX_VAR)
//         return index_var_to_string((struct IndexVarNode*)variable, model_param_map);
//     return 0;
// }

// /* forward declaration */
// static void execute_stmts(struct StmtsNode* stmts, struct model_param_map_t* model_param_map,
//                           struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance);

// static void execute_draw_dbern(struct DrawStmtNode* draw_stmt, struct model_param_map_t* model_param_map,
//                                struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
// {
//     struct ExprNode* p;
//     struct BNVertex* p_vert;
//     struct BNVertexDrawBern* vertex;
//     const char* variable_string;
//     symbol_t variable;

//     assert(expr_seq_length(draw_stmt->expr_seq) == 1);
//     p = draw_stmt->expr_seq->expr;

//     p_vert = compute_expr(p, model_param_map, variable_vertex_map, instance);
//     instance_append_vertex(instance, p_vert, 0);

//     vertex = malloc(sizeof(struct BNVertexDrawBern));
//     vertex->super.super.type = BNV_DRAW;
//     vertex->super.type = FLIP;
//     vertex->p = p_vert;

//     variable_string = variable_to_string(draw_stmt->variable, model_param_map);
//     instance_append_vertex(instance, (struct BNVertex*)vertex, variable_string);

//     variable = symbol_table_lookup_symbol(node_symbol_table(draw_stmt), variable_string);
//     variable_vertex_map_put(variable_vertex_map, variable, (struct BNVertex*)vertex);
// }

// static void execute_draw_dnorm(struct DrawStmtNode* draw_stmt, struct model_param_map_t* model_param_map,
//                                struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
// {
//     struct ExprNode* mean;
//     struct ExprNode* variance;
//     struct BNVertex* mean_vert;
//     struct BNVertex* variance_vert;
//     struct BNVertexDrawNorm* vertex;
//     const char* variable_string;
//     symbol_t variable;

//     assert(expr_seq_length(draw_stmt->expr_seq) == 2);
//     mean = draw_stmt->expr_seq->expr;
//     variance = draw_stmt->expr_seq->expr_seq->expr;

//     mean_vert = compute_expr(mean, model_param_map, variable_vertex_map, instance);
//     variance_vert = compute_expr(variance, model_param_map, variable_vertex_map, instance);

//     instance_append_vertex(instance, mean_vert, 0);
//     instance_append_vertex(instance, variance_vert, 0);

//     vertex = malloc(sizeof(struct BNVertexDrawNorm));
//     vertex->super.super.type = BNV_DRAW;
//     vertex->super.type = GAUSSIAN;
//     vertex->mean = mean_vert;
//     vertex->variance = variance_vert;

//     variable_string = variable_to_string(draw_stmt->variable, model_param_map);
//     instance_append_vertex(instance, (struct BNVertex*)vertex, variable_string);

//     variable = symbol_table_lookup_symbol(node_symbol_table(draw_stmt), variable_string);
//     variable_vertex_map_put(variable_vertex_map, variable, (struct BNVertex*)vertex);
// }

// static void execute_draw_dgamma(struct DrawStmtNode* draw_stmt, struct model_param_map_t* model_param_map,
//                                 struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
// {
//     struct ExprNode* a;
//     struct ExprNode* b;
//     struct BNVertex* a_vert;
//     struct BNVertex* b_vert;
//     struct BNVertexDrawGamma* vertex;
//     const char* variable_string;
//     symbol_t variable;

//     assert(expr_seq_length(draw_stmt->expr_seq) == 2);
//     a = draw_stmt->expr_seq->expr;
//     b = draw_stmt->expr_seq->expr_seq->expr;

//     a_vert = compute_expr(a, model_param_map, variable_vertex_map, instance);
//     b_vert = compute_expr(b, model_param_map, variable_vertex_map, instance);

//     instance_append_vertex(instance, a_vert, 0);
//     instance_append_vertex(instance, b_vert, 0);

//     vertex = malloc(sizeof(struct BNVertexDrawGamma));
//     vertex->super.super.type = BNV_DRAW;
//     vertex->super.type = GAMMA;
//     vertex->a = a_vert;
//     vertex->b = b_vert;

//     variable_string = variable_to_string(draw_stmt->variable, model_param_map);
//     instance_append_vertex(instance, (struct BNVertex*)vertex, variable_string);

//     variable = symbol_table_lookup_symbol(node_symbol_table(draw_stmt), variable_string);
//     variable_vertex_map_put(variable_vertex_map, variable, (struct BNVertex*)vertex);
// }

// static void execute_draw_stmt(struct DrawStmtNode* draw_stmt, struct model_param_map_t* model_param_map,
//                               struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
// {
//     /* variable ~ dist(expr_seq) */
// /*    symbol_t dbern;
//     symbol_t dnorm;
//     symbol_t dgamma;

//     dbern = symbol_table_lookup_symbol(node_symbol_table(draw_stmt), "dbern");
//     dnorm = symbol_table_lookup_symbol(node_symbol_table(draw_stmt), "dnorm");
//     dgamma = symbol_table_lookup_symbol(node_symbol_table(draw_stmt), "dgamma"); */

//     /*if (draw_stmt->dist == dbern)
//         execute_draw_dbern(draw_stmt, model_param_map, variable_vertex_map, instance);
//     else if (draw_stmt->dist == dnorm)
//         execute_draw_dnorm(draw_stmt, model_param_map, variable_vertex_map, instance);
//     else if (draw_stmt->dist == dgamma)
//         execute_draw_dgamma(draw_stmt, model_param_map, variable_vertex_map, instance);
//     else
//         fprintf(stderr, "execute_draw_stmt (%d): unrecognized distribution", __LINE__);*/
//     switch (draw_stmt->dist_type) {
//     case ERP_FLIP:
//         execute_draw_dbern(draw_stmt, model_param_map, variable_vertex_map, instance);
//         break;
//     case ERP_GAUSSIAN:
//         execute_draw_dnorm(draw_stmt, model_param_map, variable_vertex_map, instance);
//         break;
//     case ERP_GAMMA:
//         execute_draw_dgamma(draw_stmt, model_param_map, variable_vertex_map, instance);
//         break;
//     default:
//         fprintf(stderr, "execute_draw_stmt (%d): unrecognized distribution", __LINE__);
//     }
// }

// static void execute_let_stmt(struct LetStmtNode* let_stmt, struct model_param_map_t* model_param_map,
//                              struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
// {
//     if (let_stmt->expr->type == NEW_EXPR) {
//         /* FIXME variable = new name () */
//         fprintf(stderr, "execute_let_stmt (%d): NEW_EXPR not implemented\n", __LINE__);
//     } else {
//         /* variable = expr */
//         struct BNVertex* vertex;
//         const char* variable_string;
//         symbol_t variable;

//         vertex = compute_expr(let_stmt->expr, model_param_map, variable_vertex_map, instance);

//         variable_string = variable_to_string(let_stmt->variable, model_param_map);
//         instance_append_vertex(instance, vertex, variable_string);

//         variable = symbol_table_lookup_symbol(node_symbol_table(let_stmt), variable_string);
//         variable_vertex_map_put(variable_vertex_map, variable, vertex);
//     }
// }

// static void execute_for_stmt(struct ForStmtNode* for_stmt, struct model_param_map_t* model_param_map,
//                              struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
// {
//     /* "for" name "=" start_expr "to" end_expr "{" stmts "}" */
//     float start_value;
//     float end_value;
//     float value;

//     start_value = evaluate_expr(for_stmt->start_expr, model_param_map);
//     end_value = evaluate_expr(for_stmt->end_expr, model_param_map);

//     for (value = start_value; value <= end_value; value++) {
//         model_param_map_put(model_param_map, for_stmt->name, value);
//         execute_stmts(for_stmt->stmts, model_param_map, variable_vertex_map, instance);
//     }
// }

// static void execute_stmts(struct StmtsNode* stmts, struct model_param_map_t* model_param_map,
//                           struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
// {
//     struct StmtNode* stmt;

//     while (stmts) {
//         stmt = stmts->stmt;

//         if (stmt->type == DRAW_STMT)
//             execute_draw_stmt((struct DrawStmtNode*)stmt, model_param_map, variable_vertex_map, instance);
//         else if (stmt->type == LET_STMT)
//             execute_let_stmt((struct LetStmtNode*)stmt, model_param_map, variable_vertex_map, instance);
//         else if (stmt->type == FOR_STMT)
//             execute_for_stmt((struct ForStmtNode*)stmt, model_param_map, variable_vertex_map, instance);
//         else
//             fprintf(stderr, "instantiate_model (%d): unrecognized stmt type %d\n", __LINE__, stmt->type);

//         stmts = stmts->stmts;
//     }
// }

// static void instantiate_model(struct ModelNode* model, float* model_params,
//                               struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
// {
//     struct model_param_map_t model_param_map = {0};
//     struct ModelParamsNode* model_params_node;
//     struct BNVertex* vertex;

//     model_params_node = model->params;
//     while (model_params_node && model_params) {
//         model_param_map_put(&model_param_map, model_params_node->name, *model_params);

//         vertex = malloc(sizeof(struct BNVertex));
//         vertex->type = BNV_CONST;
//         vertex->sample = *model_params;
//         instance_append_vertex(instance, vertex, symbol_to_string(node_symbol_table(model), model_params_node->name));
//         variable_vertex_map_put(variable_vertex_map, model_params_node->name, vertex);

//         model_params_node = model_params_node->model_params;
//         model_params++;
//     }
//     if (model_params_node || model_params) {
//         fprintf(stderr, "instantiate_model (%d): model params number mismatch\n", __LINE__);
//     }

//     execute_stmts(model->stmts, &model_param_map, variable_vertex_map, instance);
// }

// void init_instance(struct ModelNode* model, float* model_params, struct pp_instance_t* instance)
// {
//     struct variable_vertex_map_t variable_vertex_map = {0};
//     instance->n = 0;
//     instance->vertices = new_list();
//     instance->vertex_names = new_list();
//     instantiate_model(model, model_params, &variable_vertex_map, instance);
// }
