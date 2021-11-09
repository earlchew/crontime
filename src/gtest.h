#ifndef GTEST_H
#define GTEST_H

#ifndef HEADERCHECK
#define GTEST_HAS_PTHREAD 0
#include "gtest/gtest.h"

// Ensure that there is code that depends on the successful inclusion of
// the underlying gtest header so that the delete-orphaned-includes.sh
// script will not remove the inclusion.

namespace {
typedef ::testing::Test CronTimeTest_T_;
}
#endif

#endif /* GTEST_H */
