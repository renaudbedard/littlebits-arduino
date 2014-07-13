#ifndef COROUTINES_H
#define COROUTINES_H

#include "Arduino.h"
#include "Util.h"

#define BEGIN_COROUTINE										\
	trace(P("Entering coroutine #%hhu ('%s') at %lu ms"),	\
		  coroutine.id, __func__, coroutine.sinceStarted);	\
	switch (coroutine.jumpLocation)							\
	{														\
	case 0:										

#define COROUTINE_LOCAL(type, name)														\
		byte COROUTINE_localIndex = 0;													\
		if (coroutine.jumpLocation == 0 && !coroutine.looping)							\
		{																				\
			assert(coroutine.numSavedLocals >= Coroutine::MaxLocals,					\
				   P("Ran out of coroutine locals! Increase Coroutine::MaxLocals"));	\
			trace(P("Allocating local '" #name "' (#%hhu)"), coroutine.numSavedLocals);	\
			COROUTINE_localIndex = coroutine.numSavedLocals;							\
			coroutine.savedLocals[coroutine.numSavedLocals++] = malloc(sizeof(type));	\
		}																				\
		else																			\
			COROUTINE_localIndex = coroutine.numRecoveredLocals++;						\
		type& name = *((type*) coroutine.savedLocals[COROUTINE_localIndex]);

#define COROUTINE_YIELD						\
		coroutine.jumpLocation = __LINE__;	\
		coroutine.numRecoveredLocals = 0;	\
		trace(P("...yielding..."));			\
		return false;						\
	case __LINE__:	

#define COROUTINE_FINALLY		\
	case -1:					\
		if (coroutine.looping)	\
			break;				

#define END_COROUTINE			\
	default:					\
		_NOP();					\
	}							\
	return !coroutine.looping;	
	
// --

class Coroutine;
typedef bool (*CoroutineBody)(Coroutine&);

// --

class Coroutine
{
public:
	const static byte MaxLocals = 8;

	CoroutineBody function;
	unsigned long barrierTime, sinceStarted, startedAt, suspendedAt;

	//void* barrierToCompare;
	//void* barrierComparedValue;
	//size_t comparedSize;

	byte id;
	bool terminated, suspended, looping;
	long jumpLocation;
	void* savedLocals[MaxLocals];
	byte numSavedLocals, numRecoveredLocals;

	Coroutine();
	~Coroutine();

	void reset();
	bool update(unsigned long millis);

	//template <typename T>
	//void waitFor(T* pointer, T equals);

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

//template <typename T>
//void Coroutine::waitFor(T* pointer, T equals)
//{
//}

#endif
