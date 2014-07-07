#ifndef COROUTINES_H
#define COROUTINES_H

#include "Arduino.h"
#include <setjmp.h>
#include <assert.h>

// --

#define BEGIN_COROUTINE(functionName)				\
	bool functionName(Coroutine& coroutine)			\
	{												\
		if (coroutine.jumpLocation != NULL)			\
			longjmp(*coroutine.jumpLocation, 1);	\

#define COROUTINE_YIELD													\
		if (coroutine.jumpLocation != NULL)								\
			free(coroutine.jumpLocation);								\
		coroutine.jumpLocation = (jmp_buf*) malloc(sizeof(jmp_buf));	\
		if (!setjmp(*coroutine.jumpLocation))							\
			return false;
		//	longjmp(coroutine.returnLocation, 1);

#define END_COROUTINE								\
		if (coroutine.jumpLocation != NULL)			\
		{											\
			free(coroutine.jumpLocation);			\
			coroutine.jumpLocation = NULL;			\
		}											\
		return true;								\
	}

// --

class Coroutine;
typedef bool (*CoroutineBody)(Coroutine&);

// --

class Coroutine
{
public:
	// TODO : encapsulate better?
	CoroutineBody function;
	unsigned long barrierTime, sinceStarted, startedAt, suspendedAt;
	//void* barrierToCompare;
	//void* barrierComparedValue;
	size_t comparedSize;
	bool terminated, suspended;
	jmp_buf* jumpLocation;
	//jmp_buf returnLocation;

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
			Serial.print("Adding coroutine ");
			Serial.println(i);
			Coroutine& coroutine = coroutines[i];
			coroutine.reset();
			coroutine.function = function;
			coroutine.startedAt = millis();

			return coroutine;
		}

	// out of coroutines!
	Serial.println("Out of allocated coroutines!");
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
			Serial.print("Removing coroutine ");
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
