#include "dostime.h"

int getDOSDate(time_t ts) {
    struct tm *res;

    res = localtime(&ts);

    return (res->tm_year - 80) * 512 + res->tm_mon * 32 + res->tm_mday;
}

int getDOSTime(time_t ts) {
    struct tm *res;

    res = localtime(&ts);

    return res->tm_hour * 2048 + res->tm_min * 32 + res->tm_sec / 2;
}
