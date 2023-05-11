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

#include <stdlib.h>

#include "gtest/gtest.h"

/* -------------------------------------------------------------------------- */
class CivilTimeTest : public ::testing::Test
{
    void SetUp()
    {
        static char TZ[] = "TZ=Universal";

        putenv(TZ);
    }
};

/* -------------------------------------------------------------------------- */
TEST_F(CivilTimeTest, Init1970)
{
    struct CivilTime civilTime_, *civilTime = &civilTime_;

    EXPECT_EQ(civilTime, initCivilTime(civilTime, 0));
    EXPECT_EQ(1970, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(1, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(1, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Thursday, queryCivilTimeCalendar(civilTime).mWeekDay);

    EXPECT_EQ(365, queryCivilTimeCalendar(civilTime).mCalendar[0]);
    EXPECT_EQ(0, queryCivilTimeCalendar(civilTime).mCalendar[12]);

    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[0] -
            queryCivilTimeCalendar(civilTime).mCalendar[1]);
    EXPECT_EQ(28,
            queryCivilTimeCalendar(civilTime).mCalendar[1] -
            queryCivilTimeCalendar(civilTime).mCalendar[2]);

    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[2] -
            queryCivilTimeCalendar(civilTime).mCalendar[3]);
    EXPECT_EQ(30,
            queryCivilTimeCalendar(civilTime).mCalendar[3] -
            queryCivilTimeCalendar(civilTime).mCalendar[4]);
    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[4] -
            queryCivilTimeCalendar(civilTime).mCalendar[5]);
    EXPECT_EQ(30,
            queryCivilTimeCalendar(civilTime).mCalendar[5] -
            queryCivilTimeCalendar(civilTime).mCalendar[6]);
    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[6] -
            queryCivilTimeCalendar(civilTime).mCalendar[7]);

    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[7] -
            queryCivilTimeCalendar(civilTime).mCalendar[8]);
    EXPECT_EQ(30,
            queryCivilTimeCalendar(civilTime).mCalendar[8] -
            queryCivilTimeCalendar(civilTime).mCalendar[9]);
    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[9] -
            queryCivilTimeCalendar(civilTime).mCalendar[10]);
    EXPECT_EQ(30,
            queryCivilTimeCalendar(civilTime).mCalendar[10] -
            queryCivilTimeCalendar(civilTime).mCalendar[11]);
    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[11] -
            queryCivilTimeCalendar(civilTime).mCalendar[12]);

    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mMinute);
}

/* -------------------------------------------------------------------------- */
TEST_F(CivilTimeTest, Init2000)
{
    struct CivilTime civilTime_, *civilTime = &civilTime_;

    EXPECT_EQ(civilTime, initCivilTime(civilTime, 946684800));
    EXPECT_EQ(2000, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(1, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(1, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Saturday, queryCivilTimeCalendar(civilTime).mWeekDay);

    EXPECT_EQ(366, queryCivilTimeCalendar(civilTime).mCalendar[0]);
    EXPECT_EQ(0, queryCivilTimeCalendar(civilTime).mCalendar[12]);

    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[0] -
            queryCivilTimeCalendar(civilTime).mCalendar[1]);
    EXPECT_EQ(29,
            queryCivilTimeCalendar(civilTime).mCalendar[1] -
            queryCivilTimeCalendar(civilTime).mCalendar[2]);

    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[2] -
            queryCivilTimeCalendar(civilTime).mCalendar[3]);
    EXPECT_EQ(30,
            queryCivilTimeCalendar(civilTime).mCalendar[3] -
            queryCivilTimeCalendar(civilTime).mCalendar[4]);
    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[4] -
            queryCivilTimeCalendar(civilTime).mCalendar[5]);
    EXPECT_EQ(30,
            queryCivilTimeCalendar(civilTime).mCalendar[5] -
            queryCivilTimeCalendar(civilTime).mCalendar[6]);
    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[6] -
            queryCivilTimeCalendar(civilTime).mCalendar[7]);

    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[7] -
            queryCivilTimeCalendar(civilTime).mCalendar[8]);
    EXPECT_EQ(30,
            queryCivilTimeCalendar(civilTime).mCalendar[8] -
            queryCivilTimeCalendar(civilTime).mCalendar[9]);
    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[9] -
            queryCivilTimeCalendar(civilTime).mCalendar[10]);
    EXPECT_EQ(30,
            queryCivilTimeCalendar(civilTime).mCalendar[10] -
            queryCivilTimeCalendar(civilTime).mCalendar[11]);
    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[11] -
            queryCivilTimeCalendar(civilTime).mCalendar[12]);

    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mMinute);
}

/* -------------------------------------------------------------------------- */
TEST_F(CivilTimeTest, Init2100)
{
    struct CivilTime civilTime_, *civilTime = &civilTime_;

    EXPECT_EQ(civilTime, initCivilTime(civilTime, 4102444800));
    EXPECT_EQ(2100, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(1, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(1, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Friday, queryCivilTimeCalendar(civilTime).mWeekDay);

    EXPECT_EQ(365, queryCivilTimeCalendar(civilTime).mCalendar[0]);
    EXPECT_EQ(0, queryCivilTimeCalendar(civilTime).mCalendar[12]);

    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[0] -
            queryCivilTimeCalendar(civilTime).mCalendar[1]);
    EXPECT_EQ(28,
            queryCivilTimeCalendar(civilTime).mCalendar[1] -
            queryCivilTimeCalendar(civilTime).mCalendar[2]);

    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[2] -
            queryCivilTimeCalendar(civilTime).mCalendar[3]);
    EXPECT_EQ(30,
            queryCivilTimeCalendar(civilTime).mCalendar[3] -
            queryCivilTimeCalendar(civilTime).mCalendar[4]);
    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[4] -
            queryCivilTimeCalendar(civilTime).mCalendar[5]);
    EXPECT_EQ(30,
            queryCivilTimeCalendar(civilTime).mCalendar[5] -
            queryCivilTimeCalendar(civilTime).mCalendar[6]);
    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[6] -
            queryCivilTimeCalendar(civilTime).mCalendar[7]);

    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[7] -
            queryCivilTimeCalendar(civilTime).mCalendar[8]);
    EXPECT_EQ(30,
            queryCivilTimeCalendar(civilTime).mCalendar[8] -
            queryCivilTimeCalendar(civilTime).mCalendar[9]);
    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[9] -
            queryCivilTimeCalendar(civilTime).mCalendar[10]);
    EXPECT_EQ(30,
            queryCivilTimeCalendar(civilTime).mCalendar[10] -
            queryCivilTimeCalendar(civilTime).mCalendar[11]);
    EXPECT_EQ(31,
            queryCivilTimeCalendar(civilTime).mCalendar[11] -
            queryCivilTimeCalendar(civilTime).mCalendar[12]);

    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mMinute);
}

/* -------------------------------------------------------------------------- */
TEST_F(CivilTimeTest, InitNotAligned)
{
    static char TZ[] = "TZ=US/Pacific";

    putenv(TZ);

    struct CivilTime civilTime_, *civilTime = &civilTime_;

    EXPECT_FALSE(initCivilTime(civilTime, 949301938));
    EXPECT_EQ(EINVAL, errno);
}

/* -------------------------------------------------------------------------- */
TEST_F(CivilTimeTest, AdvanceTime)
{
    static char TZ[] = "TZ=US/Pacific";

    putenv(TZ);

    struct CivilTime civilTime_, *civilTime = &civilTime_;

    EXPECT_EQ(civilTime, initCivilTime(civilTime, 949301880));
    /* Sun Jan 30 22:58:00 PST 2000 */
    EXPECT_EQ(949301938 - 58, queryCivilTimeUtc(civilTime));
    EXPECT_EQ(2000, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(1, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(30, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Sunday, queryCivilTimeCalendar(civilTime).mWeekDay);
    EXPECT_EQ(22, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(58, queryCivilTimeClock(civilTime).mMinute);

    EXPECT_FALSE(advanceCivilTimeMinute(civilTime, 59));
    /* Sun Jan 30 22:59:00 PST 2000 */
    EXPECT_EQ(949301940, queryCivilTimeUtc(civilTime));
    EXPECT_EQ(2000, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(1, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(30, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Sunday, queryCivilTimeCalendar(civilTime).mWeekDay);
    EXPECT_EQ(22, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(59, queryCivilTimeClock(civilTime).mMinute);

    EXPECT_FALSE(advanceCivilTimeHour(civilTime, 23));
    /* Sun Jan 30 23:00:00 PST 2000 */
    EXPECT_EQ(949302000, queryCivilTimeUtc(civilTime));
    EXPECT_EQ(2000, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(1, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(30, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Sunday, queryCivilTimeCalendar(civilTime).mWeekDay);
    EXPECT_EQ(23, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mMinute);

    EXPECT_FALSE(advanceCivilTimeDay(civilTime, 31));
    /* Mon Jan 31 00:00:00 PST 2000 */
    EXPECT_EQ(949305600, queryCivilTimeUtc(civilTime));
    EXPECT_EQ(2000, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(1, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(31, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Monday, queryCivilTimeCalendar(civilTime).mWeekDay);
    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mMinute);

    EXPECT_FALSE(advanceCivilTimeMonth(civilTime, 2));
    /* Tue Feb  1 00:00:00 PST 2000 */
    EXPECT_EQ(949392000, queryCivilTimeUtc(civilTime));
    EXPECT_EQ(2000, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(2, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(1, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Tuesday, queryCivilTimeCalendar(civilTime).mWeekDay);
    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mMinute);

    EXPECT_EQ(-1, advanceCivilTimeYear(civilTime, 2001));
    EXPECT_EQ(EAGAIN, errno);
    /* Sun Apr  2 03:00:00 PDT 2000 */
    EXPECT_EQ(954669600, queryCivilTimeUtc(civilTime));
    EXPECT_EQ(2000, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(4, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(2, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Sunday, queryCivilTimeCalendar(civilTime).mWeekDay);
    EXPECT_EQ(3-1, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mMinute);

    EXPECT_EQ(-1, advanceCivilTimeYear(civilTime, 2001));
    EXPECT_EQ(EAGAIN, errno);
    /* Sun Apr  2 03:00:00 PDT 2000 */
    EXPECT_EQ(954669600, queryCivilTimeUtc(civilTime));
    EXPECT_EQ(2000, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(4, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(2, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Sunday, queryCivilTimeCalendar(civilTime).mWeekDay);
    EXPECT_EQ(3-0, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mMinute);

    EXPECT_EQ(-1, advanceCivilTimeYear(civilTime, 2001));
    EXPECT_EQ(EAGAIN, errno);
    /* Sun Oct 29 01:00:00 PST 2000 */
    EXPECT_EQ(972810000, queryCivilTimeUtc(civilTime));
    EXPECT_EQ(2000, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(10, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(29, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Sunday, queryCivilTimeCalendar(civilTime).mWeekDay);
    EXPECT_EQ(0 - 1 - 1, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mMinute);

    EXPECT_EQ(-1, advanceCivilTimeYear(civilTime, 2001));
    EXPECT_EQ(EAGAIN, errno);
    /* Sun Oct 29 02:00:00 PST 2000 */
    EXPECT_EQ(972813600, queryCivilTimeUtc(civilTime));
    EXPECT_EQ(2000, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(10, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(29, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Sunday, queryCivilTimeCalendar(civilTime).mWeekDay);
    EXPECT_EQ(2, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mMinute);

    EXPECT_FALSE(advanceCivilTimeYear(civilTime, 2001));
    /* Mon Jan  1 00:00:00 PST 2001 */
    EXPECT_EQ(978336000, queryCivilTimeUtc(civilTime));
    EXPECT_EQ(2001, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(1, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(1, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Monday, queryCivilTimeCalendar(civilTime).mWeekDay);
    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mMinute);
}

/* -------------------------------------------------------------------------- */
TEST_F(CivilTimeTest, AdvanceTimeSpringDST)
{
    static char TZ[] = "TZ=US/Pacific";

    putenv(TZ);

    struct CivilTime civilTime_, *civilTime = &civilTime_;

    EXPECT_EQ(civilTime, initCivilTime(civilTime, 954669600));
    /* Sun Apr  2 03:00:00 PDT 2000 */
    EXPECT_EQ(954669600, queryCivilTimeUtc(civilTime));
    EXPECT_EQ(2000, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(4, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(2, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Sunday, queryCivilTimeCalendar(civilTime).mWeekDay);
    EXPECT_EQ(3-1, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mMinute);

    EXPECT_FALSE(advanceCivilTimeMinute(civilTime, 1));
    /* Sun Apr  2 03:01:00 PDT 2000 */
    EXPECT_EQ(954669660, queryCivilTimeUtc(civilTime));
    EXPECT_EQ(2000, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(4, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(2, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Sunday, queryCivilTimeCalendar(civilTime).mWeekDay);
    EXPECT_EQ(3-1, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(1, queryCivilTimeClock(civilTime).mMinute);
}

/* -------------------------------------------------------------------------- */
TEST_F(CivilTimeTest, AdvanceTimeFallDST)
{
    static char TZ[] = "TZ=US/Pacific";

    putenv(TZ);

    struct CivilTime civilTime_, *civilTime = &civilTime_;

    EXPECT_EQ(civilTime, initCivilTime(civilTime, 972810000));
    /* Sun Oct 29 01:00:00 PST 2000 */
    EXPECT_EQ(972810000, queryCivilTimeUtc(civilTime));
    EXPECT_EQ(2000, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(10, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(29, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Sunday, queryCivilTimeCalendar(civilTime).mWeekDay);
    EXPECT_EQ(0 - 1 - 1, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(0, queryCivilTimeClock(civilTime).mMinute);

    EXPECT_FALSE(advanceCivilTimeMinute(civilTime, 1));
    /* Sun Oct 29 01:00:00 PST 2000 */
    EXPECT_EQ(972810060, queryCivilTimeUtc(civilTime));
    EXPECT_EQ(2000, queryCivilTimeCalendar(civilTime).mYear);
    EXPECT_EQ(10, queryCivilTimeCalendar(civilTime).mMonth);
    EXPECT_EQ(29, queryCivilTimeCalendar(civilTime).mDay);
    EXPECT_EQ(Sunday, queryCivilTimeCalendar(civilTime).mWeekDay);
    EXPECT_EQ(0 - 1 - 1, queryCivilTimeClock(civilTime).mHour);
    EXPECT_EQ(1, queryCivilTimeClock(civilTime).mMinute);
}

/* -------------------------------------------------------------------------- */

#include "_test_.h"
