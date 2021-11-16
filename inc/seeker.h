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

void scan_dir_tolist(char *basename, char *dirname, struct list *files);
void scan_dir_tobtree(char *basename, char *dirname, struct set *files);
void seek_diff(struct list *fdir_files, struct set *sdir_files, struct metadata *meta);
void print_diffstat(struct diffstat *stat);

#endif
