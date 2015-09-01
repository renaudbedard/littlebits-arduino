#include <Util.h>
#include <Coroutines.h>

Coroutines<5> coroutines; 

void flashThrice(COROUTINE_CONTEXT(coroutine))
{
    COROUTINE_LOCAL(int, i);

    BEGIN_COROUTINE;

    for (i = 0; i < 3; i++)
    {
		Serial.println("ON!");

        coroutine.wait(100);
        COROUTINE_YIELD;

        Serial.println("off...");

        coroutine.wait(50);
        COROUTINE_YIELD;
    }

    END_COROUTINE;
}

void setup() 
{
	Serial.begin(115200);
}

void loop()
{
	coroutines.update();

	if (boolAnalogRead(A0))
		coroutines.start(flashThrice);
}
