#ifndef RECURSIVE_DESCENT_H
#define RECURSIVE_DESCENT_H

#include <stdio.h>
#include <stdlib.h>
#include <cstddef>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "../tree.h"
#include "../stack.h"
#include "lexer.h"


const double EPSILON      = 0.001;
const size_t MAX_MAS_SIZE = 100;


Tree* create_tree(array* tokens);

#endif
