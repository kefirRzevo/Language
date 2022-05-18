#include "../include/tree.h" 

Tree* tree_ctor(Tree** p_tree)
{
    Tree* new_tree  = (Tree* )calloc(1, sizeof(Tree));
    if(!new_tree)
        return nullptr;

     *p_tree        = new_tree;
    (*p_tree)->size = 0;

    return new_tree;
}


Node* node_delete(Node* p_node)
{
    if(p_node != nullptr)
    {
        node_delete(p_node->Left );
        node_delete(p_node->Right);

        free(p_node);
    }
    return nullptr;
}


Tree* tree_dtor(Tree* p_tree)
{
    assert(p_tree);

    p_tree->root = node_delete(p_tree->root);
    free(p_tree);
    return nullptr;
}


Node* node_cpy(Node* p_node)
{
    if(!p_node)
    {
        return nullptr;
    }

    Node* new_node = (Node* )calloc(1, sizeof(Node));
    if(!new_node)
        return nullptr;

    new_node->Type  = p_node->Type;
    new_node->Value = p_node->Value;

    if(p_node->Left)
    {
        new_node->Left  = node_cpy(p_node->Left);
    }
    if(p_node->Right)
    {
        new_node->Right = node_cpy(p_node->Right);
    }
    return new_node;
}


size_t count_tree_size(Node* p_node)
{
    if(p_node->Left && p_node->Right)
        return count_tree_size(p_node->Left) + count_tree_size(p_node->Right) + 1;

    if(p_node->Right)
        return count_tree_size(p_node->Right) + 1;
    if(p_node->Left)
        return count_tree_size(p_node->Left ) + 1;
    
    return 1;
}


static bool nodecmp(Node* node1, Node* node2)
{
    if(node1->Type == node2->Type)
        if(!memcmp(&node1->Value, &node2->Value, sizeof(NodeValue)))
            return true;
    
    return false;
}


Node* tree_compare(Node* n1, Node* n2)
{
        assert(n1);
        assert(n2);

        if(!nodecmp(n1, n2))
            return n1;

        if (((!n1->Left  && n2->Left)  || (n1->Left  && !n2->Left)) ||
            ((!n1->Right && n2->Right) || (n1->Right && !n2->Right)))
               return n1; 

        Node* n = nullptr;

        if (n1->Left)
                n = tree_compare(n1->Left, n2->Left);
        if (n)
                return n;

        if (n1->Right)
                n = tree_compare(n1->Right, n2->Right);
        if (n)
                return n;

        return nullptr;
}


void tree_visit(Node* p_node, void(*function)(Node*))
 {
     if(!p_node)
         return;

     function(p_node);

     if(p_node->Left)
         tree_visit(p_node->Left,  function);
     if(p_node->Right)
         tree_visit(p_node->Right, function);
 }


bool is_number(Node* node)
{
    if(!node)
        return false;

    return node->Type == NUM;
}

bool is_natural(Node* node)
{
    if(!node)
        return false;
        
    if(node->Value.number < 1 || fabs(node->Value.number - (int)node->Value.number) > EPSILON)
        return false;
    
    return true;
}

char* is_ident(Node* node)
{
    if(!node)
        return nullptr;

    if(node->Type == ID)
        return node->Value.ident;
    return nullptr;
}

int is_operator(Node* node, int type)
{
    if(!node)
        return 0;

    if(node->Type == OPER)
        if(node->Value.oper_type == type)
            return type;
    return 0;
}


double get_number(Node* node)
{
    assert(node && node->Type == NUM);

    return node->Value.number;
}


const char* get_oper_string(Node* node)
{
    assert(node && node->Type == OPER);

    #define OPER(name, keyword, ident)                      \
                else if (keyword == node->Value.oper_type)  \
                    return ident;                      

                if (0) {} 
    #include "../STANDARD_TREE"
    #undef OPER

    return nullptr;
}
