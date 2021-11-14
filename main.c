#include "set.h"
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define perror_and_exit do { perror(""); exit(EXIT_FAILURE); } while(0)

struct set *s;

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
    printf("%s\n", (char *) root->data);
    print_set(root->left);
    print_set(root->right);
}

void
scan_dir(char *dirname)
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
            scan_dir(name);
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
            
            if(!set_insert(s, filename))
                free(filename);
        }
        errno = 0;
    }
    if(errno)
        perror_and_exit;

    closedir(dir);
}

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s <dir>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    s = set_create(mstrcmp);
    for(int i = 1; i < argc; ++i)
    {
        scan_dir(argv[i]);
        print_set(s->root);
    }

    //struct dirent {
    //    ino_t          d_ino;       /* Inode number */
    //    off_t          d_off;       /* Not an offset; see below */
    //    unsigned short d_reclen;    /* Length of this record */
    //    unsigned char  d_type;      /* Type of file; not supported
    //                                   by all filesystem types */
    //    char           d_name[256]; /* Null-terminated filename */
    //};
    return 0;
}
