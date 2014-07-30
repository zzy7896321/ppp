#include "../ppp.h"
#include "../defs.h"
#include "query.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct pp_query_t* pp_compile_query(struct pp_instance_t* instance, const char* query_string)
{
    /* FIXME
    Because this implementation is wrong, for any query_string,
    the output is for query "2 < x < 3" */
	struct pp_query_t *output;
        char *single_query, *remain_query, *p;
	int varn = 0;
	_Bool positive = 1;
	
	output = (struct pp_query_t*) calloc(1, sizeof(struct pp_query_t));
	single_query = (char*) calloc(1024, sizeof(char));
	strcpy(single_query, query_string);
	remain_query = strchr(single_query, ',');
	if (remain_query) {
		*remain_query = 0;
		remain_query = remain_query + 1;
		output->next = pp_compile_query(instance, remain_query);
	}
	p = single_query;
	while(1) {
		if((*p) == ' ') {
                    p = p + 1;
                    continue;
                }
		if((*p == '=') || (*p == '!') || (*p == '<') || (*p == '>')) break;
		output->varname[varn] = (*p);
		varn = varn + 1;
		p = p + 1;
	}
	if((*p) == '=' && (*(p+1)) == '=') {
		output->compare = PPPL_COMPARE_EQ;
		p = p + 2;
	}
	else if((*p) == '!' && (*(p+1))== '='){
		output->compare = PPPL_COMPARE_NEQ;
		p = p + 2;
	}
	else if((*p) == '<' && (*(p+1)) != '='){
		output->compare = PPPL_COMPARE_LT;
		p=p+1;
	}
	else if((*p)=='>'&&(*(p+1))!='='){
		output->compare=PPPL_COMPARE_GT;
		p=p+1;
	}
	else if((*p)=='<'&&(*(p+1))=='='){
		output->compare=PPPL_COMPARE_NGT;
		p=p+2;
	}
	else if((*p)=='>'&&(*(p+1))=='='){
		output->compare=PPPL_COMPARE_NLT;
		p=p+2;
	}
	
	while(1){
		if((*p)==' ') {
                    p = p + 1;
		    continue;
                }
		if ((*p) == '+'){
			++p;
			continue;
		}

		if ((*p) == '-'){
			++p;
			positive = 0;
			continue;
		}

		if((*p)==0)
			break;
		output->threshold=output->threshold*10+((*p)-'0');
		p=p+1;
	}

	if (!positive) output->threshold = - output->threshold;
	free(single_query);
	return output;
}

int pp_query_acceptor(struct pp_instance_t* instance, name_to_value_t F, void* raw_data)
{
    /* FIXME this implementation is wrong */
	struct pp_query_t* data;
	float x;
	int result=0;
	if(raw_data==0)
		return 1;
	data = (struct pp_query_t*)raw_data;
	x = F(instance,data->varname);

//	fprintf(stderr, "pp_query_acceptor %s, op = %d, val = %f, threshold = %f\n", data->varname, data->compare, x, data->threshold);
	if(data->compare==PPPL_COMPARE_EQ)
		result=(x==data->threshold);
	if(data->compare==PPPL_COMPARE_NEQ)
		result=(x!=data->threshold);
	if(data->compare==PPPL_COMPARE_LT)
		result=(x<data->threshold);
	if(data->compare==PPPL_COMPARE_GT)
		result=(x>data->threshold);
	if(data->compare==PPPL_COMPARE_NGT)
		result=(x<=data->threshold);
	if(data->compare==PPPL_COMPARE_NLT)
		result=(x>=data->threshold);
	return result && pp_query_acceptor(instance,F, data->next);
}

