#ifndef SET_H
#define SET_H

// NOTE(niki): returns zero if values are equal,
// or -1/1 if the first argument is less/greater than the second, respectively
typedef int (*fcmp)(void *, void *); 

struct set_node
{
    void *data;
    struct set_node *parent;
    struct set_node *left;
    struct set_node *right;
};

struct set
{
    struct set_node *root;
    int unsigned size;
    fcmp cmp;
};


struct set *
set_create(fcmp);

int
set_destroy(struct set *);

int
set_insert(struct set *, void *);

int
set_remove(struct set *, void *);

int
set_contains(struct set *, void *);

#endif //SET_H
