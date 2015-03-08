#include "Arduino.h"

#ifndef PINS_H
#define PINS_H

class In
{
    public:
    class Digital
    {
    public:
        static const byte Pulse = 0;
    };
    class Analog
    {
    public:
        static const byte DutyCycle = 0;
    };
};

class Out
{
    public:
    class Digital
    {
    public:
        static const byte Oscillator = 1;
    };
    class Analog
    {
    public:
    };  
};

#endif
