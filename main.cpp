#include <stdio.h>
#include <stdlib.h>

#include "frontend/lexer.h"
#include "frontend/recursive_decent.h"
#include "backend/compiler.h"

int main()
{
    array  idents = {};
    array  tokens = {};
    
    tokenize("examples/factorial.ll", &idents, &tokens);
    Tree* program = create_tree(&tokens);
    compile_tree("asmcode", program);

    array_dtor(&idents, 1);
    array_dtor(&tokens, 0);
    //tree_dtor(program);
    /*Tree* program = tree_read(&idents, "tree");
    compile_tree("asmcode", program);
    array_dtor(&idents, 1);*/
    return 0;
}
