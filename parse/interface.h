#ifndef INTERFACE_H
#define INTERFACE_H

#include "../defs.h"
#include "parse.h"
#include "../common/symbol_table.h"
#include "../common/list.h"

typedef struct model_map_t
{
    symbol_t model_name;
    struct ModelNode* model;
    struct model_map_t* next;
} model_map_t;

ModelNode* model_map_find(model_map_t* model_map, symbol_table_t* symbol_table, const char* model_name);

typedef struct pp_state_t
{
    struct symbol_table_t* symbol_table;
    struct model_map_t* model_map;
} pp_state_t;

pp_state_t* pp_new_state();
int pp_free(pp_state_t* state);
int pp_load_file(pp_state_t* state, const char* filename);
int add_models(pp_state_t* instance, ModelsNode* models);

/*
typedef struct pp_instance_t
{
    int n;
    struct list_t* vertices;
    struct list_t* vertex_names;
} pp_instance_t;

pp_instance_t* pp_new_instance(pp_state_t* state, const char* model_name, float* model_params);
int pp_instance_num_vertices(pp_instance_t* instance);
struct BNVertex* pp_instance_vertex(pp_instance_t* instance, int i);
const char* pp_insatnce_vertex_name(pp_instance_t* instance, int i);
int pp_instance_find_num_of_vertex(pp_instance_t* instance, struct BNVertex* vertex);
float pp_name_to_value(pp_instance_t* instance, const char* name); */

#endif
