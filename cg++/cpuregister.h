#pragma once

class cpuRegister
{
    public:
        cpuRegister();
        void set(int);
        void get();
        void invalid();
        int isInvalid();
        void setName(const char *);
        const char *getName();
}