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

#include "gtest/gtest.h"

#include <errno.h>

/* -------------------------------------------------------------------------- */
class BitRingTest : public ::testing::Test
{
public:

    BitRingTest()
        : mBitRing(&mBitRing_)
    { }

private:

    void SetUp()
    {
        EXPECT_EQ(mBitRing, initBitRing(mBitRing, 1, 64, 0));
        EXPECT_EQ(1, queryBitRingMin(mBitRing));
        EXPECT_EQ(64, queryBitRingMax(mBitRing));
    }

protected:

    struct BitRing mBitRing_, * const mBitRing;
};

/* -------------------------------------------------------------------------- */
TEST_F(BitRingTest, Empty)
{
    EXPECT_FALSE(queryBitRingPopulation(mBitRing));

    for (auto member = queryBitRingMin(mBitRing); ; ++member) {
        EXPECT_FALSE(queryBitRingMembership(mBitRing, member));
        if (member == queryBitRingMax(mBitRing))
            break;
    }
}

/* -------------------------------------------------------------------------- */
TEST_F(BitRingTest, Membership)
{
    int max = queryBitRingMax(mBitRing);

    EXPECT_TRUE(addBitRingMember(mBitRing, max+1));
    EXPECT_EQ(EINVAL, errno);

    EXPECT_FALSE(queryBitRingMembership(mBitRing, max));
    EXPECT_FALSE(addBitRingMember(mBitRing, max));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, max));
    EXPECT_EQ(1U, queryBitRingPopulation(mBitRing));

    int min = queryBitRingMin(mBitRing);

    EXPECT_TRUE(addBitRingMember(mBitRing, min-1));
    EXPECT_EQ(EINVAL, errno);

    EXPECT_FALSE(queryBitRingMembership(mBitRing, min));
    EXPECT_FALSE(addBitRingMember(mBitRing, min));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, min));
    EXPECT_EQ(2U, queryBitRingPopulation(mBitRing));

    int mid = (min + max) / 2;

    EXPECT_FALSE(queryBitRingMembership(mBitRing, mid));
    EXPECT_FALSE(addBitRingMember(mBitRing, mid));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, mid));
    EXPECT_EQ(3U, queryBitRingPopulation(mBitRing));
}

/* -------------------------------------------------------------------------- */
TEST_F(BitRingTest, Separation)
{
    int min = queryBitRingMin(mBitRing);
    int max = queryBitRingMax(mBitRing);
    int mid = (min + max) / 2;

    EXPECT_EQ(-1, queryBitRingMemberSeparation(mBitRing, min-1));
    EXPECT_EQ(EINVAL, errno);

    EXPECT_EQ(-1, queryBitRingMemberSeparation(mBitRing, max+1));
    EXPECT_EQ(EINVAL, errno);

    EXPECT_FALSE(queryBitRingMemberSeparation(mBitRing, mid));

    EXPECT_FALSE(queryBitRingMembership(mBitRing, mid));
    EXPECT_FALSE(addBitRingMember(mBitRing, mid));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, mid));
    EXPECT_EQ(1U, queryBitRingPopulation(mBitRing));

    EXPECT_EQ(1, queryBitRingMemberSeparation(mBitRing, mid-1));
    EXPECT_EQ(max-0, queryBitRingMemberSeparation(mBitRing, mid-0));
    EXPECT_EQ(max-1, queryBitRingMemberSeparation(mBitRing, mid+1));

    EXPECT_FALSE(queryBitRingMembership(mBitRing, max));
    EXPECT_FALSE(addBitRingMember(mBitRing, max));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, max));
    EXPECT_EQ(2U, queryBitRingPopulation(mBitRing));

    EXPECT_EQ(1, queryBitRingMemberSeparation(mBitRing, max-1));
    EXPECT_EQ(mid-min+1, queryBitRingMemberSeparation(mBitRing, max-0));

    EXPECT_FALSE(queryBitRingMembership(mBitRing, min));
    EXPECT_FALSE(addBitRingMember(mBitRing, min));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, min));
    EXPECT_EQ(3U, queryBitRingPopulation(mBitRing));

    EXPECT_EQ(1, queryBitRingMemberSeparation(mBitRing, max));
    EXPECT_EQ(mid-min, queryBitRingMemberSeparation(mBitRing, min-0));
}

/* -------------------------------------------------------------------------- */
TEST_F(BitRingTest, InitInvalid)
{
    EXPECT_FALSE(initBitRing(mBitRing, 1, 7, " *"));
    EXPECT_EQ(EINVAL, errno);

    EXPECT_FALSE(initBitRing(mBitRing, 1, 7, "*="));
    EXPECT_EQ(EINVAL, errno);

    EXPECT_FALSE(initBitRing(mBitRing, 1, 7, "*/"));
    EXPECT_EQ(EINVAL, errno);

    EXPECT_FALSE(initBitRing(mBitRing, 1, 7, "1/"));
    EXPECT_EQ(EINVAL, errno);

    EXPECT_FALSE(initBitRing(mBitRing, 1, 7, "-"));
    EXPECT_EQ(EINVAL, errno);

    EXPECT_FALSE(initBitRing(mBitRing, 1, 7, "8"));
    EXPECT_EQ(EINVAL, errno);

    EXPECT_FALSE(initBitRing(mBitRing, 1, 7, "0 "));
    EXPECT_EQ(EINVAL, errno);
}

/* -------------------------------------------------------------------------- */
TEST_F(BitRingTest, InitWildcard)
{
    EXPECT_EQ(mBitRing, initBitRing(mBitRing, 1, 7, "*"));
    EXPECT_FALSE(queryBitRingPopulation(mBitRing));
}

/* -------------------------------------------------------------------------- */
TEST_F(BitRingTest, InitPeriod)
{
    EXPECT_EQ(mBitRing, initBitRing(mBitRing, 1, 7, "*/2"));
    EXPECT_EQ(4U, queryBitRingPopulation(mBitRing));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 1));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 3));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 5));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 7));
}

/* -------------------------------------------------------------------------- */
TEST_F(BitRingTest, InitRange)
{
    EXPECT_EQ(mBitRing, initBitRing(mBitRing, 1, 7, "2-5"));
    EXPECT_EQ(4U, queryBitRingPopulation(mBitRing));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 2));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 3));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 4));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 5));
}

/* -------------------------------------------------------------------------- */
TEST_F(BitRingTest, InitRangePeriod)
{
    EXPECT_EQ(mBitRing, initBitRing(mBitRing, 1, 7, "2-5/2"));
    EXPECT_EQ(2U, queryBitRingPopulation(mBitRing));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 2));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 4));
}

/* -------------------------------------------------------------------------- */
TEST_F(BitRingTest, InitList)
{
    EXPECT_EQ(mBitRing, initBitRing(mBitRing, 1, 7, "2,4,6"));
    EXPECT_EQ(3U, queryBitRingPopulation(mBitRing));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 2));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 4));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 6));
}

/* -------------------------------------------------------------------------- */
TEST_F(BitRingTest, InitListRange)
{
    EXPECT_EQ(mBitRing, initBitRing(mBitRing, 1, 7, "2,4-7/2,1"));
    EXPECT_EQ(4U, queryBitRingPopulation(mBitRing));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 1));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 2));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 4));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 6));
}

/* -------------------------------------------------------------------------- */
TEST_F(BitRingTest, InitDuplicate)
{
    EXPECT_EQ(mBitRing, initBitRing(mBitRing, 1, 7, "2,2-3/2,2"));
    EXPECT_EQ(1U, queryBitRingPopulation(mBitRing));
    EXPECT_TRUE(queryBitRingMembership(mBitRing, 2));
}

/* -------------------------------------------------------------------------- */

#include "_test_.h"
