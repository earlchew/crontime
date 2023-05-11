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
#include "macros.h"

#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
struct Schedule *
initSchedule(struct Schedule *self, const char *aSchedule)
{
    int rc = -1;

    char *schedule = strdup(aSchedule);
    if (!schedule)
        goto Finally;

    static const char CronSep[] = "\t ";

    char *scheduleWords[5];
    char **scheduleWordPtr = &scheduleWords[NUMBEROF(scheduleWords)];

    char *schedulePtr = schedule;

    while (scheduleWordPtr != scheduleWords) {
        char *wordPtr = strsep(&schedulePtr, CronSep);
        if (!wordPtr) {
            errno = EINVAL;
            goto Finally;
        }

        *--scheduleWordPtr = wordPtr;
    }

    if (schedulePtr) {
        errno = EINVAL;
        goto Finally;
    }

    if (!initBitRing(
            &self->mSchedules[ScheduleMinutes], 0, 59, scheduleWords[4]))
        goto Finally;

    if (!initBitRing(
            &self->mSchedules[ScheduleHours], 0, 23, scheduleWords[3]))
        goto Finally;

    if (!initBitRing(
            &self->mSchedules[ScheduleDays], 1, 31, scheduleWords[2]))
        goto Finally;

    if (!initBitRing(
            &self->mSchedules[ScheduleMonths], 1, 12, scheduleWords[1]))
        goto Finally;

    struct BitRing weekDays;

    if (!initBitRing(&weekDays, 0, 7, scheduleWords[0]))
        goto Finally;

    int firstDay = queryBitRingMin(&weekDays);
    int lastDay = queryBitRingMin(&weekDays);

    if (!initBitRing(
            &self->mSchedules[ScheduleWeekDays],
            firstDay,
            firstDay + DaysInWeek - 1, 0))
        goto Finally;

    /* Transfer the selected days of the week from the 0-7 range to the
     * canononical 0-6 range.
     */

    for (int weekday = firstDay; ; ++weekday) {

        if (queryBitRingMembership(&weekDays, weekday)) {
            if (addBitRingMember(
                    &self->mSchedules[ScheduleWeekDays],
                    firstDay + (weekday - firstDay) % DaysInWeek))
                goto Finally;
        }

        if (weekday == lastDay)
            break;
    }

    rc = 0;

Finally:

    FINALLY({
        free(schedule);
        schedule = 0;
    });

    return rc ? 0 : self;
}

/* -------------------------------------------------------------------------- */
static int
queryScheduleNext_(
    const struct Schedule *self, enum ScheduleKind aScheduleKind, int aValue)
{
    int rc = -1;

    const struct BitRing *bitring = &self->mSchedules[aScheduleKind];

    int delta = queryBitRingMemberSeparation(bitring, aValue);
    if (-1 == delta)
        goto Finally;

    if (!delta)
        delta = 1;

    if (delta > queryBitRingMax(bitring) - aValue) {
        errno = EAGAIN;
        goto Finally;
    }

    rc = 0;

Finally:

    return rc ? -1 : aValue + delta;
}

/* -------------------------------------------------------------------------- */
static int
queryScheduleMinute_(const struct Schedule *self, struct CivilTime *aCivilTime)
{
    int rc = -1;

    while (1) {

        int matched = 1;

        if (queryBitRingPopulation(&self->mSchedules[ScheduleMinutes])) {
            matched =
                queryBitRingMembership(
                    &self->mSchedules[ScheduleMinutes],
                    queryCivilTimeClock(aCivilTime).mMinute);
        }

        if (matched)
            break;

        int minute =
            queryScheduleNext_(
                self,
                ScheduleMinutes,
                queryCivilTimeWallClock(aCivilTime).mMinute);
        if (-1 == minute)
            goto Finally;

        if (advanceCivilTimeMinute(aCivilTime, minute)) {
            if (errno != EAGAIN)
                goto Finally;
        }
    }

    rc = 0;

Finally:

    return rc;
}

/* -------------------------------------------------------------------------- */
static int
queryScheduleHour_(const struct Schedule *self, struct CivilTime *aCivilTime)
{
    int rc = -1;

    while (1) {

        int matched = 1;

        if (queryBitRingPopulation(&self->mSchedules[ScheduleHours])) {
            matched =
                queryBitRingMembership(
                    &self->mSchedules[ScheduleHours],
                    queryCivilTimeClock(aCivilTime).mHour);
        }

        if (matched) {
            if (!queryScheduleMinute_(self, aCivilTime))
                break;
            if (EAGAIN != errno)
                goto Finally;
        }

        int hour =
            queryScheduleNext_(
                self,
                ScheduleHours,
                queryCivilTimeWallClock(aCivilTime).mHour);
        if (-1 == hour)
            goto Finally;

        if (advanceCivilTimeHour(aCivilTime, hour)) {
            if (EAGAIN != errno)
                goto Finally;
        }
    }

    rc = 0;

Finally:

    return rc;
}

/* -------------------------------------------------------------------------- */
static int
queryScheduleDay_(const struct Schedule *self, struct CivilTime *aCivilTime)
{
    int rc = -1;

    while (1) {

        int matched = 1;

        do {
            struct Calendar calendar = queryCivilTimeCalendar(aCivilTime);

            int day     = calendar.mDay;
            int weekDay = calendar.mWeekDay;

            if ((queryBitRingPopulation(&self->mSchedules[ScheduleWeekDays]) ||
                    queryBitRingPopulation(&self->mSchedules[ScheduleDays]))) {
                matched =
                    queryBitRingMembership(
                        &self->mSchedules[ScheduleWeekDays], weekDay) |
                    queryBitRingMembership(
                        &self->mSchedules[ScheduleDays], day);
            }

        } while (0);

        if (matched) {
            if (!queryScheduleHour_(self, aCivilTime))
                break;
            if (EAGAIN != errno)
                goto Finally;
        }

        struct Calendar wallCalendar = queryCivilTimeWallCalendar(aCivilTime);

        int wallWeekDay = wallCalendar.mWeekDay;
        int wallDay     = wallCalendar.mDay;
        int wallMonth   = wallCalendar.mMonth;

        int skipWeekDays = queryBitRingMemberSeparation(
            &self->mSchedules[ScheduleWeekDays], wallWeekDay);
        if (-1 == skipWeekDays)
            goto Finally;

        int skipDays = queryBitRingMemberSeparation(
            &self->mSchedules[ScheduleDays], wallDay);
        if (-1 == skipDays)
            goto Finally;

        int deltaDays;
        if (skipWeekDays && skipDays)
            deltaDays = skipWeekDays < skipDays ? skipWeekDays : skipDays;
        else if (skipWeekDays)
            deltaDays = skipWeekDays;
        else if (skipDays)
            deltaDays = skipDays;
        else
            deltaDays = 1;

        int lastDay =
            (wallCalendar.mCalendar[wallMonth-1]
                - wallCalendar.mCalendar[wallMonth]);

        if (deltaDays > lastDay - wallDay) {
            errno = EAGAIN;
            goto Finally;
        }

        if (advanceCivilTimeDay(aCivilTime, wallDay + deltaDays)) {
            if (EAGAIN != errno)
                goto Finally;
        }
    }

    rc = 0;

Finally:

    return rc;
}

/* -------------------------------------------------------------------------- */
static int
queryScheduleMonth_(const struct Schedule *self, struct CivilTime *aCivilTime)
{
    int rc = -1;

    while (1) {

        int matched = 1;

        if (queryBitRingPopulation(&self->mSchedules[ScheduleMonths])) {
            matched =
                queryBitRingMembership(
                    &self->mSchedules[ScheduleMonths],
                    queryCivilTimeCalendar(aCivilTime).mMonth);
        }

        if (matched) {
            if (!queryScheduleDay_(self, aCivilTime))
                break;
            if (EAGAIN != errno)
                goto Finally;
        }

        int month =
            queryScheduleNext_(
                self,
                ScheduleMonths,
                queryCivilTimeWallCalendar(aCivilTime).mMonth);
        if (-1 == month)
            goto Finally;

        if (advanceCivilTimeMonth(aCivilTime, month)) {
            if (EAGAIN != errno)
                goto Finally;
        }
    }

    rc = 0;

Finally:

    return rc;
}

/* -------------------------------------------------------------------------- */
static int
queryScheduleYear_(const struct Schedule *self, struct CivilTime *aCivilTime)
{
    int rc = -1;

    while (queryScheduleMonth_(self, aCivilTime)) {
        if (EAGAIN != errno)
            goto Finally;

        int year = queryCivilTimeCalendar(aCivilTime).mYear;

        if (advanceCivilTimeYear(aCivilTime, year+1)) {
            if (EAGAIN != errno)
                goto Finally;
        }
    }

    rc = 0;

Finally:

    return rc;
}

/* -------------------------------------------------------------------------- */
time_t
querySchedule(
    const struct Schedule *self,
    const struct CivilTime *aCivilTime,
    time_t aJitterPeriod)
{
    int rc = -1;

    struct CivilTime schedTime_ = *aCivilTime, *schedTime = &schedTime_;

    if (queryScheduleYear_(self, schedTime))
        goto Finally;

    time_t scheduled = queryCivilTimeUtc(schedTime);

    /* If a jitter time is requested, try to build a triangular probability
     * density function centred around the scheduled time. Determine the
     * duration until the scheduled time, and the duration from the scheduled
     * time to the jitter time.
     */

    if (aJitterPeriod) {
        struct CivilTime jitterTime_, *jitterTime = &jitterTime_;

        if (!initCivilTime(jitterTime, scheduled + 60))
            goto Finally;

        if (queryScheduleYear_(self, jitterTime))
            goto Finally;

        time_t jittered = queryCivilTimeUtc(jitterTime);

        if (jittered <= scheduled) {
            errno = EINVAL;
            goto Finally;
        }

        /* The lhs period might be zero if there is no time until
         * the scheduled time. In this case, use a one-sided
         * probably density function.
         */

        time_t rhsPeriod = jittered - scheduled;
        time_t lhsPeriod = scheduled - queryCivilTimeUtc(aCivilTime);

        time_t period =
            lhsPeriod && lhsPeriod < rhsPeriod ? lhsPeriod : rhsPeriod;

        if (period > aJitterPeriod)
            period = aJitterPeriod;

        int random = rand();

        time_t jitter = period * (1 - sqrt(random / (1.0 * RAND_MAX)));

        if ((random % 2) && lhsPeriod)
            scheduled -= jitter;
        else
            scheduled += jitter;
    }

    rc = 0;

Finally:

    return rc ? -1 : scheduled;
}

/* -------------------------------------------------------------------------- */
