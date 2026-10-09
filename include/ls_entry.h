/*
 * ls_entry.h - File entry abstraction for ls utility
 * Encapsulates stat information, filename, and symlink target.
 */

#ifndef LS_ENTRY_H
#define LS_ENTRY_H

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "ls_args.h"

/* Represents one filesystem entry ready for display */
typedef struct {
    char        *name;          /* basename of the entry */
    char        *fullpath;      /* complete path (parent + name) */
    struct stat  info;          /* result of lstat() on fullpath */
    char        *symlink_dest;  /* readlink target, or NULL */
} ls_item_t;

/* Create an item by joining dir/filename and calling lstat */
ls_item_t  *item_create(const char *dir, const char *filename);

/* Release all memory associated with one item */
void        item_destroy(ls_item_t *item);

/* Scan a directory and return a malloc'd array of item pointers */
ls_item_t **dir_read(const char *dirpath, const ls_config_t *cfg,
                     int *out_count);

/* Free an entire array of items */
void        items_free(ls_item_t **list, int n);

#endif /* LS_ENTRY_H */
