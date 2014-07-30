#include "../ppp.h"
#include "../defs.h"
#include "parse.h"
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

/******************************************************************************
Pointer list
******************************************************************************/

struct list_entry_t
{
    void* data;
    struct list_entry_t* next;
};

struct list_t
{
    struct list_entry_t* entry;
};

static struct list_t* new_list()
{
    struct list_t* list;
    list = malloc(sizeof(struct list_t));
    list->entry = 0;
    return list;
}

static void list_entry_append(struct list_entry_t* list_entry, void* data)
{
    assert(list_entry);
    if (list_entry->next)
        list_entry_append(list_entry->next, data);
    else {
        list_entry->next = malloc(sizeof(struct list_entry_t));
        list_entry->next->data = data;
        list_entry->next->next = 0;
    }
}

static void list_append(struct list_t* list, void* data)
{
    assert(list);
    if (list->entry)
        list_entry_append(list->entry, data);
    else {
        list->entry = malloc(sizeof(struct list_entry_t));
        list->entry->data = data;
        list->entry->next = 0;
    }
}

/******************************************************************************
Integer list
******************************************************************************/

struct ilist_entry_t
{
    int data;
    struct ilist_entry_t* next;
};

struct ilist_t
{
    struct ilist_entry_t* entry;
};

static struct ilist_t* new_ilist()
{
    struct ilist_t* list;
    list = malloc(sizeof(struct ilist_t));
    list->entry = 0;
    return list;
}

static void ilist_entry_append(struct ilist_entry_t* list_entry, int data)
{
    assert(list_entry);
    if (list_entry->next)
        ilist_entry_append(list_entry->next, data);
    else {
        list_entry->next = malloc(sizeof(struct ilist_entry_t));
        list_entry->next->data = data;
        list_entry->next->next = 0;
    }
}

static void ilist_append(struct ilist_t* list, int data)
{
    assert(list);
    if (list->entry)
        ilist_entry_append(list->entry, data);
    else {
        list->entry = malloc(sizeof(struct ilist_entry_t));
        list->entry->data = data;
        list->entry->next = 0;
    }
}

/******************************************************************************
Forward declarations
******************************************************************************/

/* FIXME should allow arbitrary length */
#define MAX_NAME_SIZE 250
#define MAX_INTEGER_VALUE_SIZE 10

typedef unsigned int symbol_t;

struct symbol_table_entry_t
{
    char string[MAX_NAME_SIZE];
    symbol_t symbol;
    struct symbol_table_entry_t* next;
};

struct symbol_table_t
{
    symbol_t last_symbol;
    struct symbol_table_entry_t* entry;
};

static struct symbol_table_t* new_symbol_table()
{
    struct symbol_table_t* symbol_table;

    symbol_table = malloc(sizeof(struct symbol_table_t));
    symbol_table->last_symbol = 0;
    symbol_table->entry = 0;

    return symbol_table;
}

static struct symbol_table_entry_t* symbol_table_find_entry_by_string(struct symbol_table_t* symbol_table, const char* string)
{
    struct symbol_table_entry_t* table_entry;

    table_entry = symbol_table->entry;
    while (table_entry) {
        if (strcmp(table_entry->string, string) == 0) {
            return table_entry;
        }
        table_entry = table_entry->next;
    }
    return 0;
}

static symbol_t symbol_table_lookup_symbol(struct symbol_table_t* symbol_table, const char* string)
{
    struct symbol_table_entry_t* table_entry;

    table_entry = symbol_table_find_entry_by_string(symbol_table, string);
    if (table_entry)
        return table_entry->symbol;

    /* entry not found, create a new entry */
    table_entry = malloc(sizeof(struct symbol_table_entry_t));
    strcpy(table_entry->string, string);
    table_entry->symbol = symbol_table->last_symbol + 1;
    table_entry->next = symbol_table->entry;

    symbol_table->last_symbol = table_entry->symbol;
    symbol_table->entry = table_entry;

    fprintf(stderr, "symbol_table_lookup_symbol (%d): create a new entry (%s, %u)\n", __LINE__, string, table_entry->symbol);
    return table_entry->symbol;
}

static struct symbol_table_entry_t* symbol_table_find_entry_by_symbol(struct symbol_table_t* symbol_table, symbol_t symbol)
{
    struct symbol_table_entry_t* table_entry;

    table_entry = symbol_table->entry;
    while (table_entry) {
        if (table_entry->symbol == symbol) {
            return table_entry;
        }
        table_entry = table_entry->next;
    }
    return 0;
}

struct ParserState;
struct ModelNode;
struct ModelParamsNode;
struct DeclsNode;
struct StmtsNode;
struct DeclNode;
struct PublicDeclNode;
struct PrivateDeclNode;
struct StmtNode;
struct DrawStmtNode;
struct LetStmtNode;
struct ForStmtNode;
struct ExprSeqNode;
struct ExprNode;
struct IfExprNode;
struct NewExprNode;
struct BinaryExprNode;
struct PrimaryNode;
struct VariableNode;
struct NameVarNode;
struct FieldVarNode;
struct IndexVarNode;
struct NumericalValueNode;
struct RealValueNode;
struct IntegerValueNode;

static struct ModelsNode* parse_models(struct ParserState* ps);
static struct ModelNode* parse_model(struct ParserState* ps);
static struct ModelParamsNode* parse_model_params(struct ParserState* ps);
static struct DeclsNode* parse_decls(struct ParserState* ps);
static struct StmtsNode* parse_stmts(struct ParserState* ps);
static struct DeclNode* parse_decl(struct ParserState* ps);
static struct PublicDeclNode* parse_public_decl(struct ParserState* ps);
static struct PrivateDeclNode* parse_private_decl(struct ParserState* ps);
static struct StmtNode* parse_stmt(struct ParserState* ps);
static struct ForStmtNode* parse_for_stmt(struct ParserState* ps);
static struct ExprSeqNode* parse_expr_seq(struct ParserState* ps);
static struct ExprNode* parse_expr(struct ParserState* ps);
static struct ExprNode* parse_add_expr(struct ParserState* ps);
static struct ExprNode* parse_term(struct ParserState* ps);
static struct PrimaryNode* parse_primary(struct ParserState* ps);
static struct VariableNode* parse_variable(struct ParserState* ps);
static struct VariableNode* parse_variable1(struct ParserState* ps, symbol_t name);
static struct NumericalValueNode* parse_numerical_value(struct ParserState* ps);
static struct IntegerValueNode* parse_integer_value(struct ParserState* ps);
static symbol_t parse_name(struct ParserState* ps);

static struct ModelsNode* parse_file1(const char* filename, struct symbol_table_t* symbol_table);

static int dump_model(struct ModelNode* model);
static int dump_model_params(struct ModelParamsNode* model_params);
static int dump_decls(struct DeclsNode* decls);
static int dump_stmts(struct StmtsNode* stmts);
static int dump_decl(struct DeclNode* decl);
static int dump_public_decl(struct PublicDeclNode* public_decl);
static int dump_private_decl(struct PrivateDeclNode* private_decl);
static int dump_stmt(struct StmtNode* stmt);
static int dump_draw_stmt(struct DrawStmtNode* draw_stmt);
static int dump_let_stmt(struct LetStmtNode* let_stmt);
static int dump_for_stmt(struct ForStmtNode* for_stmt);
static int dump_expr_seq(struct ExprSeqNode* expr_seq);
static int dump_expr(struct ExprNode* expr);
static int dump_if_expr(struct IfExprNode* if_expr);
static int dump_new_expr(struct NewExprNode* new_expr);
static int dump_binary_expr(struct BinaryExprNode* expr);
static int dump_primary(struct PrimaryNode* primary);
static int dump_variable(struct VariableNode* variable);
static int dump_name_var(struct NameVarNode* name_var);
static int dump_field_var(struct FieldVarNode* field_var);
static int dump_index_var(struct IndexVarNode* index_var);
static int dump_numerical_value(struct NumericalValueNode* numerical_value);
static int dump_real_value(struct RealValueNode* real_value);
static int dump_integer_value(struct IntegerValueNode* integer_value);
static int dump_name(symbol_t name, struct symbol_table_t* symbol_table);

struct ParserState
{
    struct symbol_table_t* symbol_table;
    int pointer;
    int program_length;
    const char* program;
};

struct CommonNode
{
    struct symbol_table_t* symbol_table;
};

static struct symbol_table_t* node_symbol_table(void* any)
{
    return ((struct CommonNode*)any)->symbol_table;
}

static void* new_node(size_t size, struct ParserState* ps)
{
    struct CommonNode* node;
    node = malloc(size);
    node->symbol_table = ps->symbol_table;
    return node;
}

struct ModelsNode /* extends CommonNode */
{
    struct CommonNode super;
    struct ModelNode* model;
    struct ModelsNode* models;
};

struct ModelNode /* extends CommonNode */
{
    struct CommonNode super;
    symbol_t name;
    struct ModelParamsNode* params;
    struct DeclsNode* decls;
    struct StmtsNode* stmts;
};

/******************************************************************************
Module interfaces
******************************************************************************/

struct model_map_t
{
    symbol_t model_name;
    struct ModelNode* model;
    struct model_map_t* next;
};

struct pp_state_t
{
    struct symbol_table_t* symbol_table;
    struct model_map_t* model_map;
};

struct pp_instance_t
{
    int n;
    struct list_t* vertices;
    struct list_t* vertex_names;
};

/**
Get total number of vertices.
*/
int pp_instance_num_vertices(struct pp_instance_t* instance)
{
    return instance->n;
}

/**
Get the i-th vertex.
*/
struct BNVertex* pp_instance_vertex(struct pp_instance_t* instance, int i)
{
    struct list_entry_t* vertex;
    vertex = instance->vertices->entry;
    while (i--)
        vertex = vertex->next;
    return vertex->data;
}

/**
Get the name of the i-th vertex.
*/
const char* pp_instance_vertex_name(struct pp_instance_t* instance, int i)
{
    struct list_entry_t* vertex_name;
    vertex_name = instance->vertex_names->entry;
    while (i--)
        vertex_name = vertex_name->next;
    return vertex_name->data;
}

/**
 *	Get the number of the vertex.
 */
int pp_instance_find_num_of_vertex(struct pp_instance_t* instance, struct BNVertex* vertex) {
	struct list_entry_t* entry = instance->vertices->entry;
	int num = 0;

	for ( ; entry && entry->data != vertex; entry = entry->next, ++num ) ;

	return (entry) ? num : -1;
}

static int add_models(struct pp_state_t* state, struct ModelsNode* models)
{
    struct model_map_t* model_map;

    if (!models) return 0;

    model_map = malloc(sizeof(struct model_map_t));
    model_map->model_name = models->model->name;
    model_map->model = models->model;
    model_map->next = state->model_map;

    state->model_map = model_map;

    return add_models(state, models->models);
}

static struct ModelNode* model_map_find(struct model_map_t* model_map,
                                        struct symbol_table_t* symbol_table, const char* model_name)
{
    struct symbol_table_entry_t* table_entry;

    table_entry = symbol_table_find_entry_by_string(symbol_table, model_name);
    if (!table_entry) return 0;

    while (model_map)
        if (model_map->model_name == table_entry->symbol)
            return model_map->model;
        else
            model_map = model_map->next;

    return 0;
}

struct pp_state_t* pp_new_state()
{
    struct pp_state_t* state;

    srand(time(NULL));
    state = malloc(sizeof(struct pp_state_t));
    state->symbol_table = new_symbol_table();
    state->model_map = 0;

    return state;
}

int pp_free(struct pp_state_t* state)
{
    /* FIXME free associated memory here */
    free(state);
    return 0;
}

int pp_load_file(struct pp_state_t* state, const char* filename)
{
    struct ModelsNode* models = parse_file1(filename, state->symbol_table);
    return add_models(state, models);
}

/* forward declaration */
static void init_instance(struct ModelNode* model, float* model_params, struct pp_instance_t* instance);

struct pp_instance_t* pp_new_instance(struct pp_state_t* state, const char* model_name, float* model_params)
{
    struct ModelNode* model;
    struct pp_instance_t* instance;

    model = model_map_find(state->model_map, state->symbol_table, model_name);
    if (!model) return 0;

    instance = malloc(sizeof(struct pp_instance_t));
    init_instance(model, model_params, instance);

    return instance;
}

float pp_name_to_value(struct pp_instance_t* instance, const char* name)
{
    struct list_entry_t* vertex;
    struct list_entry_t* vertex_name;

    vertex = instance->vertices->entry;
    vertex_name = instance->vertex_names->entry;
    while (vertex && vertex_name)
        if (strcmp((const char*)(vertex_name->data), name) == 0)
            return ((struct BNVertex*)(vertex->data))->sample;
        else
            vertex = vertex->next, vertex_name = vertex_name->next;

    fprintf(stderr, "pp_name_to_value: vertex %s missing\n", name);
    return 0;
}

/******************************************************************************
Below is the parser
******************************************************************************/

/* #define debug(fmt, arg) fprintf(stderr, fmt, arg) */
#define debug(fmt, arg) do{}while(0)

static int peek1(struct ParserState* ps, int offset)
{
    if (ps->pointer + offset >= ps->program_length) return -1;
    return ps->program[ps->pointer + offset];
}

static int peek(struct ParserState* ps)
{
    return peek1(ps, 0);
}

static int isnewline(int c)
{
    return c == '\n' || c == '\r';
}

static void skip_spaces(struct ParserState* ps)
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

static int accept(struct ParserState* ps, const char* token)
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
    return 1;
}

static void* failure(const char* message)
{
    debug("%s\n", message);
    return 0;
}

const char* symbol_to_string(struct symbol_table_t* symbol_table, symbol_t symbol)
{
    struct symbol_table_entry_t* table_entry;
    table_entry = symbol_table_find_entry_by_symbol(symbol_table, symbol);
    if (table_entry)
        return table_entry->string;
    else
        return 0;
}

int dump_name(symbol_t name, struct symbol_table_t* symbol_table)
{
    return printf("%s", symbol_to_string(symbol_table, name));
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
    if (!is_name_first_character(peek(ps))) return 0;

    length = 0;
    while (is_name_character(peek(ps))) {
        characters[length++] = peek(ps);
        ps->pointer++;
    }
    characters[length] = '\0';

    return symbol_table_lookup_symbol(ps->symbol_table, characters);
}

int dump_models(struct ModelsNode* models)
{
    dump_model(models->model);
    if (models->models)
        return dump_models(models->models);
    return 0;
}

struct ModelsNode* parse_models(struct ParserState* ps)
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

int dump_model(struct ModelNode* model)
{
    printf("model ");
    dump_name(model->name, node_symbol_table(model));
    printf("(");
    if (model->params) {
      dump_model_params(model->params);
    }
    printf(") {\n");
    dump_decls(model->decls);
    dump_stmts(model->stmts);
    return printf("}\n");
}

struct ModelNode* parse_model(struct ParserState* ps)
{
    struct ModelNode* model;
    symbol_t name;
    struct ModelParamsNode* params;
    struct DeclsNode* decls;
    struct StmtsNode* stmts;

    if (!accept(ps, "model")) return failure("parse_model: model expected");
    name = parse_name(ps);
    if (!name) return failure("parse_model: name expected");
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
    decls = parse_decls(ps);
    if (!decls) return failure("parse_model: decls expected");
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

struct ModelParamsNode /* extends CommonNode */
{
    struct CommonNode super;
    symbol_t name;
    struct ModelParamsNode* model_params;
};

int dump_model_params(struct ModelParamsNode* model_params)
{
    dump_name(model_params->name, node_symbol_table(model_params));
    if (model_params->model_params) {
        printf(", ");
        dump_model_params(model_params->model_params);
    }
  return 0;
}

struct ModelParamsNode* parse_model_params(struct ParserState* ps)
{
    symbol_t name;
    struct ModelParamsNode* model_params1;
    struct ModelParamsNode* model_params;

    name = parse_name(ps);
    if (!name) return failure("parse_model_params: name expected");
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

struct DeclsNode /* extends CommonNode */
{
    struct CommonNode super;
    struct DeclNode* decl;
    struct DeclsNode* decls;
};

int dump_decls(struct DeclsNode* decls)
{
    dump_decl(decls->decl);
    if (decls->decls)
        return dump_decls(decls->decls);
    return 0;
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

struct StmtsNode /* extends CommonNode */
{
    struct CommonNode super;
    struct StmtNode* stmt;
    struct StmtsNode* stmts;
};

int dump_stmts(struct StmtsNode* stmts)
{
    dump_stmt(stmts->stmt);
    if (stmts->stmts)
        return dump_stmts(stmts->stmts);
    return 0;
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

struct DeclNode /* extends CommonNode */
{
    struct CommonNode super;
    enum { PUBLIC_DECL, PRIVATE_DECL } type;
};

int dump_decl(struct DeclNode* decl)
{
    if (decl->type == PUBLIC_DECL)
        return dump_public_decl((struct PublicDeclNode*)decl);
    else
        return dump_private_decl((struct PrivateDeclNode*)decl);
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

struct PublicDeclNode /* extends DeclNode */
{
    struct DeclNode super;
    struct VariableNode* variable;
};

int dump_public_decl(struct PublicDeclNode* public_decl)
{
    printf("public ");
    dump_variable(public_decl->variable);
    return printf("\n");
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

struct PrivateDeclNode /* extends DeclNode */
{
    struct DeclNode super;
    struct VariableNode* variable;
};

int dump_private_decl(struct PrivateDeclNode* private_decl)
{
    printf("private ");
    dump_variable(private_decl->variable);
    return printf("\n");
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

struct StmtNode /* extends CommonNode */
{
    struct CommonNode super;
    enum { DRAW_STMT, LET_STMT, FOR_STMT } type;
};

int dump_stmt(struct StmtNode* stmt)
{
    if (stmt->type == DRAW_STMT)
        return dump_draw_stmt((struct DrawStmtNode*)stmt);
    else if (stmt->type == LET_STMT)
        return dump_let_stmt((struct LetStmtNode*)stmt);
    else if (stmt->type == FOR_STMT)
        return dump_for_stmt((struct ForStmtNode*)stmt);
    fprintf(stderr, "unrecognized stmt type\n");
    return 0;
}

struct DrawStmtNode /* extends StmtNode */
{
    struct StmtNode super;
    struct VariableNode* variable;
    symbol_t dist;
    struct ExprSeqNode* expr_seq;
};

int dump_draw_stmt(struct DrawStmtNode* draw_stmt)
{
    dump_variable(draw_stmt->variable);
    printf(" ~ ");
    dump_name(draw_stmt->dist, node_symbol_table(draw_stmt));
    printf("(");
    dump_expr_seq(draw_stmt->expr_seq);
    return printf(")\n");
}

struct LetStmtNode /* extends StmtNode */
{
    struct StmtNode super;
    struct VariableNode* variable;
    struct ExprNode* expr;
};

int dump_let_stmt(struct LetStmtNode* let_stmt)
{
    dump_variable(let_stmt->variable);
    printf(" = ");
    dump_expr(let_stmt->expr);
    return printf("\n");
}

struct StmtNode* parse_stmt(struct ParserState* ps)
{
    struct ForStmtNode* for_stmt;
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

    variable = parse_variable(ps);
    if (!variable) return failure("parse_stmt: variable expected");

    if (accept(ps, "~")) {
        dist = parse_name(ps);
        if (!dist) return failure("parse_stmt: dist expected");
        if (!accept(ps, "(")) return failure("parse_stmt: ( expected");
        expr_seq = parse_expr_seq(ps);
        if (!expr_seq) return failure("parse_stmt: expr_seq expected");
        if (!accept(ps, ")")) return failure("parse_stmt: ) expected");
        debug("%s\n", "parse_stmt: draw_stmt recognized");

        draw_stmt = new_node(sizeof(struct DrawStmtNode), ps);
        draw_stmt->super.type = DRAW_STMT;
        draw_stmt->variable = variable;
        draw_stmt->dist = dist;
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

struct ForStmtNode /* extends StmtNode */
{
    struct StmtNode super;
    symbol_t name;
    struct ExprNode* start_expr;
    struct ExprNode* end_expr;
    struct StmtsNode* stmts;
};

int dump_for_stmt(struct ForStmtNode* for_stmt)
{
    printf("for ");
    dump_name(for_stmt->name, node_symbol_table(for_stmt));
    printf(" = ");
    dump_expr(for_stmt->start_expr);
    printf(" to ");
    dump_expr(for_stmt->end_expr);
    printf(" {\n");
    dump_stmts(for_stmt->stmts);
    return printf("}\n");
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
    if (!name) return failure("parse_for_stmt: name expected");
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

struct ExprSeqNode /* extends CommonNode */
{
    struct CommonNode super;
    struct ExprNode* expr;
    struct ExprSeqNode* expr_seq;
};

size_t expr_seq_length(struct ExprSeqNode* expr_seq)
{
    if (expr_seq)
        return 1 + expr_seq_length(expr_seq->expr_seq);
    else
        return 0;
}

int dump_expr_seq(struct ExprSeqNode* expr_seq)
{
    dump_expr(expr_seq->expr);
    if (expr_seq->expr_seq) {
        printf(", ");
        return dump_expr_seq(expr_seq->expr_seq);
    }
    return 0;
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

struct ExprNode /* extends CommonNode */
{
    struct CommonNode super;
    enum { IF_EXPR, NEW_EXPR, BINARY_EXPR, PRIMARY_EXPR } type;
};

int dump_expr(struct ExprNode* expr)
{
    if (expr->type == IF_EXPR)
        return dump_if_expr((struct IfExprNode*)expr);
    if (expr->type == NEW_EXPR)
        return dump_new_expr((struct NewExprNode*)expr);
    if (expr->type == BINARY_EXPR)
        return dump_binary_expr((struct BinaryExprNode*)expr);
    if (expr->type == PRIMARY_EXPR)
        return dump_primary((struct PrimaryNode*)expr);
    fprintf(stderr, "unrecognized expr type\n");
    return 0;
}

struct IfExprNode /* extends ExprNode */
{
    struct ExprNode super;
    struct ExprNode* condition;
    struct ExprNode* consequent;
    struct ExprNode* alternative;
};

int dump_if_expr(struct IfExprNode* if_expr)
{
    printf("if ");
    dump_expr(if_expr->condition);
    printf(" then ");
    dump_expr(if_expr->consequent);
    printf(" else ");
    return dump_expr(if_expr->alternative);
}

struct NewExprNode /* extends ExprNode */
{
    struct ExprNode super;
    symbol_t name;
};

int dump_new_expr(struct NewExprNode* new_expr)
{
    printf("new ");
    dump_name(new_expr->name, node_symbol_table(new_expr));
    return printf("()");
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
        if (!name) return failure("parse_expr: name expected");
        if (!accept(ps, "(")) return failure("parse_expr: ( expected");
        if (!accept(ps, ")")) return failure("parse_expr: ) expected");

        new_expr = new_node(sizeof(struct NewExprNode), ps);
        new_expr->super.type = NEW_EXPR;
        new_expr->name = name;

        return (struct ExprNode*)new_expr;
    }

    return parse_add_expr(ps);
}

struct BinaryExprNode /* extends ExprNode */
{
    struct ExprNode super;
    enum { OP_ADD, OP_SUB, OP_MUL, OP_DIV } op;
    struct ExprNode* left;
    struct ExprNode* right;
};

int dump_binary_expr(struct BinaryExprNode* expr)
{
    dump_expr(expr->left);
    if (expr->op == OP_ADD)
        printf(" + ");
    else if (expr->op == OP_SUB)
        printf(" - ");
    else if (expr->op == OP_MUL)
        printf(" * ");
    else if (expr->op == OP_DIV)
        printf(" / ");
    else
        printf(" ? ");
    return dump_expr(expr->right);
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
    struct PrimaryNode* primary;
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

struct PrimaryNode /* extends ExprNode */
{
    struct ExprNode super;
    enum {
        NUM_EXPR,
        VAR_EXPR,
        UNARY_EXPR,
        GROUP_EXPR,
        FUNC_EXPR
    } type;
};

struct NumExprNode /* extends PrimaryNode */
{
    struct PrimaryNode super;
    struct NumericalValueNode* numerical_value;
};

struct VarExprNode /* extends PrimaryNode */
{
    struct PrimaryNode super;
    struct VariableNode* variable;
};

struct UnaryExprNode /* extends PrimaryNode */
{
    struct PrimaryNode super;
    enum { OP_NEG } op;
    struct PrimaryNode* primary;
};

struct GroupExprNode /* extends PrimaryNode */
{
    struct PrimaryNode super;
    struct ExprNode* expr;
};

struct FuncExprNode /* extends PrimrayNode */
{
    struct PrimaryNode super;
    symbol_t name;
    struct ExprSeqNode* expr_seq;
};

static int dump_unary_expr(struct UnaryExprNode* unary)
{
    if (unary->op == OP_NEG)
        printf("-");
    return dump_primary(unary->primary);
}

int dump_primary(struct PrimaryNode* primary)
{
    if (primary->type == NUM_EXPR)
        return dump_numerical_value(((struct NumExprNode*)primary)->numerical_value);

    if (primary->type == VAR_EXPR)
        return dump_variable(((struct VarExprNode*)primary)->variable);

    if (primary->type == UNARY_EXPR)
        return dump_unary_expr((struct UnaryExprNode*)primary);

    if (primary->type == GROUP_EXPR) {
        printf("(");
        dump_expr(((struct GroupExprNode*)primary)->expr);
        return printf(")");
    }

    if (primary->type == FUNC_EXPR) {
        dump_name(((struct FuncExprNode*)primary)->name, node_symbol_table(primary));
        printf("(");
        dump_expr_seq(((struct FuncExprNode*)primary)->expr_seq);
        return printf(")");
    }

    fprintf(stderr, "unrecognized primary type\n");
    return 0;
}

struct PrimaryNode* parse_primary(struct ParserState* ps)
{
    struct PrimaryNode* primary;
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

        return (struct PrimaryNode*)unary;
    }

    if (accept(ps, "(")) {
        expr = parse_expr(ps);
        if (!expr) return failure("parse_primary: expr expected");
        if (!accept(ps, ")")) return failure("parse_primary: ) expected");

        group = new_node(sizeof(struct GroupExprNode), ps);
        group->super.super.type = PRIMARY_EXPR;
        group->super.type = GROUP_EXPR;
        group->expr = expr;

        return (struct PrimaryNode*)group;
    }

    numerical_value = parse_numerical_value(ps);
    if (numerical_value) {
        num_expr = new_node(sizeof(struct NumExprNode), ps);
        num_expr->super.super.type = PRIMARY_EXPR;
        num_expr->super.type = NUM_EXPR;
        num_expr->numerical_value = numerical_value;

        return (struct PrimaryNode*)num_expr;
    }

    name = parse_name(ps);
    if (!name) return failure("parse_primary: name expected");

    if (accept(ps, "(")) {
        expr_seq = parse_expr_seq(ps);
        if (!expr_seq) return failure("parse_primary: expr_seq expected");
        if (!accept(ps, ")")) return failure("parse_primary: ) expected");

        func_expr = new_node(sizeof(struct FuncExprNode), ps);
        func_expr->super.super.type = PRIMARY_EXPR;
        func_expr->super.type = FUNC_EXPR;
        func_expr->name = name;
        func_expr->expr_seq = expr_seq;

        return (struct PrimaryNode*)func_expr;
    }

    variable = parse_variable1(ps, name);
    if (variable) {
        var_expr = new_node(sizeof(struct VarExprNode), ps);
        var_expr->super.super.type = PRIMARY_EXPR;
        var_expr->super.type = VAR_EXPR;
        var_expr->variable = variable;

        return (struct PrimaryNode*)var_expr;
    }

    return 0;
}

struct VariableNode /* extends CommonNode */
{
    struct CommonNode super;
    enum { NAME_VAR, FIELD_VAR, INDEX_VAR } type;
};

int dump_variable(struct VariableNode* variable)
{
    if (variable->type == NAME_VAR)
        return dump_name_var((struct NameVarNode*)variable);
    if (variable->type == FIELD_VAR)
        return dump_field_var((struct FieldVarNode*)variable);
    if (variable->type == INDEX_VAR)
        return dump_index_var((struct IndexVarNode*)variable);
    fprintf(stderr, "unrecognized variable type\n");
    return 0;
}

struct NameVarNode /* extends VariableNode */
{
    struct VariableNode super;
    symbol_t name;
};

int dump_name_var(struct NameVarNode* name_var)
{
    return dump_name(name_var->name, node_symbol_table(name_var));
}

struct FieldVarNode /* extends VariableNode */
{
    struct VariableNode super;
    symbol_t name;
    symbol_t field_name;
};

int dump_field_var(struct FieldVarNode* field_var)
{
    dump_name(field_var->name, node_symbol_table(field_var));
    printf(".");
    return dump_name(field_var->field_name, node_symbol_table(field_var));
}

struct IndexVarNode /* extends VariableNode */
{
    struct VariableNode super;
    symbol_t name;
    struct ExprSeqNode* expr_seq;
};

int dump_index_var(struct IndexVarNode* index_var)
{
    dump_name(index_var->name, node_symbol_table(index_var));
    printf("[");
    dump_expr_seq(index_var->expr_seq);
    return printf("]");
}

struct VariableNode* parse_variable(struct ParserState* ps)
{
    symbol_t name;

    name = parse_name(ps);
    if (!name) return failure("parse_variable: name expected");
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
        if (!field_name) return failure("parse_variable1: field_name expected");

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

struct NumericalValueNode /* extends CommonNode */
{
    struct CommonNode super;
    enum { REAL_VALUE, INTEGER_VALUE } type;
};

int dump_numerical_value(struct NumericalValueNode* numerical_value)
{
    if (numerical_value->type == REAL_VALUE)
        return dump_real_value((struct RealValueNode*)numerical_value);
    if (numerical_value->type == INTEGER_VALUE)
        return dump_integer_value((struct IntegerValueNode*)numerical_value);
    fprintf(stderr, "unrecognized numerical value type\n");
    return 0;
}

struct RealValueNode /* extends NumericalValueNode */
{
    struct NumericalValueNode super;
    struct IntegerValueNode* integer_part;
    struct IntegerValueNode* fractional;
};

int dump_real_value(struct RealValueNode* real_value)
{
    dump_integer_value(real_value->integer_part);
    printf(".");
    return dump_integer_value(real_value->fractional);
}

struct NumericalValueNode* parse_numerical_value(struct ParserState* ps)
{
    struct IntegerValueNode* integer_value;
    struct IntegerValueNode* fractional;
    struct RealValueNode* real_value;

    integer_value = parse_integer_value(ps);
    if (!integer_value) return failure("parse_numerical_value: integer_value expected");

    if (accept(ps, ".")) {
        fractional = parse_integer_value(ps);
        if (!fractional) return failure("parse_numerical_value: fractional expected");

        real_value = new_node(sizeof(struct RealValueNode), ps);
        real_value->super.type = REAL_VALUE;
        real_value->integer_part = integer_value;
        real_value->fractional = fractional;

        return (struct NumericalValueNode*)real_value;
    }

    return (struct NumericalValueNode*)integer_value;
}

struct IntegerValueNode /* extends NumericalValueNode */
{
    struct NumericalValueNode super;
    int length;
    char digits[MAX_INTEGER_VALUE_SIZE];
};

int dump_integer_value(struct IntegerValueNode* integer_value)
{
    return printf("%s", integer_value->digits);
}

struct IntegerValueNode* parse_integer_value(struct ParserState* ps)
{
    struct IntegerValueNode* integer_value;

    skip_spaces(ps);

    if (!isdigit(peek(ps))) return failure("parse_integer_value: digit expected");

    integer_value = new_node(sizeof(struct IntegerValueNode), ps);
    integer_value->super.type = INTEGER_VALUE;
    integer_value->length = 0;

    while (isdigit(peek(ps))) {
        integer_value->digits[integer_value->length++] = peek(ps);
        ps->pointer++;
    }
    integer_value->digits[integer_value->length] = '\0';

    return integer_value;
}

static struct ModelsNode* parse_file1(const char* filename, struct symbol_table_t* symbol_table)
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

struct model_param_map_entry_t
{
    symbol_t param_name;
    float value;
    struct model_param_map_entry_t* next;
};

struct model_param_map_t
{
    struct model_param_map_entry_t* entry;
};

static struct model_param_map_entry_t* model_param_map_find_entry(struct model_param_map_t* map, symbol_t symbol)
{
    struct model_param_map_entry_t* table_entry;

    table_entry = map->entry;
    while (table_entry)
        if (table_entry->param_name == symbol)
            return table_entry;
        else
            table_entry = table_entry->next;

    return 0;
}

static void model_param_map_put(struct model_param_map_t* model_param_map, symbol_t param_name, float value)
{
    struct model_param_map_entry_t* table_entry;

    table_entry = model_param_map_find_entry(model_param_map, param_name);
    if (table_entry) {
        table_entry->value = value;
    } else {
        table_entry = malloc(sizeof(struct model_param_map_entry_t));
        table_entry->param_name = param_name;
        table_entry->value = value;
        table_entry->next = model_param_map->entry;
        model_param_map->entry = table_entry;
    }
}

static float model_param_map_get(struct model_param_map_t* model_param_map, symbol_t param_name)
{
    struct model_param_map_entry_t* table_entry;

    table_entry = model_param_map_find_entry(model_param_map, param_name);
    if (table_entry)
        return table_entry->value;
    else
        return 0;
}

struct variable_vertex_map_entry_t
{
    symbol_t variable;
    struct BNVertex* vertex;
    struct variable_vertex_map_entry_t* next;
};

struct variable_vertex_map_t
{
    struct variable_vertex_map_entry_t* entry;
};

static struct variable_vertex_map_entry_t* variable_vertex_map_find_entry(struct variable_vertex_map_t* map, symbol_t variable)
{
    struct variable_vertex_map_entry_t* table_entry;

    table_entry = map->entry;
    while (table_entry)
        if (table_entry->variable == variable)
            return table_entry;
        else
            table_entry = table_entry->next;

    return 0;
}

static void variable_vertex_map_put(struct variable_vertex_map_t* map, symbol_t variable, struct BNVertex* vertex)
{
    struct variable_vertex_map_entry_t* table_entry;

    table_entry = variable_vertex_map_find_entry(map, variable);
    if (table_entry) {
        table_entry->vertex = vertex;
    } else {
        table_entry = malloc(sizeof(struct variable_vertex_map_entry_t));
        table_entry->variable = variable;
        table_entry->vertex = vertex;
        table_entry->next = map->entry;
        map->entry = table_entry;
    }
}

static struct BNVertex* variable_vertex_map_get(struct variable_vertex_map_t* map, symbol_t variable)
{
    struct variable_vertex_map_entry_t* table_entry;

    table_entry = variable_vertex_map_find_entry(map, variable);
    if (table_entry)
        return table_entry->vertex;
    else
        return 0;
}

static void instance_append_vertex(struct pp_instance_t* instance, struct BNVertex* vertex, const char* vertex_name)
{
    list_append(instance->vertices, vertex);
    if (vertex_name)
        list_append(instance->vertex_names, (void*)vertex_name);
    else
        /* FIXME should generate unique special names for intermediate results */
        list_append(instance->vertex_names, "");
    instance->n++;
}

/******************************************************************************
Compute expressions
******************************************************************************/

static struct BNVertex* compute_expr(struct ExprNode* expr, struct model_param_map_t* model_param_map,
                                     struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance);

static struct BNVertex* compute_if_expr(struct IfExprNode* expr, struct model_param_map_t* model_param_map,
                                        struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
{
    struct BNVertex* condition;
    struct BNVertex* consequent;
    struct BNVertex* alternative;
    struct BNVertexComputeIf* vertex;

    condition = compute_expr(expr->condition, model_param_map, variable_vertex_map, instance);
    consequent = compute_expr(expr->consequent, model_param_map, variable_vertex_map, instance);
    alternative = compute_expr(expr->alternative, model_param_map, variable_vertex_map, instance);

    /* FIXME generate special names for intermediate results */
    instance_append_vertex(instance, condition, 0);
    instance_append_vertex(instance, consequent, 0);
    instance_append_vertex(instance, alternative, 0);

    vertex = malloc(sizeof(struct BNVertexComputeIf));
    vertex->super.super.type = BNV_COMPU;
    vertex->super.type = BNVC_IF;
    vertex->condition = condition;
    vertex->consequent = consequent;
    vertex->alternative = alternative;
    return (struct BNVertex*)vertex;
}

static struct BNVertex* compute_binary_expr(struct BinaryExprNode* expr, struct model_param_map_t* model_param_map,
                                            struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
{
    struct BNVertex* left;
    struct BNVertex* right;
    struct BNVertexComputeBinop* vertex;

    left = compute_expr(expr->left, model_param_map, variable_vertex_map, instance);
    right = compute_expr(expr->right, model_param_map, variable_vertex_map, instance);

    instance_append_vertex(instance, left, 0);
    instance_append_vertex(instance, right, 0);

    vertex = malloc(sizeof(struct BNVertexComputeBinop));
    vertex->super.super.type = BNV_COMPU;
    vertex->super.type = BNVC_BINOP;
    vertex->left = left;
    vertex->right = right;

    if (expr->op == OP_ADD)
        vertex->binop = BINOP_PLUS;
    else if (expr->op == OP_SUB)
        vertex->binop = BINOP_SUB;
    else if (expr->op == OP_MUL)
        vertex->binop = BINOP_MULTI;
    else if (expr->op == OP_DIV)
        vertex->binop = BINOP_DIV;

    return (struct BNVertex*)vertex;
}

static float evaluate_numerical_value(struct NumericalValueNode* numerical_value);

static struct BNVertex* compute_numerical_value(struct NumericalValueNode* numerical_value)
{
    struct BNVertex* vertex;

    vertex = malloc(sizeof(struct BNVertex));
    vertex->type = BNV_CONST;
    vertex->sample = evaluate_numerical_value(numerical_value);
    return vertex;
}

static const char* variable_to_string(struct VariableNode* variable, struct model_param_map_t* model_param_map);

static struct BNVertex* compute_variable(struct VariableNode* variable, struct model_param_map_t* model_param_map,
                                         struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
{

    const char* variable_string;
	symbol_t symbol;

	struct BNVertex* variable_vertex;
	struct BNVertexComputeUnary* vertex;

    variable_string = variable_to_string(variable, model_param_map);
    symbol = symbol_table_lookup_symbol(node_symbol_table(variable), variable_string);
    variable_vertex = variable_vertex_map_get(variable_vertex_map, symbol);

	/* copy the value in case of multiple samples drawn from the same distribution */
	vertex = malloc(sizeof(struct BNVertexComputeUnary));
	vertex->super.super.type = BNV_COMPU;
	vertex->super.type = BNVC_UNARY;
	vertex->primary = variable_vertex;
	vertex->op = UNARY_POS;

	return (struct BNVertex*)vertex;
}

static struct BNVertex* compute_primary(struct PrimaryNode* primary, struct model_param_map_t* model_param_map,
                                        struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance);

static struct BNVertex* compute_unary_expr(struct UnaryExprNode* expr, struct model_param_map_t* model_param_map,
                                           struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
{
    struct BNVertex* primary;
    struct BNVertexComputeUnary* vertex;

    primary = compute_primary(expr->primary, model_param_map, variable_vertex_map, instance);

    instance_append_vertex(instance, primary, 0);

    vertex = malloc(sizeof(struct BNVertexComputeUnary));
    vertex->super.super.type = BNV_COMPU;
    vertex->super.type = BNVC_UNARY;
    vertex->primary = primary;

    if (expr->op == OP_NEG)
        vertex->op = UNARY_NEG;

    return (struct BNVertex*)vertex;
}

static struct BNVertex* compute_func_expr(struct FuncExprNode* expr, struct model_param_map_t* model_param_map,
                                          struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
{
    symbol_t sym_exp;
    symbol_t sym_log;
    struct BNVertex* value;
    struct BNVertexComputeFunc* vertex;

    sym_exp = symbol_table_lookup_symbol(node_symbol_table(expr), "exp");
    sym_log = symbol_table_lookup_symbol(node_symbol_table(expr), "log");

    if (expr->name == sym_exp) {
        assert(expr_seq_length(expr->expr_seq) == 1);

        value = compute_expr(expr->expr_seq->expr, model_param_map, variable_vertex_map, instance);
        instance_append_vertex(instance, value, 0);

        vertex = malloc(sizeof(struct BNVertexComputeFunc));
        vertex->super.super.type = BNV_COMPU;
        vertex->super.type = BNVC_FUNC;
        vertex->func = FUNC_EXP;
        vertex->args[0] = value;
        return (struct BNVertex*)vertex;
    }

    if (expr->name == sym_log) {
        assert(expr_seq_length(expr->expr_seq) == 1);

        value = compute_expr(expr->expr_seq->expr, model_param_map, variable_vertex_map, instance);
        instance_append_vertex(instance, value, 0);

        vertex = malloc(sizeof(struct BNVertexComputeFunc));
        vertex->super.super.type = BNV_COMPU;
        vertex->super.type = BNVC_FUNC;
        vertex->func = FUNC_LOG;
        vertex->args[0] = value;
        return (struct BNVertex*)vertex;
    }

    fprintf(stderr, "compute_func_expr (%d): unrecognized function %s\n", __LINE__, symbol_to_string(node_symbol_table(expr), expr->name));
    return 0;
}

static struct BNVertex* compute_primary(struct PrimaryNode* primary, struct model_param_map_t* model_param_map,
                                        struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
{
    if (primary->type == NUM_EXPR)
        return compute_numerical_value(((struct NumExprNode*)primary)->numerical_value);
    if (primary->type == VAR_EXPR)
        return compute_variable(((struct VarExprNode*)primary)->variable, model_param_map, variable_vertex_map, instance);
    if (primary->type == UNARY_EXPR)
        return compute_unary_expr((struct UnaryExprNode*)primary, model_param_map, variable_vertex_map, instance);
    if (primary->type == GROUP_EXPR)
        return compute_expr(((struct GroupExprNode*)primary)->expr, model_param_map, variable_vertex_map, instance);
    if (primary->type == FUNC_EXPR)
        return compute_func_expr((struct FuncExprNode*)primary, model_param_map, variable_vertex_map, instance);

    fprintf(stderr, "evaluate_primary (%d): unrecognized type %d\n", __LINE__, primary->type);
    return 0;
}

static struct BNVertex* compute_expr(struct ExprNode* expr, struct model_param_map_t* model_param_map,
                                     struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
{
    if (expr->type == IF_EXPR)
        return compute_if_expr((struct IfExprNode*)expr, model_param_map, variable_vertex_map, instance);
    if (expr->type == BINARY_EXPR)
        return compute_binary_expr((struct BinaryExprNode*)expr, model_param_map, variable_vertex_map, instance);
    if (expr->type == PRIMARY_EXPR)
        return compute_primary((struct PrimaryNode*)expr, model_param_map, variable_vertex_map, instance);

    fprintf(stderr, "compute_expr (%d): unrecognized type %d\n", __LINE__, expr->type);
    return 0;
}

/******************************************************************************
Evaluate expressions
******************************************************************************/

static float evaluate_expr(struct ExprNode* expr, struct model_param_map_t* model_param_map);

static float evaluate_if_expr(struct IfExprNode* expr, struct model_param_map_t* model_param_map)
{
    if (evaluate_expr(expr->condition, model_param_map))
        return evaluate_expr(expr->consequent, model_param_map);
    else
        return evaluate_expr(expr->alternative, model_param_map);
}

static float evaluate_binary_expr(struct BinaryExprNode* expr, struct model_param_map_t* model_param_map)
{
    float left;
    float right;

    left = evaluate_expr(expr->left, model_param_map);
    right = evaluate_expr(expr->right, model_param_map);

    if (expr->op == OP_ADD)
        return left + right;
    if (expr->op == OP_SUB)
        return left - right;
    if (expr->op == OP_MUL)
        return left * right;
    if (expr->op == OP_DIV)
        return left / right;

    fprintf(stderr, "evaluate_binary_expr (%d): unrecognized operator %d\n", __LINE__, expr->op);
    return 0;
}

static float evaluate_primary(struct PrimaryNode* primary, struct model_param_map_t* model_param_map);

static float evaluate_unary_expr(struct UnaryExprNode* expr, struct model_param_map_t* model_param_map)
{
    float value;

    value = evaluate_primary(expr->primary, model_param_map);
    if (expr->op == OP_NEG)
        return -value;

    fprintf(stderr, "evaluate_unary_expr (%d): unrecognized operator %d\n", __LINE__, expr->op);
    return 0;
}

static float evaluate_func_expr(struct FuncExprNode* expr, struct model_param_map_t* model_param_map)
{
    symbol_t sym_exp;
    symbol_t sym_log;
    float value;

    sym_exp = symbol_table_lookup_symbol(node_symbol_table(expr), "exp");
    sym_log = symbol_table_lookup_symbol(node_symbol_table(expr), "log");

    if (expr->name == sym_exp) {
        assert(expr_seq_length(expr->expr_seq) == 1);
        value = evaluate_expr(expr->expr_seq->expr, model_param_map);
        return exp(value);
    }

    if (expr->name == sym_log) {
        assert(expr_seq_length(expr->expr_seq) == 1);
        value = evaluate_expr(expr->expr_seq->expr, model_param_map);
        return log(value);
    }

    fprintf(stderr, "evaluate_func_expr (%d): unrecognized function %s\n", __LINE__, symbol_to_string(node_symbol_table(expr), expr->name));
    return 0;
}

static float evaluate_integer_value(struct IntegerValueNode* integer_value)
{
    float value;
    int i;

    value = 0;
    for (i = 0; i < integer_value->length; i++) {
        value *= 10;
        value += integer_value->digits[i] - '0';
    }
    return value;
}

static float evaluate_real_value(struct RealValueNode* real_value)
{
    float value;
    float base;
    int i;

    value = 0;
    for (i = 0; i < real_value->integer_part->length; i++) {
        value *= 10;
        value += real_value->integer_part->digits[i] - '0';
    }
    base = 1;
    for (i = 0; i < real_value->fractional->length; i++) {
        base /= 10;
        value += base * (real_value->fractional->digits[i] - '0');
    }
    return value;
}

static float evaluate_numerical_value(struct NumericalValueNode* numerical_value)
{
    if (numerical_value->type == REAL_VALUE)
        return evaluate_real_value((struct RealValueNode*)numerical_value);

    if (numerical_value->type == INTEGER_VALUE)
        return evaluate_integer_value((struct IntegerValueNode*)numerical_value);

    fprintf(stderr, "evaluate_numerical_value (%d): unrecognized type %d\n", __LINE__, numerical_value->type);
    return 0;
}

static const char* variable_to_string(struct VariableNode* variable, struct model_param_map_t* model_param_map);

static float evaluate_variable(struct VariableNode* variable, struct model_param_map_t* model_param_map)
{
    const char* variable_string;
    symbol_t symbol;

    variable_string = variable_to_string(variable, model_param_map);
    symbol = symbol_table_lookup_symbol(node_symbol_table(variable), variable_string);
    return model_param_map_get(model_param_map, symbol);
}

static float evaluate_primary(struct PrimaryNode* primary, struct model_param_map_t* model_param_map)
{
    if (primary->type == NUM_EXPR)
        return evaluate_numerical_value(((struct NumExprNode*)primary)->numerical_value);
    if (primary->type == VAR_EXPR)
        return evaluate_variable(((struct VarExprNode*)primary)->variable, model_param_map);
    if (primary->type == UNARY_EXPR)
        return evaluate_unary_expr((struct UnaryExprNode*)primary, model_param_map);
    if (primary->type == GROUP_EXPR)
        return evaluate_expr(((struct GroupExprNode*)primary)->expr, model_param_map);
    if (primary->type == FUNC_EXPR)
        return evaluate_func_expr((struct FuncExprNode*)primary, model_param_map);

    fprintf(stderr, "evaluate_primary (%d): unrecognized type %d\n", __LINE__, primary->type);
    return 0;
}

static float evaluate_expr(struct ExprNode* expr, struct model_param_map_t* model_param_map)
{
    if (expr->type == IF_EXPR)
        return evaluate_if_expr((struct IfExprNode*)expr, model_param_map);
    if (expr->type == BINARY_EXPR)
        return evaluate_binary_expr((struct BinaryExprNode*)expr, model_param_map);
    if (expr->type == PRIMARY_EXPR)
        return evaluate_primary((struct PrimaryNode*)expr, model_param_map);

    fprintf(stderr, "evaluate_expr (%d): unrecognized type %d\n", __LINE__, expr->type);
    return 0;
}

static const char* name_var_to_string(struct NameVarNode* name_var)
{
    return symbol_to_string(node_symbol_table(name_var), name_var->name);
}

static const char* field_var_to_string(struct FieldVarNode* field_var)
{
    const char* name;
    const char* field_name;
    char* string;

    name = symbol_to_string(node_symbol_table(field_var), field_var->name);
    field_name = symbol_to_string(node_symbol_table(field_var), field_var->field_name);

    string = malloc(strlen(name) + 1 + strlen(field_name) + 1);
    strcpy(string, name);
    strcat(string, ".");
    strcat(string, field_name);
    return string;
}

static const char* index_var_to_string(struct IndexVarNode* index_var, struct model_param_map_t* model_param_map)
{
    struct ilist_t* indices;
    struct ExprSeqNode* expr_seq;
    const char* name;
    size_t len;
    struct ilist_entry_t* index;
    char index_string[MAX_INTEGER_VALUE_SIZE];
    char* string;

    /* Step 1. Evaluate index expressions */
    indices = new_ilist();
    expr_seq = index_var->expr_seq;
    while (expr_seq) {
        ilist_append(indices, (int)evaluate_expr(expr_seq->expr, model_param_map));
        expr_seq = expr_seq->expr_seq;
    }

    /* Step 2. Calculate index_var string representation length */
    name = symbol_to_string(node_symbol_table(index_var), index_var->name);
    len = strlen(name) + 1; /* name[ */
    index = indices->entry;
    while (index) {
        sprintf(index_string, "%d", index->data);
        len += strlen(index_string);
        if (index->next)
            len += 1; /* , */
        index = index->next;
    }
    len += 1; /* ] */

    /* Step 3. Concatenate index_var string components */
    string = malloc(len + 1);
    strcpy(string, name);
    strcat(string, "[");
    index = indices->entry;
    while (index) {
        sprintf(index_string, "%d", index->data);
        strcat(string, index_string);
        if (index->next)
            strcat(string, ",");
        index = index->next;
    }
    strcat(string, "]");

    return string;
}

static const char* variable_to_string(struct VariableNode* variable, struct model_param_map_t* model_param_map)
{
    if (variable->type == NAME_VAR)
        return name_var_to_string((struct NameVarNode*)variable);
    if (variable->type == FIELD_VAR)
        return field_var_to_string((struct FieldVarNode*)variable);
    if (variable->type == INDEX_VAR)
        return index_var_to_string((struct IndexVarNode*)variable, model_param_map);
    return 0;
}

/* forward declaration */
static void execute_stmts(struct StmtsNode* stmts, struct model_param_map_t* model_param_map,
                          struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance);

static void execute_draw_dbern(struct DrawStmtNode* draw_stmt, struct model_param_map_t* model_param_map,
                               struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
{
    struct ExprNode* p;
    struct BNVertex* p_vert;
    struct BNVertexDrawBern* vertex;
    const char* variable_string;
    symbol_t variable;

    assert(expr_seq_length(draw_stmt->expr_seq) == 1);
    p = draw_stmt->expr_seq->expr;

    p_vert = compute_expr(p, model_param_map, variable_vertex_map, instance);
    instance_append_vertex(instance, p_vert, 0);

    vertex = malloc(sizeof(struct BNVertexDrawBern));
    vertex->super.super.type = BNV_DRAW;
    vertex->super.type = FLIP;
    vertex->p = p_vert;

    variable_string = variable_to_string(draw_stmt->variable, model_param_map);
    instance_append_vertex(instance, (struct BNVertex*)vertex, variable_string);

    variable = symbol_table_lookup_symbol(node_symbol_table(draw_stmt), variable_string);
    variable_vertex_map_put(variable_vertex_map, variable, (struct BNVertex*)vertex);
}

static void execute_draw_dnorm(struct DrawStmtNode* draw_stmt, struct model_param_map_t* model_param_map,
                               struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
{
    struct ExprNode* mean;
    struct ExprNode* variance;
    struct BNVertex* mean_vert;
    struct BNVertex* variance_vert;
    struct BNVertexDrawNorm* vertex;
    const char* variable_string;
    symbol_t variable;

    assert(expr_seq_length(draw_stmt->expr_seq) == 2);
    mean = draw_stmt->expr_seq->expr;
    variance = draw_stmt->expr_seq->expr_seq->expr;

    mean_vert = compute_expr(mean, model_param_map, variable_vertex_map, instance);
    variance_vert = compute_expr(variance, model_param_map, variable_vertex_map, instance);

    instance_append_vertex(instance, mean_vert, 0);
    instance_append_vertex(instance, variance_vert, 0);

    vertex = malloc(sizeof(struct BNVertexDrawNorm));
    vertex->super.super.type = BNV_DRAW;
    vertex->super.type = GAUSSIAN;
    vertex->mean = mean_vert;
    vertex->variance = variance_vert;

    variable_string = variable_to_string(draw_stmt->variable, model_param_map);
    instance_append_vertex(instance, (struct BNVertex*)vertex, variable_string);

    variable = symbol_table_lookup_symbol(node_symbol_table(draw_stmt), variable_string);
    variable_vertex_map_put(variable_vertex_map, variable, (struct BNVertex*)vertex);
}

static void execute_draw_dgamma(struct DrawStmtNode* draw_stmt, struct model_param_map_t* model_param_map,
                                struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
{
    struct ExprNode* a;
    struct ExprNode* b;
    struct BNVertex* a_vert;
    struct BNVertex* b_vert;
    struct BNVertexDrawGamma* vertex;
    const char* variable_string;
    symbol_t variable;

    assert(expr_seq_length(draw_stmt->expr_seq) == 2);
    a = draw_stmt->expr_seq->expr;
    b = draw_stmt->expr_seq->expr_seq->expr;

    a_vert = compute_expr(a, model_param_map, variable_vertex_map, instance);
    b_vert = compute_expr(b, model_param_map, variable_vertex_map, instance);

    instance_append_vertex(instance, a_vert, 0);
    instance_append_vertex(instance, b_vert, 0);

    vertex = malloc(sizeof(struct BNVertexDrawGamma));
    vertex->super.super.type = BNV_DRAW;
    vertex->super.type = GAMMA;
    vertex->a = a_vert;
    vertex->b = b_vert;

    variable_string = variable_to_string(draw_stmt->variable, model_param_map);
    instance_append_vertex(instance, (struct BNVertex*)vertex, variable_string);

    variable = symbol_table_lookup_symbol(node_symbol_table(draw_stmt), variable_string);
    variable_vertex_map_put(variable_vertex_map, variable, (struct BNVertex*)vertex);
}

static void execute_draw_stmt(struct DrawStmtNode* draw_stmt, struct model_param_map_t* model_param_map,
                              struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
{
    /* variable ~ dist(expr_seq) */
    symbol_t dbern;
    symbol_t dnorm;
    symbol_t dgamma;

    dbern = symbol_table_lookup_symbol(node_symbol_table(draw_stmt), "dbern");
    dnorm = symbol_table_lookup_symbol(node_symbol_table(draw_stmt), "dnorm");
    dgamma = symbol_table_lookup_symbol(node_symbol_table(draw_stmt), "dgamma");

    if (draw_stmt->dist == dbern)
        execute_draw_dbern(draw_stmt, model_param_map, variable_vertex_map, instance);
    else if (draw_stmt->dist == dnorm)
        execute_draw_dnorm(draw_stmt, model_param_map, variable_vertex_map, instance);
    else if (draw_stmt->dist == dgamma)
        execute_draw_dgamma(draw_stmt, model_param_map, variable_vertex_map, instance);
    else
        fprintf(stderr, "execute_draw_stmt (%d): unrecognized distribution", __LINE__);
}

static void execute_let_stmt(struct LetStmtNode* let_stmt, struct model_param_map_t* model_param_map,
                             struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
{
    if (let_stmt->expr->type == NEW_EXPR) {
        /* FIXME variable = new name () */
        fprintf(stderr, "execute_let_stmt (%d): NEW_EXPR not implemented\n", __LINE__);
    } else {
        /* variable = expr */
        struct BNVertex* vertex;
        const char* variable_string;
        symbol_t variable;

        vertex = compute_expr(let_stmt->expr, model_param_map, variable_vertex_map, instance);

        variable_string = variable_to_string(let_stmt->variable, model_param_map);
        instance_append_vertex(instance, vertex, variable_string);

        variable = symbol_table_lookup_symbol(node_symbol_table(let_stmt), variable_string);
        variable_vertex_map_put(variable_vertex_map, variable, vertex);
    }
}

static void execute_for_stmt(struct ForStmtNode* for_stmt, struct model_param_map_t* model_param_map,
                             struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
{
    /* "for" name "=" start_expr "to" end_expr "{" stmts "}" */
    float start_value;
    float end_value;
    float value;

    start_value = evaluate_expr(for_stmt->start_expr, model_param_map);
    end_value = evaluate_expr(for_stmt->end_expr, model_param_map);

    for (value = start_value; value <= end_value; value++) {
        model_param_map_put(model_param_map, for_stmt->name, value);
        execute_stmts(for_stmt->stmts, model_param_map, variable_vertex_map, instance);
    }
}

static void execute_stmts(struct StmtsNode* stmts, struct model_param_map_t* model_param_map,
                          struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
{
    struct StmtNode* stmt;

    while (stmts) {
        stmt = stmts->stmt;

        if (stmt->type == DRAW_STMT)
            execute_draw_stmt((struct DrawStmtNode*)stmt, model_param_map, variable_vertex_map, instance);
        else if (stmt->type == LET_STMT)
            execute_let_stmt((struct LetStmtNode*)stmt, model_param_map, variable_vertex_map, instance);
        else if (stmt->type == FOR_STMT)
            execute_for_stmt((struct ForStmtNode*)stmt, model_param_map, variable_vertex_map, instance);
        else
            fprintf(stderr, "instantiate_model (%d): unrecognized stmt type %d\n", __LINE__, stmt->type);

        stmts = stmts->stmts;
    }
}

static void instantiate_model(struct ModelNode* model, float* model_params,
                              struct variable_vertex_map_t* variable_vertex_map, struct pp_instance_t* instance)
{
    struct model_param_map_t model_param_map = {0};
    struct ModelParamsNode* model_params_node;
    struct BNVertex* vertex;

    model_params_node = model->params;
    while (model_params_node && model_params) {
        model_param_map_put(&model_param_map, model_params_node->name, *model_params);

        vertex = malloc(sizeof(struct BNVertex));
        vertex->type = BNV_CONST;
        vertex->sample = *model_params;
        instance_append_vertex(instance, vertex, symbol_to_string(node_symbol_table(model), model_params_node->name));
        variable_vertex_map_put(variable_vertex_map, model_params_node->name, vertex);

        model_params_node = model_params_node->model_params;
        model_params++;
    }
    if (model_params_node || model_params) {
        fprintf(stderr, "instantiate_model (%d): model params number mismatch\n", __LINE__);
    }

    execute_stmts(model->stmts, &model_param_map, variable_vertex_map, instance);
}

static void init_instance(struct ModelNode* model, float* model_params, struct pp_instance_t* instance)
{
    struct variable_vertex_map_t variable_vertex_map = {0};
    instance->n = 0;
    instance->vertices = new_list();
    instance->vertex_names = new_list();
    instantiate_model(model, model_params, &variable_vertex_map, instance);
}
