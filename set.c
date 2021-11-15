#include "set.h"
#include <assert.h>
#include <stdlib.h>

struct set *
set_create(fcmp cmp)
{
    if(!cmp)
        return NULL;
    
    struct set *set = malloc(sizeof(struct set));
    assert(set != NULL);
    
    set->root = NULL;
    set->size = 0;
    set->cmp = cmp;
    
    return set;
}

static void
set_destroy_node(struct set_node *node)
{
    if(!node)
        return;
    
    set_destroy_node(node->left);
    set_destroy_node(node->right);
    
    if(node->parent)
    {
        if(node == node->parent->right)
        {
            node->parent->right = NULL;
        }
        else
        {
            node->parent->left = NULL;
        }
    }
    
    free(node->data);
    node->left = node->right = node->parent = node->data = NULL;
    free(node);
}

int
set_destroy(struct set *set)
{
    if(!set)
        return 0;
    
    set_destroy_node(set->root);
    
    set->root = NULL;
    set->cmp = NULL;
    set->size = 0;
    free(set);
    
    return 1;
}

static struct set_node *
set_create_node(void *data, struct set_node *parent)
{
    struct set_node *node = malloc(sizeof(struct set_node));
    assert(node != NULL);
    
    node->data = data;
    node->parent = parent;
    node->left = node->right = NULL;
    
    return node;
}

static int
set_attempt_insert(struct set *set, void *data)
{
    if(!set->root)
    {
        set->root = set_create_node(data, NULL);
        return 1;
    }
    
    struct set_node *walk = set->root;
    int c;
    while((c = set->cmp(walk->data, data)) != 0)
    {
        if(c < 0)
        {
            if(walk->right)
            {
                walk = walk->right;
                continue;
            }
            else
            {
                walk->right = set_create_node(data, walk);
            }
        }
        else
        {
            if(walk->left)
            {
                walk = walk->left;
                continue;
            }
            else
            {
                walk->left = set_create_node(data, walk);
            }
        }

        return 1;
    }
    
    return 0;
}

int
set_insert(struct set *set, void *data)
{
    if(!set || !data)
        return 0;
   
    if(set_attempt_insert(set, data))
    {
        ++set->size;
        return 1;
    }

    return 0;
}

static struct set_node *
set_locate_node(struct set *set, void *data)
{
    struct set_node *walk = set->root;
    int c;
    while(walk && (c = set->cmp(walk->data, data)) != 0)
        walk = c < 0 ? walk->right : walk->left;
    
    return walk;
}

static struct set_node *
set_locate_inorder_successor(struct set_node *node)
{
    struct set_node *successor = node->right;
    while(successor->left)
        successor = successor->left;
    
    return successor;
}

static void
set_node_move_child(struct set_node *node,
                    struct set_node *old_child,
                    struct set_node *new_child)
{
    if(node)
    {
        if(node->right == old_child)
            node->right = new_child;
        else
            node->left = new_child;
        
        if(new_child)
            new_child->parent = node;
    }
    else
    {
        if(new_child)
            new_child->parent = NULL;
    }
}

static void
set_remove_node(struct set *set, struct set_node *exile)
{
    if(!exile->left && !exile->right)
    {
        set_node_move_child(exile->parent, exile, NULL);
        set->root = (exile == set->root) ? NULL : set->root;
    }
    else if(!exile->left || !exile->right)
    {
        struct set_node *child = exile->left ? exile->left : exile->right;
        set_node_move_child(exile->parent, exile, child);
        set->root = (exile == set->root) ? child : set->root;
    }
    else
    {
        struct set_node *successor = set_locate_inorder_successor(exile);
        if(successor->parent != exile)
        {
            if(successor->right)
            {
                successor->right->parent = successor->parent;
                successor->parent->left = successor->right;
            }
            else
            {
                successor->parent->left = NULL;
            }
            exile->right->parent = successor;
            successor->right = exile->right;
        }

        exile->left->parent = successor;
        successor->left = exile->left;
        set_node_move_child(exile->parent, exile, successor);
        set->root = (exile == set->root) ? successor : set->root;
    }
}

int
set_remove(struct set *set, void *data)
{
    if(!set || !data)
        return 0;
    
    struct set_node *exile = set_locate_node(set, data);
    if(exile)
    {
        set_remove_node(set, exile);
        --set->size;
        free(exile->data);
        free(exile);
        return 1;
    }

    return 0;
}

int
set_contains(struct set *set, void *data)
{
    if(!set || !data)
        return 0;
    
    return set_locate_node(set, data) ? 1 : 0;
}
