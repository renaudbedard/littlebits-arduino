#include "Pins.h"

int lastPulse;
unsigned long pulseStart;
unsigned long lastPulseLength;
bool oscState;

void setup()
{
	lastPulse = 0;
}

void loop()
{
	unsigned long time = millis();

	int pulse = digitalRead(In::Digital::Pulse);
	int dutyCycle = max(analogRead(In::Analog::DutyCycle), 1);

	if (lastPulse == 0 && pulse != 0)
	{
		// rising
		pulseStart = time;
	}
	else if (lastPulse != 0 && pulse == 0)
	{
		// descending
		lastPulseLength = (time - pulseStart) * 2;
	}
	lastPulse = pulse;

	unsigned long dutyCycleTime = (unsigned long) (lastPulseLength * dutyCycle / 1024.0f);
	if (!oscState && time - pulseStart <= dutyCycleTime)
	{
		oscState = true;
		digitalWrite(Out::Digital::Oscillator, oscState);
	}
	else if (oscState && time - pulseStart > dutyCycleTime)
	{
		oscState = false;
		digitalWrite(Out::Digital::Oscillator, oscState);
	}
}
