#include "set.h"
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define perror_and_exit do { perror(""); exit(EXIT_FAILURE); } while(0)

struct set *s;
char *g_firstdir;
char *g_seconddir;

bool
is_file_changed(char *filename)
{
    int lenfirst = strlen(filename) + strlen(g_firstdir) + 1;
    int lensecond = strlen(filename) + strlen(g_seconddir) + 1;

    char namefirst[lenfirst + 1];
    bzero(namefirst, lenfirst);
    strcat(namefirst, g_firstdir);
    namefirst[strlen(g_firstdir)] = '/';
    strcat(namefirst, filename);

    char namesecond[lenfirst + 1];
    bzero(namesecond, lensecond);
    strcat(namesecond, g_seconddir);
    namesecond[strlen(g_seconddir)] = '/';
    strcat(namesecond, filename);

    printf("namefirst: %s, namesecond: %s\n", namefirst, namesecond);

    int fd1 = open(namefirst, O_RDONLY);
    int fd2 = open(namesecond, O_RDONLY);
    if(fd1 == -1 || fd2 == -1)
        perror_and_exit;

    struct stat sb1, sb2;
    if(fstat(fd1, &sb1) == -1)
        perror_and_exit;
    if(fstat(fd2, &sb2) == -1)
        perror_and_exit;
    if(sb2.st_size != sb1.st_size)
        return true;

    char *first = (char *) mmap(NULL, sb1.st_size, PROT_READ, MAP_PRIVATE, fd1, 0);
    char *second = (char *) mmap(NULL, sb2.st_size, PROT_READ, MAP_PRIVATE, fd1, 0);
    if(first == MAP_FAILED || second == MAP_FAILED)
        perror_and_exit;
    for(off_t i = 0; i < sb1.st_size; ++i)
        if(first[i] != second[i])
            return true;

    return false;
}

int
mstrcmp(void *s1, void *s2)
{
    return(strcmp(s1, s2));
}

void
print_set(struct set_node *root)
{
    if(!root)
        return;
    printf("%s\tDELETED\n", (char *) root->data);
    print_set(root->left);
    print_set(root->right);
}

void
scan_dir(char *dirname, bool compare)
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
            int namelen = strlen(dirname) + strlen(entry->d_name) + 2;
            char name[namelen];
            bzero(name, namelen);
            strcat(name, dirname);
            if(dirname[strlen(dirname)-1] != '/')
                name[strlen(dirname)] = '/';
            strcat(name, entry->d_name);
            printf("dir: %s\n", name);
            scan_dir(name, compare);
        }
        else
        {
            char *dir = strstr(dirname, "/");
            char *filename;
            int len;
            if(dir && strlen(dir) > 1)
            {
                ++dir;
                len = strlen(dir) + 1 + strlen(entry->d_name);
                filename = malloc(len + 1);
                bzero(filename, len);
                strcat(filename, dir);
                filename[strlen(dir)] = '/';
            }
            else
            {
                len = strlen(entry->d_name);
                filename = malloc(len + 1);
                bzero(filename, len);
            }
            strcat(filename, entry->d_name);
            filename[len] = '\0';
            
            if(compare)
            {
                int contains = set_contains(s, filename);
                if(contains)
                {
                    // check if file was changed
                    bool changed = is_file_changed(filename);
                    if(changed)
                        printf("%s\tCHANGED\n", filename);

                    set_remove(s, filename);
                }
                else
                {
                    printf("%s\tADDED\n", filename);
                }
            }
            else
            {
                assert(set_insert(s, filename) != 0);
            }
        }
        errno = 0;
    }
    if(errno)
        perror_and_exit;

    closedir(dir);
}

int main(int argc, char **argv)
{
    //struct dirent {
    //    ino_t          d_ino;       /* Inode number */
    //    off_t          d_off;       /* Not an offset; see below */
    //    unsigned short d_reclen;    /* Length of this record */
    //    unsigned char  d_type;      /* Type of file; not supported
    //                                   by all filesystem types */
    //    char           d_name[256]; /* Null-terminated filename */
    //};

    if(argc != 3)
    {
        fprintf(stderr, "Usage: %s <dir1> <dir2>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    s = set_create(mstrcmp);
    g_firstdir = argv[1];
    g_seconddir = argv[2];

    scan_dir(g_firstdir, false);
    scan_dir(g_seconddir, true);
    
    print_set(s->root);
    
    return 0;
}
