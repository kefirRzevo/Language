#ifndef STACK_H
#define STACK_H


#include <stdio.h>
#include <stdlib.h>
#include <cstddef>
#include <assert.h>
#include <string.h>
#include <time.h>


typedef void* item_t;

#ifdef StackCanaryProtect
typedef long long int canary_t;
#endif

#ifdef StackHashProtect
typedef size_t hash_t;
#endif


#define SPECIFICATOR "p"
#define CANARY_SPECIFICATOR "llX"
#define HASH_SPECIFICATOR "zu"


const size_t INITIAL_CAPACITY = 10;
const size_t MAXIMUM_CAPACITY = 10000;

#ifdef StackCanaryProtect
const canary_t STACK_CANARY = 0xAB9876543210;
#endif


struct stack
{
    #ifdef StackCanaryProtect
    canary_t LeftCanary;
    #endif

    item_t* data;
    size_t size;
    size_t capacity;

    #ifdef StackHashProtect
    hash_t hash;
    #endif

    #ifdef StackCanaryProtect
    canary_t RightCanary;
    #endif
};


void StackCtor(stack *const p_stack);

void StackDtor(stack *const p_stack);

void StackPush(stack *const p_stack, const item_t element);
 
void StackPop(stack *const p_stack, item_t *const p_element);

void StackDump(const stack *const p_stack);


void stack_ctor(stack *const p_stack, const char *const stack_name);

void stack_dtor(stack *const p_stack, const char *const stack_name);

void stack_push(stack *const p_stack, const char *const stack_name, const item_t element);

void stack_pop( stack *const p_stack, const char *const stack_name, item_t *const p_element);

void stack_dump(stack *const p_stack, const char *const stack_name);


#define StackCtor(p_stack)          stack_ctor(p_stack, #p_stack);
#define StackDtor(p_stack)          stack_dtor(p_stack, #p_stack);
#define StackPush(p_stack, element) stack_push(p_stack, #p_stack, element);
#define StackPop(p_stack, p_element) stack_pop(p_stack, #p_stack, p_element);
#define StackDump(p_stack)          stack_dump(p_stack, #p_stack);


enum StackError 
{
    INVALID_DATA                 = 1 << 0,
    INVALID_SIZE                 = 1 << 1,
    INVALID_CAPACITY             = 1 << 2,
    INVALID_STACK_REINITIALIZIED = 1 << 3,
    INVALID_STACK_UNITIALIZED    = 1 << 4,
    INVALID_EMPTY_STACK          = 1 << 5,
    INVALID_HASH                 = 1 << 6,
    INVALID_DATA_LCNRY           = 1 << 7,
    INVALID_DATA_RCNRY           = 1 << 8,
    INVALID_STACK_LCNRY          = 1 << 9,
    INVALID_STACK_RCNRY          = 1 << 10,
};


enum Functions
{
    ctor_ = 1,
    push_ = 2,
    dump_ = 3,
    pop_  = 4,
    dtor_ = 5,
};


#endif //STACK_H