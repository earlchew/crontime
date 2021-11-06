/* -*- c-basic-offset:4; indent-tabs-mode:nil -*- vi: set sw=4 et: */
/*
// Copyright (c) 2021, Earl Chew
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the names of the authors of source code nor the names
//       of the contributors to the source code may be used to endorse or
//       promote products derived from this software without specific
//       prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL EARL CHEW BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "civiltime.h"
#include "die.h"
#include "parse.h"
#include "schedule.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* -------------------------------------------------------------------------- */
static const int DefaultJitterOpt = 5 * 60;

static const int MinJitterOpt = 0;
static const int MaxJitterOpt = 24 * 60 * 60;

static int JitterOpt = DefaultJitterOpt;

/* -------------------------------------------------------------------------- */
static void
usage(void)
{
    fprintf(
        stderr,
        "usage: %s [ options ] time [ schedule ] [ < schedule ]\n"
        "\n"
        "options:\n"
        "  -j,--jitter N   Jitter the schedule by N seconds [default: 300]\n"
        "\n"
        "arguments:\n"
        "  time       Time specific as Unix epoch (eg 1636919408)\n"
        "  schedule   Schedule using crontab(5) expression (eg * * * * *)\n",
        program_invocation_short_name);
    die(0);
}

/* -------------------------------------------------------------------------- */
static int
crontime(
    const struct CivilTime *aCivilTime,
    time_t aJitterPeriod,
    const char *aSchedule)
{
    int rc = -1;

    struct Schedule schedule;

    if (!initSchedule(&schedule, aSchedule))
        goto Finally;

    time_t scheduled = querySchedule(&schedule, aCivilTime, aJitterPeriod);
    if (-1 == scheduled)
        goto Finally;

    printf("%lld\n", (long long) scheduled);

    rc = 0;

Finally:

    return rc;
}

/* -------------------------------------------------------------------------- */
static char **
parseOptions(int argc, char **argv)
{
    int rc = -1;

    static struct option LongOptions[] = {
        {"jitter", required_argument, 0, 'j' },
        {"help",   no_argument,       0, '?' },
        {0,        0,                 0,  0 },
    };

    while (1) {

        int opt = getopt_long(argc, argv, "j:", LongOptions, 0);
        if (-1 == opt)
            break;

        switch (opt) {

        default:
            usage();
            goto Finally;

        case 'j':
            {
                unsigned long long jitterPeriod;

                const char *jitterPeriodEndPtr =
                    parseULongLong(&jitterPeriod, optarg);

                if (!jitterPeriodEndPtr || *jitterPeriodEndPtr)
                    die("Cannot parse jitter period %s", optarg);

                if (jitterPeriod < MinJitterOpt ||
                    jitterPeriod > MaxJitterOpt)

                    die("Jitter period %llu lies outside range [%d,%d]",
                        jitterPeriod, MinJitterOpt, MaxJitterOpt);

                JitterOpt = jitterPeriod;
            }
            break;
        }
    }

    rc = 0;

Finally:

    return rc ? 0 : argv + optind;
}

/* -------------------------------------------------------------------------- */
int
main(int argc, char **argv)
{
    int rc = -1;

    srand(getpid());

    char **arg = parseOptions(argc, argv);
    if (!arg)
        goto Finally;

    unsigned long long time;

    if (!*arg)
        usage();

    const char *timeEndPtr = parseULongLong(&time, *arg);
    if (timeEndPtr && *timeEndPtr) {
        errno = EINVAL;
        timeEndPtr = 0;
    }
    if (!timeEndPtr)
        die("Unable to parse time %s", *arg);

    ++arg;

    struct CivilTime civilTime_, *civilTime = &civilTime_;

    if (!initCivilTime(civilTime, time))
        die("Unable to convert time %lu", time);

    if (*arg) {
        if (crontime(civilTime, JitterOpt, *arg))
            die("Unabled to schedule %s", *arg);
        ++arg;
    } else {

        char *linePtr = 0;
        size_t allocLen = 0;

        for (unsigned long lineNo = 1; ; ++lineNo) {

            errno = 0;
            ssize_t lineLen = getline(&linePtr, &allocLen, stdin);
            if (-1 == lineLen) {
                if (errno)
                    die("Unable to read line %lu", lineNo);
                break;
            }

            if (lineLen) {
                if (linePtr[lineLen-1] == '\n')
                    linePtr[lineLen - 1] = 0;
            }

            if (crontime(civilTime, JitterOpt, linePtr))
                die("Unabled to schedule %s at line %lu", linePtr, lineNo);
        }

        free(linePtr);
        linePtr = 0;
    }

    rc = 0;

Finally:

    return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}

/* -------------------------------------------------------------------------- */
