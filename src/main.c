#include "list.h"
#include "seeker.h"
#include "set.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define NUM_ARGS 3

int mstrcmp(void *s1, void *s2);

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

    clock_t start = clock();
    struct list *fdir_files = list_create();
    scan_dir(meta.fdirname, meta.fdirname, LIST, fdir_files);
    struct set *sdir_files = set_create(mstrcmp);
    scan_dir(meta.sdirname, meta.sdirname, SET, sdir_files);
    seek_diff(fdir_files, sdir_files, &meta);
    meta.stat.time_spent = (float) (clock() - start) / CLOCKS_PER_SEC;
    printf("Comparison finished in %f seconds. %lu files changed, %lu removed, %lu added\n",
        meta.stat.time_spent, meta.stat.num_changed, meta.stat.num_removed, meta.stat.num_added);

    list_destroy(fdir_files);
    set_destroy(sdir_files);

    return 0;
}

int
mstrcmp(void *s1, void *s2)
{
    return strcmp(s1, s2);
}
