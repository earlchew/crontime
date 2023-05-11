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

#include "bitring.h"

#include "macros.h"
#include "parse.h"

#include <ctype.h>
#include <errno.h>

/* -------------------------------------------------------------------------- */
static int
initBitRingMembershipRange_(
    struct BitRing *self,
    unsigned long long aLhs,
    unsigned long long aRhs,
    unsigned long long aPeriod)
{
    int rc = -1;

    if (aLhs > aRhs || !aPeriod || (
            aLhs > self->mMax ||
            aRhs > self->mMax)) {
        errno = EINVAL;
        goto Finally;
    }

    for (int member = aLhs; ; member += aPeriod) {
        if (addBitRingMember(self, member))
            goto Finally;
        if (aPeriod > aRhs - member)
            break;
    }

    rc = 0;

Finally:

    return rc;
}

/* -------------------------------------------------------------------------- */
static int
initBitRingMembership_(struct BitRing *self, const char *aMembership)
{
    int rc = -1;

    const char *arg = aMembership;

    if (!isdigit((unsigned char) *arg)) {

        if ('*' != arg[0]) {
            errno = EINVAL;
            goto Finally;
        }

        if (arg[1]) {
            if ('/' != arg[1]) {
                errno = EINVAL;
                goto Finally;
            }

            unsigned long long period;
            arg = parseULongLong(&period, arg+2);
            if (!arg || *arg) {
                errno = EINVAL;
                goto Finally;
            }

            if (initBitRingMembershipRange_(
                    self, self->mMin, self->mMax, period))
                goto Finally;
        }

    } else {

        while (1) {
            unsigned long long lhs;
            arg = parseULongLong(&lhs, arg);
            if (!arg)
                goto Finally;

            unsigned long long period = 1;

            unsigned long long rhs;
            if ('-' != *arg) {
                rhs = lhs;
            } else {
                arg = parseULongLong(&rhs, arg+1);
                if (!arg)
                    goto Finally;

                if ('/' == *arg) {
                    arg = parseULongLong(&period, arg+1);
                    if (!arg) {
                        errno = EINVAL;
                        goto Finally;
                    }
                }
            }

            if (initBitRingMembershipRange_(self, lhs, rhs, period))
                goto Finally;

            if (!*arg)
                break;

            if (',' != *arg) {
                errno = EINVAL;
                goto Finally;
            }

            ++arg;
        }
    }

    rc = 0;

Finally:

    return rc;
}

/* -------------------------------------------------------------------------- */
struct BitRing *
initBitRing(struct BitRing *self, int aMin, int aMax, const char *aMembership)
{
    int rc = -1;

    if (aMax < aMin) {
	errno = EINVAL;
        goto Finally;
    }

    self->mRing = 0;
    self->mMin = aMin;
    self->mMax = aMax;

    if (aMembership) {
        if (initBitRingMembership_(self, aMembership))
            goto Finally;
    }

    rc = 0;

Finally:

    return rc ? 0 : self;
}

/* -------------------------------------------------------------------------- */
int
addBitRingMember(struct BitRing *self, int aMember)
{
    int rc = -1;

    if (aMember < self->mMin || aMember > self->mMax) {
        errno = EINVAL;
        goto Finally;
    }

    self->mRing |= (BitRingT) 1 << (aMember - self->mMin);

    rc = 0;

Finally:

    return rc;
}

/* -------------------------------------------------------------------------- */
int
queryBitRingMin(const struct BitRing *self)
{
    return self->mMin;
}

/* -------------------------------------------------------------------------- */
int
queryBitRingMax(const struct BitRing *self)
{
    return self->mMax;
}

/* -------------------------------------------------------------------------- */
unsigned
queryBitRingPopulation(const struct BitRing *self)
{
    return __builtin_popcountll(self->mRing);
}

/* -------------------------------------------------------------------------- */
int
queryBitRingMembership(const struct BitRing *self, int aMember)
{
    int rc;

    if (aMember < self->mMin || aMember > self->mMax)
        rc = 0;
    else
        rc = !! (self->mRing & ((BitRingT) 1 << (aMember - self->mMin)));

    return rc;
}

/* -------------------------------------------------------------------------- */
int
queryBitRingMemberSeparation(const struct BitRing *self, int aMember)
{
    int rc = -1;

    if (aMember < self->mMin || aMember > self->mMax) {
        errno = EINVAL;
        goto Finally;
    }

    int nextMember = __builtin_ffsll(
        (self->mRing >> (aMember - self->mMin)) >> 1);

    int separation;

    if (nextMember) {
        separation = nextMember - 1  + 1;
    } else {
        int firstMember = __builtin_ffsll(self->mRing);

        if (firstMember)
            separation = firstMember - 1 + (self->mMax - aMember + 1);
        else
            separation = 0;
    }

    rc = 0;

Finally:

    return rc ? rc : separation;
}

/* -------------------------------------------------------------------------- */
