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


Node* node_delete(Node* tree_node)
{
    if(tree_node != nullptr)
    {
        node_delete(tree_node->Left );
        node_delete(tree_node->Right);

        free(tree_node);
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
