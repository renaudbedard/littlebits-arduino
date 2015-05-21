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
        static const byte HalfSpeed = 1;
		static const byte QuarterSpeed = 5;
		static const byte EighthSpeed = 9;
    };
    class Analog
    {
    public:
		
    };  
};

#endif
