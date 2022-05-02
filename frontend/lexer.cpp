#include "../include/frontend/lexer.h"


static size_t create_buf(const char* file_path, char** buf);

static void skip_spaces();
static void find_ending(size_t* token_length);

static int make_keyword(array* tokens, int keyword);
static int make_ident(  array* tokens, array* idents, char* ident, size_t ident_size);
static int make_number( array* tokens);

static int   is_linkable(int* keyword, size_t* keyword_length);
static int is_unlinkable(int* keyword, size_t* keyword_length);


char*  iterator = nullptr;
size_t lines    = 1;


#define $$$ fprintf(stderr, "|%d|%s|\n", __LINE__, iterator);
#define $$ fprintf(stderr, "|%d|\n", __LINE__);

#undef $$
#undef $$$


static size_t create_buf(const char* file_path, char** buf)
{
    FILE* fp = fopen(file_path, "r");
    if(!fp)
        return 0;
                                      fseek(fp,  0L, SEEK_END);
    size_t file_size        = (size_t)ftell(fp) + 1;
                                      fseek(fp,  0L, SEEK_SET);

    *buf = (char *)calloc(file_size, sizeof(char));
    if(!*buf)
        return 0;

    fread(*buf, sizeof(char), file_size, fp);
    fclose(fp);
    return file_size;
}


static void skip_spaces()
{
    while(isspace(*iterator) || !strncmp(iterator, "/*", 2))
    {
        while(isspace(*iterator))
        {
            if(*iterator == '\n')
                lines++;

            iterator++;
        }

        if(!strncmp(iterator, "/*", 2))
        {
            iterator += 2;

            while(strncmp(iterator, "*/", 2) && *iterator != '\0')
            {
                if(*iterator == '\n')
                    lines++;

                iterator++;
            }

            if(*iterator != '\0')
                iterator += 2;
        }
    }
}


static int make_keyword(array* tokens, int keyword)
{
    assert(tokens);

    token temp_token = {};
    temp_token.type  = KEYWORD;
    temp_token.line  = lines;
    temp_token.value.keyword = keyword;

    if(!array_push(tokens, &temp_token))
        return 0;
    return 1;
}


static int make_ident(array* tokens, array* idents, char* ident, size_t ident_size)
{
    assert(tokens);

    token temp_token = {};
    temp_token.type  = IDENT;
    temp_token.line  = lines;

    char** idents_data = (char** )idents->data;
    for(size_t i = 0; i < idents->size; i++)
    {
        if(!strncmp(ident, idents_data[i], ident_size) && ident_size == strlen(idents_data[i]))
        {
            temp_token.value.ident = idents_data[i];
            break;
        }
    }

    if(!temp_token.value.ident)
    {
        char* temp_ident = (char* )calloc(ident_size + 1, sizeof(char));
        if(!temp_ident)
            return 0;

        memcpy(temp_ident, ident, ident_size);
        temp_ident[ident_size] = '\0';
        temp_token.value.ident = temp_ident;

        if(!array_push(idents, &temp_ident))
            return 0;
    }
    
    if(!array_push(tokens, &temp_token))
        return 0;
    
    return 1;
}


static int make_number(array* tokens)
{
    assert(tokens);

    token temp_token = {};
    temp_token.type  = NUMBER;
    temp_token.line  = lines;
    
    int n_digits = 0;
    sscanf(iterator, "%lf%n", &temp_token.value.number, &n_digits);
    iterator += n_digits;
    
    if(!array_push(tokens, &temp_token))
        return 0;

    return 1;
}


static int is_linkable(int* _keyword, size_t* keyword_length)
{
    assert(_keyword);
    assert(keyword_length);
    
    #define LINKABLE(X) X
    #define UNLINKABLE(X)
    #define KEYWORD(name, keyword, ident)                                   \
                    else if (!strncmp(ident, iterator, sizeof(ident) - 1))  \
                    {                                                       \
                        *_keyword = keyword;                                \
                        *keyword_length = sizeof(ident) - 1;                \
                        return 1;                                           \
                    }

                    if (0) {} 
    #include "../KEYWORDS"
    #undef LINKABLE
    #undef UNLINKABLE 
    #undef KEYWORD 

    return 0;
}


static int is_unlinkable(int* _keyword, size_t* keyword_length)
{
    assert(_keyword);
    assert(keyword_length);

    #define LINKABLE(X)
    #define UNLINKABLE(X) X
    #define KEYWORD(name, keyword, ident)                                   \
                    else if (!strncmp(ident, iterator, sizeof(ident) - 1))  \
                    {                                                       \
                        *_keyword = keyword;                                \
                        *keyword_length = sizeof(ident) - 1;                \
                        return 1;                                           \
                    }

                    if (0) {} 
    #include "../KEYWORDS"
    #undef LINKABLE
    #undef UNLINKABLE 
    #undef KEYWORD 

    return 0;
}


static void find_ending(size_t* token_length)
{
    char* begin = iterator;

    while(!is_unlinkable((int* )token_length, token_length) && !isspace(*iterator) && strncmp(iterator, "/*", 2) && *iterator != '\0')
        iterator++;
    
    *token_length = (size_t)(iterator - begin);
                             iterator = begin;
}


token* tokenize(const char* file_path, array* idents, array* tokens)
{
    char*  buf = nullptr;
    create_buf(file_path, &buf);

    if(!buf)
        return nullptr;

    array_ctor(tokens, sizeof(token ));
    array_ctor(idents, sizeof(char *));
 
    iterator = buf;

    while(*iterator != '\0')
    {
        skip_spaces();

        if(*iterator == '\0')
            break;

 int    keyword = 0;
 size_t keyword_length = 0;
 size_t   token_length = 0;

        if(isdigit(*iterator))
        {
            if(!make_number(tokens))
                return nullptr;

            continue;
        }

        if(is_unlinkable(&keyword, &keyword_length))
        {
            if(!make_keyword(tokens, keyword))
                return nullptr;

            iterator += keyword_length;
            continue;
        }

        find_ending(&token_length);

        if(is_linkable(&keyword, &keyword_length) && keyword_length == token_length)
        {
            if(!make_keyword(tokens, keyword))
                return nullptr;
            
            iterator += keyword_length;
            continue;
        }

        if(!make_ident(tokens, idents, iterator, token_length))
            return nullptr;

        iterator += token_length;    
    }

    free(buf);
    tokens_dump(tokens, "logfiles/token_dump.txt");

    return (token* )tokens->data;
}


const char* keyword_string(int type)
{
    #define LINKABLE(X)  X
    #define UNLINKABLE(X) X
    #define KEYWORD(name, keyword, ident)      \
                    else if (keyword == type)  \
                        return ident;                      

                    if (0) {} 
    #include "../KEYWORDS"
    #undef LINKABLE
    #undef UNLINKABLE 
    #undef KEYWORD 

    return nullptr;
}


int tokens_dump(const array* const tokens, const char* dump_file_path)
{
    assert(tokens);
    FILE* fp = fopen(dump_file_path, "w");
    if(!fp)
        return 0;
    
    dump_init(fp, tokens);
    fprintf(fp, "|==========|==========|=============================|\n");

    for(size_t i = 0; i < tokens->size; i++)
    {
        token temp_token = *((token *)((char* )tokens->data + i * tokens->item_size));
        switch(temp_token.type)
        {
            case KEYWORD:
                fprintf(fp, "|LINE: %-4zu|KEYWORD   |VALUE:   %-20s|\n"
                            "|==========|==========|=============================|\n", temp_token.line, keyword_string(temp_token.value.keyword));
                break;
            case NUMBER:
                fprintf(fp, "|LINE: %-4zu|NUMBER    |VALUE:   %-20lg|\n"
                            "|==========|==========|=============================|\n", temp_token.line, temp_token.value.number);
                break;
            case IDENT:
                fprintf(fp, "|LINE: %-4zu|IDENT     |VALUE:   %-20s|\n"
                            "|==========|==========|=============================|\n", temp_token.line, temp_token.value.ident);
                break;
            default:
                break;
        }
    }
    return 1;
}
