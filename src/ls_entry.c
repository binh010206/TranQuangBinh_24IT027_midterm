/*
 * ls_entry.c - File entry construction and directory reading
 * Builds ls_item_t objects from filesystem data.
 */

#include "ls_entry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

/* Concatenate parent directory and filename with a '/' separator */
static char *make_path(const char *dir, const char *name)
{
    size_t dlen, nlen;
    int    need_sep;
    char  *path;

    if (dir == NULL || dir[0] == '\0')
        return strdup(name);

    dlen     = strlen(dir);
    nlen     = strlen(name);
    need_sep = (dir[dlen - 1] != '/');

    path = malloc(dlen + nlen + (need_sep ? 2 : 1));
    if (path == NULL) {
        perror("ls: malloc");
        return NULL;
    }

    if (need_sep)
        sprintf(path, "%s/%s", dir, name);
    else
        sprintf(path, "%s%s", dir, name);

    return path;
}

ls_item_t *item_create(const char *dir, const char *filename)
{
    ls_item_t *item;

    item = calloc(1, sizeof(ls_item_t));
    if (item == NULL) {
        perror("ls: calloc");
        return NULL;
    }

    item->name     = strdup(filename);
    item->fullpath = make_path(dir, filename);

    if (item->name == NULL || item->fullpath == NULL) {
        item_destroy(item);
        return NULL;
    }

    /* lstat() preserves symlink info (does not follow the link) */
    if (lstat(item->fullpath, &item->info) == -1) {
        fprintf(stderr, "ls: %s: %s\n", item->fullpath, strerror(errno));
        item_destroy(item);
        return NULL;
    }

    /* If the entry is a symbolic link, read its target path */
    if (S_ISLNK(item->info.st_mode)) {
        char    buf[1024];
        ssize_t len = readlink(item->fullpath, buf, sizeof(buf) - 1);
        if (len >= 0) {
            buf[len] = '\0';
            item->symlink_dest = strdup(buf);
        }
    }

    return item;
}

void item_destroy(ls_item_t *item)
{
    if (item == NULL)
        return;
    free(item->name);
    free(item->fullpath);
    free(item->symlink_dest);
    free(item);
}

void items_free(ls_item_t **list, int n)
{
    int i;
    if (list == NULL)
        return;
    for (i = 0; i < n; i++)
        item_destroy(list[i]);
    free(list);
}

ls_item_t **dir_read(const char *dirpath, const ls_config_t *cfg,
                     int *out_count)
{
    DIR           *dp;
    struct dirent *de;
    ls_item_t    **list     = NULL;
    int            capacity = 0;
    int            count    = 0;

    dp = opendir(dirpath);
    if (dp == NULL) {
        fprintf(stderr, "ls: %s: %s\n", dirpath, strerror(errno));
        *out_count = 0;
        return NULL;
    }

    while ((de = readdir(dp)) != NULL) {
        /* Decide whether to include dot-entries */
        if (de->d_name[0] == '.') {
            if (cfg->flag_all) {
                /* -a: show everything, including . and .. */
            } else if (cfg->flag_almost_all) {
                /* -A: show dot-files but skip . and .. */
                if (strcmp(de->d_name, ".") == 0 ||
                    strcmp(de->d_name, "..") == 0)
                    continue;
            } else {
                /* Default behaviour: hide all dot-entries */
                continue;
            }
        }

        /* Expand array capacity when needed */
        if (count >= capacity) {
            capacity = (capacity == 0) ? 32 : capacity * 2;
            list = realloc(list, capacity * sizeof(ls_item_t *));
            if (list == NULL) {
                perror("ls: realloc");
                closedir(dp);
                *out_count = 0;
                return NULL;
            }
        }

        {
            ls_item_t *item = item_create(dirpath, de->d_name);
            if (item != NULL)
                list[count++] = item;
        }
    }

    closedir(dp);
    *out_count = count;
    return list;
}
