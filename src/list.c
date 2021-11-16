#include "list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct list *
list_create(void)
{
    struct list *l = malloc(sizeof(struct list));
    assert(l != NULL);
    l->head = NULL;

    return l;
}

static int
list_clear(struct list *l)
{
    if(!l)
        return 0;

    struct list_node *curr = l->head;
    while(curr)
    {
        struct list_node *next = curr->next;
        curr->next = NULL;
        free(curr->data);
        curr->data = NULL;
        free(curr);
        curr = next;
    }
    l->head = NULL;
    
    return 1;
}

int
list_destroy(struct list *l)
{
    if(!l)
        return 0;

    list_clear(l);
    free(l);

    return 1;
}

int
list_add(struct list *l, void *data)
{
    if(!l || !data)
        return 0;
    
    struct list_node *node = malloc(sizeof(struct list_node));
    assert(node != NULL);
    node->data = data;
    node->next = l->head;
    l->head = node;

    return 1;
}
