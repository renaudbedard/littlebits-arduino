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
int recordedNotes = 5;
Mode mode = None;
PlayMode playMode = Stopped;
bool buttonLastPressed;

Coroutine* playCoroutine = NULL;
Coroutines<1> coroutines;

void setup() 
{
    Serial.begin(9600);
    analogWrite(Out::Analog::Oscillator, 0);
}

//int noteIndex;
bool play(Coroutine& coroutine)	
{
	BEGIN_COROUTINE;

	Serial.println("Starting to play!");

	// linear
	Serial.println("Playing note 0");
	analogWrite(Out::Analog::Oscillator, notes[0]);

	coroutine.wait(1000);
	COROUTINE_YIELD;

	Serial.println("Playing note 1");
	analogWrite(Out::Analog::Oscillator, notes[1]);

	coroutine.wait(1500);
	COROUTINE_YIELD;

	Serial.println("Playing note 2");
	analogWrite(Out::Analog::Oscillator, notes[2]);

	coroutine.wait(500);
	COROUTINE_YIELD;

	// .data-alloc
/*
	for (noteIndex=0; noteIndex<recordedNotes; noteIndex++)
	{
		Serial.print("Playing note");
		Serial.println(noteIndex);
		analogWrite(Out::Analog::Oscillator, notes[noteIndex]);

		coroutine.wait(500);
		COROUTINE_YIELD;
	}
*/

	playMode = Stopped;
	Serial.println("All done!");

	END_COROUTINE;
}

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
		bool buttonNewlyPressed = !buttonLastPressed && buttonPressed;
		buttonLastPressed = buttonPressed;

		if (buttonNewlyPressed)
		{
			PlayMode lastPlayMode = playMode;
			switch (lastPlayMode)
			{
			case Stopped:
				if (recordedNotes == 0)
					Serial.println("Nothing to play!");
				else
				{
					playMode = Playing;
					playCoroutine = &coroutines.add(play);
				}
				break;

			case Playing:
				playMode = Paused;
				if (!playCoroutine->terminated)
				{
					Serial.println("Paused");
					playCoroutine->suspend();
				}
				break;

			case Paused:
				playMode = Playing;
				if (!playCoroutine->terminated)
				{
					Serial.println("Unpaused");
					playCoroutine->resume();
				}
				break;
			}
		}
	}
	else // if (mode == Record)
	{
	}
}

