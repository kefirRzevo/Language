#include "lexer.h"

static const char* keyword_string(int type);
static size_t create_buf(const char* file_path, char** buf);
static int    tokens_dump(const array* const tokens, const char* dump_file_path);

static void skip_spaces(char** position, size_t* lines);
static bool is_keyword_before(array* tokens);

static bool   is_keyword(char* position, int*   keyword, size_t* key_len);
static bool   is_number (array* tokens, char* position, double* number, size_t* num_len);
static bool push_ident  (array* tokens, array* idents, char* position, size_t id_len, size_t line);
static bool push_keyword(array* tokens, int   keyword, size_t line);
static bool push_number (array* tokens, double number, size_t line);


#define $$$ fprintf(stderr, "|%d|%s|\n", __LINE__, iterator);
#define $$  fprintf(stderr, "|%d|\n",    __LINE__);

//#undef  $$
//#undef $$$


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


static void skip_spaces(char** position, size_t* lines)
{
    while(isspace(**position) || !strncmp(*position, "/*", 2))
    {
        while(isspace(**position))
        {
            if(**position == '\n')
                (*lines)++;

            (*position)++;
        }

        if(!strncmp(*position, "/*", 2))
        {
            *position += 2;

            while(strncmp(*position, "*/", 2) && **position != '\0')
            {
                if(**position == '\n')
                    (*lines)++;

                (*position)++;
            }

            if(**position != '\0')
                *position += 2;
        }
    }
}


static bool is_keyword(char* position, int* keyword, size_t* key_len)          
{
    assert(position);
    assert(keyword);
    assert(key_len);

    #define KEYWORD(name, key, ident)                                       \
                    else if (!strncmp(ident, position, sizeof(ident) - 1))  \
                    {                                                       \
                        *key_len = sizeof(ident) - 1;                       \
                        *keyword = key;                                     \
                        return true;                                        \
                    }

                    if (0) {} 
    #include "../res/KEYWORDS"
    #undef KEYWORD 

    return false;
}


static bool push_keyword(array* tokens, int keyword, size_t line)
{
    assert(tokens);

    token temp_token               = {};
          temp_token.type          = KEYWORD;
          temp_token.line          = line;
          temp_token.value.keyword = keyword;

    if(!array_push(tokens, &temp_token))
        return false;

    return true;
}


static bool is_number(array* tokens, char* position, double* number, size_t* num_len) 
{
    assert(tokens);
    assert(position);
    assert(number);
    assert(num_len);

    if(!(isdigit(*position) || (is_keyword_before(tokens) && *position == '-' && isdigit(*(position + 1)))))
        return false;

    sscanf(position, "%lf%n", number, (int* )num_len);
    return true;
}


static bool push_number(array* tokens, double number, size_t line)
{
    assert(tokens);

    token temp_token              = {};
          temp_token.type         = NUMBER;
          temp_token.line         = line;
          temp_token.value.number = number;

    if(!array_push(tokens, &temp_token))
        return false;

    return true;
}


static bool push_ident(array* tokens, array* idents, char* position, size_t id_len, size_t line)
{
    assert(tokens);
    assert(idents);
    assert(position);

    token temp_token = {};
    temp_token.type  = IDENT;
    temp_token.line  = line;

    char** idents_data = (char** )idents->data;
    for(size_t i = 0; i < idents->size; i++)
    {
        if(!strncmp(position, idents_data[i], id_len) && id_len == strlen(idents_data[i]))
        {
            temp_token.value.ident = idents_data[i];
            break;
        }
    }

    if(!temp_token.value.ident)
    {
        char* temp_ident = (char* )calloc(id_len + 1, sizeof(char));
        if(!temp_ident)
            return false;

        memcpy(temp_ident, position, id_len);
        temp_ident[id_len]     = '\0';
        temp_token.value.ident = temp_ident;

        if(!array_push(idents, &temp_ident))
            return false;
    }
    
    if(!array_push(tokens, &temp_token))
        return false;
    
    return true;
}


static bool is_keyword_before(array* tokens)
{
    if(!tokens->size)
        return false;

    return ((*((token *)((char* )tokens->data + (tokens->size - 1) * sizeof(token)))).type == KEYWORD);
}


token* tokenize(const char* file_path, array* idents, array* tokens)
{
    char*  buf = nullptr;
    create_buf(file_path, &buf);

    if(!buf)
        return nullptr;

    array_ctor(tokens, sizeof(token ));
    array_ctor(idents, sizeof(char *));
 
    char*  iterator = buf;
    size_t lines    = 1;

    while(*iterator != '\0')
    {
        skip_spaces(&iterator, &lines);

        if(*iterator == '\0')
            break;

        int    keyword = 0;
        double number  = 0;
        size_t tok_len = 0;

        if(is_number(tokens, iterator, &number, &tok_len))
        {
            if(!push_number(tokens, number, lines))
            {
                free(buf);
                return nullptr;
            }
            iterator += tok_len;
            continue;
        }

        if(is_keyword(iterator, &keyword, &tok_len))
        {
            if(!push_keyword(tokens, keyword, lines))
            {
                free(buf);
                return nullptr;
            }
            iterator += tok_len;
            continue;
        }

        char* begin = iterator;

        while(!is_keyword(iterator, &keyword, &tok_len) && *iterator != '\0')
            iterator++;
            
        iterator--;
        while(isspace(*iterator))
            iterator--;
        iterator++;

        if(!push_ident(tokens, idents, begin, (size_t)(iterator - begin), lines))
        {
            free(buf);
            return nullptr;
        }
    }
    free(buf);

    if(!push_keyword(tokens, '$', lines))
        return nullptr;

    tokens_dump(tokens, "logfiles/token_dump.txt");

    return (token* )tokens->data;
}


static const char* keyword_string(int type)
{
    #define KEYWORD(name, keyword, ident)      \
                    else if (keyword == type)  \
                        return ident;                      

                    if (0) {} 
    #include "../res/KEYWORDS"
    #undef KEYWORD 

    return nullptr;
}

static int tokens_dump(const array* const tokens, const char* dump_file_path)
{
    assert(tokens);
    FILE* fp = fopen(dump_file_path, "w");
    if(!fp)
        return 0;
    
    dump_init(fp, tokens);
    fprintf(fp, "|====|==========|==========|========================================================================\n");

    for(size_t i = 0; i < tokens->size; i++)
    {
        token temp_token = *((token *)((char* )tokens->data + i * tokens->item_size));
        switch(temp_token.type)
        {
            case KEYWORD:
                fprintf(fp, "|%-4zu|LINE: %-4zu|KEYWORD   |VALUE:   %-15s\n"
                            "|====|==========|==========|========================================================================\n", i + 1, temp_token.line, keyword_string(temp_token.value.keyword));
                break;
            case NUMBER:
                fprintf(fp, "|%-4zu|LINE: %-4zu|NUMBER    |VALUE:   %-15lg\n"
                            "|====|==========|==========|========================================================================\n", i + 1, temp_token.line, temp_token.value.number);
                break;
            case IDENT:
                fprintf(fp, "|%-4zu|LINE: %-4zu|IDENT     |VALUE:   %-15s\n"
                            "|====|==========|==========|========================================================================\n", i + 1, temp_token.line, temp_token.value.ident);
                break;
            default:
                return 0;
        }
    }
    return 1;
}
