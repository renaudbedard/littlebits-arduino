#include "Pins.h"

int lastPulse;

bool halfState;
bool quarterState;
bool eighthState;

enum DutyCycleType 
{
	Half,
	Full
};

void setup()
{
	lastPulse = 0;
	halfState = quarterState = eighthState = false;
}

void loop()
{
	int pulse = digitalRead(In::Digital::Pulse);
	//DutyCycleType cycleType = digitalRead(In::Analog::DutyCycle) ? Full : Half;

	if (lastPulse == 0 && pulse != 0)
	{
		// rising
		halfState = !halfState;
		digitalWrite(Out::Digital::HalfSpeed, halfState);

		if (halfState)
		{
			quarterState = !quarterState;
			analogWrite(Out::Digital::QuarterSpeed, quarterState ? 255 : 0);

			if (quarterState) 
			{
				eighthState = !eighthState;
				analogWrite(Out::Digital::EighthSpeed, eighthState ? 255 : 0);
			}
		}
	}
	else if (lastPulse != 0 && pulse == 0)
	{
		// descending
	}
	lastPulse = pulse;
}
