//#define SERIAL_DEBUG

#define SEQUENCER_COUNT 3
#define MULTIPLIER_COUNT 6
#define QUEUE_SIZE 18

#define MAXIMUM_DRIFT_MS 5

#include <Util\Util.h>

#ifdef SERIAL_DEBUG
ADD_PRINTF_SUPPORT
#define DEBUGGED_OUTPUT 0
#endif

#include "Pins.h"

static const int Multipliers[MULTIPLIER_COUNT] = { 1, 2, 3, 4, 6, 8 };

enum PinState 
{
	On,
	Off,
	ScheduledOff
};
struct SequencerState 
{
	byte queueLength;
	ulong queue[QUEUE_SIZE];
	ulong scheduledOffTime;
	PinState state;
	byte pin;
	float multiplier;
};
SequencerState sequencers[SEQUENCER_COUNT];

ulong lastPulse;
int pulseLength;
bool wasHigh;
bool firstDiscarded;

#define DutyCycleOffset 20

enum DutyCycleType 
{
	Half,
	Full
};
DutyCycleType lastDutyCycleType;

#ifdef SERIAL_DEBUG
bool ready = false;
#endif

void setup()
{
	lastPulse = millis() - 1;
	pulseLength = 0;
	wasHigh = false;

	lastDutyCycleType = Half;

	memset(sequencers, 0, sizeof(SequencerState) * SEQUENCER_COUNT);

	sequencers[0].multiplier = 1;
	sequencers[1].multiplier = 0.5f;
	sequencers[2].multiplier = 0.25f;

	sequencers[0].pin = Out::Digital::Sequencer1;
	sequencers[1].pin = Out::Digital::Sequencer2;
	sequencers[2].pin = Out::Digital::Sequencer3;

	// pulse sequencer 3x to prepare for first beat
	for (byte j = 0; j < 3; ++j)
	{
		for (byte i = 0; i < SEQUENCER_COUNT; ++i)
			analogWrite(sequencers[i].pin, 255);
		delay(15);
		for (byte i = 0; i < SEQUENCER_COUNT; ++i)
			analogWrite(sequencers[i].pin, 0);
		delay(15);
	}

#ifdef SERIAL_DEBUG
	printf_setup();
	Serial.begin(115200);
#endif
}

/*
//int highest;
ulong wentOff;
ulong wentOn;

void test() 
{
	ulong currentTime = millis();
	int listened = digitalRead(In::Digital::Sync);
	if (listened == HIGH)
	{
		if (!wasHigh) 
		{
			wentOn = currentTime;

			int lastPulseLength = pulseLength;
			pulseLength = currentTime - lastPulse;
			lastPulse = currentTime;

			wasHigh = true;

			Serial.print(" ");
			Serial.print(currentTime - wentOff);
			Serial.print("ms [");
		}

		//highest = max(highest, listened);
	}
	else
	{
		if (wasHigh)
		{
			wentOff = currentTime;

			//Serial.print(highest);
			//Serial.print(" (");
			Serial.print(currentTime - wentOn);
			Serial.print("ms]");
			//highest = 0;
			wasHigh = false;
		}
	}
}
*/

void clearQueues() 
{
	// clear queues
	for (byte i = 0; i < SEQUENCER_COUNT; ++i)
	{
		memset(sequencers[i].queue, 0, sizeof(ulong) * QUEUE_SIZE);
		sequencers[i].queueLength = 0;
	}
}

void loop()
{
#ifdef SERIAL_DEBUG
	if (!ready) 
	{
		Serial.println("gimme a sec here");
		delay(500);
		Serial.println("ready to go");
		ready = true;
	}
#endif

	ulong currentTime = millis();

	// query duty cycle
	DutyCycleType cycleType = analogRead(In::Analog::DutyCycle) > 127 ? Full : Half;
	if (lastDutyCycleType != cycleType)
	{
#ifdef SERIAL_DEBUG
		printf("<< new duty cycle type : %s >>\n", cycleType == Full ? "full" : "half");
#endif
		lastDutyCycleType = cycleType;
	}

	// refresh multiplier
	SequencerState& variableSequencer = sequencers[0];
	float rawMultiplier = analogRead(In::Analog::Multiplier) / 1024.0f;
	float lastMult = variableSequencer.multiplier;
	variableSequencer.multiplier = Multipliers[round(rawMultiplier * (MULTIPLIER_COUNT - 1))];
	if (lastMult != variableSequencer.multiplier)
	{
		// clear queue for variable sequencer
		memset(variableSequencer.queue, 0, sizeof(ulong) * QUEUE_SIZE);
		variableSequencer.queueLength = 0;

#ifdef SERIAL_DEBUG
		printf("<< new multiplier : %i >>\n", (int) variableSequencer.multiplier);
#endif
	}

	// listen to beats from the SQ-1's Sync Out
	int listened = digitalRead(In::Digital::Sync);
	if (listened == HIGH)
	{
		if (!wasHigh) 
		{
			int lastPulseLength = pulseLength;
			int curPulseLength = currentTime - lastPulse;
			lastPulse = currentTime;

			if (abs(lastPulseLength - curPulseLength) > MAXIMUM_DRIFT_MS)
			{
				if (firstDiscarded)
				{
					// BPM change deteted
					pulseLength = curPulseLength;
#ifdef SERIAL_DEBUG
					printf("<< %i bpm >>\n", (int)(60000 / pulseLength));
#endif
					clearQueues();
				}
				else
					firstDiscarded = true;
			}

			wasHigh = true;

			// early out for invalid BPM
			if (pulseLength == 0)
				return;

			for (byte i = 0; i < SEQUENCER_COUNT; ++i)
			{
				SequencerState& sequencer = sequencers[i];
				byte headOffset = 1;
				float dividedLength = (float)pulseLength / (sequencer.multiplier * 2.0f);
#ifdef SERIAL_DEBUG
				bool noop = sequencer.queueLength == QUEUE_SIZE;
#endif

				if (sequencer.queueLength != QUEUE_SIZE && sequencer.queueLength > 0)
				{
					int distFromHead = abs(sequencer.queue[0] - currentTime);

					// should we latch that beat off?
					float ratio = distFromHead / dividedLength;
					if (ratio > 0.25 && ratio < 0.75)
						continue;

					// are we close to a trigger?
					if (distFromHead <= max((int)(0.1f * dividedLength), MAXIMUM_DRIFT_MS * 4))
						headOffset = 0;

#ifdef SERIAL_DEBUG
					//if (i == DEBUGGED_OUTPUT)
					//	Serial.println(distFromHead);
#endif
				}

				// special case for empty queue
				if (sequencer.queueLength == 0)
				{
					sequencer.queue[0] = currentTime;
					sequencer.queueLength = 1;
					headOffset = 0;
				}

				// fill queue
				for (byte j = sequencer.queueLength; j < QUEUE_SIZE; ++j)
					sequencer.queue[j] = currentTime + (ulong)((float)(j + headOffset) * dividedLength);
				sequencer.queueLength = QUEUE_SIZE;

#ifdef SERIAL_DEBUG
				//if (!noop && i == DEBUGGED_OUTPUT)
				//{
				//	Serial.print("{ ");
				//	for (byte j = 1; j < QUEUE_SIZE; ++j)
				//		printf("%lu ", sequencer.queue[j] - sequencer.queue[j - 1]);
				//	Serial.println("}");
				//}
#endif
			}
		}
	}
	else
		wasHigh = false;

	for (byte i = 0; i < SEQUENCER_COUNT; ++i)
	{
		SequencerState& sequencer = sequencers[i];
		if (sequencer.state == ScheduledOff)
		{
			if (currentTime >= sequencer.scheduledOffTime)
			{
				sequencer.state = Off;
				analogWrite(sequencer.pin, 0);

#ifdef SERIAL_DEBUG
				if (i == DEBUGGED_OUTPUT)
					printf(". ql=%i\n", sequencer.queueLength);
#endif
			}
		}
		else if (sequencer.queueLength > 0 && currentTime >= sequencer.queue[0])
		{
			// state logic
			if (sequencer.state == On)
			{
				if (cycleType == Full)
				{
					sequencer.scheduledOffTime = sequencer.queue[1] - DutyCycleOffset;
					sequencer.state = ScheduledOff;
				}
				else
				{
					sequencer.state = Off;
					analogWrite(sequencer.pin, 0);
				}
			}
			else if (sequencer.state == Off)
			{
				sequencer.state = On;
				analogWrite(sequencer.pin, 255);
			}

			// move queue back
			sequencer.queueLength--;
			memmove(sequencer.queue, &sequencer.queue[1], sizeof(ulong) * sequencer.queueLength);

#ifdef SERIAL_DEBUG
			if (i == DEBUGGED_OUTPUT)
				printf("%s ql=%i\n", sequencer.state == On ? "O" : sequencer.state == Off ? "." : "()", sequencer.queueLength);
#endif
		}
	}
}
