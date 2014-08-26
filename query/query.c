#include "query.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../infer/hash_table.h"

const char* pp_query_compare_string[] = {
	"==",
	"!=",
	"<",
	">",
	">=",
	"<=",
};

struct pp_query_t* new_pp_query(const char* varname, pp_query_compare_t compare, pp_variable_t* threshold, pp_query_t* next) {
	pp_query_t* query = malloc(sizeof(pp_query_t));
	
	int len = strlen(varname);
	query->varname = malloc(sizeof(char) * (len + 1));
	strcpy(query->varname, varname);

	query->compare = compare;
	query->threshold = threshold;
	query->next = next;

	return query;
}

int pp_query_dump(pp_query_t* query, char* buffer, int buf_size) {
	if (buf_size <= 0) {
		return -1;
	}

	buffer[0] = '\0';

	int num_written = 0;
	while (query) {
		switch (query->threshold->type) {
		case INT:
			num_written += snprintf(buffer + num_written, buf_size - num_written, "%s %s %d\n", query->varname, pp_query_compare_string[query->compare], PP_VARIABLE_INT_VALUE(query->threshold));
			break;
		case FLOAT:
			num_written += snprintf(buffer + num_written, buf_size - num_written, "%s %s %f\n", query->varname, pp_query_compare_string[query->compare], PP_VARIABLE_FLOAT_VALUE(query->threshold));
			break;
		case VECTOR:
			break;
		}
		query = query->next; 
	}

	return num_written;
}

void pp_query_destroy(pp_query_t* query) {
	while (query) {
		pp_query_t* to_free = query;
		query = query->next;
		pp_variable_destroy(to_free->threshold);
		free(to_free);
	}
}

typedef struct query_lex_t {
	const char* query_string;
} query_lex_t;

typedef enum token_t {
	TOKEN_EOF = 0,
	TOKEN_IDENTIFIER,
	TOKEN_INT,
	TOKEN_FLOAT,
	TOKEN_LT,
	TOKEN_LE,
	TOKEN_GT,
	TOKEN_GE,
	TOKEN_EQ,
	TOKEN_NE,
	TOKEN_ERROR,
} token_t;

const char* token_string[] = {
	"TOKEN_EOF",
	"TOKEN_IDENTIFIER",
	"TOKEN_INT",
	"TOKEN_FLOAT",
	"TOKEN_LT",
	"TOKEN_LE",
	"TOKEN_GT",
	"TOKEN_GE",
	"TOKEN_EQ",
	"TOKEN_NE",
	"TOKEN_ERROR",
};

query_lex_t* new_query_lex(const char* query_string) {
	query_lex_t* lex = malloc(sizeof(query_lex_t));
	lex->query_string = query_string;
	return  lex;
}

void query_lex_skipws(query_lex_t* lex) {
	while (*(lex->query_string) && isspace((int)*(lex->query_string))) ++lex->query_string;
}

int query_lex_peek(query_lex_t* lex) {
	if (*(lex->query_string)) return *(lex->query_string);
	else return EOF;
}

int query_lex_getc(query_lex_t* lex) {
	if (*(lex->query_string)) return *((lex->query_string)++);
	else return EOF;
}

int query_lex_ungetc(query_lex_t* lex, int ch) {
	if (ch == EOF) return EOF;
	if (*(--lex->query_string) == ch) {
		return ch;
	}
	++lex->query_string;
	return EOF;
}

int query_lex_next_token(query_lex_t* lex, char* buffer, unsigned n) {
	if (n < 2) return TOKEN_ERROR;

	query_lex_skipws(lex);
	int ch = query_lex_getc(lex);
	if (ch == EOF) {
		buffer[0] = '\0';
		return TOKEN_EOF;
	}

	else if (isalpha(ch) | (ch == '_')) {
		unsigned i = 0;
		buffer[i++] = ch;
		while (ch = query_lex_getc(lex), (isalnum(ch) | (ch == '_'))) {
			if (i + 1 == n) {
				buffer[i++] = '\0';
				query_lex_ungetc(lex, ch);
				return TOKEN_ERROR;	
			}
			buffer[i++] = ch;
		}

		buffer[i++] = '\0';
		query_lex_ungetc(lex, ch);
		return TOKEN_IDENTIFIER;
	}

	else if (ch == '>') {
		buffer[0] = ch;
		
		if (query_lex_peek(lex) == '=') {
			if (2 == n) {
				buffer[1] = '\0';
				return TOKEN_ERROR;
			}
			buffer[1] = query_lex_getc(lex);
			buffer[2] = '\0';
			return TOKEN_GE;
		}

		buffer[1] = '\0';
		return TOKEN_GT;
	}

	else if (ch == '<') {
		buffer[0] = ch;
		
		if (query_lex_peek(lex) == '=') {
			if (2 == n) {
				buffer[1] = '\0';
				return TOKEN_ERROR;
			}
			buffer[1] = query_lex_getc(lex);
			buffer[2] = '\0';
			return TOKEN_LE;
		}

		buffer[1] = '\0';
		return TOKEN_LT;
	}
	else if (ch == '=') {
		buffer[0] = ch;
		if (2 == n) {
			buffer[1] = '\0';
			return TOKEN_ERROR;
		}
		ch = query_lex_getc(lex);
		if (ch != '=') {
			query_lex_ungetc(lex, ch);
			buffer[1] = '\0';
			return TOKEN_ERROR;
		}

		buffer[1] = ch;
		buffer[2] = '\0';
		return TOKEN_EQ;
	}
	else if (ch == '!') {
		buffer[0] = ch;
		if (2 == n) {
			buffer[1] = '\0';
			return TOKEN_ERROR;
		}
		ch = query_lex_getc(lex);
		if (ch != '=') {
			query_lex_ungetc(lex, ch);
			buffer[1] = '\0';
			return TOKEN_ERROR;
		}
		
		buffer[1] = ch;
		buffer[2] = '\0';
		return TOKEN_NE;
	}
	else {
		unsigned i = 0;
		if ((ch == '+') | (ch == '-')) {
			buffer[i++] = ch;
			ch = query_lex_getc(lex);
		}

		if (i + 1 == n) {
			buffer[i++] = '\0';
			query_lex_ungetc(lex, ch);
			return TOKEN_ERROR;
		}

		if (isdigit(ch)) {
			do { 
				if (i + 1 == n) {
					buffer[i++] = '\0';
					query_lex_ungetc(lex, ch);
					return TOKEN_ERROR;
				}
				buffer[i++] = ch;
				ch = query_lex_getc(lex);
			} while (isdigit(ch));
		}

		if (ch != '.') {
			buffer[i++] = '\0';
			return TOKEN_INT;
		}

		if (i + 1 == n) {
			buffer[i++] = '\0';
			query_lex_ungetc(lex, ch);
			return TOKEN_ERROR;
		}
		buffer[i++] = ch;
		ch = query_lex_getc(lex);

		while (isdigit(ch)) {
			if (i + 1 == n) {
				buffer[i++] = '\0';
				query_lex_ungetc(lex, ch);
				return TOKEN_ERROR;
			}
			buffer[i++] = ch;
			ch = query_lex_getc(lex);
		}

		query_lex_ungetc(lex, ch);
		buffer[i++] = '\0';
		if ((buffer[i-1] == '.') & (i > 1 && !isdigit((int)buffer[i-2]))) {
			return TOKEN_ERROR;	// "."
		}

		return TOKEN_FLOAT;
	}
}

void query_lex_destroy(query_lex_t* lex) {
	free(lex);
}

#define PP_COMPILE_QUERY_BUFFER_SIZE 256

typedef enum pp_compile_query_state_t {
	INITIAL = 0,
	ID,
	ID_CP,
	//ID_CP_VAL,
} pp_compile_query_state_t;

static const char* pp_compile_query_state_string[] = {
	"INITIAL",
	"ID",
	"ID_CP",
};

pp_query_t* pp_compile_query(const char* query_string) {
	query_lex_t* lex = new_query_lex(query_string);
	pp_query_t* query = 0;

	char buffer[PP_COMPILE_QUERY_BUFFER_SIZE + 1];
	int token;
	int state = INITIAL;

	#define RETURN_ERROR(err_string)	\
		pp_query_destroy(query);	\
		printf("pp_compile_query: %s: %s\n", err_string, buffer);	\
		fprintf(stderr, "pp_compile_query: current state is %s\n", pp_compile_query_state_string[state]);	\
		query_lex_destroy(lex);	\
		return 0

	while ((token = query_lex_next_token(lex, buffer, PP_COMPILE_QUERY_BUFFER_SIZE + 1)) != TOKEN_EOF) {
		//printf("token = %s\n", token_string[token]);
		if (token == TOKEN_ERROR) {
			RETURN_ERROR("error in token");
		}
		switch (state) {
		case INITIAL:
			if (token != TOKEN_IDENTIFIER) {
				RETURN_ERROR("expecting identifier, but got");
			}

			query = new_pp_query(buffer, PP_QUERY_EQ, 0, query);
			state = ID;
			break;
		case ID:
			switch (token) {
			case TOKEN_LT:
				query->compare = PP_QUERY_LT;
				break;
			case TOKEN_LE:
				query->compare = PP_QUERY_LE;
				break;
			case TOKEN_GT:
				query->compare = PP_QUERY_GT;
				break;
			case TOKEN_GE:
				query->compare = PP_QUERY_GE;
				break;
			case TOKEN_EQ:
				query->compare = PP_QUERY_EQ;
				break;
			case TOKEN_NE:
				query->compare = PP_QUERY_NE;
				break;
			default:
				RETURN_ERROR("expecting relation operator, but got");
			}
			state = ID_CP;

			break;
		case ID_CP:
			switch (token) {
			case TOKEN_INT:
				{
					int value = atoi(buffer);
					query->threshold = new_pp_int(value);
				}
				break;	
			case TOKEN_FLOAT:
				{
					float value = atof(buffer);
					query->threshold = new_pp_float(value);
				}
				break;
			default:
				RETURN_ERROR("expecting a number, but got");
			}
			state = INITIAL;
			break;
		}
	}

	if (state != INITIAL) {
		RETURN_ERROR("end up in non-accepting state");
	}
	#undef RETURN_ERROR

	query_lex_destroy(lex);
	return query;
}

int pp_query_acceptor(pp_trace_t* trace, pp_query_t* query) {
	#define PP_QUERY_ACCEPTOR_COMPARE(compare, x, y, result)	\
		switch (compare) {	\
		case PP_QUERY_LT:	\
			result = (x < y);	\
			break;	\
		case PP_QUERY_LE:	\
			result = (x <= y);	\
			break;	\
		case PP_QUERY_GT:	\
			result = (x > y);	\
			break;	\
		case PP_QUERY_GE:	\
			result = (x >= y);	\
			break;	\
		case PP_QUERY_EQ:	\
			result = (x == y);	\
			break;	\
		case PP_QUERY_NE:	\
			result = (x != y);	\
			break;	\
		}

	while (query) {
		pp_variable_t* var = pp_trace_find_variable(trace, query->varname);
		if (!var) {
			return 0;
		}

		switch (var->type) {
		case INT:
			switch (query->threshold->type) {
			case INT:
				{
					int result;
					PP_QUERY_ACCEPTOR_COMPARE(query->compare, PP_VARIABLE_INT_VALUE(var), PP_VARIABLE_INT_VALUE(query->threshold), result)
					if (!result) {
						return 0;
					}
				}
				break;
			case FLOAT:
				{
					int result;
					PP_QUERY_ACCEPTOR_COMPARE(query->compare, PP_VARIABLE_INT_VALUE(var), PP_VARIABLE_FLOAT_VALUE(query->threshold), result)
					if (!result) {
						return 0;
					}
				}
				break;
			case VECTOR:
				return 0;
			}
			break;
		case FLOAT:
			switch (query->threshold->type) {
			case INT:
				{
					int result;
					PP_QUERY_ACCEPTOR_COMPARE(query->compare, PP_VARIABLE_FLOAT_VALUE(var), PP_VARIABLE_INT_VALUE(query->threshold), result)
					if (!result) {
						return 0;
					}
				}
				break;
			case FLOAT:
				{
					int result;
					PP_QUERY_ACCEPTOR_COMPARE(query->compare, PP_VARIABLE_FLOAT_VALUE(var), PP_VARIABLE_FLOAT_VALUE(query->threshold), result)
					if (!result) {
						return 0;
					}
				}
				break;
			case VECTOR:
				return 0;
			}
			break;
		case VECTOR:
			/* unhandled */
			return 0;
		}
		query = query->next;
	}

	#undef PP_QUERY_ACCEPTOR_COMPARE

	return 1;
}
