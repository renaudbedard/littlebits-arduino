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
        static const byte Keyboard = 0;
        static const byte ModeSwitch = 1;
    };
};

class Out
{
    public:
    class Digital
    {
    public:
        static const byte Unused = 1;
    };
    class Analog
    {
    public:
        static const byte Oscillator = 5;
        static const byte Unused = 9;        
    };  
};

#endif
