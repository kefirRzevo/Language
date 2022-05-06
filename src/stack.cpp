#include "../include/stack.h" 


static FILE *LOGFILE       = nullptr;
bool STATUS_OPEN_LOG_FILE  = false;


static void     realloc_stack(stack *const p_stack, size_t capacity);

static uint64_t verify_stack(stack *const p_stack);

static void     check_stack(stack* const p_stack, const char* const stack_name, Functions current_cmd, uint64_t error, item_t* p_element, item_t element);

static void logfile_init();

static void open_log_file();


#ifdef StackHashProtect
static hash_t   QHash(const void *const data, const size_t data_size);

static hash_t   ResultHash(const stack *const p_stack);
#endif


static void open_log_file()
{           
    if(STATUS_OPEN_LOG_FILE == false)           
        logfile_init();                         
    STATUS_OPEN_LOG_FILE = true;
}


static void logfile_init()
{
    const time_t        seconds = time(nullptr);
    const tm     *const date    = localtime(&seconds);
                        LOGFILE = fopen("logfiles/stk_logfile.txt", "w");
    assert(LOGFILE);

    char day[16] = "";

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

    fprintf(LOGFILE,
    "STACK LOGFILE\n"
    "Time: %02d:%02d:%02d\n"
    "Day:  %s\n"
    "=======================================================================================\n"
    "|Name_stack: |Command:    |Argument:     |Stack size: |Status?                         \n"  
    "|            |            |              |            |                                \n"
    "=======================================================================================\n"
    "|            |            |              |            |                                \n",
    date->tm_hour, date->tm_min, date->tm_sec, day);

    if(!LOGFILE)
    {
        printf("OPEN_LOG_FILE_ERROR!\n");  
        exit(3);
    }          
}
__attribute__((destructor))
void logfile_kill()
{
    if(LOGFILE)
    {
        fprintf(LOGFILE, "THIS IS THE END!\n");
        fclose(LOGFILE);
    }
}


static void realloc_stack(stack *const p_stack, size_t new_capacity)
{
    #ifndef StackCanaryProtect
    const item_t *const data = p_stack->data;
    item_t *p_new_elements   = (item_t *) realloc ((void *)data, new_capacity * sizeof(item_t));
    #endif

    #ifdef StackCanaryProtect
    //*(canary_t *) ((char *)p_stack->data + p_stack->capacity * sizeof(item_t) + sizeof(canary_t)) = 0;
    const item_t *const data = (item_t *) ((char *)p_stack->data - sizeof(canary_t));
    item_t *p_new_elements   = (item_t *) realloc ((void *)data, new_capacity * sizeof(item_t) + 2 * sizeof(canary_t));
    #endif

    assert(p_new_elements);

    #ifndef StackCanaryProtect
    p_stack->data = p_new_elements;
    #endif

    #ifdef StackCanaryProtect
    *(canary_t *) ((char *)p_new_elements + new_capacity * sizeof(item_t) + sizeof(canary_t)) = STACK_CANARY;
    *(canary_t *) p_new_elements = STACK_CANARY;
    p_stack->data = (item_t *) ((char *)p_new_elements + sizeof(canary_t));
    #endif
}


static uint64_t verify_stack(stack *const p_stack)
{
    uint64_t error = 0;

    if (p_stack->data == nullptr) 
    {
        error = error | INVALID_DATA;
    }
    if (p_stack->size > p_stack->capacity || p_stack->size < 0) 
    {
        error = error | INVALID_SIZE;
    }
    if (p_stack->capacity < 0 || p_stack->capacity > MAXIMUM_CAPACITY) 
    {
        error = error | INVALID_CAPACITY;
    }
    if(p_stack->data == nullptr)
    {
        error = error | INVALID_STACK_UNITIALIZED;
    }

    #ifdef StackHashProtect
    if (p_stack->hash != ResultHash(p_stack)) 
    {
        error = error | INVALID_HASH;
    }
    #endif

    #ifdef StackCanaryProtect
    if (p_stack->LeftCanary != STACK_CANARY) 
    {
        error = error | INVALID_STACK_LCNRY;
    }
    if (p_stack->RightCanary != STACK_CANARY) 
    {
        error = error | INVALID_STACK_RCNRY;
    }

    if(p_stack->data != nullptr)
    {
        item_t *data = (item_t *) ( (char *)p_stack->data - sizeof(canary_t));
        canary_t DataLeftCanary  = *(canary_t *) data;
        canary_t DataRightCanary = *(canary_t *) ((char *) data + p_stack->capacity * sizeof(item_t) + sizeof(canary_t));

        if (DataLeftCanary != STACK_CANARY) 
        {
            error = error | INVALID_DATA_LCNRY;
        }
        if (DataRightCanary != STACK_CANARY) 
        {
            error = error | INVALID_DATA_RCNRY;
        }
    }
    #endif

    return error;
}



static void check_stack(stack* const p_stack, const char* const stack_name, Functions current_cmd, uint64_t error, item_t* p_element, item_t element)
{  
    char function[16] = "";

    switch (current_cmd)
    {
        case ctor: strcpy(function, "stack_ctor");   break;
        case dtor: strcpy(function, "stack_dtor");   break;
        case push: strcpy(function, "stack_push");   break;
        case  pop: strcpy(function, "stack_pop" );   break;
        case dump: strcpy(function, "stack_dump");   break;
    }                               
    fprintf(LOGFILE, "|%-12s|%-12s|", stack_name, function);                            
    if(current_cmd == push && ( (error & INVALID_DATA) == 0))                           
        fprintf(LOGFILE, "%-14" SPECIFICATOR "|", element);                             
    else if(current_cmd == pop && ( (error & INVALID_EMPTY_STACK) == 0))                
        fprintf(LOGFILE, "%-14" SPECIFICATOR "|", *p_element);                          
    else                                                                                
        fprintf(LOGFILE, " -            |");                                              
    fprintf(LOGFILE, "%-12zu|", p_stack->size);                                         
                                                                                        
    if (error == 0)                                                                     
    {                                                                                   
        fprintf(LOGFILE, "OK\n"                                                         
        "|            |            |              |            |\n");                     
    }                                                                                   
    else                                                                                
    {                                                                                   
        if((error & INVALID_DATA) != 0)                                                 
            fprintf(LOGFILE, "INVALID_DATA\n"                                           
            "|            |            |              |            |");                   
        if((error & INVALID_SIZE) != 0)                                                 
            fprintf(LOGFILE, "INVALID_SIZE\n"                                           
            "|            |            |              |            |");                   
        if((error & INVALID_CAPACITY) != 0)                                             
            fprintf(LOGFILE, "INVALID_CAPACITY\n"                                       
            "|            |            |              |            |");                   
        if((error & INVALID_STACK_REINITIALIZIED) != 0)                                 
            fprintf(LOGFILE, "INVALID_STACK_REINITIALIZIED\n"                           
            "|            |            |              |            |");                   
        if((error & INVALID_STACK_UNITIALIZED) != 0)                                    
            fprintf(LOGFILE, "INVALID_STACK_UNITIALIZED\n"                              
            "|            |            |              |            |");                   
        if((error & INVALID_EMPTY_STACK) != 0)                                          
            fprintf(LOGFILE, "INVALID_EMPTY_STACK\n"                                    
            "|            |            |              |            |");                   
        if((error & INVALID_HASH) != 0)                                                 
            fprintf(LOGFILE, "INVALID_HASH\n"                                           
            "|            |            |              |            |");                   
        if((error & INVALID_DATA_LCNRY) != 0)                                           
            fprintf(LOGFILE, "INVALID_DATA_LCNRY\n"                                     
            "|            |            |              |            |");                   
        if((error & INVALID_DATA_RCNRY) != 0)                                           
            fprintf(LOGFILE, "INVALID_DATA_RCNRY\n"                                     
            "|            |            |              |            |");                   
        if((error & INVALID_STACK_LCNRY) != 0)                                          
            fprintf(LOGFILE, "INVALID_STACK_LCNRY\n"                                    
            "|            |            |              |            |");                   
        if((error & INVALID_STACK_RCNRY) != 0)                                          
            fprintf(LOGFILE, "INVALID_STACK_RCNRY\n"                                    
            "|            |            |              |            |");                   
        fprintf(LOGFILE, "\n");                                                         
        stack_dump(p_stack, stack_name);                                                
    }
}


void stack_ctor(stack *const p_stack, const char *const stack_name)
{
    assert(p_stack);
    open_log_file();
    
    uint64_t error = 0;
    if(p_stack->size     != 0  ||
       p_stack->capacity != 0  ||    
       p_stack->data     != nullptr) 
    {
        error = error | INVALID_STACK_REINITIALIZIED;
        check_stack(p_stack, stack_name, ctor, error, nullptr, 0);

        #ifdef StackHashProtect
        p_stack->hash = ResultHash(p_stack);
        #endif

        return ;
    }

    #ifndef StackCanaryProtect
    item_t *data = (item_t *) calloc(INITIAL_CAPACITY, sizeof(item_t));
    #endif

    #ifdef StackCanaryProtect
    item_t *data = (item_t *) calloc(INITIAL_CAPACITY * sizeof(item_t) + 2 * sizeof(canary_t), 1);
    #endif

    assert(data);

    #ifdef StackCanaryProtect
    p_stack->LeftCanary      = STACK_CANARY;
    p_stack->RightCanary     = STACK_CANARY;
    canary_t DataLeftCanary  = STACK_CANARY;
    canary_t DataRightCanary = STACK_CANARY;

    *(canary_t *) data = DataLeftCanary;
    *(canary_t *) ((char *) data + INITIAL_CAPACITY * sizeof(item_t) + sizeof(canary_t)) = DataRightCanary;
    data = (item_t *) ( (char *) data + sizeof(canary_t));
    #endif

    p_stack->size      = 0;
    p_stack->capacity  = INITIAL_CAPACITY;
    p_stack->data      = data;

    #ifdef StackHashProtect
    p_stack->hash = ResultHash(p_stack);
    #endif

    error = verify_stack(p_stack);
    check_stack(p_stack, stack_name, ctor, error, nullptr, 0);
} 


void stack_dtor(stack *const p_stack, const char *const stack_name)
{
    assert(p_stack);
    item_t* data      = nullptr;
    open_log_file();

    uint64_t error = verify_stack(p_stack);
    if(error)
        goto final_check;    

    data = p_stack->data;

    #ifdef StackCanaryProtect
    data = (item_t *) ((char *) data - sizeof(canary_t));
    #endif

    free(data);
    p_stack->size        = 0;
    p_stack->capacity    = 0;
    p_stack->data        = nullptr;

    #ifdef StackCanaryProtect
    p_stack->LeftCanary  = 0;
    p_stack->RightCanary = 0;
    #endif

    #ifdef StackHashProtect
    p_stack->hash        = 0;
    #endif   

final_check:
    check_stack(p_stack, stack_name, dtor, error, nullptr, 0);
}


void stack_push(stack *const p_stack, const char *const stack_name, const item_t element)
{
    assert(p_stack);
    open_log_file();

    uint64_t error = verify_stack(p_stack);
    if(error)
    {
        check_stack(p_stack, stack_name, push, error, nullptr, element);

        #ifdef StackHashProtect
        p_stack->hash = ResultHash(p_stack);
        #endif

        return;
    }

    if(p_stack->size == p_stack->capacity)
    {
        size_t new_capacity = p_stack->capacity * 2;
        realloc_stack(p_stack, new_capacity);
        p_stack->capacity = new_capacity;
    }

    item_t *data = p_stack->data;

    data[p_stack->size] = element;
    p_stack->size++;

    #ifdef StackHashProtect
    p_stack->hash = ResultHash(p_stack);
    #endif

    error = verify_stack(p_stack);
    check_stack(p_stack, stack_name, push, error, nullptr, element);
}


void stack_pop(stack *const p_stack, const char *const stack_name, item_t *const p_element)
{
    assert(p_stack);
    assert(p_element);
    open_log_file();

    uint64_t error = verify_stack(p_stack);
    if(error || p_stack->size == 0)
    {
        if(p_stack->size == 0)
            error = error | INVALID_EMPTY_STACK;
        check_stack(p_stack, stack_name, pop, error, p_element, 0);

        #ifdef StackHashProtect
        p_stack->hash = ResultHash(p_stack);
        #endif

        return;
    }

    item_t *data = p_stack->data;

    *p_element = data[p_stack->size - 1];
    p_stack->size--;

    if(p_stack->capacity == p_stack->size * 2)
    {
        size_t new_capacity = p_stack->capacity / 2;
        realloc_stack(p_stack, new_capacity);
        p_stack->capacity = new_capacity;
    }
    
    #ifdef StackHashProtect
    p_stack->hash = ResultHash(p_stack);
    #endif

    error = verify_stack(p_stack);
    check_stack(p_stack, stack_name, pop, error, p_element, 0);
}


void stack_dump(stack *const p_stack, const char *const stack_name)
{
    assert(p_stack);
    open_log_file();
    uint64_t error = verify_stack(p_stack);

    item_t *data = p_stack->data;
    
    #ifdef StackCanaryProtect
    canary_t DataLeftCanary  = 0;
    canary_t DataRightCanary = 0;
    if(data != nullptr)
    {  
        data = (item_t *) ((char *) data - sizeof(canary_t));
        DataLeftCanary  = *(canary_t *) data;
        DataRightCanary = *(canary_t *) ((char *) data + p_stack->capacity * sizeof(item_t) + sizeof(canary_t));
        data = (item_t *) ((char *) data + sizeof(canary_t));
    }
    #endif

    fprintf (LOGFILE,
                "|=====================================================|\n"
                "|            |                                        |\n" 
                "|%-12s|              STACK DUMP                |\n"
                "|            |                                        |\n"
                "|=====================================================|\n"
                "| Stack information:                                  |\n"
                "| Address:            %-32p|\n"
                "| Size:               %-32zu|\n"
                "| Capacity:           %-32zu|\n"
                "| Data:               %-32p|\n"

        #ifdef StackHashProtect
                "| Hash:               %-32" HASH_SPECIFICATOR "|\n"
        #endif

        #ifdef StackCanaryProtect
                "| Left_canary:        %-32" CANARY_SPECIFICATOR "|\n"
                "| Right_canary:       %-32" CANARY_SPECIFICATOR "|\n"
        #endif
,
        stack_name,
        p_stack,                 
        p_stack->size,        
        p_stack->capacity,       
        p_stack->data

        #ifdef StackHashProtect
,       p_stack->hash
        #endif

        #ifdef StackCanaryProtect
,       p_stack->LeftCanary,     
        p_stack->RightCanary
        #endif
);
    #ifdef StackCanaryProtect
    if(data != nullptr)
        fprintf(LOGFILE, "| Data_left_canary:   %-32" CANARY_SPECIFICATOR "|\n"
                         "| Data_right_canary:  %-32" CANARY_SPECIFICATOR "|\n", DataLeftCanary, DataRightCanary);
    #endif

    fprintf(LOGFILE, "|                                                     |\n"
                     "| Elements:                                           |\n");

    for(size_t i = 1; i <= p_stack->size; i++)
        fprintf(LOGFILE, "| *%7zu:         %-33" SPECIFICATOR " |\n", i, data[i - 1]);
      
    if( (error & INVALID_CAPACITY) == 0)
        for(size_t i = p_stack->size + 1; i <= p_stack->capacity; i++)
            fprintf(LOGFILE, "|  %7zu:         %-33" SPECIFICATOR " |\n", i, data[i - 1]);
    fprintf(LOGFILE, "|=====================================================|\n"
                     "|            |            |              |            |\n");
}


#ifdef StackHashProtect
static hash_t QHash(const void *const data, const size_t data_size)
{
    const char       *byte     = (char *) data;
    const char *const lastByte = byte + data_size;
    hash_t            hash     = 0;
    
    while (byte < lastByte)
        hash = ((hash << 0x8) + (hash >> 0x8)) ^ *byte++;
    
    return hash;
}


static hash_t ResultHash(const stack *const p_stack)
{
    hash_t data_hash  = 0, 
           stack_hash = 0;

    #ifndef StackCanaryProtect
    stack_hash = QHash(p_stack, 2 * sizeof(size_t) + sizeof(item_t*));
    #else
    stack_hash = QHash((item_t*)((char *)p_stack + sizeof(canary_t)), 2 * sizeof(int) + sizeof(item_t*));
    #endif

    if(p_stack->data != nullptr)
    {
        #ifndef StackCanaryProtect
        data_hash = QHash(p_stack->data, p_stack->capacity * sizeof(item_t));
        #else
        data_hash = QHash((item_t*)((char *)p_stack->data + sizeof(canary_t)), p_stack->capacity * sizeof(item_t));
        #endif
    }
    else 
        data_hash = 0;

    return data_hash ^ stack_hash;
}
#endif
