#include <MIDI\MIDI.h>
#include "Pins.h"

MIDI_CREATE_DEFAULT_INSTANCE();

namespace Bleep 
{
	enum Enum
	{
		MidiStep = 55,

		RedTick = 60,
		BlueTom = 62,
		GreenClap = 64,
		YellowKick = 65,

		TogglePlay = 67,

		ReverseToggle = 69,
		NoiseToggle = 70,

		BlueSequence = 71,
	};
}
//C4 – 70 – red tick
//D4 – 71 – blue tom
//E4 – 72 – clap green
//F4 – 73 – yellow kick
//
//(Some DAWs, like FL Studio, seem to be transposing the output up and octave. If your device isn’t responding to these keys, try C5, D5, etc.)
//
//G4 – toggle play
//
//A4 – Reverse toggle
//Bb4 – Noise toggle
//In noise the four ccs change different variables.
//
//C5 – Select blue sequence
//D5 – Select yellow sequence
//E5 – Select red sequence
//F5 – Select green sequence
//
//G3 – MIDI step (This is used to advance the sequence one 32 note. While midi clock is not supported, this can be used to sync with the rate of another device)

int lastPulse;
unsigned long pulseStart;
unsigned long lastPulseLength;

void setup()
{
	MIDI.begin();
}

void loop()
{
	unsigned long time = millis();

	int pulse = digitalRead(In::Digital::Pulse);

	if (lastPulse == 0 && pulse != 0)
	{
		// rising
		pulseStart = time;

		MIDI.sendNoteOn(Bleep::RedTick, 127, 1);
	}
	else if (lastPulse != 0 && pulse == 0)
	{
		// descending
		lastPulseLength = (time - pulseStart) * 2;
	}
	lastPulse = pulse;
}
