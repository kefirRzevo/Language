#include "../include/symtable.h"

static size_t N_DUMPS = 0;
static FILE*  LOGFILE = nullptr;
static FILE*  dump_init();



sym_table* create_sym_table(sym_table* table, int init_shift)
{
    assert(table);

    if(!array_ctor(&table->functions, sizeof(func)))
        return nullptr;
    if(!array_ctor(&table->scoop_levels, sizeof(array)))
        return nullptr;

    table->shift = init_shift;

    return table;
}

sym_table* delete_sym_table(sym_table* table)
{
    assert(table);

    for(size_t i = 0; i < table->scoop_levels.size; i++)
    {
        array temp_arr = {};
        array_pop(&table->scoop_levels, &temp_arr);
        array_dtor(&temp_arr, 0);
    }

    array_dtor(&table->functions, 0);
    table->shift = 0;

    return table;
}

array* create_scoop_level(sym_table* table)
{
    assert(table);
$$
    array temp_arr = {};
    if(!array_ctor(&temp_arr, sizeof(var)))
        return nullptr;$$
    if(!array_push(&table->scoop_levels, &temp_arr))
        return nullptr;
$$
    return (array* )((char* )table->scoop_levels.data + (table->scoop_levels.size - 1) * sizeof(array));
}

array* create_function_list(sym_table* table)
{
    assert(table);

    array temp_arr = {};
    if(!array_ctor(&temp_arr, sizeof(func)))
        return nullptr;

    memcpy(&table->functions, &temp_arr, sizeof(array));
    return &table->functions;
}


array* delete_scoop_level(sym_table* table)
{
    assert(table);

    array temp_arr = {};
    
    if(!array_pop(&table->scoop_levels, &temp_arr))
        return nullptr;

    for(size_t i = 0; i < temp_arr.size; i++)
    {
        var temp_var = {};
        memcpy(&temp_var, (char* )temp_arr.data + i * sizeof(var), sizeof(var));
        table->shift -= temp_var.size;
    }

    array_dtor(&temp_arr, 0);
    return nullptr;
}

func* add_function(sym_table* table, Node* func_node)
{
    assert(table);
    assert(func_node);

    if(!is_operator(func_node, FUNC))
        return nullptr;

    if(!is_ident(func_node->Left))
        return nullptr;

    Node* node = func_node;
    func temp_func = {};

    temp_func.name     = is_ident(func_node->Left);
    temp_func.declared = func_node;

    func_node = func_node->Right;
    while(func_node)
    {
        temp_func.n_params++;
        func_node = func_node->Left;
    }
    
    if(!array_push(&table->functions, &temp_func))
        return nullptr;
    
    return (func* )((char* )table->functions.data + (table->functions.size - 1) * sizeof(func));
}

var* add_variable(sym_table* table, Node* var_node)
{
    assert(table);
    assert(var_node);

    if(!is_ident(var_node))
        return nullptr;
$$
    var temp_var = {};
    temp_var.adress      = table->shift;
    temp_var.name        = is_ident(var_node);
    temp_var.declared    = var_node;
    temp_var.is_const    = false;
    temp_var.size        = 1;
$$    
    if(is_operator(var_node->Left, CONST))
        temp_var.is_const = true;
$$    
    if(is_natural(var_node->Right))
        temp_var.size = get_number(var_node->Right) + 1;
    else if(var_node->Right)
        return nullptr;
$$
    table->shift+=temp_var.size;
$$
    array temp_arr = {};
    memcpy(&temp_arr, (char* )table->scoop_levels.data + (table->scoop_levels.size - 1) * sizeof(array), sizeof(array));
$$    
    if(!array_push(&temp_arr, &temp_var))
        return nullptr;

    memcpy((char* )table->scoop_levels.data + (table->scoop_levels.size - 1) * sizeof(array), &temp_arr, sizeof(array));
$$
    return (var* )((char* )temp_arr.data + (temp_arr.size - 1) * sizeof(var));
}

func* find_function(sym_table* table, Node* func_node)
{
    assert(table);
$$
    if(!func_node)
        return nullptr;
$$
    if(!is_operator(func_node, CALL))
        return nullptr;
$$
    if(!is_ident(func_node->Left))
        return nullptr;
$$
    for(size_t i = 0; i < table->functions.size; i++)
    {
        func temp_func = {};
        memcpy(&temp_func, (char* )table->functions.data + i * sizeof(func), sizeof(func));
        $$
        if(!strcmp(temp_func.name, is_ident(func_node->Left)))
        { $$
            size_t n_params = 0;
            func_node = func_node->Right;

            while (func_node)
            {
                if(!is_operator(func_node, PARAM))
                    return nullptr;

                n_params++;
                func_node = func_node->Left;
            }
$$
            if(temp_func.n_params == n_params)
                return (func* )((char* )table->functions.data + i * sizeof(func));
        }
    }
$$
    return nullptr;
}

var* find_variable(sym_table* table, Node* var_node)
{
    assert(table);
    
    if(!var_node)
        return nullptr;

    if(!is_ident(var_node))
        return nullptr;

    for(size_t i = 0; i < table->scoop_levels.size; i++)
    {
        array temp_arr = {};
        memcpy(&temp_arr, (char* )table->scoop_levels.data + i * sizeof(array), sizeof(array));

        for(size_t j = 0; j < temp_arr.size; j++)
        {
            var temp_var = {};
            memcpy(&temp_var, (char* )temp_arr.data + j * sizeof(var), sizeof(var));

            if(!strcmp(temp_var.name, is_ident(var_node)))
                return (var* )((char* )temp_arr.data + j * sizeof(var));
        }
    }

    return nullptr;
}

size_t count_shift_size(sym_table* table)
{
    assert(table);

    size_t shift   = table->shift;
    array temp_arr = {};
    memcpy(&temp_arr, table->scoop_levels.data, sizeof(array));

    for(size_t i = 0; i < temp_arr.size; i++)
    {
        var temp_var = {};
        memcpy(&temp_var, (char* )temp_arr.data + i * sizeof(var), sizeof(var));
        shift -= temp_var.size;
    }

    return shift;
}

static FILE* dump_init()
{ 
    const time_t        seconds = time(nullptr);
    const tm     *const date    = localtime(&seconds);
                        LOGFILE = fopen("logfiles/symtable_dump.txt", "w");
    if(!LOGFILE)
        return nullptr;
$$
    char day[16] = "";
$$
    switch (date->tm_wday)
    {
        case 0: strcpy(day, "Sunday"   );   break;
        case 1: strcpy(day, "Monday"   );   break;
        case 2: strcpy(day, "Tuesday"  );   break;
        case 3: strcpy(day, "Wednesday");   break;
        case 4: strcpy(day, "Thursday" );   break;
        case 5: strcpy(day, "Friday"   );   break;
        case 6: strcpy(day, "Saturday" );   break;
    }
$$
    fprintf(LOGFILE,
    "SYMBOL TABLE LOGFILE\n"
    "Time: %02d:%02d:%02d\n"
    "Day:  %s\n",
    date->tm_hour, date->tm_min, date->tm_sec, day);   
$$
    return LOGFILE;                               
}

sym_table* dump_sym_table(sym_table* table)
{
    assert(table);
$$  
    if(!LOGFILE) 
        if(!dump_init())
            return nullptr;
$$ 
    fprintf(LOGFILE,
    "\n\nDUMP %zu\n"
    "==================================================================\n"
    "|VARIABLE /  |NODE            |SIZE /          |ADRESS  |CONST   |\n"  
    "|FUNCTION    |                |NUMBER PARAMS   |        |        |\n"
    "==================================================================\n",
    ++N_DUMPS);
$$
    fprintf(LOGFILE,
    "|                      CURRENT SHIFT = %-4d                      |\n"
    "==================================================================\n",
    table->shift);
$$
    if(table->functions.size != 0)
    {
        fprintf(LOGFILE,
        "|                         FUNCTION ARRAY                         |\n"
        "==================================================================\n");
$$  
        for(size_t i = 0; i < table->functions.size; i++)
        {
            func temp_func = {};
            memcpy(&temp_func, (char* )table->functions.data + i * sizeof(func), sizeof(func));

            fprintf(LOGFILE,
            "|%-12s|%-16p|%-16zu|        |        |\n"
            "==================================================================\n",
            temp_func.name, temp_func.declared, temp_func.n_params);
        }
    }
$$
    if(table->scoop_levels.size != 0)
    {
        fprintf(LOGFILE,
        "|                         VARIABLE ARRAY                         |\n"
        "==================================================================\n");
$$
        for(size_t i = 0; i < table->scoop_levels.size; i++)
        {
            fprintf(LOGFILE,
            "|                         %-2zu SCOOP LEVEL                         |\n"
            "==================================================================\n", i);
$$
            array temp_arr = {};
            memcpy(&temp_arr, (char* )table->scoop_levels.data + i * sizeof(array), sizeof(array));
$$
            for(size_t j = 0; j < temp_arr.size; j++)
            {
                var temp_var = {};
                memcpy(&temp_var, (char* )temp_arr.data + j * sizeof(var), sizeof(var));

                fprintf(LOGFILE,
                "|%-12s|%-16p|%-16zu|%-8d|%-8s|\n"
                "==================================================================\n",
                temp_var.name, temp_var.declared, temp_var.size, temp_var.adress, temp_var.is_const ? "true" : "false");
            }
        }
    }
$$  
    return table;
}
