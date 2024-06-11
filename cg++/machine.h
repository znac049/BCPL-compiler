#pragma once

// OCODE Abstract machine
class machine {
    public:
        static const int MAX_GLOBALS = 400;
        static const int STATIC_DATA_SIZE = 16384;
        static const int STACK_SIZE = 32768;

        int globals[MAX_GLOBALS];
        int staticData[STATIC_DATA_SIZE];
        int stack[STACK_SIZE];
        int stackP;
        int stackS;

    public:
        machine();

};