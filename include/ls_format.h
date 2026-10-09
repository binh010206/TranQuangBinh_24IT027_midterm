/*
 * ls_format.h - Output formatting for ls utility
 * Handles both long (-l) and short listing display.
 */

#ifndef LS_FORMAT_H
#define LS_FORMAT_H

#include "ls_entry.h"
#include "ls_args.h"

/*
 * Render sorted entries to stdout.
 *   list           - array of item pointers
 *   n              - number of items
 *   cfg            - active configuration flags
 *   is_dir_content - 1 when printing contents of a directory (shows "total")
 */
void format_output(ls_item_t **list, int n, const ls_config_t *cfg,
                   int is_dir_content);

#endif /* LS_FORMAT_H */
