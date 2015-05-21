#include "Pins.h"

int lastPulse;

bool halfState;
bool quarterState;
bool eighthState;

int scheduleHalf;
int scheduleQuarter;
int scheduleEighth;

#define DutyCycleOffset 20

enum DutyCycleType 
{
	Half,
	Full
};

void setup()
{
	lastPulse = 0;
	halfState = quarterState = eighthState = false;
	scheduleHalf = scheduleQuarter = scheduleEighth = 0;

	//Serial.begin(115200);
}

void loop()
{
	// schedules (for full duty cycle mode)
	if (scheduleHalf > 0 && --scheduleHalf == 0) 
		digitalWrite(Out::Digital::HalfSpeed, true);
	if (scheduleQuarter > 0 && --scheduleQuarter == 0) 
		analogWrite(Out::Digital::QuarterSpeed, 255); 
	if (scheduleEighth > 0 && --scheduleEighth == 0) 
		analogWrite(Out::Digital::EighthSpeed, 255); 

	int pulse = digitalRead(In::Digital::Pulse);
	DutyCycleType cycleType = analogRead(In::Analog::DutyCycle) > 127 ? Full : Half;

	if (lastPulse == 0 && pulse != 0)
	{
		// rising
		halfState = !halfState;
		if (cycleType == Half)
			digitalWrite(Out::Digital::HalfSpeed, halfState);

		if (halfState)
		{
			quarterState = !quarterState;

			if (cycleType == Half)
				analogWrite(Out::Digital::QuarterSpeed, quarterState ? 255 : 0);
			else
			{
				digitalWrite(Out::Digital::HalfSpeed, false);
				scheduleHalf = DutyCycleOffset;
				//Serial.println("H-");
			}

			if (quarterState) 
			{
				eighthState = !eighthState;

				if (cycleType == Half)
					analogWrite(Out::Digital::EighthSpeed, eighthState ? 255 : 0);
				else
				{
					analogWrite(Out::Digital::QuarterSpeed, 0);
					scheduleQuarter = DutyCycleOffset;
					//Serial.println("Q-");

					if (eighthState)
					{
						analogWrite(Out::Digital::EighthSpeed, 0);
						scheduleEighth = DutyCycleOffset;
						//Serial.println("E-");
					}
				}
			}
		}
	}
	else if (lastPulse != 0 && pulse == 0)
	{
		// descending
	}
	lastPulse = pulse;
}
