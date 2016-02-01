#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include "log_printf.h"

int g_level = LOG_VERBOSE;

void print_datetime(int level)
{
    FILE *output = stdout;
    struct timeval tv;
    struct tm *p;
    char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    if (level == LOG_ERROR)
    {
        output = stderr;
    }

    gettimeofday(&tv, NULL);
    p = localtime((time_t*)&tv.tv_sec);
    fprintf(output, "[%d/%02d/%02d %02d:%02d:%02d.%06ld %s]",
            (1900+p->tm_year), (1+p->tm_mon), p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec,
            tv.tv_usec, wday[p->tm_wday]);
}
void log_printf(int level, const char *fmt, ...)
{
    if (level < g_level)
    {
        return;
    }

    va_list args;
    print_datetime(level);
//    PRINT_FILE_INFO(level);  //modify by yanly
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    fflush(stdout);
    fflush(stderr);
}

/*for test*/
//int main(int argc, char *argv[])
//{
//    if (argc == 2)
//    {
//        g_level = atoi(argv[1]);
//        printf("g_level: %d\n", g_level);
//    }
//
//    log_printf(LOG_VERBOSE, "Hello, world! %d\n", 2016);
//    log_printf(LOG_NOTICE, "Hi~\n");
//    log_printf(LOG_WARNING, "Hey~\n");
//    return 0;
//}

