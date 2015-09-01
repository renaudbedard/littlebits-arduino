#include <Util.h>
#include <Coroutines.h>

ADD_PRINTF_SUPPORT

Coroutines<10> coroutines;
bool wasHeld = false;

// Flashing behavior.
int BLINK_LEN_COUNT = 40;
int BLINK_DIM_START = 10;
int BLINK_LEN_INCR = 5;

void testLocals(COROUTINE_CONTEXT(coroutine))
{
    COROUTINE_LOCAL(int, i);
    BEGIN_COROUTINE;

//	trace("Start!\n");

    for (i = 5; i < BLINK_LEN_COUNT; i += BLINK_LEN_INCR)
    {
//		trace("Part 1 for %i\n", i);
		
        coroutine.wait(i);
        COROUTINE_YIELD;

//        trace("Part 2 for %i\n", i);

        coroutine.wait(i);
        COROUTINE_YIELD;
    }

//	trace("All done!\n");

    END_COROUTINE;
}

void setup() 
{
	printf_setup();
	Serial.begin(115200);
}

void loop()
{
	coroutines.update();

	bool held = boolAnalogRead(A0);

	if (held && !wasHeld)
	{
		coroutines.start(testLocals);
		wasHeld = true;
	}
	if (!held)
		wasHeld = false;
}
