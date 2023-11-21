#include "compiler.h"
#include "../include/symtable.h"


FILE* file = nullptr;
const char* MAIN = "main";

static const char*  MAS_REG = "rsi";
static const char*  POS_REG = "rsp";
static const char*  RET_REG = "rax";
static const char*  DUL_REG = "rbx";

static const size_t INIT_SHIFT = 0;
static const size_t BUF_SIZE   = 64; 
static char  BUFFER[BUF_SIZE]  = {0};


static Node* get_global_vars    (Node* node, sym_table* table);
static Node* get_defined_funcs  (Node* node, sym_table* table);

static Node* compile_call      (Node* node, sym_table* table);
static Node* compile_while     (Node* node, sym_table* table);
static Node* compile_define    (Node* node, sym_table* table);
static Node* compile_assign    (Node* node, sym_table* table);
static Node* compile_expression(Node* node, sym_table* table);
static Node* compile_statement (Node* node, sym_table* table);
static Node* compile_return    (Node* node, sym_table* table);
static Node* compile_if        (Node* node, sym_table* table);


static Node* find_main(sym_table* table);
static const char* write_shift    (int adress);
static const char* write_to_memory(int adress);
static const char* write_number   (double number);
static       void  write_msg      (const char* msg);
static       void  write_label    (const char* label);
static       FILE* open_file      (const char* filepath);
static const char* create_label   (const char* name, void* ptr);
static       Node* syntax_error   (const char* waited, Node* node);



#define DEF_CMD(cmd_name, cmd_key, arguments_number, code)      \
        static inline void cmd_name(const char* msg = nullptr);

#include "../../Processor/cpu/commands.txt"
#undef DEF_CMD


Tree* compile_tree(const char* filepath, Tree* p_tree)
{
    assert(p_tree);
    assert(filepath);
$$    
    if(!open_file(filepath))
        return nullptr;
$$
    sym_table table; 
    create_sym_table(&table, INIT_SHIFT);
$$
    push(write_number((double)INIT_SHIFT));
    pop(POS_REG);
$$
    if(!create_scoop_level(&table))
        return nullptr;
$$
    if(!get_global_vars(p_tree->root, &table))
        return nullptr;
$$
    if(!get_defined_funcs(p_tree->root, &table))
        return nullptr;

    if(!find_main(&table))
        return (Tree* )syntax_error("main function without params", nullptr);
$$
    call("main");
    hlt();
$$
    if(!compile_define(p_tree->root, &table))
        return nullptr;
$$
    dump_sym_table(&table);
    delete_sym_table(&table);
$$
    return p_tree;
}

static Node* get_global_vars(Node* node, sym_table* table)
{
    assert(table);
    assert(node);
$$
    if(!is_operator(node, STATEMENT))
        return syntax_error("statement", node);
$$
    if(node->Left)
        if(!get_global_vars(node->Left, table))
            return nullptr; 
$$
    if(is_operator(node->Right, ASSIGN))
        if(!compile_assign(node->Right, table))
            return nullptr;
$$       
    return node;
}

static Node* get_defined_funcs(Node* node, sym_table* table)
{
    assert(node);
    assert(table);
$$
    if(node->Left)
        if(!get_defined_funcs(node->Left, table))
            return nullptr;
$$
    if(is_operator(node->Right, DEF))
        if(!add_function(table, node->Right->Left))
            return nullptr;
$$
    return node;
}

static Node* compile_statement(Node* node, sym_table* table)
{
    assert(node);
    assert(table);
$$
    if(!is_operator(node, STATEMENT))
        return syntax_error("statement", node);
$$
    if(node->Left)
        if(!compile_statement(node->Left, table))
            return nullptr;
$$
    if(node->Right->Type != OPER)
        return nullptr;
$$
    switch(node->Right->Value.oper_type)
    {
        case ASSIGN:$$
            return compile_assign(node->Right, table);
        
        case CALL:$$
            if(!compile_call(node->Right, table))
                return nullptr;
            pop(DUL_REG);$$
            return node;

        case PRINT:$$
            if(!compile_expression(node->Right->Right, table))
                return nullptr;
            out();$$
            return node;

        case IF:$$
            return compile_if(node->Right, table);

        case WHILE:$$
            return compile_while(node->Right, table);
        
        case RET:$$
            return compile_return(node->Right, table);

        default:$$
            return syntax_error("correct operator", node->Right);
    }
}

static Node* compile_assign(Node* node, sym_table* table)
{
    assert(node);
    assert(table);
$$
    if(!is_operator(node, ASSIGN))
        return nullptr;
$$
    if(!compile_expression(node->Right, table))
        return nullptr;
$$
    var* found = find_variable(table, node->Left);
$$
    if(found)
    {
        if(!((found->declared->Right && node->Left->Right) || (!found->declared->Right && !node->Left->Right)))
            return syntax_error("not redefenition", node->Left);
$$
        if(found->is_const)
            return syntax_error("not assigning const variables", node->Left);
    }
$$
    if(!found)
    {$$
        found = add_variable(table, node->Left);
        if(!found)
            return syntax_error("natural number in initializing", node->Left->Right);
    }
$$
    if(node->Left->Right)
    {$$
        if(!compile_expression(node->Left->Right, table))
            return nullptr;
$$
        pop(MAS_REG);
        pop(write_shift(found->adress));
    }
    else
    {$$
        pop(write_to_memory(found->adress));
    }
$$
    return node;
}

static Node* compile_define(Node* node, sym_table* table)
{
    assert(node);
    assert(table);
$$
    if(node->Left)
        if(!compile_define(node->Left, table))
            return nullptr;
$$
    if(!is_operator(node->Right, DEF))
        return syntax_error("function defenition", node->Right);
$$
    if(!create_scoop_level(table))
        return nullptr;
$$
    Node* def_node   = node->Right;
    Node* param_node = def_node->Left->Right;
$$
    write_label(def_node->Left->Left->Value.ident);
$$
    while(param_node)
    {$$
        if(find_variable(table, param_node->Right))
            return syntax_error("not conflicing names", param_node->Right);
$$
        var* temp_param = add_variable(table, param_node->Right);
        pop(write_to_memory(temp_param->adress));
        param_node = param_node->Left;
    }
$$
    if(!compile_statement(def_node->Right, table))
        return nullptr;
$$
    dump_sym_table(table);
    delete_scoop_level(table);
    ret();
$$
    return node;
}

static Node* compile_call(Node* node, sym_table* table)
{
    assert(node);
    assert(table);
$$
    if(!is_operator(node, CALL))
        return nullptr;
$$
    func* temp_func = find_function(table, node);
    if(!temp_func)
        return syntax_error("defined function", node->Left);
$$
    Node* param_node = node->Right;
    size_t n_params = 0;
    while(param_node)
    {$$
        if(!compile_expression(param_node->Right, table))
            return nullptr;
$$
        param_node = param_node->Left;
        n_params++;
    }
$$
    if(n_params != temp_func->n_params)
        return syntax_error("as many params as in defined function", node->Left);
$$
    size_t shift = count_shift_size(table);
    
    push(POS_REG);
    push(write_number((double)shift));
    add();
    pop(POS_REG);

    call(temp_func->name);
    push(RET_REG);

    push(write_number((double)shift));
    push(POS_REG);
    sub();
    pop(POS_REG);
$$
    return node;
}

static Node* compile_return(Node* node, sym_table* table)
{
    assert(node);
    assert(table);
$$
    if(!is_operator(node, RET))
        return nullptr;
$$
    if(!compile_expression(node->Right, table))
        return nullptr;
$$
    pop(RET_REG);
    ret();
$$
    return node;
}

static Node* compile_expression(Node* node, sym_table* table)
{
    assert(node);
    assert(table);
$$
    if(is_operator(node, CALL))
        return compile_call(node, table);
$$
    if(node->Right)
        if(!compile_expression(node->Right, table))
            return nullptr;
$$
    if(node->Left)
        if(!compile_expression(node->Left, table))
            return nullptr;
$$
    var* temp_var = nullptr;
$$
    switch(node->Type)
    {
        case ID:$$
            temp_var = find_variable(table, node);$$
            if(!temp_var)
            {$$
                return syntax_error("variable to be found", node);
            }
            else if(!node->Right)
            {
                push(write_to_memory(temp_var->adress));
            }
            else
            {$$
                pop(MAS_REG);
                push(write_shift(temp_var->adress));
            }
            return node;

        case NUM:$$
            push(write_number(get_number(node)));
            return node;

        case OPER:$$
            switch(node->Value.oper_type)
            {
                case ADD:     add();    return node;
                case SUB:     sub();    return node;
                case MUL:     mul();    return node;
                case DIV:     div();    return node;
                case POW:     pow_();   return node;
                case SIN:     sin_();   return node;
                case COS:     cos_();   return node;
                case OR:      or_();    return node;
                case AND:     and_();   return node;
                case GR:      gr();     return node;
                case LOW:     low();    return node;
                case NGR:     ngr();    return node;
                case NLOW:    nlow();   return node;
                case EQUAL:   equ();    return node;
                case NEQUAL:  nequ();   return node;
                case NOT:     not_();   return node;
                case INT:     int_();   return node;
                case SCAN:    in();     return node;
                default:                return nullptr;
            }
        
        default: return nullptr;
    }
}

static Node* compile_if(Node* node, sym_table* table)
{
    assert(node);
    assert(table);
$$
    if(!is_operator(node, IF))
        return nullptr;
$$
    if(!compile_expression(node->Left, table))
        return nullptr;
$$
    push("0");
    jne(create_label("true", node));
$$
    if(node->Right->Right)
    {$$
        if(!create_scoop_level(table))
            return nullptr;
$$
        write_msg(";false\n");
$$
        if(!compile_statement(node->Right->Right, table))
            return nullptr;
$$
        delete_scoop_level(table);
    }
$$
    jmp(create_label("continue", node));
    write_label(create_label("true", node));

    if(!node->Right->Left)
        return syntax_error("if decision", node->Left);
$$
    if(!create_scoop_level(table))
        return nullptr;
$$
    if(!compile_statement(node->Right->Left, table))
        return nullptr;
$$
    dump_sym_table(table);
    delete_scoop_level(table);
$$
    write_label(create_label("continue", node));

    return node;
}

static Node* compile_while(Node* node, sym_table* table)
{
    assert(node);
    assert(table);
$$
    if(!is_operator(node, WHILE))
        return nullptr;
$$
    write_label(create_label("again", node));
$$
    if(!compile_expression(node->Left, table))
        return nullptr;
$$
    push("0");
    je(create_label("continue", node));

    if(!create_scoop_level(table))
        return nullptr;
$$
    if(!compile_statement(node->Right, table))
        return nullptr;   
$$
    dump_sym_table(table);
    delete_scoop_level(table);
 
    jmp(create_label("again", node));
    write_label(create_label("continue", node));
$$
    return node;
}


static Node* syntax_error(const char* waited, Node* node)
{
    assert(waited);
$$
    fprintf(stderr, "Mistake: waited \"%s\"", waited);  

    if(!node)
        return nullptr;

    switch(node->Type)                                                                             
    {                                                                                                           
        case OPER:$$   
            fprintf(stderr, ", in fact OPERATOR \"%s\".\n", get_oper_string(node)); 
            break;                                                                                  
        case ID:$$    
            fprintf(stderr, ", in fact IDENTIFICATOR \"%s\".\n", node->Value.ident);             
            break;                                                                                  
        case NUM:$$    
            fprintf(stderr, ", in fact NUMBER \"%lg\".\n", get_number(node));                  
            break;                                                                                  
        default:        
            fprintf(stderr, "Unpredictable error.\n");
    }
    return nullptr;
}

static FILE* open_file(const char* filepath)
{
    assert(filepath);
$$
    file = fopen(filepath, "w");
    if(!file)
        return nullptr;
$$
    return file;
}

#define DEF_CMD(cmd_name, cmd_key, arguments_number, code)  \
    static inline void cmd_name(const char* msg)            \
    {                                                       \
            if(msg)                                         \
                fprintf(file, "%s %s\n", #cmd_name, msg);   \
            else                                            \
                fprintf(file, "%s\n", #cmd_name);           \
    }

#include "../../Processor/cpu/commands.txt"
#undef DEF_CMD

static const char* write_to_memory(int adress)
{
    snprintf(BUFFER, BUF_SIZE, "[%s + %d]", POS_REG, adress);
    return BUFFER;
}

static const char* write_shift(int adress)
{
    snprintf(BUFFER, BUF_SIZE, "[%s + %d + %s]", POS_REG, adress, MAS_REG);
    return BUFFER;
}

static const char* write_number(double number)
{
    snprintf(BUFFER, BUF_SIZE, "%lg", number);
    return BUFFER;
}

static void write_label(const char* label)
{
    fprintf(file, "%s:\n", label);
}

static void write_msg(const char* msg)
{
    fprintf(file, "%s", msg);
}

static const char* create_label(const char* name, void* ptr)
{
    snprintf(BUFFER, BUF_SIZE, "%s_%p", name, ptr);
    return BUFFER;
}

static Node* find_main(sym_table* table)
{
    assert(table);

    for(size_t i = 0; i < table->functions.size; i++)
    {
        func temp_func = {};
        memcpy(&temp_func, (char* )table->functions.data + i * sizeof(func), sizeof(func));
        if(!strcmp(temp_func.name, MAIN) && !temp_func.n_params)
            return temp_func.declared;
    }

    return nullptr;
}
