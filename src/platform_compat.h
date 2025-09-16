#include <stdint.h>

#ifdef _WIN32

#include <windows.h>
#include <basetsd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int gettimeofday(struct timeval *tp, struct timezone *tzp)
{
    FILETIME ft;
    unsigned __int64 tmpres = 0;
    static int tzflag = 0;

    if (NULL != tp)
    {
        GetSystemTimeAsFileTime(&ft);

        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        // Convert into microseconds
        tmpres /= 10;

        // Convert into seconds and microseconds
        tmpres -= 11644473600000000ULL;

        tp->tv_sec = (long)(tmpres / 1000000UL);
        tp->tv_usec = (long)(tmpres % 1000000UL);
    }
    return 0;
}

#else

#include <sys/time.h>
#include <err.h>
#include <cstddef>

#endif

inline uint32_t currentMs()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return uint32_t((time.tv_sec * 1000) + (time.tv_usec / 1000));
}
