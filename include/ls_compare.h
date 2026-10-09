/*
 * ls_compare.h - Sorting interface for ls entries
 * Provides a single function to sort an array of ls_item_t pointers.
 */

#ifndef LS_COMPARE_H
#define LS_COMPARE_H

#include "ls_entry.h"
#include "ls_args.h"

/* Sort the item array in-place according to the active config flags */
void items_sort(ls_item_t **list, int n, const ls_config_t *cfg);

#endif /* LS_COMPARE_H */
