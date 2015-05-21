#include "Arduino.h"

#ifndef PINS_H
#define PINS_H

class In
{
    public:
    class Digital
    {
    public:
    };
    class Analog
    {
    public:
		static const byte Sync = 0;
		static const byte DutyCycle = 1;
    };
};

class Out
{
    public:
    class Digital
    {
    public:
		static const byte DoubleSpeed = 1;
		static const byte QuadSpeed = 5;
		static const byte OctoSpeed = 9;
    };
    class Analog
    {
    public:
		
    };  
};

#endif
