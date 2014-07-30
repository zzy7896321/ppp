#ifndef PARSE_H_
#define PARSE_H_

struct ModelsNode;

struct ModelsNode* parse_file(const char* filename);

int dump_models(struct ModelsNode* models);

#endif
