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
#ifndef CIVILTIME_H
#define CIVILTIME_H

#include <time.h>

#include "compiler.h"

/* -------------------------------------------------------------------------- */
BEGIN_C_SCOPE;

/* https://pubs.opengroup.org/onlinepubs/9699919799/utilities/crontab.html */
/* https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/time.h.html */

enum WeekDay {
    Sunday = 0,
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
    FirstDay = Sunday,
    LastDay = Saturday,
    DaysInWeek = LastDay - FirstDay + 1,
};

struct Calendar {
    int mYear;  /* 1900 etc */
    int mMonth; /* 1 - 12 */
    int mDay;   /* 1 - 31 */

    enum WeekDay mWeekDay;

    const int *mCalendar; /* Days in month */
};

struct Clock {
    int mHour;   /* 0 - 23 */
    int mMinute; /* 0 - 59 */
};

struct Transition {
    time_t mTime;
    long mOffset;
};

struct DaylightSavings {
    struct Transition mBegin;
    struct Transition mEnd;
};

enum Mask {
    MaskNone    = 0x00,
    MaskSeconds = 0x01,
    MaskMinutes = 0x02,
    MaskHours   = 0x04,
    MaskDays    = 0x10,
    MaskMonths  = 0x20,
    MaskYears   = 0x40,
};

struct Interval {
    struct tm mTm;
    time_t mTime;

    unsigned mMask;

    struct DaylightSavings mDst;

    const int *mCalendar;
};

struct CivilTime {
    int mInterval;
    struct Interval mIntervals[2];
};

/* -------------------------------------------------------------------------- */
struct CivilTime *
initCivilTime(struct CivilTime *self, time_t aTime);

/* -------------------------------------------------------------------------- */
static __inline__ int
invertCivilTimeValue_(int aValue)
{
    return 0 - 1 - aValue;
}

/* -------------------------------------------------------------------------- */
static __inline__ int
nominalCivilTimeValue_(int aValue)
{
    return aValue < 0 ? invertCivilTimeValue_(aValue) : aValue;
}

/* -------------------------------------------------------------------------- */
int
advanceCivilTimeMinute(struct CivilTime *self, int aMinute);

int
advanceCivilTimeHour(struct CivilTime *self, int aHour);

int
advanceCivilTimeDay(struct CivilTime *self, int aDay);

int
advanceCivilTimeMonth(struct CivilTime *self, int aMonth);

int
advanceCivilTimeYear(struct CivilTime *self, int aYear);

/* -------------------------------------------------------------------------- */
struct Calendar
queryCivilTimeWallCalendar(const struct CivilTime *self);

struct Clock
queryCivilTimeWallClock(const struct CivilTime *self);

struct Calendar
queryCivilTimeCalendar(const struct CivilTime *self);

struct Clock
queryCivilTimeClock(const struct CivilTime *self);

time_t
queryCivilTimeUtc(const struct CivilTime *self);

/* -------------------------------------------------------------------------- */
END_C_SCOPE;

#endif /* CIVILTIME_H */
