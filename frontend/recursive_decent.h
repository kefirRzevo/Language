#ifndef RECURSIVE_DESCENT_H
#define RECURSIVE_DESCENT_H

#include <stdio.h>
#include <stdlib.h>
#include <cstddef>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "../include/tree.h"
#include "lexer.h"

Tree* create_tree(array* tokens);


#endif
