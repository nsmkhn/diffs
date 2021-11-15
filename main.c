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

#define NUM_ARGS 3
#define perror_and_exit do { perror(""); exit(EXIT_FAILURE); } while(0)
#define DIR_SEP_SIZE 1

struct diffstat
{
    size_t num_changed;
    size_t num_added;
    size_t num_removed;
    double time_spent;
};

struct metadata
{
    char *fdirname, *sdirname;
    struct diffstat stat;
};

bool
files_equal(char *f1, char *f2)
{
    int fd1 = open(f1, O_RDONLY);
    int fd2 = open(f2, O_RDONLY);
    if(fd1 == -1 || fd2 == -1)
        perror_and_exit;

    struct stat sb1, sb2;
    if(fstat(fd1, &sb1) == -1 || fstat(fd2, &sb2) == -1)
        perror_and_exit;
    if(sb2.st_size != sb1.st_size)
        return false;

    char *first = mmap(NULL, sb1.st_size, PROT_READ, MAP_PRIVATE, fd1, 0);
    char *second = mmap(NULL, sb2.st_size, PROT_READ, MAP_PRIVATE, fd2, 0);
    if(first == MAP_FAILED || second == MAP_FAILED)
        perror_and_exit;
    for(off_t i = 0; i < sb1.st_size; ++i)
        if(first[i] != second[i])
            return false;

    return true;
}

int
mstrcmp(void *s1, void *s2)
{
    return(strcmp(s1, s2));
}

void
print_filenames(struct set_node *root, char *suffix)
{
    if(!root)
        return;
    printf("%s%s\n", (char *) root->data, suffix);
    print_filenames(root->left, suffix);
    print_filenames(root->right, suffix);
}

void
build_dirname(char *buf, size_t buflen, char *basename, char *entry)
{
    bzero(buf, buflen);
    strcat(buf, basename);
    if(basename[strlen(basename)-1] != '/')
        buf[strlen(basename)] = '/';
    strcat(buf, entry);
}

char *
build_filename(char *dirbasename, char *dirname, char *entry)
{
    size_t len;
    char *filename;
    if(strlen(dirbasename) == strlen(dirname))
    {
        len = strlen(entry) + 1;
        filename = malloc(len);
        bzero(filename, len);
    }
    else
    {
        char *dir = dirname + strlen(dirbasename) + 1;
        len = strlen(dir) + DIR_SEP_SIZE + strlen(entry) + 1;
        filename = malloc(len);
        bzero(filename, len);
        strcat(filename, dir);
        filename[strlen(dir)] = '/';
    }
    strcat(filename, entry);

    return filename;
}

void
scan_dir(char *dirname, struct set *files, char *basename)
{
    DIR *dir = opendir(dirname);
    if(!dir)
        perror_and_exit;

    struct dirent *entry;
    errno = 0;
    while((entry = readdir(dir)))
    {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        if(entry->d_type == DT_DIR)
        {
            int namelen = strlen(dirname) + DIR_SEP_SIZE + strlen(entry->d_name) + 1;
            char name[namelen];
            build_dirname(name, namelen, dirname, entry->d_name);
            scan_dir(name, files, basename);
        }
        else
        {
            char *filename = build_filename(basename, dirname, entry->d_name);
            assert(set_insert(files, filename) != 0);
        }
        errno = 0;
    }
    if(errno)
        perror_and_exit;

    closedir(dir);
}

bool
is_file_changed(char *filename, char *fdir_basename, char *sdir_basename)
{
    int lenfirst = strlen(filename) + strlen(fdir_basename) + 1;
    int lensecond = strlen(filename) + strlen(sdir_basename) + 1;

    char namefirst[lenfirst + 1];
    bzero(namefirst, lenfirst);
    strcat(namefirst, fdir_basename);
    namefirst[strlen(fdir_basename)] = '/';
    strcat(namefirst, filename);

    char namesecond[lenfirst + 1];
    bzero(namesecond, lensecond);
    strcat(namesecond, sdir_basename);
    namesecond[strlen(sdir_basename)] = '/';
    strcat(namesecond, filename);

    return !files_equal(namefirst, namesecond);
}

void
process_node(struct set_node *root, struct set *s, struct metadata *meta)
{
    if(!root)
        return;
    if(set_contains(s, root->data))
    {
        if(is_file_changed(root->data, meta->fdirname, meta->sdirname))
        {
            printf("%s\tCHANGED\n", (char *) root->data);
            ++meta->stat.num_changed;
        }
        set_remove(s, root->data);
    }
    else
    {
        ++meta->stat.num_removed;
        printf("%s\tDELETED\n", (char *) root->data);
    }
    process_node(root->right, s, meta);
    process_node(root->left, s, meta);
}

void
print_diffstat(struct diffstat *stat)
{
    printf("Comparison finished in %f seconds.\n", stat->time_spent);
    printf("%lu files changed, %lu removed, %lu added\n",
            stat->num_changed, stat->num_removed, stat->num_added);
}

void
seek_diff(struct set *fdir_files, struct set *sdir_files,
          struct metadata *meta)
{
    printf("Comparing directories \"%s\" and \"%s\" ...\n",
            meta->fdirname, meta->sdirname);
    process_node(fdir_files->root, sdir_files, meta);
    meta->stat.num_added += sdir_files->size;
    print_filenames(sdir_files->root, "\tADDED");
}

int
main(int argc, char **argv)
{
    if(argc != NUM_ARGS)
    {
        fprintf(stderr, "Usage: %s <dir1> <dir2>\n", argv[0]);
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
