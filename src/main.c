/*
 * main.c - Entry point for the ls utility
 * Parses flags, then delegates to walk_args() for processing.
 */

#include "ls_args.h"
#include "ls_walk.h"

int main(int argc, char *argv[])
{
    ls_config_t cfg;
    int first_arg;

    /* Step 1: parse command-line flags into cfg */
    first_arg = config_parse(argc, argv, &cfg);

    /* Step 2: process all operands and produce output */
    return walk_args(argc, argv, first_arg, &cfg);
}
