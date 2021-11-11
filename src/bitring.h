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
#ifndef BITRING_H
#define BITRING_H

#include <inttypes.h>

#include "compiler.h"

/* -------------------------------------------------------------------------- */
BEGIN_C_SCOPE;

typedef uint64_t BitRingT;

struct BitRing {
    BitRingT mRing;
    int mMin;
    int mMax;
};

/* -------------------------------------------------------------------------- */
struct BitRing *
initBitRing(struct BitRing *self, int aMin, int aMax, const char *aMembership);

int
addBitRingMember(struct BitRing *self, int aMember);

/* -------------------------------------------------------------------------- */
int
queryBitRingMin(const struct BitRing *self);

int
queryBitRingMax(const struct BitRing *self);

/* -------------------------------------------------------------------------- */
unsigned
queryBitRingPopulation(const struct BitRing *self);

int
queryBitRingMembership(const struct BitRing *self, int aMember);

int
queryBitRingMemberSeparation(const struct BitRing *self, int aMember);

/* -------------------------------------------------------------------------- */
END_C_SCOPE;

#endif /* BITRING_H */
