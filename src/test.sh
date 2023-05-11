#!/usr/bin/env bash
# -*- sh-basic-offset:4; indent-tabs-mode:nil -*- vi: set sw=4 et:

[ -z "${0##/*}" ] || exec "$PWD/$0" "$@"

set -eux

say()
{
    printf '%s\n' "$*"
}

crontime()
{
    "${0%/*}/crontime" "$@"
}

backtrace()
{
    local FRAMES=$((${#FUNCNAME[@]} - 1))
    local FRAME=1

    while [ $((++FRAME)) -lt $FRAMES ] ; do
        say "${FUNCNAME[$FRAME]}():${BASH_LINENO[$FRAME-1]}" >&2
    done
}

check()
{
    "$@"
}

test_every_minute_and_one_second()
{
    # Sat Jan  1 00:00:00 PST 2000
    # Sat Jan  1 00:00:00 PST 2000
    check [ '946713600 0' = "$(crontime -j 0 946713600 '* * * * *')" ]

    # Sat Jan  1 00:00:01 PST 2000
    # Sat Jan  1 00:01:00 PST 2000
    check [ '946713660 0' = "$(crontime -j 0 946713601 '* * * * *')" ]
}

test_every_minute()
{
    # Sat Jan  1 00:00:00 PST 2000
    check [ '946713600 0' = "$(crontime -j 0 946713600 '* * * * *')" ]

    # Sun Apr  2 03:00:00 PDT 2000
    check [ '954669600 0' = "$(crontime -j 0 954669600 '* * * * *')" ]

    # Sun Oct 29 01:00:00 PST 2000
    check [ '972810000 0' = "$(crontime -j 0 972810000 '* * * * *')" ]

    # Sun Dec 31 23:59:00 PST 2000
    check [ '978335940 0' = "$(crontime -j 0 978335940 '* * * * *')" ]
}

test_spill_over()
{
    # Sat Jan  1 00:00:00 PST 2000
    # Wed Feb  2 01:01:00 PST 2000
    check [ '949482060 0' = "$(crontime -j 0 946713600 '1-58 1-22 2-28 2-11 *')" ]

    # Tue Nov 28 22:58:00 PST 2000
    check [ '975481080 0' = "$(crontime -j 0 975481080 '1-58 1-22 2-28 2-11 *')" ]

    # Tue Nov 28 22:59:00 PST 2000
    # Fri Feb  2 01:01:00 PST 2001
    check [ '981104460 0' = "$(crontime -j 0 975481140 '1-58 1-22 2-28 2-11 *')" ]
}

test_spring_dst()
{
    local SCHEDULE='0,30 1,2 1,2 4,5 *'

    #  Sat Jan  1 22:59:00 PST 2000
    #  Sat Apr  1 01:00:00 PST 2000
    check [ '954579600 0' = "$(crontime -j 0 $((946796280+60)) "$SCHEDULE")" ]

    #  Sun Apr  1 01:01:00 PST 2000
    #  Sat Apr  1 01:30:00 PST 2000
    check [ '954581400 0' = "$(crontime -j 0 $((954579600+60)) "$SCHEDULE")" ]

    #  Sat Apr  1 01:31:00 PST 2000
    #  Sat Apr  1 02:00:00 PST 2000
    check [ '954583200 0' = "$(crontime -j 0 $((954581400+60)) "$SCHEDULE")" ]

    # Note that 02:00 is selected by the schedule, but 03:00 is not
    # selected. The following test verifies that 03:00 is skipped.

    #  Sat Apr  1 02:01:00 PST 2000
    #  Sat Apr  1 02:30:00 PST 2000
    check [ '954585000 0' = "$(crontime -j 0 $((954583200+60)) "$SCHEDULE")" ]

    #  Sat Apr  1 02:31:00 PST 2000
    #  Sun Apr  2 01:00:00 PST 2000
    check [ '954666000 0' = "$(crontime -j 0 $((954585000+60)) "$SCHEDULE")" ]

    # The next two tests verify that 02:00 is selected even though
    # daylight savings causes the local clock to skip immediately
    # to 03:00, and then that an hourly schedule selects 03:00
    # following 01:00.

    #  Sun Apr  2 01:01:00 PST 2000
    #  Sun Apr  2 01:30:00 PST 2000
    check [ '954667800 0' = "$(crontime -j 0 $((954666000+60)) "$SCHEDULE")" ]

    #  Sun Apr  2 01:31:00 PST 2000
    #  Sun Apr  2 02:00:00 PDT 2000 Artificial
    #  Sun Apr  2 03:00:00 PDT 2000
    check [ '954669600 0' = "$(crontime -j 0 $((954667800+60)) "$SCHEDULE")" ]

    #  Sun Apr  2 01:01:00 PST 2000
    #  Sun Apr  2 03:00:00 PDT 2000
    check [ '954669600 0' = "$(crontime -j 0 $((954666000+60)) '0 * * * *')" ]

    #  Sun Apr  2 03:01:00 PDT 2000
    #  Sun Apr  2 03:30:00 PDT 2000
    check [ '954671400 0' = "$(crontime -j 0 $((954669600+60)) "$SCHEDULE")" ]

    # This is a final check that confirms that 03:00 is not present
    # in the test scheduled.

    #  Mon Apr  3 03:01:00 PDT 2000
    #  Mon May  1 01:00:00 PDT 2000
    check [ '957168000 0' = "$(crontime -j 0 $((954756000+60)) "$SCHEDULE")" ]
}

test_fall_dst()
{
    local SCHEDULE='0,30 1,2,3 29 10 *'

    # Sat Jul  1 22:59:00 PDT 2000
    # Sun Oct 29 01:00:00 PDT 2000
    check [ '972806400 0' = "$(crontime -j 0 $((962517480+60)) "$SCHEDULE")" ]

    # Sun Oct 29 01:01:00 PDT 2000
    # Sun Oct 29 01:00:00 PST 2000 Not Skipped
    check [ '972810000 0' = "$(crontime -j 0 $((972806400+60)) '0 * * * *')" ]

    # Sun Oct 29 01:01:00 PST 2000 Not Skipped
    # Sun Oct 29 02:00:00 PST 2000
    check [ '972813600 0' = "$(crontime -j 0 $((972810000+60)) '0 * * * *')" ]

    # Sun Oct 29 02:01:00 PST 2000
    # Sun Oct 29 03:00:00 PST 2000
    check [ '972817200 0' = "$(crontime -j 0 $((972813600+60)) '0 * * * *')" ]

    # Sun Oct 29 01:01:00 PDT 2000
    # Sun Oct 29 01:30:00 PDT 2000
    check [ '972808200 0' = "$(crontime -j 0 $((972806400+60)) "$SCHEDULE")" ]

    # Sun Oct 29 01:30:00 PDT 2000
    # Sun Oct 29 01:00:00 PST 2000 Skipped
    # Sun Oct 29 01:30:00 PST 2000 Skipped
    # Sun Oct 29 02:00:00 PST 2000
    check [ '972813600 0' = "$(crontime -j 0 $((972808200+60)) "$SCHEDULE")" ]

    # Sun Oct 29 02:01:00 PST 2000
    # Sun Oct 29 02:30:00 PST 2000
    check [ '972815400 0' = "$(crontime -j 0 $((972813600+60)) "$SCHEDULE")" ]

    # Sun Oct 29 02:31:00 PST 2000
    # Sun Oct 29 03:00:00 PST 2000
    check [ '972817200 0' = "$(crontime -j 0 $((972815400+60)) "$SCHEDULE")" ]
}

test_stdin()
{
    # Sat Jan  1 00:00:00 PST 2000
    # Wed Feb  2 01:01:00 PST 2000
    check [ '949482060 0' = "$(
        say '1-58 1-22 2-28 2-11 *' | crontime -j 0 946713600)" ]

    # Tue Nov 28 22:58:00 PST 2000
    check [ '975481080 0' = "$(
        say '1-58 1-22 2-28 2-11 *' | crontime -j 0 975481080)" ]

    # Tue Nov 28 22:59:00 PST 2000
    # Fri Feb  2 01:01:00 PST 2001
    check [ "$(
        say 981104460 0
        say 981104400 0
    )" = "$(
        {
            say '1-58 1-22 2-28 2-11 *'
            say '0-58 1-22 2-28 2-11 *'
        } | crontime -j 0 975481140)" ]
}

test_jitter()
{
    local SCHEDULE='* * * * *'

    # Sat Jul  1 22:58:00 PDT 2000

    local HISTOGRAM=$(
        for _ in {1..100} ; do
            crontime --jitter 2 962517480 "$SCHEDULE"
        done | cut -d' ' -f1 | sort | uniq -c
    )

    local LOWER=$(say "$HISTOGRAM" | awk '$2 == 962517480')
    local UPPER=$(say "$HISTOGRAM" | awk '$2 == 962517481')

    check [ 68 -le ${LOWER% *} ]
    check [ 82 -ge ${LOWER% *} ]

    check [ 18 -le ${UPPER% *} ]
    check [ 32 -ge ${UPPER% *} ]
}

main()
{
    export TZ='US/Pacific'

    trap '[ $? -eq 0 ] || { set +x; backtrace; }' EXIT

    test_every_minute_and_one_second
    test_every_minute
    test_spill_over
    test_spring_dst
    test_fall_dst
    test_jitter

    test_stdin
}

main "$@"
