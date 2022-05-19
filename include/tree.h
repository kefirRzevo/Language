#ifndef TREE_H 
#define TREE_H


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <math.h>
#include "array.h"


#define OPER_SPEC  "s"
#define   ID_SPEC  "s"
#define  NUM_SPEC  "lg"

const double EPSILON      = 0.001;


enum OperType
{
    #define OPER(name, keyword, ident)   \
        name = keyword,
        #include "../STANDARD_TREE"
    #undef OPER
};

enum NodeType
{
    NUM   = 1,
    ID    = 2,
    OPER  = 3,
};

union NodeValue
{
    double      number;
    char*       ident;
    OperType    oper_type;
};

struct Node
{
    NodeType    Type;
    NodeValue   Value;
    Node*       Left;
    Node*       Right;
};

struct Tree
{
    size_t size;
    Node*  root;
};

size_t  count_tree_size(Node* p_node);
Node*   node_delete    (Node* p_node);
Node*   node_cpy       (Node* p_node);
Node*   tree_compare   (Node* n1, Node* n2);
void    tree_visit     (Node* p_node, void(*function)(Node*));
Tree*   tree_ctor(Tree** p_tree);
Tree*   tree_dtor(Tree*  p_tree);
Tree*   tree_dump(Tree*  p_tree);
Tree*   tree_write(Tree* p_tree, const char* filepath);
Tree*   tree_read(array* idents, const char* filepath);

bool  is_number  (Node* node);
bool  is_natural (Node* node);
char* is_ident   (Node* node);
int   is_operator(Node* node, int type);

double      get_number(Node* node);
const char* get_oper_string(Node* node);


#endif
