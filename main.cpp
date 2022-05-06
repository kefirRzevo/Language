#include <stdio.h>
#include <stdlib.h>

#include "include/frontend/lexer.h"
#include "include/frontend/recursive_decent.h"

int main()
{
    array  idents = {};
    array  tokens = {};
    tokenize("example.txt", &idents, &tokens);
    Tree* program = create_tree(&tokens);
    array_dtor(&idents, 1);
    array_dtor(&tokens, 0);
    return 0;
}