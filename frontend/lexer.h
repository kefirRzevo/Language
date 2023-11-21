#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/array.h"

enum tok_keyword
{
    #define LINKABLE(X) X
    #define UNLINKABLE(X) X
    #define KEYWORD(name, keyword, ident)   \
        name = keyword,
        #include "../res/KEYWORDS"
    #undef LINKABLE
    #undef UNLINKABLE 
    #undef KEYWORD        
};

enum tok_type
{
    KEYWORD = 0,
    IDENT   = 1,
    NUMBER  = 2,
};

union tok_value
{
    char*  ident;
    double number;
    int    keyword;
};

struct token
{
    tok_value value;
    size_t    line; 
    tok_type  type; 
};


token* tokenize(const char* file_path, array* idents, array* tokens);


#endif 