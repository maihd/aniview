#include "system.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

namespace System
{
    void Exit(int value)
    {
        ::exit(value);
    }

    int Print(const char* fmt, ...)
    {
        va_list varg;
        va_start(varg, fmt);
        int res = vprintf(fmt, varg);
        va_end(varg);

        return res;
    }

    void Error(const char* fmt, ...)
    {
        char tmp[1024];
        sprintf(tmp, "[Error] %s\n", fmt);

        va_list varg;
        va_start(varg, fmt);
        int res = vfprintf(stderr, tmp, varg);
        va_end(varg);

        getchar();
        System::Exit(1);
    }
}