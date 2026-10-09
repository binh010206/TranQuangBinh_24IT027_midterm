/*
 * ls_format.c - Output formatting for ls utility
 * Implements both long format (-l/-n) and short listing,
 * along with helpers for permission strings, human-readable sizes,
 * time formatting, type suffixes (-F), and non-printable handling (-q/-w).
 */

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "ls_format.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef __linux__
#include <sys/sysmacros.h>
#endif

/* ------------------------------------------------------------------ */
/*  Internal helpers                                                   */
/* ------------------------------------------------------------------ */

/* Return the block-size unit from $BLOCKSIZE or default to 512 */
static long block_unit(void)
{
    char *env = getenv("BLOCKSIZE");
    if (env != NULL && *env != '\0') {
        long val = atol(env);
        if (val > 0)
            return val;
    }
    return 512;
}

/* Format a byte count in human-readable form (e.g. 4.0K, 26K) */
static void human_size(off_t bytes, char *buf, size_t buflen)
{
    const char *suffixes = "BKMGTP";
    double      sz       = (double)bytes;
    int         idx      = 0;

    while (sz >= 1024.0 && idx < 5) {
        sz /= 1024.0;
        idx++;
    }

    if (idx == 0)
        snprintf(buf, buflen, "%4lld%c", (long long)bytes, suffixes[idx]);
    else if (sz < 10.0)
        snprintf(buf, buflen, "%3.1f%c", sz, suffixes[idx]);
    else
        snprintf(buf, buflen, "%3.0f%c", sz, suffixes[idx]);
}

/* Build the classic 10-character permission string (e.g. "drwxr-xr-x") */
static void perm_string(mode_t mode, char *out)
{
    /* Byte 0: entry type */
    switch (mode & S_IFMT) {
    case S_IFDIR:  out[0] = 'd'; break;
    case S_IFLNK:  out[0] = 'l'; break;
    case S_IFBLK:  out[0] = 'b'; break;
    case S_IFCHR:  out[0] = 'c'; break;
    case S_IFIFO:  out[0] = 'p'; break;
    case S_IFSOCK: out[0] = 's'; break;
#ifdef S_IFWHT
    case S_IFWHT:  out[0] = 'w'; break;
#endif
    default:       out[0] = '-'; break;
    }

    /* Bytes 1-3: owner permissions */
    out[1] = (mode & S_IRUSR) ? 'r' : '-';
    out[2] = (mode & S_IWUSR) ? 'w' : '-';
    if (mode & S_ISUID)
        out[3] = (mode & S_IXUSR) ? 's' : 'S';
    else
        out[3] = (mode & S_IXUSR) ? 'x' : '-';

    /* Bytes 4-6: group permissions */
    out[4] = (mode & S_IRGRP) ? 'r' : '-';
    out[5] = (mode & S_IWGRP) ? 'w' : '-';
    if (mode & S_ISGID)
        out[6] = (mode & S_IXGRP) ? 's' : 'S';
    else
        out[6] = (mode & S_IXGRP) ? 'x' : '-';

    /* Bytes 7-9: other permissions */
    out[7] = (mode & S_IROTH) ? 'r' : '-';
    out[8] = (mode & S_IWOTH) ? 'w' : '-';
    if (mode & S_ISVTX)
        out[9] = (mode & S_IXOTH) ? 't' : 'T';
    else
        out[9] = (mode & S_IXOTH) ? 'x' : '-';

    out[10] = '\0';
}

/* Format the timestamp shown in long listing */
static void fmt_time(const ls_item_t *item, const ls_config_t *cfg,
                     char *buf, size_t buflen)
{
    time_t     t;
    time_t     now;
    struct tm *tm_p;

    /* Choose which timestamp to display */
    if (cfg->flag_ctime)
        t = item->info.st_ctime;
    else if (cfg->flag_atime)
        t = item->info.st_atime;
    else
        t = item->info.st_mtime;

    now  = time(NULL);
    tm_p = localtime(&t);

    /*
     * If the timestamp is more than ~6 months old (or in the future),
     * show the year; otherwise show hour:minute.
     */
    if (now - t > 15724800 || t > now)
        strftime(buf, buflen, "%b %e  %Y", tm_p);
    else
        strftime(buf, buflen, "%b %e %H:%M", tm_p);
}

/* Print a filename, replacing non-printable chars with '?' if -q */
static void print_name(const char *name, const ls_config_t *cfg)
{
    const char *p;

    for (p = name; *p != '\0'; p++) {
        if (cfg->flag_hide_special && !isprint((unsigned char)*p))
            putchar('?');
        else
            putchar(*p);
    }
}

/* Append the -F type indicator character after a filename */
static void print_suffix(mode_t mode)
{
    if (S_ISDIR(mode))
        putchar('/');
    else if (S_ISLNK(mode))
        putchar('@');
    else if (S_ISSOCK(mode))
        putchar('=');
    else if (S_ISFIFO(mode))
        putchar('|');
#ifdef S_ISWHT
    else if (S_ISWHT(mode))
        putchar('%');
#endif
    else if (mode & (S_IXUSR | S_IXGRP | S_IXOTH))
        putchar('*');
}

/* Compute and print block count for one entry (used by -s) */
static void print_blocks(const ls_item_t *item, const ls_config_t *cfg)
{
    if (cfg->flag_human) {
        char sb[16];
        human_size(item->info.st_blocks * 512, sb, sizeof(sb));
        printf("%5s ", sb);
    } else if (cfg->flag_kilo) {
        printf("%5lld ",
               (long long)((item->info.st_blocks * 512 + 1023) / 1024));
    } else {
        long bu = block_unit();
        printf("%5lld ",
               (long long)((item->info.st_blocks * 512 + bu - 1) / bu));
    }
}

/* ------------------------------------------------------------------ */
/*  Per-entry printing                                                 */
/* ------------------------------------------------------------------ */

/* Print one entry in long format (-l or -n) */
static void print_long(const ls_item_t *item, const ls_config_t *cfg)
{
    char perms[12];
    char timebuf[64];

    perm_string(item->info.st_mode, perms);
    fmt_time(item, cfg, timebuf, sizeof(timebuf));

    /* Optional: inode number */
    if (cfg->flag_inode)
        printf("%5lu ", (unsigned long)item->info.st_ino);

    /* Optional: block count */
    if (cfg->flag_blocks)
        print_blocks(item, cfg);

    /* Permissions and hard-link count */
    printf("%s %2lu ", perms, (unsigned long)item->info.st_nlink);

    /* Owner name or numeric UID */
    if (cfg->flag_numeric) {
        printf("%-8u ", (unsigned)item->info.st_uid);
    } else {
        struct passwd *pw = getpwuid(item->info.st_uid);
        if (pw != NULL)
            printf("%-8s ", pw->pw_name);
        else
            printf("%-8u ", (unsigned)item->info.st_uid);
    }

    /* Group name or numeric GID */
    if (cfg->flag_numeric) {
        printf("%-8u ", (unsigned)item->info.st_gid);
    } else {
        struct group *gr = getgrgid(item->info.st_gid);
        if (gr != NULL)
            printf("%-8s ", gr->gr_name);
        else
            printf("%-8u ", (unsigned)item->info.st_gid);
    }

    /* Size field: device numbers for special files, size otherwise */
    if (S_ISCHR(item->info.st_mode) || S_ISBLK(item->info.st_mode)) {
        printf("%3u, %3u ",
               major(item->info.st_rdev), minor(item->info.st_rdev));
    } else if (cfg->flag_human) {
        char sb[16];
        human_size(item->info.st_size, sb, sizeof(sb));
        printf("%5s ", sb);
    } else {
        printf("%5lld ", (long long)item->info.st_size);
    }

    /* Timestamp */
    printf("%s ", timebuf);

    /* Filename */
    print_name(item->name, cfg);

    /* -F suffix */
    if (cfg->flag_classify)
        print_suffix(item->info.st_mode);

    /* Symlink target */
    if (S_ISLNK(item->info.st_mode) && item->symlink_dest != NULL)
        printf(" -> %s", item->symlink_dest);

    putchar('\n');
}

/* Print one entry in short format (one per line) */
static void print_short(const ls_item_t *item, const ls_config_t *cfg)
{
    if (cfg->flag_inode)
        printf("%5lu ", (unsigned long)item->info.st_ino);

    if (cfg->flag_blocks)
        print_blocks(item, cfg);

    print_name(item->name, cfg);

    if (cfg->flag_classify)
        print_suffix(item->info.st_mode);

    putchar('\n');
}

/* ------------------------------------------------------------------ */
/*  Public interface                                                   */
/* ------------------------------------------------------------------ */

void format_output(ls_item_t **list, int n, const ls_config_t *cfg,
                   int is_dir_content)
{
    int i;
    int use_long = cfg->flag_long || cfg->flag_numeric;

    /* Print "total" line when listing directory contents with -l or -s */
    if (is_dir_content && (use_long || cfg->flag_blocks)) {
        blkcnt_t total = 0;
        for (i = 0; i < n; i++)
            total += list[i]->info.st_blocks;

        if (cfg->flag_kilo) {
            printf("total %lld\n",
                   (long long)((total * 512 + 1023) / 1024));
        } else {
            long bu = block_unit();
            printf("total %lld\n",
                   (long long)((total * 512 + bu - 1) / bu));
        }
    }

    /* Render each entry */
    for (i = 0; i < n; i++) {
        if (use_long)
            print_long(list[i], cfg);
        else
            print_short(list[i], cfg);
    }
}
