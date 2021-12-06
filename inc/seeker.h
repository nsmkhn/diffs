#ifndef SEEKER_H
#define SEEKER_H

#include "set.h"

struct diffstat
{
    long unsigned num_changed, num_added, num_removed;
    float time_spent;
};

struct metadata
{
    char *fdirname, *sdirname;
    struct diffstat stat;
};

void seek_diff(struct metadata *meta);

#endif
