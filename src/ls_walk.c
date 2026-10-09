/*
 * ls_walk.c - Operand processing and recursive directory traversal
 * Separates command-line operands into files and directories,
 * displays files first, then processes each directory.
 */

#include "ls_walk.h"
#include "ls_entry.h"
#include "ls_compare.h"
#include "ls_format.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

/* Recursively list a directory and its subdirectories when -R is set */
static void process_dir(const char *path, const ls_config_t *cfg,
                        int show_header, int *status)
{
    int          count = 0;
    ls_item_t  **entries;
    int          i;

    if (show_header)
        printf("%s:\n", path);

    entries = dir_read(path, cfg, &count);
    if (entries == NULL && count == 0) {
        *status = 1;
        return;
    }

    items_sort(entries, count, cfg);
    format_output(entries, count, cfg, 1);

    /* If -R is active, descend into each subdirectory */
    if (cfg->flag_recursive) {
        for (i = 0; i < count; i++) {
            if (!S_ISDIR(entries[i]->info.st_mode))
                continue;

            /* Never recurse into . or .. (infinite loop) */
            if (strcmp(entries[i]->name, ".")  == 0 ||
                strcmp(entries[i]->name, "..") == 0)
                continue;

            putchar('\n');
            process_dir(entries[i]->fullpath, cfg, 1, status);
        }
    }

    items_free(entries, count);
}

int walk_args(int argc, char *argv[], int first_arg, const ls_config_t *cfg)
{
    int status = 0;
    int nargs  = argc - first_arg;
    int i;

    /* ---- No operands: list current directory ---- */
    if (nargs == 0) {
        if (cfg->flag_dir_only) {
            /* -d: show "." itself rather than its contents */
            ls_item_t *dot = item_create("", ".");
            if (dot != NULL) {
                ls_item_t *arr[1];
                arr[0] = dot;
                format_output(arr, 1, cfg, 0);
                item_destroy(dot);
            } else {
                status = 1;
            }
        } else {
            process_dir(".", cfg, 0, &status);
        }
        return status;
    }

    /* ---- With operands: partition into files and directories ---- */
    {
        ls_item_t **file_list = NULL;
        ls_item_t **dir_list  = NULL;
        int         nfiles    = 0,  ndirs    = 0;
        int         file_cap  = 0,  dir_cap  = 0;

        for (i = first_arg; i < argc; i++) {
            ls_item_t *item = item_create("", argv[i]);
            int        is_directory;

            if (item == NULL) {
                status = 1;
                continue;
            }

            is_directory = S_ISDIR(item->info.st_mode);

            /*
             * When -d is NOT specified, symbolic links in the argument list
             * that resolve to directories are indirected through.
             */
            if (!cfg->flag_dir_only && S_ISLNK(item->info.st_mode)) {
                struct stat tgt_st;
                if (stat(item->fullpath, &tgt_st) == 0 && S_ISDIR(tgt_st.st_mode))
                    is_directory = 1;
            }

            /* -d: treat directories as ordinary files */
            if (cfg->flag_dir_only)
                is_directory = 0;

            if (is_directory) {
                if (ndirs >= dir_cap) {
                    dir_cap  = (dir_cap == 0) ? 8 : dir_cap * 2;
                    dir_list = realloc(dir_list,
                                       dir_cap * sizeof(ls_item_t *));
                }
                dir_list[ndirs++] = item;
            } else {
                if (nfiles >= file_cap) {
                    file_cap  = (file_cap == 0) ? 8 : file_cap * 2;
                    file_list = realloc(file_list,
                                        file_cap * sizeof(ls_item_t *));
                }
                file_list[nfiles++] = item;
            }
        }

        /* Display non-directory operands first */
        if (nfiles > 0) {
            items_sort(file_list, nfiles, cfg);
            format_output(file_list, nfiles, cfg, 0);
            items_free(file_list, nfiles);
        }

        /* Then process each directory operand */
        if (ndirs > 0) {
            items_sort(dir_list, ndirs, cfg);

            for (i = 0; i < ndirs; i++) {
                /* Blank line between groups */
                if (nfiles > 0 || i > 0)
                    putchar('\n');
                process_dir(dir_list[i]->fullpath, cfg,
                            (ndirs + nfiles > 1) ? 1 : 0, &status);
            }
            items_free(dir_list, ndirs);
        }
    }

    return status;
}
