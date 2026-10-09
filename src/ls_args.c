/*
 * ls_args.c - Parsing of command-line flags for ls
 * Uses getopt(3) to iterate over the flag string "AacdFfhiklnqRrSstuw".
 * Mutually exclusive pairs override each other (last flag wins).
 */

#include "ls_args.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Set every flag to zero, then apply environment-dependent defaults */
void config_init(ls_config_t *cfg)
{
    cfg->flag_all          = 0;
    cfg->flag_almost_all   = 0;
    cfg->flag_ctime        = 0;
    cfg->flag_atime        = 0;
    cfg->flag_dir_only     = 0;
    cfg->flag_recursive    = 0;
    cfg->flag_classify     = 0;
    cfg->flag_no_sort      = 0;
    cfg->flag_human        = 0;
    cfg->flag_inode        = 0;
    cfg->flag_kilo         = 0;
    cfg->flag_long         = 0;
    cfg->flag_numeric      = 0;
    cfg->flag_hide_special = 0;
    cfg->flag_reverse      = 0;
    cfg->flag_sort_size    = 0;
    cfg->flag_blocks       = 0;
    cfg->flag_sort_time    = 0;
    cfg->flag_raw_chars    = 0;

    /* The super-user always sees nearly-all entries */
    if (geteuid() == 0)
        cfg->flag_almost_all = 1;

    /* When stdout is a terminal, mask non-printable characters */
    if (isatty(STDOUT_FILENO))
        cfg->flag_hide_special = 1;
    else
        cfg->flag_raw_chars = 1;
}

int config_parse(int argc, char *argv[], ls_config_t *cfg)
{
    int ch;

    config_init(cfg);

    while ((ch = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1) {
        switch (ch) {
        case 'A':
            cfg->flag_almost_all = 1;
            cfg->flag_all        = 0;
            break;
        case 'a':
            cfg->flag_all        = 1;
            cfg->flag_almost_all = 0;
            break;
        case 'c':
            cfg->flag_ctime = 1;
            cfg->flag_atime = 0;      /* -c / -u override each other */
            break;
        case 'd':
            cfg->flag_dir_only  = 1;
            cfg->flag_recursive = 0;  /* -d / -R override each other */
            break;
        case 'F':
            cfg->flag_classify = 1;
            break;
        case 'f':
            cfg->flag_no_sort    = 1;
            cfg->flag_all        = 1;    /* -f implies -a */
            cfg->flag_almost_all = 0;
            break;
        case 'h':
            cfg->flag_human = 1;
            cfg->flag_kilo  = 0;      /* -h / -k override each other */
            break;
        case 'i':
            cfg->flag_inode = 1;
            break;
        case 'k':
            cfg->flag_kilo  = 1;
            cfg->flag_human = 0;      /* -k / -h override each other */
            break;
        case 'l':
            cfg->flag_long    = 1;
            cfg->flag_numeric = 0;    /* -l / -n override each other */
            break;
        case 'n':
            cfg->flag_numeric = 1;
            cfg->flag_long    = 0;    /* -n / -l override each other */
            break;
        case 'q':
            cfg->flag_hide_special = 1;
            cfg->flag_raw_chars    = 0;  /* -q / -w override each other */
            break;
        case 'R':
            cfg->flag_recursive = 1;
            cfg->flag_dir_only  = 0;  /* -R / -d override each other */
            break;
        case 'r':
            cfg->flag_reverse = 1;
            break;
        case 'S':
            cfg->flag_sort_size = 1;
            break;
        case 's':
            cfg->flag_blocks = 1;
            break;
        case 't':
            cfg->flag_sort_time = 1;
            break;
        case 'u':
            cfg->flag_atime = 1;
            cfg->flag_ctime = 0;      /* -u / -c override each other */
            break;
        case 'w':
            cfg->flag_raw_chars    = 1;
            cfg->flag_hide_special = 0;  /* -w / -q override each other */
            break;
        default:
            fprintf(stderr, "usage: ls [-AacdFfhiklnqRrSstuw] [file ...]\n");
            exit(EXIT_FAILURE);
        }
    }

    return optind;
}
