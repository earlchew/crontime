# Copyright (c) 2021, Earl Chew
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the names of the authors of source code nor the names
#       of the contributors to the source code may be used to endorse or
#       promote products derived from this software without specific
#       prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL EARL CHEW BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

$(eval include $(top_srcdir)/wildcard.mk)

include $(top_srcdir)/headercheck.mk

VERSION = 1.0.0
LDADD   =

AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS   = -I.
AM_CFLAGS     = $(TEST_CFLAGS)
AM_CXXFLAGS   = $(TEST_CXXFLAGS)
AM_LDFLAGS    =

OPT_FLAGS          = -O2
COMMON_FLAGS       = $(OPT_FLAGS)
COMMON_FLAGS      += -D_GNU_SOURCE -Wall -Werror
COMMON_FLAGS      += -Wno-parentheses -Wshadow
COMMON_CFLAGS      = $(COMMON_FLAGS) -std=gnu99
COMMON_CFLAGS     += -fdata-sections -ffunction-sections
COMMON_CFLAGS     += -Wmissing-prototypes -Wmissing-declarations
COMMON_CXXFLAGS    = $(COMMON_FLAGS) -std=gnu++0x
COMMON_CXXFLAGS   += -Wno-variadic-macros -Wno-long-long
COMMON_LINKFLAGS   =
TEST_LIBS          = libcrontime_.la libtz_.la libgoogletest.la
TEST_FLAGS         = -DUNITTEST -I ../googletest/googletest/include
TEST_CFLAGS        = $(TEST_FLAGS) $(COMMON_CFLAGS)
TEST_CXXFLAGS      = $(TEST_FLAGS) $(COMMON_CXXFLAGS)
TESTS              = $(check_PROGRAMS) $(check_SCRIPTS)
TEST_EXTENSIONS    = .sh # Avoid using valgrind over shell scripts

crontimedir        = $(bindir)
crontime_PROGRAMS  = crontime
check_SCRIPTS      = test.sh
check_PROGRAMS     = $(crontime_TESTS)
noinst_PROGRAMS    =
noinst_SCRIPTS     = $(check_SCRIPTS)
noinst_LTLIBRARIES = libcrontime_.la libtz_.la libgoogletest.la
lib_LTLIBRARIES    =

crontime_CFLAGS    = $(COMMON_CFLAGS)
crontime_LDFLAGS   = $(COMMON_LINKFLAGS)
crontime_LDADD     = libcrontime_.la libtz_.la -lm
crontime_SOURCES   = _crontime.c

include libtz__la.am
$(call WILDCARD_LIB,libtz__la,libtz__la_SOURCES,tz/[a-z]*[^_].[ch])
libtz__la_CFLAGS  = $(COMMON_CFLAGS) -Wno-error
libtz__la_CFLAGS += -Wno-address -Wno-maybe-uninitialized

include libcrontime__la.am
$(call WILDCARD_LIB,libcrontime__la,libcrontime__la_SOURCES,[a-z]*[^_].[ch])
libcrontime__la_CFLAGS = $(COMMON_CFLAGS)

include crontime_tests.am
$(call WILDCARD_TESTS,crontime_tests,crontime_TESTS,__*.c __*.cc,$$(TEST_LIBS))

libgoogletest_la_SOURCES  = gtest-all.cc
libgoogletest_la_CPPFLAGS = -I ../googletest/googletest

@VALGRIND_CHECK_RULES@

programs:	all
	$(MAKE) $(AM_MAKEFLAGS) $(check_PROGRAMS) $(check_SCRIPTS)

check:
	$(MAKE) $(AM_MAKEFLAGS) check-valgrind
