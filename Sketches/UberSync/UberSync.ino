#include "Pins.h"

//#define SERIAL_DEBUG

#define HIGH_THRESHOLD 2
#define LOW_THRESHOLD 1

#define TIME_OFFSET 6

unsigned long lastTime, lastDoubleTime, lastQuadTime, lastOctoTime;
bool doubleState, quadState, octoState;
int interSync, doubleInterSync, quadInterSync, octoInterSync;

bool wasHigh;
float dutyCycle;

void setup()
{
	doubleState = quadState = octoState = false;
	lastOctoTime = lastQuadTime = lastDoubleTime = lastTime = millis();
	interSync = doubleInterSync = quadInterSync = octoInterSync = 0;

	wasHigh = false;

#ifdef SERIAL_DEBUG
	Serial.begin(115200);
#endif
}

inline void refreshDuty() 
{
	dutyCycle = analogRead(In::Analog::DutyCycle) / 1024.0f;
}

void loop()
{
	unsigned long currentTime = millis();

	// listen to beats from the SQ-1's Sync Out
	int listened = analogRead(In::Analog::Sync);
	if (listened > HIGH_THRESHOLD)
	{
		if (!wasHigh) 
		{
			refreshDuty();

			wasHigh = true;

			interSync = currentTime - lastTime;
			doubleInterSync = (int) (interSync / 2.0f);
			quadInterSync = (int) (interSync / 4.0f);
			octoInterSync = (int) (interSync / 8.0f);

			lastTime = currentTime;

			// trigger all
			doubleState = quadState = octoState = true;
			lastDoubleTime = lastQuadTime = lastOctoTime = currentTime;
			analogWrite(Out::Digital::DoubleSpeed, 255);
			analogWrite(Out::Digital::QuadSpeed, 255);
			analogWrite(Out::Digital::OctoSpeed, 255);

#ifdef SERIAL_DEBUG
			Serial.print("\nDQO");
#endif
		}
	}
	else if (listened < LOW_THRESHOLD)
		wasHigh = false;

	// time multiplication
	if (!doubleState && (currentTime - lastDoubleTime) > doubleInterSync)
	{
		doubleState = true;
		lastDoubleTime = currentTime;
		analogWrite(Out::Digital::DoubleSpeed, 255);

		refreshDuty();

#ifdef SERIAL_DEBUG
		Serial.print("D");
#endif
	}

	if (!quadState && (currentTime - lastQuadTime) > quadInterSync)
	{
		quadState = true;
		lastQuadTime = currentTime;
		analogWrite(Out::Digital::QuadSpeed, 255);

		refreshDuty();

#ifdef SERIAL_DEBUG
		Serial.print("Q");
#endif
	}

	if (!octoState && (currentTime - lastOctoTime) > octoInterSync)
	{
		octoState = true;
		lastOctoTime = currentTime;
		analogWrite(Out::Digital::OctoSpeed, 255);

		refreshDuty();

#ifdef SERIAL_DEBUG
		Serial.print("O");
#endif
	}

	// timed release
	if (doubleState && (currentTime - lastDoubleTime) > dutyCycle * doubleInterSync - TIME_OFFSET)
	{
		doubleState = false;
		analogWrite(Out::Digital::DoubleSpeed, 0);

#ifdef SERIAL_DEBUG
		Serial.print("d");
#endif
	}
	if (quadState && (currentTime - lastQuadTime) > dutyCycle * quadInterSync - TIME_OFFSET)
	{
		quadState = false;
		analogWrite(Out::Digital::QuadSpeed, 0);

#ifdef SERIAL_DEBUG
		Serial.print("q");
#endif
	}
	if (octoState && (currentTime - lastOctoTime) > dutyCycle * octoInterSync - TIME_OFFSET)
	{
		octoState = false;
		analogWrite(Out::Digital::OctoSpeed, 0);

#ifdef SERIAL_DEBUG
		Serial.print("o");
#endif
	}
}
