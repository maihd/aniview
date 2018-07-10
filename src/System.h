#ifndef __SYSTEM_H__
#define __SYSTEM_H__

namespace System
{
    void Exit(int value);

    int  Print(const char* fmt, ...);
    void Error(const char* fmt, ...);
}

#endif /* __SYSTEM_H__ */