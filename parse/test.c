#include "parse.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    struct ModelsNode* result;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    result = parse_file(argv[1]);
    if (result)
        printf("%s", dump_models(result));
    else
        printf("syntax error\n");

    return 0;
}
