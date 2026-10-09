/*
 * ls_args.h - Command-line argument parsing for ls utility
 * Defines the configuration structure and parsing interface.
 */

#ifndef LS_ARGS_H
#define LS_ARGS_H

#include <unistd.h>

/* Configuration structure: stores all parsed command-line flags */
typedef struct {
    int flag_all;           /* -a: include entries beginning with '.' */
    int flag_almost_all;    /* -A: like -a but exclude . and .. */
    int flag_ctime;         /* -c: use status change time */
    int flag_atime;         /* -u: use last access time */
    int flag_dir_only;      /* -d: list directories as plain files */
    int flag_recursive;     /* -R: recurse into subdirectories */
    int flag_classify;      /* -F: append type indicator character */
    int flag_no_sort;       /* -f: disable sorting entirely */
    int flag_human;         /* -h: human-readable size format */
    int flag_inode;         /* -i: display inode serial number */
    int flag_kilo;          /* -k: display sizes in kilobytes */
    int flag_long;          /* -l: use long listing format */
    int flag_numeric;       /* -n: like -l but show numeric uid/gid */
    int flag_hide_special;  /* -q: replace non-printable with '?' */
    int flag_reverse;       /* -r: reverse sort order */
    int flag_sort_size;     /* -S: sort by file size descending */
    int flag_blocks;        /* -s: show allocated disk blocks */
    int flag_sort_time;     /* -t: sort by modification time */
    int flag_raw_chars;     /* -w: output non-printable chars raw */
} ls_config_t;

/* Initialize config with sensible defaults */
void config_init(ls_config_t *cfg);

/* Parse argv flags; returns the index of the first non-option argument */
int  config_parse(int argc, char *argv[], ls_config_t *cfg);

#endif /* LS_ARGS_H */
