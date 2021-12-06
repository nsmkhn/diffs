#include "seeker.h"
#include <stdio.h>
#include <stdlib.h>

#define NUM_ARGS 3

int
main(int argc, char **argv)
{
    if(argc != NUM_ARGS)
    {
        fprintf(stderr, "Usage: %s <dir> <dir>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct metadata meta = {0};
    meta.fdirname = argv[1];
    meta.sdirname = argv[2];
    seek_diff(&meta);
    printf("Comparison finished in %f seconds. %lu files changed, %lu removed, %lu added\n",
        meta.stat.time_spent, meta.stat.num_changed, meta.stat.num_removed, meta.stat.num_added);

    return 0;
}
