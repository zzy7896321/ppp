#include "query.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../debug.h"

const char* pp_query_compare_string[] = {
	"==",
	"!=",
	"<",
	">",
	">=",
	"<=",
};

struct pp_query_t* new_pp_query(const char* varname, ilist_entry_t* index,
								pp_query_compare_t compare, pp_variable_t* threshold, pp_query_t* next) {
	pp_query_t* query = malloc(sizeof(pp_query_t));
	
	int len = strlen(varname);
	query->varname = malloc(sizeof(char) * (len + 1));
	strcpy(query->varname, varname);
	query->index = index;

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

	DUMP_START();
	while (query) {
		DUMP(buffer, buf_size, "%s", query->varname);
		if (query->index) {
			ilist_entry_t* ind_entry = query->index;
			DUMP(buffer, buf_size, "[%d", ind_entry->data);
			while ((ind_entry = ind_entry->next)) {
				DUMP(buffer, buf_size, ", %d", ind_entry->data);
			}
			DUMP(buffer, buf_size, "]");
		}

		DUMP(buffer, buf_size, " %s", pp_query_compare_string[query->compare]);

		switch (query->threshold->type) {
		case PP_VARIABLE_INT:
			DUMP(buffer, buf_size, " %d\n", PP_VARIABLE_INT_VALUE(query->threshold));
			break;
		case PP_VARIABLE_FLOAT:
			DUMP(buffer, buf_size, " %f\n", PP_VARIABLE_FLOAT_VALUE(query->threshold));
			break;
		case PP_VARIABLE_VECTOR:
			break;
		}
		query = query->next; 
	}

	DUMP_SUCCESS();
}

void pp_query_destroy(pp_query_t* query) {
	while (query) {
		pp_query_t* to_free = query;
		query = query->next;
		pp_variable_destroy(to_free->threshold);
		free(to_free->varname);
		while (to_free->index) {
			ilist_entry_t* prev_entry = to_free->index;
			to_free->index = to_free->index->next;
			free(prev_entry);
		}
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
	TOKEN_LSQRBRACKET,
	TOKEN_RSQRBRACKET,
	TOKEN_COMMA,
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
	"TOKEN_LSQRBRACKET",
	"TOKEN_RSQRBRACKET",
	"TOKEN_COMMA",
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

	else if (ch == '[') {
		buffer[0] = '[';
		buffer[1] = '\0';
		return TOKEN_LSQRBRACKET;
	}

	else if (ch == ']') {
		buffer[0] = ']';
		buffer[1] = '\0';
		return TOKEN_RSQRBRACKET;
	}

	else if (ch == ',') {
		buffer[0] = ',';
		buffer[1] = '\0';
		return TOKEN_COMMA;
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
			query_lex_ungetc(lex, ch);
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
	ID_BEFOREIND,
	ID_IND,
	ID_CP,
	//ID_CP_VAL,
} pp_compile_query_state_t;

static const char* pp_compile_query_state_string[] = {
	"INITIAL",
	"ID",
	"ID_BEFOREIND",
	"ID_IND"
	"ID_CP",
};

pp_query_t* pp_compile_query(const char* query_string) {
	query_lex_t* lex = new_query_lex(query_string);
	pp_query_t* query = 0;

	char buffer[PP_COMPILE_QUERY_BUFFER_SIZE + 1];
	int token;
	int state = INITIAL;

	ilist_entry_t* head = 0;
	ilist_entry_t** tail_ptr = &head;

	#define RETURN_ERROR(err_string)	\
		pp_query_destroy(query);	\
		fprintf(stderr, "pp_compile_query: %s: %s(%s)\n", err_string, buffer, token_string[token]);	\
		fprintf(stderr, "pp_compile_query: current state is %s\n", pp_compile_query_state_string[state]);	\
		query_lex_destroy(lex);	\
		return 0

	while ((token = query_lex_next_token(lex, buffer, PP_COMPILE_QUERY_BUFFER_SIZE + 1)) != TOKEN_EOF) {
//		fprintf(stderr, "token = %s, value = \"%s\"\n", token_string[token], buffer);
		if (token == TOKEN_ERROR) {
			RETURN_ERROR("error in token");
		}
		switch (state) {
		case INITIAL:
			if (token != TOKEN_IDENTIFIER) {
				RETURN_ERROR("expecting identifier, but got");
			}

			query = new_pp_query(buffer, 0, PP_QUERY_EQ, 0, query);
			state = ID;
			break;
		case ID:
			switch (token) {
			case TOKEN_LSQRBRACKET:
				state = ID_BEFOREIND;
				break;
			case TOKEN_LT:
				query->compare = PP_QUERY_LT;
				state = ID_CP;
				break;
			case TOKEN_LE:
				query->compare = PP_QUERY_LE;
				state = ID_CP;
				break;
			case TOKEN_GT:
				query->compare = PP_QUERY_GT;
				state = ID_CP;
				break;
			case TOKEN_GE:
				query->compare = PP_QUERY_GE;
				state = ID_CP;
				break;
			case TOKEN_EQ:
				query->compare = PP_QUERY_EQ;
				state = ID_CP;
				break;
			case TOKEN_NE:
				query->compare = PP_QUERY_NE;
				state = ID_CP;
				break;
			default:
				RETURN_ERROR("expecting REL or '[', but got");
			}

			break;
		case ID_BEFOREIND:
			if (token != TOKEN_INT) {
				RETURN_ERROR("expecting index, but got");
			}
			{
				int value = atoi(buffer);
				ilist_entry_t* entry = malloc(sizeof(ilist_entry_t));
				entry->data = value;
				entry->next = 0;
				*tail_ptr = entry;
				tail_ptr = &(entry->next);
				state = ID_IND;
			}
			break;
		case ID_IND:
			switch (token) {
			case TOKEN_RSQRBRACKET:
				state = ID;
				break;
			case TOKEN_COMMA:
				state = ID_BEFOREIND;
				break;
			default:
				RETURN_ERROR("expecting ',' or ']', but got");
			}
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
				RETURN_ERROR("expecting number, but got");
			}

			if (head) {
				query->index = head;
				head = 0;
				tail_ptr = &head;
			}
			state = INITIAL;
			break;
		}
	}

	if (state != INITIAL) {
		RETURN_ERROR("unexpected EOF");
	}
	#undef RETURN_ERROR

	query_lex_destroy(lex);
	return query;
}

int pp_query_acceptor(pp_trace_t* trace, pp_query_t* query) {
	if (!query) return 1;
	
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
		ilist_entry_t* ind_entry = query->index;
		while (ind_entry) {
			if (!var || var->type != PP_VARIABLE_VECTOR) {
				printf("Query: subscripting to non-vector type\n");
				return -1;
			}

			var = PP_VARIABLE_VECTOR_VALUE(var)[ind_entry->data];
			ind_entry = ind_entry->next;
		}
		if (!var) {
			printf("Query: cannot find %s\n", query->varname);
			return -1;
		}

		switch (var->type) {
		case PP_VARIABLE_INT:
			switch (query->threshold->type) {
			case PP_VARIABLE_INT:
				{
					int result;
					PP_QUERY_ACCEPTOR_COMPARE(query->compare, PP_VARIABLE_INT_VALUE(var), PP_VARIABLE_INT_VALUE(query->threshold), result)
					if (!result) {
						return 0;
					}
				}
				break;
			case PP_VARIABLE_FLOAT:
				{
					int result;
					PP_QUERY_ACCEPTOR_COMPARE(query->compare, PP_VARIABLE_INT_VALUE(var), PP_VARIABLE_FLOAT_VALUE(query->threshold), result)
					if (!result) {
						return 0;
					}
				}
				break;
			case PP_VARIABLE_VECTOR:
				return 0;
			}
			break;
		case PP_VARIABLE_FLOAT:
			switch (query->threshold->type) {
			case PP_VARIABLE_INT:
				{
					int result;
					PP_QUERY_ACCEPTOR_COMPARE(query->compare, PP_VARIABLE_FLOAT_VALUE(var), PP_VARIABLE_INT_VALUE(query->threshold), result)
					if (!result) {
						return 0;
					}
				}
				break;
			case PP_VARIABLE_FLOAT:
				{
					int result;
					PP_QUERY_ACCEPTOR_COMPARE(query->compare, PP_VARIABLE_FLOAT_VALUE(var), PP_VARIABLE_FLOAT_VALUE(query->threshold), result)
					if (!result) {
						return 0;
					}
				}
				break;
			case PP_VARIABLE_VECTOR:
				return 0;
			}
			break;
		case PP_VARIABLE_VECTOR:
			printf("Query: vector comparison not implemented\n");
			/* unhandled */
			return -1;
		}
		query = query->next;
	}

	#undef PP_QUERY_ACCEPTOR_COMPARE

	return 1;
}
