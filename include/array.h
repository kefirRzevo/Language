#ifndef ARRAY_H
#define ARRAY_H


#include <stdio.h>
#include <stdlib.h>
#include <cstddef>
#include <assert.h>
#include <string.h>
#include <time.h>

#define SPECIFICATOR "p"

const size_t INIT_CAPACITY = 10;
const size_t  MAX_CAPACITY = 1280;

struct array
{
    void*   data;
    size_t  size;
    size_t  capa;
    size_t  item_size;
};


int array_ctor(array* const p_array, size_t item_size);

int array_dtor(array* const p_array, bool free_data);

int array_push(array* const p_array, void* const item);

int array_pop(array* const p_array,  void* const item);

int array_realloc(array* const p_array, size_t new_capacity);

int array_dump(const array* const p_array, const char* dump_file_path);

void dump_init(FILE* fp, const array* const p_array);

#endif