#include "../../tz/localtime.c"

#include "localtime.h"

void
ttuntil(const time_t *time, struct transition range[2])
{
    time_t t = *time;
    struct state *sp = lclptr;

    if (sp->timecnt == 0 || t < sp->ats[0]) {

        range[0].at = 0;
        range[0].off = 0;
        range[1].at = sp->ats[0];
        range[1].off = 0;

    } else {

        int lo = 1;
        int hi = sp->timecnt;

        while (lo < hi) {
            int mid = (lo + hi) / 2;

            if (t < sp->ats[mid])
                hi = mid;
            else
                lo = mid + 1;
        }

        range[0].at = sp->ats[lo-1 ? lo-2 : lo-1];
        range[0].off = sp->ttis[sp->types[lo-1 ? lo-2 : lo-1]].tt_utoff;
        range[1].at = sp->ats[lo-1];
        range[1].off = sp->ttis[sp->types[lo-1]].tt_utoff;
        range[2].at = sp->ats[lo];
        range[2].off = sp->ttis[sp->types[lo]].tt_utoff;

    }
}
