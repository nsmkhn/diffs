#ifndef SEEKER_H
#define SEEKER_H

#include "list.h"
#include "set.h"
#include <stdlib.h>

struct diffstat
{
    size_t num_changed, num_added, num_removed;
    float time_spent;
};

struct metadata
{
    char *fdirname, *sdirname;
    struct diffstat stat;
};

enum container_type
{
    LIST,
    SET
};

void scan_dir(char *basename, char *dirname, enum container_type ctype, void *container);
void seek_diff(struct list *fdir_files, struct set *sdir_files, struct metadata *meta);

#endif
