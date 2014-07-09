#ifndef COROUTINES_H
#define COROUTINES_H

#include "Arduino.h"
#include <assert.h>

#define BEGIN_COROUTINE					\
	switch (coroutine.jumpLocation)		\
	{									\
	case 0:										

#define COROUTINE_LOCAL(type, name)																\
	type name;																					\
	if (coroutine.jumpLocation == 0)															\
	{																							\
		if (coroutine.numSavedLocals >= Coroutine::MaxLocals)									\
		{																						\
			Serial.println(F("Ran out of coroutine locals! Increase Coroutine::MaxLocals"));	\
			assert(false);																		\
		}																						\
		Coroutine::SavedLocal& local = coroutine.savedLocals[coroutine.numSavedLocals++];		\
		local.sourceData = &name;																\
		local.copiedData = malloc(sizeof(type));												\
		local.length = sizeof(type);															\
	}																							\
	else																						\
	{																							\
		Coroutine::SavedLocal& local = coroutine.savedLocals[coroutine.numRecoveredLocals++];	\
		memcpy(&name, local.copiedData, local.length);											\
	}

#define COROUTINE_YIELD																\
	coroutine.jumpLocation = __LINE__;												\
	for (int COROUTINE_i=0; COROUTINE_i<coroutine.numSavedLocals; COROUTINE_i++)	\
	{																				\
		Coroutine::SavedLocal& local = coroutine.savedLocals[COROUTINE_i];			\
		memcpy(local.copiedData, local.sourceData, local.length);					\
	}																				\
	coroutine.numRecoveredLocals = 0;												\
	return false;																	\
	case __LINE__:						

#define END_COROUTINE	\
	}									

// --

class Coroutine;
typedef bool (*CoroutineBody)(Coroutine&);

// --

class Coroutine
{
public:
	const static int MaxLocals = 5;

	struct SavedLocal
	{
		const void* sourceData;
		void* copiedData;
		int length;
	};

	CoroutineBody function;
	unsigned long barrierTime, sinceStarted, startedAt, suspendedAt;

	//void* barrierToCompare;
	//void* barrierComparedValue;
	//size_t comparedSize;

	bool terminated, suspended;
	int jumpLocation;
	SavedLocal savedLocals[MaxLocals];
	int numSavedLocals;
	int numRecoveredLocals;

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
};

// --

template <int N>
class Coroutines
{
private:
	Coroutine coroutines[N];
	unsigned int activeMask;
	int activeCount;

public:
	Coroutines();

	Coroutine& add(CoroutineBody function);
	void update(unsigned long millis);
	void update();
};

// --

template <int N>
Coroutines<N>::Coroutines() :
	activeMask(0),
	activeCount(0)
{
}

template <int N>
Coroutine& Coroutines<N>::add(CoroutineBody function)
{
	for (unsigned i = 0; i < min(N, sizeof(unsigned long)); i++)
		if (!bitRead(activeMask, i))
		{
			bitSet(activeMask, i);
			activeCount++;

			// initialize
			Serial.print(F("Adding coroutine "));
			Serial.println(i);
			Coroutine& coroutine = coroutines[i];
			coroutine.reset();
			coroutine.function = function;
			coroutine.startedAt = millis();

			return coroutine;
		}

	// out of coroutines!
	Serial.println(F("Out of allocated coroutines!"));
	assert(false);
}

template <int N>
void Coroutines<N>::update(unsigned long millis)
{
	int bit = 0;
	int removed = 0;
	for (int i = 0; i < activeCount; i++)
	{
		while (!bitRead(activeMask, bit))
			bit++;

		Coroutine& coroutine = coroutines[bit];
		bool result = coroutine.update(millis);
		if (result)
		{
			// remove coroutine
			Serial.print(F("Removing coroutine "));
			Serial.println(bit);
			bitClear(activeMask, bit);
			coroutine.terminated = true;
			removed++;
		}

		bit++;
	}

	activeCount -= removed;
}

template <int N>
void Coroutines<N>::update()
{
	update(millis());
}

//template <typename T>
//void Coroutine::waitFor(T* pointer, T equals)
//{
//}

#endif
