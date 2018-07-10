#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <stdarg.h>

namespace Logger
{
    int Info(const char* fmt, ...);
    int Info(const char* fmt, va_list varg);

    int Error(const char* fmt, ...);
    int Error(const char* fmt, va_list varg);
}

#endif /* */