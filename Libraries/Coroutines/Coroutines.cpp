/*
  Coroutines.cpp - Library providing a simple coroutine system.
  Created by Renaud Bédard, July 14th, 2014.
  Released into the public domain.

  See header file for full documentation.
*/

#include "Coroutines.h"

bool CoroutineImpl::update(unsigned long millis)
{
	if (suspended)
		return false;

	if (barrierTime <= millis)
	{
		sinceStarted = startedAt > millis ? 0 : millis - startedAt;
		function(*this);
		return terminated;
	}

	return false;
}

void CoroutineImpl::reset()
{
	barrierTime = 0;
	sinceStarted = 0;
	jumpLocation = 0;
	terminated = suspended = false;
	function = NULL;
	for (int i=0; i<numSavedLocals; i++)
		free(savedLocals[i]);
	numSavedLocals = 0;
	numRecoveredLocals = 0;
	terminated = false;
	suspended = false;
	looping = false;
}

void CoroutineImpl::wait(unsigned long time)
{
	barrierTime = millis() + time;
}

void CoroutineImpl::terminate()
{
	terminated = true;
	suspended = false;
	looping = false;
	jumpLocation = -1;
	barrierTime = 0;
}

void CoroutineImpl::suspend()
{
	if (!suspended && !terminated)
	{
		suspended = true;
		suspendedAt = millis();
	}
}

void CoroutineImpl::resume() 
{
	if (suspended && !terminated)
	{
		suspended = false;
		startedAt += millis() - suspendedAt;
	}
}

void CoroutineImpl::loop() 
{
	jumpLocation = 0;
	numRecoveredLocals = 0;
	looping = true;
	trace(P("...looping..."));
}


bool CoroutineImpl::isTerminated() const
{
	return terminated;
}

bool CoroutineImpl::isSuspended() const
{
	return suspended;
}
