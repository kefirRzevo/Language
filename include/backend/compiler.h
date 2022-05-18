#ifndef COMPILER_H
#define COMPILER_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../tree.h"
#include "../stack.h"
#include "../array.h"
#include "../symtable.h"

Tree* compile_tree(const char* filepath, Tree* p_tree);


#endif
