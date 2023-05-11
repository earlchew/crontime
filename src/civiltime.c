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

#include "macros.h"

#include "tz/localtime.h"

#include <errno.h>

/* -------------------------------------------------------------------------- */
static const int LeapYear[] = {
    366 - 0,
    366 - (31),
    366 - (31+29),
    366 - (31+29+31),
    366 - (31+29+31+30),
    366 - (31+29+31+30+31),
    366 - (31+29+31+30+31+30),
    366 - (31+29+31+30+31+30+31),
    366 - (31+29+31+30+31+30+31+31),
    366 - (31+29+31+30+31+30+31+31+30),
    366 - (31+29+31+30+31+30+31+31+30+31),
    366 - (31+29+31+30+31+30+31+31+30+31+30),
    366 - (31+29+31+30+31+30+31+31+30+31+30+31),
};

static const int CommonYear[] = {
    365,
    365 - (31),
    365 - (31+28),
    365 - (31+28+31),
    365 - (31+28+31+30),
    365 - (31+28+31+30+31),
    365 - (31+28+31+30+31+30),
    365 - (31+28+31+30+31+30+31),
    365 - (31+28+31+30+31+30+31+31),
    365 - (31+28+31+30+31+30+31+31+30),
    365 - (31+28+31+30+31+30+31+31+30+31),
    365 - (31+28+31+30+31+30+31+31+30+31+30),
    365 - (31+28+31+30+31+30+31+31+30+31+30+31),
};

static const int *
calendar_(const struct tm *aTm)
{
    int year = aTm->tm_year + 1900;
    int leapYear = year % 100 ? year % 4 == 0 : year % 400 == 0;

    return leapYear ? LeapYear : CommonYear;
}

/* -------------------------------------------------------------------------- */
#define civilTimeInterval_(self) ({             \
     __auto_type self_ = (self);                \
                                                \
     &self_->mIntervals[self_->mInterval];      \
})

/* -------------------------------------------------------------------------- */
static int
applyCivilTimeDstChange_(struct CivilTime *self, long aDstChange)
{
    int rc = -1;

    struct Interval *interval = civilTimeInterval_(self);

    /* If the transition caused a daylight savings change, consider
     * the direction of the change. A negative change causes repetition,
     * and a positive change causes skipping.
     */

    if (aDstChange) {

        time_t transitionTime = timegm(&interval->mTm);
        if (-1 == transitionTime)
            goto Finally;

        transitionTime -= aDstChange;

        struct tm *transitionTm = gmtime(&transitionTime);
        if (!transitionTm)
            goto Finally;

        /* No matter what the direction of the change, a daylight
         * savings interval is created to model the transition period.
         *
         * In particular, the UTC offset at the end of the transition
         * interval must equal the UTC offset at the beginning of
         * the timezone rule. This is required so that the next
         * transition will likely be smooth and not trigger insertion
         * of another transition period.
         */

        if (aDstChange < 0) {

            if (interval->mTime + aDstChange < interval->mDst.mBegin.mTime) {

                ++self->mInterval;
                struct Interval *shadow = civilTimeInterval_(self);

                *shadow = *interval;

                /* Repetition caused by a negative change is modelled
                 * as time that is masked by the previous occurrence.
                 * Masked time is matched by *, but is not matched
                 * by explicit enumeration or ranges of values such
                 * as 1,2,3 or 1-3.
                 */

                enum Mask mask = MaskNone;

                if (shadow->mTm.tm_year != transitionTm->tm_year)
                    mask |= MaskYears;
                if (shadow->mTm.tm_mon != transitionTm->tm_mon)
                    mask |= MaskMonths;
                if (shadow->mTm.tm_mday != transitionTm->tm_mday)
                    mask |= MaskDays;
                if (shadow->mTm.tm_hour != transitionTm->tm_hour)
                    mask |= MaskHours;
                if (shadow->mTm.tm_min != transitionTm->tm_min)
                    mask |= MaskMinutes;
                if (shadow->mTm.tm_min != transitionTm->tm_min)
                    mask |= MaskSeconds;

                shadow->mMask = mask;

                /* The scope of the masked time is limited by the
                 * interval repeated by the daylight savings change.
                 */

                shadow->mDst.mEnd = shadow->mDst.mBegin;
                shadow->mDst.mEnd.mTime -= aDstChange;

                shadow->mDst.mBegin.mTime = shadow->mTime;

                interval->mDst.mBegin.mTime -= aDstChange;
            }

        } else {

            if (interval->mTime - aDstChange < interval->mDst.mBegin.mTime) {

                /* Skipping caused by a positive change is modelled
                 * as artificial time that is inserted to describe
                 * the missing period.
                 */

                ++self->mInterval;
                struct Interval *shadow = civilTimeInterval_(self);

                *shadow = *interval;

                int isdst = shadow->mTm.tm_isdst;

                shadow->mTm = *transitionTm;
                shadow->mTm.tm_isdst = isdst;

                /* The scope of the artificial time is limited by
                 * the interval skipped by the daylight savings change.
                 */

                shadow->mDst.mEnd = shadow->mDst.mBegin;
                shadow->mDst.mEnd.mTime += aDstChange;

                shadow->mDst.mBegin.mTime = shadow->mTime;

                interval->mDst.mBegin.mTime = interval->mTime;
            }
        }
    }

    rc = 0;

Finally:

    return rc;
}

/* -------------------------------------------------------------------------- */
static int
initCivilTimeTm_(struct CivilTime *self, time_t aTime)
{
    int rc = -1;

    struct Interval *interval = civilTimeInterval_(self);

    time_t time = aTime / 60 * 60;

    struct tm *tmPtr = localtime(&time);
    if (!tmPtr)
        goto Finally;

    interval->mTime = time;
    interval->mTm = *tmPtr;

    rc = 0;

Finally:

    return rc;
}

/* -------------------------------------------------------------------------- */
struct CivilTime *
initCivilTime(struct CivilTime *self, time_t aTime)
{
    int rc = -1;

    self->mInterval = 0;

    struct Interval *interval = civilTimeInterval_(self);

    if (initCivilTimeTm_(self, aTime))
        goto Finally;

    interval->mMask = MaskNone;

    struct transition range[3];
    ttuntil(&interval->mTime, range);

    interval->mDst.mBegin.mTime = range[1].at;
    interval->mDst.mBegin.mOffset = range[1].off;
    interval->mDst.mEnd.mTime = range[2].at;
    interval->mDst.mEnd.mOffset = range[2].off;

    interval->mCalendar = calendar_(&interval->mTm);

    long dstChange = range[1].off - range[0].off;

    if (applyCivilTimeDstChange_(self, dstChange))
        goto Finally;

    rc = 0;

Finally:

    return rc ? 0 : self;
}

/* -------------------------------------------------------------------------- */
static int
shadowCivilTimeValue_(
    const struct CivilTime *self,
    enum Mask aMask,
    int aValue)
{
    const struct Interval *interval = civilTimeInterval_(self);

    return (aMask & interval->mMask) ? invertCivilTimeValue_(aValue) : aValue;
}

/* -------------------------------------------------------------------------- */
struct Calendar
queryCivilTimeCalendar(const struct CivilTime *self)
{
    const struct Interval *interval = civilTimeInterval_(self);

    struct Calendar calendar = {
        .mYear = shadowCivilTimeValue_(
            self, MaskYears, interval->mTm.tm_year + 1900),

        .mMonth = shadowCivilTimeValue_(
            self, MaskMonths, interval->mTm.tm_mon + 1),

        .mDay = shadowCivilTimeValue_(
            self, MaskDays, interval->mTm.tm_mday),

        .mWeekDay = interval->mTm.tm_wday,
        .mCalendar = interval->mCalendar,
    };

    return calendar;
}

/* -------------------------------------------------------------------------- */
struct Clock
queryCivilTimeClock(const struct CivilTime *self)
{
    const struct Interval *interval = civilTimeInterval_(self);

    struct Clock clock = {
        .mHour = shadowCivilTimeValue_(
            self, MaskHours, interval->mTm.tm_hour),

        .mMinute = shadowCivilTimeValue_(
            self, MaskMinutes, interval->mTm.tm_min),
    };

    return clock;
}

/* -------------------------------------------------------------------------- */
struct Calendar
queryCivilTimeWallCalendar(const struct CivilTime *self)
{
    const struct Interval *interval = civilTimeInterval_(self);

    struct Calendar wallCalendar = {
        .mYear = interval->mTm.tm_year + 1900,
        .mMonth = interval->mTm.tm_mon + 1,
        .mDay = interval->mTm.tm_mday,

        .mWeekDay = interval->mTm.tm_wday,
        .mCalendar = interval->mCalendar,
    };

    return wallCalendar;
}

/* -------------------------------------------------------------------------- */
struct Clock
queryCivilTimeWallClock(const struct CivilTime *self)
{
    const struct Interval *interval = civilTimeInterval_(self);

    struct Clock wallClock = {
        .mHour = interval->mTm.tm_hour,
        .mMinute = interval->mTm.tm_min,
    };

    return wallClock;
}

/* -------------------------------------------------------------------------- */
time_t
queryCivilTimeUtc(const struct CivilTime *self)
{
    const struct Interval *interval = civilTimeInterval_(self);

    return interval->mTime;
}

/* -------------------------------------------------------------------------- */
static time_t
utcTime(time_t aSince, struct tm *aTm)
{
    int rc = -1;

    /* Find the UTC time that corresponds to a local time, taking into
     * consideration that daylight savings causes local time to be skipped
     * or repeated.
     */

    struct tm queryTm = {
        .tm_year = aTm->tm_year,
        .tm_mon = aTm->tm_mon,
        .tm_mday = aTm->tm_mday,
        .tm_hour = aTm->tm_hour,
        .tm_min = aTm->tm_min,
        .tm_sec = aTm->tm_sec,
        .tm_isdst = -1,
    };

    struct tm leftTm = queryTm;
    time_t leftTime = mktime(&leftTm);
    if (-1 == leftTime)
        goto Finally;

    time_t utc;

    if (queryTm.tm_year != leftTm.tm_year
            || queryTm.tm_mon != leftTm.tm_mon
            || queryTm.tm_mday != leftTm.tm_mday
            || queryTm.tm_hour != leftTm.tm_hour
            || queryTm.tm_min != leftTm.tm_min
            || queryTm.tm_sec != leftTm.tm_sec) {

        /* A daylight savings change caused the local time to be skipped,
         * and the local time that most closely matches is returned along
         * with the corresponding UTC time.
         */

        *aTm = leftTm;
        utc = leftTime;

    } else {

        /* There will be two representations if the local time is repeated
         * due to a daylight savings change. If the repetition is detected,
         * choose the earliest time that is later than aSince.
         */

        struct tm rightTm = leftTm;

        rightTm.tm_isdst = ! leftTm.tm_isdst;
        time_t rightTime = mktime(&rightTm);

        struct tm *utcPtr;

        if (-1 == rightTime || rightTm.tm_isdst == leftTm.tm_isdst) {
            utcPtr = &leftTm;
        } else {
            if (leftTime < rightTime) {
                utcPtr = aSince < leftTime ? &leftTm : &rightTm;
            } else {
                utcPtr = aSince < rightTime ? &rightTm : &leftTm;
            }
        }

        *aTm = *utcPtr;
        utc = (utcPtr == &leftTm) ? leftTime : rightTime;
    }

    rc = 0;

Finally:

    return rc ? -1 : utc;
}

/* -------------------------------------------------------------------------- */
static struct Interval *
popCivilTimeInterval_(struct CivilTime *self)
{
    int rc = -1;

    if (!self->mInterval) {
        errno = ENOENT;
        goto Finally;
    }

    --self->mInterval;

    struct Interval *interval = civilTimeInterval_(self);

    if (initCivilTimeTm_(self, interval->mDst.mBegin.mTime))
        goto Finally;

    rc = 0;

Finally:

    return rc ? 0 : civilTimeInterval_(self);
}

/* -------------------------------------------------------------------------- */
static struct Interval *
restackCivilTimeInterval_(struct CivilTime *self, time_t aTime)
{
    int rc = -1;

    if (self->mInterval) {
        errno = ENOSPC;
        goto Finally;
    }

    if (!initCivilTime(self, aTime))
        goto Finally;

    rc = 0;

Finally:

    return rc ? 0 : civilTimeInterval_(self);
}

/* -------------------------------------------------------------------------- */
static struct Interval *
rewindCivilTimeMinute_(struct CivilTime *self, time_t aSince)
{
    int rc = -1;

    struct Interval *interval = civilTimeInterval_(self);

    interval->mTime -= interval->mTm.tm_min * 60;

    interval->mTm.tm_min = 0;

    struct tm tm = interval->mTm;
    time_t time = utcTime(aSince, &tm);

    if (-1 == time)
        goto Finally;

    if (time < interval->mDst.mBegin.mTime) {

        time = (interval->mDst.mBegin.mTime - 1) / 60 * 60;

        interval = popCivilTimeInterval_(self);
        if (!interval && ENOENT == errno)
            interval = restackCivilTimeInterval_(self, time);

        if (!interval)
            goto Finally;

        errno = EAGAIN;
        goto Finally;
    }

    if (time >= interval->mDst.mEnd.mTime) {

        time = (interval->mDst.mEnd.mTime + 59) / 60 * 60;

        interval = popCivilTimeInterval_(self);
        if (!interval && ENOENT == errno)
            interval = restackCivilTimeInterval_(self, time);

        if (!interval)
            goto Finally;

        errno = EAGAIN;
        goto Finally;
    }

    interval->mTm = tm;

    interval->mTime = time;

    interval->mCalendar = calendar_(&interval->mTm);

    rc = 0;

Finally:

    return rc ? 0 : civilTimeInterval_(self);
}

/* -------------------------------------------------------------------------- */
static struct Interval *
rewindCivilTimeHour_(struct CivilTime *self, time_t aSince)
{
    int rc = -1;

    struct Interval *interval = civilTimeInterval_(self);

    interval->mTime -= interval->mTm.tm_hour * (60 * 60);

    interval->mTm.tm_hour = 0;

    interval = rewindCivilTimeMinute_(self, aSince);
    if (!interval)
        goto Finally;

    rc = 0;

Finally:

    return rc ? 0 : interval;
}

/* -------------------------------------------------------------------------- */
static void
subtractCivilTimeDays_(struct CivilTime *self, time_t aDays)
{
    struct Interval *interval = civilTimeInterval_(self);

    interval->mTime -= aDays * (24 * 60 * 60);

    interval->mTm.tm_yday -= aDays;
    interval->mTm.tm_wday = (
        interval->mTm.tm_wday + DaysInWeek - (aDays % DaysInWeek)) % DaysInWeek;
}

/* -------------------------------------------------------------------------- */
static void
advanceCivilTimeDays_(struct CivilTime *self, time_t aDays)
{
    struct Interval *interval = civilTimeInterval_(self);

    interval->mTime += aDays * (24 * 60 * 60);

    interval->mTm.tm_yday += aDays;
    interval->mTm.tm_wday = (interval->mTm.tm_wday + aDays) % DaysInWeek;
}

/* -------------------------------------------------------------------------- */
static struct Interval *
rewindCivilTimeDay_(struct CivilTime *self, time_t aSince)
{
    int rc = -1;

    struct Interval *interval = civilTimeInterval_(self);

    subtractCivilTimeDays_(self, interval->mTm.tm_mday - 1);

    interval->mTm.tm_mday = 1;

    interval = rewindCivilTimeHour_(self, aSince);
    if (!interval)
        goto Finally;

    rc = 0;

Finally:

    return rc ? 0 : interval;
}

/* -------------------------------------------------------------------------- */
static struct Interval *
rewindCivilTimeMonth_(
    struct CivilTime *self, time_t aSince, const int *aCalendar)
{
    int rc = -1;

    struct Interval *interval = civilTimeInterval_(self);

    subtractCivilTimeDays_(self, aCalendar[interval->mTm.tm_mon + 1 - 1]);

    interval->mTm.tm_mon = 0;

    interval = rewindCivilTimeDay_(self, aSince);
    if (!interval)
        goto Finally;

    rc = 0;

Finally:

    return rc ? 0 : interval;
}

/* -------------------------------------------------------------------------- */
int
advanceCivilTimeMinute(struct CivilTime *self, int aMinute)
{
    int rc = -1;

    struct Interval *interval = civilTimeInterval_(self);

    if (aMinute < 0 || aMinute > 59) {
        errno = EINVAL;
        goto Finally;
    }

    if (aMinute <= interval->mTm.tm_min) {
        errno = ERANGE;
        goto Finally;
    }

    time_t minutes = aMinute - interval->mTm.tm_min;

    interval->mTime += minutes * 60;

    interval->mTm.tm_min = aMinute;

    rc = 0;

Finally:

    return rc;
}

/* -------------------------------------------------------------------------- */
int
advanceCivilTimeHour(struct CivilTime *self, int aHour)
{
    int rc = -1;

    struct Interval *interval = civilTimeInterval_(self);

    if (aHour < 0 || aHour > 59) {
        errno = EINVAL;
        goto Finally;
    }

    if (aHour <= interval->mTm.tm_hour) {
        errno = ERANGE;
        goto Finally;
    }

    time_t since = interval->mTime;

    time_t hours = aHour - interval->mTm.tm_hour;

    interval->mTime += hours * (60 * 60);

    interval->mTm.tm_hour = aHour;

    interval = rewindCivilTimeMinute_(self, since);
    if (!interval)
        goto Finally;

    rc = 0;

Finally:

    return rc;
}

/* -------------------------------------------------------------------------- */
int
advanceCivilTimeDay(struct CivilTime *self, int aDay)
{
    int rc = -1;

    struct Interval *interval = civilTimeInterval_(self);

    int lastDay =
        interval->mCalendar[interval->mTm.tm_mon + 1 - 1] -
        interval->mCalendar[interval->mTm.tm_mon + 1 - 0];

    if (aDay < 1 || aDay > lastDay) {
        errno = EINVAL;
        goto Finally;
    }

    if (aDay <= interval->mTm.tm_mday) {
        errno = ERANGE;
        goto Finally;
    }

    time_t since = interval->mTime;

    advanceCivilTimeDays_(self, aDay - interval->mTm.tm_mday);

    interval->mTm.tm_mday = aDay;

    interval = rewindCivilTimeHour_(self, since);
    if (!interval)
        goto Finally;

    rc = 0;

Finally:

    return rc;
}

/* -------------------------------------------------------------------------- */
int
advanceCivilTimeMonth(struct CivilTime *self, int aMonth)
{
    int rc = -1;

    struct Interval *interval = civilTimeInterval_(self);

    if (aMonth < 1 || aMonth > 12) {
        errno = EINVAL;
        goto Finally;
    }

    int month = aMonth - 1;

    if (month <= interval->mTm.tm_mon) {
        errno = ERANGE;
        goto Finally;
    }

    time_t since = interval->mTime;

    time_t days =
        interval->mCalendar[interval->mTm.tm_mon + 1] -
        interval->mCalendar[month + 1 - 1];

    advanceCivilTimeDays_(self, days);

    interval->mTm.tm_mon = month;

    interval = rewindCivilTimeDay_(self, since);
    if (!interval)
        goto Finally;

    rc = 0;

Finally:

    return rc;
}

/* -------------------------------------------------------------------------- */
int
advanceCivilTimeYear(struct CivilTime *self, int aYear)
{
    int rc = -1;

    struct Interval *interval = civilTimeInterval_(self);

    if (aYear < 1900) {
        errno = EINVAL;
        goto Finally;
    }

    int year = aYear - 1900;

    if (year <= interval->mTm.tm_year) {
        errno = ERANGE;
        goto Finally;
    }

    time_t since = interval->mTime;

    /* The following expressions must count all leap days in the elapsed
     * years in the half-open interval [mTm.tm_year, year) because the date is
     * advanced through the former year, but stops at the start of the
     * latter year.
     *
     * To obtain this, compute the difference in the number of leap days
     * encountered before the former, in other words (0, mTm.tm_year), and
     * before the latter, in other words (0, year).
     *
     * For example, to compute the number of leap years encountered before
     * year 5, count the leap days included in year 1, 2, 3, and 4:
     *
     *        ( 5 - 1 ) / 4 = 1
     *
     * Similarly, to compute the number of leap years encountered before
     * year 4, only include years 1, 2, and 3:
     *
     *        ( 4 - 1 ) / 4 = 1
     */

    int leapYears = 0;
    leapYears +=
        ((year + 1900 - 1) / 4 - (interval->mTm.tm_year + 1900 - 1) / 4);
    leapYears -=
        ((year + 1900 - 1) / 100 - (interval->mTm.tm_year + 1900 - 1) / 100);
    leapYears +=
        ((year + 1900 - 1) / 400 - (interval->mTm.tm_year + 1900 - 1) / 400);

    time_t days = CommonYear[0];

    days *= year - interval->mTm.tm_year;
    days += (LeapYear[0] - CommonYear[0]) * leapYears;

    /* Explicitly latch the calendar for the present year. This captures
     * whether a leap day needs to be taken into account when rewinding
     * the present month.
     */

    const int *calendar = interval->mCalendar;

    interval->mTime += days * (24 * 60 * 60);

    interval->mTm.tm_wday = (interval->mTm.tm_wday + days) % DaysInWeek;
    interval->mTm.tm_year = year;

    interval->mCalendar = 0;

    interval = rewindCivilTimeMonth_(self, since, calendar);
    if (!interval)
        goto Finally;

    rc = 0;

Finally:

    return rc;
}

/* -------------------------------------------------------------------------- */
