/*
 *	File name   : log_printf.h
 *  Created on  : Jan 12, 2016
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#ifndef INCLUDE_LOG_PRINTF_H_
#define INCLUDE_LOG_PRINTF_H_

#define LOG_VERBOSE     0
#define LOG_NOTICE      1
#define LOG_WARNING     2
#define LOG_ERROR       3

#define PRINT_FILE_INFO(level)\
    do {\
        if (LOG_VERBOSE == level)\
            fprintf(stdout, "[VER] %s:%s:%d: ", __FILE__, __FUNCTION__, __LINE__);\
        else if (LOG_NOTICE == level)\
            fprintf(stdout, "[NOT] %s:%s:%d: ", __FILE__, __FUNCTION__, __LINE__);\
        else if (LOG_WARNING == level)\
            fprintf(stdout, "[WAR] %s:%s:%d: ", __FILE__, __FUNCTION__, __LINE__);\
        else if (LOG_ERROR == level)\
            fprintf(stderr, "[ERR] %s:%s:%d: ", __FILE__, __FUNCTION__, __LINE__);\
    } while(0)


void log_printf(int level, const char *fmt, ...);

#endif /* INCLUDE_LOG_PRINTF_H_ */
