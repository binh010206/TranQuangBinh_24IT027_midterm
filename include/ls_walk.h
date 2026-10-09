/*
 * ls_walk.h - Operand processing and directory traversal
 * Separates files from directories, handles -R recursion.
 */

#ifndef LS_WALK_H
#define LS_WALK_H

#include "ls_args.h"

/*
 * Main entry point after argument parsing.
 * Processes every operand in argv[first_arg..argc-1].
 * Returns 0 on success, >0 on error.
 */
int walk_args(int argc, char *argv[], int first_arg, const ls_config_t *cfg);

#endif /* LS_WALK_H */
