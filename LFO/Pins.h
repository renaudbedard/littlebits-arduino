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
		static const byte Speed = 0;
    };
};

class Out
{
    public:
    class Digital
    {
    public:
    };
    class Analog
    {
    public:
		static const byte Lfo = 5;		
    };  
};

#endif
