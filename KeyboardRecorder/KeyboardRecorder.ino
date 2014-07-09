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

const byte MaxRecordedNotes = 32;

int notes[MaxRecordedNotes];
byte recordedNotes = 5;
Mode mode = None;
PlayMode playMode = Stopped;
bool buttonLastPressed;

Coroutine* playCoroutine = NULL;
Coroutines<1> coroutines;

void setup() 
{
    Serial.begin(115200);
    analogWrite(Out::Analog::Oscillator, 0);
}

// this avoids the (false postive) warning for coroutine locals
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

bool play(Coroutine& coroutine)
{
	COROUTINE_LOCAL(byte, i);
	BEGIN_COROUTINE;

	for (i=0; i<recordedNotes; i++)
	{
		Serial.print(F("Playing note "));
		Serial.println(i);
		analogWrite(Out::Analog::Oscillator, notes[i]);

		coroutine.wait(500);
		COROUTINE_YIELD;
	}

	playMode = Stopped;
	Serial.println(F("All done!"));

	END_COROUTINE;
	return true;
}

#pragma GCC diagnostic pop

void loop() 
{
	unsigned long time = millis();
	coroutines.update(time);

	Mode lastMode = mode;
	mode = digitalAnalogRead(In::Analog::ModeSwitch) ? Playback : Record;

	if (mode != lastMode)
	{
		Serial.println(mode == Playback ? F("Playback mode") : F("Record mode"));
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
					Serial.println(F("Nothing to play!"));
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
					Serial.println(F("Paused"));
					playCoroutine->suspend();
				}
				break;

			case Paused:
				playMode = Playing;
				if (!playCoroutine->terminated)
				{
					Serial.println(F("Unpaused"));
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

