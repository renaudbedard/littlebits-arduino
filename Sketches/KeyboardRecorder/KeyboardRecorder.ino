#include <Util.h>
#include <Coroutines.h>
#include <EEPROM\EEPROM.h>
#include "Pins.h"

enum Mode {
	None,
	Playback,
	Record
};

const byte MaxRecordedNotes = 32;
int notes[MaxRecordedNotes];
byte recordedNotes;

Mode mode = None;

int keyLastPressed;
unsigned long keyLastPressedAt = 0;
bool keyReleased;
bool lastPulse;
bool needsReset;

Coroutine* previewCoroutine = NULL;
Coroutine* playCoroutine = NULL;
Coroutines<3> coroutines;

#if _DEBUG
ADD_PRINTF_SUPPORT;
#endif

void setup()
{
#if _DEBUG
	printf_setup();
#endif
	Serial.begin(115200);
	analogWrite(Out::Analog::Oscillator, 0);
}

// this avoids the (false postive) warning for coroutine locals
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

// plays a short bleep when the note buffer is cleared by a long press
void notifyClear(Coroutine& coroutine)
{
	BEGIN_COROUTINE;

	analogWrite(Out::Analog::Oscillator, keyLastPressed);
	coroutine.wait(100);
	COROUTINE_YIELD;

	analogWrite(Out::Analog::Oscillator, 0);

	END_COROUTINE;
}

// previews the last played note
void preview(Coroutine& coroutine)
{
	BEGIN_COROUTINE;

	if (needsReset) 
	{
		// buffer with silence to reset envelopes
		needsReset = false;
		coroutine.wait(50);
		COROUTINE_YIELD;
	}

	analogWrite(Out::Analog::Oscillator, notes[recordedNotes - 1]);

	coroutine.wait(500);
	COROUTINE_YIELD;

	// only clear the "current" preview coroutine on normal exit, not external termination
	previewCoroutine = NULL;

	COROUTINE_FINALLY
	{			
		// ensure that the oscillator plays nothing as the coroutine exits OR gets terminated
		analogWrite(Out::Analog::Oscillator, 0);
	}

	END_COROUTINE;
}

void play(Coroutine& coroutine)
{
	// used for local iteration, saved & recovered when yielding
	COROUTINE_LOCAL(byte, i);

	BEGIN_COROUTINE;

	for (i = 0; i < recordedNotes; i++)
	{
		// alternate between plays and rests
		trace(P("Playing note %hhu"), i);
		analogWrite(Out::Analog::Oscillator, notes[i]);

		coroutine.suspend();
		COROUTINE_YIELD;

		trace(P("Resting"));
		analogWrite(Out::Analog::Oscillator, 0);

		coroutine.suspend();
		COROUTINE_YIELD;
	}

	// makes the coroutine loop instead of exiting
	coroutine.loop();

	COROUTINE_FINALLY;
	{
		// since this coroutine loops, this only gets called on termination
		analogWrite(Out::Analog::Oscillator, 0);
	}

	END_COROUTINE;
}

#pragma GCC diagnostic pop

void loop() 
{
	unsigned long time = millis();
	coroutines.update(time);

	Mode lastMode = mode;
	mode = boolAnalogRead(In::Analog::ModeSwitch) ? Playback : Record;

	// mode toggle
	if (mode != lastMode)
	{
		trace(mode == Playback ? P("\n** Playback mode **\n") : P("\n** Record mode **\n"));
		lastMode = mode;

		if (mode == Record && playCoroutine != NULL && !playCoroutine->terminated)
		{
			// terminate playback coroutine when switching to recording mode
			playCoroutine->terminate();
			playCoroutine = NULL;
		}

		if (mode == Playback)
		{
			// terminate preview coroutine (if any active) when switching to playback
			if (previewCoroutine != NULL && !previewCoroutine->terminated)
			{
				previewCoroutine->terminate();
				previewCoroutine = NULL;
			}

			// start playback coroutine
			playCoroutine = &coroutines.start(play);
		}
	}

	// mode logic
	if (mode == Record)
	{
		int keyboardValue = medianAnalogRead(In::Analog::Keyboard);

		if (keyboardValue > 0)
		{
			if (keyLastPressed == 0)
				keyLastPressedAt = time;

			if (time - keyLastPressedAt > 1000 && recordedNotes > 0)
			{
				trace(P("Clearing recorded notes, held %lu ms"), time - keyLastPressedAt);

				coroutines.start(notifyClear);
				recordedNotes = 0;
				keyLastPressedAt = time;
			}

			if (keyReleased)
			{
				if (recordedNotes == MaxRecordedNotes)
					recordedNotes = 0;

				trace(P("Recorded note %hhu : %i"), recordedNotes, keyboardValue);
				notes[recordedNotes++] = keyboardValue;

				if (previewCoroutine != NULL && !previewCoroutine->terminated)
				{
					needsReset = true;
					trace(P("Interrupted preview (coroutine #%hhu)"), previewCoroutine->id);
					previewCoroutine->terminate();
				}
				previewCoroutine = &coroutines.start(preview);

				keyReleased = false;
			}

			keyLastPressed = keyboardValue;
		}
		else
		{
			keyReleased = true;
			keyLastPressed = 0;
		}
	}
	else // if (mode == Playback)
	{
		// for each value change, wake up the playback coroutine
		bool thisPulse = digitalRead(In::Digital::Pulse) == HIGH;
		if ((thisPulse != lastPulse) && playCoroutine->suspended)
			playCoroutine->resume();
		lastPulse = thisPulse;
	}
}

