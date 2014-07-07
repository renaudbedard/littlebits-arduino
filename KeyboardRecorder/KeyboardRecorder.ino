#include "Pins.h"
#include "Util.h"
#include "Coroutines.h"

enum Mode {
	None,
    Playback,
    Record
};

enum PlayMode {
	Stopped,
	Playing,
	Paused
};

const int MaxRecordedNotes = 32;

int notes[MaxRecordedNotes];
int recordedNotes = 0;
Mode mode = None;
PlayMode playMode = Stopped;

Coroutine* playCoroutine = NULL;
Coroutines<1> coroutines;

void setup() 
{
    Serial.begin(9600);
    analogWrite(Out::Analog::Oscillator, 0);
}

BEGIN_COROUTINE(play);
{
	Serial.println("Starting to play!");

	for (int i=0; i<recordedNotes; i++)
	{
		analogWrite(Out::Analog::Oscillator, notes[i]);

		coroutine.wait(1000);
		COROUTINE_YIELD;
	}
	
	Serial.println("All done!");
}
END_COROUTINE;

void loop() 
{
	unsigned long time = millis();
	coroutines.update(time);

	Mode lastMode = mode;
	mode = digitalAnalogRead(In::Analog::ModeSwitch) ? Playback : Record;

	if (mode != lastMode)
	{
		Serial.println(mode == Playback ? "Playback mode" : "Record mode");
		lastMode = mode;
	}

	if (mode == Playback)
	{
		bool buttonPressed = digitalRead(In::Digital::PlayPause) == HIGH;

		PlayMode lastPlayMode = playMode;
		switch (lastPlayMode)
		{
		case Stopped:
			if (buttonPressed)
			{
				if (recordedNotes == 0)
					Serial.println("Nothing to play!");
				else
				{
					playMode = Playing;
					playCoroutine = &coroutines.add(play);
				}
			}
			break;

		case Playing:
			if (buttonPressed)
			{
				playMode = Paused;
				if (!playCoroutine->terminated)
					playCoroutine->suspend();
			}
			break;

		case Paused:
			if (buttonPressed)
			{
				playMode = Playing;
				if (!playCoroutine->terminated)
					playCoroutine->resume();
			}
			break;
		}
	}
	else // if (mode == Record)
	{
	}
}

