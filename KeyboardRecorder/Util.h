#include "Arduino.h"

#ifndef UTIL_H
#define UTIL_H

#define ANALOG_HIGH 511

bool boolAnalogRead(byte pin);
float floatAnalogRead(byte pin);
void floatAnalogWrite(byte pin, float value);

#endif
