#include "../include/array.h"


int array_ctor(array* const p_array, size_t item_size)
{
    void* data  = (void* ) calloc(INIT_CAPACITY, item_size);

    if(!data)
        return 0;

    p_array->data      = data;
    p_array->size      = 0;
    p_array->capa      = INIT_CAPACITY;
    p_array->item_size = item_size;
    return 1;
} 


int array_dtor(array* const p_array, bool free_data)
{
    assert(p_array);   

    if(free_data)
        for(size_t i = 0; i < p_array->size; i++)
            free(*(void**)((char* )p_array->data + i * p_array->item_size));

    free(p_array->data);
    p_array->size      = 0;
    p_array->capa      = 0;
    p_array->item_size = 0;
    return 1;
}


int array_push(array* const p_array, void* const item)
{
    assert(p_array);
    assert(item);

    if(p_array->capa == p_array->size)
        if(!array_realloc(p_array, p_array->capa * 2))
            return 0;

    memcpy((char* )p_array->data + p_array->item_size * p_array->size, item, p_array->item_size);
    p_array->size++;
    return 1;
}


int array_pop(array* const p_array, void* const item)
{
    assert(p_array);
    assert(item);

    if(p_array->size == 0)
        return 0;

    if(p_array->capa == p_array->size * 2)
        if(!array_realloc(p_array, p_array->capa / 2))
            return 0;

    p_array->size--;
    memcpy(item, (char* )p_array->data + p_array->item_size * p_array->size, p_array->item_size);
    return 1;    
}


int array_realloc(array* const p_array, size_t new_capacity)
{
    assert(p_array);

    if(new_capacity > MAX_CAPACITY)
        return 0;

    void* new_data = (void* )calloc(new_capacity, p_array->item_size);

    if(!new_data)
        return 0;

    memcpy(new_data, p_array->data, p_array->capa * p_array->item_size);

    p_array->data = new_data;
    p_array->capa = new_capacity;
    return 1;
}


void dump_init(FILE* fp, const array* const p_array)
{
    assert(fp);

    const time_t        seconds = time(nullptr);
    const tm     *const date    = localtime(&seconds);

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

    fprintf(fp,
    "DUMP LOGFILE\n"
    "Time: %02d:%02d:%02d\n"
    "Day:  %s\n"
    "|===================================================================================================\n"
    "|            |                                                                                      \n" 
    "|            |              ARRAY DUMP                                                              \n"
    "|            |                                                                                      \n"
    "|===================================================================================================\n"
    "| Array information:                                                                                \n"
    "| Address:            %-78p\n"
    "| Size:               %-78zu\n"
    "| Capacity:           %-78zu\n"
    "| Data:               %-78p\n"
    "| Item size:          %-78zu\n",
    date->tm_hour, date->tm_min, date->tm_sec, day,
    p_array, p_array->size, p_array->capa, p_array->data, p_array->item_size);       
}

int array_dump(const array* const p_array, const char* dump_file_path)
{
    assert(p_array);
    FILE* fp = fopen(dump_file_path, "w");
    if(!fp)
        return 0;
    
    dump_init(fp, p_array);

    size_t iterator = 0;
    for(iterator = 0; iterator < p_array->size; iterator++)
        fprintf(fp, "| *%7zu:           %-29" SPECIFICATOR "\n", iterator + 1, (char* )p_array->data + iterator * p_array->item_size);
      
    for(iterator = p_array->size; iterator < p_array->capa; iterator++)
        fprintf(fp, "|  %7zu:           %-29" SPECIFICATOR "\n", iterator + 1, (char* )p_array->data + iterator * p_array->item_size);

    fprintf(fp, "|===================================================================================================\n");
    fclose(fp);
    return 1;
}
