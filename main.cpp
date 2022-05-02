#include <stdio.h>
#include <stdlib.h>

#include "include/frontend/lexer.h"

int main()
{
    array  idents = {};
    array  tokens = {};
    tokenize("example.txt", &idents, &tokens);
    //tree* program = create_tree(&tokens);
    array_dtor(&idents, 1);
    array_dtor(&tokens, 0);
    return 0;
}