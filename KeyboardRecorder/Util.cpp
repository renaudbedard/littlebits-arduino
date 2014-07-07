#include "Util.h"
#include "Arduino.h"

bool digitalAnalogRead(int pin)
{
	return analogRead(pin) > ANALOG_HIGH;
}