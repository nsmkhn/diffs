#ifndef LIST_H
#define LIST_H

struct list_node
{
    void *data;
    struct list_node *next;
};

struct list
{
    struct list_node *head;
};

struct list *list_create(void);
int list_destroy(struct list *l);
int list_add(struct list *l, void *data);

#endif
