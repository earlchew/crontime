crontime
========

Calculate local timezone events for crontab schedules.

#### Background

Crontab schedules are an established way to describe schedules
for recurring activities. The crontime program computes the
next event time in the local timezone for a crontab schedule,
taking into account daylight savings changes.

#### Dependencies

* GNU Make
* GNU Automake
* GNU C
* GNU C++ for tests

#### Build

* Run `autogen.sh`
* Configure using `configure`
* Build binaries using `make`
* Run tests using `make check`

#### Usage

```
usage: crontime [ options ] time [ schedule ] [ < schedule ]

options:
  -j,--jitter N   Jitter the schedule by N seconds [default: 300]

arguments:
  time       Time specific as Unix epoch (eg 1636919408)
  schedule   Schedule using crontab(5) expression (eg * * * * *)
```

#### Examples

```
% unset LANG
% export TZ=US/Pacific
% NOW=949181283
% date -d @$NOW
Sat Jan 29 13:28:03 PST 2000
% set -- $(crontime $NOW '*/5 * * * *')
% DEADLINE=$1
% JITTER=$2
% SCHEDULED=$(( DEADLINE - JITTER ))
% date -d @$SCHEDULED
Sat Jan 29 13:30:00 PST 2000
```

```
% unset LANG
% export TZ=US/Pacific
% NOW=949181283
% date -d @$NOW
Sat Jan 29 13:28:03 PST 2000
% set -- $(crontime $NOW '43 6-9 15-20 5,6 *')
% DEADLINE=$1
% JITTER=$2
% SCHEDULED=$(( DEADLINE - JITTER ))
% date -d @$SCHEDULED
Mon May 15 06:43:00 PDT 2000
```

#### Motivation

[Ksh](https://github.com/ksh93/ksh/blob/master/src/lib/libast/tm/tmxdate.c#L521)
can interpret crontab schedules but has a few limitations:
* Only schedules against the current time of day
* Schedules across daylight savings changes can be surprising

During daylight savings changes that cause the clock
to jump backwards can result in events to being
skipped. For example, in the US/Pacific timezone,
time advances as follows during such a change:

```
% unset LANG
% export TZ='US/Pacific'

% date +'%s %c' -d @$((972802800 + 3600 * 0))
972802800 Sun Oct 29 00:00:00 2000

% date +'%s %c' -d @$((972802800 + 3600 * 1))
972806400 Sun Oct 29 01:00:00 2000

% date +'%s %c' -d @$((972802800 + 3600 * 2))
972810000 Sun Oct 29 01:00:00 2000

% date +'%s %c' -d @$((972802800 + 3600 * 3))
972813600 Sun Oct 29 02:00:00 2000
```

An hourly schedule evaluated at 12:59 AM PDT should
result in an event at 01:00 AM PDT, but instead
results in an event at 01:00 AM PST.

```
% unset LANG
% export TZ='US/Pacific'
% sudo date +'%s %c' -s @$(( 972806400 - 60  ))
% date +'%s %c' -d @$(ksh -c "printf '%(%s)T\n' '0 * * * *' ")
972806340 Sun Oct 29 12:59:00 2000
972810000 Sun Oct 29 01:00:00 2000

% echo $(( 972810000 - 972806340 ))
3660

```

During daylight savings changes that cause the clock to
jump forwards can result in events being scheduled in the past.
For example, in the US/Pacific timezone, time advances
as follows during such a change:

```
% date +'%s %c' -d @$((954662400 + 3600 * 0))
954662400 Sun Apr 02 12:00:00 2000

% date +'%s %c' -d @$((954662400 + 3600 * 1))
954666000 Sun Apr 02 01:00:00 2000

% date +'%s %c' -d @$((954662400 + 3600 * 2))
954669600 Sun Apr 02 03:00:00 2000

% date +'%s %c' -d @$((954662400 + 3600 * 3))
954673200 Sun Apr 02 04:00:00 2000
```

An hourly schedule evaluated at 01:30 AM PST should
result in an event at 03:00 AM PDT, but instead
results in an event 30 minutes earlier at 01:00 AM PST
rather than 30 minutes later.

```
% export TZ='US/Pacific'
% sudo date +'%s %c' -s @$(( 954669600 - 60 * 30 ))
% date +'%s %c' -d @$(ksh -c "printf '%(%s)T\n' '0 * * * *' ")
954667800 Sun Apr 02 01:30:00 2000
954666000 Sun Apr 02 01:00:00 2000

% echo $(( 954666000 - 954667800 ))
-1800
```

#### Jitter

Unless overridden by the `--jitter 0` option, a small amount of jitter
is added to each scheduled time to counter the tendency to synchronise
events to the start of each minute, or hour.

If the event is scheduled far enough into the future, the jitter will
be symmetric around the scheduled time. Otherwise, the jitter will
be one-sided and will delay the event by a random time.

#### Daylight Savings

Local timezones typically to contend with discontinuities due to daylight
savings adjustments. The disconinuities interact with crontab schedules,
resulting in outcomes that are sometimes unexpected.

#### Beginning Daylight Savings

Typically during spring, a daylight savings change will advance the local
clock and cause the apparent time to skip ahead. For example, in the
US/Pacific timezone, the clock will instantaneously advance from 02:00
to 03:00, as illustrated in the following table:

```
UTC    08  09  10  11  12  13
Local  00  01 [02]
               03  04  05  06

[02]  Hour that is skipped but that will match a schedule
```

To avoid the surprise of a missing event scheduled at the skipped hour,
(eg `30 2 * * *`), or a periodic event during the skipped hour
(eg `30 * * * *`) , the computed schedule will first try to match the
schedule with the skipped hour (as indicated by `[02]`), and then try to
match the new daylight savings time (as indicated by `03`).

This accommodation for the daylight savings change only applies for
the transition period (ie `[02]`, `03`), and once past the scheduler
reverts to the standard behaviour (ie UTC `11`).

The following table shows some examples:

```
30 2   * * *  UTC 1030
30 2,3 * * *  UTC 1030
30 3   * * *  UTC 1030
30 0-4 * * *  UTC 0830, UTC 0930, UTC 1030, UTC 1130
```

The following graphs show a visualisation of scheduling during this
daylight savings change. The blue lines show local time relative to
midnight of the morning of the change highlighting the jump from 0200
to 0300. The red lines show the behaviour for both an explicit hourly
schedule (ie `0-23`) and an implicit hourly schedule using a wildcard
(ie `*`). Just after the hour, the delay is 59 minutes, and just before
the hour, the delay is 1 minute. This is the expected result for events
scheduled hourly, even when `0200` is skipped.

![](https://github.com/earlchew/crontime/blob/main/Spring-DST-0-23.png)

![](https://github.com/earlchew/crontime/blob/main/Spring-DST-Wildcard.png)


#### Ending Daylight Savings

Typically during autumn, a daylight savings change will rewind the local
clock and cause the apparent time to repeat. For example, in the
US/Pacific timezone, the clock will instantaneously rewind from 02:00
to 01:00, as illustrated in the following table:

```
UTC    07  08  09  10  11  12
Local  00  01 -02-
              (01) 02  03  04

-02-  Hour that will not match any schedule
(01)  Hour that will only match a wildcard
```

To avoid the surprise of a duplicate event scheduled during the repeated hour
(eg `30 1 * * *`), the computed schedule will not match the specific
repeated hour (as indicated by `(01)`) during the daylight savings change.

To avoid the complementary surprise of a missing periodic event during
the repeated hour (eg `30 * * * *`), schedules using a wildcard `*` will
match the specific repeated hour (as indicated by `(01)`).

This accommodation for the daylight savings change only applies for
the transition period (ie `(01)`), and once past the scheduler
reverts to the standard behaviour (ie UTC `10`).

The following table shows some examples:

```
30 1   * * *  UTC 0830
30 1,2 * * *  UTC 0830, UTC 1030
30 2   * * *  UTC 1030
30 0-3 * * *  UTC 0730, UTC 0830, UTC 1030, UTC 1130
```

The following graphs show a visualisation of scheduling during this
daylight savings change. The blue lines show local time relative to
midnight of the morning of the change highlighting the jump from 0200
to 0100. The red lines show the behaviour for both an explicit hourly
schedule (ie `0-23`) and an implicit hourly schedule using a wildcard
(ie `*`).

With the explicit hourly schedule (ie `0-23`), just after the first
occurence of 0100, the next hourly event is delayed by an extra hour
because 0100 is repeated. This avoids having events at `0100` being
repeated over two consecutive hours.

With an implicit hourly schedule (ie `*`), after the first occurence
of 0100, the next hourly event occurs as expected with events at `0100`
being repeated over two consective hours.

![](https://github.com/earlchew/crontime/blob/main/Autumn-DST-0-23.png)

![](https://github.com/earlchew/crontime/blob/main/Autumn-DST-Wildcard.png)
