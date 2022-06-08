#include <math.h>
#include <unistd.h>

#include "../include/frontend/recursive_decent.h"

static       size_t iterator    = 0;
static const token* p_tokens    = nullptr;


static Node* GetGrammar();
static Node* GetFuncDef();
static Node* GetAssign();
static Node* GetFunction();
static Node* GetBlock();
static Node* GetOperation();
static Node* GetExpression();
static Node* GetCompare();
static Node* GetBoolean();
static Node* GetT();
static Node* GetP();
static Node* GetU();

static const char* keyword_string(int type);
static int    is_keyword(size_t position, const int name);
static char*  is_ident  (size_t position);
static double is_number (size_t position);
static size_t is_natural(double number);
static Node*  print_error(const char* waited);

static Node* create_oper(OperType type,  Node* lnode, Node* rnode);
static Node* create_ident(char*   ident, Node* lnode, Node* rnode);
static Node* create_num(double    num,   Node* lnode, Node* rnode);


static Node* print_error(const char* waited)
{
    fprintf(stderr, "Mistake at line %zu: waited \"%s\", in fact ", p_tokens[iterator].line, waited);  
    switch(p_tokens[iterator].type)                                                                             
    {                                                                                                           
        case KEYWORD:   
            fprintf(stderr, "KEYWORD \"%s\".\n", keyword_string(p_tokens[iterator].value.keyword)); 
            break;                                                                                  
        case IDENT:    
            fprintf(stderr, "IDENTIFICATOR \"%s\".\n", p_tokens[iterator].value.ident);             
            break;                                                                                  
        case NUMBER:    
            fprintf(stderr, "NUMBER \"%lg\".\n", p_tokens[iterator].value.number);                  
            break;                                                                                  
        default:        
            fprintf(stderr, "unpredictable error.\n");
    }
    return nullptr;
}

#define ass(X)  do { if(!X) return nullptr; } while(0)


//#ifdef DEBUG
//#define $$ fprintf(stderr, "%d\t%zu\n", __LINE__, iterator);
//#endif
/*#ifndef DEBUF
#define $$
#endif*/


static const char* keyword_string(int type)
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


static int is_keyword(size_t position, const int name)
{
    if(p_tokens[position].type == KEYWORD)
        if(p_tokens[position].value.keyword == name)
            return name;
    return 0;
}


static char* is_ident(size_t position)
{
    if(p_tokens[position].type == IDENT)
        return p_tokens[position].value.ident;
    return nullptr;
}


static double is_number(size_t position)
{
    if(p_tokens[position].type == NUMBER)
    {
        if(fabs(p_tokens[position].value.number) < EPSILON)
            return 0.1 * EPSILON;
        return p_tokens[position].value.number;
    }
    return 0;
}


static size_t is_natural(double number)
{
    if(number < 1 || fabs(number - (int)number) > EPSILON)
        return 0;
    
    return (size_t)number;
}


static Node* create_oper(OperType type, Node* lnode, Node* rnode)
{
    Node* new_node = (Node *)calloc(1, sizeof(Node));
    if(!new_node)
        return nullptr;

    new_node->Type            = OPER;
    new_node->Value.oper_type = type;
    new_node->Left            = lnode;
    new_node->Right           = rnode;

    return new_node;
}


static Node* create_ident(char* ident, Node* lnode, Node* rnode)
{
    Node* new_node = (Node *)calloc(1, sizeof(Node));
    if(!new_node)
        return nullptr;

    new_node->Type         = ID;
    new_node->Value.ident  = ident;
    new_node->Left         = lnode;
    new_node->Right        = rnode;

    return new_node;
}


static Node* create_num(double num, Node* lnode, Node* rnode)
{
    if(fabs(num) < EPSILON)
        num = 0;

    Node* new_node = (Node *)calloc(1, sizeof(Node));
    if(!new_node)
        return nullptr;

    new_node->Type         = NUM;
    new_node->Value.number = num;
    new_node->Left         = lnode;
    new_node->Right        = rnode;

    return new_node;
}



static Node* GetGrammar()
{
    Node* node = nullptr;

    while(!is_keyword(iterator, KEY_STOP) && (is_ident(iterator) || is_keyword(iterator, KEY_CONST)))
    {
        Node* new_node1 = GetAssign();
        ass(new_node1);
    
        Node* new_node2 = new_node1;
        while(new_node1->Left)
        {
            new_node1 = new_node1->Left;
        }
        new_node1->Left = node;
        node = new_node2;
    }
    while(!is_keyword(iterator, KEY_STOP) && is_keyword(iterator, KEY_DEF))
    {
        iterator++;

        Node* new_node  = create_oper(STATEMENT, node, GetFuncDef());
        ass(new_node->Right);
        node = new_node;        
    }
    if(is_keyword(iterator, KEY_CONST) || (is_ident(iterator) && is_keyword(iterator + 1, KEY_ASSIGN)) ||
                                          (is_ident(iterator) && is_keyword(iterator + 1, KEY_QOPEN )))
    {
        return print_error("assignment at the beginning");
    }
    if(!is_keyword(iterator, KEY_STOP)) 
    {
        return print_error("function defenition or global assignment");
    }

    return node;
}


static Node* GetFuncDef()
{
    Node* node = nullptr;

    if(is_ident(iterator) && is_keyword(iterator + 1, KEY_OPEN))
    {
        Node* new_node = create_oper(DEF, create_oper(FUNC, nullptr, nullptr), nullptr);
        new_node->Left->Left = create_ident(is_ident(iterator), nullptr, nullptr);

        iterator+=2;

        if(is_keyword(iterator, KEY_CLOSE))
        {
            iterator++;
        }
        else if(is_ident(iterator))
        {
            Node* param_node = create_oper(PARAM, nullptr, create_ident(is_ident(iterator), nullptr, nullptr));
            new_node->Left->Right = param_node;

            iterator++;

            while(is_keyword(iterator, KEY_COMMA) && is_ident(iterator + 1) && !is_keyword(iterator + 1, KEY_STOP))
            {
                param_node = create_oper(PARAM, nullptr, create_ident(is_ident(iterator + 1), nullptr, nullptr));
                param_node->Left = new_node->Left->Right;
                new_node->Left->Right = param_node;

                iterator+=2;
            }

            if(is_keyword(iterator, KEY_CLOSE))
                iterator++;
            else
                return print_error(")");
        }
        else
        {
            return print_error("parameters or void function");
        }

        if(is_keyword(iterator, KEY_BEGIN))
        {
            new_node->Right = GetBlock();
            ass(new_node->Right);
        }
        else
        {
            return print_error("block start {");
        }

        node = new_node;
    }
    else
    {
        return print_error("function identificator (...)");
    }
    
    return node;
}


static Node* GetAssign()
{
    Node* node     = nullptr;
    bool  is_const = false; 

    if(is_keyword(iterator, KEY_CONST))
    {
        is_const = true;
        iterator++;
    }

    if(is_ident(iterator))
    {
        if(is_keyword(iterator + 1, KEY_ASSIGN))
        {
            iterator+=2;

            Node* new_node = nullptr;
            if(is_const)
                new_node = create_oper(STATEMENT, nullptr, create_oper(ASSIGN, create_ident(is_ident(iterator - 2), create_oper(CONST, nullptr, nullptr), nullptr), GetExpression()));
            else
                new_node = create_oper(STATEMENT, nullptr, create_oper(ASSIGN, create_ident(is_ident(iterator - 2), nullptr, nullptr), GetExpression()));
            ass(new_node->Right->Right);
            
            node = new_node;
        }
        else if((bool)is_keyword(iterator + 1, KEY_QOPEN)  && (bool)is_number(iterator + 2) && (bool)is_keyword(iterator + 3, KEY_QCLOSE) 
             && (bool)is_keyword(iterator + 4, KEY_ASSIGN) && (bool)is_keyword(iterator + 5, KEY_BEGIN) && (bool)is_number(iterator + 6))
        {
            size_t mas_size  = is_natural(is_number(iterator + 2));
            int    mas_index = 0; 
            char*  mas_ident = is_ident(iterator);

            iterator+=7;

            if(!mas_size)
                return print_error("natural number");

            Node* new_node = nullptr;

            if(is_const)
                new_node = create_oper(STATEMENT, nullptr, create_oper(ASSIGN, create_ident(mas_ident, create_oper(CONST, nullptr, nullptr), create_num((double)mas_index, 
                                                  nullptr, nullptr)), create_num(is_number(iterator - 1), nullptr, nullptr)));
            else
                new_node = create_oper(STATEMENT, nullptr, create_oper(ASSIGN, create_ident(mas_ident, nullptr, create_num((double)mas_index, 
                                                  nullptr, nullptr)), create_num(is_number(iterator - 1), nullptr, nullptr)));
            node = new_node;

            mas_index++;

            if(is_keyword(iterator, KEY_END))
            {
                while(mas_index < (int)mas_size)
                {
                    if(is_const)
                        new_node->Left = create_oper(STATEMENT, nullptr, create_oper(ASSIGN, create_ident(mas_ident, create_oper(CONST, nullptr, nullptr), create_num((double)mas_index, 
                                                                nullptr, nullptr)), create_num(is_number(iterator - 1), nullptr, nullptr)));
                    else
                        new_node->Left = create_oper(STATEMENT, nullptr, create_oper(ASSIGN, create_ident(mas_ident, nullptr, create_num((double)mas_index, 
                                                                nullptr, nullptr)), create_num(is_number(iterator - 1), nullptr, nullptr)));
                    new_node = new_node->Left;

                    mas_index++;
                }
            }
            else
            {            
                while((bool)is_keyword(iterator, KEY_COMMA) && (bool)is_number(iterator + 1) && !(bool)is_keyword(iterator + 1, KEY_STOP))
                {
                    if(is_const)
                        new_node->Left = create_oper(STATEMENT, nullptr, create_oper(ASSIGN, create_ident(mas_ident, create_oper(CONST, nullptr, nullptr), create_num((double)mas_index, 
                                                                nullptr, nullptr)), create_num(is_number(iterator + 1), nullptr, nullptr)));
                    else
                        new_node->Left = create_oper(STATEMENT, nullptr, create_oper(ASSIGN, create_ident(mas_ident, nullptr, create_num((double)mas_index, 
                                                                nullptr, nullptr)), create_num(is_number(iterator + 1), nullptr, nullptr)));
                    new_node = new_node->Left;

                    iterator+=2;
                    mas_index++;
                }

                if(mas_index != (int)mas_size)
                    return print_error("same number elements as massive size");

            }
            if(!is_keyword(iterator, KEY_END))
                return print_error("}");

            iterator++;
        }
        else if(is_keyword(iterator + 1, KEY_QOPEN))
        {
            iterator+=2;

            Node* new_node = nullptr;
            if(is_const)
                new_node = create_oper(STATEMENT, nullptr, create_oper(ASSIGN, create_ident(is_ident(iterator - 2), create_oper(CONST, nullptr, nullptr), GetExpression()), nullptr));
            else
                new_node = create_oper(STATEMENT, nullptr, create_oper(ASSIGN, create_ident(is_ident(iterator - 2), nullptr, GetExpression()), nullptr));

            ass(new_node->Right->Left->Right);

            if(!is_keyword(iterator, KEY_QCLOSE) || !is_keyword(iterator + 1, KEY_ASSIGN))
                return print_error("] =");

            iterator+=2;

            new_node->Right->Right = GetExpression();
            ass(new_node->Right->Right);
            node = new_node;
        }
        else
        {
            return print_error("variable or massive assignment");
        }
    }
    else
    {
        return print_error("identificator");
    }

    if(!is_keyword(iterator, KEY_SEMICOL))
        return print_error(";");

    iterator++;
    return node;
}


static Node* GetFunction()
{
    Node* node = nullptr;

    if(is_ident(iterator) && is_keyword(iterator + 1, KEY_OPEN))
    {
        Node* new_node = create_oper(CALL, create_ident(is_ident(iterator), nullptr, nullptr), nullptr);

        iterator+=2;

        if(is_keyword(iterator, KEY_CLOSE))
        {
            iterator++;
        }
        else
        {
            Node* param_node  = create_oper(PARAM, nullptr, GetExpression());
            ass(param_node->Right);
            new_node->Right = param_node;

            while(is_keyword(iterator, KEY_COMMA) && !is_keyword(iterator, KEY_STOP))
            {
                iterator++;
                
                param_node = create_oper(PARAM, nullptr, GetExpression());
                ass(param_node->Right);
                param_node->Left = new_node->Right;
                new_node->Right = param_node;
            }
            if(is_keyword(iterator, KEY_CLOSE))
                iterator++;
            else
                return print_error(")");
        }

        node = new_node;
    }
    else
    {
        return print_error("function (...)");
    }
    return node;
}


static Node* GetBlock()
{
    Node* node = nullptr;

    if(is_keyword(iterator, KEY_BEGIN))
    {
        iterator++;
        
        do
        {
            Node* new_node  = GetOperation();
            ass(new_node);

            if(is_keyword(iterator - 1, KEY_SEMICOL) && is_keyword(iterator - 2, KEY_END))
            {
                Node* new_node1 = new_node;
                while(new_node1->Left)
                {
                    new_node1 = new_node1->Left;
                }
                new_node1->Left = node;
                node = new_node;
            }
            else
            {
                new_node->Left = node;
                node = new_node;
            }
        }
        while(!is_keyword(iterator, KEY_END) && !is_keyword(iterator, KEY_STOP));

        if(!is_keyword(iterator, KEY_END))
            return print_error("}");

        iterator++;
    }
    else
    {
        node = GetOperation();
    }
    return node;
}


static Node* GetOperation()
{
    Node* node = nullptr;

    if(is_keyword(iterator, KEY_RETURN))
    {
        iterator++;

        Node* new_node  = create_oper(RET, nullptr, GetExpression());
        if(!new_node->Right)
            return print_error("expression");

        if(!is_keyword(iterator, KEY_SEMICOL))
            return print_error(";");

        iterator++;

        node = create_oper(STATEMENT, nullptr, new_node);
    }
    else if(is_keyword(iterator, KEY_IF) && is_keyword(iterator + 1, KEY_OPEN))
    {
        iterator+=2;

        Node* new_node  = create_oper(IF, GetExpression(), create_oper(DECISION, nullptr, nullptr));
        ass(new_node->Left);

        if(!is_keyword(iterator, KEY_CLOSE))
            return print_error(")");

        iterator++;

        new_node->Right->Left = GetBlock();
        ass(new_node->Right->Left);

        if(is_keyword(iterator, KEY_ELSE))
        {
            iterator++;
            new_node->Right->Right = GetBlock();
            ass(new_node->Right->Right);
        }

        node = create_oper(STATEMENT, nullptr, new_node);
    }
    else if(is_keyword(iterator, KEY_WHILE) && is_keyword(iterator + 1, KEY_OPEN))
    {
        iterator+=2;

        Node* new_node  = create_oper(WHILE, GetExpression(), nullptr);
        ass(new_node->Left);

        if(!is_keyword(iterator, KEY_CLOSE))
            return print_error(")");

        iterator++;

        new_node->Right = GetBlock();
        ass(new_node->Right);

        node = create_oper(STATEMENT, nullptr, new_node);
    }
    else if(is_keyword(iterator, KEY_OUT) && is_keyword(iterator + 1, KEY_OPEN))
    {
        iterator+=2;

        Node* new_node = create_oper(PRINT, nullptr, GetExpression());
        ass(new_node->Right);

        if(!is_keyword(iterator, KEY_CLOSE) || !is_keyword(iterator + 1, KEY_SEMICOL))
        {
            return print_error(");");
        }

        iterator+=2;

        node = create_oper(STATEMENT, nullptr, new_node);
    }
    else if(is_ident(iterator) && is_keyword(iterator + 1, KEY_OPEN))
    {
        node = create_oper(STATEMENT, nullptr, GetFunction());
        ass(node->Right);

        if(!is_keyword(iterator, KEY_SEMICOL))
            return print_error(";"); 

        iterator++;
    }
    else if( (is_ident(iterator) && (is_keyword(iterator + 1, KEY_ASSIGN) || is_keyword(iterator + 1, KEY_QOPEN))) || is_keyword(iterator, KEY_CONST) )
    {
        node = GetAssign();
        ass(node);
    }
    else
    {
        return print_error("operation");
    }
    return node;
}


static Node* GetExpression()
{
    Node* node = nullptr;

    if(is_keyword(iterator, KEY_NOT))
    {
        iterator++;

        node = create_oper(NOT, nullptr, GetCompare());
        ass(node->Right);
    }
    else
    {
        node = GetCompare();
        ass(node);
    }

    while((is_keyword(iterator, KEY_OR) || is_keyword(iterator, KEY_AND)) && !is_keyword(iterator, KEY_STOP))
    {
        iterator++;

        Node* new_node =  nullptr;
        if(is_keyword(iterator, KEY_NOT))
        {
            iterator++;

            if(is_keyword(iterator - 2, KEY_OR))
                new_node = create_oper(OR,  node, create_oper(NOT, nullptr, GetCompare()));
            else if(is_keyword(iterator - 2, KEY_AND))
                new_node = create_oper(AND, node, create_oper(NOT, nullptr, GetCompare()));
            ass(new_node->Right->Right);
        }
        else
        {
            if(is_keyword(iterator - 1, KEY_OR))
                new_node = create_oper(OR,  node, GetCompare());
            else if(is_keyword(iterator - 1, KEY_AND))
                new_node = create_oper(AND, node, GetCompare());
            ass(new_node->Right);
        }

        node = new_node;
    }
    return node;
}


static Node* GetCompare()
{
    Node* node = GetBoolean();
    ass(node);

    if(is_keyword(iterator, KEY_GREAT)  || is_keyword(iterator, KEY_LOW)   || is_keyword(iterator, KEY_GEQUAL) || 
       is_keyword(iterator, KEY_LEQUAL) || is_keyword(iterator, KEY_EQUAL) || is_keyword(iterator, KEY_NEQUAL))
    {
        iterator++;

        Node* new_node = nullptr;
        if(is_keyword(iterator - 1, KEY_GREAT))
            new_node = create_oper(GR, node, GetBoolean());
        else if(is_keyword(iterator - 1, KEY_LOW))
            new_node = create_oper(LOW, node, GetBoolean());
        else if(is_keyword(iterator - 1, KEY_GEQUAL))
            new_node = create_oper(NLOW, node, GetBoolean());
        else if(is_keyword(iterator - 1, KEY_LEQUAL))
            new_node = create_oper(NGR, node, GetBoolean());
        else if(is_keyword(iterator - 1, KEY_EQUAL))
            new_node = create_oper(EQUAL, node, GetBoolean());
        else if(is_keyword(iterator - 1, KEY_NEQUAL))
            new_node = create_oper(NEQUAL, node, GetBoolean());
        ass(new_node->Right);

        node = new_node;
    }
    return node;
}


static Node* GetBoolean()
{
    Node* node = GetT();
    ass(node);

    while((is_keyword(iterator, KEY_ADD) || is_keyword(iterator, KEY_SUB)) && !is_keyword(iterator, KEY_STOP))
    {
        iterator++;

        Node* new_node = nullptr;
        if(is_keyword(iterator - 1, KEY_ADD))
            new_node = create_oper(ADD, node, GetT());
        else if(is_keyword(iterator - 1, KEY_SUB))
            new_node = create_oper(SUB, node, GetT());
        ass(new_node->Right);

        node = new_node;
    }

    return node;
}


static Node* GetT()
{
    Node* node = GetP();
    ass(node);

    while((is_keyword(iterator, KEY_MUL) || is_keyword(iterator, KEY_DIV)) && !is_keyword(iterator, KEY_STOP))
    {
        iterator++;

        Node* new_node = nullptr;
        if(is_keyword(iterator - 1, KEY_MUL))
            new_node = create_oper(MUL, node, GetT());
        else if(is_keyword(iterator - 1, KEY_DIV))
            new_node = create_oper(DIV, node, GetT());
        ass(new_node->Right);
        ass(new_node->Right);

        node = new_node;
    }
    
    return node;
}


static Node* GetP()
{
    Node* node = GetU();
    ass(node);

    while(is_keyword(iterator, KEY_POW) && !is_keyword(iterator, KEY_STOP))
    {
        iterator++;

        Node* new_node = create_oper(POW, node, GetU());
        ass(new_node->Right);

        node = new_node;
    }

    return node;
}


static Node* GetU()
{
    Node* node = nullptr;
    if(is_keyword(iterator, KEY_OPEN))
    {
        iterator++;

        node = GetExpression();
        ass(node);

        if(!is_keyword(iterator, KEY_CLOSE))
            return print_error(")");

        iterator++;
    }
    else if(is_keyword(iterator, KEY_SIN) && is_keyword(iterator + 1, KEY_OPEN))
    {
        iterator+=2;

        node = create_oper(SIN, nullptr, GetExpression());
        ass(node->Right);

        if(!is_keyword(iterator, KEY_CLOSE))
            return print_error(")");

        iterator++;
    }
    else if(is_keyword(iterator, KEY_COS) && is_keyword(iterator + 1, KEY_OPEN))
    {
        iterator+=2;

        node = create_oper(COS, nullptr, GetExpression());
        ass(node);

        if(!is_keyword(iterator, KEY_CLOSE))
            return print_error(")");

        iterator++;
    }
    else if(is_keyword(iterator, KEY_INT) && is_keyword(iterator + 1, KEY_OPEN))
    {
        iterator+=2;

        node = create_oper(INT, nullptr, GetExpression());
        ass(node);

        if(!is_keyword(iterator, KEY_CLOSE))
            return print_error(")");

        iterator++;
    }
    else if(is_keyword(iterator, KEY_IN) && is_keyword(iterator + 1, KEY_OPEN))
    {
        iterator+=2;

        node = create_oper(SCAN, nullptr, nullptr);

        if(!is_keyword(iterator, KEY_CLOSE))
            return print_error(")");

        iterator++;
    }
    else if(is_ident(iterator) && is_keyword(iterator + 1, KEY_QOPEN))
    {
        iterator+=2;

        node = create_ident(is_ident(iterator - 2), nullptr, GetExpression());
        ass(node);

        if(!is_keyword(iterator, KEY_QCLOSE))
            return print_error("]");

        iterator++;
    }
    else if((bool)is_number(iterator))
    {
        node = create_num(is_number(iterator), nullptr, nullptr);
        iterator++;
    }
    else if(is_ident(iterator) && !is_keyword(iterator + 1, KEY_OPEN))
    {
        node = create_ident(is_ident(iterator), nullptr, nullptr);
        iterator++;
    }
    else if(is_ident(iterator) && is_keyword(iterator + 1, KEY_OPEN))
    {
        node = GetFunction();
    }
    else
    {
        return print_error("unary expression");
    }
    
    return node;
}


Tree* create_tree(array* tokens)
{
    assert(tokens);

    p_tokens = (token* )tokens->data;
    Tree* p_tree = nullptr;

    tree_ctor(&p_tree);
    Node* temp_node = GetGrammar();

    if(!temp_node)
    {
        tree_dtor(p_tree);
        return nullptr;
    }

    p_tree->root = temp_node;
    p_tree->size = count_tree_size(temp_node);

    tree_dump(p_tree);
    tree_write(p_tree, "tree");
    return p_tree;
}
