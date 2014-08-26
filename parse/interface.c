#include "interface.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

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

int add_models(struct pp_state_t* state, struct ModelsNode* models)
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

struct ModelNode* model_map_find(struct model_map_t* model_map,
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
void init_instance(struct ModelNode* model, float* model_params, struct pp_instance_t* instance);

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
