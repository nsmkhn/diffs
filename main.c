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
    struct set *fdir_files = set_create(mstrcmp);
    struct set *sdir_files = set_create(mstrcmp);
    scan_dir(meta.fdirname, fdir_files, meta.fdirname);
    scan_dir(meta.sdirname, sdir_files, meta.sdirname);
    seek_diff(fdir_files, sdir_files, &meta);
    meta.stat.time_spent = (float) (clock() - start) / CLOCKS_PER_SEC;
    print_diffstat(&meta.stat);
    set_destroy(fdir_files);
    set_destroy(sdir_files);

    return 0;
}

int
mstrcmp(void *s1, void *s2)
{
    return strcmp(s1, s2);
}
