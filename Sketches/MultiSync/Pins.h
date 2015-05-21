#include "Arduino.h"

#ifndef PINS_H
#define PINS_H

class In
{
    public:
    class Digital
    {
    public:
		static const byte Sync = 0;
    };
    class Analog
    {
    public:
		static const byte Multiplier = 0;
    };
};

class Out
{
    public:
    class Digital
    {
    public:
		static const byte VariableSpeed = 1;
    };
    class Analog
    {
    public:
    };  
};

#endif
