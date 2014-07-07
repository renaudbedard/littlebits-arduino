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

	if (barrierTime <= millis)
	{
		sinceStarted = millis - startedAt;
		//if (!setjmp(returnLocation))
			return function(*this);
	}

	return false;
}

void Coroutine::reset()
{
	barrierTime = 0;
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
