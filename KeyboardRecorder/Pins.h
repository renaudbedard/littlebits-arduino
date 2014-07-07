#ifndef PINS_H
#define PINS_H

class In
{
    public:
    class Digital
    {
    public:
        static const int PlayPause = 0;
    };
    class Analog
    {
    public:
        static const int Keyboard = 0;
        static const int ModeSwitch = 1;
    };
};

class Out
{
    public:
    class Digital
    {
    public:
        static const int Unused = 1;
    };
    class Analog
    {
    public:
        static const int Oscillator = 5;
        static const int Unused = 9;        
    };  
};

#endif
