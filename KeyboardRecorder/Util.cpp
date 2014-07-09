#include "Util.h"
#include "Arduino.h"

bool boolAnalogRead(byte pin)
{
	return analogRead(pin) > ANALOG_HIGH;
}

float floatAnalogRead(byte pin)
{
	return analogRead(pin) / 1023.0f;
}

void floatAnalogWrite(byte pin, float value)
{
	analogWrite(pin, (int) round(value * 255));
}
