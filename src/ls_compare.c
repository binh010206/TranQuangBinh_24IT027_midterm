/*
 * ls_compare.c - Sorting logic for ls entries
 * Supports sort-by-size (-S), sort-by-time (-t), reverse (-r),
 * and falls back to lexicographic order by name.
 */

#include "ls_compare.h"
#include <string.h>
#include <stdlib.h>

/* File-scope pointer so qsort's comparator can access the config */
static const ls_config_t *sort_cfg = NULL;

/* Pick the relevant timestamp from an item according to flags */
static time_t pick_time(const ls_item_t *item, const ls_config_t *cfg)
{
    if (cfg->flag_ctime)
        return item->info.st_ctime;
    if (cfg->flag_atime)
        return item->info.st_atime;
    return item->info.st_mtime;
}

/* Comparator passed to qsort */
static int cmp_entries(const void *p1, const void *p2)
{
    const ls_item_t *a = *(const ls_item_t **)p1;
    const ls_item_t *b = *(const ls_item_t **)p2;
    int cmp = 0;

    /* First criterion: size (largest first) when -S is active */
    if (sort_cfg->flag_sort_size) {
        if (a->info.st_size > b->info.st_size)
            cmp = -1;
        else if (a->info.st_size < b->info.st_size)
            cmp = 1;
    }

    /* Second criterion: time (newest first) when -t is active */
    if (cmp == 0 && sort_cfg->flag_sort_time) {
        time_t ta = pick_time(a, sort_cfg);
        time_t tb = pick_time(b, sort_cfg);
        if (ta > tb)
            cmp = -1;
        else if (ta < tb)
            cmp = 1;
    }

    /* Fallback: alphabetical order by basename */
    if (cmp == 0)
        cmp = strcmp(a->name, b->name);

    /* Flip the result when -r (reverse) is requested */
    if (sort_cfg->flag_reverse)
        cmp = -cmp;

    return cmp;
}

void items_sort(ls_item_t **list, int n, const ls_config_t *cfg)
{
    if (cfg->flag_no_sort || list == NULL || n <= 1)
        return;

    sort_cfg = cfg;
    qsort(list, (size_t)n, sizeof(ls_item_t *), cmp_entries);
    sort_cfg = NULL;
}
