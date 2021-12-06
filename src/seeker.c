#include "seeker.h"
#include "set.h"
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define perror_and_exit(s) do { perror(s); exit(EXIT_FAILURE); } while(0)
#define DIR_SEP '/'
#define DIR_SEP_LEN 1

struct diffargs
{
    struct set *files;
    struct metadata *meta;
};

static bool
files_equal(char *f1, char *f2)
{
    int fd1 = open(f1, O_RDONLY);
    int fd2 = open(f2, O_RDONLY);
    if(fd1 == -1 || fd2 == -1)
        perror_and_exit("open");

    struct stat sb1, sb2;
    if(fstat(fd1, &sb1) == -1 || fstat(fd2, &sb2) == -1)
        perror_and_exit("fstat");
    bool res = sb2.st_size == sb1.st_size;
    if(res)
    {
        char *first = mmap(NULL, sb1.st_size, PROT_READ, MAP_PRIVATE, fd1, 0);
        char *second = mmap(NULL, sb2.st_size, PROT_READ, MAP_PRIVATE, fd2, 0);
        if(first == MAP_FAILED || second == MAP_FAILED)
            perror_and_exit("mmap");
        for(off_t i = 0; i < sb1.st_size && res; ++i)
            if(first[i] != second[i])
                res = false;
        munmap(first, sb1.st_size);
        munmap(second, sb2.st_size);
    }
    close(fd1);
    close(fd2);

    return res;
}

static void
print_filenames(struct set_node *root, char *suffix)
{
    if(!root)
        return;
    printf("%s%s\n", (char *) root->data, suffix);
    print_filenames(root->left, suffix);
    print_filenames(root->right, suffix);
}

static void
fill_namebuf(char *buf, size_t buflen, char *basename, char *entryname)
{
    memset(buf, 0, buflen);
    strcat(buf, basename);
    if(basename[strlen(basename)-1] != DIR_SEP)
        buf[strlen(basename)] = DIR_SEP;
    strcat(buf, entryname);
}

static char *
alloc_and_build_filename(char *dirbasename, char *dirname, char *entryname)
{
    size_t len;
    char *filename;
    if(strlen(dirbasename) == strlen(dirname))
    {
        len = strlen(entryname) + 1;
        filename = malloc(len);
        memset(filename, 0, len);
    }
    else
    {
        dirname += strlen(dirbasename);
        if(dirbasename[strlen(dirbasename) - 1] != DIR_SEP)
            ++dirname;
        len = strlen(dirname) + DIR_SEP_LEN + strlen(entryname) + 1;
        filename = malloc(len);
        memset(filename, 0, len);
        strcat(filename, dirname);
        filename[strlen(dirname)] = DIR_SEP;
    }
    strcat(filename, entryname);

    return filename;
}

static void
process_files_in_dir(char *dirname, char *basename,
                     void (*file_action)(char *, void *), void *action_args)
{
    DIR *dir = opendir(dirname);
    if(!dir)
        perror_and_exit("opendir");

    struct dirent *entry;
    errno = 0;
    while((entry = readdir(dir)))
    {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        if(entry->d_type == DT_DIR)
        {
            int namelen = strlen(dirname) + DIR_SEP_LEN + strlen(entry->d_name) + 1;
            char name[namelen];
            fill_namebuf(name, namelen, dirname, entry->d_name);
            process_files_in_dir(name, basename, file_action, action_args);
        }
        else
        {
            char *filename = alloc_and_build_filename(basename, dirname, entry->d_name);
            file_action(filename, action_args);
        }
        errno = 0;
    }
    if(errno)
        perror_and_exit("readdir");

    closedir(dir);
}

static bool
is_file_changed(char *filename, char *fdirname, char *sdirname)
{
    int flen = strlen(filename) + DIR_SEP_LEN + strlen(fdirname) + 1;
    int slen = strlen(filename) + DIR_SEP_LEN + strlen(sdirname) + 1;
    char fbuf[flen];
    char sbuf[slen];
    fill_namebuf(fbuf, flen, fdirname, filename);
    fill_namebuf(sbuf, slen, sdirname, filename);

    return !files_equal(fbuf, sbuf);
}

static int
mstrcmp(void *s1, void *s2)
{
    return strcmp(s1, s2);
}

static void
save_filename_a(char *filename, void *args)
{
    struct set *files = (struct set *) args;
    assert(set_insert(files, filename) != 0);
}

static void
diff_file_a(char *filename, void *args)
{
    struct diffargs *dargs = (struct diffargs *) args;
    if(set_contains(dargs->files, filename))
    {
        if(is_file_changed(filename, dargs->meta->fdirname, dargs->meta->sdirname))
        {
            printf("%s CHANGED\n", filename);
            ++dargs->meta->stat.num_changed;
        }
        set_remove(dargs->files, filename);
    }
    else
    {
        printf("%s ADDED\n", filename);
        ++dargs->meta->stat.num_added;
    }
    free(filename);
}

void
seek_diff(struct metadata *meta)
{
    printf("Comparing directories \"%s\" and \"%s\"\n", meta->fdirname, meta->sdirname);
    clock_t start = clock();
    struct set *fdir_files = set_create(mstrcmp);
    process_files_in_dir(meta->fdirname, meta->fdirname, save_filename_a, fdir_files);
    process_files_in_dir(meta->sdirname, meta->sdirname, diff_file_a, &(struct diffargs) { .files = fdir_files, .meta = meta });
    meta->stat.num_removed += fdir_files->size;
    print_filenames(fdir_files->root, " DELETED");
    meta->stat.time_spent = (float) (clock() - start) / CLOCKS_PER_SEC;
    set_destroy(fdir_files);
}
