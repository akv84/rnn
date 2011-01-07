/*
    Copyright (c) 2011, Jun Namikawa <jnamika@gmail.com>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#ifdef ENABLE_MTRACE
#include <mcheck.h>
#endif
#include "mt19937ar.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "utils.h"
#include "main.h"
#include "rnn_runner.h"


#define TO_STRING_I(s) #s
#define TO_STRING(s) TO_STRING_I(s)

void display_help (void)
{
    puts("rnn-generate - a program to display output of recurrent neural "
            "networks");
    puts("");
    puts("Usage: rnn-generate [-s seed] [-n steps] [-i index] rnn-file");
    puts("Usage: rnn-generate [-v] [-h]");
    puts("");
    puts("Available options are:");
    puts("-s seed");
    puts("    `seed' is the seed for the initialization of random number "
            "generator, which specifies a starting point for the random number "
            "sequence, and provides for restarting at the same point. If this "
            "option is omitted, the current system time is used.");
    puts("-n steps");
    puts("    Number of steps to generate output of a network. "
            "Default is " TO_STRING(STEP_SIZE) ".");
    puts("-i index");
    puts("    Index of an initial state corresponding to a training example. "
            "Default is " TO_STRING(INDEX) " (use a random initial state).");
    puts("-v");
    puts("    Prints the version information and exit.");
    puts("-h");
    puts("    Prints this help and exit.");
    puts("");
    puts("Program execution:");
    puts("First, rnn-generate reads the rnn-file (ex: rnn.dat) generated by "
            "rnn-learn in order to setup model parameters. Next, it displays "
            "output of a network up to given steps with respect to the initial "
            "state corresponding to given index.");
}

static void display_version (void)
{
    printf("rnn-generate version %s\n", TO_STRING(VERSION));
}


int main (int argc, char *argv[])
{
#ifdef ENABLE_MTRACE
    mtrace();
#endif
    unsigned long seed;
    long step_size = STEP_SIZE;
    int index = INDEX;

    // 0 < seed < 4294967296
    seed = (((unsigned long)time(NULL)) % 4294967295) + 1;

    int opt;
    while ((opt = getopt(argc, argv, "s:n:i:hv")) != -1) {
        switch (opt) {
            case 's':
                seed = strtoul(optarg, NULL, 0);
                break;
            case 'n':
                step_size = atol(optarg);
                break;
            case 'i':
                index = atoi(optarg);
                break;
            case 'v':
                display_version();
                exit(EXIT_SUCCESS);
            case 'h':
                display_help();
                exit(EXIT_SUCCESS);
            default: /* '?' */
                fprintf(stderr, "Try `rnn-generate -h' for more "
                        "information.\n");
                exit(EXIT_SUCCESS);
        }
    }
    if (seed <= 0) {
        print_error_msg("seed for random number generator not in valid "
                "range: x >= 1 (integer)");
        exit(EXIT_FAILURE);
    }
    if (optind >= argc) {
        print_error_msg("Usage: rnn-generate [-s seed] [-n steps] [-i index] "
                "rnn-file\nTry `rnn-generate -h' for more information.");
        exit(EXIT_SUCCESS);
    }

    init_genrand(seed);

    struct rnn_runner runner;
    FILE *fp;
    if ((fp = fopen(argv[optind], "r")) == NULL) {
        print_error_msg("cannot open %s", argv[optind]);
        exit(EXIT_FAILURE);
    }
    init_rnn_runner(&runner, fp);
    fclose(fp);

    const int out_state_size = rnn_out_state_size_from_runner(&runner);
    set_init_state_of_rnn_runner(&runner, index);
    for (long n = 0; n < step_size; n++) {
        update_rnn_runner(&runner);
        double *out_state = rnn_out_state_from_runner(&runner);
        for (int i = 0; i < out_state_size; i++) {
            printf("%f%c", out_state[i], i < out_state_size - 1 ? '\t' : '\n');
        }
    }
    free_rnn_runner(&runner);

#ifdef ENABLE_MTRACE
    muntrace();
#endif
    return EXIT_SUCCESS;
}

