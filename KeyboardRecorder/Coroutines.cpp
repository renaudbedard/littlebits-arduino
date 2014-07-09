#include "Coroutines.h"
#include "Arduino.h"

Coroutine::Coroutine()
{
}

Coroutine::~Coroutine()
{
}

bool Coroutine::update(unsigned long millis)
{
	if (terminated)
		return true;
	if (suspended)
		return false;

	if (barrierTime <= millis)
	{
		sinceStarted = millis - startedAt;
		return function(*this);
	}

	return false;
}

void Coroutine::reset()
{
	barrierTime = 0;
	sinceStarted = 0;
	jumpLocation = 0;
	terminated = suspended = false;
	function = NULL;
	for (int i=0; i<numSavedLocals; i++)
		free(savedLocals[i].copiedData);
	numSavedLocals = 0;
	numRecoveredLocals = 0;
}

void Coroutine::wait(unsigned long time)
{
	barrierTime = millis() + time;
}

void Coroutine::terminate()
{
	terminated = true;
	suspended = false;
}

void Coroutine::suspend()
{
	if (!suspended && !terminated)
	{
		suspended = true;
		suspendedAt = millis();
	}
}

void Coroutine::resume() 
{
	if (suspended && !terminated)
	{
		suspended = false;
		startedAt += millis() - suspendedAt;
	}
}
