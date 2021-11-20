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
    node->bf = 0;
    
    return node;
}

static struct set_node *
set_attempt_insert(struct set *set, void *data)
{
    if(!set->root)
    {
        set->root = set_create_node(data, NULL);
        return set->root;
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
                return walk->right;
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
                return walk->left;
            }
        }
    }
    
    return NULL;
}

static struct set_node *
rotate_left(struct set_node *x, // root of the subtree to be rotated left
            struct set_node *z) // right child of x, z is right-heavy
{
    struct set_node *t = z->left;
    x->right = t;
    if(t)
        t->parent = x;
    z->left = x;
    x->parent = z;

    if(z->bf == 0)
    {
        x->bf = 1;
        z->bf = -1;
    }
    else
    {
        x->bf = 0;
        z->bf = 0;
    }

    return z;
}

static struct set_node *
rotate_right(struct set_node *x, // root of the subtree to be rotated right
             struct set_node *z) // left child of x, x is left-heavy
{
    struct set_node *t = z->right;
    x->left = t;
    if(t)
        t->parent = x;
    z->right = x;
    x->parent = z;

    if(z->bf == 0)
    {
        x->bf = -1;
        z->bf = 1;
    }
    else
    {
        x->bf = 0;
        z->bf = 0;
    }

    return z;
}

static struct set_node *
rotate_leftright(struct set_node *x, // root of subtree to be rotated
                 struct set_node *z) // its left child, right-heavy
{
    struct set_node *y = z->right;
    struct set_node *t1 = y->left;
    z->right = t1;
    if(t1)
        t1->parent = z;
    y->left = z;
    z->parent = y;
    struct set_node *t2 = y->right;
    x->left = t2;
    if(t2)
        t2->parent = x;
    y->right = x;
    x->parent = y;

    if(y->bf == 0)
    {
        x->bf = 0;
        z->bf = 0;
    }
    else
    {
        if(y->bf > 0)
        {
            x->bf = -1;
            z->bf = 0;
        }
        else
        {
            x->bf = 0;
            z->bf = 1;
        }
    }
    y->bf = 0;

    return y;
}

static struct set_node *
rotate_rightleft(struct set_node *x, // root of subtree to be rotated
                 struct set_node *z) // its right child, left-heavy
{
    struct set_node *y = z->left;
    struct set_node *t1 = y->right;
    z->left = t1;
    if(t1)
        t1->parent = z;
    y->right = z;
    z->parent = y;
    struct set_node *t2 = y->left;
    x->right = t2;
    if(t2)
        t2->parent = x;
    y->left = x;
    x->parent = y;

    if(y->bf == 0)
    {
        x->bf = 0;
        z->bf = 0;
    }
    else
    {
        if(y->bf > 0)
        {
            x->bf = -1;
            z->bf = 0;
        }
        else
        {
            x->bf = 0;
            z->bf = 1;
        }
    }
    y->bf = 0;

    return y;
}

static void
rebalance_oninsert(struct set *set,
                   struct set_node *z) // newly inserted node
{
    for(struct set_node *x = z->parent; x != NULL; x = z->parent)
    {
        struct set_node *g, *n;
        if(z == x->right)
        {
            if(x->bf > 0)
            {
                g = x->parent;
                if(z->bf < 0)
                    n = rotate_rightleft(x, z);
                else
                    n = rotate_left(x, z);
            }
            else
            {
                if(x->bf < 0)
                {
                    x->bf = 0;
                    break;
                }
                x->bf = 1;
                z = x;
                continue;
            }
        }
        else
        {
            if(x->bf < 0)
            {
                g = x->parent;
                if(z->bf > 0)
                    n = rotate_leftright(x, z);
                else
                    n = rotate_right(x, z);
            }
            else
            {
                if(x->bf > 0)
                {
                    x->bf = 0;
                    break;
                }
                x->bf = -1;
                z = x;
                continue;
            }
        }

        n->parent = g;
        if(g)
        {
            if(x == g->left)
                g->left = n;
            else
                g->right = n;
        }
        else
        {
            set->root = n;
        }
        break;
    }
}

int
set_insert(struct set *set, void *data)
{
    if(!set || !data)
        return 0;
   
    struct set_node *z = set_attempt_insert(set, data);
    if(z)
    {
        ++set->size;
        rebalance_oninsert(set, z);
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
        // TODO: Implement self-balancing on delete.
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
