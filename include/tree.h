#ifndef TREE_H 
#define TREE_H


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>


#define OPER_SPEC  "s"
#define   ID_SPEC  "s"
#define  NUM_SPEC  "lg"


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


Node*   node_delete(Node* tree_node);

Node*   node_cpy(Node* p_node);

Tree*   tree_ctor(Tree** p_tree);

Tree*   tree_dtor(Tree* p_tree);

Tree*   tree_dump(Tree* p_tree);

Node*   tree_compare(Node* n1, Node* n2);

size_t  count_tree_size(Node* p_node);

void    tree_visit(Node* p_node, void(*function)(Node*));

Tree*   tree_write(Tree* p_tree, const char* filepath);


#endif
