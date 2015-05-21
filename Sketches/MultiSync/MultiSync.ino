//#define SERIAL_DEBUG

#ifdef SERIAL_DEBUG
#include <Util\Util.h>
ADD_PRINTF_SUPPORT
#endif
#include "Pins.h"

static const int Multipliers[6] = { 1, 2, 3, 4, 6, 8 };
int multiplier;

unsigned long lastPulse, variablePulseAt;
int pulseLength;
bool variableState;
bool wasHigh;

void setup()
{
	variableState = false;
	lastPulse = millis();
	pulseLength = 1000;

	wasHigh = false;

#ifdef SERIAL_DEBUG
	printf_setup();
	Serial.begin(115200);
#endif
}

void variablePulse(unsigned long currentTime, bool onOff) 
{
	float rawMultiplier = analogRead(In::Analog::Multiplier) / 1024.0f;
#ifdef SERIAL_DEBUG
	int lastMult = multiplier;
#endif
	multiplier = Multipliers[round(rawMultiplier * 5)];
#ifdef SERIAL_DEBUG
	if (lastMult != multiplier)
		printf("\nNew multiplier : %i\n", multiplier);
#endif

	variablePulseAt = currentTime + (unsigned long) (pulseLength / (multiplier * 2));

	variableState = onOff;
	analogWrite(Out::Digital::VariableSpeed, onOff ? 255 : 0);
}

void loop()
{
	unsigned long currentTime = millis();

	// listen to beats from the SQ-1's Sync Out
	int listened = digitalRead(In::Digital::Sync);
	if (listened == HIGH)
	{
		if (!wasHigh) 
		{
			pulseLength = currentTime - lastPulse;
			lastPulse = currentTime;

			wasHigh = true;
			variablePulse(currentTime, true);

#ifdef SERIAL_DEBUG
			Serial.print(" T ");
#endif
		}
	}
	else
		wasHigh = false;

	// variable multiplication pulses
	if (currentTime >= variablePulseAt)
	{
		variablePulse(currentTime, !variableState);

#ifdef SERIAL_DEBUG
		Serial.print(variableState ? "V" : "v");
#endif
	}


}
