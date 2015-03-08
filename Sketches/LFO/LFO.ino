#include "Pins.h"

unsigned long lastTime;
double timeElapsed;

void setup()
{
	lastTime = millis();
	timeElapsed = 0;
}

void loop()
{
	unsigned long time = millis();

	double period = (analogRead(In::Analog::Speed) + 1) / 1024.0;

	timeElapsed += (time - lastTime) * period / 60.0;

	double lfo = sin(timeElapsed);

	int out = (int)((lfo * 0.5 + 0.5) * 242.0) + 14;
	analogWrite(Out::Analog::Lfo, out);

	lastTime = time;
}


