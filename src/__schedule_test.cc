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

#include "schedule.h"

#include "civiltime.h"

#include "gtest/gtest.h"

#include <errno.h>
#include <stdlib.h>

#include <valgrind/valgrind.h>

/* -------------------------------------------------------------------------- */
class ScheduleTest : public ::testing::Test
{
    void SetUp()
    {
        static char TZ[] = "TZ=US/Pacific";

        putenv(TZ);
    }

protected:

    time_t testSchedule_(
        struct Schedule *aSchedule,
        time_t aTime,
        time_t aJitterPeriod = 0,
        int *aJitter = 0) {

        int rc = -1;

        time_t scheduled = -1;

        struct CivilTime civilTime_, *civilTime = &civilTime_;

        if (!initCivilTime(civilTime, aTime))
            goto Finally;

        scheduled = querySchedule(aSchedule, civilTime, aJitterPeriod, aJitter);
        if (-1 == scheduled)
            goto Finally;

        rc = 0;

    Finally:

        return rc ? -1 : scheduled;
    }
};

/* -------------------------------------------------------------------------- */
TEST_F(ScheduleTest, Fields)
{
    struct Schedule schedule_, *schedule = &schedule_;

    EXPECT_FALSE(initSchedule(schedule, ""));
    EXPECT_EQ(EINVAL, errno);

    EXPECT_FALSE(initSchedule(schedule, " * * * * *"));
    EXPECT_EQ(EINVAL, errno);

    EXPECT_FALSE(initSchedule(schedule, "* * * * * "));
    EXPECT_EQ(EINVAL, errno);

    EXPECT_TRUE(initSchedule(schedule, "* * * * *"));
}

/* -------------------------------------------------------------------------- */
TEST_F(ScheduleTest, EveryMinute)
{
    struct Schedule schedule_, *schedule = &schedule_;

    EXPECT_EQ(schedule, initSchedule(schedule, "* * * * *"));

    /* Sat Jan  1 00:00:00 PST 2000 */
    EXPECT_EQ(946713600, testSchedule_(schedule, 946713600));

    /* Sun Apr  2 03:00:00 PDT 2000 */
    EXPECT_EQ(954669600, testSchedule_(schedule, 954669600));

    /* Sun Oct 29 01:00:00 PST 2000 */
    EXPECT_EQ(972810000, testSchedule_(schedule, 972810000));

    /* Sun Dec 31 23:59:00 PST 2000 */
    EXPECT_EQ(978335940, testSchedule_(schedule, 978335940));
}

/* -------------------------------------------------------------------------- */
TEST_F(ScheduleTest, Range)
{
    struct Schedule schedule_, *schedule = &schedule_;

    EXPECT_EQ(schedule, initSchedule(schedule, "0-58 1-22 2-28 2-11 *"));

    /* Sat Jan  1 00:00:00 PST 2000 */
    /* Wed Feb  2 01:00:00 PST 2000 */
    EXPECT_EQ(949482000, testSchedule_(schedule, 946713600));
}

/* -------------------------------------------------------------------------- */
TEST_F(ScheduleTest, SpillOver)
{
    struct Schedule schedule_, *schedule = &schedule_;

    EXPECT_EQ(schedule, initSchedule(schedule, "1-58 1-22 2-28 2-11 *"));

    /* Sat Jan  1 00:00:00 PST 2000 */
    /* Wed Feb  2 01:01:00 PST 2000 */
    EXPECT_EQ(949482060, testSchedule_(schedule, 946713600));

    /* Tue Nov 28 22:58:00 PST 2000 */
    EXPECT_EQ(975481080, testSchedule_(schedule, 975481080));

    /* Tue Nov 28 22:59:00 PST 2000 */
    /* Fri Feb  2 01:01:00 PST 2001 */
    EXPECT_EQ(981104460, testSchedule_(schedule, 975481140));
}

/* -------------------------------------------------------------------------- */
TEST_F(ScheduleTest, SpringDST_0200)
{
    struct Schedule schedule_, *schedule = &schedule_;

    /* Verify actions that are only scheduled at 0200 */

    EXPECT_EQ(schedule, initSchedule(schedule, "0,30 1,2 1,2 4,5 *"));

    struct Schedule hourly_, *hourly = &hourly_;

    EXPECT_EQ(hourly, initSchedule(hourly, "0 * * * *"));

    /* Sat Jan  1 22:59:00 PST 2000 */
    /* Sat Apr  1 01:00:00 PST 2000 */
    EXPECT_EQ(954579600, testSchedule_(schedule, 946796280+60));

    /* Sun Apr  1 01:01:00 PST 2000 */
    /* Sat Apr  1 01:30:00 PST 2000 */
    EXPECT_EQ(954581400, testSchedule_(schedule, 954579600+60));

    /* Sat Apr  1 01:31:00 PST 2000 */
    /* Sat Apr  1 02:00:00 PST 2000 */
    EXPECT_EQ(954583200, testSchedule_(schedule, 954581400+60));

    /* Note that 02:00 is selected by the schedule, but 03:00 is not
     * selected. The following test verifies that 03:00 is skipped.
     */

    /* Sat Apr  1 02:01:00 PST 2000 */
    /* Sat Apr  1 02:30:00 PST 2000 */
    EXPECT_EQ(954585000, testSchedule_(schedule, 954583200+60));

    /* Sat Apr  1 02:31:00 PST 2000 */
    /* Sun Apr  2 01:00:00 PST 2000 */
    EXPECT_EQ(954666000, testSchedule_(schedule, 954585000+60));

    /* The next two tests verify that 02:00 is scheduled even though
     * daylight savings causes the local clock to skip immediately
     * to 03:00, and then that an hourly schedule selects 03:00
     * following 01:00.
     */

    /* Sun Apr  2 01:01:00 PST 2000 */
    /* Sun Apr  2 01:30:00 PST 2000 */
    EXPECT_EQ(954667800, testSchedule_(schedule, 954666000+60));

    /* Sun Apr  2 01:31:00 PST 2000 */
    /* Sun Apr  2 02:00:00 PDT 2000 Artificial */
    /* Sun Apr  2 03:00:00 PDT 2000 */
    EXPECT_EQ(954669600, testSchedule_(schedule, 954667800+60));

    /* Sun Apr  2 01:01:00 PST 2000 */
    /* Sun Apr  2 03:00:00 PDT 2000 */
    EXPECT_EQ(954669600, testSchedule_(hourly, 954666000+60));

    /* Sun Apr  2 03:01:00 PDT 2000 */
    /* Sun Apr  2 03:30:00 PDT 2000 */
    EXPECT_EQ(954671400, testSchedule_(schedule, 954669600+60));

    /* Sun Apr  2 03:31:00 PDT 2000 */
    /* Mon May  1 01:00:00 PDT 2000 */
    EXPECT_EQ(957168000, testSchedule_(schedule, 954671400+60));

    /* This is a final check that confirms that 03:00 is not present
     * in the test scheduled.
     */

    /* Mon Apr  3 03:01:00 PDT 2000 */
    /* Mon May  1 01:00:00 PDT 2000 */
    EXPECT_EQ(957168000, testSchedule_(schedule, 954756000+60));
}

/* -------------------------------------------------------------------------- */
TEST_F(ScheduleTest, SpringDST_0300)
{
    struct Schedule schedule_, *schedule = &schedule_;

    /* Verify actions that are only scheduled at 0300 */

    EXPECT_EQ(schedule, initSchedule(schedule, "0,30 1,3 1,2 4,5 *"));

    struct Schedule hourly_, *hourly = &hourly_;

    EXPECT_EQ(hourly, initSchedule(hourly, "0 * * * *"));

    /* Sat Jan  1 22:59:00 PST 2000 */
    /* Sat Apr  1 01:00:00 PST 2000 */
    EXPECT_EQ(954579600, testSchedule_(schedule, 946796280+60));

    /* Sun Apr  1 01:01:00 PST 2000 */
    /* Sat Apr  1 01:30:00 PST 2000 */
    EXPECT_EQ(954581400, testSchedule_(schedule, 954579600+60));

    /* Sat Apr  1 01:31:00 PST 2000 */
    /* Sat Apr  1 03:00:00 PST 2000 */
    EXPECT_EQ(954586800, testSchedule_(schedule, 954581400+60));

    /* Note that 03:00 is selected by the schedule, but 02:00 is not
     * selected. The following test verifies that 02:00 is skipped.
     */

    /* Sat Apr  1 03:01:00 PST 2000 */
    /* Sat Apr  1 03:30:00 PST 2000 */
    EXPECT_EQ(954588600, testSchedule_(schedule, 954586800+60));

    /* Sat Apr  1 03:31:00 PST 2000 */
    /* Sun Apr  2 01:00:00 PST 2000 */
    EXPECT_EQ(954666000, testSchedule_(schedule, 954588600+60));

    /* The next two tests verify that 03:00 is scheduled noting that
     * daylight savings causes the local clock to skip immediately
     * to 03:00, and then that an hourly schedule selects 03:00
     * following 01:00.
     */

    /* Sun Apr  2 01:01:00 PST 2000 */
    /* Sun Apr  2 01:30:00 PST 2000 */
    EXPECT_EQ(954667800, testSchedule_(schedule, 954666000+60));

    /* Sun Apr  2 01:31:00 PST 2000 */
    /* Sun Apr  2 02:00:00 PDT 2000 Artificial */
    /* Sun Apr  2 03:00:00 PDT 2000 */
    EXPECT_EQ(954669600, testSchedule_(schedule, 954667800+60));

    /* Sun Apr  2 01:01:00 PST 2000 */
    /* Sun Apr  2 03:00:00 PDT 2000 */
    EXPECT_EQ(954669600, testSchedule_(hourly, 954666000+60));

    /* Sun Apr  2 03:01:00 PDT 2000 */
    /* Sun Apr  2 03:30:00 PDT 2000 */
    EXPECT_EQ(954671400, testSchedule_(schedule, 954669600+60));

    /* Sun Apr  2 03:31:00 PDT 2000 */
    /* Mon May  1 01:00:00 PDT 2000 */
    EXPECT_EQ(957168000, testSchedule_(schedule, 954671400+60));

    /* This is a final check that confirms that 02:00 is not present
     * in the test scheduled.
     */

    /* Mon Apr  3 03:01:00 PDT 2000 */
    /* Mon May  1 01:00:00 PDT 2000 */
    EXPECT_EQ(957168000, testSchedule_(schedule, 954756000+60));

    /* Mon May  1 01:01:00 PDT 2000 */
    /* Mon May  1 01:30:00 PDT 2000 */
    EXPECT_EQ(957169800, testSchedule_(schedule, 957168000+60));

    /* Mon May  1 01:31:00 PDT 2000 */
    /* Mon May  1 03:00:00 PDT 2000 */
    EXPECT_EQ(957175200, testSchedule_(schedule, 957169800+60));
}

/* -------------------------------------------------------------------------- */
TEST_F(ScheduleTest, SpringDST_0200_0300)
{
    struct Schedule schedule_, *schedule = &schedule_;

    /* Verify actions that are scheduled at 0200 and 0300 */

    EXPECT_EQ(schedule, initSchedule(schedule, "0,30 1,2,3 1,2 4,5 *"));

    struct Schedule hourly_, *hourly = &hourly_;

    EXPECT_EQ(hourly, initSchedule(hourly, "0 * * * *"));

    /* Sat Jan  1 22:59:00 PST 2000 */
    /* Sat Apr  1 01:00:00 PST 2000 */
    EXPECT_EQ(954579600, testSchedule_(schedule, 946796280+60));

    /* Sun Apr  1 01:01:00 PST 2000 */
    /* Sat Apr  1 01:30:00 PST 2000 */
    EXPECT_EQ(954581400, testSchedule_(schedule, 954579600+60));

    /* Sat Apr  1 01:31:00 PST 2000 */
    /* Sat Apr  1 02:00:00 PST 2000 */
    EXPECT_EQ(954583200, testSchedule_(schedule, 954581400+60));

    /* Note that both 02:00 and 03:00 are configured by the schedule,
     * but 03:00 is not selected. The following test verifies that
     * 03:00 is skipped.
     */

    /* Sat Apr  1 02:01:00 PST 2000 */
    /* Sat Apr  1 02:30:00 PST 2000 */
    EXPECT_EQ(954585000, testSchedule_(schedule, 954583200+60));

    /* Sat Apr  1 02:31:00 PST 2000 */
    /* Sat Apr  1 03:00:00 PST 2000 */
    EXPECT_EQ(954586800, testSchedule_(schedule, 954585000+60));

    /* Sat Apr  1 03:01:00 PST 2000 */
    /* Sat Apr  1 03:30:00 PST 2000 */
    EXPECT_EQ(954588600, testSchedule_(schedule, 954586800+60));

    /* Sat Apr  1 03:31:00 PST 2000 */
    /* Sun Apr  2 01:00:00 PST 2000 */
    EXPECT_EQ(954666000, testSchedule_(schedule, 954588600+60));

    /* The next two tests verify that either 02:00 or 03:00 are scheduled
     * noting that daylight savings causes the local clock to skip immediately
     * to 03:00, and then that an hourly schedule selects 03:00
     * following 01:00.
     */

    /* Sun Apr  2 01:01:00 PST 2000 */
    /* Sun Apr  2 01:30:00 PST 2000 */
    EXPECT_EQ(954667800, testSchedule_(schedule, 954666000+60));

    /* Sun Apr  2 01:31:00 PST 2000 */
    /* Sun Apr  2 02:00:00 PDT 2000 Artificial */
    /* Sun Apr  2 03:00:00 PDT 2000 */
    EXPECT_EQ(954669600, testSchedule_(schedule, 954667800+60));

    /* Sun Apr  2 01:01:00 PST 2000 */
    /* Sun Apr  2 03:00:00 PDT 2000 */
    EXPECT_EQ(954669600, testSchedule_(hourly, 954666000+60));

    /* Sun Apr  2 03:01:00 PDT 2000 */
    /* Sun Apr  2 03:30:00 PDT 2000 */
    EXPECT_EQ(954671400, testSchedule_(schedule, 954669600+60));

    /* Sun Apr  2 03:31:00 PDT 2000 */
    /* Mon May  1 01:00:00 PDT 2000 */
    EXPECT_EQ(957168000, testSchedule_(schedule, 954671400+60));

    /* This is a final check that confirms that 03:00 is not present
     * in the test scheduled.
     */

    /* Mon Apr  3 03:01:00 PDT 2000 */
    /* Mon May  1 01:00:00 PDT 2000 */
    EXPECT_EQ(957168000, testSchedule_(schedule, 954756000+60));
}

/* -------------------------------------------------------------------------- */
TEST_F(ScheduleTest, FallDST)
{
    struct Schedule schedule_, *schedule = &schedule_;

    EXPECT_EQ(schedule, initSchedule(schedule, "0,30 1,2,3 29 10 *"));

    struct Schedule hourly_, *hourly = &hourly_;

    EXPECT_EQ(hourly, initSchedule(hourly, "0 * * * *"));

    /* Sat Jul  1 22:59:00 PDT 2000 */
    /* Sun Oct 29 01:00:00 PDT 2000 */
    EXPECT_EQ(972806400, testSchedule_(schedule, 962517480+60));

    /* Sun Oct 29 01:01:00 PDT 2000 */
    /* Sun Oct 29 01:00:00 PST 2000 Not Skipped */
    EXPECT_EQ(972810000, testSchedule_(hourly, 972806400+60));

    /* Sun Oct 29 01:01:00 PST 2000 Not Skipped */
    /* Sun Oct 29 02:00:00 PST 2000 */
    EXPECT_EQ(972813600, testSchedule_(hourly, 972810000+60));

    /* Sun Oct 29 02:01:00 PST 2000 */
    /* Sun Oct 29 03:00:00 PST 2000 */
    EXPECT_EQ(972817200, testSchedule_(hourly, 972813600+60));

    /* Sun Oct 29 01:01:00 PDT 2000 */
    /* Sun Oct 29 01:30:00 PDT 2000 */
    EXPECT_EQ(972808200, testSchedule_(schedule, 972806400+60));

    /* Sun Oct 29 01:31:00 PDT 2000 */
    /* Sun Oct 29 01:00:00 PST 2000 Skipped */
    /* Sun Oct 29 01:30:00 PST 2000 Skipped */
    /* Sun Oct 29 02:00:00 PST 2000 */
    EXPECT_EQ(972813600, testSchedule_(schedule, 972808200+60));

    /* Sun Oct 29 02:01:00 PST 2000 */
    /* Sun Oct 29 02:30:00 PST 2000 */
    EXPECT_EQ(972815400, testSchedule_(schedule, 972813600+60));

    /* Sun Oct 29 02:31:00 PST 2000 */
    /* Sun Oct 29 03:00:00 PST 2000 */
    EXPECT_EQ(972817200, testSchedule_(schedule, 972815400+60));
}

/* -------------------------------------------------------------------------- */
TEST_F(ScheduleTest, OneSidedJitter_1_Hour)
{
    struct Schedule schedule_, *schedule = &schedule_;

    EXPECT_EQ(schedule, initSchedule(schedule, "0 * * * *"));

    time_t now, deadline, scheduled;

    /* Sat Jan  1 00:00:00 PST 2000 */
    now = 946713600;

    time_t jitterPeriod = 5 * 60; /* 5 minute limit */

    int sum = 0;
    int sumsq = 0;
    int trials = RUNNING_ON_VALGRIND ? 1 : 1000;

    for (int ix = 0; ix < trials; ++ix) {
        int jitter;
        deadline = testSchedule_(schedule, now, jitterPeriod, &jitter);
        scheduled = testSchedule_(schedule, now, 0);
        EXPECT_LE(now, deadline);
        EXPECT_EQ(deadline - jitter, scheduled);

        int delay = deadline - now;

        sum += delay;
        sumsq += delay * delay;
    }

    if (!RUNNING_ON_VALGRIND) {
        int observedPeriod = 5 * 60;

        int mean = sum / trials;
        int var = sumsq / trials - mean * mean;

        int jitterPeriodStdDev = observedPeriod / 4.24264; /* sqrt(18) */

        EXPECT_LE((jitterPeriodStdDev-2) * (jitterPeriodStdDev-2), var);
        EXPECT_GE((jitterPeriodStdDev+2) * (jitterPeriodStdDev+2), var);

        EXPECT_LE(observedPeriod / 3 - 3, mean);
        EXPECT_GE(observedPeriod / 3 + 3, mean);
    }
}

/* -------------------------------------------------------------------------- */
TEST_F(ScheduleTest, OneSidedJitter_3_Minutes)
{
    struct Schedule schedule_, *schedule = &schedule_;

    EXPECT_EQ(schedule, initSchedule(schedule, "*/3 * * * *"));

    time_t now, deadline, scheduled;

    /* Sat Jan  1 00:00:00 PST 2000 */
    now = 946713600;

    time_t jitterPeriod = 3 * 60;

    int sum = 0;
    int sumsq = 0;
    int trials = RUNNING_ON_VALGRIND ? 1 : 1000;

    for (int ix = 0; ix < trials; ++ix) {
        int jitter;
        deadline = testSchedule_(schedule, now, jitterPeriod, &jitter);
        scheduled = testSchedule_(schedule, now, 0);
        EXPECT_LE(now, deadline);
        EXPECT_EQ(deadline - jitter, scheduled);

        int delay = deadline - now;

        sum += delay;
        sumsq += delay * delay;
    }

    if (!RUNNING_ON_VALGRIND) {
        int observedPeriod = 3 * 60 / 2;

        sum = trials * observedPeriod / 3;

        int mean = sum / trials;
        int var = sumsq / trials - mean * mean;

        int jitterPeriodStdDev = observedPeriod / 4.24264; /* sqrt(18) */

        EXPECT_LE((jitterPeriodStdDev-3) * (jitterPeriodStdDev-3), var);
        EXPECT_GE((jitterPeriodStdDev+3) * (jitterPeriodStdDev+3), var);

        EXPECT_LE(observedPeriod / 3 - 2, mean);
        EXPECT_GE(observedPeriod / 3 + 2, mean);
    }
}

/* -------------------------------------------------------------------------- */
TEST_F(ScheduleTest, TwoSidedJitter_1_Minute)
{
    struct Schedule schedule_, *schedule = &schedule_;

    EXPECT_EQ(schedule, initSchedule(schedule, "*/3 * * * *"));

    time_t now, deadline, scheduled;

    /* Sat Jan  1 00:02:00 PST 2000 */
    now = 946713600 + 2 * 60;

    time_t jitterPeriod = 3 * 60;

    int sum = 0;
    int sumsq = 0;
    int trials = RUNNING_ON_VALGRIND ? 1 : 1000;

    for (int ix = 0; ix < trials; ++ix) {
        int jitter;
        deadline = testSchedule_(schedule, now, jitterPeriod, &jitter);
        scheduled = testSchedule_(schedule, now, 0);
        EXPECT_LE(now, deadline);
        EXPECT_EQ(deadline - jitter, scheduled);

        int delay = deadline - now;

        sum += delay;
        sumsq += delay * delay;
    }

    if (!RUNNING_ON_VALGRIND) {
        int observedPeriod = 1 * 60;

        int mean = sum / trials;
        int var = sumsq / trials - mean * mean;

        int jitterPeriodStdDev = observedPeriod / 2.44949; /* sqrt(6) */

        EXPECT_LE((jitterPeriodStdDev-3) * (jitterPeriodStdDev-3), var);
        EXPECT_GE((jitterPeriodStdDev+3) * (jitterPeriodStdDev+3), var);

        EXPECT_LE(observedPeriod - 4, mean);
        EXPECT_GE(observedPeriod + 4, mean);
    }
}

/* -------------------------------------------------------------------------- */

#include "_test_.h"
