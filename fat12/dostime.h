#include <time.h>

#ifndef _DOSTIME_H_
#define _DOSTIME_H_

#define getTS() time(NULL)
int getDOSTime(time_t);
int getDOSDate(time_t);

#endif
