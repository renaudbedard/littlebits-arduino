/*
  Coroutines.h - Library providing a simple coroutine system.
  Created by Renaud Bédard, July 14th, 2014.
  Released into the public domain.

  The variant of coroutines proposed in this library are inspired by Unity coroutines
  http://docs.unity3d.com/ScriptReference/Coroutine.html

  The basic idea is to be able to create blocks of code that execute sequentially,
  but can choose to stop temporarily and resume later. This is similar to threads,
  but in the case of coroutines, they never get pre-empted and will only give away
  focus if they explicitely yield.

  The library provides a coroutine manager that pre-allocates and updates coroutines,
  as well as provides wait, suspend and resume constructs to make their usage more
  convenient.

  A simple coroutine is declared like this :

	// flashes a LED attached to analog pin 5 for 100ms
	void flashOnce(Coroutine& coroutine)
	{
		BEGIN_COROUTINE;

		analogWrite(5, 255);

		coroutine.wait(100);
		COROUTINE_YIELD;

		analogWrite(5, 0);

		END_COROUTINE;
	}

  Here, the "wait" call adds 100ms to the timer that will prevent the coroutine from
  resuming on the next update. The COROUTINE_YIELD macro exits the function, and
  records the state of the coroutine so it can be resumed later. The BEGIN and END
  macros do the rest required for this to work.

  You may also use "coroutine locals", which are variables local to the coroutine
  and whose state will be preserved after a yield and recovered when resuming :

	void flashThrice(Coroutine& coroutine)
	{
		COROUTINE_LOCAL(int, i);

		BEGIN_COROUTINE;

		for (i = 0; i < 3; i++)
		{
			analogWrite(5, 255);

			coroutine.wait(100);
			COROUTINE_YIELD;

			analogWrite(5, 0);

			coroutine.wait(50);
			COROUTINE_YIELD;
		}

		END_COROUTINE;
	}

  Notice that the for(;;) loop does not declare "i" since it already has been.
  However, its value is undefined, like any other variable, until it's first set.
  Since it's declared as COROUTINE_LOCAL, after returning from the YIELD, its
  value will be restored to what it was prior to yielding.
  COROUTINE_LOCAL declarations must be done before BEGIN_COROUTINE.
  Coroutine locals live on the heap, so their size must be kept in check. Also,
  there is a maximum amount of them which defaults to 8, but can be tweaked in
  this header. (see Coroutine::MaxLocals)

  There are some preconditions that the sketch must meet to use coroutines :
  1. Declare a Coroutines<N> object, where N is the number of preallocated coroutines
	 required. In other words, the number of coroutines you expect your program to 
	 "concurrently" run.
  2. In your loop() function, call the update() function on that Coroutines<N> object.
  
  Declared coroutines will not be started automatically. The sketch needs to start
  them with a function call :

	// where "coroutines" is a Coroutine<N> instance,
	// and "flashOnce" is the name of a declared coroutine
	coroutines.start(flashOnce);

  This fires the coroutine, which will begin in the next update.
  The return type of the function must be void, and it must take a Coroutine object
  by reference (Coroutine&).

  You can keep a reference to the coroutine object via the return value of "start", 
  but since these objects are recycled, one must be careful to only use the reference
  while the coroutine it initially referred to is still alive.
  One way to do this would be to declare the coroutine reference as a pointer in the
  sketch's file-scope variables, and set it to NULL right before COROUTINE_END.

  If the sketch holds a reference or a pointer to a coroutine object, it can manipulate
  its execution from the outside using these functions :

  - suspend() will prevent any subsequent update to the coroutine
  - resume() reverts a suspended coroutine and allows it to execute in the next update
  - terminate() makes the coroutine prematurely exit in the next update

  The suspend() function may also be called from within a coroutine, which blocks
  its execution until resume() is called on it from the sketch.

  To let a coroutine clean up after an external termination, you can use the
  COROUTINE_FINALLY macro like this :

	void finallyExample(Coroutine& coroutine)
	{
		BEGIN_COROUTINE;

		coroutine.wait(1000);
		COROUTINE_YIELD;

		Serial.println("Waited 1000ms");

		COROUTINE_FINALLY
		{
			Serial.println("Exiting...");
		}

		END_COROUTINE;
	}

  In this example, the "Exiting..." string will be printed whether the coroutine
  is externally terminated or if it finished execution normally, after waiting 1000ms.
  The "Waited 1000ms" string however will only be printed if the coroutine stays
  alive for more than 1000ms.
  If used, the COROUTINE_FINALLY block must be placed before END_COROUTINE.

  There is currently no way to return something from a coroutine or to pass a parameter
  to a coroutine. However, they have access to the sketch's file-scope variables,
  which can be used for input and/or output.

  The library comes with debug-logging ability, which can be enabled by defining
  three macros :

  - trace(...)
  - assert(condition, ...)
  - P(string_literal)

  See their default definition below for how they need to be implemented.

  This coroutine implementation is based on Simon Tatham's
  http://www.chiark.greenend.org.uk/~sgtatham/coroutines.html
*/

#ifndef COROUTINES_H
#define COROUTINES_H

#include "Arduino.h"

// debugging macros, null operations unless defined prior to including this .h
// trace is a redirect to printf
#ifndef trace
#define trace(...)
#endif
// assert should be : while(cond) { trace(__VA_ARGS__); }
#ifndef assert
#define assert(cond, ...)
#endif
// P is a shortcut to PSTR (or F, but PSTR plays better with printf) with a "\n" appended
#ifndef P
#define P(string_literal)
#endif

#define BEGIN_COROUTINE							\
	trace(P("Entering coroutine #%hhu ('%s') at %lu ms"),		\
		  coroutine.id, __func__, coroutine.sinceStarted);	\
	switch (coroutine.jumpLocation)					\
	{								\
	case 0:										

#define COROUTINE_LOCAL(type, name)									\
		byte COROUTINE_localIndex = 0;								\
		if (coroutine.jumpLocation == 0 && !coroutine.looping)					\
		{											\
			assert(coroutine.numSavedLocals >= Coroutine::MaxLocals,			\
				   P("Ran out of coroutine locals! Increase Coroutine::MaxLocals"));	\
			trace(P("Allocating local '" #name "' (#%hhu)"), coroutine.numSavedLocals);	\
			COROUTINE_localIndex = coroutine.numSavedLocals;				\
			coroutine.savedLocals[coroutine.numSavedLocals++] = malloc(sizeof(type));	\
		}											\
		else											\
			COROUTINE_localIndex = coroutine.numRecoveredLocals++;				\
		type& name = *((type*) coroutine.savedLocals[COROUTINE_localIndex]);

#define COROUTINE_YIELD					\
		coroutine.jumpLocation = __LINE__;	\
		coroutine.numRecoveredLocals = 0;	\
		trace(P("...yielding..."));		\
		return;					\
	case __LINE__:	

#define COROUTINE_FINALLY		\
	case -1:			\
		if (coroutine.looping)	\
			break;				

#define END_COROUTINE					\
	default:					\
		_NOP();					\
	}						\
	coroutine.terminated = !coroutine.looping;	\
	return;
	
// --

class Coroutine;
typedef void (*CoroutineBody)(Coroutine&);

// --

class Coroutine
{
public:
	const static byte MaxLocals = 8;

	CoroutineBody function;
	unsigned long barrierTime, sinceStarted, startedAt, suspendedAt;

	byte id;
	bool terminated, suspended, looping;
	long jumpLocation;
	void* savedLocals[MaxLocals];
	byte numSavedLocals, numRecoveredLocals;

	void reset();
	bool update(unsigned long millis);

	void wait(unsigned long millis);
	void terminate();
	void suspend();
	void resume();
	void loop();
};

// --

template <byte N>
class Coroutines
{
private:
	Coroutine coroutines[N];
	unsigned int activeMask;
	byte activeCount;

public:
	Coroutines();

	Coroutine& start(CoroutineBody function);
	void update(unsigned long millis);
	void update();
};

// --

template <byte N>
Coroutines<N>::Coroutines() :
	activeMask(0),
	activeCount(0)
{
	for (byte i=0; i<N; i++)
		coroutines[i].id = i;
}

template <byte N>
Coroutine& Coroutines<N>::start(CoroutineBody function)
{
	for (byte i = 0; i < min(N, sizeof(unsigned int)); i++)
		if (!bitRead(activeMask, i))
		{
			bitSet(activeMask, i);
			activeCount++;

			// initialize
			trace(P("Adding coroutine #%hhu"), i);
			Coroutine& coroutine = coroutines[i];
			coroutine.reset();
			coroutine.function = function;
			coroutine.startedAt = millis();

			return coroutine;
		}

	// out of coroutines!
	assert(false, P("Out of allocated coroutines!"));
	abort();
}

template <byte N>
void Coroutines<N>::update(unsigned long millis)
{
	int bit = 0;
	int removed = 0;
	for (int i = 0; i < activeCount; i++)
	{
		while (!bitRead(activeMask, bit))
		{
			bit++;
			if (bit == N) bit = 0;
		}

		assert(bit >= N, P("Couldn't find active coroutine!"));

		Coroutine& coroutine = coroutines[bit];
		bool result = coroutine.update(millis);
		if (result)
		{
			// remove coroutine
			trace(P("Removing coroutine #%hhu"), bit);
			bitClear(activeMask, bit);
			coroutine.terminated = true;
			removed++;
		}

		bit++;
	}

	activeCount -= removed;
}

template <byte N>
void Coroutines<N>::update()
{
	update(millis());
}

#endif
