#include <stdint.h>

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
		static const byte Sequencer1 = 1;
		static const byte Sequencer2 = 5;
		static const byte Sequencer3 = 9;
    };
    class Analog
    {
    public:
    };  
};

#endif
