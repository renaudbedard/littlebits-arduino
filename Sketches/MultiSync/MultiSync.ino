//#define SERIAL_DEBUG

#define STATIC_ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define MAXIMUM_DRIFT_MS 5

#ifdef SERIAL_DEBUG
#include <Util\Util.h>
#define DEBUGGED_OUTPUT 2
ADD_PRINTF_SUPPORT
#endif
#include "Pins.h"

static const int Multipliers[6] = { 1, 2, 3, 4, 6, 8 };
int pulseLength;

struct SequencerState {
	unsigned long lastSync;
	bool state;
	int needsSync;
	byte pin;
	float multiplier;
};

unsigned long lastPulse;
bool wasHigh;

SequencerState sequencers[3];

void setup()
{
	lastPulse = millis();
	pulseLength = 10000;
	wasHigh = false;

	for (unsigned i=0; i<STATIC_ARRAY_SIZE(sequencers); i++)
	{
		sequencers[i].state = false;
		sequencers[i].needsSync = 0;
	}

	sequencers[0].multiplier = 1;
	sequencers[1].multiplier = 0.5f;
	sequencers[2].multiplier = 0.125f;

	sequencers[0].pin = Out::Digital::Sequencer1;
	sequencers[1].pin = Out::Digital::Sequencer2;
	sequencers[2].pin = Out::Digital::Sequencer3;

#ifdef SERIAL_DEBUG
	printf_setup();
	Serial.begin(115200);
#endif
}

void refreshMultiplier() 
{
	float rawMultiplier = analogRead(In::Analog::Multiplier) / 1024.0f;
#ifdef SERIAL_DEBUG
	float lastMult = sequencers[0].multiplier;
#endif
	sequencers[0].multiplier = Multipliers[round(rawMultiplier * ((sizeof(Multipliers) / sizeof(int)) - 1))];
#ifdef SERIAL_DEBUG
	if (lastMult != sequencers[0].multiplier)
		printf("\nNew multiplier : %i\n", (int) sequencers[0].multiplier);
#endif
}

void loop()
{
	unsigned long currentTime = millis();

	refreshMultiplier();

	// listen to beats from the SQ-1's Sync Out
	int listened = digitalRead(In::Digital::Sync);
	if (listened == HIGH)
	{
		if (!wasHigh) 
		{
			int lastPulseLength = pulseLength;
			pulseLength = currentTime - lastPulse;
			lastPulse = currentTime;

			if (abs(lastPulseLength - pulseLength) > MAXIMUM_DRIFT_MS)
			{
#ifdef SERIAL_DEBUG
				Serial.println("New BPM!");
#endif
				for (unsigned i=0; i<STATIC_ARRAY_SIZE(sequencers); i++)
					sequencers[i].needsSync = 0;
			}
			//{
			//	Serial.print("** DIFF = ");
			//	Serial.print(abs(lastPulseLength - pulseLength));
			//	Serial.println(" **");
			//}

			wasHigh = true;

			for (unsigned i=0; i<STATIC_ARRAY_SIZE(sequencers); i++)
			{
#ifdef SERIAL_DEBUG
				if (i == DEBUGGED_OUTPUT)
				{
					Serial.print(" ns=");
					Serial.print(sequencers[i].needsSync);
				}
#endif
				if (sequencers[i].needsSync++ == 0) 
				{
#ifdef SERIAL_DEBUG
					if (i == DEBUGGED_OUTPUT)
					{
						Serial.print(" ls=");
						Serial.print(currentTime - sequencers[i].lastSync);
					}
#endif
					if (currentTime - sequencers[i].lastSync > MAXIMUM_DRIFT_MS)
					{
						sequencers[i].lastSync = currentTime;

						if (sequencers[i].multiplier >= 0.5)	sequencers[i].needsSync = 0;
						else									sequencers[i].needsSync = (int) (-0.5f / sequencers[i].multiplier);

						sequencers[i].state = !sequencers[i].state;
						analogWrite(sequencers[i].pin, sequencers[i].state ? 255 : 0);
#ifdef SERIAL_DEBUG
						if (i == DEBUGGED_OUTPUT)
							Serial.println(sequencers[i].state ? " [O]!" : " [.]!");
#endif
					}
				}
			}
		}
	}
	else
		wasHigh = false;

	// variable multiplication pulses
	for (unsigned i=0; i<STATIC_ARRAY_SIZE(sequencers); i++)
	{
		if (currentTime >= sequencers[i].lastSync + pulseLength / (sequencers[i].multiplier * 2))
		{
			sequencers[i].lastSync = currentTime;

			if (sequencers[i].multiplier >= 0.5)	sequencers[i].needsSync = 0;
			else									sequencers[i].needsSync = (int) (-0.5f / sequencers[i].multiplier);

			sequencers[i].state = !sequencers[i].state;
			analogWrite(sequencers[i].pin, sequencers[i].state ? 255 : 0);

#ifdef SERIAL_DEBUG
			if (i == DEBUGGED_OUTPUT)
				Serial.println(sequencers[i].state ? " [O]" : " [.]");
#endif
		}
	}


}
