#ifndef SEEKER_H
#define SEEKER_H

#include <stdlib.h>
#include "set.h"

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

void scan_dir(char *dirname, struct set *files, char *basename);
void seek_diff(struct set *fdir_files, struct set *sdir_files, struct metadata *meta);
void print_diffstat(struct diffstat *stat);

#endif
