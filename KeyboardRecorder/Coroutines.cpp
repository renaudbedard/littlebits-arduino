#include "Coroutines.h"
#include "Arduino.h"

Coroutine::Coroutine() :
	jumpLocation(NULL)
{
}

Coroutine::~Coroutine()
{
	if (jumpLocation != NULL)
		free(jumpLocation);
}

bool Coroutine::update(unsigned long millis)
{
	if (terminated)
		return true;
	if (suspended)
		return false;

	timeToWait = timeToWait > dt ? timeToWait - dt : 0;
	sinceStarted += dt;

	if (timeToWait == 0)
		return function(*this);

	return false;
}

void Coroutine::reset()
{
	timeToWait = 0;
	sinceStarted = 0;
	terminated = suspended = false;
	function = NULL;
	if (jumpLocation != NULL)
	{
		free(jumpLocation);
		jumpLocation = NULL;
	}
}

void Coroutine::wait(unsigned long time)
{
	timeToWait = millis();
}

void Coroutine::terminate()
{
	terminated = true;
}

void Coroutine::suspend()
{
	suspended = true;
	suspendedAt = millis();
}

void Coroutine::resume() 
{
	suspended = false;
	startedAt += millis() - suspendedAt;
}
